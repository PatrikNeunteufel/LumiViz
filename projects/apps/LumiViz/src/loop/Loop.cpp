/**
 * @file Loop.cpp
 * @brief Loop: Datenmodell und Datei `.lvloop`
 */

#include "loop/Loop.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-loop");
// 2 (S108): ein Arrangement — Kanaele mit Clips, das Kit im Muster. Fassung 1
// (Kit am Loop, Abfolge als Liste) gab es nur einen Tag und wird nicht gelesen.
// 3 (S113, Entscheid F2 a): die Effekte am Clip. Fassung 2 wird weiter
// gelesen, geschrieben wird immer 3.
constexpr int kVersion = 3;
constexpr int kVersionAelteste = 2;
const QString kKitVorsatz = QStringLiteral("kit:");
const QString kArtDrumkit = QStringLiteral("drumkit");
const QString kArtInstrument = QStringLiteral("instrument");
const QString kTonVorsatz = QStringLiteral("inst:");
const QString kInstrumentVorgabe = QStringLiteral("piano");
const QString kInstrumentEndung = QStringLiteral(".lvinst");

// Pfade wie in der `.lvcomp` (composer/Komposition.cpp): relativ zum Ordner
// der Datei, wenn das Sample darunter liegt, sonst absolut.
QString fuerDatei(const QString& datei, const QString& basisOrdner)
{
    if (datei.isEmpty() || basisOrdner.isEmpty()) return QDir::fromNativeSeparators(datei);
    const QString rel = QDir(basisOrdner).relativeFilePath(datei);
    if (rel.startsWith(QStringLiteral("..")) || QFileInfo(rel).isAbsolute())
        return QDir::fromNativeSeparators(datei);
    return rel;
}

QString ausDatei(const QString& eintrag, const QString& basisOrdner)
{
    if (eintrag.isEmpty()) return {};
    if (QFileInfo(eintrag).isAbsolute() || basisOrdner.isEmpty())
        return QDir::cleanPath(QDir::fromNativeSeparators(eintrag));
    return QDir::cleanPath(QDir(basisOrdner).absoluteFilePath(eintrag));
}

double inGrenzen(double wert, double von, double bis, double ersatz)
{
    return std::isfinite(wert) ? std::clamp(wert, von, bis) : ersatz;
}

/// Qt schreibt jede Zahl einer Liste auf eine eigene Zeile — bei 64 Schritten
/// je Spur wird die Datei unlesbar lang. Listen, die nur Zahlen enthalten,
/// kommen hier auf eine Zeile.
QByteArray zahlenreihenAufEineZeile(const QByteArray& json)
{
    static const QRegularExpression kReihe(QStringLiteral("\\[\\s*(-?[0-9][-0-9.eE+,\\s]*)\\]"));
    static const QRegularExpression kLuft(QStringLiteral("\\s+"));
    const QString text = QString::fromUtf8(json);
    QString aus;
    aus.reserve(text.size());
    qsizetype stelle = 0;
    QRegularExpressionMatchIterator treffer = kReihe.globalMatch(text);
    while (treffer.hasNext())
    {
        const QRegularExpressionMatch m = treffer.next();
        aus += QStringView(text).mid(stelle, m.capturedStart() - stelle);
        aus += QLatin1Char('[') + m.captured(1).remove(kLuft).replace(QLatin1Char(','), QStringLiteral(", ")) +
               QLatin1Char(']');
        stelle = m.capturedEnd();
    }
    aus += QStringView(text).mid(stelle);
    return aus.toUtf8();
}

// --- Effekte am Clip ---

struct ZeitName
{
    EchoZeit zeit;
    const char* name;
};
constexpr ZeitName kEchoZeiten[] = {
    {EchoZeit::Sechzehntel, "1/16"}, {EchoZeit::Achtel, "1/8"}, {EchoZeit::AchtelPunktiert, "1/8d"},
    {EchoZeit::Viertel, "1/4"},      {EchoZeit::Halbe, "1/2"},
};

QString echoZeitName(EchoZeit zeit)
{
    for (const ZeitName& z : kEchoZeiten)
        if (z.zeit == zeit) return QLatin1String(z.name);
    return QLatin1String(kEchoZeiten[2].name);
}

void richteEffekte(ClipEffekte& e)
{
    const ClipEffekte vorgabe;
    e.verzerrung.drive = inGrenzen(e.verzerrung.drive, 0.0, 1.0, vorgabe.verzerrung.drive);
    e.verzerrung.ton = inGrenzen(e.verzerrung.ton, 0.0, 1.0, vorgabe.verzerrung.ton);
    e.verzerrung.mix = inGrenzen(e.verzerrung.mix, 0.0, 1.0, vorgabe.verzerrung.mix);
    if (e.echo.zeit < EchoZeit::Sechzehntel || e.echo.zeit > EchoZeit::Halbe) e.echo.zeit = vorgabe.echo.zeit;
    e.echo.feedback = inGrenzen(e.echo.feedback, 0.0, kEchoFeedbackMax, vorgabe.echo.feedback);
    e.echo.daempfung = inGrenzen(e.echo.daempfung, 0.0, 1.0, vorgabe.echo.daempfung);
    e.echo.mix = inGrenzen(e.echo.mix, 0.0, 1.0, vorgabe.echo.mix);
    e.hall.groesse = inGrenzen(e.hall.groesse, 0.0, 1.0, vorgabe.hall.groesse);
    e.hall.sekunden = inGrenzen(e.hall.sekunden, kHallSekMin, kHallSekMax, vorgabe.hall.sekunden);
    e.hall.daempfung = inGrenzen(e.hall.daempfung, 0.0, 1.0, vorgabe.hall.daempfung);
    e.hall.mix = inGrenzen(e.hall.mix, 0.0, 1.0, vorgabe.hall.mix);
}

/// Nur, was von der Vorgabe abweicht: ein Clip ohne Effekte traegt das Feld
/// nicht, ein ausgeschalteter Effekt mit eigenen Werten behaelt sie.
QJsonObject effekteAlsJson(const ClipEffekte& e)
{
    const ClipEffekte vorgabe;
    QJsonObject o;
    if (e.verzerrung != vorgabe.verzerrung)
    {
        QJsonObject v;
        v[QStringLiteral("on")] = e.verzerrung.an;
        v[QStringLiteral("drive")] = e.verzerrung.drive;
        v[QStringLiteral("tone")] = e.verzerrung.ton;
        v[QStringLiteral("mix")] = e.verzerrung.mix;
        o[QStringLiteral("distortion")] = v;
    }
    if (e.echo != vorgabe.echo)
    {
        QJsonObject v;
        v[QStringLiteral("on")] = e.echo.an;
        v[QStringLiteral("time")] = echoZeitName(e.echo.zeit);
        v[QStringLiteral("feedback")] = e.echo.feedback;
        v[QStringLiteral("damping")] = e.echo.daempfung;
        v[QStringLiteral("mix")] = e.echo.mix;
        o[QStringLiteral("echo")] = v;
    }
    if (e.hall != vorgabe.hall)
    {
        QJsonObject v;
        v[QStringLiteral("on")] = e.hall.an;
        v[QStringLiteral("size")] = e.hall.groesse;
        v[QStringLiteral("decay")] = e.hall.sekunden;
        v[QStringLiteral("damping")] = e.hall.daempfung;
        v[QStringLiteral("mix")] = e.hall.mix;
        o[QStringLiteral("reverb")] = v;
    }
    return o;
}

ClipEffekte effekteAusJson(const QJsonObject& o, QStringList* bericht)
{
    ClipEffekte e;
    if (const QJsonValue wert = o.value(QStringLiteral("distortion")); wert.isObject())
    {
        const QJsonObject v = wert.toObject();
        e.verzerrung.an = v.value(QStringLiteral("on")).toBool(false);
        e.verzerrung.drive = v.value(QStringLiteral("drive")).toDouble(e.verzerrung.drive);
        e.verzerrung.ton = v.value(QStringLiteral("tone")).toDouble(e.verzerrung.ton);
        e.verzerrung.mix = v.value(QStringLiteral("mix")).toDouble(e.verzerrung.mix);
    }
    if (const QJsonValue wert = o.value(QStringLiteral("echo")); wert.isObject())
    {
        const QJsonObject v = wert.toObject();
        e.echo.an = v.value(QStringLiteral("on")).toBool(false);
        if (const QJsonValue zeit = v.value(QStringLiteral("time")); !zeit.isUndefined())
        {
            const QString name = zeit.toString();
            const auto* treffer = std::find_if(std::begin(kEchoZeiten), std::end(kEchoZeiten),
                                               [&name](const ZeitName& z) { return name == QLatin1String(z.name); });
            if (treffer != std::end(kEchoZeiten))
                e.echo.zeit = treffer->zeit;
            else if (bericht != nullptr)
            {
                const QString zeile = QStringLiteral("Unknown echo time '%1'").arg(name);
                if (!bericht->contains(zeile)) bericht->append(zeile);
            }
        }
        e.echo.feedback = v.value(QStringLiteral("feedback")).toDouble(e.echo.feedback);
        e.echo.daempfung = v.value(QStringLiteral("damping")).toDouble(e.echo.daempfung);
        e.echo.mix = v.value(QStringLiteral("mix")).toDouble(e.echo.mix);
    }
    if (const QJsonValue wert = o.value(QStringLiteral("reverb")); wert.isObject())
    {
        const QJsonObject v = wert.toObject();
        e.hall.an = v.value(QStringLiteral("on")).toBool(false);
        e.hall.groesse = v.value(QStringLiteral("size")).toDouble(e.hall.groesse);
        e.hall.sekunden = v.value(QStringLiteral("decay")).toDouble(e.hall.sekunden);
        e.hall.daempfung = v.value(QStringLiteral("damping")).toDouble(e.hall.daempfung);
        e.hall.mix = v.value(QStringLiteral("mix")).toDouble(e.hall.mix);
    }
    return e;
}

}  // namespace

double echoSchlaege(EchoZeit zeit)
{
    switch (zeit)
    {
        case EchoZeit::Sechzehntel: return 0.25;
        case EchoZeit::Achtel: return 0.5;
        case EchoZeit::AchtelPunktiert: return 0.75;
        case EchoZeit::Viertel: return 1.0;
        case EchoZeit::Halbe: return 2.0;
    }
    return 0.75;
}

bool istKitKlang(const QString& klang)
{
    return klang.startsWith(kKitVorsatz);
}

bool istTon(const QString& klang)
{
    return klang.startsWith(kTonVorsatz);
}

QString tonKlang(const QString& instrument, int hoehe, int dauerMs)
{
    return QStringLiteral("%1%2:%3:%4").arg(kTonVorsatz, instrument).arg(hoehe).arg(dauerMs);
}

QString instrumentVorgabe()
{
    return kInstrumentVorgabe;
}

bool istInstrumentDatei(const QString& nennung)
{
    return nennung.endsWith(kInstrumentEndung, Qt::CaseInsensitive);
}

bool zerlegeTon(const QString& klang, QString& instrument, int& hoehe, int& dauerMs)
{
    if (!istTon(klang)) return false;
    // von hinten: Dauer, Tonhoehe — davor das Instrument, das als Pfad selbst
    // einen Doppelpunkt tragen kann (C:/…)
    const QString rest = klang.mid(kTonVorsatz.size());
    const qsizetype vorDauer = rest.lastIndexOf(QLatin1Char(':'));
    const qsizetype vorHoehe = vorDauer > 0 ? rest.lastIndexOf(QLatin1Char(':'), vorDauer - 1) : -1;
    if (vorHoehe <= 0) return false;
    bool hoeheOk = false;
    bool dauerOk = false;
    const int h = rest.mid(vorHoehe + 1, vorDauer - vorHoehe - 1).toInt(&hoeheOk);
    const int ms = rest.mid(vorDauer + 1).toInt(&dauerOk);
    if (!hoeheOk || !dauerOk || ms <= 0) return false;
    instrument = rest.left(vorHoehe);
    hoehe = h;
    dauerMs = ms;
    return true;
}

const Muster* Loop::findeMuster(const QString& name) const
{
    const auto it = std::find_if(muster.begin(), muster.end(), [&name](const Muster& m) { return m.name == name; });
    return it == muster.end() ? nullptr : &*it;
}

int Loop::clipTakte(const LoopClip& clip) const
{
    const Muster* m = findeMuster(clip.muster);
    return m != nullptr ? std::max(1, clip.wiederholungen) * m->takte : 0;
}

int Loop::durchlaufTakte() const
{
    int ende = 0;
    for (const Kanal& kanal : kanaele)
        for (const LoopClip& clip : kanal.clips) ende = std::max(ende, clip.startTakt + clipTakte(clip));
    return std::max(1, ende);
}

double Loop::durchlaufMs() const
{
    if (bpm <= 0.0 || schritteJeSchlag <= 0) return 0.0;
    return durchlaufSchritte() * 60000.0 / bpm / schritteJeSchlag;
}

QStringList Loop::klangNamen() const
{
    QStringList namen;
    QSet<QString> gesehen;
    const auto nimm = [&](const QString& klang) {
        if (klang.isEmpty() || gesehen.contains(klang)) return;
        gesehen.insert(klang);
        namen << klang;
    };
    for (const Muster& m : muster)
    {
        if (m.art == MusterArt::Instrument)
            for (const Note& note : m.noten) nimm(tonKlang(m.instrument, note.hoehe, notenMs(note)));
        else
            for (const Spur& spur : m.spuren) nimm(spur.klang);
    }
    return namen;
}

int Loop::notenMs(const Note& note) const
{
    if (bpm <= 0.0 || schritteJeSchlag <= 0) return 1;
    return std::max(1, static_cast<int>(std::lround(note.laenge * 60000.0 / bpm / schritteJeSchlag)));
}

void Loop::richte(QStringList* bericht)
{
    bpm = inGrenzen(bpm, kBpmMin, kBpmMax, 120.0);
    schlaegeJeTakt = std::clamp(schlaegeJeTakt, 1, kRasterMax);
    schritteJeSchlag = std::clamp(schritteJeSchlag, 1, kRasterMax);
    swing = inGrenzen(swing, 0.0, 1.0, 0.0);
    laut = inGrenzen(laut, 0.0, 1.0, 0.5);

    QStringList namen;
    for (std::size_t i = 0; i < muster.size(); ++i)
    {
        Muster& m = muster[i];
        if (m.name.isEmpty()) m.name = QStringLiteral("Pattern %1").arg(i + 1);
        if (namen.contains(m.name))
        {
            if (bericht != nullptr) bericht->append(QStringLiteral("Pattern name used twice: %1").arg(m.name));
        }
        else
            namen.append(m.name);
        m.takte = std::clamp(m.takte, kTakteMin, kTakteMax);
        if (m.art == MusterArt::Instrument)
        {
            // ein Instrument hat keine Spuren; seine Noten bleiben im Muster
            m.spuren.clear();
            m.zeilen.clear();
            if (m.instrument.isEmpty()) m.instrument = kInstrumentVorgabe;
            m.laut = inGrenzen(m.laut, 0.0, 1.0, 1.0);
            m.pan = inGrenzen(m.pan, -1.0, 1.0, 0.0);
            const int schritte = m.takte * schritteJeTakt();
            for (Note& note : m.noten)
            {
                note.hoehe = std::clamp(note.hoehe, kHoeheMin, kHoeheMax);
                note.laenge = std::clamp(note.laenge, 1, std::max(1, schritte - note.schritt));
                note.laut = inGrenzen(note.laut, 0.0, 1.0, 1.0);
            }
            m.noten.erase(std::remove_if(m.noten.begin(), m.noten.end(),
                                         [schritte](const Note& n) {
                                             return n.schritt < 0 || n.schritt >= schritte || n.laut <= 0.0;
                                         }),
                          m.noten.end());
            std::stable_sort(m.noten.begin(), m.noten.end(), [](const Note& a, const Note& b) {
                return a.schritt != b.schritt ? a.schritt < b.schritt : a.hoehe < b.hoehe;
            });
            m.noten.erase(std::unique(m.noten.begin(), m.noten.end(),
                                      [](const Note& a, const Note& b) {
                                          return a.schritt == b.schritt && a.hoehe == b.hoehe;
                                      }),
                          m.noten.end());
            continue;
        }
        m.instrument.clear();
        m.noten.clear();
        m.laut = 1.0;
        m.pan = 0.0;
        m.stumm = false;
        for (Spur& spur : m.spuren)
        {
            spur.laut = inGrenzen(spur.laut, 0.0, 1.0, 1.0);
            spur.pan = inGrenzen(spur.pan, -1.0, 1.0, 0.0);
        }
        m.zeilen.resize(m.spuren.size());
        const auto schritte = static_cast<std::size_t>(m.takte * schritteJeTakt());
        for (std::vector<double>& zeile : m.zeilen)
        {
            zeile.resize(schritte, 0.0);
            for (double& wert : zeile) wert = inGrenzen(wert, 0.0, 1.0, 0.0);
        }
    }

    for (Kanal& kanal : kanaele)
    {
        // Clips ohne Muster gehen: sie haetten keine Laenge
        const auto fehlt = [&](const LoopClip& clip) {
            if (namen.contains(clip.muster)) return false;
            if (bericht != nullptr) bericht->append(QStringLiteral("Pattern not found: %1").arg(clip.muster));
            return true;
        };
        kanal.clips.erase(std::remove_if(kanal.clips.begin(), kanal.clips.end(), fehlt), kanal.clips.end());
        for (LoopClip& clip : kanal.clips)
        {
            clip.startTakt = std::max(0, clip.startTakt);
            clip.wiederholungen = std::max(1, clip.wiederholungen);
            richteEffekte(clip.effekte);
        }
        std::stable_sort(kanal.clips.begin(), kanal.clips.end(),
                         [](const LoopClip& a, const LoopClip& b) { return a.startTakt < b.startTakt; });
        // Ueberlappung: der spaetere Clip beginnt, wo der fruehere endet
        int frei = 0;
        for (LoopClip& clip : kanal.clips)
        {
            clip.startTakt = std::max(clip.startTakt, frei);
            frei = clip.startTakt + clipTakte(clip);
        }
    }
}

QJsonObject alsJson(const Loop& loop, const QString& basisOrdner)
{
    QJsonObject wurzel;
    wurzel[QStringLiteral("format")] = kFormat;
    wurzel[QStringLiteral("version")] = kVersion;
    wurzel[QStringLiteral("bpm")] = loop.bpm;
    wurzel[QStringLiteral("beatsPerBar")] = loop.schlaegeJeTakt;
    wurzel[QStringLiteral("stepsPerBeat")] = loop.schritteJeSchlag;
    wurzel[QStringLiteral("swing")] = loop.swing;
    wurzel[QStringLiteral("volume")] = loop.laut;
    // die Tags: nur, was gesetzt ist — ein Loop ohne Tags traegt den Block nicht
    QJsonObject tags;
    const auto tag = [&tags](const char* name, const QString& wert) {
        if (!wert.isEmpty()) tags[QLatin1String(name)] = wert;
    };
    tag("title", loop.tags.titel);
    tag("artist", loop.tags.kuenstler);
    tag("album", loop.tags.album);
    tag("year", loop.tags.jahr);
    tag("genre", loop.tags.genre);
    tag("track", loop.tags.nummer);
    tag("lyrics", loop.tags.text);
    if (!tags.isEmpty()) wurzel[QStringLiteral("tags")] = tags;

    QJsonArray muster;
    for (const Muster& m : loop.muster)
    {
        QJsonObject o;
        o[QStringLiteral("name")] = m.name;
        o[QStringLiteral("bars")] = m.takte;
        if (m.art == MusterArt::Instrument)
        {
            o[QStringLiteral("kind")] = kArtInstrument;
            // ein mitgeliefertes beim Namen, eine eigene Datei beim Pfad — wie ein Sample
            o[QStringLiteral("instrument")] =
                istInstrumentDatei(m.instrument) ? fuerDatei(m.instrument, basisOrdner) : m.instrument;
            o[QStringLiteral("volume")] = m.laut;
            o[QStringLiteral("pan")] = m.pan;
            o[QStringLiteral("mute")] = m.stumm;
            // je Note eine Zeile: Schritt, Laenge, Tonhoehe, Lautstaerke
            QJsonArray noten;
            for (const Note& note : m.noten)
                noten.append(QJsonArray{note.schritt, note.laenge, note.hoehe, note.laut});
            o[QStringLiteral("notes")] = noten;
            muster.append(o);
            continue;
        }
        o[QStringLiteral("kind")] = kArtDrumkit;
        QJsonArray spuren;
        for (const Spur& spur : m.spuren)
        {
            QJsonObject s;
            s[QStringLiteral("name")] = spur.name;
            s[QStringLiteral("sound")] =
                istKitKlang(spur.klang) ? spur.klang : fuerDatei(spur.klang, basisOrdner);
            s[QStringLiteral("volume")] = spur.laut;
            s[QStringLiteral("pan")] = spur.pan;
            s[QStringLiteral("mute")] = spur.stumm;
            spuren.append(s);
        }
        o[QStringLiteral("tracks")] = spuren;
        QJsonArray zeilen;
        for (const std::vector<double>& zeile : m.zeilen)
        {
            QJsonArray werte;
            for (const double wert : zeile) werte.append(wert);
            zeilen.append(werte);
        }
        o[QStringLiteral("rows")] = zeilen;
        muster.append(o);
    }
    wurzel[QStringLiteral("patterns")] = muster;

    QJsonArray kanaele;
    for (const Kanal& kanal : loop.kanaele)
    {
        QJsonObject k;
        k[QStringLiteral("name")] = kanal.name;
        k[QStringLiteral("enabled")] = kanal.an;
        QJsonArray clips;
        for (const LoopClip& clip : kanal.clips)
        {
            QJsonObject c;
            c[QStringLiteral("pattern")] = clip.muster;
            c[QStringLiteral("start")] = clip.startTakt;
            c[QStringLiteral("repeat")] = clip.wiederholungen;
            if (const QJsonObject effekte = effekteAlsJson(clip.effekte); !effekte.isEmpty())
                c[QStringLiteral("effects")] = effekte;
            clips.append(c);
        }
        k[QStringLiteral("clips")] = clips;
        kanaele.append(k);
    }
    wurzel[QStringLiteral("channels")] = kanaele;
    return wurzel;
}

bool ausJson(const QJsonObject& wurzel, Loop& loop, const QString& basisOrdner, QStringList* bericht)
{
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    const int version = wurzel.value(QStringLiteral("version")).toInt(kVersion);
    if (version < kVersionAelteste) return false;
    if (bericht != nullptr && version > kVersion)
        bericht->append(QStringLiteral("File was written by a newer version"));

    Loop neu;
    neu.bpm = wurzel.value(QStringLiteral("bpm")).toDouble(neu.bpm);
    neu.schlaegeJeTakt = wurzel.value(QStringLiteral("beatsPerBar")).toInt(neu.schlaegeJeTakt);
    neu.schritteJeSchlag = wurzel.value(QStringLiteral("stepsPerBeat")).toInt(neu.schritteJeSchlag);
    neu.swing = wurzel.value(QStringLiteral("swing")).toDouble(neu.swing);
    neu.laut = wurzel.value(QStringLiteral("volume")).toDouble(neu.laut);
    const QJsonObject tags = wurzel.value(QStringLiteral("tags")).toObject();
    neu.tags.titel = tags.value(QStringLiteral("title")).toString();
    neu.tags.kuenstler = tags.value(QStringLiteral("artist")).toString();
    neu.tags.album = tags.value(QStringLiteral("album")).toString();
    neu.tags.jahr = tags.value(QStringLiteral("year")).toString();
    neu.tags.genre = tags.value(QStringLiteral("genre")).toString();
    neu.tags.nummer = tags.value(QStringLiteral("track")).toString();
    neu.tags.text = tags.value(QStringLiteral("lyrics")).toString();

    for (const QJsonValue& mv : wurzel.value(QStringLiteral("patterns")).toArray())
    {
        const QJsonObject o = mv.toObject();
        Muster m;
        m.name = o.value(QStringLiteral("name")).toString();
        // eine Art, die diese Fassung nicht kennt: das Muster bleibt weg,
        // seine Clips meldet `richte`
        const QString art = o.value(QStringLiteral("kind")).toString(kArtDrumkit);
        if (art != kArtDrumkit && art != kArtInstrument)
        {
            if (bericht != nullptr)
                bericht->append(QStringLiteral("Unknown pattern kind '%1': %2").arg(art, m.name));
            continue;
        }
        m.takte = o.value(QStringLiteral("bars")).toInt(kTakteMin);
        if (art == kArtInstrument)
        {
            m.art = MusterArt::Instrument;
            m.instrument = o.value(QStringLiteral("instrument")).toString();
            if (istInstrumentDatei(m.instrument))
            {
                m.instrument = ausDatei(m.instrument, basisOrdner);
                if (bericht != nullptr && !QFileInfo::exists(m.instrument))
                {
                    const QString zeile =
                        QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(m.instrument));
                    if (!bericht->contains(zeile)) bericht->append(zeile);
                }
            }
            m.laut = o.value(QStringLiteral("volume")).toDouble(1.0);
            m.pan = o.value(QStringLiteral("pan")).toDouble(0.0);
            m.stumm = o.value(QStringLiteral("mute")).toBool(false);
            for (const QJsonValue& nv : o.value(QStringLiteral("notes")).toArray())
            {
                const QJsonArray werte = nv.toArray();
                if (werte.size() < 3) continue;
                Note note;
                note.schritt = werte[0].toInt();
                note.laenge = werte[1].toInt(1);
                note.hoehe = werte[2].toInt(60);
                note.laut = werte.size() > 3 ? werte[3].toDouble(1.0) : 1.0;
                m.noten.push_back(note);
            }
            neu.muster.push_back(std::move(m));
            continue;
        }
        for (const QJsonValue& sv : o.value(QStringLiteral("tracks")).toArray())
        {
            const QJsonObject s = sv.toObject();
            Spur spur;
            spur.name = s.value(QStringLiteral("name")).toString();
            const QString klang = s.value(QStringLiteral("sound")).toString();
            spur.klang = istKitKlang(klang) ? klang : ausDatei(klang, basisOrdner);
            spur.laut = s.value(QStringLiteral("volume")).toDouble(1.0);
            spur.pan = s.value(QStringLiteral("pan")).toDouble(0.0);
            spur.stumm = s.value(QStringLiteral("mute")).toBool(false);
            if (bericht != nullptr && !spur.klang.isEmpty() && !istKitKlang(spur.klang) &&
                !QFileInfo::exists(spur.klang))
            {
                const QString zeile =
                    QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(spur.klang));
                if (!bericht->contains(zeile)) bericht->append(zeile);
            }
            m.spuren.push_back(std::move(spur));
        }
        for (const QJsonValue& zv : o.value(QStringLiteral("rows")).toArray())
        {
            std::vector<double> zeile;
            for (const QJsonValue& wert : zv.toArray()) zeile.push_back(wert.toDouble());
            m.zeilen.push_back(std::move(zeile));
        }
        neu.muster.push_back(std::move(m));
    }

    for (const QJsonValue& kv : wurzel.value(QStringLiteral("channels")).toArray())
    {
        const QJsonObject k = kv.toObject();
        Kanal kanal;
        kanal.name = k.value(QStringLiteral("name")).toString();
        kanal.an = k.value(QStringLiteral("enabled")).toBool(true);
        for (const QJsonValue& cv : k.value(QStringLiteral("clips")).toArray())
        {
            const QJsonObject c = cv.toObject();
            LoopClip clip;
            clip.muster = c.value(QStringLiteral("pattern")).toString();
            clip.startTakt = c.value(QStringLiteral("start")).toInt(0);
            clip.wiederholungen = c.value(QStringLiteral("repeat")).toInt(1);
            clip.effekte = effekteAusJson(c.value(QStringLiteral("effects")).toObject(), bericht);
            kanal.clips.push_back(std::move(clip));
        }
        neu.kanaele.push_back(std::move(kanal));
    }

    neu.richte(bericht);
    loop = std::move(neu);
    return true;
}

bool speichereLoop(const Loop& loop, const QString& pfad)
{
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const QJsonDocument doc(alsJson(loop, QFileInfo(pfad).absolutePath()));
    return datei.write(zahlenreihenAufEineZeile(doc.toJson(QJsonDocument::Indented))) >= 0 && datei.commit();
}

bool ladeLoop(const QString& pfad, Loop& loop, QStringList* bericht)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    return ausJson(doc.object(), loop, QFileInfo(pfad).absolutePath(), bericht);
}

}  // namespace lumi::loop

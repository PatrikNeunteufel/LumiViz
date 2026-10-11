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

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

const QString kFormat = QStringLiteral("lumiviz-loop");
// 2 (S108): ein Arrangement — Kanaele mit Clips, das Kit im Muster. Fassung 1
// (Kit am Loop, Abfolge als Liste) gab es nur einen Tag und wird nicht gelesen.
constexpr int kVersion = 2;
const QString kKitVorsatz = QStringLiteral("kit:");
const QString kArtDrumkit = QStringLiteral("drumkit");

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

}  // namespace

bool istKitKlang(const QString& klang)
{
    return klang.startsWith(kKitVorsatz);
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
    for (const Muster& m : muster)
        for (const Spur& spur : m.spuren)
            if (!spur.klang.isEmpty() && !namen.contains(spur.klang)) namen << spur.klang;
    return namen;
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
        o[QStringLiteral("kind")] = kArtDrumkit;
        o[QStringLiteral("bars")] = m.takte;
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
    if (version < kVersion) return false;
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
        if (const QString art = o.value(QStringLiteral("kind")).toString(kArtDrumkit); art != kArtDrumkit)
        {
            if (bericht != nullptr)
                bericht->append(QStringLiteral("Unknown pattern kind '%1': %2").arg(art, m.name));
            continue;
        }
        m.takte = o.value(QStringLiteral("bars")).toInt(kTakteMin);
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

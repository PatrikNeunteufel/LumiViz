/**
 * @file Loop.cpp
 * @brief Loop: Datenmodell und Datei `.lvloop`
 */

#include "loop/Loop.hpp"

#include "loop/Saitenplan.hpp"
#include "loop/Sample.hpp"

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
// 4 (S115, Stufe 11c, Entscheide K1 a und K2 a): eine Spur nennt auch ein
// eigenes Sample (`user:<name>`) oder ein Instrument mit Note und Laenge.
// 5 (S118, Stufe 16a, Entscheide K1 a, K3 a, K4 a): die Effekte am Clip sind
// eine Reihe von Bausteinen — `path` statt `effects`, mit dem Filter als
// viertem Baustein. Fassung 2 bis 4 werden weiter gelesen: ihre `effects`
// werden zur Reihe Distortion, Echo, Reverb.
// 6 (S124, Entscheid B1 a): ein Muster kann 1 bis 3 Takte lang sein — ein
// aelteres Programm streckte es auf 4 und haengte Stille an; es liest die Datei
// weiter, meldet aber, dass sie neuer ist.
// 7 (S124, Entscheid E1 a): der Schalter `fxOn` am Clip — ein aelteres
// Programm spielte einen abgeschalteten Clip mit Effekten.
// 8 (S132, Stufe 23a, Entscheide S-M2 a, S-M2c a): eine Note kann als fuenften
// Wert ihre Saite tragen, ein Instrument-Muster das Feld `stringChoice` — ein
// aelteres Programm verloere beides still.
// 9 (S134, Stufe 24a, Entscheide S-W2 a bis S-W4 a, S-W11 a): ein Instrument-Muster
// traegt immer `strum`, dazu `strokes`, wenn es welche gibt; eine Note als sechsten Wert ihre Artikulationen
// (dann steht die Saite als fuenfter Wert da, −1 = keine).
// 10 (S136, Stufe 25a, Entscheide S-L1 a, S-L2 a): die Artikulationen Legato (16)
// und Slide (32), eine Note als siebten Wert ihren Bend (`[[zeit, halbtoene], …]`,
// dann stehen Saite und Artikulationen davor) — ein aelteres Programm verloere
// beides still. Geschrieben wird immer 10.
constexpr int kVersion = 10;
constexpr int kVersionAelteste = 2;
const QString kKitVorsatz = QStringLiteral("kit:");
const QString kBibliothekVorsatz = QStringLiteral("lib:");
const QString kArtDrumkit = QStringLiteral("drumkit");
const QString kArtInstrument = QStringLiteral("instrument");
const QString kTonVorsatz = QStringLiteral("inst:");
const QString kInstrumentVorgabe = QStringLiteral("piano");
const QString kInstrumentEndung = QStringLiteral(".lvinst");
const QString kKitEndung = QStringLiteral(".lvkit");

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

const QString kBausteinFilter = QStringLiteral("filter");
const QString kBausteinVerzerrung = QStringLiteral("distortion");
const QString kBausteinEcho = QStringLiteral("echo");
const QString kBausteinHall = QStringLiteral("reverb");

void richteBaustein(ClipFilter& f)
{
    const ClipFilter vorgabe;
    if (f.art < FilterArt::Tiefpass || f.art > FilterArt::Bandpass) f.art = vorgabe.art;
    f.hz = inGrenzen(f.hz, kClipFilterHzMin, kClipFilterHzMax, vorgabe.hz);
    f.resonanz = inGrenzen(f.resonanz, 0.0, 1.0, vorgabe.resonanz);
}

void richteBaustein(ClipVerzerrung& v)
{
    const ClipVerzerrung vorgabe;
    v.drive = inGrenzen(v.drive, 0.0, 1.0, vorgabe.drive);
    v.ton = inGrenzen(v.ton, 0.0, 1.0, vorgabe.ton);
    v.mix = inGrenzen(v.mix, 0.0, 1.0, vorgabe.mix);
}

void richteBaustein(ClipEcho& e)
{
    const ClipEcho vorgabe;
    if (e.zeit < EchoZeit::Sechzehntel || e.zeit > EchoZeit::Halbe) e.zeit = vorgabe.zeit;
    e.feedback = inGrenzen(e.feedback, 0.0, kEchoFeedbackMax, vorgabe.feedback);
    e.daempfung = inGrenzen(e.daempfung, 0.0, 1.0, vorgabe.daempfung);
    e.mix = inGrenzen(e.mix, 0.0, 1.0, vorgabe.mix);
}

void richteBaustein(ClipHall& h)
{
    const ClipHall vorgabe;
    h.groesse = inGrenzen(h.groesse, 0.0, 1.0, vorgabe.groesse);
    h.sekunden = inGrenzen(h.sekunden, kHallSekMin, kHallSekMax, vorgabe.sekunden);
    h.daempfung = inGrenzen(h.daempfung, 0.0, 1.0, vorgabe.daempfung);
    h.mix = inGrenzen(h.mix, 0.0, 1.0, vorgabe.mix);
}

/// Die Reihe eines Clips: hoechstens `kBausteineMax` (K4 a — was dahinter
/// steht, faellt weg), jeder Wert in seinen Grenzen.
void richteEffekte(ClipEffekte& e)
{
    if (e.reihe.size() > static_cast<std::size_t>(kBausteineMax))
        e.reihe.erase(e.reihe.begin() + kBausteineMax, e.reihe.end());
    for (ClipBaustein& baustein : e.reihe)
        std::visit([](auto& werte) { richteBaustein(werte); }, baustein);
}

QJsonObject bausteinAlsJson(const ClipFilter& f)
{
    QJsonObject v;
    v[QStringLiteral("type")] = kBausteinFilter;
    v[QStringLiteral("on")] = f.an;
    v[QStringLiteral("filterType")] = filterArtName(f.art);
    v[QStringLiteral("cutoff")] = f.hz;
    v[QStringLiteral("resonance")] = f.resonanz;
    return v;
}

QJsonObject bausteinAlsJson(const ClipVerzerrung& w)
{
    QJsonObject v;
    v[QStringLiteral("type")] = kBausteinVerzerrung;
    v[QStringLiteral("on")] = w.an;
    v[QStringLiteral("drive")] = w.drive;
    v[QStringLiteral("tone")] = w.ton;
    v[QStringLiteral("mix")] = w.mix;
    return v;
}

QJsonObject bausteinAlsJson(const ClipEcho& e)
{
    QJsonObject v;
    v[QStringLiteral("type")] = kBausteinEcho;
    v[QStringLiteral("on")] = e.an;
    v[QStringLiteral("time")] = echoZeitName(e.zeit);
    v[QStringLiteral("feedback")] = e.feedback;
    v[QStringLiteral("damping")] = e.daempfung;
    v[QStringLiteral("mix")] = e.mix;
    return v;
}

QJsonObject bausteinAlsJson(const ClipHall& h)
{
    QJsonObject v;
    v[QStringLiteral("type")] = kBausteinHall;
    v[QStringLiteral("on")] = h.an;
    v[QStringLiteral("size")] = h.groesse;
    v[QStringLiteral("decay")] = h.sekunden;
    v[QStringLiteral("damping")] = h.daempfung;
    v[QStringLiteral("mix")] = h.mix;
    return v;
}

/// Die Reihe als Liste `path`, von vorn nach hinten. Ein Clip ohne Bausteine
/// traegt das Feld nicht; ein ausgeschalteter Baustein steht mit seinen Werten da.
QJsonArray reiheAlsJson(const ClipEffekte& e)
{
    QJsonArray reihe;
    for (const ClipBaustein& baustein : e.reihe)
        reihe.append(std::visit([](const auto& werte) { return bausteinAlsJson(werte); }, baustein));
    return reihe;
}

ClipFilter filterAusJson(const QJsonObject& v)
{
    ClipFilter f;
    f.an = v.value(QStringLiteral("on")).toBool(false);
    f.art = filterArtAus(v.value(QStringLiteral("filterType")).toString());
    f.hz = v.value(QStringLiteral("cutoff")).toDouble(f.hz);
    f.resonanz = v.value(QStringLiteral("resonance")).toDouble(f.resonanz);
    return f;
}

ClipVerzerrung verzerrungAusJson(const QJsonObject& v)
{
    ClipVerzerrung w;
    w.an = v.value(QStringLiteral("on")).toBool(false);
    w.drive = v.value(QStringLiteral("drive")).toDouble(w.drive);
    w.ton = v.value(QStringLiteral("tone")).toDouble(w.ton);
    w.mix = v.value(QStringLiteral("mix")).toDouble(w.mix);
    return w;
}

ClipEcho echoAusJson(const QJsonObject& v, QStringList* bericht)
{
    ClipEcho e;
    e.an = v.value(QStringLiteral("on")).toBool(false);
    if (const QJsonValue zeit = v.value(QStringLiteral("time")); !zeit.isUndefined())
    {
        const QString name = zeit.toString();
        const auto* treffer = std::find_if(std::begin(kEchoZeiten), std::end(kEchoZeiten),
                                           [&name](const ZeitName& z) { return name == QLatin1String(z.name); });
        if (treffer != std::end(kEchoZeiten))
            e.zeit = treffer->zeit;
        else if (bericht != nullptr)
        {
            const QString zeile = QStringLiteral("Unknown echo time '%1'").arg(name);
            if (!bericht->contains(zeile)) bericht->append(zeile);
        }
    }
    e.feedback = v.value(QStringLiteral("feedback")).toDouble(e.feedback);
    e.daempfung = v.value(QStringLiteral("damping")).toDouble(e.daempfung);
    e.mix = v.value(QStringLiteral("mix")).toDouble(e.mix);
    return e;
}

ClipHall hallAusJson(const QJsonObject& v)
{
    ClipHall h;
    h.an = v.value(QStringLiteral("on")).toBool(false);
    h.groesse = v.value(QStringLiteral("size")).toDouble(h.groesse);
    h.sekunden = v.value(QStringLiteral("decay")).toDouble(h.sekunden);
    h.daempfung = v.value(QStringLiteral("damping")).toDouble(h.daempfung);
    h.mix = v.value(QStringLiteral("mix")).toDouble(h.mix);
    return h;
}

/// Die Liste `path` (seit Fassung 5). Einen Baustein, den dieses Programm
/// nicht kennt, laesst es weg — das steht im Bericht.
ClipEffekte reiheAusJson(const QJsonArray& reihe, QStringList* bericht)
{
    ClipEffekte e;
    for (const QJsonValue& eintrag : reihe)
    {
        const QJsonObject v = eintrag.toObject();
        const QString art = v.value(QStringLiteral("type")).toString();
        if (art == kBausteinFilter)
            e.reihe.emplace_back(filterAusJson(v));
        else if (art == kBausteinVerzerrung)
            e.reihe.emplace_back(verzerrungAusJson(v));
        else if (art == kBausteinEcho)
            e.reihe.emplace_back(echoAusJson(v, bericht));
        else if (art == kBausteinHall)
            e.reihe.emplace_back(hallAusJson(v));
        else if (bericht != nullptr)
        {
            const QString zeile = QStringLiteral("Unknown effect '%1'").arg(art);
            if (!bericht->contains(zeile)) bericht->append(zeile);
        }
    }
    return e;
}

/// Das Feld `effects` bis Fassung 4: drei feste Effekte, von denen nur in der
/// Datei stand, was von der Vorgabe abwich. Daraus wird die Reihe in der
/// Folge, in der sie gerechnet wurden — Distortion, Echo, Reverb; was nicht
/// in der Datei stand, wird kein Baustein.
ClipEffekte effekteAusJson(const QJsonObject& o, QStringList* bericht)
{
    ClipEffekte e;
    if (const QJsonValue wert = o.value(kBausteinVerzerrung); wert.isObject())
        e.reihe.emplace_back(verzerrungAusJson(wert.toObject()));
    if (const QJsonValue wert = o.value(kBausteinEcho); wert.isObject())
        e.reihe.emplace_back(echoAusJson(wert.toObject(), bericht));
    if (const QJsonValue wert = o.value(kBausteinHall); wert.isObject())
        e.reihe.emplace_back(hallAusJson(wert.toObject()));
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

bool istBibliotheksKlang(const QString& klang)
{
    return klang.startsWith(kBibliothekVorsatz);
}

bool istMitgelieferterKlang(const QString& klang)
{
    return istKitKlang(klang) || istBibliotheksKlang(klang);
}

QString pfadFuerDatei(const QString& datei, const QString& basisOrdner)
{
    return fuerDatei(datei, basisOrdner);
}

QString pfadAusDatei(const QString& eintrag, const QString& basisOrdner)
{
    return ausDatei(eintrag, basisOrdner);
}

bool istEigenerKlang(const QString& klang)
{
    return istEigenesSample(klang);
}

QString Spur::spielt() const
{
    return instrument.isEmpty() ? klang : tonKlang(instrument, hoehe, dauerMs);
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

QString tonKlang(const TonAngabe& ton)
{
    QString name = tonKlang(ton.instrument, ton.hoehe, ton.dauerMs);
    if (ton.saite >= 0) name += QStringLiteral(":s%1").arg(ton.saite);
    if (ton.teilton > 1) name += QStringLiteral(":h%1").arg(ton.teilton);  // seit Stufe 24b
    if (ton.richtung == Richtung::Auf) name += QStringLiteral(":u");
    if ((ton.artikulationen & kPalmMute) != 0) name += QStringLiteral(":m");
    if ((ton.artikulationen & kTapping) != 0) name += QStringLiteral(":t");
    // seit Stufe 25b (S-L6 a): der Verlauf und der Bend, lesbar
    if (!ton.verlauf.empty())
    {
        QStringList teile;
        for (const TonWechsel& w : ton.verlauf)
            teile << QStringLiteral("%1%2%3").arg(w.ms).arg(w.gleitet ? QLatin1Char('g') : QLatin1Char('l')).arg(w.hoehe);
        name += QStringLiteral(":v") + teile.join(QLatin1Char(','));
    }
    if (!ton.biegung.empty())
    {
        QStringList teile;
        for (const TonBiegung& b : ton.biegung) teile << QStringLiteral("%1-%2").arg(b.ms).arg(b.viertel);
        name += QStringLiteral(":b") + teile.join(QLatin1Char(','));
    }
    if (ton.endeMs > 0) name += QStringLiteral(":e%1").arg(ton.endeMs);
    return name;
}

namespace
{

/// `:v240l59,480g62` — je Wechsel ms, `l` oder `g`, die Tonhoehe.
bool zerlegeVerlauf(QStringView text, std::vector<TonWechsel>& verlauf)
{
    for (const QStringView teil : text.split(QLatin1Char(',')))
    {
        qsizetype art = 0;
        while (art < teil.size() && teil[art].isDigit()) ++art;
        if (art == 0 || art + 1 >= teil.size()) return false;
        const QChar zeichen = teil[art];
        if (zeichen != QLatin1Char('l') && zeichen != QLatin1Char('g')) return false;
        bool msOk = false;
        bool hoeheOk = false;
        const int ms = teil.left(art).toInt(&msOk);
        const int hoehe = teil.mid(art + 1).toInt(&hoeheOk);
        if (!msOk || !hoeheOk) return false;
        verlauf.push_back(TonWechsel{ms, hoehe, zeichen == QLatin1Char('g')});
    }
    return !verlauf.empty();
}
/// `:b0-0,90-8` — je Punkt ms und Viertel.
bool zerlegeBiegung(QStringView text, std::vector<TonBiegung>& biegung)
{
    for (const QStringView teil : text.split(QLatin1Char(',')))
    {
        const qsizetype strich = teil.indexOf(QLatin1Char('-'));
        if (strich <= 0) return false;
        bool msOk = false;
        bool viertelOk = false;
        const int ms = teil.left(strich).toInt(&msOk);
        const int viertel = teil.mid(strich + 1).toInt(&viertelOk);
        if (!msOk || !viertelOk || ms < 0 || viertel < 0) return false;
        biegung.push_back(TonBiegung{ms, viertel});
    }
    return !biegung.empty();
}

/// Den Namen eines Tons zerlegen (ohne Vorsatz). Von hinten: die Zusaetze
/// `:e<ms>`, `:s<n>` (seit Stufe 23b), `:h<n>`, `:u`, `:m`, `:t` (seit Stufe
/// 24b), `:v…`, `:b…` (seit Stufe 25b), dann Dauer und Tonhoehe als reine
/// Zahlen — davor das Instrument, das als Pfad selbst einen Doppelpunkt tragen
/// kann (C:/…).
bool zerlegeTonRest(QString rest, TonAngabe& ton)
{
    TonAngabe zusatz;
    for (int runde = 0; runde < 8; ++runde)
    {
        const qsizetype vor = rest.lastIndexOf(QLatin1Char(':'));
        if (vor < 0 || vor + 2 > rest.size()) break;
        const QStringView stueck = QStringView(rest).mid(vor + 1);
        if (stueck == QLatin1String("u"))
            zusatz.richtung = Richtung::Auf;
        else if (stueck == QLatin1String("m"))
            zusatz.artikulationen |= kPalmMute;
        else if (stueck == QLatin1String("t"))
            zusatz.artikulationen |= kTapping;
        else if (stueck.front() == QLatin1Char('v') && stueck.size() > 1 && stueck[1].isDigit())
        {
            if (!zerlegeVerlauf(stueck.mid(1), zusatz.verlauf)) return false;
        }
        else if (stueck.front() == QLatin1Char('b') && stueck.size() > 1 && stueck[1].isDigit())
        {
            if (!zerlegeBiegung(stueck.mid(1), zusatz.biegung)) return false;
        }
        else
        {
            const QChar art = stueck.front();
            if (art != QLatin1Char('s') && art != QLatin1Char('e') && art != QLatin1Char('h')) break;
            bool ok = false;
            const int wert = stueck.mid(1).toInt(&ok);
            if (!ok || wert < 0) return false;
            (art == QLatin1Char('s') ? zusatz.saite : art == QLatin1Char('e') ? zusatz.endeMs : zusatz.teilton) = wert;
        }
        rest.truncate(vor);
    }
    const qsizetype vorDauer = rest.lastIndexOf(QLatin1Char(':'));
    const qsizetype vorHoehe = vorDauer > 0 ? rest.lastIndexOf(QLatin1Char(':'), vorDauer - 1) : -1;
    if (vorHoehe <= 0) return false;
    bool hoeheOk = false;
    bool dauerOk = false;
    const int h = rest.mid(vorHoehe + 1, vorDauer - vorHoehe - 1).toInt(&hoeheOk);
    const int ms = rest.mid(vorDauer + 1).toInt(&dauerOk);
    if (!hoeheOk || !dauerOk || ms <= 0) return false;
    zusatz.instrument = rest.left(vorHoehe);
    zusatz.hoehe = h;
    zusatz.dauerMs = ms;
    ton = std::move(zusatz);
    return true;
}

}  // namespace

bool zerlegeTon(const QString& klang, TonAngabe& ton)
{
    return istTon(klang) && zerlegeTonRest(klang.mid(kTonVorsatz.size()), ton);
}

bool zerlegeTon(const QString& klang, QString& instrument, int& hoehe, int& dauerMs)
{
    TonAngabe ton;
    if (!zerlegeTon(klang, ton)) return false;
    instrument = ton.instrument;
    hoehe = ton.hoehe;
    dauerMs = ton.dauerMs;
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

double Loop::schritteVonMs(double ms) const
{
    const double dauer = durchlaufMs();
    return dauer > 0.0 ? ms / dauer * durchlaufSchritte() : 0.0;
}

double Loop::stelleImMuster(double schritt, const QString& name) const
{
    const std::vector<double> stellen = stellenImMuster(schritt, name);
    return stellen.empty() ? -1.0 : stellen.front();
}

std::vector<double> Loop::stellenImMuster(double schritt, const QString& name) const
{
    const Muster* m = findeMuster(name);
    if (m == nullptr || schritt < 0.0) return {};
    const int spt = schritteJeTakt();
    const double musterSchritte = static_cast<double>(m->takte) * spt;
    if (musterSchritte <= 0.0) return {};
    struct Treffer
    {
        int startTakt;
        double stelle;
    };
    // eingeschaltete Kanaele zuerst: was man hoert, geht vor — ein ausgeschalteter nur,
    // wenn sonst keiner dort liegt
    for (const bool nurAn : {true, false})
    {
        std::vector<Treffer> treffer;
        for (const Kanal& kanal : kanaele)
        {
            if (nurAn != kanal.an) continue;
            for (const LoopClip& clip : kanal.clips)
            {
                if (clip.muster != name) continue;
                const double anfang = static_cast<double>(clip.startTakt) * spt;
                const double ende = anfang + static_cast<double>(clipTakte(clip)) * spt;
                if (schritt >= anfang && schritt < ende)
                    treffer.push_back({clip.startTakt, std::fmod(schritt - anfang, musterSchritte)});
            }
        }
        if (treffer.empty()) continue;
        if (!nurAn) return {treffer.front().stelle};
        // W8-2 a: der zuletzt begonnene zuerst — bei gleichem Anfang bleibt der obere Kanal vorn
        std::stable_sort(treffer.begin(), treffer.end(),
                         [](const Treffer& a, const Treffer& b) { return a.startTakt > b.startTakt; });
        std::vector<double> stellen;
        stellen.reserve(treffer.size());
        for (const Treffer& t : treffer) stellen.push_back(t.stelle);
        return stellen;
    }
    return {};
}

QStringList Loop::klangNamen(const SaitenVon& saiten) const
{
    QStringList namen;
    QSet<QString> gesehen;
    const auto nimm = [&](const QString& klang) {
        if (klang.isEmpty() || gesehen.contains(klang)) return;
        gesehen.insert(klang);
        namen << klang;
    };
    // die Toene der Saiteninstrumente je Vorkommen (Stufe 23b) — dieselbe Rechnung wie `plane`
    std::vector<std::vector<ClipToene>> jeKanal;
    if (saiten)
        for (std::size_t k = 0; k < kanaele.size(); ++k) jeKanal.push_back(toeneImKanal(*this, k, saiten));
    for (const Muster& m : muster)
    {
        if (m.art != MusterArt::Instrument)
        {
            for (const Spur& spur : m.spuren) nimm(spur.spielt());
            continue;
        }
        if (!saiten || !saiten(m.instrument).istSaiteninstrument())
        {
            for (const Note& note : m.noten) nimm(tonKlang(m.instrument, note.hoehe, notenMs(note)));
            continue;
        }
        for (std::size_t k = 0; k < kanaele.size(); ++k)
            for (std::size_t c = 0; c < kanaele[k].clips.size(); ++c)
            {
                if (kanaele[k].clips[c].muster != m.name) continue;
                for (const std::vector<GeplanterTon>& runde : jeKanal[k][c])
                    for (const TonAngabe& ton : runde) nimm(tonKlang(ton));
            }
    }
    return namen;
}

int Loop::notenMs(const Note& note) const
{
    if (bpm <= 0.0 || schritteJeSchlag <= 0) return 1;
    return std::max(1, static_cast<int>(std::lround(note.laenge * 60000.0 / bpm / schritteJeSchlag)));
}

const Anschlag* anschlagAn(const Muster& muster, int schritt)
{
    for (const Anschlag& anschlag : muster.anschlaege)
        if (anschlag.schritt == schritt) return &anschlag;
    return nullptr;
}

void setzeAnschlag(Muster& muster, const Anschlag& anschlag)
{
    std::vector<Anschlag>& liste = muster.anschlaege;
    liste.erase(std::remove_if(liste.begin(), liste.end(),
                               [&anschlag](const Anschlag& a) { return a.schritt == anschlag.schritt; }),
                liste.end());
    if (anschlag.weichtAb()) liste.push_back(anschlag);
    richteAnschlaege(muster);
}

void richteAnschlaege(Muster& muster)
{
    std::vector<Anschlag>& liste = muster.anschlaege;
    for (Anschlag& anschlag : liste)
    {
        // −1 heisst „wie das Muster"; was keine Zahl oder negativ ist, auch
        anschlag.spreadMs = std::isfinite(anschlag.spreadMs) && anschlag.spreadMs >= 0.0
                                ? std::min(anschlag.spreadMs, kSpreadMaxMs)
                                : -1.0;
        const int art = static_cast<int>(anschlag.art);
        if (art < static_cast<int>(Anschlagart::Hand) || art > static_cast<int>(Anschlagart::Zugleich))
            anschlag.art = Anschlagart::Hand;
    }
    const auto ohneNote = [&muster](const Anschlag& anschlag) {
        return std::none_of(muster.noten.begin(), muster.noten.end(),
                            [&anschlag](const Note& note) { return note.schritt == anschlag.schritt; });
    };
    liste.erase(std::remove_if(liste.begin(), liste.end(),
                               [&ohneNote](const Anschlag& a) { return !a.weichtAb() || ohneNote(a); }),
                liste.end());
    std::stable_sort(liste.begin(), liste.end(),
                     [](const Anschlag& a, const Anschlag& b) { return a.schritt < b.schritt; });
    liste.erase(std::unique(liste.begin(), liste.end(),
                            [](const Anschlag& a, const Anschlag& b) { return a.schritt == b.schritt; }),
                liste.end());
}

void uebernimmVorgaben(Muster& muster, const Saitensatz& saiten)
{
    muster.saitenwahl = saiten.wahl;
    muster.strum = saiten.strum;
}

void wechsleVorgaben(Muster& muster, const Saitensatz& alt, const Saitensatz& neu)
{
    // ohne Saiten gelten die leeren Vorgaben — so wird eine neue Basis mit der
    // ersten Gitarre zu einer Gitarre mit deren Vorgaben
    const Saitensatz ohne;
    const Saitensatz& vorher = alt.istSaiteninstrument() ? alt : ohne;
    const Saitensatz& nachher = neu.istSaiteninstrument() ? neu : ohne;
    if (muster.saitenwahl == vorher.wahl) muster.saitenwahl = nachher.wahl;
    if (muster.strum == vorher.strum) muster.strum = nachher.strum;
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
                if (note.saite < -1 || note.saite >= kSaitenMax) note.saite = -1;
                note.artikulationen &= kArtikulationenAlle;
                richteBend(note.bend);
            }
            if (m.saitenwahl != Saitenwahl::Hand) m.saitenwahl = Saitenwahl::JeKlang;
            richteStrum(m.strum, handStellen(m.strum.raster, schlaegeJeTakt, schritteJeSchlag));
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
            richteAnschlaege(m);
            continue;
        }
        m.instrument.clear();
        m.noten.clear();
        m.saitenwahl = Saitenwahl::JeKlang;
        m.strum = Strum{};
        m.anschlaege.clear();
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
            // je Note eine Zeile: Schritt, Laenge, Tonhoehe, Lautstaerke — seit
            // Fassung 8 als fuenfter Wert die Saite, nur wenn die Note eine traegt;
            // seit Fassung 9 als sechster die Artikulationen (die Saite dann als −1),
            // seit Fassung 10 als siebter der Bend (Saite und Artikulationen dann davor)
            QJsonArray noten;
            for (const Note& note : m.noten)
            {
                QJsonArray zeile{note.schritt, note.laenge, note.hoehe, note.laut};
                const bool mitBend = !note.bend.empty();
                if (note.saite >= 0 || note.artikulationen != 0 || mitBend) zeile.append(note.saite);
                if (note.artikulationen != 0 || mitBend) zeile.append(note.artikulationen);
                if (mitBend) zeile.append(bendAlsJson(note.bend));
                noten.append(zeile);
            }
            o[QStringLiteral("notes")] = noten;
            // seit Fassung 8; Per chord ist die Vorgabe und steht nicht da
            if (m.saitenwahl != Saitenwahl::JeKlang)
                o[QStringLiteral("stringChoice")] = saitenwahlName(m.saitenwahl);
            // seit Fassung 9, immer (S-W11 a): die Datei klingt gleich, wie immer die Vorgabe spaeter lautet
            o[QStringLiteral("strum")] = strumAlsJson(m.strum);
            if (!m.anschlaege.empty())
            {
                QJsonArray anschlaege;
                for (const Anschlag& anschlag : m.anschlaege)
                {
                    QJsonObject a{{QStringLiteral("step"), anschlag.schritt},
                                  {QStringLiteral("stroke"), anschlagartName(anschlag.art)}};
                    if (anschlag.spreadMs >= 0.0) a[QStringLiteral("spread")] = anschlag.spreadMs;
                    anschlaege.append(a);
                }
                o[QStringLiteral("strokes")] = anschlaege;
            }
            muster.append(o);
            continue;
        }
        o[QStringLiteral("kind")] = kArtDrumkit;
        QJsonArray spuren;
        for (const Spur& spur : m.spuren)
        {
            QJsonObject s;
            s[QStringLiteral("name")] = spur.name;
            s[QStringLiteral("sound")] = istMitgelieferterKlang(spur.klang) || istEigenerKlang(spur.klang)
                                             ? spur.klang
                                             : fuerDatei(spur.klang, basisOrdner);
            s[QStringLiteral("volume")] = spur.laut;
            s[QStringLiteral("pan")] = spur.pan;
            s[QStringLiteral("mute")] = spur.stumm;
            if (!spur.instrument.isEmpty())
            {
                // seit Fassung 4; `sound` bleibt daneben stehen — ein aelteres Programm spielt dann den
                s[QStringLiteral("instrument")] =
                    istInstrumentDatei(spur.instrument) ? fuerDatei(spur.instrument, basisOrdner) : spur.instrument;
                s[QStringLiteral("note")] = spur.hoehe;
                s[QStringLiteral("length")] = spur.dauerMs;
            }
            // seit S123 (Entscheid K1 a): freiwillig, ein aelteres Programm liest es nicht
            if (!spur.rolle.isEmpty()) s[QStringLiteral("role")] = spur.rolle;
            spuren.append(s);
        }
        o[QStringLiteral("tracks")] = spuren;
        // seit S123 (Entscheid K4 a): das zuletzt angewendete Kit, freiwillig
        if (!m.kit.isEmpty())
            o[QStringLiteral("kit")] = m.kit.endsWith(kKitEndung, Qt::CaseInsensitive) ? fuerDatei(m.kit, basisOrdner) : m.kit;
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
            if (!clip.effekte.reihe.empty()) c[QStringLiteral("path")] = reiheAlsJson(clip.effekte);
            if (!clip.effekteAn) c[QStringLiteral("fxOn")] = false;  // seit Fassung 7
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
        m.takte = o.value(QStringLiteral("bars")).toInt(kTakteNeu);
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
                note.saite = werte.size() > 4 ? werte[4].toInt(-1) : -1;  // seit Fassung 8
                note.artikulationen = werte.size() > 5 ? werte[5].toInt(0) : 0;  // seit Fassung 9
                if (werte.size() > 6) note.bend = bendAusJson(werte[6].toArray());  // seit Fassung 10
                m.noten.push_back(std::move(note));
            }
            m.saitenwahl = saitenwahlAusName(o.value(QStringLiteral("stringChoice")).toString());
            // seit Fassung 9; fehlt es, gilt die Vorgabe
            m.strum = strumAusJson(o.value(QStringLiteral("strum")).toObject());
            for (const QJsonValue& av : o.value(QStringLiteral("strokes")).toArray())
            {
                const QJsonObject a = av.toObject();
                Anschlag anschlag;
                anschlag.schritt = a.value(QStringLiteral("step")).toInt(-1);
                anschlag.art = anschlagartAusName(a.value(QStringLiteral("stroke")).toString());
                anschlag.spreadMs = a.value(QStringLiteral("spread")).toDouble(-1.0);
                m.anschlaege.push_back(anschlag);
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
            spur.klang = istMitgelieferterKlang(klang) || istEigenerKlang(klang) ? klang : ausDatei(klang, basisOrdner);
            spur.laut = s.value(QStringLiteral("volume")).toDouble(1.0);
            spur.pan = s.value(QStringLiteral("pan")).toDouble(0.0);
            spur.stumm = s.value(QStringLiteral("mute")).toBool(false);
            // seit Fassung 4: ein Instrument mit Note und Laenge
            spur.instrument = s.value(QStringLiteral("instrument")).toString();
            if (istInstrumentDatei(spur.instrument)) spur.instrument = ausDatei(spur.instrument, basisOrdner);
            spur.hoehe = std::clamp(s.value(QStringLiteral("note")).toInt(spur.hoehe), 0, 127);
            spur.dauerMs = std::clamp(s.value(QStringLiteral("length")).toInt(spur.dauerMs), kSpurDauerMinMs, kSpurDauerMaxMs);
            // seit S123: die Rolle im Kit, so wie sie dasteht (geprueft wird sie beim Kitwechsel, DrumKit.hpp)
            spur.rolle = s.value(QStringLiteral("role")).toString().trimmed().toLower();
            if (bericht != nullptr)
            {
                // was fehlt, steht im Bericht — fuer den Klang nur, wenn die Spur ihn auch spielt
                QString zeile;
                if (!spur.instrument.isEmpty())
                {
                    if (istInstrumentDatei(spur.instrument) && !QFileInfo::exists(spur.instrument))
                        zeile = QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(spur.instrument));
                }
                else if (istEigenerKlang(spur.klang) || istBibliotheksKlang(spur.klang))
                {
                    const QString datei = sampleDatei(spur.klang);
                    if (datei.isEmpty() || !QFileInfo::exists(datei))
                        zeile = QStringLiteral("Sample not found: %1").arg(spur.klang);
                }
                else if (!spur.klang.isEmpty() && !istKitKlang(spur.klang) && !QFileInfo::exists(spur.klang))
                    zeile = QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(spur.klang));
                if (!zeile.isEmpty() && !bericht->contains(zeile)) bericht->append(zeile);
            }
            m.spuren.push_back(std::move(spur));
        }
        for (const QJsonValue& zv : o.value(QStringLiteral("rows")).toArray())
        {
            std::vector<double> zeile;
            for (const QJsonValue& wert : zv.toArray()) zeile.push_back(wert.toDouble());
            m.zeilen.push_back(std::move(zeile));
        }
        // seit S123: das zuletzt angewendete Kit
        m.kit = o.value(QStringLiteral("kit")).toString();
        if (m.kit.endsWith(kKitEndung, Qt::CaseInsensitive)) m.kit = ausDatei(m.kit, basisOrdner);
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
            clip.effekteAn = c.value(QStringLiteral("fxOn")).toBool(true);
            // seit Fassung 5 die Reihe `path`; davor die drei festen `effects`
            if (const QJsonValue reihe = c.value(QStringLiteral("path")); reihe.isArray())
                clip.effekte = reiheAusJson(reihe.toArray(), bericht);
            else
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

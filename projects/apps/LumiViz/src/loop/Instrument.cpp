/**
 * @file Instrument.cpp
 * @brief Loop: gerechnete Instrumente (Stufe 9)
 */

#include "loop/Instrument.hpp"

#include "loop/Kit.hpp"
#include "loop/Sample.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>

namespace lumi::loop
{

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kTafel = 2048;        ///< Abtastwerte eines Wellenzugs
constexpr int kTeiltoeneMax = 512;  ///< mehr traegt die Tafel nicht sauber
constexpr double kPegel = 0.5;      ///< Luft nach oben: ein Akkord addiert sich (Entscheid L6 a)

/// Der Anteil des k-ten Teiltons (k ab 1) an der Schwingung.
double teilton(const Schicht& schicht, int k)
{
    switch (schicht.schwingung)
    {
    case Schwingung::Sinus:
        if (k == 1) return 1.0;
        return k - 2 < static_cast<int>(schicht.obertoene.size()) ? schicht.obertoene[static_cast<std::size_t>(k - 2)]
                                                                     : 0.0;
    case Schwingung::Dreieck:
        return k % 2 == 1 ? ((k / 2) % 2 == 0 ? 1.0 : -1.0) / (static_cast<double>(k) * k) : 0.0;
    case Schwingung::Saege:
        return 1.0 / k;
    case Schwingung::Rechteck:
        return k % 2 == 1 ? 1.0 / k : 0.0;
    case Schwingung::Saite:
    case Schwingung::Sample:
        break;  // kein Wellenzug: die Saite und die Aufnahme rechnen eigens
    }
    return 0.0;
}

/// Ein Wellenzug der Schwingung, ohne Teiltoene ueber `hoechsterTeilton`, auf
/// den Betrag 1 gebracht. Ein Wert mehr als `kTafel`: der erste noch einmal.
std::vector<float> wellenzug(const Schicht& schicht, int hoechsterTeilton)
{
    std::vector<double> summe(kTafel, 0.0);
    for (int k = 1; k <= hoechsterTeilton; ++k)
    {
        const double anteil = teilton(schicht, k);
        if (anteil == 0.0) continue;
        for (int i = 0; i < kTafel; ++i) summe[static_cast<std::size_t>(i)] += anteil * std::sin(2.0 * kPi * k * i / kTafel);
    }
    double spitze = 0.0;
    for (const double wert : summe) spitze = std::max(spitze, std::abs(wert));
    std::vector<float> tafel(kTafel + 1, 0.0f);
    if (spitze > 0.0)
        for (int i = 0; i < kTafel; ++i) tafel[static_cast<std::size_t>(i)] = static_cast<float>(summe[static_cast<std::size_t>(i)] / spitze);
    tafel[kTafel] = tafel[0];
    return tafel;
}

/// Die Huellkurve: Anstieg, Abfall auf `halten`, nach dem Ende der Note der
/// Ausklang — der am letzten Abtastwert bei null ankommt.
struct Huelle
{
    double anstieg = 1.0;   ///< in Abtastwerten
    double abfall = 1.0;    ///< Zeitkonstante des Abfalls
    double halten = 1.0;
    std::int64_t ende = 0;  ///< hier endet die Note
    std::int64_t aus = 1;   ///< so lang ist der Ausklang
    double ausklang = 1.0;  ///< seine Zeitkonstante

    [[nodiscard]] double gehalten(std::int64_t t) const
    {
        if (t < anstieg) return static_cast<double>(t) / anstieg;
        return halten + (1.0 - halten) * std::exp(-(static_cast<double>(t) - anstieg) / abfall);
    }
    [[nodiscard]] double bei(std::int64_t t) const
    {
        if (t < ende) return gehalten(t);
        const double nach = static_cast<double>(t - ende);
        if (nach >= static_cast<double>(aus)) return 0.0;
        return gehalten(ende) * std::exp(-nach / ausklang) * (1.0 - nach / static_cast<double>(aus));
    }
};

Huelle huelleFuer(const Schicht& schicht, double dauerMs, int rate)
{
    const double jeMs = rate / 1000.0;
    Huelle h;
    h.anstieg = std::max(1.0, schicht.anstiegMs * jeMs);
    h.abfall = std::max(1.0, schicht.abfallMs * jeMs / 3.0);
    h.halten = std::clamp(schicht.halten, 0.0, 1.0);
    h.ende = std::max<std::int64_t>(1, std::llround(dauerMs * jeMs));
    h.aus = std::max<std::int64_t>(1, std::llround(std::max(5.0, schicht.ausklangMs) * jeMs));
    h.ausklang = std::max(1.0, static_cast<double>(h.aus) / 4.0);
    return h;
}

/// Der Verzerrer: formt den Augenblickswert um, ohne Gedaechtnis. Der Betrag
/// bleibt bei hoechstens 1; mit `staerke` gegen 0 geht er in nichts ueber.
struct Verzerrer
{
    double staerke = 0.0;  ///< 0 = aus
    double norm = 1.0;

    explicit Verzerrer(double verzerrung)
    {
        staerke = 8.0 * std::clamp(verzerrung, 0.0, 1.0);
        if (staerke > 0.0) norm = 1.0 / std::tanh(staerke);
    }
    [[nodiscard]] double bei(double wert) const { return staerke > 0.0 ? std::tanh(staerke * wert) * norm : wert; }
};

void rechneSchwingung(const Schicht& schicht, double hz, const Huelle& huelle, int rate, std::vector<float>& daten)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    // Vibrato und Verstimmung heben den Ton leicht: etwas Abstand zur halben Abtastrate
    const int hoechster = std::clamp(static_cast<int>(rate / 2.0 / (hz * 1.06)), 1, kTeiltoeneMax);
    const std::vector<float> tafel = wellenzug(schicht, hoechster);
    const Verzerrer verzerrer(schicht.verzerrung);

    const int stimmen = std::clamp(schicht.stimmen, 1, 3);
    std::array<double, 3> verhaeltnis = {1.0, 1.0, 1.0};
    std::array<double, 3> phase = {0.0, 0.33, 0.67};  // fest: derselbe Ton ergibt dieselben Werte
    if (stimmen == 2) verhaeltnis = {std::exp2(-schicht.verstimmungCent / 1200.0), std::exp2(schicht.verstimmungCent / 1200.0), 1.0};
    if (stimmen == 3) verhaeltnis = {1.0, std::exp2(-schicht.verstimmungCent / 1200.0), std::exp2(schicht.verstimmungCent / 1200.0)};
    const double jeStimme = 1.0 / std::sqrt(static_cast<double>(stimmen));

    const double einsatz = std::max(1.0, schicht.vibratoEinsatzMs * rate / 1000.0);
    const bool vibrato = schicht.vibratoHz > 0.0 && schicht.vibratoCent > 0.0;
    const bool filter = schicht.filterTeilton > 0.0;
    double f1 = 0.0;
    double f2 = 0.0;
    const double pegel = kPegel * schicht.laut;

    for (std::int64_t t = 0; t < frames; ++t)
    {
        const double h = huelle.bei(t);
        double schritt = hz / rate;
        if (vibrato)
        {
            const double tiefe = schicht.vibratoCent * std::min(1.0, static_cast<double>(t) / einsatz);
            schritt *= std::exp2(tiefe * std::sin(2.0 * kPi * schicht.vibratoHz * static_cast<double>(t) / rate) / 1200.0);
        }
        double wert = 0.0;
        for (int s = 0; s < stimmen; ++s)
        {
            double& p = phase[static_cast<std::size_t>(s)];
            const double ort = p * kTafel;
            const auto i = static_cast<std::size_t>(ort);
            const double rest = ort - static_cast<double>(i);
            wert += tafel[i] + (tafel[i + 1] - tafel[i]) * rest;
            p += schritt * verhaeltnis[static_cast<std::size_t>(s)];
            p -= std::floor(p);
        }
        wert *= jeStimme;
        if (filter)
        {
            const double grenze =
                std::clamp(hz * (schicht.filterTeilton + schicht.filterHuelle * h), 40.0, rate * 0.45);
            const double a = 1.0 - std::exp(-2.0 * kPi * grenze / rate);
            f1 += a * (wert - f1);
            f2 += a * (f1 - f2);
            wert = f2;
        }
        wert = verzerrer.bei(wert);
        const auto aus = static_cast<float>(wert * h * pegel);
        daten[static_cast<std::size_t>(2 * t)] = aus;
        daten[static_cast<std::size_t>(2 * t + 1)] = aus;
    }
}

/// Die gezupfte Saite (Karplus-Strong): ein Stoss Rauschen laeuft durch eine
/// Verzoegerung von einer Schwingung Laenge und wird bei jedem Umlauf weicher
/// und leiser.
void rechneSaite(const Schicht& schicht, int hoehe, double hz, const Huelle& huelle, int rate,
                 std::vector<float>& daten)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    const Verzerrer verzerrer(schicht.verzerrung);
    // das Mitteln zweier Werte verzoegert um einen halben Abtastwert
    const double verzug = std::max(2.0, rate / hz - 0.5);
    const auto groesse = static_cast<std::size_t>(verzug) + 4;
    std::vector<double> ring(groesse, 0.0);
    // der Stoss: Rauschen aus einer festen Folge je Tonhoehe
    std::uint32_t folge = 12345u + static_cast<std::uint32_t>(hoehe) * 7919u;
    double davorRauschen = 0.0;
    for (double& wert : ring)
    {
        folge = folge * 1664525u + 1013904223u;
        const double rauschen = static_cast<double>(folge >> 8) / 8388608.0 - 1.0;
        wert = 0.5 * (rauschen + davorRauschen);
        davorRauschen = rauschen;
    }
    // der Verlust je Umlauf so, dass alle Tonhoehen aehnlich lang klingen
    // (`saiteDaempfung` gilt bei 220 Hz)
    const double daempfung = std::clamp(schicht.saiteDaempfung, 0.9, 0.9999);
    const double jeUmlauf = std::exp(std::log(daempfung) * 220.0 / hz);
    const double pegel = kPegel * schicht.laut;
    const double anstieg = std::max(1.0, schicht.anstiegMs * rate / 1000.0);

    std::size_t schreib = 0;
    double davor = 0.0;
    for (std::int64_t t = 0; t < frames; ++t)
    {
        double lese = static_cast<double>(schreib) - verzug;
        while (lese < 0.0) lese += static_cast<double>(groesse);
        const auto i = static_cast<std::size_t>(lese);
        const double rest = lese - static_cast<double>(i);
        const double y = ring[i % groesse] + (ring[(i + 1) % groesse] - ring[i % groesse]) * rest;
        ring[schreib] = 0.5 * (y + davor) * jeUmlauf;
        davor = y;
        schreib = (schreib + 1) % groesse;

        // die Saite klingt von selbst aus; die Huelle nimmt nur Einsatz und Ende
        double h = std::min(1.0, static_cast<double>(t) / anstieg);
        if (t >= huelle.ende)
        {
            const double nach = static_cast<double>(t - huelle.ende);
            h *= std::exp(-nach / huelle.ausklang) * std::max(0.0, 1.0 - nach / static_cast<double>(huelle.aus));
        }
        const auto aus = static_cast<float>(verzerrer.bei(y) * h * pegel);
        daten[static_cast<std::size_t>(2 * t)] = aus;
        daten[static_cast<std::size_t>(2 * t + 1)] = aus;
    }
}

/// So lang blendet ein Sustain loop an seiner Naht hoechstens ueber.
constexpr double kNahtMs = 10.0;

/// Eine Aufnahme, je Note schneller oder langsamer abgespielt (Stufe 11a):
/// der Abstand zum Grundton bestimmt das Tempo, zwischen den Abtastwerten
/// wird geradlinig gelesen. Solange die Note dauert, wiederholt sich der
/// Sustain loop — an seiner Naht kurz ueberblendet; danach laeuft die
/// Aufnahme weiter bis an ihr Ende. Ist zu `schicht.sample` nichts gemerkt,
/// bleibt die Schicht still. Die beiden Seiten der Aufnahme bleiben getrennt.
void rechneSample(const Schicht& schicht, int hoehe, double hz, const Huelle& huelle, int rate, std::vector<float>& daten)
{
    const std::shared_ptr<const SampleDaten> quelle = sampleDaten(schicht.sample);
    if (quelle == nullptr || quelle->rate <= 0 || quelle->klang.frames() < 2) return;
    const std::vector<float>& roh = quelle->klang.daten;
    const auto laenge = static_cast<double>(quelle->klang.frames());
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    const double jeMs = quelle->rate / 1000.0;

    const double abstand = (hoehe - std::clamp(schicht.grundton, 0, 127)) / 12.0 -
                           std::clamp(schicht.feinCent, -100.0, 100.0) / 1200.0;
    const double schritt = std::exp2(abstand) * quelle->rate / rate;
    double ort = std::clamp(schicht.sampleStartMs * jeMs, 0.0, laenge - 1.0);

    const double von = std::clamp(schicht.schleifeVonMs * jeMs, 0.0, laenge - 1.0);
    const double bis = std::clamp(schicht.schleifeBisMs * jeMs, 0.0, laenge - 1.0);
    const bool schleife = schicht.schleife && bis - von >= 2.0;
    const double naht = schleife ? std::min({kNahtMs * jeMs, (bis - von) / 4.0, von}) : 0.0;

    const auto lies = [&roh, laenge](double p, std::size_t seite) {
        if (p < 0.0 || p >= laenge - 1.0) return 0.0;
        const auto i = static_cast<std::size_t>(p);
        const double rest = p - static_cast<double>(i);
        const double a = roh[2 * i + seite];
        const double b = roh[2 * i + 2 + seite];
        return a + (b - a) * rest;
    };

    const Verzerrer verzerrer(schicht.verzerrung);
    const bool filter = schicht.filterTeilton > 0.0;
    std::array<double, 2> f1 = {0.0, 0.0};
    std::array<double, 2> f2 = {0.0, 0.0};
    const double pegel = kPegel * schicht.laut;

    for (std::int64_t t = 0; t < frames; ++t)
    {
        if (ort >= laenge - 1.0) break;  // die Aufnahme ist zu Ende: der Rest bleibt still
        const double h = huelle.bei(t);
        const bool gehalten = schleife && t < huelle.ende;
        // an der Naht: was gleich wieder beginnt, kommt schon dazu
        const double blende = gehalten && naht > 0.0 && ort > bis - naht ? (ort - (bis - naht)) / naht : 0.0;
        double a = 0.0;
        if (filter)
        {
            const double grenze = std::clamp(hz * (schicht.filterTeilton + schicht.filterHuelle * h), 40.0, rate * 0.45);
            a = 1.0 - std::exp(-2.0 * kPi * grenze / rate);
        }
        for (std::size_t seite = 0; seite < 2; ++seite)
        {
            double wert = lies(ort, seite);
            if (blende > 0.0) wert = wert * (1.0 - blende) + lies(ort - (bis - von), seite) * blende;
            if (filter)
            {
                f1[seite] += a * (wert - f1[seite]);
                f2[seite] += a * (f1[seite] - f2[seite]);
                wert = f2[seite];
            }
            daten[static_cast<std::size_t>(2 * t) + seite] = static_cast<float>(verzerrer.bei(wert) * h * pegel);
        }
        ort += schritt;
        if (gehalten && ort >= bis) ort -= bis - von;
    }
}

}  // namespace

// =============================================================================
// Die Datei `.lvinst`
// =============================================================================

namespace
{

const QString kFormat = QStringLiteral("lumiviz-instrument");

struct Wellenname
{
    Schwingung schwingung;
    const char* name;
};
constexpr std::array<Wellenname, 6> kWellen = {{{Schwingung::Sinus, "sine"},
                                                {Schwingung::Dreieck, "triangle"},
                                                {Schwingung::Saege, "saw"},
                                                {Schwingung::Rechteck, "square"},
                                                {Schwingung::Saite, "string"},
                                                {Schwingung::Sample, "sample"}}};

double inGrenzen(double wert, double von, double bis, double ersatz)
{
    return std::isfinite(wert) ? std::clamp(wert, von, bis) : ersatz;
}

std::uint64_t& fassung()
{
    static std::uint64_t eine = 0;
    return eine;
}

}  // namespace

namespace
{

QJsonObject schichtAlsJson(const Schicht& schicht)
{
    QJsonObject o;
    o[QStringLiteral("name")] = schicht.name;
    o[QStringLiteral("on")] = schicht.an;
    o[QStringLiteral("transpose")] = schicht.versatzHalbtoene;
    o[QStringLiteral("startOffset")] = schicht.einsatzMs;
    o[QStringLiteral("drive")] = schicht.verzerrung;
    for (const Wellenname& welle : kWellen)
        if (welle.schwingung == schicht.schwingung) o[QStringLiteral("waveform")] = QLatin1String(welle.name);
    QJsonArray obertoene;
    for (const double anteil : schicht.obertoene) obertoene.append(anteil);
    o[QStringLiteral("harmonics")] = obertoene;
    o[QStringLiteral("voices")] = schicht.stimmen;
    o[QStringLiteral("detune")] = schicht.verstimmungCent;
    o[QStringLiteral("attack")] = schicht.anstiegMs;
    o[QStringLiteral("decay")] = schicht.abfallMs;
    o[QStringLiteral("sustain")] = schicht.halten;
    o[QStringLiteral("release")] = schicht.ausklangMs;
    o[QStringLiteral("filterCutoff")] = schicht.filterTeilton;
    o[QStringLiteral("filterEnvelope")] = schicht.filterHuelle;
    o[QStringLiteral("vibratoRate")] = schicht.vibratoHz;
    o[QStringLiteral("vibratoDepth")] = schicht.vibratoCent;
    o[QStringLiteral("vibratoDelay")] = schicht.vibratoEinsatzMs;
    o[QStringLiteral("damping")] = schicht.saiteDaempfung;
    o[QStringLiteral("level")] = schicht.laut;
    if (schicht.schwingung == Schwingung::Sample)
    {
        // seit Fassung 4; nur eine Sample-Schicht traegt sie
        o[QStringLiteral("sample")] = schicht.sample;
        o[QStringLiteral("rootNote")] = schicht.grundton;
        o[QStringLiteral("fineTune")] = schicht.feinCent;
        o[QStringLiteral("sampleStart")] = schicht.sampleStartMs;
        o[QStringLiteral("sustainLoop")] = schicht.schleife;
        o[QStringLiteral("loopFrom")] = schicht.schleifeVonMs;
        o[QStringLiteral("loopTo")] = schicht.schleifeBisMs;
    }
    return o;
}

/// Eine Schicht lesen: aus einem Eintrag von `layers` — oder aus der Wurzel
/// einer Datei der Fassung 1, die dieselben Felder flach traegt.
Schicht schichtAusJson(const QJsonObject& wurzel)
{
    const Schicht vorgabe;
    Schicht neu;
    neu.name = wurzel.value(QStringLiteral("name")).toString();
    neu.an = wurzel.value(QStringLiteral("on")).toBool(true);
    neu.versatzHalbtoene = std::clamp(wurzel.value(QStringLiteral("transpose")).toInt(0), -24, 24);
    const QString wellenName = wurzel.value(QStringLiteral("waveform")).toString();
    for (const Wellenname& welle : kWellen)
        if (wellenName == QLatin1String(welle.name)) neu.schwingung = welle.schwingung;
    for (const QJsonValue& anteil : wurzel.value(QStringLiteral("harmonics")).toArray())
    {
        if (neu.obertoene.size() >= static_cast<std::size_t>(kObertoeneMax)) break;
        neu.obertoene.push_back(inGrenzen(anteil.toDouble(), -1.0, 1.0, 0.0));
    }
    const auto zahl = [&wurzel](const char* schluessel, double ersatz, double von, double bis) {
        return inGrenzen(wurzel.value(QLatin1String(schluessel)).toDouble(ersatz), von, bis, ersatz);
    };
    neu.stimmen = std::clamp(wurzel.value(QStringLiteral("voices")).toInt(vorgabe.stimmen), 1, 3);
    neu.verstimmungCent = zahl("detune", vorgabe.verstimmungCent, 0.0, 100.0);
    neu.anstiegMs = zahl("attack", vorgabe.anstiegMs, 0.0, 5000.0);
    neu.abfallMs = zahl("decay", vorgabe.abfallMs, 0.0, 10000.0);
    neu.halten = zahl("sustain", vorgabe.halten, 0.0, 1.0);
    neu.ausklangMs = zahl("release", vorgabe.ausklangMs, 0.0, 5000.0);
    neu.filterTeilton = zahl("filterCutoff", vorgabe.filterTeilton, 0.0, 64.0);
    neu.filterHuelle = zahl("filterEnvelope", vorgabe.filterHuelle, 0.0, 64.0);
    neu.vibratoHz = zahl("vibratoRate", vorgabe.vibratoHz, 0.0, 20.0);
    neu.vibratoCent = zahl("vibratoDepth", vorgabe.vibratoCent, 0.0, 100.0);
    neu.vibratoEinsatzMs = zahl("vibratoDelay", vorgabe.vibratoEinsatzMs, 0.0, 5000.0);
    neu.saiteDaempfung = zahl("damping", vorgabe.saiteDaempfung, 0.9, 0.9999);
    neu.laut = zahl("level", vorgabe.laut, 0.0, 1.0);
    neu.einsatzMs = zahl("startOffset", vorgabe.einsatzMs, 0.0, 1000.0);
    neu.verzerrung = zahl("drive", vorgabe.verzerrung, 0.0, 1.0);
    // seit Fassung 4; in einer aelteren Datei und an anderen Schichten fehlen sie
    const double laengsteMs = kKlangMaxSekunden * 1000.0;
    neu.sample = wurzel.value(QStringLiteral("sample")).toString();
    neu.grundton = std::clamp(wurzel.value(QStringLiteral("rootNote")).toInt(vorgabe.grundton), 0, 127);
    neu.feinCent = zahl("fineTune", vorgabe.feinCent, -100.0, 100.0);
    neu.sampleStartMs = zahl("sampleStart", vorgabe.sampleStartMs, 0.0, laengsteMs);
    neu.schleife = wurzel.value(QStringLiteral("sustainLoop")).toBool(false);
    neu.schleifeVonMs = zahl("loopFrom", vorgabe.schleifeVonMs, 0.0, laengsteMs);
    neu.schleifeBisMs = zahl("loopTo", vorgabe.schleifeBisMs, 0.0, laengsteMs);
    return neu;
}

}  // namespace

QJsonObject alsJson(const Instrument& instrument)
{
    QJsonObject o;
    o[QStringLiteral("format")] = kFormat;
    o[QStringLiteral("version")] = kInstrumentFassung;
    o[QStringLiteral("title")] = instrument.titel;
    o[QStringLiteral("category")] = instrument.art;
    o[QStringLiteral("genres")] = QJsonArray::fromStringList(instrument.genres);
    o[QStringLiteral("description")] = instrument.beschreibung;
    o[QStringLiteral("level")] = instrument.laut;
    QJsonArray schichten;
    for (const Schicht& schicht : instrument.schichten) schichten.append(schichtAlsJson(schicht));
    o[QStringLiteral("layers")] = schichten;
    return o;
}

bool ausJson(const QJsonObject& wurzel, Instrument& instrument, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    const int fassung = wurzel.value(QStringLiteral("version")).toInt(1);
    if (fassung > kInstrumentFassung)
    {
        // eine Fassung von morgen: lieber schweigen als etwas anderes spielen
        if (zuNeu != nullptr) *zuNeu = true;
        return false;
    }
    Instrument neu;
    neu.name = instrument.name;
    neu.titel = wurzel.value(QStringLiteral("title")).toString();
    // seit Fassung 3; in einer aelteren Datei fehlen sie einfach
    neu.art = wurzel.value(QStringLiteral("category")).toString().trimmed();
    for (const QJsonValue& genre : wurzel.value(QStringLiteral("genres")).toArray())
    {
        const QString text = genre.toString().trimmed();
        if (!text.isEmpty() && !neu.genres.contains(text, Qt::CaseInsensitive)) neu.genres << text;
    }
    neu.beschreibung = wurzel.value(QStringLiteral("description")).toString().trimmed();
    neu.schichten.clear();
    if (fassung < 2)
    {
        // Fassung 1 ist flach: eine Schicht, `level` ist ihre Lautstaerke
        neu.schichten.push_back(schichtAusJson(wurzel));
    }
    else
    {
        neu.laut = inGrenzen(wurzel.value(QStringLiteral("level")).toDouble(1.0), 0.0, 1.0, 1.0);
        for (const QJsonValue& schicht : wurzel.value(QStringLiteral("layers")).toArray())
        {
            if (neu.schichten.size() >= static_cast<std::size_t>(kSchichtenMax)) break;
            neu.schichten.push_back(schichtAusJson(schicht.toObject()));
        }
    }
    if (neu.schichten.empty()) neu.schichten.push_back(Schicht{});
    instrument = std::move(neu);
    return true;
}

bool speichereInstrument(const Instrument& instrument, const QString& pfad)
{
    QSaveFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    if (datei.write(QJsonDocument(alsJson(instrument)).toJson(QJsonDocument::Indented)) < 0 || !datei.commit())
        return false;
    ++fassung();  // wer Toene dieses Instruments gerechnet hat, rechnet neu
    return true;
}

bool ladeInstrument(const QString& pfad, Instrument& instrument, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    Instrument neu;
    if (!ausJson(doc.object(), neu, zuNeu)) return false;
    neu.name = pfad;
    if (neu.titel.isEmpty()) neu.titel = QFileInfo(pfad).completeBaseName();
    instrument = std::move(neu);
    return true;
}

void zaehleInstrumentFassung()
{
    ++fassung();
}

std::uint64_t instrumentFassung()
{
    return fassung();
}

QString instrumentOrdner()
{
    const QString kit = kitOrdner();
    return kit.isEmpty() ? QString() : QDir::cleanPath(kit + QStringLiteral("/../instruments"));
}

const std::vector<Instrument>& instrumente()
{
    // einmal gelesen und gemerkt — aber nur, wenn der Ordner da war
    static std::vector<Instrument> alle;
    if (!alle.empty()) return alle;
    const QString ordner = instrumentOrdner();
    if (ordner.isEmpty()) return alle;
    const QStringList dateien = QDir(ordner).entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    for (const QString& datei : dateien)
    {
        Instrument instrument;
        if (!ladeInstrument(QDir(ordner).filePath(datei), instrument)) continue;
        instrument.name = QFileInfo(datei).completeBaseName();  // ein mitgeliefertes heisst wie seine Datei
        alle.push_back(std::move(instrument));
    }
    std::stable_sort(alle.begin(), alle.end(), [](const Instrument& a, const Instrument& b) {
        return a.titel.compare(b.titel, Qt::CaseInsensitive) < 0;
    });
    return alle;
}

// --- die Arten ---------------------------------------------------------------

const std::vector<InstrumentArt>& instrumentArten()
{
    static const std::vector<InstrumentArt> kArten = {
        {"keys", "Keys"},
        {"bass", "Bass"},
        {"lead", "Lead"},
        {"pads", "Pads and strings"},
        {"plucked", "Plucked"},
        {"wind", "Wind"},
        {"percussion", "Percussion"},
        {"unusual", "Unusual"},
    };
    return kArten;
}

QString gueltigeArt(const QString& art)
{
    for (const InstrumentArt& eine : instrumentArten())
        if (art.compare(QLatin1String(eine.schluessel), Qt::CaseInsensitive) == 0)
            return QLatin1String(eine.schluessel);
    return QLatin1String(instrumentArten().back().schluessel);
}

QString artTitel(const QString& art)
{
    const QString schluessel = gueltigeArt(art);
    for (const InstrumentArt& eine : instrumentArten())
        if (schluessel == QLatin1String(eine.schluessel)) return QLatin1String(eine.titel);
    return {};
}

// --- eigene Instrumente im Benutzerordner -----------------------------------

namespace
{
QString& eigenerOrdnerImTest()
{
    static QString ordner;
    return ordner;
}
}  // namespace

bool istEigenesInstrument(const QString& nennung)
{
    return nennung.startsWith(QLatin1String(kEigenVorsatz));
}

QString eigenerInstrumentOrdner()
{
    if (!eigenerOrdnerImTest().isEmpty()) return eigenerOrdnerImTest();
    const QString basis = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return basis.isEmpty() ? QString() : QDir(basis).filePath(QStringLiteral("instruments"));
}

void setzeEigenenInstrumentOrdnerFuerTests(const QString& ordner)
{
    eigenerOrdnerImTest() = ordner;
}

QString eigenerName(const QString& titel)
{
    QString name;
    for (const QChar zeichen : titel.trimmed().toLower())
    {
        if ((zeichen >= QLatin1Char('a') && zeichen <= QLatin1Char('z')) ||
            (zeichen >= QLatin1Char('0') && zeichen <= QLatin1Char('9')))
            name.append(zeichen);
        else if (!name.isEmpty() && !name.endsWith(QLatin1Char('_')))
            name.append(QLatin1Char('_'));  // alles andere wird ein Trenner, nie zwei hintereinander
    }
    while (name.endsWith(QLatin1Char('_'))) name.chop(1);
    return name.left(64);
}

bool istEigenerName(const QString& name)
{
    return !name.isEmpty() && eigenerName(name) == name;
}

QString eigeneDatei(const QString& name)
{
    const QString ordner = eigenerInstrumentOrdner();
    if (ordner.isEmpty() || !istEigenerName(name)) return {};
    return QDir(ordner).filePath(name + QStringLiteral(".lvinst"));
}

std::vector<Instrument> eigeneInstrumente()
{
    std::vector<Instrument> alle;
    const QString ordner = eigenerInstrumentOrdner();
    if (ordner.isEmpty() || !QFileInfo(ordner).isDir()) return alle;
    const QStringList dateien = QDir(ordner).entryList({QStringLiteral("*.lvinst")}, QDir::Files, QDir::Name);
    for (const QString& datei : dateien)
    {
        const QString name = QFileInfo(datei).completeBaseName();
        if (!istEigenerName(name)) continue;  // von Hand hineingelegt, mit einem Namen, den kein Muster nennen kann
        Instrument instrument;
        if (!ladeInstrument(QDir(ordner).filePath(datei), instrument)) continue;
        instrument.name = QLatin1String(kEigenVorsatz) + name;
        alle.push_back(std::move(instrument));
    }
    std::stable_sort(alle.begin(), alle.end(), [](const Instrument& a, const Instrument& b) {
        return a.titel.compare(b.titel, Qt::CaseInsensitive) < 0;
    });
    return alle;
}

bool speichereEigenes(Instrument& instrument, const QString& name)
{
    const QString pfad = eigeneDatei(name);
    if (pfad.isEmpty() || !QDir().mkpath(QFileInfo(pfad).absolutePath())) return false;
    if (!speichereInstrument(instrument, pfad)) return false;
    instrument.name = QLatin1String(kEigenVorsatz) + name;
    return true;
}

bool loescheEigenes(const QString& name)
{
    const QString pfad = eigeneDatei(name);
    if (pfad.isEmpty() || !QFile::remove(pfad)) return false;
    ++fassung();  // wer Toene dieses Instruments gerechnet hat, rechnet neu
    return true;
}

QStringList bekannteGenres()
{
    QStringList alle;
    const auto nimm = [&alle](const Instrument& instrument) {
        for (const QString& genre : instrument.genres)
            if (!alle.contains(genre, Qt::CaseInsensitive)) alle << genre;
    };
    for (const Instrument& instrument : instrumente()) nimm(instrument);
    for (const Instrument& instrument : eigeneInstrumente()) nimm(instrument);
    alle.sort(Qt::CaseInsensitive);
    return alle;
}

bool holeInstrument(const QString& nennung, Instrument& instrument, bool* zuNeu)
{
    if (zuNeu != nullptr) *zuNeu = false;
    if (istEigenesInstrument(nennung))
    {
        const QString pfad = eigeneDatei(nennung.mid(static_cast<int>(qstrlen(kEigenVorsatz))));
        Instrument eigenes;
        if (pfad.isEmpty() || !ladeInstrument(pfad, eigenes, zuNeu)) return false;
        eigenes.name = nennung;
        instrument = std::move(eigenes);
        return true;
    }
    if (istInstrumentDatei(nennung)) return ladeInstrument(nennung, instrument, zuNeu);
    const Instrument* gefunden = findeInstrument(nennung);
    if (gefunden == nullptr) return false;
    instrument = *gefunden;
    return true;
}

const Instrument* findeInstrument(const QString& name)
{
    const std::vector<Instrument>& alle = instrumente();
    const auto it = std::find_if(alle.begin(), alle.end(), [&name](const Instrument& i) { return i.name == name; });
    return it == alle.end() ? nullptr : &*it;
}

double tonHz(int hoehe)
{
    return 440.0 * std::exp2((hoehe - 69) / 12.0);
}

QString tonName(int hoehe)
{
    static const std::array<const char*, 12> kNamen = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const int stufe = ((hoehe % 12) + 12) % 12;
    const int oktave = (hoehe - stufe) / 12 - 1;
    return QString::fromLatin1(kNamen[static_cast<std::size_t>(stufe)]) + QString::number(oktave);
}

Klang rendereTon(const Instrument& instrument, int hoehe, double dauerMs, int rate)
{
    Klang klang;
    if (rate <= 0 || dauerMs <= 0.0) return klang;
    const auto hoechstens = static_cast<std::int64_t>(kTonMaxSekunden * rate);
    std::vector<float> eine;
    bool erste = true;
    for (const Schicht& schicht : instrument.schichten)
    {
        if (!schicht.an) continue;
        // die Schicht setzt spaeter ein und endet mit der Note; ist die Note
        // kuerzer als der Einsatz, bleibt ein kurzer Anschlag
        const double einsatzMs = std::clamp(schicht.einsatzMs, 0.0, 1000.0);
        const auto einsatz = std::min<std::int64_t>(std::llround(einsatzMs * rate / 1000.0), hoechstens - 2);
        const int ton = hoehe + std::clamp(schicht.versatzHalbtoene, -24, 24);
        const double hz = tonHz(ton);
        Huelle huelle = huelleFuer(schicht, std::max(1.0, dauerMs - einsatzMs), rate);
        huelle.aus = std::min<std::int64_t>(huelle.aus, hoechstens - einsatz - 1);
        if (einsatz + huelle.ende + huelle.aus > hoechstens)
            huelle.ende = std::max<std::int64_t>(1, hoechstens - einsatz - huelle.aus);
        const std::int64_t frames = huelle.ende + huelle.aus;
        eine.assign(static_cast<std::size_t>(2 * frames), 0.0f);
        if (schicht.schwingung == Schwingung::Saite)
            rechneSaite(schicht, ton, hz, huelle, rate, eine);
        else if (schicht.schwingung == Schwingung::Sample)
            rechneSample(schicht, ton, hz, huelle, rate, eine);
        else
            rechneSchwingung(schicht, hz, huelle, rate, eine);
        if (erste && einsatz == 0)
        {
            klang.daten.swap(eine);  // eine Schicht allein: ihre Werte, unveraendert
            erste = false;
            continue;
        }
        erste = false;
        const auto ab = static_cast<std::size_t>(2 * einsatz);
        if (klang.daten.size() < ab + eine.size()) klang.daten.resize(ab + eine.size(), 0.0f);
        for (std::size_t i = 0; i < eine.size(); ++i) klang.daten[ab + i] += eine[i];
    }
    if (klang.daten.empty())
    {
        // keine Schicht an: so lang wie die Note, still
        const std::int64_t frames = std::min<std::int64_t>(std::max<std::int64_t>(1, std::llround(dauerMs * rate / 1000.0)), hoechstens);
        klang.daten.assign(static_cast<std::size_t>(2 * frames), 0.0f);
    }
    if (instrument.laut != 1.0)
    {
        const auto laut = static_cast<float>(std::clamp(instrument.laut, 0.0, 1.0));
        for (float& wert : klang.daten) wert *= laut;
    }
    return klang;
}

}  // namespace lumi::loop

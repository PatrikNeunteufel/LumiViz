/**
 * @file Instrument.cpp
 * @brief Loop: gerechnete Instrumente (Stufe 9)
 */

#include "loop/Instrument.hpp"

#include "loop/Kit.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace lumi::loop
{

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kTafel = 2048;        ///< Abtastwerte eines Wellenzugs
constexpr int kTeiltoeneMax = 512;  ///< mehr traegt die Tafel nicht sauber
constexpr double kPegel = 0.5;      ///< Luft nach oben: ein Akkord addiert sich (Entscheid L6 a)

/// Der Anteil des k-ten Teiltons (k ab 1) an der Schwingung.
double teilton(const Instrument& instrument, int k)
{
    switch (instrument.schwingung)
    {
    case Schwingung::Sinus:
        if (k == 1) return 1.0;
        return k - 2 < static_cast<int>(instrument.obertoene.size()) ? instrument.obertoene[static_cast<std::size_t>(k - 2)]
                                                                     : 0.0;
    case Schwingung::Dreieck:
        return k % 2 == 1 ? ((k / 2) % 2 == 0 ? 1.0 : -1.0) / (static_cast<double>(k) * k) : 0.0;
    case Schwingung::Saege:
        return 1.0 / k;
    case Schwingung::Rechteck:
        return k % 2 == 1 ? 1.0 / k : 0.0;
    case Schwingung::Saite:
        break;
    }
    return 0.0;
}

/// Ein Wellenzug der Schwingung, ohne Teiltoene ueber `hoechsterTeilton`, auf
/// den Betrag 1 gebracht. Ein Wert mehr als `kTafel`: der erste noch einmal.
std::vector<float> wellenzug(const Instrument& instrument, int hoechsterTeilton)
{
    std::vector<double> summe(kTafel, 0.0);
    for (int k = 1; k <= hoechsterTeilton; ++k)
    {
        const double anteil = teilton(instrument, k);
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

Huelle huelleFuer(const Instrument& instrument, double dauerMs, int rate)
{
    const double jeMs = rate / 1000.0;
    Huelle h;
    h.anstieg = std::max(1.0, instrument.anstiegMs * jeMs);
    h.abfall = std::max(1.0, instrument.abfallMs * jeMs / 3.0);
    h.halten = std::clamp(instrument.halten, 0.0, 1.0);
    h.ende = std::max<std::int64_t>(1, std::llround(dauerMs * jeMs));
    h.aus = std::max<std::int64_t>(1, std::llround(std::max(5.0, instrument.ausklangMs) * jeMs));
    h.ausklang = std::max(1.0, static_cast<double>(h.aus) / 4.0);
    return h;
}

void rechneSchwingung(const Instrument& instrument, double hz, const Huelle& huelle, int rate, std::vector<float>& daten)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    // Vibrato und Verstimmung heben den Ton leicht: etwas Abstand zur halben Abtastrate
    const int hoechster = std::clamp(static_cast<int>(rate / 2.0 / (hz * 1.06)), 1, kTeiltoeneMax);
    const std::vector<float> tafel = wellenzug(instrument, hoechster);

    const int stimmen = std::clamp(instrument.stimmen, 1, 3);
    std::array<double, 3> verhaeltnis = {1.0, 1.0, 1.0};
    std::array<double, 3> phase = {0.0, 0.33, 0.67};  // fest: derselbe Ton ergibt dieselben Werte
    if (stimmen == 2) verhaeltnis = {std::exp2(-instrument.verstimmungCent / 1200.0), std::exp2(instrument.verstimmungCent / 1200.0), 1.0};
    if (stimmen == 3) verhaeltnis = {1.0, std::exp2(-instrument.verstimmungCent / 1200.0), std::exp2(instrument.verstimmungCent / 1200.0)};
    const double jeStimme = 1.0 / std::sqrt(static_cast<double>(stimmen));

    const double einsatz = std::max(1.0, instrument.vibratoEinsatzMs * rate / 1000.0);
    const bool vibrato = instrument.vibratoHz > 0.0 && instrument.vibratoCent > 0.0;
    const bool filter = instrument.filterTeilton > 0.0;
    double f1 = 0.0;
    double f2 = 0.0;
    const double pegel = kPegel * instrument.laut;

    for (std::int64_t t = 0; t < frames; ++t)
    {
        const double h = huelle.bei(t);
        double schritt = hz / rate;
        if (vibrato)
        {
            const double tiefe = instrument.vibratoCent * std::min(1.0, static_cast<double>(t) / einsatz);
            schritt *= std::exp2(tiefe * std::sin(2.0 * kPi * instrument.vibratoHz * static_cast<double>(t) / rate) / 1200.0);
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
                std::clamp(hz * (instrument.filterTeilton + instrument.filterHuelle * h), 40.0, rate * 0.45);
            const double a = 1.0 - std::exp(-2.0 * kPi * grenze / rate);
            f1 += a * (wert - f1);
            f2 += a * (f1 - f2);
            wert = f2;
        }
        const auto aus = static_cast<float>(wert * h * pegel);
        daten[static_cast<std::size_t>(2 * t)] = aus;
        daten[static_cast<std::size_t>(2 * t + 1)] = aus;
    }
}

/// Die gezupfte Saite (Karplus-Strong): ein Stoss Rauschen laeuft durch eine
/// Verzoegerung von einer Schwingung Laenge und wird bei jedem Umlauf weicher
/// und leiser.
void rechneSaite(const Instrument& instrument, int hoehe, double hz, const Huelle& huelle, int rate,
                 std::vector<float>& daten)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
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
    const double daempfung = std::clamp(instrument.saiteDaempfung, 0.9, 0.9999);
    const double jeUmlauf = std::exp(std::log(daempfung) * 220.0 / hz);
    const double pegel = kPegel * instrument.laut;
    const double anstieg = std::max(1.0, instrument.anstiegMs * rate / 1000.0);

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
        const auto aus = static_cast<float>(y * h * pegel);
        daten[static_cast<std::size_t>(2 * t)] = aus;
        daten[static_cast<std::size_t>(2 * t + 1)] = aus;
    }
}

}  // namespace

// =============================================================================
// Die Datei `.lvinst`
// =============================================================================

namespace
{

const QString kFormat = QStringLiteral("lumiviz-instrument");
constexpr int kVersion = 1;

struct Wellenname
{
    Schwingung schwingung;
    const char* name;
};
constexpr std::array<Wellenname, 5> kWellen = {{{Schwingung::Sinus, "sine"},
                                                {Schwingung::Dreieck, "triangle"},
                                                {Schwingung::Saege, "saw"},
                                                {Schwingung::Rechteck, "square"},
                                                {Schwingung::Saite, "string"}}};

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

QJsonObject alsJson(const Instrument& instrument)
{
    QJsonObject o;
    o[QStringLiteral("format")] = kFormat;
    o[QStringLiteral("version")] = kVersion;
    o[QStringLiteral("title")] = instrument.titel;
    for (const Wellenname& welle : kWellen)
        if (welle.schwingung == instrument.schwingung) o[QStringLiteral("waveform")] = QLatin1String(welle.name);
    QJsonArray obertoene;
    for (const double anteil : instrument.obertoene) obertoene.append(anteil);
    o[QStringLiteral("harmonics")] = obertoene;
    o[QStringLiteral("voices")] = instrument.stimmen;
    o[QStringLiteral("detune")] = instrument.verstimmungCent;
    o[QStringLiteral("attack")] = instrument.anstiegMs;
    o[QStringLiteral("decay")] = instrument.abfallMs;
    o[QStringLiteral("sustain")] = instrument.halten;
    o[QStringLiteral("release")] = instrument.ausklangMs;
    o[QStringLiteral("filterCutoff")] = instrument.filterTeilton;
    o[QStringLiteral("filterEnvelope")] = instrument.filterHuelle;
    o[QStringLiteral("vibratoRate")] = instrument.vibratoHz;
    o[QStringLiteral("vibratoDepth")] = instrument.vibratoCent;
    o[QStringLiteral("vibratoDelay")] = instrument.vibratoEinsatzMs;
    o[QStringLiteral("damping")] = instrument.saiteDaempfung;
    o[QStringLiteral("level")] = instrument.laut;
    return o;
}

bool ausJson(const QJsonObject& wurzel, Instrument& instrument)
{
    if (wurzel.value(QStringLiteral("format")).toString() != kFormat) return false;
    const Instrument vorgabe;
    Instrument neu;
    neu.name = instrument.name;
    neu.titel = wurzel.value(QStringLiteral("title")).toString();
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

bool ladeInstrument(const QString& pfad, Instrument& instrument)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return false;
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(datei.readAll(), &fehler);
    if (fehler.error != QJsonParseError::NoError || !doc.isObject()) return false;
    Instrument neu;
    if (!ausJson(doc.object(), neu)) return false;
    neu.name = pfad;
    if (neu.titel.isEmpty()) neu.titel = QFileInfo(pfad).completeBaseName();
    instrument = std::move(neu);
    return true;
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

bool holeInstrument(const QString& nennung, Instrument& instrument)
{
    if (istInstrumentDatei(nennung)) return ladeInstrument(nennung, instrument);
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
    const double hz = tonHz(hoehe);
    Huelle huelle = huelleFuer(instrument, dauerMs, rate);
    const auto hoechstens = static_cast<std::int64_t>(kTonMaxSekunden * rate);
    if (huelle.ende + huelle.aus > hoechstens)
    {
        huelle.ende = std::max<std::int64_t>(1, hoechstens - huelle.aus);
    }
    const std::int64_t frames = huelle.ende + huelle.aus;
    klang.daten.assign(static_cast<std::size_t>(2 * frames), 0.0f);
    if (instrument.schwingung == Schwingung::Saite)
        rechneSaite(instrument, hoehe, hz, huelle, rate, klang.daten);
    else
        rechneSchwingung(instrument, hz, huelle, rate, klang.daten);
    return klang;
}

}  // namespace lumi::loop

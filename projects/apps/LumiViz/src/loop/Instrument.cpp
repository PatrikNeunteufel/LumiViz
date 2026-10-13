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
    case Schwingung::Rauschen:
        break;  // kein Wellenzug: die Saite, die Aufnahme und das Rauschen rechnen eigens
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

/// Der Filter einer Schicht fuer eine Seite des Tons (Stufe 11e).
///
/// Ein **Tiefpass ohne Resonanz** ist der Filter bis Stufe 11d, unveraendert:
/// zwei Stufen erster Ordnung hintereinander — jedes Instrument von frueher
/// ergibt dieselben Werte. Alles andere rechnet ein Biquad (die Formeln des
/// Kits, `make_kit.py`): Hochpass, Bandpass mit gleichbleibender Spitze, und
/// der Tiefpass mit Resonanz. Die Guete waechst mit `resonanz` von 0,707
/// (keine Betonung) bis rund 11; bei Tiefpass und Hochpass wird der Ton dabei
/// leiser gestellt, damit die Spitze an der Grenze nicht davonlaeuft.
struct Filter
{
    Filter(const SchichtFilter& werte, int rate) : m_art(werte.art), m_rate(rate), m_an(werte.teilton > 0.0)
    {
        const double resonanz = std::isfinite(werte.resonanz) ? std::clamp(werte.resonanz, 0.0, 1.0) : 0.0;
        m_alt = m_art == FilterArt::Tiefpass && resonanz <= 0.0;
        m_guete = kGueteOhne * std::exp2(4.0 * resonanz);
        if (m_art != FilterArt::Bandpass) m_laut = 1.0 / std::sqrt(m_guete / kGueteOhne);
    }
    [[nodiscard]] bool an() const { return m_an; }
    /// Ein Abtastwert durch den Filter; `grenzeHz` darf sich von Wert zu Wert aendern.
    [[nodiscard]] double bei(double wert, double grenzeHz)
    {
        if (m_alt)
        {
            const double a = 1.0 - std::exp(-2.0 * kPi * grenzeHz / m_rate);
            m_f1 += a * (wert - m_f1);
            m_f2 += a * (m_f1 - m_f2);
            return m_f2;
        }
        if (grenzeHz != m_grenze)
        {
            // nur neu rechnen, wenn die Grenze sich bewegt hat (Envelope)
            m_grenze = grenzeHz;
            const double w = 2.0 * kPi * grenzeHz / m_rate;
            const double alpha = std::sin(w) / (2.0 * m_guete);
            const double c = std::cos(w);
            const double a0 = 1.0 + alpha;
            switch (m_art)
            {
            case FilterArt::Tiefpass:
                m_b0 = (1.0 - c) / 2.0 / a0;
                m_b1 = (1.0 - c) / a0;
                m_b2 = m_b0;
                break;
            case FilterArt::Hochpass:
                m_b0 = (1.0 + c) / 2.0 / a0;
                m_b1 = -(1.0 + c) / a0;
                m_b2 = m_b0;
                break;
            case FilterArt::Bandpass:
                m_b0 = alpha / a0;
                m_b1 = 0.0;
                m_b2 = -alpha / a0;
                break;
            }
            m_a1 = -2.0 * c / a0;
            m_a2 = (1.0 - alpha) / a0;
        }
        const double aus = m_b0 * wert + m_b1 * m_x1 + m_b2 * m_x2 - m_a1 * m_y1 - m_a2 * m_y2;
        m_x2 = m_x1;
        m_x1 = wert;
        m_y2 = m_y1;
        m_y1 = aus;
        return aus * m_laut;
    }

private:
    static constexpr double kGueteOhne = 0.70710678118654752;
    FilterArt m_art;
    int m_rate;
    bool m_an;
    bool m_alt = true;
    double m_guete = kGueteOhne;
    double m_laut = 1.0;
    double m_f1 = 0.0;
    double m_f2 = 0.0;
    double m_grenze = -1.0;
    double m_b0 = 0.0;
    double m_b1 = 0.0;
    double m_b2 = 0.0;
    double m_a1 = 0.0;
    double m_a2 = 0.0;
    double m_x1 = 0.0;
    double m_x2 = 0.0;
    double m_y1 = 0.0;
    double m_y2 = 0.0;
};

/// Pitch sweep (Stufe 11d): das Verhaeltnis zur Tonhoehe der Note, je
/// Abtastwert. Die Frequenz laeuft von ihrem Start auf die der Note zu —
/// `f = ziel + (start − ziel) · e^(−t/τ)`, wie der Gleitton des Kits
/// (`asset/loop/kit/make_kit.py`); τ ist ein Drittel der Zeit, wie bei Decay.
struct Tonweg
{
    double mehr = 0.0;  ///< um so viel liegt der Start ueber der Note, als Anteil; 0 = aus
    double tau = 1.0;   ///< in Abtastwerten

    Tonweg(const Schicht& schicht, int rate)
    {
        const double start = inBereich(schicht.tonhoeheStartHt, kTonhoeheStartMin, kTonhoeheStartMax);
        const double zeitMs = inBereich(schicht.tonhoeheZeitMs, 0.0, kTonhoeheZeitMaxMs);
        if (start == 0.0 || zeitMs <= 0.0) return;
        mehr = std::exp2(start / 12.0) - 1.0;
        tau = std::max(1.0, zeitMs * rate / 1000.0 / 3.0);
    }
    [[nodiscard]] double bei(std::int64_t t) const
    {
        return mehr == 0.0 ? 1.0 : 1.0 + mehr * std::exp(-static_cast<double>(t) / tau);
    }
    /// Das groesste Verhaeltnis, das vorkommt (am Anfang einer fallenden Note).
    [[nodiscard]] double hoechstes() const { return std::max(1.0, 1.0 + mehr); }

private:
    static double inBereich(double wert, double von, double bis) { return std::isfinite(wert) ? std::clamp(wert, von, bis) : 0.0; }
};

/// Der Weg des Tons einer Schicht fuer eine Seite (Stufe 16b; Entscheide Patrik
/// S117, K1 a und K2 a): die Bausteine der Reihe, von vorn nach hinten — jeder
/// Filter mit eigenem Gedaechtnis, Drive, und die Stelle, an der die Huellkurve
/// den Ton formt. Ein offener Filter (Cutoff 0) und ein Drive mit 0 tun nichts
/// und kommen nicht vor. Fuer die Reihe Filter, Drive, Envelope ergibt das Wert
/// fuer Wert, was bis Stufe 16a fest gerechnet wurde.
class Reihe
{
public:
    /// `mitFilter` = false: die Saite klingt von selbst aus und hatte nie einen
    /// Filter — sie ueberspringt die Filter der Reihe weiter.
    Reihe(const Schicht& schicht, int rate, bool mitFilter = true) : m_grenzeMax(rate * 0.45)
    {
        bool huelle = false;
        for (const SchichtBaustein& baustein : schicht.reihe)
        {
            if (const auto* filter = std::get_if<SchichtFilter>(&baustein))
            {
                if (mitFilter && filter->teilton > 0.0) m_stufen.push_back(Stufe{Art::Filter, Filter(*filter, rate), Verzerrer(0.0), filter->teilton, filter->huelle});
            }
            else if (const auto* drive = std::get_if<SchichtDrive>(&baustein))
            {
                if (drive->wert > 0.0) m_stufen.push_back(Stufe{Art::Drive, Filter(SchichtFilter{}, rate), Verzerrer(drive->wert), 0.0, 0.0});
            }
            else if (!huelle)
            {
                huelle = true;
                m_stufen.push_back(Stufe{Art::Huelle, Filter(SchichtFilter{}, rate), Verzerrer(0.0), 0.0, 0.0});
            }
        }
        // eine Reihe ohne Envelope gibt es nach `richteReihe` nicht — sonst formt sie am Ende
        if (!huelle) m_stufen.push_back(Stufe{Art::Huelle, Filter(SchichtFilter{}, rate), Verzerrer(0.0), 0.0, 0.0});
    }
    /// Ein Abtastwert durch die Reihe. `h` ist die Huellkurve an dieser Stelle (sie oeffnet
    /// auch jeden Filter, gleich wo er steht), `hz` die Tonhoehe, der die Filter folgen.
    [[nodiscard]] double bei(double wert, double h, double hz)
    {
        for (Stufe& stufe : m_stufen)
        {
            switch (stufe.art)
            {
            case Art::Filter:
                wert = stufe.filter.bei(wert, std::clamp(hz * (stufe.teilton + stufe.huelle * h), 40.0, m_grenzeMax));
                break;
            case Art::Drive:
                wert = stufe.verzerrer.bei(wert);
                break;
            case Art::Huelle:
                wert *= h;
                break;
            }
        }
        return wert;
    }

private:
    enum class Art
    {
        Filter,
        Drive,
        Huelle,
    };
    struct Stufe
    {
        Art art;
        Filter filter;
        Verzerrer verzerrer;
        double teilton;  ///< nur Filter: Cutoff
        double huelle;   ///< nur Filter: Filter envelope
    };
    double m_grenzeMax;
    std::vector<Stufe> m_stufen;
};

/// Steht in der Reihe ein Filter, der rechnet, hinter der Envelope? Dann schwingt er nach,
/// wenn die Huellkurve schon bei null ist — der Ton endete nicht mehr bei null.
bool filterHinterHuelle(const Schicht& schicht)
{
    bool huelle = false;
    for (const SchichtBaustein& baustein : schicht.reihe)
    {
        if (std::holds_alternative<SchichtHuelle>(baustein))
            huelle = true;
        else if (const auto* filter = std::get_if<SchichtFilter>(&baustein); huelle && filter != nullptr && filter->teilton > 0.0)
            return true;
    }
    return false;
}

/// Die letzten Millisekunden eines Tons geradlinig auf null fuehren.
void blendeAus(std::vector<float>& daten, int rate)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    const std::int64_t strecke = std::min<std::int64_t>(frames, std::max(1, rate / 200));  // 5 ms
    for (std::int64_t i = 0; i < strecke; ++i)
    {
        const auto anteil = static_cast<float>(i) / static_cast<float>(strecke);
        const auto t = static_cast<std::size_t>(frames - 1 - i);
        daten[2 * t] *= anteil;
        daten[2 * t + 1] *= anteil;
    }
}

void rechneSchwingung(const Schicht& schicht, double hz, const Huelle& huelle, int rate, std::vector<float>& daten)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    const Tonweg weg(schicht, rate);
    // Vibrato und Verstimmung heben den Ton leicht: etwas Abstand zur halben Abtastrate.
    // Eine fallende Note beginnt hoeher: die Tafel traegt nur, was auch dort darunter bleibt.
    const int hoechster = std::clamp(static_cast<int>(rate / 2.0 / (hz * 1.06 * weg.hoechstes())), 1, kTeiltoeneMax);
    const std::vector<float> tafel = wellenzug(schicht, hoechster);
    const int stimmen = std::clamp(schicht.stimmen, 1, 3);
    std::array<double, 3> verhaeltnis = {1.0, 1.0, 1.0};
    std::array<double, 3> phase = {0.0, 0.33, 0.67};  // fest: derselbe Ton ergibt dieselben Werte
    if (stimmen == 2) verhaeltnis = {std::exp2(-schicht.verstimmungCent / 1200.0), std::exp2(schicht.verstimmungCent / 1200.0), 1.0};
    if (stimmen == 3) verhaeltnis = {1.0, std::exp2(-schicht.verstimmungCent / 1200.0), std::exp2(schicht.verstimmungCent / 1200.0)};
    const double jeStimme = 1.0 / std::sqrt(static_cast<double>(stimmen));

    const double einsatz = std::max(1.0, schicht.vibratoEinsatzMs * rate / 1000.0);
    const bool vibrato = schicht.vibratoHz > 0.0 && schicht.vibratoCent > 0.0;
    Reihe reihe(schicht, rate);
    const double pegel = kPegel * schicht.laut;

    for (std::int64_t t = 0; t < frames; ++t)
    {
        const double h = huelle.bei(t);
        double schritt = hz / rate * weg.bei(t);
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
        const auto aus = static_cast<float>(reihe.bei(wert, h, hz) * pegel);
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
    Reihe reihe(schicht, rate, false);  // ohne die Filter: die Saite hatte nie einen
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
        const auto aus = static_cast<float>(reihe.bei(y, h, hz) * pegel);
        daten[static_cast<std::size_t>(2 * t)] = aus;
        daten[static_cast<std::size_t>(2 * t + 1)] = aus;
    }
}

/// Das Rauschen (Stufe 11d): gleichverteilt, aus einem festen Zaehlwerk —
/// dasselbe wie im Kit (`rauschen` in `make_kit.py`) und bei jeder Note
/// dieselbe Folge: derselbe Ton ergibt dieselben Werte. Es hat keine
/// Tonhoehe; die Note (mit Transpose) bewegt nur den Filter, der ihr folgt.
/// Zwei Rauschen-Schichten eines Instruments tragen dieselbe Folge.
void rechneRauschen(const Schicht& schicht, double hz, const Huelle& huelle, int rate, std::vector<float>& daten)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    Reihe reihe(schicht, rate);
    const double pegel = kPegel * schicht.laut;
    std::uint32_t folge = 22222u;
    for (std::int64_t t = 0; t < frames; ++t)
    {
        const double h = huelle.bei(t);
        folge = folge * 1664525u + 1013904223u;
        const double wert = static_cast<double>(folge >> 8) / 8388608.0 - 1.0;
        const auto aus = static_cast<float>(reihe.bei(wert, h, hz) * pegel);
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
/// Die Aufnahme spielt so laut, wie sie ist — ohne die Luft nach oben, die eine
/// gerechnete Schwingung bekommt (`kPegel`): bei der Note ihres Grundtons
/// klingt sie wie die Datei (seit Stufe 11c; davor halb so laut).
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

    std::array<Reihe, 2> reihen = {Reihe(schicht, rate), Reihe(schicht, rate)};  // je Seite einer: jeder Filter hat sein Gedaechtnis  // je Seite einer
    const double pegel = schicht.laut;
    // Pitch sweep (N2 a): die Aufnahme laeuft am Anfang schneller (oder langsamer) und kommt
    // auf ihr Tempo — der Sustain loop rechnet mit jedem Schritt
    const Tonweg weg(schicht, rate);

    for (std::int64_t t = 0; t < frames; ++t)
    {
        if (ort >= laenge - 1.0) break;  // die Aufnahme ist zu Ende: der Rest bleibt still
        const double h = huelle.bei(t);
        const bool gehalten = schleife && t < huelle.ende;
        // an der Naht: was gleich wieder beginnt, kommt schon dazu
        const double blende = gehalten && naht > 0.0 && ort > bis - naht ? (ort - (bis - naht)) / naht : 0.0;
        for (std::size_t seite = 0; seite < 2; ++seite)
        {
            double wert = lies(ort, seite);
            if (blende > 0.0) wert = wert * (1.0 - blende) + lies(ort - (bis - von), seite) * blende;
            daten[static_cast<std::size_t>(2 * t) + seite] = static_cast<float>(reihen[seite].bei(wert, h, hz) * pegel);
        }
        ort += schritt * weg.bei(t);
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
constexpr std::array<Wellenname, 7> kWellen = {{{Schwingung::Sinus, "sine"},
                                                {Schwingung::Dreieck, "triangle"},
                                                {Schwingung::Saege, "saw"},
                                                {Schwingung::Rechteck, "square"},
                                                {Schwingung::Saite, "string"},
                                                {Schwingung::Sample, "sample"},
                                                {Schwingung::Rauschen, "noise"}}};

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

const QString kBausteinFilter = QStringLiteral("filter");
const QString kBausteinDrive = QStringLiteral("drive");
const QString kBausteinHuelle = QStringLiteral("envelope");

/// Ein Baustein der Reihe als Eintrag von `path`.
struct BausteinAlsJson
{
    QJsonObject operator()(const SchichtFilter& filter) const
    {
        QJsonObject b;
        b[QStringLiteral("type")] = kBausteinFilter;
        b[QStringLiteral("filterType")] = filterArtName(filter.art);
        b[QStringLiteral("cutoff")] = filter.teilton;
        b[QStringLiteral("envelope")] = filter.huelle;
        b[QStringLiteral("resonance")] = filter.resonanz;
        return b;
    }
    QJsonObject operator()(const SchichtDrive& drive) const
    {
        QJsonObject b;
        b[QStringLiteral("type")] = kBausteinDrive;
        b[QStringLiteral("amount")] = drive.wert;
        return b;
    }
    QJsonObject operator()(const SchichtHuelle&) const
    {
        QJsonObject b;
        b[QStringLiteral("type")] = kBausteinHuelle;
        return b;
    }
};

/// Die Werte eines Bausteins in ihre Grenzen.
struct RichteBaustein
{
    void operator()(SchichtFilter& filter) const
    {
        if (filter.art < FilterArt::Tiefpass || filter.art > FilterArt::Bandpass) filter.art = FilterArt::Tiefpass;
        filter.teilton = inGrenzen(filter.teilton, 0.0, 64.0, 0.0);
        filter.huelle = inGrenzen(filter.huelle, 0.0, 64.0, 0.0);
        filter.resonanz = inGrenzen(filter.resonanz, 0.0, 1.0, 0.0);
    }
    void operator()(SchichtDrive& drive) const { drive.wert = inGrenzen(drive.wert, 0.0, 1.0, 0.0); }
    void operator()(SchichtHuelle&) const {}
};

QJsonObject schichtAlsJson(const Schicht& schicht)
{
    QJsonObject o;
    o[QStringLiteral("name")] = schicht.name;
    o[QStringLiteral("on")] = schicht.an;
    o[QStringLiteral("transpose")] = schicht.versatzHalbtoene;
    o[QStringLiteral("startOffset")] = schicht.einsatzMs;
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
    o[QStringLiteral("vibratoRate")] = schicht.vibratoHz;
    o[QStringLiteral("vibratoDepth")] = schicht.vibratoCent;
    o[QStringLiteral("vibratoDelay")] = schicht.vibratoEinsatzMs;
    o[QStringLiteral("damping")] = schicht.saiteDaempfung;
    o[QStringLiteral("level")] = schicht.laut;
    o[QStringLiteral("pan")] = schicht.pan;  // seit Fassung 8
    // seit Fassung 5 (Pitch sweep): Halbtoene neben der Note, und die Zeit des Wegs
    o[QStringLiteral("pitchStart")] = schicht.tonhoeheStartHt;
    o[QStringLiteral("pitchTime")] = schicht.tonhoeheZeitMs;
    // seit Fassung 7: der Weg des Tons als Reihe, von vorn nach hinten
    QJsonArray reihe;
    for (const SchichtBaustein& baustein : schicht.reihe) reihe.append(std::visit(BausteinAlsJson{}, baustein));
    o[QStringLiteral("path")] = reihe;
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
    neu.vibratoHz = zahl("vibratoRate", vorgabe.vibratoHz, 0.0, 20.0);
    neu.vibratoCent = zahl("vibratoDepth", vorgabe.vibratoCent, 0.0, 100.0);
    neu.vibratoEinsatzMs = zahl("vibratoDelay", vorgabe.vibratoEinsatzMs, 0.0, 5000.0);
    neu.saiteDaempfung = zahl("damping", vorgabe.saiteDaempfung, 0.9, 0.9999);
    neu.laut = zahl("level", vorgabe.laut, 0.0, 1.0);
    neu.pan = zahl("pan", vorgabe.pan, -1.0, 1.0);  // seit Fassung 8; fehlt es, steht die Schicht in der Mitte
    neu.einsatzMs = zahl("startOffset", vorgabe.einsatzMs, 0.0, 1000.0);
    // seit Fassung 4; in einer aelteren Datei und an anderen Schichten fehlen sie
    const double laengsteMs = kKlangMaxSekunden * 1000.0;
    neu.sample = wurzel.value(QStringLiteral("sample")).toString();
    neu.grundton = std::clamp(wurzel.value(QStringLiteral("rootNote")).toInt(vorgabe.grundton), 0, 127);
    neu.feinCent = zahl("fineTune", vorgabe.feinCent, -100.0, 100.0);
    neu.sampleStartMs = zahl("sampleStart", vorgabe.sampleStartMs, 0.0, laengsteMs);
    neu.schleife = wurzel.value(QStringLiteral("sustainLoop")).toBool(false);
    neu.schleifeVonMs = zahl("loopFrom", vorgabe.schleifeVonMs, 0.0, laengsteMs);
    neu.schleifeBisMs = zahl("loopTo", vorgabe.schleifeBisMs, 0.0, laengsteMs);
    // seit Fassung 5; in einer aelteren Datei fehlen sie — dann kein Pitch sweep
    neu.tonhoeheStartHt = zahl("pitchStart", vorgabe.tonhoeheStartHt, kTonhoeheStartMin, kTonhoeheStartMax);
    neu.tonhoeheZeitMs = zahl("pitchTime", vorgabe.tonhoeheZeitMs, 0.0, kTonhoeheZeitMaxMs);
    neu.reihe.clear();
    if (const QJsonValue reihe = wurzel.value(QStringLiteral("path")); reihe.isArray())
    {
        // seit Fassung 7: die Reihe steht in der Datei. Einen Baustein, den dieses Programm
        // nicht kennt, laesst es weg.
        for (const QJsonValue& eintrag : reihe.toArray())
        {
            const QJsonObject b = eintrag.toObject();
            const QString art = b.value(QStringLiteral("type")).toString();
            if (art == kBausteinFilter)
            {
                SchichtFilter filter;
                filter.art = filterArtAus(b.value(QStringLiteral("filterType")).toString());
                filter.teilton = b.value(QStringLiteral("cutoff")).toDouble(filter.teilton);
                filter.huelle = b.value(QStringLiteral("envelope")).toDouble(filter.huelle);
                filter.resonanz = b.value(QStringLiteral("resonance")).toDouble(filter.resonanz);
                neu.reihe.emplace_back(filter);
            }
            else if (art == kBausteinDrive)
                neu.reihe.emplace_back(SchichtDrive{b.value(QStringLiteral("amount")).toDouble(0.0)});
            else if (art == kBausteinHuelle)
                neu.reihe.emplace_back(SchichtHuelle{});
        }
    }
    else
    {
        // bis Fassung 6: flache Felder, gerechnet wurde fest Filter, Drive, Envelope. Ein Filter
        // mit Cutoff 0 und ein Drive mit 0 taten nichts — sie werden kein Baustein (K2 a).
        // `filterType` und `filterResonance` gibt es seit Fassung 6; fehlt die Art oder ist sie
        // unbekannt, gilt der Tiefpass.
        SchichtFilter filter;
        filter.teilton = wurzel.value(QStringLiteral("filterCutoff")).toDouble(0.0);
        filter.huelle = wurzel.value(QStringLiteral("filterEnvelope")).toDouble(0.0);
        filter.art = filterArtAus(wurzel.value(QStringLiteral("filterType")).toString());
        filter.resonanz = wurzel.value(QStringLiteral("filterResonance")).toDouble(0.0);
        if (std::isfinite(filter.teilton) && filter.teilton > 0.0) neu.reihe.emplace_back(filter);
        const double drive = wurzel.value(QStringLiteral("drive")).toDouble(0.0);
        if (std::isfinite(drive) && drive > 0.0) neu.reihe.emplace_back(SchichtDrive{drive});
        neu.reihe.emplace_back(SchichtHuelle{});
    }
    richteReihe(neu.reihe);
    return neu;
}

}  // namespace

void richteReihe(std::vector<SchichtBaustein>& reihe)
{
    const auto istHuelle = [](const SchichtBaustein& baustein) { return std::holds_alternative<SchichtHuelle>(baustein); };
    // genau eine Envelope: von mehreren bleibt die erste
    auto erste = std::find_if(reihe.begin(), reihe.end(), istHuelle);
    if (erste != reihe.end()) reihe.erase(std::remove_if(erste + 1, reihe.end(), istHuelle), reihe.end());
    const auto grenze = static_cast<std::size_t>(kBausteineMax);
    if (reihe.size() > grenze) reihe.erase(reihe.begin() + kBausteineMax, reihe.end());
    if (std::none_of(reihe.begin(), reihe.end(), istHuelle))
    {
        // sie fehlte — oder stand hinter dem achten: sie kommt ans Ende, der letzte weicht ihr
        if (reihe.size() == grenze) reihe.pop_back();
        reihe.emplace_back(SchichtHuelle{});
    }
    for (SchichtBaustein& baustein : reihe) std::visit(RichteBaustein{}, baustein);
}

SchichtFilter Schicht::filter() const
{
    for (const SchichtBaustein& baustein : reihe)
        if (const auto* werte = std::get_if<SchichtFilter>(&baustein)) return *werte;
    return {};
}

SchichtDrive Schicht::drive() const
{
    for (const SchichtBaustein& baustein : reihe)
        if (const auto* werte = std::get_if<SchichtDrive>(&baustein)) return *werte;
    return {};
}

SchichtFilter& Schicht::holeFilter()
{
    for (SchichtBaustein& baustein : reihe)
        if (auto* werte = std::get_if<SchichtFilter>(&baustein)) return *werte;
    return std::get<SchichtFilter>(*reihe.insert(reihe.begin(), SchichtFilter{}));
}

SchichtDrive& Schicht::holeDrive()
{
    for (SchichtBaustein& baustein : reihe)
        if (auto* werte = std::get_if<SchichtDrive>(&baustein)) return *werte;
    const auto huelle = std::find_if(reihe.begin(), reihe.end(), [](const SchichtBaustein& baustein) {
        return std::holds_alternative<SchichtHuelle>(baustein);
    });
    return std::get<SchichtDrive>(*reihe.insert(huelle, SchichtDrive{}));
}

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
        {"drums", "Drums"},  // seit S115 (Entscheid K3 a): Schlaege fuer eine Drum-Spur
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

QStringList eigeneInstrumenteMitSample(const QString& nennung)
{
    QStringList titel;
    if (nennung.isEmpty()) return titel;
    for (const Instrument& instrument : eigeneInstrumente())
    {
        const bool spieltEs = std::any_of(instrument.schichten.begin(), instrument.schichten.end(), [&nennung](const Schicht& s) {
            return s.schwingung == Schwingung::Sample && s.sample == nennung;
        });
        if (spieltEs) titel << instrument.titel;
    }
    return titel;
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

QString nimmZuEigenenInstrumenten(const QString& quelle)
{
    const QFileInfo info(quelle);
    const QString ordner = eigenerInstrumentOrdner();
    Instrument instrument;
    if (!info.isFile() || ordner.isEmpty() || !ladeInstrument(quelle, instrument)) return {};
    // der Name der Datei — und wenn daraus keiner wird, der Titel des Instruments
    QString grund = eigenerName(info.completeBaseName());
    if (grund.isEmpty()) grund = eigenerName(instrument.titel);
    if (grund.isEmpty()) return {};
    const auto inhaltVon = [](const QString& pfad) {
        QFile datei(pfad);
        return datei.open(QIODevice::ReadOnly) ? datei.readAll() : QByteArray();
    };
    const QByteArray inhalt = inhaltVon(quelle);
    if (inhalt.isEmpty() || !QDir().mkpath(ordner)) return {};

    for (int zahl = 1; zahl < 1000; ++zahl)
    {
        const QString name = zahl == 1 ? grund : grund.left(60) + QLatin1Char('_') + QString::number(zahl);
        const QString ziel = eigeneDatei(name);
        if (ziel.isEmpty()) return {};
        if (!QFileInfo::exists(ziel))
        {
            if (!QFile::copy(quelle, ziel)) return {};
            ++fassung();  // ein eigenes ist dazugekommen
            return QLatin1String(kEigenVorsatz) + name;
        }
        // dieselbe Datei liegt schon da (auch: sie wurde aus dem Ordner selbst gewaehlt)
        if (inhaltVon(ziel) == inhalt) return QLatin1String(kEigenVorsatz) + name;
    }
    return {};
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
        else if (schicht.schwingung == Schwingung::Rauschen)
            rechneRauschen(schicht, hz, huelle, rate, eine);
        else
            rechneSchwingung(schicht, hz, huelle, rate, eine);
        // der Klang endet bei null — auch wenn hinter der Envelope ein Filter nachschwingt
        if (schicht.schwingung != Schwingung::Saite && filterHinterHuelle(schicht)) blendeAus(eine, rate);
        // Pan (Stufe 18): das Gesetz von Spur und Muster. In der Mitte bleibt jeder Wert,
        // wie er ist — alte Instrumente rechnen Bit fuer Bit wie vorher.
        if (const double pan = std::clamp(schicht.pan, -1.0, 1.0); pan != 0.0)
        {
            const auto links = static_cast<float>(std::min(1.0, 1.0 - pan));
            const auto rechts = static_cast<float>(std::min(1.0, 1.0 + pan));
            for (std::size_t i = 0; i + 1 < eine.size(); i += 2)
            {
                eine[i] *= links;
                eine[i + 1] *= rechts;
            }
        }
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

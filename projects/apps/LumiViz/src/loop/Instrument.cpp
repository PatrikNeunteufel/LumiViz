/**
 * @file Instrument.cpp
 * @brief Loop: gerechnete Instrumente (Stufe 9)
 */

#include "loop/Instrument.hpp"

#include "loop/Kit.hpp"
#include "loop/Saitenplan.hpp"  // kAbdaempfenMs (Stufe 23b)
#include "loop/Sample.hpp"
#include "shape/Kontur.hpp"   // die Farbe einer Shape-Schicht (Stufe 20d)
#include "shape/Tonform.hpp"  // die Schwingung Form (Stufe 19c)

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
#include <complex>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

namespace lumi::loop
{

namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr int kTafel = 2048;        ///< Abtastwerte eines Wellenzugs
constexpr int kTeiltoeneMax = 512;  ///< mehr traegt die Tafel nicht sauber
constexpr double kPegel = 0.5;      ///< Luft nach oben: ein Akkord addiert sich (Entscheid L6 a)
/// Die hoechste Spitze eines Saitentons bei Lautstaerke 1, vor dem Signal path —
/// die Grenze der Bemessung eines magnetischen Tonabnehmers (Entscheid S-K11 a).
constexpr double kSpitzeSaiteMax = 0.9;

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
    case Schwingung::Form:
        break;  // kein Wellenzug: die Saite, die Aufnahme, das Rauschen und die Form rechnen eigens
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

/// Wie weit ein Filter mit dem Frequenzgang `h` (bei der Kreisfrequenz `w`, je
/// Abtastwert) den Ton verzoegert, in Abtastwerten.
double laufzeit(std::complex<double> h, double w)
{
    return -std::arg(h) / w;
}

/// Ein Tiefpass erster Ordnung (1 + a) / (1 + a z^-1), a in (−1, 0]: bei 0
/// laesst er alles, je kleiner a, desto mehr nimmt er den Hoehen. Bei DC 1.
std::complex<double> tiefpassEins(double a, double w)
{
    return (1.0 + a) / (1.0 + a * std::polar(1.0, -w));
}

/// Ein Allpass erster Ordnung (a + z^-1) / (1 + a z^-1).
std::complex<double> allpassEins(double a, double w)
{
    const std::complex<double> z1 = std::polar(1.0, -w);
    return (a + z1) / (1.0 + a * z1);
}

/// Der Umlauf einer Saite *Extended KS* oder *Waveguide* (Stufe 23c, 23d):
/// der Verlust- und der Steifheitsfilter, wie sie in der Saitensynthese
/// ueblich entworfen werden (Vaelimaeki u. a.), und die Laenge der Verzoegerung.
///
/// - ein **Tiefpass erster Ordnung mit Verstaerkung** (Verlust): der Grundton
///   faellt in *Ring time* um 60 dB, Toene bei 2 kHz in *Ring time* mal
///   10^(−2 · High damping) — beides gleich bei jeder Tonhoehe;
/// - **acht Allpaesse** (Steifheit): sie lassen hohe Teiltoene schneller
///   umlaufen; eingestellt so, dass der achte Teilton liegt, wo ihn die
///   Inharmonizitaet B = 0,001 · Stiffness² hinlegt (f_n = n f_1 √(1 + B n²)).
///
/// Die Verzoegerung ist um die Laufzeit der Filter beim Grundton kuerzer — so
/// stimmt die Tonhoehe. Bei keiner Frequenz waechst der Ton im Umlauf.
/// `dicke`, 0 … 1 (String variation): macht die Saite dunkler und steifer.
struct Umlauf
{
    static constexpr int kAllpaesse = 8;
    double a = 0.0;             ///< Tiefpass des Verlusts
    double verstaerkung = 1.0;  ///< je Umlauf
    double c = 0.0;             ///< Allpaesse der Steifheit; 0 = keine
    double verzug = 2.0;        ///< Laenge der Verzoegerung, in Abtastwerten
    std::size_t groesse = 6;    ///< Platz im Ring
    [[nodiscard]] bool mitAllpass() const { return c != 0.0; }
};

Umlauf entwerfeUmlauf(const Schicht& schicht, double hz, int rate, double dicke)
{
    Umlauf u;
    const double periode = rate / hz;
    const double w0 = 2.0 * kPi * hz / rate;

    // --- Verlust: Ziel je Umlauf beim Grundton und bei 2 kHz (hoeher, wenn der Ton es ist)
    const double sekunden = std::clamp(schicht.klingSekunden, kKlingSekundenMin, kKlingSekundenMax);
    const double hoehen = std::clamp(schicht.hoehenDaempfung + 0.4 * dicke, 0.0, 1.0);
    const double zielGrund = std::pow(10.0, -3.0 / (hz * sekunden));
    const double hzHoch = std::min(0.45 * rate, std::max(2000.0, 1.5 * hz));
    const double wHoch = 2.0 * kPi * hzHoch / rate;
    const double zielHoch = std::pow(10.0, -3.0 / (hz * sekunden * std::pow(10.0, -2.0 * hoehen)));
    // a so, dass der Tiefpass bei 2 kHz gegen den Grundton um zielHoch / zielGrund faellt
    const double verhaeltnis = zielHoch / zielGrund;
    const auto faellt = [&](double a) { return std::abs(tiefpassEins(a, wHoch)) / std::abs(tiefpassEins(a, w0)); };
    if (verhaeltnis < 1.0)
    {
        double flach = 0.0;
        double steil = -0.999;
        if (faellt(steil) >= verhaeltnis)
            u.a = steil;
        else
        {
            for (int i = 0; i < 60; ++i)
            {
                const double mitte = 0.5 * (flach + steil);
                (faellt(mitte) > verhaeltnis ? flach : steil) = mitte;
            }
            u.a = 0.5 * (flach + steil);
        }
    }
    const double a = u.a;
    // die Verstaerkung: der Grundton trifft sein Ziel — hoechstens knapp unter 1, auch bei DC
    u.verstaerkung = std::min(0.99995, zielGrund / std::abs(tiefpassEins(a, w0)));

    // --- Steifheit: acht Allpaesse mit gleichem c; der Bezugsteilton liegt unter der Bandgrenze
    const double steif = std::clamp(schicht.steifheit + 0.4 * dicke, 0.0, 1.0);
    const double inharmonisch = 0.001 * steif * steif;
    const int bezug = std::min(8, static_cast<int>(0.45 * kPi / w0));
    // die Laufzeit der Filter beim Grundton, und daraus die Laenge der Verzoegerung
    const auto filterLaufzeit = [&](double koeffizient, double w) {
        return laufzeit(tiefpassEins(a, w), w) + Umlauf::kAllpaesse * laufzeit(allpassEins(koeffizient, w), w);
    };
    const auto verzugFuer = [&](double koeffizient) { return periode - filterLaufzeit(koeffizient, w0); };
    if (inharmonisch > 0.0 && bezug >= 2)
    {
        // die Phase eines Umlaufs: Verzoegerung plus Filter; ein Teilton liegt, wo sie 2 pi n ist
        const auto teiltonW = [&](double koeffizient, int n) {
            const double verzug = verzugFuer(koeffizient);
            const auto phase = [&](double w) { return w * verzug + w * filterLaufzeit(koeffizient, w); };
            double unten = (n - 0.5) * w0;
            double oben = std::min(kPi * 0.999, (n + 1.5) * w0);
            for (int i = 0; i < 60; ++i)
            {
                const double mitte = 0.5 * (unten + oben);
                (phase(mitte) < 2.0 * kPi * n ? unten : oben) = mitte;
            }
            return 0.5 * (unten + oben);
        };
        const double zielVerhaeltnis =
            bezug * std::sqrt(1.0 + inharmonisch * bezug * bezug) / std::sqrt(1.0 + inharmonisch);
        double ohne = 0.0;
        double mit = -0.95;
        // die Verzoegerung darf nicht unter zwei Abtastwerte fallen: so weit geht c hoechstens
        while (verzugFuer(mit) < 2.0 && mit < -1e-4) mit *= 0.5;
        if (teiltonW(mit, bezug) / w0 <= zielVerhaeltnis)
            u.c = mit;
        else
        {
            for (int i = 0; i < 50; ++i)
            {
                const double mitte = 0.5 * (ohne + mit);
                (teiltonW(mitte, bezug) / w0 < zielVerhaeltnis ? ohne : mit) = mitte;
            }
            u.c = 0.5 * (ohne + mit);
        }
    }
    u.verzug = std::max(2.0, u.mitAllpass() ? verzugFuer(u.c) : periode - laufzeit(tiefpassEins(a, w0), w0));
    u.groesse = static_cast<std::size_t>(u.verzug) + 4;
    return u;
}

/// Zweimal ein Tiefpass erster Ordnung mit der Grenze nach der Haerte des
/// Anschlags (Pluck): vom Doppelten bis zum Achtzigfachen des Grundtons; 0 =
/// er laesst alles (die Grenze liegt ueber der Bandgrenze).
double glaettenFuer(const Schicht& schicht, double hz, int rate)
{
    const double haerte = std::clamp(schicht.anschlagHaerte, 0.0, 1.0);
    const double grenzeHz = hz * 2.0 * std::pow(40.0, haerte);
    return grenzeHz >= 0.45 * rate ? 0.0 : std::exp(-2.0 * kPi * grenzeHz / rate);
}

/// Ohne Gleichanteil und gleich laut wie der Stoss von Classic (Effektivwert 1/sqrt(6)).
void normiereStoss(std::vector<double>& ring)
{
    double mittel = 0.0;
    for (const double wert : ring) mittel += wert;
    mittel /= static_cast<double>(ring.size());
    double quadrate = 0.0;
    for (double& wert : ring)
    {
        wert -= mittel;
        quadrate += wert * wert;
    }
    const double effektiv = std::sqrt(quadrate / static_cast<double>(ring.size()));
    if (effektiv > 0.0)
        for (double& wert : ring) wert *= (1.0 / std::sqrt(6.0)) / effektiv;
}

/// Den Umlauf rechnen: lesen, Verlust und Steifheit, zurueckschreiben. Was in
/// den Ton geht, sagt `abnahme(y, lese)`: `y` ist die Welle, die am Steg
/// ankommt; `lese(d)` die Welle, die vor `d` Abtastwerten den Steg verliess
/// (der eben geschriebene Wert ist d = 0).
template <class Abnahme>
void laufeUm(const Schicht& schicht, double hz, const Huelle& huelle, int rate, const Umlauf& u, std::vector<double> ring,
             std::vector<float>& daten, Abnahme abnahme)
{
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    Reihe reihe(schicht, rate, false);  // die Saite ueberspringt die Filter wie Classic
    const std::size_t groesse = u.groesse;
    const double pegel = kPegel * schicht.laut;
    const double anstieg = std::max(1.0, schicht.anstiegMs * rate / 1000.0);
    std::array<double, Umlauf::kAllpaesse> apEin{};
    std::array<double, Umlauf::kAllpaesse> apAus{};
    std::size_t schreib = 0;
    double tief = 0.0;  // das Gedaechtnis des Tiefpasses
    const auto lese = [&](double d) {
        double stelle = static_cast<double>(schreib) - d;
        while (stelle < 0.0) stelle += static_cast<double>(groesse);
        const auto i = static_cast<std::size_t>(stelle);
        const double rest = stelle - static_cast<double>(i);
        return ring[i % groesse] + (ring[(i + 1) % groesse] - ring[i % groesse]) * rest;
    };
    for (std::int64_t t = 0; t < frames; ++t)
    {
        const double y = lese(u.verzug);
        // Verlust: y_n = g (1 + a) x_n − a y_(n−1)
        tief = u.verstaerkung * (1.0 + u.a) * y - u.a * tief;
        double v = tief;
        if (u.mitAllpass())
            for (std::size_t k = 0; k < static_cast<std::size_t>(Umlauf::kAllpaesse); ++k)
            {
                const double aus = u.c * v + apEin[k] - u.c * apAus[k];
                apEin[k] = v;
                apAus[k] = aus;
                v = aus;
            }
        ring[schreib] = v;
        const double ton = abnahme(y, lese);
        schreib = (schreib + 1) % groesse;

        // die Saite klingt von selbst aus; die Huelle nimmt nur Einsatz und Ende (wie Classic)
        double h = std::min(1.0, static_cast<double>(t) / anstieg);
        if (t >= huelle.ende)
        {
            const double nach = static_cast<double>(t - huelle.ende);
            h *= std::exp(-nach / huelle.ausklang) * std::max(0.0, 1.0 - nach / static_cast<double>(huelle.aus));
        }
        const auto aus = static_cast<float>(reihe.bei(ton, h, hz) * pegel);
        daten[static_cast<std::size_t>(2 * t)] = aus;
        daten[static_cast<std::size_t>(2 * t + 1)] = aus;
    }
}

/// Ein Schwinger des Korpus: eine Mode von Decke oder Boden, oder die Luft im
/// Schallloch — ein Kolben mit wirksamer Masse und Flaeche (Wright 1996,
/// Gl. A.20–A.24, fuer beliebig viele Kolben).
struct KorpusKolben
{
    double masse = 1.0;      ///< wirksame Masse, kg
    double steife = 0.0;     ///< Steife ohne die Luft im Korpus, N/m
    double daempfung = 0.0;  ///< R, Ns/m
    double flaeche = 0.0;    ///< wirksame Flaeche, m^2; nach aussen positiv
    bool getrieben = false;  ///< die Kraft am Steg greift an (Decke) — nicht am Boden, nicht an der Luft
};

/// Ein Kolben aus den entkoppelten Werten einer Mode, wie Wright sie angibt:
/// die Frequenz bei geschlossenem Schallloch — die Luft im Korpus federt mit
/// alpha A^2 (Gl. A.12, A.28) —, die wirksame Masse und Flaeche, die Guete.
KorpusKolben kolbenAusMode(double hz, double masse, double flaeche, double guete, bool getrieben, double alpha)
{
    const double w = 2.0 * kPi * hz;
    return {masse, masse * w * w - alpha * flaeche * flaeche, masse * w / guete, flaeche, getrieben};
}

/// Der Korpus (Stufe 23d, S-K8 a; Classical S-K10 a): Decke, Boden und Luft
/// als gekoppelte Schwinger nach Christensen und Vistisen (1980), wie Wright
/// (1996) Gl. A.1 bis A.7 und A.20 bis A.24 sie schreibt — je Kolben i
///
///   m_i x_i'' = F_i − k_i x_i − R_i x_i' + A_i P,   P = −(rho c² / V) Σ A_j x_j,
///
/// die Luft im Schallloch ein Kolben ohne eigene Steife. Die Kraft F greift
/// am Steg an, an jeder Mode der Decke (wirksame Massen am Steg); abgestrahlt
/// wird, was der Volumenfluss U = Σ A_i x_i' sich aendert (eine Punktquelle,
/// Wright Gl. 4.17). Die Werte:
/// - *Parlor*: Hess (2013), Tab. 1 — eine Decke und die Luft;
/// - *Classical guitar*: Wright, Satz TOP26 (S. 138–140): zehn Moden der Decke
///   und B(1,1) aus Tab. 7.1, Luft f_h = 133 Hz, Q_h = 30, A_h, V.
/// Gerechnet mit der Trapezregel — stabil, und bei 900 Hz an 44,1 kHz kaum
/// verschoben. Weil alle Kolben nur ueber den einen Druck P gekoppelt sind,
/// loest sich der Schritt je Kolben, und P ergibt sich aus einer Gleichung.
class Korpus
{
public:
    Korpus(KorpusArt art, int rate)
    {
        if (rate <= 0) return;
        if (art == KorpusArt::Parlor)
            setzeParlor();
        else if (art == KorpusArt::Klassik)
            setzeKlassik();
        else
            return;
        m_an = true;
        const double h = 1.0 / rate;
        const double nennerSumme = [&] {
            double summe = 0.0;
            for (Zustand& z : m_kolben)
            {
                const KorpusKolben& k = z.kolben;
                const double traege = 4.0 * k.masse / (h * h) + 2.0 * k.daempfung / h;
                z.g = 1.0 / (traege + k.steife);
                z.vorher = traege - k.steife;
                z.v4 = 4.0 * k.masse / h;
                summe += k.flaeche * k.flaeche * z.g;
            }
            return summe;
        }();
        m_nenner = 1.0 + m_alpha * nennerSumme;
        m_h = h;
    }
    [[nodiscard]] bool an() const { return m_an; }
    /// Ein Abtastwert: die Kraft am Steg hinein, der abgestrahlte Schall heraus.
    [[nodiscard]] double schritt(double kraft)
    {
        if (!m_an) return 0.0;
        // der Druck im Korpus jetzt, und der Schall jetzt: die Aenderung des Volumenflusses
        double volumen = 0.0;
        for (const Zustand& z : m_kolben) volumen += z.kolben.flaeche * z.x;
        const double druck = -m_alpha * volumen;
        double schall = 0.0;
        // weiter zum naechsten Abtastwert: die Trapezregel nimmt diese Kraft und die davor
        // (die naechste ist noch unbekannt — der Korpus hoert die Saite einen Abtastwert spaeter)
        double summe = 0.0;
        for (Zustand& z : m_kolben)
        {
            const KorpusKolben& k = z.kolben;
            const double jetzt = (k.getrieben ? kraft : 0.0) + k.flaeche * druck;
            schall += k.flaeche * (jetzt - k.steife * z.x - k.daempfung * z.v) / k.masse;
            const double davor = (k.getrieben ? m_davor : 0.0) + k.flaeche * druck;
            z.basis = z.g * (davor + z.vorher * z.x + z.v4 * z.v) + (k.getrieben ? z.g * kraft : 0.0);
            summe += k.flaeche * z.basis;
        }
        const double druckNeu = -m_alpha * summe / m_nenner;
        for (Zustand& z : m_kolben)
        {
            const double x = z.basis + z.g * z.kolben.flaeche * druckNeu;
            z.v = 2.0 * (x - z.x) / m_h - z.v;
            z.x = x;
        }
        m_davor = kraft;
        return schall;
    }

private:
    struct Zustand
    {
        KorpusKolben kolben;
        double g = 0.0;       // x_n+1 = basis + g u_n+1
        double vorher = 0.0;  // 4m/h² + 2R/h − k
        double v4 = 0.0;      // 4m/h
        double basis = 0.0;
        double x = 0.0;
        double v = 0.0;
    };

    void setzeParlor()
    {
        // Hess, Tab. 1
        constexpr double c = 343.0;          // Schallgeschwindigkeit, m/s
        constexpr double rho = 1.205;        // Dichte der Luft, kg/m^3
        constexpr double volumen = 0.01183;  // Korpus, m^3
        m_alpha = rho * c * c / volumen;
        // Decke: Masse, Steife, Daempfung, wirksame Flaeche
        m_kolben.push_back({KorpusKolben{0.06653, 9.995e4, 8.994, 0.03747, true}});
        // Luft im Schallloch: Masse, Daempfung, Flaeche des Schalllochs
        m_kolben.push_back({KorpusKolben{0.001182, 0.0, 0.03803, 0.008107, false}});
    }

    void setzeKlassik()
    {
        // Wright, S. 138: A_h und V; die Luft wie beim Parlor (Wright nennt rho und c nicht)
        constexpr double c = 343.0;
        constexpr double rho = 1.205;
        constexpr double volumen = 14e-3;     // m^3
        constexpr double schallloch = 5.7e-3;  // m^2
        m_alpha = rho * c * c / volumen;
        // Tab. 7.1: Frequenz (entkoppelt), wirksame Masse, wirksame Flaeche, Guete
        struct Mode
        {
            double hz, masse, flaeche, guete;
            bool decke;
        };
        constexpr std::array<Mode, 11> top26 = {{
            {185.0, 0.080, 0.0381, 20.0, true},     // T(1,1)
            {215.0, 0.150, 0.0250, 25.0, false},    // B(1,1), der Boden
            {243.9, 0.488, 0.0, 36.0, true},        // T(2,1)
            {435.8, 2.715, -0.1215, 47.0, true},    // T(1,2)
            {506.8, 0.256, -0.0265, 35.0, true},    // T(3,1)
            {612.0, 1.093, 0.0, 22.0, true},        // T(4,1)
            {674.3, 0.490, -0.0576, 28.0, true},    // T(1,3)
            {737.7, 2.049, 0.0, 35.0, true},        // T(2,2)
            {863.6, 0.272, 0.0097, 25.0, true},     // T(1,4)
            {878.6, 0.646, 0.0107, 40.0, true},     // T(5,1)1
            {908.0, 0.437, 0.0017, 40.0, true},     // T(5,1)2
        }};
        for (const Mode& m : top26)
            m_kolben.push_back({kolbenAusMode(m.hz, m.masse, m.flaeche, m.guete, m.decke, m_alpha)});
        // die Luft: f_h = 133 Hz, Q_h = 30 (Tab. 7.1); aus omega_h² = alpha A_h² / m_h (Gl. A.14)
        const double wh = 2.0 * kPi * 133.0;
        const double mh = m_alpha * schallloch * schallloch / (wh * wh);
        m_kolben.push_back({KorpusKolben{mh, 0.0, mh * wh / 30.0, schallloch, false}});
    }

    bool m_an = false;
    std::vector<Zustand> m_kolben;
    double m_alpha = 0.0;
    double m_nenner = 1.0;
    double m_h = 0.0;
    double m_davor = 0.0;
};

/// Den Umlauf rechnen, mit Korpus (`amSteg`: der Ton wird am Steg abgenommen —
/// ohne Tonabnehmer). Der Korpus ist so bemessen, dass er im ersten Zehntel
/// einer Sekunde so laut ist wie die Saite allein (Effektivwert, der Anfang
/// einmal vorab gerechnet); Body amount mischt beide. `fabrik()` gibt je Lauf
/// eine frische Abnahme (mit eigenem Gedaechtnis).
template <class Fabrik>
void laufeMitKorpus(const Schicht& schicht, double hz, const Huelle& huelle, int rate, const Umlauf& u,
                    std::vector<double> ring, std::vector<float>& daten, Fabrik fabrik, bool amSteg)
{
    const double anteil = std::clamp(schicht.korpusAnteil, 0.0, 1.0);
    if (schicht.korpus == KorpusArt::Keiner || !amSteg || anteil <= 0.0)
    {
        laufeUm(schicht, hz, huelle, rate, u, std::move(ring), daten, fabrik());
        return;
    }
    std::vector<float> anfang(static_cast<std::size_t>(2 * std::max(1, rate / 10)), 0.0f);
    Korpus probe(schicht.korpus, rate);
    auto probeAbnahme = fabrik();
    double quadrateSaite = 0.0;
    double quadrateKorpus = 0.0;
    laufeUm(schicht, hz, huelle, rate, u, ring, anfang, [&](double y, const auto& lese) {
        const double saite = probeAbnahme(y, lese);
        const double schall = probe.schritt(saite);
        quadrateSaite += saite * saite;
        quadrateKorpus += schall * schall;
        return saite;
    });
    const double faktor = quadrateKorpus > 0.0 ? std::sqrt(quadrateSaite / quadrateKorpus) : 0.0;
    Korpus korpus(schicht.korpus, rate);
    auto abnahme = fabrik();
    laufeUm(schicht, hz, huelle, rate, u, std::move(ring), daten, [&](double y, const auto& lese) {
        const double saite = abnahme(y, lese);
        return (1.0 - anteil) * saite + anteil * faktor * korpus.schritt(saite);
    });
}

/// Die Saite *Extended KS* (Stufe 23c; S-K1 b, S-K3 a): das erweiterte
/// Karplus-Strong nach Jaffe und Smith (1983). Ein Stoss Rauschen — durch die
/// Haerte des Anschlags gefiltert und an der Anschlagstelle gekaemmt (die
/// Teiltoene mit einem Knoten dort fehlen) — laeuft durch den Umlauf
/// (`entwerfeUmlauf`). Der Ton ist die Welle, die am Steg ankommt.
void rechneSaiteKs(const Schicht& schicht, int hoehe, double hz, const Huelle& huelle, int rate, double dicke,
                   std::vector<float>& daten)
{
    const Umlauf u = entwerfeUmlauf(schicht, hz, rate, dicke);
    const double periode = rate / hz;
    const std::size_t groesse = u.groesse;
    std::vector<double> ring(groesse, 0.0);

    // --- der Stoss: dieselbe feste Folge je Tonhoehe wie Classic, durch die Haerte gefiltert
    std::uint32_t folge = 12345u + static_cast<std::uint32_t>(hoehe) * 7919u;
    const double glaetten = glaettenFuer(schicht, hz, rate);
    std::vector<double> stoss(groesse, 0.0);
    double eins = 0.0;
    double zwei = 0.0;
    for (double& wert : stoss)
    {
        folge = folge * 1664525u + 1013904223u;
        const double rauschen = static_cast<double>(folge >> 8) / 8388608.0 - 1.0;
        eins = (1.0 - glaetten) * rauschen + glaetten * eins;
        zwei = (1.0 - glaetten) * eins + glaetten * zwei;
        wert = zwei;
    }
    // an der Anschlagstelle gekaemmt: (1 - z^-M), M = Anteil mal Periode, im Umlauf gezaehlt
    const auto kamm = static_cast<std::size_t>(std::max<long long>(
        1, std::llround(std::clamp(schicht.anschlagOrt, kAnschlagOrtMin, kAnschlagOrtMax) * periode)));
    for (std::size_t i = 0; i < groesse; ++i) ring[i] = stoss[i] - stoss[(i + groesse - kamm % groesse) % groesse];
    normiereStoss(ring);

    laufeMitKorpus(
        schicht, hz, huelle, rate, u, std::move(ring), daten,
        [] { return [](double y, const auto&) { return y; }; }, true);
}

/// Der Hochpass eines Piezo-Tonabnehmers (S-K5 a): eine kapazitive Quelle an
/// einem Widerstand, f = 1 / (2 pi R C) — mit 1,5 nF und 1 MOhm 106 Hz
/// (Zollner, *Physics of the Electric Guitar* 6.2).
inline constexpr double kPiezoHochpassHz = 106.0;

/// Die Saite *Waveguide* (Stufe 23d; S-K1 a, S-K5 a, S-U3 a): die Saite als
/// zwei gegenlaeufige Wellen zwischen Steg und Sattel. Gerechnet wird ein
/// Umlauf (`entwerfeUmlauf`) — eine Welle laeuft vom Steg zum Sattel, kehrt
/// dort mit umgedrehtem Vorzeichen um und kommt zurueck; am Steg sitzen
/// Verlust und Steifheit. Die Auslenkung an der Stelle x (Anteil der Saite vom
/// Steg) ist die Summe beider Wellen dort: die eine vor x · L/2 vom Steg
/// losgelaufen, die andere vor (2 − x) · L/2 und am Sattel umgedreht.
///
/// - **Anschlag:** die Saite wird an der Anschlagstelle zu einem Dreieck
///   ausgelenkt und losgelassen (beide Wellen je die Haelfte); die Haerte
///   rundet die Spitze.
/// - **Abnahme:** ohne Tonabnehmer die Welle am Steg (wie akustisch). Ein
///   magnetischer Tonabnehmer misst die Geschwindigkeit der Saite an seiner
///   Stelle (Single coil: eine Stelle; Humbucker: zwei, 18 mm auseinander, zur
///   Haelfte je); ein Piezo die Kraft am Steg, durch seinen Hochpass. Ein
///   magnetischer Tonabnehmer ist so bemessen, dass er im ersten Zehntel einer
///   Sekunde so laut ist wie die Saite am Steg (im Effektivwert), mit der Spitze
///   des fertigen Tons hoechstens bei `kSpitzeSaiteMax` (S-K11 a) — der Anfang
///   wird dafuer einmal vorab gerechnet. *Pickup select* nimmt einen oder
///   beide (zur Haelfte je).
void rechneSaiteWg(const Schicht& schicht, double hz, const Huelle& huelle, int rate, double dicke, double mensurMm,
                   std::vector<float>& daten)
{
    const Umlauf u = entwerfeUmlauf(schicht, hz, rate, dicke);
    const double laenge = u.verzug;  // der Umlauf: zweimal die Saite
    const std::size_t groesse = u.groesse;

    // --- der Anschlag: ein Dreieck mit der Spitze an der Anschlagstelle, die Spitze durch die Haerte gerundet
    const double ort = std::clamp(schicht.anschlagOrt, kAnschlagOrtMin, kAnschlagOrtMax);
    const auto dreieck = [ort](double x) { return x <= ort ? x / ort : (1.0 - x) / (1.0 - ort); };
    std::vector<double> ring(groesse, 0.0);
    for (std::size_t alter = 0; alter < groesse; ++alter)
    {
        const double k = std::min(static_cast<double>(alter), laenge);
        // bis zur halben Laenge die Welle zum Sattel hin, danach die zurueck (am Sattel umgedreht)
        const double wert = k <= 0.5 * laenge ? 0.5 * dreieck(2.0 * k / laenge) : -0.5 * dreieck(2.0 - 2.0 * k / laenge);
        ring[(groesse - alter) % groesse] = wert;
    }
    if (const double glaetten = glaettenFuer(schicht, hz, rate); glaetten > 0.0)
    {
        // im Kreis geglaettet: zwei Runden, damit der Anfang nicht anders ist als das Ende
        double eins = 0.0;
        double zwei = 0.0;
        for (int runde = 0; runde < 2; ++runde)
            for (std::size_t i = 0; i < groesse; ++i)
            {
                eins = (1.0 - glaetten) * ring[i] + glaetten * eins;
                zwei = (1.0 - glaetten) * eins + glaetten * zwei;
                if (runde == 1) ring[i] = zwei;
            }
    }
    normiereStoss(ring);

    // --- die Abnahme
    const double mensur = mensurMm > 0.0 ? mensurMm : kMensurVorgabeMm;
    struct Stelle
    {
        double x = 0.0;      ///< Anteil vom Steg
        double davor = 0.0;  ///< die Auslenkung im Abtastwert davor
        bool erster = true;  ///< noch kein Abtastwert: die Saite ruht beim Loslassen (Geschwindigkeit 0)
    };
    struct Platz
    {
        AbnehmerArt art = AbnehmerArt::Keiner;
        std::array<Stelle, 2> stellen{};
        double piezoDavorEin = 0.0;
        double piezoDavorAus = 0.0;
        double faktor = 1.0;  ///< die Bemessung eines magnetischen Tonabnehmers
        // die Resonanz mit Kabel und Last: ein Tiefpass zweiter Ordnung (RBJ), normiert auf a0
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
        [[nodiscard]] double resonanz(double x)
        {
            const double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            return y;
        }
    };
    const auto platz = [&](const Abnehmer& a) {
        Platz p;
        p.art = a.art;
        const double x = std::clamp(a.mmVomSteg / mensur, 0.002, 0.998);
        p.stellen[0].x = x;
        p.stellen[1].x = std::clamp(x + kHumbuckerAbstandMm / mensur, 0.002, 0.998);
        if (const Resonanz r = abnehmerResonanz(a.art, schicht.kabelPf); r.hz > 0.0)
        {
            const double w = 2.0 * kPi * std::min(r.hz, 0.45 * rate) / rate;
            const double alpha = std::sin(w) / (2.0 * r.guete);
            const double a0 = 1.0 + alpha;
            p.b0 = 0.5 * (1.0 - std::cos(w)) / a0;
            p.b1 = (1.0 - std::cos(w)) / a0;
            p.b2 = p.b0;
            p.a1 = -2.0 * std::cos(w) / a0;
            p.a2 = (1.0 - alpha) / a0;
        }
        return p;
    };
    const bool hals = schicht.abnehmerHals.art != AbnehmerArt::Keiner;
    const bool steg = schicht.abnehmerSteg.art != AbnehmerArt::Keiner;
    const bool nimmHals = hals && (schicht.abnehmerWahl != AbnehmerWahl::Steg || !steg);
    const bool nimmSteg = steg && (schicht.abnehmerWahl != AbnehmerWahl::Hals || !hals);
    const double piezo = 1.0 / (1.0 + 2.0 * kPi * kPiezoHochpassHz / rate);

    // ein Tonabnehmer an seinem Platz: was er bei jedem Abtastwert abgibt (`y` die Welle am Steg)
    const auto signal = [&](Platz& p, double y, const auto& lese) {
        if (p.art == AbnehmerArt::Piezo)
        {
            // die Kraft am Steg, durch den Hochpass
            const double aus = piezo * (p.piezoDavorAus + y - p.piezoDavorEin);
            p.piezoDavorEin = y;
            p.piezoDavorAus = aus;
            return aus;
        }
        // magnetisch: die Geschwindigkeit der Saite an der Stelle (bei zwei Spulen je zur Haelfte)
        const int spulen = p.art == AbnehmerArt::Humbucker ? 2 : 1;
        double summe = 0.0;
        for (int s = 0; s < spulen; ++s)
        {
            Stelle& stelle = p.stellen[static_cast<std::size_t>(s)];
            const double jetzt = lese(stelle.x * 0.5 * laenge) - lese((2.0 - stelle.x) * 0.5 * laenge);
            if (stelle.erster)
            {
                // die Saite ist ausgelenkt, aber in Ruhe — ohne das waere der erste Abtastwert ein
                // Sprung um die ganze Auslenkung, und die Bemessung richtete sich nach ihm
                stelle.davor = jetzt;
                stelle.erster = false;
            }
            summe += jetzt - stelle.davor;
            stelle.davor = jetzt;
        }
        return p.faktor * p.resonanz(summe / spulen);
    };

    std::array<Platz, 2> plaetze = {platz(schicht.abnehmerHals), platz(schicht.abnehmerSteg)};
    if ((nimmHals && plaetze[0].art != AbnehmerArt::Piezo) || (nimmSteg && plaetze[1].art != AbnehmerArt::Piezo))
    {
        // die Bemessung: ein magnetischer Tonabnehmer ist im ersten Zehntel einer Sekunde so laut
        // wie die Saite am Steg (im Effektivwert) — dafuer wird der Anfang einmal vorab gerechnet
        std::vector<float> anfang(static_cast<std::size_t>(2 * std::max(1, rate / 10)), 0.0f);
        std::array<Platz, 2> probe = plaetze;
        std::array<double, 2> quadrate{};
        std::array<double, 2> spitzen{};
        double quadrateSteg = 0.0;
        laufeUm(schicht, hz, huelle, rate, u, ring, anfang, [&](double y, const auto& lese) {
            quadrateSteg += y * y;
            for (std::size_t k = 0; k < 2; ++k)
                if (probe[k].art != AbnehmerArt::Keiner)
                {
                    const double s = signal(probe[k], y, lese);
                    quadrate[k] += s * s;
                    spitzen[k] = std::max(spitzen[k], std::abs(s));
                }
            return y;
        });
        // so laut wie am Steg (S-K11 a) — die Spitze darf ueber der am Steg liegen (die
        // Geschwindigkeit hebt jeden Teilton an, ein tiefer Ton nah am Steg hat Hunderte), der
        // fertige Ton aber hoechstens bei kSpitzeSaiteMax
        for (std::size_t k = 0; k < 2; ++k)
            if (plaetze[k].art != AbnehmerArt::Keiner && plaetze[k].art != AbnehmerArt::Piezo && quadrate[k] > 0.0)
                plaetze[k].faktor = std::min(std::sqrt(quadrateSteg / quadrate[k]), kSpitzeSaiteMax / kPegel / spitzen[k]);
    }

    // je Lauf eine frische Abnahme mit eigenem Gedaechtnis der Tonabnehmer
    const auto fabrik = [&] {
        return [&signal, nimmHals, nimmSteg, eigene = plaetze](double y, const auto& lese) mutable {
            if (!nimmHals && !nimmSteg) return y;  // akustisch: die Welle am Steg
            const double vomHals = nimmHals ? signal(eigene[0], y, lese) : 0.0;
            const double vomSteg = nimmSteg ? signal(eigene[1], y, lese) : 0.0;
            return nimmHals && nimmSteg ? 0.5 * (vomHals + vomSteg) : vomHals + vomSteg;
        };
    };
    laufeMitKorpus(schicht, hz, huelle, rate, u, std::move(ring), daten, fabrik, !nimmHals && !nimmSteg);
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

/// Die Form (Stufe 19c, Shape_Konzept §5): der Stift faehrt mit der Tonhoehe
/// der Note um die Kontur, der Ton ist seine Bewegung. Gerechnet wird je
/// Abtastwert die Reihe selbst — Harmonics darf sich mit der Huellkurve von
/// Wert zu Wert aendern, ohne dass etwas springt (Z4 a). Stimmen, Verstimmung,
/// Vibrato und Pitch sweep wie bei `rechneSchwingung`.
void rechneForm(const Schicht& schicht, double hz, const Huelle& huelle, int rate, std::vector<float>& daten)
{
    const std::shared_ptr<const shape::Tonform> form = shape::tonformFuer(schicht.form, schicht.formKontur);
    if (form == nullptr || form->leer()) return;  // keine Form: die Schicht schweigt
    const auto frames = static_cast<std::int64_t>(daten.size() / 2);
    const Tonweg weg(schicht, rate);
    // Die Bandgrenze je Note: kein Teilton ueber 0,45 der Abtastrate. Vibrato und Verstimmung
    // heben den Ton leicht, eine fallende Note beginnt hoeher (wie bei den anderen Schwingungen).
    const double hoechste = std::max(1.0, 0.45 * rate / (hz * 1.06 * weg.hoechstes()));
    const int stimmen = std::clamp(schicht.stimmen, 1, 3);
    std::array<double, 3> verhaeltnis = {1.0, 1.0, 1.0};
    std::array<double, 3> phase = {0.0, 0.33, 0.67};  // fest: derselbe Ton ergibt dieselben Werte
    if (stimmen == 2) verhaeltnis = {std::exp2(-schicht.verstimmungCent / 1200.0), std::exp2(schicht.verstimmungCent / 1200.0), 1.0};
    if (stimmen == 3) verhaeltnis = {1.0, std::exp2(-schicht.verstimmungCent / 1200.0), std::exp2(schicht.verstimmungCent / 1200.0)};
    const double jeStimme = 1.0 / std::sqrt(static_cast<double>(stimmen));

    const double einsatz = std::max(1.0, schicht.vibratoEinsatzMs * rate / 1000.0);
    const bool vibrato = schicht.vibratoHz > 0.0 && schicht.vibratoCent > 0.0;
    const auto inBereich = [](double wert, double von, double bis, double ersatz) {
        return std::isfinite(wert) ? std::clamp(wert, von, bis) : ersatz;
    };
    const double harmonische = inBereich(schicht.harmonische, 1.0, kFormHarmonischeMax, 1.0);
    const double mitHuelle = inBereich(schicht.harmonischeHuelle, -kFormHuelleMax, kFormHuelleMax, 0.0);
    // Angle: z' = e^(−i·θ)·z; Start: ein anderer Anfang im Umlauf; Reverse: andersherum
    const shape::Punkt dreh = std::polar(1.0, -inBereich(schicht.winkelGrad, 0.0, 360.0, 0.0) * kPi / 180.0);
    const double start = inBereich(schicht.formStartProzent, 0.0, 100.0, 0.0) / 100.0;
    const double richtung = schicht.rueckwaerts ? -1.0 : 1.0;
    std::array<Reihe, 2> reihen = {Reihe(schicht, rate), Reihe(schicht, rate)};  // je Seite einer: jeder Filter hat sein Gedaechtnis
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
        const double teiltoene = std::clamp(harmonische + mitHuelle * h, 1.0, kFormHarmonischeMax);
        shape::Punkt z{0.0, 0.0};
        for (int s = 0; s < stimmen; ++s)
        {
            double& p = phase[static_cast<std::size_t>(s)];
            z += shape::tonwert(*form, start + richtung * p, teiltoene, hoechste / verhaeltnis[static_cast<std::size_t>(s)]);
            p += schritt * verhaeltnis[static_cast<std::size_t>(s)];
            p -= std::floor(p);
        }
        z *= jeStimme;
        z *= dreh;
        const auto links = static_cast<float>(reihen[0].bei(z.real(), h, hz) * pegel);
        daten[static_cast<std::size_t>(2 * t)] = links;
        daten[static_cast<std::size_t>(2 * t + 1)] =
            schicht.stereoXY ? static_cast<float>(reihen[1].bei(z.imag(), h, hz) * pegel) : links;
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
constexpr std::array<Wellenname, 8> kWellen = {{{Schwingung::Sinus, "sine"},
                                                {Schwingung::Dreieck, "triangle"},
                                                {Schwingung::Saege, "saw"},
                                                {Schwingung::Rechteck, "square"},
                                                {Schwingung::Saite, "string"},
                                                {Schwingung::Sample, "sample"},
                                                {Schwingung::Rauschen, "noise"},
                                                {Schwingung::Form, "shape"}}};

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
    if (schicht.schwingung == Schwingung::Form)
    {
        // seit Fassung 9; nur eine Shape-Schicht traegt sie
        o[QStringLiteral("shape")] = schicht.form;
        o[QStringLiteral("shapeContour")] = schicht.formKontur;
        o[QStringLiteral("shapeHarmonics")] = schicht.harmonische;
        o[QStringLiteral("shapeHarmonicsEnvelope")] = schicht.harmonischeHuelle;
        o[QStringLiteral("shapeAngle")] = schicht.winkelGrad;
        o[QStringLiteral("shapeStart")] = schicht.formStartProzent;
        o[QStringLiteral("shapeStereoXY")] = schicht.stereoXY;
        o[QStringLiteral("shapeReverse")] = schicht.rueckwaerts;
    }
    if (schicht.schwingung == Schwingung::Saite && schicht.saitenModell != SaitenModell::Klassisch)
    {
        // seit Fassung 10 (Stufe 23c); eine Saite Classic traegt sie nicht — sie bleibt, wie sie war
        const bool wellenleiter = schicht.saitenModell == SaitenModell::Wellenleiter;
        o[QStringLiteral("stringModel")] = wellenleiter ? QStringLiteral("waveguide") : QStringLiteral("extendedKS");
        if (wellenleiter)
        {
            // Stufe 23d: die Tonabnehmer, nur am Waveguide
            const auto abnehmer = [](const Abnehmer& a) {
                return QJsonObject{{QStringLiteral("type"), abnehmerArtName(a.art)},
                                   {QStringLiteral("position"), a.mmVomSteg}};
            };
            o[QStringLiteral("pickupNeck")] = abnehmer(schicht.abnehmerHals);
            o[QStringLiteral("pickupBridge")] = abnehmer(schicht.abnehmerSteg);
            o[QStringLiteral("cable")] = schicht.kabelPf;
            o[QStringLiteral("pickupSelect")] = schicht.abnehmerWahl == AbnehmerWahl::Hals   ? QStringLiteral("neck")
                                                : schicht.abnehmerWahl == AbnehmerWahl::Steg ? QStringLiteral("bridge")
                                                                                             : QStringLiteral("both");
        }
        o[QStringLiteral("pluckPosition")] = schicht.anschlagOrt;
        o[QStringLiteral("pluck")] = schicht.anschlagHaerte;
        o[QStringLiteral("ringTime")] = schicht.klingSekunden;
        o[QStringLiteral("highDamping")] = schicht.hoehenDaempfung;
        o[QStringLiteral("stiffness")] = schicht.steifheit;
        o[QStringLiteral("stringVariation")] = schicht.saitenUnterschied;
        o[QStringLiteral("body")] = schicht.korpus == KorpusArt::Parlor    ? QStringLiteral("parlor")
                                    : schicht.korpus == KorpusArt::Klassik ? QStringLiteral("classical")
                                                                           : QStringLiteral("none");
        o[QStringLiteral("bodyAmount")] = schicht.korpusAnteil;
        // seit Fassung 11 (Stufe 24a): die Spielweisen
        o[QStringLiteral("playing")] = QJsonObject{{QStringLiteral("upLevel"), schicht.aufschlagLaut},
                                                   {QStringLiteral("mute"), schicht.handDaempfung},
                                                   {QStringLiteral("tapLevel"), schicht.tappingLaut},
                                                   {QStringLiteral("touch"), schicht.knotenDruck}};
    }
    // die eigene Farbe (F1 a, Stufe 20d); ohne sie die abgeleitete
    if (!schicht.farbe.isEmpty()) o[QStringLiteral("color")] = schicht.farbe;
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
    // seit Fassung 9; in einer aelteren Datei und an anderen Schichten fehlen sie
    neu.form = wurzel.value(QStringLiteral("shape")).toString();
    neu.formKontur = std::max(0, wurzel.value(QStringLiteral("shapeContour")).toInt(vorgabe.formKontur));
    neu.harmonische = zahl("shapeHarmonics", vorgabe.harmonische, 1.0, kFormHarmonischeMax);
    neu.harmonischeHuelle = zahl("shapeHarmonicsEnvelope", vorgabe.harmonischeHuelle, -kFormHuelleMax, kFormHuelleMax);
    neu.winkelGrad = zahl("shapeAngle", vorgabe.winkelGrad, 0.0, 360.0);
    neu.formStartProzent = zahl("shapeStart", vorgabe.formStartProzent, 0.0, 100.0);
    neu.stereoXY = wurzel.value(QStringLiteral("shapeStereoXY")).toBool(false);
    neu.rueckwaerts = wurzel.value(QStringLiteral("shapeReverse")).toBool(false);
    // seit Fassung 10 (Stufe 23c); fehlt `stringModel` oder ist es unbekannt: Classic
    const QString modell = wurzel.value(QStringLiteral("stringModel")).toString();
    neu.saitenModell = modell == QLatin1String("extendedKS")  ? SaitenModell::ErweiterterKs
                       : modell == QLatin1String("waveguide") ? SaitenModell::Wellenleiter
                                                              : SaitenModell::Klassisch;
    // Stufe 23d: die Tonabnehmer; fehlen sie, gelten die Vorgaben
    const auto abnehmer = [&](const char* schluessel, const Abnehmer& vorgabeAbnehmer) {
        const QJsonObject o = wurzel.value(QLatin1String(schluessel)).toObject();
        Abnehmer a;
        a.art = abnehmerArtAusName(o.value(QStringLiteral("type")).toString());
        a.mmVomSteg = inGrenzen(o.value(QStringLiteral("position")).toDouble(vorgabeAbnehmer.mmVomSteg), 0.0,
                                kMensurMaxMm, vorgabeAbnehmer.mmVomSteg);
        return a;
    };
    neu.abnehmerHals = abnehmer("pickupNeck", vorgabe.abnehmerHals);
    neu.abnehmerSteg = abnehmer("pickupBridge", vorgabe.abnehmerSteg);
    neu.kabelPf = zahl("cable", vorgabe.kabelPf, kKabelMinPf, kKabelMaxPf);
    // ein Korpus, den dieses Programm nicht kennt: keiner
    const QString korpus = wurzel.value(QStringLiteral("body")).toString();
    neu.korpus = korpus == QLatin1String("parlor")      ? KorpusArt::Parlor
                 : korpus == QLatin1String("classical") ? KorpusArt::Klassik
                                                        : KorpusArt::Keiner;
    neu.korpusAnteil = zahl("bodyAmount", vorgabe.korpusAnteil, 0.0, 1.0);
    const QString wahl = wurzel.value(QStringLiteral("pickupSelect")).toString();
    neu.abnehmerWahl = wahl == QLatin1String("neck")     ? AbnehmerWahl::Hals
                       : wahl == QLatin1String("bridge") ? AbnehmerWahl::Steg
                                                         : AbnehmerWahl::Beide;
    neu.anschlagOrt = zahl("pluckPosition", vorgabe.anschlagOrt, kAnschlagOrtMin, kAnschlagOrtMax);
    neu.anschlagHaerte = zahl("pluck", vorgabe.anschlagHaerte, 0.0, 1.0);
    neu.klingSekunden = zahl("ringTime", vorgabe.klingSekunden, kKlingSekundenMin, kKlingSekundenMax);
    neu.hoehenDaempfung = zahl("highDamping", vorgabe.hoehenDaempfung, 0.0, 1.0);
    neu.steifheit = zahl("stiffness", vorgabe.steifheit, 0.0, 1.0);
    neu.saitenUnterschied = zahl("stringVariation", vorgabe.saitenUnterschied, 0.0, 1.0);
    // seit Fassung 11 (Stufe 24a); fehlen sie, gelten die Vorgaben
    const QJsonObject spiel = wurzel.value(QStringLiteral("playing")).toObject();
    const auto spielZahl = [&spiel](const char* schluessel, double ersatz) {
        return inGrenzen(spiel.value(QLatin1String(schluessel)).toDouble(ersatz), 0.0, 1.0, ersatz);
    };
    neu.aufschlagLaut = spielZahl("upLevel", vorgabe.aufschlagLaut);
    neu.handDaempfung = spielZahl("mute", vorgabe.handDaempfung);
    neu.tappingLaut = spielZahl("tapLevel", vorgabe.tappingLaut);
    neu.knotenDruck = spielZahl("touch", vorgabe.knotenDruck);
    // die eigene Farbe (Stufe 20d); fehlt sie oder ist sie keine, gilt die abgeleitete
    if (const QColor farbe(wurzel.value(QStringLiteral("color")).toString()); farbe.isValid())
        neu.farbe = farbe.name(QColor::HexRgb);
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
    if (instrument.saiten.istSaiteninstrument())
    {
        // seit Fassung 10: die Saiten gehoeren dem Instrument (S-M1 a)
        QJsonObject saiten;
        QJsonArray leer;
        for (const int ton : instrument.saiten.leer) leer.append(ton);
        saiten[QStringLiteral("open")] = leer;
        saiten[QStringLiteral("capo")] = instrument.saiten.capo;
        saiten[QStringLiteral("stringChoice")] = saitenwahlName(instrument.saiten.wahl);
        if (instrument.saiten.mensurMm > 0.0) saiten[QStringLiteral("scale")] = instrument.saiten.mensurMm;  // Stufe 23d
        // seit Fassung 11 (Stufe 24a): die Vorgabe fuer neue Muster, nur wenn sie abweicht
        if (instrument.saiten.strum != Strum{}) saiten[QStringLiteral("strum")] = strumAlsJson(instrument.saiten.strum);
        o[QStringLiteral("strings")] = saiten;
    }
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
    // seit Fassung 10; fehlt der Block, ist es kein Saiteninstrument
    const QJsonObject saiten = wurzel.value(QStringLiteral("strings")).toObject();
    for (const QJsonValue& ton : saiten.value(QStringLiteral("open")).toArray())
        if (ton.isDouble()) neu.saiten.leer.push_back(ton.toInt());
    neu.saiten.capo = saiten.value(QStringLiteral("capo")).toInt(0);
    neu.saiten.wahl = saitenwahlAusName(saiten.value(QStringLiteral("stringChoice")).toString());
    neu.saiten.mensurMm = saiten.value(QStringLiteral("scale")).toDouble(0.0);
    neu.saiten.strum = strumAusJson(saiten.value(QStringLiteral("strum")).toObject());  // seit Fassung 11
    richteSaiten(neu.saiten);
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

QStringList eigeneInstrumenteMitForm(const QString& nennung)
{
    QStringList titel;
    if (nennung.isEmpty()) return titel;
    for (const Instrument& instrument : eigeneInstrumente())
    {
        const bool spieltEs = std::any_of(instrument.schichten.begin(), instrument.schichten.end(), [&nennung](const Schicht& s) {
            return s.schwingung == Schwingung::Form && s.form == nennung;
        });
        if (spieltEs) titel << instrument.titel;
    }
    return titel;
}

namespace
{

/// Vier Abstufungen je Familie (F1 a): die n-te Schicht derselben Art bekommt die n-te.
struct Familie
{
    Schwingung schwingung;
    std::array<const char*, 4> farben;
};
constexpr std::array<Familie, 7> kFamilien = {{
    {Schwingung::Sinus, {"#3b7be0", "#7aa9f0", "#2556a8", "#a9c8f7"}},
    {Schwingung::Dreieck, {"#2fa356", "#6ccb88", "#1d7039", "#a3e0b4"}},
    {Schwingung::Saege, {"#d93f3f", "#ef7a7a", "#9c2626", "#f6aeae"}},
    {Schwingung::Rechteck, {"#e5801f", "#f3a85f", "#a85a10", "#f8cb98"}},
    {Schwingung::Saite, {"#c9a227", "#e2c55e", "#8d7116", "#efdc9c"}},
    {Schwingung::Rauschen, {"#8c8c8c", "#b3b3b3", "#5f5f5f", "#d2d2d2"}},
    {Schwingung::Sample, {"#9160d9", "#b590f0", "#653ba3", "#d2bdf6"}},
}};

/// Die Farbe einer Kontur in einer Form — gelesen einmal je Nennung und Fassung der Zeichnungen.
QColor konturFarbeIn(const QString& nennung, int kontur)
{
    struct Gelesen
    {
        std::uint64_t fassung = 0;
        bool gefunden = false;
        shape::Zeichnung zeichnung;
    };
    static std::mutex sperre;
    static std::map<QString, Gelesen> gemerkt;
    const std::uint64_t fassung = shape::zeichnungFassung();
    shape::Zeichnung zeichnung;
    {
        const std::lock_guard<std::mutex> halt(sperre);
        auto it = gemerkt.find(nennung);
        if (it == gemerkt.end() || it->second.fassung != fassung)
        {
            Gelesen neu;
            neu.fassung = fassung;
            neu.gefunden = shape::holeZeichnung(nennung, neu.zeichnung);
            it = gemerkt.insert_or_assign(nennung, std::move(neu)).first;
        }
        if (!it->second.gefunden) return QColor(0x8c, 0x8c, 0x8c);  // nicht gefunden: grau wie eine stumme Schicht
        zeichnung = it->second.zeichnung;
    }
    for (const shape::Kontur& k : zeichnung.konturen)
        if (kontur == 0 ? k.gewaehlt : k.kennung == kontur) return QColor(k.farbe);
    return QColor(0x8c, 0x8c, 0x8c);
}

}  // namespace

QColor abgeleiteteSchichtFarbe(const Instrument& instrument, int platz)
{
    if (platz < 0 || platz >= static_cast<int>(instrument.schichten.size())) return {};
    const Schicht& schicht = instrument.schichten[static_cast<std::size_t>(platz)];
    if (schicht.schwingung == Schwingung::Form) return konturFarbeIn(schicht.form, schicht.formKontur);
    int davor = 0;
    for (int i = 0; i < platz; ++i) davor += instrument.schichten[static_cast<std::size_t>(i)].schwingung == schicht.schwingung ? 1 : 0;
    for (const Familie& f : kFamilien)
        if (f.schwingung == schicht.schwingung) return QColor(QLatin1String(f.farben[static_cast<std::size_t>(davor) % f.farben.size()]));
    return QColor(0x8c, 0x8c, 0x8c);
}

QColor schichtFarbe(const Instrument& instrument, int platz)
{
    if (platz < 0 || platz >= static_cast<int>(instrument.schichten.size())) return {};
    const QString& eigene = instrument.schichten[static_cast<std::size_t>(platz)].farbe;
    if (const QColor farbe(eigene); !eigene.isEmpty() && farbe.isValid()) return farbe;
    return abgeleiteteSchichtFarbe(instrument, platz);
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

namespace
{

/// Wie dick eine Saite gegen die duennste des Instruments ist, mal String
/// variation der Schicht (Stufe 23c): 0 bei der hoechsten leeren Saite, 1 bei
/// der tiefsten; ohne Saite oder mit nur einer 0.
double saitenDicke(const Saitensatz& saiten, int saite)
{
    if (saite < 0 || saite >= saiten.anzahl() || saiten.anzahl() < 2) return 0.0;
    const auto [tief, hoch] = std::minmax_element(saiten.leer.begin(), saiten.leer.end());
    if (*hoch == *tief) return 0.0;
    return static_cast<double>(*hoch - saiten.leer[static_cast<std::size_t>(saite)]) / static_cast<double>(*hoch - *tief);
}

/// Einen Ton rechnen; `saite` ist die Saite des Tons (−1 = keine), sie gibt
/// einer Saite *Extended KS* ihre Dicke (String variation).
Klang rendereTonAufSaite(const Instrument& instrument, int hoehe, double dauerMs, int rate, int saite)
{
    Klang klang;
    if (rate <= 0 || dauerMs <= 0.0) return klang;
    const double dicke = saitenDicke(instrument.saiten, saite);
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
        if (schicht.schwingung == Schwingung::Saite && schicht.saitenModell == SaitenModell::ErweiterterKs)
            rechneSaiteKs(schicht, ton, hz, huelle, rate, std::clamp(schicht.saitenUnterschied, 0.0, 1.0) * dicke, eine);
        else if (schicht.schwingung == Schwingung::Saite && schicht.saitenModell == SaitenModell::Wellenleiter)
            rechneSaiteWg(schicht, hz, huelle, rate, std::clamp(schicht.saitenUnterschied, 0.0, 1.0) * dicke,
                          instrument.saiten.mensurMm, eine);
        else if (schicht.schwingung == Schwingung::Saite)
            rechneSaite(schicht, ton, hz, huelle, rate, eine);  // Classic, unberuehrt (S-K2 b)
        else if (schicht.schwingung == Schwingung::Sample)
            rechneSample(schicht, ton, hz, huelle, rate, eine);
        else if (schicht.schwingung == Schwingung::Rauschen)
            rechneRauschen(schicht, hz, huelle, rate, eine);
        else if (schicht.schwingung == Schwingung::Form)
            rechneForm(schicht, hz, huelle, rate, eine);
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

}  // namespace

Klang rendereTon(const Instrument& instrument, int hoehe, double dauerMs, int rate)
{
    return rendereTonAufSaite(instrument, hoehe, dauerMs, rate, -1);
}

std::vector<double> korpusImpulsantwort(KorpusArt art, int rate, int frames)
{
    Korpus korpus(art, rate);
    if (!korpus.an() || frames <= 0) return {};
    std::vector<double> antwort(static_cast<std::size_t>(frames), 0.0);
    double groesster = 0.0;
    for (std::size_t n = 0; n < antwort.size(); ++n)
    {
        antwort[n] = korpus.schritt(n == 0 ? 1.0 : 0.0);
        groesster = std::max(groesster, std::abs(antwort[n]));
    }
    if (groesster > 0.0)
        for (double& wert : antwort) wert /= groesster;
    return antwort;
}

Resonanz abnehmerResonanz(AbnehmerArt art, double kabelPf)
{
    // je Bauart: Bezugsfrequenz, Bezugskapazitaet, Guete (Belege in Instrument.hpp)
    double bezugHz = 0.0;
    double bezugPf = 0.0;
    double guete = 0.0;
    if (art == AbnehmerArt::SingleCoil)
    {
        bezugHz = 3500.0;
        bezugPf = 600.0;
        guete = std::pow(10.0, 4.0 / 20.0);  // +4 dB Ueberhoehung, 20 lg Q
    }
    else if (art == AbnehmerArt::Humbucker)
    {
        bezugHz = 3000.0;
        bezugPf = 450.0;
        guete = 1.83;
    }
    else
        return {};
    const double kabel = std::isfinite(kabelPf) ? std::clamp(kabelPf, kKabelMinPf, kKabelMaxPf) : kKabelVorgabePf;
    return {bezugHz * std::sqrt(bezugPf / kabel), guete};
}

Klang rendereTon(const Instrument& instrument, const TonAngabe& ton, int rate)
{
    Klang klang = rendereTonAufSaite(instrument, ton.hoehe, ton.dauerMs, rate, ton.saite);
    if (ton.endeMs <= 0 || rate <= 0) return klang;
    // abdaempfen (S-M3 a): bis zum Ende still, die letzten Millisekunden ausgeblendet
    const std::int64_t ende = std::max<std::int64_t>(1, std::llround(ton.endeMs * static_cast<double>(rate) / 1000.0));
    if (klang.frames() <= ende) return klang;  // er endet von selbst vorher
    const std::int64_t blende = std::min<std::int64_t>(ende, std::max<std::int64_t>(1, std::llround(kAbdaempfenMs * rate / 1000.0)));
    const std::int64_t ab = ende - blende;
    for (std::int64_t f = ab; f < ende; ++f)
    {
        const auto faktor = static_cast<float>(static_cast<double>(ende - f) / static_cast<double>(blende + 1));
        klang.daten[static_cast<std::size_t>(2 * f)] *= faktor;
        klang.daten[static_cast<std::size_t>(2 * f + 1)] *= faktor;
    }
    klang.daten.resize(static_cast<std::size_t>(2 * ende));
    return klang;
}

SaitenVon saitenAusInstrumenten()
{
    auto gemerkt = std::make_shared<std::map<QString, Saitensatz>>();
    return [gemerkt](const QString& nennung) {
        auto ort = gemerkt->find(nennung);
        if (ort == gemerkt->end())
        {
            Instrument instrument;
            Saitensatz saiten;
            if (holeInstrument(nennung, instrument)) saiten = instrument.saiten;
            ort = gemerkt->emplace(nennung, std::move(saiten)).first;
        }
        return ort->second;
    };
}

}  // namespace lumi::loop

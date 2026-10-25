/**
 ****************************************************************************************
 * @file   Teiltonmessung.hpp
 * @brief  Teiltoene eines Tons ueber der Zeit messen: Frequenzen und B, zwei Abklingstufen,
 *         Schwebung, Helligkeit zu Beginn, Abklingen ueber der Frequenz (Stufe 31d-2;
 *         docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S151: UF3 a, IK1)
 *
 * @details
 * Ein Code fuer *Salamander* und fuer die gerechneten Toene (UF3 a). Reines C++ ohne Qt,
 * nur gegen die Schnittstelle `lumi::loop::Fft` (IK1). Jede Ausgleichsrechnung ist linear
 * (Kipppunkt von UF3 a: nichtlinear mit mehreren Parametern).
 *
 * Ablauf von `miss`:
 * 1. *Einsatz:* die erste Probe ueber 1 % der Spitze.
 * 2. *Frequenzen:* ein langes Spektrum ab 50 ms nach dem Einsatz (Blackman-Harris, viermal
 *    aufgefuellt, Spitze parabolisch auf dB); Teilton k wird um k·f0·√(1 + B·k²) gesucht, B
 *    und f0 nach jedem gefundenen Teilton neu ausgeglichen: (f_k / k)² = f0² + f0²·B·k²
 *    (Gerade in k², kleinste Quadrate). Fuenf Teiltoene in Folge ohne Spitze beenden die Suche.
 * 3. *Huellkurven:* je Teilton der Pegel ueber der Zeit aus einer Kurzzeit-FFT (Fenster
 *    `fensterPerioden` Perioden von f0 — der Nachbar liegt am Rand der Hauptkeule; Schritt
 *    `schrittS`), dazu je Rahmen der Boden in der Mitte zu beiden Nachbarn (Rauschen und das
 *    Leck der anderen Teiltoene — langsame tiefe Teiltoene legen einen fallenden Boden unter
 *    schnelle hohe).
 * 4. *Zwei Stufen:* auf der Huellkurve in dB, ab dem Ende des Anschlags bis zuletzt `grenzeDb`
 *    ueber dem Boden — dem aus 3. oder, wenn der Teilton vor dem Ende darin ankommt, dem eigenen
 *    (Median der letzten 10 %, Gedanke nach Lundeby fuer Nachhall im Rauschen) —, eine Gerade
 *    und zwei Geraden mit dem besten Knick (jede mindestens ein Fuenftel des Bereichs); zwei
 *    Stufen, wenn sie
 *    den Rest mindestens halbieren und die fruehe um das 1,3-fache steiler faellt. T60 = 60 dB
 *    durch die Steigung (wie `decay` und `lateDecay`).
 * 5. *Schwebung:* die staerkste Periodizitaet im Rest um die Geraden, mindestens zwei Perioden
 *    im Bereich, hoechstens die halbe Fensterrate; erkannt ab 1 dB Tiefe (Spitze zu Spitze) und
 *    dem Zehnfachen des Medians im Periodogramm.
 * 6. *Abklingen ueber der Frequenz:* σ = b1 + b3·ω² aus den fruehen Stufen (Bank 2010, Gl. 7);
 *    `highDecay` = σ(f1) / σ(10·f1) wie im Modell (`modell.md` §3).
 ****************************************************************************************
 */

#pragma once

#include "loop/Fft.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <span>
#include <utility>
#include <vector>

namespace lumi::messung
{

struct Einstellungen
{
    int hoechsterTeilton = 60;
    /// Schritt der Huellkurven.
    double schrittS = 0.01;
    /// Laenge des Fensters der Huellkurven in Perioden von f0 (zwischen 20 ms und 0,5 s).
    double fensterPerioden = 8.0;
    /// Die Huellkurve zaehlt bis so weit ueber dem Rauschen.
    double grenzeDb = 10.0;
    /// Die Helligkeit: die Spitze jeder Huellkurve in so viel Zeit nach dem Einsatz.
    double anfangBisS = 0.1;
    /// Eine Spitze im langen Spektrum zaehlt so weit ueber dessen Median.
    double spitzeUeberMedianDb = 20.0;
    /// Die Schwebung wird bis hoechstens hier gesucht.
    double schwebungBisHz = 8.0;
};

struct Abklingen
{
    bool erkannt = false;
    bool zweiStufen = false;
    double t60Frueh = 0.0;  ///< s; ohne zweite Stufe gleich `t60Spaet`
    double t60Spaet = 0.0;  ///< s
    double knickS = 0.0;    ///< Zeit des Knicks nach dem Einsatz
    double vonS = 0.0;      ///< gemessener Bereich nach dem Einsatz
    double bisS = 0.0;
};

struct Schwebung
{
    bool erkannt = false;
    double frequenz = 0.0;  ///< Hz
    double tiefeDb = 0.0;   ///< Spitze zu Spitze der gefundenen Periodizitaet
};

struct Teilton
{
    int k = 0;
    double frequenz = 0.0;  ///< Hz, aus dem langen Spektrum
    std::vector<double> pegelDb;  ///< Huellkurve, Amplitude in dB, ab dem Einsatz im Schritt `schrittS`
    double rauschenDb = 0.0;
    double anfangDb = 0.0;  ///< Spitze in `anfangBisS` nach dem Einsatz
    Abklingen abklingen;
    Schwebung schwebung;
};

struct Ergebnis
{
    bool erkannt = false;
    int rate = 0;
    double einsatzS = 0.0;
    double f0 = 0.0;  ///< aus dem Ausgleich; der erste Teilton liegt bei f0·√(1 + B)
    double b = 0.0;
    double bFehler = 0.0;  ///< Standardfehler von B (genaehert: nur die Steigung)
    double schrittS = 0.0;
    std::vector<Teilton> teiltoene;
    /// σ = b1 + b3·ω² ueber die fruehen Stufen; `highDecay` wie im Modell.
    bool hochErkannt = false;
    double sigmaB1 = 0.0;
    double sigmaB3 = 0.0;
    double highDecay = 0.0;

    [[nodiscard]] const Teilton* teilton(int k) const
    {
        for (const Teilton& t : teiltoene)
            if (t.k == k) return &t;
        return nullptr;
    }
};

namespace detail
{

inline std::vector<double> blackmanHarris(std::size_t n)
{
    std::vector<double> w(n, 1.0);
    if (n < 2) return w;
    const double d = 2.0 * std::numbers::pi / static_cast<double>(n - 1);
    for (std::size_t i = 0; i < n; ++i)
    {
        const double x = d * static_cast<double>(i);
        w[i] = 0.35875 - 0.48829 * std::cos(x) + 0.14128 * std::cos(2.0 * x) - 0.01168 * std::cos(3.0 * x);
    }
    return w;
}

inline double db(double amplitude)
{
    return 20.0 * std::log10(std::max(amplitude, 1e-15));
}

/// Ein gefenstertes Stueck ab `von` (ausserhalb des Tons null), aufgefuellt auf `n`, als Pegel in dB
/// je Bin; 0 dB = ein Sinus der Amplitude 1.
class Spektrum
{
public:
    Spektrum(const lumi::loop::Fft& fft, std::size_t fensterLaenge, std::size_t n)
        : m_fft(fft), m_fenster(blackmanHarris(fensterLaenge)), m_zeit(n), m_frei(n / 2 + 1), m_db(n / 2 + 1)
    {
        double summe = 0.0;
        for (const double w : m_fenster) summe += w;
        m_normierung = 2.0 / summe;
    }

    void rechne(std::span<const double> ton, std::ptrdiff_t von)
    {
        std::fill(m_zeit.begin(), m_zeit.end(), 0.0);
        for (std::size_t i = 0; i < m_fenster.size(); ++i)
        {
            const std::ptrdiff_t stelle = von + static_cast<std::ptrdiff_t>(i);
            if (stelle >= 0 && stelle < static_cast<std::ptrdiff_t>(ton.size()))
                m_zeit[i] = ton[static_cast<std::size_t>(stelle)] * m_fenster[i];
        }
        m_fft.vorwaerts(m_zeit, m_frei);
        for (std::size_t i = 0; i < m_frei.size(); ++i) m_db[i] = db(std::abs(m_frei[i]) * m_normierung);
    }

    [[nodiscard]] std::size_t bins() const { return m_db.size(); }
    [[nodiscard]] double binDb(std::size_t i) const { return m_db[std::min(i, m_db.size() - 1)]; }

    /// Die groesste Spitze zwischen den Bins `von` und `bis` (beide eingeschlossen), parabolisch
    /// auf dB: Stelle in Bins und Pegel.
    [[nodiscard]] std::pair<double, double> spitze(std::size_t von, std::size_t bis) const
    {
        von = std::max<std::size_t>(von, 1);
        bis = std::min(bis, m_db.size() - 2);
        if (von > bis) return {0.0, -300.0};
        std::size_t beste = von;
        for (std::size_t i = von; i <= bis; ++i)
            if (m_db[i] > m_db[beste]) beste = i;
        const double a = m_db[beste - 1];
        const double b = m_db[beste];
        const double c = m_db[beste + 1];
        const double nenner = a - 2.0 * b + c;
        const double delta = nenner < 0.0 ? std::clamp(0.5 * (a - c) / nenner, -0.5, 0.5) : 0.0;
        return {static_cast<double>(beste) + delta, b - 0.25 * (a - c) * delta};
    }

private:
    const lumi::loop::Fft& m_fft;
    std::vector<double> m_fenster;
    std::vector<double> m_zeit;
    std::vector<std::complex<double>> m_frei;
    std::vector<double> m_db;
    double m_normierung = 1.0;
};

/// Kleinste Quadrate einer Geraden y = a + s·x ueber Praefixsummen: jeder Bereich in O(1).
class Geraden
{
public:
    Geraden(std::span<const double> x, std::span<const double> y)
        : m_sx(x.size() + 1, 0.0), m_sy(m_sx), m_sxx(m_sx), m_sxy(m_sx), m_syy(m_sx)
    {
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            m_sx[i + 1] = m_sx[i] + x[i];
            m_sy[i + 1] = m_sy[i] + y[i];
            m_sxx[i + 1] = m_sxx[i] + x[i] * x[i];
            m_sxy[i + 1] = m_sxy[i] + x[i] * y[i];
            m_syy[i + 1] = m_syy[i] + y[i] * y[i];
        }
    }

    struct Gerade
    {
        double achse = 0.0;
        double steigung = 0.0;
        double rest = 0.0;  ///< Summe der Quadrate der Abweichungen
    };

    /// Die Gerade ueber [von, bis).
    [[nodiscard]] Gerade ueber(std::size_t von, std::size_t bis) const
    {
        const double n = static_cast<double>(bis - von);
        const double sx = m_sx[bis] - m_sx[von];
        const double sy = m_sy[bis] - m_sy[von];
        const double sxx = m_sxx[bis] - m_sxx[von] - sx * sx / n;
        const double sxy = m_sxy[bis] - m_sxy[von] - sx * sy / n;
        const double syy = m_syy[bis] - m_syy[von] - sy * sy / n;
        Gerade g;
        g.steigung = sxx > 0.0 ? sxy / sxx : 0.0;
        g.achse = (sy - g.steigung * sx) / n;
        g.rest = std::max(0.0, syy - g.steigung * sxy);
        return g;
    }

private:
    std::vector<double> m_sx, m_sy, m_sxx, m_sxy, m_syy;
};

/// f0 und B aus (f_k / k)² = f0² + f0²·B·k²; `bFehler` aus dem Standardfehler der Steigung.
struct Inharmonie
{
    bool erkannt = false;
    double f0 = 0.0;
    double b = 0.0;
    double bFehler = 0.0;
};

inline Inharmonie gleicheAus(const std::vector<int>& k, const std::vector<double>& f)
{
    Inharmonie e;
    if (k.size() < 2) return e;
    std::vector<double> x(k.size());
    std::vector<double> y(k.size());
    for (std::size_t i = 0; i < k.size(); ++i)
    {
        x[i] = static_cast<double>(k[i]) * k[i];
        y[i] = (f[i] / k[i]) * (f[i] / k[i]);
    }
    const Geraden geraden(x, y);
    const Geraden::Gerade g = geraden.ueber(0, x.size());
    if (g.achse <= 0.0) return e;
    e.erkannt = true;
    e.f0 = std::sqrt(g.achse);
    e.b = g.steigung / g.achse;
    if (k.size() > 2)
    {
        double mx = 0.0;
        for (const double v : x) mx += v;
        mx /= static_cast<double>(x.size());
        double sxx = 0.0;
        for (const double v : x) sxx += (v - mx) * (v - mx);
        const double s2 = g.rest / static_cast<double>(k.size() - 2);
        e.bFehler = sxx > 0.0 ? std::sqrt(s2 / sxx) / g.achse : 0.0;
    }
    return e;
}

inline Abklingen zweiStufen(std::span<const double> pegelDb, double schrittS, std::size_t von, std::size_t bis)
{
    Abklingen a;
    if (bis <= von || bis - von < 10) return a;
    std::vector<double> t(pegelDb.size());
    for (std::size_t i = 0; i < t.size(); ++i) t[i] = static_cast<double>(i) * schrittS;
    const Geraden geraden(t, pegelDb);
    const Geraden::Gerade eine = geraden.ueber(von, bis);
    if (eine.steigung >= 0.0) return a;
    a.erkannt = true;
    a.vonS = t[von];
    a.bisS = t[bis - 1];
    a.t60Frueh = a.t60Spaet = -60.0 / eine.steigung;

    // jede Stufe mindestens ein Fuenftel des Bereichs: sonst „erkennt“ der Uebergang in den Boden
    // eine kurze zweite Stufe
    const std::size_t rand = std::max<std::size_t>(10, (bis - von) / 5);
    if (2 * rand > bis - von) return a;
    double besterRest = eine.rest;
    std::size_t knick = 0;
    Geraden::Gerade frueh;
    Geraden::Gerade spaet;
    for (std::size_t j = von + rand; j + rand <= bis; ++j)
    {
        const Geraden::Gerade g1 = geraden.ueber(von, j);
        const Geraden::Gerade g2 = geraden.ueber(j, bis);
        if (g1.rest + g2.rest < besterRest)
        {
            besterRest = g1.rest + g2.rest;
            knick = j;
            frueh = g1;
            spaet = g2;
        }
    }
    if (knick != 0 && besterRest < 0.5 * eine.rest && frueh.steigung < 0.0 && spaet.steigung < 0.0 &&
        frueh.steigung < 1.3 * spaet.steigung)
    {
        a.zweiStufen = true;
        a.knickS = t[knick];
        a.t60Frueh = -60.0 / frueh.steigung;
        a.t60Spaet = -60.0 / spaet.steigung;
    }
    return a;
}

/// Der Wert der gemessenen Stufen zur Zeit t (die Geraden aus `zweiStufen`, an `pegelDb` angelegt).
inline std::vector<double> restUmStufen(std::span<const double> pegelDb, double schrittS, std::size_t von, std::size_t bis,
                                        const Abklingen& a)
{
    std::vector<double> t(pegelDb.size());
    for (std::size_t i = 0; i < t.size(); ++i) t[i] = static_cast<double>(i) * schrittS;
    const Geraden geraden(t, pegelDb);
    std::vector<double> rest;
    rest.reserve(bis - von);
    if (a.zweiStufen)
    {
        const auto knick = static_cast<std::size_t>(std::lround(a.knickS / schrittS));
        const Geraden::Gerade g1 = geraden.ueber(von, knick);
        const Geraden::Gerade g2 = geraden.ueber(knick, bis);
        for (std::size_t i = von; i < bis; ++i)
        {
            const Geraden::Gerade& g = i < knick ? g1 : g2;
            rest.push_back(pegelDb[i] - (g.achse + g.steigung * t[i]));
        }
    }
    else
    {
        const Geraden::Gerade g = geraden.ueber(von, bis);
        for (std::size_t i = von; i < bis; ++i) rest.push_back(pegelDb[i] - (g.achse + g.steigung * t[i]));
    }
    return rest;
}

inline Schwebung findeSchwebung(std::span<const double> rest, double schrittS, double bisHz)
{
    Schwebung s;
    const double dauer = static_cast<double>(rest.size()) * schrittS;
    const double von = 2.0 / dauer;
    const double bis = std::min(bisHz, 0.25 / schrittS);
    if (rest.size() < 20 || von >= bis) return s;
    const double schritt = 0.1 / dauer;
    std::vector<double> leistung;
    std::vector<double> amplitude;
    for (double f = von; f <= bis; f += schritt)
    {
        std::complex<double> summe{};
        const std::complex<double> dreh = std::polar(1.0, -2.0 * std::numbers::pi * f * schrittS);
        std::complex<double> zeiger{1.0, 0.0};
        for (const double r : rest)
        {
            summe += r * zeiger;
            zeiger *= dreh;
        }
        leistung.push_back(std::norm(summe));
        amplitude.push_back(2.0 * std::abs(summe) / static_cast<double>(rest.size()));
    }
    const auto beste = static_cast<std::size_t>(std::max_element(leistung.begin(), leistung.end()) - leistung.begin());
    std::vector<double> sortiert = leistung;
    std::nth_element(sortiert.begin(), sortiert.begin() + static_cast<std::ptrdiff_t>(sortiert.size() / 2), sortiert.end());
    const double median = sortiert[sortiert.size() / 2];
    double delta = 0.0;
    if (beste > 0 && beste + 1 < leistung.size())
    {
        const double a = leistung[beste - 1];
        const double b = leistung[beste];
        const double c = leistung[beste + 1];
        const double nenner = a - 2.0 * b + c;
        if (nenner < 0.0) delta = std::clamp(0.5 * (a - c) / nenner, -0.5, 0.5);
    }
    s.frequenz = von + (static_cast<double>(beste) + delta) * schritt;
    s.tiefeDb = 2.0 * amplitude[beste];
    s.erkannt = s.tiefeDb >= 1.0 && leistung[beste] >= 10.0 * median;
    return s;
}

inline double median(std::vector<double> werte)
{
    if (werte.empty()) return 0.0;
    const auto mitte = werte.begin() + static_cast<std::ptrdiff_t>(werte.size() / 2);
    std::nth_element(werte.begin(), mitte, werte.end());
    return *mitte;
}

}  // namespace detail

/// Einen Ton messen. `nennFrequenz` ist die Tonhoehe der Taste (der erste Teilton liegt nahe).
inline Ergebnis miss(std::span<const double> ton, int rate, double nennFrequenz, const lumi::loop::Fft& fft,
                     const Einstellungen& einst = {})
{
    Ergebnis e;
    e.rate = rate;
    e.schrittS = einst.schrittS;
    if (ton.empty() || rate <= 0 || nennFrequenz <= 0.0) return e;

    // 1. Einsatz
    double spitze = 0.0;
    for (const double x : ton) spitze = std::max(spitze, std::abs(x));
    if (spitze <= 0.0) return e;
    std::size_t einsatz = 0;
    while (einsatz < ton.size() && std::abs(ton[einsatz]) <= 0.01 * spitze) ++einsatz;
    e.einsatzS = static_cast<double>(einsatz) / rate;

    // 2. Frequenzen und B aus dem langen Spektrum
    const auto langVon = static_cast<std::ptrdiff_t>(einsatz + static_cast<std::size_t>(0.05 * rate));
    const std::size_t verfuegbar = ton.size() > static_cast<std::size_t>(langVon) ? ton.size() - static_cast<std::size_t>(langVon) : 0;
    const auto langWunsch = static_cast<std::size_t>(std::max(1.0, 40.0 / nennFrequenz) * rate);
    const std::size_t langLaenge = std::min(verfuegbar, langWunsch);
    if (langLaenge < 1024) return e;
    const std::size_t langN = 4 * lumi::loop::zweierpotenzAb(langLaenge);
    detail::Spektrum lang(fft, langLaenge, langN);
    lang.rechne(ton, langVon);
    std::vector<double> alle(lang.bins());
    for (std::size_t i = 0; i < alle.size(); ++i) alle[i] = lang.binDb(i);
    const double grundDb = detail::median(alle);
    const double binHz = static_cast<double>(rate) / static_cast<double>(langN);

    std::vector<int> ks;
    std::vector<double> fs;
    detail::Inharmonie ih{true, nennFrequenz, 0.0, 0.0};
    int fehlend = 0;
    for (int k = 1; k <= einst.hoechsterTeilton && fehlend < 5; ++k)
    {
        const double erwartet = k * ih.f0 * std::sqrt(1.0 + ih.b * k * k);
        if (erwartet + 0.25 * ih.f0 >= 0.45 * rate) break;
        const auto von = static_cast<std::size_t>(std::max(0.0, (erwartet - 0.25 * ih.f0) / binHz));
        const auto bis = static_cast<std::size_t>((erwartet + 0.25 * ih.f0) / binHz);
        const auto [stelle, pegel] = lang.spitze(von, bis);
        if (pegel < grundDb + einst.spitzeUeberMedianDb)
        {
            ++fehlend;
            continue;
        }
        fehlend = 0;
        ks.push_back(k);
        fs.push_back(stelle * binHz);
        if (ks.size() >= 2)
        {
            const detail::Inharmonie neu = detail::gleicheAus(ks, fs);
            if (neu.erkannt && neu.b > -1e-3) ih = neu;
        }
        else
            ih.f0 = fs.back() / k;
    }
    if (ks.empty()) return e;
    const detail::Inharmonie ende = detail::gleicheAus(ks, fs);
    e.f0 = ende.erkannt ? ende.f0 : fs.front() / ks.front();
    e.b = ende.erkannt ? ende.b : 0.0;
    e.bFehler = ende.bFehler;
    e.erkannt = true;

    // 3. Huellkurven
    const double fensterS = std::clamp(einst.fensterPerioden / e.f0, 0.02, 0.5);
    const auto fensterL = static_cast<std::size_t>(fensterS * rate);
    const std::size_t kurzN = lumi::loop::zweierpotenzAb(fensterL);
    const double kurzBinHz = static_cast<double>(rate) / static_cast<double>(kurzN);
    const auto schritt = static_cast<std::size_t>(std::max(1.0, einst.schrittS * rate));
    const std::size_t rahmen = (ton.size() - einsatz) / schritt;
    detail::Spektrum kurz(fft, fensterL, kurzN);
    e.teiltoene.resize(ks.size());
    for (std::size_t i = 0; i < ks.size(); ++i)
    {
        e.teiltoene[i].k = ks[i];
        e.teiltoene[i].frequenz = fs[i];
        e.teiltoene[i].pegelDb.reserve(rahmen);
    }
    std::vector<std::vector<double>> rauschen(ks.size());
    for (std::size_t r = 0; r < rahmen; ++r)
    {
        const auto mitte = static_cast<std::ptrdiff_t>(einsatz + r * schritt);
        kurz.rechne(ton, mitte - static_cast<std::ptrdiff_t>(fensterL / 2));
        for (std::size_t i = 0; i < ks.size(); ++i)
        {
            const double bin = fs[i] / kurzBinHz;
            const auto [stelle, pegel] = kurz.spitze(static_cast<std::size_t>(std::max(0.0, bin - 1.0)),
                                                     static_cast<std::size_t>(bin + 1.0));
            (void)stelle;
            e.teiltoene[i].pegelDb.push_back(pegel);
            // der Boden auf beiden Seiten: Rauschen und das Leck der anderen Teiltoene
            const double unten = i > 0 && ks[i - 1] == ks[i] - 1 ? fs[i - 1] : fs[i] - e.f0;
            const double oben = i + 1 < ks.size() && ks[i + 1] == ks[i] + 1 ? fs[i + 1] : fs[i] + e.f0;
            double boden = -300.0;
            for (const double mitteHz : {0.5 * (unten + fs[i]), 0.5 * (fs[i] + oben)})
            {
                const auto b = static_cast<std::ptrdiff_t>(std::lround(mitteHz / kurzBinHz));
                for (std::ptrdiff_t d = -1; d <= 1; ++d)
                    if (b + d > 0) boden = std::max(boden, kurz.binDb(static_cast<std::size_t>(b + d)));
            }
            rauschen[i].push_back(boden);
        }
    }

    // 4. bis 5. je Teilton
    const auto anschlag = static_cast<std::size_t>(std::ceil(0.5 * fensterS / einst.schrittS));
    const auto anfangRahmen = static_cast<std::size_t>(einst.anfangBisS / einst.schrittS);
    for (std::size_t i = 0; i < e.teiltoene.size(); ++i)
    {
        Teilton& t = e.teiltoene[i];
        if (t.pegelDb.empty()) continue;
        const std::vector<double>& r = rauschen[i];
        t.rauschenDb = detail::median(std::vector<double>(r.begin() + static_cast<std::ptrdiff_t>(2 * r.size() / 3), r.end()));
        const std::size_t bisAnfang = std::min(t.pegelDb.size(), anfangRahmen + 1);
        t.anfangDb = *std::max_element(t.pegelDb.begin(), t.pegelDb.begin() + static_cast<std::ptrdiff_t>(bisAnfang));

        const std::size_t suche = std::min(t.pegelDb.size(), static_cast<std::size_t>(0.5 / einst.schrittS) + 1);
        const auto hoechster = static_cast<std::size_t>(std::max_element(t.pegelDb.begin(), t.pegelDb.begin() + static_cast<std::ptrdiff_t>(suche)) -
                                                        t.pegelDb.begin());
        const std::size_t von = hoechster + anschlag;
        // der Bereich endet, wo der Teilton zuletzt `grenzeDb` ueber dem Boden lag — der Boden
        // faellt mit, wenn er Leck der anderen ist; beide ueber ±5 Rahmen gemittelt, damit eine
        // einzelne Spitze im Rauschen den Bereich nicht offen haelt
        const auto glatt = [](const std::vector<double>& v, std::size_t j) {
            const std::size_t a = j >= 5 ? j - 5 : 0;
            const std::size_t b = std::min(v.size(), j + 6);
            double s = 0.0;
            for (std::size_t m = a; m < b; ++m) s += v[m];
            return s / static_cast<double>(b - a);
        };
        std::size_t bis = t.pegelDb.size();
        while (bis > von && glatt(t.pegelDb, bis - 1) <= glatt(r, bis - 1) + einst.grenzeDb) --bis;
        // Die Mitte zwischen den Teiltoenen liegt in den Nullstellen des Fensters zu den Nachbarn
        // und unterschaetzt den Boden am Teilton. Darum auch der eigene Boden (nach Lundeby): der
        // Median der letzten 10 % ist es, wenn eine Gerade bis `grenzeDb` darueber am Ende der
        // Aufnahme deutlich unter ihm ankaeme — sonst faellt der Teilton dort noch.
        if (bis > von)
        {
            const std::size_t zehntel = std::max<std::size_t>(1, t.pegelDb.size() / 10);
            const double eigen = detail::median(std::vector<double>(t.pegelDb.end() - static_cast<std::ptrdiff_t>(zehntel), t.pegelDb.end()));
            std::size_t vorEigen = bis;
            while (vorEigen > von && glatt(t.pegelDb, vorEigen - 1) <= eigen + einst.grenzeDb) --vorEigen;
            if (vorEigen > von + 10)
            {
                std::vector<double> zeiten(t.pegelDb.size());
                for (std::size_t j = 0; j < zeiten.size(); ++j) zeiten[j] = static_cast<double>(j) * einst.schrittS;
                const detail::Geraden::Gerade g = detail::Geraden(zeiten, t.pegelDb).ueber(von, vorEigen);
                if (g.achse + g.steigung * zeiten.back() < eigen - 3.0)
                {
                    bis = vorEigen;
                    t.rauschenDb = std::max(t.rauschenDb, eigen);
                }
            }
        }
        if (bis <= von || static_cast<double>(bis - von) * einst.schrittS < 0.3) continue;
        t.abklingen = detail::zweiStufen(t.pegelDb, einst.schrittS, von, bis);
        if (!t.abklingen.erkannt) continue;
        const std::vector<double> rest = detail::restUmStufen(t.pegelDb, einst.schrittS, von, bis, t.abklingen);
        t.schwebung = detail::findeSchwebung(rest, einst.schrittS, std::min(einst.schwebungBisHz, 0.5 / fensterS));
    }

    // 6. Abklingen ueber der Frequenz
    std::vector<double> omega2;
    std::vector<double> sigma;
    for (const Teilton& t : e.teiltoene)
    {
        if (!t.abklingen.erkannt) continue;
        const double w = 2.0 * std::numbers::pi * t.frequenz;
        omega2.push_back(w * w);
        sigma.push_back(std::log(1000.0) / t.abklingen.t60Frueh);
    }
    if (omega2.size() >= 3)
    {
        const detail::Geraden geraden(omega2, sigma);
        const detail::Geraden::Gerade g = geraden.ueber(0, omega2.size());
        e.sigmaB1 = g.achse;
        e.sigmaB3 = g.steigung;
        const double w1 = 2.0 * std::numbers::pi * e.f0 * std::sqrt(1.0 + e.b);
        const double s1 = g.achse + g.steigung * w1 * w1;
        const double s10 = g.achse + g.steigung * 100.0 * w1 * w1;
        if (s1 > 0.0 && s10 > 0.0)
        {
            e.hochErkannt = true;
            e.highDecay = s1 / s10;
        }
    }
    return e;
}

}  // namespace lumi::messung

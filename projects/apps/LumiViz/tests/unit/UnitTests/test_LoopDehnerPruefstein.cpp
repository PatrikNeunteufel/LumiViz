/**
 ****************************************************************************************
 * @file   test_LoopDehnerPruefstein.cpp
 * @brief  Loop, Stufe 15h: TD-PSOLA und der Pruefstein der Dehner an gerechneten Stimmen
 *         (docs/audio/Loop_Konzept.md §4p, Entscheid Patrik WV6 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. **Gerechnete Stimmen** mit bekannten Formanten: ein
 * Rosenberg-Puls (Rosenberg 1971, Oeffnen 40 %, Schliessen 16 % der Periode), abgeleitet (die
 * Abstrahlung am Mund), durch drei Resonatoren in Reihe (F1–F3 nach Peterson & Barney 1952,
 * Mittelwerte Maenner und Frauen; Bandbreite 50 Hz + 6 % der Frequenz — von Claude gesetzt).
 *
 * Gemessen je Stimme, Tonhoehe und Umsetzung — **ohne pYIN**, damit der Pruefstein nicht misst,
 * was er an PSOLA prueft:
 * - **Fehler der Zieltonhoehe** in Cent: alle 20 ms der Grundton am Ergebnis gegen den am Klang mal
 *   β, beide als Summe der Teiltoene im Spektrum (2048 Werte, ±100 Cent um den bekannten Ton), der
 *   Median des Betrags in der Mitte.
 * - **Huelle:** die Pegel der Teiltoene (Leistung im Band ±f0/4, eine Sekunde, Hann) bis 4,5 kHz
 *   gegen die Huelle des Klangs an derselben Frequenz (linear zwischen seinen Teiltoenen), ohne den
 *   mittleren Versatz — Effektivwert in dB.
 * - **Formantlage** F1 und F2: der hoechste Teilton zwischen den Mitten zu den Nachbarformanten,
 *   parabolisch verfeinert, am Ergebnis gegen den Klang in Prozent (nur an einem Gipfel).
 * - **Pegel** gegen den Klang, und **Pegel gegen die Huelle** (die Leistung der neuen Teiltoene
 *   gegen die, die die Huelle des Klangs dort vorhersagt), in dB.
 * - **Nebengeraeusch:** Leistung ausserhalb der Baender um die Teiltoene gegen die ganze, in dB.
 * - **Rechenzeit** in Prozent der Echtzeit.
 *
 * Der dauerhafte Fall prueft PSOLA an zwei Stimmen und ±5 Halbtoenen und dass der Pruefstein
 * Signalsmith ohne Formanterhalt als solchen erkennt. **Der volle Pruefstein** mit
 * `LUMIVIZ_PRUEFSTEIN_15H=<Ordner>`: acht Stimmen, sechs Tonhoehen und eine Dehnung, jede
 * Umsetzung, dazu Signalsmith mit Formanterhalt (`dehnerSignalsmithFormanten`) und PSOLA mit der
 * Talregel von pYIN (`nurImBereich`); schreibt `pruefstein.md` und die Hoerproben dorthin; mit
 * `LUMIVIZ_VOCADITO=<Ordner von vocadito>` dazu zwei echte Stimmen als Hoerproben.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/WavDatei.hpp"
#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Dehner.hpp"
#include "loop/Fft.hpp"
#include "loop/PYin.hpp"
#include "loop/Psola.hpp"

#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <map>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 48000;
constexpr double kHuelleBisHz = 4500.0;

struct Stimme
{
    const char* name;
    double f0;
    std::array<double, 3> formanten;
    double vibratoCent = 0.0;  ///< Ausschlag, 5,5 Hz
};

/// Peterson & Barney 1952, Tab. II (Mittelwerte): Maenner /a/ 730, 1090, 2440; /i/ 270, 2290,
/// 3010; Frauen /a/ 850, 1220, 2810; /u/ 370, 950, 2670.
const std::array<Stimme, 8> kStimmen{{
    {"man_a_110", 110.0, {730.0, 1090.0, 2440.0}},
    {"man_i_130", 130.0, {270.0, 2290.0, 3010.0}},
    {"woman_a_220", 220.0, {850.0, 1220.0, 2810.0}},
    {"woman_u_262", 262.0, {370.0, 950.0, 2670.0}},
    {"woman_a_330", 330.0, {850.0, 1220.0, 2810.0}},
    {"woman_i_392", 392.0, {310.0, 2790.0, 3310.0}},
    {"woman_a_440", 440.0, {850.0, 1220.0, 2810.0}},
    {"man_a_165_vibrato", 165.0, {730.0, 1090.0, 2440.0}, 40.0},
}};

/// Der Anteil der Rahmen, die pYIN gesungen findet (die Stimme singt durchgehend).
double gesungenProzent(const std::vector<float>& d, bool nurImBereich)
{
    PYinEinstellungen e;
    e.nurImBereich = nurImBereich;
    const Tonhoehenverlauf v = messeTonhoehenverlauf(d, kRate, e);
    std::size_t gesungen = 0;
    for (const float f : v.hz) gesungen += f > 0.0f ? 1 : 0;
    return v.hz.empty() ? 0.0 : 100.0 * static_cast<double>(gesungen) / static_cast<double>(v.hz.size());
}

/// Die Stimme, `frames` lang, Mono auf beiden Seiten, Gipfel 0,5.
std::vector<float> stimme(const Stimme& s, std::int64_t frames)
{
    std::vector<double> x(static_cast<std::size_t>(frames));
    double phase = 0.0;  // in Perioden
    double vorher = 0.0;
    for (std::int64_t i = 0; i < frames; ++i)
    {
        const double t = static_cast<double>(i) / kRate;
        const double f = s.f0 * std::pow(2.0, s.vibratoCent / 1200.0 * std::sin(2.0 * std::numbers::pi * 5.5 * t));
        phase += f / kRate;
        const double p = phase - std::floor(phase);
        double fluss = 0.0;  // Rosenberg: oeffnen 0 … 0,4, schliessen 0,4 … 0,56
        if (p < 0.4)
            fluss = 0.5 * (1.0 - std::cos(std::numbers::pi * p / 0.4));
        else if (p < 0.56)
            fluss = std::cos(std::numbers::pi * (p - 0.4) / (2.0 * 0.16));
        x[static_cast<std::size_t>(i)] = fluss - vorher;
        vorher = fluss;
    }
    for (const double fz : s.formanten)
    {
        const double r = std::exp(-std::numbers::pi * (50.0 + 0.06 * fz) / kRate);
        const double c = 2.0 * r * std::cos(2.0 * std::numbers::pi * fz / kRate);
        const double g = 1.0 - c + r * r;  // 1 bei 0 Hz
        double y1 = 0.0;
        double y2 = 0.0;
        for (double& w : x)
        {
            const double y = g * w + c * y1 - r * r * y2;
            y2 = y1;
            y1 = y;
            w = y;
        }
    }
    double gipfel = 0.0;
    for (const double w : x) gipfel = std::max(gipfel, std::abs(w));
    std::vector<float> d(static_cast<std::size_t>(2 * frames));
    for (std::int64_t i = 0; i < frames; ++i)
        d[static_cast<std::size_t>(2 * i)] = d[static_cast<std::size_t>(2 * i + 1)] =
            static_cast<float>(0.5 * x[static_cast<std::size_t>(i)] / gipfel);
    return d;
}

double median(std::vector<double> w)
{
    if (w.empty()) return 0.0;
    std::nth_element(w.begin(), w.begin() + static_cast<std::ptrdiff_t>(w.size() / 2), w.end());
    return w[w.size() / 2];
}

/// Das Leistungsspektrum von `laenge` Abtastwerten ab `von` (links), Hann, aufgefuellt auf
/// `punkte`; Bin k bei k · rate / punkte.
struct Spektrum
{
    std::vector<double> leistung;
    double binHz = 0.0;

    /// Die Leistung zwischen zwei Frequenzen (Bins einschliesslich).
    [[nodiscard]] double band(double vonHz, double bisHz) const
    {
        const auto a = static_cast<std::size_t>(std::max(0.0, std::ceil(vonHz / binHz)));
        const auto b = std::min(leistung.size() - 1, static_cast<std::size_t>(std::floor(bisHz / binHz)));
        double s = 0.0;
        for (std::size_t k = a; k <= b; ++k) s += leistung[k];
        return s;
    }
    /// Der Betrag an `hz`, zwischen den Bins linear (fuer die Suche nach dem Grundton).
    [[nodiscard]] double betragBei(double hz) const
    {
        const double k = hz / binHz;
        const auto u = static_cast<std::size_t>(k);
        if (u + 1 >= leistung.size()) return 0.0;
        const double f = k - static_cast<double>(u);
        return (1.0 - f) * std::sqrt(leistung[u]) + f * std::sqrt(leistung[u + 1]);
    }
};

Spektrum spektrum(const std::vector<float>& d, std::int64_t von, std::int64_t laenge, std::size_t punkte)
{
    std::vector<double> x(punkte, 0.0);
    for (std::int64_t i = 0; i < laenge; ++i)
    {
        const std::int64_t j = von + i;
        if (j < 0 || 2 * j >= static_cast<std::int64_t>(d.size())) continue;
        const double w = 0.5 * (1.0 - std::cos(2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(laenge)));
        x[static_cast<std::size_t>(i)] = w * d[static_cast<std::size_t>(2 * j)];
    }
    std::vector<std::complex<double>> s(punkte / 2 + 1);
    fft().vorwaerts(x, s);
    Spektrum sp;
    sp.binHz = static_cast<double>(kRate) / static_cast<double>(punkte);
    for (const auto& c : s) sp.leistung.push_back(std::norm(c));
    return sp;
}

/// Der Grundton nahe `schaetzung`: die Summe der Betraege der Teiltoene bis 3 kHz, gesucht ±100
/// Cent in Schritten von 0,5 Cent, parabolisch verfeinert. Unabhaengig von pYIN — der Pruefstein
/// soll nicht messen, was er an PSOLA prueft.
double grundtonBei(const Spektrum& sp, double schaetzung)
{
    const auto summe = [&sp](double f0) {
        double s = 0.0;
        for (int h = 1; h * f0 < 3000.0; ++h) s += sp.betragBei(h * f0);
        return s;
    };
    int beste = 0;
    double besterWert = -1.0;
    std::vector<double> werte;
    for (int c = -200; c <= 200; ++c)
    {
        werte.push_back(summe(schaetzung * std::pow(2.0, c / 2400.0)));
        if (werte.back() > besterWert)
        {
            besterWert = werte.back();
            beste = c;
        }
    }
    double versatz = 0.0;
    const auto i = static_cast<std::size_t>(beste + 200);
    if (i > 0 && i + 1 < werte.size())
    {
        const double nenner = werte[i - 1] - 2.0 * werte[i] + werte[i + 1];
        if (nenner < 0.0) versatz = std::clamp(0.5 * (werte[i - 1] - werte[i + 1]) / nenner, -0.5, 0.5);
    }
    return schaetzung * std::pow(2.0, (beste + versatz) / 2400.0);
}

/// Der Grundton der Stimme bei Abtastwert t des Klangs (bekannt, mit Vibrato).
double sollF0(const Stimme& s, double t)
{
    return s.f0 * std::pow(2.0, s.vibratoCent / 1200.0 * std::sin(2.0 * std::numbers::pi * 5.5 * t / kRate));
}

/// Der Fehler der Zieltonhoehe in Cent: je 20 ms in der Mitte der Grundton am Ergebnis gegen den
/// am Klang mal β (beide mit `grundtonBei` ueber 2048 Abtastwerte), Median des Betrags. Bei
/// Dehnung liegt Abtastwert t des Ergebnisses bei t · Verhaeltnis im Klang.
double centFehler(const Stimme& s, const std::vector<float>& ein, const std::vector<float>& aus, double beta)
{
    const auto n = static_cast<std::int64_t>(ein.size() / 2);
    const auto ausN = static_cast<std::int64_t>(aus.size() / 2);
    const double verhaeltnis = static_cast<double>(n) / static_cast<double>(ausN);
    std::vector<double> fehler;
    for (std::int64_t t = ausN / 4; t < 3 * ausN / 4; t += kRate / 50)
    {
        const double tEin = static_cast<double>(t) * verhaeltnis;
        const double f0Ein = grundtonBei(spektrum(ein, std::llround(tEin) - 1024, 2048, 16384), sollF0(s, tEin));
        const double f0Aus = grundtonBei(spektrum(aus, t - 1024, 2048, 16384), f0Ein * beta);
        fehler.push_back(std::abs(1200.0 * std::log2(f0Aus / (f0Ein * beta))));
    }
    return median(fehler);
}

/// Die Teiltoene einer Sekunde ab `von`: je Teilton Frequenz und Amplitude aus der Leistung im Band
/// ±f0/4 (unempfindlich gegen ein wenig Fehler im Grundton).
struct Teilton
{
    double hz;
    double amplitude;
};

std::vector<Teilton> teiltoene(const Spektrum& sp, double f0, double bisHz)
{
    std::vector<Teilton> t;
    for (int h = 1; h * f0 < bisHz; ++h) t.push_back({h * f0, std::sqrt(sp.band((h - 0.25) * f0, (h + 0.25) * f0))});
    return t;
}

double db(double a)
{
    return 20.0 * std::log10(std::max(a, 1e-12));
}

/// Die Huelle (dB) an `hz`, linear zwischen den Teiltoenen; ausserhalb NaN.
double huelleBei(const std::vector<Teilton>& t, double hz)
{
    for (std::size_t i = 1; i < t.size(); ++i)
        if (hz >= t[i - 1].hz && hz <= t[i].hz)
        {
            const double f = (hz - t[i - 1].hz) / (t[i].hz - t[i - 1].hz);
            return db(t[i - 1].amplitude) + f * (db(t[i].amplitude) - db(t[i - 1].amplitude));
        }
    return std::nan("");
}

/// Wie weit der Pegel von dem abweicht, den die Huelle des Klangs vorhersagt: die Leistung der
/// Teiltoene des Ergebnisses gegen die Leistung der Huelle an ihren Frequenzen, in dB. Bei 0 dB
/// folgt ein Pegelverlust allein daraus, dass die neuen Teiltoene anders unter den Formanten liegen.
double pegelGegenHuelle(const std::vector<Teilton>& ein, const std::vector<Teilton>& aus)
{
    double ist = 0.0;
    double soll = 0.0;
    for (const Teilton& t : aus)
    {
        const double h = huelleBei(ein, t.hz);
        if (std::isnan(h)) continue;
        ist += t.amplitude * t.amplitude;
        soll += std::pow(10.0, h / 10.0);
    }
    return soll > 0.0 ? 10.0 * std::log10(ist / soll) : std::nan("");
}

/// Der Fehler der Huelle in dB: Effektivwert ohne mittleren Versatz.
double huellenFehler(const std::vector<Teilton>& ein, const std::vector<Teilton>& aus)
{
    std::vector<double> d;
    for (const Teilton& t : aus)
    {
        const double h = huelleBei(ein, t.hz);
        if (!std::isnan(h)) d.push_back(db(t.amplitude) - h);
    }
    if (d.empty()) return 99.0;
    double mittel = 0.0;
    for (const double w : d) mittel += w;
    mittel /= static_cast<double>(d.size());
    double summe = 0.0;
    for (const double w : d) summe += (w - mittel) * (w - mittel);
    return std::sqrt(summe / static_cast<double>(d.size()));
}

/// Die Lage des Formanten i: der hoechste Teilton zwischen den Mitten zu den Nachbarformanten
/// (unter F1 ab 0,6 · F1), parabolisch in dB ueber seine Nachbarn. 0 = nicht zu messen (kein
/// Gipfel: der hoechste ist kleiner als ein Nachbar ausserhalb des Bereichs, oder ohne Nachbarn).
double formant(const std::vector<Teilton>& t, const std::array<double, 3>& f, std::size_t i)
{
    const double unten = i == 0 ? 0.6 * f[0] : 0.5 * (f[i - 1] + f[i]);
    const double oben = i + 1 < f.size() ? 0.5 * (f[i] + f[i + 1]) : 1.3 * f[i];
    std::size_t beste = t.size();
    for (std::size_t k = 0; k < t.size(); ++k)
        if (t[k].hz >= unten && t[k].hz <= oben && (beste == t.size() || t[k].amplitude > t[beste].amplitude)) beste = k;
    if (beste == t.size() || beste == 0 || beste + 1 >= t.size()) return 0.0;
    if (t[beste - 1].amplitude > t[beste].amplitude || t[beste + 1].amplitude > t[beste].amplitude) return 0.0;
    const double a = db(t[beste - 1].amplitude);
    const double b = db(t[beste].amplitude);
    const double c = db(t[beste + 1].amplitude);
    const double nenner = a - 2.0 * b + c;
    const double delta = nenner < 0.0 ? std::clamp(0.5 * (a - c) / nenner, -0.5, 0.5) : 0.0;
    return t[beste].hz + delta * (t[beste + 1].hz - t[beste].hz);
}

double effektiv(const std::vector<float>& d, std::int64_t von, std::int64_t bis)
{
    double summe = 0.0;
    for (std::int64_t i = von; i < bis; ++i) summe += static_cast<double>(d[static_cast<std::size_t>(2 * i)]) * d[static_cast<std::size_t>(2 * i)];
    return std::sqrt(summe / static_cast<double>(bis - von));
}

/// Leistung ausserhalb der Baender ±f0/4 um die Teiltoene gegen die ganze, in dB, bis 20 kHz.
double nebengeraeusch(const Spektrum& sp, double f0)
{
    double teil = 0.0;
    for (const Teilton& t : teiltoene(sp, f0, 20000.0)) teil += t.amplitude * t.amplitude;
    const double alle = sp.band(0.75 * f0, 20000.0);
    return 10.0 * std::log10(std::max(alle - teil, alle * 1e-9) / alle);
}

/// Ein Ergebnis des Pruefsteins.
struct Messung
{
    double cent = 0.0;
    double huelleDb = 0.0;
    double f1Prozent = std::nan("");
    double f2Prozent = std::nan("");
    double f1Hz = 0.0;
    double f2Hz = 0.0;
    double pegelDb = 0.0;
    double pegelHuelleDb = std::nan("");  ///< `pegelGegenHuelle`
    double nebenDb = 0.0;   ///< des Ergebnisses
    double nebenEinDb = 0.0;  ///< des Klangs
    double ms = 0.0;
};

/// Eine Stimme durch eine Umsetzung, `ht` Halbtoene, Laenge mal `dehnung`; gemessen in der Mitte.
Messung miss(const Dehner& u, const Stimme& s, const std::vector<float>& ein, double ht, double dehnung,
             std::vector<float>* ausgabe = nullptr)
{
    const auto n = static_cast<std::int64_t>(ein.size() / 2);
    const auto ausN = static_cast<std::int64_t>(std::llround(static_cast<double>(n) * dehnung));
    const auto anfang = std::chrono::steady_clock::now();
    std::vector<float> aus = u.rechne(ein, kRate, ausN, [ht](std::int64_t) { return ht; });
    Messung m;
    m.ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - anfang).count();
    const double beta = std::pow(2.0, ht / 12.0);
    m.cent = centFehler(s, ein, aus, beta);
    const std::int64_t mitteEin = n / 2 - kRate / 2;
    const std::int64_t mitteAus = ausN / 2 - kRate / 2;
    if (s.vibratoCent == 0.0)
    {
        const Spektrum se = spektrum(ein, mitteEin, kRate, 65536);
        const Spektrum sa = spektrum(aus, mitteAus, kRate, 65536);
        const double f0Ein = grundtonBei(se, s.f0);
        const double f0Aus = grundtonBei(sa, f0Ein * beta);
        const std::vector<Teilton> te = teiltoene(se, f0Ein, kHuelleBisHz);
        const std::vector<Teilton> ta = teiltoene(sa, f0Aus, kHuelleBisHz);
        m.huelleDb = huellenFehler(te, ta);
        m.pegelHuelleDb = pegelGegenHuelle(te, ta);
        const double f1e = formant(te, s.formanten, 0);
        const double f1a = formant(ta, s.formanten, 0);
        const double f2e = formant(te, s.formanten, 1);
        const double f2a = formant(ta, s.formanten, 1);
        m.f1Hz = f1a;
        m.f2Hz = f2a;
        if (f1e > 0.0 && f1a > 0.0) m.f1Prozent = 100.0 * (f1a / f1e - 1.0);
        if (f2e > 0.0 && f2a > 0.0) m.f2Prozent = 100.0 * (f2a / f2e - 1.0);
        m.nebenDb = nebengeraeusch(sa, f0Aus);
        m.nebenEinDb = nebengeraeusch(se, f0Ein);
    }
    m.pegelDb = db(effektiv(aus, mitteAus, mitteAus + kRate)) - db(effektiv(ein, mitteEin, mitteEin + kRate));
    if (ausgabe) *ausgabe = std::move(aus);
    return m;
}

/// Kurzname einer Umsetzung fuer Dateien und Tabellen.
std::string kurz(const Dehner& u)
{
    if (u.name().rfind("TD-PSOLA", 0) == 0) return u.name().find("range") != std::string::npos ? "psola_range" : "psola";
    return u.behaeltFormanten() ? "signalsmith_formants" : "signalsmith";
}

QString zahl(double w, int stellen = 1)
{
    return std::isnan(w) ? QStringLiteral("–") : QString::number(w, 'f', stellen);
}

}  // namespace

TEST_SUITE("LoopDehnerPruefstein")
{
    TEST_CASE("Loop Dehner 15h Marken: je Periode eine, und ohne Aenderung gibt PSOLA den Klang zurueck")
    {
        const Stimme& s = kStimmen[0];
        const std::vector<float> ein = stimme(s, 2 * kRate);
        std::vector<double> mono(ein.size() / 2);
        for (std::size_t i = 0; i < mono.size(); ++i) mono[i] = ein[2 * i];
        const std::vector<PsolaMarke> marken = psolaMarken(mono, kRate, messeTonhoehenverlauf(ein, kRate));
        // in der Mitte gesungen, Abstand eine Periode (auf 3 %) bei 95 % der Marken
        int gut = 0;
        int alle = 0;
        for (std::size_t i = 1; i < marken.size(); ++i)
        {
            if (marken[i].stelle < kRate / 2 || marken[i].stelle > 3 * kRate / 2) continue;
            ++alle;
            CHECK(marken[i].gesungen);
            const double abstand = static_cast<double>(marken[i].stelle - marken[i - 1].stelle);
            if (std::abs(abstand / (kRate / s.f0) - 1.0) < 0.03) ++gut;
        }
        CHECK(alle == doctest::Approx(s.f0).epsilon(0.02));
        CHECK(gut >= 0.95 * alle);
        // Stille hat Marken alle 5 ms und ist nicht gesungen
        const std::vector<double> still(static_cast<std::size_t>(kRate / 10), 0.0);
        const std::vector<PsolaMarke> leer = psolaMarken(still, kRate, messeTonhoehenverlauf(std::vector<float>(kRate / 5, 0.0f), kRate));
        REQUIRE(leer.size() == 20);
        CHECK(leer[1] == PsolaMarke{240, false});

        // ohne Aenderung: die Koerner ergeben den Klang wieder (zwischen der ersten und letzten Marke)
        const std::vector<float> aus = dehnerPsola()->rechne(ein, kRate, 2 * kRate, {});
        double groesste = 0.0;
        for (std::int64_t i = kRate / 10; i < 2 * kRate - kRate / 10; ++i)
            groesste = std::max(groesste, static_cast<double>(std::abs(aus[static_cast<std::size_t>(2 * i)] - ein[static_cast<std::size_t>(2 * i)])));
        CHECK(groesste < 1e-6);
    }

    TEST_CASE("Loop Dehner 15h Pruefstein: PSOLA haelt Formanten und trifft die Tonhoehe")
    {
        const std::unique_ptr<Dehner> psola = dehnerPsola();
        const std::unique_ptr<Dehner> signalsmith = std::move(dehnerUmsetzungen().front());
        for (const Stimme* s : {&kStimmen[0], &kStimmen[2]})
        {
            const std::string name = s->name;
            CAPTURE(name);
            const std::vector<float> ein = stimme(*s, 2 * kRate);
            for (const double ht : {-5.0, 5.0})
            {
                CAPTURE(ht);
                const Messung p = miss(*psola, *s, ein, ht, 1.0);
                MESSAGE("PSOLA " << name << " " << ht << ": " << p.cent << " Cent, Huelle " << p.huelleDb << " dB, F1 "
                                 << p.f1Prozent << " %, F2 " << p.f2Prozent << " %, Pegel " << p.pegelDb << " dB, Neben "
                                 << p.nebenDb << " dB (Klang " << p.nebenEinDb << ")");
                CHECK(p.cent < 5.0);
                CHECK(p.huelleDb < 3.0);
                CHECK(std::abs(p.pegelDb) < 3.0);
                // ohne Formanterhalt wandert die Huelle mit: der Pruefstein muss das sehen
                const Messung g = miss(*signalsmith, *s, ein, ht, 1.0);
                CHECK(g.cent < 5.0);
                CHECK(g.huelleDb > 2.0 * p.huelleDb);
            }
        }
    }

    TEST_CASE("Loop Dehner 15h Pruefstein voll (LUMIVIZ_PRUEFSTEIN_15H)")
    {
        const QString ordner = qEnvironmentVariable("LUMIVIZ_PRUEFSTEIN_15H");
        if (ordner.isEmpty())
        {
            MESSAGE("LUMIVIZ_PRUEFSTEIN_15H nicht gesetzt — der volle Pruefstein laeuft nicht");
            return;
        }
        REQUIRE(QDir().mkpath(ordner));
        std::vector<std::unique_ptr<Dehner>> alle = dehnerUmsetzungen();
        // zum Vergleich: Signalsmith mit Formanterhalt (der Gegenkandidat), PSOLA mit Marken aus pYIN
        // mit der Talregel (Befund S170)
        alle.insert(alle.begin() + 1, dehnerSignalsmithFormanten());
        PYinEinstellungen imBereich;
        imBereich.nurImBereich = true;
        alle.push_back(dehnerPsola(imBereich));
        const std::array<double, 6> tonhoehen{-12.0, -7.0, -3.0, 3.0, 7.0, 12.0};
        QString text;
        QTextStream t(&text);
        t << "# Prüfstein 15h — Dehner an gerechneten Stimmen\n\n"
          << "Geschrieben von `test_LoopDehnerPruefstein.cpp` (`LUMIVIZ_PRUEFSTEIN_15H`). Je Zeile: Fehler der "
             "Zieltonhöhe (Median, Cent), Fehler der Hülle bis 4,5 kHz (dB), Lage F1 und F2 gegen den Klang (%), "
             "Pegel gegen den Klang (dB), Pegel gegen die Hülle des Klangs an den neuen Teiltönen (dB; 0 = der "
             "Pegel folgt allein den Formanten), Nebengeräusch des Ergebnisses (dB, in Klammern der Klang).\n\n";
        std::map<std::string, double> msJe;
        std::map<std::string, double> sekundenJe;
        for (const Stimme& s : kStimmen)
        {
            const std::vector<float> ein = stimme(s, 3 * kRate);
            REQUIRE(schreibeWav(QDir(ordner).filePath(QString::fromLatin1(s.name) + QStringLiteral("_0.wav")), ein, kRate));
            t << "## " << s.name << " (F " << s.formanten[0] << ", " << s.formanten[1] << ", " << s.formanten[2] << " Hz"
              << (s.vibratoCent > 0.0 ? ", Vibrato ±40 Cent" : "") << ")\n\n"
              << "pYIN findet am Klang " << zahl(gesungenProzent(ein, false)) << " % der Rahmen gesungen, nur im Bereich des HMM "
              << zahl(gesungenProzent(ein, true)) << " % (PSOLA verschiebt nur, wo gesungen).\n\n"
              << "| Umsetzung | Halbtöne | Dauer | Cent | Hülle dB | F1 % | F2 % | F1 Hz | F2 Hz | Pegel dB | Pegel gegen Hülle dB | Neben dB |\n"
              << "|---|---|---|---|---|---|---|---|---|---|---|---|\n";
            std::vector<std::pair<double, double>> faelle;
            for (const double ht : tonhoehen) faelle.emplace_back(ht, 1.0);
            faelle.emplace_back(0.0, 1.25);
            for (const auto& [ht, dehnung] : faelle)
                for (const auto& u : alle)
                {
                    std::vector<float> aus;
                    const Messung m = miss(*u, s, ein, ht, dehnung, &aus);
                    msJe[kurz(*u)] += m.ms;
                    sekundenJe[kurz(*u)] += 3.0 * dehnung;
                    t << "| " << QString::fromStdString(kurz(*u)) << " | " << ht << " | " << dehnung << " | " << zahl(m.cent)
                      << " | " << (s.vibratoCent > 0.0 ? QStringLiteral("–") : zahl(m.huelleDb)) << " | " << zahl(m.f1Prozent)
                      << " | " << zahl(m.f2Prozent) << " | " << zahl(m.f1Hz, 0) << " | " << zahl(m.f2Hz, 0) << " | "
                      << zahl(m.pegelDb) << " | " << zahl(m.pegelHuelleDb) << " | "
                      << (s.vibratoCent > 0.0 ? QStringLiteral("–") : zahl(m.nebenDb) + QStringLiteral(" (") + zahl(m.nebenEinDb) + QStringLiteral(")"))
                      << " |\n";
                    const QString datei = QStringLiteral("%1_%2%3_%4.wav")
                                              .arg(QString::fromLatin1(s.name))
                                              .arg(ht > 0.0 ? QStringLiteral("+") : QString())
                                              .arg(dehnung == 1.0 ? QString::number(ht) : QStringLiteral("stretch125"))
                                              .arg(QString::fromStdString(kurz(*u)));
                    REQUIRE(schreibeWav(QDir(ordner).filePath(datei), aus, kRate));
                }
            t << "\n";
        }
        t << "## Rechenzeit (Release oder Testing, wie gebaut)\n\n| Umsetzung | % der Echtzeit |\n|---|---|\n";
        for (const auto& [name, ms] : msJe) t << "| " << QString::fromStdString(name) << " | " << zahl(ms / 10.0 / sekundenJe[name], 2) << " |\n";

        // echte Stimmen (vocadito, CC BY 4.0) nur als Hoerproben
        const QString vocadito = qEnvironmentVariable("LUMIVIZ_VOCADITO");
        if (!vocadito.isEmpty())
            for (const int nr : {2, 4})
            {
                const auto inhalt = lumi::audio::liesWav(QDir(vocadito).filePath(QStringLiteral("Audio/vocadito_%1.wav").arg(nr)));
                REQUIRE(inhalt.has_value());
                std::vector<float> ein = inhalt->stereo;
                ein.resize(std::min<std::size_t>(ein.size(), static_cast<std::size_t>(2 * 8 * inhalt->rate)));
                const QString name = QStringLiteral("vocadito_%1").arg(nr);
                REQUIRE(schreibeWav(QDir(ordner).filePath(name + QStringLiteral("_0.wav")), ein, inhalt->rate));
                for (const double ht : {-7.0, -3.0, 3.0, 7.0})
                    for (const auto& u : alle)
                    {
                        const std::vector<float> aus = u->rechne(ein, inhalt->rate, static_cast<std::int64_t>(ein.size() / 2),
                                                                 [ht](std::int64_t) { return ht; });
                        const QString probe = QStringLiteral("%1_%2%3_%4.wav")
                                                  .arg(name)
                                                  .arg(ht > 0.0 ? QStringLiteral("+") : QString())
                                                  .arg(ht)
                                                  .arg(QString::fromStdString(kurz(*u)));
                        REQUIRE(schreibeWav(QDir(ordner).filePath(probe), aus, inhalt->rate));
                    }
            }

        QFile datei(QDir(ordner).filePath(QStringLiteral("pruefstein.md")));
        REQUIRE(datei.open(QIODevice::WriteOnly | QIODevice::Truncate));
        datei.write(text.toUtf8());
        MESSAGE("pruefstein.md und Hoerproben unter " << ordner.toStdString());
    }
}

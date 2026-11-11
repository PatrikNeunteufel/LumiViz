// S190, 34j-5: Pruefstand der durchschlagenden Zunge ohne App — Vorgaben je Taste gegen alle Zielgroessen.
//
// Rechnet eine Zunge mit denselben Rechenschritten wie der Kern `freeReed` (InstrumentModels/zunge/Zunge.cpp,
// `rechneEineZunge`; ohne Bend, Vibrato, Balgwechsel), aber mit den Groessen je Taste unmittelbar (Laenge, Breite,
// Masse, Ruhelage, Seitenspalt, Daempfung, Kanal, Kanzelle), damit sich Gesetze ueber den Umfang pruefen lassen, die
// der Kern noch nicht kennt. Der erste Schritt jedes Laufs prueft Bit fuer Bit gegen den Kern.
//
// Messkette (alles an der gehaltenen Sekunde, 48 kHz): Tonhoehe aus den Nulldurchgaengen der Auslenkung; Lautheit
// K-bewertet nach BS.1770 ueber die letzten 250 ms (ohne Tor, nur Unterschiede zaehlen); Einschwingen = Zeit bis −5 dB
// der Endhuelle des Schalls und Anstieg −50 → −5 dB (Llanos-Vazquez u. a. 2014); Amplitude der Spitze gegen Laenge
// und Ruhelage; "spricht" = Amplitude ≥ ¼ der Ruhelage und in den letzten 250 ms nicht mehr als 10 % gewachsen.
//
// Bau: 2026-11-11_S190_bau.bat <quelle.cpp> <ziel.exe>;  Aufruf: <exe> [Teil des Satznamens]
#include "InstrumentModels/zunge/Zunge.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <numbers>
#include <string>
#include <thread>
#include <vector>

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kRho = 1.2041, kC = 343.0;
constexpr int kUeber = 4;
constexpr int kRate = 48000;
constexpr double kHalten = 1.0;
constexpr std::int64_t kFrames = 57600;

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

/// Die Groessen einer Taste, wie der Kern sie aus `ZungenSpiel` bildet.
struct Taste
{
    int note = 60;
    double hz = 0.0, f0 = 0.0;
    double laenge = 0.0, breite = 0.0, masse = 0.0;
    double ruhe = 0.0, spalt = 0.0, zeta = 0.0;
    double kanalLaenge = 0.0, kanalFlaeche = 0.0, kanzelle = 0.0, ventil = 0.0;
    // Modell nach Adachi u. a. 2026 (Gl. 2–7): Druckverlust an Klappe und Schlitz, Kanzellenquerschnitt
    bool adachi = false;
    double kp = 35.0, kr = 8.0, sc = 14.8e-3 * 14.6e-3, rho = 1.2041;
    double traegFaktor = 0.5;  ///< d = Faktor·√(S/π) (Adachi: 0,5, „to match with the experiment by Ricot et al.“)
};

/// Adachi u. a. 2026: ρ·(d/S)·U̇ = p₀ − K_p·ρ/2·(U/S_c)² − K_r·ρ/2·(U/S)² (Gl. 2), S = (L + W)·√(max(x,0)² + a²)
/// (Gl. 4), d = 0,5·√(S/π), p_r = K_r·ρ/2·(U/S)² (Gl. 5), ẍ = −ω₀²(x − x_rest) − 2ζω₀ẋ − (S_r/M)·p_r mit
/// S_r = L·W (Gl. 6), p_out = ρ·ΔL/S_r·U̇ mit ΔL = √(S_r/π) (Gl. 7). U implizit wie im Kern.
void rechneAdachi(const struct Taste& z, double p0, std::vector<double>& aus, std::vector<double>* spur, double* uSpitze, int ueber,
                  double anstiegS = 0.02, double haltenS = 1.0, std::int64_t frames = 57600);

/// Die Schleife des Kerns (Zunge.cpp:29–100), ohne Bend, Vibrato, Balgwechsel; dazu die Auslenkung je Ausgangswert.
void rechne(const Taste& z, double p0, std::vector<double>& aus, std::vector<double>* spur)
{
    if (z.adachi)
    {
        rechneAdachi(z, p0, aus, spur, nullptr, kUeber);
        return;
    }
    aus.assign(static_cast<std::size_t>(kFrames), 0.0);
    if (spur != nullptr) spur->assign(static_cast<std::size_t>(kFrames), 0.0);
    const double fs = static_cast<double>(kRate) * kUeber, dt = 1.0 / fs;
    const double laenge = z.laenge, masse = z.masse;
    const double w0 = 2.0 * kPi * z.f0;
    const double steife = masse * w0 * w0, daempfung = 2.0 * z.zeta * masse * w0;
    const double sr = 0.5 * laenge * z.breite;
    const double ventil = z.ventil * std::pow(10.0, 2.0 * 0.0 / 100.0);
    const double traegheit = kRho * z.kanalLaenge / z.kanalFlaeche;
    const double kKanz = kRho * kC * kC / z.kanzelle;
    const auto halten = static_cast<std::int64_t>(std::llround(kHalten * kRate)) * kUeber;
    double y = 0.0, v = 0.0, p1 = 0.0, u = 0.0, uVor = 0.0;
    for (std::int64_t n = 0; n < kFrames * kUeber; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        double env = std::min(1.0, t / 0.02);
        if (n >= halten) env *= std::max(0.0, 1.0 - (t - kHalten) / 0.03);
        const double p = p0 * env * (1.0 + 0.002 * 0.0 * std::sin(0.0));
        const double dpv = p - p1;
        const double uEin = ventil * std::sqrt(2.0 * std::abs(dpv) / kRho) * (dpv >= 0.0 ? 1.0 : -1.0);
        const double su = z.breite * std::sqrt((y + z.ruhe) * (y + z.ruhe) + z.spalt * z.spalt);
        const double c = 0.5 * kRho / (su * su);
        const double rhs = u + dt * p1 / traegheit, g = dt * c / traegheit;
        u = rhs >= 0.0 ? (-1.0 + std::sqrt(1.0 + 4.0 * g * rhs)) / (2.0 * g) : (1.0 - std::sqrt(1.0 - 4.0 * g * rhs)) / (2.0 * g);
        const double p2 = c * u * std::abs(u);
        p1 += dt * kKanz * (uEin - u + sr * v);
        const double kraft = -sr * p2 - steife * y - daempfung * v;
        v += dt * kraft / masse;
        y += dt * v;
        if (n % kUeber == kUeber - 1)
        {
            const auto i = static_cast<std::size_t>(n / kUeber);
            aus[i] = kRho / (4.0 * kPi) * (u - uVor) * kRate;
            uVor = u;
            if (spur != nullptr) (*spur)[i] = y;
        }
    }
}

void rechneAdachi(const Taste& z, double p0, std::vector<double>& aus, std::vector<double>* spur, double* uSpitze, int ueber, double anstiegS,
                  double haltenS, std::int64_t frames)
{
    aus.assign(static_cast<std::size_t>(frames), 0.0);
    if (spur != nullptr) spur->assign(static_cast<std::size_t>(frames), 0.0);
    const double dt = 1.0 / (static_cast<double>(kRate) * ueber);
    const double w0 = 2.0 * kPi * z.f0;
    const double sr = z.laenge * z.breite, rand = z.laenge + z.breite;
    const double dl = std::sqrt(sr / kPi);
    const auto halten = static_cast<std::int64_t>(std::llround(haltenS * kRate)) * ueber;
    double x = z.ruhe, v = 0.0, u = 0.0, uVor = 0.0, uMax = 0.0;
    for (std::int64_t n = 0; n < frames * ueber; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        double env = anstiegS > 0.0 ? std::min(1.0, t / anstiegS) : 1.0;
        if (n >= halten) env *= std::max(0.0, 1.0 - (t - haltenS) / 0.03);
        const double p = p0 * env;
        const double offen = std::max(x, 0.0);
        const double s = rand * std::sqrt(offen * offen + z.spalt * z.spalt);
        const double traegheit = z.rho * z.traegFaktor / std::sqrt(kPi * s);
        const double c = 0.5 * z.rho * (z.kp / (z.sc * z.sc) + z.kr / (s * s));
        const double rhs = u + dt * p / traegheit, g = dt * c / traegheit;
        u = rhs >= 0.0 ? (-1.0 + std::sqrt(1.0 + 4.0 * g * rhs)) / (2.0 * g) : (1.0 - std::sqrt(1.0 - 4.0 * g * rhs)) / (2.0 * g);
        const double pr = 0.5 * z.kr * z.rho * (u / s) * (u / s);
        const double beschl = -w0 * w0 * (x - z.ruhe) - 2.0 * z.zeta * w0 * v - sr / z.masse * pr;
        v += dt * beschl;
        x += dt * v;
        if (n > halten / 2 && n < halten) uMax = std::max(uMax, u);
        if (n % ueber == ueber - 1)
        {
            const auto i = static_cast<std::size_t>(n / ueber);
            aus[i] = z.rho * dl / sr * (u - uVor) * kRate;
            uVor = u;
            if (spur != nullptr) (*spur)[i] = x;
        }
    }
    if (uSpitze != nullptr) *uSpitze = uMax;
}

struct Mass
{
    bool spricht = false, waechst = false;
    double hz = 0.0, lufs = -300.0;
    double ampLaenge = 0.0, ampRuhe = 0.0, mitteRuhe = 0.0;
    double einS = -1.0, anstiegS = -1.0;
};

Mass messe(const Taste& z, double p0)
{
    std::vector<double> aus, y;
    rechne(z, p0, aus, &y);
    Mass m;
    const auto ende = static_cast<std::size_t>(kHalten * kRate);
    const std::size_t viertel = kRate / 4;
    auto spanne = [&](std::size_t von, std::size_t bis, double& mitte) {
        double lo = 1e30, hi = -1e30;
        for (std::size_t i = von; i < bis; ++i)
        {
            lo = std::min(lo, y[i]);
            hi = std::max(hi, y[i]);
        }
        mitte = 0.5 * (lo + hi);
        return 0.5 * (hi - lo);
    };
    double mitte = 0.0, mitteVor = 0.0;
    const double amp = spanne(ende - viertel, ende, mitte);
    const double ampVor = spanne(ende - 2 * viertel, ende - viertel, mitteVor);
    m.ampLaenge = amp / z.laenge;
    m.ampRuhe = amp / z.ruhe;
    m.mitteRuhe = mitte / z.ruhe;
    m.waechst = amp > 1.1 * ampVor;
    m.spricht = amp >= 0.25 * z.ruhe && !m.waechst;
    // Tonhoehe: aufsteigende Nulldurchgaenge um die Mitte
    double erster = -1.0, letzter = -1.0;
    int zahl = 0;
    for (std::size_t i = ende - viertel + 1; i < ende; ++i)
    {
        const double a = y[i - 1] - mitte, b = y[i] - mitte;
        if (a < 0.0 && b >= 0.0)
        {
            const double t = (static_cast<double>(i - 1) + a / (a - b)) / kRate;
            if (zahl == 0) erster = t;
            letzter = t;
            ++zahl;
        }
    }
    if (zahl >= 3) m.hz = (zahl - 1) / (letzter - erster);
    // Lautheit: K-Bewertung (BS.1770, 48 kHz), Mittel der Quadrate ueber das letzte Viertel
    {
        const double b1[3] = {1.53512485958697, -2.69169618940638, 1.19839281085285}, a1[2] = {-1.69065929318241, 0.73248077421585};
        const double b2[3] = {1.0, -2.0, 1.0}, a2[2] = {-1.99004745483398, 0.99007225036621};
        double x1 = 0, x2 = 0, y1 = 0, y2 = 0, u1 = 0, u2 = 0, w1 = 0, w2 = 0, summe = 0.0;
        for (std::size_t i = 0; i < ende; ++i)
        {
            const double x = aus[i];
            const double s = b1[0] * x + b1[1] * x1 + b1[2] * x2 - a1[0] * y1 - a1[1] * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = s;
            const double w = b2[0] * s + b2[1] * u1 + b2[2] * u2 - a2[0] * w1 - a2[1] * w2;
            u2 = u1;
            u1 = s;
            w2 = w1;
            w1 = w;
            if (i >= ende - viertel) summe += w * w;
        }
        const double ms = summe / static_cast<double>(viertel);
        m.lufs = ms > 0.0 ? -0.691 + 10.0 * std::log10(ms) : -300.0;
    }
    // Einschwingen am Schall: Spitze je 5 ms
    {
        const std::size_t block = kRate / 200;
        std::vector<double> h;
        for (std::size_t i = 0; i + block <= ende; i += block)
        {
            double s = 0.0;
            for (std::size_t k = 0; k < block; ++k) s = std::max(s, std::abs(aus[i + k]));
            h.push_back(s);
        }
        double end = 0.0;
        for (std::size_t i = h.size() - 50; i < h.size(); ++i) end = std::max(end, h[i]);
        double t5 = -1.0, t50 = -1.0;
        for (std::size_t i = 0; i < h.size(); ++i)
        {
            if (t50 < 0.0 && h[i] >= end * std::pow(10.0, -50.0 / 20.0)) t50 = 0.005 * static_cast<double>(i);
            if (t5 < 0.0 && h[i] >= end * std::pow(10.0, -5.0 / 20.0)) t5 = 0.005 * static_cast<double>(i);
        }
        m.einS = t5;
        m.anstiegS = t5 - t50;
    }
    return m;
}

void parallel(std::size_t n, const std::function<void(std::size_t)>& f)
{
    std::atomic<std::size_t> naechste{0};
    std::vector<std::thread> t;
    for (int k = 0; k < 10; ++k)
        t.emplace_back([&] {
            for (std::size_t i; (i = naechste.fetch_add(1)) < n;) f(i);
        });
    for (auto& x : t) x.join();
}

/// Ein Gesetz ueber den Umfang: Wert an C4 mal (f / f_C4)^Exponent.
struct Potenz
{
    double c4 = 0.0, hoch = 0.0;
    [[nodiscard]] double an(double hz) const { return c4 * std::pow(hz / hzVon(60.0), hoch); }
};

struct Satz
{
    std::string name;
    int noteMin = 53, noteMax = 93, schritt = 3;
    double windMin = 300.0, windMax = 2000.0;
    Potenz laenge, breite, flaechenmasse /* Masse je wirksamer Flaeche ½·L·W, kg/m² */, ruhe, spalt, kanalLaenge, kanalFlaeche;
    Potenz zeta{0.003, 0.0};
    Potenz kanzelle{14.8e-3 * 14.6e-3 * 20e-3, 0.0}, ventil{3e-4, 0.0};
    bool wieKern = false;  ///< Masse und Laenge mit den Ausdruecken des Kerns (Bit fuer Bit)
    bool adachi = false;   ///< Modell nach Adachi u. a. 2026; `flaechenmasse` dann M/(L·W)
    double kp = 35.0, kr = 8.0, traegFaktor = 0.5;
    Potenz sc{14.8e-3 * 14.6e-3, 0.0};
};

Taste tasteAus(const Satz& s, int note)
{
    Taste z;
    z.note = note;
    z.hz = hzVon(note);
    if (s.wieKern)
    {
        z.laenge = s.laenge.c4 * std::sqrt(hzVon(60.0) / z.hz);
        z.masse = (s.flaechenmasse.c4 * 0.5 * s.laenge.c4 * s.breite.c4) * z.laenge / s.laenge.c4;
    }
    else
    {
        z.laenge = s.laenge.an(z.hz);
    }
    z.breite = s.breite.an(z.hz);
    if (!s.wieKern) z.masse = s.flaechenmasse.an(z.hz) * (s.adachi ? 1.0 : 0.5) * z.laenge * z.breite;
    z.adachi = s.adachi;
    z.kp = s.kp;
    z.kr = s.kr;
    z.traegFaktor = s.traegFaktor;
    z.sc = s.sc.an(z.hz);
    z.ruhe = s.ruhe.an(z.hz);
    z.spalt = s.spalt.an(z.hz);
    z.zeta = s.zeta.an(z.hz);
    z.kanalLaenge = s.kanalLaenge.an(z.hz);
    z.kanalFlaeche = s.kanalFlaeche.an(z.hz);
    z.kanzelle = s.kanzelle.an(z.hz);
    z.ventil = s.ventil.an(z.hz);
    z.f0 = 1.02 * z.hz;
    return z;
}

double mitteVon(const Satz& s)
{
    return std::sqrt(s.windMin * s.windMax);
}

/// f₀ so, dass die Zunge am mittleren Druck auf der Note klingt (ZV2 a); falsch, wenn sie dort nicht anspricht.
bool stimme(Taste& z, double druck)
{
    for (int runde = 0; runde < 6; ++runde)
    {
        const Mass m = messe(z, druck);
        if (m.ampRuhe < 0.25 || m.hz <= 0.0) return false;
        const double cent = 1200.0 * std::log2(m.hz / z.hz);
        if (std::abs(cent) < 0.05) return true;
        z.f0 *= z.hz / m.hz;
    }
    return true;
}

double schliessdruck(const Taste& z)
{
    const double w0 = 2.0 * kPi * z.f0;
    return z.masse * w0 * w0 * z.ruhe / ((z.adachi ? 1.0 : 0.5) * z.laenge * z.breite);
}

const std::vector<double> kDruecke = {50, 71, 100, 141, 200, 283, 400, 566, 800, 1131, 1600, 2263, 3200, 4525, 6400};

void berichte(const Satz& s)
{
    std::vector<int> noten;
    for (int n = s.noteMin; n <= s.noteMax; n += s.schritt) noten.push_back(n);
    if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
    const double wind[3] = {s.windMin, mitteVon(s), s.windMax};
    std::vector<Taste> tasten(noten.size());
    std::vector<char> gestimmt(noten.size(), 0);
    parallel(noten.size(), [&](std::size_t i) {
        tasten[i] = tasteAus(s, noten[i]);
        gestimmt[i] = stimme(tasten[i], wind[1]) ? 1 : 0;
    });
    const std::size_t nd = kDruecke.size();
    std::vector<Mass> raster(noten.size() * nd), drei(noten.size() * 3);
    parallel(raster.size(), [&](std::size_t i) { raster[i] = messe(tasten[i / nd], kDruecke[i % nd]); });
    parallel(drei.size(), [&](std::size_t i) { drei[i] = messe(tasten[i / 3], wind[i % 3]); });

    std::printf("\n## %s — Wind %.0f / %.0f / %.0f Pa, Noten %d–%d\n", s.name.c_str(), wind[0], wind[1], wind[2], s.noteMin, s.noteMax);
    std::printf("note\tL mm\tB mm\tm mg\truhe mm\tf0/f ct\tp_zu Pa\tspricht von\tbis Pa\t");
    for (const char* k : {"min", "mitte", "max"}) std::printf("| %s: ct\tLUFS\tein ms\tanst ms\tA/L %%\tA/ruhe\t", k);
    std::printf("\n");
    double lo[3] = {1e9, 1e9, 1e9}, hi[3] = {-1e9, -1e9, -1e9}, centMax[3] = {0, 0, 0}, einLo = 1e9, einHi = 0, dynLo = 1e9, dynHi = -1e9;
    int ok[3] = {0, 0, 0}, alleDrei = 0;
    double steigLo = 1e9, steigHi = -1e9;
    for (std::size_t k = 0; k < noten.size(); ++k)
    {
        const Taste& z = tasten[k];
        double von = -1.0, bis = -1.0;
        for (std::size_t j = 0; j < nd; ++j)
            if (raster[k * nd + j].spricht)
            {
                if (von < 0.0) von = kDruecke[j];
                bis = kDruecke[j];
            }
        std::printf("%d\t%.1f\t%.2f\t%.1f\t%.3f\t%+.1f\t%.0f\t%.0f\t%.0f\t", z.note, 1e3 * z.laenge, 1e3 * z.breite, 1e6 * z.masse, 1e3 * z.ruhe,
                    1200.0 * std::log2(z.f0 / z.hz), schliessdruck(z), von, bis);
        bool alle = gestimmt[k] != 0;
        for (int j = 0; j < 3; ++j)
        {
            const Mass& m = drei[k * 3 + static_cast<std::size_t>(j)];
            const double cent = m.hz > 0.0 ? 1200.0 * std::log2(m.hz / z.hz) : 0.0;
            std::printf("| %s%+.1f\t%.1f\t%.0f\t%.0f\t%.1f\t%.1f\t", m.spricht ? "" : (m.waechst ? "~" : "x"), cent, m.lufs, 1e3 * m.einS,
                        1e3 * m.anstiegS, 100.0 * m.ampLaenge, m.ampRuhe);
            if (m.spricht)
            {
                ++ok[j];
                lo[j] = std::min(lo[j], m.lufs);
                hi[j] = std::max(hi[j], m.lufs);
                centMax[j] = std::max(centMax[j], std::abs(cent));
            }
            else
                alle = false;
        }
        if (alle)
        {
            ++alleDrei;
            const double dyn = drei[k * 3 + 2].lufs - drei[k * 3].lufs;
            dynLo = std::min(dynLo, dyn);
            dynHi = std::max(dynHi, dyn);
            einLo = std::min(einLo, drei[k * 3 + 1].einS);
            einHi = std::max(einHi, drei[k * 3 + 1].einS);
            const double steig = 1200.0 * std::log2(drei[k * 3 + 2].hz / drei[k * 3].hz) / (1e-3 * (wind[2] - wind[0]));
            steigLo = std::min(steigLo, steig);
            steigHi = std::max(steigHi, steig);
        }
        std::printf("\n");
    }
    std::printf("ZIEL %s: spricht min/mitte/max %d/%d/%d von %zu, an allen dreien %d | Lautheit ueber den Umfang %.1f / %.1f / %.1f LU | "
                "groesste Abweichung %.1f / %.1f / %.1f ct | Dynamik %.1f … %.1f LU | Einschwingen Mitte %.0f … %.0f ms | "
                "Tonhoehe ueber Druck %.1f … %.1f ct/kPa\n",
                s.name.c_str(), ok[0], ok[1], ok[2], noten.size(), alleDrei, hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2], centMax[0], centMax[1],
                centMax[2], dynLo, dynHi, 1e3 * einLo, 1e3 * einHi, steigLo, steigHi);
}

/// Bend ueber das Ventil (Adachi: die halb gedrueckte Taste): gestimmt mit offenem Ventil am mittleren Druck, dann
/// die Ventilflaeche verkleinert — Tonhoehe gegen die Note und Pegel gegen das offene Ventil.
void bend(const Satz& s)
{
    const std::vector<double> faktor = {1.0, 0.7, 0.5, 0.35, 0.25, 0.18, 0.12};
    const double wind[2] = {mitteVon(s), s.windMax};
    std::printf("\n## Bend %s — Ventilflaeche mal Faktor; je Feld Cent / dB gegen offen (x = spricht nicht)\n", s.name.c_str());
    for (int j = 0; j < 2; ++j)
    {
        std::printf("Druck %.0f Pa\nnote", wind[j]);
        for (const double f : faktor) std::printf("\t| %.2f", f);
        std::printf("\n");
        for (int note = s.noteMin; note <= s.noteMax; note += 8)
        {
            Taste z = tasteAus(s, note);
            if (!stimme(z, wind[0]))
            {
                std::printf("%d\tnicht gestimmt\n", note);
                continue;
            }
            std::vector<Mass> m(faktor.size());
            parallel(faktor.size(), [&](std::size_t i) {
                Taste y = z;
                y.ventil *= faktor[i];
                m[i] = messe(y, wind[j]);
            });
            std::printf("%d", note);
            for (const Mass& x : m)
                std::printf("\t| %s%+.0f / %+.1f", x.spricht ? "" : "x", x.hz > 0.0 ? 1200.0 * std::log2(x.hz / z.hz) : 0.0, x.lufs - m[0].lufs);
            std::printf("\n");
        }
    }
}

/// Die Vorlage: Groessen wie `baueVorgabeAkkordeon` (Diskant), ein Wind fuer alle Tasten.
Satz vorlageAkkordeon()
{
    Satz s;
    s.name = "Vorlage Akkordeon Diskant";
    s.wieKern = true;
    s.laenge = {32.8e-3, -0.5};
    s.breite = {3.45e-3, 0.0};
    s.flaechenmasse = {(2.0 / 3.0 * 0.248e-3) / (0.5 * 32.8e-3 * 3.45e-3), 0.0};
    s.ruhe = {0.3e-3, 0.0};
    s.spalt = {0.14e-3, 0.0};
    s.kanalLaenge = {30e-3, 0.0};
    s.kanalFlaeche = {3.45e-3 * 2e-3, 0.0};
    return s;
}

/// Gegenprobe des Modells nach Adachi an seinen eigenen Zahlen (Tab. 1, Abb. 6 und 7, S. 4–5): 1 kPa, K_p = 35 →
/// 265,1 Hz, Amplitude rund 1,6 mm, U bis rund 750 cm³/s, p_out rund ±60 Pa; K_p = 150 → 261,7 Hz.
void pruefeAdachi()
{
    for (const double anstieg : {0.0, 0.02})
    {
    std::printf("\n## Gegenprobe Adachi u. a. 2026, Zunge C4 (Tab. 1), K_r = 8, 3 s, Druckanstieg %.0f ms\nK_p\tp0 Pa\tHz\tAmplitude mm\tMitte mm\tU max cm3/s\tp_out Spitze Pa\t| 16-fach: Hz\tAmplitude mm\n", 1e3 * anstieg);
    for (const double kp : {35.0, 150.0})
        for (const double p0 : {200.0, 600.0, 1000.0, 1600.0, 2500.0})
        {
            Taste z;
            z.adachi = true;
            z.hz = z.f0 = 267.6;
            z.laenge = 32.8e-3;
            z.breite = 3.45e-3;
            z.masse = 2.0 / 3.0 * 0.248e-3;
            z.zeta = 0.003;
            z.spalt = 0.14e-3;
            z.ruhe = 0.3e-3;
            z.kp = kp;
            z.rho = 1.17;
            double erg[2][2] = {{0, 0}, {0, 0}}, uMax = 0.0, ausMax = 0.0, mitte = 0.0;
            int k = 0;
            for (const int ueber : {kUeber, 16})
            {
                std::vector<double> aus, y;
                double um = 0.0;
                rechneAdachi(z, p0, aus, &y, &um, ueber, anstieg, 3.0, 3 * kRate);
                const std::size_t ende = 3 * kRate, von = kRate * 11 / 4;
                double lo = 1e9, hi = -1e9, erster = -1, letzter = -1, am = 0.0;
                for (std::size_t i = von; i < ende; ++i)
                {
                    lo = std::min(lo, y[i]);
                    hi = std::max(hi, y[i]);
                    am = std::max(am, std::abs(aus[i]));
                }
                const double m = 0.5 * (lo + hi);
                int zahl = 0;
                for (std::size_t i = von + 1; i < ende; ++i)
                {
                    const double a = y[i - 1] - m, b = y[i] - m;
                    if (a < 0.0 && b >= 0.0)
                    {
                        const double t = (static_cast<double>(i - 1) + a / (a - b)) / kRate;
                        if (zahl == 0) erster = t;
                        letzter = t;
                        ++zahl;
                    }
                }
                erg[k][0] = zahl >= 3 ? (zahl - 1) / (letzter - erster) : 0.0;
                erg[k][1] = 0.5 * (hi - lo);
                if (k == 0)
                {
                    uMax = um;
                    ausMax = am;
                    mitte = m;
                }
                ++k;
            }
            std::printf("%.0f\t%.0f\t%.2f\t%.2f\t%+.2f\t%.0f\t%.0f\t| %.2f\t%.2f\n", kp, p0, erg[0][0], 1e3 * erg[0][1], 1e3 * mitte, 1e6 * uMax, ausMax,
                        erg[1][0], 1e3 * erg[1][1]);
        }
    }
}

/// Das Modell nach Adachi mit seiner C4-Zunge (Tab. 1); die Gesetze ueber den Umfang setzt der Aufrufer.
Satz adachiC4(const std::string& name)
{
    Satz s;
    s.name = name;
    s.adachi = true;
    s.laenge = {32.8e-3, -0.5};
    s.breite = {3.45e-3, 0.0};
    s.flaechenmasse = {(2.0 / 3.0 * 0.248e-3) / (32.8e-3 * 3.45e-3), 0.0};
    s.ruhe = {0.3e-3, 0.0};
    s.spalt = {0.14e-3, 0.0};
    s.kanalLaenge = {30e-3, 0.0};
    s.kanalFlaeche = {3.45e-3 * 2e-3, 0.0};
    s.windMin = 200.0;
    s.windMax = 1600.0;
    return s;
}

/// Einzelne Tasten ohne Stimmen: Tonhoehe gegen die Eigenfrequenz, Amplitude, Einschwingen ueber dem Druck.
void erkunde(const std::vector<Satz>& saetze)
{
    const std::vector<double> druck = {200, 400, 566, 1000, 1600, 2500};
    for (const Satz& s : saetze)
    {
        std::printf("\n## %s\nnote\tp_zu\t", s.name.c_str());
        for (const double p : druck) std::printf("| %.0f Pa: ct\tmm\tein\tanst\tLUFS\t", p);
        std::printf("| Hz/kPa 200→1000\n");
        for (const int note : {48, 60, 69, 81, 93})
        {
            Taste z = tasteAus(s, note);
            z.f0 = z.hz;
            std::vector<Mass> m(druck.size());
            parallel(druck.size(), [&](std::size_t i) { m[i] = messe(z, druck[i]); });
            std::printf("%d\t%.0f\t", note, schliessdruck(z));
            for (const Mass& x : m)
                std::printf("| %s%+.1f\t%.2f\t%.0f\t%.0f\t%.1f\t", x.spricht ? "" : (x.waechst ? "~" : "x"),
                            x.hz > 0.0 ? 1200.0 * std::log2(x.hz / z.hz) : 0.0, 1e3 * x.ampLaenge * z.laenge, 1e3 * x.einS, 1e3 * x.anstiegS, x.lufs);
            std::printf("| %.2f\n", (m[3].hz - m[0].hz) / 0.8);
        }
    }
}

bool gleichDemKern()
{
    using namespace lumi::kern::zunge;
    const ZungenSpiel k = baueVorgabeAkkordeon();
    const Satz s = vorlageAkkordeon();
    bool gleich = true;
    for (const int note : {53, 69, 93})
        for (const double druck : {300.0, 1200.0})
        {
            Taste z = tasteAus(s, note);
            z.masse = k.masseC4Kg * z.laenge / k.laengeC4M;
            ZungenAnschlag a;
            a.note = note;
            a.druckPa = druck;
            a.haltenS = kHalten;
            std::vector<double> kern, eigen;
            rechneEineZunge(k, z.f0, z.hz, a, kern, kFrames, nullptr);
            rechne(z, druck, eigen, nullptr);
            if (kern.size() != eigen.size() || std::memcmp(kern.data(), eigen.data(), kern.size() * sizeof(double)) != 0) gleich = false;
        }
    return gleich;
}

}  // namespace

int main(int argc, char** argv)
{
    std::printf("Pruefstand gleich dem Kern (6 Toene, Bit fuer Bit): %s\n", gleichDemKern() ? "ja" : "NEIN");
    if (argc >= 2 && std::string(argv[1]) == "adachi")
    {
        pruefeAdachi();
        return 0;
    }
    if (argc >= 4 && (std::string(argv[1]) == "satz" || std::string(argv[1]) == "bend"))
    {
        // satz <kern|adachi> <name> schluessel=c4[,hoch] …  (Gesetz: Wert an C4 mal (f/f_C4)^hoch)
        Satz s = std::string(argv[2]) == "adachi" ? adachiC4(argv[3]) : vorlageAkkordeon();
        s.name = argv[3];
        s.wieKern = false;
        for (int i = 4; i < argc; ++i)
        {
            const std::string a = argv[i];
            const std::size_t gl = a.find('=');
            if (gl == std::string::npos) continue;
            const std::string k = a.substr(0, gl), w = a.substr(gl + 1);
            const std::size_t ko = w.find(',');
            const double c4 = std::stod(w.substr(0, ko)), hoch = ko == std::string::npos ? 0.0 : std::stod(w.substr(ko + 1));
            if (k == "L") s.laenge = {c4, hoch};
            else if (k == "W") s.breite = {c4, hoch};
            else if (k == "mu") s.flaechenmasse = {c4, hoch};
            else if (k == "ruhe") s.ruhe = {c4, hoch};
            else if (k == "spalt") s.spalt = {c4, hoch};
            else if (k == "kl") s.kanalLaenge = {c4, hoch};
            else if (k == "ks") s.kanalFlaeche = {c4, hoch};
            else if (k == "sc") s.sc = {c4, hoch};
            else if (k == "zeta") s.zeta = {c4, hoch};
            else if (k == "ventil") s.ventil = {c4, hoch};
            else if (k == "kanz") s.kanzelle = {c4, hoch};
            else if (k == "wmin") s.windMin = c4;
            else if (k == "wmax") s.windMax = c4;
            else if (k == "nmin") s.noteMin = static_cast<int>(c4);
            else if (k == "nmax") s.noteMax = static_cast<int>(c4);
            else if (k == "schritt") s.schritt = static_cast<int>(c4);
            else if (k == "kp") s.kp = c4;
            else if (k == "kr") s.kr = c4;
            else if (k == "tf") s.traegFaktor = c4;
            else std::printf("unbekannt: %s\n", k.c_str());
        }
        std::printf("Aufruf:");
        for (int i = 1; i < argc; ++i) std::printf(" %s", argv[i]);
        std::printf("\n");
        if (std::string(argv[1]) == "bend")
            bend(s);
        else
            berichte(s);
        return 0;
    }
    if (argc >= 2 && std::string(argv[1]) == "erkunde")
    {
        std::vector<Satz> e;
        e.push_back(vorlageAkkordeon());
        for (const double v : {3e-5, 1e-5, 5e-6, 2.5e-6})
        {
            Satz s = vorlageAkkordeon();
            s.ventil = {v, 0.0};
            char n[96];
            std::snprintf(n, sizeof n, "Kern-Form, Ventil %.1f mm2", 1e6 * v);
            s.name = n;
            e.push_back(s);
        }
        for (const double f : {0.5, 1.0, 2.0, 4.0})
            for (const double zeta : {0.003, 0.001})
            {
                Satz s = adachiC4("");
                s.traegFaktor = f;
                s.zeta = {zeta, 0.0};
                char n[96];
                std::snprintf(n, sizeof n, "Adachi-Form, Traegheitsfaktor %.1f, zeta %.3f", f, zeta);
                s.name = n;
                e.push_back(s);
            }
        erkunde(e);
        return 0;
    }
    std::vector<Satz> saetze;
    saetze.push_back(vorlageAkkordeon());
    {
        Satz s = vorlageAkkordeon();
        s.name = "A0 Kern-Form, Vorlage, Wind 200-1600";
        s.windMin = 200.0;
        s.windMax = 1600.0;
        saetze.push_back(s);
    }
    saetze.push_back(adachiC4("B0 Adachi-Form, gestuft wie die Vorlage (L ~ f^-0.5, sonst fest)"));
    for (const Satz& s : saetze)
        if (argc < 2 || s.name.find(argv[1]) != std::string::npos) berichte(s);
    return 0;
}

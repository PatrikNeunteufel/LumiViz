/**
 * @file Orgel.cpp
 * @brief Klasse 11: die Tonradorgel ueber einen Clip (Herleitung im Kopf von `Orgel.hpp`)
 *
 * **Perkussion** (Bericht §5, nur Bericht): nur Obermanual, 2. (4′) oder 3. (2⅔′) Harmonische, Einzeltrigger (neu erst,
 * wenn alle Tasten los sind), fast ≈ 250 ms, slow ≈ 900 ms (Bonedo — als Abklingen auf −60 dB gelesen, offen), soft
 * −6 dB; mit Perkussion ist der 1′ stumm. **Scanner** (§6): 412 U/min = 6,87 Hz, die Leitung ≈ 0,85 ms (Werner et al.,
 * nachgelesen), V1/V2/V3 ein Drittel/die Haelfte/die ganze Leitung, Chorus = Mischung mit dem trockenen Signal; als
 * veraenderliche Verzoegerung mit Dreieck (V3: ±0,75 % ≈ 26 Cent Spitze–Spitze). **Schweller** (F-3 a): −36 dB … 0 dB,
 * dazu eine milde Saettigung des Vorverstaerkers (offen). **Rotary** (Bericht Leslie): Weiche 800 Hz 2. Ordnung (12 dB/
 * Oktave), Horn mit Doppler (Radius) und Richtwirkung, Trommel vor allem als Amplitudenmodulation („an AM ‚throb‘ is the
 * main effect“, Smith et al., nachgelesen), Drehzahl erster Ordnung mit getrennten Zeitkonstanten (Herrera et al. 2009 —
 * Werte offen).
 */
#include "klassen/11_tonrad/Orgel.hpp"

#include "gemeinsam/Werkzeug.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>

namespace ic::tonrad
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr int kRaeder = 91;
// Uebersetzungen (treibend/getrieben) C … B (Bericht §1, nur Bericht)
constexpr double kUebersetzung[12] = {85.0 / 104.0, 71.0 / 82.0, 67.0 / 73.0, 105.0 / 108.0, 103.0 / 100.0, 84.0 / 77.0,
                                      74.0 / 64.0,  98.0 / 80.0, 96.0 / 74.0, 88.0 / 64.0,   67.0 / 46.0,   108.0 / 70.0};
constexpr int kVersatz[9] = {-12, 7, 0, 12, 19, 24, 28, 31, 36};

/// Pegel je Rad nach Tabelle V (Spannungen, nur Bericht), in dB linear zwischen den Stuetzpunkten.
double radPegel(int rad)
{
    static const double punkte[][2] = {{1, 9.8}, {9, 14.0}, {20, 16.0}, {43, 9.0}, {44, 2.5}, {80, 0.7}, {91, 0.2}};
    for (int i = 0; i + 1 < 7; ++i)
        if (rad <= punkte[i + 1][0])
        {
            const double t = (rad - punkte[i][0]) / (punkte[i + 1][0] - punkte[i][0]);
            const double db = (1.0 - t) * 20.0 * std::log10(punkte[i][1]) + t * 20.0 * std::log10(punkte[i + 1][1]);
            return std::pow(10.0, db / 20.0) / 16.0;
        }
    return 0.2 / 16.0;
}

/// Das Rad auf derselben Welle (Uebersprechen im Radfach): 2/32, 4/64, 8/128, 16/192 Zaehne.
int partner(int rad)
{
    const int n = rad - 1, o = n / 12, pc = n % 12;
    if (o <= 2) return rad + 48;
    if (o == 3) return pc >= 5 ? 85 + (pc - 5) : 0;
    if (o >= 4 && o <= 6) return rad - 48;
    return 37 + (pc + 5);  // 192 Zaehne → 16 Zaehne derselben Welle
}

struct Zufall
{
    std::uint64_t s;
    double naechste()
    {
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        return static_cast<double>((s * 0x2545F4914F6CDD1Dull) >> 11) / 9007199254740992.0;
    }
};

/// Ein Biquad (RBJ Butterworth, Q = 1/√2).
struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    void setze(double f, double fs, bool hoch)
    {
        const double w = 2.0 * kPi * f / fs, al = std::sin(w) / std::sqrt(2.0), cw = std::cos(w), a0 = 1.0 + al;
        if (hoch)
        {
            b0 = (1.0 + cw) / 2.0 / a0;
            b1 = -(1.0 + cw) / a0;
        }
        else
        {
            b0 = (1.0 - cw) / 2.0 / a0;
            b1 = (1.0 - cw) / a0;
        }
        b2 = b0;
        a1 = -2.0 * cw / a0;
        a2 = (1.0 - al) / a0;
    }
    double schritt(double x)
    {
        const double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }
};

/// Eine Verzoegerung mit linearer Interpolation.
struct Verzoegerung
{
    std::vector<double> x = std::vector<double>(4096, 0.0);
    std::size_t pos = 0;
    void schreibe(double v)
    {
        x[pos] = v;
        pos = (pos + 1) % x.size();
    }
    [[nodiscard]] double lies(double d) const
    {
        d = std::clamp(d, 1.0, static_cast<double>(x.size() - 3));
        const auto i = static_cast<std::size_t>(std::floor(d));
        const double f = d - static_cast<double>(i);
        const std::size_t n = x.size();
        return (1.0 - f) * x[(pos + n - i) % n] + f * x[(pos + n - i - 1) % n];
    }
};

double pegelRiegel(int stufe)
{
    return stufe <= 0 ? 0.0 : std::pow(10.0, -3.0 * (8 - std::clamp(stufe, 0, 8)) / 20.0);
}

}  // namespace

double radHz(int rad)
{
    const int n = std::clamp(rad, 1, kRaeder) - 1, o = n / 12, pc = n % 12;
    const double zaehne = o < 7 ? 2.0 * std::exp2(o) : 192.0;
    const double ueb = o < 7 ? kUebersetzung[pc] : kUebersetzung[(pc + 5) % 12];
    return 20.0 * zaehne * ueb;
}

int radFuer(int taste, int riegel)
{
    int w = 12 + taste + kVersatz[std::clamp(riegel, 0, 8)];
    while (w < 13) w += 12;
    while (w > kRaeder) w -= 12;
    return w;
}

void rechneOrgel(const OrgelSpiel& s, const std::vector<OrgelNote>& noten, const OrgelSpur& spur, std::vector<float>& mono, std::int64_t frames,
                 int rate, OrgelMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || rate <= 0) return;
    const double fs = rate, dt = 1.0 / fs;
    // die Raeder: Zeiger mit Phase aus der Zeit (alle 4096 Takte neu verankert)
    std::vector<std::complex<double>> zeiger(kRaeder + 1), schritt(kRaeder + 1);
    std::vector<double> rad(kRaeder + 1, 0.0), pegel(kRaeder + 1, 0.0);
    for (int w = 1; w <= kRaeder; ++w)
    {
        schritt[static_cast<std::size_t>(w)] = std::polar(1.0, 2.0 * kPi * radHz(w) * dt);
        pegel[static_cast<std::size_t>(w)] = radPegel(w);
    }
    // die Kontakte je Note: neun Schliesszeiten mit Prellen (deterministisch je Note)
    struct Kontakt
    {
        std::vector<std::pair<double, bool>> wechsel;  ///< (Zeit, geschlossen)
    };
    std::vector<std::array<Kontakt, 9>> kontakte(noten.size());
    for (std::size_t i = 0; i < noten.size(); ++i)
    {
        Zufall z{0x9E3779B97F4A7C15ull ^ (static_cast<std::uint64_t>(noten[i].midi) * 0x100000001B3ull) ^ static_cast<std::uint64_t>(noten[i].abS * 1e6)};
        const double weg = 1.5e-3 / std::max(0.05, noten[i].tempo);  // 1,5 mm Versatz bei der Tastengeschwindigkeit
        for (int b = 0; b < 9; ++b)
        {
            auto& w = kontakte[i][static_cast<std::size_t>(b)].wechsel;
            double t = noten[i].abS + weg * z.naechste();
            w.push_back({t, true});
            const int prellen = 1 + static_cast<int>(3.0 * z.naechste());
            for (int p = 0; p < prellen; ++p)
            {
                t += 1e-3 * s.klickMs * (0.05 + 0.25 * z.naechste());
                w.push_back({t, false});
                t += 1e-3 * s.klickMs * (0.05 + 0.25 * z.naechste());
                w.push_back({t, true});
            }
            const double los = noten[i].bisS + 0.5e-3 * z.naechste();
            w.push_back({los, false});
        }
    }
    // Registrierung (Spur, Uebergang 50 ms)
    Registrierung reg = s.register_;
    std::array<double, 9> riegel{}, riegelZiel{};
    for (int b = 0; b < 9; ++b) riegel[static_cast<std::size_t>(b)] = riegelZiel[static_cast<std::size_t>(b)] = pegelRiegel(reg[static_cast<std::size_t>(b)]);
    std::size_t regIdx = 0;
    if (s.perkussion) riegel[8] = riegelZiel[8] = 0.0;  // 1′ stumm
    // Perkussion: Einzeltrigger
    const int percRiegel = s.perkussionDritte ? 4 : 3;
    const double percT60 = s.perkussionSchnell ? 0.25 : 0.9;
    const double percAbkl = std::exp(-std::log(1000.0) / (percT60 * fs));
    double percHuelle = 0.0;
    bool hielt = false;
    // Scanner
    const double tiefe = (s.scanner == Scanner::V1 || s.scanner == Scanner::C1) ? 1.0 / 3.0
                         : (s.scanner == Scanner::V2 || s.scanner == Scanner::C2) ? 0.5
                                                                                  : 1.0;
    const bool chorus = s.scanner == Scanner::C1 || s.scanner == Scanner::C2 || s.scanner == Scanner::C3;
    Verzoegerung scanLeitung;
    double scanTp = 0.0;
    const double scanA = 1.0 - std::exp(-2.0 * kPi * 7075.0 / fs);
    // Rotary
    Biquad weicheTief, weicheHoch;
    weicheTief.setze(800.0, fs, false);
    weicheHoch.setze(800.0, fs, true);
    Verzoegerung hornLeitung, trommelLeitung;
    bool schnell = spur.rotaryStartSchnell;
    std::size_t rotIdx = 0;
    double hornW = 2.0 * kPi * (schnell ? s.hornSchnellUpm : s.hornLangsamUpm) / 60.0;
    double trommelW = 2.0 * kPi * (schnell ? s.trommelSchnellUpm : s.trommelLangsamUpm) / 60.0;
    double hornWinkel = 0.0, trommelWinkel = kPi / 3.0;
    const double schweller = std::pow(10.0, -36.0 * (1.0 - std::clamp(spur.schweller, 0.0, 1.0)) / 20.0);
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::vector<std::size_t> kontaktPos(noten.size() * 9, 0);
    std::vector<std::array<bool, 9>> zu(noten.size());
    for (auto& a : zu) a.fill(false);
    for (std::int64_t n = 0; n < frames; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        if (n % 4096 == 0)
            for (int w = 1; w <= kRaeder; ++w) zeiger[static_cast<std::size_t>(w)] = std::polar(1.0, 2.0 * kPi * std::fmod(radHz(w) * t, 1.0));
        // die Raeder jetzt (tiefe zwoelf rechteckaehnlich: 1, ⅓, ⅕)
        for (int w = 1; w <= kRaeder; ++w)
        {
            const std::complex<double> z = zeiger[static_cast<std::size_t>(w)];
            double v = z.imag();
            if (w <= 12)
            {
                const std::complex<double> z3 = z * z * z;
                v += z3.imag() / 3.0 + (z3 * z * z).imag() / 5.0;
            }
            rad[static_cast<std::size_t>(w)] = v * pegel[static_cast<std::size_t>(w)];
            zeiger[static_cast<std::size_t>(w)] = z * schritt[static_cast<std::size_t>(w)];
        }
        // Registrierungswechsel
        while (regIdx < spur.register_.size() && t >= spur.register_[regIdx].first)
        {
            reg = spur.register_[regIdx].second;
            for (int b = 0; b < 9; ++b) riegelZiel[static_cast<std::size_t>(b)] = pegelRiegel(reg[static_cast<std::size_t>(b)]);
            if (s.perkussion) riegelZiel[8] = 0.0;
            ++regIdx;
        }
        for (int b = 0; b < 9; ++b)
            riegel[static_cast<std::size_t>(b)] += std::clamp(riegelZiel[static_cast<std::size_t>(b)] - riegel[static_cast<std::size_t>(b)], -dt / 0.05, dt / 0.05);
        // die Tasten
        double y = 0.0, perc = 0.0;
        bool haelt = false, neu = false;
        for (std::size_t i = 0; i < noten.size(); ++i)
        {
            if (t < noten[i].abS - 1e-9 || t > noten[i].bisS + 0.02) continue;
            const int taste = std::clamp(noten[i].midi - 35, 1, 61);
            for (int b = 0; b < 9; ++b)
            {
                std::size_t& p = kontaktPos[i * 9 + static_cast<std::size_t>(b)];
                const auto& w = kontakte[i][static_cast<std::size_t>(b)].wechsel;
                while (p < w.size() && t >= w[p].first) zu[i][static_cast<std::size_t>(b)] = w[p++].second;
                if (!zu[i][static_cast<std::size_t>(b)]) continue;
                const int r = radFuer(taste, b);
                const int pr = partner(r);
                const double welle = rad[static_cast<std::size_t>(r)] + (pr > 0 ? s.uebersprechen * rad[static_cast<std::size_t>(pr)] : 0.0);
                y += riegel[static_cast<std::size_t>(b)] * welle;
                if (b == percRiegel && s.perkussion) perc += welle;
            }
            if (t >= noten[i].abS && t < noten[i].bisS)
            {
                haelt = true;
                if (t < noten[i].abS + dt) neu = true;
            }
        }
        if (s.perkussion)
        {
            if (neu && !hielt)
            {
                percHuelle = 1.0;
                if (messung != nullptr) messung->perkussionStarts += 1.0;
            }
            hielt = haelt;
            y += (s.perkussionWeich ? 0.5 : 1.0) * percHuelle * perc;
            percHuelle *= percAbkl;
        }
        // Scanner: veraenderliche Verzoegerung (Dreieck 6,87 Hz), die Leitung tiefpass bei 7 kHz
        if (s.scanner != Scanner::Aus)
        {
            scanLeitung.schreibe(y);
            const double phase = std::fmod(6.87 * t, 1.0);
            const double tri = phase < 0.5 ? 2.0 * phase : 2.0 - 2.0 * phase;
            const double d = 1.0 + tiefe * 0.85e-3 * fs * tri;
            scanTp += scanA * (scanLeitung.lies(d) - scanTp);
            y = chorus ? 0.5 * (y + scanTp) : scanTp;
        }
        // Schweller und Vorverstaerker (milde Saettigung, offen)
        y = s.saettigung > 0.0 ? std::tanh(s.saettigung * schweller * y) / s.saettigung : schweller * y;
        // Rotary
        if (s.rotary)
        {
            while (rotIdx < spur.rotarySchnell.size() && t >= spur.rotarySchnell[rotIdx].first) schnell = spur.rotarySchnell[rotIdx++].second;
            const double hZiel = 2.0 * kPi * (schnell ? s.hornSchnellUpm : s.hornLangsamUpm) / 60.0;
            const double tZiel = 2.0 * kPi * (schnell ? s.trommelSchnellUpm : s.trommelLangsamUpm) / 60.0;
            hornW += (hZiel - hornW) * dt / (hZiel > hornW ? s.hornHochS : s.hornAusS);
            trommelW += (tZiel - trommelW) * dt / (tZiel > trommelW ? s.trommelHochS : s.trommelAusS);
            hornWinkel = std::fmod(hornWinkel + hornW * dt, 2.0 * kPi);
            trommelWinkel = std::fmod(trommelWinkel + trommelW * dt, 2.0 * kPi);
            const double hoch = weicheHoch.schritt(y), tief = weicheTief.schritt(y);
            hornLeitung.schreibe(hoch);
            trommelLeitung.schreibe(tief);
            // Horn: Doppler (Abstand zum Hoerer ∝ −R·cos θ), Richtwirkung (offen)
            const double dh = 2.0 + s.hornRadiusM * (1.0 - std::cos(hornWinkel)) / 343.0 * fs;
            const double horn = hornLeitung.lies(dh) * (0.55 + 0.45 * std::cos(hornWinkel));
            const double dtr = 2.0 + 0.1 * (1.0 - std::cos(trommelWinkel)) / 343.0 * fs;
            const double trommel = trommelLeitung.lies(dtr) * (0.75 + 0.25 * std::cos(trommelWinkel));
            y = horn + trommel;
            if (messung != nullptr && n % static_cast<std::int64_t>(0.01 * fs) == 0) messung->hornUpm.push_back(hornW * 60.0 / (2.0 * kPi));
        }
        summe[static_cast<std::size_t>(n)] = y;
    }
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(0.005 * fs)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) w *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(w);
    }
}

namespace
{

void normiere(OrgelSpiel& s)
{
    s.ausgangBezug = 1.0;
    std::vector<OrgelNote> akkord = {{0.0, 0.5, 60, 0.7}, {0.0, 0.5, 64, 0.7}, {0.0, 0.5, 67, 0.7}};
    OrgelSpur spur;
    spur.schweller = 1.0;
    std::vector<float> ton;
    rechneOrgel(s, akkord, spur, ton, 24000);
    const double sp = spitze(ton);
    s.ausgangBezug = sp > 0.0 ? sp / 0.5 : 1.0;
}

}  // namespace

OrgelSpiel baueVorgabeOrgel()
{
    OrgelSpiel s;
    s.name = "Tonewheel Organ";
    s.register_ = {8, 8, 8, 0, 0, 0, 0, 0, 0};  // eine Standard-Registrierung (offen, F-5 a)
    s.scanner = Scanner::Aus;
    s.rotary = true;
    s.kennzeichen = "ungemessen (Iowa/Freesound-Aufnahmen lokal)";
    normiere(s);
    return s;
}

OrgelSpiel baueVorgabeHausorgel()
{
    OrgelSpiel s;
    s.name = "Tonewheel Organ (house)";
    s.register_ = {0, 0, 8, 8, 0, 0, 0, 0, 0};  // kurz (offen)
    s.perkussion = true;
    s.perkussionDritte = true;
    s.perkussionSchnell = true;
    s.scanner = Scanner::C3;
    s.kennzeichen = "ungemessen (Naeherung des abklingenden Stosses — Hoerprobe entscheidet, F-5 a)";
    normiere(s);
    return s;
}

}  // namespace ic::tonrad

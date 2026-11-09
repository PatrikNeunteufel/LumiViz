/**
 * @file Zupf.cpp
 * @brief Klasse 2: eine gezupfte Saite in zwei Polarisationen an einer 2×2-Admittanz (Herleitung im Kopf von `Zupf.hpp`)
 */
#include "klassen/02_zupf_saite/Zupf.hpp"

#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <numbers>

namespace ic::zupf
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr std::size_t kRing = 1u << 13;

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

struct Ring
{
    std::vector<double> x = std::vector<double>(kRing, 0.0);
    std::size_t pos = 0;
    [[nodiscard]] double lies(double d) const
    {
        d = std::clamp(d, 2.0, static_cast<double>(kRing - 4));
        const auto i = static_cast<std::size_t>(std::floor(d));
        const double t = d - static_cast<double>(i) + 1.0;
        const auto at = [&](std::size_t k) { return x[(pos + kRing - k) % kRing]; };
        return -(t - 1.0) * (t - 2.0) * (t - 3.0) / 6.0 * at(i - 1) + t * (t - 2.0) * (t - 3.0) / 2.0 * at(i) - t * (t - 1.0) * (t - 3.0) / 2.0 * at(i + 1) +
               t * (t - 1.0) * (t - 2.0) / 6.0 * at(i + 2);
    }
    void schreibe(double v)
    {
        x[pos] = v;
        pos = (pos + 1) % kRing;
    }
};

/// Verlust je Umlauf (konstante Guete): Teilton k verliert e^(−πk/Q) je Periode; Gleichanteil ≤ 1.
struct Verlust
{
    double a = 0.0, b = 1.0, y1 = 0.0, verzoegerung = 0.0;
    void setze(double q, double f, double fs)
    {
        const double g1 = std::exp(-kPi / q), g8 = std::exp(-8.0 * kPi / q);
        const double w1 = 2.0 * kPi * f / fs, w8 = std::min(kPi, 8.0 * w1);
        const auto betrag = [](double aa, double w) { return std::abs(1.0 - aa * std::polar(1.0, -w)); };
        double lo = 0.0, hi = 0.999;
        for (int i = 0; i < 50; ++i)
        {
            const double m = 0.5 * (lo + hi);
            if (betrag(m, w1) / betrag(m, w8) > g8 / g1)
                lo = m;
            else
                hi = m;
        }
        a = 0.5 * (lo + hi);
        b = std::min(1.0, g1 * betrag(a, w1) / (1.0 - a)) * (1.0 - a);
        verzoegerung = -std::arg(std::complex<double>(b, 0.0) / (1.0 - a * std::polar(1.0, -w1))) / w1;
    }
    double schritt(double x) { return y1 = b * x + a * y1; }
};

/// Eine Mode des Stegs in Impulsinvarianz (wie `RohrKern`): Eingang Kraft, Ausgang Schnelle.
struct Modus
{
    std::complex<double> pol, gewicht, zustand{0.0, 0.0};
    double durchgriff = 0.0;
    double dy = 1.0, dz = 0.0;
};

}  // namespace

void rechneZupfTon(const ZupfSpiel& s, const ZupfAnschlag& a, std::vector<float>& mono, std::int64_t frames, ZupfMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || !s.spielbar || s.laengeJeNote.punkte.empty()) return;
    const double fs = a.rate, dt = 1.0 / fs;
    const int n0 = std::clamp(a.note, s.noteMin, s.noteMax);
    const double e = std::clamp(a.staerke, 0.0, 1.0);
    // der Steg: Moden mit Richtung
    std::vector<Modus> moden;
    std::array<double, 4> mDurch{0.0, 0.0, 0.0, 0.0};  // M = Σ D d dᵀ (yy, yz, zy, zz)
    for (const StegMode& sm : s.steg)
    {
        Modus m;
        const double wn = 2.0 * kPi * sm.hz, beta = wn / sm.q;
        const double wd = std::sqrt(std::max(1e-12, wn * wn - 0.25 * beta * beta));
        const std::complex<double> lambda(-0.5 * beta, wd);
        const double c = sm.spitze * beta;
        m.pol = std::exp(lambda * dt);
        m.gewicht = c * lambda / std::complex<double>(0.0, 2.0 * wd) * dt;
        m.durchgriff = 0.5 * c * dt;  // ½·C·Δt (wie `durchgriffAus`)
        const double w = sm.winkelGrad * kPi / 180.0;
        m.dy = std::cos(w);
        m.dz = std::sin(w);
        mDurch[0] += m.durchgriff * m.dy * m.dy;
        mDurch[1] += m.durchgriff * m.dy * m.dz;
        mDurch[2] += m.durchgriff * m.dz * m.dy;
        mDurch[3] += m.durchgriff * m.dz * m.dz;
        moden.push_back(m);
    }
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    const int saiten = std::max(1, s.chor.saiten);
    for (int sa = 0; sa < saiten; ++sa)
    {
        const double cent = saiten > 1 ? s.chor.verstimmungCent * (sa == 0 ? -0.5 : 0.5) : 0.0;
        const double f = hzVon(a.note) * std::exp2(cent / 1200.0);
        const double laenge = wertAn(s.laengeJeNote, n0, Skala::Logarithmisch) * hzVon(n0) / f;
        const double zug = wertAn(s.zugJeNote, n0, Skala::Logarithmisch);
        const double c = 2.0 * laenge * f;  // die Welle laeuft in 1/(2f) einmal hin
        const double z0 = zug / c;
        const double rel = s.zupfpunktJeNote.punkte.empty() ? s.zupfpunktRel : wertAn(s.zupfpunktJeNote, n0, Skala::Linear);
        Verlust verlust[2];
        verlust[0].setze(s.guete * (a.lautenzug ? 0.125 : 1.0), f, fs);
        verlust[1].setze(s.guete * (a.lautenzug ? 0.125 : 1.0) * 0.9, f, fs);  // parallel zur Decke etwas weniger (offen)
        const double k = s.laufzeitJeNote.punkte.empty() ? 1.0 : wertAn(s.laufzeitJeNote, n0, Skala::Logarithmisch);
        const double dB = std::max(2.0, k * rel * laenge / c * fs);
        const double dN = std::max(2.0, k * (1.0 - rel) * laenge / c * fs - verlust[0].verzoegerung);
        Ring zumSattel[2], vomSattel[2], zumSteg[2], vomSteg[2];
        for (Modus& m : moden) m.zustand = {0.0, 0.0};
        const double winkel = (a.polarisationGrad >= 0.0 ? a.polarisationGrad : s.polarisationGrad) * kPi / 180.0;
        const double kraft = s.kraftPpN * std::pow(s.kraftFfN / s.kraftPpN, e);
        const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * fs));
        double daempfung = 0.0;
        double uyVor = 0.0;
        // die Determinante von (I + Z0·M)
        const double a11 = 1.0 + z0 * mDurch[0], a12 = z0 * mDurch[1], a21 = z0 * mDurch[2], a22 = 1.0 + z0 * mDurch[3];
        const double det = a11 * a22 - a12 * a21;
        for (std::int64_t n = 0; n < frames; ++n)
        {
            const double t = n * dt;
            // das Notenende: Daempfer (Cembalo) bzw. Hand (Harfe) — ausser laissez vibrer
            if (n >= halten && !a.laissezVibrer) daempfung = std::min(1.0, (t - a.haltenS) / s.daempfenS);
            // der Zupf: ein Kraftstoss (halbe Kosinuswelle) in der Richtung des Winkels
            const double fz = t < s.stossS ? kraft * 0.5 * (1.0 - std::cos(2.0 * kPi * t / s.stossS)) : 0.0;
            std::array<double, 2> vPlus{}, vonSattel{}, vonSteg{};
            for (int p = 0; p < 2; ++p)
            {
                // alle Leitungen vor dem Schreiben gelesen: die Laufzeit je Strecke genau d Takte (las man nach dem
                // Schreiben, war der Umlauf 2 Takte zu kurz — n85 +83 Cent, Befund IC Session 4)
                vonSattel[p] = vomSattel[p].lies(dN);
                vonSteg[p] = vomSteg[p].lies(dB);
                const double amSattel = zumSattel[p].lies(dN);
                vPlus[p] = zumSteg[p].lies(dB);
                const double fp = fz * (p == 0 ? std::cos(winkel) : std::sin(winkel));
                const double ein = fp / (2.0 * z0);
                zumSattel[p].schreibe(vonSteg[p] + ein);
                zumSteg[p].schreibe(vonSattel[p] + ein);
                const double g = 1.0 - 0.995 * daempfung;
                vomSattel[p].schreibe(-g * verlust[p].schritt(amSattel));
            }
            // der Steg: u = (I + Z0·M)⁻¹·(u_vorher + 2·Z0·M·v⁺)
            double uvy = 0.0, uvz = 0.0;
            for (const Modus& m : moden)
            {
                const double r = 2.0 * m.zustand.real();
                uvy += r * m.dy;
                uvz += r * m.dz;
            }
            const double ry = uvy + 2.0 * z0 * (mDurch[0] * vPlus[0] + mDurch[1] * vPlus[1]);
            const double rz = uvz + 2.0 * z0 * (mDurch[2] * vPlus[0] + mDurch[3] * vPlus[1]);
            const double uy = (a22 * ry - a12 * rz) / det, uz = (a11 * rz - a21 * ry) / det;
            const double fy = z0 * (2.0 * vPlus[0] - uy), fzs = z0 * (2.0 * vPlus[1] - uz);
            for (Modus& m : moden)
            {
                const double fm = m.dy * fy + m.dz * fzs;
                // ausgeschrieben (clang: Komplex·Komplex ohne fast-math langsam)
                const double zr = m.zustand.real(), zi = m.zustand.imag();
                const double nr = zr * m.pol.real() - zi * m.pol.imag() + m.gewicht.real() * fm;
                const double ni = zr * m.pol.imag() + zi * m.pol.real() + m.gewicht.imag() * fm;
                m.zustand = {nr, ni};
            }
            vomSteg[0].schreibe(uy - vPlus[0]);
            vomSteg[1].schreibe(uz - vPlus[1]);
            summe[static_cast<std::size_t>(n)] += (uy - uyVor) * fs * 1e-3;
            uyVor = uy;
        }
    }
    // der Stoss der Taste in den Boden (Cembalo, F-4 a): ein Impuls in die Stegmoden, mit der Staerke (offen)
    if (s.tastenstoss > 0.0)
    {
        for (Modus& m : moden) m.zustand = m.gewicht * (s.tastenstoss * std::pow(10.0, (e - 0.5) * 20.0 / 20.0) / dt * 1e-4);
        for (std::int64_t n = 0; n < std::min<std::int64_t>(frames, static_cast<std::int64_t>(0.5 * fs)); ++n)
        {
            double uy = 0.0;
            for (Modus& m : moden)
            {
                uy += 2.0 * m.zustand.real() * m.dy;
                m.zustand *= m.pol;
            }
            if (n > 0) summe[static_cast<std::size_t>(n)] += uy * 1e-3;
        }
    }
    if (messung != nullptr)
    {
        // die staerkste Spitze nahe der Note (YIN sprang bei schwachem Grundton auf die Oktave): Hann-Fenster 0,1 … 0,5 s,
        // Raster 0,5 Cent ueber ±60 Cent
        const auto ab = static_cast<std::size_t>(0.1 * fs);
        const auto n = static_cast<std::size_t>(std::min<std::int64_t>(frames - static_cast<std::int64_t>(ab), static_cast<std::int64_t>(0.4 * fs)));
        double best = 0.0;
        for (double c = -60.0; c <= 60.0 && n > 1000; c += 0.5)
        {
            const double f = hzVon(a.note) * std::exp2(c / 1200.0);
            std::complex<double> z(0.0, 0.0);
            const std::complex<double> dreh = std::polar(1.0, -2.0 * kPi * f / fs);
            std::complex<double> w(1.0, 0.0);
            for (std::size_t i = 0; i < n; ++i)
            {
                z += (0.5 - 0.5 * std::cos(2.0 * kPi * i / n)) * summe[ab + i] * w;
                w *= dreh;
            }
            if (std::abs(z) > best)
            {
                best = std::abs(z);
                messung->spielHz = f;
            }
        }
        messung->schwingt = best > 0.0;
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

void rechneZupfClip(const ZupfSpiel& s, const std::vector<ClipNote>& noten, std::vector<float>& mono, std::int64_t frames, bool mitschwingen,
                    int rate)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || rate <= 0 || !s.spielbar || s.laengeJeNote.punkte.empty()) return;
    const double fs = rate, dt = 1.0 / fs;
    std::vector<Modus> moden;
    std::array<double, 4> mDurch{0.0, 0.0, 0.0, 0.0};
    for (const StegMode& sm : s.steg)
    {
        Modus m;
        const double wn = 2.0 * kPi * sm.hz, beta = wn / sm.q;
        const double wd = std::sqrt(std::max(1e-12, wn * wn - 0.25 * beta * beta));
        const std::complex<double> lambda(-0.5 * beta, wd);
        const double c = sm.spitze * beta;
        m.pol = std::exp(lambda * dt);
        m.gewicht = c * lambda / std::complex<double>(0.0, 2.0 * wd) * dt;
        m.durchgriff = 0.5 * c * dt;
        const double w = sm.winkelGrad * kPi / 180.0;
        m.dy = std::cos(w);
        m.dz = std::sin(w);
        mDurch[0] += m.durchgriff * m.dy * m.dy;
        mDurch[1] += m.durchgriff * m.dy * m.dz;
        mDurch[2] += m.durchgriff * m.dz * m.dy;
        mDurch[3] += m.durchgriff * m.dz * m.dz;
        moden.push_back(m);
    }
    // die Saiten: je Tonhoehe eine; gezupft werden sie von den Noten des Clips
    struct Saite
    {
        int note = 60;
        double z0 = 1.0, dB = 2.0, dN = 2.0;
        Verlust verlust[2];
        Ring zumSattel[2], vomSattel[2], zumSteg[2], vomSteg[2];
        std::vector<const ClipNote*> zupfe;
    };
    std::vector<int> tonhoehen;
    for (const ClipNote& cn : noten) tonhoehen.push_back(std::clamp(cn.anschlag.note, s.noteMin, s.noteMax));
    if (mitschwingen)
        for (const int n : s.mitschwingNoten) tonhoehen.push_back(std::clamp(n, s.noteMin, s.noteMax));
    std::sort(tonhoehen.begin(), tonhoehen.end());
    tonhoehen.erase(std::unique(tonhoehen.begin(), tonhoehen.end()), tonhoehen.end());
    std::vector<Saite> saiten(tonhoehen.size());
    double zSumme = 0.0;
    for (std::size_t i = 0; i < saiten.size(); ++i)
    {
        Saite& sa = saiten[i];
        sa.note = tonhoehen[i];
        const double f = hzVon(sa.note);
        const double laenge = wertAn(s.laengeJeNote, sa.note, Skala::Logarithmisch);
        const double zug = wertAn(s.zugJeNote, sa.note, Skala::Logarithmisch);
        const double c = 2.0 * laenge * f;
        sa.z0 = zug / c;
        zSumme += sa.z0;
        const double rel = s.zupfpunktJeNote.punkte.empty() ? s.zupfpunktRel : wertAn(s.zupfpunktJeNote, sa.note, Skala::Linear);
        sa.verlust[0].setze(s.guete, f, fs);
        sa.verlust[1].setze(s.guete * 0.9, f, fs);
        const double k = s.laufzeitJeNote.punkte.empty() ? 1.0 : wertAn(s.laufzeitJeNote, sa.note, Skala::Logarithmisch);
        sa.dB = std::max(2.0, k * rel * laenge / c * fs);
        sa.dN = std::max(2.0, k * (1.0 - rel) * laenge / c * fs - sa.verlust[0].verzoegerung);
        for (const ClipNote& cn : noten)
            if (std::clamp(cn.anschlag.note, s.noteMin, s.noteMax) == sa.note) sa.zupfe.push_back(&cn);
    }
    const double a11 = 1.0 + zSumme * mDurch[0], a12 = zSumme * mDurch[1], a21 = zSumme * mDurch[2], a22 = 1.0 + zSumme * mDurch[3];
    const double det = a11 * a22 - a12 * a21;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::vector<std::array<double, 2>> vPlus(saiten.size());
    double uyVor = 0.0;
    for (std::int64_t n = 0; n < frames; ++n)
    {
        const double t = n * dt;
        double sy = 0.0, sz = 0.0;  // Σ Z0ᵢ·v⁺ᵢ
        for (Saite& sa : saiten)
        {
            // die Kraftstoesse der Noten auf dieser Saite und das Daempfen nach der letzten (die Hand bzw. der Daempfer;
            // eine mitschwingende Saite ohne Note bleibt frei)
            double fz = 0.0, winkel = 0.0, daempfung = 0.0;
            bool lautenzug = false;
            for (const ClipNote* cn : sa.zupfe)
            {
                const ZupfAnschlag& a = cn->anschlag;
                const double tl = t - cn->abS;
                if (tl < 0.0) continue;
                daempfung = 0.0;
                lautenzug = a.lautenzug;
                if (tl < s.stossS)
                {
                    const double e = std::clamp(a.staerke, 0.0, 1.0);
                    fz = s.kraftPpN * std::pow(s.kraftFfN / s.kraftPpN, e) * 0.5 * (1.0 - std::cos(2.0 * kPi * tl / s.stossS));
                    winkel = (a.polarisationGrad >= 0.0 ? a.polarisationGrad : s.polarisationGrad) * kPi / 180.0;
                }
                if (tl >= a.haltenS && !a.laissezVibrer) daempfung = std::min(1.0, (tl - a.haltenS) / s.daempfenS);
            }
            (void)lautenzug;  // der Lautenzug gilt nur je Note (rechneZupfTon), offen
            for (int p = 0; p < 2; ++p)
            {
                const double vonSattel = sa.vomSattel[p].lies(sa.dN);
                const double vonSteg = sa.vomSteg[p].lies(sa.dB);
                const double amSattel = sa.zumSattel[p].lies(sa.dN);
                vPlus[static_cast<std::size_t>(&sa - saiten.data())][static_cast<std::size_t>(p)] = sa.zumSteg[p].lies(sa.dB);
                const double fp = fz * (p == 0 ? std::cos(winkel) : std::sin(winkel));
                const double ein = fp / (2.0 * sa.z0);
                sa.zumSattel[p].schreibe(vonSteg + ein);
                sa.zumSteg[p].schreibe(vonSattel + ein);
                const double g = 1.0 - 0.995 * daempfung;
                sa.vomSattel[p].schreibe(-g * sa.verlust[p].schritt(amSattel));
            }
            const auto& v = vPlus[static_cast<std::size_t>(&sa - saiten.data())];
            sy += sa.z0 * v[0];
            sz += sa.z0 * v[1];
        }
        double uvy = 0.0, uvz = 0.0;
        for (const Modus& m : moden)
        {
            const double r = 2.0 * m.zustand.real();
            uvy += r * m.dy;
            uvz += r * m.dz;
        }
        const double ry = uvy + 2.0 * (mDurch[0] * sy + mDurch[1] * sz);
        const double rz = uvz + 2.0 * (mDurch[2] * sy + mDurch[3] * sz);
        const double uy = (a22 * ry - a12 * rz) / det, uz = (a11 * rz - a21 * ry) / det;
        // die Kraft aller Saiten auf den Steg: Σ Z0ᵢ·(2v⁺ᵢ − u)
        const double fy = 2.0 * sy - zSumme * uy, fzs = 2.0 * sz - zSumme * uz;
        for (Modus& m : moden)
        {
            const double fm = m.dy * fy + m.dz * fzs;
            const double zr = m.zustand.real(), zi = m.zustand.imag();
            const double nr = zr * m.pol.real() - zi * m.pol.imag() + m.gewicht.real() * fm;
            const double ni = zr * m.pol.imag() + zi * m.pol.real() + m.gewicht.imag() * fm;
            m.zustand = {nr, ni};
        }
        for (std::size_t i = 0; i < saiten.size(); ++i)
        {
            saiten[i].vomSteg[0].schreibe(uy - vPlus[i][0]);
            saiten[i].vomSteg[1].schreibe(uz - vPlus[i][1]);
        }
        summe[static_cast<std::size_t>(n)] = (uy - uyVor) * fs * 1e-3;
        uyVor = uy;
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

void stimme(ZupfSpiel& s)
{
    if (!s.spielbar) return;
    std::vector<int> noten;
    for (int n = s.noteMin; n <= s.noteMax; n += 3) noten.push_back(n);
    if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
    std::vector<double> k(noten.size(), 1.0);
    ZupfSpiel t = s;
    t.laufzeitJeNote = {};
    parallelJe(noten.size(), [&](std::size_t i) {
        double faktor = 1.0;
        for (int runde = 0; runde < 3; ++runde)
        {
            ZupfSpiel u = t;
            u.laufzeitJeNote = kurveAusPunkten({{noten[i], faktor}});
            ZupfAnschlag a;
            a.note = noten[i];
            a.haltenS = 0.6;
            ZupfMessung m;
            std::vector<float> ton;
            rechneZupfTon(u, a, ton, static_cast<std::int64_t>(0.6 * a.rate), &m);
            if (!m.schwingt) break;
            const double fehler = 1200.0 * std::log2(m.spielHz / hzVon(noten[i]));
            if (std::abs(fehler) < 0.6) break;
            faktor *= m.spielHz / hzVon(noten[i]);
        }
        k[i] = faktor;
    });
    Kurve kurve;
    kurve.belegkraft = Belegkraft::Rechnung;
    kurve.herkunft = "Faktor auf die Laufzeiten: die Spitze des Grundtons auf der Note (stimme)";
    for (std::size_t i = 0; i < noten.size(); ++i) kurve.punkte.push_back({noten[i], k[i]});
    s.laufzeitJeNote = kurve;
}

}  // namespace ic::zupf

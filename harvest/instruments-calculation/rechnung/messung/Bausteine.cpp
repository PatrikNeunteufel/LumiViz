/**
 * @file Bausteine.cpp
 * @brief Messkette: die Bausteine B1–B8 (Herleitung im Kopf von `Bausteine.hpp`)
 */
#include "messung/Bausteine.hpp"

#include "gemeinsam/Balken.hpp"
#include "gemeinsam/Eigenzerlegung.hpp"
#include "messung/Teiltonmessung.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic::messung
{

namespace
{

constexpr double kPi = std::numbers::pi;

/// Loest A·x = b (n × n, zeilenweise) mit Pivotsuche; false, wenn singulaer.
bool loese(std::vector<double> a, std::vector<double>& b, int n)
{
    for (int k = 0; k < n; ++k)
    {
        int p = k;
        for (int i = k + 1; i < n; ++i)
            if (std::abs(a[static_cast<std::size_t>(i * n + k)]) > std::abs(a[static_cast<std::size_t>(p * n + k)])) p = i;
        if (std::abs(a[static_cast<std::size_t>(p * n + k)]) < 1e-300) return false;
        if (p != k)
        {
            for (int j = 0; j < n; ++j) std::swap(a[static_cast<std::size_t>(k * n + j)], a[static_cast<std::size_t>(p * n + j)]);
            std::swap(b[static_cast<std::size_t>(k)], b[static_cast<std::size_t>(p)]);
        }
        for (int i = k + 1; i < n; ++i)
        {
            const double f = a[static_cast<std::size_t>(i * n + k)] / a[static_cast<std::size_t>(k * n + k)];
            for (int j = k; j < n; ++j) a[static_cast<std::size_t>(i * n + j)] -= f * a[static_cast<std::size_t>(k * n + j)];
            b[static_cast<std::size_t>(i)] -= f * b[static_cast<std::size_t>(k)];
        }
    }
    for (int i = n - 1; i >= 0; --i)
    {
        double s = b[static_cast<std::size_t>(i)];
        for (int j = i + 1; j < n; ++j) s -= a[static_cast<std::size_t>(i * n + j)] * b[static_cast<std::size_t>(j)];
        b[static_cast<std::size_t>(i)] = s / a[static_cast<std::size_t>(i * n + i)];
    }
    return true;
}

std::vector<double> hann(std::size_t n)
{
    std::vector<double> w(n);
    for (std::size_t i = 0; i < n; ++i) w[i] = 0.5 - 0.5 * std::cos(2.0 * kPi * (i + 0.5) / n);
    return w;
}

/// Die Spitzen eines Rahmens: Frequenz und Amplitude (linear) parabolisch auf dB.
struct Spitze
{
    double hz;
    double amp;
};

std::vector<Spitze> spitzen(std::span<const double> x, std::size_t ab, std::size_t n, std::size_t nfft, const std::vector<double>& w,
                            double wSumme, int rate, const Fft& fft, double schwelleDb)
{
    std::vector<double> z(nfft, 0.0);
    for (std::size_t i = 0; i < n && ab + i < x.size(); ++i) z[i] = x[ab + i] * w[i];
    std::vector<std::complex<double>> s(nfft / 2 + 1);
    fft.vorwaerts(z, s);
    std::vector<double> db(s.size());
    double groesste = -300.0;
    for (std::size_t k = 0; k < s.size(); ++k)
    {
        db[k] = 20.0 * std::log10(std::max(1e-300, 2.0 * std::abs(s[k]) / wSumme));
        groesste = std::max(groesste, db[k]);
    }
    std::vector<Spitze> p;
    for (std::size_t k = 2; k + 2 < db.size(); ++k)
    {
        if (!(db[k] > db[k - 1] && db[k] >= db[k + 1]) || db[k] < groesste + schwelleDb) continue;
        const double a = db[k - 1], b = db[k], c = db[k + 1];
        const double d = 0.5 * (a - c) / (a - 2.0 * b + c);
        p.push_back({(k + d) * rate / static_cast<double>(nfft), std::pow(10.0, (b - 0.25 * (a - c) * d) / 20.0)});
    }
    return p;
}

}  // namespace

std::vector<ModalfitMode> modalfit(std::span<const double> signal, int rate, std::size_t ab, std::size_t laenge, int ordnung, int zeilen)
{
    std::vector<ModalfitMode> moden;
    const int p = 2 * std::clamp(ordnung, 1, 16);
    if (ab + laenge > signal.size()) laenge = signal.size() > ab ? signal.size() - ab : 0;
    const int m = std::clamp(zeilen, p + 2, static_cast<int>(laenge / 2));
    if (static_cast<int>(laenge) < 2 * m || rate <= 0) return moden;
    std::vector<double> x(signal.begin() + static_cast<std::ptrdiff_t>(ab), signal.begin() + static_cast<std::ptrdiff_t>(ab + laenge));
    double gross = 0.0;
    for (const double v : x) gross = std::max(gross, std::abs(v));
    if (gross <= 0.0) return moden;
    for (double& v : x) v /= gross;
    const std::size_t l = laenge - static_cast<std::size_t>(m) + 1;
    // die Kovarianz der Hankel-Matrix
    std::vector<double> r(static_cast<std::size_t>(m) * m, 0.0);
    for (int i = 0; i < m; ++i)
        for (int j = i; j < m; ++j)
        {
            double s = 0.0;
            for (std::size_t t = 0; t < l; ++t) s += x[t + static_cast<std::size_t>(i)] * x[t + static_cast<std::size_t>(j)];
            r[static_cast<std::size_t>(i * m + j)] = r[static_cast<std::size_t>(j * m + i)] = s;
        }
    const SymmetrischeZerlegung z = zerlegeSymmetrisch(m, r);
    // der Signalraum: die p groessten (aufsteigend sortiert → die letzten)
    std::vector<double> u(static_cast<std::size_t>(m) * p);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < p; ++j) u[static_cast<std::size_t>(i * p + j)] = z.vektoren[static_cast<std::size_t>(i * m + (m - p + j))];
    // Φ = (U1ᵀU1)⁻¹·U1ᵀU2
    std::vector<double> a(static_cast<std::size_t>(p) * p, 0.0), c(static_cast<std::size_t>(p) * p, 0.0);
    for (int i = 0; i < p; ++i)
        for (int j = 0; j < p; ++j)
        {
            double s1 = 0.0, s2 = 0.0;
            for (int t = 0; t + 1 < m; ++t)
            {
                s1 += u[static_cast<std::size_t>(t * p + i)] * u[static_cast<std::size_t>(t * p + j)];
                s2 += u[static_cast<std::size_t>(t * p + i)] * u[static_cast<std::size_t>((t + 1) * p + j)];
            }
            a[static_cast<std::size_t>(i * p + j)] = s1;
            c[static_cast<std::size_t>(i * p + j)] = s2;
        }
    KleineMatrix phi(p);
    for (int j = 0; j < p; ++j)
    {
        std::vector<double> spalte(static_cast<std::size_t>(p));
        for (int i = 0; i < p; ++i) spalte[static_cast<std::size_t>(i)] = c[static_cast<std::size_t>(i * p + j)];
        if (!loese(a, spalte, p)) return moden;
        for (int i = 0; i < p; ++i) phi(i, j) = spalte[static_cast<std::size_t>(i)];
    }
    const Eigenzerlegung e = zerlegeKlein(phi);
    std::vector<std::complex<double>> pole;
    for (const std::complex<double>& w : e.werte)
        if (w.imag() > 1e-9 && std::abs(w) > 0.0) pole.push_back(w);
    const int k = static_cast<int>(pole.size());
    if (k == 0) return moden;
    // die Amplituden: x[n] = Σ 2·Re(c_j·z_j^n), kleinste Quadrate in Re c, Im c
    const int n2 = 2 * k;
    std::vector<double> nn(static_cast<std::size_t>(n2) * n2, 0.0), rechts(static_cast<std::size_t>(n2), 0.0);
    std::vector<std::complex<double>> zn(static_cast<std::size_t>(k), 1.0);
    std::vector<double> basis(static_cast<std::size_t>(n2));
    for (std::size_t t = 0; t < laenge; ++t)
    {
        for (int j = 0; j < k; ++j)
        {
            basis[static_cast<std::size_t>(2 * j)] = 2.0 * zn[static_cast<std::size_t>(j)].real();
            basis[static_cast<std::size_t>(2 * j + 1)] = -2.0 * zn[static_cast<std::size_t>(j)].imag();
            zn[static_cast<std::size_t>(j)] *= pole[static_cast<std::size_t>(j)];
        }
        for (int i = 0; i < n2; ++i)
        {
            rechts[static_cast<std::size_t>(i)] += basis[static_cast<std::size_t>(i)] * x[t];
            for (int j = 0; j < n2; ++j) nn[static_cast<std::size_t>(i * n2 + j)] += basis[static_cast<std::size_t>(i)] * basis[static_cast<std::size_t>(j)];
        }
    }
    if (!loese(nn, rechts, n2)) return moden;
    for (int j = 0; j < k; ++j)
    {
        const std::complex<double> lam = std::log(pole[static_cast<std::size_t>(j)]) * static_cast<double>(rate);
        const std::complex<double> cj(rechts[static_cast<std::size_t>(2 * j)], rechts[static_cast<std::size_t>(2 * j + 1)]);
        ModalfitMode mm;
        mm.hz = lam.imag() / (2.0 * kPi);
        mm.sigma = -lam.real();
        mm.amplitude = 2.0 * std::abs(cj) * gross;
        mm.phase = std::arg(cj);
        if (mm.sigma >= 0.0) moden.push_back(mm);
    }
    std::sort(moden.begin(), moden.end(), [](const ModalfitMode& p1, const ModalfitMode& p2) { return p1.hz < p2.hz; });
    return moden;
}

Verfolgung verfolgeTeiltoene(std::span<const double> signal, int rate, const Fft& fft, double fensterS, double schrittS, double schwelleDb,
                             double sprungCent, std::size_t mindestRahmen)
{
    Verfolgung v;
    v.schrittS = schrittS;
    v.fensterS = fensterS;
    const auto n = static_cast<std::size_t>(std::llround(fensterS * rate));
    const auto schritt = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(schrittS * rate)));
    const std::size_t nfft = 4 * zweierpotenzAb(n);
    const std::vector<double> w = hann(n);
    double wSumme = 0.0;
    for (const double x : w) wSumme += x;
    std::vector<Spur> alle;
    std::vector<std::size_t> aktiv;
    std::size_t rahmen = 0;
    for (std::size_t ab = 0; ab + n <= signal.size(); ab += schritt, ++rahmen)
    {
        std::vector<Spitze> p = spitzen(signal, ab, n, nfft, w, wSumme, rate, fft, schwelleDb);
        std::vector<bool> genommen(p.size(), false);
        std::vector<std::size_t> weiter;
        // die staerksten Spuren zuerst
        std::stable_sort(aktiv.begin(), aktiv.end(), [&](std::size_t x, std::size_t y) { return alle[x].pegelDb.back() > alle[y].pegelDb.back(); });
        for (const std::size_t s : aktiv)
        {
            const double f = alle[s].hz.back();
            int beste = -1;
            double abstand = sprungCent;
            for (std::size_t i = 0; i < p.size(); ++i)
            {
                if (genommen[i]) continue;
                const double c = std::abs(1200.0 * std::log2(p[i].hz / f));
                if (c < abstand)
                {
                    abstand = c;
                    beste = static_cast<int>(i);
                }
            }
            if (beste < 0) continue;  // Tod
            genommen[static_cast<std::size_t>(beste)] = true;
            alle[s].hz.push_back(p[static_cast<std::size_t>(beste)].hz);
            alle[s].pegelDb.push_back(20.0 * std::log10(p[static_cast<std::size_t>(beste)].amp));
            weiter.push_back(s);
        }
        for (std::size_t i = 0; i < p.size(); ++i)
            if (!genommen[i])
            {
                Spur neu;
                neu.start = rahmen;
                neu.hz.push_back(p[i].hz);
                neu.pegelDb.push_back(20.0 * std::log10(p[i].amp));
                alle.push_back(std::move(neu));
                weiter.push_back(alle.size() - 1);
            }
        aktiv = std::move(weiter);
    }
    for (Spur& s : alle)
        if (s.hz.size() >= mindestRahmen) v.spuren.push_back(std::move(s));
    return v;
}

Tonhoehe tonhoehe(std::span<const double> signal, int rate, double minHz, double maxHz, double fensterS, double schrittS, double schwelle)
{
    Tonhoehe t;
    t.schrittS = schrittS;
    const auto w = static_cast<std::size_t>(std::llround(fensterS * rate));
    const auto tauMax = static_cast<std::size_t>(std::ceil(rate / minHz));
    const auto tauMin = static_cast<std::size_t>(std::floor(rate / maxHz));
    const auto schritt = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(schrittS * rate)));
    std::vector<double> d(tauMax + 2);
    for (std::size_t ab = 0; ab + w + tauMax + 1 < signal.size(); ab += schritt)
    {
        // Differenzfunktion und kumulierte mittlere normierte Differenz (YIN, Schritt 2 und 3)
        double summe = 0.0;
        d[0] = 1.0;
        for (std::size_t tau = 1; tau <= tauMax + 1; ++tau)
        {
            double s = 0.0;
            for (std::size_t i = 0; i < w; ++i)
            {
                const double x = signal[ab + i] - signal[ab + i + tau];
                s += x * x;
            }
            summe += s;
            d[tau] = summe > 0.0 ? s * tau / summe : 1.0;
        }
        // Schritt 4: die erste Senke unter der Schwelle, sonst das globale Minimum
        std::size_t best = 0;
        for (std::size_t tau = std::max<std::size_t>(2, tauMin); tau <= tauMax; ++tau)
            if (d[tau] < schwelle)
            {
                while (tau + 1 <= tauMax && d[tau + 1] < d[tau]) ++tau;
                best = tau;
                break;
            }
        double guete = 1.0;
        if (best == 0)
        {
            for (std::size_t tau = std::max<std::size_t>(2, tauMin); tau <= tauMax; ++tau)
                if (best == 0 || d[tau] < d[best]) best = tau;
            guete = d[best];
            t.hz.push_back(0.0);
            t.guete.push_back(guete);
            continue;
        }
        guete = d[best];
        // Schritt 5: parabolisch
        const double a = d[best - 1], b = d[best], c = d[best + 1];
        const double den = a - 2.0 * b + c;
        const double tauGenau = best + (std::abs(den) > 0.0 ? 0.5 * (a - c) / den : 0.0);
        t.hz.push_back(rate / tauGenau);
        t.guete.push_back(guete);
    }
    return t;
}

Vibrato vibrato(const std::vector<double>& f0Hz, const std::vector<double>& huelleDb, double schrittS, double vonHz, double bisHz,
                double fensterS)
{
    Vibrato v;
    std::vector<double> cent;
    double mittel = 0.0;
    int n = 0;
    for (const double f : f0Hz)
        if (f > 0.0)
        {
            mittel += std::log2(f);
            ++n;
        }
    if (n < 16) return v;
    mittel /= n;
    for (const double f : f0Hz) cent.push_back(f > 0.0 ? 1200.0 * (std::log2(f) - mittel) : 0.0);
    double hm = 0.0;
    for (const double h : huelleDb) hm += h;
    if (!huelleDb.empty()) hm /= static_cast<double>(huelleDb.size());
    const auto amp = [&](const std::vector<double>& x, double mitte, double f) {
        std::complex<double> s(0.0, 0.0);
        double wsum = 0.0;
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            const double w = 0.5 - 0.5 * std::cos(2.0 * kPi * (i + 0.5) / x.size());
            s += w * (x[i] - mitte) * std::exp(std::complex<double>(0.0, -2.0 * kPi * f * i * schrittS));
            wsum += w;
        }
        return 2.0 * s / wsum;
    };
    double besteF = 0.0, besteA = 0.0;
    for (double f = vonHz; f <= bisHz; f += 0.01)
    {
        const double a = std::abs(amp(cent, 0.0, f));
        if (a > besteA)
        {
            besteA = a;
            besteF = f;
        }
    }
    const double x = kPi * besteF * fensterS;
    const double fenster = x > 1e-9 ? std::sin(x) / x : 1.0;
    v.erkannt = besteA > 0.0;
    v.rateHz = besteF;
    v.tiefeCent = besteA / fenster;
    if (!huelleDb.empty())
    {
        const std::complex<double> am = amp(huelleDb, hm, besteF);
        const std::complex<double> fm = amp(cent, 0.0, besteF);
        v.amTiefeDb = std::abs(am) / fenster;
        v.amPhaseGrad = std::arg(am / fm) * 180.0 / kPi;
    }
    return v;
}

Rest rest(std::span<const double> signal, int rate, const Verfolgung& v, const Fft& /*fft*/)
{
    Rest r;
    // je Rahmen die Sinus der aktiven Spuren als kleinste Quadrate (a·cos + b·sin) im Hann-Fenster: Energie der
    // Anpassung und des Rests, ueber die Rahmen summiert
    const auto n = static_cast<std::size_t>(std::llround(v.fensterS * rate));
    const auto schritt = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(v.schrittS * rate)));
    const std::vector<double> w = hann(n);
    double ePer = 0.0, eRest = 0.0;
    std::size_t rahmen = 0;
    for (std::size_t ab = 0; ab + n <= signal.size(); ab += schritt, ++rahmen)
    {
        std::vector<double> f;
        for (const Spur& s : v.spuren)
            if (rahmen >= s.start && rahmen < s.start + s.hz.size()) f.push_back(s.hz[rahmen - s.start]);
        const int m = 2 * static_cast<int>(f.size());
        std::vector<double> a(static_cast<std::size_t>(m) * m, 0.0), b(static_cast<std::size_t>(m), 0.0);
        std::vector<double> basis(static_cast<std::size_t>(m));
        const auto bas = [&](std::size_t i) {
            const double t = (static_cast<double>(i) - 0.5 * n) / rate;
            for (std::size_t j = 0; j < f.size(); ++j)
            {
                basis[2 * j] = std::cos(2.0 * kPi * f[j] * t);
                basis[2 * j + 1] = std::sin(2.0 * kPi * f[j] * t);
            }
        };
        for (std::size_t i = 0; i < n; ++i)
        {
            bas(i);
            const double x = signal[ab + i];
            for (int p = 0; p < m; ++p)
            {
                b[static_cast<std::size_t>(p)] += w[i] * basis[static_cast<std::size_t>(p)] * x;
                for (int q = 0; q < m; ++q) a[static_cast<std::size_t>(p * m + q)] += w[i] * basis[static_cast<std::size_t>(p)] * basis[static_cast<std::size_t>(q)];
            }
        }
        const bool gut = m > 0 && loese(a, b, m);
        for (std::size_t i = 0; i < n; ++i)
        {
            double fit = 0.0;
            if (gut)
            {
                bas(i);
                for (int p = 0; p < m; ++p) fit += b[static_cast<std::size_t>(p)] * basis[static_cast<std::size_t>(p)];
            }
            const double x = signal[ab + i];
            ePer += w[i] * fit * fit;
            eRest += w[i] * (x - fit) * (x - fit);
        }
    }
    r.periodischDb = 10.0 * std::log10(std::max(1e-300, ePer));
    r.restDb = 10.0 * std::log10(std::max(1e-300, eRest));
    r.abstandDb = r.periodischDb - r.restDb;
    return r;
}

double einschwingen(std::span<const double> signal, int rate)
{
    double sp = 0.0;
    for (const double x : signal) sp = std::max(sp, std::abs(x));
    if (sp <= 0.0) return 0.0;
    std::size_t einsatz = 0;
    while (einsatz < signal.size() && std::abs(signal[einsatz]) <= 0.01 * sp) ++einsatz;
    const auto n = static_cast<std::size_t>(0.01 * rate);
    std::vector<double> pegel;
    for (std::size_t ab = einsatz; ab + n <= signal.size(); ab += n)
    {
        double s = 0.0;
        for (std::size_t i = 0; i < n; ++i) s += signal[ab + i] * signal[ab + i];
        pegel.push_back(10.0 * std::log10(std::max(1e-300, s / n)));
    }
    if (pegel.size() < 4) return 0.0;
    std::vector<double> zweite(pegel.begin() + static_cast<std::ptrdiff_t>(pegel.size() / 2), pegel.end());
    std::nth_element(zweite.begin(), zweite.begin() + static_cast<std::ptrdiff_t>(zweite.size() / 2), zweite.end());
    const double gehalten = zweite[zweite.size() / 2];
    for (std::size_t i = 0; i < pegel.size(); ++i)
        if (pegel[i] >= gehalten - 3.0) return (static_cast<double>(i) + 0.5) * 0.01;
    return static_cast<double>(pegel.size()) * 0.01;
}

Baender baenderUeberZeit(std::span<const double> signal, int rate, const Fft& fft, double fensterS, double schrittS)
{
    Baender b;
    b.schrittS = schrittS;
    b.mitten = terzMitten();
    const auto n = static_cast<std::size_t>(std::llround(fensterS * rate));
    const std::size_t nfft = zweierpotenzAb(n);
    const auto schritt = std::max<std::size_t>(1, static_cast<std::size_t>(std::llround(schrittS * rate)));
    const std::vector<double> w = hann(n);
    std::vector<double> z(nfft);
    std::vector<std::complex<double>> s(nfft / 2 + 1);
    for (std::size_t ab = 0; ab + n <= signal.size(); ab += schritt)
    {
        std::fill(z.begin(), z.end(), 0.0);
        for (std::size_t i = 0; i < n; ++i) z[i] = signal[ab + i] * w[i];
        fft.vorwaerts(z, s);
        std::vector<double> pegel(b.mitten.size(), 0.0);
        double a = 0.0, e = 0.0;
        for (std::size_t k = 1; k < s.size(); ++k)
        {
            const double f = k * static_cast<double>(rate) / nfft;
            const double p = std::norm(s[k]);
            a += f * p;
            e += p;
            for (std::size_t j = 0; j < b.mitten.size(); ++j)
                if (f >= b.mitten[j] * std::pow(2.0, -1.0 / 6.0) && f < b.mitten[j] * std::pow(2.0, 1.0 / 6.0)) pegel[j] += p;
        }
        for (double& p : pegel) p = 10.0 * std::log10(std::max(1e-300, p));
        b.pegelDb.push_back(pegel);
        b.schwerpunktHz.push_back(e > 0.0 ? a / e : 0.0);
    }
    return b;
}

std::vector<double> gleiten(const Verfolgung& v, double bisS)
{
    // die Spur mit der groessten Spitze, die in den ersten drei Rahmen beginnt
    const Spur* beste = nullptr;
    double spitze = -1e300;
    for (const Spur& s : v.spuren)
    {
        if (s.start > 2) continue;
        const double m = *std::max_element(s.pegelDb.begin(), s.pegelDb.end());
        if (m > spitze)
        {
            spitze = m;
            beste = &s;
        }
    }
    std::vector<double> f;
    if (beste == nullptr) return f;
    const auto bis = static_cast<std::size_t>(bisS / v.schrittS);
    for (std::size_t i = 0; i < beste->hz.size() && beste->start + i <= bis; ++i) f.push_back(beste->hz[i]);
    return f;
}

std::vector<double> amplitudenBei(std::span<const double> signal, int rate, double hz, const std::vector<double>& zeitenS, double fensterS)
{
    std::vector<double> db;
    const auto n = static_cast<std::size_t>(std::llround(fensterS * rate));
    const std::vector<double> w = hann(n);
    double wSumme = 0.0;
    for (const double x : w) wSumme += x;
    for (const double t : zeitenS)
    {
        const auto mitte = static_cast<std::ptrdiff_t>(std::llround(t * rate));
        const std::ptrdiff_t ab = mitte - static_cast<std::ptrdiff_t>(n / 2);
        std::complex<double> s(0.0, 0.0);
        for (std::size_t i = 0; i < n; ++i)
        {
            const std::ptrdiff_t j = ab + static_cast<std::ptrdiff_t>(i);
            if (j < 0 || j >= static_cast<std::ptrdiff_t>(signal.size())) continue;
            s += w[i] * signal[static_cast<std::size_t>(j)] * std::exp(std::complex<double>(0.0, -2.0 * kPi * hz * static_cast<double>(j) / rate));
        }
        db.push_back(20.0 * std::log10(std::max(1e-300, 2.0 * std::abs(s) / wSumme)));
    }
    return db;
}

}  // namespace ic::messung

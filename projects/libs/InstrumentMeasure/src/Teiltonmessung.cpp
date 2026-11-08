/**
 * @file   Teiltonmessung.cpp
 * @brief  Die Rechnung zu `InstrumentMeasure/Teiltonmessung.hpp` (Herleitung dort). Wortgleich aus dem Kopf der
 *         Unit-Tests gezogen (Instrumente_IC_Konzept.md, Teilplan 34e-1): in der .cpp, damit die Schalter nach
 *         E1 a (PRIVATE) gelten.
 */
#include "InstrumentMeasure/Teiltonmessung.hpp"

namespace lumi::messung
{

namespace detail
{

std::vector<double> blackmanHarris(std::size_t n)
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

double db(double amplitude)
{
    return 20.0 * std::log10(std::max(amplitude, 1e-15));
}

Spektrum::Spektrum(const lumi::kern::Fft& fft, std::size_t fensterLaenge, std::size_t n)
    : m_fft(fft), m_fenster(blackmanHarris(fensterLaenge)), m_zeit(n), m_frei(n / 2 + 1), m_db(n / 2 + 1)
{
    double summe = 0.0;
    for (const double w : m_fenster) summe += w;
    m_normierung = 2.0 / summe;
}

void Spektrum::rechne(std::span<const double> ton, std::ptrdiff_t von)
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

std::pair<double, double> Spektrum::spitze(std::size_t von, std::size_t bis) const
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

Geraden::Geraden(std::span<const double> x, std::span<const double> y)
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

Geraden::Gerade Geraden::ueber(std::size_t von, std::size_t bis) const
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

Inharmonie gleicheAus(const std::vector<int>& k, const std::vector<double>& f)
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

Inharmonie gleicheAusRobust(const std::vector<int>& k, const std::vector<double>& f, std::vector<bool>& gilt)
{
    gilt.assign(k.size(), true);
    Inharmonie e;
    for (int runde = 0; runde < 5; ++runde)
    {
        std::vector<int> kk;
        std::vector<double> ff;
        for (std::size_t i = 0; i < k.size(); ++i)
            if (gilt[i])
            {
                kk.push_back(k[i]);
                ff.push_back(f[i]);
            }
        e = gleicheAus(kk, ff);
        if (!e.erkannt || kk.size() < 3) return e;
        std::vector<double> cent(k.size());
        std::vector<double> betraege;
        for (std::size_t i = 0; i < k.size(); ++i)
        {
            const double soll = k[i] * e.f0 * std::sqrt(std::max(1e-12, 1.0 + e.b * k[i] * k[i]));
            cent[i] = std::abs(1200.0 * std::log2(f[i] / soll));
            if (gilt[i]) betraege.push_back(cent[i]);
        }
        const double grenze = std::max(5.0, 4.0 * 1.4826 * median(betraege));
        std::vector<bool> neu(k.size());
        std::size_t bleiben = 0;
        for (std::size_t i = 0; i < k.size(); ++i)
        {
            neu[i] = cent[i] <= grenze;
            bleiben += neu[i] ? 1 : 0;
        }
        if (neu == gilt || bleiben < 2) return e;
        gilt = neu;
    }
    return e;
}

Abklingen zweiStufen(std::span<const double> pegelDb, double schrittS, std::size_t von, std::size_t bis)
{
    Abklingen a;
    if (bis <= von || bis - von < 20) return a;
    const std::size_t n = bis - von;
    std::vector<double> e(n);
    for (std::size_t j = 0; j < n; ++j) e[j] = std::pow(10.0, pegelDb[von + j] / 10.0);
    const std::size_t letzte = std::max<std::size_t>(1, n / 20);
    // mindestens 10 dB Abfall des Teiltons: die Energie der ersten und der letzten 5 % des Bereichs
    double anfangE = 0.0;
    double endeE = 0.0;
    for (std::size_t j = 0; j < letzte; ++j)
    {
        anfangE += e[j];
        endeE += e[n - 1 - j];
    }
    if (endeE <= 0.0 || 10.0 * std::log10(anfangE / endeE) < 10.0) return a;
    const double endPegel = median(std::vector<double>(pegelDb.begin() + static_cast<std::ptrdiff_t>(bis - letzte),
                                                       pegelDb.begin() + static_cast<std::ptrdiff_t>(bis)));
    std::size_t m = n;
    while (m > 0 && pegelDb[von + m - 1] < endPegel + 10.0) --m;
    if (m < 20) m = n;  // ein flacher Abfall: ohne den Schnitt vor dem Ende, sonst bliebe zu wenig

    const auto schroeder = [](const std::vector<double>& energie) {
        std::vector<double> summe(energie.size() + 1, 0.0);
        for (std::size_t j = energie.size(); j-- > 0;) summe[j] = summe[j + 1] + energie[j];
        return summe;
    };
    const std::vector<double> E = schroeder(e);
    if (E[0] <= 0.0) return a;
    std::vector<double> t(m);
    std::vector<double> L(m);
    for (std::size_t j = 0; j < m; ++j)
    {
        t[j] = static_cast<double>(von + j) * schrittS;
        L[j] = 10.0 * std::log10(std::max(E[j], 1e-300) / E[0]);
    }
    const Geraden geraden(t, L);
    const Geraden::Gerade eine = geraden.ueber(0, m);
    if (eine.steigung >= 0.0) return a;
    a.erkannt = true;
    a.vonS = t[0];
    a.bisS = static_cast<double>(bis - 1) * schrittS;
    a.t60Frueh = a.t60Spaet = -60.0 / eine.steigung;

    // die fruehe Stufe darf kurz sein (am Fluegel oft eine Sekunde von zwanzig, Befund S152 an
    // Salamander), die spaete mindestens ein Fuenftel: sonst „erkennt“ der Uebergang in den Boden eine
    // kurze zweite Stufe
    const std::size_t randFrueh = std::max<std::size_t>(10, m / 20);
    const std::size_t randSpaet = std::max<std::size_t>(10, m / 5);
    if (randFrueh + randSpaet > m) return a;
    double besterRest = eine.rest;
    std::size_t knick = 0;
    Geraden::Gerade spaet;
    for (std::size_t j = randFrueh; j + randSpaet <= m; ++j)
    {
        const Geraden::Gerade g1 = geraden.ueber(0, j);
        const Geraden::Gerade g2 = geraden.ueber(j, m);
        if (g1.rest + g2.rest < besterRest)
        {
            besterRest = g1.rest + g2.rest;
            knick = j;
            spaet = g2;
        }
    }
    if (knick == 0 || besterRest >= 0.5 * eine.rest || spaet.steigung >= 0.0) return a;

    // die fruehe Stufe: Energie der spaeten Geraden abziehen, den Rest neu integrieren
    const double rahmenFaktor = 1.0 - std::pow(10.0, spaet.steigung * schrittS / 10.0);
    std::vector<double> rest(n);
    for (std::size_t j = 0; j < n; ++j)
    {
        const double tj = static_cast<double>(von + j) * schrittS;
        const double spaeteEnergie = E[0] * std::pow(10.0, (spaet.achse + spaet.steigung * tj) / 10.0) * rahmenFaktor;
        rest[j] = std::max(e[j] - spaeteEnergie, 0.0);
    }
    const std::vector<double> R = schroeder(rest);
    if (R[0] <= 0.0) return a;
    std::vector<double> tf;
    std::vector<double> lf;
    for (std::size_t j = 0; j < m; ++j)
    {
        const double l = 10.0 * std::log10(std::max(R[j], 1e-300) / R[0]);
        if (l < -20.0 && tf.size() >= 5) break;
        tf.push_back(t[j]);
        lf.push_back(l);
    }
    const Geraden::Gerade frueh = Geraden(tf, lf).ueber(0, tf.size());
    if (frueh.steigung >= 0.0 || frueh.steigung >= 1.3 * spaet.steigung) return a;

    // die spaete Stufe ebenso: Energie der fruehen Geraden abziehen, den Rest neu integrieren — sonst zieht der
    // Auslaeufer der fruehen die spaete hinter dem Knick steiler
    const double fruehFaktor = 1.0 - std::pow(10.0, frueh.steigung * schrittS / 10.0);
    std::vector<double> rest2(n);
    for (std::size_t j = 0; j < n; ++j)
    {
        const double tj = static_cast<double>(von + j) * schrittS;
        const double fruehEnergie = R[0] * std::pow(10.0, (frueh.achse + frueh.steigung * tj) / 10.0) * fruehFaktor;
        rest2[j] = std::max(e[j] - fruehEnergie, 0.0);
    }
    const std::vector<double> R2 = schroeder(rest2);
    double spaetSteigung = spaet.steigung;
    if (R2[knick] > 0.0)
    {
        std::vector<double> ls(m - knick);
        for (std::size_t j = knick; j < m; ++j) ls[j - knick] = 10.0 * std::log10(std::max(R2[j], 1e-300) / R2[knick]);
        const Geraden::Gerade g = Geraden(std::span<const double>(t).subspan(knick), ls).ueber(0, ls.size());
        if (g.steigung < 0.0) spaetSteigung = g.steigung;
    }
    a.zweiStufen = true;
    a.knickS = t[knick];
    a.t60Frueh = -60.0 / frueh.steigung;
    a.t60Spaet = -60.0 / spaetSteigung;
    return a;
}

std::vector<double> restUmStufen(std::span<const double> pegelDb, double schrittS, std::size_t von, std::size_t bis,
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

Vorwaerts huellkurveVorwaerts(std::span<const double> pegelDb, double schrittS, std::size_t hoechster, std::size_t von,
                                     std::size_t bis, double vorwaertsS, const Abklingen& a)
{
    Vorwaerts v;
    bis = std::min(bis, pegelDb.size());
    const auto rahmen = static_cast<std::size_t>(std::lround(vorwaertsS / schrittS));
    const std::size_t ende = std::min(bis, hoechster + rahmen + 1);
    if (ende > hoechster + 10)
    {
        std::vector<double> zt;
        std::vector<double> zl;
        for (std::size_t j = hoechster; j < ende; ++j)
        {
            zt.push_back(static_cast<double>(j) * schrittS);
            zl.push_back(pegelDb[j]);
        }
        const Geraden::Gerade g = Geraden(zt, zl).ueber(0, zt.size());
        if (g.steigung < 0.0)
        {
            v.erkannt = true;
            v.t60 = -60.0 / g.steigung;
            v.bisS = static_cast<double>(ende - hoechster) * schrittS;
        }
    }
    if (a.zweiStufen && bis > von)
    {
        const auto knick = static_cast<std::size_t>(std::lround(a.knickS / schrittS));
        if (knick > von && knick < bis)
        {
            double gesamt = 0.0;
            double rest = 0.0;
            for (std::size_t j = von; j < bis; ++j)
            {
                const double e = std::pow(10.0, pegelDb[j] / 10.0);
                gesamt += e;
                if (j >= knick) rest += e;
            }
            if (gesamt > 0.0 && rest > 0.0)
            {
                v.knickErkannt = true;
                v.knickTiefeDb = 10.0 * std::log10(rest / gesamt);
            }
        }
    }
    return v;
}

Schwebung findeSchwebung(std::span<const double> rest, double schrittS, double bisHz)
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
    if (!s.erkannt) return s;

    // die Tiefe der Einbrueche (TB1 a): jede volle Periode enthaelt eine Spitze und einen Einbruch
    const double periode = 1.0 / (s.frequenz * schrittS);
    std::vector<double> tiefen;
    for (std::size_t i = 0;; ++i)
    {
        const auto anfang = static_cast<std::size_t>(std::lround(static_cast<double>(i) * periode));
        const auto ende = static_cast<std::size_t>(std::lround(static_cast<double>(i + 1) * periode));
        if (ende > rest.size() || ende <= anfang) break;
        const auto [unten, oben] = std::minmax_element(rest.begin() + static_cast<std::ptrdiff_t>(anfang),
                                                       rest.begin() + static_cast<std::ptrdiff_t>(ende));
        tiefen.push_back(*oben - *unten);
    }
    if (!tiefen.empty())
    {
        const auto mitte = tiefen.begin() + static_cast<std::ptrdiff_t>(tiefen.size() / 2);
        std::nth_element(tiefen.begin(), mitte, tiefen.end());
        s.einbruchDb = *mitte;
    }
    return s;
}

double median(std::vector<double> werte)
{
    if (werte.empty()) return 0.0;
    const auto mitte = werte.begin() + static_cast<std::ptrdiff_t>(werte.size() / 2);
    std::nth_element(werte.begin(), mitte, werte.end());
    return *mitte;
}
}  // namespace detail

Ergebnis miss(std::span<const double> ton, int rate, double nennFrequenz, const lumi::kern::Fft& fft,
                     const Einstellungen& einst)
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
    const std::size_t langN = 4 * lumi::kern::zweierpotenzAb(langLaenge);
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
        const double unten = k * nennFrequenz * std::exp2(-einst.stimmungCent / 1200.0);
        const double oben = k * nennFrequenz * std::sqrt((1.0 + einst.bHoechstens * k * k) / (1.0 + einst.bHoechstens)) *
                            std::exp2(einst.stimmungCent / 1200.0);
        const double vonHz = std::max(erwartet - 0.25 * ih.f0, unten);
        const double bisHz = std::min(erwartet + 0.25 * ih.f0, oben);
        if (vonHz >= bisHz)
        {
            ++fehlend;
            continue;
        }
        const auto von = static_cast<std::size_t>(std::max(0.0, vonHz / binHz));
        const auto bis = static_cast<std::size_t>(bisHz / binHz);
        const auto [stelle, pegel] = lang.spitze(von, bis);
        if (pegel < grundDb + einst.spitzeUeberMedianDb)
        {
            ++fehlend;
            continue;
        }
        ks.push_back(k);
        fs.push_back(stelle * binHz);
        if (ks.size() >= 2)
        {
            std::vector<bool> gilt;
            const detail::Inharmonie neu = detail::gleicheAusRobust(ks, fs, gilt);
            if (!gilt.back())
            {
                // die Spitze passt nicht in die Reihe: eine fremde, kein Teilton
                ks.pop_back();
                fs.pop_back();
                ++fehlend;
                continue;
            }
            if (neu.erkannt && neu.b > -1e-3) ih = neu;
        }
        else
            ih.f0 = fs.back() / k;
        fehlend = 0;
    }
    // zum Schluss ohne die Ausreisser, die erst mit mehr Teiltoenen sichtbar wurden
    if (ks.size() >= 3)
    {
        std::vector<bool> gilt;
        (void)detail::gleicheAusRobust(ks, fs, gilt);
        std::vector<int> kk;
        std::vector<double> ff;
        for (std::size_t i = 0; i < ks.size(); ++i)
            if (gilt[i])
            {
                kk.push_back(ks[i]);
                ff.push_back(fs[i]);
            }
        ks = std::move(kk);
        fs = std::move(ff);
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
    const std::size_t kurzN = lumi::kern::zweierpotenzAb(fensterL);
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
        // in Energie gemittelt, nicht in dB: die Einbrueche einer Schwebung zoegen ein Mittel in dB tief hinunter und
        // beendeten den Bereich zu frueh (Befund S152, N1 a)
        const auto glatt = [](const std::vector<double>& v, std::size_t j) {
            const std::size_t a = j >= 5 ? j - 5 : 0;
            const std::size_t b = std::min(v.size(), j + 6);
            double s = 0.0;
            for (std::size_t m = a; m < b; ++m) s += std::pow(10.0, v[m] / 10.0);
            return 10.0 * std::log10(std::max(s / static_cast<double>(b - a), 1e-300));
        };
        std::size_t bis = t.pegelDb.size();
        while (bis > von && glatt(t.pegelDb, bis - 1) <= glatt(r, bis - 1) + einst.grenzeDb) --bis;
        // US6 a: hoechstens bis `untergrenzeDb` unter die Spitze — vom Ende her, damit ein tiefer Einbruch davor den
        // Bereich nicht beendet
        while (bis > von && glatt(t.pegelDb, bis - 1) < t.pegelDb[hoechster] - einst.untergrenzeDb) --bis;
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
                // die Gerade ueber die letzten 30 % vor dem Kandidaten: faellt der spaete Teil noch? — ueber den ganzen
                // Bereich machte eine steile fruehe Stufe sie zu steil, und das Ende eines noch fallenden Tons galt
                // als Boden (Befund S152, N1 a)
                std::vector<double> zeiten(t.pegelDb.size());
                for (std::size_t j = 0; j < zeiten.size(); ++j) zeiten[j] = static_cast<double>(j) * einst.schrittS;
                const std::size_t spaetVon = std::min(vorEigen - 10, von + 7 * (vorEigen - von) / 10);
                const detail::Geraden::Gerade g = detail::Geraden(zeiten, t.pegelDb).ueber(spaetVon, vorEigen);
                if (g.achse + g.steigung * zeiten.back() < eigen - 3.0)
                {
                    bis = vorEigen;
                    t.rauschenDb = std::max(t.rauschenDb, eigen);
                }
            }
        }
        t.bodenDb = r;
        if (bis > von)
        {
            t.bereichVon = von;
            t.bereichBis = bis;
        }
        // 7. die Huellkurve vorwaerts (HU2 a): die Gerade ab der Spitze ueber `vorwaertsS` — auch ohne Abklingzeit
        t.vorwaerts = detail::huellkurveVorwaerts(t.pegelDb, einst.schrittS, hoechster, von, bis, einst.vorwaertsS, Abklingen{});
        if (bis <= von || static_cast<double>(bis - von) * einst.schrittS < 0.3) continue;
        t.abklingen = detail::zweiStufen(t.pegelDb, einst.schrittS, von, bis);
        // mit zwei Stufen dazu die Tiefe des Knicks
        t.vorwaerts = detail::huellkurveVorwaerts(t.pegelDb, einst.schrittS, hoechster, von, bis, einst.vorwaertsS, t.abklingen);
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

Abklingkurve schroederKurve(const Teilton& t, double schrittS, std::size_t von, std::size_t ende, double rasterS,
                                   bool bodenAbziehen, double randS)
{
    Abklingkurve k;
    k.rasterS = rasterS;
    ende = std::min(ende, t.pegelDb.size());
    const auto q = static_cast<std::size_t>(std::max(1L, std::lround(rasterS / schrittS)));
    const std::size_t anfang = (von + q - 1) / q * q;
    k.ersterPunkt = anfang / q;
    if (anfang >= ende) return k;
    std::size_t n = ende - anfang;
    std::vector<double> summe(n + 1, 0.0);
    std::size_t letzteEnergie = 0;  // hinter dem letzten Rahmen mit Energie ueber dem Boden ist die Kurve zu Ende
    for (std::size_t j = n; j-- > 0;)
    {
        double e = std::pow(10.0, t.pegelDb[anfang + j] / 10.0);
        if (bodenAbziehen && anfang + j < t.bodenDb.size()) e = std::max(0.0, e - std::pow(10.0, t.bodenDb[anfang + j] / 10.0));
        if (e > 0.0 && letzteEnergie == 0) letzteEnergie = j + 1;
        summe[j] = summe[j + 1] + e;
    }
    if (summe[0] <= 0.0) return k;
    // faellt der Teilton vor dem Ende unter den Boden (am Modell: Teilton 1 unter das Leck eines langsameren Nachbarn,
    // Befund S155 an MIDI 48), endet die Kurve dort — sonst stuende sie danach flach, und ein Vergleich
    // gekuerzter Kurven teilte dort null durch null
    n = letzteEnergie;
    for (std::size_t j = 0; j < n; j += q) k.db.push_back(10.0 * std::log10(std::max(summe[j], 1e-300) / summe[0]));

    // K6-1 a: beide Seiten sind bis zum selben Ende integriert und tragen dieselbe Abschneidung; nur die letzten
    // Punkte mitteln ueber zu wenige Rahmen
    const auto rand = static_cast<std::size_t>(std::lround(randS / schrittS));
    const std::size_t bisGueltig = n > rand ? n - rand : 0;
    k.gueltig = std::min(k.db.size(), (bisGueltig + q - 1) / q);
    return k;
}

Autokovarianz autokovarianz(const Teilton& t, double schrittS, std::size_t von, std::size_t bis, double bisS,
                                   double rasterS, double glattS)
{
    Autokovarianz a;
    a.rasterS = rasterS;
    bis = std::min(bis, t.pegelDb.size());
    if (bis <= von + 2) return a;
    const std::size_t n = bis - von;
    std::vector<double> summe(n + 1, 0.0);
    for (std::size_t j = 0; j < n; ++j) summe[j + 1] = summe[j] + t.pegelDb[von + j];
    const auto halb = static_cast<std::size_t>(std::lround(0.5 * glattS / schrittS));
    std::vector<double> rest(n);
    for (std::size_t j = 0; j < n; ++j)
    {
        // das Mittel in dB, mittig und am Rand symmetrisch gekuerzt: ein Abfall gerade in dB bleibt ohne Rest
        const std::size_t h = std::min({halb, j, n - 1 - j});
        const double mittel = (summe[j + h + 1] - summe[j - h]) / static_cast<double>(2 * h + 1);
        rest[j] = t.pegelDb[von + j] - mittel;
    }
    double mittelwert = 0.0;
    for (const double r : rest) mittelwert += r;
    mittelwert /= static_cast<double>(n);
    for (double& r : rest) r -= mittelwert;
    const auto q = static_cast<std::size_t>(std::max(1L, std::lround(rasterS / schrittS)));
    const auto hoechstens = static_cast<std::size_t>(std::min(bisS / schrittS, 0.5 * static_cast<double>(n)));
    for (std::size_t tau = 0; tau <= hoechstens; tau += q)
    {
        double s = 0.0;
        for (std::size_t j = 0; j + tau < n; ++j) s += rest[j] * rest[j + tau];
        a.db2.push_back(s / static_cast<double>(n - tau));
    }
    return a;
}

std::size_t endeVorBlende(std::span<const double> ton, int rate)
{
    const auto rahmen = static_cast<std::size_t>(0.05 * rate);
    const std::size_t n = rahmen == 0 ? 0 : ton.size() / rahmen;
    if (n < 40) return ton.size();
    std::vector<double> t(n);
    std::vector<double> pegel(n);
    for (std::size_t r = 0; r < n; ++r)
    {
        double s = 0.0;
        for (std::size_t i = r * rahmen; i < (r + 1) * rahmen; ++i) s += ton[i] * ton[i];
        t[r] = static_cast<double>(r);
        pegel[r] = 10.0 * std::log10(std::max(s / static_cast<double>(rahmen), 1e-30));
    }
    const detail::Geraden geraden(t, pegel);
    const detail::Geraden::Gerade g = geraden.ueber(4 * n / 10, 7 * n / 10);
    // von hinten: die groesste Abweichung nach oben ab jeder Stelle
    double hoechste = -1e9;
    std::size_t blende = n;
    for (std::size_t r = n; r-- > 7 * n / 10;)
    {
        hoechste = std::max(hoechste, pegel[r] - (g.achse + g.steigung * t[r]));
        if (hoechste < -3.0) blende = r;
    }
    if (blende == n) return ton.size();
    const std::size_t vorher = blende * rahmen > static_cast<std::size_t>(rate) ? blende * rahmen - static_cast<std::size_t>(rate) : 0;
    return std::max(vorher, 7 * n / 10 * rahmen);
}

double pegelZuBeginn(std::span<const double> ton, int rate, double einsatzS, double dauerS)
{
    const auto von = static_cast<std::size_t>(einsatzS * rate);
    const std::size_t bis = std::min(ton.size(), von + static_cast<std::size_t>(dauerS * rate));
    if (bis <= von) return -300.0;
    double summe = 0.0;
    for (std::size_t i = von; i < bis; ++i) summe += ton[i] * ton[i];
    return detail::db(std::sqrt(2.0 * summe / static_cast<double>(bis - von)));
}

double direktschallDb(std::span<const double> ton, int rate, double einsatzS, double kurzS, double langS)
{
    const auto von = static_cast<std::size_t>(einsatzS * rate);
    const std::size_t mitte = std::min(ton.size(), von + static_cast<std::size_t>(kurzS * rate));
    const std::size_t bis = std::min(ton.size(), von + static_cast<std::size_t>(langS * rate));
    if (mitte <= von || bis <= mitte) return 0.0;
    double vorn = 0.0;
    double hinten = 0.0;
    for (std::size_t i = von; i < mitte; ++i) vorn += ton[i] * ton[i];
    for (std::size_t i = mitte; i < bis; ++i) hinten += ton[i] * ton[i];
    vorn /= static_cast<double>(mitte - von);
    hinten /= static_cast<double>(bis - mitte);
    return 10.0 * std::log10(std::max(vorn, 1e-30) / std::max(hinten, 1e-30));
}

std::vector<double> terzMitten()
{
    std::vector<double> m;
    for (int i = -15; i <= 10; ++i) m.push_back(1000.0 * std::exp2(i / 3.0));
    return m;
}

std::vector<double> terzPegel(std::span<const double> ton, int rate, double einsatzS, const lumi::kern::Fft& fft,
                                     double dauerS)
{
    const auto von = static_cast<std::size_t>(einsatzS * rate);
    const std::size_t laenge = std::min(ton.size() > von ? ton.size() - von : 0, static_cast<std::size_t>(dauerS * rate));
    std::vector<double> pegel(terzMitten().size(), -300.0);
    if (laenge < 256) return pegel;
    const std::size_t n = lumi::kern::zweierpotenzAb(laenge);
    std::vector<double> zeit(n, 0.0);
    for (std::size_t i = 0; i < laenge; ++i)
        zeit[i] = ton[von + i] * (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(laenge - 1)));
    std::vector<std::complex<double>> spektrum(n / 2 + 1);
    fft.vorwaerts(zeit, spektrum);
    const double binHz = static_cast<double>(rate) / static_cast<double>(n);
    const std::vector<double> mitten = terzMitten();
    for (std::size_t b = 0; b < mitten.size(); ++b)
    {
        const auto unten = static_cast<std::size_t>(std::ceil(mitten[b] * std::exp2(-1.0 / 6.0) / binHz));
        const auto oben = static_cast<std::size_t>(std::floor(mitten[b] * std::exp2(1.0 / 6.0) / binHz));
        double leistung = 0.0;
        for (std::size_t i = unten; i <= std::min(oben, spektrum.size() - 1); ++i) leistung += std::norm(spektrum[i]);
        pegel[b] = 10.0 * std::log10(std::max(leistung / static_cast<double>(laenge), 1e-30));
    }
    return pegel;
}

std::vector<Phantom> phantome(std::span<const double> ton, int rate, const Ergebnis& e, const lumi::kern::Fft& fft,
                                     int bisN, double dauerS)
{
    std::vector<Phantom> liste;
    if (!e.erkannt) return liste;
    const auto von = static_cast<std::ptrdiff_t>((e.einsatzS + 0.05) * rate);
    const std::size_t verfuegbar = ton.size() > static_cast<std::size_t>(von) ? ton.size() - static_cast<std::size_t>(von) : 0;
    const std::size_t laenge = std::min(verfuegbar, static_cast<std::size_t>(dauerS * rate));
    if (laenge < 4096) return liste;
    const std::size_t n = 2 * lumi::kern::zweierpotenzAb(laenge);
    detail::Spektrum spektrum(fft, laenge, n);
    spektrum.rechne(ton, von);
    std::vector<double> alle(spektrum.bins());
    for (std::size_t i = 0; i < alle.size(); ++i) alle[i] = spektrum.binDb(i);
    const double median = detail::median(alle);
    const double binHz = static_cast<double>(rate) / static_cast<double>(n);
    const double aufloesung = 4.0 * static_cast<double>(rate) / static_cast<double>(laenge);
    for (int k = 4; k <= bisN; k += 2)
    {
        const Teilton* halb = e.teilton(k / 2);
        const Teilton* quer = e.teilton(k);
        if (halb == nullptr || quer == nullptr) continue;
        const double fp = 2.0 * halb->frequenz;
        if (quer->frequenz - fp < aufloesung) continue;
        const double weite = 0.25 * aufloesung;
        const auto [sp, pp] = spektrum.spitze(static_cast<std::size_t>((fp - weite) / binHz), static_cast<std::size_t>((fp + weite) / binHz));
        const auto [sq, pq] = spektrum.spitze(static_cast<std::size_t>((quer->frequenz - weite) / binHz),
                                              static_cast<std::size_t>((quer->frequenz + weite) / binHz));
        (void)sq;
        if (pp < median + 10.0) continue;
        liste.push_back({k, sp * binHz, pp - pq});
    }
    return liste;
}

}  // namespace lumi::messung

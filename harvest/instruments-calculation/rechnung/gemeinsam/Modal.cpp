/**
 * @file Modal.cpp
 * @brief Gemeinsam: der modale Kern und der Stoss (Rechnung wie im Vorbild `HammerTon.cpp`)
 */
#include "gemeinsam/Modal.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic
{

Mode modeAus(double hz, double t60S, double masseKg, double phiEin, double phiKontakt, double gewichtAus, Ausgang ausgang)
{
    const double omega = 2.0 * std::numbers::pi * hz;
    Mode m;
    m.lambda = Komplex(-sigmaAusT60(t60S), omega);
    m.eingang = Komplex(phiEin / (masseKg * omega), 0.0);
    m.abnahme = phiKontakt;
    m.aus = Komplex(gewichtAus, 0.0);
    if (ausgang == Ausgang::Geschwindigkeit) m.aus *= m.lambda;
    if (ausgang == Ausgang::Beschleunigung) m.aus *= m.lambda * m.lambda;
    return m;
}

ModalBank::ModalBank(std::vector<Mode> moden, double rate) : m_moden(std::move(moden)), m_rate(rate)
{
    richtePole();
}

void ModalBank::richtePole()
{
    m_pol.resize(m_moden.size());
    for (std::size_t i = 0; i < m_moden.size(); ++i) m_pol[i] = std::exp(m_moden[i].lambda / m_rate);
    m_polDaempfer = m_pol;
    m_teilungFein = 0;
}

double ModalBank::auslenkung() const
{
    double y = 0.0;
    for (const Mode& m : m_moden) y += m.abnahme * m.zustand.imag();
    return y;
}

double ModalBank::ausgang() const
{
    double y = 0.0;
    for (const Mode& m : m_moden) y += (m.aus * m.zustand).imag();
    return y;
}

void ModalBank::schritt(double kraft, int teilung)
{
    const double dt = 1.0 / (m_rate * teilung);
    if (teilung == 1)
    {
        for (std::size_t i = 0; i < m_moden.size(); ++i)
            m_moden[i].zustand = m_polDaempfer[i] * (m_moden[i].zustand + m_moden[i].eingang * (kraft * dt));
        return;
    }
    if (m_teilungFein != teilung)
    {
        m_polFein.resize(m_moden.size());
        for (std::size_t i = 0; i < m_moden.size(); ++i) m_polFein[i] = std::exp(m_moden[i].lambda * dt);
        m_teilungFein = teilung;
    }
    for (std::size_t i = 0; i < m_moden.size(); ++i)
        m_moden[i].zustand = m_polFein[i] * (m_moden[i].zustand + m_moden[i].eingang * (kraft * dt));
}

void ModalBank::frei()
{
    for (std::size_t i = 0; i < m_moden.size(); ++i) m_moden[i].zustand *= m_polDaempfer[i];
}

void ModalBank::daempfe(double sigma)
{
    for (std::size_t i = 0; i < m_moden.size(); ++i)
    {
        const Komplex l = m_moden[i].lambda;
        m_polDaempfer[i] = sigma > 0.0 ? std::exp(Komplex(std::min(l.real(), -sigma), l.imag()) / m_rate) : m_pol[i];
    }
}

void ModalBank::klingeAus(std::vector<double>& summe, std::int64_t ab, std::int64_t frames, std::int64_t daempferAb,
                          double sigmaDaempfer) const
{
    if (summe.size() < static_cast<std::size_t>(std::max<std::int64_t>(0, frames))) summe.resize(static_cast<std::size_t>(frames), 0.0);
    double groesste = 0.0;
    for (const Mode& m : m_moden) groesste += std::abs(m.aus * m.zustand);
    const double schwelle = 1e-7 * groesste;
    for (const Mode& m : m_moden)
    {
        Komplex z = m.zustand;
        const Komplex pol = std::exp(m.lambda / m_rate);
        const Komplex polD = sigmaDaempfer > 0.0 ? std::exp(Komplex(std::min(m.lambda.real(), -sigmaDaempfer), m.lambda.imag()) / m_rate) : pol;
        for (std::int64_t i = ab; i < frames; ++i)
        {
            const bool daempfer = daempferAb >= 0 && i >= daempferAb;
            summe[static_cast<std::size_t>(i)] += (m.aus * z).imag();
            z *= daempfer ? polD : pol;
            if ((i & 63) == 0 && std::abs(m.aus * z) < schwelle) break;
        }
    }
}

std::vector<double> impulsantwort(const std::vector<Mode>& moden, double rate, std::int64_t frames)
{
    std::vector<double> h(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0);
    for (const Mode& m : moden)
    {
        const Komplex pol = std::exp(m.lambda / rate);
        Komplex z = pol * m.eingang;  // ein Stoss von 1 N·s im Takt 0, sichtbar ab Takt 1
        for (std::int64_t i = 1; i < frames; ++i)
        {
            h[static_cast<std::size_t>(i)] += (m.aus * z).imag();
            z *= pol;
        }
    }
    return h;
}

std::int64_t nachklangFrames(const std::vector<Mode>& moden, double rate, double db, std::int64_t maxFrames)
{
    double sigma = 0.0;
    bool erste = true;
    for (const Mode& m : moden)
    {
        const double s = -m.lambda.real();
        if (s > 0.0 && (erste || s < sigma)) sigma = s;
        erste = erste && !(s > 0.0);
    }
    if (!(sigma > 0.0)) return std::max<std::int64_t>(1, maxFrames);
    const double t = db / 20.0 * std::log(10.0) / sigma;
    return std::clamp<std::int64_t>(static_cast<std::int64_t>(std::ceil(t * rate)), 1, std::max<std::int64_t>(1, maxFrames));
}

std::array<Komplex, 4> paarSchritt(Komplex ls, Komplex lp, double kappa, double dt)
{
    const Komplex mitte = 0.5 * (ls + lp);
    const Komplex halb = 0.5 * (ls - lp);
    const Komplex d = std::sqrt(halb * halb - kappa * kappa);
    const Komplex e = std::exp(mitte * dt);
    const Komplex ch = std::cosh(d * dt);
    const Komplex sh = std::abs(d) * dt > 1e-8 ? std::sinh(d * dt) / d : Komplex(dt, 0.0);
    const Komplex ik(0.0, -kappa);
    return {e * (ch + sh * halb), e * sh * ik, e * sh * ik, e * (ch - sh * halb)};
}

std::array<Komplex, 4> paarSchrittMit(Komplex ls, Komplex lp, Komplex c, double dt)
{
    const Komplex mitte = 0.5 * (ls + lp);
    const Komplex halb = 0.5 * (ls - lp);
    const Komplex d = std::sqrt(halb * halb + c * c);
    const Komplex e = std::exp(mitte * dt);
    const Komplex ch = std::cosh(d * dt);
    const Komplex sh = std::abs(d) * dt > 1e-8 ? std::sinh(d * dt) / d : Komplex(dt, 0.0);
    return {e * (ch + sh * halb), e * sh * c, e * sh * c, e * (ch - sh * halb)};
}

StossMessung stosse(ModalBank& bank, const Stossanreger& a, double tempo, std::vector<double>& summe, int ueberabtastung,
                    double maxS, double freiS)
{
    StossMessung messung;
    const double fs = bank.rate();
    const double dtFein = 1.0 / (fs * ueberabtastung);
    const auto kontaktMax = std::max<std::int64_t>(1, std::llround(maxS * fs));
    const auto freiNoetig = std::max<std::int64_t>(1, std::llround(freiS * fs * ueberabtastung));
    if (summe.size() < static_cast<std::size_t>(kontaktMax)) summe.resize(static_cast<std::size_t>(kontaktMax), 0.0);
    double kopfY = 0.0;
    double kopfV = tempo;
    double vorigeEindrueckung = 0.0;
    const double eps = std::clamp(a.eps, 0.0, 0.999);
    const double f0 = a.q0 / (1.0 - eps);
    const double kern = a.tauS > 0.0 ? std::exp(-dtFein / a.tauS) : 0.0;
    double gedaechtnis = 0.0;
    std::int64_t frei = 0;
    bool beruehrt = false;
    bool liegtAn = false;
    double erster = 0.0;
    std::int64_t n = 0;
    for (; n < kontaktMax; ++n)
    {
        summe[static_cast<std::size_t>(n)] += bank.ausgang();
        for (int u = 0; u < ueberabtastung; ++u)
        {
            const double druck = (kopfY - bank.auslenkung()) * 1000.0;  // mm
            double kraft = 0.0;
            if (a.huntCrossley)
            {
                const double tempoMm = (druck - vorigeEindrueckung) / dtFein;
                kraft = druck > 0.0 ? std::max(0.0, a.q0 * std::pow(druck, a.p) * (1.0 + a.lambdaHc * tempoMm)) : 0.0;
            }
            else
            {
                const double phi = druck > 0.0 ? std::pow(druck, a.p) : 0.0;
                gedaechtnis = gedaechtnis * kern + phi * a.tauS * (1.0 - kern);
                kraft = std::max(0.0, f0 * (phi - (a.tauS > 0.0 ? eps * gedaechtnis / a.tauS : 0.0)));
            }
            vorigeEindrueckung = druck;
            bank.schritt(kraft, ueberabtastung);
            kopfV -= kraft / a.masseKg * dtFein;
            kopfY += kopfV * dtFein;
            if (kraft > 0.0)
            {
                const double t = (static_cast<double>(n) * ueberabtastung + u) * dtFein;
                if (!beruehrt) erster = t;
                if (!liegtAn) ++messung.kontakte;
                liegtAn = true;
                messung.kontaktS = t - erster;
                messung.kraftSpitzeN = std::max(messung.kraftSpitzeN, kraft);
                beruehrt = true;
                frei = 0;
            }
            else
            {
                liegtAn = false;
                if (beruehrt && kopfV < 0.0) ++frei;
            }
        }
        if (beruehrt && frei >= freiNoetig)
        {
            ++n;
            break;
        }
    }
    messung.frames = n;
    return messung;
}

}  // namespace ic

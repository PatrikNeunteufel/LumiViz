/**
 * @file Effekte.cpp
 * @brief Loop: die Effekte am Clip als Rechenbausteine
 */

#include "loop/Effekte.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

constexpr double kPi = 3.14159265358979323846;
/// Unter diesem Betrag gilt ein Nachklang als verklungen (60 dB).
constexpr double kVerklungen = 0.001;

/// Sehr kleine Werte in den Rueckfuehrungen bremsen den Prozessor aus
/// (Denormale); ein Wert weit unter der Hoerschwelle haelt sie fern.
constexpr float kWinzig = 1.0e-18f;

/// Der Beiwert eines Tiefpasses erster Ordnung fuer eine Grenzfrequenz:
/// `y += beiwert * (x - y)`.
float tiefpassBeiwert(double hertz, int rate)
{
    if (rate <= 0) return 1.0f;
    const double beiwert = 1.0 - std::exp(-2.0 * kPi * hertz / rate);
    return static_cast<float>(std::clamp(beiwert, 0.0, 1.0));
}

// Die Laengen von Freeverb, fuer 44100 Hz; rechts um `kSpreizung` laenger.
constexpr int kKammLaengen[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
constexpr int kAllpassLaengen[] = {556, 441, 341, 225};
constexpr int kSpreizung = 23;
constexpr float kAllpassFeedback = 0.5f;
/// Acht Kaemme addieren sich: die Eingabe kommt so leise hinein, dass der
/// Hall bei vollem Mix in der Groessenordnung der Eingabe bleibt.
constexpr float kHallEingang = 0.015f;
constexpr float kHallAusgang = 3.0f;

}  // namespace

double nachklangSekunden(const ClipEffekte& effekte, double bpm)
{
    // jeder Baustein, der nachklingt, haengt seinen Nachklang an den davor
    double sekunden = 0.0;
    for (const ClipBaustein& baustein : effekte.reihe)
    {
        if (const auto* echo = std::get_if<ClipEcho>(&baustein); echo != nullptr && echo->an && bpm > 0.0)
        {
            const double zeit = echoSchlaege(echo->zeit) * 60.0 / bpm;
            // so viele Wiederholungen, bis feedback^n unter 60 dB liegt
            const double feedback = std::clamp(echo->feedback, 0.0, kEchoFeedbackMax);
            const double wiederholungen =
                feedback > 0.0 ? std::ceil(std::log(kVerklungen) / std::log(feedback)) : 0.0;
            sekunden += zeit * (1.0 + wiederholungen);
        }
        else if (const auto* hall = std::get_if<ClipHall>(&baustein); hall != nullptr && hall->an)
            sekunden += hall->sekunden;
        else if (const auto* filter = std::get_if<ClipFilter>(&baustein); filter != nullptr && filter->an)
        {
            // ein Filter mit Resonanz schwingt nach: um 60 dB gefallen nach ln(1000) * Guete / (pi * Grenze)
            const double guete = 0.70710678118654752 * std::exp2(4.0 * std::clamp(filter->resonanz, 0.0, 1.0));
            sekunden += std::log(1.0 / kVerklungen) * guete / (kPi * std::clamp(filter->hz, kClipFilterHzMin, kClipFilterHzMax));
        }
    }
    return sekunden;
}

// --- Filter ---

void FilterEffekt::richteEin(const ClipFilter& werte, int rate)
{
    m_an = werte.an && rate > 0;
    leere();
    if (!m_an) return;
    // die Formeln des Filters einer Schicht (Instrument.cpp), mit fester Grenze
    constexpr double kGueteOhne = 0.70710678118654752;
    const double resonanz = std::isfinite(werte.resonanz) ? std::clamp(werte.resonanz, 0.0, 1.0) : 0.0;
    const double guete = kGueteOhne * std::exp2(4.0 * resonanz);
    const double hz = std::clamp(std::isfinite(werte.hz) ? werte.hz : kClipFilterHzMax, kClipFilterHzMin,
                                 std::min(kClipFilterHzMax, rate * 0.45));
    const double w = 2.0 * kPi * hz / rate;
    const double alpha = std::sin(w) / (2.0 * guete);
    const double c = std::cos(w);
    const double a0 = 1.0 + alpha;
    m_laut = 1.0;
    switch (werte.art)
    {
    case FilterArt::Hochpass:
        m_b0 = (1.0 + c) / 2.0 / a0;
        m_b1 = -(1.0 + c) / a0;
        m_b2 = m_b0;
        m_laut = 1.0 / std::sqrt(guete / kGueteOhne);
        break;
    case FilterArt::Bandpass:
        m_b0 = alpha / a0;
        m_b1 = 0.0;
        m_b2 = -alpha / a0;
        break;
    case FilterArt::Tiefpass:
    default:
        m_b0 = (1.0 - c) / 2.0 / a0;
        m_b1 = (1.0 - c) / a0;
        m_b2 = m_b0;
        m_laut = 1.0 / std::sqrt(guete / kGueteOhne);
        break;
    }
    m_a1 = -2.0 * c / a0;
    m_a2 = (1.0 - alpha) / a0;
}

void FilterEffekt::leere()
{
    m_zustand = {};
}

void FilterEffekt::rechne(float* stereo, int frames)
{
    if (!m_an) return;
    for (int f = 0; f < frames; ++f)
    {
        for (int seite = 0; seite < 2; ++seite)
        {
            Zustand& z = m_zustand[static_cast<std::size_t>(seite)];
            float& wert = stereo[2 * f + seite];
            const double ein = wert;
            const double aus = m_b0 * ein + m_b1 * z.x1 + m_b2 * z.x2 - m_a1 * z.y1 - m_a2 * z.y2 + kWinzig;
            z.x2 = z.x1;
            z.x1 = ein;
            z.y2 = z.y1;
            z.y1 = aus;
            wert = static_cast<float>(aus * m_laut);
        }
    }
}

// --- Verzerrer ---

void Verzerrer::richteEin(const ClipVerzerrung& werte, int rate)
{
    m_an = werte.an && werte.mix > 0.0 && werte.drive > 0.0;
    // dieselbe Kennlinie wie Drive je Schicht im Instrument (§4f)
    m_staerke = static_cast<float>(8.0 * werte.drive);
    m_norm = m_staerke > 0.0f ? 1.0f / std::tanh(m_staerke) : 1.0f;
    // Tone 0 = 800 Hz, 1 = 18 kHz, dazwischen gleichmaessig in Oktaven
    m_tief = tiefpassBeiwert(800.0 * std::pow(18000.0 / 800.0, werte.ton), rate);
    m_mix = static_cast<float>(werte.mix);
    leere();
}

void Verzerrer::leere()
{
    m_zustand = {};
}

void Verzerrer::rechne(float* stereo, int frames)
{
    if (!m_an) return;
    for (int f = 0; f < frames; ++f)
    {
        for (int seite = 0; seite < 2; ++seite)
        {
            float& wert = stereo[2 * f + seite];
            const float verzerrt = std::tanh(m_staerke * wert) * m_norm;
            float& zustand = m_zustand[static_cast<std::size_t>(seite)];
            zustand += m_tief * (verzerrt - zustand) + kWinzig;
            wert += m_mix * (zustand - wert);
        }
    }
}

// --- Echo ---

void EchoEffekt::richteEin(const ClipEcho& werte, int rate, double bpm)
{
    m_laenge = 0;
    if (rate > 0 && bpm > 0.0)
        m_laenge = std::max(1, static_cast<int>(std::lround(echoSchlaege(werte.zeit) * 60.0 / bpm * rate)));
    m_an = werte.an && werte.mix > 0.0 && m_laenge > 0;
    m_feedback = static_cast<float>(std::clamp(werte.feedback, 0.0, kEchoFeedbackMax));
    // Damping 0 = offen (jede Wiederholung klingt wie die davor), 1 = 1 kHz
    m_daempfung = werte.daempfung <= 0.0
                      ? 1.0f
                      : tiefpassBeiwert(18000.0 * std::pow(1000.0 / 18000.0, werte.daempfung), rate);
    m_mix = static_cast<float>(werte.mix);
    m_speicher.assign(m_an ? 2 * static_cast<std::size_t>(m_laenge) : 0, 0.0f);
    leere();
}

void EchoEffekt::leere()
{
    std::fill(m_speicher.begin(), m_speicher.end(), 0.0f);
    m_stelle = 0;
    m_zustand = {};
}

void EchoEffekt::rechne(float* stereo, int frames)
{
    if (!m_an) return;
    float* speicher = m_speicher.data();
    for (int f = 0; f < frames; ++f)
    {
        for (int seite = 0; seite < 2; ++seite)
        {
            float& wert = stereo[2 * f + seite];
            float& platz = speicher[2 * m_stelle + seite];
            const float echo = platz;  // was vor einer Echo-Zeit hineinging
            float& zustand = m_zustand[static_cast<std::size_t>(seite)];
            zustand += m_daempfung * (echo - zustand) + kWinzig;
            platz = wert + zustand * m_feedback;
            wert += m_mix * (echo - wert);
        }
        if (++m_stelle >= m_laenge) m_stelle = 0;
    }
}

// --- Hall ---

void HallEffekt::richteEin(const ClipHall& werte, int rate)
{
    m_an = werte.an && werte.mix > 0.0 && rate > 0;
    m_mix = static_cast<float>(werte.mix);
    // Damping: der Anteil, den der Tiefpass im Kamm vom alten Wert behaelt
    m_daempfung = static_cast<float>(std::clamp(werte.daempfung, 0.0, 1.0) * 0.8);
    // Size streckt die Laengen: 0 = 0,6-fach, 1 = 1,4-fach
    const double strecke = (0.6 + 0.8 * std::clamp(werte.groesse, 0.0, 1.0)) * (rate > 0 ? rate / 44100.0 : 1.0);
    const double sekunden = std::clamp(werte.sekunden, kHallSekMin, kHallSekMax);
    for (int seite = 0; seite < 2; ++seite)
    {
        for (int i = 0; i < kKaemme; ++i)
        {
            Kamm& kamm = m_kaemme[static_cast<std::size_t>(seite)][static_cast<std::size_t>(i)];
            const int laenge =
                std::max(1, static_cast<int>(std::lround((kKammLaengen[i] + seite * kSpreizung) * strecke)));
            kamm.speicher.assign(m_an ? static_cast<std::size_t>(laenge) : 0, 0.0f);
            // nach `sekunden` um 60 dB gefallen: je Umlauf 10^(-3 * Laenge / (Sekunden * Rate))
            kamm.feedback = m_an ? static_cast<float>(std::pow(10.0, -3.0 * laenge / (sekunden * rate))) : 0.0f;
        }
        for (int i = 0; i < kAllpaesse; ++i)
        {
            Allpass& allpass = m_allpaesse[static_cast<std::size_t>(seite)][static_cast<std::size_t>(i)];
            const int laenge =
                std::max(1, static_cast<int>(std::lround((kAllpassLaengen[i] + seite * kSpreizung) * strecke)));
            allpass.speicher.assign(m_an ? static_cast<std::size_t>(laenge) : 0, 0.0f);
        }
    }
    leere();
}

void HallEffekt::leere()
{
    for (auto& seite : m_kaemme)
        for (Kamm& kamm : seite)
        {
            std::fill(kamm.speicher.begin(), kamm.speicher.end(), 0.0f);
            kamm.stelle = 0;
            kamm.zustand = 0.0f;
        }
    for (auto& seite : m_allpaesse)
        for (Allpass& allpass : seite)
        {
            std::fill(allpass.speicher.begin(), allpass.speicher.end(), 0.0f);
            allpass.stelle = 0;
        }
}

void HallEffekt::rechne(float* stereo, int frames)
{
    if (!m_an) return;
    for (int f = 0; f < frames; ++f)
    {
        // der Raum hoert beide Seiten zusammen
        const float eingang = (stereo[2 * f] + stereo[2 * f + 1]) * kHallEingang + kWinzig;
        for (int seite = 0; seite < 2; ++seite)
        {
            float summe = 0.0f;
            for (Kamm& kamm : m_kaemme[static_cast<std::size_t>(seite)])
            {
                float& platz = kamm.speicher[static_cast<std::size_t>(kamm.stelle)];
                const float aus = platz;
                kamm.zustand = aus * (1.0f - m_daempfung) + kamm.zustand * m_daempfung;
                platz = eingang + kamm.zustand * kamm.feedback;
                if (++kamm.stelle >= static_cast<int>(kamm.speicher.size())) kamm.stelle = 0;
                summe += aus;
            }
            for (Allpass& allpass : m_allpaesse[static_cast<std::size_t>(seite)])
            {
                float& platz = allpass.speicher[static_cast<std::size_t>(allpass.stelle)];
                const float gespeichert = platz;
                platz = summe + gespeichert * kAllpassFeedback;
                if (++allpass.stelle >= static_cast<int>(allpass.speicher.size())) allpass.stelle = 0;
                summe = gespeichert - summe;
            }
            float& wert = stereo[2 * f + seite];
            wert += m_mix * (summe * kHallAusgang - wert);
        }
    }
}

// --- Kette ---

void EffektKette::richteEin(const ClipEffekte& effekte, int rate, double bpm)
{
    m_glieder.clear();
    m_glieder.reserve(effekte.reihe.size());
    for (const ClipBaustein& baustein : effekte.reihe)
    {
        if (const auto* filter = std::get_if<ClipFilter>(&baustein))
            std::get<FilterEffekt>(m_glieder.emplace_back(FilterEffekt{})).richteEin(*filter, rate);
        else if (const auto* verzerrung = std::get_if<ClipVerzerrung>(&baustein))
            std::get<Verzerrer>(m_glieder.emplace_back(Verzerrer{})).richteEin(*verzerrung, rate);
        else if (const auto* echo = std::get_if<ClipEcho>(&baustein))
            std::get<EchoEffekt>(m_glieder.emplace_back(EchoEffekt{})).richteEin(*echo, rate, bpm);
        else if (const auto* hall = std::get_if<ClipHall>(&baustein))
            std::get<HallEffekt>(m_glieder.emplace_back(HallEffekt{})).richteEin(*hall, rate);
    }
    m_an = effekte.einerAn();
}

void EchoEffekt::uebernimm(EchoEffekt& alt)
{
    if (!m_an || !alt.m_an || m_laenge != alt.m_laenge || m_speicher.size() != alt.m_speicher.size()) return;
    m_speicher.swap(alt.m_speicher);
    m_stelle = alt.m_stelle;
    m_zustand = alt.m_zustand;
}

void HallEffekt::uebernimm(HallEffekt& alt)
{
    if (!m_an || !alt.m_an) return;
    for (std::size_t seite = 0; seite < 2; ++seite)
    {
        for (std::size_t i = 0; i < m_kaemme[seite].size(); ++i)
            if (m_kaemme[seite][i].speicher.size() != alt.m_kaemme[seite][i].speicher.size()) return;
        for (std::size_t i = 0; i < m_allpaesse[seite].size(); ++i)
            if (m_allpaesse[seite][i].speicher.size() != alt.m_allpaesse[seite][i].speicher.size()) return;
    }
    for (std::size_t seite = 0; seite < 2; ++seite)
    {
        for (std::size_t i = 0; i < m_kaemme[seite].size(); ++i)
        {
            Kamm& kamm = m_kaemme[seite][i];
            Kamm& vorher = alt.m_kaemme[seite][i];
            kamm.speicher.swap(vorher.speicher);
            kamm.stelle = vorher.stelle;
            kamm.zustand = vorher.zustand;
        }
        for (std::size_t i = 0; i < m_allpaesse[seite].size(); ++i)
        {
            m_allpaesse[seite][i].speicher.swap(alt.m_allpaesse[seite][i].speicher);
            m_allpaesse[seite][i].stelle = alt.m_allpaesse[seite][i].stelle;
        }
    }
}

void EffektKette::uebernimm(EffektKette& alt)
{
    // der n-te Baustein einer Art hier gehoert zum n-ten derselben Art dort
    std::array<std::size_t, std::variant_size_v<Glied>> gesehen{};
    for (Glied& glied : m_glieder)
    {
        const std::size_t art = glied.index();
        std::size_t nummer = gesehen[art]++;
        for (Glied& vorher : alt.m_glieder)
        {
            if (vorher.index() != art) continue;
            if (nummer-- > 0) continue;
            std::visit(
                [&vorher](auto& neu) { neu.uebernimm(std::get<std::remove_reference_t<decltype(neu)>>(vorher)); }, glied);
            break;
        }
    }
}

void EffektKette::leere()
{
    for (Glied& glied : m_glieder)
        std::visit([](auto& effekt) { effekt.leere(); }, glied);
}

void EffektKette::rechne(float* stereo, int frames)
{
    if (!m_an || frames <= 0) return;
    for (Glied& glied : m_glieder)
        std::visit([stereo, frames](auto& effekt) { effekt.rechne(stereo, frames); }, glied);
}

}  // namespace lumi::loop

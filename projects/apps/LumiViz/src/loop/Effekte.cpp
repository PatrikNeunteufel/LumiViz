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
    double sekunden = 0.0;
    if (effekte.echo.an && bpm > 0.0)
    {
        const double zeit = echoSchlaege(effekte.echo.zeit) * 60.0 / bpm;
        // so viele Wiederholungen, bis feedback^n unter 60 dB liegt
        const double feedback = std::clamp(effekte.echo.feedback, 0.0, kEchoFeedbackMax);
        const double wiederholungen =
            feedback > 0.0 ? std::ceil(std::log(kVerklungen) / std::log(feedback)) : 0.0;
        sekunden += zeit * (1.0 + wiederholungen);
    }
    if (effekte.hall.an) sekunden += effekte.hall.sekunden;
    return sekunden;
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
    m_verzerrer.richteEin(effekte.verzerrung, rate);
    m_echo.richteEin(effekte.echo, rate, bpm);
    m_hall.richteEin(effekte.hall, rate);
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
    m_verzerrer.uebernimm(alt.m_verzerrer);
    m_echo.uebernimm(alt.m_echo);
    m_hall.uebernimm(alt.m_hall);
}

void EffektKette::leere()
{
    m_verzerrer.leere();
    m_echo.leere();
    m_hall.leere();
}

void EffektKette::rechne(float* stereo, int frames)
{
    if (!m_an || frames <= 0) return;
    m_verzerrer.rechne(stereo, frames);
    m_echo.rechne(stereo, frames);
    m_hall.rechne(stereo, frames);
}

}  // namespace lumi::loop

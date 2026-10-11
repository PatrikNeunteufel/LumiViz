/**
 * @file Effekte.hpp
 * @brief Loop: die Effekte am Clip als Rechenbausteine — Verzerrer, Echo, Hall
 *        (docs/audio/Loop_Konzept.md §4g, Entscheide Patrik S113: E2 b, G1 b)
 *
 * Nach G1 b rechnen die Effekte eines Clips im Ton-Thread Block fuer Block
 * mit. Hier stehen nur die Bausteine: sie kennen weder den Loop noch die
 * Engine. Eine `EffektKette` ist das, was EIN Clip braucht — Verzerrer, dann
 * Echo, dann Hall, in dieser festen Reihenfolge.
 *
 * Das Gedaechtnis des Loops sitzt hier (der Rechenkern bleibt ohne): Echo und
 * Hall klingen nach. Dieselbe Eingabe ergibt dieselbe Ausgabe, gleich in
 * welchen Bloecken gerechnet wird — so laesst sich alles ohne Ton pruefen.
 *
 * **Im Ton-Thread** erlaubt: `rechne` und `leere` — sie holen keinen Speicher.
 * **Nicht im Ton-Thread:** `richteEin` (legt die Verzoegerungen an).
 */
#pragma once

#include "loop/Loop.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// Wie lange ein Clip mit diesen Effekten nach seinem letzten Ton noch
/// klingt, in Sekunden — bis Echo und Hall um 60 dB gefallen sind. 0, wenn
/// weder Echo noch Hall an ist.
[[nodiscard]] double nachklangSekunden(const ClipEffekte& effekte, double bpm);

/// Verzerrer: formt den Augenblickswert um, danach ein Tiefpass fuer die
/// Klangfarbe. Leises wird lauter, die Spitze nicht: eine Eingabe bis 1
/// bleibt bis 1.
class Verzerrer
{
public:
    void richteEin(const ClipVerzerrung& werte, int rate);
    void leere();
    void rechne(float* stereo, int frames);

private:
    bool m_an = false;
    float m_staerke = 0.0f;  ///< 8 * Drive
    float m_norm = 1.0f;     ///< 1 / tanh(m_staerke)
    float m_tief = 1.0f;     ///< Beiwert des Tiefpasses, 1 = offen
    float m_mix = 1.0f;
    std::array<float, 2> m_zustand{};
};

/// Echo: eine Verzoegerung mit Rueckfuehrung, in der Rueckfuehrung ein
/// Tiefpass (jede Wiederholung wird dunkler). Links und rechts getrennt.
class EchoEffekt
{
public:
    /// `bpm` bestimmt die Laenge: die Zeit ist ein Notenwert.
    void richteEin(const ClipEcho& werte, int rate, double bpm);
    void leere();
    void rechne(float* stereo, int frames);
    /// Die Verzoegerung in Abtastwerten; 0 = aus.
    [[nodiscard]] int verzoegerung() const { return m_an ? m_laenge : 0; }

private:
    bool m_an = false;
    int m_laenge = 0;
    int m_stelle = 0;
    float m_feedback = 0.0f;
    float m_daempfung = 0.0f;
    float m_mix = 0.0f;
    std::vector<float> m_speicher;  ///< Stereo, `2 * m_laenge`
    std::array<float, 2> m_zustand{};
};

/// Hall: acht gedaempfte Kammfilter nebeneinander, danach vier Allpaesse, je
/// Seite — die Bauart von Freeverb (Jezar, gemeinfrei). Die Rueckfuehrung
/// jedes Kamms folgt aus `sekunden`: nach dieser Zeit ist er um 60 dB gefallen.
class HallEffekt
{
public:
    void richteEin(const ClipHall& werte, int rate);
    void leere();
    void rechne(float* stereo, int frames);

private:
    static constexpr int kKaemme = 8;
    static constexpr int kAllpaesse = 4;
    struct Kamm
    {
        std::vector<float> speicher;
        int stelle = 0;
        float feedback = 0.0f;
        float zustand = 0.0f;
    };
    struct Allpass
    {
        std::vector<float> speicher;
        int stelle = 0;
    };

    bool m_an = false;
    float m_daempfung = 0.0f;
    float m_mix = 0.0f;
    std::array<std::array<Kamm, kKaemme>, 2> m_kaemme;
    std::array<std::array<Allpass, kAllpaesse>, 2> m_allpaesse;
};

/// Was ein Clip braucht: Verzerrer, Echo, Hall — in dieser Reihenfolge.
class EffektKette
{
public:
    /// Die Werte uebernehmen und die Verzoegerungen anlegen; das Gedaechtnis
    /// ist danach leer. Holt Speicher — nicht im Ton-Thread.
    void richteEin(const ClipEffekte& effekte, int rate, double bpm);
    /// Echo und Hall vergessen, was sie gehoert haben (nach einem weiten
    /// Sprung der Stelle, G1 b). Holt keinen Speicher.
    void leere();
    /// Einen Block am Ort umrechnen (Stereo, `2 * frames` Werte).
    void rechne(float* stereo, int frames);
    [[nodiscard]] bool an() const { return m_an; }

private:
    bool m_an = false;
    Verzerrer m_verzerrer;
    EchoEffekt m_echo;
    HallEffekt m_hall;
};

}  // namespace lumi::loop

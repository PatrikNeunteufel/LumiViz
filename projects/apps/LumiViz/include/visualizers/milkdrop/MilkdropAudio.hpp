/**
 ****************************************************************************************
 * @file   MilkdropAudio.hpp
 * @brief  Die Audio-Aufbereitung des MilkDrop-Kerns je Frame — Wellenform, Spektrum,
 *         Wellen-Ausrichtung und die Bandenergien fuer bass/mid/treb (Session 80)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Reihenfolge wie im Original (`pluginshell.cpp:1323`, `plugin.cpp:8720`):
 *
 * 1. **AnalyzeNewSound** — PCM → `fWaveform` (−128…127); das Spektrum der
 *    Custom-Waves (`fSpectrum`) aus der GEDAEMPFTEN, noch unverschobenen Welle.
 * 2. **AlignWaves** — verschiebt `fWaveform` so, dass die ersten 480 Samples
 *    moeglichst genau zum Vorframe passen (ruhig stehende Wellen; rekursive
 *    Suche ueber Oktaven, gewichteter Fehler). Custom-Waves mit `bSpectrum=0`
 *    sehen die AUSGERICHTETE Welle.
 * 3. **DoCustomSoundAnalysis** — ein ZWEITES FFT auf der ausgerichteten
 *    LINKEN Welle, ohne Daempfung; `imm[i]` = Summe der Baender im i-ten
 *    Sechstel des Spektrums. Daraus macht MilkLoudness bass/mid/treb (unveraendert).
 *    Achtung Original-Eigenheit: `myfft.Init(576, 512, -1)` — der Kommentar sagt
 *    „UN-NORMALIZED“, aber −1 ist wahr, die Entzerrung laeuft also doch.
 *
 * Befund dahinter: bis S80 speiste LumiViz die Custom-Waves und bass/mid/treb aus
 * dem App-Spektrum (BASS-FFT) mit geschaetzten Faktoren — kalibriert gegen eine
 * Referenz, die selbst falsches PCM bekam (Offene_Punkte §3).
 ****************************************************************************************
 */

#pragma once

#include "visualizers/milkdrop/MilkdropFft.hpp"

#include <array>
#include <cmath>
#include <cstring>

namespace lumi::milkdrop {

class MilkdropAudio
{
public:
    static constexpr int kSamples = 576;
    static constexpr int kWaveSamples = 480;  ///< NUM_WAVEFORM_SAMPLES (defines.h:188)

    /// Ein Frame: rohe Wellenform je Kanal (−1…1, 576 Werte).
    void frame(const float* rohL, const float* rohR)
    {
        const float* roh[2] = {rohL, rohR};
        for (int ch = 0; ch < 2; ++ch)
        {
            MilkdropFft::quantisiere(roh[ch], m_welle[static_cast<std::size_t>(ch)]);
            // 1. Spektrum der Custom-Waves: gedaempft, unverschoben
            m_fft.spektrum(m_welle[static_cast<std::size_t>(ch)],
                           m_spektrum[static_cast<std::size_t>(ch)]);
        }
        // 2. Ausrichtung (veraendert m_welle)
        ausrichten();
        // 3. zweites FFT: ausgerichtete LINKE Welle, ohne Daempfung
        std::array<float, MilkdropFft::kBaender> links{};
        m_fft.spektrumOhneDaempfung(m_welle[0], links);
        for (int i = 0; i < 3; ++i)
        {
            const int start = MilkdropFft::kBaender * i / 6;
            const int ende = MilkdropFft::kBaender * (i + 1) / 6;
            float summe = 0.0f;
            for (int j = start; j < ende; ++j) summe += links[static_cast<std::size_t>(j)];
            m_imm[static_cast<std::size_t>(i)] = summe;
        }
    }

    /// Kaltstart (pluginshell.cpp:1001): Vorframe leer, Versatz 0
    void zuruecksetzen()
    {
        for (auto& w : m_alt) w.fill(0.0f);
        m_altVersatz = {0, 0};
    }

    /// fWaveform nach der Ausrichtung (−128…127) — Quelle der Wellenform-Waves
    [[nodiscard]] const std::array<float, kSamples>& welle(int ch) const
    {
        return m_welle[ch == 0 ? 0 : 1];
    }
    /// fSpectrum — Quelle der Spektrum-Waves
    [[nodiscard]] const std::array<float, MilkdropFft::kBaender>& spektrum(int ch) const
    {
        return m_spektrum[ch == 0 ? 0 : 1];
    }
    /// Bandenergien bass/mid/treb (roh) fuer MilkLoudness
    [[nodiscard]] const std::array<float, 3>& imm() const { return m_imm; }

private:
    /// AlignWaves (pluginshell.cpp:2342), Zeile fuer Zeile
    void ausrichten()
    {
        constexpr int kMaxOktaven = 10;
        const int n = kWaveSamples;
        int oktaven = static_cast<int>(std::floor(std::log(static_cast<float>(kSamples - n)) /
                                                  std::log(2.0f)));
        std::array<int, 2> versatz = {0, 0};
        if (oktaven >= 4)
        {
            if (oktaven > kMaxOktaven) oktaven = kMaxOktaven;
            for (int ch = 0; ch < 2; ++ch)
            {
                auto& neu = m_pyrNeu;  // Member statt static: mehrere Visualizer
                auto& alt = m_pyrAlt;  // rendern parallel
                int spls[kMaxOktaven];
                int platz[kMaxOktaven];
                std::memcpy(neu[0], m_welle[static_cast<std::size_t>(ch)].data(),
                            sizeof(float) * kSamples);
                std::memcpy(alt[0],
                            &m_alt[static_cast<std::size_t>(ch)]
                                  [static_cast<std::size_t>(m_altVersatz[static_cast<std::size_t>(ch)])],
                            sizeof(float) * n);
                spls[0] = kSamples;
                platz[0] = kSamples - n;
                for (int o = 1; o < oktaven; ++o)
                {
                    spls[o] = spls[o - 1] / 2;
                    platz[o] = platz[o - 1] / 2;
                    for (int k = 0; k < spls[o]; ++k)
                    {
                        neu[o][k] = 0.5f * (neu[o - 1][k * 2] + neu[o - 1][k * 2 + 1]);
                        alt[o][k] = 0.5f * (alt[o - 1][k * 2] + alt[o - 1][k * 2 + 1]);
                    }
                }
                if (!m_gewichteBereit)
                {
                    m_gewichteBereit = true;
                    for (int o = 0; o < oktaven; ++o)
                    {
                        const int vergleich = spls[o] - platz[o];
                        for (int k = 0; k < vergleich; ++k)
                        {
                            float g = k < vergleich / 2
                                          ? static_cast<float>(k * 2) / static_cast<float>(vergleich)
                                          : static_cast<float>((vergleich - 1 - k) * 2) /
                                                static_cast<float>(vergleich);
                            g = (g - 0.8f) * 5.0f + 0.8f;
                            if (g > 1) g = 1;
                            if (g < 0) g = 0;
                            m_gewicht[o][k] = g;
                        }
                        int k = 0;
                        while (m_gewicht[o][k] == 0 && k < vergleich) ++k;
                        m_ersteNichtNull[o] = k;
                        k = vergleich - 1;
                        while (m_gewicht[o][k] == 0 && k >= 0) --k;
                        m_letzteNichtNull[o] = k;
                    }
                }
                int n1 = 0;
                int n2 = platz[oktaven - 1];
                for (int o = oktaven - 1; o >= 0; --o)
                {
                    int besterVersatz = -1;
                    float kleinsterFehler = 0;
                    for (int v = n1; v < n2; ++v)
                    {
                        float fehler = 0;
                        for (int i = m_ersteNichtNull[o]; i <= m_letzteNichtNull[o]; ++i)
                        {
                            const float x = (neu[o][i + v] - alt[o][i]) * m_gewicht[o][i];
                            fehler += x > 0 ? x : -x;
                        }
                        if (besterVersatz == -1 || fehler < kleinsterFehler)
                        {
                            besterVersatz = v;
                            kleinsterFehler = fehler;
                        }
                    }
                    if (o > 0)
                    {
                        n1 = besterVersatz * 2 - 1;
                        n2 = besterVersatz * 2 + 2 + 1;
                        if (n1 < 0) n1 = 0;
                        if (n2 > platz[o - 1]) n2 = platz[o - 1];
                    }
                    else
                        versatz[static_cast<std::size_t>(ch)] = besterVersatz;
                }
            }
        }
        // alte Welle = UNverschobene aktuelle (vor dem Anwenden, wie im Original)
        m_alt = m_welle;
        m_altVersatz = versatz;
        for (int ch = 0; ch < 2; ++ch)
        {
            const int v = versatz[static_cast<std::size_t>(ch)];
            if (v <= 0) continue;
            auto& w = m_welle[static_cast<std::size_t>(ch)];
            for (int i = 0; i < n; ++i)
                w[static_cast<std::size_t>(i)] = w[static_cast<std::size_t>(i + v)];
            for (int i = n; i < kSamples; ++i) w[static_cast<std::size_t>(i)] = 0.0f;
        }
    }

    MilkdropFft m_fft;
    std::array<std::array<float, kSamples>, 2> m_welle{};
    std::array<std::array<float, MilkdropFft::kBaender>, 2> m_spektrum{};
    std::array<float, 3> m_imm{};
    std::array<std::array<float, kSamples>, 2> m_alt{};
    std::array<int, 2> m_altVersatz{};
    float m_pyrNeu[10][kSamples]{};
    float m_pyrAlt[10][kSamples]{};
    bool m_gewichteBereit = false;
    float m_gewicht[10][kSamples]{};
    int m_ersteNichtNull[10]{};
    int m_letzteNichtNull[10]{};
};

}  // namespace lumi::milkdrop

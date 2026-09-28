/**
 ****************************************************************************************
 * @file   MilkdropZufall.hpp
 * @brief  Die Zufallsdaten des MilkDrop-Kerns — GEMEINSAME Quelle fuer LumiViz und
 *         MilkdropRef (Session 80): Rausch-Texturen, rand_preset, rot_*-Parameter
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 * @version 1.0.0
 *
 * @details
 * **Warum das geteilt gehoert** (Vorbild `KaltstartSaat.hpp`, S75): Das Original
 * wuerfelt an drei Stellen, die ein Preset sichtbar praegen —
 *
 * - die Rausch-Texturen `noise_lq/lq_lite/mq/hq` und `noisevol_lq/hq`
 *   (`plugin.cpp:2427`, `AddNoiseTex`/`AddNoiseVol`) aus
 *   `srand(QueryPerformanceCounter ^ rand())` — also aus der UHR;
 * - `rand_preset` und die `rot_*`-Parameter (`state.cpp:1875`,
 *   `RandomizePresetVars`) aus `rand()` bei jeder Ladung;
 * - `rand_frame` jeden Frame (`milkdropfs.cpp:779`).
 *
 * Damit ist die Referenz bei Presets, die davon leben, SELBST nicht
 * reproduzierbar — zwei Laeufe zeigen verschiedene Bilder, und ein Vergleich
 * mit uns misst vor allem den Wuerfel. Befund S80 an `martin - stratospheric
 * turbulences` (Tunnel und „Fenster" an anderer Stelle).
 *
 * LumiViz leitet dieselben Groessen deterministisch ab. Die Formeln stehen hier,
 * Qt-frei und C++14, damit `MilkdropRef --zufall-lumiviz` sie nach dem Laden in
 * den Kern schreiben kann — der Kern selbst bleibt unangetastet (Weg wie
 * `--saat`). Danach wuerfeln beide Seiten gleich.
 *
 * **Grenze:** `rand_frame` setzt der Kern MITTEN im Frame neu; der Host kommt da
 * nicht heran. Presets, die `rand_frame` benutzen, bleiben in diesem Punkt
 * unvergleichbar.
 *
 * Der Code ist aus `MilkdropVisualizer.cpp` hierher VERSCHOBEN (S80), nicht neu
 * geschrieben: LumiViz rechnet bitgleich wie zuvor.
 ****************************************************************************************
 */

#ifndef LUMI_MILKDROP_ZUFALL_HPP
#define LUMI_MILKDROP_ZUFALL_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace lumi {
namespace zufall {

/// Startwert des rand_preset-/rot-Generators (vor der ersten Ladung).
constexpr unsigned int kRandSeedInit = 0x9e3779b9u;

/// Ein Schritt des LCG, den LumiViz fuer rand_preset/rot/rand_frame benutzt.
inline unsigned int lcg(unsigned int s)
{
    return s * 1664525u + 1013904223u;
}

/// Zufallszahl 0..1 aus dem LCG (Zustand wird fortgeschrieben).
inline float rand01(unsigned int& s)
{
    s = lcg(s);
    return static_cast<float>(s >> 8) / 16777216.0f;
}

/// Vier Zufallszahlen in der Reihenfolge, die LumiViz bis S80 unbeabsichtigt
/// hatte: der alte Code zog sie als Funktionsargumente
/// (`QVector4D(rand01(s), rand01(s), …)`), und MSVC wie clang (MSVC-ABI)
/// werten Argumente von RECHTS nach links aus — die erste Zahl landete in w.
/// Festgeschrieben, damit bestehende Looks bitgleich bleiben.
inline std::array<float, 4> vierRueckwaerts(unsigned int& s)
{
    const float d = rand01(s);
    const float c = rand01(s);
    const float b = rand01(s);
    const float a = rand01(s);
    return {{a, b, c, d}};
}

/// rand_preset aus dem Ladungs-Seed (LumiViz: m_randSeed nach der Ladung).
inline std::array<float, 4> randPreset(unsigned int seed)
{
    unsigned int rs = seed;
    return vierRueckwaerts(rs);
}

/// Parameter einer rot_*-Matrix (state.cpp:RandomizePresetVars).
struct RotWerte
{
    float base[3];   ///< Startwinkel x/y/z
    float speed[3];  ///< Winkel je Sekunde x/y/z
    float xlate[3];  ///< Verschiebung x/y/z
};

/// Die 20 rot_*-Parametersaetze aus dem Ladungs-Seed. Die Drehgeschwindigkeit
/// waechst mit dem Index — 0.9*(k/8)^3.2 macht rot_s1 praktisch statisch.
inline std::array<RotWerte, 20> rotWerte(unsigned int seed)
{
    std::array<RotWerte, 20> out{};
    unsigned int s = seed;
    const auto frand = [&s]() {
        s = lcg(s);
        return static_cast<float>(s >> 8) / 16777216.0f;
    };
    for (std::size_t k = 0; k < out.size(); ++k)
    {
        const float rotMult = 0.9f * std::pow(static_cast<float>(k) / 8.0f, 3.2f);
        RotWerte& p = out[k];
        for (int a = 0; a < 3; ++a)
        {
            p.xlate[a] = frand() * 2.0f - 1.0f;
            p.base[a] = frand() * 6.28f;
            p.speed[a] = (frand() * 2.0f - 1.0f) * rotMult;
        }
    }
    return out;
}

/// Eine erzeugte Rausch-Textur: RGBA-Bytes, Zeilen (bzw. Slices) hintereinander.
struct Rauschbild
{
    int groesse = 0;               ///< Kantenlaenge (2D: n×n, 3D: n×n×n)
    std::vector<unsigned char> rgba;
};

/// Alle sechs Rausch-Texturen in der Reihenfolge des Originals:
/// [0] noise_lq 256/1 · [1] noise_lq_lite 32/1 · [2] noise_mq 256/4 ·
/// [3] noise_hq 256/8 · [4] noisevol_lq 32³/1 · [5] noisevol_hq 32³/4.
///
/// AddNoiseTex/AddNoiseVol-Port (C2/C3, plugin.cpp:2427-2717): Zufallsbytes im
/// Bereich [RANGE/2, RANGE*1.5) mod 256 (RANGE 216 bei zoom>1, sonst 256), dann
/// fuer zoom>1 kubische Interpolation — X auf den Hauptzeilen, dann Y (und Z)
/// (Catmull-Rom mit Klemme; PORT: dwCubicInterpolate-Naeherung).
inline std::array<Rauschbild, 6> rauschbilder()
{
    std::array<Rauschbild, 6> out;
    unsigned int seed = 0x1234567u;
    const auto nextRand = [&seed]() {
        seed = seed * 1664525u + 1013904223u;
        return seed >> 8;
    };
    const auto catmull = [](int p0, int p1, int p2, int p3, float t) {
        const float y = p1 + 0.5f * t *
                                 (p2 - p0 +
                                  t * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3 +
                                       t * (3.0f * (p1 - p2) + p3 - p0)));
        // (std::min)/(std::max) geklammert: windows.h definiert min/max als
        // Makros, und MilkdropRef bindet beide ein
        return static_cast<unsigned char>(
            (std::min)((std::max)(static_cast<int>(y + 0.5f), 0), 255));
    };
    struct Spec
    {
        int size;
        int zoom;
    };
    const std::array<Spec, 4> specs = {{{256, 1}, {32, 1}, {256, 4}, {256, 8}}};
    for (std::size_t i = 0; i < specs.size(); ++i)
    {
        const int n = specs[i].size;
        const int zoom = specs[i].zoom;
        const unsigned int range = (zoom > 1) ? 216u : 256u;
        std::vector<unsigned char>& pixels = out[i].rgba;
        out[i].groesse = n;
        pixels.resize(static_cast<std::size_t>(n) * n * 4);
        for (unsigned char& p : pixels)
        {
            p = static_cast<unsigned char>((nextRand() % range + range / 2) & 0xFFu);
        }
        if (zoom > 1)
        {
            const auto px = [&](int x, int y, int c) -> unsigned char& {
                return pixels[(static_cast<std::size_t>(y) * n + x) * 4 +
                              static_cast<std::size_t>(c)];
            };
            // X-Pass nur auf den Hauptzeilen (y % zoom == 0)
            for (int y = 0; y < n; y += zoom)
            {
                for (int x = 0; x < n; ++x)
                {
                    if (x % zoom == 0) continue;
                    const int baseX = (x / zoom) * zoom + n;
                    const float t = static_cast<float>(x % zoom) / zoom;
                    for (int c = 0; c < 4; ++c)
                    {
                        px(x, y, c) = catmull(px((baseX - zoom) % n, y, c),
                                              px(baseX % n, y, c),
                                              px((baseX + zoom) % n, y, c),
                                              px((baseX + zoom * 2) % n, y, c), t);
                    }
                }
            }
            // Y-Pass auf allen Spalten
            for (int x = 0; x < n; ++x)
            {
                for (int y = 0; y < n; ++y)
                {
                    if (y % zoom == 0) continue;
                    const int baseY = (y / zoom) * zoom + n;
                    const float t = static_cast<float>(y % zoom) / zoom;
                    for (int c = 0; c < 4; ++c)
                    {
                        px(x, y, c) = catmull(px(x, (baseY - zoom) % n, c),
                                              px(x, baseY % n, c),
                                              px(x, (baseY + zoom) % n, c),
                                              px(x, (baseY + zoom * 2) % n, c), t);
                    }
                }
            }
        }
    }

    // Volumen: 32^3, zoom 1 (LQ) und 4 (HQ); gleicher Zufallsstrom wie oben.
    // Kubische Glaettung fuer zoom>1: X auf den Hauptzeilen der Haupt-Slices,
    // Y auf den Haupt-Slices, Z ueberall.
    const std::array<Spec, 2> volSpecs = {{{32, 1}, {32, 4}}};
    for (std::size_t i = 0; i < volSpecs.size(); ++i)
    {
        const int n = volSpecs[i].size;
        const int zoom = volSpecs[i].zoom;
        const unsigned int range = (zoom > 1) ? 216u : 256u;
        std::vector<unsigned char>& voxels = out[4 + i].rgba;
        out[4 + i].groesse = n;
        voxels.resize(static_cast<std::size_t>(n) * n * n * 4);
        for (unsigned char& p : voxels)
        {
            p = static_cast<unsigned char>((nextRand() % range + range / 2) & 0xFFu);
        }
        if (zoom > 1)
        {
            const auto vx = [&](int x, int y, int z, int c) -> unsigned char& {
                return voxels[((static_cast<std::size_t>(z) * n + y) * n + x) * 4 +
                              static_cast<std::size_t>(c)];
            };
            for (int z = 0; z < n; z += zoom)
                for (int y = 0; y < n; y += zoom)
                    for (int x = 0; x < n; ++x)
                    {
                        if (x % zoom == 0) continue;
                        const int baseX = (x / zoom) * zoom + n;
                        const float t = static_cast<float>(x % zoom) / zoom;
                        for (int c = 0; c < 4; ++c)
                        {
                            vx(x, y, z, c) = catmull(vx((baseX - zoom) % n, y, z, c),
                                                     vx(baseX % n, y, z, c),
                                                     vx((baseX + zoom) % n, y, z, c),
                                                     vx((baseX + zoom * 2) % n, y, z, c), t);
                        }
                    }
            for (int z = 0; z < n; z += zoom)
                for (int x = 0; x < n; ++x)
                    for (int y = 0; y < n; ++y)
                    {
                        if (y % zoom == 0) continue;
                        const int baseY = (y / zoom) * zoom + n;
                        const float t = static_cast<float>(y % zoom) / zoom;
                        for (int c = 0; c < 4; ++c)
                        {
                            vx(x, y, z, c) = catmull(vx(x, (baseY - zoom) % n, z, c),
                                                     vx(x, baseY % n, z, c),
                                                     vx(x, (baseY + zoom) % n, z, c),
                                                     vx(x, (baseY + zoom * 2) % n, z, c), t);
                        }
                    }
            for (int x = 0; x < n; ++x)
                for (int y = 0; y < n; ++y)
                    for (int z = 0; z < n; ++z)
                    {
                        if (z % zoom == 0) continue;
                        const int baseZ = (z / zoom) * zoom + n;
                        const float t = static_cast<float>(z % zoom) / zoom;
                        for (int c = 0; c < 4; ++c)
                        {
                            vx(x, y, z, c) = catmull(vx(x, y, (baseZ - zoom) % n, c),
                                                     vx(x, y, baseZ % n, c),
                                                     vx(x, y, (baseZ + zoom) % n, c),
                                                     vx(x, y, (baseZ + zoom * 2) % n, c), t);
                        }
                    }
        }
    }
    return out;
}

}  // namespace zufall
}  // namespace lumi

#endif  // LUMI_MILKDROP_ZUFALL_HPP

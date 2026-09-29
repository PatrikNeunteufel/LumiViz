/**
 ****************************************************************************************
 * @file   test_MilkdropFft.cpp
 * @brief  MilkdropFft (Port von vis_milk2/fft.cpp) gegen die Werte des Original-Kerns
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * Sollwerte: MilkdropFftSoll.inc — Wellenform und Spektrum, wie der Original-Kern
 * sie fuer einen Frame berechnet hat (MilkdropRef, MILKDROPREF_AUDIO_DUMP).
 * Dieselbe Wellenform hinein muss dasselbe Spektrum ergeben.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/milkdrop/MilkdropFft.hpp"
#include "visualizers/milkdrop/MilkdropPresetState.hpp"
#include "visualizers/milkdrop/MilkdropSerializer.hpp"

#include <algorithm>
#include <cmath>

#include "MilkdropFftSoll.inc"

using lumi::milkdrop::MilkdropFft;

TEST_CASE("MilkdropFft: dieselbe Wellenform ergibt das Spektrum des Original-Kerns")
{
    std::array<float, 576> welle{};
    for (int i = 0; i < 576; ++i) welle[static_cast<std::size_t>(i)] = kSollWelle[i];
    std::array<float, 512> spek{};
    MilkdropFft fft;
    fft.spektrum(welle, spek);

    float maxAbs = 0.0f;
    float maxSoll = 0.0f;
    for (int i = 0; i < 512; ++i)
    {
        maxAbs = std::max(maxAbs, std::abs(spek[static_cast<std::size_t>(i)] - kSollSpektrum[i]));
        maxSoll = std::max(maxSoll, std::abs(kSollSpektrum[i]));
    }
    INFO("groesste Abweichung " << maxAbs << " bei Spitze " << maxSoll);
    // float wie im Original; Reste nur aus Rundung/Reihenfolge der Bibliothek
    CHECK(maxAbs <= maxSoll * 1e-4f);
}

TEST_CASE("MilkdropFft: Quantisierung wie Winamp — lround(x*127), Zweierkomplement-Bereich")
{
    float roh[576];
    for (int i = 0; i < 576; ++i) roh[i] = 0.0f;
    roh[0] = 0.5f;
    roh[1] = -0.5f;
    roh[2] = 1.5f;   // uebersteuert -> geklemmt
    roh[3] = -1.5f;
    std::array<float, 576> w8{};
    MilkdropFft::quantisiere(roh, w8);
    CHECK(w8[0] == 64.0f);
    CHECK(w8[1] == -64.0f);
    CHECK(w8[2] == 127.0f);
    CHECK(w8[3] == -127.0f);
    CHECK(w8[4] == 0.0f);
}

TEST_CASE("MilkdropFft: Schalter Audio — Standard Original, Speichern/Laden")
{
    lumi::milkdrop::PresetState a;
    CHECK(a.audioOriginal);  // Entscheid Patrik a (S80)
    QJsonObject alt = lumi::milkdrop::presetToJson({});
    QJsonObject inner = alt.value("preset").toObject();
    inner.remove("audioOriginal");
    alt.insert("preset", inner);
    CHECK(lumi::milkdrop::presetFromJson(alt, nullptr).audioOriginal);  // Altbestand
    a.audioOriginal = !a.audioOriginal;
    CHECK(lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson(a), nullptr)
              .audioOriginal == a.audioOriginal);
}

TEST_CASE("Milkdrop: Schalter Seitenverhaeltnis (Waves) — Standard Original, Speichern/Laden")
{
    lumi::milkdrop::PresetState a;
    CHECK(a.aspektOriginal);  // Entscheid Patrik a (S81)
    QJsonObject alt = lumi::milkdrop::presetToJson({});
    QJsonObject inner = alt.value("preset").toObject();
    inner.remove("aspektOriginal");
    alt.insert("preset", inner);
    CHECK(lumi::milkdrop::presetFromJson(alt, nullptr).aspektOriginal);  // Altbestand
    a.aspektOriginal = false;
    CHECK_FALSE(lumi::milkdrop::presetFromJson(lumi::milkdrop::presetToJson(a), nullptr)
                    .aspektOriginal);
}

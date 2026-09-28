/**
 ****************************************************************************************
 * @file   test_MilkdropAudio.cpp
 * @brief  MilkdropAudio gegen den Original-Kern: Wellen-Ausrichtung (AlignWaves) und
 *         die Bandenergien fuer bass/mid/treb (DoCustomSoundAnalysis) ueber eine
 *         Folge von Frames
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * Sollwerte: MilkdropAudioSoll.inc (MilkdropRef, MILKDROPREF_AUDIO_SEQ). Die
 * Ausrichtung haengt am Vorframe — deshalb wird die ganze Folge nachgespielt.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/milkdrop/MilkdropAudio.hpp"

#include <algorithm>
#include <cmath>

#include "MilkdropAudioSoll.inc"

using lumi::milkdrop::MilkdropAudio;

namespace
{
/// Die Referenz gab lround(x*127) weiter; x = pcm/127 fuehrt exakt dorthin zurueck.
MilkdropAudio nachgespielt()
{
    MilkdropAudio audio;
    float l[576];
    float r[576];
    for (int f = 0; f < kSeqFrames; ++f)
    {
        for (int i = 0; i < 576; ++i)
        {
            l[i] = static_cast<float>(kSeqPcm[f][0][i]) / 127.0f;
            r[i] = static_cast<float>(kSeqPcm[f][1][i]) / 127.0f;
        }
        audio.frame(l, r);
    }
    return audio;
}
}  // namespace

TEST_CASE("MilkdropAudio: ausgerichtete Wellenform wie der Original-Kern (AlignWaves)")
{
    const MilkdropAudio audio = nachgespielt();
    for (int ch = 0; ch < 2; ++ch)
    {
        int abweichend = 0;
        for (int i = 0; i < 576; ++i)
            if (audio.welle(ch)[static_cast<std::size_t>(i)] != kSollAusgerichtet[ch][i])
                ++abweichend;
        INFO("Kanal " << ch);
        CHECK(abweichend == 0);
    }
}

TEST_CASE("MilkdropAudio: Bandenergien imm (Quelle von bass/mid/treb) wie der Original-Kern")
{
    const MilkdropAudio audio = nachgespielt();
    for (int i = 0; i < 3; ++i)
    {
        INFO("Band " << i << ": " << audio.imm()[static_cast<std::size_t>(i)] << " gegen "
                     << kSollImm[i]);
        CHECK(audio.imm()[static_cast<std::size_t>(i)] ==
              doctest::Approx(kSollImm[i]).epsilon(1e-5));
    }
}

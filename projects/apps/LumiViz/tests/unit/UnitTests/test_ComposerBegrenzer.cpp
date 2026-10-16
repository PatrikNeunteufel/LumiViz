/**
 ****************************************************************************************
 * @file   test_ComposerBegrenzer.cpp
 * @brief  Composer, Lautstaerke-Abgleich G4: der Limiter als Baustein
 *         (audio/Begrenzer, docs/ui/Composer_Konzept.md §9g, L4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung. Geprueft wird: unter der Decke
 * Bit fuer Bit gleich, um die Vorschau verzoegert; darueber nie ueber der
 * Decke; beide Kanaele gleich begrenzt; nach einer Spitze zurueck auf genau 1;
 * die Verstaerkung springt nicht (S130, L2-1 b); stueckweise wie am Stueck.
 * Der Limiter an der echten Engine steht in test_ComposerNahtMixer.cpp.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/Begrenzer.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#include <vector>

namespace
{

constexpr int kRate = 44100;

/// Stereo-Sinus, links mit `amplitudeL`, rechts mit `amplitudeR`.
std::vector<float> sinus(double sekunden, double amplitudeL, double amplitudeR, double hz = 440.0)
{
    const auto frames = static_cast<std::size_t>(sekunden * kRate);
    std::vector<float> aus(frames * 2);
    for (std::size_t i = 0; i < frames; ++i)
    {
        const double s = std::sin(2.0 * std::numbers::pi * hz * static_cast<double>(i) / kRate);
        aus[2 * i] = static_cast<float>(amplitudeL * s);
        aus[2 * i + 1] = static_cast<float>(amplitudeR * s);
    }
    return aus;
}

float spitze(const std::vector<float>& werte, std::size_t ab = 0)
{
    float s = 0.0f;
    for (std::size_t i = ab; i < werte.size(); ++i) s = std::max(s, std::abs(werte[i]));
    return s;
}

}  // namespace

TEST_SUITE("ComposerBegrenzer")
{
    TEST_CASE("Begrenzer: unter der Decke Bit fuer Bit gleich, nur rund 5 ms spaeter")
    {
        std::vector<float> ton = sinus(2.0, 0.9, 0.7);
        ton[1000] = Begrenzer::kDecke;  // genau auf der Decke greift er nicht
        const std::vector<float> vorher = ton;
        Begrenzer b(kRate);
        const std::size_t v = b.verzoegerungFrames();
        CHECK(v >= static_cast<std::size_t>(0.004 * kRate));
        CHECK(v <= static_cast<std::size_t>(0.006 * kRate));
        b.verarbeite(ton.data(), ton.size() / 2);
        // vorn Stille, danach der Ton um v Frames versetzt, Bit fuer Bit
        CHECK(spitze(std::vector<float>(ton.begin(), ton.begin() + static_cast<std::ptrdiff_t>(2 * v))) == 0.0f);
        CHECK(std::memcmp(ton.data() + 2 * v, vorher.data(), (ton.size() - 2 * v) * sizeof(float)) == 0);
        CHECK(b.verstaerkung() == 1.0);
        CHECK(b.begrenzteFrames() == 0);
    }

    TEST_CASE("Begrenzer: darueber nie ueber der Decke, beide Kanaele gleich, danach zurueck auf genau 1")
    {
        // eine Sekunde +6 dB (Amplitude 2), dann drei Sekunden −6 dB (0,5) — der
        // Rueckweg auf genau 1 dauert bei 80 ms rund 1,1 s (ln 10⁶ · 80 ms)
        std::vector<float> laut = sinus(1.0, 2.0, 1.0);
        const std::vector<float> leise = sinus(3.0, 0.5, 0.5);
        std::vector<float> ton = laut;
        ton.insert(ton.end(), leise.begin(), leise.end());
        const std::vector<float> vorher = ton;
        Begrenzer b(kRate);
        const std::size_t v = b.verzoegerungFrames();
        b.verarbeite(ton.data(), ton.size() / 2);

        CHECK(spitze(ton) <= Begrenzer::kDecke + 1e-6f);
        CHECK(b.begrenzteFrames() > 0);
        // beide Kanaele mit demselben Faktor: das Verhaeltnis links zu rechts bleibt
        for (std::size_t i = 0; i < laut.size() / 2; i += 997)
            if (std::abs(vorher[2 * i]) > 0.1f)
                CHECK(ton[2 * (i + v) + 1] / ton[2 * (i + v)] == doctest::Approx(0.5).epsilon(1e-5));
        // die letzte Sekunde (je Frame zwei Werte): wieder Bit fuer Bit gleich, die Verstaerkung genau 1
        const std::size_t ab = vorher.size() - static_cast<std::size_t>(kRate) * 2;
        CHECK(std::memcmp(ton.data() + ab, vorher.data() + ab - 2 * v, (vorher.size() - ab) * sizeof(float)) == 0);
        CHECK(b.verstaerkung() == 1.0);
        MESSAGE("begrenzte Frames: " << b.begrenzteFrames() << " von " << ton.size() / 2);
    }

    TEST_CASE("Begrenzer: die Verstaerkung springt nicht (Sichttest 11, L2)")
    {
        // Stille, dann schlagartig +12 dB (Amplitude 4) bei 100 Hz, dann wieder Stille:
        // ohne Vorschau sprang die Verstaerkung in einem Frame von 1 auf rund 0,25 —
        // das knackte. Jetzt sinkt sie ueber die Vorschau.
        std::vector<float> ton(static_cast<std::size_t>(kRate) / 10 * 2, 0.0f);
        const std::vector<float> schlag = sinus(0.5, 4.0, 4.0, 100.0);
        ton.insert(ton.end(), schlag.begin(), schlag.end());
        ton.resize(ton.size() + static_cast<std::size_t>(kRate) * 2 * 2, 0.0f);  // zwei Sekunden Stille
        Begrenzer b(kRate);
        double vorher = 1.0;
        double groessterSprung = 0.0;
        for (std::size_t i = 0; i < ton.size() / 2; ++i)
        {
            b.verarbeite(ton.data() + 2 * i, 1);
            groessterSprung = std::max(groessterSprung, std::abs(b.verstaerkung() - vorher));
            vorher = b.verstaerkung();
        }
        MESSAGE("groesster Sprung der Verstaerkung je Frame: " << groessterSprung);
        CHECK(spitze(ton) <= Begrenzer::kDecke + 1e-6f);
        CHECK(groessterSprung < 0.01);
        CHECK(b.verstaerkung() == 1.0);
    }

    TEST_CASE("Begrenzer: stueckweise wie am Stueck, zuruecksetzen")
    {
        std::vector<float> ganz = sinus(1.5, 1.6, 1.2, 100.0);
        std::vector<float> stueckweise = ganz;
        Begrenzer a(kRate);
        a.verarbeite(ganz.data(), ganz.size() / 2);
        Begrenzer b(kRate);
        std::size_t frame = 0;
        for (std::size_t happen = 7; frame < stueckweise.size() / 2; happen = happen * 5 % 3001 + 1)
        {
            const std::size_t n = std::min(happen, stueckweise.size() / 2 - frame);
            b.verarbeite(stueckweise.data() + 2 * frame, n);
            frame += n;
        }
        CHECK(std::memcmp(ganz.data(), stueckweise.data(), ganz.size() * sizeof(float)) == 0);
        CHECK(a.begrenzteFrames() == b.begrenzteFrames());
        b.zuruecksetzen();
        CHECK(b.verstaerkung() == 1.0);
    }
}

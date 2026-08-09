/**
 ****************************************************************************************
 * @file   BeatEstimator.hpp
 * @brief  Predictive BPM estimator — port of the AVS 2.81 "smart beat" logic
 *         (ref/vis_avs/avs/vis_avs/bpm.cpp, BSD-3, Copyright 2005 Nullsoft, Inc.)
 *
 * @author LumiPulse Team (port); original algorithm: Nullsoft, Inc. (BSD-3)
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Import-Fundament-Entwurf §4 (Roadmap 4.4, decision E3). Sits BEHIND an onset
 * detector (BeatModule): feed refine(onsetBeat, nowMs) once per frame; the
 * return value is the refined/predicted beat. Learns the BPM from an 8-entry
 * beat history with confidence scoring, half/double-beat correction, sticky
 * lock-in, and keeps predicting through audio fade-outs.
 *
 * Port notes (deliberate deviations from the original, all behavior-neutral in
 * intent): time comes from the caller as monotonic milliseconds (testable, no
 * GetTickCount); the unused second history (TCHist2) and all dialog/Winamp
 * coupling are dropped; three original defects are straightened out and marked
 * inline (uninitialized resync BPM, leftover interval accumulator in the
 * second averaging layer, sqrt of a negative variance).
 *
 * Threading: plain object, one thread at a time (render-thread owned).
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace lumi::modules {

/**
 * @class BeatEstimator
 * @brief AVS bpm.cpp smart-beat: BPM learning + prediction behind an onset feed
 */
class BeatEstimator
{
public:
    static constexpr int kMinBpm = 60;    // MIN_BPM
    static constexpr int kMaxBpm = 170;   // MAX_BPM

    struct Config
    {
        bool sticky = true;       ///< lock the BPM once confidence stays high
        bool onlySticky = false;  ///< refine output only after lock-in
    };

    explicit BeatEstimator(std::int64_t nowMs = 0) { reset(nowMs); }

    /// @brief Monotonic wall clock in ms (convenience for live feeding)
    [[nodiscard]] static std::int64_t steadyNowMs()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
            .count();
    }

    void setConfig(const Config& config) { m_config = config; }
    [[nodiscard]] const Config& config() const { return m_config; }

    // =========================================================================
    // Feed (once per frame)
    // =========================================================================

    /**
     * @brief Refine the onset beat of this frame (port of refineBeat)
     * @param isBeat Onset detector output for this frame
     * @param nowMs  Monotonic time in milliseconds
     * @return Refined beat: predictions replace onsets once a BPM is locked in
     */
    bool refine(bool isBeat, std::int64_t nowMs);

    // =========================================================================
    // State
    // =========================================================================

    [[nodiscard]] int bpm() const { return m_predictionBpm; }        ///< 0 = learning
    [[nodiscard]] int confidence() const { return m_confidence; }    ///< 0..100
    [[nodiscard]] bool sticked() const { return m_sticked != 0; }

    /// @brief Manual stick/unstick (original dialog buttons)
    void setSticked(bool sticked)
    {
        m_sticked = sticked ? 1 : 0;
        m_stickyConfidenceCount = 0;
    }

    /// @brief Track change: halve best confidence, unstick, optionally relearn
    void notifyTrackChanged(std::int64_t nowMs, bool resetLearning = true)
    {
        m_bestConfidence = m_bestConfidence / 2;
        m_sticked = 0;
        m_stickyConfidenceCount = 0;
        if (resetLearning) reset(nowMs);
    }

    /// @brief Full reset (port of ResetAdapt/initBpm)
    void reset(std::int64_t nowMs)
    {
        m_hist.fill(Beat{});
        m_smoother.fill(0);
        m_halfDiscriminated.fill(0);
        m_hdPos = 0;
        m_avg = 0;
        m_bpm = 0;
        m_lastBpm = 0;
        m_smPtr = 0;
        m_insertionCount = 0;
        m_predictionLastTC = 0;
        m_predictionBpm = 0;
        m_confidence = 0;
        m_bestConfidence = 0;
        m_forceNewBeat = false;
        m_topConfidenceCount = 0;
        m_stickyConfidenceCount = 0;
        m_halfCount = 0;
        m_doubleCount = 0;
        m_doResync = false;
        m_sticked = 0;
        m_lastTC = nowMs;
    }

private:
    static constexpr int kHistSize = 8;
    static constexpr int kSmootherSize = 8;
    static constexpr int kOffIMax = 8;
    static constexpr int kTopConfAdopt = 8;       // TOP_CONF_ADOPT
    static constexpr int kMinSticky = 8;          // MIN_STICKY
    static constexpr int kStickyThreshold = 70;   // STICKY_THRESHOLD
    static constexpr int kStickyThresholdLow = 85;// STICKY_THRESHOLD_LOW

    enum BeatKind : int
    {
        kNone = 0,
        kReal = 1,     // BEAT_REAL
        kGuessed = 2   // BEAT_GUESSED
    };

    struct Beat
    {
        std::int64_t tc = 0;
        int type = kNone;
    };

    [[nodiscard]] bool readyToLearn() const
    {
        for (const Beat& b : m_hist)
        {
            if (b.tc == 0) return false;
        }
        return true;
    }

    [[nodiscard]] bool readyToGuess() const
    {
        return m_insertionCount == kHistSize * 2;
    }

    void insertHist(std::int64_t tc, int type)
    {
        if (m_insertionCount < kHistSize * 2) ++m_insertionCount;
        for (int i = kHistSize - 1; i > 0; --i) m_hist[static_cast<std::size_t>(i)] = m_hist[static_cast<std::size_t>(i - 1)];
        m_hist[0] = Beat{tc, type};
    }

    void newBpm(int thisBpm)
    {
        m_smoother[static_cast<std::size_t>(m_smPtr++)] = thisBpm;
        m_smPtr %= kSmootherSize;
    }

    [[nodiscard]] int smoothedBpm() const
    {
        int sum = 0;
        int n = 0;
        for (int v : m_smoother)
        {
            if (v > 0)
            {
                sum += v;
                ++n;
            }
        }
        return n != 0 ? sum / n : 0;
    }

    /// Port of doubleBeat: compress history intervals, double the BPM
    void doubleBeat()
    {
        if (m_sticked != 0 && m_bpm > kMinBpm) return;
        std::array<std::int64_t, kHistSize> iv{};
        for (int i = 0; i < kHistSize - 1; ++i)
            iv[static_cast<std::size_t>(i)] = m_hist[static_cast<std::size_t>(i)].tc - m_hist[static_cast<std::size_t>(i + 1)].tc;
        for (int i = 1; i < kHistSize; ++i)
            m_hist[static_cast<std::size_t>(i)].tc = m_hist[static_cast<std::size_t>(i - 1)].tc - iv[static_cast<std::size_t>(i - 1)] / 2;
        m_avg /= 2;
        m_bpm *= 2;
        m_doubleCount = 0;
        m_smoother.fill(0);
        m_halfDiscriminated.fill(0);
    }

    /// Port of halfBeat: stretch history intervals, halve the BPM
    void halfBeat()
    {
        if (m_sticked != 0 && m_bpm < kMinBpm) return;
        std::array<std::int64_t, kHistSize> iv{};
        for (int i = 0; i < kHistSize - 1; ++i)
            iv[static_cast<std::size_t>(i)] = m_hist[static_cast<std::size_t>(i)].tc - m_hist[static_cast<std::size_t>(i + 1)].tc;
        for (int i = 1; i < kHistSize; ++i)
            m_hist[static_cast<std::size_t>(i)].tc = m_hist[static_cast<std::size_t>(i - 1)].tc - iv[static_cast<std::size_t>(i - 1)] * 2;
        m_avg *= 2;
        m_bpm /= 2;
        m_halfCount = 0;
        m_smoother.fill(0);
        m_halfDiscriminated.fill(0);
    }

    /// Port of TCHistStep (single-history variant)
    bool histStep(std::int64_t tc, int type)
    {
        const bool learning = readyToLearn();
        const auto thisLen = static_cast<double>(tc - m_lastTC);
        const auto avg = static_cast<double>(m_avg);

        // Sooner than half the average minus 20%: reject (or replace the head
        // when the replacement fits the average better)
        if (thisLen < avg / 2.0 - avg * 0.2)
        {
            if (learning)
            {
                const std::int64_t viaNew = m_avg - (tc - m_hist[1].tc);
                const std::int64_t viaOld = m_avg - (m_hist[0].tc - m_hist[1].tc);
                if (std::llabs(viaNew) < std::llabs(viaOld))
                {
                    m_hist[0] = Beat{tc, type};
                    return true;
                }
            }
            return false;
        }

        // Beat lands on 1/2, 1/3 ... 1/offIMax of the current period: discriminate
        if (learning)
        {
            for (int offI = 2; offI < kOffIMax; ++offI)
            {
                const double sub = avg / offI;
                if (std::fabs(sub - thisLen) < sub * 0.2)
                {
                    m_halfDiscriminated[static_cast<std::size_t>(m_hdPos++)] = 1;
                    m_hdPos %= kHistSize;
                    return false;
                }
            }
        }

        m_halfDiscriminated[static_cast<std::size_t>(m_hdPos++)] = 0;
        m_hdPos %= kHistSize;
        m_lastTC = tc;
        insertHist(tc, type);
        return true;
    }

    /// Port of CalcBPM
    void calcBpm()
    {
        if (!readyToLearn()) return;

        std::int64_t totalTC = 0;
        for (int i = 0; i < kHistSize - 1; ++i)
            totalTC += m_hist[static_cast<std::size_t>(i)].tc - m_hist[static_cast<std::size_t>(i + 1)].tc;
        m_avg = totalTC / (kHistSize - 1);

        int real = 0;
        for (const Beat& b : m_hist)
        {
            if (b.type == kReal) ++real;
        }
        const float rC = std::min(static_cast<float>(real) / kHistSize * 2.0f, 1.0f);

        // Typical drift (standard deviation of the intervals)
        std::int64_t mx = 0;
        double sc = 0.0;
        for (int i = 0; i < kHistSize - 1; ++i)
        {
            const std::int64_t v = m_hist[static_cast<std::size_t>(i)].tc - m_hist[static_cast<std::size_t>(i + 1)].tc;
            mx = std::max(mx, v);
            sc += static_cast<double>(v) * static_cast<double>(v);
        }
        // Original computes sqrt(sc/(n-1) - Avg^2) unguarded — integer
        // truncation can push the operand slightly negative: clamp to 0.
        const double variance =
            std::max(0.0, sc / (kHistSize - 1) -
                              static_cast<double>(m_avg) * static_cast<double>(m_avg));
        const auto et = static_cast<float>(std::sqrt(variance));
        const float etC = mx > 0 ? 1.0f - et / static_cast<float>(mx) : 0.0f;

        m_confidence = std::max(0, static_cast<int>(rC * etC * 100.0f - 50.0f) * 2);

        // Second layer: average only the intervals within the typical drift.
        // (Original re-used the loop variable `v` uninitialized here — the
        // accumulator starts at 0 in this port.)
        totalTC = 0;
        int totalN = 0;
        std::int64_t acc = 0;
        for (int i = 0; i < kHistSize - 1; ++i)
        {
            acc += m_hist[static_cast<std::size_t>(i)].tc - m_hist[static_cast<std::size_t>(i + 1)].tc;
            if (std::llabs(m_avg - acc) < static_cast<std::int64_t>(et))
            {
                totalTC += acc;
                ++totalN;
                acc = 0;
            }
            else if (static_cast<double>(acc) > static_cast<double>(m_avg))
            {
                acc = 0;
            }
        }
        if (totalN != 0) m_avg = totalTC / totalN;

        if (!readyToGuess()) return;

        if (m_avg != 0) m_bpm = static_cast<int>(60000 / m_avg);

        if (m_bpm != m_lastBpm)
        {
            newBpm(m_bpm);
            m_lastBpm = m_bpm;

            const int threshold =
                m_predictionBpm < 90 ? kStickyThresholdLow : kStickyThreshold;
            if (m_config.sticky && m_predictionBpm != 0 && m_confidence >= threshold)
            {
                if (++m_stickyConfidenceCount >= kMinSticky) m_sticked = 1;
            }
            else
            {
                m_stickyConfidenceCount = 0;
            }
        }

        m_bpm = smoothedBpm();

        int hdCount = 0;
        for (int flag : m_halfDiscriminated)
        {
            if (flag != 0) ++hdCount;
        }
        if (hdCount >= kHistSize / 2 && m_bpm * 2 < kMaxBpm)
        {
            doubleBeat();
            m_halfDiscriminated.fill(0);
        }
        if (m_bpm > 500 || m_bpm < 0)
        {
            const std::int64_t keepClock = m_lastTC;
            reset(keepClock);
            return;
        }
        if (m_bpm < kMinBpm)
        {
            if (++m_doubleCount > 4) doubleBeat();
        }
        else
        {
            m_doubleCount = 0;
        }
        if (m_bpm > kMaxBpm)
        {
            if (++m_halfCount > 4) halfBeat();
        }
        else
        {
            m_halfCount = 0;
        }
    }

    [[nodiscard]] bool refineActive() const
    {
        return !m_config.onlySticky || m_sticked != 0;
    }

    Config m_config;
    std::array<Beat, kHistSize> m_hist{};
    std::array<int, kSmootherSize> m_smoother{};
    std::array<int, kHistSize> m_halfDiscriminated{};
    int m_hdPos = 0;
    std::int64_t m_avg = 0;
    std::int64_t m_lastTC = 0;
    std::int64_t m_predictionLastTC = 0;
    int m_bpm = 0;
    int m_lastBpm = 0;
    int m_predictionBpm = 0;
    int m_smPtr = 0;
    int m_insertionCount = 0;
    int m_confidence = 0;
    int m_bestConfidence = 0;
    bool m_forceNewBeat = false;
    int m_topConfidenceCount = 0;
    int m_stickyConfidenceCount = 0;
    int m_halfCount = 0;
    int m_doubleCount = 0;
    bool m_doResync = false;
    int m_sticked = 0;
};

// =============================================================================
// refine — out-of-line for readability (still header-only)
// =============================================================================

inline bool BeatEstimator::refine(bool isBeat, std::int64_t nowMs)
{
    bool accepted = false;
    bool predicted = false;
    bool resyncIn = false;
    bool resyncOut = false;

    if (m_bpm != 0 && nowMs > m_predictionLastTC + 60000 / m_bpm) predicted = true;

    if (isBeat) accepted = histStep(nowMs, kReal);

    calcBpm();

    if ((accepted || predicted) && m_sticked == 0 &&
        (m_predictionBpm == 0 || m_predictionBpm > kMaxBpm || m_predictionBpm < kMinBpm))
    {
        if (m_confidence >= m_bestConfidence) m_forceNewBeat = true;
        if (m_confidence >= 50)
        {
            if (++m_topConfidenceCount == kTopConfAdopt)
            {
                m_forceNewBeat = true;
                m_topConfidenceCount = 0;
            }
        }
        if (m_forceNewBeat)
        {
            m_forceNewBeat = false;
            m_bestConfidence = m_confidence;
            m_predictionBpm = m_bpm;
        }
    }

    if (m_sticked == 0) m_predictionBpm = m_bpm;
    m_bpm = m_predictionBpm;

    if (m_predictionBpm != 0 && accepted && !predicted)
    {
        // (Original declared a second shadowing `b` here, leaving the outer one
        // uninitialized on the resync-out path — one variable in this port.)
        int resyncBpm = 0;
        const double period = 60000.0 / m_predictionBpm;
        if (static_cast<double>(nowMs) >
            static_cast<double>(m_predictionLastTC) + period * 0.7)
        {
            resyncIn = true;
            resyncBpm = static_cast<int>(static_cast<float>(m_predictionBpm) * 1.01f);
        }
        if (static_cast<double>(nowMs) <
            static_cast<double>(m_predictionLastTC) + period * 0.3)
        {
            resyncOut = true;
            resyncBpm = static_cast<int>(static_cast<float>(m_predictionBpm) * 0.98f);
        }
        if (m_sticked == 0 && m_doResync && (resyncIn || resyncOut))
        {
            newBpm(resyncBpm);
            m_predictionBpm = smoothedBpm();
        }
    }

    if (resyncIn)
    {
        m_predictionLastTC = nowMs;
        m_doResync = true;
        return refineActive() ? true : isBeat;
    }
    if (predicted)
    {
        m_predictionLastTC = nowMs;
        if (m_confidence > 25) histStep(nowMs, kGuessed);
        m_doResync = false;
        return refineActive() ? true : isBeat;
    }
    if (resyncOut)
    {
        m_predictionLastTC = nowMs;
        m_doResync = true;
        return refineActive() ? false : isBeat;
    }

    return refineActive() ? (m_predictionBpm != 0 ? false : isBeat) : isBeat;
}

} // namespace lumi::modules

/**
 ****************************************************************************************
 * @file   AudioAnalyzer.hpp
 * @brief  Audio Analyzer Service Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * Concrete implementation of IAudioAnalyzer that:
 * - Retrieves audio data from IAudioEngine
 * - Performs FFT spectrum analysis
 * - Computes frequency bands
 * - Detects beats
 * - Publishes AudioDataEvent via EventBus
 *
 * ## Beat Detection Algorithm
 *
 * Simple energy-based beat detection:
 * 1. Compute energy in bass frequency range (20-250 Hz)
 * 2. Compare with rolling average energy
 * 3. If current energy > threshold * average, beat detected
 * 4. Use history buffer to estimate BPM
 ****************************************************************************************
 */

#pragma once

#include "audio/IAudioAnalyzer.hpp"

#include <QObject>
#include <memory>

// =============================================================================
// Forward Declarations
// =============================================================================

class IAudioEngine;
class AudioPlayer;  // Concrete type needed for currentStream()
class IEventBus;

// =============================================================================
// AudioAnalyzer Implementation
// =============================================================================

/**
 * @class AudioAnalyzer
 * @brief Concrete audio analyzer service
 */
class AudioAnalyzer : public QObject, public IAudioAnalyzer
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Construct AudioAnalyzer
     *
     * @param engine Audio engine for data retrieval
     * @param player Audio player for current stream
     * @param eventBus Event bus for publishing events
     * @param parent Qt parent object
     */
    explicit AudioAnalyzer(IAudioEngine& engine,
                           AudioPlayer& player,
                           IEventBus& eventBus,
                           QObject* parent = nullptr);
    ~AudioAnalyzer() override;

    // Non-copyable
    AudioAnalyzer(const AudioAnalyzer&) = delete;
    AudioAnalyzer& operator=(const AudioAnalyzer&) = delete;

    // =========================================================================
    // IAudioAnalyzer Implementation
    // =========================================================================

    // FFT Size Configuration
    [[nodiscard]] FFTSize fftSize() const override;
    void setFFTSize(FFTSize size) override;

    // Spectrum Analysis
    [[nodiscard]] SpectrumData spectrum() const override;
    std::size_t getSpectrum(float* data, std::size_t maxSize) const override;
    [[nodiscard]] std::vector<float> smoothedSpectrum(float smoothing = 0.8f) const override;

    // Frequency Bands
    [[nodiscard]] FrequencyBands frequencyBands() const override;
    [[nodiscard]] FrequencyBands smoothedBands(float smoothing = 0.8f) const override;
    [[nodiscard]] float bandLevel(float lowFreq, float highFreq) const override;

    // Waveform
    std::size_t getWaveform(float* data, std::size_t maxSize) const override;
    [[nodiscard]] std::vector<float> waveform() const override;

    // Level Metering
    [[nodiscard]] float levelLeft() const override;
    [[nodiscard]] float levelRight() const override;
    [[nodiscard]] float levelMono() const override;
    [[nodiscard]] float levelPeak() const override;

    // Beat Detection
    [[nodiscard]] BeatInfo beatInfo() const override;
    [[nodiscard]] bool isBeat() const override;
    [[nodiscard]] float beatIntensity() const override;
    [[nodiscard]] float bpm() const override;
    void setBeatSensitivity(float sensitivity) override;

    // Processing
    void update() override;
    void reset() override;

    // Analysis Control
    [[nodiscard]] bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    [[nodiscard]] bool isBeatDetectionEnabled() const override;
    void setBeatDetectionEnabled(bool enabled) override;

    // =========================================================================
    // Additional Methods
    // =========================================================================

    /**
     * @brief Set smoothing factor for spectrum decay
     *
     * @param factor 0.0 = instant response, 1.0 = frozen (default: 0.7)
     */
    void setSmoothingFactor(float factor);
    [[nodiscard]] float smoothingFactor() const;

    /**
     * @brief Set spectrum normalization mode
     */
    enum class NormalizationMode
    {
        None,       ///< Raw FFT values
        Peak,       ///< Normalize to peak value
        RMS,        ///< Normalize to RMS
        Logarithmic ///< Logarithmic scale (dB-like)
    };
    void setNormalizationMode(NormalizationMode mode);
    [[nodiscard]] NormalizationMode normalizationMode() const;

Q_SIGNALS:
    // Qt Signals
    void spectrumUpdated(const std::vector<float>& spectrum);
    void onBeatDetected(float intensity);
    void levelsUpdated(float left, float right);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    void updateSpectrum();
    void updateWaveform();
    void updateLevels();
    void detectBeat();
    void publishAudioDataEvent();
    
    // Helper methods
    float computeBandLevel(int lowBin, int highBin) const;
    int frequencyToBin(float frequency) const;
};

/**
 ****************************************************************************************
 * @file   IAudioAnalyzer.hpp
 * @brief  Interface for Audio Analysis (FFT, Spectrum, Beat Detection)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Audio Analyzer
 *
 * Provides real-time audio analysis for visualization:
 * - FFT spectrum analysis
 * - Waveform data
 * - Level metering
 * - Beat detection
 *
 * ```
 * ┌─────────────────┐     ┌──────────────────┐     ┌────────────────────┐
 * │  IAudioEngine   │────►│  IAudioAnalyzer  │────►│  VisualizerWidget  │
 * │  (Audio Data)   │     │  (FFT/Spectrum)  │     │  (OpenGL Render)   │
 * └─────────────────┘     └────────┬─────────┘     └────────────────────┘
 *                                  │
 *                                  │ AudioDataEvent
 *                                  ▼
 *                          ┌───────────────┐
 *                          │   EventBus    │
 *                          └───────────────┘
 * ```
 *
 * ## Frequency Bands
 *
 * ```
 * Band    | Frequency Range | Description
 * --------|-----------------|------------------------
 * Sub     | 20-60 Hz        | Sub-bass (kick drums)
 * Bass    | 60-250 Hz       | Bass (bass guitar, bass synth)
 * LowMid  | 250-500 Hz      | Low midrange
 * Mid     | 500-2000 Hz     | Midrange (vocals, instruments)
 * HighMid | 2000-4000 Hz    | High midrange
 * High    | 4000-20000 Hz   | Treble (cymbals, brilliance)
 * ```
 ****************************************************************************************
 */

#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>

// =============================================================================
// Audio Analysis Data
// =============================================================================

/**
 * @struct SpectrumData
 * @brief FFT spectrum analysis result
 */
struct SpectrumData
{
    /// Raw FFT magnitudes (normalized 0.0 - 1.0)
    std::vector<float> magnitudes;
    
    /// Number of FFT bins
    std::size_t binCount = 0;
    
    /// Frequency resolution (Hz per bin)
    float frequencyResolution = 0.0f;
    
    /// Peak frequency (Hz)
    float peakFrequency = 0.0f;
    
    /// Peak magnitude
    float peakMagnitude = 0.0f;
};

/**
 * @struct FrequencyBands
 * @brief Pre-computed frequency band levels
 */
struct FrequencyBands
{
    float sub = 0.0f;       ///< 20-60 Hz
    float bass = 0.0f;      ///< 60-250 Hz
    float lowMid = 0.0f;    ///< 250-500 Hz
    float mid = 0.0f;       ///< 500-2000 Hz
    float highMid = 0.0f;   ///< 2000-4000 Hz
    float high = 0.0f;      ///< 4000-20000 Hz
    
    /**
     * @brief Get band by index (0-5)
     */
    [[nodiscard]] float operator[](int index) const
    {
        switch (index)
        {
            case 0: return sub;
            case 1: return bass;
            case 2: return lowMid;
            case 3: return mid;
            case 4: return highMid;
            case 5: return high;
            default: return 0.0f;
        }
    }
    
    /**
     * @brief Get average of all bands
     */
    [[nodiscard]] float average() const
    {
        return (sub + bass + lowMid + mid + highMid + high) / 6.0f;
    }
};

/**
 * @struct BeatInfo
 * @brief Beat detection result
 */
struct BeatInfo
{
    bool detected = false;      ///< Beat detected this frame
    float intensity = 0.0f;     ///< Beat intensity (0.0 - 1.0)
    float confidence = 0.0f;    ///< Detection confidence
    float bpm = 0.0f;           ///< Estimated BPM (0 if unknown)
    int beatNumber = 0;         ///< Beat counter since start
    std::uint64_t timestampMs = 0;  ///< Timestamp when beat was detected
};

// =============================================================================
// IAudioAnalyzer Interface
// =============================================================================

/**
 * @class IAudioAnalyzer
 * @brief Interface for real-time audio analysis
 */
class IAudioAnalyzer
{
public:
    virtual ~IAudioAnalyzer() = default;

    // =========================================================================
    // FFT Size Configuration
    // =========================================================================

    /**
     * @enum FFTSize
     * @brief Available FFT window sizes
     */
    enum class FFTSize
    {
        Size256 = 256,
        Size512 = 512,
        Size1024 = 1024,
        Size2048 = 2048,
        Size4096 = 4096,
        Size8192 = 8192
    };

    /**
     * @brief Get/Set FFT size
     */
    [[nodiscard]] virtual FFTSize fftSize() const = 0;
    virtual void setFFTSize(FFTSize size) = 0;

    // =========================================================================
    // Spectrum Analysis
    // =========================================================================

    /**
     * @brief Get current spectrum data
     */
    [[nodiscard]] virtual SpectrumData spectrum() const = 0;

    /**
     * @brief Get spectrum as simple float array
     *
     * @param[out] data Buffer to fill
     * @param maxSize Maximum number of values
     * @return Actual number of values written
     */
    virtual std::size_t getSpectrum(float* data, std::size_t maxSize) const = 0;

    /**
     * @brief Get smoothed spectrum (with decay)
     *
     * @param smoothing Smoothing factor (0.0 = no smoothing, 1.0 = frozen)
     */
    [[nodiscard]] virtual std::vector<float> smoothedSpectrum(
        float smoothing = 0.8f) const = 0;

    // =========================================================================
    // Frequency Bands
    // =========================================================================

    /**
     * @brief Get pre-computed frequency bands
     */
    [[nodiscard]] virtual FrequencyBands frequencyBands() const = 0;

    /**
     * @brief Get smoothed frequency bands
     *
     * @param smoothing Smoothing factor
     */
    [[nodiscard]] virtual FrequencyBands smoothedBands(
        float smoothing = 0.8f) const = 0;

    /**
     * @brief Get custom frequency band level
     *
     * @param lowFreq Low frequency bound (Hz)
     * @param highFreq High frequency bound (Hz)
     * @return Band level (0.0 - 1.0)
     */
    [[nodiscard]] virtual float bandLevel(float lowFreq, float highFreq) const = 0;

    // =========================================================================
    // Waveform
    // =========================================================================

    /**
     * @brief Get current waveform data
     *
     * @param[out] data Buffer to fill
     * @param maxSize Maximum number of samples
     * @return Actual number of samples written
     */
    virtual std::size_t getWaveform(float* data, std::size_t maxSize) const = 0;

    /**
     * @brief Get waveform as vector
     */
    [[nodiscard]] virtual std::vector<float> waveform() const = 0;

    // =========================================================================
    // Level Metering
    // =========================================================================

    /**
     * @brief Get left channel level (0.0 - 1.0)
     */
    [[nodiscard]] virtual float levelLeft() const = 0;

    /**
     * @brief Get right channel level (0.0 - 1.0)
     */
    [[nodiscard]] virtual float levelRight() const = 0;

    /**
     * @brief Get mono level (average of L/R)
     */
    [[nodiscard]] virtual float levelMono() const = 0;

    /**
     * @brief Get peak level (max of L/R)
     */
    [[nodiscard]] virtual float levelPeak() const = 0;

    // =========================================================================
    // Beat Detection
    // =========================================================================

    /**
     * @brief Get current beat info
     */
    [[nodiscard]] virtual BeatInfo beatInfo() const = 0;

    /**
     * @brief Check if beat was detected this frame
     */
    [[nodiscard]] virtual bool isBeat() const = 0;

    /**
     * @brief Get beat intensity
     */
    [[nodiscard]] virtual float beatIntensity() const = 0;

    /**
     * @brief Get estimated BPM (0 if unknown)
     */
    [[nodiscard]] virtual float bpm() const = 0;

    /**
     * @brief Set beat detection sensitivity
     *
     * @param sensitivity 0.0 (very sensitive) to 1.0 (less sensitive)
     */
    virtual void setBeatSensitivity(float sensitivity) = 0;

    // =========================================================================
    // Processing
    // =========================================================================

    /**
     * @brief Process audio data and update analysis
     *
     * Should be called regularly from the main loop.
     * Typically updates internal buffers and publishes events.
     */
    virtual void update() = 0;

    /**
     * @brief Reset all analysis state
     */
    virtual void reset() = 0;

    // =========================================================================
    // Analysis Control
    // =========================================================================

    /**
     * @brief Enable/Disable analysis
     *
     * When disabled, update() does nothing (saves CPU).
     */
    [[nodiscard]] virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    /**
     * @brief Enable/Disable beat detection
     *
     * Beat detection can be CPU-intensive.
     */
    [[nodiscard]] virtual bool isBeatDetectionEnabled() const = 0;
    virtual void setBeatDetectionEnabled(bool enabled) = 0;
};

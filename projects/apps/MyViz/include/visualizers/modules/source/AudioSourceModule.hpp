/**
 ****************************************************************************************
 * @file   AudioSourceModule.hpp
 * @brief  Audio Source Module - FFT data processing with embedded smoothing
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * ## Overview
 *
 * AudioSourceModule processes raw FFT spectrum data and provides:
 *   - Frequency band mapping (Linear/Log/Mel)
 *   - dB normalization
 *   - Embedded SmoothingModule (prefixed as "smooth.*")
 *   - Band-level output (Sub, Bass, LowMid, Mid, HighMid, Treble)
 *   - Overall level calculation
 *
 * ## Data Flow
 *
 * ```
 * FFT Bins (2048)
 * ████████████████████████████████████████████████████
 *                     │
 *                     ▼
 * ┌─────────────────────────────────────────────────┐
 * │           Frequency Mapping (Log/Mel)           │
 * └─────────────────────────────────────────────────┘
 *                     │
 *                     ▼
 * ┌─────────────────────────────────────────────────┐
 * │              dB Normalization                   │
 * │         (floorDb → 0.0, ceilDb → 1.0)           │
 * └─────────────────────────────────────────────────┘
 *                     │
 *                     ▼
 * ┌─────────────────────────────────────────────────┐
 * │             SmoothingModule                     │
 * │            (SMA/EMA/WMA/DEMA)                   │
 * └─────────────────────────────────────────────────┘
 *                     │
 *                     ▼
 * Normalized Bands (64)
 * ▃▅▇█▇▅▃▂▃▅▆▇█▇▅▄▃▃▄▅▆▇▇▆▅▄▃▂▂▃▄▅▆▆▅▄▃▂▁▁▂▃▃▃▂▂▁▁▁▁▁
 * ```
 *
 * @see LumiPulse_VisualSystem_Architecture.md Section 4.1.1
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/processing/SmoothingModule.hpp"

#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

namespace lumi::modules
{

// =============================================================================
// Frequency Scale Enum
// =============================================================================

/**
 * @brief Frequency to band mapping mode
 */
enum class FrequencyScale
{
    Linear,     ///< Linear mapping
    Log,        ///< Logarithmic mapping (default, perceptually uniform)
    Mel         ///< Mel scale (speech-optimized)
};

// =============================================================================
// Frequency Band Data
// =============================================================================

/**
 * @brief Pre-calculated frequency band levels
 */
struct FrequencyBands
{
    float sub = 0.0f;       ///< Sub-bass (20-60 Hz)
    float bass = 0.0f;      ///< Bass (60-250 Hz)
    float lowMid = 0.0f;    ///< Low-mid (250-500 Hz)
    float mid = 0.0f;       ///< Mid (500-2000 Hz)
    float highMid = 0.0f;   ///< High-mid (2000-4000 Hz)
    float treble = 0.0f;    ///< Treble (4000-20000 Hz)
    
    /**
     * @brief Get overall level (weighted average)
     */
    [[nodiscard]] float overall() const
    {
        // Weight: bass+mid more, treble less
        return (sub * 0.1f + bass * 0.25f + lowMid * 0.2f +
                mid * 0.25f + highMid * 0.15f + treble * 0.05f);
    }
    
    /**
     * @brief Access by index [0-5]
     */
    [[nodiscard]] float operator[](int i) const
    {
        switch (i)
        {
        case 0: return sub;
        case 1: return bass;
        case 2: return lowMid;
        case 3: return mid;
        case 4: return highMid;
        case 5: return treble;
        default: return 0.0f;
        }
    }
};

// =============================================================================
// AudioSourceModule
// =============================================================================

/**
 * @class AudioSourceModule
 * @brief Processes FFT data for visualization
 *
 * This is the primary audio input module for visualizers.
 * It handles all audio processing: FFT mapping, normalization, smoothing.
 *
 * @par Example Usage
 * @code
 * AudioSourceModule audio;
 * audio.setScale(FrequencyScale::Log);
 * audio.setBands(64);
 * audio.setParam("smooth.algorithm", static_cast<int>(SmoothingAlgorithm::EMA));
 * audio.setParam("smooth.timeMs", 50.0f);
 *
 * // In update loop:
 * audio.update(fftData, fftSize, deltaTime);
 *
 * float bassLevel = audio.bands().bass;
 * float overall = audio.overallLevel();
 * const float* spectrum = audio.spectrum();
 * @endcode
 */
class AudioSourceModule : public IModule
{
public:
    // =========================================================================
    // Construction
    // =========================================================================
    
    AudioSourceModule();
    ~AudioSourceModule() override = default;
    
    // =========================================================================
    // IModule Interface
    // =========================================================================
    
    [[nodiscard]] const char* moduleId() const override { return "audioSource"; }
    [[nodiscard]] const char* displayName() const override { return "Audio Source"; }
    [[nodiscard]] const char* category() const override { return "Source"; }
    [[nodiscard]] const char* description() const override
    {
        return "FFT spectrum processing with frequency mapping and smoothing";
    }
    
    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs() const override;
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const override;
    bool setParam(const std::string& id, const ParamValue& value) override;
    void resetToDefaults() override;
    
    // =========================================================================
    // Audio Input
    // =========================================================================
    
    /**
     * @brief Update with new FFT data
     * @param fftData Raw FFT magnitude data from BASS
     * @param fftSize Size of FFT data
     * @param deltaTime Time since last update (seconds)
     */
    void update(const float* fftData, int fftSize, float deltaTime);
    
    /**
     * @brief Reset all state
     */
    void reset();
    
    // =========================================================================
    // Output: Spectrum
    // =========================================================================
    
    /**
     * @brief Get processed spectrum data
     * @return Pointer to spectrum array (bands() count elements)
     */
    [[nodiscard]] const float* spectrum() const { return m_spectrum.data(); }
    
    /**
     * @brief Get number of output bands
     */
    [[nodiscard]] int bandCount() const { return m_bands; }
    
    /**
     * @brief Get spectrum as vector
     */
    [[nodiscard]] const std::vector<float>& spectrumVector() const { return m_spectrum; }
    
    // =========================================================================
    // Output: Levels
    // =========================================================================
    
    /**
     * @brief Get frequency band levels
     */
    [[nodiscard]] const FrequencyBands& bands() const { return m_bandLevels; }
    
    /**
     * @brief Get overall audio level (0-1)
     */
    [[nodiscard]] float overallLevel() const { return m_overallLevel; }
    
    /**
     * @brief Get raw (unsmoothed) overall level
     */
    [[nodiscard]] float rawLevel() const { return m_rawLevel; }
    
    // =========================================================================
    // Configuration: Mapping
    // =========================================================================
    
    /**
     * @brief Set frequency scale mode
     */
    void setScale(FrequencyScale scale);
    
    /**
     * @brief Get frequency scale
     */
    [[nodiscard]] FrequencyScale scale() const { return m_scale; }
    
    /**
     * @brief Set number of output bands
     * @param bands Number of bands (8-512)
     */
    void setBands(int bands);
    
    /**
     * @brief Get number of bands
     */
    [[nodiscard]] int numBands() const { return m_bands; }
    
    // =========================================================================
    // Configuration: Normalization
    // =========================================================================
    
    /**
     * @brief Set dB floor (values below map to 0)
     * @param db Floor in dB (-120 to 0)
     */
    void setFloorDb(float db);
    
    /**
     * @brief Get floor dB
     */
    [[nodiscard]] float floorDb() const { return m_floorDb; }
    
    /**
     * @brief Set dB ceiling (values above map to 1)
     * @param db Ceiling in dB (-60 to +20)
     */
    void setCeilingDb(float db);
    
    /**
     * @brief Get ceiling dB
     */
    [[nodiscard]] float ceilingDb() const { return m_ceilDb; }
    
    /**
     * @brief Enable/disable clamping to [0,1]
     */
    void setClamp(bool enabled) { m_clamp01 = enabled; }
    
    /**
     * @brief Check if clamping is enabled
     */
    [[nodiscard]] bool clamp() const { return m_clamp01; }
    
    // =========================================================================
    // Configuration: Gain
    // =========================================================================
    
    /**
     * @brief Set input gain multiplier
     * @param gain Gain (0.1 - 5.0)
     */
    void setGain(float gain);
    
    /**
     * @brief Get gain
     */
    [[nodiscard]] float gain() const { return m_gain; }
    
    // =========================================================================
    // Embedded Smoothing Module
    // =========================================================================
    
    /**
     * @brief Access the embedded smoothing module
     */
    [[nodiscard]] SmoothingModule& smoothing() { return m_smoothing; }
    [[nodiscard]] const SmoothingModule& smoothing() const { return m_smoothing; }
    
    // =========================================================================
    // Utility
    // =========================================================================
    
    /**
     * @brief Get scale name as string
     */
    static const char* scaleName(FrequencyScale scale);
    
    /**
     * @brief Get all scale names
     */
    static std::vector<std::string> scaleNames();
    
private:
    // =========================================================================
    // Internal Processing
    // =========================================================================
    
    void mapFrequencies(const float* fftData, int fftSize);
    void normalizeDb();
    void calculateBandLevels();
    int frequencyToBin(float hz, int fftSize, int sampleRate = 48000) const;
    float binToFrequency(int bin, int fftSize, int sampleRate = 48000) const;
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    FrequencyScale m_scale = FrequencyScale::Log;
    int m_bands = 64;
    float m_floorDb = -60.0f;
    float m_ceilDb = 0.0f;
    bool m_clamp01 = true;
    float m_gain = 1.0f;
    
    // =========================================================================
    // Embedded Modules
    // =========================================================================
    
    SmoothingModule m_smoothing;
    
    // =========================================================================
    // Output State
    // =========================================================================
    
    std::vector<float> m_spectrum;          ///< Processed spectrum
    std::vector<float> m_rawSpectrum;       ///< Before smoothing
    FrequencyBands m_bandLevels;            ///< Frequency band levels
    float m_overallLevel = 0.0f;            ///< Overall level (smoothed)
    float m_rawLevel = 0.0f;                ///< Raw level
    
    // =========================================================================
    // Constants
    // =========================================================================
    
    static constexpr int MIN_BANDS = 8;
    static constexpr int MAX_BANDS = 512;
    static constexpr float MIN_FLOOR_DB = -120.0f;
    static constexpr float MAX_FLOOR_DB = 0.0f;
    static constexpr float MIN_CEIL_DB = -60.0f;
    static constexpr float MAX_CEIL_DB = 20.0f;
};

// =============================================================================
// Implementation
// =============================================================================

inline AudioSourceModule::AudioSourceModule()
{
    m_spectrum.resize(m_bands, 0.0f);
    m_rawSpectrum.resize(m_bands, 0.0f);
    
    // Configure default smoothing
    m_smoothing.setAlgorithm(SmoothingAlgorithm::EMA);
    m_smoothing.setTimeMs(50.0f);
}

inline std::vector<ModuleParamDesc> AudioSourceModule::paramDescs() const
{
    std::vector<ModuleParamDesc> params;
    
    // Own parameters
    params.push_back(
        ParamBuilder("scale", ParamType::Enum)
            .displayName("Frequency Scale")
            .enumOptions(scaleNames())
            .defaultValue(static_cast<int>(FrequencyScale::Log))
            .tooltip("How frequencies map to bands")
            .group("Mapping")
            .order(0)
            .build()
    );
    
    params.push_back(
        ParamBuilder("bands", ParamType::Int)
            .displayName("Bands")
            .range(static_cast<float>(MIN_BANDS), static_cast<float>(MAX_BANDS), 8.0f)
            .defaultValue(64)
            .tooltip("Number of output frequency bands")
            .group("Mapping")
            .order(1)
            .build()
    );
    
    params.push_back(
        ParamBuilder("floorDb", ParamType::Float)
            .displayName("Floor")
            .range(MIN_FLOOR_DB, MAX_FLOOR_DB, 1.0f)
            .defaultValue(-60.0f)
            .unit("dB")
            .tooltip("Values below map to 0")
            .group("Normalization")
            .order(10)
            .build()
    );
    
    params.push_back(
        ParamBuilder("ceilDb", ParamType::Float)
            .displayName("Ceiling")
            .range(MIN_CEIL_DB, MAX_CEIL_DB, 1.0f)
            .defaultValue(0.0f)
            .unit("dB")
            .tooltip("Values above map to 1")
            .group("Normalization")
            .order(11)
            .build()
    );
    
    params.push_back(
        ParamBuilder("clamp01", ParamType::Bool)
            .displayName("Clamp to 0-1")
            .defaultValue(true)
            .tooltip("Constrain output to [0,1] range")
            .group("Normalization")
            .order(12)
            .build()
    );
    
    params.push_back(
        ParamBuilder("gain", ParamType::Float)
            .displayName("Gain")
            .range(0.1f, 5.0f, 0.1f)
            .defaultValue(1.0f)
            .tooltip("Input signal multiplier")
            .group("Gain")
            .order(20)
            .build()
    );
    
    // Aggregate smoothing params with "smooth." prefix
    for (const auto& p : m_smoothing.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "smooth." + p.id;
        prefixed.group = "Smoothing";
        
        // Also prefix the dependsOn reference if set
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "smooth." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }
    
    return params;
}

inline bool AudioSourceModule::getParam(const std::string& id, ParamValue& out) const
{
    // Check for smooth.* prefix
    if (id.rfind("smooth.", 0) == 0)
    {
        return m_smoothing.getParam(id.substr(7), out);
    }
    
    // Own parameters
    if (id == "scale")
    {
        out = static_cast<int>(m_scale);
        return true;
    }
    if (id == "bands")
    {
        out = m_bands;
        return true;
    }
    if (id == "floorDb")
    {
        out = m_floorDb;
        return true;
    }
    if (id == "ceilDb")
    {
        out = m_ceilDb;
        return true;
    }
    if (id == "clamp01")
    {
        out = m_clamp01;
        return true;
    }
    if (id == "gain")
    {
        out = m_gain;
        return true;
    }
    
    return false;
}

inline bool AudioSourceModule::setParam(const std::string& id, const ParamValue& value)
{
    // Check for smooth.* prefix
    if (id.rfind("smooth.", 0) == 0)
    {
        return m_smoothing.setParam(id.substr(7), value);
    }
    
    // Own parameters
    if (id == "scale")
    {
        if (std::holds_alternative<int>(value))
        {
            setScale(static_cast<FrequencyScale>(std::get<int>(value)));
            return true;
        }
    }
    else if (id == "bands")
    {
        if (std::holds_alternative<int>(value))
        {
            setBands(std::get<int>(value));
            return true;
        }
    }
    else if (id == "floorDb")
    {
        if (std::holds_alternative<float>(value))
        {
            setFloorDb(std::get<float>(value));
            return true;
        }
    }
    else if (id == "ceilDb")
    {
        if (std::holds_alternative<float>(value))
        {
            setCeilingDb(std::get<float>(value));
            return true;
        }
    }
    else if (id == "clamp01")
    {
        if (std::holds_alternative<bool>(value))
        {
            m_clamp01 = std::get<bool>(value);
            return true;
        }
    }
    else if (id == "gain")
    {
        if (std::holds_alternative<float>(value))
        {
            setGain(std::get<float>(value));
            return true;
        }
    }
    
    return false;
}

inline void AudioSourceModule::resetToDefaults()
{
    m_scale = FrequencyScale::Log;
    m_bands = 64;
    m_floorDb = -60.0f;
    m_ceilDb = 0.0f;
    m_clamp01 = true;
    m_gain = 1.0f;
    
    m_smoothing.resetToDefaults();
    
    m_spectrum.resize(m_bands, 0.0f);
    m_rawSpectrum.resize(m_bands, 0.0f);
    reset();
}

inline void AudioSourceModule::update(const float* fftData, int fftSize, float deltaTime)
{
    if (fftData == nullptr || fftSize <= 0)
    {
        return;
    }
    
    // Step 1: Map frequencies to output bands
    mapFrequencies(fftData, fftSize);
    
    // Step 2: Apply gain
    for (float& val : m_rawSpectrum)
    {
        val *= m_gain;
    }
    
    // Step 3: Normalize dB
    normalizeDb();
    
    // Step 4: Apply smoothing
    for (int i = 0; i < m_bands; ++i)
    {
        m_spectrum[i] = m_smoothing.process(m_rawSpectrum[i], deltaTime);
    }
    
    // Step 5: Calculate band levels
    calculateBandLevels();
    
    // Step 6: Calculate overall level
    m_rawLevel = m_bandLevels.overall();
    m_overallLevel = m_smoothing.process(m_rawLevel, deltaTime);
}

inline void AudioSourceModule::reset()
{
    std::fill(m_spectrum.begin(), m_spectrum.end(), 0.0f);
    std::fill(m_rawSpectrum.begin(), m_rawSpectrum.end(), 0.0f);
    m_bandLevels = FrequencyBands{};
    m_overallLevel = 0.0f;
    m_rawLevel = 0.0f;
    m_smoothing.reset();
}

inline void AudioSourceModule::setScale(FrequencyScale scale)
{
    m_scale = scale;
}

inline void AudioSourceModule::setBands(int bands)
{
    m_bands = std::clamp(bands, MIN_BANDS, MAX_BANDS);
    m_spectrum.resize(m_bands, 0.0f);
    m_rawSpectrum.resize(m_bands, 0.0f);
}

inline void AudioSourceModule::setFloorDb(float db)
{
    m_floorDb = std::clamp(db, MIN_FLOOR_DB, MAX_FLOOR_DB);
}

inline void AudioSourceModule::setCeilingDb(float db)
{
    m_ceilDb = std::clamp(db, MIN_CEIL_DB, MAX_CEIL_DB);
}

inline void AudioSourceModule::setGain(float gain)
{
    m_gain = std::clamp(gain, 0.1f, 5.0f);
}

inline void AudioSourceModule::mapFrequencies(const float* fftData, int fftSize)
{
    // Frequency range: ~20 Hz to ~20000 Hz
    constexpr float MIN_FREQ = 20.0f;
    constexpr float MAX_FREQ = 20000.0f;
    
    int halfFft = fftSize / 2;  // Useful bins
    
    for (int band = 0; band < m_bands; ++band)
    {
        float t = static_cast<float>(band) / static_cast<float>(m_bands - 1);
        float freq = 0.0f;
        
        switch (m_scale)
        {
        case FrequencyScale::Linear:
            freq = MIN_FREQ + t * (MAX_FREQ - MIN_FREQ);
            break;
            
        case FrequencyScale::Log:
            // Logarithmic: log(min) + t * (log(max) - log(min))
            freq = MIN_FREQ * std::pow(MAX_FREQ / MIN_FREQ, t);
            break;
            
        case FrequencyScale::Mel:
            // Mel scale: m = 2595 * log10(1 + f/700)
            {
                float melMin = 2595.0f * std::log10(1.0f + MIN_FREQ / 700.0f);
                float melMax = 2595.0f * std::log10(1.0f + MAX_FREQ / 700.0f);
                float mel = melMin + t * (melMax - melMin);
                freq = 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
            }
            break;
        }
        
        // Get bin index for this frequency
        int bin = frequencyToBin(freq, fftSize);
        bin = std::clamp(bin, 0, halfFft - 1);
        
        // Average neighboring bins for smoother output
        float sum = 0.0f;
        int count = 0;
        int range = std::max(1, halfFft / m_bands / 2);
        
        for (int i = std::max(0, bin - range); i <= std::min(halfFft - 1, bin + range); ++i)
        {
            sum += fftData[i];
            ++count;
        }
        
        m_rawSpectrum[band] = (count > 0) ? (sum / count) : 0.0f;
    }
}

inline void AudioSourceModule::normalizeDb()
{
    float rangeDb = m_ceilDb - m_floorDb;
    if (rangeDb <= 0.0f)
    {
        rangeDb = 60.0f;  // Fallback
    }
    
    for (float& val : m_rawSpectrum)
    {
        if (val <= 0.0f)
        {
            val = 0.0f;
            continue;
        }
        
        // Convert to dB
        float db = 20.0f * std::log10(val);
        
        // Normalize to [0, 1]
        val = (db - m_floorDb) / rangeDb;
        
        // Clamp if enabled
        if (m_clamp01)
        {
            val = std::clamp(val, 0.0f, 1.0f);
        }
    }
}

inline void AudioSourceModule::calculateBandLevels()
{
    // Calculate average for each frequency range
    // Assuming m_bands >= 8
    
    int sub = m_bands / 16;       // First ~6%
    int bass = m_bands / 8;       // ~12%
    int lowMid = m_bands / 4;     // ~25%
    int mid = m_bands / 2;        // ~50%
    int highMid = m_bands * 3/4;  // ~75%
    
    auto avg = [this](int start, int end) {
        if (start >= end || start < 0 || end > m_bands) return 0.0f;
        float sum = 0.0f;
        for (int i = start; i < end; ++i)
        {
            sum += m_spectrum[i];
        }
        return sum / static_cast<float>(end - start);
    };
    
    m_bandLevels.sub = avg(0, sub);
    m_bandLevels.bass = avg(sub, bass);
    m_bandLevels.lowMid = avg(bass, lowMid);
    m_bandLevels.mid = avg(lowMid, mid);
    m_bandLevels.highMid = avg(mid, highMid);
    m_bandLevels.treble = avg(highMid, m_bands);
}

inline int AudioSourceModule::frequencyToBin(float hz, int fftSize, int sampleRate) const
{
    return static_cast<int>(hz * fftSize / sampleRate);
}

inline float AudioSourceModule::binToFrequency(int bin, int fftSize, int sampleRate) const
{
    return static_cast<float>(bin) * sampleRate / fftSize;
}

inline const char* AudioSourceModule::scaleName(FrequencyScale scale)
{
    switch (scale)
    {
    case FrequencyScale::Linear: return "Linear";
    case FrequencyScale::Log:    return "Logarithmic";
    case FrequencyScale::Mel:    return "Mel";
    default: return "Unknown";
    }
}

inline std::vector<std::string> AudioSourceModule::scaleNames()
{
    return {"Linear", "Logarithmic", "Mel"};
}

} // namespace lumi::modules

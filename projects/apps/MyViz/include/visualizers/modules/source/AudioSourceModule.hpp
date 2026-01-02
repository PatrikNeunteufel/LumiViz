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
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

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

/**
 * @brief Preset audio processing configurations
 */
enum class AudioPreset
{
    Default,        ///< Balanced settings for general use
    BassHeavy,      ///< Emphasizes bass frequencies
    Vocals,         ///< Optimized for vocals and mid frequencies
    Electronic,     ///< Fast response for electronic music
    Ambient,        ///< Smooth, slow response for ambient music
    Custom          ///< User-modified settings
};

/**
 * @struct AudioPresetData
 * @brief Data structure for saveable audio presets
 */
struct AudioPresetData
{
    std::string name;
    FrequencyScale scale = FrequencyScale::Log;
    int bands = 64;
    float floorDb = -60.0f;
    float ceilDb = 0.0f;
    bool clamp01 = true;
    float gain = 1.0f;
    
    // Embedded smoothing settings
    SmoothingAlgorithm smoothAlgorithm = SmoothingAlgorithm::EMA;
    float smoothTimeMs = 50.0f;
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
    // Presets
    // =========================================================================
    
    /**
     * @brief Apply a preset configuration
     */
    void applyPreset(AudioPreset preset);
    
    /**
     * @brief Get current preset
     */
    [[nodiscard]] AudioPreset preset() const { return m_preset; }
    
    /**
     * @brief Get all preset names (builtin + user)
     */
    std::vector<std::string> presetNames() const;
    
    /**
     * @brief Get builtin preset names only
     */
    static std::vector<std::string> builtinPresetNames();
    
    // =========================================================================
    // User Preset Management
    // =========================================================================
    
    /**
     * @brief Save current settings as a named preset
     * @param name Preset name
     */
    void savePreset(const std::string& name);
    
    /**
     * @brief Load a preset by name (builtin or user)
     * @param name Preset name
     */
    void loadPreset(const std::string& name);
    
    /**
     * @brief Delete a user preset
     * @param name Preset name
     * @return true if deleted
     */
    bool deletePreset(const std::string& name);
    
    /**
     * @brief Check if a preset is a user preset (deletable)
     */
    [[nodiscard]] bool isUserPreset(const std::string& name) const;
    
    /**
     * @brief Get current preset name
     */
    [[nodiscard]] const std::string& currentPresetName() const { return m_currentPresetName; }
    
    /**
     * @brief Set directory for user presets
     */
    static void setUserPresetsDirectory(const std::string& dir);
    
    /**
     * @brief Get user presets directory
     */
    static const std::string& userPresetsDirectory() { return s_userPresetsDir; }
    
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
    AudioPreset m_preset = AudioPreset::Default;
    
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
    
    // =========================================================================
    // User Presets
    // =========================================================================
    
    std::string m_currentPresetName = "Default";
    std::map<std::string, AudioPresetData> m_userPresets;
    mutable bool m_userPresetsLoaded = false;
    
    void loadUserPresetsFromDisk();
    bool parsePresetFile(const std::string& filePath, AudioPresetData& outPreset);
    
    inline static std::string s_userPresetsDir;
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
    
    // Preset at the top
    params.push_back(
        ParamBuilder("preset", ParamType::Enum)
            .displayName("Audio Preset")
            .enumOptions(presetNames())
            .defaultValue(static_cast<int>(AudioPreset::Default))
            .tooltip("Predefined audio processing configurations")
            .order(-1)  // Always first
            .build()
    );
    
    // Own parameters
    params.push_back(
        ParamBuilder("scale", ParamType::Enum)
            .displayName("Frequency Scale")
            .enumOptions(scaleNames())
            .defaultValue(static_cast<int>(FrequencyScale::Log))
            .tooltip("How frequencies map to bands")
            .subGroup("Mapping")
            .order(0)
            .build()
    );
    
    params.push_back(
        ParamBuilder("bands", ParamType::Int)
            .displayName("Bands")
            .range(static_cast<float>(MIN_BANDS), static_cast<float>(MAX_BANDS), 8.0f)
            .defaultValue(64)
            .tooltip("Number of output frequency bands")
            .subGroup("Mapping")
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
            .subGroup("Normalization")
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
            .subGroup("Normalization")
            .order(11)
            .build()
    );
    
    params.push_back(
        ParamBuilder("clamp01", ParamType::Bool)
            .displayName("Clamp to 0-1")
            .defaultValue(true)
            .tooltip("Constrain output to [0,1] range")
            .subGroup("Normalization")
            .order(12)
            .build()
    );
    
    params.push_back(
        ParamBuilder("gain", ParamType::Float)
            .displayName("Gain")
            .range(0.1f, 5.0f, 0.1f)
            .defaultValue(1.0f)
            .tooltip("Input signal multiplier")
            .subGroup("Gain")
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
    if (id == "preset")
    {
        // Find current preset in list
        auto names = presetNames();
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (names[i] == m_currentPresetName)
            {
                out = static_cast<int>(i);
                return true;
            }
        }
        out = 0;  // Default to [Custom]
        return true;
    }
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
        bool result = m_smoothing.setParam(id.substr(7), value);
        if (result)
        {
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
        }
        return result;
    }
    
    // Own parameters
    if (id == "preset")
    {
        if (std::holds_alternative<int>(value))
        {
            int idx = std::get<int>(value);
            auto names = presetNames();
            if (idx >= 0 && idx < static_cast<int>(names.size()))
            {
                loadPreset(names[idx]);
            }
            return true;
        }
    }
    else if (id == "scale")
    {
        if (std::holds_alternative<int>(value))
        {
            setScale(static_cast<FrequencyScale>(std::get<int>(value)));
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "bands")
    {
        if (std::holds_alternative<int>(value))
        {
            setBands(std::get<int>(value));
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "floorDb")
    {
        if (std::holds_alternative<float>(value))
        {
            setFloorDb(std::get<float>(value));
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "ceilDb")
    {
        if (std::holds_alternative<float>(value))
        {
            setCeilingDb(std::get<float>(value));
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "clamp01")
    {
        if (std::holds_alternative<bool>(value))
        {
            m_clamp01 = std::get<bool>(value);
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "gain")
    {
        if (std::holds_alternative<float>(value))
        {
            setGain(std::get<float>(value));
            m_preset = AudioPreset::Custom;
            m_currentPresetName = "[Custom]";
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

inline std::vector<std::string> AudioSourceModule::builtinPresetNames()
{
    return {"Default", "Bass Heavy", "Vocals", "Electronic", "Ambient"};
}

inline std::vector<std::string> AudioSourceModule::presetNames() const
{
    // Lazy-load user presets
    if (!m_userPresetsLoaded && !s_userPresetsDir.empty())
    {
        const_cast<AudioSourceModule*>(this)->loadUserPresetsFromDisk();
    }
    
    std::vector<std::string> names;
    
    // [Custom] first
    names.push_back("[Custom]");
    
    // Builtin presets
    auto builtins = builtinPresetNames();
    for (const auto& name : builtins)
    {
        names.push_back(name);
    }
    
    // Separator if user presets exist
    if (!m_userPresets.empty())
    {
        names.push_back("---");  // Separator
        
        // User presets
        for (const auto& pair : m_userPresets)
        {
            names.push_back(pair.first);
        }
    }
    
    return names;
}

inline void AudioSourceModule::setUserPresetsDirectory(const std::string& dir)
{
    s_userPresetsDir = dir;
}

inline void AudioSourceModule::savePreset(const std::string& name)
{
    AudioPresetData preset;
    preset.name = name;
    preset.scale = m_scale;
    preset.bands = m_bands;
    preset.floorDb = m_floorDb;
    preset.ceilDb = m_ceilDb;
    preset.clamp01 = m_clamp01;
    preset.gain = m_gain;
    preset.smoothAlgorithm = m_smoothing.algorithm();
    preset.smoothTimeMs = m_smoothing.timeMs();
    
    m_userPresets[name] = preset;
    m_currentPresetName = name;
    m_preset = AudioPreset::Custom;
    
    // Save to .audio file
    if (s_userPresetsDir.empty())
    {
        return;
    }
    
    std::filesystem::create_directories(s_userPresetsDir);
    std::string filePath = s_userPresetsDir + "/" + name + ".audio";
    
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        return;
    }
    
    // Write JSON
    file << "{\n";
    file << "  \"name\": \"" << name << "\",\n";
    file << "  \"scale\": " << static_cast<int>(preset.scale) << ",\n";
    file << "  \"bands\": " << preset.bands << ",\n";
    file << "  \"floorDb\": " << preset.floorDb << ",\n";
    file << "  \"ceilDb\": " << preset.ceilDb << ",\n";
    file << "  \"clamp01\": " << (preset.clamp01 ? "true" : "false") << ",\n";
    file << "  \"gain\": " << preset.gain << ",\n";
    file << "  \"smoothAlgorithm\": " << static_cast<int>(preset.smoothAlgorithm) << ",\n";
    file << "  \"smoothTimeMs\": " << preset.smoothTimeMs << "\n";
    file << "}\n";
    file.close();
}

inline void AudioSourceModule::loadPreset(const std::string& name)
{
    // Skip [Custom] and separator
    if (name == "[Custom]" || name == "---")
    {
        return;
    }
    
    // Check builtin presets first
    if (name == "Default")
    {
        applyPreset(AudioPreset::Default);
        m_currentPresetName = name;
        return;
    }
    if (name == "Bass Heavy")
    {
        applyPreset(AudioPreset::BassHeavy);
        m_currentPresetName = name;
        return;
    }
    if (name == "Vocals")
    {
        applyPreset(AudioPreset::Vocals);
        m_currentPresetName = name;
        return;
    }
    if (name == "Electronic")
    {
        applyPreset(AudioPreset::Electronic);
        m_currentPresetName = name;
        return;
    }
    if (name == "Ambient")
    {
        applyPreset(AudioPreset::Ambient);
        m_currentPresetName = name;
        return;
    }
    
    // Check user presets
    auto it = m_userPresets.find(name);
    if (it != m_userPresets.end())
    {
        const auto& preset = it->second;
        m_scale = preset.scale;
        m_bands = preset.bands;
        m_floorDb = preset.floorDb;
        m_ceilDb = preset.ceilDb;
        m_clamp01 = preset.clamp01;
        m_gain = preset.gain;
        m_smoothing.setAlgorithm(preset.smoothAlgorithm);
        m_smoothing.setTimeMs(preset.smoothTimeMs);
        m_preset = AudioPreset::Custom;
        m_currentPresetName = name;
        
        // Resize spectrum buffers
        m_spectrum.resize(m_bands, 0.0f);
        m_rawSpectrum.resize(m_bands, 0.0f);
    }
}

inline bool AudioSourceModule::deletePreset(const std::string& name)
{
    auto it = m_userPresets.find(name);
    if (it == m_userPresets.end())
    {
        return false;  // Not a user preset
    }
    
    m_userPresets.erase(it);
    
    // Delete .audio file
    if (!s_userPresetsDir.empty())
    {
        std::string filePath = s_userPresetsDir + "/" + name + ".audio";
        std::filesystem::remove(filePath);
    }
    
    return true;
}

inline bool AudioSourceModule::isUserPreset(const std::string& name) const
{
    return m_userPresets.find(name) != m_userPresets.end();
}

inline void AudioSourceModule::loadUserPresetsFromDisk()
{
    m_userPresetsLoaded = true;
    
    if (s_userPresetsDir.empty())
    {
        return;
    }
    
    if (!std::filesystem::exists(s_userPresetsDir))
    {
        return;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(s_userPresetsDir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        
        std::string ext = entry.path().extension().string();
        if (ext != ".audio")
        {
            continue;
        }
        
        AudioPresetData preset;
        if (parsePresetFile(entry.path().string(), preset))
        {
            m_userPresets[preset.name] = preset;
        }
    }
}

inline bool AudioSourceModule::parsePresetFile(const std::string& filePath, AudioPresetData& outPreset)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // Simple JSON parsing
    auto extractString = [&content](const std::string& key) -> std::string {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = content.find(searchKey);
        if (pos == std::string::npos) return "";
        pos = content.find("\"", pos + searchKey.length());
        if (pos == std::string::npos) return "";
        size_t end = content.find("\"", pos + 1);
        if (end == std::string::npos) return "";
        return content.substr(pos + 1, end - pos - 1);
    };
    
    auto extractFloat = [&content](const std::string& key) -> float {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = content.find(searchKey);
        if (pos == std::string::npos) return 0.0f;
        pos += searchKey.length();
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) ++pos;
        return std::stof(content.substr(pos));
    };
    
    auto extractInt = [&content](const std::string& key) -> int {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = content.find(searchKey);
        if (pos == std::string::npos) return 0;
        pos += searchKey.length();
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) ++pos;
        return std::stoi(content.substr(pos));
    };
    
    auto extractBool = [&content](const std::string& key) -> bool {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = content.find(searchKey);
        if (pos == std::string::npos) return false;
        return content.find("true", pos) < content.find(",", pos);
    };
    
    outPreset.name = extractString("name");
    if (outPreset.name.empty())
    {
        std::filesystem::path p(filePath);
        outPreset.name = p.stem().string();
    }
    
    outPreset.scale = static_cast<FrequencyScale>(extractInt("scale"));
    outPreset.bands = extractInt("bands");
    outPreset.floorDb = extractFloat("floorDb");
    outPreset.ceilDb = extractFloat("ceilDb");
    outPreset.clamp01 = extractBool("clamp01");
    outPreset.gain = extractFloat("gain");
    outPreset.smoothAlgorithm = static_cast<SmoothingAlgorithm>(extractInt("smoothAlgorithm"));
    outPreset.smoothTimeMs = extractFloat("smoothTimeMs");
    
    return true;
}

inline void AudioSourceModule::applyPreset(AudioPreset preset)
{
    m_preset = preset;
    
    switch (preset)
    {
    case AudioPreset::Default:
        m_scale = FrequencyScale::Log;
        m_bands = 64;
        m_floorDb = -60.0f;
        m_ceilDb = 0.0f;
        m_gain = 1.0f;
        m_smoothing.applyPreset(SmoothingPreset::Balanced);
        break;
        
    case AudioPreset::BassHeavy:
        m_scale = FrequencyScale::Log;
        m_bands = 32;
        m_floorDb = -50.0f;
        m_ceilDb = 0.0f;
        m_gain = 1.5f;
        m_smoothing.applyPreset(SmoothingPreset::Smooth);
        break;
        
    case AudioPreset::Vocals:
        m_scale = FrequencyScale::Mel;
        m_bands = 64;
        m_floorDb = -55.0f;
        m_ceilDb = -5.0f;
        m_gain = 1.2f;
        m_smoothing.applyPreset(SmoothingPreset::Balanced);
        break;
        
    case AudioPreset::Electronic:
        m_scale = FrequencyScale::Log;
        m_bands = 128;
        m_floorDb = -70.0f;
        m_ceilDb = 0.0f;
        m_gain = 1.0f;
        m_smoothing.applyPreset(SmoothingPreset::Reactive);
        break;
        
    case AudioPreset::Ambient:
        m_scale = FrequencyScale::Log;
        m_bands = 32;
        m_floorDb = -55.0f;
        m_ceilDb = -10.0f;
        m_gain = 0.8f;
        m_smoothing.applyPreset(SmoothingPreset::Sluggish);
        break;
        
    case AudioPreset::Custom:
        // Keep current values, don't change anything
        return;
    }
    
    // Resize spectrum buffers
    m_spectrum.resize(m_bands, 0.0f);
    m_rawSpectrum.resize(m_bands, 0.0f);
}

} // namespace lumi::modules

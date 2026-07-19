/**
 ****************************************************************************************
 * @file   SmoothingModule.hpp
 * @brief  Smoothing Module - Temporal smoothing algorithms for audio visualization
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * ## Overview
 *
 * SmoothingModule provides various temporal smoothing algorithms:
 *   - None (pass-through)
 *   - SMA (Simple Moving Average)
 *   - EMA (Exponential Moving Average) - recommended
 *   - WMA (Weighted Moving Average)
 *   - DEMA (Double Exponential Moving Average)
 *
 * ## Algorithm Comparison
 *
 * ```
 * Input:    ╱╲    ╱╲╱╲      ╱╲
 *          ╱  ╲  ╱    ╲    ╱  ╲
 *
 * None:     ╱╲    ╱╲╱╲      ╱╲     (identical)
 *
 * EMA:     ╱╲    ╱─╲       ╱╲     (rounded)
 *
 * SMA:    ╱──╲  ╱───╲     ╱──╲    (delayed + smooth)
 *
 * DEMA:    ╱╲   ╱╲╱╲      ╱╲       (less lag)
 * ```
 *
 * @see LumiPulse_VisualSystem_Architecture.md Section 4.2.1
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/IModule.hpp"
#include "visualizers/modules/JsonPresetParser.hpp"

#include <deque>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace lumi::modules
{

// =============================================================================
// Smoothing Algorithm Enum
// =============================================================================

/**
 * @brief Available smoothing algorithms
 */
enum class SmoothingAlgorithm
{
    None,   ///< No smoothing (pass-through)
    SMA,    ///< Simple Moving Average
    EMA,    ///< Exponential Moving Average (recommended)
    WMA,    ///< Weighted Moving Average
    DEMA    ///< Double Exponential Moving Average
};

/**
 * @brief Preset smoothing configurations
 */
enum class SmoothingPreset
{
    Instant,    ///< No smoothing (0 ms)
    Reactive,   ///< Very responsive (20 ms)
    Balanced,   ///< Good balance (50 ms) - default
    Smooth,     ///< Smooth motion (100 ms)
    Sluggish,   ///< Very smooth (200 ms)
    Custom      ///< User-modified settings
};

/**
 * @struct SmoothingPresetData
 * @brief Data structure for saveable smoothing presets
 */
struct SmoothingPresetData
{
    std::string name;
    SmoothingAlgorithm algorithm = SmoothingAlgorithm::EMA;
    float timeMs = 50.0f;           ///< For EMA/DEMA
    int windowSize = 8;             ///< For SMA/WMA
    bool primeFirstFrame = true;
};

// =============================================================================
// SmoothingModule
// =============================================================================

/**
 * @class SmoothingModule
 * @brief Temporal smoothing for audio-reactive values
 *
 * Processes float values through various smoothing algorithms.
 * Can be embedded in other modules (e.g., AudioSourceModule).
 *
 * @par Example Usage
 * @code
 * SmoothingModule smooth;
 * smooth.setAlgorithm(SmoothingAlgorithm::EMA);
 * smooth.setTimeMs(50.0f);
 *
 * // In update loop:
 * float smoothed = smooth.process(rawValue, deltaTime);
 * @endcode
 */
class SmoothingModule : public IModule
{
public:
    // =========================================================================
    // Construction
    // =========================================================================
    
    SmoothingModule();
    ~SmoothingModule() override = default;
    
    // =========================================================================
    // IModule Interface
    // =========================================================================
    
    [[nodiscard]] const char* moduleId() const override { return "smoothing"; }
    [[nodiscard]] const char* displayName() const override { return "Smoothing"; }
    [[nodiscard]] const char* category() const override { return "Processing"; }
    [[nodiscard]] const char* description() const override
    {
        return "Temporal smoothing algorithms (SMA/EMA/WMA/DEMA)";
    }
    
    [[nodiscard]] std::vector<ModuleParamDesc> paramDescs() const override;
    [[nodiscard]] bool getParam(const std::string& id, ParamValue& out) const override;
    bool setParam(const std::string& id, const ParamValue& value) override;
    void resetToDefaults() override;
    
    // =========================================================================
    // Processing
    // =========================================================================
    
    /**
     * @brief Process a single value through smoothing
     * @param value Raw input value
     * @param deltaTime Time since last update (seconds)
     * @return Smoothed value
     */
    float process(float value, float deltaTime);
    
    /**
     * @brief Process an array of values
     * @param values Input array
     * @param count Number of values
     * @param deltaTime Time since last update (seconds)
     * @param output Output array (must be pre-allocated)
     */
    void processArray(const float* values, int count, float deltaTime, float* output);

    /**
     * @brief Temporally smooth an array with INDEPENDENT per-index state
     *
     * For per-sample display smoothing (e.g. waveform buffers, E3): every index
     * keeps its own EMA state, primed with the first frame and re-primed when
     * the count changes. Uses the module's EMA time constant (timeMs);
     * algorithm None (or timeMs <= 0) passes through. SMA/WMA/DEMA fall back
     * to EMA here — per-index window buffers are not worth their memory.
     *
     * @param values Input array
     * @param count Number of values
     * @param deltaTime Time since last update (seconds)
     * @param output Output array (must be pre-allocated; may alias values)
     */
    void processArrayPerIndex(const float* values, int count, float deltaTime, float* output);

    /**
     * @brief Reset smoothing state (clears history)
     */
    void reset();
    
    /**
     * @brief Prime the smoother with initial value
     * @param value Initial value to fill history with
     */
    void prime(float value);
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    /**
     * @brief Set smoothing algorithm
     */
    void setAlgorithm(SmoothingAlgorithm algo);
    
    /**
     * @brief Get current algorithm
     */
    [[nodiscard]] SmoothingAlgorithm algorithm() const { return m_algorithm; }
    
    /**
     * @brief Set smoothing time in milliseconds
     * @param ms Smoothing time (0-500)
     * @note Only used by EMA and DEMA algorithms
     */
    void setTimeMs(float ms);
    
    /**
     * @brief Get smoothing time
     */
    [[nodiscard]] float timeMs() const { return m_timeMs; }
    
    /**
     * @brief Set window size for SMA/WMA algorithms
     * @param size Number of samples (2-60)
     * @note Only used by SMA and WMA algorithms
     */
    void setWindowSize(int size);
    
    /**
     * @brief Get window size
     */
    [[nodiscard]] int windowSize() const { return m_windowSize; }
    
    /**
     * @brief Apply a preset configuration
     */
    void applyPreset(SmoothingPreset preset);
    
    /**
     * @brief Get current preset (if matching)
     */
    [[nodiscard]] SmoothingPreset preset() const { return m_preset; }
    
    /**
     * @brief Enable/disable priming first frame
     */
    void setPrimeFirstFrame(bool enabled) { m_primeFirstFrame = enabled; }
    
    /**
     * @brief Check if priming is enabled
     */
    [[nodiscard]] bool primeFirstFrame() const { return m_primeFirstFrame; }
    
    // =========================================================================
    // Output
    // =========================================================================
    
    /**
     * @brief Get last smoothed value
     */
    [[nodiscard]] float lastValue() const { return m_lastOutput; }
    
    // =========================================================================
    // Utility
    // =========================================================================
    
    /**
     * @brief Get algorithm name as string
     */
    static const char* algorithmName(SmoothingAlgorithm algo);
    
    /**
     * @brief Get preset name as string
     */
    static const char* presetName(SmoothingPreset preset);
    
    /**
     * @brief Get all algorithm names
     */
    static std::vector<std::string> algorithmNames();
    
    /**
     * @brief Get all preset names (builtin + user)
     */
    std::vector<std::string> presetNames() const;
    
    /**
     * @brief Get builtin preset names only (static version)
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
    
private:
    // =========================================================================
    // Internal Processing
    // =========================================================================
    
    float processSMA(float value);
    float processEMA(float value, float deltaTime);
    float processWMA(float value);
    float processDEMA(float value, float deltaTime);
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    SmoothingAlgorithm m_algorithm = SmoothingAlgorithm::EMA;
    SmoothingPreset m_preset = SmoothingPreset::Balanced;
    float m_timeMs = 50.0f;
    bool m_primeFirstFrame = true;
    
    // =========================================================================
    // State
    // =========================================================================
    
    // SMA/WMA buffer
    std::deque<float> m_buffer;
    int m_windowSize = 8;
    
    // EMA state
    float m_emaValue = 0.0f;
    
    // DEMA state
    float m_demaValue1 = 0.0f;
    float m_demaValue2 = 0.0f;
    
    // General state
    float m_lastOutput = 0.0f;
    bool m_primed = false;

    // Per-index EMA state (processArrayPerIndex)
    std::vector<float> m_perIndexState;
    
    // =========================================================================
    // Constants
    // =========================================================================
    
    static constexpr int MIN_WINDOW_SIZE = 2;
    static constexpr int MAX_WINDOW_SIZE = 60;
    static constexpr float MIN_TIME_MS = 0.0f;
    static constexpr float MAX_TIME_MS = 500.0f;
    
    // =========================================================================
    // User Presets
    // =========================================================================
    
    std::string m_currentPresetName = "Balanced";
    std::map<std::string, SmoothingPresetData> m_userPresets;
    mutable bool m_userPresetsLoaded = false;
    
    void loadUserPresetsFromDisk();
    bool parsePresetFile(const std::string& filePath, SmoothingPresetData& outPreset);
    
    inline static std::string s_userPresetsDir;
};

// =============================================================================
// Implementation (Header-only for now)
// =============================================================================

inline SmoothingModule::SmoothingModule()
{
    resetToDefaults();
}

inline std::vector<ModuleParamDesc> SmoothingModule::paramDescs() const
{
    return {
        ParamBuilder("preset", ParamType::Enum)
            .displayName("Preset")
            .enumOptions(presetNames())
            .defaultValue(static_cast<int>(SmoothingPreset::Balanced))
            .tooltip("Predefined smoothing configurations")
            .subGroup("Smoothing")
            .order(0)
            .build(),
            
        ParamBuilder("algorithm", ParamType::Enum)
            .displayName("Algorithm")
            .enumOptions(algorithmNames())
            .defaultValue(static_cast<int>(SmoothingAlgorithm::EMA))
            .tooltip("Smoothing algorithm type:\n"
                     "• None: Pass-through (no smoothing)\n"
                     "• SMA: Simple Moving Average (window-based)\n"
                     "• EMA: Exponential Moving Average (time-based, recommended)\n"
                     "• WMA: Weighted Moving Average (window-based, newer samples weighted more)\n"
                     "• DEMA: Double EMA (time-based, reduced lag)")
            .subGroup("Smoothing")
            .order(1)
            .build(),
            
        ParamBuilder("timeMs", ParamType::Float)
            .displayName("Time Constant")
            .range(MIN_TIME_MS, MAX_TIME_MS, 1.0f)
            .defaultValue(50.0f)
            .unit("ms")
            .tooltip("Smoothing time constant (τ)\n"
                     "Higher = smoother but more lag\n"
                     "Lower = more responsive but jittery")
            .subGroup("Smoothing")
            .order(2)
            .dependsOn("algorithm", std::vector<ParamValue>{2, 4})  // EMA, DEMA only
            .build(),
            
        ParamBuilder("windowSize", ParamType::Int)
            .displayName("Window Size")
            .range(MIN_WINDOW_SIZE, MAX_WINDOW_SIZE, 1)
            .defaultValue(8)
            .unit("samples")
            .tooltip("Number of samples to average\n"
                     "Higher = smoother but more lag\n"
                     "Lower = more responsive but jittery")
            .subGroup("Smoothing")
            .order(3)
            .dependsOn("algorithm", std::vector<ParamValue>{1, 3})  // SMA, WMA only
            .build(),
            
        ParamBuilder("primeFirstFrame", ParamType::Bool)
            .displayName("Prime First Frame")
            .defaultValue(true)
            .tooltip("Initialize with first input value to avoid startup transients")
            .subGroup("Smoothing")
            .advanced(true)
            .order(10)
            .dependsOn("algorithm", std::vector<ParamValue>{1, 2, 3, 4})  // All except None
            .build()
    };
}

inline bool SmoothingModule::getParam(const std::string& id, ParamValue& out) const
{
    if (id == "algorithm")
    {
        out = static_cast<int>(m_algorithm);
        return true;
    }
    if (id == "timeMs")
    {
        out = m_timeMs;
        return true;
    }
    if (id == "windowSize")
    {
        out = m_windowSize;
        return true;
    }
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
    if (id == "primeFirstFrame")
    {
        out = m_primeFirstFrame;
        return true;
    }
    return false;
}

inline bool SmoothingModule::setParam(const std::string& id, const ParamValue& value)
{
    // Helper to extract int (from int or float)
    auto getInt = [&value]() -> std::optional<int> {
        if (std::holds_alternative<int>(value))
            return std::get<int>(value);
        if (std::holds_alternative<float>(value))
            return static_cast<int>(std::get<float>(value));
        return std::nullopt;
    };
    
    // Helper to extract float (from int or float)
    auto getFloat = [&value]() -> std::optional<float> {
        if (std::holds_alternative<float>(value))
            return std::get<float>(value);
        if (std::holds_alternative<int>(value))
            return static_cast<float>(std::get<int>(value));
        return std::nullopt;
    };
    
    // Helper to extract bool
    auto getBool = [&value]() -> std::optional<bool> {
        if (std::holds_alternative<bool>(value))
            return std::get<bool>(value);
        if (std::holds_alternative<int>(value))
            return std::get<int>(value) != 0;
        return std::nullopt;
    };

    if (id == "algorithm")
    {
        if (auto v = getInt())
        {
            setAlgorithm(static_cast<SmoothingAlgorithm>(*v));
            m_preset = SmoothingPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "timeMs")
    {
        if (auto v = getFloat())
        {
            setTimeMs(*v);
            m_preset = SmoothingPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "windowSize")
    {
        if (auto v = getInt())
        {
            setWindowSize(*v);
            m_preset = SmoothingPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    else if (id == "preset")
    {
        if (auto idx = getInt())
        {
            auto names = presetNames();
            if (*idx >= 0 && *idx < static_cast<int>(names.size()))
            {
                loadPreset(names[*idx]);
            }
            return true;
        }
    }
    else if (id == "primeFirstFrame")
    {
        if (auto v = getBool())
        {
            m_primeFirstFrame = *v;
            m_preset = SmoothingPreset::Custom;
            m_currentPresetName = "[Custom]";
            return true;
        }
    }
    return false;
}

inline void SmoothingModule::resetToDefaults()
{
    m_algorithm = SmoothingAlgorithm::EMA;
    m_preset = SmoothingPreset::Balanced;
    m_timeMs = 50.0f;
    m_windowSize = 8;
    m_primeFirstFrame = true;
    m_currentPresetName = "Balanced";
    reset();
}

inline float SmoothingModule::process(float value, float deltaTime)
{
    // Prime on first call if enabled
    if (!m_primed && m_primeFirstFrame)
    {
        prime(value);
    }
    
    float result = 0.0f;
    
    switch (m_algorithm)
    {
    case SmoothingAlgorithm::None:
        result = value;
        break;
        
    case SmoothingAlgorithm::SMA:
        result = processSMA(value);
        break;
        
    case SmoothingAlgorithm::EMA:
        result = processEMA(value, deltaTime);
        break;
        
    case SmoothingAlgorithm::WMA:
        result = processWMA(value);
        break;
        
    case SmoothingAlgorithm::DEMA:
        result = processDEMA(value, deltaTime);
        break;
    }
    
    m_lastOutput = result;
    return result;
}

inline void SmoothingModule::processArray(const float* values, int count,
                                          float deltaTime, float* output)
{
    for (int i = 0; i < count; ++i)
    {
        output[i] = process(values[i], deltaTime);
    }
}

inline void SmoothingModule::processArrayPerIndex(const float* values, int count,
                                                  float deltaTime, float* output)
{
    if (count <= 0)
    {
        return;
    }

    if (m_algorithm == SmoothingAlgorithm::None || m_timeMs <= 0.0f || deltaTime <= 0.0f)
    {
        m_perIndexState.assign(values, values + count);
        std::copy(values, values + count, output);
        return;
    }

    // Prime with the first frame (also on count change)
    if (static_cast<int>(m_perIndexState.size()) != count)
    {
        m_perIndexState.assign(values, values + count);
    }

    // Same EMA math as processEMA(): α = 1 - e^(-Δt/τ), τ = timeMs / 1000
    const float tau = m_timeMs / 1000.0f;
    const float alpha = 1.0f - std::exp(-deltaTime / tau);

    for (int i = 0; i < count; ++i)
    {
        m_perIndexState[i] = alpha * values[i] + (1.0f - alpha) * m_perIndexState[i];
        output[i] = m_perIndexState[i];
    }
}

inline void SmoothingModule::reset()
{
    m_buffer.clear();
    m_emaValue = 0.0f;
    m_demaValue1 = 0.0f;
    m_demaValue2 = 0.0f;
    m_lastOutput = 0.0f;
    m_primed = false;
    m_perIndexState.clear();
}

inline void SmoothingModule::prime(float value)
{
    m_emaValue = value;
    m_demaValue1 = value;
    m_demaValue2 = value;
    m_lastOutput = value;
    
    // Fill buffer for SMA/WMA
    m_buffer.clear();
    for (int i = 0; i < m_windowSize; ++i)
    {
        m_buffer.push_back(value);
    }
    
    m_primed = true;
}

inline void SmoothingModule::setAlgorithm(SmoothingAlgorithm algo)
{
    if (m_algorithm != algo)
    {
        m_algorithm = algo;
        // Keep current value as starting point
        prime(m_lastOutput);
    }
}

inline void SmoothingModule::setTimeMs(float ms)
{
    m_timeMs = std::clamp(ms, MIN_TIME_MS, MAX_TIME_MS);
}

inline void SmoothingModule::setWindowSize(int size)
{
    m_windowSize = std::clamp(size, MIN_WINDOW_SIZE, MAX_WINDOW_SIZE);
    
    // Resize buffer if needed
    while (static_cast<int>(m_buffer.size()) > m_windowSize)
    {
        m_buffer.pop_front();
    }
}

inline void SmoothingModule::applyPreset(SmoothingPreset preset)
{
    m_preset = preset;
    
    switch (preset)
    {
    case SmoothingPreset::Instant:
        m_algorithm = SmoothingAlgorithm::None;
        m_timeMs = 0.0f;
        m_windowSize = 2;
        break;
        
    case SmoothingPreset::Reactive:
        m_algorithm = SmoothingAlgorithm::EMA;
        m_timeMs = 20.0f;
        m_windowSize = 3;    // ~50ms @60fps
        break;
        
    case SmoothingPreset::Balanced:
        m_algorithm = SmoothingAlgorithm::EMA;
        m_timeMs = 50.0f;
        m_windowSize = 8;    // ~133ms @60fps
        break;
        
    case SmoothingPreset::Smooth:
        m_algorithm = SmoothingAlgorithm::EMA;
        m_timeMs = 100.0f;
        m_windowSize = 12;   // ~200ms @60fps
        break;
        
    case SmoothingPreset::Sluggish:
        m_algorithm = SmoothingAlgorithm::DEMA;
        m_timeMs = 200.0f;
        m_windowSize = 20;   // ~333ms @60fps
        break;
        
    case SmoothingPreset::Custom:
        // Custom: keep current values, don't change anything
        return;
    }
}

inline float SmoothingModule::processSMA(float value)
{
    m_buffer.push_back(value);
    
    while (static_cast<int>(m_buffer.size()) > m_windowSize)
    {
        m_buffer.pop_front();
    }
    
    if (m_buffer.empty())
    {
        return value;
    }
    
    float sum = std::accumulate(m_buffer.begin(), m_buffer.end(), 0.0f);
    return sum / static_cast<float>(m_buffer.size());
}

inline float SmoothingModule::processEMA(float value, float deltaTime)
{
    if (m_timeMs <= 0.0f || deltaTime <= 0.0f)
    {
        m_emaValue = value;
        return value;
    }
    
    // Time constant τ = timeMs / 1000
    // α = 1 - e^(-deltaTime / τ)
    float tau = m_timeMs / 1000.0f;
    float alpha = 1.0f - std::exp(-deltaTime / tau);
    
    m_emaValue = alpha * value + (1.0f - alpha) * m_emaValue;
    return m_emaValue;
}

inline float SmoothingModule::processWMA(float value)
{
    m_buffer.push_back(value);
    
    while (static_cast<int>(m_buffer.size()) > m_windowSize)
    {
        m_buffer.pop_front();
    }
    
    if (m_buffer.empty())
    {
        return value;
    }
    
    float weightSum = 0.0f;
    float valueSum = 0.0f;
    int weight = 1;
    
    for (float val : m_buffer)
    {
        valueSum += val * static_cast<float>(weight);
        weightSum += static_cast<float>(weight);
        ++weight;
    }
    
    return valueSum / weightSum;
}

inline float SmoothingModule::processDEMA(float value, float deltaTime)
{
    if (m_timeMs <= 0.0f || deltaTime <= 0.0f)
    {
        m_demaValue1 = value;
        m_demaValue2 = value;
        return value;
    }
    
    float tau = m_timeMs / 1000.0f;
    float alpha = 1.0f - std::exp(-deltaTime / tau);
    
    // Double EMA: EMA of EMA
    m_demaValue1 = alpha * value + (1.0f - alpha) * m_demaValue1;
    m_demaValue2 = alpha * m_demaValue1 + (1.0f - alpha) * m_demaValue2;
    
    // DEMA = 2 * EMA1 - EMA2 (reduces lag)
    return 2.0f * m_demaValue1 - m_demaValue2;
}

inline const char* SmoothingModule::algorithmName(SmoothingAlgorithm algo)
{
    switch (algo)
    {
    case SmoothingAlgorithm::None: return "None";
    case SmoothingAlgorithm::SMA:  return "SMA";
    case SmoothingAlgorithm::EMA:  return "EMA";
    case SmoothingAlgorithm::WMA:  return "WMA";
    case SmoothingAlgorithm::DEMA: return "DEMA";
    default: return "Unknown";
    }
}

inline const char* SmoothingModule::presetName(SmoothingPreset preset)
{
    switch (preset)
    {
    case SmoothingPreset::Instant:  return "Instant";
    case SmoothingPreset::Reactive: return "Reactive";
    case SmoothingPreset::Balanced: return "Balanced";
    case SmoothingPreset::Smooth:   return "Smooth";
    case SmoothingPreset::Sluggish: return "Sluggish";
    default: return "Unknown";
    }
}

inline std::vector<std::string> SmoothingModule::algorithmNames()
{
    return {"None", "SMA", "EMA", "WMA", "DEMA"};
}

inline std::vector<std::string> SmoothingModule::builtinPresetNames()
{
    return {"Instant", "Reactive", "Balanced", "Smooth", "Sluggish"};
}

inline std::vector<std::string> SmoothingModule::presetNames() const
{
    // Lazy-load user presets
    if (!m_userPresetsLoaded && !s_userPresetsDir.empty())
    {
        const_cast<SmoothingModule*>(this)->loadUserPresetsFromDisk();
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

inline void SmoothingModule::setUserPresetsDirectory(const std::string& dir)
{
    s_userPresetsDir = dir;
}

inline void SmoothingModule::savePreset(const std::string& name)
{
    SmoothingPresetData preset;
    preset.name = name;
    preset.algorithm = m_algorithm;
    preset.timeMs = m_timeMs;
    preset.windowSize = m_windowSize;
    preset.primeFirstFrame = m_primeFirstFrame;
    
    m_userPresets[name] = preset;
    m_currentPresetName = name;
    m_preset = SmoothingPreset::Custom;  // Now it's a named custom preset
    
    // Save to .smooth file
    if (s_userPresetsDir.empty())
    {
        return;
    }
    
    std::filesystem::create_directories(s_userPresetsDir);
    std::string filePath = s_userPresetsDir + "/" + name + ".smooth";
    
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        return;
    }
    
    // Write JSON
    file << "{\n";
    file << "  \"name\": \"" << name << "\",\n";
    file << "  \"algorithm\": " << static_cast<int>(preset.algorithm) << ",\n";
    file << "  \"timeMs\": " << preset.timeMs << ",\n";
    file << "  \"windowSize\": " << preset.windowSize << ",\n";
    file << "  \"primeFirstFrame\": " << (preset.primeFirstFrame ? "true" : "false") << "\n";
    file << "}\n";
    file.close();
}

inline void SmoothingModule::loadPreset(const std::string& name)
{
    // Skip [Custom] and separator
    if (name == "[Custom]" || name == "---")
    {
        return;
    }
    
    // Check builtin presets first
    if (name == "Instant")
    {
        applyPreset(SmoothingPreset::Instant);
        m_currentPresetName = name;
        return;
    }
    if (name == "Reactive")
    {
        applyPreset(SmoothingPreset::Reactive);
        m_currentPresetName = name;
        return;
    }
    if (name == "Balanced")
    {
        applyPreset(SmoothingPreset::Balanced);
        m_currentPresetName = name;
        return;
    }
    if (name == "Smooth")
    {
        applyPreset(SmoothingPreset::Smooth);
        m_currentPresetName = name;
        return;
    }
    if (name == "Sluggish")
    {
        applyPreset(SmoothingPreset::Sluggish);
        m_currentPresetName = name;
        return;
    }
    
    // Check user presets
    auto it = m_userPresets.find(name);
    if (it != m_userPresets.end())
    {
        const auto& preset = it->second;
        m_algorithm = preset.algorithm;
        m_timeMs = preset.timeMs;
        m_windowSize = preset.windowSize;
        m_primeFirstFrame = preset.primeFirstFrame;
        m_preset = SmoothingPreset::Custom;
        m_currentPresetName = name;
    }
}

inline bool SmoothingModule::deletePreset(const std::string& name)
{
    auto it = m_userPresets.find(name);
    if (it == m_userPresets.end())
    {
        return false;  // Not a user preset
    }
    
    m_userPresets.erase(it);
    
    // Delete .smooth file
    if (!s_userPresetsDir.empty())
    {
        std::string filePath = s_userPresetsDir + "/" + name + ".smooth";
        std::filesystem::remove(filePath);
    }
    
    return true;
}

inline bool SmoothingModule::isUserPreset(const std::string& name) const
{
    return m_userPresets.find(name) != m_userPresets.end();
}

inline void SmoothingModule::loadUserPresetsFromDisk()
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
        if (ext != ".smooth")
        {
            continue;
        }
        
        SmoothingPresetData preset;
        if (parsePresetFile(entry.path().string(), preset))
        {
            m_userPresets[preset.name] = preset;
        }
    }
}

inline bool SmoothingModule::parsePresetFile(const std::string& filePath, SmoothingPresetData& outPreset)
{
    // Shared preset-JSON extraction (5.6) — one parser for all module presets
    auto parser = JsonPresetParser::fromFile(filePath);
    if (!parser)
    {
        return false;
    }

    outPreset.name = parser->getString("name");
    if (outPreset.name.empty())
    {
        // Use filename as fallback
        std::filesystem::path p(filePath);
        outPreset.name = p.stem().string();
    }

    outPreset.algorithm = static_cast<SmoothingAlgorithm>(parser->getInt("algorithm"));
    outPreset.timeMs = parser->getFloat("timeMs");
    outPreset.windowSize = parser->getInt("windowSize", 8);  // Default 8 for old presets
    outPreset.primeFirstFrame = parser->getBool("primeFirstFrame");

    return true;
}

} // namespace lumi::modules

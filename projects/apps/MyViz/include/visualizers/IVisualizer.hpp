/**
 ****************************************************************************************
 * @file   IVisualizer.hpp
 * @brief  Interface for all visualizers
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0 - Added module parameter introspection
 *
 * @details
 * ## Qt6 Tutorial: Visualizer Interface
 *
 * Alle Visualizer implementieren dieses Interface:
 *   - Eindeutige ID und Name
 *   - OpenGL Lifecycle (initialize, render, resize)
 *   - Optional: Audio-Daten empfangen
 *   - Modul-Parameter für ConfigPanel
 *
 * ### Wichtig
 *
 * Visualizer sind KEINE Panels! Sie rendern im zentralen VisualizerWidget.
 * Das VisualSelectPanel wechselt nur den aktiven Visualizer.
 ****************************************************************************************
 */

#pragma once

#include <QString>
#include <QSize>
#include <vector>
#include <string>

#include "visualizers/modules/IModule.hpp"

namespace lumi::modules
{
class ColorGradientModule;
class AudioSourceModule;
}

/**
 * @class IVisualizer
 * @brief Interface for OpenGL visualizers
 *
 * Visualizers render audio-reactive graphics in the main visualization area.
 */
class IVisualizer
{
public:
    virtual ~IVisualizer() = default;

    // =========================================================================
    // Identification
    // =========================================================================

    /**
     * @brief Get visualizer unique identifier
     * @return Visualizer ID (e.g., "spectrum", "waveform", "pulsing")
     */
    [[nodiscard]] virtual QString visualizerId() const = 0;

    /**
     * @brief Get visualizer display name
     * @return Name shown in visualizer selector
     */
    [[nodiscard]] virtual QString visualizerName() const = 0;

    /**
     * @brief Get visualizer description
     * @return Brief description of the visualization
     */
    [[nodiscard]] virtual QString visualizerDescription() const = 0;

    // =========================================================================
    // OpenGL Lifecycle
    // =========================================================================

    /**
     * @brief Initialize OpenGL resources
     *
     * Called once when visualizer is first activated.
     * OpenGL context is current when this is called.
     */
    virtual void initialize() = 0;

    /**
     * @brief Render a frame
     * @param deltaTime Time since last frame in seconds
     *
     * OpenGL context is current when this is called.
     */
    virtual void render(float deltaTime) = 0;

    /**
     * @brief Handle resize
     * @param size New viewport size
     *
     * Called when the visualization area is resized.
     */
    virtual void resize(const QSize& size) = 0;

    /**
     * @brief Cleanup OpenGL resources
     *
     * Called when visualizer is deactivated or destroyed.
     * OpenGL context is current when this is called.
     */
    virtual void cleanup() = 0;

    // =========================================================================
    // State
    // =========================================================================

    /**
     * @brief Check if visualizer is initialized
     */
    [[nodiscard]] virtual bool isInitialized() const = 0;

    // =========================================================================
    // Audio Data (Optional)
    // =========================================================================

    /**
     * @brief Update with audio spectrum data
     * @param spectrum Frequency spectrum data (0.0 - 1.0)
     * @param count Number of spectrum bands
     *
     * Called from audio thread - must be thread-safe!
     * Default implementation does nothing.
     */
    virtual void updateSpectrum(const float* spectrum, int count)
    {
        Q_UNUSED(spectrum)
        Q_UNUSED(count)
    }

    /**
     * @brief Update with audio waveform data
     * @param waveform Waveform samples (-1.0 to 1.0)
     * @param count Number of samples
     *
     * Called from audio thread - must be thread-safe!
     * Default implementation does nothing.
     */
    virtual void updateWaveform(const float* waveform, int count)
    {
        Q_UNUSED(waveform)
        Q_UNUSED(count)
    }

    // =========================================================================
    // Module Parameter Introspection (for ConfigPanel)
    // =========================================================================

    /**
     * @brief Check if visualizer supports parameter introspection
     * @return true if paramDescs/getParam/setParam are implemented
     */
    [[nodiscard]] virtual bool hasParameterSupport() const { return false; }

    /**
     * @brief Get all parameter descriptors from all modules
     * @return Vector of parameter descriptors
     *
     * Parameters are grouped by module and can have hierarchical IDs
     * like "audio.smooth.timeMs".
     */
    [[nodiscard]] virtual std::vector<lumi::modules::ModuleParamDesc> paramDescs() const
    {
        return {};
    }

    /**
     * @brief Get parameter value by hierarchical path
     * @param id Parameter path (e.g., "audio.smooth.timeMs", "color.scheme")
     * @param out Output value
     * @return true if parameter found
     */
    [[nodiscard]] virtual bool getParam(const std::string& id,
                                        lumi::modules::ParamValue& out) const
    {
        Q_UNUSED(id)
        Q_UNUSED(out)
        return false;
    }

    /**
     * @brief Set parameter value by hierarchical path
     * @param id Parameter path
     * @param value New value
     * @return true if parameter set successfully
     */
    virtual bool setParam(const std::string& id,
                          const lumi::modules::ParamValue& value)
    {
        Q_UNUSED(id)
        Q_UNUSED(value)
        return false;
    }

    /**
     * @brief Reset all parameters to defaults
     */
    virtual void resetToDefaults() {}

    // =========================================================================
    // Gradient Handles (Phase 4)
    // =========================================================================

    /**
     * @brief A named color gradient exposed by this visualizer
     *
     * The generic UI (gradient editor, preview, module-preset save) works
     * exclusively through these handles — no dynamic_cast on concrete
     * visualizer types. `paramPrefix` is the parameter-ID prefix of the
     * gradient's parameters (e.g. "shape.color." / "ch1Color.").
     */
    struct GradientHandle
    {
        std::string id;                                ///< e.g. "main", "left", "ch1"
        std::string displayName;                       ///< e.g. "Left Channel"
        std::string paramPrefix;                       ///< param-ID prefix incl. trailing '.'
        lumi::modules::ColorGradientModule* gradient;  ///< non-owning, visualizer-owned
    };

    /**
     * @brief All gradients of this visualizer (empty if it has none)
     *
     * Pointers stay valid for the visualizer's lifetime.
     */
    [[nodiscard]] virtual std::vector<GradientHandle> gradients() { return {}; }

    /**
     * @brief The visualizer's audio-source module (generic module-preset access)
     *
     * Analogous to gradients(): the ConfigPanel saves/loads audio and
     * smoothing module presets exclusively through this accessor — no
     * dynamic_cast on concrete visualizer types.
     *
     * @return nullptr if the visualizer has no audio-source module
     */
    [[nodiscard]] virtual lumi::modules::AudioSourceModule* audioSourceModule() { return nullptr; }

    // =========================================================================
    // Tap Points (Phase 4 — preview foundation)
    // =========================================================================

    /**
     * @brief A named probe on a pipeline-stage output (pull-based)
     *
     * `sample()` copies the stage's current output; it costs nothing unless
     * called (the preview polls only while visible). Main/UI thread only.
     */
    struct TapPoint
    {
        std::string id;                            ///< e.g. "tap.audio", "tap.map"
        std::string displayName;                   ///< e.g. "Analyse (Baender)"
        lumi::modules::PipelineStage stage = lumi::modules::PipelineStage::None;
        std::function<std::vector<float>()> sample;  ///< copy of current data
    };

    /// @brief All tap points of this visualizer (empty if none wired yet)
    [[nodiscard]] virtual std::vector<TapPoint> tapPoints() { return {}; }
};

/**
 ****************************************************************************************
 * @file   VisualizerBase.hpp
 * @brief  Base class for OpenGL visualizers
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Visualizer-Basisklasse
 *
 * VisualizerBase bietet:
 *   - IVisualizer-Implementierung
 *   - Viewport-Verwaltung
 *   - Initialisierungs-Tracking
 *   - Thread-sichere Audio-Daten Puffer
 *
 * ### Verwendung
 *
 * ```cpp
 * class PulsingVisualizer : public VisualizerBase {
 * public:
 *     PulsingVisualizer()
 *         : VisualizerBase("pulsing", tr("Pulsing"), 
 *                          tr("Simple pulsing effect"))
 *     {}
 *
 * protected:
 *     void onInitialize() override {
 *         // Setup shaders, buffers
 *     }
 *
 *     void onRender(float deltaTime) override {
 *         // Draw visualization
 *     }
 *
 *     void onResize(const QSize& size) override {
 *         // Update projection
 *     }
 *
 *     void onCleanup() override {
 *         // Free resources
 *     }
 * };
 *
 * // Self-registration:
 * REGISTER_VISUALIZER("pulsing", "Pulsing", "Simple pulsing effect", PulsingVisualizer)
 * ```
 ****************************************************************************************
 */

#pragma once

#include "IVisualizer.hpp"

#include <QString>
#include <QSize>
#include <QMutex>
#include <vector>

// Forward declaration
class ServiceContainer;

/**
 * @class VisualizerBase
 * @brief Abstract base class for visualizers
 *
 * Provides common functionality for all visualizers.
 */
class VisualizerBase : public IVisualizer
{
public:
    /**
     * @brief Construct a visualizer
     * @param id Unique visualizer identifier
     * @param name Display name
     * @param description Brief description
     */
    explicit VisualizerBase(const QString& id,
                            const QString& name,
                            const QString& description);

    ~VisualizerBase() override = default;

    // =========================================================================
    // IVisualizer Implementation
    // =========================================================================

    [[nodiscard]] QString visualizerId() const override { return m_id; }
    [[nodiscard]] QString visualizerName() const override { return m_name; }
    [[nodiscard]] QString visualizerDescription() const override { return m_description; }

    void initialize() final;
    void render(float deltaTime) final;
    void resize(const QSize& size) final;
    void cleanup() final;

    [[nodiscard]] bool isInitialized() const override { return m_initialized; }

    // Audio data (thread-safe)
    void updateSpectrum(const float* spectrum, int count) override;
    void updateWaveform(const float* waveform, int count) override;

    /**
     * @brief Feed per-channel (stereo) audio. `specInterleaved` holds
     *        binsPerCh × channels FFT bins (bin*channels + ch); `waveInterleaved`
     *        holds frames × channels samples. De-interleaved into L/R buffers.
     *        Channels < 2 → L and R receive the same (mono) data.
     */
    void updateAudioStereo(const float* specInterleaved, int binsPerCh,
                           const float* waveInterleaved, int frames,
                           int channels) override;

protected:
    // =========================================================================
    // Override Points
    // =========================================================================

    /**
     * @brief Initialize OpenGL resources
     *
     * Override to setup shaders, buffers, textures.
     */
    virtual void onInitialize() = 0;

    /**
     * @brief Render a frame
     * @param deltaTime Time since last frame in seconds
     */
    virtual void onRender(float deltaTime) = 0;

    /**
     * @brief Handle resize
     * @param size New viewport size
     */
    virtual void onResize(const QSize& size) = 0;

    /**
     * @brief Cleanup OpenGL resources
     */
    virtual void onCleanup() = 0;

    // =========================================================================
    // Protected Accessors
    // =========================================================================

    /**
     * @brief Get current viewport size
     */
    [[nodiscard]] QSize viewportSize() const { return m_viewportSize; }

    /**
     * @brief Get viewport width
     */
    [[nodiscard]] int width() const { return m_viewportSize.width(); }

    /**
     * @brief Get viewport height
     */
    [[nodiscard]] int height() const { return m_viewportSize.height(); }

    /**
     * @brief Get aspect ratio
     */
    [[nodiscard]] float aspectRatio() const;

    // =========================================================================
    // Audio Data Access (thread-safe copies)
    // =========================================================================

    /**
     * @brief Get copy of current spectrum data
     * @return Spectrum data (may be empty)
     */
    [[nodiscard]] std::vector<float> getSpectrum() const;

    /**
     * @brief Get copy of current waveform data
     * @return Waveform data (may be empty)
     */
    [[nodiscard]] std::vector<float> getWaveform() const;

    /** @brief Per-channel spectrum/waveform (fall back to the mono copy if no
     *  stereo data has been fed). Channel 0 = left, 1 = right. */
    [[nodiscard]] std::vector<float> getSpectrumChannel(int channel) const;
    [[nodiscard]] std::vector<float> getWaveformChannel(int channel) const;

    /**
     * @brief Check if new audio data is available
     * @return true if audio data was updated since last check
     */
    [[nodiscard]] bool hasNewAudioData();

private:
    QString m_id;
    QString m_name;
    QString m_description;

    bool m_initialized = false;
    QSize m_viewportSize{800, 600};

    // Thread-safe audio buffers
    mutable QMutex m_audioMutex;
    std::vector<float> m_spectrum;
    std::vector<float> m_waveform;
    std::vector<float> m_spectrumL, m_spectrumR;  // per-channel (empty = no stereo)
    std::vector<float> m_waveformL, m_waveformR;
    bool m_hasNewAudioData = false;
};

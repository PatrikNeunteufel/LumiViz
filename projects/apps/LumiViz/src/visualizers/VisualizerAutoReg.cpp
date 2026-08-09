/**
 ****************************************************************************************
 * @file   VisualizerAutoReg.cpp
 * @brief  Application-specific visualizer registrations for LumiViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * Diese Datei definiert die **LumiViz-spezifischen Visualizer**.
 * Sie wird automatisch von VisualizerRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Visualizer-Liste
 *
 * | ID | Name | Kategorie |
 * |----|------|-----------|
 * | pulsing | Pulsing | Basic |
 * | waveform | Waveform | Waveform |
 * | oscilloscope | Oscilloscope | Waveform |
 * | superscope | Superscope | Waveform |
 *
 * ## Trennung Framework vs. App
 *
 * - **VisualizerRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **VisualizerAutoReg.cpp** = LumiViz-spezifische Visualizer (diese Datei)
 *
 * ## Linker-Garantie
 *
 * VisualizerRegistry.cpp deklariert `extern void initVisualizerDefaults(VisualizerRegistry&)`.
 * Diese Referenz erzwingt, dass der Linker VisualizerAutoReg.cpp einbindet,
 * auch bei statischen Libraries.
 *
 * ## Neue Visualizer hinzufügen
 *
 * 1. Visualizer-Klasse erstellen (z.B. SpectrumVisualizer)
 * 2. Header hier inkludieren
 * 3. Registrierung in initVisualizerDefaults() hinzufügen
 ****************************************************************************************
 */

#include "services/VisualizerRegistry.hpp"
#include "visualizers/EqualizerVisualizer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/OscilloscopeVisualizer.hpp"
#include "visualizers/PulsingVisualizer.hpp"
#include "visualizers/SuperscopeVisualizer.hpp"
#include "visualizers/WaveformVisualizer.hpp"
// TODO: Zukünftige Visualizer hier inkludieren:
// #include "visualizers/ParticleVisualizer.hpp"

// =============================================================================
// initVisualizerDefaults - Called by VisualizerRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default visualizers for LumiViz
 * 
 * @param registry Reference to the VisualizerRegistry singleton
 * 
 * This function is called automatically by VisualizerRegistry::instance() on first access.
 * It registers all available visualizers.
 */
void initVisualizerDefaults(VisualizerRegistry& registry)
{
    // =========================================================================
    // BASIC CATEGORY
    // =========================================================================
    
    // Pulsing Visualizer - Audio-reactive pulsing shape
    registry.registerVisualizer(
        VisualizerDescriptor{
            "pulsing",                                           // id
            "Pulsing",                                           // name
            "Audio-reactive pulsing shape with beat detection",  // description
            "shape",                                             // category
            100                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<PulsingVisualizer>();
        },
        false);
    
    // =========================================================================
    // SPECTRUM CATEGORY
    // =========================================================================
    
    // Equalizer Visualizer - FFT-based spectrum bars with peak markers
    registry.registerVisualizer(
        VisualizerDescriptor{
            "equalizer",                                           // id
            "Equalizer",                                           // name
            "Spectrum analyzer with bars and peak markers",        // description
            "spectrum",                                            // category
            100                                                    // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<EqualizerVisualizer>();
        },
        false);
    
    // =========================================================================
    // WAVEFORM CATEGORY
    // =========================================================================
    
    // Waveform Visualizer - Audio waveform display
    registry.registerVisualizer(
        VisualizerDescriptor{
            "waveform",                                          // id
            "Waveform",                                          // name
            "Real-time audio waveform oscilloscope display",     // description
            "waveform",                                          // category
            100                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<WaveformVisualizer>();
        },
        false);
    
    // Oscilloscope Visualizer - Classic oscilloscope with trigger
    registry.registerVisualizer(
        VisualizerDescriptor{
            "oscilloscope",                                      // id
            "Oscilloscope",                                      // name
            "Classic oscilloscope with trigger synchronization", // description
            "waveform",                                          // category
            200                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<OscilloscopeVisualizer>();
        },
        false);
    
    // Superscope Visualizer - Programmable point/line visualizer
    registry.registerVisualizer(
        VisualizerDescriptor{
            "superscope",                                        // id
            "Superscope",                                        // name
            "Programmable point/line visualizer with presets",   // description
            "waveform",                                          // category
            300                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<SuperscopeVisualizer>();
        },
        false);
    
    // =========================================================================
    // EFFECTS CATEGORY
    // =========================================================================

    // Multi Effect Host - AVS-style effect chain (import target, Roadmap 5)
    registry.registerVisualizer(
        VisualizerDescriptor{
            "multieffect",                                       // id
            "Multi Effect",                                      // name
            "AVS-style effect chain host (import target)",       // description
            "effects",                                           // category
            100                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<MultiEffectVisualizer>();
        },
        false);

    // Milkdrop: KEIN eigener Registry-Eintrag mehr (N2, Entscheid E2) — die
    // Klasse lebt als Chain-Node-Engine im MultiEffect-Host weiter
    // (runMilkdropNode) und im Standalone-Testprogramm MilkdropStandalone.

    // =========================================================================
    // PARTICLE CATEGORY (TODO)
    // =========================================================================
    
    // TODO: Particle Visualizer - Audio-reactive particles
    // registry.registerVisualizer(
    //     VisualizerDescriptor{
    //         "particles", "Particle Storm",
    //         "Audio-reactive particle system", "Effects", 100
    //     },
    //     []() { return std::make_unique<ParticleVisualizer>(); },
    //     false);
}

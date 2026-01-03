/**
 ****************************************************************************************
 * @file   VisualizerAutoReg.cpp
 * @brief  Application-specific visualizer registrations for MyViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * Diese Datei definiert die **MyViz-spezifischen Visualizer**.
 * Sie wird automatisch von VisualizerRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Visualizer-Liste
 *
 * | ID | Name | Kategorie |
 * |----|------|-----------|
 * | pulsing | Pulsing | Basic |
 * | waveform | Waveform | Waveform |
 * | oscilloscope | Oscilloscope | Waveform |
 *
 * ## Trennung Framework vs. App
 *
 * - **VisualizerRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **VisualizerAutoReg.cpp** = MyViz-spezifische Visualizer (diese Datei)
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
#include "visualizers/OscilloscopeVisualizer.hpp"
#include "visualizers/PulsingVisualizer.hpp"
#include "visualizers/WaveformVisualizer.hpp"
// TODO: Zukünftige Visualizer hier inkludieren:
// #include "visualizers/SpectrumVisualizer.hpp"
// #include "visualizers/ParticleVisualizer.hpp"

// =============================================================================
// initVisualizerDefaults - Called by VisualizerRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default visualizers for MyViz
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
            "Basic",                                             // category
            100                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<PulsingVisualizer>();
        },
        false);
    
    // =========================================================================
    // SPECTRUM CATEGORY (TODO)
    // =========================================================================
    
    // TODO: Spectrum Visualizer - FFT-based spectrum bars
    // registry.registerVisualizer(
    //     VisualizerDescriptor{
    //         "spectrum", "Spectrum Analyzer",
    //         "FFT-based spectrum visualization", "Spectrum", 100
    //     },
    //     []() { return std::make_unique<SpectrumVisualizer>(); },
    //     false);
    
    // =========================================================================
    // WAVEFORM CATEGORY
    // =========================================================================
    
    // Waveform Visualizer - Audio waveform display
    registry.registerVisualizer(
        VisualizerDescriptor{
            "waveform",                                          // id
            "Waveform",                                          // name
            "Real-time audio waveform oscilloscope display",     // description
            "Waveform",                                          // category
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
            "Waveform",                                          // category
            200                                                  // order
        },
        []() -> std::unique_ptr<IVisualizer> {
            return std::make_unique<OscilloscopeVisualizer>();
        },
        false);
    
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

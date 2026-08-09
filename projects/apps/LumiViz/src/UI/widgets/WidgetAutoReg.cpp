/**
 ****************************************************************************************
 * @file   WidgetAutoReg.cpp
 * @brief  Application-specific widget registrations for LumiViz
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * Diese Datei definiert die **LumiViz-spezifischen Widgets**.
 * Sie wird automatisch von WidgetRegistry::instance() beim ersten Zugriff aufgerufen.
 *
 * ## Widget-Liste
 *
 * | ID | Name | Kategorie | Multiple | Beschreibung |
 * |----|------|-----------|----------|--------------|
 * | visualizer | Visualizer | Visualizers | true | OpenGL visualization widget |
 *
 * ## allowMultiple Flag
 *
 * Widgets mit `allowMultiple = true` können mehrfach instanziiert werden,
 * ähnlich wie Dokumente in Word. Widgets mit `allowMultiple = false` sind
 * Singletons (z.B. Volume Control, Settings Panel).
 *
 * ## Trennung Framework vs. App
 *
 * - **WidgetRegistry.hpp/cpp** = Wiederverwendbares Framework
 * - **WidgetAutoReg.cpp** = LumiViz-spezifische Widgets (diese Datei)
 *
 * ## Linker-Garantie
 *
 * WidgetRegistry.cpp deklariert `extern void initWidgetDefaults(WidgetRegistry&)`.
 * Diese Referenz erzwingt, dass der Linker WidgetAutoReg.cpp einbindet,
 * auch bei statischen Libraries.
 *
 * ## Neue Widgets hinzufügen
 *
 * 1. Widget-Klasse erstellen (von WidgetBase oder QWidget ableiten)
 * 2. Header hier inkludieren
 * 3. Registrierung in initWidgetDefaults() hinzufügen
 ****************************************************************************************
 */

#include "services/WidgetRegistry.hpp"

// Widget Headers
#include "UI/widgets/VisualizerWidget.hpp"
// TODO: Zukünftige Widgets hier inkludieren:
// #include "UI/widgets/VolumeWidget.hpp"
// #include "UI/widgets/WaveformWidget.hpp"
// #include "UI/widgets/SpectrumWidget.hpp"

// =============================================================================
// initWidgetDefaults - Called by WidgetRegistry::instance() on first access
// =============================================================================

/**
 * @brief Register all default widgets for LumiViz
 * 
 * @param registry Reference to the WidgetRegistry singleton
 * 
 * This function is called automatically by WidgetRegistry::instance() on first access.
 * It registers all available widgets.
 */
void initWidgetDefaults(WidgetRegistry& registry)
{
    // =========================================================================
    // VISUALIZERS CATEGORY
    // =========================================================================
    
    // VisualizerWidget - OpenGL canvas for audio visualization
    // allowMultiple = true: Users can open multiple visualizer windows
    registry.registerWidget(
        WidgetDescriptor{
            "visualizer",                                       // id
            "Visualizer",                                       // name
            "Visualizers",                                      // category
            "OpenGL visualization widget for audio effects",    // description
            100,                                                // order
            false                                                // allowMultiple
        },
        [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QWidget> {
            return std::make_unique<VisualizerWidget>(svc, parent);
        },
        false);
    
    // =========================================================================
    // CONTROLS CATEGORY (TODO)
    // =========================================================================
    
    // TODO: Volume Widget - Audio volume control
    // allowMultiple = false: Only one volume control needed
    // registry.registerWidget(
    //     WidgetDescriptor{
    //         "volume", "Volume Control", "Controls",
    //         "Audio volume slider with mute button", 100, false
    //     },
    //     [](ServiceContainer& svc, QWidget* parent) {
    //         return std::make_unique<VolumeWidget>(svc, parent);
    //     },
    //     false);
    
    // =========================================================================
    // DISPLAY CATEGORY (TODO)
    // =========================================================================
    
    // TODO: Waveform Widget - Audio waveform display
    // allowMultiple = true: Can display different audio sources
    // registry.registerWidget(
    //     WidgetDescriptor{
    //         "waveform", "Waveform Display", "Display",
    //         "Real-time audio waveform visualization", 100, true
    //     },
    //     [](ServiceContainer& svc, QWidget* parent) {
    //         return std::make_unique<WaveformWidget>(svc, parent);
    //     },
    //     false);
    
    // TODO: Spectrum Widget - Frequency spectrum bars
    // allowMultiple = true: Can show different frequency ranges
    // registry.registerWidget(
    //     WidgetDescriptor{
    //         "spectrum", "Spectrum Analyzer", "Display",
    //         "FFT-based frequency spectrum display", 200, true
    //     },
    //     [](ServiceContainer& svc, QWidget* parent) {
    //         return std::make_unique<SpectrumWidget>(svc, parent);
    //     },
    //     false);
}

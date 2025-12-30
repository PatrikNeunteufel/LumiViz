/**
 ****************************************************************************************
 * @file   VisualizerInit.cpp
 * @brief  Explicit visualizer registration implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * This file forces the linker to include all visualizer object files by
 * referencing symbols from each visualizer.
 *
 * ## Adding New Visualizers
 *
 * When adding a new visualizer:
 * 1. Add the visualizer's REGISTER_* macro in its .cpp file
 * 2. Include the visualizer header here
 * 3. Add a reference in forceVisualizerInclusion()
 ****************************************************************************************
 */

#include "visualizers/VisualizerInit.hpp"

// =============================================================================
// Include ALL Visualizer Headers
// =============================================================================
// These includes ensure the compiler sees the visualizer classes

#include "visualizers/PulsingVisualizer.hpp"
// TODO: Add future visualizers here:
// #include "visualizers/SpectrumVisualizer.hpp"
// #include "visualizers/WaveformVisualizer.hpp"
// #include "visualizers/ParticleVisualizer.hpp"

// =============================================================================
// Force Linker Inclusion
// =============================================================================

namespace
{

/**
 * @brief Forces the linker to include visualizer object files
 *
 * By creating instances (that are immediately destroyed), we ensure
 * the linker includes the corresponding object files, which triggers
 * the static initialization of REGISTER_* macros.
 */
void forceVisualizerInclusion()
{
    // Create temporary instances to force linking
    // The compiler won't optimize these away because they have side effects
    // (constructor/destructor calls)
    
    [[maybe_unused]] volatile auto* p1 = new PulsingVisualizer();
    delete p1;
    
    // TODO: Add future visualizers here:
    // [[maybe_unused]] volatile auto* p2 = new SpectrumVisualizer();
    // delete p2;
}

// Static initializer - runs at program startup
static struct VisualizerInitializer
{
    VisualizerInitializer()
    {
        forceVisualizerInclusion();
    }
} s_visualizerInitializer;

} // namespace

// =============================================================================
// Public API
// =============================================================================

void initializeVisualizers()
{
    // The actual work is done by the static initializer above.
    // This function exists to provide an explicit call point and
    // to ensure this translation unit is linked.
    
    // Additionally, we can log the registered visualizers here
    // (but we don't include BasicLogger to keep dependencies minimal)
}

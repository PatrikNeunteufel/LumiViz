/**
 ****************************************************************************************
 * @file   VisualizerInit.hpp
 * @brief  Explicit visualizer registration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Problem: Static Initialization in Static Libraries
 *
 * Self-registration macros (REGISTER_VISUALIZER_*) rely on static initialization.
 * When building as a static library, the linker may not include object files that
 * are not directly referenced, causing the registration to never execute.
 *
 * ## Solution
 *
 * Call `initializeVisualizers()` early in application startup to force the
 * linker to include all visualizer object files.
 *
 * ```cpp
 * // In Application::init() or main():
 * initializeVisualizers();
 * ```
 ****************************************************************************************
 */

#pragma once

/**
 * @brief Force inclusion of all visualizer registrations
 *
 * Call this function once at application startup to ensure all visualizers
 * are registered with the VisualizerRegistry.
 *
 * This function is intentionally empty - its purpose is to create a reference
 * to symbols in visualizer source files, forcing the linker to include them.
 */
void initializeVisualizers();

/**
 ****************************************************************************************
 * @file   pch.h
 * @brief  Precompiled Header
 *         CMake Architecture V2 - App-Container Template
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Precompiled Header
// CMake Architecture V2 - App-Container Template
// =============================================================================
//
// This header contains frequently used, stable includes.
// It is compiled as a precompiled header to reduce build times.
//
// Note: .h extension is standard for PCH (even for C++),
// as some compilers/tools do not process .hpp correctly.
//
// =============================================================================

#pragma once

// =============================================================================
// Standard Library
// =============================================================================

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// =============================================================================
// Extend with project-specific includes (examples commented out):
// =============================================================================

// Qt (for GUI apps)
// #include <QApplication>
// #include <QMainWindow>
// #include <QString>
// #include <QWidget>

// OpenGL (for visualizer apps)
// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

// Audio (for audio apps)
// #include <bass.h>


//// Platform-specific
//#ifdef _WIN32
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif
//#ifndef NOMINMAX
//#define NOMINMAX
//#endif
//// #include <windows.h>  // Uncomment if needed
//#endif
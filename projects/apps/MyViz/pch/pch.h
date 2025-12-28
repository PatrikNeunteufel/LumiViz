/**
 ****************************************************************************************
 * @file   pch.h
 * @brief  Precompiled Header - Qt6 Tutorial
 *         Contains frequently used, stable includes for faster compilation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * ## What is a Precompiled Header (PCH)?
 *
 * A precompiled header is a technique to speed up compilation by:
 * 1. Compiling stable headers once
 * 2. Storing the compiled state
 * 3. Reusing it for all source files
 *
 * ## What to include here:
 * - Standard library headers you use frequently
 * - Third-party library headers that rarely change
 * - Qt headers (stable, widely used)
 *
 * ## What NOT to include:
 * - Your own project headers (they change often)
 * - Headers with macros that affect other headers
 * - Platform-specific headers (unless carefully guarded)
 *
 * ## File Extension
 * We use .h (not .hpp) for PCH because:
 * - Some compilers/tools expect .h for PCH
 * - It's the established convention
 ****************************************************************************************
 */

#pragma once

 // =============================================================================
 // Standard Library - Core Types
 // =============================================================================
 // These are the most commonly used standard headers.
 // Include them here to avoid repeated parsing.

#include <cstddef>      // size_t, nullptr_t, ptrdiff_t
#include <cstdint>      // int32_t, uint64_t, etc.

// =============================================================================
// Standard Library - Containers
// =============================================================================

#include <array>        // std::array - fixed-size array
#include <map>          // std::map - sorted key-value container
#include <string>       // std::string - dynamic string
#include <unordered_map>// std::unordered_map - hash-based key-value
#include <vector>       // std::vector - dynamic array

// =============================================================================
// Standard Library - Utilities
// =============================================================================

#include <algorithm>    // std::sort, std::find, etc.
#include <chrono>       // Time utilities
#include <functional>   // std::function, std::bind
#include <memory>       // std::unique_ptr, std::shared_ptr
#include <optional>     // std::optional - nullable value
#include <string_view>  // std::string_view - non-owning string reference
#include <utility>      // std::move, std::forward, std::pair

// =============================================================================
// Standard Library - I/O (use sparingly in production)
// =============================================================================

#include <iostream>     // std::cout, std::cerr (for debugging)

// =============================================================================
// Qt6 Framework - Core
// =============================================================================
// Qt headers are large and stable - perfect for PCH.
// We include the most commonly used Qt classes.

#include <QApplication> // Main application class
#include <QMainWindow>  // Main window base class
#include <QString>      // Qt's string class
#include <QWidget>      // Base class for all UI elements

// =============================================================================
// Qt6 Framework - Additional (add as needed)
// =============================================================================
// Uncomment as your project grows:

// #include <QDebug>       // qDebug(), qWarning(), etc.
// #include <QTimer>       // Timer functionality
// #include <QThread>      // Threading support
// #include <QFile>        // File I/O
// #include <QDir>         // Directory operations
// #include <QSettings>    // Application settings

// =============================================================================
// Platform-Specific
// =============================================================================
// Windows-specific headers should be included carefully.
// These defines prevent pulling in unnecessary Windows headers.

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used Windows headers
#endif
#ifndef NOMINMAX
#define NOMINMAX  // Prevent min/max macros (conflict with std::min/max)
#endif
// #include <windows.h>  // Uncomment only if needed
#endif
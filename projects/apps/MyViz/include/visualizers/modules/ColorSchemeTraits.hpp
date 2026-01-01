/**
 ****************************************************************************************
 * @file   ColorSchemeTraits.hpp
 * @brief  Type-safe enum traits for ColorSchemeType
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Provides compile-time and runtime utilities for ColorSchemeType enum:
 * - Type-safe conversion between index and enum
 * - Localized display names
 * - Iteration over all values
 *
 * Usage:
 * @code
 * // Populate ComboBox
 * for (const auto& name : ColorSchemeTraits::displayNames()) {
 *     comboBox->addItem(name);
 * }
 *
 * // Convert index to enum
 * auto scheme = ColorSchemeTraits::fromIndex(comboBox->currentIndex());
 *
 * // Convert enum to index
 * int idx = ColorSchemeTraits::toIndex(ColorSchemeType::Neon);
 * @endcode
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/ColorSchemeModule.hpp"

#include <QObject>
#include <QString>
#include <QStringList>
#include <array>
#include <optional>

namespace lumi::modules {

/**
 * @brief Type traits for ColorSchemeType enum
 */
struct ColorSchemeTraits
{
    // =========================================================================
    // Compile-time Constants
    // =========================================================================

    /// Number of color schemes (excluding Custom)
    static constexpr size_t count = 10;

    /// All scheme values in order
    static constexpr std::array<ColorSchemeType, count> values = {
        ColorSchemeType::Fire,
        ColorSchemeType::Ocean,
        ColorSchemeType::Neon,
        ColorSchemeType::Rainbow,
        ColorSchemeType::Sunset,
        ColorSchemeType::Forest,
        ColorSchemeType::Ice,
        ColorSchemeType::Lava,
        ColorSchemeType::Galaxy,
        ColorSchemeType::Monochrome
    };

    // =========================================================================
    // Runtime Utilities
    // =========================================================================

    /**
     * @brief Get display name for a scheme (translatable)
     */
    static QString displayName(ColorSchemeType scheme)
    {
        switch (scheme)
        {
        case ColorSchemeType::Fire:       return QObject::tr("Fire");
        case ColorSchemeType::Ocean:      return QObject::tr("Ocean");
        case ColorSchemeType::Neon:       return QObject::tr("Neon");
        case ColorSchemeType::Rainbow:    return QObject::tr("Rainbow");
        case ColorSchemeType::Sunset:     return QObject::tr("Sunset");
        case ColorSchemeType::Forest:     return QObject::tr("Forest");
        case ColorSchemeType::Ice:        return QObject::tr("Ice");
        case ColorSchemeType::Lava:       return QObject::tr("Lava");
        case ColorSchemeType::Galaxy:     return QObject::tr("Galaxy");
        case ColorSchemeType::Monochrome: return QObject::tr("Monochrome");
        case ColorSchemeType::Custom:     return QObject::tr("Custom");
        }
        return QObject::tr("Unknown");
    }

    /**
     * @brief Get all display names as QStringList (for ComboBox)
     */
    static QStringList displayNames()
    {
        QStringList names;
        names.reserve(static_cast<int>(count));
        for (auto scheme : values)
        {
            names.append(displayName(scheme));
        }
        return names;
    }

    /**
     * @brief Convert ComboBox index to ColorSchemeType
     * @param index Index (0-based)
     * @return ColorSchemeType or nullopt if invalid
     */
    static std::optional<ColorSchemeType> fromIndex(int index)
    {
        if (index >= 0 && static_cast<size_t>(index) < count)
        {
            return values[static_cast<size_t>(index)];
        }
        return std::nullopt;
    }

    /**
     * @brief Convert ColorSchemeType to ComboBox index
     * @param scheme The color scheme
     * @return Index or -1 if not found (Custom)
     */
    static int toIndex(ColorSchemeType scheme)
    {
        for (size_t i = 0; i < count; ++i)
        {
            if (values[i] == scheme)
            {
                return static_cast<int>(i);
            }
        }
        return -1;  // Custom or unknown
    }

    /**
     * @brief Get default scheme
     */
    static constexpr ColorSchemeType defaultScheme()
    {
        return ColorSchemeType::Neon;
    }

    /**
     * @brief Get default scheme index
     */
    static constexpr int defaultIndex()
    {
        return 2;  // Neon
    }
};

} // namespace lumi::modules

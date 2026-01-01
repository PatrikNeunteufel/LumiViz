/**
 ****************************************************************************************
 * @file   VisualizerPresetManager.hpp
 * @brief  Manages saving and loading of visualizer presets
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Each visualizer has its own preset folder:
 *   presets/
 *     PulsingVisualizer/
 *       Fire.json
 *       Ocean.json
 *       MyCustomPreset.json
 *     SpectrumVisualizer/
 *       Classic.json
 *       ...
 *
 * Preset files contain:
 *   - Header with visualizer ID, version, description
 *   - All parameter values as key-value pairs
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/IModule.hpp"

#include <QString>
#include <QJsonObject>
#include <QStringList>

#include <string>
#include <vector>
#include <map>
#include <optional>

class IVisualizer;

namespace lumi {

// =============================================================================
// Preset Data Structure
// =============================================================================

/**
 * @struct VisualizerPreset
 * @brief Complete preset data for a visualizer
 */
struct VisualizerPreset
{
    // Header
    QString name;                    ///< Preset display name
    QString visualizerId;            ///< ID of the visualizer this preset is for
    QString description;             ///< Optional description
    QString author;                  ///< Optional author name
    int version = 1;                 ///< Preset format version
    
    // Parameters
    std::map<std::string, modules::ParamValue> parameters;
    
    // Validation
    [[nodiscard]] bool isValid() const { return !name.isEmpty() && !visualizerId.isEmpty(); }
};

// =============================================================================
// VisualizerPresetManager
// =============================================================================

/**
 * @class VisualizerPresetManager
 * @brief Manages preset save/load operations for visualizers
 */
class VisualizerPresetManager
{
public:
    VisualizerPresetManager();
    ~VisualizerPresetManager() = default;
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    /**
     * @brief Set the base directory for presets
     * @param path Directory path (e.g., "presets" or absolute path)
     */
    void setPresetsDirectory(const QString& path);
    
    /**
     * @brief Get the current presets directory
     */
    [[nodiscard]] QString presetsDirectory() const { return m_presetsDir; }
    
    // =========================================================================
    // Preset Discovery
    // =========================================================================
    
    /**
     * @brief Get list of available presets for a visualizer
     * @param visualizerId ID of the visualizer
     * @return List of preset names (without extension)
     */
    [[nodiscard]] QStringList availablePresets(const QString& visualizerId) const;
    
    /**
     * @brief Check if a preset exists
     * @param visualizerId Visualizer ID
     * @param presetName Preset name
     */
    [[nodiscard]] bool presetExists(const QString& visualizerId, 
                                    const QString& presetName) const;
    
    // =========================================================================
    // Load/Save Operations
    // =========================================================================
    
    /**
     * @brief Load a preset from file
     * @param visualizerId Visualizer ID
     * @param presetName Preset name (without extension)
     * @return Loaded preset, or nullopt if failed
     */
    [[nodiscard]] std::optional<VisualizerPreset> loadPreset(
        const QString& visualizerId, 
        const QString& presetName) const;
    
    /**
     * @brief Save a preset to file
     * @param preset Preset data to save
     * @return true if saved successfully
     */
    bool savePreset(const VisualizerPreset& preset);
    
    /**
     * @brief Delete a preset file
     * @param visualizerId Visualizer ID
     * @param presetName Preset name
     * @return true if deleted successfully
     */
    bool deletePreset(const QString& visualizerId, const QString& presetName);
    
    /**
     * @brief Rename a preset
     * @param visualizerId Visualizer ID
     * @param oldName Current name
     * @param newName New name
     * @return true if renamed successfully
     */
    bool renamePreset(const QString& visualizerId, 
                      const QString& oldName, 
                      const QString& newName);
    
    // =========================================================================
    // Visualizer Integration
    // =========================================================================
    
    /**
     * @brief Create a preset from current visualizer state
     * @param visualizer The visualizer to capture
     * @param presetName Name for the preset
     * @param description Optional description
     * @return Created preset
     */
    [[nodiscard]] VisualizerPreset capturePreset(
        IVisualizer* visualizer,
        const QString& presetName,
        const QString& description = {}) const;
    
    /**
     * @brief Apply a preset to a visualizer
     * @param visualizer Target visualizer
     * @param preset Preset to apply
     * @return true if applied successfully
     */
    bool applyPreset(IVisualizer* visualizer, const VisualizerPreset& preset);
    
    // =========================================================================
    // Utility
    // =========================================================================
    
    /**
     * @brief Get the file extension for preset files
     */
    static QString presetExtension() { return QStringLiteral(".lvp"); }  // LumiViz Preset
    
    /**
     * @brief Ensure the presets directory exists for a visualizer
     */
    bool ensurePresetDirectory(const QString& visualizerId);

private:
    QString getPresetPath(const QString& visualizerId, const QString& presetName) const;
    QString getVisualizerDir(const QString& visualizerId) const;
    
    QJsonObject presetToJson(const VisualizerPreset& preset) const;
    std::optional<VisualizerPreset> jsonToPreset(const QJsonObject& json) const;
    
    QJsonValue paramValueToJson(const modules::ParamValue& value) const;
    std::optional<modules::ParamValue> jsonToParamValue(
        const QJsonValue& json, 
        modules::ParamType expectedType) const;
    
    QString m_presetsDir;
    
    static constexpr int CURRENT_FORMAT_VERSION = 1;
};

} // namespace lumi

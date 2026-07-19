/**
 ****************************************************************************************
 * @file   VisualizerPresetManager.cpp
 * @brief  Implementation of visualizer preset management
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/IVisualizer.hpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>

#include <BasicLogger.h>

#include <algorithm>
#include <vector>

namespace lumi {

// =============================================================================
// Constructor
// =============================================================================

VisualizerPresetManager::VisualizerPresetManager()
{
    // Default to app data location
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_presetsDir = appData + "/presets";
    
    // Note: Gradient presets directory is set in Application::init() 
    // before any visualizers are created
}

// =============================================================================
// Configuration
// =============================================================================

void VisualizerPresetManager::setPresetsDirectory(const QString& path)
{
    m_presetsDir = path;
    BasicLogger::logInfo("PresetManager: Directory set to " + path.toStdString());
}

// =============================================================================
// Preset Discovery
// =============================================================================

QStringList VisualizerPresetManager::availablePresets(const QString& visualizerId) const
{
    QStringList presets;
    
    QDir dir(getVisualizerDir(visualizerId));
    if (!dir.exists())
    {
        return presets;
    }
    
    QString ext = presetExtension();
    QStringList filters;
    filters << "*" + ext;
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo& file : files)
    {
        presets << file.baseName();
    }
    
    return presets;
}

bool VisualizerPresetManager::presetExists(const QString& visualizerId, 
                                           const QString& presetName) const
{
    return QFile::exists(getPresetPath(visualizerId, presetName));
}

// =============================================================================
// Load/Save Operations
// =============================================================================

std::optional<VisualizerPreset> VisualizerPresetManager::loadPreset(
    const QString& visualizerId, 
    const QString& presetName) const
{
    QString path = getPresetPath(visualizerId, presetName);
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        BasicLogger::logWarning("PresetManager: Cannot open " + path.toStdString());
        return std::nullopt;
    }
    
    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError)
    {
        BasicLogger::logError("PresetManager: JSON parse error: " + error.errorString().toStdString());
        return std::nullopt;
    }
    
    auto preset = jsonToPreset(doc.object());
    if (!preset)
    {
        BasicLogger::logError("PresetManager: Invalid preset format");
        return std::nullopt;
    }
    
    // Validate visualizer ID matches
    if (preset->visualizerId != visualizerId)
    {
        BasicLogger::logWarning("PresetManager: Preset visualizer ID mismatch");
        return std::nullopt;
    }
    
    BasicLogger::logInfo("PresetManager: Loaded preset '" + presetName.toStdString() + "'");
    return preset;
}

bool VisualizerPresetManager::savePreset(const VisualizerPreset& preset)
{
    if (!preset.isValid())
    {
        BasicLogger::logError("PresetManager: Invalid preset data");
        return false;
    }
    
    // Ensure directory exists
    if (!ensurePresetDirectory(preset.visualizerId))
    {
        return false;
    }
    
    QString path = getPresetPath(preset.visualizerId, preset.name);
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        BasicLogger::logError("PresetManager: Cannot write to " + path.toStdString());
        return false;
    }
    
    QJsonDocument doc(presetToJson(preset));
    file.write(doc.toJson(QJsonDocument::Indented));
    
    BasicLogger::logInfo("PresetManager: Saved preset '" + preset.name.toStdString() + "'");
    return true;
}

bool VisualizerPresetManager::deletePreset(const QString& visualizerId, 
                                            const QString& presetName)
{
    QString path = getPresetPath(visualizerId, presetName);
    
    if (!QFile::exists(path))
    {
        return false;
    }
    
    if (QFile::remove(path))
    {
        BasicLogger::logInfo("PresetManager: Deleted preset '" + presetName.toStdString() + "'");
        return true;
    }
    
    BasicLogger::logError("PresetManager: Failed to delete " + path.toStdString());
    return false;
}

bool VisualizerPresetManager::renamePreset(const QString& visualizerId,
                                            const QString& oldName,
                                            const QString& newName)
{
    QString oldPath = getPresetPath(visualizerId, oldName);
    QString newPath = getPresetPath(visualizerId, newName);
    
    if (!QFile::exists(oldPath))
    {
        return false;
    }
    
    if (QFile::exists(newPath))
    {
        BasicLogger::logWarning("PresetManager: Target name already exists");
        return false;
    }
    
    // Load, rename, save
    auto preset = loadPreset(visualizerId, oldName);
    if (!preset)
    {
        return false;
    }
    
    preset->name = newName;
    
    if (savePreset(*preset))
    {
        QFile::remove(oldPath);
        return true;
    }
    
    return false;
}

// =============================================================================
// Visualizer Integration
// =============================================================================

VisualizerPreset VisualizerPresetManager::capturePreset(
    IVisualizer* visualizer,
    const QString& presetName,
    const QString& description) const
{
    VisualizerPreset preset;
    preset.name = presetName;
    preset.visualizerId = visualizer->visualizerId();
    preset.description = description;
    preset.version = CURRENT_FORMAT_VERSION;
    
    // Capture all parameters
    auto params = visualizer->paramDescs();
    for (const auto& desc : params)
    {
        modules::ParamValue value;
        if (visualizer->getParam(desc.id, value))
        {
            preset.parameters[desc.id] = value;
        }
    }
    
    BasicLogger::logDebug("PresetManager: Captured " + 
                          std::to_string(preset.parameters.size()) + " parameters");
    
    return preset;
}

bool VisualizerPresetManager::applyPreset(IVisualizer* visualizer, 
                                          const VisualizerPreset& preset)
{
    if (preset.visualizerId != visualizer->visualizerId())
    {
        BasicLogger::logError("PresetManager: Visualizer ID mismatch - preset is for '" +
                              preset.visualizerId.toStdString() + "', but visualizer is '" +
                              visualizer->visualizerId().toStdString() + "'");
        return false;
    }
    
    int applied = 0;
    int failed = 0;

    // Legacy presets (old key schema) are translated through the alias map
    const bool translate = preset.formatVersion < CURRENT_FORMAT_VERSION;
    auto resolveKey = [&](const std::string& key) {
        return translate ? translateLegacyKey(preset.visualizerId, key) : key;
    };

    // FIRST: Apply preset parameters (like color_gradient.preset)
    // These load default values that may be overridden by subsequent parameters
    for (const auto& [paramId, value] : preset.parameters)
    {
        if (paramId.find(".preset") != std::string::npos)
        {
            if (visualizer->setParam(resolveKey(paramId), value))
            {
                ++applied;
                BasicLogger::logDebug("PresetManager: Applied preset param '" + paramId + "'");
            }
            else
            {
                BasicLogger::logWarning("PresetManager: Failed to set preset param '" + paramId + "'");
                ++failed;
            }
        }
    }

    // THEN: Apply all other parameters (may override preset defaults)
    for (const auto& [paramId, value] : preset.parameters)
    {
        // Skip preset parameters (already applied above)
        if (paramId.find(".preset") != std::string::npos)
        {
            continue;
        }

        if (visualizer->setParam(resolveKey(paramId), value))
        {
            ++applied;
        }
        else
        {
            BasicLogger::logWarning("PresetManager: Failed to set param '" + paramId + "'");
            ++failed;
        }
    }
    
    BasicLogger::logInfo("PresetManager: Applied preset '" + preset.name.toStdString() +
                         "' (" + std::to_string(applied) + " applied, " +
                         std::to_string(failed) + " failed)");
    
    return failed == 0;
}

// =============================================================================
// Legacy-Key-Migration (Phase 4)
// =============================================================================

namespace
{
    using AliasMap = std::map<std::string, std::string>;

    std::map<QString, AliasMap>& aliasRegistry()
    {
        static std::map<QString, AliasMap> registry;
        return registry;
    }
} // namespace

void VisualizerPresetManager::registerKeyAliases(const QString& visualizerId,
                                                 std::map<std::string, std::string> aliases)
{
    auto& entry = aliasRegistry()[visualizerId];
    for (auto& [oldKey, newKey] : aliases)
    {
        entry.insert_or_assign(oldKey, newKey);
    }
}

std::string VisualizerPresetManager::translateLegacyKey(const QString& visualizerId,
                                                        const std::string& key)
{
    const auto& registry = aliasRegistry();
    auto it = registry.find(visualizerId);
    if (it == registry.end())
    {
        return key;
    }
    auto keyIt = it->second.find(key);
    return keyIt == it->second.end() ? key : keyIt->second;
}

void VisualizerPresetManager::clearKeyAliases()
{
    aliasRegistry().clear();
}

// =============================================================================
// Utility
// =============================================================================

bool VisualizerPresetManager::ensurePresetDirectory(const QString& visualizerId)
{
    QDir dir(getVisualizerDir(visualizerId));
    if (dir.exists())
    {
        return true;
    }
    
    if (dir.mkpath("."))
    {
        BasicLogger::logInfo("PresetManager: Created directory for " + visualizerId.toStdString());
        return true;
    }
    
    BasicLogger::logError("PresetManager: Failed to create directory");
    return false;
}

QString VisualizerPresetManager::getPresetPath(const QString& visualizerId, 
                                                const QString& presetName) const
{
    return getVisualizerDir(visualizerId) + "/" + presetName + presetExtension();
}

QString VisualizerPresetManager::getVisualizerDir(const QString& visualizerId) const
{
    return m_presetsDir + "/visuals/" + visualizerId;
}

// =============================================================================
// JSON Conversion
// =============================================================================

QJsonObject VisualizerPresetManager::presetToJson(const VisualizerPreset& preset) const
{
    QJsonObject obj;
    
    // Header
    QJsonObject header;
    header["name"] = preset.name;
    header["visualizerId"] = preset.visualizerId;
    header["description"] = preset.description;
    header["author"] = preset.author;
    header["version"] = preset.version;
    header["formatVersion"] = CURRENT_FORMAT_VERSION;
    obj["header"] = header;
    
    // Parameters
    QJsonObject params;
    for (const auto& [id, value] : preset.parameters)
    {
        params[QString::fromStdString(id)] = paramValueToJson(value);
    }
    obj["parameters"] = params;
    
    return obj;
}

std::optional<VisualizerPreset> VisualizerPresetManager::jsonToPreset(
    const QJsonObject& json) const
{
    VisualizerPreset preset;
    
    // Header
    if (!json.contains("header") || !json["header"].isObject())
    {
        return std::nullopt;
    }
    
    QJsonObject header = json["header"].toObject();
    preset.name = header["name"].toString();
    preset.visualizerId = header["visualizerId"].toString();
    preset.description = header["description"].toString();
    preset.author = header["author"].toString();
    preset.version = header["version"].toInt(1);
    // Missing formatVersion = legacy file from before the field existed
    preset.formatVersion = header["formatVersion"].toInt(1);
    
    if (!preset.isValid())
    {
        return std::nullopt;
    }
    
    // Parameters
    if (json.contains("parameters") && json["parameters"].isObject())
    {
        QJsonObject params = json["parameters"].toObject();
        for (auto it = params.begin(); it != params.end(); ++it)
        {
            QString paramId = it.key();
            QJsonValue jsonValue = it.value();
            
            // Detect type from JSON value
            if (jsonValue.isBool())
            {
                preset.parameters[paramId.toStdString()] = jsonValue.toBool();
            }
            else if (jsonValue.isDouble())
            {
                // JSON doesn't distinguish int/float - always store as float
                // The setParam implementations should handle both types
                double d = jsonValue.toDouble();
                preset.parameters[paramId.toStdString()] = static_cast<float>(d);
            }
            else if (jsonValue.isString())
            {
                preset.parameters[paramId.toStdString()] = jsonValue.toString().toStdString();
            }
            else if (jsonValue.isArray())
            {
                // Could be Color4f
                QJsonArray arr = jsonValue.toArray();
                if (arr.size() == 4)
                {
                    modules::Color4f color = {
                        static_cast<float>(arr[0].toDouble()),
                        static_cast<float>(arr[1].toDouble()),
                        static_cast<float>(arr[2].toDouble()),
                        static_cast<float>(arr[3].toDouble())
                    };
                    modules::ParamValue pv;
                    pv.emplace<7>(color);  // Index 7 = Color4f
                    preset.parameters[paramId.toStdString()] = pv;
                }
            }
        }
    }
    
    return preset;
}

QJsonValue VisualizerPresetManager::paramValueToJson(const modules::ParamValue& value) const
{
    return std::visit([](auto&& arg) -> QJsonValue {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, bool>)
        {
            return arg;
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            return arg;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            return static_cast<double>(arg);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return arg;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return QString::fromStdString(arg);
        }
        else if constexpr (std::is_same_v<T, std::vector<float>>)
        {
            QJsonArray arr;
            for (float f : arg) arr.append(static_cast<double>(f));
            return arr;
        }
        else if constexpr (std::is_same_v<T, std::vector<int>>)
        {
            QJsonArray arr;
            for (int i : arg) arr.append(i);
            return arr;
        }
        else if constexpr (std::is_same_v<T, modules::Color4f>)
        {
            QJsonArray arr;
            arr.append(static_cast<double>(arg[0]));
            arr.append(static_cast<double>(arg[1]));
            arr.append(static_cast<double>(arg[2]));
            arr.append(static_cast<double>(arg[3]));
            return arr;
        }
        else
        {
            return QJsonValue();
        }
    }, value);
}

std::optional<modules::ParamValue> VisualizerPresetManager::jsonToParamValue(
    const QJsonValue& json, 
    modules::ParamType expectedType) const
{
    using namespace modules;
    
    switch (expectedType)
    {
        case ParamType::Bool:
            if (json.isBool()) return json.toBool();
            break;
            
        case ParamType::Int:
        case ParamType::Enum:
            if (json.isDouble()) return static_cast<int>(json.toDouble());
            break;
            
        case ParamType::Float:
            if (json.isDouble()) return static_cast<float>(json.toDouble());
            break;
            
        case ParamType::String:
            if (json.isString()) return json.toString().toStdString();
            break;
            
        case ParamType::Color:
            if (json.isArray())
            {
                QJsonArray arr = json.toArray();
                if (arr.size() == 4)
                {
                    Color4f color = {
                        static_cast<float>(arr[0].toDouble()),
                        static_cast<float>(arr[1].toDouble()),
                        static_cast<float>(arr[2].toDouble()),
                        static_cast<float>(arr[3].toDouble())
                    };
                    ParamValue pv;
                    pv.emplace<7>(color);
                    return pv;
                }
            }
            break;
            
        default:
            break;
    }
    
    return std::nullopt;
}

} // namespace lumi

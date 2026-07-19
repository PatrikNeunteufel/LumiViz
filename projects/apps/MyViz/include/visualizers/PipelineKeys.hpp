/**
 ****************************************************************************************
 * @file   PipelineKeys.hpp
 * @brief  Pipeline-key conventions: key prefix → stage/group (Phase 4)
 *
 * Single source for the key-prefix → PipelineStage mapping of the config
 * pipeline (Config_Pipeline_Concept.md §4.2/§4.3). Used by the migrated
 * visualizers when they translate module schemas into pipeline keys.
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/IModule.hpp"

#include <string>

namespace lumi {

/// @brief Pipeline stage from the key prefix (Konzept §4.3)
[[nodiscard]] inline modules::PipelineStage stageForKey(const std::string& key)
{
    using modules::PipelineStage;
    if (key.rfind("audio.", 0) == 0) return PipelineStage::AudioSource;
    if (key.rfind("map.", 0) == 0) return PipelineStage::Mapping;
    if (key.rfind("color.", 0) == 0) return PipelineStage::Color;
    if (key.rfind("peak.", 0) == 0 || key.rfind("particle.", 0) == 0)
        return PipelineStage::PeakParticle;
    if (key.rfind("post.", 0) == 0) return PipelineStage::Post;
    return PipelineStage::Render;
}

/// @brief Canonical group name per stage (display title comes from the
///        ConfigPanel stage table; this is the schema-side group key)
[[nodiscard]] inline const char* groupForStage(modules::PipelineStage stage)
{
    using modules::PipelineStage;
    switch (stage)
    {
        case PipelineStage::AudioSource: return "Audio";
        case PipelineStage::Mapping: return "Mapping";
        case PipelineStage::Color: return "Color";
        case PipelineStage::PeakParticle: return "Peaks";
        case PipelineStage::Post: return "Post";
        default: return "Render";
    }
}

} // namespace lumi

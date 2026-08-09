/**
 ****************************************************************************************
 * @file   IModule.hpp
 * @brief  Module Interface - Base for all LumiPulse modules
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * ## Overview
 *
 * IModule defines the interface for all LumiPulse modules. It provides:
 *   - Parameter introspection (paramDescs)
 *   - Runtime parameter access (getParam/setParam)
 *   - Lifecycle hooks (initialize, activate, deactivate, update)
 *
 * ## Parameter System
 *
 * Parameters are identified by hierarchical paths:
 * ```
 * "smooth.algorithm"    -> SmoothingModule algorithm
 * "smooth.timeMs"       -> SmoothingModule time
 * "audio.smooth.timeMs" -> Nested: AudioSourceModule → SmoothingModule → timeMs
 * ```
 *
 * ## Usage
 *
 * ```cpp
 * class MyModule : public IModule {
 * public:
 *     const char* moduleId() const override { return "myModule"; }
 *     // ... implement interface
 * };
 * ```
 *
 * @see LumiPulse_VisualSystem_Architecture.md Section 3
 ****************************************************************************************
 */

#pragma once

#include <string>
#include <vector>
#include <variant>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

namespace lumi::modules
{

// =============================================================================
// Parameter Value Types
// =============================================================================

/// RGBA color (normalized 0-1)
using Color4f = std::array<float, 4>;

/// 2D vector
using Vec2f = std::array<float, 2>;

/// 3D vector
using Vec3f = std::array<float, 3>;

/// 4D vector
using Vec4f = std::array<float, 4>;

/// Variant holding any parameter value
using ParamValue = std::variant<
    bool,
    int,
    float,
    std::string,
    Vec2f,
    Vec3f,
    Vec4f,
    Color4f
>;

// -----------------------------------------------------------------------------
// Color access helpers
//
// Vec4f and Color4f alias the same std::array type, so the variant contains
// that alternative twice and type-based access (std::holds_alternative<Color4f>,
// std::get<Color4f>) would be ill-formed. These helpers are the single place
// that knows the Color4f alternative's index — no scattered magic numbers.
// -----------------------------------------------------------------------------

/// Index of the Color4f alternative in ParamValue
inline constexpr std::size_t kParamValueColorIndex = 7;

static_assert(std::is_same_v<std::variant_alternative_t<kParamValueColorIndex, ParamValue>,
                             Color4f>,
              "kParamValueColorIndex must address the Color4f alternative");

/// @brief Does the value hold a color (the Color4f alternative)?
[[nodiscard]] inline bool holdsColor(const ParamValue& value) noexcept
{
    return value.index() == kParamValueColorIndex;
}

/// @brief Read the color alternative (only valid if holdsColor() is true)
[[nodiscard]] inline const Color4f& getColor(const ParamValue& value)
{
    return *std::get_if<kParamValueColorIndex>(&value);
}

/// @brief Construct a ParamValue holding a color
[[nodiscard]] inline ParamValue makeColorValue(const Color4f& color)
{
    return ParamValue(std::in_place_index<kParamValueColorIndex>, color);
}

// =============================================================================
// Parameter Type Enum
// =============================================================================

/**
 * @brief Parameter data type
 */
enum class ParamType
{
    Bool,       ///< Boolean toggle
    Int,        ///< Integer value
    Float,      ///< Floating point value
    String,     ///< Text string
    Enum,       ///< Enumerated choice (stored as int)
    Vec2,       ///< 2D vector
    Vec3,       ///< 3D vector
    Vec4,       ///< 4D vector
    Color       ///< RGBA color
};

// =============================================================================
// Parameter Widget Hint
// =============================================================================

/**
 * @brief Suggested UI widget for parameter
 */
enum class ParamWidget
{
    Default,        ///< Auto-select based on type
    Slider,         ///< Float/Int with range
    Spinbox,        ///< Numeric with arrows
    Checkbox,       ///< Bool toggle
    Dropdown,       ///< Enum selection
    ColorPicker,    ///< Color4f
    TextInput,      ///< Single-line string
    TextArea,       ///< Multi-line string (for scripts)
    Knob,           ///< Rotary control
    Toggle,         ///< On/Off switch
    ButtonGroup     ///< Enum as buttons
};

// =============================================================================
// Pipeline Stage (Phase 4)
// =============================================================================

/**
 * @brief The config-pipeline stage a parameter belongs to
 *
 * The ConfigPanel renders parameter groups strictly in this order — the UI
 * follows the data flow (Config_Pipeline_Concept.md §4.1/§4.2). `None` marks
 * parameters of not-yet-migrated visualizers; the panel then falls back to
 * the legacy "1. Audio"-style group-name prefix.
 */
enum class PipelineStage : std::uint8_t
{
    None         = 0,  ///< Unmigrated — legacy group-prefix ordering applies
    AudioSource  = 1,  ///< Analysis: FFT size, scale, smoothing, dB floor/ceil
    Mapping      = 2,  ///< Band/data mapping: bands, gain, sampleCount, trigger
    Color        = 3,  ///< Gradient/solid color (gradient handles)
    Render       = 4,  ///< Geometry/rendering: bars, lines, shapes, display
    PeakParticle = 5,  ///< Peak spawner physics and particles
    Post         = 6   ///< Post processing: hold/fade, phosphor, mirror, glow
};

// =============================================================================
// Parameter Descriptor
// =============================================================================

/**
 * @brief Complete description of a module parameter
 *
 * Used for:
 * - ConfigPanel UI generation
 * - Preset serialization
 * - Node→Parameter conversion
 */
struct ModuleParamDesc
{
    // -------------------------------------------------------------------------
    // Identification
    // -------------------------------------------------------------------------

    std::string id;             ///< Unique within module (e.g., "emaAlpha")
    std::string displayName;    ///< UI label (e.g., "EMA Alpha")
    std::string group;          ///< Collapsible group (e.g., "Smoothing")
    std::string subGroup;       ///< Nested group (e.g., "Advanced")
    std::string tooltip;        ///< Help text
    PipelineStage stage = PipelineStage::None;  ///< Pipeline stage (Phase 4)
    
    // -------------------------------------------------------------------------
    // Type & Value
    // -------------------------------------------------------------------------
    
    ParamType type = ParamType::Float;
    ParamValue defaultValue;
    
    // -------------------------------------------------------------------------
    // Constraints
    // -------------------------------------------------------------------------
    
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float step = 0.01f;
    std::vector<std::string> enumOptions;  ///< For Enum type
    
    // -------------------------------------------------------------------------
    // UI Hints
    // -------------------------------------------------------------------------
    
    ParamWidget widget = ParamWidget::Default;
    int order = 0;              ///< Sort order within group
    bool advanced = false;      ///< Hide in "Advanced" section
    bool hidden = false;        ///< Completely hidden from UI (for internal serialization)
    bool canBeInput = true;     ///< Can be converted to node input
    
    // -------------------------------------------------------------------------
    // Dependencies (conditional visibility)
    // -------------------------------------------------------------------------
    
    std::string dependsOn;                   ///< Other param ID
    std::vector<ParamValue> dependsValues;   ///< Any of these values makes param visible (OR logic)
    
    // -------------------------------------------------------------------------
    // Units & Formatting
    // -------------------------------------------------------------------------
    
    std::string unit;           ///< e.g., "ms", "Hz", "%", "px"
    std::string format;         ///< printf format (e.g., "%.2f")
};

// =============================================================================
// Module Interface
// =============================================================================

/**
 * @class IModule
 * @brief Base interface for all LumiPulse modules
 *
 * Modules are reusable algorithm building blocks that can be:
 * - Used standalone in BasisVisuals
 * - Wrapped as Nodes in the Node-Graph system
 * - Configured via ConfigPanel UI
 * - Serialized to/from JSON presets
 */
class IModule
{
public:
    virtual ~IModule() = default;
    
    // -------------------------------------------------------------------------
    // Identification
    // -------------------------------------------------------------------------
    
    /**
     * @brief Unique module type ID
     * @return e.g., "smoothing", "gradient", "audioSource"
     */
    [[nodiscard]] virtual const char* moduleId() const = 0;
    
    /**
     * @brief Human-readable display name
     * @return e.g., "Smoothing", "Color Gradient", "Audio Source"
     */
    [[nodiscard]] virtual const char* displayName() const = 0;
    
    /**
     * @brief Module category for organization
     * @return e.g., "Source", "Processing", "Render", "Effect", "Color"
     */
    [[nodiscard]] virtual const char* category() const = 0;
    
    /**
     * @brief Description for tooltips
     */
    [[nodiscard]] virtual const char* description() const = 0;
    
    // -------------------------------------------------------------------------
    // Parameter Introspection
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get all parameter descriptors
     *
     * Used by ConfigPanel to generate UI controls.
     * Parameters from embedded modules should be prefixed.
     *
     * @return Vector of parameter descriptors
     */
    [[nodiscard]] virtual std::vector<ModuleParamDesc> paramDescs() const = 0;
    
    /**
     * @brief Get parameter value by ID
     * @param id Parameter ID (supports dot notation for nested: "smooth.timeMs")
     * @param out Output value
     * @return true if parameter found
     */
    [[nodiscard]] virtual bool getParam(const std::string& id, 
                                        ParamValue& out) const = 0;
    
    /**
     * @brief Set parameter value by ID
     * @param id Parameter ID (supports dot notation)
     * @param value New value
     * @return true if parameter found and value valid
     */
    virtual bool setParam(const std::string& id, 
                         const ParamValue& value) = 0;
    
    /**
     * @brief Reset all parameters to defaults
     */
    virtual void resetToDefaults() = 0;
    
    // -------------------------------------------------------------------------
    // Lifecycle
    // -------------------------------------------------------------------------
    
    /**
     * @brief Called once after construction
     */
    virtual void initialize() {}
    
    /**
     * @brief Called when parent visual becomes active
     */
    virtual void activate() {}
    
    /**
     * @brief Called when parent visual becomes inactive
     */
    virtual void deactivate() {}
    
    /**
     * @brief Called each frame before render
     * @param deltaTime Time since last frame in seconds
     */
    virtual void update(float deltaTime) { (void)deltaTime; }
};

// =============================================================================
// Processing Module Interface
// =============================================================================

/**
 * @class IProcessingModule
 * @brief Module that transforms data (input → output)
 * @tparam TInput Input data type
 * @tparam TOutput Output data type
 */
template<typename TInput, typename TOutput>
class IProcessingModule : public IModule
{
public:
    /**
     * @brief Process input data and produce output
     * @param input Input data
     * @return Processed output
     */
    virtual TOutput process(const TInput& input) = 0;
};

// =============================================================================
// Render Context
// =============================================================================

/**
 * @brief Context passed to render modules
 */
struct RenderContext
{
    float deltaTime = 0.0f;         ///< Time since last frame (seconds)
    int frameNumber = 0;            ///< Frame counter
    int viewportWidth = 0;          ///< Viewport width in pixels
    int viewportHeight = 0;         ///< Viewport height in pixels
    float aspectRatio = 1.0f;       ///< Width / Height
    bool beatThisFrame = false;     ///< Beat detected this frame
    float beatIntensity = 0.0f;     ///< Beat strength [0..1]
};

// =============================================================================
// Render Module Interface
// =============================================================================

/**
 * @class IRenderModule
 * @brief Module that renders to OpenGL context
 */
class IRenderModule : public IModule
{
public:
    /**
     * @brief Render to current OpenGL context
     * @param ctx Render context
     */
    virtual void render(const RenderContext& ctx) = 0;
};

// =============================================================================
// Source Module Interface
// =============================================================================

/**
 * @class ISourceModule
 * @brief Module that provides data (audio, time, etc.)
 * @tparam TOutput Output data type
 */
template<typename TOutput>
class ISourceModule : public IModule
{
public:
    /**
     * @brief Get current output data
     * @return Output data
     */
    [[nodiscard]] virtual TOutput getData() const = 0;
};

// =============================================================================
// Helper: Parameter Builder
// =============================================================================

/**
 * @brief Fluent builder for ModuleParamDesc
 *
 * @par Example
 * @code
 * auto desc = ParamBuilder("timeMs", ParamType::Float)
 *     .displayName("Smoothing Time")
 *     .range(0.0f, 500.0f)
 *     .defaultValue(50.0f)
 *     .unit("ms")
 *     .group("Smoothing")
 *     .build();
 * @endcode
 */
class ParamBuilder
{
public:
    ParamBuilder(std::string id, ParamType type)
    {
        m_desc.id = std::move(id);
        m_desc.type = type;
    }
    
    ParamBuilder& displayName(std::string name)
    {
        m_desc.displayName = std::move(name);
        return *this;
    }
    
    ParamBuilder& group(std::string grp)
    {
        m_desc.group = std::move(grp);
        return *this;
    }
    
    ParamBuilder& subGroup(std::string sub)
    {
        m_desc.subGroup = std::move(sub);
        return *this;
    }

    /// @brief Assign the config-pipeline stage (Phase 4)
    ParamBuilder& stage(PipelineStage s)
    {
        m_desc.stage = s;
        return *this;
    }
    
    ParamBuilder& tooltip(std::string tip)
    {
        m_desc.tooltip = std::move(tip);
        return *this;
    }
    
    ParamBuilder& range(float min, float max, float stp = 0.01f)
    {
        m_desc.minValue = min;
        m_desc.maxValue = max;
        m_desc.step = stp;
        return *this;
    }
    
    ParamBuilder& defaultValue(ParamValue val)
    {
        m_desc.defaultValue = std::move(val);
        return *this;
    }
    
    ParamBuilder& enumOptions(std::vector<std::string> opts)
    {
        m_desc.enumOptions = std::move(opts);
        return *this;
    }
    
    ParamBuilder& widget(ParamWidget w)
    {
        m_desc.widget = w;
        return *this;
    }
    
    ParamBuilder& unit(std::string u)
    {
        m_desc.unit = std::move(u);
        return *this;
    }
    
    ParamBuilder& format(std::string fmt)
    {
        m_desc.format = std::move(fmt);
        return *this;
    }
    
    ParamBuilder& order(int ord)
    {
        m_desc.order = ord;
        return *this;
    }
    
    ParamBuilder& advanced(bool adv = true)
    {
        m_desc.advanced = adv;
        return *this;
    }
    
    ParamBuilder& canBeInput(bool can = true)
    {
        m_desc.canBeInput = can;
        return *this;
    }
    
    /// @brief Set single dependency value
    ParamBuilder& dependsOn(std::string paramId, ParamValue val)
    {
        m_desc.dependsOn = std::move(paramId);
        m_desc.dependsValues = {std::move(val)};
        return *this;
    }
    
    /// @brief Set multiple dependency values (OR logic - visible if ANY matches)
    ParamBuilder& dependsOn(std::string paramId, std::vector<ParamValue> vals)
    {
        m_desc.dependsOn = std::move(paramId);
        m_desc.dependsValues = std::move(vals);
        return *this;
    }
    
    [[nodiscard]] ModuleParamDesc build() const
    {
        return m_desc;
    }
    
private:
    ModuleParamDesc m_desc;
};

} // namespace lumi::modules

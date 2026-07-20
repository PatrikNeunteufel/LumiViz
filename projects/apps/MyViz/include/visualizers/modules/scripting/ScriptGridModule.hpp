/**
 ****************************************************************************************
 * @file   ScriptGridModule.hpp
 * @brief  Scriptable grid field: EEL point slot runs per grid node (AVS Movement/
 *         Dynamic Movement, MilkDrop per_vertex)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Import-Fundament-Entwurf §2.1 (Roadmap 4.2). CPU-only, no GL: execute() fills
 * a displacement field (one source coordinate per grid node) that a render path
 * later applies as a mesh over the feedback/source image.
 *
 * Node contract (superset of AVS DMove and MilkDrop per_vertex):
 * - Inputs per node: x, y (-1..1), d (distance from center), r (angle, atan2),
 *   plus frame-level w, h, time, dt, b. The host never writes `t` (script-owned).
 * - Outputs: x, y (rect mode) or d, r (polar mode, AVS default) — the flag
 *   rectCoords() picks which pair is read back; alpha (0..1) only when the
 *   point source mentions it (else 1).
 * - Runtime errors disable the slot (LuaScriptEngine behavior) — the field
 *   falls back to the identity mapping.
 *
 * Slots are EEL via ScriptSlotHost; a shared ScriptContext connects the module
 * to the other script bearers of the same preset (reg/q/gmegabuf).
 * Threading: one thread at a time (render-mutex contract, §12).
 ****************************************************************************************
 */

#pragma once

#include "scripting/ScriptContext.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lumi::scripting { class ScriptSlotHost; }

namespace lumi::modules {

/// One grid node result: target source coordinate (-1..1) + alpha
struct GridNode
{
    float u = 0.0f;
    float v = 0.0f;
    float alpha = 1.0f;
};

/**
 * @class ScriptGridModule
 * @brief EEL script runs per grid node and yields a displacement field
 */
class ScriptGridModule
{
public:
    static constexpr int kMinRes = 2;
    static constexpr int kMaxResX = 96;
    static constexpr int kMaxResY = 72;
    static constexpr int kDefaultResX = 32;  ///< MilkDrop default mesh
    static constexpr int kDefaultResY = 24;

    explicit ScriptGridModule(std::shared_ptr<scripting::ScriptContext> context = {});
    ~ScriptGridModule();  // out-of-line: unique_ptr<ScriptSlotHost> member

    // =========================================================================
    // Sources (EEL) — changes take effect at the next execute()
    // =========================================================================

    void setInitCode(const std::string& code);
    void setBeatCode(const std::string& code);
    void setFrameCode(const std::string& code);
    void setPointCode(const std::string& code);

    // =========================================================================
    // Configuration
    // =========================================================================

    /// @brief Grid resolution (clamped to [kMinRes, kMaxRes*]); resets the field
    void setGridSize(int xres, int yres);
    [[nodiscard]] int xres() const { return m_xres; }
    [[nodiscard]] int yres() const { return m_yres; }

    /// @brief true: scripts read/write x,y — false (AVS default): d,r
    void setRectCoords(bool rect) { m_rectCoords = rect; }
    [[nodiscard]] bool rectCoords() const { return m_rectCoords; }

    // =========================================================================
    // Execution
    // =========================================================================

    /**
     * @brief Run Beat (on beat), Frame, then the Point slot per grid node
     * @param width/height Viewport size (script inputs w/h)
     * @param isBeat       Beat flag for this frame
     * @param deltaTime    Seconds since last frame
     */
    void execute(float width, float height, bool isBeat, float deltaTime);

    /// @brief Field in row-major order (yres rows x xres columns)
    [[nodiscard]] const std::vector<GridNode>& field() const { return m_field; }

    [[nodiscard]] const GridNode& node(int gx, int gy) const
    {
        return m_field[static_cast<std::size_t>(gy) * static_cast<std::size_t>(m_xres) +
                       static_cast<std::size_t>(gx)];
    }

    [[nodiscard]] const std::string& lastScriptError() const { return m_lastScriptError; }

    /// @brief Env access for tests/diagnosis
    [[nodiscard]] double getVariable(const std::string& name) const;
    void setVariable(const std::string& name, double value);

    /// @brief Reset scripts/time/field (fresh sandbox at next execute())
    void resetState();

private:
    void initializeScripts();
    void fillIdentity();

    std::string m_initCode;
    std::string m_beatCode;
    std::string m_frameCode;
    std::string m_pointCode;

    int m_xres = kDefaultResX;
    int m_yres = kDefaultResY;
    bool m_rectCoords = false;  ///< AVS default: polar (d, r)
    bool m_scriptSetsAlpha = false;

    bool m_compiled = false;
    float m_totalTime = 0.0f;

    std::shared_ptr<scripting::ScriptContext> m_context;
    std::unique_ptr<scripting::ScriptSlotHost> m_script;
    std::string m_lastScriptError;
    std::vector<GridNode> m_field;
};

} // namespace lumi::modules

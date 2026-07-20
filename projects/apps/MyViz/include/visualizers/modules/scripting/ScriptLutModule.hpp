/**
 ****************************************************************************************
 * @file   ScriptLutModule.hpp
 * @brief  Scriptable per-entry color LUT (AVS Color Modifier model): the Level
 *         slot runs once per 256 entries and yields three channel tables
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Import-Fundament-Entwurf §2.2 (Roadmap 4.2). CPU-only, no GL: execute()
 * maintains a 256-entry RGB lookup table that a color post pass later uploads
 * as a 1D texture.
 *
 * Level contract (ref r_dcolormod.cpp): per entry i the inputs red = green =
 * blue = i/255 are set, the script writes new red/green/blue (clamped 0..1).
 * The Level slot maps to ScriptSlotHost's Point slot (chunk name "*.point").
 * recompute=false (AVS default "recompute every frame" is TRUE there — here the
 * flag is explicit): table is built once after compile; recompute=true rebuilds
 * it every execute() after the Frame/Beat runs.
 *
 * Slots are EEL via ScriptSlotHost; shared ScriptContext connects the module
 * to the preset's other script bearers. Threading: render-mutex contract §12.
 ****************************************************************************************
 */

#pragma once

#include "scripting/ScriptContext.hpp"

#include <array>
#include <memory>
#include <string>

namespace lumi::scripting { class ScriptSlotHost; }

namespace lumi::modules {

/**
 * @class ScriptLutModule
 * @brief EEL script builds a 256-entry RGB LUT (Color Modifier model)
 */
class ScriptLutModule
{
public:
    static constexpr int kEntries = 256;

    explicit ScriptLutModule(std::shared_ptr<scripting::ScriptContext> context = {});
    ~ScriptLutModule();  // out-of-line: unique_ptr<ScriptSlotHost> member

    // =========================================================================
    // Sources (EEL) — changes take effect at the next execute()
    // =========================================================================

    void setInitCode(const std::string& code);
    void setBeatCode(const std::string& code);
    void setFrameCode(const std::string& code);
    void setLevelCode(const std::string& code);  ///< runs per LUT entry

    // =========================================================================
    // Configuration
    // =========================================================================

    /// @brief true: rebuild the LUT every execute() (scripted animation)
    void setRecompute(bool recompute) { m_recompute = recompute; }
    [[nodiscard]] bool recompute() const { return m_recompute; }

    // =========================================================================
    // Execution
    // =========================================================================

    /// @brief Run Beat (on beat) and Frame, rebuild the LUT when due
    void execute(bool isBeat, float deltaTime);

    /// @brief LUT value (0..1); channel 0=R 1=G 2=B, index clamped to 0..255
    [[nodiscard]] float lut(int channel, int index) const;

    [[nodiscard]] const std::array<std::array<float, kEntries>, 3>& tables() const
    {
        return m_tables;
    }

    [[nodiscard]] const std::string& lastScriptError() const { return m_lastScriptError; }

    /// @brief Env access for tests/diagnosis
    [[nodiscard]] double getVariable(const std::string& name) const;
    void setVariable(const std::string& name, double value);

    /// @brief Reset scripts/time/LUT (fresh sandbox at next execute())
    void resetState();

private:
    void initializeScripts();
    void rebuildLut();
    void fillIdentity();

    std::string m_initCode;
    std::string m_beatCode;
    std::string m_frameCode;
    std::string m_levelCode;

    bool m_recompute = false;
    bool m_compiled = false;
    bool m_lutBuilt = false;
    float m_totalTime = 0.0f;

    std::shared_ptr<scripting::ScriptContext> m_context;
    std::unique_ptr<scripting::ScriptSlotHost> m_script;
    std::string m_lastScriptError;
    std::array<std::array<float, kEntries>, 3> m_tables{};
};

} // namespace lumi::modules

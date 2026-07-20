/**
 ****************************************************************************************
 * @file   ScriptSlotHost.hpp
 * @brief  The recurring "4 EEL slots -> transpile -> compile -> run with fallback"
 *         pattern as a shared building block (extracted from SuperscopeModule)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * One host = one script-bearing module instance (Superscope, ScriptGridModule,
 * ScriptLutModule). Sources are EEL (dialect per constructor, default Avs) and
 * are transpiled via EelTranspiler at compile time — a transpile error means
 * "slot is empty" (AVS behavior) plus a recorded first error.
 *
 * Shared-variable sync (Import-Fundament-Entwurf §1): at compileAll() the
 * generated Lua is scanned for reg00..reg99 and q1..q64 identifiers; run()
 * pulls those values from the ScriptContext into the environment before the
 * call and pushes them back afterwards. The hot path stays lean — slots that
 * mention no shared variables sync nothing.
 *
 * Threading: same contract as LuaScriptEngine — one thread at a time
 * (render-mutex protected via the owning module, Visualizer_Architecture §12).
 ****************************************************************************************
 */

#pragma once

#include "scripting/LuaScriptEngine.hpp"
#include "scripting/ScriptContext.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::scripting {

/**
 * @class ScriptSlotHost
 * @brief EEL slot quartet on a LuaScriptEngine with context-synced reg/q vars
 */
class ScriptSlotHost
{
public:
    using Slot = LuaScriptEngine::Slot;

    /// EEL source dialect (mirror of lumi::eel::Dialect — kept here so this
    /// header does not depend on the transpiler headers)
    enum class Dialect
    {
        Avs,
        Milkdrop
    };

    /**
     * @param chunkPrefix Chunk-name prefix for error messages (e.g. "superscope"
     *                    → "superscope.point")
     * @param context     Shared preset context; nullptr → private context
     * @param dialect     EEL dialect of all four slot sources
     */
    explicit ScriptSlotHost(std::string chunkPrefix,
                            std::shared_ptr<ScriptContext> context = {},
                            Dialect dialect = Dialect::Avs);

    // =========================================================================
    // Sources (EEL)
    // =========================================================================

    /// @brief Set a slot's EEL source (takes effect at the next compileAll())
    void setSource(Slot slot, std::string eelSource);

    [[nodiscard]] const std::string& source(Slot slot) const
    {
        return m_sources[static_cast<std::size_t>(slot)];
    }

    /// @brief Case-insensitive whole-word search in a slot's EEL source
    ///        (e.g. does the point code mention "red"?)
    [[nodiscard]] bool sourceMentions(Slot slot, std::string_view word) const;

    // =========================================================================
    // Compile / Run
    // =========================================================================

    /**
     * @brief Transpile + compile all four slots (clears slots whose source is
     *        empty or fails). Records the FIRST error; later slots still compile.
     * @return true when no slot errored
     */
    bool compileAll();

    /**
     * @brief Run a slot with reg/q context sync around the call
     * @return false if the slot is empty or errored (error recorded; the slot
     *         disables itself on runtime errors — LuaScriptEngine behavior)
     */
    bool run(Slot slot);

    [[nodiscard]] bool has(Slot slot) const { return m_engine.has(slot); }

    // =========================================================================
    // Access
    // =========================================================================

    [[nodiscard]] LuaScriptEngine& engine() { return m_engine; }
    [[nodiscard]] const LuaScriptEngine& engine() const { return m_engine; }
    [[nodiscard]] ScriptContext& context() { return *m_engine.context(); }

    /// @brief First transpile/compile/runtime error since the last clearError()
    [[nodiscard]] const std::string& lastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }

private:
    /// One shared variable mentioned by a slot's generated Lua
    struct SharedVar
    {
        enum class Kind
        {
            Reg,  ///< reg00..reg99
            Q     ///< q1..q64
        };
        std::string name;  ///< environment name ("reg07", "q12")
        Kind kind;
        int index;  ///< reg: 0-based; q: 1-based (script numbering)
    };

    /// Scan generated Lua source for reg/q identifiers (whole words)
    static std::vector<SharedVar> scanSharedVars(const std::string& luaSource);

    void recordError(const std::string& message);

    std::string m_chunkPrefix;
    Dialect m_dialect;
    LuaScriptEngine m_engine;
    std::array<std::string, LuaScriptEngine::kSlotCount> m_sources;
    std::array<std::vector<SharedVar>, LuaScriptEngine::kSlotCount> m_sharedVars;
    std::string m_lastError;
};

} // namespace lumi::scripting

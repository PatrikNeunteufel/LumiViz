/**
 ****************************************************************************************
 * @file   LuaScriptEngine.hpp
 * @brief  Sandboxed Lua 5.4 engine for visualizer scripts (Superscope first)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * One engine = one sandboxed script environment for one preset/visualizer instance.
 * Holds a private lua_State plus a whitelisted environment table (_ENV) shared by
 * up to four script slots (Init/Beat/Frame/Point — the AVS execution model).
 *
 * Sandbox contract (Import-Analyse §7.5):
 * - Chunks load in text mode only ("t"), with a custom _ENV — scripts never see _G.
 * - Environment whitelist: math subset (unqualified: sin, cos, ...), constants
 *   (pi, pi2), rand (own deterministic PRNG — no math.random), the `eel` prelude
 *   (EEL-faithful semantics for the transpiler), and `app` (small app-global
 *   atomic register set, decision §10.3).
 * - Unknown variable reads yield 0.0 (EEL semantics), enforced via __index.
 * - megabuf is engine-local; gmegabuf lives in the ScriptContext — engines of
 *   the same preset share it (decision §10.3, Import-Fundament-Entwurf §1).
 *   reg00..reg99/q1..q64 are plain environment variables; the ScriptSlotHost
 *   syncs them with the shared ScriptContext at slot boundaries.
 *
 * Threading: NOT thread-safe. Use from one thread at a time — the owning module
 * is protected by the visualizer render-mutex contract (Visualizer_Architecture §12).
 ****************************************************************************************
 */

#pragma once

#include "scripting/ScriptContext.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>

struct lua_State;

namespace lumi::scripting {

/**
 * @class LuaScriptEngine
 * @brief Sandboxed Lua 5.4 state with the AVS-style 4-slot execution model
 */
class LuaScriptEngine
{
public:
    /// Script slots in the AVS execution model
    enum class Slot : int
    {
        Init = 0,   ///< Once after (re)compile or reset
        Beat,       ///< On detected beat
        Frame,      ///< Once per frame
        Point       ///< Once per point (hot path)
    };
    static constexpr int kSlotCount = 4;

    /// Capacity clamp for megabuf/gmegabuf indices (original: 8M doubles max)
    static constexpr std::int64_t kBufCapacity = 8'388'608;

    /// Number of app-global atomic register slots (decision Import-Analyse §10.3)
    static constexpr int kAppGlobalSlots = 32;

    /**
     * @brief Create an engine, optionally on a shared preset context
     * @param context Shared state (gmegabuf; reg/q via ScriptSlotHost-Sync).
     *                nullptr → private context (isolated, previous behavior).
     */
    explicit LuaScriptEngine(std::shared_ptr<ScriptContext> context = {});
    ~LuaScriptEngine();

    LuaScriptEngine(const LuaScriptEngine&) = delete;
    LuaScriptEngine& operator=(const LuaScriptEngine&) = delete;

    /// @brief The (shared or private) preset context of this engine
    [[nodiscard]] const std::shared_ptr<ScriptContext>& context() const
    {
        return m_context;
    }

    // =========================================================================
    // Compilation
    // =========================================================================

    /**
     * @brief Compile Lua source into a slot (replaces previous content)
     * @param slot      Target slot
     * @param source    Lua source; empty/whitespace clears the slot
     * @param chunkName Name shown in error messages (e.g. "superscope.point")
     * @return true on success (or clear); false → slot disabled, lastError() set
     */
    bool compile(Slot slot, const std::string& source, const char* chunkName);

    /// @brief Remove a slot's compiled chunk
    void clear(Slot slot);

    /// @brief True if the slot holds a runnable chunk
    [[nodiscard]] bool has(Slot slot) const;

    // =========================================================================
    // Execution
    // =========================================================================

    /**
     * @brief Run a slot. On runtime error the slot is disabled (no per-frame
     *        error spam) and lastError() is set.
     * @return false if the slot is empty or errored
     */
    bool run(Slot slot);

    // =========================================================================
    // Environment variables (numbers only — the script data model)
    // =========================================================================

    void setNumber(const char* name, double value);
    [[nodiscard]] double number(const char* name) const;  ///< 0.0 if unset/non-number

    // =========================================================================
    // Host-Zugriff auf den engine-lokalen megabuf (S48, Terrain-Grid): der
    // Host spiegelt Datenfelder (z. B. Hoehen-Grid) vor dem Slot-Lauf hinein
    // und liest sie danach zurueck — dieselbe Ablage, die megabuf(idx) im
    // Skript sieht. Unbesetzte Indizes lesen 0.0 (EEL-Semantik).
    // =========================================================================

    [[nodiscard]] double megabufValue(std::int64_t index) const
    {
        const auto it = m_megabuf.find(index);
        return it != m_megabuf.end() ? it->second : 0.0;
    }
    void setMegabufValue(std::int64_t index, double value)
    {
        if (index >= 0 && index < kBufCapacity) m_megabuf[index] = value;
    }

    // =========================================================================
    // Audio analysis (AVS-faithful getspec/getosc/gettime backing data)
    // =========================================================================

    /// @brief Feed the current frame's visualisation data (AVS layout, 576*4 bytes:
    ///        spectrum L/R then waveform L/R). getspec/getosc read from this.
    void setVisData(const unsigned char* data576x4);
    /// @brief Set the clock gettime() subtracts from (seconds since start).
    void setScriptTime(double seconds) { m_scriptTime = seconds; }

    /// @brief Darf der Host die Bequemlichkeits-Variable `time` je Frame
    ///        setzen? In AVS-EEL ist `time` ein gewoehnlicher USER-Name —
    ///        ein Quartett, das `time` selbst zuweist (el-vis_hypno07:
    ///        `time=2.0` als Konstante fuer die Laufmittel-Laenge), wuerde vom
    ///        Host-Inject jeden Frame ueberschrieben (Befund S59).
    ///        ScriptSlotHost::compileAll() setzt das Flag aus den Quellen.
    void setTimeInjectable(bool on) { m_timeInjectable = on; }
    [[nodiscard]] bool timeInjectable() const { return m_timeInjectable; }

    // =========================================================================
    // Diagnostics / tests
    // =========================================================================

    /// @brief Evaluate a Lua expression in the sandbox and return its number value
    bool evalNumber(const std::string& expr, double& out);

    [[nodiscard]] const std::string& lastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }

    /// @brief Seed the deterministic PRNG behind rand()/eel.rand()
    /// @brief Eigenen Zufallsgenerator setzen — schaltet den Strom des
    ///        geteilten ScriptContext ab (Tests/Nicht-AVS-Nutzer, S49)
    void seedRandom(std::uint64_t seed)
    {
        m_rng.seed(seed);
        m_ownRandom = true;
    }

private:
    void buildSandbox();
    bool pushEnv() const;  ///< pushes env table onto the Lua stack

    // C closures bound into the sandbox (upvalue = engine pointer)
    static int lRand(lua_State* L);
    static int lMbRead(lua_State* L);
    static int lMbWrite(lua_State* L);
    static int lGmbRead(lua_State* L);
    static int lGmbWrite(lua_State* L);
    static int lAppGet(lua_State* L);
    static int lAppSet(lua_State* L);
    static int lGetSpec(lua_State* L);
    static int lGetOsc(lua_State* L);
    static int lGetTime(lua_State* L);

    lua_State* m_state = nullptr;
    int m_envRef = -2;  // LUA_NOREF
    std::array<int, kSlotCount> m_slotRefs;
    std::string m_lastError;
    // Basis-Seed 0x4141f00d (MilkDrop); der Ctor mischt je Instanz einen
    // Nonce dazu (S14: Engines duerfen nicht dieselbe rand()-Folge ziehen).
    std::mt19937_64 m_rng{0x4141f00dULL};
    bool m_ownRandom = false;  ///< true: lokaler Generator statt Preset-Strom

    // AVS-layout visualisation data for getspec/getosc (spectrum L/R + waveform
    // L/R, 576 bytes each) and the gettime() clock. Zero until the host feeds it.
    std::array<unsigned char, 576 * 4> m_visdata{};
    double m_scriptTime = 0.0;
    bool m_timeInjectable = true;  ///< false: Skript besitzt `time` selbst (S59)

    // Engine-local script buffer (AVS: megabuf is per effect)
    std::unordered_map<std::int64_t, double> m_megabuf;
    // Preset context: gmegabuf home; shared when engines get the same context
    std::shared_ptr<ScriptContext> m_context;
};

} // namespace lumi::scripting

/**
 ****************************************************************************************
 * @file   LuaScriptEngine.cpp
 * @brief  Implementation of the sandboxed Lua 5.4 script engine
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "scripting/LuaScriptEngine.hpp"

#include <lua.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>

namespace lumi::scripting {

namespace {

constexpr double kEpsilon = 0.00001;  // EEL truthiness/equality epsilon (g_closefact)

/// App-global atomic register set (decision Import-Analyse §10.3) — process-wide,
/// each slot individually atomic, no cross-slot transaction guarantees.
std::array<std::atomic<double>, LuaScriptEngine::kAppGlobalSlots> s_appGlobals{};

bool isBlank(const std::string& s)
{
    for (char c : s)
    {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') return false;
    }
    return true;
}

// =============================================================================
// Stateless sandbox functions (EEL-faithful semantics, Import-Analyse §7.2)
// =============================================================================

bool truthy(double x) { return std::fabs(x) > kEpsilon; }

int lTruthy(lua_State* L)
{
    lua_pushboolean(L, truthy(luaL_checknumber(L, 1)));
    return 1;
}

int lEqual(lua_State* L)
{
    lua_pushnumber(L, std::fabs(luaL_checknumber(L, 1) - luaL_checknumber(L, 2)) < kEpsilon ? 1.0 : 0.0);
    return 1;
}

int lAbove(lua_State* L)
{
    lua_pushnumber(L, luaL_checknumber(L, 1) > luaL_checknumber(L, 2) ? 1.0 : 0.0);
    return 1;
}

int lBelow(lua_State* L)
{
    lua_pushnumber(L, luaL_checknumber(L, 1) < luaL_checknumber(L, 2) ? 1.0 : 0.0);
    return 1;
}

int lBand(lua_State* L)
{
    lua_pushnumber(L, truthy(luaL_checknumber(L, 1)) && truthy(luaL_checknumber(L, 2)) ? 1.0 : 0.0);
    return 1;
}

int lBor(lua_State* L)
{
    lua_pushnumber(L, truthy(luaL_checknumber(L, 1)) || truthy(luaL_checknumber(L, 2)) ? 1.0 : 0.0);
    return 1;
}

int lBnot(lua_State* L)
{
    lua_pushnumber(L, truthy(luaL_checknumber(L, 1)) ? 0.0 : 1.0);
    return 1;
}

int lBitAnd(lua_State* L)
{
    const auto a = static_cast<std::int64_t>(std::llround(luaL_checknumber(L, 1)));
    const auto b = static_cast<std::int64_t>(std::llround(luaL_checknumber(L, 2)));
    lua_pushnumber(L, static_cast<double>(a & b));
    return 1;
}

int lBitOr(lua_State* L)
{
    const auto a = static_cast<std::int64_t>(std::llround(luaL_checknumber(L, 1)));
    const auto b = static_cast<std::int64_t>(std::llround(luaL_checknumber(L, 2)));
    lua_pushnumber(L, static_cast<double>(a | b));
    return 1;
}

// EEL % (nseel_asm_mod, S48 zeilengenau): Divisor = FPU-round(max(b, 1))
// (der |b-1|+b+1-Trick klemmt auf >= 1 — div0-sicher), Zaehler FPU-gerundet,
// Rest per UNSIGNED 32-bit-Division — negative Zaehler wickeln ueber 2^32
// (Neon-Coaster-Befund: mf = getosc(..)*200%4 mit negativem getosc).
int lMod(lua_State* L)
{
    const double a = luaL_checknumber(L, 1);
    const double b = luaL_checknumber(L, 2);
    const auto ia = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(std::llrint(a)));
    const auto id = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(std::llrint(std::max(b, 1.0))));
    lua_pushnumber(L, id == 0 ? 0.0 : static_cast<double>(ia % id));
    return 1;
}

int lToInt(lua_State* L)
{
    lua_pushnumber(L, static_cast<double>(std::llround(luaL_checknumber(L, 1))));
    return 1;
}

// EEL sqrt: sqrt(|x|) — never NaN for negative input
int lEelSqrt(lua_State* L)
{
    lua_pushnumber(L, std::sqrt(std::fabs(luaL_checknumber(L, 1))));
    return 1;
}

int lInvSqrt(lua_State* L)
{
    lua_pushnumber(L, 1.0 / std::sqrt(luaL_checknumber(L, 1)));
    return 1;
}

int lSigmoid(lua_State* L)
{
    const double x = luaL_checknumber(L, 1);
    const double c = luaL_checknumber(L, 2);
    const double z = 1.0 + std::exp(-x * c);
    lua_pushnumber(L, std::fabs(z) > kEpsilon ? 1.0 / z : 0.0);
    return 1;
}

int lSign(lua_State* L)
{
    const double x = luaL_checknumber(L, 1);
    lua_pushnumber(L, x > 0.0 ? 1.0 : (x < 0.0 ? -1.0 : 0.0));
    return 1;
}

// Metatable __index: unknown variable reads yield 0.0 (EEL semantics)
int lEnvIndex(lua_State* L)
{
    lua_pushnumber(L, 0.0);
    return 1;
}

// megabuf index rule: floor(i + 0.0001), clamped to capacity; -1 = invalid
std::int64_t bufIndex(double i)
{
    const auto idx = static_cast<std::int64_t>(std::floor(i + 0.0001));
    return (idx >= 0 && idx < LuaScriptEngine::kBufCapacity) ? idx : -1;
}

} // namespace

// =============================================================================
// C closures bound to the engine (upvalue 1 = engine pointer)
// =============================================================================

int LuaScriptEngine::lRand(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    // EEL rand(x) = rand()%max(x,1) (nseel-cfunc.c:54) — der Strom liegt im
    // geteilten ScriptContext, damit alle Skripte eines Presets DENSELBEN
    // MSVC-Strom teilen wie im Original (S49). seedRandom() schaltet auf den
    // lokalen Generator um (Tests/Nicht-AVS-Nutzer).
    auto range = static_cast<std::int64_t>(luaL_optnumber(L, 1, 0.0));  // truncation
    if (range < 1) range = 1;
    const std::int64_t value =
        self->m_ownRandom || self->m_context == nullptr
            ? static_cast<std::int64_t>(self->m_rng() % static_cast<std::uint64_t>(range))
            : self->m_context->nextRand() % range;
    lua_pushnumber(L, static_cast<double>(value));
    return 1;
}

int LuaScriptEngine::lMbRead(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const std::int64_t idx = bufIndex(luaL_checknumber(L, 1));
    if (idx < 0)
    {
        lua_pushnumber(L, 0.0);
        return 1;
    }
    const auto it = self->m_megabuf.find(idx);
    lua_pushnumber(L, it != self->m_megabuf.end() ? it->second : 0.0);
    return 1;
}

int LuaScriptEngine::lMbWrite(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const std::int64_t idx = bufIndex(luaL_checknumber(L, 1));
    const double value = luaL_checknumber(L, 2);
    if (idx >= 0) self->m_megabuf[idx] = value;
    lua_pushnumber(L, value);  // assign() semantics: returns the written value
    return 1;
}

int LuaScriptEngine::lGmbRead(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const std::int64_t idx = bufIndex(luaL_checknumber(L, 1));
    lua_pushnumber(L, idx >= 0 ? self->m_context->gmbRead(idx) : 0.0);
    return 1;
}

int LuaScriptEngine::lGmbWrite(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const std::int64_t idx = bufIndex(luaL_checknumber(L, 1));
    const double value = luaL_checknumber(L, 2);
    if (idx >= 0) self->m_context->gmbWrite(idx, value);
    lua_pushnumber(L, value);
    return 1;
}

namespace {
// Faithful port of AVS getvis (avs_eelif.cpp): average the |data| over a band
// window (centred on `bc`, width `bw`), per channel (0 = both, 1 = left, 2 =
// right). `xorv` = 0 for spectrum, 128 for the signed waveform.
double getvis(const unsigned char* visdata, int bc, int bw, int ch, int xorv)
{
    if (ch && ch != 1 && ch != 2) return 0.0;
    if (bw < 1) bw = 1;
    bc -= bw / 2;
    if (bc < 0) { bw += bc; bc = 0; }
    if (bc > 575) bc = 575;
    if (bc + bw > 576) bw = 576 - bc;
    if (bw < 1) return 0.0;
    long accum = 0;
    if (!ch)
    {
        for (int x = 0; x < bw; ++x)
        {
            accum += (visdata[bc] ^ xorv) - xorv;
            accum += (visdata[bc + 576] ^ xorv) - xorv;
            ++bc;
        }
        return static_cast<double>(accum) / (static_cast<double>(bw) * 255.0);
    }
    const unsigned char* vd = (ch == 2) ? visdata + 576 : visdata;
    for (int x = 0; x < bw; ++x) accum += (vd[bc++] ^ xorv) - xorv;
    return static_cast<double>(accum) / (static_cast<double>(bw) * 127.5);
}
}  // namespace

int LuaScriptEngine::lGetSpec(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const int bc = static_cast<int>(luaL_checknumber(L, 1) * 576.0);
    const int bw = static_cast<int>(luaL_optnumber(L, 2, 0.0) * 576.0);
    const int ch = static_cast<int>(luaL_optnumber(L, 3, 0.0) + 0.5);
    lua_pushnumber(L, getvis(self->m_visdata.data(), bc, bw, ch, 0) * 0.5);
    return 1;
}

int LuaScriptEngine::lGetOsc(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const int bc = static_cast<int>(luaL_checknumber(L, 1) * 576.0);
    const int bw = static_cast<int>(luaL_optnumber(L, 2, 0.0) * 576.0);
    const int ch = static_cast<int>(luaL_optnumber(L, 3, 0.0) + 0.5);
    lua_pushnumber(L, getvis(self->m_visdata.data() + 576 * 2, bc, bw, ch, 128));
    return 1;
}

int LuaScriptEngine::lGetTime(lua_State* L)
{
    auto* self = static_cast<LuaScriptEngine*>(lua_touserdata(L, lua_upvalueindex(1)));
    const double sc = luaL_optnumber(L, 1, 0.0);
    lua_pushnumber(L, self->m_scriptTime - sc);
    return 1;
}

void LuaScriptEngine::setVisData(const unsigned char* data)
{
    if (data != nullptr) std::copy(data, data + m_visdata.size(), m_visdata.begin());
}

int LuaScriptEngine::lAppGet(lua_State* L)
{
    const auto slot = static_cast<std::int64_t>(luaL_checknumber(L, 1));
    const bool valid = slot >= 0 && slot < kAppGlobalSlots;
    lua_pushnumber(L, valid ? s_appGlobals[static_cast<std::size_t>(slot)].load(std::memory_order_relaxed) : 0.0);
    return 1;
}

int LuaScriptEngine::lAppSet(lua_State* L)
{
    const auto slot = static_cast<std::int64_t>(luaL_checknumber(L, 1));
    const double value = luaL_checknumber(L, 2);
    if (slot >= 0 && slot < kAppGlobalSlots)
    {
        s_appGlobals[static_cast<std::size_t>(slot)].store(value, std::memory_order_relaxed);
    }
    lua_pushnumber(L, value);
    return 1;
}

// =============================================================================
// Construction / Destruction
// =============================================================================

LuaScriptEngine::LuaScriptEngine(std::shared_ptr<ScriptContext> context)
    : m_context(context != nullptr ? std::move(context)
                                   : std::make_shared<ScriptContext>())
{
    // S14: AVS' rand() ist ein GLOBALER Strom — verschiedene Effekte ziehen
    // daraus VERSCHIEDENE Werte. Ein identischer Default-Seed je Engine liess
    // alle Instanzen dieselbe "Zufalls"-Folge ziehen (Beleg Session 45: Egos
    // Doppel-Scope randomisiert af/bf identisch -> Subtract loescht exakt
    // schwarz). Darum je Instanz ein eigener, aber weiterhin deterministischer
    // Seed (Erzeugungsreihenfolge); explizites seedRandom() gilt unveraendert.
    static std::atomic<std::uint64_t> s_instanceNonce{0};
    m_rng.seed(0x4141f00dULL ^
               (s_instanceNonce.fetch_add(1, std::memory_order_relaxed) *
                0x9E3779B97F4A7C15ULL));
    m_slotRefs.fill(LUA_NOREF);
    m_state = luaL_newstate();
    if (m_state == nullptr)
    {
        m_lastError = "luaL_newstate failed";
        return;
    }
    // Host-side stdlib (scripts never see it — they run with a custom _ENV)
    luaL_openlibs(m_state);
    lua_gc(m_state, LUA_GCGEN, 0, 0);
    buildSandbox();
}

LuaScriptEngine::~LuaScriptEngine()
{
    if (m_state != nullptr)
    {
        lua_close(m_state);
    }
}

// =============================================================================
// Sandbox
// =============================================================================

void LuaScriptEngine::buildSandbox()
{
    lua_State* L = m_state;

    lua_newtable(L);  // env

    // --- math subset, unqualified (whitelist — Import-Analyse §7.5) ---
    static constexpr const char* kMathFns[] = {
        "sin", "cos", "tan", "asin", "acos", "atan", "sqrt", "abs",
        "floor", "ceil", "exp", "log", "min", "max", "fmod"
    };
    lua_getglobal(L, "math");
    for (const char* name : kMathFns)
    {
        lua_getfield(L, -1, name);
        lua_setfield(L, -3, name);
    }
    // mod = math.fmod (natural float modulo for hand-written scripts;
    // EEL integer % lives in eel.mod)
    lua_getfield(L, -1, "fmod");
    lua_setfield(L, -3, "mod");
    // atan2 = math.atan (two-argument form in Lua 5.4)
    lua_getfield(L, -1, "atan");
    lua_setfield(L, -3, "atan2");
    lua_getfield(L, -1, "huge");
    lua_setfield(L, -3, "huge");
    lua_pop(L, 1);  // math

    // --- constants ---
    lua_pushnumber(L, 3.14159265358979323846);
    lua_setfield(L, -2, "pi");
    lua_pushnumber(L, 6.28318530717958647692);
    lua_setfield(L, -2, "pi2");

    // --- engine-bound functions ---
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, &LuaScriptEngine::lRand, 1);
    lua_setfield(L, -2, "rand");

    // --- audio analysis (bare globals, AVS getspec/getosc/gettime) ---
    const struct { const char* name; lua_CFunction fn; } audioFns[] = {
        {"getspec", &LuaScriptEngine::lGetSpec},
        {"getosc", &LuaScriptEngine::lGetOsc},
        {"gettime", &LuaScriptEngine::lGetTime},
    };
    for (const auto& [name, fn] : audioFns)
    {
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, fn, 1);
        lua_setfield(L, -2, name);
    }

    // --- eel prelude (EEL-faithful semantics for transpiled code) ---
    lua_newtable(L);
    const luaL_Reg eelFns[] = {
        {"truthy", lTruthy}, {"equal", lEqual},   {"above", lAbove},
        {"below", lBelow},   {"band", lBand},     {"bor", lBor},
        {"bnot", lBnot},     {"bitand", lBitAnd}, {"bitor", lBitOr},
        {"mod", lMod},       {"toint", lToInt},   {"sqrt", lEelSqrt},
        {"invsqrt", lInvSqrt}, {"sigmoid", lSigmoid}, {"sign", lSign},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, eelFns, 0);
    const struct { const char* name; lua_CFunction fn; } boundFns[] = {
        {"rand", &LuaScriptEngine::lRand},
        {"mbread", &LuaScriptEngine::lMbRead},
        {"mbwrite", &LuaScriptEngine::lMbWrite},
        {"gmbread", &LuaScriptEngine::lGmbRead},
        {"gmbwrite", &LuaScriptEngine::lGmbWrite},
    };
    for (const auto& [name, fn] : boundFns)
    {
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, fn, 1);
        lua_setfield(L, -2, name);
    }
    lua_setfield(L, -2, "eel");

    // --- app-global atomic register set (decision §10.3) ---
    lua_newtable(L);
    lua_pushcfunction(L, &LuaScriptEngine::lAppGet);
    lua_setfield(L, -2, "gget");
    lua_pushcfunction(L, &LuaScriptEngine::lAppSet);
    lua_setfield(L, -2, "gset");
    lua_setfield(L, -2, "app");

    // --- unknown reads -> 0.0 (EEL semantics) ---
    lua_newtable(L);
    lua_pushcfunction(L, lEnvIndex);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    m_envRef = luaL_ref(L, LUA_REGISTRYINDEX);
}

bool LuaScriptEngine::pushEnv() const
{
    if (m_state == nullptr || m_envRef == LUA_NOREF) return false;
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_envRef);
    return true;
}

// =============================================================================
// Compilation
// =============================================================================

bool LuaScriptEngine::compile(Slot slot, const std::string& source, const char* chunkName)
{
    if (m_state == nullptr) return false;

    clear(slot);
    if (isBlank(source)) return true;  // empty source = cleared slot, no error

    lua_State* L = m_state;
    if (luaL_loadbufferx(L, source.data(), source.size(), chunkName, "t") != LUA_OK)
    {
        m_lastError = lua_tostring(L, -1) != nullptr ? lua_tostring(L, -1) : "unknown compile error";
        lua_pop(L, 1);
        return false;
    }

    // Bind the sandbox environment (_ENV is upvalue 1 of a main chunk)
    pushEnv();
    lua_setupvalue(L, -2, 1);

    m_slotRefs[static_cast<int>(slot)] = luaL_ref(L, LUA_REGISTRYINDEX);
    return true;
}

void LuaScriptEngine::clear(Slot slot)
{
    int& ref = m_slotRefs[static_cast<int>(slot)];
    if (m_state != nullptr && ref != LUA_NOREF)
    {
        luaL_unref(m_state, LUA_REGISTRYINDEX, ref);
    }
    ref = LUA_NOREF;
}

bool LuaScriptEngine::has(Slot slot) const
{
    return m_slotRefs[static_cast<int>(slot)] != LUA_NOREF;
}

// =============================================================================
// Execution
// =============================================================================

bool LuaScriptEngine::run(Slot slot)
{
    const int ref = m_slotRefs[static_cast<int>(slot)];
    if (m_state == nullptr || ref == LUA_NOREF) return false;

    lua_State* L = m_state;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        m_lastError = lua_tostring(L, -1) != nullptr ? lua_tostring(L, -1) : "unknown runtime error";
        lua_pop(L, 1);
        clear(slot);  // disable: no error spam from the render loop
        return false;
    }
    return true;
}

// =============================================================================
// Environment variables
// =============================================================================

void LuaScriptEngine::setNumber(const char* name, double value)
{
    if (!pushEnv()) return;
    lua_pushnumber(m_state, value);
    lua_setfield(m_state, -2, name);
    lua_pop(m_state, 1);
}

double LuaScriptEngine::number(const char* name) const
{
    if (!pushEnv()) return 0.0;
    lua_getfield(m_state, -1, name);  // __index yields 0.0 for unknown names
    const double value = lua_tonumber(m_state, -1);
    lua_pop(m_state, 2);
    return value;
}

// =============================================================================
// Diagnostics / tests
// =============================================================================

bool LuaScriptEngine::evalNumber(const std::string& expr, double& out)
{
    if (m_state == nullptr) return false;

    lua_State* L = m_state;
    const std::string chunk = "return (" + expr + ")";
    if (luaL_loadbufferx(L, chunk.data(), chunk.size(), "eval", "t") != LUA_OK)
    {
        m_lastError = lua_tostring(L, -1) != nullptr ? lua_tostring(L, -1) : "unknown compile error";
        lua_pop(L, 1);
        return false;
    }
    pushEnv();
    lua_setupvalue(L, -2, 1);
    if (lua_pcall(L, 0, 1, 0) != LUA_OK)
    {
        m_lastError = lua_tostring(L, -1) != nullptr ? lua_tostring(L, -1) : "unknown runtime error";
        lua_pop(L, 1);
        return false;
    }
    out = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return true;
}

} // namespace lumi::scripting

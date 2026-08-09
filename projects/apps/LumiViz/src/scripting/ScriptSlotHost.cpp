/**
 ****************************************************************************************
 * @file   ScriptSlotHost.cpp
 * @brief  Implementation of the shared EEL slot-quartet host
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "scripting/ScriptSlotHost.hpp"

#include <EelTranspiler.hpp>

#include <cctype>

namespace lumi::scripting {

namespace {

constexpr std::array<const char*, LuaScriptEngine::kSlotCount> kSlotNames = {
    "init", "beat", "frame", "point"};

bool isIdentChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

char toLowerAscii(char c)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

/// Weist die EEL-Quelle dem Wort irgendwo einen Wert zu (`wort = …`, nicht
/// `==`)? Case-insensitiv, Wortgrenzen wie sourceMentions. Vergleiche wie
/// `time<=3` zaehlen nicht: nach dem Wort folgt dort kein `=` als ERSTES
/// Zeichen; `time==x` faellt durch den Blick auf das Folgezeichen.
bool sourceAssigns(const std::string& src, std::string_view word)
{
    if (word.empty() || src.size() < word.size()) return false;
    for (std::size_t i = 0; i + word.size() <= src.size(); ++i)
    {
        if (i > 0 && isIdentChar(src[i - 1])) continue;
        std::size_t j = 0;
        while (j < word.size() &&
               toLowerAscii(src[i + j]) == toLowerAscii(word[j]))
        {
            ++j;
        }
        if (j != word.size()) continue;
        std::size_t k = i + j;
        if (k < src.size() && isIdentChar(src[k])) continue;  // laengerer Name
        while (k < src.size() &&
               std::isspace(static_cast<unsigned char>(src[k])) != 0)
        {
            ++k;
        }
        if (k < src.size() && src[k] == '=' &&
            (k + 1 >= src.size() || src[k + 1] != '='))
        {
            return true;
        }
    }
    return false;
}

} // namespace

ScriptSlotHost::ScriptSlotHost(std::string chunkPrefix,
                               std::shared_ptr<ScriptContext> context,
                               Dialect dialect)
    : m_chunkPrefix(std::move(chunkPrefix)),
      m_dialect(dialect),
      m_engine(std::move(context))
{
}

void ScriptSlotHost::setSource(Slot slot, std::string eelSource)
{
    m_sources[static_cast<std::size_t>(slot)] = std::move(eelSource);
}

bool ScriptSlotHost::sourceMentions(Slot slot, std::string_view word) const
{
    const std::string& src = m_sources[static_cast<std::size_t>(slot)];
    if (word.empty() || src.size() < word.size()) return false;

    for (std::size_t i = 0; i + word.size() <= src.size(); ++i)
    {
        if (i > 0 && isIdentChar(src[i - 1])) continue;  // not a word start
        std::size_t j = 0;
        while (j < word.size() &&
               toLowerAscii(src[i + j]) == toLowerAscii(word[j]))
        {
            ++j;
        }
        if (j == word.size() &&
            (i + j >= src.size() || !isIdentChar(src[i + j])))
        {
            return true;
        }
    }
    return false;
}

bool ScriptSlotHost::compileAll()
{
    // In AVS-EEL ist `time` ein USER-Name. Weist eines der vier Quartette ihn
    // zu, gehoert er dem Skript — der Host darf seine Bequemlichkeits-Uhr
    // `time` (E1) dann nicht mehr jeden Frame darueberschreiben. el-vis_hypno07
    // setzt `time=2.0` als Laufmittel-Konstante; mit Inject wurde daraus die
    // Sekundenuhr und die Laufmittel-Laenge wuchs mit der Spielzeit (S59).
    bool timeOwned = false;
    for (const std::string& src : m_sources) timeOwned = timeOwned || sourceAssigns(src, "time");
    m_engine.setTimeInjectable(!timeOwned);

    for (int s = 0; s < LuaScriptEngine::kSlotCount; ++s)
    {
        const auto slot = static_cast<Slot>(s);
        const std::string chunkName = m_chunkPrefix + "." + kSlotNames[s];
        const auto eelDialect = m_dialect == Dialect::Milkdrop
                                    ? lumi::eel::Dialect::Milkdrop
                                    : lumi::eel::Dialect::Avs;

        const auto result =
            lumi::eel::transpile(m_sources[static_cast<std::size_t>(s)], eelDialect);
        if (!result.ok)
        {
            // Transpile error = slot stays empty (AVS behavior), first error kept
            m_engine.clear(slot);
            m_sharedVars[static_cast<std::size_t>(s)].clear();
            recordError(chunkName + ": " + result.error);
            continue;
        }

        if (!m_engine.compile(slot, result.lua, chunkName.c_str()))
        {
            m_sharedVars[static_cast<std::size_t>(s)].clear();
            recordError(m_engine.lastError());
            continue;
        }
        m_sharedVars[static_cast<std::size_t>(s)] = scanSharedVars(result.lua);
    }
    return m_lastError.empty();
}

bool ScriptSlotHost::run(Slot slot)
{
    if (!m_engine.has(slot)) return false;

    ScriptContext& ctx = *m_engine.context();
    const auto& shared = m_sharedVars[static_cast<std::size_t>(slot)];

    // Pull shared values into the environment before the call ...
    for (const SharedVar& var : shared)
    {
        const double value = var.kind == SharedVar::Kind::Reg ? ctx.reg(var.index)
                                                              : ctx.q(var.index);
        m_engine.setNumber(var.name.c_str(), value);
    }

    const bool ok = m_engine.run(slot);
    if (!ok)
    {
        recordError(m_engine.lastError());
        return false;
    }

    // ... and push the (possibly changed) values back afterwards
    for (const SharedVar& var : shared)
    {
        const double value = m_engine.number(var.name.c_str());
        if (var.kind == SharedVar::Kind::Reg)
        {
            ctx.setReg(var.index, value);
        }
        else
        {
            ctx.setQ(var.index, value);
        }
    }
    return true;
}

std::vector<ScriptSlotHost::SharedVar>
ScriptSlotHost::scanSharedVars(const std::string& luaSource)
{
    std::vector<SharedVar> vars;
    const auto alreadyKnown = [&vars](const std::string& name) {
        for (const SharedVar& v : vars)
        {
            if (v.name == name) return true;
        }
        return false;
    };

    std::size_t i = 0;
    const std::size_t n = luaSource.size();
    while (i < n)
    {
        if (!isIdentChar(luaSource[i]) || std::isdigit(static_cast<unsigned char>(luaSource[i])) != 0)
        {
            // skip non-word content and anything starting with a digit
            while (i < n && isIdentChar(luaSource[i])) ++i;
            while (i < n && !isIdentChar(luaSource[i])) ++i;
            continue;
        }

        const std::size_t start = i;
        while (i < n && isIdentChar(luaSource[i])) ++i;
        const std::string word = luaSource.substr(start, i - start);

        // reg00..reg99 (transpiler output is lowercase)
        if (word.size() == 5 && word.compare(0, 3, "reg") == 0 &&
            std::isdigit(static_cast<unsigned char>(word[3])) != 0 &&
            std::isdigit(static_cast<unsigned char>(word[4])) != 0)
        {
            if (!alreadyKnown(word))
            {
                const int index = (word[3] - '0') * 10 + (word[4] - '0');
                vars.push_back(SharedVar{word, SharedVar::Kind::Reg, index});
            }
            continue;
        }

        // q1..q64
        if (word.size() >= 2 && word.size() <= 3 && word[0] == 'q')
        {
            bool digits = true;
            for (std::size_t k = 1; k < word.size(); ++k)
            {
                digits = digits && std::isdigit(static_cast<unsigned char>(word[k])) != 0;
            }
            if (digits)
            {
                const int index = std::stoi(word.substr(1));
                if (index >= 1 && index <= ScriptContext::kQCount &&
                    word[1] != '0' && !alreadyKnown(word))
                {
                    vars.push_back(SharedVar{word, SharedVar::Kind::Q, index});
                }
            }
        }
    }
    return vars;
}

void ScriptSlotHost::recordError(const std::string& message)
{
    if (m_lastError.empty())
    {
        m_lastError = message;
    }
}

} // namespace lumi::scripting

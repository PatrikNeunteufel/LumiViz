/**
 ****************************************************************************************
 * @file   ScriptLutModule.cpp
 * @brief  Implementation of the scriptable color LUT module
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/scripting/ScriptLutModule.hpp"

#include "scripting/ScriptSlotHost.hpp"

#include <algorithm>

namespace lumi::modules {

using scripting::ScriptSlotHost;
using Slot = ScriptSlotHost::Slot;

ScriptLutModule::ScriptLutModule(std::shared_ptr<scripting::ScriptContext> context)
    : m_context(std::move(context))
{
    fillIdentity();
}

ScriptLutModule::~ScriptLutModule() = default;

// =============================================================================
// Sources
// =============================================================================

void ScriptLutModule::setInitCode(const std::string& code)
{
    m_initCode = code;
    m_compiled = false;
}

void ScriptLutModule::setBeatCode(const std::string& code)
{
    m_beatCode = code;
    m_compiled = false;
}

void ScriptLutModule::setFrameCode(const std::string& code)
{
    m_frameCode = code;
    m_compiled = false;
}

void ScriptLutModule::setLevelCode(const std::string& code)
{
    m_levelCode = code;
    m_compiled = false;
}

// =============================================================================
// Execution
// =============================================================================

void ScriptLutModule::initializeScripts()
{
    if (m_script == nullptr)
    {
        m_script = std::make_unique<ScriptSlotHost>("scriptlut", m_context);
    }
    m_lastScriptError.clear();
    m_script->clearError();

    // Level maps to the host's Point slot (chunk name "scriptlut.point")
    m_script->setSource(Slot::Init, m_initCode);
    m_script->setSource(Slot::Beat, m_beatCode);
    m_script->setSource(Slot::Frame, m_frameCode);
    m_script->setSource(Slot::Point, m_levelCode);
    if (!m_script->compileAll())
    {
        m_lastScriptError = m_script->lastError();
    }

    auto& engine = m_script->engine();
    engine.setNumber("time", static_cast<double>(m_totalTime));
    engine.setNumber("b", 0.0);

    if (m_script->has(Slot::Init) && !m_script->run(Slot::Init) &&
        m_lastScriptError.empty())
    {
        m_lastScriptError = m_script->lastError();
    }
    m_compiled = true;
    m_lutBuilt = false;  // (re)build at the next execute()
}

void ScriptLutModule::execute(bool isBeat, float deltaTime)
{
    if (!m_compiled)
    {
        initializeScripts();
    }
    m_totalTime += deltaTime;

    auto& engine = m_script->engine();
    engine.setNumber("time", static_cast<double>(m_totalTime));
    engine.setNumber("dt", static_cast<double>(deltaTime));
    engine.setNumber("b", isBeat ? 1.0 : 0.0);

    if (isBeat && m_script->has(Slot::Beat) && !m_script->run(Slot::Beat))
    {
        m_lastScriptError = m_script->lastError();
    }
    if (m_script->has(Slot::Frame) && !m_script->run(Slot::Frame))
    {
        m_lastScriptError = m_script->lastError();
    }

    if (!m_lutBuilt || m_recompute)
    {
        rebuildLut();
    }
}

void ScriptLutModule::rebuildLut()
{
    if (!m_script->has(Slot::Point))
    {
        fillIdentity();  // no/errored level script: identity LUT
        m_lutBuilt = true;
        return;
    }

    auto& engine = m_script->engine();
    for (int i = 0; i < kEntries; ++i)
    {
        const double level = static_cast<double>(i) / (kEntries - 1);
        engine.setNumber("red", level);
        engine.setNumber("green", level);
        engine.setNumber("blue", level);

        if (!m_script->run(Slot::Point))
        {
            // Runtime error disabled the slot: identity from here on
            m_lastScriptError = m_script->lastError();
            fillIdentity();
            m_lutBuilt = true;
            return;
        }

        m_tables[0][static_cast<std::size_t>(i)] =
            std::clamp(static_cast<float>(engine.number("red")), 0.0f, 1.0f);
        m_tables[1][static_cast<std::size_t>(i)] =
            std::clamp(static_cast<float>(engine.number("green")), 0.0f, 1.0f);
        m_tables[2][static_cast<std::size_t>(i)] =
            std::clamp(static_cast<float>(engine.number("blue")), 0.0f, 1.0f);
    }
    m_lutBuilt = true;
}

// =============================================================================
// Access / Helpers
// =============================================================================

float ScriptLutModule::lut(int channel, int index) const
{
    if (channel < 0 || channel > 2) return 0.0f;
    const int i = std::clamp(index, 0, kEntries - 1);
    return m_tables[static_cast<std::size_t>(channel)][static_cast<std::size_t>(i)];
}

void ScriptLutModule::fillIdentity()
{
    for (int i = 0; i < kEntries; ++i)
    {
        const float level = static_cast<float>(i) / (kEntries - 1);
        m_tables[0][static_cast<std::size_t>(i)] = level;
        m_tables[1][static_cast<std::size_t>(i)] = level;
        m_tables[2][static_cast<std::size_t>(i)] = level;
    }
}

double ScriptLutModule::getVariable(const std::string& name) const
{
    return m_script != nullptr ? m_script->engine().number(name.c_str()) : 0.0;
}

void ScriptLutModule::setVariable(const std::string& name, double value)
{
    if (m_script != nullptr)
    {
        m_script->engine().setNumber(name.c_str(), value);
    }
}

void ScriptLutModule::resetState()
{
    m_script.reset();
    m_compiled = false;
    m_lutBuilt = false;
    m_totalTime = 0.0f;
    m_lastScriptError.clear();
    fillIdentity();
}

} // namespace lumi::modules

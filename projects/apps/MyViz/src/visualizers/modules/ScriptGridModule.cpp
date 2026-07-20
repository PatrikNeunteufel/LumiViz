/**
 ****************************************************************************************
 * @file   ScriptGridModule.cpp
 * @brief  Implementation of the scriptable grid field module
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/modules/scripting/ScriptGridModule.hpp"

#include "scripting/ScriptSlotHost.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::modules {

using scripting::ScriptSlotHost;
using Slot = ScriptSlotHost::Slot;

ScriptGridModule::ScriptGridModule(std::shared_ptr<scripting::ScriptContext> context)
    : m_context(std::move(context))
{
    fillIdentity();
}

ScriptGridModule::~ScriptGridModule() = default;

// =============================================================================
// Sources
// =============================================================================

void ScriptGridModule::setInitCode(const std::string& code)
{
    m_initCode = code;
    m_compiled = false;
}

void ScriptGridModule::setBeatCode(const std::string& code)
{
    m_beatCode = code;
    m_compiled = false;
}

void ScriptGridModule::setFrameCode(const std::string& code)
{
    m_frameCode = code;
    m_compiled = false;
}

void ScriptGridModule::setPointCode(const std::string& code)
{
    m_pointCode = code;
    m_compiled = false;
}

// =============================================================================
// Configuration
// =============================================================================

void ScriptGridModule::setGridSize(int xres, int yres)
{
    m_xres = std::clamp(xres, kMinRes, kMaxResX);
    m_yres = std::clamp(yres, kMinRes, kMaxResY);
    fillIdentity();
}

// =============================================================================
// Execution
// =============================================================================

void ScriptGridModule::initializeScripts()
{
    if (m_script == nullptr)
    {
        m_script = std::make_unique<ScriptSlotHost>("scriptgrid", m_context);
    }
    m_lastScriptError.clear();
    m_script->clearError();

    m_script->setSource(Slot::Init, m_initCode);
    m_script->setSource(Slot::Beat, m_beatCode);
    m_script->setSource(Slot::Frame, m_frameCode);
    m_script->setSource(Slot::Point, m_pointCode);
    if (!m_script->compileAll())
    {
        m_lastScriptError = m_script->lastError();
    }
    m_scriptSetsAlpha = m_script->sourceMentions(Slot::Point, "alpha");

    auto& engine = m_script->engine();
    engine.setNumber("w", 0.0);
    engine.setNumber("h", 0.0);
    engine.setNumber("time", static_cast<double>(m_totalTime));
    engine.setNumber("b", 0.0);

    if (m_script->has(Slot::Init) && !m_script->run(Slot::Init) &&
        m_lastScriptError.empty())
    {
        m_lastScriptError = m_script->lastError();
    }
    m_compiled = true;
}

void ScriptGridModule::execute(float width, float height, bool isBeat, float deltaTime)
{
    if (!m_compiled)
    {
        initializeScripts();
    }
    m_totalTime += deltaTime;

    auto& engine = m_script->engine();
    engine.setNumber("w", static_cast<double>(width));
    engine.setNumber("h", static_cast<double>(height));
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

    if (!m_script->has(Slot::Point))
    {
        fillIdentity();  // no/errored point script: identity mapping
        return;
    }

    m_field.resize(static_cast<std::size_t>(m_xres) * static_cast<std::size_t>(m_yres));
    std::size_t idx = 0;
    for (int gy = 0; gy < m_yres; ++gy)
    {
        const double y = static_cast<double>(gy) / (m_yres - 1) * 2.0 - 1.0;
        for (int gx = 0; gx < m_xres; ++gx, ++idx)
        {
            const double x = static_cast<double>(gx) / (m_xres - 1) * 2.0 - 1.0;
            const double d = std::sqrt(x * x + y * y);
            const double r = std::atan2(y, x);

            engine.setNumber("x", x);
            engine.setNumber("y", y);
            engine.setNumber("d", d);
            engine.setNumber("r", r);
            if (m_scriptSetsAlpha) engine.setNumber("alpha", 1.0);

            GridNode& out = m_field[idx];
            if (!m_script->run(Slot::Point))
            {
                // Runtime error disabled the slot: identity from here on
                m_lastScriptError = m_script->lastError();
                fillIdentity();
                return;
            }

            if (m_rectCoords)
            {
                out.u = static_cast<float>(engine.number("x"));
                out.v = static_cast<float>(engine.number("y"));
            }
            else
            {
                const double dOut = engine.number("d");
                const double rOut = engine.number("r");
                out.u = static_cast<float>(std::cos(rOut) * dOut);
                out.v = static_cast<float>(std::sin(rOut) * dOut);
            }
            out.alpha = m_scriptSetsAlpha
                            ? std::clamp(static_cast<float>(engine.number("alpha")),
                                         0.0f, 1.0f)
                            : 1.0f;
        }
    }
}

// =============================================================================
// Helpers
// =============================================================================

void ScriptGridModule::fillIdentity()
{
    m_field.assign(static_cast<std::size_t>(m_xres) * static_cast<std::size_t>(m_yres),
                   GridNode{});
    std::size_t idx = 0;
    for (int gy = 0; gy < m_yres; ++gy)
    {
        const float y = static_cast<float>(gy) / (m_yres - 1) * 2.0f - 1.0f;
        for (int gx = 0; gx < m_xres; ++gx, ++idx)
        {
            m_field[idx].u = static_cast<float>(gx) / (m_xres - 1) * 2.0f - 1.0f;
            m_field[idx].v = y;
            m_field[idx].alpha = 1.0f;
        }
    }
}

double ScriptGridModule::getVariable(const std::string& name) const
{
    return m_script != nullptr ? m_script->engine().number(name.c_str()) : 0.0;
}

void ScriptGridModule::setVariable(const std::string& name, double value)
{
    if (m_script != nullptr)
    {
        m_script->engine().setNumber(name.c_str(), value);
    }
}

void ScriptGridModule::resetState()
{
    m_script.reset();
    m_compiled = false;
    m_totalTime = 0.0f;
    m_lastScriptError.clear();
    fillIdentity();
}

} // namespace lumi::modules

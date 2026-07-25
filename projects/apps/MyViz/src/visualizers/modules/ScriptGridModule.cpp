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

namespace {
constexpr double kHalfPi = 1.57079632679489661923;
}  // namespace

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
    engine.setNumber("sw", 0.0);
    engine.setNumber("sh", 0.0);
    engine.setNumber("time", static_cast<double>(m_totalTime));
    engine.setNumber("b", 0.0);
    // AVS seeds alpha=0.5 once (r_dmove.cpp:295); the script owns it afterwards
    // (persistent like every EEL var, NOT reset per point).
    engine.setNumber("alpha", 0.5);

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
    // AVS registers the surface size as sw/sh for Movement point code
    // (r_trans.cpp); built-in formulas #9/#10 divide by sqrt((sw^2+sh^2)/4).
    engine.setNumber("sw", static_cast<double>(width));
    engine.setNumber("sh", static_cast<double>(height));
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

    // AVS polar convention (r_trans.cpp:459-464 / r_dmove.cpp:324-332): d and r
    // live in PIXEL space — d = pixel distance from center over the half
    // diagonal (corner = 1), r = atan2 over pixel offsets (+pi/2). Only x/y are
    // NDC. On square surfaces this equals the former NDC math; on non-square
    // ones it removes the aspect distortion (S2: circles stay circles).
    const double halfW = width > 0.0f ? width * 0.5 : 1.0;
    const double halfH = height > 0.0f ? height * 0.5 : 1.0;
    const double maxD = std::sqrt(halfW * halfW + halfH * halfH);
    const double invMaxD = 1.0 / maxD;

    m_field.resize(static_cast<std::size_t>(m_xres) * static_cast<std::size_t>(m_yres));
    std::size_t idx = 0;
    for (int gy = 0; gy < m_yres; ++gy)
    {
        const double y = static_cast<double>(gy) / (m_yres - 1) * 2.0 - 1.0;
        for (int gx = 0; gx < m_xres; ++gx, ++idx)
        {
            const double x = static_cast<double>(gx) / (m_xres - 1) * 2.0 - 1.0;
            // Konvention Skript-Rand (S46, Befund A): das Skript sieht den
            // AVS-Raum (y+ = unten) — unser Gitter/GL hat y+ = oben. Nur die
            // SICHT wird gespiegelt (yS), inkl. r: die Spiegelung kehrt sonst
            // die Drehrichtung aller Rotations-Skripte um. Rueckweg unten.
            const double yS = -y;
            const double xd = x * halfW;
            const double yd = yS * halfH;
            const double d = std::sqrt(xd * xd + yd * yd) * invMaxD;
            const double r = std::atan2(yd, xd) + kHalfPi;

            engine.setNumber("x", x);
            engine.setNumber("y", yS);
            engine.setNumber("d", d);
            engine.setNumber("r", r);

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
                out.v = -static_cast<float>(engine.number("y"));  // AVS -> GL
            }
            else
            {
                // back-transform in pixel space, then per-axis to NDC
                // (sin-Anteil traegt die AVS-y-Richtung -> zurueck nach GL)
                const double dPix = engine.number("d") * maxD;
                const double rOut = engine.number("r") - kHalfPi;
                out.u = static_cast<float>(std::cos(rOut) * dPix / halfW);
                out.v = -static_cast<float>(std::sin(rOut) * dPix / halfH);
            }
            out.alpha = m_scriptSetsAlpha
                            ? std::clamp(static_cast<float>(engine.number("alpha")),
                                         0.0f, 1.0f)
                            : 0.5f;  // AVS default (*var_alpha = 0.5)
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

void ScriptGridModule::setVisData(const unsigned char* data, double scriptTime)
{
    if (m_script != nullptr)
    {
        m_script->engine().setVisData(data);
        m_script->engine().setScriptTime(scriptTime);
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

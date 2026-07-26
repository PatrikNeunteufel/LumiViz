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
    // setVisData()-Aufrufe VOR dem Erst-Compile landeten ins Leere — ohne
    // Nachfuettern sahen Init/Beat des allerersten Frames nur Null-Audio (S47).
    if (m_visBytes != nullptr)
    {
        engine.setVisData(m_visBytes);
        engine.setScriptTime(m_visTime);
    }
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

    // r_dmove.cpp:297-298 / r_trans.cpp: FRAME zuerst, dann Beat — der
    // Frame-Code rechnet mit den Beat-Werten des VORHERIGEN Frames (S47).
    if (m_script->has(Slot::Frame) && !m_script->run(Slot::Frame))
    {
        m_lastScriptError = m_script->lastError();
    }
    if (isBeat && m_script->has(Slot::Beat) && !m_script->run(Slot::Beat))
    {
        m_lastScriptError = m_script->lastError();
    }

    const int w = std::max(1, static_cast<int>(width));
    const int h = std::max(1, static_cast<int>(height));
    if (!m_script->has(Slot::Point))
    {
        fillIdentity();  // no/errored point script: identity mapping
        fillIdentityFx(w, h);
        return;
    }

    // Gitterpositionen exakt wie r_dmove.cpp:304-332: der Schritt zwischen zwei
    // Gitterspalten ist ein TRUNKIERTER 16.16-Wert ((w<<16)/(XRES-1)) und wird
    // aufaddiert — die Stuetzstellen liegen also nicht exakt auf x*w/(XRES-1),
    // und der letzte Punkt erreicht den Rand knapp nicht.
    // AVS polar convention (r_trans.cpp:459-464 / r_dmove.cpp:324-332): d and r
    // live in PIXEL space — d = pixel distance from center over the half
    // diagonal (corner = 1), r = atan2 over pixel offsets (+pi/2). Only x/y are
    // NDC.
    const double dw2 = static_cast<double>(w) * 32768.0;  // (w/2) << 16
    const double dh2 = static_cast<double>(h) * 32768.0;
    const double xsc = 2.0 / w;
    const double ysc = 2.0 / h;
    const double maxD = std::sqrt(static_cast<double>(w) * w +
                                  static_cast<double>(h) * h) * 0.5;
    const double invMaxD = 1.0 / maxD;
    // r_dmove.cpp:311 skaliert max_screen_d NACH divmax_d auf 16.16 hoch; die
    // Klammerung unten (cos * (d*m)) ist die des Originals.
    const double maxScreenDFx = maxD * 65536.0;
    const double halfW = w * 0.5;
    const double halfH = h * 0.5;
    const int xcDpos = (w << 16) / (m_xres - 1);
    const int ycDpos = (h << 16) / (m_yres - 1);
    // Ohne AVS-Modus liegen die Stuetzstellen exakt (s. setAvsGridPositions):
    // dieselbe Schleife, nur ohne den Trunkierungsrest je Schritt.
    const double xStepExact = (w * 65536.0) / (m_xres - 1);
    const double yStepExact = (h * 65536.0) / (m_yres - 1);

    m_field.resize(static_cast<std::size_t>(m_xres) * static_cast<std::size_t>(m_yres));
    m_fieldFx.resize(m_field.size());
    // Zeilen laufen in AVS-Ordnung (0 = OBEN) — Punkt-Skripte mit Seiteneffekten
    // (Zaehler, megabuf) sehen sonst die umgekehrte Reihenfolge. Das float-Feld
    // bleibt GL-Ordnung (y+ = oben) und wird beim Schreiben gespiegelt.
    for (int gy = 0; gy < m_yres; ++gy)
    {
        const double ycPos = m_avsGridPositions ? static_cast<double>(gy) * ycDpos
                                                : static_cast<double>(gy) * yStepExact;
        const std::size_t rowGl =
            static_cast<std::size_t>(m_yres - 1 - gy) * static_cast<std::size_t>(m_xres);
        const std::size_t rowFx =
            static_cast<std::size_t>(gy) * static_cast<std::size_t>(m_xres);
        for (int gx = 0; gx < m_xres; ++gx)
        {
            const double xcPos = m_avsGridPositions
                                     ? static_cast<double>(gx) * xcDpos
                                     : static_cast<double>(gx) * xStepExact;
            const double xd = (xcPos - dw2) * (1.0 / 65536.0);
            const double yd = (ycPos - dh2) * (1.0 / 65536.0);

            // Konvention Skript-Rand (S46, Befund A): das Skript sieht den
            // AVS-Raum (y+ = unten). Rueckweg beim Schreiben von out.v.
            engine.setNumber("x", xd * xsc);
            engine.setNumber("y", yd * ysc);
            engine.setNumber("d", std::sqrt(xd * xd + yd * yd) * invMaxD);
            engine.setNumber("r", std::atan2(yd, xd) + kHalfPi);

            if (!m_script->run(Slot::Point))
            {
                // Runtime error disabled the slot: identity from here on
                m_lastScriptError = m_script->lastError();
                fillIdentity();
                fillIdentityFx(w, h);
                return;
            }

            GridNode& out = m_field[rowGl + static_cast<std::size_t>(gx)];
            GridNodeFx& fx = m_fieldFx[rowFx + static_cast<std::size_t>(gx)];
            if (m_rectCoords)
            {
                const double ox = engine.number("x");
                const double oy = engine.number("y");
                out.u = static_cast<float>(ox);
                out.v = -static_cast<float>(oy);  // AVS -> GL
                fx.x = static_cast<int>((ox + 1.0) * dw2);
                fx.y = static_cast<int>((oy + 1.0) * dh2);
            }
            else
            {
                // back-transform in pixel space, then per-axis to NDC
                // (sin-Anteil traegt die AVS-y-Richtung -> zurueck nach GL)
                const double dOut = engine.number("d");
                const double rOut = engine.number("r") - kHalfPi;
                const double dPix = dOut * maxD;
                out.u = static_cast<float>(std::cos(rOut) * dPix / halfW);
                out.v = -static_cast<float>(std::sin(rOut) * dPix / halfH);
                const double dFx = dOut * maxScreenDFx;
                fx.x = static_cast<int>(dw2 + std::cos(rOut) * dFx);
                fx.y = static_cast<int>(dh2 + std::sin(rOut) * dFx);
            }
            const double va = m_scriptSetsAlpha
                                  ? std::clamp(engine.number("alpha"), 0.0, 1.0)
                                  : 0.5;  // AVS default (*var_alpha = 0.5)
            out.alpha = static_cast<float>(va);
            fx.a = static_cast<int>(va * 255.0 * 65536.0);
        }
    }
}

// =============================================================================
// Helpers
// =============================================================================

void ScriptGridModule::fillIdentity()
{
    m_fieldFx.clear();  // ohne w/h nicht bestimmbar — fillIdentityFx() folgt
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

bool ScriptGridModule::buildTransTable(int width, int height, bool wrap,
                                       bool subpixel, std::vector<int>& out)
{
    if (!m_compiled) initializeScripts();
    if (!m_script->has(Slot::Point)) return false;

    const int w = std::max(2, width);
    const int h = std::max(2, height);
    out.assign(static_cast<std::size_t>(w) * static_cast<std::size_t>(h), 0);

    auto& engine = m_script->engine();
    engine.setNumber("w", static_cast<double>(w));
    engine.setNumber("h", static_cast<double>(h));
    engine.setNumber("sw", static_cast<double>(w));
    engine.setNumber("sh", static_cast<double>(h));

    // r_trans.cpp:428-451 — w2/h2 kommen aus INTEGER-Division (w/2), maxD aus
    // sqrt(w*w+h*h)/2. Zeile 0 ist oben (AVS-Raum, den das Skript sieht).
    const double maxD =
        std::sqrt(static_cast<double>(w) * w + static_cast<double>(h) * h) / 2.0;
    const double divMaxD = 1.0 / maxD;
    const double w2 = w / 2;
    const double h2 = h / 2;
    const double xsc = 1.0 / w2;
    const double ysc = 1.0 / h2;

    std::size_t idx = 0;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x, ++idx)
        {
            const double xd = x - w2;
            const double yd = y - h2;
            engine.setNumber("x", xd * xsc);
            engine.setNumber("y", yd * ysc);
            engine.setNumber("d", std::sqrt(xd * xd + yd * yd) * divMaxD);
            engine.setNumber("r", std::atan2(yd, xd) + kHalfPi);
            if (!m_script->run(Slot::Point))
            {
                m_lastScriptError = m_script->lastError();
                return false;  // Slot tot: Aufrufer laesst das Bild unberuehrt
            }

            double tmp1;  // Zielzeile
            double tmp2;  // Zielspalte
            if (m_rectCoords)
            {
                tmp1 = (engine.number("y") + 1.0) * h2;
                tmp2 = (engine.number("x") + 1.0) * w2;
            }
            else
            {
                const double dOut = engine.number("d") * maxD;
                const double rOut = engine.number("r") - kHalfPi;
                tmp1 = (h / 2) + std::sin(rOut) * dOut;
                tmp2 = (w / 2) + std::cos(rOut) * dOut;
            }

            int ow, oh;
            unsigned packed;
            if (subpixel)
            {
                oh = static_cast<int>(tmp1);
                ow = static_cast<int>(tmp2);
                int xPartial = static_cast<int>(32.0 * (tmp2 - ow));
                int yPartial = static_cast<int>(32.0 * (tmp1 - oh));
                if (wrap)
                {
                    // r_trans.cpp:387-393: Modulo auf w-1/h-1 — die letzte
                    // Spalte/Zeile bleibt als BLEND4-Nachbar frei.
                    ow %= (w - 1);
                    oh %= (h - 1);
                    if (ow < 0) ow += w - 1;
                    if (oh < 0) oh += h - 1;
                }
                else
                {
                    if (ow < 0) { xPartial = 0; ow = 0; }
                    if (ow >= w - 1) { xPartial = 31; ow = w - 2; }
                    if (oh < 0) { yPartial = 0; oh = 0; }
                    if (oh >= h - 1) { yPartial = 31; oh = h - 2; }
                }
                packed = static_cast<unsigned>(ow + oh * w) |
                         (static_cast<unsigned>(yPartial) << 22) |
                         (static_cast<unsigned>(xPartial) << 27);
            }
            else
            {
                // Ohne Subpixel rundet das Original (+0.5) statt zu trunkieren.
                tmp1 += 0.5;
                tmp2 += 0.5;
                oh = static_cast<int>(tmp1);
                ow = static_cast<int>(tmp2);
                if (wrap)
                {
                    ow %= w;
                    oh %= h;
                    if (ow < 0) ow += w;
                    if (oh < 0) oh += h;
                }
                else
                {
                    ow = std::clamp(ow, 0, w - 1);
                    oh = std::clamp(oh, 0, h - 1);
                }
                packed = static_cast<unsigned>(ow + oh * w);
            }
            out[idx] = static_cast<int>(packed);
        }
    }
    return true;
}

void ScriptGridModule::fillIdentityFx(int width, int height)
{
    // Identitaet in AVS-Rohform = die Gitterposition selbst (r_dmove rect-Zweig
    // mit unveraendertem x/y liefert genau xc_pos/yc_pos zurueck).
    m_fieldFx.assign(static_cast<std::size_t>(m_xres) * static_cast<std::size_t>(m_yres),
                     GridNodeFx{});
    const int xcDpos = (width << 16) / (m_xres - 1);
    const int ycDpos = (height << 16) / (m_yres - 1);
    const int alpha = static_cast<int>(0.5 * 255.0 * 65536.0);
    std::size_t idx = 0;
    int ycPos = 0;
    for (int gy = 0; gy < m_yres; ++gy)
    {
        int xcPos = 0;
        for (int gx = 0; gx < m_xres; ++gx, ++idx)
        {
            m_fieldFx[idx] = GridNodeFx{xcPos, ycPos, alpha};
            xcPos += xcDpos;
        }
        ycPos += ycDpos;
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
    m_visBytes = data;  // gepuffert: initializeScripts() fuettert nach
    m_visTime = scriptTime;
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

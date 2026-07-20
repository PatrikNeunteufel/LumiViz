/**
 ****************************************************************************************
 * @file   ScriptContext.hpp
 * @brief  Shared per-preset script state: reg00-99, q1-q64 (snapshots), gmegabuf
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * One ScriptContext is shared by all script hosts of ONE preset/visualizer
 * instance (Import-Fundament-Entwurf §1, decision Import-Analyse §10.3:
 * reg/gmegabuf are preset-local). Engines receive it as shared_ptr; without an
 * explicit context every engine gets a private one (previous behavior).
 *
 * q1..q64 carry the MilkDrop data flow (MD3 superset, decision E5): the host
 * calls captureInitSnapshot() after the init runs, restoreInitSnapshot() at
 * the start of each frame, captureFrameSnapshot() after the frame run —
 * point-level runs then read the frame values. AVS-dialect hosts simply never
 * use the q API.
 *
 * Threading: a ScriptContext belongs to exactly one render thread — all script
 * hosts of a preset run sequentially on it, so there is NO locking here
 * (Visualizer_Architecture §12). The app-global atomic register set
 * (app.gget/gset) is separate and remains in LuaScriptEngine.
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

namespace lumi::scripting {

/**
 * @class ScriptContext
 * @brief Preset-local shared state for all script engines of one preset
 */
class ScriptContext
{
public:
    static constexpr int kRegCount = 100;  ///< reg00..reg99 (EEL global registers)
    static constexpr int kQCount = 64;     ///< q1..q64 (MilkDrop, MD3 superset)

    // =========================================================================
    // reg00..reg99
    // =========================================================================

    [[nodiscard]] double reg(int index) const
    {
        return (index >= 0 && index < kRegCount)
                   ? m_regs[static_cast<std::size_t>(index)]
                   : 0.0;
    }

    void setReg(int index, double value)
    {
        if (index >= 0 && index < kRegCount)
        {
            m_regs[static_cast<std::size_t>(index)] = value;
        }
    }

    // =========================================================================
    // q1..q64 with MilkDrop snapshot semantics
    // =========================================================================

    /// @param index 1-based (q1 -> 1), matching the script names
    [[nodiscard]] double q(int index) const
    {
        return (index >= 1 && index <= kQCount)
                   ? m_q[static_cast<std::size_t>(index - 1)]
                   : 0.0;
    }

    void setQ(int index, double value)
    {
        if (index >= 1 && index <= kQCount)
        {
            m_q[static_cast<std::size_t>(index - 1)] = value;
        }
    }

    /// @brief Freeze the post-init q values (call once after the init runs)
    void captureInitSnapshot() { m_qInit = m_q; }

    /// @brief Reset q to the post-init values (call at the start of each frame)
    void restoreInitSnapshot() { m_q = m_qInit; }

    /// @brief Freeze the post-frame q values (basis for point-level runs)
    void captureFrameSnapshot() { m_qFrame = m_q; }

    /// @brief Reset q to the post-frame values (e.g. before each point batch)
    void restoreFrameSnapshot() { m_q = m_qFrame; }

    // =========================================================================
    // gmegabuf (shared script buffer; index rules live in the engine closures)
    // =========================================================================

    [[nodiscard]] double gmbRead(std::int64_t index) const
    {
        const auto it = m_gmegabuf.find(index);
        return it != m_gmegabuf.end() ? it->second : 0.0;
    }

    void gmbWrite(std::int64_t index, double value) { m_gmegabuf[index] = value; }

    void reset()
    {
        m_regs.fill(0.0);
        m_q.fill(0.0);
        m_qInit.fill(0.0);
        m_qFrame.fill(0.0);
        m_gmegabuf.clear();
    }

private:
    std::array<double, kRegCount> m_regs{};
    std::array<double, kQCount> m_q{};
    std::array<double, kQCount> m_qInit{};
    std::array<double, kQCount> m_qFrame{};
    std::unordered_map<std::int64_t, double> m_gmegabuf;
};

} // namespace lumi::scripting

/**
 ****************************************************************************************
 * @file   OffscreenBufferPool.hpp
 * @brief  Up to 8 named offscreen FBO slots per owner (AVS global-buffer model)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Import-Fundament-Entwurf §3 (Roadmap 4.3, API scaffolding only): the
 * counterpart of AVS' getGlobalBuffer (ref rlib.cpp:430) — buffers are
 * allocated on demand, are dropped and reallocated on size changes, and live
 * per owner (per preset, later per multi-effect host). Real users (Buffer
 * Save, blend mode "Buffer") arrive with Roadmap 5.
 *
 * Lifecycle: render-thread owned; call clear() with the context current
 * (onCleanup/context change).
 ****************************************************************************************
 */

#pragma once

#include <QOpenGLFramebufferObject>

#include <array>
#include <memory>

namespace lumi::render {

/**
 * @class OffscreenBufferPool
 * @brief On-demand pool of 8 offscreen buffers (AVS getGlobalBuffer semantics)
 */
class OffscreenBufferPool
{
public:
    static constexpr int kSlots = 8;

    /**
     * @brief Fetch slot n at the given size
     * @param allocate false: only return an existing, size-matching buffer
     * @return nullptr for invalid slot, size mismatch without allocate, or
     *         failed creation
     */
    QOpenGLFramebufferObject* get(int n, int width, int height, bool allocate)
    {
        if (n < 0 || n >= kSlots || width <= 0 || height <= 0) return nullptr;
        auto& slot = m_buffers[static_cast<std::size_t>(n)];

        if (slot != nullptr &&
            (slot->width() != width || slot->height() != height))
        {
            slot.reset();   // size changed: drop (AVS behavior)
        }
        if (slot == nullptr)
        {
            if (!allocate) return nullptr;
            auto fbo = std::make_unique<QOpenGLFramebufferObject>(width, height);
            if (!fbo->isValid()) return nullptr;
            slot = std::move(fbo);
        }
        return slot.get();
    }

    /// @brief Release all buffers (context must be current)
    void clear()
    {
        for (auto& slot : m_buffers) slot.reset();
    }

private:
    std::array<std::unique_ptr<QOpenGLFramebufferObject>, kSlots> m_buffers;
};

} // namespace lumi::render

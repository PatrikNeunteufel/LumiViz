/**
 ****************************************************************************************
 * @file   SetParamCommand.hpp
 * @brief  Undo-able parameter change on an IVisualizer (Phase 4)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "services/ICommand.hpp"
#include "visualizers/IVisualizer.hpp"

#include <QMutex>

#include <string>
#include <utility>

/**
 * @class SetParamCommand
 * @brief Sets a visualizer parameter; undo restores the previous value
 *
 * Consecutive changes to the SAME parameter of the SAME visualizer merge into
 * one undo step (slider-drag coalescing via ICommand::canMergeWith).
 *
 * Since the render-thread decoupling the command locks the visualizer's
 * render mutex around setParam — undo/redo fires from the menu and bypasses
 * the ConfigPanel, so the guard must live HERE (QMutexLocker is null-safe).
 *
 * @warning Holds a reference to the visualizer. Whoever owns the CommandBus
 *          history MUST clear() it when the visualizer changes or dies
 *          (ConfigPanel does this in setVisualizer()).
 */
class SetParamCommand : public ICommand
{
public:
    using ParamValue = lumi::modules::ParamValue;

    SetParamCommand(IVisualizer& visualizer,
                    std::string paramId,
                    ParamValue oldValue,
                    ParamValue newValue,
                    QMutex* renderMutex = nullptr)
        : m_visualizer(visualizer)
        , m_paramId(std::move(paramId))
        , m_oldValue(std::move(oldValue))
        , m_newValue(std::move(newValue))
        , m_renderMutex(renderMutex)
    {
    }

    bool execute() override
    {
        QMutexLocker lock(m_renderMutex);
        return m_visualizer.setParam(m_paramId, m_newValue);
    }

    void undo() override
    {
        QMutexLocker lock(m_renderMutex);
        m_visualizer.setParam(m_paramId, m_oldValue);
    }

    [[nodiscard]] std::string description() const override
    {
        return "Set " + m_paramId;
    }

    [[nodiscard]] bool canMergeWith(const ICommand& next) const override
    {
        const auto* other = dynamic_cast<const SetParamCommand*>(&next);
        return other != nullptr
            && &other->m_visualizer == &m_visualizer
            && other->m_paramId == m_paramId;
    }

    void mergeWith(const ICommand& next) override
    {
        // canMergeWith guarantees the type; keep our old value, adopt the target
        const auto& other = static_cast<const SetParamCommand&>(next);
        m_newValue = other.m_newValue;
    }

private:
    IVisualizer& m_visualizer;
    std::string m_paramId;
    ParamValue m_oldValue;
    ParamValue m_newValue;
    QMutex* m_renderMutex;  ///< non-owning; guards against the render thread
};

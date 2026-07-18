/**
 ****************************************************************************************
 * @file   CommandDispatcher.tpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <typeinfo>
namespace viz::core
{
    template<typename C>
    bool CommandDispatcher::send(const C& cmd, const CommandContext&, std::string label, bool recordUndo)
    {
        static_assert(std::is_copy_constructible_v<C>,
                      "CommandDispatcher::send<C>: C must be copy-constructible (redo stores a snapshot)");

        auto erased = m_reg.find(keyOf<C>());
        if (!erased)
            return false;

        // execute now, get undo memento
        std::function<void()> undo = erased(static_cast<const void*>(&cmd));
        if (!recordUndo)
            return true;

        // redo replays the same command without stacking again
        auto redo = [this, snapshot = cmd]()
        { (void)this->send<C>(snapshot, CommandContext{}, {}, /*recordUndo*/ false); };

        if (label.empty())
            label = defaultLabel(typeid(C).name());
        m_hist.push({std::move(undo), std::move(redo), std::move(label)});
        return true;
    }
} // namespace viz::core

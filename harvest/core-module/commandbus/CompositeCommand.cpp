/**
 ****************************************************************************************
 * @file   CompositeCommand.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "CompositeCommand.hpp"

namespace viz::core
{

    CompositeCommand::CompositeCommand(std::string label)
    : m_label(std::move(label)) {}

    const char* CompositeCommand::name() const noexcept
    {
        return m_label.c_str();
    }

    void CompositeCommand::add(const ICommandPtr& c)
    {
        if (c)
            m_children.push_back(c);
    }

    CommandResult CompositeCommand::execute(const CommandContext& ctx)
    {
        if (m_executed)
            return CommandResult::Ok();
        for (std::size_t i = 0; i < m_children.size(); ++i)
        {
            CommandResult r = m_children[i]->execute(ctx);
            if (!r.ok)
            {
                for (std::size_t j = i; j-- > 0;)
                    (void)m_children[j]->undo(ctx);
                return r;
            }
        }
        m_executed = true;
        return CommandResult::Ok();
    }

CommandResult CompositeCommand::undo(const CommandContext& ctx)
    {
        // Always attempt to undo children in reverse order.
        // Rationale:
        // - In transactional usage, child commands were already executed BEFORE this composite
        //   existed (the bus executed them while the transaction was open).
        //   In that case, this composite might have never seen execute(), so m_executed can be false.
        //   We still must rollback those children.
        // - Each child command is responsible for being idempotent/safe if undo() is called
        //   when it was not executed (it should no-op).
        for (std::size_t i = m_children.size(); i-- > 0;)
        {
            (void)m_children[i]->undo(ctx);
        }
        m_executed = false;
        return CommandResult::Ok();
    }


} // namespace viz::core

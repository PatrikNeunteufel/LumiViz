/**
 ****************************************************************************************
 * @file   TransactionGuard.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "TransactionGuard.hpp"

namespace viz::core
{
    TransactionGuard::TransactionGuard(CommandBus& bus, std::string label)
    : m_bus(bus), m_label(std::move(label)), m_active(true)
    {
        m_bus.beginTransaction(m_label);
    }
    TransactionGuard::~TransactionGuard()
    {
        if (m_active)
        {
            CommandContext ctx{}; // only needed if rollback undoes buffered cmds
            m_bus.rollbackTransaction(ctx);
        }
    }
    void TransactionGuard::commit()
    {
        m_bus.commitTransaction();
        m_active = false;
    }
}

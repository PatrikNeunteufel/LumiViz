/**
 ****************************************************************************************
 * @file   TransactionGuard.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <string>

#include "CommandBus.hpp"
#include "CommandContext.hpp"

namespace viz::core
{

    /**
     * @brief RAII guard for CommandBus transactions.
     * If not committed, rolls back on destruction.
     */
    class TransactionGuard
    {
      public:
        TransactionGuard(CommandBus& bus, std::string label = {});

        ~TransactionGuard();

        void commit();

      private:
        CommandBus& m_bus;
        std::string m_label;
        bool m_active{false};
    };

} // namespace viz::core

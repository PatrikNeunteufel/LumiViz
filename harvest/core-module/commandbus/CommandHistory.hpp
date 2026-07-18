/**
 ****************************************************************************************
 * @file   CommandHistory.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */

#pragma once
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace viz::core
{
    struct HistoryEntry
    {
        std::function<void()> undo;
        std::function<void()> redo;
        std::string label;
    };
    class CommandHistory
    {
      public:
        void begin(const std::string& label = {});
        void commit();
        void rollback();
        void push(HistoryEntry e); // respects transaction
        bool undo();
        bool redo();
        bool canUndo() const noexcept;
        bool canRedo() const noexcept;
        std::string topUndo() const;
        std::string topRedo() const;

      private:
        void clearRedo();
        mutable std::mutex m_mtx;
        std::vector<HistoryEntry> m_undo, m_redo, m_txbuf;
        int m_txDepth{0};
        std::string m_txLabel;
    };
} // namespace viz::core

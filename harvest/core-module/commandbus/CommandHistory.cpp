/**
 ****************************************************************************************
 * @file   CommandHistory.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "CommandHistory.hpp"

#include <cassert>
namespace viz::core
{
    void CommandHistory::begin(const std::string& l)
    {
        std::lock_guard lk(m_mtx);
        if (m_txDepth++ == 0)
        {
            m_txLabel = l;
            m_txbuf.clear();
        }
    }
    void CommandHistory::commit()
    {
        std::lock_guard lk(m_mtx);
        assert(m_txDepth > 0);
        if (--m_txDepth == 0)
        {
            if (!m_txbuf.empty())
            {
                auto vec = std::move(m_txbuf);
                auto undo = [vec]
                {
                    for (size_t i = vec.size(); i-- > 0;)
                        if (vec[i].undo)
                            vec[i].undo();
                };
                auto redo = [vec]
                {
                    for (size_t i = 0; i < vec.size(); ++i)
                        if (vec[i].redo)
                            vec[i].redo();
                };
                clearRedo();
                m_undo.push_back({std::move(undo), std::move(redo), m_txLabel.empty() ? "Transaction" : m_txLabel});
            }
            m_txLabel.clear();
            m_txbuf.clear();
        }
    }
    void CommandHistory::rollback()
    {
        std::lock_guard lk(m_mtx);
        assert(m_txDepth > 0);
        if (--m_txDepth == 0)
        {
            for (size_t i = m_txbuf.size(); i-- > 0;)
                if (m_txbuf[i].undo)
                    m_txbuf[i].undo();
            m_txbuf.clear();
            m_txLabel.clear();
        }
    }
    void CommandHistory::push(HistoryEntry e)
    {
        std::lock_guard lk(m_mtx);
        if (m_txDepth == 0)
        {
            clearRedo();
            m_undo.push_back(std::move(e));
        }
        else
        {
            m_txbuf.push_back(std::move(e));
        }
    }
    bool CommandHistory::undo()
    {
        std::lock_guard lk(m_mtx);
        if (m_txDepth > 0 || m_undo.empty())
            return false;
        auto e = std::move(m_undo.back());
        m_undo.pop_back();
        if (e.undo)
            e.undo();
        m_redo.push_back(std::move(e));
        return true;
    }
    bool CommandHistory::redo()
    {
        std::lock_guard lk(m_mtx);
        if (m_txDepth > 0 || m_redo.empty())
            return false;
        auto e = std::move(m_redo.back());
        m_redo.pop_back();
        if (e.redo)
            e.redo();
        m_undo.push_back(std::move(e));
        return true;
    }
    bool CommandHistory::canUndo() const noexcept
    {
        std::lock_guard lk(m_mtx);
        return !m_undo.empty();
    }
    bool CommandHistory::canRedo() const noexcept
    {
        std::lock_guard lk(m_mtx);
        return !m_redo.empty();
    }
    std::string CommandHistory::topUndo() const
    {
        std::lock_guard lk(m_mtx);
        return m_undo.empty() ? std::string{} : m_undo.back().label;
    }
    std::string CommandHistory::topRedo() const
    {
        std::lock_guard lk(m_mtx);
        return m_redo.empty() ? std::string{} : m_redo.back().label;
    }
    void CommandHistory::clearRedo()
    {
        m_redo.clear();
    }
} // namespace viz::core

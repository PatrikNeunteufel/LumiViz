/**
 ****************************************************************************************
 * @file   CommandRegistry.tpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
namespace viz::core
{

    template<typename C, typename F>
        requires std::is_invocable_r_v<std::function<void()>, F, const C&>
    void CommandRegistry::registerHandler(F&& fn)
    {
        // Zusätzlicher Check (freundliche Fehlermeldung):
        static_assert(std::is_invocable_r_v<std::function<void()>, F, const C&>,
                      "registerHandler<C>(F): F must be invocable as std::function<void()>(const C&)");

        std::unique_lock lk(m_mtx);
        // Type-erased Wrapper: void* -> const C& -> std::function<void()>
        m_handlers[keyOf<C>().id] = [f = std::forward<F>(fn)](const void* p) -> std::function<void()>
        {
            const C& cc = *static_cast<const C*>(p);
            return std::invoke(f, cc); // erwartet: std::function<void()> (Undo-Memento)
        };
    }

    template<typename C> bool CommandRegistry::hasHandler() const noexcept
    {
        std::shared_lock lk(m_mtx);
        return m_handlers.find(keyOf<C>().id) != m_handlers.end();
    }

    template<typename C> void CommandRegistry::unregisterHandler()
    {
        std::unique_lock lk(m_mtx);
        m_handlers.erase(keyOf<C>().id);
    }

    inline ErasedDo CommandRegistry::find(CommandKey k) const
    {
        std::shared_lock lk(m_mtx);
        if (auto it = m_handlers.find(k.id); it != m_handlers.end())
            return it->second;
        return {};
    }

} // namespace viz::core

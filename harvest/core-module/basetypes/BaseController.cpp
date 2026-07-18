/**
 ****************************************************************************************
 * @file   BaseController.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "BaseController.hpp"
#include "core/commandbus/CommandBus.hpp"   // ← NEU: holt die (Shim-)Definition rein
//#include "core/commandbus/CommandContext.hpp"


namespace viz::core
{
    BaseController::BaseController(ServiceContainer& services, EventBus& events, CommandBus& commands) noexcept
        : m_services(services), m_events(events), m_commands(commands)
    {
    }

    BaseController::~BaseController() = default;

    bool BaseController::initialize() noexcept
    {
        return onInitialize();
    }
    bool BaseController::start() noexcept
    {
        return onStart();
    }
    bool BaseController::tick(float dt) noexcept
    {
        return onTick(dt);
    }
    void BaseController::stop() noexcept
    {
        onStop();
    }

    void BaseController::dispatch(ICommand& cmd) noexcept
    {
        // Use the new CommandBus sync API:
        // - The bus expects a shared_ptr<ICommand>. We wrap the reference into a
        //   non-owning shared_ptr with a no-op deleter to avoid lifetime issues.
        // - This keeps the BaseController public API stable while internally
        //   switching from 'dispatch(...)' to the new 'execute(...)' entry point.
        //
        // NOTE: If/when we add an async path in BaseController, we'll forward to
        //       CommandBus::submitAsync(...) similarly.

        m_commands.execute(std::shared_ptr<ICommand>(&cmd, [](ICommand*) {}), CommandContext{} // empty/default context
        );
    }
    CommandResult BaseController::execute(ICommand& cmd, const CommandContext& ctx) noexcept
    {
        // Wrap non-owning shared_ptr with no-op deleter to avoid copies.
        try
        {
            return m_commands.execute(std::shared_ptr<ICommand>(&cmd, [](ICommand*) {}), ctx);
        }
        catch (...)
        {
            // Keep noexcept guarantee at controller boundary.
            return CommandResult::Fail("BaseController::execute() threw");
        }
    }

    std::future<CommandResult>
    BaseController::submitAsync(ICommand& cmd, CommandContext ctx,
                                std::function<void(const CommandResult&)> completion) noexcept
    {
        try
        {
            return m_commands.submitAsync(std::shared_ptr<ICommand>(&cmd, [](ICommand*) {}), std::move(ctx),
                                          std::move(completion));
        }
        catch (...)
        {
            // Create a ready future with failure to keep noexcept.
            std::promise<CommandResult> p;
            p.set_value(CommandResult::Fail("BaseController::submitAsync() threw"));
            return p.get_future();
        }
    }
} // namespace viz::core

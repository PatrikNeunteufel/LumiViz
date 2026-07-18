/**
 ****************************************************************************************
 * @file   CompositeCommand.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <vector>
#include <memory>
#include <string>
#include "core/commandbus/ICommand.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"

namespace viz::core
{

    /**
     * @brief Composite of commands used for transactional grouping.
     *
     * Semantik:
     * - In Transaktionen werden Kinder *bereits vor* Hinzufügen ausgeführt (durch CommandBus::submit).
     * - execute(): No-Op (Kinder sind schon gelaufen), liefert Ok.
     * - undo(): Kinder in *Reverse-Reihenfolge* rückgängig.
     * - redo(): Kinder in *Vorwärts-Reihenfolge* erneut ausführen (Symmetrie/Bequemlichkeit).
     */
    class CompositeCommand final : public ICommand
    {
      public:
        explicit CompositeCommand(std::string label);

        const char* name() const noexcept override;
        void add(const ICommandPtr& c);

        CommandResult execute(const CommandContext& ctx) override;
        CommandResult undo(const CommandContext& ctx) override;

      private:
        std::string m_label;
        std::vector<ICommandPtr> m_children;
        bool m_executed{false};
    };

} // namespace viz::core

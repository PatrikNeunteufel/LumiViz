3# 📘 CommandBus Usage Guide

## 🔑 Überblick
Der **CommandBus** ist das zentrale Modul für Undo/Redo-Management im Projekt. Er führt `ICommand`-Objekte aus, verwaltet Undo-/Redo-Stacks und unterstützt Transaktionen, Coalescing, Adapters und EventBus-Bridges.

Struktur analog zu `ServiceContainer` und `EventBus`:
```
src/core/commandbus/
  ICommand.hpp
  CommandBus.hpp/.cpp
  CommandContext.hpp
  CommandResult.hpp
  CompositeCommand.hpp/.cpp
  TransactionGuard.hpp/.cpp
  TCommandAdapter.hpp/.tpp
  CoalescingPolicy.hpp/.cpp
```

---

## 🧩 Kernkonzepte

### ICommand
```cpp
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual const char* name() const noexcept = 0;
    virtual CommandResult execute(const CommandContext&) = 0;
    virtual CommandResult undo(const CommandContext&) = 0;
};
```

### CommandResult
- Ergebnisobjekt (`ok`, `errorCode`, `errorMessage`).
- `CommandResult::Ok()` / `CommandResult::Fail(msg, code)` als Factorys.

### CommandContext
- Enthält thread-/umgebungsspezifische Infos.
- Wird bei `execute`, `undo`, `redo` übergeben.

### CommandBus
- `submit(cmd, ctx)` → Command ausführen + auf Undo-Stack legen.
- `undo(ctx)` / `redo(ctx)` → Stacks navigieren.
- `beginTransaction(label)` / `commitTransaction()` / `rollbackTransaction(ctx)` → Makro-Commands.
- `undoSize()`, `redoSize()`, `topUndoName()`, … → Query-API.

### CompositeCommand
- Container für mehrere Kinder-Commands (Transaktionsgruppen).
- `undo` ruft Kinder rückwärts auf, `redo` vorwärts.

### TransactionGuard
- RAII-Wrapper für sichere Transaktionen.
```cpp
{
    TransactionGuard g(bus, "Move Layer");
    bus.submit(cmd1);
    bus.submit(cmd2);
    g.commit(); // bei vergessener commit() → rollback im Destruktor
}
```

### TCommandAdapter
- Template, um einfache Payloads + Lambdas in Commands zu verwandeln.
```cpp
struct Counter { int v{0}; };

using AddPayload = int;

auto add = std::make_shared<TCommandAdapter<AddPayload>>(
    "Add",
    5,
    [](const AddPayload& p, const CommandContext&) {
        c.v += p;
        return CommandResult::Ok();
    },
    [](const AddPayload& p, const CommandContext&) {
        c.v -= p;
        return CommandResult::Ok();
    }
);

bus.submit(add);
```

### CoalescingPolicy
- Policy für das Zusammenfassen ähnlicher Commands.
- Global über `bus.coalescing()` konfigurierbar.

### EventBus-Bridge
- CommandBus publiziert Events (StacksChanged, WillUndo, DidUndo, WillRedo, DidRedo).
- UI kann sich einklinken (z. B. Menü/Toolbar-Enable, History-Panel).

---

## 🛠️ Beispiele

### Einfaches Command
```cpp
class IncrementCommand : public ICommand {
public:
    IncrementCommand(int& target, int delta)
      : m_ref(target), m_delta(delta) {}

    const char* name() const noexcept override { return "Increment"; }
    CommandResult execute(const CommandContext&) override {
        m_ref += m_delta;
        return CommandResult::Ok();
    }
    CommandResult undo(const CommandContext&) override {
        m_ref -= m_delta;
        return CommandResult::Ok();
    }
private:
    int& m_ref;
    int m_delta;
};

int x = 0;
CommandBus bus;
bus.submit(std::make_shared<IncrementCommand>(x, 5));
```

### Undo / Redo
```cpp
bus.undo();  // x -= 5
bus.redo();  // x += 5
```

### Transaktion
```cpp
{
    TransactionGuard tx(bus, "Move Object");
    bus.submit(std::make_shared<IncrementCommand>(x, 5));
    bus.submit(std::make_shared<IncrementCommand>(x, 10));
    tx.commit();
}
// Beide Commands erscheinen als ein Eintrag im Undo-Stack
```

### Mit Coalescing
```cpp
bus.coalescing().set<IncrementCommand>([](const auto& a, const auto& b){
    return a.m_delta + b.m_delta;
});
```

---

## 🧪 Testing
- Unit-Tests unter `tests/core/commandbus/`
- Abgedeckt: Basic submit, Undo/Redo, Transaktionen, Adapter, Coalescing, EventBus-Bridge.

---

## 📑 Fazit
Der CommandBus ist ein **vollwertiges Undo/Redo-Subsystem** mit klarer Architektur, vorbereitet für Integration in ImGui-UI, Shortcut-System und Projekt-Snapshots.

**Best Practices:**
- Für kleine Einmal-Aktionen: `TCommandAdapter` verwenden.
- Für komplexe Operationen: Eigene `ICommand`-Implementierung.
- Für Gruppenaktionen: `TransactionGuard` nutzen.
- Immer Fehlercodes prüfen (`CommandResult`).


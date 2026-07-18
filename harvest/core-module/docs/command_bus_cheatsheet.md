# ⚡ CommandBus Cheatsheet

> Quick reference for daily use. Short, practical, consistent with ServiceContainer/EventBus.

---

## Core Types (where to look)
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

## Minimal API
```cpp
// Execute and push to undo stack
CommandResult submit(const ICommandPtr& cmd, const CommandContext& ctx = {});

// Undo / Redo
CommandResult undo(const CommandContext& ctx = {});
CommandResult redo(const CommandContext& ctx = {});

// Query
bool        canUndo() const;    // is undo available?
bool        canRedo() const;    // is redo available?
size_t      undoSize() const;   // entries in undo stack
size_t      redoSize() const;   // entries in redo stack
std::string topUndoName() const;
std::string topRedoName() const;

// Transactions (macro commands)
void beginTransaction(const std::string& label);
void commitTransaction();
void rollbackTransaction(const CommandContext& ctx = {});

// RAII helper
TransactionGuard(CommandBus& bus, std::string label);
void TransactionGuard::commit() noexcept; // otherwise dtor rolls back
```

---

## ICommand quick template
```cpp
class MyCommand : public ICommand {
public:
  MyCommand(Target& t, int v) : t_(t), v_(v) {}
  const char* name() const noexcept override { return "MyCommand"; }
  CommandResult execute(const CommandContext&) override { t_.apply(v_); return CommandResult::Ok(); }
  CommandResult undo(const CommandContext&) override    { t_.apply(-v_); return CommandResult::Ok(); }
private:
  Target& t_; int v_{};
};

// Use
MyTarget x; CommandBus bus; CommandContext ctx{};
bus.submit(std::make_shared<MyCommand>(x, +5), ctx);
```

---

## TCommandAdapter (lambdas → command)
```cpp
struct Counter { int v{0}; } c;
using Payload = int;
auto cmd = std::make_shared<TCommandAdapter<Payload>>(
  "Add",
  3,
  [&](const Payload& p, const CommandContext&){ c.v += p; return CommandResult::Ok(); },
  [&](const Payload& p, const CommandContext&){ c.v -= p; return CommandResult::Ok(); }
);

bus.submit(cmd);
```

---

## Transactions (group multiple commands)
```cpp
{
  TransactionGuard tx(bus, "Move Object");
  bus.submit(makeMoveX(...));
  bus.submit(makeMoveY(...));
  tx.commit(); // one undo entry
}
```

### Manual variant
```cpp
bus.beginTransaction("Paint Stroke");
for (auto& step : steps) bus.submit(step);
bus.commitTransaction();
```

---

## Coalescing (merge similar commands)
```cpp
// Define merge rule for a specific command type
bus.coalescing().set<MyCommand>([](const MyCommand& last, const MyCommand& incoming){
  // return true if merged (modify last to absorb incoming)
  return tryMerge(last, incoming);
});

// Reset or scope rules
bus.coalescing().clearAll();
// or
auto scope = bus.coalescing().scoped([](auto& pol){ pol.clearAll(); /* customize */ });
```

---

## EventBus Bridge (if wired)
- Emitted events (names may vary slightly in your build):
  - `CommandStacksChanged { undoSize, redoSize }`
  - `WillUndo { name }`, `DidUndo { name, ok }`
  - `WillRedo { name }`, `DidRedo { name, ok }`
  - `WillSubmit { name }`, `DidSubmit { name, ok }`

Use to:
- Enable/disable menu items
- Update History panel
- Telemetry/Logging

---

## CommandResult helpers
```cpp
// Constructors/factories
CommandResult::Ok();
CommandResult::Fail("reason", /*code=*/123);

// Typical pattern
auto r = bus.submit(cmd);
if (!r.ok) { /* show toast/log, prevent UI state */ }
```

---

## Common patterns
- **Idempotent undo/redo**: keep state consistent even when called repeatedly by tests.
- **Invalidate redo on submit**: any successful `submit()` clears redo.
- **Do small work in commands**: keep UI responsive; split large ops into atomic steps inside a transaction.
- **Prefer TransactionGuard**: protects against early returns/exceptions.

---

## Gotchas (avoid bugs)
- **Forgotten `commit()`** → your transaction will rollback in `~TransactionGuard()`.
- **Redo after new submit** → always cleared; don’t rely on previous redo entries.
- **Thread affinity** (if enabled in your context) → submit/undo/redo should run on UI thread.
- **CompositeCommand empty** → decide if allowed; usually ok but no-op.

---

## Testing snippets (Catch2)
```cpp
TEST_CASE("redo cleared after submit"){
  CommandBus bus; CommandContext ctx{}; test::Counter c{};
  REQUIRE(bus.submit(std::make_shared<test::AddCommand>(c,1), ctx).ok);
  REQUIRE(bus.undo(ctx).ok);
  REQUIRE(bus.redoSize()==1);
  REQUIRE(bus.submit(std::make_shared<test::AddCommand>(c,2), ctx).ok);
  REQUIRE(bus.redoSize()==0);
}
```

---

## Perf tips
- Reserve child count for big transactions (if API available).
- Avoid unnecessary heap by reusing command objects for rapid inputs (adapters help).
- Only wire EventBus bridge if UI listens (lazy subscribers check).

---

## Troubleshooting quick map
- **Linker errors** → ensure `CommandBus.cpp`, `CompositeCommand.cpp`, `TransactionGuard.cpp` are in CMake target.
- **Tests not discovered** → target links `Catch2::Catch2WithMain`, proper `add_test()` added.
- **Redo still present after submit** → verify logic clearing `redoStack_` upon successful `submit()`.

---

## Migration checklist (when adding new commands)
- [ ] Meaningful `name()` for UI/history.
- [ ] Correct inverse in `undo()`.
- [ ] No external side effects left unrolled (GPU/Audio API symmetry!).
- [ ] Optional: tag/StrongID for filtering and analytics.
- [ ] Unit test covering success + undo/redo + edge (double-undo).


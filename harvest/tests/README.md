# harvest/tests — Unit-Tests der viz::core-Generation

**Quelle:** `NewViz2025/tests/core/` (Catch2). **Ziel: Phase 3** — Test-Fundament für LumiViz/LumiViz.

## Inhalt

| Datei | Zeilen | Prüft |
|---|---|---|
| `newviz2025-core/eventbus/test_eventbus.cpp` | 94 | Publish/Subscribe, Scoped/Weak-Abos, Snapshot-Dispatch |
| `newviz2025-core/servicecontainer/test_servicecontainer.cpp` | 834 | Lifetimes (Singleton/Scoped/Transient), Zyklen-Erkennung, Threading, Fehlerfälle |
| `newviz2025-core/servicecontainer/howtoTest.md` | — | Anleitung, wie die Suite aufgebaut ist |
| `newviz2025-core/commandbus/test_commandbus_all.cpp` | 430 | Submit/Undo/Redo, Transaktionen, Coalescing, Adapter, EventBus-Bridge |
| `newviz2025-core/basetypes/test_Base*.cpp` | ~850 | BaseController (sync/async), BaseManager, BaseAgent, BaseRegistry |
| `eventbus_tests.txt` | — | Notizen/Log zu den EventBus-Tests |

## Wiederverwertung

- **Framework wechseln:** Catch2 → doctest (`TEST_CASE`/`CHECK` sind fast 1:1; `SECTION` → `SUBCASE`).
- **API-Abgleich nötig:** Der heutige LumiViz-EventBus (`IEventBus` mit Prioritäten + Queue) hat eine
  andere API als der getestete viz::core-EventBus (Scoped/Weak-Handles). Tests, deren Feature noch
  fehlt, dokumentieren das **Soll** — erst Feature nachrüsten (siehe harvest/core-module), dann Test portieren.
- **ServiceContainer:** LumiViz' ServiceContainer ist dem getesteten sehr ähnlich → die 834 Zeilen
  sind der schnellste Weg zu echter Abdeckung. Hier anfangen.
- Ziel-Ort: `projects/apps/LumiViz/tests/unit/` (Targets in Solution.json aktivieren, `skip: false`).

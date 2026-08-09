# harvest/core-module — viz::core-Module (NewViz2025-Generation)

**Quelle:** `NewViz2025/src/core/`. **Ziel: Phase 4** (CommandBus, EventBus-Upgrade) bzw. Referenz.

## Inhalt

| Ordner | Was es ist | Status ggü. heutigem LumiViz |
|---|---|---|
| `commandbus/` | **Vollständiges Undo/Redo-Subsystem**: ICommand, CommandBus (Stacks, Transaktionen, Coalescing), CompositeCommand, TransactionGuard (RAII), TCommandAdapter (Lambda-Commands), EventBus-Bridge | **Fehlt in LumiViz komplett.** Kandidat für Parameter-Änderungen, Gradient-Editor, Preset-Bearbeitung |
| `eventbus/` | EventBus mit **RAII-`SubscriberHandle`**, **Weak-Abos** (Lebenszeit an `shared_ptr` gebunden), **Snapshot-Dispatch** (reentranz-sicher), adressbasiertem Topic-Key (kein RTTI-Problem) | LumiViz-EventBus hat nur ID-basiertes Unsubscribe + Prioritäten/Queue. Weak/RAII nachrüsten = Panel-Lifetime-Fallen beseitigen |
| `servicecontainer/` | DI-Container: Singleton (call_once), Scoped, Transient, Eager-Build, Zyklen-Erkennung | LumiViz-Version sehr ähnlich — als Abgleich/Referenz |
| `basetypes/` | BaseController (sync/async), BaseManager, BaseAgent, BaseRegistry | Taxonomie-Vokabular (siehe harvest/konzepte/Taxiome.md); LumiViz nutzt eigene Basisklassen |
| `docs/` | Usage-Guides + Cheatsheets zu CommandBus, EventBus, ServiceContainer (die „übergeordneten" losen Dokus) | Beschreiben die **hiesige** API, nicht die LumiViz-API — bei Portierung mitziehen |

## Wiederverwertung

1. **EventBus-Upgrade zuerst** (klein, hoher Nutzen): `subscribeScoped`/`subscribeScopedWeak` +
   Snapshot-Dispatch in den LumiViz-EventBus übernehmen; API bleibt abwärtskompatibel erweiterbar.
2. **CommandBus als neues Modul** `projects/apps/LumiViz/.../services/commandbus/` einführen, wenn
   die Config-Pipeline (Phase 4) Undo/Redo für Parameter braucht — nicht vorher einbauen.
3. Zu jedem übernommenen Modul: Tests aus harvest/tests mitportieren, Guides aus `docs/` anpassen.

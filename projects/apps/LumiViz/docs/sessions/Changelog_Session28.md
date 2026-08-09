# Changelog — Session 28 (2026-07-17/18)

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Changelog
> **Status:** Abgeschlossen

> **Thema:** Repo-Reorganisation (NewViz → LumiViz, Build-System → CMakeCraft) —
> hier nur die **Produkt**-Änderungen an LumiViz; Infrastruktur siehe Commit-Historie.

## Neue Features

### 🧪 Unit-Test-Fundament (`LumiViz.UnitTests`)

Erstes aktives Test-Target — **36 Cases / 221 Assertions** (doctest, via ctest oder Exe):

| Suite | Abdeckung |
|---|---|
| ServiceContainer | Singleton/Factory/Instance/Transient, Fehlerfälle, Verwaltung |
| EventBus | Publish/Subscribe, Prioritäten, consume(), Queue/dispatchQueued |
| ColorGradientModule | Sampling, Stop-Invarianten, Midpoints, **JSON-Roundtrip**, Presets |
| Playlist | Verwaltung, Navigation (Wrap), Shuffle, M3U-Roundtrip, Events |
| VisualizerPresetManager | Save/Load/Rename/Delete/Overwrite, Typ-Vertrag |

### 🎨 `ColorGradientModule::fromJson()` implementiert

War seit Januar ein TODO-Stub (`return false`) — Gradient-Konfigurationen waren nie aus
JSON wiederherstellbar. Jetzt vollständiger Roundtrip (inkl. Midpoints), verifiziert bis
zur Sampling-Äquivalenz.

## Fixes

- **ServiceContainer:** Factory-Deadlock behoben — `resolve()` innerhalb einer
  Registrierungs-Factory (dokumentiertes Muster!) verklemmte den nicht-rekursiven Mutex;
  jetzt `std::recursive_mutex`.

## Dokumentierte bekannte Punkte (Fix in Phase 4 vorgemerkt)

- `resolve()/tryResolve()` auf **Transient**-Services liefert einen Dangling Pointer —
  nur `createTransient()` ist sicher (geskippter Test dokumentiert das).
- `Event::consume()` ist nicht-const, Handler erhalten aber `const&` → const_cast nötig.
- PresetManager-Vertrag: JSON-Zahlen kommen als **float** zurück (auch für int-Parameter).

## Doku

- Neuer Einstieg `docs/INDEX.md`; Produkt-Changelogs unter `docs/sessions/`;
  Versionskopien entfernt (Git ist die Historie).
- Phase-4-Pflichtenheft: `harvest/config-pipeline/README.md` (Config-UI folgt der
  Pipeline-Reihenfolge: Audio-Analyse → Mapping → Farbe → Rendering → Peak/Partikel → Post).

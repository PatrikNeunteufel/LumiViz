# ScriptContext — geteilter Preset-Skript-Zustand

> **Version:** 1.0.0
> **Datum:** 2026-07-20
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Import-Phase Roadmap 4.1)
> **Modul:** lumi::scripting::ScriptContext
> **Dateien:** ScriptContext.hpp (header-only)
> **Namespace:** lumi::scripting
> **Abhängigkeiten:** keine
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Preset-lokaler Zustand, den sich **alle Skript-Träger eines Presets** teilen
(Entwurf [Import_Fundament_Entwurf.md](../../docs/visuals/Import_Fundament_Entwurf.md) §1,
Entscheid Import-Analyse §10.3): `reg00–99`, `q1–q64` (MD3-Superset, Entscheid E5)
und `gmegabuf`. Engines erhalten ihn als `shared_ptr` — ohne expliziten Kontext
bekommt jede Engine einen privaten (Alt-Verhalten, Engines isoliert).

## 2. API-Kern

- `reg(i)` / `setReg(i, v)` — 0-basiert, außerhalb 0..99 wirkungslos/0.
- `q(i)` / `setQ(i, v)` — **1-basiert** wie die Skript-Namen (q1 → 1).
- Snapshots (MilkDrop-Datenfluss): `captureInitSnapshot()` nach den Init-Läufen,
  `restoreInitSnapshot()` am Frame-Beginn, `captureFrameSnapshot()` nach dem
  Frame-Lauf, `restoreFrameSnapshot()` vor Punkt-Batches. AVS-Hosts nutzen die
  q-API schlicht nicht.
- `gmbRead/gmbWrite` — gmegabuf-Speicher; Index-Klemmung (floor+1e-4, Kapazität)
  macht weiterhin die Engine-Closure.
- `reset()` — alles auf 0/leer.

## 3. Threading

Ein Kontext gehört genau **einem** Render-Thread; alle Skript-Träger eines
Presets laufen dort sequenziell → kein Locking (Visualizer_Architecture §12).
Die 32 app-globalen Atomic-Slots (`app.gget/gset`) sind davon getrennt und
bleiben in der LuaScriptEngine.

## 4. Sync-Modell

reg/q liegen im Kontext, Skripte sehen sie als gewöhnliche Env-Variablen —
den Abgleich an den Slot-Grenzen macht der
[ScriptSlotHost](ScriptSlotHost.md) über beim Kompilieren erhobene
Sync-Listen (Hot-Path bleibt frei, wenn ein Slot keine reg/q erwähnt).
**Bewusste Abweichung vom Entwurfstext:** der PRNG bleibt engine-lokal
(deterministisches Seeding je Engine; ein geteilter PRNG machte die
rand()-Folgen reihenfolgeabhängig zwischen Modulen).

## 5. Tests

`tests/unit/UnitTests/test_ScriptContext.cpp` — Grenzen/Defaults, Snapshots,
gmegabuf-Teilung zweier Engines, Isolation ohne expliziten Kontext,
reg/q-Sync über ScriptSlotHosts, Mehr-Frame-Ablauf zweier Träger.

## 6. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — Roadmap 4.1 (Session 33) |

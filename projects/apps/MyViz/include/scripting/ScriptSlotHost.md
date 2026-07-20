# ScriptSlotHost — EEL-Slot-Quartett als gemeinsamer Baustein

> **Version:** 1.0.0
> **Datum:** 2026-07-20
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Import-Phase Roadmap 4.1)
> **Modul:** lumi::scripting::ScriptSlotHost
> **Dateien:** ScriptSlotHost.hpp, ScriptSlotHost.cpp
> **Namespace:** lumi::scripting
> **Abhängigkeiten:** LuaScriptEngine, ScriptContext, EelTranspiler (Lib)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Extraktion des wiederkehrenden Musters aus dem SuperscopeModule
(Entwurf §1, Entscheid E2): **vier EEL-Slots → Transpile → compile → run mit
Fehlerzustand und Fallback**. Nutzer: SuperscopeModule (seit 4.1), die
kommenden ScriptGrid-/ScriptLut-Module (4.2). Ein Host = ein skriptfähiges
Modul; mehrere Hosts eines Presets teilen sich einen
[ScriptContext](ScriptContext.md).

## 2. Vertrag

- Quellen sind **EEL** (Dialekt im Konstruktor, Default Avs — nie rohes Lua
  in die Slots, Merkregel Session 32). `compileAll()` transpiliert alle vier
  Slots; Transpile-/Compile-Fehler ⇒ Slot bleibt leer (AVS-Verhalten), der
  **erste** Fehler steht in `lastError()`, übrige Slots kompilieren weiter.
- `run(slot)` synchronisiert `regNN`/`qN` mit dem Kontext (Pull vor, Push nach
  dem Lauf) anhand von Sync-Listen, die beim Kompilieren aus dem erzeugten
  Lua erhoben werden — Slots ohne reg/q syncen nichts (Point-Hot-Path).
- `sourceMentions(slot, wort)` — wortgenaue, case-insensitive Suche in der
  EEL-Quelle (ersetzt das frühere Substring-`find` des Superscope:
  `"shredder"` triggert kein `red` mehr, `RED` sehr wohl — EEL ist
  case-insensitiv).
- Host-Inputs/Outputs (i, v, b, n, w, h, time, dt / x, y, skip, red, green,
  blue) setzt weiterhin der **Besitzer** über `engine()` — der Host kennt
  keine Domäne.

## 3. Tests

`test_ScriptContext.cpp` (Host-Teile) + die bestehenden Superscope-/
LuaScriptEngine-/EelTranspiler-Suiten, die nach dem Umzug unverändert grün
sind (Akzeptanzkriterium 4.1).

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — Extraktion aus SuperscopeModule, reg/q-Sync (Session 33) |

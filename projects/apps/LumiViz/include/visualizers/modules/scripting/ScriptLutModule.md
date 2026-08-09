# ScriptLutModule — skriptbare Farb-LUT (Color Modifier)

> **Version:** 1.0.0
> **Datum:** 2026-07-20
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Import-Phase Roadmap 4.2 — CPU-Kern, 1D-Textur-Upload folgt später)
> **Modul:** lumi::modules::ScriptLutModule
> **Dateien:** scripting/ScriptLutModule.hpp, ScriptLutModule.cpp
> **Namespace:** lumi::modules
> **Abhängigkeiten:** ScriptSlotHost, ScriptContext (kein GL, kein Qt)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Die Skript-Laufform „**pro LUT-Eintrag**" (Entwurf §2.2): Semantik des AVS
Color Modifier (ref r_dcolormod.cpp). Das **Level**-Skript läuft je einem der
256 Einträge mit `red = green = blue = i/255` als Input und schreibt die neuen
Kanalwerte (geklemmt 0..1) in drei Tabellen — später eine 1D-LUT-Textur des
Farb-Post-Passes. Ohne bzw. mit fehlerhaftem Level-Skript: **Identitäts-LUT**.

## 2. Vertrag

- Slots: Init / Beat / Frame / **Level** (Level liegt auf dem Point-Slot des
  [ScriptSlotHost](../../../scripting/ScriptSlotHost.md), Chunk-Name
  `scriptlut.point`).
- `recompute=false` (Default): LUT wird einmal nach dem Kompilieren gebaut;
  `true`: jede `execute()` nach Beat-/Frame-Lauf neu (skriptete Animation).
- Frame-Inputs: `time, dt, b`; geteilter
  [ScriptContext](../../../scripting/ScriptContext.md) (reg/q/gmegabuf).
- Zugriff: `lut(channel, index)` (geklemmt) bzw. `tables()`.

## 3. Tests

`test_ScriptModules.cpp`: Identität, Invert, Kanal-Unabhängigkeit + Klemmen,
recompute-Verhalten (einmalig vs. je Frame), Beat-Einfluss, geteilter Kontext.

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — Roadmap 4.2 CPU-Kern (Session 33) |

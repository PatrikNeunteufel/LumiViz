# ScriptGridModule — skriptbares Gitterfeld (Movement/per_vertex)

> **Version:** 1.0.0
> **Datum:** 2026-07-20
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Import-Phase Roadmap 4.2 — CPU-Kern, GL-Anbindung folgt mit 4.3/5)
> **Modul:** lumi::modules::ScriptGridModule
> **Dateien:** scripting/ScriptGridModule.hpp, ScriptGridModule.cpp
> **Namespace:** lumi::modules
> **Abhängigkeiten:** ScriptSlotHost, ScriptContext (kein GL, kein Qt)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Die Skript-Laufform „**pro Gitterknoten**" (Entwurf §2.1): Ziel-Semantik von AVS
Movement/Dynamic Movement und MilkDrop per_vertex. `execute()` füllt ein
Verschiebungsfeld (`GridNode {u, v, alpha}` je Knoten, row-major) — die
Quellkoordinate, aus der ein späterer Render-Pass das Bild je Mesh-Knoten
abtastet. Ohne bzw. mit fehlerhaftem Point-Skript: **Identitätsfeld** (Bild
unverändert).

## 2. Knoten-Vertrag

- **Inputs je Knoten:** `x, y` (−1..1), `d` (Abstand vom Zentrum,
  `sqrt(x²+y²)`), `r` (Winkel, `atan2(y,x)`); je Frame: `w, h, time, dt, b`.
  **`t` schreibt der Host nie** (Skript-Eigentum, wie Superscope).
- **Outputs:** Polar-Modus (Default, AVS-treu): `d, r` → `u=cos(r)·d`,
  `v=sin(r)·d`. Rect-Modus (`setRectCoords(true)`): `x, y` direkt.
  `alpha` (0..1) nur, wenn das Point-Skript es erwähnt (sonst 1).
- Slots: Init (einmal), Beat (bei Beat), Frame (je execute), Point (je Knoten) —
  EEL via [ScriptSlotHost](../../../scripting/ScriptSlotHost.md); geteilter
  [ScriptContext](../../../scripting/ScriptContext.md) verbindet reg/q/gmegabuf
  mit den übrigen Skript-Trägern des Presets.

## 3. Grenzen

Gitter 2..96 × 2..72 (Default 32×24 = MilkDrop-Mesh). Laufzeitfehler
deaktivieren den Slot (Engine-Verhalten) → Feld fällt auf Identität zurück,
`lastScriptError()` gesetzt.

## 4. Tests

`test_ScriptModules.cpp`: Identität (ohne Skript, mit `x=x;y=y`), Zoom
(rect + polar `d=d*0.5`), Rotation (`r=r+$PI/2`), alpha-Erwähnung,
Frame-Variablen über mehrere execute(), geteilter Kontext (reg), Syntaxfehler.

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — Roadmap 4.2 CPU-Kern (Session 33) |

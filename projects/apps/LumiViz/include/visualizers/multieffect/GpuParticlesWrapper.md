# GpuParticlesWrapper — Shader-Bausteine des GPU-Partikel-Nodes

> **Version:** 1.0.0
> **Datum:** 2026-08-06
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Strang G2, Session 69)
> **Modul:** lumi::gpuparticles (freie Funktionen)
> **Dateien:** GpuParticlesWrapper.hpp (header-only)
> **Namespace:** lumi::gpuparticles
> **Abhängigkeiten:** keine (kein GL, kein Qt — Text/Zahlen, testbar)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Shader-Bausteine des **GPU-Partikel-Nodes** (`GpuParticlesParams`, Plan §G2):
die moderne Antwort auf die searchlight-Klasse — zehntausende Partikel per
Instancing statt 800×16-gmegabuf-Schleifen. RENDER-Modul (zeichnet aufs
Chain-Bild), modernes Regelwerk; Audio ad-hoc als Uniforms (Entscheid Patrik
S69: Strang G vor Vereinheitlichung V2).

**Architektur:** Zustand (pos.xy + vel.xy) = EIN RGBA32F-Texel je Partikel in
einem Ping-Pong-Texturpaar (Breite fix `kStateWidth`=256, Höhe =
`stateRows(count)`, max. 65536 Partikel). Ein Fragment-Pass integriert je
Frame (Schwerkraft + Drag + optionales Nutzer-Kraftfeld), ein instanzierter
Quad-Draw zeichnet (texelFetch im Vertex-Shader — VTF, GL 3.3 core; Ecke =
`aPos` des geteilten Fullscreen-Quads).

**Alter ohne Speicher (deterministisch):** feste hash-basierte Phase und
Lebensdauer je Partikel; `alter01 = fract(t/leben + phase)`. Der Update-Pass
erkennt den Zyklus-Wechsel per floor()-Vergleich mit dem Vorframe (uDelta)
und setzt das Partikel frisch an den Spawn — Richtung/Streuung je Zyklus neu
aus dem Hash. Kein rand(), keine CPU-Arbeit pro Partikel; mit der Sim-Uhr
voll reproduzierbar.

## 2. API-Kern

- `commonPrelude()` — GEMEINSAMES Prälude beider Pässe (Uniforms, Wang-Hash,
  Lebenslauf-Formeln). Beide Pässe müssen dieselben Formeln sehen, sonst
  springt der Respawn zu einem anderen Zeitpunkt, als der Draw ausblendet
  (testerzwungen).
- `updateFragment(userForce)` — Integrations-Pass; injiziert das optionale
  Kraftfeld hinter `#line 1` (Treiberfehler = Nutzer-Zeilen). Vertrag:
  `vec2 kraft(vec2 pos, vec2 vel, float alter)` = Zusatz-Beschleunigung in
  UV/s²; leer = Null-Kraft. `uReset` initialisiert (Erststart/Anzahl-Wechsel).
- `renderVertex()` / `renderFragment()` — instanzierter Sprite-Draw: Größe
  `uSize→uSize·uSizeEnd` übers Alter, Farbverlauf `uColorStart→uColorEnd`,
  weicher Kreis + Ausblenden am Lebensende; Instanzen ≥ uCount wandern aus
  dem Clip-Raum.
- `starterForce()` — Palette-Vorbelegung: Treble-Wirbel + Bass-Auftrieb.
- Klemmen `kMinCount`/`kMaxCount`, Layout `kStateWidth`/`stateRows()` — SSOT
  für Serializer-Leser, Panel und Renderer.

## 3. Render-Pfad (MultiEffectVisualizer::runGpuParticles)

Programm-Rebuild nur bei Kraftfeld-Wechsel (Snapshot `gpCompiled`), Zustands-
FBOs nur bei Zeilen-Wechsel (Anzahl ⇒ frischer Start). Update: Partner-FBO
beschreiben, aktuellen lesen, Swap. Render: `bindActive()`, Blend additiv
(`SRC_ALPHA, ONE`) oder Alpha, `glDrawArraysInstanced` über den geteilten
Quad-VAO, danach Blend aus. Kompilierfehler im geteilten `stError` →
`shadertoyError(nodeId)` (Panel-Anzeige + Apply-Poll). Parameter-Skript
(Strang D): `spawnx spawny spread speed dir fan gravx gravy drag life size`.

## 4. Tests

`tests/unit/UnitTests/test_GpuParticlesWrapper.cpp` — 7 Cases: #line-/
Injektions-Vertrag, gemeinsames Prälude beider Pässe, Instanz-Fetch,
Starter, stateRows, Serializer-Roundtrip aller 21 Felder, Leser-Klemmen.
Sichtbeweis: `asset/effectchain/gpuparticles_sonde.lvfx` (8192er-Fontäne
mit Wirbel-Kraftfeld + Fadeout-Trails, `out/gpuparticles_sonde_s69/`,
Warnungen=0).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-08-06 | Erstfassung — Strang G2 (Session 69) |

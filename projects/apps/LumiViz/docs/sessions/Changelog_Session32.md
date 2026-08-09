# Changelog Session 32 — Import-Phase-Start: Lua-Skripting + EEL-Transpiler

> **Datum:** 2026-07-20
> **Typ:** Produkt-Changelog
> **Sprache:** Deutsch

## Neu

- **Import-Analyse AVS/MilkDrop**
  ([Import_Analyse_AVS_MilkDrop.md](../visuals/Import_Analyse_AVS_MilkDrop.md)):
  Analyse der Referenz-Repos (Original-Winamp-AVS, projectM, MilkDrop3/2077,
  EEL-Dialekte) als Grundlage der Import-Phase — Lizenzmatrix, Zielbild
  „Übersetzen statt Emulieren", AVS-Effekt-Inventar, EEL-Semantik-Vertrag,
  Architektur-Lücken, 6-Punkte-Roadmap und die fünf Grundsatzentscheidungen
  (u. a. **Lua 5.4 festgeschrieben**, reg/gmegabuf preset-lokal + 32 app-globale
  Atomic-Register).
- **Lua-Skript-Engine**
  ([LuaScriptEngine.md](../../include/scripting/LuaScriptEngine.md)):
  sandboxed Lua 5.4 je Visualizer-Instanz — Whitelist-Umgebung (kein io/os/…,
  unbekannte Variablen lesen 0.0), AVS-4-Slot-Modell (Init/Beat/Frame/Point),
  **eel-Prelude** mit EEL-treuer Semantik (ε-Vergleiche, Integer-Modulo,
  sqrt(|x|), megabuf), deterministisch seedbarer PRNG, app-globales
  Atomic-Register-Set (`app.gget/gset`). lua54 wird dynamisch gelinkt und
  deployt (Solution.json).
- **EEL→Lua-Transpiler** als eigene Lib
  ([EelTranspiler.md](../../../../libs/EelTranspiler/include/EelTranspiler.md),
  header-only, Qt-frei): Lexer (alle Hex-Formate, $PI/$E/$PHI) → Pratt-Parser
  (AVS- **und** MilkDrop-Syntax: `if()`-Stil wie Infix `== != && || ?:`,
  Kompound-Zuweisungen, `buf[i]`/`gmem[i]`) → Lua-Codegen (Hoisting, lazy
  Kurzschluss-Konstrukte, loop-Caps 4096/2²⁰, 8-Zeichen-Aliasing-Warnung).
  25 Golden-Tests prüfen end-to-end gegen die EEL-Semantik.
- **Superscope: Skript-Modus** (Parameter „Lua Script", `render.script.lua`):
  Die vier Code-Slots (Init/Beat/Frame/Point) werden als **EEL** transpiliert
  und sandboxed ausgeführt — die eingebauten Presets laufen damit als echte
  Skripte (inkl. Beat-Randomisierung bei Lissajous/Flower/Starburst/Hypocycloid);
  Point-Skripte können per `red/green/blue` selbst färben. Ohne bzw. mit
  fehlerhaftem Skript rendert weiterhin die eingebaute Mathematik.
  **Gemessen: 1000 Punkte ≈ 0,3–0,6 ms/Frame** (~3 % des 60-fps-Budgets).

## Gefixt

- **Skript-Zeitbasis:** Der Host überschrieb die Skript-Variable `t` jeden Frame
  mit der Host-Zeit — Presets mit eigener t-Akkumulation (Spirale, Circle, …)
  liefen im Skript-Modus ~67× zu schnell. Jetzt gehört `t` dem Skript; der Host
  liefert `time` (absolut) und `dt`.
- **DNA-Preset im Skript-Modus:** Der Code-String war nur eine grobe Näherung
  der nativen Implementierung — die Slots sind jetzt leer, DNA rendert im
  Skript-Modus unverändert nativ.
- **Lissajous/Hypocycloid im Skript-Modus** (Sichtprüfung Patrik): Lissajous
  nutzte `b` als Frequenz — das ist die Host-Beat-Variable (wird jeden Frame
  überschrieben) → flache Linie; Hypocycloid nutzte `R` und `r` — EEL/Lua sind
  case-insensitiv, beide waren dieselbe Variable → Punktkollaps. Variablen
  umbenannt (`fa/fb`, `br/sr`); reservierte Host-Namen sind jetzt im
  Skript-Vertrag dokumentiert.

## Tests

LumiViz.UnitTests: **91 → 135 Cases** (0 Skips) — neu: Sandbox-/Prelude-Golden-Tests,
Superscope-Skript-Verhalten inkl. Regressionen (t-Besitz, DNA-Fallback,
Fehler-Fallback), EEL→Lua-Golden-Suite, Performance-Messung.

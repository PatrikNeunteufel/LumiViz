# EelTranspiler — EEL → Lua 5.4 (Import-Zeit-Übersetzer)

> **Version:** 1.0.0  
> **Datum:** 2026-07-20  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 2 — AVS-Dialekt + MilkDrop-Kern)  
> **Modul:** lumi::eel (Lib **EelTranspiler**, header-only INTERFACE)  
> **Dateien:** EelTranspiler.hpp (API), EelTranspilerLexer.hpp, EelTranspilerParser.hpp, EelTranspilerCodeGen.hpp  
> **Namespace:** lumi::eel (detail: lumi::eel::detail)  
> **Abhängigkeiten:** keine (kein Qt, keine App-Teile) — Ziel-Runtime ist die LuaScriptEngine-Sandbox  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [API](#2-api)
3. [Dialekte](#3-dialekte)
4. [Übersetzungsregeln](#4-übersetzungsregeln)
5. [Fehlerphilosophie und Warnungen](#5-fehlerphilosophie-und-warnungen)
6. [Bewusste Abweichungen](#6-bewusste-abweichungen)
7. [Tests](#7-tests)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Einmalige Übersetzung von EEL-Skripten (AVS-Superscope-Slots, MilkDrop
per_frame/per_vertex) in Lua-Quelltext für die
[LuaScriptEngine](../../../apps/MyViz/include/scripting/LuaScriptEngine.md)-Sandbox —
zur Laufzeit existiert kein EEL (Zielbild Import-Analyse §4). Aufbau klassisch
dreistufig (§7.4): Lexer → Pratt-Parser (Superset-Grammatik, ~12 Präzedenzstufen)
→ Codegen (AST → Lua-String, hoisting-basiert).

## 2. API

```cpp
#include <EelTranspiler.hpp>

auto result = lumi::eel::transpile(eelQuelle, lumi::eel::Dialect::Avs);
if (result.ok)      { /* result.lua -> LuaScriptEngine::compile(...) */ }
else                { /* result.error: "Zeile N: ..." — Slot gilt als leer */ }
for (auto& w : result.warnings) { /* Import-Report */ }
```

Leere/Whitespace-Quelle → `ok=true` mit leerem `lua`. `transpile` wirft nie.

## 3. Dialekte

| | `Dialect::Avs` | `Dialect::Milkdrop` |
|---|---|---|
| loop/while-Cap | 4096 | 1 048 576 |
| Hex-Formate | `3Bh`, `0x3B` | zusätzlich `$x3B`, `$'c'` (alle Formate werden immer akzeptiert) |
| 8-Zeichen-Aliasing-Warnung | ja (§10.2) | nein |
| Infix `== != <= >= && \|\| ?: ^`, Kompound-Zuweisung, `buf[i]`/`gmem[i]` | akzeptiert (Superset-Leniency) | dialekt-typisch |

## 4. Übersetzungsregeln

- **Zuweisung als Expression** → Hoisting in Auswertungsreihenfolge
  (`x=(y=5)*2` → `y = 5.0` · `x = (y * 2.0)`); in lazy Kontexten nie über die
  Zweiggrenze — dort Statement-Form mit Env-Temp `__eel_tN`.
- **Lazy:** `if(c,a,b)` / `?:` mit puren Zweigen → `(eel.truthy(c) and (a) or (b))`
  (EEL-Werte sind Zahlen — in Lua immer truthy, kein and/or-Problem); mit
  Nebenwirkungen → `if ... then ... else ... end` + Temp. `&&`/`||` kurzschließend,
  Ergebnis 1.0/0.0. `band()`/`bor()` bleiben **eager**.
- **Zahlwerk:** Literale in Float-Form (`5` → `5.0`); `%` → `eel.mod`,
  `&`/`|` → `eel.bitand/bitor`, `sqrt` → `eel.sqrt`, Vergleiche → 1.0/0.0
  (`==` → `eel.equal` mit ε=1e-5).
- **Buffers:** `megabuf(i)` → `eel.mbread/mbwrite`; `buf[i]` → `mbread((buf)+(i))`;
  `gmem[i]`/`gmegabuf(i)` → `eel.gmbread/gmbwrite` (alles preset-lokal, §10.3).
- **Kontrollfluss:** `loop(n, body)` → `for` mit Cap; `while(body)` → `for`-Schleife
  mit Abbruch bei untruthy; `exec2/exec3` → Statement-Sequenz.
- **Identifier:** lowercased (EEL case-insensitiv); Lua-Reserviertwörter → Suffix
  `_` (`end` → `end_`); Temps `__eel_tN` sind Env-Variablen (kein 200-Locals-Limit).
- **Host-Funktionen:** `getosc/getspec/gettime` werden durchgereicht (Sandbox-Env
  liefert sie); `getkbmouse/setmousepos/freembuf/memcpy/memset` → `0.0`-Stub + Warnung.

## 5. Fehlerphilosophie und Warnungen

Syntaxfehler → `ok=false` mit Positionsangabe; der Aufrufer behandelt den Slot
als leer (AVS-Verhalten: nicht kompilierbar = tut nichts) und meldet es im
Import-Report. Warnungen (kein Abbruch): 8-Zeichen-Aliasing-Verdacht (AVS),
unbekannte Funktionen (durchgereicht — Host kann sie stellen), Stub-Ersetzungen.

## 6. Bewusste Abweichungen

1. **Konventionelle Präzedenz-Schichtung** statt der historischen EEL2-Kuriosa
   (dort bindet `-` stärker als `+` usw.); relevant nur für klammerlose
   Exoten-Ausdrücke. Beibehalten: `^` rechtsassoziativ, unäres Minus bindet
   stärker als `^` (`-2^2 = 4`).
2. **Zuweisungen als Expression auch im AVS-Dialekt** (Superset-Leniency —
   Original-AVS-Presets enthalten sie nie).
3. **8-Zeichen-Aliasing wird nicht emuliert**, nur gewarnt (Entscheid §10.2).
4. `rand(x)` → Integer-Semantik beider Dialekte (Entscheid §10.1, Engine-seitig).

## 7. Tests

Golden-Suite `tests/unit/UnitTests/test_EelTranspiler.cpp` (25 Cases) prüft
**end-to-end**: EEL-Quelle → transpile → Ausführung in der LuaScriptEngine →
Zahlenvergleich gegen die Semantik-Goldens der Import-Analyse §7.2
(ε-Vergleiche, Integer-`%`, `sqrt(|x|)`, if-Laziness, Hoisting, loop-Caps,
megabuf-Adressierung, Dialekt-Formate).

## 8. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-20 | Erstfassung — Lexer/Parser/Codegen, AVS-Dialekt komplett + MilkDrop-Kern (Infix, ?:, Kompound, buf[]/gmem[]), Golden-Suite (Session 32) |

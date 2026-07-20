# LuaScriptEngine — Sandboxed Lua 5.4 für Visualizer-Skripte

> **Version:** 1.1.0  
> **Datum:** 2026-07-20  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 1 — Keimzelle Superscope)  
> **Modul:** lumi::scripting::LuaScriptEngine  
> **Dateien:** LuaScriptEngine.hpp, LuaScriptEngine.cpp  
> **Namespace:** lumi::scripting  
> **Abhängigkeiten:** lua54 (dynamisch, `externals/lua54`)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Sandbox-Vertrag](#2-sandbox-vertrag)
3. [API](#3-api)
4. [Skript-Umgebung](#4-skript-umgebung)
5. [Threading und Fehlerverhalten](#5-threading-und-fehlerverhalten)
6. [Performance](#6-performance)
7. [Verwendung (Superscope)](#7-verwendung-superscope)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Eine Engine = eine sandboxed Skript-Umgebung für **eine** Preset-/Visualizer-Instanz.
Sie hält einen privaten `lua_State` plus eine whitelisted Environment-Tabelle (`_ENV`),
die sich bis zu vier Skript-Slots teilen — das AVS-Ausführungsmodell:

| Slot | Läuft |
|---|---|
| `Init` | einmal nach (Re-)Compile bzw. Reset |
| `Beat` | bei erkanntem Beat |
| `Frame` | einmal pro Frame |
| `Point` | pro Punkt (Hot-Path) |

Grundlage: [Import_Analyse_AVS_MilkDrop.md](../../docs/visuals/Import_Analyse_AVS_MilkDrop.md)
§7 (EEL→Lua) und §10 (Entscheidungen). Die Engine ist die Laufzeit-Basis für den
späteren EEL→Lua-Transpiler (Roadmap 2) — heute führt sie handgeschriebene
Lua-Skripte aus (Superscope).

## 2. Sandbox-Vertrag

- Chunks laden **nur im Text-Modus** (`"t"`) mit eigenem `_ENV` — Skripte sehen `_G` nie.
- Whitelist statt Blacklist: kein `io`, `os`, `debug`, `load`, `error`, `type`,
  keine Metatable-Funktionen, keine Koroutinen — nicht vorhandene Namen lesen als `0.0`.
- **Unbekannte Variablen lesen 0.0** (EEL-Semantik, via `__index`).
- `rand` ist ein eigener, **deterministisch seedbarer** PRNG (`seedRandom`) —
  `math.random` wird nicht durchgereicht.
- `reg00..reg99`/`q1..q64` sind einfache Env-Variablen; der
  [ScriptSlotHost](ScriptSlotHost.md) synchronisiert sie an Slot-Grenzen mit dem
  geteilten [ScriptContext](ScriptContext.md) (preset-lokal, Entscheid §10.3).
  `megabuf` ist engine-lokal; **`gmegabuf` lebt im ScriptContext** — Engines mit
  demselben Kontext teilen ihn (Roadmap 4.1). Ohne expliziten Kontext erhält
  jede Engine einen privaten (Alt-Verhalten).
- App-globales Register-Set: `app.gget(i)`/`app.gset(i, v)` — 32 Slots als
  `std::atomic<double>` prozessweit; jeder Slot einzeln atomar, keine
  Transaktions-Garantie über mehrere Slots (Entscheid §10.3).

## 3. API

| Methode | Zweck |
|---|---|
| `compile(slot, source, chunkName)` | Quelle in Slot übersetzen; leere Quelle räumt den Slot; `false` → Slot deaktiviert, `lastError()` gesetzt |
| `run(slot)` | Slot ausführen; Laufzeitfehler deaktiviert den Slot (kein Fehler-Spam) |
| `has(slot)` / `clear(slot)` | Slot-Status / Slot räumen |
| `setNumber(name, v)` / `number(name)` | Env-Variablen (Zahlen — das Skript-Datenmodell) |
| `evalNumber(expr, out)` | Ausdruck in der Sandbox auswerten (Tests/Diagnose) |
| `seedRandom(seed)` | PRNG deterministisch seeden (Default: MilkDrop-Seed `0x4141f00d`) |
| `lastError()` / `clearError()` | letzter Compile-/Laufzeitfehler |

## 4. Skript-Umgebung

**Unqualifizierte Math-Teilmenge:** `sin cos tan asin acos atan atan2 sqrt abs floor
ceil exp log min max fmod mod huge` (`mod` = `math.fmod`), Konstanten `pi`, `pi2`,
dazu `rand(x)` (Integer 0…x−1, Entscheid §10.1).

**`eel`-Prelude** (EEL-treue Semantik für transpilierten Code, Import-Analyse §7.2):

| Funktion | Semantik |
|---|---|
| `eel.truthy(x)` | Lua-boolean, wahr ⇔ \|x\| > 1e-5 |
| `eel.equal/above/below(a,b)` | 1.0/0.0; equal mit ε=1e-5 |
| `eel.band/bor/bnot(...)` | logisch, **eager**, 1.0/0.0 |
| `eel.bitand/bitor(a,b)` | 64-bit-Runden, bitweise |
| `eel.mod(a,b)` | Integer-Runden, Divisor 0 → 0, Ergebnis \|Rest\| |
| `eel.toint(x)` | round-to-nearest |
| `eel.sqrt(x)` | `sqrt(\|x\|)` — nie NaN |
| `eel.invsqrt/sigmoid/sign` | wie EEL |
| `eel.rand(x)` | wie `rand` |
| `eel.mbread/mbwrite(i[,v])` | megabuf: floor(i+1e-4), Clamp [0, 8 M), Default 0 |
| `eel.gmbread/gmbwrite(i[,v])` | gmegabuf (engine-lokal = preset-lokal) |

Lazy-Konstrukte (`if(c,a,b)`, `&&`, `||`) sind **keine** Prelude-Funktionen —
sie werden vom Transpiler als Lua-Kontrollfluss erzeugt (Import-Analyse §7.3).

## 5. Threading und Fehlerverhalten

- **Nicht thread-safe** — Nutzung von genau einem Thread zur Zeit. Der Besitzer
  (SuperscopeModule) ist über den Render-Mutex-Vertrag geschützt
  (Visualizer_Architecture §12): Config-Schreibpfade (setParam) laufen unter Mutex,
  execute() im Render-Thread.
- Compile-Fehler: Slot deaktiviert, `lastError()` gesetzt — kein Throw.
- Laufzeitfehler: Slot **deaktiviert sich selbst** (verhindert Fehler-Spam mit
  Frame-Rate); der Aufrufer fällt auf sein Fallback zurück.
- Aus dem Render-Thread wird **nie geloggt** (BasicLogger nicht thread-safe) —
  Fehler werden nur als String gehalten (`lastScriptError()` am Modul).

## 6. Performance

Gemessen (Unit-Test, Release-untypischer Testing-Build!): Superscope-Spiral-Skript,
**1000 Punkte ≈ 0,3–0,6 ms/Frame** — rund 3 % des 60-fps-Budgets (16,7 ms). Die
Prognose der Import-Analyse (§7.4: „<5 % eines Kerns") ist damit bestätigt;
LuaJIT ist nicht nötig (Entscheid §10.5). GC läuft generational (`LUA_GCGEN`).

## 7. Verwendung (Superscope)

`SuperscopeModule` (Param `render.script.lua`): Bei aktivem Skript-Modus werden die
vier Code-Slots als **EEL** (AVS-Dialekt) durch den
[EelTranspiler](../../../../libs/EelTranspiler/include/EelTranspiler.md) übersetzt
und als Lua ausgeführt — auch die Builtin-Preset-Strings laufen so als echte
Skripte. Vertrag: Host-Inputs `i, v, b, n, w, h, time, dt` — **`t` schreibt der
Host nie**, es gehört dem Skript (AVS-Stil: Frame-Code akkumuliert `t=t+0.02`).
Outputs: `x, y, skip`, plus `red/green/blue` [0..1] **wenn der Point-Code sie
erwähnt** (sonst färbt der Gradient wie bisher). Der Frame-Code darf `n`
(Punktzahl, Clamp 8–4096) ändern. Ohne kompilierten Point-Code (leer oder
Transpile-Fehler) rendert die hartkodierte Preset-Mathematik (z. B. DNA).
Rohes Lua ist weiterhin direkt an der Engine möglich (compile/run) — die
Superscope-Slots sind bewusst EEL (Import-treu).

**Reservierte Namen (Host-Vertrag):** `i, v, b, n, w, h, time, dt` (Inputs —
`b` wird JEDEN Frame überschrieben!) und `x, y, skip, red, green, blue`
(Outputs). Skripte dürfen sie lesen bzw. als Output schreiben, aber **nie als
freie Variablen zweckentfremden**. Achtung Case-Insensitivität: `R` und `r`
sind DIESELBE Variable (Lissajous-`b`- und Hypocycloid-`R/r`-Bug, Session 32).

```
-- Frame:  t=t+0.02; n=512
-- Point:  r=0.5+v*0.3; a=i*$PI*2+t;
--         x=sin(a)*r; y=cos(a)*r;
--         red=i; green=0.2; blue=1-i
```

## 8. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.1.0 | 2026-07-20 | Roadmap 4.1: Konstruktor nimmt shared ScriptContext; gmegabuf wandert in den Kontext; Superscope läuft über ScriptSlotHost (Session 33) |
| 1.0.0 | 2026-07-20 | Erstfassung — Sandbox, eel-Prelude, 4-Slot-Modell, app-globales Atomic-Register-Set, Superscope-Anbindung (Session 32) |

# MilkParser — .milk-Preset-Parser (Import-Zeit)

> **Version:** 1.2.0  
> **Datum:** 2026-07-22  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 6, M1 — Text → Struktur; M5 — Shader-Klassifikation)  
> **Modul:** lumi::milk (Lib **MilkParser**, header-only INTERFACE)  
> **Dateien:** MilkParser.hpp (API), MilkParserTypes.hpp, MilkShaderClassifier.hpp (M5)  
> **Namespace:** lumi::milk (detail: lumi::milk::detail)  
> **Abhängigkeiten:** keine (kein Qt, keine App-Teile) — Code-Blöcke bleiben Quelltext (Milk-EEL/HLSL), Transpilation macht [EelTranspiler](../../EelTranspiler/include/EelTranspiler.md)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [API](#2-api)
3. [Dateiformat](#3-dateiformat)
4. [Datenmodell](#4-datenmodell)
5. [Fehlerphilosophie und Import-Report](#5-fehlerphilosophie-und-import-report)
6. [Bewusste Abweichungen](#6-bewusste-abweichungen)
7. [Tests](#7-tests)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

Liest MilkDrop-Presets (`.milk`, zeilenbasiertes key=value-Format) in eine
Struktur ein: Scalars in Dateireihenfolge, nummerierte Code-Familien
(`per_frame_N`, `per_pixel_N`, `warp_N`, `wave_I_per_pointN`, …) zu Code-Blöcken
konkateniert. Das Datenmodell ist das **MD3-Superset** (Entscheid §6.2,
2026-07-22): bis 16 Custom Waves/Shapes, Sprite-Sektionen,
`PSVERSION[_WARP/_COMP]`-Header — was der Renderer noch nicht kann, parst
trotzdem sauber und ist für den Import-Report sichtbar.

Vertragsgrenze wie beim [AvsParser](../../AvsParser/include/AvsParser.md):
**Text → Struktur.** Kein EEL→Lua, kein HLSL-Handling, keine Übersetzung in
LumiViz-Presets (das macht der MilkdropVisualizer ab M3). Verhalten aus den
Referenz-Presets (`ref/winamp_orig`, `asset/Milkdrop3`) und der
MilkDrop3-Quelle (BSD) abgelesen; projectM diente NUR als Konzept-Referenz
(LGPL — kein Code übernommen). Konzept:
`projects/apps/MyViz/docs/visuals/MilkDrop_Import_Konzept.md` §2.2.

## 2. API

```cpp
#include <MilkParser.hpp>

auto r = lumi::milk::parseFile(pfad);      // oder parse(text) / parse(ptr, size)
if (!r.ok) { /* r.error: keine Preset-Daten erkennbar / Datei nicht lesbar */ }
r.generation();                            // 1 (MD1), 2 (v201/PSVERSION), 3 (v300+)
r.value("fDecay", 0.98);                   // Scalar-Lookup, case-insensitiv
r.perFrameCode; r.perPixelCode;            // konkatenierte Code-Blöcke (Milk-EEL)
r.warpShader; r.compShader;                // HLSL-Quelltext (Backtick-Zeilen)
r.wave(0); r.shape(1); r.sprites;          // Waves/Shapes/Sprites (MD3-Superset)
for (auto& w : r.warnings) { /* Import-Report */ }
```

`parse` wirft nie. `ok=false` gibt es NUR, wenn gar keine Preset-Daten
erkennbar sind (leer / keine key=value-Zeilen) oder die Datei unlesbar ist.

## 3. Dateiformat

```
MILKDROP_PRESET_VERSION=201        optionale Header (fehlen bei MD1)
PSVERSION=2 / PSVERSION_WARP / PSVERSION_COMP
[preset00]                         Sektion (fehlt bei handgeschriebenen Dateien)
fDecay=0.980000                    Scalars (f*/n*/b*/zoom/…), Reihenfolge egal
per_frame_init_N= / per_frame_N= / per_pixel_N=      nummerierte Code-Zeilen
warp_N=`…  /  comp_N=`…            Shader-Zeilen (Backtick = Zeilenumbruch bleibt)
wavecode_I_<param>= / wave_I_initN= / wave_I_per_frameN= / wave_I_per_pointN=
shapecode_I_<param>= / shape_I_initN= / shape_I_per_frameN=   (kein per_point)
[SPRITEn_BEGIN] … code_N= … [SPRITEn_END]             MD3-Sprites ([SPRITEn] auch)
```

Konkatenations-Regeln (Original-`ReadCode`-Verhalten, per Korpus verifiziert):

- Familien werden ab Index **1** gejoint; die **erste Lücke bricht ab**
  (verwaiste Zeilen danach → Warnung). Leere Werte (`per_frame_9=`) zählen als
  vorhanden.
- Zeilen **ohne** führenden Backtick werden **ohne** Zeilenumbruch aneinander
  gehängt — Ausdrücke dürfen mitten im Term umbrechen (häufig im Korpus).
- Zeilen **mit** führendem Backtick verlieren den Backtick und behalten den
  Zeilenumbruch (Shader-Quelltext bleibt lesbar).
- **Inline-Kommentare** (`//` UND `\\`) enden am ORIGINAL-Zeilenende
  (`StripLinefeedCharsAndComments`, state.cpp:1525) — sie werden VOR der
  Konkatenation zeilenweise gestrippt (nur Nicht-Backtick-Zeilen; Shader
  behalten ihre Kommentare, sie haben echte Umbrüche). Ohne das frisst ein
  `//rad` mitten im Preset den gesamten Rest-Code (Befund Session 39:
  18 Milkdrop3-Presets).

Toleranzen: UTF-8-BOM, CRLF, reine `//`-Kommentarzeilen, fehlendes
`[preset00]` (handgeschriebene Presets), unbekannte Sektionen (übersprungen),
Groß-/Kleinschreibung der Keys (Lookup case-insensitiv, Original nutzte
INI-Semantik).

## 4. Datenmodell

- `ParseResult` — `ok/error`, Versions-Header (`presetVersion`, `psVersion*`),
  `params` (alle Scalars als `KeyValue` in Dateireihenfolge; Zugriff über
  `rawValue`/`value`/`valueInt`), fünf Code-Blöcke (`perFrameInitCode`,
  `perFrameCode`, `perPixelCode`, `warpShader`, `compShader`), `waves`,
  `shapes`, `sprites`, `warnings`.
- `CustomWave` (Index 0–15) — `params` (Prefix `wavecode_I_` abgestreift),
  `initCode`/`frameCode`/`pointCode`.
- `CustomShape` (Index 0–15) — wie Wave, ohne `pointCode`.
- `Sprite` — Sektions-Index, `params`, `code` (aus `code_N`).
- `generation()` — 3 ab `MILKDROP_PRESET_VERSION>=300`, 2 ab `>=200` oder
  vorhandener `PSVERSION`, sonst 1.
- Zahlen-Parsing locale-unabhängig (`std::from_chars`); `paramInt` versteht
  `0x`-Hex (`SpriteColorKey`).

## 5. Fehlerphilosophie und Import-Report

Wie AvsParser (Import-Analyse §4.3): nie hart scheitern. Warnungen entstehen
für: Lücken mit verwaisten Zeilen, doppelte Code-Zeilen (erste gewinnt),
Wave-/Shape-Index über Superset-Cap 16, unbekannte Sektionen, Zeilen ohne `=`
(gezählt, mit Erstbeispiel), Code-Block über 2^20 Bytes (EelTranspiler-Cap),
fehlendes `[preset00]`. Scalar-Duplikate gewinnen still per Erst-Treffer
(INI-Semantik des Originals).

## 6. Bewusste Abweichungen

- **Kein `mesh_x/mesh_y`:** steht in keinem der 910 Korpus-Presets — die
  Mesh-Auflösung war ein App-Setting; bei uns Visualizer-Parameter (Entscheid
  §6.1: Default 32×24, Cap 96×72).
- **Superset-Cap 16** für Waves/Shapes (MD3-Maximum); höhere Indizes werden
  ignoriert und gemeldet statt das Datenmodell zu sprengen.
- Sektionsname `[SPRITEn]` (ohne `_BEGIN`) wird als Sprite-Beginn akzeptiert
  (kommt im Korpus vor).

## 7. Tests

`test_MilkParser.cpp` (UnitTests-Target): synthetische Fixtures für alle
Format-Kanten (Konkatenation, Lücken-Abbruch, Backtick, Waves/Shapes/Sprites,
MD3-Superset, BOM/CRLF, Fehlertoleranz) + zwei Korpus-Läufe (Smoke,
umgebungsabhängig — skippen sauber, wenn der Bestand fehlt):

| Korpus | Dateien | Generationen 1/2/3 | Ergebnis (2026-07-22) |
|---|---|---|---|
| `asset/Milkdrop3/presets` (untracked) | 352 | 7 / 259 / 86 | 352 ok, 1 Warnung |
| `../ref/winamp_orig/…/Milkdrop2/presets` | 558 | 303 / 255 / 0 | 558 ok, 8 Warnungen |

Die Warnungen sind echte Preset-Defekte (BrainStain-Dateien mit `per_frame_8`
ohne `=` — dort bricht auch das Original ab) bzw. das fehlende `[preset00]`
einer handgeschriebenen Datei.

## 8. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 39, M1): key=value-Parser, Code-Familien mit Lücken-Abbruch, Backtick-Shader, MD3-Superset (16 Waves/Shapes, Sprites, PSVERSION-Header), Import-Report; Korpus 910/910 |
| 1.1.0 | 2026-07-22 | Inline-Kommentar-Stripping (`//`+`\\` bis Original-Zeilenende, state.cpp:1525-treu) vor der Konkatenation — behebt 18 Milkdrop3-Presets, deren Rest-Code vom Kommentar geschluckt wurde (Session 39, M3-Diagnose) |
| 1.2.0 | 2026-07-22 | **MilkShaderClassifier.hpp** (Session 40, M5 Stufe B): klassifiziert warp/comp-HLSL als None/Md1Default/Md1Plus/Custom — generierte Default-Familie (GenWarp/GenCompPShaderText, plugin.cpp:8782-8847) + lineare Extras als affines Modell `gain·Basis + Σ Bn·blurN` mit eingebackenen Echo/Gamma/Hue/Filter-Konstanten; whitespace-insensitives Matching, Feature-Flags (Blur/Noise/Texturen/rand, präfix-bewusst); Korpus-Gate 910 in `test_MilkShaderClassifier.cpp` |

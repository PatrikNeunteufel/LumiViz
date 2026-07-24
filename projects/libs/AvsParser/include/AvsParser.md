# AvsParser — .avs-Preset-Parser (Import-Zeit-Container)

> **Version:** 1.2.0  
> **Datum:** 2026-07-20  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert (Import-Phase Roadmap 3 — Container + Kernmengen-Decoder)  
> **Modul:** lumi::avs (Lib **AvsParser**, header-only INTERFACE)  
> **Dateien:** AvsParser.hpp (API), AvsParserTypes.hpp, AvsParserReader.hpp, AvsParserEffects.hpp  
> **Namespace:** lumi::avs (detail: lumi::avs::detail)  
> **Abhängigkeiten:** keine (kein Qt, keine App-Teile) — EEL-Slots bleiben Quelltext, Transpilation macht [EelTranspiler](../../EelTranspiler/include/EelTranspiler.md)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [API](#2-api)
3. [Dateiformat](#3-dateiformat)
4. [Datenmodell](#4-datenmodell)
5. [Dekodierte Kernmenge](#5-dekodierte-kernmenge)
6. [Fehlerphilosophie und Import-Report](#6-fehlerphilosophie-und-import-report)
7. [Bewusste Abweichungen](#7-bewusste-abweichungen)
8. [Tests](#8-tests)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

Liest „Nullsoft AVS Preset 0.1/0.2"-Dateien (rekursives TLV-Binärformat) in einen
Effekt-Baum ein. Jeder Knoten behält seinen Roh-Blob; für die Import-Kernmenge
(~17 Effekte + Effektlisten) werden zusätzlich benannte Int-Felder, Farbtabellen
und EEL-Code-Slots dekodiert. Die Übersetzung in LumiViz-Presets (Roadmap 5)
und die EEL→Lua-Transpilation sind bewusst NICHT Teil dieser Lib — Vertragsgrenze:
Bytes → Baum. Alle Layouts sind 1:1 aus `ref/vis_avs` (BSD-3, Nullsoft 2005)
abgelesen; Fundstellen als Kommentar an jedem Decoder
(Analyse: `Import_Analyse_AVS_MilkDrop.md` §5).

## 2. API

```cpp
#include <AvsParser.hpp>

auto r = lumi::avs::parseFile(pfad);          // oder parse(bytes) / parse(ptr, size)
if (!r.ok) { /* r.error: keine/kaputte Signatur — keine .avs-Datei */ }
r.formatVersion;                              // 1 oder 2
r.root;                                       // EffectNode, isList=true, "Main"
for (auto& w : r.warnings) { /* Import-Report */ }
```

`parse` wirft nie. `ok=false` gibt es NUR bei ungültiger Signatur/zu kurzer Datei.

## 3. Dateiformat

```
"Nullsoft AVS Preset 0.2\x1a"        Signatur (0.1 wird ebenso akzeptiert)
[Listen-Config des Root]             beginnt mit Mode-Byte
```

Listen-Config (Root wie verschachtelt, ref r_list.cpp:64):

```
[1 Byte]   Mode (Bit 0 = Clear every frame; Bit 7 gesetzt -> volles int32-Mode folgt)
[int32?]   volles Mode (Bits 8-12 Blend-In, 16-20 Blend-Out^1, 24-31 Extended-Size)
[Extended] wenn Size>0: inblendval, outblendval, bufferin, bufferout,
           ininvert, outinvert, beat_render, beat_render_frames (je int32, geführt)
0..n Einträge:
  [int32]  Effekt-ID  (<16384 Builtin-Index · >=16384: 32-Byte-APE-ID-String folgt
                       · 0xFFFFFFFE (= -2 signiert): verschachtelte Liste)
  [int32]  Blob-Länge L
  [L Byte] Config-Blob
```

Der EEL-Code einer Liste (use_code, Init, Frame) reist als Pseudo-Eintrag mit
APE-ID `"AVS 2.8+ Effect List Config"` und wird dem Listen-Knoten zugeschlagen
(kein Kind). EEL-Strings: neues Format = Versions-Byte `0x01` + längenpräfixierte
Strings (Länge inkl. NUL); Altformat = feste 256er-Blöcke (Quartett: 1024).
Slot-Reihenfolge im File immer `[Point/Level, Frame, Beat, Init]`.
Alte benannte APEs (z. B. `"Nullsoft MIRROR v1"`) werden per Alias-Tabelle auf
Builtins gemappt (ref rlib.cpp:159).

## 4. Datenmodell

- **`ParseResult`** — ok/error, formatVersion, `root`, `warnings` (Import-Report),
  `effectCount()`.
- **`EffectNode`** — `id`, `apeId`, `name`, `rawConfig` (immer), `decoded`,
  `fields` (benannte int32 in Dateireihenfolge, Zugriff `field("blend")`),
  `colors`, `code` (Zugriff `slot("point")`), bei Listen `isList` + `list` +
  `children`.
- **`ListInfo`** — rohes `mode` + Accessoren (`clearEveryFrame()`, `enabled()`,
  `blendIn()`, `blendOut()`), Extended-Felder, `useCode`/`initCode`/`frameCode`.

## 5. Dekodierte Kernmenge

Effect List (rekursiv) · SuperScope (36) · Movement (15, inkl. `!rect`-Marker
und 32767-Expression) · Dynamic Movement (43) · Blur (6) · Fadeout (3) ·
Brightness (22) · Fast Brightness (44) · Color Modifier (45) · Colorfade (11) ·
Clear Screen (25) · OnBeat Clear (5) · Buffer Save (18) · Mirror (26) ·
Invert (37) · Roto Blitter (9) · Blitter Feedback (4) · Custom BPM (33) ·
Set Render Mode (40). Feldnamen = Variablennamen der jeweiligen `load_config`
(r_*.cpp). Alles andere (inkl. der 5 eincompilierten Builtin-APEs) bleibt
Roh-Blob mit aufgelöstem Namen.

## 6. Fehlerphilosophie und Import-Report

Preset-Bestände sind schmutzig (Analyse §4.3): strukturelle Schäden NACH gültiger
Signatur brechen nie hart ab — das Parsen stoppt an der Schadstelle der jeweiligen
Liste, alles davor bleibt erhalten, und der Befund landet pfad-präfixiert in
`warnings` (`"Main/2 SuperScope: …"`). Gemeldet werden: abgeschnittene
Einträge/Blobs, negative Längen, unbekannte Builtin-IDs, unbekannte APEs,
Verschachtelung > 100 Ebenen (Schutz vor degenerierten Dateien).

## 7. Bewusste Abweichungen

1. **Toleranter als das Original bei Minimal-Dateien:** Signatur ohne Nutzdaten
   parst als leeres Preset (AVS verlangt > 26 Bytes).
2. **OOB-Peeks des Originals** (Versions-Byte-Check, GET_INT in Slack) lesen bei
   uns definiert 0 statt Puffer-Slack — auf validen Dateien identisches Verhalten.
3. **Strings werden am ersten NUL gekappt** (Original schleppt den NUL im
   RString mit).
4. `which` (Buffer Save) wird wie im Original auf 0..7 geklemmt.

## 8. Tests

`tests/unit/UnitTests/test_AvsParser.cpp` (17 Cases): synthetische
Binär-Fixtures über einen Byte-Builder (Signaturen, TLV-Grundformen, neues +
Altformat-Code-Quartett, Movement-Sonderfall, verschachtelte Liste mit
Extended-Data + Listen-Code, APE-Alias, Fehlertoleranz-Fälle) plus Korpus-Lauf
über die 35 Referenz-Presets aus `../ref/vis_avs` (lokal, unversioniert;
fehlt der Korpus, läuft nur der synthetische Teil). Korpus-Stand: 35/35 ok,
170 Effekte, 5 Warnungen (Community-APE „FunkyFX FyrewurX", erwartet unbekannt).

## 9. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.2.0 | 2026-07-24 | Alias-APEs: `child.id` wird beim Aliasing auf den Builtin-Index umgeschrieben (vorher blieb die Roh-ID — 0.1-Presets: Pointer-Wert — stehen und der Chain-Translator konnte trotz decodierter Felder nicht dispatchen; Beleg „Winamp Starfield v1"/„Winamp Mosaic v1" in Spacefolding, Session 44) |
| 1.1.0 | 2026-07-22 | `decodeApe`: FunkyFX FyrewurX v1 (enabled + opakes Config-Wort) — kein „unbekannter APE"-Report mehr (Session 38) |
| 1.0.0 | 2026-07-20 | Erstfassung — Container-TLV, Kernmengen-Decoder, Import-Report-Gerüst (Session 33) |

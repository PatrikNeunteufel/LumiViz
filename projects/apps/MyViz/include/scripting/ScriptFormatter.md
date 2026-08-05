# ScriptFormatter — Beautify-Kerne der Chain-Skript-Editoren

> **Version:** 1.0.0
> **Datum:** 2026-08-05
> **Typ:** CppModuleDoc
> **Status:** Implementiert (Offene_Punkte §7 Editor-Komfort, Session 69)
> **Modul:** lumi::scripting (freie Funktionen)
> **Dateien:** ScriptFormatter.hpp (header-only)
> **Namespace:** lumi::scripting
> **Abhängigkeiten:** keine (kein Qt — bewusst pur, Entscheid Patrik 2026-08-05)
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Übersicht

Die beiden Beautify-Kerne hinter dem Beautify-Knopf des Groß-Editors
([EelScriptEditing](../UI/panels/EelScriptEditing.hpp)): bewusst minimal,
„clang-format-artig" nur im Geiste. **Whitespace-only-Vertrag:** jedes Token
der Eingabe überlebt byte-identisch (Kommentare eingeschlossen) — ein
Beautify kann nie ändern, was ein Skript rechnet. Die Tests erzwingen das
(Eingabe ohne Weißraum == Ausgabe ohne Weißraum).

## 2. API-Kern

- `FormatOptions` — gemeinsame Stellschrauben beider Kerne (Settings-Dialog,
  Block „Editor"): `indentWidth` (Leerzeichen je Stufe), `spaceAroundOperators`
  (nur EEL: ` = ` statt `=`), `maxBlankLines` (Leerzeilen-Klemme).
- `beautifyEel(src, opt)` — EEL-Statement-Re-Flow: **ein Statement pro Zeile**
  (Umbruch nach jedem `;`), Einzug = Klammertiefe am Zeilenbeginn (deckt
  `loop(`/`if(`/`exec2/3(`-Körper und `(a;b)`-Blöcke gleichermaßen; eine
  Zeile, die mit `)` beginnt, nutzt die umschließende Stufe). `// …` hinter
  dem `;` derselben Input-Zeile bleibt am Statement kleben; alleinstehende
  Kommentare behalten ihre Zeile; `/* … */` bleibt inline. Unäre `+`/`-`/`!`
  kleben am Operanden; Zahlen-Token (`1e-5`, `0x1F`, `2h`, `$PI`, `$'c'`)
  bleiben ganz.
- `beautifyGlsl(src, opt)` — zeilenbasiertes **Brace-Re-Indent** (passt auch
  für HLSL-`shader_body`): nur der Einzug wird neu gebaut, die Token-Abstände
  innerhalb der Zeile bleiben. `#`-Präprozessor-Zeilen auf Spalte 0;
  Block-Kommentar-Innenzeilen unangetastet; Braces in Kommentaren zählen
  nicht; bei führendem `}` bestimmt der innerste Closer die Spalte.

## 3. Bewusste Grenzen

- Kein Zeilen-Split langer Ausdrücke, kein Umbruch von `if(a, b, c)`-Argumenten
  ohne inneres `;` — nur Weißraum-Neuaufbau nach den zwei Regeln oben.
- Milkdrop-Spezialfall (geklärt 2026-08-05): die `per_frame_N=`-Zeilen-
  Verteilung betrifft NUR einen etwaigen `.milk`-Rückexport — als Chain-Node/
  `.lvfx` leben die Skripte als ganze Strings (`assembleCode`), der Formatter
  arbeitet auf genau diesen.
- Nicht-ASCII-Pseudo-Kommentare (tote Identifier, S48-Befund) werden wie
  gewöhnliche Token durchgereicht — statement-lokal, Semantik unverändert.

## 4. Tests

`tests/unit/UnitTests/test_ScriptFormatter.cpp` — 26 Cases: Statement-Umbruch,
mehrstufiger Einzug, Operator-Abstände an/aus, unäres Minus, Exponenten,
Kommentar-Kleben, Leerzeilen-Klemme, Idempotenz, Whitespace-only-Vertrag,
GLSL-Re-Indent (Präprozessor, Kommentar-Braces, Block-Kommentar roh,
Mehrfach-Closer, Einzugsbreite), Leer-Eingaben.

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-08-05 | Erstfassung — Editor-Komfort §7 (Session 69) |

# AvsChainTranslator

> **Modul:** `lumi::multieffect` · `include/visualizers/multieffect/AvsChainTranslator.hpp` +
> `src/visualizers/AvsChainTranslator.cpp` · GL-frei ·
> **Seit:** Import-Phase Roadmap 5.5 (Session 34) ·
> **Steuerdokument:** `docs/visuals/Import_Multieffekt_Host_Entwurf.md` (E4/E7)

Übersetzt einen geparsten AVS-Baum (`lumi::avs::ParseResult` aus der Qt-freien
`AvsParser`-Lib) in eine Host-[`EffectChain`](EffectChain.md). Lebt im App-Modul
(E7 — mappt Parser-Daten auf Host-Runtime-Params, kein GL, aber App-Typen).

```cpp
lumi::avs::ParseResult parsed = lumi::avs::parseFile(path);
TranslationResult t = translateAvsTree(parsed);   // t.root ist compiled
// t.report: PROBLEME — Parser-Warnungen, nicht unterstützte Effekte (pfad-präfixiert)
// t.notes : HINWEISE — planmäßige `_p`-Umbenennungen, bewusst ignorierte Effekte
```

Die Trennung ist der Grund, warum das Meldungsfenster wieder aussagekräftig ist:
`report` rechtfertigt einen Dialog, `notes` nicht. `loadAvsFile` schreibt beides
zusammen in einen **„Import Notes"-Knoten** der Kette (Entscheid Patrik, S51) —
vorher gingen echte Probleme in einem Dutzend Umbenennungs-Zeilen unter.

Der Host bietet `MultiEffectVisualizer::loadAvsFile(path, &report)` als
GUI-Einstieg (unter `renderMutex()`; setzt ein Reset-Flag, damit der
Render-Thread die alten Knoten-Runtimes freigibt — Knoten-IDs werden neu vergeben).

## Mapping-Regeln

- **Effektlisten** → `ListParams` (Blend In/Out aus `ListInfo`, Adjustable-Alpha
  aus `in/outBlendVal`, OnBeat, `clearEveryFrame`, EEL Init/Frame-Slots).
- **17 dekodierte Kern-Effekte** → ihre `EffectParams` (Feldnamen 1:1 aus dem
  AvsParser-Decoder verifiziert).
- **Set Render Mode (40)** ist ein **eigener Ketten-Knoten** mit Laufzeit-Zustand
  (`SetRenderModeParams`): Linienbreite und der volle 10er-`BLEND_LINE`-Modus
  (S44/S9) gelten für die *folgenden* Zeichner. Der Zustand wird je Frame
  zurückgesetzt und um Listen-Renders gerettet (S45/S3, wie `r_list.cpp:433/440`).
  Das frühere „Ausrollen in die folgenden SuperScopes" (E4) gibt es nicht mehr.
- **Movement-Builtin-Formeln** → `MovementParams` mit dem AVS-Point-Code aus
  `movementBuiltinFormula()` (23 Formeln 1:1 als `eval_desc`). #1 „slight fuzzify"
  und #7 „blocky partial out" sind keine Formeln, sondern Spezial-C-Code — sie
  laufen seit S44 über `MovementParams.builtinRemap` (eigener Per-Pixel-Shader),
  nicht mehr über Passthrough. `ScriptGridModule` teilt die AVS-Polar-Konvention;
  `d`/`r` rechnen seit S45 im PIXEL-Raum wie `r_dmove.cpp:324-332`.
- **Alles andere** (unbekannt, `decoded=false`, exotisch) → `PassthroughParams`
  (Quell-ID + Notiz) + Report-Eintrag. **Nie werfen** (AVS-Philosophie).
- **Farben:** AVS-COLORREF `0x00BBGGRR` → Host `0x00RRGGBB` (R/B getauscht).

## Bewusste Näherungen (Sichttest-Feinschliff, im Code markiert)

- **Blitter/Roto Feedback** Zoom-/Rotations-Mapping (`scale`/`zoom_scale`/`rot_dir`
  → Faktor) ist approximativ — exakte AVS-Slider-Kurve später kalibrieren.
- **SuperScope-Farbtabelle** (`colors[]`) wird nicht übernommen — Farben kommen
  aus dem Punkt-Skript.

> **Erledigt (S50):** der **geteilte ScriptContext** für Scopes. Der Scope-Host
> wurde ohne `activeContext()` gebaut und hatte damit einen isolierten
> `reg00..reg99`-Raum; AVS hält diese Register GLOBAL. Presets, die sich in allen
> Scopes eine Kamera-Matrix aus den Registern einer Dynamic Movement holen
> („Mister Santa"), zeigten deshalb nur den Hintergrund.
>
> **Erledigt (S51):** die **Import-Kollisionsregel D2** — Namen des Lumi-Sets ohne
> AVS-Builtin-Bedeutung werden beim Import auf `_p` umbenannt, einheitlich über
> alle Slots einer Komponente, case-insensitiv geprüft, mit ℹ-Zeile im Report.
> Listen-Codes laufen über `ListInfo` und bleiben unangetastet (dort ist `beat`
> ein Builtin). SSOT der reservierten Namen: `include/scripting/ScriptBaseKeys.hpp`.
> Umbenennungs-Ziele sind kanonisch kleingeschrieben — EEL ist case-insensitiv,
> das transpilierte Lua nicht.

## Absicherung

`tests/unit/UnitTests/test_AvsChainTranslator.cpp` — Feld-Mapping, COLORREF-Swap,
Passthrough, SRM-Unroll, verschachtelte Liste; **Korpus-Smoke: 35/35
Referenz-Presets übersetzen ohne Crash** (163 Knoten, umgebungsabhängig).

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
- **Set Render Mode (40)** wird **ausgerollt** (E4): Linienbreite (Bits 16–23)
  wandert in die *folgenden* SuperScope-Effekte; der Knoten wird Passthrough+Notiz.
- **Movement-Builtin-Formeln** → `MovementParams` mit dem AVS-Point-Code aus
  `movementBuiltinFormula()` (23 Formeln 1:1 als `eval_desc`; #0/#1/#7 sind keine
  Remaps → Passthrough). `ScriptGridModule` teilt dafür die AVS-Polar-Konvention.
- **Alles andere** (unbekannt, `decoded=false`, exotisch) → `PassthroughParams`
  (Quell-ID + Notiz) + Report-Eintrag. **Nie werfen** (AVS-Philosophie).
- **Farben:** AVS-COLORREF `0x00BBGGRR` → Host `0x00RRGGBB` (R/B getauscht).

## Bewusste Näherungen (Sichttest-Feinschliff, im Code markiert)

- **Blitter/Roto Feedback** Zoom-/Rotations-Mapping (`scale`/`zoom_scale`/`rot_dir`
  → Faktor) ist approximativ — exakte AVS-Slider-Kurve später kalibrieren.
- **SuperScope-Farbtabelle** (`colors[]`) wird nicht übernommen — Farben kommen
  aus dem Punkt-Skript; geteilter ScriptContext für den Scope steht noch aus.

## Absicherung

`tests/unit/UnitTests/test_AvsChainTranslator.cpp` — Feld-Mapping, COLORREF-Swap,
Passthrough, SRM-Unroll, verschachtelte Liste; **Korpus-Smoke: 35/35
Referenz-Presets übersetzen ohne Crash** (163 Knoten, umgebungsabhängig).

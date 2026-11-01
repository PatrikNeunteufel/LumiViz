# Changelog Session 168 (2026-10-31/11-01)

> **Raster und Swing stehen jetzt im Kopf des Loop-Panels, und jedes Muster kann anders swingen.** *Beats per bar*,
> *Steps per beat* und *Swing* gelten für den ganzen Loop und standen bisher nur im Drum-Editor; jetzt stehen sie
> neben BPM. In Drum- und Instrument-Editor zeigt eine Zeile das Raster an und daneben den **Swing offset** des
> Musters — ein Solist swingt oft etwas weniger als das Schlagzeug. Dazu ist Stufe 15 (Wave-Clips: Tonspuren wie
> Gesang, mit Autotune) fertig geplant. Steuerdokument: `docs/audio/Loop_Konzept.md` §4p.

## Neu

- **Swing offset** je Drum- und Instrument-Muster (RS1-1 a): in Prozentpunkten, gespielt wird Swing des Loops plus
  Offset (0 bis 100 %); der Editor zeigt „plays …“. Swing wirkt auf Schritte und Noten.
- **Stufe 15 geplant** (WV1 a–WV9 a, Skizze SK15-1 a): Wave-Muster als dritte Art mit eigenem Fenster *Wave editor*,
  Ausschnitt, Lage, Gain und Blenden, *Follow tempo*, Spuren Expression, Pitch und Autotune (Zielnoten), Aufnahme,
  *Bounce to wave clip*, Steuersignal für eine spätere Talkbox. Teilschritte 15b–15m.
- Recherche zu Zeitdehnung, Tonhöhe und Tonhöhenmessung unter `docs/modellierung/wave-clips/recherche/`.

## Geändert

- **Raster und Swing** im Kopf des Panels Loop neben BPM, nicht mehr im Drum-Editor (RS1 a).
- `.lvloop` **Fassung 15** (`swingOffset` je Muster, immer geschrieben). Alte Loops klingen unverändert; eingefrorene
  Clips werden beim nächsten Speichern neu eingefroren.
- Benutzerhandbuch 1.205.0 (§16).

## Intern

- `swingVon`, `setzeSwingOffset`; Bauteil `loop/RasterZeile`; Rechenkern und Saitenplan je Muster.
- Tests: 1688 Cases (+3, `test_LoopSwingOffset`). Voller Lauf unter Release-clang und Testing-MSVC je 1688 / 1688.
- HörTest: LumiViz im Sync `sync_hoertest`, Listenformat HT-1 entschieden (HörTest_Helper Konzept 0.3.0 §12),
  Liste pv1 (LV-2) freigegeben.

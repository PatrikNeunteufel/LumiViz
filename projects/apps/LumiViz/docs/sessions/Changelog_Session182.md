# Changelog Session 182 (2026-11-09)

> **Die gerechneten Instrumente bekommen eigene Loops.** Jedes Instrument mit Rechenkern zeigt in einem eigenen Loop,
> was es kann: Lage, Stärke, Länge, Pedal, Ausdruck und seine besonderen Spielweisen. Dazu ein Ensemble mit allen.
> Daneben ist die Werkstatt `InstrumentsCalculation` in LumiViz aufgegangen.

## Neu

- **Loops der Instrumente mit Kern** im neuen Ordner *modelled_instruments*: je Instrument ein Loop (21), etwa Marimba,
  Vibraphon, die Klaviere, die Trommeln, das Clavinet. Jeder spielt dasselbe Programm — einen Lauf über den Umfang, acht
  Stärken, vier Längen, ein Arpeggio mit Pedal und einen Akkord, der an- und abschwillt — und danach die Spielweisen des
  Instruments: Vibraphon mit Motor und Bogen, Pauke mit Wirbel und Pedal, Trommeln mit Randschlag, Cross-Stick und
  gedämpft, Clavinet mit Dämpferschieber und Tonabnehmern, Upright mit Practice-Pedal.
- **Modelled ensemble** — ein Stück mit allen Instrumenten mit Kern zusammen.
- Die Beispiele *Grand piano*, *Bars*, *Membrane*, *Keys* und die *Nocturne* (unter *full*) liegen jetzt ebenfalls dort.

## Geändert

- Die Instrumente, die das Ensemble spielt, führen im Fenster *Instruments* zusätzlich das Genre *Fusion*.

## Bekannt

- Das Wurlitzer (Reed electric piano) ist im Bass viel zu laut und im Diskant zu leise; die Kalimba und das Clavinet
  ähnlich in der Gegenrichtung. Die Ursache ist gefunden (Bauwerte, die über die Tasten fest sind), die Berichtigung folgt.
- Die neuen Loops der einzelnen Instrumente sind leiser als die übrigen Beispiele; angeglichen wird, wenn das Wurlitzer
  berichtigt ist.

## Intern

- `InstrumentsCalculation` ist aufgelöst: LumiViz ist die einzige Quelle für Rechenkerne und Vorgaben. Doku unter
  `docs/modellierung/<klasse>/ic/`, Code als Vorlage unter `harvest/instruments-calculation/`, Plan
  `docs/modellierung/anbindung-ic/Aufloesung_IC_Plan.md`; Konzept `docs/audio/Instrumente_IC_Konzept.md` 0.14.0.
- Fremde Dokumente und Hörproben liegen außerhalb des Repos (`…\cmake\quellen`, `…\cmake\samples\Hoerproben`), Index
  ohne Archiv unter `docs/archiv/` (`tools/archiv_index.py`); die IC-Hörlisten versioniert unter `hoertest/listen/ic/`.
- Neue Wächter: `*Modelled loops*` (jede Spielweise ändert den Ton), `Loop Pegel LP3 a` (Lautheit über den Umfang),
  `*Hoerlisten AU3-1*` (Liste im Repo gleich der am Hörort). 1855 Testfälle.
- Liste gemeinfreier ganzer Stücke für später: `docs/audio/Ganze_Stuecke_Kandidaten.md`.

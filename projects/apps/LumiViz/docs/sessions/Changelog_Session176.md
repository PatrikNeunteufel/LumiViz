# Changelog Session 176 (2026-11-07)

> **Der Flügel ist der erste Rechenkern.** Teilschritt 34d ist fertig: der Flügel rechnet als Kern `hammerString` aus
> der Bibliothek `InstrumentModels`, Bit für Bit wie vorher; `.lvinst` steht in Fassung 21, und der Instrument-Creator
> zeigt eine Kern-Schicht mit den Werten ihrer Tafel (`docs/audio/Instrumente_IC_Konzept.md` 0.7.0).

## Neu

- **Bibliothek `InstrumentModels`** (`projects/libs/InstrumentModels`, STATIC, BK1 a): die Kerne der Instrumentklassen
  über `InstrumentCore`, je Klasse ein Unterordner; heute der Flügel (`hammer/`, `lumi::kern::hammer`) mit seiner Tafel
  (22 Kurven, Gruppe *Pedals*) und der Fabrik `kerne()`. Quellpaket `package_instrumentmodels` für IC.
- **`.lvinst` Fassung 21:** eine Kern-Schicht trägt `core`, `coreParams` (jede Kurve mit Belegkraft und Herkunft) und
  `body`; ein fehlender Wert oder ein unbekannter Kern macht die Datei unlesbar. Der mitgelieferte *Grand Piano* steht in
  Fassung 21 mit unveränderten Werten.
- **Instrument-Creator:** eine Kern-Schicht zeigt im Signal path *Core*, *Body* und *Level*. *Core* listet die Werte des
  Kerns zum Ansehen, in ihren Gruppen, jede Kurve als kleine Form mit Marke der Belegkraft (*Measured*, *Source*,
  *Computed*, *Open*); neben *Type* steht *Unmeasured*, solange ein klangbestimmender Wert nicht gemessen ist.

## Geändert

- Der Körper des Flügels steht an der Schicht (FU1 a), die erste Taste ohne Dämpfer in der Tafel (FU2 a); der Block
  `keyboard` trägt nur noch, was die Planung braucht.
- Die alte Saite `modal` (Fassung 16–20) wird nicht mehr als Flügel gelesen.

## Intern

- Tests **1770 Cases**; voller Lauf 1770 / 1770 unter Testing-MSVC, unter Release-clang nach der Korrektur eines
  Fassungs-Literals grün. `Loop Werte` unverändert, `klangfassung.txt` nur mit neuen Abdrücken.
- Flügel-Bezug je Plattform `hammer_bezug_windows.txt` (IK2-5 a): alle neun Töne Bit für Bit unter beiden Compilern;
  die alten Dateien je Compiler waren im Chor älter als 34c.
- Rechenzeit Taste 40 abwechselnd gegen den Stand 34c gemessen: gleich oder schneller.
- Übernahme-Journal 0.2.0 (J12–J18), Modul-Doku `InstrumentModels.md`, `InstrumentCore.md` 0.1.1.

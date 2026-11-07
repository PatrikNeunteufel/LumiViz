# Changelog Session 174 (2026-11-07)

> **Build-System und Planung, nichts am Programm geändert.** LumiViz baut mit CMakeCraft v0.14.0, und die Skizze von
> Creator und Editor für die Instrumente aus `InstrumentsCalculation` ist gebilligt
> (`docs/audio/Instrumente_IC_Konzept.md` 0.5.0).

## Neu

- **CMakeCraft v0.14.0** (Teilschritt 34b): eine Bibliothek kann eigene Compiler-Schalter tragen (`compile_options`),
  und eine Bibliothek lässt sich als Quellpaket schnüren, das ein Projekt ohne CMakeCraft mit einfachem CMake einbindet
  (Quellen, Köpfe, erzeugtes `CMakeLists.txt` mit allen Schaltern, mitgelieferte Header-Bibliotheken wie pocketfft,
  Version, Prüfsumme je Datei). Damit bekommt `InstrumentsCalculation` die Rechen-Bibliotheken mit denselben
  Fließkomma-Schaltern wie LumiViz.
- **Skizze Creator und Editor** (Teilschritt 34a, CK1 a, CK2 a): ein Creator für alle Instrumentklassen, gebaut aus der
  Tafel der Werte je Kern — Gruppen, Regler, Listen, Haken, Kurven über die Tastatur mit Herkunft (*Measured*,
  *Source*, *Computed*, *Open*), Hinweis *Retune* bei veralteten gestimmten Werten, Marke *Unmeasured*, Anspielen je
  Stufe; der Instrument-Editor zeigt die Spuren, die der Kern spielen kann.

## Intern

- Pin `cmakecraft.pin` v0.14.0, `CMakeCraftBootstrap.cmake` auf dem Stand der Vorlage.
- Tests unverändert **1753 Cases**; voller Lauf nach dem Pin-Wechsel unter Release-clang und Testing-MSVC grün.

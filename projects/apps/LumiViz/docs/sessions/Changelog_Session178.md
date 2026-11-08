# Changelog Session 178 (2026-11-08)

> **Die Stabspiele.** Teilschritt 34g ist fertig: die Klasse 4 aus InstrumentsCalculation rechnet in LumiViz als ein Kern
> für neun Instrumente — Marimba, Xylophon, Vibraphon, Glockenspiel, Röhrenglocken, Celesta, Kalimba und zwei elektrische
> Klaviere —, Ton für Ton Bit für Bit wie in InstrumentsCalculation (`docs/audio/Instrumente_IC_Konzept.md` 0.10.0,
> Übersicht in `docs/modellierung/platte-stab/README.md`).

## Neu

- **Neun Instrumente aus dem Kern `bar`:** `marimba`, `xylophone`, `vibraphone` (Rohr mit Motor, Dämpferleiste, Bogen ab
  E♭4), `glockenspiel`, `tubular_bells`, `celesta`, `kalimba`, `tine_electric_piano`, `reed_electric_piano` — modale Stäbe
  und Zungen mit Schlägel, Hammer oder Zupfen, Resonator, Dämpfer und Tonabnehmer; alle **ungemessen** (Werte aus Quellen,
  Rechnungen und Einschätzungen). Die Steeldrum der Klasse geht nicht mit (Platzhalter).
- **Die bisherigen Nachbildungen** heißen **`synth_marimba`, `synth_vibraphone`, `synth_kalimba`** (Titel „Synth …“); die
  mitgelieferten Loops spielen sie unverändert.
- **Beispiel `bars_demo`:** acht Takte, zuerst die Stabspiele mit Schlägel, dann Celesta, Kalimba und die elektrischen
  Klaviere.
- **Hörliste `hoertest/listen/stab_34g`** für HörTest_Helper: je Instrument drei Lagen und drei Stärken, dazu blinde Paare
  IC gegen Synth.
- **Rechenkern:** ein Kern dient vielen Instrumenten — der Tastenumfang ist ein Wert des Instruments (KS1 a); gestimmte
  Werte werden in der Reihenfolge der Tafel gestimmt (KS2 a: der Pegel des Resonators, danach die Fenster des Bogens).

## Geändert

- Der Creator zeigt den Kasten *Body* nur, wo der Körper außerhalb des Kerns liegt (beim Flügel), nicht bei den
  Stabspielen (CK1 a).
- Eine leere Kurve bekommt die Vorgabe der Tafel im Umfang des Instruments und mit ihren Quellen.
- `InstrumentTuneTool` stimmt erstmals einen echten Kern (Pegel des Partners, Bogenfenster in rund 2½ Minuten).
- Quellenregister um sieben Quellen ergänzt, `instruments/CREDITS.md` neu.

## Intern

- Tests **1815 Cases**; voller Lauf 1815 / 1815 unter Release-clang und Testing-MSVC. Neue Fälle `test_InstrumentModelsStab`
  (Balken, Reibung, Kern, IC-Fälle, Rechenzeit), `test_LoopStab` (Dateien, Hörliste); B6 „Modalfit am Modell“ in
  `test_InstrumentMeasure`; Creator-Fall mit dem Vibraphon.
- `Loop Werte` nur mit neuen Namen und neuen Zeilen, `klangfassung.txt` mit gleichen Werten.
- Rechenzeit: höchstens 2,17 % der Echtzeit (Vibraphon mit Motor), 0,70 × Flügel.
- Bibliotheken `InstrumentCore` 0.3.0, `InstrumentModels` 0.2.0, `InstrumentMeasure` 0.3.0 (hängt jetzt an
  `InstrumentModels`); Übernahme-Journal J31–J41.

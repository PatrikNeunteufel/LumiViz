# Changelog Session 179 (2026-11-08)

> **Die Trommeln.** Teilschritt 34h ist gebaut: die Klasse 5 aus InstrumentsCalculation rechnet in LumiViz als ein Kern
> für sieben Trommeln — Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo —, Bit für Bit wie in InstrumentsCalculation
> (`docs/audio/Instrumente_IC_Konzept.md` 0.11.0, Übersicht in `docs/modellierung/membran/README.md`). Offen ist die
> Nennung der Quellen der Werte, die keine Kurven sind (QM1 a, entschieden).

## Neu

- **Sieben Instrumente aus dem Kern `membrane`:** `low_tom`, `timpani` (die Note wählt die Spannung, C3–G3), `bass_drum`,
  `snare_drum` (mit Teppich), `tabla` (harmonisch durch den Belag), `conga`, `bongo` — Felle mit Luftlast und Kessel,
  Schlag mit Hertz-Kontakt, Tonhöhengleiten; alle **ungemessen**. Die bisherige Nachbildung heißt **`synth_conga`**.
- **Stärke:** die Anschlagstufe ist die Lautstärke in dB — eine Note bei −20 dB klingt rund 20 dB leiser (TM1 a).
- **Spielweisen** an Kit-Rolle und Drum-Spur (TM2 a): Mitte, Rand, Randschlag, Cross-Stick, gedämpft, Wirbel, Teppich ab
  (`coreTracks`; `.lvloop` Fassung 18, `.lvkit` Fassung 2).
- **Kit „Modelled drums“:** Bassdrum, Snare, Cross-Stick, Randschlag, Tom, Conga, Bongo, Tabla; Hats und Becken behält
  eine Spur beim Kitwechsel.
- **Beispiel `membrane_demo`** und **Hörliste `hoertest/listen/membran_34h`** für HörTest_Helper.

## Geändert

- Alle mitgelieferten Kits in Fassung 2 (sonst unverändert).
- Quellenregister um neun Quellen ergänzt.
- Wechselt eine Drum-Spur oder Kit-Rolle Klang oder Instrument, fällt ihre Spielweise weg.

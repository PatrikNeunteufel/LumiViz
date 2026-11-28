# Changelog Session 224 (2026-11-27 bis 2026-11-28)

> Diese Sitzung hat die Saitenschleife der Rechenfassung 2 gebaut und in den Kern `pluckedString` gesetzt (34k-7d-4 (a),
> (b), (c), (e); Rechenfassung 1 aus IC ist entfernt), danach Quelle und Saite kausal gemacht (VL1 a–VL7 a) und die
> Tafel des Umlauf-Filters danach neu entworfen. Offen ist 34k-7d-4 (d), Hörort und Lautheit gegen den Bezug.

## Entscheide (Konzept 0.92.0, 0.93.0)

- **PK1 a:** Der Pegel der Stegkraft wird im Band 20 Hz – 20 kHz je Fenster geprüft (ohne Mittelwert, Hann-Fenster); über
  das ganze Band trägt ihn die statische Last unter 20 Hz, dort nur Meldung.
- **ZD1 a, ZD1-1 a, ZD2 a:** Stärke der Harfe in der Form der Gitarre, Neigung der Harfe 0 (an der VCSL-Konzertharfe
  gemessen: k −0,054 im Bereich −0,153 … +0,056); Laute und Oud mit dem Gesetz der Gitarre (Lücke in den Offenen Punkten).
- **ZN1 a:** Das Notenende ist ein Dämpfer an der Saite wie der Lautenzug, sein Leitwert aus `dampingTime` auf −60 dB.
- **VL1 a:** Vorlauf der Note im Kern 2 ms. **VL2 a:** Neigung je Stufe minimalphasig. **VL3 a:** FFT der Quelle fest
  2¹⁸ (s₀ · Dauer = 8), nur die erste Hälfte genutzt. **VL4 a, VL6 a:** zuerst die Ursachen, zuerst die Quellen.
  **VL5 a:** Bandgrenze der Quelle kausal (1 bis 20 kHz, darüber auf −120 dB). **VL7 a:** Verlust der Saite kausal
  (Kramers–Kronig), Zug und Biegesteife je Saite und Länge an den Teiltönen 1–30 angepasst.

## Neu

- `InstrumentCore` 0.10.0: Ausgleich (`Matrix`, `loese`, `Qr`, `kleinsteQuadrate`, `nnls`), aus `InstrumentMeasure`
  umgezogen, Bit für Bit.
- `InstrumentBody` 0.29.0: `KoerperAmSteg.hpp` — `stegAus(körper)` mit SR1 a.
- `InstrumentModels` 0.12.0: `zupf/Schleife` (Quelle per FFT, Umlauf-Filter, Steg, Dämpfer, Quellfilter kausal),
  `zupf/Zupf` und `zupf/ZupfKern` (Rechenfassung 2), `gemeinsam/Umlauf` mit `kausalerVerlust`, `kausalAngepasst`,
  `kausalerVerlustAuf` (Δk + i Im k auf der Geraden ω − i s₀ analytisch fortgesetzt), `wellenzahl(saite, länge, ω)`.
- Tests `test_InstrumentModelsZupf` (neu) und in `test_InstrumentModelsZupfBezug` die Fälle zu Schleife, Steg, VL1 a,
  VL3 a und VL7 a.
- Proben `docs/modellierung/gezupfte-saite/proben/2026-11-27_S224_*` und `2026-11-28_S224_laute38_kopplung` (`.py`,
  `.txt`).

## Geändert

- Bezug `proben/2026-11-26_S222_bezug_zupf.py` und `werte/zupf_pruefstand.txt`: Band je Fenster (PK1 a), die Stufe
  nicht mehr doppelt (GZ6 a), VL2 a, VL5 a, VL7 a mit Fortsetzung auf der Geraden.
- `werte/zupf_umlauf.json` nach VL7 a neu entworfen; `werte/zupf_ic.txt` mit den 45 Abdrücken von Rechenfassung 2.
- Entfernt: die Fassung aus IC (`rechneZupfTon`, `rechneZupfClip`, `rechneZupfSumme`, `delayFactor`, `staerkeBei`,
  `anschlagAus`, `harfeMitPedalen`; Journal J81).
- `tools/archiv_index.py`: Archiv seit S224 unter `<LW>:\archive\LumiViz\samples` und `…\quellen`;
  `docs/archiv/Index_samples.md`, `Index_quellen.md` neu.
- Konzept 0.93.0, Journal 0.22.0 (J81), Offene Punkte 1.435.0, Modul-Doku `InstrumentCore`, `InstrumentMeasure`,
  `InstrumentBody`, `InstrumentModels` 0.12.0.

## Befunde

- Der Fehler der Fortsetzung (Δk an |Re ω|) ließ den Bezug der Laute 38 rund 10 % langsamer abklingen (Band 1–3 s 0,64 dB
  über der Schleife); berichtigt 0,03 dB. Verstimmung im Chor, z₀ und Gruppenlaufzeit tragen es nicht (Probe
  `2026-11-28_S224_laute38_kopplung`).
- Harfe mit starrem Steg: 6 Tasten über 10 Cent (Taste 36 −20,8 Cent) — für 34k-7f.

## Verifikation

- Eng `*34k-7d*,*InstrumentModels 34d*,*Instrument Quellen*,*Instrument Stimmen*,*Instrument-Creator*,*34k-3*,*VL7 a*`
  47 / 47 unter Release-clang und Testing-MSVC; Abdrücke unter beiden Bit für Bit gleich.
- Die 17 Fälle des Bezugs über den Kern: Lage ≤ 0,93 Cent, Abklingen ≤ 5,8 %, Band ≤ 0,03 / 0,08 / 0,39 dB.
- Tafel: jede Stelle hält UL2 a außer Laute Chor 8.1 (bis 12,3 Cent).
- Kein voller Lauf; **1944 Cases**.

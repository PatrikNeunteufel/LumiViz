# Changelog Session 218 (2026-11-23)

> Diese Sitzung hat die Zielgrößen für Massen- und Strahlungsmatrix der Öffnungen der Harfe gerechnet (KQ71 a) und
> danach gebaut (KQ72 a): die Öffnungen werden aus ihrer Geometrie gerechnet, als Kolben in einer Wand. Mündungskorrektur
> und die beiden Güten der Luft (4,4 / 28) entfallen, an der Luft steht kein angepasster Wert mehr. Damit hält die Harfe
> an den Tasten 50–62 erstmals GZ14 a (größter Sprung 11,6 LU).

## Neu

- **KQ72 a (löst KQ63 a und KQ64 a ab):** Öffnungen aus ihrer Geometrie, Kolben in einer Wand — Massenmatrix (die
  Öffnungen belasten sich gegenseitig), Strahlungswiderstand als Matrix R_ij = ρ c k² / 2π · sin(k d) / (k d), Schall an
  den Hörorten √2 · ρ / 4π (die Leistung passt zum Widerstand), innerer Verlust aus Bells Güten mit nur Öffnung 6 offen
  abzüglich deren Abstrahlung.
- **`.lvinstbodyplan` Fassung 11:** `cavity.openingModel` (`endCorrection` / `geometry`), `cavity.surroundings` (`wall` /
  `free`), `cavity.innerLoss` (`openings`, `points` mit `f` / `q`, `source`), je Öffnung `length`; immer geschrieben,
  Fassung 10 liest sich mit Mündungskorrektur.
- **`.lvinstbody` Fassung 8:** je Mode `airLoss` (η = (1 − airShare) · η der Platte + airLoss); der Wächter rechnet die
  Dämpfung damit und prüft ihn ohne Geometrie gegen die Anteile.
- `InstrumentBody` 0.28.0: in `Luftraum.hpp` `oeffnungsIntegrale`, `oeffnungsMassen` (Rechtecke in 5-mm-Zellen,
  Selbstterm geschlossen), Strahlungswiderstand, `rohrGroessen` (innerer Verlust), Pfropfen mit Massenmatrix über
  Cholesky gekoppelt, Druck über die Länge der Öffnung gemittelt; der eine Druck und die Mündung rechnen Bit für Bit wie
  vorher. `InstrumentBodyTool` meldet bei Geometrie keine Helmholtz-Frequenz aus der Mündung.
- Tests (zwei neue Fälle, 1930 Cases): in `test_LoopKoerperRohr` „KQ71 … Integrale, Massen, Abstrahlung“, „KQ71 … im
  Bauplan und in der Datei“ und „KQ71 die Luft der mitgelieferten Harfe gegen Bell 1988 Kap. 4“ (alle 36 Lagen,
  rms < 6,5 %, Güten nur Öffnung 6 auf 10 %, Helmholtz 190 ± 3 Hz).
- Probe `modellierung/gezupfte-saite/proben/2026-11-23_S218_harfe_matrix_koerper.py` (`rohr`, `guete`, `schreibe … wand|frei`).

## Geändert

- Harfe (`make_harfe.py`): Öffnungen mit Länge, `geometry`, `wall`, innerer Verlust aus Bell mit nur Öffnung 6 offen;
  Körper neu. Der Fall „Luftraum die mitgelieferte Harfe“ misst ohne Saiten gegen Bell Kasten G in der Harfe ohne Saiten
  (KQ70 a) und besaitet nach KQ57 a, neue bekannte Abweichungen.
- Alle Baupläne auf Fassung 11 (die fünf übrigen `make_*.py`), ihre Körper neu gerechnet (1 h 54 min) — gleich bis auf
  Fassung, Abdruck und Rechenzeit; die Gitarre in `make_gitarre.py` auf Fassung 8 und in der Datei um `airLoss` ergänzt.
- Harfenbericht `2026-11-13_S197_harfe_gelesen.md` §26, `InstrumentBody.md` 0.28.0, README der Körper,
  `docs/audio/Instrumente_IC_Konzept.md` 0.87.0, `docs/Offene_Punkte.md` 1.429.0.

## Zahlen

| | Wand (gebaut) | freier Raum | gebaut S215 |
|---|---|---|---|
| Bells 36 Lagen, rms | 6,1 % | 7,4 % | 7,2 % |
| Helmholtz (Bell 190 Hz) | 192 Hz | 209 Hz | 190 Hz (angepasst) |
| Güten Luft alle offen (Bell 6 / 6 / 7 / 10 / 15 / 30) | 4,7 / 5,9 / 7,8 / 13,4 / 21,6 / 27,8 | 6,0 / 7,9 / 10,6 / 16,6 / 24,5 / 30,5 | 4,4 / 28 gesetzt |
| Paar besaitet (Bell 187 / 207 Hz) | −6,8 / +5,1 % | −1,8 / +8,0 % | −7,5 / +4,1 % |
| Abstrahlung 33–22, größte Abweichung | 14,6 dB | 12,9 dB | 10,9 dB |
| Lautheit 50–62 mit Mitschwingen, Spanne / größter / mittlerer | 11,6 / 11,6 / 2,6 LU | 13,1 / 12,3 / 3,4 | 13,8 / 13,8 / 2,9 |

Gebauter Körper ohne Saiten 165,2 / 210,7 / 232,9 / 319,7 / 434,0 / 542,7 Hz gegen Bell G 170 / 216 / 249 / 316 / 432 /
545 Hz; besaitet 174,2 / 217,4 / 249,3 / 328,1 / 438,2 / 607,1 / 731,0 Hz.

## Prüfung

`*Loop Koerper*,*Loop Werte*,*Klangfassung*,*Hammer*` ohne `*KQ18 Drehfeder*,*KQ20*`: Release-clang 118 / 118,
Testing-MSVC 118 / 118; kein voller Lauf.

## Offen

- KQ70 a: Wirkung des Besaitens (Bell 187 / 207 Hz besaitet, Modell 174 / 217 Hz), mit Fable.
- Luftmoden A3–A5 bleiben 6–10 % zu tief; Abstrahlung an Saite 29.

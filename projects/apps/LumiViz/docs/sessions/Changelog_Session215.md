# Changelog Session 215 (2026-11-23)

> Diese Sitzung hat KQ59 a gebaut: `InstrumentBody` rechnet die Luft der Harfe als Rohr längs des Kastens, mit einem
> Luftpfropfen je Öffnung an ihrem Ort. Neu sind Bauplan-Fassung 10, Körperdatei-Fassung 7 und ein Wächter gegen
> Bell 1988 Kap. 4. Alle Körper sind neu gerechnet. Am gebauten Körper tragen die Plattenmoden der Harfe zu viel Luft;
> der größte Sprung der Lautheit liegt bei 13,8 LU und ist bekannte Abweichung (KQ67 a).

## Neu

- **Das Rohr in `InstrumentBody/Luftraum.hpp`** (KQ59 a, KQ62 a, KQ65 a): Längsmoden eines beidseitig geschlossenen
  Rohrs mit veränderlichem Querschnitt (finite Volumen längs y, 20 Zellen je Wellenlänge an der oberen Grenze, 64–600
  Zellen), alle bis zur oberen Grenze des Fensters; je Loch und Öffnung ein Pfropfen; gekoppelt an die Plattenmoden über
  das Volumen je Länge (`volumenLaengs` in `Auswertung.hpp`) und dicht zerlegt mit `kern::zerlegeSymmetrisch`. Moden
  unter 1 Hz (Durchfluss ohne Druck) fallen weg. `mitLuftraum(…, bisHz)` zweigt nach `cavity.kind`; der eine Druck
  rechnet Bit für Bit wie vorher.
- **Bauplan-Fassung 10** (KQ61 a): `cavity.kind` (`singlePressure` / `tube`) und `cavity.tube` (`sections` mit `y` und
  `area`, `q`, `source`), immer geschrieben; eine Datei der Fassung 9 liest sich als ein Druck, ein unbekanntes Wort
  macht sie unlesbar. `pruefe` verlangt mindestens zwei Querschnitte, aufsteigendes y, das Volumen auf 1 % und die
  Öffnungen im Rohr.
- **Körperdatei Fassung 7** (KQ63 a): je Mode `tubeShare`, der Anteil der Bewegungsenergie in der Luft längs. Die
  Dämpfung ist η = (1 − Luft) · η_Platte + (Luft − Rohr) / Q_Öffnung + Rohr / Q_Rohr; der Wächter `*31c-5d*` rechnet
  sie damit nach.
- **Statistischer Teil** (KQ66 a): zählt nur Moden mit weniger als halber Energie in der Luft
  (`kStatistikLuftGrenze` 0,5).
- `InstrumentBody` 0.27.0 hängt an `InstrumentCore` (`Solution.json`: Bibliothek nach ihrer Abhängigkeit).
- `InstrumentBodyTool` nennt das Rohr und den Anteil längs je Mode.
- Test `test_LoopKoerperRohr` (vier Fälle): gerades Rohr gegen n c / 2L, Rohr ohne Längsmoden gleich dem einen Druck
  auf 1e-9, Rohr im Bauplan und in der Datei, die Luft der mitgelieferten Harfe gegen Bell Kap. 4 (KQ64 a, bekannte
  Abweichungen mit Wert).
- Proben `modellierung/gezupfte-saite/proben/2026-11-23_S215_harfe_rohr_wahl.py` (`rohr`, `profile`, `volumen`,
  `gekoppelt`) und `…_harfe_abstrahlung_q.py`.

## Geändert

- **Die Harfe** (`make_harfe.py`): Luft als Rohr, Querschnitt ~ Breite² nach Bell Abb. 2.7 auf 27 l, Mündung 1,83
  (Helmholtz 190 Hz, KQ60 a), Güten 4,4 in den Öffnungen und 28 längs (KQ63 a); 167 Moden.
- Fall „Luftraum die mitgelieferte Harfe“: Lagen gegen Bell besaitet (KQ57 a), ohne Saiten Bodenmode bei
  verschlossenen Öffnungen gegen Helmholtz auf 3 %, Abstrahlung 33–22 mit Saiten als Dämpfer gegen den Median der
  zwölf Abweichungen, Bauch von S(1,1)−; neue bekannte Abweichungen mit Wert.
- Die übrigen sechs Baupläne auf Fassung 10 (`singlePressure`), ihre Körper neu gerechnet, gleich bis auf Fassung,
  Abdruck, Rechenzeit und `tubeShare` 0; die Gitarre in `make_gitarre.py` auf Fassung 7 und in der Datei ergänzt (nicht
  neu gefittet).
- Harfenbericht §23, Konzept 0.84.0 (KQ61 a–KQ67 a), Offene Punkte 1.426.0, `InstrumentBody.md` 0.27.0, README der
  Körper.

## Zahlen

| Am gebauten Körper der Harfe | Wert | Ziel |
|---|---|---|
| Rohr allein, nur Öffnung 6: A1–A5 | −0,3 / +0,1 / −2,0 / −2,5 / −3,2 % | 3 % |
| Rohr allein, alle offen: Helmholtz, Abfall zum Diskant | 190,1 Hz, 18,9 dB | 190 ± 1,5 Hz, rund 18 dB |
| Rohr allein, alle offen: A2–A5 | −11,5 / −13,2 / −8,9 / −5,9 % | bekannte Abweichung, Ursache offen |
| besaitet: Lagen gegen Bell | −7,5 / +4,2 / −4,0 / −1,9 / +0,5 / +8,8 / +5,0 % | 7 % |
| Abstrahlung 33–22 | 4 / 12, größte 11,4 dB | 12 / 12 auf 6 dB |
| Q ohne Saiten: Paar, S(1,2)–S(1,5) | 8 / 10, 17, 13, 17, 23 | Bell 11 / 16, 39, 45, 43, 31 |
| Tasten 50–62 mit Mitschwingen | 13,8 / 13,8 / 2,9 LU | größter ≤ 12 LU |

Mit einer Güte 13,4 (KQ63 b) läge der größte Sprung bei 10,5 LU, die Luft allein aber auf den Faktor 1,9 statt 1,3
neben Bells Güten (Helmholtz Q 13 statt 6) — sie gliche die zu viele Luft in den Plattenmoden aus. Gewählt KQ67 a.

## Tests

`*Loop Koerper*,*Loop Werte*,*Klangfassung*,*Hammer*` ohne `*KQ18 Drehfeder*,*KQ20*` 116 / 116 unter Release-clang
(399 s) und Testing-MSVC (473 s); kein voller Lauf. 1928 Cases.

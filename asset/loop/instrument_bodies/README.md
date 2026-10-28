# Resonanzkörper von Instrumenten

Hier liegen die **Körper**, die Instrumente benutzen: der Resonanzboden eines Flügels, später die Decke einer
Gitarre oder der Korpus einer Violine. Je Körper zwei Dateien (Entscheide Patrik S147: BM3 a, BM3-1 a, BM3-2 a):

| Datei | Was | Wer schreibt sie |
|---|---|---|
| `<name>.lvinstbodyplan` | der **Bauplan**: Umriss, Platte mit Dicke, Rippen, Stege, Materialien, die Punkte, an denen die Saiten den Steg treffen | von Hand oder ein `make_*.py` hier |
| `<name>.lvinstbody` | der **Körper**: die daraus gerechneten Moden (Frequenz, Dämpfung, Masse je Stegpunkt, Abstrahlung) und der Abdruck des Bauplans | das Werkzeug `InstrumentBodyTool` (ab Stufe 31c-4/31c-5) |

Ein Instrument nennt den Körper beim Namen: `"soundboard": {"body": "lib:steinway_d", …}` in der `.lvinst`. Das
Programm liest nur Körper, es rechnet nie (BM5 a). Ein Körper wird committet.

Einheiten SI (m, kg/m³, Pa). Was ein Bauplan ist, beschreibt
`projects/libs/InstrumentBody/include/InstrumentBody/InstrumentBody.md`; das Steuerdokument ist
`projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1.

## Inhalt

| Name | Instrument | Quelle | Erzeugt von |
|---|---|---|---|
| `steinway_d` | Steinway D, Resonanzboden | Chabassier & Duruflé, RT-0425 (2012), Abb. 2 und Anhang A; Holz nach Chabassier, Joly & Chaigne 2013, Tab. I; Breiten nach MAESSTRO Tab. 1 | Bauplan: `make_steinway_d.py <RT-0425.pdf>` (S147, 31c-1; Hörorte S148, 31c-5a). Körper: `InstrumentBodyTool steinway_d.lvinstbodyplan --to 5000` (S148, 31c-5d) |
| `steinway_d_glass` | gedachtes Instrument (GB1 a, GL1 a): der Steinway D mit einer Platte aus Kalk-Natron-Floatglas, Dicke wie Holz (6–10 mm); Rippen Fichte, Steg Buche | wie `steinway_d`; Glas nach Datenblättern zu EN 572 (NGA FM05-12 als Gegenwert), η 1·10⁻³ nach Cremer/Heckl über Irvine 2010 Tab. 7 | Bauplan: derselbe Lauf von `make_steinway_d.py` (S151, 31c-8). Körper: `InstrumentBodyTool … --to 5000` (392 Moden, 2 min) |
| `steinway_d_glass_same_mass` | wie oben, die Platte auf die Masse der Platte aus Holz (Dicke mal 380/2500, 0,9–1,5 mm) | wie oben | wie oben (1985 Moden, 1,3 h, Spitze 9,4 GB) |

Die Glasböden sind gedachte Instrumente: ob das Glas unter der Saitenlast bricht, ist nicht gerechnet; in der
Dämpfung fehlen Rand und Abstrahlung (DM1 a) — für Holz stecken sie im Gesetz von RT-0425, für Glas nicht.

`make_steinway_d.py` liest die PDF (nur lokal unter `projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/`),
wertet Abb. 2 aus und schreibt den Bauplan des Steinway und der beiden Glasböden. **Maßgebend ist die Datei im Repo**: ein neuer Lauf zuerst mit
`--ziel <ordner>` in eine Kopie, `--bild <datei.png>` legt ein Kontrollbild ab (Umriss, Rippen, Steg und
Stegpunkte über Abb. 2 (d)).

**Fassung 3 der Baupläne (S160):** der Block `damping` trägt das Dämpfungsgesetz α(f) = a·f² + b·f + c mit Quelle
(HU4-1 a: c = 4,5 1/s hebt die tiefsten Moden auf die gemessenen 1–3 %, Ege, Boutillon & Rébillat 2013; mit c = 0
nach RT-0425 lagen sie bei 1,1 %), der Block `backside` die Rückseite der Platte (HU4 a: aus — an den Hörorten
12 cm über den Saiten wirkt der akustische Kurzschluss nicht, Befund HU4 im Spielweisen_Konzept §4.1; für Hörorte im
Fernfeld vorgesehen). `make_steinway_d.py` schreibt beide; Fassung 2 ohne die Blöcke bleibt lesbar.

**Ein Körper wird neu gerechnet**, wenn sich sein Bauplan ändert oder die Rechnung (`kRechenFassung` in
`projects/libs/InstrumentBody/include/InstrumentBody/Koerper.hpp`) — der Test „Loop Koerper 31c-5d Waechter“ meldet es.
Neu rechnen mit dem Werkzeug (Release-clang, rund 22 Minuten für den Steinway):

```
out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe asset/loop/instrument_bodies/steinway_d.lvinstbodyplan --to 5000
```

Es schreibt `steinway_d.lvinstbody` daneben. In `asset/Asset.cmake` (Kopie neben die Exe) gehen nur die Körper mit,
nicht die Baupläne — das Programm liest nur Körper (BM5 a).

## Prüffall MAESSTRO (Testdaten, nicht mitgeliefert)

`make_steinway_d.py <RT-0425.pdf> --maesstro <MAESSTRO.pdf>` schreibt den Prüfstein von MAESSTRO (Elie, Cotte &
Boutillon 2022, „reference piano“) nach `projects/apps/LumiViz/tests/unit/UnitTests/koerper/maesstro_rp.lvinstbodyplan`
(S150, PF1 a, PF2 a): der Steinway D mit den Vereinfachungen aus MAESSTRO Tab. 1–2 und zwei getrennten Stegen
(Tasten 21–40 und 41–108). `--bild` legt ihn auf Abb. 4 (a) von MAESSTRO. Der Test „Loop Koerper 31c-6 MAESSTRO
Prueffall“ prüft die Datei. Der Körper liegt daneben (`maesstro_rp.lvinstbody`, S150, k = 3 bis 5 kHz, rund 15 min):

```
out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe projects/apps/LumiViz/tests/unit/UnitTests/koerper/maesstro_rp.lvinstbodyplan --to 5000
```

Derselbe Lauf schreibt die drei Varianten aus MAESSTRO §3.2 daneben (S151, 31c-8, VA1 a): `maesstro_mp1` (Platte
16 mm), `maesstro_mp2` (doppelter Rippenabstand: Rippe 1, 3, …, 17, wie Abb. 4 b — `--bild` legt sie zusätzlich auf
Abb. 4 b, `<bild>_mp2.png`), `maesstro_mp3` (ohne Rippen). Ihre Körper liegen ebenfalls unter den Testdaten (gerechnet
wie RP, 5–17 min je Körper); der Test „Loop Koerper 31c-8 Richtungen MAESSTRO (VA1 a)“ prüft sie gegen RP.

# LV-5 — Pegel der Hörlisten: warum IC-Klänge abseits liegen (S186, 2026-11-10)

> **Anlass:** Befund HT 2 (`sync_hoertest/inbox/HT-20261101-0845-…md` Punkt 2), Übergabe
> `ic/UEBERGABE_LUMIVIZ_HOERTEST.md` §3.2 und §4 Schritt 4. **Ergebnis:** die Messungen von IC und HT sind für Klänge
> ab 0,4 s **gleich**; die Klänge liegen wirklich abseits. Ursache ist das **Angleichen in einem Schritt** bei IC: das
> absolute Gate (−70 LUFS) macht die integrierte Lautheit leiser Rohtöne vom Maßstab abhängig. Die LumiViz-Listen haben
> den Fehler nicht (sie regeln nach). Entschieden PN1 a, PN2 a, KZ1 a (§5), umgesetzt (§6).

## 1 Die drei Messungen

| | Blöcke | Gate | unter 0,4 s | Quelle |
|---|---|---|---|---|
| IC `lautheit` | 400 ms, Schritt 100 ms, ab Wert 0 | −70 absolut, −10 relativ | Mittel über den ganzen Klang ohne Gate | `harvest/instruments-calculation/rechnung/gemeinsam/Werkzeug.cpp:263–303` [gelesen] |
| HT `integrierteLautheit` | Stücke zu 100 ms, je vier zu einem Block (gleich); ein letztes Teilstück fällt weg | gleich | „zu kurz oder zu leise“ (kein Wert) | `HoerTest_Helper/projects/exec/Hoertest/src/Lautheit.cpp` [gelesen] |
| LV `lumi::audio::messeLautheit` | gleich | gleich; relatives Gate nicht unter −70 | — (Hörlisten von LV halten 2 s) | `projects/libs/AudioCore/src/Lautheit.cpp:157–162` [gelesen] |

Für Mono-Klänge ab 0,4 s rechnen alle drei dasselbe. Nachgemessen mit einem eigenen Skript (BS.1770-4, dieselben
Konstanten des K-Filters; Scratchpad S186 `lv5_messen.py`) an den Dateien am Hörort
`…\cmake\samples\Hoerproben\IC\<liste>\`: **HT und Skript stimmen auf 0,01 LU** (Upright C4 moderator −41,68, Honky-tonk
C4 moderator −42,06, Cimbalom Dis4 a-30 −41,24, Vibraphon D3 a-30 −32,26, Bassdrum e0 −36,57 LUFS). [gemessen]

## 2 Ursache: Angleichen in einem Schritt über das absolute Gate

IC misst den **Rohton** (`Sammlung.cpp:157`, `lufsVorher`) und multipliziert dann mit
`10^((ziel − lufsVorher)/20)` (`:183`). Bei leisen Rohtönen fällt der Ausklang unter −70 LUFS und zählt nicht; nach
dem Verstärken liegt er darüber, zählt mit und zieht die Lautheit herunter. Beleg: rechnet man die Datei um
`lufsVorher − ziel` zurück, kommt `lufsVorher` genau heraus — mit viel weniger Blöcken über −70: [gemessen]

| Klang | lufs_vorher | Blöcke über −70: Rohton → Datei | Datei gegen Pegel |
|---|---|---|---|
| 01 Upright C4 moderator | −58,21 | 1 → 18 (von 20) | −8,38 LU |
| 01 Cimbalom Dis4 a-30 | −64,27 | 1 → 10 (von 10) | −7,94 LU |
| 04 Vibraphon D3 a-30 | −55,04 | 14 → 54 (von 57) | −2,86 LU |
| 04 Kalimba Gis4 a-30 | −60,17 | 4 → 15 (von 18) | −1,61 LU |
| 05 Bassdrum e0 | −52,79 | 2 → 14 (von 17) | −5,77 LU |
| 05 Tabla rand | −51,95 | 4 → 12 (von 17) | −1,20 LU |
| Gegenprobe 01 Upright C4 a0 | −27,82 | 20 → 20 | 0,00 LU |
| Gegenprobe 04 Vibraphon D3 a0 | −25,64 | 57 → 57 | 0,00 LU |

Die Vermutung der Übergabe („Blockgrenze oder relatives Gate“) trifft nicht: Blöcke und relatives Gate sind gleich,
es ist das absolute Gate am anderen Maßstab. Die Abweichung ist umso größer, je leiser der Rohton und je länger der
leise Ausklang.

## 3 Bestand über alle elf IC-Listen

`Hoertest --liste-pruefen` je Liste (alle gültig, 0 Fehler) und `lufs_vorher = −300` aus den `liste.json`: [gemessen]

| Liste | Klänge | abseits (> ±1 LU) | still (Rohton ganz unter −70, nicht angeglichen) | unter 0,4 s |
|---|---|---|---|---|
| 01 Hammer–Saite | 70 | 5 | 1 (Flügel Port C6 a-30) | 0 |
| 04 Platte–Stab | 108 | 6 | 8 (Glockenspiel A6 a-30, Celesta C6/Ais7 a-30, Kalimba E3 a-30, Rhodes D7 a-30, Wurlitzer E4 a-30, Ais6 a-30/a-12) | 9 (Marimba Ais6, Xylophon D6, Ais7 je drei Stärken, 0,31–0,33 s) |
| 05 Membran | 53 | 6 | 0 | 0 |
| 07 Lippe–Rohr | 39 | 1 (Trompete Gis4 e0) | 2 (Trompete A3 e0, Fis5 e0) | 0 |
| 02, 03, 06, 08–11 | 322 | 0 | 0 | 0 |

Die stillen Klänge liegen in der Datei bei −80 bis −86 LUFS (Celesta C6 a-30: −80,4, Wurlitzer E4 a-30: −85,6 LUFS ohne
Gate) — beim Hören nicht zu hören.

## 4 Die Listen von LumiViz

`test_LoopStab.cpp:324–341` (ebenso Klasse 1 und Trommel) **regelt nach**: verstärken, neu messen, bis 0,05 LU am Ziel —
der Kommentar dort nennt genau diese Ursache. HT über die drei Listen am Hörort `…\Hoerproben\LumiViz\`: [gemessen]

- `klasse1_34i` −35,3, `membran_34h` −30,9, `stab_34g` −26,6 LUFS, **0 außerhalb**.
- `stab_34g`: **5 still** — `xylophone_103_a-30` (−85,7 LUFS), `glockenspiel_93_a-30` (−83,0), `celesta_103_a-30` (−120,5),
  `tine_electric_piano_95_a-30` (−102,3), `reed_electric_piano_91_a-30` (−81,4). Bei a0 hat `xylophone_103` −21,2 LUFS roh,
  bei a-30 −85,7 (roh, nicht angeglichen): rund 64 dB für 30 dB Stärke. [gemessen; Ursache am Modell nicht gesucht]

## 5 Entscheide (Patrik S186: „alle a“)

- **PN1 a — die IC-Listen nachregeln:** jeder Klang verstärken, neu messen, bis 0,05 LU vom Pegel; der Pegel sinkt, wenn sonst
  eine Spitze über 0,99 käme. Werkzeug `tools/hoertest/ic_nachregeln.py` (numpy/scipy, `--trocken`, `--repo`); die Klänge
  waren ungehört (L5), kein Urteil wird entwertet.
- **PN2 a — stille Klänge kommen nicht in eine Liste:** der Schreiber meldet sie, sie sind ein Befund am Modell
  (`Offene_Punkte.md` §6, Hörtest). Spricht das Modell an, sind sie beim nächsten Schreiben wieder dabei. Gegen b (ohne
  Gate angleichen, +50 bis +94 dB): im Programm wären die Anschläge stumm, verstärkt hört man vor allem den Rest der Rechnung.
- **KZ1 a — ein Klang einer Liste ist mindestens 0,4 s lang:** der Schreiber füllt hinten mit Stille auf; dann misst
  BS.1770-4 jeden Klang gleich, ohne Sonderregel. Vorschlag an HT: `--liste-pruefen` meldet einen kürzeren als Fehler
  (heute Hinweis „zu kurz oder zu leise“). Kipppunkt: spielt HT Klänge nahtlos oder überblendet, wäre die Stille hörbar.

## 6 Umgesetzt (S186)

| Liste | Pegel vorher → nachher | neu geschrieben | heraus (PN2 a) | aufgefüllt (KZ1 a) |
|---|---|---|---|---|
| IC 01 | −33,3 → −34,7 LUFS | 69 von 69 | 1 (Punkt p009) | — |
| IC 02 | −25,1 → −25,6 | 52 von 56 (lagen 0,05–1 LU daneben) | — | — |
| IC 04 | −29,4 | 37 von 100 | 8 (Punkte p041, p062, p065, p070, p085, p092, p095, p096) | 9 |
| IC 05 | −30,8 → −33,0 | 53 von 53 | — | — |
| IC 07 | −28,2 | 1 von 37 | 2 (Punkte p027, p033) | — |
| IC 03, 06, 08–11 | gleich | Byte für Byte unberührt | — | — |
| LV `stab_34g` | −26,6 | 0 (die übrigen 91 Byte für Byte gleich) | 5 | — |
| LV `klasse1_34i`, `membran_34h` | gleich | Byte für Byte gleich | — | — |

Prüfung: `Hoertest --liste-pruefen` über alle 14 Listen gültig, **0 außerhalb, 0 nicht messbar**; übrig nur die Hinweise
auf gleiche Abdrücke (LV-6). Zweiter Lauf des Werkzeugs: „nichts zu tun“ für alle elf. Unveränderte Klänge Byte für Byte
gleich (Abdrücke verglichen). Wächter `Hoerlisten AU3-1 a` grün. Die fünf alten IC-Listen liegen gesichert unter
`…\cmake\samples\Hoerproben\IC_vor_S186\` (darf weg, sobald die neuen abgenommen sind). Die Schreiber von LumiViz teilen
seit S186 `tests/unit/UnitTests/HoerlistePegel.hpp` (`gleicheAn`, `ohneStille`, `schreibeKlaenge`) — vorher stand dieselbe
Pegelregel dreimal da, bei Stab und Klasse 1 ohne die Runde, die den Pegel bei zu hoher Spitze senkt.

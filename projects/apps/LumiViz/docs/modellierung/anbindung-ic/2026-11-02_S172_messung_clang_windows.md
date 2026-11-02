# Messung: IC `rechnung/` unter Release-clang (Windows) — Abdrücke und Rechenzeit

> **Datum:** 2026-11-02 · **Sitzung:** LumiViz S172 · **Entscheid:** M1 a (Patrik S172: hier messen, nur lesend und
> bauend, nichts im IC-Repo ändern) · **Zweck:** Zahlen für IK2-5 (Gleitkomma-Schalter, Abdrücke je Compiler) und
> IK2-6 (Rechenzeitgrenze); Antwort auf IC-A9 Schritt 1 und IC-A10 Schritt 1 der Prüfung S166.
> **Stand der Quellen:** IC `master` `bb7e6d6` plus der uncommittete Stand von IC Session 5 (31.10.) im Arbeitsbaum;
> LumiViz `ebf7077d`.

## 1. Umgebung

| Größe | Wert |
|---|---|
| Umgebung | `vcvars64.bat` von Visual Studio 18 (wie `tools\bau_clang.bat` in LumiViz) |
| Compiler | `C:\Program Files\LLVM\bin\clang++.exe`, **clang 20.1.7**, Ziel `x86_64-pc-windows-msvc` (Laufzeit UCRT) |
| Generator / Art | Ninja, `CMAKE_BUILD_TYPE=Release` |
| Schalter | die von IC `rechnung/CMakeLists.txt` Z. 19–23: `-Wall -Wextra -Wpedantic -ffp-contract=off -fno-fast-math` (der Nicht-MSVC-Zweig greift, weil clang nicht als `MSVC` zählt) |
| Bauordner | `rechnung/out-clang` (im IC-Repo ignoriert, `.gitignore` Z. 378) |
| Bau | 84 Schritte, 29 Warnungen, rund zwei Minuten mit 8 Aufträgen |
| Rechner | Patriks Arbeitsrechner; kein Libre Hardware Monitor erreichbar (`localhost:8085` leer), Taktprobe deshalb mit einer festen Rechenlast auf einem Thread (`taktprobe.py` im Scratchpad: zwölf Durchläufe, Urteil „ruhig“ bei max/min < 1,15) |
| Werkzeuge | `ic_tests.exe --no-colors` (voller Lauf), `-tc=*Rechenzeit*` (drei Läufe allein) |

## 2. Voller Lauf: 125 Fälle, 115 grün, 10 rot — alle roten Prüfungen sind Abdrücke

| Zählung | Wert |
|---|---|
| Testfälle | 125, davon 115 bestanden, 10 nicht |
| Prüfungen | 1503, davon 1474 bestanden, 29 nicht |
| rote Prüfungen außer `erwarteterAbdruck` | **keine** |
| Warnungen (`WARN`, nicht rot) | drei beim Alphorn (Spitzen 4–6 um +99/+65/+45 Cent über der Reihe; in IC Klasse 7 `offen.md` geführt), eine bei der Klarinette n62 e0,5 (mehrstabil, `offen.md`) |

### 2.1 Abdrücke: Windows-clang gleich MSVC in allen 29 Fällen

Vergleich der 29 abweichenden Abdrücke (`ist`, Windows-clang) mit `werte/abdruck_msvc.txt` und `werte/abdruck_gcc.txt`
(`abdruck_vergleich.py` im Scratchpad; `abdruck_clang.txt` stammt aus Linux, clang 18.1):

| Gruppe | Einträge | gleich Linux-clang | gleich MSVC (Windows) |
|---|---:|---:|---:|
| abweichend von `abdruck_clang.txt` | 29 | 0 | **29** |
| gleich `abdruck_clang.txt` | 13 | 13 | — (dort sind gcc, clang und MSVC ohnehin gleich) |

Die 29: alle Klasse 1 (`klasse1_*` sechs, `hammer_vorgabe_*` fünf, `saite_Hackbrett_clip_C4_G4`), `zupf_Gitarre_m52_mf`,
`stab_*` zehn (alle außer Kalimba), `trommel_Tom_mf`, `trommel_Pauke_m52_mf`, `becken_Crash_e80`,
`rohrblatt_Klarinette_m60_mf`, `rohrblatt_Altsaxophon_m70_mf`, `luftstrahl_Blockfloete_m74_mf`. Die 13 gleichen: Blech
(drei), Bogen (vier), Labialpfeife, Kalimba, Orgel, Akkordeon, Cembalo, Harfen-Clip — dieselbe Menge, die IC Session 5
für MSVC gegen Linux fand.

**Deutung:** Die Abweichung hängt **an der Plattform** (Mathematikbibliothek der UCRT: `exp`, `sin`, `pow` …), nicht am
Compiler — zwei verschiedene Compiler auf Windows liefern dieselben Bits, derselbe Compiler auf Linux andere. Das ist
ein Befund an der Zuordnung der Abdruckdateien: `compilerName()` in `werkzeuge/TestHilfe.hpp` unterscheidet nur den
Compiler; für Windows-clang heißt die Datei deshalb `abdruck_clang.txt`, obwohl die Werte zu `abdruck_msvc.txt` gehören.
**Folge für LumiViz (IK2-5):** Es gibt auf Windows **einen** Bezug (MSVC = clang), die Abdrücke je Compiler, wie LumiViz
sie führt (`hammer_bezug_<compiler>.txt`, `loop_werte.txt` unter beiden Compilern gleich), bleiben ein Wächter je
Plattform, nicht je Compiler. Die Ursache je Ton (erster abweichender Abtastwert, beteiligte Funktion — IC-A9 Schritt 2)
ist damit nicht gesucht; nur eingegrenzt.

Nicht geprüft: ob die Abweichung im Rundungsrauschen bleibt (Pegel je Block). Der IC-Bezugstest `test_hammer_bezug`
(Pegel je Block gegen `hammer_bezug_*`) war im vollen Lauf grün — das sagt für den Flügel-Port, dass die Abweichung
unter seiner Toleranz liegt.

## 3. Rechenzeit: drei Läufe allein, je nach ruhiger Taktprobe

Taktprobe vorher: max/min 1,062 (ruhig); nachher: 1,056 (ruhig). Je Lauf `ic_tests -tc=*Rechenzeit*` (zwölf Fälle; die
sieben roten darin sind dieselben Abdrücke wie in §2, keine Rechenzeit-Prüfung war rot). Die Grenze „hier“ rechnet IC
als Flügel-Port × 5 / 4,6 (`echtzeitGrenzeHier`): im Lauf 1 3,41 %, Lauf 2 und 3 3,61 %.

| Instrument (tiefster Ton, Stärke wie im Test) | Lauf 1 | Lauf 2 | Lauf 3 | Einheit |
|---|---:|---:|---:|---|
| **Hammer-Saite A0 (Flügel-Port, Bezug)** | 3,14 | 3,38 | 3,32 | % der Echtzeit |
| Upright 21 | 1,74 | 1,88 | 1,76 | % |
| Honky-tonk 21 | 1,71 | 1,82 | 1,76 | % |
| CP-70 28 | 1,98 | 2,09 | 2,07 | % |
| Clavinet 29 | 0,60 | 0,64 | 0,62 | % |
| Hackbrett 43 (Deluxe) | 3,36 | 3,64 | 3,39 | % |
| Cimbalom 38 (Deluxe) | 4,67 | 5,34 | 4,63 | % |
| Gitarre E2 | 0,19 | 0,19 | 0,20 | % |
| Cello C2 ff | 1,13 | 1,16 | 1,17 | % |
| Marimba 36 | 0,29 | 0,27 | 0,27 | % |
| Xylophon 65 | 0,17 | 0,17 | 0,16 | % |
| Vibraphon 48 | 2,09 | 2,15 | 2,21 | % |
| Glockenspiel 79 | 0,09 | 0,09 | 0,09 | % |
| Röhrenglocken 48 | 0,25 | 0,25 | 0,25 | % |
| Celesta 60 | 0,16 | 0,16 | 0,16 | % |
| Kalimba 50 | 0,09 | 0,09 | 0,09 | % |
| Rhodes 28 | 0,19 | 0,18 | 0,18 | % |
| Wurlitzer 33 | 0,14 | 0,14 | 0,14 | % |
| Steeldrum 60 | 0,10 | 0,09 | 0,09 | % |
| Tom ff | 2,40 | 2,40 | 2,40 | % |
| Pauke ff | 2,25 | 2,26 | 2,33 | % |
| Crash (vorab, Grenze 10 ×) | 5,5 | 5,4 | 5,7 | × Echtzeit |
| Posaune tiefster Ton | 1,30 | 1,25 | 1,30 | % |
| Klarinette tiefster Ton | 3,10 | 3,23 | 3,18 | % |
| Labialpfeife C2 | 0,36 | 0,36 | 0,36 | % |
| Akkordeon Musette | 1,30 | 1,40 | 1,37 | % |
| Orgel, Akkord mit Rotary (Clip) | 1,23 | 1,25 | 1,24 | % |

**Einordnung gegen LumiViz:** Der Flügel in LumiViz (mit Stoß, Hysterese, Ebenen, Boden) lag von Hand gemessen bei
3,73–3,82 % (S156) und 4,35–4,60 % (S153) der Echtzeit an Taste 40; der IC-Port misst „Vorgabe des Vorbilds“ ohne diese
Teile und liegt bei 3,14–3,38 % an A0. Die Zahlen sind deshalb **nicht** direkt vergleichbar; absolut gilt für die
Klassen der Reihenfolge Z1:

| Klasse (Z1) | Höchstwert über drei Läufe | unter 4,6 % (RZ1-1 a)? | unter 5 % (AUFTRAG_BAU §4.3e)? |
|---|---:|---|---|
| 4 Platte/Stab | 2,21 % (Vibraphon) | ja | ja |
| 5 Membran | 2,40 % (Tom) | ja | ja |
| 1 Rest (ohne Deluxe) | 2,09 % (CP-70) | ja | ja |
| 10 Zunge | 1,40 % (Akkordeon) | ja | ja |
| 2 Zupf (je Note) | 0,20 % (Gitarre) | ja | ja |
| 1 Deluxe | 5,34 % (Cimbalom) | nein (Deluxe, F-1 a) | nein |

Die Streuung über die drei Läufe liegt bei höchstens 8 % des Werts (Vibraphon 2,09–2,21), beim Cimbalom 15 %
(4,63–5,34) — alle unter dem, was ein Drosseln ausmacht (S106: Faktor 8).

## 4. Folgen und Aufträge

- **IK2-5:** Ein Windows-Bezug für beide Compiler; die Schalter aus IC (`-ffp-contract=off`, `/fp:contract-`) sind für
  die Bitgleichheit zwischen clang und MSVC auf Windows offenbar hinreichend (dieselben Bits ohne gemeinsamen Compiler).
  Ob sie *nötig* sind, ist nicht gemessen (kein Bau ohne Schalter).
- **IK2-6:** Alle Klassen der Reihenfolge Z1 liegen unter 2,5 % — weit unter 4,6 % und 5 %. Die Wahl der Grenze
  entscheidet heute kein Instrument; nur die Deluxe-Fälle (Hackbrett, Cimbalom je Clip, Becken vorab) liegen darüber.
- **An IC (als Auftrag, Datei `pruefungen/LV-2026-11-02-clang-windows-messung.md` im IC-Repo):** IC-A9 Schritt 1 ist
  erledigt (Befund oben); Schritt 2 (Ursache je Ton) offen; Zuordnung der Abdruckdateien nach Plattform statt Compiler;
  IC-A10 Schritt 1 erledigt (diese Tabelle), Schritt 2 (Eintrag in `BERICHT_BAU.md`/`KLASSEN.md`) bei IC.

## 5. Reichweite

Gemessen wurde nur unter Windows-clang 20.1.7 im Release-Bau von IC mit dessen Schaltern; nicht unter Testing-MSVC,
nicht unter den CMakeCraft-Schaltern von LumiViz, nicht ohne Gleitkomma-Schalter. Die Rechenzeit gilt für den tiefsten
Ton je Instrument mit der Dauer, die der jeweilige Test ansetzt (meist 2 s), auf einem Thread. Das Innere der Kerne
wurde nicht gelesen. Logs im Scratchpad der Sitzung (`ic_tests_clang_lauf1.log`, `rz_lauf1..3.log`, `rz_takt_*.log`).

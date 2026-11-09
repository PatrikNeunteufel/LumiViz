# Messung aus LumiViz S172: `rechnung/` unter Windows-clang — Antwort auf IC-A9 und IC-A10

> **Von:** LumiViz, Session 172 (2026-11-02), Entscheid Patrik M1 a (Messung in der LumiViz-Sitzung, nur lesend und
> bauend; Bauordner `rechnung/out-clang`, ignoriert). **Vollständiger Bericht mit allen Zahlen:** LumiViz
> `projects/apps/LumiViz/docs/modellierung/anbindung-ic/2026-11-02_S172_messung_clang_windows.md`.
> **Gemessener Stand:** `master` `bb7e6d6` plus der uncommittete Arbeitsbaum von IC Session 5 (31.10.).
> **Bezug:** [LV-2026-10-30-erster-baudurchgang.md](LV-2026-10-30-erster-baudurchgang.md) IC-A9, IC-A10.

## Umgebung

clang 20.1.7 (`C:\Program Files\LLVM\bin`), Ziel `x86_64-pc-windows-msvc`, in der Umgebung von Visual Studio 18
(`vcvars64.bat`), Ninja, Release, mit den Schaltern aus `rechnung/CMakeLists.txt` (Nicht-MSVC-Zweig:
`-ffp-contract=off -fno-fast-math`). 84 Bauschritte, 29 Warnungen.

## IC-A9 Schritt 1 — erledigt: Windows-clang ist bitgleich mit MSVC

Voller Lauf `ic_tests`: 125 Fälle, 115 grün, 10 rot; **alle 29 roten Prüfungen sind Abdruck-Vergleiche gegen
`werte/abdruck_clang.txt`** (das aus Linux, clang 18.1, stammt). Nichts anderes war rot; drei `WARN` beim Alphorn und
eine bei der Klarinette wie in `offen.md`.

| Abdrücke | Zahl | gleich Linux-clang | gleich MSVC |
|---|---:|---:|---:|
| abweichend von `abdruck_clang.txt` | 29 | 0 | **29** |
| gleich `abdruck_clang.txt` | 13 | 13 | 13 (dort stimmen alle drei Dateien überein) |

Die Mengen sind dieselben wie bei MSVC gegen Linux (IC Session 5). **Deutung:** Die Abweichung hängt an der Plattform
(Mathematikbibliothek der UCRT), nicht am Compiler. Zwei Compiler auf Windows liefern dieselben Bits, derselbe Compiler
auf Linux andere.

**Daraus für IC (Vorschlag, Entscheid bei Patrik):**

1. `compilerName()` in `werkzeuge/TestHilfe.hpp` ordnet die Abdruckdatei dem Compiler zu; für Windows-clang heißt sie
   deshalb `abdruck_clang.txt`, obwohl die Werte zu `abdruck_msvc.txt` gehören. Zuordnung nach **Plattform und
   Compiler** (etwa `abdruck_windows.txt` für MSVC und clang, `abdruck_linux.txt` für gcc und clang) oder nach dem,
   was die Bits bestimmt (Mathematikbibliothek).
2. IC-A9 Schritt 2 (an einem Ton der Klasse 1 den ersten abweichenden Abtastwert und die beteiligte Funktion nennen)
   bleibt offen; die Eingrenzung oben macht die Suche kleiner: nur `exp`/`sin`/`pow`-Aufrufe der UCRT gegen glibc.
3. `test_hammer_bezug` (Pegel je Block gegen `hammer_bezug_*`) war grün — die Abweichung liegt für den Flügel-Port
   unter dessen Toleranz.

## IC-A10 Schritt 1 — erledigt: Rechenzeit absolut unter Windows-clang

Drei Läufe `ic_tests -tc=*Rechenzeit*` allein, je nach ruhiger Taktprobe (feste Rechenlast, max/min 1,06). Grenze
„hier“ nach `echtzeitGrenzeHier`: 3,41–3,61 %. Höchstwert der drei Läufe je Instrument, % der Echtzeit:

| Instrument | max | Instrument | max |
|---|---:|---|---:|
| Flügel-Port A0 (Bezug) | 3,38 | Vibraphon 48 | 2,21 |
| Upright 21 | 1,88 | Marimba 36 | 0,29 |
| Honky-tonk 21 | 1,82 | Xylophon 65 | 0,17 |
| CP-70 28 | 2,09 | Glockenspiel 79 | 0,09 |
| Clavinet 29 | 0,64 | Röhrenglocken 48 | 0,25 |
| Hackbrett 43 (Deluxe) | 3,64 | Celesta 60 | 0,16 |
| Cimbalom 38 (Deluxe) | 5,34 | Kalimba 50 | 0,09 |
| Gitarre E2 | 0,20 | Rhodes 28 | 0,19 |
| Cello C2 ff | 1,17 | Wurlitzer 33 | 0,14 |
| Tom ff | 2,40 | Steeldrum 60 | 0,10 |
| Pauke ff | 2,33 | Posaune tiefster Ton | 1,30 |
| Crash vorab | 5,7 × Echtzeit | Klarinette tiefster Ton | 3,23 |
| Labialpfeife C2 | 0,36 | Akkordeon Musette | 1,40 |
| Orgel Akkord mit Rotary (Clip) | 1,25 | | |

Zum Vergleich: der Flügel in LumiViz (mit Stoß, Hysterese, Ebenen, Boden) lag von Hand bei 3,73–3,82 % (S156) und
4,35–4,60 % (S153) an Taste 40 — der Port misst die Vorgabe des Vorbilds ohne diese Teile, die Zahlen sind nicht direkt
vergleichbar. **IC-A10 Schritt 2 und 3** (absolut und Einheit in `BERICHT_BAU.md` und `KLASSEN.md` eintragen) bei IC.

## Was LumiViz daraus entscheidet (nur zur Kenntnis)

IK2-5 (ein Windows-Bezug für beide Compiler; ob die Gleitkomma-Schalter nötig sind, ist nicht gemessen) und IK2-6
(alle Klassen der ersten Anbindung unter 2,5 %, die Grenze 4,6 % gegen 5 % entscheidet heute kein Instrument) —
Vorlagen in LumiViz S172, Ergebnis im Konzept `docs/audio/Instrumente_IC_Konzept.md`.

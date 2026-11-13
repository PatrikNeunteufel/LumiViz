# InstrumentBodyTool — einen Instrumentkörper rechnen

**Zweck:** liest einen Bauplan (`.lvinstbodyplan`) und rechnet mit der Bibliothek `InstrumentBody` das Netz, die
Platte, die Moden, ihre Auswertung (Stegpunkte, Dämpfung, Abstrahlung je Hörort) und den statistischen Teil darüber;
dann schreibt es den Körper (`.lvinstbody`, seit 31c-5d). Die App liest nur Körper und rechnet nie (BM5 a,
`projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1).

**Exe (Release-clang):** `out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe`
— nicht an die App gebunden; es braucht nur Qt Core neben sich.

## Aufruf

```bash
InstrumentBodyTool <plan.lvinstbodyplan> [--from Hz] [--to Hz] [--mesh-to Hz] [--order k] [--mesher name] [--solver name] [--frequencies datei] [--shapes datei] [--statistics-to Hz] [--seed n] [--out datei]
InstrumentBodyTool --list
```

| Schalter | Vorgabe | Wirkung |
|---|---|---|
| `--to` | 5000 | Moden bis zu dieser Frequenz |
| `--from` | 0 | Moden ab dieser Frequenz |
| `--mesh-to` | wie `--to` | Elementgröße nach MAESSTRO Gl. 1 für diese Frequenz |
| `--order` | 3 | Ordnung k der Lagrange-Dreiecke (EL1 a) |
| `--mesher`, `--solver` | die erste Umsetzung | Netzgeber und Eigenlöser nach dem Anfang ihres Namens (BM4 a, BM5-1 a) |
| `--frequencies` | — | die Frequenzen, eine je Zeile, in diese Datei |
| `--shapes` | — | die Formen der Moden in diese Textdatei (S196): Kopfzeile `lumiviz-body-shapes 1`, `nodes N` mit x y je Zeile, `triangles T` mit drei Knoten je Zeile, `modes M`, je Mode `mode <Hz>` und eine Zeile mit w an den N Knoten des Netzes (gehalten = 0) — zum Vergleich mit Bildern gemessener Formen; für wenige Moden gedacht. Zeichnen: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-13_S196_formen_bild.py` |
| `--statistics-to` | 20000 | der statistische Teil über den gerechneten Moden bis hierher (0 = keiner; ST1 a) |
| `--seed` | 1 | der Samen des statistischen Teils |
| `--out` | neben dem Bauplan | wohin der Körper geschrieben wird |
| `--list` | — | zeigt die Netzgeber und Eigenlöser |

Die Umsetzungen stehen in `projects/libs/InstrumentBody/include/InstrumentBody/Umsetzungen.hpp` — eine neue dort
eintragen, dann ist sie hier wählbar und die Tests laufen für sie.

**Mit Hohlraum** (Block `cavity` im Bauplan mit `volume` > 0; S196, F-K1 a) sammelt das Werkzeug erst alle Moden der
Platte, koppelt sie über den Luftraum (`InstrumentBody/Luftraum.hpp`) und schreibt die gekoppelten Moden in den Körper;
es meldet Volumen, Zahl der Löcher, die Helmholtz-Frequenz bei starren Wänden, die Moden der Platte allein und je
gekoppelter Mode den Anteil der Luft. `--shapes` schreibt dann die gekoppelten Formen der Platte.

## Bau

Ein Executable der Solution (`Solution.json`, Abschnitt `executables`); es kompiliert den Leser des Bauplans aus
`projects/apps/LumiViz/src/loop/KoerperBauplan.cpp` und `core/Mitgeliefert.cpp` direkt mit (Executables laufen vor dem
App-Container, Muster `MilkdropStandalone`); seit 31c-5d dazu `loop/Koerper.cpp` (die Datei des Körpers).

```bash
tools\bau_clang.bat InstrumentBodyTool
```

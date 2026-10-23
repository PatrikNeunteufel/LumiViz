# InstrumentBodyTool — einen Instrumentkörper rechnen

**Zweck:** liest einen Bauplan (`.lvinstbodyplan`) und rechnet mit der Bibliothek `InstrumentBody` das Netz, die
Platte, die Moden, ihre Auswertung (Stegpunkte, Dämpfung, Abstrahlung je Hörort) und den statistischen Teil darüber;
dann schreibt es den Körper (`.lvinstbody`, seit 31c-5d). Die App liest nur Körper und rechnet nie (BM5 a,
`projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1).

**Exe (Release-clang):** `out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe`
— nicht an die App gebunden; es braucht nur Qt Core neben sich.

## Aufruf

```bash
InstrumentBodyTool <plan.lvinstbodyplan> [--from Hz] [--to Hz] [--mesh-to Hz] [--order k] [--mesher name] [--solver name] [--frequencies datei] [--statistics-to Hz] [--seed n] [--out datei]
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
| `--statistics-to` | 20000 | der statistische Teil über den gerechneten Moden bis hierher (0 = keiner; ST1 a) |
| `--seed` | 1 | der Samen des statistischen Teils |
| `--out` | neben dem Bauplan | wohin der Körper geschrieben wird |
| `--list` | — | zeigt die Netzgeber und Eigenlöser |

Die Umsetzungen stehen in `projects/libs/InstrumentBody/include/InstrumentBody/Umsetzungen.hpp` — eine neue dort
eintragen, dann ist sie hier wählbar und die Tests laufen für sie.

## Bau

Ein Executable der Solution (`Solution.json`, Abschnitt `executables`); es kompiliert den Leser des Bauplans aus
`projects/apps/LumiViz/src/loop/KoerperBauplan.cpp` und `core/Mitgeliefert.cpp` direkt mit (Executables laufen vor dem
App-Container, Muster `MilkdropStandalone`); seit 31c-5d dazu `loop/Koerper.cpp` (die Datei des Körpers).

```bash
tools\bau_clang.bat InstrumentBodyTool
```

# CDT

Constrained Delaunay Triangulation mit Ruppert-Verfeinerung — das Netz des Instrumentkörpers (Stufe 31c-2;
`projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1, Entscheid BM4 a, Patrik S147).

| | |
|---|---|
| Fassung | 2.0.1 (Tag vom 2026-10-13; die Verfeinerung seit 2.0.0 vom 2026-09-20) |
| Quelle | <https://github.com/artem-ogre/CDT/archive/refs/tags/2.0.1.zip> |
| Geladen | 2026-10-23 (Freigabe Patrik) |
| Größe des Archivs | 4 350 160 Byte |
| SHA-256 des Archivs | `10A4454180E0831DC49BA0135012625C716D9BC497828C433DB2A2D2AD1F4FCD` |
| Lizenz | MPL-2.0 (`LICENSE`); darin mitgeliefert: `predicates.h` (robuste Prädikate, W. C. Lenthe 2019) unter BSD-3-Clause (`LICENSE-predicates.txt`), `portable_nth_element.hpp` aus LLVM unter Apache-2.0 mit LLVM-Ausnahme (`LICENSE-LLVM.txt`) |

## Was hier liegt

- `include/` — die elf Header aus `CDT/include` des Archivs, **unverändert** (Byte für Byte verglichen).
- `LICENSE`, `LICENSE-LLVM.txt`, `LICENSE-predicates.txt` — unverändert aus `CDT/` des Archivs.
- `Include.cmake` — **eigen**: legt das Ziel `CDT` (INTERFACE, nur Header) an und bindet es an ein Ziel.
  `Solution.json` nennt sie über das Feld `include` (Muster `externals/miniz`).

Nicht übernommen: `CDT/src` (nur für den Bau als Bibliothek), `CDT/tests`, `CDT/extras`, `docs/`,
`visualizer/`, die CMake- und Conan-Dateien des Archivs.

## Wie LumiViz es benutzt

Nur in der Bibliothek `InstrumentBody`, nur hinter ihrer Schnittstelle des Netzes (BM4 a, Muster BM5-1 a): kein
Typ von CDT geht nach außen; ein eigener Generator kann dieselbe Schnittstelle umsetzen und wird mit denselben
Tests gegen CDT verglichen. Gebunden an die Unit-Tests und (ab 31c-4) an `InstrumentBodyTool` — **nicht an die
App**. Als reine Header (ohne `CDT_USE_AS_COMPILED_LIBRARY`).

## Auf eine neue Fassung heben

Das Archiv des Tags laden (Freigabe), Prüfsumme bilden, `include/` und die drei Lizenzdateien ersetzen, die
Tabelle oben und `THIRD_PARTY_NOTICES.md` nachführen, beide Compiler bauen, `*Loop Koerper*` fahren und die
Bezugswerte des Netzes vergleichen.

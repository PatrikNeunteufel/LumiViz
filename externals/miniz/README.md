# miniz

Zip lesen und schreiben — für den Export und Import der eigenen Daten
(`.lvdata`; `projects/apps/LumiViz/docs/core-services/Datenablage_Konzept.md`,
Entscheide D5 a und E2 b, Patrik S114).

| | |
|---|---|
| Fassung | 3.1.2 |
| Quelle | <https://github.com/richgel999/miniz/releases/download/3.1.2/miniz-3.1.2.zip> |
| Geladen | 2026-10-12 |
| Größe des Archivs | 101 958 Byte |
| SHA-256 des Archivs | `F0446D863F9C19926AD9483C523FDC42E42B8D4A6A431D27E09D49C79A140D9A` |
| Lizenz | Wortlaut der MIT-Lizenz, siehe `LICENSE` (der Name steht dort nicht); im Kopf von `miniz.h` steht außerdem „public domain … See ‚unlicense' statement at the end of this file" |

## Was hier liegt

- `miniz.c`, `miniz.h` — unverändert aus dem Archiv (die zusammengefasste
  Fassung in zwei Dateien).
- `LICENSE` — unverändert aus dem Archiv.
- `Include.cmake` — **eigen**: baut die statische Bibliothek `miniz` und bindet
  sie an ein Ziel. `Solution.json` nennt sie über das Feld `include`.

Nicht übernommen: `examples/`, `ChangeLog.md`, `readme.md` des Archivs.

## Wie LumiViz es benutzt

Nur über `core/ZipArchiv` und nur über Speicher (`MINIZ_NO_STDIO`): LumiViz
liest und schreibt die Datei selbst mit Qt und reicht miniz die Bytes. Die
Datei-Funktionen von miniz öffnen mit `fopen` und kämen unter Windows mit
Umlauten im Pfad nicht zurecht.

## Auf eine neue Fassung heben

Das Archiv des Releases laden, Prüfsumme bilden, `miniz.c`, `miniz.h` und
`LICENSE` ersetzen, die Tabelle oben nachführen, beide Compiler bauen,
`*ZipArchiv*` und `*Datensicherung*` fahren.

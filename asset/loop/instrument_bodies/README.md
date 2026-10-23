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
| `steinway_d` | Steinway D, Resonanzboden | Chabassier & Duruflé, RT-0425 (2012), Abb. 2 und Anhang A; Holz nach Chabassier, Joly & Chaigne 2013, Tab. I; Breiten nach MAESSTRO Tab. 1 | `make_steinway_d.py <RT-0425.pdf>` (S147, 31c-1) — Bauplan; der Körper folgt |

`make_steinway_d.py` liest die PDF (nur lokal unter `projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/`),
wertet Abb. 2 aus und schreibt den Bauplan. **Maßgebend ist die Datei im Repo**: ein neuer Lauf zuerst mit
`--ziel <ordner>` in eine Kopie, `--bild <datei.png>` legt ein Kontrollbild ab (Umriss, Rippen, Steg und
Stegpunkte über Abb. 2 (d)).

In `asset/Asset.cmake` (Kopie neben die Exe) kommen die Körper, sobald es den ersten gibt (31c-5).

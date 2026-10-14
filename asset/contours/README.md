# Formen (`.lvcontour`)

Die mitgelieferten Formen für **Klang aus einer Form** (Session 120, Stufe 19a;
Konzept: `projects/apps/LumiViz/docs/audio/Shape_Konzept.md`). Eine Schicht
im Instrument-Creator spielt eine Form als Wellenzug (Waveform Shape), und der
Knoten Shape der Effect Chain zeichnet sie.

| Datei | Titel | Konturen |
|---|---|---|
| `circle.lvcontour` | Circle | 1 — vier Kurven; klingt als reiner Sinus |
| `square.lvcontour` | Square | 1 — vier Strecken |
| `triangle.lvcontour` | Triangle | 1 — drei Strecken |
| `star.lvcontour` | Star | 1 — zehn Strecken |
| `heart.lvcontour` | Heart | 1 — vier Kurven |
| `heart_and_star.lvcontour` | Heart and star | 2 — als ein Pfad (As one path) |

Seit S123 dazu vierzehn weitere — zehn mit einer Kontur, vier mit mehreren:

| Datei | Titel | Konturen |
|---|---|---|
| `moon.lvcontour` | Moon | 1 — eine Sichel aus zwei Bögen, spitze Enden |
| `drop.lvcontour` | Drop | 1 — glatt, oben eine Spitze |
| `bolt.lvcontour` | Bolt | 1 — sieben Strecken |
| `note.lvcontour` | Note | 1 — eine Achtelnote: Kopf, Hals und Fähnchen in einem Umriss |
| `flower.lvcontour` | Flower | 1 — fünf Blätter, glatt |
| `gear.lvcontour` | Gear | 1 — zehn Zähne, viele Ecken; klingt erst mit vielen Harmonics nach Zahnrad |
| `cross.lvcontour` | Cross | 1 — zwölf Strecken |
| `house.lvcontour` | House | 1 — mit Kamin |
| `infinity.lvcontour` | Infinity | 1 — die Lemniskate; der Pfad kreuzt sich in der Mitte |
| `cat.lvcontour` | Cat | 1 — runder Kopf, zwei spitze Ohren |
| `smiley.lvcontour` | Smiley | 4 — Gesicht, zwei Augen, Mund; als Schichten (Layers) |
| `yin_yang.lvcontour` | Yin and yang | 4 — Kreis, Welle, zwei Punkte; als ein Pfad, kürzeste Sprünge |
| `rings.lvcontour` | Rings | 5 — fünf Ringe; als ein Pfad, kürzeste Sprünge |
| `atom.lvcontour` | Atom | 4 — Kern und drei Bahnen; als Schichten |

## Pflege

```
python asset/contours/make_contours.py
```

Das Skript schreibt alle zwanzig neu. Wer eine Form dazulegt, trägt sie im
Skript ein und stellt die Zahl in `test_ShapeKontur.cpp` nach.

- **Format:** JSON, Fassung 1 — beschrieben in
  `projects/apps/LumiViz/include/shape/Kontur.hpp`. Eine Kontur ist ein
  geschlossener Pfad aus Strecken und Kurven (`M`, `L`, `C`, `Z`), in den
  Koordinaten der Zeichnung, y nach unten.
- **Eine Schicht nennt eine mitgelieferte Form beim Dateinamen ohne Endung**
  (`heart`) und die Kontur über ihre feste Kennung (`id`). Eine Kennung also
  nie neu vergeben, eine Datei nicht umbenennen.
- **Neben die Exe** kommen die `.lvcontour` über `asset/Asset.cmake`; diese
  Notiz und das Skript bleiben draußen. Das Programm liest die Kopie neben der
  Exe — nach einer Änderung hier LumiViz bauen.

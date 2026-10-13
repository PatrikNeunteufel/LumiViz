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

## Pflege

```
python asset/contours/make_contours.py
```

Das Skript schreibt alle sechs neu. Wer eine Form dazulegt, trägt sie im
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

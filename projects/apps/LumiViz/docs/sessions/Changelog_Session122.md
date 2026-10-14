# Changelog Session 122 (2026-10-14)

> **Eigene Formen.** Das Fenster **Edit shape** ist gebaut: Konturen zeichnen,
> aus einem SVG holen oder in einem Bild finden, ordnen, anhören und als Form
> speichern. Mehrere Konturen spielen als Schichten oder als ein Pfad, mit
> Sprüngen in Reihenfolge der Liste, auf kürzesten Wegen oder an Brücken von
> Hand. Jede Schicht im Instrument-Creator hat jetzt eine Farbe.
> **Geprüft ist alles mit Unit-Tests und Bildern aus den Tests.** Im Programm
> gesehen oder gehört ist nichts; eine Sichttest-Liste dafür liegt bereit.

## Neu

- **Edit shape** — im Instrument-Creator mit **Edit...** neben Shape (die Form
  der Schicht) oder **New shape...** im Menü hinter Browse... (leer).
  - **Pen** zeichnet frei Hand (geglättet, in Kurven umgerechnet) und mit
    Klicks Eckpunkte, beides in einer Kontur; schließen mit einem Klick auf
    den ersten Punkt, Doppelklick oder Enter, Esc verwirft.
  - **Move point** schiebt Punkte, die Kurven gehen mit.
  - **Contours**: Haken, Farbe, Name, Anteil am Umlauf; Rename, Remove, ↑ ↓.
  - **Preview**: ein Umlauf über der Zeit (L, R), **Harmonics** und **Play**.
  - Ctrl+Z, Ctrl+Y; **Save** (eigene Form, mit Rückfrage und „Used by"),
    **Save as...** (eine neue unter My shapes — so wird auch eine
    mitgelieferte zur eigenen).
- **Open SVG...** holt die Konturen einer SVG-Datei: jeder Pfad eine Kontur,
  Löcher ebenso, offene Linien geschlossen; **Replace**, **Add** oder
  **Cancel**, wenn schon Konturen da sind; höchstens 64 (die längsten); eine
  Farbe, die alle teilen, wird durch die Farben der Reihe ersetzt.
- **Open image...** findet Konturen in einem Bild: **From image** mit **Split
  by** (Colour, Brightness, Transparency), **Levels**, **Smooth**, **Min
  size**, **Detail**; **Apply live** an — jede Änderung ersetzt die Konturen
  aus dem Bild —, aus — Vorschau in Grau und **Take contours**; **Show
  image**; Konturen am Bildrand heißen „edge".
- **Use as** — **Layers, one per contour** oder **One path** mit **Jumps**
  (In list order, Shortest, Drawn by hand); **Bridge** setzt Sprünge von
  Hand. Der Ton folgt der gewählten Art; eine Kontur mit zwei Brücken wird
  unterwegs verlassen und wieder betreten.
- **Save mit Layers** im Creator legt je weiterer angehakter Kontur eine Kopie
  der Schicht an (ohne Doppelte, bis acht).
- **Farben der Schichten**: ein Farbpunkt vor jeder Schicht — Sine Blau,
  Triangle Grün, Saw Rot, Square Orange, Plucked string Ocker, Noise Grau,
  Sample Violett, in Abstufungen; eine Shape-Schicht in der Farbe ihrer
  Kontur. **Colour...** wählt eine eigene, **Automatic** stellt zurück.

## Geändert

- Wird eine Form gespeichert, zeichnet der Knoten **Shape** gleich die neue
  Fassung, und Loops und Instrumente rechnen ihre Töne neu.
- Die `.lvinst` kann an einer Schicht `color` tragen — ohne neue Fassung: ein
  älteres LumiViz liest die Datei weiter und zeigt die abgeleitete Farbe.
- Instrumente ohne Shape-Schicht klingen Bit für Bit wie vorher.

## Bekannt

- Farben, die ein SVG über CSS-Variablen angibt (`var(--…)`), kennt Qt Svg
  nicht: solche Pfade kommen nicht an.
- Das Bild hinter From image wird nicht in der Form gespeichert.

## Tests

- 1200 Cases (neunzehn neu: `test_ShapeZeichnen`, `test_ShapeFenster`,
  `test_ShapeSvg`, `test_ShapeBild`, `test_ShapeSpruenge`); voller Lauf
  1199 / 1199 nach Stufe 20d unter Testing-MSVC, der Fall zu den
  Sichttest-Vorlagen danach unter beiden Compilern grün.

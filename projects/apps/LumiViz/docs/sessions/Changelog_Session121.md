# Changelog Session 121 (2026-10-13)

> **Klang aus einer Form ist im Programm angekommen.** Vier Stufen sind
> gebaut: zwei neue Knoten für die Effect Chain — **Shape** und **XY Scope** —,
> die Form als Schwingung im Klangerzeuger, und die **Shape-Schicht im
> Instrument-Creator**. Damit lässt sich ein Instrument bauen, das eine
> gezeichnete Form spielt, und sein Bild im Scope ansehen.
> **Geprüft ist alles mit Unit-Tests, Messungen und Bildern aus den Tests.**
> Im Programm gesehen hat Patrik den Knoten Shape (ohne Befund an Claude);
> gehört ist nichts. Eigene Formen lassen sich noch nicht zeichnen.

## Neu

- **Knoten Shape** (Effect Chain, Palette unter Images): zeichnet die Konturen
  einer Form-Datei (`.lvcontour`) als Linien — zusammengesetzt aus beliebig
  vielen Teiltönen (**Harmonics**, 1 bis 512, auch Zwischenwerte). Dazu
  **Contour**, **Points**, **Line width**, Lage, **Size**, **Rotation**,
  eigene Farbe, **Line blend** und **Show circles** mit **Speed**: die Kreise
  und Arme, aus denen die Form entsteht, laufen mit. Skriptfelder Init,
  Frame, Beat, Song und **Point** (je Punkt der Linie). Neun
  Voreinstellungen: Heart, Epicycles, Harmonics Sweep, Beat Harmonics, Beat
  Pulse, Bass Spin, Wave Outline, Rainbow Outline, Contour Switch.
- **Knoten XY Scope** (Palette unter Scopes & Sources): zeichnet links gegen
  rechts, ein Punkt je Abtastwert, genauer als die Skriptfunktion `getosc`.
  **Gain**, **Draw** (Lines oder Dots), **Line width**, **Rotate 45°**, Lage,
  **Size**, **Color from** (Table oder Gradient), **Line blend**; dieselben
  fünf Skriptfelder. Sechs Voreinstellungen: Goniometer, Lissajous, Beat
  Zoom, Slow Spin, Rainbow Trace, Corner Meter.
- **Waveform Shape im Instrument-Creator:** eine Schicht spielt eine Form —
  ein Stift fährt mit der Tonhöhe um die Kontur, der Ton ist seine Bewegung.
  **Shape** mit **Browse...**, **Contour** (eine Kontur oder „All, one
  path"), **Harmonics**, **Harmonics envelope**, **Angle**, **Start**,
  **Stereo XY**, **Reverse**; rechts das Bild **Shape, as it plays**. Voices,
  Detune, Pitch sweep, Vibrato, Envelope, Filter und Drive gelten wie bei
  Sine und Saw.

## Geändert

- **`.lvinst` steht in Fassung 9** (die Schwingung Shape). Ältere Dateien
  lesen sich wie bisher und klingen Bit für Bit gleich; ein älteres Programm
  liest eine Datei der Fassung 9 nicht.
- Die Liste **Waveform** im Creator trägt acht Einträge.

## Für Entwickler

- **Entscheide (Patrik):** V1 a bis V4 a (Knoten Shape: fünf Skriptfelder,
  Regler, neun Presets; XY Scope bekommt dieselben Felder), T1 a (Pegel:
  kein Ausgleich, die Form am fernsten Punkt gemessen), T2 a (Angle dreht
  auch mit Stereo XY), X1 a und X2 a (XY Scope: Regler, sechs Presets); die
  Reihenfolge der Stufen blieb. `docs/audio/Shape_Konzept.md` 0.16.0.
- **Stufe 19b:** neu `shape/Zeichenform` (eine Zeichnung zum Zeichnen:
  gemeinsam eingepasst, Umriss mit Zwischenspeicher, Kreise und Arme);
  `ShapeParams`, `MultiEffectVisualizer::runShape` mit eigenem Skript-Host.
- **Stufe 19c:** neu `shape/Tonform` (eine Zeichnung für den Ton: eine
  Kontur oder alle als ein Pfad mit Sprüngen, je Nennung gemerkt);
  `Schwingung::Form`, `rechneForm`, `kInstrumentFassung` 9.
- **Stufe 19d:** `XyScopeParams`, `runXyScope`. Im Feld Point zeigt `y` nach
  oben (der rechte Wert) — bei SuperScope und Shape nach unten.
- **Stufe 19e:** `InstrumentCreator` mit den Zeilen der Shape-Schicht,
  `FormBild`, `setzeForm`.
- **Erzeugt:** `FieldBindings.cpp`, `FieldDocs.cpp`, `inventory.json`,
  `inventory_docs.json` (zwei neue Knotentypen; `harvest_field_docs.py` führt
  `shape.contour` als Zusatzfeld). AvsStandalone übersetzt `shape/` und
  `core/Mitgeliefert` mit.
- **Tests:** 1181 Cases (achtzehn neu), voller Lauf 1181 / 1181, 0 Skips unter
  Testing-MSVC; unter Release-clang je Stufe die Fälle des Bereichs. Neu
  `test_ShapeKnoten.cpp` (fünf Fälle, `LUMIVIZ_SHAPE_KNOTEN_BILD`),
  `test_ShapeTon.cpp` (neun Fälle, `LUMIVIZ_19C_WAV`),
  `test_XyScopeKnoten.cpp` (zwei Fälle, `LUMIVIZ_XY_SCOPE_BILD`), je ein Fall
  in `test_PaletteUi.cpp` (`LUMIVIZ_SHAPE_PANEL_BILD`) und
  `test_LoopInstrumentEditor.cpp` (`LUMIVIZ_INSTRUMENT_SHAPE_BILD`); sieben
  Handbuch-Beispiele belegt.
- **Gemessen (Release-clang):** Per point 0,19 ms bei 512 Punkten; der Umriss
  0,09 ms in der Vorgabe. Eine Shape-Schicht kostet 4,1 ms je Sekunde Ton bei
  24 Harmonics, 8,6 ms bei 64, 40 ms an der Bandgrenze bei A1 (eine teure
  Schicht mit Säge: 2,2 ms). Die Testing-Fassung rechnet das dreißig- bis
  vierzigmal langsamer.

## Offen

- **Im Programm ungesehen und ungehört:** der Knoten XY Scope, die
  Shape-Schicht im Creator, ihr Ton, das Scope an einer Schicht mit Stereo
  XY; vom Knoten Shape liegt kein Befund vor. Sichttest-Listen dafür gibt es
  nicht.
- **Zu hören** (36 Hörproben, lokal): ob die **Ecken** gedämpft werden sollen,
  und die **Kante des Sprungs** bei mehreren Konturen als ein Pfad — bei A4
  liegen −33 bis −34 dB des Tons als zurückgefallener Anteil neben den
  Teiltönen, gleich bei 16, 48 und 160 Harmonics.
- **Nicht gebaut:** das Fenster Edit shape (Stufe 20a, als Nächstes) mit dem
  Knopf Edit... im Creator, SVG, Konturen aus einem Bild, die Arten der
  Sprünge „kürzeste Wege" und „von Hand"; nach dem Schreiben einer Form
  entstehen schon gerechnete Töne nicht neu, und der Knoten Shape liest eine
  geänderte Datei erst bei neuer Nennung; Feld-Sonden für `shape` und
  `xyScope`.
- **Kein mitgeliefertes Instrument** trägt eine Shape-Schicht.
- Aus den Sitzungen 118 bis 120 weiter: Liste 10 Abschnitt V (Pan, acht
  Schichten, Icons), Abschnitt D, X7; Patriks Ohr an den Hörproben zu Kick
  und Clap; `asset/loop/effects_demo2.lvloop`.

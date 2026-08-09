# Changelog Session 39 (2026-07-22) — MilkDrop-Import M1–M4: Parser, Skript-Vertrag, MilkdropVisualizer

> **Typ:** Produkt-Changelog
> **Bezug:** [MilkDrop_Import_Konzept.md](../visuals/MilkDrop_Import_Konzept.md) (v1.4.0) ·
> [MilkdropVisualizer.md](../../include/visualizers/MilkdropVisualizer.md) ·
> [Offene_Implementierungen.md](../Offene_Implementierungen.md)
> **Tests:** LumiViz.UnitTests 346 Cases grün, 0 Skips, 9949 Assertions (vorher 308/4108)
> **Sichttest:** M3+M4 durch Patrik BESTANDEN (3 Kalibrier-Runden)

## Neu

### MilkDrop-Presets laden und rendern (.milk → Bild)

Ein Doppelklick auf eine `.milk`-Datei im Import-Browser schaltet auf den neuen
**Milkdrop-Visualizer** und rendert das Preset mit der originalen MD1-Pipeline:
Warp-Mesh mit per_pixel-Gleichungen (je Vertex), decay-Feedback, Basis-Waveform
(alle 8 Modi), **Custom Waves und Shapes bis 16** (init/per_frame/per_point mit
t1–t8-Snapshots, Farbverlaufs-Fans, textured-Shapes als „Glaslinse", Instanzen),
**Motion Vectors** mit echter Warp-Rückverfolgung, Rahmen, Darken-Center und
MD1-Composite (Video-Echo mit Spiegelungen, Gamma, brighten/darken/solarize/
invert). Mesh-Auflösung als Parameter (Default 32×24, Cap 96×72). Shader-Presets
(MD2/MD3) laufen vorerst im MD1-Fallback — Report weist darauf hin (M5).

### Lib MilkParser (Import-Fundament)

Header-only-Parser `.milk → Struktur` mit Original-Ladeverhalten: nummerierte
Code-Zeilen mit Lücken-Abbruch, Backtick-Shader, **Inline-Kommentare enden am
Original-Zeilenende** (Original-Regel, sonst frisst ein `//`-Kommentar den
restlichen Code), MD3-Superset (16 Waves/Shapes, Sprites, PSVERSION-Header).
**Korpus: 910/910 Presets parsen fehlerfrei** (Milkdrop3-Pack + Winamp-Pack).

### Milk-Skript-Vertrag (Engine/Transpiler)

- `int()` ist jetzt korrekt der **floor-Alias** des Originals (1153 Fundstellen
  im Korpus); Funktionsargumente dürfen `;`-Statement-Sequenzen sein
  (`if(bt, t0=time; pk=vol, 0)`). **Transpile-Abdeckung damit 100 %**
  (892/892 per_frame, 590/590 per_pixel).
- **MilkLoudness:** bass/mid/treb als Loudness relativ zum Langzeit-Mittel
  (~1.0-Baseline, wie MilkDrop-Presets es erwarten) plus geglättete
  `bass_att/mid_att/treb_att`.
- q1–q64-Snapshots und wave-lokale t1–t8 nach Original-Semantik (Golden-Tests).
- Editor-Kategorien (`symbolCategory`) kennen das komplette
  MilkDrop-Variablen-Set.

### Kalibrier-Preset-Satz (committet)

`asset/calibration/milkdrop/m3|m4`: 18 minimale Presets, die je EINEN Aspekt
isolieren (Orientierung, Zoom, Rotation, Warp, Decay, Wave-Modi, Echo, Gamma,
Borders, per_pixel-Tunnel · Wave-Geometrie/-Pegel/-Spektrum, Shape-Verlauf/
-Instanzen/-Textur, Motion Vectors, t/q-Vertrag) — mit README je Ordner
(Beobachtung, Erwartung, Stellschraube). Läuft als Pflicht-Korpus in der
Test-Suite. Sichtkalibrierte Werte: kWavePortScale=192, kSpecPortScale=8.

## Entscheide (Patrik)

Mesh-Auflösung als Visualizer-Parameter (32×24/Cap 96×72; kein Preset-Feld —
Messung über 910 Presets) · MD3-Superset sofort parsen, rendern schrittweise ·
Korpus = beide Packs · `asset/Milkdrop3/` bleibt untracked (Community-Rechte).

## Offen (nächste Session)

**M5:** Blur-Pyramide + Shader-Muster-Module (Stufe B) + Korpus-Statistik →
Stufe-C-Entscheid · **M6:** MilkdropPanel, Crossfade, Playlist · Kür: Sprites,
fShader-Wash, Decay-Dither · geparkt: Wormhole-Bisektion, Sichttests §7/§8.

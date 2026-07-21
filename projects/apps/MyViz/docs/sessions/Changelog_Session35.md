# Changelog Session 35 (2026-07-20/21) — §5.2-Effekt-Abdeckung + Editor/SuperScope

> **Typ:** Produkt-Changelog
> **Bezug:** [Import_Analyse_AVS_MilkDrop.md](../visuals/Import_Analyse_AVS_MilkDrop.md) §5.2
> **Tests:** MyViz.UnitTests 251 Cases grün, 0 Skips, 3745 Assertions (vorher 222)

## Neu

### Blend-Modi Batch 2 — alle 14 AVS-List-Modi

Subtractive 1-2/2-1, Every-other-line/-pixel, XOR und **Buffer** (globale
Pool-Puffer-Tiefe als Alpha, Index + Invert) rendern jetzt echt; der frühere
Replace-Fallback entfällt.

### Ketten-Editor — bedienbarer

- **Drag & Drop**: Effekte/Gruppen im Baum verschieben — auf eine Gruppe = hinein,
  dazwischen = umsortieren, auf Leerfläche = ans Root-Ende (heraus).
- **Auge-Toggle** (👁) pro Zeile zum Ausblenden (Effekt oder ganze Gruppe).
- **Clone-Button** (inkl. Subtree).
- Nesting jetzt klar eingerückt (mehrstufig).

### SuperScope — Figuren + Farbe

- **Figur-Preset-Dropdown** (Spiral, Butterfly, Hypocycloid …) lädt die EEL-Skripte
  in die Felder.
- **Hybrid-Farbmodell** (`Color mode`): Gradient (per Punkt) × zeit-gezykelte
  AVS-Farbtabelle, kombiniert per Blend (Gradient/Table/Additive/Multiply/Average).
  Die Basisfarbe **belegt red/green/blue vor dem Point-Code vor** (AVS-treu) — der
  Code kann sie behalten, modulieren oder überschreiben. Gradient-Previews im Dropdown.
- AVS-`colors[]` beim Import übernommen.

### Movement — 23 Builtin-Formeln

Die eingebauten Movement-Formeln (big swirl, tunneling, kaleida …) werden importiert
und gerendert; dafür die `ScriptGridModule`-Polar-Konvention an AVS angeglichen.

### §5.2-Effekt-Abdeckung komplett — 16 neue Effekte

- **Pixel:** Mosaic, Grain, Scatter, Interferences.
- **Wasser/Bump:** Water, Bump (bewegliche Lichtquelle per EEL), Water Bump
  (Höhenfeld-Wellensimulation + Beat-Tropfen).
- **Renderer:** Starfield, Timescope (scrollendes Spektrogramm), Dot Grid, Dot Plane,
  Dot Fountain.
- **APE:** Channel Shift, Color Reduction, Multiplier (+ generischer APE-Import-Pfad).
- **Delays:** Video Delay, Multi Delay (6 geteilte Puffer).

Alle über den AVS-Import verfügbar (Parser + Übersetzer + Persistenz + Panel-Editor).

## Geändert

- `SuperscopeModule`: Farbpipeline (Gradient × Tabelle × Vorbelegen) zentralisiert —
  der Standalone-SuperScope profitiert ebenfalls.
- Effect-Chain-Panel: Spalten Name/👁/Type/Description, Farb-/Figur-/Delay-Editoren,
  Gradient-Preview-Delegate.

## Bekannte Rauheiten (Sichttest-Nachzug)

- **Sichttest-kalibriert** (Startwerte im Code): 3D-Projektion Dot Plane/Fountain,
  Water-Bump-Refraktionsstärke, Starfield-Geschwindigkeit/Helligkeit.
- Skript-Editor-Feinschliff (Syntaxhervorhebung, Vergrößern, kontextbezogene Hilfe,
  Kategorie-Sortierung im Modul-Dropdown) — geplant.
- Fehlende Module: „Moving Particle", „Jheriko: Global" (APE) — noch Passthrough.

## Verifikation

251/251 Cases grün, 0 Skips; VS-Testing baut (`/WX`). Neue Unit-Tests je Effekt
(Translator-Mapping inkl. Color-Swap/Float-Bits/APE-Dispatch, Serializer-Roundtrip,
Anzeigename) + ScriptGridModule-Konvention + SuperscopeModule-Farb-Vorbelegen.
GL/UI-Sichttest steht aus (Shader kompilieren erst zur Laufzeit).

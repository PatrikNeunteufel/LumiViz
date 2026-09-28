# Changelog Session 79 (2026-09-27)

> **Module finden statt suchen, und fast jeder Wert tanzt zur Musik.** Die
> Auswahl-Combo der Effektkette ist ein Palettenfenster mit Suche, Filtern und
> Drag & Drop geworden. Und die Formelfelder (init / frame / beat) erreichen
> jetzt Farben, Paletten, dutzende weitere Regler und die Werte in eigenen
> Shadern.

## Neu

- **Effekt-Palette** („+" in der Effektkette): ein Fenster, das offen bleibt,
  bis man es schließt.
  - **Suche** über Name, Originalname („Trans / Movement") und Beschreibung;
    Enter fügt den ersten Treffer ein.
  - **Filter** nach Herkunft (AVS · APE · MilkDrop · LumiViz) und Rolle
    (zeichnet · verändert Bild · Farbe · Steuerung).
  - **★ Favoriten** und **zuletzt benutzt** oben.
  - Kurzbeschreibung je Modul, mit Hinweis, ob es Formelfelder hat.
  - **Einfügen** per „Add", Doppelklick, Enter — oder **per Drag & Drop** an
    die gewünschte Stelle der Kette.
- **Farben per Formel** in 16 Modulen: `color_r`, `color_g`, `color_b`
  (0..1; bei Modulen mit mehreren Farben z. B. `clipcolor_*`, `fogcolor_*`).
- **Fraktal-Paletten per Formel:** `colorscale`, `colorcycle` und
  `colorphase` — etwa `colorphase = colorphase + 0.25` im Beat-Slot schiebt
  die Farben im Takt. Dazu Innen-, Hintergrund- und Negativfarbe.
- **43 weitere Regler per Formel** — Fraktal-Details (Iterationen,
  Julia-Werte, Licht, Oktaven …), Bump-Tiefe, 3D-Profile, Terrain,
  Glow Orbs, GPU-Partikel, Water-Bump-Tropfenort u. a.
- **Set Render Mode** hat Formelfelder: z. B. `linewidth = 1 + bass*4`.
- **ISF-Filter:** jeder Regler eines importierten Filters ist unter seinem
  Namen per Formel steuerbar.
- **Eigene Shader-Werte:** wer in Shadertoy, Pixel Filter, Mesh Warp oder dem
  GPU-Partikel-Kraftfeld `uniform float wobble;` schreibt, setzt `wobble` per
  Formel. Shadertoy hat dafür jetzt Init-/Frame-/Beat-Felder.
- **Host-Gruppe:** die Ausgangs-Deckkraft (`outadjustalpha`) per Formel — ein
  ganzes Visual kann im Takt pulsieren.

## Behoben

- Acht Module versprachen in ihrer Beschreibung Formel-Variablen, die nie
  wirkten (u. a. Brightness `distance`, Kleinian `p`/`q`, Interleave
  `x2`/`y2`) — sie wirken jetzt.
- **Set Render Mode** vergaß beim Speichern den Wert „Adjustable alpha".
- Die **Variablen-Referenz** (ⓘ neben den Code-Feldern) zeigt jetzt für jedes
  Modul mit Formelfeldern seine Variablen — vorher bei rund vierzig Modulen
  nur „No dedicated variable reference".

## Doku

- NEU `docs/visuals/Formelfelder_Inventar.md` — alle Module, ihre Formelfelder,
  offene Vorschläge
- `include/UI/panels/MultiEffectPanel.md` (Palette, Referenz),
  `include/visualizers/MultiEffectVisualizer.md` (Skriptvariablen), Wrapper-Docs
- `docs/Offene_Punkte.md` 1.74.0 → **1.75.0**

Version der App unverändert (0.4.0).

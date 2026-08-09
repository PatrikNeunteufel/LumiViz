# Changelog — Session 60 (2026-07-31 / 2026-08-01)

**Die Kalibrier-Runde (Session 44–60) ist abgeschlossen.** Das in dieser
Session eingeführte Fertig-Kriterium (`Offene_Punkte.md` §0, sechs messbare
Haken) ist vollständig erfüllt: Modul-Matrix **43/43** (41 gemessen, `water`/
`grain` als abgenommene bewusste Grenzen), Modul-Sonden **91/91**, Feld- und
Edit-Sonden-Vollauf **grün** (0 WIRKUNGSLOS, 0 unerklärte STUMM), Zwillinge
**67/67**, Abschluss-Messlauf 24 s-Sonden + 8 Kern-Presets in beiden Größen
ohne Regression, Tests **485/485**.

## Treue-Fixes

- **Water Bump rendert wieder** — ein stiller Qt-Uniform-Typfehler
  (`setUniformValue(QPoint)` lädt float-vec2 auf ivec2) hatte den Knoten zum
  bit-exakten Passthrough gemacht; der Fix löste zugleich die offene
  Trail-Matrixzeile (0,100 → 0,003).
- **Rotating Stars, Osc Star, Osc Ring: exakte Ports** der Original-Renderer
  (Lokal-Peak-Suche in rohen visdata-Bytes, Hüllkurven, Cast-Eigenheiten) —
  alle drei Matrix-Zeilen pixelgenau (Deckung 1,00). Die winzigen
  Referenz-Sterne ersetzen die früheren großen Näherungs-Sterne.
- **Dot Grid stand vertikal gespiegelt** — bei Raster 8 traf nie eine Zeile
  die Referenz; jetzt pixelgenau.
- **Blitter Feedback:** Zoom-out sampelt wie das Original immer nearest, und
  der y-Anker der Abbildung saß eine Zeile zu tief — Zeile grün (Deckung 0,93).
- **Convolution „wrap" umgesetzt** — an der Original-APE vermessen: ab
  scale 2 läuft die Negativ-Verrechnung als 16-bit-Überlauf mit unsigned
  Division (bei scale 1 wirkungslos); zwei neue Dauersonden.
- **AVI-Knoten: Vorschub-Gate referenz-treu** (kein Sofort-Start mehr; drei
  Gate-Sonden auf dem Testvideo exakt 0). Der Rest von el-visVR09 ist reine
  Indeo-Decoder-Differenz (VfW 32-bit vs. FFmpeg).
- **Tie Tunnel DM:** 0,148 → 0,074, der Würfel sitzt deckungsgleich; offen
  bleibt die Farb-Phase der Tunnelbänder.

## Werkzeuge

- **AvsRef:** `r_avi`-Uhr virtualisiert (`--tick-hz` wirkt jetzt auch auf den
  AVI-Vorschub) — AVI-Presets sind erstmals messbar; length-Debug-Print.
- Neue Sonden: Convolution wrap (2) · AVI-Gate (3, Scratchpad) ·
  rotatingStars-Felder `audioGain`/`bandLo` als begründet „nicht prüfbar".

## Neu für Benutzer

- **84 mitgelieferte Knoten-Voreinstellungen für 26 Typen** (68 neu):
  Bewegungs-Klassiker inkl. der acht originalen AVS-Movement-Formeln,
  Faltungskerne, Farbverläufe, Wasser-/Gitter-/Zoom-/Spiegel-/Beat-Vorlagen —
  alles Teil-Presets. Katalog: `asset/nodepresets/README.md`;
  Benutzerhandbuch §11 erweitert.
- **MilkDrop-Texturen `worms`/`rose`/`grad3` beschafft** — 27 von 35
  Fehler-Log-Zeilen der MilkDrop-Packs verschwinden.

## Entscheide (Patrik)

water_bump-Ganzzahl-Kern bleibt · water/grain als bewusste Grenzen abgenommen ·
Hotkeys §9.2–9.5 · Playlist-Defaults (kein automatischer Wechsel) ·
Lights-Lookahead in den Backlog · Config-Pipeline-Abnahmetabelle gestrichen ·
Feldreihenfolge bleibt Init·Frame·Beat·Point · Vorlagen-Konvention
(Teil-Presets, sprechende Namen, `asset/nodepresets/<typkey>/`).

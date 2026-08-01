# Komposit-Presets — zehn Themenwelten

200 **ganze** Presets (je Ordner ein Hauptthema, 20 Varianten), komponiert im
Geist der el-vis/UnConeD-Klassiker: Buffer-Tunnel, Feedback-Schleifen,
Beat-Physik (der `ti`-Abkling-Schub), 3D-Projektionen in purem EEL.

**Erzeugt** von `asset/calibration/avs/make_composit_presets.py` (Variation
seeded — reproduzierbar); Änderungen dort machen, nicht in den Dateien.

| Ordner | Hauptthema | Architektur |
|---|---|---|
| `01_wurmloecher` | Tunnelflüge | Band-Liste rendert in Buffer-Slot 0, eine Dynamic Movement raytraced daraus den Flug (Tie-Tunnel-Bauart, frei variiert: Twist, Kamerapfad, Mosaik-Körnung, Channel-Shift auf Beat) |
| `02_plasmanebel` | organische Nebel | Punktwolken → Water → gekreuzte Sinus-Verwerfung → Colorfade |
| `03_kaleidoskop` | Spiegelwelten | asymmetrische Blüten-Scopes → Drall-Sog → Interferences-Echo → Mehrfach-Spiegel (auf Beat zufällig) |
| `04_sternenstaub` | Weltraum | Starfield + rotierende Dot-Ebenen/Sternchen + leichter Roto-Zoom, Korn-Schleier |
| `05_maschinenraum` | Industrie-Rhythmen | Custom-BPM-Taktfilter → Beat-Stanzen (OnBeat-Clear, Mosaik) → Kolben-Scope → Scanline-Gitter → Faltungs-Kanten → Beat-Beben |
| `06_drahtgeister` | 3D-Skulpturen | Torusknoten, Lissajous-Käfige und Spindeln in purem SuperScope-EEL (eigene Rotationsmatrix + Projektion), Regenbogen über die Bahn |
| `07_tiefsee` | Biolumineszenz | Planktonschwärme auf Strömungsfeldern + pulsierende Qualle + Water-Bump-Brechung + Blaudrift |
| `08_farbenrausch` | Farbzyklen | Swirl-Feedback durch Color-Map-Paletten gejagt, Colorfade-Läufe, Channel-Shift-Akzente |
| `09_fraktaltraeume` | Fraktale (**.lvfx**) | Host-Module: Julia-Puls mit Warp-Trail · Mandelbulb-Kamera am Bass · Endlos-Zoom + Kleinian-Overlay · Attraktor über Reaktions-Diffusion — je mit Bloom |
| `10_lichterstadt` | Lights-Ästhetik (**.lvfx**) | DomainWarp-Grundnebel (Warp am Bass) + Funkenflug/Lichtkugeln/3D-Lichtfaden + Bloom & Vignette |

Die acht .avs-Themen sind Original-EEL-konform und rendern auch in AvsRef —
damit taugen sie zusätzlich als Vergleichs-Material. `rand()`- und
Beat-getriebene Presets sind zwischen den Renderern nicht bit-stabil.

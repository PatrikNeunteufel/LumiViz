# Changelog — Session 61 (2026-08-01)

Nach dem Abschluss der Kalibrier-Runde die erste Ausbau-Session: der letzte
benannte Treue-Rest **Tie Tunnel ist gelöst**, der Vorlagen-Katalog wächst auf
**125 Voreinstellungen für 39 Knotentypen**, und zwei neue Asset-Sammlungen
liefern **125 Beispiel-Presets** und **200 Komposit-Presets**. Zwei
Sichtprüfungs-Runden (Patrik) deckten einen systematischen Farbfehler der
Generatoren und mehrere Modul-/Vorlagen-Schwächen auf — alle behoben.
Tests **485/485**.

## Treue-Fixes

- **Tie Tunnel GELÖST** (0,074/0,098 → 0,010/0,009 in 320er, 0,002/0,010 in
  640er; Schwester-Preset SSC 0,001): Die Farb-Phase der Tunnelbänder war der
  beim Import **weggeworfene Startwinkel der Dot Plane** — `r_dotpln`/`r_dotfnt`
  speichern ihre laufende Rotation als 8. Preset-Feld (`r = rr/32`, hier
  44,84°). Neues Feld `startRotation` bei Dot Plane UND Dot Fountain
  (Startwert-Bauart wie `interfRotationSeed`); Matrix-Zeilen 01/19 bleiben
  pixelgenau, Zwillinge 67/67.
- **SuperScope-Figur „Heart" stand Kopf** — die Figuren-EEL läuft im
  AVS-kalibrierten Chain-Host (+y zeigt nach unten), die Mathe-Formel war
  y-up. y im EEL negiert (Modul-SSOT und Vorlagen-Datei synchron); die native
  Rechnung des Standalone-Visualizers bleibt unverändert richtig.
- **domainWarp bewegte sich praktisch nicht** — `uTime` floss nur ins
  Warp-Feld, nie in den Abtastpunkt; der Shader hat jetzt eine direkte
  Grunddrift (Host-Modul ohne Referenzvertrag).

## Neu für Benutzer

- **Vorlagen-Katalog: 125 Voreinstellungen für 39 Knotentypen** (+41):
  die komplette **Fraktal-Familie** (Mandelbrot-Ausschnitte, Julia-Puls,
  Mandelbulb, Plasma, Endlos-Zoomer, Lyapunov „Zircon City", Kleinian,
  Attraktoren, Flame, Reaktions-Diffusion), **Bloom** (inkl. der
  Lights-Referenzwerte), blur/simpleScope/customBpm — und sechs aus der
  Preset-Sammlung **geerntete Dynamic-Movement-Warps** (Quelle je Vorlage im
  Katalog genannt). `asset/nodepresets/README.md`, Benutzerhandbuch 1.4.0.
- **`asset/examples/`: je Voreinstellung ein Beispiel-Preset** (125, flach,
  Name = Typ + Vorlagenname) mit passendem Render-Material. 90 davon als
  .avs — direkt mit `compare_avsref.py` gegen die Referenz messbar
  (Vergleichslauf 85/90 OK, 0 Fehler).
- **`asset/composits/`: 200 ganze Presets in zehn Themenwelten** (je 20):
  Wurmlöcher, Plasmanebel, Kaleidoskop, Sternenstaub, Maschinenraum,
  Drahtgeister, Tiefsee, Farbenrausch (Original-EEL-.avs) sowie
  Fraktalträume und Lichterstadt (.lvfx mit Host-Modulen) — komponiert nach
  el-vis/UnConeD-Bauarten (Buffer-Tunnel, Beat-Physik, 3D-EEL-Projektionen),
  beidseitig render-validiert.

## Werkzeuge

- `make_example_presets.py` — Beispiel-Generator (Rückabbildung des
  kalibrierten Imports, .lvfx-Fallback, EEL-Dialekt-Filter).
- `make_composit_presets.py` — Komposit-Generator (seeded, reproduzierbar;
  dokumentierte Schleifen-Hygiene für Feedback-Kompositionen).
- Import-Ernte über `AvsStandalone --dump` (700+ Knoten-Kandidaten für
  weitere Vorlagen-Runden).

## Befunde aus der Sichtprüfung (behoben)

- **Farb-Doppelfehler der Generatoren:** `avsColor` ist ein No-op (AVS
  speichert 0x00RRGGBB) — der R/B-Tausch beim Erzeugen verdrehte alle Farben,
  und der AvsRef-Vergleich blieb grün, weil beide Seiten dieselbe verdrehte
  Datei lasen. Lehre: Rückabbildungs-Vergleiche beweisen Übereinstimmung,
  nicht Intention.
- **Bloom-Beispiele zeigten nichts** — die 1-px-Spirale überlebt das
  Glow-Downsample nicht; Bloom-Beispiele stehen jetzt auf einer hellen
  Lichtfaden-Bühne (Wirkung bewiesen, Mit/Ohne-Diff 0,134).
- **Lebendigkeits-Pass über 15 Vorlagen** (Starfield-Dichte, Attraktor-
  Audio-Morphing, Lorenz-Tempo, DDM/Shift kräftiger, Burning-Ship-Ausschnitt);
  Datenfixes: Zufallsspiegel (`slower` war bool), Hufeisen-Nebel (vier
  Abbildungen kollabieren).

## Offen / notiert

- 🟠 **Blitter-Zoom-in** wird über Dauer-Rückkopplung weicher als die
  Referenz (0,062–0,163) — Vermessung ausstehend (§1).
- ⚪ **fractalZoomer erschöpft nach Minuten die float-Präzision** (Endlos-Zoom
  → Pixelbrei/Schwarz) — braucht einen Host-Tiefen-Reset (§7).
- 🟡 Entscheide: Host-Klassiker-Vorlagen (Asset-Suchpfad im
  VisualizerPresetManager) · Star-Figur (Spitze zeigt nach unten).
- **Nächste Session (Vorgabe Patrik): Grafikkarten-Auswahl** — einstellbar,
  via Settings umstellbar, Änderung löst sofort einen Neustart aus; nur mit
  Vorher/Nachher-Messlauf (§8).

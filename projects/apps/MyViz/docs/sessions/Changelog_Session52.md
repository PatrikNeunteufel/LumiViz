# Changelog — Session 52 (2026-07-27)

> Hotkey-Reparatur und -Ausbau, Screenshot-Ablage, vier Renderer-Fixes aus der
> Kalibrier-Runde, zwei nachgebaute APEs und eine Doku-Sanierung. Zwei der
> Befunde lagen nicht im Renderer, sondern in der **Messung**.

## Bedienung

- **Hotkeys `Bild ab`/`Bild auf` funktionieren ab Werk.** Die Vorbelegung war
  tot: `"PageDown"` ist kein Name, den Qt kennt (dort heißen die Tasten
  `PgDown`/`PgUp`) — ein unbekannter Name ergibt eine unbrauchbare „Taste", und
  zwar still. Wer die Tasten schon von Hand gesetzt hat, braucht einmal
  *Einstellungen → Hotkeys → „Alle auf Standard"*. `Bild ab` geht in der Liste
  nach unten, also vorwärts.
- **Die Transport-Hotkeys sind verdrahtet** (Hotkey-Stufe 2): Leertaste =
  Wiedergabe/Pause, `Ctrl+←/→` = Song wechseln, `Ctrl+↑/↓` = Lautstärke (5 % je
  Anschlag). Sie wirken auch, wenn das Player-Panel nicht sichtbar ist.
- **`Druck` legt einen Screenshot des Visuals ab** — ein Ordner je Programmlauf
  (benannt nach dem Startzeitpunkt), darin das Bild unter dem Presetnamen und
  eine `.txt` mit dem vollständigen Preset-Pfad. Mehrfach dasselbe Preset ergibt
  `_2`, `_3`; überschrieben wird nie. Zielordner über den Einstellungsschlüssel
  `screenshot/baseDir`, sonst wird `asset/calibration/screenshot` gesucht.
- **Im Vollbild erscheint bei Fehlern kein Meldungsfenster mehr.** Stattdessen
  wird automatisch ein Screenshot aufgenommen und die Meldung an `fehler.log` im
  selben Ordner angehängt. Im Fenster bleibt der Dialog.

## AVS-Import — Treue

| Preset | vorher | nachher |
|---|---|---|
| 15 Alien Alloy | 0,647 | **0,008** |
| 05 Deep Red Sea | 0,943 | **0,011** |
| 07 Milky Way Xtreme | 0,346 | **0,033** |
| 19 High Voltage | 0,126 | **0,005** |
| 20 Inhaler | 0,345 | **0,168** |
| 14 Reflectosphere | 0,174 | **0,085** |
| 06 Wtf I'm Lost | 0,094 | **0,003** |
| greatwho 15/16 (Spiegel-Symmetrie) | 0,042 | **0,0000** |

Die Ursachen, alle am Quelltext der Referenz belegt:

- **Texer II** belegte die Sprite-Farbe **je Punkt** neutral vor — also nach dem
  Frame-Slot, in dem viele Presets sie berechnen. Ergebnis: durchgehend weiße
  Sprites. In „Alien Alloy" sind die vier Texer am Bildrand die einzige
  Energiequelle des Wirbels; das Bild lief deshalb über die Frames nach Schwarz.
- **Mirror** wertete alle vier Richtungen in **einem** Durchgang aus der
  unveränderten Textur aus, die Regeln überschrieben sich. Das Original läuft
  vier Schleifen nacheinander — genau darauf beruht, dass zwei aktive Achsen ein
  symmetrisches Bild ergeben.
- **Custom BPM** ließ jeden `skipVal`-ten statt jeden `skipVal+1`-ten Beat durch;
  außerdem sind die drei Betriebsarten im Original exklusiv, `skipfirst` fehlte
  ganz, und `skipval=0` heißt „jeden Beat".
- **Adjustable-Blend** (BLEND_LINE 7): `v` gewichtet den **Framebuffer**, nicht
  die neue Farbe — wir hatten es vertauscht.
- **Clear Screen** reichte seine eigene Modus-Nummerierung roh in die
  BLEND_LINE-Tabelle. Aus „50/50 gegen Schwarz" wurde „MAX gegen Schwarz", also
  ein No-op: das Bild klang nie ab.

## AVS-Import — Abdeckung

- **`Metaballs 3D` und `Tentacles 3D`** (UnConeD) sind als **Verhaltens-Nachbau**
  umgesetzt — dieselbe Abmachung wie bei FyrewurX: beide APEs sind closed-source,
  ihr Preset-Blob trägt nur eine Farbtafel, die Geometrie ist host-eigen.
  „Yummy Plastics" und „Rubber Starfish" blieben vorher leer und laden jetzt
  warnungsfrei. APE-Bilanz **19 ✅**.
- **MilkDrop-Sampler**: `sampler_` wird nur noch abgeschnitten, wenn es da steht
  (`sampler MilkDrop3_001` fand seine Textur nicht, `sampler tex` — 25 Presets im
  Pack — war ein Absturzpfad); Filter-Präfixe gelten case-insensitiv und in
  beiden Reihenfolgen. `onefish.jpg` ergänzt, Texturensatz vollständig.
- **24 `rot_*`-Rotationsmatrizen** im HLSL-Transpiler samt Matrix-Indizierung.

## Werkzeuge

- **Das Messrauschen ist benannt und auf unserer Seite behoben.** Vier Läufe,
  jeder Renderer mit sich selbst verglichen: unsere Streuung kam vollständig vom
  Beat-Detektor (bis 0,21 MAE) — mit erzwungenem Beat sind wir **0,0000**.
  `compare_avsref.py` fährt deshalb `--beat-period 30` als Vorgabe. AvsRef setzt
  nach jedem Laden ein festes `srand()`; der Rest dort sind die APE-DLLs
  (Fremdcode), Presets mit APEs also mehrfach messen.
- Neue Sondenstufen `6_alloy`, `7_rand`, `8_blend` — letztere mit **unsymmetrischen**
  Adjustable-Werten, weil der alte Wächter mit 50:50 die Vertauschung nicht
  sehen konnte.

## Doku

`docs/Offene_Punkte.md` ist neu der eine Ort für „was ist noch zu tun"; die
veralteten `Offene_Implementierungen.md` und `Offene_Sichttests.md` (Stand
Session 37) sind entfernt. Korrigiert: Modul-Abdeckung (Text und AVI standen als
nicht umgesetzt, obwohl seit S44 gebaut), `AvsChainTranslator.md`, beide
Kalibrier-Protokolle (Befunde S15–S23) und der Projekt-Steckbrief.

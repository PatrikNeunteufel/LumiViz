# Changelog — Session 44 (2026-07-23/24)

Die große **AVS-Kalibrier-Runde**: zwölf Systembefunde über vier
Preset-Sammlungen, zehn davon behoben — plus drei neue Effekte (**Text**,
**AVI**, Comment-Knoten mit eigenem Feld) und das freigegebene
**Vereinheitlichungs-Konzept** für die Skript-Schicht. Tests am Ende:
**412 Cases grün, 0 Skips**; Builds VS-Debug/-Testing (`/WX`) +
Ninja-Clang-Release grün. Sichtungs-SSOT neu:
`docs/visuals/AVS_Sichttest_Protokoll.md`.

## Behoben

- **Sechs Presets crashten die Anwendung** (Dot Plane las bei leerem
  Spektrum über das Puffer-Ende) und **spektrum-getriebene Effekte sahen im
  Chain-Betrieb Stille**: die Mono-Audio-Getter fallen jetzt auf den
  Stereo-L/R-Mix zurück. Folge: das JC-Pack läuft 100/100 crash- und
  schwarzfrei, die „10 schwarzen Korpus-Presets" aus Session 43 waren
  großteils Messartefakte.
- **Alte Presets (AVS-Format 0.1) verloren Starfield/Mosaic & Co.:** deren
  APE-Kennungen tragen Pointer-Werte; der Parser schreibt sie jetzt auf die
  Builtin-Indizes um — die ref-Korpus-Schwarzliste ist damit leer
  („Spacefolding" rendert).
- **„Wird weiß / zu hell":** AVS kennt 10 Linien-Blend-Modi (u. a.
  **Maximum** als Helligkeits-Deckel) — der Import hatte fast alles auf
  Additiv reduziert. Jetzt volle Tabelle in Übersetzer, Renderer und Editor.
- **„Spektrum fehlt" bei SuperScopes:** das Kanal-Feld ist ein Bitfeld
  (Bit „Spektrum statt Waveform" + Kanalwahl) und wurde falsch gelesen;
  zusätzlich rechnete `v` auf falscher Skala. `v` folgt jetzt exakt dem
  Original (interpolierte visdata-Bytes, /128−1, Spektrum-Stille = −1),
  die Spektrum-Frequenzachse entspricht dem Winamp-Vertrag (inkl. der
  leeren Fade-Bänder oben), und die Verstärkung ist als
  Winamp-Sättigungsäquivalent hergeleitet.
- **Movement-Lücke:** die zwei Builtins ohne Formel („slight fuzzify",
  „blocky partial out") sind jetzt als Pixel-Remaps umgesetzt — keine
  „Movement not supported"-Meldungen mehr.
- **Import übernahm Kommentare nicht:** Comment-Knoten tragen ihren Text
  jetzt in einem eigenen Mehrzeilen-Feld im Editor (persistiert in .lvfx).

## Neu

- **Text-Effekt (AVS id 28):** kompletter Port des GDI-Texters auf die
  Qt-Schrift-Engine — Wort-Zyklus (Timer/Beat, Zufallswort/-position,
  Leerphasen), Schriftart/-größe/-stil aus dem Preset, Ausrichtung +
  Prozent-Versatz, Outline/Schatten; Blend wirkt wie im Original nur auf
  den Textpixeln. Mit vollem Editor im Panel.
- **AVI-Effekt (AVS id 32):** Videos laufen über Video for Windows wie im
  Original (auch alte Codecs), inklusive Geschwindigkeits-Drossel,
  Beat-Haltefenster und adaptivem Blend; Videodateien werden vom
  Preset-Ordner aufwärts gefunden. Mit Editor im Panel.
- **Vereinheitlichungs-Konzept** (`docs/visuals/Vereinheitlichung_Konzept.md`):
  ein gemeinsames Skript-Set für AVS/MilkDrop/LumiViz (Audio-Variablen,
  Funktionen, Konstanten), kollisionsfreie Import-Umbenennung, Fahrplan für
  die Portierung der Alt-Visualizer in die Effektkette — alle fünf
  Grundsatz-Entscheide gefällt.
- **AVS-Sichttest-Protokoll** als laufendes Kalibrier-Dokument (Befunde,
  Belege aus den Original-Quellen, Sammlungs-Sweeps).
- Diagnose: Standalone-Screenshots jetzt in echter Auflösung (DPI-Fix).

## Offen

- Movement-Koordinaten im Pixel-Raum + Set-Render-Mode-Reset je Frame
  (letzte belegte Wormhole-Abweichungen); Seite-an-Seite-Urteile gegen
  echtes AVS (Grau-/Weiß-Konvergenzen einzelner Presets);
  HISTORY-Schwarz-Familie; danach Vereinheitlichung Phase V1–V3.

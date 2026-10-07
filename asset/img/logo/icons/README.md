# Datei-Icons

Icons für die Dateitypen, mit denen LumiViz umgeht — als **SVG** (Quelle, so
zeigt sie die App) und als **ICO** (16–256 px, das Format für Datei-Icons unter
Windows).

| Endung(en) | Icon | Quelle | in der App benutzt |
|---|---|---|---|
| `.lvfx` (Einzel-Preset) | `lumiviz.ico` | Handarbeit | Effect Browser (LumiViz-Presets) |
| `.lvfx2` (Preset-Stapel: Kette mit Host-Gruppe) | `lumiviz_lvfx2.ico` „Stapel" | erzeugt aus `lumiviz.ico` | Effect Browser |
| — (Reserve) | `lumiviz_marke.ico` „Marke" | erzeugt aus `lumiviz.ico` | noch nicht |
| `.lvkar` (Karaoke-Preset, S89) | `lumiviz_karaoke.ico` — Marke mit Mikrofon im Kreis | erzeugt aus `lumiviz.ico` | Effect Browser (S89) |
| `.lvtrail` (Trail-Preset der Karaoke-Ebene, S92) | `lumiviz_trail.ico` — Marke mit Schweif im Kreis | erzeugt aus `lumiviz.ico` | Effect Chain: jede Effect List, die einen Lyrics Trail enthält |
| — (Reserve) | `lumiviz_faecher.ico` „Fächer" | erzeugt aus `lumiviz.ico` | noch nicht |
| `.avs` | `avs.ico` | Handarbeit | Effect Browser (AVS-Presets) |
| `.milk`, `.milk2` | `milkdrop.ico` | Handarbeit | Effect Browser (MilkDrop-Presets) |
| `.mp3` | `audio_mp3.svg` / `.ico` | erzeugt | Audio Browser |
| `.wav`, `.aiff` | `audio_wav.svg` / `.ico` | erzeugt | Audio Browser |
| `.flac` u. ä. | `audio_flac.svg` / `.ico` | erzeugt | Audio Browser |
| `.ogg`, `.opus` | `audio_ogg.svg` / `.ico` | erzeugt | Audio Browser |
| `.m4a`, `.aac` | `audio_m4a.svg` / `.ico` | erzeugt | Audio Browser |
| übrige Audioformate | `audio.svg` / `.ico` | erzeugt | Audio Browser |
| `.lrc` (Songtext) | `lyrics_lrc.svg` / `.ico` | erzeugt | noch nicht |
| `.lvcomp` (Komposition des Composers, S100) | **noch keins** — vorgesehen `lumiviz_lvcomp.ico`; das Aussehen ist nicht entschieden (vier Entwürfe, darunter der Fächer) | — | noch nicht; entschieden (Patrik S100): Effect Browser und Panel Composer |
| **die App selbst** (`LumiViz.exe`) | `lumiviz.ico` — **noch nicht eingebaut** (s. u. „Das Icon der App") | Handarbeit | Fenster und Taskleiste zeigen das Standard-Icon |

### Die Icons im Bild

Vorschaubilder (64 px) aller `.ico` dieses Ordners. Sie liegen einmal, unter
`projects/apps/LumiViz/docs/bilder/handbuch/icons/` — das Benutzerhandbuch (§16)
zeigt dieselben Dateien. Neu bauen nach jedem neuen oder geänderten Icon:
`python asset/img/logo/icons/build_previews.py`.

| Bild | Datei | Zugewiesen |
|:---:|---|---|
| ![lumiviz](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz.png) | `lumiviz.ico` | `.lvfx`; vorgesehen auch für die App |
| ![lumiviz_lvfx2](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvfx2.png) | `lumiviz_lvfx2.ico` „Stapel" | `.lvfx2` |
| ![lumiviz_karaoke](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_karaoke.png) | `lumiviz_karaoke.ico` | `.lvkar` |
| ![lumiviz_trail](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_trail.png) | `lumiviz_trail.ico` | `.lvtrail` |
| ![lumiviz_faecher](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_faecher.png) | `lumiviz_faecher.ico` „Fächer" | Reserve — Grundlage für `.lvcomp` (Entscheid Patrik S100: der Fächer; ob mit Zeitstrahl, ist offen) |
| ![lumiviz_marke](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_marke.png) | `lumiviz_marke.ico` „Marke" | Reserve |
| ![avs](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/avs.png) | `avs.ico` | `.avs` |
| ![milkdrop](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/milkdrop.png) | `milkdrop.ico` | `.milk`, `.milk2` |
| ![audio_mp3](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/audio_mp3.png) | `audio_mp3.ico` | `.mp3` |
| ![audio_wav](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/audio_wav.png) | `audio_wav.ico` | `.wav`, `.aiff` |
| ![audio_flac](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/audio_flac.png) | `audio_flac.ico` | `.flac` u. ä. |
| ![audio_ogg](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/audio_ogg.png) | `audio_ogg.ico` | `.ogg`, `.opus` |
| ![audio_m4a](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/audio_m4a.png) | `audio_m4a.ico` | `.m4a`, `.aac` |
| ![audio](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/audio.png) | `audio.ico` | übrige Audioformate |
| ![lyrics_lrc](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lyrics_lrc.png) | `lyrics_lrc.ico` | `.lrc` |

**Diese Tabellen sind die Liste der zugewiesenen Icons** (Stand S100, 2026-10-07).
Ohne eigenes Icon sind heute: Kurven-Presets des Composers (`.lvfade`,
`.lvshape`), Farbverläufe (`.grad`), Knoten-Voreinstellungen (`.json`),
Playlists (`.m3u8`).

Was davon `register_file_icons.ps1` im **Explorer** einträgt (s. u. „Im Explorer
eintragen"; die Liste im Skript und diese Tabelle zusammen pflegen):

| Endung | ProgID | Icon | eingetragen mit |
|---|---|---|---|
| `.lvfx` | `LumiViz.Preset` | `lumiviz.ico` | `-Apply` (eigene Endung) |
| `.lvfx2` | `LumiViz.PresetStack` | `lumiviz_lvfx2.ico` | `-Apply` |
| `.lvkar` | `LumiViz.Karaoke` | `lumiviz_karaoke.ico` | `-Apply` |
| `.lvtrail` | `LumiViz.Trail` | `lumiviz_trail.ico` | `-Apply` |
| `.lvcomp` | `LumiViz.Composition` | `lumiviz_lvcomp.ico` | `-Apply`, sobald die Datei da ist |
| `.avs` | `LumiViz.Avs` | `avs.ico` | `-Apply -Foreign` (fremde Endung) |
| `.milk`, `.milk2` | `LumiViz.MilkDrop` | `milkdrop.ico` | `-Apply -Foreign` |
| `.lrc` | `LumiViz.Lyrics` | `lyrics_lrc.ico` | `-Apply -Foreign` |
| Audioformate | — | — | nie |

Für die drei von Hand gezeichneten Icons gibt es kein SVG; die aus
`lumiviz.ico` abgeleiteten entstehen aus dessen Pixelbild (s. u.).

## ICO-Dateien neu bauen

```
python asset/img/logo/icons/build_icons.py
```

Braucht ein normales Python mit PySide6 und Pillow. Das Skript erzeugt
`lyrics_lrc.svg` und legt zu jeder `*.svg` die gleichnamige `.ico` ab; die drei
Handarbeit-Icons fasst es nicht an.

## Aus `lumiviz.ico` abgeleitete Icons (Session 88)

```
python asset/img/logo/icons/build_lvfx2_icons.py
```

Braucht ein normales Python mit Pillow. Das Skript liest `lumiviz.ico` und
schreibt fünf `.ico` (16–256 px) — neben den dreien unten die beiden Marken
mit eigenem Zeichen, `lumiviz_karaoke.ico` (Mikrofon, S89) und
`lumiviz_trail.ico` (Schweif, S92):

- **Stapel** (`lumiviz_lvfx2.ico`): das Icon vorn, zwei abgedunkelte Kopien
  versetzt dahinter — das Icon für `.lvfx2` (Entscheid Patrik S88: gewählt,
  weil das vordere Icon auch in 32 px noch als LumiViz zu erkennen ist).
- **Marke** (`lumiviz_marke.ico`): das Icon ganz, unten rechts eine runde
  Marke mit drei Ebenen in den Logofarben.
- **Fächer** (`lumiviz_faecher.ico`): drei gleich große Icons diagonal
  gestaffelt.

Marke und Fächer sind **Reserve** (Wunsch Patrik S88: behalten). Ideen für
ihre Verwendung, nichts davon entschieden:

- **Marke** für **Knoten-Voreinstellungen** (`asset/nodepresets/`, heute
  `.json` ohne eigene Endung): „ein Teil von LumiViz, nicht das ganze Preset".
  Patrik: hat auch etwas von einer Palette. Die Marke ließe sich als System
  ausbauen — dasselbe Icon, je Dateiart ein anderes Zeichen im Kreis.
- **Fächer** für eine künftige Datei der **Timeline** (mehrere Presets
  nebeneinander über die Zeit) oder für eine Visual-Playlist.

## Im Explorer eintragen (Windows, Skript seit S100)

Ziel (Patrik S100): alle Datei-Icons sollen im Explorer erscheinen, vorerst
unter Windows. LumiViz selbst trägt nichts ein; dafür gibt es das Skript
`register_file_icons.ps1` in diesem Ordner. Es schreibt nur unter
`HKCU\Software\Classes` (je Benutzer, ohne Administratorrechte):

```
powershell -ExecutionPolicy Bypass -File asset\img\logo\icons\register_file_icons.ps1
powershell -ExecutionPolicy Bypass -File asset\img\logo\icons\register_file_icons.ps1 -Apply
powershell -ExecutionPolicy Bypass -File asset\img\logo\icons\register_file_icons.ps1 -Apply -Foreign
powershell -ExecutionPolicy Bypass -File asset\img\logo\icons\register_file_icons.ps1 -Remove
```

- **Ohne Schalter** ändert es nichts und zeigt nur, was es täte.
- **`-Apply`** trägt die eigenen Endungen ein (`.lvfx`, `.lvfx2`, `.lvkar`,
  `.lvtrail`, `.lvcomp`), **`-Foreign`** dazu `.avs`, `.milk`, `.milk2`, `.lrc`.
  Audioformate nie.
- **`-Remove`** entfernt genau die eigenen Einträge (Schlüssel `LumiViz.*` und
  Endungen, die auf einen davon zeigen).
- Eine Endung, die schon einem anderen Programm gehört, bleibt unberührt und
  wird gemeldet. Hat der Benutzer im Explorer „Öffnen mit … immer" gewählt,
  gilt dessen Icon — das Skript meldet es.
- Die Icons werden mit ihrem **Pfad** eingetragen (dieser Ordner). Zieht das
  Repo oder eine Installation um: `-Remove`, dann am neuen Ort `-Apply`.
- **Nur das Icon, kein „Öffnen mit LumiViz":** `LumiViz.exe <datei>` lädt heute
  keine Datei (geprüft S100: die App wertet ihre Argumente nicht aus). Ein
  Doppelklick im Explorer fragt deshalb weiter, womit geöffnet werden soll.
  Sobald die App eine Datei als Argument öffnet, kommt
  `shell\open\command` dazu.

**Stand S100:** nur der Anzeige-Modus ist gelaufen; `-Apply` und `-Remove`
sind **nicht ausgeführt** worden.

## Das Icon der App (offen, Wunsch Patrik S100)

`LumiViz.exe` zeigt im Explorer und in der Taskleiste heute das Standard-Icon:
die Exe trägt keine Icon-Ressource, und das Fenster setzt kein Icon. Nötig sind
zwei Dinge:

1. **Icon-Ressource in der Exe** — eine `.rc`-Datei mit einer Zeile
   (`IDI_ICON1 ICON "lumiviz.ico"`), die zu den Quellen der App gehört. Das
   Icon der Exe im Explorer, in Verknüpfungen und in der Taskleiste kommt von
   dort. Zu klären: ob CMakeCraft `.rc`-Dateien für Apps mitbaut (MSVC und
   clang) — eine Lücke dort gehört nach CMakeCraft.
2. **Fenster-Icon** — `QApplication::setWindowIcon` beim Start, damit auch
   Titelleiste und Alt+Tab es zeigen (mit der Ressource aus 1 nimmt Qt es
   unter Windows meist von selbst).

## Notiz für einen späteren Installer

Ein Installer sollte das Eintragen **wählbar** machen, je Dateityp getrennt
(das Skript oben tut dasselbe von Hand):

- **Eigene Formate** (`.lvfx`, `.lvfx2`): Icon **und** „Öffnen mit LumiViz" —
  unstrittig, die Endungen gehören LumiViz.
- **Fremde Preset-Formate** (`.avs`, `.milk`, `.milk2`) und `.lrc`: Icon und
  „Öffnen mit" nur auf Wunsch — andere Programme (Winamp, MilkDrop, Player
  mit Songtext-Anzeige) können sie schon belegen.
- **Audioformate** (`.mp3`, `.flac` …): standardmäßig **nichts** ändern; höchstens
  LumiViz unter „Öffnen mit" eintragen, ohne die Zuordnung oder das Icon des
  bisherigen Players zu ersetzen.

Technisch (Windows, je Benutzer, ohne Administratorrechte):
`HKCU\Software\Classes\<.endung>` → ProgID, darunter
`HKCU\Software\Classes\<ProgID>\DefaultIcon` = `"<Pfad>\<icon>.ico"` und
`...\shell\open\command` = `"<Pfad>\LumiViz.exe" "%1"`; danach
`SHChangeNotify(SHCNE_ASSOCCHANGED, …)`, damit der Explorer die Icons neu
lädt. Die Deinstallation muss genau die eigenen Einträge wieder entfernen und
eine vorher bestehende Zuordnung nicht anfassen. Voraussetzung in der App:
`LumiViz.exe <datei>` muss die Datei öffnen (Preset laden bzw. Titel
abspielen) — das ist zu prüfen, bevor der Installer es anbietet.

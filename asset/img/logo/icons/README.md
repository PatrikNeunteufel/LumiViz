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
| `.lvcomp` (Komposition des Composers, S100) | `lumiviz_lvcomp.ico` — der Fächer mit Zeitstrahl und Abspielmarke (Entscheide Patrik S100) | erzeugt aus `lumiviz.ico` | Effect Browser (Doppelklick öffnet die Komposition), Panel Composer vor dem Dateinamen |
| `.lvloop` (Loop, S120) | `lumiviz_lvloop.ico` — Marke mit Schritt-Raster und Bogen zurück zum Anfang | erzeugt aus `lumiviz.ico` | Audio Browser, Loop-Panel (Datei-Zeile) |
| `.lvinst` (Instrument, S120) | `lumiviz_lvinst.ico` — Marke mit Klaviertasten | erzeugt aus `lumiviz.ico` | Instrument-Editor (Kopfzeile), Import data (Liste) |
| `.lvcontour` (Kontur für die Waveform Shape, S120 — die Datei gibt es noch nicht) | `lumiviz_lvcontour.ico` — Marke mit Kontur und Stift | erzeugt aus `lumiviz.ico` | noch nicht |
| `.lvfade` (Blende des Curve Editors, S120) | `lumiviz_lvfade.ico` — Marke mit Keil | erzeugt aus `lumiviz.ico` | Import data (Liste) |
| `.lvshape` (Stück des Curve Editors, S120) | `lumiviz_lvshape.ico` — Marke mit Hügel | erzeugt aus `lumiviz.ico` | Import data (Liste) |
| `.lvmute` (Mute-Profil des Composers, S120) | `lumiviz_lvmute.ico` — Marke mit Lautsprecher und Kreuz | erzeugt aus `lumiviz.ico` | Import data (Liste) |
| `.lvdata` (Export der eigenen Daten, S120) | `lumiviz_lvdata.ico` — Marke mit Ablage und Pfeil | erzeugt aus `lumiviz.ico` | Import data (Kopf) |
| `.grad` (Farbverlauf, S120) | `lumiviz_grad.ico` — Marke mit Verlauf | erzeugt aus `lumiviz.ico` | Import data (Liste, an jedem eigenen Verlauf) |
| **die App selbst** (`LumiViz.exe`, S100) | `lumiviz_app.ico` — `lumiviz.ico` auf ein Quadrat gesetzt, in den üblichen Stufen | erzeugt aus `lumiviz.ico` | Icon-Ressource der Exe (`projects/apps/LumiViz/main/LumiViz.rc`): Explorer, Verknüpfungen, Taskleiste, Fenster |

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
| ![lumiviz_lvcomp](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvcomp.png) | `lumiviz_lvcomp.ico` — Fächer mit Zeitstrahl | `.lvcomp` |
| ![lumiviz_lvloop](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvloop.png) | `lumiviz_lvloop.ico` | `.lvloop` |
| ![lumiviz_lvinst](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvinst.png) | `lumiviz_lvinst.ico` | `.lvinst` |
| ![lumiviz_lvcontour](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvcontour.png) | `lumiviz_lvcontour.ico` | `.lvcontour` |
| ![lumiviz_lvfade](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvfade.png) | `lumiviz_lvfade.ico` | `.lvfade` |
| ![lumiviz_lvshape](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvshape.png) | `lumiviz_lvshape.ico` | `.lvshape` |
| ![lumiviz_lvmute](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvmute.png) | `lumiviz_lvmute.ico` | `.lvmute` |
| ![lumiviz_lvdata](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_lvdata.png) | `lumiviz_lvdata.ico` | `.lvdata` |
| ![lumiviz_grad](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_grad.png) | `lumiviz_grad.ico` | `.grad` |
| ![lumiviz_app](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_app.png) | `lumiviz_app.ico` | die App (`LumiViz.exe`) |
| ![lumiviz_faecher](../../../../projects/apps/LumiViz/docs/bilder/handbuch/icons/lumiviz_faecher.png) | `lumiviz_faecher.ico` „Fächer" | Reserve — die Grundlage von `lumiviz_lvcomp.ico` |
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

**Diese Tabellen sind die Liste der zugewiesenen Icons** (Stand S120, 2026-10-13).
Ohne eigenes Icon sind heute: Knoten-Voreinstellungen (`.json`, ohne eigene
Endung), Playlists (`.m3u8`), die Presets der einzelnen Visualizer (`.lvp` —
in S120 nicht nachgesehen, ob das Format noch geschrieben wird).

**Die acht Icons aus S120** (Entscheid Patrik I1 a: alle als Marke mit einem
Zeichen im Kreis; die Zeichen an vier Vorschaublättern gewählt — für `.lvloop`
das Schritt-Raster mit Bogen statt zweier Pfeile, die wie „aktualisieren"
aussahen; für `.lvdata` die gefüllte Ablage). Gefüllte Flächen statt dünner
Striche, damit bei 32 px etwas stehen bleibt; bei 16 px ist kein Zeichen der
Familie zu lesen, auch das Mikrofon nicht. **In der App** (Entscheid Patrik A2 a)
stehen sie, wo ein Dateiname steht oder Dateien verschiedener Art
untereinander: Audio Browser und Datei-Zeile des Loop-Panels (`lvloop`),
Kopfzeile des Instrument-Editors (`lvinst`), Dialog Import data — vor dem
Namen der Datei (`lvdata`) und in der Liste „je Datei entscheiden" je nach
Endung (`dateiIconFuerEndung`; ein Verlauf ist dort eine `.json` und trägt
das Icon seines Bereichs). Bewusst ohne: das Fenster Instruments, der
Creator, die Menüs der Blenden, Stücke und Mute-Profile, die Verlaufs-Combos
— dort ist jede Zeile von derselben Art. Geladen wird über
`dateiArtIcon(<endung>)` in `UI/widgets/PresetTypeIcons.hpp`. `.grad` gilt im Skript für den Explorer als fremde Endung (von
Claude gesetzt: sie trägt kein `lv` und kann anderen Programmen gehören).

Was davon `register_file_icons.ps1` im **Explorer** einträgt (s. u. „Im Explorer
eintragen"; die Liste im Skript und diese Tabelle zusammen pflegen):

| Endung | ProgID | Icon | eingetragen mit |
|---|---|---|---|
| `.lvfx` | `LumiViz.Preset` | `lumiviz.ico` | `-Apply` (eigene Endung) |
| `.lvfx2` | `LumiViz.PresetStack` | `lumiviz_lvfx2.ico` | `-Apply` |
| `.lvkar` | `LumiViz.Karaoke` | `lumiviz_karaoke.ico` | `-Apply` |
| `.lvtrail` | `LumiViz.Trail` | `lumiviz_trail.ico` | `-Apply` |
| `.lvcomp` | `LumiViz.Composition` | `lumiviz_lvcomp.ico` | `-Apply` |
| `.lvloop` | `LumiViz.Loop` | `lumiviz_lvloop.ico` | `-Apply` |
| `.lvinst` | `LumiViz.Instrument` | `lumiviz_lvinst.ico` | `-Apply` |
| `.lvcontour` | `LumiViz.Contour` | `lumiviz_lvcontour.ico` | `-Apply` |
| `.lvfade` | `LumiViz.Fade` | `lumiviz_lvfade.ico` | `-Apply` |
| `.lvshape` | `LumiViz.CurvePiece` | `lumiviz_lvshape.ico` | `-Apply` |
| `.lvmute` | `LumiViz.Mute` | `lumiviz_lvmute.ico` | `-Apply` |
| `.lvdata` | `LumiViz.Data` | `lumiviz_lvdata.ico` | `-Apply` |
| `.grad` | `LumiViz.Gradient` | `lumiviz_grad.ico` | `-Apply -Foreign` |
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
schreibt fünfzehn `.ico` (16–256 px) — neben den dreien unten, dem Icon der
Komposition und dem der App die Marken mit eigenem Zeichen:
`lumiviz_karaoke.ico` (Mikrofon, S89), `lumiviz_trail.ico` (Schweif, S92) und
die acht aus S120 (`lumiviz_lvloop.ico` … `lumiviz_grad.ico`). Ein Lauf
schreibt alle neu; die bestehenden kamen in S120 Byte für Byte gleich heraus:

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
  `.lvtrail`, `.lvcomp`, seit S120 `.lvloop`, `.lvinst`, `.lvcontour`,
  `.lvfade`, `.lvshape`, `.lvmute`, `.lvdata`), **`-Foreign`** dazu `.avs`,
  `.milk`, `.milk2`, `.lrc`, `.grad`. Audioformate nie.
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

**Stand S120:** auf Patriks Rechner sind die fünf eigenen Endungen aus S100
und die vier fremden eingetragen (aus der Registry gelesen, 13.10.2026). Für
die acht neuen ist nur der Anzeige-Modus gelaufen; `-Apply` führt Patrik aus.

## Das Icon der App (S100, Wunsch Patrik)

`LumiViz.exe` trägt seit S100 eine Icon-Ressource:
`projects/apps/LumiViz/main/LumiViz.rc` mit der Zeile
`IDI_ICON1 ICON "…/lumiviz_app.ico"`, eingetragen in `main/Source.cmake` (nur
unter Windows). CMake übersetzt sie mit dem Ressourcen-Compiler des Toolsets;
am Build-System war nichts zu ändern. Geprüft: beide Bauarten (MSVC, clang)
bauen, und das aus der Exe gelesene Icon ist das LumiViz-Icon.

`lumiviz_app.ico` entsteht aus `lumiviz.ico` (`build_lvfx2_icons.py`): auf ein
Quadrat gesetzt, in den sieben üblichen Stufen. `lumiviz.ico` selbst ist nicht
quadratisch und trägt nur eine sehr große Stufe.

**Nicht geprüft:** Taskleiste, Titelleiste und Alt+Tab im laufenden Programm.
Qt nimmt unter Windows `IDI_ICON1` auch als Fenster-Icon; zeigt das Fenster es
nicht, fehlt ein `QApplication::setWindowIcon` beim Start. Der Explorer merkt
sich Icons — nach einem Neubau zeigt er das neue unter Umständen erst nach
einem Ab- und Anmelden.

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

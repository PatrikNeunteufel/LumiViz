# Datei-Icons

Icons für die Dateitypen, mit denen LumiViz umgeht — als **SVG** (Quelle, so
zeigt sie die App) und als **ICO** (16–256 px, das Format für Datei-Icons unter
Windows).

| Endung(en) | Icon | Quelle | in der App benutzt |
|---|---|---|---|
| `.lvfx` (Einzel-Preset) | `lumiviz.ico` | Handarbeit | Effect Browser (LumiViz-Presets) |
| `.lvfx2` (Preset-Stapel: Kette mit Host-Gruppe) | `lumiviz_lvfx2.ico` „Stapel" | erzeugt aus `lumiviz.ico` | Effect Browser |
| — (Reserve) | `lumiviz_marke.ico` „Marke" | erzeugt aus `lumiviz.ico` | noch nicht |
| `.lvkar` (Karaoke-Preset, S89) | `lumiviz_karaoke.ico` — Marke mit Mikrofon im Kreis | erzeugt aus `lumiviz.ico` | noch nicht (Zuordnung im Effect Browser folgt mit dem Karaoke-Panel) |
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
schreibt drei `.ico` (16–256 px):

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

## Notiz für einen späteren Installer

LumiViz registriert heute **keine** Dateitypen in Windows — die Icons erscheinen
nur in der App. Ein Installer (oder ein Einrichtungsskript) sollte das
**wählbar** machen, je Dateityp getrennt:

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

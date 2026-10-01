# Datei-Icons

Icons für die Dateitypen, mit denen LumiViz umgeht — als **SVG** (Quelle, so
zeigt sie die App) und als **ICO** (16–256 px, das Format für Datei-Icons unter
Windows).

| Endung(en) | Icon | Quelle | in der App benutzt |
|---|---|---|---|
| `.lvfx`, `.lvfx2` | `lumiviz.ico` | Handarbeit | Effect Browser (LumiViz-Presets) |
| `.avs` | `avs.ico` | Handarbeit | Effect Browser (AVS-Presets) |
| `.milk`, `.milk2` | `milkdrop.ico` | Handarbeit | Effect Browser (MilkDrop-Presets) |
| `.mp3` | `audio_mp3.svg` / `.ico` | erzeugt | Audio Browser |
| `.wav`, `.aiff` | `audio_wav.svg` / `.ico` | erzeugt | Audio Browser |
| `.flac` u. ä. | `audio_flac.svg` / `.ico` | erzeugt | Audio Browser |
| `.ogg`, `.opus` | `audio_ogg.svg` / `.ico` | erzeugt | Audio Browser |
| `.m4a`, `.aac` | `audio_m4a.svg` / `.ico` | erzeugt | Audio Browser |
| übrige Audioformate | `audio.svg` / `.ico` | erzeugt | Audio Browser |
| `.lrc` (Songtext) | `lyrics_lrc.svg` / `.ico` | erzeugt | noch nicht |

Für die drei von Hand gezeichneten Icons gibt es kein SVG.

## ICO-Dateien neu bauen

```
python asset/img/logo/icons/build_icons.py
```

Braucht ein normales Python mit PySide6 und Pillow. Das Skript erzeugt
`lyrics_lrc.svg` und legt zu jeder `*.svg` die gleichnamige `.ico` ab; die drei
Handarbeit-Icons fasst es nicht an.

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

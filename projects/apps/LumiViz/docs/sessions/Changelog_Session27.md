# Changelog - Session 27 (2025-12-31)

> **Thema:** Audio Integration - FLAC Support, Shuffle/Loop, Multi-Selection, Panel-Aufteilung

---

## Neue Features

### 🎵 FLAC Metadata Support

**BassEngine.cpp** unterstützt jetzt vollständiges Metadata-Parsing:

| Format | Tags | Encoding |
|--------|------|----------|
| MP3 | ID3v2, ID3v1 | UTF-16, Latin-1 |
| FLAC | Vorbis Comments | UTF-8 |
| OGG | Vorbis Comments | UTF-8 |
| WavPack | APE Tags | UTF-8 |
| M4A/AAC | MP4 Tags | UTF-8 |

**Plugin Auto-Loading:** Plugins werden automatisch beim Start aus dem EXE-Verzeichnis geladen.

---

### 🔁 Single-Track Loop (PlayerPanel)

Neuer Loop-Button im PlayerPanel für `RepeatMode::One`:

```
[⏮] [▶/⏸] [⏹] [⏭] [🔁]     🔊━━━○
                    ↑
              Single Loop
```

- Wiederholt nur den aktuellen Track
- Synchronisiert mit PlaylistPanel über `PlaybackModeChangedEvent`

---

### 🔀 Shuffle & Playlist Loop (PlaylistPanel)

Button-Layout aktualisiert:

```
[+] [🗑️] [🔄]  [💾] [📂]        [🔀] [🔁]
```

- **🔀 Shuffle**: Zufällige Track-Auswahl
- **🔁 Playlist Loop**: `RepeatMode::All` - Neustart nach letztem Track

---

### 📋 Multi-Selection (PlaylistPanel)

`ExtendedSelection` aktiviert:

| Aktion | Verhalten |
|--------|-----------|
| Klick | Einzelauswahl |
| Ctrl+Klick | Toggle |
| Shift+Klick | Bereich |
| Ctrl+Shift+Klick | Bereich hinzufügen |

- **Remove** löscht alle ausgewählten Tracks (von hinten nach vorne)
- **Aktueller Track** nur noch bold markiert (keine Hintergrundfarbe)

---

### ⚙️ Panel-Aufteilung: ConfigPanel & SettingsPanel

**Vorher:** ConfigPanel hatte Audio, Visuals, Performance Tabs

**Nachher:**

| Panel | ID | Inhalt |
|-------|-----|--------|
| **ConfigPanel** | `config` | Nur Visualizer: Smoothing, Peak Hold, Color Scheme |
| **SettingsPanel** (NEU) | `settings` | Audio + Performance Tabs |

---

## Geänderte Dateien

### Audio System

| Datei | Änderung |
|-------|----------|
| `BassEngine.cpp` | Plugin Auto-Loading, Metadata Parsing (ID3v2, Vorbis, APE, MP4) |
| `AudioPlayer.cpp` | `publishPlaybackModeChanged()` mit `repeatMode` statt `loop` |
| `AudioEvents.hpp` | `PlaybackModeChangedEvent.repeatMode` (0=None, 1=One, 2=All) |

### Panels

| Datei | Änderung |
|-------|----------|
| `PlayerPanel.hpp/cpp` | Loop-Button, `m_loopEnabled`, `updateLoopButton()` |
| `PlaylistPanel.hpp/cpp` | Multi-Selection, Icons getauscht, kein Background |
| `ConfigPanel.hpp/cpp` | Vereinfacht auf Visualizer-Konfiguration |
| `SettingsPanel.hpp/cpp` | **NEU** - Audio + Performance |
| `PanelAutoReg.cpp` | SettingsPanel registriert |

### CMake

| Datei | Änderung |
|-------|----------|
| `src/UI/panels/Source.cmake` | SettingsPanel.cpp hinzugefügt |
| `include/UI/panels/Source.cmake` | SettingsPanel.hpp hinzugefügt |

---

## RepeatMode Übersicht

| Wert | Konstante | Panel | Icon | Beschreibung |
|------|-----------|-------|------|--------------|
| 0 | `None` | - | - | Kein Repeat |
| 1 | `One` | PlayerPanel | 🔁 | Aktueller Track |
| 2 | `All` | PlaylistPanel | 🔁 | Gesamte Playlist |

**Synchronisation:** Beide Panels subscriben `PlaybackModeChangedEvent` und aktualisieren ihren Button-State entsprechend.

---

## Bugfixes

- **FLAC Tags nicht erkannt:** Plugins wurden nicht geladen → Auto-Loading in `initialize()`
- **UTF-16 Garbage Characters:** ID3v2 Encoding-Byte wurde ignoriert → vollständiges Parsing
- **Playlist Background:** Aktueller Track war dunkel hinterlegt → nur bold Font

---

## Neue Panel hinzufügen - Kurzreferenz

```bash
1. include/UI/panels/MyPanel.hpp erstellen
2. src/UI/panels/MyPanel.cpp erstellen
3. In PanelAutoReg.cpp: registry.registerPanel(...)
4. In include/UI/panels/Source.cmake: MyPanel.hpp
5. In src/UI/panels/Source.cmake: MyPanel.cpp
6. cmake --preset reconfigure
```

---

## Nächste Schritte

- [ ] Visualizer-Konfiguration mit Events verbinden
- [ ] AudioAnalyzer für Beat Detection
- [ ] Weitere Visualizer-Effekte

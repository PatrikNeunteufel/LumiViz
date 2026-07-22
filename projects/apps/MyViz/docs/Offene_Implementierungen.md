# MyViz — Offene Implementierungen

> **Stand:** 2026-07-21 (Ende Session 37) · **Typ:** Status/Backlog
> **Entscheide (Patrik, Session 37):** A ✅ ja, B ✅ ja (module×slot), E ✅ behalten.
> **Reihenfolge:** nächste Session zuerst **etwas anderes** (TBD), danach **1 → 3 → 2**
> (siehe „Geplant").

Legende: 🟢 geplant/entschieden · 🟡 später · ⚪ nur Notiz (Zukunft) · 🔧 Kleinkram

---

## Geplant (entschieden) — Reihenfolge 1 → 3 → 2

### 🟢 P1 — Set Render Mode auf ALLE Scope-Effekte (Entscheid A: ja)
Aktuell liest nur `runSuperScope` den `m_renderMode`. Ausdehnen auf: Simple,
Oscilliscope Star, Ring, Rotating Stars, Bass Spin, Moving Particle (Linienbreite
+ Blend übernehmen). Effektiv: die glBlendFunc/Größe in diesen ~6 Handlern aus
`m_renderMode` ableiten, wenn `set`. Klein, abgegrenzt.

### 🟢 P3 — Skript-System verfeinern (Entscheid B: module×slot-genau)
1. **SSOT-Symboltabelle** Modul×Slot×Name → Kategorie/Typ/Range/Text (ersetzt die
   globale `symbolCategory` + den Hand-HTML-Referenztext).
2. **Referenz aus der Tabelle generieren** (statt Hand-HTML je Modul).
3. **Kategorie-Highlighter modul-bewusst** (Editor kennt aktives Modul+Slot).
4. **Fehler-Markierung präziser** (read-only je Slot statt global konservativ).
   Bezug: `docs/visuals/Skript_Variablen_Konzept.md` §3/§8 Schritt 1–4 (Verfeinerung).

### 🟢 P2 — Visual-Playlist + Hotkeys
Preset-Queue mit Auto-Wechsel (Seq/Random; Songwechsel/Timer/Beat),
`IVisualPlaylist` + `VisualPlaylistPanel`, zentraler Hotkey-Layer (Audio-Hotkeys
existieren NOCH NICHT — nur Buttons + Esc), „zur Playlist"-Aktion im Import-Browser.
**Hat eigene offene Design-Entscheide** — Konzept `docs/visuals/ui/Visual_Playlist_Konzept.md` §6.

---

## Später (empfohlen zurückgestellt)

- 🟡 **Variable-Set Variante B** — benannte Globals (`speed` statt `reg07`), Host
  mappt Name→reg-Slot bzw. `ScriptContext`-Named-Global-Map. (A = „Global Variables"
  deckt den Bedarf.)
- 🟡 **Text-Effekt (Builtin 28)** — volles GDI/QPainter-Textrendering (Font/Layout).
  Letzte AVS-Import-Lücke, hoher Aufwand; aktuell no-op + Notiz.
- 🟡 **Erweiterte Audio-Vars** — ~~`bass_att/mid_att/treb_att`~~ ✅ **M2 (S39):**
  `MilkLoudness`-Modul + symbolCategory-Namen (Einspeisung macht der
  MilkdropVisualizer in M3). Noch offen: `peak`, `rms`; `fps/frame/progress`
  als Namen registriert, Werte liefert erst der Milk-Host.
- 🟡 **MilkDrop-Import (Roadmap 6)** — eigener Plan; **nach** verifiziertem AVS-Import.
- 🟡 **Assets-Ordner-Fallback** für Bild-Lader (konfigurierbarer Ordner; Nutzung
  durch Texer II / Picture II / Texer).
- 🟡 **Multi-Drag absichern** — falls das Block-Reparenting (Entscheid E: behalten)
  im Sichttest Bugs zeigt: auf „Reorder in gleicher Ebene" beschränken.

---

## Nur Notiz (Zukunft — bewusst nichts machen)

- ⚪ **Dynamische Modulparameter** — alle Params per init/frame/beat/point wie
  SuperScope; erst Basis prüfen. (Memory: `zukunft-dynamische-modulparameter`)
- ⚪ **Standalones → LumiViz-Module** — Equalizer/Oszilloskop/Pulsing als
  MultiEffect-Module. (Memory: `zukunft-standalones-zu-lumiviz-modulen`)
- ⚪ **Custom-Functions-Modul** — Modul zur Definition eigener Funktionen (Schwester
  zum Variable-Set-Modul). (Memory: `zukunft-custom-functions-modul`)
- ⚪ **Stereo bass/mid/treb** — Kurzvars per-Kanal (aktuell mono; L/R via getspec/getosc).
- ⚪ **Video-Capture-Modul** (Idee Patrik, 2026-07-22) — Visual + Audio zusammen
  als Video aufnehmen; Qualität (und damit Dateigröße) **je Aufnahme** einstellbar
  (Auflösung/fps/Bitrate/Codec). Kandidaten: FFmpeg-Pipe oder Qt6 QMediaRecorder/
  QScreenCapture; Frames aus dem FBO des Render-Threads abgreifen (Threading-
  Vertrag §12 beachten), Audio aus der BASS-Pipeline. Passt zu Visual-Playlist
  (P2: ganze Sets aufzeichnen). (Memory: `zukunft-video-capture-modul`)

---

## Kleinkram / Kür (aus früheren Sessions)

- 🔧 Pulsing-Defaults-Mismatch · `File → Open Audio…`-Stub · Undock-Dauertest ·
  Waveform-Glättungs-Default.
- 🔧 en-Übersetzungen (de=SSOT) · Bass-PreFetch-Hook · `CMakeUserPresets.json` →
  `.example` · App-Umbenennung **MyViz → LumiViz**.

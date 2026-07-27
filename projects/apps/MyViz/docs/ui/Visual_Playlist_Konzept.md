# MyViz — Visual-Playlist, Import-Browser-Erweiterung & Hotkeys (Konzept)

> **Version:** 0.1.0
> **Datum:** 2026-07-21
> **Typ:** Konzept (Vorschlag — Umsetzung folgt später)
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Import_Multieffekt_Host_Entwurf.md](../visuals/Import_Multieffekt_Host_Entwurf.md) ·
> `ImportBrowserPanel` · `PlaylistPanel` (Audio) · `IAudioPlayer`/`IPlaylist`
> **Sprache:** Deutsch

---

## 1. Begriffsklärung (drei verschiedene Dinge)

Wichtig, weil sie leicht verwechselt werden:

| Begriff | Was | Analogie |
|---|---|---|
| **EffectChain / Effect List** | Die **interne Komposition eines einzelnen** Presets — mehrere Effekte übereinander (der Multieffekt-Host-Baum). | Die *Spuren* eines Songs |
| **Visual-Playlist** (neu) | Eine **geordnete Folge ganzer Presets/Visuals**, durch die die App weiterschaltet. | Die *Playlist* von Songs |
| **Import-Browser** | Ein **Datei-Browser** (Ordnerinhalt), zum Sichten/Importieren einzelner Dateien. | Der *Datei-Explorer* |

Der Import-Browser ist also **kein** Playlist-Ersatz und **keine** EffectList — er ist die
Quelle, aus der man Presets in die Visual-Playlist (oder direkt in den Host) zieht.

---

## 2. Import-Browser — Erweiterung

**Ist-Zustand:** browst einen Ordner, Filter AVS/MilkDrop/Beide, Doppelklick importiert.

**Erweiterung:**
1. **Mehr Typen browsen:** Filter erweitern um **`.lvfx`** (native Effektketten) und
   später weitere Preset-Formate. Filter-Combo: `AVS · MilkDrop · LumiViz (.lvfx) · Alle`.
   Doppelklick auf `.lvfx` → `loadChainFile` statt `loadAvsFile` (Dispatch nach Endung).
2. **Aktionen pro Eintrag** (Kontextmenü / Buttons):
   - *Laden* (wie bisher, Doppelklick).
   - ***Zur Visual-Playlist hinzufügen*** (Einzeldatei oder ganzer Ordner rekursiv).
   - *Im Ordner anzeigen*.
3. **Mehrfachauswahl** → „alle zur Playlist".

Bewusst **nicht** hier: Auto-Wechsel/Timing — das ist Sache der Visual-Playlist (§3).
Der Browser bleibt ein Browser.

---

## 3. Visual-Playlist (Preset-Queue) — Kernkonzept

Eine eigene, **von der Audio-Playlist getrennte** Warteschlange ganzer Visuals.

### 3.1 Datenmodell
- Geordnete Liste von **Preset-Referenzen**: `{ typ: avs|lvfx|milk, pfad, anzeigename }`.
  (Optional später: eingebettete `.lvfx`-Kopie, damit die Playlist self-contained ist —
  konsistent mit dem Bild-Einback-Entscheid.)
- Aktueller Index; Historie für „zurück".
- Persistenz als `.lvpl` (JSON) — wiederverwendbar wie M3U bei Audio.

### 3.2 Auto-Wechsel-Modi (An/Aus + Reihenfolge)
- **Aus** (Standard): nur manueller Wechsel (Hotkey/Klick).
- **Sequenziell**: der Reihe nach, mit Loop-Option.
- **Zufällig**: Shuffle (ohne sofortige Wiederholung; „Bag-Shuffle").

### 3.3 Auslöser (wann gewechselt wird)
Orthogonal zum Modus — ein oder mehrere aktiv:
- **Manuell**: Hotkey / Panel-Buttons (immer verfügbar).
- **Bei Songwechsel**: hört auf `PlaylistIndexChangedEvent` des Audio-Players →
  neues Visual pro Track (der „ein Song, ein Look"-Klassiker).
- **Per Timing**: Timer mit konfigurierbarem Intervall (z. B. 30 s), optional
  **beat-quantisiert** (Wechsel erst am nächsten Beat → kein hartes Springen;
  nutzt den vorhandenen `BeatEstimator`/`m_frameBeat`).
- (Kür) **OnBeat-N**: alle N Beats.

### 3.4 Übergang
- v1: **harter Schnitt** (sofort laden).
- Kür: **Crossfade** zwischen altem und neuem Visual (zwei Host-Instanzen + Blend-Pass) —
  eigener Aufwand, später.

### 3.5 UI
- **`VisualPlaylistPanel`** (nach Vorbild `PlaylistPanel`): Liste, Add/Remove/Clear,
  Save/Load `.lvpl`, Doppelklick lädt, aktuelles Preset **fett**; Toolbar mit
  **Auto-Wechsel An/Aus**, **Modus** (Seq/Random), **Auslöser** (Song/Timer +
  Intervall-Feld), **Prev/Next**.
- Registrierung wie andere Panels (`PanelAutoReg`, „View/Panels").

### 3.6 Verdrahtung
- Neuer Dienst **`IVisualPlaylist`** (im `ServiceContainer`), analog `IPlaylist`.
- Events: `VisualPlaylistChangedEvent`, `VisualPresetChangedEvent(index)` —
  das Panel und ggf. die Statusleiste reagieren.
- Laden eines Presets = bestehender Pfad (`loadAvsFile`/`loadChainFile` am
  Multi-Effect-Host) — der Auto-Switch auf den Host (schon vorhanden) greift.
- **Threading:** Ketten-Wechsel läuft wie gehabt über `renderMutex()` +
  `m_pendingRuntimeReset` (Render-Thread gibt alte GL-Runtimes frei).

---

## 4. Hotkeys → eigenes Konzept

Hotkeys sind seit Session 51 in **[Hotkey_Konzept.md](Hotkey_Konzept.md)** geregelt
(SSOT) — dieser Abschnitt ist nur noch der Verweis. Grund für die Auslagerung: die
Belegung muss über drei Ausbaustufen stabil bleiben (aktives Verzeichnis →
Visual-Playlist → Composer), die Transporttasten sind dauerhaft für den Audio-Player
reserviert, und die Konfigurierbarkeit ist keine Kür mehr, sondern Teil der Stufe 1.

Für dieses Konzept relevant: `preset.next`/`preset.previous` sind **Ereignisse**
(`PresetStepEvent`), nicht Methoden. Wenn die Visual-Playlist kommt, wechselt nur der
**Empfänger** dieses Ereignisses vom Import-Browser zur Queue — an der
Tastenbelegung ändert sich nichts.

---

## 5. Zusammenspiel (ein Bild)

```
Import-Browser ──"zur Playlist"──►  Visual-Playlist ──lädt──►  Multi-Effect-Host
   (Dateien)                        (Preset-Queue)             (rendert 1 Preset)
                                        ▲   ▲
                 Songwechsel ───────────┘   └─────── Timer / Beat / Hotkey
              (PlaylistIndexChangedEvent)            (Auslöser)
```

---

## 5a. Ausblick: Composer (Fernziel, Patrik S51)

Die Visual-Playlist ist die **eindimensionale** Vorstufe. Das Fernziel ist ein
**Composer**: mp3 und Presets in mehreren **Spuren** über der Zeit arrangiert, um ein
ganzes **Programm** zu bauen (Analogie: eine DAW-Anordnung, nur mit Visuals als
Spurinhalt). Was daraus schon jetzt folgt:

- Eine Preset-Referenz braucht später einen **Zeitbezug** (Start, Dauer) — das
  Datenmodell §3.1 sollte erweiterbar bleiben, nicht auf „nur Reihenfolge" festgelegt.
- Der Übergang (§3.4) wird dort zum **Spur-Blend**; ein Crossfade ist damit keine Kür
  mehr, sondern Vorarbeit.
- Die Auslöser (§3.3) werden zu einer **Zeitachse**; „bei Songwechsel" ist deren
  einfachster Spezialfall.
- Hotkeys bleiben unverändert — sie sprechen Aktionen an, nicht Quellen
  ([Hotkey_Konzept.md](Hotkey_Konzept.md) §1).

Kein Umsetzungsauftrag, nur die Richtung, damit die Playlist-Struktur nicht dagegen
gebaut wird.

---

## 6. Offene Entscheide (vor Umsetzung)

1. **Playlist-Inhalt:** nur Pfad-Referenzen (klein, kann brechen) vs. `.lvfx`
   eingebettet (self-contained, größer) — Empfehlung: Referenzen v1, Einbetten Kür.
2. **Hotkey-Belegung:** Pfeil vs. Ctrl+Pfeil für Audio/Visual; welche Tasten für
   Auto/Modus.
3. **Auslöser-Default:** Auto-Wechsel initial Aus (Empfehlung) · welcher Auslöser
   voreingestellt (Songwechsel vs. Timer).
4. **Beat-Quantisierung** des Timer-Wechsels: v1 (empfohlen) oder Kür.
5. **Import-Browser-Erweiterung jetzt** (klein, `.lvfx`-Filter + „zur Playlist") oder
   zusammen mit der Playlist.

---

## 7. Grober Umsetzungs-Schnitt (wenn freigegeben)

1. Import-Browser: `.lvfx`-Filter + Endungs-Dispatch + „zur Playlist"-Aktion.
2. `IVisualPlaylist` + `VisualPlaylistPanel` + Events + `.lvpl`-Persistenz.
3. Auto-Wechsel-Engine (Modus + Auslöser: Songwechsel-Hook, Timer, optional Beat).
4. Zentraler Hotkey-Layer (Audio + Visual).
5. (Kür) Crossfade, Hotkey-Editor.

---

## 8. Changelog

- **0.2.0** (2026-07-27, Session 51): §4 Hotkeys nach
  [Hotkey_Konzept.md](Hotkey_Konzept.md) ausgelagert (dort auch die
  Reservierungs-Regel für die Transporttasten und der Settings-Editor) · §5a
  Ausblick Composer ergänzt (Spuren über der Zeit — Folgen für Datenmodell,
  Übergang und Auslöser).
- **0.1.0** (2026-07-21): Erstfassung — Begriffsklärung, Import-Browser-Erweiterung,
  Visual-Playlist (Modi/Auslöser/UI/Verdrahtung), Hotkey-Vorschlag, Entscheide.

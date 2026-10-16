# LumiViz — Hotkeys (Konzept)

> **Version:** 1.3.0
> **Datum:** 2026-10-16
> **Typ:** Konzept (**Stufen 1 + 2 umgesetzt**, Session 51/52; §9 umgesetzt S129)
> **Status:** Alle Entscheide §9 gefallen (§9.1 S52, §9.2–§9.5 Patrik S60) und gebaut
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Visual_Playlist_Konzept.md](Visual_Playlist_Konzept.md) (§4 verweist hierher) ·
> [Menu_System.md](Menu_System.md) (`MenuRegistry` trägt schon Shortcut-Slots) ·
> `ImportBrowserPanel` · `PlaylistPanel` · `SettingsPanel`
> **Sprache:** Deutsch

---

## 1. Warum ein eigenes Konzept

Hotkeys sind hier kein Komfort-Detail, sondern die **Bedien-Schnittstelle einer
Live-Anwendung**: wer ein Visual vorführt, hat keine Hand für Menüs. Die Belegung
muss deshalb *einmal* festgelegt werden und über drei Ausbaustufen stabil bleiben —
ein Hotkey, den Anwender gelernt haben, darf später nicht die Bedeutung wechseln.

| Stufe | Wodurch geblättert wird | Status |
|---|---|---|
| **1. Verzeichnis** | das **aktive Verzeichnis des Import-Browsers** — vor/zurück durch die Presets darin | **jetzt** |
| **2. Visual-Playlist** | eine kuratierte Preset-Queue ([Konzept](Visual_Playlist_Konzept.md) §3) | später |
| **3. Composer** | ein Programm aus **Spuren**: mp3 und Presets über der Zeit arrangiert | Fernziel |

Dieselben Tasten bedienen in allen drei Stufen dieselbe *Absicht* („nächstes
Preset"). Was sich ändert, ist nur die **Quelle**, aus der das nächste Preset kommt.
Darum ist die Aktion `preset.next` von Anfang an ein Ereignis und keine
Browser-Methode — Stufe 2 und 3 hängen sich an dasselbe Ereignis, und keine
Tastenbelegung muss angefasst werden.

---

## 2. Reservierungs-Regel (der wichtigste Entscheid)

**Die Standard-Transporttasten gehören dem Audio-Player — dauerhaft.** Sie sind aus
jeder anderen Kategorie ausgeschlossen; der Editor lehnt eine solche Zuweisung ab
(§6). Seit Stufe 2 (Session 52) sind sie auch verdrahtet.

| Reserviert | Aktion | Begründung |
|---|---|---|
| `Space` | Play/Pause | Standard in jedem Player; muss frei bleiben, auch wenn Presets „springen" naheliegend wäre |
| `Ctrl+→` / `Ctrl+←` | nächster / voriger **Song** | Standard |
| `Ctrl+↑` / `Ctrl+↓` | Lautstärke | Standard |
| `Media Play/Pause`, `Media Next`, `Media Prev`, `Media Stop` | Transport | Systemtasten, gehören der Audio-Wiedergabe |

Konsequenz für Presets: **keine** Transporttaste, kein `Space`. Die Preset-Navigation
bekommt eigene, unmodifizierte Tasten (§4) — unmodifiziert, weil sie im Betrieb am
häufigsten gebraucht wird.

---

## 3. Aktions-Modell

Eine Aktion ist ein **stabiler Bezeichner**, keine Taste. Die SSOT ist eine Tabelle
im Code (`include/services/ShortcutRegistry.hpp`) mit je Aktion:

| Feld | Zweck |
|---|---|
| `id` | punktierter Bezeichner, z. B. `preset.next` — wandert nie |
| `label` | Anzeigename im Editor |
| `category` | `Transport` · `Preset` · `Ansicht` · `Kette` — gliedert den Editor und trägt die Reservierungs-Regel |
| `defaultSequence` | Vorbelegung als `QKeySequence`-Text |
| `reserved` | true = nur diese Kategorie darf die Taste haben |

> **Falle (Session 52):** `defaultSequence` ist **Qt-Text**, keine freie
> Beschreibung. Qt kennt die Bildtasten als `PgDown`/`PgUp` — das plausibel
> aussehende `"PageDown"` ergibt `Qt::Key_unknown` und damit eine **tote**
> Vorbelegung, ohne jede Fehlermeldung. Wächter: `test_Shortcuts.cpp` schickt
> jede Vorbelegung durch `QKeySequence` und wieder zurück.

Ausgelöst wird **nie** direkt Logik, sondern ein Ereignis auf dem EventBus
(`PresetStepEvent{delta}` usw.). Damit gilt für jede Aktion:

- Eine Taste, ein Ereignis, **ein** Empfänger-Vertrag — keine Logik-Doppelung
  zwischen Hotkey, Menü und Panel-Button (alle drei feuern dasselbe Ereignis).
- Stufe 2/3 ersetzen nur den Empfänger.

---

## 4. Vorbelegung (Vorschlag)

**Transport** (reserviert, §2): wie in der Tabelle dort.

**Preset**
| Taste | `defaultSequence` | Aktion-ID | Wirkung Stufe 1 |
|---|---|---|---|
| `Bild ab` | `PgDown` | `preset.next` | nächstes Preset im aktiven Verzeichnis des Import-Browsers |
| `Bild auf` | `PgUp` | `preset.previous` | voriges Preset |
| — | — | `preset.reload` | aktuelles Preset neu laden (nützlich beim Kalibrieren) |

Richtung wie in jeder Liste: **`Bild ab` geht nach unten = vorwärts.**

`Bild auf`/`Bild ab` statt der Pfeiltasten, weil Pfeile in jeder Liste, jedem Baum
und jedem Editor gebraucht werden — eine app-globale Pfeiltaste wäre ein
Dauerkonflikt. Statt `Space`/`Backspace` (MilkDrop-Konvention), weil `Space` nach
§2 dem Transport gehört.

**Ansicht**
| Taste | Aktion-ID |
|---|---|
| `F11` | `view.fullscreen` (seit S129 über den Filter; das Menü zeigt die Taste nur an) |
| `Esc` | `view.exitFullscreen` (bereits vorhanden, hier nur dokumentiert) |
| `Druck` | `view.screenshot` — Bild des Visuals in den Ordner des Laufs ([Screenshot_Ablage.md](Screenshot_Ablage.md)) |

Später (Stufe 2, aus dem Playlist-Konzept): `preset.autoToggle`, `preset.shuffleToggle`.

Die ganze Belegung ist **eine Tabelle an einer Stelle** — eine Änderung ist ein
Einzeiler und bricht nichts.

---

## 5. Das Fokus-Problem (technisch entscheidend)

Unmodifizierte Tasten dürfen **nicht** feuern, während getippt wird. Die App hat
Pfad- und Suchfelder, Zahlenfelder und — kritisch — die **EEL-Skript-Editoren**:
ein `PageDown` im Skript muss blättern, nicht das Preset wechseln.

`QShortcut` löst das *nicht*: es greift **vor** dem Fokus-Widget und verschluckt die
Taste, auch wenn der Handler nichts tut. Deshalb:

> **Entscheid:** Ein Anwendungs-**Ereignisfilter** (`qApp->installEventFilter`)
> statt `QShortcut`. Er prüft zuerst, ob das Fokus-Widget Texteingabe annimmt
> (`QLineEdit`, `QPlainTextEdit`/`QTextEdit` ohne ReadOnly, `QAbstractSpinBox`,
> editierbare `QComboBox`, `QKeySequenceEdit`) — dann wird das Ereignis
> **unangetastet** durchgelassen.

Regeln des Filters:

1. **Nur `KeyPress`**, keine Auto-Repeat-Unterdrückung (Halten blättert weiter —
   beim Durchsuchen eines Verzeichnisses gewollt).
2. **Modifizierte** Sequenzen (`Ctrl+…`) dürfen auch im Textfeld feuern, außer die
   Kombination ist dort selbst belegt — deshalb bleibt die Textfeld-Sperre auf
   unmodifizierte Tasten und `Shift`-Kombinationen beschränkt.
3. Der Filter kennt nur die Zuordnung Taste→Aktion-ID und veröffentlicht das
   Ereignis. Kein Wissen über Browser, Playlist oder Host.

---

## 6. Editor in den Einstellungen

Im `SettingsPanel` ein eigener Abschnitt **Hotkeys**:

- Tabelle je Kategorie: *Aktion · Taste · Vorbelegung · Zurücksetzen*.
- Taste ändern über `QKeySequenceEdit` (nimmt eine Sequenz auf).
- **Kollisionsprüfung** beim Übernehmen:
  - Taste schon belegt → die Zeile wird markiert und die Zuweisung **abgelehnt**
    (kein stilles Überschreiben; wer tauschen will, macht zuerst die andere frei).
  - Taste ist nach §2 reserviert und die Aktion gehört nicht zum Transport →
    abgelehnt, mit Begründung im Statustext.
- **Zurücksetzen** zweistufig: je Zeile und „alle auf Vorbelegung".
- Persistenz: `QSettings`, Schlüssel `shortcuts/<id>` = Sequenztext. **Nur
  Abweichungen** werden geschrieben; eine fehlende Einstellung heißt „Vorbelegung".
  Damit wirkt eine geänderte Vorbelegung in einer neuen Version automatisch für
  jeden, der die Taste nie angefasst hat.
- Leere Sequenz ist erlaubt = Aktion ohne Taste.

---

## 7. Was „erweiterte Hotkeys" später heißen kann

Bewusst gestaffelt, damit Stufe 1 klein bleibt:

| Ausbau | Inhalt | Bedingung |
|---|---|---|
| **Mehrere Tasten je Aktion** | Alternativbelegung (z. B. `PageDown` **und** Numpad) | Registry-Feld wird eine Liste; Editor bekommt „+" |
| **Kontext-Hotkeys** | Taste gilt nur, wenn ein bestimmtes Panel den Fokus hat (z. B. `Entf` in der Kette) | Filter fragt zusätzlich das aktive Panel |
| **Sequenzen/Akkorde** | `G` dann `P` | erst wenn die Aktionsmenge wirklich groß wird — vorher Kategorien nutzen |
| **Zahlen-Direktwahl** | `1`…`9` = Preset-Slot / Spur | passt zum Composer (Stufe 3) |
| **System-globale Hotkeys** | wirken, wenn die App **nicht** im Vordergrund ist | plattformspezifisch (`RegisterHotKey`), nur wenn ein echter Vorführ-Bedarf entsteht |
| **Externe Steuerung** | MIDI-Controller / Gamepad als Quelle derselben Aktions-IDs | eigener Eingabe-Adapter, der dieselben Ereignisse feuert — das Aktions-Modell (§3) trägt das schon |

Wichtig: **alle** diese Ausbauten ändern nur die *Eingabeseite*. Die Aktions-IDs und
die Ereignisse bleiben — das ist der Grund für den Zuschnitt in §3.

---

## 8. Umsetzungs-Schnitt

1. **Stufe 1 ✅ (Session 51):** `ShortcutRegistry` (SSOT) +
   `ShortcutManager` (Ereignisfilter auf `qApp`) + `PresetStepEvent`;
   Import-Browser blättert im aktiven Verzeichnis (`nextPresetRow`, permanentes
   Abo — wirkt auch bei unsichtbarem Panel); Einstellungen → *Hotkeys* mit
   Kollisions-/Reservierungsprüfung, Zurücksetzen je Zeile und für alle.
   Transport-Aktionen sind **registriert und reserviert**, aber `wired=false`:
   ihre Tasten werden nicht verschluckt und verhalten sich normal.
   Gates: `test_Shortcuts.cpp` (Registry-Eindeutigkeit, Reservierungs-Regel,
   Blätter-Logik inkl. Rändern und leerem Verzeichnis). Der Filter selbst
   (Fokus-Verhalten) bleibt Sichttest.
2. **Stufe 2 ✅ (Session 52):** Transport-Aktionen verdrahtet. Der Filter
   veröffentlicht `TransportCommandEvent{action}` (`audio/AudioEvents.hpp`);
   Empfänger ist das `PlayerPanel` mit **permanentem** Abo, damit die Tasten auch
   bei unsichtbarem Panel wirken. Seine eigenen Knöpfe rufen den Player nicht mehr
   direkt, sondern feuern denselben Befehl — eine Wahrheit je Absicht (§3).
   Lautstärke-Schrittweite 5 %.
3. Mit der Visual-Playlist: Empfänger von `PresetStepEvent` wechselt vom Browser
   zur Queue (Belegung unverändert).
4. Mit dem Composer: Spur-/Slot-Aktionen ergänzen (§7 Zahlen-Direktwahl).

---

## 9. Entscheide

1. ~~**Vorbelegung Preset-Navigation**~~ — **entschieden (Session 52):** `Bild ab`
   = vorwärts, `Bild auf` = zurück, wie in jeder Liste.
2. ~~**Blättert Stufe 1 nur über Presets oder auch in Unterordner hinein?**~~ —
   **entschieden (Patrik S60):** nur die Presets des aktiven Verzeichnisses
   (Ordner und `..` werden übersprungen, keine Rekursion). Gebaut seit S51.
3. ~~**Am Ende des Verzeichnisses:** anhalten oder umlaufen~~ — **entschieden
   (Patrik S60): umlaufen.** Gebaut S129 (bis dahin hielt der Code nach dem
   Vorschlag von S51 an): nach dem letzten Preset kommt das erste, vor dem
   ersten das letzte; die Statuszeile sagt „Back to the first preset“ bzw.
   „… last preset“. Ist das gewählte das einzige Preset, geschieht nichts
   („No other preset in this folder“) — es wird nicht neu geladen.
   `ImportBrowserPanel::nextPresetRow`, Test `test_Shortcuts`.
4. ~~**Verhalten bei Mehrfachauswahl**~~ — **entschieden (Patrik S60):**
   Blättern wirkt auf den **zuletzt gewählten** Eintrag. Erfüllt: die Liste
   des Browsers erlaubt nur eine Auswahl (`SingleSelection`), geblättert wird
   ab `currentRow()` (S129 geprüft).
5. ~~**Sollen Menü-Einträge die Tasten anzeigen?**~~ — **entschieden (Patrik
   S60): ja, aus der Shortcut-Registry, nur als Anzeige; ausgelöst wird beim
   Filter.** Gebaut S129: `MenuItemDesc::hotkeyAction` nennt die Aktion, der
   `MenuManager` setzt ihre aktuelle Taste hinter einen Tabulator in den Text
   (Qt zeigt sie dort wie ein Kürzel, ohne sie zu belegen) und zieht nach, wenn
   der `ShortcutManager` `sequenceChanged` meldet. Betrifft **View → Karaoke**
   (`view.karaoke`) und **View → Fullscreen** (`view.fullscreen`). Fullscreen
   trug bis dahin `F11` als eigene Menütaste (zwei Wahrheiten, die
   Registry-Zeile stand auf „noch ohne Wirkung“): jetzt ist `view.fullscreen`
   verdrahtet (`ToggleFullscreenEvent` ohne Quelle), umbelegbar, und wirkt auch
   im Vollbild-Fenster, das das Menü nicht erreicht. Die übrigen Menütasten
   (Ctrl+O, Ctrl+S, …) kennt nur das Menü; sie bleiben dort. Test:
   `test_PaletteUi` „Hotkeys S129“.

---

## 10. Changelog

- **1.3.0** (2026-10-16, Session 129): §9 nachgezogen — die Entscheide §9.2–§9.5
  (Patrik S60) waren nur in `Offene_Punkte.md` vermerkt. Gebaut: §9.3 umlaufen,
  §9.5 Menü zeigt die Taste aus der Registry (Karaoke, Fullscreen),
  `view.fullscreen` verdrahtet; §9.4 als erfüllt geprüft.
- **1.2.0** (2026-07-27, Session 52): Aktion `view.screenshot` auf `Druck` —
  Ablage und Vollbild-Fehlerregel in [Screenshot_Ablage.md](Screenshot_Ablage.md).
- **1.1.0** (2026-07-27, Session 52): Vorbelegung der Preset-Navigation korrigiert
  (`PgDown`/`PgUp` statt des von Qt nicht lesbaren `PageDown`/`PageUp` — die
  Vorbelegung war tot, §3-Falle und Wächter ergänzt) · Entscheid §9.1 · **Stufe 2**:
  Transport-Aktionen verdrahtet über `TransportCommandEvent`, `PlayerPanel` als
  permanenter Empfänger, seine Knöpfe feuern denselben Befehl.
- **1.0.0** (2026-07-27, Session 51): Erstfassung — Ausbaustufen (Verzeichnis /
  Visual-Playlist / Composer), Reservierungs-Regel für die Transporttasten,
  Aktions-Modell mit Ereignissen, Vorbelegungs-Vorschlag, Fokus-Problem samt
  Entscheid gegen `QShortcut`, Settings-Editor mit Kollisionsprüfung und
  Zurücksetzen, Staffelung „erweiterter" Hotkeys, Umsetzungs-Schnitt, Entscheide.
  Löst §4 des [Visual-Playlist-Konzepts](Visual_Playlist_Konzept.md) ab.

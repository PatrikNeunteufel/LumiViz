# LumiViz — Benutzerhandbuch

> **Version:** 1.55.0
> **Datum:** 2026-10-02
> **Typ:** Benutzerhandbuch
> **Status:** Aktiv
> **Zielgruppe:** Anwender
> **Sprache:** Deutsch

LumiViz ist ein Audio-Player mit Echtzeit-Visualisierung: Musik abspielen,
zuschauen, und jede Stufe der Visualisierung live verstellen. Dieses Handbuch
beschreibt die Bedienung der App als Ganzes; die Detail-Bedienung des
Konfigurations-Panels steht im
[ConfigPanel-Guide §2](ui/ConfigPanel_Guide.md#2-bedienung).

---

## Inhaltsverzeichnis

1. [Der erste Blick](#1-der-erste-blick)
2. [Musik abspielen (Player)](#2-musik-abspielen-player)
3. [Playlist verwalten](#3-playlist-verwalten)
4. [Visualizer auswählen](#4-visualizer-auswählen)
5. [Visualizer konfigurieren](#5-visualizer-konfigurieren)
6. [Vollbild](#6-vollbild)
7. [Fenster-Layout (Docking)](#7-fenster-layout-docking)
8. [Einstellungen & Frame-Modus](#8-einstellungen--frame-modus)
9. [Tastenkürzel](#9-tastenkürzel)
10. [Automatisch gemerkt / Bekanntes](#10-automatisch-gemerkt--bekanntes)
11. [Effektketten, Milkdrop & Host-Gruppen](#11-effektketten-milkdrop--host-gruppen)
12. [Shadertoy](#12-shadertoy)
13. [Video & Kamera als Quelle](#13-video--kamera-als-quelle)
14. [Stilfilter (Pixel Filter)](#14-stilfilter-pixel-filter)

---

## 1. Der erste Blick

```
┌───────────────────────────────────────────────┬──────────────┐
│ Menü: File · Edit · View · Settings · Help    │              │
├───────────────────────────────────────────────┤  Seitentabs  │
│                                               │  (Auto-Hide) │
│         Visualizer (zentrale Ansicht)         │  Visualizer  │
│                                               │  Config      │
│                                               │  Playlist    │
├───────────────────────────────────────────────┤  Visualizers │
│ Player (Dock unten)                           │              │
├───────────────────────────────────────────────┴──────────────┤
│ Statusleiste: Meldungen · FPS-Anzeige (rechts)                │
└───────────────────────────────────────────────────────────────┘
```

Die Panels sind **andockbare Fenster** (siehe §7): am Rand als schmale
Seitentabs geparkt, per Klick ausklappbar, frei verschieb- und abdockbar.
Die FPS-Anzeige rechts unten zeigt die Bildrate des Haupt-Visualizers.

## 2. Musik abspielen (Player)

Das **Player-Panel** steuert die Wiedergabe:

| Element | Funktion |
|---|---|
| ⏮ / ⏭ | Voriger / nächster Track der Playlist |
| ▶ / ⏸ | Wiedergabe starten / pausieren |
| ⏹ | Stopp (zurück an den Trackanfang) |
| Loop-Taste | **Aktuellen Track** wiederholen (leuchtet bei aktiv) |
| Resume-Taste (⏩) | **Resume playback**: nach dem Neustart dort weiterspielen (leuchtet bei aktiv) |
| Fortschrittsbalken | Anfassen und ziehen = Spulen (Seek) |
| 🔇 + Lautstärkeregler | Stummschalten / Lautstärke |

Musik kommt über die Playlist (§3) in die App — Titel dort doppelklicken
startet die Wiedergabe.

**Resume playback** (auch unter **Settings → Audio**): Beendest du LumiViz,
**während ein Titel läuft**, spielt der nächste Start denselben Titel an
derselben Stelle weiter. Warst du beim Beenden auf Pause oder Stopp, bleibt es
still. Steht der Titel noch in der Playlist, läuft er von dort (und danach
normal weiter zum nächsten); fehlt die Datei inzwischen, passiert nichts.
Festgehalten wird der Stand beim geordneten Beenden — nach einem Absturz gilt
der vom letzten Beenden davor.

**Ziehen zum Abspielen** (seit Session 84): Zieh eine Audiodatei (aus dem
Audio Browser oder dem Explorer) oder einen **Playlist-Eintrag** auf das
Player-Panel **oder auf das Bild der Visualisierung**. Ein Playlist-Eintrag
wird direkt angesprungen; eine Datei, die schon in der Playlist steht, ebenso
— sonst wird sie angehängt und gespielt. Der Eintrag bleibt dabei in der
Playlist (es wird nie verschoben).

## 3. Playlist verwalten

Das **Playlist-Panel** führt die Titelliste:

- **Add files** — Audiodateien hinzufügen (Mehrfachauswahl möglich).
- **Doppelklick** auf einen Titel — abspielen.
- **Remove selected / Clear** — Auswahl entfernen bzw. Liste leeren.
- **Save / Load** — Playlist als Datei sichern/laden
  (**M3U/M3U8**, PLS, JSON).
- **Shuffle-Modus** — nächster Titel wird zufällig gewählt (mischt die Liste
  nicht um).
- **Loop-Playlist** — nach dem letzten Titel geht es vorn weiter.
- **Ziehen** (seit Session 84): Titel in der Liste verschieben (auch
  mehrere), Audiodateien aus dem **Audio Browser** oder dem Explorer an eine
  bestimmte Stelle fallen lassen, Titel hinaus auf Player oder Visualisierung
  ziehen (§2). *Behoben:* Verschieben änderte früher nur die Anzeige — danach
  spielte ein Doppelklick den falschen Titel.

### Tag View

**View → Panels → Tag View** zeigt, was LumiViz aus dem laufenden Titel liest:
**Title**, **Artist**, **Album**, **Year**, **Genre**, **Track**, dazu **File**
und **Format** (Dauer) — und darüber das **Cover** in größerer Vorschau
(eingebettet, sonst das Ordnerbild). Ein fehlender Tag steht als „—" da. Die
Werte lassen sich markieren und kopieren. Es sind dieselben, die der
Text-Knoten in seine Vorlagen einsetzt (`{title}`, `{artist}` …) — das Panel
zeigt also auch, warum eine Vorlage leer bleibt. Ändern lassen sich die Tags
hier nicht.

Darunter steht der **Songtext** des laufenden Titels, so wie ihn der
Lyrics-Knoten führt: die Zeile **Lyrics** nennt Quelle, *synced* oder *not
synced* und die Zeilenzahl, das Feld darunter den Text ohne Zeitmarken (nur
lesen, markieren und kopieren). Ein online geholter oder über
**Alternatives…** gewählter Text erscheint von selbst, sobald er da ist. Ohne
Text steht dort *No lyrics for this track*.

### Audio Browser

**View → Panels → Audio Browser** zeigt einen Ordner mit Musik, wie der
Effect Browser die Presets: Unterordner zum Hineingehen, darunter alle Dateien,
die die Audio-Engine abspielen kann (samt den Formaten der geladenen
BASS-Plugins). Als Icon steht das **Albumcover**: zuerst das in der Datei
eingebettete (MP3/ID3, FLAC, M4A — bei mehreren Bildern das Frontcover),
sonst ein Cover-Bild im Ordner (`cover`, `folder`, `front`, `albumart…` als
`.jpg`/`.png`), sonst ein Icon der Formatfamilie (MP3, WAV/AIFF, FLAC u. ä.,
OGG/Opus, M4A/AAC, sonstige). Die Cover erscheinen nach und nach — ein großer
Ordner bleibt dabei bedienbar. Ogg/Opus-Cover werden noch nicht gelesen.

- **Doppelklick** — an die Playlist hängen und abspielen (steht der Titel
  schon drin: dorthin springen).
- **Ziehen** — in die Playlist (an die Stelle), auf Player oder
  Visualisierung (abspielen). Mehrfachauswahl geht.
- Der zuletzt geöffnete Ordner wird gemerkt; beim ersten Mal öffnet der
  Musik-Ordner von Windows.

**Session-Playlist:** Beim Beenden wird die aktuelle Playlist automatisch
gespeichert und beim nächsten Start wiederhergestellt (inklusive Position in
der Liste, ohne Autostart). Es gibt nichts zu tun — einfach schließen.

## 4. Visualizer auswählen

Im **Visualizers-Panel** stehen die verfügbaren Visualisierungen
(Pulsing, Equalizer, Waveform, Oscilloscope, Superscope), gruppiert nach
Kategorie und mit Kurzbeschreibung. Auswahl markieren → **Apply** — die
zentrale Ansicht wechselt sofort.

**Mehrere Visualizer gleichzeitig:** *View → New Visualizer* (Ctrl+N) öffnet
ein weiteres Visualizer-Fenster als Tab bzw. Dock — jedes rendert unabhängig
mit eigener Bildrate; frei anordnen wie in §7 beschrieben.

## 5. Visualizer konfigurieren

Das **Visualizer-Config-Panel** zeigt alle Parameter des aktiven Visualizers,
gegliedert nach den Pipeline-Stufen (1 Audio/Analyse → 6 Post). Die
vollständige Bedienungsanleitung — Parametergruppen, Presets speichern/laden,
Farbverlaufs-Editor, Stufen-Vorschauen (Auge-Symbol im Gruppenkopf) — steht im
**[ConfigPanel-Guide §2](ui/ConfigPanel_Guide.md#2-bedienung)**.

Das Wichtigste in Kürze:

- Jede Änderung wirkt **sofort** im Bild.
- **Undo/Redo:** *Edit → Undo/Redo* (Ctrl+Z / Ctrl+Y) — Slider-Züge werden
  zu einem Schritt zusammengefasst.
- **Presets:** oben im Panel je Visualizer speichern/laden; „Default" setzt
  auf die Werkseinstellung zurück.
- **Live-Vorschauen** je Stufe über das Auge-Symbol einblenden (standardmäßig
  aus, kosten nichts solange ausgeblendet).

## 6. Vollbild

- **Rein:** Doppelklick auf die Visualizer-Fläche — oder *View → Fullscreen*
  (F11). Es erscheint NUR das Bild, randlos ohne Menü/Tabs/Panels.
- **Raus:** **Esc** oder erneuter Doppelklick.
- Bei mehreren Visualizern geht genau der **doppelt geklickte** in den
  Vollbildmodus (F11/Menü nehmen den Haupt-Visualizer).

## 7. Fenster-Layout (Docking)

Alle Panels und Visualizer-Fenster sind frei andockbar:

- **Verschieben:** Titelleiste ziehen — beim Ziehen erscheinen
  Andock-Zonen; loslassen dockt an, außerhalb entsteht ein
  **schwebendes Fenster**.
- **Tabs:** Zwei Docks übereinander ziehen stapelt sie als Tabs.
- **Auto-Hide:** Pin-Symbol in der Dock-Titelleiste parkt das Panel als
  Seitentab am Rand; Klick auf den Tab klappt es temporär aus.
- **Panels ein-/ausblenden:** *View → Panels*.
- **Perspektiven:** *View → Perspectives* — benannte Layouts speichern und
  umschalten; *Reset Layout* stellt das Standard-Layout wieder her,
  *Save Layout as Default* macht das aktuelle Layout zum Standard.
- Das Layout wird beim Beenden **automatisch gespeichert** und beim
  nächsten Start wiederhergestellt.

## 8. Einstellungen & Frame-Modus

**Settings-Panel** (Seitentab) mit sechs Reitern:

- **Audio:** Ausgabegerät wählen; Puffergröße (kleiner = weniger Latenz,
  größer = stabiler) und Samplerate.
- **Performance:** Frame-Modus und Ziel-FPS · **Graphics Card** — welche
  Grafikkarte LumiViz rendert (Automatic (Windows decides) / High Performance / Power Saving).
  Der Wert landet in der Windows-Grafikeinstellung und greift erst beim
  Prozessstart, deshalb **startet die App bei einer Änderung sofort neu**.
  Darunter zeigt *Active GPU* die tatsächlich benutzte Karte.
- **Panels:** **Effect Browser: Reset Start Folder** (Startordner zurücksetzen) — er springt danach in
  den **Programmordner**, wo der mitgelieferte Ordner `presets/` liegt · Render-Scale-Divisor
  für AVS-Importe · **Image Search Folder** (Bilder-Suchordner) — wird beim Import durchsucht, wenn
  neben dem Preset kein Bild für Picture/Texer liegt, und ist der Startordner der
  Bildauswahl in der Effektkette · **User Data: Open Folder** (Benutzerdaten-Ordner) — dort liegen
  deine eigenen Knoten-Voreinstellungen (`nodepresets/<typ>/`); der Ordner wird
  beim ersten Öffnen angelegt · **MilkDrop Preset Switch** + **MilkDrop Fade
  Amount** — der App-Standard dafür, was beim MilkDrop-Preset-Wechsel mit dem
  geerbten Bild passiert (Details und die Regeln je Node: §11,
  „Preset-Wechsel: das geerbte Bild").
- **Editor:** wie **Beautify** im großen Code-Editor formatiert —
  *Beautify Indent Width* (Einzug), *Spaces around Operators* (nur EEL) und
  *Max. Blank Lines* (so viele Leerzeilen hintereinander bleiben stehen).
- **Camera:** Kamera-Clips aufnehmen und verwalten (Gerät, Dauer,
  *● Record Camera Clip*, Liste der *Camera clips*) — Details in §13.
- **Hotkeys:** Tastenbelegung je Aktion (siehe §9).

Der **Frame-Modus** (auch über *Settings → Frame Mode* im Menü) bestimmt die
Bildrate der Visualizer:

| Modus | Verhalten |
|---|---|
| **Limited (60 FPS)** | feste Zielrate, Standard — sparsam und gleichmäßig |
| **Unlimited** | so schnell wie möglich (Benchmark/Test) |
| **VSync** | synchron zur Bildwiederholrate des Monitors (z. B. 240 Hz) |

Die Oberfläche bleibt in allen Modi flüssig — das Rendering läuft von der
Bedienung entkoppelt.

## 9. Tastenkürzel

| Kürzel | Aktion |
|---|---|
| Ctrl+N | Neues Visualizer-Fenster |
| F11 | *Fullscreen* ein/aus (Haupt-Visualizer) |
| Esc | Vollbild verlassen |
| Ctrl+Z / Ctrl+Y | Parameter-Änderung rückgängig / wiederherstellen |
| Ctrl+O | *Open Audio…* (derzeit ohne Funktion, siehe §10) |
| Ctrl+Shift+N | *File → New Effect Chain* — leere Kette als Ausgangspunkt für ein neues Preset |
| Ctrl+S | *File → Save Effect Chain* — speichert in die laufende eigene Datei, ohne nachzufragen (sonst wie Save As, s. u.) |
| Ctrl+Shift+S | *File → Save Effect Chain As…* — fragt immer nach Ort und Namen |
| F1 | *About LumiViz* |
| Alt+F4 | Beenden |
| Ctrl+K | *Karaoke On / Off* — Songtext über jedem Preset ein- und ausschalten (umbelegbar; auch **View → Karaoke** und der Haken im Karaoke-Panel) |

### Speichern und ungespeicherte Änderungen

**Save** schreibt ohne Dialog, wenn die Kette aus einer eigenen `.lvfx` oder
`.lvfx2` geladen oder dorthin gespeichert wurde. In allen anderen Fällen
öffnet auch Save den Dialog von **Save As…**: bei einer neuen Kette, bei
einem importierten `.avs` oder `.milk`, bei einem mitgelieferten Preset aus
dem Programmordner, und wenn sich die Endung ändern muss (eine Kette mit
Host-Gruppe ist `.lvfx2`, eine ohne `.lvfx`). Nach dem Speichern gilt die
gespeicherte Datei als laufendes Preset.

Hat die Kette **ungespeicherte Änderungen**, fragt LumiViz nach, bevor sie
ersetzt wird — bei *New Effect Chain*, beim Laden und Importieren über das
Menü, beim Doppelklick im Effect Browser, beim Blättern mit Bild ab/Bild auf,
beim Wechsel auf einen anderen Visualizer (Panel *Visualizers*) und beim
Beenden: **Save** (speichern, dann weiter), **Discard** (weiter ohne
Speichern), **Cancel** (zurück). Als Änderung zählt nur, was auch gespeichert
würde — ein Feld hin und zurück gestellt ist keine, ein frisch geladenes oder
importiertes Preset auch nicht. Im **Vollbild** verlässt LumiViz dafür erst
das Vollbild und fragt dann. Knoten-Voreinstellungen fragen nie nach: sie zu
laden und zu verändern ist eine Änderung an der Kette wie jede andere.

### Die Preset-Zeile

Die Panels **Effect Chain** und **Karaoke** tragen oben dieselbe Zeile:

- **Dropdown:** ganz oben das laufende Preset — mit einem `*` dahinter,
  solange es ungespeicherte Änderungen hat —, darunter deine **Favoriten**
  und die **zehn zuletzt benutzten**. Es zeigt nicht alle Dateien eines
  Ordners; dafür ist der Effect Browser da. Eine Auswahl **lädt sofort**
  (mit der Rückfrage oben, falls etwas ungespeichert ist). Das Mausrad über
  dem Dropdown wechselt nichts. Ein Eintrag, dessen Datei es nicht mehr gibt,
  steht grau da und fällt beim Anklicken aus der Liste.
- **★** macht das laufende Preset zum Favoriten bzw. nimmt es wieder heraus.
- **Load…**, **Save**, **Save As…** tun dasselbe wie die Einträge im Menü
  File. Im Effect-Chain-Panel landet in „zuletzt" alles, was als Visual
  geladen oder gespeichert wurde (`.avs`, `.milk`, `.lvfx`, `.lvfx2`) — egal
  ob über Menü, Effect Browser oder die Zeile selbst. Das Karaoke-Panel führt
  eigene Listen für `.lvkar`.

### Presets durchblättern

| Kürzel | Aktion |
|---|---|
| Bild ab | *Next Preset* im **aktiven Ordner des Effect Browsers** |
| Bild auf | *Previous Preset* |

`Bild ab` geht in der Liste nach unten, also **vorwärts**.

Geblättert wird über die Presets des Ordners, der im Effect Browser offen ist —
Unterordner werden übersprungen, am Ende des Ordners hält es an. Das Panel muss
dafür **nicht** sichtbar oder fokussiert sein. Während in einem Textfeld oder
Skript-Editor getippt wird, blättern die Tasten nicht, sondern verhalten sich
normal.

### Musikwiedergabe

In der Hotkey-Liste stehen diese Aktionen unter der Kategorie *Playback*, Vollbild
und Screenshot unter *View*.

| Kürzel | Aktion |
|---|---|
| Leertaste | *Play / Pause* |
| Ctrl+→ / Ctrl+← | *Next Song* / *Previous Song* |
| Ctrl+↑ / Ctrl+↓ | *Volume Up* / *Volume Down* (5 % je Anschlag) |

Diese Tasten wirken wie die Knöpfe des Player-Panels und funktionieren auch, wenn
das Panel nicht sichtbar ist.

### Screenshot of the Visual

`Druck` legt ein Bild **des Visuals** (ohne Panels) ab — in einem Ordner je
Programmlauf, benannt nach dem Startzeitpunkt:

```
…\asset\calibration\screenshot\2026-07-27_11-47-57\
    Alien Alloy_avs.png     das Bild
    Alien Alloy_avs.txt     der vollständige Pfad des Presets
```

Dieselbe Aufnahme mehrfach ergibt `_2`, `_3` — es wird nie überschrieben. Der
Zielordner lässt sich über den Einstellungsschlüssel `screenshot/baseDir`
verlegen; ohne Eintrag sucht die App den Projekt-Ordner `asset/calibration`.

> Öffnet `Druck` bei Dir das Windows-Snipping-Tool, ist das eine
> Windows-Einstellung. Dann in *Settings → Hotkeys* eine andere Taste
> vergeben.

**Im Vollbild** erscheint bei einem Fehler **kein** Meldungsfenster mehr:
stattdessen wird automatisch ein Screenshot aufgenommen und die Meldung an
`fehler.log` im selben Ordner angehängt. Im Fenster kommt weiterhin der Dialog.

### Eigene Belegung

*Settings → Hotkeys*: jede Aktion hat ein Aufnahmefeld, ein
*Default*-Knopf je Zeile und *All to defaults* für die ganze Liste. Eine
Taste, die schon vergeben ist, wird **abgelehnt** statt still umgehängt — erst
die andere Zuweisung freimachen. Die Transporttasten (Leertaste, Ctrl+Pfeile)
sind für die Musikwiedergabe reserviert und lassen sich nicht an Preset-Aktionen
vergeben.

## 10. Automatisch gemerkt / Bekanntes

**Automatisch gespeichert** (keine Aktion nötig):

- Fenster-Layout (Panels, Docks, Perspektiven)
- Session-Playlist inkl. aktueller Position
- Eingeblendete Stufen-Vorschauen je Visualizer

**Bekannte Einschränkungen:**

- *File → Open Audio…* (Ctrl+O) ist noch ohne Funktion — Dateien über die
  Playlist (**Add files**) hinzufügen.
- Im Preset-Dropdown des Config-Panels kann „Default" bei einzelnen
  Visualizern von der Start-Optik abweichen (bekannt beim Pulsing-Farbverlauf).

---

## 11. Effektketten, Milkdrop & Host-Gruppen

Der Visualizer **„Multi Effect"** rendert eine frei editierbare Effektkette.
Bearbeitet wird sie im Panel **View → Panels → Effect Chain**; Presets lädt
der **Effect Browser** (Panel rechts, bis Session 82 „Import Browser") per
Doppelklick — `.avs`, `.milk`, `.lvfx` und `.lvfx2`. Geladene `.avs`/`.milk`
werden beim Speichern zur `.lvfx`. Ein Preset aus dem Browser in den Baum
**gezogen** wird dort ein Presetverweis (s. Host-Gruppen).

### Ketten-Editor

- **Baum** = die Kette; das **Auge** (Spalte 2) blendet einen Effekt aus/ein.
- **Toolbar:** Typ im Dropdown wählen, **+** fügt in die selektierte
  Liste/Gruppe ein (sonst ans Ende), **−** entfernt, **⧉** klont,
  **↑/↓** sortiert; Drag & Drop verschiebt (auf eine Gruppe fallen lassen =
  hinein).
- Unter dem Baum: der **Eigenschafts-Editor** des selektierten Eintrags.
- **File → New Effect Chain** (`Ctrl+Shift+N`) beginnt mit einer leeren Kette —
  der Ausgangspunkt für ein eigenes Preset. Es wird vorher gefragt.

### Voreinstellungen je Knoten

Ganz oben im Eigenschafts-Editor steht die Zeile **Preset**. Sie gilt für
**jeden** Knotentyp — auch für Effect List, Movement oder eine Host-Gruppe.

- **Auswählen** lädt einen benannten Parametersatz. Mitgeliefertes steht ohne
  Zusatz in der Liste, Selbstgespeichertes mit `*`. Der Name **bleibt stehen**,
  solange du nichts änderst; ohne geladene Voreinstellung steht dort
  **(none)**.
- Sobald du danach etwas änderst (Regler, Formel, Farbe), zeigt die Zeile
  **(unsaved) – Name**: der Stand stammt von dieser Voreinstellung, ist aber
  nicht mehr derselbe. Name und Beschreibung des Knotens zählen nicht — sie
  gehören nicht zur Voreinstellung.
- **Save as…** fragt nach einem Namen und zeigt **jedes Feld des Knotens mit
  einem Häkchen**. Was du abwählst, kommt nicht in die Datei — und bleibt beim
  Laden unangetastet. So speicherst du „nur die Formeln, nicht die Farbe".
- **🗑** löscht eine eigene Voreinstellung — die gewählte oder, bei
  „(unsaved)", die, von der der Stand stammt. Mitgelieferte lassen sich nicht
  löschen.
- **Milkdrop-Elemente** haben seit Session 83 eine eigene Zeile: Markiere im
  Baum eine einzelne **Custom Wave**, **Custom Shape** oder ein **Sprite** —
  oben im Editor steht dann *Preset* für genau dieses Element. Mitgeliefert
  sind z. B. *Oscilloscope Line*, *Spectrum Circle* (Waves) und *Pulse With
  Bass*, *Slow Fade* (Shapes). Nummer und An/Aus des Elements bleiben beim
  Laden, wie sie sind.
- Auch die Milkdrop-Sektionen **Code** und **Parameters** tragen eine eigene
  Zeile. *Code*: kleine per_frame-/per_pixel-Bausteine (*Slow Rotation*,
  *Beat Zoom*, *Ripple (per pixel)* …) — ein Baustein setzt nur die Felder,
  die er braucht. *Parameters*: Stellungen der Basiswerte, der Gruppe *Styling* und
  der „bisher"-Schalter, z. B. **Original** (*Styling* aus, alle drei
  „bisher"-Schalter auf Original), **Old LumiViz Look (pre-S80)** (weicher Sog
  nach oben links samt *Soften* und die drei „bisher"-Stellungen — der
  Look bis Session 80 in einem Zug), *Soft Pull Up-Left*, *Radial Swirl*,
  *Mirror Echo*, *Modern Rules (IEEE)*.
- **Namen** der mitgelieferten Presets und Vorlagen sind englisch — wie die
  übrige Oberfläche.
- Die früheren **Template**-Listen bei **SuperScope 3D** und **3D Camera**
  sind jetzt ebenfalls Voreinstellungen dieser Zeile (gleiche Namen, z. B.
  *Spiral Galaxy*, *Orbit*) — und damit um eigene erweiterbar.
- Die **SuperScope-Figuren** (Spiral, Butterfly, Hypocycloid …) sind seit
  Session 53 solche Voreinstellungen — das frühere Dropdown „Figure" ist
  entfallen. Da sie nur die Formeln enthalten, behältst du beim Laden deine
  Farbtafel und Linienbreite.
- Seit Session 60/61 liefert die App einen **Grundkatalog von 125
  Voreinstellungen für 39 Knotentypen** mit: Bewegungs-Klassiker (u. a. die
  acht originalen AVS-Movement-Formeln wie *Big Swirl Out* und *6-way
  Kaleida*), Faltungskerne (*Blur*, *Sharpen*, *Edges*, *Emboss*),
  Farbverläufe (*Fire*, *Ocean*, *Neon*), Wasser-, Gitter-, Zoom-, Spiegel- und
  Beat-Vorlagen, die **Fraktal-Familie** (Mandelbrot-Ausschnitte, Mandelbulb,
  Plasma, Attraktoren, Reaktions-Diffusion), **Bloom** (inkl. der
  Lights-Referenzwerte) sowie sechs aus der Preset-Sammlung **geerntete
  Dynamic-Movement-Warps** (Quelle je Vorlage im Katalog genannt). Alles
  Teil-Presets: sie setzen nur die Felder, die den Charakter ausmachen. Der
  vollständige Katalog mit Erklärung je Vorlage steht in
  `asset/nodepresets/README.md`.

Eigene Voreinstellungen liegen im Benutzerdaten-Ordner (*Settings →
Panels → User Data: Open Folder*), mitgelieferte unter
`asset/nodepresets/`. Wer eine eigene weitergeben will, kopiert die Datei
dorthin.

### Werte per Formel steuern

Fast jeder Knoten hat unten drei EEL-Felder: **Init** (einmal), **Frame** (jedes
Bild) und **Beat** (bei jedem Beat). Darin lassen sich die Regler desselben
Knotens berechnen — etwa bei Rotating Stars:

```
Frame:  points = 5 + bass*8
```

Die schreibbaren Namen sind die Feldbezeichnungen in Kleinschreibung; dazu gibt
es `b` (Beat), `w`/`h` (Bildmaße) und den Audio-Satz (`bass`, `mid`, `treb`,
`vol` …). Eigene Variablen darfst du frei benennen — sie gelten innerhalb
**eines** Knotens über alle Slots hinweg. Zwischen Knoten wandern nur
`reg00..reg99`, `q1..q64` und `gmegabuf`.

Ein leeres Feld kostet nichts: ohne Formel läuft kein Skript.

Seit Session 85 lässt sich **jedes Zahl-, Schalter- und Auswahlfeld** eines
Knotens so steuern — auch bei Fraktalen, 3D-Knoten, Glow Orbs, Texer II &
Co. (dort etwa `type`, `smooth`, `blend`). Die ⓘ-Seite listet alle Namen.
Bewegst du einen Regler, während ein Skript läuft, **gilt ab da der Regler**
und das Skript rechnet von diesem Wert weiter — sein Zustand (Zähler, Phasen)
bleibt erhalten; früher begann das Skript bei jeder Reglerbewegung von vorn.

Seit Session 86 gilt das auch für **SuperScope** und **Effect List**:

- **SuperScope** — in Init, Frame und Beat sind `linewidth`, `dotsize`,
  `rendermode`, `lineblend`, `audiochannel`, `spectrumsource`,
  `colorcycleframes` und `pointcount` schreibbar. Die AVS-Namen behalten
  Vorrang: `n` bestimmt die Punktzahl (ein Schreiben auf `pointcount` wird an
  `n` weitergereicht), `linesize` und `drawmode` schlagen `linewidth` und
  `rendermode`. `colorcycleframes` wirkt ab dem nächsten Bild.
- **Effect List** — im Listen-Code (mit **Use list code**) sind `blendin`,
  `blendout`, `bufferin`, `bufferout`, `bufferininvert`, `bufferoutinvert`,
  `cleareveryframe`, `onbeatrender`, `onbeatframes` sowie `inadjustalpha` und
  `outadjustalpha` (0..255) schreibbar. `alphain`/`alphaout` (0..1) bleiben wie
  in AVS und gewinnen, wenn das Skript beide Schreibweisen benutzt.
- **Dynamic Movement** — in Init, Frame und Beat sind `wrap`, `nomove`,
  `subpixel`, `rectcoords` und `buffern` schreibbar. **Nicht** `blend` (viele
  AVS-Presets benutzen den Namen als eigene Variable) und nicht die
  Gittergröße `xres`/`yres`.
- **Host-Gruppe** — im Parameter-Skript der Gruppe zusätzlich zu
  `outadjustalpha` jetzt `blendout`, `curvein`, `curveout` und
  `crossfadeseconds` (wirkt ab dem nächsten Bild).
- **Host Switcher** — `intervalseconds`, `order` und `nextonsongchange`;
  **Switch after (s)** lässt sich auch mit dem **~**-Knopf modulieren.

- **Text** und **AVI** — haben jetzt eigene Felder **Init**, **Frame** und
  **Beat** (und Song). Text: `normspeed`, `onbeatspeed`, `onbeat`, `blend`,
  `randomword`, `insertblank` wirken sofort; Schrift, Farbe (`color_r/_g/_b`),
  Lage (`xshift`, `yshift`, `halign`, `valign`) und Kontur lassen den Text neu
  zeichnen und folgen dem Skript deshalb nur aus Init, Song und Beat. AVI:
  `blend`, `adapt`, `persist`, `speedms`.

Ohne Formelfelder und deshalb nicht per Skript steuerbar bleiben Movement
(nur die Bewegungsformel), Render Scale und Debug Bars.

**Text: Schrift wählen.** Das Feld **Font** ist eine Auswahlliste aller
installierten Schriften; ein Name lässt sich weiterhin eintippen (importierte
Presets nennen Schriften, die nicht installiert sein müssen — dann nimmt das
System die nächstliegende). Leer = Standardschrift. Das Häkchen **Bold** setzt
**Weight** auf 700 (aus: 400); gezeichnet wird fett ab Weight 600.

**Text: Titel und Interpret anzeigen.** Im Textfeld stehen Platzhalter für die
Tags des laufenden Titels — der Knopf **Insert tag** fügt sie an der
Schreibmarke ein:

| Platzhalter | Inhalt |
|---|---|
| `{title}` | Titel (ohne Tag: der Dateiname) |
| `{artist}` `{album}` `{year}` `{genre}` `{track}` | Interpret, Album, Jahr, Genre, Tracknummer |
| `{file}` | Dateiname ohne Pfad und Endung |

Eckige Klammern machen einen Teil **wahlfrei**: er verschwindet, wenn ein Tag
darin leer ist. `[{artist} – ]{title}[ ({year})]` ergibt „New Order – Blue
Monday (1983)", ohne Jahr „New Order – Blue Monday", ohne Interpret „Blue
Monday". Das `;` trennt weiterhin die Wörter: `{title};{artist};{album}` zeigt
die drei nacheinander. Gelesen werden ID3v2/ID3v1 (MP3), Vorbis-Kommentare
(Ogg, FLAC), APE und MP4. Ist kein Titel geladen, zeichnet eine reine
Tag-Vorlage nichts. Die Voreinstellung **Now Playing** bringt eine fertige
Zeile unten im Bild mit.

**Lyrics: Songtext zum laufenden Titel.** Der Knoten **Lyrics** (Effekt-Palette
→ Images) zeigt den Text des laufenden Titels. Den Text sucht LumiViz beim
Titelwechsel, in dieser Reihenfolge — synchron vor unsynchron, nah am Song vor
fern:

1. eingebetteter **synchroner** Text (ID3 `SYLT`, oder ein eingebetteter Text
   mit Zeitmarken)
2. `<Song>.lrc` neben dem Song
3. `<Interpret> - <Titel>.lrc` im Benutzerordner
   (`%APPDATA%\LumiViz Project\LumiViz\lyrics`)
4. eingebetteter unsynchroner Text (ID3 `USLT`, Vorbis `LYRICS`)
5. `<Song>.txt` neben dem Song
6. `<Interpret> - <Titel>.txt` im Benutzerordner

Synchron ist ein Text, sobald er Zeitmarken trägt (`[01:23.45]`) — die Endung
entscheidet nicht. Oben im Editor steht laufend als kleine Tabelle, was
gefunden wurde:

| Zeile | Inhalt |
|---|---|
| **Status** | *Lyrics loaded*, *No lyrics found …* oder, solange ein Online-Abruf läuft, *…downloading…* |
| **Source** | woher der Text stammt (eingebettet, Datei neben dem Song, Benutzerordner, LRCLIB) |
| **Timing** | *synced* oder *not synced* |
| **Lines** | Zeilenzahl |
| **Origin** | Herkunftsvermerk eines aus dem Netz geholten Texts |
| **File** | Name der gelesenen Datei (voller Pfad im Tooltip); **Open folder** öffnet ihren Ordner — bei eingebettetem Text gibt es keinen |

| Feld | Bedeutung |
|---|---|
| **Preview lines** | So viele kommende Zeilen stehen unter der laufenden (0–4), in **Color upcoming**. |
| **Preview size (%)** | Schriftgröße der Vorschau-Zeilen in Prozent der laufenden Zeile (10–400); ihre Zeilenhöhe geht mit. |
| **Preview weight (0 = as line)** | Gewicht der Vorschau-Zeilen: 0 = wie die laufende Zeile, sonst 400 normal oder 700 fett. |
| **Preview X shift / Y shift (%)** | Versatz der Vorschau-Zeilen gegen ihren gewohnten Platz, in Prozent von Breite und Höhe (negativ = nach links bzw. oben). Die laufende Zeile bleibt stehen. |
| **Line spacing (%)** | Zeilenabstand in Prozent der Schrifthöhe (50–400); 115 ist der Abstand bis Version 1.54. |
| **Color (voice 1)**, **Color voice 2**, **Color both voices** | Farbe der laufenden Zeile; bei Duetten je Stimme. Stimm-Marken am Zeilenanfang: `v1:`, `v2:`, `v1+v2:` (auch `M:`, `F:`, `D:`); eine Marke gilt bis zur nächsten. |
| **Blend** | Wie sich der Text mit dem Bild mischt: **Replace**, **Additive**, **50/50**, **Subtract**, **Invert** (weißer Text kehrt die Farbe darunter um), **XOR**, **Multiply**, **Maximum**, **Minimum** — dieselben Arten wie bei der Karaoke-Ebene (unten). Ab **Subtract** mischt nur die Füllung der Buchstaben, die Kontur bleibt normal; bei Replace, Additive und 50/50 mischt die Kontur wie bisher mit. Der **Lyrics Trail** hat dasselbe Feld. |
| **Timing offset (ms)** | Verschiebt synchronen Text für **alle** Titel: positiv = früher, negativ = später. Eine `[offset:…]`-Zeile in der Datei wirkt zusätzlich. |
| **Offset for this track** (eigene Zeile unter der Tabelle) | Derselbe Versatz, aber **nur für den laufenden Titel**. LumiViz merkt ihn sich je Titel (Interpret, Titel, Album, Dauer) in `offsets.json` im Benutzerordner der Songtexte; die Songtext-Datei wird dafür nicht angefasst, und er gilt auch für eingebetteten Text. Er zählt zu **Timing offset (ms)** und zu `[offset:…]` dazu. Dasselbe Feld steht als **Track offset** im Panel Lyrics Sync. |
| **Unsynced: beats per line** | Unsynchroner Text steht als Block; mit einem Wert > 0 rückt er alle N Beats eine Zeile weiter. |
| **Mark unsynced lyrics** | Schreibt „(not synced)" über einen unsynchronen Text — er läuft nicht zum Gesang. |

Synchroner Text läuft an der **Abspielposition** des Players, auch nach dem
Springen im Titel. Trägt die Datei Wort-Zeiten (`<01:23.80>` im Text), füllt
sich die laufende Zeile **Wort für Wort**. Zu lange Zeilen werden kleiner
gesetzt, bis sie passen. Schrift, Lage, Kontur und Formelfelder wie beim
Text-Knoten — dazu das Feld **Line**: es läuft **einmal bei jedem
Zeilenwechsel** (nach Song, vor Frame) und ist die Stelle für alles, was mit
der Zeile wechseln soll. Lage, Größe und Farben folgen dem Skript von hier
(aus Frame folgen sie nicht, weil sie den Text neu zeichnen lassen). Es liest
`line` (Nummer der laufenden Zeile, 0 = vor der ersten) und `linecount`; beide
stehen auch in den anderen Feldern bereit. Beispiele: `yshift = -6 - rand(8);`
(jede Zeile etwas anders hoch), `halign = line % 3;` (links, Mitte, rechts im
Wechsel). Auch Zurückspulen und ein neuer Titel gelten als Zeilenwechsel. Die
Karaoke-Ebene hat dasselbe Feld im Reiter *Lyrics*. Voreinstellungen: feste Lagen (**Karaoke Bottom**, **Karaoke
Top**, **Karaoke Classic**, **Karaoke Neon**, **Big Centre**, **Three Lines
Left**, **Side Right**, **Subtitle Small**, **Verse Sheet**) und bewegte
Karaoke-Varianten, die per Skript arbeiten: **Karaoke Beat Colours** (neue
Farbe je Beat), **Karaoke Song Colours** (eine Farbe je Song), **Karaoke
Bounce** (Sprung je Beat, Weite im Init) und **Karaoke Flip** (wechselt alle
`flip_beats` Beats zwischen unten und oben). Eine Lyrics-Voreinstellung
ersetzt auch die Skriptfelder des Knotens.

**Songtexte aus dem Netz** (aus, bis du es einschaltest): **Settings → Audio →
Search lyrics online**. Findet LumiViz lokal keinen Text, fragt es dann den
Dienst lrclib.net — dabei gehen **Interpret, Titel, Album und Dauer** des
laufenden Titels an diesen Dienst. Gefragt wird nur, wenn Titel und Interpret
bekannt sind; ein Treffer verlangt, dass die Dauer auf zwei Sekunden passt.
Der Dienst ist oft beschäftigt und bittet dann um einen neuen Versuch: LumiViz
fragt für den laufenden Titel von selbst wieder (nach 2, 5, 10, 20 und 40
Sekunden) — so lange steht in der Tabelle *…downloading…*, der Text erscheint,
sobald er da ist. Ein
Fund wird gespeichert, damit beim nächsten Mal nichts mehr hinausgeht:
**Save fetched lyrics** wählt den Ort — **User folder**
(`%APPDATA%\LumiViz Project\LumiViz\lyrics`, Vorgabe) oder **Next to the song** (gleicher Name
wie der Song, `.lrc` für synchronen, `.txt` für unsynchronen Text; ist der
Ordner schreibgeschützt, landet der Fund im Benutzerordner). Die Rechte an den
Texten liegen bei Dritten; sie sind für den eigenen Gebrauch gedacht.

**Falscher Text?** Unter der Tabelle im Editor stehen die Knöpfe in zwei
Reihen — oben **Alternatives…** und **Wrong lyrics**, darunter **Lyrics
file…** und **Open folder** (öffnet den Ordner der gezeigten Datei):

- **Alternatives…** sucht bei lrclib.net nach anderen Fassungen. Titel und
  Interpret sind aus den Tags vorbelegt und lassen sich ändern — nützlich bei
  Zusätzen wie „(Feat. …)" oder „(album version)". Die Liste zeigt Album,
  Dauer (mit Abweichung zur Datei), ob der Text synchron ist und die
  Zeilenzahl; die Fassung mit der passendsten Dauer steht oben, die gerade
  gezeigte ist **fett** und trägt den Vermerk *shown now*. Darunter steht der
  Text mit seinen Zeitmarken (farbig abgesetzt). Markierst du bis zu **drei**
  Einträge (Ctrl oder Shift), stehen sie nebeneinander. Läuft der Titel,
  zeigt ein Balken in jeder Spalte, wo der Song gerade ist (**Follow
  playback** hält die Stelle im Blick) — so siehst du, welche Fassung zum
  Gesang passt. Der Griff zwischen Liste und Texten teilt die Höhe.
  Ganz oben stehen Titel, **Länge** der Datei und die laufende Spielzeit —
  die Länge ist der Maßstab für die Spalte Duration. Wechselt der Song,
  bleibt das Fenster beim bisherigen Titel, sagt aber, was jetzt läuft, und
  bietet **Switch to the playing track** an.
  **Use these lyrics** über einer Spalte ersetzt die gespeicherte Fassung;
  die bisherige wird dabei nicht gesperrt, das Fenster bleibt offen. Die
  Suche sendet die beiden Suchbegriffe an den Dienst, auch wenn die
  selbsttätige Online-Suche aus ist.
- **Wrong lyrics** verwirft den gezeigten Text: die geholte Datei wird
  gelöscht und der Eintrag **gesperrt**, er wird nicht wieder geholt. Danach
  öffnen sich die Alternativen. Der Knopf gilt nur für online geholte Texte;
  eine selbst angelegte Datei wird nie gelöscht.

- **Lyrics file…** weist dem laufenden Titel eine vorhandene `.lrc`- oder
  `.txt`-Datei zu. Sie wird **unverändert kopiert** — in den Benutzerordner
  der Songtexte, unter `<Interpret> - <Titel>` (mit `.lrc`, wenn sie
  Zeitmarken trägt, sonst `.txt`); das Original bleibt, wo es ist. Eine dort
  schon liegende Datei dieses Namens wird ersetzt. Der Text erscheint sofort.
  Die Suchreihenfolge gilt weiter: hat der Titel einen eingebetteten
  synchronen Text oder liegt eine `.lrc` neben dem Song, gewinnt diese —
  LumiViz sagt dir dann, welche Quelle stattdessen gezeigt wird.

Die Sperrliste steht unter **Settings → Audio → Blocked lyrics** (**Show…**);
**Unblock** gibt einen Eintrag wieder frei. Sie liegt als `blocked.json` im
Benutzerordner der Songtexte.

**Kein Text beim Dienst?** Hat lrclib.net zu einem Titel nichts — unbekannt,
ohne Text eingetragen oder keine Fassung mit passender Dauer —, merkt sich
LumiViz das und fragt für diesen Titel **7 Tage** nicht wieder (bei als
instrumental eingetragenen Titeln **30 Tage**). In der Tabelle steht dann
gleich *No lyrics found — not online either.*, ohne dass etwas hinausgeht.
Danach wird von selbst wieder gefragt, denn der Dienst trägt fehlende Titel
nach. Sofort wieder fragen: **Settings → Audio → Lyrics not found → Ask
again** vergisst alle gemerkten Titel; dasselbe geschieht, wenn du in der
Sperrliste einen Eintrag freigibst. **Alternatives…** sucht immer, unabhängig
davon. War der Dienst nur beschäftigt oder das Netz weg, wird nichts gemerkt.
Die Merker liegen als `notfound.json` im Benutzerordner der Songtexte.

**Nachsynchronisieren: Panel „Lyrics Sync"** (View → Panels). Macht aus einem
unsynchronen Text einen synchronen, oder stellt einen synchronen nach. Das
Panel zeigt den Text des laufenden Titels als Liste mit den Spalten **Time**
und **Text**; die Zeile an der Abspielposition steht fett. Darüber läuft die
**Spielzeit** mit, und eine **Wellenform** zeigt vier Sekunden vor und nach
der Abspielposition (rote Marke in der Mitte). Die senkrechten Striche darin
sind die Zeiten der Zeilen, die markierte ist hervorgehoben — so siehst du, ob
ein Strich auf dem Einsatz sitzt.

| Bedienung | Wirkung |
|---|---|
| **Tap** (oder Enter bzw. T in der Liste) | gibt der markierten Zeile die Abspielposition und rückt zur nächsten Textzeile |
| **−100 ms** / **+100 ms** | verschiebt die markierten Zeilen |
| **From here −** / **From here +** | verschiebt alle Zeilen ab der ersten markierten |
| **Clear time** | nimmt den markierten Zeilen die Zeit |
| F2 in der Spalte **Time** | Zeit von Hand eintragen (`mm:ss.xx`) |
| Doppelklick auf eine Zeile | springt im Titel 1,5 s vor die Zeile (Vorhören) |
| **▶** in der Spalte **Go** | springt genau auf die Zeit der Zeile |
| Klick in die Wellenform | springt an diese Stelle |
| **Reload** | nimmt den Text des laufenden Titels neu — ungespeicherte Zeiten gehen dabei verloren |
| **Save** | schreibt eine `.lrc` neben die Textdatei (gleicher Name); bei eingebettetem Text dorthin, wo geholte Texte landen |
| **Undo** / **Redo** (Ctrl+Z / Ctrl+Y in der Liste) | nimmt den letzten Schritt an den Zeiten zurück (Klopfen, Verschieben, Zeit löschen, Zeit von Hand) bzw. stellt ihn wieder her. Nach **Undo** ist wieder die Zeile markiert, an der der Schritt begann — ein verklopfter Einsatz lässt sich gleich neu klopfen. **Save** und **Reload** leeren die Geschichte |
| **Track offset** | verschiebt den **ganzen** Text dieses Titels (positiv = früher), ohne die Zeiten der Zeilen zu ändern; wirkt sofort und wird je Titel gemerkt — **Save** ist dafür nicht nötig. Sitzt ein fertiger Text gleichmäßig daneben, ist das der kürzeste Weg |

Jede Änderung geht sofort an den Lyrics-Knoten — du siehst im Bild, was du
klopfst. Gespeichert wird erst mit **Save**, und nur, wenn jede Textzeile eine
Zeit hat (Leerzeilen brauchen keine). Die bisherige `.txt` bleibt liegen; die
Suche nimmt ab dann die `.lrc`. Wechselt der Titel, während ungespeicherte
Zeiten da sind, bleibt deine Arbeit im Panel stehen, bis du speicherst oder
**Reload** drückst.

**Lyrics Trail: die alte Zeile verwehen lassen.** Der Knoten **Lyrics Trail**
(Effekt-Palette → Images) zeichnet die Zeile, die eben zu Ende ging, noch
einmal — an seiner eigenen Stelle der Kette. Er hat keinen eigenen Text und
keine eigene Schrift: beides leiht er sich vom Lyrics-Knoten. Was in der Kette
*nach* ihm kommt, lässt die Zeile verschwinden. Das Muster:

```
Effect List            (Clear every frame aus, Input: Ignore, Output: Additive)
├─ Blitter Feedback    scale 35 — zieht das Bild der Liste zur Mitte, „nach hinten"
├─ Fadeout             4 — lässt es verblassen
└─ Lyrics Trail        zeichnet die alte Zeile frisch hinein
Lyrics                 die laufende Zeile, wie gewohnt
```

| Feld | Bedeutung |
|---|---|
| **Lyrics node** | Name des Lyrics-Knotens, dem der Trail folgt; leer = der erste der Kette |
| **Hold (ms)** | so lange wird die alte Zeile gezeichnet; danach tragen die Effekte sie weg |
| **X shift / Y shift (%)** | Versatz gegen die Stelle, an der die Zeile stand (negativ = nach oben) |
| **Own color** / **Color** | eigene Farbe statt der, die die Zeile hatte — die Zeile wird dafür einfarbig neu gezeichnet |
| **Blend** | wie beim Lyrics-Knoten |

Der Trail zeigt **das Bild, das stand**: der Lyrics-Knoten hebt das Bild der
laufenden Zeile auf und gibt es beim Zeilenwechsel an den Trail weiter. Was
die Zeile im Augenblick des Wechsels zeigte, weht also weg — Wort für Wort
Gesungenes in seinen zwei Farben, eine verkleinerte lange Zeile, und die
Stelle, an der sie stand, auch wenn ein Skript sie bewegt (etwa die
Voreinstellungen *Karaoke Bounce* und *Flip*). Hat der Lyrics-Knoten die
Zeile nicht selbst gezeichnet (er ist ausgeschaltet, oder du bist im Titel
gesprungen), zeichnet der Trail sie aus dem Text neu, ganz in der Farbe der
Zeile; mit **Own color** tut er das immer. Auf eine **Pause-Zeile** (Zeit
ohne Text) folgt kein zweiter Trail: die Zeile davor weht einmal weg, wenn
die Pause beginnt. Bei **unsynchronem Text** gibt es keine Zeiten; dort
zeichnet der Trail die Zeile, die der Lyrics-Knoten eben verlassen hat, wenn
dieser mit **Unsynced: beats per line** weiterrückt. Dafür muss der
Lyrics-Knoten eingeschaltet sein (bei synchronem Text genügt es, dass es ihn
gibt). Eine zu langsame Drift (Blitter
Feedback **scale** 33) lässt die Zeile zu einem Strich schrumpfen, statt sie
wegzutragen — 35 und mehr wirkt. Ein fertiges Beispiel steht in
`asset/effectchain/20261001.mix.lvfx2` (Liste „Lyrics Trail List").

**Karaoke-Ebene: Songtext über jedem Preset.** Mit **Karaoke on** im Panel
**Karaoke** (View → Panels) zeigt LumiViz den Songtext des laufenden Titels über **allem**,
was als Kette läuft — auch über importierten AVS- und MilkDrop-Presets, ohne
dass die Kette einen Lyrics-Knoten braucht. Die Ebene liegt über dem fertigen
Bild: sie wird von den Effekten des Presets nicht mitgezogen oder verwischt.
Sie zeichnet die laufende Zeile unten mittig mit einer Vorschau-Zeile, dazu
den Trail (die eben beendete Zeile weht nach hinten weg). Hat das laufende
Preset selbst einen **eingeschalteten** Lyrics-Knoten, **gewinnt das Preset**:
es zeigt den Text auf seine Art, und die Ebene tritt zurück — der Text steht
nie doppelt da. Das Panel zeigt dann neben dem Schalter **Preset shows its own
lyrics** — so ist erklärt, warum der Schalter gerade nichts zu ändern scheint.
In Screenshots aus der App ist die Ebene enthalten. Ein- und ausschalten
lässt sich die Ebene auch über **View → Karaoke** und mit **Ctrl+K**; alle
drei zeigen denselben Zustand.

Das **Aussehen der Zeile** stellst du im Karaoke-Panel im Reiter **Lyrics**
ein — dieselben Felder wie am Lyrics-Knoten einer Kette (Schrift, Farben,
Lage, Vorschau-Zeilen, Kontur, Skripte, die Zeile **Preset** mit den
Lyrics-Voreinstellungen und die Songtext-Tabelle). Jede Änderung gilt sofort
und bleibt über den Neustart erhalten. Den Stand der ganzen Ebene (Zeile und
Trail) hältst du als **Karaoke-Preset** (`.lvkar`) fest — über die
**Preset-Zeile** oben im Panel, dieselbe wie im Panel *Effect Chain* (siehe
§9 „Die Preset-Zeile"): **Save As…** legt ihn an einem Ort deiner Wahl ab,
**Save** schreibt in das laufende Karaoke-Preset, **Load…** und das Dropdown
holen einen Stand zurück, **Default** stellt das mitgelieferte Aussehen wieder
her. Hat die Ebene ungespeicherte Änderungen, fragt LumiViz vor dem Laden und
vor Default nach. Beim Beenden fragt es nicht: die laufende Ebene bleibt
ohnehin erhalten. Im **Effect Browser** erscheinen sie mit einem
eigenen Icon (Mikrofon im Kreis); ein **Doppelklick** lädt das Karaoke-Preset
in die Ebene — die Effektkette bleibt, wie sie ist, und das Karaoke-Panel
zeigt den neuen Stand. Eingeschaltet wird die Ebene dadurch nicht: das macht
weiter **Karaoke on**.

Der **Trail** hat seinen eigenen Reiter **Lyrics Trail**: oben **Trail on**
(aus = die Zeile steht ohne Nachwehen da), darunter eine kurze Liste von
Modulen, die nur auf dem Bild des Trails arbeiten — mitgeliefert Blitter
Feedback (lässt die alte Zeile nach hinten wegziehen) und Fadeout (lässt sie
verblassen). Mit **+** öffnet sich die Palette; sie zeigt hier nur Module, die
ein Bild **umformen** oder **färben** — was selbst zeichnet oder steuert,
gehört nicht in den Trail und lässt sich auch nicht hineinziehen. Am Ende der
Liste steht fest der **Lyrics Trail**: er lässt sich nicht entfernen und
bleibt immer das Letzte. An ihm stellst du Haltezeit, Versatz, eigene Farbe
und mit **Blend** / **Blend strength (%)** ein, wie sich der Trail mit dem
Bild mischt. Rechnet ein Preset mit **Render Scale** in kleinerer
Auflösung, bleibt die Zeile der Ebene trotzdem scharf — sie wird in der
Auflösung des Fensters gezeichnet; nur der Trail folgt der Auflösung des
Presets.

Wie sich die Zeile mit dem Bild dahinter **mischt**, stellst du im Reiter
**Lyrics** ein: Feld **Blend** und daneben **Blend strength (%)** von 0–100.
Die Stärke nimmt nur die Füllung der Buchstaben zurück, die Kontur bleibt
stehen. Beide Felder gibt es genauso am Lyrics-Knoten und am Lyrics Trail
einer Kette. Für den Trail der Ebene stehen dieselben beiden Felder am Lyrics
Trail im Reiter *Lyrics Trail* (Vorgabe: Additive, 100 %). (Bis Version 1.45 standen
Mischart und Stärke der Ebene unter Settings → Audio — die dort gewählten
Werte werden beim ersten Start übernommen.)

| Mischart | Wirkung |
|---|---|
| **Normal** | deckend über dem Bild (Vorgabe der Zeile) |
| **Additive** | hellt auf (Vorgabe des Trails) |
| **50/50** | halb Text, halb Bild |
| **Subtract** | dunkelt ab |
| **Invert** | weißer Text kehrt die Farbe darunter um; mit weniger Stärke nur teilweise — bei 50 % wird aus jeder Farbe Mittelgrau |
| **XOR** | bitweises Exklusiv-Oder wie in AVS; bei weißem Text wie Invert, bei farbigem bunt |
| **Multiply** · **Maximum** · **Minimum** | wie bei den Effect Lists; mit weißem Text bleiben von Multiply und Minimum nur die Konturen |

Bei jeder Mischart außer Normal mischt nur die **Füllung** der Buchstaben —
die Kontur bleibt normal gezeichnet, damit der Text lesbar bleibt. Über
mittelgrauem Bild geben Invert und XOR wenig Kontrast; dort trägt die Kontur.

**Zwei Fassungen desselben Songs.** Im Benutzerordner heißt die Datei
`<Interpret> - <Titel>`. Geholte Texte tragen dazu die Dauer des Titels
(`[length:…]`); passt sie nicht zum laufenden Titel (mehr als 2,5 s
Unterschied), gilt die Datei für ihn nicht, und sein Text liegt unter
`<Interpret> - <Titel> [m-ss]`. So überschreiben sich Album- und
Single-Fassung nicht mehr. Dateien ohne Dauerangabe gelten für jede Fassung.

**Wiederholte Zeilen.** Folgt dieselbe Zeile mehrmals hintereinander, blendet
sie kurz vor jeder Wiederholung zur Vorschau-Farbe und ist mit dem Einsatz
wieder da — so sieht man, wann die Wiederholung beginnt.

Jede so gespeicherte Datei trägt oben ihren **Herkunftsvermerk** — Dienst,
Kennung des Eintrags und Datum:

```
[re:LumiViz]
[source:LRCLIB id 3396226, fetched 2026-10-01]
```

Die beiden Kopfzeilen erscheinen nicht im Bild. Die Tabelle im Editor des
Lyrics-Knotens zeigt den Vermerk in der Zeile **Origin**, auch nach einem
Neustart. Dateien, die du selbst anlegst, bekommen keinen Vermerk.

**Fenstergröße ändern.** Das Bild der Effektkette und jeder Effect List bzw.
Host-Gruppe wird beim Größenwechsel in die neue Größe übernommen (wie in AVS
„reuse image on window resize"). Nachzüge — Blur, Fadeout, Trails — laufen beim
Ziehen am Fensterrand weiter, statt abzubrechen und sich neu aufzubauen.

**Script active** — gleich unter Name und Beschreibung, bei jedem Knoten mit
Formelfeldern: ausgeschaltet läuft der Knoten, **als stünden seine Felder
leer** (Init, Frame, Beat, Point, Song). Der Code bleibt gespeichert — zum
Vergleichen mit und ohne Skript, ohne etwas zu löschen. Shader (GLSL/HLSL)
sind davon nicht betroffen. Die Effect List hat dafür wie bisher ihr eigenes
„Use list code", Milkdrop-Nodes haben ihre Skripte im Preset. Ein Host
Switcher mit abgeschaltetem Skript wechselt weiter nach Zeit und Songwechsel.

**Song** — ganz unten im Editor jedes Knotens mit Frame-Feld: läuft **einmal
bei jedem Songwechsel**, direkt vor dem Frame-Feld, und sieht dieselben
Variablen wie die anderen Felder des Knotens. Damit reagiert ein Preset auf
einen neuen Titel — etwa eine neue Farbe würfeln oder einen Zähler
zurücksetzen:

```
Song:   farbe = rand(6); zaehler = 0;
```

Bei der Effect List wirkt das Song-Feld nur mit eingeschaltetem Listen-Code
(wie Init und Frame). Movement und Milkdrop-Nodes haben (noch) keins.

> **Movement ist die Ausnahme.** Es baut eine *statische* Tabelle — sein
> Point-Code läuft nur, wenn sich Größe oder Text ändern. Ein Wert, der sich
> über die Zeit ändert (ein Beat-Zähler etwa), kann das Bild dort nicht bewegen.
> Für Zeitabhängiges ist **Dynamic Movement** der richtige Knoten; der Editor
> weist darauf hin.

### Werte ohne Formel bewegen (Modulation)

Neben vielen Zahlfeldern steht ein kleiner **~**-Knopf. Er lässt das Feld um
seinen Reglerwert schwingen oder pumpen — ganz ohne Formel:

| Einstellung | Bedeutung |
|---|---|
| **Source** | **Bass**, **Mid**, **Treble**, **Level** (Lautstärke) und **Beat envelope** (springt beim Beat hoch und klingt ab) bewegen das Feld nur in eine Richtung (0..1). Die **LFOs** (sine, triangle, saw, square, random) schwingen nach oben und unten (−1..1). |
| **Depth** | Wie weit: 1 = der ganze Bereich des Felds, 0,1 = ein Zehntel. Negativ kehrt die Richtung um. Felder ohne festen Bereich bewegen sich um den entsprechenden Anteil ihres Reglerwerts. Der Wert bleibt immer im erlaubten Bereich. |
| **Rate** | Nur LFOs: Schwingungen pro Sekunde (Hz). |
| **Beat sync** | Nur LFOs: schwingt **im Takt der Musik** statt frei. Der Wert ist die Länge einer Schwingung in Beats — 4 = ein Takt, 1 = je Beat, 0,5 = zweimal je Beat. Die Schwingung beginnt auf dem Beat; alle Takt-LFOs laufen gleich. **off** = es gilt **Rate**. Das Tempo misst LumiViz aus den erkannten Beats; fällt ein Beat aus, läuft der LFO im letzten Tempo weiter. |
| **Smoothing** / **Decay** | Audio-Quellen: wie stark geglättet wird (Sekunden). Beat envelope: wie schnell sie nach einem Beat abklingt. |
| **Active** | Diese eine Modulation ein/aus. |

Ist eine Modulation gesetzt, ist der **~**-Knopf hervorgehoben, und daneben
steht der aktuelle Wert (**→ 0.73**). **Remove** im Dialog löscht sie.
Sobald ein Knoten Modulationen trägt, erscheint oben der Schalter
**Modulation active** — aus, und alle Felder stehen auf ihrem Reglerwert.

Die Reihenfolge je Bild: **Regler → Modulation → Formel.** Eine Formel sieht
den modulierten Wert und darf ihn überschreiben. Modulationen wirken auch bei
ausgeschaltetem *Script active*. Sie werden mit der Kette gespeichert.

Modulierbar sind die Zahlfelder der Knoten mit Formelfeldern, dazu
**SuperScope** (Point count, Line width, Dot size, Cycle frames), **Effect
List** (In/Out Alpha, In/Out Buffer, OnBeat frames), **Host-Gruppe** (Out
Alpha, Crossfade duration), **Dynamic Movement** (Source buffer), **Host
Switcher** (Switch after), **Text**
(Word time, Beat hold), **AVI** (Persist, Frame time) und Debug Bars (Orbit
speed). Zwei Besonderheiten:

- **SuperScope, Point count:** jede Änderung des modulierten Werts setzt `n`
  im Skript neu — wie eine Reglerbewegung. Ein Skript, das `n` selbst setzt,
  gewinnt nur in den Slots, die danach laufen.
- **Effect List mit Listen-Code:** `alphain` und `alphaout` stehen zu Beginn
  jedes Bilds auf dem Reglerwert bzw. dem modulierten Wert (wie in AVS). Der
  Frame-Code sieht ihn und darf ihn überschreiben; er schreibt ihn aber nicht
  von Bild zu Bild fort.

**Farbfelder** tragen den **~**-Knopf ebenfalls. Dort wählt **Target**, was
sich bewegt: **Brightness** (Helligkeit), **Hue** (Farbton — Depth 1 ist ein
voller Umlauf im Farbkreis; bei Grau ohne Wirkung) oder ein einzelner Kanal
**Red**, **Green**, **Blue**. Depth 1 ist jeweils der ganze Bereich. Der
laufende Wert steht als `#RRGGBB` daneben. Je Farbfeld gibt es eine
Modulation; Farbtafeln (etwa die Farbliste des SuperScope) und Farben, die
den Knoten neu aufbauen (Text), sind nicht modulierbar.

Noch **nicht** modulierbar: Schalter und Auswahlfelder, sowie Render Scale.

### Bilder in Picture, Picture II und Texer

Die Bild-Knoten zeigen in der Zeile **Image**, ob ein Bild eingebettet ist.
**Choose…** wählt eine Datei und bettet sie in die Kette ein — ein gespeichertes
Preset bleibt damit für sich allein lauffähig. **Clear** wirft das Bild wieder
raus.

Findet der Import das Bild eines AVS-Presets nicht neben der Datei, sucht er bis
zu drei Ordner aufwärts und zuletzt im **Bilder-Suchordner** aus den
Einstellungen (§8).

### Milkdrop-Presets bearbeiten

Ein geladenes `.milk` erscheint als **Milkdrop-Node** mit sechs Sektionen im
Baum: **Code** (Init/Frame/Point-EEL) · **Waves** · **Shapes** · **Shader**
(Warp/Comp-HLSL) · **Sprites** · **Parameters (base values)** (alle numerischen Basiswerte:
Decay, Gamma, Echo, fShader-Farbwash, Waveform, Motion, Borders, Motion
Vectors, Blur).

- **Wave/Shape/Sprite anlegen:** im Toolbar-Dropdown „Custom Wave", „Custom
  Shape" oder „Sprite" wählen und **+** drücken (Milkdrop-Node oder eine
  seiner Sektionen muss selektiert sein). **Entfernen/Klonen:** das Element
  im Baum markieren, dann **−**/**⧉**. Ein Element anklicken zeigt seinen
  Voll-Editor (numerische Startwerte + Code).
- **ⓘ neben jedem Skript-Feld** öffnet die zur Sektion passende
  Variablen-Referenz (per_frame/per_pixel, Wave, Shape, Sprite) — nur
  Original-MilkDrop-Variablen, damit Presets kompatibel bleiben. Auch die
  **Shader-Editoren** haben ein ⓘ (Inputs, Konstanten, Sampler, Funktionen).
- Hinweis: Presets mit eigenem Comp-Shader „backen" Gamma/Echo/Filter/
  fShader in den Shader ein — die Parameter-Regler wirken dort erst, wenn
  der Shader geleert wird (Original-Verhalten).

### Preset-Wechsel: das geerbte Bild

MilkDrop löscht den Bildpuffer beim Preset-Wechsel **nie** — jedes Preset
startet auf dem letzten Bild seines Vorgängers. Das ist Kern-Ästhetik des
Originals: „Verstärker"-Presets ohne eigene Energiequelle leben von diesem
Erbe, und chaotische Feedback-Presets (z. B. die *Rock The House*-Familie)
kippen je nach Vorgänger dauerhaft in **andere Farb-/Form-Äste**. Dasselbe
Preset kann nach Vorgänger A also anders aussehen als nach Vorgänger B oder
nach einem App-Start — das ist kein Fehler, sondern Original-Verhalten.

Seit Session 66 ist das Verhalten einstellbar. Im Milkdrop-Node (Sektion
**Parameters**) steht **„Buffer on preset change"** (Puffer bei Preset-Wechsel):

| Einstellung | Wirkung beim Laden des nächsten Presets |
|---|---|
| **App setting** | folgt dem Standard aus *Settings → Panels* (Vorgabe: *Keep image (original)*) |
| **Keep (original)** | das Bild bleibt vollständig — Original-MilkDrop-Semantik |
| **Clear** | frischer Start: deterministische Rausch-Saat + neu gewürfelte Preset-Zufallswerte (Rotations-/Farbphasen wie bei einer frischen Instanz). Die Audio-Analyse läuft bewusst eingeschwungen weiter (ein Reset ließe laufende Musik auf der leeren Rampe zu extremen Beat-Spitzen explodieren). **Wichtig:** Was die Musik gerade liefert, prägt ein reaktives Preset immer — leiser App-Start und volle Musik ergeben zwangsläufig verschiedene Bilder; das ist Eingabe, kein geerbter Zustand |
| **Fading (one-off mix)** | Mix **im Moment des Wechsels**: der Regler **Fading inheritance (%)** bestimmt, wie viel vom alten Bild als Startpunkt überlebt (0 % = *Clear*, 100 % = *Keep*) — kein zeitlicher Verlauf |
| **Fade out (over time)** | das geerbte Bild **stirbt über die Ausblend-Dauer (*Fade-out duration*) weg**: das Echo wird jedes Bild zusätzlich gedämpft und nicht nachgespeist; was das neue Preset frisch zeichnet, bleibt. Nach Ablauf ist nur noch das neue Preset zu sehen |

> **„Clear" ist nicht Schwarz:** Der frische Start ist die Rausch-Saat des
> Kaltstarts — mit Schwarz blieben „Verstärker"-Presets ohne eigene
> Energiequelle für immer dunkel. Presets wie *Rock The House* zünden aus der
> Saat in unter einer Sekunde wieder ein Vollbild; entscheidend ist, dass es
> **ihr eigenes** Bild ist und nicht die Farben/Formen des Vorgängers trägt.
>
> Damit die Saat selbst nicht als Rausch-Blitz zu sehen ist, blendet
> **„MilkDrop Start Fade-in"** (*Settings → Panels*, Vorgabe: an) das
> Bild nach jedem frischen Saat-Start (App-Start, Größenwechsel, *Clear*)
> etwa eine halbe Sekunde von Schwarz ein — rein kosmetisch, die Preset-
> Dynamik und die Saat-Energie bleiben unangetastet.

Die Einstellung gehört zum **Node**, nicht zum Preset — sie überlebt das
Durchblättern von Presets (Bild auf/ab) und wird mit der Kette gespeichert.

![Milkdrop-Node: Buffer on preset change](bilder/handbuch/milkdrop_puffer_wechsel.png)

Der App-Standard dazu steht in *Settings → Panels* („MilkDrop Preset
Switch" + „MilkDrop Fade Amount" — der Prozentwert ist nur im Fade-Modus
aktiv):

![Settings: MilkDrop Preset Switch](bilder/handbuch/settings_milkdrop_wechsel.png)

**Was bei „Keep" genau erhalten bleibt** — die Erbschaft ist bewusst
klein:

- **Das Bild im Feedback-Puffer** (beide Hälften des Doppelpuffers). Es ist
  die einzige echte Erbschaft und überlebt auch Fenstergrößen-Änderungen
  (das Bild wird beim Resize umkopiert).
- **Die Lautstärke-Historie** der Audio-Analyse (die Langzeit-Mittel hinter
  `bass`/`mid`/`treb` und `*_att`) läuft über den Wechsel weiter — wie im
  Original, wo die Sound-Analyse global ist.
- **Die Host-Einstellungen des Nodes:** Mesh X/Y, Kalibrier-Raster,
  Textur-Basisordner und der Puffer-Wechsel-Schalter selbst.

**Alles andere startet bei jedem Preset-Wechsel neu:**

- Alle **Skripte**: `per_frame_init` läuft neu; `q1..q64`, `reg00..reg99`,
  `gmegabuf`, eigene Variablen und `monitor` sind frisch.
- **`time`, `frame`, `progress`** beginnen bei 0.
- **Custom Waves/Shapes/Sprites** (inkl. `t1..t8`-Schnappschüsse und
  Sprite-Lebenszustand).
- **Warp-/Comp-Shader** werden neu übersetzt, **Texturen** neu geladen,
  die fShader-Farbwash-Phasen neu bestimmt.

Zwei Grenzfälle: Beim Laden einer **ganzen Ketten-Datei** (.lvfx-Kette) oder
beim Visualizer-Wechsel wird die Laufzeit immer komplett neu aufgebaut — dort
gibt es kein Erbe, unabhängig vom Schalter. Und der Prüfstand-Schalter
`LUMIVIZ_MILKDROP_NOSEED` macht die „frische Saat" schwarz statt verrauscht
(Kalibrier-Werkzeug, kein App-Zustand).

### Regelwerk (Rule set): Legacy / Modern / Custom

Jeder Milkdrop-Node deklariert in der **Parameters**-Sektion seine Betriebsart
(**Rule set**, Session 65):

- **Legacy (faithful to original)** — Standard bei jedem Import: alle vier
  D3D9-Emulationen sind aktiv, das Bild entspricht dem kalibrierten
  Original-Verhalten.
- **Modern (IEEE)** — reines IEEE/GL-Rechnen ohne die Legacy-Krücken; für
  eigene, neue Presets.
- **Custom** — die vier Einzelschalter zählen:
  **D3D9 division contract** (0/0 = 0), **UNORM8 truncation** im Feedback-Pfad,
  **q garbage epsilon** für Presets ohne `per_frame_init`, **UV sanitize**
  (NaN→0 + Riesen-UV-Wrap). Jeder Tooltip nennt den Beweis-Fall.

Dazu je Shader-Stufe ein **PS-Version-Override** (**PS version Warp** /
**PS version Comp**: Auto (.milk)/PS2/PS3/„Force MD1" — Letzteres ignoriert
den Custom-Shader-Text und rendert den exakten MD1-Pfad).

**Script random** (Skript-Zufall, Session 80): wie `rand()` im Code des Presets würfelt.

- **Original (ns-eel2)** — **Standard**, auch für ältere gespeicherte
  Ketten. Wie MilkDrop 2: eine Kommazahl zwischen 0 und x, aus einem
  Generator mit festem Startwert. Presets, die im Init würfeln (z. B.
  *martin – stratospheric turbulences*), zeigen damit dieselbe Szene wie das
  Original; Zufall je Gitterpunkt wirkt nachweislich wie im Original.
- **LumiViz (previous)** — ganze Zahlen, `rand(1)` ist dann immer 0. Holt den
  Look zurück, den MilkDrop-Presets in LumiViz bis Session 80 hatten.

**Audio** (Session 80): woraus `bass`/`mid`/`treb` (samt `_att`) und die Werte
der Custom-Waves (`value1`/`value2`) entstehen.

- **Original (MilkDrop)** — **Standard**, auch für ältere gespeicherte
  Ketten. Die Aufbereitung von MilkDrop 2: aus der 8-bit-Wellenform ein
  eigenes FFT (mit Hüllkurve und Entzerrung), die Welle am Vorframe
  ausgerichtet, daraus die Bandenergien. Gegen den Original-Kern geprüft
  (identische Werte).
- **LumiViz (previous)** — das Spektrum der App mit geschätzten Faktoren; der
  Klang, den MilkDrop-Presets in LumiViz bis Session 80 hatten.

**Aspect ratio (waves)** (Seitenverhältnis, Session 81): wie die Custom-Waves ihre
x/y-Werte aufs Bild legen.

- **Original (MilkDrop)** — **Standard**, auch für ältere gespeicherte
  Ketten. Wie MilkDrop: Auf einem Breitbild wird die Kurve senkrecht um das
  Seitenverhältnis gestreckt, um die Bildmitte (4:3 um 4/3, 16:9 um 16/9).
  Ein Wert `y = 0.1` liegt dann schon unter dem unteren Rand. So haben die
  Autoren ihre Presets gesehen; auf quadratischem Bild ändert sich nichts.
- **LumiViz (previous)** — x/y sind Anteile des Bildes (0 = linker/unterer,
  1 = rechter/oberer Rand), die Kurve bleibt ungestreckt im Bild. Der Look
  von Session 39 bis 80.

### Alte Verhaltensweisen zurückholen („bisher"-Schalter)

Wenn LumiViz einem Original-Verhalten näherkommt und sich das Bild dadurch
sichtbar ändert, wird das neue Verhalten **Standard** — das alte bleibt über
einen Schalter erreichbar, damit eigene Ketten ihren gewohnten Look behalten
können. Alle diese Schalter im Überblick (Milkdrop-Node, **Parameters**-Sektion):

| Schalter | Standard | „Bisher" holt zurück | Seit |
|---|---|---|---|
| **Script random** | Original (ns-eel2) | LumiViz (previous): ganze Zahlen aus `rand()` | Session 80 |
| **Audio** | Original (MilkDrop) | LumiViz (previous): App-Spektrum mit geschätzten Faktoren | Session 80 |
| **Aspect ratio (waves)** | Original (MilkDrop) | LumiViz (previous): Custom-Waves ungestreckt, bildfüllend | Session 81 |
| **Styling → Setting „Soft pull ↖"** | aus | den leichten Sog nach oben links samt Weichzeichnen, den jedes MilkDrop-Preset bis Session 80 hatte (damals ein Versatzfehler, heute ein Gestaltungsmittel) | Session 80 |

**Nicht verwechseln:** Das **Regelwerk** (*Rule set*) **„Legacy"** oben ist *kein* „bisher"-
Schalter — es ist die original-treue Betriebsart (D3D9-Verhalten nachgebildet)
und ohnehin Standard.

### Styling: Drift und Weichzeichnen (kein Original-Verhalten)

In der **Parameters**-Sektion eines Milkdrop-Nodes, Gruppe **„Styling (not
original behaviour)"** (Gestaltung, Session 80). Nichts davon kennt das Original — es sind Gestaltungsmittel.
Bei jedem Import und jedem neuen Node steht alles **aus**. Jedes Werkzeug hat
einen eigenen Schalter; ausgeschaltet bleiben seine Werte stehen.

| Werkzeug | Regler | Wirkung |
|---|---|---|
| **Linear drift** | Strength 0 … 4 px/Frame · Direction 0 … 360° (0 = rechts, 90 = oben, 180 = links, 270 = unten) | Das ganze Bild wandert in eine Richtung, Nachbilder ziehen Schlieren. Bleibt **scharf**: Bruchteile werden aufgespart (0,5 px = jeden zweiten Frame ein ganzes Pixel) |
| **Radial drift** | Strength 0 … 4 px/Frame · Angle 0 … 360° · Centre X/Y 0 … 1 (0 = links bzw. unten) | Jeder Punkt wandert **gleich schnell**, die Richtung hängt von seiner Lage zum Zentrum ab: **0° nach außen**, **90° Wirbel gegen den Uhrzeigersinn**, **180° Sog nach innen**, **270° Wirbel im Uhrzeigersinn**, dazwischen Spiralen. Anders als `zoom`/`rot` (am Rand schneller) überall gleich schnell. Zeichnet immer etwas weich |
| **Soften** (Weichzeichnen) | Strength 0 … 1 | Jeder Frame wird mit dem Nachbarpixel gemischt, ohne das Bild zu verschieben. Bei `decay = 1` wirkt das wie ein langsames Ausblenden |

1 px/Frame sind bei 60 fps 60 Pixel je Sekunde. Linear und radial lassen sich
kombinieren (z. B. Wirbel, der langsam nach oben abdriftet).

Zwei Knöpfe in der Zeile **Setting** setzen bekannte Stellungen:

- **Original** — alle drei aus: das Bild steht still und scharf, wie im
  Original-MilkDrop.
- **Soft pull ↖** (weicher Sog) — Linear drift 0,7071 px bei 135° plus Soften 1:
  das Bild zieht weich nach oben links weg. Das ist der Look, den LumiViz **bis
  Session 80** bei jedem Milkdrop-Preset hatte — damals ungewollt (ein
  Portierungsfehler, der den D3D9-Halbpixel-Ausgleich unter OpenGL doppelt
  anwandte). Bitgleich nachgebildet: wer ein Preset „wie früher" sehen will,
  drückt diesen Knopf.

**Per Skript steuerbar.** Im `per_frame`-Code des Nodes stehen die Werte als
Variablen bereit — jeder Frame beginnt mit den Panel-Werten, das Skript darf
sie ändern (wie `zoom` oder `rot`). **Winkel dort in Bogenmaß** (wie `rot`,
`ang`): 90° = π/2 ≈ 1,5708.

| Variable | Bedeutung |
|---|---|
| `drift_lin`, `drift_dir` | Linear drift: Strength (px/Frame), Direction (rad, 0 = rechts, π/2 = oben) |
| `drift_rad`, `drift_ang` | Radial drift: Strength, Angle (rad, 0 = außen, π/2 = Wirbel ↺, π = innen) |
| `drift_cx`, `drift_cy` | Centre X/Y der Radial drift (0 … 1, y: 0 = unten) |
| `weich` | Soften (0 … 1) |

Wirksam ist ein Wert nur, solange der Schalter seines Werkzeugs an ist. Im
Skript sind auch negative Stärken erlaubt (bis ±8; negativ = Gegenrichtung).
Beispiele:

```
drift_dir = time * 0.5;                  // Linear-Drift dreht langsam im Kreis
drift_ang = 1.5708 + bass_att * 0.8;     // Wirbel, der bei Bass zum Sog aufzieht
drift_rad = if(above(bass, 1.3), 3, 0.5); // Stoss nach aussen auf Bass-Spitzen
drift_cx = 0.5 + 0.3 * sin(time);        // Zentrum pendelt links/rechts
```

Einen eigenen Beat-Slot hat MilkDrop nicht — Beat-Reaktionen laufen über
`bass`, `bass_att` & Co. im `per_frame`-Code.

### Host-Gruppen & Crossfade

Eine **Host-Gruppe** (Dropdown „Host Group") kapselt ein komplettes Visual —
eine ganze AVS-Kette, ein Milkdrop-Preset oder eigene Effekte — mit eigenem
Feedback-Bild, eigenen Buffer-Slots und eigenen Skript-Variablen. Mehrere
Gruppen dürfen gleichzeitig aktiv sein und mischen sich über ihren
**Blend Out**; eine Gruppe in einer Gruppe ist nicht erlaubt.

- **Crossfade:** Das Auge einer Gruppe blendet sie **weich** ein/aus
  (Dauer: „Crossfade duration (s)" — gilt synchron für alle Gruppen;
  **Fade-in curve** / **Fade-out curve** je Gruppe: Linear, S-curve (smooth),
  Ease-In, Ease-Out, Exponential). Beide Visuals laufen während des Übergangs live weiter.
- **„Switch to this group (crossfade)"** im Gruppen-Editor blendet die
  gewählte Gruppe ein und alle anderen aus — der schnelle Preset-Wechsel.
- **Ein fertiges Preset als Gruppe:** In der Effekt-Palette legt
  **„Preset as Group…"** eine neue Host-Gruppe an und fragt nach der Datei —
  `.avs`, `.milk` oder `.lvfx`. Der Inhalt ist eine **Kopie**: du kannst ihn
  im Baum frei ändern, die Datei bleibt unberührt. Der Gruppen-Editor zeigt
  die Quelle (**Source:**); **„Reload from source"** liest die Datei erneut ein
  (ersetzt den Inhalt, fragt vorher). Grenzen: Beat/BPM, Multi-Delay und
  Render Scale gelten für die ganze Kette, nicht je Gruppe — ein AVS-Preset
  bekommt in der Gruppe deshalb **keinen** eigenen Render-Scale-Knoten; eine
  `.lvfx` mit eigenen Host-Gruppen läuft in der Gruppe als Effect List.
  Hatte die Wurzel des Presets eigene Einstellungen (etwa AVS „clear every
  frame"), bleibt sie als eine Effect List in der Gruppe erhalten.
- **„Load preset into this group (.avs, .milk, .lvfx)..."** macht dasselbe
  für eine bestehende Gruppe (ersetzt deren Inhalt).
- **Presetverweis statt Kopie:** **„Preset Reference…"** in der Palette legt
  eine Gruppe an, die nur **auf die Datei verweist**. Gespeichert wird nur
  der Pfad; beim Laden der Kette wird das Preset jedes Mal frisch aus der
  Datei gelesen — so baust du z. B. eine Show-Datei aus fertigen Presets, die
  du weiter einzeln pflegst. Der Inhalt erscheint nicht im Baum (Änderungen
  daran gingen beim Speichern verloren); alle Gruppen-Einstellungen (Blend,
  Crossfade, Kurven, Skripte) gelten wie gewohnt, und der **Host Switcher**
  schaltet Verweise wie jede andere Gruppe. Im Editor: Pfad eintippen oder
  **Open…**, **Reload** liest die Datei erneut. Ein relativer Pfad gilt
  ab dem Ordner der Kettendatei. Der Schalter **„Preset reference (save the
  path only)"** wandelt
  eine kopierte Gruppe in einen Verweis um (Inhalt wird neu gelesen) und
  zurück (der geladene Inhalt bleibt als Kopie). Verweist eine Datei über
  Umwege auf sich selbst, bricht das Laden dort mit einer Meldung ab.
- **Presets in die Kette ziehen** (seit Session 84): Zieh eine `.avs`,
  `.milk` oder `.lvfx` aus dem **Effect Browser** — oder aus dem
  Windows-Explorer — in den Baum der Effect Chain. Sie wird dort, wo du sie
  fallen lässt, ein **Presetverweis** (wie „Preset Reference…", nur ohne
  Dateidialog): auf eine Liste = hinein, an eine Kante = dazwischen, auf die
  Leerfläche = ans Ende. Mehrere Dateien auf einmal landen in ihrer
  Reihenfolge. Ordner lassen sich nicht ziehen; die Datei selbst wird dabei
  nie verschoben. Innerhalb einer Host-Gruppe gilt die Tiefenregel wie sonst.
- **Namen sind eindeutig:** Der Name einer Gruppe ist ihre ID für den Host
  Switcher. Eine zweite „Host Group" heißt deshalb automatisch „Host Group 2"
  (auch beim Klonen und beim Laden älterer Dateien). Benennst du eine Gruppe
  um, ziehen alle Switcher-Listen mit; ist der Name schon vergeben, bekommt
  er den nächsten freien Zähler.

#### Host Switcher — Gruppen automatisch wechseln

Der **Host Switcher** (Palette, *Structure & Control*) schaltet zwischen
Host-Gruppen um; der Übergang ist der Crossfade der Gruppen. Er gehört auf
die oberste Ebene der Kette.

- **Groups:** Aus der Auswahl die Gruppen des geladenen Presets
  hinzufügen (**Add**), mit ▲/▼ umsortieren, entfernen (**Remove**). Die Nummer vor dem
  Namen ist der **Index** (1, 2, …), den Skripte benutzen. Eine Gruppe, die
  es in der Kette nicht mehr gibt, ist als „(missing in the chain)" markiert.
  *Running* zeigt die gerade aktive Gruppe.
- **Start group:** die Gruppe beim Laden.
- **Switch after (s):** nach so vielen Sekunden zur nächsten Gruppe (0 = aus).
- **Next on song change:** beim nächsten Titel zur nächsten Gruppe.
- **Order:** nächste Gruppe nach Liste (*List order*) oder zufällig (*Random*, nie dieselbe).
- **Keep group state:** an = jede Gruppe der Liste **merkt sich ihren Stand**,
  solange sie ruht — Bild, Zwischenpuffer, Skriptwerte, Laufzeit — und läuft
  beim nächsten Betreten **dort weiter**, wo sie verlassen wurde. Aus (Vorgabe)
  = eine Gruppe beginnt jedes Mal mit leerem Bild. Die ruhenden Gruppen
  rechnen nicht weiter; sie halten nur ihren Stand (und dessen Speicher).
- **Reset buffers:** leert die Puffer der ruhenden Gruppen — sie beginnen
  beim nächsten Betreten **ganz frisch** (auch Skripte und Partikel starten
  neu). Die gerade laufende Gruppe bleibt unberührt. Im Skript:
  `clearbuffers = 1`, z. B. im Song-Feld, damit jeder Titel frisch beginnt.
- **Skripte** Init / Frame / Beat / **Song** (läuft einmal bei jedem
  Songwechsel). Lesen: `index`, `count`, `seconds` und `beatcount` (beide
  **0 direkt nach einem Wechsel** — so erkennt der Frame-Slot, dass gerade
  umgeschaltet wurde), dazu `bass`/`mid`/`treb`, `getspec` usw. Schreiben:
  `switch = n` (Gruppe n), `nextgroup = 1` (nächste) oder `clearbuffers = 1`
  (Puffer leeren, s. o.). Alle drei gelten nur für den Frame, in dem sie
  gesetzt werden. Beispiel alle 32 Beats:
  `nextgroup = above(beatcount, 31);`
- **Voreinstellungen** (Zeile *Preset* oben im Editor): sechs fertige
  Wechsel-Regeln. Sie setzen nur Wechselart und Skripte — die **Gruppenliste
  und die Startgruppe bleiben, wie sie sind**. Die Zahlen stehen als Variablen
  im Init-Feld und lassen sich dort ändern.

  | Voreinstellung | Wechselt … | Stellgrößen (Init) |
  |---|---|---|
  | **Random Runtime** | nach einer zufälligen Zahl Beats, zufällige Reihenfolge; die Zahl wird je Song neu gewürfelt | `min_runtime` 5, `max_runtime` 35 |
  | **Random Runtime per Group** | wie oben, aber jede Gruppe würfelt ihre eigene Laufzeit | `min_runtime` 5, `max_runtime` 35 |
  | **Random Seconds** | nach einer zufälligen Zeit in Sekunden, je Gruppe neu gewürfelt, zufällige Reihenfolge | `min_seconds` 10, `max_seconds` 40 |
  | **Every 16 Beats** | alle 16 Beats (vier Takte im 4/4), in Listenreihenfolge | `beats_per_group` 16 |
  | **Bass Drop** | bei einem Bass-Schlag, der das laufende Mittel um den Faktor `kick` übertrifft — frühestens nach `min_seconds`, spätestens nach `max_seconds`; zufällige Reihenfolge | `min_seconds` 8, `max_seconds` 60, `kick` 1.8 |
  | **One per Song** | bei jedem Songwechsel (**Next on song change**), zufällige Reihenfolge; kein Skript | — |
- **Gespeichert wird, was du eingestellt hast:** Der Switcher schaltet nur
  zur Laufzeit; die Augen der Gruppen in der Datei bleiben unverändert.
  Solange ein Switcher eine Gruppe führt, entscheidet er — ihr Auge und
  „Switch to this group" wirken erst wieder, wenn der Switcher aus ist
  (sein Auge) oder die Gruppe nicht in seiner Liste steht.
- **Speichern:** Ketten mit Host-Gruppe(n) werden automatisch als
  **`.lvfx2`** gespeichert, flache Ketten bleiben `.lvfx`; laden kann die
  App beides. Im Effect Browser trägt ein `.lvfx2` ein eigenes Icon — das
  LumiViz-Icon als **Stapel** —, ein `.lvfx` das einfache.

---

## 12. Shadertoy

Seit Session 65 gibt es den Ketten-Knoten **„Shadertoy (GLSL)"**: er führt
einen Shadertoy-Shader (`mainImage`-Funktion) als Effekt in der Kette aus —
mit dem vollen Uniform-Satz (`iTime`, `iResolution`, `iMouse`, `iFrame` …)
plus LumiViz-Extras (`bass`, `mid`, `treb`, `vol`, `beat`).

### Was an einem `iChannel` hängen kann

Jeder der vier `iChannel` bekommt seine Quelle über eine eigene Auswahl —
im Image-Pass und in jedem Buffer-Pass getrennt.

| Quelle | Was ankommt | Wofür |
|---|---|---|
| **None** | schwarz | Kanal bewusst leer lassen |
| **Buffer A–D** | Ausgang des jeweiligen Puffers | Multipass wie auf shadertoy.com. Eine Referenz auf sich selbst oder einen **späteren** Puffer liest das **Vorframe** (Feedback), eine auf einen früheren das frische Bild dieses Frames. |
| **Audio** | 512×2-Textur im Shadertoy-Layout (Zeile 0 = Spektrum, Zeile 1 = Waveform) | alles Audioreaktive — dazu gibt es `bass`/`mid`/`treb`/`vol`/`beat` auch als einfache Zahlen |
| **Chain input** (Ketten-Eingang) *(neu, S72)* | das Bild, das dieser Knoten von der Kette vorfindet | **Shadertoy-Bildfilter laufen damit direkt.** Sehr viele Shader auf shadertoy.com erwarten an `iChannel0` ein Bild und verzerren/färben es — ohne diese Quelle blieb das schwarz. Beispiel: einen Blur-/Glitch-Shader hinter einen Superscope hängen. |
| **AVS buffer 1–8** *(neu, S72)* | der Inhalt eines „Buffer Save"-Slots | ein **eingefrorenes** Bild als Textur weiterverwenden: irgendwo in der Kette mit „Buffer Save" in Slot 3 schreiben, hier Slot 3 lesen. Der Stand ist der **beim Lauf dieses Knotens** — die Reihenfolge in der Kette zählt also, genau wie bei Buffer Save selbst. Ein nie beschriebener Slot ist schwarz. |

In Host-Gruppen greifen die AVS-Buffer automatisch auf die Puffer **der
eigenen Gruppe** zu — dieselbe Trennung wie bei Buffer Save.

- **Blend:** Replace, Additive oder 50/50 mit dem Ketten-Bild darunter.
- **Fehler:** Kompilierfehler (**⚠ Compile error**) zeigen die Zeilennummern **deines** Codes; der
  Knoten reicht das Bild solange unverändert durch.
- **Editor-Komfort:** Groß-Editor und ⓘ-Referenz (Uniforms, Audio-Layout,
  Buffer) an jedem GLSL-Feld.

**Import von shadertoy.com:** Im Knoten-Editor unter **Shadertoy URL / ID** eine
Shadertoy-URL oder -ID eintragen und den Import-Knopf **⤓** drücken (Netzzugriff nur auf Knopfdruck; dafür
ist ein Shadertoy-**API-Key** in den Einstellungen nötig — die API liefert
nur Shader der Sichtbarkeit „public + api"). Der **Shadertoy-Browser**
(Panel) sucht mit Stichwort und Sortierung, zeigt Thumbnails, und ein
Doppelklick lädt den Shader als Ein-Knoten-Kette.

**Mitgeliefert:** 100 eigene, portable Shader in vier Serien liegen als
fertige Ketten unter `asset/effectchain/shadertoys/` — jeder mit einem
„STELLSCHRAUBEN"-Konstantenblock am Anfang zum Spielen.

---

## 13. Video & Kamera als Quelle

Seit Session 70 gibt es den Ketten-Knoten **„Video/Camera (Source)"**
(Palette, Rubrik „Scopes & Sources"): er zeichnet Videobilder auf das
Ketten-Bild — als Startpunkt einer Kette oder als Ebene mitten drin.

**Drei Quellen** (Umschalter **Source** im Knoten):

- **File** — MP4, MKV, WebM, MOV, WMV, AVI (über das FFmpeg-Backend von
  Qt Multimedia); Feld **Video file** bzw. Knopf **Choose video file…**.
  Der Pfad bleibt im Preset stehen; Videos werden **nicht**
  eingebettet. Relative Pfade werden beim Laden gegen den Preset-Ordner
  aufgelöst.
- **Camera (live)** — Gerät unter **Camera** wählen. **Eine Kamera startet nie
  von selbst:** erst der Knopf „Allow camera (this app run)" schaltet
  sie frei (dann fragt Windows ggf. nach der Berechtigung). Beim Laden eines
  Presets mit Kamera-Quelle fragt die App einmal per Dialog
  („Camera source in preset"), ob sie freigeben soll — ohne Freigabe bleibt
  der Knoten schwarz.
- **Camera Clip** — ein kurzer, aufgezeichneter Kamera-Clip aus den
  Einstellungen (s. u.), der wie eine Datei abgespielt wird: eine
  reproduzierbare Kamera-Quelle — auch dann nutzbar, wenn gerade keine
  Kamera angeschlossen ist.

**Abspielverhalten:**

- **Real-time streaming** (Schalter): an = die Datei streamt uhrgetrieben
  (beliebig lange Videos, wenig Speicher); aus = **Frame-Schritt** — die
  Datei wird einmal komplett dekodiert und läuft dann deterministisch mit
  der Sim-Uhr (kurze Clips; zwei Läufe sind bit-identisch).
- **Speed** (0,05–20×), **Loop** (aus = letztes Bild halten),
  **Opacity** und **Blend** (Replace/Additive/50/50).
- **Fit** (Einpassung): Stretch · Fit (letterbox) (Balken zeigen das
  Ketten-Bild darunter) · Fill (crop).
- **Parameter-Skripte** `init`/`frame`/`beat` können `speed` und `opacity`
  audio-reaktiv steuern (`bass`, `mid`, `treb`, `vol`, `beat`).

**Camera Clips (Settings → Camera):** Gerät wählen (**Camera**, ggf.
**Rescan Devices**), **Duration** einstellen, **„● Record Camera Clip"** —
der Clip landet benutzerlokal (nicht im Projekt) und erscheint danach im
Knoten unter Source „Camera Clip". Der Aufnahme-Klick erteilt zugleich die
Kamera-Freigabe des App-Laufs. Die Liste **Camera clips** zeigt die
vorhandenen Clips; **Delete Selected Clip** löscht einen, **Open Folder**
öffnet den Ordner. Neue Clips heißen seit Session 84
`camera_clip_<Datum>_<Uhrzeit>.mp4`; ältere `testaufnahme_…`-Clips bleiben
gültig und stehen in derselben Liste.

---

## 14. Stilfilter (Pixel Filter)

Seit Session 70 gibt es den Ketten-Knoten **„Pixel Filter (GLSL)"**
(Palette, Rubrik „GPU Modules"): ein frei skriptbarer Stilfilter, der je
Pixel über das Ketten-Bild läuft — und damit auf **jede** Quelle wirkt:
Video, Kamera, Scopes, MilkDrop, Shadertoy.

- **Eigene Filter schreiben:** eine GLSL-Funktion
  `vec4 farbe(vec2 uv, vec4 src)` liefert die neue Farbe (`src` =
  Quellpixel; Nachbarn über `texture(uTex, …)` — so entstehen Kantenzüge).
  Der Audio-Satz (`bass`, `mid`, `treb`, `vol`, `beat`) steht als Uniforms
  bereit. Groß-Editor, Apply und ⓘ-Referenz wie bei Shadertoy.
  (*Warum „farbe"? `filter` ist in GLSL ein reserviertes Wort.*)
- **Werks-Looks:** 12 Voreinstellungen liegen bei — **Take On Me Comic**
  (der a-ha-Rotoskopie-Look: Tusche-Konturen, Papier, beat-zitternde
  Schraffur), *Pencil Sketch*, *Posterize Pop Art*, *Newsprint Halftone*,
  *CRT Monitor*, *VHS Tape*, *Kuwahara Oil Painting*, *Sepia Nostalgia*,
  *Noir Black and White*, *Thermal Image*, *Pixel Art*, *Duotone Neon*.
  Laden über die Zeile „Preset"; eigene Filter
  speichert **„Save as…"** in dieselbe Sammlung.
- **Filter stapeln:** mehrere Filter = mehrere Knoten hintereinander —
  umsortieren, ein-/ausschalten und gruppieren wie jeden anderen Knoten.
  Ein Knoten ist genau EIN Render-Pass; Looks mit echtem Multipass
  (Blur-Pyramiden) baut man im Shadertoy-Knoten mit Buffer A–D.
- **Regler Mix:** blendet zwischen Original und Filter (per Skript
  `mixamount` auch audio-gesteuert — z. B. Comic nur auf dem Beat).
- **Klassiker-Kombi:** Video-/Kamera-Quelle (§13) + *Take On Me Comic* =
  das Musikvideo-Gefühl live; Demo: `asset/examples/pixelFilter -
  Take On Me.lvfx`.

### Filter als Datei laden und weitergeben

Der Groß-Editor (⤢) hat an allen Shader-Feldern **„Import…"** und
**„Export…"** — praktisch, um Filter zu sammeln, zu sichern oder zu teilen.

- **Export** schlägt einen sprechenden Namen vor:
  `<Preset>[.<Feld>].<Vertrag>.glsl`, zum Beispiel
  `MeinLook.pixelfilter.glsl` oder `Tunnel.image.shadertoy.glsl`. Existiert
  die Datei schon, zählt der Vorschlag automatisch hoch
  (`MeinLook(2).pixelfilter.glsl`). MilkDrop-Felder werden als `.hlsl`
  gespeichert — sie sind HLSL, keine GLSL.
- **Import** ersetzt nur den Editor-Text; übernommen wird er erst mit
  **Apply** oder **OK**. Jedes Feld merkt sich seinen **eigenen** Ordner.
- **Warnung bei falschem Vertrag:** Die Shader-Sorten sind nicht
  austauschbar — ein Stilfilter erwartet `farbe(uv, src)`, ein Shadertoy
  `mainImage(...)`, ein Mesh-Warp `warp(uv)`. Lädst du versehentlich das
  Falsche, sagt LumiViz das im Klartext (Dialog **„Different Shader
  Contract"**) und nennt den zuständigen Knoten, statt dich in einen
  unverständlichen Shader-Fehler laufen zu lassen. Laden kannst du trotzdem
  (**Load Anyway**) — etwa wenn du nur einzelne Hilfsfunktionen übernehmen
  willst.
- **Fremde Shader:** Wer Filter aus dem Netz übernimmt (Shadertoy, ISF &
  Co.), sollte die **Lizenz des jeweiligen Shaders** beachten. Jeder Import
  schreibt Titel, Autor, Quelle und Lizenz in den Knoten; oben im Knoten
  stehen sie sichtbar. Beim **Export** wandert dieselbe Angabe als
  Kommentarblock an den Anfang der Datei — die Herkunft geht also beim
  Weitergeben nicht mehr verloren.

### Vorlagen im großen Editor

Oben im Groß-Editor (⤢) jedes Shader-Feldes steht **Template:**. Die Auswahl
**ersetzt** den Editor-Inhalt — **Strg+Z** holt den alten Text zurück;
übernommen wird wie immer erst mit **Apply** oder **OK**.

- **Empty** gibt es immer: nur der Funktionsrumpf, den das Feld erwartet
  (`mainImage`, `warp`, `farbe`, `kraft`, ISF-`main`, MilkDrop-`shader_body`).
- Dazu je Feld zwei bis drei einfache Vorlagen — etwa *Color Gradient* und
  *Rings* (Shadertoy), *Swirl* und *Bass Zoom* (Mesh Warp), *Grayscale* und
  *Edges* (Pixel Filter), *Slow Fade* (MilkDrop Warp), *Bass Brightness*
  (MilkDrop Comp).
- Die Zeile zeigt, wovon der Text stammt, wie die Preset-Zeile im Panel:
  den **Namen der Vorlage**, solange du nichts änderst; **(unsaved) – Name**,
  sobald du danach etwas änderst; **(none)**, wenn der Text von keiner
  Vorlage kommt. Gleicht der Text beim Öffnen einer Vorlage, steht ihr Name
  gleich da.
- **Save as template…** legt den Editor-Inhalt als eigene Vorlage ab
  (in der Liste mit `*`); die Zeile zeigt danach diese Vorlage.
- Die ISF-Vorlagen sind **Generatoren ohne Bildquelle**: Bildquellen und
  Regler eines ISF-Shaders kommen über „Import ISF file…" in den
  Knoten, eine Vorlage ersetzt nur den Code.

Mitgelieferte Vorlagen liegen unter `asset/shadertemplates/`, eigene im
Benutzerdaten-Ordner unter `shadertemplates/`.

### ISF-Filter laden (fertige Filter aus dem Netz)

**ISF** („Interactive Shader Format", *isf.video*) ist so etwas wie
Shadertoy für Filter: GLSL-Dateien mit der Endung **`.fs`**, die ihre
Regler gleich mitbringen. Die Kategorie **FX** passt genau auf den
Stilfilter — LumiViz übersetzt sie beim Import automatisch.

So geht's: im Stilfilter-Knoten den Groß-Editor (**⤢**) öffnen →
**Import…** → eine `.fs`-Datei wählen. Danach steht der übersetzte Filter
im Editor, oben im Knoten die **Herkunft**, und darunter erscheinen die
**Regler des Filters** als aufklappbarer Baum — mit dem richtigen
Bedienelement je Sorte: Kästchen, Zähler, Klartext-Auswahl, Farbwähler,
XY-Feld. Ein Regler wirkt sofort.

Zwei Dinge, die LumiViz dabei sagt statt sie zu verschlucken:

- **Nicht jede `.fs` ist ein Filter.** *Generatoren* (erzeugen ein Bild aus
  dem Nichts) und *Übergänge* (blenden zwischen zwei Bildern) haben kein
  Quellbild und werden mit einer Klartext-Meldung abgelehnt. Ebenso
  Dateien, die **Geometrie** verformen — dafür ist der Mesh-Warp-Knoten da.
- **Manche Filter brauchen eine zweite Datei.** Kanten-, Blur- und
  Schärfefilter legen ihre Nachbar-Koordinaten in einer gleichnamigen
  **`.vs`** ab. Liegt sie neben der `.fs`, wird sie automatisch
  mitgenommen; fehlt sie, sagt LumiViz welche Datei es braucht.

Was der Filter nicht übernehmen kann (zusätzliche Bilder, Multipass,
Audio-Texturen), steht nach dem Import als Hinweisliste im Dialog — der
Filter läuft trotzdem.

---

## Changelog

| Version | Datum | Änderungen |
|---|---|---|
| 1.55.0 | 2026-10-02 | Session 90. §11 Lyrics (und Karaoke-Ebene): NEU Felder für die Vorschau-Zeilen — **Preview size (%)**, **Preview weight**, **Preview X shift / Y shift (%)** — und **Line spacing (%)** |
| 1.54.0 | 2026-10-02 | Session 90. §11 Lyrics Trail: der Trail zeigt **das Bild, das stand** (Wort-Stand, verkleinerte Zeile, Lage) statt einer neu gezeichneten Zeile; mit *Own color* und ohne laufenden Lyrics-Knoten zeichnet er wie bisher neu; nach einer Pause-Zeile kein zweiter Trail |
| 1.53.0 | 2026-10-02 | Session 89. §11 Lyrics (und Karaoke-Ebene): NEU Skriptfeld **Line** — läuft einmal je Zeilenwechsel, liest `line` und `linecount`; Lage, Größe und Farben folgen dem Skript von dort |
| 1.52.0 | 2026-10-02 | Session 89. §11 Host Switcher: NEU **Keep group state** (Gruppen merken sich ihren Stand und laufen beim Wiederbetreten weiter), **Reset buffers** und Skriptvariable `clearbuffers` |
| 1.51.0 | 2026-10-02 | Session 89. §9 NEU **Die Preset-Zeile** in den Panels Effect Chain und Karaoke (laufendes Preset, Favoriten ★, zehn zuletzt benutzte, lädt sofort, Load…/Save/Save As…); Karaoke-Presets an frei wählbarem Ort, **Delete** entfällt, Rückfrage bei ungespeicherter Karaoke-Ebene |
| 1.50.0 | 2026-10-02 | Session 89. §9 NEU **Save** (Ctrl+S) und **Save As…** (Ctrl+Shift+S) für die Effektkette; **Rückfrage bei ungespeicherten Änderungen** vor jedem Ersetzen der Kette und beim Beenden (im Vollbild: erst Vollbild verlassen) |
| 1.49.0 | 2026-10-02 | Session 89. Karaoke: Menü **View → Karaoke**, Tastenkürzel **Ctrl+K** (*Karaoke On / Off*), Zustandszeile *Preset shows its own lyrics* im Karaoke-Panel |
| 1.48.0 | 2026-10-02 | Session 89. §11 Lyrics: Karaoke-Presets (`.lvkar`) im Effect Browser — eigenes Icon, Doppelklick lädt in die Karaoke-Ebene |
| 1.47.0 | 2026-10-02 | Session 89. §11 Lyrics: Karaoke-Panel, NEU Reiter **Lyrics Trail** — *Trail on*, Liste der Trail-Module, Palette nur mit umformenden und färbenden Modulen, fester Lyrics Trail am Ende |
| 1.46.0 | 2026-10-02 | Session 89. §11 Lyrics / Lyrics Trail: NEU Feld **Blend strength (%)**; Mischart und Stärke der Karaoke-Ebene stehen im Karaoke-Panel (Reiter *Lyrics*) statt unter Settings → Audio, alte Werte werden übernommen |
| 1.45.0 | 2026-10-02 | Session 89. §11 Lyrics: NEU Panel **Karaoke** — Schalter *Karaoke on* (bisher Settings → Audio), Reiter *Lyrics* mit dem Aussehen der Zeile, **Karaoke-Presets** (`.lvkar`: Save…, Load, Delete, Default) |
| 1.44.0 | 2026-10-02 | Session 89. §11 Lyrics: die Zeile der Karaoke-Ebene bleibt mit Render Scale scharf (Fensterauflösung) |
| 1.43.0 | 2026-10-02 | Session 88. §2 NEU **Resume playback** (Player-Panel und Settings → Audio): nach dem Neustart dort weiterspielen. §11 Lyrics / Lyrics Trail: Feld **Blend** mit neun Mischarten (neu Subtract, Invert, XOR, Multiply, Maximum, Minimum) |
| 1.42.0 | 2026-10-02 | Session 88. §11 Karaoke-Ebene: NEU **Mischarten** je Zeile und Trail (Normal, Additive, 50/50, Subtract, Invert, XOR, Multiply, Maximum, Minimum) mit Stärke; die Kontur bleibt normal |
| 1.41.0 | 2026-10-02 | Session 88. §11 Lyrics: NEU **Karaoke-Ebene** — Songtext über jedem Preset (Settings → Audio → Karaoke layer); ein Lyrics-Knoten im Preset gewinnt |
| 1.40.0 | 2026-10-02 | Session 88. §11 Host-Gruppen: `.lvfx2` trägt im Effect Browser ein eigenes Icon (Stapel) |
| 1.39.0 | 2026-10-02 | Session 88. §11 Lyrics Trail: folgt den **skriptgesteuerten** Werten des Lyrics-Knotens (Karaoke Bounce, Flip) und arbeitet auch mit **unsynchronem Text** (Wechsel alle N Beats). §11 Lyrics-Editor: Knöpfe in zwei Reihen, **Offset for this track** als eigene Zeile |
| 1.38.0 | 2026-10-02 | Session 88. §11 Lyrics Sync: NEU **Undo** / **Redo** (Ctrl+Z / Ctrl+Y) für jeden Schritt an den Zeiten |
| 1.37.0 | 2026-10-02 | Session 88. §11 Lyrics: NEU Knopf **Lyrics file…** — eine vorhandene `.lrc`/`.txt` dem laufenden Titel zuweisen (wird in den Benutzerordner kopiert) |
| 1.36.0 | 2026-10-02 | Session 88. §11 Lyrics: NEU **Versatz je Titel** — Feld **Offset for this track** im Editor des Lyrics-Knotens und **Track offset** im Panel Lyrics Sync; je Titel gemerkt (`offsets.json`), ohne die Songtext-Datei anzufassen |
| 1.35.0 | 2026-10-02 | Session 88. §2 Tag View zeigt den **Songtext** des laufenden Titels (Quelle, synchron oder nicht, Zeilenzahl, Text) |
| 1.34.0 | 2026-10-02 | Session 88. §11 Host Switcher: die sechs **Voreinstellungen** (Random Runtime, Random Runtime per Group, Random Seconds, Every 16 Beats, Bass Drop, One per Song) mit ihren Stellgrößen. §11 Lyrics: Titel ohne Text beim Dienst werden **gemerkt** und 7 Tage (instrumental 30) nicht wieder gefragt; **Settings → Audio → Lyrics not found → Ask again** |
| 1.33.0 | 2026-10-01 | Session 87. §11 Lyrics: NEU Knoten **Lyrics Trail** (die eben beendete Zeile noch einmal zeichnen, damit Effekte sie verwehen lassen; Muster mit Effect List, Blitter Feedback und Fadeout) |
| 1.32.0 | 2026-10-01 | Session 87. §11 Lyrics: NEU Panel **Lyrics Sync** (Zeiten je Zeile mitklopfen, nachstellen, vorhören, als `.lrc` speichern) |
| 1.31.0 | 2026-10-01 | Session 87. §11 Lyrics: Angaben im Editor als **Tabelle** mit Status (*…downloading…*); Online-Abruf **wiederholt sich von selbst**, wenn der Dienst beschäftigt ist; zehn neue Voreinstellungen, darunter bewegte Karaoke-Varianten; **Alternatives…** (andere Fassung suchen, eigene Suchbegriffe), **Wrong lyrics** mit Sperrliste (Settings → Audio); wiederholte Zeilen blenden vor dem Einsatz; Fassungen eines Songs getrennt nach Dauer. Playlist: Dauer je Titel; Shuffle, Loop und Repeat bleiben über den Neustart erhalten |
| 1.30.0 | 2026-10-01 | Session 86. §11 Lyrics: NEU **Songtexte aus dem Netz** (Opt-in „Search lyrics online", Dienst lrclib.net, was gesendet wird; „Save fetched lyrics": User folder / Next to the song) |
| 1.29.0 | 2026-10-01 | Session 86. §11 NEU Knoten **Lyrics**: Songtext zum laufenden Titel (eingebettet, `.lrc`/`.txt` neben dem Song oder im Benutzerordner; Suchreihenfolge), synchron an der Abspielposition mit Vorschau-Zeilen und Wort-für-Wort-Füllung, Duett-Farben, unsynchroner Block, drei Voreinstellungen |
| 1.28.0 | 2026-10-01 | Session 86. §2 NEU Panel **Tag View**: Tags, Datei, Dauer und Cover des laufenden Titels (nur Anzeige) |
| 1.27.0 | 2026-10-01 | Session 86. §11 NEU „Text: Titel und Interpret anzeigen": Platzhalter `{title}` `{artist}` `{album}` `{year}` `{genre}` `{track}` `{file}`, wahlfreie Teile in eckigen Klammern, Knopf **Insert tag**, Voreinstellung **Now Playing** |
| 1.26.0 | 2026-10-01 | Session 86. §11 Text: **Font** ist eine Auswahlliste der installierten Schriften (weiter editierbar), NEU Häkchen **Bold**. §11 NEU „Fenstergröße ändern": das Bild von Kette, Effect List und Host-Gruppe bleibt beim Größenwechsel erhalten — Blur und Trails brechen nicht mehr ab |
| 1.25.0 | 2026-10-01 | Session 86. §11 Modulation: NEU **Beat sync** — LFOs im Takt der Musik (Länge einer Schwingung in Beats). §11 Voreinstellungen: 7 für **Text** (Beat Words, Slideshow, Random Flash, Subtitle, Beat Colours, Bass Tempo, Beat Jump) und 5 für **AVI** |
| 1.24.0 | 2026-10-01 | Session 86. §11 Modulation: **Farbfelder** modulierbar (Target Brightness / Hue / Red / Green / Blue, Live-Wert als `#RRGGBB`). §11 Effekt-Palette: **Text**, **AVI** und **Comment** stehen jetzt in der Palette |
| 1.23.0 | 2026-10-01 | Session 86. §11 „Werte per Formel steuern": **Text** und **AVI** haben jetzt Init/Frame/Beat-Felder; bei Text folgen Schrift, Farbe, Lage und Kontur dem Skript nur aus Init, Song und Beat |
| 1.22.0 | 2026-10-01 | Session 86. §11 „Werte per Formel steuern": auch **Dynamic Movement** (`wrap`, `nomove`, `subpixel`, `rectcoords`, `buffern`), **Host-Gruppe** (`blendout`, `curvein`, `curveout`, `crossfadeseconds`) und **Host Switcher** (`intervalseconds`, `order`, `nextonsongchange`); Host Switcher → Switch after (s) modulierbar. Liste der Knoten ohne Formelfelder |
| 1.21.0 | 2026-10-01 | Session 86. §11 „Werte per Formel steuern": die Felder von **SuperScope** (`linewidth`, `dotsize`, `rendermode` …) und **Effect List** (`blendout`, `bufferin`, `outadjustalpha` …) sind jetzt Skriptvariablen; AVS-Namen (`n`, `linesize`, `drawmode`, `alphain`, `alphaout`) behalten Vorrang. Behoben: Auswahlfelder mit im Panel gebauter Liste (z. B. Clear → Blend) sprangen auf den ersten Eintrag, sobald der Knoten ein Skript trug |
| 1.20.0 | 2026-10-01 | Session 86. §11 Modulation: jetzt auch **SuperScope** (Point count, Line width, Dot size, Cycle frames) und **Effect List** (In/Out Alpha, In/Out Buffer, OnBeat frames), Host-Gruppe, Dynamic Movement, Text, AVI, Debug Bars; Point count setzt `n` neu. Effect List: `alphain`/`alphaout` stehen im Listen-Code zu Beginn jedes Bilds auf dem Reglerwert (wie in AVS) — der Alpha-Regler wirkt jetzt auch bei laufendem Listen-Code sofort |
| 1.19.0 | 2026-09-30 | Session 85. §11 NEU **Werte ohne Formel bewegen (Modulation)**: „~"-Knopf an Zahlfeldern (Audio, Beat-Hüllkurve, LFOs), Live-Wert, Schalter **Modulation active**. §11 „Werte per Formel steuern": jedes Feld skriptbar, auch bei Fraktalen/3D/Glow Orbs/Texer II; eine Reglerbewegung setzt das Skript nicht mehr zurück. Die ⓘ-Seite jedes Skriptfelds beginnt mit seinem Takt (einmal/je Frame/je Beat/je Punkt …) |
| 1.18.1 | 2026-09-30 | Session 84. Vorlagen-Zeile im Groß-Editor zeigt den Zustand (Name / „(unsaved) – Name" / „(none)") statt zurückzuspringen; §11 Presets aus dem Effect Browser/Explorer in die Kette ziehen = Presetverweis; §2/§3 Ziehen zum Abspielen (Player, Visualisierung), Playlist nimmt Dateien an und sortiert im Dienst um, NEU **Audio Browser** (Albumcover als Icon, Fallback Ordnerbild/Formaticon); Feld-Tooltips englisch; §13 Kamera-Clips heißen `camera_clip_…` |
| 1.18.0 | 2026-09-30 | Session 83. Zitierte Oberflächen-Beschriftungen auf die englische Oberfläche gezogen (§8 Einstellungen/Kamera, §9 Tastenkürzel, §11 Milkdrop/Host-Gruppen/Host Switcher, §12–§14); „Testaufnahme" heißt **Camera Clip** |
| 1.17.0 | 2026-09-30 | Session 83. Namen **aller** mitgelieferten Presets, Beispiele und Vorlagen englisch (§11, §14); NEU Presets für die Milkdrop-Sektionen **Code** und **Parameter** (u. a. *Original*, *Old LumiViz Look (pre-S80)*). §11 Voreinstellungen: Combo behält den geladenen Namen, **„(unsaved) – Name"** nach einer Änderung, **(none)** ohne Voreinstellung, 🗑 zielt auf die Quelle; NEU Voreinstellungen je **Milkdrop-Element** (Wave/Shape/Sprite); Template-Listen von SuperScope 3D/3D Camera sind jetzt Voreinstellungen. §11 NEU Schalter **Script active**. §14 NEU **Vorlagen im großen Editor** (Leer + einfache Vorlagen je Shader-Feld, eigene speichern) |
| 1.16.0 | 2026-09-29 | Session 82. §11 Host-Gruppen: NEU **Presetverweis** („Preset Reference…", nur Pfad gespeichert, Inhalt beim Laden frisch; Schalter Kopie ↔ Verweis; vom Host Switcher schaltbar) |
| 1.15.0 | 2026-09-29 | Session 82. §11 „Werte per Formel steuern": NEU **Song-Feld** in jedem Knoten mit Frame-Feld (einmal je Songwechsel, vor Frame) |
| 1.14.0 | 2026-09-29 | Session 82. §11 NEU **Host Switcher** (Gruppenliste mit Index, Startgruppe, Wechsel nach Zeit/Songwechsel/zufällig, Skripte Init/Frame/Beat/Song mit `switch`/`nextgroup`, schaltet nur zur Laufzeit); Gruppennamen eindeutig mit Zähler, Umbenennen zieht Switcher-Listen mit. Eingeklappte Gruppen bleiben beim Wechsel zu |
| 1.13.0 | 2026-09-29 | Session 82. §11 Host-Gruppen: **„Preset as Group…"** in der Effekt-Palette (`.avs`/`.milk`/`.lvfx` als Kopie in eine neue Gruppe, Quelle + „Aus Quelle neu laden", Grenzen), Gruppen-Knopf lädt jetzt alle drei Formate. „Import Browser" heißt **Effect Browser** (§8, §9, §11) |
| 1.11.0 | 2026-09-28 | Session 80. §11 NEU „Gestaltung: Drift und Weichzeichnen" — Linear-Drift (Stärke + Richtung), Radial-Drift (Stärke, Winkel außen/Wirbel/Sog, Zentrum) und Weichzeichnen je Milkdrop-Node, je Werkzeug ein Schalter, Winkel im Panel in Grad; per `per_frame` steuerbar (`drift_lin/dir/rad/ang/cx/cy`, `weich`, Bogenmaß); Knöpfe „Original" und „Weicher Sog ↖" (= der Look bis S80) |
| 1.10.0 | 2026-08-07 | Session 72 (Filter-Strang). **§12: Tabelle „Was an einem `iChannel` hängen kann"** — die beiden neuen Quellen **Ketten-Eingang** (macht Shadertoy-Bildfilter direkt lauffähig) und **AVS-Buffer 1–8** (Buffer-Save-Slots als Textur, Stand beim Lauf des Knotens, Host-Gruppen-Trennung) je mit Anwendungsbeispiel. **§14 NEU „ISF-Filter laden"** — Import von `.fs`-Dateien, Parameter-Baum mit typrichtigen Bedienelementen, was abgelehnt wird (Generator/Übergang/Geometrie) und warum manche Filter eine `.vs` brauchen. §14 Lizenz-Absatz: Herkunft wandert jetzt auch in die **exportierte Datei** |
| 1.9.0 | 2026-08-07 | NEU §14 „Filter als Datei laden und weitergeben" (S71): Import…/Export… an allen Shader-Feldern erklärt (Namensschema `<Preset>[.<Feld>].<Vertrag>.glsl`, automatisch hochzählender Vorschlag, eigener Ordner je Feld, `.hlsl` für MilkDrop), Warnung bei falschem Shader-Vertrag, Lizenz-Hinweis für fremde Shader. Schließt eine Doku-Lücke seit S69 |
| 1.8.0 | 2026-08-06 | NEU §14 „Stilfilter (Pixel Filter)" (S70): skriptbarer Pixel-Filter (farbe()-Vertrag, Mix, Stapeln als Knoten) + 12 Werks-Looks inkl. Take-On-Me-Comic |
| 1.7.0 | 2026-08-06 | NEU §13 „Video & Kamera als Quelle" (S70): videoSource-Knoten (Datei/Kamera/Testaufnahme, Streaming vs. Frame-Schritt, Einpassung, Blend/Deckkraft, Parameter-Skripte) + Settings-Tab „Kamera" mit Testaufnahmen |
| 1.6.0 | 2026-08-04 | §11: neue Einstellung „MilkDrop Start Fade-in" (Sicht-Blende, S67) — halbe Sekunde Schwarz-Einblendung über der Rausch-Saat bei App-Start/Größenwechsel/Löschen; Vorgabe an |
| 1.5.1 | 2026-08-03 | §11: fünfte Option „Ausblenden (über Zeit)" (Erbe stirbt über die Dauer weg), Fading als „einmaliger Mix" präzisiert, Hinweis „Löschen ≠ Schwarz" (Rausch-Saat) |
| 1.5.0 | 2026-08-03 | §11: „Preset-Wechsel: das geerbte Bild" (S66-Schalter Behalten/Löschen/Fading + exakte Liste, was Behalten erhält) und „Regelwerk" (S65). +§12 Shadertoy (S65). §8: Graphics-Card-Einstellung (S61) + MilkDrop-Preset-Switch-Standard |
| 1.4.0 | 2026-08-01 | §11: Grundkatalog auf 125 Vorlagen für 39 Knotentypen erweitert (Session 61 — Fraktal-Familie, Bloom, blur/simpleScope/customBpm, sechs geerntete Dynamic-Movement-Warps mit Quellenangabe) |
| 1.3.0 | 2026-08-01 | §11: Grundkatalog der mitgelieferten Voreinstellungen (Session 60 — 84 Vorlagen für 26 Knotentypen, Katalogverweis auf `asset/nodepresets/README.md`) |
| 1.2.0 | 2026-07-27 | §11 erweitert (Session 53): Voreinstellungen je Knoten (mit Feldauswahl beim Speichern, SuperScope-Figuren als Presets), Werte per Formel (Init/Frame/Beat, Variablen-Regeln, Movement-Ausnahme), Bild-Auswahl bei Picture/Texer, *New Effect Chain*. §8 auf vier Reiter berichtigt (waren zwei) + Bilder-Suchordner und Benutzerdaten-Ordner. §9 um `Ctrl+Shift+N` |
| 1.1.0 | 2026-07-23 | +§11 (Session 42): Effektketten-Editor, Milkdrop-Node mit sechs Sektionen (inkl. Wave/Shape/Sprite anlegen, Parameter-Sektion, sektions-genaue ⓘ-Referenzen, Shader-ⓘ), Host-Gruppen mit Crossfade (Kurven, Wechsel-Button, .lvfx2) |
| 1.0.0 | 2026-07-20 | Initial (Session 31): Player, Playlist inkl. Session-Playlist, Visualizer-Auswahl, Config-Verweis, echtes Vollbild, Docking/Perspektiven, Frame-Modus, Tastenkürzel |

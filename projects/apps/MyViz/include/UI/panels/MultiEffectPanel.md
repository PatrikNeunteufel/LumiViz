# MultiEffectPanel

> **Panel:** `multieffect_chain` / „Effect Chain" (dockbar, Default versteckt) ·
> **Seit:** Import-Phase Roadmap 5.7b (Session 34) ·
> **Steuerdokument:** `docs/visuals/Import_Multieffekt_Host_Entwurf.md` (E5)

Baum-Editor für die Effektkette des [MultiEffectVisualizer](../../visualizers/MultiEffectVisualizer.md).
Öffnen über **View → Panels → Effect Chain**; aktiv nur, wenn der Visualizer
„Multi Effect" ausgewählt ist (folgt dem aktiven Visualizer per
`VisualizerChangedEvent`, sonst Hinweistext).

## Funktion

- **Baumansicht** der Kette (verschachtelte Listen + Effekte). **Spalte 0 = Name**
  (die Baum-Spalte: trägt Expand-Pfeil + Ebenen-Einrückung, damit Nesting über
  alle Ebenen sichtbar ist). **Spalte 1 = Auge-Toggle** (👁/—, schmale Fixspalte)
  zum Ausblenden — setzt `enabled` (bei einer Gruppe die ganze Gruppe; der
  Renderer gated in `renderNode`).
- **Toolbar:** Typ-Auswahl + **Add** (in die selektierte Liste, sonst Root),
  **Remove**, **Clone** (⧉ — dupliziert Knoten inkl. Teilbaum direkt dahinter;
  frische `nodeId`s), **Up/Down** (innerhalb des Elternknotens).
- **Drag & Drop:** Effekte/Gruppen im Baum verschieben — auf eine Gruppe fallen
  lassen = **hineinschieben**, zwischen Knoten = umsortieren, auf Leerfläche =
  ans Root-Ende (**herausschieben**). Der Tree (`ChainTreeWidget`) mutiert nie
  selbst: `dropEvent` meldet nur Quelle/Ziel/Position, das Modell wird bewegt
  (queued, außerhalb des Drop-Events) und der Baum daraus neu gebaut.
- **Parameter-Editor** je selektiertem Knoten: Spinner/Checkbox/Farbwähler/
  Enum + **EEL-Skript-Textfelder** (Init/Frame/Beat/Point bzw. Level) für alle
  Effekt-Typen (inkl. Mosaic); Passthrough-Knoten zeigen nur ihre Konserven-Notiz.
- **Voreinstellungs-Zeile „Preset" (Session 53, Etappen 1 + 1b):** ganz oben im
  Editor, **für jeden Knotentyp** — Dropdown (mitgeliefert + eigene, letztere mit
  `*`), „Save as…", Löschen (nur eigene). Sie kennt keinen einzigen Typ beim Namen:
  sie läuft über `effectTypeKey()` und `nodeToJson`/`nodeFromJson`
  ([`NodePresetStore`](../../visualizers/multieffect/NodePresetStore.hpp)), gilt also
  automatisch auch für künftige Knoten. Laden ist **eine** `mutate`-Operation
  (renderMutex + `recompileChain`, undo-fähig) und baut den Editor danach neu auf.
  Nicht sichtbar bei Milkdrop-Sektionen/-Elementen — das sind Navigations-Items ohne
  eigene `params`. Konzept:
  [Knoten_Parameter_Konzept.md](../../../docs/visuals/Knoten_Parameter_Konzept.md) §3.
  - **Merge statt Ersatz:** eine Datei überschreibt genau die Felder, die sie
    enthält; alles andere bleibt stehen. Damit ist ein **Teil-Preset** möglich.
  - **Feldauswahl beim Speichern** (Vorgabe Patrik): der Dialog listet jedes
    Parameterfeld des Knotens mit Häkchen (Liste generisch aus `nodeToJson`, also
    für jeden Typ). Abgewählt = nicht in der Datei = beim Laden unangetastet —
    so speichert man „nur die Formeln, nicht die Farbe". `type` bleibt immer
    drin, daran hängt der Typwächter.
  - **Das frühere „Figure"-Dropdown ist entfallen.** Die SuperScope-Figuren
    liegen als Teil-Presets in `asset/nodepresets/superScope/` (13 Dateien, nur
    die vier EEL-Slots + `pointCount`) und laufen über dieselbe Zeile — eine
    Voreinstellungs-Liste statt zweier nebeneinander. Eine Figur lässt damit
    Farbe, Linienbreite und Blend des Knotens stehen, genau wie vorher.
- **Parameter-Skripte (Strang D, Session 53):** jeder Knoten mit numerischen
  Parametern hat unten drei EEL-Felder (Init/Frame/Beat), die seine Regler je
  Frame ausrechnen — 48 Renderer insgesamt. Die schreibbaren Namen sind die
  Feldnamen in Kleinschreibung, dazu `b`/`w`/`h` und der Audio-Satz; sie stehen im
  Doxygen-Kommentar des jeweiligen `…Params`-Structs. Ein leeres Feld kostet
  nichts. Die drei **Klasse-A**-Knoten (Dot Plane · Bass Spin · Moving Particle)
  tragen darüber eine Hinweiszeile: dort sind die Renderer-Konstanten die
  Referenz, ein Skript verlässt sie — und die ⚠ an den Reglern kann das nicht
  sehen, weil sie nur feste Werte vergleicht.
- **Referenz-Regler (`addRefDouble`, Session 53):** für die drei **Klasse-A**-Knoten
  (`Dot Plane`, `Bass Spin`, `Moving Particle`), deren Konstanten die AVS-Referenz
  *sind*. Die Vorgabe ist der Originalwert; weicht der Wert ab, hängt ein **⚠** an
  der Beschriftung und der Tooltip nennt den Referenzwert. So bleibt sichtbar,
  warum ein Preset in der Kalibrierung ausschert (Entscheid Patrik,
  Knoten_Parameter_Konzept §8.4).
- **Bild-Zeile (`addImageRow`, Session 53 — S50-Vorgabe):** Picture · Picture II ·
  Texer · Texer II zeigten bisher nur, **ob** das Bild eingebettet ist. Jetzt
  „Choose…" (Datei wird base64 eingebettet, die Kette bleibt damit
  selbsttragend) und „Clear". Startordner ist der **Bilder-Suchordner** aus den
  Einstellungen (`import/imageSearchDir`) — denselben Schlüssel liest der Import
  als letzte Zuflucht, wenn neben dem Preset nichts liegt.
- **Kernel-Gitter (`addKernelGrid`) und Gradient-Stopps (`addGradientStops`),
  Session 53:** `Convolution` bekommt sein 7×7-Gitter (Mitte hervorgehoben,
  Knopf „Identity"), `ColorMap` seine Stützstellen (Position 0–255 + Farbe,
  `+`/`−`). Beide standen vorher als „imported, read-only" da. Die Reihenfolge
  der Stopps ist egal — `buildColorMapLut` sortiert selbst.
- **Farbtafel-Zeile (`addColorTable`, Session 53):** ein Farbfeld je Palette-
  Eintrag plus „+"/„−"; der Zugriff auf den `colors`-Vektor kommt als Funktion
  herein, damit sich alle Knoten mit Palette dieselbe Zeile teilen (SuperScope,
  **Metaballs 3D**, **Tentacles 3D**). Längenänderung baut den Editor neu
  (verzögert, nicht aus dem Signal heraus).
- **Metaballs 3D / Tentacles 3D (Session 53):** Editoren für die beiden
  APE-Verhaltens-Nachbauten aus S52 — Metaballs (Kugelzahl, Radius, Tempo,
  Isowert, Blend), Tentacles (Zahl, Segmente, Länge, Dicke, Tempo, Blend), je
  mit Farbtafel; beide auch in der Palette („Scopes & Sources", neben FyrewurX).
  Grenzen wie im Renderer/`ChainSerializer` (count 1..16, segments 2..256).
- **SuperScope-Farbe (Hybrid):** `Color mode` (Gradient · Table · Additive ·
  Multiply · Average) kombiniert zwei Quellen — den zeit-gezykelten **AVS-Farb­
  tabellen**-Wert (Swatch-Editor + Cycle-Frames) und den per-Punkt-**Gradient**
  (benannte Presets, mit Preview im Dropdown). Diese **Basisfarbe belegt das
  Modul vor jedem Point-Code in `red/green/blue` vor** (AVS r_sscope) — der Code
  kann sie behalten, modulieren (`red=red*v`) oder überschreiben. Default =
  Gradient/„Neon" (unverändertes Alt-Verhalten).

- **Milkdrop-Node (N2 Session 41, N3.1–N3.3 Session 42 — Entscheid E1):** Der
  Node zeigt sechs **Anzeige-Kinder** im Baum (Code · Waves · Shapes · Shader ·
  Sprites · **Parameter**); Waves/Shapes/Sprites haben darunter
  **Element-Items** (je Wave/Shape/Sprite eines). **Basis-Waveform (S43):**
  erstes, festes Element-Item unter „Waves" (Sentinel
  `kMilkBasisWaveElement = kMilkSectionBase − 1`) — die immer gerenderte
  Standard-Welle (nWaveMode/Alpha/Farbe/Position …) hat damit einen sichtbaren
  Editor-Einstieg (Sichttest-Befund: gerenderte Welle war bei „Waves (0
  aktiv)" nicht auffindbar); nicht lösch-/klonbar, Sektions-Label = „Waves
  (Basis + N Custom aktiv)". **Parameter-Sektion (N3.2):**
  die komplette numerische Preset-Fläche in Gruppen (General/Composite
  inkl. fShader, Motion/Warp, Borders, Motion Vectors,
  Blur-Pyramide) mit Startwerte-Hinweis (per_frame kann überschreiben) und
  Baked-Hinweis bei vorhandenem Comp-Shader; die Basis-Waveform-Gruppe ist
  seit S43 nur noch ein Wegweiser auf die Waves-Ansicht; die
  Wave-/Shape-Einzel-Ansichten
  tragen zusätzlich ihre numerischen Init-Parameter. Sektions- und Element-Items sind Navigations-Items
  mit Sentinel-Pfaden (`kMilkSectionBase`; Elemente = `[…, Sentinel, Index]`,
  Zerlegung über `splitMilkPath`), nicht drag-/editierbar — `nodeAtPath`
  liefert für sie bewusst `nullptr`, der Property-Editor trennt Sektion/
  Element ab und editiert den echten Node. **Add/Remove/Clone (N3.1):** Die
  Palette führt „Custom Wave / Custom Shape / Sprite" (Kategorie
  „Milkdrop-Node-Inhalte") — „+" legt das Element im PresetState des
  selektierten Milkdrop-Nodes an (Waves/Shapes Cap 16, kleinster freier
  `index`, `enabled=true`); „−"/⧉ wirken auf selektierte Element-Items.
  **Sprite-Editor (N3.3):** je Sprite alle Startwerte (Bild, Colorkey, Layer,
  Blend 0–4, Alpha, Burn, x/y, sx/sy, rot, Speed, Repeat) + per-Frame-EEL.
  Jede Mutation bumpt `MilkdropNodeParams.revision`
  (Render-Host-Übernahme-Vertrag); Shader-Edits reklassifizieren via
  `analyzeWarp/CompShader` (SSOT = Text). Palette-Eintrag „Milkdrop
  (Preset-Pipeline)" mit MilkDrop-Origin-Icon. **Sektions-/Element-Optik
  (S43):** alle Milk-Unterknoten tragen das MilkDrop-Icon in der Typ-Spalte;
  Wave-/Shape-Elemente haben zusätzlich den **Auge-Toggle** (Spalte 1) wie
  Chain-Zeilen — er setzt `enabled` (mutate + Revision-Bump) und zieht
  Element-Label („(aus)") + Sektions-Zähler in-place nach; die
  Editor-Checkbox wird beim selektierten Element mit-aktualisiert
  (umgekehrt — Checkbox → Auge — erst beim nächsten Baum-Rebuild:
  bekannte Rauheit).

- **Host-Gruppe (HG1, Session 42):** Container-Node wie eine Effect List, aber
  mit **eigenem Laufzeit-Bestand** (persistenter Feedback-Buffer, eigene
  Buffer-Save-Slots, eigener Skript-Kontext). Palette „Host Group";
  **Tiefenregel** — keine Gruppe in einer Gruppe: das Add-Dropdown und
  Drag&Drop verweigern es, der Compile-Pass degradiert verschachtelte Gruppen
  aus Dateien zur Effect List. Editor: Blend Out/Alpha, **Crossfade-Dauer**
  (Änderung synchronisiert ALLE Gruppen, Entwurf §2.4), individuelle Ein-/
  Ausgangskurven (linear; weitere mit HG2), `.lvfx`-Import in die Gruppe
  (children ersetzt, frische nodeIds, Quelle wird angezeigt). Speichern einer
  Kette mit Gruppe(n) erzeugt `.lvfx2`.

## Verträge

- **Alle Mutationen** (Struktur wie Parameter) laufen unter dem `renderMutex()`
  des Widgets + `recompileChain()` — der editierbare-Ketten-Vertrag aus E5.
  Neue/entfernte Knoten → der Render-Thread gibt verwaiste GL-Runtimes frei
  (Knoten-IDs vom Compile-Pass).
- **Navigation** über Index-Pfade (`QList<int>` im Item), nicht über rohe
  Zeiger — übersteht Baum-Rebuilds.

## Bekannte Rauheiten (Sichttest-Nachzug)

- Skript-Textfelder committen bei **jedem Tastendruck** (der Render-Thread
  transpiliert dann im nächsten Frame neu) — bei langen Skripten spürbar;
  Debounce/Commit-on-Focus-Out ist ein Feinschliff.
- Struktur-Änderungen bauen den Baum komplett neu; nach Drag & Drop wird der
  bewegte Knoten wieder selektiert, die Expansion sonst nicht wiederhergestellt.

## Absicherung

Qt-UI → kein Unit-Test; Datenmodell + Compile-Pass sind separat getestet
(`test_EffectChain`), Persistenz über `test_ChainSerializer`, die Voreinstellungen
über `test_NodePresetStore` (Roundtrip inkl. Formeln · Rahmen bleibt draußen ·
Typwächter · Benutzer-vor-Asset · Teil-Preset lässt den Rest stehen · **jede
mitgelieferte Datei muss ladbar sein** · **jede Figur der Modul-Bibliothek hat
ihre Datei mit demselben EEL** — das Modul ist Qt-frei und kann die Dateien nicht
selbst lesen, der Wächter hält beide Seiten zusammen).
Panel selbst: Sichttest (Import → editieren → speichern → laden).

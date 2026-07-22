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
- **SuperScope-Figur-Dropdown:** lädt eine Figur (Spiral, Butterfly, Hypocycloid,
  …) aus der `SuperscopeModule`-Preset-Bibliothek (SSOT) und trägt deren EEL in
  die Init/Frame/Beat/Point-Felder + Point-Count ein.
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
  **Element-Items** (je Wave/Shape/Sprite eines). **Parameter-Sektion (N3.2):**
  die komplette numerische Preset-Fläche in sechs Gruppen (General/Composite
  inkl. fShader, Basis-Waveform, Motion/Warp, Borders, Motion Vectors,
  Blur-Pyramide) mit Startwerte-Hinweis (per_frame kann überschreiben) und
  Baked-Hinweis bei vorhandenem Comp-Shader; die Wave-/Shape-Einzel-Ansichten
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
  (Preset-Pipeline)" mit MilkDrop-Origin-Icon.

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
(`test_EffectChain`), Persistenz über `test_ChainSerializer`. Panel selbst:
Sichttest (Import → editieren → speichern → laden).

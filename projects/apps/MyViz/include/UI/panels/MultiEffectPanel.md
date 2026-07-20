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

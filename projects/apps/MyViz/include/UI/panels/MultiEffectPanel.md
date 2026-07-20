# MultiEffectPanel

> **Panel:** `multieffect_chain` / „Effect Chain" (dockbar, Default versteckt) ·
> **Seit:** Import-Phase Roadmap 5.7b (Session 34) ·
> **Steuerdokument:** `docs/visuals/Import_Multieffekt_Host_Entwurf.md` (E5)

Baum-Editor für die Effektkette des [MultiEffectVisualizer](../../visualizers/MultiEffectVisualizer.md).
Öffnen über **View → Panels → Effect Chain**; aktiv nur, wenn der Visualizer
„Multi Effect" ausgewählt ist (folgt dem aktiven Visualizer per
`VisualizerChangedEvent`, sonst Hinweistext).

## Funktion

- **Baumansicht** der Kette (verschachtelte Listen + Effekte), je Knoten eine
  Enable-Checkbox.
- **Toolbar:** Typ-Auswahl + **Add** (in die selektierte Liste, sonst Root),
  **Remove**, **Up/Down** (innerhalb des Elternknotens).
- **Parameter-Editor** je selektiertem Knoten: Spinner/Checkbox/Farbwähler/
  Enum + **EEL-Skript-Textfelder** (Init/Frame/Beat/Point bzw. Level) für alle
  19 Effekt-Typen; Passthrough-Knoten zeigen nur ihre Konserven-Notiz.

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
- Struktur-Änderungen bauen den Baum komplett neu (Selektion/Expansion gehen
  verloren) — akzeptabel für die erste Fassung.

## Absicherung

Qt-UI → kein Unit-Test; Datenmodell + Compile-Pass sind separat getestet
(`test_EffectChain`), Persistenz über `test_ChainSerializer`. Panel selbst:
Sichttest (Import → editieren → speichern → laden).

# Host-Gruppen + Crossfade (E5) — Entwurf

> **Stand:** 2026-07-23 (Session 42) · **Status:** **freigegeben** (Patrik, 2026-07-23, v1.2.0) ·
> **Basis:** [MilkDrop_Import_Konzept.md](MilkDrop_Import_Konzept.md) §6b
> (Entscheide E5/E6) + Entscheide Patrik Session 42 (§2) ·
> **Fortschritts-SSOT:** [MilkDrop_Import_Status.md](MilkDrop_Import_Status.md)

## 1. Zielbild

Der MultiEffect-Host ist die **eine Effect-List der App**: Er provided alle
Effekt-Typen gleichberechtigt — LumiViz-eigene Module, AVS-Module und den
Milkdrop-Node (seit N1). Darauf setzt eine neue Container-Ebene auf:

- **Host-Gruppe** („Level-1-Host"): ein Node-Typ, der ein **komplettes
  Visual** kapselt — eine ganze AVS-Kette, ein ganzes Milkdrop-Preset oder
  ein LumiViz-Setup — mit **eigenem Laufzeitzustand** (Buffers, Feedback,
  Blur, Skript-Kontexte).
- **Crossfade (E5)** wird eine Funktion des Haupt-Hosts: Er blendet zwischen
  zwei Host-Gruppen per **echtem Doppel-Rendering** (beide leben, Bild-Mix).
- Die **Visual-Playlist (E6)** schaltet später Host-Gruppen einheitlich —
  egal, was drinsteckt.
- **Globale Ebene gratis:** Nodes, die direkt im Haupt-Host liegen (außerhalb
  jeder Host-Gruppe), laufen beim Crossfade unverändert weiter — z. B.
  Overlays oder globale Farb-/Post-Effekte.

Damit sind Milkdrop, AVS und Eigenes auf **zwei** Ebenen kombinierbar:
innerhalb einer Kette (Nodes mischen — geht heute schon) und zwischen
Visuals (Host-Gruppen-Wechsel mit Crossfade).

## 2. Entscheide (Patrik, 2026-07-22, Session 42)

1. **Host-Gruppe = neuer Node-Typ** (kein Umbau des bestehenden List-/
   Buffer-Knotens).
2. **Import:** Bestehende `.lvfx` (ohne Host-Gruppe) können in eine
   Host-Gruppe importiert werden — die Kette wird zu deren Inhalt.
3. **Persistenz:** Sobald ein Preset mindestens eine Host-Gruppe enthält,
   wird es im neuen Schwester-Format **`.lvfx2`** gespeichert; flache
   Presets bleiben `.lvfx`. Keine Migration des Bestands nötig — die
   Antwort auf „ist eine flache Kette implizit eine Host-Gruppe?" ist damit:
   nein, sie bleibt flach und ist bei Bedarf importierbar.
4. **Crossfade-Settings je Host-Modul, aber synchron** — mit einer Ausnahme
   (Präzisierung 2026-07-23): Die **Wechsel-Settings** (z. B. Dauer, Trigger)
   liegen am Host-Modul und werden bei Änderung auf **alle** Host-Module
   übertragen (UI zeigt einen **Hinweis**). Die **Eingangs- und
   Ausgangskurve** sind dagegen **je Host-Gruppe individuell** definierbar
   (in/out getrennt) — jede Gruppe bringt ihren eigenen Blend-Charakter mit.
   Beim Wechsel A→B wirken Ausgangskurve von A und Eingangskurve von B.
   Die **Playlist-Anbindung ist global**.
5. **Tiefenregel:** genau eine Ebene — Haupt-Host → Host-Gruppen. Keine
   Host-Gruppe in einer Host-Gruppe.
6. **Anzahl unbegrenzt (2026-07-23):** Die Kette darf beliebig viele
   Host-Gruppen enthalten, und **mehrere dürfen gleichzeitig aktiv sein** —
   sie rendern dann wie normale Chain-Nodes in Ketten-Reihenfolge mit ihren
   Blend-Modi übereinander. „Zwei" gilt nur für den Crossfade-Moment:
   geblendet wird paarweise A→B (§4). Performance-Hinweis: jede aktive
   Gruppe kostet ein volles Rendering (eigenes Feedback/Blur) — der
   Freeze-Frame-Fallback (§4) greift je Gruppe.

## 3. Datenmodell

- Neuer Variant-Fall **`HostGroupParams`** in `EffectParams`
  (EffectChain.hpp). Die Gruppe nutzt — wie `ListParams` — die vorhandenen
  `ChainNode::children` als Inhalt; damit funktionieren Panel-Baum,
  Drag&Drop, Compile-Pass (nodeId-Vergabe) und Serializer-Pfade ohne
  Sonderwege. `isList()` bekommt ein Gegenstück `isHostGroup()`; die
  „children nur für Listen"-Regel wird auf „Listen + Host-Gruppen" erweitert.
- Felder (Startumfang): Crossfade-Settings (Dauer, Kurve — gespiegelt über
  die Sync-Regel aus §2.4), optional Herkunfts-Metadaten (importierte
  .lvfx-Quelle, Origin-Icon).
- **Laufzeit:** je Host-Gruppe (per nodeId) ein eigener Runtime-Satz —
  eigener Buffer-/Feedback-Bestand, eigene Skript-Kontexte. Vorbild ist die
  bestehende per-nodeId-Runtime des Milkdrop-Nodes (`runMilkdropNode`):
  der Milkdrop-Node ist strukturell bereits ein „Level-1-Host mit einem
  Insassen".

## 4. Crossfade-Zustandsmodell (E5)

- Zustände des Haupt-Hosts: `Active(A)` → `Blending(A→B, progress 0..1)` →
  `Active(B)`. Der Crossfade wechselt die **designierte aktive Gruppe**
  (Playlist-Slot); davon unabhängig dürfen weitere Host-Gruppen dauerhaft
  parallel laufen (§2.6) — sie sind vom Blend nicht betroffen.
- Während `Blending` rendern **beide** Gruppen vollständig (je eigenes
  Feedback/Blur — Entscheid E5 „echtes Doppel-Rendering", beide audio-live);
  die **Mix-Stufe** mischt im Haupt-Host: Gewicht von A folgt dessen
  **Ausgangskurve**, Gewicht von B dessen **Eingangskurve** (§2.4) —
  linear/linear ist der Default der ersten Ausbaustufe.
- **Ausbaustufe:** per-Vertex-Warp-UV-Blend (à la `m_fBlendProgress`) für
  den Fall Milkdrop↔Milkdrop.
- **Freeze-Frame** bleibt ausschließlich automatischer
  **Performance-Fallback** (Frame-Budget-Trigger), nie Default.
- Nur ein Blend gleichzeitig. Verhalten bei erneutem Wechsel **während**
  eines laufenden Blends: offener Kleinpunkt (§8).
- Die Milkdrop-`progress`-Variable (heute 60-s-Stub) speist sich künftig aus
  der Host-Gruppen-Laufzeit/Playlist.

## 5. Tiefenregel — Durchsetzung

Drei Stellen verhindern verschachtelte Host-Gruppen:

1. **Add-Dropdown/Palette:** Der Eintrag „Host-Gruppe" wird ausgeblendet
   bzw. deaktiviert, wenn die Einfüge-Position innerhalb einer Host-Gruppe
   liegt.
2. **Drag&Drop:** Die Move-Validierung (`moveNodesLocked`) lehnt Drops ab,
   deren Zielpfad durch eine Host-Gruppe führt, wenn der bewegte Teilbaum
   selbst eine Host-Gruppe enthält.
3. **Lade-/Import-Pfad:** Findet der Serializer/Import verschachtelte
   Host-Gruppen (defekte/fremde Datei), wird **aufgeflacht** (Kinder rücken
   hoch) + Import-Report-Warnung — kein Hard-Fail (Fehlerphilosophie der
   Import-Phase).

## 6. Persistenz `.lvfx2`

- ChainSerializer: neuer Node-Key `"hostgroup"`; Format-Wahl beim Speichern
  automatisch (Kette enthält ≥ 1 Host-Gruppe → `.lvfx2`, sonst `.lvfx`).
- Laden: beide Endungen über denselben Dispatch; Import-Browser filtert
  `.lvfx` + `.lvfx2` (+ `.milk`/`.avs` wie bisher).
- „.lvfx in Host-Gruppe importieren": Kontextaktion an der Host-Gruppe
  (Datei wählen → Kette wird `children`); nur zulässig, wenn die Quelle
  selbst keine Host-Gruppe enthält (sonst §5.3-Aufflachen mit Warnung).

## 7. Playlist-Anbindung (E6, Ausblick)

Die Visual-Playlist arbeitet **global**: Sie wechselt die aktive Host-Gruppe
(mit Crossfade nach §4) und ist damit für Milkdrop/AVS/LumiViz identisch.
Details bleiben im Playlist-Konzept (ui/Visual_Playlist_Konzept.md).

## 8. Umsetzungsschnitte + offene Kleinpunkte

| Schnitt | Inhalt |
|---|---|
| **HG1** | Node-Typ `HostGroupParams` + Runtime-Trennung (eigene Buffers je Gruppe) + Palette/Panel + Tiefenregel (§5) + Serializer/`.lvfx2` + .lvfx-Import in Gruppe — noch ohne Crossfade |
| **HG2** | Crossfade: Zustandsmodell, linearer Bild-Mix, Settings am Host-Modul + Sync mit UI-Hinweis |
| **HG3** | Performance-Fallback (Freeze-Frame), `progress`-Anbindung, Feinschliff |
| danach | Visual-Playlist (E6) auf Host-Gruppen |

Offene Kleinpunkte (bei Freigabe mitentscheiden oder in HG2 klären):
- Wechsel während laufendem Blend: laufenden Blend hart abschließen (Snap)
  oder Ziel ersetzen?
- Kurven-Satz: Start nur linear; welche weiteren Kurven-Typen für die
  individuellen Ein-/Ausgangskurven (§2.4) — z. B. ease-in/out, exponentiell,
  S-Kurve?

## 9. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.0.0 | 2026-07-22 | Erstfassung (Session 42): Zielbild Host-Gruppen als neuer Node-Typ, Entscheide Patrik (§2: .lvfx2, Settings-Sync, Tiefenregel), Zustandsmodell E5, Durchsetzung Tiefenregel, Umsetzungsschnitte HG1–HG3 |
| 1.1.0 | 2026-07-23 | Entscheid Patrik §2.6: Anzahl der Host-Gruppen unbegrenzt, mehrere gleichzeitig aktiv erlaubt (rendern als normale Nodes mit Blends); Crossfade bleibt paarweise A→B auf der designierten aktiven Gruppe (§4 präzisiert) |
| 1.2.0 | 2026-07-23 | Entscheid Patrik §2.4 präzisiert: Ein-/Ausgangskurven **je Host-Gruppe individuell** (vom Settings-Sync ausgenommen); Mix-Stufe = out-Kurve(A) × in-Kurve(B), linear/linear als Default (§4) |
| 1.3.0 | 2026-07-23 | **HG1 umgesetzt** (Session 42): Node-Typ + Runtime-Trennung (Pool/Context-Scope-Switch), Tiefenregel (Degradierung statt Aufflachen — Kinder bleiben in einer Effect List erhalten, dokumentierte Abweichung von §5.3), `.lvfx2`, Panel + .lvfx-Import in Gruppe. Fortschritt: [MilkDrop_Import_Status.md](MilkDrop_Import_Status.md) §2 |

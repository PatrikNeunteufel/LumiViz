# harvest/old_docs — archivierte App-Doku (Stand vor der Doku-Neuordnung, Session 29)

> **Herkunft:** Kompletter Inhalt von `projects/apps/MyViz/docs/` (außer `sessions/` —
> die Produkt-Changelogs bleiben am deklarierten Ort). Verschoben 2026-07-18
> (Session 29, Phase 4-Vorbereitung) — die App-Doku wird nach Domänen neu aufgebaut
> (UI · Audio · Visuals · Services), die alten Inhalte dienen als Quelle.
> **Dieser Ordner ist temporär:** Sobald ein Inhalt in die neue Doku übernommen oder
> als obsolet erkannt ist, wird er hier gestrichen; ist alles abgearbeitet, fliegt die Gruppe.

## Inhalt (alte Struktur)

| Ordner/Datei | Was drinsteckt | Verwertung |
|---|---|---|
| `INDEX.md`, `README_MODULES.md` | Einstieg/Index der alten Struktur | ersetzt durch neuen `docs/INDEX.md` |
| `architecture/` | Event-System, Registries (+ LazyInit), VisualSystem „LumiPulse", Layout-Persistenz | offen |
| `concepts/` | PostProcess-Module, Superscope (Entwürfe) | offen |
| `integration/` | Application-Integration (Bootstrap/DI-Verdrahtung) | offen |
| `modules/` | Subsystem-Doku: Audio, Panels, Menü, Presets, IModule, ColorGradient, Smoothing, AudioSource | offen |
| `references/` | Parameter, Enums, Dateiformate, Preset-System, Visualizer-Architektur, API-Cheatsheet | offen |
| `userguide/` | ConfigPanel, Preset-System, Visualizer-Module, OpenGL-Context | offen |

## Bekannte Schieflagen der alten Doku (Grund der Neuordnung)

- Struktur nach Doku-**Typ** (architecture/modules/references/userguide) statt nach
  **Domäne** (UI / Audio / Visuals / Services) — Zusammengehöriges liegt verstreut.
- `references/Parameter_Reference.md` dokumentiert nur die Modul-Welt des
  PulsingVisualizer (`audio.*`, `shape.*`) — die Equalizer-Key-Welt (`bands`, `grad.*`,
  `peak.*`) fehlt komplett (siehe `harvest/config-pipeline/`).
- Teilweise Soll-Beschreibungen, die vom Ist-Code abweichen (vor Übernahme gegen Code prüfen).

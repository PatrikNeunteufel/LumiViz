# MyViz — Dokumentations-Index

> **Version:** 2.0.0
> **Datum:** 2026-07-18
> **Typ:** Index
> **Status:** Aktiv
> **Sprache:** Deutsch

Einstieg in die App-Dokumentation, geordnet nach **Domänen** (Neuordnung Session 29).
**Modul-Doku liegt bei den Headern** (`include/**/[Modul].md`, Format: CppModuleDoc) —
hier liegt alles Übergreifende je Domäne.

**Für Anwender:** [Benutzerhandbuch](Benutzerhandbuch.md) — Bedienung der App als
Ganzes (Player, Playlist, Visualizer, Vollbild, Docking, Einstellungen).

| Domäne | Inhalt |
|---|---|
| [core-services/](core-services/) | Service-Fundament: [Event-System](core-services/Event_System.md) · [Registries](core-services/Registries.md) (inkl. Lazy-Init) · [Bootstrap/DI-Verdrahtung](core-services/Bootstrap_Integration.md) |
| [audio/](audio/) | Audioplayer-Domäne: [Audio-System](audio/Audio_System.md) (BassEngine, Player, Analyzer, Playlist + Signalkette zur Visualisierung) |
| [visuals/](visuals/) | Visualizer-Domäne: [Visualizer-Architektur](visuals/Visualizer_Architecture.md) (IVisualizer, Modul-System, Parameter) · [Parameter-Referenz](visuals/Parameter_Reference.md) (alle Visualizer, Pipeline-Schema!) · [OpenGL-Context-Handling](visuals/OpenGL_Context_Handling.md) · [Config-Pipeline-Konzept](visuals/Config_Pipeline_Concept.md) (Phase 4, **Stabil**) · [Umsetzungsplan](visuals/Config_Pipeline_Umsetzungsplan.md) · [Key-Migration Alt→Neu](visuals/Parameter_Key_Migration.md) · [Preview-Viewer-Entwurf](visuals/Preview_Viewer_Entwurf.md) · [Render-Thread-Entwurf](visuals/Render_Thread_Entwurf.md) (umgesetzt) · [Import-Analyse AVS/MilkDrop](visuals/Import_Analyse_AVS_MilkDrop.md) (ref/-Repos, EEL→Lua, Import-Roadmap) · [Import-Fundament-Entwurf](visuals/Import_Fundament_Entwurf.md) (Roadmap 4, umgesetzt) · [Multieffekt-Host-Entwurf](visuals/Import_Multieffekt_Host_Entwurf.md) (Roadmap 5, **umgesetzt 5.1–5.7b**) · [Modul-Abdeckung](visuals/Import_Modul_Abdeckung.md) (Abdeckungsmatrix Builtins+APEs + Priorisierung) · [Modul-Umsetzungsplan](visuals/Import_Modul_Umsetzungsplan.md) (Rezept + Batches A–G) · [MilkDrop-Import-Konzept](visuals/MilkDrop_Import_Konzept.md) (Roadmap 6, M1–M6 + Shader-Stufen + Entscheide) · [**MilkDrop-Import-Status**](visuals/MilkDrop_Import_Status.md) (SSOT Fortschritt + Bezeichnungs-Legende) · [Vereinheitlichungs-Konzept](visuals/Vereinheitlichung_Konzept.md) (Skript-Set AVS↔Milk, Standalone-Portierung, Gradients — Vorbereitung Meganode-Split, Entwurf) |
| [ui/](ui/) | UI-Domäne: [Panel-System](ui/Panel_System.md) (Qt-ADS-Docking) · [Menü-System](ui/Menu_System.md) · [ConfigPanel-Guide](ui/ConfigPanel_Guide.md) · [Visual-Playlist-Konzept](ui/Visual_Playlist_Konzept.md) (Preset-Auto-Wechsel + Import-Browser-Erweiterung + Hotkeys, Konzept) |
| [presets/](presets/) | Persistenz-Domäne: [Preset-System](presets/Preset_System.md) · [Dateiformat-Referenz](presets/FileFormat_Reference.md) · [Layout-Persistenz](presets/Layout_Persistence.md) |
| [sessions/](sessions/) | **Produkt-Changelogs** je Arbeitsphase ([jüngster Stand](sessions/Changelog_Session44.md)) |

## Regeln

- **Keine Versionskopien** (`Name100.md`) — Git ist die Historie.
- Formate/Vorlagen für neue Dokumente und Modul-Doku: CMakeCraft **autorenwerk**
  (nach Bootstrap lokal: `.externals/cmakecraft/<version>/docs/de/autorenwerk/`).
- **Prozess-Doku** (Session-Reports, Handover) liegt bewusst NICHT im Repo, sondern lokal in
  `.claude/sessions/` · `.claude/handover/` — hier in `sessions/` stehen nur die
  **Produkt**-Changelogs (was sich an der App geändert hat).
- Die **alte Doku** (Struktur nach Doku-Typ) liegt bis zur vollständigen Einarbeitung
  archiviert in `harvest/old_docs/` (Repo-Root) — dort auch die unimplementierten
  Konzepte (PostProcess, Superscope-Entwurf, LumiPulse-Node-Vision).
- Build-System-Fragen → CMakeCraft-Doku (`docs/INDEX.md` dort).

# MyViz — Dokumentations-Index

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Index
> **Status:** Aktiv

Einstieg in die App-Dokumentation. **Modul-Doku liegt bei den Headern** (`include/**/[Modul].md`,
Format: CppModuleDoc) — hier liegt alles Übergreifende.

| Bereich | Inhalt |
|---|---|
| [architecture/](architecture/) | Soll-Architektur: [Event-System](architecture/Event_Architecture.md) · [Registries](architecture/Registry_Architecture.md) (+ [LazyInit](architecture/Registry_LazyInit.md)) · [VisualSystem „LumiPulse"](architecture/LumiPulse_VisualSystem_Architecture.md) · [Layout-Persistenz](architecture/Layout_Persistence.md) |
| [concepts/](concepts/) | Geplantes/Entwürfe: [PostProcess-Module](concepts/PostProcessModule-Concept.md) · [Superscope](concepts/SuperscopeVisualizer-Concept.md) — Umsetzungs-Blaupausen zusätzlich in `harvest/` (Repo-Root) |
| [modules/](modules/) | Subsystem-Doku: Audio, Panels, Menü, Presets, Module (IModule, ColorGradient, Smoothing, AudioSource) — Index: [README_MODULES.md](README_MODULES.md) |
| [references/](references/) | Nachschlagen: [Parameter](references/Parameter_Reference.md) · [Enums](references/Enum_Reference.md) · [Dateiformate](references/FileFormat_Reference.md) · [Preset-System](references/Preset-System-Reference.md) · [Visualizer-Architektur](references/Visualizer-Architecture-Reference.md) · [API-Cheatsheet](references/API_Cheatsheet.md) |
| [userguide/](userguide/) | Bedienung/Entwicklung: ConfigPanel, Preset-System, Visualizer-Module, OpenGL-Context |
| [integration/](integration/) | [Application-Integration](integration/Application_Integration.md) (Bootstrap/DI-Verdrahtung) |
| [sessions/](sessions/) | **Produkt-Changelogs** je Arbeitsphase ([jüngster Stand](sessions/Changelog_Session27b_Nachtrag.md)) |

## Regeln

- **Keine Versionskopien** (`Name100.md`) — Git ist die Historie (aufgeräumt 2026-07-18).
- Formate/Vorlagen für neue Dokumente und Modul-Doku: CMakeCraft **autorenwerk**
  (nach Bootstrap lokal: `.externals/cmakecraft/docs/de/autorenwerk/`).
- **Prozess-Doku** (Session-Reports, Handover) liegt bewusst NICHT im Repo, sondern lokal in
  `.claude/sessions/` · `.claude/handover/` — hier in `sessions/` stehen nur die
  **Produkt**-Changelogs (was sich an der App geändert hat).
- Build-System-Fragen → CMakeCraft-Doku (`docs/INDEX.md` dort).

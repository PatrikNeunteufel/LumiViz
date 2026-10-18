# LumiViz — Dokumentations-Index

> **Version:** 2.4.0
> **Datum:** 2026-10-18
> **Typ:** Index
> **Status:** Aktiv
> **Sprache:** Deutsch

Einstieg in die App-Dokumentation, geordnet nach **Domänen** (Neuordnung Session 29).
**Modul-Doku liegt bei den Headern** (`include/**/[Modul].md`, Format: CppModuleDoc) —
hier liegt alles Übergreifende je Domäne.

**Für Anwender:** [Benutzerhandbuch](Benutzerhandbuch.md) — Bedienung der App als
Ganzes (Player, Playlist, Visualizer, Vollbild, Docking, Einstellungen).

**Vor jedem Prüf- oder Messlauf:** [**Werkzeug-Wegleitung**](Werkzeug_Wegleitung.md) —
welches Programm wofür (App · Standalones · Referenz-Renderer) und die sieben Fallen,
die schon Arbeitszeit gekostet haben: Debug statt Release (Faktor 20), fehlender
Render-Scale-Divisor im Standalone, GPU-Vorgabe am EXE-Pfad, Pfade mit Leerzeichen,
Fenster nicht schließen, echte Musik im Referenzvergleich.

**Sichttests mit dem Tester fahren:** [**Sichttest-Steuerung**](Sichttest_Steuerung.md) —
`LumiViz.exe --testing`, die fünfzehn Aktionen, die der Tester auslöst, wo der Code
liegt, wie das Paket bezogen wird. Für Entwickler; nicht im Benutzerhandbuch und
nicht im Release-Paket.

**Treue gegen das Original nachweisen:** [**Kalibrierung**](kalibrierung/INDEX.md) —
formatübergreifender Einstieg (AVS · MilkDrop · Shadertoy · ISF): die fünf
Methodenregeln, die Werkzeuge je Format, die Audio-Testsignale und der Stand
je Format. Dazu [Messmittel](kalibrierung/Messmittel.md) — die Kalibrier-Raster
und wie sie abgenommen werden.

**Was ist noch zu tun:** [**Offene Punkte**](Offene_Punkte.md) — die Arbeitsliste
(Kalibrier-Befunde, offene Urteile, Sichttests, Entscheide, Backlog). Ein Ort;
löst die früheren `Offene_Implementierungen.md` + `Offene_Sichttests.md` ab, die
auf Session 37 stehengeblieben waren. Laufendes Prüfprotokoll zum Ausfüllen:
[Sichttest S79/S80](Sichttest_Protokoll_S79.md) (Effekt-Palette, Formelfelder).

**Was könnte noch kommen:** [**Modul-Ideen**](Modul_Ideen.md) — alle notierten
Ideen für neue Module und den Ausbau bestehender auf einer Seite, je mit Stand
und Verweis auf die Einzelheiten.

| Domäne | Inhalt |
|---|---|
| [core-services/](core-services/) | Service-Fundament: [Event-System](core-services/Event_System.md) · [Registries](core-services/Registries.md) (inkl. Lazy-Init) · [Bootstrap/DI-Verdrahtung](core-services/Bootstrap_Integration.md) · [Datenablage](core-services/Datenablage_Konzept.md) (Konzept, Entwurf S114: Mitgeliefertes neben der Exe, Eigenes im Benutzerordner, Export und Import) · [Publish](core-services/Publish_Konzept.md) (Konzept, Entwurf S125: ein Ding mit allem, was es nennt, als Paket `.lvdata` Fassung 2) |
| [audio/](audio/) | Audioplayer-Domäne: [Audio-System](audio/Audio_System.md) (BassEngine, Player, Analyzer, Playlist + Signalkette zur Visualisierung); [Loop](audio/Loop_Konzept.md) (Konzept, Entscheide, Regeln und Messungen: Loop-Panel als Arrangement, Drum-Editor, `.lvloop` als Clip im Composer; S108, gebaut bis Stufe 8 in S109); [Klang aus einer Form](audio/Shape_Konzept.md) (Konzept, S120: die Waveform Shape einer Schicht aus einer Kontur — gebaut bis Stufe 20, S122); [VST3](audio/VST3_Konzept.md) (Konzept S123: fremde Instrumente und Effekte im Loop — Entwurf, nichts entschieden); [Saiteninstrumente](audio/Saiten_Konzept.md) (Konzept S132: Saiten und Stimmung am Instrument, Waveguide und Extended KS, Creator-Skizze gebilligt — gebaut Stufe 23a) |
| [visuals/](visuals/) | Visualizer-Domäne: [Visualizer-Architektur](visuals/Visualizer_Architecture.md) (IVisualizer, Modul-System, Parameter) · [Parameter-Referenz](visuals/Parameter_Reference.md) (alle Visualizer, Pipeline-Schema!) · [OpenGL-Context-Handling](visuals/OpenGL_Context_Handling.md) · [Config-Pipeline-Konzept](visuals/Config_Pipeline_Concept.md) (Phase 4, **Stabil**) · [Umsetzungsplan](visuals/Config_Pipeline_Umsetzungsplan.md) · [Key-Migration Alt→Neu](visuals/Parameter_Key_Migration.md) · [Preview-Viewer-Entwurf](visuals/Preview_Viewer_Entwurf.md) · [Render-Thread-Entwurf](visuals/Render_Thread_Entwurf.md) (umgesetzt) · [Import-Analyse AVS/MilkDrop](visuals/Import_Analyse_AVS_MilkDrop.md) (ref/-Repos, EEL→Lua, Import-Roadmap) · [Import-Fundament-Entwurf](visuals/Import_Fundament_Entwurf.md) (Roadmap 4, umgesetzt) · [Multieffekt-Host-Entwurf](visuals/Import_Multieffekt_Host_Entwurf.md) (Roadmap 5, **umgesetzt 5.1–5.7b**) · [Modul-Abdeckung](visuals/Import_Modul_Abdeckung.md) (Abdeckungsmatrix Builtins+APEs + Priorisierung) · [Modul-Umsetzungsplan](visuals/Import_Modul_Umsetzungsplan.md) (Rezept + Batches A–G) · [MilkDrop-Import-Konzept](visuals/MilkDrop_Import_Konzept.md) (Roadmap 6, M1–M6 + Shader-Stufen + Entscheide) · [**MilkDrop-Import-Status**](visuals/MilkDrop_Import_Status.md) (SSOT Fortschritt + Bezeichnungs-Legende) · [Vereinheitlichungs-Konzept](visuals/Vereinheitlichung_Konzept.md) (Skript-Set AVS↔Milk, Standalone-Portierung, Gradients — Vorbereitung Meganode-Split, Entwurf) · [**AVS-Kalibrier-Methodik**](visuals/AVS_Kalibrier_Methodik.md) (Standard-Vorgehen: flächenunabhängiges Urteil, Sondenformen, Bisektion, Layouts pinnen, Verifikationsgürtel) · [**Knoten-Parameter-Konzept**](visuals/Knoten_Parameter_Konzept.md) (Voreinstellungen je Knoten, fehlende Editor-Felder, festgenagelte Werte freimachen, dynamische EEL-Felder — Default-Vertrag + Etappen, Entwurf) · [**Formelfelder-Inventar**](visuals/Formelfelder_Inventar.md) (alle 84 Module: Skript-Slots, gebundene Variablen, Kandidaten + Vorschläge A–F; S79) · [**Feldsteuerung (Block F)**](visuals/Feldsteuerung_Konzept.md) (jedes Feld per Skript/Modulation, Feldtabelle als SSOT; Entwurf S84, Entscheide E1–E6 offen) · [**Karaoke-Ebene**](visuals/Karaoke_Ebene_Konzept.md) (Songtext über jedem Preset als eigene Ebene beim Present, Karaoke-Panel mit zwei Reitern; Kurzkonzept S88, Entscheide K1–K5 offen) · [**ISF-Import & Parameter-Baum**](visuals/ISF_Import_Parameterbaum_Plan.md) (Filter aus ISF laden, generischer typsicherer Parameter-Baum für alle Module, Lizenz-Kette — 4 Stufen, Entwurf) |
| [ui/](ui/) | UI-Domäne: [Panel-System](ui/Panel_System.md) (Qt-ADS-Docking) · [Menü-System](ui/Menu_System.md) · [ConfigPanel-Guide](ui/ConfigPanel_Guide.md) · [Visual-Playlist-Konzept](ui/Visual_Playlist_Konzept.md) (Preset-Auto-Wechsel + Import-Browser-Erweiterung + Ausblick Composer, Konzept) · [**Hotkey-Konzept**](ui/Hotkey_Konzept.md) (Ausbaustufen, Reservierung der Transporttasten, Aktions-Modell, Settings-Editor) · [Screenshot-Ablage](ui/Screenshot_Ablage.md) (Ordner je Programmlauf, Aufnahme im Render-Thread, Vollbild-Fehlerregel) |
| [presets/](presets/) | Persistenz-Domäne: [Preset-System](presets/Preset_System.md) · [Dateiformat-Referenz](presets/FileFormat_Reference.md) · [Layout-Persistenz](presets/Layout_Persistence.md) |
| [tutorials/](tutorials/) | **Lehr- & Referenzmaterial** (Blueprint-System-Formate): [Shader-Tutorial-Serie](tutorials/ShaderTutorials-overview.md) (Wegleitung + 9 Tutorials, Shadertoy↔LumiViz) · [Raymarching-Referenz](tutorials/Raymarching-reference.md) — Heimat auch für künftige Milkdrop-/AVS-Tutorials und weitere Referenzdokumente |
| [sessions/](sessions/) | **Produkt-Changelogs** je Arbeitsphase ([jüngster Stand](sessions/Changelog_Session53.md)) |

Die beiden Kalibrier-Protokolle ([AVS-Sichttest-Protokoll](visuals/AVS_Sichttest_Protokoll.md),
[MilkDrop-Import-Status](visuals/MilkDrop_Import_Status.md)) sind **Befund-Archive**:
sie halten fest, was untersucht und woran es lag. Was daraus noch offen ist, steht
gebündelt in [Offene_Punkte.md](Offene_Punkte.md).

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

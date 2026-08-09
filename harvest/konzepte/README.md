# harvest/konzepte — Architektur- und Zukunftskonzepte der Vorgänger

**Quellen:** Viz2025, NewViz2025, lose Dateien. **Ziel:** Roadmap-Material nach Phase 4;
bei der Doku-Reorg (Phase 2) nach `docs/konzepte/` einsortieren.

| Datei | Was es ist | Relevanz |
|---|---|---|
| `viz_2025_node_reference_manual_visualizer_pipeline.md` | **Node-Editor-Referenz**: komplette Visualizer-Pipeline als Node-Graph (Audio-Input → Analyse → Generatoren → Modifier → Compositing → Output), Typsystem mit Farbcodierung, Skript-/Shader-Lebenszyklen (AVS/MilkDrop-artig), Standard-Uniforms | Die große Zukunftsvision; auch ohne Node-Editor als **Begriffs- und Schnittstellen-Referenz** wertvoll (z. B. Steuerquellen UI/Control/Script je Parameter → passt zur Config-Pipeline) |
| `Taxiome.md` / `taxonome.md` | **Taxonomie & Basisklassen** (System/Service/Manager/Host/Controller/Registry/Agent/Node), Abhängigkeitsrichtung, Lifecycle-Regeln (idempotent, Reverse-Shutdown), CPU/GPU-Dualpfad | Architektur-Vokabular; Abgleich mit heutiger LumiViz-Architektur lohnt (zwei Varianten, `Taxiome` ist die überarbeitete) |
| `plug_in_system_architektur_integrationsleitfaden.md` + `Tiefenanalyse …`-Varianten (als .docx/.pdf im Archiv) | Plug-in-System-Entwurf **+ Schwachstellen-Analyse** | Review-Material für die heutige Registry-/Self-Registration-Architektur |
| `potentielle Problemstellen.md` | Gesammelte bekannte Risiken der Viz2025-Codebasis | Checkliste: Welche gelten im heutigen Code noch? |
| `dialog_manager_readme.md`, `filedialogs.md` | Dialog-Manager-Konzept | Abgleich mit heutigem DialogManager/DialogRegistry |
| `auto_menu_concept.md` | Automatische Menü-Generierung | Abgleich mit heutiger MenuRegistry |
| `diagram.mermaid` | Architektur-Diagramm Viz2025 | Historie/Vergleich |
| `visuals.txt` | Wunschliste Visuals (Fractal, Bump, Shader, 3D) | Feature-Backlog |
| `cmake_specialUse.md` | CMake-Sondernotizen aus NewViz2025 | ggf. für CMakeCraft-Doku |

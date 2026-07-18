# harvest/ — Wiederverwertungslager aus den Vorgänger-Projekten

> **Zweck:** Vor der Archivierung der vier Vorgänger (Visualizer_better_wave →
> GreatVisual_most param eq → Viz2025 → NewViz2025) wurden hier alle wiederverwertbaren
> Funde gesichert. **Dieser Ordner ist temporär:** Jedes Stück wandert bei seiner
> Plan-Phase an den endgültigen Ort; ist eine Gruppe eingearbeitet, wird sie gelöscht.
> Die Originale (inkl. docx/pdf-Duplikate) liegen im Archiv unter `Visuals_Project/_archive/`.
>
> Erstellt: 2026-07-17 (Session NewViz_Session28). Plan-Phasen siehe Projekt-CLAUDE.md.

| Gruppe | Inhalt | Quelle | Ziel-Phase |
|---|---|---|---|
| [tests/](tests/README.md) | ~2.300 Zeilen Catch2-Unit-Tests (EventBus, ServiceContainer, CommandBus, BaseTypes) | NewViz2025 | **Phase 3** — auf doctest/MyViz-API portieren |
| [core-module/](core-module/README.md) | CommandBus komplett (Undo/Redo), EventBus mit RAII/Weak-Abos, ServiceContainer, BaseTypes + Usage-Guides/Cheatsheets | NewViz2025 (`viz::core`-Generation) | **Phase 4** — CommandBus einführen, EventBus-Features nachrüsten |
| [config-pipeline/](config-pipeline/README.md) | Equalizer-Parameter-Vollreferenz + Ideen zu Preview-Viewern, Default-Buttons, Gruppen-Presets, AudioSource je Visual vs. gemeinsam | GreatVisual, better_wave, lose Notizen | **Phase 4** — Anforderungen für die einheitliche Config-Pipeline |
| [konzepte/](konzepte/README.md) | Node-Editor-Referenz, Taxonomie/Basisklassen, Plug-in-System-Analyse, Auto-Menü, Dialog-Manager | Viz2025, NewViz2025, lose Dateien | Zukunft (nach Phase 4) — Roadmap-Material |
| [prozess/](prozess/README.md) | Chat-Index + Sprint-Report-Muster aus der KI-gestützten Arbeit | NewViz2025 | **Phase 2** — Vorlage für `.claude/sessions/` |

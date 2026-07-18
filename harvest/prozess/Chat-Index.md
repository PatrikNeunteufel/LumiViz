# 📋 Projektplan viz2025 — Chat-Index

---

## Sprint 1 — Application & Core

**Tag 1 – Projektgrundgerüst & Ordnerstruktur**  
- **Titel:** `Sprint-01 Tag-01: Projektgrundgerüst & Ordnerstruktur`  
- **Prompt:**  
  > „Leite mich Schritt für Schritt beim Aufbau der Projektstruktur (Ordner, source.cmake, common/Types.hpp, clang-format etc.). Ziel ist eine kompakte, erweiterbare Basis.“  
- **Referenzen:**  
  - 0826T23 Core Vereinheitlichung Vorschlag  
  - 0826T22 Projektanalyse C++ Anwendung (für Gesamtaufbau src/)  

**Tag 2 – ServiceContainer**  
- **Titel:** `Sprint-01 Tag-02: ServiceContainer`  
- **Prompt:**  
  > „Erstelle mit mir die Basisklasse ServiceContainer (DI), inkl. RAII, Scoped/Shared Services, Unit-Test Beispiele.“  
- **Referenzen:**  
  - 0826T23 Core Vereinheitlichung Vorschlag  

**Tag 3 – EventBus**  
- **Titel:** `Sprint-01 Tag-03: EventBus`  
- **Prompt:**  
  > „Baue einen typsicheren EventBus (templated Topics, SubscriberHandles, Weak-Abo). Zeige Unit-Test Beispiel.“  
- **Referenzen:**  
  - 0826T23 Core Vereinheitlichung Vorschlag  

**Tag 4 – CommandBus**  
- **Titel:** `Sprint-01 Tag-04: CommandBus`  
- **Prompt:**  
  > „Implementiere CommandBus mit Command-Interface, Undo/Redo Stack, Beispiel OpenFileCommand.“  
- **Referenzen:**  
  - 0826T23 Core Vereinheitlichung Vorschlag  

**Tag 5 – Basisklassen (Manager/Controller/Registry/Agent)**  
- **Titel:** `Sprint-01 Tag-05: Basisklassen für Manager/Controller/Registry/Agent`  
- **Prompt:**  
  > „Erstelle vereinheitlichte Basisklassen (BaseManager, UIManager, BaseController, BaseRegistry, Agent). Bitte mit Beispielskeletons.“  
- **Referenzen:**  
  - 0826T22 Projektanalyse C++ Anwendung  
  - 0826T22 Manager Architektur Analyse  
  - 0826T23 Registry Architektur Analyse  
  - 0827T09 Traits in Projektanwendung  
  - 0827T19 Agenten Basisstruktur Vorschlag  

**Tag 6 – Application-Schicht**  
- **Titel:** `Sprint-01 Tag-06: Application-Schicht`  
- **Prompt:**  
  > „Implementiere Application inkl. ApplicationLifetime, ApplicationController. Orchestriere ServiceContainer, Manager, Controller. Logging & Event AppStarted.“  
- **Referenzen:**  
  - 0827T14 Projektstruktur und Verbesserungsvorschläge  

---

## Sprint 2 — WindowSystem

**Tag 7 – Windowing & Main Menu Framework**  
- **Titel:** `Sprint-02 Tag-07: Windowing & Main Menu`  
- **Prompt:**  
  > „Baue WindowSystem (Create/Destroy, Resize-Events, DPI). Menü-Registry + Menü-Manager.“  
- **Referenzen:**  
  - 0826T23 Registry Architektur Analyse  

**Tag 8 – Shortcuts & Commands-Verknüpfung**  
- **Titel:** `Sprint-02 Tag-08: Shortcuts & CommandBus Binding`  
- **Prompt:**  
  > „Binde Menüeinträge an CommandBus, inkl. Shortcuts (Ctrl+O). Beispiel Help→About.“  
- **Referenzen:**  
  - 0827T20 Projektanalyse und Architekturplanung  

---

## Sprint 3 — DialogSystem

**Tag 9 – Dialog-Basisschicht**  
- **Titel:** `Sprint-03 Tag-09: Dialog-Basisschicht`  
- **Prompt:**  
  > „Baue DialogRegistry, DialogManager, Dialog-Interface (modal/non-modal). Beispiel-Stub.“  
- **Referenzen:**  
  - 0827T20 Projektanalyse und Architekturplanung  

**Tag 10 – About-Dialog**  
- **Titel:** `Sprint-03 Tag-10: About-Dialog`  
- **Prompt:**  
  > „Implementiere AboutDialog (Version, Build-Infos, Drittlibs, Lizenz). Öffnen via Menü/Command.“  
- **Referenzen:**  
  - 0827T20 Projektanalyse und Architekturplanung (ergänzend zur Menü-Verknüpfung)  

---

## Sprint 4 — PanelSystem & Settings

**Tag 11 – Panel-Basisschicht & Docking**  
- **Titel:** `Sprint-04 Tag-11: Panel-Basis & Docking`  
- **Prompt:**  
  > „Baue PanelRegistry, PanelManager, Panel-Interface. Dockspace einrichten.“  
- **Referenzen:**  
  - 0826T23 Registry Architektur Analyse  

**Tag 12 – UIController & Settings-Infra**  
- **Titel:** `Sprint-04 Tag-12: UIController & Settings-Infra`  
- **Prompt:**  
  > „Implementiere UIController (öffnet Panels/Dialogs via Bus). ConfigService (INI/JSON). SettingsPanel Stub.“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur (Nodes & Verknüpfungen)  
  - 0827T20 Projektanalyse und Architekturplanung  

**Tag 13 – Settings Panel (Basis)**  
- **Titel:** `Sprint-04 Tag-13: Settings Panel`  
- **Prompt:**  
  > „Erweitere Settings Panel (Allgemein, UI, Pfade). Persistenz über ConfigService, Events feuern.“  
- **Referenzen:**  
  - 0827T20 Projektanalyse und Architekturplanung  

---

## Sprint 5 — AudioEngine & Panels

**Tag 14 – AudioEngine**  
- **Titel:** `Sprint-05 Tag-14: AudioEngine`  
- **Prompt:**  
  > „Implementiere AudioEngine (Bass/Wasapi), PCM-Tap, FFT, Peaks/RMS. Events für TrackLoaded, PlaybackState, AudioFrame.“  
- **Referenzen:**  
  - 0827T12 Audiosignale für Visualisierung  
  - 0827T12 AVS-Module und Pipeline  
  - 0827T12 MilkDrop Module Übersicht  

**Tag 15 – Player Panel**  
- **Titel:** `Sprint-05 Tag-15: Player Panel`  
- **Prompt:**  
  > „Erstelle PlayerPanel mit Open, Play/Pause, Seekbar, Timecodes. FileOpenDialog.“  

**Tag 16 – Playlist Panel & Open/Save**  
- **Titel:** `Sprint-05 Tag-16: Playlist Panel`  
- **Prompt:**  
  > „Implementiere PlaylistPanel (Add/Remove/Reorder, Save/Load JSON/M3U).“  

---

## Sprint 6 — NodeEditor

**Tag 17 – NodeEditor Framework**  
- **Titel:** `Sprint-06 Tag-17: NodeEditor Framework`  
- **Prompt:**  
  > „Erstelle NodeEditorPanel (ImGui). Nodes mit Ports, Verbindungslinien, Typprüfung. Mandatory vs. Optional markiert.“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur  
  - 0827T20 Projektanalyse und Architekturplanung  

**Tag 18 – NodeEditor Features**  
- **Titel:** `Sprint-06 Tag-18: NodeEditor Features`  
- **Prompt:**  
  > „Füge Kontextmenüs hinzu (Node Add/Remove), Layout-Persistenz, lose Nodes, Warnungen für fehlende Mandatory-Ports.“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur  

---

## Sprint 7 — VisualEngine & erste Nodes

**Tag 19 – VisualEngine Core**  
- **Titel:** `Sprint-07 Tag-19: VisualEngine Core`  
- **Prompt:**  
  > „Implementiere VisualEngine (Graph build/run, Scheduling). Backend-Abstraktion (IRenderTarget CPU/GPU).“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur  

**Tag 20 – Erste produktive Nodes**  
- **Titel:** `Sprint-07 Tag-20: Erste Nodes`  
- **Prompt:**  
  > „Erstelle AudioFFTNode, AudioPCMNode, FuncGeneratorNode, GradientBackgroundNode, ScopeWaveNode, BarsSpectrumNode, BlurNodeCPU, ColorAdjustNode.“  
- **Referenzen:**  
  - 0827T12 Audiosignale für Visualisierung  
  - 0827T12 AVS-Module und Pipeline  
  - 0827T12 MilkDrop Module Übersicht  

**Tag 21 – Preset-Handling & Persistenz**  
- **Titel:** `Sprint-07 Tag-21: Preset Handling`  
- **Prompt:**  
  > „Implementiere PresetSerializer (JSON) & PresetPanel (Load/Save). Optional Node PresetIO.“  
- **Referenzen:**  
  - 0827T20 Projektanalyse und Architekturplanung  

**Tag 22 – Bool- & Switch-Nodes**  
- **Titel:** `Sprint-07 Tag-22: Bool & Switch Nodes`  
- **Prompt:**  
  > „Implementiere BoolConstantNode, And/Or/Not/Xor, TriggerToggle, SwitchNode<T> mit optionalem Gruppenschalter.“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur (Skript vs Param Switches)  

---

## Sprint 8 — Basis Config Panel (Vertiefung)

**Tag 23 – Config Panel Ausbau**  
- **Titel:** `Sprint-08 Tag-23: Config Panel Ausbau`  
- **Prompt:**  
  > „Audio-Device Auswahl, Render-Backend, Pfade, UI-Theme. Live-Apply + Persistenz.“  
- **Referenzen:**  
  - 0827T20 Projektanalyse und Architekturplanung  

---

## Sprint 9 — Lua Integration

**Tag 24 – Lua Runtime & Binding**  
- **Titel:** `Sprint-09 Tag-24: Lua Runtime & Binding`  
- **Prompt:**  
  > „Implementiere LuaEngine (State mgmt, Sandbox, Error-Bridge). Binding Traits für Scalar/Vec/Image/FFT.“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur (Switch-Logik für Skripte)  

**Tag 25 – Lua-Nodes**  
- **Titel:** `Sprint-09 Tag-25: Lua-Nodes`  
- **Prompt:**  
  > „Erstelle LuaParamNode und LuaVisualNode (zugreifen auf Audio/Time). Fehleranzeige im Node.“  

**Tag 26 – Param↔Script Parallelpfad**  
- **Titel:** `Sprint-09 Tag-26: Param↔Script Switch`  
- **Prompt:**  
  > „Baue gekoppelte SwitchNodes für Param/Skript-Weiche. UI-Toggle ‚Scripting Mode‘. Runtime-Umschaltung glitchfrei.“  
- **Referenzen:**  
  - 0827T22 Modulstruktur und Architektur  

---

## Sprint 10 — Feinschliff & Doku

**Tag 27 – Stabilität & Telemetrie**  
- **Titel:** `Sprint-10 Tag-27: Stabilität & Telemetrie`  
- **Prompt:**  
  > „Teste Fehlerpfade (fehlende Devices, Files). Timeout-Guards. Logging erweitern.“  

**Tag 28 – Dokumentation & Beispiel-Presets**  
- **Titel:** `Sprint-10 Tag-28: Doku & Beispiele`  
- **Prompt:**  
  > „Erstelle Doxygen-Startseite, Modulübersichten, Architekturdiagramm. Beispiel-Presets/Playlists ins Repo.“  
- **Referenzen:**  
  - 0827T14 Projektstruktur und Verbesserungsvorschläge (Architektur-Doku)  

---

📌 Zusätzlicher Meta-Chat (dieser hier):  
- **Kürzel:** `0828T21`  
- **Titel:** Projektplan/Chat-Index  
- **Inhalt:** Enthält den gesamten Projektplan und Chat-Index mit Prompts & Referenzen.

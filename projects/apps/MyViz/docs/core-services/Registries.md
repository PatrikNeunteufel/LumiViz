# Registries — Self-Registration mit Lazy-Init

> **Version:** 1.0.0
> **Datum:** 2026-07-18
> **Typ:** Reference
> **Status:** Aktiv
> **Sprache:** Deutsch

---

## 1. Übersicht

Das Registry-Pattern erlaubt dezentrale Registrierung von Komponenten (Panels, Dialoge,
Menüs, Widgets, Visualizer) ohne Änderung am Framework-Code: pro Registry gibt es eine
wiederverwendbare Singleton-Klasse (Framework, `include/services/*Registry.hpp` +
`src/services/*Registry.cpp`) und eine app-spezifische `*AutoReg.cpp` mit allen
Registrierungen.

Pattern-Details, Descriptor-Definitionen und How-to-Beispiele stehen in der
header-nahen Modul-Doku
[`../../include/services/Registries.md`](../../include/services/Registries.md) —
dieses Dokument hält den **verifizierten Ist-Stand** der registrierten Komponenten fest.

### Die 5 Registries

| Registry | Zweck | AutoReg-Datei (verifiziert) |
|----------|-------|------------------------------|
| **MenuRegistry** | Menü-Struktur (Container, Groups, Items) | `src/UI/managers/MenuAutoReg.cpp` |
| **PanelRegistry** | Dock-Panels | `src/UI/panels/PanelAutoReg.cpp` |
| **DialogRegistry** | Dialoge | `src/UI/dialogs/DialogAutoReg.cpp` |
| **WidgetRegistry** | Dock-Inhalte/Widgets | `src/UI/widgets/WidgetAutoReg.cpp` |
| **VisualizerRegistry** | Visualizer-Effekte | `src/visualizers/VisualizerAutoReg.cpp` |

---

## 2. Lazy-Init-Pattern (Kurzfassung)

Statische Libraries + Registrierungs-Makros scheitern an Dead-Code-Elimination des
Linkers. Lösung: jede `XxxRegistry.cpp` deklariert
`extern void initXxxDefaults(XxxRegistry&)` und ruft sie beim ersten `instance()`
auf. Die `extern`-Referenz zwingt den Linker, die zugehörige `XxxAutoReg.cpp`
(mit der Definition) einzubinden — Registrierung ist damit garantiert.
Alle fünf Registries folgen exakt diesem Muster (verifiziert in
`src/services/{Menu,Panel,Dialog,Widget,Visualizer}Registry.cpp`).

Zusätzlich bietet `VisualizerRegistry.hpp` `REGISTER_VISUALIZER*`-Makros für
Self-Registration in Übersetzungseinheiten; der produktive Weg in MyViz ist aber
durchgängig die AutoReg-Datei.

---

## 3. Registrierte Komponenten (Ist-Stand aus dem Code, 2026-07-18)

### 3.1 Panels (`PanelAutoReg.cpp`)

| ID | Titel | Order | Default sichtbar | Menü-Pfad |
|----|-------|-------|------------------|-----------|
| `player` | Player | 100 | ja | View/Panels |
| `playlist` | Playlist | 200 | ja | View/Panels |
| `config` | Visualizer Config | 300 | nein | View/Panels |
| `settings` | Settings | 350 | nein | View/Panels |
| `visual_select` | Visualizers | 400 | ja | View/Panels |

`config` konfiguriert den aktiven Visualizer; `settings` (SettingsPanel) hält globale
App-Einstellungen (Audio/Performance, u. a. Frame-Mode). Die Panel-ID ist zugleich
`objectName` für die Layout-Persistenz — Ändern bricht gespeicherte Layouts.

### 3.2 Visualizer (`VisualizerAutoReg.cpp`)

| ID | Name | Kategorie (im Code) | Order |
|----|------|---------------------|-------|
| `pulsing` | Pulsing | `shape` | 100 |
| `equalizer` | Equalizer | `spectrum` | 100 |
| `waveform` | Waveform | `waveform` | 100 |
| `oscilloscope` | Oscilloscope | `waveform` | 200 |
| `superscope` | Superscope | `waveform` | 300 |

Descriptor-Feldreihenfolge ist `id, name, description, category, order`
(siehe `VisualizerRegistry.hpp`); die Kategorie-Strings oben sind die tatsächlich im
Code stehenden Werte. Ein Particle-Visualizer ist als TODO auskommentiert.

### 3.3 Dialoge (`DialogAutoReg.cpp`)

| ID | Titel | Order | Modal | Menü-Pfad | Shortcut |
|----|-------|-------|-------|-----------|----------|
| `about` | About MyViz | 900 | ja | Help | F1 |

Ein Settings-Dialog ist als TODO auskommentiert. Der `about`-Dialog wird seit
Phase 4 Schritt 1 vom `DialogManager` geöffnet
(siehe [Event_System.md](Event_System.md), Abschnitt DialogManager).

### 3.4 Widgets (`WidgetAutoReg.cpp`)

| ID | Name | Kategorie | Order | allowMultiple |
|----|------|-----------|-------|---------------|
| `visualizer` | Visualizer | Visualizers | 100 | nein |

`allowMultiple = false` wird vom `DockManager` beim `CreateVisualizerEvent` geprüft:
existiert bereits eine Instanz, wird keine zweite erstellt. (Der Doku-Kopf in
`WidgetAutoReg.cpp` behauptet `true` — der registrierte Wert im Code ist `false`.)
Volume-, Waveform- und Spectrum-Widgets sind als TODO auskommentiert.

### 3.5 Menü-Struktur (`MenuAutoReg.cpp`)

```
File (100)
├── Open Audio... (100)   [Ctrl+O]   (derzeit nur Debug-Log)
└── Exit (900)            [Alt+F4]

View (200)
├── New Visualizer        [Ctrl+N]   → CreateVisualizerEvent
├── Fullscreen                       → ToggleFullscreenEvent
├── Panels / Perspectives            → vom DockManager befüllt
├── Reset Layout                     → ResetLayoutEvent
└── Save Default Layout              → SaveDefaultLayoutEvent

Settings (300)
└── Frame Mode (exklusiv)
    ├── Limited / Unlimited / VSync  → FrameModeChangedEvent{0|1|2}

Help (900)
└── About MyViz... (F1)              → OpenDialogEvent{"about"}
```

Menü-Items lösen keine direkten Aktionen aus, sondern publizieren Events
(siehe [Event_System.md](Event_System.md)).

---

## 4. Regeln für neue Komponenten

1. Klasse erstellen (PanelBase / QDialog / IVisualizer / QWidget).
2. In der passenden `*AutoReg.cpp` registrieren (Header inkludieren, Eintrag in
   `initXxxDefaults()`).
3. Neue Quelldateien in die jeweilige `Source.cmake` eintragen
   (`sources.mode: explicit` — kein Globbing).
4. IDs: lowercase, eindeutig, stabil (Layout-Persistenz!); Order in 100er-Schritten.

---

## Siehe auch

- Modul-Doku: [`Registries.md`](../../include/services/Registries.md) (Pattern, Descriptors, Beispiele)
- [Event_System.md](Event_System.md) — Events, die Registries-Komponenten publizieren/abonnieren
- [Bootstrap_Integration.md](Bootstrap_Integration.md) — wann welche Registry initialisiert wird

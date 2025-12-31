# DockManager — Qt-ADS Docking Integration

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** MyViz::UI::DockManager  
> **Dateien:** DockManager.hpp, DockManager.cpp, PanelManager.hpp, PanelManager.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt-ADS 4.4.x, Qt6::Widgets, ServiceContainer, EventBus, PanelRegistry, WidgetRegistry  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Architektur](#2-architektur)
3. [API](#3-api)
4. [Event-Integration](#4-event-integration)
5. [Layout-Persistence](#5-layout-persistence)
6. [PanelManager](#6-panelmanager)
7. [Konfiguration](#7-konfiguration)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

DockManager kapselt Qt-ADS (Advanced Docking System) und ermöglicht:
- Mehrere Visualizer-Panels mit Tabs
- Dock-Panels aus PanelRegistry
- Drag & Drop Anordnung
- Floating Windows
- Layout speichern/laden mit Versionierung
- **Dezentrale Event-Handler** (keine MainWindow-Änderungen nötig)

### 1.2 Komponenten

```
┌──────────────────────────────────────────────────────────────────────────┐
│                           DockManager                                     │
├──────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────┐    ┌─────────────────────┐                      │
│  │  ads::CDockManager  │    │    PanelManager     │                      │
│  │  (Qt-ADS Widget)    │    │  (Panel-Erstellung) │                      │
│  └─────────────────────┘    └─────────────────────┘                      │
│                                                                           │
│  Events empfangen:                                                        │
│  • CreateVisualizerEvent → createVisualizer()                            │
│  • ResetLayoutEvent → resetLayout()                                       │
│  • ChangeVisualizerEvent → setVisualizer()                               │
│  • TogglePanelEvent → togglePanel()                                       │
│  • SaveDefaultLayoutEvent → saveDefaultLayout()                          │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Architektur

### 2.1 Dezentrale Events

DockManager empfängt Events direkt - **MainWindow muss nicht geändert werden**:

```
┌──────────────────────────────────────────────────────────────────────────┐
│                              EventBus                                     │
├──────────────────────────────────────────────────────────────────────────┤
│  CreateVisualizerEvent ──► DockManager::subscribeToEvents()              │
│  ResetLayoutEvent      ──► DockManager::subscribeToEvents()              │
│  ChangeVisualizerEvent ──► DockManager::subscribeToEvents()              │
│  TogglePanelEvent      ──► DockManager (via PanelManager)                │
│  SaveDefaultLayoutEvent──► DockManager::subscribeToEvents()              │
└──────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt-ADS 4.4.x | Extern | Advanced Docking System |
| Qt6::Widgets | Extern | QMainWindow, QMenu |
| ServiceContainer | Intern | Dependency Injection |
| EventBus | Intern | Event-Handling |
| PanelRegistry | Intern | Panel-Descriptors + Factories |
| WidgetRegistry | Intern | allowMultiple Check |
| VisualizerWidget | Intern | OpenGL Visualizer |

---

## 3. API

### 3.1 DockPosition Enum

```cpp
enum class DockPosition
{
    Center,     // Tabs (wenn belegt)
    Left,
    Right,
    Top,
    Bottom,
    Floating    // Schwebendes Fenster
};
```

### 3.2 Konstruktor

```cpp
explicit DockManager(ServiceContainer& services, QMainWindow* pMainWindow);
```

**Wichtig:** Layout wird NICHT im Konstruktor restored! MainWindow muss `restoreLayout()` aufrufen NACHDEM alle Widgets erstellt wurden.

### 3.3 Visualizer-Erstellung

| Methode | Beschreibung |
|---------|--------------|
| `createVisualizer(title, pos)` | Neuer Visualizer an Position |
| `createVisualizerRelativeTo(title, pos, ref)` | Relativ zu anderem Widget |
| `visualizers()` | Liste aller Visualizer |

**allowMultiple Check:**
```cpp
// Intern geprüft bei CreateVisualizerEvent:
const auto* desc = WidgetRegistry::instance().descriptor("visualizer");
if (desc && !desc->allowMultiple && !m_impl->visualizers.empty()) {
    return;  // Nicht erstellen - existiert bereits
}
```

### 3.4 Layout-Management

| Methode | Beschreibung |
|---------|--------------|
| `restoreLayout()` | **Public** - Stellt Layout wieder her oder wendet Defaults an |
| `resetLayout()` | Setzt auf Default-Layout zurück |
| `saveDefaultLayout()` | Speichert aktuelles Layout als Default |
| `savePerspective(name)` | Speichert benanntes Layout |
| `loadPerspective(name)` | Lädt benanntes Layout |
| `perspectiveNames()` | Liste gespeicherter Perspectives |

### 3.5 Panel-Management

| Methode | Beschreibung |
|---------|--------------|
| `populatePanelsMenu(menu)` | Befüllt Menü mit Panel-Toggles |
| `populatePerspectivesMenu(menu)` | Befüllt Menü mit Perspectives |

---

## 4. Event-Integration

### 4.1 Event-Subscription

```cpp
void DockManager::subscribeToEvents()
{
    auto* eventBus = m_impl->pServices->tryResolve<IEventBus>();
    if (!eventBus) return;
    
    // Create Visualizer (mit allowMultiple Check)
    m_impl->subscriptionIds.push_back(
        eventBus->subscribe<CreateVisualizerEvent>(
            [this](const CreateVisualizerEvent& e) {
                const auto* desc = WidgetRegistry::instance().descriptor("visualizer");
                if (desc && !desc->allowMultiple && !m_impl->visualizers.empty()) {
                    return;
                }
                createVisualizer(QString::fromStdString(e.title), DockPosition::Center);
            }));
    
    // Reset Layout
    m_impl->subscriptionIds.push_back(
        eventBus->subscribe<ResetLayoutEvent>(
            [this](const ResetLayoutEvent&) { resetLayout(); }));
    
    // Toggle Panel
    m_impl->subscriptionIds.push_back(
        eventBus->subscribe<TogglePanelEvent>(
            [this](const TogglePanelEvent& e) {
                m_impl->pPanelManager->togglePanel(QString::fromStdString(e.panelId));
            }));
    
    // Change Visualizer
    m_impl->subscriptionIds.push_back(
        eventBus->subscribe<ChangeVisualizerEvent>(
            [this](const ChangeVisualizerEvent& e) {
                if (!m_impl->visualizers.empty()) {
                    m_impl->visualizers[0]->setVisualizer(QString::fromStdString(e.visualizerId));
                }
            }));
}
```

### 4.2 Cleanup

```cpp
DockManager::~DockManager()
{
    unsubscribeFromEvents();  // Subscriptions aufräumen
}
```

---

## 5. Layout-Persistence

### 5.1 Kritische Reihenfolge

```cpp
// RICHTIG:
// 1. DockManager erstellen (Panels werden erstellt)
m_pDockManager = std::make_unique<DockManager>(*m_pServices, this);

// 2. Visualizer erstellen
auto* pViz = m_pDockManager->createVisualizer(...);

// 3. JETZT Layout wiederherstellen
m_pDockManager->restoreLayout();  // Alle Widgets existieren!
```

### 5.2 ObjectName-Stabilität

Qt-ADS identifiziert Widgets über `objectName()`. Der Titel kann sich ändern, aber objectName muss stabil bleiben:

```cpp
// In createVisualizer():
QString objectName = (vizNumber == 1)
    ? QStringLiteral("visualizer")
    : QStringLiteral("visualizer_%1").arg(vizNumber);
pDock->setObjectName(objectName);  // ← Stabil!
pDock->setWindowTitle(dynamicTitle);  // ← Kann sich ändern
```

| Widget | objectName |
|--------|------------|
| Player Panel | `player` |
| Playlist Panel | `playlist` |
| Settings Panel | `config` |
| Visualizers Panel | `visual_select` |
| Visualizer 1 | `visualizer` |
| Visualizer N | `visualizer_N` |

### 5.3 aboutToQuit Signal

Layout wird via `aboutToQuit` gespeichert (nicht im Destruktor!):

```cpp
// Im Konstruktor:
connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
    saveLayoutToSettings();  // VOR Zerstörung!
});
```

### 5.4 Versionierung

```cpp
namespace {
    constexpr int LAYOUT_VERSION = 4;  // Bei Struktur-Änderung erhöhen!
}

bool DockManager::restoreLayoutFromSettings()
{
    QSettings settings;
    settings.beginGroup("DockManager");
    
    int savedVersion = settings.value("Version", 0).toInt();
    if (savedVersion != LAYOUT_VERSION) {
        settings.remove("");  // Alte Settings löschen
        return false;
    }
    // ...
}
```

---

## 6. PanelManager

### 6.1 Zweck

PanelManager verwaltet Panel-Instanzen aus PanelRegistry:

```cpp
class PanelManager : public QObject
{
public:
    void createAllPanels();           // Erstellt alle registrierten Panels
    void applyDefaultVisibility();    // Wendet defaultVisible an
    
    bool isPanelVisible(const QString& panelId) const;
    void showPanel(const QString& panelId);
    void hidePanel(const QString& panelId);
    void togglePanel(const QString& panelId);
};
```

### 6.2 Wichtig: Zwei-Phasen-Initialisierung

```cpp
// Phase 1: Panels erstellen (alle sichtbar)
void PanelManager::createAllPanels()
{
    // NUR erstellen, KEINE Sichtbarkeit setzen!
}

// Phase 2: Default-Visibility anwenden (NUR wenn kein Layout restored)
void PanelManager::applyDefaultVisibility()
{
    for (const auto& desc : descriptors) {
        if (!desc.defaultVisible) {
            m_dockWidgets[desc.id]->closeDockWidget();
        }
    }
}
```

---

## 7. Konfiguration

### 7.1 Qt-ADS Config Flags

```cpp
ads::CDockManager::setConfigFlags(
    DefaultOpaqueConfig |
    DockAreaHasTabsMenuButton |
    DockAreaHasUndockButton |
    DockAreaCloseButtonClosesTab |
    TabCloseButtonIsToolButton |
    AllTabsHaveCloseButton |
    OpaqueSplitterResize |
    FocusHighlighting
);
```

### 7.2 Auto-Hide Config

```cpp
ads::CDockManager::setAutoHideConfigFlags(
    DefaultAutoHideConfig |
    AutoHideFeatureEnabled |
    DockAreaHasAutoHideButton
);
```

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2025-12-31** | **+PanelManager, +dezentrale Events, +Layout-Persistence Fix (aboutToQuit, objectName), +allowMultiple Check, +restoreLayout() public** |
| 1.0.0 | 2025-12-28 | Initial: Qt-ADS Integration, Multi-Visualizer, Perspectives |

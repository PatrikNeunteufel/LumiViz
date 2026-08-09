# WidgetBase — Template Base for Qt Widgets

> **Version:** 2.0.0  
> **Datum:** 2025-12-31  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::UI::Widgets  
> **Dateien:** IWidget.hpp, WidgetBase.hpp, WidgetBase.tpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6, ServiceContainer  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Template-Architektur](#2-template-architektur)
3. [API](#3-api)
4. [Widget erstellen](#4-widget-erstellen)
5. [Unterstützte Basisklassen](#5-unterstützte-basisklassen)
6. [Changelog](#6-changelog)

---

## 1. Übersicht

### 1.1 Zweck

**WidgetBase** ist eine **Template-Basisklasse**, die einheitlichen Zugriff auf ServiceContainer und EventBus für verschiedene Qt-Widget-Typen bietet.

### 1.2 Warum Template?

```cpp
// ❌ Ohne Template: Jede Basisklasse braucht eigene Implementierung
class StandardWidgetBase : public QWidget, public IWidget { ... };
class OpenGLWidgetBase : public QOpenGLWidget, public IWidget { ... };  // Duplikat!
class FrameWidgetBase : public QFrame, public IWidget { ... };          // Duplikat!

// ✅ Mit Template: Eine Implementierung für alle
template<typename BaseWidget>
class WidgetBase : public BaseWidget, public IWidget { ... };

// Verwendung:
class VolumeWidget : public WidgetBase<QWidget> { ... };
class VisualizerWidget : public WidgetBase<QOpenGLWidget> { ... };
class FramedWidget : public WidgetBase<QFrame> { ... };
```

---

## 2. Template-Architektur

### 2.1 Klassendiagramm

```
                    ┌─────────────────────┐
                    │      IWidget        │  (Interface)
                    ├─────────────────────┤
                    │ + widgetId()        │
                    │ + widgetName()      │
                    │ + startUpdates()    │
                    │ + stopUpdates()     │
                    └──────────┬──────────┘
                               │
                               ▼
    ┌──────────────────────────────────────────────────────────┐
    │            WidgetBase<BaseWidget>                         │
    │  (Template: BaseWidget = QWidget, QOpenGLWidget, ...)    │
    ├──────────────────────────────────────────────────────────┤
    │ # m_services : ServiceContainer&                          │
    │ # m_widgetId : QString                                    │
    │ # m_isUpdating : bool                                     │
    ├──────────────────────────────────────────────────────────┤
    │ # services() : ServiceContainer&                          │
    │ # eventBus() : IEventBus*                                 │
    │ # onStartUpdates() [virtual]                              │
    │ # onStopUpdates() [virtual]                               │
    └────────────────────────┬─────────────────────────────────┘
                             │
       ┌─────────────────────┼─────────────────────┐
       ▼                     ▼                     ▼
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│WidgetBase    │     │WidgetBase    │     │WidgetBase    │
│ <QWidget>    │     │<QOpenGLWidget>│    │  <QFrame>    │
└──────┬───────┘     └──────┬───────┘     └──────┬───────┘
       │                    │                    │
       ▼                    ▼                    ▼
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│ VolumeWidget │     │ Visualizer   │     │ StatusWidget │
│              │     │ Widget       │     │              │
└──────────────┘     └──────────────┘     └──────────────┘
```

### 2.2 Type Aliases

```cpp
// Vordefinierte Aliases
using StandardWidgetBase = WidgetBase<QWidget>;

// Eigene Aliases erstellen:
#include <QOpenGLWidget>
using OpenGLWidgetBase = WidgetBase<QOpenGLWidget>;

#include <QFrame>
using FrameWidgetBase = WidgetBase<QFrame>;
```

---

## 3. API

### 3.1 Konstruktor

```cpp
WidgetBase(ServiceContainer& services,
           const QString& id,
           const QString& name,
           QWidget* parent = nullptr);
```

### 3.2 IWidget-Methoden

```cpp
[[nodiscard]] QString widgetId() const;   // Unique ID
[[nodiscard]] QString widgetName() const; // Display name
void startUpdates();                       // Start animations/timers
void stopUpdates();                        // Stop animations/timers
```

### 3.3 Protected Helpers

```cpp
ServiceContainer& services();              // Container-Zugriff
IEventBus* eventBus();                     // EventBus-Zugriff

virtual void onStartUpdates() {}           // Override für Start-Logik
virtual void onStopUpdates() {}            // Override für Stop-Logik
```

### 3.4 Auto Start/Stop

```cpp
// Automatisch bei Sichtbarkeit:
void showEvent(QShowEvent*) override {
    startUpdates();  // → onStartUpdates()
}

void hideEvent(QHideEvent*) override {
    stopUpdates();   // → onStopUpdates()
}
```

---

## 4. Widget erstellen

### 4.1 Standard-Widget

```cpp
// VolumeWidget.hpp
#pragma once
#include "UI/widgets/WidgetBase.hpp"

class QSlider;

class VolumeWidget : public WidgetBase<QWidget>
{
    Q_OBJECT

public:
    explicit VolumeWidget(ServiceContainer& services, 
                          QWidget* parent = nullptr);

protected:
    void onStartUpdates() override;
    void onStopUpdates() override;

private:
    QSlider* m_pSlider = nullptr;
    IEventBus::SubscriberId m_volumeSubId = 0;
};
```

```cpp
// VolumeWidget.cpp
#include "VolumeWidget.hpp"
#include "services/IEventBus.hpp"
#include "audio/AudioEvents.hpp"

VolumeWidget::VolumeWidget(ServiceContainer& services, QWidget* parent)
    : WidgetBase(services, "volume", tr("Volume"), parent)
{
    m_pSlider = new QSlider(Qt::Horizontal, this);
    m_pSlider->setRange(0, 100);
}

void VolumeWidget::onStartUpdates()
{
    m_volumeSubId = eventBus()->subscribe<VolumeChangedEvent>(
        [this](const VolumeChangedEvent& e) {
            m_pSlider->setValue(static_cast<int>(e.volume * 100));
        });
}

void VolumeWidget::onStopUpdates()
{
    eventBus()->unsubscribe(m_volumeSubId);
}
```

### 4.2 OpenGL-Widget

```cpp
// VisualizerWidget.hpp
#pragma once
#include "UI/widgets/WidgetBase.hpp"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class VisualizerWidget : public WidgetBase<QOpenGLWidget>,
                         protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VisualizerWidget(ServiceContainer& services, 
                              QWidget* parent = nullptr);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void onStartUpdates() override;
    void onStopUpdates() override;

private:
    QTimer* m_pRenderTimer = nullptr;
};
```

---

## 5. Unterstützte Basisklassen

| Basisklasse | Verwendung | Beispiel |
|-------------|------------|----------|
| `QWidget` | Standard-Widgets | VolumeWidget, StatusWidget |
| `QOpenGLWidget` | OpenGL-Rendering | VisualizerWidget |
| `QFrame` | Widgets mit Rahmen | CardWidget |
| `QAbstractScrollArea` | Scrollbare Bereiche | LogWidget |
| `QGraphicsView` | 2D-Szenen | NodeEditorWidget |
| `QQuickWidget` | QML-Integration | QmlVisualizerWidget |

### 5.1 Template-Constraints

```cpp
// Das Template funktioniert mit allen Klassen die von QWidget erben
template<typename BaseWidget = QWidget>
class WidgetBase : public BaseWidget, public IWidget
{
    // BaseWidget muss:
    // - Von QWidget erben (direkt oder indirekt)
    // - showEvent/hideEvent haben
    // - parent-Konstruktor unterstützen
};
```

### 5.2 Q_OBJECT Einschränkung

```cpp
// ⚠️ WICHTIG: Q_OBJECT kann nicht in Templates verwendet werden!
// Jede abgeleitete Klasse muss Q_OBJECT selbst hinzufügen:

template<typename T>
class WidgetBase : public T, public IWidget
{
    // KEIN Q_OBJECT hier!
};

class MyWidget : public WidgetBase<QWidget>
{
    Q_OBJECT  // HIER muss es sein!
public:
    // ...
};
```

---

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **2.0.0** | **2025-12-31** | **Template-Architektur für multiple Basisklassen** |
| 1.0.0 | 2025-12-28 | Initial: QWidget-only |

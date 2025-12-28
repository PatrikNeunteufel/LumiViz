# MainWindow — Hauptfenster der Anwendung

> **Version:** 1.0.0  
> **Datum:** 2025-12-28  
> **Typ:** CppModuleDoc  
> **Status:** In Entwicklung  
> **Modul:** MyViz::UI::MainWindow  
> **Dateien:** MainWindow.hpp, MainWindow.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Qt6::Widgets (QMainWindow, QWidget)  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Thread-Sicherheit](#5-thread-sicherheit)
6. [Fehlerbehandlung](#6-fehlerbehandlung)
7. [Qt6-Konzepte](#7-qt6-konzepte)
8. [Changelog](#8-changelog)

---

## 1. Übersicht

### 1.1 Zweck

MainWindow ist das Hauptfenster der MyViz-Anwendung. Es erbt von `QMainWindow` und stellt den Container für alle UI-Elemente bereit.

### 1.2 Verantwortlichkeiten

- Fenster-Konfiguration (Titel, Größe)
- Hosting des Central Widget
- (Zukünftig: Menü, Toolbar, StatusBar)

### 1.3 Nicht-Verantwortlichkeiten

- Anwendungs-Lifecycle (→ Application)
- Audio-Verarbeitung (→ AudioEngine)
- Visualisierung (→ VisualizerWidget)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| Qt6::Widgets | Extern | QMainWindow, QWidget Basisklassen |
| Qt6::Gui | Extern | GUI-Grundfunktionen |
| Qt6::Core | Extern | Qt Kernsystem (Q_OBJECT, Signals/Slots) |

---

## 3. API

### 3.1 Konstruktion
```cpp
explicit MainWindow(QWidget* parent = nullptr);
~MainWindow() override;
```

| Parameter | Typ | Default | Beschreibung |
|-----------|-----|---------|--------------|
| `parent` | `QWidget*` | `nullptr` | Parent-Widget für Ownership |

### 3.2 Öffentliche Methoden

Aktuell keine zusätzlichen öffentlichen Methoden. Alle Funktionalität wird über geerbte QMainWindow-Methoden bereitgestellt.

### 3.3 Geerbte Methoden (wichtigste)

| Methode | Quelle | Beschreibung |
|---------|--------|--------------|
| `show()` | QWidget | Zeigt das Fenster an |
| `hide()` | QWidget | Versteckt das Fenster |
| `close()` | QWidget | Schließt das Fenster |
| `resize(w, h)` | QWidget | Ändert Fenstergröße |
| `setWindowTitle(title)` | QWidget | Setzt Fenstertitel |

---

## 4. Verwendung

### 4.1 Einfaches Beispiel
```cpp
#include "UI/MainWindow.hpp"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

### 4.2 Mit Application-Klasse
```cpp
// In Application::init()
m_pMainWindow = std::make_unique<MainWindow>();
m_pMainWindow->show();

// In Application::run()
return m_pQtApp->exec();
```

---

## 5. Thread-Sicherheit

**Nicht thread-safe.** 

Alle Qt-Widget-Operationen müssen vom Main-Thread (GUI-Thread) erfolgen. Dies ist eine Qt-Grundregel.

---

## 6. Fehlerbehandlung

- Keine Exceptions
- Qt-Widgets melden Fehler über Return-Werte oder Signals
- Bei kritischen Fehlern: `qFatal()` oder `Q_ASSERT()`

---

## 7. Qt6-Konzepte

### 7.1 Q_OBJECT Macro

Das `Q_OBJECT` Macro ist erforderlich für:
- Signals und Slots
- Meta-Object System (`qobject_cast`)
- Property System
- Übersetzungen (`tr()`)

### 7.2 Parent-Child Ownership

Qt verwendet ein Parent-Child-Modell für Memory Management:
- Parent besitzt seine Children
- Beim Löschen des Parents werden Children automatisch gelöscht
- Top-Level-Fenster (parent = nullptr) werden bei QApplication-Ende gelöscht

### 7.3 QMainWindow Layout
```
+------------------------------------------+
|              Menu Bar                    |
+------------------------------------------+
|              Tool Bar                    |
+------+---------------------------+-------+
| Dock |      Central Widget       | Dock  |
+------+---------------------------+-------+
|              Status Bar                  |
+------------------------------------------+
```

---

## 8. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2025-12-28** | **Initial: Leeres Fenster ohne Menü** |
# Qt6 — UserGuide

> **Version:** 1.0.0  
> **Datum:** 2025-12-15  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** Alle Entwickler  
> **Sprache:** Deutsch  
> **English:** [qt6.md](../../../en/userguides/externals/Qt6.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Installation](#2-installation)
3. [Solution.json Konfiguration](#3-solutionjson-konfiguration)
4. [C++ Verwendung](#4-c-verwendung)
5. [Qt Widgets](#5-qt-widgets)
6. [Signals & Slots](#6-signals--slots)
7. [Qt Quick / QML](#7-qt-quick--qml)
8. [Fortgeschrittene Techniken](#8-fortgeschrittene-techniken)
9. [Troubleshooting](#9-troubleshooting)
10. [Weiterführende Informationen](#10-weiterführende-informationen)
11. [Changelog](#11-changelog)

---

## 1. Übersicht

**Qt6** ist ein umfassendes C++ Framework für Desktop-, Mobile- und Embedded-Anwendungen.

| Aspekt | Wert |
|--------|------|
| **Typ** | System External |
| **Version** | 6.x (empfohlen: 6.6+) |
| **Lizenz** | LGPL / Commercial |
| **Website** | [qt.io](https://www.qt.io/) |

### Warum Qt6?

| Vorteil | Beschreibung |
|---------|--------------|
| 🖥️ **Cross-Platform** | Windows, Linux, macOS, Mobile |
| 🎨 **Widgets & QML** | Classic und Modern UI |
| 📡 **Networking** | HTTP, WebSocket, Bluetooth |
| 💾 **Datenbanken** | SQL, SQLite, PostgreSQL |

### Qt6 Module

| Modul | Beschreibung |
|-------|--------------|
| **Core** | Basis-Funktionalität |
| **Gui** | GUI-Grundlagen |
| **Widgets** | Desktop-Widgets |
| **Quick** | QML/JavaScript UI |
| **Network** | Netzwerk-Funktionen |
| **Sql** | Datenbank-Zugriff |
| **Multimedia** | Audio/Video |

---

## 2. Installation

### 2.1 Qt Online Installer

1. Download von [qt.io/download](https://www.qt.io/download)
2. Qt Online Installer ausführen
3. Qt 6.x und benötigte Module auswählen
4. Compiler-Kit auswählen (MSVC, MinGW, GCC, Clang)

### 2.2 Paketmanager (Linux)

```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-declarative-dev

# Fedora
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel

# Arch
sudo pacman -S qt6-base qt6-declarative
```

### 2.3 Homebrew (macOS)

```bash
brew install qt@6
```

### 2.4 Installationspfade

| Platform | Typischer Pfad |
|----------|----------------|
| **Windows** | `C:\Qt\6.6.0\msvc2019_64` |
| **Linux** | `/usr/lib/qt6` oder `~/Qt/6.6.0/gcc_64` |
| **macOS** | `/usr/local/opt/qt@6` oder `~/Qt/6.6.0/macos` |

---

## 3. Solution.json Konfiguration

### 3.1 Minimal (Widgets)

```json
{
    "externals": {
        "qt6": {
            "system": true,
            "components": ["Widgets"]
        }
    },
    "executables": [
        {
            "name": "QtApp",
            "type": "GUI",
            "externals": ["qt6"]
        }
    ]
}
```

### 3.2 Mit mehreren Komponenten

```json
{
    "externals": {
        "qt6": {
            "system": true,
            "components": ["Widgets", "Network", "Sql"]
        }
    },
    "executables": [
        {
            "name": "QtApp",
            "type": "GUI",
            "externals": ["qt6"]
        }
    ]
}
```

### 3.3 QML/Quick Anwendung

```json
{
    "externals": {
        "qt6": {
            "system": true,
            "components": ["Quick", "QuickControls2", "Qml"]
        }
    },
    "executables": [
        {
            "name": "QmlApp",
            "type": "GUI",
            "externals": ["qt6"]
        }
    ]
}
```

### 3.4 CMake-Integration

Qt6 benötigt `CMAKE_PREFIX_PATH`:

```bash
# CMake konfigurieren
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.6.0/gcc_64 ..

# Oder in CMakeUserPresets.json
{
    "configurePresets": [
        {
            "name": "qt6",
            "cacheVariables": {
                "CMAKE_PREFIX_PATH": "/path/to/Qt/6.6.0/gcc_64"
            }
        }
    ]
}
```

---

## 4. C++ Verwendung

### 4.1 Minimale Qt Application

```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QLabel label("Hello, Qt6!");
    label.setWindowTitle("My First Qt App");
    label.resize(200, 100);
    label.show();
    
    return app.exec();
}
```

### 4.2 Mit MainWindow

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.setWindowTitle("Qt MainWindow");
    window.resize(400, 300);
    
    QWidget* central = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(central);
    
    QLabel* label = new QLabel("Hello, Qt6!");
    QPushButton* button = new QPushButton("Click me");
    
    layout->addWidget(label);
    layout->addWidget(button);
    
    window.setCentralWidget(central);
    window.show();
    
    return app.exec();
}
```

### 4.3 Eigene Widget-Klasse

**mainwindow.h:**
```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QPushButton;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
    
private slots:
    void onButtonClicked();
    
private:
    QLabel* label;
    QPushButton* button;
    int clickCount = 0;
};

#endif
```

**mainwindow.cpp:**
```cpp
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    
    setWindowTitle("My Application");
    resize(400, 300);
    
    QWidget* central = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(central);
    
    label = new QLabel("Click count: 0");
    button = new QPushButton("Click me");
    
    layout->addWidget(label);
    layout->addWidget(button);
    
    setCentralWidget(central);
    
    connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
}

void MainWindow::onButtonClicked() {
    clickCount++;
    label->setText(QString("Click count: %1").arg(clickCount));
}
```

**main.cpp:**
```cpp
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

---

## 5. Qt Widgets

### 5.1 Häufige Widgets

| Widget | Beschreibung |
|--------|--------------|
| `QLabel` | Text/Bild anzeigen |
| `QPushButton` | Button |
| `QLineEdit` | Einzeilige Texteingabe |
| `QTextEdit` | Mehrzeilige Texteingabe |
| `QCheckBox` | Checkbox |
| `QRadioButton` | Radio Button |
| `QComboBox` | Dropdown |
| `QSlider` | Schieberegler |
| `QSpinBox` | Zahleneingabe |
| `QProgressBar` | Fortschrittsbalken |
| `QListWidget` | Liste |
| `QTableWidget` | Tabelle |
| `QTreeWidget` | Baumansicht |

### 5.2 Layouts

```cpp
// Vertikal
QVBoxLayout* vbox = new QVBoxLayout();
vbox->addWidget(widget1);
vbox->addWidget(widget2);

// Horizontal
QHBoxLayout* hbox = new QHBoxLayout();
hbox->addWidget(widget1);
hbox->addWidget(widget2);

// Grid
QGridLayout* grid = new QGridLayout();
grid->addWidget(widget1, 0, 0);  // Zeile 0, Spalte 0
grid->addWidget(widget2, 0, 1);  // Zeile 0, Spalte 1
grid->addWidget(widget3, 1, 0, 1, 2);  // Zeile 1, 2 Spalten breit

// Form
QFormLayout* form = new QFormLayout();
form->addRow("Name:", nameEdit);
form->addRow("Email:", emailEdit);
```

### 5.3 Dialoge

```cpp
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>

// Message Box
QMessageBox::information(this, "Info", "Operation completed");
QMessageBox::warning(this, "Warning", "Something went wrong");

int result = QMessageBox::question(this, "Confirm", "Are you sure?",
    QMessageBox::Yes | QMessageBox::No);
if (result == QMessageBox::Yes) {
    // Ja geklickt
}

// File Dialog
QString filename = QFileDialog::getOpenFileName(this,
    "Open File", "", "Text Files (*.txt);;All Files (*)");

QString saveFile = QFileDialog::getSaveFileName(this,
    "Save File", "", "Text Files (*.txt)");

// Input Dialog
QString text = QInputDialog::getText(this, "Input", "Enter name:");
int number = QInputDialog::getInt(this, "Input", "Enter number:", 0, 0, 100);

// Color Dialog
QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");
```

### 5.4 Menüs und Toolbars

```cpp
// Menübar
QMenuBar* menuBar = this->menuBar();

QMenu* fileMenu = menuBar->addMenu("&File");
QAction* newAction = fileMenu->addAction("&New", QKeySequence::New);
QAction* openAction = fileMenu->addAction("&Open", QKeySequence::Open);
fileMenu->addSeparator();
QAction* exitAction = fileMenu->addAction("E&xit", QKeySequence::Quit);

connect(exitAction, &QAction::triggered, this, &QWidget::close);

// Toolbar
QToolBar* toolbar = addToolBar("Main");
toolbar->addAction(newAction);
toolbar->addAction(openAction);

// Statusbar
statusBar()->showMessage("Ready");
```

---

## 6. Signals & Slots

### 6.1 Grundlagen

```cpp
// Verbindung herstellen
connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);

// Beispiel
connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
```

### 6.2 Lambda als Slot

```cpp
connect(button, &QPushButton::clicked, [this]() {
    label->setText("Button clicked!");
});

// Mit Parameter
connect(slider, &QSlider::valueChanged, [this](int value) {
    label->setText(QString("Value: %1").arg(value));
});
```

### 6.3 Eigene Signals & Slots

```cpp
class Counter : public QObject {
    Q_OBJECT
    
public:
    Counter() : count(0) {}
    
    int value() const { return count; }
    
public slots:
    void increment() {
        count++;
        emit valueChanged(count);
    }
    
    void decrement() {
        count--;
        emit valueChanged(count);
    }
    
    void setValue(int value) {
        if (count != value) {
            count = value;
            emit valueChanged(count);
        }
    }
    
signals:
    void valueChanged(int newValue);
    
private:
    int count;
};

// Verwendung
Counter counter;
connect(&counter, &Counter::valueChanged, [](int value) {
    qDebug() << "Value changed to:" << value;
});

counter.increment();  // Ausgabe: "Value changed to: 1"
```

---

## 7. Qt Quick / QML

### 7.1 QML-Grundlagen

**main.qml:**
```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 400
    height: 300
    title: "QML App"
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Label {
            text: "Hello, QML!"
            font.pixelSize: 24
        }
        
        Button {
            text: "Click me"
            onClicked: {
                console.log("Button clicked!")
            }
        }
    }
}
```

**main.cpp:**
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}
```

### 7.2 C++ Backend für QML

**backend.h:**
```cpp
#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    
public:
    explicit Backend(QObject* parent = nullptr);
    
    QString message() const;
    void setMessage(const QString& msg);
    
    Q_INVOKABLE void processData();
    
signals:
    void messageChanged();
    
private:
    QString m_message;
};

#endif
```

**main.cpp:**
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "backend.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    
    Backend backend;
    
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    
    Column {
        Label {
            text: backend.message
        }
        
        Button {
            text: "Update"
            onClicked: backend.processData()
        }
    }
}
```

---

## 8. Fortgeschrittene Techniken

### 8.1 Threading

```cpp
#include <QThread>
#include <QObject>

class Worker : public QObject {
    Q_OBJECT
    
public slots:
    void doWork() {
        // Lange Operation
        for (int i = 0; i < 100; i++) {
            QThread::msleep(50);
            emit progress(i);
        }
        emit finished();
    }
    
signals:
    void progress(int value);
    void finished();
};

// Verwendung
QThread* thread = new QThread();
Worker* worker = new Worker();
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &Worker::doWork);
connect(worker, &Worker::finished, thread, &QThread::quit);
connect(worker, &Worker::finished, worker, &QObject::deleteLater);
connect(thread, &QThread::finished, thread, &QObject::deleteLater);

connect(worker, &Worker::progress, [](int value) {
    qDebug() << "Progress:" << value;
});

thread->start();
```

### 8.2 Netzwerk

```cpp
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

QNetworkAccessManager* manager = new QNetworkAccessManager(this);

connect(manager, &QNetworkAccessManager::finished, [](QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Response:" << data;
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();
});

// GET Request
manager->get(QNetworkRequest(QUrl("https://api.example.com/data")));

// POST Request
QNetworkRequest request(QUrl("https://api.example.com/data"));
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
manager->post(request, "{\"key\": \"value\"}");
```

### 8.3 Datenbank

```cpp
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// Verbindung
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("mydb.sqlite");

if (!db.open()) {
    qDebug() << "Error:" << db.lastError().text();
    return;
}

// Tabelle erstellen
QSqlQuery query;
query.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");

// Einfügen
query.prepare("INSERT INTO users (name) VALUES (:name)");
query.bindValue(":name", "John");
query.exec();

// Abfragen
query.exec("SELECT * FROM users");
while (query.next()) {
    int id = query.value(0).toInt();
    QString name = query.value(1).toString();
    qDebug() << id << name;
}

db.close();
```

### 8.4 Ressourcen-System

**resources.qrc:**
```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/">
        <file>images/logo.png</file>
        <file>qml/main.qml</file>
        <file>styles/app.css</file>
    </qresource>
</RCC>
```

```cpp
// Zugriff
QPixmap pixmap(":/images/logo.png");
QFile file(":/styles/app.css");
```

---

## 9. Troubleshooting

### 9.1 "Qt6 not found"

**Problem:** CMake findet Qt6 nicht

**Lösung:** `CMAKE_PREFIX_PATH` setzen:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.6.0/gcc_64 ..
```

### 9.2 MOC nicht ausgeführt

**Problem:** Signals/Slots funktionieren nicht

**Lösung:** 
- `Q_OBJECT` Makro in Klasse
- Header in CMakeLists.txt auflisten
- `AUTOMOC ON` in CMake

### 9.3 "undefined reference to vtable"

**Problem:** Link-Fehler bei Q_OBJECT-Klassen

**Lösung:** CMake neu konfigurieren (MOC muss laufen).

### 9.4 QML-Dateien nicht gefunden

**Problem:** QML-Dateien werden nicht geladen

**Lösung:** 
- In .qrc einbinden
- `qrc:/` Prefix verwenden

---

## 10. Weiterführende Informationen

### Offizielle Ressourcen

| Ressource | Link |
|-----------|------|
| **Website** | [qt.io](https://www.qt.io/) |
| **Dokumentation** | [doc.qt.io](https://doc.qt.io/) |
| **Qt6 Docs** | [doc.qt.io/qt-6](https://doc.qt.io/qt-6/) |
| **Tutorials** | [doc.qt.io/qt-6/qtexamplesandtutorials.html](https://doc.qt.io/qt-6/qtexamplesandtutorials.html) |
| **Qt Forum** | [forum.qt.io](https://forum.qt.io/) |
| **GitHub** | [github.com/qt](https://github.com/qt) |

### Bücher & Kurse

| Ressource | Beschreibung |
|-----------|--------------|
| **Qt Wiki** | [wiki.qt.io](https://wiki.qt.io/) |
| **Qt Examples** | Mitgelieferte Beispiele |

### Siehe auch

- [Externals.md](../Externals.md) — Externals Kombinationen
- [Git_Externals_GUI.md](../../references/externals/Git_Externals_GUI.md) — Alternative: GLFW + ImGui

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.2** | **2025-12-15** | **Initial: Detaillierter UserGuide für Qt6** |

# Qt6 Integration – Benutzerhandbuch

> **Version:** 1.0.0  
> **Datum:** 2025-12-14  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Modul:** externals/qt6/Include.cmake v0.6.0  
> **Sprache:** Deutsch  
> **English:** [Qt6_Integration_UserGuide.md](../../en/userguides/Qt6_Integration.md)

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Installation von Qt6](#4-installation-von-qt6)
5. [Konfiguration](#5-konfiguration)
6. [Umgebungsvariablen](#6-umgebungsvariablen)
7. [Beispielprojekt](#7-beispielprojekt)
8. [Plattform-spezifische Hinweise](#8-plattform-spezifische-hinweise)
9. [Verfügbare Komponenten](#9-verfügbare-komponenten)
10. [Stolpersteine und Lösungen](#10-stolpersteine-und-lösungen)
11. [Troubleshooting](#11-troubleshooting)
12. [Siehe auch](#12-siehe-auch)

---

## 1. Überblick

Die Qt6-Integration ermöglicht die Verwendung von Qt6 in CMake Architecture Projekten.

### Features

- Flexible Pfad-Erkennung (Umgebungsvariablen, Hints, Auto-Detection)
- Backup-Pfade (USB-Stick, Netzlaufwerk)
- Automatisches DLL-Deployment (Windows)
- RPATH-Konfiguration (Linux/macOS)
- Komponentenauswahl

**Wichtig:** Qt6 ist ein **System External** – die Installation erfolgt außerhalb des Projekts.

### Unterstützte Plattformen

| Plattform | Compiler | Deployment |
|-----------|----------|------------|
| Windows | MSVC 2022, Clang | windeployqt (automatisch) |
| Linux | GCC, Clang | RPATH (automatisch) |
| macOS | Apple Clang | macdeployqt (für Bundles) |

---

## 2. Voraussetzungen

### Checkliste

- [ ] **Qt6 installiert** (Version 6.5 oder höher empfohlen)
- [ ] **CMake 3.24+** (für Qt6-Unterstützung)
- [ ] **Compiler:** MSVC 2022, GCC 11+, oder Clang 14+
- [ ] **CMake Architecture** eingerichtet
- [ ] **externals/qt6/Include.cmake** vorhanden (v0.6.0+)

### Empfohlene Versionen

| Komponente | Minimum | Empfohlen |
|------------|---------|-----------|
| Qt6 | 6.2 | 6.7+ |
| CMake | 3.21 | 3.28+ |
| Include.cmake | 0.5.0 | 0.6.0 |

---

## 3. Schnellstart

### 3.1 Minimale Solution.json

```json
{
    "externals": {
        "qt6": {
            "path": "externals/qt6",
            "options": {
                "hint": "${QT_ROOT}"
            }
        }
    },
    "executables": [
        {
            "name": "MyQtApp",
            "type": "GUI",
            "externals": ["qt6"]
        }
    ]
}
```

### 3.2 Build

```bash
# Konfigurieren
cmake --preset windows-ninja-debug

# Bauen
cmake --build out/build/windows-ninja-debug

# Ausführen
./out/build/windows-ninja-debug/bin/Debug/MyQtApp.exe
```

---

## 4. Installation von Qt6

### 4.1 Qt Online Installer (Empfohlen)

**Download:** https://www.qt.io/download-qt-installer

**Installationsschritte:**

1. Qt Maintenance Tool herunterladen und starten
2. Qt Account erstellen (kostenlos für Open Source)
3. Installation auswählen:
   - Qt → Qt 6.x.x → Desktop
   - Windows: MSVC 2022 64-bit
   - Linux: GCC 64-bit
   - macOS: macOS
4. Optional: Additional Libraries (OpenGL, Network, etc.)
5. Developer and Designer Tools → CMake, Ninja (optional)

**Typische Installationspfade:**

| Plattform | Pfad |
|-----------|------|
| Windows | `C:\Qt\6.10.1\msvc2022_64` |
| Linux | `~/Qt/6.10.1/gcc_64` |
| macOS | `~/Qt/6.10.1/macos` |

### 4.2 System-Pakete (Linux)

**Ubuntu/Debian:**
```bash
# Basis-Entwicklung
sudo apt install qt6-base-dev qt6-tools-dev

# Zusätzliche Module
sudo apt install qt6-multimedia-dev     # Multimedia
sudo apt install qt6-webengine-dev      # WebEngine
sudo apt install qt6-declarative-dev    # QML/Quick
```

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-tools
sudo pacman -S qt6-multimedia qt6-webengine  # Optional
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
```

**openSUSE:**
```bash
sudo zypper install qt6-base-devel qt6-tools-devel
```

### 4.3 Homebrew (macOS)

```bash
# Installation
brew install qt@6

# Pfad ermitteln
brew --prefix qt@6
# Ergebnis: /opt/homebrew/opt/qt@6 (Apple Silicon)
#           /usr/local/opt/qt@6 (Intel)
```

**Umgebungsvariable setzen:**
```bash
echo 'export QT_ROOT="$(brew --prefix qt@6)"' >> ~/.zshrc
source ~/.zshrc
```

### 4.4 Manuelle Installation

1. **Download:** https://download.qt.io/archive/qt/
2. Gewünschte Version wählen (z.B. 6.7.0)
3. Passende Architektur wählen:
   - Windows: `qt-everywhere-src-6.7.0.zip` oder Online Installer
   - Linux: `qt-everywhere-src-6.7.0.tar.xz`
4. Entpacken nach gewünschtem Pfad
5. Pfad als Umgebungsvariable setzen

---

## 5. Konfiguration

### 5.1 Pfad-Auflösung (Priorität)

Die Include.cmake sucht Qt6 in folgender Reihenfolge:

```
1. QT_ROOT Umgebungsvariable        ← Höchste Priorität
2. QT6_DIR Umgebungsvariable
3. CMAKE_PREFIX_PATH
4. hint aus Solution.json options
5. Standard-Pfade (Auto-Detection)
6. backup aus Solution.json options ← Mit WARNING
```

### 5.2 Minimale Konfiguration

```json
"externals": {
    "qt6": {
        "path": "externals/qt6"
    }
}
```

**Voraussetzung:** `QT_ROOT` oder `QT6_DIR` muss gesetzt sein.

### 5.3 Mit Pfad-Hint

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "C:/Qt/6.10.1/msvc2022_64"
    }
}
```

### 5.4 Mit Umgebungsvariable im Hint

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}"
    }
}
```

### 5.5 Mit Backup-Pfad

Für portable Installationen (USB-Stick, Netzlaufwerk):

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}",
        "backup": "E:/Backup/Qt/6.10.1/msvc2022_64"
    }
}
```

**Verhalten:**
- Backup wird NUR verwendet wenn alle anderen Pfade fehlschlagen
- Es erscheint eine **WARNING** (kein Fehler)
- Build läuft weiter

**Warning-Ausgabe:**
```
CMake Warning:
  [qt6] Primary Qt6 installation not found!
    Using BACKUP location: E:/Backup/Qt/6.10.1/msvc2022_64
    
    This may be slower (USB/network drive) and is not recommended for production.
```

### 5.6 Mit Komponenten-Auswahl

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}",
        "components": ["Core", "Widgets", "Gui", "OpenGL", "Network"]
    }
}
```

**Standard-Komponenten (wenn nicht angegeben):** Core, Gui, Widgets

### 5.7 Vollständiges Beispiel

```json
"qt6": {
    "path": "externals/qt6",
    "options": {
        "hint": "${QT_ROOT}",
        "backup": "E:/Backup/Qt/6.10.1/msvc2022_64",
        "components": ["Core", "Widgets", "Gui", "OpenGL"]
    }
}
```

---

## 6. Umgebungsvariablen

### 6.1 QT_ROOT (Empfohlen)

**Windows (dauerhaft – Systemvariable):**
```cmd
setx QT_ROOT "C:\Qt\6.10.1\msvc2022_64"
```
→ Neues Terminal öffnen!

**Windows (temporär – aktuelle Session):**
```cmd
set QT_ROOT=C:\Qt\6.10.1\msvc2022_64
```

**Linux/macOS (.bashrc oder .zshrc):**
```bash
export QT_ROOT="$HOME/Qt/6.10.1/gcc_64"
```
→ `source ~/.bashrc` ausführen!

### 6.2 Alternative: QT6_DIR

```bash
export QT6_DIR="/opt/Qt/6.10.1/gcc_64"
```

### 6.3 Visual Studio und Umgebungsvariablen

**Problem:** Visual Studio erbt NICHT automatisch System-Umgebungsvariablen, die nach dem Start von VS gesetzt wurden.

**Symptom:** CMake findet Qt nicht, obwohl `echo %QT_ROOT%` den korrekten Pfad zeigt.

**Lösungen:**

#### Option A: Visual Studio neu starten (nach setx)

Nach `setx QT_ROOT "..."` muss Visual Studio **komplett** neu gestartet werden.

#### Option B: CMakeUserPresets.json (Empfohlen)

Erstelle `CMakeUserPresets.json` im Projekt-Root:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "qt-env",
            "hidden": true,
            "description": "Qt6 Environment",
            "environment": {
                "QT_ROOT": "C:/Qt/6.10.1/msvc2022_64"
            }
        },
        {
            "name": "windows-ninja-debug-clang-qt",
            "displayName": "Windows Debug (Clang + Qt)",
            "inherits": ["windows-ninja-debug-clang", "qt-env"]
        },
        {
            "name": "windows-ninja-release-clang-qt",
            "displayName": "Windows Release (Clang + Qt)",
            "inherits": ["windows-ninja-release-clang", "qt-env"]
        },
        {
            "name": "windows-ninja-debug-msvc-qt",
            "displayName": "Windows Debug (MSVC + Qt)",
            "inherits": ["windows-ninja-debug-msvc", "qt-env"]
        }
    ]
}
```

**Wichtig:** `CMakeUserPresets.json` in `.gitignore` aufnehmen (benutzerspezifisch)!

#### Option C: hint in Solution.json

```json
"options": {
    "hint": "C:/Qt/6.10.1/msvc2022_64"
}
```

---

## 7. Beispielprojekt

### 7.1 Projektstruktur

```
MyProject/
├── CMakeLists.txt
├── CMakePresets.json
├── CMakeUserPresets.json          ← Benutzerspezifisch (nicht committen)
├── Solution.json
├── externals/
│   └── qt6/
│       └── Include.cmake
└── projects/
    └── demos/
        └── exec/
            └── QtMarkdownViewer/
                └── src/
                    └── main.cpp
```

### 7.2 main.cpp

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        auto* central = new QWidget(this);
        auto* layout = new QVBoxLayout(central);
        
        auto* label = new QLabel("Hello Qt6!", this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        
        setCentralWidget(central);
        setWindowTitle("Qt6 Demo");
        resize(400, 300);
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

// WICHTIG: MOC-Include am Ende der Datei!
#include "main.moc"
```

### 7.3 Solution.json

```json
{
    "schemaVersion": "0.1",
    "solution": {
        "name": "QtDemo",
        "version": "1.0.0"
    },
    "settings": {
        "standards": {
            "cxx_standard": 20
        }
    },
    "externals": {
        "qt6": {
            "path": "externals/qt6",
            "options": {
                "hint": "${QT_ROOT}",
                "components": ["Core", "Widgets", "Gui"]
            }
        }
    },
    "executables": [
        {
            "name": "QtMarkdownViewer",
            "displayName": "Markdown Viewer",
            "version": "1.0.0",
            "type": "GUI",
            "path": "projects/demos/exec/QtMarkdownViewer/src",
            "externals": ["qt6"]
        }
    ]
}
```

### 7.4 Build-Befehle

```bash
# Konfigurieren
cmake --preset windows-ninja-debug-clang-qt

# Bauen
cmake --build out/build/windows-ninja-debug-clang-qt

# Ausführen
./out/build/windows-ninja-debug-clang-qt/bin/Debug/QtMarkdownViewer.exe
```

---

## 8. Plattform-spezifische Hinweise

### 8.1 Windows

**DLL-Deployment:**
- `windeployqt` kopiert automatisch alle benötigten DLLs
- Debug-DLLs enden auf `d` (z.B. `Qt6Cored.dll`)
- Release-DLLs ohne Suffix (z.B. `Qt6Core.dll`)

**Qt-Verzeichnis zum PATH hinzufügen (Alternative zu windeployqt):**
```cmd
set PATH=%QT_ROOT%\bin;%PATH%
```

**Compiler-Kompatibilität:**
- Qt für MSVC 2022 → MSVC oder Clang (mit MSVC-Target)
- Qt für MinGW → Nur MinGW

### 8.2 Linux

**RPATH:**
- Automatisch konfiguriert in Include.cmake
- Zeigt auf Qt-Installation
- Kein `LD_LIBRARY_PATH` nötig

**System-Qt vs Installer-Qt:**
```bash
# System-Qt
qmake6 --version  # /usr/bin/qmake6

# Installer-Qt
~/Qt/6.10.1/gcc_64/bin/qmake --version
```

**AppImage erstellen (optional):**
```bash
# linuxdeployqt installieren
# https://github.com/probonopd/linuxdeployqt
linuxdeployqt MyApp -appimage
```

### 8.3 macOS

**Bundle vs Executable:**
- GUI-Apps sollten als Bundle gebaut werden
- `type: "GUI"` in Solution.json setzt `MACOSX_BUNDLE`

**macdeployqt:**
```bash
# Manuell ausführen
~/Qt/6.10.1/macos/bin/macdeployqt MyApp.app
```

**Code Signing (für Distribution):**
```bash
codesign --deep --force --sign "Developer ID" MyApp.app
```

---

## 9. Verfügbare Komponenten

### Basis-Module

| Komponente | Beschreibung | Typische Verwendung |
|------------|--------------|---------------------|
| **Core** | Basisklassen, Container, IO | Immer benötigt |
| **Gui** | GUI-Basis, Fonts, Images | Desktop-/Mobile-Apps |
| **Widgets** | Desktop-Widgets | Desktop-Anwendungen |

### Erweiterte Module

| Komponente | Beschreibung |
|------------|--------------|
| **OpenGL** | OpenGL-Integration |
| **Network** | HTTP, TCP, UDP |
| **Sql** | Datenbank-Abstraktion |
| **Xml** | XML-Parser |
| **Concurrent** | Threading-Utilities |
| **PrintSupport** | Druckfunktionen |

### QML/Quick

| Komponente | Beschreibung |
|------------|--------------|
| **Qml** | QML-Engine |
| **Quick** | Qt Quick (QML-UI) |
| **QuickControls2** | Moderne UI-Komponenten |

### Multimedia

| Komponente | Beschreibung |
|------------|--------------|
| **Multimedia** | Audio/Video |
| **MultimediaWidgets** | Media-Player-Widgets |

### Spezial

| Komponente | Beschreibung |
|------------|--------------|
| **WebEngine** | Chromium-Browser |
| **WebChannel** | Web-Integration |
| **Charts** | Diagramme |
| **3D** | 3D-Rendering |
| **Svg** | SVG-Support |
| **Pdf** | PDF-Rendering |

---

## 10. Stolpersteine und Lösungen

### 10.1 AUTOMOC funktioniert nicht

**Problem:**
```
fatal error: 'main.moc' file not found
```

**Ursache:** `CMAKE_AUTOMOC ON` wurde global gesetzt, wirkt aber nur auf Targets die **danach** erstellt werden.

**Hintergrund:** Im Build-System wird das Executable VOR dem Aufruf der Include.cmake erstellt. Die globale `CMAKE_AUTOMOC` Einstellung hat zu diesem Zeitpunkt keine Wirkung mehr.

**Lösung:** AUTOMOC muss direkt auf dem Target gesetzt werden:
```cmake
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
```

**Status:** ✅ In Include.cmake v0.4.0+ behoben.

### 10.2 Qt-Header werden nicht gefunden

**Problem:**
```
fatal error: 'QApplication' file not found
```

**Ursache:** Der Qt6-Alias (INTERFACE Library) wurde nicht mit Include-Directories erstellt.

**Hintergrund:** CMake's `find_package(Qt6)` stellt nur einzelne Targets bereit (`Qt6::Core`, `Qt6::Widgets`). Das Build-System erwartet aber ein einziges Target `qt6`.

**Lösung:** Include.cmake erstellt ein Alias-Target:
```cmake
add_library(qt6 INTERFACE)
target_link_libraries(qt6 INTERFACE Qt6::Core Qt6::Gui Qt6::Widgets)
```

**Status:** ✅ In Include.cmake v0.3.0+ behoben.

### 10.3 DLLs fehlen zur Laufzeit (Windows)

**Problem:**
```
Die Ausführung des Codes kann nicht fortgesetzt werden, da Qt6Cored.dll nicht gefunden wurde.
```

**Ursache:** Qt-DLLs müssen im selben Verzeichnis wie die .exe liegen oder im PATH sein.

**Lösung:** `windeployqt` kopiert alle benötigten DLLs automatisch nach dem Build:
```cmake
find_program(_WINDEPLOYQT windeployqt HINTS "${_QT6_PREFIX}/bin")
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND "${_WINDEPLOYQT}" "$<TARGET_FILE:${TARGET}>"
)
```

**Status:** ✅ In Include.cmake v0.5.0+ implementiert.

### 10.4 Debug/Release DLLs werden verwechselt (Multi-Config)

**Problem:** Bei Visual Studio (Multi-Config-Generator) werden zur Configure-Zeit die falschen DLLs kopiert.

**Ursache:** `CMAKE_BUILD_TYPE` ist bei Multi-Config-Generatoren zur Configure-Zeit **leer**. Die Konfiguration wird erst beim Build gewählt.

**Hintergrund:**
```cmake
# Ninja (Single-Config): CMAKE_BUILD_TYPE = "Debug" oder "Release"
# Visual Studio (Multi-Config): CMAKE_BUILD_TYPE = "" (leer!)
```

**Lösung:** Generator Expressions verwenden:
```cmake
# Kopiert Qt6Cored.dll nur bei Debug, sonst "true" (no-op)
$<IF:$<CONFIG:Debug>,copy_if_different,true>
```

**Status:** ✅ In Include.cmake v0.5.1+ behoben.

### 10.5 Qt wird trotz Installation nicht gefunden

**Problem:**
```
[qt6] Qt6 not found!
  Searched paths:
    - C:/Qt/6.10.1/msvc2022_64
    ...
```

**Ursache:** Umgebungsvariable nicht gesetzt oder Visual Studio erbt sie nicht.

**Diagnose:**
```bash
# Windows
echo %QT_ROOT%
dir "%QT_ROOT%\lib\cmake\Qt6"

# Linux/macOS
echo $QT_ROOT
ls "$QT_ROOT/lib/cmake/Qt6"
```

**Mögliche Ursachen und Lösungen:**

| Ursache | Lösung |
|---------|--------|
| Umgebungsvariable nicht gesetzt | `setx QT_ROOT "..."` |
| VS erbt Variable nicht | CMakeUserPresets.json verwenden |
| Falscher Pfad/Version | Pfad überprüfen |
| Beschädigte Installation | Qt neu installieren |
| Falsches Compiler-Kit | MSVC-Qt für MSVC, GCC-Qt für GCC |

---

## 11. Troubleshooting

### Checkliste bei Problemen

1. ☐ Qt installiert?
2. ☐ Richtiger Pfad? (`dir %QT_ROOT%\lib\cmake\Qt6`)
3. ☐ Umgebungsvariable gesetzt?
4. ☐ VS neu gestartet nach `setx`?
5. ☐ Include.cmake aktuell (v0.6.0)?
6. ☐ CMake-Cache gelöscht? (`cmake --fresh`)
7. ☐ Richtige Komponenten in `components`?

### Häufige Fehler

| Fehler | Lösung |
|--------|--------|
| `Qt6 not found` | Umgebungsvariable/hint prüfen |
| `QApplication file not found` | Include.cmake v0.3.0+ verwenden |
| `main.moc file not found` | Include.cmake v0.4.0+ verwenden |
| `Qt6Cored.dll not found` | Include.cmake v0.5.0+ verwenden, rebuild |
| `undefined reference to QApplication` | "Widgets" in components hinzufügen |

### Debug-Ausgabe aktivieren

```bash
cmake --preset ... --debug-output
```

Oder in CMakeLists.txt:
```cmake
set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
```

---

## 12. Siehe auch

- [Solution_Schema.md](../references/Solution_Schema.md) – JSON-Schema Referenz
- [Externals.md](../references/Externals.md) – Externals Referenz
- [Adding_Externals Uuserguide](Adding_Externals.md) – Neue Externals hinzufügen
- [CMakeUserPresets_Reference.md](../references/CMakeUserPresets.md) – User Presets Konfiguration

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.5.0** | **2025-12-14** | **Blueprint v0.5.0 Konformität: Voraussetzungen, Stolpersteine/Troubleshooting getrennt, Siehe auch** |
| 0.2.0 | 2025-12-11 | Umfassende Überarbeitung: Stolpersteine, detaillierte Installation, VS-Workarounds, Plattform-Support |
| 0.1.0 | 2025-12-10 | Initial |

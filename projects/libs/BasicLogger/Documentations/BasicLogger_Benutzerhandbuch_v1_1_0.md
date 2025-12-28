# BasicLogger — Benutzerhandbuch

> **Version:** 1.1.0  
> **Datum:** 2025-12-28  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [BasicLogger_UserGuide.md](../../en/guides/BasicLogger_UserGuide.md)

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Wie integriere ich BasicLogger?](#4-wie-integriere-ich-basiclogger)
5. [Wie konfiguriere ich das Logging?](#5-wie-konfiguriere-ich-das-logging)
6. [Wie verwende ich den Logger?](#6-wie-verwende-ich-den-logger)
7. [Stolpersteine und Lösungen](#7-stolpersteine-und-lösungen)
8. [Troubleshooting](#8-troubleshooting)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Überblick

**BasicLogger** ist eine minimale, header-only Logging-Bibliothek für C++-Projekte. Sie ermöglicht einfaches Protokollieren von Nachrichten sowohl auf der Konsole als auch in Dateien.

### Features

- **Header-only:** Keine Kompilierung nötig — einfach einbinden
- **Vier Log-Level:** Debug, Info, Warning, Error
- **Dual-Output:** Konsole und/oder Datei
- **Plattformübergreifend:** Windows, Linux, macOS
- **Zwei Nutzungsarten:** Globaler Logger oder eigene Instanzen
- **Automatisches Flushing:** Keine verlorenen Nachrichten

---

## 2. Voraussetzungen

- [ ] C++17-kompatibler Compiler (GCC 7+, Clang 5+, MSVC 2017+)
- [ ] Standard-Library mit `<chrono>`, `<fstream>`, `<iomanip>`
- [ ] Zugriff auf `BasicLogger.h`

---

## 3. Schnellstart

**1. Header einbinden:**

```cpp
#include <BasicLogger.h>
```

**2. Loggen mit globalem Logger:**

```cpp
int main() {
    BasicLogger::logInfo("Application started");
    BasicLogger::logWarning("Low memory");
    BasicLogger::logError("Connection failed");
    return 0;
}
```

**Ausgabe:**

```
[2025-12-28 14:30:00] [INFO] Application started
[2025-12-28 14:30:00] [WARN] Low memory
[2025-12-28 14:30:00] [ERROR] Connection failed
```

---

## 4. Wie integriere ich BasicLogger?

### 4.1 Header kopieren

Kopiere `BasicLogger.h` in dein Projekt:

```
MyProject/
├── include/
│   └── BasicLogger.h    ← Hierhin kopieren
├── src/
│   └── main.cpp
└── CMakeLists.txt
```

### 4.2 Include-Pfad setzen

**CMake:**

```cmake
target_include_directories(MyApp PRIVATE include)
```

**Compiler direkt:**

```bash
g++ -std=c++17 -I include src/main.cpp -o MyApp
```

### 4.3 Header einbinden

```cpp
#include <BasicLogger.h>
// oder
#include "BasicLogger.h"
```

---

## 5. Wie konfiguriere ich das Logging?

### 5.1 Log-Level einstellen

Steuere, welche Nachrichten angezeigt werden:

```cpp
// Nur Warnungen und Fehler
BasicLogger::setLogLevel(BasicLogger::Level::Warning);

// Alles inklusive Debug
BasicLogger::setLogLevel(BasicLogger::Level::Debug);

// Nur Fehler
BasicLogger::setLogLevel(BasicLogger::Level::Error);
```

**Level-Hierarchie:**

```
Debug < Info < Warning < Error
```

Nachrichten unterhalb des eingestellten Levels werden ignoriert.

### 5.2 In Datei loggen

Aktiviere das Datei-Logging:

```cpp
BasicLogger::setLogFile("application.log");
BasicLogger::logInfo("This goes to console AND file");
```

Die Datei wird im Append-Modus geöffnet — bestehende Inhalte bleiben erhalten.

### 5.3 Konsole deaktivieren

Für reine Datei-Protokollierung:

```cpp
BasicLogger::setLogFile("silent.log");
BasicLogger::setConsoleOutput(false);
BasicLogger::logInfo("Only in file, not on console");
```

### 5.4 Eigene Logger-Instanz

Für mehrere Log-Ziele oder unterschiedliche Konfigurationen:

```cpp
// Logger nur für Konsole
BasicLogger::Logger consoleLogger;
consoleLogger.setLevel(BasicLogger::Level::Info);

// Logger nur für Datei
BasicLogger::Logger fileLogger("errors.log");
fileLogger.setConsoleOutput(false);
fileLogger.setLevel(BasicLogger::Level::Error);

consoleLogger.info("User logged in");       // → Konsole
fileLogger.error("Database connection lost"); // → errors.log
```

---

## 6. Wie verwende ich den Logger?

### 6.1 Convenience-Funktionen (empfohlen)

Am einfachsten mit den globalen Funktionen:

```cpp
BasicLogger::logDebug("Variable x = 42");
BasicLogger::logInfo("Processing item 5 of 10");
BasicLogger::logWarning("Cache nearly full");
BasicLogger::logError("File not found: config.json");
```

### 6.2 Generische log()-Methode

Für dynamisches Level:

```cpp
BasicLogger::Level level = isProduction 
    ? BasicLogger::Level::Warning 
    : BasicLogger::Level::Debug;

BasicLogger::Logger logger;
logger.log(level, "Dynamic log level");
```

### 6.3 Formatierte Nachrichten

BasicLogger akzeptiert `std::string`. Verwende String-Streams oder `std::format` (C++20):

**Mit std::ostringstream:**

```cpp
#include <sstream>

int count = 42;
std::ostringstream oss;
oss << "Processed " << count << " items";
BasicLogger::logInfo(oss.str());
```

**Mit std::format (C++20):**

```cpp
#include <format>

int count = 42;
BasicLogger::logInfo(std::format("Processed {} items", count));
```

### 6.4 Log-Datei ordentlich schließen

Bei Programmende oder Wechsel der Log-Datei:

```cpp
BasicLogger::closeLogFile();
```

---

## 7. Stolpersteine und Lösungen

### 7.1 Debug-Nachrichten erscheinen nicht

**Problem:** `logDebug()` zeigt keine Ausgabe.

**Ursache:** Standard-Level ist `Info` — Debug liegt darunter.

**Lösung:**

```cpp
BasicLogger::setLogLevel(BasicLogger::Level::Debug);
BasicLogger::logDebug("Now visible!");
```

### 7.2 Log-Datei bleibt leer

**Problem:** Datei existiert, aber keine Einträge.

**Ursache 1:** Pfad ohne Schreibrechte.

**Lösung:** Rückgabewert prüfen:

```cpp
BasicLogger::Logger logger;
if (!logger.setLogFile("/protected/path/app.log")) {
    std::cerr << "Cannot write to log file!" << std::endl;
}
```

**Ursache 2:** Log-Level zu hoch für geschriebene Nachrichten.

### 7.3 Doppelte Ausgaben

**Problem:** Gleiche Nachricht erscheint zweimal.

**Ursache:** Sowohl globaler Logger als auch eigene Instanz aktiv.

**Lösung:** Entscheide dich für einen Ansatz:

```cpp
// Entweder global
BasicLogger::logInfo("Only global");

// Oder eigene Instanz
BasicLogger::Logger myLogger;
myLogger.info("Only local");
```

### 7.4 Zeitstempel in falscher Zeitzone

**Problem:** Zeitstempel zeigt UTC statt lokale Zeit.

**Ursache:** Dies sollte nicht passieren — BasicLogger verwendet `localtime_s`/`localtime_r`.

**Lösung:** System-Zeitzone prüfen:

```bash
# Linux
timedatectl

# Windows
tzutil /g
```

---

## 8. Troubleshooting

### Checkliste

- [ ] Header korrekt eingebunden?
- [ ] Include-Pfad in CMake/Compiler gesetzt?
- [ ] Log-Level niedrig genug für gewünschte Nachrichten?
- [ ] Schreibrechte für Log-Datei vorhanden?
- [ ] C++17 aktiviert (`-std=c++17`)?

### Häufige Fehler

| Symptom | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| `'BasicLogger' not found` | Header nicht gefunden | Include-Pfad prüfen |
| Keine Debug-Ausgabe | Level zu hoch | `setLogLevel(Level::Debug)` |
| Leere Log-Datei | Keine Schreibrechte | Pfad/Rechte prüfen |
| Kompilierfehler | C++17 nicht aktiviert | `-std=c++17` Flag hinzufügen |
| Crash bei `setLogFile` | Ungültiger Pfad | Verzeichnis muss existieren |

### Compiler-Flags

**GCC/Clang:**

```bash
g++ -std=c++17 -I include src/main.cpp -o app
```

**MSVC:**

```bash
cl /std:c++17 /I include src\main.cpp
```

---

## 9. Siehe auch

- [BasicLogger — Referenz](BasicLogger_Referenz_v1_1_0.md) — Vollständige API-Dokumentation

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-28** | **Initiales Blueprint-konformes Benutzerhandbuch** |
| 1.0.0 | 2025-12-09 | Ursprüngliche Version |

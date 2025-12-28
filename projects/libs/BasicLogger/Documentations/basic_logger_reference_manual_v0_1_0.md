# BasicLogger – Reference Manual

> **Zielgruppe:** Anwendende Entwickler:innen, die den `BasicLogger` in eigene C++-Projekte integrieren wollen  
> **Modul:** `projects/libs/BasicLogger/include/BasicLogger.h`  
> **Modul-Version:** 1.1.0  
> **Stand:** 2025-12-09  
> **Sprache:** Deutsch  

---

## 1. Zweck und Überblick

Der `BasicLogger` ist ein kompakter, header-only C++-Logger, der ohne externe Abhängigkeiten auskommt. Er eignet sich für kleine bis mittlere Projekte, Tools und Services, bei denen:

- einfache **Text-Logs** benötigt werden,
- **Log-Level** zur Filterung ausreichen,
- Ausgabe wahlweise auf **Konsole** und/oder in eine **Logdatei** erfolgen soll,
- eine schnelle Integration ohne komplexe Konfiguration gewünscht ist.

Dieses Referenz-Manual beschreibt:

- wie der Logger in ein Projekt eingebunden wird,
- welche API-Funktionen zur Verfügung stehen,
- typische Anwendungsfälle und Best Practices,
- wie Konsolen-Logging ein- und ausgeschaltet wird.

---

## 2. Voraussetzungen

- C++17 oder höher (empfohlen C++20)
- Verfügbarkeit der Standardbibliothek (iostream, fstream, chrono, ctime, iomanip, sstream, memory)
- Unterstützung für `localtime_s` (Windows) oder `localtime_r` (POSIX)

Build-Systeme wie CMake, Make, Visual Studio-Projekte etc. werden unterstützt, solange der Header im Include-Pfad vorhanden ist.

---

## 3. Integration ins Projekt

### 3.1 Dateien und Verzeichnisstruktur

Der Logger besteht aus einer einzigen Header-Datei:

- `projects/libs/BasicLogger/include/BasicLogger.h`

Empfohlen wird, diesen Pfad (oder das übergeordnete `include`-Verzeichnis) als **Include-Verzeichnis** im Build-System zu registrieren.

### 3.2 CMake-Beispiel

```cmake
# Beispiel: BasicLogger als Header-only Library einbinden

# Pfad zum BasicLogger-Header (an Projektstruktur anpassen)
set(BASIC_LOGGER_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/projects/libs/BasicLogger/include")

# Eigenes Ziel, das den Logger nutzt
add_executable(MyApp
    src/main.cpp
)

# Include-Verzeichnis hinzufügen
target_include_directories(MyApp
    PRIVATE
        ${BASIC_LOGGER_INCLUDE_DIR}
)
```

### 3.3 Einbinden im C++-Code

```cpp
#include "BasicLogger.h"

int main() {
    // ... Code ...
}
```

Sobald der Header im Include-Pfad liegt, kann er von jeder Übersetzungseinheit aus eingebunden werden.

---

## 4. Grundkonzept des Loggers

Der `BasicLogger` stellt zwei Nutzungsarten zur Verfügung:

1. **Globaler Logger** (empfohlen für einfache Anwendungen)
   - Zugriff über `BasicLogger::getGlobalLogger()` oder Convenience-Funktionen wie `BasicLogger::logInfo("...")`.

2. **Eigene Logger-Instanzen** (pro Modul/Komponente)
   - Direkte Instanziierung von `BasicLogger::Logger` mit optionalem Logfile.

Logausgaben können:

- abhängig vom **Log-Level** gefiltert werden,
- auf der **Konsole**, in einer **Datei** oder gleichzeitig in beiden erscheinen,
- über **Convenience-Methoden** (`info`, `warning`, `error`, etc.) erzeugt werden.

---

## 5. Log-Level

### 5.1 Enum `BasicLogger::Level`

```cpp
enum class Level {
    Debug,
    Info,
    Warning,
    Error
};
```

Die Level geben die Wichtigkeit der Meldung an:

- `Debug`   – Detailinformationen für Entwicklung / Debugging
- `Info`    – Allgemeine Statusmeldungen
- `Warning` – Warnungen, potenzielle Probleme
- `Error`   – Fehler, die Aufmerksamkeit erfordern

Über den **Mindest-Log-Level** wird gesteuert, welche Meldungen überhaupt ausgegeben werden.

---

## 6. Globaler Logger (empfohlene Nutzung)

### 6.1 Konfiguration

```cpp
#include "BasicLogger.h"

int main() {
    using namespace BasicLogger;

    // Mindest-Log-Level festlegen (Standard ist Info)
    setLogLevel(Level::Debug);

    // Optional: Logdatei aktivieren
    setLogFile("app.log");

    // Optional: Konsolen-Logging ein-/ausschalten
    setConsoleOutput(true);   // oder false

    // ... Rest der Anwendung ...
}
```

### 6.2 Logging mit Convenience-Funktionen

```cpp
#include "BasicLogger.h"

void run() {
    using namespace BasicLogger;

    logInfo("Application started");
    logDebug("Internal state: x=42");
    logWarning("Configuration file missing, using defaults");
    logError("Failed to open database");
}
```

### 6.3 Konsolen-Logging deaktivieren

```cpp
#include "BasicLogger.h"

int main() {
    using namespace BasicLogger;

    setLogLevel(Level::Info);
    setLogFile("service.log");

    // Nur Datei-Logging, keine Konsole:
    setConsoleOutput(false);

    logInfo("Service started (file-only logging)");
    return 0;
}
```

Die Meldungen werden in diesem Beispiel ausschließlich in `service.log` geschrieben.

---

## 7. Eigene Logger-Instanzen

Statt des globalen Loggers können eigene Instanzen genutzt werden, z. B. pro Modul oder Klasse.

### 7.1 Erstellen einer Logger-Instanz

```cpp
#include "BasicLogger.h"

class Worker {
public:
    Worker()
        : m_logger("worker.log")        // Logfile im Append-Modus
    {
        m_logger.setLevel(BasicLogger::Level::Debug);

        // Nur Datei-Logging für diesen Worker
        m_logger.setConsoleOutput(false);
    }

    void doWork() {
        m_logger.info("Worker started");
        m_logger.debug("Processing item 1 of 10");
    }

private:
    BasicLogger::Logger m_logger;
};
```

### 7.2 Wichtige Methoden der Klasse `Logger`

```cpp
namespace BasicLogger {

class Logger {
public:
    Logger();
    explicit Logger(const std::string& logFilePath);

    void setLevel(Level level);
    void setConsoleOutput(bool enabled);

    bool setLogFile(const std::string& path);
    void closeLogFile();

    void log(Level level, const std::string& message);

    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);
};

} // namespace BasicLogger
```

**Hinweise:**

- `setLogFile` öffnet die Datei im Append-Modus; Rückgabe `false` bei Fehler.
- `setConsoleOutput(false)` schaltet Konsolen-Logging für diese Instanz ab.

---

## 8. Format der Log-Ausgaben

Eine typische Log-Zeile sieht folgendermaßen aus:

```text
[2025-12-09 10:15:23] [INFO] Application started
```

Allgemeines Format:

```text
[YYYY-MM-DD HH:MM:SS] [LEVEL] Nachrichtentext
```

- Der Zeitstempel verwendet die lokale Systemzeit.
- `LEVEL` entspricht dem Wert des Log-Levels (`DEBUG`, `INFO`, `WARN`, `ERROR`).

---

## 9. Typische Anwendungsfälle

### 9.1 CLI-Tool mit Konsole und Logdatei

```cpp
#include "BasicLogger.h"

int main(int argc, char* argv[]) {
    using namespace BasicLogger;

    setLogLevel(Level::Info);
    setLogFile("tool.log");
    setConsoleOutput(true);  // Konsole + Datei

    logInfo("Tool started");

    // ... Programm-Logik ...

    logInfo("Tool finished");
    return 0;
}
```

### 9.2 Hintergrunddienst ohne Konsole (nur Datei)

```cpp
#include "BasicLogger.h"

int main() {
    using namespace BasicLogger;

    setLogLevel(Level::Warning);
    setLogFile("service.log");

    // Keine Ausgabe im Terminal / Dienstkonsole
    setConsoleOutput(false);

    logInfo("This info will only go to file, not to console");
    logError("Critical error occurred");

    return 0;
}
```

### 9.3 Modul-spezifische Logs

```cpp
#include "BasicLogger.h"

class NetworkModule {
public:
    NetworkModule()
        : m_logger("network.log")
    {
        m_logger.setLevel(BasicLogger::Level::Info);
        m_logger.setConsoleOutput(true);
    }

    void connect() {
        m_logger.info("Connecting to server...");
    }

private:
    BasicLogger::Logger m_logger;
};
```

---

## 10. Fehlerbehandlung und Rückgabewerte

Der `BasicLogger` verwendet keine eigenen Exceptions oder Error-Codes. Relevante Punkte:

- `bool Logger::setLogFile(const std::string& path)`
  - Rückgabe `false`, wenn die Datei nicht geöffnet werden kann.
  - Konsolen-Logging (falls aktiv) funktioniert weiterhin.

- Wenn keine Logdatei gesetzt ist, werden Meldungen nur auf der Konsole ausgegeben (sofern `setConsoleOutput(true)` aktiv ist).

Implementierende Projekte können nach Aufruf von `setLogFile()` optional prüfen, ob die Datei erfolgreich geöffnet wurde, und entsprechend reagieren.

---

## 11. Best Practices

- **Log-Level wählen**:
  - Entwicklungs-Builds: meist `Debug` als Mindest-Log-Level.
  - Produktion: oft `Info` oder `Warning`, um Logflut zu vermeiden.

- **Konsolen-Logging bewusst steuern**:
  - CLI-Tools: Konsole in der Regel aktiviert lassen (schnelles Feedback).
  - Dienste / Daemons: Konsole eher deaktivieren und Datei-Logging nutzen.

- **Logdatei-Pfade konfigurieren**:
  - Pfade nicht hart kodieren, sondern konfigurierbar halten (Konfigurationsdatei, Umgebungsvariablen, CLI-Optionen).

- **Rotation / Größe überwachen**:
  - Aktuell keine automatische Log-Rotation vorhanden.
  - In produktiven Systemen ggf. externe Tools (Logrotate, Windows-Task) verwenden.

---

## 12. Einschränkungen

- **Thread-Sicherheit**:
  - Der Logger ist nicht explizit thread-sicher.
  - Gleichzeitige Zugriffe aus mehreren Threads können zu vermischten Ausgaben führen.

- **Kein strukturiertes Logging**:
  - Nur reiner Text-Output, keine JSON-/Key-Value-Strukturen.

- **Keine Log-Rotation**:
  - Logdateien wachsen unbegrenzt, solange das Programm läuft und loggt.

---

## 13. FAQ (Häufige Fragen)

### F1: Wie schalte ich das Konsolen-Logging komplett ab?

Verwende die globale Funktion:

```cpp
BasicLogger::setConsoleOutput(false);
```

oder pro Instanz:

```cpp
BasicLogger::Logger logger("app.log");
logger.setConsoleOutput(false);
```

### F2: Was passiert, wenn `setLogFile()` fehlschlägt?

- Die Funktion gibt `false` zurück.
- Es wird **nicht** in eine Datei geschrieben.
- Konsolen-Logging (falls aktiviert) ist davon nicht betroffen.

### F3: Wie ändere ich den Mindest-Log-Level?

```cpp
BasicLogger::setLogLevel(BasicLogger::Level::Warning);
```

Nur Meldungen mit Level `Warning` und `Error` werden dann noch ausgegeben.

### F4: Kann ich mehrere Logdateien gleichzeitig nutzen?

- Mit dem globalen Logger: nur eine Logdatei gleichzeitig.
- Mit eigenen Instanzen: mehrere Logger mit jeweils eigener Datei sind möglich.

---

## 14. Kurzübersicht der wichtigsten Funktionen

### Globale Funktionen (`namespace BasicLogger`)

- `Logger& getGlobalLogger();`
- `void setLogLevel(Level level);`
- `void setLogFile(const std::string& path);`
- `void setConsoleOutput(bool enabled);`
- `void closeLogFile();`
- `void logDebug(const std::string& msg);`
- `void logInfo(const std::string& msg);`
- `void logWarning(const std::string& msg);`
- `void logError(const std::string& msg);`

### Instanzmethoden von `Logger`

- `Logger();`
- `explicit Logger(const std::string& logFilePath);`
- `void setLevel(Level level);`
- `void setConsoleOutput(bool enabled);`
- `bool setLogFile(const std::string& path);`
- `void closeLogFile();`
- `void log(Level level, const std::string& message);`
- `void debug(const std::string& msg);`
- `void info(const std::string& msg);`
- `void warning(const std::string& msg);`
- `void error(const std::string& msg);`

---

Dieses Referenz-Manual kann als Grundlage für interne Projektdokumentation und Onboarding neuer Entwickler:innen verwendet werden, die den `BasicLogger` in ihren Anwendungen einsetzen.


# BasicLogger — Referenz

> **Version:** 1.1.0  
> **Datum:** 2025-12-28  
> **Typ:** Reference  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [BasicLogger_Reference.md](../../en/reference/BasicLogger_Reference.md)

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [API nach Kategorie](#3-api-nach-kategorie)
   - 3.1 [Enum `Level`](#31-enum-level)
   - 3.2 [Klasse `Logger`](#32-klasse-logger)
   - 3.3 [Freie Funktionen](#33-freie-funktionen)
   - 3.4 [Globale Convenience-Funktionen](#34-globale-convenience-funktionen)
4. [Schnellreferenz](#4-schnellreferenz)
5. [Verwendung in Code](#5-verwendung-in-code)
6. [Siehe auch](#6-siehe-auch)
7. [Changelog](#7-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert die vollständige API des **BasicLogger** — einer minimalen, header-only Logging-Bibliothek für C++17+.

### Eigenschaften

- Header-only (keine Kompilierung nötig)
- Konsolen- und Datei-Ausgabe
- Vier Log-Level (Debug, Info, Warning, Error)
- Plattformübergreifend (Windows, Linux, macOS)
- Globale und instanzbasierte Nutzung

### Namespace

Alle Typen und Funktionen befinden sich im Namespace `BasicLogger`.

---

## 2. Konventionen

### Notation

| Symbol | Bedeutung |
|--------|-----------|
| `✓` | Pflichtparameter |
| `—` | Optionaler Parameter / kein Default |
| `[out]` | Ausgabeparameter |
| `const&` | Konstante Referenz (keine Kopie) |

### Log-Level Hierarchie

```
Debug < Info < Warning < Error
```

Nachrichten unterhalb des eingestellten Minimums werden ignoriert.

### Ausgabeformat

```
[YYYY-MM-DD HH:MM:SS] [LEVEL] Message
```

---

## 3. API nach Kategorie

### 3.1 Enum `Level`

```cpp
enum class Level {
    Debug,
    Info,
    Warning,
    Error
};
```

| Aspekt | Wert |
|--------|------|
| **Typ** | `enum class` (strongly typed) |
| **Scope** | `BasicLogger::Level` |
| **Seit** | v1.0.0 |

**Werte:**

| Enumerator | Ausgabe | Verwendung |
|------------|---------|------------|
| `Debug` | `DEBUG` | Detaillierte Entwickler-Informationen |
| `Info` | `INFO` | Allgemeine Statusinformationen |
| `Warning` | `WARN` | Potenzielle Probleme |
| `Error` | `ERROR` | Fehler, die Aufmerksamkeit erfordern |

---

### 3.2 Klasse `Logger`

#### Konstruktoren

##### `Logger()`

| Aspekt | Wert |
|--------|------|
| **Typ** | Default-Konstruktor |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Erstellt einen Logger mit Konsolen-Ausgabe. Minimum-Level ist `Info`.

**Beispiel:**
```cpp
BasicLogger::Logger logger;
logger.info("Application started");
```

---

##### `Logger(const std::string& logFilePath)`

| Aspekt | Wert |
|--------|------|
| **Typ** | Expliziter Konstruktor |
| **Parameter** | `logFilePath` — Pfad zur Log-Datei ✓ |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Erstellt einen Logger mit Konsolen- und Datei-Ausgabe.

**Beispiel:**
```cpp
BasicLogger::Logger logger("app.log");
logger.info("Logged to console and file");
```

**Hinweise:**
- Datei wird im Append-Modus geöffnet
- Bei Fehlern wird nur Konsole verwendet

---

#### Konfigurationsmethoden

##### `void setLevel(Level level)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `level` — Minimales Log-Level ✓ |
| **Rückgabe** | — |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Setzt das minimale Log-Level. Nachrichten unterhalb werden ignoriert.

**Beispiel:**
```cpp
logger.setLevel(BasicLogger::Level::Debug);  // Alle Level aktiv
logger.setLevel(BasicLogger::Level::Error);  // Nur Error
```

---

##### `void setConsoleOutput(bool enabled)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `enabled` — Konsole an/aus ✓ |
| **Rückgabe** | — |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Aktiviert oder deaktiviert die Konsolen-Ausgabe.

**Beispiel:**
```cpp
logger.setConsoleOutput(false);  // Nur Datei
```

---

##### `bool setLogFile(const std::string& path)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `path` — Pfad zur Log-Datei ✓ |
| **Rückgabe** | `true` bei Erfolg, `false` bei Fehler |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Öffnet eine Log-Datei im Append-Modus.

**Beispiel:**
```cpp
if (!logger.setLogFile("debug.log")) {
    std::cerr << "Could not open log file\n";
}
```

---

##### `void closeLogFile()`

| Aspekt | Wert |
|--------|------|
| **Rückgabe** | — |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Schließt die aktuelle Log-Datei. Ressourcen werden freigegeben.

---

#### Log-Methoden

##### `void log(Level level, const std::string& message)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `level` — Log-Level ✓, `message` — Nachricht ✓ |
| **Rückgabe** | — |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Protokolliert eine Nachricht mit dem angegebenen Level.

**Beispiel:**
```cpp
logger.log(BasicLogger::Level::Info, "Processing started");
```

---

##### `void debug(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `msg` — Nachricht ✓ |
| **Äquivalent** | `log(Level::Debug, msg)` |
| **Seit** | v1.0.0 |

---

##### `void info(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `msg` — Nachricht ✓ |
| **Äquivalent** | `log(Level::Info, msg)` |
| **Seit** | v1.0.0 |

---

##### `void warning(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `msg` — Nachricht ✓ |
| **Äquivalent** | `log(Level::Warning, msg)` |
| **Seit** | v1.0.0 |

---

##### `void error(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `msg` — Nachricht ✓ |
| **Äquivalent** | `log(Level::Error, msg)` |
| **Seit** | v1.0.0 |

---

### 3.3 Freie Funktionen

##### `const char* levelToString(Level level)`

| Aspekt | Wert |
|--------|------|
| **Parameter** | `level` — Log-Level ✓ |
| **Rückgabe** | C-String der Level-Bezeichnung |
| **Seit** | v1.0.0 |

**Rückgabewerte:**

| Input | Output |
|-------|--------|
| `Level::Debug` | `"DEBUG"` |
| `Level::Info` | `"INFO"` |
| `Level::Warning` | `"WARN"` |
| `Level::Error` | `"ERROR"` |
| Sonstige | `"UNKNOWN"` |

---

##### `std::string getTimestamp()`

| Aspekt | Wert |
|--------|------|
| **Rückgabe** | Aktueller Zeitstempel als String |
| **Format** | `YYYY-MM-DD HH:MM:SS` |
| **Seit** | v1.0.0 |

**Hinweise:**
- Verwendet lokale Zeitzone
- Plattformspezifische Implementierung (`localtime_s` / `localtime_r`)

---

##### `Logger& getGlobalLogger()`

| Aspekt | Wert |
|--------|------|
| **Rückgabe** | Referenz auf globale Logger-Instanz |
| **Seit** | v1.0.0 |

**Beschreibung:**  
Gibt die globale Singleton-Instanz zurück. Lazy-Initialisierung beim ersten Aufruf.

---

### 3.4 Globale Convenience-Funktionen

Diese Funktionen operieren auf der globalen Logger-Instanz.

##### `void setLogLevel(Level level)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().setLevel(level)` |
| **Seit** | v1.0.0 |

---

##### `void setLogFile(const std::string& path)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().setLogFile(path)` |
| **Seit** | v1.0.0 |

---

##### `void setConsoleOutput(bool enabled)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().setConsoleOutput(enabled)` |
| **Seit** | v1.0.0 |

---

##### `void closeLogFile()`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().closeLogFile()` |
| **Seit** | v1.0.0 |

---

##### `void logDebug(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().debug(msg)` |
| **Seit** | v1.0.0 |

---

##### `void logInfo(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().info(msg)` |
| **Seit** | v1.0.0 |

---

##### `void logWarning(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().warning(msg)` |
| **Seit** | v1.0.0 |

---

##### `void logError(const std::string& msg)`

| Aspekt | Wert |
|--------|------|
| **Äquivalent** | `getGlobalLogger().error(msg)` |
| **Seit** | v1.0.0 |

---

## 4. Schnellreferenz

### Klasse Logger

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `Logger()` | — | — | Konstruktor (nur Konsole) |
| `Logger(path)` | `string` | — | Konstruktor (Konsole + Datei) |
| `setLevel(level)` | `Level` | — | Minimum-Level setzen |
| `setConsoleOutput(b)` | `bool` | — | Konsole an/aus |
| `setLogFile(path)` | `string` | `bool` | Log-Datei öffnen |
| `closeLogFile()` | — | — | Log-Datei schließen |
| `log(level, msg)` | `Level`, `string` | — | Nachricht loggen |
| `debug(msg)` | `string` | — | Debug-Nachricht |
| `info(msg)` | `string` | — | Info-Nachricht |
| `warning(msg)` | `string` | — | Warnung |
| `error(msg)` | `string` | — | Fehler |

### Globale Funktionen

| Funktion | Parameter | Beschreibung |
|----------|-----------|--------------|
| `setLogLevel(level)` | `Level` | Globales Level setzen |
| `setLogFile(path)` | `string` | Globale Datei setzen |
| `setConsoleOutput(b)` | `bool` | Globale Konsole an/aus |
| `closeLogFile()` | — | Globale Datei schließen |
| `logDebug(msg)` | `string` | Globales Debug |
| `logInfo(msg)` | `string` | Globales Info |
| `logWarning(msg)` | `string` | Globale Warnung |
| `logError(msg)` | `string` | Globaler Fehler |

### Hilfsfunktionen

| Funktion | Parameter | Rückgabe | Beschreibung |
|----------|-----------|----------|--------------|
| `levelToString(level)` | `Level` | `const char*` | Level als String |
| `getTimestamp()` | — | `string` | Aktueller Zeitstempel |
| `getGlobalLogger()` | — | `Logger&` | Globale Instanz |

---

## 5. Verwendung in Code

### Einbinden

```cpp
#include <BasicLogger.h>
```

### Instanzbasiert

```cpp
BasicLogger::Logger logger("app.log");
logger.setLevel(BasicLogger::Level::Debug);
logger.info("Application started");
logger.error("Something went wrong");
```

### Global

```cpp
BasicLogger::setLogFile("global.log");
BasicLogger::setLogLevel(BasicLogger::Level::Debug);
BasicLogger::logInfo("Using global logger");
BasicLogger::logError("Global error");
```

### Nur Datei (ohne Konsole)

```cpp
BasicLogger::Logger logger("silent.log");
logger.setConsoleOutput(false);
logger.info("Only in file");
```

---

## 6. Siehe auch

- [BasicLogger — Benutzerhandbuch](BasicLogger_Benutzerhandbuch_v1_1_0.md)

---

## 7. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-28** | **Initiale Blueprint-konforme Referenz** |
| 1.0.0 | 2025-12-09 | Ursprüngliche API |

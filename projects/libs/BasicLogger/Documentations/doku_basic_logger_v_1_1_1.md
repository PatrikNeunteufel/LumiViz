# BasicLogger – Technische Dokumentation

> **Modul:** BasicLogger (Header-only C++ Logger)
> **Version:** 1.1.1
> **Status:** Stable
> **Datum:** 2025-12-09

---
## 1. Solution.json-Konfiguration

Damit der **BasicLogger** im CMake-Architecture-V2-System korrekt eingebunden wird, muss im `Solution.json`
unter `libraries` folgender Eintrag vorhanden sein:

```json
{
  "name": "BasicLogger",
  "version": "1.0.0",
  "type": "INTERFACE",
  "public_headers": "projects/libs/BasicLogger/include"
}
```

**Hinweise:**

- `type: "INTERFACE"` kennzeichnet den BasicLogger als header-only Library (kein eigenes Buildziel).
- `public_headers` zeigt auf den öffentlichen Include-Ordner und ermöglicht z. B. `#include <BasicLogger.h>`.
- Solange dieser Block existiert, können Executables den BasicLogger einfach über `dependencies` referenzieren.

---

## 2. Übersicht
Der **BasicLogger** ist ein kompakter, header-only Logger, der ohne zusätzliche Build-Targets eingebunden werden kann.
Ziel ist maximale Einfachheit bei minimalem Einbindungsaufwand.

---

## 3. Ziele und Einsatzszenarien

Der BasicLogger deckt folgende typische Einsatzszenarien ab:

- Kleine bis mittlere C++-Projekte
- Tools, Prototypen, Testprogramme
- Gemeinsamer Logger für mehrere Executables
- Logging nach Konsole und/oder Datei

Der Logger ist NICHT als High-Performance-Logger mit asynchronen Queues oder Rotations-Logfiles gedacht, sondern als
übersichtliche, leicht zu verstehende Basislösung.

---

## 4. Architektur und Design

### 4.1 Header-only Ansatz

- Single-Header-Implementierung (z. B. `BasicLogger.h`)
- Keine eigene Library, kein spezielles Buildziel erforderlich
- Nutzung über `#include <BasicLogger.h>` und einen globalen Logger

### 4.2 Globale Logger-Instanz

- Zentraler Zugriff über eine globale/Singleton-ähnliche Instanz
- Initialisierung an einem definierten Punkt (z. B. Programmbeginn)
- Nutzung von Log-Leveln, um Ausgaben zu filtern

### 4.3 Log-Ziele

Aktuell vorgesehene/typische Ziele:

- Konsole (stdout)
- Log-Datei (z. B. im Arbeitsverzeichnis)

Die konkrete Konfiguration kann je nach Projekt angepasst werden.

---

## 5. Einbindung ins Projekt

### 5.1 Include-Pfad

Der BasicLogger wird über einen Include-Ordner (z. B. `projects/libs/BasicLogger/include`) eingebunden.
Über diesen Pfad wird der Header zur Verfügung gestellt und kann im Code verwendet werden.

Beispiel:

```cpp
#include <BasicLogger.h>
```

### 5.2 Abhängigkeiten

- Die Library ist weitgehend eigenständig.
- Es werden nur Standard-C++-Bibliotheken verwendet (z. B. `<iostream>`, `<fstream>`, `<string>`, `<chrono>`).

---

## 6. Initialisierung und Grundkonfiguration

### 6.1 Initialisierung

Typischerweise wird der Logger früh im Programmlebenszyklus initialisiert, z. B. direkt in `main()` oder
in einer zentralen Initialisierungsfunktion.

Wichtige Punkte:

- Festlegen des Log-Levels (z. B. Debug, Info, Warning, Error)
- Aktivieren/Deaktivieren von Ausgabezielen (Konsole, Datei)
- Optional: Setzen eines Log-Dateinamens

### 6.2 Log-Level

Der Logger unterscheidet üblicherweise mehrere Level, z. B.:

- Debug
- Info
- Warning
- Error

Damit lassen sich in der Entwicklung ausführlichere Logs aktivieren, während im Release-Betrieb
nur Warnungen und Fehler protokolliert werden.

---

## 7. Nutzung im Code

### 7.1 Einfache Log-Ausgaben

Beispielhafte Nutzung (schematisch):

```cpp
LOG_INFO("Application started");
LOG_WARNING("Configuration value is deprecated");
LOG_ERROR("Failed to open file");
```

### 7.2 Formatierte Nachrichten

Falls der Logger formatierte Strings unterstützt, können zusätzlich Platzhalter verwendet werden, z. B.:

```cpp
LOG_DEBUG("Processing item {} of {}", index, totalCount);
```

(Die konkrete Syntax hängt von der tatsächlichen Implementierung ab.)

---

## 8. Typische Verwendung in Executables

Der BasicLogger wird als gemeinsame Basis-Library in mehreren Executables genutzt, z. B.:

- `MinimalConsole`
- `consolePlayer`
- `MinimalWindow`
- `imGuiApp`

Die Executables referenzieren die Library über das Buildsystem (z. B. `dependencies: ["BasicLogger"]` im Solution-System)
und konfigurieren den Logger früh im Programmstart.

---

## 9. Erweiterungsmöglichkeiten

Mögliche zukünftige Erweiterungen:

- Asynchrones Logging (Queue + Worker-Thread)
- Log-Rotation (z. B. tägliche Files, Größenlimit)
- Zusätzliche Ausgabeziele (z. B. Syslog, Netzwerk)
- Strukturierte Logs (z. B. JSON)

---

## 10. Bekannte Einschränkungen

- Keine Hochleistungs- oder Realtime-Logger-Architektur
- Datei-I/O kann blockierend wirken
- Header-only-Ansatz kann bei sehr vielen Includes die Compile-Zeit erhöhen

---

## 11. Wartung und Versionierung

- Änderungen am Logger sollten sauber dokumentiert und versioniert werden.
- Die aktuelle Dokumentation bezieht sich auf die Version **1.1.1**.
- Kompatibilität zu bestehenden Executables sollte bei API-Änderungen beachtet werden.


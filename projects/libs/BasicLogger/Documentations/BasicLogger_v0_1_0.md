# BasicLogger – Header-only C++ Logger

> **Version:** 0.1.0  
> **Datum:** 2025-12-09  
> **Typ:** Modul-Doku  
> **Status:** In Entwicklung  
> **Modul:** projects/libs/BasicLogger/include/BasicLogger.h  
> **Modul-Version:** 1.1.0  
> **Basiert auf:** Documentation_Blueprint_v0_1_0 (de)  

---

## 1. Übersicht

`BasicLogger` ist ein minimalistischer, header-only Logger für C++-Projekte mit:

* Log-Level-Unterstützung (`Debug`, `Info`, `Warning`, `Error`)
* Ausgabe auf Konsole (std::cout)
* Optionaler Datei-Log (std::ofstream, Append-Modus)
* Einem globalen Logger (`getGlobalLogger()`) für bequeme Nutzung
* Möglichkeit, Konsolen-Logging ein- und auszuschalten

Ziel ist ein einfach integrierbarer Logger ohne zusätzliche Abhängigkeiten.

---

## 2. Abhängigkeiten

* Standard C++-Bibliothek:

  * `<iostream>` – Konsolenausgabe
  * `<fstream>` – Datei-Output
  * `<string>` – Strings
  * `<chrono>`, `<ctime>` – Zeitstempel
  * `<iomanip>`, `<sstream>` – Formatierung des Zeitstempels
  * `<memory>` – `std::unique_ptr` für Logfile-Handle

Plattform-spezifische Teile:

* Windows: `localtime_s`
* POSIX: `localtime_r`

---

## 3. Konzept / Design

### 3.1 Grundidee

* Der Logger ist als einfache Klasse `BasicLogger::Logger` umgesetzt.
* Er unterstützt:

  * Mindest-Log-Level (`m_minLevel`)
  * Konsole an/aus (`m_logToConsole`)
  * optionales Logfile via `std::ofstream`.

### 3.2 Globaler Logger

* Über `BasicLogger::getGlobalLogger()` steht eine Singleton-Instanz zur Verfügung.
* Globale Convenience-Funktionen kapseln Standardoperationen:

  * `setLogLevel(...)`
  * `setLogFile(...)`
  * `setConsoleOutput(...)`
  * `closeLogFile()`
  * `logDebug(...)`, `logInfo(...)`, `logWarning(...)`, `logError(...)`

Dadurch kann der Logger schnell in bestehenden Projekten genutzt werden, ohne überall Instanzen durchzureichen.

---

## 4. API-Referenz

### 4.1 Namespace

`namespace BasicLogger { ... }`

### 4.2 enum class Level

`Debug`, `Info`, `Warning`, `Error`

---

## 5. Fehlerbehandlung / Limitierungen

* `setLogFile(...)` gibt `false` zurück, wenn Datei nicht geöffnet werden kann.
* Keine eigenen Error Codes.
* Kein Exception-Mechanismus.
* Nicht thread-sicher.
* Keine Log-Rotation.

---

## 6. Best Practices

* Log-Level im Release-Build restriktiver setzen.
* Konsolen-Logging gezielt steuern.
* Log-Pfade konfigurierbar halten.
* Rotation für große Logs in Erwägung ziehen.

---

## 7. Bekannte Einschränkungen

* Multi-Thread Support eingeschränkt.
* Keine Format-Platzhalter.
* Log-Dateien wachsen unbegrenzt.

---

## 8. Migration

| Von   | Nach  | Hinweis                                       |
| ----- | ----- | --------------------------------------------- |
| 1.0.0 | 1.1.0 | Globaler Konsolen-Output-Schalter hinzugefügt |

Keine Breaking Changes.

---

## 9. Siehe auch

* Documentation_Blueprint_v0_1_0
* Künftige Ergänzung: Logging_Guidelines

---

## 10. Changelog

| Version | Datum      | Änderungen                                                   |
| ------- | ---------- | ------------------------------------------------------------ |
| 0.1.0   | 2025-12-09 | Erste Version der Modul-Dokumentation für BasicLogger v1.1.0 |

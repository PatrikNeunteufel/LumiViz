# BasicLogger – Technische Dokumentation

> **Modul:** BasicLogger (Header-only C++ Logger)
> **Version:** 1.1.0
> **Status:** Stable
> **Datum:** 2025-12-09

---

## 1. Übersicht
Der **BasicLogger** ist ein kompakter, header-only Logger, der ohne externe Abhängigkeiten auskommt und sowohl **Konsolen- als auch Datei-Logging** unterstützt. Ziel ist maximale Einfachheit bei minimalem Einbindungsaufwand.

---

## 2. Features
- Header-only, kein Buildziel erforderlich
- Log-Level: Debug, Info, Warning, Error
- Konsole & Datei gleichzeitig möglich
- Globaler Logger verfügbar
- Zeitstempel im Format `YYYY-MM-DD HH:MM:SS`

---

## 3. Architektur
| Element | Beschreibung |
|--------|-------------|
| `Logger` | zentrale Klasse für Logging |
| `getGlobalLogger()` | globaler Singleton-Logger |
| Convenience-Funktionen | `logInfo`, `logWarning`, etc. |

Instanzloggers und globaler Logger können parallel existieren.

---

## 4. Datenfluss
1. Nachricht kommt über `log(Level, std::string)`
2. Zeitstempel generiert via `getTimestamp()`
3. Format: `[timestamp] [LEVEL] message`
4. Ausgabe erfolgt je nach Konfiguration an Konsole und/oder Datei

---

## 5. Fehlerfälle
- `setLogFile()` kann fehlschlagen → Rückgabe `false`
- Kein Thread-Safety garantiert
- Keine Logrotation implementiert

---

## 6. Einsatzempfehlung
- Entwicklungsmodus: `Debug`
- Produktion: `Info` oder `Warning`
- Für Dienste: Console-Output deaktivieren + Dateilog

---

## 7. Erweiterungsoptionen
- Logrotation
- Thread-Safe via `std::mutex`
- Strukturierte Logs (JSON)

---

## 8. Changelog
| Version | Änderung |
|--------|----------|
| 1.1.0 | Globaler Console-Schalter, aktualisierte Zeitstempel |
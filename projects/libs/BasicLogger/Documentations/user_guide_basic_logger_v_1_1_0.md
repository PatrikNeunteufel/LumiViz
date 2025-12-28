# BasicLogger – Benutzerhandbuch

> **Version:** 1.1.0  
> **Typ:** Benutzer-Doku  
> **Status:** Stable  
> **Modul:** BasicLogger

---

## 1. Einführung

Der **BasicLogger** ist ein sofort einsetzbarer Logger für C++ Anwendungen. Er benötigt keinerlei Setup, keine zusätzlich zu bauenden Libraries und funktioniert in kleinen Tools genauso wie in GUI-Anwendungen.

Hauptvorteile:
- Kein Buildziel – nur den Header einbinden
- Logdatei & Konsole gleichzeitig
- Individuelle Level-Steuerung

---

## 2. Einbindung

### Minimal
```cpp
#include "BasicLogger.h"
using namespace BasicLogger;
```

### Globalen Logger verwenden
```cpp
logInfo("App gestartet");
logError("Fehler aufgetreten");
```

---

## 3. Konfiguration

### LogLevel setzen
```cpp
getGlobalLogger().setLogLevel(Logger::Level::Warning);
```

### Logdatei aktivieren
```cpp
getGlobalLogger().setLogFile("myapp.log");
```

### Konsolenausgabe deaktivieren
```cpp
getGlobalLogger().setConsoleOutput(false);
```

---

## 4. Typische Anwendungsfälle

| Fall | Empfehlung |
|------|------------|
| Debuggen | Console AUS, Datei EIN |
| CLI Tools | Console EIN |
| Hintergrunddienste | Console AUS, Datei EIN |
| Testautomatisierung | nur Datei |

---

## 5. Troubleshooting

| Problem | Ursache | Lösung |
|--------|--------|--------|
| Logdatei wird nicht erstellt | Schreibrechte fehlen | anderen Pfad nutzen |
| Konsole zeigt nichts | ConsoleOutput deaktiviert | wieder aktivieren |
| Nach Export fehlen Logs | Nur Konsole aktiviert | Datei aktivieren |

---

## 6. Best Practices

- Logdatei beim Start setzen
- Level zur Laufzeit nur bei Bedarf ändern
- Für Multithreading: Mutex-Schutz ergänzen

---

## 7. Kurzreferenz

| Funktion | Zweck |
|----------|-------|
| `logInfo()` | neutrale Nachricht |
| `logWarning()` | auffällige Situation |
| `logError()` | kritischer Fehler |
| `setLogFile()` | Datei aktivieren |
| `setConsoleOutput()` | Konsole kontrollieren |
| `setLogLevel()` | Level filtern |

---

## 8. Changelog
| Version | Änderung |
|--------|----------|
| 1.1.0 | globaler Console-Schalter + Zeitformat angepasst |
| 1.0.0 | Erste Version des Benutzerhandbuches |


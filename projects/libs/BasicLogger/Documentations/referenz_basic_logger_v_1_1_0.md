# BasicLogger – Referenz

> **Version:** 1.1.0  
> **Status:** Stable  
> **Typ:** API-Referenz  
> **Modul:** BasicLogger

---

## 1. Klassenübersicht

### **class Logger**
Logging-Klasse für Konsole und Datei.

| Konstruktor | Beschreibung |
|------------|-------------|
| `Logger()` | Konsole aktiv, kein File |
| `Logger(const std::string& filename)` | File-Logging aktiviert |

| Methode | Beschreibung |
|--------|-------------|
| `setLogLevel(Level)` | Filter einstellen |
| `setConsoleOutput(bool)` | Konsole aktivieren/deaktivieren |
| `setLogFile(string)` | Logdatei setzen |
| `log(Level, string)` | generische Loggingfunktion |
| `logDebug/Info/Warning/Error()` | Convenience wrappers |
| `getTimestamp()` | Liefert `YYYY-MM-DD HH:MM:SS` |

---

## 2. ENUM Level

| Wert | Bedeutung |
|------|-----------|
| Debug | volle Detailtiefe |
| Info | Standardinfos |
| Warning | auffällige Situationen |
| Error | kritische Fehler |

---

## 3. Globaler Logger

### `Logger& getGlobalLogger()`
Singleton-Instanz. Empfohlen für kleine Projekte und schnelle Integration.

#### Globale Wrapper
- `logDebug(const std::string&)`
- `logInfo(const std::string&)`
- `logWarning(const std::string&)`
- `logError(const std::string&)`

---

## 4. Dateilogging

`bool setLogFile(const std::string& filename)`

| Verhalten | Ergebnis |
|----------|---------|
| Datei kann geöffnet werden | true |
| Fehler (Rechte/Pfad/Lock) | false |

**Wichtig:** Datei wird sofort geöffnet und während Programmlebens offen gehalten.

---

## 5. Threading

⚠ **Nicht thread-safe.**

Für parallele Anwendungen wird Anpassung mit `std::mutex` empfohlen.

---

## 6. Format

Format: `[YYYY-MM-DD HH:MM:SS] [LEVEL] message`

Beispiel:
```
[2025-12-09 02:13:55] [INFO] Starting playback
```

---

## 7. Fehlerbehandlung

| Fehler | Verhalten |
|-------|-----------|
| Logdatei kann nicht geschrieben werden | Konsolenfallback |
| ungültiger LogLevel | ignoriert |
| kein Ziel aktiviert | stilles Ignorieren |

---

## 8. Rückgabewerte
Alle logX()-Funktionen geben `void` zurück – Logging ist **fire & forget**.

---

## 9. Siehe auch
- Doku_BasicLogger_v1_1_0
- UserGuide_BasicLogger_v1_1_0

---

## 10. Changelog
| Version | Änderung |
|--------|----------|
| 1.1.0 | Unified Timestamp, globaler Console-Schalter |
| 1.0.0 | Erste Implementierung |
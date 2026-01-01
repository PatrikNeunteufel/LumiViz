# Compiler Speicherprobleme — Troubleshooting Guide

> **Version:** 1.0.0  
> **Datum:** 2026-01-01  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler  
> **Sprache:** Deutsch  
> **English:** [Compiler_Memory_Issues_Troubleshooting.md](../../en/guides/Compiler_Memory_Issues_Troubleshooting.md)

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Fehlerbild](#4-fehlerbild)
5. [Ursachen](#5-ursachen)
6. [Lösungen](#6-lösungen)
7. [Stolpersteine und Lösungen](#7-stolpersteine-und-lösungen)
8. [Troubleshooting](#8-troubleshooting)
9. [Siehe auch](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Überblick

Dieser Guide beschreibt die Diagnose und Behebung von **Compiler-Speicherfehlern**, die während des Builds von C++/Qt-Projekten auftreten können.

### Features

- Diagnose von Out-of-Memory (OOM) Compiler-Crashes
- Lösungen für Clang/MSVC Kompatibilitätsprobleme
- Windows Page File Optimierung
- Präventionsmaßnahmen für zukünftige Builds

### Symptome

Diese Fehler manifestieren sich als:

- Compiler-Crashes bei Qt-Header-Parsing
- "Out of Memory" Meldungen während der Kompilierung
- Fehlgeschlagene Builds trotz korrektem Quellcode

---

## 2. Voraussetzungen

- [ ] Windows 10/11 als Entwicklungsumgebung
- [ ] CMake 3.19+ installiert
- [ ] Visual Studio 2022 mit C++ Workload
- [ ] Administrator-Rechte (für Page File Änderungen)
- [ ] Mindestens 10 GB freier Festplattenspeicher auf C:

---

## 3. Schnellstart

**Problem:** Build schlägt mit "LLVM error: out of memory" fehl.

**Schnellste Lösung:** Wechsel von Clang zu MSVC:

```bash
# Von Clang...
cmake --preset windows-ninja-debug-clang

# ...zu MSVC wechseln
cmake --preset windows-ninja-debug
cmake --build --preset windows-ninja-debug
```

**Visual Studio:** CMake > Konfiguration ändern > `windows-ninja-debug` (ohne `-clang`)

---

## 4. Fehlerbild

### 4.1 Typische Fehlermeldungen

Folgende Fehlermeldungen deuten auf Compiler-Speicherprobleme hin:

```
LLVM error: out of memory
Allocation failed
Die Auslagerungsdatei ist zu klein
IO failure on output stream: no space on device
Exception Code: 0xC000001D
```

### 4.2 Betroffene Dateien

Die Fehler treten typischerweise beim Parsen dieser Header auf:

| Kategorie | Dateien |
|-----------|---------|
| **Qt-Header** | `qcompare.h`, `qtmetamacros.h`, `qchar.h`, `qnamespace.h` |
| **STL-Header** | `vector`, `unordered_map`, `type_traits` |
| **Bibliotheken** | Qt-ADS Module (`DockManager`, `DockWidget`, etc.) |

### 4.3 Build-Log Beispiel

```
[9/44] Building CXX object CMakeFiles/MyViz.Core.dir/.../PulsingVisualizer.cpp.obj
FAILED: CMakeFiles/MyViz.Core.dir/.../PulsingVisualizer.cpp.obj
...
LLVM error: out of memory
Allocation failed
```

---

## 5. Ursachen

### 5.1 Primäre Ursachen

| Ursache | Beschreibung | Schweregrad |
|---------|--------------|-------------|
| **Clang + MSVC Inkompatibilität** | Clang 18+ hat bekannte Probleme mit neueren MSVC-Headern (14.40+). Die template-lastigen Qt 6.x Header verstärken dieses Problem. | Hoch |
| **Parallele Kompilierung** | Ninja startet standardmäßig viele parallele Compiler-Prozesse. Jeder Clang-Prozess kann 2-4 GB RAM benötigen. | Mittel |
| **Unzureichende Auslagerung** | Windows Page File ist zu klein konfiguriert. Standard ist oft nur 2-4 GB, während Build-Prozesse 16+ GB benötigen können. | Mittel |
| **Festplattenspeicher** | Weniger als 10 GB freier Speicher auf C: kann zu "no space on device" führen. | Niedrig |

### 5.2 Bekannte problematische Kombinationen

| Clang | MSVC Headers | Qt | Status |
|-------|--------------|-----|--------|
| 18.x - 20.x | 14.40+ | 6.7+ | ⚠ Problematisch |
| 20.1.8 | 14.50.35717 | 6.10.1 | ⚠ Bekannt fehlerhaft |
| MSVC 19.x | 14.50+ | 6.10.1 | ✓ Stabil |

---

## 6. Lösungen

### 6.1 Wie wechsle ich von Clang zu MSVC? (Empfohlen)

Der effektivste Fix ist der Wechsel von Clang zu MSVC.

**Kommandozeile:**

```bash
# Vorher (Clang)
cmake --preset windows-ninja-debug-clang
cmake --build --preset windows-ninja-debug-clang

# Nachher (MSVC)
cmake --preset windows-ninja-debug
cmake --build --preset windows-ninja-debug
```

**Visual Studio:**

1. Öffne CMake > Konfiguration ändern
2. Wähle `windows-ninja-debug` (ohne `-clang`)
3. Konfiguriere und baue neu

> **Hinweis:** MSVC ist auf Windows der stabilste Compiler für Qt-Projekte.

### 6.2 Wie reduziere ich die parallelen Jobs?

Falls du Clang beibehalten musst:

```bash
# Nur 2 parallele Kompilierungen (RAM-schonend)
cmake --build --preset windows-ninja-debug-clang -- -j2

# Single-threaded (langsam aber stabil)
cmake --build --preset windows-ninja-debug-clang -- -j1
```

**Empfehlung:** Starte mit `-j2` und erhöhe schrittweise.

### 6.3 Wie erhöhe ich den Windows Page File?

1. Drücke `Windows + Pause` → **System** → **Erweiterte Systemeinstellungen**
2. Wähle **Erweitert** → **Leistung** → **Einstellungen**
3. Wähle **Erweitert** → **Virtueller Arbeitsspeicher** → **Ändern**
4. Deaktiviere **"Auslagerungsdateigröße automatisch verwalten"**
5. Wähle **Benutzerdefinierte Größe**:
   - Anfangsgröße: `8192` MB
   - Maximale Größe: `16384` MB
6. Klicke **Festlegen** → **OK**
7. **Neustart erforderlich**

> **Hinweis:** Für große C++/Qt-Projekte empfehlen wir mindestens 16 GB Page File.

### 6.4 Wie prüfe ich den Festplattenspeicher?

**PowerShell:**

```powershell
# Freien Speicher prüfen
Get-PSDrive C | Select-Object Used, Free

# Build-Ordner löschen (falls < 10 GB frei)
Remove-Item -Recurse -Force .\out\build\
```

**Mindestanforderung:** 10 GB freier Speicher auf C:

---

## 7. Stolpersteine und Lösungen

### 7.1 Build funktioniert nach Preset-Wechsel nicht

**Problem:** Nach dem Wechsel von Clang zu MSVC erscheinen seltsame Fehler.

**Ursache:** CMake-Cache enthält noch Clang-Konfiguration.

**Lösung:** Build-Ordner vollständig löschen:

```bash
# Alten Cache löschen
Remove-Item -Recurse -Force .\out\build\windows-ninja-debug\

# Neu konfigurieren
cmake --preset windows-ninja-debug
```

### 7.2 Fehler kehrt nach Page File Erhöhung zurück

**Problem:** Trotz erhöhtem Page File tritt der Fehler wieder auf.

**Ursache:** Neustart wurde nicht durchgeführt.

**Lösung:** Windows muss nach Page File Änderungen neu gestartet werden.

### 7.3 "no space on device" trotz freiem Speicher

**Problem:** Festplatte zeigt 50 GB frei, aber Build meldet "no space".

**Ursache:** Page File und TEMP-Ordner liegen auf C:, das Build-Projekt auf D:.

**Lösung:** Prüfe Speicher auf **C:**, nicht auf dem Projekt-Laufwerk:

```powershell
Get-PSDrive C | Select-Object Used, Free
```

---

## 8. Troubleshooting

### 8.1 Diagnose-Checkliste

- [ ] RAM-Auslastung während Build prüfen (Task-Manager > Leistung)
- [ ] Freier Festplattenspeicher auf C: > 10 GB?
- [ ] Clang-Version prüfen (`clang --version`)
- [ ] MSVC-Version prüfen (`cl.exe`)
- [ ] Qt-Version prüfen (`qmake --version`)
- [ ] Page File Größe prüfen (Systemeigenschaften)

### 8.2 Häufige Fehler und Lösungen

| Fehler | Mögliche Ursache | Lösung |
|--------|------------------|--------|
| `LLVM error: out of memory` | Clang + Qt Inkompatibilität | Zu MSVC wechseln |
| `Die Auslagerungsdatei ist zu klein` | Page File zu klein | Page File auf 16 GB erhöhen |
| `no space on device` | Festplatte voll | Speicher auf C: freigeben |
| `Exception Code: 0xC000001D` | Compiler-Crash | Parallele Jobs reduzieren (`-j2`) |
| Build hängt ohne Fehler | RAM erschöpft | Task-Manager prüfen, Jobs reduzieren |

### 8.3 Diagnose-Befehle

```powershell
# System-Info
systeminfo | findstr /B /C:"Gesamter physischer Speicher" /C:"Verfügbarer physischer Speicher"

# Festplattenspeicher
Get-PSDrive C, D | Select-Object Name, Used, Free

# Compiler-Versionen
clang --version
cl.exe 2>&1 | Select-String "Version"

# Laufende Compiler-Prozesse
Get-Process | Where-Object { $_.Name -match "clang|cl" }
```

---

## 9. Siehe auch

- [CMake_Standard.md](../standards/CMake_Standard.md) — CMake Coding Standards
- [Cpp_Coding_Standard.md](../standards/Cpp_Coding_Standard.md) — C++ Coding Standards
- [Qt6 Integration Guide](Qt6_Integration_UserGuide.md) — Qt6 Setup und Konfiguration

### Externe Ressourcen

- [Clang Compatibility Notes](https://clang.llvm.org/docs/MSVCCompatibility.html)
- [Qt Documentation](https://doc.qt.io/)
- [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)

---

## 10. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-01** | **Initial: Diagnose und Lösungen für Clang/MSVC OOM-Fehler** |

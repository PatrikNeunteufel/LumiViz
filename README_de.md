# ❖ CMake Modular Build System – Template & Architecture

Dieses Projekt stellt ein **modulares, erweiterbares und dokumentiertes CMake-Build-System** bereit, das als Grundlage für unterschiedliche Softwareprojekte dienen kann – von kleinen Tools bis zu komplexen Multi-Library-Setups.

Es adressiert typische Probleme gewachsener Build-Systeme:
unklare Abhängigkeiten, fehlende Standardisierung, schwer nachvollziehbare Fehler, nicht reproduzierbare Build-Konfigurationen und fehlende Trennung zwischen Projekt- und Benutzerkonfiguration.

## 🎯 Ziele

- Standardisierte Build-Architektur
- Wiederverwendbare CMake-Module
- JSON-gesteuerte Projektdefinitionen
- Transparente Fehler- und Log-Ausgabe
- Klare, versionierte Dokumentation
- Einheitliche Preset-Steuerung
- Plattformübergreifende Builds (Windows / Linux)

## 👥 Zielgruppen

| Zielgruppe | Nutzen |
|------------|--------|
| Einzelentwickler | Schnell neue Projekte mit wiederverwendbarem Setup starten |
| Teams / Firmen | Konsistente Build-Prozesse über mehrere Projekte |
| Open-Source Maintainer | Saubere Onboarding-Struktur & Beitragssysteme |
| CI/CD-Automatisierer | Klare Presets → deterministischer Build |
| Fortgeschrittene CMake-Nutzer | Erweiterbare Modularchitektur |

## 🧩 Was dieses Projekt löst

- Kein „Copy-Paste Chaos“ von alten CMakeLists
- Eindeutig definierte Rollen:
  - Projektkonfiguration ↔ Benutzerkonfiguration ↔ Build-Automation
- Konsistente Fehlermeldungen
- Presets statt Kommandozeilenargumentlisten
- JSON-Schema statt unkontrollierter Variablen
- Dokumentation als Teil des Systems

## 🚀 Quick Onboarding (5 Minuten)

Falls du neu im Projekt bist oder das System zum ersten Mal ausführst:

### 1️⃣ Repository klonen

```bash
git clone <URL>
cd <projekt>
```

### 2️⃣ Lokale Benutzerkonfiguration (optional)

Wenn du eigene Pfade / vcpkg / Toolchains verwendest:

```bash
copy CMakeUserPresets.example.json CMakeUserPresets.json   # Windows
cp CMakeUserPresets.example.json CMakeUserPresets.json     # Linux/Mac
```

Bearbeite `CMakeUserPresets.json` um Speicherorte oder Toolchains anzupassen.

### 3️⃣ Build ausführen

Beispiel – Visual Studio Projekt:

```bash
cmake --preset vs-debug
cmake --build --preset build
```

Beispiel – Ninja Release Build:

```bash
cmake --preset ninja-release
cmake --build --preset build
```

## 📚 Weiterführende Dokumentation

weiterführende Dokumentationen werden unter [Documentations](Documentations/de/README.md) bereitgestellt.  


| Thema | Datei |
|-------|------|
| Funktionsweise der Presets | `/Documentations/References/CMakePresets_Manual.md` |
| Struktur von Solution JSON | `/Documentations/References/Solution_Schema.md` |
| CMake-Architektur-Blueprint | `/Documentations/Blueprints/CMake_Blueprint.md` |
| Dokumentationsstandard | `/Documentations/Blueprints/Documentation_Blueprint.md` |
| Modulübersicht | `/Documentations/Modules/core/*` |

## 🧭 Status

Dieses System befindet sich in aktiver Weiterentwicklung. Änderungen an den Blueprints und Guidelines wirken sich direkt auf die Modularchitektur aus.


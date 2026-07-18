# Preset System — Benutzerhandbuch

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** Guide  
> **Status:** Stabil  
> **Zielgruppe:** C++ Entwickler, Endbenutzer  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Überblick](#1-überblick)
2. [Voraussetzungen](#2-voraussetzungen)
3. [Schnellstart](#3-schnellstart)
4. [Wie funktioniert das Dropdown?](#4-wie-funktioniert-das-dropdown)
5. [Wie speichere ich ein Preset?](#5-wie-speichere-ich-ein-preset)
6. [Wie lade ich ein Preset?](#6-wie-lade-ich-ein-preset)
7. [Wie lösche ich ein Preset?](#7-wie-lösche-ich-ein-preset)
8. [Wie teile ich Presets?](#8-wie-teile-ich-presets)
9. [Stolpersteine und Lösungen](#9-stolpersteine-und-lösungen)
10. [Troubleshooting](#10-troubleshooting)
11. [Siehe auch](#11-siehe-auch)
12. [Changelog](#12-changelog)

---

## 1. Überblick

Das Preset-System ermöglicht das Speichern, Laden und Teilen von Visualizer-Konfigurationen auf verschiedenen Ebenen.

### Preset-Hierarchie

```
┌─────────────────────────────────────────────────────────┐
│              Visualizer Preset (.lvp)                   │
│              Speichert ALLE Parameter                   │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐ │
│  │   Audio     │  │  Smoothing  │  │    Gradient     │ │
│  │  (.audio)   │  │  (.smooth)  │  │    (.grad)      │ │
│  └─────────────┘  └─────────────┘  └─────────────────┘ │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Preset-Typen

| Typ | Beschreibung | Dateiendung |
|-----|--------------|-------------|
| **Visualizer** | Komplette Konfiguration | `.lvp` |
| **Audio** | FFT-Settings + Smoothing | `.audio` |
| **Smoothing** | Nur Glättungsparameter | `.smooth` |
| **Gradient** | Farbverlauf mit Stops | `.grad` |

---

## 2. Voraussetzungen

### Für Entwickler

- [ ] Qt6 für QStandardPaths
- [ ] nlohmann/json für Serialisierung
- [ ] Schreibrechte im AppData-Verzeichnis

### Für Endbenutzer

- [ ] MyViz installiert
- [ ] Mindestens ein Visualizer aktiv

---

## 3. Schnellstart

### Für Entwickler: System initialisieren

```cpp
#include <QStandardPaths>

void Application::initPresets()
{
    QString base = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/presets";
    
    // Module-Verzeichnisse
    SmoothingModule::setUserPresetsDirectory(
        (base + "/smoothing").toStdString());
    AudioSourceModule::setUserPresetsDirectory(
        (base + "/audio").toStdString());
    ColorGradientModule::setUserPresetsDirectory(
        (base + "/gradients").toStdString());
    
    // Visualizer-Verzeichnis
    m_presetManager->setPresetsDirectory(base + "/visualizer");
}
```

### Für Endbenutzer: Preset speichern

1. Passe die Parameter nach Wunsch an
2. Klicke auf **Save** neben dem Preset-Dropdown
3. Gib einen Namen ein (z.B. "MeinPreset")
4. Klicke **OK**

---

## 4. Wie funktioniert das Dropdown?

### Dropdown-Struktur

Jedes Preset-Dropdown hat diese Struktur:

```
┌────────────────────────────┐
│ [Custom]              ▼    │
├────────────────────────────┤
│ [Custom]       ← Index 0   │  Manuell geändert
│ Default        ← Index 1   │  Builtin-Default
│ ───────────    ← Index 2   │  Separator (deaktiviert)
│ Fire           ← Index 3   │  Builtin-Preset
│ Ocean          ← Index 4   │  Builtin-Preset
│ ───────────    ← Separator │  (falls User-Presets)
│ MeinPreset     ← Index N   │  User-Preset
└────────────────────────────┘
```

### [Custom] — Wann erscheint es?

Das Dropdown springt automatisch auf **[Custom]**, wenn:

- Ein Parameter manuell geändert wird
- Der Wert nicht mehr einem bekannten Preset entspricht

```cpp
// Beispiel: User ändert timeMs
viz.setParam("audio.smooth.timeMs", 75.0f);
// → audio.smooth.preset wird automatisch [Custom]
// → Visualizer-Preset wird auch [Custom]
```

### Separator

Der Separator (`---`) ist:

- **Nicht auswählbar** (disabled)
- Trennt Builtin-Presets von User-Presets
- Erscheint nur, wenn User-Presets existieren

---

## 5. Wie speichere ich ein Preset?

### 5.1 Per UI (Endbenutzer)

1. Stelle alle Parameter wie gewünscht ein
2. Klicke **Save** neben dem Preset-Dropdown
3. Wähle einen Namen:
   - ✅ `MeinPreset`
   - ✅ `Preset_v2`
   - ✅ `Bass Heavy`
   - ❌ `Mein/Preset` (kein `/`)
   - ❌ `Preset.v2` (kein `.`)
4. Bestätige mit **OK**

### 5.2 Per Code (Entwickler)

**Modul-Preset:**

```cpp
// Smoothing
smooth.setAlgorithm(SmoothingAlgorithm::EMA);
smooth.setTimeMs(100.0f);
smooth.savePreset("UltraSmooth");

// Audio
audio.setGain(2.0f);
audio.setBands(128);
audio.savePreset("HighDetail");

// Gradient
gradient.clearStops();
gradient.addStop(0.0f, {1, 0, 0, 1});
gradient.addStop(1.0f, {0, 0, 1, 1});
gradient.savePreset("RedToBlue");
```

**Visualizer-Preset:**

```cpp
// Alle aktuellen Parameter erfassen
auto preset = m_presetManager->capturePreset(&viz, "MeinPreset");

// In Datei speichern
if (!m_presetManager->savePreset(preset))
{
    qWarning() << "Preset konnte nicht gespeichert werden";
}
```

### 5.3 Speicherort

Die Dateien werden hier gespeichert:

| OS | Pfad |
|----|------|
| Windows | `%APPDATA%/MyViz/presets/` |
| Linux | `~/.local/share/MyViz/presets/` |
| macOS | `~/Library/Application Support/MyViz/presets/` |

**Unterordner:**

```
presets/
├── visualizer/
│   └── pulsing/
│       └── MeinPreset.lvp
├── smoothing/
│   └── UltraSmooth.smooth
├── audio/
│   └── HighDetail.audio
└── gradients/
    └── RedToBlue.grad
```

---

## 6. Wie lade ich ein Preset?

### 6.1 Per UI (Endbenutzer)

1. Öffne das Preset-Dropdown
2. Wähle das gewünschte Preset
3. Die Parameter werden sofort angewendet

### 6.2 Per Code (Entwickler)

**Modul-Preset laden:**

```cpp
// Per Name
smooth.loadPreset("UltraSmooth");
gradient.loadPreset("RedToBlue");

// Per Index (im Parameter-System)
viz.setParam("audio.smooth.preset", 3);  // Index 3
```

**Visualizer-Preset laden:**

```cpp
auto preset = m_presetManager->loadPreset("pulsing", "MeinPreset");

if (preset)
{
    m_presetManager->applyPreset(&viz, *preset);
    
    // UI aktualisieren
    m_configPanel->syncFromVisualizer();
}
else
{
    qWarning() << "Preset nicht gefunden";
}
```

### 6.3 Preset-Liste abrufen

```cpp
// Modul-Presets
std::vector<std::string> names = smooth.presetNames();
// → ["[Custom]", "Instant", "Reactive", "Balanced", ...]

// Visualizer-Presets
QStringList presets = m_presetManager->availablePresets("pulsing");
// → ["Default", "MeinPreset", ...]
```

---

## 7. Wie lösche ich ein Preset?

### 7.1 Per UI (Endbenutzer)

1. Wähle das zu löschende Preset im Dropdown
2. Klicke **Delete** (nur bei User-Presets sichtbar)
3. Bestätige die Löschung

### 7.2 Per Code (Entwickler)

```cpp
// Modul-Preset
bool deleted = smooth.deletePreset("MeinPreset");

if (!deleted)
{
    // Preset existiert nicht oder ist Builtin
    qWarning() << "Konnte Preset nicht löschen";
}

// Visualizer-Preset
m_presetManager->deletePreset("pulsing", "MeinPreset");
```

### 7.3 Wichtige Hinweise

- **Builtin-Presets können nicht gelöscht werden**
- Die Löschung ist **permanent** (kein Papierkorb)
- Nach Löschung springt Dropdown auf **Default**

---

## 8. Wie teile ich Presets?

### 8.1 Preset exportieren

1. Navigiere zum Preset-Verzeichnis (siehe Speicherort)
2. Kopiere die gewünschte Datei:
   - `MeinPreset.lvp` für Visualizer
   - `MeinSmooth.smooth` für Smoothing
   - etc.

### 8.2 Preset importieren

1. Kopiere die Preset-Datei in den passenden Unterordner
2. Starte die Anwendung neu (oder aktualisiere die Liste)
3. Das Preset erscheint im Dropdown

### 8.3 Preset-Pakete erstellen

Für größere Preset-Sammlungen:

```
MeinePresets/
├── visualizer/
│   └── pulsing/
│       ├── Energetic.lvp
│       └── Chill.lvp
├── smoothing/
│   ├── Reactive.smooth
│   └── UltraSmooth.smooth
└── gradients/
    ├── Neon.grad
    └── Sunset.grad
```

Anweisung für Benutzer:
> "Kopiere den Inhalt von `MeinePresets/` nach `%APPDATA%/MyViz/presets/`"

---

## 9. Stolpersteine und Lösungen

### 9.1 Preset wird nicht gespeichert

**Problem:** `savePreset()` gibt false zurück.

**Ursachen:**
1. Verzeichnis nicht gesetzt
2. Keine Schreibrechte
3. Ungültiger Name

**Lösung:**

```cpp
// 1. Verzeichnis prüfen
qDebug() << "Path:" << QString::fromStdString(
    SmoothingModule::getUserPresetsDirectory());

// 2. Verzeichnis erstellen falls nötig
QDir dir(path);
if (!dir.exists())
{
    dir.mkpath(".");
}

// 3. Gültigen Namen verwenden
QString name = "MeinPreset";
name.remove(QRegularExpression("[/\\\\.]"));
```

---

### 9.2 User-Presets erscheinen nicht im Dropdown

**Problem:** Gespeicherte Presets fehlen in der Liste.

**Ursachen:**
1. Falscher Speicherort
2. Falsche Dateiendung
3. Liste nicht aktualisiert

**Lösung:**

```cpp
// 1. Dateien prüfen
qDebug() << "Files:" << QDir(path).entryList({"*.smooth"});

// 2. Liste manuell aktualisieren
m_configPanel->refreshPresetDropdowns();

// 3. Pfad zur Laufzeit prüfen
for (const auto& name : smooth.presetNames())
{
    qDebug() << QString::fromStdString(name);
}
```

---

### 9.3 [Custom] bleibt nicht ausgewählt

**Problem:** Nach Änderung springt Dropdown zurück.

**Ursache:** Signal-Loop zwischen UI und Visualizer.

**Lösung:**

```cpp
void ConfigPanel::onParamChanged(...)
{
    // Signale temporär blockieren
    QSignalBlocker blocker(m_presetCombo);
    
    m_visualizer->setParam(paramId, value);
    m_presetCombo->setCurrentIndex(0);  // [Custom]
}
```

---

### 9.4 Preset lädt falsche Werte

**Problem:** Nach dem Laden sind Parameter anders als erwartet.

**Ursache:** Preset wurde mit anderer Modul-Version erstellt.

**Lösung:**

```cpp
// Version im Preset prüfen
auto preset = loadPreset("MeinPreset");
if (preset && preset->version != "1.0")
{
    qWarning() << "Preset-Version inkompatibel";
    migratePreset(preset);
}
```

---

## 10. Troubleshooting

### Checkliste

- [ ] Ist das Preset-Verzeichnis gesetzt?
- [ ] Existiert das Verzeichnis?
- [ ] Hat die Anwendung Schreibrechte?
- [ ] Ist der Preset-Name gültig (kein `/`, `\`, `.`)?
- [ ] Stimmt die Dateiendung?

### Häufige Fehler

| Symptom | Ursache | Lösung |
|---------|---------|--------|
| Preset fehlt | Falscher Ordner | Pfad prüfen |
| Speichern scheitert | Keine Rechte | Als Admin ausführen |
| Delete funktioniert nicht | Builtin-Preset | Nur User-Presets löschbar |
| Werte falsch | Version alt | Preset neu erstellen |
| Dropdown leer | Verzeichnis nicht gesetzt | `setUserPresetsDirectory()` |

### Debug-Modus

```cpp
// Alle Pfade ausgeben
qDebug() << "Smoothing:" 
         << QString::fromStdString(SmoothingModule::getUserPresetsDirectory());
qDebug() << "Audio:" 
         << QString::fromStdString(AudioSourceModule::getUserPresetsDirectory());
qDebug() << "Gradient:" 
         << QString::fromStdString(ColorGradientModule::getUserPresetsDirectory());

// Preset-Inhalt ausgeben
QFile file(path + "/MeinPreset.smooth");
if (file.open(QIODevice::ReadOnly))
{
    qDebug() << file.readAll();
}
```

---

## 11. Siehe auch

### Referenzen

- [FileFormat_Reference.md](reference/FileFormat_Reference.md) — JSON-Schemas
- [Parameter_Reference.md](reference/Parameter_Reference.md) — Preset-Parameter

### Modul-Dokumentation

- [Preset_System.md](modules/Preset_System.md) — API-Details
- [SmoothingModule.md](modules/SmoothingModule.md) — .smooth Format
- [ColorGradientModule.md](modules/ColorGradientModule.md) — .grad Format

---

## 12. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: Vollständiges Benutzerhandbuch** |

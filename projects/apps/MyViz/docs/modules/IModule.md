# IModule — Base Interface für Visualizer-Module

> **Version:** 1.0.0  
> **Datum:** 2026-01-02  
> **Typ:** CppModuleDoc  
> **Status:** Stabil  
> **Modul:** lumi::modules::IModule  
> **Dateien:** IModule.hpp  
> **Namespace:** lumi::modules  
> **Abhängigkeiten:** glm, std::variant  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Abhängigkeiten](#2-abhängigkeiten)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Interna](#5-interna)
6. [Thread-Sicherheit](#6-thread-sicherheit)
7. [Fehlerbehandlung](#7-fehlerbehandlung)
8. [Siehe auch](#8-siehe-auch)
9. [Changelog](#9-changelog)

---

## 1. Übersicht

### 1.1 Zweck

IModule definiert das **Interface für alle LumiPulse Visualizer-Module**. Es ermöglicht einheitliche Parameter-Introspection, Runtime-Zugriff auf Parameter und standardisierte Lifecycles.

### 1.2 Verantwortlichkeiten

- Definition des Parameter-Systems (ParamValue, ParamType, ModuleParamDesc)
- Einheitliche Schnittstelle für `getParam()` / `setParam()`
- Fluent API für Parameter-Definition (ParamBuilder)
- Lifecycle-Hooks (`update`, `resetToDefaults`)

### 1.3 Nicht-Verantwortlichkeiten

- Keine konkrete Implementierung von Algorithmen
- Keine UI-Logik (→ ConfigPanel)
- Keine Persistenz (→ Preset-System)

---

## 2. Abhängigkeiten

| Dependency | Typ | Zweck |
|------------|-----|-------|
| `glm` | Extern | vec2, vec3 für Parameter |
| `std::variant` | Standard | ParamValue Typ |
| `std::vector` | Standard | Parameter-Listen |
| `std::string` | Standard | Parameter-IDs |

---

## 3. API

### 3.1 ParamValue

```cpp
using ParamValue = std::variant<
    std::monostate,  // 0: Empty
    int,             // 1: Integer
    float,           // 2: Float
    bool,            // 3: Boolean
    std::string,     // 4: String
    glm::vec2,       // 5: Vec2
    glm::vec3,       // 6: Vec3
    Color4f          // 7: RGBA Color
>;
```

### 3.2 ParamType Enum

| Wert | Beschreibung |
|------|--------------|
| `Int` | Integer mit min/max |
| `Float` | Float mit min/max/step |
| `Bool` | Checkbox |
| `Enum` | Dropdown mit Options |
| `String` | Textfeld |
| `Vec2` | 2D Vector |
| `Vec3` | 3D Vector |
| `Color4f` | RGBA Farbwähler |
| `Button` | Aktions-Button |

### 3.3 ModuleParamDesc

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `id` | `std::string` | Eindeutige Parameter-ID |
| `type` | `ParamType` | Datentyp |
| `displayName` | `std::string` | UI-Anzeigename |
| `tooltip` | `std::string` | Tooltip-Text |
| `minValue` | `ParamValue` | Minimum (Int/Float) |
| `maxValue` | `ParamValue` | Maximum (Int/Float) |
| `defaultValue` | `ParamValue` | Standardwert |
| `enumOptions` | `std::vector<std::string>` | Enum-Optionen |
| `subGroup` | `std::string` | UI-Gruppierung |
| `order` | `int` | Sortierreihenfolge |
| `dependsOn` | `std::string` | Visibility-Abhängigkeit |
| `dependsValues` | `std::vector<ParamValue>` | Werte für Sichtbarkeit |

### 3.4 IModule Interface

| Methode | Parameter | Rückgabe | Beschreibung |
|---------|-----------|----------|--------------|
| `moduleId()` | — | `const char*` | Eindeutige Modul-ID |
| `displayName()` | — | `const char*` | UI-Anzeigename |
| `category()` | — | `const char*` | Kategorie (Source, Processing, etc.) |
| `description()` | — | `const char*` | Beschreibungstext |
| `paramDescs()` | — | `std::vector<ModuleParamDesc>` | Alle Parameter-Definitionen |
| `getParam(id, out)` | `string`, `ParamValue&` | `bool` | Parameter abfragen |
| `setParam(id, value)` | `string`, `ParamValue` | `bool` | Parameter setzen |
| `resetToDefaults()` | — | `void` | Auf Standardwerte zurücksetzen |
| `update(deltaTime)` | `float` | `void` | Frame-Update (optional) |

### 3.5 ParamBuilder (Fluent API)

| Methode | Parameter | Beschreibung |
|---------|-----------|--------------|
| `ParamBuilder(id, type)` | `string`, `ParamType` | Konstruktor |
| `displayName(name)` | `string` | UI-Name setzen |
| `tooltip(text)` | `string` | Tooltip setzen |
| `range(min, max)` | `T`, `T` | Min/Max für Int/Float |
| `defaultValue(val)` | `T` | Standardwert |
| `enumOptions(opts)` | `vector<string>` | Enum-Optionen |
| `subGroup(name)` | `string` | UI-Gruppe |
| `order(n)` | `int` | Sortierung |
| `dependsOn(id, values)` | `string`, `vector<T>` | Visibility |
| `build()` | — | `ModuleParamDesc` |

---

## 4. Verwendung

### 4.1 Modul implementieren

```cpp
class MyModule : public IModule
{
public:
    const char* moduleId() const override { return "myModule"; }
    const char* displayName() const override { return "My Module"; }
    const char* category() const override { return "Processing"; }
    const char* description() const override { return "Example module"; }
    
    std::vector<ModuleParamDesc> paramDescs() const override
    {
        return {
            ParamBuilder("intensity", ParamType::Float)
                .displayName("Intensity")
                .range(0.0f, 1.0f)
                .defaultValue(0.5f)
                .build()
        };
    }
    
    bool getParam(const std::string& id, ParamValue& out) const override
    {
        if (id == "intensity") { out = m_intensity; return true; }
        return false;
    }
    
    bool setParam(const std::string& id, const ParamValue& value) override
    {
        if (id == "intensity") {
            m_intensity = std::get<float>(value);
            return true;
        }
        return false;
    }

private:
    float m_intensity = 0.5f;
};
```

### 4.2 ParamBuilder mit Visibility

```cpp
ParamBuilder("timeMs", ParamType::Float)
    .displayName("Time Constant")
    .range(1.0f, 500.0f)
    .defaultValue(50.0f)
    .tooltip("Smoothing time in milliseconds")
    .subGroup("Smoothing")
    .dependsOn("algorithm", {2, 4})  // Nur bei EMA(2) oder DEMA(4)
    .build();
```

---

## 5. Interna

### 5.1 Parameter-Hierarchie

Module können eingebettete Sub-Module haben. Parameter-Pfade verwenden Punkte:

```
"smooth.algorithm"       → SmoothingModule.algorithm
"audio.smooth.timeMs"    → AudioSourceModule → SmoothingModule → timeMs
```

### 5.2 Prefix-Handling

```cpp
bool AudioSourceModule::setParam(const std::string& id, const ParamValue& value)
{
    // Delegiere an embedded SmoothingModule
    if (id.rfind("smooth.", 0) == 0) {
        return m_smoothing.setParam(id.substr(7), value);
    }
    // Eigene Parameter...
}
```

---

## 6. Thread-Sicherheit

**Nicht thread-safe.** Parameter-Zugriffe müssen vom selben Thread erfolgen wie Rendering.

---

## 7. Fehlerbehandlung

- `getParam()` gibt `false` zurück bei unbekannter ID
- `setParam()` gibt `false` zurück bei unbekannter ID oder ungültigem Wert
- Keine Exceptions

---

## 8. Siehe auch

- [SmoothingModule.md](SmoothingModule.md) — Konkrete Implementierung
- [AudioSourceModule.md](AudioSourceModule.md) — FFT-Verarbeitung mit embedded Smoothing
- [ColorGradientModule.md](ColorGradientModule.md) — Farb-Gradients

---

## 9. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-01-02** | **Initial: IModule Interface, ParamBuilder, ParamValue** |

# CppModuleDoc — Standard für C++ Modul-Dokumentation

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Blueprint  
> **Status:** In Development  
> **Based on:** Doc v0.5, Blueprint v0.5  
> **Target Audience:** Entwickler  
> **Language:** English  
> **German:** [CppModuleDoc.md](../../en/blueprints/CppModuleDoc.md)

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Scope](#2-geltungsbereich)
3. [Header-Erweiterungen](#3-header-erweiterungen)
4. [Requiredabschnitte](#4-pflichtabschnitte)
5. [API-Dokumentationsformat](#5-api-dokumentationsformat)
6. [Interface vs. Implementation](#6-interface-vs-implementation)
7. [Example: Vollständige Moduldokumentation](#7-beispiel-vollständige-moduldokumentation)
8. [Review Checklist](#8-review-checkliste)
9. [See Also](#9-siehe-auch)
10. [Changelog](#10-changelog)

---

## 1. Overview

This blueprint defines die **Struktur für C++ Modul-Dokumentationen**. Eine Moduldokumentation beschreibt eine C++ Klasse oder ein Modul (zusammengehörige .hpp/.cpp/.tpp/.inl Dateien).

### 1.1 Zielgruppe

- Entwickler, die das Modul verwenden wollen
- Entwickler, die das Modul warten oder erweitern

### 1.2 Abgrenzung

| Dokumentations-Typ | Fragestellung | Example |
|-------------------|---------------|----------|
| **CppModuleDoc** | "Was macht diese Klasse und wie verwende ich sie?" | ShaderManager.md |
| **Concept** | "Wie ist die Gesamtarchitektur?" | MyVisualizer Concept |
| **Reference** | "Welche Error-Codes gibt es?" | ErrorCodes.md |

### 1.3 Dokumentations-Granularität

| Modul-Typ | Dokumentation |
|-----------|---------------|
| **Interface** (z.B. IShader) | Eigene Datei, fokussiert auf Vertrag |
| **Implementation** (z.B. OpenGLShader) | Eigene Datei, fokussiert auf konkrete Logik |
| **Zusammengehörig** (z.B. ShaderManager) | Eine Datei für .hpp + .cpp |
| **Header-Only** | Eine Datei für .hpp |

---

## 2. Scope

Dieser Blueprint gilt für Dokumentationen von:

- C++ Klassen (.hpp/.cpp)
- C++ Templates (.hpp/.tpp)
- Header-Only Libraries
- Interfaces (rein virtuelle Klassen)

**Dateinamen-Konvention:**

```
docs/[lang]/modules/[Namespace]/[Klassenname].md
```

**Examples:**

- `docs/de/modules/gpu/ShaderManager.md`
- `docs/de/modules/gpu/IShader.md`
- `docs/de/modules/visuals/NullVisualizer.md`

---

## 3. Header-Erweiterungen

### 3.1 Requiredfelder

| Feld | Description |
|------|--------------|
| `Module:` | Voll qualifizierter Klassenname |
| `Dateien:` | Liste der zugehörigen Dateien |
| `Namespace:` | C++ Namespace |
| `Dependencies:` | Externe und interne Dependencies |

### 3.2 Vollständiger Header

```markdown
# [Klassenname] — [Kurzbeschreibung]

> **Version:** X.Y.Z  
> **Date:** YYYY-MM-DD  
> **Type:** CppModuleDoc  
> **Status:** [Stable | In Development | Deprecated]  
> **Module:** [Namespace]::[Klassenname]  
> **Dateien:** [file.hpp], [file.cpp]  
> **Namespace:** [namespace]  
> **Dependencies:** [Liste]  
> **Target Audience:** Entwickler  
> **Language:** English  
```

---

## 4. Requiredabschnitte

### 4.1 Struktur für Implementationen

```
## 1. Overview
### 1.1 Zweck
### 1.2 Verantwortlichkeiten
### 1.3 Nicht-Verantwortlichkeiten

## 2. Dependencies

## 3. API
### 3.1 Konstruktion
### 3.2 Öffentliche Methoden
### 3.3 Signale/Slots (falls Qt)

## 4. Usage
### 4.1 Einfaches Example
### 4.2 Fortgeschrittenes Example (optional)

## 5. Interna (optional)
### 5.1 Implementationsdetails
### 5.2 Pimpl (falls verwendet)

## 6. Thread-Sicherheit

## 7. Errorbehandlung

## Changelog
```

### 4.2 Struktur für Interfaces

```
## 1. Overview
### 1.1 Zweck
### 1.2 Vertrag

## 2. Dependencies

## 3. Interface-Definition
### 3.1 Rein virtuelle Methoden
### 3.2 Virtuelle Methoden mit Default
### 3.3 Nicht-virtuelle Methoden

## 4. Implementationsanforderungen

## 5. Bekannte Implementationen

## Changelog
```

### 4.3 Abschnitts-Details

| # | Abschnitt | Inhalt | Required |
|---|-----------|--------|---------|
| 1 | Overview | Zweck, Verantwortlichkeiten | ✅ |
| 2 | Dependencies | Externe/Interne Dependencies | ✅ |
| 3 | API | Methoden-Dokumentation | ✅ |
| 4 | Usage | Code-Examples | ✅ |
| 5 | Interna | Implementation Details | Optional |
| 6 | Thread-Sicherheit | Concurrency-Garantien | ✅ |
| 7 | Errorbehandlung | Exceptions, Error-Codes | ✅ |

---

## 5. API-Dokumentationsformat

### 5.1 Methoden-Tabelle

```markdown
### 3.2 Öffentliche Methoden

| Methode | Parameters | Rückgabe | Description |
|---------|-----------|----------|--------------|
| `load(path)` | `const std::string&` | `bool` | Lädt Ressource, `true` bei Success |
| `get(name)` | `const std::string&` | `T*` | Gibt gecachte Ressource oder `nullptr` |
```

### 5.2 Detaillierte Methoden-Dokumentation

Für komplexe Methoden zusätzlich:

```markdown
#### `bool load(const std::string& path)`

Lädt eine Ressource aus dem angegebenen Pfad.

**Parameters:**
- `path` — Relativer oder absoluter Dateipfad

**Rückgabe:**
- `true` — Ressource erfolgreich geladen
- `false` — Error (siehe `getErrorLog()`)

**Exceptions:**
- Keine (Error über Return-Wert)

**Example:**
```cpp
if (!manager.load("shaders/basic.vert"))
{
    std::cerr << manager.getErrorLog();
}
```
```

### 5.3 Signale/Slots (Qt)

```markdown
### 3.3 Signale

| Signal | Parameters | Description |
|--------|-----------|--------------|
| `initialized()` | — | Emittiert nach erfolgreicher Initialisierung |
| `errorOccurred(msg)` | `QString` | Emittiert bei Errorn |

### 3.4 Slots

| Slot | Parameters | Description |
|------|-----------|--------------|
| `setPaused(paused)` | `bool` | Pausiert/Fortsetzt Rendering |
```

---

## 6. Interface vs. Implementation

### 6.1 Interface-Dokumentation

Fokus auf **Vertrag** (was muss eine Implementation erfüllen?):

```markdown
## 3. Interface-Definition

### 3.1 Rein virtuelle Methoden

| Methode | Vertrag |
|---------|---------|
| `initialize(ctx)` | Muss Ressourcen allokieren, `true` bei Success |
| `render(ctx)` | Muss Frame rendern, darf nicht blockieren |
| `cleanup()` | Muss alle Ressourcen freigeben |

### Invarianten

- Nach `initialize()` mit `true`: `isInitialized()` gibt `true` zurück
- Nach `cleanup()`: Objekt kann erneut initialisiert werden
```

### 6.2 Implementation-Dokumentation

Fokus auf **konkrete Logik**:

```markdown
## 5. Interna

### 5.1 OpenGL-Ressourcen

| Ressource | Typ | Lebenszyklus |
|-----------|-----|--------------|
| `m_program` | `GLuint` | `initialize()` → `cleanup()` |
| `m_vao` | `GLuint` | `initialize()` → `cleanup()` |

### 5.2 Render-Pipeline

1. `glUseProgram(m_program)`
2. `glBindVertexArray(m_vao)`
3. `glDrawArrays(...)`
```

---

## 7. Example: Vollständige Moduldokumentation

```markdown
# ShaderManager — Shader-Verwaltung und Caching

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** CppModuleDoc  
> **Status:** Stable  
> **Module:** myvis::gpu::ShaderManager  
> **Dateien:** ShaderManager.hpp, ShaderManager.cpp  
> **Namespace:** myvis::gpu  
> **Dependencies:** OpenGLShader, IShader, glad, glm  
> **Target Audience:** Entwickler  
> **Language:** English  

---

## Table of Contents

1. [Overview](#1-übersicht)
2. [Dependencies](#2-abhängigkeiten)
3. [API](#3-api)
4. [Usage](#4-verwendung)
5. [Thread-Sicherheit](#5-thread-sicherheit)
6. [Errorbehandlung](#6-fehlerbehandlung)

---

## 1. Overview

### 1.1 Zweck

ShaderManager verwaltet das Laden, Kompilieren und Cachen von GLSL-Shadern.

### 1.2 Verantwortlichkeiten

- Shader aus Source-Code oder Datei laden
- Kompilierte Shader cachen
- Einheitliche Interface für Shader-Zugriff

### 1.3 Nicht-Verantwortlichkeiten

- Keine Uniform-Verwaltung (→ IShader)
- Kein automatisches Hot-Reload

---

## 2. Dependencies

| Dependency | Typ | Zweck |
|------------|-----|-------|
| OpenGLShader | Intern | Konkrete Shader-Implementation |
| IShader | Intern | Interface für polymorphen Zugriff |
| glad | Extern | OpenGL Functions |
| glm | Extern | Mathematik für Uniforms |

---

## 3. API

### 3.1 Konstruktion

```cpp
ShaderManager();
~ShaderManager();
```

### 3.2 Öffentliche Methoden

| Methode | Parameters | Rückgabe | Description |
|---------|-----------|----------|--------------|
| `loadFromSource(name, vert, frag)` | `string, string, string` | `IShader*` | Kompiliert Shader aus Source |
| `loadFromFile(name, vertPath, fragPath)` | `string, string, string` | `IShader*` | Lädt und kompiliert aus Dateien |
| `get(name)` | `string` | `IShader*` | Gibt gecachten Shader oder `nullptr` |
| `remove(name)` | `string` | `void` | Entfernt Shader aus Cache |
| `clear()` | — | `void` | Entfernt alle Shader |
| `getErrorLog()` | — | `string` | Letzter Error |

---

## 4. Usage

### 4.1 Einfaches Example

```cpp
ShaderManager shaders;

auto* basic = shaders.loadFromSource(
    "basic",
    R"(#version 330 core
       layout(location=0) in vec3 aPos;
       void main() { gl_Position = vec4(aPos, 1.0); })",
    R"(#version 330 core
       out vec4 FragColor;
       void main() { FragColor = vec4(1.0); })"
);

if (basic)
{
    basic->bind();
    // Rendern...
    basic->unbind();
}
```

---

## 5. Thread-Sicherheit

**Nicht thread-safe.** Alle Aufrufe müssen vom GL-Context-Thread erfolgen.

---

## 6. Errorbehandlung

- Bei Error: Methode gibt `nullptr` zurück
- Errordetails: `getErrorLog()` enthält GLSL-Compiler-Error
- Keine Exceptions

---

## Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| 1.0.0 | 2025-12-23 | Initial |
```

---

## 8. Review Checklist

**Header:**

- [ ] Modul-Name vollqualifiziert (mit Namespace)
- [ ] Dateien aufgelistet
- [ ] Dependencies dokumentiert

**Inhalt:**

- [ ] Zweck klar beschrieben
- [ ] Verantwortlichkeiten definiert
- [ ] Alle öffentlichen Methoden dokumentiert
- [ ] Mindestens ein Usagesbeispiel
- [ ] Thread-Sicherheit dokumentiert
- [ ] Errorbehandlung dokumentiert

**API-Dokumentation:**

- [ ] Parameters mit Typen
- [ ] Return Values mit Bedeutung
- [ ] Exceptions/Errorfälle dokumentiert

**Code-Examples:**

- [ ] Kompilierbar (syntaktisch korrekt)
- [ ] Minimalistisch (nur relevanter Code)
- [ ] Kommentiert wo nötig

---

## 9. See Also

- [Doc.md](Doc.md) — Allgemeine Dokumentations-Regeln
- [Cpp.md](Cpp.md) — C++ Code-Standards
- [Concept.md](Concept.md) — Für Architecture-Dokumentation

---

## 10. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.1.0** | **2025-12-23** | **Initial: Struktur für C++ Modul-Dokumentation** |

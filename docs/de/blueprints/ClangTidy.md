# ClangTidy Blueprint – Statische Analyse

> **Version:** 1.0.0  
> **Datum:** 2025-12-05  
> **Typ:** Blueprint  
> **Status:** In Entwicklung (Pre-Release)  
> **Geltungsbereich:** Alle `.clang-tidy`-Konfigurationen  
> **Bezug:** Cpp_Coding_Standard v0.1, C_Coding_Standard v0.1, ClangFormat_Blueprint v0.1  
> **Sprache:** Deutsch  

---

## 1. Zweck

Dieses Blueprint definiert **Struktur und Wartung** der `.clang-tidy`-Konfiguration.

**Was dieses Dokument ist:**
- Konzeptuelle Grundlage für statische Analyse
- Definition der Check-Kategorien und Profile
- Erklärung der Namenskonventionen

**Was dieses Dokument nicht ist:**
- Die technische Konfiguration selbst (→ `.clang-tidy`-Datei)

---

## 2. Verhältnis zu anderen Standards

| Dokument | Definiert |
|----------|-----------|
| **Cpp_Coding_Standard** | Strukturelle und sprachliche Regeln |
| **C_Coding_Standard** | Embedded-spezifische Regeln |
| **`.clang-tidy`** | Technische Durchsetzung |

### Konfliktauflösung

| Konflikttyp | Priorität |
|-------------|-----------|
| Formatierung | `.clang-format` gewinnt |
| Naming/Statische Regeln | `.clang-tidy` gewinnt |
| Architektur/Funktional | Schriftlicher Standard gewinnt |

---

## 3. Philosophie und Strategie

### 3.1 Ziele

- Bugs früh erkennen
- Code-Klarheit verbessern
- Verantwortungsvolle Modernisierung (keine Stil-Kriege)
- Namens-Konsistenz durchsetzen
- MISRA/CERT-Alignment wo sinnvoll

### 3.2 Profile

| Profil | Zweck | Verwendung |
|--------|-------|------------|
| **Dev-Gentle** | Alltags-Entwicklung | Default |
| **Dev-Strict** | Schärfere Checks | Vor Merge |
| **CI-Strict** | Pull Requests / Merge Gates | CI Pipeline |
| **API-Gate** | Öffentliche Libraries | Höchste Stufe |

**Aktuelles Default-Profil:** `Dev-Gentle`

### 3.3 Verantwortlichkeit

- `.clang-tidy` wird zentral von Architektur/Tooling gepflegt
- Projekte können Änderungen vorschlagen
- Keine leichtfertigen Forks

---

## 4. Namenskonventionen

Durchgesetzt via `readability-identifier-naming`.

### 4.1 C++ (PC-Applikationen)

| Entität | Konvention | Beispiel |
|---------|------------|----------|
| Namespace | `lower_case` | `audio`, `core_utils` |
| Klasse/Struct/Enum | `CamelCase` | `LogManager`, `AudioBuffer` |
| Enum-Konstante | `CamelCase` | `LogLevelInfo`, `ColorRed` |
| Funktion/Methode | `camelBack` | `writeLog()`, `openFile()` |
| Parameter | `camelBack` | `filePath`, `bufferSize` |
| Lokale Variable | `camelBack` | `currentIndex`, `tempValue` |
| Member-Variable | `m_` Prefix | `m_buffer`, `m_logger` |
| Globale Konstante | `UPPER_CASE` | `MAX_BUFFER_SIZE` |
| Globale Variable | `g_` Prefix | `g_logger` |
| Statische interne Variable | `s_` Prefix | `s_cache` |
| Makro | `UPPER_CASE` | `DEBUG_LOG` |

### 4.2 C (Embedded)

| Entität | Konvention | Beispiel |
|---------|------------|----------|
| Modul-Funktion | `modul_snake_case` | `timer_init()`, `gpio_set()` |
| Typ-Alias | `CamelCase` oder `snake_case` | `TimerHandle`, `gpio_pin_t` |
| Lokale Variable | `snake_case` | `current_state` |
| Globale Variable | `g_` Prefix | `g_systemState` |
| Statische Variable | `s_` Prefix | `s_bufferIndex` |
| Konstante/Makro | `UPPER_CASE` | `ADC_TIMEOUT_TICKS` |

---

## 5. Check-Kategorien

### 5.1 Aktivierte Check-Gruppen

| Gruppe | Zweck | Priorität |
|--------|-------|-----------|
| `clang-analyzer-*` | UB, Leaks, Null-Deref | **Kritisch** |
| `bugprone-*` | Logik-Fehler | **Kritisch** |
| `performance-*` | Ineffiziente Patterns | Hoch |
| `readability-*` | Lesbarkeit, Naming | Mittel |
| `modernize-*` | C++-Modernisierung | Mittel |

### 5.2 Umgang mit Warnungen

| Check-Typ | Anforderung |
|-----------|-------------|
| `clang-analyzer-*`, `bugprone-*` | **Muss behoben** oder begründet unterdrückt werden |
| `modernize-*` | Fallweise bewerten, kein Zwang |
| `performance-*` | Aktiviert wo Klarheit nicht leidet |

### 5.3 Deaktivierte Checks

Folgende Checks sind im Dev-Gentle-Profil deaktiviert (zu viel Rauschen):

- `modernize-use-trailing-return-type` – Stilfrage
- `modernize-use-auto` – Explizite Typen oft lesbarer
- `readability-implicit-bool-conversion` – Zu strikt
- `readability-identifier-naming` – Optional aktivierbar

---

## 6. Magic Numbers

### 6.1 Erlaubte Werte

Automatisch toleriert:
- `-1`, `0`, `1`, `2`
- Alle Zweierpotenzen (für Bitmasken, Alignment)

### 6.2 Empfehlung

Domänenwerte (z.B. `44100` Hz, `2.2` Gamma) sollten als `constexpr` oder `enum` benannt werden:

```cpp
// ❌ Magic Number
setSampleRate(44100);

// ✅ Benannte Konstante
constexpr int SAMPLE_RATE_CD = 44100;
setSampleRate(SAMPLE_RATE_CD);
```

---

## 7. Value-Parameter-Optimierung

Bestimmte Typen dürfen per Value übergeben werden:

- `std::string_view`
- `std::span`
- `std::optional`

Dies entspricht modernen C++-Guidelines und vermeidet unnötige Indirektion.

---

## 8. Änderungsmanagement

### 8.1 Wann ändern?

Änderungen nur wenn:
- Regel verursacht wiederholt begründete Unterdrückungen
- Regel kollidiert mit Plattform-/Embedded-Constraints
- Neue Regel verbessert Sicherheit oder Klarheit

### 8.2 Prozess

1. **Vorschlag:** Motivation, Kategorien, Beispiel-Warnungen
2. **Review:** Tooling/Architektur-Freigabe
3. **Umsetzung:** `.clang-tidy` und Version-Tag aktualisieren
4. **Dokumentation:** Bei konzeptueller Änderung Blueprint aktualisieren

### 8.3 Datei-Header

```yaml
# C/C++ clang-tidy Konfiguration
# Version: 0.1.0
# Blueprint: Documentations/Blueprints/ClangTidy_Blueprint_v0_1_0.md
# Profil: Dev-Gentle
# Datum: 2025-12-05
```

---

## 9. Integration in CI und Entwicklung

| Kontext | Verwendung |
|---------|------------|
| IDE (on-save) | Optional, Dev-Gentle |
| Pre-Commit | Optional |
| CI Pipeline | Empfohlen, Dev-Strict oder CI-Strict |

### Profile via CLI

```bash
# Dev-Strict (Warnings als Errors für Analyzer/Bugprone)
clang-tidy -warnings-as-errors=clang-analyzer-*,bugprone-* ...

# API-Gate (alle Warnings als Errors)
clang-tidy -warnings-as-errors=* ...
```

---

## 10. Third-Party-Code

### Header-Filter

```yaml
HeaderFilterRegex: '^(?!.*[/\\](external|third_party|deps)[/\\]).*'
```

### Quellen

Third-Party-Quellen werden über CMake ausgeschlossen:

```cmake
# Kein clang-tidy für External
set_target_properties(ThirdPartyLib PROPERTIES
    CXX_CLANG_TIDY ""
)
```

---

## 11. Siehe auch

- [Cpp_Coding_Standard](../standards/Cpp_Coding_Standard.md) – C++ Stil-Richtlinien
- [C_Coding_Standard](../standards/C_Coding_Standard.md) – C Stil-Richtlinien
- [ClangFormat_Blueprint](ClangFormat.md) – Formatierung

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.0** | **2025-12-05** | **Initial: Profile, Check-Kategorien, Namenskonventionen** |

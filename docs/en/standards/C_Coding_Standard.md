# C Coding Standard — Stil-Richtlinien für Embedded

> **Version:** 1.0.0  
> **Date:** 2025-12-26  
> **Type:** Standard  
> **Status:** Stable  
> **Target Audience:** Embedded-Entwickler, Firmware-Entwickler  
> **Scope:** Alle C-Projekte (Embedded, Firmware)  
> **Durchsetzung:** clang-tidy, MISRA-Checker, Code Review  
> **Language:** English  
> **German:** [C_Coding_Standard.md](../../en/standards/C_Coding_Standard.md)

---

## Table of Contents

1. [Zweck und Scope](#1-zweck-und-geltungsbereich)
2. [Sprachversion und Compiler](#2-sprachversion-und-compiler)
3. [Grundprinzipien](#3-grundprinzipien)
4. [Datei-Header](#4-datei-header)
5. [Datei- und Modul-Organisation](#5-datei--und-modul-organisation)
6. [Formatierung](#6-formatierung)
7. [Namenskonventionen](#7-namenskonventionen)
8. [Struct-Layout](#8-struct-layout)
9. [Typen und Daten](#9-typen-und-daten)
10. [Pointer und Speicher](#10-pointer-und-speicher)
11. [Kontrollfluss](#11-kontrollfluss)
12. [Errorbehandlung](#12-fehlerbehandlung)
13. [Concurrency und Interrupts](#13-concurrency-und-interrupts)
14. [Hardware-Zugriff](#14-hardware-zugriff)
15. [Dokumentation](#15-dokumentation)
16. [MISRA und CERT](#16-misra-und-cert)
17. [Tests und Statische Analyse](#17-tests-und-statische-analyse)
18. [Verhältnis zu C++ (PC)](#18-verhältnis-zu-c-pc)
19. [Legacy-Code und Ausnahmen](#19-legacy-code-und-ausnahmen)
20. [See Also](#20-siehe-auch)
21. [Changelog](#21-changelog)

---

## 1. Zweck und Scope

Dieser Standard definiert **Coding-Konventionen für C** im Unternehmen, mit Fokus auf **Embedded-Systeme und Firmware**.

### Zielgruppe

Dieser Standard richtet sich an Entwickler, die C Code für Embedded-Systeme, Firmware und sicherheitskritische Anwendungen schreiben. Er ist verbindlich für alle neuen Embedded-Projekte.

### Anwendungsbereich

| Sprache | Fokus | Typische Projekte |
|---------|-------|-------------------|
| C | Embedded | Firmware, MCUs, sicherheitskritische Teile, Real-Time |
| **C++** | PC-Applikationen | Tools, GUIs, Services |

### Alignment

Dieser Standard orientiert sich an:
- **MISRA C:2012** — Sicherheit, Robustheit, UB-Vermeidung
- **SEI CERT C** — Sichere, defensive C-Programmierung

---

## 2. Sprachversion und Compiler

### 2.1 Standard-Version

| Projekt-Typ | C Standard |
|-------------|------------|
| Neue Embedded-Projekte | **C99** (Minimum) |
| Mit Toolchain-Support | C11 erlaubt |

### 2.2 Compiler-Erweiterungen

Erlaubt **nur wenn**:
- Notwendig für Hardware-Zugriff/Performance/Memory-Layout
- Klar dokumentiert an Abstraktionsgrenze

### 2.3 Undefined Behavior

> Code darf **niemals** auf Undefined Behavior oder unspezifiziertem Verhalten basieren.

---

## 3. Grundprinzipien

1. **Determinismus und Vorhersagbarkeit** über maximale Performance
2. **Sicherheit und Robustheit** über clevere Konstrukte
3. **Einfachheit und Klarheit** über Over-Engineering
4. **Statisch analysierbar** — Code muss prüfbar sein

---

## 4. Datei-Header

### 4.1 Standard-Header für C Dateien

Jede `.h` und `.c` Datei **muss** mit folgendem Doxygen-kompatiblen Header beginnen:

```c
/**
 ****************************************************************************************
 * @file   filename.h
 * @brief  Short description
 *         Optional second line for context
 *
 * @author Author Name
 * @date   Month YYYY
 ****************************************************************************************
 */
```

### 4.2 Requiredfelder

| Feld | Description |
|------|--------------|
| `@file` | Exakter Dateiname |
| `@brief` | Kurzbeschreibung (1-2 Zeilen) |
| `@author` | Hauptautor |
| `@date` | Erstellungsdatum (Monat Yeshr) |

### 4.3 Optionale Felder

| Feld | Usage |
|------|------------|
| `@version` | Bei versionierten Komponenten |
| `@copyright` | Bei speziellen Lizenzen |
| `@see` | Verweise auf verwandte Dateien |
| `@note` | Importante Notee (z.B. Hardware-Dependencies) |

### 4.4 Example

```c
/**
 ****************************************************************************************
 * @file   gpio_driver.h
 * @brief  GPIO Driver Interface
 *         Low-level GPIO control for TMS320F28P65x
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @note   Hardware: TMS320F28P650DK
 ****************************************************************************************
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <stdint.h>
// ...
```

### 4.5 Sprache

- **Englisch** ist Required für alle öffentlichen APIs und wiederverwendbare Module
- Projektspezifische interne Dateien können German verwenden

---

## 5. Datei- und Modul-Organisation

### 5.1 Dateinamen

| Regel | Example |
|-------|----------|
| **PascalCase** | `GpioDriver.h`, `TimerConfig.c` |
| **Gleich wie Modul/Haupttyp** | Modul `Timer` → `Timer.h`, `Timer.c` |
| **Modul-Präfix bei Bedarf** | `EpwmRegs.h`, `AdcHal.c` |

### 5.2 Dateiendungen

| Endung | Usage |
|--------|------------|
| `.h` | C Header |
| `.c` | C Implementation |

### 5.3 Ordnernamen

| Regel | Example |
|-------|----------|
| **lowercase** | `source/`, `include/`, `drivers/` |
| **snake_case** | `hal_layer/`, `device_support/` |

### 5.4 Include Guards

```c
#ifndef MODULE_NAME_H
#define MODULE_NAME_H

// ... content ...

#endif /* MODULE_NAME_H */
```

Oder `#pragma once` (falls Projektrichtlinie und Toolchain-Support).

### 5.5 Include-Reihenfolge

```c
// 1. Zugehöriger Header
#include "timer.h"

// 2. Standard-Header
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 3. Plattform/Vendor-Header
#include "device.h"
#include "F28x_Project.h"

// 4. Projekt-Header
#include "hal/gpio.h"
#include "config/board_config.h"
```

---

## 6. Formatierung

### 6.1 Autorität

- Formatierung via `clang-format` (wo verfügbar)
- Bei Embedded-Toolchains ohne clang-format: Manuelle Einhaltung

### 6.2 Overview

| Aspekt | Regel |
|--------|-------|
| Einrückung | 4 Spaces |
| Tabs | Nie verwenden |
| Klammern | **Allman-Stil** (neue Zeile) |
| Zeilenlänge | 80-100 Zeichen |
| Pointer | Links ausgerichtet (`int* ptr`) |

### 6.3 Klammern-Stil (Allman)

```c
// Functions
void function(void)
{
    // ...
}

// Structs
typedef struct
{
    uint32_t value;
    uint16_t flags;
} DataPacket;

// Kontrollstrukturen
if (condition)
{
    // ...
}
else
{
    // ...
}

// Schleifen
for (int i = 0; i < count; ++i)
{
    // ...
}
```

### 6.4 Leerzeichen

```c
// Nach Kommas
function(a, b, c);

// Um Operatoren
x = a + b * c;

// Vor Klammern bei Kontrollstrukturen
if (condition)
while (running)
for (i = 0; i < n; i++)

// Keine Leerzeichen in Klammern
function(a);      // ✅
function( a );    // ❌
```

---

## 7. Namenskonventionen

### 7.1 Overview

| Entität | Konvention | Example |
|---------|------------|----------|
| Modul-Funktion | `modul_snake_case` | `timer_init()`, `gpio_set_pin()` |
| Lokale Variable | `snake_case` | `current_index`, `buffer_size` |
| Globale Variable | `g_` Präfix | `g_systemState` |
| Statische Variable | `s_` Präfix | `s_bufferIndex` |
| Konstante/Makro | `UPPER_CASE` | `MAX_BUFFER_SIZE`, `ADC_TIMEOUT` |
| Typ-Alias | `PascalCase` oder `_t` Suffix | `TimerHandle`, `gpio_pin_t` |
| Struct-Member | `snake_case` | `data_length`, `is_valid` |

### 7.2 Modul-Prefix

Functions erhalten Modul-Präfix für Namensraum-Emulation:

```c
// Timer module
void timer_init(void);
void timer_start(TimerHandle handle);
void timer_stop(TimerHandle handle);

// GPIO module
void gpio_init(void);
void gpio_set_pin(uint8_t pin, bool state);
uint8_t gpio_read_pin(uint8_t pin);

// EPWM module
void epwm_configure(uint16_t period);
void epwm_set_duty(uint16_t cmpa, uint16_t cmpb);
```

### 7.3 Präfixe

| Präfix | Bedeutung | Example |
|--------|-----------|----------|
| `g_` | Globale Variable | `g_systemTicks` |
| `s_` | Statische Variable (file scope) | `s_initialized` |
| `p` | Pointer | `pBuffer`, `pConfig` |
| `m_` | Member (in Struct-Kontexten, optional) | `m_state` |
| — | Lokale Variable / Parameters | `count`, `index`, `value` |

### 7.4 Register-Konventionen

| Element | Konvention | Example |
|---------|------------|----------|
| Register-Struct | `UPPER_CASE` + `_REGS` | `EPWM_REGS` |
| Bitfeld | `snake_case` oder vendor-style | `TBCTL.bit.CTRMODE` |
| Register-Adresse | `UPPER_CASE` + `_BASE` | `EPWM1_BASE` |

---

## 8. Struct-Layout

### 8.1 Reihenfolge der Elemente

```c
/**
 * @brief Configuration structure for Timer module
 */
typedef struct
{
    /* ═══════════════════════════════════════════════════════════════ */
    /* Configuration Parameterss                                        */
    /* ═══════════════════════════════════════════════════════════════ */
    uint32_t period;            /**< Timer period in ticks */
    uint16_t prescaler;         /**< Clock prescaler value */
    uint8_t  mode;              /**< Operating mode */

    /* ═══════════════════════════════════════════════════════════════ */
    /* State Variables                                                 */
    /* ═══════════════════════════════════════════════════════════════ */
    volatile uint32_t counter;  /**< Current counter value */
    volatile bool     running;  /**< Timer running flag */

    /* ═══════════════════════════════════════════════════════════════ */
    /* Callbacks                                                       */
    /* ═══════════════════════════════════════════════════════════════ */
    void (*callback)(void);     /**< Overflow callback function */
} TimerConfig;
```

### 8.2 Alignment und Padding

```c
// ✅ Gut: Größere Typen zuerst (minimiert Padding)
typedef struct
{
    uint32_t timestamp;     // 4 bytes
    uint16_t id;            // 2 bytes
    uint8_t  flags;         // 1 byte
    uint8_t  reserved;      // 1 byte (explizites Padding)
} Message;  // Total: 8 bytes, no hidden padding

// ❌ Schlecht: Ineffizientes Layout
typedef struct
{
    uint8_t  flags;         // 1 byte + 3 padding
    uint32_t timestamp;     // 4 bytes
    uint8_t  id;            // 1 byte + 3 padding
} BadMessage;  // Total: 12 bytes with hidden padding
```

### 8.3 Best Practices

- **Dokumentation** für jedes Member (Doxygen `/**< */`)
- **Explizites Padding** statt implizitem Compiler-Padding
- **`volatile`** für ISR-geteilte Daten
- **`const`** für unveränderliche Configuration

### 8.4 Bitfield-Kürzel

Für Bitfield-Unions folgende Abkürzungen für die Union-Member verwenden:

| Kürzel | Typ | Bits |
|--------|-----|------|
| `b` | bit (struct) | variabel |
| `c` | char | 8 |
| `s` | short | 16 |
| `l` | long | 32 |
| `f` | float | 32 |
| `d` | double | 64 |
| `ll` | long long | 64 |

```c
// Register mit Bitfield-Zugriff
typedef union
{
    struct
    {
        uint16_t enable   : 1;   /**< Bit 0: Enable flag */
        uint16_t mode     : 2;   /**< Bit 1-2: Operating mode */
        uint16_t reserved : 13;  /**< Bit 3-15: Reserved */
    } b;                         /**< Bitfield access */
    uint16_t s;                  /**< 16-bit access */
} ControlReg;

// 32-Bit Register
typedef union
{
    struct
    {
        uint32_t counter : 24;   /**< Bit 0-23: Counter value */
        uint32_t status  : 8;    /**< Bit 24-31: Status flags */
    } b;                         /**< Bitfield access */
    uint32_t l;                  /**< 32-bit access */
} TimerReg;

// Usage
ControlReg ctrl;
ctrl.s = 0;              // Ganzes Register löschen
ctrl.b.enable = 1;       // Einzelnes Bit setzen
ctrl.b.mode = 2;         // Bitfeld setzen
```

### 8.5 Union-Grundlagen

Unions erlauben mehrere Interpretationen desselben Speicherbereichs.

| Anwendung | Description |
|-----------|--------------|
| Register-Zugriff | Bitfeld + Ganzwort-Zugriff (siehe §8.4) |
| Type-Punning | Vorsicht: Kann UB sein, `memcpy` bevorzugen |
| Variante Daten | Verschiedene Typen im selben Speicher |
| Protokoll-Parsing | Header + Body Overlays |

```c
// ✅ Gut: Union für Register-Zugriff
typedef union
{
    struct
    {
        uint16_t enable : 1;
        uint16_t mode   : 3;
        uint16_t rsvd   : 12;
    } b;
    uint16_t s;
} ConfigReg;

// ✅ Gut: Union für variante Daten mit Tag
typedef struct
{
    uint8_t type;           // Tag: welcher Typ aktiv ist
    union
    {
        int32_t  intValue;
        float    floatValue;
        uint8_t  bytes[4];
    } data;
} Variant;

// ⚠️ Vorsicht: Type-Punning via Union
// Technisch UB in C (außer bei char), aber oft toleriert
union FloatBits
{
    float f;
    uint32_t bits;
};
// Sicherer: memcpy verwenden
```

**Best Practices:**
- Immer mit Tag-Feld verwenden wenn Typ variabel
- Für Register: Bitfield + Ganzwort-Member
- Explizit dokumentieren welcher Member aktiv ist
- Größenvalidierung mit `static_assert` oder `_Static_assert`

---

## 9. Typen und Daten

### 9.1 Fixed-Width Types

Verwende `<stdint.h>` wo Größe wichtig ist:

| Typ | Usage |
|-----|------------|
| `uint8_t`, `int8_t` | Byte-Daten |
| `uint16_t`, `int16_t` | 16-Bit-Werte |
| `uint32_t`, `int32_t` | 32-Bit-Werte |
| `size_t` | Größen und Indizes |
| `bool` (C99) | Boolesche Werte |

### 9.2 Signed/Unsigned

- **Keine Mischung** ohne explizite Behandlung
- Truncation und Sign-Extension bewusst handhaben

---

## 10. Pointer und Speicher

### 10.1 Pointer-Regeln

| Regel | Description |
|-------|--------------|
| Keine wilde Pointer-Arithmetik | Außer einfach, begrenzt, dokumentiert |
| Validierung | Pointer vor Dereference prüfen |
| `const`-Correctness | Dokumentiert Intent |

```c
// ✅ const-correct
void processData(const uint8_t* data, size_t length);

// ✅ Pointer to constant pointer
const char* const MESSAGE = "Hello";
```

### 10.2 Dynamische Allokation

| Regel | Embedded-Kontext |
|-------|------------------|
| **Zur Laufzeit vermeiden** | `malloc`/`free` nicht verwenden |
| Nur in Init-Phase | Falls unvermeidbar, dokumentieren |
| Fallback-Strategie | Dokumentieren was bei Fehlschlag passiert |

### 10.3 Ownership

- Jede dynamisch allokierte Ressource hat **einen klaren Owner**
- Ownership-Transfer **explizit** in Funktionsnamen/Dokumentation

---

## 11. Kontrollfluss

### 11.1 Strukturierter Code

Erlaubt:
- `if` / `else`
- `switch` / `case`
- `for` / `while` / `do-while`

### 11.2 goto

**Generell vermeiden.** Erlaubt nur für:
- Kontrolliertes Error-Handling mit Cleanup
- In einer einzigen Funktion

```c
int processFile(const char* path)
{
    FILE* file = NULL;
    int result = -1;
    
    file = fopen(path, "r");
    if (!file)
    {
        goto cleanup;
    }
    
    // ... processing ...
    
    result = 0;
    
cleanup:
    if (file)
    {
        fclose(file);
    }
    return result;
}
```

### 11.3 Functions

- **Eine klare Verantwortung** pro Funktion
- Nicht übermäßig lang (Richtwert: 50-100 Zeilen)

### 11.4 Multiple Conditions

Bei mehreren Bedingungen: **jede Bedingung in Klammern** für Klarheit und MISRA-Konformität.

```c
// ✅ Richtig: Jede Bedingung in Klammern
if ((value > MIN_VALUE) && (value < MAX_VALUE))
{
    // ...
}

if ((condition1) && (condition2) || (condition3))
{
    // ...
}

// ❌ Falsch: Keine Klammern
if (value > MIN_VALUE && value < MAX_VALUE)
{
    // ...
}
```

### 11.5 Division — Divisor prüfen

Division durch 0 kann zu undefiniertem Verhalten oder Prozessor-Absturz führen. **Divisor immer prüfen.**

```c
// ✅ Sicher: Divisor validieren
if (divisor == 0)
{
    divisor = 1;  // Default-Wert oder Error-Handling
}
result = value / divisor;

// Alternativ: Explizites Error-Handling
if (divisor == 0)
{
    return RESULT_ERROR_DIVISION_BY_ZERO;
}
result = value / divisor;
```

---

## 12. Errorbehandlung

### 12.1 Keine Exceptions

C verwendet **Return Codes** und **Out-Parameters**:

```c
typedef enum
{
    RESULT_OK = 0,
    RESULT_ERROR_INVALID_PARAM,
    RESULT_ERROR_TIMEOUT,
    RESULT_ERROR_HARDWARE
} Result;

Result sensor_read(uint16_t* outValue);
```

### 12.2 Return Values prüfen

**Jeder Return Value muss:**
- Geprüft werden, oder
- Explizit mit Kommentar ignoriert werden

```c
// ✅ Checked
Result result = sensor_read(&value);
if (result != RESULT_OK)
{
    handleError(result);
}

// ✅ Explicitly ignored
(void)printf("Debug: %d\n", value);  // Return value irrelevant
```

### 12.3 Error-Code-Design

- Enumeriert und dokumentiert
- Eindeutige Codes pro Modul
- Mapping zu Logging/Diagnostik

---

## 13. Concurrency und Interrupts

### 13.1 Shared Data

Daten zwischen Interrupt und Main-Context:

| Anforderung | Mechanismus |
|-------------|-------------|
| `volatile` | Für Register und ISR-Flags |
| Atomare Operationen | Für Multi-Byte-Werte |
| Critical Sections | Interrupt-Disable wo nötig |

### 13.2 Richtlinien

- **Critical Sections minimal halten**
- **Race Conditions by Design vermeiden**
- **Keine Trial-and-Error-Synchronisation**

```c
// ✅ Atomic access
static volatile uint32_t s_tickCounter;

void SysTick_Handler(void)
{
    s_tickCounter++;
}

uint32_t getTicks(void)
{
    uint32_t ticks;
    __disable_irq();
    ticks = s_tickCounter;
    __enable_irq();
    return ticks;
}
```

---

## 14. Hardware-Zugriff

### 14.1 Register-Handling

| Regel | Description |
|-------|--------------|
| `volatile` | Für alle Hardware-Register |
| Kapselung | In dedizierten Modulen/Treibern |
| Keine Magic Addresses | Benannte Konstanten verwenden |

### 14.2 Register-Definition

```c
// ✅ Structured access
typedef struct
{
    volatile uint32_t CR;      // Control Register
    volatile uint32_t SR;      // Status Register
    volatile uint32_t DR;      // Data Register
} UART_TypeDef;

#define UART1 ((UART_TypeDef*)0x40011000UL)
```

### 14.3 Dokumentation

- Endianness dokumentieren
- Alignment-Anforderungen dokumentieren
- Memory-Mapped I/O kennzeichnen

---

## 15. Dokumentation

### 15.1 TODO-Marker

Für unfertige Aufgaben, Ideen oder zu behebende Probleme: `TODO` im Kommentar verwenden.

```c
// TODO: Implement timeout handling
// TODO: Optimize buffer allocation
// TODO: Add error recovery for communication failure
```

IDEs können TODO-Marker automatisch erkennen und auflisten (z.B. MPLAB-X: Window → Action Items, VS Code: Todo Tree Extension).

### 15.2 Code-Kommentare

| Kommentar-Typ | Usage |
|---------------|------------|
| `//` | Kurze Inline-Kommentare, temporäre Notizen |
| `/* */` | Mehrzeilige Kommentare, Sektions-Header |
| `/** */` | Doxygen-Dokumentation |

### 15.3 Funktions-Dokumentation

Öffentliche Functions mit Doxygen dokumentieren:

```c
/**
 * @brief  Initialize the timer module
 * @param  config Pointer to configuration structure
 * @return RESULT_OK on success, error code otherwise
 * @note   Must be called before any other timer function
 */
Result timer_init(const TimerConfig* config);
```

| Tag | Usage |
|-----|------------|
| `@brief` | Kurzbeschreibung (Required) |
| `@param` | Parameters-Description |
| `@return` | Return Value |
| `@note` | Importante Notee |
| `@warning` | Warningen, Einschränkungen |
| `@see` | Verwandte Functions |

### 15.4 Sprache

| Kontext | Sprache |
|---------|---------|
| Öffentliche APIs | **Englisch** (Required) |
| Wiederverwendbare Module | **Englisch** (Required) |
| Projekt-interne Dateien | German erlaubt |
| Commit-Messages | Englisch empfohlen |

---

## 16. MISRA und CERT

### 16.1 MISRA C:2012 Alignment

| Kategorie | Umsetzung |
|-----------|-----------|
| Typen | Fixed-Width, keine impliziten Konvertierungen |
| Kontrollfluss | Strukturiert, kein unreachable Code |
| Pointer | Validierung, begrenzte Arithmetik |
| UB-Vermeidung | Keine Abhängigkeit von undefiniertem Verhalten |

### 16.2 SEI CERT C Alignment

| Kategorie | Umsetzung |
|-----------|-----------|
| Input-Validierung | Alle externen Inputs prüfen |
| Integer-Handling | Overflow/Truncation bewusst handhaben |
| Buffer-Handling | Bounds prüfen |
| Ressourcen | Keine Leaks, klares Ownership |

### 16.3 Compliance-Dokumentation

Projekte mit MISRA/CERT-Anspruch dokumentieren:
- Anwendbare Regeln
- Begründete Abweichungen

---

## 17. Tests und Statische Analyse

### 17.1 Tests

| Test-Typ | Description |
|----------|--------------|
| Unit Tests | Wo praktikabel (PC-hosted) |
| Integration Tests | Auf Ziel-Hardware oder Simulation |

### 17.2 Statische Analyse

Empfohlene Tools:
- `clang-tidy` (PC-Build)
- Vendor-spezifische Checker
- MISRA-Checker

Warningen mit Safety-Relevanz **müssen** behoben oder begründet werden.

---

## 18. Verhältnis zu C++ (PC)

### 18.1 Unterschiede

| Aspekt | C (Embedded) | C++ (PC) |
|--------|--------------|----------|
| Exceptions | No | Yes |
| Dynamic Allocation | Vermeiden | Erlaubt |
| Standard Library | Minimal | Voll |
| Abstraktion | Prozedural | OOP erlaubt |

### 18.2 Shared Components

Interfaces zwischen C und C++:

```c
// header.h
#ifdef __cplusplus
extern "C" {
#endif

void shared_function(int param);

#ifdef __cplusplus
}
#endif
```

---

## 19. Legacy-Code und Ausnahmen

### 19.1 Legacy-Code

- Temporär erlaubt wenn nicht compliant
- Neuer Code **immer** nach Standard
- Refactoring-Chancen nutzen

### 19.2 Intentionale Abweichungen

- Kommentar im Code
- Begründung dokumentieren
- Scope minimieren

---

## 20. See Also

- [Cpp_Coding_Standard.md](Cpp_Coding_Standard.md) — C++ (PC)
- [CMake_Standard.md](CMake_Standard.md) — Build-System
- [Git_Standard.md](Git_Standard.md) — Versionskontrolle
- MISRA C:2012 Guidelines
- SEI CERT C Coding Standard

---

## 21. Changelog

| Version | Datum | Changes |
|---------|-------|------------|
| **0.10.0** | **2025-12-19** | **Neu: Union-Grundlagen (8.5) mit Best Practices und Examplesn** |
| 0.9.0 | 2025-12-19 | Neu: Bitfield-Kürzel (8.4), Kapitel Dokumentation (15) mit TODO, Code-Kommentare, Funktions-Doku |
| 0.8.0 | 2025-12-19 | Neu: TODO-Marker (3.1), Multiple Conditions in Klammern (11.4), Division durch 0 prüfen (11.5) |
| 0.7.0 | 2025-12-19 | Vereinheitlichung: Dateinamen auf PascalCase (wie C++), Präfix-Tabelle vereinfacht |
| 0.6.0 | 2025-12-19 | Konsolidierung: Neuer Abschnitt 8 Struct-Layout, Dateinamen-Konventionen (5.1), Include-Reihenfolge (5.5), Register-Konventionen (7.4), erweiterte Präfix-Tabelle (7.3) |
| 0.5.1 | 2025-12-18 | Neuer Abschnitt 4: Datei-Header mit Doxygen-Format, Requiredfelder, Sprachregelung |
| 0.5.0 | 2025-12-13 | Migration auf Blueprint v0.5: Neuer Header, Table of Contents, Encoding-Fix |
| 0.1.0 | 2025-12-05 | Initial: Embedded-Fokus, MISRA/CERT-Alignment, Interrupt-Handling |

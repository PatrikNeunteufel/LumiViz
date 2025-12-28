# C Coding Standard — Firmenrichtlinie

> **Version:** 1.0.0  
> **Datum:** 2025-12-19  
> **Typ:** Standard  
> **Status:** Stabil  
> **Zielgruppe:** Embedded-Entwickler, Firmware-Entwickler  
> **Geltungsbereich:** Alle C-Projekte (Embedded, Firmware, TMS320, dsPIC)  
> **Sprache:** Deutsch

---

## Inhaltsverzeichnis

1. [Zweck und Geltungsbereich](#1-zweck-und-geltungsbereich)
2. [Sprachversion und Compiler](#2-sprachversion-und-compiler)
3. [Grundprinzipien](#3-grundprinzipien)
4. [Projektstruktur](#4-projektstruktur)
5. [Datei-Header (optional)](#5-datei-header-optional)
6. [Datei- und Modul-Organisation](#6-datei--und-modul-organisation)
7. [Formatierung](#7-formatierung)
8. [Namenskonventionen](#8-namenskonventionen)
9. [Header-Dateien](#9-header-dateien)
10. [Source-Dateien](#10-source-dateien)
11. [Struct-Layout und Bitfields](#11-struct-layout-und-bitfields)
12. [Typen und Daten](#12-typen-und-daten)
13. [Pointer und Speicher](#13-pointer-und-speicher)
14. [Kontrollfluss](#14-kontrollfluss)
15. [Fehlerbehandlung](#15-fehlerbehandlung)
16. [Concurrency und Interrupts](#16-concurrency-und-interrupts)
17. [Hardware-Zugriff](#17-hardware-zugriff)
18. [Dokumentation und TODOs](#18-dokumentation-und-todos)
19. [MISRA und CERT (Informativ)](#19-misra-und-cert-informativ)
20. [Legacy-Code und Ausnahmen](#20-legacy-code-und-ausnahmen)
21. [Siehe auch](#21-siehe-auch)
22. [Changelog](#22-changelog)

---

## 1. Zweck und Geltungsbereich

Dieser Standard definiert **Coding-Konventionen für C** im Unternehmen, mit Fokus auf **Embedded-Systeme und Firmware**.

### 1.1 Ziel

Einheitlicher Code über verschiedene Mitarbeiter und Projekte hinweg. Jeder, der Code schreibt, soll diesen Standard anwenden.

### 1.2 Anwendungsbereich

| Plattform | Beispiele |
|-----------|-----------|
| TI C2000 | TMS320F28P65x (Master, CLA, Slave) |
| Microchip | dsPIC33, PIC18F |
| Weitere | Nach Bedarf |

---

## 2. Sprachversion und Compiler

### 2.1 Standard-Version

| Projekt-Typ | C Standard |
|-------------|------------|
| Neue Embedded-Projekte | **C99** (Minimum) |
| Mit Toolchain-Support | C11 erlaubt |

### 2.2 Compiler-Erweiterungen

Erlaubt **nur wenn**:
- Notwendig für Hardware-Zugriff, Performance oder Memory-Layout
- Klar dokumentiert an Abstraktionsgrenze

### 2.3 Undefined Behavior

> Code darf **niemals** auf Undefined Behavior oder unspezifiziertem Verhalten basieren.

---

## 3. Grundprinzipien

1. **Determinismus und Vorhersagbarkeit** über maximale Performance
2. **Sicherheit und Robustheit** über clevere Konstrukte
3. **Einfachheit und Klarheit** über Over-Engineering
4. **Einheitlichkeit** — Code soll aussehen, als wäre er von einer Person geschrieben

---

## 4. Projektstruktur

### 4.1 Ordnerstruktur (Dual-Core TMS320)

```
Project/
├── General/              /* Code für alle Subprojekte        */
│   ├── Header_Files/
│   └── Source_Files/
├── Master_CLA/           /* Master Core und CLA              */
│   ├── Master/
│   │   ├── Header_Files/
│   │   └── Source_Files/
│   └── CLA/
│       ├── Header_Files/
│       └── Source_Files/
└── Slave/                /* Slave Core                       */
    ├── Header_Files/
    └── Source_Files/
```

### 4.2 Beschreibung

| Ordner | Inhalt |
|--------|--------|
| `General` | Allgemeiner Code, verwendbar von allen Subprojekten |
| `Master_CLA/Master` | Code nur für Master Core |
| `Master_CLA/CLA` | Code nur für CLA Core (programmiert via Master) |
| `Slave` | Code nur für Slave Core |

---

## 5. Datei-Header (optional)

Ein Doxygen-kompatibler Header kann verwendet werden:

```c
/**
 ****************************************************************************************
 * @file   Dateiname.h
 * @brief  Kurzbeschreibung
 *         Optionale zweite Zeile
 *
 * @author Autor Name
 * @date   Monat YYYY
 ****************************************************************************************
 */
```

| Feld | Beschreibung |
|------|--------------|
| `@file` | Exakter Dateiname |
| `@brief` | Kurzbeschreibung (1-2 Zeilen) |
| `@author` | Hauptautor |
| `@date` | Erstellungsdatum |

---

## 6. Datei- und Modul-Organisation

### 6.1 Dateiendungen

| Endung | Verwendung |
|--------|------------|
| `.h` | C Header |
| `.c` | C Implementierung |

### 6.2 Include-Reihenfolge

```c
/* 1. Zugehöriger Header */
#include "Timer.h"

/* 2. Standard-Header */
#include <stdint.h>
#include <stdbool.h>

/* 3. Plattform/Vendor-Header */
#include "device.h"
#include "F28x_Project.h"

/* 4. Projekt-Header */
#include "Hal/Gpio.h"
#include "Config/BoardConfig.h"
```

### 6.3 Wichtig

- Header-Dateien sollen nur die notwendigen `*.h` inkludieren
- **Niemals `*.c` Dateien inkludieren**
- Idealerweise inkludiert ein Header keine anderen Header

---

## 7. Formatierung

### 7.1 Übersicht

| Aspekt | Regel |
|--------|-------|
| Einrückung | 3 Spaces |
| Tabs | Nie verwenden |
| Klammern | **Allman-Stil** (neue Zeile) |
| Zeilenlänge | **110 Zeichen** |
| Kommentare | **Immer `/* */`** (kein `//`) |

### 7.2 Klammern-Stil (Allman)

```c
void FUNCTION_NAME(void)
{
   /* ... */
}

if (Condition)
{
   /* ... */
}
else
{
   /* ... */
}
```

### 7.3 Ausrichtung von Gleichheitszeichen

Wenn möglich, alle Gleichheitszeichen auf gleicher Position:

```c
Counter        = 0;
BufferSize     = 1024;
IsInitialized  = false;
```

### 7.4 Schliessende Klammer mit Kommentar

Bei Funktionen, switch-cases, if-else, while und for: Kommentar was beendet wird.

```c
void INITIALIZE_HARDWARE(void)
{
   /* ... */
} /* end of INITIALIZE_HARDWARE */

switch (State)
{
   case STATE_IDLE:
      /* ... */
      break; /* end of case STATE_IDLE */
      
   default:
      break; /* end of default */
} /* end of switch(State) */
```

---

## 8. Namenskonventionen

### 8.1 Übersicht

| Entität | Konvention | Beispiel |
|---------|------------|----------|
| **Konstanten** | UPPER_CASE | `PI`, `MAX_VOLTAGE` |
| **Makros** | UPPER_CASE | `ADC_TIMEOUT`, `BUFFER_SIZE` |
| **Pointer** | `ptr_` Präfix | `ptr_Data`, `ptr_Buffer` |
| **Typen** | `t_` Präfix | `t_Timestamp`, `t_EPWM_REGS` |
| **Funktionen** | UPPER_CASE | `INITIALIZE_HARDWARE()` |
| **Variablen** | CapitalStart | `Counter`, `BufferSize` |
| **Structs/Unions** | UPPER_CASE | `MY_REGISTER`, `DATA_PACKET` |
| **Globale Variable** | `g_` Präfix | `g_SystemState` |
| **Statische Variable** | `s_` Präfix | `s_Counter` |

### 8.2 Beispiele

```c
/* Konstanten */
#define MAX_BUFFER_SIZE   1024
#define PI                3.14159f

/* Pointer */
uint8_t* ptr_Data;
t_Config* ptr_Config;

/* Typen */
typedef uint32_t t_Timestamp;
typedef struct { /* ... */ } t_EPWM_REGS;

/* Funktionen */
void INITIALIZE_HARDWARE(void);
void PROCESS_DATA(uint8_t* ptr_Buffer, uint16_t Length);

/* Variablen */
uint16_t Counter;
uint32_t BufferSize;
bool     IsActive;
```

---

## 9. Header-Dateien

### 9.1 Header Guard

Alle Header-Dateien werden mit einem Header Guard geschützt:

```c
#ifndef __MODULE_NAME_H__
#define __MODULE_NAME_H__

/* Hier der eigentliche Code */

#endif /* __MODULE_NAME_H__ */
```

> **Hinweis:** Namen mit doppeltem Unterstrich (`__`) sind gemäss C-Standard reserviert für Compiler/Implementation. Diese Konvention wird aus historischen Gründen beibehalten — bei neuen Projekten kann alternativ `_MODULE_NAME_H_` verwendet werden.

### 9.2 Struktur einer Header-Datei

```c
#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

/* ═══════════════════════════════════════════════════════════════════════════ */
/* Includes                                                                    */
/* ═══════════════════════════════════════════════════════════════════════════ */
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════════════════ */
/* Constant Definitions                                                        */
/* ═══════════════════════════════════════════════════════════════════════════ */

/* Timer Configuration
   ------------------- */
#define TIMER_PERIOD      1000     /* Timer period in us                      */
#define TIMER_PRESCALER   16       /* Prescaler value                         */

/* ═══════════════════════════════════════════════════════════════════════════ */
/* Structures and Bit Fields                                                   */
/* ═══════════════════════════════════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════════════════════════════════ */
/* Variable Declaration                                                        */
/* ═══════════════════════════════════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════════════════════════════════ */
/* Function Signature                                                          */
/* ═══════════════════════════════════════════════════════════════════════════ */

#endif /* __EXAMPLE_H__ */
```

### 9.3 Constant Definitions

Konstanten in passenden Sektionen gruppieren. Namen in UPPER_CASE. Werte und Kommentare ausgerichtet.

```c
/* ADC Configuration
   ----------------- */
#define ADC_RESOLUTION    12       /* ADC resolution in bits                  */
#define ADC_VREF          3.3f     /* Reference voltage                       */
#define ADC_MAX_VALUE     4095     /* Maximum ADC value (2^12 - 1)            */
```

### 9.4 Variable Declaration

Variablen beginnen mit Grossbuchstaben. Ausrichtung beachten.

```c
/* Timer Variables
   --------------- */
extern uint32_t   TimerCounter;    /* Current timer count                     */
extern bool       TimerExpired;    /* Timer expired flag                      */
extern t_Config   CurrentConfig;   /* Active configuration                    */
```

### 9.5 Function Signature

Funktionsnamen in UPPER_CASE. Alle Prototypen auflisten.

```c
/* Initialization
   -------------- */
void INITIALIZE_TIMER(void);
void INITIALIZE_ADC(void);

/* Runtime Functions
   ----------------- */
void PROCESS_ADC_DATA(uint16_t* ptr_Buffer, uint16_t Length);
bool CHECK_TIMEOUT(uint32_t Timeout);
```

---

## 10. Source-Dateien

### 10.1 Funktionsbeschreibung

Vor jeder Funktion eine Asterisk-Linie, dann Titel mit Unterstreichung (=), dann Beschreibung.

```c
/******************************************************************************
 * INITIALIZE_HARDWARE
 * ====================
 * Initialize all hardware peripherals.
 * Must be called once at startup before any other function.
 ******************************************************************************/
void INITIALIZE_HARDWARE(void)
{
   /* Variablen */
   uint16_t   Counter;
   bool       Success;
   
   /* Initialization
      -------------- */
   Counter = 0;
   Success = false;
   
   /* Configure Clocks
      ---------------- */
   /* Enable peripheral clocks */
   CLOCK_ENABLE_PERIPHERAL(CLOCK_ADC);
   CLOCK_ENABLE_PERIPHERAL(CLOCK_PWM);
   
} /* end of INITIALIZE_HARDWARE */
```

### 10.2 Funktions-Variablen

Alle deklarierten Variablen am Anfang der Funktion, ausgerichtet.

```c
void PROCESS_DATA(void)
{
   uint16_t   Index;
   uint32_t   Sum;
   bool       IsValid;
   float      Average;
   
   /* ... */
} /* end of PROCESS_DATA */
```

### 10.3 Funktions-Abschnitte

Abschnitte mit Titel und Unterstreichung (Bindestrich) markieren.

```c
void MAIN_LOOP(void)
{
   /* Initialization
      -------------- */
   Counter = 0;
   
   /* Main Processing
      --------------- */
   while (Running)
   {
      /* Read Inputs
         ----------- */
      READ_ADC_VALUES();
      
      /* Process Data
         ------------ */
      CALCULATE_AVERAGE();
      
      /* Update Outputs
         -------------- */
      UPDATE_PWM_OUTPUTS();
      
   } /* end of while(Running) */
   
} /* end of MAIN_LOOP */
```

### 10.4 Switch-Case

Titel und Unterstreichung vor switch. Break mit Kommentar.

```c
/* State Machine
   ------------- */
switch (CurrentState)
{
   case STATE_IDLE:
      /* Wait for start command */
      if (StartCommand)
      {
         CurrentState = STATE_RUNNING;
      } /* end of if(StartCommand) */
      break; /* end of case STATE_IDLE */
      
   case STATE_RUNNING:
      /* Execute main process */
      PROCESS_DATA();
      break; /* end of case STATE_RUNNING */
      
   case STATE_ERROR:
      /* Handle error condition */
      HANDLE_ERROR();
      break; /* end of case STATE_ERROR */
      
   default:
      /* Unknown state - reset */
      CurrentState = STATE_IDLE;
      break; /* end of default */
      
} /* end of switch(CurrentState) */
```

### 10.5 If mit mehreren Bedingungen

Bei mehreren Bedingungen: jede Bedingung in Klammern.

```c
if ((Condition1) && (Condition2))
{
   /* ... */
} /* end of if */

if ((Value > MIN_VALUE) && (Value < MAX_VALUE) || (OverrideActive))
{
   /* ... */
} /* end of if */
```

### 10.6 Keine Magic Numbers

Keine Zahlen direkt im Code. Stattdessen Konstanten im Header definieren.

```c
/* Header: */
#define TIMEOUT_MS   500

/* Source: */
/* Falsch: */
if (Counter > 500) { /* ... */ }

/* Richtig: */
if (Counter > TIMEOUT_MS) { /* ... */ }
```

### 10.7 Division — Divisor prüfen

Division durch 0 kann den Prozessor zum Absturz bringen. Immer Divisor prüfen.

```c
/* Division sicher ausführen */
if (Divisor == 0)
{
   Divisor = 1;  /* oder anderen Default-Wert setzen */
} /* end of if(Divisor == 0) */

Result = Value / Divisor;
```

---

## 11. Struct-Layout und Bitfields

### 11.1 Struct-Layout

Structs in logische Abschnitte gliedern. 3 Spaces für Alignment.

```c
/* Timer Configuration Structure
   ----------------------------- */
/* Holds all configuration parameters for a timer instance. */
typedef struct
{
   /* Configuration Parameters
      ------------------------ */
   uint32_t   Period;           /* Timer period in ticks                     */
   uint16_t   Prescaler;        /* Clock prescaler value                     */
   uint8_t    Mode;             /* Operating mode                            */
   
   /* State Variables
      --------------- */
   volatile uint32_t   Counter; /* Current counter value                     */
   volatile bool       Expired; /* Timer expired flag                        */
   
   /* Callbacks
      --------- */
   void (*ptr_Callback)(void);  /* Callback function pointer                 */
   
} t_TimerConfig;
```

### 11.2 Alignment und Padding

Grössere Typen zuerst für minimales Padding.

```c
/* Richtig: Grössere Typen zuerst */
typedef struct
{
   uint32_t   Timestamp;        /* 4 bytes                                   */
   uint16_t   Id;               /* 2 bytes                                   */
   uint8_t    Flags;            /* 1 byte                                    */
   uint8_t    Reserved;         /* 1 byte (explizites Padding)               */
} t_Message;  /* Total: 8 bytes, kein verstecktes Padding */

/* Falsch: Ineffizientes Layout */
typedef struct
{
   uint8_t    Flags;            /* 1 byte + 3 padding                        */
   uint32_t   Timestamp;        /* 4 bytes                                   */
   uint8_t    Id;               /* 1 byte + 3 padding                        */
} t_BadMessage;  /* Total: 12 bytes mit verstecktem Padding */
```

### 11.3 Bitfields und Unions

Abkürzungen für Bitfield-Unions:

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
/* Register Definition
   ------------------- */
/* Control register with bitfield access. */
typedef union
{
   struct
   {
      uint16_t   Enable   : 1;  /* Bit 0: Enable flag                        */
      uint16_t   Mode     : 2;  /* Bit 1-2: Operating mode                   */
      uint16_t   Reserved : 13; /* Bit 3-15: Reserved                        */
   } b;                         /* Bitfield access                           */
   uint16_t s;                  /* 16-bit access                             */
} t_CONTROL_REG;
```

### 11.4 Union-Grundlagen

Unions erlauben mehrere Interpretationen desselben Speicherbereichs.

| Anwendung | Beschreibung |
|-----------|--------------|
| Register-Zugriff | Bitfeld + Ganzwort-Zugriff (siehe §11.3) |
| Type-Punning | Vorsicht: Kann UB sein |
| Variante Daten | Verschiedene Typen im selben Speicher |
| Protokoll-Parsing | Header + Body Overlays |

```c
/* Union für variante Daten mit Tag
   -------------------------------- */
typedef struct
{
   uint8_t Type;                 /* Tag: welcher Typ aktiv ist               */
   union
   {
      int32_t   IntValue;        /* Ganzzahl                                 */
      float     FloatValue;      /* Fliesskomma                              */
      uint8_t   Bytes[4];        /* Byte-Array                               */
   } Data;
} t_Variant;

/* Verwendung */
t_Variant Var;
Var.Type = 1;                    /* Integer aktiv                            */
Var.Data.IntValue = 42;
```

**Best Practices:**
- Immer mit Tag-Feld verwenden wenn Typ variabel
- Für Register: Bitfield + Ganzwort-Member (siehe §11.3)
- Explizit dokumentieren welcher Member aktiv ist

---

## 12. Typen und Daten

### 12.1 Fixed-Width Types

Verwende `<stdint.h>` wo Grösse wichtig ist:

| Typ | Verwendung |
|-----|------------|
| `uint8_t`, `int8_t` | Byte-Daten |
| `uint16_t`, `int16_t` | 16-Bit-Werte |
| `uint32_t`, `int32_t` | 32-Bit-Werte |
| `size_t` | Grössen und Indizes |
| `bool` (C99) | Boolesche Werte |

### 12.2 Signed/Unsigned

- **Keine Mischung** ohne explizite Behandlung
- Truncation und Sign-Extension bewusst handhaben

---

## 13. Pointer und Speicher

### 13.1 Pointer-Regeln

| Regel | Beschreibung |
|-------|--------------|
| `ptr_` Präfix | Alle Pointer mit `ptr_` kennzeichnen |
| Validierung | Pointer vor Dereference prüfen |
| `const`-Correctness | Dokumentiert Intent |

```c
/* const-correct */
void PROCESS_DATA(const uint8_t* ptr_Data, size_t Length);

/* Pointer validieren */
if (ptr_Buffer != NULL)
{
   /* Sicher zu verwenden */
   Value = *ptr_Buffer;
} /* end of if(ptr_Buffer != NULL) */
```

### 13.2 Dynamische Allokation

| Regel | Embedded-Kontext |
|-------|------------------|
| **Zur Laufzeit vermeiden** | `malloc`/`free` nicht verwenden |
| Nur in Init-Phase | Falls unvermeidbar, dokumentieren |

---

## 14. Kontrollfluss

### 14.1 Strukturierter Code

Erlaubt:
- `if` / `else`
- `switch` / `case`
- `for` / `while` / `do-while`

### 14.2 goto

**Generell vermeiden.** Erlaubt nur für kontrolliertes Error-Handling mit Cleanup.

### 14.3 Funktionen

- **Eine klare Verantwortung** pro Funktion
- Nicht übermässig lang (Richtwert: 50-100 Zeilen)

---

## 15. Fehlerbehandlung

### 15.1 Return Codes

C verwendet **Return Codes** und **Out-Parameter**:

```c
typedef enum
{
   RESULT_OK = 0,
   RESULT_ERROR_INVALID_PARAM,
   RESULT_ERROR_TIMEOUT,
   RESULT_ERROR_HARDWARE
} t_Result;

t_Result SENSOR_READ(uint16_t* ptr_OutValue);
```

### 15.2 Rückgabewerte prüfen

**Jeder Rückgabewert muss:**
- Geprüft werden, oder
- Explizit mit Kommentar ignoriert werden

```c
/* Geprüft */
Result = SENSOR_READ(&Value);
if (Result != RESULT_OK)
{
   HANDLE_ERROR(Result);
} /* end of if(Result != RESULT_OK) */

/* Explizit ignoriert */
(void)PRINTF("Debug: %d\n", Value);  /* Return value irrelevant */
```

---

## 16. Concurrency und Interrupts

### 16.1 Shared Data

Daten zwischen Interrupt und Main-Context:

| Anforderung | Mechanismus |
|-------------|-------------|
| `volatile` | Für Register und ISR-Flags |
| Atomare Operationen | Für Multi-Byte-Werte |
| Critical Sections | Interrupt-Disable wo nötig |

### 16.2 Richtlinien

- **Critical Sections minimal halten**
- **Race Conditions by Design vermeiden**

```c
/* Atomic access */
static volatile uint32_t s_TickCounter;

void SYSTICK_HANDLER(void)
{
   s_TickCounter++;
} /* end of SYSTICK_HANDLER */

uint32_t GET_TICKS(void)
{
   uint32_t Ticks;
   
   DISABLE_INTERRUPTS();
   Ticks = s_TickCounter;
   ENABLE_INTERRUPTS();
   
   return Ticks;
} /* end of GET_TICKS */
```

---

## 17. Hardware-Zugriff

### 17.1 Register-Handling

| Regel | Beschreibung |
|-------|--------------|
| `volatile` | Für alle Hardware-Register |
| Kapselung | In dedizierten Modulen/Treibern |
| Keine Magic Addresses | Benannte Konstanten verwenden |

### 17.2 Register-Definition

```c
/* Register-Struktur */
typedef struct
{
   volatile uint32_t   CR;      /* Control Register                          */
   volatile uint32_t   SR;      /* Status Register                           */
   volatile uint32_t   DR;      /* Data Register                             */
} t_UART_REGS;

#define UART1_BASE   0x40011000UL
#define UART1        ((t_UART_REGS*)UART1_BASE)
```

---

## 18. Dokumentation und TODOs

### 18.1 TODO-Marker

Für unfertige Aufgaben, Ideen oder zu behebende Probleme: `TODO` verwenden.

```c
/* TODO: Implement timeout handling */
/* TODO: Optimize buffer allocation */
/* TODO: Add error recovery for communication failure */
```

### 18.2 Dokumentations-Ablage

| Ordner | Beschreibung |
|--------|--------------|
| `_Documentation/General_Functions` | Code als Standard für verschiedene Plattformen |
| `_Documentation/Platforms/<Name>` | Allgemeine Beschreibung einer Plattform |
| `_Documentation/Platforms/<Name>/Variants` | Übersicht für Varianten-spezifische Teile |
| `_Documentation/Platforms/<Name>/Functions` | Beschreibung jeder Funktion oder jedes Moduls |

---

## 19. MISRA und CERT (Informativ)

Dieser Standard orientiert sich an bewährten Praktiken aus:

- **MISRA C:2012** — Sicherheit, Robustheit, UB-Vermeidung
- **SEI CERT C** — Sichere, defensive C-Programmierung

Eine vollständige MISRA/CERT-Compliance ist aktuell nicht gefordert. Die Prinzipien dienen als Orientierung für robuste, sichere Embedded-Software.

---

## 20. Legacy-Code und Ausnahmen

### 20.1 Legacy-Code

- Temporär erlaubt wenn nicht compliant
- Neuer Code **immer** nach Standard
- Refactoring-Chancen nutzen

### 20.2 Intentionale Abweichungen

- Kommentar im Code
- Begründung dokumentieren
- Scope minimieren

---

## 21. Siehe auch

- IDE-Settings: `N:\QMS\Software_Settings\MPLAB-X`
- Dokumentation: `N:\Projekte\<Projekt>\Firmware\_Documentation`

---

## 22. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.1.0** | **2025-12-19** | **Neu: Union-Grundlagen (11.4) mit Best Practices und Beispielen** |
| 1.0.0 | 2025-12-19 | Initial: Konsolidierung aus MPLAB-X V3 und internem Standard |

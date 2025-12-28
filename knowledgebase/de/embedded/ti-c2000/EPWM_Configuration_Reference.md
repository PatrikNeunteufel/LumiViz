# EPWM Configuration — Reference

> **Version:** 0.1.1  
> **Datum:** 2025-12-18  
> **Typ:** Reference  
> **Status:** In Entwicklung  
> **Zielgruppe:** Embedded-Entwickler (C2000, Power Electronics)  
> **Hardware:** TMS320F28P650 (ePWM Type 5)  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Konventionen](#2-konventionen)
3. [Counter-Modi](#3-counter-modi)
4. [PWM-Grundkonfigurationen](#4-pwm-grundkonfigurationen)
5. [Compare-Register](#5-compare-register)
6. [Action Qualifier](#6-action-qualifier)
7. [Phasenversatz und Synchronisation](#7-phasenversatz-und-synchronisation)
8. [Global Load und One-Shot](#8-global-load-und-one-shot)
9. [Fortgeschrittene Konfigurationen](#9-fortgeschrittene-konfigurationen)
10. [Schnellreferenz](#10-schnellreferenz)
11. [Troubleshooting](#11-troubleshooting)
12. [Siehe auch](#12-siehe-auch)
13. [Anhang A: High-Resolution PWM (HRPWM)](#anhang-a-high-resolution-pwm-hrpwm)
14. [Changelog](#13-changelog)

---

## 1. Übersicht

Diese Referenz dokumentiert die Konfiguration des Enhanced Pulse Width Modulator (ePWM) Peripherals auf TI C2000 Mikrocontrollern, speziell für den TMS320F28P650 mit ePWM Type 4.

### Zielgruppe

- Embedded-Entwickler mit Fokus auf Leistungselektronik
- Motor Control, DC/DC-Wandler, Inverter-Anwendungen

### Kernkonzept: Shadow-Register

Das ePWM-Modul verwendet **Shadow-Register** für alle kritischen Parameter. Dieses Konzept ist identisch mit Double-Buffering bei Grafikkarten:

```
┌─────────────────┐     Load Event      ┌─────────────────┐
│ Shadow Register │ ──────────────────► │ Active Register │
│  (CPU schreibt) │   (ZRO/PRD/SYNC)    │ (Hardware liest)│
└─────────────────┘                     └─────────────────┘
```

**Vorteile:**
- Keine Glitches durch asynchrone Schreibzugriffe
- Atomare Updates mehrerer Register möglich
- Definierte Übergabezeitpunkte

---

## 2. Konventionen

### 2.1 Notation

| Symbol | Bedeutung |
|--------|-----------|
| `ZRO` | Counter = Zero Event |
| `PRD` | Counter = Period Event |
| `CAU` | Compare A Up (Counter aufwärts bei CMPA) |
| `CAD` | Compare A Down (Counter abwärts bei CMPA) |
| `CBU` | Compare B Up |
| `CBD` | Compare B Down |

### 2.2 Action Qualifier Werte

| Wert | Aktion | Beschreibung |
|------|--------|--------------|
| `0` | Disabled | Keine Aktion |
| `1` | Clear | Output auf LOW setzen |
| `2` | Set | Output auf HIGH setzen |
| `3` | Toggle | Output invertieren |

### 2.3 ASCII-Diagramm Legende

```
~~~~  = HIGH-Pegel
____  = LOW-Pegel
│     = Event-Zeitpunkt
──►   = Zählrichtung
```

---

## 3. Counter-Modi

### 3.1 Up-Count Mode (`ctrmode = 0`)

```
Counter:   0 ────────────────────────────────────────────────────────────────> TBPRD ─┐
           │                                                                          │
           └──────────────────────────────────────────────────────────────────────────┘
                                         Reset auf 0
```

| Aspekt | Wert |
|--------|------|
| **Register** | `TBCTL.bit.CTRMODE = 0` |
| **PWM-Frequenz** | `f_PWM = f_TBCLK / (TBPRD + 1)` |
| **Verfügbare Events** | ZRO, CAU, CBU, PRD |
| **Anwendung** | Edge-Aligned PWM, einfache DC/DC |

### 3.2 Down-Count Mode (`ctrmode = 1`)

```
Counter:   TBPRD ────────────────────────────────────────────────────────────> 0 ─────┐
           │                                                                          │
           └──────────────────────────────────────────────────────────────────────────┘
                                      Reset auf TBPRD
```

| Aspekt | Wert |
|--------|------|
| **Register** | `TBCTL.bit.CTRMODE = 1` |
| **PWM-Frequenz** | `f_PWM = f_TBCLK / (TBPRD + 1)` |
| **Verfügbare Events** | PRD, CAD, CBD, ZRO |
| **Anwendung** | Spezielle Sync-Szenarien, 180° Phasenversatz |

### 3.3 Up-Down-Count Mode (`ctrmode = 2`)

```
Counter:   0 ────────────────────────> TBPRD ────────────────────────> 0
           │          aufwärts           │          abwärts           │
           └─────────────────────────────┴────────────────────────────┘
                              Richtungswechsel
```

| Aspekt | Wert |
|--------|------|
| **Register** | `TBCTL.bit.CTRMODE = 2` |
| **PWM-Frequenz** | `f_PWM = f_TBCLK / (2 × TBPRD)` |
| **Verfügbare Events** | ZRO, CAU, CAD, CBU, CBD, PRD |
| **Anwendung** | Center-Aligned PWM, Motorsteuerung |

### 3.4 Warum Up- und Down-Count existieren

Die Hardware für Up-Down-Count benötigt beide Zählrichtungen. Up-only und Down-only sind "Abfallprodukte" dieser Implementierung. Down-Count ist primär nützlich für **Phasenversatz ohne Phase-Register**:

```
EPWM1 (Up):    0 ─────────────────────> TBPRD
EPWM2 (Down):  TBPRD <───────────────── 0
               ════════════════════════════════> Zeit
               Automatisch 180° versetzt bei gleichzeitigem Start
```

---

## 4. PWM-Grundkonfigurationen

### 4.1 Center-Aligned PWM (Symmetrisch)

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* CENTER-ALIGNED PWM (Up-Down Count Mode) with Complementary Outputs                                  */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/* Counter:   0 ─────────────────> CMPA ─────────────────> TBPRD ─────────────────> CMPA ─────────> 0  */
/*            │                    │                       │                        │               │  */
/* Channel A: _________LOW_________|~~~~~~~~~~~~~~~~~~~~~~~HIGH~~~~~~~~~~~~~~~~~~~~~|______LOW______|  */
/* Channel B: ~~~~~~~~~HIGH~~~~~~~~|_______________________LOW______________________|~~~~~~HIGH~~~~~|  */
/*                                                                                                     */
/* Actions:   A: Set@CAD, Clr@CAU                          B: Clr@CAD, Set@CAU                         */
/*                                                                                                     */
/* Characteristics:                                                                                    */
/*   - Symmetric waveform around TBPRD (center point)                                                  */
/*   - PWM frequency = TBCLK / (2 × TBPRD)                                                             */
/*   - Reduced harmonic content, ideal for motor control and inverters                                 */
/*   - A/B outputs are 180° phase-shifted (complementary)                                              */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
ptr_Regs->TBCTL.bit.CTRMODE        = 2;       /* Up-Down-Count mode for center-aligned PWM              */
ptr_Regs->TBCTL.bit.PHSEN          = 0;       /* Disable phase loading from TBPHS register              */
ptr_Regs->TBCTL.bit.CLKDIV         = 0;       /* TBCLK = EPWMCLK / 1 (no prescaling)                    */
ptr_Regs->TBCTL.bit.HSPCLKDIV      = 0;       /* High-speed prescaler = /1                              */
ptr_Regs->TBCTL.bit.FREE_SOFT      = 2;       /* Free-run: counter continues on emulation suspend       */

ptr_Regs->TBPRD                    = 10000;   /* Period: PWM_freq = TBCLK / (2 × 10000)                 */
ptr_Regs->TBCTR                    = 0;       /* Clear counter to start from zero                       */

ptr_Regs->CMPA.bit.CMPA            = 5000;    /* 50% duty cycle: CMPA = TBPRD / 2                       */

ptr_Regs->AQCTLA.bit.CAU           = 1;       /* Channel A: Clear (LOW) when counter hits CMPA going up */
ptr_Regs->AQCTLA.bit.CAD           = 2;       /* Channel A: Set (HIGH) when counter hits CMPA going dn  */

ptr_Regs->AQCTLB.bit.CAU           = 2;       /* Channel B: Set (HIGH) when counter hits CMPA going up  */
ptr_Regs->AQCTLB.bit.CAD           = 1;       /* Channel B: Clear (LOW) when counter hits CMPA going dn */
EDIS;
```

| Aspekt | Wert |
|--------|------|
| **Anwendung** | Motorsteuerung, 3-Phasen-Inverter, Active Rectifier |
| **Vorteile** | Geringerer Ripple, symmetrische Schaltflanken |
| **Duty Cycle Formel** | `Duty = CMPA / TBPRD × 100%` |

### 4.2 Edge-Aligned PWM — Rising Edge at Period Start

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* EDGE-ALIGNED PWM (Up-Count Mode) - Rising Edge at Period Start                                      */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/* Counter:   0 ───────────────────────────────────────> CMPA ───────────────────────────────> TBPRD   */
/*            │                                          │                                     │       */
/* Channel A: ~~~~~~~~~~~~~~~~~~~HIGH~~~~~~~~~~~~~~~~~~~~|______________LOW____________________|       */
/* Channel B: ____________________LOW____________________|~~~~~~~~~~~~~~HIGH~~~~~~~~~~~~~~~~~~~|       */
/*                                                                                                     */
/* Actions:   A: Set@ZRO, Clr@CAU                         B: Clr@ZRO, Set@CAU                          */
/*                                                                                                     */
/* Characteristics:                                                                                    */
/*   - Asymmetric waveform, rising edge synchronized to period start                                   */
/*   - PWM frequency = TBCLK / (TBPRD + 1)                                                             */
/*   - Simple edge-aligned PWM, suitable for LED dimming, simple DC/DC                                 */
/*   - A/B outputs are complementary                                                                   */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
ptr_Regs->TBCTL.bit.CTRMODE        = 0;       /* Up-Count mode for edge-aligned PWM                     */
ptr_Regs->TBCTL.bit.PHSEN          = 0;       /* Disable phase loading from TBPHS register              */
ptr_Regs->TBCTL.bit.CLKDIV         = 0;       /* TBCLK = EPWMCLK / 1 (no prescaling)                    */
ptr_Regs->TBCTL.bit.HSPCLKDIV      = 0;       /* High-speed prescaler = /1                              */
ptr_Regs->TBCTL.bit.FREE_SOFT      = 2;       /* Free-run: counter continues on emulation suspend       */

ptr_Regs->TBPRD                    = 10000;   /* Period: PWM_freq = TBCLK / (10000 + 1)                 */
ptr_Regs->TBCTR                    = 0;       /* Clear counter to start from zero                       */

ptr_Regs->CMPA.bit.CMPA            = 5000;    /* ~50% duty cycle                                        */

ptr_Regs->AQCTLA.bit.ZRO           = 2;       /* Channel A: Set (HIGH) when counter equals zero         */
ptr_Regs->AQCTLA.bit.CAU           = 1;       /* Channel A: Clear (LOW) when counter hits CMPA          */

ptr_Regs->AQCTLB.bit.ZRO           = 1;       /* Channel B: Clear (LOW) when counter equals zero        */
ptr_Regs->AQCTLB.bit.CAU           = 2;       /* Channel B: Set (HIGH) when counter hits CMPA           */
EDIS;
```

| Aspekt | Wert |
|--------|------|
| **Anwendung** | LED-Dimming, einfache Buck/Boost-Wandler |
| **Vorteile** | Einfache Konfiguration, vorhersagbares Timing |
| **Duty Cycle Formel** | `Duty = CMPA / (TBPRD + 1) × 100%` |

### 4.3 Edge-Aligned PWM — Falling Edge at Period Start

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* EDGE-ALIGNED PWM (Up-Count Mode) - Falling Edge at Period Start                                     */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/* Counter:   0 ───────────────────────────────────────> CMPA ───────────────────────────────> TBPRD   */
/*            │                                          │                                     │       */
/* Channel A: ____________________LOW____________________|~~~~~~~~~~~~~~~~~HIGH~~~~~~~~~~~~~~~~|       */
/* Channel B: ~~~~~~~~~~~~~~~~~~~~HIGH~~~~~~~~~~~~~~~~~~~|_________________LOW_________________|       */
/*                                                                                                     */
/* Actions:   A: Clr@ZRO, Set@CAU                         B: Set@ZRO, Clr@CAU                          */
/*                                                                                                     */
/* Characteristics:                                                                                    */
/*   - Asymmetric waveform, falling edge synchronized to period start                                  */
/*   - PWM frequency = TBCLK / (TBPRD + 1)                                                             */
/*   - Inverted edge-alignment compared to Rising Edge variant                                         */
/*   - A/B outputs are complementary                                                                   */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
ptr_Regs->TBCTL.bit.CTRMODE        = 0;       /* Up-Count mode for edge-aligned PWM                     */
ptr_Regs->TBCTL.bit.PHSEN          = 0;       /* Disable phase loading from TBPHS register              */
ptr_Regs->TBCTL.bit.CLKDIV         = 0;       /* TBCLK = EPWMCLK / 1 (no prescaling)                    */
ptr_Regs->TBCTL.bit.HSPCLKDIV      = 0;       /* High-speed prescaler = /1                              */
ptr_Regs->TBCTL.bit.FREE_SOFT      = 2;       /* Free-run: counter continues on emulation suspend       */

ptr_Regs->TBPRD                    = 10000;   /* Period: PWM_freq = TBCLK / (10000 + 1)                 */
ptr_Regs->TBCTR                    = 0;       /* Clear counter to start from zero                       */

ptr_Regs->CMPA.bit.CMPA            = 5000;    /* ~50% duty cycle                                        */

ptr_Regs->AQCTLA.bit.ZRO           = 1;       /* Channel A: Clear (LOW) when counter equals zero        */
ptr_Regs->AQCTLA.bit.CAU           = 2;       /* Channel A: Set (HIGH) when counter hits CMPA           */

ptr_Regs->AQCTLB.bit.ZRO           = 2;       /* Channel B: Set (HIGH) when counter equals zero         */
ptr_Regs->AQCTLB.bit.CAU           = 1;       /* Channel B: Clear (LOW) when counter hits CMPA          */
EDIS;
```

### 4.4 High-Frequency Clock Generator (Toggle Mode)

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* HIGH-FREQUENCY CLOCK GENERATOR (Toggle Mode)                                                        */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/* Counter:   0 ─────────> TBPRD ─────────> 0 ─────────> TBPRD ─────────> 0 ─────────> TBPRD ───────>  */
/*            │            │                │            │                │            │               */
/* Channel B: ~~~~~~~~~~~~~|________________|~~~~~~~~~~~~|________________|~~~~~~~~~~~~|_____________  */
/*                                                                                                     */
/* Actions:   B: Toggle@ZRO                                                                            */
/*                                                                                                     */
/* Characteristics:                                                                                    */
/*   - Generates high-frequency square wave                                                            */
/*   - Output frequency = TBCLK / (2 × (TBPRD + 1)) due to toggle                                      */
/*   - Useful for external clock generation, SPI clock, or timing reference                            */
/*   - Only one channel used in toggle mode                                                            */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
ptr_Regs->TBCTL.bit.CTRMODE        = 0;       /* Up-Count mode                                          */
ptr_Regs->TBCTL.bit.CLKDIV         = 1;       /* TBCLK prescaler = /2                                   */
ptr_Regs->TBCTL.bit.HSPCLKDIV      = 1;       /* High-speed prescaler = /2 → total /4                   */
ptr_Regs->TBPRD                    = 12;      /* Period = 12 → Output freq depends on TBCLK             */
ptr_Regs->AQCTLB.bit.ZRO           = 3;       /* Channel B: Toggle output on every zero event           */
EDIS;
```

| Aspekt | Wert |
|--------|------|
| **Anwendung** | Taktgenerierung, SPI-Clock, Timing-Referenz |
| **Frequenz Formel** | `f_out = f_TBCLK / (2 × (TBPRD + 1))` |
| **Hinweis** | Faktor 2 durch Toggle-Verhalten |

---

## 5. Compare-Register

### 5.1 Übersicht

| Register | Direkte AQ-Aktion | Sync-Out | Event Trigger | Anwendung |
|----------|-------------------|----------|---------------|-----------|
| **CMPA** | ✓ | ✓ | ✓ | Primärer Duty Cycle |
| **CMPB** | ✓ | ✓ | ✓ | Zweiter Duty Cycle / Pulsposition |
| **CMPC** | — | ✓ | ✓ | ADC-Trigger, Sync |
| **CMPD** | — | ✓ | ✓ | ADC-Trigger, Sync |

### 5.2 CMPA vs. CMPB — Unabhängige Verwendung

CMPA und CMPB sind **nicht** an Channel A/B gebunden. Jeder Action Qualifier kann auf beide reagieren:

```c
/* Beide Channels reagieren auf CMPA (komplementär) */
ptr_Regs->AQCTLA.bit.CAU = 1;    /* Clear bei CMPA up */
ptr_Regs->AQCTLB.bit.CAU = 2;    /* Set bei CMPA up   */

/* Channel A reagiert auf BEIDE Compare-Register */
ptr_Regs->AQCTLA.bit.CAU = 2;    /* Set bei CMPA up   */
ptr_Regs->AQCTLA.bit.CBU = 1;    /* Clear bei CMPB up */
```

### 5.3 Asymmetrischer Puls mit CMPA und CMPB

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* ASYMMETRIC PULSE - Independent Control of Rising and Falling Edge                                   */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/* Counter:   0 ──────────> CMPA ──────────────────────────> CMPB ──────────────────────────> TBPRD    */
/*            │             │                                │                                │        */
/* Channel A: ______LOW_____|~~~~~~~~~~~~~~~~HIGH~~~~~~~~~~~~|______________LOW_______________|        */
/*                          ↑                                ↑                                         */
/*                     Pulse Start                      Pulse End                                      */
/*                                                                                                     */
/* Actions:   A: Set@CAU, Clr@CBU                                                                      */
/*                                                                                                     */
/* Characteristics:                                                                                    */
/*   - Full control over pulse POSITION (CMPA) and WIDTH (CMPB - CMPA)                                 */
/*   - Useful for phase-shifted sampling, precise timing control                                       */
/*   - ADC can be triggered at any point within the PWM cycle                                          */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
ptr_Regs->TBCTL.bit.CTRMODE        = 0;       /* Up-Count mode                                          */
ptr_Regs->TBPRD                    = 10000;

ptr_Regs->CMPA.bit.CMPA            = 2000;    /* Pulse starts at count 2000 (20% into period)           */
ptr_Regs->CMPB.bit.CMPB            = 7000;    /* Pulse ends at count 7000 (70% into period)             */
                                              /* Pulse width = 7000 - 2000 = 5000 counts (50% duty)     */

ptr_Regs->AQCTLA.bit.ZRO           = 1;       /* Clear at Zero (ensure defined start state)             */
ptr_Regs->AQCTLA.bit.CAU           = 2;       /* Set at CMPA (pulse rising edge)                        */
ptr_Regs->AQCTLA.bit.CBU           = 1;       /* Clear at CMPB (pulse falling edge)                     */
EDIS;
```

| Parameter | Kontrolle |
|-----------|-----------|
| **CMPA** | Puls-Startposition |
| **CMPB** | Puls-Endposition |
| **CMPB - CMPA** | Pulsbreite |
| **CMPA / TBPRD** | Phasenverschiebung des Pulses |

---

## 6. Action Qualifier

### 6.1 Ereignis-Matrix

| Event | Up-Count | Down-Count | Up-Down-Count |
|-------|----------|------------|---------------|
| ZRO | ✓ | ✓ | ✓ |
| PRD | ✓ | ✓ | ✓ |
| CAU | ✓ | — | ✓ |
| CAD | — | ✓ | ✓ |
| CBU | ✓ | — | ✓ |
| CBD | — | ✓ | ✓ |

### 6.2 AQCTLA / AQCTLB Register-Struktur

```c
union AQCTLA_REG {
    struct {
        Uint16 ZRO:2;     /* Action when TBCTR = 0                */
        Uint16 PRD:2;     /* Action when TBCTR = TBPRD            */
        Uint16 CAU:2;     /* Action when TBCTR = CMPA (up count)  */
        Uint16 CAD:2;     /* Action when TBCTR = CMPA (down count)*/
        Uint16 CBU:2;     /* Action when TBCTR = CMPB (up count)  */
        Uint16 CBD:2;     /* Action when TBCTR = CMPB (down count)*/
        Uint16 rsvd:4;
    } bit;
    Uint16 all;
};
```

### 6.3 Typische Konfigurationsmuster

| Muster | ZRO | CAU | CAD | Resultat |
|--------|-----|-----|-----|----------|
| Rising Edge Aligned | Set(2) | Clear(1) | — | HIGH am Anfang |
| Falling Edge Aligned | Clear(1) | Set(2) | — | LOW am Anfang |
| Center-Aligned A | — | Clear(1) | Set(2) | HIGH in der Mitte |
| Center-Aligned B | — | Set(2) | Clear(1) | LOW in der Mitte |
| Toggle Clock | Toggle(3) | — | — | 50% Rechteck |

---

## 7. Phasenversatz und Synchronisation

### 7.1 Interleaved-Betrieb — Übersicht

| Phasen | Versatz | Formel | Anwendung |
|--------|---------|--------|-----------|
| 2 | 180° | TBPHS = TBPRD / 2 | Interleaved Buck/Boost |
| 3 | 120° | TBPHS = TBPRD / 3 | 3-Phasen Inverter, BLDC |
| 4 | 90° | TBPHS = TBPRD / 4 | 4-Phasen VRM |
| N | 360°/N | TBPHS = TBPRD / N | Allgemein |

### 7.2 2-Phasen Interleaved Buck (180° Versatz)

```
Stufe 1:  |▀▀▀▀▀▀▀▀|________|▀▀▀▀▀▀▀▀|________|
Stufe 2:  |________|▀▀▀▀▀▀▀▀|________|▀▀▀▀▀▀▀▀|
          ════════════════════════════════════════> t
Summe:    ≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈≈  (Ripple verdoppelt Frequenz, halbiert Amplitude)
```

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* 2-PHASE INTERLEAVED CONFIGURATION (180° Phase Shift)                                                */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */

/* EPWM1: Master — generates SYNCOUT at CTR=ZRO */
EALLOW;
EPWM1REGS.TBCTL.bit.CTRMODE        = 0;       /* Up-Count                                               */
EPWM1REGS.TBCTL.bit.PHSEN          = 0;       /* Master: no phase loading                               */
EPWM1REGS.TBCTL.bit.SYNCOSEL       = 1;       /* SYNCOUT on CTR=ZRO                                     */
EPWM1REGS.TBPRD                    = 10000;
EPWM1REGS.CMPA.bit.CMPA            = 5000;    /* 50% Duty                                               */
EPWM1REGS.AQCTLA.bit.ZRO           = 2;       /* Set at Zero                                            */
EPWM1REGS.AQCTLA.bit.CAU           = 1;       /* Clear at CMPA                                          */
EDIS;

/* EPWM2: Slave — receives SYNCIN, loads phase */
EALLOW;
EPWM2REGS.TBCTL.bit.CTRMODE        = 0;       /* Up-Count                                               */
EPWM2REGS.TBCTL.bit.PHSEN          = 1;       /* Enable phase loading                                   */
EPWM2REGS.TBCTL.bit.SYNCOSEL       = 0;       /* Pass through SYNCIN                                    */
EPWM2REGS.TBPHS.bit.TBPHS          = 5000;    /* 180° = TBPRD / 2                                       */
EPWM2REGS.TBPRD                    = 10000;
EPWM2REGS.CMPA.bit.CMPA            = 5000;    /* Same duty as Master                                    */
EPWM2REGS.AQCTLA.bit.ZRO           = 2;
EPWM2REGS.AQCTLA.bit.CAU           = 1;
EDIS;
```

### 7.3 Komplementär vs. Phasenversetzt

| Konzept | Beschreibung | Anwendung |
|---------|--------------|-----------|
| **Komplementär** (A/B) | Invertierter Pegel, Summe = 100% | H-Brücke diagonal |
| **Phasenversetzt** (Module) | Zeitversetzt, gleicher Duty | Interleaved Wandler |

```
Komplementär:     PWM1A: ▀▀▀▀▀▀____    (60% Duty)
                  PWM1B: ______▀▀▀▀    (40% Duty, automatisch)

Phasenversetzt:   PWM1A: ▀▀▀▀▀▀____    (60% Duty)
                  PWM2A: ___▀▀▀▀▀▀_    (60% Duty, 180° später)
```

---

## 8. Global Load und One-Shot

### 8.1 Problem: Compare-Skip bei Phase-Load

Wenn der Counter durch Phase-Load über einen Compare-Punkt springt, wird das Event verpasst:

```
Counter:   ...198, 199, 200 ──PHASE LOAD──> 300, 301, 302...
                         ↑                    ↑
CMPA=250:                └── nicht erreicht ──┘  (Event verpasst!)
```

**Resultat:** Output bleibt im falschen Zustand bis zum nächsten Zyklus.

### 8.2 Lösung: Global Load mit One-Shot Mode

Global Load ermöglicht **atomare Updates** mehrerer Register zu einem definierten Zeitpunkt:

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* GLOBAL LOAD + ONE-SHOT CONFIGURATION                                                                */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/*   Shadow Registers          Global Load Event           Active Registers                            */
/*   ┌─────────────┐                                      ┌─────────────┐                              */
/*   │ CMPA (new)  │ ─────────────┐                       │ CMPA (old)  │                              */
/*   │ CMPB (new)  │ ─────────────┼──► CTR=ZRO ──────────►│ CMPB (old)  │                              */
/*   │ TBPHS (new) │ ─────────────┘    (atomar)           │ TBPHS (old) │                              */
/*   └─────────────┘                                      └─────────────┘                              */
/*                                                                                                     */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

void init_epwm_global_load(void)
{
    EALLOW;
    
    /* Enable Global Load */
    EPWM1REGS.GLDCTL.bit.GLD        = 1;      /* Global Load enable                                     */
    EPWM1REGS.GLDCTL.bit.GLDMODE    = 0;      /* Load event: CTR = Zero                                 */
    EPWM1REGS.GLDCTL.bit.OSHTMODE   = 1;      /* One-Shot mode: load once, then stop                    */
    
    /* Select which registers use Global Load */
    EPWM1REGS.GLDCFG.bit.CMPA_CMPAHR    = 1;  /* CMPA via Global Load                                   */
    EPWM1REGS.GLDCFG.bit.CMPB_CMPBHR    = 1;  /* CMPB via Global Load                                   */
    EPWM1REGS.GLDCFG.bit.TBPRD_TBPRDHR  = 1;  /* TBPRD via Global Load                                  */
    EPWM1REGS.GLDCFG.bit.AQCTLA_AQCTLA2 = 1;  /* Action Qualifier via Global Load                       */
    
    EDIS;
}

void update_pwm_parameters(uint16_t new_phase, uint16_t new_cmpa)
{
    /* Write to shadow registers (no immediate effect) */
    EPWM1REGS.TBPHS.bit.TBPHS = new_phase;
    EPWM1REGS.CMPA.bit.CMPA   = new_cmpa;
    
    /* Trigger One-Shot: all registers transfer at next load event */
    EPWM1REGS.GLDCTL2.bit.OSHTLD = 1;
}
```

### 8.3 Multi-Modul Linking

Für synchrones Update mehrerer EPWM-Module:

```c
/* Link EPWM2 and EPWM3 to EPWM1's Global Load */
EPWM2REGS.EPWMXLINK.bit.GLDCTL2LINK = 0;     /* Link to EPWM1                                          */
EPWM3REGS.EPWMXLINK.bit.GLDCTL2LINK = 0;     /* Link to EPWM1                                          */

/* Now when EPWM1 triggers OSHTLD, all three modules update simultaneously */
```

### 8.4 Unterschied: Trip Zone One-Shot vs. Global Load One-Shot

| Feature | Trip Zone One-Shot | Global Load One-Shot |
|---------|-------------------|---------------------|
| **Zweck** | Fehler-Handling | Register-Synchronisation |
| **Trigger** | Hardware Trip Signal | Software (OSHTLD) |
| **Aktion** | PWM auf sicheren Zustand | Shadow → Active Transfer |
| **Verhalten** | Bleibt bis Software-Clear | Einmaliger Transfer |
| **Register** | TZCTL, TZOSTFLG | GLDCTL, GLDCTL2 |

---

## 9. Fortgeschrittene Konfigurationen

### 9.1 ADC-Trigger mit CMPC/CMPD

CMPC und CMPD können keine direkten Action Qualifier Aktionen auslösen, aber ADC-Sampling triggern:

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* ADC TRIGGER AT SPECIFIC POINT IN PWM CYCLE                                                          */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/* Counter:   0 ────────> CMPC ────────> CMPA ────────> CMPD ────────> TBPRD                           */
/*            │           │              │              │              │                               */
/* Channel A: ~~~~~~~~~~~~|~~~~~~~~~~~~~~|______________|______________|                               */
/*                        ↑                             ↑                                              */
/*                   ADC Sample 1                  ADC Sample 2                                        */
/*                   (before switch)               (after switch)                                      */
/*                                                                                                     */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
/* PWM Configuration */
ptr_Regs->CMPA.bit.CMPA             = 5000;   /* Switching point                                        */
ptr_Regs->CMPC                      = 4000;   /* ADC trigger before switching                           */
ptr_Regs->CMPD                      = 6000;   /* ADC trigger after switching                            */

/* Event Trigger Configuration */
ptr_Regs->ETSEL.bit.SOCASEL         = 5;      /* SOCA on CMPC up                                        */
ptr_Regs->ETSEL.bit.SOCAEN          = 1;      /* Enable SOCA                                            */
ptr_Regs->ETSEL.bit.SOCBSEL         = 6;      /* SOCB on CMPD up                                        */
ptr_Regs->ETSEL.bit.SOCBEN          = 1;      /* Enable SOCB                                            */
EDIS;
```

### 9.2 Dead-Band für H-Brücken

```c
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/* DEAD-BAND CONFIGURATION - Prevent Shoot-Through in H-Bridge                                         */
/* ═══════════════════════════════════════════════════════════════════════════════════════════════════ */
/*                                                                                                     */
/*             Without Dead-Band:                    With Dead-Band:                                   */
/*   PWMxA:    ▀▀▀▀▀▀▀▀▀▀▀▀|_____________            ▀▀▀▀▀▀▀▀▀▀|_______________                        */
/*   PWMxB:    ____________|▀▀▀▀▀▀▀▀▀▀▀▀▀            ______________|▀▀▀▀▀▀▀▀▀▀▀                        */
/*                         ↑                                   ↑──↑                                    */
/*                    SHOOT-THROUGH!                       Dead-Band (safe)                            */
/*                                                                                                     */
/*                                                                                                     */
/* prt_Regs is a pointer to the EPWMxREGS Register of the dedicated ePWM                               */
/* ─────────────────────────────────────────────────────────────────────────────────────────────────── */

EALLOW;
ptr_Regs->DBCTL.bit.OUT_MODE        = 3;      /* Dead-band on both edges                                */
ptr_Regs->DBCTL.bit.POLSEL          = 2;      /* Active High Complementary (AHC)                        */
ptr_Regs->DBCTL.bit.IN_MODE         = 0;      /* PWMxA is source for both                               */
ptr_Regs->DBRED.bit.DBRED           = 100;    /* Rising Edge Delay: 100 TBCLK cycles                    */
ptr_Regs->DBFED.bit.DBFED           = 100;    /* Falling Edge Delay: 100 TBCLK cycles                   */
EDIS;
```

### 9.3 Software Force für Initialisierung

```c
/* Force known output state at startup */
EALLOW;
ptr_Regs->AQSFRC.bit.ACTSFA         = 1;      /* Force Channel A LOW                                    */
ptr_Regs->AQSFRC.bit.OTSFA          = 1;      /* Trigger one-shot force                                 */
ptr_Regs->AQSFRC.bit.ACTSFB         = 1;      /* Force Channel B LOW                                    */
ptr_Regs->AQSFRC.bit.OTSFB          = 1;      /* Trigger one-shot force                                 */
EDIS;
```

---

## 10. Schnellreferenz

### 10.1 Counter-Modi

| ctrmode | Modus | PWM-Frequenz | Events |
|---------|-------|--------------|--------|
| 0 | Up-Count | TBCLK / (TBPRD+1) | ZRO, CAU, CBU, PRD |
| 1 | Down-Count | TBCLK / (TBPRD+1) | PRD, CAD, CBD, ZRO |
| 2 | Up-Down | TBCLK / (2×TBPRD) | ZRO, CAU, CAD, CBU, CBD, PRD |
| 3 | Freeze | — | — |

### 10.2 Action Qualifier Werte

| Wert | Aktion |
|------|--------|
| 0 | Disabled |
| 1 | Clear (LOW) |
| 2 | Set (HIGH) |
| 3 | Toggle |

### 10.3 Typische Anwendungen

| Anwendung | Counter-Mode | Konfiguration |
|-----------|--------------|---------------|
| LED-Dimming | Up-Count | Rising Edge Aligned |
| Buck-Wandler | Up-Count | Edge Aligned |
| BLDC Motor | Up-Down | Center Aligned + Dead-Band |
| 3-Phasen Inverter | Up-Down | Center Aligned, 120° Phase |
| Interleaved PFC | Up-Count | 180° Phase, Global Load |
| H-Brücke | Up-Down | Komplementär + Dead-Band |

### 10.4 Phasen-Berechnung

```c
/* Allgemeine Formel für N-Phasen Interleaved */
#define CALC_PHASE(n, N, TBPRD)  ((TBPRD) * (n) / (N))

/* Beispiele */
TBPHS_Phase2 = CALC_PHASE(1, 2, TBPRD);  /* 180° für 2-Phasen */
TBPHS_Phase2 = CALC_PHASE(1, 3, TBPRD);  /* 120° für 3-Phasen */
TBPHS_Phase3 = CALC_PHASE(2, 3, TBPRD);  /* 240° für 3-Phasen */
```

---

## 11. Troubleshooting

### 11.1 Checkliste

- [ ] EALLOW vor Register-Schreibzugriffen?
- [ ] EDIS nach Register-Schreibzugriffen?
- [ ] GPIO für PWM-Output konfiguriert?
- [ ] Clock für EPWM-Modul aktiviert?
- [ ] TBPRD ≠ 0?
- [ ] Counter-Mode zum Anwendungsfall passend?

### 11.2 Häufige Probleme

| Problem | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| Kein PWM-Signal | GPIO nicht konfiguriert | GPIO Mux auf EPWM setzen |
| Falsche Frequenz | Prescaler falsch | CLKDIV, HSPCLKDIV prüfen |
| Kein Phase-Sync | PHSEN = 0 | PHSEN = 1 für Slaves |
| Glitches bei Updates | Direkte Register-Writes | Shadow-Register / Global Load verwenden |
| Compare-Skip | Phase-Load überspringt CMPA | Global Load + One-Shot |
| Shoot-Through | Kein Dead-Band | DBCTL konfigurieren |

### 11.3 Debug-Tipps

```c
/* Counter im Debug anhalten */
ptr_Regs->TBCTL.bit.FREE_SOFT = 0;   /* Stop immediately on suspend */
ptr_Regs->TBCTL.bit.FREE_SOFT = 1;   /* Complete current cycle, then stop */
ptr_Regs->TBCTL.bit.FREE_SOFT = 2;   /* Free-run (ignore suspend) */

/* Aktuellen Counter-Wert lesen */
uint16_t current_count = ptr_Regs->TBCTR;

/* Counter-Richtung prüfen (Up-Down Mode) */
uint16_t direction = ptr_Regs->TBSTS.bit.CTRDIR;  /* 0=Down, 1=Up */
```

---

## 12. Siehe auch

- [TMS320F28P65x Technical Reference Manual](https://www.ti.com/lit/ug/spruj53b/spruj53b.pdf) — Kapitel 18: ePWM
- [C2000 ePWM Developer's Guide (SPRAD12A)](https://www.ti.com/lit/an/sprad12a/sprad12a.pdf)
- [Leverage New Type ePWM Features (SPRACY1)](https://www.ti.com/lit/an/spracy1/spracy1.pdf)
- C2000Ware Beispiele: `C:\ti\c2000\C2000Ware\driverlib\f28p65x\examples\epwm`

---

## Anhang A: High-Resolution PWM (HRPWM)

### A.1 Übersicht

Wenn die konventionelle PWM-Auflösung nicht ausreicht (typisch unter 9-10 Bit), bietet HRPWM zusätzliche Auflösung durch **MEP (Micro Edge Positioner)** Technologie. MEP positioniert Flanken in ~150ps Schritten.

> **Hinweis:** Nicht alle EPWM-Module haben HRPWM-Fähigkeiten. Siehe Device Datasheet für Details.

### A.2 Relevante Register

| Register | Beschreibung |
|----------|--------------|
| **HRCNFG** | HRPWM Configuration — Aktivierung und Modus (nur bei HRPWM-fähigen Modulen) |
| **CMPA.CMPAHR** | High-Resolution Compare A (8 zusätzliche Bits) |
| **CMPB.CMPBHR** | High-Resolution Compare B (8 zusätzliche Bits) |
| **TBPRDHR** | High-Resolution Period |
| **HRMSTEP** | MEP Scale Factor (von SFO kalibriert, global) |

### A.3 Kalibrierung mit SFO

Die MEP-Schrittgröße variiert mit Temperatur und Spannung. Die **SFO (Scale Factor Optimizer)** Bibliothek kalibriert zur Laufzeit:

```c
#include "SFO_V8.h"

int status;

void main(void)
{
    /* Initiale Kalibrierung (blockierend) */
    while(SFO() == SFO_INCOMPLETE);
    
    for(;;)
    {
        /* Regelung, etc. */
        
        /* SFO im Hintergrund — non-blocking, aktualisiert HRMSTEP */
        status = SFO();
        if(status == SFO_ERROR) { /* Handle error */ }
    }
}
```

**Warum Kalibrierung nötig ist:**

| Bedingung | MEP-Schrittgröße |
|-----------|------------------|
| 25°C, nominal | ~140ps |
| 85°C, nominal | ~170ps |
| **Abweichung** | **~20%** |

Ohne Laufzeit-Kalibrierung wäre die HRPWM-Präzision bei Temperaturänderungen nicht gewährleistet.

### A.4 Typische Anwendung

```c
/* HRPWM auf Channel A aktivieren */
EALLOW;
EPwm1Regs.HRCNFG.bit.EDGMODE    = 1;    /* Rising edge HR control                */
EPwm1Regs.HRCNFG.bit.CTLMODE    = 0;    /* Duty control via CMPAHR               */
EPwm1Regs.HRCNFG.bit.HRLOAD     = 0;    /* Load on ZRO                           */
EPwm1Regs.HRCNFG.bit.AUTOCONV   = 1;    /* Auto-convert CMPAHR                   */
EDIS;
```

> **Siehe auch:** TI Application Note HRPWM Reference Guide für Details zur Konfiguration.

---

## Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.1** | **2025-12-18** | **Anhang A: HRPWM Kurzübersicht mit SFO-Kalibrierung** |
| 0.1.0 | 2025-12-18 | Initial: Counter-Modi, PWM-Grundkonfigurationen, Compare-Register, Action Qualifier, Phasenversatz, Global Load, Troubleshooting |

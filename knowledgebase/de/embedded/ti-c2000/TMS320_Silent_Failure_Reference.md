# TMS320 / C2000 Silent Failure Reference

> **Version:** 0.1.0  
> **Datum:** 2025-12-19  
> **Typ:** Reference  
> **Status:** In Entwicklung  
> **Zielgruppe:** Embedded-Entwickler, Firmware-Entwickler  
> **Sprache:** Englisch  
> **English:** —

---

## Inhaltsverzeichnis

1. [Overview](#1-overview)
2. [EPWM — Enhanced Pulse Width Modulator](#2-epwm--enhanced-pulse-width-modulator)
3. [HRPWM — High Resolution PWM](#3-hrpwm--high-resolution-pwm)
4. [ADC — Analog-to-Digital Converter](#4-adc--analog-to-digital-converter)
5. [PIE Interrupt Controller](#5-pie-interrupt-controller)
6. [DMA — Direct Memory Access](#6-dma--direct-memory-access)
7. [FLASH / OTP / ECC](#7-flash--otp--ecc)
8. [IPC / Dual Core](#8-ipc--dual-core)
9. [Protected Registers (EALLOW)](#9-protected-registers-eallow)
10. [System Control / Clocking](#10-system-control--clocking)
11. [GPIO](#11-gpio)
12. [SCI / SPI / I2C](#12-sci--spi--i2c)
13. [Debugging Checklist](#13-debugging-checklist)
14. [Siehe auch](#14-siehe-auch)
15. [Changelog](#15-changelog)

---

## 1. Overview

### Definition

> A **silent failure** is any configuration or register operation that appears to be accepted by the hardware but does **not** produce the intended functional effect, **without** raising a fault, interrupt, or explicit error indication.

### Subsystem Summary

| Subsystem | Typical Silent Failure Sources |
|-----------|-------------------------------|
| EPWM | Lock key usage, shadow vs. active, timer mode changes, TBCLKSYNC, EALLOW |
| HRPWM | Missing calibration, invalid HRMSTEP, wrong edge mode, HR clock off |
| ADC | SOC without ADC clock, sampling window too short, reference not settled |
| PIE Interrupts | IER/PIEEN mismatch, missing ACK, wrong ISR declaration/type |
| DMA | Missing auto-init, missing trigger, CPU overwriting active DMA region |
| FLASH / OTP / ECC | ECC silent correction, missing wait states, program/read timing |
| IPC / Dual Core | Secondary core not started, missing ACK, unconfigured shared RAM |
| SysCtrl / Clocking | Peripheral clock disabled, TBCLKSYNC disabled, PLL issues |
| GPIO | Input qualification filtering, floating inputs, missing pull-ups |
| SCI / SPI / I2C | CPOL/CPHA mismatch, baud misconfiguration, repeated-start issues |

---

## 2. EPWM — Enhanced Pulse Width Modulator

### 2.1 Summary

| Category | Effect | Why Subtle | Symptoms |
|----------|--------|------------|----------|
| **EPWMLOCK key not in single 32-bit write** | Write ignored | KEY must be written simultaneously with payload | Trip-zone config unchanged, dead-band not updated |
| **Shadow-to-active transfer not triggered** | New values never become active | Write only updates shadow | CMPA/B written but duty unchanged |
| **TBCTL/CTRMODE change while running** | Glitches | Timer mode changes have timing windows | Duty jumps, phase shifts, alignment loss |
| **TBCLKSYNC disabled** | Time-base does not run | Registers update but counter held in reset | PWM outputs remain static |
| **EALLOW missing** | Writes ignored | Protection enforced without error | Protected bits never change |

### 2.2 EPWMLOCK — Key and Bits in One Write

**Root Causes:**
- Using bitfield syntax `reg.bit.xxx = 1;` directly on hardware register
- Separate writes for key and payload
- Compiler generating 16-bit accesses instead of 32-bit

**Mitigation:**
```c
// ✅ Correct: Single 32-bit write
uint32_t lockValue = (EPWM_LOCK_KEY << 16) | EPWM_LOCK_TZCLR;
EPwm1Regs.EPWMLOCK.all = lockValue;

// ❌ Wrong: Separate writes
EPwm1Regs.EPWMLOCK.bit.KEY = 0xA5A5;  // May be ignored
EPwm1Regs.EPWMLOCK.bit.TZCLR = 1;     // May be ignored
```

### 2.3 Shadow-to-Active Transfer

**Root Causes:**
- Load mode left at default that never triggers
- Load configured on event (e.g. zero) that doesn't occur in current timer mode

**Mitigation:**
- Document **when** shadow-to-active transfer occurs (zero, period, immediate)
- Force known load events during bring-up
- Avoid dynamic load mode changes

### 2.4 Timer Mode Changes

**Root Causes:**
- Switching CTRMODE while counter at arbitrary position
- Applying sync events during compare/dead-band updates

**Mitigation:**
- Change CTRMODE only when TBCTR = 0 or TBCLKSYNC disabled
- Sequence: disable → configure → clear counter → enable

### 2.5 TBCLKSYNC and EALLOW

**TBCLKSYNC = 0:** Time-base clock disabled; registers update but no counter activity.

**EALLOW missing:** Writes to protected registers silently ignored.

**Mitigation:**
- Use central initialization with documented TBCLKSYNC enable point
- Wrap protected writes: `EALLOW(); reg = value; EDIS();`

---

## 3. HRPWM — High Resolution PWM

### 3.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **HRMSTEP invalid/not calibrated** | No real HR behavior | Duty changes in coarse steps only |
| **Calibration while PWM active** | Edges jitter/shift | Random jitter or phase jumps |
| **Wrong edge mode** | HR applies to wrong edge | One transition fine, other coarse |
| **HR clock disabled** | HR features silently off | PWM works but without fine resolution |

### 3.2 Mitigation

- Perform MEP/HRMSTEP calibration **before** enabling EPWM outputs
- Don't run HR calibration during time-critical switching
- Verify HR control bits match desired edge (rising/falling, A/B)
- Ensure all relevant clocks enabled before configuring HRPWM

---

## 4. ADC — Analog-to-Digital Converter

### 4.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **SOC but ADC clock off** | No conversions | EOC flags never set, ISR not triggered |
| **Sampling window too short** | Under-sampling | Measurements consistently low or noisy |
| **VREF not settled** | Gain/offset drift | Reading drifts over time/temperature |
| **Interrupt not firing** | ISR never called | Conversion appears stuck |

### 4.2 Mitigation

1. **ADC Clock:** Verify ADC module clock enabled before configuring SOCs
2. **Sampling Window:** Dimension according to worst-case source impedance; validate with known voltage sources
3. **Reference Settling:** Apply recommended settling time before using results
4. **Interrupt Chain:** Cross-check ADCINT → PIE channel → IER bit → ACK handling

---

## 5. PIE Interrupt Controller

### 5.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **PIEENx set but IER off** | Interrupt never reaches CPU | Peripheral indicates interrupt, CPU doesn't react |
| **IER set but no PIEACK** | Interrupt fires only once | First event works, subsequent ignored |
| **Wrong ISR declaration** | Improper return/stack | Random behavior or no ISR execution |
| **Wrong vector index** | Different ISR invoked | Debugger shows vector but code path wrong |

### 5.2 Interrupt Enable Chain

```
Peripheral Flag → PIE Channel Enable → PIE Group Enable → CPU IER Bit → Global Enable
```

### 5.3 Mitigation

- Treat interrupt enable as complete chain
- **Always issue PIEACK** for its PIE group before ISR return
- Follow compiler/device guidelines for ISR declaration
- Maintain central mapping table: interrupt sources ↔ ISR functions

---

## 6. DMA — Direct Memory Access

### 6.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **Trigger never occurs** | DMA doesn't start | Config looks correct but no movement |
| **Auto-init not configured** | Single transfer only | First block works, subsequent don't |
| **CPU overwrites DMA region** | Data corruption | Memory contents inconsistent, no bus fault |

### 6.2 Mitigation

- Validate trigger source (toggle GPIO in corresponding ISR or use known periodic source)
- Configure auto-reload/continuous mode for continuous transfers
- Clearly separate DMA-owned and CPU-owned memory regions

---

## 7. FLASH / OTP / ECC

### 7.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **Missing wait states** | Timing violations | Rare, non-reproducible field failures |
| **ECC corrects silently** | Errors masked | System runs with hidden single-bit errors |
| **Read-while-program/erase** | Undefined read data | Spurious wrong values, no fault |
| **OTP misinterpreted** | Wrong calibration | Peripheral behavior incorrect from reset |

### 7.2 Mitigation

- Configure FLASH wait states per frequency/voltage requirements
- Monitor ECC error counters; treat recurring corrections as warning
- Avoid reading FLASH sectors during program/erase
- Verify OTP values during bring-up

---

## 8. IPC / Dual Core

### 8.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **Secondary core not started** | IPC messages not processed | One core functional, other idle |
| **IPC flag/ACK not handled** | Deadlock | Both cores waiting on each other |
| **Shared RAM not configured** | Writes lost/invalid | One core doesn't see data from other |

### 8.2 Mitigation

- Explicitly release secondary core from reset and verify boot
- Implement robust IPC: clear ownership, timeouts, acknowledgement
- Configure shared RAM consistently on both cores

---

## 9. Protected Registers (EALLOW)

### 9.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **Write without EALLOW** | Write silently ignored | Config code executes but register unchanged |
| **RMW blocked** | Partial/ignored bits | Some bits update, others remain |
| **EALLOW scope too wide** | Unintentional modification | Debugging difficulties, unexpected side effects |

### 9.2 Mitigation

```c
// ✅ Small, local scope
EALLOW;
SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;
EDIS;

// ❌ Wide scope (risky)
EALLOW;
// ... 50 lines of code ...
EDIS;
```

- Annotate write-protected registers in documentation
- Code reviews focusing on protection boundary correctness

---

## 10. System Control / Clocking

### 10.1 Summary

| Category | Effect | Symptoms |
|----------|--------|----------|
| **Peripheral clock disabled** | Writes accepted, no operation | Registers change but no activity |
| **TBCLKSYNC disabled** | PWM time-base doesn't run | EPWM outputs static despite config |
| **PLL not locked** | Timing-dependent issues | Behavior changes with temp/supply/freq |

### 10.2 Mitigation

- Central system-clock initialization; explicitly enable all required peripheral clocks
- Separate config phase (clocks on, modules disabled) from active phase (modules enabled, TBCLKSYNC released)
- Check/monitor PLL lock bits

---

## 11. GPIO

### 11.1 Silent Failures

- **Floating inputs:** Random logic levels without digital error
- **Input qualification too long:** Apparent loss of events (buttons unresponsive)

### 11.2 Mitigation

- Always define pull configuration for unused/input pins
- Dimension input qualification based on signal characteristics

---

## 12. SCI / SPI / I2C

### 12.1 Silent Failures

- **SPI CPOL/CPHA mismatch:** Valid bit patterns sampled on wrong edge
- **Baud rate misconfiguration:** Framing issues without obvious error flags
- **I2C repeated-start:** Unexpected behavior if timing requirements not met

### 12.2 Mitigation

- Verify mode settings against external device specification
- Check error status registers periodically and during bring-up
- Use oscilloscope/logic analyzer for protocol-level validation

---

## 13. Debugging Checklist

When a peripheral appears "configured" but doesn't behave as expected:

| # | Check | Question |
|---|-------|----------|
| 1️⃣ | **Clocking** | Is peripheral clock enabled? For EPWM: is TBCLKSYNC released? |
| 2️⃣ | **Protection** | Does register require EALLOW/EDIS? For lock registers: correct key in single 32-bit write? |
| 3️⃣ | **Shadow vs. Active** | Does register use shadowing? Is transfer correctly configured and triggered? |
| 4️⃣ | **Interrupts** | Full chain enabled? (peripheral → PIE → IER → global) ACKs issued in ISRs? |
| 5️⃣ | **Timing** | ADC: sampling window long enough? Reference settled? PWM: config changes in safe timing windows? |
| 6️⃣ | **Multi-core** | Both cores running? IPC and shared RAM properly configured and acknowledged? |
| 7️⃣ | **External** | Confirm behavior with scope/logic analyzer, not just register views |

---

## 14. Siehe auch

- [TMS320_Silent_Failure_Cheatsheet.md](TMS320_Silent_Failure_Cheatsheet.md) — Quick reference
- [EPWM_Configuration_Reference.md](EPWM_Configuration_Reference.md) — ePWM peripheral
- TMS320F28P65x Technical Reference Manual

---

## 15. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.0** | **2025-12-19** | **Initial: Konvertiert aus Tms320_Silent_Failure_Documentation.docx** |

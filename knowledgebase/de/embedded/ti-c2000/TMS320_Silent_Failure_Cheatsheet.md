# TMS320 / C2000 Silent Failure Cheatsheet

> **Version:** 0.1.0  
> **Datum:** 2025-12-19  
> **Typ:** Cheatsheet  
> **Status:** In Entwicklung  
> **Zielgruppe:** Embedded-Entwickler, Firmware-Entwickler  
> **Sprache:** Englisch  
> **English:** —

---

## Inhaltsverzeichnis

1. [EPWM / HRPWM](#1-epwm--hrpwm)
2. [ADC](#2-adc)
3. [PIE Interrupts](#3-pie-interrupts)
4. [DMA](#4-dma)
5. [FLASH / OTP / ECC](#5-flash--otp--ecc)
6. [IPC / Dual Core](#6-ipc--dual-core)
7. [Protected Registers (EALLOW)](#7-protected-registers-eallow)
8. [SysCtrl / Clocking](#8-sysctrl--clocking)
9. [Quick Debug Order](#9-quick-debug-order)
10. [Siehe auch](#10-siehe-auch)
11. [Changelog](#11-changelog)

---

**Use case:** Print, laminate, keep next to oscilloscope.

---

## 1. EPWM / HRPWM

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **EPWMLOCK key not in same 32-bit write** | Config ignored | KEY + bits require one write | Build value local → single write |
| **Shadow write not active** | PWM unchanged | No load event | Configure load (Zero/Period/Immediate) |
| **CTRMODE change while running** | Glitch / jump | Timing window | Change only at TBCTR = 0 |
| **TBCLKSYNC = 0** | Output static | Clock gated | Enable after config |
| **HRMSTEP not calibrated** | Coarse changes | Calibration missing | Calibrate before enabling |
| **HR edge mode mis-set** | One edge coarse | Wrong mode | Match topology |

---

## 2. ADC

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **ADC clock disabled** | No conversion | Clock off | Enable ADC clock first |
| **Sample window too short** | Low readings | Settling | Increase ACQPS |
| **VREF not settled** | Drift | Reference time | Follow datasheet settle time |
| **ADCINT not firing** | ISR dead | PIE/IER mismatch | Enable chain + ACK |

---

## 3. PIE Interrupts

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **PIEEN but CPU IER off** | ISR never runs | Chain incomplete | Enable PIE + IER |
| **No PIEACK** | Only first ISR | ACK missing | Issue ACK before return |
| **Wrong ISR declaration** | Random behavior | ABI mismatch | Use correct ISR prototype |

---

## 4. DMA

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **No transfer** | No movement | Trigger not firing | Validate trigger source |
| **Single run only** | First copy works | No auto-reload | Enable continuous/auto-init |
| **Data corruption** | Random values | CPU writing buffer | Separate DMA/CPU regions |

---

## 5. FLASH / OTP / ECC

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **No wait states** | Field failures | Timing | Configure WS correctly |
| **ECC silent correct** | Hidden errors | Bit upset | Monitor ECC counters |
| **Read-while-program** | Wrong values | Undefined | Avoid access during program |

---

## 6. IPC / Dual Core

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **CPU2 inactive** | No response | Boot not released | Start secondary core |
| **Deadlock** | Both waiting | No ACK | Add ACK + timeout |
| **Shared RAM lost** | No visibility | Not configured | Match region ownership |

---

## 7. Protected Registers (EALLOW)

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **Write ignored** | Config unchanged | No EALLOW | Wrap write in EALLOW/EDIS |
| **Partial write (RMW)** | Bits not updated | Compiler access width | Use 32-bit write |

---

## 8. SysCtrl / Clocking

| Failure | Symptom | Root | Fix |
|---------|---------|------|-----|
| **Peripheral clock off** | No activity | Clock gating | Enable module clock |
| **TBCLKSYNC off** | PWM no counter | Held reset | Enable after config |
| **PLL not locked** | Timing drift | Unstable clock | Verify lock bits |

---

## 9. Quick Debug Order

```
1️⃣ Clock enabled?
2️⃣ EALLOW required?
3️⃣ Shadow vs. active?
4️⃣ Interrupt chain complete?
5️⃣ Timer/Sync window safe?
6️⃣ Dual-core ACK + RAM ownership correct?
7️⃣ Validate externally — scope/LA, not debugger only.
```

---

## 10. Siehe auch

- [TMS320_Silent_Failure_Reference.md](TMS320_Silent_Failure_Reference.md) — Detailed documentation
- [EPWM_Configuration_Reference.md](EPWM_Configuration_Reference.md) — ePWM peripheral

---

## 11. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **0.1.0** | **2025-12-19** | **Initial: Konvertiert aus Tms320_Silent_Failure_Cheat_Sheet.docx** |

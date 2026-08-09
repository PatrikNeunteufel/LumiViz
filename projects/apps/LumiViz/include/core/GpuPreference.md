# GpuPreference — Persistente GPU-Auswahl (Windows UserGpuPreferences)

> **Version:** 1.0.0  
> **Datum:** 2026-08-01  
> **Typ:** CppModuleDoc  
> **Status:** Implementiert  
> **Modul:** LumiViz::Core::GpuPreference  
> **Dateien:** GpuPreference.hpp, GpuPreference.cpp  
> **Namespace:** (global)  
> **Abhängigkeiten:** Windows-Registry (advapi32, automatisch), BasicLogger  
> **Zielgruppe:** Entwickler  
> **Sprache:** Deutsch  

---

## Inhaltsverzeichnis

1. [Übersicht](#1-übersicht)
2. [Mechanismus](#2-mechanismus)
3. [API](#3-api)
4. [Verwendung](#4-verwendung)
5. [Grenzen und Entscheide](#5-grenzen-und-entscheide)
6. [Changelog](#6-changelog)

---

## 1. Übersicht

Zur Laufzeit lässt sich die GPU in OpenGL nicht umschalten. Der verlässliche
Weg für eine einstellbare, **persistente** Präferenz ist der
per-Anwendung-Eintrag von Windows — dasselbe, was die Windows-Oberfläche
unter *System → Anzeige → Grafikeinstellungen* schreibt. Dieses Modul liest
und schreibt genau diesen Eintrag; er ist die **einzige Steuerung (SSOT)**.

Der Eintrag greift beim **nächsten Prozessstart**. Deshalb löst das
SettingsPanel bei einer Änderung sofort den Neustart der App aus
(Entscheid S61: kein „gilt ab nächstem Start"-Hinweis-Zustand).

## 2. Mechanismus

```
HKCU\Software\Microsoft\DirectX\UserGpuPreferences
  Wertname (REG_SZ) = voller Exe-Pfad, native Separatoren
                      z. B. C:\...\LumiViz.exe
  Daten             = "GpuPreference=N;"  (ggf. neben fremden Tokens)
```

| N | Modus | Bedeutung |
|---|-------|-----------|
| 0 | `Automatic` | Windows entscheidet |
| 1 | `PowerSaving` | integrierte GPU bevorzugen |
| 2 | `HighPerformance` | dedizierte GPU bevorzugen |

Fremde Tokens im selben Wert (z. B. `SwapEffectUpgradeEnable=1;`) bleiben beim
Schreiben erhalten — `upsertToken` ersetzt nur den `GpuPreference`-Token.

## 3. API

| Methode | Beschreibung |
|---------|--------------|
| `parseToken(value)` | liest den Modus aus einem Registry-Datenwert (pur) |
| `upsertToken(value, mode)` | setzt/ersetzt den Token, erhält fremde Tokens (pur) |
| `modeToString(mode)` | lesbarer Name fürs Log |
| `readForExecutable(exePath)` | gespeicherte Präferenz einer Exe (nullopt = kein Eintrag) |
| `writeForExecutable(exePath, mode)` | schreibt die Präferenz (legt Schlüssel an) |

Die Token-Logik ist pur und ohne Registry testbar
(`test_GpuPreference.cpp`); nur die zwei `…ForExecutable` fassen die Registry
an (auf Nicht-Windows: No-ops).

## 4. Verwendung

```cpp
// SettingsPanel: Combo-Änderung -> schreiben -> Sofort-Neustart
const std::wstring exe = QDir::toNativeSeparators(
    QCoreApplication::applicationFilePath()).toStdWString();
if (GpuPreference::writeForExecutable(exe, GpuPreference::Mode::HighPerformance))
{
    QProcess::startDetached(...);   // neue Instanz liest den neuen Eintrag
    QCoreApplication::quit();
}

// Application::init(): gespeicherte Präferenz loggen
const auto stored = GpuPreference::readForExecutable(exe);
```

Welche Karte **wirklich** rendert, zeigt `GL_RENDERER` nach dem
Kontext-Aufbau — das SettingsPanel zeigt beides nebeneinander an
(Einstellung + tatsächliche Karte).

## 5. Grenzen und Entscheide

- **Nur Windows.** Auf anderen Plattformen sind die Registry-Funktionen No-ops.
- **Pfadgebunden:** Der Eintrag hängt am Exe-Pfad. Wandert die Exe (anderer
  Build-Ordner, Deployment), gilt der Eintrag der neuen Exe — Verhalten wie
  bei der Windows-Oberfläche selbst.
- **Export-Flags entfernt (S62):** `NvOptimusEnablement` /
  `AmdPowerXpressRequestHighPerformance` waren nicht einstellbar, wurden von
  Windows überstimmt, sobald ein UserGpuPreferences-Eintrag existiert — und
  griffen ohne Eintrag auf diesem Gerät (AMD-iGPU + NVIDIA-dGPU) nachweislich
  nicht. Die Registry ist die einzige Steuerung.
- **§8-Messregel:** Ein GPU-Wechsel ändert Treiber-Interpolation/-Rundung.
  Matrix, Modul-/Feld-Sonden und die Bit-Identität hängen an der Karte —
  Umbauten an diesem Mechanismus nur mit Vorher/Nachher-Messlauf
  (Baseline: `out/gpu_baseline_radeon610m/`).

## 6. Changelog

| Version | Datum | Änderungen |
|---------|-------|------------|
| **1.0.0** | **2026-08-01** | **Initial (Session 62): UserGpuPreferences lesen/schreiben, pure Token-Logik, SettingsPanel-Anbindung mit Sofort-Neustart; löst gpu.ini/GpuSelector/Export-Flags ab** |

# 📊 Sprint-01 Tag-01 Abschlussbericht

## 🎯 Ziel laut Projektplan
**Titel:** `Sprint-01 Tag-01: Projektgrundgerüst & Ordnerstruktur`  
**Prompt:**
> „Leite mich Schritt für Schritt beim Aufbau der Projektstruktur (Ordner, source.cmake, common/Types.hpp, clang-format etc.). Ziel ist eine kompakte, erweiterbare Basis.“

**Referenzen:**  
- 0826T23 Core Vereinheitlichung Vorschlag  
- 0826T22 Projektanalyse C++ Anwendung (für Gesamtaufbau src/)

---

## ✅ Erfüllte Punkte
- **Projektstruktur erstellt**
  - `src/` mit Unterordnern (`app/`, `common/`, `core/`)
  - `cmake/` für modulare Konfiguration
  - `.clang-format`, `.editorconfig`, `.gitignore`
- **CMake modularisiert**
  - Top-Level `CMakeLists.txt` reduziert auf reine `include(...)`
  - Unterteilung in `Init.cmake`, `Options.cmake`, `Settings.cmake`, `Sources.cmake`, `Targets.cmake`, `Warnings.cmake`, `CompilerOptions.cmake`, `LTO.cmake`, `Output.cmake`
  - Erweiterungsmodule: `Bass.cmake`, `Lua.cmake`, `OpenGLSetup.cmake`, `Fetch_GLFW_ImGui.cmake`
- **common/**
  - `Types.hpp` (Grundtypen, StrongId, Projektkonstante)
  - `Attributes.hpp` (Makros für `VIZ_NODISCARD`, `VIZ_LIKELY`, `VIZ_UNLIKELY`)
- **Formatierungs-Setup**
  - `.clang-format` abgestimmt mit `.editorconfig`
  - clang-format-Targets: `clang-format-check`, `clang-format-fix`
- **Basisklassen**
  - `ServiceContainer.hpp`
  - `EventBus.hpp`
  - `CommandBus.hpp`
  - `Application.hpp/.cpp`
  - `main.cpp`

---

## ⚠️ Offene Punkte von Tag 1
- Keine funktionalen Tests/Unit-Tests erstellt (wäre erst ab Tag 2+ relevant).
- Noch keine tieferen Manager-/Controller-/Registry-Basen (geplant ab Tag 5).

---

## 📦 Vorweggenommene Inhalte späterer Tage
- **Tag 2 – ServiceContainer**  
  → Bereits vollständig implementiert (`ServiceContainer.hpp`).

- **Tag 3 – EventBus**  
  → Bereits implementiert (`EventBus.hpp`).

- **Tag 4 – CommandBus**  
  → Bereits implementiert (`CommandBus.hpp`).

- **Tag 5 – Basisklassen für Manager/Controller/Registry/Agent**  
  → Noch nicht umgesetzt, aber Vorarbeit geleistet (Application-Klasse nutzt bereits Services und Busse).

- **Tag 6 – Application-Schicht**  
  → Grundgerüst in `Application.hpp/.cpp` vorhanden (init/tick/shutdown).  
  → `ApplicationController`/`ApplicationLifetime` noch offen.

---

# 📑 Merkblatt für die Übernahme

## 1. Typen
- Verwende projektweit die Kurztypen aus `viz::types` (`u32`, `f64`, …).
- IDs immer mit `StrongId` anlegen:
  ```cpp
  using NodeId = viz::types::StrongId<u64, struct NodeIdTag>;
  ```

## 2. Attribute
- Immer `VIZ_NODISCARD` statt `[[nodiscard]]` einsetzen:
  ```cpp
  VIZ_NODISCARD bool has() const;
  ```
- Performance-Hints mit `VIZ_LIKELY(expr)` und `VIZ_UNLIKELY(expr)`.

### Erklärung zu `VIZ_LIKELY` / `VIZ_UNLIKELY`

Die beiden Makros sind **Compiler-Hints** für die **Branch Prediction** der CPU:

```cpp
#define VIZ_LIKELY(x)   __builtin_expect(!!(x), 1)
#define VIZ_UNLIKELY(x) __builtin_expect(!!(x), 0)
```

- Moderne CPUs raten bei `if`-Abfragen, ob ein Zweig wahr ist, und arbeiten spekulativ vor.
- **Richtig geraten** → schnell ✅
- **Falsch geraten** → Pipeline flushen, teuer ❌

Mit `__builtin_expect` geben wir dem Compiler einen Hinweis:
- `VIZ_LIKELY(expr)` → Ausdruck ist meistens **true**
- `VIZ_UNLIKELY(expr)` → Ausdruck ist meistens **false**

➡️ Der Compiler optimiert die Maschinenbefehle so, dass der **wahrscheinliche Zweig** bevorzugt wird.

**Typische Einsätze:**
- Fehler- oder Ausnahmefälle: `if (VIZ_UNLIKELY(error))`
- Assertions: `if (VIZ_UNLIKELY(!condition)) abort();`
- Hot-Loops: `if (VIZ_LIKELY(i < max))`

**Wichtig:**
- Nur ein **Hint** → CPU kann eigene Heuristiken nutzen.
- Übermäßiger Einsatz macht Code unleserlich.
- Auf MSVC wird es zu `(x)` gemappt → portable, aber ohne Hint.

---

## 3. Safe IDs (StrongId)
- IDs sind nicht austauschbar (kein Vergleich zwischen z. B. `AudioId` und `VisualId`).
- Zugriff auf den Wert nur bewusst über `.get()` oder `explicit operator T()`:
  ```cpp
  AudioId aid{42};
  u32 raw = static_cast<u32>(aid); // bewusst
  ```
- Vergleich nur zwischen gleichen Typen:
  ```cpp
  AudioId a1{1}, a2{2};
  if (a1 == a2) { /* ... */ }
  ```

## 4. Formatierung
- `.editorconfig` regelt Editor-Disziplin (LF, UTF-8, 4 Spaces, newline at EOF).
- `.clang-format` regelt Layout (Allman-Braces, ColumnLimit 120, SortIncludes).
- Externe Ordner (z. B. `external/`) sind aus Formatierung ausgeschlossen.

## 5. CMake-Organisation
- **Nur `CMakeLists.txt` bearbeiten**, Rest modular unter `cmake/`.
- Wichtige Optionen:
  - `VIZ_WARNINGS_AS_ERRORS=ON`
  - `VIZ_NO_EXCEPTIONS=ON` (standardmäßig OFF, falls Exceptions benötigt werden)
  - `VIZ_ENABLE_LTO=ON` für Interprocedural Optimization
- Hilfs-Targets:
  - `clang-format-check` / `clang-format-fix`
  - `run-clang-tidy` (falls aktiviert)
  - `install` (falls aktiviert)

---

✅ Damit ist Sprint-01 Tag-01 abgeschlossen, mit Vorwegnahme der Kernteile von Tag 2–4 und Teilen von Tag 6.  
⚠️ Ab Tag 5 weiter mit Basisklassen für Manager/Controller/Registry/Agent.


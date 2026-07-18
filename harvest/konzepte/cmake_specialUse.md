Gute Fragen—kurz & präzise:

---

# Unterschied: `CMAKE_CURRENT_SOURCE_DIR` vs. `CMAKE_CURRENT_LIST_DIR`

* **`CMAKE_CURRENT_SOURCE_DIR`**
  Verweist auf das **Quellverzeichnis der gerade aktiven `CMakeLists.txt`-Directory** (Scope durch `add_subdirectory()` bestimmt).
  → Benutzen in **CMakeLists** innerhalb eines Verzeichnisses, um Pfade relativ zu **diesem Verzeichnis** zu setzen.

* **`CMAKE_CURRENT_LIST_DIR`**
  Verweist auf den **Ordner der aktuell eingelesenen Datei** – also auch für **eingebundene Module** (`include(...)`).
  → Benutzen in **.cmake-Modulen** unter `cmake/`, wenn du z. B. `../src/Source.cmake` einbinden willst. Es ist robust, egal von wo aus das Modul inkludiert wird.

**Praxis-Regel:**

* In `CMakeLists.txt` (pro Ordner): `CMAKE_CURRENT_SOURCE_DIR`
* In **ausgelagerten Modulen** (`cmake/*.cmake`): `CMAKE_CURRENT_LIST_DIR`
  (und dann oft `set(_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")`, um sicher auf das Repo-Root zu zeigen)

**Warum dein Fehler auftrat:**
Dein Modul lag in `cmake/`. Hättest du dort `CMAKE_CURRENT_SOURCE_DIR` verwendet, zeigt das (beim Include aus dem Projekt-Root) auf das **Projekt-Root**, und `../src/...` läuft ins Leere. Mit `CMAKE_CURRENT_LIST_DIR` zeigt es korrekt auf `.../cmake`, und `../src/...` passt.

---

# Wie aktivieren/benutzen: Testing, Install, clang-format, clang-tidy

Ich habe dir dafür optionale Module + CMake-Optionen angelegt. Du steuerst **alles über CMake-Optionen** oder **explizite Targets**.

## 1) Testing

* **Aktivieren:**

  * CLI/Ninja:

    ```bash
    cmake -S . -B build -DVIZ_ENABLE_TESTS=ON
    cmake --build build -j
    ctest --test-dir build
    ```
  * Visual Studio: in **CMakeSettings.json** oder über „**CMake → Change CMake Settings**“ die Konfiguration um `-DVIZ_ENABLE_TESTS=ON` erweitern. Tests erscheinen im **Test-Explorer**, wenn dein `tests/`-Ordner existiert.
* **Was passiert:**
  `enable_testing()` wird gesetzt; optional `add_subdirectory(tests)`.

## 2) Install

* **Aktivieren:**

  * CLI:

    ```bash
    cmake -S . -B build -DVIZ_ENABLE_INSTALL=ON
    cmake --build build --target install
    ```

    Optional Zielpfad: `-DCMAKE_INSTALL_PREFIX="E:/…/out/install/x64-debug"`
  * Visual Studio: „Build → Install <Project>“ (erscheint als **INSTALL** Target), vorher `-DVIZ_ENABLE_INSTALL=ON`.
* **Was passiert:**
  `install(TARGETS …)` schreibt Binärdateien nach `bin/`, libs nach `lib/` etc.

## 3) Packaging (falls eingebunden)

* **Aktivieren:**

  ```bash
  cmake -S . -B build -DVIZ_ENABLE_INSTALL=ON -DVIZ_ENABLE_PACKAGING=ON
  cmake --build build --target package
  ```
* **Was passiert:**
  CPack erzeugt je nach Generator (z. B. ZIP/TGZ) ein Paket. Setz den Generator in `Packaging.cmake`.

## 4) clang-format

* **CMake tut von sich aus nichts.**
  Du hast **manuelle Targets**:

  * **Check (nur prüfen, bricht bei Abweichungen ab):**

    ```bash
    cmake --build build --target clang-format-check
    ```
  * **Fix (formatieren):**

    ```bash
    cmake --build build --target clang-format-fix
    ```
  * Visual Studio: im **CMake Targets View** oder „Build → Build Target…“ das jeweilige Target auswählen.
  * Voraussetzung: `clang-format` muss im PATH sein; `.clang-format` liegt im Repo-Root.

## 5) clang-tidy (zwei Modi)

* **On-Build (prüft während des Kompilierens, ändert nichts):**

  ```bash
  cmake -S . -B build -DVIZ_TIDY_ON_BUILD=ON
  cmake --build build
  ```

  → CMake hängt `clang-tidy` an jeden Compile-Aufruf. **Keine Auto-Fixes**, solange `VIZ_TIDY_FIX=OFF`.
* **Manuell (einmalig aufrufen, optional mit Fix):**

  ```bash
  cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build build
  # Nur prüfen:
  cmake --build build --target run-clang-tidy
  # Mit Fixes:
  cmake -S . -B build -DVIZ_TIDY_FIX=ON
  cmake --build build --target run-clang-tidy
  ```

  * Visual Studio: ebenfalls das Target **run-clang-tidy** auswählen.
  * **Kontrolle behalten:** Standard bleibt **ohne Fix**. Fixes nur, wenn du `VIZ_TIDY_FIX=ON` setzt **und** das manuelle Target startest (oder On-Build + Fix explizit aktivierst, was ich nicht empfehle).

**Tipp:** Für manuelles tidy ist `CMAKE_EXPORT_COMPILE_COMMANDS=ON` hilfreich (Ninja/Makefile-Generator). VS setzt intern auch Compile-Daten, aber für das Target `run-clang-tidy` ist die JSON praktisch.

---

# Mini-Cheatsheet (welche Variable wann?)

* **Top-Level CMakeLists / per-Directory:**
  `CMAKE_SOURCE_DIR` → Projekt-Root
  `CMAKE_CURRENT_SOURCE_DIR` → Ordner dieser CMakeLists
* **In Moduldateien (`cmake/*.cmake`):**
  `CMAKE_CURRENT_LIST_DIR` → Ordner der **aktuellen** cmake-Datei
  → robust für `include()`-Kaskaden
* **Immer robust von Modul → Root:**
  `set(_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")`

---

Wenn du willst, gebe ich dir noch eine **konkrete VS-Einstellung** (CMakeSettings.json-Snippet) mit allen Schaltern gesetzt/aus – sag mir, welche Defaults du pro Konfiguration (Debug/Release) bevorzugst.

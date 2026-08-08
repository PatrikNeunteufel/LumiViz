# LumiViz bauen

Der Build braucht **drei Vorbereitungsschritte**, bevor CMake das erste Mal
laufen kann. Sie sind unten einzeln beschrieben. Wer sie überspringt, bekommt
Fehlermeldungen, die auf etwas anderes zu deuten scheinen — deshalb bitte der
Reihe nach.

- [1. Voraussetzungen](#1-voraussetzungen)
- [2. Qt finden lassen](#2-qt-finden-lassen)
- [3. BASS beschaffen](#3-bass-beschaffen-pflicht)
- [4. Konfigurieren und bauen](#4-konfigurieren-und-bauen)
- [5. Tests](#5-tests)
- [6. Wenn etwas schiefgeht](#6-wenn-etwas-schiefgeht)

---

## 1. Voraussetzungen

| Was | Version | Anmerkung |
|---|---|---|
| **CMake** | ≥ 3.25 | wegen der Preset-Version 6 |
| **Git** | beliebig | wird **während** der Konfiguration aufgerufen, um Abhängigkeiten zu holen |
| **C++-Compiler mit C++20** | MSVC 2022, Clang 15+, GCC 12+ | Windows ist die Hauptplattform |
| **Qt** | 6.x (entwickelt gegen 6.10) | Module: `Core Widgets Gui OpenGL OpenGLWidgets Multimedia Network` **und die privaten Gui-Header** (`GuiPrivate`) |
| **BASS + BASSFLAC** | 2.4 | proprietär, **muss selbst beschafft werden** — siehe Schritt 3 |
| **OpenGL** | 3.3 Core | Treiber der Grafikkarte |

Nicht selbst installieren musst du: **CMakeCraft** (das Build-System) und das
**Qt Advanced Docking System** — beide holt CMake beim Konfigurieren selbst per
`git clone`. Dafür braucht die Konfiguration **eine Netzverbindung** beim ersten
Lauf; danach liegen sie im Cache (`.externals/`) und der Build ist offlinefähig.

> **Qt-Hinweis:** LumiViz nutzt private Qt-Gui-Header (`GuiPrivate`). Die sind in
> den offiziellen Qt-Installern enthalten, fehlen aber in manchen
> Distributionspaketen. Unter Linux ggf. `qt6-base-private-dev` o. ä. nachinstallieren.

---

## 2. Qt finden lassen

CMakeCraft sucht Qt über **Umgebungsvariablen**. Setze eine davon auf das
Verzeichnis deiner Qt-Installation (das mit `bin/`, `lib/`, `include/` darin):

`QT_ROOT`, `QT_DIR`, `Qt6_ROOT` oder `Qt6_DIR` — die erste existierende gewinnt.

**Windows (PowerShell, nur für diese Sitzung):**

```powershell
$env:QT_ROOT = "C:/Qt/6.10.1/msvc2022_64"
```

**Linux/macOS:**

```bash
export QT_ROOT=$HOME/Qt/6.10.1/gcc_64
```

Dauerhaft geht es am bequemsten über eine eigene `CMakeUserPresets.json` — eine
Vorlage liegt bei:

```bash
cp CMakeUserPresets.example.json CMakeUserPresets.json
```

Die Datei ist bewusst nicht versioniert; trage dort deine Pfade ein. Sie darf
außerdem eigene Presets definieren (vcpkg-Toolchain, Job-Anzahl, Signieren).

---

## 3. BASS beschaffen (Pflicht)

**BASS ist proprietär und darf über dieses Repository nicht weitergegeben
werden** — weder die Binärdateien noch das SDK. Ohne diesen Schritt bricht der
Build beim Linken ab.

Kurzfassung: Auf <https://www.un4seen.com/> die Pakete **BASS** und **BASSFLAC**
herunterladen und in die unten gezeigte Struktur unter `externals/bass/`
entpacken.

**Die vollständige Anleitung mit allen Pfaden steht in
[`externals/bass/SETUP.md`](externals/bass/SETUP.md).**

Lizenzlage in einem Satz: BASS ist für nicht-kommerzielle Nutzung kostenlos,
kommerzielle Nutzung erfordert eine Lizenz von un4seen — Einzelheiten in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

---

## 4. Konfigurieren und bauen

### Windows — Visual Studio 2022, x64, Debug

```bash
cmake --preset windows-vs-x64-debug_dynamic
```

```bash
cmake --build --preset build-vs-x64-Debug
```

### Windows — Release

```bash
cmake --preset windows-vs-x64-release_dynamic
```

```bash
cmake --build --preset build-vs-x64-Release
```

### Linux — GCC, Debug

```bash
cmake --preset linux-gcc-debug
```

```bash
cmake --build --preset build-linux-gcc-Debug
```

Alle verfügbaren Presets listet:

```bash
cmake --list-presets
```

Es gibt außerdem Ninja-, Ninja-Multi-Config-, clang-cl-, ASan- und
macOS-Xcode-Presets. Hauptplattform ist Windows/MSVC; die übrigen Kombinationen
werden nicht regelmäßig durchgebaut.

Das Ergebnis landet unter `out/`.

---

## 5. Tests

Die Tests brauchen eine Konfiguration mit dem **Testing**-Preset — im normalen
Debug-Build sind die Test-Targets nicht aktiv.

```bash
cmake --preset windows-vs-x64-testing_dynamic
```

```bash
cmake --build --preset build-vs-x64-Testing
```

```bash
ctest --preset ctest-vs-x64-Testing -R UnitTests
```

Erwartet: **593 doctest-Fälle, alle grün, keine übersprungenen.** Die
Test-Executable lässt sich auch direkt starten — die nötigen Qt-, BASS- und
Lua-DLLs werden neben sie kopiert.

Unter Linux entsprechend `linux-gcc-testing` / `build-linux-gcc-Testing` /
`ctest-linux-gcc-Testing`.

---

## 6. Wenn etwas schiefgeht

### `bass.lib … missing and no known rule to make it` (oder ein ähnlicher Linker-Fehler)

Schritt 3 fehlt oder die Dateien liegen am falschen Platz. Der Pfad muss exakt
stimmen — für Windows x64 wird
`externals/bass/bass24/win/c/x64/bass.lib` erwartet, nicht `win/c/bass.lib`
(das ist die 32-Bit-Variante). Siehe [`externals/bass/SETUP.md`](externals/bass/SETUP.md).

### `[Bootstrap] CMakeCraft <version> konnte aus keiner Quelle geholt werden`

Die Konfiguration wollte das Build-System per `git clone` holen und kam nicht
durch. Mögliche Ursachen: keine Netzverbindung, Firewall, oder `git` nicht im
Pfad. Abhilfen nennt die Fehlermeldung selbst; die einfachste ist ein lokaler
Checkout:

```bash
cmake --preset windows-vs-x64-debug_dynamic -DCMAKECRAFT_LOCAL_DIR=../CMakeCraft
```

### Qt wird nicht gefunden

Schritt 2 prüfen. `QT_ROOT` muss auf das Verzeichnis **mit** `bin/` und `lib/`
zeigen (z. B. `C:/Qt/6.10.1/msvc2022_64`), nicht auf `C:/Qt` und nicht auf
`.../bin`. Ob die Variable ankommt, verrät die Konfigurationsausgabe.

### Übersetzungsfehler zu fehlenden privaten Qt-Headern

`GuiPrivate` fehlt in der Installation — siehe den Qt-Hinweis in Abschnitt 1.

### Die Anwendung startet, zeigt aber ein schwarzes Bild

Der OpenGL-Kontext kommt nicht zustande (zu alter Treiber, oder auf einem
Notebook läuft die falsche GPU). LumiViz schreibt beim Start ein Log neben die
Exe; dort steht, welcher Kontext und welche GPU gewählt wurden.

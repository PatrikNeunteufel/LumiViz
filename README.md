# LumiViz

**Audio-Visualizer für Windows und Linux — Qt 6 / OpenGL, C++20.**

LumiViz spielt Musik ab und rechnet dazu Bilder: klassische Visualisierungen
(Equalizer, Oszilloskop, Superscope, Waveform, Pulsing) und ein Knoten-basierter
Multieffekt-Host, der **AVS-** und **MilkDrop**-Presets importiert und
nachrechnet. Dazu kommen Shader-Knoten für **Shadertoy**-GLSL und das
**Interactive Shader Format (ISF)**.

> **Status:** aktiv in Entwicklung, Einzelentwickler-Projekt. Es gibt noch keine
> Release-Binaries — wer LumiViz nutzen will, baut es selbst (siehe
> [BUILDING.md](BUILDING.md)).

---

## Was es kann

**Audio.** Wiedergabe über [BASS](https://www.un4seen.com/) (MP3, FLAC, WAV u. a.),
Playlist mit M3U-Unterstützung, FFT- und Waveform-Analyse als Signalquelle für
alles Visuelle, Beat-Erkennung.

**Visualisierung.** Fünf eigenständige Visualizer plus ein **Multieffekt-Host**,
in dem sich Effekt-Knoten zu Ketten verschalten lassen — Renderer, Transformationen,
Puffer-Rückkopplung, Skript-Knoten. Skripte laufen in EEL-Schreibweise
(nach Lua übersetzt) oder direkt in Lua, in einer abgeschotteten Umgebung.

**Import fremder Formate.**

| Format | Stand |
|---|---|
| **AVS** (Winamp Advanced Visualization Studio) | 44 Builtin-Effekte + 17 APEs; gegen einen mitgelieferten Referenz-Renderer kalibriert |
| **MilkDrop** | Warp-Mesh, Waveform, Composite, Shader-Stufen; gegen MilkDrop3 kalibriert |
| **Shadertoy** (GLSL) | Shader-Knoten mit iChannel-Anbindung an die Audiodaten |
| **ISF** (Interactive Shader Format) | eigener Knotentyp, führt ISF-Dateien unverändert aus — inkl. Vertex-Stufe und Multipass |

**Presets werden nicht mitgeliefert** — LumiViz lädt deine eigenen Sammlungen.
Warum: siehe [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), Abschnitt
„Nicht enthaltene Inhalte“.

**Oberfläche.** Andockbare Panels (Qt Advanced Docking System), frei belegbare
Hotkeys, Vollbildmodus, Screenshot-Ablage, gespeicherte Layouts.

---

## Bauen

Kurzfassung für Windows:

```bash
git clone https://github.com/PatrikNeunteufel/LumiViz.git
```

Danach **BASS beschaffen** (proprietär, darf nicht mitgeliefert werden — Anleitung:
[`externals/bass/SETUP.md`](externals/bass/SETUP.md)), `QT_ROOT` setzen und:

```bash
cmake --preset windows-vs-x64-debug_dynamic
```

```bash
cmake --build --preset build-vs-x64-Debug
```

**Die vollständige Anleitung mit allen Voraussetzungen steht in
[BUILDING.md](BUILDING.md)** — bitte dort anfangen, der Build braucht drei
Vorbereitungsschritte.

---

## Aufbau des Repositorys

| Pfad | Inhalt |
|---|---|
| `projects/apps/MyViz/` | die Anwendung — `audio/`, `visualizers/`, `UI/`, `services/`, `scripting/` |
| `projects/libs/` | wiederverwendbare Header-only-Bibliotheken: `EelTranspiler`, `AvsParser`, `MilkParser`, `HlslTranspiler`, `BasicLogger` |
| `projects/apps/MyViz/docs/` | Architektur- und Anwenderdokumentation, Einstieg: [`docs/INDEX.md`](projects/apps/MyViz/docs/INDEX.md) |
| `tools/AvsRef`, `tools/MilkdropRef` | Referenz-Renderer der Originale, dienen als Vergleichsmaßstab der Kalibrierung |
| `asset/` | eigene Presets, Effektketten, Shader und Kalibrier-Material |
| `externals/` | mitgelieferte Fremdbibliotheken (doctest, glad) |
| `Solution.json` | Beschreibung der Solution: Targets, Abhängigkeiten, Externals |

Das Build-System ist ausgelagert:
**[CMakeCraft](https://github.com/PatrikNeunteufel/CMakeCraft)** — ein
JSON-gesteuerter CMake-Aufbau, der beim Konfigurieren automatisch in der in
`cmakecraft.pin` festgelegten Version geholt wird.

Das Anwendungs-Target heißt aus historischen Gründen noch **MyViz**.

## Tests

```bash
ctest --preset ctest-vs-x64-Testing -R UnitTests
```

593 doctest-Fälle über Service-Fundament, Skript-Übersetzer, Parser, Preset-Persistenz
und OpenGL-Rauchtests. Voraussetzung: Konfiguration mit dem Testing-Preset
(siehe [BUILDING.md](BUILDING.md)).

---

## Lizenz

LumiViz steht wahlweise unter **MIT** ([LICENSE-MIT](LICENSE-MIT)) oder
**Apache-2.0** ([LICENSE-APACHE](LICENSE-APACHE)) — such dir aus, was besser passt.

Das gilt für den LumiViz-eigenen Code. Die Anwendung nutzt und portiert
Fremdkomponenten mit eigenen Bedingungen — darunter das **proprietäre BASS**,
das für nicht-kommerzielle Nutzung kostenlos ist, für kommerzielle Nutzung aber
eine Lizenz von un4seen erfordert. Vollständige Aufstellung:
**[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)**.

Sofern du nicht ausdrücklich etwas anderes erklärst, gilt jeder Beitrag, den du
zur Aufnahme einreichst, als unter denselben beiden Lizenzen stehend.

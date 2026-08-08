# Zu LumiViz beitragen

Danke fürs Interesse! LumiViz ist ein Einzelentwickler-Projekt — Beiträge sind
willkommen, aber lies bitte zuerst die zwei Abschnitte unten. Sie sparen dir
möglicherweise vergebliche Arbeit.

## Bevor du anfängst

**Bauen können ist Voraussetzung.** Der Build braucht Vorbereitungsschritte, die
über ein `git clone` hinausgehen — allen voran die selbst zu beschaffende
Audio-Bibliothek BASS. Anleitung: [BUILDING.md](../BUILDING.md).

**Für alles außer Tippfehlern: erst ein Issue, dann Code.** Große Teile von
LumiViz sind gegen Referenz-Renderer kalibriert; was wie ein Fehler aussieht,
ist manchmal eine bewusst nachgebildete Eigenart des Originals. Ein kurzes Issue
vorab klärt das schneller als ein fertiger Pull Request.

**Lizenz deines Beitrags.** Mit dem Einreichen stellst du deinen Beitrag unter
dieselbe Dual-Lizenz wie LumiViz (MIT **oder** Apache-2.0), sofern du nicht
ausdrücklich etwas anderes erklärst.

**Kein fremdes Material einreichen.** Presets, Shader, Texturen und Logos von
Dritten gehören nicht ins Repository — auch nicht als Testmaterial. Warum, steht
in [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md) unter „Nicht enthaltene
Inhalte“. Übernimmst du fremden Code, muss die Herkunft im Dateikopf stehen und
die Lizenz in `THIRD_PARTY_NOTICES.md` ergänzt werden.

## Womit du am meisten hilfst

- **Preset-Befunde.** Ein AVS- oder MilkDrop-Preset, das anders aussieht als im
  Original, ist der wertvollste Fehlerbericht überhaupt — dafür gibt es eine
  eigene Issue-Vorlage. Bitte Screenshots von beidem beilegen.
- **Plattformen außer Windows.** Linux und macOS werden nicht regelmäßig
  durchgebaut. Baufehler dort sind echte Befunde.
- **Tests.** Neue Testfälle in `projects/apps/MyViz/tests/` sind immer willkommen.

## Konventionen

| Thema | Regel |
|---|---|
| **Sprache** | C++20 |
| **Formatierung** | `clang-format` mit der Konfiguration im Repo-Root — vor dem Commit laufen lassen |
| **Statische Prüfung** | `clang-tidy`, Konfiguration ebenfalls im Root |
| **Header** | `#pragma once` |
| **Membervariablen** | Präfix `m_` |
| **Aufzählungen** | `enum class` |
| **Protokollierung** | `BasicLogger` — kein `std::cout`, kein `printf` |
| **Neue Dateien** | in die zuständige `Source.cmake` eintragen — die Quelldatei-Listen sind **explizit**, es wird bewusst nicht geglobbt |
| **Modul-Doku** | `*.md` neben dem Header, Format CppModuleDoc; keine versionierten Kopien (`Name100.md`) — Git ist die Historie |

Die Dokumentation ist auf Deutsch, Bezeichner im Code auf Englisch. Halte dich
an das, was in der Umgebung deiner Änderung schon steht.

## Pull Requests

Vor dem Absenden:

1. Debug **und** Testing bauen ohne Fehler
2. `ctest --preset ctest-vs-x64-Testing -R UnitTests` ist grün, ohne
   übersprungene Fälle
3. `clang-format` gelaufen
4. Neues Verhalten hat einen Test
5. Keine absoluten Pfade, keine lokalen Eigenheiten im Code

Änderungen am Rendern brauchen zusätzlich einen **Sichttest**: Screenshot
vorher/nachher, und bei Import-Effekten den Vergleich gegen den Referenz-Renderer
(`tools/AvsRef` bzw. `tools/MilkdropRef`). Wie das methodisch geht, steht in
`projects/apps/MyViz/docs/visuals/AVS_Kalibrier_Methodik.md`.

## Verhalten

Es gilt der [Verhaltenskodex](CODE_OF_CONDUCT.md).

## Fragen?

[Issue eröffnen](https://github.com/PatrikNeunteufel/LumiViz/issues) — dafür gibt
es eine eigene Vorlage „Frage“.

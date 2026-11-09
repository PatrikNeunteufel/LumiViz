# rechnung/ — die Rechenkerne der Instrumentklassen

> **Auftrag:** [AUFTRAG_BAU.md](../AUFTRAG_BAU.md) (Cloud-Sitzungen ab 2026-10-29). **Vorgaben für Offenes:**
> [FRAGEN_KONSOLIDIERT.md](../FRAGEN_KONSOLIDIERT.md) (alle Bündel von Patrik beantwortet, 2026-10-29; Stellen im Code
> in §3 dort). **Vorbild:** [gemeinsam/vorbild-lumiviz/](../gemeinsam/vorbild-lumiviz/README.md) (Kopie, nicht ändern).

Reines C++20, ohne Qt, ohne `windows.h`; Tests mit doctest, ohne Anwendung. Baubar unter Linux (gcc, clang) und
Windows (MSVC, clang).

```
cmake -S rechnung -B rechnung/out -DCMAKE_BUILD_TYPE=Release
cmake --build rechnung/out -j
ctest --test-dir rechnung/out --output-on-failure
```

## Aufbau

| Ordner | Inhalt |
|---|---|
| `gemeinsam/` | Bausteine aller Klassen: `Membran` (Fellmoden mit Luftlast als Rayleigh-Integral und Kessel), `Rohrschleife` (Klasse 7), `Kurve` (Kurven über den Tonumfang mit Herkunft, wie `Tastenkurve`), `Eigenzerlegung` (kleiner komplexer Eigenlöser aus dem Vorbild, meldet Entartung — L11), `Modal` (Moden als komplexe Pole, Impulsinvarianz, Stoß nach Stulov/Hertz/Hunt–Crossley), `Balken` (Moden eines Balkens aus finiten Elementen, symmetrischer Eigenlöser), `Koerper` (Körper aus Modenlisten, Mobilität, Antwort; **Luftraum** nach T4 a), `Rohr` (Eingangsimpedanz aus der Bohrung, Moden der Impedanz, Rohr-Kern im Zeitbereich nach T5 a), `Fft` (pocketfft hinter eigener Schnittstelle), `Json`, `Werkzeug` (WAV, Pegel/LUFS-Näherung, SHA-256-Abdruck, Rechenzeit, Pool) |
| `klassen/<nn_name>/` | je Klasse Kern, Instrumente (`baueVorgabe…()`), Tests |
| `messung/` | die Messkette (AUFTRAG_BAU §5): portiert aus dem Vorbild (`Teiltonmessung`, `Fluegelmessung`, `Strukturmessung`, `Zielsumme`, `LevenbergMarquardt`, `NelderMead`, `Kurvenabstand`), die Bausteine B1–B8 (`Bausteine`), Zugang zu Aufnahmen nur über `IC_SAMPLES` (`Samples`) |
| `setzen/` | das Setz-Werkzeug (AUFTRAG_BAU §6): `setze` (LM mit Grenzen je Gruppe nach L8, Wirkung vorab L9, verrauschte Spalten L11, wellige Größen zuerst 1D L12 — nicht, wenn ein zweiter freier Parameter dieselbe Wahrnehmung trägt (L16), Glätte des Ziels `welligkeit` (L16), Ergebnis als Vorschlag (L18), Protokoll je Iteration mit Wiederaufnahme L7, Hörschwelle getrennt L13), `schnitt` (L14); geprüft an synthetischen Aufgaben (`test_setzen.cpp`) — der Lauf an Aufnahmen bleibt lokal |
| `werkzeuge/` | Testtreiber, Hilfen der Tests, `ic_hoerproben` (Hörproben als WAV), `ic_messe` (nur lokal: einen Datensatz unter `IC_SAMPLES` messen) |
| `werte/` | Bezugs- und Abdruckdateien je Compiler (`abdruck_gcc.txt`, `abdruck_clang.txt`, …) |
| `externals/` | doctest (MIT), pocketfft (BSD-3) — je mit Lizenzdatei |

## Regeln

- **Feste Rechenregeln** (FRAGEN_KONSOLIDIERT E1 a): kein fast-math, keine automatische FMA (`-ffp-contract=off`,
  MSVC `/fp:precise /fp:contract-`), feste Reihenfolge; derselbe Build liefert denselben Ton. Abdrücke je Compiler in
  `werte/`; neu schreiben mit `IC_ABDRUCK_SCHREIBEN=1` nach einer gewollten Änderung am Klang. Bei chaotischen Modellen
  (Klasse 6) prüfen die Tests zusätzlich Statistiken.
- **Kein Pfad zu Aufnahmen im Code** (AUFTRAG_BAU §0): Werkzeuge, die lokal an Aufnahmen messen oder setzen, lesen die
  Wurzel aus `IC_SAMPLES` (oder einem Argument) und brechen mit Meldung ab, wenn sie fehlt; Pfade darunter aus der
  `LADELISTE.tsv` des Datensatzes, Prüfung über `SHA256SUMS`.
- **Keine Zahl ohne Herkunft:** jede Kurve trägt `belegkraft` und `herkunft` (Quelle mit Seite, Rechnung, oder
  „offen“ mit Vorgabe und Grund); je Klasse `modell.md` und `offen.md` unter `klassen/<Klasse n - Name>/`.
- **Rechenzeit:** die Grenze ist rund 5 % der Echtzeit im tiefsten Ton (Flügel in LumiViz 4,6 %); weil die Zahl an der
  Maschine hängt, misst jeder Test relativ zum portierten Flügel-Kern auf derselben Maschine
  (`werkzeuge/TestHilfe.hpp`, `echtzeitGrenzeHier`). Darüber: Marke **Deluxe**. Ausnahme Klasse 6 (von Kármán, F-1 a:
  vorab je Stärke): Grenze 10 × Echtzeit für die Rechnung vorab.
- **Erzeugte Stimmungen:** wo die Stimmung durch das Modell lange dauert (Klassen 3, 4 — gestrichenes Vibraphon —, 7, 8, 9, 10), liegt sie als
  `Stimmung.inc` vor, erzeugt von `ic_stimme_<klasse>`; nach jeder Änderung am Modell neu erzeugen (sonst stimmen die
  Abdrücke, aber die Noten nicht mehr).
- **Komplexe Zahlen in heißen Schleifen** ausgeschrieben: clang ruft für `std::complex`-Multiplikation ohne fast-math die
  NaN-sichere Bibliothek (vierfach langsamer, Befund Klasse 5).
- **Determinismus:** Zufall nur mit festem Startwert; der Pool (`parallelJe`) nur über Aufgaben, die für sich rechnen.

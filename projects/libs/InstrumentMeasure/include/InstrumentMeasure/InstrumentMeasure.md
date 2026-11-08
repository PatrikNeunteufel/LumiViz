# InstrumentMeasure — Messkette und Setzen

> **Version:** 0.2.0
> **Datum:** 2026-11-08
> **Typ:** CppModuleDoc
> **Status:** gebaut (Stufe 34e)
> **Modul:** InstrumentMeasure (STATIC, `projects/libs/InstrumentMeasure`)
> **Namespace:** `lumi::messung`, das Setzen in `lumi::messung::setzen`
> **Abhängigkeiten:** InstrumentCore, AudioCore (PUBLIC), Standardbibliothek; kein Qt, kein `windows.h`
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Zweck

Messen am Ausgang eines Tons und Setzen der Werte eines Kerns gegen gemessene Ziele. **IK2-2 a** (Patrik S173,
`docs/audio/Instrumente_IC_Konzept.md` §5): je Baustein die reichere Fassung als die eine Quelle — die Messkette ist der
LumiViz-Stand (aus den Unit-Tests gezogen), dazu die Bausteine B1–B8 und `setzen` aus IC; Richtung App → Messung →
Kern. Der Name nach **KN1 a**. In 34e hängt die Bibliothek nur an `InstrumentCore` und `AudioCore` (ME1 a), nicht an
`InstrumentModels`: was ein bestimmtes Instrument misst (die Flügel-Suche), bleibt beim Aufrufer und liefert dem
Setzen sein `Modell` (ME3 a). Jede Übernahme steht im
[Übernahme-Journal](../../../../apps/LumiViz/docs/modellierung/anbindung-ic/Uebernahme_Journal.md) (J19 ff.).

## 2. Bau

- `Solution.json`: STATIC, `compile_options` nach E1 a (IK2-5 a) wie die Schwestern — MSVC `/fp:precise`, clang/gcc
  `-ffp-contract=off -fno-fast-math`, **PRIVATE**. Deshalb wird in den `.cpp`-Dateien gerechnet; in den Köpfen stehen
  nur Deklarationen, Vorgabewerte, kleine Zugriffe und Konstanten (34e-1: 98 Funktionen und zwei Klassen aus den
  Köpfen der Tests in die `.cpp` gezogen, die Rümpfe wortgleich).
- `src/Source.cmake` nennt Quellen **und** Köpfe (sonst fehlen sie im Paket).
- Paket `package_instrumentmeasure` → `out/package/instrumentmeasure-v<ver>.zip`: verlangt die Ziele `InstrumentCore`
  und `AudioCore` (deren Pakete zuerst einbinden).

## 3. Schnittstellen

| Kopf | Inhalt |
|---|---|
| `Teiltonmessung.hpp` | ein Ton ohne Qt: Teiltöne, Inharmonie B, Abklingen (zwei Stufen, Schroeder), Schwebung, Hüllkurve vorwärts, Terzpegel, Phantome; `miss` gegen `lumi::kern::Fft` |
| `Fluegelmessung.hpp` | die Aufnahme *Salamander* (Tasten, Ebenen, Dateien), Messung je Ton und Kanal, Verdichten, die Dateien `salamander_*.txt`, Ziele und Toleranzen (ZF4 a) |
| `Strukturmessung.hpp` | ein Teilton an der Struktur des Ziels (zwei Geraden mit Knick, Schwebung bei der Frequenz der Struktur) |
| `Kurvenabstand.hpp` | Abstand zweier Abklingkurven und ihrer Schwebung (RV1 a) |
| `Zielsumme.hpp` | Vergleich je Größe mit Untergrenze der Toleranz (UG1 a), Residuen, Summe je Taste, Berichte |
| `Hoerschwellen.hpp` | die Hörschwellen je Größe aus `werte/hoerschwellen.txt` (HW1 a), hörbar getroffen (HW2 a) |
| `Probe.hpp` | Berichte lesen, eindeutig über Startpunkte, Vorzeichentest mit Holm, Ausreißer (HW2 a–HW4 a; bis S176 `Probe7512`) |
| `LevenbergMarquardt.hpp` | LM mit Grenzen, einfach und je Block (7.5.10, 7.5.12 A2/A3); Endregel nach ME4 a: auch der letzte kleine Schritt wird ausgewertet und genommen, wenn er senkt |
| `NelderMead.hpp` | Nelder–Mead (Werkzeug) |
| `Bausteine.hpp` | aus IC: B1 Teiltonverfolgung, B2 Tonhöhe (YIN), B3 Vibrato, B4 Rest, B5 Einschwingen, B6 Modalfit (ESPRIT), B7 Bänder über der Zeit, B8 Gleiten; `amplitudenBei` (Messung an der Struktur des Bezugs, L4) |
| `Samples.hpp` | aus IC, allgemein: die Wurzel lokaler Aufnahmen aus einer Umgebungsvariable (Name vom Aufrufer), `SHA256SUMS` lesen, Dateien prüfen |
| `Stimmen.hpp` | 34f-3 (IK2-10 a, IK2-11 a): `Stimmer` je Kern, `stimmer()` (die eingebauten; heute leer, die Klassen bringen ihre mit), `stimmeUndNormiere` (die veralteten gestimmten Werte neu, Abdrücke merken, Bezug über `normiereNachTafel`), `veralteteWerte` (ohne Rechnen), `bezugVeraltet`; das Programm `InstrumentTuneTool` schreibt die Datei |
| `Setzen.hpp` | aus IC: `setzen::setze` (Gruppen, Messgrößen, `Modell`; L7–L18), `schnitt`, `welligkeit`, `glaette`, `setzeMehrfach`, `eindeutig`, `probe`, `alsTasten`; innen das LM je Block; Schritt und Ende je Parameter, Beobachter je Iteration und Startzustand (ME3 a) |

### 3.1 Das Setzen

Gesucht wird im normierten Raum u ∈ [0, 1] je Parameter (linear oder logarithmisch). Eine Gruppe (eine Note, eine
Taste) ist ein Block des LM: eigenes λ, eigene Annahme, eigenes Ende; alle Auswertungen einer Runde laufen auf einem
Pool (`kern::parallelJe`). Das `Modell` des Aufrufers rechnet deshalb **ohne eigenen Pool** — `parallelJe` verschachtelt
sich nicht. Vor dem Lauf: L9 (wirkungslose Parameter fest), L11 (verrauschte Spalten melden), L12 (wellige zuerst
eindimensional, außer zwei Träger derselben Wahrnehmung, L16); im Lauf A3 (kein Schritt ohne Wirkung, je Iteration neu).
Das Ergebnis ist ein Vorschlag (L18).

## 4. Tests

- `test_Teiltonmessung.cpp`, `test_Kurvenabstand.cpp`, die Salamander-Fälle (`test_LoopSalamander*.cpp`) — die
  Messkette; **Wächter:** `LUMIVIZ_SALAMANDER=<Ordner>` misst die Aufnahme neu, `salamander_*.txt` ohne Diff.
- `test_InstrumentMeasure.cpp` — `*InstrumentMeasure 34e*`: die Bausteine B1–B8, `amplitudenBei`, Zielformat, Samples
  (Fälle aus IC `test_messung.cpp`, mit deren Grenzen; „Modalfit am Modell“ kommt mit 34g).
- `test_InstrumentMeasureSetzen.cpp` — `*InstrumentMeasure 34e Setzen*`: die zehn Fälle aus IC `test_setzen.cpp`.
- `test_InstrumentStimmen.cpp` — `*Instrument Stimmen*`: am Prüfkern `testTuned` Stimmen, Abdruck, Bezug, rot bei veralteter
  Stimmung und verstelltem Bezug; der Wächter über die mitgelieferten `.lvinst` (heute ohne Fall).
- `test_LoopSalamanderSuche.cpp` — LM einfach und je Block (7.5.10, 7.5.12); die Flügel-Suche über `setzen`
  (`LUMIVIZ_75_SUCHE2`, mit `…_ITERATIONEN` für eine kurze Probe).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.2.0 | 2026-11-08 | Session 177. 34f-3: `Stimmen` (IK2-10 a, IK2-11 a). |
| 0.1.0 | 2026-11-08 | Session 177. Angelegt mit 34e (KN1 a, IK2-2 a, ME1 a–ME4 a). |

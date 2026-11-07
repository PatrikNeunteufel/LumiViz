# InstrumentCore — die Bibliothek der Rechenkerne

> **Version:** 0.1.1
> **Datum:** 2026-11-07
> **Typ:** CppModuleDoc
> **Status:** gebaut (Stufe 34c); die Kerne in `InstrumentModels` (34d, BK1 a)
> **Modul:** InstrumentCore (STATIC, `projects/libs/InstrumentCore`)
> **Namespace:** `lumi::kern`
> **Abhängigkeiten:** Standardbibliothek, pocketfft (`externals/pocketfft`, PRIVATE); kein Qt, kein `windows.h`
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Zweck

Die Rechenkerne der Instrumente rechnen hier, ohne Qt, hinter **einer** Ton-Schnittstelle
(`docs/audio/Instrumente_IC_Konzept.md` IK2-0 a, IK2-3 a, KN1 a). LumiViz nutzt die Bibliothek im Prozess und vorab
(Tonspeicher, Freeze); InstrumentsCalculation (IC) bezieht sie als Quellpaket (`package_instrumentcore`, TP1 a).
Die Bausteine kommen je aus der reicheren Fassung von IC oder LumiViz; jede Übernahme steht im
[Übernahme-Journal](../../../../apps/LumiViz/docs/modellierung/anbindung-ic/Uebernahme_Journal.md).

## 2. Bau

- `Solution.json`: STATIC, `compile_options` nach E1 a (IK2-5 a) — MSVC `/fp:precise` (ohne `/fp:contract`: keine
  automatische FMA; `/fp:contract-` gibt es nicht, Journal J0), clang/gcc
  `-ffp-contract=off -fno-fast-math`, **PRIVATE**. Deshalb wird in den `.cpp`-Dateien gerechnet, nicht in den Köpfen:
  was ein Kopf rechnet, übersetzt der Bezieher mit seinen Schaltern.
- `src/Source.cmake` nennt jede Quelle und jeden Kopf (private Köpfe gehören dorthin, sonst fehlen sie im Paket).
- `projects/libs/.gitattributes` und `externals/pocketfft/.gitattributes`: LF im Arbeitsbaum, damit `SHA256SUMS` des
  Pakets auf jedem Rechner gleich sind (LV-4).

## 3. Schnittstellen

| Kopf | Inhalt |
|---|---|
| `Tonkern.hpp` | `Tonkern` (`nachklang`, `rechne` → Mono `float`, auf Wunsch innere Größen), `KernTyp` (Tafel, Auskunft, `erzeuge`), `KernAuskunft` (Rechenweg Note/Kette/Clip, Stufe im Namen, Spuren, innere Größen, Vorbereitung teuer), `Fabrik` (je Name; `erzeuge` richtet die Werte und baut; die eingebauten Kerne kennt `kerne()` in `InstrumentModels`, BK1 a), `normiere` (Bezug des Ausgangs, IK2-10 a) |
| `Tonauftrag.hpp` | `Frames`, `KettenNote {beginn, ende, note, hz, gleitet}`, `Spur` mit `SpurPunkt`, feste Namen `spur::kExpression`/`kPitch`/`kVibrato`, `Tonauftrag {rate, noten, stufe, frames, spuren}`, `spurWert`, `einzelnote`, `pruefeAuftrag` |
| `Parametertafel.hpp` | `Wert` (Zahl, Ganzzahl, Schalter, Wahl, Kurve, Liste), `Werte`, `ParameterAngabe` (Gruppe, Art, Darstellung, Einheit, Grenzen, Skala, Wahlen, Felder einer Liste, Vorgabe, Belegkraft, Herkunft, klangbestimmend, gestimmt mit Abhängigkeiten), `Parametertafel` (Kern, Titel, Fassung der Rechnung, Tastenumfang, Gruppen, Werte); `pruefeTafel`, `vorgaben`, `richteWerte`, `eingabenAbdruck`, `veralteteGestimmte`, `ungemessen`, `wertAlsText`, `belegkraftName` |
| `Anschlag.hpp` | `anschlagStufe`, `anschlagFaktor`, `anschlagRest`, `kAnschlagStufeMin` (−40), `kAnschlagSchrittDb` (1) |
| `Kurve.hpp` | `Kurve` über den Tonumfang (Stützpunkte, Abweichungen, Belegkraft, Herkunft), `wertAn` (monoton kubisch; bis 34d Bit für Bit gegen `Tastenkurve` der App geprüft, seither rechnet der Flügel selbst damit), `richteKurve` mit Umfang, `kurveAusPunkten`, `kurveKonstant`, `kurveAlsText` |
| `Modal.hpp` | Moden als komplexe Pole, `ModalBank` mit Impulsinvarianz, `impulsantwort`, `nachklangFrames`, gekoppeltes Paar, `Stossanreger`/`stosse` |
| `Eigenzerlegung.hpp` | `KleineMatrix` bis 32 × 32, `zerlegeKlein` (Hessenberg, QR nach Wilkinson, Nachschärfen, links als Inverse), `kleinsterAbstand`, `entartet` |
| `Fft.hpp` | `Fft` (`vorwaerts`, `rueckwaerts`), `fftUmsetzungen()`, `fft()` (pocketfft `4d865cb`), `direkteDft()` (Prüfumsetzung, O(n²)), `zweierpotenzAb`, `falte` |
| `Pool.hpp` | `parallelJe` (höchstens 11 Threads, Ausnahme beim Rufer), `poolThreads` |
| `Pruefsumme.hpp` | `Sha256`, `sha256`, `abdruck` (floats Little-Endian) |

### 3.1 Eigenzerlegung (aus der Modul-Doku der App, S153)

Die Moden der Hammer-Saite je Teilton (Stufe 31d-7, P2 a, P6 a): N ≤ 3 Saiten mit je zwei Schwingungsebenen geben eine
allgemein komplexe Matrix bis 6 × 6. A = rechts · diag(λ) · links; für eine Antwort c·e^(At)·b zählt je Mode
(c·v_m)(w_m·b). Verfahren: eine schon diagonale Matrix bleibt Bit für Bit (darauf baut die Zusage, dass eine Saite mit
γ = 0 wie vorher rechnet); sonst Householder auf Hessenberg-Form, QR mit Verschiebung nach Wilkinson (alle zehn Schritte
eine Ausnahmeverschiebung), Eigenvektoren rückwärts eingesetzt, isolierte (Abstand über 10⁻⁸·‖A‖) mit einem Schritt
inverser Iteration nachgeschärft, `links` als Inverse von `rechts` mit Pivotsuche. Neu aus IC: liegt ein Eigenwert
näher, meldet die Zerlegung `entartet` (L11) — die Vektoren des Paars sind dann beliebig.

## 4. Tests

- `tests/unit/UnitTests/test_InstrumentCore.cpp` — `*InstrumentCore*`: Fft gegen die direkte DFT, Eigenzerlegung
  (Entartung, 20 × 20), Anschlagstufe, SHA-256 (FIPS 180-2 B.1/B.2), Pool, Kurve (Grenzen, Ersatz),
  Modal gegen die geschlossene Impulsantwort, Tonauftrag, Parametertafel (Prüfung, Richten, gestimmte Werte,
  ungemessen), Fabrik und `normiere` an einem Prüfkern.
- `test_LoopFft.cpp` (jede Umsetzung aus `fftUmsetzungen()`), `test_LoopKleinEigen.cpp` (gegen Eigen).
- Wächter der Bits: `*Loop Werte*`, `*Klangfassung*`, Flügel-Bezug in `test_LoopHammerTon`.

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.1 | 2026-11-07 | Session 176. Nach 34d: `kerne()` in `InstrumentModels` (BK1 a); MSVC-Schalter ohne `/fp:contract-` (J0); der Test gegen `Tastenkurve` entfällt (der Typ ist weg, die Gleichheit belegt der Flügel-Bezug). |
| 0.1.0 | 2026-11-07 | Session 175. Angelegt (34c, KN1 a, KN2 a); §3.1 aus `include/loop/Eigenzerlegung.md` der App (0.1.0, S153). |

# InstrumentModels — die Rechenkerne der Instrumentklassen

> **Version:** 0.1.0
> **Datum:** 2026-11-07
> **Typ:** CppModuleDoc
> **Status:** gebaut (Stufe 34d: der Flügel)
> **Modul:** InstrumentModels (STATIC, `projects/libs/InstrumentModels`)
> **Namespace:** `lumi::kern` (Fabrik), je Klasse `lumi::kern::<klasse>` (`lumi::kern::hammer`)
> **Abhängigkeiten:** InstrumentCore (PUBLIC), Standardbibliothek; kein Qt, kein `windows.h`
> **Zielgruppe:** Entwickler
> **Sprache:** Deutsch

---

## 1. Zweck

Die Kerne der Instrumentklassen über der einen Ton-Schnittstelle von `InstrumentCore` (`Tonkern`, `KernTyp`, `Fabrik`,
Parametertafel). **BK1 a** (Patrik S175, `docs/audio/Instrumente_IC_Konzept.md` §6): zwei Bibliotheken für das
Rechnen — `InstrumentCore` kennt keinen Kern, `InstrumentModels` trägt alle; Richtung App → (Messung →) Models → Core.
IC baut neue Kerne gegen das Paket von `InstrumentCore` allein (TP1 a) und bekommt die fertigen als Paket
`instrumentmodels`.

Je Klasse ein Unterordner mit eigenem Namensraum und eigener `Source.cmake`; geteilte Bausteine der Kerne bleiben in
dieser einen Bibliothek. Jede Übernahme steht im
[Übernahme-Journal](../../../../apps/LumiViz/docs/modellierung/anbindung-ic/Uebernahme_Journal.md) (J12 ff.).

## 2. Bau

- `Solution.json`: STATIC, `compile_options` nach E1 a (IK2-5 a) wie `InstrumentCore` — MSVC `/fp:precise`,
  clang/gcc `-ffp-contract=off -fno-fast-math`, **PRIVATE**. Gerechnet wird in den `.cpp`-Dateien.
- `src/Source.cmake` nennt `Kerne.cpp` und bindet je Klasse `src/<klasse>/Source.cmake` ein; jede nennt ihre Quellen
  **und** Köpfe (sonst fehlen sie im Paket).
- Paket `package_instrumentmodels` → `out/package/instrumentmodels-v<ver>.zip`: das erzeugte `CMakeLists.txt` bricht ab,
  wenn das Ziel `InstrumentCore` fehlt (dessen Paket zuerst einbinden), und linkt es PUBLIC.

## 3. Schnittstellen

| Kopf | Inhalt |
|---|---|
| `Kerne.hpp` | `kerne()`: die Fabrik der eingebauten Kerne — heute `hammerString` (34d); die Klassen aus IC folgen ab 34g |
| `hammer/HammerSaite.hpp` | das Spiel des Flügels: `HammerSaite` (22 Kurven über A0–C8), `kurvenAngaben()` (die eine Liste: Feld, Name in `coreParams`, Titel, Gruppe, Einheit, Skala, Grenzen), `richteHammerSaite`, `hammerSaiteVorgabe` (jede Kurve mit Belegkraft und Herkunft), `chorAn`, die Formeln nach RT-0425, Stulov & Kartofelev 2009 und Rigaud |
| `hammer/HammerTon.hpp` | der Ton: `HammerAnschlag` (Taste, Hz, Stufe, Halten, Dämpfer, Rate, `tempoFaktor`, Moderator), `rechneHammerTon`, `hammerNachklang`, `hammerTempo`, Moden des Chors (`chorModen`, `ebenenModen`, `stossJeSaite`, `hammerTeiltonModen`), `kHammerKraftBezug`, `kSchalldruckBezug` |
| `hammer/HammerKern.hpp` | der Kern: `kKernName` `"hammerString"`, `kernTyp()` (Tafel, Auskunft, `erzeuge`), `Pedalwerte` (Gruppe *Pedals*, FU2 a), `werteAus`/`saiteAus`/`pedalwerteAus` (Spiel ↔ Werte), `anschlagAus`, `HammerKern` |

### 3.1 Der Flügel — Kern `hammerString`

- **Tafel** (IK2-4 a, §6a): Gruppen *String*, *Choir*, *Decay*, *Hammer*, *Damper*, *Pedals*; die 22 Kurven aus
  `kurvenAngaben()`, dazu `undampedFrom` (Ganzzahl, 89), `softFactor` (1), `moderatorStiffness` (2),
  `moderatorExponent` (2). Tastenumfang 21–108, `rechenFassung` 1. Der Körper steht **nicht** in der Tafel, sondern
  an der Schicht (FU1 a) — die App faltet ihn.
- **Auskunft:** Rechenweg *Note*, Stufe im Namen (`:a<stufe>`), Spuren `soft` und `practice` (Stufe ≥ 0,5 am Beginn der
  Note), innere Größe `contact` (Kontaktzeit, Kraftspitze); die Vorbereitung ist billig.
- **Rechnen:** `HammerKern::rechne` rechnet je Auftrag die erste Note mit `rechneHammerTon`, in Einheiten von
  `kHammerKraftBezug` Newton am Steg (Mono). Mit den Vorgaben (`tempoFaktor` 1, Moderator 0) Bit für Bit wie die
  Hammer-Saite der App bis S175 (Flügel-Bezug `hammer_bezug_windows.txt`, alle Töne).
- **Belegkraft:** jede Kurve trägt die ihre; besteht eine aus gemessenen und belegten Teilen (B: gemessen bis E2,
  darüber Rigaud), gilt die schwächere. Ungemessen bleibt der Flügel, solange ein klangbestimmender Wert nicht
  *Gemessen* ist (`kern::ungemessen`) — heute die Pedale und die vorläufigen Kurven.

## 4. Tests

- `tests/unit/UnitTests/test_InstrumentModels.cpp` — `*InstrumentModels*`: `kerne()` kennt den Flügel (Tafel), Werte ↔
  Spiel, der Adapter rechnet wie `rechneHammerTon`, Soft und Practice über Spuren.
- `test_LoopHammerSaite.cpp` (Kurven, Vorgaben, Datei Fassung 21), `test_LoopHammerTon.cpp` (am gerechneten Ton
  gemessen; Bezug je Plattform Bit für Bit, Rechenzeit `*31b-3*`), `test_LoopKleinEigen.cpp`.
- Wächter der Bits: `*Loop Werte*`, `*Klangfassung*`, Flügel-Bezug; im Creator `test_LoopInstrumentEditor`
  „Instrument-Creator 34d“ (Ansicht der Tafel, Save as Bit für Bit).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-11-07 | Session 176. Angelegt mit 34d (BK1 a, FU1 a, FU2 a): der Flügel als Kern `hammerString`. |

# InstrumentModels — die Rechenkerne der Instrumentklassen

> **Version:** 0.2.0
> **Datum:** 2026-11-08
> **Typ:** CppModuleDoc
> **Status:** gebaut (Stufe 34d: der Flügel; 34g: Klasse 4, der Kern `bar`)
> **Modul:** InstrumentModels (STATIC, `projects/libs/InstrumentModels`)
> **Namespace:** `lumi::kern` (Fabrik, geteilte Bausteine), je Klasse `lumi::kern::<klasse>` (`lumi::kern::hammer`, `lumi::kern::stab`)
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
| `Kerne.hpp` | `kerne()`: die Fabrik der eingebauten Kerne — `hammerString` (34d), `bar` (34g); die übrigen Klassen aus IC folgen |
| `gemeinsam/Balken.hpp` | 34g, aus IC: Biegemoden eines Balkens aus finiten Elementen (Euler–Bernoulli, Lager frei, gelenkig, eingespannt), `formAn`, `freiFreiVerhaeltnisse`, `kragBetaL`; der Löser `zerlegeSymmetrisch` aus dem Core |
| `gemeinsam/Reibung.hpp` | 34g, aus IC: die Reibung Bogen–Schwinger nach Galluzzo mit Haften und Hysterese (der Bogen am Stab, später Klasse 3) |
| `stab/Stab.hpp` | 34g, aus IC: das Spiel `StabSpiel` (Kurven über den Umfang des Instruments), `StabAnschlag`, `StabMessung` (mit dem gehaltenen Teil für den Stimmer), `stabModen`, `stabNachklang`, `rechneStabTon`, `richteStabSpiel`, die zehn Vorgaben (`baueVorgabe…`, `alleStabSpiele`) |
| `stab/StabKern.hpp` | der Kern: `kKernName` `"bar"`, `kernTyp()` (Tafel mit dem Umfang je Instrument, Auskunft, `erzeuge`), die Spuren `motor`, `bowed`, `malletOffset`, `damperPressure`, die gestimmten Werte `partnerGain` und `bowForce*`, `werteAus`/`spielAus`, `anschlagAus`, `vorlagen()` (die zehn mit Namen nach KS4 a), `StabKern` |
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

### 3.2 Die Stabspiele — Kern `bar` (34g)

- **Ein Kern, viele Instrumente** (KS1 a): die Tafel trägt den möglichen Umfang 21–108, jedes Instrument seinen eigenen
  in `noteMin`/`noteMax` (Gruppe *Keys*); Kurven, Probe des Bezugs und Ansicht folgen ihm. Gruppen *Keys*, *Bar*,
  *Partner*, *Mallet*, *Damper*, *Motor*, *Pickup*, *Bow*, *Output*; 57 Werte; Grenzen aus IC `richteStabSpiel`.
- **Gestimmt** (KS2 a, KS3 a): `partnerGain` (Ziel `partnerGainDb` an `partnerGainNote`), danach die sechs Fenster des
  Bogens; der Stimmer steht in `InstrumentMeasure/StabStimmen`. Bezug `outputReference` (IK2-10 a).
- **Auskunft:** Rechenweg *Note*, Stufe im Namen, Spuren `expression`, `motor`, `bowed`, `malletOffset`, `damperPressure`
  (im Loop mit 31g), innere Größe `contact`; der Körper liegt im Kern (`koerperAnDerSchicht` aus — der Creator zeigt
  keinen *Body*).
- **Adapter:** die erste Note; der Dämpfer am Ende der Note; ohne Spur `motor` kein Motor (Phase 0); gestrichen ab
  `bowedFrom` (63). Die Rechnung Bit für Bit wie IC (die elf Abdrücke aus IC `werte/abdruck_msvc.txt`, Windows).
- Übersicht der Instrumente, Rechenzeit und Kennzahl: `apps/LumiViz/docs/modellierung/platte-stab/README.md`.

## 4. Tests

- `tests/unit/UnitTests/test_InstrumentModels.cpp` — `*InstrumentModels*`: `kerne()` kennt den Flügel (Tafel), Werte ↔
  Spiel, der Adapter rechnet wie `rechneHammerTon`, Soft und Practice über Spuren.
- `test_InstrumentModelsStab.cpp` — `*InstrumentModels 34g*`: Balken und Reibung, die Tafel `bar`, gestimmt und
  normiert Bit für Bit wie IC, die Fälle aus IC `test_stab.cpp`, der Adapter, die Reihenfolge der gestimmten Werte, die
  Rechenzeit (meldet Zahlen); das volle Stimmen der Bogenfenster am Modell mit `LUMIVIZ_34G_STIMMEN=1` (rund 150 s).
- `test_LoopStab.cpp` — die mitgelieferten Dateien (schreiben mit `LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1`, Wächter,
  Hörproben `LUMIVIZ_34G_WAV`).
- `test_LoopHammerSaite.cpp` (Kurven, Vorgaben, Datei Fassung 21), `test_LoopHammerTon.cpp` (am gerechneten Ton
  gemessen; Bezug je Plattform Bit für Bit, Rechenzeit `*31b-3*`), `test_LoopKleinEigen.cpp`.
- Wächter der Bits: `*Loop Werte*`, `*Klangfassung*`, Flügel-Bezug; im Creator `test_LoopInstrumentEditor`
  „Instrument-Creator 34d“ (Ansicht der Tafel, Save as Bit für Bit).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.2.0 | 2026-11-08 | Session 178. 34g (KS1 a–KS4 a): Klasse 4 als Kern `bar` (Stab, Vorgaben, Adapter), geteilte Bausteine `gemeinsam/Balken` und `gemeinsam/Reibung`; der Flügel meldet `koerperAnDerSchicht` (Journal J31–J40). |
| 0.1.0 | 2026-11-07 | Session 176. Angelegt mit 34d (BK1 a, FU1 a, FU2 a): der Flügel als Kern `hammerString`. |

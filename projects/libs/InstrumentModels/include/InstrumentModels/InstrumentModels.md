# InstrumentModels — die Rechenkerne der Instrumentklassen

> **Version:** 0.7.0
> **Datum:** 2026-11-12
> **Typ:** CppModuleDoc
> **Status:** gebaut (Stufe 34d: der Flügel; 34g: Klasse 4, der Kern `bar`; 34h: Klasse 5, der Kern `membrane`; 34i: Klasse 1 ohne Flügel — Upright, Honky-tonk, CP-70 auf `hammerString`, der Kern `struckString`; 34j: Klasse 10, der Kern `freeReed`)
> **Modul:** InstrumentModels (STATIC, `projects/libs/InstrumentModels`)
> **Namespace:** `lumi::kern` (Fabrik, geteilte Bausteine), je Klasse `lumi::kern::<klasse>` (`lumi::kern::hammer`, `lumi::kern::saite`, `lumi::kern::stab`, `lumi::kern::trommel`, `lumi::kern::zunge`)
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
| `Kerne.hpp` | `kerne()`: die Fabrik der eingebauten Kerne — `hammerString` (34d), `bar` (34g), `membrane` (34h), `struckString` (34i), `freeReed` (34j); die übrigen Klassen folgen |
| `zunge/Zunge.hpp` | 34j: das Spiel `ZungenSpiel` (Größen je Taste als Kurven: Länge, Breite, Masse je Fläche, Dämpfung, Aufbiegung, Seitenspalt, Kanzelle, Kanal, Ventil; ein Windbereich `windMin`/`windMax`; Chöre und Bass-Chöre als `Chor`; die gestimmten Kurven f₀, Bend, Stärke je Stufe, Nachklang), `ZungenAnschlag` (Expression, Pitch, Vibrato, Balg je als Wert und Verlauf), `ZungenMessung`, `rechneZungenTon`, `rechneEineZunge`, `zungeAn`, `windBei`/`windMitte`, `bendAn`/`oeffnungFuer`/`ventilFuerCent` (ZV8 a), `staerkeBei` (ZV10 a), `nachklangBei` (ZV13 a), die vier Vorgaben `baueVorgabeAkkordeon/Harmonium/Melodica/Mundharmonika` (`src/zunge/Vorgaben.cpp`, Herleitung S190) |
| `zunge/ZungeKern.hpp` | der Kern: `kKernName` `"freeReed"`, `kernTyp()` (Tafel „Free reed“, Rechenfassung 2, neun Gruppen), die gestimmten Namen (`reedFrequency`, `bendStep1…7`, `bendLimitOpening`, `bendLimitCent`, `strengthAtLevel2…20`, `releaseTime`; `gestimmteNamen()`), die Spur `bellows`, die innere Größe `reedTip`, `werteAus`/`spielAus`, `anschlagAus`, `vorlagen()` (`accordion`, `harmonium`, `melodica`, `harmonica`), `werteVon`, `ZungeKern` |
| `saite/Saite.hpp` | 34i, aus IC: das Spiel `SaitenSpiel` (Kurven über den Umfang, Tangente oder Schlägel, Mittelsteg, Mitschwingen, Dämpfer, Garn, Schieber, Tonabnehmer, Verstärker), `SaitenAnschlag`, `SaitenMessung`, `saitenNachklang`, `rechneSaitenTon`, `rechneSaitenClip` (Mitschwingen je Clip), `richteSaitenSpiel` (auf den Umfang der Tafel, `kSaitenUmfangMin/Max`, KL4-1 a), die drei Vorgaben `baueVorgabeClavinet/Hackbrett/Cimbalom` |
| `saite/SaiteKern.hpp` | der Kern: `kKernName` `"struckString"`, `kernTyp()` (Tafel mit zehn Gruppen, Bezug in einer Runde), die Spuren `damperSlider`, `keyPressure`, `pickupSelection`, `werteAus`/`spielAus`, `abnehmerWahlAus`, `anschlagAus`, `vorlagen()` (`clavinet_modal` ausgeliefert, `hammered_dulcimer`, `cimbalom` nicht — KL1 a), `werteVon`, `SaiteKern` |
| `gemeinsam/Membran.hpp` | 34h, aus IC: die Moden kreisrunder Felle mit Luftlast (Rayleigh) und Kessel als Luftfeder, zwei Felle gekoppelt (`Fell`, `FellMode`, `Kessel`, `fellModen`, `formAn`, `besselNull`); der Löser `zerlegeSymmetrisch` aus dem Core |
| `trommel/Trommel.hpp` | 34h, aus IC: das Spiel `TrommelSpiel` (Felle mit `Belag` als Werte, Kessel, Schlägel, Gleiten, Teppich, Rauschen, Wirbel), `TrommelAnschlag` (mit `tempo` von außen und der Spur `pitch`), `TrommelMessung`, `bereite`, `rechneTrommelTon`, `trommelNachklang`, `belagFunktion`, die sieben Vorgaben (`baueVorgabe…`) |
| `trommel/TrommelKern.hpp` | der Kern: `kKernName` `"membrane"`, `kernTyp()` (Tafel mit dem Umfang je Instrument, Bezug in zwei Runden), die Spuren `stroke`, `roll`, `snaresOff`, `pitch`, `werteAus`/`spielAus`, `schlagAus`, `anschlagAus`, `vorlagen()` (die sieben mit Namen nach TM4 a, je mit den Quellen der Werte ohne Kurve, QM1 a), `werteVon` (die Werte einer Vorlage mit ihren Quellen; die Tafel trägt die der ersten Vorlage), `TrommelKern` |
| `gemeinsam/Balken.hpp` | 34g, aus IC: Biegemoden eines Balkens aus finiten Elementen (Euler–Bernoulli, Lager frei, gelenkig, eingespannt), `formAn`, `freiFreiVerhaeltnisse`, `kragBetaL`; der Löser `zerlegeSymmetrisch` aus dem Core |
| `gemeinsam/Reibung.hpp` | 34g, aus IC: die Reibung Bogen–Schwinger nach Galluzzo mit Haften und Hysterese (der Bogen am Stab, später Klasse 3) |
| `stab/Stab.hpp` | 34g, aus IC: das Spiel `StabSpiel` (Kurven über den Umfang des Instruments), `StabAnschlag`, `StabMessung` (mit dem gehaltenen Teil für den Stimmer), `stabModen`, `stabNachklang`, `rechneStabTon`, `richteStabSpiel`, die zehn Vorgaben (`baueVorgabe…`, `alleStabSpiele`) |
| `stab/StabKern.hpp` | der Kern: `kKernName` `"bar"`, `kernTyp()` (Tafel mit dem Umfang je Instrument, Auskunft, `erzeuge`), die Spuren `motor`, `bowed`, `malletOffset`, `damperPressure`, die gestimmten Werte `partnerGain` und `bowForce*`, `werteAus`/`spielAus`, `anschlagAus`, `vorlagen()` (die zehn mit Namen nach KS4 a, je mit den Quellen der Werte ohne Kurve, QM1 a), `werteVon` (die Werte einer Vorlage mit ihren Quellen; die Tafel trägt die der ersten Vorlage), `StabKern` |
| `hammer/HammerSaite.hpp` | das Spiel des Flügels: `HammerSaite` (22 Kurven über A0–C8), `kurvenAngaben()` (die eine Liste: Feld, Name in `coreParams`, Titel, Gruppe, Einheit, Skala, Grenzen), `richteHammerSaite`, `hammerSaiteVorgabe` (jede Kurve mit Belegkraft und Herkunft), `chorAn`, die Formeln nach RT-0425, Kartofelev & Stulov 2014 (Hysterese ε und Steife Q0, seit S187 HS-3 a) und Rigaud |
| `hammer/HammerTon.hpp` | der Ton: `HammerAnschlag` (Taste, Hz, Stufe, Halten, Dämpfer, Rate, `tempoFaktor`, Moderator, `ueberabtastung` — Vorgabe `kHammerUeberabtastung` 32 seit S187), `rechneHammerTon`, `hammerNachklang`, `hammerTempo`, Moden des Chors (`chorModen`, `ebenenModen`, `stossJeSaite`, `hammerTeiltonModen`), `kHammerKraftBezug`, `kSchalldruckBezug` |
| `hammer/HammerKern.hpp` | der Kern: `kKernName` `"hammerString"`, `kernTyp()` (Tafel „Hammer string“, Auskunft, `erzeuge`), `Pedalwerte` (Gruppe *Pedals*, FU2 a), `werteAus` (seit 34i mit dem Umfang, KL4 a)/`saiteAus`/`pedalwerteAus` (Spiel ↔ Werte), `anschlagAus`, `vorlagen()` (34i: `upright_piano`, `honky_tonk_piano`, `electric_grand_piano`; der Flügel selbst ist die Tafel), `werteVon`, `HammerKern` |
| `hammer/HammerSaite.hpp` (34i) | dazu `baueVorgabeUpright`, `baueVorgabeHonkytonk`, `baueVorgabeCp70` aus IC `VorgabenRest.cpp` (`src/hammer/Vorgaben.cpp`) |

### 3.1 Der Flügel — Kern `hammerString`

- **Tafel** (IK2-4 a, §6a; Titel „Hammer string“, TT1 a): Gruppen *Keys* (seit 34i `noteMin`/`noteMax`, KL4 a),
  *String*, *Choir*, *Decay*, *Hammer*, *Damper*, *Pedals*; die 22 Kurven aus `kurvenAngaben()`, dazu `undampedFrom`
  (Ganzzahl, 89), `softFactor` (1), `moderatorStiffness` (2), `moderatorExponent` (2). Möglicher Tastenumfang 21–108,
  `rechenFassung` 2 (seit S187, HS-3 a: der Kontakt 32-fach überabgetastet, mit der Hysterese nahe 1 erst dort
  konvergent). Der Körper steht **nicht** in der Tafel, sondern
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

### 3.3 Die Trommeln — Kern `membrane` (34h)

- **Ein Kern, sieben Trommeln:** Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo (Dateinamen nach TM4 a); eine feste
  Trommel hat eine Note (die Rolle im Kit), die Pauke den Umfang C3–G3 und `tunable` (die Note wählt die Spannung, (1,1)
  auf der Note). Gruppen *Keys*, *Heads* (Liste, ein oder zwei Felle, je mit Belag als Werte), *Shell*, *Stroke*,
  *Glide*, *Snares*, *Noise*, *Roll*, *Output*. Keine Kurven, keine gestimmten Werte; der Bezug in zwei Runden (die
  Rauschanteile hängen am Bezug, IK2-10 a).
- **Adapter** (TM1 a): die Geschwindigkeit des Kopfes folgt der Stufe in dB wie bei Hammer und Stab, tempoFf ·
  10^(Stufe/20) — `tempoPp` aus IC steht nicht in der Tafel. **Spielweisen** (TM2 a) als Spuren am Beginn der Note:
  `stroke` 0–5 (normal, Mitte, Rand, Randschlag, Reifen/Cross-Stick, gedämpft), `roll` (Wirbel über die Länge der Note),
  `snaresOff`; `pitch` ist das Pedal der Pauke. Feste Werte je Kit-Rolle und Drum-Spur trägt die App (`coreTracks`).
- **Vorbereitung:** die Moden je Instrument beim Bau des Kerns (`bereite`, 0,05–0,3 s; `vorbereitungTeuer`); die Vorgaben
  sind Daten. **Nachklang:** neu (IC kennt keinen) — die langsamste Mode bis −60 dB nach den Schlägen.
- Die Rechnung Bit für Bit wie IC (`werte/trommel_ic.txt`: zwei IC-Abdrücke, sieben ff-Töne, sieben Spielweisen der
  Snare, sieben Bezüge — gerechnet mit den IC-Quellen unter MSVC, `docs/modellierung/anbindung-ic/abdruck_trommel.cpp`).
- Übersicht, Rechenzeit und Kennzahl: `apps/LumiViz/docs/modellierung/membran/README.md`.

### 3.4 Klasse 1 ohne Flügel (34i)

- **Auf dem Flügel-Kern** (KL2 a): Upright (Flügel mit Bass gekappt auf 1,189 m, µ und B mal (L_F/L_U)²), Honky-tonk
  (das Upright, ±1 Hz bei A4 ab MIDI 41), CP-70 (eine Saite bis MIDI 42, 73 Tasten 28–100). Die IC-Vorgabe des Flügels
  ist dieselbe wie unsere (fünf IC-Abdrücke Bit für Bit). Ohne Boden (KL3-1 a: ein Upright-Bauplan mit Maßen fehlt).
- **Kern `struckString`** (KL1 a): die angeschlagene Saite aus IC — Tangente (Stoß und Klemme, nach dem Loslassen drei
  Halbtöne tiefer), Schlägel, Mittelsteg 3 : 2, Mitschwingen je Clip. Gruppen *Keys*, *String*, *Decay*, *Course*,
  *Excitation*, *Tangent*, *Damper*, *Pickups* (Liste der Lagen), *Amp*, *Output*. Bezug in einer Runde (er teilt nur
  den Ausgang); die Probe von LumiViz gibt dieselben Bits wie IC `normiere`. Auskunft: Rechenweg *Note*, Stufe im Namen,
  Spuren am Beginn der Note, innere Größen `contact`, `release`.
- **Umfang** (KL4-1 a): die Kurven aller Kerne richten sich nur nach dem Umfang der Tafel, nicht nach dem des Instruments
  (`richteStabSpiel`, `richteSaitenSpiel` ebenso) — ein kleinerer Umfang ändert keinen Ton darin.
- Die Rechnung Bit für Bit wie IC (`werte/klasse1_ic.txt`: zwölf Abdrücke aus `abdruck_msvc.txt`).

### 3.5 Die durchschlagende Zunge — Kern `freeReed` (34j)

- **Ein Kern, vier Instrumente:** Akkordeon (F1–A6, Bass-Seite unter F3 mit 8′ + 4′, Diskant Musette aus zwei Chören 8′),
  Harmonium (C3–F6, ein Chor, Gehäuse als Tiefpass 3 kHz), Melodica (F3–C6), Mundharmonika (C4–C7). Gruppen *Keys*,
  *Reed*, *Chamber*, *Wind*, *Ranks* und *Bass ranks* (Listen von `Chor`), *Case*, *Tunable*, *Output*.
- **Rechenfassung 2** (34j-6, ZV2 a–ZV9 a): die Größen je Taste als Kurven mit Quelle und Belegkraft (Gesetze aus der
  Herleitung S190, `docs/modellierung/zunge/2026-11-11_S190_vorgaben_je_taste.md`), ein Windbereich je Instrument über
  der Expression, f₀ am mittleren Druck gestimmt (ZV5 a), Bend über das Ventil je Taste auf dem Raster 2^(−1/4) mit
  Grenze (ZV8 a) — an Akkordeon und Melodica (`palletBend`).
- **Laufender Wind** (34j-7, ZV3 a, SK4 a): `KernAuskunft::anregungDauernd` — die Expression ist der Wind, kein Pegel;
  Expression, `pitch`, `vibrato` und `bellows` (Richtung 0/1, am Wechsel ein Druckeinbruch) über die Zeit, eine konstante
  Spur Bit für Bit wie der feste Wert. Stärke = `staerkeBei(Stufe)` · Expression (ZV10 a: je Taste gestimmt, bei welcher
  Stärke der Ton 2 … 20 dB leiser klingt; unter der Dynamik der Taste 0). Nachklang je Taste gestimmt (ZV13 a: der
  längste Ausklang über sechs Haltedauern und drei Stärken bis −60 dB unter der lautesten Stelle der Note).
- **Gestimmt** im Stimmer `InstrumentMeasure/ZungeStimmen` (alle vier in rund 41 s); Bezug `outputReference` über
  `normiere` (IK2-10 a).
- Die Rechnung des Übergangs Bit für Bit wie IC (34j-2), seit Rechenfassung 2 gegen `werte/zunge_ic.txt`, das der Kern
  selbst schreibt (`LUMIVIZ_ZUNGE_BEZUG_SCHREIBEN=1`, Vermerk J70).
- Übersicht, Befunde und Rechenzeit: `apps/LumiViz/docs/modellierung/zunge/README.md`.

## 4. Tests

- `tests/unit/UnitTests/test_InstrumentModels.cpp` — `*InstrumentModels*`: `kerne()` kennt den Flügel (Tafel), Werte ↔
  Spiel, der Adapter rechnet wie `rechneHammerTon`, Soft und Practice über Spuren.
- `test_InstrumentModelsStab.cpp` — `*InstrumentModels 34g*`: Balken und Reibung, die Tafel `bar`, gestimmt und
  normiert Bit für Bit wie IC, die Fälle aus IC `test_stab.cpp`, der Adapter, die Reihenfolge der gestimmten Werte, die
  Rechenzeit (meldet Zahlen); das volle Stimmen der Bogenfenster am Modell mit `LUMIVIZ_34G_STIMMEN=1` (rund 150 s).
- `test_LoopStab.cpp` — die mitgelieferten Dateien (schreiben mit `LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1`, Wächter,
  Hörproben `LUMIVIZ_34G_WAV`).
- `test_InstrumentModelsMembran.cpp` — `*34h*`: die Membran (Bessel-Verhältnisse, Nullstellen, Form), die Tafel
  `membrane`, Bezug und Abdrücke Bit für Bit wie IC, die Fälle aus IC `test_trommel.cpp`, der Adapter (Stufe, Spuren,
  Nachklang, innere Größen).
- `test_LoopTrommel.cpp` — die mitgelieferten Dateien (schreiben mit `LUMIVIZ_TROMMEL_INSTRUMENTE_SCHREIBEN=1`, Wächter),
  die Spielweise an Drum-Spur und Kit-Rolle, die Hörliste `LUMIVIZ_34H_WAV`, die Rechenzeit (meldet Zahlen).
- `test_InstrumentModelsKlasse1.cpp` — `*34i*`: die Flügel-Vorgabe gegen IC, Upright/Honky-tonk/CP-70 und
  Clavinet/Hackbrett/Cimbalom/Clip Bit für Bit wie IC (direkt und über den Kern), der Bezug über die Tafel gleich IC
  `normiere`, die IC-Fälle aus `test_hammer_rest.cpp`, die Rechenzeit (meldet Zahlen); der Messfall
  `LUMIVIZ_34I_KLEMMEN=1` (was das alte Klemmen am CP-70 änderte).
- `test_LoopKlasse1.cpp` — die mitgelieferten Dateien (schreiben mit `LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1`, Wächter),
  die Hörliste `LUMIVIZ_34I_WAV`.
- `test_InstrumentModelsZunge.cpp` — `*34j*`: die Tafel `freeReed`, die Gesetze je Taste, die Abdrücke und Kurven gegen
  `werte/zunge_ic.txt`, Windbereich und Bend als Wächter, Stärke je Stufe und Nachklang am Ton nachgemessen, konstante
  Spuren und Balg; Messfälle `LUMIVIZ_34J_MESSUNG`, `LUMIVIZ_ZV12_MESSUNG`, alle vier stimmen mit `LUMIVIZ_34J_STIMMEN=1`.
- `test_LoopZunge.cpp` — die mitgelieferten Dateien (schreiben mit `LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN=1`, Wächter), der
  Wind im Ton der App (Expression, Crescendo, Pitch, Vibrato), die Hörliste `LUMIVIZ_34J_WAV`, die Rechenzeit (Zahlen).
- `test_LoopHammerSaite.cpp` (Kurven, Vorgaben, Datei Fassung 24), `test_LoopHammerTon.cpp` (am gerechneten Ton
  gemessen; Bezug je Plattform Bit für Bit, Rechenzeit `*31b-3*`), `test_LoopKleinEigen.cpp`.
- Wächter der Bits: `*Loop Werte*`, `*Klangfassung*`, Flügel-Bezug; im Creator `test_LoopInstrumentEditor`
  „Instrument-Creator 34d“ (Ansicht der Tafel, Save as Bit für Bit).

## 5. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.7.0 | 2026-11-12 | Session 192 (Bibliothek 0.7.0; S183 trug die Doku 0.6.0, `Solution.json` blieb auf 0.5.0). S187 HS-3 a: der Hammer nach Kartofelev & Stulov 2014 (`stulovHammerHysterese`, `stulovHammerSteife`), `kHammerUeberabtastung` 32, `hammerString` Rechenfassung 2, die Celesta 32-fach (J65). 34j (S189–S192): der Kern `freeReed` unter `zunge/` — Übernahme Bit für Bit (J66–J68), Rechenfassung 2 mit den Größen je Taste (J70), laufender Wind, Stärke je Stufe und Nachklang je Taste (J71, J72), die vier Vorlagen `accordion`, `harmonium`, `melodica`, `harmonica` (J73). |
| 0.6.0 | 2026-11-09 | Session 183. LP2 b (WU1 a, KA1 a, CL1 a): die Vorgaben von Wurlitzer, Kalimba und Clavinet je Taste (`stab/Vorgaben.cpp` `wurlitzerMasseKg`, `kalimbaMasse`; `saite/Vorgaben.cpp` Durchmesser je Taste); Kern `bar`: Zupfen mit Kraft (`zupfKraftN`, Tafel `pluckForce`), Abnahmen `Schlitz` (Wurlitzer, Tafel `slot`) und `Steg` (Kalimba, Tafel `bridge`), Rechenfassung 2; Kern `struckString`: `durchmesserMm` statt `masseGm` (Tafel `stringDiameter` statt `stringMass`), der magnetische Tonabnehmer mit d² gewichtet, Rechenfassung 2; `StabStimmen` stimmt den Partner auch bei Abnahme Steg (Journal J60–J64). |
| 0.5.0 | 2026-11-09 | Session 181. 34i: Upright, Honky-tonk, CP-70 als Vorlagen des Flügel-Kerns (`hammer/Vorgaben.cpp`, `Vorlage`, `vorlagen()`, `werteVon`), der Umfang in der Tafel des Flügels (KL4 a), die Tafel heißt „Hammer string“ (TT1 a); der Kern `struckString` (`saite/`, KL1 a); `richteStabSpiel`/`richteSaitenSpiel` auf den Umfang der Tafel (KL4-1 a) (Journal J53–J59). |
| 0.4.0 | 2026-11-08 | Session 180. QM1 a: `Vorlage::quellen` und `werteVon` in `membrane` und `bar` — die Quellen je Instrument für die Werte ohne Kurve (Trommel je 2–5 der 9 Quellen; Celesta und Reed electric piano Stulov, Tine electric piano Sønderbo und Falaize & Hélie); die Tafel trägt die der ersten Vorlage; die Herkunft der Schlägel der Membran sagt jetzt, dass nur der Stock an der Kontaktzeit gesetzt ist (wie IC `Vorgaben.cpp`) (Journal J50, J51). |
| 0.3.0 | 2026-11-08 | Session 179. 34h (TM1 a–TM4 a): Klasse 5 als Kern `membrane` (Trommel, Vorgaben, Adapter), geteilter Baustein `gemeinsam/Membran` (Journal J42–J46). |
| 0.2.0 | 2026-11-08 | Session 178. 34g (KS1 a–KS4 a): Klasse 4 als Kern `bar` (Stab, Vorgaben, Adapter), geteilte Bausteine `gemeinsam/Balken` und `gemeinsam/Reibung`; der Flügel meldet `koerperAnDerSchicht` (Journal J31–J40). |
| 0.1.0 | 2026-11-07 | Session 176. Angelegt mit 34d (BK1 a, FU1 a, FU2 a): der Flügel als Kern `hammerString`. |

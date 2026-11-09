# Membran (Klasse 5) — der Kern `membrane`

> **Seit:** Session 179 (2026-11-08), Stufe 34h. **Zweck:** was LumiViz von der Klasse 5 aus `InstrumentsCalculation`
> übernommen hat, wie es geprüft ist und was es kostet. Die **Entscheide** stehen in
> [`audio/Instrumente_IC_Konzept.md`](../../audio/Instrumente_IC_Konzept.md) §6 (Teilplan 34h, TM1 a–TM4 a); die
> **Übernahme** je Baustein im [Übernahme-Journal](../anbindung-ic/Uebernahme_Journal.md) §6 (J42 ff.). Die
> **Recherche, das Modell und der Messplan** bleiben in IC (Z3 a, IK2-12 a): IC `klassen/Klasse 5 - Membran/`
> (`modell-entwurf.md`, `modell.md`, `messplan.md`, `offen.md`, `quellen.md`).

## Was LumiViz rechnet

Ein Kern für alle Trommeln: die Felle modal (Galerkin über die idealen Besselmoden, Luftlast nach Rayleigh, der Kessel als
Luftfeder, zwei Felle über dasselbe Volumen gekoppelt; `InstrumentModels/gemeinsam/Membran`), der Schlag mit
Hertz-Kontakt (`InstrumentCore/Modal`), das Tonhöhengleiten aus der Energie des Fells (Marogna, Avanzini & Bank 2010
Gl. 20), der Teppich der Snare als Rauschen, getastet vom Resonanzfell, ein statistischer Teil über `modesUpTo`, der Klick
des Reifens, Wirbel und Prellen. Die Herleitung steht im Kopf von `InstrumentModels/trommel/Trommel.hpp`.

| Datei | Instrument | Note / Umfang | Besonderes | Art |
|---|---|---|---|---|
| `low_tom` | Low tom | A2 (45) | zwei Felle 13″ (2000 / 1800 N/m, Avanzini & Marogna), Stock | drums |
| `timpani` | Timpani | C3–G3 (48–55) | ein Fell 25″ nach Packer, Kessel als halbes Ellipsoid, Paukenschlägel; die Note wählt die Spannung | percussion |
| `bass_drum` | Bass drum | C2 (36) | zwei Felle 22″ (Spannungen offen), Filzschlägel nahe am Rand | drums |
| `snare_drum` | Snare drum | D2 (38) | Schlag- und Resonanzfell 14″ (3325 N/m nach Bilbao et al.), Teppich | drums |
| `tabla` | Tabla | D4 (62) | ein kleines Fell mit dem Belag (syahi) nach Sathej & Adhikari — die Moden harmonisch | percussion |
| `conga` | Conga | D♯4 (63) | offener Kessel (ohne Luftfeder), Hand | percussion |
| `bongo` | Bongo | C4 (60) | wie die Conga, 7″ | percussion |

Alle sieben sind **ungemessen** (Z2 a, IK2-9). Die Nachbildung, deren Namen die IC-Conga braucht, heißt `synth_conga`
(IK2-8 a, TM4 a). Die Kit-Trommeln (Tom, Bassdrum, Snare) spielen auf jeder Note denselben Ton; ihre Rolle im Kit gibt die
Note 36.

## Stärke und Spielweisen (TM1 a, TM2 a)

- **Stärke:** die Geschwindigkeit des Kopfes folgt der Anschlagstufe in dB wie bei Hammer und Stab: tempoFf ·
  10^(Stufe/20). Gemessen (RMS über 1 s, Stufe −20 gegen 0): −20,0 bis −23,9 dB; Umfang von Stufe −40 bis 0
  30,9–49,8 dB (die IC-Abbildung über `staerke` gab nur −9,1 bis −11,3 dB und 18,2–23,9 dB).
- **Spielweisen** als Spuren am Beginn der Note: `stroke` (0 normal, 1 Mitte, 2 Rand, 3 Randschlag, 4 Reifen/Cross-Stick,
  5 gedämpft), `roll` (Wirbel über die Länge der Note), `snaresOff`; `pitch` ist das Pedal der Pauke. Feste Werte je
  Kit-Rolle und Drum-Spur (`coreTracks` in `.lvkit` Fassung 2 und `.lvloop` Fassung 18); Werte je Schritt mit 31g.

## Das Kit `modelled_drums` (TM3 a)

| Rolle | Klang | Volume |
|---|---|---|
| kick | `bass_drum` | 0,421 |
| snare | `snare_drum` | 0,472 |
| rim | `snare_drum`, `stroke` 4 (Cross-Stick) | 1,0 — **gekappt**, fehlen 4,9 dB |
| snare 2 | `snare_drum`, `stroke` 3 (Randschlag) | 1,0 — **gekappt**, fehlen 1,6 dB |
| tom 1 | `low_tom` | 0,637 |
| perc 1 | `conga` | 0,638 |
| perc 2 | `bongo` | 0,637 |
| perc 3 | `tabla` | 0,616 |

Hats, Ride und Crash kommen mit Klasse 6; bis dahin behält eine Spur sie beim Kitwechsel (K2 a). Alle sieben Trommeln
stehen wie jedes Kern-Instrument auf dem Bezug (−13,4 LUFS, `instrument_pegel.py`), die Kit-Trommeln eingeschlossen — das
Kit regelt nur nach unten. Das Beispiel `membrane_demo.lvloop` (8 Takte, 100 bpm) spielt das Kit, einen Wirbel als eigene
Spur, die Pauke (C3 / G3) und die Tabla als Instrument mit wechselnder Stärke.

## Prüfung

- Bit für Bit wie IC: `tests/unit/UnitTests/werte/trommel_ic.txt` — die zwei IC-Abdrücke (Tom mf, Pauke an 52 mf), je
  Instrument ff an der Mitte (über den Kern bei Stufe 0), die Spielweisen der Snare und die sieben Bezüge; gerechnet mit
  den IC-Quellen unter MSVC (`docs/modellierung/anbindung-ic/abdruck_trommel.cpp`, ohne Änderung am IC-Repo). Geprüft im
  Kern (`test_InstrumentModelsMembran`) und an den mitgelieferten Dateien (`test_LoopTrommel`), unter Release-clang und
  Testing-MSVC.
- Die Fälle aus IC `test_trommel.cpp` mit den IC-Grenzen: Bessel-Verhältnisse, Pauke gegen Packer (1,50 : 1,97 : 2,44 auf
  4 %), das (0,1)-Paar zweier Felle, die Tabla harmonisch, Kontakt und Kraft, Gleiten, die Pauke auf der Note,
  Spielweisen, Wirbel, Teppich, Determinismus.
- Hörproben: die Liste `hoertest/listen/membran_34h` für HörTest_Helper (geschrieben von `test_LoopTrommel` mit
  `LUMIVIZ_34H_WAV`): je Trommel a-18 / a-9 / a0 (die IC-Stärken pp, mf, ff), die Pauke in drei Lagen, sieben
  Spielweisen der Snare, der Tom gedämpft, drei blinde Paare IC gegen `synth_conga`; 38 Klänge auf −30,9 LUFS,
  `Hoertest.exe --liste-pruefen` „Liste gültig“.

## Befunde

- **Der Cross-Stick ist leise:** rund 15 dB unter der Snare (Schlag M max −28,7 gegen −13,4 LUFS bei Volume 1). IC rechnet
  ihn als Stock mit 0,15 der Geschwindigkeit und einen Klick (`clickLevel` 0,3, „offen“, `Trommel.cpp:143`). Im Kit
  gekappt; zu klären mit der Messung (IC-A5).
- **Der Teppich folgt der Stärke nicht:** IC teilt sein Rauschen durch die Geschwindigkeit (`Trommel.cpp:194`, „offen“) —
  unter etwa Stufe −20 wird die Snare kaum leiser (−40 gegen −30: 3 dB, Schwerpunkt 5,4–5,9 kHz).
- **Plattform:** die Bits hängen an der Laufzeit — mit `clang++` und statischer ucrt weicht die Bongo in einem Wert um ein
  float-ULP ab; MSVC `/MD` und der LumiViz-Bau unter clang (dynamische ucrt) sind gleich.

## Rechenzeit (IK2-6 a)

Gemessen am 2026-11-08 (S179) unter Release-clang, je dreimal allein nach der Taktprobe (feste Last, Spanne 1,02): ff an der
Mitte, 2 s Ton, das Beste aus drei Läufen.

| Trommel | Anteil der Echtzeit |
|---|---|
| Snare (der schwerste Ton) | **3,09–3,10 %** — **1,01–1,02 × Flügel-Kern** (A0, 4 s gehalten, 3,04–3,07 %) |
| Bassdrum | 2,76–2,83 % |
| Tom, Conga, Pauke | rund 2,2–2,3 % |
| Bongo, Tabla | unter 0,9 % |

Grenze 4,6 % (IK2-6 a) eingehalten; `*31b-3 Rechenzeit*` im Bass 2,61–2,67 %. Die Moden eines Instruments rechnet der Kern
beim Bau (alle sieben mit Bezug 1,4 s).

## Aus InstrumentsCalculation (S182)

- [ic/](ic/) — die Vorarbeiten aus IC zu Klasse 5 (dazu die Berichte zu Aufnahmen und Spielweisen des Schlagwerks, Klassen 5–6) (Auflösung von IC, AU1 a / AU1-1 a): Wortlaut unverändert, nur die Links gerichtet.

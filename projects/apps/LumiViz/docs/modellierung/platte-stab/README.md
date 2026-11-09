# Platte und Stab (Klasse 4) — der Kern `bar`

> **Seit:** Session 178 (2026-11-08), Stufe 34g. **Zweck:** was LumiViz von der Klasse 4 aus `InstrumentsCalculation`
> übernommen hat, wie es geprüft ist und was es kostet. Die **Entscheide** stehen in
> [`audio/Instrumente_IC_Konzept.md`](../../audio/Instrumente_IC_Konzept.md) §6 (Teilplan 34g, KS1 a–KS4 a); die
> **Übernahme** je Baustein im [Übernahme-Journal](../anbindung-ic/Uebernahme_Journal.md) §5 (J31–J41). Die
> **Recherche, das Modell und der Messplan** bleiben in IC (Z3 a, IK2-12 a): IC `klassen/Klasse 4 - Platte und Stab/`
> (`modell-entwurf.md`, `modell.md`, `messplan.md`, `offen.md`, `quellen.md`).

## Was LumiViz rechnet

Ein Kern für alle Stabspiele (KS1 a): der Stab modal (Biege- und Torsionsmoden in den Formen des gleichförmigen Balkens,
`InstrumentModels/gemeinsam/Balken`), Schlägel im Zeitbereich (Hertz oder Hunt–Crossley, `InstrumentCore/Modal`), Partner
der Moden als gekoppelte Schwinger (Resonatorrohr, Tonebar, Kasten), Motor des Vibraphons, Dämpfer (Leiste oder einzeln),
Abnahme akustisch oder über einen Tonabnehmer, die Zunge der Kalimba (gezupft) und der Bogen am Stab
(`gemeinsam/Reibung`). Die Herleitung steht im Kopf von `InstrumentModels/stab/Stab.hpp`.

| Datei | Instrument | Umfang | Besonderes | Tonhöhe geprüft |
|---|---|---|---|---|
| `marimba` | Marimba | C2–C7 (36–96) | Unterschnitt 1 : 4 : 10, Rohr, Werte nach Rucz & Ulveczki 2021 | Periode und erste Mode |
| `xylophone` | Xylophon | F4–C8 (65–108) | 1 : 3 : 9, das Orff-Xylophon der Arbeit | Periode und erste Mode |
| `vibraphone` | Vibraphon | C3–F6 (48–89) | Rohr mit Motor (Spur `motor`), Dämpferleiste, Bogen ab E♭4 (Spur `bowed`) | Periode und erste Mode |
| `glockenspiel` | Glockenspiel | G5–C8 (79–108) | Stahl frei–frei 1 : 2,757 : 5,404, ohne Rohr | erste Mode (unharmonisch) |
| `tubular_bells` | Röhrenglocken | C3–F4 (48–65) | gehört eine Oktave unter Mode 4, Dämpferpedal | erste Mode |
| `celesta` | Celesta | C4–C8 (60–108) | Stahlstab, Holzkasten als eine Mode, Filzhammer (Stulov 2009) | erste Mode (unharmonisch) |
| `kalimba` | Kalimba | D3–D6 (50–86) | Kragbalken, gezupft, ohne Kasten (Chapman 2006) | Periode und erste Mode |
| `tine_electric_piano` | Tine electric piano (Rhodes) | E1–E7 (28–100) | Tine, Tonebar, Neopren-Hammer, magnetischer Tonabnehmer | Periode und erste Mode |
| `reed_electric_piano` | Reed electric piano (Wurlitzer) | A1–C7 (33–96) | Stahlzunge, Filzhammer, elektrostatischer Tonabnehmer | Periode und erste Mode |
| — (`steelpan`) | Steeldrum | C4–E6 | **nicht ausgeliefert** (KS4 a): Tonfelder als Platzhalter (Balken statt Schale), in IC Deluxe | — |

Alle neun sind **ungemessen** (Z2 a, IK2-9): kein klangbestimmender Wert trägt *Gemessen*. Die Nachbildungen, deren
Namen die IC-Instrumente brauchen, heißen `synth_marimba`, `synth_vibraphone`, `synth_kalimba` (IK2-8 a, KS4 a).

## Gestimmte Werte (KS2 a, KS3 a)

- `partnerGain` — der Pegel des Partners, durch das Modell so gesucht, dass der Ton an `partnerGainNote` mit Partner um
  `partnerGainDb` lauter ist als ohne (+15 dB, „by as much as 15 dB“, Rucz & Ulveczki 2021 S. 7; F4, die Celesta C6).
- `bowForceLow*`/`bowForceHigh*` (pp, mf, ff) — die Fenster der Bogenkraft je dritter Note, in denen der Stab anschwingt,
  auf ±15 Cent spielt und haftet; sie hängen am Pegel des Partners (gestimmt in der Reihenfolge der Tafel).
- Gestimmt mit `InstrumentTuneTool <datei> --all --reference` (Befehl in `asset/loop/README.md`); der Wächter
  `*Instrument Stimmen*` meldet veraltete, ohne zu rechnen.

## Prüfung

- Bit für Bit wie IC: die elf Abdrücke aus IC `werte/abdruck_msvc.txt` und die Fenster aus IC `Stimmung.inc`
  (`tests/unit/UnitTests/werte/stab_ic.txt`), geprüft im Kern (`test_InstrumentModelsStab`) und an den mitgelieferten
  Dateien (`test_LoopStab`).
- Die Fälle aus IC `test_stab.cpp` (Moden 1 : 4 : 10 und 1 : 3 : 9, Rucz: T60 0,7 s mit Rohr, +15 dB, Kontakt 0,6 ms,
  Konvergenz des Kontakts, die Tine nach Falaize & Hélie, Motor, Dämpferleiste, Klirren des Tonabnehmers, Helligkeit mit
  der Stärke, Grenzen, gestrichen auf ±10 Cent) und B6 „Modalfit am Modell“ aus IC `test_messung.cpp`.
- Hörproben: die Liste `hoertest/listen/stab_34g` für HörTest_Helper (geschrieben von `test_LoopStab` mit
  `LUMIVIZ_34G_WAV`): je Instrument drei Lagen bei a-30 / a-12 / a0 und 9 blinde Paare IC gegen Synth, alle auf
  −27,2 LUFS (der höchste Pegel ohne Spitze über −1 dBFS).
- **Befunde beim Hören vorab** (wie in der IC-Liste der Klasse): fünf Töne bei a-30 in der Höhe sind still (unter
  −70 LUFS — Xylophon und Celesta G7, Glockenspiel A6, Tine B6, Reed G6; der Anschlag ist dort zu schwach); bei der Kalimba
  sind die Stärken nach dem Angleichen gleich (das Zupfen ist linear, die Stärke ändert nur den Pegel).

## Rechenzeit (IK2-6 a)

Gemessen am 2026-11-08 (S178) unter Release-clang auf dem Rechner des Projekts (Windows 11, der Prozessor an der
Wärmegrenze — RZ1-1 a: nach der Taktprobe, je dreimal allein; `*34g Rechenzeit*` meldet die Zahlen):

| Ton | Anteil an der Echtzeit | Verhältnis zum Flügel-Kern |
|---|---|---|
| Vibraphon C3 mit Motor, Stufe 0, 2 s (der schwerste) | 2,07–2,17 % | 0,67–0,70 |
| Vibraphon gestrichen an E♭4, ff, 2 s | 0,60–0,62 % | 0,19–0,20 |
| Marimba C2 | 0,26–0,28 % | 0,085 |
| Röhrenglocken C3 | 0,24–0,25 % | 0,08 |
| Tine electric piano E1 | 0,18–0,19 % | 0,06 |
| Xylophon, Celesta, Reed electric piano, Glockenspiel, Kalimba | 0,08–0,17 % | 0,03–0,05 |
| Flügel-Kern A0, 4 s gehalten, 4,8 s Ton (Bezug) | 3,0–3,2 % | 1 |

Die Grenze von 4,6 % hält jedes Instrument ein. IC maß in S172 unter Windows-clang 2,21 % für das Vibraphon.

## Offen

- Die Messung (IC Plan A–D, messplan.md): jeder Wert ist eine Quelle, eine Rechnung oder eine Einschätzung.
- Die Lizenzen der Arbeiten im Register (`asset/loop/instrument_quellen.json`: „not checked“); die Klausel zur
  Veröffentlichung bei Sønderbo 2024.
- Die Spuren `motor`, `bowed`, `malletOffset`, `damperPressure` sind am Kern geprüft; im Loop spielbar mit 31g (Editor).

## Aus InstrumentsCalculation (S182)

- [ic/](ic/) — die Vorarbeiten aus IC zu Klasse 4 (Auflösung von IC, AU1 a / AU1-1 a): Wortlaut unverändert, nur die Links gerichtet.

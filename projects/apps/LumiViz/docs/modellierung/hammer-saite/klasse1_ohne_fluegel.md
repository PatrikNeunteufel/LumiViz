# Klasse 1 ohne Flügel — Upright, Honky-tonk, CP-70, Clavinet

> **Seit:** Session 181 (2026-11-09), Stufe 34i. **Zweck:** was LumiViz von der Klasse 1 (Rest) aus
> `InstrumentsCalculation` übernommen hat, wie es geprüft ist und was offen bleibt. Die **Entscheide** stehen in
> [`audio/Instrumente_IC_Konzept.md`](../../audio/Instrumente_IC_Konzept.md) §6 (Teilplan 34i, KL1 a–KL4 a, KL4-1 a,
> KL3-1 a, TT1 a); die **Übernahme** je Baustein im [Übernahme-Journal](../anbindung-ic/Uebernahme_Journal.md) §6b
> (J53 ff.). Recherche, Modell und Messplan bleiben in IC (Z3 a, IK2-12 a): IC `klassen/Klasse 1 - Hammer-Saite/`
> (`modell-entwurf.md`, `modell.md`, `messplan.md`, `offen.md`, `quellen.md`, `recherche/`).

## Was LumiViz rechnet

| Datei | Titel | Kern | Umfang | Tastatur | Pegel (`level`) |
|---|---|---|---|---|---|
| `upright_piano` | Upright piano | `hammerString` | 21–108 | Soft, Practice, Sustain | 1,8787 |
| `honky_tonk_piano` | Honky-tonk piano | `hammerString` | 21–108 | Soft, Practice, Sustain | 2,3097 |
| `electric_grand_piano` | Electric grand piano (CP-70) | `hammerString` | 28–100 | Sustain | 2,5073 |
| `clavinet_modal` | Clavinet modal | `struckString` | 29–88 | — | 0,5258 |

Alle **ungemessen** (IC F-6 b: K18 und VCSL für das Upright, FreePats für das Honky-tonk liegen lokal; CP-70 und Clavinet
ohne freie Aufnahme). Upright, Honky-tonk und CP-70 sind Vorgaben auf dem Kern des Flügels (die Rechnungen aus IC
`VorgabenRest.cpp` wortgleich); das Clavinet ist der zweite Saitenkern aus IC (`Saite.cpp`), der auch Hackbrett und
Cimbalom trägt — beide ziehen als Vorlagen mit, ohne Datei (KL1 a, Deluxe). Das heutige `clavinet` (Waveguide) bleibt
bis zum Hörtest (IK2-8 a, KL2 a).

**Ohne Boden** (KL3-1 a): der Ausgang ist die Stegkraft wie in IC. Ein Bauplan des Pleyel P131 nach Chaigne, Cotté &
Viggiano 2013 wäre möglich bis auf die Teile, die je Taste wirken — Steglinie und Bassteg gibt es dort nur als Bild ohne
Achsen, die Stegpunkte je Taste gar nicht. Eine Recherche nach einem Upright-Bauplan mit Maßen läuft; ihr Bericht kommt
hierher (`recherche/`).

## Prüfung

- **Die Vorgabe des Flügels** gibt die fünf IC-Abdrücke `hammer_vorgabe_m*_a-12_d500` Bit für Bit — sie ist in beiden
  Repos dieselbe.
- **Bit für Bit wie IC** (`tests/unit/UnitTests/werte/klasse1_ic.txt`, aus IC `abdruck_msvc.txt`): Upright, Honky-tonk,
  CP-70, Clavinet, Hackbrett, Cimbalom (Taste 60, Stufe −12) und der Clip des Hackbretts (C4, G4) — direkt, über den Kern
  und aus den mitgelieferten Dateien. Der Bezug über die Tafel (1 s) gleich IC `normiere` (0,5 s).
- Die IC-Fälle aus `test_hammer_rest.cpp`: Mensur des Upright, Practice leiser und dunkler, Soft mit Faktor 1 neutral,
  ±1 Hz beim Honky-tonk, eine Saite des CP-70 bis 42, drei Halbtöne nach dem Loslassen, Tastendruck mit Grenze 0,
  Schieber, Tonabnehmer, Mittelsteg, Mitschwingen je Clip.
- Hörliste `hoertest/listen/klasse1_34i` (50 Klänge, 44 Punkte, −35,3 LUFS, `--liste-pruefen` gültig); Demo
  `keys_demo.lvloop`; Creator mit Upright und CP-70 (Bild gesehen).

## Befunde

- **Das Klemmen der Kurven** auf den Umfang eines Instruments änderte den Klang darin (am CP-70 der Ton an Taste 60 um
  −36 dB, B an Taste 28 um 95 %) — behoben für alle Kerne (KL4-1 a); die Abdrücke von `bar` und `membrane` blieben gleich.
- **Honky-tonk bei pp:** an Taste 103 bei a-30 ist das Honky-tonk Bit für Bit das Upright (bis auf den Pegel); ab a-12
  wirkt die Verstimmung. Vermutlich trifft der Hammer leise über `stossLeise` praktisch eine Saite — ungeprüft.
- **Pegel der Hörliste** −35,3 LUFS: ein Klang mit hoher Spitze je Lautheit bestimmt den gemeinsamen Pegel.

## Rechenzeit (IK2-6 a)

**Ausgesetzt**, bis die Kühlung des Rechners erneuert ist (Patrik S181). Der Fall `*34i Rechenzeit*` ist gebaut
(meldet Zahlen: tiefster Ton je Instrument, Anteil an der Echtzeit, Verhältnis zum Flügel-Kern). In IC (S172, unter
Windows-clang): CP-70 2,09 %, Upright 1,88 %, Clavinet 0,64 %, Hackbrett 3,64 %, Cimbalom 5,34 % (Deluxe).

## Offen

- Körper des Upright (Recherche, KL3-1 a).
- Hörtest `klasse1_34i` (Patrik): welches Clavinet den Namen behält; ob das Upright sich vom Flügel abhebt.
- Lizenzen der fünf neuen Quellen im Register (`yamaha-u1`, `ptg-honkytonk`, `smem-cp70`, `modartt-cp80`, `remaggi2012`).
- Messung an K18 (Upright) und FreePats (Honky-tonk) — die Aufnahmen liegen auf der Wechselplatte (bei Patrik anfordern).

# Modell Hammer-Saite (Rest) — gebaut (Klasse 1, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/01_hammer_saite/` rechnet. **Stand:** 2026-10-29, IC Session 4
> (Cloud). **Code:** `HammerSaite.hpp/.cpp`, `HammerTon.hpp/.cpp` (der **portierte Flügel-Kern**, Prüfstein des
> Grundgerüsts, mit Erweiterungen), `VorgabenRest.cpp` (Upright, Honky-tonk, CP-70), `Saite.hpp/.cpp`,
> `SaitenVorgaben.cpp` (Clavinet, Hackbrett, Cimbalom). **Tests:** `test_hammer_bezug.cpp`, `test_hammer_ton.cpp`,
> `test_hammer_rest.cpp`. **Offen:** [offen.md](offen.md). **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 a,
> F-2 a, F-3 a, F-4-1 a, F-5 a, F-6 b, F-7 a, F-8 a. Belegkraft wie in Klasse 4 (*Quelle* / *nur Bericht* /
> *Rechnung* / *offen*). **Alle Instrumente sind ungemessen** (K18 und VCSL liegen lokal; CP-70, Clavinet, Cimbalom
> ohne freie Aufnahme — F-6 b).
> **Der Flügel selbst** ist nicht Teil des Auftrags (AUFTRAG_BAU §10); er wird in LumiViz fertig. Den LumiViz-Stand vor
> jeder Arbeit an Klasse 1 neu lesen (Hinweis Patrik).

## 1. Der portierte Kern (Prüfstein)

`HammerTon` und `HammerSaite` aus `gemeinsam/vorbild-lumiviz/code/loop/` (LumiViz `d1c06309`), Rechnung wortgleich;
`Tastenkurve` ist `ic::Kurve` (dieselbe Interpolation), die Qt-Datei entfällt (JSON über `gemeinsam/Json`).
**Gegen den Bezug** `werte/hammer_bezug_clang.txt` (P8 a, S153; die Saite aus `hammer_bezug_saite.json`, ohne die
später gekommene Hysterese ε = 0): alle neun Töne, Zahl der Abtastwerte genau; Pegel je 0,5 s bei einer Saite auf
< 5·10⁻⁷ dB (die Datei hat sechs Nachkommastellen), im Chor auf ≤ 0,004 dB (P8 a: „im Chor mit Toleranz“ — der
Eigenlöser kam nach dem Bezug, 31d-7); unter gcc 13 und clang 18 Bit für Bit gleich. Die Prüfsumme der Datei ist
**nicht nachrechenbar** (Verfahren nicht in der Kopie; FNV-1a, `std::hash`, Polynom 31 geprüft) — offen.md.

**Erweiterungen des Ports** (mit den Vorgaben Bit für Bit wie das Vorbild, Abdrücke unverändert):

| Erweiterung | Rechnung | Herkunft |
|---|---|---|
| Pedal Mitte *Bass sustain* (`PedalArt::BassSustain`, `bassSustainBis`) | hebt die Dämpfer unter der Grenztaste (`daempferWirkt`) | F-2 a |
| Pedal links *Soft* (`tempoFaktor`, `Tastatur::softFaktor`) | Hammergeschwindigkeit mal Faktor | Vorgabe 1 (neutral, ungemessen: „nearly negligible“, Forum, nur Bericht) |
| Pedal Mitte *Practice* (`moderatorSteife`, `moderatorExponent`) | ein zweiter Filz Q_m·δ_m^p_m in Reihe: die Eindrückung teilt sich so, dass beide dieselbe Kraft tragen (Halbierung je Feinschritt, Gedächtnis des Hammerfilzes vom vorigen Schritt) | Entwurf §2.1 (Einschätzung, kein Modell in der Literatur); Vorgabe Q_m = 2 N/mm², p_m = 2 (offen) |

## 2. Upright, Honky-tonk, E-Flügel (auf dem Kern des Flügels)

| Größe | Upright | Honky-tonk | CP-70 |
|---|---|---|---|
| Saitenlänge | Flügel (RT-0425) **gekappt auf 1,189 m** (Yamaha U1 46,8″, nur Bericht) — offen bis K18 (F-7 a: zurückrechnen aus B) | wie Upright | Flügel (offen) |
| Masse je m | gleiche Spannung T = 4L²f0²µ → µ·(L_F/L_U)² (Rechnung) | wie Upright | Flügel |
| Inharmonizität B | B_F·(L_F/L_U)² (Bericht: „bei 0,6-facher Länge … etwa das 2,8-fache“, Einschätzung) — A0: 2,85-fach | wie Upright | Flügel |
| Verstimmung im Chor | Flügel (an Salamander) | **7,87 Cent ab MIDI 41** (±1 Hz bei A4, „only detuned the plain wire strings“, Forum, nur Bericht) | Flügel |
| Saiten im Chor | Flügel | Flügel | **1 bis MIDI 42, darüber 2** (nur Bericht) |
| Anschlagstelle | **1/8** (Patent 1/7–1/9, nur Bericht) | 1/8 | Flügel (0,12) |
| Hammer, Abklingen, Dämpfer, Winkel | vom Flügel übernommen (an Salamander bzw. Stulov) — offen | dto. | dto. |
| Boden | **keiner** — Ausgang Stegkraft (Körper aus Bauplan nach Chaigne et al. 2013 offen, lokal) | keiner | keiner (Piezo misst die Stegkraft, Entwurf §2.3) |
| Pedale | Soft / Practice / Sustain (F-2 a) | wie Upright | nur Sustain; 73 Tasten E1–E7 (offen) |

## 3. Clavinet, Hackbrett, Cimbalom (Saitenkern `Saite`)

Saite wie im Vorbild (Teiltöne mit B, Chor in zwei Ebenen über `ebenenModen`, zwei Stufen, `abklingHoch`).

**Clavinet** (F-3 a: Tangente als eigene Anregung): bewegtes Ende des klingenden Teils, Kosinus-Anstieg auf
d = 1 mm (offen) in d/v, v = 4 m/s bei Stufe 0 („1 to 4 m/s“, Gabrielli et al. 2013 S. 11, **Quelle**); beim
Loslassen Projektion auf die ganze Saite L·2^(3/12) (**−3 Halbtöne**, S. 6, **Quelle** — Test: 3 Halbtöne tiefer auf
0,2 %, im Ton gemessen auf 1 %), Garn T60 0,15 s (offen); Tastendruck als Pitch nach oben mit **Grenze 0 Cent**
(F-4-1 a — Test: bitgleich ohne Druck); Dämpferschieber stufenlos 0 … 1 (T60 0,3 s in Stellung 1, offen); zwei
Tonabnehmer (Geschwindigkeit) bei 0,1 und 0,3 der Länge vom Steg (Remaggi 2012 auf 0,5 m, nur Bericht). Offen: B je
Taste (Gabrielli Tab. 1), T60, die Kennlinie der Tonabnehmer (Polynom 4. Ordnung, Gabrielli Tab. 2), das Klopfen.

**Hackbrett, Cimbalom** (Deluxe nach F-1 a): Saitendaten aus **Peterson 2005** (Spannungstabelle, nur Bericht —
Test: f = √(T/µ)/(2L) trifft die Noten auf 5 %; die letzte Zeile „G#3 196“ ist G3), B = π³Ed⁴/(64TL²) (Rechnung);
Schlägel 10 g (Peterson: „8-12 grams“, nur Bericht) mit der Steife des Orff-Schlägels (übertragen); Cimbalom vier
Saiten im Diskant, drei im Bass (Wikipedia, nur Bericht), Schlägel mit Baumwolle als Filz (Stulov, übertragen),
Dämpferleiste. **Mittelsteg 3 : 2** (Peterson 1994, nur Bericht): der andere Abschnitt mit f·L_a/L_b, gekoppelt, wo
Teiltöne auf 0,5 % gleich liegen (jeder dritte des langen auf jeden zweiten des kurzen), Kopplung −0,5·σ_frueh am Steg
(offen). Abklingen „comparable to that of an undamped piano“ (Peterson 2005, nur Bericht) → die zweite Stufe des Flügels,
eine Stufe („can not be found in hammered dulcimers“, Mayer & Rust 2010, nur Bericht).

## 4. Schwellen (L4)

| Schwelle | Wert | gesucht/gesetzt |
|---|---|---|
| Mittelsteg: Teiltöne beider Abschnitte koppeln auf 0,5 % | 0,5 % | gesetzt |
| Bass sustain: Grenztaste | MIDI 60 | gesetzt (Ausstattung) |
| Honky-tonk: Verstimmung nur blanke Saiten ab MIDI 41 | 41 | gesetzt (Bassteg des Flügels) |
| Chor-Zahl gerundet (Kurve `strings`) | 0,5 | gesetzt (wie Vorbild) |
| Entartung (L11): δ ≥ 0,01 Cent im Saitenkern | 0,01 | Grenze |

## 5. Geprüft und Rechenzeit

| Prüfung | Ergebnis |
|---|---|
| Port gegen den Bezug (neun Töne) | eine Saite < 5·10⁻⁷ dB, Chor ≤ 0,004 dB; gcc = clang |
| Upright: Länge, µ, B, Spannung; Pedale (Bass sustain, Dämpfer) | exakt |
| Moderator leiser und dunkler; Soft mit Faktor 1 bitgleich | grün |
| Honky-tonk: äußere Saiten 2 Hz bei A4 — die Eigenwerte des Chors 2 Hz auseinander | auf 10 % |
| Clavinet: −3 Halbtöne nach dem Loslassen; Tastendruck mit Grenze 0 bitgleich; Schieber; heller mit der Stärke | grün |
| Hackbrett: Peterson-Tabelle 5 %; Mittelsteg wirkt am dritten Teilton | grün |
| Determinismus (Threads), Abdruck (gcc = clang) | grün |
| Rechenzeit tiefster Ton 2 s (Cloud; Flügel-Bezug 6,5 %) | Upright 3,4–4,7 %, Honky-tonk 3,4 %, CP-70 3,7–5,4 %, Clavinet 1,0 %, **Hackbrett 4,6–5,7 %, Cimbalom 7,7–9,1 % (Deluxe)** |

## 6. Hörproben

`ic_hoerproben <Ordner> 1` — 66 WAVs (Port des Flügels zum Vergleich mit LumiViz, Upright, Honky-tonk, CP-70,
Clavinet mit Schieber, Hackbrett, Cimbalom; je drei Noten × drei Stärken), −23 LUFS, `liste.txt`. Gehört: noch nicht
(L5, lokal).

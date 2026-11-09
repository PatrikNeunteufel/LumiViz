# Offen — Hammer-Saite (Rest), Klasse 1

> **Stand:** 2026-10-29, IC Session 4 (Cloud). Gebaut: [modell.md](modell.md). Code: `rechnung/klassen/01_hammer_saite/`.
> Alle Instrumente **ungemessen**; gesetzt wird lokal an K18 (Upright), FreePats (Honky-tonk), VCSL (Gegenprüfung).
> Spalte „Frage“: Kennung aus `offene-fragen.md`, oder „—“ (nur Daten).

## Größen ohne Quelle (Vorgabe mit Grund)

| Größe | Stand | Quelle | Vorgabe | Frage |
|---|---|---|---|---|
| Prüfsumme des Bezugs `hammer_bezug_clang.txt` | **nicht nachrechenbar** | Verfahren steht nur in `test_LoopHammerTon.cpp` (LumiViz, nicht kopiert) | Vergleich am Pegel je 0,5 s | — (lokal in LumiViz nachlesen, dann Test ergänzen) |
| Upright: Mensur (Länge, Masse, Längsverhältnis) | offen | U1-Bass 46,8″ (nur Bericht) | Flügel gekappt, gleiche Spannung | F-7 a: Fletcher 1964 beschaffen, sonst aus B an K18 zurückrechnen (lokal) |
| Upright: B | offen | Bericht (Einschätzung) | B_F·(L_F/L_U)² | — (M1 an K18) |
| Upright: Hammer, Abklingen, Verstimmung, Dämpfer, Winkel | vom Flügel | — | Flügel | — (M2–M6 an K18) |
| Upright: Boden (P131 nach Chaigne et al. 2013), Abstrahlung, Hörorte | **nicht gebaut** | Chaigne 2013 (teils nachgelesen); Steglinie nur als Bild | ohne Boden (Stegkraft) | — (Körper aus Bauplan: lokal, AUFTRAG_BAU §10) |
| Upright: Soft-Faktor | offen, **neutral 1** | Forum (nur Bericht) | 1 | F-2 a (Ort entschieden) |
| Upright: Moderator Q_m, p_m | offen | kein Modell gefunden | 2 N/mm², 2 | F-2 a; Hörprobe |
| Upright: Bass-sustain-Grenze | offen | — | MIDI 60 | F-2 a |
| Honky-tonk: Verstimmung über den Umfang | offen | ±1 Hz bei A4 (nur Bericht) | 7,87 Cent fest ab MIDI 41 | — (M3 an FreePats) |
| Honky-tonk: Tack piano | **nicht gebaut** | keine Messung, kein Modell | — | — |
| CP-70: Saitenlängen, Hammer (Wildleder), Steg auf dem Rahmen | offen | Prospekt (nur Bericht) | Flügel | F-6 b (ohne Aufnahme) |
| CP-70: Vorverstärker, EQ, Tremolo | **nicht gebaut** (Effekt nach dem Instrument) | Prospekt (nur Bericht) | — | — |
| Clavinet: B je Taste, T60, Länge | offen | Gabrielli Tab. 1 (nur Bericht) | 10⁻⁴, 10 s, 0,5 m | F-6 b |
| Clavinet: Weg der Tangente d, Form des Stoßes | offen | Gabrielli: gefitteter Puls | 1 mm, Kosinus | — |
| Clavinet: Garndämpfung | offen | — | T60 0,15 s | — |
| Clavinet: Tastendruck | **Grenze 0 Cent** | keine Messung | 0 | F-4-1 a |
| Clavinet: Dämpferschieber T60 | offen | — | 0,3 s | F-4-1 a (Ort entschieden) |
| Clavinet: Wahl der Tonabnehmer, Verstärker | **gebaut** (Gabrielli 2013 S. 3, S. 6) | Quelle | Steg/Mitte/beide/Gegenphase; Shelves −3 dB/130 Hz, +3 dB/4 kHz (als RBJ-Shelf S = 1 gelesen) | — |
| Clavinet: Kennlinie 4. Ordnung (Tab. 2) | **nicht gebaut** | Gabrielli 2013 Tab. 2 (Quelle) | linear | Einheit der Auslenkung nicht genannt; in m, cm, mm mit Abb. 13 (0,080 → 0,074 T über 1,5 cm) nicht vereinbar — Remaggi 2012 (DAFx) lesen |
| Clavinet: Klangschalter Soft/Medium/Treble/Brilliant (Tab. 3) | **nicht gebaut** | Gabrielli 2013 Tab. 3 (Bauteile, H_i(z)) | ohne | Normierung der Impedanzen und Zusammenschaltung („cascading the H_i(z)“) nicht eindeutig — Schaltplan [36] oder Messung |
| Clavinet: Klopfen der Tangente | **nicht gebaut** | — | — | — |
| Hackbrett/Cimbalom: Saitendaten außerhalb G3–G5, Bass-Chöre | offen | Peterson 2005 nur Diskantsteg | Randwerte | — |
| Hackbrett: Chor-Zahl, Verstimmung | offen | Bericht 2–5 | 2, 0,3 Cent | — |
| Hackbrett/Cimbalom: Kopplung am Mittelsteg | offen | — | 0,5·σ_frueh | — |
| Hackbrett/Cimbalom: Schlägel (Holz/Leder, Baumwolle) | offen | Peterson (Masse, nur Bericht) | 10 g, Orff bzw. Stulov übertragen | — |
| Hackbrett/Cimbalom: Abklingen | offen | „comparable to an undamped piano“ (nur Bericht) | zweite Stufe des Flügels | — |
| Hackbrett/Cimbalom: **Mitschwingen aller Chöre** (ohne Dämpfer) | **gebaut** (T2 a / K2 F-3 a, je Clip, Deluxe), ungemessen | — | die stärkste Mode je Teilton an die nächste eines anderen Chors (0,5 %), Kopplung 0,5·σ_früh wie am Mittelsteg (offen); C4 nach 2,5 s +1,1 dB am Grundton; Cimbalom nur mit Pedal | Kopplung an einer Aufnahme mit Pedal (lokal); die anderen Chöre nur mit dem angeschlagenen Abschnitt (nicht beide Seiten des Mittelstegs) |
| Hackbrett/Cimbalom: Kasten mit Luftraum | **nicht gebaut** | keine Daten | ohne (Stegkraft) | T4 a (Baustein da) |

## Vereinfachungen

| Vereinfachung | Warum | Folge |
|---|---|---|
| Kein Boden bei Upright, Honky-tonk, Hackbrett, Cimbalom | Körper aus Bauplan lokal (AUFTRAG_BAU §0, §10) | Ausgang ist die Stegkraft, nicht der Schall am Hörort |
| Clavinet: Puls statt Kontaktgesetz der Gummispitze | Q0, p der Spitze ohne Quelle; Gabrielli nimmt selbst einen Puls | Helligkeit über die Stärke nur über die Dauer d/v |
| Mittelsteg: nur die stärkste Mode je Teilton gekoppelt | Paar-Kopplung je Teilton | Chor-Gegenlagen koppeln nicht über den Steg |
| Moderator: Gedächtnis des Hammerfilzes aus dem vorigen Feinschritt | explizit, Halbierung je Schritt | Fehler von der Ordnung Δt_fein/τ |

## Feinabstimmung (lokal, L6, T9 a)

Plan A–D nach L15–L18 steht im [messplan.md](messplan.md) (F-B6 a, 2026-10-30); Hörschwellen aus
`gemeinsam/hoerschwellen.md` (angelegt, nur Bericht — lokal nachzulesen); Bezug K18 (sechs Stufen, CC BY 4.0), VCSL (CC0), FreePats für das Honky-tonk.

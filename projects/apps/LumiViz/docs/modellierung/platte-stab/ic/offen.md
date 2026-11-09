# Offen — Platte und Stab (Klasse 4)

> **Stand:** 2026-10-29, IC Session 4 (Cloud, Bau der Grundlagen). Nachgeführt bei jedem Schritt (Patrik: „es muss
> einfach ganz klar notiert sein, was noch offen ist“). Gebaut: [modell.md](modell.md). Code:
> `rechnung/klassen/04_platte_stab/`. **Alle zehn Instrumente sind ungemessen** — gesetzt wird lokal an Iowa (F-6 a)
> und, nach Freigabe von Datei und Größe, an jRhodes3d/SASS-E nur zum Messen (F-7 c).
> **Spalte „Frage“:** eine Kennung aus `offene-fragen.md`/`FRAGEN_KONSOLIDIERT.md`, oder „—“ (kein Entscheid nötig,
> nur Daten).

## Größen ohne Quelle (Vorgabe mit Grund)

| Größe | Stand | Quelle | Vorgabe | Frage |
|---|---|---|---|---|
| Dämpfung über den Umfang (alle) | offen | Rucz nur F4 | fester ξ (T60 ∝ 1/f) | — (M2 an Iowa) |
| Dämpfung über die Mode κ | offen | Rucz S. 7 qualitativ | 0 (Glockenspiel 1) | — (M2) |
| Masse des Stabs je Note | offen | Rucz nur F4 (96,7 g) | konstant | — (M4) |
| Formen φ_n mit Unterschnitt | **vereinfacht** | kein Profil mit Quelle | gleichförmiger Balken | — (Bauplan-Weg, Modalfit M1) |
| Moden über der dritten | **vereinfacht** | — | Abstand des gleichförmigen Balkens | — (M1) |
| Torsion f_t1/f_1, Dämpfung | offen | La Favre (nur Bericht, ohne Zahl) | 3, wie Biegung; ohne Wirkung bei Versatz 0 | — (M1, Anschlag außerhalb der Achse) |
| Abstrahlgewicht je Mode | offen | — | 1 | — (M3) |
| Anschlagstelle | offen | Messplan: „Mitte üblich“ (Einschätzung) | 0,5 | T3b a: Passage-Spur Schlägel-Ort |
| Schlägel Garn, Kordel, Gummi, Plastik, Messing | offen | nur der Orff-Schlägel (Rucz) | alle wie Rucz | K4 F-5 a (Ort entschieden), Werte M4 |
| v_max der Schlägel | offen | — | 5 m/s wie der Flügel | — (M4) |
| Resonator: ξ über den Umfang, höhere Rohrmoden | offen | Rucz nur F4, Mode 1 | ξ fest 8,8·10⁻³ für alle Moden | — (M5) |
| Rohrmoden (2m − 1)·f_r | **vereinfacht** | — | ideale Endkorrektur (fest über die Frequenz) | — (gemeinsamer Rohr-Kern, `gemeinsam/Rohr`, sobald ein Radius mit Quelle da ist) |
| Kopplung Stab–Rohr κ/κ_c | offen | — | 1,2 | — (M5: zweistufiges Abklingen) |
| Partner-Abstrahlung | gesetzt (Rechnung) | Rucz S. 7 „+15 dB“ (F4) | ein Gewicht für den ganzen Umfang | — (M5) |
| Vibraphon: Verhältnis 3, ξ, Masse | offen | CCRMA 1 : 4 (nur Bericht) | Balken-Abstand, 10⁻⁴, wie Palisander | — (M1, M2) |
| Vibraphon-Motor: Tiefe, Perioden je Umdrehung, Restöffnung, Hochlauf, Haltewinkel | offen | Patente (nur Bericht) | 2 je Umdrehung, r = 0, 0,5 s | K4 F-2 a (Ort entschieden); Messung M6 braucht eine Aufnahme mit bekanntem Motor |
| Dämpfer σ_D aller Instrumente | offen | Patent (nur Bericht, unklar) | 0,15–0,5 s | — (M7) |
| Glockenspiel, Röhrenglocken, Celesta: alle Werte außer den Balkenverhältnissen | offen | CCRMA (nur Bericht) | Einschätzungen | — |
| Röhrenglocken: Schlagton eine Oktave unter Mode 4 | nur Bericht | CCRMA | f_1 = 0,224·f_Note | — (Hörprobe) |
| Celesta: Kasten je Ton | offen | keine Fachquelle | eine Mode auf f_1, wie das Rohr | — |
| Kalimba: Kasten (Luftraum) | **nicht gebaut** | keine Maße | Brett-Kalimba | T4 a (Baustein da) — Daten |
| Kalimba: Zupfen, Auslenkung bei ff | offen | — | 1 mm an der Spitze | — |
| Rhodes: Stimmfeder, Tonebar (Frequenz, ξ, Kopplung), zwei Polarisationen | offen / **nicht gebaut** (zweite Polarisation) | Pfeifle 2017 (nur Bericht) | Tonebar auf f_1, ξ 10⁻³, 1,2·κ_c | K4 F-7 c: an jRhodes3d messen (lokal) |
| Rhodes: Tonabnehmer y0 (Voicing), Form Φ(y) | offen | Pfeifle: Punktladungen (nur Bericht) | Polmodell, y0 = 0,5 mm | K4 F-7 c |
| Wurlitzer: Zunge (Lötzinn), Abstand, C(y) aus FEM, Hammer | offen | Pfeifle (nur Bericht) | Kragbalken, 3 mm, Plattenkondensator, Stulov an C4 | K4 F-7 c |
| Steeldrum: Formen der Tonfelder, Kopplungskoeffizient c, Dämpfung | offen; Kopplung mit **Grenze 0** | Thomas et al. 2014 (nur Bericht) | Balkenformen als Platzhalter, c = 0 | K4 F-7 c: an SASS-E messen (lokal); T8 a |
| Bogen am Vibraphon (Bogen × Stab) | **gebaut** (T1 a), ungemessen | Reibung Klasse 3 (Galluzzo), v_B Chudy — übertragen | Fenster am Modell (Stimmung.inc); oben 18/18 auf ±10 Cent | — |
| Vibraphon gestrichen, tiefe Lage mit Resonator | **Befund** | — | rastet auf die obere Mode des Paars Stab–Rohr ein (+80 … +110 Cent); ohne Resonator sauber | κ/κ_c (von Rucz übertragen) an Iowa setzen, dann `ic_stimme_stab` |
| Bogen: Stelle, μ_s, Bogenhaar am Metall | offen | — | Ende des Stabs, μ_s 1,2 (Klasse 3) | — |

## Vereinfachungen gegenüber dem Entwurf (AUFTRAG_BAU §7: „jede Vereinfachung eine Zeile“)

| Vereinfachung | Warum | Folge |
|---|---|---|
| Kein Schub (Timoshenko) im Balken | Verhältnisse kommen ohnehin als Kurven aus der Literatur | Formen und obere Moden dicker Stäbe einige Prozent daneben |
| Partner nur im Paarfenster 6 % | RWA-Kopplung gilt nur nahe Gleichheit | ferne Rohrmoden ohne Wirkung |
| Partner während des Kontakts aus | κ·1 ms ≪ 1 | vernachlässigbar |
| Akustischer Ausgang als Σ g·q̈ (Monopol) | Abstrahlung kleiner Körper offen (`gemeinsam/koerper.md` §5) | Richtwirkung fehlt |

## Feinabstimmung (lokal, nach den Grundlagen — L6, T9 a)

Plan A–D nach L15–L18 steht im [messplan.md](messplan.md) (F-B6 a, 2026-10-30); Hörschwellen aus
`gemeinsam/hoerschwellen.md` (angelegt, nur Bericht — lokal nachzulesen). Bezug Iowa MIS (Marimba, Xylophon, Vibraphon, Bells), VCSL zum Gegenprüfen; die Messkette
(Modalfit ohne Reihe B6) entsteht unter `rechnung/messung/` (AUFTRAG_BAU §5).

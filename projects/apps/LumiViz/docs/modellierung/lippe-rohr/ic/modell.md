# Modell Lippe und Rohr — gebaut (Klasse 7, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/gemeinsam/Rohrschleife` und `rechnung/klassen/07_lippe_rohr/` rechnen.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Rohrschleife.hpp/.cpp` (gemeinsam), `Blech.hpp/.cpp`,
> `Vorgaben.cpp`, `Stimmung.inc` (erzeugt von `ic_stimme_blech`). **Tests:** `test_blech.cpp` (8 Testfälle).
> **Offen:** [offen.md](offen.md). **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 … F-7 a (über
> FRAGEN_KONSOLIDIERT T2, T3a, T3b, T3c, T5, T7, T8). Spielbar: **Posaune, Alphorn** (ungemessen), **Trompete** (F-B4 a,
> 2026-10-30: Bohrung nach Soares et al. 2025 Abb. 4, Ventile; ungemessen). Tuba, Horn, Didgeridoo: Gerüst ohne Bohrung
> (T8, Kipppunkt b; F-B1 a/F-B4 a ohne Fund).

**Trompete** (F-B4 a): Bohrung der vereinfachten B♭-Trompete von Soares, Cochelin, Fréour, Vergez, Arimoto (Acta
Acustica 2025, 9, 68, Abb. 4 S. 9, **Quelle**, CC BY 4.0): r = 8/8/2/5/5/15/20/60 mm bei z = −1355 … 0 mm. Kessel, Kehle
und Seele als ein Volumen 5,31 cm³ (Rechnung — der Kern kennt Kessel → Zylinder → Rest), Zylinder r 5 mm × 840 mm
(trägt die Ventile), Rest 350/40/40 mm bis r 60 mm. Spitzen der Impedanz offen 236/347/459/608/739/859 Hz (B♭-Ziel
233/349/466/587/698). **Ventile** 2/1/3 Halbtöne als L·(2^(k/12) − 1), L = 1,355 m (Rechnung). `stimme` wählt je Note
den Griff **am Modell**: für jede der 8 Teilmengen die Lippe im Sekantenverfahren, der kleinste Rest (+3 Cent je Ventil)
gewinnt — die Spitzen allein wählen falsch, weil das Modell rund 40 Cent über ihnen spielt; dazu ein **Stimmzug** für das
ganze Instrument aus dem oberen Quartil der Abweichung (die Lippe hebt leicht, senkt kaum). Der gewählte Griff ist der des
Modells, nicht immer der des Spielers (offen.md). Lippe und Druck wie bei der Posaune (Petiot 2023).

## 1. Aufbau (T5 a)

Mund → Lippe → **Kessel** (Nachgiebigkeit V/(ρc²)) → **Zylinder** als Verzögerung (veränderliche Länge = Zug;
Geschwindigkeit c + β·p⁺/(ρc), β = 1,2 = (γ + 1)/2 — das **Schmettern** in der Schleife, Cooper & Abel 2010 nach T5 a)
→ **Rest** der Bohrung als Reflexion und Abstrahlung aus der Übertragungsmatrix (FIR, Wandverluste nach Zwikker–Kosten
in erster Ordnung, Abstrahlung ohne Flansch nach Levine–Schwinger genähert). Die Lippe: ÿ + (ω_L/Q)ẏ + ω_L²y = Δp/µ,
nach außen schlagend, Spalt H0 + y, Fluss nach Bernoulli; mit Vokaltrakt in Reihe (zwei Resonanzen, normal null)
geschlossen gelöst. Growl: zweites Ventil (Stimmlippen, Öffnung (1 + sin)/2 bei der gesungenen Tonhöhe) vor dem
Mundraum.

**Abweichung vom Entwurf (begründet):** der Entwurf (§2) nannte einen modalen Kern mit Zug-Raster und das Schmettern als
Stufe am Rohrausgang; T5 a hat das ersetzt (Zug und Schmettern in der Schleife). Die Reflexion des Rests ist hier eine
abgetastete Impulsantwort (FIR) statt Moden wie bei Maestre 2018 — dieselbe lineare Bohrung, ohne Anpassung von Polen.

## 2. Größen

| Größe | Posaune | Alphorn | Herkunft / Belegkraft | Grenze | gesetzt an |
|---|---|---|---|---|---|
| Zylinder | r 0,7 cm, 1,596 m + Zug 0 … 1,06 m | (keiner: 1 cm am Mundstück) | Willemsen et al. 2021 Tab. 1 S. 157 (**Quelle**): Innenzug 70,8 / 71,1 cm, Bogen 17,7, Außenzug 2 × 53 cm; mittlerer Radius (Rechnung, offen) | Zug 0 … 2 m | Tonhöhe je Lage |
| Rest | Schwanenhals 24,1 cm r 0,71; Stimmzug 25,4 cm 0,75 → 1,07; Schallstück 50,2 cm 1 → 10,8 als Bessel-Horn ε = 0,7 | Kegel 539,92 / 844,91 / 754,84 mm auf R2–R4 = 16 / 28,5 / 44 mm, Kegel 4 (444,94 mm) als Zylinder, Becher 0,5 m 44 → 90 mm, R1 = 2,75 mm | Posaune: Willemsen Tab. 1 (**Quelle**), Profil des Schallstücks **offen**; Alphorn: Marcinnò et al. 2026 Tab. 3 S. 18 (**Quelle**), R1 (Lechgold, nur Bericht), Becher (Abb. 3b, abgelesen) **offen** | — | Impedanz (UNSW lokal), Tonhöhen |
| Kessel | 11,6 cm³ | (π/12)·(19 mm)³ = 1,8 cm³ | Boutin 2013 (nur Bericht); Lechgold 19 mm (nur Bericht, Rechnung) | — | Formant, Register |
| Lippe µ, Q | 1,3 kg/m², 5 | dto. | Petiot et al. 2023 Tab. 2 (nur Bericht, Trompete übertragen) | µ 0,05 … 20, Q 0,3 … 50 | Helligkeit (M2), Einschwingen (M3) |
| Lippe Breite, Spalt | 10 mm, 0,29 mm | dto. | Willemsen Tab. 2 (nur Bericht) | 1 … 50 mm, 0 … 3 mm | — |
| Lippe je Note (pp/mf/ff), Zuglage je Note | Kurven je Note | Kurven je Note | **Rechnung**: `stimme` — Register und Zuglage aus den Spitzen von Z mit Kessel, Feinstimmung durch das Modell auf 3 Cent | — | Tonhöhe (M1) |
| Anblasschwelle je Note P_s | Kurve | Kurve | **Rechnung**: kleinster Druck mit Wechseldruck im Kessel ≥ 30 % nach 0,4 s | — | — |
| Blasdruck über Expression | 1,3·P_s … min(14 kPa, 6·P_s), geometrisch in e | dto. | Petiot Tab. 1 „500 to 14000“ Pa (**Quelle**); 1,3 und 6 **offen** | 10 Pa … 30 kPa | Pegel und Spektrum über pp/mf/ff (M2) |
| Schmettern β | 1,2 | 1,2 (fast ohne Zylinder) | Burgers (Lehrbuch, offen) | 0 … 2 | Schwerpunkt über der Stärke (M4) |
| Dämpfer Straight / Cup / Harmon | 77 Hz Q 5 / 1 kHz Q 2 / 1,5 kHz Q 3, Restfläche 0,1 / 0,1 / 0,05 | — | Straight 77 Hz, Cup −20 dB um 1 kHz (nur Bericht); Rest **offen** | — | VSCO 2 CE (M5) |
| Vokaltrakt | Gewicht 0 | 0 | Fletcher et al. 2006 II Tab. I (nur Bericht): 1,5 / 2,5 kHz, Q 10; Spitze offen | 0 … 1 | Bend, Formanten (M6) |
| Zungenstoß (Anstieg des Drucks) | 5 ms | 5 ms | **offen** (Befund: mit 30 ms wuchs die Schwingung nahe der Schwelle zu langsam) | — | Einschwingen (M3) |

## 3. Schwellen (L4)

| Schwelle | Wert | gesucht/gesetzt |
|---|---|---|
| Register der Note: die kleinste Zuglage, bei der Spitze n die Note erreicht | — | gesetzt (`stimme`) — ein Registerwechsel am Bezug ist eine große Abweichung, kein Sprung der Messung |
| Anblasschwelle: Wechseldruck ≥ 30 % des Blasdrucks | 0,3 | gesetzt |
| Stoß in der Schleife: eine spätere Welle überholt eine frühere → sie verschmelzen | — | Physik (keine Wahl) |

## 4. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Schleife linear (β = 0) gegen die Übertragungsmatrix mit Kessel: Spitzen 2–6 | < 5 Cent |
| Posaune: jede vierte Note E2 … A4 bei pp, mf, ff schwingt und trifft | ±10 Cent (gemessen meist ±5) |
| Kette B♭3 → A♭3 mit Zugbewegung | zweite Note ±15 Cent |
| Schmettern (Myers et al. 2012 S. 681, **Quelle**: Schwerpunkt vor der Stürze ×2,7, im Mundstück ×1,3 im Crescendo) | am Schallstück **×1,54** (linear ×1,22), im Kessel **×1,09** — die Richtung stimmt, die Größe ist kleiner (offen.md) |
| Alphorn gegen Marcinnò Tab. 1 (Zielskala 92/139/185/231/278 Hz) | Spitze 2: +186 Cent; die Reihe ohne Mundstück bis +110 Cent unharmonisch — **Befund, offen** (Bohrung ≈ 0,4 m zu kurz) |
| Vibrato der Lippe, Growl, Dämpfer (leiser), Gerüste still | grün |
| **Wah-Wah** am Harmon (T3b a: Öffnungsgrad-Kurve an der Note; zweiter Filtersatz bei geschlossener Hand, linear gemischt) | zu −50,2 dB, offen −33,1 dB = Harmon ohne Hand |
| Determinismus, Abdruck (gcc = clang), Rechenzeit Posaune E2 2 s | **2,4 %** (Flügel-Bezug hier 6,5 %) |

**Zeit je Auswertung beim Setzen (L7):** ein Ton von 0,4 s rund 10 ms; die Stimmung aller 38 Noten der Posaune
(Schwelle mit 9 Halbierungen, Feinstimmung mit bis zu 5 Läufen je Stärke) dauert 3–6 min auf einem Thread — darum
liegt sie als erzeugte Datei vor (`Stimmung.inc`, neu mit `ic_stimme_blech` nach jeder Änderung am Modell).

## 5. Hörproben

`ic_hoerproben <Ordner> 7`: 37 WAVs (Posaune, Alphorn, Trompete (seit F-B4 a): drei Noten × pp/mf/ff; Vibrato, Growl, Fall, Legato,
drei Dämpfer). Gehört: noch nicht (L5).

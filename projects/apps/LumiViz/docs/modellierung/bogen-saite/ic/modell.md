# Modell Bogen-Saite — gebaut (Klasse 3, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/03_bogen_saite/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Bogen.hpp/.cpp`, `Vorgaben.cpp`, `Stimmung.inc` (erzeugt von
> `ic_stimme_bogen`). **Tests:** `test_bogen.cpp` (8 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:**
> [offene-fragen.md](offene-fragen.md) F-1 a, **F-2 b**, F-3 … F-8 a. Spielbar: **Cello, Violine** (ungemessen),
> Ensemble aus Stimmen (F-8 a); **Viola, Kontrabass** (F-B3 a, 2026-10-30): Saiten und Mensur aus Händlerangaben,
> Z0 = T/(2Lf), Güte übertragen, Korpus aus den Bestandsfrequenzen mit offenen Stärken — **ungemessen**.

## 1. Aufbau (F-2 b: Waveguide mit Reibstelle, Korpus modal am Steg)

**Saite:** Schnellewellen auf zwei Strecken je Richtung (Bogen–Sattel/Finger, Bogen–Steg), Verzögerungen mit
Lagrange-Interpolation 3. Ordnung, doppelt abgetastet (96 kHz). Am Sattel bzw. Finger fest (−1) mit einem
Verlustfilter erster Ordnung je Umlauf, angepasst an konstante Güte (Teilton k verliert e^(−πk/Q) je Periode), Gleichanteil
≤ 1; seine Phasenlaufzeit wird von der Länge abgezogen. Steifigkeit nicht gebaut. **Reibung** am Bogenpunkt nach
Galluzzo et al. 2017 Gl. 1 (**Quelle**): μ = 0,4·e^(−|Δv|/0,01) + 0,45·e^(−|Δv|/0,1) + 0,35, Haften bis μ_s = 1,2; der
Schnitt der Geraden v = v_h + f/(2Z0) (Woodhouse 2014 Gl. 2, nur Bericht) mit der Kurve nach Friedlander mit Hysterese
(es haftet weiter, solange die Haftkraft reicht; gleitet es, der äußerste Schnittpunkt — Newton ab dem letzten, Raster
mit Halbierung als Rückfall). **Steg:** die Admittanzmoden des Korpus im `RohrKern` (Kraft → Schnelle), geschlossen
gelöst, die reflektierte Welle v⁻ = u − v⁺ — so wirkt der Korpus auf die Saite zurück (**Wolfton**). **Ausgang:** du/dt
am Steg (Abstrahlung genähert, offen).

**Bogen:** v_B, F_B, Abstand zum Steg absolut (wie gespielt); Expression als **Weg** (F-3 a): v_B geometrisch von 0,08
bis 0,4 m/s, F_B in der Mitte des **Schelleng-Fensters**, das `stimme` je Note bei pp und ff **am Modell** sucht (nicht
aus der Formel — Schoonderwaldt et al. 2008 S. 604: „The minimum bow force was found to be independent of bow
velocity“, **Quelle**). Ansatz und Absetzen 50 ms, Bogenwechsel nach 0,65 m Haar (F-4 a). Vibrato als Längenänderung
(F-5 a, 5,5 Hz) — die Amplitudenmodulation je Teilton entsteht durch den Korpus. Spielweisen (F-6 a): Pizzicato
(Kraftstoß 3 ms), Spiccato (Kraft in Sprüngen), Tremolo (Bogenwechsel mit `springHz`), Col legno (Stoß 1 ms ohne
Reibung) — **ungemessen**; sul ponticello/tasto (Abstand ×0,5/×2), con sordino (Moden über 600 Hz ×0,75 in der
Frequenz, ×0,5 in der Stärke — offen). **Ensemble** (F-8 a): N Stimmen mit Streuung von Stimmung ±6 Cent, Einsatz
0–40 ms, Vibrato-Rate ±0,4 Hz, Korpus über 1 kHz ±3 % (gesetzt).

## 2. Größen

| Größe | Cello | Violine | Herkunft / Belegkraft | gesetzt an |
|---|---|---|---|---|
| Mensur | 690 mm | 327 mm | Händler (nur Bericht) / Bestand LumiViz | — |
| Saiten Z0 | C 1,48 · G 0,91 · **D 0,55** · A 0,58 kg/s | G 0,34 · **D 0,25** · A 0,19 · **E 0,18** | **fett:** Galluzzo Tab. 1, Schoonderwaldt S. 607 (nur Bericht); sonst Z0 = T/(2Lf) aus Spannungen (Larsen, Dominant — Händler) | Pegel, Fenster |
| Güte der Saite | 500 | 410 | Serafin 1999 / Schoonderwaldt S. 618 (nur Bericht); für alle Saiten übertragen (offen) | Abklingen (M1) |
| Bogen–Steg | 55 mm | 25 mm | Cello: β 0,08 (Galluzzo, nur Bericht), Spieler 4,5–9,2 cm (Chudy Tab. 1, **Quelle**); Violine offen | Helligkeit |
| v_B pp … ff | 0,08 … 0,4 m/s | dto. | Cello Legato 0,166 m/s im Mittel (Chudy, **Quelle**); Weg offen | Pegel über pp/mf/ff (M3) |
| F_B | Mitte des Fensters (Rechnung) | dto. | — | Schelleng-Fenster am Roboterbogen (M2, lokal) |
| Korpus | 104 Hz (Q 20, 0,02) · **174,1 Hz Q 13,4, 0,0525 s/kg** · 1 kHz (Q 2, 0,03) | 272,5 · 407 · 475 · 541 Hz (Q 50, je 0,05) · 2,3 kHz (Q 2, 0,05) | **fett:** Zhang & Woodhouse 2014 Tab. II (**Quelle**); Frequenzen Bestand/Bericht; **Stärken offen** (F-7 a: Modalfit an CNSM, lokal) | Stegmobilität (M4) |
| Dämpfer | ×0,75 Frequenz, ×0,5 Stärke über 600 Hz | dto. | Woodhouse 2014 S. 34 (Richtung, **Quelle**); Werte offen | Elie: 1–2,5 kHz −5…−13 dB |

**Viola und Kontrabass** (F-B3 a, Patrik 2026-10-30 — **ungemessen**):

| Größe | Viola | Kontrabass | Herkunft / Belegkraft |
|---|---|---|---|
| Mensur | 375 mm (16–16,5 Zoll) | 1060 mm (3/4) | Händler (For-Tune Charm, Kaplan Vivo LM 380 mm; Thomann zu Spirocore 3/4 — nur Bericht) |
| Saiten | Dominant 141: C 4,9 · G 4,8 · D 5,0 · A 6,0 kg → Z0 0,49 · 0,32 · 0,22 · 0,18 kg/s | Spirocore 3885.0: E 33 · A 32 · D 31 · G 30,5 kg → Z0 3,71 · 2,69 · 1,95 · 1,44 kg/s | Händler (electricviolinshop.com; Long & McQuade — nur Bericht), Z0 = T/(2Lf) |
| Güte der Saite | 410 | 500 | von Violine bzw. Cello übertragen (F-B3 a) |
| Bogen–Steg | 29 mm | 85 mm | Violine bzw. Cello im Verhältnis der Mensuren (Rechnung, offen) |
| Korpus | 210 · 281 · 328 · 373 Hz (Q 50, je 0,05) · 1,5 kHz (Q 2) | A0 66 Hz (Q 20, 0,02) · T1 115 Hz (Q 13,4, 0,0525) | Viola: die Moden der Violine × 0,77 / 0,69 / 0,65 nach Pätynen & Lokki S. 24 („23, 31, 35, and 44 percent lower“, wörtlich) — Rechnung; Kontrabass: Frequenzen Bestand S132, Güte und Stärke vom Cello; **Stärken offen** |

## 3. Schwellen (L4)

| Schwelle | Wert | gesucht/gesetzt |
|---|---|---|
| Die Note spielt (Helmholtz): ±50 Cent, Haften > 50 % der Takte | — | gesetzt (`stimme`) |
| Fenster: der längste Lauf auf dem Raster 2^(1/4) von 5 mN bis 8 N, ohne je einen Randschritt | — | gesetzt |
| Haften/Gleiten: Hysterese nach Friedlander | μ_s = 1,2 | Physik + Quelle |

## 4. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Pizzicato der leeren D-Saite: Teiltöne 1–4 bei k·f | ±0,6 % |
| Helmholtz am Cello D3 mf: Haften gegen 1 − β = 0,92 | **0,82** (Gleiten länger als ideal — die runde Ecke bei kleiner Kraft, Woodhouse); −1 Cent |
| Schelleng-Fenster Cello D3 | pp 0,057–0,76 N, ff 0,76–1,52 N (F_max steigt mit v_B — Schelleng); Galluzzo spielte 0,4–3,2 N bei β 0,08 (nur Bericht) |
| Jede dritte Note bei pp, mf, ff | **Cello 48/48, Violine 42/42, Viola 40/42, Kontrabass 42/42** auf ±10 Cent; ff lauter als pp — die Viola verfehlt nur die leere C-Saite (C3) bei pp (schwingt nicht) und mf (Oktave, +1198 Cent) |
| Einschwingen Cello G2/D3/A3 mf | 25–55 ms (Guettler & Askenfelt 1997, **Quelle**: annehmbar 50 ms, mit Mehrfachgleiten 90 ms) |
| **Wolfton**: kleinste Kraft an F3 (an der Korpusmode 174 Hz) gegen C3 | **0,226 N gegen 0,057 N** — entsteht aus der Rückwirkung, nicht gebaut |
| Vibrato 25 Cent: Hülle des stärksten Teiltons | 1,9 dB (ohne 0,0 dB) — AM von selbst |
| Dämpfer am Pizzicato der Violine, 1–2,5 kHz | −1,9 dB (Elie: −5…−13 dB — kleiner, offen.md) |
| Spielweisen, Lagen, Ensemble (deterministisch, ≠ eine Stimme); Viola und Kontrabass „ungemessen“ gekennzeichnet | grün |
| Determinismus, Abdruck (gcc = clang), Rechenzeit Cello C2 ff 2 s | **1,9 %** (Grenze hier 5,5–6,3 %) |

**Zeit je Auswertung beim Setzen (L7):** ein Ton von 0,6 s rund 25 ms (mit Messung 120 ms); die Stimmung (je Note zwei
Fenster auf 32 Rasterpunkten) für alle vier Instrumente 10¾ min auf vier Threads — darum als erzeugte Datei.

## 5. Hörproben

`ic_hoerproben <Ordner> 3`: je Instrument drei Noten × pp/mf/ff, Vibrato, Pizzicato, Spiccato, Tremolo, Col legno,
ponticello, tasto, sordino, Legato, Ensemble 8 — 76 WAVs (Cello, Violine, Viola, Kontrabass). Gehört: noch nicht (L5).

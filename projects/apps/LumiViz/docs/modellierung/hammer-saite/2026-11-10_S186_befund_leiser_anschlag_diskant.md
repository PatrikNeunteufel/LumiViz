# Befund LV6 (a) — der leise Anschlag im Diskant (S186, 2026-11-10)

> **Anlass:** LV-6 (`modellierung/anbindung-ic/2026-11-10_S186_lv6_gleiche_klaenge.md` §4): bei a-30 im Diskant (Upright
> und Honky-tonk G7, CP-70 H6) nur ein Puls, danach null; dazu die fünf stillen Stabspiele aus PN2 a. Entscheid NS1 a
> (Patrik S186): erst dieser Befund, dann 34j. **Ergebnis:** keine Schwelle im Rechenweg — der Hammer-Kern regt ab
> Taste 84 im Bereich ppp … p die Saite viel zu schwach an, bei allen vier Klavieren (auch `grand_piano`). Zwei Ursachen
> im Hammersatz: die **Hysterese fehlt im Diskant** (ein Vorzeichen in Stulov Gl. 9, seit HU3-1 c ohne Beleg) und
> vermutlich der **Exponent p ≈ 5** des nicht intonierten Satzes. Beim Stab ein zweiter Effekt (Nullstellen des Pulses).

## 1 Messung

Messfall `tests/unit/UnitTests/test_LoopLeiserAnschlag.cpp` (`LUMIVIZ_LV6A_MESSUNG=<datei.md>`, Release-clang, 3 s):
`rechneHammerTon` bzw. `rechneStabTon` direkt, je Taste (60 … 108) und Stufe (−40 … 0), ungedämpft, 1 s. Je Ton:
Geschwindigkeit, Kontaktzeit, Kontakt/Periode, größte Kraft, Pegel des Rohtons, **Nachklang** = Energie ab Kontaktende
+ 1 ms gegen die ganze. Gegen *Salamander*: `pegelumfang` (Pegel des Tons in den ersten 0,3 s, v16 gegen v1,
`werte/salamander_c5.txt`), am Modell der Pegel des Tons (Pegel + Nachklang) von ppp (−21, DY2 a: 0,45 m/s) bis ff (0).
[gemessen; v1 von *Salamander* ≈ ppp ist eine Annahme — die Geschwindigkeit der Aufnahme ist nicht bekannt]

## 2 Hammer-Saite: Kontakt mehrere Perioden lang

| Kontakt/Periode | Nachklang | Beispiel |
|---|---|---|
| 2 … 3 | −2 … −20 dB | Ton |
| ≈ 5 | ≈ −60 dB | kaum Ton |
| ≈ 9 | −93 dB | a-30 an G7 (103): nur der Puls |

Umfang des Tons ppp → ff, Modell (`grand_piano`, Vorgabe gleich) gegen *Salamander*:

| Taste | 60 | 72 | 84 | 90 | 96 | 100 | 103 | 106 | 108 |
|---|---|---|---|---|---|---|---|---|---|
| *Salamander* | 19,3 | 21,5 | 22,9 | 28,6 | 28,7 | 35,2 | 24,9 | 38,9 | 23,6 |
| Modell heute | 22,8 | 24,4 | 33,9 | 46,4 | 43,2 | 46,9 | 64,7 | 87,6 | 89,0 |
| Probe ε = 0,99 | 23,1 | 25,0 | 35,5 | 46,4 | 44,8 | 51,2 | 67,4 | 74,2 | 76,1 |
| Probe ε = 0,997 | 23,5 | 25,3 | 34,2 | 38,8 | 41,0 | 44,5 | 48,7 | 54,2 | 57,4 |

Upright, Honky-tonk und CP-70 wie der Flügel (±7 dB). In der Mitte trifft das Modell *Salamander* auf 3 dB, ab 84 läuft
es davon. [gemessen]

Kontaktzeit gegen Askenfelt & Jansson 1990 (Abb. 7, abgelesen ±0,1 ms; Recherche
`recherche/2026-11-10_S186_kontaktzeit_leiser_anschlag.md`; Modell bei −18 / −12 / 0 als pp / mf / ff):

| Taste | gemessen pp / mf / ff | Modell heute | ε = 0,997 |
|---|---|---|---|
| C6 (84) | 2,5 / 1,1 / 0,85 | 1,74 / 1,40 / 1,09 | 1,34 / 1,14 / 0,98 |
| C7 (96) | 0,9 / 0,65 / 0,55 | 1,43 / 1,05 / 0,71 | 1,00 / 0,77 / 0,60 |
| C8 (108) | 1,2 / 0,55 / 0,45 | 1,24 / 0,86 / 0,52 | 0,82 / 0,58 / 0,42 |

Lange Kontakte bei pp an C8 sind **gemessen** (≈ 5 Perioden, Askenfelt & Jansson Abb. 8: „even the fundamental period
is much too short to be optimally excited“) — die schwache Anregung im obersten Diskant ist real, aber *Salamander*
klingt bei v1 dort noch mit 24 dB Umfang, das Modell nicht.

## 3 Ursachen im Hammersatz

- **Hysterese (ε).** Stulov & Kartofelev 2009, Gl. 9, am Bild der Seite gelesen (S146 und S186): **ε = 0,9894 +
  8,8·10⁻⁵ n²**, τ = 2,72 − 0,02 n + 9·10⁻⁵ n² **[µs]**. Mit „+“ ist ε ab n ≈ 11 über 1 (unmöglich; S146: „nur im Bass
  brauchbar“). HU3-1 c (S159) schreibt „−“ — ohne Beleg; damit fällt ε im Diskant auf 0,31 (C8). In erster Näherung wirkt
  der Filz wie Q0·u^p plus eine Dämpfung mit der Zeit ε/(1 − ε)·τ: mit „−“ an G7 rund 1 µs (wirkungslos, der Puls bleibt
  glatt und symmetrisch), mit ε ≈ 0,99 rund 0,2–0,3 ms. Die Probe (§2) zeigt die Wirkung; welches ε gilt, ist offen
  (Stulov 2005, *Acta Acustica* 91, die Quelle der Formeln, ist nicht gelesen). [gelesen; gemessen]
- **Exponent p.** Der Satz ist an **nicht intonierten** neuen Abel-Hämmern gemessen (VITA 2010, wörtlich), p = 3,7 + 0,015 n
  (C8: 5,0); eingespielte Diskanthämmer bei Hall (KTH, Abb. 20, abgelesen) p ≈ 2,7–3,4, Russell „p≈4 in the treble“. Mit
  großem p wächst die Kontaktzeit bei kleinem v stark (T ∝ v^−(p−1)/(p+1)). Nicht nachgerechnet. [Recherche]
- **Abbildung der Stufe.** v = 5 m/s·10^(Stufe/20) reicht bis 0,05 m/s; unter ≈ 0,21 m/s erreicht der Hammer die Saite
  nicht mehr (Goebl 2001, Fußnote 6, wörtlich) — Stufen unter −27 sind physikalisch ohne Ton. Erklärt a-30 in der
  Hörliste, **nicht** den Abstand bei ppp … p (DY2 a: ppp −21 = 0,45 m/s). [Recherche]

## 4 Stab

- Celesta und Tine-Piano wie die Klaviere (Celesta 103 a-30: Kontakt 10 Perioden, Nachklang −69 dB).
- Xylophon, Glockenspiel, Reed: einzelne Einbrüche des Pegels bei Kontakt/Periode ≈ 1,5 und ≈ 2,5 (Glockenspiel 103 −6,
  Reed 91 −30, Glockenspiel 91 −36, Xylophon 103 −30), immer ein einziger Kontakt — **vermutet:** Nullstellen im
  Spektrum eines fast symmetrischen Kraftpulses auf dem Grundton (nicht nachgerechnet). Mit Hysterese würde der Puls
  asymmetrisch und die Nullstellen flacher.
- Damit sind die fünf stillen Stabspiele aus PN2 a erklärt: Celesta, Tine (Kontakt), Xylophon 103, Glockenspiel 93,
  Reed 91 (Nullstelle).

## 5 HS-1 a: der belegte Satz (S186)

**ε belegt.** Kartofelev & Stulov 2014 (*Acta Mechanica*, DOI 10.1007/s00707-014-1109-1; Dissertation Kartofelev, TTU
2014, S. 73, Gl. 4–7, **am Bild der Seite gelesen**): p = 3,7 + 0,015 N; **γ = 0,9894 + 0,000088 N, 0,9895 ≤ γ ≤ 0,9972**;
τ0 = 2,72 − 0,02 N + 0,00009 N² µs; F0 = 15 500·e^(0,059 N) N/mm^p; 1 ≤ N ≤ 88, nach Stulov 2005. Das **n²** im Druck von
2009 ist der Druckfehler, nicht das Vorzeichen — der Kern rechnet seit HU3-1 c ein falsches ε (C8: 0,31 statt 0,997).
Gegenprobe (Recherche, eigene Rechnung): VITA 2010 n = 70 im Dreiparametermodell α = 0,395 ms, Q0 = 4270 N/mm^p; mit der
linearen Form α = γτ0/(1 − γ) = 395 µs, Q0 = F0(1 − γ) = 4279. Recherche:
`recherche/2026-11-10_S186_hammerfilz_hysterese_exponent.md`. p eingespielter Hämmer je Taste: nichts Neues.

**Konvergenz.** Mit ε nahe 1 und τ0 ≈ 1,7 µs reicht der Kontakt 8-fach (2,6 µs) nicht: an C8 bei 0,45 m/s Umfang 59 dB
(8-fach), 44 (32-fach), 43 (128-fach). Für die Messung ist die Überabtastung seit S186 ein Feld von `HammerAnschlag`
(`ueberabtastung`, Vorgabe `kHammerUeberabtastung` = 8, Bit für Bit wie vorher — Bezug `*test_LoopHammerTon*`
grün). Heute (ε klein) ist 8-fach konvergiert.

**Gegen alle Zielgrößen** (Messfall `LUMIVIZ_HS1_MESSUNG=<datei.md>` in `test_LoopLeiserAnschlag.cpp`, Vorgabe des
Flügels, Release-clang; Werte bei 128-fach, konvergiert):

| Satz | (1) Kontakt, RMS log | (2) Umfang des Tons, RMS dB (C7 / D7 / C8) | (3) Helligkeit RMS C4 / C6 | (5) Auslenkung / Tab. IV |
|---|---|---|---|---|
| heute (ε „−“) | 0,33 | 18,6 (39 / 58 / 87) | 0,32 / 0,71 | 0,31 … 0,73 |
| S1: ε linear, Q0 nach 2009 | 0,29 | 6,0 (31 / 34 / 43) | 0,17 / **1,69** | 0,31 … 0,64 |
| S2: ε linear, Q0 = F0(1 − ε) | 0,29 | 6,1 (32 / 35 / 44) | 0,19 / **1,69** | 0,31 … 0,64 |
| Probe ε = 0,997 fest | 0,30 | 5,9 | 0,41 / 1,75 | 0,30 … 0,65 |

*Salamander* C7 / D7 / C8: 29 / 25 / 24 dB. Kontaktzeit S1 an C7 0,93 / 0,74 / 0,60 ms gegen 0,9 / 0,65 / 0,55, an C8
0,71 / 0,53 / 0,38 gegen 1,2 / 0,55 / 0,45. (4) Spreizung über den Umfang steckt in (2): A0 … C6 bleibt S1 bei 20–34 dB
wie heute.

**Widerspruch.** Der belegte Satz trifft den Umfang des Tons dreimal besser, die Kontaktzeit leicht besser und die
Helligkeit an C4 doppelt so gut — **aber an C6 wird der Hammer mit der Stärke nicht mehr heller** (Steigungen −0,15 …
0,16 statt *Salamander* 0,73 … 2,55; heute 0,39 … 3,53). Mit großer Hysterese bestimmt die Dämpfung des Filzes den Puls,
die Nichtlinearität kommt kaum noch zur Wirkung. Offen, ob das am Satz liegt (unintonierte Hämmer, p ≈ 4,3 an C6), an der
Messung der Helligkeit am Ton ohne Boden, oder an *Salamander* (Boden, Mikrofon). Noch nicht gerechnet: Rechenzeit mit
32-fachem Kontakt; der Stab (dieselbe Filzformel) mit dem neuen ε.

## 6 Offen

Vorlage an Patrik (S186): wie mit dem Widerspruch an C6 umgehen, bevor der Satz in den Kern kommt.

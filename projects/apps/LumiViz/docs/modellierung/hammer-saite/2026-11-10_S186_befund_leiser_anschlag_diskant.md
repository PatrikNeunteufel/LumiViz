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

## 6 HS-2: woher der Widerspruch kommt (S187, 2026-11-11)

**Werkzeug.** `HammerMessung::mitKraftVerlauf` hält die Kraft je feinem Schritt fest (nur zum Messen, Bit für Bit wie
vorher: 42 / 42 Fälle `*Hammer*,*Leiser*` unter Release-clang). Neuer Messfall `LUMIVIZ_HS2_MESSUNG=<datei.md>` in
`test_LoopLeiserAnschlag.cpp`: an C4 und C6 je Stufe Kontakt, Kraft, Grundton, Teiltöne 2 … 8 **am Ton und am Kraftpuls
allein**, dazu die Rechenzeit 8- gegen 32-fach. Im Messfall HS-1 neu: die Helligkeit gegen *Salamander* an **allen 30
gemessenen Tasten** (`helligkeit_n` in `salamander_c5.txt`, bisher nur C4/C6 genutzt), `LUMIVIZ_HS1_FEIN=<n>`,
`LUMIVIZ_HS1_NUR_S2`, und drei Sätze „S2 mit α × k“.

**Konvergenz.** Die Helligkeit ist bei 32-fach konvergiert (gegen 128-fach höchstens 0,05).

**Ursache: die Zähigkeit des Filzes.** Für t ≫ τ0 (τ0 ≈ 1,8 µs, Kontakt ≈ 1 ms) rechnet Stulovs Gesetz
F ≈ Q0·(φ + α·dφ/dt) mit α = ετ0/(1 − ε). Heute (ε „−“) ist α an C6 ≈ 3 µs, mit dem belegten ε ≈ 360 µs. Das ist ein
Hochpass (1 + jωα) auf den Puls, gleich für jede Stärke: an C6 ist bei S2 schon der **Kraftpuls allein** bei 0,16 m/s
so hell wie bei 5 m/s (Teilton 8 gegen 1: −34 dB bzw. −30 dB; heute −112 bzw. −20 dB). Steigung am Puls über alle
Stufen 0,00 … 0,04 (heute 1,1 … 1,6). Derselbe Hochpass hebt den leisen Anschlag im hohen Diskant — darum bessert S2 den
Umfang des Tons. **Ein Regler, zwei Zielgrößen gegenläufig.**

**α als Regler** (S2, ε so verkleinert, dass α × k; Q0, τ0, p wie S2; 32-fach):

| Satz | (1) Kontakt RMS log | (2) Umfang RMS dB | (3) Helligkeit RMS Bass / Mitte / Diskant | (3) C4 / C6 | (5) Auslenkung / Tab. IV |
|---|---|---|---|---|---|
| heute (ε „−“) | 0,33 | 18,7 | 0,25 / 0,39 / **0,61** | 0,33 / 0,71 | 0,32 … 0,74 |
| S2 (α × 1, belegt) | 0,28 | **6,5** | 0,26 / 0,38 / 1,03 | 0,15 / 1,67 | 0,31 … 0,64 |
| α × 0,3 | **0,24** | 12,8 | 0,25 / **0,28** / 0,73 | 0,21 / 1,32 | 0,32 … 0,67 |
| α × 0,1 | 0,27 | 16,9 | 0,24 / 0,35 / **0,55** | 0,28 / 0,98 | 0,33 … 0,70 |
| α × 0,03 | 0,31 | 17,1 | 0,25 / 0,38 / 0,59 | 0,31 / 0,68 | 0,33 … 0,73 |

(Bereiche: 21–45, 48–69, 72–105; 63 / 56 / 57 Paare Taste × Teilton.) Kein α trifft Umfang und Helligkeit zugleich.
Je Taste (Modell − *Salamander*, Mittel n = 2 … 8): S2 liegt ab Taste 66 durchweg zu dunkel im Zuwachs (−0,4 … −1,6,
am stärksten 78–84), heute 63–81 zu hell (+0,3 … +0,6) und **ab 84 in allen Sätzen zu dunkel** (−0,2 … −0,7) — dort
fehlt dem Modell in jeder Fassung etwas. Der Bass hängt an keinem Satz (0,24 … 0,26). Die Kontaktzeit an C6 pp bleibt
in allen Sätzen zu kurz (1,3 … 1,7 ms gegen 2,5 bei Askenfelt & Jansson), die Auslenkung bei 30 … 75 % von Tab. IV.

**Die Zielgröße (2) ist im hohen Diskant nicht sicher.** `pegelumfang` ist der Pegel des **ganzen** Signals der ersten
0,3 s (v16 gegen v1, `Fluegelmessung.cpp`), nicht der Saite: Anschlaggeräusch, Raum und Rauschen der Aufnahme zählen bei
v1 mit, und Teilton 1 ist dort „kaum messbar“ (Kommentar am Code, Befund S152). Die Werte 96–108 springen (28,7 / 35,2 /
24,9 / 38,9 / 23,6 dB) — so springt kein Hammersatz. Das Modell misst dagegen die reine Saite. Der Vorteil von S2 in (2)
kommt ganz aus 87–108 (21–84 liegen heute und S2 auf 0,5 dB gleich); ob er echt ist, zeigt erst ein Umfang **an den Teiltönen** von *Salamander* (Archiv auf E:).

**Rechenzeit 32-fach.** 1 s Ton, S2, kleinstes von fünf: Bass +5 … +10 % (Taste 21: 32,0 → 35,2 ms), Mitte +5 %, oben
bis +33 % auf 1 ms. Gegen die Grenze 4,6 % im Bass (in Ruhe heute 3,9 … 4,2 %) wird es knapp; gültig erst nach dem Tausch
der Kühlung.

**Stab.** Nur die Celesta nimmt den Stulov-Filz (Taste 88, `stab/Vorgaben.cpp`), mit 16-fachem Kontakt (1,3 µs > τ0
1,8 µs ist nicht konvergiert). Sie hängt an derselben Entscheidung; gerechnet erst im Umbau.

## 7 HS-2 a: der Umfang an den Teiltönen (S187)

Entscheid **HS-2 a** (Patrik S187): vor der Wahl des Satzes den Umfang von *Salamander* an den Teiltönen nachmessen.
Messfall `LUMIVIZ_HS2_SAL_UMFANG=<datei.md>` mit `LUMIVIZ_SALAMANDER` in `test_LoopSalamander.cpp` (35 s): je Taste und
Ebene, je Kanal der Pegel des ganzen Signals der ersten 0,3 s (wie `pegelumfang`), die **Summe der Teiltöne 1 … 8**
zu Beginn (`anfangDb`), Teilton 1, das Rauschen vor dem Einsatz; schreibt die Umfänge je Taste nach `<datei>.txt`. Im
Messfall HS-1 misst `LUMIVIZ_HS1_SAL_TEILTOENE=<datei.txt>` das Modell **auf dieselbe Weise** (`messeKanal`, ppp 0,45
→ ff 5 m/s) und vergleicht.

***Salamander*, v16 gegen v1, dB:**

| Taste | 72 | 78 | 84 | 87 | 90 | 93 | 96 | 99 | 102 | 105 | 108 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ganzes Signal | 21,4 | 22,5 | 21,8 | 25,6 | 27,8 | 25,2 | 28,3 | 33,9 | 24,5 | 38,7 | 22,0 |
| Teiltöne 1 … 8 | 21,9 | 22,7 | 23,7 | 28,1 | 31,7 | 26,0 | 29,7 | 38,7 | 34,2 | 54,4 | 38,7 |

Bis Taste 78 gleich; 84–96 an den Teiltönen 2 … 4 dB mehr, 99–108 5 … 17 dB mehr — dort trägt bei v1 das Geräusch
neben den Teiltönen (108: ganz −42,5, Teiltöne −56,4 dB). An 105 und 108 liegt v1 nahe am Rauschen vor dem Einsatz
(−55 / −48 dB), der Wert ist dort eine Untergrenze. Bis 81 liegen ganzes Signal und Teiltöne im ganzen Umfang auf wenigen
dB gleich (Ausreißer 48: v1 links nur teilweise erkannt).

**Modell gleich gemessen** (32-fach): RMS an den Teiltönen über alle 30 Tasten / ab Taste 87 (8 Tasten):

| Satz | Teiltöne, alle | Teiltöne ab 87 | ganzes Signal 0,3 s |
|---|---|---|---|
| heute (ε „−“) | 13,5 | 25,3 | 4,8 |
| S1 | 5,4 | 7,5 | 5,1 |
| **S2** | **5,4** | **7,4** | 5,1 |
| α × 0,3 | 9,6 | 17,1 | 4,9 |
| α × 0,1 | 12,8 | 23,9 | 4,8 |
| Probe ε = 0,997 | 5,4 | 7,3 | 5,2 |

**Ergebnis.** Der Vorteil von S2 in (2) ist echt: an der Saite selbst ist der leise Anschlag ab Taste 87 heute
um 5 … 48 dB zu leise (90: 46 statt 32, 102: 59 statt 34, 108: 87 statt 39), mit S2 zwischen −4 und +9 dB (nur 105:
−14, dort ist der Wert von *Salamander* unsicher). Auch das Modell trägt
beim leisen Anschlag Energie neben den Teiltönen (den Puls des Kontakts selbst): am ganzen Signal gemessen liegen alle
Sätze bei 5 dB — die alte Fassung von (2) hat den Unterschied darum noch unterschätzt. Nach dem Kipppunkt der Vorlage
HS-2 gilt damit S2; die fehlende Helligkeit je Stärke im Diskant (§6) bleibt eigener offener Punkt.

## 8 HS-3 a: der belegte Satz im Kern (S187)

Entscheid **HS-3 a** (Patrik S187). `stulovHammerHysterese` rechnet ε = 0,9894 + 0,000088 N, `stulovHammerSteife`
Q0 = 15 500·e^(0,059 N)·(1 − ε) (Kartofelev & Stulov 2014, Gl. 5 und 7; Register `kartofelev2014`); Masse, p, τ0 bleiben
nach Stulov & Kartofelev 2009. `kHammerUeberabtastung` 32, Kern `hammerString` Rechenfassung 2. Celesta (Kern `bar`,
Taste 88) mit 32-fach, Cimbalom (Deluxe) über dieselben Formeln. Neu geschrieben: `grand_piano`, `upright_piano`,
`honky_tonk_piano`, `electric_grand_piano`, `celesta` (übrige Kern-Dateien nach dem Stimmen Byte für Byte gleich);
IC-Abdrücke mit Vermerk (Journal J65), `hammer_bezug_windows.txt` (unter Release-clang und Testing-MSVC gleich);
Pegel auf den Bezug (Flügel 4,2418 → 4,4916, Upright 1,8787 → 1,9779, Honky-tonk 2,3097 → 2,4356, CP-70 2,5073 → 2,6044,
Celesta 1,4618 → 1,4287; `alt = neu`); Lautstärke dreier Loops (LV1 a); `loop_werte.txt` (25 Zeilen),
`klangfassung.txt`; Hörlisten `klasse1_34i` (72 Klänge / 64 Punkte — die drei Klänge ohne Ton klingen jetzt) und
`stab_34g` (94 / 88), beide gültig.

**Folgen am Wächter.** Die Celesta-Spur *Half damper* (Taste 84, 300 ms) unterscheidet sich nur noch um −42,6 dB vom
normalen Ton (vorher −37,8; *Damper off* −29,3 statt −24,5): der Anschlag trägt mehr Energie, der Nachklang anteilig
weniger. `test_LoopHammerTon` prüft die Helligkeit an C6 nicht mehr (HS-4).

**HS3-1 a** (Patrik S187): die Spielweisen der Celesta im Beispiel-Loop an Taste 72 statt 84 — der Dämpfer wirkt dort
(`LUMIVIZ_IL2_PROBE=celesta`, Half damper / 300 ms: 60 −29,0, 72 −34,2, 84 −42,6, 96 −61,3 dB; der Dämpfer der Celesta
wirkt nach oben immer weniger, die hohen Stäbe klingen ohnehin schnell ab). `make_modelled_loops.py`
(`NOTE_DER_SPIELWEISEN`), `celesta.lvloop` (drei Noten), Spitze unverändert, eine Zeile `loop_werte.txt`, Klangfassung.
**HS3-2 a:** die Hörliste `pv1` ist als Stand vor HS-3 abgeschlossen (`hoertest/README.md`).

## 9 HS-4: wo die Helligkeit verloren geht (S187, Zwischenstand)

Recherche `recherche/2026-11-11_S187_helligkeit_je_staerke.md`. Eigene Rechnungen (Skripte unter `proben/2026-11-11_S187_hs4_wand_saite.py` und
`proben/2026-11-11_S187_hs4_wellenleiter_gesetze.py`; die Probe in Schichten nur im Scratchpad S187; Filz wie im Kern, Steigung am Kraftpuls bei n·f1 wie an *Salamander*), C6:

| Filz | Wand | unendliche Saite (3) | endliche ideale Saite (Wellenleiter, x_H 0,12) | Kern (§6) |
|---|---|---|---|---|
| S2 | 0,82 … 1,47 | 0,56 … 1,97 | −0,05 … 0,67 | −0,25 … 0,04 |
| vor HS-3 | 0,90 … 2,73 | 0,74 … 2,64 | 0,28 … 1,61 | 0,29 … 1,56 |

*Salamander* C6: 0,73 … 2,55. **Der Kern rechnet richtig**: Auch die exakte endliche Saite ohne Modenschnitt wird mit
S2 kaum heller. Der Verlust liegt in der Kopplung des zähen Filzes an die endliche Saite (Reflexionen vom kurzen Ende
während des Kontakts von 1–3 Perioden), nicht im Filz allein. Ein steilerer oder flacherer Filz (p ± 0,5 … +1, Q0 bei
0,5 mm gleich) ändert den Diskant nicht (RMS 0,99 … 1,06). Ein Filz in Schichten (Zähigkeit an der Oberfläche wie S2, im
Kern 3 µs, d0 0,05 … 0,4 mm; Voigt-Form am Wellenleiter, Probe) auch nicht: mit der Zähigkeit von S2 drückt der Hammer
bei ff kaum tiefer ein als bei ppp (0,24 gegen 0,21 mm). Zwei Kennzahlen zeigen, was fehlt: die Kontaktzeit an C6 pp /
ff ist am Flügel 2,5 / 0,85 ms (Verhältnis 2,9; Askenfelt & Jansson), im Modell 1,32 / 0,98 (1,35); T2 − T1 an C6 bei
*Russell* pp −19,8, ff −6,7 dB, im Modell (Ton) −15 bzw. −16 dB — der leise Anschlag ist zu kurz, der laute zu dunkel.

**HS-4 a: zwei Gesetze am Wellenleiter** (`proben/2026-11-11_S187_hs4_wellenleiter_gesetze.py`, Saite nach den Kurven der Vorgabe, C6; *Salamander* RMS-Ziel 0,
Umfang an den Teiltönen 23,7, Askenfelt 2,5 / 1,1 / 0,85 ms, Russell T2 − T1 pp → ff +13,1 dB):

| Gesetz | Helligkeit RMS | Umfang | Kontakt pp / mf / ff | T2 − T1 pp → ff |
|---|---|---|---|---|
| S2 | 1,83 | 34,0 | 1,34 / 1,18 / 1,02 | −3,9 |
| S2 + zweite Relaxation, k2 = 1 … 10, T_R = 0,1 … 1 ms | 1,68 … 1,81 | 26,6 … 32,8 | 1,21 … 1,32 / … / 0,97 … 1,04 | −2,6 … +7,3 |
| Zähigkeit ∝ (v_ref/|u̇|)^q, q = 0,5 / 1, v_ref = 0,5 / 2 m/s | 2,03 … 2,08 | 26,3 … 28,6 | 1,26 … 1,58 / … / 1,04 … 1,08 | −2,0 … +8,2 |

**Keines trägt**: die Helligkeit bleibt um 1,7 … 2,1 RMS, das Verhältnis pp/ff der Kontaktzeit um 1,2 … 1,5. Die zweite
Relaxation hebt den leisen Anschlag (Umfang −7 dB) und T2 − T1 (bis +7 dB), nicht die Steigung. **Nebenbefund:** an den
Tasten 96 und 108 trifft S2 die Kontaktzeiten schon (0,95 / 0,75 / 0,60 gegen 0,9 / 0,65 / 0,55 ms; 0,74 / 0,55 / 0,38
gegen 1,2 / 0,55 / 0,45) — die Kennzahl „pp zu kurz“ ist vor allem C6 pp (2,5 ms, abgelesen, Geschwindigkeit der
Abbildung nicht genannt). Die Zielgröße Helligkeit selbst ist nicht nachgeprüft wie der Umfang in HS-2 a: an *Salamander*
zählen je Teilton nur die Ebenen, in denen er gefunden ist (an C6 bei v1 13 von 16 Teiltönen beider Kanäle), die
Steigung gilt je dB Grundton, und der Grundton wächst im Modell an C6 um 2,1 dB je dB Stärke. Die nicht erreichten
Originale stehen in `Bibliothek_offen.md` (Abschnitt S187).

**HS4-1 a: die Zielgröße nachgeprüft** (Messfall `LUMIVIZ_HS4_SAL_HELL=<datei.md>` mit `LUMIVIZ_SALAMANDER` in
`test_LoopSalamander.cpp`; im Modell `LUMIVIZ_HS1_HELL_SUMME=1`). Helligkeit an *Salamander* n = 2 … 8 in vier Fassungen:

| Taste | A wie V1 a | B je dB Summe der Teiltöne | C nur Teiltöne in allen Ebenen | D nur v6 … v16 |
|---|---|---|---|---|
| 60 | 0,17 … 1,90 | 0,17 … 1,67 | wie A | 0,03 … 1,54 |
| 72 | 0,26 … 1,67 | 0,25 … 1,62 | wie A | 0,26 … 1,07 |
| 84 | 0,72 … 2,52 | 0,68 … 2,34 | 0,72 … 1,98 (n ≤ 5) | 0,77 … 2,75 |

**Die Zielgröße trägt**: Bezug, Auswahl der Ebenen und Teiltöne verschieben sie um höchstens ein Drittel, das
Hellerwerden bleibt in jeder Fassung. Je Ebene an C6 (Mittel der Kanäle, Teilton n gegen 1): v1 −25,0 / −33,5 / −54,0
(n = 2 / 3 / 4), v16 −6,3 / −19,0 / −13,5 dB, der Grundton steigt dabei um 23 dB. Das Modell (S2, Diagnose §6) liegt bei
ppp bei −15 / −20 / −24 dB, bei ff bei −16 / −23 / −19 — **zu hell im Leisen, zu dunkel im Lauten**. Ein Filz, der im
Leisen einen dunklen Puls mit genug Grundton gibt und im Lauten einen hellen, ist mit keinem geprüften Gesetz gefunden.
Nach dem Kipppunkt von HS-4 ruht der Punkt bis zu den Originalen (`Bibliothek_offen.md`); die Pegel je Ebene sind die
schärfere Zielgröße für den nächsten Anlauf.

## 10 Offen

- **HS-4:** die Helligkeit je Stärke ab Taste 66 zu klein, ab 84 in jedem Satz — ein Mechanismus fehlt dem Modell;
  danach eine neue Flügel-Liste statt `pv1`.

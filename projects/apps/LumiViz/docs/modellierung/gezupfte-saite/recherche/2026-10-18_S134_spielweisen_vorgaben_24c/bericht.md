# Recherche 24c — Vorgaben für Up level, Mute, Tap level, Touch

Stand 18.10.2026. Nur gelesen und gemessen, im Repo nichts geändert. Gelesen: `Saiten_Konzept.md`
§1, §3 (S-K3 a, S-W1 a bis S-W7 a), §4b, §5c/5d/5m, §6/§6b; die Platzhalter stehen in
`include/loop/Instrument.hpp:363–369` (0,8 / 0,6 / 0,65 / 0,3), *Spread* ist laut
`include/loop/Spielweise.hpp:58` die Zeit **von der ersten bis zur letzten Saite**.

**Belegarten in diesem Bericht**

- **gelesen** – in der Textebene der Quelle selbst gefunden (pdftotext / `quelle_suchen.py`), mit
  Seite und kurzem Wortlaut.
- **gemessen** – von mir an freien Aufnahmen gemessen (IDMT-SMT-Guitar, IDMT-SMT-Bass), mit
  Skript und Rohtabelle im Scratchpad. Das ist kein Literaturwert, sondern meine Auswertung fremder
  Aufnahmen. Die Grenzen stehen jeweils dabei.
- **abgeleitet** – gerechnet oder simuliert, die Rechnung steht dabei.
- **über Werkzeug** – nur aus einer WebFetch-/Suchantwort, nicht selbst im Wortlaut gelesen. Das
  ist kein Beleg.

---

## 0. Reichweite der Suche

**Suchbegriffe** (Websuche, Standard und erweitert): palm mute + physical model / waveguide /
DAFx / ICMC / JASA; palm muting decay time / spectral centroid / hexaphonic; Pakarinen
flageolet; guitar harmonics finger contact damping node; Evangelista Eckerholm touch;
strumming microtiming / inter-string delay; downstroke upstroke amplitude / polarization /
picking direction; up-strum down-strum; hammer-on / tapping amplitude; Zollner Hammer-on /
Tapping / Flageolett; Datensätze mit Spieltechniken (IDMT, GPT, Guitar-TECHS, GOAT).

**Selbst gelesen (PDF in `dl/`):**

| Kürzel | Quelle |
|---|---|
| Biral14 | J. Biral, N. d'Alessandro, A. Freed, „Towards a Dynamic Model of the Palm Mute Guitar Technique Based on Capturing Pressure Profiles Between the Guitar Strings", ICMC/SMC 2014, S. 1483 ff. http://www.icmc14-smc14.net/images/proceedings/PS4-B10-TowardsaDynamicModel.pdf |
| Rebour12 | L. Reboursière et al., „Left and right-hand guitar playing techniques detection", NIME 2012. https://nime.org/proceedings/2012/nime2012_213.pdf |
| Abesser13 | J. Abeßer, P. Kramer, C. Dittmar, G. Schuller, „Parametric Audio Coding of Bass Guitar Recordings Using a Tuned Physical Modeling Algorithm", DAFx-13. https://dafx.de/paper-archive/2013/papers/24.dafx2013_submission_9.pdf |
| Pakar05 | J. Pakarinen, „Physical Modeling of Flageolet Tones in String Instruments", EUSIPCO 2005. https://research.spa.aalto.fi/publications/papers/eusipco-flageolets/Pakarinen05Eusipco.pdf |
| Bilbao14 | S. Bilbao, A. Torin, „Numerical simulation of string/barrier collisions: the fretboard", DAFx-14. https://www.dafx14.fau.de/papers/dafx14_stefan_bilbao_numerical_simulation_of_s.pdf |
| Bilbao24 | S. Bilbao, R. Russo, C. J. Webb, M. Ducceschi, „Real-Time Guitar Synthesis", DAFx-24. https://www.dafx.de/paper-archive/2024/papers/DAFx24_paper_50.pdf |
| Freire18 | S. Freire, A. Armondes, J. Viana, R. Silva, „Strumming on an Acoustic Nylon Guitar: Microtiming, Beat Control and Rhythmic Expression in Three Different Accompaniment Patterns", SMC 2018, S. 544–549. https://zenodo.org/records/1422685 |
| Barb14 | I. Barbancho et al., „Estimation of the Direction of Strokes and Arpeggios", ISMIR 2014. https://archives.ismir.net/ismir2014/paper/000130.pdf |
| Laur10 | M. Laurson, V. Välimäki, H. Penttinen, „Simulating Idiomatic Playing Styles in a Classical Guitar Synthesizer: Rasgueado as a Case Study", DAFx-10. https://dafx.de/paper-archive/2010/DAFx10/LaursonValimakiPenttinen_DAFx10_P20.pdf |
| Zoll-1.5 | M. Zollner, *Physics of the Electric Guitar*, Kap. 1.5 „The plucking process" (Übers. T. Zwicker 2020). https://gitec-forum-eng.de/wp-content/uploads/2020/08/poteg-1-5-picking-process.pdf |
| Zoll-7.7.5 | dto., Kap. 7.7.5 „Damping by finger, hand and capo". https://gitec-forum-eng.de/wp-content/uploads/2019/08/poteg-7-7-5-finger-hand-capo-damping.pdf |
| Murgul25 | S. Murgul, J. Schimper, M. Heizmann, „Joint Transcription of Acoustic Guitar Strumming Directions and Chords", ISMIR 2025. https://arxiv.org/pdf/2508.07973 |
| IDMT-G | A. Eppler, C. Kehling, A. Männchen, *IDMT-SMT-GUITAR Dataset Description* (2016) und Datensatz, Zenodo 10.5281/zenodo.7544110, CC BY-NC-ND 4.0 |
| IDMT-B | J. Abeßer, *IDMT-SMT-Bass* (Description v1.1, 2011) und Datensatz, Zenodo 10.5281/zenodo.7188892, CC BY-NC-ND 4.0 (Zenodo; die Description-PDF nennt „by-sa") |

**Geprüft, ohne Treffer zur Frage:** Zollner 1.5.2 (Plektrum), 1.6.2 (räumliche Schwingung) und
7.7.5 (die Hand am Hals, kein Palm mute). Das Inhaltsverzeichnis von Zollners Buch auf
gitec-forum-eng.de hat keinen Abschnitt zu Hammer-on, Tapping, Flageolett oder Palm mute. In
Bilbao24 und Murgul25 stehen keine Pegelangaben. Ein NIME-2017-Papier (nime2017_paper0030)
habe ich geladen, aber nicht ausgewertet.

**Nicht erreicht oder nicht frei:** Laurson/Erkut/Välimäki/Kuuskankare, *CMJ* 25(3) 2001
(Project MUSE). Evangelista/Eckerholm, IEEE TASLP 2010. Evangelista, *JASA* „Models of the
player's interactions with strings" (10.1121/10.0023672). Kramer/Abeßer et al., ICASSP 2012
(das Bass-Waveguide-Modell mit „damping point"). Abeßer et al., ICASSP 2010. Die numediart
GuitarDB aus Rebour12.

**Nicht gefunden** (kein Treffer ist kein Beweis):

- eine Literaturangabe zur T60 oder Abklingrate bei Palm mute in Zahlen;
- eine gemessene Pegeldifferenz Up- gegen Downstroke je Saite;
- ein gemessener Pegel von Tapping oder Hammer-on gegen einen gezupften Ton;
- eine Angabe, wie lange der Finger beim Flageolett auf dem Knoten liegt.

Für Palm mute und Flageolett habe ich die Lücke durch eigene Messung an IDMT-Aufnahmen
geschlossen (§1, §4). Für Up/Down und Tapping bleibt sie offen.

---

## 1. Palm mute → Regler *Mute*

### 1.1 Literatur (gelesen)

- **Lage der Hand:** Die Hand liegt am Steg, auf den ersten Zentimetern der Saite. Biral14
  S. 1485 beschreibt ein Druckprofil „along the first few centimeters of each string". Zu Abb. 5
  (S. 1486):
  „the area between the bridge and the bridge pickup must be sensed". Beim Strat-Steg-Tonabnehmer
  sind das ≈ 41 mm (§6b, Errede).
- **Ort vor Kraft:** Biral14 S. 1488 (Fazit): die Entfernung der Hand vom Steg „is more important
  than the applied force".
- **Wirkung:** Rebour12 S. 3 (§6.1): Ein gedämpfter Ton „is shorter and decreases faster", und
  „high frequencies decrease faster than other part of the spectrum". Das ist nur qualitativ.
- **Bass:** Abesser13 S. 1 definiert MU so: Die Saite wird mit dem Daumen angeschlagen, „while
  simultaneously damping the vibrating string by using the inner hand side". Im Modell stellt
  der Text dafür die „damping modules" ein (S. 3, §3.1). Zahlen nennt er nicht.
- **Modell:** Pakar05 (Abb. 1, Gl. 2–3) koppelt einen Wave-Digital-Widerstand über einen
  3-Port-Serienknoten an den Waveguide. Gedacht ist das für Flageoletts, physikalisch ist es
  derselbe Punktdämpfer wie eine Hand am Steg. Mit Z1 = Z2 = Z0 und Z3 = R:
  Reflexion −ρ/(2+ρ), Durchgang 2/(2+ρ), ρ = R/Z0.

### 1.2 Eigene Messung an IDMT-SMT-Guitar (gemessen)

**Material.** Datensatz 2, Init-Aufnahmen `<G>_<Saite>_muted5.wav` (fünf Töne MU je Saite, alle
am 5. Bund) gegen `<G>_<Saite>_fret_0-20.wav` (PK, Bund 0–20, je ≈ 1 s).

- Gitarren: Aristides 010, Fender Stratocaster, Gibson Les Paul; je sechs Saiten, also
  18 Saiten.
- Aufnahme direkt am Line-In (IDMT-G S. 7).
- Die Beschreibung erklärt MU nicht weiter. Sie nennt nur „muted" (IDMT-G S. 10).

**Befund: fünf Stufen.** In jeder der 18 Dateien nimmt die Dämpfung vom 1. zum 5. Ton
monoton zu. Ich nenne die Töne deshalb M1 bis M5. Dass das eine gewollte Abstufung ist, steht
nirgends. Das ist meine Lesart der Messung.

**Methode.** STFT 2048/256, je Teilton k eine Regression des Pegels über die Zeit,
T60 = 60/|dB/s|. Die normale Referenz habe ich aus Datensatz 1 genommen (Strat und Ibanez, Noten
am 5. Bund, 2,3 s lang), weil die Fenster der `fret_0-20`-Töne für eine T60 von mehreren
Sekunden zu kurz sind.

**T60 bei ≈ 200–260 Hz** (D- und G-Saite am 5. Bund: 196 / 262 Hz). Je Zelle der Median über
6 Werte (3 Gitarren × 2 Saiten):

| | normal (ds1, 4 Werte) | M1 | M2 | M3 | M4 | M5 |
|---|---|---|---|---|---|---|
| Grundton k=1 | **8,7 s** (7,5–10,3) | 7,3 s (2,0–21) | **2,16 s** (1,7–2,9) | **0,98 s** (0,55–1,15) | **0,46 s** (0,31–0,76) | **0,28 s** (0,19–0,62) |
| Verhältnis zu normal | 1 | ≈ 0,84 | 0,25 | 0,11 | 0,053 | 0,032 |
| Teiltöne k=4..8 (≈ 0,8–2,1 kHz) | 3,1 s (2,3–3,7) | 0,82 s | 0,38 s | ≈ 0,2 s | ≤ 0,15 s* | ≤ 0,15 s* |
| Verhältnis zu normal | 1 | 0,26 | 0,12 | ≈ 0,07 | ≤ 0,05 | ≤ 0,05 |

\* An der Messgrenze. Mit dem 46-ms-Fenster und dem Abbruch bei 40 dB liegen viele Werte bei
0,09–0,12 s. Schneller lässt sich so nicht messen.

**Weitere Befunde (gemessen):**

- **Der Spitzenpegel bleibt.** MU gegen normal liegt bei −4 bis +6 dB, im Median ≈ 0 dB. Eine
  Ausnahme ist die tiefe E-Saite der Les Paul mit −10 bis −14 dB. Die Energie der ersten 0,5 s
  fällt dagegen von ≈ 0 dB (M1) auf −8 bis −22 dB (M5). Die Hand nimmt also nicht den Anschlag
  weg, sondern das Ausklingen.
- **Leicht gedämpft trifft vor allem die Höhen.** Bei M1 bleibt der Grundton fast unverändert,
  während k=4..8 auf ≈ ¼ fällt.
- **Tonhöhe:** MU gegen normal liegt bei −10 bis +11 Cent ohne gemeinsame Richtung. Eine
  Verstimmung muss das Modell nicht nachbilden.
- **Lage der Hand aus dem Abklingen** (abgeleitet, `fit_daempfer.py`). Ich habe je Note
  σ_k = σ_k,normal + 2·f1·ρ·sin²(k·π·ξ) an die Teiltöne k=1..10 angepasst. Für M1/M2 ergibt das
  über 36 Noten ξ ≈ 0,083 bis 0,086, also **≈ 40 mm vom Steg** (Spanne 5–96 mm, schwingende
  Länge am 5. Bund ≈ 485 mm). ρ liegt im Median bei 0,021 (M1) und 0,048 (M2). Ab M3 wird der Fit
  unsicher, weil die hohen Teiltöne an der Messgrenze liegen. Die 40 mm passen zu Biral14
  („first few centimeters", zwischen Steg und Steg-Tonabnehmer).

### 1.3 E-Bass, IDMT-SMT-Bass (gemessen)

Material: drei Bässe (Fame Baphomet, Warwick, Yamaha BB604), neck+bridge, MU gegen FS,
Saiten 1–4, Bund 0 und 5. Vier Paare mit einer Pegeldifferenz über 10 dB habe ich verworfen,
weil der Einsatz dort nicht sicher erkannt wurde. Es bleiben 20 Paare.

- **Grundton:** T60 MU/FS im Median **0,44** (0,27–0,76). Absolut liegt MU bei **4,6–8 s**,
  FS bei 9–18 s.
- **Teiltöne k=4..8:** Verhältnis im Median **0,09**.
- **Folgerung:** Der IDMT-Bass-„muted" ist am Grundton eine leichte Dämpfung (wie M1–M2 der
  Gitarre), aber der Bass klingt dabei noch 5–8 s. Das bestätigt das Argument aus S-W6 a, dass
  ein Bass gedämpft länger klingt als eine Gitarre.

### 1.4 Modell und Vorgabe (abgeleitet)

**Physik: ein Punktdämpfer.** Ein Punkt-Dashpot R an der Stelle x vom Steg dämpft die Mode k
mit der Rate

  σ_k = R·sin²(kπx/L) / (μL) = **2·f1·ρ·sin²(kπ·x/L)**,  ρ = R/Z0.

Hergeleitet aus modaler Masse μL/2 und Dämpfung R·φ_k(x)². Mit Z0 = μc und c = 2L·f1 folgt die
zweite Form. Meine Waveguide-Simulation (`sim_daempfer.py`) bestätigt die Formel für ρ ≤ 1 am
Grundton auf 8 %.

**Was die Formel ohne weiteren Regler liefert:**

1. Die Höhen werden zuerst gedämpft, weil sin² bis k ≈ L/(2x) wächst. Bei x = 40 mm und
   L = 648 mm ist das k ≈ 8.
2. Der Bass klingt gedämpft länger, weil f1 kleiner und x/L kleiner ist. Bei gleichem ρ
   bekommt A1 auf einem 864-mm-Bass nur **≈ 1/7** der Zusatzdämpfung, die A3 (220 Hz) auf einer
   648-mm-Gitarre bekommt: 55·sin²(π·40/864) / (220·sin²(π·40/648)) = 0,14.
3. Der Anschlag bleibt so laut wie gemessen.

**Vorschlag für den Regler:** **Mute = m ∈ [0, 1], ρ = m², Hand fest bei 40 mm vom Steg.**
Der Regler verstellt genau eine Größe, wie S-K3 a es verlangt.

- **Waveguide:** der Pakar05-Knoten an dieser Stelle; bei einem gegriffenen Ton liegt x
  innerhalb der schwingenden Länge.
- **Extended KS:** dieselbe Zusatzrate σ(f) = 2·f1·ρ·sin²(π·f·x/(f1·L)) im Umlauffilter.

**Die gemessenen Stufen in m** (Rückrechnung bei 220 Hz, x/L = 40/485, normal 8,7 s):
M1 ≈ 0,07, M2 ≈ 0,29, M3 ≈ 0,47, M4 ≈ 0,71, M5 ≈ 0,92. Mit ρ = m² liegen die fünf Stufen also
gleichmäßig über den Regler. Bei ρ = m wären sie unten zusammengedrängt. Der IDMT-Bass-„muted"
entspricht m ≈ 0,47.

**Klingdauer neben dem Regler:**
T60(220 Hz) = 6,91 / (6,91/RingTime + 2·220·m²·sin²(π·40 mm/L)).

Mit Ring time 3,2 s und L = 648 mm: m = 0 → 3,2 s · 0,3 → 1,9 s · **0,6 → 0,86 s** ·
0,8 → 0,55 s · 1 → 0,37 s.

**Vorgabe: 0,6 bleibt.** Der Wert liegt zwischen M3 und M4. Belegt ist nur der Bereich M1–M5.
Welche Stufe „das übliche Palm mute" ist, sagt keine Quelle.

**Grenze des Modells (abgeleitet).** Bei ρ ≳ 0,3 sättigt der Punktdämpfer in der Simulation
für k ≥ 4 bei ≈ 0,4–0,7 dB je Grundperiode, weil er reflektiert statt zu dämpfen. Gemessen
fallen die hohen Teiltöne bei M3–M5 schneller (T60 ≤ 0,15–0,2 s). Die echte Hand liegt flächig
auf mehreren Zentimetern (Biral14). Für Extended KS ist die Formel deshalb näher an der Messung
als der Punkt-Knoten. Für den Waveguide wären zwei oder drei Dämpferpunkte zwischen 20 und 60 mm
näher, als eigene Frage.

---

## 2. Upstroke gegen Downstroke, Strum → Regler *Up level* und *Spread*

### 2.1 Belege

**Versatz der Saiten** (gelesen, Freire18, Nylongitarre, hexaphoner Tonabnehmer):

- Table 6, S. 547:
  - 6-note strummed, adagio: Abstand benachbarter Saiten **8–11 ms**, ganzer Akkord **37–46 ms**;
  - 6-note strummed, moderate: **5–12,5 ms** bzw. **26–62 ms**;
  - 6-note Arpeggio: 16–40 bzw. 64–202 ms;
  - Blockakkorde: 0–6 bzw. 5–18 ms.
- Table 5, S. 547 (Wonderwall, ganzer Akkord in ms):

  | Akkord | WW1 | WW2 | WW3 |
  |---|---|---|---|
  | 6 Saiten | 26 | 39 | 62 |
  | 5 Saiten | 36 | 40 | 60 |
  | 4 Saiten | 23 | 26 | 42 |
  | 3 Saiten | 16 | 21 | 23 |

- Table 3, S. 546: gestrummte Akkorde mit 3 Tönen haben einen Saitenabstand von 11–23 ms.
- Die Grenzen der Messung nennt Freire18 S. 545: Einsätze automatisch „within a margin of error
  of 10 ms", danach von Hand am Signalverlauf nachgezogen.

**Gleichmäßiger Abstand:** Barb14 S. 3 begründet ihn: „because of human physiology", der
Anschlag laufe mit fester Geschwindigkeit.

**Achtung, Benennung:** Barb14 S. 1 nennt den Lauf „from the lowest to the highest pitch"
*upstroke*. Gitarristen nennen das Downstroke. Wer dort Zahlen übernimmt, muss die Namen
tauschen.

**Up trifft weniger Saiten:** Freire18 S. 547 schreibt zu den Up-Akkorden in Wonderwall, ein
Spieler „used all six strings …, while the others used much fewer". Abgeleitet aus Table 1,
S. 545 (Experiment 1, durchgehend Down-Up): Die tiefe E-Saite wurde in B1/B2 nur 13–14-mal
angeschlagen, die anderen Saiten 25–32-mal. Die tiefe Saite fehlt also etwa in jedem zweiten
Anschlag, vermutlich in den Up-Anschlägen. Das steht dort nicht wörtlich.

**Rasgueado:** Laur10 S. 2 findet 24 Einsätze (4 Finger × 6 Saiten) zwischen 30 und 330 ms.
Abgeleitet sind das im Mittel ≈ 12,5 ms je Einsatz, mit überlappenden Fingern.

**Pegel und Klang** (gelesen, Zoll-1.5):

- S. 1-36: Die Querkräfte beim Anschlag erreichen „often … 5 N (or even 10 N occasionally)".
- S. 1-40 (Abb. 1.39, Strat, leicht bis sehr stark): „the level-difference … is no less than
  39 dB at 4 kHz, the fundamental changes only by 7 dB". Ein schwächerer Anschlag ist also vor
  allem dunkler, nicht nur leiser.
- S. 1-36: Der Spieler formt den Ton über Stärke und Richtung, „both pressing-down and
  lifting-up of a string happen".

**Druck der Hand:** Biral14 S. 1487 findet bei Up und Down dieselben drei Phasen im Druckprofil
und schreibt, sie zu unterscheiden „requires further analysis". Eine Pegelzahl nennt er nicht.

**Nicht gefunden:** ein gemessener Pegelunterschied Up gegen Down je Saite. Die Treffer dazu
waren Lehrseiten, die sich auch noch widersprechen. Murgul25 nutzt für das Training gerechnete
Aufnahmen aus einer Sample-Bibliothek (S. 2 f.), das ist kein Beleg.

### 2.2 Vorschläge

**Up level: 1,0 statt 0,8.**

- *Herleitung:* Für eine geringere Kraft je Saite beim Up-Anschlag gibt es keinen Beleg.
  Belegt ist, dass Up weniger Saiten trifft und von oben beginnt. Das macht die Planung (Strum),
  nicht der Pegel.
- *Wer Up leiser stellt:* Er sollte wissen, dass ein schwächerer Anschlag nach Zoll-1.5 vor
  allem die Höhen verliert. Ein reiner Pegelfaktor bildet das nicht nach. Ob *Up level* auch
  *Pluck* mitnimmt, wäre eine eigene Frage, weil S-K3 a „eine Größe je Regler" verlangt.

**Vorzeichen** (abgeleitet). In einem linearen Saitenmodell ist das umgedrehte Vorzeichen bei
einer Saite in Ruhe nicht hörbar. Hörbar wird es nur, wenn der neue Anschlag auf eine noch
schwingende Saite addiert wird, also beim Down-Up-Spiel. Wenn die Wiedergabe den Zustand der
Saite bei jedem Ton neu setzt, wirkt es nicht. Das habe ich im Code nicht geprüft.

**Spread** (ganzer Akkord, so steht es in `Spielweise.hpp`): **40 ms statt 20 ms** für den
Akkord über sechs Saiten.

- *Herleitung:* 8 ms Saitenabstand × 5 Abstände = 40 ms, mitten in 37–46 ms (adagio) und
  26–62 ms (moderate).
- *Die 20 ms von heute:* Sie entsprechen bei sechs Saiten 4 ms je Saite, also schon fast einem
  Blockakkord. Bei drei Saiten sind 20 ms richtig (Table 5: 16–23 ms).
- *Offene Frage* (Entscheidungsvorlage unten): Die Messung spricht für einen festen
  Saitenabstand, also eine feste Handgeschwindigkeit, nicht für eine feste Gesamtzeit.

---

## 3. Tapping → Regler *Tap level*

### 3.1 Belege

**Kräfte und Zeiten aus Simulationen, nicht gemessen** (gelesen):

- Bilbao14 S. 7, Abb. 4: Finger mit 5 g Masse, „velocity 3 m/s".
- Bilbao14 S. 2: Ein Anschlag dauert „on the order of 1-10 ms".
- Bilbao24 S. 7: Ein Finger mit 0,9 N gegen einen Anschlag mit 5 N ergibt „similar to a finger
  tap sound".

Das sind Eingaben in Simulationen, keine Messwerte.

**Erkennung:** Rebour12 Tab. 1 erkennt Hammer-ons über das Spektrum. Ein Teil davon „do not show
any amplitude increase at all" (S. 2). Eine Pegelzahl gibt es nicht.

**Nicht gefunden:** ein gemessener Tapping- oder Hammer-on-Pegel gegen einen gezupften Ton.
Der Datensatz in *Data in Brief* 2023 (Tapping, Hammer-on) taugt nicht: Handy-Mikrofon nach einem
Verstärker-Simulator (über Werkzeug, PMC10698518).

### 3.2 Herleitung (abgeleitet, mit Annahmen)

**Anschlag.** Ein Dreieck mit der Spitze y0 bei β vom Steg hat die Energie
E_p = T·y0² / (2β(1−β)L).

**Tapping.** Der Finger drückt die Saite in der Zeit τ um die Höhe a über dem Bund auf den
Bund. Am neuen Ende läuft dann eine Rampe los mit der Energie E_t = T·a²/(c·τ), c = 2L·f1.

**Verhältnis:** E_t/E_p = **β(1−β)·a² / (f1·τ·y0²)**.

**Werte:**

- β = 0,13 (Vorgabe Pluck position);
- y0 ≈ 2 mm, weil 5 N oft Kontakt zum letzten Bund bei 2,1 mm Abstand bringen (Zoll-1.5
  S. 1-36, Abb. 1.35);
- a ≈ 1,5 mm Saitenlage (nicht belegt, übliche Einstellung);
- τ = a / 3 m/s ≈ 0,5 ms (Fingertempo aus Bilbao14, auch das nicht gemessen).

Damit gilt E_t/E_p ≈ 127/f1:

| f1 | 110 Hz | 220 Hz | 440 Hz |
|---|---|---|---|
| E_t/E_p | +0,6 dB | **−2,4 dB** | −5,4 dB |

**Spektrum.** Die Rampe ist nach τ geglättet, das wirkt wie ein Tiefpass um 1/(πτ) ≈ 0,6 kHz.
Das ist der „weiche Anschlag" aus S-W6 a. Der Anregungsort liegt am Bund, also am
sattelseitigen Ende der schwingenden Länge.

### 3.3 Vorschlag

**Tap level 0,75 statt 0,65**, bezogen auf 220 Hz wie Ring time.

- *Herleitung:* √(10^(−0,24)) = 0,76.
- *Gültigkeit:* Der Wert ist nur abgeleitet. Mit den Annahmen zu a und τ liegt die Spanne
  bei 0,5–1,2.
- *Frequenz:* Die Physik sagt, dass Tapping bei tiefen Tönen relativ lauter ist
  (Amplitude ∝ f1^−½). Ob der Regler das mitnimmt, ist eine Frage der Umsetzung.

**Besser als jede Ableitung:** am eigenen Instrument messen. *Record sample* aufnehmen, gezupft
gegen getappt am selben Bund, und den Pegel vergleichen (§5).

---

## 4. Natürliche und künstliche Flageoletts → Regler *Touch*

### 4.1 Literatur (gelesen)

- **Mechanismus:** Pakar05 S. 1: Der Spieler „gently damps the vibrating string", sodass „only
  those modes" klingen, die am Finger einen Knoten haben. Künstliche Flageoletts sind dasselbe
  am gegriffenen Ton (S. 1).
- **Modell:** Ein WD-Widerstand am Waveguide, statisch oder zeitlich veränderlich (Abb. 1, 2,
  Gl. 2–3). Im Beispiel (Abb. 3) sind Z1 = Z2 = 0,5 und „the WDF resistance is varied within
  [0..0.51]". Das entspricht ρ = R/Z0 bis ≈ 1. Der Dämpfer setzt dort bei 0,3 s ein (S. 4).
- **Bass:** Abesser13 S. 1–2 lässt die Greifhand die Saite an ganzzahligen Teilungen „softly
  dampen", Moden mit Bauch am Dämpfungspunkt „cannot be excited".
- **Erkennbarkeit:** Rebour12 S. 3 erkennt Flageoletts am Anschlag (Verhältnis Grundton zu
  „subharmonic at 1.5·F0") zu 100 %.

### 4.2 Eigene Messung (gemessen)

**Gitarre, IDMT-SMT-Guitar Datensatz 2** (`<G>_NH_XII/VII/V/IV/IX`), je 6 Saiten × 3 Gitarren.
Bezug ist der normale Ton der leeren Saite derselben Gitarre. Energie der ersten 0,5 s, Median
(Spanne):

| n (Bund) | Energie 0,5 s gegen leer | Spitze gegen leer | Spitze gegen gegriffen, gleiche Höhe |
|---|---|---|---|
| 2 (XII) | **−8,6 dB** (−11,5 … +4,2) | −5,2 dB | −8,2 dB (Bund 12) |
| 3 (VII) | **−9,0 dB** (−15,7 … +1,3) | −5,7 dB | −13,6 dB (Bund 19) |
| 4 (V) | **−10,5 dB** (−17,2 … −2,9) | −5,0 dB | — |
| 5 (IV, IX) | **−13,2 dB** (−21,2 … −1,9) | −3,7 dB | — |

Die Stratocaster weicht ab: Bei ihr ist das Flageolett etwa so laut wie die leere Saite, die
Spitze liegt sogar 2–4 dB darüber. Vermutlich wurde die Referenzdatei leiser gespielt, prüfen
lässt sich das nicht. Ohne die Strat liegen n = 2 bei ≈ −10 dB und n = 5 bei ≈ −15 dB.

**Bass, IDMT-SMT-Bass** (FS HA gegen FS NO Bund 0), Energie 0,5 s im Median: n = 2 −9,1 dB,
n = 3 −3,9 dB, n = 4 −7,4 dB.

**Theorie im Vergleich** (abgeleitet, `ha_energie.py`). Der Energieanteil der Moden mit Knoten
am Finger ist bei einem Dreieck an β = 0,1–0,25:

| n | 2 | 3 | 4 | 5 |
|---|---|---|---|---|
| Anteil | −3,5 … −4,8 dB | −5,9 … −9,6 dB | −7,8 … −12,1 dB | −9,6 … −14 dB |

Er geht gegen null, wenn der Anschlag selbst auf einem Knoten liegt. Die Reihenfolge stimmt mit
der Messung überein, gemessen sind es rund 3–5 dB weniger. Das ist plausibel: Wer Flageoletts
spielt, schlägt anders an, und dazu kommt der Tonabnehmer. Damit ist S-W8 a bestätigt: Die
Lautstärke folgt aus der Physik.

**Tonhöhe** (gemessen): Das Flageolett n = 5 liegt **−13 bis −16 Cent** unter der
gleichstufigen Note (Aristides, Bund IV). Theoretisch sind es −13,7 Cent. Das bestätigt S-W7 a.

**Wie schnell die übrigen Moden verschwinden** (gemessen, `ha_verlauf.py`). Ich habe den Pegel
des Grundtons der leeren Saite relativ zum Flageolett-Grundton verfolgt, für n = 2, 3, 4 auf den
Saiten 1–3. Zwischen 50 und 100–150 ms fällt er mit **≈ 330–720 dB/s**, das sind **≈ 4–6 dB je
Periode** der leeren Saite. Ab ≈ 150–200 ms liegt er bei −45 bis −75 dB, das ist vermutlich der
Grund des Messverfahrens. Die Werte bei 25 ms sind unbrauchbar, weil das Fenster über den
Einsatz reicht.

Wann der Finger abgehoben wird, lässt sich daraus nicht trennen. Der Flageolett-Grundton selbst
klingt danach so lange wie ein gegriffener Ton gleicher Höhe (T60 meist 5–15 s, gegriffen
4–20 s). Ein Finger, der liegen bliebe, dämpft also kaum die Knotenmoden.

### 4.3 Simulation des Fingers als Widerstand (abgeleitet, `sim_daempfer.py`)

Verlustfreier Waveguide, Pakar05-Knoten bei ξ = ½. Die Moden ohne Knoten am Finger (hier
k = 1, 3) fallen je Periode der leeren Saite so:

| ρ = R/Z0 | 0,05 | 0,1 | 0,2 | **0,3** | 0,5 | 1 | 2 | 10 | 100 |
|---|---|---|---|---|---|---|---|---|---|
| dB je Periode | −0,87 | −1,74 | −3,49 | ≈ −5,2 | −8,9 | −19,1 | **sofort weg** | −7,0 | −0,7 |

- **ρ = 2** ist die Anpassung: Zwei gleich große Wellen, die von beiden Seiten ankommen, werden
  vollständig geschluckt.
- **ρ > 2** wirkt zunehmend als Klemme: Die Energie geht in die Moden der Teilstücke. Bei
  ρ = 100 liegt k = 2 nach 20 Perioden +3 dB über dem Fall ohne Dämpfer, bei einer festen Klemme
  +10 dB.
- **Fest gedrückt bei n = 3:** Die Teilstücke klingen dann auf 1,5·f0, also auf dem gegriffenen
  7. Bund. Das ist der bekannte Fehler, zu fest zu drücken.
- Die Knotenmoden (k = 2) bleiben bei ρ ≤ 10 unberührt (≤ 0,34 dB je Periode).

Die gemessenen 4–6 dB je Periode entsprechen ρ ≈ 0,25–0,35.

### 4.4 Vorschlag

**Touch = ρ = R/Z0 ∈ [0, 1]** (Pakar05 nutzt bis ≈ 1). **Vorgabe 0,3**, wie heute.

- *Herleitung:* Bei ρ = 0,3 fallen die Moden ohne Knoten um ≈ 5 dB je Periode. Gemessen sind
  4–6 dB, im Median über drei Gitarren und n = 2–4.
- *Verlauf:* Touch 0 ist nicht berührt. Touch 1 ist sehr fest, ≈ 19 dB je Periode, die Moden
  ohne Knoten sind nach 3 Perioden um ≈ 57 dB unten. Über ρ = 2 hinaus würde der Finger klemmen;
  das gehört nicht auf diesen Regler.
- *Wie lange der Finger liegt:* Belegt ist nichts. Gemessen sind die Moden ohne Knoten nach
  ≈ 150 ms um ≥ 45 dB unten. Bei ρ = 0,3 braucht das auf E2 ≈ 9 Perioden ≈ 110 ms.
- *Umsetzung:* Der Dämpfer kann die ganze Note liegen bleiben (im Punktmodell wirkungsgleich,
  weil die Knotenmoden unberührt bleiben) oder nach ≈ 150 ms abgehoben werden, wie Pakar05 es
  zeitlich verändert. Ich empfehle, ihn liegen zu lassen: Das ist einfacher, gleich hörbar und
  physikalisch nicht falsch, solange der Punkt klein ist.
- *Künstliche Flageoletts:* dasselbe, ξ ist dann auf die gegriffene Länge bezogen.

---

## 5. Offene Lücken

1. **Up gegen Down, Pegel je Saite:** nicht belegt. Weg: selbst aufnehmen (gleiche Saite,
   20 Down und 20 Up, Plektrum) und den Spitzenpegel und das Spektrum vergleichen; oder ein
   Datensatz mit hexaphonem Tonabnehmer und annotierter Richtung, den ich nicht gefunden habe.
2. **Tapping:** nur abgeleitet, mit nicht belegter Saitenlage und Fingergeschwindigkeit. Weg:
   *Record sample* am eigenen Instrument, gezupft gegen getappt am selben Bund.
3. **Palm mute:**
   - Die Abstufung M1–M5 bei IDMT ist nicht dokumentiert, nur gemessen.
   - Die normale Referenz stammt aus einem anderen Teildatensatz (andere Spieler, die Strat
     vielleicht dieselbe).
   - Alles sind E-Gitarren über den Tonabnehmer. Die Akustikgitarre ist nicht gemessen.
   - Der Punktdämpfer unterschätzt die Höhendämpfung bei starkem Mute (§1.4).
4. **Flageolett:** Liegedauer des Fingers nicht belegt; die Pegel hängen am Bezug, das zeigt die
   Strat-Abweichung.
5. **Polarität beim Up-Anschlag:** Ob sie hörbar wird, hängt daran, ob die Wiedergabe eine
   klingende Saite fortsetzt. Im Code nicht geprüft.

## 6. Entscheidungsfrage, die aus der Recherche folgt

**F-Spread** — *Ort:* `include/loop/Spielweise.hpp:58`, Strum::spreadMs.
*Heutiger Wortlaut:* „Spread: von der ersten bis zur letzten Saite", Vorgabe 20 ms.

*Problem:* Gemessen ist der Abstand benachbarter Saiten gleichmäßig (Freire18, Barb14). Die
Gesamtzeit wächst deshalb mit der Zahl der Saiten im Akkord:

| Saiten (Table 5) | 3 | 4 | 6 |
|---|---|---|---|
| Gesamtzeit | 16–23 ms | 23–42 ms | 26–62 ms |

Mit einer festen Gesamtzeit läuft ein Akkord mit drei Saiten halb so schnell über die Saiten wie
einer mit sechs.

- **a (Empfehlung):** „Spread: Abstand zweier benachbarter Saiten in ms (die Hand), Vorgabe
  8 ms; ein Akkord über sechs Saiten dauert 40 ms."
- **b:** „Spread: von der ersten bis zur letzten Saite, Vorgabe 40 ms (für sechs Saiten
  gemessen); kleinere Akkorde nehmen dieselbe Zeit."

*Warum die Empfehlung:*

- *Ausschlag:* a gibt wieder, was gemessen ist: eine Hand mit fester Geschwindigkeit. Jeder
  Akkord stimmt dann ohne Umrechnung, und die Zahl hat dieselbe Bedeutung wie in der Literatur
  (intertone spread).
- *Für b:* Das Feld und die Datei bleiben, wie sie sind, und die Zahl sagt direkt, wie lang der
  Akkord ist.
- *Warum b hier weniger wiegt:* Das wiegt weniger, weil `.lvloop` Fassung 9 erst seit S134
  besteht und kaum Dateien mit eigenem Spread existieren.
- *Wann die Empfehlung kippt:* wenn schon gespeicherte Muster mit eigenem Spread in Gebrauch
  sind, deren Klang sich nicht ändern darf, oder wenn Patrik Spread als „Länge des Strichs"
  denkt.

---

## Dateien

- Bericht: `…/scratchpad/recherche_24c/bericht.md`
- Quellen-PDFs und Aufnahmen: `…/recherche_24c/dl/` (IDMT-Auszüge über HTTP-Range geladen,
  ≈ 120 MB; nichts daraus ausgeführt)
- Skripte: `…/recherche_24c/skripte/` — `fernzip.py` (ZIP-Auszug), `messe_idmt.py`,
  `fasse_idmt.py`, `fit_daempfer.py`, `normal_ds1.py`, `messe_bass.py`, `ha_verlauf.py`,
  `ha_energie.py`, `sim_daempfer.py`
- Rohtabellen: `zusammenfassung_idmt.md`, `fit_daempfer.md`, `normal_ds1.md`,
  `messung_bass.md`, `ha_verlauf.md`, `ha_energie.md`, `sim_daempfer.md`, `messung_idmt.txt`

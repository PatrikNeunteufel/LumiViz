# Helligkeit je Anschlagstärke im Diskant — Recherche

- **Anlass:** HS-4, S187. Seit HS-3 a rechnet der Hammer nach Kartofelev & Stulov 2014 (p = 3,7 + 0,015 N, γ = 0,9894 + 0,000088 N, τ0 = 2,72 − 0,02 N + 0,00009 N² µs, F0 = 15 500·e^(0,059 N)). Der leise Diskant klingt damit laut genug, aber der Ton wird mit der Stärke kaum heller. An C6 liegt die Steigung „dB Teilton n gegen Teilton 1 je dB Grundton“ im Modell bei −0,1…0,2, bei Salamander bei 0,7…2,6. Gesucht ist ein Mechanismus, der beides liefert.
- **Auftrag:** Mechanismen aus der Literatur, die das Hellerwerden mit der Stärke erklären (vor allem MIDI 66–108). Je Mechanismus: (1) Wirkung auf die Helligkeit je Stärke, (2) bleibt der leise Diskant laut genug, (3) veröffentlichte Parameter je Taste, (4) Messungen zum Prüfen. Dazu die Frage, ob eine Quelle zeigt, dass Stulovs Erbmodell mit seinen eigenen Parametern im Diskant heller wird oder nicht. Was in `2026-11-10_S186_hammerfilz_hysterese_exponent.md` und `2026-11-10_S186_kontaktzeit_leiser_anschlag.md` steht, wird hier nicht wiederholt.
- **Datum:** 2026-11-11
- **Werkzeuge:** WebSearch, WebFetch, curl, pdftotext, pdftoppm, `quelle_suchen.py` für die lokalen PDFs unter `…\cmake\quellen\hammer-saite\`, Python mit PIL (Abbildungen ausmessen) und ein eigenes kleines Rechenskript (Hammer gegen starre Wand, Abschnitt 1.3). Ein Browser wurde nicht benutzt. Die Kopien liegen im Scratchpad unter `quellen_hs4\`, nicht im Repo.

Markierungen: **[wörtlich]** = in der Quelle gelesen, Formeln am Seitenbild geprüft · **[abgelesen]** = aus einer Abbildung · **[Zusammenfassung]** = meine Folgerung oder Rechnung · **[ungeprüft]** = nur aus einer Sekundärquelle oder einem Suchwerkzeug.

## 0 Ergebnis in Kürze

- Die Literatur nennt einen einzigen Hauptmechanismus: Der Filz wird steifer, je stärker er eingedrückt wird. Askenfelt & Jansson nennen das „an automatic treble control connected to the volume knob“ [wörtlich]. Alle anderen Mechanismen sind dagegen klein oder im Diskant schwach belegt.
- **Stulovs Filzgesetz selbst wird mit der Stärke deutlich heller.** Gegen eine starre Wand gerechnet (Abschnitt 1.3) steigt die Grenzfrequenz des Kraftspektrums an Hammer 88 von 2,6 auf 6,9 kHz, wenn v von 1,0 auf 3,3 m/s steigt. Gemessen an einem Steinway-Hammer 88 sind es nur +1,8 kHz. Das Erbmodell wird also eher zu stark heller, nicht zu schwach. Wenn im Modell an C6 die Helligkeit verloren geht, geschieht das in der Kopplung zwischen Hammer und Saite oder in der Rechnung, nicht im Filzgesetz [Zusammenfassung].
- **Die einzige gefundene Rechnung mit Stulovs Satz an einer Diskantsaite** ist Kartofelev (Diss. 2014, Abb. 1.5 und 1.6): A7, ideal biegsame Saite, V = 3/4/5 m/s. Dort steigen Mode 2 und 3 gegenüber Mode 1 um ≈ 2–4 dB, wenn v von 3 auf 5 m/s steigt [abgelesen]. Das Erbmodell wird an der Saite also etwas heller, aber nur bei starkem Anschlag und an einer Taste gezeigt.
- Ein zweiter **vollständiger Satz je Taste** mit derselben Gesetzesform liegt vor: Chabassier & Duruflé, RT-0425 (Steinway D), F = K Φ + R dΦ/dt. Darin ist das Verhältnis R/K im Diskant 0,7–8 µs, bei Stulov dagegen 250–580 µs. Das sind zwei Größenordnungen Unterschied, und der Trend über die Tasten läuft entgegengesetzt (Abschnitt 1.2).

## 1 Steife des Filzes steigt mit der Eindrückung (Exponent p, Hysterese)

### (1) Wirkung auf die Helligkeit je Stärke

- **Askenfelt & Jansson 1990**, Vorlesung *The nonlinear hammer* [wörtlich]: „The secret is hidden in the felt hammers which become progressively harder the more they are compressed (nonlinear stiffness).“ … „This increase in effective stiffness sharpens the initial pulse created at hammer contact, and the high-frequency part of the spectrum is enhanced at loud dynamics, as illustrated in Fig. 20. The changes are large, in fact much larger than the changes reached in the experiments with exchanged hammers or voicing.“ Abb. 20 zeigt nur C4 (p, mf, f).
- **Hall 1990**, Vorlesung *compare* [wörtlich]: „notice that the pianissimo spectrum is much steeper than the fortissimo“ (Abb. 15, C4). Eine lineare Theorie „predicts the same spectrum shape for both piano and forte playing … This is clearly wrong“. Hall führt das auf den Hammer zurück: „I believe the crucial part of the system lies in the hammers.“
- **Russell (Penn State), Seite *Hammer nonlinearity, dynamics and the piano sound*:** Spektren der Saite für C2, C4 und C6 eines Yamaha-Flügels bei ff und pp [wörtlich: „In all three cases the louder blow (ff) contains many more high frequency harmonics than does the softer blow (pp).“]. Aus der Abbildung `C6-ff-pp.gif` habe ich ausgemessen (10 dB je Teilstrich, 29,75 px je 10 dB) [abgelesen, ±1 dB]:

| C6 (Yamaha) | T2 − T1 | T3 − T1 | T4 − T1 |
|---|---|---|---|
| ff | −6,7 dB | −23,2 dB | −37,6 dB |
| pp | −19,8 dB | ≤ −35,6 dB (nahe am Rauschen) | im Rauschen (≤ −38) |
| Zunahme pp → ff | **+13,1 dB** | ≥ +12,4 dB | — |

  Die beiden Kurven sind gegeneinander verschoben, deshalb lässt sich der Pegelunterschied des Grundtons nicht ablesen. Nimmt man für pp → ff etwa 25–33 dB an (Askenfelt & Jansson: „33 dB“, S186), ergibt sich für T2 eine Steigung von ≈ 0,4–0,5 dB je dB Grundton [Zusammenfassung]. Salamander zeigt an C6 für n = 2 eine Steigung von 1,17.
- **Patent US 5 423 241** (Scarton, Kennedy, Yungman, Liang; Rensselaer, 1995): Kraft des Hammers auf einem Kraftaufnehmer, gemessen für alle 88 Hämmer eines Steinway L, jeweils unintoniert und intoniert. Die „cut-off frequency“ ist die Frequenz, bei der das Kraftspektrum auf die Hälfte des Spitzenwerts fällt [wörtlich]. Belegte Sätze [wörtlich]:
  - „for voiced piano hammer #88, the cut-off frequency … increases by 1800 hertz as velocity increases from 40 inches per second to 130 inches per second“ (1,02 → 3,30 m/s)
  - „the cut-off frequency for voiced hammer #1 is 1.53 kilohertz and the cut-off frequency for voiced hammer #88 is 3.46 kilohertz“, gemessen gegen die „string replica impact plate“ bei „94 to 102 inches per second“ (2,38–2,59 m/s)
  - „the piano hammers have a cut-off frequency which is 300 to 400 hertz higher when struck against a flat plate“
  - Steigung an Hammer #1: „4.876 hertz per inch per second for the unvoiced piano hammer and 9.350 hertz per inch per second for the voiced piano hammer“
- [Zusammenfassung] Daraus folgt: Am höchsten Hammer steigt die Grenzfrequenz um ≈ 790 Hz je m/s. Der intonierte Hammer wird mit v fast doppelt so schnell heller wie der unintonierte (an #1 gemessen). Stulovs Satz stammt von **unintonierten** Abel-Hämmern (S186).

### (1.2) Zwei vollständige Sätze je Taste mit derselben Gesetzesform

Bei Chabassier, Joly & Chaigne 2013 (JASA 134, Gl. 13) lautet das Gesetz F = K_H Φ(d) + R_H d/dt Φ(d) mit Φ = (…)₊^p [wörtlich, Text; die Gleichung ist im lokalen PDF als Text lesbar, das Bild habe ich nicht geprüft]. Das ist dieselbe Form wie Stulovs Näherung für t ≫ τ0, F = Q0(u^p + α d(u^p)/dt), mit α = R_H/K_H [Zusammenfassung].

**RT-0425 (Chabassier & Duruflé 2012), S. 8, Gl. (2)–(5)** [wörtlich, am Seitenbild geprüft], i = 1…88:
- M_H = −6,2348·10⁻⁵ i + 0,0112 (kg)
- p = 2,4295·10⁻⁴ i² − 0,007703 i + 2,337
- K_H = 10^(5,3097·10⁻² i + 7,6425)
- R_H = K_H × 10^(0,04366 i − 2,294)

Gl. (5) passt so gedruckt nicht zu Abb. 4c der Quelle. Dort steigt R von ≈ 10^5,4 auf ≈ 10^6,2, während K von ≈ 10^7,7 auf ≈ 10^12,3 steigt [abgelesen]. Die Abbildung passt nur zu einem **negativen** Exponenten 10^(−0,04366 i − 2,294). Das deckt sich mit dem Vermerk aus S159, dass in RT-0425 Gl. 5 ein Minus fehlt [Zusammenfassung].

| Taste i | p (RT-0425) | α = R/K (RT-0425) | p (Stulov 2014) | α = γτ0/(1−γ) (Stulov) |
|---|---|---|---|---|
| 1 | 2,33 | 4,9 ms | 3,72 | 254 µs |
| 40 | 2,43 | 91 µs | 4,30 | 289 µs |
| 64 (C6) | 2,84 | 8,2 µs | 4,66 | 362 µs |
| 84 | 3,39 | 1,1 µs | 4,96 | 520 µs |
| 88 | 3,54 | 0,73 µs | 5,02 | 579 µs |

[Zusammenfassung, aus den Formeln gerechnet.] Nach Chabassier ist der Diskanthammer also fast rein elastisch mit p ≈ 2,8–3,5. Nach Stulov ist er stark zäh mit p ≈ 5. Gemessene p eingespielter Diskanthämmer liegen bei ≈ 2,7–3,4 (Hall, S186). Chabassier gibt nicht an, woher R_H im Einzelnen stammt: „These parameters are based on measurements on the Steinway D piano“ [wörtlich, S. 4].

### (1.3) Eigene Rechnung: Hammer gegen eine starre Wand [Zusammenfassung]

Das Skript liegt im Scratchpad (`wand.py`). Das Erbintegral wird exakt-exponentiell integriert, dt = 5 ns. Die Grenzfrequenz f½ ist wie im Patent definiert (halbe Amplitude des Kraftspektrums gegenüber Gleichanteil). L4k ist der Pegel bei 4 kHz gegenüber dem Gleichanteil.

| Hammer 88 | v [m/s] | 0,5 | 1,02 | 2,5 | 3,3 | 5,0 |
|---|---|---|---|---|---|---|
| Stulov-Erbmodell (2014) | f½ [kHz] | 1,48 | 2,62 | 5,44 | 6,87 | 9,78 |
| | L4k [dB] | −33,3 | −12,9 | −3,4 | −2,2 | −1,1 |
| | Halbwertsbreite [ms] | 0,28 | 0,16 | 0,075 | 0,060 | 0,042 |
| nur Q0·u^p (ohne Zähigkeit) | f½ [kHz] | 1,24 | 2,00 | 3,62 | 4,39 | 5,77 |
| Chabassier RT-0425 | f½ [kHz] | 0,53 | 0,76 | 1,24 | 1,43 | 1,81 |
| | L4k [dB] | −102 | −90 | −57 | −46 | −33 |
| **Messung Patent (Steinway L, #88)** | f½ [kHz] | — | Anstieg **+1,8** bis 3,3 m/s | **3,46** (Ersatzsaite), ≈ 3,8 (flache Platte) | | — |

An Taste 64 ergibt das Erbmodell f½ = 1,10 / 1,96 / 3,96 / 4,96 / 7,01 kHz.

- Das Erbmodell wird an der Wand heller als alle anderen Fassungen: von 1,02 auf 3,3 m/s +4,2 kHz, gemessen sind +1,8 kHz. Der Zähigkeitsterm verstärkt das Hellerwerden an der Wand noch. Die Wand bremst das „Hochpass“-Bild also nicht, im Gegenteil.
- Bei 2,5 m/s liegt das Erbmodell 1,6–2,0 kHz über der Messung, das reine Potenzgesetz mit Stulovs Q0 trifft sie fast (3,6 kHz). Chabassiers Satz liegt um den Faktor ≈ 3 darunter.
- Hall (Vorlesung *compare*, Abb. 18; die Taste ist nicht genannt) misst an der Wand pp 12 N bei 0,47 ms Halbwertsbreite und ff 145 N bei 0,22 ms [abgelesen, S186]. Das Erbmodell an Hammer 88 ergibt 9,7 N bei 0,28 ms (0,5 m/s) und 166 N bei 0,075 ms (2,5 m/s). Die Kraft passt, die Pulse sind aber 2- bis 3-mal schmaler.

**Folgerung:** Stulovs Gesetz mit seinem eigenen Satz liefert das Hellerwerden im Filz. Wenn das Modell an C6 trotzdem nicht heller wird, entsteht der Verlust in der Kopplung mit der Saite. Im Diskant ist die Saite für den Hammer ein Dämpfer (Impedanz 2Z je Saite), und dazu kommen die schnell zurückkehrenden Wellen vom kurzen Stück zur Agraffe. Wo der Filz weit „härter“ ist als die Saite, bestimmt die Saite die Pulsform, und die hängt nicht von v ab. Eine Probe mit einer unendlich langen Saite (2Z = 12,6 kg/s, drei Saiten) zeigt genau das: f½ bleibt bei Taste 84 für alle drei Filzgesetze bei 0,43–0,67 kHz und ändert sich mit v kaum. Diese Probe ist grob vereinfacht und ersetzt nicht die Rechnung im Kern.

### (1.4) Rechnet jemand Stulovs Satz an einer Diskantsaite?

- **Kartofelev, Diss. TTU 2014, Abschnitt 1.3, S. 16–19** [wörtlich]: A7 (Taste 85), „L = 61 mm, … l = 2.6 mm, … µ = 4.2 g/m, … T = 774.6 N“, Hammermasse ein Drittel, „m = 1.8 g“, „Q0 = 8387.4 N/mm^p̂, … p̂ = 4.975, hereditary parameter α = 0.5312 ms“. Die Saite ist „an ideal flexible string“, Steg und Boden sind weggelassen. Zu Abb. 1.5 heißt es: „It is evident that with the increasing of the amplitude of the string vibration the mean level of high frequency modes increases; this phenomenon confirms that the interaction between the termination and the string is nonlinear“. Der Text schreibt das Hellerwerden also der Auflage am Capo d'Astro zu, nicht dem Hammer.
- [abgelesen, ±1,5 dB, Bild bei 600 dpi] Abb. 1.5 (Capo, R = 15 mm, t = 6,21 ms; Mode 1 bei allen Kurven ≈ −14 dB):

| Mode − Mode 1 | V = 3 | V = 4 | V = 5 m/s |
|---|---|---|---|
| 2 | −10,1 | −7,6 | −5,8 |
| 3 | −19,3 | −16,4 | −14,5 |
| 4 | −23,5 | −23,3 | −22,3 |

  Mit ideal fester Auflage (schwarze Linie in Abb. 1.5 bei V = 3 und in Abb. 1.6 bei V = 5) ändern sich Mode 2 − Mode 1 und Mode 3 − Mode 1 von 3 auf 5 m/s nur um ≈ +1,5…+4 dB [abgelesen, unsicher, weil die schwarze Linie in Abb. 1.5 teilweise verdeckt ist].
- [Zusammenfassung] Das ist der einzige gefundene Beleg für Stulovs Satz an einer Diskantsaite. Er zeigt bei ideal fester Auflage ein **schwaches** Hellerwerden zwischen 3 und 5 m/s (≈ 4,4 dB in v), gröber geschätzt eine Steigung von 0,3–0,8. Unter 3 m/s, also bei p und pp, wurde nicht gerechnet. Eine Quelle, die zeigt, dass das Erbmodell im Diskant **nicht** heller wird, habe ich nicht gefunden.

### (2) Bleibt der leise Diskant laut genug?

- [Zusammenfassung] Mit Chabassiers Satz nein: An der Wand liegt bei 0,5 m/s an Taste 88 der Pegel bei 2 kHz 92 dB unter dem Gleichanteil (Stulov −11 dB). Genau das beschreibt der Befund aus HS-2: kleines α, leiser Diskant zu leise.
- Askenfelt & Jansson [wörtlich, S186]: Auch real gilt „In the treble … even the fundamental period is much too short to be optimally excited“. Conklin (Vorlesung *The hammers*) [wörtlich]: „In order to produce tones of uniform loudness all across the scale, the extreme treble hammers must be much harder than the middle or bass hammers.“ Abb. 10 dort [abgelesen]: relative Härte C1–C4 ≈ 87, C5 ≈ 88, C6 ≈ 90, C7 ≈ 93, C8 ≈ 98.
- [Zusammenfassung] Ein niedrigeres p (≈ 3–3,5 wie Hall und RT-0425), dazu Q0 so gestimmt, dass ff gleich bleibt, macht den Hammer bei kleinem v härter als p = 5. Die Kontaktzeit wächst dann zu kleinem v hin schwächer (gegen eine Wand T ∝ v^−(p−1)/(p+1): p = 5 → v^−0,67, p = 3,5 → v^−0,56). Der Filz bleibt dabei nichtlinear und wird weiter heller. Diese Richtung erfüllt beide Ziele, ist aber nur als Richtung belegt, nicht als Satz je Taste.

### (3) Parameter je Taste

Belegt sind Stulov 2014 (S186) und RT-0425 (Gl. 2–5, oben, mit dem Vorbehalt zum Vorzeichen in Gl. 5). Hall misst p je Oktave (Abb. 20a–d, S186). Das Patent misst f½ aller 88 Hämmer bei 2,4–2,6 m/s, unintoniert und intoniert. Die Tabelle steht in der Patentschrift als Abbildung und wurde nicht ausgemessen.

### (4) Prüfbare Messungen

- Kontaktzeit pp/mf/ff an C4–C8 (Askenfelt & Jansson Abb. 7, S186)
- f½ je Hammer bei 2,5 m/s und die Steigung über v an #1 und #88 (Patent)
- Hammer gegen Wand bei pp/ff (Hall Abb. 18)
- Spektren C6 bei pp und ff (Russell)
- Hysteresekurven bei 0,74/1,00/1,31 m/s (Kartofelev & Stulov 2014, Abb. 1; S186)
- **Kartofelev Abb. 1.5/1.6** mit vollständigem Parametersatz. Das ist ein direkter Prüffall für den Kern: A7 an ideal biegsamer Saite bei 3/4/5 m/s nachrechnen. Wird der Kern dort zwischen 3 und 5 m/s nicht heller als ≈ 2–4 dB, liegt das an der Physik. Wird er gar nicht heller, liegt es an der Rechnung.

## 2 Härteverlauf im Filz: weiche Oberfläche, harter Kern

- **(1) Wirkung:** Conklin [wörtlich]: „the hardness should have a gradient such that the string-contacting surface is softer than the inner material. If there is no gradient, the result can be poor tone or undesirable noise components.“ Abb. 9 zeigt die Shore-A-Härte über die Filzbreite [abgelesen]: weicher Hammer 91 → 68, mittlerer 95 → 80, harter 96 → 90, jeweils von 0 % bis 100 % der Breite. Welches Ende die Oberfläche ist, ist nicht beschriftet; die Folgerung „100 % = Oberfläche“ stammt aus dem Text. Chabassier & Duruflé 2014 (JSV, Vorabdruck V2, Abb. 11c, nach einem Lehrbuch der Intonation) [wörtlich]: „Zone 1 : weak - light needling for piano dynamics. Zone 2 : medium - deep needling for mezzo-forte dynamics. Zone 3 : heavy - deep needling for forte dynamics.“ Dazu: „the coefficients of the hammer head - string interaction could be modeled as a function of the contact point … Unfortunately, to the best of our knowledge, this kind of experimental data is not available in the literature“ [wörtlich].
- [Zusammenfassung] Mechanisch ist das ein p, das mit der Eindrückung steigt: weich, solange nur die Oberfläche arbeitet, steif, sobald der Kern erreicht ist. Das verstärkt das Hellerwerden bei ff, ohne pp zu ändern.
- **(2)** Für den leisen Diskant hilft das nicht. Eine weiche Oberfläche macht pp eher dunkler und leiser.
- **(3) Parameter:** keine je Taste. Es gibt nur Shore-Profile für drei Hämmer an G5 (Conklin Abb. 9) und die relative Härte über den Umfang (Abb. 10).
- **(4) Prüfbar:** nur mittelbar, über die Kontaktzeit pp/ff und f½ über v.

## 3 Belastungsrate und Relaxationszeit im Hörbereich

- **(1)** Aus der Diss. Kartofelev, S. 16 [wörtlich]: „A main feature of hammers is that the slope of the force–compression characteristics is strongly dependent on the rate of loading.“ Im Erbmodell wirkt die Rate nur über τ0. Der Satz je Taste hat τ0 = 1,7–2,7 µs. Die „realistic values“ anderer Arbeiten liegen bei τ0 = 10 µs (Report r307, ε = 0,96) und 20 µs (Kartofelev & Stulov 2014 Gl. 20, γ = 0,99) (S186).
- [Zusammenfassung] Alle diese τ0 sind ≪ Kontaktzeit (0,45–2,5 ms im Diskant). Die Steife je Rate geht deshalb schon in den ersten Mikrosekunden von F0 auf Q0 über, und zwischen pp und ff ändert sie sich im Hörbereich praktisch nicht. Ein Filz, der bei ff (Kontakt ≈ 0,5–0,9 ms) merklich steifer wäre als bei pp (1,2–2,5 ms), bräuchte eine Relaxationszeit von ≈ 0,1–1 ms. Er würde dann beides liefern: bei pp weich und zäh, bei ff steif. **Werte dafür habe ich nicht gefunden.** Stulov 2004 (Acta Mech. 169, 13–21) nennt laut Abstract ein Vier- und ein Dreiparametermodell, „equivalent … for all realistic values of hammer velocity“ [ungeprüft, Katalogeintrag]. Yanagisawa & Nakamura 1984 (*Dynamic compression characteristics of piano hammer felt*, J. Acoust. Soc. Jpn.) habe ich nicht erreicht.
- **(3)** nicht veröffentlicht · **(4)** Hysteresekurven bei drei Geschwindigkeiten (Kartofelev & Stulov 2014, Abb. 1).

## 4 Biegung des Hammerstiels und Drehung des Kopfes

- **(1)** Chabassier & Duruflé 2014 (Vorabdruck V2, D1, Timoshenko-Stiel, bei gleicher Hammergeschwindigkeit ≈ 3,4 m/s, nur der Anschlag ist verschieden) [wörtlich]: Kraftspektrum „around 600 Hz where the staccato force exhibits a drop in its spectrum of 10 dB compared to the spectrum of the force obtained with no shank, and the legato force exhibits a drop even more dramatic of about 25 dB“. Weiter: „partial 16 is below -40 dB for the legato touch …, but reaches almost -20 db in the staccato touch“. Askenfelt & Jansson (*Hammer resonances*) [wörtlich]: Moden des Stiels bei ≈ 50 Hz („backwash“) und 250 Hz („ripple“), „a treble hammer was much higher (+80%)“. Conklin [wörtlich]: „a major vibrational mode around 260 Hz … it can be heard in the treble register, as part of the "knock" component of the tone, beginning somewhere around A4 (key 49)“. Vyasarayani, Birkett & McPhee 2009 (JASA 125, 4034–4042): Laut Zusammenfassung ändert ein sehr nachgiebiger Stiel die Bewegung von Hammer und Saite merklich [ungeprüft].
- [Zusammenfassung] Der Stiel ändert die Klangfarbe bei **gleicher** Geschwindigkeit, je nach Anschlag. Dass er das Hellerwerden mit v erklärt, zeigt keine Quelle. Die Zahlen gelten für D1, nicht für den Diskant.
- **(2)** Für den leisen Diskant nicht belegt · **(3)** Für C4 sind nur Frequenzen der Stielmoden veröffentlicht, im Diskant +80 % · **(4)** Beschleunigung des Hammerkopfs (Askenfelt & Jansson Abb. 12; Chaigne 2016, Abb. 14 an D♯6, Bösendorfer).

## 5 Geometrische Nichtlinearität der Saite (Längsschwingung, Phantomteiltöne)

- **(1)** Chabassier, Joly & Chaigne 2013, Tab. IV (S. 49) [wörtlich, am Bild geprüft]: ADR (größte Auslenkung durch Durchmesser) für G6 0,12 / 0,443 / 0,96 bei 0,5 / 1,5 / 3 m/s, für C♯5 bis 2,64 bei 4,5 m/s. Text: „nonlinear effects should be noticeable in the bass and medium ranges, even for moderate hammer velocity, whereas they should be less easily detectable for the treble notes, except for strong impacts. This simple criterion is in accordance with experimental observations.“ Conklin (*Longitudinal string modes*) [wörtlich]: „the longitudinal mode is important in determining the tone color of the bass and tenor regions“. Chaigne 2016 nennt für V ≈ 1 m/s den Kennwert der Nichtlinearität, der „regularly decreases as the pitch of the note … increases“ [wörtlich].
- [Zusammenfassung] Phantomteiltöne wachsen mit dem Quadrat der Amplitude, also mit 2 dB je dB. An C6 lägen die geraden Phantome (2f_n) nur wenige Hz neben den Teiltönen und fielen in der Messung mit ihnen zusammen. Bei ADR < 1 sind sie im Diskant aber schwach. Als Hauptursache einer Steigung von 0,7–2,6 an C6 ist das nicht belegt, es kommt höchstens bei ff dazu.
- **(3)** Parameter der Saite liegen vor (RT-0425, Saitentafeln) · **(4)** An Salamander-C6 ff prüfen, ob neben T2 ein zweiter Gipfel bei genau 2f1 liegt.

## 6 Stoßgeräusch Hammer–Saite und Knock

- Miranda Valiente et al., Forum Acusticum 2026 (Beitrag 000032) [wörtlich]: „collision-generated acoustic pressure is negligible in the low register but can contribute measurably at higher notes, especially for receivers or microphones placed close to the hammer.“ Für C7: „making the collision contribution more noticeable at low frequencies“. Conklin: Knock ab A4 (Abschnitt 4).
- [Zusammenfassung] Dieses Geräusch ist breitbandig oder tieffrequent und nicht harmonisch, es erklärt keine Teiltöne n = 2…8. Es könnte die Messung an Salamander aber verfälschen, wenn die Pegel der Teiltöne früh nach dem Anschlag gemessen werden: Bei ff steckt mehr Geräusch in den Bändern der Teiltöne. Ob das zutrifft, hängt vom Messfenster der LumiViz-Messung ab und sollte dort geprüft werden.

## 7 Intonation und eingespielte Hämmer

- Patent (oben): intoniert gegen unintoniert an #1 9,35 gegen 4,88 Hz je in/s. Dazu [wörtlich]: „the process of voicing a piano hammer can increase the cut-off frequency by 250 hertz or more for piano hammers in the bass range and by 500 hertz or more for hammers in the treble range.“ Askenfelt & Jansson (*Voicing*, C4, mf) [wörtlich]: Die Intonation reduziert „the upper limit of the spectra from about 8 kHz to approximately 4 kHz“. Hall (*compare*) [wörtlich]: Bei abgenutzten Hämmern ist p kleiner, „we did not get as much darker tone as we expected when trying to play softly“.
- [Zusammenfassung] Stulovs Satz beschreibt **neue, unintonierte** Hämmer. Das Patent zeigt, dass intonierte Hämmer mit v steiler heller werden. Eine Umrechnung des Satzes auf intonierte Hämmer ist nicht veröffentlicht.

## Reichweite

- **Gelesen:**
  - KTH-Vorlesungen: Askenfelt & Jansson (*nonlinea*, *voicing*, *hammres*, *basmidtr*, *stringvib*), Hall (*compare*, *complica*, *further*), Conklin (*thehammers*, *longitudinal*), mit Abbildungen
  - Penn-State-Seiten von Russell (*Dynamics*, Abbildungen C6/C4/C1–C8)
  - Purdue *Piano Physics*
  - Patent US 5 423 241 (Google Patents, Volltext per curl)
  - Chabassier & Duruflé 2014, Vorabdruck V2 (math.u-bordeaux.fr)
  - Chaigne 2016, JASA 140 (CC BY, pub.mdw.ac.at)
  - Miranda Valiente et al., FA 2026
  - lokal: RT-0425, Chabassier, Joly & Chaigne 2013, Bank 2006, Nishiguchi 2004, Lehtonen 2005 (Durchsuchung)
  - die Diss. Kartofelev aus dem Scratchpad von S186
- **Nicht erreicht oder gesperrt:**
  - UWSpace (Diss. Vyasarayani; Anubis-Sperre)
  - Springer-PDF Giordano & Jiang 2004 (nur HTML-Seite ohne Volltext); Purdue e-Pubs ohne PDF
  - ResearchGate (nicht versucht)
  - JASA-Originale von Giordano & Winans 2000, Giordano & Millis 2004, Hall & Askenfelt 1988, Hall 1992, Askenfelt & Jansson 1993, Russell & Rossing 1998, Birkett 2013, Vyasarayani 2009, Chaigne & Askenfelt 1994 II, Stulov 1995/2004/2005/2008 (Bezahlschranke, keine offene Kopie gefunden)
  - Stulov ICA/ISMA 2004
  - Yanagisawa & Nakamura 1984
- **Nicht gefunden:**
  - eine veröffentlichte Messung der Teiltonpegel je Stärke im Diskant mit mehr als zwei Stufen
  - eine Relaxationszeit des Filzes im Bereich 0,1–1 ms
  - Parameter für Filz in Schichten je Taste
  - eine Quelle, die belegt, dass Stulovs Satz im Diskant **nicht** heller wird

## Quellen

1. Askenfelt, A. & Jansson, E. (1990): From touch to string vibration, Seiten *The nonlinear hammer*, *Voicing*, *Hammer resonances*, *Bass – mid – treble*. https://www.speech.kth.se/music/5_lectures/askenflt/nonlinea.html (ebenso `voicing.html`, `hammres.html`, `basmidtr.html`)
2. Hall, D. E. (1990): The hammer and the string, Seiten *compare*, *complica*, *further*. https://www.speech.kth.se/music/5_lectures/hall/compare.html
3. Conklin, H. A. (1990): Piano design factors, Seiten *The hammers* (Abb. 9, 10), *Longitudinal string modes*. https://www.speech.kth.se/music/5_lectures/conklin/thehammers.html
4. Russell, D. A. (1997): Hammer nonlinearity, dynamics and the piano sound. https://www.acs.psu.edu/drussell/Piano/Dynamics.html (Abb. `Figures/C6-ff-pp.gif`)
5. Scarton, H. A., Kennedy, W. C., Yungman, J. F. & Liang, P. N.-J. (1995): Quantitative method for evaluation of the state of piano hammer felt tonal regulation, US 5 423 241. https://patents.google.com/patent/US5423241A/en
6. Chabassier, J. & Duruflé, M. (2012): Physical parameters for piano modeling, Inria RT-0425, S. 8, Gl. (2)–(5), Abb. 4 (lokal: `Chabassier_Durufle_2012_PhysicalParametersPianoModeling_RT0425.pdf`)
7. Chabassier, J., Chaigne, A. & Joly, P. (2013): Modeling and simulation of a grand piano, JASA 134, 648–665, Gl. (13), Tab. IV (lokal: `Chabassier_Joly_Chaigne_2013_ModelingSimulationGrandPiano_JASA_hal-00873089.pdf`)
8. Chabassier, J. & Duruflé, M. (2014): Energy based simulation of a Timoshenko beam in non-forced rotation. Influence of the piano hammer shank flexibility on the sound, J. Sound Vib. (Vorabdruck V2). https://www.math.u-bordeaux.fr/~durufle/article/HammerJSVV2.pdf
9. Chaigne, A. (2016): Reconstruction of piano hammer force from string velocity, JASA 140, 3504–3517, Abb. 3. https://pub.mdw.ac.at/media/content_files/3504_1_online.pdf
10. Kartofelev, D. (2014): Nonlinear Sound Generation Mechanisms in Musical Acoustics, Diss. TTU, Abschnitt 1.3, S. 16–19, Abb. 1.5, 1.6. https://digikogu.taltech.ee/en/Download/4b12b551-ccf9-4ac9-b66d-236f45b853f3
11. Miranda Valiente, P., Squicciarini, G., Thompson, D., Owen Norris, D. & Hernandez, C. (2026): Physics-based modelling of mechanical impact noise in piano acoustics: hammer–string acceleration noise, Forum Acusticum 2026. https://dael.euracoustics.org/confs/fa2026/data/articles/000032.pdf
12. Piano Physics at Purdue (Giordano). https://www.physics.purdue.edu/piano/piano.html
13. Vyasarayani, C., Birkett, S. & McPhee, J. (2009): JASA 125, 4034–4042, nur Eintrag. https://uwaterloo.ca/motion-research-group/publications/modelling-dynamics-compliant-piano-action-mechanism [ungeprüft]
14. Stulov, A. (2004): Dynamic behavior and mechanical features of wool felt, Acta Mech. 169, 13–21, nur Katalogeintrag [ungeprüft]

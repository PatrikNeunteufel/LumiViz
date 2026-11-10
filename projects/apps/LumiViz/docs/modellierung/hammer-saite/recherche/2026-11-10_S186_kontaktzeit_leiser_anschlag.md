# Kontaktzeit Hammer–Saite bei leisem Anschlag im Diskant — Recherche

- **Anlass:** Befund LV6 (a), S186: Im Diskant (MIDI 96–108) ergibt das Modell bei kleiner Hammergeschwindigkeit fast keinen Ton. Der Kontakt dauert dort mehrere Perioden (G7 3,0 ms bei 0,16 m/s, Periode 0,32 ms).
- **Auftrag:** Belegen, ob (a) die Abbildung Anschlagstufe → v (0,05…5 m/s) zu tief reicht oder (b) der Hammer im Diskant bei kleinem v zu lange anliegt. Gesucht sind Messungen der Kontaktzeit nach Taste und Dynamik, die kleinsten Geschwindigkeiten und die Filzparameter im Diskant.
- **Datum:** 2026-11-10
- **Werkzeuge:** WebSearch, WebFetch, curl, pdftotext und `quelle_suchen.py`. Abbildungen wurden als GIF geladen und am Bild abgelesen. Ein Browser wurde nicht benutzt. Die Kopien liegen im Scratchpad unter `quellen_lv6a\`, nicht im Repo.

Markierungen: **[wörtlich]** = in der Quelle selbst gelesen · **[abgelesen]** = aus der Abbildung abgelesen, ±0,1 ms (kleine GIFs mit 256 px) · **[Zusammenfassung]** = von mir zusammengefasst oder gefolgert · **[ungeprüft]** = nur aus einem Suchwerkzeug, kein Beleg.

## 1 Kontaktdauer nach Taste und Dynamik

**Askenfelt & Jansson (1990)**, Vorlesung „From touch to string vibration“ (Five lectures on the acoustics of the piano, KTH), Seite *String contact duration and dynamic level*, Abb. 6–8. Die Abbildungen stammen aus der JASA-Reihe.

- [wörtlich] „the contact durations decrease from about 4 ms in the bass to less than 1 ms in the highest treble.“
- [wörtlich] Zu Abb. 7: „The bars indicate the range in contact duration between a blow in ff (left end) and pp (right end). The vertical line in each bar represents a blow by a pendulum at mezzo forte.“
- [wörtlich] „In the treble on the other hand, even the fundamental period is much too short to be optimally excited“

| Taste | ff [ms] | mf (Pendel) [ms] | pp [ms] | pp in Perioden T0 (Abb. 8) |
|---|---|---|---|---|
| C4 | ≈2,0 | ≈2,15 | ≈2,8 | ≈0,7 |
| C5 | ≈1,25 | ≈1,45 | ≈2,3 | ≈1,2 |
| C6 | ≈0,85 | ≈1,1 | ≈2,5 | ≈2,6 |
| C7 | ≈0,55 | ≈0,65 | ≈0,9 | ≈1,9 |
| C8 | ≈0,45 | ≈0,55 | ≈1,2 | **≈5** |

Die Werte sind [abgelesen] aus Abb. 7. Die letzte Spalte stammt aus Abb. 8 („% of T0/2“): C8 reicht dort bis ≈1000 % von T0/2, C7 bis ≈380 %, C6 bis ≈525 %. Gegenprobe: Bei C8 ist T0/2 = 0,119 ms, also 1000 % ≈ 1,19 ms. Das passt zu Abb. 7.

Abb. 6 zeigt C4, die Geschwindigkeit gegen die Kontaktzeit [abgelesen]: v ≈ 0,1 m/s → 4 ms; 0,6 → 3,0; 1,3 → 2,5; 2 → 2,3; 5 → 2,0; 9 → 1,6–1,7 ms. Über fast zwei Dekaden der Geschwindigkeit ändert sich die Kontaktzeit nur um den Faktor ≈2,4. [wörtlich] „The variation in contact duration within a comfortably accessible dynamic range (p - ff) is about +/- 20 % compared to mezzo forte.“

- [Zusammenfassung] Welche Geschwindigkeit „pp“ in Abb. 7 bedeutet, steht dort nicht. An C4 liegt das pp-Ende bei ≈2,8 ms, und das entspricht in Abb. 6 etwa 0,8 m/s. Das ist eine Folgerung, kein Wert der Quelle.

**Hall (1990)**, Vorlesung „Piano string excitation“ (KTH-Reihe), Seite *compare*:

- Abb. 18 zeigt einen Hammer gegen eine starre Wand [abgelesen bzw. Beschriftung]: ff 145 N mit 0,22 ms Halbwertsbreite, pp 12 N mit 0,47 ms. Welche Taste das ist, steht nicht dabei.
- Abb. 19 zeigt Steinway D, C5, „EXPONENT 2.5“, gemessen von ppp bis fff [wörtlich, Beschriftung].
- [wörtlich] „a greater degree of nonlinearity toward the treble. The decrease in contact time from bass to treble is mainly a result of the decreasing hammer mass.“

## 2 Hammergeschwindigkeiten (pp, ff, Untergrenze)

- **Goebl (2001)**, JASA 110(1), 563–572, Fußnote 6, S. 572 [wörtlich]: „The performing pianists typically produce IHV values between 30 (4.26 m/s) and about 600 (0.21 m/s), where at 600 the hammers will not reach the strings anymore (silent note).“ Gemessen wurde am Bösendorfer SE290.
- **Goebl (2001)**, S. 563 [wörtlich], nach Askenfelt (1990, S. 43): „at a piano touch (hammer velocity 1 m/s) … at a forte attack (5 m/s)“. Auf S. 564 nach Askenfelt & Jansson (1991, S. 2385): die Zeit bis zum Tastenboden „ranges from 25 ms (forte or 5-m/s final hammer velocity, FHV) to 160 ms (piano or 1-m/s FHV)“.
- **Askenfelt & Jansson**, Vorlesungsseite *motions* [wörtlich]: Die Tastengeschwindigkeit bei mezzo forte beträgt „approximately 0.3 - 0.5 m/s“, und „the hammer velocities are about five times higher than the key velocities“. Zu forte: „the maximum hammer velocity is about 5 m/s (18 km/h), not very far from the highest velocity observed during the experiments.“
- **Askenfelt & Jansson**, Seite *keybott* [wörtlich]: „The dynamic span ranges from pp to ff, corresponding to 33 dB.“
- **Goebl & Bresin (2003)**, JASA 114(4), S. 2278 [wörtlich]: Die Disklavier-Solenoide schaffen am G6 „up to 3.5 m/s“, am C1 „only up to 2.4 m/s“.
- [Zusammenfassung] Als Bereich ergeben sich etwa 0,2–5 m/s, das sind rund 27 dB in v. pp liegt bei ≈0,5–1 m/s, ff bei ≈4–5 m/s. Unter ≈0,2 m/s erreicht der Hammer die Saite nicht mehr. Ein Einzelpunkt bei ≈0,1 m/s (C4, Abb. 6) ist der tiefste gefundene Messwert. Die heutige Abbildung reicht von 0,05 bis 5 m/s, also 40 dB, und liegt damit unten um rund 12 dB unter der belegten Spielgrenze.

## 3 Filzparameter im Diskant

- **Stulov & Kartofelev (2010)**, VITA 2010 (Formeln (25), (26)) [wörtlich]: „The continuous variations in the hammer parameters vs. key number n were obtained experimentally by measuring a whole hammer set of recently produced unvoiced Abel hammers. The result of those experiments are presented in [8, 9, 10].“ [8–10] sind Stulov 2004, 2005 und eine weitere Stulov-Arbeit. Die Formeln lauten Q0 = 183·exp(0,045 n) [N/mm^p], p = 3,7 + 0,015 n, m = 11,074 − 0,074 n + 10⁻⁴ n² [g], „for hammer number 1 ≤ n ≤ 88“. Die Masse ist an „n = 88 (C8) 5.3 g“. Für n = 70: „Q0 = 4270 N/mm^p; nonlinearity exponent p = 4.75; hereditary parameter = 0.395 ms“.
  - [Zusammenfassung] Daraus folgt für n = 88: Q0 ≈ 9,6·10³ N/mm^p, p ≈ 5,0. Die Formel ist ein Fit über einen **ganz gemessenen Satz** bis n = 88, also nicht extrapoliert. Der Satz war allerdings **unintoniert** (unvoiced, neu). Die τ-Formel lässt sich mit dem gedruckten Wert für n = 70 nicht nachrechnen, weil die Einheit unklar ist. Bitte nicht ungeprüft übernehmen.
- **Hall**, Abb. 20a–d [abgelesen]: Der Exponent p steigt zum Diskant hin. Beim Steinweg-Satz (gebraucht) liegt C1 bei ≈2,15, C4 bei ≈2,45 und G7 bei ≈3,2. G7 ist dort die höchste gemessene Taste. Beim neuen Satz „Steinway D / Isaac“ (noch nicht eingespielt) liegt C8 bei ≈2,7 und die Taste darunter bei ≈3,4. Beim Muir Wood (um 1800) liegt C7 bei ≈2,6.
- **Russell & Rossing**, Seite der Penn State, „The piano hammer as a nonlinear spring“: p liegt bei 2,2–3,5 für Hämmer aus Klavieren und 1,5–2,8 für unbenutzte. Für intonierte Hämmer beschreibt die Seite einen glatten Anstieg „p≈2 in the bass, to p≈4 in the treble“ [wörtlich, auf der Seite selbst gelesen].
- [Zusammenfassung] Gemessene p für intonierte oder eingespielte Diskanthämmer liegen bei ≈2,7–4. Stulovs unintonierter Satz hat ≈5.

## 4 Wird der Kontakt real länger als einige Perioden?

- [abgelesen] **Ja, bei pp an C8.** Nach Askenfelt & Jansson Abb. 8 dauert der Kontakt dort ≈1000 % von T0/2, also ≈5 Perioden. Schon bei ff sind es ≈1,75 Perioden. An C7 sind es bei pp ≈2 Perioden, an C6 ≈2,6. Schwache Anregung im obersten Diskant ist also real und ausdrücklich so beschrieben (Zitat in Abschnitt 1).
- [Zusammenfassung] Vergleich mit dem Modell: Ich habe einen Stulov-Hammer gegen eine starre Wand gerechnet, mit den Formeln aus Abschnitt 3 und ohne Hysterese. An n = 88 ergibt das 2,85 / 1,80 / 1,14 / 0,72 / 0,46 / 0,29 ms bei 0,16 / 0,32 / 0,63 / 1,26 / 2,5 / 5 m/s, an n = 83 (G7) 2,93 ms bei 0,16 m/s. Die Kontaktzeiten des Modells (G7 3,0 ms, C8 2,9 ms bei 0,16 m/s) entsprechen also dem Stulov-Satz, ein Fehler der Rechnung ist darin nicht zu sehen. Das gemessene pp an C8 (≈1,2 ms) erreicht das Modell bei etwa 0,6–0,7 m/s, das ff (≈0,45 ms) etwa bei 5 m/s. Im belegten Spielbereich 0,5–5 m/s stimmt das Modell mit Askenfelt & Jansson also grob überein. Die Kontaktzeiten von 2–3 ms entstehen erst unter ≈0,3 m/s, und das liegt unter der dokumentierten Spielgrenze von 0,21 m/s.
- [Zusammenfassung] Ein Vorbehalt zu (b): Mit p ≈ 5 hängt die Kontaktzeit stark von v ab (starre Wand: T ∝ v^−(p−1)/(p+1) = v^−0,67). Mit den gemessenen p ≈ 3 der eingespielten Hämmer (Hall) wäre es v^−0,5, also schwächer. Bei kleinem v läge der Kontakt dann kürzer, wenn Q0 auf denselben ff-Wert gestimmt ist. Eine Messung der Kontaktzeit im Diskant unter ≈0,5 m/s habe ich nicht gefunden.

**Befund in einem Satz:** Die Belege sprechen vor allem für (a). Unterhalb von ≈0,2 m/s gibt es real keinen Ton, die Abbildung reicht aber bis 0,05 m/s. (b) gilt nur, wenn man die gemessenen p eingespielter Hämmer (≈3) dem unintonierten Stulov-Satz (≈5) vorzieht. Lange Kontakte von einigen Perioden bei pp an C8 sind gemessen und kein Fehler des Modells.

## Reichweite

- **Gelesen:** die Vorlesungsseiten von KTH zu Askenfelt & Jansson (stricont, motions, keybott, timing, measure) und Hall (compare, further), mit Abbildungen; Goebl 2001 und Goebl & Bresin 2003 als PDF; Stulov & Kartofelev 2009 (Acoustics High Tatras) und VITA 2010 mit Poster als PDF; die Seite von Russell & Rossing an der Penn State.
- **Nicht erreicht:** die JASA-Originale Askenfelt & Jansson 1990/1991/1993, Hall & Askenfelt 1988, Chaigne & Askenfelt 1994 (Kopie bei ntnu.no ohne Antwort), Giordano & Winans 2000, Birkett 2013, Stulov 1995/2005/2008 (alle hinter einer Bezahlschranke) und Russell & Rossing 1998 (die Seite von typeset.io lieferte HTML statt PDF). Boutillon 1988 und Yanagisawa & Nakamura wurden nicht gefunden. Alles, was diese Quellen betrifft, ist hier **[ungeprüft]** oder fehlt.

## Quellen

1. Askenfelt, A. & Jansson, E. (1990): From touch to string vibration, Seite *String contact duration and dynamic level*, Abb. 6–8. https://www.speech.kth.se/music/5_lectures/askenflt/stricont.html (Abbildungen `images/45_06.gif`, `45_07.gif`, `46_08.gif`)
2. dies., Seite *Key and hammer motions*: https://www.speech.kth.se/music/5_lectures/askenflt/motions.html · Seite *Key bottom*: https://www.speech.kth.se/music/5_lectures/askenflt/keybott.html
3. Hall, D. E. (1990): Seite *compare*, Abb. 18–20. https://www.speech.kth.se/music/5_lectures/hall/compare.html
4. Goebl, W. (2001): Melody lead in piano performance: expressive device or artifact? JASA 110(1), 563–572. https://iwk.mdw.ac.at/goebl/papers/Goebl_JASA2001_melodyLead.pdf
5. Goebl, W. & Bresin, R. (2003): Measurement and reproduction accuracy of computer-controlled grand pianos. JASA 114(4), 2273–2283. https://iwk.mdw.ac.at/goebl/papers/Goebl-Bresin_JASA2003_reproAccuracy.pdf
6. Stulov, A. & Kartofelev, D. (2010): Vibration of strings with nonlinear supports (VITA 2010), Gl. (25), (26). https://www.tud.ttu.ee/web/dmitri.kartofelev/publications/VITA2010.pdf
7. Stulov, A. & Kartofelev, D. (2009): Influence of the elastic parameters of bass hammers on the contact time duration, Acoustics High Tatras. https://www.tud.ttu.ee/web/dmitri.kartofelev/publications/34IAC2009.pdf
8. Russell, D. A.: The piano hammer as a nonlinear spring. https://www.acs.psu.edu/drussell/piano/nonlinearhammer.html

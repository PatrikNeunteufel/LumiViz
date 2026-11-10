# Hammerfilz nach Stulov: richtige Form von ε(n), τ(n) und gemessene p — Recherche

- **Anlass:** HS-1 a, S186. Im Stulov-Satz (Stulov & Kartofelev 2009, Gl. 9) steht ε = 0,9894 + 8,8·10⁻⁵ n². Damit wäre ε ab n ≈ 11 größer als 1. Das Modell rechnet seit HU3-1 c mit einem Minus, für das es keinen Beleg gibt.
- **Auftrag:** Die richtige Form von ε(n) und τ(n) finden, dazu gemessene ε, τ und p (Q0) von Klavierhämmern je Taste und offene Kopien. Was die Recherche `2026-11-10_S186_kontaktzeit_leiser_anschlag.md` schon gefunden hat, wird nicht wiederholt.
- **Datum:** 2026-11-10
- **Werkzeuge:** WebSearch, WebFetch, curl, pdftotext, pdftoppm. Jede übernommene Formel wurde am PNG der Seite geprüft. Ein Browser wurde nicht benutzt. Die Kopien liegen im Scratchpad unter `quellen_hs1\`, nicht im Repo.

Markierungen: **[wörtlich]** = in der Quelle gelesen, bei Formeln am Seitenbild geprüft · **[abgelesen]** · **[Zusammenfassung]** = meine Rechnung oder Folgerung · **[ungeprüft]** = nur Sekundärquelle oder Suchwerkzeug.

## 1 Die richtige Form: ε ist linear in n, nicht quadratisch

**Kartofelev & Stulov (2014)**, „Propagation of deformation waves in wool felt“, *Acta Mechanica* 225, DOI 10.1007/s00707-014-1109-1. Die Arbeit ist als Publication III in Kartofelevs Dissertation abgedruckt (TTU 2014, „Nonlinear Sound Generation Mechanisms in Musical Acoustics“), dort S. 72–73. Gl. (3)–(7) sind [wörtlich] und am Seitenbild geprüft (S. 72 und 73 der PDF):

- Gl. (3): F(u(t)) = F0 [u^p̂(t) − (γ/τ0) ∫₀ᵗ u^p̂(ξ) e^((ξ−t)/τ0) dξ]. Der Text heißt ε hier γ, „hereditary amplitude“.
- „The continuous variations in the hammer felt parameters versus key number N were obtained in [9] by numerical simulation of the experimental data for a whole piano hammer set.“ [9] ist Stulov 2005, Acta Acustica 91, 1086–1097.

| Gl. | Formel (am Bild geprüft) | Bereich laut Quelle |
|---|---|---|
| (4) | p̂ = 3,7 + 0,015 N | 3,72 ≤ p̂ ≤ 4,98 |
| (5) | **γ = 0,9894 + 0,000088 N** | **0,9895 ≤ γ ≤ 0,9972** |
| (6) | τ0 = 2,72 − 0,02 N + 0,00009 N² | 1,65 ≤ τ0 ≤ 2,70 (µs) |
| (7) | F0 = 15 500 e^(0,059 N) | 16 440 ≤ F0 ≤ 2 787 300 (N/mm^p̂) |

„for hammer number 1 ≤ N ≤ 88. Here, the unit for relaxation time τ0 is µs, and the unit for the instantaneous stiffness F0 is N/mm^p̂.“ [wörtlich]

Gegenprobe an Stulov & Kartofelev 2009, Gl. 9 (am Seitenbild geprüft): Dort steht wirklich „ε = 0.9894 + 8.8×10⁻⁵n²“. Das Vorzeichen ist also richtig, aber **das Quadrat ist der Druckfehler**. Die Formel für τ stimmt in beiden Arbeiten überein, ebenso die Einheit µs.

**Unabhängige Bestätigung** [Zusammenfassung, eigene Rechnung]: Stulov & Kartofelev, VITA 2010, S. VITA-3, rechnen mit dem Dreiparameter-Modell F = Q0[u^p + α d(u^p)/dt] und nennen für n = 70 „Q0 = 4270 N/mm^p; p = 4.75; hereditary parameter α = 0.395 ms“ [wörtlich, am Bild geprüft]. Für langsames Laden gilt Q0 = F0(1−γ) (Gl. 4 der Arbeit von 2009, [wörtlich]). Entwickelt man den Speicherterm, ergibt sich α = γτ0/(1−γ). Mit der linearen Form erhält man bei n = 70 γ = 0,99556, τ0 = 1,761 µs, **α = 395 µs** und Q0 = 963 758 · 0,00444 = **4279 N/mm^p**. Beide Werte treffen die gedruckten bis auf die letzte Stelle. Mit dem heutigen Minus (ε = 0,558) käme α ≈ 4 µs heraus, mit n²·10⁻⁷ etwa 280 µs.

Der Vergleich ist nicht ganz glatt: Die Polynomformel für α in VITA Gl. (25) (α = 259,5 + 0,58n + 6,6·10⁻²n² − 1,25·10⁻³n³ + 1,172·10⁻⁵n⁴, Einheit dort „[ms]“, gemeint sind offenbar µs) ergibt bei n = 70 nur 476 µs, nicht 395. Der eigene Einzelwert der Arbeit passt also zum Satz (4)–(7), nicht zu ihrem Polynom. Dieselbe Formel steht in der Dissertation als Gl. (1.8) mit „ms“.

### Abgeleitete Tafel je Hammer [Zusammenfassung, aus (4)–(7) und m nach 2009 Gl. 9 gerechnet]

| n | γ | τ0 [µs] | F0 [N/mm^p] | Q0 = F0(1−γ) | 183·e^(0,045n) | p | α = γτ0/(1−γ) [µs] | m [g] |
|---|---|---|---|---|---|---|---|---|
| 1 | 0,98949 | 2,70 | 16 442 | 173 | 191 | 3,715 | 254 | 11,00 |
| 20 | 0,99116 | 2,36 | 50 443 | 446 | 450 | 4,00 | 264 | 9,63 |
| 40 | 0,99292 | 2,06 | 164 160 | 1 162 | 1 107 | 4,30 | 289 | 8,27 |
| 60 | 0,99468 | 1,84 | 534 237 | 2 842 | 2 723 | 4,60 | 345 | 6,99 |
| 70 | 0,99556 | 1,76 | 963 758 | 4 279 | 4 270 | 4,75 | **395** | 6,38 |
| 80 | 0,99644 | 1,70 | 1 738 608 | 6 189 | 6 697 | 4,90 | 475 | 5,79 |
| 84 | 0,99679 | 1,68 | 2 201 381 | 7 062 | 8 018 | 4,96 | 520 | 5,56 |
| 88 | 0,99714 | 1,66 | 2 787 332 | 7 961 | 9 600 | 5,02 | 579 | 5,34 |

F0(1−γ) und 183·e^(0,045n) liegen bis n ≈ 70 innerhalb von ±10 % beieinander. Bei n = 88 liegt F0(1−γ) 17 % unter dem Q0-Fit. Die beiden Fits sind getrennt gerechnet.

Folgerungen für das Modell [Zusammenfassung]:
- Mit dem heutigen Minus (ε(88) ≈ 0,31) ist die Hysterese im Diskant praktisch abgeschaltet (α ≈ 0,7 µs statt ≈ 580 µs), und F0 = Q0/(1−ε) ist dort rund 250-mal zu klein.
- τ0 ≈ 1,7–2,7 µs ist viel kürzer als ein Abtastschritt bei 48 kHz (20,8 µs). Den Speicherterm sollte man deshalb exakt-exponentiell integrieren oder mit Unterschritten rechnen. Diesen Punkt bitte im Rechenkern prüfen.

## 2 Gemessene ε und τ überhaupt

| Quelle | Werte | Markierung |
|---|---|---|
| Stulov & Kartofelev 2013 (Report r307_13, „Wave propagation and dispersion in microstructured wool felt“), S. 3, Gl. (12)/(13) | „realistic values of hereditary parameters ε = 0.96 and τ0 = 10 µs“; dazu Es = 0,6 MPa (Mittel aus statischen Proben von Hammerfilz), ρ = 10³ kg/m³, Ed = 15 MPa, α0 = 0,25 ms | [wörtlich, Bild geprüft] |
| Forum Acusticum 2014 (FA_2014_felt) | dieselben Werte ε = 0,96 und τ0 = 10 µs; „hereditary amplitude is always ε < 1“ | [wörtlich, Text] |
| Kartofelev & Stulov 2014, Gl. (20) (Dissertation S. 75) | „For realistic results … γ = 0.99 and τ0 = 20 µs“, Ed = 60 MPa | [wörtlich, Text] |
| ebd., Abb. 1 | gemessene Hysteresekurven eines Hammers bei 1,31 / 1,00 / 0,74 m/s (tc = 1,7 / 2,0 / 2,5 ms), Spitzenkraft ≈ 53 / 34 / 21 N; die Taste ist nicht genannt | [abgelesen] |
| Stulov, SMAC 2003, zitiert bei J. O. Smith, *Physical Audio Signal Processing*, Abschnitt „Including Hysteresis“ | α = 248 + 1,83 n − 5,5·10⁻² n² (µs). Das ist ein anderer, älterer Fit: bei n = 70 107 µs, bei n = 88 negativ | Formel [wörtlich] aus dem Seitenquelltext, Original nicht gesehen → **[ungeprüft]** |

Stulov 1995 (JASA 97), Stulov 2004 (Acta Mech. 169), Stulov 2005 (Acta Acustica 91), Giordano & Millis, Bensa et al. 2003 und Izadbakhsh et al. 2008: **nicht erreicht**, keine Werte je Taste übernommen.

## 3 Exponent p und Q0 gemessener eingespielter oder intonierter Hämmer

Über die Vorgänger-Recherche hinaus (Hall p ≈ 2,7–3,4 im Diskant; Russell „p≈2 in the bass, to p≈4 in the treble“) wurde nichts Neues mit Werten je Taste gefunden.
- Purdue Piano Physics (Giordano): „an exponent p=3 was thought to model real hammers fairly well“, und das Potenzgesetz beschreibe reale Hämmer „not very well“ [ungeprüft, WebFetch]. Die Seite nennt keine Tasten.
- Chaigne, Chabassier & Duruflé, Vienna Talk 2015, Tab. 1 (Modell nach Chabassier, Bezugston C5): Hammermasse 7,9 g, V = 3 m/s, „Stiffness KH 2.8·10¹⁰ SI-units“ [wörtlich, Text]. Der Exponent ist im Textauszug nicht lesbar, das Bild wurde nicht geprüft. Die Felzverluste stehen dort als Dämpfungsterm R_H d/dt(…), nicht nach Stulov.
- Hall & Askenfelt 1988, Giordano & Winans 2000, Russell & Rossing 1998, Birkett 2013 und Vyasarayani 2009: nicht erreicht.

## Reichweite

- **Gesucht:** die Publikationsliste von Kartofelev (TTU), TalTech digikogu, HAL, CORE, die PASP-Seiten von J. O. Smith (CCRMA), Purdue, Vienna Talk 2015 sowie WebSearch nach Stulov 1995/2003/2005, Bensa 2003, Giordano & Winans 2000 und Chabassier 2013.
- **Gelesen** (PDF, pdftotext, Formeln am Bild): 27 PDFs von tud.ttu.ee/web/dmitri.kartofelev/publications/, darunter 34IAC2009, VITA2010, r307_13 und FA_2014_felt, außerdem die Dissertation Kartofelev (digikogu) und Vienna Talk 2015 Nr. 19. Die HTML-Seiten von CCRMA wurden im Quelltext gelesen.
- **Nicht erreicht:** HAL (hal-00873089, Chabassier 2013) und CORE haben eine Bot-Sperre (Anubis bzw. eine HTML-Seite statt PDF). air.unimi.it (Avanzini SMC04) lieferte HTML. Stulov Research Report Mech 243/02 (2002), „Experimental and computational studies of the piano hammer II“, ist nur im TalTech-Netz zugänglich. Die JASA- und Acta-Originale liegen hinter Bezahlschranken. Scribd wurde nicht benutzt.

## Quellen

1. Kartofelev, D. & Stulov, A. (2014): Propagation of deformation waves in wool felt. Acta Mechanica, DOI 10.1007/s00707-014-1109-1; abgedruckt in Kartofelev, D. (2014): Nonlinear Sound Generation Mechanisms in Musical Acoustics, Diss. TTU, S. 69–81 (Gl. 3–7 S. 72–73). https://digikogu.taltech.ee/en/Download/4b12b551-ccf9-4ac9-b66d-236f45b853f3 (Eintrag: https://digikogu.taltech.ee/en/item/1fa6b3e7-affb-4000-8736-8d288e53c28e)
2. Stulov, A. & Kartofelev, D. (2009): Piano hammer-string interaction: the influence of the elastic parameters of bass hammers on the contact time duration, Acoustics High Tatras, Gl. 4 und 9. https://www.tud.ttu.ee/web/dmitri.kartofelev/publications/34IAC2009.pdf
3. Stulov, A. & Kartofelev, D. (2010): Vibration of strings with nonlinear supports, VITA 2010, Gl. 7, 25, 26, S. VITA-3. https://www.tud.ttu.ee/web/dmitri.kartofelev/publications/VITA2010.pdf
4. Stulov, A. & Kartofelev, D.: Wave propagation and dispersion in microstructured wool felt (Report), Gl. 12–13. https://www.tud.ttu.ee/web/dmitri.kartofelev/publications/r307_13.pdf
5. Kartofelev, D. & Stulov, A. (2014): Forum Acusticum 2014. https://www.tud.ttu.ee/web/dmitri.kartofelev/publications/FA_2014_felt.pdf
6. Smith, J. O.: Physical Audio Signal Processing, „Including Hysteresis“ (zitiert Stulov, SMAC-03, Bd. I, S. 175–178). https://ccrma.stanford.edu/~jos/pasp/Including_Hysteresis.html
7. Chaigne, A., Chabassier, J. & Duruflé, M. (2015): Energy analysis of structural changes in pianos, Vienna Talk 2015, Tab. 1. https://viennatalk2015.mdw.ac.at/proceedings/ViennaTalk2015_submission_19.pdf
8. Purdue Piano Physics. https://www.physics.purdue.edu/piano/piano.html
9. Nicht erreicht: Stulov 2005, Acta Acustica 91, 1086–1097; Stulov 1995, JASA 97, 2577–2585; Chabassier, Chaigne & Joly 2013, JASA 134, 648–665 (https://hal-ensta-paris.archives-ouvertes.fr/hal-00873089); Stulov, Report Mech 243/02 (https://digikogu.taltech.ee/en/Item/33edde3e-64c5-4be9-bc56-76de1ff634a6).

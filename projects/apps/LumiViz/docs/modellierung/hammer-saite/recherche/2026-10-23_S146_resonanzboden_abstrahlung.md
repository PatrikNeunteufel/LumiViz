# Recherche S146: Resonanzboden — vom schwingenden Boden zum abgestrahlten Schall

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** Modelle für die Abstrahlung eines Resonanzbodens (Abstrahlgrad,
> Koinzidenz, modale Abstrahlung, Rayleigh-Integral, Näherungen für Flügel, Richtwirkung), umsetzbar für eine
> Synthese, offline als Impulsantwort oder modale Gewichte.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = im Volltext gesehen, [nur Sekundär]/[nur Zusammenfassung] = nicht im Original gesehen.
> **Von Claude selbst nachgelesen:** Chabassier et al. 2013 „2 400 modes …“ (in der Fassung der Autoren).
> Alles andere: nur Bericht.
> **Quellen-PDFs:** `../quellen/`, `../../koerper/quellen/` (nicht versioniert); die im Bericht genannten Pfade ins
> Scratchpad gibt es nicht mehr.
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund Recherche Boden und Körper“.

---

## Bericht: Vom schwingenden Resonanzboden zum abgestrahlten Schall

**Reichweite der Suche:** Websuche und Volltexte von arXiv, HAL, der Seite von Chaigne/Cotté, der Seite von Bank und zwei Fachartikeln aus dem Archives of Acoustics bzw. IJAV. Die PDFs habe ich mit `pdftotext` gelesen. Die Originale von Suzuki, Giordano, Berthaut, Trévisan, Bissinger und Wallace habe ich **nicht** gesehen (Bezahlschranke, 403 oder nur PostScript). Im Repo habe ich nichts geändert.

### 1. Funde

**Boutillon & Ege 2013**, „Vibroacoustics of the piano soundboard: Reduced models, mobility synthesis, and acoustical radiation regime", JSV 332(18) 4261–4279, https://arxiv.org/abs/1305.3057
- [wörtlich] Abschnitt 5.1, Gl. (29): fc = c_a²/(2π)·(D_x^H)^(−1/2). D ist dabei die Steifigkeit je Masse in m⁴s⁻², also eine normierte Größe. Werte: „≈150 in spruce plates (cut-off corners …) and ≈100 m⁴s⁻² in the homogenised central zone". Daraus folgen Koinzidenzfrequenzen von **1,5 und 1,8 kHz**. Beide liegen über der Grenze f_lim ≈ 1,1 kHz (Gl. 3: ω_g^s = (π/p)²·√D̄). Darunter verhält sich der Boden wie „a set of plates in their subsonic regime".
- [wörtlich] Rippenabstand p = 10–18 cm. Für p = 13 cm und 8 mm Fichte gilt: f_g^a ≈ 1,3 kHz, und die Strukturwellen bleiben **bis etwa 7 kHz unterschallig**. Eine homogene Ersatzplatte wäre dagegen schon ab fc ≈ 1,8 kHz überschallig.
- [wörtlich, als Zitat von Suzuki] Suzuki hat die Abstrahlgrad eines Stutzflügels von 10 Hz bis 5,4 kHz gemessen. Es gibt „no sensitive increase of the average radiation efficiency", nur einen „very smooth change" im „transition range" von 1–1,6 kHz, danach fällt sie langsam ab.
- Für das Feld oberhalb f_lim nehmen die Autoren den übrigen Boden als ruhend an, also als Schallwand.

**Suzuki 1986**, „Vibration and sound radiation of a piano soundboard", JASA 80(6) 1573–1582
- [nur Sekundär] Die Fundstelle steht wörtlich in den Literaturlisten von Chaigne 2013 und Boutillon/Ege 2013. Laut Sekundärquellen: Modalanalyse und Oberflächenintensität, unter etwa 90 Hz kaum Abstrahlung, erste Mode bei etwa 49,5 Hz. Die Messkurve selbst habe ich nicht gesehen.

**Chaigne, Cotté & Viggiano 2013**, „Dynamical properties of piano soundboards", JASA 133(4) 2456–2466, https://perso.ensta-paris.fr/~cotte/files/Chaigne_Cotte_al_jasa133_2013%20piano%20soundboard.pdf
- [wörtlich] Abschnitt IV: „A Rayleigh integral is used … thus the soundboard is supposed to be in baffled conditions. These conditions are not realistic for a piano soundboard in the complete audio range." Für 1–3 kHz sei der Fehler klein, denn die Wellenlänge von höchstens 30 cm ist klein gegen den Boden.
- [wörtlich] Lokalisierte Moden „broaden the cone of directivity and reduce the number of lobes". Ein Feld von **4×2 Monopolen** an den Extremen des gemessenen Geschwindigkeitsfelds (Abstand 11,3 × 13,3 cm) gibt die Richtwirkung bei 1542 Hz gut wieder.

**Chabassier, Joly & Chaigne 2013**, „Modeling and simulation of a grand piano", JASA 134, 648, HAL hal-00873089 (offen)
- [wörtlich] Die Luft wird mit den linearisierten Euler-Gleichungen gerechnet (c_a = 340 m/s, ρ_a = 1,29 kg/m³). Die Zarge ist ein starres Hindernis, der Rand ist mit PML abgeschlossen. Der Boden rechnet modal (2400 Moden, gerechnet mit 420 000 Freiheitsgraden), die Luft mit „slightly more than 9·10⁷ dofs", Zeitschritt 10⁻⁶ s. Für 10 kHz rechnen sie „60 millions of degrees of freedom" vor. Fig. 17 trennt die Dämpfung durch Abstrahlung von den übrigen Verlusten. Das ist offline als Referenz denkbar, für eine Synthese aber viel zu teuer.
- [wörtlich] Chaigne/Chabassier/Durufle, Vienna Talk 2015: Abstrahlung in eine „closed box" mit PML, 2,3·10⁷ Freiheitsgrade in der Luft.

**Zou & Crocker 2009**, „Sound Power Radiated from Rectangular Plates", Archives of Acoustics 34(1) 25–39 (offen) — das allgemeine Werkzeug
- [wörtlich] Gl. (5): Fernfeld nach dem Rayleigh-Integral, p = −ikρc·e^{ikr}/(2πr)·∫∫u(x,y)·exp(−i(αx/Lx+βy/Ly))dxdy. Gl. (17): **W = uᵀCu**, mit der reellen, symmetrischen, positiv definiten Kopplungsmatrix C nach Gl. (15). Gl. (18): σ = W/(ρc·LxLy·⟨|u|²⟩).
- [wörtlich] Nach Nikiforov ist der Strahlungswiderstand einer eingespannten Platte bei tiefen Frequenzen doppelt so groß wie der einer frei aufliegenden.

**Bank 2007**, „Direct Design of Parallel Second-Order Filters for Instrument Body Modeling", ICMC 2007, I 458–465, http://www.mit.bme.hu/~bank/publist/icmc07.pdf (frei abrufbar)
- [wörtlich] Er trennt die Eigenschaften „radiation" und „termination" des Bodens. Die Pole liegen logarithmisch (Gl. 4/5, R = 0,98) oder kommen aus einem gewarpten Entwurf. Die Gewichte bestimmt er per kleinster Quadrate gegen die Ziel-Impulsantwort.
- [wörtlich] Aufwand: Eine FIR braucht bei 44,1 kHz etwa 2000 Taps, für hohe Töne sogar ≥10 000. Die parallele Filterbank braucht Ordnung 50–200. Für eine Aufnahme aus 2 m Abstand, die stark nicht minimalphasig ist, nimmt er 50 Abschnitte plus 200 FIR-Taps. Die Rechnung ist „around four times less" aufwendig als ein klassisches IIR-Filter.
- [wörtlich] Stereo geht mit denselben Polen: je Kanal ändern sich nur die Ausgangsgewichte. Morphing zwischen Böden ist möglich.

**Giordano 1998**, „Sound production by a vibrating piano soundboard: Experiment", JASA 104, 1648 — [nur Zusammenfassung] Das Verhältnis Druck/Geschwindigkeit hat sein Maximum bei etwa 1 kHz und fällt unter einigen 100 Hz und über etwa 5 kHz ab. Die Theorie dazu steht bei Jiang & Giordano 1999.

**Berthaut, Ichchou & Jézéquel 2003**, Applied Acoustics 64(11) 1113–1136 — [nur Sekundär] Wellen sind zwischen den Rippen eingeschlossen. Zur Abstrahlung habe ich nichts gefunden.

**Trévisan, Ege & Laulagnet 2017**, „A modal approach to piano soundboard vibroacoustic behavior", JASA, HAL hal-01456083 — [nur Zusammenfassung] Die gerippte Platte wird in eine größere, frei aufliegende Rechteckplatte mit Federn eingebettet. Die Abstrahlung kommt über deren **Strahlungsimpedanzen**, also modal mit Schallwand. Ergebnis: Schallleistung bis 3 kHz in Abhängigkeit vom Koppelpunkt.

**Wallace 1972**, JASA 51(3) 946–952, und **Wallace 1987**, „acoustic radiation damping of the modes of a rectangular panel", JASA 81(6) — [nur Sekundär] Abstrahlgrad je Mode einer frei aufliegenden Platte in Schallwand, mit Asymptoten für tiefe Frequenzen. Die Rangfolge ist ungerade-ungerade ≫ gemischt ≫ gerade-gerade. Die Formeln selbst habe ich nicht im Volltext gesehen.

**Bissinger** (Violine) — [nur Zusammenfassung] Abstrahleigenschaften je Mode, gerechnet aus experimenteller Modalanalyse mit der Randelementmethode. Er trennt Abstrahl- und Materialdämpfung. Den Volltext von 2008 habe ich nicht erreicht.

### 2. Zugang und Lizenz
Frei zugänglich: arXiv (Boutillon/Ege), HAL (Chabassier, Trévisan), die Seite von Bank, Archives of Acoustics, das Autoren-PDF von Chaigne (mit ASA-Copyright-Vermerk). Hinter der Bezahlschranke bei ASA/Elsevier: Suzuki, Giordano, Wallace, Berthaut, Bissinger. Ein Nachbau von Formeln ist rechtlich unproblematisch.

### 3. Empfehlung (meine Einschätzung)
Für einen modalen Boden halte ich folgenden Weg für passend:
- **Offline:** die Modenformen φ_n auf einem Raster abtasten. Dann je Mode die komplexe Übertragung zum Hörort mit dem Rayleigh-Integral in Schallwand rechnen, H_n(ω) ∝ iωρ/(2π)·∫φ_n e^{−ikR}/R dS, und zusätzlich die Leistungsmatrix C wie bei Zou/Crocker. Die Diagonale ergibt σ_n und die Abstrahldämpfung je Mode (wie bei Wallace 1987 und Chabassier Fig. 17).
- **Zur Laufzeit:** Druck p(t) = Σ_n g_n · (Modenantwort_n), mit einem frequenzabhängigen Gewicht g_n je Mode und Hörort. Das ist strukturell dieselbe Form wie Banks Parallelfilter. Die Pole sind die Bodenmoden, die Ausgangsgewichte kommen aus dem Rayleigh-Integral statt aus einem Fit, und ein zweiter Hörort kostet nur weitere Gewichte.
- **Grenzen:** Unter ~200–300 Hz, wo die Wellenlänge größer als der Boden ist, und für den akustischen Kurzschluss zwischen Ober- und Unterseite stimmt die Schallwand nicht. Dort ein Dipol-Korrekturfaktor oder ein einmaliger offline-Lauf mit BEM/FEM samt Zarge (Chabassier-Ansatz) als Eichung. Die Koinzidenz bei etwa 1,5–1,8 kHz sollte nach Suzuki und Boutillon/Ege **keinen** scharfen Sprung erzeugen.
- **Oberhalb ~3–5 kHz:** statt einzelner Moden eine statistische Gewichtung oder der FIR-Teil nach Bank.

### 4. Nicht gefunden
- Die Formeln von Wallace und Leppington für σ_mn im Wortlaut.
- Suzukis σ(f)-Kurve als Zahlen.
- Ein veröffentlichtes Modell für **offenen Deckel**, Raum unter dem Boden oder Reflexion am Deckel des Flügels.
- Die Abstrahlformulierung von Giordano/Jiang.
- Bissingers Abstrahlung je Mode im Detail.
- Rechenaufwand für das Rayleigh-Integral je Mode in einer Synthese-Arbeit.

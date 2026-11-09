# Recherche IC1: Becken und Gong — Physik, Rechenverfahren, Rechenkosten

> **Auftrag:** Klasse 6 (Crash, Ride, Hi-Hat, Gong, Tamtam): lineare Moden, Nichtlinearität (von Kármán,
> Wellenturbulenz, Tonhöhengleiten), Rechenverfahren mit Kosten, Hi-Hat, Ride/Crash/China/Splash, Schlag — mit Zahlen;
> Prüfung der Vorschätzung „eher Deluxe“.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug] = vom
> Abrufwerkzeug als Zitat/Feldinhalt; [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (rund 15 Suchen, 55 Abrufe; HAL-Volltexte, MDPI, DAFx25-Server, NESS-Seite
> nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Bericht IC – Klasse 6 „Becken und Gong“: Physik, Rechenverfahren, Zahlen, Rechenkosten

**Auftrag:** Physik und Rechenverfahren von Becken und Gongs (Crash, Ride, Hi-Hat, Gong, Tamtam), mit Zahlenwerten und Rechenkosten. Recherche ohne Schreiben in Dateien.
**Datum:** 2026-10-26 (gilt auch als Abrufdatum aller Quellen unten)

**Reichweite:**
- Etwa 15 Websuchen und etwa 55 Abrufe.
- Orte: HAL-API, Crossref-API, arXiv-API und arXiv-Abstract-Seiten, ENSTA (perso.ensta.fr/~touze), DAFx-Archiv, NTNU (DAFx-15), Edinburgh ERA, J-STAGE (AST), Forum-Acusticum-Archiv (dael.euracoustics.org), acoustics.org, auditory.org, pubs.aip.org (ein Meeting-Abstract) sowie eine Autorenseite der École polytechnique.
- Sprachen: Englisch und Französisch (nur HAL-Titel).
- **Nicht erreichbar:**
  - HAL-Volltexte: Anubis-Sperre, etwa bei Jossic 2018.
  - MDPI (403): Brezas 2024 *Vibration* und Kaselouris 2023 *Acoustics*.
  - sam.ensam.eu (403).
  - DAFx25-Server (Verbindung abgelehnt).
  - vkgong.ensta-paristech.fr (DNS-Fehler).
  - NESS-Seite „cymbals-and-gongs“ (404).
  - Fletcher & Rossing sowie Rossing *Science of Percussion Instruments*: Bücher, nicht frei, nicht gelesen.
- **Vom Werkzeug zwischengespeicherte PDFs**, alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793039130153-c7ojz3.pdf`: Ducceschi & Touzé, DAFx-15
  - `webfetch-1793039182007-kuzfvz.pdf`: Bilbao et al., DAFx23 „Real-Time Gong Synthesis“
  - `webfetch-1793039225349-qdjy02.pdf`: Chaigne, Touzé & Thomas 2005, AST
  - `webfetch-1793039435176-ybk5jq.pdf`: Brezas et al., Forum Acusticum 2023
  - `webfetch-1793039480999-txef0i.pdf`: Hamilton-Vorlesungsnotizen (ohne Ertrag)
  - `webfetch-1793039538224-zpfezf.pdf`: Bilbao 2010, IEEE TASLP
  - `webfetch-1793039580699-1g4z54.pdf`: Sekiguchi & Samejima 2023, AST (Hi-Hat)
  - `webfetch-1793039716585-w6mz4r.pdf`: Jossic et al. 2017, JASA
  - `webfetch-1793039791015-1u8lvu.pdf`: Zheleznov & Bilbao, Forum Acusticum 2026
  - `webfetch-1793039840126-fvwdwp.pdf`: Harrison & Hill 2013, Proc. IOA
  - `webfetch-1793039865972-qrn81f.pdf`: Ducceschi, Touzé & Bilbao, SMAC 2013
- **Wie gelesen:** Die PDFs habe ich lokal mit `pdftotext` gelesen. Seitenangaben nennen die PDF-Seite und, wo erkennbar, die gedruckte Seite.

**Kennzeichnung der Belege:**
- [wörtlich, selbst gelesen, S. x]: im PDF selbst gelesen.
- [wörtlich, Werkzeug]: Zitat oder Feldinhalt aus der API-Antwort.
- [nur Zusammenfassung]: Zusammenfassung eines Werkzeugs; das ist kein Beleg.

---

## 1. Lineare Moden: Frequenzen, Dämpfung, Form, Material

**Modenzahl und Modendichte**
- Wilbur & Rossing 1996, 16-Zoll-Becken: „some 300 modes have been observed“ [wörtlich, Werkzeug]. Sie passen ein abgewandeltes Chladni-Gesetz an, mit zwei Parametern (Rossing 1982) und mit drei Parametern (Perrin 1985) [nur Zusammenfassung].
- Fletcher & Bassett 1979, angeschlagenes Becken: „very complex with more than 600 identifiable frequencies“ [wörtlich, Werkzeug]. „Chladni patterns were formed for sixty of the strongest modes“ [wörtlich, Werkzeug].
- Fletcher & Bassett 1980, 18-Zoll-Becken („French cymbal“), Hüllkurve des Spektrums in Terzbändern:
  - Anstieg „about 7 dB per octave, to a peak in the 630-Hz region“ [wörtlich, Werkzeug]
  - „Another broad peak occurs in the 4-kHz band“ [wörtlich, Werkzeug]
  - danach „16 dB per octave“ Abfall; bei 16 kHz 33 dB unter dem Maximum [wörtlich, Werkzeug].

**Gemessene Modenfrequenzen eines 8-Zoll-Beckens (Brezas et al. 2023)**
- Becken aus Messing MS63, nicht B20.
- Gemessen mit ESPI (elektronische Speckle-Interferometrie):
  - (2,0) 210 Hz, (3,0) 555 Hz, (4,0) 1040 Hz, (5,0) 1520 Hz
  - (3,1) 1630 Hz, (4,1) 1740 Hz, (2,1) 1760 Hz, (5,1) 2030 Hz
  - [wörtlich, selbst gelesen, S. 3, Tabelle 1; Spaltenversatz durch die Textextraktion, Zuordnung von mir rekonstruiert]
- Geometrie:
  - Dicke „vary in a range from 1.2 mm to 1.4 mm“ [wörtlich, selbst gelesen, S. 2]
  - „radius is 101.6 mm and its height is 28 mm“ [wörtlich, selbst gelesen, S. 2]
- Mittlerer Fehler der FEM: 7,4 / 7 / 9,2 % [selbst gelesen, S. 3].
- Folge für das Modell: Kuppe und Wölbung verschieben die Moden stark gegenüber der flachen Platte.

**Form und Wölbung (Chaigne, Touzé & Thomas 2005)**
- Eigenfrequenzen sind „highly sensitive to small changes of curvature“ [wörtlich, selbst gelesen, S. 2 (gedr. 404)].
- Beim Tamtam sind die Zwillingsmoden deutlich verstimmt („significant differences“) [wörtlich, selbst gelesen, S. 2].
- Gemessene Kugelkappe mit innerer Resonanz 1:2: Mode (6,0) bei etwa 111 Hz, Mode (0,1) bei etwa 224 Hz [wörtlich, selbst gelesen, S. 3, Abb. 2].

**Material: B20-Bronze**
- Kein gemessener Kennwert für B20 gefunden.
- Die einzigen Zahlen sind Rechenparameter im Hi-Hat-Modell von Sekiguchi & Samejima 2023 (14-Zoll-Becken):
  - Dichte 8 860 kg/m³, E = 110 GPa, ν = 0,33
  - Dicke H = 1,0 mm, Radius R = 0,1778 m
  - [wörtlich, selbst gelesen, PDF-S. 5 (gedr. 356), Tabelle 1]
  - Herkunft der Werte nennt die Tabelle nicht.

**Dicke und Durchmesser in den Synthesepapieren (Modellwahl, keine Messung)**
- Touzé, Seite „modalsynthesis“:
  - Becken: 40 cm Durchmesser, 0,5 mm Dicke
  - großer Gong: 80 cm, 1 mm, außerdem 1 m
  - [nur Zusammenfassung, Werkzeug-Paraphrase]
- Touzé, Seite „tapercymbals“:
  - „a circular plate of radius 0.2 m“ [wörtlich, Werkzeug]
  - innen 1 mm, Rand 1 / 0,7 / 0,4 mm [nur Zusammenfassung]

**Dämpfung**
- Gemessenes Gesetz für eine Platte mit h = 0,5 mm (Humbert), wiedergegeben in DAFx-15: ξᵢ = 8 × 10⁻³ ωᵢ⁻⁰·⁴ [wörtlich, selbst gelesen, S. 6, Bildunterschrift Abb. 4].
- DAFx-15 betont, dass Dämpfung „mode by mode“ frei gesetzt werden kann [selbst gelesen, S. 1].
- Fletcher & Bassett 1979 messen Abklingraten mit einem „10-Hz bandwidth filter“ [wörtlich, Werkzeug]. Zahlen enthält der Abstract nicht.
- **Gemessene Abklingzeiten je Mode für Crash oder Ride habe ich nicht gefunden.**

## 2. Nichtlinearität, Wellenturbulenz, Schwellen, Tonhöhengleiten

**Zwei Mechanismen (Legge & Fletcher 1989)**
- Erstens eine „upward cascade of energy from the low-frequency modes“ [wörtlich, Werkzeug]. Ursache ist die Kopplung an Stellen „of sharp change in shape“ [wörtlich, Werkzeug].
- Zweitens ein Übergang zu „multiple fractional subharmonics or even chaotic motion“ [wörtlich, Werkzeug].
- Beide zeigen Hysterese. Das Spektrum entwickelt sich „over a period of several seconds“ [wörtlich, Werkzeug].

**Übergangsszenario (Chaigne, Touzé & Thomas 2005)**
- Zwei Bifurkationen: periodisch, dann quasiperiodisch, dann chaotisch [selbst gelesen, S. 1–2].
- Die erste tritt „suddenly for a given force threshold“ auf [wörtlich, selbst gelesen, S. 2].
- Neue Frequenzen entstehen nach Kombinationsregeln aus quadratischer Nichtlinearität (Wölbung) [selbst gelesen, S. 2].
- Bei dünnen Becken treten Kopplung und Energieübertrag schon bei kleinen Amplituden auf, vor jedem Gleiten [selbst gelesen, S. 3].
- Lyapunov-Exponenten an Beckensignalen: mindestens einer positiv; Chaos ist nachgewiesen [selbst gelesen, S. 6].
- Touzé & Chaigne 1999 sehen „mode-locking and quasi-periodicity“ vor dem Chaos [wörtlich, Werkzeug].

**Schwellen in Zahlen (nur Simulation)**
- Ducceschi & Touzé DAFx-15, Platte 21 × 18 cm, 1 mm:
  - „weicher“ Schlag: P = 100 N, T_wid = 1 ms
  - „harter“ Schlag: P = 300 N, T_wid = 0,8 ms
  - [wörtlich, selbst gelesen, S. 7, Bildunterschrift Abb. 6]
  - Gültig ist das Schema bis „2 - 3 times the thickness“ Auslenkung [wörtlich, selbst gelesen, S. 6].
- SMAC 2013, Stahlplatte 0,4 × 0,6 m, 1 mm:
  - Schläge mit p₀ = 15, 100, 200 N
  - „As the excitation grows, energy is passed to higher parts of the spectrum“ [wörtlich, selbst gelesen, S. 4]
- Allgemein gilt: Nichtlinearität wird hörbar ab Auslenkungen in der Größenordnung der Dicke (Chaigne 2005, S. 1) [selbst gelesen].
- **Gemessene Schwellen in N oder m/s für echte Crash- oder Ride-Becken nicht gefunden.**

**Wellenturbulenz (wave turbulence)**
- Boudaoud et al. 2008 (PRL 100, 234504): Spektren „may be rescaled on a single curve“, aber unvereinbar mit der Theorie schwacher Turbulenz [wörtlich, Werkzeug, arXiv-Abstract].
- Humbert et al. 2013 (EPL): trotz Dämpfung turbulent, „with a cut-off frequency“, die von der eingespeisten Leistung abhängt [wörtlich, Werkzeug].
- Ducceschi, Cadot, Touzé & Bilbao 2014 (Physica D 280–281, DOI 10.1016/j.physd.2014.04.008):
  - Eine Front läuft zu hohen Frequenzen.
  - „the frequency front increases as t^{1/3}“ bzw. „evolves linearly with time“, je nach Anregungsfall [wörtlich, Werkzeug]
  - Eine realistische Imperfektion hat wenig Einfluss [nur Zusammenfassung].
- Humbert, Josserand, Touzé & Cadot 2016 (Physica D 316, 34–42): ein **phänomenologisches Modell** für die zeitliche Entwicklung des Spektrums.
  - Grenzfälle: Rayleigh-Jeans-Gleichverteilung und Kolmogorov-Zakharov-Spektrum
  - „Frequency-dependent damping laws can easily be accounted for“ [wörtlich, Werkzeug]
  - Wichtig für Frage 3 (billiger Ansatz).

**Tonhöhengleiten (pitch glide) bei Gongs**
- Fletcher 1985 (JASA 78, 2069): Die Grundfrequenz einer flachen Kugelkappe fällt „by as much as 50%“, wenn die Amplitude etwa die Schalenhöhe erreicht. Bei mehr als etwa dem Doppelten liegt sie über der Kleinsignalfrequenz [wörtlich, Werkzeug]. Größere Dicke verringert die Verschiebung [nur Zusammenfassung].
- Chaigne 2005 zitiert Fletcher:
  - großer Operngong mit flacher Mitte: Abwärtsgleiten „by as much as three semitones“ (≈ 300 Cent)
  - kleinere, stark gewölbte Gongs gleiten aufwärts
  - [wörtlich, selbst gelesen, S. 2]
  - Härtend bedeutet Gleiten nach unten, weichend Gleiten nach oben [selbst gelesen, S. 3].
- Jossic et al. 2017 (JASA 141(6), 4567), Xiaoluo, Durchmesser ≈ 23 cm (PDF-S. 4, gedr. 4570):
  - Grundmode ≈ 451 Hz
  - „upward pitch glide from 370 to 450 Hz“ [wörtlich, selbst gelesen, PDF-S. 2 (gedr. 4568)]
  - Eigene Rechnung: das sind **≈ 339 Cent aufwärts**.
- Jossic et al. 2018 (JASA 144, 431): Ein einzelner nichtlinearer Mode reicht nicht. Grund sind „1:2 internal resonances“ [wörtlich, Werkzeug].
- Becken: Gleiten ist „frequently heard on thick gongs, but not on thin cymbals“ [wörtlich, selbst gelesen, Chaigne 2005, S. 3].

## 3. Rechenverfahren und Kosten

**Tabelle der Rechenzeiten (Sekunden Rechenzeit je Sekunde Klang)**

| Verfahren | Fall | Rechenzeit / s Klang | Beleg |
|---|---|---|---|
| FDTD, von Kármán, implizit (SMAC 2013) | Stahl 0,4×0,6 m, 1 mm, 100 kHz, Gitter 51×76 | „about two hours“ (≈ 7 200 s); einfacheres Schema „about an hour“ | [wörtlich, selbst gelesen, S. 4]; MATLAB, Core i5 650 |
| Modal, von Kármán, N_w = 150 (SMAC 2013) | gleiche Platte, 20 kHz | „about 10 minutes per second“ (≈ 600 s) | [wörtlich, selbst gelesen, S. 4] |
| Modal, DAFx-15 (Störmer-Verlet, explizit) | 21×18 cm; N = 100 Moden bis 9 kHz, 50 Moden in der Ebene | „about 8 times real-time“ (≈ 8 s); sinusförmig angeregt ≈ 1,5 s | [wörtlich, selbst gelesen, S. 8 bzw. S. 6]; MATLAB, i7 2,4 GHz |
| Modal, Touzé-Seite (groß, N bis 1000 Moden, 25–80 kHz) | schnelle Simulation | „approx. 1 minute CPU per seconds of sounds“ | [wörtlich, Werkzeug] |
| Modal, Nguyen & Touzé (Becken mit Verjüngung) | N = 800, 100 kHz | „N=800 modes“, „100 kHz“; Zeit nicht genannt | [wörtlich, Werkzeug] |
| FD-Schale (Bilbao 2010) | Kugelschale, 32–44,1 kHz | „between 1 s and 160 s per second of audio output“ | [wörtlich, selbst gelesen, PDF-S. 8 (gedr. 879)]; MATLAB, Laptops |
| FD-Platte, Echtzeit (Bilbao, Webb, Wang & Ducceschi, DAFx23) | Stahl 0,5 mm, A = 0,03–0,06 m², Gitter z. B. 25×31, 44,1 kHz, C++ -O3 -mavx2 | 0,150–0,993 s (Tabelle 1); z. B. A = 0,06 m² auf i7-1260P: 0,484 s | [wörtlich, selbst gelesen, S. 7, Tab. 1] |

**Speicher und Skalierung**
- Modale Kopplung: Der Tensor wächst mit N_w⁴. Bei N_w ≈ 150 belegt er „about 1 GB of memory“ [wörtlich, selbst gelesen, SMAC 2013, S. 4].
- Seine Berechnung mit N_F = 484 dauert „about 0.5 hours“ (einmalig, offline) [wörtlich, selbst gelesen, S. 4].
- Durch Symmetrien sind über die Hälfte der Koeffizienten null oder doppelt [selbst gelesen, S. 4–5].
- Zheleznov & Bilbao 2026: „O(Mx4) = O(Nx4) coupling terms“ bei der modalen Rechnung [wörtlich, selbst gelesen, S. 4]. Pseudospektral lässt sich das auf O(Nx² log Nx) senken [selbst gelesen, S. 4].
  - Sie verweisen darauf, dass das modale Modell „requires a GPU for real-time applications“, mit Verweis auf Diaz & Sandler, DAFx25 [wörtlich, selbst gelesen, S. 1].
  - Eigene Zeiten haben sie noch nicht: „Additional work is required to evaluate real-time capability“ [wörtlich, selbst gelesen, S. 5].
  - Nicht begutachtet (Hinweis auf S. 1).

**Stabilität**
- Explizites Störmer-Verlet ist laut DAFx23 „highly prone to explosive instability“ [wörtlich, selbst gelesen, S. 1].
- DAFx23 verwendet ein energieerhaltendes Schema mit quadratisierter Energie (SAV/IEQ), das explizit lösbar ist.
- Der Biharmonik-Löser braucht bis zu 76 % der Zeit [selbst gelesen, S. 7].

**Echtzeit-Grenze in DAFx23**
- Die Flächen sind klein. Ein Crash mit 40 cm Durchmesser hat A ≈ 0,126 m², mehr als doppelt so viel wie die größte Tabellenzeile (eigene Rechnung).
- Die Rechenzeit „tracks the total surface area A“ [wörtlich, selbst gelesen, S. 6].
- Spektrogramme zeigen bei großer Amplitude „wideband noise – the crash“ und bei der großen Platte (0,16 m²) einen „swell“ über einige hundert ms [selbst gelesen, S. 7].

**GPU**
- Nur indirekt belegt: Diaz & Sandler DAFx25 (JAX, GPU) laut Suche [nur Zusammenfassung]. Den Server erreichte ich nicht.
- Bilbao & Torin 2014 (GPU, modulare Umgebung) ohne Zahlen [nur Zusammenfassung].

**Billigere physiknahe Ansätze**
- Cook, PhISEM bzw. stochastische modale Synthese: Resonanzfilter, angeregt durch gesteuertes Rauschen [nur Zusammenfassung].
- Phänomenologisches Spektralmodell von Humbert et al. 2016 (siehe Abschnitt 2): Es liefert die zeitliche Entwicklung der spektralen Energie bei gegebener Einspeisung und Dämpfung. Es ist ein Kandidat für „Energieübertrag zwischen Bändern“ [nur Zusammenfassung des Abstracts].
- Das Prinzip bestätigt Rossing (Laienfassung ASA 2000): Moden „furnish energy to other modes“; „The sound spectrum changes with time“ [wörtlich, Werkzeug].
- **Ein ausgearbeitetes Becken-Syntheseverfahren „gemessene Moden plus Rauschteppich mit Energietransport“ habe ich in DAFx, JASA oder JAES nicht gefunden.** Kein Treffer ist kein Beweis.

## 4. Hi-Hat

Quelle: Sekiguchi & Samejima 2023, AST 44(5), 352–359.
- Modell: zwei dünne Kugelschalen, finite Differenzen in Polarkoordinaten; der Stock ist eine Punktmasse.
- Zwei Kontaktmodelle, wörtlich „a spring model and an algorithmically mimicked inelastic collision model“ [wörtlich, selbst gelesen, S. 1 (gedr. 352)]. Das Stoßmodell kam dem Original näher [selbst gelesen, S. 8].
- Parameter [selbst gelesen, PDF-S. 5, Tabelle 1]:
  - Federsteife Stock K_st = 3,0 × 10⁹, Federsteife zwischen den Becken K_inter = 1,0 × 10⁹ (nichtlineare Feder mit Exponent)
  - Gitter Δr = 8,5 × 10⁻³ m
  - Spektrogramm des echten Hi-Hats bei 48 kHz aufgenommen
- Abstand der Becken d = 5,0 × 10⁻⁶ m gegen 0 m: Bei 0 m klingt es insgesamt schneller ab, vor allem tief [selbst gelesen, PDF-S. 7].
- Negativer Abstand (Fußdruck, statische Verformung) war nicht rechenbar: „numerical divergence“ [wörtlich, selbst gelesen, PDF-S. 8].
- Kippbewegung fehlt. Ausblick: Hertz-Kontakt.
- Schlagstelle: je näher am Rand, „the brighter the tone will be“ [wörtlich, selbst gelesen, PDF-S. 8].
- Rechenzeit nennt der Text nicht (Suche nach Zeitangaben ohne Treffer).
- Harrison & Hill 2013, 14-Zoll-Hi-Hat: unter der Mitte deutlich leiser, offen wie geschlossen [selbst gelesen, S. 3]. Das betrifft die Abstrahlung, kein Modell.

## 5. Ride, Crash, China, Splash

- Harrison & Hill 2013 (Polardiagramme, 17/19-Zoll-Crash, 21-Zoll-Ride, 14-Zoll-China, 14-Zoll-Hi-Hat):
  - Tiefe Frequenzen (100–250 Hz) fallen nach dem Anschlag leicht ab, hohe steigen [selbst gelesen, S. 4].
  - China: Pegel steigen, dann bei 500 ms Abfall um „approximately 15 dB“; schneller als beim Crash, wegen geringerer Masse [wörtlich, selbst gelesen, PDF-S. 6].
  - Ride: Richtwirkung 250–500 Hz hängt davon ab, ob Spitze auf Bogenmitte oder Kante auf Rand trifft [selbst gelesen, S. 4].
  - „By 500 ms all cymbals reach steady state vibratory patterns“ [wörtlich, selbst gelesen, S. 5].
- Curley et al. 2014 (Crash 14 und 16 Zoll, Schlagroboter): Unterschiede in „modal density, decay time, and beating“ [wörtlich, Werkzeug], ohne Zahlen.
- Nguyen & Touzé 2019: Verjüngung und Form bestimmen den glockigen Klang (Kuppe) gegen den Crash-Klang [nur Zusammenfassung des HAL-Abstracts].
- **Gemessene Unterschiede Kuppe gegen Rand am Ride (Abklingzeiten, Aufbau des „wash“) und Splash-Daten nicht gefunden.** Brezas 2024 (Splash, CC BY 4.0) war wegen 403 nicht lesbar.

## 6. Schlag: Stock, Schlägel, Schlagstelle

- Kontaktzeit Stock–Becken: **nicht gemessen gefunden.**
  - Modellwahl Touzé: Becken „1 ms“, Gong 6 ms Halbbreite eines angehobenen Kosinus [nur Zusammenfassung].
  - DAFx-15: 0,8–1 ms [selbst gelesen, S. 7].
  - DAFx23: T = 2 ms (kleine Platte) bzw. 4 ms („swell“, große Platte) [selbst gelesen, S. 7].
  - Bilbao 2010: Einzelheiten nicht gelesen.
- Zum Vergleich Stock auf Trommelfell: 5–8 ms bei mezzoforte, laut Dahl-Review [nur Zusammenfassung].
- Jossic 2017, Schlägel am Xiaoluo:
  - Impulse p ≈ 31 g·m/s (klein, kein Gleiten) bis 83 g·m/s (Gleiten) [wörtlich, selbst gelesen, PDF-S. 6 (gedr. 4572)]
  - Nützlich als Maßstab für weiche Schlägel.
- Wirbel und Swell: DAFx23 zeigt die langsame Wanderung der Energie über einige hundert ms [selbst gelesen, S. 7]. Gemessene Daten zu Wirbeln nicht gefunden.

---

## Quellen (Fundort, frei oder nicht, Rechtsvermerk im Wortlaut, Abruf 2026-10-26)

1. **Ducceschi, M.; Touzé, C. (2015): „Simulations of Nonlinear Plate Dynamics: An Accurate and Efficient Modal Algorithm“, Proc. DAFx-15, Trondheim.**
   - Fundort: NTNU-PDF.
   - Frei. Im PDF kein Rechtsvermerk (Suche nach „copyright“ und „creative commons“: 0 Treffer).
2. **Bilbao, S.; Webb, C.; Wang, Z.; Ducceschi, M. (2023): „Real-Time Gong Synthesis“, Proc. DAFx23, Copenhagen.**
   - Fundort: dafx.de/paper-archive/2023/DAFx23_paper_19.pdf. Frei.
   - Vermerk S. 1: „Creative Commons Attribution 4.0 International License“.
3. **Ducceschi, M.; Touzé, C.; Bilbao, S. (2013): „Sound synthesis of gongs obtained from nonlinear thin plates vibrations: comparison between a modal approach and a finite difference scheme“, SMAC 2013.**
   - Fundort: perso.ensta.fr/~touze/PDF/ducceschi_smac2013.pdf. Frei.
   - Vermerk S. 1: „Creative Commons Attribution 3.0 Unported License“. HAL hal-01138174 führt „hal-authorisation-v1“.
4. **Chaigne, A.; Touzé, C.; Thomas, O. (2005): „Nonlinear vibrations and chaos in gongs and cymbals“, Acoust. Sci. & Tech. 26(5), 403–409.**
   - Fundort: perso.ensta.fr/~touze/PDF/ChaosGong.pdf. Frei lesbar.
   - Im PDF kein Rechtsvermerk gefunden. J-STAGE als frei zugänglich gemeldet [nur Zusammenfassung].
5. **Bilbao, S. (2010): „Percussion synthesis based on models of nonlinear shell vibration“, IEEE TASLP 18(4), 872–880.**
   - Fundort: Edinburgh ERA. Frei lesbar.
   - Vermerk S. 1: „© 2010 IEEE“. ERA-Seite: „all material is copyright © The University of Edinburgh 2026“, sofern nicht anders angegeben.
6. **Sekiguchi, S.; Samejima, T. (2023): „Physical modeling and sound synthesis of the hi-hat“, Acoust. Sci. & Tech. 44(5), 352–359.**
   - Fundort: J-STAGE. Frei lesbar (verschlüsseltes PDF, Kopieren erlaubt).
   - Vermerk S. 1: „©2023 The Acoustical Society of Japan“.
7. **Jossic, M.; Mamou-Mani, A.; Chomette, B.; Roze, D.; Ollivier, F.; Josserand, C. (2017): „Modal active control of Chinese gongs“, JASA 141(6), 4567–4578.**
   - Fundort: yakari.polytechnique.fr (Autorenseite).
   - Vermerk S. 1: „© 2017 Acoustical Society of America“. Nicht frei lizenziert.
8. **Brezas, S. et al. (2023): „Experimental and Computational Vibroacoustic Study of Cymbals“, Forum Acusticum 2023, DOI 10.61782/fa.2023.0309.**
   - Frei. Vermerk S. 1: „Creative Commons Attribution 3.0 Unported License“.
9. **Zheleznov, V.; Bilbao, S. (2026): „Explicit and Stable Pseudospectral Time-Domain Method for the Föppl–von Kármán Equations“, Forum Acusticum 2026, Graz.**
   - Frei. Vermerk S. 1: „Creative Commons Attribution 3.0 Unported License“. Nicht begutachtet.
10. **Harrison, J.J.; Hill, A.J. (2013): „A Scientific Approach to Microphone Placement for Cymbals in Live Sound“, Proc. IOA 35(2).**
    - Fundort: adamjhill.com.
    - Kein Rechtsvermerk im PDF gefunden.
11. **Metadaten und Abstracts über Crossref** (nur Abstract, Volltext nicht frei, kein Lizenzfeld):
    - Legge & Fletcher 1989, JASA 86, 2439 (10.1121/1.398451)
    - Fletcher 1985, JASA 78, 2069 (10.1121/1.392664)
    - Jossic et al. 2018, JASA 144, 431 (10.1121/1.5038114)
    - Fletcher & Bassett 1980, JASA 68 S111 (10.1121/1.2004552)
    - Bilbao 2008, „Cymbal Synthesis“, JASA 123, 3521
    - Curley et al. 2014, JASA 136, 2285
    - Touzé & Chaigne 1999, JASA 106, 2142
    - Kuratani et al. 2016, JASA 140, 3161
    - Lizenzfeld vorhanden nur bei Kaselouris et al. 2023 *Acoustics* 5(1) und Brezas et al. 2024 *Vibration* 7(1): „creativecommons.org/licenses/by/4.0/“ (Feldinhalt). Volltext 403.
12. **Fletcher, H.; Bassett, I.G. (1979): „A study of cymbal vibrations“, JASA 66 S18.** Fundort: pubs.aip.org, nur Abstract.
13. **Wilbur, C.; Rossing, T.D. (1996): „Vibrational modes of cymbals and Chladni's law“, ASA 132.** Fundort: auditory.org, nur Abstract, kein Vermerk.
14. **HAL-Einträge mit Lizenzfeld „hal-authorisation-v1“:**
    - hal-01134639 (JSV 344, 313–331, 2015)
    - hal-02050810 (Nguyen & Touzé, JASA 145, 977–988, 2019)
    - hal-01135260, hal-01134801, hal-00838867, hal-01257109
    - Abweichend: hal-01134659 „creativecommons.org/licenses/by/4.0/“; hal-01325042 „CC BY-NC 4.0“.
    - Volltexte wegen Anubis nicht gelesen.
15. **arXiv 0810.0891 und 1709.09884:** Lizenz „arxiv.org/licenses/nonexclusive-distrib/1.0/“.
16. **Touzé, C.: Webseiten „modalsynthesis.html“ (Stand 8.9.2014) und „tapercymbals.html“.** Kein Rechtsvermerk.

## Nicht gefunden (kein Treffer ist kein Beweis)

- Gemessener E-Modul und gemessene Dichte von B20-Beckenbronze. Gemessene Dickenprofile handelsüblicher Crash- und Ride-Becken.
- Gemessene Abklingzeiten oder Q-Werte je Mode für Crash und Ride.
- Gemessene Schwellen (Schlagkraft oder -geschwindigkeit) für Crash bzw. Wellenturbulenz an echten Becken und Tamtams.
- Daten zu Kuppe gegen Rand beim Ride und zum Aufbau des „wash“. Messdaten zu Splash und China außer Harrison & Hill.
- Kontaktzeit Stock–Becken (gemessen).
- Rechenzeiten des Hi-Hat-Modells und von Nguyen & Touzé (N = 800).
- GPU-Rechenzeiten in Zahlen.
- Cent-Werte für den Daluo (Abwärtsgleiten). Es gibt nur „three semitones“, zitiert über Chaigne aus Fletcher.
- VK-Gong-Code: Lizenz und Ort ungeklärt (Server nicht erreichbar).

## Eigene Einschätzung (kein Beleg)

**Welches Modell trägt für vorab gerechnete Schläge**
- Die Rahmenbedingung „Sekunden statt Stunden je Schlag“ ist mit einem **energieerhaltenden FD-Schema für die von-Kármán-Platte nach DAFx23** heute erfüllbar.
- Kleine Platten rechnen dort schneller als Echtzeit in C++. Für 40–50 cm Durchmesser schätze ich linear nach der Fläche etwa 1–2 s Rechenzeit je Sekunde Klang auf einem Laptop. Für einen Schlag von 5–10 s sind das 10–20 s; das ist vertretbar.
- Modal mit voller Kopplung (N⁴) ist bei mehr als einigen hundert Moden zu teuer und zu speicherhungrig. Für Gongs mit Gleiten reichen dagegen wenige Moden, je nach Instrument 50–150, als modale Rechnung.
- Ehrlich gesagt bilden alle schnellen Schemata eine **flache Rechteckplatte aus Stahl** ab. Eine Kuppe, Wölbung und Verjüngung, also das, was den Charakter von Ride und Crash ausmacht, verlangt das Schalenmodell (Bilbao 2010: 1–160 s je s in MATLAB) oder Rayleigh-Ritz mit variabler Dicke (Nguyen & Touzé: 800 Moden, 100 kHz). Das ist teurer und nicht auf Echtzeit getrimmt.

**Trifft „Deluxe“ zu?**
- Teilweise. Die **Rechenzeit ist nicht mehr das Hindernis**, denn vorab gerechnet reichen Sekunden bis Minuten je Schlag.
- „Deluxe“ trifft zu für **Modelltreue und Datenlage**:
  - reale Form und Material sind kaum gemessen
  - es gibt keine gemessenen Moden und Dämpfungen für Crash und Ride
  - die Hi-Hat-Kontaktmechanik ist nur vorläufig modelliert
- „Chaos“ ist kein Kostenproblem, sondern ein Problem der Reproduzierbarkeit (siehe Risiken).

**Billige Stufe passend zum LumiViz-Muster „Moden plus statistischer Teil“**
- Gemessene oder per FEM gerechnete lineare Moden für die tiefen Anteile.
- Dazu ein Rauschteppich in Bändern, dessen Energie nach einer Transportgleichung von tief nach hoch wandert. Vorbild wäre das phänomenologische Modell von Humbert et al. 2016 mit Front t^{1/3} bzw. linear, Abschneidefrequenz abhängig von der Schlagenergie und Dämpfung je Band.
- Beim Gong kommt Gleiten je Mode hinzu, als Duffing-Rückgrat ω = ω₀(1 + CA²), mit Vorzeichen je Instrument.
- Kosten: vernachlässigbar.
- Physiknah ist das, gerechnet nicht: Kalibrieren ließe es sich an einigen von-Kármán-Läufen. Dieser Weg ist **nicht als fertiges Verfahren belegt**.

**Risiken**
1. **Determinismus:** Die Rechnung ist chaotisch (positive Lyapunov-Exponenten). Bitgleiche Ergebnisse gibt es nur mit fester Rechenreihenfolge, ohne fast-math, mit gleicher SIMD-Breite und gleichem Compiler. Schon FMA gegen keine FMA verändert den Verlauf nach kurzer Zeit sichtbar, statistisch bleibt er gleich. Für vorab gerechnete Töne heißt das: Ergebnis cachen und versionieren, nicht als „immer gleich reproduzierbar über Plattformen“ zusagen. Tests sollten Spektralstatistik prüfen, nicht Abtastwerte.
2. **Stabilität:** Explizites Störmer-Verlet explodiert bei großen Amplituden. Nötig sind energieerhaltende Schemata (SAV/IEQ, DAFx23).
3. **Bandbreite:** Grobe Gitter wie 25×31 oder Modenabbruch (N_w = 150) schneiden die hohen Anteile ab, die den Crash tragen (SMAC 2013: „high frequency content … really poor“).
4. **Parameterlage:** Material, Dicke und Wölbung echter B20-Becken sind ungemessen. Ohne eigene Messung (Moden, Abklingzeiten) bleibt jede Rechnung nur ein plausibles Becken, kein bestimmtes Modell.
5. **Hi-Hat:** Fußdruck (negativer Abstand) ist im einzigen gefundenen Modell numerisch nicht rechenbar. Kippbewegung und Hertz-Kontakt fehlen. Das Hi-Hat ist der riskanteste Teil der Klasse.

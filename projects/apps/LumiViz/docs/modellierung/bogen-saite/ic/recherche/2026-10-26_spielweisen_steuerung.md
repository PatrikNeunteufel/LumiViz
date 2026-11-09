# Recherche IC1: Bogen-Saite — Spielweisen und ihre Steuerung

> **Auftrag:** Klasse 3: Vibrato (Rate, Tiefe, Form, AM; Beleg für die Vorgabe 5,5 Hz aus SK10 a), Bogenparameter beim
> Spielen und Abbildung der Expression, Artikulationen (Détaché bis Pizzicato, Portamento), Dämpfer, Ensemble.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug] = vom
> Abrufwerkzeug als Zitat (meist Crossref-/HAL-Abstract); [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (23 Suchen, rund 45 Abrufe; DiVA/HAL-Volltexte, SAGE, Acta Acustica, KTH-QPSR,
> IOA nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` und `../offene-fragen.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Bericht IC – Klasse 3 Bogen-Saite: Spielweisen und ihre Steuerung (gemessene Vorgabewerte)

**Auftrag:** Vibrato, Bogenparameter, Artikulationen, Dämpfer und Ensemble der Streicher. Gesucht sind gemessene Werte mit Quelle als Vorgabe für ein physikalisches Modell.
**Datum:** 2026-10-26

**Reichweite**
- **Umfang:** 23 Websuchen und etwa 45 Abrufe mit WebFetch.
- **Orte:**
  - Crossref-API und Semantic-Scholar-API für Abstracts von JASA, JRME und IEEE
  - HAL-API (archives-ouvertes.fr)
  - arXiv
  - knutsacoustics.com (Guettler, KTH-Arbeiten)
  - viennatalk.mdw.ac.at
  - idmil.org (McGill)
  - QMUL (Dixon)
  - Repository Cambridge (Woodhouse)
  - UNSW (Wolfe)
  - lam.jussieu.fr
  - Stellenbosch
- **Sprachen:** Suchen auf Englisch. Deutsch nur über den Titel von Meyer 1992, Volltext nicht gefunden.
- **Nicht erreichbar:**
  - DiVA (KTH) und HAL-Volltexte: Bot-Sperre „Anubis“. Dadurch fehlen die Schoonderwaldt-Dissertation 2009 und die Demoucron-Dissertation 2008 (tel-00349920).
  - journals.sagepub.com (403), acta-acustica.edpsciences.org (403).
  - speech.kth.se: STL-QPSR-PDFs mit Fehler 500, die Archivseite lädt nur per JavaScript.
  - ioa.org.uk: Guettler & Askenfelt, Kinetik des Spiccato (404).
  - repositorioslatinoamericanos.uchile.cl (leer).
  - Für JASA 1986/1989 (Askenfelt) liefern die APIs kein Abstract.
- **Vom Werkzeug zwischengespeicherte PDFs** (alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):
  - `webfetch-1793036900486-b5c01r.pdf`: Guettler, Dissertation KTH 2002
  - `webfetch-1793037053510-w2lw64.pdf`: Guettler, „Bow Notes“, Proc. IOA 1997
  - `webfetch-1793037596520-on28nr.pdf`: Guettler, „Some Physical Properties of the Modern Violin Bow“
  - `webfetch-1793037032358-spfr5g.pdf`: Demoucron, Vienna Talk 2010
  - `webfetch-1793036970961-8kzvli.pdf`: Schoonderwaldt, Guettler & Askenfelt, Acta Acustica 2008
  - `webfetch-1793037162759-uifd92.pdf`: Schoonderwaldt, Sinclair & Wanderley, ENACTIVE 2007
  - `webfetch-1793037148307-zyg5n6.pdf`: Schoonderwaldt, Vienna Talk 2010
  - `webfetch-1793036971643-im6auk.pdf`: Chudy, Dixon & Pérez, ICA 2013
  - `webfetch-1793037227587-wd12wa.pdf`: Mansour, Woodhouse & Scavone, angenommene Fassung
  - `webfetch-1793037413167-ili07h.pdf`: d'Alessandro & Castellengo, JASA 1994
  - `webfetch-1793037505118-5d0w0v.pdf`: Schubert & Wolfe, JIMS 2013
  - `webfetch-1793037452687-pgkwtj.pdf`: Böck & Widmer, ISMIR 2013 (ohne Bezug zum Thema)
- **Selbst gelesen:** Die zwischengespeicherten PDFs habe ich mit `pdftotext` auf die Standardausgabe gelesen und dabei nichts geschrieben. Das ist die Grundlage aller Angaben mit dem Vermerk „selbst gelesen“. Seitenangaben sind die gedruckte Seite, sonst steht „PDF-S.“ dabei.
- **Kennzeichnung der Angaben:**
  - [wörtlich, selbst gelesen, S. x]: aus der Textebene selbst gelesen.
  - [wörtlich, Werkzeug]: Wortlaut, den das Abrufwerkzeug als Zitat geliefert hat. Meist stammt er aus dem Crossref- oder HAL-Abstract.
  - [nur Zusammenfassung]: Das ist kein Beleg.

---

## Quellen

Alle Abrufe am 2026-10-26. „Rechtsvermerk nicht gesehen“ heißt: Auf der abgerufenen Seite stand keiner.

**Q1.** Geringer, John M.; Allen, Michael L. (2004): „An Analysis of Vibrato among High School and University Violin and Cello Students“. *Journal of Research in Music Education* 52(2), 167–178, doi:10.2307/3345438.
- Fundort: Crossref-API (Abstract). Die SAGE-Seite gab 403 zurück.
- Nicht frei. Rechtsvermerk nicht gesehen.

**Q2.** MacLeod, Rebecca B. (2008): „Influences of Dynamic Level and Pitch Register on the Vibrato Rates and Widths of Violin and Viola Players“. *JRME* 56(1), 43–54, doi:10.1177/0022429408323070.
- Fundort: Crossref-Abstract. Nicht frei. Rechtsvermerk nicht gesehen.

**Q3.** MacLeod, Rebecca B. (2010): „A Pilot Study of Relationships between Pitch Register and Dynamic Level and Vibrato Rate and Width in Professional Violinists“. *String Research Journal* 1(1), 75–83, doi:10.1177/194849921000100105.
- Fundort: Crossref-Abstract. Nicht frei. Rechtsvermerk nicht gesehen.

**Q4.** Fletcher, H.; Sanders, L. C. (1967): „Quality of Violin Vibrato Tones“. *JASA* 41(6), 1534–1544, doi:10.1121/1.1910516.
- Fundort: Crossref-Abstract. Nicht frei. Rechtsvermerk nicht gesehen.

**Q5.** Mellody, Maureen; Wakefield, Gregory H. (2000): „The time-frequency characteristics of violin vibrato: Modal distribution analysis and synthesis“. *JASA* 107(1), 598–611, doi:10.1121/1.428326.
- Fundort: Crossref-Abstract. Nicht frei. Rechtsvermerk nicht gesehen.

**Q6.** Hu, Steven; Kim, Sophia H.; Kim, Helena H.; Mackay, Hugo; Heller, Eric J. (2025): „Influence of string register locations on vibratos among violoncellists“. arXiv:2512.18162v1.
- Frei. Vermerk: „License: CC BY 4.0“ (arXiv-HTML-Seite).

**Q7.** Prame, E. (1997): „Vibrato extent and intonation in professional Western lyric singing“. *JASA* 102(1), 616–621, doi:10.1121/1.419735.
- Fundort: Crossref-Abstract. Nicht frei.

**Q8.** Guettler, Knut (2002): „The Bowed String – On the Development of Helmholtz Motion and On the Creation of Anomalous Low Frequencies“. Doktorarbeit, KTH Stockholm, TRITA-TMH 02:3.
- Fundort: knutsacoustics.com/files/thesis-guettler-020607.pdf.
- Frei abrufbar. Rechtsvermerk im Text nicht gefunden; das PDF ist gegen Änderungen geschützt.

**Q9.** Guettler, Knut (1997): „Bow Notes“. Proc. I.O.A. Vol 19 Part 5.
- Fundort: knutsacoustics.com. Frei abrufbar. Rechtsvermerk nicht gefunden.

**Q10.** Guettler, Knut (ohne Jahr): „Some Physical Properties of the Modern Violin Bow“.
- Fundort: knutsacoustics.com/files/some-bow-properties-cism.pdf. Frei abrufbar. Weder Jahr noch Rechtsvermerk im Text.

**Q11.** Demoucron, Matthias (2010): „The Violinist's Bestiary: Characterizing Bowing Parameters of Technical Bow Strokes“. Proceedings of the Second Vienna Talk, 19.–21. Sept. 2010, mdw Wien, S. 55–58.
- Fundort: viennatalk.mdw.ac.at. Frei. Rechtsvermerk nicht gefunden.

**Q12.** Schoonderwaldt, Erwin; Guettler, Knut; Askenfelt, Anders (2008): „An Empirical Investigation of Bow-Force Limits in the Schelleng Diagram“. *Acta Acustica united with Acustica* 94, 604–622, doi:10.3813/AAA.918070.
- Fundort: knutsacoustics.com.
- Vermerk auf S. 604: „© S. Hirzel Verlag · EAA“. Nicht frei lizenziert.

**Q13.** Schoonderwaldt, Erwin; Sinclair, Stephen; Wanderley, Marcelo M. (2007): „Why do we need 5-DOF force feedback? The case of violin bowing“. ENACTIVE/07.
- Fundort: idmil.org. Frei abrufbar. Rechtsvermerk nicht gefunden.

**Q14.** Schoonderwaldt, E. (2009): „The player and the bowed string: Coordination of bowing parameters in violin and viola performance“. *JASA* 126(5), 2709–2720, doi:10.1121/1.3203209.
- Fundort: Crossref-Abstract. Nicht frei.

**Q15.** Chudy, M.; Dixon, S.; Pérez (2013): Messungen der Bogenparameter beim Cello, ICA 2013.
- Fundort: webspace.eecs.qmul.ac.uk/s.e.dixon/pub/2013/Chudy-Dixon-Perez-ICA2013.pdf.
- Der Titel ist in der Textebene nicht lesbar und daher nicht geprüft. Rechtsvermerk nicht gefunden.

**Q16.** Demoucron, Matthias (2008): „On the control of virtual violins – Physical modelling and control of bowed string instruments“. Dissertation, Verteidigung 2008-11-24, tel-00349920.
- Fundort: HAL-API, nur das Abstract; der Volltext ist gesperrt.
- Lizenzfeld: „https://about.hal.science/hal-authorisation-v1/“.

**Q17.** Rasamimanana, N.; Bernardin, D.; Wanderley, M. M.; Bevilacqua, F. (2007): „String Bowing Gestures at Varying Bow Stroke Frequencies: A Case Study“. hal-01161225.
- Fundort: HAL-API (Abstract). Lizenzfeld: hal-authorisation-v1.

**Q18.** Mansour, H.; Woodhouse, Jim; Scavone, G. P.: „Enhanced Wave-Based Modelling of Musical Strings. Part 1: Plucked Strings“. *Acta Acustica united with Acustica* 102.
- Fundort: Repository Cambridge, Handle 1810/261228.
- Vermerk: „this item's license is described as http://www.rioxx.net/licenses/all-rights-reserved“.

**Q19.** Wolfe, Joe: „Articulation and vibrato on the violin“.
- Fundort: newt.phys.unsw.edu.au/jw/violinarticulation.html.
- Vermerk: „© Joe Wolfe / J.Wolfe@unsw.edu.au“; das Zeichen war in der Ausgabe verstümmelt.

**Q20.** Schubert, Emery; Wolfe, Joe (2013/2014): „The rise of fixed pitch systems and the slide of continuous pitch …“. *Journal of Interdisciplinary Music Studies* 7(1&2), Art. #13071201, doi:10.4407/jims.2014.06.001.
- Fundort: UNSW-Reprint. Rechtsvermerk nicht geprüft.

**Q21.** d'Alessandro, Christophe; Castellengo, Michèle (1994): „The pitch of short-duration vibrato tones“. *JASA* 95(3), 1617ff.
- Fundort: lam.jussieu.fr.
- Vermerk am Seitenfuß: „Redistribution subject to ASA license or copyright“.

**Q22.** Sole, Ignasi (2026): „Spectrographic Portamento Gradient Analysis …“. arXiv:2604.22037.
- Lizenz CC BY 4.0 laut arXiv-Seite [Werkzeug].

**Q23.** Pàmies-Vilà, Montserrat; Saitis, Charalampos (2024): „Timbral effects of col legno tratto techniques on bowed cello sounds“. *JASA* 155 (3_Suppl.), A60, doi:10.1121/10.0026797.
- Fundort: Crossref. Nur Tagungs-Abstract.

**Q24.** Evangelista, Marcus Vinicius da Silva; Freire, Sérgio (2025): „Audio analysis of the effects of heavy violin practice mutes: spectral and loudness changes“. *Acta Acustica* 9, Art. 45.
- Lizenz-URL laut Crossref: creativecommons.org/licenses/by/4.0.

**Q25.** Ohne Autor auf der Seite: „Violin vibrato technique and its implication for pitch perception“, Abstract.
- Fundort: ich.music.mcgill.ca/research/icmc98/. Rechtsvermerk nicht gesehen.

---

## 1. Vibrato

**Mechanismus:** Der Finger rollt periodisch auf der Saite. Das moduliert die Grundfrequenz, also eine Frequenzmodulation (frequency modulation, FM). Über die stark frequenzabhängige Abstrahlung des Korpus wird daraus eine je Teilton verschiedene Amplitudenmodulation (amplitude modulation, AM).

**Rate**
- Geringer & Allen 2004, Violine und Cello, Schüler und Studenten (Q1):
  - „the mean rate of vibrato was approximately 5.5 Hz“ [wörtlich, Werkzeug]
  - Kein signifikanter Unterschied nach Instrument oder Erfahrung [nur Zusammenfassung].
- Mellody & Wakefield 2000, Violine (Q5): „single sinusoid with a mean rate of 5.9 Hz“ [wörtlich, Werkzeug].
- MacLeod 2010, vier professionelle Violinsolisten (Q3): „mean vibrato rate was 6.63 Hz“ [wörtlich, Werkzeug].
- Hu et al. 2025, Cello (Q6), [wörtlich, Werkzeug]:
  - kommerzielle Aufnahmen: Mittel 6.475 Hz (s = 0.4192)
  - TinySOL-Aufnahmen: Mittel 5.261 Hz
- Fletcher & Sanders 1967 (Q4): Im Crossref-Abstract steht verstümmelt „about sit times per second“ [wörtlich, Werkzeug]. Gemeint sind vermutlich sechs; das ist nicht belegt.
- Wolfe (Q19): Die Hüllkurve „varies about six times per second“ [wörtlich, Werkzeug].
- **Register und Lage:**
  - MacLeod 2008 (Q2): „0.32 Hz faster and approximately 26 cents wider during high pitches“ [wörtlich, Werkzeug]. Gemeint ist: hohe gegen tiefe Töne.
  - Fall eines Profis, Allen et al. 2009: 6,3 Hz in der 6. Lage gegen 5,7 Hz in der 1. Lage [nur Zusammenfassung, Sekundärquelle].
- Übersicht der Literatur, Spanne 4–10 Hz, Mittel 5–7 Hz [nur Zusammenfassung, Sekundärquelle ohne sicheren Autor].

**Tiefe (Breite)**
- MacLeod 2010 (Q3): „Mean width of the soloists' vibrato was 63 cents“ [wörtlich, Werkzeug]. Ob das von Spitze zu Spitze gemeint ist, ist im Abstract nicht gesagt.
- MacLeod 2008 (Q2): „increased vibrato width approximately 4 cents in the forte passages“ [wörtlich, Werkzeug]. Gemeint ist: gegenüber piano.
- Geringer & Allen 2004 (Q1): „The mean width of violin vibratos was larger than cello vibratos.“ [wörtlich, Werkzeug] Zahlen stehen nur im Volltext.
- Meyer 1992, Acustica 76, 283–291, zitiert nach Hu et al. (Q6): „typically range from 0.2 to 0.35 semitones in depth“ [wörtlich, Werkzeug, Zitat aus zweiter Hand]. Das Original habe ich nicht gesehen.
- Kontrabass, brasilianische Masterarbeit: 5,0 Hz und 0,96 % [nur Zusammenfassung, Quelle nicht erreichbar].
- Mick 2024/2025, Kontrabass: breiter und schneller in hohen Registern, ohne Zahlen [nur Zusammenfassung über Q6].

**Form und Lage zum Ton**
- Fletcher & Sanders (Q4) messen die Tiefe als Abweichung „in cents, from the note being played“ [wörtlich, Werkzeug].
- Mellody & Wakefield (Q5): Ein einzelner Sinus beschreibt die FM gut [wörtlich, siehe oben]. Die Form ist damit belegt.
- Geringer & Allen (Q1): „Pitch oscillations during vibrato were alternations both above and below conceived pitch.“ [wörtlich, Werkzeug]
- Gegenposition aus der Pädagogik (Q25): Gelehrt wird „vibrate only below and up to the principal tone“ (Bang 1923) [wörtlich, Werkzeug]. In einer FFT-Voranalyse lagen die Grundfrequenzen leicht unter dem Zielton [nur Zusammenfassung].
- MacLeod & Geringer in The Strad: leicht nach unten, „approximately 2-5 cents“ [nur Zusammenfassung, Zeitschrift, kein Original].

**Begleitende AM**
- Fletcher & Sanders (Q4), beide [wörtlich, Werkzeug]:
  - „an intensity-level variation of the harmonics at the same vibrato rate“
  - „the intensity-level variation was greatly different for different harmonics“
- Mellody & Wakefield (Q5), beide [wörtlich, Werkzeug]:
  - „the absence of amplitude modulation causes marked changes in both sound quality …“
  - „the absence of frequency modulation has little effect on the perceptual response“
- Wolfe (Q19): Die spektrale Hülle „varies strongly during one cycle of vibrato“ [wörtlich, Werkzeug].

**Einsatzverzögerung:** Nicht gefunden (siehe unten).

**Gesang zum Vergleich, ein Satz:** Prame 1997 (Q7) misst bei Opernsängern „±71 cent“ mittlere Auslenkung („mean vibrato extent … ±34 and ±123 cent“) [wörtlich, Werkzeug]. Das ist ein Mehrfaches der Streicherwerte.

**Vorgeschlagener Ort (Einschätzung):**
- Vibrato-Spur: Tiefe in Cent.
- Rate als Ausstattung je Instrument und Spieler, wobei Register und Dynamik leicht modulieren.
- Die AM entsteht im Modell von selbst, wenn die FM durch eine realistische Stegadmittanz und Abstrahlung läuft. Die Messungen sagen, dass genau dieser Anteil den Klang trägt. Sie wäre also kein eigener Parameter.

---

## 2. Bogenparameter beim Spielen

**Belegt ist, was wie wirkt**
- Schoonderwaldt et al. 2007 (Q13), S. 2: „the amplitude of the string vibration is proportional to vB/β“ [wörtlich, selbst gelesen, S. 2; das β fehlt in der Textebene]. Lauter wird es durch „increasing bow velocity and/or decreasing bow-bridge distance“. Die Obertöne kommen „mainly … by increasing bow force“ [wörtlich, selbst gelesen, S. 2].
- Crescendo in einem ganzen Bogen, Schoonderwaldt et al. 2007, S. 2–3: Die Geschwindigkeit war am Anfang des Abstrichs niedrig (pp) und stieg bis zum Bogenwechsel (ff). Gleichzeitig ging der Bogen näher an den Steg, und die Kraft stieg [nur eigene Zusammenfassung des selbst gelesenen Texts].
  - In der Abbildung: Frosch (pp) → Spitze (ff) → Frosch (pp).
  - Die Neigung (tilt) läuft mit der Kraft. Die Schrägstellung (skewness) dient dazu, den Kontaktpunkt wandern zu lassen.
- Askenfelt nach Chudy et al. (Q15), S. 1: Geschwindigkeit und Steg-Abstand sind „the main player controls of the sound level“. Die Kraft „does not contribute to the amplitude of sound“ und steige nur mit, „to adjust to higher bow velocities“ [wörtlich, selbst gelesen, PDF-S. 1].
- Schoonderwaldt 2009 (Q14): drei Dynikstufen „(pp, mf, and f) on all four strings“ und „clear trends in the coordinated variations“ [wörtlich, Werkzeug]. Zahlen nur im Volltext; nicht erreichbar.

**Zahlen**
- Kraft:
  - „Normal bow forces range between 0.5 and 1.5 N for the violin“, nach Askenfelt 1986 (Q9) [wörtlich, selbst gelesen, PDF-S. 2].
  - Sautillé: Fmax von „around 0.5 N in piano to more than 3 N in forte“ (Q11) [wörtlich, selbst gelesen, S. 57].
- Steg-Abstand β bei der Violine (Q12): Das gewählte Raster „represent[s] a substantial part of the control space in normal violin playing“ und deckt den „β range (about 1/30 to 1/6)“ ab, in Abständen von 11 bis 55 mm [wörtlich, selbst gelesen, S. 608]. Die Messgeschwindigkeiten der Bogenmaschine waren 5, 10, 15 und 20 cm/s (S. 604).
- Cello, sechs fortgeschrittene Cellisten, D-Dur-Tonleiter (Q15):
  - Legato mit ganzem Bogen bei 80 bpm, Noten zu 0,8 s:
    - Abstand im Mittel 5.89 cm, Spanne 2.43–11.24 cm [wörtlich, selbst gelesen, PDF-S. 3]
    - Geschwindigkeit 16.62 cm/s, Spanne 6.14–32.39 [wörtlich, selbst gelesen, PDF-S. 3]
  - Bei 160 bpm, Noten zu 0,34 s: „up to 40.48 cm/s on average“, Spanne 12.27–89.88 cm/s [wörtlich, selbst gelesen, PDF-S. 4]
- Bogenlänge (Q10):
  - Violinbogen „ca 740-750 mm from cap to the tip“ [wörtlich, selbst gelesen, PDF-S. 7]
  - Haar „normal length (650 mm)“ bei 60 N Spannung [wörtlich, selbst gelesen, PDF-S. 8]
  - Für Cello und Kontrabass habe ich keine Werte gefunden.

**Bogenwechsel**
- Demoucron 2015, Acta Acustica 101(2), „The performance of bow changes“ [nur Zusammenfassung, nicht gelesen].
- Guettler, Vienna Talk: Ein schnellerer Wechsel sei möglich, „some 14%“ [nur Zusammenfassung].
- Gute Einschwinger nach Guettler 2002 (Q8):
  - Akzeptanzgrenze für Vorläufer-Transienten an der leeren G-Saite: „50 ms (10 nominal periods)“ bei verlängerten Perioden (S. 9) [wörtlich, selbst gelesen]
  - „90 ms (18 nominal periods) for multiple-slip attacks“ (S. 10) [wörtlich, selbst gelesen]
  - In echtem Spiel waren 44 % von 1694 Einsätzen „perfekt“, also mit Helmholtz-Bewegung nach weniger als 5 ms (S. 10) [nur eigene Zusammenfassung des gelesenen Texts].

**Abbildung von Expression 0…1 auf (Kraft, Geschwindigkeit, β)**
- Belegt ist nur die Richtung: v steigt, β fällt, F steigt.
- Belegt sind außerdem die Rollen: v/β bestimmt den Pegel, F zusammen mit β die Klangfarbe.
- Eine veröffentlichte Formel habe ich nicht gefunden. Demoucron 2008 (Q16) modelliert laut Abstract „variations in dynamic level, bow changes“ mit „analytical functions“ [wörtlich, Werkzeug]; der Volltext ist gesperrt.
- Maestre et al. 2010, IEEE TASLP 18(4), 855–871, nutzt Bézier-Segmente, laut Q11 S. 55 [nur Zusammenfassung].
- Vorschlag in „Eigene Einschätzung“.

---

## 3. Artikulationen

Alle Angaben zum Ort sind Einschätzung.

**Détaché und Legato**
- Détaché „on the string“ verlangt plötzliche Halte am Notenende, die die Saite abdämpfen (Q9, PDF-S. 2) [nur eigene Zusammenfassung, selbst gelesen].
- Bei Legato cantabile muss die Kraft stabil bleiben (ebd.).
- Für Saitenwechsel gibt es ein geometrisches Modell, Schoonderwaldt & Demoucron 2012, hal-00811209 [nur Titel und Abstract].
- Ort: Note (Détaché) beziehungsweise Kette (Legato, wie an der Gitarre).

**Martelé**
- „pinch“ vor der Bewegung; die Kraft fällt „from about 1.5 N to less than 0.5 N“ (Q11) [wörtlich, selbst gelesen, S. 57].
- Schwingungen im Kraftverlauf um „13 Hz“, vermutlich Bogenantwort (S. 58) [wörtlich, selbst gelesen].
- Modell: konstanter Teil, dann Sinusabfall; die Geschwindigkeit als „cos-cos“-Glocke.
- Ort: Note.

**Spiccato, Sautillé, Ricochet** (springender Bogen)
- Sprungrate des Violinbogens: „typically between 6 and 35 Hz for impacts at hair lengths of 8 to 60 cm from the frog“ bei 60 N Haarspannung (Q8, S. 17) [wörtlich, selbst gelesen].
- Aufbauzeit „in the order of 30 ms“, danach verlässt der Bogen die Saite, und die Saite klingt exponentiell aus (Q8, S. 15) [wörtlich, selbst gelesen].
- Vier Phasen (Q8, S. 16) [eigene Zusammenfassung, selbst gelesen]:
  - Aufbau
  - freies Abklingen
  - „forced decay“ durch leicht aufliegenden Bogen
  - fester Griff vor dem Gegenstrich
- Ricochet: „normally 10 - 20 per second“ (Q9, PDF-S. 2–3) [wörtlich, selbst gelesen].
- Tiefste Drehmode des Bogens: „near 6 Hz (at the frog)“ bis „near 30 Hz (at the tip)“ (Q9, PDF-S. 6) [wörtlich, selbst gelesen].
- Sautillé:
  - Kraft glockenförmig, Spitze „around 0.74 N“ im Beispiel G auf D-Saite, mf, 90 bpm [wörtlich, selbst gelesen, Q11 S. 56].
  - Geschwindigkeit als halber Sinus je Strich.
  - Kontaktdauer T als Modellparameter. Ein Zahlenwert steht nicht im Text, nur Kurven in der Abbildung.
- Ort: Note (Spiccato) oder Passage (Sautillé, Ricochet als Gruppe).

**Tremolo**
- Mit steigender Strichfrequenz kippt das Geschwindigkeitsprofil „from a rectangle shape to a sinus shape“ (Q17) [wörtlich, Werkzeug].
- Eine Rate in Hz habe ich nicht gefunden.
- Wolfe (Q19): Die Strichrichtung wechselt schnell, „the bowing direction is changed rapidly“ [wörtlich, Werkzeug]. Im Satz laufen die Bögen in zufälliger Phase [nur Zusammenfassung].
- Ort: Note.

**Col legno**
- Battuto: „loud initial transient“, danach schneller Abfall (Q19) [wörtlich, Werkzeug].
- Tratto (Q23):
  - Der Kontakt des Holzes bringt Rauschen: „the contact between the scrubbing stick and the string introduces noise“ [wörtlich, Werkzeug].
  - Die Neigung variiert stark zwischen Haar, Haar und Holz, nur Holz.
- Keine Zahlen.
- Ort: Note, eventuell Passage.

**Sul ponticello und sul tasto**
- Nur qualitativ belegt (Q19), beide [wörtlich, Werkzeug]:
  - ponticello: „lots of high harmonics“
  - tasto: „weaker high harmonics“
- Ein β-Wert für ponticello oder tasto fehlt; das normale Spiel liegt bei β ≈ 1/30…1/6 (Q12).
- Ort: Passage, als Verschiebung von β.

**Flageolett:** Nichts gefunden.

**Pizzicato**
- Melka 1970: „in violin pizzicato no tone build-up lasted longer than 4 ms“ (Q8, S. 3) [wörtlich, selbst gelesen].
- Wolfe (Q19): Der Ton „decays rapidly away“ [wörtlich, Werkzeug].
- Mansour et al. (Q18):
  - Zeitbereichsmodell aus gemessener Saitendämpfung und Korpusantwort des Cellos, „validated against measured pizzicato notes“ [wörtlich, Werkzeug].
  - Q-Faktoren je Mode für Cello-Saiten liegen dort als Abbildung vor (S. 4).
  - Der Verlauf der Q-Faktoren weicht von dem bei Gitarren- und Cembalosaiten ab (S. 4, eigene Zusammenfassung, selbst gelesen).
- Zupfstelle und Abklingzeiten in Zahlen: nicht gefunden.
- Ort: Note.

**Portamento und Glissando** (Schubert & Wolfe, Q20)
- F0-Änderung über „at least 20 ms“, um hörbar zu sein (S. 3).
- Leech-Wilkinson: „swoop … generally shorter than 300 ms“ (S. 3).
- Glissando-Schwelle nach Hart et al. 1990: „Gthr = 0.16/(T2)“ in Halbtönen pro Sekunde (ST/s), also 0,16/T² (S. 4).
- Nach d'Alessandro et al. 1998 (S. 3–4): 1,5 Halbtöne in 50 ms gelten nicht als Portamento, in 200 ms schon.
- Form: monoton oder nicht monoton. Langsame Rutscher können „commence gently and then increase suddenly“ (S. 18).
- Alle Angaben dieses Punkts [wörtlich, selbst gelesen]; die Messungen stammen aus dem Gesang.
- Streicher, Sole 2026 (Q22): Gradienten bei historischen Cello-Aufnahmen von „approximately 600~Hz/s“ bis „over 4,000~Hz/s“ [wörtlich, Werkzeug].
- Ort: Pitch-Spur oder Übergang in einer Kette.

---

## 4. Dämpfer (con sordino)

- Wie lange das Aufsetzen dauert: **nicht gefunden.**
- Wirkung: Masse auf dem Steg, dadurch Tiefpass, laut Acoustics Today 2016 [nur Zusammenfassung].
- Evangelista & Freire 2025 (Q24) vergleichen Spektrum (LTAS) und Lautheit mehrerer Dämpfer. Das Abstract enthält keine Zahlen.
- Ort: Passage, mit einer Umschaltstelle, die eine Pause braucht (Einschätzung).

## 5. Ensemble

- Papiotis et al. 2012 (UPF/MTG) untersuchen die Synchronität von Intonation, Timing, Dynamik und Artikulation in Violinduos [nur Zusammenfassung, keine Zahlen gesehen].
- Tremolo im Satz in zufälliger Phase (Wolfe) [nur Zusammenfassung].
- Zahlen zu Vibrato-Synchronität, Streuung der Intonation oder Einsatzstreuung im Satz: nicht gefunden.

---

## Nicht gefunden

Kein Treffer ist kein Beweis. Die Volltexte von Schoonderwaldt 2009, Demoucron 2008, Askenfelt 1986/1989 und Meyer 1992 waren nicht erreichbar und enthalten vermutlich einen Teil davon.
- Einsatzverzögerung des Vibratos in ms bei Streichern.
- Zahlenwerte für die Breite bei Geringer & Allen 2004 und bei MacLeod 2008 (absolut).
- Rate und Tiefe beim Kontrabass aus einer Primärquelle.
- Messwerte für (F, v, β) je Stufe pp/mf/f (Askenfelt 1989, Schoonderwaldt 2009).
- Tremolo-Rate in Hz.
- β-Werte für sul ponticello und sul tasto.
- Flageolett.
- Col legno in Zahlen.
- Zupfstelle und T60 beim Pizzicato.
- Bogenlänge von Cello und Kontrabass.
- Dauer, um den Dämpfer aufzusetzen.
- Ensemble-Zahlen.

## Eigene Einschätzung (kein Beleg)

**Ist 5,5 Hz als Vorgabe für gestrichenes Vibrato belegt?**
- Teilweise. 5,5 Hz ist als gemessener Mittelwert belegt, nämlich für Schüler und Studenten an Violine und Cello (Q1, „approximately 5.5 Hz“).
- Profis und Aufnahmen liegen höher:
  - 5,9 Hz (Q5)
  - 6,63 Hz bei Solisten (Q3)
  - 6,48 Hz bei kommerziellen Cello-Aufnahmen gegen 5,26 Hz bei TinySOL (Q6)
- Die Aussage „5 bis 7 Hz“ trägt damit über mehrere Primärquellen. Die Zahl 5,5 Hz liegt am unteren Rand.
- Die Gleichsetzung mit dem Gesang ist für die Rate vertretbar. Für die Tiefe ist sie es ausdrücklich nicht: Gesang ±71 Cent, Streicher etwa 20–35 Cent (Meyer, aus zweiter Hand) beziehungsweise 63 Cent Breite.
- Naheliegend wäre eine Vorgabe um 6 Hz für einen Profiklang. Das ist eine Frage an Patrik und kein Beleg.

**Expression 0…1** (Vorschlag aus dem belegten Wirkprinzip):
- Den Pegel über v/β führen, die Klangfarbe über F/Fmax(v, β) innerhalb des Schelleng-Fensters.
- Eine mögliche Kopplung (Einschätzung, Spannweiten aus Q12 und Q15):
  - v von etwa 5 auf etwa 40 cm/s
  - β von etwa 1/6 auf etwa 1/30
  - F so, dass sie anteilig zwischen Fmin und Fmax mitläuft, grob 0,5 bis 1,5 N und darüber
- Die Kraft wirkt laut Askenfelt nicht direkt auf den Pegel.

**Notendauer:** Bei 650 mm Haar und 10–20 cm/s ergeben sich etwa 3–6,5 s je Strich. Bei den gemessenen 16,6 cm/s am Cello sind es um 4 s, wenn man den Cellobogen ähnlich lang annimmt (nicht belegt). Längere Noten brauchen einen Bogenwechsel.

**Orte nach Häufigkeit des Wechsels**
- Ausstattung: Rate und Grundtiefe des Vibratos.
- Passage: Dämpfer, ponticello/tasto, Sautillé und Tremolo als Gruppe.
- Note: Pizzicato, Spiccato, Martelé, Col legno, Tremolo.
- Kette: Legato und Portamento.

**Spiccato und Sautillé im Modell:** Sie lassen sich gut als Kraftimpuls (Glocke, Dauer etwa 30 ms bis zum Abheben) mit anschließendem freiem Abklingen rechnen. Die Sprungrate 6–35 Hz begrenzt das Tempo.

**AM im Vibrato:** Sie sollte aus der Rechnung kommen, also aus FM und Korpusantwort, und kein eigener Parameter sein. Die Messungen (Q4, Q5) zeigen, dass sie für den Klang wichtiger ist als die FM selbst.

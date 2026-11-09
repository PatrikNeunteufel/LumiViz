# Recherche IC1: Bogen-Saite — Physik und Rechenverfahren der gestrichenen Saite

> **Auftrag:** Klasse 3: Reibmodelle Bogen–Saite, Spielbereich (Schelleng, Guettler), Rechenverfahren und Kosten,
> Saitendaten, Steg und Kopplung, Einschwingen — mit Zahlenwerten; Rahmen die Waveguide-Saite von LumiViz (S-K1 a).
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [S, S. x] = vom Agenten im PDF gelesen; [W] = vom Abrufwerkzeug als Zitat/Feldinhalt; [Z] = nur
> Zusammenfassung.
> **Reichweite:** im Bericht selbst (18 Suchen, 36 Abrufe; Desvages & Bilbao 2016, van Walstijn 2026, Demoucron-
> Volltext, Dissertationen Desvages und Percival nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs und Textauszüge liegen im Cache der Claude-Sitzung (`tool-results/`),
> nicht im Repo. Der Agent empfiehlt, zwei Textauszüge geschützter Werke zu löschen — das entscheidet Patrik
> (siehe `STAND.md`, Cache der Sitzung).

---

# Bericht: Klasse 3 „Bogen-Saite“: Physik und Rechenverfahren der gestrichenen Saite

**Auftrag:** Reibmodelle, Spielbereich, Rechenverfahren und Kosten, Saitendaten, Steg und Kopplung, Einschwingen. Dazu Zahlenwerte für Bogen und Saite.
**Datum:** 2026-10-26

**Reichweite:**
- **Umfang:** 18 Websuchen, 36 Abrufe mit WebFetch (davon 11 ohne Ergebnis), dazu lokales Auslesen der vom Werkzeug zwischengespeicherten PDFs mit pdftotext und `quelle_suchen.py`, nur auf die Standardausgabe.
- **Orte:** Cambridge-Repositorium (Woodhouse), knutsacoustics.com (Guettler), CCRMA (Smith, Serafin), DAFx-Archiv, NTNU, Edinburgh Research Explorer und ERA, HAL-API, Crossref-API, Frontiers.
- **Sprachen:** Englisch; eine deutsche Suche zu den Mensuren.
- **Nicht erreichbar:**
  - MDPI Applied Sciences 6/135 (Desvages & Bilbao 2016): 403.
  - Acta Acustica 2026 (van Walstijn u. a.): PDF, EPUB und Abstract je 403.
  - HAL-Volltext der Dissertation Demoucron: Anubis-Sperre.
  - Dissertation Desvages 2018 (10,6 MB) und Dissertation Percival 2013: über der Größengrenze des Werkzeugs.
  - IoA-PDF Smith & Woodhouse „Modelling the friction of rosin“: 404.
  - AAU-PDF Willemsen u. a. 2019: 403.
- **Nicht gefunden:** Dissertation Serafin 2004, Woodhouse 2003, Smith & Woodhouse 2000 im Volltext.

**Vom Werkzeug zwischengespeichert** (Ordner `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):
- `webfetch-1793036734420-52e8fb.pdf`: Desvages & Bilbao, DAFx-15
- `webfetch-1793036751361-fekl15.pdf`: Woodhouse 2014, Review, akzeptiertes Manuskript
- `webfetch-1793036834996-m8tp9u.pdf`: Galluzzo, Woodhouse, Mansour 2017, akzeptierte Fassung
- `webfetch-1793036970961-8kzvli.pdf`: Schoonderwaldt, Guettler, Askenfelt 2008
- `webfetch-1793036971643-im6auk.pdf`: Chudy, Dixon, Perez, ICA 2013
- `webfetch-1793037115937-wvqb0a.pdf`: Serafin, Smith, Woodhouse 1999
- `webfetch-1793037272331-0cqiz8.pdf`: Russo, Ducceschi, Bilbao 2022
- `webfetch-1793037410846-fdcz6i.pdf`: Guettler & Askenfelt 1997
- `webfetch-1793036900486-b5c01r.pdf`: Guettler, Dissertation 2002
- `webfetch-1793037346634-eg6nue.pdf`: Guettler, „String stiffness“ (Notiz)
- `webfetch-1793037247181-13zoj2.pdf`: Willemsen & Serafin, DAFx-22. Falscher Treffer, nicht ausgewertet.
- Zwei Bash-Ausgaben hat das Werkzeug als **Textauszüge** abgelegt: `b1a3qyn45.txt` (DAFx-15, 58 KB) und `b350ju9hm.txt` (Galluzzo 2017, S. 2–12). Das sind Volltext-Stücke geschützter Werke; sie sollten gelöscht werden.

**Kennzeichnung:**
- [S, S. x] = wörtlich, selbst gelesen im PDF.
- [W] = wörtlich, vom Werkzeug geliefert.
- [Z] = nur Zusammenfassung, kein Beleg.

Seitenangaben sind gedruckte Seiten, wo vorhanden. Beim Review (Manuskript) ist die gedruckte Seite gleich der PDF-Seite.

---

## Quellen

**Q1:** Woodhouse, J. (2014), „The acoustics of the violin: a review", Rep. Prog. Phys. 77, 115901
- DOI 10.1088/0034-4885/77/11/115901; akzeptiertes Manuskript im Cambridge-Repositorium, handle 1810/245817 (Fußzeile „RepProgPhys violin … 13/8/14“). Frei lesbar.
- Recht: „this item's license is described as http://www.rioxx.net/licenses/all-rights-reserved" [W, Metadatenseite, abgerufen 2026-10-26].

**Q2:** Galluzzo, P. M., Woodhouse, J., Mansour, H. (2017), „Assessing friction laws for simulating bowed-string motion", Acta Acustica 103
- DOI 10.3813/AAA.919136; Repositorium DOI 10.17863/CAM.20507, akzeptierte Fassung vom 4. 8. 2017. Frei lesbar.
- Recht: Repositorium „all-rights-reserved“ (rioxx) [W]; Abstract mit „© 2017 S. Hirzel Verlag." [W, 2026-10-26].

**Q3:** Desvages, C., Bilbao, S. (2015), „Two-polarisation finite difference model of bowed strings with nonlinear contact and friction forces", Proc. DAFx-15, Trondheim
- PDF auf ntnu.edu. Frei.
- Auf S. 1 kein Rechtsvermerk gefunden [S]. Kein Treffer ist kein Beweis.
- Die erweiterte Fassung (Applied Sciences 6(5):135, 2016) war nicht erreichbar.

**Q4:** Schoonderwaldt, E., Guettler, K., Askenfelt, A. (2008), „An Empirical Investigation of Bow-Force Limits in the Schelleng Diagram", Acta Acustica united with Acustica 94, 604–622
- DOI 10.3813/AAA.918070. Kopie auf knutsacoustics.com, der privaten Seite Guettlers.
- Vermerk: „© S. Hirzel Verlag · EAA" [S, S. 604]. Keine freie Lizenz.

**Q5:** Serafin, S., Smith, J. O., Woodhouse, J. (1999), „An investigation of the impact of torsion waves and friction characteristics on the playability of virtual bowed strings", Proc. IEEE WASPAA, New Paltz
- ccrma.stanford.edu/~serafin/SerafinEtAl.pdf.
- Kein ©-Vermerk im Text gefunden [S].

**Q6:** Russo, R., Ducceschi, M., Bilbao, S. (2022), „Efficient simulation of the bowed string in modal form", Proc. DAFx20in22, Wien, S. 122–129
- dafx.de/paper-archive/2022/papers/DAFx20in22_paper_14.pdf.
- Lizenz: „distributed under the terms of the Creative Commons Attribution 4.0 International License" [S, S. 122].

**Q7:** Guettler, K., Askenfelt, A. (1997), „Acceptance limits for the duration of pre-Helmholtz transients in bowed string attacks", J. Acoust. Soc. Am. 101(5), 2903–2913
- Kopie auf knutsacoustics.com.
- Vermerk: „© 1997 Acoustical Society of America" [S, S. 2903].

**Q8:** Guettler, K. (2002), „The Bowed String – On the Development of Helmholtz Motion and On the Creation of Anomalous Low Frequencies", Doktorarbeit, KTH, TRITA-TMH 02:3
- knutsacoustics.com.
- Rechtsvermerk nicht geprüft.

**Q9:** Matusiak, E., Chatziioannou, V., Van Walstijn, M. (2025), „Numerical modelling of elasto-plastic friction in bow–string interaction with guaranteed passivity", Front. Signal Process. 5
- DOI 10.3389/frsip.2025.1525044.
- Lizenz: „distributed under the terms of the Creative Commons Attribution License (CC BY)" [W, 2026-10-26].

**Q10:** van Walstijn, M., Chatziioannou, V., Lampis, A., Matusiak, E. (2026), „A thermal elasto-plastic friction model for bowed-string simulation", Acta Acustica 10, 47
- DOI 10.1051/aacus/2026042.
- Lizenz-URL laut Crossref: creativecommons.org/licenses/by/4.0 [W]. Volltext gesperrt (403).

**Q11:** Demoucron, M. (2008), „On the control of virtual violins – Physical modelling and control of bowed string instruments", Diss., HAL tel-00349920
- Lizenz „https://about.hal.science/hal-authorisation-v1/" [W, HAL-API]. Nur das Abstract gelesen.

**Q12:** Smith, J. O., Physical Audio Signal Processing, Seiten „Bow-String Scattering Junction" und „Bowed String Synthesis Extensions", CCRMA
- „Copyright © 2026-08-21 by Julius O. Smith III" [W].

**Q13:** Chudy, M., Dixon, S., Perez (2013), Beitrag ICA 2013 zu Bogenparametern beim Cello, webspace.eecs.qmul.ac.uk
- Titel nur aus der Suche [Z]. Rechtsvermerk nicht geprüft.

**Q14:** Serafin, S., „Impact of String Stiffness on Virtual Bowed Strings", CCRMA-Seite
- „Copyright © 2019-02-05 by Stefania Serafin" [W].

**Q15:** Guettler, K., „String stiffness" (Notiz, 2012), knutsacoustics.com. Ohne Vermerk.

---

## 1. Reibmodelle Bogen–Saite

**Grundgerüst (Friedlander, McIntyre/Schumacher/Woodhouse):**
- Die Saite am Bogenpunkt ist ein Wellenleiter mit „v = vh + f / 2Z0" [S, Q1 S. 7, Gl. 2]. Darin ist vh der Anteil aus der Vorgeschichte, die zurücklaufenden Wellen.
- Der neue Punkt (f, v) liegt im Schnitt dieser Geraden mit der Reibkurve, die Konstruktion von Friedlander [S, Q1 S. 8].
- Bei mehreren Schnittpunkten („Friedlander's ambiguity") entscheidet eine Hystereseregel [S, Q2 S. 5].
- Folge der Hysterese: Die Note wird mit steigender Bogenkraft tiefer, „the note may play progressively flat" [S, Q1 S. 10].

**Klassische Reibkurve (Exponentialform, nach Smith & Woodhouse 2000):**
- „μ = 0.4e^((v−vb)/0.01) + 0.45e^((v−vb)/0.1) + 0.35" [S, Q2 S. 5, Gl. 1; gleichlautend Q5 S. 2 und Q3 S. 3].
- Daraus folgen μs = 1,2 bei Relativgeschwindigkeit 0 und μd → 0,35. Das ist eigene Rechnung.
- Galluzzo hat daraus eine „rekonstruierte“ Kurve gewonnen: „μ = 0.4e^((v−vb)/0.7) + 0.35" [S, Q2 S. 5, Gl. 2], also μs = 0,75.
- Laut Q2 deutet der große Unterschied der beiden Kurven auf ernste Mängel des Kurvenmodells hin: „the friction curve model must have serious shortcomings" [S, Q2 S. 5].

**Hyperbolische Reibkurve:**
- „μd = 0.3 and μs = 0.8“ mit μ = μd + (μs−μd)·v0/(v0 + v − vb) [S, Q5 S. 2].
- Einen Zahlenwert für v0 nennt die Stelle nicht [S]. Q5 nennt v0 „initial bow velocity“.
- Q4 schreibt die Kurve als μ = μd + K/(z − z0) mit z = vB/β [S, S. 605, Gl. 3]. Aus der Anpassung folgt „μs = 0.8 and (μs − μd) = 0.4" [S, Q4 S. 612]. Q4 nennt dort auch Δμ ≈ 0,4 [S, S. 611].

**Weiche Kurve (Bilbao):**
- ϕ(η) = √(2a)·η·e^(−aη²+1/2), mit a als freiem Parameter [S, Q6 S. 123, Gl. 2].
- Im Beispiel gilt „a = 100", bei c = 150 m/s, L = 0,7 m, vB = 0,2 m/s [S, Q6 S. 126, Abb. 3].

**Thermisches oder plastisches Modell (Smith & Woodhouse 2000; Woodhouse 2003):**
- Die Kontakttemperatur ist eine einzige Zustandsvariable. Sie wird aus einer instationären Wärmeflussrechnung im Kontaktvolumen gewonnen [S, Q2 S. 6].
- Reibkraft: „μ = A·ky(T)·sgn(v)/N“ mit ky(T) als Schubfließspannung über der Temperatur [S, Q5 S. 2].
- Zwei Varianten wurden untersucht, „viscous“ und „plastic“; die plastische passte besser [S, Q2 S. 6].
- Die Parameter sind nicht gemessen, sondern an die Gleichgewichtsgleiten-Kurve (Gl. 1) angepasst [S, Q2 S. 6].
- Zahlen aus Tabelle 2 [S, Q2 S. 8]:
  - Kontaktradius 250 / 200 / 500 µm
  - Referenzkraft 1 / 1 / 3 N
  - Schichtdicke 1 / 1 / 10 µm
  - Spalten: Galluzzo-Prüfstand / Schumacher-Prüfstand / Gleichgewichtsgleiten
- Thermische Stoffwerte stehen „exactly as listed in Table 1 of [18]", also in Woodhouse 2003, nicht gelesen [S, Q2 S. 8].
- Ergebnis: Das thermische Modell ist „benign", die Helmholtzbewegung stellt sich schneller und sicherer ein [S, Q2 S. 6]. Im Guettler-Diagramm liegt es „a lot closer than the friction-curve model" [S, Q1 S. 48]. Es ist aber „a bit too benign and smooth" [S, Q1 S. 48].
- Gesamturteil Q2: Das Temperaturmodell schnitt „significantly better" ab [W, Abstract].

**Kolophonium:**
- Glasübergang „a little above normal room temperature" [S, Q1 S. 17].
- Bass-Kolophonium ist weicher und fließt bei Wärme [S, Q1 S. 17].
- Es gibt Hinweise auf Schmelzschmierung durch Reibungswärme [S, Q1 S. 19].

**Elasto-plastische Modelle (Typ LuGre/Dupont):**
- Serafin, Avanzini und Rocchesso 2003 [Z].
- Q9: μs = 1,0207, μd = 0,5071, Stribeck-Geschwindigkeit 0,228 m/s [W, Tab. 1]. Diese Zahlen sind nur über das Werkzeug geliefert, Gleichungen fehlten im Abruf.
- Q10 verbindet elasto-plastisches Vorgleiten mit thermischen Effekten. Geprüft wurde es an einem modal entwickelten Saitenmodell mit Torsion und breitem Bogen [W, Abstract].

**Bogenbreite:**
- Das Haarband ist „several millimetres wide" [S, Q1 S. 10]. In Q4 sind es „10 mm" [S, Q4 S. 608]; in Q9 0,01 m [W].
- Folge ist teilweises Gleiten an den Rändern („differential slipping"): ein Rauschanteil, der mit der Stärke wächst [S, Q1 S. 10]. Spieler steuern ihn über Kraft, Ort und Kippen des Bogens [S, Q1 S. 11].

**Haarnachgiebigkeit:** Die Längselastizität der Haare beeinflusst die Spannungsspitzen an den Bandrändern [S, Q1 S. 14–15]. Q9 nennt eine Haarsteifigkeit von 48 297 N/m und eine Dämpfung von 57,674 kg/s, konzentriert gerechnet [W].

**Torsion:**
- Die Saitenoberfläche wird tangential erregt; Torsionsmoden sind deutlich stärker gedämpft [S, Q1 S. 13].
- Wirkt im Maximum der Bogenkraft: Statt Z0 tritt Ztot = Z0·ZR/(Z0+ZR) ein. ZR ist „typically a factor 2 to 4 higher than Z0" [S, Q4 S. 605].
- Q5: Ohne Torsion bleibt die Spielbarkeit erhalten [S, Q5 S. 1]. Torsion ist entbehrlich „even in the highest quality instances" [S, Q5 S. 3].
- Q2 widerspricht teilweise. Eigener Befund: kaum Empfindlichkeit gegen Torsion. In einer anderen Umsetzung („[7]“, Mansour u. a.) war der Effekt aber „far from negligible" [S, Q2 S. 13].

## 2. Spielbereich

**Schelleng-Grenzen:**
- Fmax = 2·Z0·vB / (β·(μs−μd)) und Fmin = Z0²·vB / (2R·β²·(μs−μd)²) [S, Q4 S. 604, Gl. 1–2].
- Das β ist im Textauszug verloren gegangen. Die Exponenten passen zu „minimum force varies proportional to β^−2 while the maximum force is proportional to β^−1" [S, Q1 S. 5].
- Die Formen mit hyperbolischer Kurve (Gl. 4–5) enthalten (vB + z0·β), sodass beide Grenzen für vB → 0 endlich bleiben [S, Q4 S. 605].
- Messbefund Q4: „The minimum bow force was found to be independent of bow velocity" [S, Q4 S. 604]. Das widerspricht Schelleng.
- Messbereich Q4 [S, Q4 S. 608]:
  - β ≈ 1/30 bis 1/6, Bogen-Steg-Abstand 11–55 mm
  - Bogenkraft 49–3000 mN
  - Bogengeschwindigkeit vB = 5, 10, 15, 20 cm/s
- „bow velocities around 20 cm/s are typical" [S, Q4 S. 615, nach Askenfelt].
- Leises Spiel nahe der unteren Grenze bei großem β ist üblich [S, Q4 S. 620].
- Cello-D-Saite in Q2: Schelleng-Diagramm bei vB = 0,05 m/s und β ≈ 0,02–0,18 [S, Q2 S. 7].

**Guettler-Diagramm:**
- Achsen sind Bogenkraft (konstant gehalten) und Beschleunigung aus der Ruhe [S, Q1 S. 47].
- „Perfekte“ Einschwingvorgänge liegen nur in einem Keil, „larger when bowing far from the bridge" [S, Q1 S. 47].
- Messung an Cello D3 (147 Hz) bei β = 0,08. Bereich 0,4–3,2 N und 0,08–3,2 m/s², β zwischen 1/28 und 1/6 [S, Q2 S. 8].
- Die Messung ist gesprenkelt und nicht wiederholbar, typisch für ein empfindliches nichtlineares System [S, Q1 S. 47–48].

**Pegel und Klangfarbe:**
- Je geringer die Kraft, desto runder die Helmholtz-Ecke und desto weniger hohe Teiltöne: „the lower the bow force, the more rounded the waveform becomes" [S, Q1 S. 5].
- Die größte Bandbreite der Klangfarbe liegt an der Spitze des Schelleng-Dreiecks, nahe am Steg [S, Q1 S. 45]. Messungen dazu bei Schoonderwaldt u. a. [S, Q1 S. 45].
- Nach Askenfelt steuern Bogengeschwindigkeit und Bogen-Steg-Abstand den Pegel. Die Kraft „mainly regulates the harmonic content" [S, Q13 S. 1, Sekundärquelle].
- Demoucron untersucht die Wirkung der drei Parameter auf „sound level and spectral centroid" [W, Q11 Abstract].
- Typische Zahlen nach Askenfelt 1986, aus dem Suchwerkzeug: piano 29 cm/s, 42 mm, 0,5 N; forte 21 cm/s, 18 mm, 2,3 N [Z]. **Im PDF Q13 nicht wiedergefunden, ungeprüft.**
- Cello (Q13): Mittel 16,62 cm/s im Legato, bis 40,48 cm/s im Détaché, Höchstwert 89,88 cm/s [S, Q13 S. 3–4]. Bogen-Steg-Abstand 4,5–9,2 cm im Mittel je Spieler [S, Q13 S. 4, Tab. 1].

## 3. Rechenverfahren und Kosten

**Waveguide oder Impulsantwort mit Reibstelle (McIntyre/Woodhouse, Smith):**
- Smith: Reflexion über die Bogentabelle ρ(Δv) = r/(1+r), mit r = 0,25·Rb/Rs [W, Q12]. Umsetzbar als „memoryless table lookup or segmented polynomial" [W]. Hysterese vernachlässigt [W].
- Erweiterungen: thermisches Modell, Bogenbreite, Haardynamik [W, Q12].
- Woodhouse-Typ mit Torsion: „The sampling rate in all cases was 200 kHz" [S, Q5 S. 2], damals auf einem Pentium II 333 MHz.
- Q5 S. 2 zum Determinismus: „different computers gave slightly different results" (chaotische Bereiche) [S].

**Modal mit Reibstelle:**
- Demoucron: „a bowed-string model based on modal solutions of the string equation" [W, Q11]. Kosten nicht gelesen.
- Q6, nicht-iteratives Schema, modal, steife und gedämpfte Cello-D3-Saite in Matlab. Verhältnis Laufzeit/Echtzeit bei fs = OS·44 100 [S, Q6 S. 129, Tab. 2]:
  - OS = 1: 0,19
  - OS = 2: 0,39
  - OS = 5: 0,91
- Für die ideale Saite in Q6 S. 128, Tab. 1 [S]:
  - modal: 0,69–0,88 bei OS 1 und 2,5 bei OS 2
  - iterative Finite-Differenzen-Schemata: 3,9–167
- Modal lässt sich je Mode eine eigene Abklingkonstante setzen, „without any additional computational effort" [S, Q6 S. 128].
- Laut Q6 senkt die Steifheit die Zahl der hörbaren Moden und damit die Rechenzeit [S, Q6 S. 128].
- Q9 und Q10 verwenden modale Entwicklung mit elasto-plastischer bzw. thermischer Reibung [W]. Q9 nennt „heavy oversampling" als Hauptgrenze [W, Abschn. 7]; die Referenzrechnungen laufen mit 1024-facher Überabtastung [Z].

**Finite Differenzen (Desvages & Bilbao):**
- Zwei Polarisationen, energieerhaltend, mit Kontakt für Bogen, Finger und Griffbrett [S, Q3 S. 1–2].
- Gerechnet bei „Fs = 44.1 kHz" [S, Q3 S. 7].
- Steuerung über Stützpunktfunktionen für Bogenort, Normalkraft und Tangentialkraft [S, Q3 S. 7]. Der Bogen wird also kraftgetrieben, nicht geschwindigkeitsgetrieben.
- Kosten: „does not run in real-time" [S, Q3 S. 8].
- Beispiel Cello D: xB = 0,851 L, Normalkraft 2 N, Tangentialkraft 4,4 N für Helmholtzbewegung [S, Q3 S. 7, Abb. 3].
- Willemsen u. a. 2019, elasto-plastisch mit Finite Differenzen: unter 6 % CPU je Saite [Z, PDF nicht erreichbar].

**Überabtastung:** Gesichert sind 200 kHz (Q5) sowie Faktor 1–5 auf 44,1 kHz im modalen Schema (Q6). Eine allgemeine Mindestrate nennt keine der gelesenen Quellen.

## 4. Saitendaten

**Cello D3, Galluzzo-Prüfstand [S, Q2 S. 5, Tab. 1]:**
- 146,8 Hz, Spannung 111 N, 2,7 g/m
- Biegesteifigkeit 3,0·10⁻⁴ (gedruckt „N/m2“, gemeint wohl N·m²)
- Z0 = 0,55 kg/s
- Torsion: 758 m/s, Q = 45, Impedanz 1,8 kg/s
- Verlustbeiwerte nach [7] (Mansour): ηF = 23·10⁻⁵ (gegriffen 69·10⁻⁵), 12,5·10⁻², ηB = 0,11 s⁻¹. Die Zuordnung der Zeilen ist im Satz unklar.

**Violin-E, Schumacher-Prüfstand [S, Q2 S. 5]:** 693 Hz, 72,5 N, 0,42 g/m, Z0 = 0,175 kg/s, Torsion 4620 m/s, Q = 30.

**Cello D in Q5 [S, Q5 S. 2]:** B = 0,0004 N·m². Z0 = 0,55 und Torsionsimpedanz 1,8 kg/s. Torsionswelle 5,2-mal schneller als die Transversalwelle. Q = 500 (transversal) und 45 (Torsion). Gemessen an einer Nylonkern-Saite bei 147 Hz.

**Violine [S, Q4 S. 607]:** D aus Stahl, umsponnen, 0,70 mm, „ρL = 1.29 g/m, Z0 = 0.25 kg/s". E aus Stahl, 0,26 mm, 0,41 g/m, Z0 = 0,18 kg/s.

**Dämpfung Violine D [S, Q4 S. 618]:** τ1 = 0,521 s, Q1 = 480 am Monochord; 0,448 s, Q1 = 410 an der Violine. Raman-Widerstand R = 76 bzw. 65 kg/s.

**Pickering (Dominant) nach Q14:** Biegesteifigkeit B für E/A/D/G = 5,2·10⁻¹⁵, 6,5·10⁻¹⁴, 1,57·10⁻¹², 1,50·10⁻¹² [W]. Spannung 72,6 / 56,3 / 43,7 / 44,6 [W]. Die Tabelle ist im Abruf verstümmelt, Einheiten zum Teil unplausibel; nicht verwenden ohne das Original.

**Q15:** Die Inharmonizität wächst mit der vierten Potenz des Kerndurchmessers [S]. Gestrichen werden die Phasen bei jedem Haft-Gleit-Wechsel neu gekoppelt [S].

**Herstellerwerte (nur Händler):**
- Dominant 135B: E ≈ 7,8 / A 5,5 / D 4,1 / G 4,4 kg [Z, Händlerseite].
- Cello in lb, auf 690 mm bezogen (aitchisoncellos.com): Larsen A 39,3, D 29,7, G 27,6, C 30,1 lb [Z].

**Viola, Kontrabass, Mensuren:** Nicht belegt, siehe „Nicht gefunden“. Matusiak u. a. rechnen mit L = 0,7 m und 149,74 N [W, Q9 Tab. 2]. Russo u. a. verwenden L = 0,7 m [S, Q6].

## 5. Steg und Kopplung

**Stegadmittanz:**
- In der Simulation über Impulsantworten h1(t) oder h2(t). Möglich aus Messdaten, modaler Anpassung oder FEM [S, Q1 S. 14].
- Je Kerbe gilt voll dreidimensional eine 3×3-Matrix von Übertragungsfunktionen [S, Q1 S. 14].
- Die Mindestkraft hängt direkt vom Korpus ab, die Höchstkraft kaum [S, Q1 S. 45]. Mit gemessener Admittanz lässt sie sich je Note vorhersagen [S, Q1 S. 45].

**Wolfton:**
- Entsteht, wenn die Mindestkraft beim Anwachsen der Korpusresonanz die tatsächliche Kraft übersteigt. Es folgt ein Wechsel zwischen Helmholtz- und Doppelgleitbewegung [S, Q1 S. 11–12].
- Am stärksten beim Cello, auf tiefen Saiten und bei geringer Kraft [S, Q1 S. 11].
- Simuliert mit einer einzigen Korpusresonanz in h1(t) [S, Q1 S. 12].

**Zweite Polarisation:**
- Wird über Schwankungen der Normalkraft und über die Kopplung am Steg erregt [S, Q1 S. 12].
- In Q3 sind beide Polarisationen nur über die Kontaktkräfte gekoppelt. Die Steg-Kopplung fehlt dort [S, Q3 S. 2].
- Längswellen der Saite sind möglich, ihre Wirkung ist wenig erforscht [S, Q1 S. 13].

## 6. Einschwingen

**Hörbare Grenzen:** „50 ms (≤10 nominal periods, open G string, violin)" für verlängerte Perioden und „90 ms (≤18 periods)" für Mehrfachgleiten. Das gilt für einen neutralen Ansatz [S, Q7 S. 2903]. An anderer Stelle nennt Q7 30–50 ms [S, Q7 S. 2905].

**Häufigkeit:** Etwa die Hälfte der Ansätze im normalen Spiel ist nicht perfekt [S, Q7 S. 2903].

**Physik des Ansatzes:**
- Jeder echte Ansatz beginnt mit Kraft oder Geschwindigkeit gleich null [S, Q1 S. 47].
- Laut Q8 gelten für den perfekten Ansatz strenge Bedingungen an Geschwindigkeit und Beschleunigung. Sind sie erfüllt, öffnet sich der Schelleng-Bereich schnell [S, Q8 S. 17].
- Bei der Cello-Messung reicht der Bereich der Einschwingdauer bis über 20 Perioden („unacceptably long") [S, Q2 S. 8].

**Bogenwechsel:**
- Abbremsen, einige Perioden Haften, dann neuer Ansatz. Je nach Ort im Guettler-Diagramm ist er sofort sauber oder einige Perioden „scratchy“ [S, Q1 S. 49].
- Echtes Spiel ist nie stationär: Kraft, Geschwindigkeit und Ort werden ständig moduliert [S, Q1 S. 49].

---

## Nicht gefunden

- **Volltexte:** Woodhouse 2003 (thermische Stoffwerte), Smith & Woodhouse 2000, Dissertation Serafin 2004, Demoucron 2008 (nur Abstract; Kosten und Modenzahl offen), Desvages & Bilbao 2016 und Dissertation 2018 (Rechenzeiten).
- **Messdaten:** Askenfelt 1986/1989 im Original; Pickering im Original.
- **Saiten und Mensuren:** Herstellerdaten von Thomastik und Pirastro in N mit linearer Dichte; Werte für Viola und Kontrabass. Mensuren für Viola, Cello und Kontrabass sind in keiner gelesenen Quelle belegt. Gängig sind etwa 325 mm (Violine), 690 mm (Cello, Bezugslänge bei Pirastro [Z]) und rund 1050 mm (Kontrabass); nicht belegt.
- **Kosten und Abtastung:** Eine allgemeine Regel zur Mindestüberabtastung der Reibstelle. Rechenzeiten in C++.

Kein Treffer ist kein Beweis: Die Lücken stammen aus gesperrten oder zu großen Volltexten, nicht aus belegter Abwesenheit.

## Eigene Einschätzung (kein Beleg)

**Trägt für vorab gerechnete Töne mit Steuerverlauf: modale Saite mit punktförmiger Reibstelle, Bogen geschwindigkeitsgetrieben.** Das passt zum modal gerechneten Flügel, nutzt Moden-Dämpfung ohne Mehrkosten und ist gemessen schneller als Echtzeit (Q6: 0,2–0,9 in Matlab bei 1- bis 5-facher Überabtastung; in C++ vermutlich deutlich darunter).

Die Waveguide-Saite in LumiViz trägt als Alternative den Ansatz von McIntyre und Woodhouse direkt: Gl. 2 plus Reibkurve. Laut Q5 verlangt der Woodhouse-Typ hohe Raten (200 kHz), das treibt die Kosten. Vorab gerechnet ist Echtzeit ohnehin nicht nötig. Selbst Faktor 4–5 bleibt wohl unter der Rechenzeitgrenze.

**Reibmodell gestuft:**
- Stufe 1: klassische Kurve (Gl. 1), ohne freie Parameter.
- Stufe 2: plastisch-thermisch mit einer Temperatur als Zustand. Sauberer laut Q1, Q2 und Q5; kostet nur ein Filter je Takt. Die Stoffwerte aus Woodhouse 2003 fehlen aber noch.

**Abbildung der Steuerspuren:** Expression 0…1 sollte nicht direkt auf die Kraft gehen, sondern auf einen Pfad im Raum (vB, β, F) innerhalb der Schelleng-Grenzen.
- Pegel über vB und β.
- Helligkeit über F/Fmin.
- Ansätze nach Guettler gestalten: Kraft konstant, Beschleunigung im Keil.

**Risiken:**
1. **Determinismus:** Q5 meldet rechnerabhängige Ergebnisse in chaotischen Bereichen. Für ein Vorab-Rechnen mit Ablage braucht es bitgenaue Fließkomma-Regeln, sonst sind Töne nicht reproduzierbar.
2. **Spielbarkeit:** Die Reibkurve neigt zu Kratzansätzen; das thermische Modell ist gutmütiger, aber „zu glatt“.
3. **Wolfton:** Er entsteht nur, wenn der Korpus *in der Rückkopplung* am Steg sitzt, nicht als Ausgangsfilter. Die Korpusresonanzen hinter dem Steg in LumiViz müssen deshalb in h1 eingehen.
4. **Torsion und Bogenbreite:** Die Befunde widersprechen sich (Q5 und Q2). Für den ersten Wurf ist das vermutlich verzichtbar; das Rauschen bei Bogenbreite fehlt dann.
5. **Saitendaten:** Für Viola und Kontrabass fehlen belastbare Werte. Dafür ist eine eigene Recherche oder Messung nötig.

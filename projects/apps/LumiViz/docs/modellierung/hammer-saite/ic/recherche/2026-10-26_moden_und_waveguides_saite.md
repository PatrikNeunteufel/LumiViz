# Recherche IC1: Z1 — Moden und Waveguides kombiniert, Saite (Klassen 1, 2, 3)

> **Auftrag:** Zusatzfrage Z1 aus LumiViz (`AUFGABE.md`): welche Verfahren Moden und digitale Wellenleiter bei Hammer-,
> Zupf- und Bogensaite verbinden, was je Weg geht, wie sie an Aufnahmen gesetzt werden; Abgleich mit der Antwort von
> Claude in LumiViz (S154).
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug] = vom
> Abrufwerkzeug zitiert; [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (12 Suchen, rund 54 Abrufe; HAL-Volltexte Bensa 2003 und Bank & Chabassier 2019,
> IEEE Xplore, Springer, Aaltodoc nicht erreicht; Lee/Smith/Välimäki 2010 nicht gelesen).
> **Anmerkung der Sitzung:** Die Befunde je Klasse stehen in `klassen/hammer-saite/recherche/2026-10-26_moden_und_waveguides.md`
> und den gleichnamigen Dateien der Klassen 2 und 3. **Von der Sitzung selbst nachgelesen:** siehe dort.

---

# Z1: Moden und Waveguides bei der Saite (Klassen 1, 2, 3)

**Auftrag:** Literaturprüfung, wie sich Moden und digitale Wellenleiter (digital waveguides, DWG) für Hammer-, Zupf- und Bogensaite verbinden lassen, mit Abgleich der Claude-Antwort in LumiViz.
**Datum:** 2026-10-26.

**Reichweite:**
- **Umfang:** 12 Websuchen und rund 54 WebFetch-Abrufe. Das liegt über den geplanten 30–45, weil viele Server umgeleitet oder gesperrt haben. Dazu kommt lokales `pdftotext`, nur auf die Zwischenspeicher des Werkzeugs, ohne Ausgabe in Dateien.
- **Orte:** BME-Seite von Bank (Volltexte), CCRMA/PASP (Smith), DAFx-Archiv, Crossref-API, Semantic-Scholar-API, HAL-API, Cambridge-Repository und NESS Edinburgh (Clavinet-Preprint).
- **Sprachen:** Englisch (Quellen), Deutsch (Suche nicht nötig).
- **Nicht erreichbar:**
  - HAL-Volltexte (Anubis-Sperre): Bensa et al. 2003, Bank & Chabassier 2019.
  - IEEE Xplore: leere Seite. Abstracts von Lee/Smith/Välimäki 2010 und Maestre/Scavone/Smith 2017 sind in Crossref und Semantic Scholar nicht hinterlegt.
  - Springer-Link (Cookie-Umleitung): Cembalo-Artikel Välimäki et al. 2004.
  - Aaltodoc: 403.
  - Kopie Välimäki et al. 2006 (Rep. Prog. Phys.) an der Uni Victoria: 404.
  - HAL-API: Aramaki et al. 2001 bei zwei Abfragen ohne Treffer.
  - Ohne Abstract: Karjalainen/Välimäki/Tolonen 1998 (CMJ, nur Crossref-Metadaten) und Välimäki et al. 1996 (JAES, nur Suchmaschinen-Zusammenfassung).
- **Vom Abrufwerkzeug zwischengespeicherte PDFs** (alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):
  - `webfetch-1793040358753-ed292n.pdf`: Bank/Zambon/Fontana 2010
  - `webfetch-1793040412293-psx7q0.pdf`: Bank, Dissertation 2006
  - `webfetch-1793040620339-chj9q4.pdf`: Lee/Smith/Abel/Berners, DAFx 2009
  - `webfetch-1793040662139-15o95n.pdf`: Bank et al., EURASIP JASP 2003
  - `webfetch-1793040753737-2w6yf3.pdf`: Trautmann/Bank/Välimäki/Rabenstein, AES 22 2002
  - `webfetch-1793040801498-hlq5o5.pdf`: Mansour/Woodhouse/Scavone 2016, Teil 1, angenommene Fassung
  - `webfetch-1793040827674-l4a5ea.pdf`: Bank & Välimäki 2003, IEEE SPL
  - `webfetch-1793040918416-fuuo8v.pdf`: Bank & Sujbert 2005, JASA
  - `webfetch-1793040978321-cq4zyn.pdf`: Russo/Ducceschi/Bilbao, DAFx 2022
  - `webfetch-1793041073473-puxxk9.pdf`: Rauhala & Välimäki, DAFx 2006
  - `webfetch-1793041121596-dylh0n.pdf`: Gabrielli et al. 2013 (Clavinet)
  - `webfetch-1793041150770-xcco0j.pdf`: Bank, DAFx 2009
  - `webfetch-1793041170471-vvf0gr.pdf`: Bank & Karjalainen, DAFx 2010

**Kennzeichnung der Angaben:**
- **[wörtlich, selbst gelesen, S. x]**: von mir mit `pdftotext` aus dem PDF gelesen. Bei Bank 2010 ist die Seite die des Preprints; die Zeitschriftenseite ist 808 plus Preprintseite. Bei der Dissertation Bank 2006 ist es die gedruckte Seite (PDF-Seite minus 14).
- **[wörtlich, Werkzeug]**: Wortlaut, den WebFetch zitiert hat.
- **[nur Zusammenfassung]**: kein Beleg.

---

## 1. Quellen

Abrufdatum aller Quellen: 2026-10-26.

| Nr. | Autoren, Jahr, Titel, Ort | Fundort | frei? | Rechtsvermerk im Wortlaut (Fundstelle) |
|---|---|---|---|---|
| Q1 | B. Bank, S. Zambon, F. Fontana, 2010, „A Modal-Based Real-Time Piano Synthesizer“, IEEE Trans. on Audio, Speech, and Language Processing, Vol. 18, No. 4, pp. 809–821 | home.mit.bme.hu/~bank/publist/taslp10.pdf | Autorenkopie | Im PDF kein Vermerk gefunden. Auf der Liste: „the copyright of IEEE journal papers belongs to IEEE.“ / „Only personal use is permitted.“ [wörtlich, Werkzeug; home.mit.bme.hu/~bank/publist/index.html] |
| Q2 | B. Bank, 2006, „Physics-based Sound Synthesis of String Instruments Including Geometric Nonlinearities“, Dissertation, BME Budapest | home.mit.bme.hu/~bank/phd/phd.pdf | frei | Auf der Seite kein Vermerk [Werkzeug, …/phd/index.html] |
| Q3 | B. Bank, F. Avanzini, G. Borin, G. De Poli, F. Fontana, D. Rocchesso, 2003, „Physically Informed Signal Processing Methods for Piano Sound Synthesis: A Research Overview“, EURASIP JASP 2003:10, 941–952 | …/publist/jasp03.pdf | frei | „c 2003 Hindawi Publishing Corporation“ [selbst gelesen, S. 941] |
| Q4 | B. Bank, V. Välimäki, 2003, „Robust Loss Filter Design for Digital Waveguide Synthesis of String Tones“, IEEE SPL 10(1), 18–20 | …/publist/spl03.pdf | Autorenkopie | „1070-9908/03$17.00 © 2003 IEEE“ [selbst gelesen, S. 18] |
| Q5 | B. Bank, L. Sujbert, 2005, „Generation of longitudinal vibrations in piano strings: From physics to sound synthesis“, JASA 117(4), 2268–2278 | …/publist/jasa05.pdf | Autorenkopie | „© 2005 Acoustical Society of America“ [selbst gelesen, S. 2268] |
| Q6 | N. Lee, J. O. Smith III, J. Abel, D. Berners, 2009, „Pitch glide analysis and synthesis from Recorded Tones“, Proc. DAFx-09, Como | dafx.de/paper-archive/2009/papers/paper_86.pdf | frei | Kein Vermerk gefunden (Suche nach „copyright“ und „Creative Commons“) |
| Q7 | N. Lee, J. O. Smith, V. Välimäki, 2010, „Analysis and Synthesis of Coupled Vibrating Strings Using a Hybrid Modal-Waveguide Synthesis Model“, IEEE TASLP 18(4), 833–842, DOI 10.1109/tasl.2009.2033979 | nur Crossref und Semantic Scholar; Tonbeispiele unter ccrma.stanford.edu/~nalee/ieee09_njv-results/ | nein („CLOSED“, Semantic Scholar) | Crossref-Lizenz-URL: ieeexplore.ieee.org/Xplorehelp/downloads/license-information/IEEE.html [Werkzeug]. **Inhalt nicht gelesen.** |
| Q8 | L. Trautmann, B. Bank, V. Välimäki, R. Rabenstein, 2002, „Combining Digital Waveguide and Functional Transformation Methods for Physical Modeling of Musical Instruments“, AES 22nd Conf., 307–316 | …/publist/aes22.pdf | Autorenkopie | Kein Vermerk gefunden |
| Q9 | H. Mansour, J. Woodhouse, G. P. Scavone, 2016, „Enhanced wave-based modelling of musical strings. Part 1: Plucked strings“, Acta Acustica united with Acustica 102, DOI 10.3813/AAA.919021 | repository.cam.ac.uk/handle/1810/261228 (angenommene Fassung) | Repositorium | „Except where otherwised noted, this item's license is described as“ rioxx … all-rights-reserved [Werkzeug, Repositoriumsseite] |
| Q10 | R. Russo, M. Ducceschi, S. Bilbao, 2022, „Efficient Simulation of the Bowed String in Modal Form“, Proc. DAFx20in22, Wien | dafx.de/paper-archive/2022/papers/DAFx20in22_paper_14.pdf | frei | „This is an open-access article distributed under the terms of the Creative Commons Attribution 4.0 International License“ [selbst gelesen, S. 1] |
| Q11 | J. Rauhala, V. Välimäki, 2006, „Dispersion Modeling in Waveguide Piano Synthesis Using Tunable Allpass Filters“, Proc. DAFx-06, Montreal | dafx.de/paper-archive/2006/papers/p_071.pdf | frei | Kein Vermerk gefunden |
| Q12 | L. Gabrielli, V. Välimäki, H. Penttinen, S. Squartini, S. Bilbao, 2013, „A digital waveguide-based approach for Clavinet modeling and synthesis“, EURASIP J. Adv. Signal Process. 2013:103 | ness.music.ed.ac.uk/wp-content/uploads/2013/06/Gabrielli.pdf (vorläufige Fassung) | frei | „This is an open access article distributed under the terms of the Creative Commons Attribution License (http://creativecommons.org/licenses/by/2.0)“ [selbst gelesen, S. 1] |
| Q13 | B. Bank, 2009, „Energy-based Synthesis of Tension Modulation in Strings“, Proc. DAFx-09 | …/publist/dafx09.pdf | Autorenkopie | Kein Vermerk gefunden |
| Q14 | B. Bank, M. Karjalainen, 2010, „Passive Admittance Matrix Modeling for Guitar Synthesis“, Proc. DAFx-10, Graz | …/publist/dafx10adm.pdf | Autorenkopie | Kein Vermerk gefunden |
| Q15 | J. O. Smith III, Physical Audio Signal Processing (PASP): Seiten „Coupled_Strings“, „Commuted_Piano_Synthesis“, „Coupled_Piano_Strings“, „Linear_Commuted_Bowed_Strings“ | ccrma.stanford.edu/~jos/pasp/ | frei lesbar | „Copyright © 2026-08-21 by Julius O. Smith III“ [Werkzeug] |
| Q16 | J. Bensa, S. Bilbao, R. Kronland-Martinet, J. O. Smith III, 2003, „The simulation of piano string vibration: From physical models to finite difference schemes and digital waveguides“, JASA | HAL-API, hal-00088329 | Volltext gesperrt | Lizenzfeld der HAL-API: creativecommons.org/licenses/by/4.0/ [Werkzeug]. Nur das Abstract, und nur als Zusammenfassung. |
| Q17 | B. Bank, J. Chabassier, 2019, „Model-based digital pianos: from physics to sound synthesis“, IEEE Signal Processing Magazine | HAL-API, hal-01894219 | Volltext gesperrt | Lizenzfeld: about.hal.science/hal-authorisation-v1/ [Werkzeug] |

---

## 2. Welche Verfahren Moden und Waveguides verbinden (Frage 1)

**A. Waveguide plus parallele Resonatoren für einzelne Teiltöne** (Bank 2001, Dissertation Q2, Überblick Q3)
- Ein Waveguide liefert alle Teiltöne. Wenige Resonatoren zweiter Ordnung laufen parallel dazu und erzeugen nur dort Schwebung und zweistufiges Abklingen (two-stage decay), wo es hörbar ist.
- Die Resonatoren werden nur für die Teiltöne gebaut, deren Schwebung und zweistufiges Abklingen erheblich ist; die übrigen Teiltöne klingen einfach exponentiell ab, wie der Waveguide es vorgibt [selbst gelesen, Q2 S. 54: „implemented only for those partials whose beating and two-stage decay are significant“].
- „Five to ten resonators have been found to be enough“ [selbst gelesen, Q2 S. 54].
- Beim Cembalo genügte ein einziger Resonator („only one resonator“) [selbst gelesen, Q2 S. 55, mit Verweis auf Välimäki et al.].
- Der Gedanke dahinter: Eine Sinuskomponente eines Modenpaars liefert der Waveguide-Teilton, die andere der Resonator [selbst gelesen, Q3 S. 947].
- Rechenaufwand: „less than five multiplications per sample“ je Note [selbst gelesen, Q3 S. 948]. Mit mehreren Abtastraten liegt er eine Größenordnung unter dem einer zweiten Saite: „an order of magnitude smaller computational resources compared to implementing a second digital waveguide“ [selbst gelesen, Q2 S. 58].

**B. Moden für die tiefen Teiltöne, Waveguide für die hohen** (Lee, Smith, Abel, Berners 2009, Q6; offenbar ausgebaut in Lee/Smith/Välimäki 2010, Q7)
- Der Waveguide ist auf den Grundton gestimmt und liefert alle Teiltöne bis zur halben Abtastrate. Eine Resonatorbank bildet die tiefen, psychoakustisch wichtigen Teiltöne nach [selbst gelesen, Q6 S. DAFX-6, sinngemäß].
- Die tiefen Teiltöne werden aus dem Waveguide entfernt („we remove them from our synthesis model“), und zwar durch Hochpass auf das Anregungssignal, offline, also ohne Kosten im Betrieb [selbst gelesen, Q6 S. DAFX-6].
- Die Tonhöhenänderung (pitch glide) entsteht durch Resonatoren mit zeitveränderlicher Frequenz statt durch Ändern der Verzögerungslänge: „it is only necessary to modulate the frequencies of the lower partials“ [selbst gelesen, Q6 S. DAFX-1].
- Je Teilton gibt es zwei Resonatoren mit Abklingrate, Frequenz, Anfangsphase und Anfangsbetrag; so lassen sich Schwebung und zweistufiges Abklingen genau einstellen [selbst gelesen, Q6 S. DAFX-6, sinngemäß].
- Q7 (Gitarre, gekoppelte Saiten) ist nur über Titel und Tonbeispiele belegt. Ein Seminar-Abstract von Lee (2009) spricht von Verfahren „to accurately tune and adjust individual partials of a digital waveguide model“ [wörtlich, Werkzeug; chrischafe.stanford.edu/events/virtual-string-synthesis]. Wie Q7 im Einzelnen aufgebaut ist, ist **offen**.

**C. Waveguide-Saite plus modaler Steg/Korpus** (Mansour/Woodhouse/Scavone 2016, Q9; Bank & Karjalainen 2010, Q14)
- **Q9:**
  - Die Saite ist ein Wellenmodell mit gemessenen Reflexionsfunktionen für Dispersion und Dämpfung.
  - Der Korpus geht als 2×2-Admittanzmatrix aus Moden ein, mit Frequenz, Güte, Modenmasse und „modal angle“; das koppelt beide Polarisationen. Gekoppelt sind auch mitschwingende Saiten [selbst gelesen, Q9 S. 6–8].
  - Die Moden wurden bis 2 kHz an Messungen angepasst. Darüber wurden 166 Moden statistisch erzeugt [selbst gelesen, Q9 S. 7].
- **Q14:** Eine Steg-Admittanz aus Filtern zweiter Ordnung, die selbst passiv sind, wird für den Waveguide in eine Reflektanz umgerechnet und an eine Gitarrensaite mit zwei Polarisationen angeschlossen [selbst gelesen, Q14 S. 1].

**D. Kommutierte Synthese (commuted synthesis)** (Smith/Van Duyne 1995; PASP Q15; bewertet in Q1)
- Die Korpus-Impulsantwort wird als Anregungstabelle in den Waveguide gegeben. Das setzt „linearity and time invariance“ voraus [wörtlich, Werkzeug, Q15].
- Nicht genau nachbildbar sind dabei Wiederanschlag (restrike) und nichtlineare Saitenschwingung [selbst gelesen, Q1 S. 9].
- Für den Bogen gilt die Erweiterung nur im Sonderfall des „ideal“ bowed attack [Werkzeug, Q15].

**E. Physik aus der Funktionaltransformation, Umsetzung als Waveguide** (FTM → DWG, Trautmann et al. 2002, Q8)
- Die Funktionaltransformationsmethode (functional transformation method, FTM; selbst eine Modalmethode, vgl. Q2 S. 27) rechnet Teiltonfrequenzen und Abklingraten aus den physikalischen Größen. Daraus werden die Filter des Waveguides entworfen [selbst gelesen, Q8 S. 1 und 9].
- Ergebnis: für lineare Saiten eine „acoustically indistinguishable DWG approximation“ [selbst gelesen, Q8 S. 1].
- Grenzen: Die Ersparnis sinkt mit zunehmender Dispersion. Bei häufiger Parameteränderung kosten die Filterentwürfe so viel wie die FTM selbst, und die Änderungen können hörbare Übergänge erzeugen [selbst gelesen, Q8 S. 9].

**F. Längswellen: Waveguide plus modale Längsresonatoren** (Bank 2006, Q2 Kap. 6; Bank & Sujbert 2005, Q5)
- Mit konzentriertem (lumped) Dispersionsfilter entstehen falsche Spitzen. Ursache: Die Modenformen sind „not orthogonal to each other“ [selbst gelesen, Q2 S. 113].
- Abhilfe ist, den Dispersionsfilter auf viele Stellen zu verteilen (Beispiel: 128 Allpässe erster Ordnung). Bei 50–100 Beobachtungspunkten wird der Aufwand dann „comparable to the finite-difference method or the modal-based approach“ [selbst gelesen, Q2 S. 113].
- Q1 begründet damit die Wahl des modalen Verfahrens: „precise modeling is not possible with digital waveguides in their efficient form“ [selbst gelesen, Q1 S. 3].

**G. Kopplung über einen gemeinsamen Steg** (Q15, Q1)
- PASP koppelt die Saiten über einen Streuknoten (scattering junction) am Steg und analysiert sie über eine „eigenanalysis of the coupling matrix“ [wörtlich, Werkzeug, Q15 Coupled_Strings].
- Q1 S. 9 [selbst gelesen]: Mitschwingen ist mit Waveguides „relatively straightforward … with bi-directional coupling“. Im modalen Modell wird die Kopplung dagegen auf eine Richtung vereinfacht („uni-directional“), weil die Stabilität sonst schwierig ist.

**Nicht geprüft oder nicht erreicht:** Karjalainen/Välimäki/Tolonen 1998, Välimäki et al. 1996, Aramaki 2001, Bensa 2003 (Volltext), Wave Digital Filters mit modalen Teilen, Erkut (Kantele) — siehe Abschnitt 7.

---

## 3. Was je Weg geht und was nicht (Frage 2)

**Steuerung je Teilton**
- Modal: „all the parameters of the partials can be set independently“ [selbst gelesen, Q1 S. 3]. Ebenso Q2 S. 29: Frequenzen, Amplituden und Abklingzeiten der Moden sind einzeln steuerbar, „the measured data can be directly uploaded“.
- Waveguide: Abklingen und Frequenz aller Teiltöne hängen an einer Schleife. Mit gemeinsamem Abschluss „the partial envelopes of the different notes cannot be controlled independently“ [selbst gelesen, Q3 S. 946].
- Ein Hochordnungs-Verlustfilter „cannot be tuned intuitively by the user“ [selbst gelesen, Q2 S. 47].

**Obergrenze der Teiltöne**
- Waveguide: „all partials are simulated up to Nyquist frequency“ [selbst gelesen, Q8 S. 6]. Ebenso Q6: „provides all partials up to half the sampling rate“ [S. DAFX-6].
- Modal: Die Moden werden über eine obere Frequenzgrenze und eine absolute Höchstzahl begrenzt [selbst gelesen, Q1 S. 11]. Im Flügel waren es 10 000 Resonatoren bei voller Polyphonie [Q1 S. 12].
- Wie genau die Teiltöne bis zur Nyquist-Frequenz sitzen, entscheidet beim Waveguide der Dispersionsfilter (siehe Inharmonizität).

**Frequenzabhängiger Verlust**
- Modal: je Mode ein eigenes R_k [selbst gelesen, Q1 S. 3].
- Waveguide: Verlustfilter. Die Abklingzeiten der Messung streuen so stark, dass Filter vertretbarer Ordnung ihnen nicht folgen. Deshalb glättet man sie vor dem Entwurf [selbst gelesen, Q4 S. 19].

**Inharmonizität**
- Waveguide braucht einen Allpass-Dispersionsfilter. Mehrere Dutzend Teiltöne lassen sich mit Ordnung ≤ 20 richtig setzen; ein Beispiel nutzt Ordnung 18 [selbst gelesen, Q3 S. 946].
- Rauhala & Välimäki stellen Kaskaden bis M = 40 Stufen dar, mit Fehlergrenzen von 0,5 % = „±8.39 cents“ [selbst gelesen, Q11 S. 4]. Die Inharmonizität B ist dabei in Echtzeit steuerbar [Q11 S. 3 und 5].
- Beim Clavinet reicht höchstens Ordnung 8 [selbst gelesen, Q12 S. 15].
- Modal: Inharmonizität folgt exakt aus der Formel (Q1 S. 3; FTM in Q8 S. 6: „the simulated frequencies have always the right pitch“).

**Nichtlineares**
- **Hammer:** in beiden Verfahren möglich (modal in Q1; Waveguide in Q3).
- **Längswellen und Phantomteiltöne:** modal „very straightforward“ [Q1 S. 3]. Beim effizienten Waveguide geht es nicht genau (siehe F oben).
- **Spannungsmodulation (tension modulation):** im Waveguide über veränderliche Verzögerungslänge mit Bruchteilverzögerung (fractional delay) [selbst gelesen, Q13 S. 3 und 6]. Modal über FTM oder energieerhaltende Modalverfahren [Q13 S. 3].
- **Bogen:** Für gestrichene Saiten kehrt sich das Verhältnis um. Die nichtlineare Reibungskraft „makes a time-domain approach more natural“ [selbst gelesen, Q9 S. 12]. Modal geht es aber auch, und zwar schneller als Echtzeit [selbst gelesen, Q10 S. 1 und 8].

**Zeitveränderliches (Bend, Vibrato)**
- Waveguide: Länge über die Bruchteilverzögerung ändern [Q13 S. 6].
- Lee et al. ändern die Verzögerungslänge bewusst nicht. Ihre Begründung: „aliasing is not a concern for our model“ [selbst gelesen, Q6 S. DAFX-1].
- Modal mit zeitveränderlicher Frequenz: Direktform-Strukturen „lead to artifacts such as amplitude modulation“, deshalb nehmen sie rekursive Oszillatoren [selbst gelesen, Q6 S. DAFX-7].
- Bei FTM→DWG können Parameteränderungen hörbare Übergänge erzeugen [Q8 S. 9].

**Rechenzeit**
- Nylonsaite: DWG 16/40 FLOPS je Abtastwert gegen FTM 90/177 [selbst gelesen, Q8 S. 6, Tabelle 1].
- Q1: modal „probably higher“ als Waveguide-Flügel, „one third of the resources of a today's PC“ [S. 3]; etwa 30 % auf einem Core 2 Duo [S. 12].
- Q2 S. 29: Modal liegt im Aufwand zwischen Finite-Differenzen-Verfahren und Waveguide.
- Clavinet: 82 Multiplikationen je Saite [Q12 S. 15].

---

## 4. Setzen an Aufnahmen (Frage 3)

**Waveguide-Weg**
- Aus der Aufnahme werden je Teilton Frequenz f_k und Abklingzeit τ_k gemessen. Daraus folgt die Vorgabe für die Schleifenverstärkung, g_k = e^(−k/(f_k τ_k)), und der Verlustfilter wird an diese Vorgabe angepasst [selbst gelesen, Q2 S. 23–24; Q4 S. 18].
- Die Frequenzen gehen in Dispersions- und Bruchteilverzögerungsfilter ein, getrennt entworfen [Q2 S. 24].
- Diese Trennung „cannot give mathematically optimal results“ [selbst gelesen, Q2 S. 24].
- Gewichtet wird über die Abklingzeit (Taylor-Gewichtung), denn die Abklingzeit hängt nichtlinear vom Filterbetrag ab; ein Betrag über 1 macht die Schleife instabil [selbst gelesen, Q4 S. 18–19].

**Gibt es eine schnelle Vorhersage je Teilton?**
- Ja, aber nur für die einzelne Schleife: Q2 Gl. (2.61) (S. 23) gibt τ_k direkt aus |H_r| bei der Teiltonfrequenz; die Frequenz folgt aus der Phasenverzögerung der Schleife [selbst gelesen].
- **Einschätzung:** Die Vorwärtsrechnung „Parameter → Frequenz und Abklingen je Teilton“ ist billig, nämlich der Frequenzgang des Schleifenfilters an den Teiltonstellen. Sie ist sogar eine **Rückwärtsformel** (aus der Messung folgt die Filtervorgabe direkt).
- Was sie nicht hat: zweistufiges Abklingen und Schwebung. Die einzelne Schleife erzeugt je Teilton nur „exponentially decaying sinusoids“ [selbst gelesen, Q4 S. 18], die übrigen Teiltöne klingen „simple exponential“ ab [Q2 S. 55].
- Bei gekoppelten Waveguides (Chor, zwei Ebenen) braucht die Vorhersage eine Eigenanalyse der Kopplung [Q15, Werkzeug]. Ein Kopplungsfilter muss entworfen werden; die Resonatorbank spart das: „there is no need for coupling filter design“ [selbst gelesen, Q3 S. 948].

**Was dabei verloren geht**
1. **Feinheit je Teilton:** Ein Filter niedriger Ordnung mittelt über die Teiltöne. Q4 glättet die Messdaten ausdrücklich; Q2 S. 47 nennt das Ergebnis „they just sound different“ (gemeint ist der Unterschied zwischen einpoligem und Hochordnungs-Filter).
2. **Zwei Stufen und Schwebung:** in der einzelnen Schleife nicht vorhanden. Abhilfe sind Resonatoren (Verfahren A/B) oder Schwebungs-Entzerrer (beating equalizer, Q12 S. 10–11; dort sind mehr als drei gleichzeitig kaum hörbar).
3. **Phase:** Die Anfangsphasen der Teiltöne folgen aus Anregung und Abgriffsort, nicht aus eigenen Parametern. **Einschätzung, kein Beleg.** Bei den Resonatoren sind Anfangsamplitude und -phase dagegen frei: In Q3 S. 948 gleichen sie Restfehler der Mehrraten-Filter aus; in Q6 S. DAFX-6 gehören sie zu den Parametern je Teilton.
4. **Fehler der Inharmonizität** bei hohen Teiltönen außerhalb des angepassten Bereichs [Q11 S. 4, Abbildung 5].

---

## 5. Tabelle: Verfahren × Eigenschaften

| Verfahren | Steuerung je Teilton | Obergrenze | Verlust | Nichtlineares | Zeitveränderliches | Rechenzeit | Setzen an Aufnahme |
|---|---|---|---|---|---|---|---|
| **Modal allein** (Q1) | voll [Q1 S. 3] | Grenzfrequenz und Höchstzahl [Q1 S. 11] | je Mode [Q1 S. 3] | Hammer, Längswellen, Phantome einfach [Q1 S. 3]; Chor nur einseitig gekoppelt [Q1 S. 9]; Bogen modal möglich [Q10] | Frequenz je Mode änderbar; Artefakte je nach Struktur [Q6 S. DAFX-7] | höher als DWG [Q1 S. 3]; 3 Multiplikationen je Resonator [Q1 S. 11] | Messung direkt übernehmbar [Q2 S. 29]; je Teilton schnell vorhersagbar (Entscheid SZ3 a) |
| **DWG allein** (Q4, Q11, Q12) | nein, nur gesamt [Q3 S. 946; Q2 S. 47] | bis Nyquist [Q8 S. 6] | Filter, geglättet [Q4 S. 19] | Hammer ja; Längswellen ungenau ohne verteilte Dispersion [Q2 S. 113]; Bogen natürlich [Q9 S. 12] | Länge über Bruchteilverzögerung [Q13 S. 6]; Aliasing-Thema [Q6 S. DAFX-1] | am niedrigsten [Q8 S. 6; Q12 S. 3] | Filterentwurf aus f_k und τ_k [Q2 S. 23–24]; eine Stufe je Teilton [Q4 S. 18] |
| **A: DWG + Resonatorbank** (Q2, Q3) | nur für die 5–10 Resonator-Teiltöne [Q2 S. 54] | bis Nyquist (DWG) | DWG-Filter plus Resonatoren | wie DWG | Einschätzung: Resonatoren lassen sich mitführen, die Gleichheit mit dem Waveguide-Teilton ist aber nötig | < 5 Multiplikationen je Note zusätzlich [Q3 S. 948] | Modenpaare je Teilton plus Verlustfilter; einfacher als Kopplungsfilter [Q3 S. 948] |
| **B: Moden tief + DWG hoch** (Q6, Q7) | voll für die tiefen Teiltöne [Q6 S. DAFX-6] | bis Nyquist (DWG) | modal tief, Filter hoch | Bend durch die Resonatoren [Q6 S. DAFX-1] | Glide mit zeitveränderlichen Resonatoren ohne Längenänderung [Q6 S. DAFX-1] | Einschätzung: modale Kosten nur für wenige Teiltöne | tief: je Teilton aus der Messung [Q6 S. DAFX-2]; hoch: Filterentwurf. Q7 nicht gelesen |
| **C: DWG-Saite + modaler Korpus/Steg** (Q9, Q14) | Saite nein; Korpus je Mode | bis Nyquist; Korpusmoden bis 7 kHz, statistisch [Q9 S. 7] | Reflexionsfunktionen aus Messung [Q9 S. 4] | zwei Polarisationen und Mitschwingen über den Steg [Q9 S. 6–9]; Passivität beachten [Q14 S. 1] | Einschätzung: wie DWG | Einschätzung: Korpusmoden kosten den Hauptteil | Korpus aus Admittanzmessung (Pol-Residuen-Anpassung) [Q9 S. 7]; Saite aus gemessener Dämpfung und Dispersion |
| **D: kommutiert** (Q15, Q1) | nein | bis Nyquist | wie DWG | Hammer nur als Filter; Wiederanschlag nicht genau [Q1 S. 9; Q3 S. 942] | Einschätzung: Saite wie DWG | sehr niedrig [Q1 S. 9] | Anregungstabelle aus Aufnahme bzw. Korpusantwort |
| **E: FTM → DWG** (Q8) | Physik je Mode, Umsetzung gesamt | bis Nyquist | aus der Physik entworfen | nur linear [Q8 S. 9] | Parameteränderungen teuer, Übergänge hörbar [Q8 S. 9] | Ersparnis gegenüber FTM; sinkt mit der Dispersion [Q8 S. 9] | aus der Physik, nicht aus der Aufnahme |
| **F: DWG + modale Längsresonatoren** (Q2 Kap. 6, Q5) | Längsmoden ja | — | — | Phantome; mit konzentriertem Dispersionsfilter falsche Spitzen [Q2 S. 113] | — | mit verteilter Dispersion etwa wie modal [Q2 S. 113] | Einschätzung: aufwendig |

---

## 6. Abgleich mit der Claude-Antwort in LumiViz

1. **„Waveguide gibt alle Teiltöne ohne Obergrenze“ — bestätigt, mit Einschränkung.**
   - Er erzeugt alle Teiltöne bis zur Nyquist-Frequenz [Q8 S. 6; Q6 S. DAFX-6].
   - Richtig sitzen sie aber nur so weit, wie der Dispersionsfilter angepasst ist (mehrere Dutzend Teiltöne bei Ordnung ≤ 20 [Q3 S. 946]; Fehlergrenze ±8,39 Cent [Q11 S. 4]).
   - Die Grenze „Nyquist“ hat auch das modale Modell; dort ist sie nur ausdrücklich gezogen [Q1 S. 11].

2. **„Frequenzabhängiger Verlust per Filter“ — bestätigt, aber kein Vorteil.**
   - Das modale Modell hat ihn je Mode exakt [Q1 S. 3].
   - Der Waveguide-Filter muss glätten [Q4 S. 19].

3. **„Nichtlineares natürlich“ — teils bestätigt, teils widerlegt.**
   - Bestätigt für die Bogenreibung [Q9 S. 12] und die Spannungsmodulation [Q13 S. 3].
   - Widerlegt für Längswellen und Phantomteiltöne. Mit effizientem Waveguide geht es nicht genau [Q1 S. 3; Q2 S. 113], modal dagegen „very straightforward“ [Q1 S. 3].
   - Den Hammer haben beide Verfahren.

4. **„Bend natürlich“ — teils bestätigt.**
   - Die Länge lässt sich über die Bruchteilverzögerung ändern [Q13 S. 6].
   - Lee et al. weichen der Längenänderung aber wegen Aliasing aus und machen den Glide modal [Q6 S. DAFX-1].
   - Wie gut sich Bend und Vibrato je Verfahren bei vorab gerechneten Tönen anhören, ist in den gelesenen Quellen **offen**.

5. **„Er nimmt die Steuerung je Teilton“ — bestätigt** [Q3 S. 946; Q2 S. 47].
   - Die Kombinationen A und B geben sie für ausgewählte Teiltöne zurück [Q2 S. 54; Q6 S. DAFX-6].

6. **„… und die modale Vorhersage fürs Setzen“ — teils widerlegt.**
   - Für die einzelne Schleife gibt es eine geschlossene Formel je Teilton (τ_k aus |H_r|, f_k aus der Phasenverzögerung) [Q2 S. 23].
   - Sie kennt aber nur eine Stufe und keine Schwebung [Q4 S. 18].
   - Für gekoppelte Schleifen (Chor, zwei Ebenen) braucht es Eigenanalyse und Kopplungsfilter [Q15; Q3 S. 948].
   - Im Kern stimmt die Aussage also für das, was SZ3 a braucht (zwei Stufen, Schwebung, Verstimmung).

7. **„Kopplung im Chor am Steg hat dieselbe Physik wie bei Moden“ — bestätigt, mit wichtiger Ergänzung.**
   - Die Physik ist dieselbe (Weinreich, zitiert in Q1 S. 9).
   - Die Umsetzung unterscheidet sich zugunsten des Waveguides: Zweiseitige Kopplung ist dort „relatively straightforward“. Modal wird aus Stabilitätsgründen auf einseitige Kopplung vereinfacht [Q1 S. 9; Q2 S. 29].

---

## 7. Nicht gefunden / nicht gelesen

- **Lee/Smith/Välimäki 2010 (Q7):** Inhalt nicht erreichbar, kein Abstract in Crossref oder Semantic Scholar, IEEE leer. Zuschnitt und Kalibrierung des hybriden Modells sind daher **offen**.
- **Bensa et al. 2003 (Q16):** nur als Zusammenfassung des Abstracts (Verbindung von Messparametern zu Waveguide-Filtern, Erweiterung auf gekoppelte Saiten). Kein Beleg.
- **Ohne Treffer oder nicht erreicht:** Aramaki et al. 2001 (HAL-API zweimal ohne Treffer); Välimäki et al. 1996 (nur Suchmaschinen-Zusammenfassung); Karjalainen/Välimäki/Tolonen 1998 (nur Metadaten); Erkut (Kantele) und Rauhala, Dissertation 2007 (nicht erreicht).
- **Nicht gesucht:** Wave Digital Filters mit modalen Teilen; Hackbrett und Cimbalom.
- **Maestre/Scavone/Smith 2017** (Waveguide-Violine plus Steg-Admittanz und Abstrahlung): nur Titel und Metadaten.
- **Cembalo-Artikel Välimäki et al. 2004:** Volltext durch Springer gesperrt; der eine Resonator ist nur über Q2 S. 55 belegt.
- Kein Treffer ist kein Beweis.

---

## 8. Eigene Einschätzung (kein Beleg)

**Klasse 1, Hammer-Saite (Flügel, Upright, Clavinet, Hackbrett, Cimbalom)**
- Der modale Weg in LumiViz deckt sich mit Q1, und SZ3 a (Setzen am Modell der Moden je Teilton) findet in der Literatur klare Stütze. Der Waveguide hat diese Vorhersage für zwei Stufen, Schwebung und Längswellen nicht.
- Eine Kombination lohnt höchstens als **B: Moden bis zu einer Grenzfrequenz, darüber ein Waveguide**, dessen tiefe Teiltöne über den Hochpass auf die Anregung entfernt sind [Q6]. Das brächte hohe Teiltöne bis zur Nyquist-Frequenz ohne Kosten je Mode, verlöre dort aber die Steuerung je Teilton und die genaue Inharmonizität.
- Da die Töne vorab gerechnet werden, wiegt der Rechenvorteil des Waveguides wenig. Ich sehe für den Flügel keinen zwingenden Grund für die Kombination; offen ist, ob die Obergrenze der Moden in den hohen Lagen hörbar ist.
- Für das Clavinet ist der Waveguide belegt [Q12], mit Schwebungs-Entzerrern statt Resonatoren. Wer das Setzen nach SZ3 a will, nimmt eher modal oder Verfahren A.
- Hackbrett und Cimbalom haben mehrchörige Saiten auf einem gemeinsamen Steg. Dort spricht die einfache zweiseitige Kopplung im Waveguide [Q1 S. 9] für C oder G. Das ist unbelegt.

**Klasse 2, gezupfte Saite (Gitarre, Harfe, Cembalo)**
- Der Waveguide-Bestand in LumiViz ist gut gestützt: Korpus als Moden bzw. Admittanz hinter dem Steg (Verfahren C) [Q9, Q14].
- Fürs Setzen nach Muster SZ3 a böte sich **A** an (Waveguide plus 1–5 Resonatoren für schwebende Teiltöne; beim Cembalo genügt einer [Q2 S. 55]) oder **B** (tiefe Teiltöne modal, mit Pitch-Glide [Q6]). Damit käme die Vorhersage je Teilton für die tiefen Teiltöne zurück; der Rest bleibt Filterentwurf.
- Die Harfe mit vielen offenen Saiten spricht wie unter Klasse 1 für die zweiseitige Kopplung im Waveguide.

**Klasse 3, Bogen-Saite (Violinfamilie)**
- Die Reibung ist der Kern, und die Wellenform ist hier natürlicher [Q9 S. 12]. Modal ist möglich und schnell [Q10], hat aber keinen Vorteil beim Setzen: Beim Bogen entscheidet die Wechselwirkung (Helmholtz-Bewegung), nicht das Abklingen je Teilton.
- Angemessen erscheint mir **C: Waveguide-Saite mit gemessener Dämpfung und Dispersion plus modaler Korpus als 2×2-Admittanz** [Q9]. Die schnelle Vorhersage je Teilton fürs Setzen gibt es dort für die freie Saite (Q-Faktoren und Inharmonizität, vgl. Q9 S. 10–11), nicht für den gestrichenen Ton.
- Die kommutierte Synthese gilt nur für ideale Einsätze [Q15] und passt deshalb nicht zum Steuerverlauf (Bogenführung).

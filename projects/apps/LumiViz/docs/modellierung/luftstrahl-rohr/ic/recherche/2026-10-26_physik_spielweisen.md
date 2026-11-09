# Recherche IC1: Luftstrahl und Rohr — Strahl, Anblasgeräusch, Bohrungen, Einschwingen, Spielweisen

> **Auftrag:** Klasse 9 (Querflöte, Altflöte, Blockflöte, Shakuhachi, Labialpfeifen): jet-drive-Modell mit Werten,
> Anblasgeräusch, Bohrungs- und Mensurdaten, Einschwingen, Spielweisen, Abgleich mit dem Bestand; aufbauend auf
> `gemeinsam/rohr.md`.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen] = im PDF-Bild selbst gelesen; [wörtlich, Werkzeug] = vom Werkzeug als Zitat;
> [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (25 Suchen, 34 Abrufe, 5 PDFs; HAL-Volltexte, Acta Acustica, natmus.dk,
> de la Cuadra 2005 nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Klasse 9 „Luftstrahl und Rohr“: Vorab-Recherche (IC, Abruf 2026-10-26)

## Reichweite

- **Umfang:** 25 Websuchen, 34 Abrufe (WebFetch), dazu 5 Lesevorgänge in PDFs, die das Abrufwerkzeug selbst zwischengespeichert hat. Das sind etwa 60 Aufrufe und damit mehr als die vorgegebenen 30–45. Grund: Viele Abrufe sind gescheitert und mussten anders wiederholt werden.
- **Sprachen der Suche:** Englisch. Deutsche und japanische Begriffe kamen nur als Stichwort vor (Normmensur, meri/kari, jiari).
- **Orte:** arXiv und ar5iv, HAL-API, J-STAGE, UNSW (newt.phys.unsw.edu.au), ANU Open Research, Crossref-API, die Seite von M. Castellengo am LAM (lam.jussieu.fr), flute-a-bec.com (Ph. Bolton), colinpykett.org.uk, Wikipedia, ccrma.stanford.edu, Acoustics Today.
- **Nicht erreichbar:**
  - HAL-Volltexte (hal.science, hal.sorbonne-universite.fr): Bot-Sperre „Anubis“.
  - acta-acustica.edpsciences.org, HTML und PDF: HTTP 403.
  - natmus.dk: Website wegen Sicherheitsupdate geschlossen.
  - labs.sonicfield.org: DNS-Fehler.
  - Dissertation de la Cuadra 2005 (ccrma): größer als 10 MB, nicht ladbar.
  - Fletcher 1976a (UNSW-PDF): Werkzeugfehler.
  - ANU-PDF zu Fletcher 1975: ein Scan ohne Textebene, vom Werkzeug nicht lesbar.
- **Vom Werkzeug zwischengespeicherte PDFs** (selbst nichts heruntergeladen), alle im Ordner `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793033854405-au0fv0.pdf`: arXiv 1402.3143
  - `webfetch-1793033922885-9ihb0p.pdf`: Fletcher, ANU, Scan
  - `webfetch-1793034049732-jfldfr.pdf`: Physics Today, April 2026, Angster/Llorca-Bofí/Miklós, eine Seite, ohne Zahlen
  - `webfetch-1793034104928-puv3wa.pdf`: Castellengo ISMA 2004, selbst gelesen
  - `webfetch-1793034118665-591abq.pdf`: Castellengo 1999, S. 387–389 selbst gelesen
  - `webfetch-1793034324939-ssbs8c.pdf`: Fletcher & Douglas 1980, S. 767–768 selbst gelesen
  - `webfetch-1793034354225-g6tac4.pdf`: Bolton, Blockflöten-Block, S. 1–3 selbst gelesen
- **Zur Kennzeichnung:** Hinter „[wörtlich, selbst gelesen]“ steht ein Wortlaut, den ich im PDF-Bild selbst gelesen habe. „[wörtlich, Werkzeug]“ heißt: Das Werkzeug gab den Text als Zitat oder Feldinhalt zurück.

---

## 1. Strahl und Kante: das jet-drive-Modell (Strahlantrieb)

### Quellen und Rechte

- **Terrien, Vergez, de la Cuadra, Fabre:** „Is the jet-drive flute model able to produce modulated sounds like Flautas de Chinos ?“
  - Kommentar auf arXiv: „Stockholm Music Acoustics Conference, Stockholm : Sweden (2013)“ [wörtlich, Werkzeug]
  - Fundort: arXiv 1402.3143, frei zugänglich
  - Lizenz: „http://arxiv.org/licenses/nonexclusive-distrib/1.0/“ [wörtlich, Werkzeug, arxiv.org/abs/1402.3143, 2026-10-26]. Das ist keine freie Lizenz.
- **Terrien, Blandin, Vergez, Fabre:** „Regime change thresholds in flute-like instruments: influence of the mouth pressure dynamics“
  - Fundort: arXiv 1403.7487; eine Journal-Angabe fehlt auf arXiv
  - Lizenz: dieselbe arXiv-Lizenz [wörtlich, Werkzeug, 2026-10-26]
- **Auvray, Ernoult, Terrien, Fabre, Vergez:** „Effect of Changing the Vocal Tract Shape on the Sound Production of the Recorder: An Experimental and Theoretical Study“
  - Journal-Angabe: „Acta Acustica united with Acustica, Hirzel Verlag, 2015, 101, pp.317-330“ [wörtlich, Werkzeug]
  - Fundort: arXiv 1601.05545; Lizenz wie oben [wörtlich, Werkzeug, 2026-10-26]
- **Fletcher, N. H. & Douglas, Lorna M.:** „Harmonic generation in organ pipes, recorders, and flutes“
  - Fundstelle: „J. Acoust. Soc. Am. 68(3), Sept. 1980“, S. 767–771
  - Rechte: „© 1980 Acoustical Society of America“ [wörtlich, selbst gelesen, S. 767]
  - Fundort: PDF frei abrufbar auf newt.phys.unsw.edu.au (Fletcheretal1980.pdf). Im ANU-Repositorium ist die Arbeit laut Suchergebnis bis 2037 gesperrt [nur Zusammenfassung].

### Parameter mit Werten

**Konvektion und Laufzeit**
- Konvektionsgeschwindigkeit (convection velocity) der Störung: allgemein „0.3U_j ≤ c_p ≤ 0.5U_j“; in der Simulation c_p = 0.4 U_j [wörtlich, Werkzeug, 1402.3143, Abschn. 3.2.1 / Tab. 2].
- Laufzeit τ = W/c_v, mit „c_v ≈ 0.4 U_j“ [wörtlich, Werkzeug, 1403.7487, Abschn. 2.3].

**Verstärkung entlang des Strahls (amplification)**
- α_i: Tabelle 2 nennt „0.4/h = 400“ [wörtlich, Werkzeug, 1402.3143].
- Die Blockflöten-Simulation in 1601.05545, Tab. 1, trennt zwei Strahlformen [wörtlich, Werkzeug]:
  - antisymmetrisch: c_p = 0.4⟨u⟩, α_i = 0.3/h
  - symmetrisch: c_p = 0.8⟨u⟩, α_i = 0.2/h

**Empfänglichkeit (receptivity)**
- Modell: Querauslenkung des Strahls am Kanalausgang, η(0,t) = (h/U_j)·v_ac. In der Suche wird es de la Cuadra zugeschrieben [nur Zusammenfassung].
- Zitat dazu: „the receptivity is represented as a transverse displacement“ [wörtlich, Werkzeug, 1402.3143, Abschn. 3.2.1].

**Strahlgeschwindigkeit aus dem Druck**
- Bernoulli: U_j = 39 m/s entspricht „a mouth pressure of about 915 Pa“ [wörtlich, Werkzeug, 1402.3143, Abschn. 4].

**Geometrie**
- Querflöten-Simulation: „W = 1cm, H = 6mm and y0 = 0.2 mm“, dazu h = 1 mm [wörtlich, Werkzeug, 1402.3143, Abschn. 4 / Tab. 2].
- Blockflöten-Labor (1601.05545, Abschn. 3.1 / Tab. 1) [nur Zusammenfassung; Formatierung auf der Seite teils zerstört]:
  - W = 4 mm, h = 1 mm, Innendurchmesser 19 mm
  - Länge „26.5 cm“ [wörtlich, Werkzeug]
  - y0 = 0.1 mm, H = 12 mm

**Strahlprofil und Sättigung**
- Bickley-Profil mit b = 2h/5 [wörtlich, Werkzeug, 1402.3143, Abschn. 3.2.2].
- Fletcher & Douglas setzen „V(y) = V0 sech²(y/b)“ an und erhalten den Fluss in das Rohr „U(t) = V0 b{1 + tanh[(y0 + a sin ωt)/b]}“ [wörtlich, selbst gelesen, S. 768, Gl. 1–2]. Das ist die tanh-Sättigung.
- Abstand der Quellpunkte: δ_d ≈ (4/π)√(2hW), nach Verge [wörtlich, Werkzeug, 1402.3143, Abschn. 3.2.2].

**Verluste**
- „modeled as nonlinear losses“, mit α_vc ≈ 0.6 als vena-contracta-Faktor [wörtlich, Werkzeug, 1402.3143, Abschn. 3.2.3].

**Resonator**
- In 1601.05545 ist der Resonator **modal mit drei Moden** angesetzt [nur Zusammenfassung von Tab. 1]:
  - ω1 = 3547 rad/s, ω2 = 2.023 ω1, ω3 = 3.066 ω1
  - Dämpfungen ε ≈ 0.03–0.04
- Terrien u. a. 2012 (arXiv 1207.7136) schreiben den Resonator ebenfalls als Summe von Moden [nur Zusammenfassung].

**Bereich des Modells**
- Organisten-Pfeifen haben w/h etwa 10, Blockflöten w/h etwa 4: „For recorders, builders choose a ratio w/h of 4“ [wörtlich, selbst gelesen, Castellengo 1999, S. 389].
- Auvray u. a. 2014 erweitern das Kriterium W/h zu einem „dynamic aspect ratio“ [wörtlich, Werkzeug, HAL-API-Abstract hal-01426971].

### Register und Überblasen

- **Schwellen beim Querflöten-Griff G4:** statische Schwelle „at Pm=311.5 Pa“; bei F4 und G4 „294 Pa and 314 Pa“ [wörtlich, Werkzeug, 1403.7487, Abschn. 3.2 und 4.2].
- **Einfluss der Anstiegsrate:** Die dynamische Schwelle wächst mit der Rate a: „Increasing a thus enlarges the hysteresis“. Im Modell gilt „proportional to the slope a to the power 2/3“ [wörtlich, Werkzeug, 1403.7487, Abschn. 3.1 und 4.2].
- **Spieler gegen künstlichen Mund:** Beim Musiker ist die Hysterese 169 % bis 380 % breiter [wörtlich, Werkzeug, Abschn. 1].
- **Blockflöte:**
  - Schwelle 1.→2. Register steigend 695 Pa, fallend 534 Pa (ohne Kopplung an den Vokaltrakt)
  - je nach Phase des Vokaltrakts 630–745 Pa bzw. 495–520 Pa [nur Zusammenfassung, 1601.05545, Abschn. 3.3]
- **Verhältnis θ = W/(U_j·T):** In 1403.7487 nicht beziffert [Werkzeug-Auskunft]. Das Bild der „halben Wellenlänge zwischen Spalt und Lippe“ bei Fletcher & Douglas siehe unten.

### Tonhöhe und Klangfarbe über dem Druck

- **Tonhöhe:** Steigt der Blasdruck, wird die Laufzeit kürzer und die Frequenz steigt leicht; der Satz beginnt mit „If the blowing pressure is increased to shorten the jet travel time“ [wörtlich, selbst gelesen, Fletcher & Douglas 1980, S. 767]. Dieselbe Stelle nennt die Bedingung „just half a wavelength“ der Störung zwischen Spalt und Lippe.
- **Gerade Harmonische:** Sie verschwinden bei y0 = 0: „even harmonics are all identically zero when the lip is symmetrically placed“ [wörtlich, selbst gelesen, S. 768].
- **Ordnung der Harmonischen:** Die n-te Harmonische wächst mit der n-ten Potenz der Grundton-Amplitude [wörtlich, selbst gelesen, S. 768: „varies as the nth power“].
- **Folgerung für Querflöte und Orgelpfeife:** Spieler (Querflöte) und Intonateur (Orgelpfeife) steuern mit dem Versatz y0 vor allem die zweite Harmonische.
- **Klangfarbe und W:** „strong influence of the distance W … on the timbre“ [wörtlich, Werkzeug, Suchergebnis-Abstract Verge, Fabre, Hirschberg (Wijnands) 1997, JASA 101(5), 2914–2924]. Die dimensionslose Amplitude des Grundtons hängt bei fester Geometrie nur von der Strouhal-Zahl ab [ebd., wörtlich, Werkzeug].
- **Querflöte, laut gegen leise:** Laut wachsen vor allem die Oberteiltöne: „the higher harmonics become more important as the note is played more loudly“ [wörtlich, Werkzeug, UNSW fluteacoustics.html]. Laut Fletcher 1975 betrifft der Unterschied piano/forte hauptsächlich die oberen Teiltöne [nur Zusammenfassung].
- **Strahlwinkel:** Onogi u. a., Acta Acustica 5 (2021) 11, DOI 10.1051/aacus/2021003, CC BY 4.0 [nur Zusammenfassung; Seite 403]. Der Strahlwinkel wirkt auf den Pegelunterschied H2/H3 schwächer als der Versatz und stärker als der Abstand Lippe–Kante.

### Querflöte gegen feste Kernspalte

- **Spieler regelt Abstand und Öffnung:** „moving the lips forward to shorten the distance along the jet“ für hohe Töne; „reduce the lip aperture when playing high notes“ [wörtlich, Werkzeug, UNSW fluteacoustics.html].
- **Strahlgeschwindigkeit:** „typically in the range 20 to 60 metres per second“ [wörtlich, Werkzeug, ebd.].
  - Dieselbe Seite nennt laut Werkzeug als Mundruck „typically a few tens of kPa“. Das widerspricht den 300–900 Pa bei Terrien. Ich halte es für einen Fehler des Werkzeugs oder der Seite; **ungeprüft**.
- **Messungen an Spielern:** de la Cuadra, Fabre, Montgermont, Chafe 2008 („Analysis of Flute Control Parameters…“) messen Abstand Lippe–Kante, Form der Lippenöffnung und Strahlgeschwindigkeit; Zahlen nicht erreicht [nur Zusammenfassung].
- **Strahllänge über der Frequenz:** Fletcher 1975 (vier Spieler) berichtet eine Beziehung Strahllänge–Frequenz, alle Messwerte innerhalb ±20 % [nur Zusammenfassung].
- **Feste Kernspalte:** Bei Blockflöte und Orgelpfeife sind W, h und y0 fest. Der Spieler hat bei der Blockflöte nur Druck (und den Vokaltrakt, 1601.05545). Bei der Orgelpfeife ist auch der Druck fest.

---

## 2. Anblasgeräusch

- **Hirschberg & Verge:** „Turbulence Noise in Flue Instruments“, 1995. HAL hal-01157132, Lizenzfeld „https://about.hal.science/hal-authorisation-v1/“ [wörtlich, Werkzeug, HAL-API, 2026-10-26]. Das PDF ist durch Anubis gesperrt.
  - Inhalt laut Abstract [nur Zusammenfassung]: ein einfaches Rauschmodell für Zeitbereichs-Simulationen, gewonnen aus Spektren in einer kleinen blockflötenartigen Pfeife.
  - Die Quelle ist dipolartig und skaliert mit U_j². Ihr Maximum liegt bei der Strouhal-Zahl größter Instabilität eines ebenen Strahls (Re > 3000).
  - Eine Zahl für diese Strouhal-Zahl habe ich nicht erreicht.
- **Drei Schallquellen** (Strahl, Wirbelablösung an der Kante, Turbulenz im Mund des Instruments): Verge u. a., JASA 1996/97 [nur Zusammenfassung].
- **Rauschen und Druck (Castellengo 2004):** „Turbulence noise increases with the supply pressure“ [wörtlich, selbst gelesen, S. 2].
  - Shakuhachi: Atemgeräusch „well pronounced around 9 kHz“, in der Nähe der ersten Quermode [wörtlich, selbst gelesen, S. 4].
  - Querflöte (19 mm): erste Quermode bei etwa 10,5 kHz, nach Pierce [wörtlich, selbst gelesen, S. 4].
- **Mehr Druck bringt mehr Rauschen:** „higher jet velocities produce more turbulence“ [wörtlich, Werkzeug/Suchergebnis, NIME 2005 Silva u. a.].
- **Größeres W:** lauter, aber auf Kosten zusätzlichen Turbulenzrauschens [nur Zusammenfassung, Verge 1997].
- **Synthese (Chafe):** „Adding Vortex Noise to Wind Instrument Physical Models“, ICMC Banff, Sept. 1995, S. 57–60. Seite ccrma.stanford.edu/~cc/pub/html/addVortNoi.html, PDF verlinkt, nicht gelesen.
  - Abstract ASA 1993: Das Rauschen der Flöte sei kontinuierlicher als bei Streich- und Rohrblatttönen [nur Zusammenfassung].
- **Pegel des Rauschens relativ zum Ton in dB:** keinen Messwert gefunden.

---

## 3. Bohrung und Daten

### Böhm-Querflöte
- Kein offen lizenzierter Datensatz gefunden.
- Ansätze [nur Zusammenfassung]:
  - McGee (mcgee-flutes.com, „Boehm Head Bore“, eigene Messungen, ohne Lizenzangabe)
  - Coltman zu Böhms Entwurf (JAMIS 1983, amis.mircat.org)
  - UNSW-Impedanzen (bekannt, ohne Lizenz)
- Zwei UNSW-Seiten habe ich geprüft (fluteacoustics.html und die Shakuhachi-Seite RLsoundclips.html). Beide tragen nur einen Copyright-Hinweis auf Joe Wolfe; das Zeichen wird verstümmelt dargestellt. Keine Lizenz [Werkzeug, 2026-10-26].

### Blockflöte
- **Bolton, Philippe:** „Making a new block for a recorder“, flute-a-bec.com/bouchongb.pdf, „written in 1983“ [wörtlich, selbst gelesen, S. 1].
  - Kanal: „the windway entrance is often a little more than 1 mm high and the exit about 0,9 mm“ [S. 1]
  - Fasen: „two small chamfers about 0.5 to 1 mm wide“ [S. 2]
  - Block: „about 1/10 mm below the level of the labium“ [S. 2]
  - Rampe unter dem Labium: etwa 1 cm bei der Altblockflöte [S. 2]
  - Vorbehalt: „They are just given as guidelines“ [S. 2]
  - Auf S. 1–3 kein Copyright- oder Lizenzvermerk gesehen.
- **Bauzeichnungen:** Kopenhagener Musikmuseet (Zeichnungen Bergstrøm, Denner-Maße) laut Suchergebnis nur privat kopierbar, Veröffentlichung nur mit schriftlicher Erlaubnis [nur Zusammenfassung]. Der eigene Abruf scheiterte, weil natmus.dk geschlossen war. Wortlaut also **nicht** belegt.
- **Weitere Wege:** ICOM-CIMCIM-Register technischer Zeichnungen, ohne Lizenzprüfung [nur Zusammenfassung].
- **Ernoult u. a.:** Full-Waveform-Inversion, Acta Acustica 5 (2021) 47, DOI 10.1051/aacus/2021038, CC BY [nur Zusammenfassung]. Ein Datensatz dazu ist nicht gefunden.

### Shakuhachi
- **Yoshikawa & Kajiwara:** „Cross fingerings and associated intonation anomaly in the shakuhachi“, Acoust. Sci. & Tech. 36(4) (2015) 314–325, DOI 10.1250/ast.36.314, frei lesbar.
  - Rechtevermerk: „© 2015 by The Acoustical Society of Japan“ [wörtlich, Werkzeug, J-STAGE-Seite, 2026-10-26]
  - Ohne Bohrungsmaße auf der Seite.
  - Zu meri/kari gibt es keine Yoshikawa-Arbeit im Treffer.
- **UNSW (Wolfe/Smith):** Eingangsimpedanz für „ro“ und „tsu“, Instrument „Bei Shu“ [nur Zusammenfassung].
  - Zitat zum Spielen: „The Z(f) of the radiation field varies for different playing positions“ [wörtlich, Werkzeug, shakuhachiresults.html]
  - Keine Lizenz.
- **Museumsdatensatz (NMM, emuseum.nmmusd.org):** 603 mm Länge, Bohrung 19,7–22,3 mm, utaguchi 11,4 × 2,8 mm [nur Zusammenfassung; Rechte nicht geprüft].

### Orgel-Labialpfeifen
- **Normmensur:** Töpfer-Normalmensur „155.5 mm (6.12 in) at 8′ C“, Halbierung auf der 17. Note, Mundbreite „one-quarter of the circumference“ [wörtlich, Werkzeug].
  - Lizenz: „Text is available under the Creative Commons Attribution-ShareAlike 4.0 License“ [wörtlich, Werkzeug, en.wikipedia.org/wiki/Organ_flue_pipe_scaling, 2026-10-26].
  - Nach der globalen Regel sind Wikipedia-Angaben prüfbedürftig.
- **Rucz, Esteve Fontestad, Angster, Miklós:** „Wall losses in straight ducts with non-circular cross sections: a scaling rule of identical losses for wooden flue organ pipes“, Acta Acustica 9 (2025) 10, DOI 10.1051/aacus/2024084.
  - Lizenz laut Crossref-Feld: „https://creativecommons.org/licenses/by/4.0“ [wörtlich, Werkzeug, Crossref-API, 2026-10-26]
  - Volltext: HTTP 403, Maße nicht gelesen.
  - Laut Suchergebnis wird bei Holzpfeifen der Aufschnitt beim Intonieren eingestellt [nur Zusammenfassung].
- **Weitere Rucz/Angster-Arbeiten** (CFD des Pfeifenfußes 2014, Stimmschlitz 2013, ISMA 2010) [nur Zusammenfassung]. Die Rechte habe ich nicht geprüft.
- **Messpfeife bei Castellengo 2004:** „length = 312 mm; internal diameter = 27 mm; mouth dimensions lxL = 21x7 mm; flue width 0.2 <h <0.3“, erste Mode 475 Hz [wörtlich, selbst gelesen, S. 1].
- **Synchronisationsversuch (arXiv physics/0506094):** Aufschnitt 7 mm, Mundbreite 37 mm, Spalt 0,4 mm [nur Zusammenfassung].
- **Druckabhängigkeit, Pykett:** gemessene gedackte Pfeife, Frequenz linear im Winddruck: f = 0.0829 p + 170.2 Hz (p in mm WS, gültig etwa 50–110 mm WS) [nur Zusammenfassung].
- **Aufschnitt und Klang:** Niedriger Aufschnitt klingt heller, hoher weicher (Diapason-Artikel); bei hohem Winddruck wird mehr Aufschnitt genommen (toff.org.uk) [nur Zusammenfassung].
- **Fletcher, N. H.:** „Sound production by organ flue pipes“, J.A.S.A. 60 (4) (1976) 926–936, so zitiert bei Castellengo 2004 [wörtlich, selbst gelesen, Lit. 8]. Nicht gelesen.
- **Physics Today, April 2026:** Angster, Llorca-Bofí, Miklós, „Matching pipe organs to room acoustics“, S. 52. Allgemein, ohne Zahlen [selbst gelesen].

---

## 4. Einschwingen

- **Castellengo, M.:** „Acoustical Analysis of Initial Transients in Flute Like Instruments“, „ACUSTICA · acta acustica Vol. 85 (1999) 387–400“, „© S. Hirzel Verlag · EAA“ [wörtlich, selbst gelesen, S. 387].
  - **Neu:** Der Volltext liegt frei abrufbar auf lam.jussieu.fr. Die Rechtslage dieser Kopie ist ungeklärt. Bisher galt im Projekt „nur Bericht“.
  - Keeler: mittlere Einschwingzeit „25 to 30 periods for flutes, 50 periods for the diapason family“ [S. 387].
  - Jones 1939: Vorläufer (forerunner) etwa „five and a half time the frequency of the fundamental“ [S. 387].
  - Nach Nolle: Vorläufer und „Next Mode Burst“ sind am stärksten bei schnellem (nicht abruptem) Anstieg, „typically from 1 to 10 periods“ [S. 388]. Der volle Aufbau des Grundtons dauert etwa 10 Perioden [S. 388].
  - Fletcher teilt Anstiege in drei Klassen: „plosive“, „abrupt“, „slow“ [S. 388]; plosiv bedeutet zunächst Vorherrschaft der 2. Mode.
  - Mahu/Verge: Anstiege „very short (2ms) and very soft (20ms)“ [S. 389].
  - Zusammenfassung: Mundtöne (mouth tones) werden wie Schneidentöne gesteuert, durch Strahlgeschwindigkeit und Abstand Spalt–Labium [S. 387].
- **Castellengo, M.:** „Flute-like instrument transients: An analytical study of mouth-tone production versus pressure rise time“, ISMA 2004, Nara [wörtlich, selbst gelesen, S. 1]. Frei auf lam.jussieu.fr, kein Rechtevermerk im PDF gesehen.
  - Bis 360 Pa am Fuß dauern die drei Anstiege „160 ms, 45 ms, and 26 ms“ [S. 3].
  - Die Schwelle „(125 Pa) is crossed at 40 ms (20T), 9 ms (4T), and 4ms (2T)“ [S. 3].
  - Der kürzeste Anstieg wirkt nicht am präzisesten. Mundtöne sind unharmonisch und liegen an den Schnittpunkten der Schneidenton-Kurven mit den Rohrmoden [S. 3–4].
  - Hohe Druckanstiege, wie beim Zungenstoß, erzeugen Anteile über 4 kHz [S. 3].
- **Verge u. a.:** „Jet Formation and Jet Velocity Fluctuations in a Flue Organ Pipe“, JASA 95 (1994) 1119–1132 (HAL hal-01105552, nicht frei).
  - Abstract: „the initial impact of the jet with the labium appears to be a crucial factor“ [wörtlich, Werkzeug, HAL-API].
  - Turbulenz setzt nach etwa drei Perioden ein [nur Zusammenfassung].
- **Nolle, A. W. & Finch, T. L.:** „Starting transients of flue organ pipes in relation to pressure rise time“, J.A.S.A. 91(4) (1992) 2190–2202 [wörtlich, selbst gelesen, Castellengo 2004, Lit. 10]. Nicht gelesen.
- **Zungenstoß an der Flöte:** Im Modell ist er ein sehr steiler Druckanstieg. Dazu passen die Hysterese-Ergebnisse (Schwelle ∝ a^(2/3)) und Castellengo 2004. Eine eigene Messung zum Zungenstoß habe ich nicht gefunden.

---

## 5. Spielweisen

**Vibrato der Flöte**
- Fletcher, „Acoustical correlates of flute performance technique“, JASA (eingegangen 1974, gemeint ist wohl Fletcher 1975). Der vom Auftrag genannte Titel „Some acoustical principles of flute technique“ wurde nicht gefunden.
- Abstract-Wortlaut aus dem Suchergebnis [wörtlich, Werkzeug/Suchergebnis]:
  - „an amplitude modulation of the upper partials of the tone with little change in the fundamental“
  - „consistently about 5 Hz“
  - „associated with a 10% variation in blowing pressure“
- Rechte: ANU-Eintrag „Restricted until 2037-12-30“, Lizenzfelder leer [wörtlich, Werkzeug, 2026-10-26]. Band und Seiten sind auf der ANU-Seite leer, 57/233 also nicht bestätigt.

**Flatterzunge**
- Keine Messwerte gefunden. Es gibt nur einen Gastvortrag J. Delisle, Univ. Wien 2019, über Spektraldeskriptoren bei Vibrato und Flatterzunge [nur Zusammenfassung].
- Modulierte Klänge im jet-drive-Modell: Einhüllende „8.6 Hz“ bei der Flauta de Chinos (1402.3143) [Werkzeug/Suchergebnis]. Das ist ein Instrumenteffekt, keine Flatterzunge.

**Bend über den Ansatz**
- Querflöte: mehr Abdecken des Mundlochs macht „the note flatter“ [wörtlich, Werkzeug, UNSW].
- Die Lippe verdeckt das Mundloch, der Endeffekt wächst, die Tonhöhe sinkt [wörtlich, Werkzeug, Acoustics Today, Frühjahr 2018, S. 57]. Satzanfang: „the pitch can be lowered significantly“. Autoren nicht sichtbar.
- Shakuhachi:
  - meri um „one semitone (chu meri) or two semitones (no meri)“ [wörtlich, Werkzeug, UNSW RLsoundclips]
  - kari hebt durch weniger Abschirmung (baffling) durch das Gesicht
  - die Klangfarbe ändert sich mit dem Impedanzspektrum
- Physikalisch ändert sich vor allem die **Abstrahlungsimpedanz am Eingang, also der Resonator**, und daneben W und Versatz.

**Glissando-Kopfstück (Robert Dick)**
- Teleskop-Kopfstück: Herausziehen verlängert das Rohr [nur Zusammenfassung]. Keine Zahl zum Bend-Umfang gefunden.

**Tremulant**
- Pykett, „Tremulant Simulation in Digital Organs“, „Copyright © C E Pykett“; das Zeichen erscheint verstümmelt [wörtlich, Werkzeug, 2026-10-26].
- Werte [nur Zusammenfassung, Kurzzitate wörtlich, Werkzeug]:
  - Druckhub „peak to peak wind pressure deviation of some 75%“
  - etwa ±30 mm WS bei Kirchenorgeln, ±100 mm WS bei Kinoorgeln
  - Amplituden- und Frequenzmodulation zugleich; die Pfeifen modulieren nicht phasengleich
- Raten: etwa 6/s als „schnell“ (BYU OrganTutor), 3,8 Hz bei einer Einzelmessung (jOrgan) [nur Zusammenfassung].

**Winddruck der Orgel**
- Fest je Werk. Die Frequenz hängt linear vom Druck ab (Pykett, siehe oben).

### Abgleich mit dem Bestand der App

Der Bestand wurde nicht neu gelesen; ich stütze mich auf die Angaben im Auftrag.

- **Expression = Blasdruck:** Physikalisch hebt Druck allein die Tonhöhe leicht (Fletcher & Douglas S. 767) und führt ab etwa 300 Pa (Querflöte G4) bzw. 700 Pa (Blockflöte) ins nächste Register. Querflötisten gleichen das mit W und Abdecken aus.
  - Eine Expression, die nur den Druck steuert, ergibt bei der Querflöte Tonhöhendrift und ungewolltes Überblasen.
  - Bei der Blockflöte entspricht diese Drift der Praxis.
- **Vibrato:** Bei der Flöte ist Vibrato überwiegend eine Modulation des Drucks und der Amplitude, nicht der Tonhöhe (Fletcher). Ist „Vibrato“ in der App als Tonhöhen-Modulation umgesetzt, widerspricht das der Messung.
- **Pfeifenorgel:** Der Spieler hat keine dauernde Anregung je Ton. Expression, Vibrato und Pitch passen nicht zur Labialpfeife. Ein Tremulant (Windmodulation des ganzen Werks) und der Schwellkasten sind das Gegenstück.
- **Bend:** Bei Querflöte und Shakuhachi ändert er den Resonator (Abdecken, meri/kari), nicht nur den Anreger. Das muss in „Spur und Kurve“ eines vorab gerechneten Tons Platz haben.

---

## 6. Rechenzeit

- Keine neuen Zahlen gefunden.
- In 1601.05545, Tab. 1, steht laut Werkzeug als Abtastrate „Sr = 0.1 GHz“. Das ist vermutlich verstümmelt; **ungeprüft**.
- TubeLab (arXiv 2609.22468, Sept. 2026): Transfermatrix-Methode im Browser für Bohrungen „without tone holes“, keine Zeitangaben, nicht flötenspezifisch [wörtlich, Werkzeug].

---

## Nicht gefunden

- Pegel des Anblasgeräusches relativ zum Ton in dB.
- Die Strouhal-Zahl des Rausch-Maximums als Zahl.
- Messwerte zur Flatterzunge (Rate, Tiefe).
- Fletcher 1975 im Volltext (Werte zu Druck je Oktave und zur Strahllänge).
- Der Titel „Some acoustical principles of flute technique“.
- Ein frei lizenzierter Datensatz zur Böhm-Flöte oder eine frei lizenzierte Blockflöten-Bauzeichnung.
- Bohrungsdaten der Shakuhachi mit klarer Lizenz.
- Eine Yoshikawa-Arbeit zu meri/kari.
- Zahlen zu Aufschnitt, Kernspalt und Winddruck aus einer Norm- oder Lehrbuchquelle (nur Einzelpfeifen).
- Der Bend-Umfang des Glissando-Kopfstücks.
- Eine OpenWInD-Version mit Flöte.
- Rechenzeiten.
- Rucz/Angster/Miklós zum Einschwingen und Ausklang.

Kein Treffer ist kein Beweis; die HAL- und Acta-Volltexte waren gesperrt.

---

## Eigene Einschätzung (kein Beleg)

**Was das Modell braucht und woher**
- Je Instrument: h, W, y0, H (Fenster), b = 2h/5, c_p ≈ 0.4 U_j, α_i ≈ 0.3–0.4/h, α_vc ≈ 0.6, δ_d, dazu das Mundvolumen.
- Startwerte liegen in arXiv 1402.3143 (Querflöte) und 1601.05545 (Blockflöte); Bolton liefert die Kanalmaße der Blockflöte.
- Die Moden kommen aus dem vorhandenen Rohr-Kern.
- Vom Spieler: Druckverlauf (Expression). Bei der Querflöte zusätzlich W(t), y0(t) und Abdecken. Sie sollten aus der Note abgeleitet werden: W etwa so mitführen, dass die Laufzeit zur Periode passt (Fletchers halbe Wellenlänge).

**Trägt der modale Kern?**
- Ja. Die maßgeblichen Zeitbereichs-Arbeiten (Terrien, Auvray) nutzen selbst einen Resonator aus 2–3 Moden mit verzögertem, tanh-gesättigtem Strahl.
- Zwei Lücken:
  - (a) Mundtöne und Vorläufer im Einschwingen hängen am Schneidenton und an Quermoden. Der modale Rohr-Kern deckt sie nicht ab; sie brauchen einen eigenen Pfad (Schneidenton-Oszillator plus geformtes Rauschen).
  - (b) Das Abdecken ändert die Admittanz am Mundloch, also den Resonator. Bei Bend und meri/kari müssen die Moden je Ton mit der Kurve mitlaufen.

**Aufwand und Risiko je Instrument**
- **Blockflöte:** geringstes Risiko. Feste Geometrie, gute Parameter in 1601.05545, Spielhilfe nur über den Druck.
- **Orgel-Labialpfeife:** geringes bis mittleres Risiko.
  - Einfache Anregung: fester Druck, gerechnet einmal je Pfeife.
  - Das Einschwingen (Castellengo, Nolle & Finch) ist der Klangkern und schwer zu treffen.
  - Mensuren über die Normmensur (CC BY-SA, zu prüfen), aber kaum frei lizenzierte Mund- und Kernspaltmaße.
  - Viele Pfeifen bedeuten Masse an Vorabrechnung.
- **Querflöte:** mittleres Risiko. Zum Modell kommt die Regelung W/Abdecken/Versatz des Spielers; ohne sie driften Tonhöhe und Register. Die Bohrung ist bekannt (UNSW, ohne Lizenz).
- **Altflöte:** wie die Querflöte, dazu fehlen Daten (keine gefunden). Wahrscheinlich skaliert aus der Querflöte, unsicher.
- **Shakuhachi:** höchstes Risiko.
  - Offene Kante ohne Kernspalt.
  - meri/kari verändert den Resonator.
  - Starkes Atemgeräusch um 9 kHz und Quermoden gehören zum Stil.
  - Keine lizenzierte Bohrung.

**Bestand der App:** Für Flöten sollte „Vibrato“ auf den Anreger (Druck) wirken, nicht auf die Tonhöhe. Für Labialpfeifen sollten Expression, Vibrato und Pitch durch Tremulant und Werk-Winddruck ersetzt werden. Das ist ein Vorschlag zur Prüfung, kein Befund.

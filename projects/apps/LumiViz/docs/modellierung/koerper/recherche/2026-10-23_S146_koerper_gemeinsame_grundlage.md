# Recherche S146: eine gemeinsame Beschreibung von Instrumentenkörpern

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** Gibt es eine einheitliche Beschreibung von Instrumentenkörpern
> über Instrumente hinweg (Moden mit Frequenz, Dämpfung, Masse am Einkoppelpunkt, Abstrahlgewicht), und welche
> Datensätze liefern Werte dafür; wie trennen Arbeiten Rückwirkung auf die Saite und Abstrahlung.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = im Volltext gesehen (nur zwei PDFs), [nur Zusammenfassung] = sonst.
> **Von Claude selbst nachgelesen:** nichts davon (Grundlage für KB1 a, dort als „nur Bericht“ geführt).
> **Quellen-PDFs:** `../quellen/` (nicht versioniert).
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund Recherche Boden und Körper“, KB1 a.

---

## Recherche: instrumentübergreifende Körperbeschreibung und Datensätze

**Reichweite:** etwa 14 Websuchen und Abrufe am 23.10.2026. Volltext habe ich nur bei zwei PDFs gelesen (pdftotext): Rau/Smith/Abel FA2023 und Ducceschi et al. DAFx26. Alles andere stammt aus Zusammenfassungen von Suchwerkzeugen und ist deshalb **kein Beleg**.

### Funde

**1. Ducceschi, Russo, Webb: „Measurement-Informed Nonlinear Modal Synthesis of 65 Classical Guitars“ (DAFx26)**
https://dafx.de/paper-archive/details/KZ0Ws85vmVK5-Tceo9RZ1w (PDF: dafx.de/paper-archive/2026/papers/DAFx26_paper_40.pdf, CC BY 4.0)
- Diese Arbeit trennt Impedanzwirkung und Abstrahlung genau so, wie Ihr es sucht. **[wörtlich]** „the driving-point compliance of the guitar body is represented by a sum of J modal contributions“. Die Formel: H_b(ω)=Σ_j φ_j/(ω_j²−ω²+2iσ_jω), mit Modalmassen m_j. Die Saite wird über die Bedingung u(x_b)=u_b angekoppelt.
- Für die Abstrahlung **[wörtlich]**: „modelled here as a modal reverberator whose frequency response takes the pole-residue form“, mit reellen Residuen b_l. Bass- und Diskantseite des Stegs werden getrennt angepasst, und **[wörtlich]** „Each complex-conjugate pole pair is realised in the time domain as a parallel second-order IIR section [24]“. [24] ist Bank, ICMC 2007.
- Umfang **[wörtlich]**: „retaining 50–200 bridge modes and 80–300 radiation poles per side“. Angepasst wurde über 20–10000 Hz (Steg) bzw. 20–12000 Hz (Abstrahlung).
- Wichtig: Steg-Moden und Abstrahl-Pole sind **zwei getrennte Polsätze**, kein gemeinsamer Modensatz.
- Code und Klänge: https://github.com/Nemus-Project/65_modelled_guitars. **[nur Zusammenfassung]** Dort liegen nur Klangbeispiele, keine Modaldateien und keine Lizenzangabe.

**2. Mores-Archiv, 65 Gitarren (1803–2018), Datensatz**
https://zenodo.org/records/4604576 (in der Literatur auch …/4604577)
- Lizenz laut Zenodo-API **[wörtlich]**: `{"id": "cc-by-4.0"}`.
- **[nur Zusammenfassung]** Die Steg-Mobilität wurde mit Hammer und Beschleunigungsaufnehmer gemessen, dazu drei Kanäle Nahfeld-Abstrahlung. Die Rohdaten liegen in `qualified_selected_impulses.mat` (32,7 MB), dazu MATLAB-Skripte und Fotos (gesamt 1,1 GB).
- Diese Quelle eignet sich am besten, um Gitarrenkörper (Admittanz und Abstrahlung) mit freien Daten zu füllen.

**3. Rau, Smith, Abel (CCRMA), FA2023: Vergleich von Modal-Fit-Verfahren**
- **[wörtlich]** Sie verwenden die Darstellung „the response of a set of M damped simple harmonic oscillators“, also h(t)=Σ a_m e^{2πf_m t(i−ζ_m)} mit Frequenz, Dämpfungsgrad und Amplitude.
- Gemessen wurden 10 Gitarren, 5 Violinen, Viola, 2 Celli, Kontrabass, Mandoline und 20 Deckenhölzer, insgesamt 850 Messungen.
- Daten: https://ccrma.stanford.edu/~mrau/FA2023/ (ZIP, 2,05 GB). **[nur Zusammenfassung]** Die Seite nennt keine Lizenz, also müsste man vor einer Nutzung nachfragen.
- Das Paper zitiert außerdem Maestre/Scavone/Smith, IEEE/ACM TASLP 25(5), 2017: „Joint modeling of bridge admittance and body radiativity …“.

**4. Maestre, Scavone, Smith, WASPAA 2015 und TASLP 2017**
- **[nur Zusammenfassung]** Sie schätzen modale Parameter aus gemessener Steg-Treibpunktadmittanz und Abstrahlprofilen gemeinsam ab, durch Optimierung gegen beide. Daraus entstehen Filter für Reflexion (Saite) und Abstrahlung im Wellenleiter.
- Dieser Ansatz passt am direktesten zu „ein Modensatz, zwei Ausgänge“. Den Volltext habe ich nicht gelesen; das UPF-Repositorium lieferte „Access Denied“.

**5. Wright, Dissertation (Cambridge, 1996), Gitarre**
https://hakwright.co.uk/thesis.html
- **[wörtlich]** „each mode being characterised by four parameters: a resonance frequency, an effective mass, an effective monopole area and a Q-value“.
- Das ist die kompakteste physikalische Vier-Parameter-Form je Mode. Sie enthält Einkopplung (Masse) und Abstrahlung (Monopolfläche) zugleich.
- Lizenz **[wörtlich]**: „All rights reserved“. Die Werte darf man zitieren und nachbauen, die Datei nicht weitergeben.

**6. Bank: parallele Filter 2. Ordnung für Instrumentenkörper (ICMC 2007), MATLAB-Code**
https://home.mit.bme.hu/~bank/parfilt
- **[nur Zusammenfassung]** Die Pole sind fest vorgegeben (z. B. logarithmisch verteilt), nur die Nullstellen werden angepasst. Das ist ein Werkzeug, um die Abstrahlseite aus einer gemessenen Körper-IR in Moden zu überführen.
- Bank, Zambon, Fontana 2010 (TASLP 18(4)) **[nur Zusammenfassung]**: Für den Resonanzboden diskutieren sie FFT-Faltung gegenüber parallelen Filtern 2. Ordnung.
- Wie diese Arbeit Stegadmittanz und Abstrahlung trennt, habe ich **nicht am Text geprüft**.

**7. Ege, Boutillon: „Synthetic description of the piano soundboard mechanical mobility“**
https://arxiv.org/abs/1210.5688
- **[nur Zusammenfassung]** Sie geben eine Mobilität mit wenigen Parametern an. Unter etwa 1,1 kHz verhält sich der Resonanzboden **[wörtlich]** wie „a homogeneous plate with isotropic properties and clamped boundary conditions“, darüber sind die Wellen zwischen den Rippen eingeschlossen.
- Daraus folgt: Für den Flügel genügt oberhalb einer Grenze eine **statistische** Beschreibung (Modendichte, charakteristische Impedanz) statt einzelner Moden. Ergänzend: arXiv 1305.3057 und 1210.3948 (Boutillon, Ege, Paulello).

**8. Bissinger, VIOCADEAS (Violine)**
- **[nur Zusammenfassung]** Laut https://strad3d.org/bios.html wurden 14 bewertete Violinen und ein Oktett erfasst. Das Projekt war eine modale Mechanik- und Abstrahl-Datenbank.
- Eine **öffentlich herunterladbare** Fassung habe ich nicht gefunden.

**9. CNSM-Datensatz (Violine, Einspielen)**
https://zenodo.org/records/18696785
- Lizenz CC BY 4.0, eine Datei `dataset.zip` (654 MB).
- **[nur Zusammenfassung]** Laut arXiv 2605.18305 enthält er Stegadmittanz, MoCap und Audio. Den Inhalt habe ich nicht geprüft.

**10. Austauschformat SDIF „1RES“**
https://cnmat.org/SDIF/FrameTypes.html
- **[nur Zusammenfassung]** Spalten: Frequenz, Amplitude, Abklingrate, Phase. Der Typ ist ausdrücklich nicht final standardisiert.
- Er kennt nur einen Ausgang, also weder Masse noch Treibpunkt. Für Euren Zweck reicht er nicht.

### Nicht gefunden
- Ein veröffentlichtes, **instrumentübergreifendes einheitliches Körperformat** oder Framework als Standard. Alle Arbeiten verwenden dieselbe Mathematik (Summe gedämpfter Oszillatoren bzw. Pol-Residuen), aber jeweils ein eigenes Format.
- Offene Moden- oder Admittanzdaten für Pauke, Trommelkessel oder Klavier-Resonanzboden. Zu Pauke und Snare gibt es nur Arbeiten von Rossing/Zhao und Abschlussarbeiten, nicht frei verfügbar. SDDS (BCU) ist nach Snippet eine Audiosammlung.
- Den Volltext von Bank 2010 und Maestre 2017.

### Eigene Einschätzung (kein Beleg)
Vorschlag „Körper“ (JSON), getragen von der Literatur (1, 4, 5):
- **Moden** je Eintrag: `f` [Hz], `zeta` oder `Q`, `masse` [kg] effektiv am Treibpunkt (je Treibpunkt eine Liste, bei der Gitarre Bass- und Diskantseite), `abstrahlung` als Residuum je Ausgang, wahlweise physikalisch als Monopolfläche nach Wright. Dazu optional `richtung` bzw. Stereo-Gewichte.
- **Getrennt erlaubt:** ein eigener Polsatz nur für die Abstrahlung, weil Messfits zwei verschiedene Sätze liefern (Fund 1). Dazu ein Feld `herkunft` mit Quelle, Fit-Verfahren und Frequenzbereich.
- **Oberhalb `f_grenze` statistisch:** Modendichte, mittlere Admittanz bzw. charakteristische Impedanz und Dämpfungsgesetz. Damit bedient man den Flügel nach Ege/Boutillon und die hohen Lagen der Gitarre. Die Dichte wird bei der Synthese zu Zufallsmoden ausgewürfelt.

Füllbar wären die Instrumente so:
- **Gitarre:** Mores (CC BY, Rohdaten → Fit nach Fund 1 bzw. 6), Wright (Vier-Parameter-Werte, zitierbar).
- **Violinfamilie:** CNSM (CC BY, Inhalt prüfen), Rau FA2023 (Lizenz klären).
- **Flügel:** die Parameter von Ege/Boutillon für den statistischen Teil, die tiefen Moden als Plattenmoden.
- **Trommeln:** vorerst nur analytisch, Membran mit Luftlast nach Rossing; keine offenen Messdaten gefunden.

# Recherche IC1: Z1 — Moden und Waveguides kombiniert, Rohr (Klassen 7, 8, 9)

> **Auftrag:** Zusatzfrage Z1 aus LumiViz (`AUFGABE.md`): welche Verfahren Moden und digitale Wellenleiter beim Rohr
> verbinden, was je Weg geht, wie sie an Messungen gesetzt werden, was sie je Klasse 7/8/9 bringen.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [W-s, S. x] = vom Agenten per `pdftotext` gelesen; [W-w] = vom Abrufwerkzeug zitiert; [Z] = nur
> Zusammenfassung; [E] = Einschätzung.
> **Reichweite:** im Bericht selbst (20 Suchen, 32 Abrufe; Acta Acustica, DOAJ, HAL-Volltext Taillard 2018, Smith 1986,
> Msallam 2000 nicht erreicht).
> **Anmerkung der Sitzung:** Der Agent hat PDFs aus dem Cache mit `pdftotext` in Textdateien im Scratchpad umgewandelt
> (nicht im Projekt; Dateien im Bericht genannt). Die Befunde je Klasse stehen in
> `klassen/lippe-rohr/recherche/2026-10-26_moden_und_waveguides.md` und den gleichnamigen Dateien der Klassen 8 und 9.
> **Von der Sitzung selbst nachgelesen:** siehe dort.

---

# Z1, Rohr (Klassen 7, 8, 9): Moden und Waveguides kombiniert

**Auftrag:** Wie Verfahren der Moden (modal) und der digitalen Wellenleiter (digital waveguide, DWG) beim Rohr zusammenwirken, was sie leisten, wie man sie an Messungen setzt, und was sie je Klasse 7/8/9 bringen.

**Datum:** 2026-10-26

**Reichweite:**
- **Abrufe:** 20 Websuchen und 32 Abrufe, also mehr als die geplanten 30–45.
- **Orte:** dafx.de, CCRMA (~jos/pasp, ~gary), caml.music.mcgill.ca, ISMA-2017-Tagungsband, vbn.aau.dk, pure.ed.ac.uk, dael.euracoustics.org, arXiv, HAL-API, Crossref-API, openwind.inria.fr.
- **Sprachen:** Englisch, dazu französische HAL-Einträge.
- **Nicht erreichbar:**
  - acta-acustica.edpsciences.org: 403
  - doaj.org: 403
  - hal.science-Volltext von Taillard 2018: Anubis-Sperre
  - Scavones Dissertation: nur über ftp verlinkt, nicht abgerufen
  - Smith 1986 (ICMC): kein Volltext gefunden
  - Msallam et al. 2000: weder Volltext noch Abstract
- **Vom Abrufwerkzeug zwischengespeicherte PDFs** (nicht von mir heruntergeladen), alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793040314803-f2yebi.pdf`: Maestre/Scavone/Smith, DAFx-18
  - `webfetch-1793040790731-iuec9j.pdf`: Maestre/Smith/Scavone, ISMA 2017
  - `webfetch-1793040425410-yku589.pdf`: Scavone/Smith, ICMC 1997
  - `webfetch-1793040605748-xp1r73.pdf`: Scavone, SMAC 2003
  - `webfetch-1793040493184-pxzwue.pdf`: Mignot/Hélie/Matignon, DAFx-09
  - `webfetch-1793040517843-xx8udh.pdf`: Willemsen et al., DAFx20in21
  - `webfetch-1793040676327-6vrjx5.pdf`: Harrison et al., CMJ
  - `webfetch-1793040739943-uo72qk.pdf`: Cooper/Abel, DAFx-10
  - `webfetch-1793040676184-fnxyya.pdf`: Petiot et al., Forum Acusticum 2023
- **Abweichung vom Auftrag „nichts in Dateien schreiben":** Um diese PDFs selbst zu lesen, habe ich sie mit `pdftotext` in Textdateien im Scratchpad umgewandelt, nicht ins Projekt. Ordner `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\scratchpad\`, Dateien `maestre18.txt`, `isma17_61.txt`, `icmc97.txt`, `smac03.txt`, `mignot09.txt`, `willemsen21.txt`, `harrison15.txt`, `cooper10.txt`, `fa23_199.txt`. Im Projekt habe ich nichts geändert.

**Kennzeichnung der Angaben:**
- **[W-s, S. x]**: wörtlich, selbst gelesen (pdftotext der zwischengespeicherten PDFs)
- **[W-w]**: wörtlich, vom Werkzeug zitiert
- **[Z]**: nur Zusammenfassung, kein Beleg
- **[E]**: Einschätzung

**Korrektur am Auftrag:** Die IEEE-Arbeit „Digital Waveguide Modeling for Wind Instruments: Building a State Space Representation Based on the Webster Lokshin Model" stammt von **Mignot, Hélie und Matignon** (nicht Roze). Sie erschien in IEEE Trans. Audio, Speech, Lang. Process. 18(4):843–854, 2010. Beleg ist das Literaturverzeichnis von Harrison et al. [W-s, Manuskript-Literaturliste]. Die Arbeit selbst habe ich nicht gelesen.

---

## 1. Quellen

1. **Maestre, Scavone, Smith (2018).** „Joint Modeling of Impedance and Radiation as a Recursive Parallel Filter Structure for Efficient Synthesis of Wind Instrument Sound". Proc. DAFx-18, Aveiro, S. DAFx-157–164.
   - Fundort: dafx.de/paper-archive/2018/papers/DAFx2018_paper_29.pdf, frei.
   - Rechtsvermerk: Im Text keiner gefunden (Suche nach „copyright|licen|creative", Abruf 2026-10-26).
2. **Maestre, Smith, Scavone (2017).** „Analysis-Synthesis of Saxophone Input Impedances via Recursive Parallel Filters". Proc. ISMA 2017, Montreal, „Peer Reviewed Paper".
   - Fundort: isma2017.cirmmt.mcgill.ca, frei.
   - Rechtsvermerk: keiner gefunden (2026-10-26).
3. **Scavone, Smith (1997).** „Digital Waveguide Modeling of Woodwind Toneholes". ICMC 1997, Thessaloniki.
   - Fundort: ccrma.stanford.edu/~gary/papers/icmc97.pdf, frei.
   - Rechtsvermerk: keiner; nur der Schrift-Vermerk der AMS (2026-10-26).
4. **Scavone (1997).** „An Acoustic Analysis of Single-Reed Woodwind Instruments with an Emphasis on Design and Performance Issues and Digital Waveguide Modeling Techniques". Diss., Stanford.
   - Fundort: ccrma.stanford.edu/~gary/thesis.html. Gelesen habe ich nur die Seite mit dem Abstract; dort laut Werkzeug kein Vermerk.
5. **Scavone (2003).** „Modeling Vocal-Tract Influence in Reed Wind Instruments". Proc. Stockholm Music Acoustics Conference 2003.
   - Fundort: caml.music.mcgill.ca/~gary/papers/smac03.pdf, frei.
   - Rechtsvermerk: keiner gefunden.
6. **Mignot, Hélie, Matignon (2009).** „State-Space Representations for Digital Waveguide Networks of Lossy Flared Acoustic Pipes". Proc. DAFx-09, Como.
   - Fundort: dafx.de/paper-archive/2009/papers/paper_17.pdf, frei.
   - Rechtsvermerk: keiner gefunden.
7. **Mignot, Hélie, Matignon (2010).** IEEE TASLP 18(4), siehe Korrektur oben. Nicht gelesen.
8. **Willemsen, Bilbao, Ducceschi, Serafin (2021).** „A Physical Model of the Trombone using Dynamic Grids for Finite-Difference Schemes". Proc. DAFx20in21, Bd. 2, S. 152–159.
   - Fundort: vbn.aau.dk, frei.
   - Lizenz [W-s, S. 152]: „distributed under the terms of the Creative Commons Attribution 3.0 Unported License". Repository: „Creative Commons License CC BY 3.0" (2026-10-26).
9. **Harrison, Bilbao, Perry, Wishart.** „An Articulated Brass Instrument Synthesis Environment" (Manuskripttitel). Computer Music Journal 39(5):80–95, DOI 10.1162/COMJ_a_00332.
   - Das Repository nennt das Jahr 2016, das Manuskript 2015.
   - Fundort: pure.ed.ac.uk, Manuskriptfassung.
   - Rechte [W-s, Deckblatt]: Copyright „is retained by the author(s) and / or other copyright owners" (2026-10-26).
10. **Cooper, Abel (2010).** „Digital Simulation of 'Brassiness' and Amplitude-Dependent Propagation Speed in Wind Instruments". Proc. DAFx-10, Graz.
    - Fundort: dafx.de, frei.
    - Rechtsvermerk: keiner gefunden.
11. **Petiot, Roatta, Fréour, Arimoto (2023).** „Contribution of Machine Learning and Physics-Based Sound Simulations for the Characterization of Brass Instruments". Forum Acusticum 2023, Turin, S. 375ff., DOI 10.61782/fa.2023.0199.
    - Fundort: dael.euracoustics.org.
    - Lizenz [W-s, S. 375]: „Creative Commons Attribution 3.0 Unported License".
12. **Silva, Debut, Kergomard, Vergez, Deblevid, Guillemain (2007).** „Simulation of Single Reed Instruments Oscillations Based on Modal Decomposition of Bore and Reed Dynamics". arXiv:0705.2803.
    - Lizenz: nur der arXiv-Link „view license" [W-w].
13. **Szwarcberg, Colinot, Vergez, Jousserand (2023).** „Amplitude-dependent modal coefficients accounting for localized nonlinear losses in a time-domain integration of woodwind model". Acta Acustica 2023, arXiv:2311.02081.
    - Lizenz: arXiv non-exclusive distribution license 1.0 [W-w].
14. **Szwarcberg et al. (2023/2024).** „Modal decomposition synthesis for localized nonlinear losses at termination of a woodwind model …". Forum Acusticum 2023, arXiv:2401.05751.
    - Lizenz: arXiv-Lizenz [W-w].
15. **Taillard, Silva, Guillemain, Kergomard (2018).** „Modal analysis of the input impedance of wind instruments. Application to the sound synthesis of a clarinet". Applied Acoustics 141:271–280, DOI 10.1016/j.apacoust.2018.07.018.
    - HAL hal-01853608, Lizenzfeld „https://about.hal.science/hal-authorisation-v1/" [W-w, HAL-API]; Crossref: Elsevier TDM license.
    - Volltext gesperrt (Anubis).
16. **Ernoult, Chabassier, Rodriguez, Humeau (2021).** „Full waveform inversion for bore reconstruction of woodwind-like instruments". Acta Acustica 5, 47, DOI 10.1051/aacus/2021038.
    - HAL hal-03231946, Lizenzfeld hal-authorisation-v1 [W-w].
    - Dass der Artikel unter CC BY 4.0 steht, sagt nur die Suchzusammenfassung [Z]; Journal und DOAJ antworteten mit 403.
17. **Guillemain, Kergomard, Voinier (2005).** „Real-time synthesis of clarinet-like instruments using digital impedance models". JASA, DOI 10.1121/1.1937507 [W-w, Crossref]. Nicht frei.
18. **McIntyre, Schumacher, Woodhouse (1983).** „On the oscillations of musical instruments". JASA, DOI 10.1121/1.390157 [W-w, Crossref]. Nicht frei.
19. **Berjamin, Lombard, Vergez, Cottanceau.** „Time-domain numerical modeling of brass instruments including nonlinear wave propagation, viscothermal losses, and lips vibration". Acta Acustica united with Acustica, DOI 10.3813/AAA.919038, arXiv:1511.04247.
    - Jahr unklar: Das Werkzeug nennt 2015, die Suche 2017.
    - Lizenz: arXiv-Lizenz [W-w].
20. **Thibault, Chabassier (2020).** „Time-domain simulation of a dissipative reed instrument". HAL hal-03132474.
    **Thibault (2019).** „Wind Instrument Sound Synthesis through Physical Modeling". HAL hal-03230082.
    - Lizenzfeld jeweils hal-authorisation-v1 [W-w].
21. **OpenWInD**, openwind.inria.fr: „available on gitlab under GPL-v3.0 license" [W-w]. Fußzeile „Copyright © Openwind, 2019 – 2025" [W-w].
22. **Rozzi, Ferroni (2026).** „TubeLab: Interactive inverse design of wind instrument bores with hard spectral constraints". arXiv:2609.22468, eingereicht 2026-09-18.
    - Lizenz: arXiv-Lizenz 1.0 [W-w].
23. **Smith, „Physical Audio Signal Processing"**, Seiten „Brasses" und „Bell_Models", ccrma.stanford.edu/~jos/pasp.
    - Vermerk „Copyright © 2026-08-21 by Julius O. Smith III" [W-w].
24. **Nur als Zitat in gelesenen Arbeiten, selbst nicht gelesen:**
    - Msallam, Dequidt, Caussé, Tassart (2000), Acta Acustica united with Acustica 86(4):725–736. Zitiert bei Cooper/Abel [17] und Willemsen [4].
    - Gazengel, Gilbert, Amir (1995), „From the measured input impedance to the synthesis signal: where are the traps?", Acta Acustica 3:445–472. Zitiert bei ISMA-17 [2].
    - Bilbao, Chick (2013), JASA 134(6):3860–3871.
    - Rodet, Vergez (2000), J. Signal Processing 4(1):79–87.
    - Cook (1991), „Tbone", ICMC.
    - Smith (1986), ICMC S. 275–280 [Z].

---

## 2. Verfahren, die Moden und Waveguides beim Rohr verbinden (Frage 1)

**A. Klassischer Waveguide**
- Aufbau: Zwei Verzögerungsleitungen (delay lines), ein Endfilter für Verlust und Schallstück, das Blatt als nichtlineares Streuglied (nonlinear scattering).
- Maestre et al. beschreiben ihn als üblichen Ansatz für zylindrisch angenäherte Luftsäulen [W-s, DAFx-157].
- Kegelstücke sind möglich, ergeben aber laut derselben Arbeit Resonanzen, „difficult to tune for sound synthesis" [W-s, DAFx-157].
- Das Schallstück (bell) ist kein dünn besetzter Waveguide. Smith: „cannot be accurately modeled as a sparse digital waveguide" [W-w, PASP Bell_Models]. Er empfiehlt abgeschnittene IIR-Filter (truncated IIR, TIIR), angepasst an Reflektometrie-Daten [W-w].

**B. Waveguide mit Tonlöchern als Streuglieder (Scavone/Smith 1997)**
- Aus den Transfermatrix-Parametern nach Keefe werden Streuparameter des Waveguides [W-s, Abstract].
- Zweitor (two-port) mit Serien- und Querimpedanz; Kosten: „four second-order filtering operations per tonehole" [W-s, Abschn. 3].
- Alternative ist ein Dreitor (three-port) mit „one multiply and one filtering operation" [W-s, Abschn. 3]. Er vernachlässigt die Serienterme, die laut Text „much less critical" sind [W-s, Abschn. 2].
- Die Dissertation (1997) behandelt außerdem konische Luftsäulen, Registerlöcher und den Vokaltrakt [W-w, Abstract-Seite].

**C. Waveguide-Netz nach Webster-Lokshin in Zustandsraumform (Mignot/Hélie/Matignon 2009/2010)**
- Ein Netz vom Kelly-Lochbaum-Typ mit verlustbehafteten, aufgeweiteten Stücken (flared pipes), in Diagonalform. Die Ordnung wird mit Mitteln der Regelungstechnik reduziert [W-s, DAFX-1 und DAFX-3].
- An einer echten Posaune gezeigt: 11 Stücke gegenüber 5. Nur das Modell mit 11 Stücken trägt die Unregelmäßigkeiten der gemessenen Hüllkurve [W-s, DAFX-4].
- Grenze laut Autoren: „only linear resonators with static parameters have been presented" [W-s, DAFX-5]. Zug, Ventile, Lippen und Tonlöcher stehen als offen.

**D. Reflexionsfunktion statt Impedanz**
- Grundlage ist McIntyre/Schumacher/Woodhouse 1983, die Zerlegung in Quelle und Resonator; bei Harrison als „canonical description" bezeichnet [W-s, Manuskript S. 2].
- Vergez/Rodet: Trompetenrohr als Reflexionsfunktion aus der gemessenen Impedanz [Z].
- Bei der inversen Fourier-Transformation einer Impedanzmessung entsteht zeitliche Faltung (time-aliasing); Maestre verweist dafür auf Gazengel/Gilbert/Amir 1995 [W-s, ISMA-17, Einleitung].

**E. Moden der Impedanz, im Zeitbereich integriert**
- Silva et al. 2007: „the input impedance of the bore is projected on the modes" [W-w].
- Taillard/Silva/Guillemain/Kergomard 2018: Prony/LSCE an gemessenen Impedanzen. Fiktive Punkte bei hohen Frequenzen sollen „ensure the passivity of the resonator in the whole frequency band" [W-w, HAL-Abstract]. Sie liefern „a real-time synthesis of clarinet sounds" und eine Behandlung der Moden „during the transition between fingerings" [W-w].
- Szwarcberg et al. 2023: Pole und Residuen werden abhängig von der Schnelle interpoliert, so gehen örtliche nichtlineare Verluste am Ende ein [W-w].
- Petiot et al. 2023, Trompete: Die Impedanz wird durch 11 Moden mit je 4 Parametern beschrieben, zusammen 44 [W-s, S. 377].

**F. Mischform: modale Reflektanz als Waveguide-Abschluss (Maestre/Scavone/Smith 2017/2018)**
- Wichtig für den Projektvorschlag: Maestre 2018 ist **schon eine Mischform**. Die Moden der Impedanz werden als Reflektanz R(z) realisiert und über Wanderwellen P⁺/P⁻ an ein Blatt als nichtlineares Streuglied gekoppelt [W-s, DAFx-157].
- Begründung gegen die rein modale Kopplung: Deren implizite Verfahren „may cause numerical dispersion and require high computational cost" [W-s, DAFx-157].
- Abstrahlung über dieselbe Resonatorbank, „at a very low additional cost" [W-s, DAFx-160].
- Griffwechsel als lineare Mischung Z = Σ w_f Z_f. Passiv, wenn alle Gewichte nichtnegativ sind; quasistatisch gedacht [W-s, DAFx-162].
- „average of 32 resonators", 48 kHz, „above 30 times faster than real-time" [W-s, DAFx-163].
- Offen laut Autoren [W-s, DAFx-163]:
  - Wie viele Resonatoren nötig sind
  - Ob die Überblendung den echten Übergang trifft
  - Lippenanregung
  - Ein Vokaltrakt als Reflektanz

**G. Vokaltrakt-Kopplung im Waveguide (Scavone 2003)**
- Vor dem Blatt sitzt eine Resonanz, umgesetzt als Streuglied: „converted to a travelingwave scattering junction" [W-s, Abschn. Single resonance].
- Gekoppelt mit einem Waveguide-Saxophon „closely parallel" zum echten Spiel [W-s, Ergebnisabschnitt].

**H. Längenänderung beim Zug im Waveguide**
- Willemsen et al.: Waveguides erlauben die Änderung „simply by varying the length of the corresponding delay line". Das gilt aber nur für „simplified bore profiles" [W-s, S. 152].
- Harrison et al.: Ventile durch Kürzen der Leitung „does not include the effects from other interacting tube sections" [W-s, Einleitung].
- Eine veröffentlichte Waveguide-Posaune mit zeitveränderlicher Bruchteil-Verzögerung (fractional delay) habe ich nicht gefunden. Willemsen nennt Cook 2002 als Echtzeit-Beispiel [W-s, S. 152, Lit. 5].

**I. Schmettern (brassiness) im Waveguide**
- Cooper/Abel 2010: ein beidseitiger Waveguide (bidirectional waveguide) mit druckabhängiger Verzögerung; die Summe beider Wanderwellen steuert die Verzögerung [W-s, DAFX-4]. Dazu ein vereinfachter Prozessor (lumped processor), „suitable for real-time applications" [W-s, DAFX-5].
- Für den Vorschlag „Stufe am Ausgang" zentral: Nach Cooper/Abel zeigte Msallam 2000, dass der Rückwärtsanteil in der Schleife „does affect the fundamental frequency and increase the brassiness" [W-s, DAFX-4, Zitat über Msallam].
- Berjamin et al.: nichtlineare Ausbreitung mit einem TVD-Verfahren, Verluste über Gedächtnisvariablen [W-w].

**J. FDTD als Gegenpol**
- Harrison et al. zum modalen Weg: Die Moden müssen „computed beforehand" werden. Komplexe zeitliche Änderung des Rohrs, etwa beim Ventilspiel, „can become awkward" [W-s, Einleitung].
- FDTD für Trompeten „real-time on modern laptops" [W-s, Schluss].
- Willemsen: dynamisches Gitter für den Zug, „no audible artifacts" auch bei schnellem Zug [W-s, Schluss].
- Thibault/Chabassier: energieerhaltende Verfahren, Parameter „permitted to vary continuously", auch Öffnen und Schließen von Tonlöchern [W-w]. OpenWInD (GPL-v3) rechnet Impedanz und Zeitbereich; die Flöten-Anblasung ist „coming soon" [W-w].

---

## 3. Tabelle: Verfahren × Eigenschaften (Fragen 2 und 3)

| Verfahren | Steuerung je Resonanz | Obergrenze | Verlust (frequenzabhängig) | Tonlöcher und Griffwechsel | Nichtlineares | Zeitveränderliches | Rechenzeit | Setzen an Messung |
|---|---|---|---|---|---|---|---|---|
| A Waveguide zyl./kon. | nein, Resonanzen folgen aus Länge und Filter [E] | keine, Reihe geht bis Nyquist [E] | gebündelt im Endfilter [W-s, M18 DAFx-157] | über B | Blatt als Streuglied [W-s, M18 DAFx-157]; Schmettern über I | Länge = Verzögerung, nur einfache Profile [W-s, Willemsen S. 152] | sehr gering [E] | Schallstück-Filter an Reflektometrie (Steiglitz-McBride) [W-w, PASP]; schwer stimmbar bei Kegeln [W-s, M18] |
| B Tonloch-Streuglieder | nein [E] | keine [E] | Loch-Reflektanzen als Filter [W-s, Sc/Sm97] | ja, je Loch Zwei- oder Dreitor [W-s, Sc/Sm97]; Übergang offen/zu: nicht belegt | Blatt als Streuglied [E] | Lochzustand umschaltbar; stetiger Übergang nicht gefunden | 4 Filter 2. Ordnung je Loch [W-s, Sc/Sm97 Abschn. 3] | gegen Keefe-Daten verglichen [W-s, Abstract] |
| C Webster-Lokshin-Zustandsraum | nein [E] | keine [E] | viskothermisch, Wand [W-s, Mignot09 DAFX-1] | nicht behandelt [W-s, DAFX-5] | nicht behandelt | „static parameters" [W-s, DAFX-5] | Ordnung reduzierbar; „equivalent to former models" [W-s, DAFX-5] | Geometrie → Impedanz, gegen gemessene Posaune verglichen [W-s, DAFX-4] |
| D Reflexionsfunktion (Faltung) | nein [E] | durch Messband begrenzt [E] | in der Messung enthalten [E] | je Griff eine Funktion [E] | Blatt/Lippe nichtlinear [Z, Vergez/Rodet] | Wechsel nur durch Umschalten [E] | Faltung teurer als rekursiv [W-s, M18 DAFx-157] | direkt aus gemessener Impedanz; zeitliche Faltung (time-aliasing) als Falle [W-s, ISMA17 zu Gazengel 1995] |
| E Moden, Zeitintegration | **ja**, je Pol und Residuum [W-w, Taillard; W-s, Petiot S. 377] | endlich, passiv nur mit HF-Ergänzung [W-w, Taillard] | in Polbreiten [E]; örtlich nichtlinear über interpolierte Pole [W-w, Szwarcberg] | Übergang behandelt [W-w, Taillard] | Blatt als ODE [W-w, Silva07] | Moden vorab je Zustand; schwierig bei Ventilspiel [W-s, Harrison] | Echtzeit Klarinette [W-w, Taillard]; implizit teuer und dispersiv [W-s, M18] | Prony/LSCE an Messung [W-w, Taillard]; **schnelle Vorhersage**: ML von 44 Modalparametern → Spielfrequenz je Regime [W-s, Petiot S. 378] |
| F modale Reflektanz + Waveguide-Blatt | **ja**, b0/b1 je Resonator steuern Betrag und Phase [W-s, M18 DAFx-158] | endlich, Mittel 32; HF auf Z/Zc→1 überblendet [W-s, M18 DAFx-159/163] | in Polen, Abstrahlung über dieselbe Bank [W-s, DAFx-160] | lineare Mischung der Griffe, passiv [W-s, DAFx-162]; Treue „to be elucidated" [W-s, DAFx-163] | Blatt als Streuglied; kein Schmettern | quasistatische Gewichte [W-s, DAFx-162]; Zug nicht behandelt | „above 30 times faster than real-time" [W-s, DAFx-163] | SQP auf Pole, Zähler geschlossen; Impedanz und Abstrahlung gemessen [W-s, DAFx-158] |
| G Vokaltrakt als Waveguide | Vokaltrakt-Resonanz einstellbar [W-s, Sc03] | – | RLC-Analogie [W-s, Sc03] | – | Kopplung am Blatt [W-s, Sc03] | in Echtzeit steuerbar [Z, Abstract] | gering [E] | nicht behandelt |
| I nichtlineare Ausbreitung | – | – | – | – | Schmettern in der Schleife [W-s, Cooper/Abel DAFX-4]; TVD [W-w, Berjamin] | – | vereinfacht echtzeitfähig [W-s, Cooper/Abel DAFX-5] | spektrale Anreicherung gemessen [W-s, Cooper/Abel Abstract] |
| J FDTD | nein [E] | Gitter [E] | viskothermisch, Abstrahlung [Z, Bilbao/Chick] | Löcher öffnen und schließen [W-w, Thibault]; Ventile [W-s, Harrison] | Lippe, Ausbreitung (offen bei Willemsen) [W-s] | **ja**, Zug über dynamisches Gitter [W-s, Willemsen]; Ventile [W-s, Harrison] | Trompete Echtzeit [W-s, Harrison]; Plugin [W-s, Willemsen] | Geometrie; FWI-Inversion aus Impedanz, 14 Variablen „in about 1 minute on a laptop" [W-w, Ernoult HAL-Abstract] |

**Inversion der Bohrung (Frage 3):**
- Ernoult et al. 2021: Der Gradient wird „with better accuracy and less additional cost than with finite-difference" berechnet [W-w].
- TubeLab 2026: Transfermatrix plus „a single adjoint backward pass per mode", harte Nebenbedingungen auf Moden, nur „bores without tone holes". Geprüft an „16 numerical control-machined didgeridoos" [W-w].

**Schnelle Vorhersage der Messgrößen (Frage 3):**
- Gefunden nur auf modaler Seite: Petiot et al. lernen von Modalparametern auf die äquivalente Grundtonhöhe (EFP) je Regime. Die wichtigste Größe je Regime ist „the frequency of the mode corresponding to this regime" [W-s, S. 380].
- Eine solche Vorhersage für Waveguide-Parameter habe ich nicht gefunden.

---

## 4. Abgleich mit dem vorgeschlagenen modalen Rohr-Kern

**Grundsatz:** Der Vorschlag beruft sich auf Maestre 2018, und dieses Verfahren **ist bereits** Moden plus Waveguide-Schnittstelle (Reflektanz, Blatt als Streuglied) [W-s, DAFx-157]. Die Frage lautet also nicht „Moden oder Waveguide", sondern **welche Teile zusätzlich als Waveguide laufen sollen**.

**Wo ein Waveguide-Teil verbessern würde [E, gestützt wie angegeben]:**
1. **Zugposaune.** Der zylindrische Zug ist genau der Fall, für den eine veränderliche Verzögerung „simple and computationally efficient" ist [W-s, Willemsen S. 152].
   - Moden überblenden ist quasistatisch [W-s, M18 DAFx-162].
   - Beim Ventilspiel bzw. bei Zeitveränderlichem werden Moden „awkward" [W-s, Harrison].
   - Denkbar [E]: Mundstück und Schallstück modal bzw. als Filter, dazwischen der Zug als Waveguide mit Bruchteil-Verzögerung. Veröffentlicht fand ich diese Form nicht.
2. **Schmettern.** Eine Stufe nur am Ausgang lässt die Rückwirkung weg. Nach Msallam (über Cooper/Abel) verändert gerade der Rückwärtsanteil in der Schleife die Grundfrequenz und das Schmettern [W-s, DAFX-4].
   - Ein kurzes Waveguide-Stück mit druckabhängiger Verzögerung im zylindrischen Teil wäre die belegte Waveguide-Lösung [W-s, Cooper/Abel].
   - Für vorab gerechnete Töne ist das unkritisch billig [E].
3. **Vokaltrakt.** Er lässt sich als Waveguide bzw. Reflektanz vor dem Anreger koppeln [W-s, Sc03]; Maestre nennt das selbst als nächsten Schritt [W-s, DAFx-163]. Das passt an die Wanderwellen-Schnittstelle des Vorschlags [E].
4. **Stetige Tonloch-Übergänge und halb gedeckte Löcher.** Ein Netz aus Streugliedern [W-s, Sc/Sm97] oder FDTD [W-w, Thibault] bildet Zwischenzustände physikalisch. Die Mischung der Moden ist nur eine Überblendung zwischen gemessenen Endzuständen [E].

**Wo ein Waveguide nicht verbessern würde:**
1. **Setzen an Aufnahmen und Messungen.** Die modale Form trägt die Messgrößen direkt: Pole und Residuen aus der Impedanz [W-s, M18; W-w, Taillard]. Ein Ersatzmodell (Surrogat) sagt die Spielfrequenz aus den Modalparametern vorher [W-s, Petiot]. Das entspricht dem Flügel-Muster (SZ3 a). Ein Waveguide hat diese Abbildung nicht; seine Parameter sind Geometrie bzw. Filter [E].
2. **Echte Bohrungen mit Schallstück.** Das Schallstück ist im Waveguide ohnehin ein Filter [W-w, PASP]. Die feine Profiltreue verlangt viele Stücke [W-s, Mignot09 DAFX-4]. Modal ist das kompakter [E].
3. **Rechenzeit vorab.** Schon modal mehr als 30-fach schneller als Echtzeit [W-s, M18]. Ein Waveguide spart hier nichts Nennenswertes [E].
4. **Steuerung je Resonanz** (Stimmung, Ansprache je Regime) gibt es nur modal [W-s, M18 DAFx-158; Petiot S. 380].

**Gesamt [E]:** Den modalen Kern behalten, die Waveguide-Schnittstelle (Reflektanz/Wanderwellen) ausdrücklich zum Vertrag machen, und **gezielt** Waveguide-Stücke einsetzen: Zug, nichtlinearer Zylinderteil, Vokaltrakt. Für Ventilspiel und halb gedeckte Löcher bleibt FDTD die Deluxe-Stufe. Ich stelle das hier zur Entscheidung, es ist keine Festlegung.

---

## 5. Nicht gefunden

Kein Treffer ist kein Beweis.
- Smith 1986 (ICMC): Volltext nicht gefunden, nur bibliographische Angaben [Z].
- Msallam et al. 2000: weder Volltext noch Abstract. Der Befund zum Rückwärtsanteil ist nur über Cooper/Abel belegt.
- Schumacher 1981: nicht gesucht bzw. nicht gefunden.
- Välimäkis Bruchteil-Verzögerung für Kegel: nur Einträge, kein Text. Belegt nur als Verweis „Valimaki, et al. [9]" bei Cooper/Abel [W-s, DAFX-1].
- Eine veröffentlichte Mischform „Moden für tiefe Resonanzen + Waveguide/Reflexionsfunktion für hohe": nicht gefunden. Am nächsten kommen die HF-Überblendung bei Maestre (Z/Zc→1) [W-s, DAFx-159] und die fiktiven HF-Punkte bei Taillard [W-w].
- Eine Waveguide-Zugposaune mit zeitveränderlicher Bruchteil-Verzögerung als eigene Arbeit: nicht gefunden.
- Waveguide + modales Schallstück als eigene Arbeit: nicht gefunden.
- Modale Synthese für Flöte, Blockflöte oder Labialpfeife mit Strahlanregung (jet): nicht gefunden. Auvray et al. 2014 vergleichen Strahl-Modelle (jet-drive, discrete-vortex) im Zeitbereich, im Abstract ohne Moden [W-w]. In OpenWInD ist die Flöte „coming soon" [W-w].
- Eine schnelle Vorhersage der Messgrößen aus Waveguide-Parametern: nicht gefunden.
- Acta Acustica 2022 (10.1051/aacus/2022004), Lippe + komplexe Moden + Fortsetzung (continuation): 403, nur Suchzusammenfassung [Z].
- Alphorn, Didgeridoo-Synthese, Shakuhachi: nichts Spezifisches zur Kombination. Ausnahme TubeLab für die Inversion von Didgeridoo-Bohrungen [W-w].

---

## 6. Eigene Einschätzung (kein Beleg), je Klasse

**Klasse 7 (Lippe + Rohr):**
- **Posaune:** Hier bringt die Kombination am meisten. Der Zug als Waveguide-Stück mit Bruchteil-Verzögerung zwischen modalem Mundstück- bzw. Schallstückteil, dazu Schmettern mit druckabhängiger Verzögerung im zylindrischen Teil, innerhalb der Schleife.
- **Trompete und Tuba:** Ventile als Umschalten bzw. Mischen der Moden reichen für sauberes Spiel. Halbventil und Ventil-Glissando brauchen FDTD (Bilbao 2011, Harrison).
- **Horn:** Stopfen und die Hand im Schallstück sind modal je Zustand messbar. Ein Waveguide hilft hier nicht.
- **Alphorn und Didgeridoo:** Ein einziger Griff, also rein modal. Beim Didgeridoo ist der Vokaltrakt entscheidend und als Reflektanz bzw. Waveguide vor der Lippe sinnvoll; die Inversion der Bohrung über TubeLab bzw. FWI ist möglich.
- Nutzen gesamt: hoch bei Posaune, mittel bei Didgeridoo (Vokaltrakt), gering sonst.

**Klasse 8 (Rohrblatt + Rohr):**
- Maestre 2018 ist hier fast der vorgeschlagene Kern.
- Ein Waveguide bringt den Vokaltrakt (Saxophon-Altissimo, Bend; Scavone 2003) und eventuell stetige Tonloch-Übergänge über Streuglieder, für Triller und halb gedeckte Löcher.
- Griffwechsel per Überblendung ist belegt, aber in seiner Treue offen.
- Nichtlineare Verluste am Lochende gehen modal über Szwarcberg (amplitudenabhängige Pole).
- Nutzen: mittel (Vokaltrakt), sonst gering.

**Klasse 9 (Luftstrahl + Rohr):**
- Der Strahl ist eine Verzögerung (Konvektionszeit) plus Nichtlinearität. Das liegt dem Waveguide nahe: Bei STK und CCRMA ist die Flöte klassisch ein Waveguide mit Strahl-Verzögerungsleitung [Z]. Mit einer modalen Reflektanz am Rohr ist das gut kombinierbar, die Schnittstelle bleibt gleich.
- Für das Setzen an Messungen bleibt die modale Admittanz bzw. Impedanz je Griff sinnvoll. Eine Kombination für die Flöte ist aber nicht belegt.
- Orgel-Labialpfeife: ein fester Ton, also rein modal plus Strahl. Ein Waveguide bringt nichts außer der Strahl-Verzögerung.
- Shakuhachi: Meri/Kari sind Änderungen an der Anblasung; das ist eine Frage des Anregers, nicht des Rohrs.
- Nutzen: mittel, als Waveguide für den Strahl und nicht für das Rohr.

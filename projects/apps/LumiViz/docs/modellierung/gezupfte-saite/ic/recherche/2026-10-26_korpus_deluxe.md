# Recherche IC2: Gezupfte Saite (Deluxe) — der gerechnete Gitarrenkorpus

> **Auftrag:** Klasse 2: Modelle, Daten, Zwei-Richtungs-Admittanz, Streuung und Abstrahlung eines gerechneten
> Gitarrenkorpus (Stahl, Nylon) über die Korpusse Parlor und Classical in LumiViz hinaus.
> **Datum:** 2026-10-26 (IC Session 2).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, Werkzeug]; [nur Zusammenfassung]; [eigene Rechnung]; [wörtlich, selbst gelesen] kommt nicht vor.
> **Reichweite:** im Bericht selbst (21 Suchen, 39 Abrufe; HAL, Acta Acustica, MDPI, IRCAM, ORCA gesperrt; vier PDFs vom
> Werkzeug zwischengespeichert, aber nicht lesbar).
> **Von der Sitzung selbst nachgelesen (IC Session 2, 2026-10-26, `quelle_suchen.py`/`pdftotext` in den PDFs des
> Caches):** Bank & Karjalainen 2010 (Abstract PDF-S. 1; Gibson 1960er, 2×2 durch Drahtbruch, R = 30, „beating and
> two-stage decay“ PDF-S. 5; Stabilität PDF-S. 6; kein Rechtsvermerk); Hess 2013 (Abstract, Tab. 1 S. 3); Scherrer &
> Depalle (JCAA, „2x2 admittance matrix of the body at the bridge“, Abgabewinkel PDF-S. 1); Bécache et al.
> (Vorabdruck „Numerical simulation of a guitar“, Abstract und Einleitung PDF-S. 1). Die übrigen Angaben (Brauchler,
> Russell, Ducceschi, Cardiff) **nicht** selbst gelesen. Eingeflossen in `quellen.md`, `modell-entwurf.md`,
> `offene-fragen.md`.

---

# Recherche Klasse 2: Gerechneter Gitarrenkorpus „Deluxe“ (Stahl- und Nylongitarre)

**Auftrag:** Was es über die zwei Korpusse von LumiViz (gekoppelte Schwinger Decke + Helmholtz) hinaus an Modellen, Daten, Zwei-Richtungs-Admittanzen, Streuungs- und Abstrahlungsbefunden gibt.
**Datum:** 2026-10-26

**Reichweite:**
- **Umfang:** 21 Websuchen (zwei davon im Modus „extended“) und 39 Abrufe mit WebFetch, Fehlschläge mitgezählt.
- **Sprachen:** Englisch; deutsch- und französischsprachige Treffer kamen nur nebenbei.
- **Suchbegriffe (Auswahl):** Derveaux/Bécache „time-domain simulation of a guitar“; Brauchler „reverse-engineered FE guitar“ und „possibilistic identification“; Woodhouse „plucked guitar transients“; „three-mass model“; „dreadnought FE X-bracing“; „bridge mobility in-plane“; „guitar radiation monopole dipole“; Tonholz-Kennwerte; Mores 65 Gitarren; Ducceschi 65 guitars.
- **Datenbanken und Orte:** Crossref-API (4 Abfragen), arXiv (abs- und HTML-Seiten), PMC, Savart Journal, DAFx-Archiv, euphonics.org, CCRMA (J. O. Smith), ITM Stuttgart, Vienna Talk 2015, Website von D. Russell.

**Nicht erreichbar:**
- **Gesperrt (HTTP 403 oder Anubis-Botsperre):** HAL (ENSTA), Acta Acustica (edpsciences, alle Artikel), DOAJ, MDPI, IRCAM, ORCA Cardiff, RUN (UNL).
- **Weitergeleitet, nicht verfolgt:** nature.com leitet auf eine Anmeldung um; der Artikel wurde stattdessen über PMC gelesen.
- **DNS-Fehler:** fpl.fs.usda.gov (Wood Handbook).
- **404:** ein Proceedings-Papier des IOA.

**Wichtigste Einschränkung:** WebFetch kann **komprimierte PDFs nicht lesen**. Bei allen vier abgerufenen PDFs kam nur Binärinhalt zurück. Das Werkzeug hat die PDFs dabei selbst zwischengespeichert, unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\85f9b48d-67e4-462b-b6d7-b07e6390615b\tool-results\`:
- `webfetch-1793049118660-4grgko.pdf`: Bécache et al., guitare_CS.pdf (1,5 MB)
- `webfetch-1793049219598-cuk141.pdf`: JCAA-Artikel (Scherrer, McGill), Zwei-Richtungs-Admittanz (172 KB)
- `webfetch-1793049231572-yfcjep.pdf`: Bank & Karjalainen, DAFx 2010 (240 KB)
- `webfetch-1793049348860-jdk4d7.pdf`: Hess, Savart Journal 2013 (358 KB)

Gemäß der Regel „PDFs nur über das Abrufwerkzeug“ habe ich diese Dateien **nicht** mit anderen Mitteln geöffnet. Eine Freigabe, sie mit `quelle_suchen.py` zu lesen, würde die Belege deutlich verbessern (siehe Abschnitt 8).

Kein Treffer ist kein Beweis. Die Suche war breit, aber durch die Sperren lückenhaft.

**Kennzeichnung:**
- **[wörtlich, Werkzeug]:** Das Abrufwerkzeug hat den Wortlaut als Zitat geliefert.
- **[nur Zusammenfassung]:** Das Werkzeug hat eine Angabe als Stichpunkt wiedergegeben, nicht als Zitat. Das gilt auch für Zahlen. Diese Angaben haben keine Belegkraft.
- **[eigene Rechnung]:** von mir gerechnet.
- **[wörtlich, selbst gelesen]:** **kommt nicht vor**, weil kein Volltext selbst gelesen werden konnte.

---

## 1 Quellen

**Q1. Rechnung im Zeitbereich (Bécache, Chaigne, Derveaux, Joly 2003)**
- **Zitat:** Bécache, É.; Chaigne, A.; Derveaux, G.; Joly, P. (2003): „Time-domain simulation of a guitar: Model and method“, J. Acoust. Soc. Am. 114(6), 3368–3383.
- **Belegt:** Titel, Autoren und Ort stammen aus der Websuche und dem Research Portal von IP Paris [nur Zusammenfassung].
- **Fundort:** HAL hal-00989042 (gesperrt). Vorstufe „Numerical simulation of a guitar“ in *Computational Fluid and Solid Mechanics 2003*, S. 1244–1246, DOI 10.1016/b978-008044046-0/50305-5 (Crossref [wörtlich, Werkzeug]). Kurzfassung JASA 112(5, Suppl.) 2409 (2002), DOI 10.1121/1.4779840 (Crossref [wörtlich, Werkzeug]).
- **Frei/Recht:** JASA nicht frei; Rechtsvermerk nicht gelesen.
- **Abruf:** 2026-10-26. Das PDF von perso.ensta.fr war nicht lesbar (siehe oben).

**Q2. FE-Modell aus CT-Daten (Brauchler, Ziegler, Eberhard 2021)**
- **Zitat:** Brauchler, A.; Ziegler, P.; Eberhard, P. (2021): „An entirely reverse-engineered finite element model of a classical guitar in comparison with experimental data“, J. Acoust. Soc. Am. 149(6), 4450–4462. DOI 10.1121/10.0005310.
- **Frei/Recht:** Crossref nennt keine Lizenz-URL [wörtlich, Werkzeug]; der Artikel ist nicht frei. Eine Kopie liegt als Anhang einer Stanford-Mailingliste, mit ungeklärter Rechtslage; nicht abgerufen.
- **Abruf:** 2026-10-26.

**Q3. Unterscheidung baugleicher Gitarren (Brauchler, Hose, Ziegler, Hanss, Eberhard 2022)**
- **Zitat:** Brauchler, A.; Hose, D.; Ziegler, P.; Hanss, M.; Eberhard, P. (2022): „Distinguishing geometrically identical instruments: Possibilistic identification of material parameters in a parametrically model order reduced finite element model of a classical guitar“, J. Sound Vib. 535, 117071. DOI 10.1016/j.jsv.2022.117071.
- **Frei/Recht:** Lizenz laut Crossref „elsevier.com/tdm/userlicense/1.0“ [wörtlich, Werkzeug]; nicht frei. Keine Kurzfassung in Crossref.
- **Abruf:** 2026-10-26.

**Q4. Geometrie gegen Holzstreuung (Brauchler et al. 2023)**
- **Zitat:** Brauchler, A.; Gonzalez, S.; Vierneisel, M.; Ziegler, P.; Antonacci, F.; Sarti, A.; Eberhard, P. (2023): „Model-predicted geometry variations to compensate material variability in the design of classical guitars“, Sci. Rep. 13, 12766. DOI 10.1038/s41598-023-37943-y.
- **Fundort:** PMC.
- **Frei/Recht:** frei. Rechtsvermerk: „This article is licensed under a Creative Commons Attribution 4.0 International License“ [wörtlich, Werkzeug].
- **Abruf:** 2026-10-26.

**Q5. Diskrepanzmodell für reduzierte Modelle (Cillo et al. 2024)**
- **Zitat:** Cillo, P.; Brauchler, A.; Gonzalez, S.; Ziegler, P.; Antonacci, F.; Sarti, A.; Eberhard, P. (2024): „Improving accuracy in parametric reduced-order models for classical guitars through data-driven discrepancy modeling“, Acta Acustica 8, 59. DOI 10.1051/aacus/2024055.
- **Frei/Recht:** Lizenz-URL laut Crossref creativecommons.org/licenses/by/4.0 [wörtlich, Werkzeug]. Volltext: 403.
- **Abruf:** 2026-10-26.

**Q6. Polarisationskopplung am Sattel (Brauchler, Ziegler, Eberhard 2020)**
- **Zitat:** Brauchler, A.; Ziegler, P.; Eberhard, P. (2020): „Examination of polarization coupling in a plucked musical instrument string via experiments and simulations“, Acta Acustica 4, 9. DOI 10.1051/aacus/2020008.
- **Frei/Recht:** CC BY 4.0 laut Crossref [wörtlich, Werkzeug]. Volltext: 403.
- **Abruf:** 2026-10-26.

**Q7. Port-Hamilton-Modell (Rettberg et al. 2022)**
- **Zitat:** Rettberg, J.; Wittwar, D.; Buchfink, P.; Brauchler, A.; Ziegler, P.; Fehr, J.; Haasdonk, B. (2022): „Port-Hamiltonian Fluid-Structure Interaction Modeling and Structure-Preserving Model Order Reduction of a Classical Guitar“, arXiv:2203.10061.
- **Verwandte DOI:** 10.1080/13873954.2023.2173238.
- **Frei/Recht:** frei; „CC BY-NC-SA 4.0“ laut arXiv-Seite [nur Zusammenfassung des Lizenzfelds].
- **Abruf:** 2026-10-26.

**Q8. Holzkennwerte aus Chladni-Mustern (Ducceschi et al. 2023)**
- **Zitat:** Ducceschi, M.; Duran, S.; Tahvanainen, H.; Ausiello, L. (2023): „A method to estimate the rectangular orthotropic plate elastic constants using least-squares and Chladni patterns“, arXiv:2311.06935v1.
- **Frei/Recht:** frei; „License: CC BY 4.0“ [wörtlich, Werkzeug].
- **Abruf:** 2026-10-26.

**Q9. Modalanalyse Gibson Hummingbird (Russell 1998)**
- **Zitat:** Russell, D. A. (1998): „Modal Analysis of an Acoustic Folk Guitar“, Webseite Kettering University, heute acs.psu.edu/drussell/guitars/hummingbird.html.
- **Frei/Recht:** frei lesbar, aber nicht frei lizenziert: „Copyrighted © 1998 Daniel A. Russell, Ph.D, …“ [wörtlich, Werkzeug].
- **Abruf:** 2026-10-26.

**Q10. Abwandlungen an einer Stahlsaitengitarre (Hess 2013)**
- **Zitat:** Hess, D. P. (2013): „Frequency Response Evaluation of Acoustic Guitar Modifications“, Savart Journal, veröffentlicht am 5. Juli 2013.
- **Frei/Recht:** frei lesbar; „All site contents copyright © Savart Journal 2011 - 2025“ [wörtlich, Werkzeug]. Volltext-PDF nicht lesbar.
- **Abruf:** 2026-10-26.

**Q11. Erste Gitarrenmode (Sali 2011)**
- **Zitat:** Sali, S. (2011): „Modeling and Optimizing of the First Guitar Mode“, Savart Journal, veröffentlicht am 13. Juni 2011.
- **Frei/Recht:** Rechtsvermerk wie Q10.
- **Abruf:** 2026-10-26.

**Q12. Admittanzmatrix für Synthese (Bank & Karjalainen 2010)**
- **Zitat:** Bank, B.; Karjalainen, M. (2010): „Passive Admittance Matrix Modeling for Guitar Synthesis“, DAFx-2010, Graz.
- **Fundort:** dafx.de/paper-archive/2010/DAFx10/BankKarjalainen_DAFx10_P60.pdf (PDF nicht lesbar).
- **Frei/Recht:** frei lesbar; kein Rechtsvermerk auf der Archivseite.
- **Abruf:** 2026-10-26.

**Q13. 65 Klassikgitarren aus Messdaten (Ducceschi, Russo, Webb 2026)**
- **Zitat:** Ducceschi, M.; Russo, R.; Webb, C. J. (2026): „Measurement-Informed Nonlinear Modal Synthesis of 65 Classical Guitars“, DAFx-2026, Cambridge (MA).
- **Fundort:** dafx.de/paper-archive/2026/papers/DAFx26_paper_40.pdf.
- **Frei/Recht:** frei lesbar; kein Rechtsvermerk auf der Archivseite.
- **Abruf:** 2026-10-26.

**Q14. Zehnsaitige brasilianische Gitarre (Paiva et al. 2015)**
- **Zitat:** Paiva, G.; Gautier, F.; Ablitzer, F.; Santos, J. M. C. (2015): „Time-domain simulations of a ten-string Brazilian guitar“, Proc. Third Vienna Talk on Music Acoustics, S. 125.
- **Fundort:** viennatalk2015.mdw.ac.at.
- **Frei/Recht:** Rechtsvermerk zum Text nicht gefunden.
- **Abruf:** 2026-10-26.

**Q15. Euphonics, Abschnitte 4.3 und 5.1 (Woodhouse)**
- **Zitat:** Euphonics (J. Woodhouse), Abschnitte „4.3 Sound radiation“ und „5.1 Stringed instrument overview“, euphonics.org.
- **Frei/Recht:** frei. „Except where otherwise noted, the content on this site is licensed under a“ … „Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License“ [wörtlich, Werkzeug].
- **Abruf:** 2026-10-26.

**Q16. Synthetische Stegadmittanz (Smith 2010)**
- **Zitat:** Smith, J. O. III (2010): *Physical Audio Signal Processing*, W3K Publishing, Abschnitt „Building a Synthetic Guitar Bridge Admittance“.
- **Fundort:** ccrma.stanford.edu.
- **Frei/Recht:** „Copyright © 2026-08-21 by Julius O. Smith III“ [wörtlich, Werkzeug].
- **Abruf:** 2026-10-26.

**Q17. Abstrahlung von Archtop-Gitarren (Longo et al. 2025)**
- **Zitat:** Longo, G.; Gonzalez, S.; Dalisay, J. D. E.; Nania, T.; Antonacci, F. (2025): „Influence of thickness profile and bracing pattern in the radiation patterns of archtop guitars“, J. Acoust. Soc. Am. 157(2), 1141–1150. DOI 10.1121/10.0035805.
- **Frei/Recht:** Crossref nennt keine Lizenz [wörtlich, Werkzeug].
- **Abruf:** 2026-10-26.

**Q18. Abstrahlungsmessungen an Gitarren (Cardiff, ORCA eprint 70916)**
- **Zitat:** „Sound radiation measurements on guitars and other stringed musical instruments“, ORCA Cardiff, eprint 70916. Vermutlich eine Dissertation aus der Gruppe Richardson; Autor und Jahr nicht gesehen.
- **Belegt:** Nur aus der Websuche; die Landeseite gab 403 [nur Zusammenfassung].
- **Abruf:** 2026-10-26.

**Q19. Archiv Mores (Bekanntes Zenodo-Archiv, hier nur ergänzt)**
- **Inhalt:** Das bekannte Zenodo-Archiv 4604576. Die Websuche nennt mehr als 60 Gitarren, meist klassische, Flamenco- und romantische Gitarren, und einen Konflikt der DOIs 4604576 und 4604577 [nur Zusammenfassung]. Am Ursprung prüfen.
- **Abruf:** 2026-10-26.

## 2 Modelle und Kosten

| Modell | Freiheitsgrade, Moden | Aufwand | Genauigkeit gegen Messung | Quelle, Kennzeichnung |
|---|---|---|---|---|
| Zwei-Massen-Modell (Decke + Luft) und Drei-Massen-Modell (Decke + Boden + Luft) | 2 bzw. 3 FG; das Drei-Massen-Modell sagt drei Resonanzen mit gleicher Form, aber verschiedener Phasenlage von Decke, Boden und Luftpfropfen voraus | trivial | Hess 2013: das Modell mit konzentrierten Parametern folgt den gemessenen Änderungen eng; keine Zahl gesehen | IOA-Treffer (Richardson et al.) und Q10, beide [nur Zusammenfassung] |
| FE mit eingeschlossener Luft (Fluid-Struktur-Kopplung), Geometrie aus CT | fünf Holzarten orthotrop, Luft vernetzt | hoch | „The quality of the resulting model is demonstrated by comparing“; konkrete Fehlerzahlen nicht gesehen | Q2 [wörtlich, Werkzeug] für das Zitat, sonst [nur Zusammenfassung] |
| FE vollständig gegen reduziertes Modell (parametrische Modellreduktion, PMOR) | volles Modell N = 400 128 FG, reduziert n = 600 FG, 30 Moden bis 1000 Hz | 78 s → 0,04 s (Faktor 1950) für die ersten 30 Modalparameter | Reduziertes Modell R² = 0,96; nach Anpassung mittlerer Frequenzfehler 1,55 % (27 Cent), höchstens 4,99 % (88 Cent, 6. Mode); mittlerer MAC 0,92 über 13 Moden; Wiederholbarkeit der Messung ±0,7 % | Q4 [nur Zusammenfassung] |
| Reduziertes Modell + neuronales Diskrepanzmodell | – | – | Die Kombination sei genauer als jedes Verfahren allein | Q5 [nur Zusammenfassung] |
| Port-Hamilton-FSI mit strukturerhaltender Reduktion, transient | – | – | – | Q7 [wörtlich, Werkzeug] für den ersten Satz der Kurzfassung |
| 3D im Zeitbereich: Saite + Decke (Spektralverfahren) + Luft im Freifeld (fiktive Gebiete), energieerhaltend | – | hoch (3D-Luft) | Vergleich Decke im Vakuum / starre Decke / elastische Decke; Studien zu Dicke und Volumen | Q1 [nur Zusammenfassung] |
| Hybrid: gemessene Korpusmoden (experimentelle Modalanalyse) gekoppelt an Saitenmoden | – | – | zeigt Mitschwingen und Schwebung | Q14 [wörtlich, Werkzeug] |
| Modalsynthese mit Steg-Nachgiebigkeit und Abstrahlung aus Messung, je Gitarre | – | O(N) je Zeitschritt (Sherman–Morrison) | – | Q13 [nur Zusammenfassung] |

**Wichtig zu Q4:** Die Zahlen gelten nach meinem Eindruck für **Decken** (top 1 / top 2), nicht für fertige Korpusse. Das ist ungeklärt, der Volltext ist nicht selbst gelesen.

## 3 Daten

**Holz (Decke)**

| Größe | Wert | Quelle, Kennzeichnung |
|---|---|---|
| Fichte (Fiemme), Gitarrendecke 60 × 41,2 cm, Dicke 3 mm | ρ = 399 kg/m³; E_x = 10,4 GPa (Std. 1,73 %); E_y = 1003 MPa (3,7 %); G_xy = 530 MPa (14 %); ν nicht bestimmt | Q8, Tab. 10 / Abschn. 4.2 [nur Zusammenfassung] |
| Abete rosso (*Picea abies*), Decke 1 / Decke 2 | E_L = 11,6 GPa, ρ = 403 kg/m³ / E_L = 9,31 GPa, ρ = 407 kg/m³; 35 Parameter je Decke identifiziert | Q4 [nur Zusammenfassung] |

Q8 sagt im Bildtext, die Werte passten zu Bucur, *Handbook of materials for string musical instruments*, S. 96 [wörtlich, Werkzeug: „Values are in line with previously reported values for spruce“].

**Holz (Boden)**
- Für Palisander und Mahagoni (E längs/quer, G, Dämpfung) habe ich **keinen Primärbeleg** erreicht. Die IRCAM-Tabelle und die MDPI-Seite waren gesperrt, das Wood Handbook war nicht erreichbar.
- Werte aus Suchzusammenfassungen (IRCAM: Palisander 730–740 kg/m³, E_L 12 GPa) haben **keine Belegkraft**.

**Beleistung**

| Größe | Wert | Quelle, Kennzeichnung |
|---|---|---|
| Fächerbalken (Klassik) | 7 × 7 mm | Q4 [nur Zusammenfassung] |
| Querbalken | 23,5 × 7 mm | Q4 [nur Zusammenfassung] |

Zu X-Bracing (Stahlsaite) gibt es keine Zahlen.

**Gemessene Moden einer Dreadnought-Form (Gibson Hummingbird, frühe 1960er)**
- Die Gitarre ist „is a flat-top folk guitar with a standard Dreadnought shape“ mit „Sitka Spruce top, Mahogany back and sides, 24-3/4" scale length“ [wörtlich, Werkzeug].
- Gemessen mit gestimmten Saiten, an Gummibändern aufgehängt [nur Zusammenfassung].
- **Keine Güte und keine wirksame Masse.**

| Mode | f [Hz] | Beschreibung | Kennzeichnung |
|---|---|---|---|
| 1 | 59 | Biegung wie ein frei-freier Balken | [nur Zusammenfassung] |
| 2 | 103 | „Atmen“: Decke und Boden gegenphasig, Helmholtz | [nur Zusammenfassung] |
| 3 | 188 | Decke und Luft gleichphasig, Boden leicht gegenphasig | [nur Zusammenfassung] |
| 4 | 202/203 | Decke, Luft und Boden gleichphasig, Halsbiegung | [nur Zusammenfassung] |
| 5 | 223 | zweite Biegemode | [nur Zusammenfassung] |
| 6 | 231 | – | [nur Zusammenfassung] |
| 7 | 262 | (0,1) | [nur Zusammenfassung] |
| 8 | 315 | (1,0) | [nur Zusammenfassung] |
| 9 | 385 | (0,2) Boden | [nur Zusammenfassung] |
| 10 | 481 | (1,1) Boden | [nur Zusammenfassung] |
| 11 | 749 | Decke (1,2) mit Boden (1,3) | [nur Zusammenfassung] |

Quelle: Q9. Wörtlich [Werkzeug]: „The Low Frequency modes are dominated by strong coupling between the motion of the top plate, bottom plate and air cavity.“

**Volumen und Schalllochfläche einer Dreadnought:** **nicht belegt** gefunden. Nur Forenangaben: Schallloch 4" bzw. 4,25"; Luftresonanz „near G … around 98 Hz“. Sie haben keine Belegkraft.

## 4 Zwei Richtungen und Kopplung

- **Messung in zwei Polarisationen:** Brauchler et al. 2020 (Q6) haben eine gezupfte Saite mit zwei Laser-Vibrometern in beiden Polarisationen gemessen. Danach kommt die Kopplung der Polarisationen aus der **Torsion der Saite am Sattel** [nur Zusammenfassung]. Wörtlich [Werkzeug]: „the transient motion of a realistically plucked guitar string is studied experimentally and numerically“.
- **Admittanzmatrix für Synthese:** Bank & Karjalainen 2010 (Q12) erweitern die passive Admittanz zur **2×2-Admittanzmatrix**. Die Matrix ist eine Summe von Filtern zweiter Ordnung mal positiv semidefiniter Matrizen, umgesetzt als Reflektanz für einen Waveguide mit zwei Polarisationen, **angepasst an gemessene Stegadmittanzen einer Gitarre** [nur Zusammenfassung].
  - Welche Gitarre, welche Beträge und welche Nebendiagonalen: nicht gelesen, das PDF ist zwischengespeichert.
  - Laut dem EU-Bericht NEST-PM (Websuche) führten gemessene Stegdaten direkt in der Synthese oft zu **Instabilität**. Daraus entstand der passive Ansatz [nur Zusammenfassung].
- **Weitere Zwei-Richtungs-Admittanz:** Der JCAA-Artikel (Scherrer, McGill; PDF zwischengespeichert) koppelt laut Suchtreffer „the 2x2 admittance matrix of the body at the bridge“ in ein Waveguide-Netz [wörtlich, Werkzeug, aus dem Suchtreffer]. Bibliographische Daten sind nicht verifiziert.
- **Woodhouse 2004** (Acta Acustica 90, 945–965, „Plucked guitar transients“) ist der zentrale Kandidat für **gemessene Admittanzen senkrecht und parallel** am Steg. Nicht erreichbar, Bezug nur über Literaturangaben [nur Zusammenfassung].
- **Senkrecht gegen parallel:**
  - Smith (Q16, ältere Fassung pasp05): Die senkrechte Stegadmittanz sei viel größer als die waagrechte, daraus folge ein zweistufiges Abklingen [nur Zusammenfassung].
  - Euphonics (Q15): „Vibration perpendicular to the soundboard is much better.“ [wörtlich, Werkzeug]
  - Eine Zahl für das Verhältnis habe ich nicht gefunden.
- **Kopplung der Saiten untereinander:** Paiva et al. 2015 (Q14) zeigen Mitschwingen und Schwebung über gemessene Korpusmoden: „sympathetic resonances and beating phenomena“ [wörtlich, Werkzeug]. Debut, Antunes et al. (Portugiesische Gitarre, 12 Saiten, Applied Acoustics 2016) koppeln alle Saiten mit beiden Polarisationen am Steg [nur Zusammenfassung; RUN gesperrt].

## 5 Streuung und Setzen an Messung

- **Holzwerte aus Modaldaten (Inversion):** Die Gruppe in Stuttgart identifiziert Holzwerte durch Abgleich von Modell und Messung (Model Updating). Q3 nutzt dafür ein „possibilistisches“ Verfahren auf einem reduzierten FE-Modell. In Q4 sind es 35 Parameter je Decke [nur Zusammenfassung].
- **Holz gegen Geometrie:** Nach Q4 streuen die Eigenfrequenzen durch Holzstreuung im Mittel **etwas stärker** als durch Geometriestreuung. Die Moden 1, 2, 4, 6 und 8 sind am empfindlichsten für Holzwerte. Die Zahlenwerte der relativen Standardabweichung stehen nur in Abb. 3c [nur Zusammenfassung].
- **Ausgleich über Geometrie:** Q4 macht Decke 2 durch Änderung der Balkenhöhen Decke 1 ähnlich. Mittlerer Rest 1,55 %, höchstens 4,99 % (Zahlen siehe Abschnitt 2) [nur Zusammenfassung].
- **Streuung der Holzwerte selbst:** Q8 gibt für eine Decke Standardabweichungen von 1,7 % (E_L), 3,7 % (E_R) und 14 % (G_LR) an [nur Zusammenfassung]. Das ist die Unsicherheit der Schätzung, nicht die Streuung zwischen Hölzern.
- **Mores-Daten als Modalfit:** Ducceschi/Russo/Webb 2026 (Q13) ziehen aus den 65 Gitarren von Mores **Modalparameter der Steg-Nachgiebigkeit und der Abstrahlung vom Steg zur Luft je Gitarre** [nur Zusammenfassung]. Das ist die direkteste Vorlage für „Moden aus Messung“. Wie gut ein Bauplanmodell die Unterschiede der 65 Gitarren **erklärt**, habe ich nirgends gefunden.
- **Sonstige Vergleiche:** French et al. (Savart, Artikel 28) fanden „no clear correlation between features of measured frequency response functions and subjective ratings“ [wörtlich, Werkzeug]. French 2008 (*Sound and Vibration* 42, Streuung in einer Gruppe von Gitarren) ist nur als Zitat bekannt, nicht erreicht.

## 6 Abstrahlung

- **Cardiff (Q18):**
  - Abstrahlgrad von 80 bis 2000 Hz, Zerlegung in Kugelfunktionen.
  - Die **Monopolleistung der tiefen Dreiergruppe von Resonanzen** liefert unter 300 Hz den größten Anteil, darüber dominieren Dipole.
  - Der Abstrahlgrad hängt mehr von der Modenform ab, nämlich ungleich großen gegenphasigen Bäuchen, als von der Frequenz.
  - Alles [nur Zusammenfassung aus der Websuche]; die Landeseite gab 403.
- **Euphonics (Q15):** Zur Gitarre nur Grundsätzliches. Ein Umfang von etwa 0,9 m ergibt bei 82,4 Hz eine Helmholtzzahl von etwa 0,22, die bei rund 380 Hz Eins erreicht. Die beiden gekoppelten Moden bei 100 und 200 Hz liegen darunter [nur Zusammenfassung]. Für Fichte gelte: „there is not a single critical frequency“ [wörtlich, Werkzeug].
- **Freifeld ohne Schallwand:** Q1 rechnet die Luft um die ganze Gitarre im Freifeld [nur Zusammenfassung]. Hess 2013 (Q10) beschreibt eine Schätzung des Freifeld-Schalldrucks bei tiefen Frequenzen aus Nahfeld-Impulsantworten ohne reflexionsarmen Raum [nur Zusammenfassung].
- **Richtcharakteristik:** Longo et al. 2025 (Q17) rechnen Richtcharakteristiken von Archtop-Gitarren mit einem parametrischen 3D-Modell. Modelle und Code sind laut Kurzfassung offen [nur Zusammenfassung aus der Websuche]. Wörtlich aus Crossref [Werkzeug]: „we develop a fully parametric three-dimensional model of the guitar body.“
- **Gemessene Schallleistung je Mode in Zahlen:** nicht gefunden.

## 7 Nicht gefunden

- Volltexte von Christensen (3 FG), Caldersmith 1978 und Popp/Rossing/Polstein (Drei-Massen-Modell mit Parametern).
- Elejabarrieta 2001/2002 und Ezcurra 2005 in diesem Durchgang nicht neu gesucht, weil bekannt.
- Gough, Torres/Boullosa: nicht erreicht.
- **Gemessene Dreadnought-Moden mit Güte und wirksamer Masse.** Vorhanden sind nur Frequenzen bei Russell; Hess 2013 könnte sie enthalten (PDF zwischengespeichert, nicht lesbar).
- Volumen und Schallloch einer Dreadnought mit Beleg.
- Primärwerte für Palisander und Mahagoni (E, G, Dämpfung). Wood Handbook und MDPI Forests 16(4) 680 nicht erreicht.
- Zahlen zum Verhältnis von senkrechter zu paralleler Stegadmittanz. Woodhouse 2004, Bank 2010 und Scherrer kamen nur als Hinweise oder PDF-Binärinhalt.
- Eine Arbeit, die die Streuung im Mores-Datensatz mit einem Bauplanmodell erklärt.

## 8 Eigene Einschätzung (kein Beleg)

**Was trägt:**
- **Ein modales Korpusmodell mit 2×2-Admittanz am Steg** (senkrecht und parallel, positiv semidefinite Residuenmatrizen nach Bank 2010) ist die günstigste Stufe „Deluxe“. Es passt direkt zum vorhandenen Waveguide mit Verlust- und Steifheitsfilter: zwei Polarisationen plus eine passive Matrix-Reflektanz. Kosten: etwa 20–40 Filter zweiter Ordnung je Matrixelement; Stabilität durch Passivität garantiert.
- **Mitschwingen der leeren Saiten:** alle sechs Saiten an denselben modalen Steg koppeln (wie Paiva, Debut/Antunes), linear im Aufwand.
- **Daten für die Nylongitarre:** Am besten fundiert sind Modalfits aus den Mores-Daten nach dem Weg von Q13 (CC BY 4.0). Damit gibt es Streuung „geschenkt“, nämlich 65 reale Korpusse statt eines Bauplans.
- **Daten für die Stahlsaitengitarre:** Es fehlen offene Messdaten einer Dreadnought. Russell liefert nur Frequenzen. Ein Drei-Massen-Modell (Decke, Boden, Helmholtz) mit Russells 103/188/203 Hz als Ziel und gesetzten Holzwerten wäre machbar. Es braucht aber Güte und wirksame Masse aus einer eigenen Messung oder aus Hess 2013.
- **Bauplan → Korpus über die eigene Plattenbibliothek (FE, Reissner–Mindlin, Rippen):** technisch erreichbar. Q4 zeigt aber selbst für Decken einen Rest von etwa 1,5 % im Mittel und etwa 5 % maximal, und das erst *nach* Inversion von 35 Parametern. Ohne Messung zum Abgleich wird ein Bauplanmodell einzelne Gitarren nicht treffen, sondern eine plausible Gitarre erzeugen.
- **Luft im Korpus:** Für die vollständige Kopplung mit dem Luftraum genügt bei tiefen Frequenzen eine Helmholtz-Kopplung (Volumen, Hals). Eine 3D-Luft im FE-Modell (Größenordnung 4·10⁵ FG nach Q4) wäre ein Offline-Schritt, gefolgt von Reduktion auf einige hundert FG bzw. 30 Moden.
- **Abstrahlung ohne Schallwand:** Monopol für die tiefe Dreiergruppe von Resonanzen plus Dipole darüber ist nach Cardiff ein brauchbarer und billiger Ansatz. Die Rayleigh-Rechnung in Schallwand aus der Flügel-Bibliothek überschätzt die Monopolanteile unter etwa 300 Hz nicht grob, unterschlägt aber die gegenphasige Rückseite und den Boden.

**Kosten, grob:**

| Ausbaustufe | Aufwand |
|---|---|
| 2×2-Matrix + Mitschwingen | wenige Tage Rechenkern; Daten aus Mores-Fit |
| Drei-Massen-Modell Dreadnought | klein, aber ohne belegte Parameter |
| Bauplan-FE mit Luft und Reduktion | Wochen; Nutzen ohne Abgleich an Messung fraglich |

**Nächster Schritt mit Freigabe:** Die vier vom Werkzeug zwischengespeicherten PDFs (Pfade im Kopf) mit `quelle_suchen.py` lesen. Woodhouse 2004 und das Wood Handbook über einen anderen Zugang beschaffen.

# Recherche IC1: Bogen-Saite — Korpus, Dämpfer, Abstrahlung, Ensemble

> **Auftrag:** Klasse 3: Korpus der Streichinstrumente (Stärken der Signaturmoden der Violine, Viola, Cello, Güte des
> Kontrabasses), Dämpfer (con sordino), Abstrahlung, Streicher-Ensemble; aufbauend auf den Werten aus LumiViz S132
> (Jansson, Bissinger, Brown) und `gemeinsam/koerper.md`.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug] = vom
> Abrufwerkzeug als Zitat/Feldinhalt; [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (28 Suchen, 28 Abrufe; Acta Acustica 2025, UPF-Repositorium, KTH-QPSR Askenfelt
> 1982, Brown 2004, TNO nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Bericht IC, Klasse 3 „Bogen-Saite“: Korpus, Dämpfer, Abstrahlung, Ensemble

**Auftrag:** Vorab-Recherche zum Korpus der Streichinstrumente, zum Dämpfer (con sordino), zur Abstrahlung und zum Streicher-Ensemble.
**Datum:** 2026-10-26 (gilt auch als Abrufdatum aller Quellen)

**Reichweite:**
- **Umfang:** 28 Websuchen und 28 Abrufe. Englisch, Deutsch, Portugiesisch.
- **Orte:** Cambridge Apollo (Woodhouse), loria.fr (Elie), arXiv, MTG/UPF, Aalto, strad3d.org (Bissinger), euphonics.org, acoustics.org, UFMG-Repositorium, TNO-Repositorium, KTH.
- **Nicht erreichbar:**
  - Acta Acustica 2025 (Evangelista und Freire, Dämpfer): HTTP 403, als HTML und als PDF.
  - repositori.upf.edu: „Access Denied“.
  - KTH-QPSR-Datei für Askenfelt 1982 (`…/qpsr/1982/1982_23_4_149-174.pdf`): HTTP 500. Der Dateiname ist nach dem Archivmuster geraten.
  - Brown 2004: doublebassblog leitet weiter auf doublebasshq, dort 404.
  - IOA-Proceedings Gough „generic shell model“: 404.
  - UFMG-Konferenz-PDF auf musica.ufmg.br: Zertifikatsfehler.
  - TNO (Rasch 1979): Datei nur auf Anfrage per E-Mail.
- **Vom Werkzeug zwischengespeicherte PDFs**, alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793036751361-fekl15.pdf`: Woodhouse-Review, Preprint
  - `webfetch-1793036797807-8fm68o.pdf`: Elie, Gautier, David
  - `webfetch-1793036845657-j74tp1.pdf`: Malvermi u. a., arXiv
  - `webfetch-1793036888192-m6wx0g.pdf`: Zhang und Woodhouse
  - `webfetch-1793036935518-7i1bd3.pdf`: Bissinger 2006
  - `webfetch-1793037008270-ms9tyb.pdf`: Maestre, Scavone, Smith 2013
  - `webfetch-1793037041785-l2257s.pdf`: Pätynen und Lokki 2010
  - `webfetch-1793037140408-de0qvw.pdf`: Evangelista u. a., UFMG 2020
  - Dazu zwei Textdateien, die die Umgebung selbst aus meiner Konsolenausgabe angelegt hat: `b5vfofd0r.txt` und `bxzhwqviw.txt` im selben Ordner.
- **Wie ich gelesen habe:** Die PDFs habe ich mit `pdftotext` auf die Standardausgabe gelesen; ich selbst habe keine Datei geschrieben. Seitenangaben sind PDF-Seiten. Beim Woodhouse-Preprint stimmt die PDF-Seite mit der Kopfzeile „RepProgPhys violin NN“ überein, nicht mit der Paginierung der Zeitschrift.

**Kennzeichnung je Angabe:**
- [wörtlich, selbst gelesen, S. x] = im PDF selbst gelesen
- [wörtlich, Werkzeug] = vom Abrufwerkzeug als Zitat oder Feldinhalt geliefert
- [nur Zusammenfassung] = sonst; das ist **kein Beleg**

---

## Quellen

**Q1. Woodhouse, J. (2014): „The acoustics of the violin: a review“, Rep. Prog. Phys. 77, 115901.**
- Gelesen als Preprint „RepProgPhys violin … 13/8/14“, 96 Seiten.
- Fundort: https://www.repository.cam.ac.uk/handle/1810/245817, Datei OA-1506_Violin.pdf.
- Frei abrufbar, aber Rechtsvermerk im Datensatz: „this item's license is described as http://www.rioxx.net/licenses/all-rights-reserved“ [wörtlich, Werkzeug, Datensatzseite]. Also keine offene Lizenz.

**Q2. Zhang, A.; Woodhouse, J.: „Reliability of the input admittance of bowed-string instruments measured by the hammer method“, J Acoust Soc Am 136, DOI 10.1121/1.4900570.**
- Gelesen als Preprint „Dated: November 4, 2014“.
- Fundort: https://www.repository.cam.ac.uk/handle/1810/246474.
- Rechtsvermerk wie Q1: „rioxx … all-rights-reserved“ [wörtlich, Werkzeug].

**Q3. Elie, B.; Gautier, F.; David, B.: „Acoustic signature of violins based on bridge transfer mobility measurements“.**
- Veröffentlicht in JASA 136(3), 1385–1393 (2014). Diese Angabe stammt aus der Literaturliste von Q8, S. 13, nicht aus dem Preprint selbst.
- Fundort: https://members.loria.fr/BElie/papers/Elieetal_jasa2014.pdf (Autoren-Preprint, 9 Seiten).
- Im PDF steht kein Rechtsvermerk [selbst gelesen]. Rechte vermutlich bei ASA, nicht geprüft.

**Q4. Bissinger, G. (2006): „The violin bridge as filter“, JASA 120(1), 482–491.**
- Fundort: http://strad3d.org/files/bissinger/JASA%202006%20vn%20bridge%20as%20filter.pdf.
- Rechtsvermerk: „© 2006 Acoustical Society of America. DOI: 10.1121/1.2207576“ [wörtlich, selbst gelesen, S. 1]. Nicht frei lizenziert.

**Q5. Maestre, E.; Scavone, G.; Smith, J. (2013): „Digital Modeling of Bridge Driving-Point Admittances from Measurements on Violin-Family Instruments“, SMAC/SMC 2013 Stockholm.**
- Fundort: https://mtg.upf.edu/system/files/publications/smac_adm_2013_v3.pdf.
- Rechtsvermerk: „Copyright: c 2013 Esteban Maestre et al.“ und „Creative Commons Attribution 3.0 Unported License“ [wörtlich, selbst gelesen, S. 1]. **Frei, CC BY 3.0.**

**Q6. Malvermi, R.; Gonzalez, S.; Quintavalla, M.; Antonacci, F.; Sarti, A.; Torres, J. A.; Corradi, R. (2021): „Feature-based Representation for Violin Bridge Admittances“, arXiv:2103.14895.**
- Lizenzlink auf der Abstract-Seite: http://creativecommons.org/licenses/by/4.0/ [wörtlich, Werkzeug]. **CC BY 4.0.**

**Q7. Pätynen, J.; Lokki, T. (2010): „Directivities of Symphony Orchestra Instruments“, Acta Acustica united with Acustica 96, 138–167, DOI 10.3813/AAA.918265.**
- Fundort: https://users.aalto.fi/~ktlokki/Publs/patynen_aaua_2010.pdf.
- Rechtsvermerk: „© S. Hirzel Verlag · EAA“ [wörtlich, selbst gelesen, S. 1]. Selbstarchiviert, nicht offen lizenziert.

**Q8. Evangelista, M. V. da S.; Garcia, M. F.; Mota, D. A.; Garcia, S. F. (2020): „Estudo comparativo do efeito de três diferentes surdinas na LTAS de seis violinos“.**
- Konferenzbeitrag ANPPOM, Universidade Federal de Minas Gerais.
- Fundort: https://repositorio.ufmg.br/handle/1843/61960.
- Auf der Datensatzseite ist kein Rechtsvermerk genannt [Werkzeug]; im PDF habe ich auch keinen gesehen.

**Q9. euphonics.org, Abschnitt „5.3 Signature modes and formants“.**
- Fundort: https://euphonics.org/5-3-signature-modes-and-formants/.
- Lizenz: „licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International“ [wörtlich, Werkzeug].
- Autor laut Werkzeug nicht angegeben; vermutlich Woodhouse, nicht geprüft. Nur „ISSN 2977-5612“.

**Q10. Rossing, T. D.; Roberts, M.; Bynum, E.; Nickerson, L.: „Modal Analysis of Violins and Cellos“.**
- Vortrag beim 135. ASA-Treffen, „Presented … June 24, 1998“.
- Fundort: https://acoustics.org/pressroom/httpdocs/135th/rossing.htm.
- Kein Rechtsvermerk gesehen [Werkzeug].

**Nur über die Suche, nicht selbst gelesen:**
- Evangelista, M. V. da S.; Freire, S. (2025), Acta Acustica 9, 45, DOI 10.1051/aacus/2025029 (403).
- Rasch, R. A. (1979), „Synchronization in performed ensemble music“, Acustica 43(2), 121–131.
- Ternström, S. (1993), „Perceptual evaluations of voice scatter in unison choir sounds“, J. Voice.
- Chandna, Cuesta, Gómez (2020), arXiv:2009.09875: Lizenz CC BY 4.0 [wörtlich, Werkzeug]; der Inhalt nur [nur Zusammenfassung].
- Meyer, J. (1972), „Directivity of the bowed stringed instruments …“, JASA 51, 1994.
- Weinreich, G. (1997), „Directional tone color“, JASA 101, 2338–46. Zitat laut Q1, S. 66ff, Literaturliste [147].

---

## 1. Violine: Stärken der Moden, Bridge Hill

| Mode / Merkmal | Frequenz | Güte | Stärke | Quelle | Kennz. |
|---|---|---|---|---|---|
| CBR / B1− / B1+ (eine Violine, frühe Modalanalyse) | 436 / 454 / 538 Hz | — | — | Q1, Bildunterschriften | [wörtlich, selbst gelesen, S. 62] |
| A0 / CBR / B1− / B1+ (Beispielinstrument) | 272 / 407 / 462 / 551 Hz | — | — | Q9 | [wörtlich, Werkzeug] |
| Bridge Hill | „broader hump around 2–3 kHz“ | — | Spitzen „comparable to the high peaks of B1- and B1+“ | Q1 | [wörtlich, selbst gelesen, S. 32] |
| Bridge Hill | „peaking around semitone 40 (2.3 kHz)“ | im Text nicht genannt | „some 20 dB higher than … a guitar“ | Q9 | [wörtlich, Werkzeug] |
| Bridge Hill (12 Violinen) | „near 2.3 kHz“ | — | nur in Abbildungen; keine Korrelation mit der Qualität | Q4 | [wörtlich, selbst gelesen, S. 1] |
| „Vertical hill“ | „5-6 kHz range“ | — | — | Q1 | [wörtlich, selbst gelesen, S. 35] |
| Grenze der Modenüberlappung | Überlappungsfaktor > 1 „around 1–2 kHz“ | — | — | Q1 | [wörtlich, selbst gelesen, S. 20] |

**Ergebnis zur Lücke „Stärke je Mode“:** nicht geschlossen. Keine der gelesenen Quellen nennt die Stegadmittanz von A0, CBR, B1− und B1+ als Zahl im Text oder in einer Tabelle. Werte gibt es nur als Kurven:
- Q1, Abb. 19: Impedanzverhältnis in dB.
- Q4, Abb. 1, 5 und 7: Mobilität min/max für gute und schlechte Violinen.
- Q6, Abb. 2: Merkmalsmatrizen F, P, Q der ersten zehn Spitzen von 7 Cremoneser Violinen.
- Q3, Abb. 3: auf 0 dB normiert, also ohne Absolutwert.

**Fürs Modell brauchbar:**
- **Q5 (CC BY 3.0) gibt das Verfahren**, mit dem man Stärken aus offenen Messdaten selbst gewinnt. Die tiefen Moden werden einzeln angepasst (Frequenz und Halbwertsbreite durch Optimierung, Gewinn passiv durch semidefinite Programmierung). Den Bridge Hill modellieren sie als „a single, highly-damped resonance“ [wörtlich, selbst gelesen, S. 3], im zweidimensionalen Fall mit zwei Moden. Ordnung M = 18–27, genau zwischen 100 Hz und 6 kHz [selbst gelesen, S. 6]. Die Messdaten liegen nicht bei.
- **Q1 zum Hörbaren:** Erst eine Verschiebung einer tiefen Mode um „about a semitone (6%)“ wird wahrgenommen; auf Frequenz reagieren Hörer feiner als auf Amplitude oder Dämpfung [wörtlich, selbst gelesen, S. 42]. Das ist ein Maß dafür, wie genau Stärke und Güte überhaupt sein müssen.

## 2. Viola und Cello

| Instrument / Mode | Frequenz | Güte | Stärke | Quelle | Kennz. |
|---|---|---|---|---|---|
| Cello: größte tiefe Resonanz = Wolfton, „probably the B1… mode“* | 171,7–176,5 Hz | Q = 18,1 (Stahlrahmen), 18,5 (Schaumstoff), **13,4 (vom Spieler gehalten)** | **−25,0 bis −25,6 dB re 1 m/(N·s)**, also rund 0,055 m/(N·s) | Q2, Tab. II | [wörtlich, selbst gelesen, S. 9] |
| dieselbe Mode mit 1, 3 oder 4 Aufnehmern am Steg | 171,7 / 173,3 / 173,9 Hz | 18,8 / 26,2 / 24,8 | −25,2 / −24,4 / −25,4 dB | Q2, Tab. I | [wörtlich, selbst gelesen, S. 7] |
| Cello: Bridge Hill | „in the vicinity of 1 kHz“ | — | — | Q2 | [wörtlich, selbst gelesen, S. 3] |
| Cello: zwei Hills | „one around 1 kHz and the other around 2–2.3 kHz“ | — | — | Q9 | [wörtlich, Werkzeug] |
| Cello: Atmungsmode / T1 / C3 | „around 100 Hz“ / „around 140 Hz“ / „180 Hz“ | — | — | Q10 | [wörtlich, Werkzeug] |
| Cello A0 | „near 104 Hz“ | — | — | Q7, S. 25, zitiert Askenfelt [48] | [wörtlich, selbst gelesen], Sekundärzitat |
| Viola: Spitzen der Leistungsantwort gegenüber der Violine (290, 530, 1500, 3350 Hz) | „23, 31, 35, and 44 percent lower“ | — | — | Q7 | [wörtlich, selbst gelesen, S. 24] |
| Viola allgemein | Hauptresonanzen „20–40% lower than with the violin“ | — | — | Q7, S. 23, nach Fletcher und Rossing | [wörtlich, selbst gelesen] |

\* Das Zeichen hinter „B1“ ist im Text verloren gegangen. pdftotext hat die Tabellen verschoben ausgegeben; die Zuordnung von Zeile und Spalte habe ich nach der Reihenfolge rekonstruiert. Vor einer Übernahme an der PDF-Seite gegenprüfen.

Wiederholbarkeit derselben Messung über ein Jahr: Abweichungen bis „about 5 dB in amplitude and 5 Hz shift“ [wörtlich, selbst gelesen, Q2, S. 7].

**Wolfton, Mechanismus** (Q1, S. 11–12, selbst gelesen):
- Er tritt bei Viola und noch mehr beim Cello auf, weil die großen Instrumente „under-sized compared to a scaled-up violin“ sind [wörtlich]. Die Saiten sind schwerer, der Korpus dünner, die Kopplung zwischen beiden also stärker.
- Ablauf: Die Korpusresonanz schaukelt sich mit der Zeitkonstante ihrer Dämpfung auf, dadurch steigt die Mindest-Bogenkraft. Die Helmholtzbewegung kippt in Doppelgleiten (double slipping), der Korpus klingt ab, die Helmholtzbewegung kehrt zurück. Das ist der „warbling wolf“.
- Für das Modell heißt das: Der Wolfton entsteht in der Rechnung von selbst, wenn die Bogen-Saite an eine Korpusmode mit Q2-Werten gekoppelt ist (Q ≈ 13–26, |Y| ≈ −25 dB).

**Viola:** Für Signaturmoden mit Güte und Stärke habe ich keine begutachtete Quelle gefunden. Q5 hat die Viola gemessen, zeigt sie aber nur in Abb. 7.

## 3. Kontrabass

| Mode | Frequenz | Güte | Stärke | Quelle | Kennz. |
|---|---|---|---|---|---|
| A0 bei guten Bässen | „around 60 Hz“ | — | — | Q7, S. 28, zitiert Askenfelt | [wörtlich, selbst gelesen], Sekundärzitat |
| T1 | „approximately at 100 Hz“ | — | — | ebenda | ebenda |

- **Askenfelt 1982 bibliographisch:** Q7 zitiert ihn als „Dept. for Speech, Music and Hearing, Quarterly progress and status report 23 (1982) 149–174“ [wörtlich, selbst gelesen, S. 30]. Andere Listen führen ihn als Catgut Acoust. Soc. Newsletter No. 38, 34–39 [nur Zusammenfassung]. Vermutlich gibt es zwei Fassungen.
- **Erreichbar war er nicht.** Die Lücke „Güte des Kontrabasses“ bleibt offen. Kein Treffer ist kein Beweis: Im QPSR-Archiv kann er liegen; ich habe den Dateinamen geraten und bekam einen Serverfehler.

## 4. Abstrahlung und Richtcharakteristik

- **Violine:** unterhalb von 500 Hz „rather omnidirectional“, bei 2–6,3 kHz nach vorn [wörtlich, selbst gelesen, Q7, S. 21]. Nach Meyer rundstrahlend „up to 600 Hz within -10 dB“, bei 550–700 Hz zu beiden Seiten geteilt [wörtlich, selbst gelesen, Q7, S. 23, Sekundärzitat].
- **Cello:** unter 300 Hz in allen Richtungen über −6 dB; Bündelung bei 1250 Hz [selbst gelesen, Q7, S. 27].
- **Kontrabass:** die Richtcharakteristik ändert sich zum ersten Mal deutlich „around 200 Hz“ [wörtlich, selbst gelesen, Q7, S. 28].
- **Spieldynamik:** ändert die Richtcharakteristik kaum [selbst gelesen, Q7, S. 22].
- **Abfall der Abstrahlung:** etwa −15 dB/Oktave oberhalb von 3 kHz (Q7, S. 24, nach Fletcher und Rossing) [selbst gelesen].
- **Woodhouse zur Abstrahlung:** Oberhalb der Helmholtzzahl 1 („around 1 kHz for a violin body“) taugt die Multipol-Beschreibung nicht mehr [wörtlich, selbst gelesen, Q1, S. 21]. Weinreichs „directional tone colour“: Vibrato zusammen mit einer stark frequenzabhängigen Richtcharakteristik ergibt ein „scintillating“ Schallfeld [wörtlich, selbst gelesen, Q1, S. 36].
- **Bezug zu Weinreich bei Pätynen:** Gemeinsame Teiltöne verschiedener Noten strahlen ähnlich ab; Pätynen verbinden das mit Weinreich [selbst gelesen, Q7, S. 22]. Den Dipol unterhalb von A0, den Weinreich beschreibt, fanden sie mit Terzbandanalyse nicht [selbst gelesen, Q7, S. 23].
- **Was ein Modell braucht:** unter etwa 500–600 Hz eine Monopolquelle. Darüber Richtungsfilter je Frequenzband, die von Note zu Note springen. Messdaten dafür: 22 Mikrofone bei Q7. Ob deren Rohaufnahmen frei sind, habe ich nicht geprüft.

## 5. Dämpfer (Sordino)

| Dämpfer | Masse | Wirkung | Quelle | Kennz. |
|---|---|---|---|---|
| ein bestimmter Violindämpfer | „approximately weighs 1 g“ | gemittelte laterale Transfermobilität: < 1000 Hz „hardly modified“; 1000–2500 Hz um „5 to 13 dB“ tiefer; > 2500 Hz „around 2-5 dB“; die Spektralformante F2 (Bridge Hill) verschwindet | Q3 | [wörtlich, selbst gelesen, S. 7] |
| Gummi, rund, Mitte auf A/D | 1,85 g | „Efeito suave“, wirkt vor allem oberhalb von 1400 Hz | Q8 | [wörtlich, selbst gelesen, S. 4, S. 12] |
| Ebenholz, Gabelform | 5,70 g | starke Wirkung, mehr Energie in der ersten Oktave | Q8 | [wörtlich, selbst gelesen, S. 4, S. 12] |
| Übungsdämpfer, verchromtes Metall | 44,9 g | alle drei Dämpfer nehmen bei 1300–6000 Hz viel Energie weg; beim 44,9-g-Dämpfer verlagert sich keine Energie in die Tiefe | Q8 | [selbst gelesen, S. 11–12] |
| Übungsdämpfer über 35 g | > 35 g | keine Energieübertragung in die Tiefe | Evangelista und Freire 2025 | [nur Zusammenfassung] |

- **Mechanismus:** zusätzliche Masse am Steg; „its main action is to shift the bridge hill downwards“ [wörtlich, selbst gelesen, Q1, S. 34].
- **Nicht gefunden:** eine Verschiebung des Bridge Hill in Hz, ein Pegel des Übungsdämpfers in dB. Bei Q8 stehen die dB-Werte nur in Graph 6. Die Zeit zum Aufsetzen habe ich nirgends belegt gefunden.

## 6. Streicher-Ensemble

- **Einsatzzeit:** Asynchronie in Trios, gemessen als Standardabweichung der Einsatzdifferenzen, typisch 30–50 ms (Rasch 1979). Sie nimmt mit steigendem Tempo ab. [nur Zusammenfassung, Abstract aus zweiter Hand]
- **Tonhöhenstreuung, nur beim Chor gemessen:**
  - Ternström 1993: 14 Cent Standardabweichung „tolerable“, 0–5 Cent bevorzugt [nur Zusammenfassung].
  - Echte Chor-Mehrspuraufnahmen: 16–30 Cent [nur Zusammenfassung].
  - **Für Streichergruppen habe ich keine gemessene Streuung in Cent gefunden.**
- **Vibrato einzelner Spieler:** Rate 5–7 Hz (Spanne 4–10 Hz); Breite je nach Quelle 10–30 Cent oder im Mittel 63 Cent bei Solisten. Meyer 1992 („Zur klanglichen Wirkung des Streicher-Vibratos“, Acustica 76): das Vibrato moduliert auch die Amplitude und die Spektralhülle [alles nur Zusammenfassung]. Messungen der Unterschiede zwischen Spielern einer Gruppe: nicht gefunden.

---

## Nicht gefunden (kein Treffer ist kein Beweis)
- Violine: Stegadmittanz oder Abstrahlung von A0, CBR, B1− und B1+ als Zahl im Text oder in einer Tabelle; Güte des Bridge Hill.
- Viola: Moden mit Güte und Stärke aus einer begutachteten Quelle.
- Kontrabass: Güte und Dämpfung; Askenfelt 1982 im Volltext.
- Dämpfer: Hz-Verschiebung des Bridge Hill, dB-Wert des Übungsdämpfers, Zeit zum Aufsetzen.
- Ensemble: Stimmungsstreuung in Streichergruppen und Unterschiede im Vibrato zwischen Spielern; Meyer zum Chorwirkung in der Streichergruppe.
- Neue offen lizenzierte Admittanzdateien über die schon bekannten (CNSM u. a.) hinaus: keine gefunden.

## Eigene Einschätzung (kein Beleg)
- **Stärken für die Violine:** Die Lücke lässt sich wohl am saubersten schließen, wenn man die CNSM-Stegmobilitäten (cc-by-4.0) selbst mit dem Verfahren aus Q5 (CC BY 3.0) anpasst. Daraus kommen Frequenz, Halbwertsbreite und Gewinn je Mode, mit eigener Quelle für alle drei Werte. Für das Cello wäre Q2 die Quelle für Güte und Stärke der Wolfton-Mode, wenn der Rechtsstatus (all rights reserved) nur die Zahlen betrifft. Fakten sind in der Regel frei; das ist eine rechtliche Frage, die Patrik entscheiden sollte.
- **Spielerhaltung beim Cello:** Q2 zeigt, dass der Spieler die Güte senkt (13,4 statt etwa 18). Ein Modell sollte Messungen im Rahmen nicht ungeprüft als Spielzustand nehmen.
- **Ensemble:** N unabhängige Stimmen. Je Stimme eine eigene Verstimmung (vorläufig etwa 5–15 Cent Standardabweichung, unbelegt), ein Einsatzversatz (Standardabweichung 20–50 ms, nach Rasch), ein Vibrato (Rate 5–7 Hz, eigene Phase, leicht gestreute Rate) und ein **eigener Korpus**. Für den Korpus gibt es zwei Wege: verschiedene gemessene Instrumente, oder Modenfrequenzen, die oberhalb von etwa 1 kHz statistisch gestreut werden (Woodhouse: „statistical overlap“, Q1, S. 21). Ein bloßer Chorus auf einer Stimme würde den Weinreich-Effekt nicht abbilden.
- **Kosten:** Eine Modalfilterbank mit 20–30 Biquads braucht etwa 100–150 Multiplikationen und Additionen je Abtastwert und Stimme, also rund 5–7 Millionen Operationen pro Sekunde bei 48 kHz. Bei 16 Stimmen sind das etwa 0,1 Milliarden pro Sekunde; dazu kommen 16 Bogen-Saite-Modelle (Wellenleiter mit Reibung), beides ist billig. Teurer würde eine Raumfaltung je Stimme mit eigener Richtcharakteristik. Billiger wäre eine Gruppenrichtcharakteristik mit Dekorrelation je Stimme.

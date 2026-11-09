# Recherche IC1: Rohrblatt und Rohr — Rohrblatt, Bohrung, Einschwingen, Spielweisen, Blasdruck

> **Auftrag:** Klasse 8 (Klarinette, Saxophon, Oboe, Fagott): Rohrblatt-Kennlinie und -Parameter, Bohrungen und
> Tonlöcher, Zungenstoß und Legato, Spielweisen, Blasdruck je Instrument, Rechenzeit; aufbauend auf `gemeinsam/rohr.md`.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = Wortlaut/Feldinhalt, den das Werkzeug zitierte oder den der Agent per `pdftotext` las (mit
> Seite); [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (rund 26 Suchen, 28 Abrufe, 10 PDF-Auszüge; HAL-Volltexte, OpenWInD, Acta Acustica
> nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Klasse 8 „Rohrblatt und Rohr“ (Klarinette, Saxophon, Oboe, Fagott): Vorab-Recherche

## Reichweite
- **Umfang:** rund 26 Websuchen und 28 Abrufe (WebFetch, darunter 5 Abfragen der HAL-API). Dazu kamen 10 lokale Textauszüge mit `pdftotext` aus PDFs, die das Abrufwerkzeug selbst in den Cache gelegt hatte. Abrufdatum durchgehend 2026-10-26.
- **Sprachen der Suche:** Englisch, dazu einzelne französische Fachbegriffe („à la machoire“, „sur l'air“).
- **Durchsuchte Orte:** arXiv/ar5iv, HAL-API, UNSW (Wolfe-Gruppe), KTH (Fuks), DAFx-Archiv, DAEL/EuroAcoustics, QUB-Portal, mdw Wien, CCRMA (JOS) und das Repositorium der London Met.
- **Nicht erreichbar:**
  - OpenWInD-GitLab (Anubis „Access Denied“), daher kein Beispielverzeichnis gesehen.
  - HAL-Volltexte (Antwort nur „Oh noes!“).
  - acta-acustica.edpsciences.org (403), oro.open.ac.uk (403) und iris.unibs.it (403).
  - Bei van Walstijn & Avanzini 2007 (QUB-PDF) ist die Textebene durch die Schriftkodierung verstümmelt. Die Parametertabelle war deshalb nicht lesbar.
- **Gecachte Dateien:** Das Werkzeug hat die PDFs selbst abgelegt unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793033907983-r9fot6.pdf`: Li et al. 2016
  - `webfetch-1793034071048-aj9hoe.pdf`: Almeida et al. 2013
  - `webfetch-1793034026774-nz0lrj.pdf`: Chen et al. 2009, Acoustics Australia
  - `webfetch-1793034219778-nl7him.pdf`: Chen et al. 2009, JASA
  - `webfetch-1793034139147-jbs0rh.pdf`: Maestre/Scavone/Smith, DAFx-18
  - `webfetch-1793034259868-lwkb27.pdf`: Almeida, ISMRA 2016
  - `webfetch-1793034457118-b9mg2z.pdf`: Chatziioannou et al. 2019; Vermerk „personal use only“
  - `webfetch-1793033896128-xzet97.pdf`: Pàmies-Vilà et al., ISMA 2019
  - `webfetch-1793033895561-1i5x1y.pdf`: van Walstijn & Avanzini 2007, nicht lesbar
  - `webfetch-1793033858088-b9alzi.pdf`: Skouroupathis & Panagopoulos
  - `b3py3gwoy.txt` im selben Ordner: Ein zu langer grep-Auszug wurde vom Werkzeug ungefragt dort abgelegt. Er kann gelöscht werden.
- **Kennzeichnung:** **[wörtlich]** heißt: Wortlaut oder Feldinhalt, den das Werkzeug als Zitat zurückgab, oder Text, den ich per `pdftotext` selbst gelesen habe. Die Seite ist dann angegeben. Bei JASA-PDFs ist die gedruckte Seite aus „Pages: x–y“ und der PDF-Seite abgeleitet. **[nur Zusammenfassung]** heißt: Das Werkzeug hat geglättet.

---

## 1. Rohrblatt

### 1.1 Quasistatische Kennlinie (Einzelrohrblatt)
- **Dalmont, Gilbert, Ollivier 2003:** „Nonlinear characteristics of single-reed instruments: quasistatic volume flow and reed opening measurements“, J. Acoust. Soc. Am. 114(4), 2253–2262 [nur Zusammenfassung; Angabe aus Literaturlisten Dritter]. Volltext nicht gefunden. Zahlenwerte daraus also **nicht** belegt.
- **Einfachstes Modell mit zwei Größen:** Steife und Ruheöffnung. Der Fachartikel Acta Acustica 2025 (aacus220063, Open Access) sagt sinngemäß, beide reichten nicht aus, um andere Spieleigenschaften vorherzusagen [nur Zusammenfassung].
- **Kennlinie der Grundformel** (Silva et al. 2007, arXiv 0705.2803): „pM=KH“, mit H als „the height without any pressure difference“ und einer „rectangular aperture of width W“ [wörtlich, Abschnitt Model]. Zahlenwerte für K, H und W **stehen dort nicht**. Bekannt sind nur die dimensionslosen Werte „γ = 0.45“ und „ζ = 0.4“ (Bildunterschrift Fig. 2) [wörtlich].
- **Bloß angerissen:** Gazengel, Dalmont u. a. 2024 beschreiben einen quasistatischen Messaufbau für das Einzelrohrblatt-Mundstück (Δp gegen Strahlquerschnitt). Fundort FHNW-Repositorium (irf.fhnw.ch) [nur Zusammenfassung].

### 1.2 Dynamisches Rohrblatt: Werte mit Quelle
Die besten gefundenen Zahlen stammen aus **Chatziioannou, Schmutzhard, Pàmies-Vilà, Hofmann: „Investigating Clarinet Articulation Using a Physical Model and an Artificial Blowing Machine“**, Acta Acustica vol. 105 (2019), Heft 4, DOI 10.3813/AAA.919348.
- **Fundort:** DAEL. Das PDF trägt den Vermerk „Document downloaded by @DAEL … personal use only“ [wörtlich, Fußzeile jeder Seite]. Frei lesbar, aber nicht frei weitergebbar.

Table I, gedruckte S. 690 [wörtlich, Werte „initial / portato / staccato“]:

| Größe | Startwert | portato | staccato | Einheit |
|---|---|---|---|---|
| L (Rohrlänge) | 0.3294 / 0.2468 | 0.3343 | 0.2466 | m |
| m (Masse) | 3.25 | 8.62 | 7.16 | mg |
| k (Steife) | 1765 | 1308 | 1713 | N/m |
| (Dämpfung, Symbol im Text verloren) | 3000 | 8190 | 10569 | s⁻¹ |
| y_lay | 0.246 | 0.385 | 0.256 | mm |
| y_eq | 0.41 | 0.87 | 0.54 | mm |
| k_lay | 1.97×10⁶ | 6.16×10⁶ | 8.53×10⁶ | „N/m2“ (vermutlich N/m^α) |
| r_lay | 2 | 1.7 | 2.1 | s/m |
| k_tg (Zunge) | 3.16×10⁴ | 1.06×10⁴ | 1.14×10⁴ | „N/mtg“ (N/m^α_tg) |
| r_tg | 2 | 1.5 | 2.1 | s/m |
| max(y_tg) | 0.51 | 0.63 | 0.41 | mm |

- **Herkunft der Startwerte:** „from [6, 12, 23]“ [wörtlich, Fußnote b].
- **Viskosität:** „1.82 × 10-5 kg/(m·s)“ [wörtlich, S. 3 der PDF].
- **Bauart des Modells:** Anschlag an der Bahn und Zunge werden je über ein Kollisionspotential gerechnet. Die Berührung am Mundstück und die Zunge laufen über den Hunt-Crossley-Ansatz („Hunt-Crossley impac[t]“) [wörtlich, PDF-S. 4]. Das Modell ist Masse-Feder mit einem Freiheitsgrad. Die Autoren betonen, dass der ganze Komplex aus Rohrblatt, Lippe und Mundstück auf einen Oszillator abgebildet wird [nur Zusammenfassung von PDF-S. 10].

**Weitere Werte:**
- **Rohrblatt-Resonanz in Silva et al. 2007:** „ω_r = 2π×1500“ und „q_r = 0.4“ [wörtlich, Bildunterschrift Fig. 1].
- **Rohrblatt-Resonanz in Almeida (ISMRA 2016):** „the reed's characteristic frequency is set to 2500 Hz“ [wörtlich, PDF-S. 13]. Das ist eine Oboen-Simulation.
- **Anschlag-Steife:** „about 100 times the stiffness of the non-beating reed“ (Silva et al. 2007, Abschnitt Beating reed) [wörtlich].
- **Rohrblattmasse:** Smith (PASP, CCRMA): „the mass of the reed can be neglected to first order“ [wörtlich]. Rechtevermerk: „Copyright © 2026-08-21 by Julius O. Smith III“ [wörtlich].
- **Verteiltes Modell:** Avanzini & van Walstijn 2004 rechnen das Rohrblatt als Balken mit veränderlichem Querschnitt (PDE 4. Ordnung) und Bahn sowie Lippe als bedingte Kontaktkräfte. Das zugehörige konzentrierte Modell ist van Walstijn & Avanzini, Acta Acustica united with Acustica 93(3), 435–446 (2007): Die Steife je Fläche wird als Funktion des Drucks hergeleitet, Masse und Dämpfung kommen hinzu [nur Zusammenfassung]. Der QUB-Rechtevermerk lautet: „Copyright … is retained by the author(s) and / or other copyright owners“ [wörtlich, Deckblatt]. Die Zahlen daraus waren nicht lesbar.
- **Messung der Rohrblattparameter:** Taillard, Laloë, Gross, Dalmont, Kergomard 2014 (arXiv 1202.2114) haben 55 Rohrblätter holographisch gemessen. E_L und G_LT bestimmen die Eigenfrequenzen [nur Zusammenfassung].

### 1.3 Lippe und Ansatz als Parameter
- **Almeida, George, Smith, Wolfe:** „The clarinet: How blowing pressure, lip force, lip position and reed "hardness" affect pitch, sound level, and spectrum“, J. Acoust. Soc. Am. 134(3), 2247–2255 (2013), frei bei UNSW.
  - Die Abrisslinie (extinction) hat „a negative slope of a few square centimeters“ [wörtlich, S. 2247].
  - „High P and low F produce the highest sound levels and stronger higher harmonics“ [wörtlich, S. 2247].
  - Der Schwellendruck liegt bei „almost 4 down to 2 kPa as the lip force is increased“ [wörtlich, S. 2254].
- **Muñoz Arancón:** Laut Seminartext (CIRMMT) gibt es ein instrumentiertes Mundstück und ein iteratives Schätzverfahren für Ersatzparameter des Rohrblatts unter Spielbedingungen [nur Zusammenfassung]. Kein Paper mit Werten gefunden.
- **Chatziioannou & van Walstijn:** „Estimation of clarinet reed parameters by inverse modelling“, Acta Acustica united with Acustica 98(4) (2012) 629–639 [nur Zusammenfassung, aus einer Literaturliste]. Volltext nicht erreicht. Die Vorstufe „Inverting the Clarinet“ (DAFx-09, frei bei dafx.de) arbeitet zweistufig über Druck und Fluss im Mundstück [nur Zusammenfassung].
- **Ducasse 2003:** nur als Zitat in Li et al. 2016 gefunden („Ducasse, 2003“ unter den Modellen mit Zungenstoß) [wörtlich, S. 1089]. Inhalt nicht gefunden.
- **Neu (2026):** Tonetti & Piana (Univ. Brescia) schätzen Steife, Resonanz, Dämpfung und Ruheöffnung bei verschiedenen Lippenkräften (Hochgeschwindigkeitskamera, gemessene Impedanz als Randbedingung) [nur Zusammenfassung; AIP-Scilight und Suchauszug]. Vorstufe: Forum Acusticum 2025 (dael.euracoustics.org …/fa2025/…/000442.pdf). Volltext nicht gelesen.

### 1.4 Doppelrohrblatt (Oboe, Fagott): Ergänzung zu Almeida 2007
- **Fagott:** „typically 7 mm2 (against around 2 mm2 for the oboe)“, Ruheöffnung S₀ (Almeida, Vergez, Caussé, ar5iv physics/0607011, Abschn. III.2.1) [wörtlich].
- **Oboe:** p_M „near 33 kPa“ (III.2.1). Strommaximum gemessen zwischen p_M/4 und p_M/5 statt p_M/3 (IV.1). Druckrückgewinn C_p ≈ 0.8 (IV.2.1) [nur Zusammenfassung].
- **HAL-Metadaten zu Doppelrohrblättern** [wörtlich, Felder]:
  - Vergez, Almeida, Caussé, Rodet: „Simple physical model of double-reed musical instruments : influence of aero-dynamical losses in the embouchure …“, Acta Acustica united with Acustica, 2003, hal-00088314, openAccess false.
  - Guillemain: „A Digital Synthesis Model of Double-Reed Wind Instruments“, Eurasip Journal on Applied Signal Processing, 2004, hal-00091651, licence_s `https://about.hal.science/hal-authorisation-v1/`, openAccess true. Laut Abstract „real-time synthesis model for double-reed wind instruments“ [wörtlich].
  - Almeida, Vergez, Caussé, Rodet: „Physical study of double-reed instruments for application to sound-synthesis“, 2002, hal-01161426, openAccess true.
  - Grothe: „Investigation of bassoon embouchures with an artificial mouth“, 2012, hal-00811179, openAccess true. Inhalt: künstlicher Mund mit verstellbaren Lippen; Resonanz und Dämpfung des Rohrblatts aus der Impedanz; f₀ 58–620 Hz [nur Zusammenfassung des Abstracts]. Volltext blockiert.

### 1.5 Saxophon-Mundstück (Ersatzvolumen)
Quelle: Chen, Smith, Wolfe, „Saxophone Acoustics: Introducing a Compendium of Impedance and Sound Spectra“, Acoustics Australia 37(1), 18–23 (2009), frei bei UNSW.
- **Kegelstumpf beim Sopransaxophon:** „truncated at a diameter of 9.2 mm, and the missing cone of length ~150 mm is replaced by a mouthpiece with a volume of 2.25 cm3“ [wörtlich, S. 20].
- **Nachgiebigkeit des Rohrblatts:** „between 1.2 and 1.9 cm3“. Fehlender Kegel: „3.35 cm3“ [wörtlich, S. 20].
- **Innenlänge des Mundstücks:** „Lm … of 44 mm“ [wörtlich, S. 20].
- **Halbwinkel:** „1.74° and 1.52° for the soprano and tenor“. Oboe und Fagott: „0.71° and 0.41°“ [wörtlich, S. 18]. Längen: Sopran 710 mm, Tenor 1490 mm [wörtlich, S. 18].
- **Altsaxophon:** **nicht** gefunden.

---

## 2. Bohrung und Tonlöcher

### 2.1 Klarinette B♭: abgedruckte Tabelle (Fund)
**Debut, Kergomard, Laloë: „Analysis and optimisation of the tuning of the twelfths for a clarinet resonator“.** arXiv physics/0309051, Kommentar „28 June 2004 (submitted to Applied Acoustics)“ [wörtlich]. Erschienen in Applied Acoustics 66(4) (2005) 365–409 [nur Zusammenfassung, aus einer Literaturliste].
- **Tabelle:** Table 3 in Abschn. 3.5, Unterschrift „Numerical data concerning the Buffet Crampon clarinet investigated.“ [wörtlich].
- **Bezugspunkt:** laut Unterschrift „Distances are given from the reed tip“ [wörtlich, laut Werkzeug]. Dazu kommt ein Hinweis auf eine Lagekorrektur von etwa −12 mm [nur Zusammenfassung].
- **Rechte:** Die arXiv-Lizenz zeigt auf `http://arxiv.org/licenses/assumed-1991-2003/`. Dort steht: „arXiv.org has a perpetual, non-exclusive license to distribute this article.“ [wörtlich]. **Keine CC-Lizenz.** Zahlen sind als Fakten nutzbar, die Tabelle selbst ist nicht frei.
- **Vorbehalt:** Das Werkzeug gab die Werte nach der ar5iv-Extraktion **umformatiert** zurück („the pairings below follow the order in which the values appear“). Wörtlich bestätigt sind nur die Zeilen 24 (152.7/1.4/13.0), 12 (289.3/2.85/06.9) und 1 (543.7/5.8/04.4) sowie die Bohrungspunkte 0/0, 117.5/14.6 und 664/60. **Vor Übernahme am Original prüfen.**

Tonlöcher; Spalten ℓ, r, h in mm; Loch 24 ist das Registerloch. Die Paarung der Werte ist unsicher [nur Zusammenfassung]:

| Nr | ℓ | r | h | | Nr | ℓ | r | h |
|---|---|---|---|---|---|---|---|---|
| 24 | 152.7 | 1.4 | 13.0 | | 12 | 289.3 | 2.85 | 6.9 |
| 23 | 166.9 | 2.35 | 7.1 | | 11 | 308.2 | 3.95 | 7.4 |
| 22 | 193.4 | 2.8 | 7.1 | | 10 | 318.8 | 2.4 | 7.2 |
| 21 | 202.9 | 2.7 | 7.1 | | 9 | 348.9 | 3.55 | 6.5 |
| 20 | 213.2 | 2.55 | 7.2 | | 8 | 364.8 | 4.05 | 8.9 |
| 19 | 230.4 | 2.4 | 6.4 | | 7 | 369.5 | 3.95 | 6.1 |
| 18 | 238.9 | 3.9 | 10.8 | | 6 | 391.0 | 3.85 | 8.9 |
| 17 | 241.3 | 2.35 | 6.8 | | 5 | 413.1 | 4.85 | 8.9 |
| 16 | 252.2 | 2.45 | 8.8 | | 4 | 445.6 | 5.4 | 6.0 |
| 15 | 270.5 | 2.3 | 6.9 | | 3 | 472.9 | 6.1 | 5.0 |
| 14 | 284.3 | 3.3 | 8.8 | | 2 | 504.9 | 5.6 | 5.2 |
| 13 | 287.1 | 2.7 | 6.8 | | 1 | 543.7 | 5.8 | 4.4 |

Bohrung (Position in mm / Durchmesser in mm): 0/0 · 89/14.8 · 117.5/14.6 · 117.5/15.6 · 152.5/14.5 · 492.5/14.5 · 526/15.1 · 565/16.9 · 578.2/18.4 · 598/22.2 · 664/60.

**Weitere Klarinetten-Geometrie:**
- **Noreland et al.:** „The logical clarinet: numerical optimization of the geometry of woodwind instruments“, Acta Acustica united with Acustica 99 (2013) 615–628. arXiv 1209.3637, Lizenz `nonexclusive-distrib/1.0` [wörtlich].
  - **Keine Tabelle**, die Lochanordnung steht nur in Abbildungen.
  - Im Text: Bohrung 2a = 14.75 mm bzw. 14.27 mm (Prototyp); „in tune within 8 cents for a 29 notes range“ [wörtlich laut Werkzeug].

### 2.2 Saxophon, Oboe, Fagott
- **Saxophon:** Register-/Oktavloch „2 mm diameter and 6 mm deep on the soprano“ (Chen et al. 2009 AA, S. 22) [wörtlich]. Für das Altsaxophon gibt es Impedanzen je Griff, aber **keine Geometrie**:
  - Maestre, Scavone, Smith, DAFx-18, frei. Im PDF kein Lizenzvermerk gefunden.
  - MDPI Acoustics 7(1):15 (2025), Druck längs der Alt-Bohrung [nur Zusammenfassung].
- **Oboe:** keine freie Tabelle gefunden. Hinweis: Studie an fünf Howarth-S10-Oboen (Pulsreflektometrie, oro.open.ac.uk/23255, 403) [nur Zusammenfassung].
- **Fagott:**
  - Dart, „The baroque bassoon: form, construction, acoustics and playing qualities“, Doktorarbeit, London Metropolitan University, Oktober 2011. „Bore and tonehole dimensions of the bassoons studied are presented“ [wörtlich]. Zwei PDFs mit 100 MB und 124 MB, nicht abgerufen. Rechte: nur „© London Metropolitan University“ [wörtlich]. Gilt für das **Barock**fagott.
  - Petersen, Colinot, Silva, H.-Turcotte, „The bassoon tonehole lattice: Links between the open and closed holes and the radiated sound spectrum“, JASA 2021, hal-03365402, hal-authorisation-v1 [wörtlich, Felder]. Volltext blockiert, also offen, ob eine Geometrie abgedruckt ist.
  - Ernoult & Grothe, „Experimental study of the effects of the long chimney of a closed tonehole on the sound of a bassoon“, JASA 2023, hal-03789436, Lizenz `https://creativecommons.org/licenses/by/4.0/` [wörtlich]. Kamine „a few tens of millimeters long“, Kaminresonanzen nahe 2 kHz [wörtlich, Abstract]. Wichtig für den Fagott-Kern: lange geschlossene Kamine.
  - FHNW „fagottini“: Zenodo-Datensätze für **kleine** Fagotte [nur Zusammenfassung].

### 2.3 Tonlochmodell, Registerklappe, Gabelgriffe
- **Keefe:** „Theory of the single woodwind tone hole“, J. Acoust. Soc. Am. 72, 676–687 (1982); dazu „Experiments …“, 688–699 [nur Zusammenfassung, aus der Springer-Literaturliste]. Modell als symmetrisches T-Glied mit Reihen- und Querimpedanz (MUMT-618-Skript Scavone) [nur Zusammenfassung]. Die Formeln selbst wurden nicht gelesen.
- **Lefebvre/Scavone 2012:** Tonlochformeln, nur indirekt gefunden.
- **Dalmont/Lefebvre:** nicht gefunden. Die HAL-Abfrage nach Dalmont mit „side hole/tone hole“ ergab 0 Treffer. Das kann an der Abfrage liegen.
- **Registerloch der Klarinette:** Debut et al. zeigen, dass das Registerloch nicht allein die Duodezimen verstimmt. Offene Löcher, Konizität und Becher gleichen sich teilweise aus [nur Zusammenfassung].
- **Gabelgriffe:** nur die Flöte gefunden (Wolfe & Smith 2003, „Cut off frequencies and cross fingering …“). Für Rohrblattinstrumente nicht gefunden.

---

## 3. Einschwingen und Zungenstoß

**Li, Almeida, Smith, Wolfe:** „The effect of blowing pressure, lip force and tonguing on transients: A study using a clarinet-playing machine“, J. Acoust. Soc. Am. 140(2), 1089–1100 (2016), DOI 10.1121/1.4960594, frei bei UNSW.
- **Ablauf:** „After tongue release, the reed quickly comes to rest, losing its mechanical energy.“ [wörtlich, S. 1089].
- **Anstiegsraten:** „several tens to several hundreds of dB s−1“ [wörtlich, S. 1089].
  - Höchstwert: „maximum rate of 730 dB s−1“ bei F_lip = 0,5 N und etwa 5 kPa [wörtlich, S. 1093].
  - Bei F_lip über 2,5 N: „r is always less than 240 dB s−1“ [wörtlich, S. 1094].
- **Einsatzzeit:** t200Pa ist „77, 41, and 32 ms“ für verschiedene Zungenkräfte und Anfangsauslenkungen [wörtlich, S. 1092]. Der Rohrblattkanal öffnet sich „in about 8 ms“ [wörtlich, S. 1093].
- **Zungenkraft und Beschleunigung:** Zungenkraft 0,05–0,15 N, Beschleunigung „2.51 ± 0.12 m s−2“ [wörtlich]. Beide haben kaum Einfluss auf r, verkürzen aber die Einsatzzeit und heben die 3. Harmonische [wörtlich, Abstract S. 1089].
- **Hysterese:** Im Bereich „(2.14–2.36 kPa)“ lassen sich Töne nur mit Zungenstoß anspielen [wörtlich, S. 1098].
- **Arbeitspunkt:** P = 4,6 kPa, F_lip = 1,5 N [wörtlich].

**UNSW-Seite „Clarinet (and saxophone) articulations“** (newt.phys.unsw.edu.au/jw/articulations.html):
- „475±20 dB.s−1“ [wörtlich]; der Kanal öffnet „in about 7 milliseconds“ [wörtlich].
- Rechte: „© Joe Wolfe / J.Wolfe@unsw.edu.au“ [wörtlich; das ©-Zeichen ist auf der Seite verstümmelt].

**Modelle mit Zunge:**
- Chatziioannou et al. 2019 (siehe 1.2): Zungenbahn stückweise linear über t1…t4, außerhalb „-2 × 10-5 m“ [wörtlich, PDF-S. 9].
- Mensch-Daten: Pàmies-Vilà, Hofmann, Chatziioannou, „The player-reed interaction during note transitions in the clarinet“, ISMA 2019, S. 320 ff., frei bei mdw. Untersucht wurden 11 Spieler; Zunge und Blasdruck werden je nach Dynamik und Artikulation (legato/staccato) angepasst [nur Zusammenfassung].

**Legato und Griffwechsel (zeitveränderliches Rohr):**
- **Maestre, Scavone, Smith, DAFx-18 (Aveiro):** „Joint modeling of impedance and radiation as a recursive parallel filter structure …“, am **Altsaxophon** gemessen.
  - Modale Resonatorbank je Griff mit **linearer Überblendung** beim Wechsel: „a simple model based on linear interpolation of input impedance and sound pressure radiation models“ [wörtlich, S. 1].
  - Begründung: Griffwechsel seien langsam genug, also „quasi-static“ [wörtlich, S. 6]. Die Passivität bleibt für alle Gewichte erhalten [nur Zusammenfassung, S. 6].
  - Fig. 4 zeigt „A linear mixing of 5 steps“ von E5 nach B♭4 [wörtlich, S. 7].
- **Szwarcberg, Colinot, Vergez et al. 2025** („Register jumps on the clarinet …“, arXiv 2506.03875, Lizenz nonexclusive-distrib 1.0):
  - Rechnet mit Wellenleiter, nicht modal: „the hole is instantaneously opened“ (Abschn. 4.2) [wörtlich laut Werkzeug].
  - Ob der Registerwechsel gelingt, hängt von der Phase ab, zu der das Loch geöffnet wird: „depends on the phase at which the hole is opened“ [wörtlich, Abstract].
  - Im Versuch dauert das Öffnen 30–50 ms [nur Zusammenfassung]. Das modale Modell nennen sie „more resource-intensive“ [wörtlich, Abschn. 5.3.3].
- **Tonloch im Zwischenzustand:**
  - Scavone & Cook 1998: Tonloch als Wellenleiter, stufenlos von offen bis zu [nur Zusammenfassung].
  - van Walstijn & Scavone: „The Wave Digital Tonehole Model“, Zustand während der Rechnung verstellbar [nur Zusammenfassung].
- **Guillemain, Kergomard, Voinier 2005:** HAL hal-00009254, hal-authorisation-v1, openAccess true [wörtlich, Felder]. Der Abstract erwähnt Griffwechsel **nicht**. Eine Suchauszugs-Angabe zu „transitions between fingerings“ ist unbestätigt.

---

## 4. Spielweisen

- **Vibrato:**
  - Gilbert, Terroir, Simon: „Vibrato of saxophones“, JASA 2005, hal-00474666 [wörtlich, Felder]. Unterscheidet Vibrato „à la machoire“ (Kiefer) und „sur l'air“ (Luft) [wörtlich]. Keine Zahlen im Abstract.
  - Almeida, ISMRA 2016 („Non-stationary tones in reed instruments: A sequence of stationary states?“, frei bei UNSW): Oboe mit Vibrato bei 5 Hz um einen Mittelwert von „6.5 kPa“ [wörtlich, PDF-S. 6]. Abweichung vom quasistatischen Bild: „up to 0.5% for frequency values and up to 10-20% for the values of the harmonic amplitude“ [wörtlich, PDF-S. 13].
  - Fuks (KTH): Oboe-Vibrato mit Druckmodulation von etwa 10–25 % [nur Zusammenfassung].
  - Rate und Tiefe allgemein (4–12 Hz) nur aus Überblicksquellen, nicht instrumentspezifisch [nur Zusammenfassung]. Dass die Klarinette meist ohne Vibrato gespielt wird: **kein Beleg gefunden**.
- **Bend und Glissando:** Chen, Smith, Wolfe, „Pitch bending and glissandi on the clarinet: Roles of the vocal tract and partial tone hole closure“, J. Acoust. Soc. Am. 126(3), 1511–1520 (2009).
  - Halb abgedecktes Tonloch hebt die Impedanzspitzen. Im 2. Register und darüber bilden die Spieler Resonanzen im Vokaltrakt, die vergleichbar groß sind wie die der Bohrung. „much easier to bend notes down than up“ [wörtlich, S. 1511].
  - Bend um „400 cents or a major third“ bei Griff C6 [wörtlich, S. 1519].
  - Bohrungsspitzen: etwa 90 MPa s m⁻³ (1. Register) und „40–50 MPa s m-3“ (2. Register) [wörtlich, S. 1518].
  - Ab Trakt-Spitzen über „20–25 MPa s m-3“ folgt der Ton dem Trakt [wörtlich, S. 1518]. Untere Grenze für große Bends etwa „D5 587 Hz“ [wörtlich, S. 1518].
  - Gershwin: Finger gleiten über offene Löcher, gleichzeitig wird der Trakt nachgeführt [wörtlich, Abstract].
- **Saxophon-Altissimo:** Chen et al., Science 2008, „Experienced saxophonists learn to tune their vocal tracts“ [nur Zusammenfassung]. Die Seitenangabe (726 oder 776) ist widersprüchlich.
- **Atem- und Luftgeräusch:**
  - Smith (PASP): „Possibly the most important neglected phenomenon in this model is sound generation due to turbulence of the flow, especially near reed closure.“ [wörtlich]
  - „Practical synthesis models have always included a noise component of some sort which is modulated by the reed“ [wörtlich]. Messgrundlage laut Smith dünn.
  - Kein Messmodell für Turbulenzrauschen am Rohrblatt gefunden.
- **Multiphonics:**
  - Doc, Vergez, Missoum, „A minimal model of a single-reed instrument producing quasi-periodic sounds“, Acta Acustica 2014 [nur Zusammenfassung].
  - Taillard, Kergomard, Laloë 2010 (Iterated maps, Nonlinear Dynamics) behandeln Regime, nicht Klangfarbe [nur Zusammenfassung].
- **Flatterzunge, Growl, Subtone:** **keine** physikalische Messung gefunden. Für den Growl gibt es nur die Kopplung mit dem Vokaltrakt (Chen et al.). Zur Subtone-Nähe nur eine Klarinetten-Modellstudie: Lippendämpfung sei entscheidend, damit das Rohrblatt auf der tiefsten Resonanz schwingt [nur Zusammenfassung, Fundort uantwerpen.be].

---

## 5. Expression und Anregung (Blasdruck)

**Fuks & Sundberg 1996**, KTH TMH-QPSR 3/1996, 41–57; überarbeitet als „Blowing pressures in bassoon, clarinet, oboe and saxophone“, Acustica/Acta acustica [wörtlich, Zitierzeile der KTH-Seite]. Kein Rechtevermerk auf der Seite. Werte in cm H₂O; 10 cm H₂O ≈ 1 kPa:

| Instrument | Bereich [wörtlich] | ≈ kPa (eigene Umrechnung) |
|---|---|---|
| Klarinette | „20 to 55 cm H2O“ | 2,0–5,4 |
| Altsax | Sx1 „15 to 80“, Sx2 bis „55 cm H2O“ | 1,5–7,8 |
| Oboe | „35 to 95 … and from 41 to 124 cm H2O“ | 3,4–12,2 |
| Fagott | „12 to 90 … and from 13 to 52 cm H2O“ | 1,2–8,8 |

- **pp/ff-Mittel je Ton** stehen nur im Anhang. Die Lesung des Werkzeugs (z. B. Oboe Ob2 ff 50–120, pp 39–51) ist [nur Zusammenfassung].
- **Abhängigkeit von der Tonhöhe:** „In the double reed instruments, pressure tends to be increased continuously with pitch.“ [wörtlich]. Klarinette eher fallend mit Spitze am ersten überblasenen Ton; Saxophon mit Spitze um Eb4–G4 [nur Zusammenfassung].
- **Schwelle und Abriss an der Klarinette:**
  - Schwelle 2–4 kPa je nach Lippenkraft (Almeida et al. 2013, S. 2254) [wörtlich]. Dalmont & Frappé fanden „approximately 3 kPa“ [wörtlich, ebd.].
  - Abriss bei kleiner Lippenkraft über 6,7 kPa [wörtlich, S. 2250].
- **Klangfarbe und Pegel:** „a very strong correlation between sound level and spectral centroid“ [wörtlich, Almeida et al. 2013, S. 2252]. Höchster Pegel und hellster Klang bei hohem P und kleinem F [wörtlich, S. 2247].
- **Doppelrohrblatt:** p_M der Oboe etwa 33 kPa (Almeida 2007). Das liegt weit über dem Spieldruck: Das Rohrblatt schlägt dort nicht wie bei der Klarinette durch Druck allein zu [eigene Einordnung].

---

## 6. Rechenzeit (neu gefunden)
- **Maestre et al. DAFx-18:** „an average of 32 resonators per fingering, a sampling frequency of 48 kHz“, mit Wechseln in etwa 10 % der Zeit: „above 30 times faster than real-time in one core of a laptop computer“ [wörtlich, S. 7].
- **Chatziioannou et al. 2019:** „runs faster than real time on a standard desktop computer (Intel® Xeon(R) CPU E51650; 16Gb RAM)“ [wörtlich, gedr. S. 692 bzw. PDF-S. 11]. Energieerhaltendes, unbedingt stabiles Verfahren mit Zunge und Bahnanschlag.
- **Silva et al. 2007 (modal, Rohrblatt-Dynamik):** „definitely not intended for sound synthesis“ [wörtlich, Conclusion].

---

## Nicht gefunden
- Volltexte und Zahlen von Dalmont/Gilbert/Ollivier 2003, Chatziioannou & van Walstijn 2012, Ducasse 2003 und Muñoz Arancón.
- Die Tabelle von van Walstijn & Avanzini 2007 (Text nicht lesbar).
- Freie **Bohrungs- und Tonlochtabellen** für Altsaxophon, moderne Oboe und modernes Fagott. Für das Fagott nur Barock (Dart 2011) und kleine Fagotte (FHNW).
- OpenWInD-Beispieldateien (GitLab blockiert).
- Dalmont/Lefebvre-Tonlochmessungen und die Keefe-Formeln im Wortlaut.
- Gabelgriffe an Rohrblattinstrumenten.
- Messungen zu Flatterzunge, Growl und Subtone.
- Ein Rauschmodell mit Messgrundlage.
- Instrumentspezifische Vibrato-Rate und -Tiefe; ein Beleg für „Klarinette meist ohne Vibrato“.
- Die Griffwechsel-Methode bei Guillemain 2005.

Kein Treffer ist kein Beweis.

## Eigene Einschätzung (kein Beleg)
- **Benötigte Größen je Instrument und Herkunft:**
  - Ruheöffnung H, Steife je Fläche K (oder p_M = KH), Breite W, Resonanz f_r und Güte q_r, Anschlag-Steife, Lippenkraft (verschiebt H und K), Ersatzvolumen des Mundstücks, Blasdruckbereich.
  - **Klarinette:** gut belegt (Chatziioannou 2019 Table I, Li 2016, Almeida 2013, Debut-Tabelle).
  - **Saxophon:** Mundstück für das Sopran belegt, das Alt muss abgeleitet werden (Kegel mit Halbwinkel und Mundstücksvolumen ≈ fehlender Kegel).
  - **Oboe und Fagott:** Kennlinie belegt (Almeida 2007, S₀ des Fagotts 7 mm²). Dynamische Werte und Geometrie fehlen; Volltexte von Grothe 2012 und Petersen 2021 bei HAL wären der nächste Schritt.
- **Aufwand und Risiko:**
  - Klarinette gering; Saxophon mittel (Alt-Geometrie fehlt, starke Kopplung mit dem Vokaltrakt bei Bend und Altissimo).
  - Oboe mittel bis hoch (Druckrückgewinn, feuchte Hysterese, hohe Spieldrücke).
  - Fagott hoch (lange Kamine mit Resonanzen nahe 2 kHz, Halbloch-Technik, keine moderne Geometrie).
- **Kippt Legato den modalen Kern? Nein.** Maestre et al. zeigen genau den Weg „Moden je Griff + lineare Überblendung zweier Bänke“, passiv und schnell. Das passt zu „Spur und Kurve stehen im Namen des Tons“: Ein Übergang ist ein eigener, vorab gerechneter Ton.
  - **Grenze 1, Registerwechsel:** Hier ist das Ergebnis phasenabhängig und bistabil (Szwarcberg 2025). Eine Überblendung kann ein anderes Regime treffen als ein echtes Loch. Das ist für einen deterministisch vorab gerechneten Ton beherrschbar, aber zu prüfen.
  - **Grenze 2, Halbloch-Glissando:** Das Glissando mit halb offenem Loch (Gershwin) ist eine stetige Geometrieänderung. Ein Raster von Zwischenmoden (z. B. Lochradius gestuft) ginge auch modal; FDTD wäre nur für den Deluxe-Pfad nötig.
- **Widersprüche zum Bestand:**
  - „Bend über Pitch“: In der Praxis kommt er aus Vokaltrakt und Lippe, nicht aus dem Rohr. Ein Bend nach oben ist physikalisch eng, nach unten bis etwa eine große Terz, und auf der Klarinette erst ab etwa D5 groß. Mit einem Trakt-Resonator in Reihe am Eingang ließe sich das treu rechnen.
  - Expression = Blasdruck allein ist zu wenig: Pegel und Klangfarbe hängen gleichrangig von der Lippenkraft ab. Expression sollte daher auf einer Kurve in der (P, F)-Ebene laufen.
  - Klarinette und Oboe brauchen mit steigender Tonhöhe verschiedene Druckbereiche. Expression muss also je Ton auf kPa skaliert werden.
  - Atemgeräusch ist ohne belastbares physikalisches Modell; vorerst bleibt nur ein modulierter Rauschanteil (STK-Praxis).

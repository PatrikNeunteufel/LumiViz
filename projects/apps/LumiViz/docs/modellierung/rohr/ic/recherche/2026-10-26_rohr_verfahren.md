# Recherche IC1: ein Rohrmodell für Lippe, Rohrblatt und Luftstrahl — Verfahren, Kosten, Daten

> **Auftrag:** Verfahren für das Rohr (Wellenleiter, Reflexionsfunktion, modale Impedanz, FDTD, TMM) und ihre Kosten;
> was vorab aus der Bohrung rechenbar ist; gemeinsamer Kern und Schnittstelle Rohr–Anreger; Anreger kurz;
> Abstrahlung und nichtlineare Ausbreitung; freie Bohrungen und Impedanzen; Rechenzeit.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = Wortlaut/Feldinhalt, den das Werkzeug zitierend zurückgab; [wörtlich, selbst gelesen, S. x] =
> der Agent hat die PDF-Seite selbst gelesen; [nur Zusammenfassung] = sonst (kein Beleg).
> **Reichweite:** im Bericht selbst (19 Suchen, 26 Abrufe, drei PDFs seitenweise; nur Englisch; HAL-Volltexte,
> OpenWInD-GitLab, Modalys-Doku gesperrt).
> **Von der Sitzung selbst nachgelesen:** siehe `../rohr.md` (je Angabe vermerkt); was dort nicht als „nachgelesen“
> steht, gilt als „nur Bericht“.
> **Eingeflossen in:** `../rohr.md`.
> **Anmerkung der Sitzung:** Die drei PDFs, die WebFetch selbsttätig zwischengespeichert hat (Ernoult et al. 2023,
> Harrison et al. 2015, Bilbao 2008), liegen im Cache der Claude-Sitzung, nicht in diesem Repo.

---

# Ein Rohrmodell für drei Anregungen (Lippe, Rohrblatt, Luftstrahl): Recherchebericht

Abrufdatum aller Online-Angaben: **2026-10-26**.

## Reichweite

- **Umfang:** 19 Websuchen und 26 Abrufe (WebFetch). Dazu kommen 3 PDFs, die ich selbst seitenweise als Bild gelesen habe (siehe Hinweis unten). Das sind etwa 45 Vorgänge, also etwas mehr als die vorgesehenen 25–40. Grund: Viele Abrufe sind gescheitert.
- **Sprachen:** Gesucht habe ich auf Englisch. Französische Seiten (IRCAM-Forum) habe ich gelesen.
- **Quellen:**
  - Websuche (nur US-Index)
  - HAL, über die API `api.archives-ouvertes.fr`. Die HAL-Webseiten und `hal.science/.../document` sperrt ein Bot-Schutz (Anubis). Volltexte von HAL waren deshalb **nicht** erreichbar, nur Metadaten und Abstract-Felder.
  - Zenodo-API, arXiv, PyPI, DAFx-Archiv, UNSW-Seiten, GitHub (STK), IRCAM-Forum, ANR
- **Suchbegriffe (Auswahl):**
  - OpenWInD temporal simulation reed lips
  - Moreesc
  - Modalys license
  - UNSW clarinet impedance database
  - zenodo wind instrument impedance / bore
  - Harrison Bilbao Perry valved brass FDTD
  - Guillemain Kergomard Voinier real-time
  - Auvray Fabre jet-drive
  - brassiness nonlinear propagation
  - Adachi Sato lip
  - Almeida Vergez Caussé double reed
  - organ flue pipe scaling dataset
- **Nicht erreichbar:**
  - gitlab.inria.fr/openwind (Anubis „Access Denied“)
  - HAL-Volltexte (Anubis)
  - Modalys-Dokumentation `support.ircam.fr/docs/Modalys/beta/` (HTTP 403)
- **Hinweis zur Regel „nichts speichern“:** WebFetch hat drei PDFs **ohne mein Zutun** lokal zwischengespeichert. Ich habe sie mit Read gelesen, weil WebFetch sie nicht entziffern konnte. Die Dateien liegen unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793015639285-fu5ded.pdf` (Ernoult et al. 2023)
  - `webfetch-1793015688569-8dvfwj.pdf` (Harrison et al. 2015)
  - `webfetch-1793016050068-faimme.pdf` (Bilbao 2008)

  Ich habe sie **nicht** gelöscht. Patrik möge entscheiden.
- **Kennzeichnung:**
  - **[wörtlich]** heißt: Das Werkzeug hat den Text als Zitat bzw. Feldinhalt zurückgegeben.
  - **[wörtlich, selbst gelesen, S. x]** heißt: Ich habe die PDF-Seite selbst als Bild gelesen. Das ist die stärkste Belegform in diesem Bericht.
  - **[nur Zusammenfassung]** heißt: nur aus einer Werkzeug-Zusammenfassung, kein Beleg.

---

## 1. Verfahren für das Rohr und ihre Kosten

### 1a. Was vorab aus der Bohrung rechenbar ist und was im Zeitbereich laufen muss

**Vorab im Frequenzbereich rechenbar:** Eingangsimpedanz, Moden (Pole/Residuen), Reflexionsfunktion.

- **OpenWInD** rechnet die Impedanz aus der Geometrie.
  - Zitat: „It provides the computation of the acoustic response in the frequency domain“ [wörtlich, selbst gelesen, Ernoult/Cabaret/Chabassier 2023, S. 4873, Abstract]
  - Die Geometrie umfasst „(main bore, side holes, valves)“ [wörtlich, ebd.].
  - Moden lassen sich direkt aus der FEM-Diskretisierung gewinnen: „computing directly the modal parameters from the FEM discretisation“ [wörtlich, selbst gelesen, S. 4874, §2.1]. Ergebnis sind Frequenz, Gütefaktor und Betrag der Resonanzen (ebd.). Das gilt aber nur „With some constraints on the radiation models“ [wörtlich, ebd.].
  - Zugehörige Arbeit: R. Auvray, J. Chabassier, „Direct computation of modal parameters for musical wind instruments“, *Journal of Sound and Vibration* 528, 116775, 2022, DOI 10.1016/j.jsv.2022.116775, HAL hal-03613608 [wörtlich, HAL-API-Felder]. HAL meldet `openAccess_bool: false`, also **nicht frei zugänglich**.
  - Abweichende Autorenreihenfolge: In der Literaturliste von Ernoult et al. 2023 steht „J. Chabassier and R. Auvray“ [selbst gelesen, S. 4876, Ref. 11]. HAL nennt Auvray zuerst.

**Zwingend im Zeitbereich:** die nichtlineare Kopplung Anreger–Rohr, also die Bernoulli-Strömung am Rohrblatt bzw. an der Lippe und der Strahl an der Kante.

- Bei Guillemain et al. 2005 heißt es, das physikalische Modell „describes the nonlinear coupling between the resonator and the excitor through the Bernoulli equation“ [nur Zusammenfassung, Suchergebnis zu HAL hal-00009254].

### 1b. Die Verfahren im Einzelnen

**Übertragungsmatrizen (TMM) und 1D-FEM für die Impedanz (vorab, Frequenzbereich)**

- Quelle: R. Tournemenne, J. Chabassier, „A comparison of a one-dimensional finite element method and the transfer matrix method for the computation of wind music instrument impedance“, *Acta Acustica united with Acustica* 105, 838, 2019, DOI 10.3813/AAA.919364, HAL hal-01963674, `openAccess_bool: true` [wörtlich, HAL-API].
- Bandangabe uneinheitlich: Ernoult et al. 2023 zitieren „vol. 5, p. 838“ [selbst gelesen, S. 4876, Ref. 4]. HAL nennt 105.
- Aussagen aus dem Abstract:
  - „the TMM is more efficient in lossless cases and for lossy cylinders“ [wörtlich, HAL-Abstract]
  - „the FEM is shown to be more efficient when targeting a specific precision“ [wörtlich, ebd.], gemeint ist die verlustbehaftete Trompete.
- Kosten: Der Abstract nennt nur, dass die Rechenzeiten verglichen werden, aber keine Zahlen.
- Für das Projekt unkritisch, weil einmalig vorab gerechnet (eigene Einschätzung, siehe unten).

**Digitaler Wellenleiter (digital waveguide, Smith; STK)**

- STK-Klassen:
  - Brass: „a simple brass instrument waveguide model, a la Cook“ [wörtlich, STK-Doku `classstk_1_1Brass.html`]
  - Flute: „a simple flute physical model, as discussed by Karjalainen, Smith“ [wörtlich] und „The jet model uses a polynomial, a la Cook.“ [wörtlich, `classstk_1_1Flute.html`]
  - BlowHole: „a two-port register hole and a three-port dynamic tonehole“ [wörtlich, `classstk_1_1BlowHole.html`]
- Grenze bei BlowHole: Die Abstände zwischen Rohrblatt/Registerloch und Tonloch/Schallstück sind fest [nur Zusammenfassung]. Wellenleiter sind also für **feste** Geometrien gebaut.
- Bilbao 2008 zu Wellenleitern: Sie bringen ihren Effizienzvorteil vor allem bei Zylinder und Kegel, „in which case they yield an extreme efficiency advantage“ [wörtlich, selbst gelesen, Bilbao DAFx-08, S. DAFX-1].
- Patenthinweis in der STK-Doku: „possibly subject to patents held by Stanford University, Yamaha“ [wörtlich, Brass- und Flute-Seite].
- Kosten: In den Quellen keine Zahl gefunden, siehe Abschnitt 6.

**Reflexionsfunktion (McIntyre, Schumacher & Woodhouse 1983)**

- Literatur: „This review paper appeared in the Journal of the Acoustical Society of America, vol. 74, pages 1325-1345 (1983)“ [wörtlich, Seite von M. E. McIntyre, damtp.cam.ac.uk/user/mem2/papers/MUS-ACOUST/].
- Dort ist ein Reprint verlinkt mit „copyright © 1983 American Acoustical Society“ [wörtlich]. Frei lesbar, aber urheberrechtlich geschützt.
- Inhalt (Faltung des Ausgangsflusses mit der Reflexionsfunktion) habe ich **nicht selbst gelesen**. Kosten pro Sample wachsen mit der Länge der Reflexionsfunktion (eigene Einschätzung).

**Modale Darstellung bzw. digitale Impedanz (Guillemain, Kergomard, Silva u. a.)**

- Quelle: P. Guillemain, J. Kergomard, Thierry Voinier, „Real-time synthesis of clarinet-like instruments using digital impedance models“, *JASA* 118, 483–494, 2005, DOI 10.1121/1.1937507, HAL hal-00009254, open access [wörtlich, HAL-API].
  - Der Vorname „Thierry“ stammt aus dem HAL-Feld. Eine Suchzusammenfassung nannte „Thomas“.
  - Abstract: „A real-time synthesis model of wind instruments sounds“ [wörtlich] und „uses sampled versions of the physical variables“ [wörtlich].
- **Moreesc** (modaler Rahmen für Rohrblatt **und** Lippe): F. Silva, C. Vergez, P. Guillemain, J. Kergomard, V. Debut, „Moreesc: A framework for the simulation and analysis of sound production in reed and brass instruments“, *Acta Acust united Ac* 100, 126–138, 2014 [wörtlich, selbst gelesen als Ref. 1 bei Harrison et al. 2015, S. DAFX-1/8]. Eine Bezugsquelle oder Lizenz von Moreesc habe ich **nicht gefunden**.

**Finite Differenzen / FDTD (Bilbao, Harrison)**

- Bilbao 2008, Rechenaufwand:
  - Gesamtzahl der Operationen „6f_s²/γ = 6Lf_s²/c operations/second“ [wörtlich, selbst gelesen, Bilbao DAFx-08, S. DAFX-5, §3.5].
  - Speicherbedarf etwa 2 f_s L / c Einheiten [selbst gelesen, ebd.].
  - Der Aufwand hängt nicht von der Bohrung ab: „computational effort is independent of the choice of bore profile“ [wörtlich, S. DAFX-5, §4.1].
- Grenze dieses Modells: verlustfrei.
  - Wandverluste führen auf Ableitungen gebrochener Ordnung, „which cause immense difficulty numerically“ [wörtlich, S. DAFX-7].
  - Die Annahme kleiner Radien bei langsamer Querschnittsänderung muss gelten [nur sinngemäß, S. DAFX-7].
- Harrison et al. 2015 nehmen die Verluste mit:
  - Grundlage ist das Modell von Zwikker–Kosten; Keefe liefert die Konstanten [selbst gelesen, S. DAFX-2, Refs. 9, 10].
  - Umsetzung über eine Kettenbruch-Näherung (CFE) mit „M = 20 is a suitable order filter“ [wörtlich, S. DAFX-4].
  - Dieser Filter ist der Kostentreiber: „The dominant time-intensive feature of the algorithm“ [wörtlich, S. DAFX-7, §6]. Gemeint sind die gebrochenen Ableitungen.

**FEM in Raum, energiekonsistente Differenzen in der Zeit (OpenWInD)**

- „Finite differences in time are used to perform an energy-consistent time discretisation“ [wörtlich, openwind.inria.fr].
- Zugehörige Arbeit: A. Thibault, J. Chabassier, „Time-domain simulation of a dissipative reed instrument“, 2020, HAL hal-03132474, open access [wörtlich, HAL-API].
  - Abstract: „a new model for approximating losses due to viscothermal boundary layers in the time domain“ [wörtlich].
  - Ernoult et al. geben als Ort *Forum Acusticum*, Lyon, Dez. 2020 an [selbst gelesen, S. 4876, Ref. 12].
- Viskothermische Modelle: J. Chabassier, A. Thibault, „Viscothermal models for wind musical instruments“, Inria Research Report 9356, 2020, hal-02917351 [selbst gelesen, Ref. 6; HAL-API].
- Kosten: Im Zugänglichen **keine** Zahl.

**Grenzen je Verfahren (konisch, Schallstück, Tonlöcher)**

- Konus/Zylinder: Das FEM-Verfahren „unifies the cases cylinders vs. cones“ [wörtlich, Abstract Tournemenne/Chabassier].
- Tonlöcher und Ventile in OpenWInD: über „pipe junctions including acoustic reactances and inertances“ [wörtlich, selbst gelesen, Ernoult et al., S. 4874].
- Ventile in FDTD als Rohrverzweigung mit zeitveränderlicher Überlappung [selbst gelesen, Harrison et al., S. DAFX-2, Gl. 4–6].
- Ein neueres Tonlochmodell mit schaltendem Port-Hamilton-System (PHS) für Notenübergänge, Frontiers in Signal Processing 2025, DOI 10.3389/frsip.2025.1519450 [nur Zusammenfassung]. Autoren nicht geprüft.

---

## 2. Ein gemeinsamer Kern: Frameworks und Schnittstelle

**OpenWInD (Inria, Python)**

- Lizenz:
  - „open source and available on gitlab under GPL-v3.0 license“ [wörtlich, openwind.inria.fr]
  - PyPI-Feld License: „GNU General Public License v3 (GPLv3) (GPLv3)“ [wörtlich, pypi.org/project/openwind]
  - Klassifikator: „OSI Approved :: GNU General Public License v3 (GPLv3)“ [wörtlich]
  - Stand auf PyPI: Version 0.12.4 vom 29.05.2026 [wörtlich laut Werkzeug].
- Die LICENSE-Datei im GitLab-Repo war wegen Anubis **nicht** prüfbar.
- Anreger:
  - „by coupling a reed, or lips, to the entry of a pipe“ [wörtlich, openwind.inria.fr]
  - Flöte: „(flute embouchure coming soon)“ [wörtlich, ebd.]
  - Das ANR-Projekt OWN-MUSIC (ANR-23-CE10-0015, Koordinator Augustin Ernoult) meldet dagegen: „implemented the physical models of flute-type instruments in the Openwind software“ [wörtlich, anr.fr]. Damit lässt sich „predict the tuning profile of a recorder“ [wörtlich].
  - Ob die Flöte im Zeitbereich in der freigegebenen Version enthalten ist, ist **ungeklärt**.
- Lippe und Rohrblatt: „A one-degree-of-freedom oscillator modeling the reed or lips“ [wörtlich, selbst gelesen, Ernoult et al. 2023, S. 4874].
- Abstrahlung: wählbar „(infinite flanged, unflanged, semi-open hole, etc)“ [wörtlich, ebd.].
- Paper: A. Ernoult, J. Cabaret, J. Chabassier, „Openwind: a software to simulate wind instruments, as a tool for acoustic teachers“, *Forum Acusticum* 2023, Turin, DOI 10.61782/fa2023.0233, frei. Lizenz des Papers: „Creative Commons Attribution 3.0 Unported License“ [wörtlich, selbst gelesen, S. 4873].

**STK (Synthesis ToolKit, C++)**

- Datei `LICENSE`, raw.githubusercontent.com/thestk/stk/master/LICENSE:
  - „Copyright (c) 1995-2023 Perry R. Cook and Gary P. Scavone“ [wörtlich]
  - „Permission is hereby granted, free of charge, to any person obtaining“ [wörtlich]
  - Bitte um Rücksendung von Änderungen: „not a binding provision of this license“ [wörtlich]
  - Zusammengenommen ist das eine MIT-artige Lizenz.
- Klassen Clarinet, Saxofony, Brass, Flute und BlowHole teilen sich die Wellenleiter-Bausteine. Das ist aber kein gemeinsames Rohrmodell aus der Bohrung (eigene Einschätzung).

**Modalys (IRCAM)**

- Die Projektseite forum.ircam.fr/projects/detail/modalys/ nennt **keine** Lizenz [nur Zusammenfassung].
- Forum-Mitgliedschaft:
  - Kostenlos: „Licence Free: Usage individuel à but non lucratif“ [wörtlich, forum.ircam.fr/about/welcome]
  - Ausgenommen sind „exceptées les technologies Premium“ [wörtlich]
  - Premium kostet „200€ / an HT“ [wörtlich]
- Ob Modalys Free oder Premium ist, steht auf dieser Seite **nicht** [nur Zusammenfassung].
- Modalys ist nicht quelloffen (eigene Einschätzung, kein Beleg). Die Dokumentation war nicht erreichbar (403).

**Moreesc:** Rohrblatt und Lippe auf modaler Rohrdarstellung (siehe 1b). Software und Lizenz **nicht gefunden**.

**Harrison et al. 2015:** Das Brass-Programm lässt sich laut Autoren durch kleine Änderung am Anreger auf Klarinette umstellen: „simulate a reed rather than a lip“ [wörtlich, selbst gelesen, S. DAFX-8]. Ventile ließen sich durch Tonlöcher ersetzen (ebd.).

**Schnittstelle Rohr ↔ Anreger (belegt)**

- FDTD, Bilbao 2008: Druck und Volumenfluss am Rohreingang, nämlich p(0,t) = p_in(t) und u(0,t) = u_in(t) [selbst gelesen, S. DAFX-2, Gl. 6].
  - Die Flüsse ergeben sich aus u_in = u_m − u_r, also Bernoulli-Fluss minus Fluss durch die Rohrblattbewegung [selbst gelesen, ebd.].
  - Pro Schritt ist dafür nur eine quadratische Gleichung in |Δp| zu lösen [selbst gelesen, S. DAFX-4, Gl. 16].
- Harrison et al. 2015 verwenden für die Lippe dieselbe Form, Gl. 10e: S(0) v(0,t) = u_m + u_r [selbst gelesen, S. DAFX-3].
- Moden/Impedanz (Guillemain 2005): Druck und Fluss am Eingang über die digitale Impedanz [nur Zusammenfassung].

---

## 3. Anreger, soweit für die Schnittstelle nötig

**Lippe**

- 1 Freiheitsgrad, nach außen schlagend: „a simple one degree of freedom, outward striking reed model“ [wörtlich, selbst gelesen, Harrison et al. 2015, S. DAFX-3]. Bilbao 2008 schaltet Rohrblatt/Lippe durch Vorzeichenwechsel von Δp um: „a simple extension to ‚blown open‘ brass-like instruments“ [wörtlich, S. DAFX-6].
- 2 Freiheitsgrade: S. Adachi, M. Sato, „Trumpet sound simulation using a two-dimensional lip vibration model“, *JASA* 99(2), 1200–1209, 1996 [nur Zusammenfassung]. Inhalt laut Zusammenfassung: Schwing- und Dehnbewegung; Übergang von nach außen schlagend zu seitlich schlagend in höheren Moden.

**Rohrblatt**

- Dynamisch, 1 Freiheitsgrad mit Anschlag an der Mundstückbahn als Potenzgesetz-Abstoßung [selbst gelesen, Bilbao 2008, S. DAFX-2, Gl. 3].
- Quasistatisch beim Doppelrohrblatt: A. Almeida, C. Vergez, R. Caussé, „Quasistatic nonlinear characteristics of double-reed instruments“, *JASA* 121(1), 536–546, 2007, DOI 10.1121/1.2390668. Frei als arXiv physics/0607011 [nur Zusammenfassung].
  - Kernaussage: Klarinette, Oboe und Fagott folgen ähnlichen Prinzipien. Unterschiede entstehen durch Druckrückgewinn im konischen Hülsenteil [nur Zusammenfassung].
- Schnittstelle in allen Fällen: Fluss u(Δp, y) am Rohreingang.

**Luftstrahl–Kante**

- R. Auvray, A. Ernoult, B. Fabre, P.-Y. Lagrée, „Time-domain simulation of flute-like instruments: Comparison of jet-drive and discrete-vortex models“, *JASA* 136, 389–400, 2014, DOI 10.1121/1.4875716, HAL hal-01426971, open access [wörtlich, HAL-API].
  - „The jet-drive model depicts the jet by its transverse perturbation“ [wörtlich, Abstract]
  - Gegenmodell: „two independent shear layers“ (discrete vortex) [wörtlich]
- STK-Flöte: Strahl als Polynom [wörtlich, siehe oben].
- Verhoeven: nicht gesucht.

**Anblasgeräusch:** Nicht gezielt gesucht.

---

## 4. Abstrahlung und nichtlineare Ausbreitung

**Levine–Schwinger:** H. Levine, J. Schwinger, „On the radiation of sound from an unflanged circular pipe“, *Phys Rev* 73(4), 383–406, 1948 [selbst gelesen als Ref. 14 bei Harrison et al.].

- Für den Zeitbereich dient eine rationale Näherung nach Silva et al. 2009: Γ = 0,505, Λ = 0,613, Θ = 1,111 [selbst gelesen, Harrison et al., S. DAFX-2/3, Gl. 7–8].
- Quelle der Näherung: F. Silva, Ph. Guillemain, J. Kergomard, B. Mallaroni, A. N. Norris, „Approximation formulae for the acoustic radiation impedance of a cylindrical pipe“, *J Sound Vib* 322, 255–263, 2009 [selbst gelesen, Ref. 15]. Zugang nicht geprüft.
- Bilbao 2008 warnt, dass Reihenentwicklungen der Abstrahlimpedanz die Passivität verlieren, „numerical instabilities can arise in simulation“ [wörtlich, S. DAFX-2]. Er empfiehlt positiv-reelle rationale Näherungen.

**Richtwirkung:** Nicht gesucht.

**Schmettern (brassiness, nichtlineare Ausbreitung)**

- A. Hirschberg, J. Gilbert, R. Msallam, A. Wijnands, „Shock waves in trombones“, *JASA* 99(3), 1754–1758, 1996 [selbst gelesen als Ref. 35 bei Bilbao 2008 und Ref. 31 bei Harrison et al.].
- J. Gilbert, L. Menguy, M. Campbell, „A simulation tool for brassiness studies“, *JASA* 123(4), 1854–1857, 2008 [selbst gelesen, Ref. 32 bei Harrison et al.].
- Zeitbereichsmodell mit nichtlinearer Ausbreitung: H. Berjamin, B. Lombard, C. Vergez, E. Cottanceau, *Acta Acustica united with Acustica*, Hirzel 2015, DOI 10.3813/AAA.919038, arXiv 1511.04247 (frei).
  - „The non-linear propagation is simulated by a TVD scheme“ [wörtlich, arXiv-Abstract]
  - Lippe: „one-mass model for the lips“ [wörtlich]
  - arXiv-Lizenzfeld: „http://arxiv.org/licenses/nonexclusive-distrib/1.0/“ [wörtlich]
  - Kosten: im Abstract **keine**.
- Harrison et al. zur Stabilität: Nichtlineare Ausbreitung erfordert „entirely new numerical design techniques“ [wörtlich, S. DAFX-8], wegen „severe stability issues as shocks develop“ [wörtlich, ebd.].

---

## 5. Daten

**UNSW Music Acoustics (Wolfe, Smith u. a.): gemessene Eingangsimpedanzen als Excel-Dateien**

| Instrument | Dateien | Quelle |
|---|---|---|
| Querflöte | `Bfoot.xlsx`, `Bfoot_multiphonics.xlsx`, `Cfoot.xlsx`, `Cfoot_multiphonics.xlsx`, `Classical.xlsx` unter www.phys.unsw.edu.au/music/flute/ | [wörtlich laut Werkzeug, newt.phys.unsw.edu.au/music/flute/] |
| Klarinette | `ImpedClarinet.xlsx` | [wörtlich, newt.phys.unsw.edu.au/music/clarinet/]. Hinweis dort: „Reed compliance is not included in the download files.“ [wörtlich] |
| Saxophon | `ImpedSorpranoSax.xlsx` (Schreibweise so auf der Seite), `ImpedTenorSax.xlsx` | [wörtlich, newt.phys.unsw.edu.au/music/saxophone/] |
| Posaune (vermutlich) | `ZTbn_dry_26_3degC.xls`, „Z(f) by Henri Boutin“ | [wörtlich, newt.phys.unsw.edu.au/jw/brassacoustics.html]. Trompete und Horn dort nur als TIF-Bilder. |

- Aufbau der Dateien: je vier Blätter |Z|, arg(Z), Re(Z), Im(Z). Spalte 1 ist die Frequenz, die übrigen Spalten sind Noten [nur Zusammenfassung; Saxophon-Satz „Column 1 is always frequency.“ wörtlich].
- **Lizenz:** Auf keiner der vier Seiten eine Lizenz- oder Nutzungsangabe [Werkzeugbefund, geprüft am 2026-10-26].
  - Fußzeilen: „© 1997-2006 Music Acoustics UNSW“ [wörtlich, clarinet/E6.html] und auf der Startseite „1997-... Music Acoustics UNSW“ (das ©-Zeichen wurde dort verstümmelt wiedergegeben).
  - Die Brass-Seite nennt nur für Zeitschriften-Reprints: „This article may be downloaded for personal use only.“ [wörtlich]
- Datenbank-Paper: Dickens, France, Smith, Wolfe, *Acoustics Australia* 2007, phys.unsw.edu.au/jw/reprints/AAclarinet.pdf [nur Zusammenfassung].
- Saxophon: Chen, Smith, Wolfe 2009 [nur Zusammenfassung].

**Bohrungsprofile**

- Kein freier, lizenzierter Datensatz für Trompete, Posaune, Horn, Oboe, Blockflöte oder Orgelpfeifen gefunden.
- Ein Bildungsartikel stellt fest, dass Brass-Geometrie außer wenigen Literaturbeispielen ohne eigene Messung „not generally known“ ist [nur Zusammenfassung, Suchergebnis zu par.nsf.gov/servlets/purl/10391391].
- Fagottini/Tenoroon (FHNW, historical-bassoon.ch): Tabellen mit Bohrdurchmessern einzelner Teile. Verweis auf „links to Zenodo“ [wörtlich]. Lizenz auf der Seite keine; Zenodo-Einträge nicht geprüft.
- Zenodo-API (drei Abfragen) fand keinen Bohrungs- oder Impedanzdatensatz für Blasinstrumente.
  - Einziger Blasinstrument-Treffer: PIV-Daten einer Bassblockflöte, Lasickas/Chatziioannou, DOI 10.5281/zenodo.18185889, Lizenz-ID „cc-by-4.0“ [wörtlich]. Keine Bohrungsgeometrie.
  - Fehltreffer: „OrganADR“ (10.5281/zenodo.15129153, „mit-license“) hat nichts mit Orgeln zu tun.
- OpenWInD liefert laut Doku Beispiele mit [selbst gelesen, Ernoult et al. S. 4874: „several examples (‚how to‘)“]. Ob dort Bohrungen echter Instrumente liegen, war wegen der GitLab-Sperre **nicht prüfbar**.
- Rechenbeispiel aus dem Paper: ein 50-cm-Kegel als `geom = [[0, 0.5, 7e-3, 15e-3, 'cone']]` [selbst gelesen, S. 4874].

---

## 6. Rechenzeit: Zahlen aus der Literatur

| Quelle | Modell | Hardware / Umgebung | Ergebnis |
|---|---|---|---|
| Harrison, Bilbao, Perry, DAFx-15, Trondheim 2015 (frei, ntnu.edu) | FDTD Ventil-Trompete, 1,381 m, viskothermische Verluste (CFE, M = 20), 1-FG-Lippe, Abstrahlung, 44,1 kHz | Intel Core i5-4300U | 1 s Ton: MATLAB 14,02 s; Serial C 2,69 s; AVX 1,22 s; **AVX (memory optimised) 0,86 s**; GPU 12,75 s [wörtlich, Tabelle 6, selbst gelesen, S. DAFX-7] |
| Bilbao, DAFx-08, Espoo 2008 (frei, dafx.de) | FDTD Klarinettengeometrie, **verlustfrei**, 1-FG-Rohrblatt | Dell, 2,0 GHz Pentium, MATLAB | „approximately 3.9 s to generate 5 s of sound output, at 44.1 kHz“ [wörtlich, S. DAFX-5]; Operationen „on the order of tens of megaoperations/second“ [wörtlich, ebd.] |
| Bilbao 2008, Abstract | Vergleich FD mit Wellenleiter | — | „somewhat more computationally intensive than an efficient waveguide structure“ [wörtlich, S. DAFX-1] |
| Guillemain et al. 2005 | digitale Impedanz, Klarinette | — | nur „real-time“ [wörtlich]; keine Zahl im Abstract |
| OpenWInD, Thibault/Chabassier, Berjamin et al., Wellenleiter, Reflexionsfunktion | — | — | **keine Zahlen gefunden** |

Bei Harrison et al. ist der Faktor 0,86 also etwa 86 % der Echtzeit.

---

## Nicht gefunden

- Rechenzeiten für OpenWInD-Zeitbereichssimulationen und für Wellenleiter- bzw. Reflexionsfunktions-Modelle (keine Zahl in den zugänglichen Quellen).
- Die LICENSE-Datei im OpenWInD-GitLab (gesperrt; Lizenz nur über Website und PyPI belegt).
- Die Modalys-Lizenz (Doku 403; Forum-Seite ohne Zuordnung Free/Premium).
- Moreesc als Software und seine Lizenz.
- Freie, lizenzierte Bohrungsprofile für Trompete, Posaune, Horn, Klarinette, Saxophon, Oboe, Querflöte, Blockflöte und Orgelpfeifen.
- Lizenzangaben zu den UNSW-Impedanzdaten.
- Volltexte auf HAL (Anubis).
- **Nicht gesucht:** Verhoeven, Anblasgeräusch, Richtwirkung, Keefe-Tonlochmodelle im Detail, Ventilmodelle jenseits Harrison, Scavone-Dissertation, NESS-Projekt-Code, Datenbanken außerhalb von Zenodo und HAL (z. B. Mendeley Data, figshare, BIAS/Kausel), deutsch- oder französischsprachige Suchen.

---

## Eigene Einschätzung (kein Beleg)

**Was vorab gerechnet werden kann.** Bohrung → Impedanz (TMM oder 1D-FEM) → Moden (Pole/Residuen) bzw. Reflexionsfunktion: Das ist linear, hängt nicht von der Spielsteuerung ab und lässt sich je Griff einmal vorab rechnen. So entsteht eine Kurve über den Tonumfang. Die nichtlineare Anregung muss **pro Note mit ihrer Steuerungsspur** im Zeitbereich laufen, denn Einschwingen, Lautstärke und Schmettern hängen vom Druckverlauf ab. Ein Katalog aus vorab gerechneten Noten ginge nur für eine feste Hüllkurve je Note.

**Kostenschätzung für die 5-%-Grenze (hochgerechnet, nicht gemessen)**

- **FDTD mit Verlusten** (Harrison: 86 % Echtzeit): etwa 17-fach über dem Budget. Ein Fall für **„Deluxe“**.
- **FDTD verlustfrei** (Bilbao 2008): etwa 0,78 × Echtzeit in MATLAB. In C mit dem typischen Faktor 5 aus Harrison wären das etwa 15 %. Ohne Verluste klingt es aber unrealistisch.
- **Modale Darstellung:** 10–30 Moden als komplexe Ein-Pol-Filter plus ein 1-FG-Anreger. Grob 100–300 Operationen pro Sample, das sind bei 44,1 kHz rund 5–15 Mio. Operationen/s, also **deutlich unter 1 %** der Echtzeit auf einem Kern. Verluste und Abstrahlung stecken dabei schon in Dämpfung und Residuen der Moden.
- **Wellenleiter:** ähnlich billig. Die Bohrung muss aber in Zylinder- und Kegelstücke zerlegt werden, Verluste und Schallstück werden in Filter zusammengefasst. Für beliebige, gemessene Profile ist das umständlich.

**Vorschlag für den gemeinsamen Kern (Klassen 7/8/9): die modale Darstellung der Eingangsimpedanz.** Sie wird vorab aus der Bohrung per TMM oder FEM gerechnet, nach dem Muster von Auvray/Chabassier 2022, Guillemain 2005 und Moreesc. Schnittstelle sind Druck p und Volumenfluss u am Rohreingang. Gründe:

1. **Eine Rohrbeschreibung für alle drei Anreger.** Rohrblatt und Lippe koppeln über u(Δp, y) am Eingang, sie unterscheiden sich nur im Vorzeichen bzw. in der Schlagrichtung (Bilbao 2008, Harrison 2015). Der Strahl koppelt über denselben Port, wobei am Eingang statt der Impedanz die Admittanz maßgeblich ist.
2. **Die teure Physik liegt im Vorab-Teil.** Wandverluste (Zwikker–Kosten), Konus, Schallstück, Tonlöcher und Ventile stecken in den Moden. Der Zeitbereich bleibt billig. Das entspricht dem Prinzip „Größen als Kurven über den Tonumfang, gegen Aufnahmen gesetzt“: Die Modenparameter lassen sich direkt mit den UNSW-Impedanzen vergleichen.
3. **Deluxe-Pfad:** FDTD oder FEM im Zeitbereich nach OpenWInD-Art für Ventil-Zwischenstellungen, Notenübergänge mit schaltenden Tonlöchern und nichtlineare Ausbreitung (Schmettern). Hier versagt die modale Darstellung, weil sie lineare und zeitinvariante Rohre voraussetzt.

**Woran die Empfehlung kippen würde:**

- Wenn sich Übergänge zwischen Noten (Legato, Halbventil) als klangwesentlich herausstellen und modales Umschalten hörbar falsch klingt.
- Wenn Schmettern bei Blechbläsern als Kerneigenschaft gilt.

In beiden Fällen wäre FDTD/FEM als Kern sauberer, mit dem Preis „Deluxe“ für alle Blechbläser.

**Lizenzen**

- **OpenWInD (GPL-3):** Für Vorab-Rechnungen als externes Werkzeug unproblematisch. Code in die eigene C++-Rechnung zu übernehmen, würde aber die GPL nach sich ziehen. Sauberer ist es, nur die Verfahren aus den Papers nachzubauen.
- **STK:** MIT-artig, also übernehmbar. Die Wellenleiter-Patente (Stanford/Yamaha) sind vermutlich abgelaufen; das ist **nicht geprüft**.
- **UNSW-Daten:** Ohne Lizenzangabe gilt das Urheberrecht. Für den Abgleich lokal nutzbar, aber nicht weitergeben. Eine Nachfrage bei Joe Wolfe wäre sauber.

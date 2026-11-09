# Recherche IC1: Lippe und Rohr — Lippe, Bohrung, Zug und Ventile, Dämpfer, Schmettern, Spielweisen

> **Auftrag:** Klasse 7 (Trompete, Posaune, Horn, Tuba, mit Dämpfern): Lippenmodelle und Parameter, Bohrungsprofile,
> Posaunenzug und Halbventil, Dämpfer, Schmettern, Spielweisen, Rechenzeit; aufbauend auf `gemeinsam/rohr.md`.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = im PDF-Bild selbst gelesen (mit Seite) oder vom Werkzeug als Zitat/Feldinhalt; [nur
> Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (22 Suchen, rund 37 Abrufe, 11 PDFs aus dem Cache; Acta Acustica, HAL-Volltexte,
> OpenWInD, ICA 2016, Springer nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Klasse 7 „Lippe und Rohr“: Trompete, Posaune, Horn und Tuba mit Dämpfern (Recherche IC, 2026-10-26)

## Reichweite

- **Umfang:** 22 Websuchen (WebSearch, Sprache Englisch, eine davon „extended“) und rund 37 Abrufe (WebFetch). Zu den Abrufen zählen 6 Anfragen an die HAL-API, eine an die Crossref-API und eine auf die arXiv-Abstract-Seite. Damit liegt der Bericht über dem vorgegebenen Rahmen von 30–45. Der Grund: Viele PDFs kamen über WebFetch nur als Binärdatei an. Ihre Seiten habe ich danach aus dem Werkzeug-Cache selbst gelesen, mit dem Read-Werkzeug als Seitenbilder. Angaben daraus tragen **[wörtlich, S. x]**.
- **Suchbegriffe (Auswahl):** lip reed parameters mass stiffness, lipping up/down, trombone slide FDTD dynamic grid, half-valve, brass mutes impedance, harmon/wah-wah, brassiness potential, bore profile data trumpet/horn/tuba, growl, multiphonics, slurred transients, vibrato brass.
- **Orte:** UNSW (newt.phys.unsw.edu.au), dafx.de, arXiv, HAL-API, Acoustics Today, euphonics.org, Edinburgh Research Explorer, ISMA 2017 (cirmmt.mcgill.ca), Vienna Talk 2015 (mdw.ac.at), dael.euracoustics.org (Forum Acusticum), mdw/IWK.
- **Nicht erreichbar:**
  - acta-acustica.edpsciences.org: alle Volltexte mit HTTP 403, darunter die Lippen-Übersicht 2022 (aacus210088), 2023 (aacus220092) und das Tuba-Paper 2022 (aacus220031).
  - hal.science-Volltexte und gitlab.inria.fr (OpenWInD-Beispiele): Anubis-Bot-Schutz.
  - ICA 2016 (Paper 399, Dämpfer): Zertifikatsfehler.
  - musicweb.ucsd.edu (Smyth, SMC 2011): Zertifikat abgelehnt.
  - link.springer.com (Smyth & Scott 2011): Cookie-Weiterleitung.
  - ioa.org.uk (Campbell 2021): 404.
- **Abruf-Cache (vom Werkzeug angelegt, nicht von mir gespeichert):** alle unter
  `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`

  | Datei | Inhalt | gelesen |
  |---|---|---|
  | `webfetch-1793033876565-oa2lm5.pdf` | Boutin, Smith, Wolfe 2020, JASA | S. 1–5 |
  | `webfetch-1793033921022-x4v41l.pdf` | Giordano, ISMA 2017 | S. 1–4 |
  | `webfetch-1793033935787-ls0gba.pdf` | Vergez/Rodet, ICMC 2001 | nicht gelesen |
  | `webfetch-1793033968261-zi8t5u.pdf` | Strauss, arXiv 2404.05056 | S. 1–4 |
  | `webfetch-1793034158929-5rioqd.pdf` | Willemsen et al., DAFx 2021 | S. 1, 5–8 |
  | `webfetch-1793034210622-608wed.pdf` | Myers et al. 2012, JASA | S. 1–6 |
  | `webfetch-1793034274460-enr0uk.pdf` | Bilbao, DAFx-11 | S. 2–4 |
  | `webfetch-1793034335897-b9bm8h.pdf` | Hruška et al., Vienna Talk 2015 | S. 1–3 |
  | `webfetch-1793034364430-gtdayr.pdf` | Boutin, Fletcher, Smith, Wolfe, SMAC 2013 | S. 1–4 |
  | `webfetch-1793034430732-te8r9c.pdf` | Petiot et al., Forum Acusticum 2023 | S. 1–3 |
  | `webfetch-1793034492040-1jrt5w.pdf` | Gilbert & Dalmont, ISMA 2010 | S. 1–2 |

- **Kennzeichnung:**
  - **[wörtlich]**: von mir im PDF-Bild gelesen, mit Seite. Oder ein Feldinhalt bzw. Zitat, das das Werkzeug als solches zurückgab.
  - **[nur Zusammenfassung]**: Aussage stammt nur aus einer Werkzeug-Zusammenfassung.
  - **Seitenangaben:** gedruckte Seitenzahl, wo vorhanden, sonst „PDF-S.“.

---

## 1. Lippe

### 1.1 Modelle und das Verhalten der Lippe im Register

**Boutin, Fletcher, Smith, Wolfe (2013): „Lip motion, the playing frequency of the trombone and the upstream and downstream impedances“**
- Ort: SMAC 2013, UNSW-Server (newt.phys.unsw.edu.au/jw/reprints/SMACtrombone.pdf), frei zugänglich.
- **Kernbefund für das Register: Die Lippe wechselt den Ventil-Typ.** Im Abstract (PDF-S. 1) [wörtlich]: „consistent with a (+1,−1) valve for B♭2 and (−1,+1) for B♭3“.
  - Die Paare (σ1, σ2) stehen für Fletchers Ventil-Einteilung (JASA 93, 2172) [wörtlich, PDF-S. 1 und 4]. Also nach außen schlagend (outward striking) beim tiefen Ton und anders beim höheren.
- Lage der Spielfrequenz zur Resonanz der Bohrung [wörtlich, PDF-S. 4]: „fp is always located above the bore resonance when the note played is B♭2 and below when it is B♭3“.
- Art der Bewegung [wörtlich, PDF-S. 1]: Für B♭3 sei „the lip motion is mainly transverse“.
- Mundstück der Messung [wörtlich, PDF-S. 2]: Kesselvolumen „11.6 cm³“, Durchmesser „2.53 cm“.
- Rechte [wörtlich, PDF-S. 1]: „open-access article distributed under the terms of the Creative Commons Attribution 3.0 Unported License“.

**Boutin, Smith, Wolfe (2020): „Trombone lip mechanics with inertive and compliant loads (“lipping up and down”)“**
- Ort: J. Acoust. Soc. Am. 147 (6), June 2020, S. 4133–4144, doi 10.1121/10.0001466. PDF auf dem UNSW-Server (newt.phys.unsw.edu.au/jw/reprints/lipping.pdf).
- Ausgangslage [wörtlich, S. 4133]: „Trombonists normally play at a frequency slightly above a bore resonance.“
- Lage der Spanne [wörtlich, S. 4133]: Der Bereich des Treibens (lipping up and down) liegt „roughly symmetrical about the peak in bore impedance“.
- Umfang des Treibens: B♭2, „nominally 116.5 Hz but which was “lipped” to frequencies in the range 100–125 Hz“ [wörtlich, S. 4134].
  - Eigene Rechnung: etwa −260 bis +120 Cent um den Nennton.
- **Wirksame Lippenbreite** [wörtlich, S. 4135]: „The average value of L was 13 mm, which is 0.56 times the inner width of the mouthpiece.“
- Phase zwischen Fluss und Druck [wörtlich, S. 4137]: „varies from about +69° to −75°“.
- Der Mund trägt mehr bei, je weiter der Spieler treibt [wörtlich, S. 4137]: „reaches 41% while lipping up from F3“.
- Rechte [wörtlich, S. 4133]: „© 2020 Acoustical Society of America“. Keine offene Lizenz.

**Newton, Campbell, Gilbert (2007): „Mechanical response measurements of real and artificial brass players lips“**
- Ort: J. Acoust. Soc. Am. Laut HAL-Feld Jahr 2007, Lizenzfeld „https://about.hal.science/hal-authorisation-v1/“ [wörtlich, HAL-API]. Volltext durch Bot-Schutz gesperrt.
- Inhalt [nur Zusammenfassung]:
  - Starke, nach außen schlagende Resonanz; der gespielte Ton liegt darüber.
  - Teils eine schwächere zweite Resonanz, nach innen schlagend, über der Spielfrequenz.
  - Menschliche Lippen haben kleinere Gütefaktoren (Q) als künstliche.

**Euphonics, Abschnitt „11.5 “Brass” instruments“** (euphonics.org, ISSN 2977-5612)
- Zitate, die das Werkzeug als Zitat zurückgab [wörtlich]:
  - „the playing frequency always lies above the lip resonance frequency“
  - Gütefaktor der Lippe: „The Q-factor of the lip resonance is here taken to be 15“
  - Masse je Fläche: „The results to be shown here use the value 0.6 kg/m²“ (Kornett/Zink)
  - Breite: „a width 6 mm has been used“
  - Mundruck: „all have a mouth pressure 10 kPa“
- Weitere Werte [nur Zusammenfassung]:
  - Posaune: 9 kg/m².
  - Andere Q-Werte: 1, 2, 5, 7, 30.
  - L_lip = 8 mm.
  - Die Lippenresonanz wird je Register als Parameter durchgefahren.
- Rechte [wörtlich]: „Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License“.

### 1.2 Parameter mit Werten

| Quelle | Instrument | Masse | Steife | Q | Breite | Ruhespalt | Lippen-f₀ | Mundruck |
|---|---|---|---|---|---|---|---|---|
| Petiot, Roatta, Fréour, Arimoto, FA 2023, Tab. 1 [wörtlich, S. 376] | Trompete B | μl „0.5 to 2“ kg m⁻² | – | „5“ | „10“ mm | „0.05“ mm | „Variable, according to the regime simulated“ | „500 to 14000“ Pa |
| dieselbe, Tab. 2 (gewählter „virtueller Musiker“) [wörtlich, S. 377] | Trompete B | „1.3“ kg m⁻² | – | „[4, 6]“ | „10“ mm | „0.5“ mm | je Regime, „Range : +-30 cent“ | „8000“ Pa |
| Willemsen, Bilbao, Ducceschi, Serafin, DAFx 2021, Tab. 2 [wörtlich, S. 157] | Posaune | „5.37·10⁻⁵“ kg | Stoß-Steife K_c „10⁴“ N/m | Dämpfung σ_r „5“ s⁻¹ | „0.01“ m | H₀ „2.9·10⁻⁴“ m | „20 ≤ ω_r/2π ≤ 1000“ | „0 ≤ P_m ≤ 6000“ Pa |
| Giordano, ISMA 2017 [wörtlich, S. 101–102] | Trompete (3D-Spielzeugmodell) | „m = 4x10⁻⁶ kg“ | „k = 125 N/m“ | „Q = 3“ | Lippe „2.0 mm wide“ | „0.6 mm“ | – | „typically 10 kPa“ |
| Boutin et al. 2020 [wörtlich, S. 4135] | Posaune, Spieler | – | – | – | wirksam „13 mm“ | – | – | – |

Hinweise zur Tabelle:
- **Widerspruch in der Quelle:** Petiot et al. nennen den Ruhespalt einmal mit 0.05 mm (Tab. 1) und einmal mit 0.5 mm (Tab. 2).
- Die Werte bei Willemsen et al. stammen nach Tab. 2 aus Smyth & Scott 2011 ([16]), Harrison-Harsley 2018 ([13]) und Benade ([17]).
- Die Lippenfrequenz koppelt Willemsen an die Rohrlänge [wörtlich, S. 157]: Der Faktor „F = 2.4 was heuristically found to best match the 4th resonating mode“.
- Weitere Werte nur aus Such-Zusammenfassungen [nur Zusammenfassung]:
  - Smyth und Kollegen (Posaune): Lippenresonanz 310 Hz, Q = 6.
  - Ein Trompetenmodell: Q = 5 passe besser zu gemessenen Einschwingvorgängen als 1–3.
  - Laut der Zusammenfassung von Strauss nennt die Literatur für menschliche Lippen Q ≈ 0.46–1.8, für künstliche 3–10.
- Strauss (arXiv 2404.05056, „A Lip Vibration Model Using Mechanical Properties of Flesh“, Einzelautor) [wörtlich, PDF-S. 2]: „Ultrasonic measurements of the lower lip average 33.7 kPa“ (Elastizitätsmodul). Ein ungewöhnlicher Ansatz mit der Lippe als Platte. Belegkraft gering.
- **Nicht erreicht:**
  - Velut et al., Übersicht der Lippenwerte (Acta 2022/2023, 403).
  - Cullen/Gilbert/Campbell 2000.
  - Fréour/Scavone 2013 (nur über Dritte zitiert).

### 1.3 Änderung über den Tonumfang
- Gemessen: Die Lippe liegt im tiefen Register unter der Spielfrequenz und schlägt nach außen. Im höheren Register ändert sich der Ventil-Typ (Boutin 2013, siehe 1.1).
- In der Simulation legt man die Lippenfrequenz je Regime neu fest. Petiot 2023 variiert sie um ±30 Cent um das Regime [wörtlich, S. 377]. Willemsen 2021 koppelt sie an die Rohrlänge.
- Ein Werteverlauf der Masse oder der Steife über die Register, gemessen an Spielern, wurde **nicht gefunden**.

---

## 2. Bohrung und Mundstück

**Posaune, Maße als Tabelle: Willemsen et al. 2021, Tab. 1, „Geometry of a measured trombone taken from [16]“** [wörtlich, S. 157]

| Teil | Länge (cm) | Radius (cm) |
|---|---|---|
| Innenzug (1) | 70.8 | 0.69 |
| Außenzug, ausgezogen (2) | 53 | 0.72 |
| Zugbogen (3) | 17.7 | 0.74 |
| Außenzug, ausgezogen (4) | 53 | 0.72 |
| Innenzug (5) | 71.1 | 0.69 |
| Schwanenhals (6) | 24.1 | 0.71 |
| Stimmzug (7) | 25.4 | 0.75, 1.07 |
| Schallstück (8) | 50.2 | 1, 10.8 |

- Gesamtlänge laut Tab. 2: „2.593 ≤ L ≤ 3.653“ m.
- Die Quelle [16] ist T. R. Smyth and F. S. Scott, „Trombone synthesis by model and measurement“, *EURASIP Journal on Advances in Signal Processing*, 2011 [wörtlich, Literaturliste S. 159].
  - Laut Such-Zusammenfassung ist der Artikel Open Access unter CC BY 2.0 [nur Zusammenfassung]. Die Lizenz habe ich nicht selbst gesehen (Springer-Weiterleitung).
- Rechte am DAFx-Paper [wörtlich, S. 152]: „open-access article distributed under the terms of the Creative Commons Attribution 3.0 Unported License“.
- Das Schallstück ist nur mit Anfangs- und Endradius angegeben; das Flare-Profil fehlt.

**Brassiness-Parameter B je Instrument: Myers, Pyle, Gilbert, Campbell, Chick, Logie (2012)**
- Ort: „Effects of nonlinear sound propagation on the characteristic timbres of brass instruments“, J. Acoust. Soc. Am. 131 (1), Pt. 2, 678–688.
- Methode [wörtlich, S. 680]: B wird aus ≥10 Bohrungsmessungen berechnet; es heißt, Messungen „at ten or more points … give sufficient accuracy“.
- Die Werte stehen in Kapitel 5. Laut S. 680 sind über 1000 Instrumente vermessen. **Die Profile selbst sind nicht veröffentlicht.**

**Trompete**
- Petiot et al. 2023 [wörtlich, S. 377]: gemessene Eingangsimpedanz von „9 real Bb trumpets“, zerlegt in „11 modes … 44 modal parameters“. Die Bohrung selbst ist nicht angegeben.
- Ein freier Datensatz für das Trompetenprofil wurde **nicht gefunden**.
- HAL-Treffer mit Profilen im Text, Volltext nicht gelesen:
  - Tournemenne et al. 2019, JASA, hal-01963668. Lizenzfeld „hal-authorisation-v1“ [wörtlich, HAL-API].
  - Petiot, Fréour, Arimoto 2025, „Numerical design of a trumpet lead pipe…“, hal-05529764. Lizenzfeld „https://creativecommons.org/licenses/by/4.0/“ [wörtlich, HAL-API].

**Mundstück**
- IWK/mdw, „Untersuchung zur Beeinflussung des Resonanzverhaltens von Trompeten durch unterschiedliche Mundstücksformen“ (Kausels Institut):
  - Seele (Bohrung) [wörtlich]: „von 3 auf 5.4 mm“.
  - Wirkungen [nur Zusammenfassung]:
    - Kesselvolumen: Verschiebung bis etwa 35 Cent, beim zylindrischen Extremfall bis 180 Cent.
    - Seele: bis 10 Cent an den Trompetenresonanzen.
    - Backbore: bis 30 Cent.
  - Rechte [wörtlich]: „© 2026 Institut für musikalische Akustik – Wiener Klangstil (IWK)“.
- Boutin 2013: Kessel 11.6 cm³, Ø 2.53 cm (siehe 1.1).

**Horn und Tuba**
- Ein Datensatz wurde **nicht gefunden**. Hinweise:
  - Edinburgh-Dissertation zu Hornbögen (era.ed.ac.uk/handle/1842/2582), nicht geöffnet.
  - Tuba-Paper in Acta 2022 (aacus220031, 403); laut Such-Zusammenfassung enthält es Profile und ist CC BY 4.0 [nur Zusammenfassung].
  - Mattéoli et al., „Diversity of ghost notes in tubas, euphoniums and saxhorns“, arXiv 2207.05395. Lizenzlink [wörtlich]: „http://arxiv.org/licenses/nonexclusive-distrib/1.0/“ (keine offene Lizenz).

**Ventile (Umwege)**
- Bilbao, DAFx-11 (siehe 3.) [wörtlich, DAFx-338]: Ein Ventil ist gekennzeichnet durch „its location along the bore, the lengths of the default and bypass tubes“. Zahlen für eine konkrete Trompete enthält das Paper nicht (S. 2–4 gelesen).

**OpenWInD-Beispielgeometrien:** gitlab.inria.fr ist durch Bot-Schutz gesperrt, also **nicht geprüft**.

---

## 3. Posaunenzug und Ventile

**Zug: Willemsen, Bilbao, Ducceschi, Serafin, „A Physical Model of the Trombone Using Dynamic Grids for Finite-Difference Schemes“, DAFx20in21, S. 152–159** (frei zugänglich, CC BY 3.0)
- Ansatz [wörtlich, S. 152]: eine Methode „to dynamically vary the number of grid points“ des FDTD-Gitters.
- Kopplung von Lippe und Länge: Die Lippenfrequenz wird mit der Länge mitgeführt (S. 157, Gl. 42).
- Grenze für die Änderungsgeschwindigkeit [wörtlich, S. 157]: Ein Gitterpunkt darf „every 20 samples“ hinzukommen. Den ganzen Bereich von L durchläuft das Modell damit „in ca. 0.06 s“.
- Artefakte [wörtlich, S. 158]: Die Methode „does not introduce perceivable audible artifacts“. Das beruht auf informellem Hören der Autoren.
- Code: github.com/SilvinWillemsen/cppBrass/releases [wörtlich, Fußnote S. 157]. Die Lizenz des Codes habe ich nicht geprüft.

**Ventile und Halbventil: Bilbao, „Modelling of Brass Instrument Valves“, DAFx-11 Paris, S. DAFx-338–340**
- Modell: Verbindung dreier Rohre, mit gleichem Druck in allen drei Rohren und Volumenflüssen, die sich zu null addieren (Gl. 7–8) [wörtlich, S. DAFx-338].
- Ventilstellung über Flächenanteile: „S(d) = q(d)S(in)“, „S(b) = q(b)S(in)“ (Gl. 9) [wörtlich, S. DAFx-338].
- Zeitveränderliche Ventile kosten wenig [wörtlich, S. DAFx-340]: „O(1) additional operations per time step“.
- Laut Abstract werden Halbventil-Impedanzen gezeigt [nur Zusammenfassung, dafx.de-Detailseite].
- Die Rechtslage habe ich nicht gelesen (Seite 1 nicht geöffnet).

**Modaler Weg:** Ein Paper, das einen modalen bzw. Impedanz-Kern mit einem zeitveränderlichen Halbventil verbindet, wurde **nicht gefunden** [nur Zusammenfassung der Suche]. Die modale Zerlegung gemessener Impedanzen ist üblich: Petiot 2023 nutzt 11 Moden [wörtlich, S. 377].

---

## 4. Dämpfer

**Hauptquelle: Campbell, Gilbert, Myers (2021), „“Put a Sock in It!” Mutes for Musical Horns“**
- Ort: Acoustics Today, Summer 2021. Titel und DOI 10.1121/AT.2021.17.2.13 stammen nur aus der Such-Zusammenfassung; Autoren und Seitenkopf „MUTES FOR MUSICAL HORNS“ [wörtlich, S. 16 und 21].
- Rechte: nicht gefunden.
- Cutoff-Frequenz der Trompete ohne Dämpfer [wörtlich, S. 16]: „the cutoff frequency for this instrument is in the region of 1,200 Hz“.
- Gerader Dämpfer (straight), Messung an der Tenorposaune [wörtlich, S. 19]:
  - Schall entweicht nur „through the narrow annular space“ zwischen Dämpfer und Stürze.
  - „a small additional peak, corresponding to the parasitic resonance, has appeared at 77 Hz“
  - „the pitches of the second, third, and fourth peaks have been slightly raised“
- Störresonanz beim geraden Dämpfer [wörtlich, S. 20]: „the insertion of the mute has split the second peak into two smaller peaks“. Betroffen ist die 7. Zugposition. Ein aktiver Dämpfer gibt den Pedalton B♭1 (58 Hz) zurück.
- Cup-Dämpfer [wörtlich, S. 20]: „a straight mute surrounded by an annular bowl“; „an attenuation of 20 dB“ (schmalbandig um 1000 Hz).
- Harmon-Dämpfer [wörtlich, S. 20]: „has a cork strip“; der Stiel erlaubt verschiedene Klangfarben.
- Plunger [wörtlich, S. 16]: erzeugt den „wah-wah“-Klang.
- Mit eingesetztem Dämpfer [wörtlich, S. 18]: „the pitches of the acoustic modes from the eighth downward are increasingly flattened“. Kontext: Naturtrompete mit transponierendem Dämpfer.
- Hand in der Hornstürze, Stopfen [wörtlich, S. 19]: „a new set of natural notes a semitone higher than the unstopped set“.

**Weitere Quellen**
- Velut, Vergez, Gilbert (2017), „Physical modelling of trombone mutes, the pedal note issue“, *Acta Acustica united with Acustica*. HAL hal-01385524, Lizenzfeld „hal-authorisation-v1“ [wörtlich, HAL-API].
  - Methode [wörtlich, Abstract-Zitat]: „Linear stability analysis and time-domain simulations are used“.
- Meurisse et al. (2015), „An active mute for the trombone“, JASA [wörtlich, HAL-Felder].
- Werte nur aus Such-Zusammenfassungen [nur Zusammenfassung]:
  - Ancell 1960, JASA (Kornett): alle Dämpfer wirken als Helmholtz-Resonator bei 200–300 Hz; beim Harmon Formanten durch das Rohr des Stiels.
  - Yoshikawa & Nobara 2017: Cup-Dämpfer als zusätzlicher Hohlraum plus verkleinerte Öffnung; der gerade Dämpfer als Hochpass um 1800 Hz (nach Backus).
  - POMA 2023 (doi 10.1121/2.0001738): Dämpfer ändern die Richtwirkung vor allem über 1 kHz.
  - ICA 2016, Paper 399: Impedanzmessung und FEM für Trompete und Posaune mit Straight, Cup und Harmon, 50–3000 Hz.
- Zum Wah-Wah-Dämpfer (Hand über dem Stiel) mit Zahlen wurde **nichts gefunden**.

---

## 5. Schmettern (brassiness)

**Gilbert & Dalmont, „Brassy sounds, from trombone to elephant“, ISMA 2010**
- Gelesen über einen Drittserver (collinsaudio.com); Rechtslage unklar.
- Maß ist die Stoßbildungslänge x_s = 2γP_at·c / ((γ+1)(∂p/∂t)_max) (Gl. 1, S. 1).
- Wann das Schmettern einsetzt [wörtlich, S. 1]: „When the length of the bore is comparable to this critical distance“; dann gibt es „highly distorted waves“.
- Wann es fehlt [wörtlich, S. 2]: „If xs is much larger than L, linear theory of acoustic propagation is rightfull.“
- Abb. 4 zeigt bei einem Posaunen-Crescendo F4 einen Anstieg SC2/SC1 bis etwa 4 bei P1rms etwa 4000 Pa. Das ist nur aus der Abbildung abgelesen.

**Myers et al. 2012 (JASA 131, 678–688)**
- Definition (Gl. 6, S. 680): B = (1/L_ecl)∫D₀/D(x)dx.
- Größe von B [wörtlich, S. 680]: B ist „higher for “cylindrical” instruments like the trumpet and trombone“.
- **Tab. I (S. 681), B bei 8–9 ft** [wörtlich]:

  | Instrument | B |
  |---|---|
  | Tenorposaune, französisch | 0.81 |
  | Tenorposaune, deutsch | 0.76 |
  | Bassposaune, modern | 0.70 |
  | Tenorposaune, amerikanisch | 0.67 |
  | Basstrompete | 0.61 |
  | Bariton | 0.54 |
  | Doppelhorn, B-Seite | 0.51 |
  | Saxhorn basse | 0.45 |
  | Euphonium | 0.40 |
  | Wagnertuba | 0.36 |
  | Ophikleide | 0.30 |

- Wo das Schmettern entsteht, gemessen an der Tenorposaune [wörtlich, S. 681]: „SC(in) typically increased from 1.2 to 1.6, and SC(out) from 1.5 to 4.0“.
  - SC ist der Spektralschwerpunkt im Mundstück (in) bzw. vor der Stürze (out).
  - Die Aufhellung im Crescendo kommt also überwiegend aus der Ausbreitung im Rohr. Ein Teil kommt laut S. 682 auch aus der Kopplung an der Lippe.
- Skalierung [wörtlich, S. 679]: B ist „unaffected by a linear scaling of the radial dimensions“. Absolut engere Bohrung schmettert trotzdem früher (S. 679, sinngemäß).
- Rechte:
  - [wörtlich, S. 678]: „© 2012 Acoustical Society of America“.
  - Edinburgh-Deckblatt [wörtlich]: „Publisher's version/PDF may be used on authors own or employers website only“.

**Modale Ansätze für die nichtlineare Ausbreitung**
- Soares, Cochelin, Freour, Vergez, Arimoto (2025), „Nonlinear propagation in brass instruments using modal approaches on the one-dimensional Blackstock equation“.
  - Ort: CFA 2025, hal-05365315. Lizenzfeld „https://creativecommons.org/licenses/by/4.0/“ [wörtlich, HAL-API].
  - Inhalt [nur Zusammenfassung]:
    - Modale Diskretisierung zu gewöhnlichen Differentialgleichungen mit konstanten quadratischen Tensoren.
    - Beispiele: Zylinder, Kegel, vereinfachte Trompete.
    - Viskothermische Verluste noch nicht explizit enthalten.
- Gilbert, Campbell, Myers, Pyle (2007), ISMA, hal-00475561 [wörtlich, HAL-Felder]. Inhalt [nur Zusammenfassung]: Ein Parameter für spektrale Anreicherung steigt monoton mit B.

---

## 6. Spielweisen

**Zungenstoß und Einschwingen: Hruška, Švejda, Guštar, „Transients of trumpet tone: basic links between perception and measurements of lips opening surface and pressure in player's mouth“**
- Ort: Vienna Talk 2015, S. 4–6; Messung an F4 und B♭4 (349 und 466 Hz).
- Scharfer Einsatz, mf [wörtlich, S. 5]: „Initial leap of mouth pressure is almost 2 kPa“.
- Weicher Einsatz, p [wörtlich, S. 5]:
  - „Initial “blow off” of ca. 250 Pa is observed at 0.1s“
  - „Initial leap of mouth pressure is less than 1 kPa“
- Ohne Zunge [wörtlich, S. 6]: „Without tongue action there is no clear peak in the mouth pressure“. Gemeint ist das Tripelstaccato ta–ka.
- Legato F4→B♭4 [wörtlich, S. 6]: „For a short time (0.2-0.25 s, see fig. 7) the player's lips are not fully closing.“
- Abwärts gebunden [wörtlich, S. 6]: „exhibits less demands on player's embouchure“.
- Rechte: nicht gesehen.

**Lippenbindung (lip slur): Logie, Chick, Stevenson, Campbell, „Upward and Downward Slurred Transients on Brass Instruments: Why is One Not Simply the Inverse of the Other?“**
- Ort: CFA 2010, hal-00539675 [wörtlich, HAL-Felder].
- Abstract-Zitate [wörtlich]:
  - „the player uses significantly different techniques for slurring upwards than for slurring downwards“
  - „particularly noticeable when the player moves between larger intervals“

**Singen beim Spielen (Multiphonics, Nähe zum Growl): Velut, Vergez, Gilbert, „Measurements and time-domain simulations of multiphonics in the trombone“**
- Ort: JASA 140 (2016) 2876, doi 10.1121/1.4964634 [wörtlich, HAL-Felder].
- Modell [wörtlich, Abstract-Zitat]: Das Modell „is forced while self-oscillating to reproduce simultaneous singing and playing“.
- Ergebnis [wörtlich, Abstract-Zitat]: „at the cost of a high forcing pressures“.

**Treiben (lipping) als Tonhöhe der Lippe:** etwa −260 bis +120 Cent bei B♭2 (Boutin 2020, siehe 1.1).

**Nicht gefunden:**
- Growl am Blech als eigene Messung.
- Flatterzunge am Blech.
- Lippentriller.
- Fall und Doit.
- Vibrato mit Rate und Tiefe am Blech. Nur [nur Zusammenfassung]: Bei der Posaune gibt es Vibrato aus dem Ansatz und aus dem Zug.

---

## 7. Rechenzeit

- **Willemsen et al. 2021 (Posaune, FDTD mit dynamischem Gitter, 44,1 kHz, C++/JUCE):** Tab. 3 [wörtlich, S. 158]. Gemessen auf einem „MacBook Pro with a 2.2 GHz Intel i7“ [wörtlich, S. 158].

  | Zustand | CPU ohne Grafik | CPU mit Grafik |
  |---|---|---|
  | kurz (L = 2.593 m) | 4.3 % | 12.1 % |
  | lang (L = 3.653 m) | 5.2 % | 14.4 % |
  | schnell wechselnd | 10.1 % | 17.7 % |

  - Viskothermische Verluste fehlen dort. Laut S. 159 sind sie als künftige Arbeit genannt.
- **Bilbao DAFx-11:**
  - Kosten pro Sekunde: N(4M+6)f_s mit M als Filterordnung für die viskothermischen Verluste.
  - Für L = 1 m, M = 20, 44,1 kHz [wörtlich, S. DAFx-340]: „on the order of 500 Mflops“.
- **Giordano 2017 (3D Navier–Stokes):** „4.4x10⁷“ Gitterpunkte [wörtlich, S. 102]. Für Klangsynthese nicht brauchbar.

---

## Rechte und Lizenzen (Abruf 2026-10-26)

| Datensatz / Text | Vermerk [wörtlich] | Ort |
|---|---|---|
| Willemsen et al. 2021 (Posaunengeometrie Tab. 1, Lippenwerte Tab. 2) | „Creative Commons Attribution 3.0 Unported License“ | DAFx-PDF, S. 152 |
| Smyth & Scott 2011 (Ursprung der Geometrie) | nicht gesehen; laut Suche CC BY 2.0 [nur Zusammenfassung] | – |
| Boutin et al. 2013 (SMAC) | „Creative Commons Attribution 3.0 Unported License“ | PDF-S. 1 |
| Petiot et al. 2023 (FA, Lippenwerte) | „Creative Commons Attribution 3.0 Unported License“ | S. 375 |
| Boutin et al. 2020 (JASA) | „© 2020 Acoustical Society of America“ | S. 4133 |
| Myers et al. 2012 (B-Tabelle) | „© 2012 Acoustical Society of America“ | S. 678 |
| Euphonics 11.5 | „Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License“ | euphonics.org |
| Soares et al. 2025 | Lizenzfeld „https://creativecommons.org/licenses/by/4.0/“ | HAL-API |
| Petiot, Fréour, Arimoto 2025 (Mundrohr) | Lizenzfeld „https://creativecommons.org/licenses/by/4.0/“ | HAL-API |
| Velut et al. 2016 und 2017; Newton et al. 2007; Tournemenne et al. 2019 | Lizenzfeld „https://about.hal.science/hal-authorisation-v1/“ | HAL-API |
| IWK Mundstück | „© 2026 Institut für musikalische Akustik – Wiener Klangstil (IWK)“ | mdw.ac.at |
| Mattéoli et al. (arXiv) | „http://arxiv.org/licenses/nonexclusive-distrib/1.0/“ | arXiv-Abstract-Seite |

Faktenwerte wie Maße und Zahlen sind meist nicht selbst geschützt. Diese Einordnung ist meine Einschätzung, keine Rechtsauskunft.

---

## Nicht gefunden

1. **Freie Bohrungsprofile:**
   - Trompete (Braden, Noreland, Kausel/BIAS, Ernoult/Chabassier).
   - Horn und Tuba.
   - Kein offener Datensatz gefunden. OpenWInD-Beispiele und Acta-Volltexte waren gesperrt. Die einzige vollständige Tabelle ist die der Posaune (Willemsen/Smyth), und dort fehlt das Flare-Profil.
2. **Lippenparameter je Register an Spielern** (Masse, Steife über den Tonumfang). Die Übersicht von Velut et al. war nicht erreichbar.
3. Werte von Cullen/Gilbert/Campbell 2000 und Fréour/Scavone 2013 im Original.
4. Zahlen zum Wah-Wah-Dämpfer. Impedanzwerte des Harmon-Dämpfers im Original (Ancell nur über die Suche).
5. Physik von Growl am Blech, Flatterzunge, Lippentriller, Fall und Doit. Vibrato-Rate und -Tiefe am Blech.
6. Ein Paper, das einen modalen Kern mit Halbventil oder Zug verbindet.
7. Eine Schwelle in dB, ab der das Schmettern hörbar wird; es gibt nur Abbildungen.

Kein Treffer ist kein Beweis.

---

## Eigene Einschätzung (kein Beleg)

**Größen, die das Modell braucht, und woher sie kommen**
- **Lippe, 1 Freiheitsgrad, nach außen schlagend:** je Instrument Startwerte aus Petiot 2023 (Trompete) und Willemsen 2021 (Posaune):
  - μ etwa 0.5–2 kg/m²
  - Breite 8–13 mm
  - Q etwa 3–6 in der Simulation, beim Menschen eher kleiner
  - Ruhespalt zwischen 0.05 und 0.5 mm, laut Quellen uneinheitlich
  - Mundruck 0.5–14 kPa
- **Lippenfrequenz je Note:** nicht messen, sondern vorab je Note suchen, sodass die Spielfrequenz dem Zielton entspricht. Petiot macht das so mit ±30 Cent.
- **Ventil-Typ im hohen Register:** Der Befund von Boutin 2013, dass sich der Ventil-Typ ändert, spricht dafür, im hohen Register einen 2-Freiheitsgrad-Ansatz oder ein umschaltbares (σ1, σ2) vorzusehen. Sonst wird das Treiben im hohen Register falsch.

**Posaunenzug: kippt den modalen Kern nicht**
- Die Länge ändert sich unterhalb der Audiorate; Willemsen begrenzt den ganzen Hub auf ≥ 0.06 s.
- Vorschlag: Moden (Frequenz, Dämpfung, Residuum) auf einem Raster von etwa 20–50 Zugstellungen per TMM vorab rechnen und je Mode interpolieren.
- Risiken:
  - Zuordnung der Moden über die Zugstellungen (mode tracking).
  - Residuen nahe der Cutoff-Frequenz.
- Ein quasistatisches Modell verliert nur kleine Effekte der Bewegung. Das ist unbelegt.
- FDTD bleibt der Deluxe-Pfad wie geplant.

**Halbventil: liegt ehrlich im FDTD-Pfad**
- Die Verbindung zweier Wege erzeugt zusätzliche Impedanzspitzen. Ein Überblenden zwischen „offen“ und „gedrückt“ trifft das nicht.
- Modal ginge nur mit vorab gerechneten Impedanzen für mehrere q-Stufen. Dabei ändert sich die Zahl der Moden, was schwierig wird.

**Schmettern: unverzichtbar für Posaune und Trompete ab f, kippt den Kern aber nicht**
- Bei Posaune und Trompete (B 0.6–0.8) ist es für ein Zielbild „klingt wie das Instrument“ ab f unverzichtbar. SC(out) steigt auf 1.5→4.0 gegenüber SC(in) 1.2→1.6.
- Bei Horn und Tuba (B 0.36–0.51) ist es schwächer, wohl verzichtbar für ein erstes Ziel.
- Den modalen Kern kippt es nicht, wenn man es als Stufe am Ausgang rechnet: verzerrende Ausbreitung der auslaufenden Welle über eine äquivalente Zylinderlänge, nach dem Maß x/x_s.
- Die Rückwirkung auf die Lippe wird dabei vernachlässigt. Das ist eine Vereinfachung, im Messplan zu prüfen.
- Der modale Blackstock-Ansatz (Soares 2025) wäre die saubere modale Lösung, ist aber noch Forschungsstand.
- **Folge für den Bestand:** Expression muss auch diese Ausgangsstufe steuern, nicht nur die Anregung.

**Dämpfer**
- **Straight und Cup:** je Dämpfer ein eigener vorab gerechneter Impedanz- und Abstrahlungssatz.
  - 1D-TMM reicht wohl nicht für den Ringspalt; besser rotationssymmetrische FEM oder Messung.
  - Wichtig ist die Störresonanz bei etwa 77 Hz (Posaune) und ihre Wirkung auf tiefe Töne.
- **Harmon:** wie Straight und Cup, ohne Ringspalt und mit Stiel.
- **Wah-Wah:** eher ein zeitveränderliches Filter der Abstrahlung, gesteuert über eine Kurve je Note. Das widerspricht dem Bestand, der „Dämpfer“ nur als Spur über eine Passage führt.
- **„Umstecken“:** physikalisch spielt der Bläser in dieser Zeit nicht. Der Übergang ist eher eine Pause als ein gerechneter Übergang. Das ist ein Widerspruch zur Formulierung im Bestand.
- **Stopfen beim Horn** (+1 Halbton, neue Resonanzreihe) fehlt im Zielbild.

**Spielweisen am Anreger**
- **Fall und Doit:** Kurven für Lippenfrequenz und Mundruck, die durch die Regime gleiten; bei Ventilen zusätzlich mit Halbventil.
- **Lippentriller:** schnelle Modulation der Lippenfrequenz zwischen zwei Regimen.
- **Growl:** zusätzliche Druckquelle stromaufwärts mit eigener Stimm-Tonhöhe (nach Velut, „forcing“). Das braucht eine zweite Tonhöhe als Eingang, die der Bestand nicht vorsieht.
- **Flatterzunge:** periodische Unterbrechung von Druck bzw. Spalt.
- **Vibrato:** Modulation von Lippenfrequenz und Mundruck, bei der Posaune auch der Zuglänge.
- **Zungenstoß:** Sprung im Mundruck (bis etwa 2 kPa) mit Freigabe durch die Zunge. Weicher Einsatz ohne Spitze.
- **Lippenbindung:** asymmetrische Kurven auf- und abwärts.
- **Pitch über die Lippe:** nur etwa −260 bis +120 Cent um einen Ton. Größere Tonhöhenbeugungen gehen nur über einen Wechsel des Regimes.

**Aufwand und Risiko je Instrument**
- **Posaune:** beste Datenlage (Geometrie, Lippe, Spielermessungen). Risiko beim Zug mittel, beim Schmettern hoch. **Empfohlen als erstes Instrument.**
- **Trompete:** Lippendaten vorhanden, Bohrung fehlt. Ventile und Halbventil sind das Risiko.
- **Horn:** keine Bohrung, dichte hohe Moden, Hand in der Stürze, Doppelhorn. **Höchster Aufwand.**
- **Tuba:** keine Bohrung. Schmettern weniger wichtig, Geisternoten möglich. Mittel.

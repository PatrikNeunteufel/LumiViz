# Recherche IC1: Physik, Rechenverfahren, Bauteile und Maße — Platte und Stab

> **Auftrag:** Stab (Moden, Unterschnitt, Dämpfung, Aufhängung), Kontakt Schlägel–Stab, Resonator und Vibraphon-Motor,
> Glockenspiel, Röhrenglocken, Celesta, Kalimba, Steeldrum, Rhodes, Wurlitzer; was modal vorab rechenbar ist, Kosten;
> Bauteile und Maße mit Herkunft.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = Wortlaut, den der Agent selbst sah (PDF-Seite) oder den das Werkzeug zitierte; [nur
> Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (20 Suchen, 35 Abrufe, sieben PDFs aus dem Cache gelesen; JASA/Acta Acustica, HAL,
> Shear, Gabrielli 2020 nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Recherche Klasse „Platte und Stab“: Physik, Rechenverfahren, Bauteile und Maße

Abrufdatum aller Quellen: 2026-10-26. Kennzeichnung: **[wörtlich]** heißt, ich habe den Wortlaut selbst gesehen (bei PDFs auf der genannten Seite, über das Leseprogramm als Seitenbild oder über `pdftotext`) oder das Abrufwerkzeug hat ihn als Zitat beziehungsweise Feldinhalt zurückgegeben. **[nur Zusammenfassung]** heißt, die Angabe stammt aus einer Zusammenfassung des Such- oder Abrufwerkzeugs. Sie ist kein Beleg.

## Reichweite

- **Umfang:** 20 Websuchen und 35 Abrufe (WebFetch). Davon sind 12 fehlgeschlagen: 403, Bot-Sperre (Anubis auf HAL und RCAAP), Zertifikatsfehler (hibberts.co.uk, bioresources.cnr.ncsu.edu), DNS-Fehler (labs.sonicfield.org) oder 404. Damit liege ich über dem Rahmen von 30–45 Abrufen; die Fehlschläge zählen mit.
- **Zusätzlich lokal gelesen:** sieben PDFs, die WebFetch selbst in seinen Cache gelegt hat. Gelesen habe ich sie über das Leseprogramm (Seitenbilder), mit `pdftotext` und mit `~/.claude/tools/quelle_suchen.py`. Ich habe nichts heruntergeladen und keine Datei geschrieben.
- **Dateien im WebFetch-Cache** (Pflichtmeldung), alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`:
  - `webfetch-1793025712476-mhf890.pdf`: Rucz & Ulveczki, ICSV27
  - `webfetch-1793025751806-5sloba.pdf`: Pfeifle, DAFx-17
  - `webfetch-1793025782841-e4j470.pdf`: Sønderbo, Masterarbeit Aalborg
  - `webfetch-1793025845180-chunnq.pdf`: Thomas, Monteil & Touzé, ENOC 2014
  - `webfetch-1793025689894-a11ox3.pdf`: Bilbao, Torin & Chatziioannou, arXiv 1405.2589
  - `webfetch-1793026114796-k5g67c.pdf`: Chapman, Canadian Acoustics 2006
  - `webfetch-1793026176868-uwo29m.pdf`: Falaize & Hélie, DAFx-15
- **Suchbegriffe** (alle englisch, eine Suche französisch):
  - Chaigne/Doutaut xylophones I und II
  - Beaton/Scavone undercut FE
  - Petrolito/Legge optimal undercuts
  - Henrique/Antunes mallet percussion
  - Rhodes tine/tonebar/pickup (Münster, Pfeifle, Gabrielli)
  - Wurlitzer electrostatic pickup
  - steelpan nonlinear coupling
  - tubular bells strike note
  - marimba bar dimensions
  - Dämpfung von Stäben
  - Suits
  - Hertz-Steife des Schlägels
  - Doutaut, Dissertation („thèse“)
  - kalimba
  - vibraphone motor
  - Orduña-Bustamante
  - Falaize/Hélie
- **Datenbanken:** Websuche allgemein; direkt abgerufen wurden arXiv, DAFx-Archiv, REAL (MTAK), NOVA Research, Google Patents, The Wood Database, CCRMA, Digital Commons UAB und das AAU-Projektportal. Zenodo habe ich nicht abgefragt.
- **Nicht erreichbar:** alle JASA- und Acta-Acustica-Volltexte (Verlage, nicht frei). Die Chaigne/Doutaut-Teile I und II habe ich weder im Volltext noch als Abstract-Zitat erreicht. Ebenfalls nicht erreichbar:
  - HAL (Bot-Sperre)
  - Curtin-Repositorium (403)
  - UAB-Thesis-PDF (403)
  - Shear, Masterarbeit UCSB (Zertifikatsfehler)
  - Hamdan et al., BioResources (Zertifikatsfehler)
  - Hibbert, Seiten zu Röhrenglocken (Zertifikat abgelaufen)
  - Gabrielli et al. 2020 bei IRIS Univpm (403)

---

## 1. Stab

### 1.1 Moden, Balkenmodell
- **Pfeifle 2017 (DAFx-17):** stützt sich auf Traill-Nash & Collar 1953. Danach ist der Schubbalken (shear beam) „a better approximation for the vibrations of the fundamental“ als Euler–Bernoulli (S. DAFX-19) **[wörtlich]**. Er sei weniger aufwendig als Timoshenko, „similar accurate“ (ebd.) **[wörtlich]**. Das gilt dort für die Rhodes-Tine, nicht für Stabspiele.
- **Kirkland 2014 (UAB-Thesis):** Laut Abstract stützt sich die Modellbildung ungleichförmiger Stäbe auf Euler–Bernoulli- und Timoshenko-Theorie. Frühere Arbeiten erreichten 1–2 % Fehler; „musical tolerances require 0.25% error“ **[wörtlich laut Abrufwerkzeug; Abstract-Seite]**. Das Verfahren nimmt die Modenfrequenzen als Nebenbedingungen und minimiert die Masse; als Beleg dienen Vibraphon-Stäbe A4 **[nur Zusammenfassung]**.
- **Rucz & Ulveczki 2021 (ICSV27):** Die vertikalen Biegemoden werden durch den Unterschnitt auf „1 : 4 : 10 or 1 : 3 : 9“ gestimmt (S. 1) **[wörtlich]**. Das ist eine Abweichung von der Angabe 1:3:6 in der Auftragsfrage: Für das Xylophon nennt diese Quelle 1:3:9.
- **Torsionsmoden** (Rucz & Ulveczki, S. 7) **[wörtlich]**:
  - Sie werden „near the nodal line of the first vertical mode“ angeregt, besonders bei starkem Schlag.
  - Liegt der Schlagpunkt auf der Längsachse, werden sie nicht angeregt.
  - Sie strahlen kaum ab, können aber „a significant amount of energy“ abziehen.

### 1.2 Unterschnitt-Stimmung
- **CCRMA-Seite (Scavone):**
  - Marimba: zweiter Teilton „a ratio of 4.0“, dritter „around 9.2 times the fundamental“ **[wörtlich]**.
  - Xylophon: „less undercut“, erster Oberton „a twelfth (a ratio of 3.0)“ **[wörtlich]**.
  - Vibraphon: „deeply undercut aluminum bars“, Oberton im Doppeloktav-Abstand **[wörtlich]**.
- **HyperPhysics (R. Nave):** Bei der Marimba liegt der andere Oberton „close to the 10th harmonic“ **[wörtlich]**.
- **Faustregeln nach Flynt (Mechanical Music Digest 2009, Erstdruck AMICA Bulletin 1972/73):**
  - Material aus der Mitte zu nehmen „will flatten only the fundamental“ **[wörtlich]**.
  - Material „about 25% inward from each end“ zu nehmen senkt nur den Oberton **[wörtlich]**.
  - Die Frequenz ist proportional zur Dicke und zu 1/L² **[wörtlich laut Abrufwerkzeug]**.
- **Rechenverfahren aus Zielverhältnissen**, nur bibliographisch belegt, ohne Inhalt:
  - Beaton & Scavone, „Three-dimensional tuning of idiophone bar modes via finite element analysis“, JASA 149(6), 3758–3768 (2021). Gefunden im Lebenslauf von Scavone; nicht frei erreicht **[nur Zusammenfassung]**.
  - Beaton & Scavone, Vibraphon-Prototyp mit 3D-Ausschnitt, JASA Express Letters 2(8), 083201, DOI 10.1121/10.0013470 **[nur Zusammenfassung]**.
  - Henrique & Antunes, „Optimal design and physical modelling of mallet percussion instruments“, Acta Acustica u. w. Acustica 89, 948–962 (2003) **[nur Zusammenfassung; Literaturangabe Dritter]**.
  - Curtin-Thesis „Automatic multi-modal tuning of idiophone bars“: Unterschnitt mit Timoshenko-Rezeptanzen vorhergesagt. Autor und Jahr nicht gesehen **[nur Zusammenfassung]**.
  - Petrolito & Legge (JASA 1997) und Orduña-Bustamante (1991): Ich habe nur Erwähnungen Dritter gefunden, keinen Titel aus der Quelle selbst. Bork siehe Abschnitt 3.
- **Kalimba-Zahl als Euler–Bernoulli-Anker:** Für den einseitig eingespannten Stab nennt Chapman nach Rayleigh „f1/f0 = 6.2669“ (Canadian Acoustics 34(3), 2006, S. 27) **[wörtlich]**.

### 1.3 Dämpfung
- **Rucz & Ulveczki, Tab. 1 (S. 5):** Palisanderstab F4 mit 348,8 Hz, Masse 96,7 g, Modendämpfung ξ_m,1 = 2,57·10⁻⁴ **[wörtlich]**. Die viskoelastischen Verluste seien für höhere Teiltöne „significantly greater“ (S. 7) **[wörtlich]**. Ein Frequenzgesetz nennt die Quelle nicht.
- **Patent US 4,411,187 (Roper & Montz, 1983):**
  - Vibraphon: „30 seconds in the low end to about 6 seconds“ (3 Oktaven) **[wörtlich]**.
  - Marimba: „decay period of two seconds to 1/50 second“ **[wörtlich]**.
  - Bedämpft erreicht Aluminium die halbe Amplitude in 1,5 s, Palisander in 0,5 s **[wörtlich]**.
  - Das ist Herstellerprosa, keine Messung.
- **Sønderbo 2024 (Tine-Modell, Stahl):** σ₀ = 0,0001 s⁻¹ und σ₁ = 0,005 m²/s (Tab. 3.1, S. 9) **[wörtlich]**. Es ist das übliche Zwei-Parameter-Gesetz: frequenzunabhängiger Anteil plus Anteil über ∂ₜ∂ₓₓ.

### 1.4 Aufhängung
- Knoten liegen nach Flynt „about 25% inward from each end“ **[wörtlich]**. Den genauen Wert 0,224·L habe ich nicht belegt gefunden.

## 2. Schlägel–Stab

- **Rucz & Ulveczki (S. 3, Gl. 6–7):** Hertz-Kontakt F = K_M·δ^(3/2) **[wörtlich, Formel]**. Werte aus Tab. 1 (S. 5) **[wörtlich]**:
  - m_M = 25·10⁻³ kg
  - K_M = 5·10⁷ N/m^(3/2)
  - v_M(0) = 1 m/s
  - Kontaktzeit 0,6 ms
  - Kontaktfläche 3,88 mm²
- **Numerik während des Kontakts** (S. 5) **[wörtlich]**: 48 kHz, im Kontakt „oversampling of 100–200 times“.
- **Pfeifle 2017 (S. DAFX-19):** Hysterese-Hammer nach Hunt–Crossley, F = k·x^α + λ·x^α·x_t, für Rhodes und Wurlitzer **[wörtlich, Formel 1]**.
- **Sønderbo 2024, Tab. 3.2 (S. 14), Hunt–Crossley für den Rhodes-Neoprenkopf** **[wörtlich]**:
  - M_h = 0,011 kg
  - α_h = 2,8
  - k_h = 1,5·10¹¹
  - λ = 9·10¹⁰
  - v_in bis 4 m/s
  - Die Herkunft dieser Werte ist in der Arbeit an [15] gebunden; nicht geprüft.
- **Falaize & Hélie 2015 (DAFx-15, S. 5), Rhodes-ähnlich:** m_h = 5 g, k_h = 10⁶ N/m, Breite a_h = 1 cm, Hammer bei z_h = 2,35 cm **[wörtlich, pdftotext]**.
- **Bilbao, Torin & Chatziioannou (arXiv 1405.2589, S. 14):** Schlägel auf Membran, Masse 0,028 kg, α = 2,54 **[wörtlich, Textebene]**. Membran, nicht Stab.
- **Chaigne & Doutaut 1997**, „Numerical simulations of xylophones. I. Time-domain modeling of the vibrating bars“, JASA 101(1):539–557. Angabe so in der Literaturliste bei Rucz, S. 8 **[wörtlich]**. Nicht frei zugänglich. Die Schlägelwerte daraus habe ich **nicht** gefunden.

## 3. Resonator

- **Doutaut, Matignon & Chaigne**, „Numerical simulations of xylophones. II. Time-domain modeling of the resonator and of the radiated sound pressure“, JASA 104(3):1633–1647, 1998. So bei Rucz, S. 8 **[wörtlich]**. Eine Suchzusammenfassung nannte Band 103; widersprüchlich, am Verlag prüfen. Nicht frei zugänglich.
- Laut Rucz nahm Teil II an, dass die Rückwirkung des Resonators auf den Stab vernachlässigbar ist. Das gelte für Xylophon-Stäbe „in the higher frequency range“ und „cannot be generalized“ (S. 1) **[wörtlich]**.
- **Rucz & Ulveczki 2021, gekoppeltes 3D-FE-Modell** (Orff-Xylophon F4, Helmholtz-Resonator):
  - Resonator f_a,1 = 349,1 Hz, ξ_a,1 = 8,8·10⁻³ (Tab. 1, S. 5) **[wörtlich]**.
  - Ein gestimmter Resonator hebt den Pegel um „as much as 15 dB“ (S. 7) **[wörtlich]**.
  - Er verkürzt T₋₆₀ von nahe 10 s auf 0,7 s (S. 7) **[wörtlich]**.
  - Hauptsächlich betroffen ist der Grundton (S. 7) **[wörtlich]**.
  - Nachbarstäbe verschieben die Resonatorfrequenz; ohne Stäbe steigt sie um einen Halbton (S. 4, 6) **[wörtlich]**.
  - Modal reduziert: „A few tens of modes were found to be sufficient“ (S. 4) **[wörtlich]**.
- **Soares, Debut & Antunes 2022**, „The Vibroacoustic Interaction In Mallet Percussion Instruments: Modelling And Experiments“, Revista de acústica 53(3-4), 13–20. Konzentriertes Modell (lumped) mit mehreren Stab- und Resonatormoden; Einfluss von Dämpfungsverhältnis, Lage und Abstand des Rohrs **[nur Zusammenfassung]**. Lizenz siehe Tabelle.
- **Bork**, „Practical tuning of xylophone bars and resonators“, Applied Acoustics 46:103–127, 1995 (Rucz, S. 8) **[wörtlich, Literaturangabe]**. Nicht frei erreicht.
- **Vibraphon-Motor:** CCRMA spricht von „motor-driven discs at the top of the resonators“ **[wörtlich]**, ohne Drehzahl. Eine gemessene Modulationsrate in Hz oder eine Modulationstiefe habe ich nicht gefunden. Die 6 Hz bei mmdigest sind ein synthetisches Demo **[nur Zusammenfassung]**.

## 4. Weitere Instrumente

- **Glockenspiel** (CCRMA) **[wörtlich]**:
  - Umfang „G5 (f = 784 Hz) to C8 (f = 4186 Hz)“.
  - „All but the lowest transverse mode die out quickly“.
  - Stahlwerte fehlen.
- **Röhrenglocken** (CCRMA) **[wörtlich]**:
  - „lengths of brass tubing“, oben mit Messingstopfen.
  - Moden mit „rough ratios of 2:3:4“.
  - Der Schlagton beruht auf „modes 4, 5, and 6“; gehört wird eine Oktave unter Mode 4.
  - Durchmesser 30–38 mm und Moden 9²:11²:13² nur über Wikipedia **[nur Zusammenfassung]**. Rossing (1982), Science of Sound, S. 241–242, ist zitiert, aber nicht erreicht.
- **Celesta:** keine Fachquelle gefunden.
- **Kalimba** (Chapman, „Investigating the sound of an African thumb piano (kalimba)“, Canadian Acoustics Vol. 34 No. 3 (2006), S. 26–27):
  - Eine Zunge ist frei am Schlagende, über einen Steg (fulcrum) gestützt und dahinter eingespannt (S. 26) **[wörtlich]**.
  - f₁/f₀ liegt bei 5,3–5,9 **[wörtlich]**.
  - Tab. 1 (S. 27) **[wörtlich]**, Maße a (frei bis Steg) und b (Steg bis Klemmleiste):

    | Ton | a | b | f₀ | f₁ | f₁/f₀ |
    |---|---|---|---|---|---|
    | D3 | 5,4 cm | 1,4 cm | 304 Hz | 1780 Hz | 5,86 |
    | D4 | 3,7 cm | 1,3 cm | 581 Hz | 3080 Hz | 5,30 |

  - Synthese aus „two exponentially damped sinusoids“ (S. 26) **[wörtlich]**.
  - JASA 2008, Modell „clamped-supported-free beam“, nur Abstract **[nur Zusammenfassung]**.
- **Steeldrum:**
  - Bryde & Mahadevan, „Localization in musical steelpans“, arXiv 2212.14465, eingereicht am 29. Dez. 2022, verwandte DOI 10.1098/rspa.2022.0869 **[wörtlich]**. Inhalt: Schalenmodell, Lokalisierungslandschaft (localization landscape), FE **[nur Zusammenfassung]**.
  - Thomas, Monteil & Touzé, „Complicated dynamics exhibited by thin shells displaying numerous internal resonances: application to the steelpan“, ENOC 2014, Wien (S. 1–2):
    - Kopplungen setzen schon bei Amplituden „of order 1/25 times the thickness“ ein **[wörtlich]**.
    - Modelle mit drei Freiheitsgraden und quadratischen Termen für 1:2:2 und 1:2:4 **[wörtlich, Gleichungen S. 2]**.
    - G3 zeigt „1:2:2:4:4:6:8“ **[wörtlich]**.
    - Beim Spielen werden Energieübergänge „up to a dozen of mode“ beobachtet **[wörtlich]**.
  - Literatur daraus **[wörtlich, S. 2]**: Achong (2003) JSV 266, 193–197; Rossing, Hansen & Hampton (2000) JASA 108(2) 803–812.
- **Rhodes:**
  - Pfeifle 2017 (DAFx-17, Edinburgh):
    - Tine als Kragbalken mit zwei Polarisationen und Kirchhoff-artigem Term K(u) für große Auslenkung; Stimmfeder als Zusatzmasse (S. DAFX-19/20, Gl. 2–3) **[wörtlich]**.
    - Tonabnehmer: Induktion nach Faraday, Magnetfeld über ein Modell mit magnetischen Punktladungen (Gl. 4–8) **[wörtlich]**.
    - Tonebar auf einer Brass-Leiste, „sometimes π/2 twisted“ (S. DAFX-18) **[wörtlich]**.
  - Falaize & Hélie 2015: Euler–Bernoulli-Kragbalken „based on a standard modal decomposition“, Hammer und Tonabnehmer nichtlinear, passives Verfahren **[wörtlich, Abstract]**. Werte (S. 5) **[wörtlich]**: Stahl r = 2 mm, ρ = 7750 kg/m³, E = 180·10⁹ N/m², 440 Hz ergibt l_b = 7,83 cm, Tonabnehmer 1 mm Abstand, 48 kHz. Die Zeitschriftenfassung erschien im JSV 390 (2017) 289–309, DOI 10.1016/j.jsv.2016.11.008 **[nur Zusammenfassung]**.
  - Gabrielli, Cantarini, Castellini & Squartini, JASA 148(5) 3052–3064 (2020), DOI 10.1121/10.0002002: unharmonische Moden im Anschlag, Laservibrometrie, Intermodulation durch den Tonabnehmer **[nur Zusammenfassung]**. Nicht erreicht.
  - Wendland 2009 (TU Berlin) und Münster et al., JASA 136(4):2164 (2014, Tagungsabstract): nur Literaturangaben bei Pfeifle, S. DAFX-24 **[wörtlich]**.
- **Wurlitzer** (Pfeifle 2017, S. DAFX-18/21):
  - Elektrostatischer Tonabnehmer, Zunge auf 0 V gegen eine Platte; das Handbuch nennt 170 V, gemessen wurden „considerably lower (130V)“ **[wörtlich]**.
  - Zunge als ungleichförmiger Kragbalken in einer Richtung **[wörtlich]**.
  - Strom i = u₀·∂C/∂t, C scheibenweise aus FEM **[wörtlich, Gl. 10–12]**.
  - Gestimmt wird über Lötzinn an der Spitze **[wörtlich]**.
  - Hämmer aus Ahornsperrholz mit Filz **[wörtlich]**.
  - Gabrielli zum Wurlitzer: nicht gefunden.

## 5. Modal gegen Zeitbereich, Kosten

- Rucz: modale Basis mit „a few tens of modes“, Newmark-Verfahren bei 48 kHz, im Kontakt 100–200-fach überabgetastet (S. 4–5) **[wörtlich]**.
- Pfeifle 2017: Finite Differenzen auf FPGA (Virtex-7), keine CPU-Kosten angegeben **[wörtlich, S. DAFX-21/22]**.
- Sønderbo 2024: Echtzeit-FDTD; die Tine-Länge wird gegenüber dem Original verlängert, um zu stimmen (S. 10–11) **[wörtlich]**. Rechenzeiten habe ich nicht gelesen.
- Falaize & Hélie: modal mit passivem Verfahren, Modenzahl nicht gefunden.

## 6. Bauteile und Maße

| Größe | Wert | Quelle, Ort | Kennz. |
|---|---|---|---|
| Honduras-Palisander (*Dalbergia stevensonii*) | 1025 kg/m³, E = 22 GPa | wood-database.com/honduran-rosewood | [wörtlich] |
| Afrik. Padouk (*Pterocarpus soyauxii*) | 745 kg/m³, E = 13,07 GPa | wood-database.com/african-padauk | [wörtlich] |
| Padouk-Marimba La Favre, C2 | L 23 3/32″, B 3,72″, D 1,55″, Bogenmitte 0,25″ | lafavre.us/marimba-bars.htm | [Feldinhalt laut Werkzeug] |
| dto. F#4 / C7 | 13 9/32″ × 1,94″ × 0,88″ / 7 11/16″ × 1,62″ × 0,74″ | ebd. | [Feldinhalt laut Werkzeug] |
| „5-Oktaven-Palisander-Marimba“ (repräsentativ) C2 / C7 | L 21 3/4″ / 7 3/16″ | ebd. | [Feldinhalt laut Werkzeug] |
| Resonatoren La Favre | F#4: AD 2,25″, Luftsäule 8 1/4″; C2: AD 4″, 53 5/8″ gesamt | ebd. | [Feldinhalt laut Werkzeug] |
| Verbundstäbe Patent, E5 | ca. 11 7/8″ × 1 1/2″, Al 3/8″ + Palisander 3/16″ | US 4,411,187, Tab. I | [nur Zusammenfassung, Tabelle verstümmelt] |
| Yamaha YM-4600A | Breite 2,5″→1,625″, Dicke 1″→0,75″ | usa.yamaha.com | [nur Zusammenfassung] |
| Orff-Stab F4 (Palisander) | 96,7 g, 348,8 Hz | Rucz, Tab. 1, S. 5 | [wörtlich] |
| Rhodes-Tines (73 Tasten) | 0,1564 … 0,0226 m, Quelle Service Manual (Schnitttabelle) | Sønderbo, S. 10 | [wörtlich] |
| Rhodes-Tine, Modellwerte | r = 0,0005 m, ρ = 7850, E = 2·10¹¹ | Sønderbo, Tab. 3.1, S. 9 | [wörtlich]; der Radius ist für ein echtes Tine fraglich, A = πr ist dort ein Druckfehler |
| Kalimba-Zungen | siehe Abschnitt 4 | Chapman, S. 27 | [wörtlich] |

Unterschnittprofile als Zahlenreihe, Resonatorlängen je Note in Tabellenform, Maße von Röhrenglocken und Stahlwerte für das Glockenspiel habe ich nicht gefunden.

### Lizenzen und Rechte (alle [wörtlich], abgerufen 2026-10-26)
- **arXiv 2212.14465 und 1405.2589:** Link „view license“ auf `arxiv.org/licenses/nonexclusive-distrib/1.0/` (Abstract-Seite). Das ist keine freie Weitergabelizenz.
- **Sønderbo (AAU):** „The content of this report is freely available“, aber „publication (with reference) may only be pursued due to agreement with the author“ (Titelblatt, S. ii).
- **Wood Database:** „Copyright © 2008–2026 Eric Meier. All Rights Reserved.“
- **CCRMA:** „©1998-2003 CCRMA, Stanford University. All Rights Reserved.“
- **Mechanical Music Digest:** „Compilation copyright 1995-2026 by Jody Kravitz.“
- **NOVA (Soares et al.):** „Licence: Unspecified“; dazu „All rights are reserved, including those for text and data mining“.
- **Ohne gesehenen Lizenzvermerk:** DAFx-17 (Pfeifle), DAFx-15 (Falaize & Hélie), ICSV27 (Rucz), Canadian Acoustics (Chapman), ENOC 2014, La Favre, HyperPhysics. Nichts gesehen heißt nicht frei.

---

## Nicht gefunden
- Schlägelwerte aus Chaigne & Doutaut 1997 und deren Dämpfungsgesetz.
- Wortlaut aus den Arbeiten von Petrolito & Legge, Orduña-Bustamante und Henrique & Antunes.
- Volltext von Beaton & Scavone 2021.
- Shear, Masterarbeit UCSB 2011 (Abruf scheiterte).
- Gabrielli: Volltext zum Rhodes, Arbeiten zum Wurlitzer gar nicht.
- Fachquelle zur Celesta.
- Maße und Moden von Röhrenglocken aus Rossing.
- Drehzahl und Modulationstiefe des Vibraphon-Motors.
- Werkstoff- und Dämpfungswerte für Aluminium- und Stahlstäbe.
- Unterschnittprofile als Zahlen.
- Dämpfung je Mode über der Frequenz für Holz und Aluminium.

Kein Treffer ist kein Beweis.

## Eigene Einschätzung (kein Beleg)
- **Physik:**
  - Frei-freier Euler–Bernoulli-Stab: Verhältnisse ungefähr 1 : 2,76 : 5,40, Knoten bei etwa 0,224·L. Das stammt aus Lehrbuchwissen und ist hier nicht belegt.
  - Timoshenko oder Schubbalken lohnt, sobald die Dicke gegen die Biegewellenlänge nicht mehr klein ist: obere Teiltöne, dicke Bassstäbe, Xylophon-Oberlage.
  - Für die Stimmung zählt der Unterschnitt mehr als die Balkentheorie. Praktisch: 1D-Modell (FE oder FD) mit veränderlichem Querschnitt, Profil auf Zielverhältnisse optimieren, gegen Aufnahmen setzen.
- **Rechenaufwand:** Die modale Vorabrechnung ist für alle Stäbe gut machbar, mit etwa 10–40 Moden je Note. Den Kontakt im Zeitbereich rechnet man mit Überabtastung nur während etwa 1 ms; das ist weit unter 5 % der Echtzeit.
- **Aufwand und Risiko je Instrument:**
  - **Marimba, Xylophon:** Aufwand mittel, Risiko gering bis mittel. Offen sind Profildaten und Dämpfungskurven.
  - **Vibraphon:** Aufwand mittel. Der Motor als periodische Änderung von Kopplung und Abstrahlung des Resonators ist einfach; das Risiko liegt in fehlenden Messwerten.
  - **Glockenspiel:** Aufwand gering, Risiko gering.
  - **Celesta:** Aufwand mittel. Die Hammermechanik ist ähnlich dem Flügel; das Risiko liegt in fehlenden Daten.
  - **Röhrenglocken:** Aufwand mittel. Rohrmoden sind modal gut darstellbar; Risiko beim Schlagton, der eine Wahrnehmungsfrage ist (prüfen gegen Aufnahmen).
  - **Kalimba:** Aufwand gering. Risiko beim Kasten und beim Schnarren.
  - **Steeldrum:** Aufwand hoch, Risiko hoch. Die geometrische Nichtlinearität mit internen Resonanzen braucht modal gekoppelte quadratische und kubische Terme im Zeitbereich. Das ist machbar mit wenigen Moden je Feld, aber schwer zu bemessen.
  - **Rhodes:** Aufwand mittel bis hoch. Tine plus Tonebar modal linear, Tonabnehmer als statische Nichtlinearität danach; das ist günstig. Risiko beim Voicing und bei den Polarisationen.
  - **Wurlitzer:** Aufwand mittel. Zunge modal, Kapazitätskurve C(x) als Nachschlagtabelle. Risiko: Eingangsschaltung und Messwerte sind dünn belegt.

> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S153, 2026-10-25) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** Teilplan 31d-7, Entscheid P1 a (`docs/audio/Spielweisen_Konzept.md` §4.1).
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen; Angaben aus der HAL-API oder einer Suchmaschine sind nur Metadaten. Zitierfähig ist erst,
> was aus der Quelle selbst unter `quellen/` nachgelesen ist.

# Recherche: Zwei Polarisationen je Saite, Kopplung am Steg (Weinreich 1977 und Folgearbeiten)

**Datum:** 2026-10-25
**Auftrag:** Frei zugängliche Quellen finden, die je Saite zwei Schwingungsebenen mit Kopplung am Steg in Gleichungen fassen, angefangen bei Weinreich 1977.

**Hinweis zum Ablauf:** Ich habe nichts bewusst heruntergeladen. WebFetch legt aber abgerufene PDFs selbsttätig als Zwischenspeicher ab. Vier solche Dateien liegen jetzt in `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\3d658580-2527-4986-8a7c-609dd2c6673a\tool-results\` (webfetch-*.pdf: DAFx 1999, Tan 2015, Valiente 2024, Ege/Chaigne 2007). Nur aus diesen Dateien stammen die Aussagen „Volltext gesehen“; gelesen habe ich sie mit `pdftotext`. Sie können gelöscht werden.

HAL war für WebFetch gesperrt (Anubis: „Access Denied“). Die Metadaten der HAL-Einträge stammen deshalb aus der HAL-Such-API (api.archives-ouvertes.fr), die Volltexte habe ich dort **nicht** gesehen.

## 1. Weinreich 1977

- **Angabe** (Verlagsseite pubs.aip.org, abgerufen): G. Weinreich, „Coupled piano strings“, J. Acoust. Soc. Am. 62(6), 1474–1484, 1. Dezember 1977, DOI 10.1121/1.381677.
- **Zugang:** Bezahlschranke. Die Seite meldet „You do not currently have access“, Einzelkauf $40.
- Laut Abstract auf derselben Seite enthält der Artikel Experimente „on string coupling and the two polarizations“. Den Volltext habe ich nicht gesehen.
- **Freie Fassung:** keine gefunden.
- **Freier Ersatz vom Autor selbst:** G. Weinreich, „The coupled motion of piano strings“, 2. Vorlesung aus *Five lectures on the Acoustics of the Piano*, © 1990 Royal Swedish Academy of Music.
  - Abschnitt „Vertical and horizontal motion“: https://www.speech.kth.se/music/5_lectures/weinreic/motion.html, Folgeseite `strings.html`.
  - Abgerufen; der Abruf zeigt keine Gleichungen.
  - Inhalt, als Zitat aus dem Abruf: Der Steg „gives“ much more easily in the vertical than in the horizontal direction. Und: „the vertical polarization is the primary one excited by the hammer“.
  - Zahlen zu Fig. 1: Der erste Schall fällt mit etwa 8 dB/s ab, der Nachklang mit weniger als einem Viertel davon. Die Seite widerspricht sich bei der Note (Bildunterschrift Eb3 = 311 Hz, Text Eb4).

## 2. Arbeiten mit zwei Polarisationen und Kopplung am Steg

**A. Aramaki, Bensa, Daudet, Guillemain, Kronland-Martinet**, „Resynthesis of coupled piano strings vibrations based on physical modeling“, DAFx-99, Trondheim 1999.
- PDF frei: https://www.dafx.de/paper-archive/1999/aramaki.pdf, 4 Seiten, 58 KB. Volltext gesehen.
- Er passt **nicht** zur Frage nach zwei Ebenen. Wörtlich: „we shall consider only the vertical polarisations“.
- Zwei Saiten werden durch zwei komplexe Koppelfilter F, H gekoppelt, als Wellenleiter. Übertragungsfunktion T1 = G1/(1 − H F G1 G2); die Lösung ist eine Summe von Paaren gedämpfter Sinusschwingungen.
- Das Modell mit zwei Polarisationen einer Saite steht laut Ref. [4] in Daudet, Guillemain, Kronland-Martinet, „Resynthesis of Piano Strings Vibrations Based on Physical Modelling“, ICMC 1999 Peking. Nicht abgerufen.
- Die Zeitschriftenfassung in J. New Music Research 30(3), 2001 kenne ich nur aus einer Suchmaschinen-Zusammenfassung; nicht geprüft.

**B. Julien Bensa**, „Analyse et synthèse de sons de piano par modèles physiques et de signaux“, Dissertation 2003.
- HAL tel-00003588, PDF https://theses.hal.science/tel-00003588/document. Laut HAL-API frei; Volltext nicht gesehen.
- Laut API-Abstract: gekoppelte Wellenleiter „that reproduce beating“ und ihre Kalibrierung aus Modalparametern. Ob dort zwei Polarisationen vorkommen, ist ungeprüft.

**C. Jacques Cuenca, René Caussé**, „Three-dimensional interaction between strings, bridge and soundboard in modern piano's treble range“, 19th ICA 2007.
- HAL hal-01161420, PDF https://hal.science/hal-01161420/document. Laut API frei; Volltext nicht gesehen.
- **Der inhaltlich nächste Fund.** Laut Abstract (API und Suchergebnis) wird je Bauteil eine 3×3-Eingangsadmittanzmatrix gebildet und über Stetigkeitsbedingungen zusammengesetzt.
- Ergebnis laut Abstract: Die zwei Biegepolarisationen einer Saite beeinflussen sich über die Elastizität des Stegs, Saiten im Chor ebenso; daraus entstehen Schwebung und doppeltes Abklingen. Verglichen wird mit einem vereinfachten Klavier.
- **Zuerst lesen.**

**D. Jin Jack Tan**, „Piano acoustics: string's double polarisation and piano source identification“, Dissertation, Université Paris-Saclay, 2017.
- HAL tel-01755039, PDF https://pastel.hal.science/tel-01755039/document. Laut API frei; Volltext nicht gesehen.
- Laut API-Abstract hat Teil 2 die Kopplung Saite–Steg zum Gegenstand: „numerical schemes“ und Experimente am Monochord, die doppelte Polarisation und doppeltes Abklingen zeigen.
- **Wahrscheinlich die vollständigste Fassung in Gleichungen. Ebenfalls zuerst lesen.**

**E. Tan, Touzé, Cotté**, „Double polarisation in nonlinear vibrating piano strings“, Proc. Third Vienna Talk 2015, S. 182–187.
- PDF frei: https://perso.ensta.fr/~touze/PDF/viennatalk2015_JJT.pdf (auch HAL hal-01194580), 6 Seiten, 290 KB. Volltext gesehen.
- Er behandelt die **nichtlineare** Kopplung (Kirchhoff-Carrier, Gl. 1–4, innere Resonanz 1:1), **nicht** die Kopplung am Steg.
- Zwei Aussagen sind für uns nützlich:
  - S. 1: Die zweite Ebene werde „after a few milliseconds“ angeregt, mit vergleichbarer Amplitude.
  - S. 1: Die doppelte Polarisation sei bei Chabassier nicht modelliert; das erkläre womöglich das fehlende doppelte Abklingen.
- Seine Literaturliste nennt weitere Ursachen: Hundley/Benioff/Martin 1978, Suzuki/Nakamura 1990, Burred 2004, Karatsovis 2011 (Southampton), Chaigne/Chabassier/Burban SMAC 2013.

**F. Pablo Miranda Valiente, G. Squicciarini, D. Thompson, D. O. Norris, C. Hernandez**, „Modelling the una-corda effect in pianos“, J. Phys.: Conf. Ser. 2909 (2024) 012036, doi:10.1088/1742-6596/2909/1/012036.
- Offener Zugang, CC BY 4.0. PDF https://iopscience.iop.org/article/10.1088/1742-6596/2909/1/012036/pdf, 13 Seiten, 1,3 MB. Volltext gesehen.
- Zwei Saiten mit je drei Richtungen (T, L, P) am Resonanzboden.
- Gl. 4 (S. 4): 3×3-Mobilitätsmatrix am Steg aus einem FE-Modell, auf 102 Moden reduziert.
- Gl. 6–7 (S. 5): Kontakt als Feder und Dämpfer je Richtung. Hertz-Steifigkeit „4.8 × 10^6 N/m“; Dämpfer 10 Ns/m „to provide numerical stability“.
- Gl. 14–16: Der Hammer greift nur in einer Richtung an, die Komponenten der anderen Richtungen sind null. Die zweite Ebene entsteht also allein über den Steg.
- Zustandsraum mit Modalkoordinaten.
- Vorarbeiten (nicht abgerufen):
  - Valiente/Squicciarini/Thompson, Internoise 2022, „Piano string vibration modelling using coupled mobilities and a state-space approach“. Das PDF bei eprints.soton.ac.uk gab 403 zurück.
  - Dieselben Autoren, Vienna Talk 2022.

**G. Julius O. Smith, *Physical Audio Signal Processing*** (CCRMA, frei).
- Seite „Coupled Horizontal and Vertical Waves“: https://ccrma.stanford.edu/~jos/pasp/Coupled_Horizontal_Vertical_Waves.html. Abgerufen.
- 2×2-Koppelmatrix H(z) am Steg; im symmetrischen Fall [[A,B],[B,A]] mit den Eigenvektoren [1,1] und [1,−1] und den Eigenwerten A±B.
- Wörtlich: „The ith eigenvalue of the coupling matrix equals the frequency response seen by the ith eigenpolarization.“
- Nur Wellenleiter, ohne Admittanz in Zahlen.

**Nur am Rand, ohne zwei Ebenen am Klavier:**
- Thomas, Rousseau, Caussé, Marandas, ISMA 1998, hal-01105499: laut API nur „vertical string coupling“.
- Ege und Chaigne, „End conditions of piano strings“, ISMA 2007, arXiv:1101.4511: Volltext gesehen; eine Polarisation, die horizontale wird ausdrücklich vermieden („otherwise we get double peaks“).
- Gough 1981, „The theory of string resonances on musical instruments“, Acustica 49, 124–141: nur als Literaturangabe gefunden, nicht frei.
- Woodhouse 2004, „On the synthesis of guitar plucks“, Acta Acustica 90, 928–944: Gitarre; nur als Literaturangabe gefunden, nicht frei.

## Messwerte zur horizontalen Ebene

Keine Zahl zur Admittanz des Stegs horizontal gegen vertikal gefunden, auch keine zur Anregung der horizontalen Ebene durch den Hammer. Am nächsten kommen:
- Weinreichs Vorlesung: qualitativ, dazu die Abklingraten (siehe oben).
- Tan 2015: „similar order“ nach einigen Millisekunden.
- Valiente 2024 (Abb. 2): gerechnete Mobilitäten eines FE-Bodens, ohne Messung.

Kandidaten für Messungen, nicht abgerufen: Tan 2017 Teil 2, Cuenca/Caussé 2007, Hundley et al. 1978 (JASA 64, 1303–1309, laut einer Literaturliste), Karatsovis 2011 (eprints.soton.ac.uk/333304).

## Reichweite der Suche

- **Suchmaschine (WebSearch):** Weinreich-Titel und DOI; Aramaki/Bensa; Bensa-Dissertation; Gough 1981; „two polarizations bridge admittance matrix piano“; Tan/Touzé; Hundley 1978; Valiente/Squicciarini; Cuenca/Caussé; Woodhouse 2004; Lee/Smith/Välimäki.
- **HAL-API:** Autor Bensa; Titel mit polarization/polarisation/„coupled strings“/„double decay“ zusammen mit piano/string.
- **Abgerufen:** pubs.aip.org, dafx.de, CCRMA (PASP), speech.kth.se, perso.ensta.fr, iopscience, arxiv.
- **Gesperrt oder fehlgeschlagen:** HAL-Seiten (Anubis), eprints.soton.ac.uk (403), ioa.org.uk/Baxandall (404).

**Nicht gefunden:**
- eine freie Fassung von Weinreich 1977;
- ein freies PDF von Lee/Smith/Välimäki, „Analysis and Synthesis of Coupled Vibrating Strings Using a Hybrid Modal-Waveguide Synthesis Model“ (IEEE, nur die Seite mit Klangbeispielen bei CCRMA);
- eine Zeitschriftenfassung von Tans Kopplung am Steg;
- Arbeiten von Issanchou oder Bilbao mit zwei Polarisationen am Klavier (nicht gezielt gesucht);
- Messzahlen zur horizontalen Admittanz.

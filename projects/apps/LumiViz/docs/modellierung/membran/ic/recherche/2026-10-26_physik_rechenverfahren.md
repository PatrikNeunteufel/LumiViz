# Recherche IC1: Membran — Physik und Rechenverfahren

> **Auftrag:** Klasse 5 (Pauke, Toms, Bassdrum, Snare, Conga, Bongo, Tabla): Moden mit Luftlast, zweifellige Trommeln,
> Snare-Teppich, Schlägel und Stock, Spannungsmodulation, Tabla/Mridangam, Rechenverfahren und Kosten — mit Zahlen;
> Abgleich mit `gemeinsam/koerper.md`.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten im PDF gelesen (Seitenbild oder `pdftotext`); [wörtlich,
> Werkzeug] = vom Abrufwerkzeug als Zitat/Feldinhalt; [nur Zusammenfassung] = sonst; [eigene Rechnung] = kein Beleg.
> **Reichweite:** im Bericht selbst (12 Suchen, 34 Abrufe; HAL-Volltext Rhaouti 1999, Kirby & Sandler 2021,
> Rossing 1992 Physics Today nicht erreicht; arXiv-Lizenzen offen).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../messplan.md`, `../offene-fragen.md`, `../quellen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.
> Packer 1993 verlangt für Zitate die schriftliche Zustimmung des Autors — aus dieser Arbeit werden nur Zahlen
> übernommen.

---

# Bericht IC – Klasse 5 „Membran“: Physik und Rechenverfahren, mit Zahlenwerten

**Auftrag:** Physik und Rechenverfahren der Membran-Instrumente (Pauke, Toms, Bassdrum, Snare, Conga, Bongo, Tabla) mit Zahlenwerten, als Vorarbeit für vorab gerechnete Töne in LumiViz.
**Datum:** 2026-10-26

**Reichweite**
- 12 Websuchen und 34 Abrufe mit WebFetch. Dazu kamen Textauszüge mit `pdftotext` und das eigene Lesen der Seiten zwischengespeicherter PDFs mit Read. Geschrieben und gesondert heruntergeladen habe ich nichts.
- Orte: arXiv (API, Kurzseiten, PDFs), Crossref-API (JASA-Abstracts), DAFx-Archiv, Edinburgh Research Explorer, HAL-API, Durham E-Theses, KTH, NIU Huskie Commons, FAU CRIS (nur Suchtreffer).
- Sprachen: Englisch, Französisch (HAL-Abstracts), Deutsch.
- Nicht erreichbar:
  - HAL-Volltext Rhaouti/Chaigne/Joly 1999: Bot-Sperre Anubis, nur der Abstract über die API.
  - JASA-Volltext Kirby & Sandler 2021: AIP 404, das QMUL-Repositorium lieferte eine leere Seite.
  - Physics Today 1992 (Rossing): PDF größer als 10 MB, Abruf abgebrochen.
  - Cornell-Kopie Marogna/Avanzini/Bank: Zertifikat abgelehnt. Ich habe stattdessen die DAFx-Fassung gelesen.
  - Uni der Bundeswehr München, Tabla-Studie 2003: Verbindung verweigert.
  - Crossref einmal 429 (Bilbao, Snare in JASA 2012).
  - AES-Fassung Bilbao & Webb 2013: hinter Bezahlschranke. Gelesen habe ich die DAFx-12-Fassung.
  - Bei arXiv hat das Abrufwerkzeug die Lizenzfelder nicht geliefert, weil es meine Rückfrage für eine eingebettete Anweisung hielt. **Die Lizenzen der arXiv-Quellen sind deshalb offen.**
- **Zwischengespeicherte PDFs** (alle im Ordner `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):
  - `webfetch-1793039109154-rcgpu2.pdf`: Sathej & Adhikari, arXiv 0809.1320
  - `webfetch-1793039136640-tt3183.pdf`: Torin, Hamilton, Bilbao, DAFx-14
  - `webfetch-1793039163615-nsptvs.pdf`: Bilbao & Webb, DAFx-12
  - `webfetch-1793039208327-xtx91m.pdf`: Avanzini & Marogna, IEEE TASLP 2010
  - `webfetch-1793039231421-z7ckeh.pdf`: Marogna, Avanzini, Bank, DAFx-10
  - `webfetch-1793039330012-wv27sl.pdf`: Packer 1993, Durham MSc
  - `webfetch-1793039136885-179zzx.pdf`: Wagner 2006, KTH MSc
  - `webfetch-1793039413752-zzhpum.pdf`: Torin & Newton, „Collisions in Drum Membranes“
  - `webfetch-1793039470326-rqa9jm.pdf`: Bilbao, Torin, Chatziioannou, arXiv 1405.2589
  - `webfetch-1793039548689-d5kw44.pdf`: Kumar 2023, arXiv 2307.09425

**Kennzeichnung**
- [wörtlich, selbst gelesen, S. x]: im PDF selbst gelesen, als Seitenbild oder über `pdftotext`. „S.“ ist die gedruckte Seite, „PDF-S.“ die Seite in der Datei.
- [wörtlich, Werkzeug]: Zitat oder Feldinhalt, den das Abrufwerkzeug geliefert hat.
- [nur Zusammenfassung]: sonst.
- [eigene Rechnung]: von mir gerechnet, kein Beleg.

---

## 0. Abgleich mit der Vorarbeit

| Behauptung | Befund | Kennzeichnung |
|---|---|---|
| Die Luftlast bestimmt die Tonhöhen der Pauke (Christian et al. 1984). | **Bestätigt.** Im Crossref-Abstract steht „frequency ratios f11 : f21 : f31 : f41 are found to be close to the harmonic ratios 2 : 3 : 4 : 5“, die Luftlast wird über eine Green-Funktion gerechnet. Unabhängig davon zeigt Packer 1993, dass die Luftlast die Moden mit nur radialen Knotenlinien an die harmonische Reihe heranführt (Tab. 4.1, S. 47). | [wörtlich, Werkzeug]; Packer [selbst gelesen, S. 47] |
| Zwei Felle koppeln über die eingeschlossene Luft (Zhao 1990). | **Bestätigt, nur über den Abstract.** Gemeint ist eine MS-Thesis (NIU, Betreuer Rossing). Im Abstract stehen „acoustical coupling through the air enclosure“ und „A two-mass/three-spring model“ für die Mode 01. Die Messung bei Wagner 2006 zeigt dazu passend aufgespaltene Modenpaare (unten). | [wörtlich, Werkzeug]; Wagner [selbst gelesen] |
| FDTD mit 3D-Luft ist teuer (Bilbao & Webb 2013). | **Bestätigt (DAFx-12-Fassung).** MATLAB auf einem Intel Core i7 braucht 28,0 min für 1 s Ton bei 44,1 kHz, die GPU (Tesla C2050) 82 s für 1 s Ton. | [wörtlich, selbst gelesen, DAFX-4 und DAFX-6] |
| Tonhöhengleiten nach dem Schlag (Avanzini, Marogna & Bank 2012). | **Bestätigt.** Crossref (JASA 2012, DOI 10.1121/1.3651097) liefert „Pitch glides due to tension modulation“ und „only slightly more computational resources than linear models“. Die DAFx-10-Vorfassung habe ich selbst gelesen (Abschnitt 5). | [wörtlich, Werkzeug]; DAFx-10 [selbst gelesen] |

---

## 1. Pauke: Moden mit Luftlast

### Ideale Membran im Vakuum (Bessel-Nullstellen)

| Mode | f/f(0,1) | f/f(1,1) |
|---|---|---|
| (0,1) | 1,000 | 0,628 |
| (1,1) | 1,593 | 1,000 |
| (2,1) | 2,136 | 1,340 |
| (0,2) | 2,295 | 1,441 |
| (3,1) | 2,653 | 1,665 |
| (1,2) | 2,917 | 1,831 |
| (4,1) | 3,155 | 1,980 |
| (5,1) | 3,647 | 2,289 |

Kennzeichnung: [eigene Rechnung, scipy `jn_zeros`, mathematische Tatsache]. Kumar 2023 nennt dieselbe Reihe „1:1.59:2.14:2.30:2.65…“ [wörtlich, selbst gelesen, PDF-S. 15].

### Pauke mit Luft und Kessel: 64-cm-Pauke, Spannung 5360 N/m (Packer 1993)

Die Tabellen stehen auf S. 47 (Tab. 4.1, gerechnet), S. 57 (Tab. 6.1, gerechnete T60) und S. 63 (Tab. 7.1 und 7.2, gemessen).

| Mode | gerechnet (Hz) | Verhältnis zu (1,1) | gemessen (Hz) | Verhältnis zu (1,1) | T60 gerechnet (s) | T60 gemessen (s) |
|---|---|---|---|---|---|---|
| (0,1) | 138 | 0,80 | 140 | 0,81 | 0,06 | <0,3 |
| (1,1) | 172 | 1,00 | 172 | 1,00 | 0,8 | 0,8 |
| (2,1) | 261 | 1,52 | 258 | 1,50 | 1,8 | 1,7 |
| (0,2) | 291 | 1,69 | 284 | 1,65 | 0,2 | 0,4 |
| (3,1) | 344 | 2,00 | 340 | 1,97 | 4,6 | 2,7 |
| (1,2) | 390 | 2,27 | 344 | 2,00 | 0,2 | 0,5 |
| (4,1) | 427 | 2,48 | 420 | 2,44 | 10,5 | 1,7 |
| (2,2) | 471 | 2,74 | 493 | 2,86 | 0,8 | 0,5 |
| (0,3) | 511 | 2,97 | 467 | 2,71 | 0,1 | <0,3 |
| (5,1) | 506 | 2,94 | 501 | 2,91 | 16,2 | 2,6 |

Kennzeichnung: [wörtlich, selbst gelesen, S. 47/57/63]. Die Zahlen habe ich am Seitenbild geprüft. Der Textauszug verschiebt die Tabellenzeilen, ihm darf man nicht trauen.

- **Ältere Reihen bei Packer:** Rayleigh 1,00:1,50:1,88:2,00, Benade 1,00:1,504:1,742:2,000, Rossing (1982) 1,00:1,50:2,00:2,44:2,90 [selbst gelesen, S. 60; nur Zahlen].
- **Rechtsvermerk Packer:** Die Arbeit verlangt für Zitate die schriftliche Zustimmung des Autors. Ich gebe deshalb nur Zahlen wieder und keine Sätze.
- **Mylar, Packer Anh. B (S. 73):**
  - Dichte 1650 kg/m³, E = 3,5 GPa, ν = 0,49
  - Fell 1,9·10⁻⁴ m dick
  - Radius 0,32 m (kleine) bzw. 0,355 m (große Pauke)
  - Flächendichte 0,262 kg/m²
  - Kennzeichnung: [wörtlich, selbst gelesen, S. 73]
  - **Achtung, in sich widersprüchlich:** 1650 × 1,9·10⁻⁴ ergibt 0,31 kg/m², nicht 0,262. Zu 0,262 passt eine Dichte um 1380 kg/m³, wie sie für PET üblich ist [eigene Rechnung].
- **Kesselvolumen:** Packer berichtet nahezu harmonische Verhältnisse für L/a = 1,2 [selbst gelesen, S. 48].
- **Durchmesser:** üblich ist ein Paar aus 71 und 64 cm (28 und 25 in) [selbst gelesen, S. 8].
- **Luftlast über der Frequenz:** Oberhalb etwa 500 Hz wird die Luftlast vernachlässigbar [nur Zusammenfassung des Textauszugs, S. 38].
- **Rickey, St-Amant, Smith 1986, JASA (DOI 10.1121/1.2023471):** Klangspektren einer Pauke in der Vakuumkammer bei Drücken von 0,01 bis 1 atm [nur Zusammenfassung, Crossref].
- **Bertsch 2001, Wiener Pauke (HAL hal-05705482):** „the 'quasiharmonic' modes 11 and 31 have higher amplitudes“ [wörtlich, Werkzeug, HAL-API]. Keine Lizenzangabe.

### Rechenwege

- **Green-Funktion bzw. Rayleigh-Integral, modal (Christian et al. 1984):**
  - Eigenwertgleichung mit Luftlast als Integral, iterativ bis etwa 0,5 % gelöst. Bei Packer ist das Verfahren auf S. 47 nachgezeichnet (Gl. 4.36) [selbst gelesen].
  - Ergebnis ist eine Modenliste aus Frequenz und Abklingzeit. Das passt unmittelbar zu LumiViz.
- **3D-FEM im Zeitbereich (Rhaouti, Chaigne, Joly 1999, JASA 105, 3545–3562):**
  - Abstract: „derived by means of three-dimensional finite elements“, „coupled with both the external and internal sound field“, „the nonlinear interaction with the membrane“ (Schlägel) [wörtlich, Werkzeug, Crossref/HAL-API].
  - Auf HAL liegt die Arbeit mit der Lizenz **CC BY-NC 4.0** (Lizenzfeld `https://creativecommons.org/licenses/by-nc/4.0/`, HAL-API, abgerufen 2026-10-26). Der Volltext war durch Anubis gesperrt.
- **Modale Näherung mit Massen- bzw. Steifezuschlag (Avanzini & Marogna 2010, S. 896):**
  - Moden mit Knotendurchmessern (n > 0): Die Luft wirkt als träge Last und senkt die Frequenz durch zusätzliche Modenmasse.
  - Achsensymmetrische Moden (n = 0): Die Luft wirkt als zusätzliche Steife.
  - Kennzeichnung: [selbst gelesen, S. 896].
- **3D-FDTD (Bilbao & Webb):**

  | Größe | Wert |
  |---|---|
  | Membrangitter | 127 × 127, also 16 129 Punkte |
  | Luftvolumen | 75 × 75 × 75, also 421 875 Punkte |
  | Abtastrate | 44,1 kHz |
  | Rechenzeit MATLAB auf i7 | 28 min je s Ton |
  | Rechenzeit GPU | 82 s je s Ton |
  | Rechenzeit für 3 s Ton | 4 min statt fast 1,5 h |
  | Anteil Membran am Rechenkern | etwa 70 %, davon der lineare Löser 48,76 % |

  - Kennzeichnung: [wörtlich, selbst gelesen, DAFX-4 bis DAFX-6].
  - Die Autoren urteilen selbst, das Modell erfasse den Klang bei starken Schlägen nicht ausreichend, und nennen „membrane itself“ als Kandidaten [selbst gelesen, DAFX-7].
  - Die Pauke wird in einen Würfel von 1 m³ eingebettet, der Kessel ist der Einfachheit halber ein Paraboloid [selbst gelesen, DAFX-2].

---

## 2. Zweifellige Trommeln (Tom, Bassdrum, Snare)

**Messung an der Snare mit Schlag am Rand (Wagner 2006, KTH-MSc, Tab. 4.3, S. 36):**

| Mode | nur Schlagfell | Schlag- und Resonanzfell, Paar 1 | Paar 2 |
|---|---|---|---|
| (0,1) | 188 Hz | 240 Hz | 366 Hz |
| (1,1) | 352 Hz | 349 Hz | 407 Hz |

- Kennzeichnung: [wörtlich, selbst gelesen, S. 36, am Seitenbild geprüft].
- Jede Mode spaltet sich in ein gleichphasiges und ein gegenphasiges Paar auf.
- Rechtsvermerk: In der Thesis habe ich keinen gefunden (`pdftotext`-Suche nach „copyright“, „licen“, „©“). Kein Treffer ist kein Beweis.

**Rossing, Bork, Zhao, Fystrom 1992, „Acoustics of snare drums“, JASA (DOI 10.1121/1.404080)**
- Bei höheren Frequenzen „the drum heads vibrate more or less independently“.
- „The snares move in a complex way, being in contact with the snare head“.
- Kennzeichnung: [wörtlich, Werkzeug, Crossref]. Der Abstract enthält keine Zahlen, den Volltext habe ich nicht erreicht.

**Fystrom & Rossing 1990, Snare-Kessel (DOI 10.1121/1.2027972)**
- „The modal decay rate in the free shell increases rapidly with modal frequency.“
- Nach dem Abstract fließt „a considerable amount of vibrational energy“ in den Kessel.
- Kennzeichnung: [wörtlich, Werkzeug].

**Modell bei Avanzini & Marogna 2010 (S. 896–899)**
- Die Luftkopplung ist konzentriert als Feder und Dämpfer zwischen den mittleren Auslenkungen beider Felle (Gl. 19). Sie wirkt nur auf (0,m)-Moden.
- Modenpaare für n > 0, also Doubletten, erfasst das Modell ausdrücklich nicht.
- Simulierte Spannungen:
  - T₁ = 2000 und T₂ = 1800 N/m: Schwebungen.
  - T₁ = 2000 und T₂ = 1100 N/m: keine Schwebung, dafür ein reicheres Spektrum.
- Kennzeichnung: [selbst gelesen, S. 896 und 899].

**Zahlen zur Bassdrum:** **Nicht gefunden**, siehe unten.

---

## 3. Snare-Teppich: nichtlinearer Kontakt

**Torin, Hamilton, Bilbao 2014, DAFx-14, Erlangen**

Modell [selbst gelesen, DAFX-2 bis DAFX-7]:
- Zwei Felle, ein starrer Kessel, Luft in 3D.
- Jede Teppichsaite ist eine 1D-Saite mit Verlusten. Der Kontakt ist eine Potenzgesetz-Straffunktion (penalty) Φ = κ/(β+1)·[η]₊^(β+1).
- Ein energieerhaltendes Schema, je Zeitschritt mit einer nichtlinearen Gleichung (Gl. 44 und 50).
- Snares haben üblicherweise „12-15 in number“ Drähte, in den Beispielen werden 7 gerechnet.
- Schlägel: 0,028 kg, Anfangsgeschwindigkeit −5 m/s, Abtastrate 44 100 Hz.

Kosten und Probleme:
- Die 3D-Dispersion erzeugt hörbares „Zischen“. Abhilfe schafft viskothermische Dämpfung mit σa = 2·10⁻⁶ m.
- Überabtasten wäre die Alternative, kostet aber laut Text das 16-Fache je Verdopplung der Abtastrate.
- In [3] (Bilbao & Webb) wurde für eine Ausgabe bis 700 Hz mit 24 kHz gerechnet.
- Rechenzeiten nennt das Papier nicht.

Rechtsvermerk und Lizenz:
- Erste Seite: „© Torin, A., Hamilton, B., & Bilbao, S. (2014)…“ und „Publisher's PDF“.
- „General rights: Copyright for the publications made accessible via the Edinburgh Research Explorer is retained by the author(s)…“ [wörtlich, selbst gelesen, Deckblatt; Abruf 2026-10-26].
- Keine CC-Lizenz.

**Bilbao, Torin, Chatziioannou, „Numerical Modeling of Collisions in Musical Instruments“** (arXiv 1405.2589; im PDF-Kopf steht „Acta Acustica united with Acustica Vol. 98 (2012)“)
- Ältere Snare-Simulationen behandelten den Teppichkontakt „ad hoc“ und ohne Stabilitätsbedingung.
- Beispiel Schlägel und Membran: Masse 0,028 kg, K = 1,6·10⁸, α = 2,54; Membran 0,26 kg/m², Tm = 3325 N/m, Abtastrate 22 050 Hz.
- Die Snare-Simulation läuft mit 48 kHz.
- Kennzeichnung: [selbst gelesen, PDF-S. 14–16].
- Lizenz: **nicht ermittelt**.

**Avanzini & Marogna 2010, konzentrierte Alternative (S. 897)**
- Der verteilte Kontakt kostet N·M Operationen je Abnahmepunkt. Deshalb wird nur die erste Mode einer steifen Saite gerechnet, die das Fell in der Saitenmitte berührt.
- Kennzeichnung: [selbst gelesen, S. 897].

**Torin & Newton, „Collisions in Drum Membranes: a preliminary study…“** (Edinburgh, NESS-Server)
- Versuchsaufbau: eine Bodhrán (40 cm, Ziegenfell) mit einer einzelnen Saite als Teppich, gemessen mit Hochgeschwindigkeitskamera und Laservibrometer [selbst gelesen, S. 1].
- Die Parametertabelle ist im Textauszug verschoben, ich übernehme keine Zahlen daraus.
- Tagungsort im PDF nicht ermittelt.

---

## 4. Schlägel und Stock: Kontakt

| Größe | Wert | Quelle | Kennzeichnung |
|---|---|---|---|
| Kontaktzeit Stock und Snare-Fell | 3–6 ms | Wagner 2006, Abstract | [wörtlich, selbst gelesen, PDF-S. 3] |
| Beispiel: Stock verlässt das Fell nach | ≈3,5 ms | Wagner, Kap. 4.1 | [selbst gelesen, PDF-S. 26] |
| Kontaktzeit bei höherer Spannung oder näher am Rand | kürzer | Wagner, Abb. 4.7 und 4.10 | [nur Zusammenfassung des Textauszugs] |
| Eigenmoden des Stocks | ≈400 Hz, 1,0 kHz, 1,7 kHz, 2,7 kHz | Wagner | [selbst gelesen, PDF-S. 28] |
| Kontaktkraft (Avanzini/Marogna) | Hunt-Crossley: F = kζ^α + λζ^α·ζ̇ für ζ > 0 | 2010, Gl. 9 | [selbst gelesen, S. 893] |
| Kontaktkraft (Bilbao/Torin) | Hertz-Potenzgesetz f = κ[η]₊^α, α > 1 | DAFx-14, Gl. 10 | [selbst gelesen, DAFX-3] |
| Paukenschlägel (Bilbao & Webb) | nicht gerechnet, sondern vorgegebener Kraftverlauf (Kosinus-Puls) | DAFx-12, Abschn. 2.6 | [selbst gelesen, DAFX-3] |

- **Schlagstelle:** Nahe der Mitte geht die Energie vor allem in die tiefsten Moden, nahe dem Rand in höhere (Avanzini & Marogna, S. 898, Abb. 6) [selbst gelesen].
- **Schlag in die Paukenmitte:** Er erzeugt nach Packer einen tonlosen „thwack“ [selbst gelesen, S. 60; nur Inhalt, kein Zitat wegen des Rechtsvermerks].
- **Filzschlägel (Stulov, Chaigne & Doutaut):** **Nicht gelesen.** Belegt ist nur, dass das IRCAM-Programm Modalys ein Filzmodell nach Stulov mit Hysterese führt, und das nur über eine Suchmaschinen-Zusammenfassung [nur Zusammenfassung].
- **Kontaktzeit von Paukenschlägeln in ms:** **nicht gefunden.**

---

## 5. Nichtlinearität: Spannungsmodulation und Tonhöhengleiten

**Modell nach Avanzini & Marogna 2010 (Berger-Näherung zu von Kármán)**
- Die Spannung T_NL ist proportional zur Flächenzunahme. Modal ergibt sich T_NL = Eh/(4R²(1−ν²))·Σ μ²z²/‖K‖² (Gl. 15).
- Pro Schritt entsteht eine verzögerungsfreie Schleife, die iterativ gelöst wird.
- Parameter (S. 898):

  | Größe | Wert |
  |---|---|
  | R | 16–20 cm |
  | h | 0,2 mm |
  | σ | 0,27 kg/m² |
  | E | 3,5·10⁹ Pa |
  | ν | 0,2 |
  | d₁ | 1,25 kg/(m²s) |
  | d₃ | 5·10⁻⁴ kg/s |
  | Abtastrate | 44,1 kHz |
  | Modenzahl | bis 420 |

- Das Gleiten dauert „a few tenths of seconds“ (S. 899).
- Kennzeichnung: [selbst gelesen, S. 894–899].

**Energie-Abkürzung (Marogna, Avanzini, Bank, DAFx-10)**
- Der quasistatische Spannungsanteil ist T_qs = Eh/(2πR²(1−ν²)T₀)·E(t) (Gl. 20).
- Die Energie darf um den Faktor 32 unterabgetastet gerechnet werden, also alle ≈0,73 ms. Dann ist der Zusatzaufwand „negligible“ (Abstract und DAFX-6).
- 20 m/s Schlaggeschwindigkeit liegt nach den Autoren nahe den höchsten Dynamikstufen.
- Kennzeichnung: [wörtlich, selbst gelesen, DAFX-1 und DAFX-6].

**Größe in Cent**

| Fall | Wert | Kennzeichnung |
|---|---|---|
| Gemessen, Snare: (0,1) bei 140 Hz steigt vom leisen zum lauten Schlag um etwa 3 Hz (Wagner, Abb. und Text S. 26–27) | 3 Hz entsprechen ≈ **37 Cent** | Messung [selbst gelesen, PDF-S. 32–33]; Umrechnung [eigene Rechnung] |
| Simulation DAFx-10, Abb. 1 und 2: T₀ = 800 N/m, Schlag mit 5 m/s, T_qs ≈ 100 N/m nach etwa 10 ms und ≈ 50 N/m nach 0,1 s | ≈ +100 Cent nach 10 ms, ≈ +50 Cent nach 0,1 s | Ablesung aus der Abbildung und Rechnung [eigene Rechnung, unsicher] |
| Kirby & Sandler 2021, JASA 150, 202–214, DOI 10.1121/10.0005509: Tom „struck at 67 different intensities“, Analyse mit der DCT | Der Abstract nennt keine Größe in Cent | [wörtlich, Werkzeug, Crossref]. Lizenzfeld laut Crossref `https://creativecommons.org/licenses/by/4.0/` (abgerufen 2026-10-26); Volltext nicht erreicht |
| Pauke, Bilbao & Webb: Tonhöhengleiten | „a minor effect in timpani“ | [wörtlich, selbst gelesen, DAFX-6] |

---

## 6. Tabla, Mridangam, Bayan; Conga und Bongo

**Sathej & Adhikari, „The eigenspectra of Indian musical drums“** (arXiv 0809.1320, 2008; laut arXiv-Seite auch JASA, DOI 10.1121/1.3058632)
- **Fell und Belastung:** Das syahi (sihai) ist „approximately eight times as dense“ wie das Leder und bedeckt etwa ein Viertel der Fläche [wörtlich, selbst gelesen, S. 2].
- **Modell:** glatte tanh-Dichte, gelöst mit Fourier-Chebyshev-Kollokation. Gitter Nr = 65 und Nθ = 30 (konzentrisch) bzw. Nr = 65 und Nθ = 56 (exzentrisch), Löser MATLAB `eigs` [selbst gelesen, S. 4].
- **Optimale Parameter:** σ = 2,57, k = 0,492, ξ = 0,091. Instrumentenbauer liegen bei 2,5 < σ < 4 und 0,45 < k < 0,55 [selbst gelesen, S. 5–6].
- **Luft:** Die Arbeit vernachlässigt sie. Sie würde vor allem die Moden mit Knotenkreisen anheben, am stärksten die Grundmode [selbst gelesen, S. 7].
- **Bayan:** Der Handballen wird längs des Durchmessers durch Mitte und syahi bewegt, um die Tonhöhe zu modulieren. Bis zu einer Exzentrizität von etwa 0,1 ändern sich die tiefen Eigenwerte kaum [selbst gelesen, S. 6–7]. Eine **Messung in Cent oder Hz** für das Biegen der Bayan-Tonhöhe habe ich **nicht gefunden**.
- Lizenz: **nicht ermittelt**.

Dayan, Tab. I bei σ = 3,125, k = 0,4, normiert auf den ersten Oberton [wörtlich, selbst gelesen, S. 5]:

| Mode | Experiment (Raman) | Verbundmembran | glattes Modell (Abweichung in Cent) |
|---|---|---|---|
| ψ01 | 1,03 | 1,0309 | 1,0345 (+7,6) |
| ψ11 | 2,00 | 2,0000 | 2,0000 |
| ψ21 | 3,00 | 3,0412 | 3,0393 (+22,5) |
| ψ02 | 3,00 | 3,1546 | 3,0534 (+30,5) |
| ψ31 | 4,00 | 4,0928 | 4,0086 (+3,7) |
| ψ12 | 4,00 | 4,2268 | 4,1463 (+62,2) |
| ψ03 | 5,04 | 4,9794 | 4,7784 (−92,3) |
| ψ41 | 5,03 | 5,1134 | 5,0023 (−9,6) |
| ψ22 | 5,08 | 5,3093 | 5,2491 (+56,7) |

Bayan, Tab. II bei k = 0,29 und ε = 0,18, Experiment [wörtlich, selbst gelesen, S. 7]: 0,54 / 0,95 / 1,00 / 1,52 / 1,54 / 1,75 / 2,06 / 2,1 / 2,32 / 2,36.

**Kumar 2023, Übersicht zum Mridangam** (arXiv 2307.09425)
- Raman: bis zu fünf harmonische Töne. Die Grundmode ist verschoben, nach Raman um etwa 10 %, tatsächlich um 7 %.
- Das Verhältnis dheem zu chappu stabilisiert sich bei 0,535, statt 0,5 bei rein harmonischer Lage, und das „accurate to within 0.05“ über mehrere Instrumente.
- Weitere Verhältnisse: 1,07 ± 0,01 und nam zu chappu 1,5 ± 0,012.
- Kennzeichnung: [selbst gelesen, PDF-S. 18–19 und 28].
- Lizenz: nicht ermittelt.

**Weitere Arbeiten:** Malu & Siddharthan 2000, „Acoustics of the Indian Drum“ (arXiv math-ph/0001030), nur in der Trefferliste gesehen, nicht gelesen.

**Conga und Bongo**
- Fletcher & Rossing ordnen Conga und Tom als ein Fell über einem offenen Kessel ein. Das ist belegt über Wagner, S. 3, wo die Einteilung zitiert wird [selbst gelesen].
- Eine **Messung der Moden mit offenem Kessel: nicht gefunden.**

---

## 7. Rechenverfahren und Kosten

**Modenzahl bis 10 kHz** [eigene Rechnung, ideale Membran, Doppelmoden doppelt gezählt; die Weyl-Formel N ≈ π²a²f²/c² stimmt auf etwa 1 % überein]:

| Fall | c | Moden bis 5 kHz | Moden bis 10 kHz |
|---|---|---|---|
| Pauke a = 0,32 m, T = 5360 N/m, σ = 0,262 kg/m² | 143 m/s | ≈ 1200 | ≈ **4900** |
| Tom a = 0,18 m, T = 2000 N/m, σ = 0,27 kg/m² | 86 m/s | ≈ 1050 | ≈ **4250** |
| Snare 14″, a = 0,178 m, T = 3325 N/m, σ = 0,26 kg/m², je Fell | 113 m/s | ≈ 590 | ≈ **2400** |

- Avanzini & Marogna: 15 × 15 = 240 Moden reichen bis ≈ 4 kHz, und die Frequenzverzerrung durch die bilineare Transformation (frequency warping) liegt dabei unter 3 %. Höhere Moden seien so dicht und so schnell abklingend, dass man sie als Anschlagsgeräusch wahrnimmt [selbst gelesen, S. 895].
- **Modal:** Je Mode und Abtastwert ist es ein Filter 2. Ordnung. 5000 Moden bei 44,1 kHz ergeben ≈ 2·10⁸ Multiplikationen und Additionen je s Ton, also ≈ 0,05–0,2 s auf einem Kern [eigene Rechnung, keine Messung].
- **FTM (Rabenstein, Petrausch):** Mit Polyphasen-Filterbank „over nine times faster at the cost of nine milliseconds of delay“ (DAFx-03, Petrausch & Rabenstein) [wörtlich, Werkzeug, DAFx-Archivseite]. Eine Version mit Spannungsmodulation lief in Echtzeit (EUSIPCO 2005) [nur Zusammenfassung]. Modenzahlen habe ich nicht gelesen.
- **FDTD bzw. NESS:** Pauke 3D: 28 min (CPU, MATLAB) bzw. 82 s (GPU) je s Ton (Tabelle in §1). Für die Snare in 3D wurden keine Rechenzeiten gefunden.
- **Vorab rechenbar:**
  - Modenlisten (Frequenz, T60, Modenform am Schlag- und Abnahmeort) je Instrument und Spannung, einmalig gerechnet mit BEM/FEM oder Green-Funktion, oder gemessen.
  - Die Energie-Skalierung für das Tonhöhengleiten ist ein einziger Faktor.

---

## Quellen

1. **Sathej, G.; Adhikari, R.** (2008): „The eigenspectra of Indian musical drums“, arXiv:0809.1320 [math-ph], auch JASA (DOI 10.1121/1.3058632). Frei. Lizenz **nicht ermittelt**.
2. **Torin, A.; Hamilton, B.; Bilbao, S.** (2014): „An Energy Conserving Finite Difference Scheme for the Simulation of Collisions in Snare Drums“, Proc. DAFx-14, Erlangen. Edinburgh Research Explorer, frei lesbar. Vermerk „© Torin, A., Hamilton, B., & Bilbao, S. (2014)…“ (Deckblatt, Abruf 2026-10-26), keine CC-Lizenz.
3. **Bilbao, S.; Webb, C.** (2012): „Timpani Drum Synthesis in 3D on GPGPUs“, Proc. DAFx-12, York. dafx.de, frei lesbar. Im PDF und auf der Archivseite kein Lizenzvermerk gesehen. Zeitschriftenfassung „Physical Modeling of Timpani Drums in 3D on GPGPUs“, JAES 61(10):737–748, 2013, hinter Bezahlschranke.
4. **Avanzini, F.; Marogna, R.** (2010): „A Modular Physically Based Approach to the Sound Synthesis of Membrane Percussion Instruments“, IEEE TASLP 18(4), 891–902. Kopie auf avanzini.di.unimi.it. Vermerk „1558-7916/$26.00 © 2010 IEEE“ und „Authorized licensed use limited to: ELETTRONICA E INFORMATICA PADOVA“ (S. 891). Nicht frei lizenziert.
5. **Marogna, R.; Avanzini, F.; Bank, B.** (2010): „Energy Based Synthesis of Tension Modulation in Membranes“, Proc. DAFx-10, Graz. dafx.de, frei lesbar. Kein Lizenzvermerk gesehen.
6. **Avanzini, F.; Marogna, R.; Bank, B.** (2012): „Efficient synthesis of tension modulation in strings and membranes based on energy estimation“, JASA, DOI 10.1121/1.3651097. Nur Abstract (Crossref). Crossref ohne Lizenzfeld.
7. **Packer, T. A.** (1993): „Acoustics of the orchestral kettledrum“, MSc, Durham University. Durham E-Theses, frei lesbar. Nutzungsregel: „The full-text may be used and/or reproduced … for personal research or study, educational, or not-for-profit purposes provided that…“. Titelblatt: „No quotation from it should be published without his prior written consent“ (PDF-S. 1–2, Abruf 2026-10-26).
8. **Wagner, A.** (2006): „Analysis of Drumbeats – Interaction between Drummer, Drumstick and Instrument“, Master's Thesis, KTH TMH. Frei lesbar. Kein Rechtsvermerk gefunden.
9. **Bilbao, S.; Torin, A.; Chatziioannou, V.**: „Numerical Modeling of Collisions in Musical Instruments“, arXiv:1405.2589; im PDF-Kopf „Acta Acustica united with Acustica Vol. 98 (2012)“. Lizenz nicht ermittelt.
10. **Torin, A.; Newton, M.**: „Collisions in Drum Membranes: a preliminary study on a simplified system“, PDF auf ness.music.ed.ac.uk. Tagungsort und Lizenz nicht ermittelt.
11. **Kumar, A. S.** (2023): „Musical Excellence of Mridangam: an introductory review“, arXiv:2307.09425. Lizenz nicht ermittelt.
12. **Christian, Davis, Tubis, Anderson, Mills, Rossing** (1984): JASA, DOI 10.1121/1.391449. Nur Abstract.
13. **Rossing, Bork, Zhao, Fystrom** (1992): JASA, DOI 10.1121/1.404080. Nur Abstract.
14. **Fystrom, Rossing** (1990): JASA, DOI 10.1121/1.2027972. Nur Abstract.
15. **Zhao, H.** (1990): „Acoustics of snare drums: an experimental study of the modes of vibration, mode coupling, and sound radiation patterns“, MS, NIU. Huskie Commons: „In Copyright“, „reproduction or distribution in any format is prohibited“ [wörtlich, Werkzeug].
16. **Rhaouti, Chaigne, Joly** (1999): JASA 105, 3545–3562. HAL hal-05707183, **CC BY-NC 4.0** (HAL-API). Volltext gesperrt.
17. **Kirby, T.; Sandler, M.** (2021): JASA 150, 202–214, DOI 10.1121/10.0005509. **CC BY 4.0** laut Crossref. Volltext nicht erreicht.
18. **Ramdane & Chaigne** (1992), J. Phys. IV, und **Ramdane** (1990), J. Phys. Colloques. HAL, Lizenz „hal-authorisation-v1“. Nur Abstract.
19. **Petrausch & Rabenstein** (2003), DAFx-03. Nur die Archivseite.

---

## Nicht gefunden

Kein Treffer ist kein Beweis: Die Suche war auf die genannten Orte und etwa 46 Abrufe beschränkt.

- Zahlen zur Bassdrum (Moden, Spannung, Kopplung).
- Toms: gemessene Modenfrequenzen mit zwei Fellen. Es gibt nur Simulationen bei Avanzini.
- Kontaktzeiten von Paukenschlägeln. Filzparameter nach Stulov sowie Chaigne & Doutaut habe ich nicht gelesen.
- Tonhöhengleiten der Toms in Cent aus einer Messung. Der Volltext von Kirby & Sandler war nicht erreichbar.
- Biegen der Bayan-Tonhöhe in Cent.
- Moden von Conga und Bongo mit offenem Kessel.
- Die Tabelle von Christian et al. 1984 im Wortlaut. Ersatzweise habe ich die Tabellen bei Packer genommen.
- Rechenzeiten für FDTD der Snare und für FTM-Membranen.
- Rossing & Sykes 1982 und Raman 1934/35 im Original.

---

## Eigene Einschätzung (kein Beleg)

**Was für vorab gerechnete Schläge trägt**
- Ein **modales Modell nach dem Muster des Flügels** passt:
  - je Fell eine Modenliste mit Luftlast, also Frequenz, T60 und Modenform am Schlag- und Abnahmeort;
  - Hunt-Crossley-Kontakt in modalen Koordinaten;
  - energiebasierte Spannungsmodulation nach Marogna, Avanzini und Bank für das Tonhöhengleiten;
  - bei zwei Fellen eine konzentrierte Luftfeder zwischen den (0,m)-Moden.
- Kosten etwa 0,05–0,2 s je s Ton auf einem Kern bei 2000–5000 Moden, also deutlich unter Echtzeit. Oberhalb von etwa 4–5 kHz lässt sich auf einen statistischen Teil umsteigen, weil die Moden dort dicht liegen und schnell abklingen.
- Die Modenlisten selbst würde ich **einmalig** aus Green-Funktion, BEM oder FEM mit Kessel rechnen, oder messen. Für die Pauke bilden Packers Zahlen ein Prüfziel.
- **Tabla und Mridangam** gehen gut modal: Die Eigenwertaufgabe nach Sathej ist billig und vorab rechenbar. Die Luftlast fehlt dort allerdings noch und hebt vor allem die Grundmode an.

**Risiken**
1. **Snare-Teppich:** Der verteilte Kontakt ist der teuerste und unsicherste Teil. Volles FDTD kostet nach den Pauken-Zahlen Minuten bis Stunden je s Ton auf der CPU. Die modale Näherung mit einer Saitenmode (Avanzini) ist grob. Wahrscheinlich nötig ist eine Mischform: modale Felle, wenige Drahtmoden mit Straf-Kontakt, und ein statistischer Rauschanteil, der von der Auslenkung des Resonanzfells gesteuert wird. Das gehört vor IK1 zur Absegnung.
2. **Doubletten bei n > 0:** Das Luftmodell nach Avanzini erfasst sie nicht. Das betrifft Schwebungen bei Toms und Snare.
3. **Datengrundlage für Luftlast und Dämpfung:** Die T60-Werte von Packer weichen zwischen Rechnung und Messung um bis zu Faktor 6 ab, bei (4,1) und (5,1). Die Dämpfung braucht deshalb Messung oder Fit.
4. **Rechte:**
   - CC-lizenziert sind nur Rhaouti 1999 (CC BY-NC) und Kirby & Sandler 2021 (CC BY).
   - IEEE (Avanzini & Marogna), die Durham-Thesis (Packer) und die NIU-Thesis (Zhao) sind eingeschränkt.
   - Die arXiv-Lizenzen sind noch nachzutragen.

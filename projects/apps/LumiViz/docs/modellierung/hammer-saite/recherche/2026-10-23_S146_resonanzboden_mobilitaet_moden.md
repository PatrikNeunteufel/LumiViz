# Recherche S146: Resonanzboden — gemessene Mobilität am Steg und modale Daten

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** gemessene Treibpunkt-Mobilität am Steg und modale Tabellen
> (Eigenfrequenzen, Dämpfung, modale Massen) von Flügel- und Klavierböden, mit Zahlen und Fundstelle.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [W] = wörtlich im Volltext, [A] = aus Abbildung abgelesen, [Z] = nur Zusammenfassung/Sekundärquelle.
> **Von Claude selbst nachgelesen:** Ege 2013 „about 0.06 modes Hz-1“ und „1% to 3% (mean of 2.3% …)“;
> Boutillon & Ege 2013 ⟨D_iso⟩ = 68 m⁴s⁻² (Stutzflügel); Plath 2019 Tab. 6.2 D1 13,70 kg; Corradi 2017 Tab. 2
> erste Mode 43,1 Hz. Nicht nachgelesen: Gl. 27 (charakteristische Mobilität), die Mobilitätswerte bei Plath.
> **Quellen-PDFs:** `../quellen/` (nicht versioniert); die im Bericht genannten Pfade ins Scratchpad gibt es nicht mehr.
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund Recherche Boden und Körper“.

---

## Bericht: gemessene Steg-Mobilität und modale Daten von Resonanzböden

Kennzeichnung: **[W]** = wörtlich im Volltext gesehen, **[A]** = aus Abbildung abgelesen (grob), **[Z]** = nur Zusammenfassung oder Sekundärquelle. Die Volltexte habe ich mit pdftotext gelesen, die Abbildungen gerendert. Im Repo ist nichts geändert.

### 1. Funde

**Ege, Boutillon, Rébillat, „Vibroacoustics of the piano soundboard: (Non)linearity and modal properties…“, JSV 2013 — arXiv 1212.2323**
- Gemessen wurde ein **Klavier** (upright), kein Flügel. [W] §2.3: „An upright piano (Atlas brand) … 0.91 m × 1.39 m × 8 mm“. Methode: Impulshammer auf einem Raster von 12 × 10 Punkten, Beschleunigung an 5 Punkten, dazu akustische Anregung. Hochauflösende Modalanalyse (ESPRIT) im Bereich 50 Hz–3 kHz.
- Dämpfung [W] §5.1: Verlustfaktoren bis etwa 1200 Hz „range from 1% to 3% (mean of 2.3% for the 55 lowest-frequency estimations)“. Die Abklingraten steigen „from a mean value of about 80 s⁻¹ below 1200 Hz to about 130 s⁻¹ between 1200 and 1500 Hz“. Über 1,8 kHz liegen sie wieder bei den Werten von Fichte. Modenüberlappung [W]: 30 % bei 150 Hz, 100 % bei 1 kHz, 60 % bei 3 kHz.
- Modendichte [W] §5.3: mittlerer Modenabstand „around 22 Hz for the 21 lowest modes“. Unter 1,1 kHz strebt die Dichte gegen „about 0.06 modes Hz⁻¹“. Über 1,1 kHz sind die Moden zwischen den Rippen lokalisiert.
- Erste Moden, gemessen [W] Fig. 14: 81, 114, 134, 159, 177, 205, 253, 274, 295, 303 Hz.
- Rippendaten [W] Tab. C.1/C.2: Rippenabstand 12,7–13,7 cm. Holzdaten [W] Tab. 1, z. B. „Mediocre spruce“: E_L 8,8 GPa, E_R 0,35 GPa, G_LR 0,5 GPa, ν 0,3, ρ 400 kg/m³.
- Zugang: frei (arXiv). Rohdaten gibt es keine.

**Boutillon & Ege, „…Reduced models, mobility synthesis…“, JSV 2013 — arXiv 1305.3057**
- Formeln [W] Gl. 27: Y_c,plate = n(f)/(4M) und Y_c,beam = n(f)/(4M)·(1−j). Modenabstand nach Rayleigh verteilt (Gl. 23–24), Dämpfung ζ = 2,3 %·r mit Chi-Quadrat-verteiltem r (Gl. 25).
- Das ist ein Modell. Es wird aber gegen die Impedanz verglichen, die Giordano 1998 am Steg gemessen hat. Fig. 7 [A]: am Steg bei C4 (Klavier) |Z| ≈ 0,5–3·10³ kg/s zwischen 100 Hz und 5 kHz, darüber Abfall auf ≈ 2·10² kg/s um 9 kHz. Zwischen den Rippen ≈ 10³ kg/s, ab etwa 1 kHz Abfall auf rund 3–5·10² kg/s.
- Anhang B [W]: äquivalente isotrope Steifigkeit ⟨D_iso⟩ für Klaviere 96–105 m⁴s⁻², für Stutzflügel 68 m⁴s⁻² (Berthaut) und 73 m⁴s⁻² (Suzuki).

**Ege & Boutillon, ISMA 2010 (arXiv 1210.5688) und Forum Acusticum 2011 (arXiv 1210.5109)**
- Masse [W] 1210.5109: Für „our upright piano“ ist M „almost equal to 9 kg“ (mit Rippen, Stegen und Stangen). Modale Masse m = M/4. Für die Wellenleiter über 1,1 kHz gilt „0.7 kg overall“. Verlustfaktor 2 % einheitlich, erste Mode „typical value of 70 Hz“.
- Literaturübersicht [W] 1210.5688: Es gibt „Only four measurements of the admittance (or impedance) at the bridge“: Wogram 1980, Nakamura 1983, Conklin 1996, Giordano 1998. Alle außer Conklin sind an Klavieren gemessen. Giordano unter etwa 700 Hz: am Steg nahe einer Rippe „1 to 2 · 10³ kg s⁻¹“, zwischen den Rippen „0.6 to 0.7 · 10³ kg s⁻¹“.
- **Einzige Flügelmessung: Conklin 1996.** Sie steht in Part 2 (JASA 100, 695–708), nicht in Part 1. Fig. 6 (Konzertflügel, Steg bei E2, 0–3,2 kHz) [A]: Skala −40 bis −90 dB, Mittelwert etwa −65 bis −70 dB. Die Bezugsgröße ist in der Reproduktion **nicht angegeben**. Ist sie 1 m/(N·s), entspricht das grob 3–6·10⁻⁴ s/kg; das ist meine Umrechnung, nicht belegt.
- Weiter [W] zu Conklin: Mit Rahmen und Saiten liegen die Spitzen „about 15 dB less“. In Saitenrichtung ist die Mobilität bei C6 im Diskant mit der normalen Richtung vergleichbar, zwischen 2 und 3 kHz.

**Wogram, KTH „Five lectures“ (1990), Seiten inputimp.html und modal.html**
- Klavier, Schwingerreger mit Impedanzkopf. [W]: „fourteen measuring points … spaced about 12 cm apart along the bridges“, Sweep 20 Hz–10 kHz. Weiter [W]: „around 1000 kg/s at 100 Hz and 10 kg/s at 10 000 Hz“, die Phase oben nahe −90°.
- **Widerspruch:** Fig. 5 (0 dB = 1000 kg/s) zeigt bei 10 kHz etwa −20 dB [A], also rund 100 kg/s. Ege gibt aus Wogram 1980 „160 kg s⁻¹ at 10⁴ Hz“ an und hält den Abfall für ein Messartefakt [W].
- Konzertflügel, 2,9 m lang, mit Platte und Saiten [W]: Moden bei 62, 90, 105, 127, 187, 222, 245, 325 Hz, mit Beschreibung der Modenformen. Werte für die Dämpfung fehlen.

**Conklin, KTH „Five lectures“, howdoes.html**
- Konzertflügel [W]: erste Mode bei 49 Hz, weitere bei 67, 89 und 184 Hz (8. Mode). Ohne Platte und Saiten liegt die erste Mode bei 48 Hz, montiert „around 60 Hz“. Fig. 21 zeigt die Mobilität nur in relativen dB.

**Corradi, Miccoli, Squicciarini, Fazioli, „Modal analysis of a grand piano soundboard at successive manufacturing stages“, Applied Acoustics 125 (2017)**
- Gelesen habe ich den Preprint vom 27.07.2016. **Fazioli F278**, Impulshammer, 60 Knoten. Ausbaustufe 1 frei ohne Stege, Stufe 2 frei mit Stegen, Stufe 3 in den Rahmen geleimt, ohne Saiten.
- [W]: „52 modes up to 450 Hz … 39 modes up to 386 Hz … 34 modes up to 348 Hz“. Dämpfungsgrad 0,4–1,0 % in Stufe 1–2, 0,8–4,0 % in Stufe 3.
- Tab. 2 [W], Stufe 3 (eingebaut), die ersten 20 Moden: 43,1 · 46,0 · 60,8 · 65,7 · 75,7 · 81,3 · 93,5 · 103,9 · 112,9 · 126,3 · 135,9 · 142,3 · 151,5 · 153,6 · 157,1 · 161,5 · 170,9 · 174,8 · 185,0 · 191,2 Hz.
- Fig. 4b und Fig. 10 [A] sind Transfer-Mobilitäten, **keine Treibpunkt-Mobilität**. Die Anregung liegt am Steg bei A♯4, die Antwort bei D♯3 bzw. drei Rippen weiter. Werte: etwa −60 dB re 1 m/(s·N), also ≈ 10⁻³ s/kg, im Bereich 100 Hz–4 kHz.
- Zugang: Preprint frei (17,7 MB). Daten gibt es keine.

**Plath, „From Workshop to Concert Hall…“, Dissertation Uni Hamburg 2019** (ediss.sub.uni-hamburg.de/handle/ediss/6308)
- Zwei Konzertflügel (D1, D4). Der Hersteller ist im Text **nicht ausdrücklich genannt**; Steinway D-274 ist naheliegend, aber unbelegt.
- Methode [W] §6.3: Treibpunkt-Mobilität mit Impulshammer, „15 positions … string termination points on the bass and treble bridges“. Tab. 4.2 nennt die Lagen von Taste 1 (27,5 Hz) bis 987,77 Hz mit x/y in cm.
- Mittlere Mobilität 50–1000 Hz, Fig. 6.9 [A]: blanke Platte ≈ 0,005 s/kg, eingebaut und besaitet (Stufe 5–7) ≈ 0,0007–0,0009 s/kg. Wogram und Giordano als Vergleich ≈ 0,001 s/kg.
- [W]: Die Saiten heben die Resonanzen „approx. 20 Hz“ an und senken sie um „approx. 10 dB“. Von 500 bis 5000 Hz bleibt die mittlere Mobilität konstant.
- Masse [W] Tab. 6.2: D1 im fertigen Boden **13,70 kg** (Rippen 4,17 kg, Steg 3,15 kg).
- Dämpfung [W]: T60 „approx. 0.5 s“ für Stufe 5. Gemessen ist das am Schalldruck im Nahfeld, nicht modal.
- Fig. 6.10 zeigt Kurven in dB, die Bezugsgröße habe ich nicht gefunden.
- Zugang: frei lesbar (25 MB). Lizenz habe ich nicht geprüft.

**Nur über Sekundärquellen [Z]**
- Suzuki 1986: Flügel (6 ft), Moden 49,5 / 76,5 / 85,3 / 116,1 Hz, Modenabstand etwa 22–25 Hz. Die Angaben zum Instrument widersprechen sich zwischen den Quellen.
- Kindel & Wang 1987: zwei Konzertflügel, Moden bis 130 Hz. Zahlen habe ich keine gefunden.

### 2. Zugang und Daten
- Frei lesbar sind die arXiv-Arbeiten, die KTH-Seiten, der Preprint von Corradi und die Dissertation von Plath.
- **Herunterladbare Mobilitätsdaten habe ich nicht gefunden.**
- Am nächsten kommt Zenodo 3271370 (Plath 2019, CC BY 4.0, 4,6 GB): Kraft am Tastenbett, Beschleunigung am Steg und Kunstkopf. Das ist keine Mobilität. Nicht heruntergeladen.

### 3. Gesucht, nicht gefunden
- Volltexte von Giordano 1998 (JASA 103, 2128), Conklin 1996 Part 1 und 2, Wogram 1980/84, Nakamura 1983, Suzuki 1986, Kindel & Wang, Berthaut 2003, Mamou-Mani und Moore & Zietlow.
- Eine Phase der Steg-Mobilität am **Flügel**. Nur Wogram zeigt eine Phase, und zwar am Klavier.
- Tabellen modaler Massen. Gefunden habe ich nur das Modell m = M/4.
- Tabellen der Dämpfung je Mode an einem fertig besaiteten Flügel.
- Messungen an Yamaha.
- Die Bezugsgröße der dB-Skalen bei Conklin und bei Plath Fig. 6.10.
- Der Southampton-Postprint (eprints.soton.ac.uk/408405) und die Polimi-Seiten antworteten mit 403.

Quellen: [arXiv 1212.2323](https://arxiv.org/abs/1212.2323), [arXiv 1305.3057](https://arxiv.org/abs/1305.3057), [arXiv 1210.5688](https://arxiv.org/abs/1210.5688), [arXiv 1210.5109](https://arxiv.org/abs/1210.5109), [KTH Wogram](https://www.speech.kth.se/music/5_lectures/wogram/inputimp.html), [KTH Wogram modal](https://www.speech.kth.se/music/5_lectures/wogram/modal.html), [KTH Conklin](https://www.speech.kth.se/music/5_lectures/conklin/howdoes.html), [Corradi Preprint](https://miccoli.faculty.polimi.it/preprints/soundboard-20160727.pdf), [Plath Diss.](https://ediss.sub.uni-hamburg.de/handle/ediss/6308), [Zenodo 3271370](https://zenodo.org/record/3271370).

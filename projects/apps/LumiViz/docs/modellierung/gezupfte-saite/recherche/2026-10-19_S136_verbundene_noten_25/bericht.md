# Recherche Stufe 25 — verbundene Noten auf einer Saite (Hammer-on, Pull-off, Slide, Bend)

Stand 19.10.2026. Kennzeichnung je Angabe: **[gelesen]** = an der Quelle selbst gelesen (Seite,
kurzes Zitat), **[gemessen]** = eigene Messung an Daten (Methode unten, Rohzahlen in den
Dateien dieses Ordners), **[abgeleitet]** = eigene Rechnung (Rechenweg steht dabei).
Zusammenfassungen von Suchwerkzeugen sind nirgends als Beleg verwendet; wo nur eine
Sekundärquelle gelesen wurde, steht das dabei.

## Quellen (Kürzel)

| Kürzel | Quelle | gelesen als |
|---|---|---|
| Pak05 | J. Pakarinen, M. Karjalainen, V. Välimäki, S. Bilbao, „Energy behavior in time-varying fractional delay filters for physical modeling synthesis of musical instruments“, Proc. ICASSP 2005, Bd. 3, S. III-1–III-4. https://aaltodoc.aalto.fi/server/api/core/bitstreams/c4d811b1-05e4-4949-91dc-3c06ee3b9e9e/content | PDF, Seiten als Bild gelesen (Formeln) |
| PPV08 | J. Pakarinen, T. Puputti, V. Välimäki, „Virtual Slide Guitar“, Computer Music Journal 32(3), 2008, S. 42–54 (Nachdruck in Pakarinens Diss.) https://aaltodoc.aalto.fi/server/api/core/bitstreams/b890243c-fd3e-435c-a755-b98bd860507a/content | PDF-Text |
| PVP08 | J. Pakarinen, V. Välimäki, T. Puputti, „Slide guitar synthesizer with gestural control“, NIME 2008, S. 49–52. https://nime.org/proceedings/2008/nime2008_049.pdf | PDF-Text |
| PPB07 | J. Pakarinen, H. Penttinen, B. Bank, „Analysis of handling noises on wound strings“, JASA 122(6), 2007, EL197–EL202 (Nachdruck, Seitenzählung des Nachdrucks 1–4). https://aaltodoc.aalto.fi/server/api/core/bitstreams/475276ab-f408-4ef4-a46f-5528bb8ca63d/content | PDF-Text |
| VPEK06 | V. Välimäki, J. Pakarinen, C. Erkut, M. Karjalainen, „Discrete-time modelling of musical instruments“, Rep. Prog. Phys. 69, 2006, S. 1–78. https://aaltodoc.aalto.fi/server/api/core/bitstreams/3e583e58-7367-48b0-a571-fc530b2a3d20/content | PDF-Text, Abschn. 11.3 |
| Val95 | V. Välimäki, *Discrete-Time Modeling of Acoustic Tubes Using Fractional Delay Filters*, Diss. TKK 1995, Abschn. 3.5, S. 114–117. http://users.spa.aalto.fi/vpv/publications/vesan_vaitos/ch3_pt4_transients.pdf | PDF-Text |
| Laur01 | M. Laurson, C. Erkut, V. Välimäki, M. Kuuskankare, „Methods for Modeling Realistic Playing in Acoustic Guitar Synthesis“, CMJ 25(3), 2001, S. 38–49 (Nachdruck in Erkuts Diss.). https://aaltodoc.aalto.fi/server/api/core/bitstreams/768dbe9c-526c-414a-b2ce-0742574b8064/content | S. 47 als Bild gelesen |
| Oez10 | T. H. Özaslan, J. L. Arcos, „Legato and glissando identification in classical guitar“, SMC 2010, S. 457–462. https://iiia.csic.es/media/filer_public/83/30/83307d66-b287-4dd2-a579-3a0249ade134/3846.pdf | PDF-Text |
| Grimes14 | D. R. Grimes, „String Theory – The Physics of String-Bending and Other Electric Guitar Techniques“, PLoS ONE 9(7), 2014, doi:10.1371/journal.pone.0102088 (Volltext-XML über Europe PMC; Formeln dort als MathML, beim Lesen verloren) | Text ohne Formeln |
| Su19 | T.-W. Su, Y.-P. Chen, L. Su, Y.-H. Yang, „TENT: Technique-Embedded Note Tracking for Real-World Guitar Solo Recordings“, TISMIR 2(1), 2019. https://transactions.ismir.net/articles/23 (XML-Fassung) | Text |
| IDMT-G | A. Eppler, C. Kehling, A. Männchen, *IDMT-SMT-GUITAR Dataset Description* (20.01.2016) und Datensatz V2, Zenodo 10.5281/zenodo.7544110 (CC BY-NC-ND 4.0) | PDF-Text; Daten per Teilabruf |
| Mit23 | A. Mitsou u. a., „A multimodal dataset for electric guitar playing technique recognition“, Data in Brief 52 (2023) 109842; Datensatz Zenodo 10.5281/zenodo.10075352 (Lizenz laut Zenodo CC BY 4.0) | Artikel (PMC10698518) per Abrufwerkzeug mit wörtlichen Zitaten; Daten per Teilabruf |
| Lynn | B. Lynn, „The Karplus-Strong Algorithm“, https://theory.stanford.edu/~blynn/sound/karplusstrong.html — **Sekundärquelle** zu Jaffe/Smith 1983 | HTML |

Nicht gelesen (kein freier Zugang gefunden oder Zeit): Jaffe/Smith 1983 im Original,
Karjalainen/Välimäki/Tolonen 1998 (CMJ 22(3)), Cuzzucoli/Lombardo 1999, Laakso u. a. 1996
„Splitting the unit delay“, Bilbao/Torin 2015 (JAES 63(5), nur Mitglieder), Evangelista/Eckerholm
2010 (IEEE TASLP 18(4)). Was diese sagen, steht hier **nicht**.

## Daten und eigene Messungen

**IDMT-SMT-Guitar dataset2** [gelesen, IDMT-G S. 5 und 7]: Ausdrucksstile „normal (NO), bending
(BE), slide (SL), vibrato (VI), natural harmonics (HA) and dead notes (DN)“ (S. 5) — **kein
Hammer-on, kein Pull-off** (Suche nach „hammer“, „pull“, „legato“ in der Beschreibung: 0 Treffer;
in Lick-Dateinamen steht „H“ für harmonics, Abb. 1.4). Aufnahme: Gitarren „mostly plugged
directly into the Line-In“ (S. 7), also DI ohne Verstärker, 44,1 kHz, dataset2 24 bit.
Teildateien: je Gitarre (Aristides 010 = AR, Fender Stratocaster = FS, Gibson Les Paul = LP)
`<g>_G_V_bending` (3 Bends, G-Saite Bund 5, C4), `<g>_G_V_slide` (7 Slides, Annotation
`modulationFrequencyRange` 100…700 Cent), `<g>_V_vibrato_slow-fast` (12 Ereignisse),
Licks mit B/S/V im Namen (Lick1–6) und `<g>_G_fret_0-20` als Normalton-Bezug. Abruf mit
`fernzip.py` (HTTP-Range, ohne das 1,3-GB-Archiv zu laden).

**Mitsou 2023** [gelesen über Abrufwerkzeug, Zitate]: neun Techniken, darunter „hammer-on,
pull-off, slide, and bend“; Aufnahme mit Smartphone (Xiaomi Redmi Note 8) im Raum,
Verstärkersimulationen „SL100, Plexiglass, and DC-Modern“ (Avid Eleven Rack, Soldano/Vox/
Marshall). Gain nicht angegeben. Übungsblatt Hammer-On_Exercise_1 [gelesen, als Bild]:
Achtelpaare auf einer Saite, angeschlagen Bund 1, gebunden Bund 2/3/4, „Continue up to the 9th
fret and repeat for every string“.

Skripte (alle in diesem Ordner, mit `python -I`):
- `codes_idmt.py`, `liste_ds2.py`, `zeige_anno.py`, `fernzip.py` — Auswahl und Abruf.
- `messe_bend_slide.py` — Grundton YIN (1024/220 Werte, 23 ms/5 ms, Schwelle 0,15, Suchband
  0,6…1,7 × Sollton), Pegel RMS 10 ms; Definitionen im Kopf des Skripts. Rohausgabe
  `messung_bend_slide.txt`, je Ereignis `bend_slide_roh.tsv`, Kurven `bend_slide_kurven.tsv`.
- `fasse_bend_slide.py` → `zusammenfassung_bend_slide.txt`.
- `messe_legato_magcil.py` → `messung_legato_magcil.txt` (drei WAVs: Hammer Carvin/Plexiglass,
  Hammer RWG/SL100, Pull-off Carvin/Plexiglass).

---

## 1. Hammer-on und Pull-off

### Belege

- **Synthese, Laurson/Erkut** [gelesen, Laur01 S. 47]: Slurs werden mit einem kleinen Anschlag
  gemacht: „sending a pluck event with a small amplitude value“; dabei „the gain of the loop
  filter is not modified“. Portamento = „extremely fast chromatic left-hand slur passage“.
  Eine Zahl für „small“ steht dort nicht.
- **Karplus-Strong, Jaffe/Smith** [gelesen nur sekundär, Lynn]: „Slurs can be simulated by using
  a new value of N on the fly.“ Original nicht gelesen.
- **Messung an Legato-Einsätzen** [gelesen, Oez10 S. 458]: Legato-Einsätze „are not as powerful
  in terms of energy“ und unterscheiden sich in der Harmonizität vom Anschlag. Keine dB-Zahl.
- **Energie beim Verkürzen im Modell** [gelesen, PPV08 S. 46]: Wird die Waveguide-Saite plötzlich
  halbiert, werden „half of the signal samples … discarded“ und rund 50 % der Energie gehen
  verloren. PPV08 nennt das für den **Slide** ein Artefakt; zur echten Saite: „might be
  controversial“ (S. 46). VPEK06 S. 64: ob ein Modell bei Parameteränderung wie das Instrument
  reagiert, sei durch Messung „extremely difficult, if not impossible“ zu klären.
- **Datensätze** [gelesen]: IDMT-G hat kein Hammer-on/Pull-off (s. o.). Su19 hat 70 Hammer-ons
  und 143 Pull-offs in 42 Soli („clean tone or distortion“), Audio nicht frei (Repo SoloLa ohne
  Audio, geprüft). GOAT (DI, Tabulatur mit Legato) nur auf Anfrage (arXiv 2509.22655, per
  Abrufwerkzeug: „available by request“).

### Eigene Messung [gemessen] — Mitsou 2023, Hammer/Pull-Übungen

Methode: Grundton YIN (5 ms), gebundener Wechsel = stabiler Tonhöhenwechsel 1…3 Halbtöne in
Bindungsrichtung; „Sprung“ = Max-RMS 0…40 ms nach dem Wechsel minus Mittel 10…0 ms davor;
„gegen Anschlag“ = Spitze des gebundenen Tons minus Spitze des vorigen angeschlagenen Tons.

| | n | Sprung über den abgeklungenen Pegel | Spitze gebunden gegen Spitze Anschlag | Abklingen davor |
|---|---|---|---|---|
| Hammer-on (2 Aufnahmen) | 123 | Median **+3,3 dB** (P10 +0,6, P90 +7,2) | Median **+0,2 dB** (P10 −4,0, P90 +4,3) | −14,8 dB/s |
| Pull-off (1 Aufnahme) | 64 | Median **+2,7 dB** (P10 +0,9, P90 +6,9) | Median **−0,1 dB** (P10 −5,5, P90 +3,4) | −11,1 dB/s |

**Gültigkeit:** Smartphone im Raum, Verstärkersimulation mit unbekanntem Gain (Plexiglass/SL100
sind Hochgain-Vorbilder). Ein Begrenzer drückt Pegelunterschiede zusammen. [abgeleitet] Unter
einer monotonen, verdichtenden Kennlinie ist ein gemessener positiver Sprung eine
**Untergrenze** des sauberen Sprungs; der Vergleich „gegen Anschlag“ sagt über den sauberen
Wert nur, dass seine Streuung mindestens so groß ist. Belastbar ist: **beide Techniken regen die
Saite messbar neu an** (P10 > 0 dB) — reine Verkürzung ohne Anregung ergäbe ≈ 0 dB (s. u.).

### Physik [abgeleitet]

- **Hammer-on, verbleibende Schwingung:** Im Augenblick des Aufschlags bleibt der Zustand des
  Abschnitts Steg–neuer Bund erhalten; der Abschnitt dahinter wird abgekoppelt. Bei gleich
  verteilter Energiedichte bleibt der Anteil L₂/L₁ = 2^(−n/12): 1 HT −0,25 dB, 2 HT −0,5 dB,
  5 HT −1,25 dB **Energie**, die **Amplitude** am Abnehmer springt nicht. Genau das macht eine
  ungekorrigierte Umlaufverkürzung (Abtastwerte bleiben, die Schleife wird kürzer) — für den
  plötzlichen Wechsel ist sie physikalisch richtig; eine Energiekorrektur nach Pak05 gehört
  **nicht** dazu (die gilt für langsames Gleiten, s. Frage 2).
- **Hammer-on, Zusatz-Anregung:** Der Finger trifft die Saite unmittelbar an der neuen
  Begrenzung (am Bund). Eine Anregung im Abstand β·L vom Ende regt Teilton k mit ∝ sin(kπβ)
  an; für kleines β ≈ kπβ: schwach und hochbetont (Klick-artig). Das passt zum gelesenen
  Befund „weniger Energie, andere Harmonizität“ (Oez10) und zum gemessenen positiven Sprung.
- **Pull-off:** Der abgehende Finger zupft seitlich an der Stelle des alten Bunds; vom neuen Ende
  (unterer Finger) aus liegt diese bei β = 1 − 2^(−n/12): 1 HT 0,056, 2 HT 0,109, 3 HT 0,159
  → Kerbe bei Teilton 1/β ≈ 18, 9, 6. Der neu hinzukommende Abschnitt war zwischen den Fingern
  eingeklemmt, also nahezu in Ruhe; im Modell wird er nicht aus alten Werten gefüllt, sondern
  aus der Zupfform.
- **Spektrum des alten Anteils:** Der Zupfpunkt liegt fest zum Steg (Abstand d), relativ zur
  neuen Länge also bei d/L₂ statt d/L₁ — die Kerben der Zupfstelle rücken zu niedrigeren
  Teiltönen (Faktor L₂/L₁).

### Folgerung für Modell und Vorgaben

- Modell: **ein Ton**; beim Wechsel Umlauf sprunghaft auf die neue Länge, Zustand behalten,
  **keine** Energiekorrektur; dazu eine kleine Zusatz-Anregung — Hammer: Impuls an der neuen
  Begrenzung (β ≈ Fingerabstand zum Bund / L), Pull-off: Zupfform bei β = 1 − 2^(−n/12).
  Verlustfilter der Zieltonhöhe (wie beim angeschlagenen Ton an diesem Bund; Laur01 lässt den
  Schleifengewinn unverändert).
- Vorgabe Pegel: **offen**. Belegt ist nur: Sprung ≥ ca. +3 dB über den abgeklungenen Pegel
  (Untergrenze aus komprimierter Messung), Spitze ungefähr in Höhe des vorigen Anschlags mit
  ±4 dB Streuung. Vorschlag: Zusatz-Anregung als Parameter, Startwert so, dass der Sprung im
  Mittel +3 dB ergibt; endgültig erst nach einer sauberen DI-Messung (s. Lücken).

---

## 2. Zeitvariable Verzögerung (Länge während des Klingens ändern)

### Belege [gelesen]

- **Problem** (Pak05 S. III-1): Lagrange und Allpass seien „somewhat problematic … when the
  length of the string is varied due to glissando or vibrato“ — Energie und Aliasing.
- **Allpass vs. FIR** (Val95 S. 114–117): FIR-Interpolatoren haben beim Umschalten der
  Koeffizienten **keine Transienten**, wohl aber eine Unstetigkeit im Ausgang, die bei kleinen
  Schritten unhörbar ist; Koeffizienten möglichst je Abtastwert nachführen („verify that
  updating is frequent enough“, S. 116). Rekursive Filter (Allpass) erzeugen Transienten, „clicks
  in audio applications“ (S. 117); fünf Gegenmittel (Überblenden, Koeffizienten-Interpolation …).
  Bei Lagrange in Waveguides lieber die Ordnung senken als außerhalb des optimalen Bereichs
  arbeiten — außerhalb ist Lagrange „not a passive filter“ (S. 116).
- **Energie, Formel** (Pak05 S. III-2, Gl. 4–6): Springt der Lesepunkt um Δx (Δx = 1 heißt
  Verlängerung um einen Abtastwert), korrigiert die „zeroth order“-Variante den Ausgang mit
  p_c = √(1 − Δx)·p ≈ (1 − Δx/2)·p. Gl. 9 ist eine Korrektur erster Ordnung (zwei
  Interpolationen je Takt).
- **Energie, Ergebnis** (Pak05 S. III-3, Abb. 4): Test 128 → 64 → 128 Abgriffe in rund 300 ms,
  |Δx| = 0,01. Ungekorrigiert sinkt die Energie mit der Länge (Amplitude bleibt), korrigiert bleibt
  die Energie konstant, „amplitude … increases 3 dB when the delay is halved“. Die Korrektur
  erster Ordnung „does not improve the model behavior noticeably“. Empfehlung: „A fifth-order
  Lagrange interpolator with zeroth-order energy correction“.
- **Aliasing** (Pak05 S. III-3): im Umlauf skalieren Frequenzen mit f′ = (l₁/l₂)·f; hohe Teiltöne
  klingen schnell ab, Überabtastung hilft.
- **WDF-Saite** (Pak05 S. III-4): energieerhaltende zeitvariable Allpässe (Wellendigitalfilter);
  dort entspricht die Änderung „varying the string tension, rather than the string length“.
- **Praxis** (PPV08 S. 46): Lagrange 5. Ordnung, betrieben im Verzögerungsbereich 2,0–3,0
  Abtastwerte (zwei Werte im ganzzahligen Teil ausgeglichen); Grenzfrequenz der
  Nullte-Ordnung-Korrektur f_lim = 2L·f₀/|Δx| (Gl. 4) — für eine Oktave in 0,5 s auf der tiefen
  Saite |Δx| = 0,0061, f_lim ≈ 16 kHz, „accurate enough“.

### Physik [abgeleitet] — welche Korrektur ist richtig?

Für langsame Änderung (relativer Längenschritt je Umlauf ≪ 1) ist je Mode E/ω eine adiabatische
Invariante. Mit E = ¼·μ·L·ω²·A² (Mode mit Auslenkungsamplitude A):

| Änderung | Auslenkung A | Schnelle/Kraft ∝ ωA | Energie | Gewinn je Umlauf (Verzögerung D → D(1+Δx)) |
|---|---|---|---|---|
| Länge (Slide), ω ∝ 1/L | konstant | ∝ f (+6 dB/Oktave) | ∝ f | Auslenkungswellen g = 1; Schnellewellen g = 1/(1+Δx) ≈ 1 − Δx |
| Spannung (Bend), L fest, ω ∝ √T | ∝ f^(−½) | ∝ f^(½) (+3 dB/Oktave) | ∝ f | Auslenkung g = √(1+Δx); Schnelle g = 1/√(1+Δx) ≈ 1 − Δx/2 |

Rechenweg: Pro Umlauf wird jeder umlaufende Wert einmal mit g multipliziert; in dieser Zeit ändert
sich D um D·Δx. Soll die Amplitude ∝ D^p gehen, folgt g = (1+Δx)^p. Pakarinens √(1 − Δx) (Energie
konstant) entspricht damit bis zur ersten Ordnung dem **Bend mit Schnellewellen**, beim Slide
liegt sie zwischen den beiden physikalischen Fällen. Der plötzliche Wechsel (Hammer-on) ist nicht
adiabatisch: dort g = 1 (Zustand bleibt, Frage 1).

### Folgerung

- Interpolation: Lagrange (FIR) 5. Ordnung, Bereich 2–3 Abtastwerte, Koeffizienten je Abtastwert;
  Allpass nur mit Transienten-Gegenmittel. Für Bend alternativ WDF-Allpass-Kette (Pak05).
- Energie: **nach der umlaufenden Größe** wählen (im LumiViz-Code nachsehen, ob der Umlauf
  Auslenkung, Schnelle oder Kraft trägt) und nach Art der Änderung (Länge/Spannung,
  langsam/plötzlich). Die Messung zum Slide (Frage 3) zeigt allerdings keinen Pegelgewinn,
  sondern Mehrverlust — beim **Bundslide** ist der Verlauf stufig (Frage 3), jede Stufe ist eher
  „plötzlich“; dort also g = 1 plus gemessener Mehrverlust.

---

## 3. Slide

### Eigene Messung [gemessen] — IDMT, G-Saite ab Bund 5, je Gitarre 7 Slides (DI)

| Weg (HT) | Gleitdauer AR / FS / LP (ms) |
|---|---|
| 1 | 10 / 5 / 5 (praktisch ein Sprung) |
| 2 | 60 / 55 / 35 |
| 3 | 100 / 95 / 60 |
| 4 | 140 / 120 / 100 |
| 5 | 185 / 165 / 175 |
| 6 | 200 / 180 / 195 |
| 7 | 200 / 224 / 224 |

- Gerade ab 2 HT (n = 18): **Gleitdauer ≈ −15 ms + 35 ms je Halbton**; Geschwindigkeit
  2700–3500 Cent/s, unabhängig vom Weg. Gleitbeginn 80–150 ms nach dem Anschlag.
- Verlauf: Gerade in Cent erklärt R² = 0,97–0,99 (≥ 4 HT), in Hz minimal weniger — **linear in
  Bünden = exponentiell in Hz**. Zugleich **stufig**: 66 % der Rahmen (54–85 %) liegen ±20 Cent
  um einen ganzen Halbton, gleichverteilt wären 40 % — die Bünde sind als Stufen hörbar.
- Pegel: nach minus vor dem Gleiten −1…−9 dB; bereinigt um das Abklingen des Normaltons
  (−9…−13 dB/s zwischen 0,1 und 0,6 s): **Mehrverlust Median −2 dB (AR, FS), −4 dB (LP)**,
  Spanne +0,5…−6,3 dB, **ohne Gang mit der Weglänge**. Kein Pegelgewinn.
- In Licks (66 SL-Ereignisse, gespielt im Zusammenhang): 1 HT Median 10 ms, 2 HT 40 ms, 3 HT 55 ms
  — schneller als die Einzelaufnahmen (~20 ms/HT); Stufigkeit 0,45 (kurze Wege, grob aufgelöst).

### Belege [gelesen]

- **Kontaktgeräusch** (PPB07, Nachdruck S. 1–2): Frequenz der Teiltöne und RMS-Pegel des Geräuschs
  „linearly proportional to the sliding speed“; f_Hn = n·v_s·d_w (d_w Windungen je Meter);
  dazu statische Resonanzen der Längsschwingung; die Bewegung beschleunigt und bremst („first
  accelerates and then decelerates“). Gemessen mit Fingerkuppe/-nagel auf umsponnenen Saiten.
- **Bottleneck-Slide** (PPV08 S. 43–45): schnelle Slides lauter und heller als langsame; blanke
  Saiten „very much quieter“, ohne harmonische Struktur, ähnlich weißem Rauschen; beim Aufsetzen
  ein kleiner Klick; statische Resonanzen ab ≈ 1500 Hz (Längsmoden). Synthese: Pulsfolge mit
  Feuerrate f_c = |dL/dt|·n_w, Resonator, tanh-Formung, IIR 4. Ordnung, Pegel g_TV = f_c/100
  (S. 47–48), Tabelle 1 mit Polen/Nullstellen je Röhrchen und Saite (S. 49).
- **Energie** (PPV08 S. 46, PVP08 S. 50): ohne Korrektur „unnaturally quick decay“, Korrektur
  nach Pak05.
- Su19 (Heuristik, keine Messung): Slides > 3,5 HT als „long slide“; Wechsel unter 0,5 HT gelten
  nicht als neue Note.

### Folgerung

- Vorgabe Gleitdauer **Bundslide: 35 ms je Halbton** (Einzelaufnahme, ruhiges Tempo), im Spiel
  eher 20 ms/HT; Slide über 1 HT ≈ Sprung (5–10 ms). Verlauf **linear in Cent**, mit
  Bundstufen (Variante: Stufen je Bund mit kurzen Übergängen — Stufigkeit 0,66 als Zielmaß).
- Pegel: kein Pegelgewinn; **Mehrverlust −2 dB** (Spanne 0…−6) je Slide, nicht je Halbton.
- Gleitgeräusch: nur bei umsponnenen Saiten mit harmonischer Struktur; Grundfrequenz
  v·d_w. [abgeleitet] Handgeschwindigkeit aus Bundabständen: Abstand Bund m → m+n = L·(2^(−m/12)
  − 2^(−(m+n)/12)); Beispiel L = 648 mm (Beispielwert, nicht gemessen), 5 → 10 in 180 ms:
  122 mm → 0,68 m/s. d_w (Windungsdichte) ist **nicht** belegt — bei Saitenherstellern
  nachschlagen oder messen. Die IDMT-G-Saite ist bei E-Gitarren-Sätzen meist blank (nicht
  belegt); das Geräusch wurde hier nicht gemessen.

---

## 4. Bend

### Eigene Messung [gemessen] — IDMT, G-Saite Bund 5

Einzelaufnahmen (didaktisch, langsam, je Gitarre drei Bends):

| | Hub (Cent) | Halt (Cent) | Anstieg 10–90 % | 25/50/75-%-Lage | Mehrverlust | Halte-Abklingen |
|---|---|---|---|---|---|---|
| AR | 89 / 194 / 286 | 89 / 186 / 282 | 549 / 723 / 733 ms | 0,09–0,16 / 0,27–0,34 / 0,57–0,73 | −0,7 / −0,3 / −0,4 dB | −4…−5 dB/s |
| FS | 98 / 203 / 269 | 98 / 188 / 266 | 309 / 509 / 594 ms | 0,14–0,21 / 0,38–0,45 / 0,71–0,92 | +0,4 / +0,6 / +0,3 dB | −5…−6 dB/s |
| LP | 109 / 192 / 282 | 109 / 192 / 279 | 215 / 364 / 733 ms | 0,18–0,23 / 0,45–0,48 / 0,72–0,81 | −0,8 / −2,0 / 0,0 dB | −5…−9 dB/s |

Licks (gespielt im Zusammenhang, BE-Ereignisse):

| Hub-Klasse | n | Hub Median (Spanne) | Anstieg 10–90 % | Beginn (10 %) nach Onset | 50-%-Lage | Überschwinger |
|---|---|---|---|---|---|---|
| ~1 HT | 24 | 107 c (63–149) | **82 ms** (45–160) | 186 ms (32–271) | 0,52 | 7 c (1–63) |
| ~2 HT | 15 | 170 c (157–232) | **95 ms** (70–135) | 206 ms (71–286) | 0,47 | 5 c (2–19) |

- Form: Median 25/50/75 % bei 0,20 / 0,47 / 0,78 des Anstiegs (linear wäre 0,19 / 0,50 / 0,81)
  → **nahezu linear in Cent**, leicht vorn betont; in den langsamen Einzelaufnahmen deutlicher
  vorn (50 % bei 0,27–0,48 → schnell los, langsam ins Ziel).
- Ziel: Halteabweichung (Standardabw. über 300 ms) 1,5–9,8 Cent. Gegen den nächsten ganzen
  Halbton lagen die Haltewerte der Einzelaufnahmen −34…+9 Cent daneben (eher darunter, größer
  bei 3 HT). Die Ganzton-Klasse der Licks hat Median 170 Cent. **Die gewollte Zielhöhe ist nicht
  annotiert** (`modulationFrequencyRange` = 0) — ob „zu kurz“ Absicht oder Ungenauigkeit ist,
  bleibt offen.
- Startwert: bereits vor dem Bend +4…+33 Cent über dem Sollton (Greifdruck; IDMT-Stimmung).
- Pegel: **kein messbarer Mehrverlust** (−2,0…+0,6 dB, Median −0,3 dB gegen das Abklingen des
  Normaltons), Abklingen im Halten −4…−9 dB/s.

### Belege und Physik

- [gelesen, Grimes14] Grundfrequenz der gespannten Saite aus der Wellengleichung (Gl. 1, Mersenne);
  Bend dehnt die Saite (Hooke), Gl. 6 gibt f in Abhängigkeit vom Biegewinkel — experimentell an
  mehreren Saiten bestätigt; Bend „up“ oder „down“ hebt immer („the net effect is always to raise
  pitch“); Vibrato senkt nie unter die Ruhetonhöhe. Formeln selbst nicht lesbar (MathML verloren).
- [abgeleitet] f ∝ √T ⇒ T-Verhältnis 2^(n/6): 1 HT +12 %, 2 HT +26 %, 3 HT +41 %.
  Inharmonizität B ∝ 1/T (B = π³Ed⁴/(64TL²)) ⇒ −21 % beim Ganzton-Bend — bei blanken Saiten
  wenige Zehntel Cent am 6. Teilton, nicht messbar mit diesen Daten. Adiabatisch (Tabelle in
  Frage 2): Schnelle-Amplitude +3 dB/Oktave → +0,5 dB beim Ganzton — passt zum Messbefund
  „kein messbarer Unterschied“.
- [gelesen, Su19] Heuristik „long Bend“: Mindestdauer 0,2 s „according to domain knowledge“ —
  keine Messung.

### Folgerung

- Vorgabe Anstieg **im Spiel 80–100 ms** (10–90 %), nicht deutlich vom Hub abhängig; als langsamer
  „Ausdrucks-Bend“ 200–700 ms. Beginn häufig ~0,2 s nach dem Anschlag (Lick-Median).
- Kurve: **linear in Cent** als Grundform (optional leicht vorn betont); Überschwinger klein
  (Median 5–7 Cent).
- Pegel/Abklingen: wie der normale Ton; im Modell Spannungsänderung (Pak05-WDF oder
  Verzögerungsänderung mit g nach Tabelle Frage 2 — der Unterschied liegt unter 1 dB).
- Zielgenauigkeit als Streuung ±10 Cent (Halten); systematisches Zu-kurz-Kommen nicht als Vorgabe
  übernehmen, solange die Absicht nicht belegt ist.

---

## 5. Vibrato (kurz)

- [gemessen, IDMT `V_vibrato_slow-fast`, 36 Ereignisse, A2…A4] Je Saite zwei Ereignisse; das
  erste ist das schnellere: Rate Median **4,7 Hz** (3,6–5,9), das zweite **2,2 Hz** (1,5–4,4);
  Tiefe (halbe Spanne 5.–95. Perzentil) 7–24 Cent, Median ≈ 13 Cent; Mittellage +3…+43 Cent über
  dem Sollton (Vibrato hebt nur).
- [gelesen, Laur01 S. 47] Rate „typically around 5–6 Hz“, Tiefe über eine Hüllkurve mit zwei
  „humps“; mehr Tiefe an höheren Bünden.

---

## Reichweite der Suche

- Datensätze: IDMT-SMT-Guitar V2 (Beschreibung ganz, dataset2 alle Annotationen, Audio der
  Technik-Dateien), Mitsou 2023 (Dateiliste, README, zwei Übungsblätter, drei von 110
  Hammer/Pull-WAVs), Su19-Repo (ohne Audio), GOAT (nur Beschreibung). Nicht geprüft:
  GuitarSet, EGDB, IDMT-SMT-Bass auf Slide/Bend.
- Literatur: Websuche (Standard/erweitert) zu Pak05, PPV08, Hammer-on-Physik, Legato-Analyse,
  Jaffe/Smith; Aaltodoc-Volltexte der Dissertationen Pakarinen 2008 und Erkut 2002 (alle
  Einzelartikel geladen; durchsucht nach slur, legato, hammer, pull-off, glissando, portamento,
  time-varying); Four Decades of Digital Waveguides (JAES 2026) durchsucht — nichts Neues.
  Für Hammer-on/Pull-off fand sich **keine physikalische Messung an sauberem Signal**; kein
  Treffer ist kein Beweis.

## Offene Lücken

1. **Pegel und Spektrum des Legato-Tons gegen einen gezupften Ton gleicher Höhe, sauber (DI)** —
   die einzige gemessene Quelle ist verzerrt/komprimiert. Weg: eigene DI-Aufnahme (je Saite
   Bund 5: angeschlagen; Bund 3 angeschlagen + Hammer auf 5; Bund 7 angeschlagen + Pull-off auf 5;
   je 5 Wiederholungen) — Messskript lässt sich aus `messe_legato_magcil.py` ableiten; oder GOAT
   anfragen.
2. Zahl für „small amplitude“ bei Laur01 und Wortlaut von Jaffe/Smith 1983 (Original nicht gelesen).
3. Welche Größe im LumiViz-Umlauf läuft (Auslenkung/Schnelle/Kraft) — entscheidet die Korrektur g.
4. Windungsdichte d_w der mitgelieferten Saiten für das Gleitgeräusch; Gleitgeräusch eines
   Bundslides auf umsponnener Saite nicht gemessen (IDMT-Slides nur G-Saite).
5. Gewollte Zielhöhe der IDMT-Bends unbekannt → Zielgenauigkeit nur als Streuung belegt.
6. Slides nur auf einer Saite (G), ab Bund 5, aufwärts; abwärts nur in Licks (wenige, kurz).
7. Inharmonizität und Teiltonabklingen während des Bends nicht gemessen (rechnerisch < 1 Cent).
8. Bilbao/Torin 2015, Evangelista/Eckerholm 2010, Cuzzucoli/Lombardo 1999, KVT98 nicht gelesen.

# Recherche S195: Klassische Gitarre (Nylon, 650 mm) — Quellen mit Zahlen für Saite, Zupf, Steg

> **Auftrag:** Quellen mit Zahlen für ein Modell je Saite der klassischen Gitarre (gegriffen am Bund, zwei
> Polarisationen, Steg): Saitensatz mit Zug, Saitenkennwerte und Dämpfung, Zupf, Stegadmittanz und Körper, Bund,
> Ziele (Ausklang). Ergänzt, wiederholt nicht: `ic/recherche/2026-10-26_korpus_deluxe.md`, `ic/quellen.md`,
> `recherche/2026-10-18_S132_korpus_tonabnehmer_messwerte.md`; Wright 1996, Mores 2021 und Iowa MIS liegen lokal.
> **Datum:** 2026-11-12 (Sitzung S195).
> **Bericht eines Agenten (Claude), unverändert.** Nichts ins Projekt geladen, kein Code geändert.
>
> **Reichweite:** 19 Websuchen, 17 Abrufe mit WebFetch, 13 Abfragen mit curl an APIs (GitHub, Zenodo, HAL,
> arXiv, Crossref, DSpace von McGill) — zusammen rund 49 Zugriffe, Fehlschläge mitgezählt. Sprache der Suche Englisch.
> **Gesperrt / nicht erreicht:** core.ac.uk (403, zwei PDFs), hal.science (Anubis-Botsperre), daddario.com
> (HTTP 429 zweimal), Savarez (Zugtabelle nur als Bild), bohrium.dp.tech (404). Die HAL-API lief.
>
> **Wie gelesen:** WebFetch kann komprimierte PDFs nicht lesen, legt sie aber im Cache dieser Sitzung ab
> (`C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\240c30b3-…\tool-results\`,
> **nicht im Repo**). Diese Dateien habe ich mit `pdftotext -layout` in den Scratchpad gelesen. Zwei Dissertationen
> von McGill habe ich über die Textfassung des Repositoriums (DSpace-Bündel `TEXT`) per curl gelesen — diese ist
> **nach rund 100 kB abgeschnitten** (etwa bis S. 25–30), der Rest der Arbeit ist ungelesen.
>
> **Kennzeichnung:**
> - **[wörtlich, Textebene]** — Wortlaut bzw. Tabellenzeile aus der Textebene des Dokuments (pdftotext oder
>   DSpace-Text), von mir gelesen; Zeichen wie η, ω, σ gingen beim Auszug teils verloren und sind ergänzt, wo markiert.
> - **[Textebene, Tabelle umgestellt]** — aus der Textebene, aber die Spalten waren im Auszug verschoben und sind
>   von mir zugeordnet.
> - **[wörtlich, Werkzeug]** — WebFetch hat Wortlaut/Tabelle geliefert (HTML-Seite).
> - **[wörtlich, API]** — Feldinhalt einer API-Antwort.
> - **[nur Zusammenfassung]** — nur Stichpunkt einer Suche oder eines Abrufs; keine Belegkraft.
> - **[eigene Rechnung]** — von mir gerechnet.
>
> Kein Treffer ist kein Beweis.

---

## 1 Quellen

| Nr. | Quelle | Zugang | Rechtslage (Wortlaut, Ort) |
|---|---|---|---|
| G1 | Ducceschi, M.; Russo, R.; Webb, C. J.: „Measurement-Informed Nonlinear Modal Synthesis of 65 Classical Guitars“, Proc. DAFx26, Cambridge MA, S. 323–330 | dafx.de/paper-archive/2026/papers/DAFx26_paper_40.pdf (8,2 MB), im Cache | „This is an open-access article distributed under the terms of the Creative Commons Attribution 4.0 International License“ (S. 1, Fußzeile) [wörtlich, Textebene] |
| G2 | Nemus-Project/65_modelled_guitars (GitHub), Begleitrepo zu G1, angelegt 2026-03-27 | api.github.com | `"license": null` [wörtlich, API]; keine Lizenzdatei |
| G3 | Woodhouse, J.; Manuel, E. K. Y.; Smith, L. A.; Wheble, A. J. C.; Fritz, C.: „Perceptual Thresholds for Acoustical Guitar Models“, Acta Acustica united with Acustica 98(3), 475–486 (2012), DOI 10.3813/AAA.918531 | DAEL `dael.euracoustics.org/bin/EAA/aaua_dl?document_id=65199` (1,1 MB), im Cache; HAL hal-01461755 gesperrt | DAEL: „Document downloaded by @DAEL … personal use only“ (jede Seite) [wörtlich, Textebene]; HAL-API: `licence_s: https://creativecommons.org/licenses/by/4.0/` [wörtlich, API] |
| G4 | Woodhouse, J.; Lynch-Aird, N.: „Choosing Strings for Plucked Musical Instruments“, Acta Acustica united with Acustica 105, 516–529 (2019), DOI 10.3813/AAA.919333 | repository.cam.ac.uk/handle/1810/294170, Bitstream 1,34 MB, im Cache | „open access article under the terms of the CC BY 4.0 license“ (S. 516) [wörtlich, Textebene]; Repositorium: „Attribution 4.0 International“ [wörtlich, Werkzeug] |
| G5 | Scherrer, B.: „Physically-informed indirect acquisition of instrumental gestures on the classical guitar: Extracting the angle of release“, Diss. McGill 2013 | mcgill.scholaris.ca (DSpace), PDF 36 790 038 B; Textfassung 100 969 B (abgeschnitten) | „All items in eScholarship@McGill are protected by copyright with all rights reserved unless otherwise indicated.“ [wörtlich, API `dc.rights`] |
| G6 | Traube, C.: „An interdisciplinary study of the timbre of the classical guitar“, Diss. McGill 2004 | mcgill.scholaris.ca, PDF 19 466 637 B; Textfassung 100 250 B (abgeschnitten) | „The author retains copyright“ [wörtlich, Textebene, Vorspann] |
| G7 | Elie, B.; Gautier, F.; David, B.: „Macro parameters describing the mechanical behavior of classical guitars“, JASA (2012), Vorabdruck „Dated: September 30, 2012“ | members.loria.fr/BElie/papers/Elieetal_jasa2012.pdf (328 KB), im Cache | im Vorabdruck kein Vermerk gefunden (Suche nach „copyright“, „©“: 0) |
| G8 | Mores, R.: „Archive for the acoustical documentation of classical Spanish guitars, flamenco guitars and romantic guitars … – bridge mobility“, Zenodo 4604577, 2021-03-15 | lokal vorhanden | `license: cc-by-4.0` [wörtlich, API Zenodo] |
| G9 | D’Addario EJ45 (Normal) — Händler Soundpure, Tabelle „Diameter / Tension“ | soundpure.com/p/…/7799 | nicht gelesen |
| G10 | Rau, M.; Maestre, E.; Smith, J. O.; Scavone, G.: „An Exploration of Guitar Neck Admittance Measurements Taken at Different String Stopping Locations“, ISMA 2017 | isma2017.cirmmt.mcgill.ca, im Cache | nicht gesucht |
| G11 | Savarez 500CR (Herstellerseite) | savarez.com | „Copyright © Savarez“ [wörtlich, Werkzeug] |

Nur als Literaturangabe gesehen (nicht gelesen):
- Woodhouse, J. 2004a „On the synthesis of guitar plucks“, Acta Acust. 90, 928–944; 2004b „Plucked guitar
  transients: Comparison of measurements and synthesis“, Acta Acust. 90(5), 945–965 [wörtlich, Textebene, G1 Lit. 23].
- Hill, T. J. W.; Richardson, B. E.; Richardson, S. J.: „Acoustical parameters for the characterisation of the
  classical guitar“, Acta Acust. Acust. 90, 335–348 (2004) [wörtlich, Textebene, G7 Lit. 10].
- Chaigne, A. 1991, Daten zu D’Addario Pro Arté, „[Chaigne, 1991, Table 1]“ [wörtlich, Textebene, G5 Tab. 2.2].
- Pavlidou, M. 1997, Diss. Cardiff, „Fig. 6.21, p. 136“ und „Fig. 6.18“ [wörtlich, Textebene, G5].
- Woodhouse, J.; Langley, R. S. 2012: „Interpreting the input admittance of violins and guitars“, Acta Acust. 98,
  611–628 [nur Zusammenfassung].

## 2 Saitensatz mit Zug je Saite (650 mm)

**Gemessener Satz (Woodhouse):** G3 Tab. I, Saitensatz (a) „D'Addario Pro Arté "Composites, hard tension" nylon
strings“ [wörtlich, Textebene]. Dieselben Saiten nennt G1 als Quelle der Dämpfung „(hard tension, 650 mm scale)“.

| Saite | Note | f (Hz) | T (N) | m (g/m) | EI (10⁻⁶ N m²) | B (10⁻⁶) | Kennz. |
|---|---|---|---|---|---|---|---|
| 1 | E4 | 329.6 | 70.3 | 0.38 | 130 | 43 | [wörtlich, Textebene] G3 Tab. I |
| 2 | B3 | 246.9 | 53.4 | 0.52 | 160 | 70 | dto. |
| 3 | G3 | 196.0 | 58.3 | 0.90 | 310 | 124 | dto. |
| 4 | D3 | 146.8 | 71.2 | 1.95 | 51 | 17 | dto. |
| 5 | A2 | 110.0 | 73.9 | 3.61 | 40 | 13 | dto. |
| 6 | E2 | 82.4 | 71.6 | 6.24 | 57 | 19 | dto. |

- Einheit EI im Auszug „(10-6 N/m2 )“ [wörtlich, Textebene]; gemeint ist N m².
- Probe: f = (1/2L)·√(T/m) mit L = 0,65 m gibt 330,9 / 246,5 / 195,8 / 147,0 / 110,1 / 82,4 Hz [eigene Rechnung] —
  die Tabelle ist in sich stimmig.
- Summe der Wellenwiderstände √(T·m) der sechs Saiten: 2,117 N s/m [eigene Rechnung]; G3 nennt „2.12 Ns/m“
  [wörtlich, Textebene] — gleich.
- Zum Bau: „the top three strings are monofilaments while the lower three have multifilament cores with metal
  wind[ings]“ [wörtlich, Textebene, G3 S. 480, Wortende im Auszug getrennt].

**Herstellerangaben D’Addario (nur über Händler; Herstellerseite gab HTTP 429):**

| Saite | Art.-Nr. | d (inch) | d (mm) | T (lbs) | T (kg) | Kennz. |
|---|---|---|---|---|---|---|
| 1 E | J4501 | 0.0280 | 0.7112 | 16.230 | 7.360 | [Werkzeug, Tabelle umgestellt] G9 |
| 2 B | J4502 | 0.0322 | 0.8180 | 12.040 | 5.460 | dto. |
| 3 G | J4503 | 0.0403 | 1.0236 | 11.880 | 5.390 | dto. |
| 4 D | J4504 | 0.0290 | 0.7366 | 15.620 | 7.080 | dto. |
| 5 A | J4505 | 0.0350 | 0.8890 | 15.890 | 7.210 | dto. |
| 6 E | J4506 | 0.0430 | 1.0922 | 14.190 | 6.430 | dto. |

- Werkstoff: „Pro-Arte basses are wound using silver-plated copper on a multi-filament nylon core“, Diskant „Laser
  sorted clear nylon treble strings“ [wörtlich, Werkzeug, G9]. Mensur auf der Seite **nicht** angegeben.
- EJ46 (Hard), nur aus der Suche: J4601 0.0285″ 7.63 kg, J4602 0.0327″ 5.63, J4603 0.041″ 5.58, J4604 0.030″ 7.37,
  J4605 0.036″ 7.69, J4606 0.044″ 6.90 kg [nur Zusammenfassung].
- **Widerspruch:** zZounds nennt für EJ45 15.3/11.6/12.1/15.6/15.0/14.0 lbs [nur Zusammenfassung] — weicht bis 1 lb ab.
- „Unit weight“: kein Herstellerwert gefunden. Die Formel UW = (T × 386.4)/(2 × L × F)² stehe im
  D’Addario-Tension-Guide [nur Zusammenfassung]; der Link `daddario.com/globalassets/pdfs/accessories/tension_chart_13934.pdf`
  führte auf eine Rechner-Seite ohne Tabelle [wörtlich, Werkzeug].

**Masse je Länge aus den Händlerwerten, L = 0,65 m, μ = T/(2Lf)²** [eigene Rechnung, g = 9,80665]:

| Saite | EJ45 T (N) | EJ45 μ (g/m) | ρ gleichwertig (kg/m³) | EJ46 T (N) | EJ46 μ (g/m) |
|---|---|---|---|---|---|
| 1 | 72.2 | 0.393 | 989 | 74.8 | 0.407 |
| 2 | 53.5 | 0.520 | 989 | 55.2 | 0.536 |
| 3 | 52.9 | 0.814 | 989 | 54.7 | 0.843 |
| 4 | 69.4 | 1.906 | 4472 | 72.3 | 1.984 |
| 5 | 70.7 | 3.458 | 5570 | 75.4 | 3.688 |
| 6 | 63.1 | 5.494 | 5864 | 67.7 | 5.896 |

- Summe EJ45 38,9 kg, EJ46 40,8 kg [eigene Rechnung].
- Die Diskantsaiten ergeben für alle drei dieselbe Dichte 989 kg/m³ — die Händlerwerte sind offenbar **aus einer
  festen Dichte gerechnet**, nicht gemessen (Vermutung). Gemessen: Nylon ρ = 1070 kg/m³ (G4 Tab. I), G1 setzt
  1140 kg/m³.
- **G1 Tab. 1** „String parameters (D'Addario EJ45)“: Saiten 1–3 „plain nylon (ρ = 1140 kg/m3, E = 2.7 GPa)“, 4–6
  „wound (E = 2.5 GPa)“; d = 0.67 / 0.82 / 1.00 / 0.79 / 0.97 / 1.15 mm; ρ = 1140 / 1140 / 1140 / 4500 / 5200 /
  5900 kg/m³ [wörtlich, Textebene, ρ-Zeichen ergänzt]. **Achtung:** d der Diskantsaiten weicht vom Händlerwert ab
  (0,67 statt 0,711 mm; 1,00 statt 1,024 mm); die Dichten der Bässe passen zu meiner Rechnung oben auf ±7 %.

## 3 Saitenkennwerte: Dämpfung, Steife, Inharmonizität

**Dämpfungsgesetz (Valette, drei Parameter), G3 Gl. (4)** [wörtlich, Textebene, Symbole η, ω ergänzt]:
η_n = [T (η_F + η_A/ω_n) + EI η_B (nπ/L)²] / [T + EI (nπ/L)²]. „The damping is lowest in the mid range of
frequencies, with a minimum level set by η_F“; η_A für Luftdämpfung, η_B „associated with the complex bending
stiffness of the string“ [wörtlich, Textebene]. Inharmonizität f_n ≈ n f₀ √(1 + B n²), B = π² EI/(T L²)
(G3 Gl. 2–3) [wörtlich, Textebene, π ergänzt].

| Saite | η_F (10⁻⁵) | η_B (10⁻²) | η_A (1/s) | Kennz. |
|---|---|---|---|---|
| 1 E4 | 40 | 2.4 | 1.5 | [wörtlich, Textebene] G3 Tab. I (a) |
| 2 B3 | 40 | 2.0 | 1.2 | dto. |
| 3 G3 | 14 | 2.0 | 1.7 | dto. |
| 4 D3 | 5 | 2.0 | 1.2 | dto. |
| 5 A2 | 7 | 2.5 | 0.9 | dto. |
| 6 E2 | 2 | 2.0 | 1.2 | dto. |

- Gewonnen aus Tönen auf jedem Bund einer Saite als „cloud plot of damping factor against frequency“; „Approximate
  values of A, F, B can be estimated by fitting equation (4) to the trend of this floor level“, „carried out
  manually“ [wörtlich, Textebene, G3 S. 479].
- Für Stahlsaiten (Martin 80/20, neu und gespielt) stehen in derselben Tabelle die Zeilen (b), (c) — hier weggelassen.

**Werkstoffgesetz Nylon (G4 Tab. I und Gl. 13–16)** [wörtlich, Textebene]:

| Größe | Wert | Kennz. |
|---|---|---|
| E′ Nylon | „4.5 + 39 σ“ GPa, σ in GPa (Zugspannung) | [wörtlich, Textebene, σ ergänzt] |
| E″ Nylon | 0.25 GPa (unabhängig von der Spannung) | dto. |
| ρ Nylon | 1070 kg/m³ | dto. |
| Fluorcarbon | E′ 3.2 + 41 σ GPa, E″ 0.18 GPa, ρ 1800 kg/m³ | dto. |
| Darm | E′ 6.0 GPa, E″ 0.24 GPa, ρ 1320 kg/m³ | dto. |
| δ_E = E″/E′ | „falling from approximately 0.04 to 0.02 as stress increases“ | dto. |
| η_bend | ≈ β δ_E (Gl. 14); β aus Gl. (7) mit E, d², n², L⁴, f₁² — Gl. (7) im Auszug unvollständig, nicht übernehmen ohne Blick ins PDF | [Textebene, unvollständig] |
| η_air | klassisch nach Fletcher/Rossing mit „ad hoc correction factor (d + 0.2)“, d in mm; ρ_a = 1.2 kg/m³, ν_a = 1.5·10⁻⁵ m²/s | [wörtlich, Textebene] |
| η_body | „small enough that it can be ignored“ (SEA-Schätzung, hohe Frequenzen) | [wörtlich, Textebene] |
| Schwelle „musikalisch“ | „a threshold value η ≈ 0.05“ | [wörtlich, Textebene, η ergänzt] |
| Rolloff | drei Nylonsaiten 0,65 m an derselben Gitarre: d 0.50 mm @ 327.5 Hz Rolloff „around 13 kHz“, d 0.96 mm @ 327.5 Hz „around 7 kHz“, d 1.68 mm @ 131 Hz nur bis 1,5 kHz messbar | [wörtlich, Textebene, Fig. 2] |

- Folge für „gegriffen am Bund“ [eigene Einschätzung]: Zug bleibt beim Greifen gleich, also E′ gleich; mit kürzerer
  Länge wächst B ∝ 1/L² und η_bend ∝ n²/L⁴ — das Gesetz aus G4 trägt die Änderung je Bund ohne neue Messung.
- G5 Tab. 2.2 (nach Chaigne 1991, D’Addario Pro Arté): √(ES/T) = 5.98 / 7.62 / 8.09 / 5.91 / 6.22 / 6.30
  (Saite I–VI) [wörtlich, Textebene] — Verhältnis Längs- zu Quergrundton.
- G1: Saitenverluste „two-parameter model of Woodhouse“, σ₀,s = 6 ln 10 / T60,s; „T60,s ∈ [2.43, 4.58] s, σ_f,s ∈
  [2.0, 2.5]×10⁻⁴ s, with the G string (string 3) being the most heavily damped of the three plain-nylon trebles“
  [wörtlich, Textebene, Symbole ergänzt]. In G3 Tab. I stehen keine T60; die Herkunft der T60 bei Woodhouse habe ich
  nicht geprüft.

## 4 Zupf

| Größe | Wert | Quelle | Kennz. |
|---|---|---|---|
| Auslenkung beim Ablösen h, 16 cm vom Steg | „soft“ 0.8×10⁻³ m, „strong“ 4×10⁻³ m | G5 Tab. 2.1 nach Pavlidou 1997, Fig. 6.21, S. 136 | [wörtlich, Textebene] |
| Neigung am Steg h/x₀ | 5×10⁻³ bzw. 25×10⁻³ | G5 Tab. 2.1 | [wörtlich, Textebene] |
| Breite von Finger/Nagel (Tiefpass) | Gauß-Filter exp(−ω²/K²), K = v/h, „width h was given the value 7.5 mm“ | G3 S. 477 | [wörtlich, Textebene, Formel im Auszug verstümmelt] |
| Kontaktbreite (Annahme) | „w = 2 cm“ für Grenzfrequenzen der nicht punktförmigen Anregung | G5 Tab. 3.1 (Titel) | [wörtlich, Textebene] |
| Zupfstelle normal | „somewhere between a third and a tenth of the string length (i.e. 3 to 20 cm)“ | G6 S. 25 | [wörtlich, Textebene]; ⅓·650 = 217 mm, ⅒·650 = 65 mm [eigene Rechnung] — die Klammer passt nicht genau |
| Spielorte (Biberian, *Prisms II*) | Flautando „at the half-way nodal point“; Sul tasto „between 12th and 19th frets“; Normale „between sound hole and bridge, but closer to the sound hole“; Ponticello „as near the bridge as possible“ | G6 S. 26 | [wörtlich, Textebene] |
| Gemessene Zupfstellen | „plucking position p ranging from 4 to 17 cm from the bridge“; „from 3 cm to 30 cm“ | G6 Abb.-Verz. | [wörtlich, Textebene] |
| Zupfstelle im Modell G1 | e = x_e/L ∈ [0.68, 0.82], Steg bei b = 0.995 → 0,18–0,32 L vom Steg = 114–205 mm | G1 Tab. 2 | [Textebene, Tabelle umgestellt] + [eigene Rechnung] |
| Zupfwinkel (Ablösewinkel) | Messung „with θr ∈ [0°; 90°]“, 15 cm vom Steg, Beschleunigung am Steg bei Saite V | G5 Abb. 3.16 | [wörtlich, Textebene] |
| Apoyando gegen tirando | „Difference between rest and free strokes in terms of the AOR“, Bahnen „from [Pavlidou, 1997, Fig. 6.18]“ | G5 Abb. 3.12 | [wörtlich, Textebene]; **Winkel als Zahl nicht gelesen** (Kapitel 3 jenseits der Textfassung) |
| Zupfkraft im Modell | Datensätze mit „plucking force of 0.25 N“, eine Gitarre mit 0.1 / 0.25 / 0.5 / 1.0 / 1.5 N | G2 README | [wörtlich, API] |
| Kraftverlauf | aufgezeichneter Impuls, „normalised, and scaled by a peak amplitude Fmax“ | G1 §2.5 | [wörtlich, Textebene] |
| Zupf als Stufe | „The string is assumed to be released from rest“; Kraftstufe am Steg (Reziprozität); Winkel mischt die Polarisationen | G3 S. 477 | [wörtlich, Textebene] |

- Gezupfte Saite bei G5: L = 65 cm, T = 70 N als Rechenbeispiel (Abb. 3.8) [wörtlich, Textebene].
- **Nicht gefunden:** Kraft beim Ablösen in N und Dauer der Ablösung je Spielart; Winkel apoyando/tirando als Zahl.

## 5 Steg und Körper

| Größe | Wert | Quelle | Kennz. |
|---|---|---|---|
| Mores-Gitarren | 65 Gitarren, Bau 1803–2018, Mensur 551–668 mm, Masse 670 g bis über 1750 g | G1 §3.1 | [wörtlich, Textebene] |
| Messung Mores | Hammer Dytran 5800SL (9,8 g) am Stegsattel, zwei Beschleunigungsaufnehmer (Diskant-, Bassseite), drei Mikrofone 10 cm über der Decke, 48 kHz/24 bit | G1 §3.2 | [wörtlich, Textebene] |
| Modalfit Steg (G1) | Fenster 3000 Samples, Spitzen 20–10 000 Hz (Prominenz ≥ 0,5 dB, Abstand ≥ 1 Hz), Dämpfung aus −3-dB-Breite, Residuen per Tikhonov-LS reell, Moden mit ζ < 0,2 ω verworfen (im Auszug „j < 0.2j“) | G1 §3.4 | [wörtlich, Textebene, Symbole verloren] |
| Umfang des Fits | „retaining 50–200 bridge modes and 80–300 radiation poles per side“; Abstrahlung bis 12 kHz; Beispiel Nr. 55: 68 Stegmoden, 125 Abstrahlpole | G1 §5.1, Abb. 4–5 | [wörtlich, Textebene] |
| Richtung | nur senkrecht (Hammer am Sattel); „Only the treble-side pairs are used here“ | G1 §3.2 | [wörtlich, Textebene]; Richtung „senkrecht“ ist mein Schluss |
| **Veröffentlichte Moden** | **keine** — G2 enthält nur WAV (83 Dateien, 9 Gitarren, Musikbeispiele), keinen Code, keine Modentabellen, keine Lizenz; G1 verspricht „together with the synthesis code“ unter dieser URL | G1 §6, G2 | [wörtlich, Textebene] + [wörtlich, API] |
| Admittanz 2×2 je Saitenort | Anhang A: Yzz, Yyy, Yzy „measured using hammer-LDV, for string I … VI“; Modell nach Bank & Karjalainen 2010 | G5 Abb.-Verz. A.4–A.13, B.1–B.4 | [wörtlich, Textebene]; **Werte nicht gelesen** (nur im PDF) |
| Lehre senkrecht/waagrecht | „the bridge on a guitar is typically easier to "push" into the top plate than it is to "shear" sidewise“ → zweistufiger Ausklang | Smith, PASP (CCRMA) | [nur Zusammenfassung] |
| 15 Gitarren, tiefe Moden | Helmholtz 75–100 Hz und erste Deckenmode 150–250 Hz: relative Streuung der Frequenz „5.5% and 6%“, Q „about 33%“; Kompensation der gedämpften Saiten über ΣZ = 2,12 N s/m | G3 S. 482–483, Abb. 7 | [wörtlich, Textebene]; Mittelwerte nur in der Abbildung |
| Admittanz normal, vier Gitarren | Spitze 80–100 Hz (Helmholtz), Häufung 150–250 Hz, Abfall bis Senke 400–500 Hz; dB re 1 m/(N s) | G3 Abb. 2 | [wörtlich, Textebene]; Kurven nicht abgelesen |
| Abstrahlung im Modell G3 | Kugelstrahler, Tiefpass mit „roll-off at 250 Hz, corresponding to a sphere radius around 0.2 m“ | G3 S. 477 | [wörtlich, Textebene] |
| Modenüberlapp 30 % ab | 9 Gitarren von Gitarrenbauern 465–910 Hz, 3 Industriegitarren 506–635 Hz (Tab. IV, f30) | G7 | [wörtlich, Textebene] |
| Hill et al. 2004 | effektive Masse, Q, Monopol je Mode für mehrere Gitarren | Acta Acust. 90, 335–348 | **nicht erreicht** |
| Bund als Ende | Admittanz am Hals an Bund 0/2/4/6/8/10, zwei Richtungen — aber an einer **Hollow-Body-E-Gitarre** | G10 | [wörtlich, Textebene] |

## 6 Bund und Griff

| Größe | Wert | Quelle | Kennz. |
|---|---|---|---|
| Bundabstand vom Sattel, 12-TET | x_n = L(1 − 2^(−n/12)): Bund 1 36,5 mm, 2 70,9, 3 103,4, 5 163,1, 7 216,2, 12 325,0, 19 433,1 mm | — | [eigene Rechnung] |
| Saitenlage, Sattel-/Stegkompensation | — | — | **nicht gesucht** (Zeitrahmen) |

## 7 Ziele: Ausklang, Dynamik

| Größe | Wert | Quelle | Kennz. |
|---|---|---|---|
| T60 am Grundton je Saite (leer) | 2,43–4,58 s (Spanne über die sechs Saiten, nicht je Saite) | G1 §5.1 nach Woodhouse 2004b | [wörtlich, Textebene] |
| Dämpfung je Bund | Punktwolken je Saite (G3 Abb. 3, Saite 3) | G3 | nur Abbildung |
| Dynamik | Synthese mit 0,1–1,5 N Zupfkraft (eine Gitarre) als Hörvergleich | G2 | [wörtlich, API] |
| Pegel über den Umfang | — | — | **nicht gefunden** |

## 8 Ladeliste (Vorschlag — geladen wird erst nach Freigabe)

| Datei (Vorschlag) | URL | Größe | Lizenz / Recht | Wozu |
|---|---|---|---|---|
| `Ducceschi_Russo_Webb_2026_65ModelledGuitars_DAFx26.pdf` | https://www.dafx.de/paper-archive/2026/papers/DAFx26_paper_40.pdf | 8,2 MB | CC BY 4.0 (S. 1) | Weg des Modalfits an Mores, Parameter Tab. 1–2 |
| `Woodhouse_LynchAird_2019_ChoosingStrings.pdf` | https://api.repository.cam.ac.uk/server/api/core/bitstreams/fe311a9e-af57-4b43-9fa7-5fbef825c7db/content | 1,34 MB | CC BY 4.0 | Werkstoffgesetz Nylon, Dämpfung je Teilton |
| `Woodhouse_etal_2012_PerceptualThresholdsGuitarModels.pdf` | https://dael.euracoustics.org/bin/EAA/aaua_dl?document_id=65199 | 1,1 MB | DAEL „personal use only“; HAL-Feld CC BY 4.0 (hal-01461755, dort gesperrt) | Tab. I: vollständiger Saitensatz mit η_F, η_B, η_A; Zupfbreite |
| `Scherrer_2013_AngleOfRelease_Diss.pdf` | https://mcgill.scholaris.ca/server/api/core/bitstreams/dfcf6f49-8a14-410e-829e-275e973ba4ef/content | 36,8 MB | „all rights reserved“ (nur lesen, Zahlen als Fakten) | 2×2-Admittanz je Saitenort (Anh. A/B), Ablösewinkel, Pavlidou-Daten |
| `Traube_2004_TimbreClassicalGuitar_Diss.pdf` | https://mcgill.scholaris.ca/server/api/core/bitstreams/6537ba36-1616-46df-99ab-c809883788bb/content | 19,5 MB | „The author retains copyright“ | Zupfstellen gemessen (4–17 cm, 3–30 cm) |
| `Elie_Gautier_David_2012_MacroParametersGuitars.pdf` | https://members.loria.fr/BElie/papers/Elieetal_jasa2012.pdf | 328 KB | kein Vermerk im Vorabdruck | Grenze f30, Ersatzplatte (nachrangig) |
| (Hörvergleich, kein Laden ins Repo) `65_modelled_guitars` WAV | https://github.com/Nemus-Project/65_modelled_guitars | Repo 158 MB (API `size`) | **keine Lizenz** — nur anhören | Vergleich mit eigenem Modell an denselben Gitarren |

Die ersten drei PDFs und Elie liegen schon im WebFetch-Cache dieser Sitzung (Pfad im Kopf) — für die Ablage unter
`…\cmake\quellen\` müssten sie trotzdem nach Freigabe neu geholt oder von dort kopiert werden.

## 9 Nicht gefunden / nicht gelesen

- **Woodhouse 2004a/b im Volltext** (Admittanz senkrecht/parallel, Modenliste, T60 je Saite): kein freier Ort
  gefunden; DAEL führt vermutlich die Hefte von Acta Acustica 90 (Muster wie G3), Seiten-ID nicht gefunden.
- Hill, Richardson & Richardson 2004 (effektive Massen, Q, Monopol je Mode): kein freier Ort gefunden.
- Lambourg & Chaigne 1993, Christensen & Vistisen 1980, Cuzzucoli & Lombardo 1999: in der HAL-API (Chaigne + guitar)
  nicht vorhanden; die übrigen **nicht gesucht**.
- Pavlidou 1997 (Kraft, Dauer, Winkel des Zupfs): nur über G5 zitiert; Volltext nicht gesucht (ORCA war im Oktober
  gesperrt).
- Scherrer 2013 ab Kapitel 3 und Anhang A (Zahlen der 2×2-Admittanz, Ablösewinkel apoyando/tirando): Textfassung
  abgeschnitten, PDF 36,8 MB nicht geladen.
- D’Addario-Herstellertabelle (HTTP 429), „unit weight“; Augustine, Hannabach **nicht gesucht**; Savarez nur Bild.
- Woodhouse & Langley 2012 (Q über der Frequenz, 15 Gitarren): kein freier Ort gefunden.
- Saitenlage, Kompensation; Pegel über den Umfang; Dynamikumfang einer echten Nylongitarre.
- Eine **veröffentlichte Modentabelle** der Mores-Gitarren: G2 enthält entgegen G1 §6 keinen Code und keine Parameter.

## 10 Einschätzung (kein Beleg)

- **Saiten je Saite: reicht.** G3 Tab. I ist ein vollständiger, an einer Gitarre gemessener Satz (T, m, EI, B, η_F,
  η_B, η_A) für D’Addario Pro Arté Composites hard, 650 mm — genau die Saiten, die G1 für die Dämpfung nimmt. Für die
  Lage am Bund trägt das Werkstoffgesetz aus G4 (E′(σ), E″ fest, Luft mit (d + 0,2)), weil der Zug beim Greifen gleich
  bleibt. Die Händlerwerte von D’Addario (Normal/Hard) taugen für Zug und μ eines zweiten Satzes, sind aber offenbar
  aus fester Dichte gerechnet; die Steife der Bässe muss aus G3 kommen (nicht vorhersagbar, sagt G3 selbst).
- **Zupf je Saite: zum Teil.** Belegt sind Auslenkung (0,8–4 mm bei 16 cm), Breite (7,5 mm Gauß), Orte (Traube,
  Biberian, G1 0,18–0,32 L) und dass der Ablösewinkel die Polarisationen mischt. Es fehlen Zahlen für den Winkel
  apoyando/tirando und für Kraft/Dauer der Ablösung — beides steht vermutlich in Scherrer 2013 (Kap. 3) und Pavlidou.
  Erster Ladekandidat ist deshalb Scherrer.
- **Körper: Modalfit an Mores (G8) nach dem Weg von G1**, nicht Bauplan. Gründe: 65 gemessene Gitarren unter CC BY 4.0
  lokal vorhanden, der Fitweg ist in G1 §3.4–3.5 schrittweise beschrieben (50–200 Stegmoden, −3-dB-Dämpfung,
  Tikhonov-Residuen, Abstrahlung als Pol-Residuen-Filter je Seite); eine fertige Modentabelle gibt es **nicht**. Mores
  misst nur senkrecht: für die zweite Richtung (parallel) und die Kopplung braucht es eine 2×2-Messung — Scherrer
  Anh. A (eine Gitarre, alle sechs Saitenorte) oder Bank & Karjalainen 2010 (liegt aus IC Session 2 vor). Wright 1996
  Tab. 5.4 (tiefe Moden mit wirksamer Masse) dient als Gegenprobe der tiefen Moden; die Streuung aus G3 (Frequenz
  ±5,5–6 %, Q ±33 %) als Maß, ob die Mores-Fits plausibel streuen.
- **Am Bund:** Verluste am Hals beim Greifen sind nur an einer E-Gitarre gemessen (G10); für die Nylongitarre bleibt
  der Bund im Modell ein festes Ende, solange nichts Besseres vorliegt.

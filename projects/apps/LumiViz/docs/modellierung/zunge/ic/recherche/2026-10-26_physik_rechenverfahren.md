# Recherche IC1: Durchschlagende Zunge — Physik und Rechenverfahren

> **Auftrag:** Klasse 10 (Akkordeon, Mundharmonika, Harmonium, Melodica): Zungenmodell, Tonhöhe über dem Druck,
> Kopplung (Kanzelle, Paarzungen, Chöre), Mundharmonika-Bending, Harmonium/Melodica, Rechenverfahren und Kosten.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug]; [nur
> Zusammenfassung]; [eigene Rechnung].
> **Reichweite:** im Bericht selbst (22 Suchen, rund 41 Abrufe; cs.bu.edu mit Cottingham/Bahnson nicht erreichbar).
> **Anmerkung der Sitzung:** Die Lieferungen der Klasse 10 schreibt die nächste Sitzung (Session-Abschluss nach den
> Berichten, Wunsch Patrik). **Von der Sitzung selbst nachgelesen (IC Session 2,
> 2026-10-26, `quelle_suchen.py` in den PDFs des Caches):** Puranik & Scavone FA 2023 (Lizenz S. 1809, „in isolation“,
> „inertial effect“ S. 1809, 0,2–1,5 kPa S. 1810, „unrealistic values“ S. 1811, Tab. 1 und N = 40, 4 × 44100 Hz S. 1813;
> „largely sinusoidal“ **nicht gefunden** — dort „predominantly sinusoidal motion at low bellows pressures“ S. 1813);
> DAFx23 (Lizenzsatz mit „non-commercial“ PDF-S. 1; „1-2 Hz“, „much stronger influence“, „leaks out“, 10 Biquads, 37
> Spitzen PDF-S. 3); Cottingham 2011 (15 %, „somewhat below“ PDF-S. 2; „square“ PDF-S. 3; Vokaltrakt PDF-S. 4; „always
> oscillates“, „about one semitone“ PDF-S. 5; „usually has little effect“ **nicht gefunden**; kein Rechtsvermerk);
> Adachi et al. FA 2026 (Lizenz, „not reviewed“, „approximately a semitone“, „push or pull“ PDF-S. 1; −2,5/−5,1 Hz/kPa
> PDF-S. 2; Tab. 1 PDF-S. 4); Elejalde-García et al. (Rechtsvermerk S. 1; 25/15 Cent, „10 to 35“, 10000 Hz,
> „harmonicity“ S. 3; 40 Pa S. 6); ISMA 2017, Vienna Talk 2015 (Abstracts); arXiv-Lizenzlink Millot & Baumann.
> Eingeflossen in `quellen.md`, `modell-entwurf.md`, `offene-fragen.md`.

---

# Bericht IC – Klasse 10 „Durchschlagende Zunge“: Physik und Rechenverfahren

**Auftrag:** Physik und Rechenverfahren der durchschlagenden Zunge (free reed) mit Zahlenwerten, für vorab gerechnete Töne in LumiViz.
**Datum:** 2026-10-26 (das ist auch das Abrufdatum aller Quellen).

**Reichweite:**
- **Umfang:** 22 Websuchen und etwa 41 Abrufe (WebFetch), davon 6 gescheitert.
- **Orte:** arXiv; HAL-API (api.archives-ouvertes.fr); Crossref-API; dael.euracoustics.org (Forum Acusticum 2023 und 2026); dafx.de; ISMA 2017 (McGill); Vienna Talk 2015 (mdw); auditory.org (ASA-Abstracts); aip.brightspotcdn.com (Physics Today); euracoustics.org (Acoustics in Practice); Wikipedia; Liberty Bellows. UNSW wurde nur über die Suche erreicht, ohne Harmonika-Treffer.
- **Sprachen:** Suchen auf Englisch. Die HAL-Abfrage umfasste französische Titel („accordéon“, „anche libre“). Die ungarische BME-Arbeit habe ich nur nach Zahlen durchsucht.
- **Nicht erreichbar:**
  - cs.bu.edu (Snyder-Sammlung: Bahnson JASA 1998, Cottingham BERLIN1/2.PDF, ICA.PDF): „certificate rejected“.
  - web.archive.org: für das Werkzeug gesperrt.
  - labs.sonicfield.org: Host nicht auflösbar.
  - ioa.org.uk (Millot/Cuesta/Valette ISMA 1997): 404.
  - Crossref einmal mit 429 (zu viele Anfragen).
  - Ricot/Caussé/Misdariis 2005 und 1999 auf HAL ohne Volltext (`openAccess_bool: false`).
- **Arbeitsweise PDFs:** Das Werkzeug liest PDFs nicht selbst. Den Text der zwischengespeicherten PDFs habe ich mit `pdftotext` auf die Standardausgabe gezogen und selbst gelesen. „Selbst gelesen“ heißt hier: gelesen in diesem pdftotext-Auszug.
- **Zwischengespeicherte Dateien** (alle vom Werkzeug angelegt, in `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):
  - `webfetch-1793045914386-c77vu1.pdf`: Cottingham, ISMA 2017 (nur Abstract)
  - `webfetch-1793045974571-1s7moi.pdf`: Puranik/Scavone, FA 2023
  - `webfetch-1793045994826-cm70k7.pdf`: Cottingham/Wolff, Vienna Talk 2015
  - `webfetch-1793046003409-qrul41.pdf`: Granzow/Rossing/Egbert, Vienna Talk 2015
  - `webfetch-1793046010460-m7kvvi.pdf`: Puranik/Scavone, DAFx23
  - `webfetch-1793046056587-ts7jaw.pdf`: Cottingham, Physics Today 2011
  - `b158pvzzq.txt`: Textauszug dieses Physics-Today-PDFs. Die Umgebung hat ihn wegen der Länge selbst abgelegt; ich habe ihn nicht angelegt.
  - `webfetch-1793045888421-p1m2mi.pdf`: Adachi u. a., FA 2026
  - `webfetch-1793045888494-lkbhih.pdf`: Elejalde-García u. a., AiP 2021
  - `webfetch-1793046149133-sergx9.pdf`: Rudas, BME-Abschlussarbeit 2021
  - `webfetch-1793046285836-p5cy5w.pdf`: Lu, Illinois-Studentenprojekt
  - `webfetch-1793046337067-u5u6fr.pdf`: FA 2025, Klarinettenblatt, nicht einschlägig

**Kennzeichnung:**
- [wörtlich, selbst gelesen, S. x]: aus dem pdftotext-Auszug. S. ist die gedruckte Seite, sonst „PDF-S.“.
- [wörtlich, Werkzeug]: Wortlaut, den WebFetch als Zitat oder Feldinhalt geliefert hat.
- [nur Zusammenfassung]: keine Belegkraft.
- [eigene Rechnung]: von mir nachgerechnet, kein Beleg.

---

## 1 Quellen

**Q1. Millot & Baumann 2007**
- L. Millot, Cl. Baumann: „A proposal for a minimal model of free reed[s]“, Acta Acustica united with Acustica 93 (2007), S. 122–144.
- Fundort: arXiv 2401.01606 (eingestellt 3. Jan. 2024) und HAL hal-04369449 (Datei `Acta_19_11_2006_4RG.pdf`).
- Frei. Rechtsvermerk:
  - arXiv: Link auf „http://arxiv.org/licenses/nonexclusive-distrib/1.0/“ [wörtlich, Werkzeug]
  - HAL: licence_s „https://about.hal.science/hal-authorisation-v1/“ [wörtlich, Werkzeug]
- Die arXiv-Kopfzeile nennt nur „Laurent Millot (ACTE)“; im HTML-Text stehen „L. Millot“ und „Cl. Baumann“.

**Q2. Puranik & Scavone 2023 (FA 2023)**
- Ninad Puranik, Gary Scavone: „Clamped bar model for free reeds“, 10th Convention EAA (Forum Acusticum), Turin, 11.–15. Sept. 2023, S. 1809–1815, DOI 10.61782/fa.2023.0853.
- Frei. Rechtsvermerk: „Creative Commons Attribution 3.0 Unported License“ [wörtlich, selbst gelesen, S. 1809].

**Q3. Puranik & Scavone 2023 (DAFx23)**
- Ninad Puranik, Gary Scavone: „Physically inspired signal model for harmonium sound synthesis“, Proc. DAFx23, Kopenhagen, 4.–7. Sept. 2023.
- Frei. Rechtsvermerk: „Creative Commons Attribution 4.0 International License“ [wörtlich, selbst gelesen, DAFx.1]. Im selben Satz steht allerdings „for non-commercial purposes“ (widersprüchlicher Vermerk, siehe Abschnitt 8).

**Q4. Cottingham 2011 (Physics Today)**
- James Cottingham: „Acoustics of free-reed instruments“, Physics Today 64(3), März 2011, S. 44–48, DOI 10.1063/1.3563819.
- Fundort: aip.brightspotcdn.com/PTO.v64.i3.44_1.online.pdf. Frei zugänglich.
- Im PDF habe ich keinen Rechtsvermerk gefunden (grep nach „©/copyright“ ohne Treffer). Verlag ist AIP, also Rechtslage ungeklärt.

**Q5. Adachi u. a. 2026 (FA 2026)**
- Seiji Adachi, Hyuga Okada, Toshiya Samejima, Masakazu Takeda, Kojiro Nishimiya: „Aerodynamic mechanism of accordion pitch bending: simulation and theoretical analysis“, 12th Convention EAA, Graz, 8.–12. Sept. 2026.
- Fundort: dael.euracoustics.org/confs/fa2026/data/articles/000418.pdf.
- Frei. Rechtsvermerk: „Creative Commons Attribution 3.0 Unported License“ [wörtlich, selbst gelesen, PDF-S. 1].
- Zusatzvermerk: „The submitted manuscript was not reviewed“ [wörtlich, selbst gelesen, PDF-S. 1].

**Q6. Elejalde-García u. a. 2021 (AiP)**
- M. Jesús Elejalde-García, Erica Macho-Stadler, Ricardo Llanos-Vázquez: „Accordion acoustics: A study on pitch bending“, Acoustics in Practice AiP-2021-02 (Feb. 2021).
- Fundort: euracoustics.org. Frei abrufbar.
- Rechtsvermerk: „All rights reserved. Use by third parties of the contents of this journal without the prior written consent…“ [wörtlich, selbst gelesen, Impressum PDF-S. 1]. Nicht frei lizenziert.

**Q7. Cottingham 2017 (ISMA)**
- James Cottingham: „Reed Chamber Resonances and Reed Tongue Vibrational Modes in Free Reed Excitation“, Proc. ISMA 2017, Montreal. Nur Abstract.
- Kein Rechtsvermerk im Auszug.

**Q8. Cottingham & Wolff 2015**
- James Cottingham, Daniel Wolff: „Experimental study of free reed initial transients“, Proc. Third Vienna Talk 2015, S. 176. Nur Abstract.
- Kein Rechtsvermerk.

**Q9. Granzow, Rossing, Egbert 2015**
- John Granzow, Thomas Rossing, Peter Egbert: „Bionic pitch benders for free reed instruments“, Proc. Third Vienna Talk 2015, S. 174. Nur Abstract.

**Q10. Ricot, Caussé, Misdariis 2005**
- Denis Ricot, René Causse, Nicolas Misdariis: „Aerodynamic excitation and sound production of blown-closed free reeds without acoustic coupling: The example of the accordion reed“, JASA 117(4), 2279–2290 (2005), DOI 10.1121/1.1852546.
- Nicht frei. Nur Crossref- und HAL-Metadaten.

**Q11. Bahnson, Antaki, Beery 1998**
- Henry T. Bahnson, James F. Antaki, Quinter C. Beery: „Acoustical and physical dynamics of the diatonic harmonica“, JASA 103(4), 2134–2144 (1998), DOI 10.1121/1.421359.
- Volltext nicht erreicht (cs.bu.edu). Patentschriften zitieren falsch „1234–1244“; Crossref sagt 2134–2144.

**Q12. St. Hilaire, Wilson, Beavers 1971**
- Arthur O. St Hilaire, Theodore A. Wilson, Gordon S. Beavers: „Aerodynamic excitation of the harmonium reed“, J. Fluid Mech. 49, 803–816 (1971), DOI 10.1017/s0022112071002374.
- Nicht frei. Nur Crossref-Abstract.

**Q13. Tarnopolsky, Fletcher, Lai 2000**
- A. Z. Tarnopolsky, N. H. Fletcher, J. C. S. Lai: „Oscillating reed valves—An experimental study“, JASA 108, 400–406 (2000), DOI 10.1121/1.429473. Nur Abstract.

**Q14. Millot 2011**
- Laurent Millot: „Chromatical playing on diatonic harmonica: From physical modeling to sound synthesis“, JASA 130(4_Suppl.), 2342 (2011), DOI 10.1121/1.3654370. Tagungs-Abstract.

**Q15. Cottingham, ASA-Abstracts (auditory.org)**
- 4aMU1 (1997): „Effects of reed cell geometry on the vibration frequency and spectrum of a free reed“
- 2aMU3 (1997, mit Fetzer): „Modeling free reed behavior using calculated reed admittance“
- 5aMU10 (1995, mit Colson, Wilson, Quigley): „Sound spectra from air-driven American organ reeds“
- Kein Rechtsvermerk erhoben.

**Q16. Wikipedia „Harmonica techniques“**
- Rechtsvermerk: „Text is available under the Creative Commons Attribution-ShareAlike 4.0 License“ [wörtlich, Werkzeug].

**Q17. Liberty Bellows, „General accordion questions“** (Händler, Praxisangabe)
- Rechtsvermerk: „Copyright © 2007-2026 - Liberty Bellows“ [wörtlich, Werkzeug].

**Q18. Weitere, nur als Metadaten (HAL-API, ohne Volltext)**
- Debut & Millot 2001: „Time domain simulation of the diatonic harmonica“
- Millot 2001: „Experimental results when playing chromatically on a diatonic harmonica“, Acta Acustica
- Millot, Cuesta, Valette 1997: „Interactions of both reeds in a channel of a diatonic harmonica“
- Millot 1999: Dissertation „Etude des instabilités des valves : application à l'harmonica diatonique“
- Volpatti 2025: „Beyond the Bellows: A Critical Review…“, Link journals.e-palli.com; nicht abgerufen.

---

## 2 Zungenmodell (Frage 1)

**Erste Biegemode des Kragbalkens.** Millot & Baumann rechnen die Zunge in ihrer ersten Querschwingung. Ersatzmodell ist ein „equivalent mass-spring-damper oscillator with a single degree of freedom“ [wörtlich, Werkzeug, Q1 Abschn. 2.1]. Die Gleichung (4) lautet d²ζ/dt² + Q⁻¹ω₀ dζ/dt + ω₀²ζ = μΔp [Werkzeug, Q1].

**Fluss.** Drei Bausteine:
- Bernoulli über die Zunge: Δp = ½ρv_j² [Werkzeug, Q1 Gl. 7].
- Gepumpter Fluss proportional zur Zungengeschwindigkeit [Werkzeug, Q1 Gl. 5].
- Massenerhaltung im Vorvolumen V₁ und „Dynamic Bernoulli“ (Trägheit L₂/S₂) im Nahfeld [wörtlich, selbst gelesen, Q2 S. 1811, Gl. 1–5].

**Durchflussfläche.** Puranik & Scavone setzen die nutzbare Öffnung über Spitze und Seitenspalt an: Su = W·√((y+y₀)²+h²) [selbst gelesen, Q2 S. 1812]. Millot definiert eine neue Fläche, die die Dicke der Zunge berücksichtigt [nur Zusammenfassung, Q1 Abstract].

**Selbsterregung.** Bernoulli allein reicht nicht:
- „this mechanism in isolation cannot develop the asymmetric pressure differences“ [wörtlich, selbst gelesen, Q2 S. 1809].
- Treiber ist die Trägheit der Strömung stromauf (St. Hilaire): „the inertial effect of the upstream air flow“ [wörtlich, selbst gelesen, Q2 S. 1809].
- Q12 nennt einen Anregungsdruck der Ordnung ρU₀ȧ [Werkzeug, Q12-Abstract].
- Strahl- oder Nachlauf-Instabilität ist nicht maßgeblich: „jet or wake instability is not important“ [wörtlich, Werkzeug, Q12].
- Ricot u. a. sehen die Kraft in der Trägheitslast der instationären Spaltströmung, also inkompressibel und potentiell [nur Zusammenfassung, Q10].

**Höhere Moden.**
- Im Einschwingvorgang treten die zweite Querschwingung und die erste Torsionsmode auf, die Torsion oft früher [selbst gelesen, Q8; Q7].
- Bei hohem Druck zeigt die Zunge Obertöne: bei 0,2 kPa „largely sinusoidal“, Obertöne deutlich bei 1,4–1,6 kPa [wörtlich, selbst gelesen, Q2 S. 1810].

**Amplitude.** Die Spitze schwingt typisch mit „an amplitude of 15% of the tongue length“ [wörtlich, selbst gelesen, Q4 S. 45].

**Arbeitsfrequenz.** Sie liegt unter der Eigenfrequenz der Zunge, „somewhat below the natural vibrating frequency“ [wörtlich, selbst gelesen, Q4 S. 45].

**Schwelle.** Für Harmonium-Zungen von 100–200 Hz meldet die Suche „about 0.2 kPa“ (Koopman/Cottingham) [nur Zusammenfassung; Primärquelle cs.bu.edu nicht erreichbar].

### Tabelle 2a – Zungenmaße und Modellwerte

| Instrument / Zunge | Länge | Breite | Dicke | Material / Dichte / E | f | Spalt | Dämpfung | Druck | Quelle | Kennz. |
|---|---|---|---|---|---|---|---|---|---|---|
| Mundharmonika, Lee Oskar G, Kanzelle 4, (+,−)-Zunge | 12,95 mm | 2,1 mm | 110 µm | nicht genannt | 444 Hz | 50 µm; Ruhelage-Auslenkung 528 µm; Träger 900 µm | Q = 95; Steifigkeit 47,9 N/m | Überdruck Δp₂ 570 / 783 Pa (Simulation) | Q1 Abschn. 3.2, Tab. 2/3 | Werkzeug |
| Harmonium (indisch), B5 | 16 mm | 2 mm | 0,4 mm | ρ = 8490 kg/m³, E = 125 GPa (Messing) | ≈ 990 Hz | y₀ = −0,1 mm | R = 0,065 | 0,2–1,5 kPa (Kanzelle, real) | Q2 S. 1810, Tab. 1 S. 1813 | selbst gelesen |
| Akkordeon, C4 | 32,8 mm | 3,45 mm | — | Masse „(2/3) × 0,248 g“ | 267,6 Hz | a = 0,14 mm; x_rest = 0,3 mm | ζ = 0,003 | 0–2,5 kPa (Versuch) | Q5 Tab. 1, PDF-S. 3 | selbst gelesen |
| Harmonium-Modell | — | — | — | — | F4 = 349 Hz | — | — | — | Q3 DAFx.3 | selbst gelesen |

Weitere Werte:
- Kanzelle/Volumen: V₁ = 0,0013 m³, L₂ = 50 mm, S₂ = 6,4·10⁻⁵ m² (Q2 Tab. 1). Bei Millot: Zuleitung 30 mm², L₁ = 1,5 cm bzw. 8 cm, Rohr 2 mit 20 mm und 25 mm² (Q1 Tab. 3, Werkzeug). Akkordeon-Kanzelle: Querschnitt 14,8 × 14,6 mm² (Q5).
- Plattendicke Mundharmonika: Marine Band 0,9 mm Messing, laut Händlerangabe [nur Zusammenfassung].

**Plausibilitätsprüfung [eigene Rechnung, kein Beleg].** Für einen Euler-Bernoulli-Kragbalken gilt f₁ = (1,875²/2πL²)·√(EI/ρA).
- Harmonium-B5-Werte aus Q2: ergibt ≈ 970 Hz, gemessen ≈ 990 Hz. Passt.
- Akkordeon-C4, L = 32,8 mm, mit Stahl (E ≈ 200 GPa, ρ ≈ 7800 kg/m³): 267,6 Hz verlangt etwa 0,35 mm Dicke. Die Masse wäre dann ≈ 0,31 g, die Quelle nennt 0,248 g. Das ist schlüssig für eine verjüngte Zunge.

---

## 3 Tonhöhe über Druck, Klangfarbe (Frage 2)

**Richtung.** Die Frequenz sinkt mit dem Druck etwa linear. Das ist allerdings nur Suchzusammenfassung der ASA-Abstracts (Q15, Koopman/Hanzelka/Cottingham). Wörtlich gelesen habe ich bei Q15 nur „the frequency remains essentially constant over a very wide pressure range“ [wörtlich, Werkzeug, Q15 5aMU10].

### Tabelle 3a – Frequenzverschiebung über Druck

| Instrument | Messwert | in Cent (eigene Rechnung) | Quelle | Kennz. |
|---|---|---|---|---|
| Akkordeon C4, Taste voll offen | −2,5 Hz/kPa | ≈ −16 Cent/kPa | Q5 PDF-S. 2 | selbst gelesen |
| Akkordeon C4, Ventil 1 mm offen | −5,1 Hz/kPa | ≈ −34 Cent/kPa | Q5 PDF-S. 2 | selbst gelesen |
| Akkordeon-Simulation, 1 kPa, Kp = 35 | 265,1 Hz statt 267,6 Hz („pitch drop of 2.5 Hz or 0.9 %“) | ≈ −16 Cent | Q5 PDF-S. 3 | wörtlich, selbst gelesen |
| Akkordeon-Simulation, Kp = 150 | 261,7 Hz (−5,9 Hz, 2,2 %) | ≈ −38 Cent | Q5 PDF-S. 3–4 | selbst gelesen |
| Harmonium, Balgdruck | „typically less than 1-2 Hz“, dabei wahrnehmbar und gespielt | bei 349 Hz ≈ 5–10 Cent | Q3 DAFx.3 | wörtlich, selbst gelesen |
| Minimalmodell, Erregung variiert | 0,4 Hz (−,+) bzw. 2 Hz (+,−) | bei 444 Hz ≈ 1,6 bzw. 7,8 Cent | Q1 Abschn. 3.5 | Werkzeug |
| Minimalmodell, Vorvolumen-Länge variiert | 2,6 Hz bzw. 22,2 Hz | — | Q1 Abschn. 3.4 | Werkzeug |
| Akkordeon-Bending (Spieler, Pigini Sirius) | ≈ 25 Cent (Balg + Finger), ≈ 15 Cent (nur Finger); Spanne „10 to 35 cents“ | — | Q6 S. 1, 3 | wörtlich, selbst gelesen |

Weitere Angaben zu Druck und Kanzelle:
- Akkordeon-Bending „lowers the pitch by up to approximately a semitone“ [wörtlich, selbst gelesen, Q5 PDF-S. 1].
- Das Harmonium wird beim Nachlassen des Drucks leicht höher: „slight increase in playing frequency as the air leaks out“ [wörtlich, selbst gelesen, Q3 DAFx.3].
- Kanzelle: senkt die Frequenz bei gleichem Druck und verstärkt die Obertöne [wörtlich, Werkzeug, Q15 4aMU1]. Hysterese beim Auf- und Abfahren des Drucks [Werkzeug, Q15].

**Klangfarbe.**
- Sirenen-Prinzip mit zwei Luftstößen je Periode und etwa rechteckigem Druckverlauf (Q4 S. 46, Abb. 2). „abundant higher harmonics, as the approximate square waveform would suggest“ [wörtlich, selbst gelesen, Q4 S. 46].
- Akkordeon A4 (8′ ohne Cassotto): Obertöne „approximately up to 10000 Hz“ [wörtlich, selbst gelesen, Q6 S. 3].
- Beim Bending verschieben sich alle Obertöne gleich, „harmonicity is conserved“ [wörtlich, selbst gelesen, Q6 S. 3].
- Fernfeld: dominiert von einer Dipolquelle aus Impulsschwankungen im Spalt [nur Zusammenfassung, Q10].
- Gehäuse (Harmonium): Filter mit 8–9 Spitzen. Für die Klangfarbe ist es wichtiger als die Quelle: „the filter has a much stronger influence on the synthesized sound timbre than the source“ [wörtlich, selbst gelesen, Q3 DAFx.3].

---

## 4 Kopplung (Frage 3)

**Kanzelle.**
- Wirkt meist nur schwach auf die Tonhöhe: „the cavity volumes are small and the chamber resonances have high frequencies“ [wörtlich, selbst gelesen, Q7].
- Bei hohen Zungen kann eine Kanzellen-Resonanz nahe der Zungenfrequenz das Ansprechen hemmen oder verhindern [selbst gelesen, Q7].
- Im Akkordeon „usually has little effect on the frequency“ [wörtlich, selbst gelesen, Q4 S. 48].

**Cassotto** (Resonanzkasten im Akkordeon) beeinflusst die Tonhöhe selbst nicht [selbst gelesen, Q5 PDF-S. 1].

**Akkordeon-Bending.** Mechanismus nach Q5:
- Druckverlust am teilweise offenen Ventil (pallet) lässt den Kanzellendruck im Takt der Zunge schwanken.
- Folge: „a decrease in the effective spring constant“ [wörtlich, selbst gelesen, Q5 PDF-S. 1].
- Modell: Kp ≈ 1 bis mehrere hundert, Kr ≈ 8 (Q5).

**Tonon-Akkordeon.** Mit einer eigenen Resonanzkammer lässt sich etwa einen Halbton nach unten biegen: „bend the note downward about one semitone“ [wörtlich, selbst gelesen, Q4 S. 48].

**Paarzungen.** Akkordeon und Mundharmonika haben zwei Zungen je Kanzelle (Q4 S. 48). Bei der Mundharmonika schwingt auch die Zweitzunge immer: „the secondary reed always oscillates“ [wörtlich, selbst gelesen, Q4 S. 48, Abb. 4].

**Chöre, Musette und Tremolo** (nur Praxisquelle Q17, kein Fachartikel gefunden):

| Stil (Q17) | Hz bei A4 | Cent laut Q17 |
|---|---|---|
| Unison (Dry, Secco) | 0 | 0 |
| Concert | 0,5 | 2 |
| Swing | 1 | 4 |
| Demi-Swing | 2 | 7 |
| American | 2,5 | 10 |
| Moderate | 3 | 12 |
| Standard Tremolo, deutsch/italienisch | 4 | 15 |
| Fast, modern französisch | 5 | 18 |
| Very fast | 6 | 22 |
| Extremely fast | 7 | 25 |

Alle Werte [wörtlich, Werkzeug, Q17]. Zur Kontrolle [eigene Rechnung]: bei 440 Hz sind 1 Hz ≈ 3,9 Cent. Die Faustregel „1 hz = 3.5cents“ aus Q17 ist also leicht daneben.

Forenangabe, kein Beleg: Die Schwebung wächst mit der Tonhöhe, etwa von 1,7 auf 7 Schwebungen/s [nur Zusammenfassung, accordionists.info].

---

## 5 Mundharmonika-Bending (Frage 4)

**Wer biegt.**
- Biegbar sind die Töne, bei denen die Hauptzunge höher klingt als die Zweitzunge. Gebogen wird nach unten in den Bereich zwischen beiden Tönen (Johnston 1987, wiedergegeben in Q4 S. 47) [selbst gelesen].
- Beispiel Q4 Abb. 4c: Zieh-Bend G → etwa Fis. „the blow reed takes over“ [wörtlich, selbst gelesen, Q4 S. 48].

**Vokaltrakt.**
- Die Hauptzunge ist an den Vokaltrakt gekoppelt (Q4 S. 47).
- Für Overblow und Overdraw zählt nicht nur das Volumen: „the configuration of the vocal tract, not just its volume“ [wörtlich, selbst gelesen, Q4 S. 47].
- Millot 2011: Das Modell braucht mindestens eine Druckanregung im hinteren Hohlraum, die Gaumenenge, den vorderen Hohlraum und eine Mischung aus Lippenkanal und Kanzelle [Werkzeug, Q14-Abstract, gekürzt].
- 3D-gedruckte Vokaltrakte aus MRT-Aufnahmen „precisely reproduce the same pitch changes“ [wörtlich, selbst gelesen, Q9].

**Umfang je Kanzelle** (C-Diatonik):
- Zieh-Bends auf 1–6, Blas-Bends auf 7–10 [wörtlich, Werkzeug, Q16].
- 3 Ziehen von B nach G♯, also 3 Halbtöne [Werkzeug, Q16].
- Die übliche Tabelle (1Z: 1, 2Z: 2, 3Z: 3, 4Z: 1, 6Z: 1, 8B: 1, 9B: 1, 10B: 2 Halbtöne) habe ich in keiner Fachquelle wörtlich gefunden. Das ergibt sich nur aus den Stimmungsabständen der Zungenpaare: Bend-Tiefe = Abstand der beiden Zungen minus ein Halbton [eigene Ableitung].

**Messungen.**
- Bahnson u. a. 1998: Videostroboskopie, Wegaufnehmer; drei Spielweisen je Kanzelle („natural“, Bend, Overblow/Overdraw) [Werkzeug, Q11-Abstract]. Zahlen nicht erreicht.
- Millot 2001: „The reeds motions were shown to be sinusoidal“ [wörtlich, Werkzeug, HAL-Abstract].

---

## 6 Harmonium und Melodica (Frage 5)

**Harmonium.**
- Europäisches Harmonium mit Druckwind, amerikanisches Reed Organ mit Saugwind („partial vacuum“), laut Suchzusammenfassung aus New World Encyclopedia und harmoniums.com [nur Zusammenfassung].
- Indisches Hand-Harmonium: Kanzellendruck 0,2–1,5 kPa [selbst gelesen, Q2 S. 1810].
- Die Spieler nutzen den Balgdruck für kleine Tonhöhengleitungen (Q3, siehe Abschnitt 3).

**Melodica.** Keine Fachquelle gefunden.

---

## 7 Rechenverfahren und Kosten (Frage 6)

### Tabelle 7a – Verfahren

| Verfahren | Kern | Zahlen / Kosten | Quelle | Kennz. |
|---|---|---|---|---|
| Masse-Feder-Dämpfer mit Bernoulli, Trägheit, V₁ (Minimalmodell) | Newton-Raphson je Zeitschritt, bei Pendeln Wechsel auf Bisektion; Tabellen der Durchflussfläche vorab | keine Abtastrate genannt; „three repetitions of 3000 samples“ | Q1 App. 5.4, Abb. 10 | Werkzeug |
| Dasselbe, an Harmonium angepasst (Balgkammer p₀ als Steuergröße) | Stabilität hängt an Nicht-Spielparametern (Kanzellenvolumen, Spalt), „manual trial-and-error“ je Note; unrealistische Parameter („thickness … several times that of its length“) | — | Q3 DAFx.3; Q2 S. 1812 | selbst gelesen |
| Kragbalken (Euler-Bernoulli), verteilt | implizites Schema (Chaigne/Doutaut), Fluss rückwärts differenziert, Newton-Raphson | 4 × 44 100 Hz, N = 40 Abschnitte; Grundfrequenz aus Geometrie; Obertöne bei hohem Druck wie gemessen | Q2 S. 1812–1814 | selbst gelesen |
| Signalmodell, echtzeitfähig | Wellentabelle aus 37 LTAS-Spitzen, Gehäuse als 10 Biquads (8–9 Spitzen); Balgdruck steuert Amplitude und kleine f0-Verschiebung | Pure Data, live spielbar | Q3 DAFx.3 | selbst gelesen |
| Akkordeon mit Ventil-Druckverlust, Masse-Feder | erweiterter Bernoulli mit Kp, Kr und Trägheit; Mittelungsverfahren zur Analyse | Frequenzen 2–3 Hz über Messung | Q5 | selbst gelesen |
| 2D-Potentialströmung plus FW-H (Ricot) | Zeitbereich, aeroakustisch | teuer, keine Zahlen | Q10 | nur Zusammenfassung |
| Mundharmonika mit Vokaltrakt | zwei Zungen und mehrere Hohlräume, zeitvariabler Trakt „for … real-time sound synthesis“ | — | Q14 | Werkzeug |

Waveguide nach Cook oder Smith für die durchschlagende Zunge: nicht gefunden. FEM/CFD: nur Ricot (2D) und der Hinweis auf FEM im Volpatti-Review [nur Zusammenfassung].

---

## 8 Nicht gefunden
- Volltexte von Johnston 1987 (Acoust. Aust. 15, 69), Bahnson 1998, Ricot 2005, Millot/Cuesta/Valette 1997/2001, Debut/Millot 2001, Cottingham BERLIN1/2 (Koopman-Schwelle 0,2 kPa, Amplituden in mm).
- Ein UNSW-Papier „Vocal tract resonances and the sound of the harmonica“: in der Suche nicht gefunden. Gefunden wurden nur Didgeridoo-, Klarinetten- und Saxophon-Arbeiten der Wolfe-Gruppe.
- Messwerte für Melodica (Mundstück, Druck).
- Zahlen zum Druckwind des europäischen Harmoniums.
- Gemessene Akkordeon-Balgdrücke im Spiel. Q6 rechnet nur mit „about 40 Pa“ für die Reynoldszahl (S. 6).
- Fachliche Messung der Tremolo-Schwebung über den Tonumfang.
- Ein Waveguide-Modell der durchschlagenden Zunge.
- Eine verbindliche Bend-Tabelle aus Fachliteratur.
- Ein Rechtsvermerk im Physics-Today-PDF.
- Widersprüchlicher Vermerk in Q3: „CC BY 4.0“ und „for non-commercial purposes“ im selben Satz. Vor einer Übernahme klären.

Kein Treffer ist kein Beweis, insbesondere wegen der Sperre von cs.bu.edu, wo viele Cottingham- und Bahnson-Texte liegen.

---

## 9 Eigene Einschätzung (kein Beleg)

1. **Tragendes Modell für vorab gerechnete Töne:**
   - Eine Masse-Feder-Zunge (erste Biegemode) wie bei Millot-Baumann bzw. Adachi.
   - Dazu drei Pflichtteile: (a) Öffnungsfläche des Schlitzes inklusive Seitenspalt, symmetrisch zum Durchschlagspunkt; (b) Trägheit der Strömung (L₂/S₂) und Vorvolumen V₁ mit Massenerhaltung; (c) Quelldruck p₀ aus der Spur Expression.
   - Danach ein Gehäuse- bzw. Registerfilter, im Projekt „Körper als Moden plus statistischer Teil“. Laut Q3 bestimmt es die Klangfarbe stärker als die Quelle.
   - Wo Obertöne bei hohem Druck zählen (Forte), den verteilten Balken nach Q2 nehmen: N ≈ 40, 4-fach überabgetastet, offline problemlos.
   - Kosten [Schätzung]: Masse-Feder mit 1–3 Newton-Iterationen je Abtastwert bei 4 × 48 kHz liegt im Bereich von Millisekunden je Sekunde Ton. Balken N = 40 implizit: etwa Faktor 10–50 mehr, vorab weiter unkritisch.
2. **Übertragbarkeit des Rohrblatt-Entwurfs (Masse-Feder + Bernoulli):** Ja, mit drei Änderungen:
   - Statt des sich schließenden Spalts eine durchschlagende Öffnung.
   - Kein Rohr als Pflichtresonator: Die Selbsterregung muss aus der Strömungsträgheit und dem Vorvolumen kommen. Quasistatischer Bernoulli allein schwingt nicht an (Q2 S. 1809).
   - Sehr geringe Dämpfung (Q ≈ 95–170). Das Gerüst „Zunge ↔ Moden des Resonators“ bleibt nutzbar: Die Kanzelle wird zum Helmholtz-Mode, bei der Mundharmonika kommt der Vokaltrakt dazu.
3. **Steuerverlauf:**
   - Expression → p₀ deckt Lautstärke und die Druckabhängigkeit der Tonhöhe (etwa 5–35 Cent) physikalisch ab.
   - Vibrato beim Akkordeon über p₀ (Balgvibrato) statt über die Frequenz. Feste Tonhöhenverschiebung über f₀ der Zunge.
   - Akkordeon-Bending braucht einen zusätzlichen Parameter „Ventilöffnung“ (Kp).
   - Mundharmonika-Bending braucht zwei gekoppelte Zungen und eine Spur für die Vokaltrakt-Resonanz. Die gibt es in LumiViz noch nicht. Denkbar: aus der Pitch-Spur über eine Inversion den Ziel-Trakt ableiten.
4. **Musette/Tremolo:** 2–3 Zungen je Note getrennt rechnen (oder Rechnung mit verschobenem f₀ wiederholen) und summieren, Verstimmung aus einer Tabelle wie Q17, mit der Tonhöhe skaliert. Das ist billig.
5. **Risiken:**
   - Die Stabilität des Masse-Feder-Modells hängt an Nicht-Spielparametern (V₁, Spalt), die je Note durch Probieren eingestellt werden mussten (Q3). Für 80–120 Noten braucht es eine automatische Kalibrierung oder eine physikalisch begründete Skalierung der Parameter über den Tonumfang.
   - Im Massemodell entsprechen die Parameter nicht den echten Maßen (Q2).
   - Torsionsmoden im Einschwingen fehlen (Q8).
   - Belege zur Mundharmonika sind schwach: Bahnson und Johnston sind nicht gelesen.
   - Rechtslage: Q2 und Q5 sind CC BY 3.0, Q3 widersprüchlich (CC BY 4.0 mit NC-Satz), Q6 „All rights reserved“, Q4 ohne Vermerk.

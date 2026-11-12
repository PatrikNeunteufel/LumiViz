# Klangfarbe der durchschlagenden Zunge über Blasdruck und Lautstärke — Recherche in den lokalen Quellen

| | |
|---|---|
| **Typ** | Agentenbericht (unverändert abgelegt) |
| **Datum** | 2026-11-12 |
| **Sitzung** | LumiViz S192, Auftrag ZV12 a (Befund „lauter wird dunkler“ vor 34j-8 untersuchen) |
| **Frage** | Ändert sich die Klangfarbe (Spektrum, Teiltöne, Schwerpunkt, Helligkeit) einer durchschlagenden Zunge mit dem Blasdruck bzw. der Lautstärke, und zwar gemessen? Welche Mechanismen nennen die Quellen? |
| **Quellen** | nur lokal: `…\Visuals_Project\cmake\quellen\zunge\` (28 PDFs). Kein Netz, kein Browser. |
| **Werkzeuge** | `pdftotext -layout` (alle 28 Dateien, Seitenumbrüche erhalten), eigenes Suchskript über die Textebene je PDF-Seite, `quelle_suchen.py` zur Gegenprobe, `pdftoppm` für Seitenbilder (angesehen). |

## Kennzeichen

- **gemessen** — Messwert oder Messbild in der Quelle selbst.
- **gerechnet** — Ergebnis eines Modells / einer Simulation der Quelle.
- **behauptet** — Aussage ohne Messung oder Rechnung in der Quelle (Hörurteil, Praxiswissen, Literaturverweis).
- **(abgelesen)** — Zahl von mir aus einer Abbildung abgelesen (Seitenbild angesehen); Ablesegenauigkeit etwa ±0,5 dB bzw. ±0,1 mm.
- „PDF S. n“ = Seite im PDF; „gedr. S.“ = gedruckte Seitenzahl, wo die Quelle eine trägt.

## Kurzantwort

1. **Ein direkter Beleg fehlt in diesem Bestand.** Keine der 28 Dateien zeigt das **abgestrahlte Spektrum einer freien Zunge ohne Resonator bei zwei verschiedenen Blasdrücken** (oder einen spektralen Schwerpunkt über dem Druck). Ob der Ton bei mehr Druck heller, gleich oder dunkler wird, ist hier **am Schall nicht gemessen**.
2. **Gemessen an der Zungenbewegung:** Mit steigendem Druck wird die Bewegung **weniger sinusförmig, mit mehr höheren Anteilen** (Puranik & Scavone 2023, Harmonium-Zunge B5; Bahnson u. a. 1998, Mundharmonika; Cottingham u. a. 1999, Harmonium-Zunge, dort ab „etwas über normalem Spieldruck“ auch die 2. Biegemode). Das ist die **Bewegung der Zunge, nicht der abgestrahlte Schall.**
3. **Gemessen am Schall, aber vermengt:** Beim Akkordeon-Bend (Ventil halb zu, Balg stärker) wird der Ton **leiser und dunkler**: Grundton etwa −4 dB, Teiltöne 9–20 etwa −10 bis −15 dB (Elejalde-García u. a. 2021, Abb. 4, abgelesen). Das ist aber **keine reine Druckänderung** (Drossel am Ventil, Tonhöhe sinkt mit, der Druck an der Zunge ist nicht gemessen).
4. **Kein Beleg für „lauter wird dunkler“.** Alle Hinweise in diesem Bestand weisen, wo sie überhaupt eine Richtung haben, auf **lauter = obertonreicher** (Zungenbewegung) bzw. **leiser = dunkler** (Bend). Einzige Gegenspur ohne Klangaussage: bei hohem Druck **nimmt die Amplitude wieder ab** und die Ruhelage wandert in den Schlitz (Cottingham u. a. 1999, gemessen).
5. **Mechanismen, die die Quellen nennen:** Sirene / gehackter Volumenstrom (Pulsform), nichtlinearer Durchfluss (Bernoulli) als Quelle der Harmonischen, die Zunge als Filter mit ihren Biegemoden, höhere Biegemoden und Torsion bei hohem Druck bzw. im Einschwingen, Pumpfluss (tief) gegen Strahlfluss (hoch), Abstrahlung ∝ dU/dt, Durchschlagen durch den Rahmen, Kanzelle/Cassotto als Filter. **Keine Quelle verknüpft** Schliessdauer/Tastverhältnis oder Amplitude/Schlitzlänge **ausdrücklich mit der Klangfarbe über dem Druck.**

## Je Quelle

### 1. Puranik & Scavone 2023 — *Clamped bar model for free reeds* (Forum Acusticum 2023, S. 1809–1815)

- **Zunge:** Harmonium-Zunge B5 (990 Hz), Messing, 16 × 2 × 0,4 mm (Tab. 1), in 3D-gedrucktem Schlitz auf einem Windkasten, Laser-Vibrometer nahe der Wurzel, Druck im Windkasten gemessen.
- **Druck:** Mund angeblasen, von kaum hörbar bis „so stark wie möglich“; in einem echten Harmonium 0,2–1,5 kPa (PDF S. 2, gedr. S. 1810).
- **Aussage:** Bei niedrigem Druck (≈ 0,2 kPa) ist die Zungenbewegung weitgehend sinusförmig, bei hohem (1,4–1,6 kPa) treten die höheren Harmonischen deutlich hervor (PDF S. 2, gedr. S. 1810; Spektrogramm Abb. 3, PDF S. 3, gedr. S. 1811). Das Modell (Euler-Bernoulli-Balken) zeigt dasselbe zwischen 250 Pa (Abb. 8) und 1650 Pa (Abb. 9), PDF S. 5–6, gedr. S. 1813–1814. Abb. 10 (PDF S. 7, gedr. S. 1815): Langzeitspektrum der gemessenen Zungengeschwindigkeit bei hohem Druck mit Spitzen beim Grundton und nahe der 3. Eigenfrequenz (≈ 16 kHz), keine sichtbare Spitze bei der 2. (≈ 6 kHz).
- **Zahlen:** Die Spektrogramme haben **keine Farbskala** — Pegel der Teiltöne sind nicht ablesbar. Nur die Richtung ist belegt.
- **Mechanismus (gerechnet):** Der Druck an der Zunge hat durch den nichtlinearen Durchfluss (Bernoulli, Gl. 3) ein harmonisches Spektrum; die Zunge wirkt als Filter mit formantartigen Spitzen bei ihren Eigenfrequenzen (PDF S. 5, gedr. S. 1813).
- **Zitat:** „the higher harmonic components become more prominent at the higher blowing pressures“ (PDF S. 2).
- **Belegkraft:** **gemessen** (Zungengeschwindigkeit, nicht Schall, qualitativ) und **gerechnet**.

### 2. Cottingham, Lilly & Reed 1999 — *The motion of air-driven free reeds* (Forum Acusticum Berlin 1999; keine gedruckten Seitenzahlen in der Datei)

- **Zunge:** Harmonium-Zungen amerikanischer Reed Organs, 100–200 Hz, Zunge ≈ 4 cm; A♯ (118 Hz) und C3.
- **Druck:** 0–30 cm WS (≈ 0–2,9 kPa), normaler Spieldruck ≈ 0,8 kPa.
- **Aussage:** (a) Nach der Schwelle (≈ 0,2 kPa) springt die Amplitude auf mehrere Millimeter, bleibt mit steigendem Druck **nahezu gleich** und **nimmt bei hohem Druck wieder ab**; die Ruhelage wandert 1–2 mm in Strömungsrichtung (PDF S. 1, Bericht über Koopman & Cottingham). (b) Abb. 2 (PDF S. 2), VIT-Sensor in der Mitte der Zunge (≈ 2 cm von der Spitze), **abgelesen:** bei 3–12 cm WS Ausschlag ≈ +1,0…+1,15 / −1,2…−1,45 mm um die Ruhelage, bei 30 cm WS ≈ +0,35 / −1,7 mm, Ruhelage von 0 auf ≈ −0,6 mm; Spitze-Spitze von ≈ 2,4 mm auf ≈ 2,0 mm. (c) Bei Drücken „etwas über normalem Spieldruck“ zeigt das Spektrum der Zungenbewegung Spuren der 2. Biegemode (f₂ = 6,27 f₁), Abb. 4 (PDF S. 2–3). (d) Abb. 6 (PDF S. 4): Geschwindigkeit der A♯-Zunge bei 0,8 kPa sinusförmig, Spitze ≈ ±1100 cm/s (abgelesen); bei 1,2 kPa unregelmäßiger, mit abgeflachten Spitzen ≈ ±1000 cm/s (abgelesen). Die Bildunterschrift von Abb. 6b sagt 0,8 kPa, der Text 1,2 kPa — vermutlich Druckfehler in der Unterschrift.
- **Zum Schall:** Abb. 3 (PDF S. 2) zeigt bei 0,9 kPa (C3): Zunge fast sinusförmig, Schalldruck nahe der Zunge pulsartig (zwei Öffnungen je Periode). **Nur ein Druck** — kein Vergleich der Schallspektren.
- **Zitat:** „the waveform becomes more irregular, and the amplitude is also slightly smaller“ (PDF S. 4).
- **Belegkraft:** **gemessen** (Zungenbewegung).

### 3. Bahnson, Antaki & Beery 1998 — *Acoustical and physical dynamics of the diatonic harmonica* (JASA 103, S. 2134–2144)

- **Instrument:** Hohner-Mundharmonika (Golden Melody, Marine Band), Wegsensoren an Blas- und Ziehzunge.
- **Aussage:** Die Abweichung der Zungenbewegung von der Sinusform war bei **höherem Druck und Durchfluss ausgeprägter** als bei niedrigem — ausdrücklich **nicht in den Abbildungen gezeigt** (PDF S. 7, gedr. S. 2140). Die Ziehzunge trug deutlich die 3. Harmonische. Bei hohem Druck verschob sich die Phase zwischen den Zungen leicht (PDF S. 10, gedr. S. 2143). Die Stimmform des Mundraums bestimmt die Klangfarbe, „Gegenstand weiterer Untersuchung“ (PDF S. 10, gedr. S. 2143).
- **Zahlen:** keine.
- **Zitat:** „more prominent for higher air pressure and flow“ (PDF S. 7).
- **Belegkraft:** **gemessen** (Zungenbewegung, ohne gezeigte Daten).

### 4. Elejalde-García, Llanos-Vázquez u. a. 2021 — *Accordion acoustics: a study on pitch bending* (Acoustics in Practice 2021)

- **Instrument:** Konzertakkordeon Pigini Sirius, rechter Diskant, 8′ außerhalb des Cassotto, Töne A♯2–A3 (117–221 Hz), Mikrofon 50 cm, je 15 Wiederholungen. Ausgangston mezzoforte ≈ 70 dBA.
- **Was variiert wird:** Bend „Balg und Finger“ (Ventil halb offen, mehr Balgdruck) und „nur Finger“ (Ventil langsam zu, Balg gleich). **Der Druck an der Zunge ist nicht gemessen.**
- **Aussage (Abb. 4, PDF S. 5, gedr. S. 5, abgelesen, Mittel über 12 Töne):** Ausgangston mf: Teiltöne fallen langsam, h2 ≈ −5, h5 ≈ −2,5, h10 ≈ −8,5, h20 ≈ −10,5 dB unter h1 (≈ 71 dB). Nach dem Bend „Balg und Finger“: h1 ≈ −4 dB, h2–h8 ≈ −4 … −10 dB, h9–h20 ≈ −10 … −15 dB gegenüber vorher; „nur Finger“ ähnlich, bis ≈ −15 dB. Tab. 2 (PDF S. 4, gedr. S. 4): mittlerer Pegelverlust über 20 Teiltöne 7,9 dB bzw. 11,1 dB. Gerade Teiltöne 2/4/6 fallen stärker als 1/3/5; ab h7 fallen die geraden weniger als die ungeraden (PDF S. 5). Der leisere, gebogene Ton ist also **dunkler**.
- **Mechanismus (behauptet):** Die Zunge ist durchflussgesteuert; schließt das Ventil, kommt weniger Luft an, Energie, Frequenz und Amplitude sinken; der Balg gleicht teilweise aus (PDF S. 5). Viskosität im Spalt (Re ≈ 133, Abschätzung PDF S. 6).
- **Zitat:** „The different harmonics experience a different change in sound pressure levels“ (PDF S. 5).
- **Belegkraft:** **gemessen** (abgestrahlter Schall), aber **vermengt** mit Drossel und Tonhöhe — kein Beleg für eine reine Druckabhängigkeit.

### 5. Llanos-Vázquez, Elejalde-García & Macho-Stadler 2008 — *Controllable pitch-bending effects in the accordion playing* (Acoustics ’08 Paris, S. 8565–8570)

- Gleiches Instrument, 16′ und 8′, E1–E3, Ausgang mf ≈ 70 dBA, erste 10 Teiltöne.
- **Aussage:** Abb. 7 (PDF S. 5, gedr. S. 8569, abgelesen), F2 8′ ohne Cassotto: vorher h1 ≈ 65, h2 ≈ 62, h3 ≈ 69, h4 ≈ 56, h5 ≈ 66, h6 ≈ 59, h7 ≈ 65, h8 ≈ 48, h9 ≈ 60, h10 ≈ 58 dB; nach dem Bend h1 ≈ 60, h2 ≈ 49, h3 ≈ 65, h4 ≈ 43,5, h5 ≈ 61, h6 ≈ 37, h7 ≈ 55,5, h8 ≈ 42, h9 ≈ 44,5, h10 ≈ 48 dB. Abb. 8–11: Verlust in Prozent des dB-Werts steigt mit der Teiltonnummer (≈ 10 % bei h1 bis ≈ 25–35 % bei h9–h10, 8′). Der Spieler drückte stärker auf den Balg, der Schallpegel sank trotzdem (PDF S. 5). Cassotto dämpft oberhalb 6 kHz um „etwa 30 % im Schalldruckpegel“ (PDF S. 3, gedr. S. 8567).
- **Mechanismus (behauptet):** Bei fast geschlossenem Ventil wechselt die Kammer von offen-offen zu offen-geschlossen, daher fallen die geraden Teiltöne stärker (PDF S. 5).
- **Zitat:** „The even harmonics suffer a greater decrease of the sound pressure level“ (PDF S. 5).
- **Belegkraft:** **gemessen**, **vermengt** wie Nr. 4.

### 6. Llanos-Vázquez u. a. 2014 — *Acústica de la lengüeta libre del acordeón* (Tecniacústica 2014) und Llanos-Vázquez u. a. 2004 (Guimarães)

- 2014: Einschwingen bei **piano ≈ 55 dBA und mezzoforte ≈ 70 dBA** gemessen (PDF S. 2–3) — aber nur **Einschwingzeiten** (Tab. I, PDF S. 3), **keine Spektren piano gegen mezzoforte**. Bend: nur der Mittelwert über 20 Teiltöne (Abb. 4, PDF S. 5), kein Verlauf je Teilton. Spektraler Schwerpunkt nur für **belastete Zungen** (Masse an der Spitze), nicht über dem Druck (PDF S. 7).
- 2004: Bend-Beschreibung, keine Klangfarbe über dem Druck.
- **Belegkraft:** zur Frage **schweigend**.

### 7. Adachi u. a. 2026 — *Aerodynamic mechanism of accordion pitch bending* (Forum Acusticum 2026, nicht begutachtet; keine gedruckten Seitenzahlen)

- **Zunge:** Akkordeon-Zunge 267,6 Hz, 32,8 × 3,45 mm, Spalt 0,14 mm, Ruheauslenkung 0,3 mm (Tab. 1, PDF S. 4).
- **Aussage (gerechnet):** Abb. 7 (PDF S. 5, abgelesen), Kp = 35: Amplitude ≈ 0,4 mm bei 0,2 kPa, ≈ 1,7 mm bei 1,0 kPa, ≈ 3,1 mm bei 2,5 kPa — **steigt monoton**, ohne Sättigung. Abb. 6 (PDF S. 4) bei 1 kPa: der Volumenstrom fließt nur, solange die Zunge über dem Rahmen steht (Öffnungsfläche S(x) = (L+W)·max(0,x)), also etwa die halbe Periode; der Schalldruck hat scharfe Spitzen beim Schließen und Öffnen.
- **Mechanismus:** Abstrahlung p_out = ρ·ΔL/S_r · dU/dt (Gl. 7, PDF S. 4). **Spektren über dem Druck zeigt die Arbeit nicht.**
- **Zitat:** „When p0 increases, the amplitude monotonically increases“ (PDF S. 4).
- **Belegkraft:** **gerechnet**; zur Klangfarbe **schweigend**.

### 8. Millot & Baumann 2007 — *A proposal for a minimal model of free reeds* (Acta Acustica 93, S. 122–144; hier arXiv-Fassung, Seiten 1–41)

- **Aussage (gerechnet):** Die Spielfrequenz hängt kaum von der Anregung ab (0,4 Hz bzw. 2 Hz über v₀ = 1…10 m/s, Abb. 22, PDF S. 25), die „Dynamik“ (Betrag des Überdrucks p₂) wächst mit der Anregung (Abstract PDF S. 1; Schluss PDF S. 32). Spektren (Abb. 16–17, PDF S. 20–21) nur bei **einer** Anregung.
- **Mechanismus (vermutet von den Autoren):** Der Pumpfluss (von der Zungenbewegung verdrängte Luft, sinusförmig) speist eher den tiefen Teil des Klangs, der Strahl durch den Spalt wegen seiner starken Nichtlinearität eher den hohen (PDF S. 20–21).
- **Zitat:** „the dynamic of the sound increases with the excitation level“ (PDF S. 1).
- **Belegkraft:** **gerechnet**; Klangfarbe über der Anregung **nicht untersucht**.

### 9. Tarnopolsky, Fletcher & Lai 2000 — *Oscillating reed valves — an experimental study* (JASA 108, S. 400–406)

- **Achtung, anderer Ventiltyp:** ein nach außen schwingendes Messingblatt 28 × 52 mm (0,15 / 0,33 mm), 100–300 Hz, über einem 4 mm dicken Rahmen, freier Spalt 0,5 mm — **kein durchschlagendes Zungenpaar wie im Akkordeon**, aber „frei“ (nicht schlagend).
- **Aussage (gemessen):** Amplitude steigt nach der Schwelle fast linear mit dem Druck (Abb. 4a, PDF S. 5, gedr. S. 404, abgelesen: V = 0,6 L von ≈ 1,5 mm bei 200 Pa auf ≈ 5,3 mm bei 500 Pa), mit Hysterese. Im Grenzzyklus schließt das Blatt **fast die halbe Periode in die Rahmenöffnung**; über ≈ 5 mm tritt es hinten aus dem Rahmen und lässt einen zweiten Strom zu (PDF S. 4, gedr. S. 403). Die Frequenz hängt kaum vom Druck ab (PDF S. 6, gedr. S. 405). Die Bewegung ist sehr nahe sinusförmig (PDF S. 5).
- **Spektrum:** ausdrücklich **nicht berichtet**; es ließe sich aus dem Gesamtstrom U(t) + 0,4·W·L·dx/dt ableiten (PDF S. 6, gedr. S. 405).
- **Zitat:** „details of the spectral envelope depend upon rather fine aspects of the valve construction“ (PDF S. 6).
- **Belegkraft:** **gemessen** (Amplitude, Druck), zur Klangfarbe **schweigend**.

### 10. Puranik & Scavone 2023 — *Signal model of an Indian harmonium* (DAFx 23, Paper 47)

- **Aussage (behauptet / Modellwahl):** Der Balgdruck moduliert vor allem die Amplitude; die Grundfrequenz ändert sich um weniger als 1–2 Hz und steigt leicht, wenn der Druck beim Auslaufen sinkt (§ 4.3, PDF S. 3). Die Synthese nimmt **feste Teiltongewichte** aus einem Langzeitspektrum und regelt mit dem Druck nur die Hüllkurve.
- **Zitat:** „The primary role of the bellows pressure is to modulate the amplitude“ (PDF S. 3).
- **Belegkraft:** **behauptet**; zur Klangfarbe über dem Druck keine Messung.

### 11. Fouché 1928 — *Étude expérimentale des tuyaux à anche* (Ann. Fac. Sci. Toulouse; gedr. Seite = PDF-Seite + 235, geprüft an PDF S. 3 = 238 und PDF S. 45 = 280)

- **Anordnung:** Harmonium-Zungen (+E, „parle sans résonateur“, PDF S. 4, gedr. S. 239) **mit angekoppeltem Rohr (Bourdon) veränderlicher Länge**; Druck in mm WS. Meist ist die Rohrlänge die Variable, nicht der Druck.
- **Aussagen (Hörurteil):** Die freie Zunge ohne Resonator klingt „criard“ (schrill, grell) (PDF S. 22, gedr. S. 257; PDF S. 73, gedr. S. 308). Kleine Zungenamplitude: Ton „faible, voilé“ (schwach, verschleiert); wächst die Amplitude und dringt die Zunge weiter in den Rahmen, wird der Ton „intense“, dann „criard“ (PDF S. 71, gedr. S. 306). Bei 36 cm Rohr und steigendem Druck springt der Ton bei ≈ 130 mm WS von 175 auf 192 Hz, die Klangfarbe von „sourd“ (dumpf) zu „éclatant“ (strahlend) (PDF S. 22, gedr. S. 257) — ein **Regimewechsel des Rohrs**, nicht der Zunge allein. Die Amplitude der Zunge wächst mit dem Druck (PDF S. 21, gedr. S. 256). Ohne Resonator ist die Frequenz der freien Zunge vom Druck unabhängig (PDF S. 31, gedr. S. 266).
- **Zitat:** «Le son est fort, le timbre criard.» (PDF S. 20).
- **Belegkraft:** **gemessen** (Frequenz, Amplitude), Klangfarbe **behauptet** (Hörurteil), und an **Zunge mit Rohr** — auf das Akkordeon/Harmonium ohne Rohr nur mit Vorsicht übertragbar. Richtung, wo vorhanden: **größere Amplitude / mehr Druck → greller**.

### 12. Tonon 2005 — *Reed cavity design and resonance* (PICA 2)

- **Aussage (behauptet):** Die Teiltöne entstehen, weil die Zunge den Luftstrom periodisch zerhackt; die irrige Meinung, sie kämen von höheren Moden der Zunge, wird ausdrücklich zurückgewiesen (Fußnote 10). Abweichung von der Harmonizität durch weitere Zungenmoden sei sehr klein und trete nur bei sehr hohem Blasdruck auf (PDF S. 5, mit Verweis auf Cottingham, Lilly & Reed 1999 = Nr. 2). Kammerresonanzen können einzelne Teiltöne stärken oder schwächen, hörbar vor allem unter dem 4. Teilton (PDF S. 5).
- **Zitat:** „occurring only at very high blowing pressures“ (PDF S. 5).
- **Belegkraft:** **behauptet**.

### 13. Cottingham 2011 — *Acoustics of free-reed instruments* (Physics Today 64(3), S. 44–48) und Braasch & Cottingham 2023 (Acoustics Today 19(4), S. 14–22)

- **Aussage:** Gemessene Druckwelle nahe einer Akkordeon-Zunge ist fast rechteckig wie bei einer Sirene, das Spektrum reich an hohen Harmonischen; berechneter Volumenstrom (Millot & Baumann) mit zwei Stößen je Periode, die kleineren beim Durchschlagen unter die Platte (Cottingham 2011 Abb. 2, PDF S. 3, gedr. S. 46; Braasch & Cottingham Abb. 4, PDF S. 3, gedr. S. 16). Amplitude der Spitze typischerweise **15 % der Zungenlänge** (Cottingham 2011 PDF S. 2, gedr. S. 45; Braasch & Cottingham PDF S. 3, gedr. S. 16). Kammern beeinflussen die Klangfarbe. Braasch & Cottingham: nur freie Zungen halten die Tonhöhe bei veränderlichem Winddruck (PDF S. 6, gedr. S. 19); beim Einschwingen erscheinen die höheren Harmonischen verzögert, nach Frequenz aufsteigend (Abb. 5, PDF S. 4, gedr. S. 17).
- **Zur Frage:** **keine Aussage** zur Klangfarbe über dem Blasdruck. Die Klangänderung beim Bawu (Abb. 3, gedr. S. 47) kommt von der Rohrlänge.
- **Zitat:** „the sound spectrum has abundant higher harmonics“ (Cottingham 2011, PDF S. 3).
- **Belegkraft:** **gemessen** (Wellenform, ein Druck), sonst Überblick.

### 14. Cottingham 2013 (Folien CCRMA), Cottingham & Wolff 2015, Cottingham 2017 (Abstracts)

- 2013: Geschwindigkeitsspektren entlang der Zunge bei **0,6 kPa** (Folie 9, PDF S. 9); erste drei Biegemoden und erste Torsion sind am angeblasenen Labor-Rohrblatt oft nachweisbar (PDF S. 8). Spektrogramm mit steigendem Druck zeigt Moden 1–3, aber an einer **Zunge mit nicht passendem Rohr** (PDF S. 16).
- 2015/2017: 2. Biegemode und 1. Torsion treten vor allem **im Einschwingen** auf (Abstracts, PDF S. 1). Kammerresonanzen beeinflussen die Klangfarbe, wirken auf die Anregung meist wenig (2017).
- **Belegkraft:** **gemessen**, aber nicht über dem Druck am Schall.

### 15. Goldbrunner — *Harmoniumreparatur* (Anleitung, deutsch)

- **Aussage (behauptet):** Am Harmonium sei „der Wechsel der Klangfarbe mit der Lautstärke“ zu entdecken — **ohne Richtung** (PDF S. 42). Intonation: eine zu hoch stehende Zunge spricht träge an und ist zu laut, eine zu tiefe spricht schlecht an und ist zu leise; Abwägung zwischen „hartem und rundem Klang“ (PDF S. 36, nach Visser).
- **Zitat:** „Der Wechsel der Klangfarbe mit der Lautstärke“ (PDF S. 42).
- **Belegkraft:** **behauptet** (Praxis).

### 16. Fletcher 2001 — Überblick (UNSW) — nur Analogie

- Für die **Klarinette** (schlagendes Blatt mit Rohr): leise gespielt wird die Strömungsform einfacher; sehr kräftig gespielt schließt das Blatt einmal je Periode und erzeugt starke Obertöne (PDF S. 3). **Nicht für die freie Zunge belegt.**
- **Zitat:** „very vigorous playing, in which the reed closes once in each cycle“ (PDF S. 3).
- **Belegkraft:** **behauptet/gerechnet** für ein anderes System.

### Ohne Aussage zur Frage (durchsucht)

Cottingham 1998 (ICA, gedr. S. 761–762: Frequenz über Druck, Phase Zunge/Schall), Cottingham, Reed & Busha 1999 (Frequenz sinkt fast linear mit dem Druck, bei sehr hohem Druck steigt sie; Dämpfung/Anfachung über Druck, Maximum bei 1,0–1,5 kPa), Fletcher 1993, Tarnopolsky u. a. 2001 (Wirbel), Hergert 2023 (Schwebung; definiert Helligkeit als spektralen Schwerpunkt, aber nicht über dem Druck), Pomberger u. a. 2021 (Werkstoff des Stimmstocks, Pegel der Teiltöne, kein Druckvergleich gefunden), Lu 2014 (2-Ziehen gegen 3-Blasen), Granzow u. a. 2015, Guimarães 2004 (Bend).

## Gesamtlage

**Gemessen belegt**

- Die **Zungenbewegung** wird mit steigendem Druck reicher an höheren Anteilen (Puranik & Scavone 2023, Bahnson u. a. 1998, Cottingham u. a. 1999; bei Letzteren die 2. Biegemode erst über dem normalen Spieldruck).
- Die **Amplitude** über dem Druck: an Harmonium-Zungen nach der Schwelle **fast konstant, bei hohem Druck abnehmend**, Ruhelage wandert in Strömungsrichtung (Cottingham u. a. 1999); am großen Versuchsventil dagegen **fast linear steigend** (Tarnopolsky u. a. 2000). Das widerspricht sich nicht zwingend (andere Bauform, anderer Bereich), ist aber für eine Vorgabe nicht dasselbe.
- **Leiser durch den Bend** (Drossel am Ventil) → die oberen Teiltöne fallen 2–3-mal stärker als der Grundton, der Ton wird dunkler (Elejalde-García u. a. 2021; Llanos-Vázquez u. a. 2008). Vermengt mit Drossel und Tonhöhe.
- Die Tonhöhe der freien Zunge hängt nur schwach vom Druck ab (mehrere Quellen).

**Nur Modell**

- Amplitude steigt monoton mit dem Druck (Adachi u. a. 2026), Betrag des Drucksignals steigt mit der Anregung (Millot & Baumann 2007), Zungenbewegung im Balkenmodell bei 1650 Pa obertonreicher als bei 250 Pa (Puranik & Scavone 2023). **Kein Modell der Sammlung rechnet das abgestrahlte Spektrum über dem Druck aus.**

**Nur behauptet**

- Klangfarbe wechselt mit der Lautstärke (Goldbrunner, ohne Richtung); größere Amplitude / mehr Druck → greller (Fouché, Hörurteil, mit Rohr); höhere Zungenmoden nur bei sehr hohem Druck (Tonon); Balgdruck regelt vor allem die Amplitude (Puranik & Scavone, DAFx).

**Widersprüchlich oder offen**

- Amplitude über dem Druck (Sättigung und Abnahme gegen monotonen Anstieg), siehe oben.
- Ob die zusätzlichen Anteile der **Zungenbewegung** im **Schall** ankommen: Tonon bestreitet, dass Zungenmoden die Teiltöne machen (der zerhackte Strom macht sie), Puranik & Scavone sehen die Zunge als Filter des Drucksignals. Für die Frage nach der Klangfarbe des Schalls ist das ungeklärt.

**Ergebnis für ZV12 a:** In diesem Bestand gibt es **keinen Beleg, dass eine durchschlagende Zunge lauter dunkler klingt**. Wo eine Richtung erkennbar ist, geht sie zu **lauter = obertonreicher** (Zungenbewegung, gemessen) bzw. **leiser = dunkler** (Bend, gemessen, vermengt). Ein gemessenes Spektrum des abgestrahlten Schalls bei zwei Drücken fehlt; es ist der fehlende Beleg.

## Genannte Mechanismen und wer sie nennt

| Mechanismus | Quelle (Belegkraft) | Bezug zur Klangfarbe über dem Druck |
|---|---|---|
| Sirene: Strom wird zerhackt, Druckwelle fast rechteckig, viele Harmonische | Cottingham 2011, Braasch & Cottingham 2023 (gemessen, ein Druck); Tonon 2005 (behauptet) | nicht über dem Druck |
| Nichtlinearer Durchfluss (Bernoulli) erzeugt die Harmonischen des Drucks an der Zunge; die Zunge filtert mit ihren Eigenmoden | Puranik & Scavone 2023 (gerechnet) | ja: mehr Druck → mehr hohe Anteile in der Bewegung |
| Höhere Biegemoden (f₂ = 6,27 f₁), Torsion | Cottingham u. a. 1999 (gemessen, über normalem Spieldruck); Cottingham 2013/2015/2017 (gemessen, v. a. Einschwingen); Tonon (behauptet, nur bei sehr hohem Druck) | ja, an der Bewegung |
| Pumpfluss (tief) gegen Strahl durch den Spalt (hoch) | Millot & Baumann 2007 (vermutet); Tarnopolsky u. a. 2000 (Gesamtstrom inkl. 0,4·W·L·dx/dt) | nicht über dem Druck |
| Abstrahlung ∝ dU/dt, scharfe Spitzen beim Öffnen und Schließen | Adachi u. a. 2026 Gl. 7 (gerechnet) | nicht über dem Druck |
| Durchschlagen durch den Rahmen, kleinere Stöße unter der Platte, Austritt hinter dem Rahmen | Cottingham 2011 Abb. 2; Tarnopolsky u. a. 2000 (gemessen, > 5 mm); Fouché (Hörurteil: tieferes Eindringen → intensiver, greller) | nur bei Fouché, mit Rohr |
| Schliessdauer / Tastverhältnis | Tarnopolsky u. a. 2000: im Grenzzyklus fast halbe Periode im Rahmen (gemessen); Adachi Abb. 6: Strom nur bei x > 0 (gerechnet) | **keine Quelle** verknüpft es mit der Klangfarbe |
| Amplitude gegen Schlitzlänge / Ruheöffnung | Cottingham 2011 (15 % der Zungenlänge); Cottingham u. a. 1999 (Ruhelage wandert 1–2 mm); Goldbrunner (Aufbiegung ↔ Lautstärke, Ansprache) | **keine Quelle** mit Klangfarbe über dem Druck |
| Kanzelle / Cassotto / Kammerresonanz | Llanos-Vázquez 2008 (Cassotto dämpft über 6 kHz; Kammer offen-offen → offen-geschlossen beim Bend, gerade Teiltöne fallen), Tonon 2005, Cottingham 2017 | Filter, unabhängig vom Druck; beim Bend mit der Ventilstellung |

## Reichweite

- **Durchsucht:** alle 28 PDFs im Ordner, jede vollständig per `pdftotext -layout` in Text gewandelt; jede hatte eine Textebene (keine OCR nötig). Die drei Hauptquellen (Cottingham 2011, Braasch & Cottingham 2023, Tarnopolsky u. a. 2000) ganz gelesen; Puranik & Scavone (FA 2023), Cottingham u. a. 1999 (beide Berlin), Cottingham 1998, Llanos-Vázquez 2008 und 2014, Adachi 2026 ganz gelesen; die übrigen gezielt nach Stichworten mit Kontext.
- **Begriffe:** spectr, timbre, bright, centroid, tone quality, harmonic content, overtone, partial, harmonic, waveform, amplitude, blowing pressure, bellows pressure, loud, forte, dynamic, crescendo, intensity, excitation; französisch timbre, harmoniques, intensité, criard, voilé, sourd, éclatant, amplitude, sans résonateur; spanisch espectr, armónic, presión, timbre, centroide; deutsch Klangfarbe, Klang, Lautstärke, Forte.
- **Seitenbilder angesehen:** Puranik & Scavone FA 2023 S. 3, 6; Cottingham u. a. 1999 (Berlin 1) S. 2–4; Llanos-Vázquez 2008 S. 5; Llanos-Vázquez 2014 S. 5; Elejalde-García 2021 S. 4–5; Adachi 2026 S. 4–5; Tarnopolsky u. a. 2000 S. 5.
- **Nicht im Ordner, aber oft zitiert und für die Frage wahrscheinlich wichtig:** Ricot, Caussé & Misdariis 2005 (JASA 117, Akkordeon-Zunge ohne akustische Kopplung, Schallerzeugung), St. Hilaire, Wilson & Beavers 1971 (Harmonium-Zunge), Koopman & Cottingham 1997 (Amplitude über Druck), Braasch & Ahrens 2000 (Einschwingen), Johnston 1987, Millot u. a. 2001, Llanos-Vázquez u. a. 2014 in Acta Acustica 100 (Einschwingen piano/mf). Ob diese Spektren über dem Druck enthalten, ist von hier aus nicht prüfbar.
- **Grenzen:** Kein Treffer ist kein Beweis — die Textebene kann Lesefehler enthalten, und Abbildungen ohne Bildunterschrift zum Druck werden über Stichworte nicht gefunden. Abgelesene Zahlen sind Bildwerte mit der genannten Ungenauigkeit. Zitate sind auf ein kurzes je Quelle beschränkt; alles andere ist sinngemäß wiedergegeben.

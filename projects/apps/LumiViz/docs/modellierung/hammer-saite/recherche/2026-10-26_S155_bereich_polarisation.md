> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: γ (`polarisationWinkelGrad`) und δ (`polarisationCent`). Suche: im Netz.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Recherche: Wertebereiche für γ (`polarisationWinkelGrad`) und δ (`polarisationCent`)

> **Typ:** Agentenbericht (Recherche-Agent, S155, 2026-10-26). **Prüfstand:** „Volltext gesehen“ heißt: das PDF
> wurde mit `pdftotext`/`quelle_suchen.py` gelesen, das Zitat steht wörtlich in dessen Ausgabe. Bei HTML-Seiten
> stammen die Zitate aus dem Rohtext über `curl` (nicht aus einer WebFetch-Zusammenfassung), wo so vermerkt.
> Sätze aus einer Suchmaschinen-Zusammenfassung sind **kein Beleg** und als solche gekennzeichnet.

## Auftrag

Belegte Wertebereiche je Taste (MIDI 21–108) für
- **γ** — Winkel der ersten Eigenrichtung der Saite gegen die Senkrechte (0 … 90°),
- **δ** — Abstand der zweiten Eigenrichtung in Cent (0 … 20),

vor allem für einsaitige Basstöne (MIDI 21–30). Gesucht: gemessene Frequenzunterschiede der zwei Polarisationen,
Winkel der Eigenrichtungen, Energieanteil der horizontalen Ebene, Schwebungen an einsaitigen Basstönen.
Ausgangsstand: `recherche/2026-10-25_S153_zweite_polarisation.md` und `recherche/2026-10-25_S153_lesenotiz_cuenca_tan.md`
(beide gelesen; hier nur, was dort fehlt, plus die Zahlen aus Tan 2017 wörtlich nachgeprüft).

## Ergebnis vorweg

**Es gibt keine veröffentlichte Messreihe von γ oder δ je Taste an einem Konzertflügel**, die ich im Volltext
gefunden hätte. Belegt sind nur Einzelpunkte:

| Was | Wert | Woher | Prüfstand |
|---|---|---|---|
| δ an einer Saite auf festen Lagern (Monochord) | 0,1–0,2 Hz bei ≈ 188–220 Hz = **0,8–1,9 Cent** | Tan 2017, §4.5 | Volltext |
| δ an einer Saite über einem Stück Bassteg (Zickzack über Stegstifte) | 0,2 Hz, „could range between 0.2Hz to 0.4Hz“ bei ≈ 126,5–127 Hz = **2,7–5,5 Cent** | Tan 2017, §7.2, §7.4 | Volltext |
| Eigenrichtungen der Saite (Monochord) | **+5° / −85°** | Tan 2017, §4.5 | Volltext |
| Eigenrichtungen mit Steg | **+38° / −53°**, Versatz Saitenachsen ↔ Stegachsen **+7°** | Tan 2017, §7.2, §7.4 | Volltext |
| Winkel, der in Tans Rechnung das gemessene Bild ergibt | **22°** | Tan 2017, §7.4 | Volltext |
| E1 (MIDI 28), Einzelsaite eines Flügels/Klaviers, Drehsinn der Bahn wechselt | „about every 6 s“ → gedeutet **Δf ≈ 0,083 Hz (3,5 Cent)** oder 0,17 Hz (7,0 Cent) | Tanaka/Mizutani/Nagai, ASA 1996, Abstract | Abstract (Rohtext), Deutung eigen |
| E1, Anteil horizontal nach der ersten Reflexion am Stegstift | „about 1/5 of the amplitude of vertical velocity“ → gedeutet **γ ≈ 11°**, ≈ 4 % Energie | Mori et al. ICA 2004, **nur über** Nishiguchi 2004 | Volltext des Übersichtsartikels; Primärquelle nicht gesehen |
| Stegstifte im Flügel | „about 15 to 20 degrees to the vertical“; Saite „changes angle by about 15 degrees in the horizontal plane“ | Patent US7884271B2 (Dain) | Rohtext Google Patents |
| Querkopplung am Steg (Kawai-Stutzflügel) | vertikale Antwort auf horizontale Anregung „up to 20 dB lower“ unter ≈ 1 kHz | Karatsovis 2011, §4.5.1 | Volltext |

## Je Quelle

### 1. Jin Jack Tan, *Piano acoustics: string's double polarisation and piano source identification*, Diss. Université Paris-Saclay 2017 (tel-01755039)
- **Zugang:** lokal `quellen/Tan_2017_DoublePolarisationSourceIdentification_PhD_tel-01755039.pdf` (HAL, offen). Volltext gesehen (pdftotext). Seiten unten = gedruckte Seitenzahlen, soweit im Text erkennbar.
- **Monochord (Kap. 4, Zusammenfassung §4.5, S. 86):** „The two identified polarisation angles are +5° and -85° respectively. The detuning between the two modes is found to be between 0.1Hz and 0.2Hz.“
  - Frequenz: Achse von Abb. 4.5 „219.8 219.6 219.4“ Hz; Tab. 4.1 nennt aber f₁ = „187.54“ Hz (mittlere Frequenz, 6 Messungen). Welche gilt, habe ich nicht geklärt → beide gerechnet.
  - Umrechnung δ = 1200·log₂((f+Δf)/f): 0,1 Hz bei 187,54 Hz → 0,92 Cent; 0,2 Hz → 1,85 Cent; bei 219,6 Hz → 0,79 bzw. 1,58 Cent.
- **Saite über Stegstück (Kap. 7; Stück eines Bassstegs Pleyel P-131, Saite im Zickzack über Stegstifte):**
  - §7.2 (S. 143): „at +20°, the dominant mode is about 0.2Hz lower than the orthogonal mode“.
  - §7.2 (S. 148): „the introduction of the lumped bridge introduces a strongly observed detuning of 0.2Hz between the string modes“; „the detuning could range between 0.2Hz to 0.4Hz … As a first assumption, the minimum detuning value of 0.2Hz is adopted.“; Eigenrichtungen „+0.2Hz at -53° and -0.2Hz at +38°“.
  - Frequenz: Achse von Abb. 7.6 „127 … 126.5“ Hz. Umrechnung bei 126,75 Hz: 0,2 Hz → 2,73 Cent; 0,4 Hz → 5,45 Cent.
  - §7.4 (S. 163): „it exhibits a detuning of 0.2Hz and it has an angular offset, γ, of +7° between the string axes and the bridge axes“; „This can be artificially promoted by increasing the angular offset angle from +7° to +22°.“ (Im pdftotext steht statt γ ein leeres Zeichen; dass es γ ist, folgt aus der Lesenotiz S153 und Tab. 7.2 „Angular offset … 7°“.)
  - §7.4 (S. 164): „Energy is principally transmitted through the vertical oscillator even if the string is initially excited horizontally.“
  - Tab. 7.2: Anregungswinkel „-7° (vertical), +83° (horizontal)“, „Angular offset … 7°“; Abb. 7.21: Dichte 7807,3 statt 7828,6 kg/m³ „forming a difference of 0.4Hz between the two frequencies“.
- **Grenzen:** Prüfstand, gezupft, kein Flügel, keine Hammeranregung; eine Saite, eine Frequenz.
- **Zitiert von** Simionato & Fasciani (unten) als „±0.1 or 0.2 Hz (Tan, 2017)“ — also keine eigene Messung dort.

### 2. H. Tanaka, K. Mizutani, K. Nagai, „Two-dimensional motion of a single piano string“, ASA 132nd meeting, Hawaii, Dez. 1996, Abstract 5pMU4
- **Zugang:** https://auditory.org/asamtgs/asa96haw/5pMU/5pMU4.html (frei, nur Abstract). Rohtext über curl gelesen.
- Zitate: „The wrapped single string E1 was measured with this equipment.“ — „(1) The amplitudes of the horizontal and vertical vibrations repeat, increasing and decreasing during the after sound. This means the sound is formed by the coupling of both vibrations, which occurs at the inclined pins of the bridge.“ — „(2) When the string is struck by the hammer, it vibrates at first vertically to the soundboard and then, after few seconds, it starts to rotate. The direction of the rotation changes alternately about every 6 s.“ — „(3) The first direction of the rotation is not determined. It depends on how one touches the key.“
- Instrument nicht genannt (Flügel oder Klavier unbekannt).
- **Eigene Deutung (nicht belegt):** Der Drehsinn einer Ellipsenbahn kehrt sich um, wenn die Phasendifferenz der zwei Eigenschwingungen 0 oder π durchläuft — zweimal je Schwebungsperiode. Dann T_Schwebung ≈ 12 s, Δf ≈ 1/12 Hz = 0,083 Hz. E1 = 440·2^((28−69)/12) = 41,20 Hz → δ = 1200·log₂(1 + 0,0833/41,20) = **3,50 Cent**. Liest man „alle 6 s“ als volle Periode: Δf ≈ 0,167 Hz → **6,99 Cent**.
- Zeitschriftenfassung: Tanaka/Nagai/Mizutani, „Experimental analysis of two-dimensional vibration of a piano string measured with an optical device“, JASA 105(2), 1181 (1999) — laut Literaturliste bei Nishiguchi 2004 und Karatsovis 2011; das ist ebenfalls ein Tagungs-Abstract; nicht gefunden.

### 3. I. Nishiguchi, „Recent research on the acoustics of pianos“, Acoust. Sci. & Tech. 25(6), 413–418 (2004), DOI 10.1250/ast.25.413 (Invited Review)
- **Zugang:** J-STAGE, frei („FREE ACCESS“). Heruntergeladen (siehe unten). Volltext gesehen, §4, S. 416.
- „Mori [46] also measured the movement of a E1 string using a photonic displacement sensor and a pair of accelerometers. According to his experiment, horizontal vibration begins when the vertical wave generated by a hammer strike reaches the bridge pin for the first time. It is estimated that about 1/5 of the amplitude of vertical velocity is transformed into horizontal velocity at bridge pin.“
- „The movement of a string obtained by a high-speed camera and a mirror was also reported for A♯2 and A3 [14,21]. It is shown that the decay of the vertical displacement is faster than that of horizontal one.“ (♯ im pdftotext als „]“.)
- „By means of this model, the discrepancy between the vertical frequency and the horizontal frequency which was observed by Tanaka et al. [43,44] is explained.“ — d. h. Tanaka et al. haben einen Frequenzunterschied beobachtet; **eine Zahl nennt der Übersichtsartikel nicht.**
- Modell [21,45]: zwei orthogonale Federn k_I, k_II und ein Drehwinkel am Stegstift (Abb. 3) — dieselbe Struktur wie unser γ; **Zahlen nennt der Artikel nicht.**
- [46] = T. Mori, K. Kishi, I. Nakamura, „Measurements and analyses on the driving point characteristics of the piano bridge and the two-dimensional vibration of single strings“, Proc. ICA 2004 — nicht gefunden.
- **Eigene Deutung:** Amplitudenverhältnis 1/5 → wenn die erste Eigenrichtung um γ gekippt ist und nur die Projektion zählt: tan γ = 0,2 → γ = 11,3° (sin γ = 0,2 → 11,5°); Energieanteil sin²γ ≈ 3,8–4 %. Das ist ein Ansatz, keine Messung von γ.

### 4. C. Karatsovis, *Acoustic Features of Piano Sounds*, Diss. University of Southampton, ISVR, November 2011 (eprints 333304)
- **Zugang:** eprints.soton.ac.uk gab 403; geladen über Wayback (Schnappschuss 2026-04-06). Lizenz laut Deckblatt: „A copy can be downloaded for personal non-commercial research or study, without prior permission or charge. This thesis cannot be reproduced or quoted extensively from without first obtaining permission“. Volltext gesehen.
- §4.5.1 (S. 98–99): Messung an einem „`Kawai' baby grand piano“; „the measured vertical response on the soundboard, due to a horizontal excitation, can be up to 20 dB lower than its response due to a vertical excitation for frequencies up to about 1,000 Hz.“
- §2.4.3.1 (S. 30): über Weinreich 1977 — „two electrodes measured the string at an angle of 45 degrees to the vertical and horizontal directions“ (Messaufbau, kein Eigenwinkel); „the soundboard is stiffer in the horizontal direction“.
- Literatur, nicht gefunden: Naganuma & Nakamura, „Consideration on the two-dimensional vibration of single piano strings“, Proc. ISMA 2004 — **genau unser Fall, nicht abrufbar gefunden.**
- Keine Zahl für δ oder γ.

### 5. A. R. Baxandall, I. S. Brown, C. E. Gough, „Vibrations of piano strings and coupling to the soundboard“, Proc. Institute of Acoustics (Jahr nicht im Text; zitiert Gough 1981 → frühestens 1981)
- **Zugang:** ioa.org.uk gibt heute 404; geladen über Wayback (Schnappschuss 2025-04-29). 4 Seiten, Scan mit schlechter Textebene (OCR, teils Buchstabensalat). Volltext gesehen, soweit lesbar.
- S. 331: „these modes will be polarised in directions nearly parallel and perpendicular to the motion of the string on the bridge … in practice, these directions are generally not strictly parallel and perpendicular to the plane of the soundboard“ (OCR: „notion“ statt „motion“).
- S. 332–333: horizontales Zupfen regt den gekoppelten Modus deutlich an → „the coupling to the soundboard cannot be strictly perpendicular to the plane of the soundboard“; aus Dämpfung und Frequenzverschiebung lasse sich „the direction of coupling at the bridge“ schätzen.
- Abb. 3: Breite und Abklingzeit „coupled“/„uncoupled“ für Einzelsaiten über die Tastatur, Steinway (Nr. 244269) — **die Werte sind im Scan nicht lesbar**; Achsen 55–3520 Hz. **Diese Abbildung wäre die beste bekannte Quelle für eine Verteilung je Register** — von Hand am Bild ablesen (Patrik/Claude mit Bildansicht).

### 6. Patent US7884271B2, R. J. Dain, „String-bridge interface system and method“ (veröff. 2011-02-08)
- **Zugang:** Google Patents, frei. Rohtext über curl gelesen.
- „In traditional pianos, contact between string and bridge top is maintained by two pins driven in at an angle of about 15 degrees to the vertical.“
- „The bridge pins are inserted in the bridge cap at about 15 to 20 degrees to the vertical“
- „The string changes angle by about 15 degrees in the horizontal plane which is the amount of wrapping round the bridge pin.“
- „The change of angle is typically about 1 to 5 degrees“ (Abknickung in der Senkrechten über dem Stegkopf, Druck).
- „a piano string that is initially excited to vibrate in the vertical plane will begin to develop a component of vibration in the horizontal plane. The plane of vibration rotates with time.“
- **Bedeutung für γ:** Geometrie, keine Messung. Sie macht plausibel, dass die Eigenrichtung am Stift einige bis gut 15–20° gekippt sein kann; dass γ dem Stiftwinkel gleich ist, sagt die Quelle nicht.

### 7. Weitere, ohne neue Zahl
- **Weinreich, KTH-Vorlesung** (motion.html, strings.html; Rohtext über curl): nur Abklingraten „about 8 dB/sec“, Nachklang „less than one-quarter that rate“; Einzelsaite eines Trichords („Eb3 = 311 Hz“ in der Bildunterschrift, „Eb4“ im Text), kein Bass-Einzelsaiter. Kein Winkel, kein Δf.
- **I. Nakamura, „Fundamental theory and computer simulation of the decay characteristics of piano sound“, J. Acoust. Soc. Jpn. (E) 10(5), 289–297 (1989)** (J-STAGE, frei; Volltext gesehen, S. 289–290): referiert Hundley/Benioff/Martin 1978 — „the direction of the string vibration was not a factor since the vertical component dominated the vibration throughout the duration of the tone“; Wogram 1981 — „a double decay phenomenon is observed even for a single string“. **Gegenstimme:** Nach Hundley et al. bleibt die Senkrechte durchweg vorherrschend (spricht für kleines γ). Hundley 1978 selbst (JASA 64, 1303–1309) nicht gesehen.
- **A. Chaigne, „Linear string-soundboard coupling in pianos“, ICA 2010** (frei; Volltext gesehen): Stegstift p1 bildet „an angle α < π/2 with the horizontal plane“ — ohne Zahl; Modell nur senkrecht.
- **Simionato & Fasciani, „Sines, Transient, Noise Neural Modeling of Piano Notes“, arXiv:2409.06513v3 (2025)**: „the horizontal vibration is usually detuned with respect to the vertical one (±0.1 or 0.2 Hz (Tan, 2017))“ — Sekundärzitat von Tan (Monochord!), keine eigene Messung. Lizenz arXiv (nicht geprüft).
- **euphonics.org §7.3 (Woodhouse-Buch, CC BY-NC-SA 4.0)**, nur über WebFetch-Zusammenfassung: Klaviersaiten „excited predominantly in the vertical plane“; keine Zahl — **nicht als Beleg werten.**
- **kevinhunt.com.au, Appendix 2** (P. Phillips, Einzelmechanik Yamaha C3, Saiten C2), nur über WebFetch: qualitativ, keine Zahl.
- **Urabe & Hirobayashi 2008; Kodera & Hirobayashi 2009** (Toyama): laut **Suchmaschinen-Zusammenfassung** verschiedene Frequenz senkrecht/waagrecht mit Schwebung, A4, Hochgeschwindigkeitskamera — Seiten gaben 404, **nicht gesehen, kein Beleg.**
- **Brauchler, Ziegler, Eberhard, Acta Acustica 4(3) 2020**, „Examination of polarization coupling in a plucked musical instrument string …“ (Gitarre, CC BY): 403 bei WebFetch; laut Suchmaschine ≈ 0,2 Hz und Torsion am Sattel — **nicht gesehen.**

## Bereich je Register

Leseweise: „belegt“ = Zahl aus Volltext/Rohtext; „gedeutet“ = eigene Umrechnung einer belegten Beobachtung;
„Ansatz“ = Vorschlag ohne direkte Quelle. MIDI-Frequenzen nach A4 = 440 Hz.

| Register / MIDI | γ min | γ max | γ typ. | δ min | δ max | δ typ. | Quelle | Zitat-Kurzform | Prüfstand |
|---|---|---|---|---|---|---|---|---|---|
| Bass-Einzelsaiter, 21–30 (27,5–46,2 Hz) | ≈ 5° (Ansatz, Monochord +5°) | ≈ 22° (Tan-Rechnung); geometrisch bis ≈ 15–20° (Stiftwinkel) | **≈ 11°** (gedeutet aus Mori E1: 1/5 der Amplitude) | ≈ 3,5 Cent (gedeutet, Tanaka E1, 0,083 Hz) | ≈ 7 Cent (gedeutet, Tanaka E1, 0,17 Hz) | **3,5–7 Cent** (nur E1 = MIDI 28) | Tanaka 1996; Mori 2004 via Nishiguchi 2004; Tan 2017; Dain-Patent | „changes alternately about every 6 s“; „about 1/5 of the amplitude“; „+7° to +22°“; „15 to 20 degrees to the vertical“ | Abstract + Übersicht; Deutung eigen; Primärarbeiten Mori/Naganuma nicht gesehen |
| Tiefer Bass mit Chören, 31–~45 (≈ 49–110 Hz) | — | — | — | — | — | — | keine Zahl gefunden | — | — |
| ≈ 126 Hz (B2/C3, MIDI ≈ 47–48), Saite über Bassteg-Stück, Prüfstand | 7° (gemessener Versatz) | 22° (für das Bild der Messung nötig) | 7° | 2,7 Cent (0,2 Hz) | 5,5 Cent (0,4 Hz) | 2,7 Cent | Tan 2017 §7.2, §7.4 | „detuning of 0.2Hz … angular offset … +7°“; „0.2Hz to 0.4Hz“; „+7° to +22°“ | Volltext; Prüfstand, kein Flügel |
| ≈ 188–220 Hz (MIDI ≈ 54–57), Saite auf festen Lagern | 5° (Eigenrichtung +5°) | — | 5° | 0,8 Cent (0,1 Hz) | 1,9 Cent (0,2 Hz) | ≈ 1,2 Cent | Tan 2017 §4.5 | „+5° and -85°“; „between 0.1Hz and 0.2Hz“ | Volltext; Monochord ohne Steg (unterer Rand, kein Flügel) |
| Mitte/Diskant, ≈ 58–108 | — | — | — | — | — | — | keine Zahl; Baxandall/Brown/Gough Abb. 3 (Steinway, 55–3520 Hz) unlesbar | „not strictly parallel and perpendicular to the plane of the soundboard“ | Scan, Werte nicht lesbar |
| alle (Stegquerkopplung, nicht γ/δ selbst) | — | — | — | — | — | — | Karatsovis 2011 (Kawai-Stutzflügel) | „up to 20 dB lower … up to about 1,000 Hz“ | Volltext |

## Einschätzung für den Optimierer (Vorschlag, nicht belegt)

- **δ:** Die belegten Werte liegen zwischen 0,8 und ≈ 7 Cent; der heutige Rahmen 0–20 Cent ist nach oben großzügig. Ein Rahmen **0,5–8 Cent** im Bass (MIDI 21–30) wäre durch Tan (Steg 2,7–5,5 Cent) und Tanakas E1 (3,5–7 Cent, gedeutet) gedeckt; 20 Cent ist durch nichts belegt.
- **γ:** Belegt sind 5° (feste Lager), 7° (Steg gemessen), 22° (Rechnung passend zur Messung), ≈ 11° (gedeutet aus Mori E1). Ein Rahmen **3–25°** deckt alles; 90° ist durch nichts gedeckt. Gegenstimme Hundley 1978 (Senkrechte dominiert durchweg) spricht für das untere Ende.
- Beides sind Einzelpunkte; eine Abhängigkeit von der Taste ist **nicht** belegt.

## Offene Punkte

1. **Baxandall/Brown/Gough, Abb. 3** (Steinway, Einzelsaiten über die Tastatur, „coupled“/„uncoupled“): am Bild ablesen — die einzige gefundene Verteilung über die Register.
2. **Mori/Kishi/Nakamura, ICA 2004** und **Naganuma/Nakamura, ISMA 2004** („two-dimensional vibration of single (piano) strings“): Primärquellen für E1; nicht online gefunden.
3. **Tanaka/Nagai/Mizutani 1996/1999**: ob eine Zahl für den Frequenzunterschied existiert; nur Abstracts.
4. **Hundley/Benioff/Martin 1978** (JASA 64, 1303–1309, Bezahlschranke) und **Weinreich 1977** (Bezahlschranke): Gegenstimme bzw. Grundlage, nicht gesehen.
5. **Bensa 2003** (HAL tel-00003588): **gesperrt** (HAL-Bot-Sperre, curl erhielt 12 KB HTML statt PDF). Ebenso Cuenca/Tan-HAL-Seiten (liegen aber lokal).
6. **Valiente/Squicciarini/Thompson, Internoise 2022 (IOA)**: Login-Weiterleitung; **POMA 49 (2023) 035019**: Weiterleitung auf signierten Verlags-Link, nicht gefolgt.
7. **Tan 2017, Monochord-Frequenz**: Tab. 4.1 (187,54 Hz) gegen Achse Abb. 4.5 (≈ 219,6 Hz) — nachlesen, welcher Aufbau.

## Reichweite

- **Suchmaschine** (8 Anfragen): Polarisations-Frequenzabstand Basssaite; optische Bahnmessung; Hundley 1978; Stegstifte/„side draft“; Valiente; Weinreich-Folgearbeiten; Daudet ICMC 1999 (zweimal, nicht gefunden); Tanaka; Mori/Naganuma; Urabe/Hirobayashi; Nishiguchi.
- **Abgerufen und gelesen:** KTH-Vorlesung (2 Seiten), auditory.org (Tanaka-Abstract), J-STAGE (Nakamura 1989, Nishiguchi 2004), ICA 2010 Chaigne, arXiv 2409.06513, Google Patents (2 Patente), euphonics.org, kevinhunt.com.au, Wayback (IOA, Southampton).
- **Lokal gelesen:** Tan 2017 (gezielt nach Winkeln/Verstimmung), Bank 2000/2006/2010, Zambon 2008, Miranda Valiente 2024, Cuenca 2007 (nur gezielte Suchwörter).
- **Gesperrt/fehlgeschlagen:** HAL (Bot-Sperre), eprints.soton.ac.uk (403; über Wayback umgangen — Archivkopie, keine Bezahlschranke), ioa.org.uk (404; Wayback), acta-acustica.edpsciences.org (403), patents.justia.com (403), author.ioa.org.uk (Login), pubs.aip.org POMA (Weiterleitung), Toyama-Pure (404).
- Rund 38 Abrufe/Suchen. Kein Treffer ist kein Beweis: Japanische Tagungsbände (ASJ, ISMA 2004 Nara, ICA 2004 Kyoto) sind die wahrscheinlichste Fundstelle und waren nicht erreichbar.
- Eingebauter Browser oder Chrome wurden **nicht** benutzt; es wurde kein Dialog geöffnet.

## Dateien

**Heruntergeladen nach `projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/`** (Freigabe Patrik über den Koordinator):

| Datei | URL | Größe | Zugang/Lizenz |
|---|---|---|---|
| `Baxandall_Brown_Gough_VibrationsPianoStringsCouplingSoundboard_IOA.pdf` | http://web.archive.org/web/20250429063526/https://www.ioa.org.uk/system/files/publications/AR%20BAXANDALL%20IS%20BROWN%20CE%20GOUGH%20VIBRATIONS%20OF%20PIANO%20STRINGS%20AND%20COUPLING%20TO%20THE%20SOUNDBOARD.pdf | 207 598 B, 4 S. | IOA-Proceedings, früher frei auf ioa.org.uk; Lizenz nicht angegeben |
| `Karatsovis_2011_PhD_Southampton_eprint333304.pdf` | http://web.archive.org/web/20260406101009/https://eprints.soton.ac.uk/333304/1/Karatsovis_PhD_document.pdf | 2 486 432 B, 192 S. | ePrints Soton, persönliche nichtkommerzielle Nutzung, Zitate nur kurz |
| `Nishiguchi_2004_RecentResearchAcousticsPianos_AST.pdf` | https://www.jstage.jst.go.jp/article/ast/25/6/25_6_413/_pdf | 170 753 B, 6 S. | J-STAGE „FREE ACCESS“ |

(Eine vierte Datei, Niimi/Yoshino/Yatabe 2026, war thematisch fremd und ist wieder gelöscht.)

**Zwischenkopien von WebFetch** (können gelöscht werden), unter
`C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\tool-results\`:
`webfetch-1793045348817-xgh0r7.pdf` (Chaigne ICA 2010), `webfetch-1793045265195-clgw6c.pdf` (arXiv 2409.06513),
`webfetch-1793045393116-fnl355.pdf` (Nakamura 1989).

**Arbeitsdateien im Scratchpad:** `…\scratchpad\tan2017.txt`, `…\scratchpad\karatsovis.txt` (pdftotext-Ausgaben).

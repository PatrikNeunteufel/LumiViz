# Durchschlagende Zunge: Schwelle, Amplitude, Einschwingen und Tonhöhe über Maßen und Druck

- **Typ:** Agentenbericht (Recherche, unverändert abgelegt)
- **Datum:** 2026-11-11
- **Sitzung:** LumiViz S190, Stufe 34j-4
- **Auftrag:** Theorie und Messungen suchen, wie Ansprechschwelle, obere Grenze („Ersticken“), Amplitude, Einschwingzeit
  und Tonhöhe einer durchschlagenden Zunge von Zungenmaßen, Aufbiegung/Spalt, Kanzelle und Druck abhängen. Ziel sind
  Skalierungsgesetze, mit denen alle Tasten bei einem gemeinsamen Balgdruck ansprechen.
- **Werkzeuge:** WebSearch, WebFetch, `curl -sI` (nur Kopfzeilen). Kein Browser. WebFetch legt PDF-Antworten von
  selbst im Sitzungsordner `tool-results` ab; diese Kopien habe ich mit dem Read-Werkzeug **als Seitenbild** gelesen
  (wichtig für Formeln). Im Repo liegt nur dieser Bericht.
- **Reichweite der Suche:** rund 30 Suchanfragen (englisch, deutsch, ein wenig französisch/spanisch) zu Schwelle,
  Amplitude, Tonhöhe über Druck, Einschwingen, Spieldruck, Stimmerregeln; 14 PDFs und 6 Webseiten gelesen. Nicht
  durchsucht: Google Scholar, ResearchGate, JASA/POMA-Volltexte hinter der Bezahlschranke, Bücher (Fletcher & Rossing,
  Richter 1985). **Kein Treffer ist kein Beweis** — besonders bei Spieldrücken (Abschnitt 6) ist die Lücke eine Lücke
  der Suche, nicht der Literatur.
- **Kennzeichen je Angabe:**
  **[B]** = im Seitenbild der Quelle selbst gelesen (Text, Gleichung oder Abbildung);
  **[A]** = von mir aus einer Abbildung abgelesen (Ablesefehler rund ±5 % der Achse);
  **[Z]** = nur Zusammenfassung eines Werkzeugs (kein Beleg);
  **[E]** = eigene Rechnung oder Ableitung aus belegten Angaben.
- **Zu Zitaten:** Wörtliche Zitate sind bewusst auf wenige kurze Bruchstücke beschränkt (Urheberrecht); sonst steht
  die Fundstelle (Seite, Gleichung, Abbildung) mit sinngemäßer Wiedergabe. Die Fundstellen sind so genau, dass sich
  jede Angabe im PDF in Sekunden prüfen lässt.

---

## 1 Quellen

| Nr. | Quelle | Zugang | Rechtsvermerk (Fundort) |
|---|---|---|---|
| Q1 | N. H. Fletcher: *Autonomous vibration of simple pressure-controlled valves in gas flows.* J. Acoust. Soc. Am. 93 (4), 2172–2180 (1993). DOI 10.1121/1.406857. `https://www.phys.unsw.edu.au/music/people/publications/Fletcher1993.pdf` | frei (UNSW), ganz gelesen | „© 1993 Acoustical Society of America“ (Fußzeile S. 2172) |
| Q2 | A. Z. Tarnopolsky, N. H. Fletcher, J. C. S. Lai: *Oscillating reed valves — An experimental study.* J. Acoust. Soc. Am. 108 (1), 400–406 (2000). `https://www.phys.unsw.edu.au/music/people/publications/Tarnopolskyetal2000.pdf` | frei (UNSW), ganz gelesen | „© 2000 Acoustical Society of America“ (Fußzeile S. 400) |
| Q3 | J. P. Cottingham, C. H. Reed, M. Busha: *Variation of frequency with blowing pressure for an air-driven free reed.* Forum Acusticum / 137th ASA, Berlin 1999. `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN2.PDF` | frei, ganz gelesen (4 S.) | keiner gefunden |
| Q4 | J. P. Cottingham, C. J. Lilly, C. H. Reed: *The motion of air-driven free reeds.* Berlin 1999. `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN1.PDF` | frei, ganz gelesen (4 S.) | keiner gefunden |
| Q5 | J. P. Cottingham: *Theoretical and experimental investigation of the air-driven free reed.* ICA/ASA Seattle 1998, S. 761–762. `https://www.cs.bu.edu/fac/snyder/TablatureWeb/ICA.PDF` | frei, ganz gelesen (2 S.) | keiner gefunden |
| Q6 | H. T. Bahnson, J. F. Antaki, Q. C. Beery: *Acoustical and physical dynamics of the diatonic harmonica.* J. Acoust. Soc. Am. 103 (4), 2134–2144 (1998). DOI 10.1121/1.421359. `https://www.cs.bu.edu/fac/snyder/TablatureWeb/Bahnson%20JASA%201998.pdf` | frei, S. 2135–2142 gelesen | Fußzeile „J. Acoust. Soc. Am.“; Copyright-Zeile steht auf S. 2134 (nicht gelesen) |
| Q7 | J. Braasch, J. P. Cottingham: *Free Reeds: An Intertwined Tale of Asian and Western Musical Instruments.* Acoustics Today 19 (4), 14–22 (2023). DOI 10.1121/AT.2023.19.4.14 | frei, ganz gelesen | „©2023 Acoustical Society of America. All rights reserved.“ (S. 14) |
| Q8 | J. Cottingham, D. Wolff: *Experimental study of free reed initial transients.* Vienna Talk 2015 (nur Abstract, S. 176). `https://viennatalk2015.mdw.ac.at/abstracts/ViennaTalk2015_submission_39.pdf` | frei, gelesen | keiner gefunden |
| Q9 | J. Cottingham: *Reed Chamber Resonances and Reed Tongue Vibrational Modes in Free Reed Excitation.* ISMA 2017 (nur Abstract). `https://isma2017.cirmmt.mcgill.ca/proceedings/pdf/ISMA_2017_paper_52.pdf` | frei, gelesen | keiner gefunden |
| Q10 | R. Llanos-Vázquez, M. J. Elejalde-García, E. Macho-Stadler: *Controllable pitch-bending effects in the accordion playing.* Acoustics'08 Paris, S. 8565–8570. `https://www.conforg.fr/acoustics2008/cdrom/data/articles/001900.pdf` | frei (über https; `curl -sI` über http meldet 404), ganz gelesen | keiner gefunden |
| Q11 | R. Llanos-Vázquez u. a.: *Efecto „pitch bending“ en el acordeón.* Acústica 2004 Guimarães, paper 009. `https://www.sea-acustica.es/archivo/publicaciones/Guimaraes04_ID9.pdf` | frei, ganz gelesen | keiner gefunden |
| Q12 | N. H. Fletcher: *Recent progress in the acoustics of wind instruments.* Acoust. Sci. & Tech. 22 (3), 169–176 (2001). `https://www.phys.unsw.edu.au/music/people/publications/Fletcher2001.pdf` | frei, ganz gelesen | keiner gefunden |
| Q13 | Accordion Revival (Werkstatt-Seite), Kapitel Stimmen/Voicing. `https://accordionrevival.pages.dev/accordion-repair3` | frei, über WebFetch gelesen | Betreiber behält sich alle Rechte vor, keine gewerbliche Vervielfältigung (Seitentext, laut Werkzeug); kein No-AI-Vermerk gefunden |
| Q14 | H. Pomberger, M. Frank, S. Trabesinger, R. Höldrich: *Material-Dependent Sound and Vibration of a Harmonica Reed Block.* DAGA 2021, S. 852–855. `https://pub.dega-akustik.de/DAGA_2021/data/articles/000550.pdf` | frei, ganz gelesen | keiner gefunden |
| Q15 | A. O. St. Hilaire, T. A. Wilson, G. S. Beavers: *Aerodynamic excitation of the harmonium reed.* J. Fluid Mech. 49, 803–816 (1971). DOI 10.1017/S0022112071002374 | **gesperrt** (Bezahlschranke), nur Abstract über Werkzeug | „© 1971 Cambridge University Press“ [Z] |
| Q16 | D. Ricot, R. Caussé, N. Misdariis: *Aerodynamic excitation and sound production of blown-closed free reeds without acoustic coupling: The example of the accordion reed.* J. Acoust. Soc. Am. 117 (4), 2279 (2005). HAL hal-01106161 | **nicht erreicht** (HAL: Zugriff verweigert durch Bot-Schutz „Anubis“) | — |
| Q17 | C. E. Pykett: *The physics of free reeds* (Webseite). `https://www.colinpykett.org.uk/physics-of-free-reed-wind-instruments.htm` | frei, über WebFetch (nur qualitativ) | „Copyright © C E Pykett 2023“ [Z]; Seite „Conditions of use“ nicht gelesen |

Schon bekannt und hier nur als Bezug genannt: Puranik & Scavone FA 2023, Adachi u. a. FA 2026, Millot & Baumann 2007,
Cottingham Physics Today 2011, Elejalde-García u. a. 2021.

**Wichtig zur Einordnung von Q1/Q2:** Fletcher ordnet Ventile nach (σ1, σ2). Die aufschlagende/einwärts schlagende Zunge
ist (−,+) („blown closed“), die Lippe (+,−). Tarnopolsky u. a. (Q2) messen **nur (+,−)**, also eine vom Druck
*aufgeblasene* Klappe aus einem Vorratsvolumen — die Zahlen von Q2 sind **kein** Akkordeon-Maß, sie belegen aber, dass
Fletchers lineare Schwellentheorie quantitativ trägt. Die Akkordeon-/Harmoniumzunge mit Aufbiegung verhält sich nach
Cottingham (Q5, S. 762; Q3, S. 2) wie eine schließende Zunge (−,+), die symmetrische asiatische Zunge wie eine
öffnende.

---

## 2 Schwellendruck der Selbsterregung

### 2.1 Formeln

**F1 — Bewegungsgleichung und Ruhelage (Q1, Gl. 4–6, S. 2174) [B]**

    x'' + 2k x' + ω0²(x − x0) = σ1 μ1 p1 + σ2 μ2 p2,   μ1 = S1/m,  μ2 = (S2 + σ2 S3)/m
    x̄ = x0 + (σ1 μ1 p̄1 + σ2 μ2 p̄2)/ω0²                                     (Gl. 6)

k ist hier die Dämpfung (k = ω0/2Q, so in Q2 S. 401 ausdrücklich), x0 die Öffnung ohne Druck, m die wirksame Masse,
S die wirksame Fläche.

**F2 — Schwellenbedingung (Q1, Gl. 20, S. 2176) [B]**, für rein reaktive Abschlüsse Z1 = jX1 (Zufluss), Z2 = jX2
(Abfluss), p̄2 = 0:

    (2ρ p̄1)^(1/2) · W · (σ2 μ2 X2 − σ1 μ1 X1)
    ───────────────────────────────────────────────────────  >  2 k ω0
    ρ + (W² / 2p̄1) · (x0 + σ1 μ1 p̄1/ω0²)² · (X1 + X2)²

W = Breite der Öffnung. Dazu die Frequenz (Gl. 19) mit demselben Nenner.

**F3 — Vorzeichenbedingungen (Q1, Gl. 21–24, S. 2176–2177) [B]**

- (−,+), also die schließende Zunge: **X1 + X2 > 0** (die Summe der Abschlüsse muss *träge* sein) und **ω < ω0**.
- (+,−): X1 + X2 < 0 (nachgiebig, etwa ein Vorratsvolumen) und ω > ω0.

**F4 — Abschlüsse im nicht resonanten Fall (Q1, Gl. 29, S. 2178) [B]**

    Z1 ≈ −j ρc²/(V ω)   (Volumen V vor dem Ventil),      Z2 ≈ j ρ l2 ω / A2   (Rohr der Länge l2, Fläche A2 dahinter)

**F5 — Kanalträgheit der Öffnung selbst (Q1, Gl. 13 und Text davor, S. 2175) [B]:** Trägheit der Luft im Spalt
ρδ/(W x), δ = Kanallänge (bei der Zunge: Plattendicke). Fletcher lässt sie in Gl. 14–20 als kleine Korrektur weg.

**Woran gegengeprüft**

1. Gl. 6 gegen den Text unter Abb. 5 (S. 2178): Mit den Beispielwerten aus Gl. 28 (m = 100 mg, S = 1 cm², x0 = 1 mm,
   ω0 = 1000 s⁻¹) ergibt Gl. 6 für σ1 = −1 den Schließdruck p = m ω0² x0 / S = 1000 Pa. Der Text nennt genau diese
   Grenze, und die Kurven in Abb. 5 enden bei 1000 Pa. **Vorzeichen und Exponenten von Gl. 6 stimmen.** [E an B]
2. Gl. 20 gegen Abb. 5: (−,+), großes Volumen (X1 ≈ 0), Rohr 100 mm lang, Durchmesser 10 mm (X2 = 1,53·10⁶ Pa·s/m³),
   Q = 5 (Kurve „100“). Die Breite W steht nicht in Gl. 28; mit der **Annahme W = 10 mm** (passt zu S = 1 cm²) rechne
   ich eine Schwelle von rund 185 Pa; abgelesen sind es rund 230–250 Pa. Größenordnung und Lage stimmen; die
   Abweichung liegt an der geratenen Breite. **Form von Gl. 20 damit bestätigt, nicht auf die Stelle genau.** [E an A]
3. Gl. 21 gegen Abb. 5: Schwingung nur für enge (träge) Abflussrohre unterhalb rund 20 mm Durchmesser, darüber nicht —
   passt zu „X1 + X2 > 0“. [B]

**F6 — Fassung mit Seitenspalt, Kontraktion und Verdrängung (Q2, Gl. 2–8, S. 401–402) [B]**, für (+,−):

    F(x) = W[x² + b²]^(1/2) + 2L[a(x)² + b²]^(1/2)                    (Gl. 2)  Austrittsfläche, b = Seitenspalt
    p = ρU²/(2C²F(x)²) + ∂/∂t[ρ U δ / (C F(x))]                       (Gl. 3)  C = 0,61 (scharfkantiger Schlitz)
    x'' + 2k x' + ω0²(x − x0) = 1,5 W L p / m                         (Gl. 4)
    a(x) ≈ 0,6 x0 + 0,4 x                                             (Gl. 5)  mittlere Öffnung längs der Seiten
    k = ω0/(2Q) + β ρ v (W + 0,8L) x0 / m                             (Gl. 6)  aerodynamische Zusatzdämpfung, β ≈ 1
    dp/dt = (ρc²/V)·(U0 − U − 0,4 W L dx/dt)                          (Gl. 7)
    Schwelle: Gl. 8 (S. 402), rechte Seite (4/3)·k ω0 m

Die Faktoren 1,5 und 0,4 folgen aus der Biegeform des Kragbalkens (Anhang, Gl. A2–A5: γ ≈ 1,5; a(x) ≈ 0,6x0 + 0,4x).
Gl. 8 habe ich im Seitenbild gelesen, übertrage sie hier aber **nicht** Zeichen für Zeichen (mehrzeiliger Bruch mit
Überstrichen; Abschreibfehler wären wahrscheinlicher als Nutzen). Wer sie braucht: Q2, S. 402, linke Spalte.
**Gegenprüfung in der Quelle selbst:** Abb. 2 und 3 zeigen Messung (a) und Rechnung (b) übereinander, ohne freie
Parameter; die Autoren nennen die Übereinstimmung mäßig befriedigend.

### 2.2 Was die Formeln für die Schwelle sagen

Aus F2 für die schließende Zunge (σ1 = −1, σ2 = +1, μ1 = μ2 = μ = S/m), solange der zweite Summand im Nenner klein
gegen ρ ist [E]:

    p_th ≈ (ρ/2) · [ 2 k ω0 / (W μ X) ]²        mit X = X1 + X2 > 0

Mit k = ω0/(2Q) und einer trägen Last X = ω0·M (M = akustische Masse hinter/um die Zunge):

    p_th ≈ (ρ/2) · [ ω0 · (m/S) / (Q · W · M) ]²

m/S ist die wirksame Flächenmasse der Zunge, also ∝ ρ_Zunge · h (Dicke h). Die Schwelle steigt demnach
**quadratisch** mit ω0·h/(Q·W·M). Das ist abgeleitet, nicht in der Quelle so ausgeschrieben.

Belegte Aussagen dazu:

- Die Schwelle steigt mit der Dämpfung (kleineres Q): Q1 Abb. 5–7; Q2 Abb. 2 (Messung) [B].
- Es gibt eine obere Druckgrenze, an der die Kurve des schließenden Ventils abbricht, weil es zu ist: Q1 Abb. 4 und
  Text S. 2177 [B].
- Für (+,−) gibt es ein günstigstes Vorratsvolumen; größere und kleinere Volumen heben die Schwelle: Q1 Abb. 6,
  Q2 Abb. 2/3 [B]. **Für die schließende Akkordeonzunge gilt das nicht in dieser Form** — dort muss die Last träge sein.
- Woher bei der Akkordeonzunge ohne Resonator die Trägheit kommt: nach St. Hilaire (Q15) und Ricot (Q16) aus der
  instationären Strömung durch die Spalte selbst [Z, beide nur aus Zusammenfassungen]; bei Fletcher ist das der in
  Gl. 14–20 weggelassene Term F5.
- Llanos-Vázquez u. a. (Q11, S. 4) deuten mit Fletchers Bedingung, warum eine halb geschlossene Klappe mehr Druck
  zum Anspielen braucht: die Reaktanz der Kanzelle wird negativer [B, als Vermutung der Autoren formuliert].
- Kanzellenresonanz nahe der Zungenfrequenz kann hohe Zungen am Ansprechen hindern (Q9, Abstract) [B]. Bei der
  Steirischen sind die Kanzellenresonanzen nur in der viergestrichenen Oktave wirksam (Q14, S. 852, mit Verweis auf
  Richter 1985) [B].

### 2.3 Messwerte zur Schwelle

| Wert | Einheit | Zunge / Ventil | Quelle | Kz. |
|---|---|---|---|---|
| ≈ 0,2 | kPa | Harmoniumzungen 100–200 Hz (amerikanische Saugluftorgel), Schwelle der Schwingung | Q4, S. 1 (zitiert Koopman & Cottingham 1997) | B |
| ≈ 0,05 (Nulldurchgang der Wachstumsrate) | kPa | Orgelzunge C3 (≈ 131–137 Hz) | Q3, Abb. 4 | A |
| 200–220 (Minimum bei ≈ 1,5 l) | Pa | (+,−)-Klappe 28 × 52 mm, 0,15 mm Messing, 105 Hz, Q = 55 | Q2, Abb. 2a | A |
| ≈ 60 (flach ab ≈ 2 l) | Pa | dieselbe Klappe, Q = 16 | Q2, Abb. 2a | A |
| 230 / 260 / 330 (Minima) | Pa | (+,−)-Klappe 0,33 mm, mit Masse auf 139 / 180 / 280 Hz gestimmt, Q = 44–52 | Q2, Abb. 3a | A |
| bis ≈ 900 | Pa | dieselbe, 280 Hz, bei 1,2 l Volumen (abseits des Optimums) | Q2, Abb. 3a | A |
| 100–700 je nach Rohr und Q | Pa | Rechenbeispiel (−,+), m = 100 mg, S = 1 cm², x0 = 1 mm, ω0 = 1000 s⁻¹ | Q1, Abb. 5 | A |

**Belegt und für uns wichtig (Q2, Abb. 3):** Bei *einer* Klappe, deren Frequenz nur über aufgeklebte Masse geändert
wird (Steifigkeit gleich, Masse ∝ 1/f²), liegt das Schwellenminimum bei 139 Hz **tiefer** als bei 280 Hz, und das
günstigste Volumen ist bei der hohen Frequenz kleiner. Die Schwelle hängt also auch dann von der Frequenz ab, wenn
die Feder gleich bleibt. Das ist (+,−), die Richtung des Effekts (höhere Frequenz → höhere Schwelle bei sonst
gleichem Bau) deckt sich aber mit unserem Befund.

Nicht gefunden: eine gemessene Schwellenkurve **über den Tonumfang** eines Akkordeons oder einer Mundharmonika.

---

## 3 Obere Grenze („Ersticken“)

**Theorie [B]:** Für das schließende Ventil endet der Schwingbereich dort, wo der Ruhedruck das Ventil ganz
schließt (Q1, Abb. 4, Punkt D; S. 2177). Aus Gl. 6:

    p_zu = m ω0² x0 / S = K_wirk · x0 / S          (K_wirk = wirksame Federsteife am Bezugspunkt)

**Zahlen**

| Wert | Einheit | Was | Quelle | Kz. |
|---|---|---|---|---|
| 150 | N/m | Federsteife an der Spitze, Orgelzunge C (Wolfinger), mit Gewichten gemessen, linear | Q5, S. 761 | B |
| 2,1·10⁻⁴ | kg | wirksame Masse derselben Zunge (ω_r = 860 s⁻¹) | Q5, S. 761 | B (geprüft: 150/860² = 2,03·10⁻⁴) |
| 4,0 / 8,0 | mm | Breite b / wirksame Länge a (S_r = a·b), Zungenlänge rund 4 cm | Q5, S. 762; Q3, S. 1 | B |
| 0,012 | – | Dämpfungsbeiwert D derselben Zunge | Q5, S. 762 | B |
| ≈ 0,21 | mm/kPa | daraus statische Nachgiebigkeit S_r/K = 32 mm² / 150 N/m; Schließdruck ≈ 4,7 kPa je mm Aufbiegung | Q5, Formel X0 = Xu − (S_r/(M_r ω_r²))·P0 | E |
| 1–2 | mm | Verschiebung der Ruhelage in Strömungsrichtung über den Druckbereich | Q4, S. 1 | B |
| ≈ 0,22 | mm/kPa | Verschiebung der Ruhelage **in Zungenmitte**, A♯-Zunge 118 Hz, 0 → 30 cm H2O: +0,05 → −0,62 mm | Q4, Abb. 2 | A |
| bis 4,1 | kPa | Orgelzunge C3 klingt noch bei 42 cm H2O (höchster Messpunkt) | Q3, Abb. 3 | A |
| 3,3 | s⁻¹ | Wachstumsrate bei 3 kPa, nach Maximum 11,5 s⁻¹ bei 1 kPa fallend | Q3, Abb. 4 | A |

**Stimmerregeln (Werkstatt, kein Messbeleg) [Z, Q13]:** Aufbiegung der Spitze als Ausgangswert etwa **eine
Zungendicke**, danach nach Gehör. Zu kleiner Spalt: die Zunge spricht bei plötzlichem hohem Druck gar nicht an
(„choke“). Zu großer Spalt: träge Ansprache, besonders bei leichtem Balgdruck. Maßstab: eine richtig eingestellte
Zunge spricht bei sehr leichtem Druck gleich an wie ihre Nachbarn. Eine Regel „tiefe Zungen mehr Aufbiegung“
**mit Zahl habe ich nicht gefunden** (weder auf Q13 noch sonst); Q13 nennt keinen Unterschied tief/hoch.

**Beobachtungen am Instrument [B]:**

- Bei großen Balgdrücken sinkt der Ton tiefer Zungen von selbst, die Zunge „ertrinkt“ (span. „se ahoga“) — Q11,
  Tabelle 1. Je größer die Zunge, desto deutlicher.
- Die Zungen C6 bis C♯8 (ohne Ventilstreifen) ließen sich **bei keiner Dynamik und keiner Härte des Anschlags**
  ersticken — Q10, S. 8570. Die Autoren führen das auf das Leck durch den offenen Gegenschlitz zurück.

Damit ist belegt: Ersticken ist ein Problem der **tiefen, großen** Zungen; hohe Zungen ersticken praktisch nicht.
Das entspricht unserem Modellbefund am unteren Ende und zeigt, dass ein Ersticken tiefer Zungen an sich nicht
falsch ist — die Frage ist nur, bei welchem Druck.

---

## 4 Amplitude über dem Druck, Tonhöhe über dem Druck, Hysterese

### 4.1 Amplitude

| Wert | Einheit | Zunge, Bedingung | Quelle | Kz. |
|---|---|---|---|---|
| „15 % der Zungenlänge“ | – | Spitzenamplitude, als „einigermaßen üblich“ bezeichnet, ohne Messangabe | Q7, S. 16 | B |
| „mehrere Millimeter“, dann nahezu konstant, bei hohem Druck fallend | mm | Harmoniumzungen 100–200 Hz, Spitze, ab Schwelle ≈ 0,2 kPa | Q4, S. 1 | B |
| ≈ 1,6 | mm | Spitzenamplitude, Zunge 4,2 cm, bei 4 und 6 cm H2O (0,4–0,6 kPa), Laser | Q4, Abb. 5 | A |
| ≈ 1,2 (Mitte) | mm | A♯-Zunge 118 Hz, **Zungenmitte** (2 cm von der Spitze), 3–12 cm H2O; halber Hub aus oberer und unterer Hüllkurve | Q4, Abb. 2 | A |
| ≈ 1,05 (Mitte) | mm | dieselbe bei 25–30 cm H2O | Q4, Abb. 2 | A |
| ≈ 0,85 (Mitte) | mm | C3-Zunge bei 0,9 kPa | Q4, Abb. 3 | A |
| 2–6, mit dem Druck steigend | mm | (+,−)-Klappe 28 mm lang, 106 Hz, 100–700 Pa, je nach Volumen | Q2, Abb. 4a | A |
| < 2 | mm | dieselbe Bauart als **aufschlagende** Klappe | Q2, S. 405 | B |

Umrechnung Mitte → Spitze [E]: Die erste Biegeform des Kragbalkens hat in der Mitte 0,34 der Spitzenauslenkung
(Q4, Abb. 5 bestätigt die Form an der Zunge). 1,2 mm in der Mitte sind rund 3,5 mm an der Spitze, also rund **9 %**
von 4 cm; die Lasermessung bei 0,4–0,6 kPa gibt 1,6 mm = **4 %**; die C3-Zunge bei 0,9 kPa rund 2,5 mm = **6 %**.

**Befund:** Die einzigen **gemessenen** Amplituden, die ich gefunden habe (Harmoniumzungen bei 0,4–3 kPa), liegen bei
**4–9 % der Länge**, nicht bei 15 %. Die 15 % stehen in Q7 (und sinngemäß in Cottingham 2011) als Aussage ohne
Messkurve. Unsere 5–11 % im Modell liegen im gemessenen Bereich der Harmoniumzunge. Für Akkordeonzungen fehlt mir
eine Messung.

Zu Q4 Abb. 6 (Geschwindigkeit bis ≈ 1100 „cm/s“ bei 118 Hz): Das ergäbe 15 mm Amplitude und passt nicht zu Abb. 2
und 5; ich halte die Achseneinheit für falsch (mm/s gäbe 1,5 mm) und nutze die Abbildung nicht. Außerdem nennt die
Bildunterschrift 6b denselben Druck wie 6a, der Text 1,2 kPa.

**Verlauf (Q4, Abb. 2) [A]:** Nach der Schwelle springt die Amplitude sofort auf ihren Wert und bleibt bis 12 cm H2O
praktisch gleich; darüber nimmt sie um rund 15 % ab, während die Ruhelage stetig in den Schlitz wandert. Bei 0,8 kPa
ist die Bewegung fast sinusförmig, bei 1,2 kPa unregelmäßiger und etwas kleiner (Q4, S. 4) [B]. Oberhalb des normalen
Spieldrucks erscheint die zweite Biegemode (f2 = 6,27·f1) im Spektrum der Zungenbewegung (Q4, S. 2) [B].

### 4.2 Tonhöhe über dem Druck

| Wert | Einheit | Zunge, Bereich | Quelle | Kz. |
|---|---|---|---|---|
| 132,0 → 128,0 | Hz | Orgelzunge C3, ≈ 0,5 → 24 cm H2O (≈ 2,3 kPa); Minimum bei 22–26 cm H2O | Q3, Abb. 3 | A |
| ≈ −2,0 bis −2,2 | Hz/kPa | dieselbe, linearer Teil 1–15 cm H2O | Q3, Abb. 3 | E aus A |
| ≈ −26 bis −29 | Cent/kPa | dieselbe, umgerechnet bei 131 Hz | — | E |
| 128 → 134,5 | Hz | dieselbe, Wiederanstieg von 26 → 42 cm H2O (2,5 → 4,1 kPa) | Q3, Abb. 3 | A |
| 136,9 → 134,2 | Hz | Wolfinger-C-Zunge, 0,05 → 1,2 kPa (Messung) | Q5, Abb. 2 | A |
| ≈ −2,3 | Hz/kPa (≈ −30 Cent/kPa) | dieselbe | — | E aus A |
| 0,6–1,0 | kPa | Bereich, in dem Fletchers lineares Modell die Messung trifft („typische Spieldrücke“) | Q5, S. 762 | B |
| 1–8 | % | Tonhöhenabfall beim Pitch-Bending (halb geschlossene Klappe **und** mehr Balgdruck), E1–E3, Pigini Sirius | Q10, S. 8567, Abb. 2–6 | B |
| −2,5 / −5,1 | Hz/kPa | Akkordeon, große / kleine Klappenöffnung (Adachi u. a. FA 2026) | nur Suchmaschinen-Auszug | Z (liegt euch vor) |

Aussagen [B]:

- Im normalen Spielbereich sinkt die Frequenz annähernd linear mit dem Druck; bei sehr kleinem Druck steigt sie
  zunächst kurz, weit über Spieldruck flacht sie ab und steigt dann stark (Q3, Summary und S. 3).
- Fletchers lineares Modell (Leitwert der Zunge, Arbeitspunkt am negativen Minimum von Re[Y_r], das bei der
  aufgebogenen Zunge **unter** der Eigenfrequenz liegt) gibt den Abfall im mittleren Bereich wieder, aber nur wenn
  die aerodynamische Dämpfung/Anfachung **druckabhängig** angesetzt wird (Q3, S. 3–4; Q5, S. 762). Cottingham nimmt
  dafür eine quadratische Näherung der gemessenen Wachstumsrate (Q3, Abb. 5, Beschriftung
  „Dr=0.0235(p−2)^2 − 0.68“, p in kPa).
- Tiefe Zungen reagieren am stärksten auf den Druck (Q13 [Z]; Q11 Tabelle 1 [B]).

**Einordnung unseres Befunds (−1 Cent/kPa):** Gemessen sind an Harmoniumzungen um 130 Hz rund **−25 bis −30 Cent/kPa**
(−2 bis −2,3 Hz/kPa); Adachis −2,5 Hz/kPa hat dieselbe Größe in Hz. In Fletchers Bild entsteht der Abfall nicht aus
der Federkennlinie, sondern aus der Lage des Arbeitspunkts unter der Resonanz, die mit Druck und Dämpfung wandert
(Gl. 19 in Q1 hat den Druck im Zähler und Nenner). Ein Modell, dessen Tonhöhe kaum auf den Druck reagiert, hat
vermutlich zu wenig reaktive (träge) Last oder eine zu schwache Kopplung S/m — dieselbe Größe, die auch die Schwelle
bestimmt. Das ist eine Deutung [E], kein Beleg.

### 4.3 Hysterese

- (+,−)-Klappe: Ist die Schwingung einmal da, lässt sich der Druck senken, ohne dass sie abreißt; Messkurven mit
  Pfeilen in Q2, Abb. 4a; Drucksprung beim Anwachsen 10–320 Pa je nach Volumen (Q2, Abb. 5) [B/A].
- Akkordeon: Ein Patent für elektronische Akkordeons (US 6 946 594) arbeitet je Zunge mit einem Einschaltdruck Pon
  und einem kleineren Ausschaltdruck Poff, ohne Zahlen [Z].
- Messung der Hysterese an einer Akkordeon- oder Harmoniumzunge: nicht gefunden.

---

## 5 Einschwingzeit, höhere Moden, Torsion

**Formel [E, Lehrbuch]:** Bei exponentiellem Anwachsen mit Rate ε (s⁻¹) dauert der Weg von der Anfangsamplitude a0
bis zum Anteil r der Endamplitude A:  t = ln(r·A/a0)/ε. St. Hilaire u. a. haben das exponentielle Wachstum gemessen
(Q15, Abstract [Z]).

**Messwerte der Wachstumsrate, Orgelzunge C3 (Q3, Abb. 4) [A]:**

| Blasdruck (kPa) | ε (s⁻¹) | 1/ε (ms) | ε/ω (ω ≈ 860 s⁻¹) |
|---|---|---|---|
| 0 | ≈ −1 bis −1,5 (Dämpfung) | — | — |
| 0,2–0,3 | ≈ 4,5 | ≈ 220 | 0,5 % |
| 0,5 | ≈ 8,3 | ≈ 120 | 1,0 % |
| 1,0 | ≈ 11,5 (Maximum) | ≈ 87 | 1,3 % |
| 1,5 | ≈ 10,3 | ≈ 97 | 1,2 % |
| 2,0 | ≈ 9,0 | ≈ 110 | 1,0 % |
| 2,5 | ≈ 6,8 | ≈ 150 | 0,8 % |
| 3,0 | ≈ 3,3 | ≈ 300 | 0,4 % |

Text dazu (Q3, S. 4) [B]: negative Werte (Dämpfung) bei kleinem Druck, größtes Wachstum bei 1,0–1,5 kPa, danach
allmähliche Abnahme. Bei 1 kPa braucht eine C3-Zunge von 1 % auf 90 % der Endamplitude rund ln(90)/11,5 ≈ 0,39 s,
also rund 50 Perioden [E]. Hinweis: Der Wert bei p = 0 (−1 bis −1,5 s⁻¹) passt nicht zu D = 0,012 aus Q5, wenn man
D als 1/Q liest (das gäbe ω·D/2 ≈ 5 s⁻¹); die Definition von D steht in den zwei Seiten nicht, deshalb nicht
gleichsetzen.

**Diese Tabelle ist die brauchbarste Eichgröße für das Modell:** Sie gibt für eine Zunge mit bekannter Steife
(150 N/m), Masse (0,21 g), Breite (4 mm) und Frequenz die Netto-Anfachung über dem Druck, einschließlich Abfall bei
hohem Druck.

**Ablauf und Moden [B]:**

- Der Einschwingvorgang beginnt mit einer Verschiebung der Ruhelage, oft mit wenigen unregelmäßigen Schwingungen;
  dann wächst die Amplitude, und die Frequenz stellt sich ein; danach wächst die Amplitude weiter bis zum Endwert
  (Q8, Abstract).
- Zweite Biegemode und erste Torsionsmode sind im frühen Einschwingen nachgewiesen, die Torsion oft zuerst (Q8).
- Große, tiefe Zungen sprechen langsam an; eine Krümmung der Zunge, die die Torsion beim Anregen stärkt, kann das
  etwas mildern (Q9, Abstract).
- Freie Zungen haben gegenüber aufschlagenden einen langen Einsatz; die höheren Teiltöne kommen verzögert nach dem
  Grundton; die Grundfrequenz steigt im Einsatz über einige Dutzend Millisekunden an (Q7, S. 17–18, Abb. 5 nach
  Braasch & Ahrens 1999/2000 — dort an Orgel-Zungenpfeifen gemessen).
- Mundharmonika: Der Übergang in einen Bend dauert in der Messung rund 100–200 ms (Q6, Abb. 7) [A].

Nicht gefunden: gemessene Einschwingzeiten über Tonhöhe **und** Druck als Tabelle (das wäre Braasch & Ahrens, Acta
Acustica 86 (2000) 662–670, und Paquette/Biernat/Cottingham in POMA — nicht frei erreicht).

---

## 6 Gemessene Spieldrücke

| Wert | Einheit | Instrument, Angabe | Quelle | Kz. |
|---|---|---|---|---|
| ≈ 0,8 | kPa | Harmoniumzunge (amerikanische Orgel), „ungefähr normaler Spieldruck“ | Q4, S. 4 | B |
| 0,6–1,0 | kPa | Harmoniumzunge im Instrument, „typische Spieldrücke“ | Q5, S. 762 | B |
| 0–3 (Messung bis ≈ 4,1) | kPa | Laborbereich an Harmoniumzungen | Q3, S. 3 und Abb. 3 | B / A |
| 0,2–1,5 | kPa | Kanzellendruck im indischen Harmonium (Puranik & Scavone FA 2023) | nur Suchmaschinen-Auszug | Z (liegt euch vor) |
| „40 bis 130 nm“ | (Einheit im Original offenbar verdruckt, vermutlich mm Wassersäule = 0,4–1,3 kPa) | Salonorgel | pumporganrestorations.com | Z |
| 2–3 | Zoll Wassersäule (0,5–0,75 kPa) | Saugluftorgel, Forenangabe | Mechanical Music Digest | Z |
| ≈ 70 | dB(A) in 50 cm | Akkordeon (Pigini Sirius), mezzoforte — **Schallpegel, kein Druck** | Q10, S. 8567 | B |
| 2–3 / 4–6 | kPa | zum Vergleich: Klarinette leise / laut | Q12, S. 172 (nach Fuks & Sundberg) | B |
| 0,2–2 | kPa | zum Vergleich: Flöte tief / sehr hoch | Q12, S. 174 | B |
| „Hunderte ml/s“ | Volumenstrom | Mundharmonika-Kanzelle, mittlerer Durchfluss | Q7, S. 16 | B |

**Nicht gefunden (trotz gezielter Suche auf Englisch und Deutsch, auch nach MIDI-Balgsensoren und medizinischen
Arbeiten):** ein in Pascal gemessener Balgdruck des Akkordeons für pp–ff, ein gemessener Munddruck bei
Mundharmonika oder Melodica. Bahnson u. a. (Q6) geben auf den gelesenen Seiten **keinen** Druck an (Auslenkungen
sind „relativ“). Die Arbeiten der Gruppe aus Bilbao (Q10, Q11) regeln die Dynamik über den Schallpegel, nicht über
den Druck. Wahrscheinliche Fundorte: Richter 1985 (Buch), Dissertation Llanos-Vázquez 2015 (UPV/EHU, Link 404),
Millot/Cuesta/Valette, Acustica 87 (2001) 262–270, Ricot 2005.

---

## 7 Adachi u. a., JASA 2026 (Mundharmonika)

Kein freier Volltext, kein Preprint gefunden (arXiv, Suchmaschine; J-STAGE und Repositorium der Kyushu-Universität
habe ich nicht eigens durchsucht). Die Suche liefert nur die FA-2026-Arbeit zum Akkordeon, die euch vorliegt.
Parameter und Gleichungen der JASA-Arbeit kann ich deshalb nicht angeben.

---

## 8 Schallabstrahlung — was ist die Ausgangsgröße?

- Fletcher/Tarnopolsky (Q2, S. 405) [B]: Das abgestrahlte Spektrum folgt im Prinzip aus dem **gesamten
  Volumenstrom U(t) + 0,4·W·L·dx/dt**, also Strömung durch die Öffnung **plus** Verdrängungsstrom der bewegten
  Zunge. Die Autoren führen es nicht aus, weil die Hüllkurve an Feinheiten des Baus hängt.
- Braasch & Cottingham (Q7, S. 16–17) [B]: Die Tonerzeugung gleicht einer Sirene; der Luftstrom wird zerhackt. Der
  Volumenstrom hat zwei große Stöße je Periode (Zunge über bzw. unter der Platte), kleinere beim Durchgang durch die
  Platte (nach Millot & Baumann). Der Druck nahe der Zunge ähnelt einer Rechteck-/Pulswelle, während die Zunge fast
  sinusförmig schwingt (Q4, S. 2, Abb. 3).
- Pomberger u. a. (Q14, S. 852 und 855) [B]: Stimmstock und Gehäuse strahlen nicht nennenswert; der Schall stammt
  aus dem von den Zungen modulierten Luftstrom.
- Ricot u. a. (Q16) [Z]: Zusätzlich zur aerodynamischen Anregung eine Druckkomponente aus der örtlichen
  Hin-und-her-Bewegung der Luft um die Zunge. Ob die Arbeit das ausdrücklich „Dipol“ nennt, konnte ich nicht
  prüfen.

Klare, belegte Aussage also: **Ausgangsgröße ist der Volumenstrom durch den Schlitz, ergänzt um den
Verdrängungsstrom der Zunge**; die Platte/der Stock als Strahler ist vernachlässigbar.

---

## 9 Nicht gefunden / nicht erreicht

| Quelle | Stand |
|---|---|
| Ricot, Caussé, Misdariis, JASA 117 (2005) 2279 | HAL (hal-01106161, beide Adressen) verweigert den Zugriff (Bot-Schutz); kein IRCAM-Volltext gefunden |
| St. Hilaire, Wilson, Beavers, JFM 49 (1971) 803 | Cambridge: Bezahlschranke; nur Abstract über Werkzeug |
| Tarnopolsky/Fletcher/Lai 2000 | **erreicht und ganz gelesen** (Q2). Der „Vorgänger“ ist Fletcher 1993 (Q1), ebenfalls erreicht. `Tarnopolskyetal2001.pdf` (J. Sound Vib. 247, Strömungsbilder) geladen, aber nicht gelesen |
| Bahnson 1998 | **erreicht** (Q6), S. 2135–2142 gelesen; keine Drücke |
| Cottingham-Sammlung cs.bu.edu | drei Arbeiten erreicht (Q3–Q5); weitere Dateinamen dort nicht bekannt, Verzeichnis nicht aufgelistet |
| Adachi u. a., JASA 159 (5) 2026 | kein freier Volltext gefunden |
| Biernat & Cottingham POMA 2014; Paquette; Coyle/Behrens/Cottingham 2009 | nur Abstracts über Suchmaschine; Volltext nicht gefunden |
| Tonon 2009 („A dynamical theory for the free reed“ o. ä.) | nicht gefunden; nur Tonon 2005 (Kanzellenresonanz, concertina.org), ohne Druck- oder Schwellenangaben |
| Debut & Millot 2001; Millot Dissertation 1999; Millot/Cuesta/Valette 2001 | nicht gefunden |
| Johnston, Acoustics Australia 15 (1987) 69–75 | genannte Adresse (mis.ucd.ie) existiert nicht mehr; Archiv der Zeitschrift nicht geprüft |
| Fletcher & Rossing, *The Physics of Musical Instruments* (Kapitel zu Zungen) | Springer, Bezahlschranke |
| G. Richter, *Akustische Probleme bei Akkordeons und Mundharmonikas*, Teil 1 und 2 (Schmülling 1985) | Buch, nicht online; in Q14 als Quelle für Kanzellenresonanz und Ansprache genannt — **vermutlich die ergiebigste Quelle für Akkordeon-Zahlen** |
| Braasch & Ahrens, Acta Acustica 86 (2000) 662–670 (Einschwingen freier Zungenpfeifen) | nicht gesucht/erreicht |
| Dissertation Llanos-Vázquez 2015 (UPV/EHU, „Acústica del Acordeón“) | Handle `addi.ehu.es/handle/10810/16562` gibt 404 |
| Dissertation Puranik (McGill) | in eScholarship nicht gefunden (nur über Suchmaschine gesucht) |
| Koopman & Cottingham, Reed Organ Society Bulletin 15 (1997) 17–23 | nicht online gefunden; Ursprung der Angaben „Schwelle 0,2 kPa“ und Amplitude über Druck |

---

## 10 Ladeliste

Größen aus `curl -sI` (Content-Length). Rechtsvermerke siehe Abschnitt 1; alle sind Verlags- oder Tagungsbeiträge
ohne offene Lizenz, also Ablage unter `…\cmake\quellen\zunge\`, nicht im Repo.

| URL | Bytes | wofür |
|---|---|---|
| `https://www.phys.unsw.edu.au/music/people/publications/Fletcher1993.pdf` | 853 591 | Schwellentheorie, Gl. 4–6, 13, 19–29, Abb. 4–7 — **Pflicht** |
| `https://www.phys.unsw.edu.au/music/people/publications/Tarnopolskyetal2000.pdf` | 125 148 | Verfeinerte Gleichungen (Seitenspalt, Kontraktion, Verdrängung), gemessene Schwellen und Amplituden — **Pflicht** |
| `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN2.PDF` | 671 902 | Tonhöhe über Druck, **Wachstumsrate über Druck** (Abb. 3–5) — **Pflicht** |
| `https://www.cs.bu.edu/fac/snyder/TablatureWeb/BERLIN1.PDF` | 607 160 | Amplitude und Ruhelage über Druck, Biegeform, Wellenformen — **Pflicht** |
| `https://www.cs.bu.edu/fac/snyder/TablatureWeb/ICA.PDF` | 156 110 | Zungenparameter (k, M, a, b, D), Tonhöhe über Druck — **Pflicht** |
| `https://www.cs.bu.edu/fac/snyder/TablatureWeb/Bahnson%20JASA%201998.pdf` | 1 721 345 | Mundharmonika: Zungenpaar, Wellenformen, Bends |
| `https://acousticstoday.org/wp-content/uploads/2023/11/Free-Reeds-An-Intertwined-Tale-of-Asian-and-Western-Musical-Instruments-Jonas-Braasch-and-James-P.-Cottingham-3.pdf` | 406 964 | Überblick, Einsatz freier Zungen, „15 %“ |
| `https://www.conforg.fr/acoustics2008/cdrom/data/articles/001900.pdf` | 436 100 (laut WebFetch; `curl -sI` über http: 404) | Akkordeon: Bending 1–8 %, hohe Zungen ersticken nicht |
| `https://www.sea-acustica.es/archivo/publicaciones/Guimaraes04_ID9.pdf` | 225 883 | Akkordeon: tiefe Zungen „ertrinken“, Deutung mit Fletcher |
| `https://www.phys.unsw.edu.au/music/people/publications/Fletcher2001.pdf` | 188 169 | Überblick, Vergleichsdrücke anderer Bläser |
| `https://www.phys.unsw.edu.au/music/people/publications/Tarnopolskyetal2001.pdf` | 813 342 | Strömungsbilder am Zungenventil (ungelesen; nur falls die Wirbeldämpfung β gebraucht wird) |
| `https://viennatalk2015.mdw.ac.at/abstracts/ViennaTalk2015_submission_39.pdf` | 528 153 | Abstract Einschwingen |
| `https://isma2017.cirmmt.mcgill.ca/proceedings/pdf/ISMA_2017_paper_52.pdf` | 193 265 | Abstract Kanzellenresonanz, Torsion |

Zu beschaffen (nicht frei): Ricot 2005, St. Hilaire 1971, Richter 1985 (beide Teile), Braasch & Ahrens 2000,
Koopman & Cottingham 1997, Adachi JASA 2026.

---

## 11 Eigene Einschätzung (keine Quelle sagt das so — Ableitung aus Abschnitt 2 und 3)

**Zwei Größen bestimmen beide Enden des Schwingbereichs**, und beide enthalten dieselbe Kopplung S/m:

1. **Obere Grenze** (belegt als Mechanismus, Q1 Abb. 4; Formel aus Q1 Gl. 6):
   p_zu = K·x0/S. Für einen gleichmäßigen Kragbalken unter gleichmäßigem Druck ist die Spitzenauslenkung
   1,5·p·L⁴/(E·h³) (Balkenlehre, hier nicht aus einer der Quellen), also

       p_zu ≈ E·h³·x0 / (1,5·L⁴)

   Mit der Werkstattregel x0 ≈ h (Q13) wird daraus **p_zu ≈ (E/1,5)·(h/L)⁴**. Für Stahl (E ≈ 200 GPa) und
   h/L = 0,01 sind das rund 1,3 kPa. Das trifft die Größenordnung unseres Befunds („tiefe Zungen ersticken über
   rund 1 kPa“) erstaunlich genau.

2. **Untere Grenze** (Form belegt durch Q1 Gl. 20, die geschlossene Näherung ist von mir):
   p_th ≈ (ρ/2)·[ω0·(m/S)/(Q·W·M)]², mit m/S ∝ ρ_Zunge·h.

**Folge für die Stufung:**

- Nur die Länge stufen (h fest): f ∝ h/L² ⇒ h/L ∝ √f ⇒ **p_zu ∝ f²** und **p_th ∝ f²/(Q·W·M)²**. Tiefe Zungen
  ersticken früh, hohe brauchen viel Druck. Das ist genau das Muster, das unser Modell zeigt (F3 < 150 Pa, A6 ab
  1 200 Pa; tiefe Zungen ersticken über 1 kPa). **Das Modell verhält sich also folgerichtig — die Maßtafel ist das
  Problem, nicht die Rechnung.**
- **Geometrisch ähnliche Seitenansicht** (h/L fest, also L ∝ 1/f **und** h ∝ 1/f, Aufbiegung x0 ∝ h): p_zu bleibt
  über den Umfang gleich, und ω0·h bleibt gleich, also auch p_th — sofern Q, Breite und träge Last nicht mit der
  Taste wandern. Das ist das Skalierungsgesetz, das ich vorschlage zu prüfen.
- Reale Zungensätze liegen dazwischen (Länge ändert sich über sechs Oktaven viel weniger als 64-fach; tiefe Zungen
  tragen Gewichte an der Spitze, Q11 S. 3 [B]; hohe sind dünn geschliffen). Ein Spitzengewicht senkt f bei gleicher
  Feder — es lässt p_zu unverändert, erhöht aber m/S und damit p_th. **Maße realer Zungensätze je Taste habe ich in
  dieser Recherche nicht erhoben**; dafür wären Hergert FA 2023 und Puranik (liegen euch vor) sowie Richter 1985 die
  Quellen.
- Stellgrößen je Taste in der Reihenfolge ihrer Wirkung [E]: (a) Dicke h zusammen mit L (setzt h/L), (b) Aufbiegung
  x0 (hebt nur p_zu, linear; in Fletchers Gl. 20 senkt ein größeres x0 über den Nenner zugleich die Anfachung),
  (c) Breite W und Seitenspalt b (W geht quadratisch in p_th ein; der Seitenspalt nach Q2 Gl. 2 als Leck),
  (d) träge Last (Plattendicke/Kanaltiefe δ nach Q1 Gl. 13, Kanzelle und Tonloch nach Gl. 29),
  (e) Dämpfung Q (quadratisch in p_th).

**Was belegt ist und was nicht**

| Aussage | Stand |
|---|---|
| Schwelle steigt mit Dämpfung; es gibt eine obere Druckgrenze beim schließenden Ventil | belegt (Q1, Q2 Messung für (+,−)) |
| Schließende Zunge braucht träge Last, schwingt unter ω0 | belegt als Theorie (Q1); Cottingham bestätigt „unter ω_r“ an der aufgebogenen Zunge (Q5) |
| Anfachung steigt bis ≈ 1–1,5 kPa und fällt danach (C3-Harmoniumzunge) | belegt, Messung (Q3 Abb. 4) |
| Amplitude nach der Schwelle sofort groß, dann fast konstant, bei hohem Druck leicht fallend | belegt, Messung (Q4) |
| Amplitude 4–9 % der Länge bei Harmoniumzungen | aus Abbildungen abgelesen und umgerechnet (Q4) |
| „15 % der Länge“ | Aussage ohne Messkurve (Q7) |
| Tonhöhe −2 bis −2,3 Hz/kPa bei ≈ 131–137 Hz | abgelesen (Q3, Q5) |
| Tiefe Zungen ersticken, C6–C♯8 nie | belegt als Beobachtung (Q10, Q11) |
| Aufbiegung ≈ eine Zungendicke | Werkstattregel (Q13), nur über Werkzeug gelesen |
| p_th ∝ [ω0·h/(Q·W·M)]², p_zu ≈ (E/1,5)(h/L)⁴, Ähnlichkeitsstufung gleicht beide aus | **abgeleitet**, nicht belegt; am Modell nachzurechnen |
| „Tiefe Zungen brauchen mehr Aufbiegung“ mit Zahl | nicht gefunden |
| Balgdruck Akkordeon pp–ff, Munddruck Mundharmonika/Melodica in Pa | nicht gefunden |

**Vorschlag für die Nachrechnung (vor einer Vorlage):** je Taste p_zu und p_th aus der heutigen Maßtafel mit den
beiden Näherungen ausrechnen und gegen die im Modell gemessenen Fenster legen. Stimmen die Verläufe, ist die
Näherung als Entwurfsregel brauchbar. Danach die Ähnlichkeitsstufung (h/L fest, x0 = h) am Modell prüfen — gegen
alle vier Zielgrößen (Schwelle, obere Grenze, Amplitude/Länge, Cent/kPa), dazu die Wachstumsrate gegen Q3 Abb. 4
(Zunge mit K = 150 N/m, M = 0,21 g, b = 4 mm, f ≈ 137 Hz: Maximum ≈ 11,5 s⁻¹ bei 1 kPa).

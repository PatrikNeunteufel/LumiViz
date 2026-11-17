> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_02_harfe_besaitet_admittanz` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 20:23–20:35, claude-opus-5-5, 66 Runden, success, 2.74 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Quellen unter `…\cmake\quellen\gezupfte-saite\`):
> - **Wörtliche Zitate in der Textebene gefunden** (normalisiert): Le Carrou 2006 S. 48–50 (Papier, Punkt 34 zwischen
>   A♭ und B♭ 103,8 / 116,5 Hz, „6-mm-thick semi-conical shell“, 0,029 m³, Mode 1 „connections to the arm …“, „it seems
>   difficult to extract these two similar mode shapes“ — Ligatur „ﬃ“ fehlt in der Textebene, `pdf_suche.py` findet
>   „difficult“ deshalb nicht); Bell S. 147 (Plastikbänder, Winkel, Firth), S. 150 (am **Seitenbild**: „more even and in
>   general slowly increases“ — die OCR-Textebene verliest „even“), S. 155 (85 / 92 Hz), S. 157 („rarely exceed ±3dB“),
>   S. 158 (Hochfläche 1–1,5 kHz); Waltham 2008 S. 52 (Maße, 11–12 → 2–2,5 mm, Hammer und 0,2-g-Aufnehmer); Le Carrou
>   2010 S. 3204–3205 (70 Hz–20 kHz, 2,35 m, „below 220 Hz … non-directional“).
> - **Le Carrou 2006 Abb. 3.4 am Seitenbild:** f und ζ aller sechs Moden stimmen (54,8 / 5,5 % … 168,5 / 1,4 %), Q =
>   1/(2ζ) richtig gerechnet.
> - **Le Carrou 2006 Abb. 3.5 am Seitenbild (graue Kurve, rechte Achse = linke × 2):** die Werte bis 168 Hz stimmen in
>   der Ablesegenauigkeit (Tal 100 Hz 0,4–0,5, Saiten A♭ / B♭ 0,6 / 1,0, Mode 4 6,6–6,8 · 10⁻³ s/kg). **Die Spitze bei
>   207 Hz liegt höher als im Bericht: rund 7,1 statt 5,8 · 10⁻³ s/kg, also 73 statt 71 dB in Bells Maß.** Der
>   Widerspruch zu Bell an den Saiten 33 / 32 (28 / 33 dB) bleibt; an der Spitze 5–13 statt 7–15 dB.
> - **Le Carrou 2010 Abb. 2 am Seitenbild:** stimmt bis auf den ersten Einbruch (bei rund 275 Hz auf rund 30 dB, nicht
>   290 Hz / 26 dB); Spitze 62–63 dB um 200 Hz bestätigt.
> - **Folge für KQ29 a (Güte der Luft der Harfe 5,5, Moden 192 / 208 Hz Q 13):** an der **besaiteten** Camac sind T1 und
>   A0 nach Abb. 3.4 deutlich schwächer gedämpft — Q 22 (152 Hz) und 36 (168,5 Hz); KQ29 a stützt sich auf Bells
>   **unbesaitete** Salvi (Q 11 / 16). Für die Spitze um 207 Hz gibt Le Carrou kein ζ. Gehört in die Nacharbeit zur
>   Harfe (Auftrag 22).
> - **Nicht gegengelesen:** Bell Abb. 7.21 (relative Kurve, PDF 308), Le Carrou Abb. 3.3 / 3.8 und Tab. 3.2, der
>   Camac-Blog (`[Werkzeug]`), die Ladeliste.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_02_harfe_besaitet_admittanz.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_02_harfe_besaitet_admittanz/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.

# Bericht: Admittanz und Abstrahlung der besaiteten Konzertharfe (34k)

- **Auftrag:** `auftraege/2026-11-16_02_harfe_besaitet_admittanz.md`
- **Datum:** 2026-11-16 · **Modell:** claude-opus-5-5 · **Budget:** 5 USD, verbraucht rund 2,4 USD
- **Reichweite:** 10 Suchläufe mit `pdf_suche.py` in Le Carrou 2006 (Thèse), Le Carrou 2007, 2009, 2010, Waltham 2008
  und Bell 1988 (Kap. 6–8, PDF 255–335). Dazu 12 Textauszüge mit `pdftotext` und 10 Seitenbilder (Le Carrou 2006
  PDF 61–63, 65, 66, 158; Le Carrou 2010 PDF 2–3; Waltham 2008 PDF 1–2; Bell PDF 308). Im Netz 3 WebSearch und 3 WebFetch;
  davon brachte einer HTTP 404 (Bell & Firth, IOA). **Nicht durchgesehen:** Chadefaux 2012/2013, Camac 2022,
  Le Carrou/Gautier/Badeau ICA 2007; Le Carrou 2009 nur per Suche (keine Admittanzwerte gefunden). Die Seiten von Bell
  Kap. 7–8, die nur aus Abbildungen bestehen, haben keine brauchbare Textebene; 19 Seiten gab `pdf_suche` als „ohne
  Textebene“ zurück. Ohne OCR ist dort nichts gesucht.
- **Aufgebaut auf** `LumiViz/…/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` mit den Abschnitten §11 (Profile
  S(1,2)–S(1,8), Messorte, Bezug 116 dB = 1 s/kg), §12 (Feder an der Diskantkante; Lücke an Ort 2) und §14 (Bell
  Abb. 7.22 und 7.6, Le Carrou 2007 Abb. 5 und 7; der Widerspruch, der hier untersucht wird). Diese Werte werden hier
  nicht wiederholt.
- Seitenbilder liegen unter `ergebnisse/2026-11-16_02_harfe_besaitet_admittanz/seiten/`.

## Kurzfassung

1. **Beide Messungen an der besaiteten Harfe wurden mit gedämpften Saiten gemacht.** Bell dämpfte mit Plastikbändern,
   Le Carrou mit Papier. Beide nennen den Betrag der Punkt-Admittanz in s/kg (= m s⁻¹ N⁻¹), also dieselbe Größe und
   dieselbe Einheit. Nur die dB-Bezüge sind verschieden (siehe 1.4).
2. **Am selben Saitenort widersprechen sich die beiden um 25–33 dB.** Le Carrous Punkt 34 liegt zwischen den Saiten
   A♭ und B♭ (103,8 / 116,5 Hz). Dort misst er bei 104–117 Hz 0,6–1·10⁻³ s/kg, in Bells Maß 52–56 dB. Bell Abb. 7.22
   gibt an Saite 33 (104 Hz) 80 dB und an Saite 32 89 dB. An der Spitze um 207 Hz ist der Abstand kleiner: Le Carrou
   71 dB, Bell 78–86 dB.
3. **Eine belegte Erklärung, warum Saiten die Admittanz anheben, gibt es in keiner gelesenen Quelle.** Bell beschreibt
   den Befund nur („more even … slowly increases“, S. 150). Er verweist auf Firth, nach dem die Spitzen beim Besaiten
   *niedriger* werden. Dass die Täler steigen, erklärt niemand.
4. **Die Harfen sind sehr verschieden gebaut.** Le Carrous Camac hat in der Mitte des Bodens Kohlefaser, eine
   Aluminiumschiene und Zedernholz. Bells Salvi hat einen Boden aus Fichte. Das ist ein möglicher Grund für die
   niedrigere Mobilität bei Le Carrou, aber eine eigene Einschätzung, kein Beleg.
5. **Neu für den hohen Frequenzbereich:** Bell Abb. 7.21 zeigt die besaitete Harfe von 50 Hz bis 10 kHz an den
   Messorten 4 und 7. Die Kurve steigt bis zu einer Hochfläche bei 1–1,5 kHz und fällt dann flach ab. Die Skala ist
   **nur relativ** (dB ohne Bezug).
6. **Abstrahlung je Kraft bis 1 kHz:** Le Carrou 2010 Abb. 2 (Fernfeld 2,35 m, halbreflexionsarm, Bezug 2·10⁻⁵ Pa/N).
   Bis 2 kHz ist nichts gefunden.

---

## 1 Besaitet gegen unbesaitet

### 1.1 Le Carrou 2006, Thèse, Kap. 3 (Camac Atlantide Prestige, besaitet)

Datei `LeCarrou_2006_VibroacoustiqueHarpeConcert_These.pdf`. Der Messaufbau steht auf S. 48 (PDF 59) und ist
`[wörtlich]` gelesen:

- „The concert harp is studied in playing configuration: all strings are mounted and tuned. For these measurements,
  strings are damped with paper to prevent their vibration while keeping the static deformation and load imposed by
  them on the soundboard.“
- Angeregt wird mit einem Shaker (weißes Rauschen) über einen Stab durch Schallloch 4 an der Rückseite des Bodens. Die
  Kraft misst ein Aufnehmer, der direkt auf die Rückseite geklebt ist. Die Schnelle misst ein Laservibrometer
  **senkrecht zum Boden** (S. 49).
- „The excitation point is labeled 34 … located between the Ab and the Bb string (respective fundamental frequencies at
  103.8 and 116.5 Hz) attachment points.“ Punkt 34 liegt also **im Bass**, auf der Saitenleiste.
- Der Bereich ist 0–300 Hz (S. 49). Eine Messung ohne Saiten gibt es in der Thèse nicht. Die Suche nach „sans
  cordes“, „unstrung“ und „cordée“ ergab nur die Stelle auf S. 124, die sich auf die besaitete Harfe bezieht.

**Abb. 3.5 (S. 52, PDF 63), |ẇ₃₄/F|, Punkt-Mobilität, besaitet mit gedämpften Saiten.** Die rechte Achse ist linear,
0–10·10⁻³ m s⁻¹ N⁻¹; die graue Kurve ist „Soundboard“. Abgelesen am Bild auf ±0,4·10⁻³. Die Umrechnung in Bells Maß
ist `[eigene Rechnung]`: L = 20·log₁₀(Y) + 116,0 dB.

| f [Hz] | |ẇ₃₄/F| [10⁻³ s/kg] | in Bells dB | Marke |
|---|---|---|---|
| 55 (Mode 1) | 0,7 | 53 | [am Bild abgelesen ±0,4] |
| 81 (Mode 2) | 2,0 | 62 | [am Bild abgelesen ±0,4] |
| 100 (Tal) | 0,5 | 50 | [am Bild abgelesen ±0,4] |
| 104 / 117 (Saiten A♭/B♭) | 0,6 / 1,0 | 52 / 56 | [am Bild abgelesen ±0,4] |
| 123 (Mode 3) | 1,4 | 59 | [am Bild abgelesen ±0,4] |
| 152 (Mode 4) | 6,8 | 73 | [am Bild abgelesen ±0,4] |
| 168 (Mode 6) | 4,6 | 69 | [am Bild abgelesen ±0,6] |
| 207 (Spitze) | 5,8 | 71 | [am Bild abgelesen ±0,4] |
| 250 | 0,8 | 54 | [am Bild abgelesen ±0,4] |
| 300 | 0,4 | 48 | [am Bild abgelesen ±0,4] |

Das passt zum Wert in S197 §14 (0,5–6·10⁻³, 50–72 dB).

**Abb. 3.3 (S. 50, PDF 61): Transfer |ẇ₂₃/F|** (Anregung an Punkt 34, Antwort an Punkt 23; dessen Lage zeigt nur
Abb. 3.2). Die Achse ist in dB mit dem Vermerk „Ref 1dB: 5×10⁻⁸ ms⁻¹N⁻¹“. Gelesen als 0 dB = 5·10⁻⁸ s/kg, das ist
eine **Annahme**; dann gilt L_Bell = L − 30,0 dB `[eigene Rechnung]`. Die Spitzen liegen bei 80–90 dB, also 50–60 dB
in Bells Maß. Der gemessene Verlauf zwischen 190 und 300 Hz liegt bei 78–90 dB, also 48–60 dB `[am Bild abgelesen
±2 dB]`.

**Abb. 3.8 (S. 55, PDF 66)** zeigt |ẇ₃₄/F| in dB von 180 bis 260, **ohne Bezug**. Die Achse ist in sich nicht
stimmig: Spitze 152 Hz zu Tal 95 Hz ergibt dort rund 54 dB, in Abb. 3.5 linear nur 23 dB. **Nicht als Pegel
verwenden.** Brauchbar ist Tab. 3.2 auf derselben Seite; sie steht in Abschnitt 3.

### 1.2 Bell 1988 (Salvi „Orchestra“): wie die besaitete Harfe gemessen ist

`[wörtlich]`, Bell S. 147 (PDF 273), §7.3:

- „Tests were also made with the harp strung and tuned to pitch, the strings being heavily damped by ribbons of plastic
  being woven between the strings. During these tests, the vibrator was held at a slight angle to the plane of the
  strings (<10°), with the pin of the impedance head being as close to the centre of the cover bar as the strings would
  allow.“
- Die fertige Harfe hing an Schnüren an Hals und Fuß („resonant frequency … less than ½Hz“). Gemessen wurde im
  reflexionsarmen Raum.
- Dieselbe Seite: „Firth[4] found that the peak-trough heights of the input admittance levels of the modes decreased
  when the harp was strung.“

`[wörtlich]`, Bell S. 150 (PDF 276), über die fertige Harfe:

- Unbesaitet gegen den losen Kasten: „no great difference … a general slight increase in input admittance level for
  the upper half of the harp at frequencies above 500Hz“.
- Besaitet: „The principal change is that the input admittance levels at the resonant antinodes have lowered. The
  peak-trough heights of all the antinodes has also fallen. … The input admittance of the harp is now more even and in
  general slowly increases across the frequency range.“

`[wörtlich]`, Bell S. 157 (PDF 283), §7.6: Anregung unter dem Saitenwinkel (30°) gegen senkrecht, gemessen an der
**unbesaiteten** Harfe an den Messorten 8 und 28: „The difference rarely exceed ±3dB.“ Der Winkel von höchstens 10°
bei der besaiteten Messung erklärt den Unterschied also nicht.

`[wörtlich]`, Bell S. 159 (PDF 285), §7.8, zu Abb. 7.22: „values of input admittance … at the string position and
frequency“. Die Methode wird dort nicht eigens genannt. Abb. 7.22 gehört zu Kap. 7, und dort sind alle
besaiteten Messungen mit gedämpften Saiten gemacht (S. 147). Dass das auch für Abb. 7.22 gilt, ist
**`[eigene Einschätzung]`, nicht ausdrücklich belegt**.

**Bell Abb. 7.21 (PDF 308), „Two Input Admittance Traces of Soundbox on Constructed Harp (with strings)“, 50 Hz–10 kHz,
logarithmische Frequenzachse.** Die dB-Achse ist relativ (0 bis −40 dB, kein Bezug angegeben); Messort 7 ist laut
Bild „shifted downwards by 20 dB for clarity“. Abgelesen am gedrehten Bild auf ±1,5 dB und ±10 % in f; bei Messort 7
ist die Verschiebung um 20 dB schon zurückgerechnet.

| f [Hz] | Messort 4 [dB rel.] | Messort 7 [dB rel.] |
|---|---|---|
| 50 | −20 | −20 |
| 100 | −17 | −15 (Spitze bei 105 Hz −9) |
| 200 | −11 | −10 |
| 300–600 | Spitzen 0 … −5, Täler −8 … −11 | Spitzen +2 … −4, Täler −5 … −10 |
| 1000–1300 | Hochfläche, höchste Spitze ≈ +4 | Hochfläche, höchste Spitze ≈ +2 |
| 2000 | −4 | −4 |
| 5000 | −7 | −12 |
| 10000 | −10 | −15 |

Bell S. 158 `[wörtlich]`: „The input admittance rise to a plateau of maxima between 1kHz and 1.5kHz, where after the
input admittance level smoothly falls back … this region is said to be mass controlled.“

Bezieht man die Kurve auf Bells Werte an den obersten Saiten (Abb. 7.22: 98–103 dB bei 700–990 Hz, Messort 4 liegt am
Diskant), dann läge 0 dB rel. bei etwa 101 dB. Daraus ergäben sich für Messort 4 rund 81 dB bei 50 Hz, 90 dB bei
200 Hz, 97 dB bei 2 kHz und 94 dB bei 5 kHz. Das ist **`[eigene Rechnung]` mit einer Annahme über den Bezug, ±5 dB,
nicht belegt.** Zwischen 2 und 10 kHz fällt die Kurve nur um 6 dB (Messort 4); reine Masse gäbe rund 14 dB `[eigene
Rechnung]`.

### 1.3 Waltham et al. 2008 (Salvi Aurora)

Datei `Waltham_etal_2008_HarpSoundboards_CanAcoust36.pdf`, S. 52–53 (PDF 1–2), `[wörtlich]`: „the soundboard was
scanned by measuring the driving-point admittance at many points along the axis of symmetry … using a small, light
(0.2g) accelerometer and an impact hammer.“

Abb. 4 ist eine Graustufenkarte: Höhe h (0–1200 mm) über f (0–1000 Hz), Admittanz in s/kg von 10⁻³ bis 10⁻¹, also
56–96 dB in Bells Maß `[eigene Rechnung]`. Einzelwerte lassen sich aus der Graustufe nicht ablesen.

**Ob die Harfe besaitet war, sagt der Text nicht.** Das Instrument war eine fertige Harfe, die eine Harfenistin zur
Verfügung stellte. Abb. 4 zeigt die Saitenorte und ihre Grundfrequenzen als Punkte. Es gibt keinen Satz zur Dämpfung
der Saiten.

### 1.4 Gleiche Größe, gleicher Bezug?

| | Bell 1988 | Le Carrou 2006/2007 | Waltham 2008 |
|---|---|---|---|
| Größe | input admittance = Betrag der Punkt-Admittanz | |ẇ/F|, Betrag der Punkt-Mobilität (Punkt 34); Abb. 3.3 ist ein Transfer | driving-point admittance |
| Einheit | s/kg | m s⁻¹ N⁻¹ (= s/kg) | s/kg |
| dB-Bezug | 0 dB = 1,58·10⁻⁶ s/kg, 116 dB = 1 s/kg (S. 133, S197 §11) | linear (Abb. 3.5); Abb. 3.3 „Ref 1dB: 5×10⁻⁸“; Abb. 3.8 ohne Bezug | logarithmische Graustufe |
| Anregung | Vibrator mit Stift und Impedanzkopf auf der Deckleiste; besaitet bis 10° schräg, etwas neben der Mitte | Shaker mit Stab von hinten, Kraftaufnehmer geklebt; Schnelle per Laser von vorn | Hammer, Beschleunigungsaufnehmer 0,2 g |
| Saiten | Plastikbänder, „heavily damped“ | Papier | nicht genannt |

Umrechnung: L_Bell = 20·log₁₀(Y / 1 s/kg) + 116,0 dB `[eigene Rechnung, aus Bells Bezug]`. Bei Le Carrou Abb. 3.3:
L_Bell = L_LC − 30,0 dB, falls „Ref 1dB“ für 0 dB steht. Beide meinen den **Betrag**; einen Realteil nennt keine der
Quellen.

**Der Widerspruch bleibt nach der Umrechnung bestehen.** Am Ort der Saiten 33/32 (Bell) bzw. am Punkt 34 zwischen
A♭ und B♭ (Le Carrou), jeweils bei der Frequenz der Saite:

| f [Hz] | Bell Abb. 7.22, Salvi, besaitet | Le Carrou Abb. 3.5, Camac, besaitet | Differenz |
|---|---|---|---|
| 104 | 80 dB (Saite 33) | 52 dB | 28 dB |
| ≈ 117 | 89 dB (Saite 32) | 56 dB | 33 dB |
| ≈ 207 (Spitze) | 78–86 dB (Abb. 7.9 besaitet, S197 §14) | 71 dB | 7–15 dB |

Die Zahlen aus Bell sind `[am Bild abgelesen]` in S197. Bell ordnet Saite 33 der Frequenz 104 Hz zu (S197 §14). Die
Zuordnung von Saite 32 zu rund 117 Hz ist `[eigene Einschätzung]` aus der Tonfolge.

### 1.5 Erklärungen in der Literatur, warum Saiten die Admittanz verändern

| Wer, wo | Aussage | Zahl | Marke |
|---|---|---|---|
| Bell 1988, S. 150 | Besaitet: Spitzen niedriger, Abstand Spitze–Tal kleiner, „more even“, steigt langsam über f | keine Zahl im Text; Abb. 7.6 (S197) | [wörtlich] |
| Firth (Bells Ref. [4]), zitiert bei Bell S. 147 | Abstand Spitze–Tal nimmt beim Besaiten ab | keine | [wörtlich, nur als Zitat bei Bell; Firth nicht gelesen] |
| Bell 1988, S. 150 | Kasten an der Harfe (noch ohne Saiten): obere Hälfte über 500 Hz etwas höher | „slight“ | [wörtlich] |
| Bell 1988, S. 150 | S(1,1)− am losen Kasten: „the entire treble end of the box moving in opposition“; an der Harfe „hinder[s] the motion of the entire box“ | 153 Hz am losen Kasten G | [wörtlich] |
| Bell 1988, S. 155 (Suchtreffer) | Mode des ganzen Körpers: Kasten und Säule gegen den Hals | „85Hz on the unstrung harp and 92Hz on the strung harp“ | [wörtlich, Kontext 250 Zeichen] |
| Le Carrou 2006, S. 50 | Mode 1 (54,8 Hz): Bewegung des ganzen Körpers, „depending on its connections to the arm and to the bottom of the pillar“ | 54,8 Hz, ζ 5,5 % | [wörtlich + am Bild] |
| Le Carrou 2006, S. 50 | Moden 4 und 6 kennt man an der unbesaiteten Harfe (Bell) und an einer besaiteten spanischen Harfe (Firth 1989); „when the harp is strung it seems difficult to extract these two similar mode shapes [Bell, 87]“ | – | [wörtlich] |
| Le Carrou 2006, Kap. 7, und 2009 | mitschwingende Saiten (Moden Saite–Saite), gemessen mit der Beschleunigung am Boden zwischen den Saiten 30 und 31; Konfiguration (2) „alle Saiten gedämpft außer 31“ ändert Dämpfung und Frequenz der Komponenten | 2009 S. 19: höchstens 0,5 % in f, bis 30 % in der Dämpfung (Suchtreffer) | [wörtlich, Suchtreffer] |

**Was keine der gelesenen Quellen erklärt:** warum bei gedämpften Saiten die *Täler* steigen (Bell) und Le Carrou an
einer besaiteten Harfe 25–33 dB tiefer misst. Mögliche Ursachen, alle **`[eigene Einschätzung]`, ungeprüft**:

- **(a) Anderer Bau.** Le Carrou 1.2.2: Kohlefaser und eine Aluminiumschiene auf der Mittellinie. Das macht die
  Saitenleiste steifer, die Mobilität am Saitenort also tiefer.
- **(b) Die Dämpfungsbänder.** Die gewebten Bänder verbinden die Saiten untereinander und mit dem Boden. Bell maß
  besaitet außerdem etwas neben der Mittellinie.
- **(c) Der Impedanzkopf.** Ohne Kompensation der Masse unter dem Kraftaufnehmer verfälscht er die Werte. Bell gibt
  dazu in den gelesenen Stellen nichts an; Kap. 6.3 ist nicht gelesen.

Der Probe in S197 §11 zufolge kann ein reiner Widerstand je Saite die Täler nicht heben. Eine parallel zugeschaltete
Last senkt die Punkt-Admittanz im Allgemeinen `[eigene Einschätzung]`.

## 2 Das Diskantende des Bodens

| Instrument | Angabe | Quelle | Marke |
|---|---|---|---|
| Salvi Aurora | „around 1.4m in length, 0.5m wide at the base, 0.1m at the top, and of thickness varying from 11-12mm at the bottom (bass) end to 2-2.5mm at the top (treble). It is made of strips of spruce (Sitka or Engelmann) between 3 to 8 cm wide bonded together, and covered with a thin veneer, typically also of spruce.“ | Waltham et al. 2008, S. 52, Abschn. 1 (PDF 1); Quelle dort: Waltham & Kotlicki 2008 | [wörtlich] |
| allgemein | „The highest strings a few cm from the top“ | Waltham 2008, S. 52 | [wörtlich] |
| Camac (Konzertharfe) | „we taper our soundboards from 10mm in the bass register of a concert harp, to 2mm at the top“; Fichte; Furnier; „reinforcing bars“ und „soundboard ribs“ | Camac-Blog „Soundboards“, 2015-04-01, https://www.camac-harps.com/blog/en/news/soundboards/ | [Werkzeug] (WebFetch zitiert Wortlaut) |
| Camac Atlantide Prestige | Vorderteil aus Western Red Cedar quer zur Faser, außen Ahornfurnier quer; innen „éclisses“ aus Oregon-Kiefer auf Kohlefaser (Epoxid); dazwischen Kohlefaser mit doppeltem Ahornfurnier (längs und quer); Gegenleiste aus Fichte, an der die Saiten hängen; darunter eine Aluminiumschiene „permet de rigidifier l'ensemble de l'axe“ | Le Carrou 2006, S. 17–18, §1.2.2, Abb. 1.6 (PDF 28–29) | [wörtlich]; keine Dicken angegeben |
| Camac Atlantide Prestige | Außenmaße: Kasten 1330 mm, Höhe 1708 mm, Schalllöcher (Tab. A.3) | Le Carrou 2006, Anh. A.2 (PDF 158) | [am Bild]; keine Dicke des Bodens |
| Camac Atlantide Prestige | „soundbox … 6-mm-thick semi-conical shell“, Luftvolumen 0,029 m³ | Le Carrou 2006, S. 48 | [wörtlich] |
| Salvi „Orchestra“ | „tapering thickness which varies from 12 mm at the bass to 2 mm at the treble … European Spruce“, Länge 136 cm | Bell & Firth, IOA-Proceedings, laut Suchmaschine | **[nur Zusammenfassung]** – Abruf HTTP 404 |
| Le Carrou, Modell | Der Boden verhält sich in Mode 4 wie eine Platte, „quasi-clamped“ (S. 50); in Kap. 7 als Balken „entre deux encastrements, l'un au niveau de la colonne et l'autre au niveau de la console“ (S. 124–125; Abb. 7.1 nennt Saite 11 als Grenze des Teils, der sich wie ein Balken biegt) | Le Carrou 2006 | [wörtlich]; Abb. 7.1 nicht am Bild geprüft |

**Hinweis für LumiViz:** S197 §12 rechnet mit „3 mm + Furnier“ am Diskant. Drei Quellen nennen 2–2,5 mm (Salvi Aurora,
Camac) bzw. 2 mm (Salvi „Orchestra“, nur als Zusammenfassung). Das ist gegen Bell Abb. 2.4/2.8 zu prüfen, die in
LumiViz schon gelesen sind.

**Nicht gefunden:** Halt am Kopfblock (Leim, Breite, Steifigkeit) sowie die Enden von Saitenleiste und Querleisten,
für keine Marke. Zu Lyon & Healy gibt es nichts.

## 3 Güte und Höhe der tiefen Moden an besaiteten Harfen

**Le Carrou 2006 (Camac, besaitet, Saiten mit Papier gedämpft).** Abb. 3.4 (PDF 62, Dämpfungsgrad ζ) und Tab. 3.2
(PDF 66, f aus der Antwort an Punkt 34) sind am Bild gelesen. Q = 1/(2ζ) ist `[eigene Rechnung]`; die Mobilität stammt
aus Abb. 3.5.

| Mode (Le Carrou) | f Abb. 3.4 [Hz] | f Tab. 3.2 [Hz] | ζ | Q | |ẇ₃₄/F| [10⁻³ s/kg] / Bells dB |
|---|---|---|---|---|---|
| 1 (ganzer Körper) | 54,8 | 61,5 | 5,5 % | 9 | 0,7 / 53 |
| 2 | 80,9 | 84,5 | 4,8 % | 10 | 2,0 / 62 |
| 3 | 123,4 | 124,5 | 2,5 % | 20 | 1,4 / 59 |
| 4 (T1 bzw. S(1,1)−) | 152,2 | 153,5 | 2,3 % | 22 | 6,8 / 73 |
| 5 (Nickmode) | 161,9 | – | 0,9 % | 56 | „not clearly present“ |
| 6 (A0) | 168,5 | 172 | 1,4 % | 36 | 4,6 / 69 |
| Spitze ohne Nummer | ≈ 207 | – | – | – | 5,8 / 71 |

Le Carrou 2010, S. 3205, ordnet in einer anderen Messung 148 und 166 Hz T1 und A0 zu `[wörtlich]`. Die Frequenzen
in Abb. 3.4 und Tab. 3.2 weichen um bis zu 7 Hz voneinander ab: zwei Auswertungen derselben Harfe.

**Bell 1988 (Salvi „Orchestra“).** S(1,1)− in der Harfe ohne Saiten: 170 Hz, Q 11. S(1,1)+: 216 Hz, Q 16;
besaitet 207 Hz, Q nicht bestimmbar. Diese Werte sind schon in S197 §14 belegt und werden hier nicht wiederholt. Neu
ist die Mode des ganzen Körpers bei 85 Hz ohne Saiten und 92 Hz mit Saiten (S. 155, `[wörtlich]`, Suchtreffer). Ein Q
der besaiteten S(1,1)− ist **nicht gefunden**; Abb. 7.8 ist nicht erneut am Bild gelesen.

## 4 Abstrahlung im Fernfeld je Kraft

**Le Carrou, Leclère & Gautier 2010, JASA 127, Abb. 2** (S. 3205, PDF 3; Datei
`LeCarrou_Leclere_Gautier_2010_HarpRadiation_JASA127.pdf`). Camac Atlantide Prestige, besaitet. Ob die Saiten gedämpft
waren, ist in den gelesenen Stellen nicht genannt.

Messaufbau `[wörtlich]`, S. 3204:

- halbreflexionsarmer Raum, 1000 m³, „valid frequency range of 70 Hz–20 kHz“;
- Mikrofone auf einem Bogen in 2,35 m von der Drehachse, ≥ 2 m vom Kasten;
- Shaker an S2 zwischen den Saiten 30 und 31 (138,6 / 123,5 Hz);
- Mikrofon „in the direction normal to the soundboard … at a height of 1.4 m“;
- „Ref 1 dB: 2 × 10⁻⁵ Pa/N“.

Die Messung reicht nur bis 1 kHz. Abgelesen am Bild auf ±2 dB; Pa/N ist `[eigene Rechnung]` aus
2·10⁻⁵·10^(L/20).

| f [Hz] | |P/F| [dB re 2·10⁻⁵ Pa/N] | Pa/N |
|---|---|---|
| 134 | 40 | 2,0·10⁻³ |
| 148 | 57 | 14·10⁻³ |
| 166 | 60 | 20·10⁻³ |
| ≈ 190–200 (Spitze) | 62 | 25·10⁻³ |
| ≈ 290 (Einbruch) | 26 | 0,4·10⁻³ |
| 384 | 58 | 16·10⁻³ |
| 460 | 50 | 6,3·10⁻³ |
| 582 | 46 | 4·10⁻³ |
| 644 | 54 | 10·10⁻³ |
| 688 | 41 | 2,2·10⁻³ |
| 756 | 53 | 9·10⁻³ |
| ≈ 850 (Einbruch) | 26 | 0,4·10⁻³ |
| 892 | 48 | 5·10⁻³ |
| 1000 | 40–44 | 2–3·10⁻³ |

Zwischen 300 und 1000 Hz schwankt der Pegel meist zwischen 40 und 55 dB. Die Richtcharakteristik in Abb. 3 umfasst
13 Frequenzen von 134 bis 892 Hz. S. 3205 `[wörtlich]`: „below 220 Hz, the concert harp is non-directional … above
220 Hz, more complex patterns with lobes appear“. Die Spitze von 62 dB passt zu Le Carrou 2007 Abb. 7 (27·10⁻³ Pa/N
bei 204 Hz, also 62,6 dB, S197 §14).

**Bell 1988, Kap. 8** (S. 163–166, Suchtreffer `[wörtlich]`):

- „The completed, strung harp … was mounted on a turntable … The strings were damped by plastic ribbons … excited by
  a vibrator and pin system“;
- angeregt mit weißem Rauschen, ausgewertet in Terzbändern, polare Darstellung.

**Einen Bezug je Kraft habe ich nicht gefunden**: Die Suche nach „force“ und „constant“ auf PDF 311–335 ergab keinen
Treffer, und fünf Seiten dort haben keine Textebene. Abstand und Raum für Kap. 8 sind nicht gelesen.

**Bis 2 kHz: nicht gefunden.**

## Reicht das für eine Korrektur des Körpers auf „besaitet“?

- **Belegt:**
  - Beide besaitet gemessenen Harfen wurden mit gedämpften Saiten gemessen; Größe und Einheit sind dieselben.
  - Bell misst an der besaiteten Salvi am Saitenort und bei der Frequenz der Saite 74–103 dB.
  - Le Carrou misst an der besaiteten Camac im Bass (Punkt 34) bis 300 Hz 48–73 dB.
  - Bells Harfe hat nach dem Besaiten niedrigere Spitzen und höhere Täler und steigt langsam an. Über 1 kHz zeigt
    Abb. 7.21 die Gestalt (Hochfläche bei 1–1,5 kHz, dann flacher Abfall), aber nur relativ.
  - Der Boden ist am Diskant laut zwei Quellen 2–2,5 mm dick.
  - Für die Camac gibt es Güten der tiefen Moden im besaiteten Zustand (Q 9–36).
- **Bliebe Annahme:**
  - jeder Mechanismus, der die Täler hebt (Abschnitt 1.5: keiner belegt);
  - der absolute Pegel von Abb. 7.21;
  - die Übertragung von der Camac (Kohlefaser, Aluminium) auf die Salvi (Fichte);
  - ob Abb. 7.22 mit gedämpften Saiten gemessen ist (wahrscheinlich, aber nicht ausdrücklich gesagt).
- **Offener Widerspruch:** 25–33 dB am selben Saitenort im Bass. Fachlich lässt er sich mit den gelesenen Quellen
  nicht entscheiden. Bell ist die einzige Messung an *derselben* Harfe in beiden Zuständen und am gleichen Bautyp wie
  das Modell; Le Carrou misst sauberer (Kraft direkt, Laser), aber an einer anders gebauten Harfe. Eine Korrektur, die
  sich nur auf Bells besaitete Werte stützt, wäre eine Anpassung an eine einzelne Messung ohne verstandenen
  Mechanismus.

## Nicht gefunden – Suchwege

- **Mobilität einer besaiteten Konzertharfe an den Saitenorten bis 5 kHz mit absolutem Bezug:**
  - Le Carrou 2006: Kap. 3, 4 und 7 sowie Anh. A–D (Suche nach „mobilité“, „point 34“, „sans cordes“, „cordée“,
    „pot vibrant“, „marteau“) – nur bis 300 Hz;
  - Le Carrou 2010 und 2009 (Suche nach „admittance“, „mobility“, „strung“);
  - Waltham 2008 (nur 0–1 kHz, Zustand nicht genannt);
  - Bell Abb. 7.21 (relativ).
  - Chadefaux 2012/2013 nicht durchgesehen.
- **Messung ohne Saiten an Le Carrous Harpe:** keine; Suche nach „sans cordes“ und „unstrung“ in der Thèse.
- **Erklärung, warum die Täler steigen:** Bell S. 147–159 (Text) gelesen; Firth (Bells Ref. [4]) nicht verfügbar.
- **Halt am Kopfblock, Enden der Leisten:** Le Carrou §1.2 und Anh. A; Waltham 2008; Camac-Blog; Websuche „concert
  harp soundboard thickness treble … Lyon Healy Salvi“.
- **Abstrahlung je Kraft über 1 kHz:** Le Carrou 2010 reicht bis 1 kHz; in Bell Kap. 8 kein Bezug gefunden.
- **Q der besaiteten S(1,1)− bei Bell:** nicht gesucht über S197 hinaus.

## Ladeliste (nicht geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Waltham_Kotlicki_2008_HarpSoundboards_JASA124.pdf` | https://doi.org/10.1121/1.2956479 | ? | JASA, hinter Bezahlschranke (zendy verlangt Login) | Messungen „at each stage of construction“, also womöglich besaitet gegen unbesaitet an derselben Harfe (laut Suchmaschinen-Zusammenfassung; [nur Zusammenfassung]) |
| `Bell_Firth_IOA_HarpSoundboardSoundbox.pdf` | https://www.ioa.org.uk/system/files/publications/AJ%20BELL%20I%20FIRTH%20VIBRATIONS%20OF%20THE%20CONCERT%20HARP%20SOUNDBOARD%20AND%20SOUNDBOX.pdf | ? | IOA-Proceedings; Abruf HTTP 404, andere URL suchen | Dicke des Salvi-Bodens 12→2 mm im Wortlaut |
| `Firth_1977_AcousticsOfHarp_Acustica.pdf` (Titel zu prüfen) | Bells Literaturliste, Ref. [4] Kap. 7 | ? | Acustica, Bibliothek | Abstand Spitze–Tal mit und ohne Saiten |
| `FA2023_Harp_ConductanceMap_000712.pdf` | https://dael.euracoustics.org/confs/fa2023/data/articles/000712.pdf | ? | Forum Acusticum 2023, offen | laut Suchmaschine: Konduktanz (Realteil der Mobilität) längs der Saitenösen, statische Last der Saiten beibehalten ([nur Zusammenfassung]) |
| `Daltrop_2009_HarpSoundbox_UBC.pdf` | über https://phas.ubc.ca (Honours Thesis) | ? | UBC | Messungen am Harfenkasten |
| `Daltrop_etal_2013_HarpSoundboxFEA_JASA.pdf` | https://doi.org/10.1121/1.4788052 | ? | JASA, Bezahlschranke | Kasten, Experiment und FE |

## Nicht geliefert / von der LumiViz-Sitzung nachzuprüfen

- **Noch zu messen bzw. zu lesen:**
  - Mobilitätswerte einer besaiteten Harfe mit absolutem Bezug über 300 Hz außerhalb von Bell fehlen;
  - Abstrahlung je Kraft über 1 kHz fehlt;
  - Maße zum Halt am Kopfblock fehlen.
- **Nachzuprüfen:**
  - Bell §6.3: ob die Masse am Impedanzkopf kompensiert ist (Ursache (c) in 1.5).
  - Ob Abb. 7.22 auf den Messungen mit gedämpften Saiten beruht.
  - Den Bezug von Abb. 7.21 über eine Stelle, an der Abb. 7.6 oder 7.22 denselben Messort und dieselbe Frequenz
    tragen. Der Bezug in diesem Bericht ist nur geschätzt.
  - Die Diskantdicke 2–2,5 mm gegen Bell Abb. 2.4/2.8 und gegen „3 mm + Furnier“ im Bauplan.
  - Le Carrou Abb. 3.3, „Ref 1dB: 5×10⁻⁸“: Ist das wirklich 0 dB?
  - Le Carrou Abb. 3.8: Die dB-Achse ist ohne Bezug und in sich nicht stimmig; nicht verwenden.
  - Ob die Zuordnung Bell-Saite 32 ≈ 117 Hz mit Tab. 2.1/2.2 übereinstimmt.

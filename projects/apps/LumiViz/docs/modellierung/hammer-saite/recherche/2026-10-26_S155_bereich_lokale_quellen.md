> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: alle sieben Groessen in den lokalen PDFs unter `quellen/`. Suche: nur lokal.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Lokale Quellen: belegte Bereiche für sieben Größen der Hammer-Saite

> **Typ:** Agentenbericht (Recherche-Agent, 2026-10-26), nur lokal vorhandene PDFs unter
> `docs/modellierung/hammer-saite/quellen/`, kein Netzzugriff.
> **Seitenangaben:** „PDF n“ = Seite im PDF; „gedr.“ = gedruckte Seite (vom Werkzeug vermutet, wo
> unplausibel von Hand aus Kopf-/Fußzeile nachgezogen und so gekennzeichnet).
> **Arten:** **gemessen** (Instrument genannt) · **angesetzt** (Parameter einer Simulation) ·
> **Aussage** (ohne Zahl oder Zahl nur aus zweiter Hand) · **abgeleitet** (von mir aus Zahlen der
> Quelle gerechnet — keine Angabe der Quelle).
> Bekanntes aus `recherche/2026-10-25_S153_*.md` (Cuenca-Mechanismus, Tan-Prüfstand qualitativ,
> Valiente-Modell) ist nur dort wiederholt, wo eine Zahl mit Seite dazukommt.

## 0. Reichweite

**Ordnerstand:** Beim Start 25 PDFs. Während der Arbeit (21:13–21:18) legte eine andere Sitzung
18 weitere ab; Endstand 21:19 = 43 PDFs. Alle 43 sind durchsucht. Eine Datei
(`Niimi_Yoshino_Yatabe_2026_…`) erschien kurz unvollständig (0 Seiten) und war beim Endstand wieder
weg — nicht durchsucht.

**Textebene:** Alle 43 haben eine. Eingeschränkt lesbar:
- `Baxandall_Brown_Gough_…_IOA.pdf` — Textebene teils verwürfelt (überlagerte Spalten), Abb. 3
  (Breite/Abklingzeit) nicht lesbar.
- `Chaigne_Askenfelt_1994_…_I_JASA_KopieKursseite.pdf` — OCR-artig, Wörter zusammengezogen;
  Gl. (3) nur verstümmelt.
- `Nakamura_1989_…_JASJ.pdf` — japanische Schriften fehlen (CMap), englischer Text lesbar.
- Griechische Buchstaben fehlen in fast allen Textebenen (RT-0425, Chabassier 2013, Valiente) —
  Bezeichner aus dem Zusammenhang erschlossen und so gekennzeichnet.
- Kein OCR gefahren.

**Suchbegriffe (regex, ohne Groß-/Kleinschreibung), über alle 25 Ausgangs-PDFs:**
`mistun`, `detun`, `d[ée]saccord`, `unison`, `[0-9.,]+ ?cents?`; `decay time`, `T60`, `dB/s`,
`decay rate`, `two-stage|double decay`, `aftersound`, `prompt sound`; `damping`, `amortissement`,
`b1 … b3`; `polari[sz]ation`, `horizontal (motion|vibration|direction)`; Winkel (Tan: `angle of`,
Grad); `uneven|unequal`, `simultaneous`, `hammer … (tilt|misalign|parallel)`,
`hammer … (each|all|two|three) strings`, `small differences in the excitation`, `voicing`;
`bridge pin`, `zig-?zag`, `side ?bearing`, `chevalet`, `pointes`; frz. `battement`,
`d[ée]croissance`, `frappe in[ée]gale`.
**Über die 18 nachgelieferten PDFs:** `decay`, `T60`, `dB/s`, `mistun|detun|unison`, `cents`,
`polari[sz]`, `angle`, `uneven|unequal`, `aftersound|two-stage|double decay`, `damping` /
`loss factor`; dazu gezielt gelesen: Bensa 2003 Tab. I/Gl. 37, Nakamura 1989 S. 289–291,
Woodhouse 2022 Abschn. II–III, Cerisano 2016 ganz, Burred 2004 Kap. 5.4, Karatsovis Kap. 2.4/4.2,
Aramaki 1999 ganz, Chaigne/Askenfelt 1994 Abschn. I.
**Kein Treffer ist kein Beweis** — Abbildungen (Kurven mit Zahlenwerten) sind fast nirgends als Text
lesbar; dort steckt viel, was hier fehlt (siehe §9).

---

## 1. `decay` — T60 des Grundtons, erste Stufe

**1a. Plath 2019, Diss.** (`Plath_2019_WorkshopToConcertHall_Diss.pdf`), PDF 202 / gedr. 168,
Abb. 12.11 (PDF 203, gedr. 169 von Hand) — **gemessen**, Konzertflügel „D1“ (Steinway-Nomenklatur;
Hersteller im Text nicht ausdrücklich, PDF 165 / gedr. 131: „A concert grand piano is used … (denoted
as D1“), Kunstkopf am Spielerplatz, Schroeder-Integration, **eine** Gerade über den ganzen Ton:
> „T60 values decrease from approx. 8 s to 10 s in the bass range to approx. 0.5 s in the treble range.“

Und zur Methode (ebd.): „assumes a single exponential decay“. Also weder reine erste noch zweite
Stufe, sondern eine Mischung; Gesamtschall, nicht Grundton allein.

**1b. Bensa et al. 2003** (`Bensa_et_al_2003_…_JASA_KopieKursseite.pdf`) — **gemessen**, Yamaha
Disklavier C6 (PDF 7 / S. 1101: „Yamaha Disklavier C6 grand piano“), Beschleunigung am Steg, Verlust
je Teilton aus der analytischen Hüllkurve:
- Tab. I (PDF 2 / S. 1096): b1 = 0,25 / 1,1 / 9,17 s⁻¹ für C2 / C4 / C7; b2 = 7,5·10⁻⁵ / 2,7·10⁻⁴ /
  2,1·10⁻³ m²s⁻¹; L = 1,23 / 0,63 / 0,10 m.
- Gl. (37a,b) (PDF 9 / S. 1103), Ausgleich über die Tastatur, linear in f0 (Minuszeichen fehlt in
  der Textebene, aus Tab. I bestätigt): **b1 = 4,4·10⁻³·f0 − 4·10⁻² s⁻¹**,
  **b2 = 1,0·10⁻⁶·f0 + 1·10⁻⁵ m²s⁻¹**.
- Für Mehrsaiter (PDF 9): „the calculated damping coefficients … can be thought of describing the
  global perceived decay of the sound.“ — also ebenfalls keine Trennung der Stufen.
- Modell (S. 1096): σ(β) = b1 + b2·β², β = nπ/L (Wellenzahl; „q = b1 + b2 β²“, Zeichen aus dem
  Zusammenhang).
- **abgeleitet** (ich, T60 = ln 1000 / σ, Teilton 1): C2 ≈ 27,6 s, C4 ≈ 6,2 s, C7 ≈ 0,61 s.

**1c. Karatsovis 2011, Diss.** (`Karatsovis_2011_PhD_Southampton_eprint333304.pdf`), PDF 103 /
gedr. 88 — **gemessen** an einer Aufnahme C3 aus der Bibliothek „Gigastudio“ (Instrument unbekannt,
PDF 102 / gedr. 87):
> „for the fundamental and first harmonic frequency there is a relatively fast decay up to about 2.2
> s and 3 s respectively, followed by a slower decay“ — für den 3. Teilton „about 0.2 s“.

Das ist die **Dauer** der ersten Stufe, kein T60; die Steigungen stehen nur in Abb. 4.3.

**1d. Nakamura 1989** (`Nakamura_1989_…_JASJ.pdf`), PDF 2 / gedr. 290 — **angesetzt** (Zitat einer
eigenen Simulation von 1981, C4, verglichen mit einem Klavier): bei ±1 Cent „a prompt sound with a
quick decay proceeds during the first 1 second“. Dauer, kein T60.

**1e. Hörschwelle für Abweichungen:** Bank 2006, Diss., PDF 58 / gedr. 44 — **Aussage mit Zahl aus
zweiter Hand** (Järveläinen & Tolonen 2001):
> „The overall decay time of the tone can be varied from -25 to +40% without audible consequences“

Für den Optimierer brauchbar als Toleranzband um einen Messwert.

**1f. Anschlagstärke:** Cheng/Dixon/Mauch 2015 (`Cheng_…_ICASSP.pdf`), PDF 4 — **gemessen**, RWC-Flügel,
p/m/f: „dynamics have no significant absolute effect on the decay rate“. Stützt `decay` unabhängig
von der Stufe.

**Ohne Zahl im Text, nur in Abbildungen:** Elie 2022 Abb. 8 (T15 C1–G♯7, simuliert), Bank 2000
Abb. 6.1 (A4), Bank 2006 Abb. 3.2 (F2), Lehtonen 2005 Abb. 4.3 (T60 je Teilton, Taste 3),
Jirasek 2024 Abb. 12/13 (Petrof), Burred 2004 Abb. 5.12 (Sofort- und Nachklang je Lage),
Chabassier 2013 Abb. 17 (C5, simuliert).

## 2. `lateDecay` — T60 der zweiten Stufe, Verhältnis spät/früh

**2a. Tan 2017, Diss.** (`Tan_2017_…_tel-01755039.pdf`), Abb. 7.14, PDF 167 / gedr. 154 — **gemessen**
am Prüfstand (Stück eines Bassstegs Pleyel P-131, eine Saite im Zickzack über die Stegstifte, gezupft;
Saite nach Tab. 7.2: L = 0,608 m, d = 1,3 mm, T0 = 330,161 N → f ≈ 146 Hz **abgeleitet**).
Ausgleichsgeraden der RMS-Hüllkurve in dB (Text der Abbildung):
> vertikal angeregt: „urms = -3.2t - 20.6“ und „urms = -1.4t - 23.6“;
> horizontal angeregt: „urms = -2.7t - 20.9“ und „urms = -1.1t - 24.3“

**abgeleitet:** T60 früh 18,8 s / spät 42,9 s (vertikal), 22,2 s / 54,5 s (horizontal);
Verhältnis spät/früh **2,3 bzw. 2,5**. Ohne Resonanzboden, eine Saite, also nur der
Polarisations-Mechanismus.

**2b. Tan 2017, Simulation** (Abb. 7.17–7.21, PDF 172–175 / gedr. 159–162) — **angesetzt**:
Steigungspaare z. B. −1,8/−1,4 dB/s (γ = 7°), −2,7/−1,4 dB/s (γ = 22°), −1,7/−1,2 dB/s (Verstimmung
0,4 Hz), −1,1/−0,9 dB/s (horizontal, γ = 7°); Verhältnis spät/früh **1,2–1,9**.

**2c. Baxandall/Brown/Gough** (`Baxandall_…_IOA.pdf`), PDF 1 / gedr. 331 — **Aussage** (Hundley et
al. referiert): bei einem Dreichor „a decay rate three times faster than that of a single string“ —
das ist der Faktor N der Gleichlage im Modell (§5 `modell.md`), keine Zahl für die zweite Stufe.

**2d. Woodhouse 2022** (`Woodhouse_2022_…_JASA_AAM.pdf`), PDF 6 — **gemessen** (Steg-Admittanz eines
Broadwood-Stutzflügels an A4, PDF 4) und daraus **berechnet**: mit drei Saiten liegt der
Steg-Verlustfaktor „above [air] by some 20 dB over the entire frequency range above about 160 Hz“.
Daraus folgt nur eine **Grenze**: die zweite Stufe (Gegenlage, „no energy … to the body“) ist durch
die inneren Verluste der Saite (Luft, Viskoelastik) nach unten begrenzt — `lateDecay` ≤ T60 der Saite
allein (§3c–3e). Keine T60-Zahl.

**2e. Ohne Zahl:** Lehtonen 2005 PDF 16 / gedr. 8 („after some milliseconds the decay rate changes“),
Bank 2000 PDF 23, Burred 2004 PDF 31 / gedr. 29 („Double decay is especially noticeable in the middle
register … In the highest notes, the decay is made up of just one continuous curve“), Cuenca 2007
Abb. 3–4 (nur Bild), Plath Abb. 2.8 (Hochgeschwindigkeitskamera, PDF 56, nur Bild).

## 3. `highDecay` — T60(10·f1)/T60(f1); Verlustgesetz σ(ω) = b1 + b3·ω²

**3a. Bensa 2003 — gemessen (Yamaha C6), Gesetz und Werte s. 1b.** Mit β1 = π/L:
**abgeleitet** (ich, Teilton 10 statt 10·f1, Steifigkeit vernachlässigt — im Diskant liegt 10·f1
unter Teilton 10, das Verhältnis wird dort größer):
| Taste | σ1 [s⁻¹] | σ10 [s⁻¹] | T60(f1) | `highDecay` ≈ σ1/σ10 |
|---|---|---|---|---|
| C2 (36) | 0,2505 | 0,299 | 27,6 s | **0,84** |
| C4 (60) | 1,107 | 1,771 | 6,2 s | **0,62** |
| C7 (96) | 11,24 | 216,4 | 0,61 s | **0,052** |

Umrechnung in die Form `b3·ω²`: b3 = b2/c² (c = 160,9 / 329,6 / 418,6 m/s aus Tab. I) —
**abgeleitet**: b3 ≈ 2,9·10⁻⁹ / 2,5·10⁻⁹ / 1,2·10⁻⁸ s.

**3b. Chabassier & Duruflé 2012, RT-0425** (`…_RT0425.pdf`), Gl. (6), PDF 12 / gedr. 9 —
**angesetzt** für alle 88 Saiten (Steinway-D-Nachbau), mit dem Satz:
> „The damping coefficients are difficult to tune (lack of measurements, difficulty to dissociate the
> origins of damping).“

R_u,i = (α·i + β)₊ ∈ [0; 0,42] s⁻¹, η_u,i = γ·i + δ ∈ [1,6·10⁻⁹; 3,9·10⁻⁹] s, mit α = 5·10⁻³,
β = −0,015, γ = 2,78·10⁻¹¹, δ = 1,5274·10⁻⁹ (i = Taste 1…88; Bezeichner R, η aus dem Zusammenhang,
die Textebene verliert die Griechen). Bestätigt durch Anhang A (z. B. C1 = Taste 4: 5,0·10⁻³ und
1,6·10⁻⁹). Gesetz (Chabassier/Joly/Chaigne 2013, Gl. 8, PDF 11 / gedr. 10): „a constant term … and a
quadratic term“; **abgeleitet** σ(ω) = R_u + η_u·ω². Achtung: das sind die Verluste der Saite
**ohne** Resonanzboden (der wird dort eigens gerechnet).

**3c. Tan 2017 — gemessen an einer Monochord-Saite** (kein Flügel), Tab. 4.3, PDF 94 / gedr. 81,
Gl. 2.136, PDF 60 / gedr. 47: R_u = 0,5067 s⁻¹, η_u = 3,8·10⁻⁹ s („experimentally determined“);
f1 = 187,54 Hz (Tab. 4.1, PDF 92 / gedr. 79). Mit Stegstück (Gl. 7.1, PDF 158 / gedr. 145,
„custom fit“): R = 0,0957 s⁻¹, η = 7·10⁻⁹ s.

**3d. Miranda Valiente 2024** (`MirandaValiente_…_JPhysConfSer.pdf`), PDF 4 — **angesetzt, aus
Messung**: „estimated through measurements, a constant damping ratio of … 5·10⁻⁵ is adopted“
(Zeichen ζ fehlt in der Textebene). Frequenzunabhängiges ζ ⇒ σ = ζ·ω (abgeleitet), also ein
anderes Gesetz als b1 + b3ω²; an C2 **abgeleitet** T60 ≈ 340 s (Saite allein).

**3e. Woodhouse 2022**, PDF 2 — **Aussage** (Formeln ohne Klavierzahlen): innere Dämpfung
„dominated by air damping for low-order modes“, Biegeverlust wächst mit der Ordnung.

**3f. Form ohne Zahlen:** Bank/Zambon/Fontana 2010 Gl. (7), PDF 3 („1/τ_k = b1 + b3·(2πf_k)²“;
„Example values … can be found in [21]“ = Bensa 2003, s. 3a); Bank 2006 Gl. 3.8, PDF 55 / gedr. 41
(„b1 = 1/τ0 and b3 = b1/(2π f1/2)²“; T60 fällt oberhalb f1/2 mit 12 dB/Dekade);
Chaigne/Askenfelt 1994 Gl. (3) (nur verstümmelt; „derived from experimental values“, keine Werte in
Teil I).

## 4. Verstimmung im Chor (`detune`, Abstand der äußeren Saiten)

**4a. Plath 2019, Diss.**, Abb. 12.5, PDF 194 / gedr. 160 — **gemessen**, Konzertflügel D1, Tasten
42–88 (= MIDI 62–108), Spitzen um f1, Mittel über drei Saitenpaare, fünf Anschläge:
> „Up to F6 the average detuning is in the range proposed by Kirk with differences between strings of
> approx. 1-2 cent. In the high treble range differences of up to 5 cent appear.“

Messgrenze (PDF 194): Auflösung 0,25 Hz, deshalb erst ab Taste 42. **Achtung Größe:** Mittel der drei
Paarabstände, nicht Abstand der äußeren; bei gleichmäßig gespreiztem Dreichor (−c/2, 0, +c/2) ist der
äußere Abstand das **1,5-fache** des Mittels (abgeleitet) → ≈ 1,5–3 Cent bis F6, bis ≈ 7,5 Cent ganz
oben.

**4b. Plath 2019** (Diss. PDF 161 / gedr. 127 und PDF 193 / gedr. 159 von Hand; ISMA-Fassung PDF 4) —
**Aussage aus zweiter Hand** (Kirk 1959): „Technicians usually detune the unison strings by 1-2 cents“.

**4c. Nakamura 1989**, PDF 1 / gedr. 289 — **Aussage aus zweiter Hand** (Kirk 1959): „the most favored
amount of mistuning is 1 or 2 cents among strings of a note“; eigene Simulationen mit 1, 5, 10 Cent
(PDF 2–5, **angesetzt**).

**4d. Cuenca & Caussé 2007**, PDF 7 / gedr. 6 — **Aussage** (ohne Beleg im Text): „Differences in
fundamental frequency between strings in a such tuned piano are situated in the range of 0.5 to 4
cents“; Simulation D♯4 mit 2 und 4 Cent (**angesetzt**).

**4e. Karatsovis 2011**, PDF 46 / gedr. 31 — **Aussage**: „slightly mistuned from each other by much
less than 1 Hz“; Simulation mit ≈ 0,2 Hz (PDF 101 / gedr. 86, **angesetzt**).

**4f. Aramaki et al. 1999** (`Aramaki_…_DAFx99.pdf`), PDF 3 — **gemessen** (Laservibrometrie an zwei
gekoppelten Saiten, Instrument nicht genannt): „The frequency gap is typically in the range [0.1Hz ;
5Hz]“ — Abstand der **gekoppelten** Dublett-Teiltöne, nicht die Verstimmung der freien Saiten.

**4g. Am Rand:** Cerisano 2016 (`Cerisano_2016_UnisonDrift_PTJ.pdf`, PDF 7–8) misst an Uprights und
einem Weber-Flügel die Tonhöhe des ganzen Chors gegen die Einzelsaite: „as much as two cents sharp or
flat“, Schwanken „up to 3.0 cents“ — Verschiebung durch Kopplung, nicht die Verstimmung selbst.
Plath ISMA 2019 PDF 8 („approx 2.5 cent“, „up to 10 cent“) betrifft die Stimmung **zwischen zwei
Zuständen**, nicht den Chor.

**Ohne Wert:** Chabassier 2013 PDF 12 („slightly detuned“, nur über T0,i), Valiente 2024 PDF 3 („differing
only in tension“), Elie 2022 PDF 15 („detuning factor“).

## 5. γ — Winkel der ersten Eigenrichtung gegen die Senkrechte

**5a. Tan 2017, Monochord** (Klemmen an beiden Enden, kein Steg), §4.5, PDF 99 / gedr. 86 —
**gemessen**: „The two identified polarisation angles are +5° and -85° respectively.“

**5b. Tan 2017, mit Stegstück** (Pleyel-Bassteg, Zickzack), PDF 161 / gedr. 148 und §7.4, PDF 176 /
gedr. 163 — **gemessen**: Eigenrichtungen bei +38° und −53°; „it has an angular offset … of +7°
between the string axes and the bridge axes“.

**5c. Tan 2017, Simulation**, Tab. 7.2, PDF 171 / gedr. 158 — **angesetzt**: „Angular offset … 7°“;
für eine deutliche zweite Stufe „artificially increased from +7° to +22°“ (PDF 176 / gedr. 163).

**5d. Burred 2004** (`Burred_2004_AcousticsOfThePiano.pdf`), PDF 31 / gedr. 29 — **Aussage aus
zweiter Hand** (Weinreich 1977): „Immediately after impact, the vertical component is more than ten
times the horizontal component“. **abgeleitet:** wäre das allein Geometrie, entspräche es
γ ≲ arctan(0,1) ≈ 6°.

**5e. Baxandall/Brown/Gough**, PDF 1 / gedr. 331 — **Aussage**, Steinway (Messung): die Moden sind
„generally not strictly parallel and perpendicular to the plane of the soundboard“. Zahl in der
Textebene nicht lesbar.

**5f. Gegenstimme:** Nakamura 1989, PDF 1 — referiert Hundley et al. 1978: „the vertical component
dominated the vibration throughout the duration of the tone“.

## 6. δ — Frequenzabstand der zwei Polarisationen

**6a. Tan 2017, Monochord**, PDF 99 / gedr. 86 — **gemessen**: „The detuning between the two modes is
found to be between 0.1Hz and 0.2Hz.“ Bei f1 = 187,54 Hz (Tab. 4.1) **abgeleitet** 0,9–1,8 Cent.

**6b. Tan 2017, mit Stegstück**, PDF 161 / gedr. 148 — **gemessen**: „a strongly observed detuning of
0.2Hz“; je nach Definition „could range between 0.2Hz to 0.4Hz“. Bei f ≈ 146 Hz (Tab. 7.2)
**abgeleitet** 2,4–4,7 Cent.

**6c. Simionato 2024** (`Simionato_2024_…_arXiv2409.06513.pdf`), PDF 3 — **Aussage aus zweiter Hand**
(Tan 2017): „usually detuned with respect to the vertical one (±0.1 or 0.2 Hz …)“.

**6d. Ohne Zahl:** Tan/Touzé/Cotté 2015 PDF 1 („known to have close values“); Baxandall
(„difference in frequency of the "coupled" and "uncoupled" modes“ wird gemessen, Wert nur in der
unlesbaren Abb. 3).

## 7. u — ungleicher Stoß des Hammers auf die Saiten eines Chors

**Keine Zahl in keiner Quelle.** Nur Aussagen:
- Bank 2000, PDF 29 / gedr. 29: „Because of the unevenness of the felt the hammer compliance for the
  distinct strings can be dissimilar“.
- Lehtonen 2005 (`Lehtonen_2005_…_MSc_TKK.pdf`), PDF 16 / gedr. 8, nach Weinreich: „one string of the
  string group can be hit harder than another“.
- Burred 2004, PDF 30 / gedr. 28: „on impact one string has a slightly greater amplitude than the
  other“ (Weinreich-Bild, Abb. 5.9).
- Miranda Valiente 2024, PDF 3: „due to small differences in the excitation and due to the mistuning“.
- Chabassier/Joly/Chaigne 2013, PDF 13 / gedr. 12: Hammerfeder K_H^i je Saite, „the model can
  eventually account for the fact that the interaction with the hammer might not be uniform for each
  string of a note“ — Möglichkeit, kein Wert.
- Zeitversatz: nirgends für die Saiten eines Chors. (Jirasek 2024 PDF 3 handelt vom Versatz der
  Töne eines Akkords, nicht eines Chors.)

---

## 8. Übersicht

| Größe | Register / MIDI | Wert / Bereich | Art | Instrument | Datei, Seite | Zitat-Kurzform |
|---|---|---|---|---|---|---|
| decay | Bass → Diskant (alle 88) | ≈ 8–10 s → ≈ 0,5 s (einstufig gefittet) | gemessen | Konzertflügel D1 (Steinway-Nomenklatur) | Plath Diss., PDF 202 / S. 168, Abb. 12.11 | „approx. 8 s to 10 s in the bass … 0.5 s in the treble“ |
| decay | C2 / C4 / C7 (36/60/96) | b1 = 0,25 / 1,1 / 9,17 s⁻¹ → T60 ≈ 27,6 / 6,2 / 0,61 s (abgel.) | gemessen | Yamaha Disklavier C6 | Bensa 2003, Tab. I, PDF 2 / S. 1096 | „Physical model parameters for piano tones C2, C4, and C7“ |
| decay | ganze Tastatur | b1 = 4,4·10⁻³·f0 − 0,04 s⁻¹ | gemessen (Ausgleich) | Yamaha C6 | Bensa 2003, Gl. 37a, PDF 9 / S. 1103 | „b1 and b2 are both increasing functions“ |
| decay | C3 (48) | erste Stufe endet ≈ 2,2 s (T1), 3 s (T2), 0,2 s (T3) | gemessen | Sample-Bibliothek, Instrument unbekannt | Karatsovis, PDF 103 / S. 88 | „fast decay up to about 2.2 s and 3 s“ |
| decay | — | −25 % … +40 % unhörbar | Aussage (Järveläinen) | — | Bank 2006, PDF 58 / S. 44 | „varied from -25 to +40% without audible consequences“ |
| decay | alle | kein Einfluss der Dynamik | gemessen | RWC-Flügel | Cheng 2015, PDF 4 | „no significant absolute effect on the decay rate“ |
| lateDecay | ≈ 146 Hz (≈ D3) | −3,2 → −1,4 dB/s und −2,7 → −1,1 dB/s; spät/früh 2,3–2,5 (abgel.) | gemessen | Prüfstand, Pleyel-Bassteg, 1 Saite | Tan 2017, Abb. 7.14, PDF 167 / S. 154 | „urms = -3.2t … -1.4t“ |
| lateDecay | ≈ 146 Hz | spät/früh 1,2–1,9 | angesetzt | Simulation desselben Prüfstands | Tan 2017, Abb. 7.17–7.21, PDF 172–175 | „urms = -1.8t … -1.4t“ |
| lateDecay | ab ≈ 160 Hz | Steg-Verlust 20 dB über Luftdämpfung ⇒ spät ≤ Saite allein | gemessen (Admittanz) + Rechnung | Broadwood-Stutzflügel, A4 | Woodhouse 2022, PDF 6 | „above [air] by some 20 dB“ |
| highDecay | C2 / C4 / C7 | ≈ 0,84 / 0,62 / 0,05 (abgel. aus b1, b2) | gemessen | Yamaha C6 | Bensa 2003, Tab. I + Gl. 37b | „b2 … 7.5·10⁻⁵ … 2.1·10⁻³“ |
| highDecay | ganze Tastatur | b2 = 1,0·10⁻⁶·f0 + 10⁻⁵ m²/s; b3 = b2/c² ≈ 2,5·10⁻⁹…1,2·10⁻⁸ s (abgel.) | gemessen (Ausgleich) | Yamaha C6 | Bensa 2003, Gl. 37b, PDF 9 / S. 1103 | s. o. |
| highDecay | Taste 1–88 | R ∈ [0; 0,42] s⁻¹, η ∈ [1,6; 3,9]·10⁻⁹ s, σ = R + η·ω² | angesetzt (Saite ohne Boden) | Steinway-D-Nachbau (virtuell) | RT-0425, Gl. 6, PDF 12 / S. 9 | „difficult to tune (lack of measurements …)“ |
| highDecay | 187,5 Hz | R = 0,5067 s⁻¹, η = 3,8·10⁻⁹ s; mit Steg R = 0,0957, η = 7·10⁻⁹ | gemessen | Monochord-Saite | Tan 2017, Tab. 4.3 PDF 94 / S. 81; Gl. 7.1 PDF 158 / S. 145 | „experimentally determined“ |
| highDecay | C2 | ζ = 5·10⁻⁵ konstant (σ ∝ ω) | angesetzt aus Messung | Saite (Labor) | Valiente 2024, PDF 4 | „constant damping ratio … 5·10⁻⁵“ |
| detune | MIDI 62–89 (Taste 42 bis F6) | Mittel der Paare ≈ 1–2 Cent (äußere ≈ 1,5–3, abgel.) | gemessen | Konzertflügel D1 | Plath Diss., Abb. 12.5, PDF 194 / S. 160 | „approx. 1-2 cent“ |
| detune | MIDI 90–108 | Mittel der Paare bis 5 Cent (äußere bis ≈ 7,5, abgel.) | gemessen | Konzertflügel D1 | ebd. | „up to 5 cent“ |
| detune | — | 1–2 Cent | Aussage (Kirk 1959) | — | Plath S. 127/159; Nakamura PDF 1 / S. 289 | „1 or 2 cents among strings“ |
| detune | — | 0,5–4 Cent | Aussage | — | Cuenca 2007, PDF 7 / S. 6 | „range of 0.5 to 4 cents“ |
| detune | D♯4 | 2 und 4 Cent | angesetzt | Simulation | Cuenca 2007, PDF 7 / S. 6 | „2 and 4 cents mistunings“ |
| detune | — | ≪ 1 Hz | Aussage | — | Karatsovis, PDF 46 / S. 31 | „much less than 1 Hz“ |
| detune (gekoppelt) | ≈ 157 Hz (3. Teilton 470 Hz) | Dublett-Abstand 0,1–5 Hz | gemessen | 2 Saiten, Instrument ungenannt | Aramaki 1999, PDF 3 | „typically in the range [0.1Hz ; 5Hz]“ |
| γ | 187,5 Hz | +5° / −85° | gemessen | Monochord (ohne Steg) | Tan 2017, PDF 99 / S. 86 | „+5° and -85°“ |
| γ | ≈ 146 Hz | Eigenrichtungen +38° / −53°; Versatz Saite–Steg +7° | gemessen | Prüfstand, Pleyel-Bassteg | Tan 2017, PDF 161 / S. 148; PDF 176 / S. 163 | „angular offset … of +7°“ |
| γ | ≈ 146 Hz | 7° (wie gemessen), 22° für Bild der Messung | angesetzt | Simulation | Tan 2017, Tab. 7.2 PDF 171 / S. 158 | „from +7° to +22°“ |
| γ | — | Amplitude vertikal > 10 × horizontal ⇒ ≲ 6° (abgel.) | Aussage (Weinreich) | — | Burred 2004, PDF 31 / S. 29 | „more than ten times the horizontal“ |
| γ | — | nicht genau parallel/senkrecht zum Boden | Aussage aus Messung | Steinway | Baxandall et al., PDF 1 / S. 331 | „not strictly parallel and perpendicular“ |
| δ | 187,5 Hz | 0,1–0,2 Hz ≈ 0,9–1,8 Cent (abgel.) | gemessen | Monochord | Tan 2017, PDF 99 / S. 86 | „between 0.1Hz and 0.2Hz“ |
| δ | ≈ 146 Hz | 0,2 (bis 0,4) Hz ≈ 2,4–4,7 Cent (abgel.) | gemessen | Prüfstand, Pleyel-Bassteg | Tan 2017, PDF 161 / S. 148 | „detuning of 0.2Hz“ |
| u | — | **keine Zahl** | Aussage | — | Bank 2000 PDF 29; Lehtonen PDF 16 / S. 8; Burred PDF 30 / S. 28; Chabassier 2013 PDF 13 / S. 12 | „one string … can be hit harder than another“ |

## 9. Offene Punkte

1. **Zwei Stufen getrennt am Flügel gemessen** — in den lokalen Quellen nirgends als Zahl je Taste.
   Plath und Bensa fitten einstufig; Tan trennt sauber, aber am Prüfstand mit einer Saite. Die
   Zahlen stecken wahrscheinlich in Abbildungen (Burred Abb. 5.12 nach Quelle [14], Lehtonen
   Abb. 4.3, Karatsovis Abb. 4.3, Plath Abb. 12.11) — ablesen nur mit Bild, hier nicht getan.
2. **Bensa gegen Plath im Bass:** C2 abgeleitet 27,6 s (Steg, Teilton 1) gegen 8–10 s (Schall,
   Gesamtton, Schroeder). Verschiedene Größen (Steg-Beschleunigung je Teilton vs. Breitband-EDC) —
   nicht als Widerspruch lesen, aber für die Grenzen des Optimierers entscheiden, welche Größe
   `decay` meint.
3. **`detune` als Abstand der äußeren Saiten** ist nirgends direkt angegeben; Plath mittelt die drei
   Paare. Der Faktor 1,5 gilt nur für gleichmäßige Spreizung. Unterhalb Taste 42 (MIDI 62) keine
   Messung (Auflösung 0,25 Hz).
4. **γ und δ am echten Flügel** fehlen; Tans Werte stammen von Monochord und einem ausgebauten
   Stegstück. Baxandall et al. haben am Steinway gemessen, die Zahlen stehen nur in der unlesbaren
   Abb. 3 (Textebene verwürfelt; Abhilfe: Bild ansehen, kein OCR ohne Auftrag).
5. **u:** keine einzige Zahl, weder Kraft- noch Zeitunterschied. Wer eine Grenze braucht, muss sie
   setzen und als ungesichert kennzeichnen.
6. **Primärquellen fehlen lokal:** Weinreich 1977, Hundley/Benioff/Martin 1978, Kirk 1959,
   Martin 1947, Meyer & Melka 1983, Chaigne/Askenfelt 1994 Teil II (dort b1/b3 für C2/C4/C7),
   Bensa 2003 Abb. 13 (b1, b2 je Taste — nur die Ausgleichsgeraden sind Text).
7. **Seitenzahlen:** bei Plath rät das Werkzeug teils falsch (PDF 193 → „6“, PDF 203 → „14“); hier
   von Hand nach Folgeseite gesetzt (159, 169). Bensa: Kopfzeilen PDF 2 = S. 1096, PDF 7 = S. 1101,
   PDF 9 = S. 1103.

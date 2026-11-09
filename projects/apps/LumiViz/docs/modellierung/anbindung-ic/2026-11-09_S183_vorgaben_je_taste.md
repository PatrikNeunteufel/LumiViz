# Herleitung S183: Vorgaben je Taste — Wurlitzer, Kalimba, Clavinet (LP2 b)

> **Datum:** 2026-11-09 (Session 183, Fable) · **Anlass:** Befund S182 (`2026-11-09_S182_pegel_ueber_den_umfang.md`):
> Gefälle der Lautheit über den Umfang aus Bauwerten, die über die Tasten fest sind · **Auftrag:** LP2 b — die Vorgaben je
> Taste mit Quellen herleiten · **Stand:** Herleitung und Proben fertig, Entscheide WZ1, KA1, CL1 bei Patrik.
> **Proben:** Skripte `herleitung.py`, `schlitz_abschaetzung.py` im Scratchpad der Sitzung (nicht im Repo, wie S182);
> Messgröße das Momentary-Maximum je Ton (lautester 400-ms-Block, K-bewertet), Töne alle 3 Halbtöne, Stärke 0,7 (wie LP3 a).

## 1 Quellen (neu unter `…\cmake\quellen\`, Index `docs/archiv/Index_quellen.md`)

| Quelle | Ablage | Belegkraft | Was sie trägt |
|---|---|---|---|
| Pfeifle 2017, „Real-time Physical Model of a Wurlitzer and Rhodes Electric Piano“, DAFx-17, S. DAFX-17…24 | `platte-stab/Pfeifle_2017_…pdf` | Fachquelle, **gelesen** | Zunge „non-uniform cantilever beam“, höhere Moden „comparably small or non-existent even under extreme playing conditions“ (S. DAFX-21); Tonabnehmer: geladene Platte „has cutouts at the position of the reed … reeds are designed to vibrate freely between the symmetric cutouts“ (S. DAFX-18); 170 V laut Handbuch, gemessen 130 V; Stimmen über Lot an der Spitze. **Keine Maße, keine Hammerwerte.** |
| Wurlitzer Service Manual Series 200/200A (OS 153, Scan tropicalfishvintage) | `platte-stab/Wurlitzer_Service_Manual_…pdf` | Herstellerdokument, **gelesen** | „Three ply maple covered with mothproofed felt“, „Swedish Sandvik steel reeds“ (S. 2); Blow distance 1 5/64″, Let-off 1/8″ (S. 17); Pickups „160 to 170 volt DC above ground“ (S. 4); zu laute Töne: „bend the ends of the pickup up slightly (1/32″ to 1/16″)“ (S. 43). **Keine Zungenmaße.** |
| Vintage Vibe, Reed Case Study; EP-Forum „Wurlitzer 200 Reed Dimensions“ (cinnanon 2014, DocWurly 2016); EP-Forum „The formula for lengths of Wurly reeds“ (DocWurly 2017); Paleophone „Reed Compatibility History“ | Wortlaut im Agentenbericht der Sitzung | **nur Bericht** (Händler, Forum) | Länge je Nummer: #1 = 2 19/20″, −1/20″ je Zunge bis #20 = 2″, ab #21 −1/44″ bis #64 = 1″ (zwei Quellen gleich); Breite je Gruppe 1–14 .155″, 15–20 .129″, 21–42 .120″, 43–50 .110″, 51–64 .097″ (VV; Paleophone .150/.125/.120/.110/.097); Dicke der Zunge: 140B .024–.026″ bis #42, .022″ ab #43 (Forum); 200A-Text „.020″ on the tongue of a 200 reed to .026″ on the 200A“ (VV); Basis ab #21 ≈ .032″ (DocWurly). Seitlicher Abstand Zunge–Schlitz (140B): .0115″ → .0075″ je Seite. |
| Chapman 2006, „Investigating the sound of an African thumb piano (kalimba)“, Canadian Acoustics 34(3) 26–27 | `platte-stab/Chapman_2006_…pdf` | Fachquelle, **gelesen** | Tab. 1 (S. 27): D3 a 5,4 cm b 1,4 cm f₀ 304 Hz; E3 5,0/1,3/336; G♯3 4,6/1,4/415; A♯3 4,2/1,3/462; D4 3,7/1,3/581 — **Chapmans „D3“ = 293,7 Hz = MIDI 62** (Klaviernote); „some care was taken to produce uniformly loud notes with the thumbs as one would in a typical performance“ (S. 26). Instrument: kleine Kalimba (2008: Hugh Tracey Pentatonic). Keine Breite, Dicke, Pegel. |
| Chapman 2008 (ASA, Lay-Paper acoustics.org), Ludwigsen 2004/2011/2023 (JASA-Abstracts) | nur Abstracts gelesen | Zusammenfassung | f₁/f₀ 5,3–5,9 bzw. 5,9–6,8; Modell „clamped-supported-free beam“; längste Zunge der Treble-Kalimba B3 247 Hz. Chapman 2012 (JASA 131, 945) nicht frei erreichbar. |
| cmuse „Kalimba Tine Length Calculator“ | Wortlaut im Agentenbericht | nur Bericht | Federstahl „around 1.2 millimeters is typical“, E 200 GPa, ρ 7850; 17-Tasten-C-Kalimba C4 „about 61 mm active“, E6 „about 27 mm“. |
| Gabrielli et al. 2013 (EURASIP), Gabrielli et al. 2011 (ICMC); Remaggi et al. 2012 | `hammer-saite/…` (lagen schon) | Fachquelle, gelesen | 60 Saiten F1–E6, „the first 23 strings wound“; Tangente: Puls N = f_s·d/v, v „1 to 4 m/s“; Tonabnehmer: Fluss fällt „negative exponential“ mit dem Abstand, „simulations have been carried out for different string gauges“ (S. 6) ohne Zahlen; Verstärker Shelf −3 dB/130 Hz, +3 dB/4 kHz; Saite 67,8 cm bei 161 Hz (Remaggi). **Keine Längentabelle, keine Stärken.** |
| Vintage Vibe „Clavinet Strings“; Electric Keys; EP-Forum 9669/9397 | Wortlaut im Agentenbericht | **nur Bericht** | 10× .032″ F1–, 5× .028″, 9× .022″, 3× .011″ F3–G3, 35× .009″ G♯3–E6 (zwei extra); „.011 gauge strings … to achieve a more consistent tone and volume between string sizes .022 and .009“; Original .008″ statt .009″; **Material der Umspinnung nirgends genannt**. |
| Kipness, Clavinet FAQ 1.6 (clavinet.com) | `hammer-saite/Kipness_ClavinetFAQ_…pdf` | nur Bericht | „60 strings of various lengths and gauges“; Tonabnehmer „extremely weak“, Übertrager; nichts zu Längen. |

## 2 Wurlitzer (`reed_electric_piano`, Kern `bar`, `baueVorgabeWurlitzer`)

### 2.1 Zunge je Taste (Rechnung aus „nur Bericht“)

Zunge n = MIDI − 32 (#1 = A1, #64 = C7). Gesamtlänge L(n) nach DocWurly/Paleophone; Breite w je Gruppe (VV); Stahl
E = 200 GPa, ρ = 7850 kg/m³ (Sandvik-Federstahl, Lehrbuch). **Einspannlänge c** (Basis mit Schraube) aus Zunge #64 ohne Lot:
f = 0,1615·t/L_frei²·√(E/ρ) → c = 10,6 mm (Dicke „Forum“) bzw. 9,4 mm („VV 200A“); damit L_frei = L − c. Steife an der Spitze
k = E·w·t³/(4 L_frei³), Masse der Zunge m_Z = ρ·w·t·L_frei, **Lot aus f₀**: m_Lot = k/(2πf₀)² − 0,2427·m_Z, und die
Masse, die der Kern als gleichförmigen Stab erwartet (Form an der Spitze 1, modale Masse 0,2427·m):
**m_äquiv = m_Z + m_Lot/0,2427**.

| # | MIDI | f₀ Hz | L mm | L_frei mm | w mm | t mm (VV 200A) | k N/mm | m_Z g | m_Lot g | **m_äquiv g** |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 33 | 55,0 | 74,9 | 65,6 | 3,94 | 0,508 | 0,09 | 1,03 | 0,52 | **3,16** |
| 10 | 42 | 92,5 | 63,5 | 54,1 | 3,94 | 0,508 | 0,16 | 0,85 | 0,28 | **1,98** |
| 20 | 52 | 164,8 | 50,8 | 41,4 | 3,28 | 0,508 | 0,30 | 0,54 | 0,15 | **1,16** |
| 21 | 53 | 174,6 | 50,2 | 40,9 | 3,05 | 0,660 | 0,64 | 0,65 | 0,38 | **2,20** |
| 33 | 65 | 349,2 | 43,3 | 33,9 | 3,05 | 0,660 | 1,12 | 0,54 | 0,10 | **0,96** |
| 42 | 74 | 587,3 | 38,1 | 28,7 | 3,05 | 0,660 | 1,85 | 0,45 | 0,03 | **0,56** |
| 50 | 82 | 932,3 | 33,5 | 24,1 | 2,79 | 0,660 | 2,87 | 0,35 | ≈ 0 | **0,35** |
| 64 | 96 | 2093 | 25,4 | 16,0 | 2,46 | 0,660 | 8,60 | 0,21 | 0 | **0,21** |

Mit der Dicke „Forum“ (.025″/.022″): m_äquiv 6,5 g (#1) … 0,16 g (#64), Verhältnis 41 (32 dB); mit „VV 200A“ 15 (24 dB).
Heute: **2 g fest**. Ab #44–50 kommt rechnerisch kein Lot mehr heraus (|m_Lot| < 0,01 g) — passt zu den winzigen Lotpunkten
im Diskant; der Sprung bei #20/#21 (dünne Basszunge → dickere, geschliffene Zunge) ist in den Quellen so beschrieben.
Die Dicke ist die unsicherste Größe (±20 % zwischen den Berichten); sie geht in m_Z linear, in das Lot mit t³.

### 2.2 Hammer

Dreilagiger Ahornhammer mit Filz (Manual), kein Maß, keine Steife. Heute: **Stulov-Flügelhammer an C4** (8,3 g, p 4,3, q₀ 1107,
ε 0,85) — zu schwer und zu weich für eine Zunge von 0,2–3 g. Geprüfte Alternativen: Stulov am Flügel-Diskant n = 100 (4,7 g,
p 5,2, q₀ 16 473, ε 0,11; „übertragen“) und der Rhodes-Neoprenhammer (11 g, Hunt–Crossley). Ohne Aufnahme mit Stärken (M8,
F-7) bleibt der Hammer **offen**.

### 2.3 Tonabnehmer — die eigentliche Ursache des Übersteuerns

Heute: Plattenkondensator C(y) = 1/(d − y), d = 3 mm, abgeschnitten bei 0,9·d. Die Zunge schwingt im Bass aber 5–8 mm
(Geschwindigkeit nach dem Stoß 2–3 m/s durch ω): sie läuft in die Grenze, der Ausgang klemmt bei Spitze 1,0. Das echte
Bauteil (Pfeifle Fig. 2, Forum: seitlicher Spalt 0,2–0,3 mm) ist ein **Schlitz**: die Zunge schwingt durch einen Ausschnitt der
geladenen Platte, die Kapazität zwischen Zungenkante und Schlitzkante ist am größten in der Plattenebene und fällt zu beiden
Seiten ab. Für große Wege wird daraus ein Begrenzer: jeder Durchgang liefert einen Puls fester Fläche ΔC ≈ C₀, die Grundwelle
des Ausgangs sättigt bei ∝ U₀·C₀·f — der Bass wird reich an Obertönen („bark“), aber nicht lauter. Das Manual stimmt laute Töne
genau so ab: Plattenende 0,8–1,6 mm aufbiegen = Arbeitspunkt y₀ verschieben.

### 2.4 Proben (Momentary-Maximum, Spanne über 33…96)

| Variante | Spanne | Bass (33–57) | Diskant (84–96) | Spitze im Bass |
|---|---|---|---|---|
| heute | **34,3 LU** | −12 … −6 | −28 … −41 | 1,00 (übersteuert bis 57) |
| W1 Masse je Taste (Forum) | 30,4 | −11 … −6 | −25 … −36 | 1,00 bis 66 |
| W1b Masse je Taste (VV 200A) | 27,8 | −12 … −9 | −32 … −35 | 1,00 bis 66 |
| **W2** Masse (Forum) + Stulov n = 100 | **22,4** | −21 … −19 | −34 … −33 | 0,79 … 0,14 |
| W3 Masse (Forum) + Rhodes-Hammer | 38,1 | −11 … −5 | −26 … −43 | 1,00 bis 69 |
| W4 nur Stulov n = 100 (Masse 2 g) | 25,4 | −12 … −23 | −29 … −33 | 1,00 bis 48 |

**Abschätzung Schlitz** (`schlitz_abschaetzung.py`: Grundmode, elastischer Stoß v_Z = 2·m_H/(m_H + m_eff)·v_H, C(y) = C₀·w/(1 + (y/T)²),
Ausgang dC/dt, dieselbe Messgröße; **kein Lauf des Kerns**): Plattenkondensator d = 3 mm in dieser Rechnung 19–21 LU; Schlitz
mit T = 0,5 mm 6–9 LU, T = 1,0 mm 4–7 LU, T = 1,5 mm 3–5 LU — für alle drei Hämmer (4,7 g / 8,3 g / 3 g) und beide Dicken
gleich. Die Pegelverteilung über die Tasten hängt am Tonabnehmer, nicht am Hammer.

## 3 Kalimba (`kalimba`, Kern `bar`, `baueVorgabeKalimba`)

### 3.1 Zunge je Taste

Aus Chapman Tab. 1: a·√f₀ = 94,2 / 91,7 / 93,7 / 90,3 / 89,2 cm·√Hz (Streuung 5 %) → **a(f) = 91,8 cm·√Hz/√f** (Kragbalken
f ∝ 1/a²). Gleichförmiger Kragbalken 1,2 mm Federstahl rechnet 335/391/462/555/715 Hz gegen gemessen 304/336/415/462/581 —
der Abschnitt b hinter dem Steg senkt f₀ um 10–20 %, die Dicke 1,2 mm (cmuse) ist damit plausibel. Probe am Hugh Tracey
Treble: C4 → a = 5,7 cm, cmuse „about 61 mm active“; B3 (Ludwigsen) → 5,8 cm. Masse m = ρ·w·t·a = 0,0471 kg/m · a:
MIDI 50 (D3) 7,6 cm 3,6 g · 62 (D4) 5,4 cm 2,5 g · 74 (D5) 3,8 cm 1,8 g · 86 (D6) 2,7 cm 1,3 g (heute 3 g fest).
**Nebenbefund:** die Kurve `secondRatio` setzt Chapmans D3/D4 auf MIDI 50/62; Chapmans Noten sind Klaviernoten (293,7 Hz)
→ richtig MIDI 62/74.

### 3.2 Zupfen: Weg oder Kraft

Heute: statische Auslenkung δ = 1 mm an jeder Zunge (`velocityMax`, „offen“); die Masse kürzt sich heraus, der Ausgang
(Beschleunigung der Zunge) ist ∝ ω²·δ → +12 dB/Oktave, gemessen 37,4 LU über drei Oktaven. Steife an der Spitze
k = E·w·t³/(4a³): 1,0 N/mm (MIDI 50) … 22,5 N/mm (86). Ein Daumen gibt eine begrenzte **Kraft**, keinen festen Weg: bei
F = 3 N wäre δ = 3,0 mm (50) … 0,13 mm (86). Dann Beschleunigung ∝ F/m ∝ f^0,5 (+3 dB/Okt.), **Kraft am Steg** = k·δ = F
→ eben. Die Kalimba strahlt über Brett/Kasten, den die Kraft am Steg treibt — nicht über die Zunge selbst.

### 3.3 Proben (je Note eigene `.lvinst` mit δ(n); Kraft am Steg als m(n)·Beschleunigung gewichtet)

| Variante | Spanne |
|---|---|
| heute (δ 1 mm fest, Beschleunigung) | **37,4 LU** (−47 an 50, −10 an 86) |
| K1 feste Kraft, Ausgang Beschleunigung | 10,4 LU |
| **K1 feste Kraft, Ausgang Kraft am Steg** | **1,3 LU** |
| heute, aber Kraft am Steg | 28,4 LU |

Die absolute Kraft kürzt der Bezug (`normiereNachTafel`) heraus; sie wirkt nur auf die Form δ ∝ a³.

## 4 Clavinet (`clavinet_modal`, Kern `struckString`, `baueVorgabeClavinet`)

Heute: Weg der Tangente 1 mm fest (die Mechanik gibt den Spalt Tangente–Amboss für alle Tasten vor — das ist richtig so),
Ausgang Geschwindigkeit am Tonabnehmer → ∝ f, dazu der Shelf des Verstärkers: 22,0 LU (Momentary) / 23,8 LU (integriert).
Was im Modell fehlt: der magnetische Tonabnehmer sieht das **Volumen** der Saite — induzierte Spannung ∝ Geschwindigkeit ×
d_magn² (Dipolmoment ∝ Volumen; Gabrielli simulierte „different string gauges“, Zahlen nicht veröffentlicht). Stärken je
Taste (VV, Zählung 10/5/9/3/33): Tasten 1–10 .032″, 11–15 .028″, 16–24 .022″, 25–27 .011″, 28–60 .009″ (Gabrielli: 23
umsponnen). Gewicht gegen .009″: .032″ +22 dB, .028″ +19,7, .022″ +15,5, .011″ +3,5 — **wenn die Umspinnung Stahl ist**.
Bei Kupferumspinnung zählt nur der Kern (Einschätzung .015/.014/.012″): +8,9 / +7,7 / +5,0 dB. Material nirgends belegt.

| Variante (Gewicht auf die S182-Basis gelegt, linear) | Spanne Momentary | integriert |
|---|---|---|
| heute | **22,0 LU** | 23,8 |
| C1 ∝ d² (Stahl umsponnen) | **14,2 LU** (Bass 29–50 −13 … −8, ab 53 −20 … −13) | 13,3 |
| C1 ∝ d² (Kupfer, nur Kern) | 13,2 LU | — |

Offen bleibt die Saitenlänge je Taste (Remaggi: 67,8 cm bei 161 Hz, sonst nichts): sie bestimmt die relative Lage des
Steg-Tonabnehmers (4 cm fest) und damit den Kammfilter je Taste — ohne Harfe nicht herleitbar.

## 5 Folgerung

- **Wurlitzer:** Masse je Taste und ein leichterer, härterer Hammer sind Vorgaben mit Herleitung, bringen aber nur 34 → 22 LU
  und lassen den Bass klemmen. Die Spanne entscheidet der Tonabnehmer: der Schlitz (Kern-Änderung, drei Werte T, y₀, g) ist
  nach der Abschätzung der Weg unter 10 LU — und er erklärt das Übersteuern und das „bark“ des Instruments.
- **Kalimba:** Zupfen mit Kraft statt Weg und Ausgang Kraft am Steg (Kern-Änderung, ein neuer Wert `pluckForce`, eine Wahl
  des Ausgangs) macht die Spanne eben; die Vorgaben (a(f), m(f) aus Chapman) kommen dazu.
- **Clavinet:** ein Tonabnehmer-Gewicht ∝ d² mit der Stärke je Taste (neuer Wert `stringDiameter` als Kurve, Kern rechnet
  μ und Gewicht daraus) bringt 22 → 14 LU; der Rest ist Saitenlänge und Verstärker-Shelf.

Alle drei brauchen danach: `kRechenFassung` des Kerns hoch, IC-Abdrücke (`werte/stab_ic.txt`, `klasse1_ic.txt`) mit
Begründung neu (Journal), `instrument_pegel.py --messung` für die drei Instrumente und `--loops`, `loop_werte.txt`,
`klangfassung.txt`, dann die drei Ausnahmen in `Loop Pegel LP3 a` entfernen.

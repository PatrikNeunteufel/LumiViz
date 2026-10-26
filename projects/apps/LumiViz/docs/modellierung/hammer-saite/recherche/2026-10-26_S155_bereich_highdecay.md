> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: `highDecay` (σ = b1 + b3·ω², T60 bei 10·f1 zu T60 bei f1). Suche: im Netz.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Recherche `highDecay` – belegte Bereiche je Register

Stand 2026-10-26 · Agent „highDecay“ · Sprache Deutsch, Zitate im Original

## Auftrag

`highDecay` = T60(Teilton bei 10·f1) / T60(Grundton), Verluste quadratisch in der Frequenz:
σ_k = σ_1·(1 + h·((f_k/f_1)² − 1)), h = (1/highDecay − 1)/99. Gesucht: belegte b1, b3 (bzw. R, η)
je Taste/Register am Konzertflügel, daraus highDecay min/max/typisch.

**Umrechnung (für alle Quellen gleich).** Mit σ(ω) = b1 + b3·ω² und f_10 ≈ 10·f1 (Inharmonizität
vernachlässigt, siehe offene Punkte):

    highDecay = T60_10 / T60_1 = σ_1 / σ_10 = (b1 + b3·ω1²) / (b1 + 100·b3·ω1²),   ω1 = 2π·f1

Bei Bensa et al. hängt der Verlust an der Wellenzahl β statt an ω: σ(β) = b1 + b2·β², Mode k einer
gelenkig gelagerten Saite β_k = kπ/L, also

    highDecay = (b1 + b2·(π/L)²) / (b1 + 100·b2·(π/L)²)        (braucht die Länge L je Taste)

Äquivalent b3 ≈ b2/c² mit c = 2·L·f1.

## Reichweite

- Rund 30 Abrufe (WebSearch/WebFetch) plus lokales Lesen. Volltext gesehen: Bensa et al. 2003,
  Chaigne & Askenfelt 1994 Teil I, Chabassier/Chaigne/Joly 2014 (M2AN Teil 1), Chabassier/Duruflé/Joly
  2016 (M2AN Teil 2), Simionato & Fasciani 2025 (arXiv), RT-0425 (lokal, nur §3.2 und Anhang-Tabellen
  zur Dämpfung).
- **Nicht erreicht:** Chaigne & Askenfelt 1994 **Teil II** (JASA 95, 1631–1640; enthält vermutlich die
  b3-Werte zu C2/C4/C7) – nur Bezahlschranke gefunden; Valette & Cuesta 1993 (Buch, nicht online);
  Cheng/Dixon/Mauch ICASSP 2015 (Zertifikatsfehler beim Abruf); Aalto-Diplomarbeit „Analysis and
  Parametric Synthesis of the Piano Sound“ (2005, lib.tkk.fi – Zertifikatsfehler); HAL-Seiten
  (Anubis-Sperre gegen automatischen Abruf).
- Kein Treffer ist kein Beweis: Weitere Messreihen (Giordano, Lehtonen, Ortiz-Berenguer, Bank) wurden
  nicht durchsucht bzw. liegen lokal beim anderen Agenten (Bank 2000/2006/2010).

## Quellen

### Q1 – Bensa, Bilbao, Kronland-Martinet, Smith III (2003) – **Hauptquelle, gemessen**

- „The simulation of piano string vibration: From physical models to finite difference schemes and
  digital waveguides“, J. Acoust. Soc. Am. 114 (2), Aug. 2003, 1095–1107, DOI 10.1121/1.1587146
  (Titel/Autoren aus der PDF-Kopfzeile).
- Zugang: Kopie auf einer Kursseite `https://www.math.kent.edu/~zheng/62262/piano_2.pdf` (Fußzeile
  „Redistribution subject to ASA license or copyright“) – **keine Verlags-/Autorenseite, daher nicht
  ins `quellen/` abgelegt**; Verlag: ASA/AIP, Bezahlschranke. **Volltext gesehen** (Text und Seite 1103
  als Bild geprüft – die Minuszeichen in Gl. 37a fehlen in der Textebene, im Bild vorhanden).
- Flügel: „Yamaha Disklavier C6 grand piano“ (S. 1102), Anschlag „2.2 m s−1, which corresponds roughly
  to a MIDI value of 80“, Beschleunigungsaufnehmer am Steg; σ je Teilton über analytisches Signal.
  Für mehrchörige Töne: σ „can be thought of describing the global perceived decay of the sound“ –
  also **inklusive Stegkopplung und Chor**.
- Modell Gl. (6)/(8), S. 1096: Verlust „σ(β) = −b1 − b2 β²“ (Vorzeichen als Dämpfung).
- **Gl. (37a,b), S. 1103 (gedruckt), PDF-Seite 9:**
  „b1 = 4.4×10−3 f0 − 4×10−2, (37a)“ · „b2 = 1.0×10−6 f0 + 1×10−5. (37b)“
  Dazu: „The fits are linear as a function of the fundamental frequency“; „b1 and b2 are both
  increasing functions of MIDI note number“.
- **Messbereich:** Fig. 13 („Values of ‚equivalent‘ b1 and b2 fitted from measured data as a function
  of the fundamental frequency“) zeigt Daten nur von rund 30 Hz bis rund 520 Hz (Achse bis 500,
  Kurve bis knapp darüber) – **oberhalb von etwa C5 (MIDI 72) ist Gl. 37 Extrapolation.**
- **Streuung (aus Fig. 13 abgelesen, nur Augenmaß, kein Zahlenwert im Text):** b1 liegt zwischen etwa
  dem 0,5-Fachen und bis zum rund 3-Fachen der Geraden (z. B. bei ~120 Hz ≈ 1,8 s⁻¹ gegen Gerade ≈ 0,5);
  b2 etwa ±50 % um die Gerade. Die Autoren: „deviations … must be attributed to the impedance ratio
  between the strings and the soundboard, which varies over the length of the bridge“.
- **Tabelle I, S. 1096** („Physical model parameters for piano tones C2, C4, and C7“):
  L = 1.23 / 0.63 / 0.10 m; c = 160.9 / 329.6 / 418.6 m s⁻¹; „b1 0.25 1.1 9.17 s−1“;
  „b2 7.5×10−5 2.7×10−4 2.1×10−3 m2 s−1“. Herkunft laut S. 1100: „taken from values provided in the
  papers by Chaigne and Askenfelt, except for the parameter b2, which comes from the calibration
  procedure“. → **b1 für C2/C4/C7 sind die Werte von Chaigne & Askenfelt** (Teil I/II), b2 aus Gl. 37b
  (stimmt auf die Ziffer: z. B. C7 4.4e-3·2093 − 0.04 = 9.17).
- Umrechnung Tabelle I (L aus Tabelle I):
  C2: β1 = π/1.23 = 2.554, b2β1² = 4.89e-4 → σ1 = 0.2505, σ10 = 0.2989 → **highDecay 0.838**, T60_1 = 6.91/0.2505 = 27.6 s
  C4: β1 = 4.987, b2β1² = 6.72e-3 → σ1 = 1.107, σ10 = 1.772 → **0.625**, T60_1 = 6.2 s
  C7: β1 = 31.42, b2β1² = 2.073 → σ1 = 11.24, σ10 = 216.5 → **0.052**, T60_1 = 0.61 s
- Umrechnung Gl. 37 über die Tastatur: L je Taste **nicht aus Bensa** (Yamaha C6 nicht angegeben),
  ersatzweise die „realistischen“ Längen aus RT-0425 Anhang A (Steinway-D-Modell) – **Annahme**.
  Streuband: b1 ×[0,5; 3], b2 ×[0,5; 1,5] (aus Fig. 13 abgelesen); min = (0,5·b1, 1,5·b2),
  max = (3·b1, 0,5·b2). Skript: `…\scratchpad\recherche\bensa_hd.py`.

  | Taste | MIDI | f0 Hz | b1 s⁻¹ | b2 m²/s | L m (RT-0425) | hD typ | hD min | hD max |
  |---|---|---|---|---|---|---|---|---|
  | A0 | 21 | 27.5 | 0.081 | 3.8e-5 | ≈2.0 (Annahme) | 0.90 | – | – |
  | C1 | 24 | 32.7 | 0.104 | 4.3e-5 | 2.007 | 0.909 | 0.770 | 0.984 |
  | C2 | 36 | 65.4 | 0.248 | 7.5e-5 | 1.602 | 0.896 | 0.743 | 0.981 |
  | C3 | 48 | 130.8 | 0.536 | 1.4e-4 | 1.259 | 0.861 | 0.674 | 0.974 |
  | E3 | 52 | 164.8 | 0.685 | 1.8e-4 | 1.013 | 0.805 | 0.580 | 0.961 |
  | C4 | 60 | 261.6 | 1.111 | 2.7e-4 | 0.657 | 0.645 | 0.380 | 0.916 |
  | E4 | 64 | 329.6 | 1.410 | 3.4e-4 | 0.529 | 0.545 | 0.289 | 0.877 |
  | C5 | 72 | 523.3 | 2.262 | 5.3e-4 | 0.344 | 0.344 | 0.154 | 0.756 |
  | C6* | 84 | 1046.5 | 4.565 | 1.1e-3 | 0.180 | 0.133 | 0.055 | 0.465 |
  | C7* | 96 | 2093 | 9.169 | 2.1e-3 | 0.095 | 0.048 | 0.023 | 0.201 |
  | C8* | 108 | 4186 | 18.38 | 4.2e-3 | ≈0.05 (Annahme) | 0.021 | – | – |

  \* extrapoliert (außerhalb der Messdaten von Fig. 13).

### Q2 – Chaigne & Askenfelt (1994), Teil I – Form des Gesetzes, keine Zahlen

- „Numerical simulations of piano strings. I. A physical model for a struck string using finite
  difference methods“, JASA 95 (2), Feb. 1994, 1112–1118 (aus PDF-Kopf/Fußzeile). Kopie:
  `https://www.math.kent.edu/~zheng/62262/piano_wave.pdf` (Kursseite, ASA-Vermerk; nicht abgelegt).
  **Volltext gesehen** (Textebene stark verstümmelt).
- Gl. (1) enthält „−2b1 ∂y/∂t + 2b3 ∂³y/∂t³“ (Form bestätigt über Bensa Anhang Gl. A1). Gl. (3):
  Abklingrate wächst mit ω² (Glyphen unlesbar). Zitat S. 1113: „The constants b1 and b3 in Eq. (3) were
  derived from experimental values through standard fitting procedures“; „account globally for the
  losses in the air and in the string material, as well as for those due to the coupling to the
  soundboard“; „only a fair approximation of the reality“.
- **Keine Zahlenwerte für b1/b3 in Teil I.** Die Werte stehen laut Bensa in den Papieren 3,4 (Teil I/II);
  Teil II nicht zugänglich → **b3 von Chaigne & Askenfelt offen.**

### Q3 – Chabassier & Duruflé, RT-0425 (2012) – angesetzt, nicht gemessen (lokal vorhanden)

- „Physical parameters for piano modeling“, INRIA RT-0425; lokal
  `…\quellen\Chabassier_Durufle_2012_PhysicalParametersPianoModeling_RT0425.pdf`. **Volltext gesehen**,
  PDF-Seite 12 = gedruckt S. 9, §3.2, Gl. (6), als Bild geprüft:
  „The damping coefficients are difficult to tune (lack of measurements, difficulty to dissociate the
  origins of damping). They are set to the following values.“
  „R_u,i = R_φ,i = (α i + β)+ ∈ [0; 0,42]“, „R_v,i = 0,5“, „η_u,i = η_φ,i = γ i + δ ∈
  [1,6 × 10−9; 3,9 × 10−9]“, „η_v,i = 10−9“, „α = 5 × 10−3, β = −0,015, γ = 2,78 × 10−11 and
  δ = 1,5274 × 10−9“, i ∈ [1, 88].
- Gesetz aus Chabassier/Chaigne/Joly 2014 (Q4), Gl. (3.67): ρA∂²u + 2ρA R_u ∂u − 2T0 η_u ∂²ₓ∂ₜu − T0∂²ₓu = 0
  → σ = R_u + η_u·(T0/ρA)·κ² = **R + η·ω²** (nicht-dispersiv). Also b1 = R_u, b3 = η_u.
- i = MIDI − 20 (geprüft: Anhang A, C1 = Taste 4: R = 0.005, Tabelle „5.0 × 10−03“; C2 = Taste 16:
  0.065 ✓; η(4) = 1.64e-9, Tabelle „1.6 × 10−09“ ✓).
- **Wichtig:** Im Chabassier-Modell ist das die *Saite allein*; der Steg/Resonanzboden bringt eigene
  Verluste. Daher T60 der Grundtöne absurd lang (C4 36 s, C2 106 s, A0 → ∞ wegen R = 0).
- Umrechnung je Taste (Skript `…\scratchpad\recherche\rt_formel.py`):
  MIDI 21–23: R = 0 → highDecay = 1/100 = **0.010** (Artefakt der Klammer (·)+);
  24 (C1) 0.425 · 28 0.684 · 33 0.692 · 36 (C2) 0.665 · 45 0.514 · 48 (C3) 0.451 · 57 0.266 ·
  60 (C4) 0.214 · 69 (A4) 0.103 · 72 (C5) 0.080 · 81 0.038 · 84 (C6) 0.031 · 93 0.018 ·
  96 (C7) 0.016 · 105 0.012 · 108 (C8) 0.0115.
  Die Anhang-Tabellen (beide virtuellen Flügel, 2×84 Töne C1–B7) geben dieselben Werte (Parser
  `…\scratchpad\recherche\rt_parse.py`).

### Q4 – Chabassier, Chaigne, Joly (2014), M2AN 48, 1241–1278 – Gesetz, keine Zahlen

- „Time domain simulation of a piano. Part 1: model description“, DOI 10.1051/m2an/2013136.
  **Volltext gesehen**, §3.5 (gedruckt S. 1261–1262): „we propose as a first approach to use a very
  simple model that allows to reproduce these effects without trying to model the underlying
  physics“; Gl. (3.67); „Ru and ηu are empirical … damping coefficients which are respectively
  homogeneous to a time or the inverse of a time“. Keine Zahlenwerte.
- **Heruntergeladen** (Freigabe Patrik): siehe unten.

### Q5 – Chabassier, Duruflé, Joly (2016), M2AN 50, 93–133 (Teil 2) – keine Zahlen gefunden

- Numdam, Volltext gesehen (Textsuche nach „damping“, „R_u“, Tabellen): keine Zahlen zur
  Saitendämpfung; verweist auf RT-0425 („[8] … Physical parameters for piano modeling“). Lokal
  bereits vorhanden.

### Q6 – Simionato & Fasciani (2025), arXiv:2409.06513v3 – nur Form

- „Sines, Transient, Noise Neural Modeling of Piano Notes“, arXiv (CC-Lizenz nicht geprüft).
  Volltext gesehen: Valette-Cuesta-Form „1/Q_m = 1/Q_m,air + 1/Q_m,vis + 1/Q_m,ther“, Gl. (11) mit
  b0…b3 je Note gelernt – **keine Zahlenwerte** für b1/b3. Nur Bestätigung der Form.

## Tabelle „Bereich je Register“

hD = highDecay = T60(10·f1)/T60(f1). „B03“ = Bensa et al. 2003 (Yamaha C6, gemessen am Steg, global
inkl. Steg/Chor); „RT“ = RT-0425 Gl. (6) (angesetzt, Saite allein). Längen für B03-Umrechnung aus
RT-0425 (Annahme). min/max B03 aus dem Streuband in Fig. 13 (abgelesen).

| Register (MIDI) | b1 [s⁻¹] B03 | b2 [m²/s] B03 | R / η RT | hD min | hD max | hD typisch | Quelle | Zitat-Kurzform | Prüfstand |
|---|---|---|---|---|---|---|---|---|---|
| A0–B0 (21–23) | 0.08–0.10 | 3.8–4.1e-5 | 0 / 1.6e-9 | 0.75 (B03) | 0.98 (B03) | ≈0.90 (B03); RT 0.010 (Artefakt) | B03 Gl.37 extrapoliert unter ~30 Hz; RT Gl.6 | „b1 = 4.4×10−3 f0 − 4×10−2“ | B03: Rand der Messdaten |
| C1–B1 (24–35) | 0.10–0.22 | 4.3–7.2e-5 | 0.005–0.06 / 1.6–1.9e-9 | 0.43 (RT C1) | 0.98 | 0.90 (B03) · 0.43–0.69 (RT) | B03 Fig.13/Gl.37; RT Gl.6 | „fitted from measured data“ | B03 gemessen |
| C2–B2 (36–47) | 0.25–0.50 | 7.5e-5–1.3e-4 | 0.065–0.12 / 2.0–2.3e-9 | 0.47 (RT B2) | 0.98 | 0.84 (B03 Tab. I) – 0.90 (B03 Gl.37) · 0.66 (RT C2) | B03 Tab. I (b1 von Chaigne & Askenfelt) | „b1 0.25 1.1 9.17 s−1“ | gemessen |
| C3–B3 (48–59) | 0.54–1.04 | 1.4–2.6e-4 | 0.125–0.18 / 2.3–2.6e-9 | 0.24 (RT B3) | 0.97 | 0.86→0.70 (B03) · 0.45→0.24 (RT) | B03; RT | „increasing functions of MIDI note number“ | gemessen |
| C4–B4 (60–71) | 1.11–2.10 | 2.7–5.0e-4 | 0.185–0.24 / 2.6–2.9e-9 | 0.09 (RT B4) | 0.92 | 0.63–0.65 (C4, B03) → ~0.37 (B4) · 0.21→0.09 (RT) | B03 Tab. I/Gl.37; RT | „b2 = 1.0×10−6 f0 + 1×10−5“ | gemessen |
| C5–B5 (72–83) | 2.26–4.3 | 5.3e-4–1.0e-3 | 0.245–0.30 / 3.0–3.3e-9 | 0.035 (RT B5) | 0.76 | 0.34→0.15 (B03) · 0.08→0.035 (RT) | B03 (ab ~C5 extrapoliert); RT | Fig. 13 endet bei ~520 Hz | Rand / extrapoliert |
| C6–B6 (84–95) | 4.6–8.6 | 1.1–2.0e-3 | 0.305–0.36 / 3.3–3.6e-9 | 0.016 (RT) | 0.47 | 0.13→0.05 (B03) · 0.031→0.017 (RT) | B03 extrapoliert; RT | – | nur extrapoliert/angesetzt |
| C7–C8 (96–108) | 9.2–18.4 | 2.1–4.2e-3 | 0.365–0.425 / 3.6–4.0e-9 | 0.0115 (RT C8) | 0.20 | 0.05 (B03 Tab. I C7) → 0.02 · 0.016→0.012 (RT) | B03 Tab. I (C7) + Gl.37; RT | „b1 … 9.17“ (C7) | b1 C7 von Chaigne & Askenfelt; b2 extrapoliert |

## Einschätzung

- Belegt gemessen ist nur der Bereich 30–520 Hz (Bensa, ein Yamaha C6, mf); dort ergibt sich
  highDecay ≈ 0,9 im Bass, ≈ 0,65 bei C4, ≈ 0,35 bei C5, mit einer Streuung je Taste, die in b1 bis
  Faktor 3 reicht – hD-Band etwa ±0,15…0,3.
- RT-0425 liegt durchweg niedriger (C4 0,21 statt 0,65), weil dort nur die Saite gedämpft wird
  (kleines R); die Stegverluste sitzen im Chabassier-Modell separat. Welche der beiden Größen zu
  LumiViz passt, hängt davon ab, ob `highDecay` die Steg-/Bodenverluste enthält (→ Bensa) oder nicht
  (→ RT-0425).
- Als Optimierer-Grenzen je Register vertretbar: Bass (21–47) 0,45…0,99; Mitte (48–71) 0,1…0,97;
  oben (72–95) 0,015…0,8; Diskant (96–108) 0,01…0,25 – die Ränder kommen aus RT (unten) bzw. aus
  Bensa-Streuband (oben).

## Offene Punkte

1. **b3 von Chaigne & Askenfelt (Teil II, JASA 95, 1631–1640)** – nicht zugänglich; damit fehlt der
   zweite unabhängige Messsatz zu C2/C4/C7 (Bensa übernahm nur b1).
2. **Saitenlängen des Yamaha C6** für die B03-Umrechnung fehlen; Steinway-D-Längen aus RT-0425 sind
   Annahme. hD ist empfindlich: C2 mit L = 1.23 m (Tab. I) 0,838, mit L = 1.602 m (RT) 0,896.
3. Inharmonizität: f_10 > 10·f1 (B bis 8e-4 bei 500 Hz, Fig. 12 → f_10/10f1 ≈ √(1+100B) ≈ 1,04 bei
   C5). Das Modell rechnet mit (f_k/f1)²; die Abweichung in σ_10 ist dort ≈ 8 % – vernachlässigt.
4. Bensa misst die *global wahrgenommene* Abklingrate mehrchöriger Töne (Doppelabklingen, Schwebung
   nicht getrennt) – für ein Modell mit eigenem Chor/Steg ist das eine obere Schranke der Saitenverluste.
5. Nicht geprüft: Bank 2000/2006/2010 (lokal, anderer Agent), Cheng/Dixon/Mauch 2015 (RWC-Flügel,
   30 Teiltöne – wäre die beste offene Zweitmessung), Aalto-Diplomarbeit 2005, Valette & Cuesta 1993.

## Zwischenkopien und Dateien

- Abrufwerkzeug-Zwischenkopien (WebFetch, automatisch abgelegt):
  - `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\tool-results\webfetch-1793045265195-clgw6c.pdf` (arXiv 2409.06513)
  - `…\tool-results\webfetch-1793045297384-jvvgi1.pdf` (M2AN 2014 Teil 1)
  - `…\tool-results\webfetch-1793045319692-l1tutf.pdf` (M2AN 2016 Teil 2)
  - `…\tool-results\webfetch-1793045373205-5ack2c.pdf` (Bensa 2003, Kopie math.kent.edu)
  - `…\tool-results\webfetch-1793045395390-3pfftl.pdf` (Chaigne & Askenfelt 1994 I, Kopie math.kent.edu)
- Textauszüge/Bilder/Skripte: `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\recherche\` (`txt\`, `img\`, `bensa_hd.py`, `rt_parse.py`, `rt_formel.py`).
- **Heruntergeladen nach `quellen\` (Freigabe Patrik):**
  - `Chabassier_Chaigne_Joly_2014_TimeDomainSimulationPiano_Part1_M2AN_numdam.pdf` – URL
    `https://www.numdam.org/item/M2AN_2014__48_5_1241_0.pdf`, 3 959 024 Byte, freier Zugang über das
    Archiv Numdam (Mathdoc/CNRS); Lizenztext auf der Numdam-Seite nicht angegeben. Von `.gitignore`
    Zeile 471 erfasst (geprüft).
  - Bensa 2003 und Chaigne & Askenfelt 1994 I **nicht** abgelegt: nur auf einer fremden Kursseite
    verfügbar (keine Verlags-/Archiv-/Autorenseite).

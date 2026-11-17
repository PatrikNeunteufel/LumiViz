> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_12_harfe_grundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 22:21–22:45, claude-opus-5-5, 114 Runden, success, 7.58 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Seitenbilder des Laufs im Archiv der Werkbank, `seiten/`):
> - **Bell Abb. 7.22 (PDF 309) am Seitenbild:** die Achse heißt nur „Input admittance level (dB)“, **kein Bezug** —
>   bestätigt. Die 24 Werte selbst abgelesen (±1 dB) und regressiert: **6,03 dB/Oktave**; eine Reihenfeder k =
>   5,2·10⁴ N/m trifft sie im Mittel auf +0,7 dB, Streuung 3,4 dB (Bericht +0,9 / 3,5). Die Steigung ist damit
>   belegt; dass sie aus dem Messweg stammt, bleibt **Hypothese** (kein Satz bei Bell, §6.3 nicht gelesen).
>   **Folge für S201:** die Prüfung „Admittanz an Saite 26 bei 207 Hz 87,3 gegen 86,1 dB“ (KQ29 a) setzt für Abb. 7.22
>   den Bezug 116 dB = 1 s/kg aus Kap. 6 voraus — er ist dort nicht angegeben.
> - **Bell S. 76 (PDF 134) am Seitenbild:** Helmholtz-Moden je einzelner Öffnung 75 / 87 / 94 / 88 / 86 Hz mit **Q 10 /
>   9 / 9 / 11 / 10** — bestätigt. Die Bedingung „starre, eingegrabene Wände“ steht auf dieser Seite nicht (nicht
>   nachgeprüft). S. 77: Volumina mit Trennwänden, ±5 % — stimmt.
> - **Le Carrou 2006 S. 104 (PDF 115) am Seitenbild:** Gl. (6.9)–(6.16) und Abb. 6.2 stimmen; α_i ist der Winkel
>   zwischen Saite und Bodenachse, also bei 30° **Querterm cos² = 0,75, Längsterm sin² = 0,25** — die Herleitung (S5) und
>   der Befund „S198 nahm 0,25 für den Querwiderstand“ sind damit richtig begründet (S198 selbst nicht nachgesehen).
> - **Nicht gegengelesen:** Bell Tab. 2.1 / 2.2, Abb. 2.10 (Züge), Le Carrou Gl. (4.16)–(4.21) und die 2-FHG-Matrizen,
>   Waltham Abb. 4, die Rechnungen r1–r10 Zahl für Zahl, Abschnitte 3–6 im Einzelnen.
> - **Nacharbeit:** Auftrag `2026-11-17_22_harfe_nacharbeit` der Werkbank (Messweg bei Bell, besaitete Güten, offene
>   Quellen der Ladelisten 02 / 12). Für die LumiViz-Seite: Winkelfaktor der Saitenlast (0,75 statt 0,25), Ziel Z1 ohne
>   Bell 7.22, Güten besaitet gegen KQ29 a.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_12_harfe_grundlagen.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_12_harfe_grundlagen/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_harfe_grundlagen_werkbank/`: `anhang.py`, `r10_lecarrou_2fhg.py`, `r1_saiten.py`, `r2_mittelwert.py`, `r3_waltham_zuschnitt.py`, `r4_waltham_grau.py`, `r5_koerper_lesen.py`, `r6_koerper_rechnen.py`, `r7_abstrahlung_je_schnelle.py`, `r8_saiten_auf_moden.py`, `r9_z1_je_term.py`.

# Bericht: Berechnungsgrundlagen für den Körper der besaiteten Konzertharfe (Stufe 2)

- **Auftrag:** `auftraege/2026-11-16_12_harfe_grundlagen.md`
- **Datum:** 2026-11-16 · **Modell:** claude-opus-5-5
- **Voraussetzung:** erfüllt. Der Stufe-1-Bericht `ergebnisse/2026-11-16_02_harfe_besaitet_admittanz/bericht.md` liegt
  vor; `laeufe/2026-11-16_02_harfe_besaitet_admittanz.zustand.json` meldet `"status": "fertig"`, `ende_art` „success“.
- **Reichweite:** **Kein Netzabruf** (0 WebSearch, 0 WebFetch, 0 curl). Gelesen wurde nur lokal:
  - **Seitenbilder** (unter `seiten/`):
    - Bell 1988: PDF 68 (Abb. 2.10), 69–70 (Tab. 2.1/2.2), 134–135 (§4.2.5), 283 (§7.6), 293 (Abb. 7.6), 309 (Abb. 7.22/7.23);
    - Le Carrou 2006, Thèse: PDF 80 (Tab. 4.1, Gl. 4.16–4.18), 82 (Gl. 4.19–4.21), 115 (Gl. 6.9–6.18), 130 (Gl. 6.37–6.39);
    - Waltham 2008: S. 2 bei 300 dpi, Abb. 4 vergrößert.
  - **Textebene:**
    - Le Carrou 2006: Kap. 4 und 6 ganz, Anhang 1;
    - Le Carrou, Gautier & Foltête 2007: ganz;
    - Bell 1988: PDF 239–242 (§6.3).
  - **Suchen mit `pdf_suche.py`:** Bell Kap. 2, 4, 6, 7; Le Carrou 2006 Kap. 1–3; Lynch-Aird & Woodhouse 2017 und 2018;
    Woodhouse 2021; Woodhouse & Lynch-Aird 2019; Woodhouse et al. 2012; Waltham 2008.
  - **Camac 2022:** Textebene ganz.
  - **LumiViz, nur gelesen:**
    - S197-Doku §1–§14;
    - `make_harfe.py`;
    - `Bauplan.hpp` (Stegpunkt, Öffnung, Hohlraum, Dämpfung, Rand);
    - `Luftraum.hpp` (Kommentare);
    - `concert_harp.lvinstbody` (145 Moden; je Mode Form an den 80 Stegpunkten, η, Luftanteil, Abstrahlung am Hörort);
    - `proben/2026-11-16_S201_harfe_gegen_bell_7_22.txt`;
    - `proben/2026-11-14_S198_harfe_saitenorte.json`.
  - **Nicht erreicht oder nicht gelesen:**
    - Le Carrou, Gautier & Badeau 2009: nicht geöffnet; ihr Modell steht als Le Carrou et al. 2005 in Kap. 6 der Thèse, das ist gelesen.
    - Le Carrou 2010: nur die Werte aus Stufe 1.
    - Bell Kap. 8;
    - Chadefaux 2012/2013.
- **Rechnungen:** zehn kleine Skripte `r1_…py` bis `r10_…py` in diesem Ordner. Wortlaut und Ausgabe stehen im Anhang.
  `lc2006.txt`, `lc2007.txt`, `bell_239_242.txt`, `camac.txt` und `waltham_s2.txt` sind Textauszüge aus `pdftotext`
  (Windows-1252), nur zum Suchen gedacht.

Marken: `[wörtlich]` (Wortlaut oder Wert aus der Quelle), `[am Bild geprüft]`, `[Textebene]` (Wortlaut nur aus
`pdftotext`, nicht am Bild geprüft), `[hergeleitet]`, `[Lehrbuch, nicht gelesen]` (Standardformel, hier aus keiner
gelesenen Quelle belegt), `[Annahme]`, `[eigene Rechnung]`, `[eigene Einschätzung]`.

---

## Kurzfassung

1. **Kein Saitenterm hebt die Punkt-Admittanz abseits der Resonanzen um 10–37 dB.** Alle Terme sind unten mit Zahlen
   gerechnet:
   - **Gedämpfte Saiten.** Jede wirkt als Widerstand Z₀ cos²30° von 0,1–1 N s/m an ihrer Öse. Der Boden hat dort
     im Modell 47–250 N s/m, bei Bell rund 60, bei Le Carrou rund 1600 N s/m. Über die 24 Saiten von Abb. 7.22 bleibt der Median der Änderung bei 0,0 dB (Bereich −1,7 bis +2,7 dB).
   - **Längsfedern der Saiten** (EA/L · sin²30°, zusammen 0,47 MN/m). Sie verschieben die tiefen Moden und damit die
     Admittanz an einzelnen Saiten um −15 bis +22 dB. Über alle Saiten heben sie nicht: Median +1,4 dB.
   - **Geometrische Steife** aus der Vorspannung (T/L · cos²30°): sie verschiebt die Frequenzen um höchstens 0,4 %.
2. **Bells besaitete Werte (Abb. 7.22) passen nicht zum Boden, wie Bell ihn selbst beschreibt.** `[eigene Rechnung]`
   - Die Admittanz der unendlichen Platte mit Deck- und Zugleiste liegt an den Ösen bei **61–64 dB** (Bells Maß,
     116 dB = 1 s/kg).
   - Dagegen liegen im Median: das Modell **+6 dB**, Le Carrous besaitete Camac **−5 bis −9 dB**, Bell besaitet
     **+27 dB** (+13 bis +39 dB).
   - Bell liegt sogar 4–27 dB (Median 15 dB) über der Platte ganz ohne Leisten.
   - Ein Mittelwertsatz (Skudrzyk) erlaubt das nicht: Starke Dämpfung zieht die Admittanz zum Wert der unendlichen
     Struktur hin, nicht darüber.
3. **Abb. 7.22 steigt mit genau 6,0 dB/Oktave** (Regression 20,1 dB/Dekade).
   - Eine einzige Feder in Reihe mit dem Messpunkt, Y = ω/k mit **k = 5,2·10⁴ N/m**, trifft alle 24 Werte: im Mittel
     +0,9 dB, Streuung 3,5 dB `[eigene Rechnung]`.
   - Das ist die Handschrift einer Nachgiebigkeit im Messweg, nicht die des Bodens.
   - Die Achse von Abb. 7.22 trägt **keinen Bezug** („Input admittance level (dB)“) `[am Bild geprüft]`.
   - **Empfehlung an die Sitzung:** den Pegel von Abb. 7.22 nicht als Ziel für den Körper nehmen (Abschnitt 6).
4. **Was die Saiten nach den Rechnungen erklären** – die Verschiebung der tiefen Moden und die Güten, nicht den Pegel.
   - **Längsfedern bei starrem Hals:** S(1,1)− steigt um +23 %, gemessen sind +10 % (170 → 187 Hz). Das Gestell muss
     also etwa so nachgiebig sein wie die Saiten zusammen (Faktor 0,4 auf die Federn).
   - **Gedämpfte Saiten mit dem Winkelfaktor cos²30° = 0,75:** Sie senken die Güten von S(1,2)–S(1,4) von 36–42 auf
     23–29. Gemessen besaitet sind 27–32.
   - **Der Winkelfaktor in LumiViz ist zu klein:** Die S198-Probe rechnete mit 0,25 Z. Das ist der Faktor für die
     Längs-, nicht für die Querrichtung (Le Carrou Gl. 6.12–6.15).
5. **Das Diskantende kann die obersten Saiten nicht um 30 dB heben** `[eigene Rechnung]`.
   - An den Ösen 10–13 bestimmen die Leisten die Admittanz. Ein Boden von 2 statt 3 mm hebt die Platte um 5 dB, die
     Öse aber um weniger als 1 dB.
   - Das Leistenende wirkt nur auf die obersten drei bis vier Saiten (bis +6 dB an Saite 00).
   - Für die Tasten 92–94 (Ösen der Saiten 5/6) bringt das Ende unter 1 dB.
6. **Dämpfung der tiefen Moden.**
   - Bell hat die Güte der Luft im Kasten mit starren, eingegrabenen Wänden gemessen: **Q 9–11** je Öffnung
     (75–94 Hz) `[am Bild geprüft]`. LumiViz setzt 5,5 (angepasst).
   - Mit Q 10 und dem Holzgesetz geben die drei Moden Q 19–20.
   - Abstrahlung bringt bei S(1,1)+ je nach Ansatz Q_rad 77–1800. Damit erreicht S(1,1)+ Bells 16.
   - S(1,1)− (Bell 11) braucht zusätzlich Verluste im Gestell.
7. **Abstrahlung (Z4).**
   - **Je Schnelle zu hoch:** Das Modell gibt je Schnelle an der Öse 14–20 dB mehr Druck als Le Carrou (wirksame
     Fläche 0,3–1,1 m² gegen 0,05–0,09 m²).
   - **Ansatz:** Die Platte strahlt im Modell in den Halbraum (Rayleigh, Schallwand, ρ/2π), die Öffnungen in den
     Vollraum (ρ/4π). Die Platte zählt also doppelt gegen die Öffnungen.
   - **Z4 mit dem heutigen Körper** (Güte der Luft 5,5): **+7 bis +9 dB** an den Ösen 30–33 (nahe Le Carrous Punkt 34),
     **+12 bis +13 dB** an Öse 26. Le Carrou misst +16,6 dB. Die +19,8 dB der Vorgabe galten für den Körper mit Q 16.

---

## 1 Gleichungen

### 1.1 Saite an der Öse (Lücke 1)

**(S1) Zug aus Frequenz.** T = 4 μ L² f².
- Lynch-Aird & Woodhouse 2017, Gl. (1), S. 2: „f1 = 1/(2LV) √(F/µ)“ `[Textebene]`.
- Die Formel ist dieselbe wie (S3) mit k_t L = π.

**(S2) Kopplung Saite–Boden unter dem Winkel α.** Le Carrou 2006, Thèse, S. 104, Gl. (6.12)–(6.15) `[am Bild geprüft]`:

```
u_s(O_i) =  u_b cos α_i + w_b sin α_i                       (6.12)
w_s(O_i) = −u_b sin α_i + w_b cos α_i                       (6.13)
N_b(O+) − N_b(O−) + N_s cos α_i − Q_s sin α_i = 0           (6.14)
Q_b(O+) − Q_b(O−) + N_s sin α_i + Q_s cos α_i = 0           (6.15)
```

- u, w: Längs- und Querverschiebung; N, Q: Längs- und Querkraft; Index s für die Saite, b für den Boden („beam“).
- α_i ist der Winkel zwischen Bodenachse und Saite (Abb. 6.2).
- Le Carrou nennt für die Harfe „about 40°“ (S. 101) `[Textebene]`. Bell nennt für die Salvi „meet the soundboard at an
  angle of 30°“ (S. 157, PDF 283) `[am Bild geprüft]`.
- Der Boden ist dort ein Balken, der an beiden Enden eingespannt ist; die Saiten sind am fernen Ende fest. Es gibt
  keine Dissipation (S. 103).

**(S3) Wellenzahlen der Saite.** k_l = ω √(ρ_s/(E_s A_s)), k_t = ω √(ρ_s/T₀).
- Le Carrou 2006, S. 119, Gl. (6.38) `[am Bild geprüft]`; ρ_s ist dort die Masse je Länge (= μ).

**(S4) Impedanz der Saite an der Öse, Hals starr.** `[hergeleitet aus S3]`

```
Z_t(ω) = −j Z₀ cot(k_t L),    Z₀ = √(T μ)
Z_l(ω) = −j √(EA μ) cot(k_l L)
```

Konvention: Kraft auf den Boden = −Z · Schnelle der Öse, Zeitfaktor e^{jωt}.

**(S5) Zusatz in Normalrichtung des Bodens.** `[hergeleitet]`

```
Z_n(ω) = sin²α · Z_l(ω) + cos²α · Z_t(ω)
```

- Herleitung aus (S2) mit u_b = 0: Eine Normalbewegung w der Öse gibt der Saite die Längsbewegung w sin α und die
  Querbewegung w cos α. Die Kraft auf den Boden ist N sin α + Q cos α.
- Mit α = 30°: sin²α = 0,25, cos²α = 0,75.

**(S6) Grenzfälle von (S5).** `[hergeleitet]`
- **Längs, tief** (k_l L ≪ 1): Z_l → EA/(jωL), eine Feder **k_l = EA/L**. Die erste Längsresonanz liegt bei c_l/(2L)
  (Tabelle 2.2).
- **Quer, tief:** Z_t → T/(jωL), die **geometrische Steife** aus der Vorspannung, **k_g = T/L**.
- **Quer, Saite stark gedämpft** (Bänder, Papier; die Welle kehrt nicht zurück): Z_t → **Z₀**, ein reiner Widerstand.
- **Quer, ungedämpft, bei der eigenen Frequenz** (k_t L = nπ) mit Verlust η_s: |Z_t| ≈ 2 Z₀/(π η_s). Die Öse wird
  dann gesperrt.

**(S7) Saiten im Modenraum, erste Ordnung, diagonal.** `[hergeleitet; Annahme: Kopplung zwischen den Moden vernachlässigt]`

```
ω_k'² = ω_k² + Σ_j k_j φ_k(j)²
η_k'  = η_k + Σ_j R_j φ_k(j)² / ω_k
```

- φ_k(j): Form der Mode k an Öse j, massennormiert (wie `bridge` in der Körperdatei, 1/√kg).
- k_j = sin²α · EA/L (Hals starr) bzw. Faktor γ · sin²α · EA/L (Hals nachgiebig).
- R_j = cos²α · Z₀.

**(S8) Hals nachgiebig: Saiten und Gestell in Reihe.** `[hergeleitet, Annahme: das Gestell wirkt als eine Feder k_G
für alle Saiten]`

```
k_eff = (Σ k_j) · k_G / (Σ k_j + k_G),    γ = k_G / (Σ k_j + k_G)
```

Zahlen für k_G (Hals als Balken, Säule als Stab) wurden **nicht gefunden** (Abschnitt 7). γ lässt sich nur aus Bells
Verschiebung von S(1,1)− zurückrechnen (3.2).

**(S9) Vorspannung in der Ebene des Bodens.** ω'² = ω² (1 + N/(EI k²)) für einen Balken unter der Längskraft N, Mode mit
Wellenzahl k. Euler-Last: P_cr = π² EI/L².
- `[Lehrbuch, nicht gelesen]` – nur als Größenordnung (3.3).

### 1.2 Mittelwert der Punkt-Admittanz (Lücken 1 und 2)

**(P1) Unendliche Platte.** Y∞ = 1/(8 √(D m″)); für den orthotropen Boden D = √(D_x D_y), D = E h³/(12(1−ν²)).
- `[Lehrbuch (Cremer/Heckl), nicht gelesen]`.

**(P2) Unendlicher Balken.** Z = 2(1+j) m′ c_B mit c_B = (EI/m′)^{1/4} √ω, also |Y| = 1/(2√2 m′ c_B).
- `[Lehrbuch, nicht gelesen]`.

**(P3) Öse auf Deck- und Zugleiste.** Y_Öse ≈ 1/(Z_Balken + 1/Y_Platte).
- `[Annahme: Leisten als Verbundbalken (Deckleiste außen, Boden, Zugleiste innen), Platte parallel am Punkt]`.

**(P4) Mittelwertsatz.** Der Frequenzmittelwert des Realteils der Punkt-Admittanz einer endlichen Struktur ist der
Wert der unendlichen Struktur. Je stärker die Dämpfung, desto näher bleibt |Y| an Y∞.
- Woodhouse 2021, S. 6 (PDF 6): „the same Skudrzyk mean level would be expected for them all (Skudrzyk, 1980)“
  `[Textebene]`. Die Formel selbst steht in Skudrzyk 1980 (JASA 67) `[nicht gelesen, Ladeliste]`.

**(P5) Freies Leistenende.** Am freien Ende eines halbunendlichen Balkens ist |Y| = 4 |Y∞| (+12 dB). Die Erhöhung klingt
über etwa λ_B/2 ab.
- Hüllkurve 1 + 3 e^{−k_B x}: `[Lehrbuch, nicht gelesen; Hüllkurve Annahme]`.

### 1.3 Messweg (Lücke 1, Gegenrechnung)

**(M1) Nachgiebigkeit in Reihe zum Messpunkt.** Y_gemessen = Y_Boden + jω/k_c. Ist k_c klein, steigt Y_gemessen ∝ f
(+6 dB/Oktave), gleich, welcher Boden darunter liegt.
- `[hergeleitet]`.

**(M2) Bells Eichung.** „a final additional frequency sweep was made with the vibrator removed from the soundbox and a
200g mass fastened to the impedance head … the absolute value of the admittance of this mass could be calculated“.
- Bell S. 130–131 (PDF 241–242) `[Textebene]`. Dazu |Y| = 1/(ωm) `[Lehrbuch]`.
- Gleiche Stelle: Impedanzkopf B+K 8001, ein „small pin“ zwischen Kopf und Kasten; „reproducibility error of less than
  1dB“ (S. 128) `[Textebene]`.

### 1.4 Dämpfung je Mode (Lücke 3)

**(D1) Gesetz in LumiViz.**
- η = α(f)/(2πf) mit α = a f² + b f + c (`Bauplan.hpp`, `Daempfung`) `[wörtlich]`.
- Gekoppelt mit der Luft: „der Verlustfaktor ist der der Platte mal ihrem Anteil plus Anteil der Luft / Guete“
  (`Luftraum.hpp`, Kommentar zu `koppleMitLuftraum`) `[wörtlich]`.
- Also η_k = (1 − a_k) α(f_k)/(2π f_k) + a_k/Q_Luft.
- Am Körper nachgerechnet: Mit dem Luftanteil a_k aus der Datei ergibt sich für alle drei tiefen Moden genau
  Q_Luft = 5,5 `[eigene Rechnung, R6]`.

**(D2) Abstrahlungsverlust einer Mode als kleine Quelle.** `[hergeleitet aus der Leistung eines Monopols, Lehrbuch]`

```
η_rad = ρ ω V² / (4π c)    (Vollraum)
η_rad = ρ ω V² / (2π c)    (Halbraum)
```

V ist der Volumenfluss je Modalkoordinate (m³/√kg).

**(D3) Dasselbe in Punktgrößen.** `[hergeleitet]`

```
Q_rad = ω M_P / R,    R = ρ ω² S_eff² / (4π c)
```

M_P ist die Modalmasse am Punkt (1/φ²), S_eff die wirksame Fläche je Punktschnelle (A2).

**(D4) Zwei Freiheitsgrade um T1/A0 (Camac).** Le Carrou 2006, S. 71, Gl. (4.19)–(4.21) `[am Bild geprüft]`.
q = (z₅, w₃₄): z₅ ist der Luftkolben in der untersten Öffnung, w₃₄ der Boden an Punkt 34.

```
M q̈ + B q̇ + K q = f                                         (4.1)
M = [[0,54, −5,02], [−5,02, 49,22]] kg
B = [[5,37, −19,67], [−19,67, 205,31]] kg/s
K = [[590,41, −5366,46], [−5366,46, 51310,00]] · 10³ kg/s²
```

- Nachgerechnet: s = −31,9 + j 974,3 und −32,6 + j 1067,8, also 155,2 Hz (Q 15,3) und 170,0 Hz (Q 16,4). Tab. 4.2 gibt
  −31,9 + j 976,6 und −32,5 + j 1065,9 `[eigene Rechnung, R10]`.
- Le Carrou selbst schreibt: „les valeurs numériques de ces matrices ne sont que des ordres de grandeurs“ (S. 71); der
  Bezug zu physikalischen Größen „n'a pu être clairement défini“ (S. 74) `[am Bild geprüft / Textebene]`.

### 1.5 Abstrahlung (Lücke 4)

**(A1) Monopol im Fernfeld.** p(r) = jωρ U e^{−jkr}/(4π r), U = U_Boden + Σ U_Öffnungen mit Vorzeichen.
- `[Lehrbuch, nicht gelesen]`.
- Le Carrou 2007, S. 563, für die gemessene Harfe: „In the frequency range 50–220 Hz, the directivity patterns are
  found to be nondirectional“ `[Textebene]`.
- Le Carrou 2010, S. 3205: „below 220 Hz, the concert harp is non-directional … above 220 Hz, more complex patterns
  with lobes appear“ `[wörtlich, aus Stufe 1]`.

**(A2) Wirksame Fläche aus Messung.** S_eff = (p/v) · 4π r/(ρ ω). `[hergeleitet aus A1]`

**(A3) Modell, Druck je Kraft am Hörort.** `[hergeleitet aus den Einheiten der Datei]`

```
p/F = Σ_k r_k · (−ω²) · φ_k(Öse) / (ω_k² − ω² + j η_k ω_k²)
```

- r_k ist laut Datei „complex pressure per modal acceleration at the listening point, sqrt(kg)/m^2 (Rayleigh, baffled
  plate)“ (`concert_harp.lvinstbody`, `units`) `[wörtlich]`.
- `Luftraum.hpp` (Kommentar zu `LochLuft`): ein Loch der Platte strahlt „in den Halbraum (ρ/2π)“, eine Öffnung
  außerhalb der Platte „frei in den ganzen Raum (ρ/4π)“ `[wörtlich]`.
- Gegenprobe: Mit derselben Synthese ergibt sich |Y| an den Ösen 33/21/12/10 zu 69,5/68,0/69,2/68,0 dB. S201 nennt
  69,7/68,2/68,7/66,8 dB `[eigene Rechnung, R6]`.

**(L1) Masse am Punkt aus Massenbeladung.** M = Δm/((f/f_m)² − 1). `[hergeleitet]`
- Le Carrou 2007 Tab. II: 153,5 → 148,5 Hz mit „a mass m = 200 g on both sides of the central line“ `[Textebene]`.
- Ergebnis: 2,9 kg (200 g gesamt) bzw. 5,8 kg (2 × 200 g). Die Lesart ist nicht entschieden.

---

## 2 Parametertabelle

| Größe | Zahl | Einheit | Quelle | Marke | gilt für |
|---|---|---|---|---|---|
| Saitenwinkel gegen den Boden | 30 | ° | Bell S. 157 (PDF 283) | [am Bild geprüft] | Salvi „Orchestra“ |
| Saitenwinkel (Modell Le Carrou) | ≈ 40 | ° | Le Carrou 2006 S. 101 | [Textebene] | Konzertharfe allgemein |
| Saiten: f, Durchmesser, Länge (46) | Tab. 2.1/2.2, siehe R1 | Hz, mm, m | Bell PDF 69–70 | [am Bild geprüft] | Salvi |
| Metallsaiten: Kern/Umspinnung | 0,80–1,30 / 0,24–0,68 | mm | Bell Tab. 2.2 | [am Bild geprüft] | Saiten 34–44 |
| Zug je Saite | 53–322 (Darm, Nylon), 377–463 (Metall) | N | Bell Abb. 2.10 (PDF 68), abgelesen ±8 N | [am Bild geprüft] | Salvi |
| Summe der Züge | 9786 aus Abb. 2.10; Bell nennt 9350 | N | Bell S. 38 (PDF 58) | [eigene Rechnung] / [Textebene] | |
| Normallast auf die Saitenleiste | Σ T sin 30° = 4,9 | kN | aus den Zeilen oben | [eigene Rechnung] | |
| Dichte Darm | 1320–1325 | kg/m³ | Lynch-Aird & Woodhouse 2018, Tab. (PDF 4) | [Textebene] | Darmsaiten |
| Dichte Nylon | 1067–1160 | kg/m³ | Lynch-Aird & Woodhouse 2017, Tab. (PDF 7) und Abb. 11 | [Textebene] | |
| E Darm | „about 5.5–6.5 GPa“ (allgemein berichtet) | GPa | Lynch-Aird & Woodhouse 2018 PDF 7 | [Textebene] | gerechnet mit 6 |
| E Nylon | 4 bis über 10, abhängig von Spannung und Zeitskala | GPa | Lynch-Aird & Woodhouse 2017 PDF 15 | [Textebene] | gerechnet mit 6 [Annahme] |
| Metallsaite: Stahlkern 200 GPa, 7850 kg/m³; Kupfer 8900 kg/m³, Füllgrad π/4 | – | | – | [Annahme] | Zug aus S1 dann 13 % über Abb. 2.10 |
| Z₀ = √(Tμ) je Saite | 0,11 (00) … 1,26 (33) … 4,0 (44); Summe 51,7 | N s/m | R1 | [eigene Rechnung] | |
| Längsfeder EA/L je Saite | 14–24 (Darm, Nylon), 82–180 (Metall); Σ sin²30° · EA/L = 0,47 MN/m | kN/m | R1 | [eigene Rechnung] | E ±30 % → ±30 % |
| erste Längsresonanz c_l/(2L) | 846 (Saite 44) … 966 (33) … 16 000 (00) | Hz | R1 | [eigene Rechnung] | |
| Geometrische Steife T/L | 250–750 | N/m | R1 | [eigene Rechnung] | |
| Verbundbalken an der Öse: EI / m′ | 1293 / 0,89 (y 0,39 m); 455 / 0,60 (0,72 m); 165 / 0,36 (0,95 m) | N m² / kg/m | R2 aus Bauplan (Deck-, Zugleiste, Boden) | [eigene Rechnung] | Buche nach Bauplan |
| Y∞ Balken + Platte an den Ösen 33–10 | 60,8–64,4 | dB re Bell | R2 | [eigene Rechnung] | |
| Y∞ Platte allein, Ösen 33–10 | 69,4–77,2 | dB | R2 | [eigene Rechnung] | |
| Y∞ Platte am Diskantende, Holz 3 / 2,5 / 2 mm + 1 mm Furnier | 83,3 / 85,6 / 88,3 | dB | R9 | [eigene Rechnung] | |
| Reihenfeder, die Bell 7.22 erklärt | 5,2·10⁴ (Streuung der Einzelwerte 3,5 dB) | N/m | R8 | [eigene Rechnung] | Hypothese |
| Bells Messkette | B+K 8001, Stift, 200-g-Eichung; Bleimasse 2,3 kg, Erreger mit Kopf und Stift 1,1 kg | – | Bell S. 128–131 | [Textebene] | |
| Q der Helmholtz-Mode, eine Öffnung, Kasten eingegraben | 10 / 9 / 9 / 11 / 10 bei 75 / 87 / 94 / 88 / 86 Hz (Öffnungen 6, 1, 2, 3, 4) | – | Bell S. 76 (PDF 134) | [am Bild geprüft] | Kasten starr (Sand) |
| Helmholtz alle offen, starr | 190 | Hz | Bell S. 85 (PDF 143) | [Textebene] | Q dazu nicht gefunden |
| Holzgesetz in LumiViz | a 2,6·10⁻⁴, b 0, c 20,5 | 1/s | `make_harfe.py` | [wörtlich] | angepasst |
| Q_Luft in LumiViz | 5,5 | – | `make_harfe.py`, KQ29 a | [wörtlich] | angepasst |
| Luftanteil der Moden 129,0 / 191,8 / 208,4 Hz | 0,305 / 0,328 / 0,334 | – | Körperdatei | [wörtlich] | |
| Modalmasse an Öse 32 (Moden 129 / 192 / 208 / 275 Hz) | 0,74 / 3,63 / 1,65 / 2,28 | kg | Körperdatei, 1/φ² | [eigene Rechnung] | |
| 2-FHG Camac (T1/A0) | M, B, K wie (D4); Masse an w₃₄ quasistatisch 2,55 kg | | Le Carrou 2006 S. 71 | [am Bild geprüft] | Camac, besaitet |
| Masse an Punkt 34 aus Abb. 3.5 (Q 22, 6,8·10⁻³ s/kg, 152 Hz) | 3,4 | kg | Stufe 1 / R7 | [eigene Rechnung] | |
| wirksame Fläche je Punktschnelle (Camac, 2,35 m) | 0,053 / 0,071 / 0,089 / 0,080 / 0,066 bei 152 / 168 / 204 / 245 / 295 Hz | m² | R7 aus Le Carrou 2007 Abb. 7 und Thèse Abb. 3.5 | [eigene Rechnung] | Werte der Abbildungen ±0,4·10⁻³ |
| dasselbe im Modell (Ösen 32/33, Hörort 1 m, auf 2,35 m umgerechnet) | 0,30 / 1,11 / 0,50 / 0,70 / 0,50 | m² | R7 | [eigene Rechnung] | Hörort im Nahfeld |
| Halsquerschnitt, Säule, Steifigkeit des Gestells | – | | nicht gefunden | | |

---

## 3 Lücke 1: der besaitete Zustand

### 3.1 Einzelterme an der eigenen Öse (R1)

| Saite (f) | Längs \|Z_l\| sin² | Quer gedämpft Z₀ cos² | Quer ungedämpft \|Z_t\| cos², η_s 0,01, mit gemessenem Zug | Feder sin² EA/L als \|Z\| = k/ω | Boden \|Z\| = 1/\|Y\| (Modell) |
|---|---|---|---|---|---|
| 33 (104 Hz) | 8,7 | 0,95 | 7,6 | 9,0 | 207 |
| 26 (207 Hz) | 3,4 | 0,63 | 14,7 | 3,7 | ≈ 47 (auf der Mode) |
| 21 (330 Hz) | 2,0 | 0,40 | 7,9 | 2,1 | ≈ 250 |
| 12 (830 Hz) | 0,6 | 0,18 | 1,0 | 0,7 | ≈ 220 |
| 10 (988 Hz) | 0,5 | 0,17 | 0,4 | 0,6 | ≈ 250 |

Alle Werte in N s/m. **Örtlich sind die Terme meist 1–10 % der Bodenimpedanz**, höchstens 30 % (ungedämpfte Saite 26
auf der Mode bei 208 Hz) `[eigene Rechnung]`.

- **Ungedämpfte Saite genau auf ihrer Frequenz:** Nach (S6) wären es 2 Z₀/(π η_s) · 0,75. Für Saite 33 mit η_s 0,01
  sind das 60 N s/m, die Admittanz der eigenen Öse sinkt dann um etwa 3 dB.
- **Mit gemessenem statt gerechnetem Zug** ist die Saite verstimmt (k_t L ≠ π), dann sind es nur 7,6 N s/m.
- Das gilt für den Spielzustand. Bell und Le Carrou haben mit gedämpften Saiten gemessen (Stufe 1).

### 3.2 Alle 46 Saiten auf die Moden (S7; R8, R9)

| Mode des Körpers | f heute | Längsfeder, Hals starr | Querfeder T/L | gedämpfte Saiten: Q heute → besaitet |
|---|---|---|---|---|
| S(1,1)− | 129,0 | 158,7 (+23,0 %) | +0,42 % | 13,0 → 8,6 |
| (Luft, geteilt) | 191,8 | 214,3 (+11,8 %) | +0,26 % | 13,1 → 9,8 |
| (Luft, geteilt) | 208,4 | 218,6 (+4,9 %) | +0,22 % | 13,0 → 11,0 |
| S(1,2) | 274,8 | 291,7 (+6,2 %) | +0,32 % | 35,5 → 22,6 |
| S(1,3) | 346,7 | 362,2 (+4,5 %) | +0,31 % | 41,9 → 28,0 |
| S(1,4) | 430,8 | 445,4 (+3,4 %) | +0,26 % | 39,2 → 28,8 |
| S(1,5) | 526,8 | 538,4 (+2,2 %) | +0,18 % | 35,7 → 28,4 |
| (S(1,6)/S(1,7)) | 640,7 / 767,4 | +1,2 / +0,7 % | ≤ 0,1 % | 31,6 → 27,3 / 27,8 → 25,2 |

**Gegen Bell (Kasten G in der Harfe, ohne → mit Saiten; S197 §8):**

| Mode | Frequenz gemessen | Güte gemessen | passt zur Rechnung? |
|---|---|---|---|
| S(1,1)− | 170 → 187 Hz (+10 %) | – | Rechnung mit starrem Hals +23 % |
| S(1,1)+ | 216 → 207 Hz (−4 %) | – | Federn können nicht senken |
| S(1,2) | +3 % | 39 → 32 | |
| S(1,3) | +6 % | 45 → 27 | |
| S(1,4) | 0 % | 43 → 28 | |
| S(1,5) | +2 % | 31 → 19 | |
| S(1,7) | −0,5 % | 33 → 17 | |

Daraus `[eigene Einschätzung]`:
- **Die Längsfedern erklären Richtung und Größenordnung der Verschiebung nach oben.** Bei starrem Hals sind sie für
  S(1,1)− etwa 2,5-mal zu stark.
- **Nach (S8) ergibt sich γ ≈ 0,4**, damit k_G ≈ 0,3 MN/m in Normalrichtung. Die statische Durchbiegung unter
  4,9 kN Normallast wäre dann rund 15 mm. Das ist für einen Hals viel. Wahrscheinlicher geben auch die Schale (3 mm)
  und die Blöcke nach. Belegt ist nichts davon.
- **Die gedämpften Saiten mit cos²30°** treffen die Güten von S(1,3) und S(1,4) (28/29 gegen 27/28). S(1,2) wird zu
  stark gesenkt (23 gegen 32), S(1,5)–S(1,7) zu wenig (28/25 gegen 19/17).
- In S198 §11 hieß es „0,25 Z ist zu klein; erst 3 Z erreicht die Güten“. Mit dem richtigen Winkelfaktor 0,75 ist es
  das Dreifache von 0,25 Z, also ein Teil der Lücke.

### 3.3 Statische Last und Vorspannung (S9)

- Die Normallast auf der Saitenleiste ist **4,9 kN** (aus Abb. 2.10, α = 30°), die Last längs des Bodens 8,5 kN.
- Die Euler-Last der Leisten allein liegt bei P_cr = π² EI/L² ≈ 2,9 kN (EI ≈ 500 N m², L = 1,31 m). Die Längslast muss
  also im Wesentlichen die Schale tragen.
- Welcher Teil im Boden bleibt und ob er dort zieht oder drückt, ist mit den gelesenen Quellen **nicht herleitbar**.
- Zur Größenordnung: Bei 1 kN Längskraft im Boden ergibt (S9) für S(1,1) (k ≈ π/1,31 m⁻¹) N/(EI k²) ≈ 0,3. Bei Druck
  sinkt die Frequenz um bis 16 %, bei Zug steigt sie um bis 14 %.
- Le Carrou 2006 S. 14 (Kap. 1) über die Leisten: sie „empêchent ainsi la table de bomber et de se déformer
  excessivement sous la tension“ `[Textebene]`. Eine Zahl zur Durchbiegung gibt es nicht.
- **Offener Term, möglicherweise groß; nicht in den Optionen beziffert.**

### 3.4 Kann eine Kopplung über Saiten die Punkt-Admittanz heben? Die Gegenrechnung

**(a) Örtlich (3.1):** nein. Jede passive Last an der Öse liegt parallel zum Boden und ist um eine Größenordnung
kleiner.

**(b) Global über die Moden (R9):**

| Öse (f) | ohne | Längsfeder starr | Feder × 0,4 | gedämpft | Feder × 0,4 + gedämpft | Bell 7.22 |
|---|---|---|---|---|---|---|
| 33 (104 Hz) | 69,5 | 63,3 (−6,2) | 66,3 (−3,2) | 69,3 (−0,2) | 66,2 (−3,3) | 80 |
| 21 (330 Hz) | 68,0 | 53,5 (−14,5) | 62,1 (−5,9) | 68,0 (+0,1) | 62,8 (−5,2) | 95 |
| 12 (830 Hz) | 69,2 | 69,9 (+0,7) | 69,5 (+0,3) | 69,2 (0,0) | 69,5 (+0,2) | 103 |
| Median Saiten 33–10 | – | +1,4 (−14,5 … +22,0) | – | 0,0 (−1,7 … +2,7) | +0,2 (−7,3 … +9,9) | – |

Alle Werte in dB re Bell. Die Federn verschieben Spitzen und Täler über die Saiten, ohne zu heben. Die Dämpfung ebnet
um wenige dB ein.

**(c) Mittelwert (P1–P4, R2):**

| Ösen 33–10 | Median gegen Y∞ (Balken + Platte) | Bereich |
|---|---|---|
| Modell (S201) | +6,3 dB | −10 … +27 |
| Bell 7.22 | +27,0 dB | +13 … +39 |
| Le Carrou, Punkt 34, 104/117 Hz (Camac) | 51,6/56,0 dB, also −5 … −9 dB gegen die Salvi | – |

Die Platte ohne jede Leiste käme an den Ösen auf 69–77 dB. Bell liegt auch darüber, und zwar um 4–27 dB (Median 15 dB).

**(d) Messweg (M1, R8):** Abb. 7.22 steigt mit 20,1 dB/Dekade = **6,0 dB/Oktave**. Eine Reihenfeder k_c = 5,2·10⁴ N/m
gibt mit Y = ω/k_c:

| | 104 Hz | 207 Hz | 330 Hz | 494 Hz | 740 Hz | 988 Hz |
|---|---|---|---|---|---|---|
| Reihenfeder | 78 dB | 84 dB | 88 dB | 91 dB | 95 dB | 98 dB |
| Bell | 80 dB | 86 dB | 95 dB | 91 dB | 99 dB | 99 dB |

Streuung 3,5 dB, größter Rest +10 dB an Saite 32.

- Als Steife des Bodens wären 5·10⁴ N/m um mehr als eine Größenordnung zu weich: die Modalmassen von 0,7–3,6 kg bei
  130–430 Hz geben 0,5–24 MN/m.
- Mögliche Orte einer solchen Nachgiebigkeit `[eigene Einschätzung, nicht belegt]`:
  - der Kontakt des Stifts. Bell setzte ihn besaitet „as close to the centre of the cover bar as the strings would
    allow“, mit dem Erreger „at a slight angle to the plane of the strings (<10°)“ (S. 147, Stufe 1);
  - die zwischen die Saiten gewebten Kunststoffbänder.
- Bells Eichung mit 200 g (M2) prüft nur Kopf und Elektronik, nicht den Kontakt am Kasten.
- **Unbesaitet** stimmen Bells Täler an Messort 5 (66–72 dB) und die Spitze an Ort 10 (100 dB) mit dem Modell
  überein (S198 §11) und liegen nahe Y∞.

**(e) Ergebnis:** Der Abstand von 10–37 dB gehört nach diesen Rechnungen **nicht** zum Boden. Er stammt weder von den
Saiten noch von einem zu steifen Boden:
- Ein Boden, der Bells Mittel erreicht, müsste an den Ösen eine 20- bis 30-mal kleinere Impedanz haben als Deck- und
  Zugleiste mit Bells eigenen Maßen.
- **Am wahrscheinlichsten ist ein Unterschied in Bezug oder Messweg** der besaiteten Messung von Bell
  `[eigene Einschätzung]`.
- Größe und Einheit sind bei Bell und Le Carrou dieselben (Stufe 1: Betrag der Punkt-Admittanz, s/kg = m/(N s)).
- Einen Realteil nennt keine der beiden Quellen. Transfer statt Punkt kommt bei Abb. 7.22 nach Bells Text nicht in
  Frage („at the string position and frequency“, Stufe 1).

### 3.5 Kopplung an Hals und Säule

- **Hals starr:** (S4)–(S7) mit den Zahlen oben.
- **Hals nachgiebig:** (S8). Gebraucht würden:
  - die Biegesteife EI und die Masse je Länge des Halses (Querschnitt, Holz, Länge zwischen Säule und Kasten);
  - Längssteife und Masse der Säule;
  - die Steife der Verbindungen zu Kasten und Fuß;
  - die Lage der Stimmwirbel (Saitenende) am Hals.
- In Bell (Kap. 2, Suche „column“, „neck“) und Le Carrou (Kap. 1–3, „pillar“, „console“, „colonne“) steht keine dieser
  Zahlen.
- Indirekt gibt es dies:
  - die Moden der ganzen Harfe: Bell B1 85 / 92 Hz ohne / mit Saiten, S. 155 `[wörtlich, Stufe 1]`;
  - Le Carrou: Moden 1–3 (61,5 / 84,5 / 124,5 Hz) „depending on its connections to the arm and to the bottom of the
    pillar“ (2007, S. 562) `[Textebene]`;
  - γ ≈ 0,4 aus 3.2.
- Mit einem Balkenmodell des Halses ließe sich k_G ersetzen, sobald Maße vorliegen (Ladeliste, Abschnitt 6).

---

## 4 Lücke 2: das Diskantende

| Maßnahme | Gleichung | Wirkung an den Ösen der obersten 10 Saiten (Z1) | Tasten 92–94, Ösen 5/6, y 1,09–1,12 m (Z2) |
|---|---|---|---|
| (a) Boden 2–2,5 mm statt 3 mm (+1 mm Furnier) | P1, P3 | Platte +2,3 bis +5,0 dB, an der Öse wegen der Leisten < 1 dB | < 1 dB |
| (a′) E quer/längs geändert | P1: Y∞ ∝ (D_x D_y)^{−1/4} | −10 % E → +0,5 dB | – |
| (b) Kopfblock nachgiebig (Weg- und Drehfeder je Länge) | Randabschnitt in `Bauplan.hpp` (`wegfeder`, `drehfeder`) | nicht herleitbar: Maße von Block und Verleimung nicht gefunden. S198 §12: 3·10⁶ N/m² angepasst; an Ort 2 schließt das 5–10 dB von 17–22 dB | – |
| (c) Leistenenden frei bzw. verjüngt | P5 | Saiten 00–2 bis +6 dB (2960 Hz, 3 cm vor dem Ende); Saite 5 (1661 Hz, 16 cm, λ_B 0,28 m) +0,6 dB; Saite 10 +0,2 dB | < 1 dB |

Alle Wirkungen sind `[eigene Rechnung, R9]`.

**Belege zum Diskantende:**
- Dicke am Diskant:
  - Waltham 2008: „2-2.5mm at the top“ `[wörtlich, Stufe 1]`;
  - Camac-Blog: „to 2mm at the top“ `[Werkzeug, Stufe 1]`;
  - der Bauplan rechnet mit 3 mm + 1 mm Furnier (Bell Abb. 2.4 a).
- Kopfblock und Leistenenden: in keiner Quelle Maße (Stufe 1, hier nicht weiter gefunden).

**Folgerung** `[eigene Einschätzung]`:
- Das Diskantende kann die Lücke zu Bell an den Saiten 10–13 (30–34 dB) nicht schließen. Nach 3.4 gehört diese Lücke
  ohnehin wohl nicht zum Boden.
- Für die Spanne der Lautheit (Z2) liefern (a)–(c) an den Tasten 92–94 nach dem Mittelwert unter 1 dB.
- Was dort mehr bringt, sind modale Wirkungen nahe der weichen Kante. S198 §11: eine freie Diskantkante +5 bis
  +10 dB an S(1,7)/S(1,8). Diese hängen an der angepassten Feder, nicht an einer belegten Zahl.
- Die Abschätzung über die nächste Mode (Masse/Steife) braucht die Formen bei 1,5–1,9 kHz an den Ösen 5/6. Sie liegen
  in der Körperdatei, sind hier aber nicht nach Moden aufgeschlüsselt.

---

## 5 Lücke 3: Dämpfung je Mode

### 5.1 Zerlegung der heutigen Güten (D1, R6)

| Mode | f [Hz] | Q heute | Luftanteil | η Holz (1−a)·α/2πf | a/Q_Luft (5,5) | η_rad Halbraum / Vollraum aus r_k (D2) |
|---|---|---|---|---|---|---|
| S(1,1)− | 129,0 | 13,0 | 0,305 | 0,021 | 0,056 | 0,0011 / 0,0022 |
| – | 191,8 | 13,1 | 0,328 | 0,017 | 0,060 | 0,013 / 0,027 |
| – | 208,4 | 13,0 | 0,334 | 0,016 | 0,061 | 0,026 / 0,052 |
| S(1,2) | 274,8 | 35,5 | 0,031 | 0,022 | 0,006 | 0,017 / 0,034 |
| S(1,3) | 346,7 | 41,9 | 0,001 | 0,024 | 0,000 | 0,002 / 0,004 |

**Wie η_rad hier zustande kommt:**
- Der Volumenfluss je Modalkoordinate V ist aus dem Druck am Hörort zurückgerechnet (1 m, Rayleigh), mit V = 2π r |r_k|/ρ
  bzw. 4π r |r_k|/ρ. Das gibt eine Spanne, keinen Wert.
- Die Rückrechnung zählt die Öffnungen (ρ/4π) halb. Der Hörort in 1 m liegt im Nahfeld eines 1,31 m langen Bodens.

**Zweiter Weg (D3)** mit Le Carrous gemessener wirksamer Fläche (0,05–0,09 m²) und der Modalmasse an Öse 32:
Q_rad = 400–4300 (R7) `[eigene Rechnung]`.

**Erster Grundsatz** mit der ganzen Fläche 0,33 m² und einer Form wie ein halber Sinus: V ≈ 0,15 m³/√kg, also
Q_rad ≈ 120 bei 208 Hz `[eigene Rechnung]`.

### 5.2 Gegen die Messung

**Luft.** Bell misst Q 9–11 für die Helmholtz-Mode mit einer offenen Öffnung im eingegrabenen Kasten (S. 76).
- Ein Kasten in 120 kg Sand ist nicht verlustfrei; die Güte schließt also Verluste an die Wand ein.
- Für die Luft allein mit starren Wänden geben die Standardformeln (Reibung in der Öffnung, Wärmeleitung an der Wand,
  Abstrahlung der Öffnung) für Öffnung 1 bei 87 Hz Q ≈ 60 `[Lehrbuch, nicht gelesen; eigene Überschlagsrechnung,
  nicht als Skript im Anhang]`.
- **Die gemessenen 9–11 sind also vor allem Wandverlust** (Schale, Sand) `[eigene Einschätzung]`.

**Rechnung mit Q_Luft = 10 (Bell) statt 5,5** `[eigene Rechnung]`:

| Mode | η Holz + Luft | Q | mit η_rad 0,013 (Q_rad ≈ 77, Vollraum aus V_Halbraum) | Bell (in der Harfe) |
|---|---|---|---|---|
| 129,0 Hz | 0,052 | 19,3 | – | S(1,1)− Q 11 |
| 191,8 Hz | 0,050 | 20,2 | 17,8 (mit η_rad 0,0067) | – |
| 208,4 Hz | 0,050 | 20,2 | 16,0 | S(1,1)+ Q 16 |

**Le Carrou (Camac, besaitet):**
- Abb. 3.4: T1 ζ 2,3 % (Q 22), A0 1,4 % (Q 36) `[Stufe 1, am Bild]`.
- Tab. 4.1: Q 19 und 19,5; 2-FHG (D4): Q 15,3 und 16,4.
- Die drei Auswertungen derselben Harfe streuen bis zum Faktor 2.

**Anteile** `[eigene Einschätzung]`:
- **S(1,1)+:** Holz (Gesetz) etwa ein Drittel, Luft und Wand etwa die Hälfte, Abstrahlung 0–25 %.
- **S(1,1)−** (Bell Q 11): Mit Q_Luft 10 fehlen η ≈ 0,04. Das ist nicht erklärt.
  - Bell: in S(1,1)− bewegt sich am losen Kasten „the entire treble end of the box“; in der Harfe wird „the motion of
    the entire box“ behindert (S. 150, Stufe 1).
  - Le Carrous Moden der ganzen Harfe haben ζ 4,8–5,5 % (Q 9–10).
  - Ein Verlust im Gestell bei Moden, an denen der ganze Kasten teilnimmt, ist naheliegend, aber nicht belegt.

### 5.3 Abbildung in LumiViz

**Heute:** η = (1−a) α(f)/2πf + a/Q_Luft (D1). Q_Luft = 5,5 steht für Luft, Wand, Gestell und Abstrahlung zusammen.

Sauberer wären drei getrennte Terme `[eigene Einschätzung]`:

| Term | Gleichung | Daten |
|---|---|---|
| (i) Luft mit Wandverlust | a/Q_Luft | Bell gemessen 9–11 |
| (ii) Abstrahlung je Mode | D2 mit V aus dem Volumenfluss der gekoppelten Form (Boden + Öffnungen mit Vorzeichen), Vollraum | rechnet die Bibliothek schon in `Luftraum.hpp` (Volumen a_k, Öffnungen); fehlt nur als Verlust |
| (iii) Verlust im Gestell | Güte oder η je Mode, gewichtet mit dem Anteil der Bewegung des ganzen Kastens | keine Zahl belegt; Le Carrou ζ 5 % an den Moden der ganzen Harfe als Größenordnung |

Ein Gesetz α(f) allein bildet das nicht ab. (ii) wächst mit ω V², (i) und (iii) hängen am Luftanteil bzw. an der Form.

---

## 6 Lücke 4: Abstrahlung im Fernfeld je Kraft

**Gesetz unter 220 Hz:** (A1). Ein Monopol aus Boden und Öffnungen; „non-directional“ (Le Carrou 2007, 2010).
- T1: Öffnungen und Boden etwa gleichphasig. A0: etwa gegenphasig. Le Carrou 2007, S. 566: „For T1, the displacements
  of these pistons are approximately in phase with the bending displacement of the soundboard. For A0, these motions are
  approximately out of phase“ `[Textebene]`.
- Le Carrou 2007, S. 563: die Kolben sind gleichphasig unter f_B = 160 Hz; zwischen f_B und f_A = 250 Hz wächst die
  Phase zwischen Boden und Kolben „from 0° to 180°“ `[Textebene]`.

**Über 220 Hz:** Keulen (Le Carrou 2010). Ein Monopol trägt dort nicht. Bis 2 kHz ist kein Gesetz aus einer Quelle
belegt.

**Modell:** (A3). Platte im Halbraum (ρ/2π), Öffnungen im Vollraum (ρ/4π), ohne Rückseite (`backside` aus).

**Zahlenvergleich** (R6, R7; Anregung an den Ösen 30/31 bzw. 32/33, Modell auf 2,35 m mit 1/r umgerechnet):

| f [Hz] | Le Carrou 2010 (S2, Saiten 30/31), 2,35 m [dB re 2·10⁻⁵ Pa/N] | Modell Ösen 30/31 | Abstand |
|---|---|---|---|
| 134 | 40 | 69,3 | +29 |
| 148 | 57 | 65,6 | +9 |
| 166 | 60 | 67,1 | +7 |
| 195 | 62 | 74,8 | +13 |
| 290 | 26 (Einbruch) | 72,6 | +47 |
| 384 | 58 | 59,8 | +2 |
| 460 | 50 | 57,4 | +7 |
| 582 | 46 | 60,6 | +15 |
| 644 | 54 | 70,5 | +17 |
| 756 | 53 | 54,7 | +2 |
| 892 | 48 | 61,6 | +14 |
| 1000 | 40–44 | 62,8 | +19…+23 |

| Z4: 204 Hz gegen 240–250 Hz | Le Carrou 2007 (Punkt 34) | Modell Öse 33 / 32 | Öse 31 / 30 | Öse 26 |
|---|---|---|---|---|
| Unterschied | +16,6 dB | +7,2 / +7,6 dB | +8,1 / +8,7 dB | +12,1 dB (207 gegen 247 Hz: +13,3) |
| p/F bei 204 Hz in 2,35 m | 0,027 Pa/N | 0,12 / 0,14 Pa/N | – | – |

**Normiert auf die Schnelle** (A2, R7) bleibt das Modell um den Faktor 5–10 (14–20 dB) über Le Carrou.
- Die wirksame Fläche wird im Modell bis 1,1 m². Der Boden hat 0,33 m², die Öffnungen haben 0,045 m².
- Das deutet auf drei Ursachen `[eigene Einschätzung]`:
  - die Schallwand (+6 dB für die Platte);
  - den Hörort in 1 m im Nahfeld (das Umrechnen mit 1/r auf 2,35 m ist dann zu grob);
  - Moden, deren Knoten nahe der Öse liegt, so dass p/v dort groß wird (168 Hz: |Y| = 56,7 dB).

**Der Terzverlauf p/F** (Öse 31, 1 m) ist 63 Hz 43, 125 Hz 74, 200 Hz 84, 400 Hz 72, 1 kHz 69, 1,25 kHz 78, 2 kHz 68 dB.
Über 300 Hz tragen die höheren Moden bis +12 dB (R6).

**Le Carrous Kurve** bricht bei 290 und 850 Hz um 30 dB ein. Das Modell zeigt dort keinen Einbruch. Das ist ein
Richtungs- bzw. Auslöschungseffekt (Keulen über 220 Hz), den ein einzelner Hörort mit Schallwand nicht wiedergibt
`[eigene Einschätzung]`.

---

## 7 Annahmen, die bleiben

1. **Winkel 30°** für alle Saiten (Bell). Le Carrou nennt etwa 40° für die Harfe allgemein. Mit 40° wird sin² = 0,41
   (Längsfedern +65 %), cos² = 0,59 (Querwiderstand −21 %).
2. **Die E-Moduln der Saiten** (Darm, Nylon 6 GPa, Stahlkern 200 GPa) und der Aufbau der Metallsaiten sind
   angenommen. Der Zug aus (S1) liegt 13 % über Abb. 2.10, wegen der Metallsaiten.
3. **Die Saiten sind am Hals fest**, nur mit einem Faktor γ für das Gestell; γ ≈ 0,4 ist aus einer einzigen
   Verschiebung zurückgerechnet.
4. **Diagonale Näherung (S7):** Die Saiten koppeln die Moden nicht untereinander. Bei 0,47 MN/m gegen Modalsteifen von
   0,5–20 MN/m sind die Moden 129–208 Hz davon am stärksten betroffen.
5. **Der Verbundbalken (P3)** ist nur unter den Leisten gerechnet: Platte parallel am Punkt, Buche mit 13,7 GPa längs.
   Eine breitere mittragende Platte senkt Y∞ weiter.
6. **Der Mittelwertsatz (P4)** gilt für das Mittel über die Frequenz, nicht für jede einzelne Saite.
7. **Der Bezug von Bell Abb. 7.22** ist im Bild nicht angegeben. S197 setzt 116 dB = 1 s/kg (aus Kap. 6) voraus, und
   dieser Bericht ebenso.
8. **Die Reihenfeder (M1)** ist eine Hypothese, die zu den Zahlen passt. Ein Beleg in Bells Text fehlt.
9. **Abstrahlungsdämpfung (5.1):** Der Volumenfluss ist aus dem Druck am Hörort zurückgerechnet, nicht aus `Luftraum.hpp`
   direkt (dort a_k, nicht in der Datei).
10. **Der Vergleich mit Le Carrou** überträgt Ergebnisse von der Camac (Kohlefaser, Aluminiumschiene) auf die Salvi.
11. **Bells Q 9–11 der Luft** gelten für einzelne Öffnungen bei 75–94 Hz im eingegrabenen Kasten. Bei allen offenen
    Öffnungen (190 Hz) und freier Schale ist keine Güte gefunden.

---

## 8 Optionen für LumiViz (keine Empfehlung nach Aufwand)

Zahlen sind `[eigene Rechnung]`, wo nicht anders vermerkt. Die Lautheit (Z2) ist hier nicht gerechnet; LU-Angaben
schließen von dB der Körperantwort auf LU der Taste (gleiche Größe angenommen).

**Option 1 – Saiten als Zusatz an den Ösen** (S5–S8): Längsfeder sin²α EA/L in Reihe mit dem Gestell (γ);
Querwiderstand cos²α Z₀ nur für den Vergleich „besaitet, gedämpft“; im Spielzustand keine Querlast.
- **Daten:** Tab. 2.1/2.2, Abb. 2.10, α 30°, E der Saiten, γ (aus S(1,1)− zurückgerechnet oder aus Halsmaßen).
- **Z1:** Median über die Saiten 33–10 +0,2 dB (−7 … +10 dB an einzelnen Saiten). Schließt die 10–37 dB nicht.
- **Z2:** verschiebt die Moden. Die laute Stelle um 208 Hz (Tasten 55–57) wandert mit γ = 0,4 um etwa +2 % bis +5 %
  (rund ein Halbton). Ob die Spanne sinkt, ist nicht gerechnet.
- **Z3:** Im Spielzustand ändern sich die Güten nicht (keine Querdämpfung). Im Messzustand fallen sie von 13 auf
  9–11, Bell hat dort Q unbestimmbar.
- **Z4:** Mit der Verschiebung der Spitze ±1–3 dB am Verhältnis (geschätzt).
- **Bringt:** die gemessene Verschiebung der tiefen Moden beim Besaiten. Vergleich gegen die besaitete Harfe:
  S(1,1)− 187 Hz, S(1,2) 257 Hz.

**Option 2 – Diskantende nachgiebiger** (P1, P3, P5; Boden 2–2,5 mm, Leistenenden verjüngt bis frei).
- **Daten:** Dicke am Diskant (Waltham, Camac; nicht von der Salvi „Orchestra“ selbst); Form der Leistenenden (nicht
  gefunden).
- **Z1:** obere Ösen 10–13 < 1 dB; Ösen 00–2 bis +6 dB.
- **Z2:** Tasten 92–94 nach dem Mittelwert < 1 dB; mehr nur über Moden nahe der Kante (heute angepasste Feder).
- **Z3, Z4:** keine Änderung (tiefe Moden).

**Option 3 – Dämpfung je Mode aus drei Termen** (5.3): Luft mit Q 9–11 (Bell gemessen), Abstrahlung aus dem
Volumenfluss jeder gekoppelten Mode (Vollraum), Verlust im Gestell je Mode.
- **Daten:** Bell S. 76; V je Mode (liegt in der Bibliothek); Gestell nur als Größenordnung (Le Carrou ζ 5 %).
- **Z3:** S(1,1)+ Q 16–18 (Bell 16). S(1,1)− Q 19 ohne Gestellterm (Bell 11); mit Gestellterm anpassbar, aber nicht
  belegt.
- **Z2:** die Tasten 55–57 werden mit Q 16–20 statt 13 um +1,8 bis +3,7 dB lauter. Das läuft der Spanne entgegen
  (heute 19,6 LU).
- **Z4:** +1,8 bis +3,7 dB am Verhältnis 204/245 Hz.
- **Z1:** an den Ösen der Spitzen ±2–4 dB, abseits nichts.

**Option 4 – Abstrahlung mit Monopol aus Boden und Öffnungen im Vollraum unter etwa 220–250 Hz** (A1), darüber die
heutige Rayleigh-Rechnung; Hörort im Fernfeld (≥ 2 m), wie bei Le Carrou.
- **Daten:** in der Bibliothek vorhanden (Volumen der Platte a_k, Öffnungen).
- **Z4:** Die Platte verliert gegen die Öffnungen den Faktor 2 (−6 dB). Bei A0 (gegenphasig) ändert sich die
  Auslöschung; die Zahl braucht die Aufteilung Platte/Öffnungen je Mode, die die Datei nicht trägt. Absolut rückt p/F
  um bis zu 6 dB auf Le Carrou zu (heute +7 bis +29 dB unter 200 Hz).
- **Z2:** Tasten unter etwa 250 Hz bis −6 dB, also auch die lauten Tasten 55–57. Das kann die Spanne von 19,6 LU
  verringern, aber nur, wenn die lautesten Tasten dort liegen (S197 §14: 55–57 mit −5 bis −7 LUFS, die leisesten 92–94
  mit −23 bis −25 LUFS). Geschätzt −2 bis −6 LU an der Spanne `[eigene Einschätzung]`.
- **Z1, Z3:** keine Änderung (Z3 nur, wenn Option 3 die Abstrahlung als Verlust nimmt).

**Zu Z1 als Zielgröße:** Nach 3.4 ist der Pegel von Bell 7.22 wohl kein Ziel für den Körper. Ob das Ziel ersetzt wird,
etwa durch Bells unbesaitete Profile und Y∞, entscheidet die Sitzung (Abschnitt 9).

---

## 9 Was die LumiViz-Sitzung prüfen muss

1. **Bell Abb. 7.6, Feld 3, gegen Feld 2 am Bild abgreifen** (Ort 17, mit / ohne Saiten, gleiche Achse): Steigen die
   besaiteten Werte auch dort ∝ f? Die Reihenfeder sagt +14 dB von 150 bis 800 Hz voraus, ohne Saiten gibt es keinen
   Trend.
2. **Bell Abb. 7.21 (besaitet, 50 Hz–10 kHz):** Steigt die Kurve bis zur Hochfläche mit 6 dB/Oktave (50 Hz → 1 kHz:
   +26 dB)? Stufe 1 las +20 bis +24 dB.
3. **Die besaitete Messung gegen Y∞** an einer zweiten Stelle: Bell Abb. 7.8–7.16 besaitet; Täler gegen 61–64 dB.
4. **Der Winkelfaktor der Saitenlast** in S198 (0,25 Z): Mit (S5) ist der Querterm 0,75 Z, der Längsterm 0,25 Z_l.
5. **Probe am Modell für Option 1:** die Längsfedern 0,47 MN/m mit γ 1 / 0,4 an die 46 Ösen legen, gegen Bell besaitet
   (S(1,1)− 187, S(1,2) 257, S(1,3) 334 Hz). Die diagonale Näherung hier gibt 159 / 292 / 362 Hz bei γ = 1.
6. **Die Summe der Züge:** Abb. 2.10 abgelesen 9786 N gegen Bells 9350 N (+5 %). Die Ablesung liegt bei ±8 N je Punkt;
   Saite 43 fehlt im Bild.
7. **Abstrahlung im Fernfeld:** einen zweiten Hörort in 2,35 m einführen und p/v gegen Le Carrou (R7) prüfen, bevor Z4
   bewertet wird.
8. **Die Luftgüte von Bell bei allen offenen Öffnungen:** Bildunterschrift Abb. 4.8 vi (PDF um 145–150) am Bild lesen.

---

## 10 Nicht gefunden / nicht herleitbar

| Was | wo gesucht | Ergebnis |
|---|---|---|
| Maße, Masse, Steife von Hals und Säule | Bell Kap. 2 (PDF 51–70: „column“, „neck“, „pillar“, „harmonic curve“); Le Carrou 2006 Kap. 1–3 („pillar“, „console“, „colonne“); Le Carrou 2007 ganz | nichts Zahlenmäßiges |
| Kopfblock: Maß, Holz, Verleimung | Stufe 1 (Le Carrou §1.2, Anh. A; Waltham; Camac-Blog); hier Bell Kap. 2 nicht weiter | nicht gefunden |
| Leistenenden (Form) | Bell Abb. 2.4 (S197); sonst nichts | nicht gefunden |
| Bezug der Achse von Bell Abb. 7.22 | Bild PDF 309; Text S. 159 (Stufe 1) | „Input admittance level (dB)“, ohne Bezug |
| Legende von Waltham 2008 Abb. 4 | Bild bei 300 dpi, 8-fach vergrößert; Textebene | untere Marke 10⁻³ oder 10⁻⁵ nicht entscheidbar (eingebettetes Rasterbild). Grauwerte an 18 Saitenpunkten: Stufen 2–5, das sind 72–85 dB (Lesart 10⁻³) oder 47–74 dB (10⁻⁵). Taugt so nicht als Schiedsrichter |
| Q der Helmholtz-Mode, alle Öffnungen offen | Bell PDF 136–146 („Q-“, „190“, „all the apertures“) | Frequenz 190 Hz, Q nicht im Text |
| Gesetz p/F über 220 Hz bis 2 kHz | Le Carrou 2007 ganz; Le Carrou 2010 nur Stufe 1; Bell Kap. 8 nicht gelesen | nicht gefunden |
| Längslast im Boden (Zug oder Druck, Anteil) | Bell Kap. 2; Le Carrou Kap. 1 | nur qualitativ (Le Carrou S. 14) |
| Formel des Mittelwertsatzes im Wortlaut | Woodhouse 2021 (nur Verweis), 2019, 2012; Waltham 2008 | Skudrzyk 1980 nicht vorhanden |

---

## 11 Ladeliste (nicht geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Skudrzyk_1980_MeanValueMethod_JASA67.pdf` | keine URL; Woodhouse 2021, Literatur: „J. Acoust. Soc. Am. 67(4), 11…“ (Seitenzahl abgeschnitten) | ? | JASA, vermutlich Bezahlschranke | Wortlaut und Gültigkeit von (P4) |
| `Waltham_Kotlicki_2008_HarpSoundboards_JASA124.pdf` | https://doi.org/10.1121/1.2956479 | ? | JASA, Bezahlschranke (Stufe 1) | Legende und Zustand (besaitet?) der Admittanzkarte; unabhängiger Pegel auf der Saitenleiste |
| `Bell_1997_HelmholtzHigherAirModesHarp_CASJ33.pdf` | Le Carrou 2007, Ref. 10: J. Catgut Acoust. Soc. 33, 2–8 (1997); keine URL geprüft | ? | Catgut Acoustical Society | Q der Luft bei allen offenen Öffnungen |
| `Christensen_Vistisen_1980_LowFreqGuitar_JASA68.pdf` | keine URL; Le Carrou 2007, Ref. 4: JASA 68, 758–766 (1980) | ? | JASA | physikalisch deutbares 2-FHG-Modell (Massen, Flächen), das Le Carrou nicht liefern konnte |
| `Gautier_Dauchez_2004_IntensityConcertHarp_ApplAcoust65.pdf` | Le Carrou 2007, Ref. 16: Appl. Acoust. 65, 1221–1231 | ? | Elsevier | Fernfeld und Phasen Boden/Öffnungen (f_B 175 Hz) |
| `Firth_1977_AcousticsOfHarp_Acustica37.pdf` | Le Carrou 2007, Ref. 7: Acustica 37, 148–154 | ? | Bibliothek | besaitet gegen unbesaitet (Stufe 1) |

Die beiden DOIs stammen aus Stufe 1. Die übrigen Zeilen nennen nur die Literaturangabe aus den gelesenen Quellen;
nichts ist abgerufen.

---

## 12 Was der Auftrag verlangt hat und nicht geliefert ist

- **Gleichungen für „Hals nachgiebig“ mit Daten:** nur die Form (S8) und ein zurückgerechneter Faktor; Hals- und
  Säulendaten fehlen.
- **Je Term die Wirkung auf Z2 in LU:** nicht gerechnet (kein Prüfstand hier); nur dB der Körperantwort.
- **Abstrahlung bis 2 kHz:** das Modell ist bis 2 kHz gerechnet, ein belegtes Gesetz über 220 Hz gibt es nicht.
  Verglichen ist an 14 Stellen bis 1 kHz.
- **Abstrahlungsdämpfung:** nur als Spanne (5.1), weil die Datei den Volumenfluss je Mode nicht trägt.
- **Kopfblock als Lager (Weg- und Drehfeder aus Maß und Werkstoff):** nicht herleitbar.

---

## Anhang: Skripte und Ausgaben

Die Skripte liegen in diesem Ordner. Wortlaut und Ausgabe hängt `anhang.py` unten an. Es führt jedes Skript neu aus,
damit Text und Ausgabe übereinstimmen.
- `r3_waltham_zuschnitt.py` und `r5_koerper_lesen.py` sind Hilfen zum Ansehen (Bildzuschnitt, Aufbau der Körperdatei).
- `r4_waltham_grau.py` misst die Grauwerte in Waltham Abb. 4.

### r1_saiten.py

```python
"""R1: Saitentabelle (Bell 1988 Tab. 2.1/2.2, Abb. 2.10), Zug, Impedanzen, Federn an der Oese (Gl. S1-S6)."""
import math

# Bell Tab. 2.1 / 2.2 (am Bild): Saite, f [Hz], Durchmesser [mm], Laenge [m]; Werkstoff
TAB = [("00", 2960, 0.52, 0.071, "N"), ("0", 2637, 0.52, 0.082, "N"), ("1", 2489, 0.56, 0.088, "N"),
       ("2", 2218, 0.56, 0.095, "N"), ("3", 1976, 0.61, 0.105, "N"), ("4", 1864, 0.62, 0.113, "N"),
       ("5", 1661, 0.65, 0.123, "N"), ("6", 1480, 0.69, 0.132, "N"), ("7", 1318, 0.70, 0.143, "N"),
       ("8", 1244, 0.69, 0.154, "G"), ("9", 1109, 0.70, 0.164, "G"), ("10", 988, 0.73, 0.177, "G"),
       ("11", 932, 0.76, 0.192, "G"), ("12", 830, 0.80, 0.208, "G"), ("13", 740, 0.85, 0.225, "G"),
       ("14", 659, 0.90, 0.242, "G"), ("15", 622, 0.95, 0.265, "G"), ("16", 554, 1.00, 0.285, "G"),
       ("17", 494, 1.06, 0.327, "G"), ("18", 466, 1.12, 0.354, "G"), ("19", 415, 1.18, 0.385, "G"),
       ("20", 370, 1.24, 0.421, "G"), ("21", 330, 1.30, 0.455, "G"), ("22", 311, 1.36, 0.498, "G"),
       ("23", 277, 1.44, 0.542, "G"), ("24", 247, 1.52, 0.591, "G"), ("25", 233, 1.60, 0.644, "G"),
       ("26", 207, 1.68, 0.699, "G"), ("27", 185, 1.76, 0.753, "G"), ("28", 165, 1.84, 0.813, "G"),
       ("29", 155, 1.92, 0.880, "G"), ("30", 139, 2.00, 0.945, "G"), ("31", 123, 2.10, 0.999, "G"),
       ("32", 116, 2.20, 1.053, "G"), ("33", 104, 2.35, 1.103, "G")]
# Tab. 2.2 Metall: Saite, f, Kern, Umspinnung, gesamt [mm], Laenge
MET = [("34", 92, 0.80, 0.24, 1.45, 1.150), ("35", 82, 0.80, 0.32, 1.55, 1.180), ("36", 77, 0.80, 0.35, 1.65, 1.222),
       ("37", 69, 0.85, 0.40, 1.75, 1.257), ("38", 61, 0.90, 0.43, 1.92, 1.299), ("39", 58, 0.95, 0.45, 2.05, 1.330),
       ("40", 52, 1.00, 0.50, 2.15, 1.359), ("41", 46, 1.10, 0.55, 2.30, 1.397), ("42", 41, 1.10, 0.60, 2.45, 1.421),
       ("43", 39, 1.25, 0.65, 2.60, 1.445), ("44", 35, 1.30, 0.68, 2.75, 1.475)]
# Bell Abb. 2.10, am Bild abgelesen (+-8 N); 43 fehlt im Bild (interpoliert)
ZUG_ABB = {"00": 53, "0": 55, "1": 55, "2": 55, "3": 69, "4": 59, "5": 68, "6": 78, "7": 63, "8": 89, "9": 89,
           "10": 90, "11": 103, "12": 89, "13": 105, "14": 113, "15": 117, "16": 105, "17": 160, "18": 148,
           "19": 151, "20": 165, "21": 163, "22": 177, "23": 184, "24": 200, "25": 237, "26": 239, "27": 227,
           "28": 251, "29": 260, "30": 289, "31": 280, "32": 322, "33": 279, "34": 433, "35": 446, "36": 463,
           "37": 447, "38": 410, "39": 438, "40": 397, "41": 424, "42": 384, "43": 380, "44": 377}
RHO = {"N": 1140.0, "G": 1320.0}     # Lynch-Aird & Woodhouse 2017 Tab. (1067-1160), 2018 Tab. 1 (1320-1325)
E = {"N": 6e9, "G": 6e9}              # 2018 S. 7: Darm "about 5.5-6.5 GPa"; Nylon 2017: 4-10 GPa (Annahme 6)
THETA = math.radians(30.0)            # Bell S. 157: "meet the soundboard at an angle of 30 deg"
S2, C2 = math.sin(THETA) ** 2, math.cos(THETA) ** 2

zeilen = []
for name, f, d, L, w in TAB:
    A = math.pi * (d * 1e-3) ** 2 / 4
    mu = RHO[w] * A
    zeilen.append((name, f, L, mu, E[w] * A, w))
for name, f, dk, du, dg, L in MET:
    Ak = math.pi * (dk * 1e-3) ** 2 / 4
    Ag = math.pi * (dg * 1e-3) ** 2 / 4
    # Annahme: Stahlkern 7850 kg/m3, Kupferumspinnung 8900 kg/m3 mit Fuellgrad pi/4 (Runddraht auf Kern)
    mu = 7850 * Ak + 8900 * (math.pi / 4) * (Ag - Ak)
    zeilen.append((name, f, L, mu, 200e9 * Ak, "M"))

print("Saite  f    L     mu[g/m] T_rech T_Abb  Z0[Ns/m] EA/L[kN/m] T/L[N/m] f_l1[Hz]")
summe_r = summe_a = sumZ0 = sumk = 0.0
daten = {}
for name, f, L, mu, EA, w in zeilen:
    T = 4 * mu * L * L * f * f
    Ta = ZUG_ABB[name]
    Z0 = math.sqrt(Ta * mu)
    kl = EA / L
    fl1 = math.sqrt(EA / mu) / (2 * L)
    summe_r += T; summe_a += Ta; sumZ0 += Z0; sumk += kl * S2
    daten[name] = dict(f=f, L=L, mu=mu, T=Ta, Z0=Z0, kl=kl, EA=EA, fl1=fl1)
    print(f"{name:>4} {f:5d} {L:5.3f} {mu*1e3:6.2f} {T:6.0f} {Ta:5d} {Z0:7.2f} {kl/1e3:8.1f} {Ta/L:7.0f} {fl1:7.0f}")
print(f"Summe Zug gerechnet {summe_r:.0f} N, aus Abb. 2.10 {summe_a:.0f} N (Bell S. 38: 9350 N)")
print(f"Summe Z0 = {sumZ0:.1f} Ns/m; Summe EA/L sin^2(30) = {sumk/1e6:.2f} MN/m; Normallast sum T sin30 = {summe_a*0.5:.0f} N")

# S5: Zusatzimpedanz an der Oese in Normalrichtung, Saite am Hals starr: Z_n = sin^2 Z_l + cos^2 Z_t
def z_t(s, f, eta=0.01):
    """Querimpedanz am Ende einer am anderen Ende festen Saite, Z = -j Z0 cot(kL), Verlust ueber k komplex."""
    import cmath
    k = 2 * math.pi * f / math.sqrt(s["T"] / s["mu"]) * complex(1, -eta / 2)
    return -1j * s["Z0"] / cmath.tan(k * s["L"])

def z_l(s, f, eta=0.02):
    import cmath
    c = math.sqrt(s["EA"] / s["mu"])
    k = 2 * math.pi * f / c * complex(1, -eta / 2)
    return -1j * math.sqrt(s["EA"] * s["mu"]) / cmath.tan(k * s["L"])

print("\nZusatzimpedanz an der eigenen Oese bei der Frequenz der Saite (Hals starr):")
print("Saite f    |Z_l| sin2 [Ns/m]  Z_t gedaempft (Z0) cos2 | Z_t ungedaempft eta=0.01 cos2 | Feder EA/L sin2 als |Z|=k/w")
for n in ("33", "26", "21", "12", "10"):
    s = daten[n]; f = s["f"]
    zl = z_l(s, f) * S2
    zt_d = s["Z0"] * C2
    zt_u = z_t(s, f, 0.01) * C2
    print(f"{n:>4} {f:5d} {abs(zl):10.1f}  {zt_d:8.2f}  {abs(zt_u):10.1f}  {s['kl']*S2/(2*math.pi*f):8.1f}")
```

Ausgabe (Exit-Code 0):

```text
Saite  f    L     mu[g/m] T_rech T_Abb  Z0[Ns/m] EA/L[kN/m] T/L[N/m] f_l1[Hz]
  00  2960 0.071   0.24     43    53    0.11     17.9     746   16156
   0  2637 0.082   0.24     45    55    0.12     15.5     671   13989
   1  2489 0.088   0.28     54    55    0.12     16.8     625   13035
   2  2218 0.095   0.28     50    55    0.12     15.6     579   12075
   3  1976 0.105   0.33     57    69    0.15     16.7     657   10925
   4  1864 0.113   0.34     61    59    0.14     16.0     522   10151
   5  1661 0.123   0.38     63    68    0.16     16.2     553    9326
   6  1480 0.132   0.43     65    78    0.18     17.0     591    8690
   7  1318 0.143   0.44     62    63    0.17     16.1     441    8022
   8  1244 0.154   0.49     72    89    0.21     14.6     578    6922
   9  1109 0.164   0.51     67    89    0.21     14.1     543    6500
  10   988 0.177   0.55     68    90    0.22     14.2     508    6023
  11   932 0.192   0.60     77   103    0.25     14.2     536    5552
  12   830 0.208   0.66     79    89    0.24     14.5     428    5125
  13   740 0.225   0.75     83   105    0.28     15.1     467    4738
  14   659 0.242   0.84     85   113    0.31     15.8     467    4405
  15   622 0.265   0.94    102   117    0.33     16.0     442    4023
  16   554 0.285   1.04    103   105    0.33     16.5     368    3740
  17   494 0.327   1.16    122   160    0.43     16.2     489    3260
  18   466 0.354   1.30    142   148    0.44     16.7     418    3011
  19   415 0.385   1.44    147   151    0.47     17.0     392    2769
  20   370 0.421   1.59    155   165    0.51     17.2     392    2532
  21   330 0.455   1.75    158   163    0.53     17.5     358    2343
  22   311 0.498   1.92    184   177    0.58     17.5     355    2141
  23   277 0.542   2.15    194   184    0.63     18.0     339    1967
  24   247 0.591   2.40    204   200    0.69     18.4     338    1804
  25   233 0.644   2.65    239   237    0.79     18.7     368    1655
  26   207 0.699   2.93    245   239    0.84     19.0     342    1525
  27   185 0.753   3.21    249   227    0.85     19.4     301    1416
  28   165 0.813   3.51    253   251    0.94     19.6     309    1311
  29   155 0.880   3.82    284   260    1.00     19.7     295    1211
  30   139 0.945   4.15    286   289    1.09     19.9     306    1128
  31   123 0.999   4.57    276   280    1.13     20.8     280    1067
  32   116 1.053   5.02    299   322    1.27     21.7     306    1012
  33   104 1.103   5.73    301   279    1.26     23.6     253     966
  34    92 1.150  11.97    536   433    2.28     87.4     377    1260
  35    82 1.180  13.62    510   446    2.46     85.2     378    1151
  36    77 1.222  15.38    545   463    2.67     82.3     379    1046
  37    69 1.257  17.30    521   447    2.78     90.3     356    1019
  38    61 1.299  20.79    522   410    2.92     97.9     316     952
  39    58 1.330  23.68    564   438    3.22    106.6     329     920
  40    52 1.359  26.05    520   397    3.22    115.6     292     903
  41    46 1.397  29.86    493   424    3.56    136.1     304     903
  42    41 1.421  33.77    459   384    3.60    133.8     270     835
  43    39 1.445  38.17    485   380    3.81    169.9     263     877
  44    35 1.475  42.66    455   377    4.01    180.0     256     846
Summe Zug gerechnet 10585 N, aus Abb. 2.10 9786 N (Bell S. 38: 9350 N)
Summe Z0 = 51.7 Ns/m; Summe EA/L sin^2(30) = 0.47 MN/m; Normallast sum T sin30 = 4893 N

Zusatzimpedanz an der eigenen Oese bei der Frequenz der Saite (Hals starr):
Saite f    |Z_l| sin2 [Ns/m]  Z_t gedaempft (Z0) cos2 | Z_t ungedaempft eta=0.01 cos2 | Feder EA/L sin2 als |Z|=k/w
  33   104        8.7      0.95         7.6       9.0
  26   207        3.4      0.63        14.7       3.7
  21   330        2.0      0.40         7.9       2.1
  12   830        0.6      0.18         1.0       0.7
  10   988        0.5      0.17         0.4       0.6
```

### r2_mittelwert.py

```python
"""R2: Mittelwert der Punkt-Admittanz an den Oesen (unendliche Platte, unendlicher Balken) gegen Bell 7.22,
Modell (S201) und Le Carrou; Gl. P1-P4. Werte des Bauplans aus make_harfe.py (nur gelesen)."""
import math

L = 1.31
def lin(y, a, b): return a + (b - a) * y / L
ZUG = [(0.0, 0.020, 0.032), (0.29, 0.025, 0.036), (0.55, 0.019, 0.039), (0.80, 0.013, 0.034), (1.05, 0.0072, 0.027),
       (1.28, 0.0056, 0.020)]
def zug(y):
    for (y0, h0, b0), (y1, h1, b1) in zip(ZUG, ZUG[1:]):
        if y <= y1:
            t = (y - y0) / (y1 - y0); return h0 + t * (h1 - h0), b0 + t * (b1 - b0)
    return ZUG[-1][1], ZUG[-1][2]
RHO_F, EX, EY, NU = 500.0, 7.3e9, 0.855e9, 0.3     # Fichte geeicht (Bauplan)
RHO_B, EB = 750.0, 13.7e9                           # Buche, Faser laengs der Leiste (Bauplan, Chabassier)

def dB(Y): return 20 * math.log10(Y) + 116.0      # Bell S. 133: 116 dB = 1 s/kg

def platte(y):
    h = lin(y, 0.010, 0.003) + 0.001
    m2 = RHO_F * h
    Dx, Dy = EX * h**3 / 12 / (1 - NU**2), EY * h**3 / 12 / (1 - NU**2)
    return h, m2, math.sqrt(Dx * Dy)

def balken(y, flansch=0.0):
    """Deckleiste aussen + Platte (Flanschbreite) + Zugleiste innen; EI um die eigene Nullinie, m'."""
    h, m2, _ = platte(y)
    bc, hc = lin(y, 0.033, 0.018), lin(y, 0.005, 0.003)
    hs, bs = zug(y)
    bf = max(flansch, bc, bs)
    teile = [(EB, bs, hs, -hs / 2), (EY, bf, h, h / 2), (EB, bc, hc, h + hc / 2)]
    EA = sum(E * b * t for E, b, t, z in teile)
    zn = sum(E * b * t * z for E, b, t, z in teile) / EA
    EI = sum(E * (b * t**3 / 12 + b * t * (z - zn) ** 2) for E, b, t, z in teile)
    mp = RHO_B * (bc * hc + bs * hs) + RHO_F * bf * h
    return EI, mp

def Y_platte(y):                                     # P1: 1/(8 sqrt(D m''))  (Cremer/Heckl, orthotrop: D = sqrt(Dx Dy))
    _, m2, D = platte(y); return 1 / (8 * math.sqrt(D * m2))

def Z_balken(y, f, flansch=0.0):                     # P2: Z = 2 (1+j) m' c_B, c_B = (EI/m')^(1/4) sqrt(w)
    EI, mp = balken(y, flansch); w = 2 * math.pi * f
    cB = (EI / mp) ** 0.25 * math.sqrt(w)
    return 2 * (1 + 1j) * mp * cB

ORTE = {"33": 0.393, "32": 0.420, "31": 0.450, "30": 0.476, "29": 0.506, "28": 0.534, "27": 0.559, "26": 0.581,
        "25": 0.609, "24": 0.634, "23": 0.666, "22": 0.692, "21": 0.723, "20": 0.745, "19": 0.773, "18": 0.798,
        "17": 0.823, "16": 0.850, "15": 0.877, "14": 0.905, "13": 0.929, "12": 0.951, "11": 0.976, "10": 0.999}
F = {"33": 104, "32": 116, "31": 123, "30": 139, "29": 155, "28": 165, "27": 185, "26": 207, "25": 233, "24": 247,
     "23": 277, "22": 311, "21": 330, "20": 370, "19": 415, "18": 466, "17": 494, "16": 554, "15": 622, "14": 659,
     "13": 740, "12": 830, "11": 932, "10": 988}
BELL = dict(zip([str(s) for s in range(33, 9, -1)],
                [80, 89, 74, 79, 81, 82, 85, 86, 84, 87, 88, 89, 95, 85, 89, 89, 91, 90, 93, 93, 99, 103, 98, 99]))
MODELL = {"33": 69.7, "32": 74.8, "31": 80.2, "30": 74.0, "29": 56.8, "28": 58.9, "27": 72.7, "26": 88.3, "25": 74.1,
          "24": 70.5, "23": 64.6, "22": 51.3, "21": 68.2, "20": 72.0, "19": 59.5, "18": 60.6, "17": 57.1, "16": 62.2,
          "15": 72.5, "14": 74.4, "13": 67.0, "12": 68.7, "11": 70.2, "10": 66.8}

print("Saite  f   y     Platte  Balken  Balken+Platte | Bell 7.22  Modell(S201) | Bell-B+P  Modell-B+P")
d_b, d_m = [], []
for s in ORTE:
    y, f = ORTE[s], F[s]
    Yp = Y_platte(y)
    Zb = Z_balken(y, f)
    Ybp = 1 / (Zb + 1 / Yp)
    d_b.append(BELL[s] - dB(abs(Ybp))); d_m.append(MODELL[s] - dB(abs(Ybp)))
    print(f"{s:>4} {f:4d} {y:5.3f} {dB(Yp):6.1f} {dB(abs(1/Zb)):6.1f} {dB(abs(Ybp)):8.1f}      | {BELL[s]:5.1f} {MODELL[s]:8.1f}     | "
          f"{d_b[-1]:+6.1f} {d_m[-1]:+8.1f}")
med = lambda v: sorted(v)[len(v) // 2]
print(f"Median: Bell - (Balken+Platte) {med(d_b):+.1f} dB, Modell - (Balken+Platte) {med(d_m):+.1f} dB")
for y in (0.393, 0.723, 0.951):
    EI, mp = balken(y); h, m2, D = platte(y)
    print(f"y={y}: EI={EI:.1f} N m2, m'={mp:.3f} kg/m, h={h*1e3:.1f} mm, m''={m2:.2f} kg/m2, D={D:.2f} N m")
# Le Carrou Punkt 34 (Camac), 104/117 Hz: 0,6/1,0e-3 s/kg
print(f"Le Carrou P34: {dB(0.6e-3):.1f} / {dB(1.0e-3):.1f} dB")
```

Ausgabe (Exit-Code 0):

```text
Saite  f   y     Platte  Balken  Balken+Platte | Bell 7.22  Modell(S201) | Bell-B+P  Modell-B+P
  33  104 0.393   69.4   64.0     60.9      |  80.0     69.7     |  +19.1     +8.8
  32  116 0.420   69.7   63.8     60.8      |  89.0     74.8     |  +28.2    +14.0
  31  123 0.450   70.0   63.8     61.0      |  74.0     80.2     |  +13.0    +19.2
  30  139 0.476   70.3   63.6     60.9      |  79.0     74.0     |  +18.1    +13.1
  29  155 0.506   70.6   63.4     60.8      |  81.0     56.8     |  +20.2     -4.0
  28  165 0.534   70.9   63.5     61.0      |  82.0     58.9     |  +21.0     -2.1
  27  185 0.559   71.2   63.3     60.9      |  85.0     72.7     |  +24.1    +11.8
  26  207 0.581   71.5   63.2     60.9      |  86.0     88.3     |  +25.1    +27.4
  25  233 0.609   71.8   63.2     61.0      |  84.0     74.1     |  +23.0    +13.1
  24  247 0.634   72.1   63.4     61.2      |  87.0     70.5     |  +25.8     +9.3
  23  277 0.666   72.5   63.5     61.4      |  88.0     64.6     |  +26.6     +3.2
  22  311 0.692   72.8   63.5     61.5      |  89.0     51.3     |  +27.5    -10.2
  21  330 0.723   73.2   63.9     61.9      |  95.0     68.2     |  +33.1     +6.3
  20  370 0.745   73.5   63.9     61.9      |  85.0     72.0     |  +23.1    +10.1
  19  415 0.773   73.9   64.0     62.0      |  89.0     59.5     |  +27.0     -2.5
  18  466 0.798   74.2   64.0     62.1      |  89.0     60.6     |  +26.9     -1.5
  17  494 0.823   74.6   64.3     62.5      |  91.0     57.1     |  +28.5     -5.4
  16  554 0.850   75.0   64.5     62.7      |  90.0     62.2     |  +27.3     -0.5
  15  622 0.877   75.3   64.7     62.9      |  93.0     72.5     |  +30.1     +9.6
  14  659 0.905   75.8   65.1     63.4      |  93.0     74.4     |  +29.6    +11.0
  13  740 0.929   76.1   65.3     63.6      |  99.0     67.0     |  +35.4     +3.4
  12  830 0.951   76.5   65.4     63.7      | 103.0     68.7     |  +39.3     +5.0
  11  932 0.976   76.9   65.6     64.0      |  98.0     70.2     |  +34.0     +6.2
  10  988 0.999   77.2   66.0     64.4      |  99.0     66.8     |  +34.6     +2.4
Median: Bell - (Balken+Platte) +27.0 dB, Modell - (Balken+Platte) +6.3 dB
y=0.393: EI=1293.0 N m2, m'=0.891 kg/m, h=8.9 mm, m''=4.45 kg/m2, D=161.28 N m
y=0.723: EI=454.6 N m2, m'=0.595 kg/m, h=7.1 mm, m''=3.57 kg/m2, D=83.16 N m
y=0.951: EI=164.6 N m2, m'=0.359 kg/m, h=5.9 mm, m''=2.96 kg/m2, D=47.43 N m
Le Carrou P34: 51.6 / 56.0 dB
```

### r4_waltham_grau.py

```python
"""R4: Waltham et al. 2008 Abb. 4 (Salvi Aurora, Punkt-Admittanz auf der Saitenleiste): Grauwert an den Saitenpunkten
-> Stufe der Legende -> s/kg, unter zwei Lesarten der Legende (10^-1 ... 10^-5 oder 10^-1 ... 10^-3)."""
import math
from PIL import Image
d = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\seiten"
im = Image.open(d + r"\waltham300-2.png").convert("L")
X0, Y0 = 1338, 33
# Stufen der Legende (Grauwert, von dunkel=hoch nach hell=tief), aus r3 gemessen
STUFEN = [1, 27, 56, 84, 114, 141, 171, 199, 226]
def stufe(g): return min(range(len(STUFEN)), key=lambda i: abs(STUFEN[i] - g))
# weisse Kreise im Bildfeld suchen
pts = []
for y in range(262, 590):
    for x in range(150, 826):
        if im.getpixel((X0 + x, Y0 + y)) > 236:
            pts.append((x, y))
cl = []
for x, y in pts:
    for c in cl:
        if abs(c[0] / c[2] - x) < 12 and abs(c[1] / c[2] - y) < 12:
            c[0] += x; c[1] += y; c[2] += 1; break
    else:
        cl.append([x, y, 1])
kreise = sorted([(c[0] / c[2], c[1] / c[2]) for c in cl if c[2] > 5])
f_von_x = lambda x: (x - 123.0) / 0.71        # 0 Hz bei x 123, 1000 Hz bei x 833 (Achsenteilung)
h_von_y = lambda y: (609.0 - y) / 0.2825      # 0 mm bei y 609, 1200 mm bei y 270
print("f[Hz]  h[mm]  Grau  Stufe(0=dunkel)  dB_Bell(10^-1..10^-5)  dB_Bell(10^-1..10^-3)")
erg = []
for x, y in kreise:
    ring = []
    for a in range(0, 360, 15):
        for r in (13, 16):
            px = int(x + r * math.cos(math.radians(a))); py = int(y + r * math.sin(math.radians(a)))
            ring.append(im.getpixel((X0 + px, Y0 + py)))
    ring.sort(); g = ring[len(ring) // 2]
    s = stufe(g)
    # Stufe s (0..8) ueberdeckt in log10 Y den Bereich -1 - (s+1)*D ... -1 - s*D, Mitte -1 - (s+0.5)*D
    y5 = -1 - (s + 0.5) * 4 / 9; y3 = -1 - (s + 0.5) * 2 / 9
    erg.append((f_von_x(x), s, 20 * y5 + 116, 20 * y3 + 116))
    print(f"{f_von_x(x):6.0f} {h_von_y(y):6.0f} {g:5d} {s:4d}  {20*y5+116:8.1f}  {20*y3+116:8.1f}")
print(len(kreise), "Kreise")
```

Ausgabe (Exit-Code 0):

```text
f[Hz]  h[mm]  Grau  Stufe(0=dunkel)  dB_Bell(10^-1..10^-5)  dB_Bell(10^-1..10^-3)
    46     99   116    4      56.0      76.0
    59    167   130    5      47.1      71.6
    75    245   114    4      56.0      76.0
   101    320   137    5      47.1      71.6
   129    390   137    5      47.1      71.6
   161    453   101    4      56.0      76.0
   179    475    99    3      64.9      80.4
   218    534    61    2      73.8      84.9
   260    582    61    2      73.8      84.9
   345    666    60    2      73.8      84.9
   392    698    61    2      73.8      84.9
   439    723    60    2      73.8      84.9
   492    754   100    4      56.0      76.0
   583    809    99    3      64.9      80.4
   700    859   100    4      56.0      76.0
   782    892    99    3      64.9      80.4
   879    916    60    2      73.8      84.9
   984    947    61    2      73.8      84.9
18 Kreise
```

### r6_koerper_rechnen.py

```python
"""R6: aus concert_harp.lvinstbody (nur gelesen): Punkt-Admittanz an den Oesen, p/F am Hoerort bis 2 kHz,
Verlustfaktor je Mode zerlegt, Abstrahlungsdaempfung je Mode (Gl. D1-D4, A1-A3)."""
import json, math, cmath
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
k = json.load(open(p, encoding="utf-8"))
print("units:", k["units"])
print("Hoerort:", k["listeningPoints"])
RHO, C = k["air"]["density"], k["air"]["speedOfSound"]
modes = k["modes"]
keys = k["bridgePoints"]

def key_of(hz):
    return int(round(69 + 12 * math.log2(hz / 440.0))) + 1

def Y(b, f):
    w = 2 * math.pi * f
    s = 0j
    for m in modes:
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        s += phi * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return 1j * w * s

def pF(b, f, nur_unter=None):
    w = 2 * math.pi * f
    s = 0j
    for m in modes:
        if nur_unter and m["f"] > nur_unter: continue
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        r = complex(m["radiation"][0][0], m["radiation"][0][1])
        s += r * (-w * w) * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return s

dB = lambda y: 20 * math.log10(abs(y)) + 116
# 1) Gegenprobe gegen S201 (Saiten 33, 26, 21, 12, 10)
for s, hz in (("33", 104), ("26", 207), ("21", 330), ("12", 830), ("10", 988)):
    b = keys.index(key_of(hz))
    print(f"Saite {s} {hz} Hz Taste {key_of(hz)}: |Y| = {dB(Y(b, hz)):.1f} dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)")

# 2) Verlustfaktor je Mode zerlegt: eta = (1 - airShare) * alpha/(2 pi f) + airShare / Q_luft
d = k["damping"]
print("\nMode   f[Hz]   eta    Q   airShare  eta_Platte(Gesetz) Q_luft_rueck  eta_rad(Halbraum) eta_rad(Vollraum)")
for m in modes[:8]:
    f = m["f"]; eta_law = (d["a"] * f * f + d["b"] * f + d["c"]) / (2 * math.pi * f)
    a = m["airShare"]
    ql = a / (m["eta"] - (1 - a) * eta_law) if m["eta"] > (1 - a) * eta_law and a > 1e-6 else float("nan")
    r = abs(complex(*m["radiation"][0]))
    w = 2 * math.pi * f
    # Monopol: p(1 m) = rho * qdd * V / (2 pi r) (Halbraum, Rayleigh) bzw. /(4 pi r) (Vollraum); V = Volumen je Modalkoordinate
    V_h = 2 * math.pi * 1.0 * r / RHO; V_v = 4 * math.pi * 1.0 * r / RHO
    eta_h = RHO * w * V_h ** 2 / (2 * math.pi * C)    # Halbraum: P = rho w^2 U^2 /(4 pi c)
    eta_v = RHO * w * V_v ** 2 / (4 * math.pi * C)    # Vollraum: P = rho w^2 U^2 /(8 pi c)
    print(f"{f:7.1f} {m['eta']:.4f} {1/m['eta']:5.1f} {a:7.3f}  {eta_law:.4f}          {ql:6.1f}      {eta_h:.2e}  {eta_v:.2e}")

# 3) p/F am Hoerort (1 m vor dem Boden), Anregung an den Oesen der Saiten 31 / 30 (Le Carrou 2010: zwischen 30 und 31)
b31 = keys.index(key_of(123)); b30 = keys.index(key_of(139))
LC2010 = [(134, 40), (148, 57), (166, 60), (195, 62), (290, 26), (384, 58), (460, 50), (582, 46), (644, 54), (688, 41),
          (756, 53), (850, 26), (892, 48), (1000, 42)]
print("\nf     p/F(1 m) dB re 2e-5 Pa/N, Oese 31 | Oese 30 | auf 2,35 m (-7,4 dB) Mittel | Le Carrou 2010 (2,35 m, Camac)")
for f, lc in LC2010:
    a1 = abs(pF(b31, f)); a0 = abs(pF(b30, f))
    L1 = 20 * math.log10(a1 / 2e-5); L0 = 20 * math.log10(a0 / 2e-5)
    m = 20 * math.log10((a1 + a0) / 2 / 2e-5) - 20 * math.log10(2.35)
    print(f"{f:5d}  {L1:6.1f}  {L0:6.1f}   {m:6.1f}    {lc}")
# Z4: 204 Hz gegen 240-250 Hz, gemittelt ueber 1/12-Oktave
def band(b, f0):
    fs = [f0 * 2 ** (i / 240) for i in range(-10, 11)]
    return 10 * math.log10(sum(abs(pF(b, f)) ** 2 for f in fs) / len(fs))
for b, n in ((b31, "31"), (b30, "30")):
    z4 = 20 * math.log10(abs(pF(b, 204))) - 10 * math.log10(sum(abs(pF(b, f)) ** 2 for f in range(240, 251)) / 11)
    print(f"Z4 Oese {n}: p/F(204 Hz) - mittel(240-250 Hz) = {z4:+.1f} dB")
# p/F-Verlauf in Terzbaendern 63 Hz - 2 kHz (Oese 31)
print("\nTerzband  p/F Oese 31 [dB re 2e-5 Pa/N, 1 m]  davon Moden <= 300 Hz")
for fm in (63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000):
    fs = [fm * 2 ** (i / 60) for i in range(-10, 11)]
    e = sum(abs(pF(b31, f)) ** 2 for f in fs) / len(fs)
    e3 = sum(abs(pF(b31, f, 300)) ** 2 for f in fs) / len(fs)
    print(f"{fm:6d}  {10*math.log10(e/4e-10):6.1f}   {10*math.log10(e3/4e-10):6.1f}")
```

Ausgabe (Exit-Code 0):

```text
units: f in Hz; eta loss factor; bridge: mode shape at the bridge point, 1/sqrt(kg) (modal mass 1/value^2); radiation: complex pressure per modal acceleration at the listening point, sqrt(kg)/m^2 (Rayleigh, baffled plate)
Hoerort: ['front']
Saite 33 104 Hz Taste 45: |Y| = 69.5 dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)
Saite 26 207 Hz Taste 57: |Y| = 82.5 dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)
Saite 21 330 Hz Taste 65: |Y| = 68.0 dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)
Saite 12 830 Hz Taste 81: |Y| = 69.2 dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)
Saite 10 988 Hz Taste 84: |Y| = 68.0 dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)

Mode   f[Hz]   eta    Q   airShare  eta_Platte(Gesetz) Q_luft_rueck  eta_rad(Halbraum) eta_rad(Vollraum)
  129.0 0.0768  13.0   0.305  0.0306             5.5      1.12e-03  2.24e-03
  191.8 0.0763  13.1   0.328  0.0249             5.5      1.33e-02  2.67e-02
  208.4 0.0769  13.0   0.334  0.0243             5.5      2.61e-02  5.21e-02
  274.8 0.0282  35.5   0.031  0.0232             5.5      1.69e-02  3.38e-02
  346.7 0.0239  41.9   0.001  0.0238             5.5      1.92e-03  3.84e-03
  430.8 0.0255  39.2   0.001  0.0254             5.5      6.19e-03  1.24e-02
  522.5 0.0279  35.9   0.000  0.0279             nan      8.47e-09  1.69e-08
  526.8 0.0280  35.7   0.000  0.0280             5.5      2.81e-04  5.61e-04

f     p/F(1 m) dB re 2e-5 Pa/N, Oese 31 | Oese 30 | auf 2,35 m (-7,4 dB) Mittel | Le Carrou 2010 (2,35 m, Camac)
  134    76.9    76.6     69.3    40
  148    73.1    72.9     65.6    57
  166    74.8    74.4     67.1    60
  195    82.2    82.3     74.8    62
  290    79.8    80.3     72.6    26
  384    67.8    66.7     59.8    58
  460    63.1    66.2     57.4    50
  582    67.5    68.4     60.6    46
  644    77.9    78.0     70.5    54
  688    64.8    67.0     58.5    41
  756    60.6    63.5     54.7    53
  850    65.9    66.9     59.0    26
  892    66.4    71.0     61.6    48
 1000    69.6    70.8     62.8    42
Z4 Oese 31: p/F(204 Hz) - mittel(240-250 Hz) = +8.1 dB
Z4 Oese 30: p/F(204 Hz) - mittel(240-250 Hz) = +8.7 dB

Terzband  p/F Oese 31 [dB re 2e-5 Pa/N, 1 m]  davon Moden <= 300 Hz
    63    42.8     43.5
    80    48.1     48.6
   100    56.6     56.7
   125    74.2     74.3
   160    74.9     75.1
   200    83.9     83.8
   250    82.9     82.9
   315    79.2     77.6
   400    71.6     69.9
   500    64.0     68.2
   630    71.8     67.4
   800    65.5     66.8
  1000    69.1     66.5
  1250    78.2     66.4
  1600    74.7     66.2
  2000    67.8     66.2
```

### r7_abstrahlung_je_schnelle.py

```python
"""R7: Abstrahlung je Schnelle p/v und wirksame Flaeche S_eff (Gl. A1, A2) - Modell (Koerperdatei) gegen Le Carrou
(Thèse Abb. 3.5 Mobilitaet an Punkt 34; Le Carrou 2007 Abb. 7 p/F bei Anregung an 34, 2,35 m); dazu Abstrahlungsguete
je Mode mit der gemessenen S_eff (Gl. D3)."""
import json, math
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
k = json.load(open(p, encoding="utf-8"))
modes, keys = k["modes"], k["bridgePoints"]
RHO, C = 1.2, 343.0
def key_of(hz): return int(round(69 + 12 * math.log2(hz / 440.0))) + 1
def Y(b, f):
    w = 2 * math.pi * f; s = 0j
    for m in modes:
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        s += phi * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return 1j * w * s
def pF(b, f):
    w = 2 * math.pi * f; s = 0j
    for m in modes:
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        s += complex(*m["radiation"][0]) * (-w * w) * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return s
# Punkt 34 liegt zwischen den Saiten A-flat / B-flat (103,8 / 116,5 Hz) = Bell-Saiten 33 / 32
b33, b32 = keys.index(key_of(104)), keys.index(key_of(116))
# Le Carrou: (f, |v34/F| [s/kg] Thèse Abb. 3.5 (Stufe 1), |p/F| [Pa/N] Le Carrou 2007 Abb. 7 bei 2,35 m (S197 Par. 14))
LC = [(152, 6.8e-3, 14e-3), (168, 4.6e-3, 14e-3), (204, 5.8e-3, 27e-3), (245, 0.8e-3, 4e-3), (295, 0.4e-3, 2e-3)]
print("f    LC: p/v[Pa s/m]@2,35m  S_eff[m2] | Modell: |Y|dB  p/F@1m  p/v@2,35m  S_eff[m2]")
for f, v, pp in LC:
    w = 2 * math.pi * f
    pv = pp / v; S = pv * 4 * math.pi * 2.35 / (RHO * w)
    ym = (abs(Y(b33, f)) + abs(Y(b32, f))) / 2; pm = (abs(pF(b33, f)) + abs(pF(b32, f))) / 2
    pvm = pm / 2.35 / ym   # 1/r von 1 m auf 2,35 m (Fernfeld angenommen)
    Sm = pvm * 4 * math.pi * 2.35 / (RHO * w)
    print(f"{f:4d}  {pv:6.2f}               {S:.3f}    | {20*math.log10(ym)+116:5.1f}  {pm:.3f}  {pvm:6.2f}    {Sm:.3f}")
# Abstrahlungsguete je Mode, Punktmasse an der Oese der Saite 32 (1/phi^2), R = rho w^2 S^2 /(4 pi c) (Vollraum)
print("\nMode f   M_Punkt(Saite 32)[kg]  Q_rad mit S_eff 0,05 / 0,09 m2   Q gesamt (Datei)")
for m in modes[:6]:
    f = m["f"]; w = 2 * math.pi * f; phi = m["bridge"][b32]
    M = 1 / phi ** 2
    q = [w * M / (RHO * w * w * S * S / (4 * math.pi * C)) for S in (0.05, 0.09)]
    print(f"{f:6.1f}  {M:8.2f}      {q[0]:8.0f} / {q[1]:6.0f}      {1/m['eta']:.1f}")
# Le Carrou T1: Punktmasse aus Abb. 3.5 (Q 22, 6,8e-3 s/kg bei 152 Hz) und aus Tab. II (200 g senkt 153,5 -> 148,5 Hz)
print("\nLe Carrou T1: M = Q/(w Y) =", round(22 / (2 * math.pi * 152 * 6.8e-3), 2), "kg;  Massenprobe: M = dm/((f/fm)^2-1) =",
      round(0.2 / ((153.5 / 148.5) ** 2 - 1), 2), "kg (200 g) bzw.", round(0.4 / ((153.5 / 148.5) ** 2 - 1), 2), "kg (2 x 200 g)")
```

Ausgabe (Exit-Code 0):

```text
f    LC: p/v[Pa s/m]@2,35m  S_eff[m2] | Modell: |Y|dB  p/F@1m  p/v@2,35m  S_eff[m2]
 152    2.06               0.053    |  66.7  0.093   11.53    0.297
 168    3.04               0.071    |  56.7  0.122   47.47    1.107
 204    4.66               0.089    |  70.1  0.311   26.07    0.501
 245    5.00               0.080    |  58.1  0.131   43.92    0.702
 295    5.00               0.066    |  60.0  0.138   37.33    0.496

Mode f   M_Punkt(Saite 32)[kg]  Q_rad mit S_eff 0,05 / 0,09 m2   Q gesamt (Datei)
 129.0      0.74          1304 /    402      13.0
 191.8      3.63          4324 /   1335      13.1
 208.4      1.65          1807 /    558      13.0
 274.8      2.28          1896 /    585      35.5
 346.7      1.26           829 /    256      41.9
 430.8      3.32          1760 /    543      39.2

Le Carrou T1: M = Q/(w Y) = 3.39 kg;  Massenprobe: M = dm/((f/fm)^2-1) = 2.92 kg (200 g) bzw. 5.84 kg (2 x 200 g)
```

### r8_saiten_auf_moden.py

```python
"""R8: (a) Bell Abb. 7.22 gegen eine Reihenfeder Y = w/k (Gl. M1); (b) Saiten als Zusatz an allen 46 Oesen, im
Modenraum der Koerperdatei: Frequenzverschiebung durch Laengsfeder, Querfeder (Vorspannung), Guete durch den
Wellenwiderstand gedaempfter Saiten (Gl. S5-S7); gegen Bells gemessene Verschiebungen (S197 Par. 8)."""
import json, math, sys
sys.path.insert(0, r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen")
import importlib.util
spec = importlib.util.spec_from_file_location("r1", r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\r1_saiten.py")

# (a) Reihenfeder
F = [104, 116, 123, 139, 155, 165, 185, 207, 233, 247, 277, 311, 330, 370, 415, 466, 494, 554, 622, 659, 740, 830, 932, 988]
B = [80, 89, 74, 79, 81, 82, 85, 86, 84, 87, 88, 89, 95, 85, 89, 89, 91, 90, 93, 93, 99, 103, 98, 99]
ks = sorted(2 * math.pi * f / 10 ** ((L - 116) / 20) for f, L in zip(F, B))
k = ks[len(ks) // 2]
res = [L - (20 * math.log10(2 * math.pi * f / k) + 116) for f, L in zip(F, B)]
sl = (sum((math.log10(f) - 2.5) * L for f, L in zip(F, B)) / sum((math.log10(f) - 2.5) ** 2 for f in F))
mF = sum(math.log10(f) for f in F) / len(F); mB = sum(B) / len(B)
steig = sum((math.log10(f) - mF) * (L - mB) for f, L in zip(F, B)) / sum((math.log10(f) - mF) ** 2 for f in F)
print(f"(a) Reihenfeder k = {k:.3g} N/m (Median); Rest Bell - Feder: Mittel {sum(res)/len(res):+.1f} dB, "
      f"Streuung {math.sqrt(sum(r*r for r in res)/len(res)):.1f} dB, groesster {max(res, key=abs):+.1f} dB")
print(f"    Steigung von Bell 7.22 (Regression): {steig:.1f} dB/Dekade = {steig*math.log10(2):.1f} dB/Oktave (Feder: 20 / 6,0)")

# (b) Saiten auf die Moden
import r1_saiten as r1  # druckt die Tabelle erneut; nur Daten verwenden
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
kd = json.load(open(p, encoding="utf-8"))
modes, keys = kd["modes"], kd["bridgePoints"]
def key_of(hz): return int(round(69 + 12 * math.log2(hz / 440.0))) + 1
S2, C2 = r1.S2, r1.C2
print("\n(b) Mode  f    | Laengsfeder Hals starr: f' (df%) | Querfeder T/L: df% | gedaempft R=Z0 cos2: Q vorher -> nachher")
for m in modes[:12]:
    wk = 2 * math.pi * m["f"]
    dk_l = dk_t = dr = 0.0
    for name, s in r1.daten.items():
        b = keys.index(key_of(s["f"]))
        phi2 = m["bridge"][b] ** 2
        dk_l += s["kl"] * S2 * phi2
        dk_t += s["T"] / s["L"] * C2 * phi2
        dr += s["Z0"] * C2 * phi2
    f_l = math.sqrt(wk * wk + dk_l) / (2 * math.pi)
    f_t = math.sqrt(wk * wk + dk_t) / (2 * math.pi)
    eta2 = m["eta"] + dr / wk
    print(f"     {m['f']:6.1f} | {f_l:6.1f} ({100*(f_l/m['f']-1):+5.1f} %)         | {100*(f_t/m['f']-1):+5.2f} %   | "
          f"{1/m['eta']:5.1f} -> {1/eta2:5.1f}")
```

Ausgabe (Exit-Code 0):

```text
(a) Reihenfeder k = 5.2e+04 N/m (Median); Rest Bell - Feder: Mittel +0.9 dB, Streuung 3.5 dB, groesster +10.1 dB
    Steigung von Bell 7.22 (Regression): 20.1 dB/Dekade = 6.0 dB/Oktave (Feder: 20 / 6,0)
Saite  f    L     mu[g/m] T_rech T_Abb  Z0[Ns/m] EA/L[kN/m] T/L[N/m] f_l1[Hz]
  00  2960 0.071   0.24     43    53    0.11     17.9     746   16156
   0  2637 0.082   0.24     45    55    0.12     15.5     671   13989
   1  2489 0.088   0.28     54    55    0.12     16.8     625   13035
   2  2218 0.095   0.28     50    55    0.12     15.6     579   12075
   3  1976 0.105   0.33     57    69    0.15     16.7     657   10925
   4  1864 0.113   0.34     61    59    0.14     16.0     522   10151
   5  1661 0.123   0.38     63    68    0.16     16.2     553    9326
   6  1480 0.132   0.43     65    78    0.18     17.0     591    8690
   7  1318 0.143   0.44     62    63    0.17     16.1     441    8022
   8  1244 0.154   0.49     72    89    0.21     14.6     578    6922
   9  1109 0.164   0.51     67    89    0.21     14.1     543    6500
  10   988 0.177   0.55     68    90    0.22     14.2     508    6023
  11   932 0.192   0.60     77   103    0.25     14.2     536    5552
  12   830 0.208   0.66     79    89    0.24     14.5     428    5125
  13   740 0.225   0.75     83   105    0.28     15.1     467    4738
  14   659 0.242   0.84     85   113    0.31     15.8     467    4405
  15   622 0.265   0.94    102   117    0.33     16.0     442    4023
  16   554 0.285   1.04    103   105    0.33     16.5     368    3740
  17   494 0.327   1.16    122   160    0.43     16.2     489    3260
  18   466 0.354   1.30    142   148    0.44     16.7     418    3011
  19   415 0.385   1.44    147   151    0.47     17.0     392    2769
  20   370 0.421   1.59    155   165    0.51     17.2     392    2532
  21   330 0.455   1.75    158   163    0.53     17.5     358    2343
  22   311 0.498   1.92    184   177    0.58     17.5     355    2141
  23   277 0.542   2.15    194   184    0.63     18.0     339    1967
  24   247 0.591   2.40    204   200    0.69     18.4     338    1804
  25   233 0.644   2.65    239   237    0.79     18.7     368    1655
  26   207 0.699   2.93    245   239    0.84     19.0     342    1525
  27   185 0.753   3.21    249   227    0.85     19.4     301    1416
  28   165 0.813   3.51    253   251    0.94     19.6     309    1311
  29   155 0.880   3.82    284   260    1.00     19.7     295    1211
  30   139 0.945   4.15    286   289    1.09     19.9     306    1128
  31   123 0.999   4.57    276   280    1.13     20.8     280    1067
  32   116 1.053   5.02    299   322    1.27     21.7     306    1012
  33   104 1.103   5.73    301   279    1.26     23.6     253     966
  34    92 1.150  11.97    536   433    2.28     87.4     377    1260
  35    82 1.180  13.62    510   446    2.46     85.2     378    1151
  36    77 1.222  15.38    545   463    2.67     82.3     379    1046
  37    69 1.257  17.30    521   447    2.78     90.3     356    1019
  38    61 1.299  20.79    522   410    2.92     97.9     316     952
  39    58 1.330  23.68    564   438    3.22    106.6     329     920
  40    52 1.359  26.05    520   397    3.22    115.6     292     903
  41    46 1.397  29.86    493   424    3.56    136.1     304     903
  42    41 1.421  33.77    459   384    3.60    133.8     270     835
  43    39 1.445  38.17    485   380    3.81    169.9     263     877
  44    35 1.475  42.66    455   377    4.01    180.0     256     846
Summe Zug gerechnet 10585 N, aus Abb. 2.10 9786 N (Bell S. 38: 9350 N)
Summe Z0 = 51.7 Ns/m; Summe EA/L sin^2(30) = 0.47 MN/m; Normallast sum T sin30 = 4893 N

Zusatzimpedanz an der eigenen Oese bei der Frequenz der Saite (Hals starr):
Saite f    |Z_l| sin2 [Ns/m]  Z_t gedaempft (Z0) cos2 | Z_t ungedaempft eta=0.01 cos2 | Feder EA/L sin2 als |Z|=k/w
  33   104        8.7      0.95         7.6       9.0
  26   207        3.4      0.63        14.7       3.7
  21   330        2.0      0.40         7.9       2.1
  12   830        0.6      0.18         1.0       0.7
  10   988        0.5      0.17         0.4       0.6

(b) Mode  f    | Laengsfeder Hals starr: f' (df%) | Querfeder T/L: df% | gedaempft R=Z0 cos2: Q vorher -> nachher
      129.0 |  158.7 (+23.0 %)         | +0.42 %   |  13.0 ->   8.6
      191.8 |  214.3 (+11.8 %)         | +0.26 %   |  13.1 ->   9.8
      208.4 |  218.6 ( +4.9 %)         | +0.22 %   |  13.0 ->  11.0
      274.8 |  291.7 ( +6.2 %)         | +0.32 %   |  35.5 ->  22.6
      346.7 |  362.2 ( +4.5 %)         | +0.31 %   |  41.9 ->  28.0
      430.8 |  445.4 ( +3.4 %)         | +0.26 %   |  39.2 ->  28.8
      522.5 |  522.5 ( +0.0 %)         | +0.00 %   |  35.9 ->  35.9
      526.8 |  538.4 ( +2.2 %)         | +0.18 %   |  35.7 ->  28.4
      612.2 |  612.2 ( +0.0 %)         | +0.00 %   |  32.6 ->  32.6
      640.7 |  648.6 ( +1.2 %)         | +0.10 %   |  31.6 ->  27.3
      715.0 |  715.0 ( +0.0 %)         | +0.00 %   |  29.3 ->  29.3
      767.4 |  772.7 ( +0.7 %)         | +0.06 %   |  27.8 ->  25.2
```

### r9_z1_je_term.py

```python
"""R9: (1) Wirkung jedes Saitenterms auf |Y| an den Oesen der Saiten 33 / 21 / 12 bei ihrer Frequenz (Z1), durch
Neusynthese im Modenraum (diagonale Naeherung); (2) Diskantende: Y_inf bei duennerem Boden, Balkenende (Gl. P1-P3, E1)."""
import json, math, cmath, io, contextlib
import importlib.util
spec = importlib.util.spec_from_file_location("r1", r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\r1_saiten.py")
r1 = importlib.util.module_from_spec(spec)
with contextlib.redirect_stdout(io.StringIO()):
    spec.loader.exec_module(r1)
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
kd = json.load(open(p, encoding="utf-8"))
modes, keys = kd["modes"], kd["bridgePoints"]
def key_of(hz): return int(round(69 + 12 * math.log2(hz / 440.0))) + 1
oesen = {n: keys.index(key_of(s["f"])) for n, s in r1.daten.items()}
# je Mode: Zusatzsteife (Laengsfeder, Hals starr) und Zusatzwiderstand (gedaempfte Saite, Z0 cos2) - diagonal
add = []
for m in modes:
    dk = dr = 0.0
    for n, s in r1.daten.items():
        phi2 = m["bridge"][oesen[n]] ** 2
        dk += s["kl"] * r1.S2 * phi2; dr += s["Z0"] * r1.C2 * phi2
    add.append((dk, dr))
def Y(b, f, feder=0.0, widerstand=0.0):
    w = 2 * math.pi * f; s = 0j
    for m, (dk, dr) in zip(modes, add):
        wk2 = (2 * math.pi * m["f"]) ** 2
        s += m["bridge"][b] ** 2 / (wk2 + feder * dk - w * w + 1j * m["eta"] * wk2 + 1j * w * widerstand * dr)
    return 1j * w * s
dB = lambda y: 20 * math.log10(abs(y)) + 116
print("Saite f   | ohne | +Laengsfeder (Hals starr) | +0,4 x Feder (Hals nachgiebig) | +gedaempft Z0cos2 | beides | Bell 7.22")
BELL = {"33": 80, "21": 95, "12": 103}
for n in ("33", "21", "12"):
    s = r1.daten[n]; b = oesen[n]; f = s["f"]
    # die eigene Saite: Zusatzimpedanz direkt am Punkt (Laengs- und Querterm, gedaempft)
    y0 = Y(b, f); y1 = Y(b, f, 1.0); y2 = Y(b, f, 0.4); y3 = Y(b, f, 0, 1.0); y4 = Y(b, f, 0.4, 1.0)
    print(f"{n:>4} {f:4d} | {dB(y0):5.1f} | {dB(y1):5.1f} ({dB(y1)-dB(y0):+.1f}) | {dB(y2):5.1f} ({dB(y2)-dB(y0):+.1f}) | "
          f"{dB(y3):5.1f} ({dB(y3)-dB(y0):+.1f}) | {dB(y4):5.1f} ({dB(y4)-dB(y0):+.1f}) | {BELL[n]}")
# Median ueber alle Saiten 33-10 (die Oesen von Bell 7.22)
F = [104, 116, 123, 139, 155, 165, 185, 207, 233, 247, 277, 311, 330, 370, 415, 466, 494, 554, 622, 659, 740, 830, 932, 988]
d1, d3, d4 = [], [], []
for f in F:
    b = keys.index(key_of(f)); y0 = dB(Y(b, f))
    d1.append(dB(Y(b, f, 1.0)) - y0); d3.append(dB(Y(b, f, 0, 1.0)) - y0); d4.append(dB(Y(b, f, 0.4, 1.0)) - y0)
md = lambda v: sorted(v)[len(v) // 2]
print(f"Saiten 33-10: Median der Aenderung: Feder {md(d1):+.1f} dB (Bereich {min(d1):+.1f}..{max(d1):+.1f}), "
      f"gedaempft {md(d3):+.1f} ({min(d3):+.1f}..{max(d3):+.1f}), beides {md(d4):+.1f} ({min(d4):+.1f}..{max(d4):+.1f})")

# (2) Diskantende
RHO_F, EX, EY, NU, RHO_B, EB = 500.0, 7.3e9, 0.855e9, 0.3, 750.0, 13.7e9
def Yp(h): m2 = RHO_F * h; D = math.sqrt(EX * EY) * h**3 / 12 / (1 - NU**2); return 1 / (8 * math.sqrt(D * m2))
print("\nDiskant: Y_inf der Platte (ohne Leisten), dB re Bell")
for h_holz in (0.003, 0.0025, 0.002):
    print(f"  Holz {h_holz*1e3:.1f} mm + 1 mm Furnier: {dB(Yp(h_holz+0.001)):.1f} dB")
# Balken an der Oese der Saite 5 (y 1,118 m, 1661 Hz): Abstand zum Ende der Zugleiste (1,28 m) und der Deckleiste (1,305 m)
EI, mp = 120.0, 0.30   # grob aus r2 bei y ~1,12 m extrapoliert (y 0,951: 164,6 N m2 / 0,359 kg/m)
for f, y in ((1661, 1.118), (988, 0.999), (2960, 1.246)):
    w = 2 * math.pi * f; kB = (mp * w * w / EI) ** 0.25; lam = 2 * math.pi / kB
    x = 1.28 - y
    # halbunendlicher Balken mit freiem Ende, Antrieb im Abstand x (Euler-Bernoulli, Cremer/Heckl): Faktor gegen unendlich
    # Y(x)/Y_inf = 1 + [ (1-j) e^{-2jkx} ... ] - hier nur die Huellkurve: 1 + 3 e^{-k x}   (x=0: 4, x>>: 1)
    fak = 1 + 3 * math.exp(-kB * x)
    print(f"  {f} Hz an y {y}: lambda_B {lam:.2f} m, Abstand zum Leistenende {x:.3f} m, Huelle |Y|/|Y_inf| bis {20*math.log10(fak):.1f} dB")
```

Ausgabe (Exit-Code 0):

```text
Saite f   | ohne | +Laengsfeder (Hals starr) | +0,4 x Feder (Hals nachgiebig) | +gedaempft Z0cos2 | beides | Bell 7.22
  33  104 |  69.5 |  63.3 (-6.2) |  66.3 (-3.2) |  69.3 (-0.2) |  66.2 (-3.3) | 80
  21  330 |  68.0 |  53.5 (-14.5) |  62.1 (-5.9) |  68.0 (+0.1) |  62.8 (-5.2) | 95
  12  830 |  69.2 |  69.9 (+0.7) |  69.5 (+0.3) |  69.2 (-0.0) |  69.5 (+0.2) | 103
Saiten 33-10: Median der Aenderung: Feder +1.4 dB (Bereich -14.5..+22.0), gedaempft -0.0 (-1.7..+2.7), beides +0.2 (-7.3..+9.9)

Diskant: Y_inf der Platte (ohne Leisten), dB re Bell
  Holz 3.0 mm + 1 mm Furnier: 83.3 dB
  Holz 2.5 mm + 1 mm Furnier: 85.6 dB
  Holz 2.0 mm + 1 mm Furnier: 88.3 dB
  1661 Hz an y 1.118: lambda_B 0.28 m, Abstand zum Leistenende 0.162 m, Huelle |Y|/|Y_inf| bis 0.6 dB
  988 Hz an y 0.999: lambda_B 0.36 m, Abstand zum Leistenende 0.281 m, Huelle |Y|/|Y_inf| bis 0.2 dB
  2960 Hz an y 1.246: lambda_B 0.21 m, Abstand zum Leistenende 0.034 m, Huelle |Y|/|Y_inf| bis 6.3 dB
```

### r10_lecarrou_2fhg.py

```python
"""R10: Le Carrou 2006 Thèse Gl. (4.19)-(4.21), q = (z5, w34): Eigenwerte (gegen Tab. 4.2), Guete, Punktmasse an w34,
Mobilitaet |w34/F| (gegen Abb. 4.4 A: Spitzen um 4-6e-3 m/s/N)."""
import numpy as np
M = np.array([[0.54, -5.02], [-5.02, 49.22]])
B = np.array([[5.37, -19.67], [-19.67, 205.31]])
K = np.array([[590.41, -5366.46], [-5366.46, 51310.00]]) * 1e3
# Zustandsform: s^2 M + s B + K = 0
A = np.block([[np.zeros((2, 2)), np.eye(2)], [-np.linalg.solve(M, K), -np.linalg.solve(M, B)]])
s = np.linalg.eigvals(A)
s = sorted([x for x in s if x.imag > 0], key=lambda x: x.imag)
for x in s:
    print(f"s = {x.real:.1f} + j {x.imag:.1f}  ->  f = {abs(x)/(2*np.pi):.1f} Hz, zeta = {-x.real/abs(x)*100:.2f} %, Q = {abs(x)/(-2*x.real):.1f}")
print("Tab. 4.2: s4 = -31,9 + j 976,6; s6 = -32,5 + j 1065,9")
Minv = np.linalg.inv(M)
print(f"det M = {np.linalg.det(M):.3f} kg^2; Masse, die eine Kraft an w34 sieht (quasistatisch): 1/(M^-1)_22 = {1/Minv[1,1]:.2f} kg")
print("|w34/F| [1e-3 m/s/N]:")
for f in (130, 140, 150, 155, 160, 165, 170, 175, 180, 190):
    w = 2 * np.pi * f
    H = np.linalg.inv(-w * w * M + 1j * w * B + K)
    print(f"  {f} Hz: {abs(1j*w*H[1,1])*1e3:.2f}   |V5/F|-artig |z5'/F| = {abs(1j*w*H[0,1])*1e2:.2f} e-2")
```

Ausgabe (Exit-Code 0):

```text
s = -31.9 + j 974.3  ->  f = 155.2 Hz, zeta = 3.27 %, Q = 15.3
s = -32.6 + j 1067.8  ->  f = 170.0 Hz, zeta = 3.05 %, Q = 16.4
Tab. 4.2: s4 = -31,9 + j 976,6; s6 = -32,5 + j 1065,9
det M = 1.378 kg^2; Masse, die eine Kraft an w34 sieht (quasistatisch): 1/(M^-1)_22 = 2.55 kg
|w34/F| [1e-3 m/s/N]:
  130 Hz: 0.99   |V5/F|-artig |z5'/F| = 0.87 e-2
  140 Hz: 1.60   |V5/F|-artig |z5'/F| = 1.37 e-2
  150 Hz: 3.37   |V5/F|-artig |z5'/F| = 2.76 e-2
  155 Hz: 4.40   |V5/F|-artig |z5'/F| = 3.40 e-2
  160 Hz: 2.55   |V5/F|-artig |z5'/F| = 1.67 e-2
  165 Hz: 0.65   |V5/F|-artig |z5'/F| = 0.20 e-2
  170 Hz: 1.45   |V5/F|-artig |z5'/F| = 1.99 e-2
  175 Hz: 1.89   |V5/F|-artig |z5'/F| = 2.12 e-2
  180 Hz: 1.57   |V5/F|-artig |z5'/F| = 1.65 e-2
  190 Hz: 1.10   |V5/F|-artig |z5'/F| = 1.09 e-2
```

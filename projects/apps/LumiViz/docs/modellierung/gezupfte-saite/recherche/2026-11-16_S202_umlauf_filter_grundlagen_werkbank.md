> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_11_umlauf_filter_grundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 21:55–22:19, claude-opus-5-5, 76 Runden, success, 5.91 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen. **Nicht gegengelesen:** keine Formel,
> Tabelle oder Zahl dieses Berichts ist bisher an der Quelle nachgelesen; die Marken der Belegkraft sind die des Laufs.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_11_umlauf_filter_grundlagen.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_11_umlauf_filter_grundlagen/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_umlauf_filter_grundlagen_werkbank/`: `beispiele.py`, `bend_verzug.py`, `kompakt.py`, `moden_diskret.py`, `moden_passiv_raster.py`, `umlauf_entwurf.py`, `umlauf_lauf.py`, `verlust_kaskade.py`, `verlust_kaskade_diag.py`, `umlauf_entwurf.txt`.

# Bericht — Berechnungsgrundlage für den Umlauf-Filter und die Stegkopplung der gezupften Saite (Stufe 2)

- **Auftrag:** `auftraege/2026-11-16_11_umlauf_filter_grundlagen.md` (LumiViz S201, Stufe 2 zu `…_01_umlauf_filter_saite_steg`)
- **Datum:** 2026-11-16 · **Modell:** claude-opus-5-5 · ein Lauf, ohne Haltepunkt
- **Voraussetzung:** Bericht der Stufe 1 liegt vor (`ergebnisse/2026-11-16_01_umlauf_filter_saite_steg/bericht.md`) und ist
  gelesen.
- **Reichweite:** 0 Websuchen, 2 Abrufe: `dafx.de/paper-archive/2006/papers/p_071.pdf` (Rauhala & Välimäki DAFx-06) —
  **ECONNREFUSED**, wie in beiden Läufen der Stufe 1; JOS PASP „Thiran Allpass Interpolators“ (Inhalt, s. S2). Keine neue
  Quelle geladen. Lokal: `pdf_suche.py` auf 3 PDFs (Bank & Karjalainen 2010, Bank & Välimäki 2003 — beide aus
  `ergebnisse/2026-11-16_01_…/abrufe/` —, Välimäki u. a. 2004), **2 Seiten am Bild gelesen** (Bank & Karjalainen S. DAFX-4,
  Välimäki u. a. 2004 S. 937). LumiViz-Repo gelesen: Prüfstand `2026-11-15_S199_pruefstand_zupf.py`, die Proben S200
  (`umlauf_filter`, `zeitbereich_kandidat`, `zeitbereich_bend`, `zeitbereich_rechenzeit`) und S201
  (`umlauf_filter_entwurf`, `umlauf_filter_varianten`, `dispersion_formeln_gegenprobe`) samt `.txt`, Konzept §6 GZ5–GZ9,
  die fünf Körperdateien (nur Modenzahl). **Nicht erreicht:** wie Stufe 1 (DAFx-Archiv, Laakso 1996, Pakarinen 2005,
  Välimäki-Dissertation zur Transienten-Beseitigung).
- **Hauptsache dieses Laufs ist Rechnung, nicht Recherche:** die Gleichungen unten sind überwiegend `[hergeleitet]` oder
  stammen aus dem Prüfstand bzw. aus Stufe 1; jede ist an einem Zahlenbeispiel nachgerechnet.

**Unklarer Punkt im Auftrag (erster Satz, wie verlangt):** „Darm- und Stahlsaiten der Harfe an C7 / G7“ — im Prüfstand
sind C7 (Taste 96) und G7 (Taste 103) **Nylonsaiten** (Bell 1988 Tab. 2.1, Saiten 00–8; `harfe_taste` → Werkstoff „N“).
Gerechnet sind deshalb C7 und G7 (Nylon) **und** zusätzlich zwei Darmsaiten (Taste 46, die steifste laut S201, und Taste 60)
und eine Metallsaite (Taste 27, Bass).

Belegmarken: `[wörtlich, Seitenbild geprüft]` · `[wörtlich, nur Textebene]` · `[Werkzeug]` · `[nur Zusammenfassung]` ·
`[hergeleitet]` · `[eigene Rechnung]` · `[eigene Einschätzung]` · `[Prüfstand]` = so im LumiViz-Skript, nicht an einer
Quelle neu geprüft · `[Stufe 1]` = Wortlaut und Marke aus dem Bericht der Stufe 1 übernommen.

**Skripte** (alle unter `ergebnisse/2026-11-16_11_umlauf_filter_grundlagen/`, Aufruf `python -B <skript>`; sie importieren
den Prüfstand und die S201-Gegenprobe aus dem LumiViz-Repo, ohne dort Bytecode zu schreiben):

| Skript | wozu | Ausgabe |
|---|---|---|
| `umlauf_entwurf.py` | Entwurfsverfahren (Gl. V2–V6, D1–D5, S1–S3), Bewertung | Bibliothek |
| `umlauf_lauf.py` | Punkt 2: 18 Tasten × 7 Dispersionen × 5 Verluste × 5 Bruchteil-Glieder | `umlauf_entwurf.txt` (volle Tabellen) |
| `kompakt.py` | Auszug aus `umlauf_entwurf.txt` | Konsole (unten übernommen) |
| `verlust_kaskade.py`, `…_diag.py` | Verlust passiv durch den Bau (Gl. V7) | Konsole (unten) |
| `bend_verzug.py` | Punkt 3: Bend ±1 Halbton in 50 ms im Zeitschritt | Konsole (unten) |
| `moden_diskret.py`, `moden_passiv_raster.py` | Punkt 4 | Konsole (unten) |
| `beispiele.py` | je Gleichung ein Zahlenbeispiel | Konsole (unten) |

Die Skripte sind wegen ihrer Länge (90–260 Zeilen) **nicht** als Codeblock eingefügt (Abweichung vom Auftrag, „Skripte als
Codeblock“); eingefügt sind die Kernzeilen der Entwurfsformeln (Anhang A) und alle Ausgaben, auf die sich der Bericht
stützt.

---

## 0. Bezug und Messgrößen

**Gl. (Z1) Ziel-Umlauf** `[Prüfstand]`, `saite_wellen` (Z. 274–280) und `saiten_eta` (Z. 222–231):

 k² = (−T + √(T² + 4 EI m ω²)) / (2 EI); k ← k (1 − i η/2);
 η = [T (η_F + η_A/ω) + EI η_B k²] / (T + EI k²) (Valette; bei monofilen Saiten Luft nach Stokes + E″/E-Term);
 **H_ziel(ω) = exp(−2 i k L)**, also |H_ziel| = exp(2 Im k · L), φ_ziel = 2 Re k · L.

Der Prüfstand gibt dafür Woodhouse u. a. 2012 Gl. 2–4 bzw. Woodhouse & Lynch-Aird 2019 an; nicht neu gegengeprüft
(Auftrag: „Valette/Woodhouse nicht neu herleiten“).

**Gl. (Z2) Teilton und Abklingen am starren Steg** `[hergeleitet]`: Teilton n bei φ(f_n) = 2πn; Abklingrate
σ_n = −ln|H(f_n)| / τ_g(f_n), τ_g = dφ/dω. Für Ziel und Modell gleich gerechnet.
*Hinweis:* S201 nahm σ_n = −f_n ln|E(f_n)| (statt Division durch τ_g). Die Prozentfehler hier sind deshalb nicht Ziffer für
Ziffer mit S201 vergleichbar, wohl aber in der Größe.
*Beispiel* (`beispiele.py`, G-Saite leer): f₁ = 195,793 Hz (c/2L = 195,781 Hz), σ₁ = 0,9376 1/s, σ₈ = 2,327, σ₁₆ = 8,599 1/s.

**Gl. (Z3) Abklingen als Formel** `[hergeleitet]` aus Z1 für EI k² ≪ T, k ≈ ω/c:
 **σ(f) ≈ η_A/2 + π η_F f + η_B EI m ω³ / (2 T²)**.
*Beispiel:* G-Saite σ(f₁) = 0,938 (exakt 0,938), σ(f₈) = 2,333 (2,327), σ(f₁₆) = 8,809 (8,599) 1/s.
Folge, die alles Weitere bestimmt: −ln|H_ziel| je Umlauf = σ/f₀ (Gl. Z5, Beispiel 4,788·10⁻³ gegen σ₁/f₁ = 4,789·10⁻³)
hat einen **konstanten**, einen **linearen** und einen **kubischen** Anteil in f; am Gitarren-G ist −ln G an Teilton 32 das
50-Fache von Teilton 1 (`verlust_kaskade_diag.py`).

**Gl. (Z4)** B = π² EI / (T L²) `[hergeleitet]`; f_n ≈ n f₀ √(1 + B n²). *Beispiel:* G-Saite B = 1,242·10⁻⁴; Teilton 12
nach Näherung 2370,29 Hz, nach Z1 2370,29 Hz.

**Hörbare Teiltöne.** Bewertet wird bis 20 kHz; weil Teiltöne mit σ > 100 1/s (T60 < 69 ms) die Größtwerte beherrschen,
ohne lange zu klingen, steht daneben immer die Spalte **„hörbar“ = Teiltöne mit σ ≤ 100 1/s** `[eigene Einschätzung]` (die
Grenze ist eine Setzung; die vollen Spalten stehen in `umlauf_entwurf.txt`). Am Cembalo sind **alle** Teiltöne bis 20 kHz
„hörbar“ (σ(20 kHz) unter 100 1/s), an der Laute (Chor 1) nur die bis 1,7 kHz.

---

## 1. Entwurfsverfahren je Taste (ohne Optimierer zur Laufzeit)

Umlauf des Modells: **H_m(z) = z^(−N) · H_fd(z) · A_disp(z) · H_loss(z)** (ganzzahliger Verzug, Bruchteil, Dispersion,
Verlust). Reihenfolge des Entwurfs: Verlust → Dispersion (an der Restphase φ_ziel − φ_loss) → Stimmung (N, Δ).
Välimäki u. a. 2004, S. 937: „The total delay line length L in the feedback loop is 1+R+L1 plus the phase delay caused by
the fractional delay filter F(z) and the allpass filter A_d(z).“ `[wörtlich, Seitenbild geprüft]` (in diesem Lauf am Bild
gelesen).

### 1a Verlustfilter

**Gl. (V1)** Välimäki u. a. 2004, S. 937, Gl. (1), (2): H(z) = b (r + z^−R)/(1 + a z⁻¹), b = g (1 + a); „It is sufficient to
require that the sum g + |r| remains below one, or |r| < 1−g.“ `[wörtlich, Seitenbild geprüft]`. Mit r = 0, R = 0 der
Ein-Pol **H(z) = b/(1 + a z⁻¹)**, a < 0 Tiefpass, |H(0)| = g.

**Gl. (V2) Ein-Pol durch zwei Sollwerte, geschlossen** `[hergeleitet]`. Gegeben G₁ = |H_ziel(f₁)|, G₂ = |H_ziel(f₂)|,
c_i = cos(2π f_i/fs), r = G₁²/G₂²:
 (1 − r) a² + 2 (c₂ − r c₁) a + (1 − r) = 0 → a = −p ± √(p² − 1), p = (c₂ − r c₁)/(1 − r), die Wurzel mit |a| < 1
 (Produkt der Wurzeln = 1); b = G₁ √(1 + 2 a c₁ + a²).
*Beispiel* G-Saite, f₂ = 8 f₁: b = 0,786102, a = −0,210212; |H(f₁)| = 0,995223 = Ziel; |H(8f₁)| = 0,988381 = Ziel;
|H(0)| = b/(1+a) = 0,995333.
Gültigkeit: jede Taste; f₂ ≤ 0,45 fs. Trifft nur zwei Punkte.

**Gl. (V3) Passivität des Ein-Pols** `[hergeleitet]`: max_ω |H| = b/(1 − |a|) (bei ω = 0 für a < 0, bei fs/2 für a > 0).
Ist es > 1, wird |H(0)| = 1 gesetzt und nur f₁ getroffen: (1 − G₁²) a² + 2 (1 − G₁² c₁) a + (1 − G₁²) = 0, b = 1 + a.
An den 18 gerechneten Tasten griff die Klemme nie (größtes |H| 0,99980 an Harfe G7). S201 meldet an der Harfe ein
|H| = 1,000305 (Taste 101) — genau dort greift V3.

**Gl. (V4) Sollwerte und Gewicht nach Bank & Välimäki 2003** `[Stufe 1: wörtlich, Seitenbild geprüft]`, S. 18–19:
g_k = e^(−1/(f₀ τ_k)); w_k = 1/(g_k² (ln g_k)⁴) ≈ 1/(g_k − 1)⁴ (Gl. 2). Der Satz „The magnitude response of the designed
filters never exceeded unity“ steht S. 19 (in diesem Lauf mit `pdf_suche.py` wiedergefunden, PDF-Seite 2, gedruckt S. 19;
`[wörtlich, nur Textebene]`).
*Beispiel* G-Saite: τ₁…₃ = 1,067 / 0,967 / 0,871 s; Gewicht relativ zu k = 1: 1 / 0,68 / 0,45.

**Gl. (V5) IIR N-ter Ordnung, Gleichungsfehler + Steiglitz-McBride** `[hergeleitet; Verfahren wie `invfreqz` bei Bank &
Välimäki, das Papier nennt nur `invfreqz`]`: Ziel H_d an den Teiltönen mit Mindestphase (Cepstrum aus ln|H_ziel| auf
0…fs/2); minimiere Σ_k w_k^(1/2)/|A_alt(e^{iω_k})| · |B(e^{iω_k}) − H_d,k A(e^{iω_k})|² — je Schritt **eine lineare Lösung**,
6 Schritte; Pole außerhalb gespiegelt. **Offline je Taste**, nicht geschlossen.

**Gl. (V6) Passivitätsprüfung** `[hergeleitet]`: max |H| auf 16 385 Punkten 0…fs/2; ist es > 1, wird b durch das Maximum
geteilt. **Befund:** an 5 der 18 Tasten liegt der Bank-Entwurf über 1 (Harfe 46: iir4 1,011, iir8 1,003; Harfe 96: iir2
1,00005; Harfe 103: iir2 1,004, iir4 1,162; Cembalo 29: iir4 1,0007; Laute 1: iir8 1,002), und das Teilen zerstört dann das
Abklingen (Harfe 103 iir4: +8373 %, Harfe 46 iir4: +202 %). V5 ist **nicht** passiv sicher; nötig ist eine Regel „niedrigste
Fehler unter den passiven Ordnungen 2/4/8“ oder ein Entwurf mit Nebenbedingung |H| ≤ 1 (nicht gerechnet).

**Gl. (V7) Passiv durch den Bau** `[hergeleitet]`: H = g · Π (1 + a_m)/(1 + a_m z⁻¹) · Π (1 + c_m z⁻¹)/(1 + c_m), 0 < g ≤ 1,
−1 < a_m ≤ 0, 0 ≤ c_m < 1 ⇒ |H| ≤ g ≤ 1. Offline angepasst am relativen Abklingfehler. **Ergebnis negativ:** 1–12 bleiben
bei 10–80 % (Tabelle 2c).

**Gl. (V8) Warum ein Filter niedriger Ordnung den Verlust nicht trägt** `[hergeleitet]`: ln|H(e^{iω})| eines reellen
rationalen Filters ist gerade und glatt in ω, also ≈ c₀ + c₂ ω² bei kleinem ω. Das Ziel (Z3) hat den Anteil π η_F f/f₀,
**linear in |ω|**. Ein Filter mit Größtwert bei ω = 0 (Ein-Pol, V7) kann diesen Anteil nur durch Ausgleich über das Band
nachbilden — daher die 25–45 % an den Teiltönen 1–12 bei V2 und V7. Ein IIR ab Ordnung 4 legt seinen Größtwert unter f₁ und
erreicht dann 0,1–1 % (Tabelle 2b). Das erklärt auch die 13–21 % am Grundton der FIR in S201 (symmetrisches FIR = gerade
in ω).

### 1b Dispersion

**Gl. (D1) Allpass 1. Ordnung, K gleiche Stufen** `[Stufe 1: Werkzeug, JOS PASP; Standard]`:
A(z) = ((a + z⁻¹)/(1 + a z⁻¹))^K; Phasenverzug je Stufe bei ω → 0: (1 − a)/(1 + a) Takte `[hergeleitet]`.
*Beispiel* G-Saite K = 16, a = −0,4828: 2,867 Takte je Stufe, 45,9 Takte zusammen.

**Gl. (D2) Rauhala & Välimäki 2006, Wert a₁ geschlossen aus B, Taste, M** `[Stufe 1: Werkzeug]` (Faust `effect.lib`):
kd = exp(k₁ ln²B + k₂ ln B + k₃), Cd = exp((m₁ ln M + m₂) ln B + m₃ ln M + m₄), D = exp(Cd − Ikey·kd),
a₁ = (1 − D)/(1 + D); k₁…m₄ = −0,00179, −0,0233, −2,93, 0,0126, 0,0606, −0,00825, 1,97. Papier nicht erreicht; die
Konstanten sind an Klaviersaiten bei 44,1 kHz angepasst, Erratum zu Gl. (7) unbekannt.
*Beispiel* G-Saite M = 8: a₁ = −0,5498 (S201: −0,549).

**Gl. (D3) Abel, Välimäki & Smith 2010 (`adf.m`)** `[Stufe 1: Werkzeug]`: Zahl der Biquads
nap = ⌊φ₀/2π⌋, φ₀ = 2π (df/fs) pd₀ − μ₀ 2π df/fs, pd₀ = (fs/f₀)/√(1 + B (df/f₀)²), μ₀ = pd₀/(1 + B (df/f₀)²) — **die
Stufenzahl als geschlossene Funktion von B, f₀, df, fs**; Polwinkel und -radien geschlossen (Stufe 1, Abschnitt 1b).
*Beispiel* G-Saite df = 5 kHz: nap = 1; Cembalo 29: 32; Harfe 27: 43.

**Gl. (D4) K gleiche Allpässe, ein Wert angepasst** `[aus S201 übernommen]`: a minimiert Σ_{n ≤ 40, f_n ≤ Band} (Cent_n)²,
Verzug so, dass der Grundton stimmt; Raster 195 Werte + Feinsuche. **Ein Parameter, offline je Taste** (Tafel: ein Wert je
Taste und K).

**Gl. (D5) Grenze der Stufenzahl** `[hergeleitet]`: K · (1 − a)/(1 + a) + τ_loss + Δ ≤ D_tot − 2, D_tot = fs/f₀ (Umlauf
in Takten). An hohen Tasten begrenzt das K: Harfe G7 (D = 15,3) kommt auf K = 8, Harfe C7 (D = 22,9) auf K = 16.

### 1c Bruchteil-Verzug und Stimmung

**Gl. (S1) Stimmung** `[hergeleitet]`: D_tot = [φ_ziel(ω₁) − φ_loss(ω₁) − φ_disp(ω₁)]/ω₁; N = ⌊D_tot − (N_fd − 0,5)⌋
(Thiran) bzw. ⌊D_tot − (N_fd − 1)/2⌋ (Lagrange); Δ = D_tot − N. Danach wahlweise **Nachstimmen**: 2–3 Newton-Schritte auf
Δ, so dass der Phasenverzug des Bruchteil-Filters bei ω₁ (nicht bei 0) stimmt — geschlossen genug für eine Tafel.
*Beispiel* G-Saite: Umlauf 245,417 Takte = N 198 + Δ 1,0370 + 46,4 (Dispersion und Verlust); Thiran-Wert η = −0,0182.
*Ohne* Nachstimmen (Δ aus der Gleichstrom-Formel) bleibt am Grundton ein Fehler: Thiran 1 −0,17 Cent (Harfe C7), −0,57 Cent
(Harfe G7), ≤ 0,014 Cent sonst; Lagrange 1 −0,28 Cent (G7); Thiran 3 / Lagrange 3–5 ≤ 0,01 Cent.

**Gl. (S2) Thiran N-ter Ordnung** `[Werkzeug]` (JOS PASP, in diesem Lauf abgerufen): a_k = (−1)^k C(N,k) Π_{n=0}^{N}
(Δ − N + n)/(Δ − N + k + n); Stabilität dort nur „For sufficiently large Δ, stability is guaranteed“ und „Rule of thumb:
Δ ≈ order“ — **eine genaue Grenze nennt die Seite nicht**. 1. Ordnung: η = (1 − Δ)/(1 + Δ) `[Stufe 1: Werkzeug]`.
|H| = 1 für alle ω (Allpass) — **ändert das Abklingen nicht**.

**Gl. (S3) Lagrange N-ter Ordnung** `[hergeleitet, Standard]`: h_k = Π_{m≠k} (Δ − m)/(k − m). Betrag < 1 bei hohen
Frequenzen, am stärksten bei Bruchteil 0,5. *Beispiel:* |H(5 kHz)| / |H(10 kHz)| = 0,947 / 0,793 (N = 1, = cos(ω/2)),
0,996 / 0,940 (N = 3), 0,9996 / 0,981 (N = 5).

**Gl. (S4) Bedingung, damit der Verlust den Lagrange-Betrag ausgleichen kann** `[hergeleitet]`: |H_loss| = G/|H_fd| ≤ 1
verlangt **−ln|H_fd(ω)| ≤ −ln G(ω) = σ(f)/f₀ für alle ω**. *Beispiel* Cembalo 84 bei 5 kHz: −ln G = 1,31·10⁻³ je Umlauf,
−ln|H_L1(Δ 0,5)| = 5,45·10⁻² — 40-mal zu viel. Die Bedingung bricht an **hohen, verlustarmen** Saiten (wenig Zeit je Umlauf,
also wenig Verlust je Umlauf); Tabelle 2d.

### 1d Was geschlossen geht, was eine Anpassung braucht

| Schritt | Verfahren | geschlossen? | Anpassung vorab in einer Tafel je Taste? |
|---|---|---|---|
| Verlust | Ein-Pol V2/V3 | ja | — |
| Verlust | IIR N = 2/4/8 (V5) | nein: 6 lineare Lösungen | ja: N + 1 + N Werte je Taste; dazu Passivitätsprüfung V6 und Ausweichregel |
| Verlust | Kaskade V7 | nein: nichtlineare kleinste Quadrate | ja, aber Fehler zu groß (2c) |
| Dispersion | Rauhala (D2) | ja (Konstanten aus fremder Anpassung) | — |
| Dispersion | adf (D3) | ja, auch die Stufenzahl | — (nap Biquads je Taste) |
| Dispersion | K gleiche Allpässe (D4) | nein: 1-D-Suche | ja: **ein** Wert je Taste und K |
| Stimmung | S1 mit Gleichstrom-Formel | ja | — |
| Stimmung | S1 mit Nachstimmen | nein: Newton, 2–3 Schritte | ja: Δ je Taste (bei festem Ton) |
| Bruchteil | Thiran/Lagrange (S2/S3) | ja | — |

**Gültigkeit (je Verfahren, aus den 18 Tasten)** `[eigene Rechnung]`:

| Verfahren | gilt (hörbare Teiltöne ≤ 10 Cent bzw. ≤ 5 %) | kippt |
|---|---|---|
| K = 16 / 32 (D4, Band 5 kHz) | B ≤ 2,5·10⁻⁴ **und** f₀ ≥ 200 Hz: hörbar 3–22 Cent (K32), 1–12 ≤ 1 Cent | Bass mit großem B (f/f₀)²: Harfe 27 (97 Cent), Laute Chor 1 (35), Cembalo 29/36 (813/360 bis 20 kHz; ≤ 5 kHz 124/16), Oud 1 (72) |
| adf (D3, df 5 kHz) | B (df/f₀)² ≲ 0,25: Gitarre E2 (0,07) 3,0 Cent ≤ 5 kHz, Cembalo 36 (0,09) 3,6, Oud 1 (0,22) 35,8 | B (df/f₀)² ≥ 0,5: Harfe 46 (0,56) 128 Cent, Cembalo 29 (0,56) 152, Harfe 27 (4,8) und Laute 1 (4,7) über 1000 Cent; nap = 0 bei f₀ ≳ 200 Hz → keine Dispersion |
| Rauhala M8 (D2) | an keiner Taste besser als D4 mit gleichem K (48 kHz, Saiten keine Klaviersaiten) | Harfe C7/G7: 109/171 Cent |
| Verlust iir4 (V5) | wo passiv: 1–12 ≤ 1 % an 13 von 18 Tasten | nicht passiv an Harfe 46, 103, Cembalo 29; Bass-Metall/Laute 1: 1–12 5–27 % |
| Verlust Ein-Pol (V2) | 1–12: Gitarre E2 3,5 %, Oud 1 3,8 % | sonst 9–88 % an 1–12 |
| Thiran 1 | überall; kein Einfluss aufs Abklingen | — |
| Lagrange 3/5 | Bass und Mitte (≤ 1 % Unterschied) | hohe, verlustarme Saiten (S4): Cembalo 60/84, Harfe C7/G7, Gitarre e′ Bund 12, Laute Chor 7 |

---

## 2. Nachgerechnete Beispiele

Alle Zahlen: Umlauf allein am **starren Steg** gegen H_ziel (Z1), fs 48 kHz. Auswahl: Gitarre E2 leer, G-Saite leer / Bund 2
/ Bund 12 (EI 310·10⁻⁶ N m²), e′ Bund 12; Harfe 27 (Metall), 46, 60 (Darm), 96 = C7, 103 = G7 (Nylon); Cembalo 29, 36, 60,
84; Laute Chor 1 (Fluorocarbon, leer), Chor 7 (Nylon, Bund 5); Oud Chor 1 (umsponnen, leer), Chor 5 (Nylon, Bund 5).
Volle Tabellen (dazu Phasenfehler in rad bis 5 / 20 kHz, Betragsfehler max |Δ ln|H|| bis 20 kHz, Zahl der Teiltöne in
±10 Cent / 5 % über **alle** Teiltöne bis 20 kHz): `umlauf_entwurf.txt`.

### 2a Saitendaten (Prüfstand) und Umlauf

| Taste | f₁ Hz | B | Umlauf Takte | Teiltöne ≤ 20 kHz | davon hörbar (bis Hz) | σ₁ 1/s |
|---|---|---|---|---|---|---|
| Gitarre Saite 6 Bund 0 | 82,40 | 1,86e-5 | 583,2 | 188 | 98 (8767) | 0,605 |
| Gitarre Saite 3 Bund 0 | 195,79 | 1,24e-4 | 245,4 | 77 | 38 (8079) | 0,938 |
| Gitarre Saite 3 Bund 2 | 219,77 | 1,56e-4 | 218,6 | 68 | 34 (8119) | 0,949 |
| Gitarre Saite 3 Bund 12 | 391,66 | 4,97e-4 | 122,7 | 38 | 19 (8079) | 1,034 |
| Gitarre Saite 1 Bund 12 | 661,77 | 1,73e-4 | 72,6 | 28 | 20 (13684) | 1,590 |
| Harfe 27 (Metall) | 39,29 | 2,92e-4 | 1228,1 | 167 | 50 (2584) | 0,557 |
| Harfe 46 (Darm) | 116,75 | 3,05e-4 | 412,0 | 91 | 27 (3484) | 0,640 |
| Harfe 60 (Darm) | 261,72 | 2,44e-4 | 183,6 | 57 | 22 (6088) | 1,003 |
| Harfe 96 C7 (Nylon) | 2095,05 | 1,48e-3 | 22,9 | 9 | 7 (15177) | 4,251 |
| Harfe 103 G7 (Nylon) | 3139,67 | 2,33e-3 | 15,3 | 6 | 5 (16131) | 5,633 |
| Cembalo 29 | 43,65 | 4,24e-5 | 1108,2 | 244 | 244 (19991) | 0,102 |
| Cembalo 36 | 65,41 | 1,60e-5 | 734,9 | 226 | 226 (19930) | 0,129 |
| Cembalo 60 | 261,63 | 1,39e-5 | 183,7 | 73 | 73 (19794) | 0,358 |
| Cembalo 84 | 1046,56 | 1,16e-4 | 45,9 | 18 | 18 (19189) | 0,698 |
| Laute Chor 1 leer | 73,45 | 1,02e-3 | 653,9 | 89 | 20 (1742) | 0,421 |
| Laute Chor 7 Bund 5 | 392,01 | 5,34e-5 | 122,6 | 48 | 37 (15025) | 2,049 |
| Oud Chor 1 leer | 65,41 | 3,82e-5 | 734,9 | 195 | 83 (6101) | 0,560 |
| Oud Chor 5 Bund 5 | 261,65 | 2,18e-4 | 183,7 | 58 | 23 (6354) | 1,471 |

### 2b Dispersion — Fehler der Teiltonlage in Cent (Verlust V3, Thiran 1, nachgestimmt; Grundton überall |·| ≤ 0,005 Cent)

Je Zelle: **Teiltöne 1–12 / alle ≤ 5 kHz / hörbare** (größter Betrag). Rechts die Zahl der hörbaren Teiltöne innerhalb
±10 Cent für K32.

| Taste | ohne | K16 (5 kHz) | K16 (20 kHz) | K32 (5 kHz) | Rauhala M8 | adf 5 kHz | K32: in ±10 Cent |
|---|---|---|---|---|---|---|---|
| Git. 6/0 | 2,3/52,4/142 | 0,4/11,6/70 | 0,4/11,6/70 | 0,2/7,7/53 | 1,6/7,9/75 | 0,2/3,0/64 | 61/98 |
| Git. 3/0 | 15,2/59,5/142 | 1,8/3,6/38 | 6,6/11,2/16 | 0,9/1,5/22 | 0,5/15,3/67 | 1,4/23,4/90 | 31/38 |
| Git. 3/2 | 19,1/57,3/143 | 1,6/2,9/36 | 9,0/14,8/15 | 0,8/1,2/20 | 1,9/15,3/68 | 5,4/26,3/96 | 29/34 |
| Git. 3/12 | 58,6/58,6/141 | 1,5/1,5/22 | 36,6/36,6/49 | 0,5/0,5/5,8 | 19,1/19,1/64 | (nap 0) | 19/19 |
| Git. 1/12 | 17,9/5,9/51 | 2,0/0,3/13 | 4,9/2,3/5,4 | 1,0/0,6/3,1 | 13,2/4,3/39 | (nap 0) | 20/20 |
| Harfe 27 | 35/868/473 | 21/479/159 | 21/479/159 | 8,1/353/97 | 6,2/686/325 | 280/2195/1612 | 21/50 |
| Harfe 46 | 37/286/172 | 17/47/24 | 22/33/33 | 6,5/22,8/10,7 | 6,2/165/79 | 6,2/128/54 | 22/27 |
| Harfe 60 | 29/65/96 | 1,8/2,7/10,7 | 16,8/25,5/26 | 0,7/1,3/5,1 | 5,4/19,4/36 | 12,1/33,8/57 | 22/22 |
| Harfe C7 | 102/4,0/62 | 39/0,5/18 | 31/3,2/29 | (K = 16) | 182/6,3/109 | (nap 0) | 5/7 |
| Harfe G7 | 42/0,0/26 | (K = 8) 39/0/23 | 35/0/18 | (K = 8) | 246/0/171 | (nap 0) | 3/5 |
| Cemb. 29 | 5,2/285/1088 | 1,3/160/902 | 1,3/160/902 | 0,7/124/813 | 1,5/195/965 | 2,1/152/211 | 48/244 |
| Cemb. 36 | 2,0/71/516 | 0,3/23/406 | 0,3/23/406 | 0,2/15,8/360 | 1,6/19,4/425 | 0,3/3,6/359 | 64/226 |
| Cemb. 60 | 1,5/3,8/58 | 0,0/0,1/26,5 | 0,4/0,9/18,2 | 0,1/0,1/19,3 | 0,6/1,2/25 | (nap 0) | 61/73 |
| Cemb. 84 | 16,0/1,7/36 | 0,8/0,1/6,6 | 0,4/0,2/6,3 | 1,0/0,1/6,7 | 44/4,5/100 | (nap 0) | 18/18 |
| Laute 1 | 116/857/292 | 87/192/109 | 87/192/109 | 17/86/35 | 34/597/138 | 917/1995/1347 | 9/20 |
| Laute 7 | 6,0/6,0/57 | 0,4/0,4/13 | 2,3/2,3/4,6 | 0,2/0,2/14 | 1,7/1,7/30 | (nap 0) | 33/37 |
| Oud 1 | 4,7/148/202 | 1,1/58/98 | 1,1/58/98 | 0,6/41/72 | 1,6/76/121 | 0,8/35,8/36 | 49/83 |
| Oud 5 | 26/58/93 | 1,4/2,7/12 | 14,2/21,5/22 | 0,6/1,0/5,4 | 4,5/16,5/35 | 13,2/33,5/60 | 23/23 |

`[eigene Rechnung]`. Lesart: an Saiten mit kleinem B·n² (Gitarre ab G, Harfe Mitte, Oud 5, Cembalo ab Mitte) bringt K32
alle hörbaren Teiltöne in ±10 Cent außer 20–22 Cent an der G-Saite (Teiltöne über 5 kHz; mit Band 20 kHz 15–16 Cent, dafür
1–12 schlechter). An Bass-Saiten mit großem B (f/f₀)² reicht keine Kette gleicher Allpässe 1. Ordnung (Harfe 27, Laute 1,
Cembalo 29/36 über 5 kHz); adf ist dort, wo es gilt (B (df/f₀)² ≲ 0,25), ≤ 5 kHz am besten.
**Harfe C7/G7:** 1–12 bleiben bei 31–39 Cent, weil die Teiltöne 3–9 über 5 kHz liegen und die Kette wegen D5 nur K = 8–16
Stufen hat; die Fehler sitzen an Teilton 7–9 (15–19 kHz).

**Gegen S200 (`…_S200_umlauf_filter.txt`, Teiltöne 1–12, mit Körper):** Saite 6 Bund 0 fir5+ap 0,9 / fir9+ap 0,5 Cent —
hier K8 0,6, K16 0,4; Saite 3 Bund 2: 20,2 / 13,5 Cent — hier K8 2,7, K16 1,6; Saite 1 Bund 12: 2,1 / 1,4 — hier K8 0,8,
K16 2,0; Cembalo 29: 3,1 / 2,0 — hier 2,3 / 1,3; Cembalo 84: 0,7 / 0,5 — hier 0,5 / 0,8; Harfe 96: 1,1 / 15,8 — hier
39 / 39 (S200: K = 4/8, Teiltöne bis 12 kHz; hier bis 20 kHz). **Die große Abweichung an der G-Saite (S200 13,5–20 Cent,
hier ≤ 2,7)** stammt nicht aus dem Filterentwurf: S201 (starrer Steg, gleiche Anpassung) kommt dort auf denselben
Bereich wie hier (Mittel 2,4 Cent bei K8). Vermutlich verschiebt der Körper einzelne Teiltöne verschieden stark
`[eigene Einschätzung]` — **an der LumiViz-Seite mit Körper nachzuprüfen**.

### 2c Verlust — Fehler des Abklingens in % (Dispersion K16 5 kHz, Thiran 1)

Je Zelle: **Teiltöne 1–12 / ≤ 5 kHz / hörbare**; * = Entwurf lag über |H| = 1 und ist nach V6 skaliert; rechts die Zahl der
hörbaren Teiltöne innerhalb 5 % mit iir4. Grundton: V2/V3 exakt (0,0 %), iir ≤ 0,2 % (ohne *).

| Taste | Ein-Pol V3 | iir2 | iir4 | iir8 | Kaskade V7 (1 Pol, 2 Null) | iir4: in 5 % |
|---|---|---|---|---|---|---|
| Git. 6/0 | 3,5/74/87 | 0,7/9,3/29 | 0,1/4,7/22 | 0,1/5,0/21 | 43/43/54 | 59/98 |
| Git. 3/0 | 9,2/43/65 | 4,8/15/38 | 0,3/0,5/13,5 | 0,4/2,2/13,3 | 24/24/32 | 31/38 |
| Git. 3/2 | 12,4/41/64 | 4,3/14/37 | 0,3/0,4/13,4 | 0,3/2,0/13,0 | 25/25/33 | 27/34 |
| Git. 3/12 | 24,5/24,5/50 | 7,8/7,8/20 | 0,9/0,9/8,6 | 1,2/1,2/8,5 | 23/23/29 | 16/19 |
| Git. 1/12 | 15,8/15,8/38 | 1,7/1,2/8,8 | 1,1/0,1/4,8 | 1,0/0,1/5,1 | 11/10/22 | 20/20 |
| Harfe 27 | 15,6/89/79 | 3,1/75/43 | 5,3/64/36 | 5,1/63/38 | 43/82/64 | 20/50 |
| Harfe 46 | 12,5/70/56 | 14,6/84/70 | 202* | 57* | 29/59/41 | 2/27 |
| Harfe 60 | 17,4/41/52 | 18,2/40/51 | 0,4/1,9/1,9 | 6,3* | 20/20/30 | 22/22 |
| Harfe C7 | 79/54/79 | 17/2,7/12* | 14,6/0,2/5,8 | 12,9/0,2/6,5 | 55/11/31 | 6/7 |
| Harfe G7 | 88/0,1/88 | 230* | 8373* | 12,4/0,1/8,0 | 49/8,5/30 | 0/5 |
| Cemb. 29 | 40/301/301 | 8,1/33/79 | 30* | 0,2/37/79 | 70/70/78 | 15/244 |
| Cemb. 36 | 41/484/484 | 8,8/8,8/62 | 0,4/9,5/61 | 0,1/7,9/61 | 81/81/81 | 63/226 |
| Cemb. 60 | 40/127/326 | 6,9/6,9/19 | 0,2/0,2/12,8 | 0,0/0,1/8,5 | 60/60/60 | 59/73 |
| Cemb. 84 | 21,6/21,6/21,6 | 3,2/3,1/6,0 | 0,4/0,3/5,1 | 0,4/0,0/2,6 | 21/21/21 | 17/18 |
| Laute 1 | 30/81/43 | 46/95/76 | 27/61/29 | 32*/44/32 | 39/78/39 | 4/20 |
| Laute 7 | 23/23/35 | 3,5/3,5/14 | 0,3/0,3/7,8 | 0,2/0,2/7,9 | 19/19/27 | 31/37 |
| Oud 1 | 3,8/79/83 | 0,9/11/26 | 0,3/19/31 | 0,2/19/27 | 41/52/61 | 49/83 |
| Oud 5 | 14,4/32/47 | 8,7/30/43 | 0,4/0,4/2,4 | 0,3/1,8/5,5 | 15/15/24 | 23/23 |

`[eigene Rechnung]`. Gegen S201 (`…_S201_umlauf_filter_entwurf.txt`, Ein-Pol über die ganze Gitarre 1–12 bis 40 %, FIR
bis 36 %): hier Ein-Pol 3,5–24,5 % an den gerechneten Gitarrentasten (S201s Größtwert liegt an Saite 3 Bund 19, hier nicht
gerechnet). Der Bank-Entwurf iir4, wo passiv, senkt 1–12 an Gitarre, Harfe Mitte, Cembalo, Oud 5 auf ≤ 1 %.

### 2d Bruchteil-Glied — Einfluss auf Abklingen (Verlust iir4 mit Ausgleich des Lagrange-Betrags, K16)

Hörbare Teiltöne, größter Fehler des Abklingens in %; in Klammern die Zahl der Rasterpunkte ≤ 20 kHz (von 40 000), an denen
der Verlust über 1 liegen müsste (S4 verletzt). Die Teiltonlage ändert sich dabei um höchstens wenige Cent (volle Tabelle).

| Taste | Thiran 1 | Thiran 3 | Lagrange 1 | Lagrange 3 | Lagrange 5 |
|---|---|---|---|---|---|
| Git. 6/0 | 22,0 | 22,0 | 33,0 | 22,4 | 21,8 |
| Git. 3/2 | 13,4 | 13,3 | 49,0 | 16,0 | 13,9 |
| Git. 3/12 | 8,6 | 8,5 | 76,8 | 13,2 | 9,0 |
| Git. 1/12 | 4,8 | 4,6 | 158 (35 055) | 50,6 | 17,2 |
| Harfe 60 | 1,9 | 1,8 | 10,5 | 1,7 | 1,6 |
| Harfe C7 | 5,8 | 4,1 | 1260 (38 325) | 600 (29 197) | 215 (18 623) |
| Harfe G7 | (iir4*) | (iir4*) | 56 719 (38 643) | 1077 (30 798) | 498 (22 917) |
| Cemb. 36 | 60,5 | 60,5 | 1191 (28 610) | 71,5 | 66,2 |
| Cemb. 60 | 12,8 | 12,8 | 407 (35 284) | 925 (23 401) | 1428 (15 822) |
| Cemb. 84 | 5,1 | 5,1 | 817 (38 186) | 2225 (30 101) | 12 186 (23 864) |
| Laute 7 | 7,8 | 7,6 | 247 (27 797) | 46,7 | 19,5 |
| Oud 5 | 2,4 | 2,4 | 6,2 | 2,3 | 2,5 |

`[eigene Rechnung]`. Lagrange-Ergebnisse hängen am Bruchteil Δ der Taste (Δ nahe 0,5 am schlimmsten); deshalb ist
Lagrange 5 an Cembalo 84 schlechter als Lagrange 1.

---

## 3. Zeitveränderliche Länge (Bend, Slide, Vibrato)

**Gl. (B1) Verzug je Takt** `[hergeleitet]`: D(n) = fs/f(n) − τ_loss(f(n)) − τ_disp(f(n)) (Phasenverzüge in Takten bei der
laufenden Soll-Tonhöhe); im Lauf unten ohne Dispersion, τ_loss des festen Ein-Pols geschlossen.
**Gl. (B2) Änderungsrate** `[hergeleitet]`: ΔD/Takt = D₀ (1 − 2^(−h/12)) / (t_Bend · fs). *Beispiel* E4, +1 Halbton in
50 ms: 145,62 Takte → 0,00341 Takte je Takt; G3 0,0057; Harfe C4 0,0043; Harfe C7 0,0005.
**Gl. (B3) fester Verlustfilter bei veränderter Länge** `[hergeleitet]`: σ = −ln|H(f)| · f₀ (ein Umlauf je Periode), also
σ_fest/σ_neu ≈ 2^(h/12) bei gleichem Verlust je Umlauf → **+5,9 % je Halbton** aufwärts (+12,2 % bei zwei).
Nachgerechnet (`bend_verzug.py`, statisch): Grundton-Abklingen fest gegen neu entworfen +5,6 / +11,6 % (G-Saite, +1/+2 HT),
+4,2 / +8,7 % (Harfe 60); −5,3 / −10,4 % abwärts.
**Gl. (B4) Lagrange bei laufendem Bruchteil** `[hergeleitet]`: |H_fd(ω; Δ(n))| wechselt mit Δ zwischen 1 (Δ ganz) und S3
(Δ halb) → Amplitudenmodulation im Takt der Bruchteil-Durchläufe; Thiran: |H| = 1, aber rekursiv (Ausschwingen beim
Sprung des ganzzahligen Teils).

**Zahlenbeispiel ±1 Halbton in 50 ms** (`bend_verzug.py`; starrer Steg, eine Polarisation; +1 HT in 50 ms ab 0,15 s, oben
150 ms, −1 HT in 50 ms; Ein-Pol V3 fest; Bezug: dieselbe Schleife mit Kaiser-Sinc, 64 Werte bzw. höchstens 2·(⌊D⌋−1); Messung:
Teiltöne 1–3, komplexe Demodulation am Soll-Verlauf, 10-ms-Hann, Median). Werte **gegen den Bezug** (also nur der Fehler
des Verzugsglieds):

| Saite | Glied | Cent gehalten | Cent Rampe auf (max) | Cent oben | Cent Rampe ab (max) | Hülle dB (max in Rampen / danach) | Differenz > 10 kHz in den Rampen dB re Signal |
|---|---|---|---|---|---|---|---|
| Gitarre E4 (D 146) | Lagrange 1 | 0,00 | 0,63 | 0,00 | 1,03 | 1,34 / −1,34 | −99,8 |
| | Lagrange 3 | 0,00 | 0,01 | 0,00 | 0,02 | 0,02 / −0,02 | −112,7 |
| | Lagrange 5 | 0,00 | 0,01 | 0,00 | 0,02 | 0,01 / −0,01 | −129,4 |
| | Thiran 1 | +0,01 | 0,30 | +0,01 | 0,50 | 0,11 / −0,01 | −84,6 |
| Gitarre G3 (D 245) | Lagrange 1 | 0,00 | 0,38 | 0,00 | 0,70 | 0,67 / −0,68 | −99,5 |
| | Lagrange 3 / 5 | 0,00 | 0,01 | 0,00 | 0,01 | 0,01 / 0,00 | −110 / −114 |
| | Thiran 1 | 0,00 | 0,02 | 0,00 | 0,09 | 0,02 / 0,00 | −83,9 |
| Harfe C4 Darm (D 184) | Lagrange 1 | 0,00 | 0,19 | 0,00 | 0,82 | 0,41 / −0,34 | −119,6 |
| | Lagrange 3 / 5 | 0,00 | 0,01 | 0,00 | 0,02–0,03 | 0,00 / 0,00 | −138 / −140 |
| | Thiran 1 | 0,00 | 0,03 | 0,00 | 0,04 | 0,01 / 0,00 | −99,6 |
| Harfe C7 Nylon (D 23) | Lagrange 1 | (Ton nach 0,2 s erloschen: −55 dB gegen Bezug) | | | | 50 / −55 | −91,5 |
| | Lagrange 3 | −0,02 | 0,56 | −0,33 | 5,94 | 0,71 / −0,79 | −102,1 |
| | Lagrange 5 | 0,00 | 0,31 | −0,02 | 1,93 | 0,04 / +0,01 | −112,3 |
| | Thiran 1 | −0,21 | 2,99 | −0,58 | 5,06 | 0,09 / +0,02 | −78,3 |

Bezug selbst gegen das Soll: gehalten −0,14 … +0,09 Cent; in der Rampe läuft die Messung (10-ms-Fenster) bis 8,5–12,8 Cent
nach — **das ist Messverzug, kein Modellfehler**, darum gegen den Bezug verglichen. Pegel nach dem Bend gegen ohne Bend
−0,2 dB (Gitarre, Harfe C4), −1,1 dB (C7) — Folge von B3 (in der hohen Lage mehr Umläufe je Sekunde mit festem Verlust).

`[eigene Einschätzung]` aus der Tabelle: bei ±1 Halbton in 50 ms (Rate ≤ 0,006 Takte je Takt) ist **keines** der Glieder
hörbar klickend (Differenz über 10 kHz ≤ −78 dB); tonhöhentreu (< 1 Cent) sind Lagrange 3/5 überall außer C7 (bis 6 Cent
in der Rampe ab, L3) und Thiran 1 außer C7 (5 Cent). Lagrange 1 verliert Pegel (−0,3 … −1,3 dB) und an kurzen Saiten den
Ton. **Thiran 1 ohne Zustandskorrektur** (Beiwert jeden Takt neu, ganzzahliger Teil springt) klickt bei dieser Rate nicht;
eine Transientenformel und Grenzen in Takten je Takt aus der Literatur liegen nicht vor (Laakso 1996, Välimäki 1995 nicht
erreicht). Schnellere Änderungen (Slide über 5 Bünde in 50 ms: Rate ≈ 0,03 Takte je Takt an E4) sind **nicht gerechnet**.

**Nachführen von Dispersion und Verlust** (`bend_verzug.py`, statisch, hörbare Teiltöne):

| Saite, Bend | a (K16) neu / fest | Cent max neu / fest | Abklingen % max neu / fest | Grundton-Abklingen neu / fest |
|---|---|---|---|---|
| G-Saite −2 HT | −0,4985 / −0,4828 | 42,0 / 52,0 | 66,2 / 69,8 | 0,0 / −10,4 |
| G-Saite −1 HT | −0,4915 / −0,4828 | 42,0 / 47,5 | 66,2 / 68,0 | 0,0 / −5,3 |
| G-Saite +1 HT | −0,4755 / −0,4828 | 36,8 / 31,9 | 64,4 / 62,4 | 0,0 / +5,6 |
| G-Saite +2 HT | −0,4665 / −0,4828 | 36,2 / 24,5 | 63,7 / 59,6 | 0,0 / +11,6 |
| Harfe 60 −2 HT | −0,4701 / −0,4548 | 13,4 / 22,3 | 54,0 / 58,4 | 0,0 / −7,6 |
| Harfe 60 +2 HT | −0,4389 / −0,4548 | 8,5 / 6,2 | 47,3 / 42,3 | 0,0 / +8,7 |

Lesart `[eigene Einschätzung]`: der Allpass-Wert ändert sich je Halbton um rund 0,008 — glatt in L; eine **Tafel über die
Länge mit linearer Interpolation** trägt ihn (a ändert sich je Halbton um < 2 %). Der Grundton-Abklingfehler eines festen
Verlusts folgt B3 (≈ 6 % je Halbton); bei ±1 HT bleibt er unter der 5-%-Marke nur, wenn der Verlust mitgeführt wird. Dass
„fest“ an einzelnen Stellen weniger Cent zeigt als „neu“, liegt daran, dass D4 nur bis 5 kHz anpasst, die hörbaren Teiltöne
aber bis 8 kHz reichen. Das Umstellen eines Allpasses mit veränderlichem Beiwert ist hier nicht auf Transienten geprüft
(Allpass 1. Ordnung wie Thiran 1: bei dieser Rate unauffällig).

---

## 4. Kopplung an die Modalbank am Steg

**Gl. (M1) eine Mode** `[Prüfstand]`, Z. 44: Y(s) = φ² s/(s² + η ω_n s + ω_n²), Q = 1/η. Pol λ = −η ω_n/2 + i ω_d,
ω_d = ω_n √(1 − η²/4); Residuum R = λ/(2 i ω_d); Y(s) = R/(s − λ) + R*/(s − λ*) `[hergeleitet]`.

Diskretisierungen (je Mode aus Pol und Residuum; p = e^{λT}, x = λT, T = 1/fs) `[hergeleitet]`:
- **(M2) impulsinvariant:** H(z) = T [R/(1 − p z⁻¹) + c.c.].
- **(M3) „trapez“ (heutige Form des Kerns):** H_M2 − T h(0)/2, h(0) = 2 Re R = 1; deckt sich mit `moden_diskret` des
  Kandidaten (B0 = c T/2, B1 = c T p/2, c = 2R).
- **(M4) bilinear:** s = (2/T)(1 − z⁻¹)/(1 + z⁻¹); vorverzerrt: s = (ω_n / tan(ω_n T/2))(1 − z⁻¹)/(1 + z⁻¹). Bank &
  Karjalainen 2010: „If a positive real function H(s) is converted to a discrete-time function H(z) by the bilinear
  transform, it remains positive real“ `[Stufe 1: wörtlich, Seitenbild geprüft]`.
- **(M5) foh (Kraft stückweise linear):** H(z) = R [(p − 1 − x) + (1 − p + x p) z⁻¹] / (λ x (1 − p z⁻¹)) + c.c.; deckt sich
  mit dem Kandidaten (B0 = c (p − 1 − x)/(λx), B1 = c (p(x − 1) + 1)/(λx)).
- **(M6) Höhe der Spitze bei foh** `[hergeleitet]`: Dreieckshalt → Faktor sinc²(f/fs): −0,31 / −0,80 / −1,82 / −3,30 dB bei
  5 / 8 / 12 / 16 kHz; nachgerechnet (Tabelle) −0,31 / −0,80 / −1,81…−1,82 / −3,21…−3,30.

**Fehler breiter Moden bei 48 kHz** (`moden_diskret.py`; Spitze von |Y|, Güte aus der −3-dB-Breite, Höhe der Spitze;
„Band“ = größter dB-Fehler von |Y| zwischen f_n/2 und min(2 f_n, 23,9 kHz); Passivität = min Re Y_d / max Re Y):

| f_n, Q | ii: Cent / Q % / Höhe dB | trapez | bilinear | bilinear vorverzerrt | foh | Band-Fehler dB ii / trapez / bil. / bil. v. / foh |
|---|---|---|---|---|---|---|
| 5 kHz, 5 | −1,1 / −0,2 / +0,56 | −1,3 / +0,2 / +0,01 | −59 / +7,1 / 0,00 | 0 / +7,6 / 0,00 | −1,3 / +0,2 / −0,31 | 1,6 / 1,0 / 2,1 / 1,7 / 1,4 |
| 5 kHz, 10 | −0,3 / −0,1 / +0,28 | −0,3 / +0,1 / 0,00 | −59 / +7,0 / 0,00 | 0 / +7,5 / 0,00 | −0,3 / 0,0 / −0,31 | 1,4 / 1,0 / 3,4 / 1,7 / 1,4 |
| 5 kHz, 30 | 0,0 / 0,0 / +0,09 | 0,0 / 0,0 / 0,00 | −59 / +7,0 / 0,00 | 0 / +7,5 / 0,00 | 0,0 / 0,0 / −0,31 | 1,2 / 1,0 / 8,3 / 1,7 / 1,4 |
| 8 kHz, 5 | −2,8 / −0,5 / +0,90 | −3,4 / +0,5 / +0,04 | −142 / +17,6 / 0,00 | 0 / +21,2 / 0,00 | −3,4 / +0,5 / −0,80 | 3,1 / 3,2 / 6,0 / 5,0 / 4,1 |
| 8 kHz, 10 | −0,8 / −0,1 / +0,45 | −0,9 / +0,1 / +0,01 | −142 / +17,4 / 0,00 | 0 / +21,0 / 0,00 | −0,9 / +0,1 / −0,80 | 2,8 / 3,3 / 7,8 / 5,0 / 4,2 |
| 8 kHz, 30 | −0,1 / 0,0 / +0,15 | −0,1 / 0,0 / 0,00 | −142 / +17,4 / 0,00 | 0 / +20,9 / 0,00 | −0,1 / 0,0 / −0,80 | 2,6 / 3,3 / 15,7 / 5,0 / 4,2 |
| 12 kHz, 5 | −6,3 / −1,2 / +1,36 | −8,2 / +1,1 / +0,11 | −286 / +37,5 / 0,00 | 0 / +57,6 / 0,00 | −8,5 / +1,1 / −1,81 | 5,2 / 10,5 / 42 / 40 / 19 |
| 12 kHz, 10 | −1,8 / −0,3 / +0,68 | −2,1 / +0,3 / +0,03 | −286 / +37,2 / 0,00 | 0 / +57,2 / 0,00 | −2,2 / +0,3 / −1,82 | 4,8 / 16 / 42 / 40 / 25 |
| 12 kHz, 30 | −0,2 / 0,0 / +0,23 | −0,2 / 0,0 / 0,00 | −286 / +37,0 / 0,00 | 0 / +57,0 / 0,00 | −0,2 / 0,0 / −1,82 | 4,6 / 26 / 42 / 40 / 33 |
| 16 kHz, 5 | −11,8 / −2,3 / +1,89 | −16,4 / +1,5 / +0,32 | −448 / +62,4 / 0,00 | 0 / +142 / 0,00 | −18,0 / +1,7 / −3,21 | 7,2 / 6,2 / 45 / 40 / 14 |
| 16 kHz, 10 | −3,8 / −0,6 / +0,94 | −4,6 / +0,4 / +0,08 | −448 / +62,0 / 0,00 | 0 / +142 / 0,00 | −4,7 / +0,4 / −3,28 | 6,8 / 12 / 45 / 41 / 20 |
| 16 kHz, 30 | −0,5 / −0,1 / +0,31 | −0,5 / +0,1 / +0,01 | −448 / +61,8 / 0,00 | 0 / +142 / 0,00 | −0,5 / 0,0 / −3,30 | 6,5 / 21 / 45 / 41 / 29 |

`[eigene Rechnung]`. Die großen Band-Fehler (10–45 dB) liegen in den Flanken nahe fs/2 (Faltung bzw. die Nullstelle der
bilinearen Abbildung bei fs/2), nicht an der Spitze.

**Passivität je Mode** (`moden_passiv_raster.py`, f_n 100 Hz – 22 kHz, Q 0,6 – 100): min Re Y_d / max Re Y ist für **alle
fünf** Formen ≥ 0 — an keinem Rasterpunkt negativ. Unterschied: **impulsinvariant und trapez tragen einen breiten
Wirkanteil** auch fern der Mode (Q 1,5 bei 10 kHz: ii 0,50, trapez 0,066 der Spitze; foh 4,5·10⁻⁹, bilinear 8·10⁻¹⁰).
`[eigene Einschätzung]`: das ist der Mechanismus hinter den Fehlern des Kandidaten mit Dämpfer bei „trapez“ (S200: Pegel
1–2 s bis −8 dB, Abfall bis 15,6 dB) — die breiten Dämpfer-Moden (Q 1,5) dämpfen über trapez auch die tiefen Teiltöne; foh
nicht.

**Durchgriff** (z → ∞, für I + Z₀ Y_i): ii 20,8 µs, trapez 10,4 µs (= T/2), bilinear 3,9–9,6, foh 6,3–10,0 µs je φ² = 1 —
**immer > 0**, also Y_i = Σ φ² d₀ ⪰ 0 und I + Z₀ Y_i invertierbar.

**Gl. (M7) Bedingungen für Passivität der geschlossenen Schleife** `[hergeleitet]` (mit Bank & Karjalainen Gl. (1), (2),
(6), (13)–(17) `[Stufe 1 bzw. dieser Lauf: wörtlich, Seitenbild geprüft]`, S. DAFX-4 hier am Bild gelesen):
 (i) jede Mode stabil (|p| < 1) und **Re Y_d(e^{iω}) ⪰ 0 für alle ω**; bei zwei Polarisationen
  Y_d(e^{iω}) + Y_d(e^{iω})ᴴ ⪰ 0 — hinreichend: jede Mode ein nichtnegatives Gewicht φ_r² bzw. eine Matrix d_r d_rᵀ ⪰ 0
  (Bank & Karjalainen Gl. 6) und jede einzelne Mode passiv diskretisiert (oben: alle fünf Formen);
 (ii) Stegreflexion R_b = (I + Z₀ Y_d)⁻¹ (Z₀ Y_d − I) (Prüfstand Z. 212–214); mit Z₀ = diag(z₀) > 0 gilt
  ‖R_b(e^{iω})‖₂ ≤ 1 ⟺ (i); skalar |R_b|² = (|Z₀Y|² − 2 Re Z₀Y + 1)/(|Z₀Y|² + 2 Re Z₀Y + 1) ≤ 1 ⟺ Re Z₀Y ≥ 0;
 (iii) Umlauf |H_m(e^{iω})| ≤ 1 für alle ω (V3/V6; Thiran |H| = 1; Lagrange nur mit S4);
 (iv) **kleine Kreisverstärkung:** ‖H_m(e^{iω}) R_b(e^{iω})‖ < 1 für alle ω ⇒ geschlossene Schleife stabil
  (Satz von der kleinen Kreisverstärkung; R_b stabil, weil I + Z₀ Y_d bei positiv reellem Y_d keine Nullstelle außerhalb
  hat).
 Der Schritt mit Durchgriff (Bank & Karjalainen Gl. 17; Kandidat u = (I + z₀M)⁻¹(u_frei + 2 z₀ M v⁺)) ist eine exakte
 Umformung derselben linearen Gleichung — er ändert die Passivität nicht.
*Zahlenbeispiel:* Gitarre E2 z₀ = 0,668 N s/m (Kandidat); eine foh-Mode 5 kHz, Q 10, φ² = 1: Z₀ Y_i = 0,668 · 9,8·10⁻⁶ —
I + Z₀ Y_i ≈ 1,0000066, gut bedingt.

---

## 5. Rechenaufwand je Abtastwert

**Gl. (R1)** `[hergeleitet]` — Multiplikationen je Takt und Stimme, P Polarisationen, M Moden am Steg:

 C = P · [C_fd + C_loss + C_disp] + M · (8 + 2P) + 2P² + 2P

mit C_fd = 1 (Thiran 1, Ein-Multiplikator-Form η(x − y₁) + x₁), 2N + 1 (Thiran N), N + 1 (Lagrange N; bei laufendem Δ
zusätzlich ≈ N (N + 1) für die Beiwerte, Farrow); C_loss = 2 (Ein-Pol), 2N_L + 1 (IIR N_L); C_disp = K (Allpass 1. Ordnung,
Ein-Multiplikator-Form; Välimäki 2004 Tab. 1 rechnet 2 Mult. je Stufe `[Stufe 1: wörtlich, Seitenbild geprüft]`) bzw.
2 · nap (Biquad-Allpass in der Form a₁(x₁ − y₁) + a₂(x − y₂) + x₂); je Mode: komplexer Zustand p·z (4), zwei Einspeisungen
B₁, B₀ (je 2), Projektion e·F (P) und Ausgabe Re z · e (P). Zum Vergleich Bank & Karjalainen: „Roughly, the required
computational power is RK(K + 3) multiply and accumulate instructions per sample.“ `[wörtlich, Seitenbild geprüft]`
(S. DAFX-4, hier am Bild gelesen); mit K = P = 2: 10 je Mode — dieselbe Größe wie 8 + 2P = 12.

**Speicher** `[hergeleitet]`: Verzugsleitung ⌈fs/f₀⌉ + N_fd Werte je Polarisation; je Mode 2 Zustände + 6 Beiwerte
(p, B₀, B₁ komplex) + P Formwerte; Filterzustände 1 je Allpass-Stufe, 2 je Biquad, N_L je IIR.

**Modenzahlen** (aus den Körperdateien gelesen, alle bis 5 kHz; dazu 14 Dämpfer-Moden nach GZ5-1 a wie im Kandidaten):
Gitarre 75, Harfe 145, Cembalo 389, Laute 128, Oud 243.

**Zahlenbeispiele** `[eigene Rechnung]`, P = 2:

| Fall | Umlauf je Polarisation | Umlauf × 2 | Moden M (8 + 2P = 12) | Steg 2P² + 2P | Summe | Anteil Umlauf |
|---|---|---|---|---|---|---|
| Gitarre, Ein-Pol + K16 + Thiran 1 | 2 + 16 + 1 = 19 | 38 | 89 × 12 = 1068 | 12 | 1118 | 3,4 % |
| Gitarre, iir8 + K32 + Thiran 1 | 17 + 32 + 1 = 50 | 100 | 1068 | 12 | 1180 | 8,5 % |
| Gitarre, iir4 + adf (nap 3, E2) + Lagrange 5 (laufend, +30) | 9 + 6 + 36 = 51 | 102 | 1068 | 12 | 1182 | 8,6 % |
| Harfe, iir8 + K32 + Thiran 1 | 50 | 100 | 159 × 12 = 1908 | 12 | 2020 | 5,0 % |
| Gitarre mit Mitschwingen (6 Saiten), iir8 + K32 | 6 × 100 = 600 | — | 1068 + Kopplung | — | ≥ 1680 | — |

Die Quellen geben **keine** Zeiten auf genannter Hardware (Ducceschi u. a. 2026: MATLAB, „a standard workstation“, Stufe 1).
Nur als Anhalt, nicht als Quelle: LumiViz' eigene Messung S200 (`…_zeitbereich_rechenzeit.txt`) — Gitarre 89 Moden mit
16-Werte-Umlauf 0,84 %, Umlauf allein 0,06 % der Echtzeit. `[eigene Einschätzung]`: weil die Moden rund 90 % der
Multiplikationen tragen, verschiebt die Wahl des Umlauf-Filters (19 → 50 Mult. je Polarisation) die Rechenzeit um
Zehntelprozent der Echtzeit, nicht um ganze — die 4,6 % entscheiden zwischen den Optionen unten nicht.

---

## 6. Parametertabelle

| Größe | Zahl | Einheit | Quelle | Marke | Gültigkeit |
|---|---|---|---|---|---|
| fs | 48 000 | Hz | Auftrag | — | — |
| Saitendaten (T, m, EI, η_F, η_A, η_B, L) | je Taste, Tab. 2a | N, kg/m, N m², 1, 1/s, 1, m | Prüfstand (Woodhouse 2012 Tab. I; Bell 1988; Taskin/Di Veroli; W&LA 2019) | [Prüfstand] | wie dort |
| Rauhala k₁, k₂, k₃ | −0,00179; −0,0233; −2,93 | — | Faust `effect.lib` | [Stufe 1: Werkzeug] | Klavier, 44,1 kHz; hier überall schlechter als D4 |
| Rauhala m₁…m₄ | 0,0126; 0,0606; −0,00825; 1,97 | — | ebd. | [Stufe 1: Werkzeug] | ebd. |
| adf β | 0,85 | — | `adf.m` Beispiel | [Stufe 1: Werkzeug] | B (df/f₀)² ≲ 0,25 [eigene Rechnung] |
| adf df | 5000 | Hz | gesetzt (wie S201) | [eigene Einschätzung] | — |
| Ein-Pol a, b G-Saite | −0,210212; 0,786102 | — | V2 | [eigene Rechnung] | Taste 55 |
| K16-Wert a G-Saite | −0,4828 | — | D4 | [eigene Rechnung] | Taste 55, Band 5 kHz |
| Thiran-1 η G-Saite | −0,0182 (Δ 1,0370, N 198) | — | S1/S2 | [eigene Rechnung] | Taste 55 |
| Bank-Gewicht | 1/(1 − g)⁴ | — | Bank & Välimäki 2003 Gl. 2 | [Stufe 1: wörtlich, Seitenbild geprüft] | — |
| Hörbarkeitsgrenze | σ ≤ 100 | 1/s | gesetzt | [eigene Einschätzung] | — |
| Lagrange-Betrag Δ 0,5 | 0,947/0,793 (N1), 0,996/0,940 (N3), 0,9996/0,981 (N5) bei 5/10 kHz | — | S3 | [eigene Rechnung] | fs 48 kHz |
| Durchgriff je Mode (φ² = 1) | trapez 10,4; foh 6,3–10,0; ii 20,8 | µs (s/kg) | M2–M5 | [eigene Rechnung] | 5–16 kHz, Q 5–30 |
| Modenzahl bis 5 kHz | Gitarre 75, Harfe 145, Cembalo 389, Laute 128, Oud 243 | — | Körperdateien | [wörtlich aus der Datei] | GZ5 a |
| Bend-Rate ±1 HT / 50 ms | 0,0034 (E4), 0,0057 (G3), 0,0043 (C4), 0,0005 (C7) | Takte je Takt | B2 | [eigene Rechnung] | — |

---

## 7. Annahmen, die bleiben

1. Bewertet ist der **Umlauf am starren Steg**. Lautheit und alles, was der Körper an den Teiltönen verschiebt, ist hier
   nicht gerechnet (S200 tat es; die Abweichung an der G-Saite, 2b, zeigt, dass der Körper dazukommt).
2. „Hörbar“ = σ ≤ 100 1/s ist eine Setzung; die Rangfolge der Verfahren ändert sie nicht (volle Spalten in der .txt).
3. H_ziel (Z1) mit allen Saitendaten aus dem Prüfstand; die dort vermerkten Annahmen (Metallsaiten der Harfe: EI vom
   Stahlkern, Verluste übertragen; Cembalo η_B = 10⁻³; Oud-Mensur 0,60 m) tragen in die Zahlen hier hinein — die größten
   Fehler (Harfe 27, Laute 1, Cembalo-Bass) sitzen genau an Saiten mit solchen Annahmen.
4. Rauhala- und adf-Formeln nur als Abschrift eines Abrufwerkzeugs (Stufe 1); die Gegenprobe S201 und hier zeigen
   plausible Werte, das Papier ist nicht gelesen.
5. Bend: eine Polarisation, starrer Steg, ohne Dispersion, Ein-Pol fest; Bezug ist eine Sinc-Interpolation, nicht ein
   physikalisches Modell der laufenden Länge (Energiefrage nach Pakarinen 2005 offen).
6. Passivität der Moden am Raster geprüft (f_n 100 Hz – 22 kHz, Q 0,6 – 100), nicht bewiesen.

---

## 8. Optionen für LumiViz

Keine Empfehlung nach Aufwand; jede Option gegen alle Zielgrößen. „1–12“ = Teiltöne 1–12, „hörbar“ wie oben; Zahlen aus
2b–2d und 3.

**Option A — Tafel je Taste, hohe Ordnung, Thiran 1.**
Verlust iir4/iir8 nach V4/V5 offline, mit Passivitätsprüfung V6 und der Regel „kleinster Fehler unter den passiven
Ordnungen 2/4/8, sonst Ein-Pol V3“; Dispersion K32 gleiche Allpässe (D4, ein Wert je Taste; an hohen Tasten durch D5
weniger), an Bass-Saiten mit B (5 kHz/f₀)² ≲ 0,25 stattdessen adf (D3); Thiran 1, nachgestimmt (S1); Moden foh.
Daten: je Taste a (bzw. nap Biquads), N_L + 1 + N_L Verlustwerte, Δ, N; bei Bend eine Tafel über die Länge.
- *Tonhöhe:* Grundton exakt; 1–12 ≤ 1 Cent außer Harfe C7/G7 (39), Harfe 27 (8), Harfe 46 (6,5), Laute 1 (17);
  hörbar ≤ 22 Cent außer Bass-Saiten (Gitarre E2: 53, Harfe 27: 97, Laute 1: 35, Oud 1: 36–72, Cembalo 29/36 über
  5 kHz: 211/360).
  Bend ±1 HT/50 ms: ≤ 0,5 Cent (5 Cent an C7).
- *Abklingen:* 1–12 ≤ 1,1 % an 13 von 18 Tasten; hörbar 2–22 % an Gitarre, Harfe Mitte, Oud 5, Laute 7; 36–79 % an
  Bass-Metall, Cembalo-Bass (über 5 kHz), Laute 1; mit festem Verlust bei Bend +5,9 % je Halbton, mit Tafel über die Länge 0.
- *Lautheit:* hier nicht gerechnet (am Prüfstand nach S200 zu messen).
- *Rechenzeit:* ≈ 100 Mult. für den Umlauf gegen ≈ 1080 für Moden und Steg (Gitarre) — +5 % gegenüber Option C.
- *48 kHz:* alle Entwürfe bei 48 kHz gerechnet; Rauhala nicht verwendet.
- *Kippt:* wenn V6 an einer Taste keine passive Ordnung findet (an 18 Tasten: Harfe 46 fällt auf den Ein-Pol mit 56 %);
  dann bräuchte es einen Entwurf mit Nebenbedingung |H| ≤ 1 (nicht gerechnet).

**Option B — wie A, aber Lagrange 5 statt Thiran 1.**
- *Tonhöhe:* gehalten wie A; Bend besser an E4/G3/C4 (≤ 0,02 Cent statt ≤ 0,5), an C7 1,9 statt 5 Cent.
- *Abklingen:* **bricht an hohen, verlustarmen Saiten** (S4): Cembalo 60/84 +1428 % / +12 186 %, Harfe C7/G7 +215 / +498 %,
  Gitarre e′ Bund 12 17 %, Laute 7 19,5 %; sonst wie A. Mit Lagrange 3: ähnlich (bis +2225 %).
- *Rechenzeit:* +5 (fest) bzw. +35 Mult. (laufend) je Polarisation.
- *Kippt:* nur tragbar, wenn Lagrange auf Tasten beschränkt wird, die S4 erfüllen (Bass und Mitte), sonst Thiran.

**Option C — geschlossen ohne Tafel für den Verlust: Ein-Pol V2/V3 + K16 (D4) + Thiran 1.**
- *Tonhöhe:* wie A mit K16 (hörbar ≤ 38 Cent an G-Saite, 70 an E2).
- *Abklingen:* Grundton exakt; 1–12 9–40 % außer Gitarre E2/Oud 1 (≤ 4 %); hörbar 22–484 %.
- *Lautheit:* S200 maß für den Ein-Pol am Cembalo bis 1,66 LU (> 1 LU), Gitarre 0,15 LU.
- *Rechenzeit:* geringste (≈ 38 Mult. Umlauf je Stimme).
- *Kippt:* gegen das Ziel „Abklingen je Teilton“ an allen Instrumenten außer den genannten zwei Tasten.

**Option D — Moden trapez statt foh (sonst A).**
- *Abklingen/Lautheit:* einzelne Moden genauso gut (Spitze ≤ 16 Cent, Q ≤ 1,5 %, Höhe ≤ 0,3 dB statt foh −0,3…−3,3 dB),
  aber breite Moden (Dämpfer, Q 1,5) tragen 6,6 % ihres Wirkanteils bis in die tiefen Teiltöne; der Kandidat S200 zeigt
  dafür Lautheit −0,17 … −1,19 LU und Pegel 1–2 s bis −8 dB.
- *Rechenzeit:* gleich.
- *Kippt:* nur ohne breite Moden. Mit foh ist der Höhenfehler bekannt und geschlossen (sinc², M6) — er ließe sich durch
  φ²/sinc²(f_n/fs) je Mode ausgleichen `[hergeleitet, nicht gerechnet]`.

---

## 9. Was die LumiViz-Sitzung prüfen muss

1. **Lautheit und Teiltöne mit Körper** für A/B/C am Prüfstand (Verfahren S200: H_ziel durch H_m ersetzen) — besonders
   G-Saite Bund 2 (S200 13,5–20 Cent, hier ≤ 2,7 Cent am starren Steg) und Cembalo 29/84 (S200: 1,1–1,7 LU).
2. **Passivität des Verlusts je Taste über den ganzen Umfang** (V6): hier lag der Bank-Entwurf an 5 von 18 Tasten über 1;
   die Tasten, an denen keine Ordnung passiv ist, bestimmen, ob ein Entwurf mit Nebenbedingung nötig wird.
3. **Bass-Saiten mit großem B (f/f₀)²** (Harfe Metall, Laute Chor 1, Cembalo 29/36, Oud 1): keine der Ketten trägt sie;
   dort zuerst die Prüfstand-Annahmen prüfen (EI vom Kern, η_B), bevor ein Filter höherer Ordnung gesucht wird.
4. **Harfe C7/G7:** 1–12 bleiben bei 31–39 Cent (Teiltöne 7–9 über 15 kHz) — ob das hörbar zählt, ist eine Frage an die
   Grenze „hörbar“ (dort 18–23 Cent).
5. **Bend:** schnellere Raten (Slide ≈ 0,03 Takte je Takt) und Thiran mit Dispersionskette im Lauf nicht gerechnet.
6. **Faust-/MATLAB-Zeilen** (D2, D3) weiter nur `[Werkzeug]`; Rauhala ist nach diesen Rechnungen nicht nötig.
7. **Hörbarkeitsgrenze** σ ≤ 100 1/s festlegen oder ersetzen.

---

## 10. Nicht gefunden / nicht herleitbar

| Lücke | Suchweg | Stand |
|---|---|---|
| Fehlermaße und Erratum Rauhala & Välimäki 2006 | dafx.de (ECONNREFUSED, dritter Versuch) | nicht erreicht; durch Rechnung ersetzt (2b) |
| Genaue Stabilitätsgrenze Thiran N | JOS PASP abgerufen: nur „sufficiently large Δ“ | nicht in der Quelle; hier nur N = 1, 3 mit Δ ∈ [N − 0,5, N + 0,5) gerechnet, stabil |
| Transientenformel / Grenzrate klickfreies Umstellen (Välimäki 1995, Laakso 1996, Van Duyne 1997) | Stufe 1; dieser Lauf keine neue Suche | nicht gefunden; Ersatz: Zeitschritt-Rechnung (3) |
| Energiefehler bei veränderlicher Länge (Pakarinen 2005) | Stufe 1: 403 | nicht erreicht |
| Passiver Verlustfilter höherer Ordnung mit Nebenbedingung | — | nicht gerechnet (V7 als Versuch, negativ) |
| Zeiten auf genannter Hardware | Stufe 1 | keine Quelle; nur LumiViz-eigene Messung |
| Fehlervergleich foh/ii/bilinear in einer Quelle | Stufe 1 | keine Quelle; hier gerechnet (4) |

## 11. Ladeliste (nichts geladen; nur Dateien, die nicht unter `…\cmake\quellen\gezupfte-saite\` liegen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Zugang | wozu |
|---|---|---|---|---|
| `Bank_Valimaki_2003_RobustLossFilter_SPL10.pdf` | https://home.mit.bme.hu/~bank/publist/spl03.pdf | 200,8 KB | Autorenseite, IEEE © | V4; liegt schon unter `ergebnisse/…_01_…/abrufe/` |
| `Bank_Karjalainen_2010_PassiveAdmittanceMatrix_DAFx10.pdf` | https://home.mit.bme.hu/~bank/publist/dafx10adm.pdf | 240,1 KB | Autorenseite | M7, R1; liegt schon unter `…_01_…/abrufe/` |
| `Rauhala_Valimaki_2006_TunableDispersion_DAFx06.pdf` | über https://dafx.de/paper-archive/ (DAFx-06, S. 71–76) | unbekannt | frei; dreimal nicht erreichbar | D2 Fehlermaße |
| `Abel_Smith_2006_HighOrderAllpassDispersion_DAFx06.pdf` | über https://dafx.de/paper-archive/ | unbekannt | frei; nicht erreichbar | Dispersion hoher Ordnung für Bass-Saiten (Punkt 9.3) |
| `Pakarinen_etal_2005_EnergyTimeVaryingFD_ICASSP.pdf` | https://core.ac.uk/download/28975045.pdf | unbekannt | 403 in Stufe 1 | Punkt 3 Energie |
| `Rocchesso_2000_FractionallyAddressedDelayLines_TSAP.pdf` | https://arxiv.org/pdf/cs/0005022 | unbekannt | arXiv | Punkt 3 |
| `Valimaki_1995_FractionalDelay_Diss.pdf` | nicht gesucht (Helsinki University of Technology) | — | — | Transienten-Beseitigung Thiran |

---

## Am Ende: nicht geliefert

- **Lautheit je Taste** (≤ 1 LU) für die Optionen — nicht gerechnet (nur Umlauf am starren Steg); S200-Werte zitiert.
- **Skripte als Codeblock** — nur Kernzeilen (Anhang A), die Skripte liegen im Ordner.
- Je Instrument **zwei Tasten** sind geliefert (Gitarre 5, Harfe 5, Cembalo 4, Laute 2, Oud 2); Laute und Oud nur mit je
  einer Saite des Chors.
- Bend nur ±1 HT/50 ms; Vibrato und schneller Slide nicht gerechnet.
- Transientengleichung für Thiran mit Zustandskorrektur, Farrow-Strukturen: nicht hergeleitet.

---

## Anhang A — Kernzeilen der Entwurfsformeln (aus `umlauf_entwurf.py`, `moden_diskret.py`)

```python
def pol1_2p(Gt, f1, f2):            # Gl. (V2)
    g1, g2 = np.interp([f1, f2], F, Gt)
    c1, c2 = np.cos(2*np.pi*f1/FS), np.cos(2*np.pi*f2/FS)
    r = g1**2 / g2**2
    p = (c2 - r*c1) / (1 - r)
    a = -p + np.sign(p)*np.sqrt(max(p*p - 1, 0.0))      # |a| < 1 (sonst 1/a)
    b = g1*np.sqrt(1 + 2*a*c1 + a*a)                   # H = b / (1 + a z^-1)

def pol1_passiv(Gt, f1, f2):        # Gl. (V3): falls b/(1-|a|) > 1
    # (1 + a)^2 = g1^2 (1 + 2 a c1 + a^2),  b = 1 + a
    wur = np.roots([1 - g1*g1, 2*(1 - g1*g1*c1), 1 - g1*g1])

def thiran(D, N):                   # Gl. (S2)
    a = [(-1)**k * comb(N, k) * prod((D-N+n)/(D-N+k+n) for n in range(N+1)) for k in range(N+1)]
    return a[::-1], a               # Zaehler, Nenner

def lagrange(D, N):                 # Gl. (S3)
    h[k] = prod((D - m)/(k - m) for m != k)

# Gl. (M5), foh je Mode:  p = exp(lam T), x = lam T, R = lam/(2 i w_d)
b0 = R*(p - 1 - x)/(lam*x);  b1 = R*(1 - p + x*p)/(lam*x)
H  = (b0 + b1*zi)/(1 - p*zi) + (conj(b0) + conj(b1)*zi)/(1 - conj(p)*zi)
```

## Anhang B — Ausgabe `beispiele.py`

```
(Z1/Z2) G-Saite: f1 195.793 Hz (c/2L = 195.781 Hz), sigma_1 0.9376 1/s, sigma_8 2.327, sigma_16 8.599
(Z4) B = pi^2 EI/(T L^2) = 1.242e-04; f_n-Naeherung n f0 sqrt(1+B n^2) fuer n=12: 2370.29 Hz, exakt 2370.29 Hz
(Z3) Naeherung sigma(f_1) = ... = 0.938 1/s (exakt 0.938)
(Z3) Naeherung sigma(f_8) = ... = 2.333 1/s (exakt 2.327)
(Z3) Naeherung sigma(f_16) = ... = 8.809 1/s (exakt 8.599)
(Z5) -ln G je Umlauf bei f1: 4.788e-03; sigma_1/f1 = 4.789e-03
(V2) Ein-Pol durch f1 und 8 f1: b 0.786102, a -0.210212; |H(f1)| 0.995223 = Ziel 0.995223; |H(8f1)| 0.988381 = Ziel 0.988381; |H(0)| = b/(1+a) = 0.995333 <= 1
(V4) Bank & Valimaki: tau_k = 1.0666, 0.9673, 0.8712 s; Gewicht 1/(1-g)^4 (rel. zu k=1): 1.0, 0.677, 0.446
(D1/D5) K16, a -0.4828: Phasenverzug je Stufe bei 0 = (1-a)/(1+a) = 2.867 Takte; K mal = 45.9
(S1) Umlauf 245.417 Takte = N 198 + Delta 1.0370 + Filter; Thiran-1-Wert eta = (1-D)/(1+D) = -0.0182
(D2) Rauhala-Formel M 8: a1 = -0.5498; (D3) adf, df 5 kHz: nap = 1 Biquads
(S3) Lagrange 1, Bruchteil 0,5: |H(5 kHz)| = 0.9469  (cos(w/2) = 0.9469); |H(10 kHz)| = 0.7934
(S3) Lagrange 3: 0.9959 / 0.9404;  Lagrange 5: 0.9996 / 0.9812
(S4) Cembalo 84: -ln G(5 kHz) je Umlauf 1.31e-03 gegen -ln|H_Lagrange1(Delta 0,5)| = 5.45e-02
(B2) E4: Verzug 145.62 Takte, +1 Halbton in 50 ms: 0.00341 Takte je Takt
(B3) fester Verlust, +1 / +2 Halbtoene: Grundton-Abklingen + 5.9 / + 12.2 % (Naeherung)
(M6) foh: 20 log sinc^2(f/fs) = -0.31 / -0.80 / -1.82 / -3.30 dB bei 5 / 8 / 12 / 16 kHz
```
(Ausgabe gekürzt auf eine Zeile je Gleichung; die Werte sind unverändert.)

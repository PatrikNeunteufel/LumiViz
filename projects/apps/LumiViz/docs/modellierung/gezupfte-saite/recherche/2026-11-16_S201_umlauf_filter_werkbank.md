> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag
> `2026-11-16_01_umlauf_filter_saite_steg` (gestellt in LumiViz S201), zwei Segmente am 2026-11-16 (19:24–19:26 bis zum
> Haltepunkt, 19:51–19:59 fortgesetzt; claude-opus-5-5, 26 + 45 Runden, regulär beendet, 0,70 + 3,00 USD).
> **Unverändert abgelegt in S201** — nur dieser Kopf ist dazugekommen. Die zwei abgerufenen PDFs liegen in der Werkbank
> unter `ergebnisse/2026-11-16_01_umlauf_filter_saite_steg/abrufe/`.
> **Seit S202:** der Auftrag liegt im Archiv der Werkbank (`archiv/auftraege/`, `archiv/ergebnisse/`, `archiv/laeufe/`);
> die zwei PDFs liegen nur noch unter `…\cmake\quellen\gezupfte-saite\` (im Archiv entfernt) (`Bank_Valimaki_2003_RobustLossFilter_SPL10.pdf`,
> `Bank_Karjalainen_2010_PassiveAdmittanceMatrix_DAFx10.pdf`).
>
> **Gegengelesen in S201 (LumiViz-Sitzung, an den Seitenbildern):**
> - **Bank & Välimäki 2003, S. 19** — Gl. (1), Gl. (2), f′(g_k), das frühere Gewicht 1/(1 − g_k), Glättung
>   [0,25 0,5 0,25], Mindestphase über Hilbert, Beispiel F♯2 (92,2 Hz, 64 Teiltöne bis 6,57 kHz, 22,05 kHz, Ordnung
>   2 / 8 / 16 mit `invfreqz`), „never exceeded unity“: **stimmt alles mit dem Bericht überein.**
> - **Bank & Karjalainen 2010, DAFX-2 und DAFX-4** — Gl. (1)–(6), (13)–(17), Aufwand RK(K + 3), die Warnung zu
>   Finite-Differenzen: **stimmt überein.** (DAFX-3 und DAFX-5 nicht angesehen.)
> - **Välimäki u. a. 2004, S. 937** — Gl. (1)–(3), Stabilität g + |r| < 1, Länge der Schleife, Satz zum Bend:
>   **stimmt überein.** (Tab. 1 S. 944 und Ducceschi 2026 S. 325 nicht noch einmal angesehen.)
> - **Die zwei Entwurfsformeln, die nur als Code vom Abrufwerkzeug zitiert sind** (Abschnitt 1a, 1b), nachgerechnet an
>   der idealen steifen Saite (`proben/2026-11-16_S201_dispersion_formeln_gegenprobe.py`, Rohausgabe daneben):
>   - `adf.m` (Abel, Välimäki & Smith): die Beispiele 2 und 3 aus dem Kopf des Codes geben mit **4 Biquads 0,4 Cent** an
>     den Teiltönen 1–12 und 5,9 Cent über das ganze Band (16 gleiche Allpässe 1. Ordnung, angepasst: 3,5–4,3 und
>     10,9 Cent) — die Abschrift trägt dort. **Beispiel 1 (32,7 Hz, 21 Biquads) gibt 12,5 / 248 Cent: nicht bestätigt**
>     — Abschrift oder meine Auswertung (Verzug bei hoher Ordnung); am Originalcode prüfen. Bei kleinem Produkt aus
>     B und Band liefert die Formel **keinen** Biquad (G-Saite der Gitarre am 12. Bund, Band 5 kHz).
>   - Rauhala & Välimäki (Faust): der Wert a₁ der Formel liegt bei klavierähnlichen Fällen nahe am angepassten
>     (−0,917 / −0,811 / −0,687 gegen −0,932 / −0,826 / −0,685), Teiltöne 1–12 auf 2,7–5 Cent; an der G-Saite der
>     Gitarre am 12. Bund (B 5 · 10⁻⁴, 392 Hz) 19 Cent gegen 3,3 Cent mit angepasstem Wert — außerhalb des Bereichs, an
>     den die Konstanten angepasst sind, wie der Bericht vermutet.
> - **Nicht gegengelesen:** alles mit den Marken `[Werkzeug]` und `[nur Zusammenfassung]` in Abschnitt 3 (gebrochene
>   Verzögerung) und 5; die Originalpapiere zu 1a / 1b waren im Lauf nicht erreichbar (DAFx-Archiv).

# Bericht — Umlauf-Filter und Stegkopplung der gezupften Saite im Zeitbereich

- **Auftrag:** `auftraege/2026-11-16_01_umlauf_filter_saite_steg.md` (LumiViz S201, Vorbereitung 34k-6/34k-7)
- **Datum:** 2026-11-16 · **Modell:** claude-opus-5-5 · zwei Läufe (Haltepunkt dazwischen; Notiz `zwischenstand.md`)
- **Reichweite:** 14 Websuchen (standard), 21 WebFetch-Aufrufe (davon 7 ohne Inhalt: 2× DAFx-Archiv `dafx.de`
  „ECONNREFUSED“, 2× HTTP 404 uvic.ca, 2× HTTP 403 `core.ac.uk`/`cris.unibo.it`, 1× Login-Umleitung `site.unibo.it`,
  1× Datei > 10 MB `era.ed.ac.uk`); 2 PDFs als Seitenbild gelesen, die das Abrufwerkzeug selbst im Sitzungsordner
  abgelegt hat (Bank & Välimäki 2003, Bank & Karjalainen 2010); 4 lokale PDFs durchsucht, davon 3 am Seitenbild
  gelesen. **Nicht erreicht:** DAFx-Archiv insgesamt (Rauhala & Välimäki DAFx-06, Abel & Smith DAFx-06, Bilbao u. a.
  DAFx-24), Laakso u. a. 1996, Van Duyne & Smith 1994, Rocchesso & Scalcon 1996, Bank 2000, Pakarinen u. a. 2005,
  Maestre/Scavone/Smith (nur Fundstellen), Välimäki u. a. 2006 (Rep. Prog. Phys.).
- **Lokale Fundstellen** (aus `C:\Users\patri\source\repos\Visuals_Project\cmake\quellen\gezupfte-saite\`) sind mit
  **(lokal)** markiert. Alles andere stammt aus dem Netz.

Der Auftrag ist klar. Abgearbeitet in der verlangten Reihenfolge; Punkt 1 und 3 nur teilweise mit Seitenbild belegt
(die Original-PDFs waren nicht erreichbar), Punkt 2 und 4 mit Seitenbild, Punkt 5 nur aus der lokalen Quelle.

Belegmarken: `[wörtlich, Seitenbild geprüft]` · `[wörtlich, nur Textebene]` · `[Werkzeug]` (Wortlaut oder Codezeile,
die nur das Abrufwerkzeug zitiert hat — eine Zeile Code gilt als Feldinhalt, ist aber nicht am Original geprüft) ·
`[nur Zusammenfassung]` · `[eigene Einschätzung]`.

---

## 1. Dispersionsfilter für steife Saiten aus B und f0

### 1a Rauhala & Välimäki 2006 — „tunable dispersion filter“, Kaskade gleicher Allpässe 1. Ordnung

Quelle des Wortlauts: **nicht** das Papier (DAFx-06, S. 71–76, und IEEE SPL 13(5):253–256 nicht erreicht), sondern
die Faust-Umsetzung `piano_dispersion_filter(M,B,f0)` in `effect.lib` (Faust 0.9.47, STK-4.3-Lizenz), die J. O.
Smith nach eigenem Wortlaut aus diesem Papier übernommen hat; Abruf
`https://scm.cri.minesparis.psl.eu/git/Faustine.git/blob_plain/HEAD:/interpreter/preprocessor/faust-0.9.47mr3/architecture/effect.lib`.
Alle Zeilen `[Werkzeug]`:

```
piano_dispersion_filter(M,B,f0) = -Df0*M,seq(i,M,tf1(a1,1,a1))
Bc = max(B,0.000001);
k1 = -0.00179;  k2 = -0.0233;  k3 = -2.93;
m1 = 0.0126;    m2 = 0.0606;   m3 = -0.00825;  m4 = 1.97;
kd = exp(k1*log(Bc)*log(Bc) + k2*log(Bc)+k3);
Cd = exp((m1*log(M)+m2)*log(Bc)+m3*log(M)+m4);
D = exp(Cd - Ikey(f0)*kd);
a1 = (1-D)/(1+D); // By Eq. 3, have D >= 0, hence a1 >= 0 also
Df0 = polydel(a1) - polydel(1.0/a1);
polydel(a) = atan(sin(wT)/(a+cos(wT)))/wT;
wT = 2*PI*f0/SR;
Ikey(f0) = logb(trt,f0*trt/27.5);
logb(b,x) = log(x) / log(b); // log-base-b of x
//   NOTE: An erratum in Eq. (7) is corrected in Dr. Rauhala's
```

- Lesart: M gleiche Stufen (a1 + z⁻¹)/(1 + a1 z⁻¹) (Faust `tf1(b0,b1,a1)`); `trt` = 2^(1/12) laut Werkzeug, also
  Ikey = Tastennummer ab A0 (27,5 Hz) **+ 1**. Der erste Ausgang −Df0·M ist laut Werkzeug „meant to be added to a
  delay-line length“ — das ist die **Stimmung des Grundtons**: die Kette verzögert bei f0 um M·Df0 Takte, um die der
  Verzug kürzer wird `[Werkzeug]`.
- Die Konstanten k1…m4 sind eine **Anpassung** der Autoren (an Klaviersaiten, Tastenbereich und Abtastrate des
  Papiers) — Abtastrate, Bereich von B und M, Fehlermaß (Cent je Teilton) stehen im Papier, **nicht gefunden** `[eigene
  Einschätzung]`. Die Formel hängt von f0 nur über die Klaviertaste ab; für fs ≠ 44,1 kHz ist unbelegt, ob sie gilt.
- Der Verweis auf ein Erratum in Gl. (7) (in Rauhalas Dissertation berichtigt) heißt: das Papier allein reicht nicht.
- JOS PASP, „Dispersion Filter Design I“: Schwelle der Hörbarkeit B_thresh = exp[2,54 log(f0) − 24,6]; f_n = n f0
  √(1 + B n²) `[Werkzeug]` (https://ccrma.stanford.edu/~jos/pasp/Dispersion_Filter_Design_I.html, ohne Gl.-Nummern).
  Rocchesso & Scalcon: „it is highly cost-effective to obtain an allpass filter which correctly tunes only the
  lowest-frequency partial“ `[Werkzeug]`.

### 1b Abel, Välimäki & Smith 2010 — Biquad-Kaskade aus der Soll-Gruppenlaufzeit

Quelle: IEEE SPL 17(4):406–409, DOI 10.1109/LSP.2010.2040924 — **Papier nicht gelesen**. Wortlaut aus dem
MATLAB-Code `adf.m` auf https://ccrma.stanford.edu/wiki/Dispersion_Filter_Design (Original:
`http://www.acoustics.hut.fi/publications/papers/spl-adf/adf.m`), alle Zeilen `[Werkzeug]`:

```
function sos = adf(f0,B,df,beta)
% f0 = fundamental frequency, Hz;  B = inharmonicity coefficent;  df = design bandwidth, Hz
% beta = smoothing factor (e.g., beta = 0.85)
fs = 44100;
tau0 = fs/f0;
pd0 = tau0/sqrt(1 + B*(df/f0).^2);
mu0 = pd0/(1 + B*(df/f0)^2);
phi0 = 2*pi*(df/fs)*pd0 - mu0*2*pi*df/fs;
nap = floor(phi0/(2*pi));
phik = pi*[1:2:2*nap-1];
etan = [0:nap-1]/(1.2*nap) * df;
pdn = tau0./sqrt(1 + B*(etan/f0).^2);
taun = pdn./(1 + B*(etan/f0).^2);
phin = 2*pi*(etan/fs).*pdn;
theta = fs/(2*pi) * (phik - phin + (2*pi/fs)*etan.*taun) ./ (taun - mu0);
delta = diff([-theta(1) theta])/2;
cc = cos(theta * 2*pi/fs);
eta = (1 - beta.*cos(delta*2*pi/fs))./(1 - beta);
alpha = sqrt(eta.^2 - 1) - eta;
temp = [ones(nap,1) 2*alpha'.*cc' alpha'.^2];
sos = [fliplr(temp) temp];
```

- **Geschlossen, kein Optimierer:** die Ordnung (nap Biquads) folgt aus B, f0, df, fs; die Polwinkel aus einem
  Newton-Schritt, die Polradien aus beta. Beispiele im Kopf: `adf(32.702, 0.0002, 2100, 0.85)`, `adf(65.406, 0.0001,
  2500, 0.85)`, `adf(130.81, 0.00015, 4300, 0.85)` `[Werkzeug]`.
- Laut Wiki-Zusammenfassung: Pole dort, wo die Fläche der Gruppenlaufzeit ungerade Vielfache von π erreicht; Polfrequenzen
  sind Wurzeln von Quartiken, mit Näherung; „outperform a previous closed-form design“; Bandbreite wächst mit der
  Ordnung `[nur Zusammenfassung]`. **Fehlermaße in Cent, Gültigkeitsbereich: nicht gefunden.**
- Die Code-Zeilen enthalten **keine** Berechnung des Verzugs, den die Kette dem Grundton nimmt (gefragt: Werkzeug
  „No line computes the filter's final delay or corrects a loop length“) `[Werkzeug]`. Aus dem Ansatz (Soll ist die
  Gruppenlaufzeit relativ zu mu0) folgt, dass der Rest-Verzug der Schleife gesondert bestimmt werden muss `[eigene
  Einschätzung]`.

### 1c Välimäki u. a. 2004 (lokal) — Cembalo

`Valimaki_etal_2004_HarpsichordSynthesis_EURASIP.pdf`, S. 937–938 `[wörtlich, Seitenbild geprüft]`:
- Schleife: Verzug, gebrochener Allpass F(z), Dispersions-Allpass A_d(z), Verlust; „The total delay line length L in
  the feedback loop is 1+R+L1 plus the phase delay caused by the fractional delay filter F(z) and the allpass filter
  A_d(z).“ (S. 937) — so wird die Kette in die Stimmung eingerechnet.
- Gemessenes B: bei den 19 tiefsten Tasten „about 10^−5“, hörbar; „important to implement the inharmonicity for the
  lowest 2 octaves or so“ (S. 938). Keine Entwurfsformel für A_d im Text (Verweise [16, 21, 22, 23]).
- Kosten: Tab. 1, S. 944: Dispersions-Allpass 1. Ordnung 2 Mult. / 2 Add.; S. 943 (nur Textebene): „In practice, the
  lowest tones require a higher-“ [order allpass].

### 1d Van Duyne & Smith 1994, Rocchesso & Scalcon 1996, Bank 2000, Abel & Smith 2006

**Nicht gelesen.** Abel & Smith DAFx-06 „Robust Design of Very High-Order Allpass Dispersion Filters“ und Rauhala &
Välimäki DAFx-06 liegen im DAFx-Archiv, das in beiden Läufen die Verbindung verweigerte (s. Ladeliste). Bank 2000
(Diplomarbeit „Physics-Based Sound Synthesis of the Piano“, Rep. 54 HUT) ist auf Banks Seite verlinkt
(`home.mit.bme.hu/~bank/thesis/`) `[Werkzeug]`.

---

## 2. Verlustfilter aus vorgegebenem Abklingen je Teilton

### 2a Bank & Välimäki 2003, „Robust Loss Filter Design for Digital Waveguide Synthesis of String Tones“

IEEE SPL 10(1):18–20, DOI 10.1109/LSP.2002.806707; PDF von Banks Seite (`home.mit.bme.hu/~bank/publist/spl03.pdf`),
**alle Angaben `[wörtlich, Seitenbild geprüft]`**:

- S. 18: Schleife 1/(1 − z^−L H(z)); H(z) = H_l(z) H_d(z) H_fd(z) (Verlust, Dispersion, gebrochene Verzögerung).
  Sollwerte: g_k = e^(−1/(f0 τ_k)), τ_k Zeitkonstante des k-ten Teiltons; Vorgabe |H_l(e^{jϑ_k})| = g_k; Umkehrung
  τ_k = −1/(f0 ln g_k).
- S. 19: Fehler e_τ = Σ_k (τ̂_k − τ_k)²; Taylor 1. Ordnung →
  **Gl. (1)** e_τ ≈ Σ_k (f′(g_k)(ĝ_k − g_k))² = Σ_k w_k (ĝ_k − g_k)², w_k = (f′(g_k))²;
  f′(g_k) = 1/(f0 g_k (ln g_k)²); für g_k = 1 − ε: f′ ≈ 1/(f0 (g_k − 1)²);
  **Gl. (2)** w_k = 1/(g_k² (ln g_k)⁴) ≈ 1/(g_k − 1)⁴.
  Früher: „ad hoc weighting function w_k = 1/(1 − g_k) for the one-pole loop filter“ ([3] = Välimäki u. a. 1996 JAES).
- Glättung: τ_k mit Dreieck [0,25 0,5 0,25] falten, „Therefore, it is worthwhile to smooth the decay time data … This
  way, the condition ĝ_k ≈ g_k can be assured.“
- Phase (§V): Mindestphase empfohlen, Phase aus dem Betrag per Hilbert-Transformation; fehlende Hochtonwerte aus einem
  Ein-Pol-Entwurf, die letzten fünf Werte linear überblendet; „a nonlinear phase response corresponds to a slightly
  inharmonic tone, which does not corrupt the sound quality.“
- Beispiel (§VI): Klavier F♯2 (f0 = 92,2 Hz), 64 Teiltöne bis 6,57 kHz, fs = 22,05 kHz; IIR der Ordnung 2, 8, 16 mit
  MATLAB `invfreqz` (gewichtete kleinste Quadrate). Fehlermaß nur als Abb. 1 (Betrag, Abklingzeit), **keine Zahl**.
- **Passivität:** **nicht garantiert**, nur beobachtet: „The magnitude response of the designed filters never exceeded
  unity, i.e., the digital waveguide loop remained always stable.“ (S. 19/20). Stabilitätsbedingung im Text: Betrag
  ≤ 1 an den Teiltonfrequenzen, sonst „the digital waveguide becomes unstable“ (S. 19).
- Aufwand je Abtastwert: nicht genannt (ein IIR der Ordnung N: ≈ 2N+1 Mult. `[eigene Einschätzung]`).

### 2b Ein-Pol mit Ripple — Välimäki u. a. 2004 (lokal), Perng 2012 (lokal)

`Valimaki_etal_2004_HarpsichordSynthesis_EURASIP.pdf` `[wörtlich, Seitenbild geprüft]`:
- **Gl. (1)** H(z) = b (r + z^−R)/(1 + a z^−1); **Gl. (2)** b = g(1 + a); **Gl. (3)** R = round(r_rate L) (S. 937).
- Stabilität: „It is sufficient to require that the sum g + |r| remains below one, or |r| < 1 − g. In practice, a
  slightly larger magnitude of r still results in a stable system when r < 0“ (S. 937).
- **Gl. (4)** τ(k) = −1/(f0 ln[G(k)]), T60 = 6,9078 τ (S. 938). Beispiel Abb. 4: g = 0,995, a = −0,05, r = 0,0020,
  r_rate = 0,5, L = 200, f0 = 220,5 Hz.
- Aufwand Tab. 1 (S. 944): Ein-Pol 2 Mult./1 Add., Ripple 1/1; ganze Saite 13 Mult./10 Add. je Abtastwert.

`Perng_2012_HarpsichordPlectrumStringInteraction_Diss.pdf`, S. 121, Gl. (5.1) H(z) = g (1 + a1)/(1 + a1 z^−1) (Jaffe &
Smith, Välimäki u. a.) `[wörtlich, Seitenbild geprüft]` — ohne Entwurfsformel für g, a1.

### 2c Nicht gefunden
Geschlossene Ein-Pol-Formel (g, a aus T60 bei zwei Frequenzen) im Wortlaut einer Quelle; Erkut; Lee/Smith/Välimäki.
Faust `physmodels.lib` enthält nur `rho = pow(0.001,1.0/(freq*t60))` (Betrag je Umlauf aus T60) `[Werkzeug]`.

---

## 3. Gebrochene, zeitveränderliche Verzögerung (Bend, Slide, Vibrato)

Laakso u. a. 1996 (IEEE SPM 13(1):30–60, DOI 10.1109/79.482137) **nicht erreicht**; Wortlaut nur aus JOS PASP / CMJ
1996 (HTML), alles `[Werkzeug]`:

- Thiran-Allpass N-ter Ordnung: a_k = (−1)^k (N über k) ∏_{n=0}^{N} (Δ−N+n)/(Δ−N+k+n), k = 0…N; „maximally flat
  group-delay at dc“; „For sufficiently large Δ, stability is guaranteed“; „Rule of thumb: Δ ≈ order“
  (https://ccrma.stanford.edu/~jos/pasp/Thiran_Allpass_Interpolators.html). Exakte Stabilitätsgrenze dort nicht.
- Allpass 1. Ordnung: y(n) = η x(n) + x(n−1) − η y(n−1), H(z) = (η + z⁻¹)/(1 + η z⁻¹), „η ≈ (1−Δ)/(1+Δ)“;
  empfohlen Δ ∈ [0,1; 1,1] ↔ η ∈ [−0,05; 0,82]; „ramping the coefficients of the allpass gradually "grows" or "hides"
  one sample of delay. This tells us how to handle resets when crossing sample boundaries.“
  (…/First_Order_Allpass_Interpolation.html)
- Ausschwingen: lang, wenn der Pol bei −η nahe am Einheitskreis liegt; τ/T ≈ 1/(1−R), 60 dB ≈ 7 Zeitkonstanten;
  Beispiel T60 ≤ 100 ms bei fs = 10 kHz → η ≤ 0,993 (…/Minimizing_First_Order_Allpass_Transient.html).
- Umschalten: „instantly switching from one delay to another“ gibt „a transient artifact due to the recursive nature
  of the allpass filter“; Abhilfe: Välimäkis „general transient elimination scheme for recursive digital filters of
  arbitrary order“. Lagrange (FIR): „the switching problem does not arise since the interpolating filter is
  nonrecursive“, aber „a time-varying amplitude distortion at high frequencies“
  (https://ccrma.stanford.edu/~jos/cmj96/Delay_Line_Interpolation.html).
- Lokal, Välimäki u. a. 2004, S. 937 `[wörtlich, Seitenbild geprüft]`: „the recursive nature of the allpass
  fractional delay filter, which can cause transients during pitch bends, is not harmful“ — weil das Cembalo **keinen**
  Bend braucht.
- **Energie bei veränderlicher Länge:** Pakarinen, Karjalainen, Välimäki, Bilbao, ICASSP 2005 „Energy behavior in
  time-varying fractional delay filters …“ — zwei Verfahren: „sliding termination point simulation with energy
  correction“ und „controllable wave digital filter delay lines“; Beispiele Glissando, Vibrato `[nur Zusammenfassung]`
  (Volltext `core.ac.uk/download/28975045.pdf` gab 403). Rocchesso 2000, „Fractionally-addressed delay lines“, IEEE
  TSAP 8(6):717–727, frei als arXiv cs/0005022: ein gebrochener Zeiger für Lesen und Schreiben, „physical
  interpretation“ für Spannungs-/Dichtemodulation `[Werkzeug]` (Abstract), Inhalt nicht gelesen.
- **Nicht gefunden:** Tonhöhenfehler je Ordnung und Frequenz in Cent; Van Duyne/Jaffe/Scandalis/Stilson 1997 (ICMC,
  „lossless, click-free, pitchbend-able …“) — nur ein Stanford-Patent mit zwei Allpässen und getrennten Lesezeigern
  als Spur `[nur Zusammenfassung]`; Pakarinen u. a. Slide-Gitarre.

---

## 4. Kopplung Wellenleiter – Modalbank am Steg, zwei Polarisationen, passiv

### 4a Bank & Karjalainen 2010, „Passive Admittance Matrix Modeling for Guitar Synthesis“, DAFx-10, S. 3–7

PDF von Banks Seite (`home.mit.bme.hu/~bank/publist/dafx10adm.pdf`), **alle Angaben `[wörtlich, Seitenbild geprüft]`**
(DAFX-2 bis DAFX-5):

- **Gl. (1)** Re{H(jω)} = ½(H(jω) + H*(jω)) ≥ 0 (positiv reell, PR, für asymptotisch stabile Systeme);
  **Gl. (2)** dasselbe auf dem Einheitskreis für H(e^{−jϑ}).
- **Gl. (3)** v = Y f; **Gl. (4)** Y(jω) = Σ_r Φ_rᵀ Φ_r · jω / (m_r(ω_r² − ω² + 2jσ_r ω_r ω)) — Real-Teil positiv
  semidefinit, „since … a linear combination of real positive semidefinite matrices Φ_rᵀΦ_r with positive weights“.
- **Gl. (5a)** Y(z) = Σ_{r=1}^{R} Y_r H_r(z); **(5b)** H_r(z) = (1 − z⁻²)/((1 − p_r z⁻¹)(1 − p_r* z⁻¹)),
  „the bilinearly transformed discrete-time versions of the second-order functions of Eq. (4). If a positive real
  function H(s) is converted to a discrete-time function H(z) by the bilinear transform, it remains positive real“.
  **Gl. (6)**: hinreichend für Passivität: alle Y_r positiv semidefinit.
- Anpassung (§3.1): gemeinsamer Nenner per AR-Modell im verzerrten Bereich (Gl. 7–9, p_r = (p̃_r + λ)/(1 + λ p̃_r)),
  dann Y_r per kleinster Quadrate (Gl. 10–11) und Projektion auf die nächste positiv semidefinite Matrix (negative
  Eigenwerte verwerfen).
- **Aufwand** (S. DAFX-3/4): RK Biquads (2RK Mult.), R Matrizen (RK² Mult.), Summe — „Roughly, the required
  computational power is RK(K + 3) multiply and accumulate instructions per sample.“ (K = Zahl der Richtungen/Punkte.)
- **Lösung in einem Schritt mit Durchgriff** (§4.1): **Gl. (13)** v⁻ = H_v(z) v⁺ = (Y(z) + Y0)⁻¹ (Y(z) − Y0) v⁺;
  **Gl. (14)** Y(z) = Y_i + z⁻¹ Y_p(z) (Y_i = erster Wert der Impulsantwort); **Gl. (15)** H_r(z) = 1 + z⁻¹
  (b_{r,1} + b_{r,2} z⁻¹)/(1 + a_{r,1} z⁻¹ + a_{r,2} z⁻²) mit b_{r,1} = −a_{r,1}, b_{r,2} = −1 − a_{r,2};
  **Gl. (16)** Y_p = Σ Y_r H_{p,r}, Y_i = Σ Y_r; **Gl. (17)** v⁻ = (Y_i + Y0)⁻¹ [z⁻¹ Y_p(z)(v⁺ − v⁻) + (Y_i − Y0) v⁺].
  „The non-computable delay-free loop is avoided because of the decomposition … only a matrix with constant elements
  (Y_i + Y0) has to be inverted.“ Y0 für eine lineare Saite diagonal (keine innere Kopplung der Polarisationen).
- Warnung (§4): passive Admittanz als Abschluss eines **Finite-Differenzen**-Modells „often result[s] in unstable
  systems, unless special measures are taken to ensure numerical energy conservation“; im Wellenformat
  „automatically avoided“.
- Beispiel (§5): Gibson-Gitarre, 2×2-Admittanz (y senkrecht, z parallel zur Decke), Drahtbruch-Anregung; R = 100
  „an overkill“; R = 30 „a good compromise between sound quality and computational efficiency“; Saite E (f0 = 82 Hz)
  mit Ein-Pol-Verlust; Schwebung und zweistufiges Abklingen bei Teiltönen nahe großer Admittanz. **Kein Zahlenmaß** für
  Fehler in Frequenz, Güte, Höhe — nur Abb. 3/5.

`[eigene Einschätzung]`: Der Stegschritt des LumiViz-Kandidaten (`…/proben/2026-11-16_S200_zeitbereich_kandidat.py`,
Docstring: „u = (I + z0 M)^-1 (u_frei + 2 z0 M v+)“) hat dieselbe Gestalt wie Gl. (17): Durchgriff abspalten, konstante
Matrix invertieren. Bank & Karjalainen belegen Passivität **nur für die bilineare Abbildung** jeder Mode (Gl. 5b) und
nicht-negative Gewichte; für „trapez“ (Impulsinvarianz mit halbem Durchgriff) und „foh“ sagen sie nichts.

### 4b Ducceschi, Russo & Webb 2026 (lokal) — Stegmoden in der Zustandsform

`Ducceschi_Russo_Webb_2026_65ModelledGuitars_DAFx26.pdf`, S. 325 `[wörtlich, Seitenbild geprüft]`:
**Gl. (14)** H_b(ω) = Σ_{j=1}^{J} β_j / ((ω_j^b)² − ω² + 2iσ_j^b ω) (Nachgiebigkeit am Stegpunkt, gemessen);
**Gl. (15)** m_j^b(r̈_j + 2σ_j^b ṙ_j + (ω_j^b)² r_j) = −β_j F_b; **Gl. (16)** u(x_b,t) = u_b(t), F_b implizit aus der
Bedingung; **Gl. (21–24)** gemeinsamer Zustand y = [q; r], F_b über T = I − f(vᵀf)⁻¹vᵀ; **Gl. (26)** Energie ≥ 0
„ensuring numerical stability in the unforced case“. S. 326 (nur Textebene): „Modes satisfying ω_j < 0.2σ_j are
discarded as overdamped.“ Eine Polarisation (senkrecht).

### 4c Nicht gefunden
Fehlervergleich impulsinvariant / bilinear / first-order hold für breite Moden über 5 kHz in Frequenz, Güte, Höhe — in
keiner gelesenen Quelle. Karjalainen/Välimäki/Tolonen 1998 (CMJ) nicht gesucht. Maestre/Scavone/Smith: nur Fundstellen
(WASPAA 2015 „Digital modeling of string instrument bridge reflectance and body radiativity …“, UPF-Repositorium, Open
Access; SMAC 2013 „Digital modeling of bridge driving-point admittance …“, Moden-Amplituden per semidefiniter
Programmierung „while enforcing passivity“, CC BY 3.0) `[nur Zusammenfassung]`.

---

## 5. Alternative ohne Wellenleiter

Nur lokal: Ducceschi, Russo & Webb 2026 `[wörtlich, Seitenbild geprüft]`, S. 325 und 329:
- Modale Saite mit Steifheit: **Gl. (18)** ω_m^s = √(T0/µ (mπ/L)² + EI/µ (mπ/L)⁴) — Steifheit exakt je Mode, kein
  Filterentwurf. Verlust **Gl. (25)** c_m = σ_{0,s} + (η_{f,s}/2) ω_m, σ_{0,s} = 6 ln10 / T60,s (Woodhouse).
- Tab. 2: fs 48 kHz, Moden bis 10 kHz („Modal freq. limit“), M ≈ 200 bei E2, nichtlineare Kopplung nur bis 5 kHz
  (M_nl ≈ 60), Mitschwingen bis 5 kHz; Steg 50–200 Moden, Abstrahlung 80–300 Pole je Seite.
- Aufwand: zwei Sherman–Morrison-Schritte, O(N) je Zeitschritt (S. 328/329). „In MATLAB, a single 5 s note at 48 kHz
  requires 0.1–0.5 s depending on the mode count; the dominant cost is the radiation filter bank.“ — also 2–10 % der
  Echtzeit in MATLAB, **einschließlich** Abstrahlung, Rechner nicht genannt („a standard workstation“). Echtzeit-C++
  erst „future directions“.
- Bend/Slide (zeitveränderliche Länge): **nicht behandelt** (Saitenlänge fest, Bünde f = 0…15 als getrennte Töne).
- **Nicht erreicht:** Bilbao, Russo, Webb, Ducceschi, DAFx-24 „Real-Time Guitar Synthesis“ (Zeitmessungen laut
  Zusammenfassung vorhanden), Russo/Ducceschi/Bilbao DAFx-22 „Efficient Simulation of the Bowed String in Modal Form“
  („faster than real-time“ `[nur Zusammenfassung]`), Ducceschi DAFx-23 (Unibo-Kopien hinter Login, DAFx-Archiv nicht
  erreichbar).

---

## Tabelle: Verfahren × Genauigkeit, Aufwand, Passivität, zeitveränderlich

| Verfahren | Genauigkeit (Autoren) | Aufwand je Abtastwert | Passivität | zeitveränderlich | Stelle |
|---|---|---|---|---|---|
| Dispersion: M gleiche Allpässe 1. Ord. (Rauhala & Välimäki 2006) | nicht gefunden | M × 2 Mult. (Tab. 1 Välimäki 2004 je Stufe: 2/2) | Allpass, verlustfrei | Koeffizient geschlossen aus B, f0 → umstellbar; Transienten wie 1c | Faust `effect.lib` [Werkzeug]; Välimäki 2004 Tab. 1 (lokal) |
| Dispersion: nap Biquads (Abel, Välimäki & Smith 2010) | „outperform a previous closed-form design“, keine Zahl | nap Biquads, nap aus B, f0, df, fs | Allpass, Pole im Kreis (Radius aus beta) | geschlossen, aber nap ändert sich mit f0 | `adf.m` [Werkzeug] |
| Verlust Ein-Pol + Ripple (Välimäki 2004) | Abb. 4 nur | 3 Mult. (2 + 1) | g + \|r\| < 1 hinreichend | — | S. 937, Tab. 1 (lokal) [wörtlich, Bild] |
| Verlust IIR Ord. 2/8/16, gewichtet (Bank & Välimäki 2003) | nur Abb. 1 | ≈ 2N+1 Mult. [eigene Einschätzung] | **nicht garantiert**, „never exceeded unity“ beobachtet | Entwurf mit `invfreqz` (Optimierer) | S. 18–20 [wörtlich, Bild] |
| Gebrochene Verzögerung Allpass 1. Ord. | Δ ∈ [0,1; 1,1] empfohlen | 2 Mult. | verlustfrei | Transient beim Umstellen; Abhilfe Välimäki | JOS PASP [Werkzeug] |
| Gebrochene Verzögerung Lagrange (FIR) | Betragsfehler bei hohen Frequenzen | N+1 Mult. | Betrag ≤ 1 nur bei kleiner Ordnung (nicht belegt) | kein Umschalt-Transient | JOS CMJ96 [Werkzeug] |
| Stegadmittanz-Matrix, R Moden, K Richtungen (Bank & Karjalainen 2010) | R = 30 „good compromise“, kein Zahlenmaß | ≈ RK(K+3) MAC | PR garantiert: bilinear + Y_r ≥ 0 | — | DAFx-10 S. 2–5 [wörtlich, Bild] |
| Modale Saite + Stegmoden, SAV (Ducceschi u. a. 2026) | — | O(N), N = M + J; MATLAB 0,1–0,5 s je 5-s-Ton | Energie ≥ 0 (Gl. 26) | nicht behandelt | DAFx26 S. 325, 329 (lokal) [wörtlich, Bild] |

---

## Reicht das für einen Entwurf je Taste ohne Optimierer zur Laufzeit?

- **Geschlossen aus B, f0, fs belegt:** (1) Dispersion als M gleiche Allpässe 1. Ordnung nach Rauhala & Välimäki —
  Koeffizient a1 aus B, f0 (Taste) und M, Phasenverzug bei f0 für die Stimmung (Df0·M) `[Werkzeug]`; Konstanten an
  Klaviersaiten angepasst, Gültigkeitsbereich ungeklärt. (2) Biquad-Kaskade nach Abel, Välimäki & Smith — Ordnung,
  Polwinkel, Radien geschlossen aus f0, B, df, beta, fs `[Werkzeug]`; Rest-Verzug für die Stimmung muss eigens aus der
  Phase der Kaskade bei f0 gerechnet werden (keine Zeile dazu im Code). (3) Verlust je Umlauf aus T60 je Teilton:
  g_k = e^(−1/(f0 τ_k)) und Gewicht w_k = 1/(g_k − 1)⁴ — **die Vorgabe** ist geschlossen `[wörtlich]`.
  (4) Gebrochene Verzögerung Allpass 1. Ordnung η ≈ (1−Δ)/(1+Δ) `[Werkzeug]`; Thiran-Koeffizienten geschlossen.
  (5) Stegkopplung: wenn die Moden aus der Körperdatei kommen (GZ7 a), sind die Modalbank und der Durchgriff Y_i
  geschlossen (Gl. 14–17); Passivität für die bilineare Abbildung belegt.
- **Braucht eine Anpassung:** der Verlustfilter höherer Ordnung (Bank & Välimäki: gewichtete kleinste Quadrate,
  `invfreqz`, Glättung, Hilbert-Phase) — ein Entwurf je Taste, kein Laufzeit-Optimierer, aber nicht geschlossen und
  ohne garantierte Passivität. Der Ein-Pol (2 Werte an 2 Frequenzen) bzw. Ein-Pol + Ripple ist geschlossen lösbar,
  trägt aber nur einen glatten Verlauf. Die Konstanten der Rauhala-Formel sind selbst eine Anpassung an das Klavier.
- **Bliebe Annahme:** Gültigkeit der Rauhala-Konstanten für Darm-, Nylon- und Stahlsaiten der Harfe/Gitarre bei 48 kHz;
  Fehler der Allpass-Dispersion in Cent je Teilton; Passivität der „foh“-Diskretisierung (nur bilinear belegt);
  klickfreies Umstellen des gebrochenen Allpasses bei Bend/Slide (nur Hinweis auf Välimäkis Verfahren); Energiefehler
  bei veränderlicher Länge (Pakarinen 2005 nicht gelesen).

`[eigene Einschätzung]` Für die Fragen, die LumiViz schon im Prüfstand misst (Cent je Teilton, Abklingen), geben die
gefundenen Quellen **keine Fehlerzahlen**, an denen sich eine Fassung messen ließe — die Prüfung bleibt am Prüfstand.

---

## Nicht gefunden (mit Suchwegen)

| Lücke | wo gesucht |
|---|---|
| Original-Wortlaut und Fehlermaße Rauhala & Välimäki 2006 | WebSearch ×3; DAFx-Archiv (ECONNREFUSED in beiden Läufen); Faust-Code statt Papier |
| Abel & Smith 2006, Abel/Välimäki/Smith 2010 Papiertext, Gültigkeitsbereich | WebSearch; CCRMA-Wiki (nur Code); Scribd-Kopie nicht abgerufen (Herkunft unklar) |
| Van Duyne & Smith 1994, Rocchesso & Scalcon 1996 | nur Erwähnung in JOS PASP und Välimäki 2004 Literatur |
| Geschlossene Ein-Pol-Formel aus T60 bei zwei Frequenzen | Välimäki 2004, Perng 2012 (lokal), Faust `physmodels.lib`; Välimäki u. a. 1996 JAES nicht gesucht |
| Laakso u. a. 1996, Tonhöhenfehler je Ordnung | WebSearch; kein freier Volltext |
| Pakarinen u. a. 2005 Energie | core.ac.uk 403 |
| Van Duyne u. a. 1997 klickfrei | WebSearch; nur Patent-Spur |
| Diskretisierung foh/impulsinvariant/bilinear, Fehler über 5 kHz | WebSearch; Bank & Karjalainen 2010 (nur bilinear) |
| Echtzeitfaktoren Modal/FD (Bilbao, Ducceschi DAFx-22/23/24) | DAFx-Archiv nicht erreichbar, Unibo hinter Login, unibo-cris 403, Edinburgh-Datei > 10 MB |
| Penttinen 2006, Perng/Smith/Rossing 2011 (lokal) | `pdf_suche.py` auf Thiran, dispersion, loss filter, bridge admittance, passiv: keine Treffer |

## Ladeliste (nichts geladen; nur Dateien, die nicht lokal liegen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Zugang | wozu |
|---|---|---|---|---|
| `Bank_Valimaki_2003_RobustLossFilter_SPL10.pdf` | https://home.mit.bme.hu/~bank/publist/spl03.pdf | 200,8 KB | Autorenseite, IEEE © 2003 | Punkt 2, Gl. (1)–(2) |
| `Bank_Karjalainen_2010_PassiveAdmittanceMatrix_DAFx10.pdf` | https://home.mit.bme.hu/~bank/publist/dafx10adm.pdf | 240,1 KB | Autorenseite (DAFx-Proceedings) | Punkt 4, Gl. (1)–(17) |
| `Rauhala_Valimaki_2006_TunableDispersion_DAFx06.pdf` | über https://dafx.de/paper-archive/details/n4Qv1ScD_VbmqShsq0ZVQQ | unbekannt | DAFx-Archiv frei; im Lauf nicht erreichbar | Punkt 1, Formel + Fehlermaße + Erratum Gl. (7) |
| `Abel_Smith_2006_HighOrderAllpassDispersion_DAFx06.pdf` | über https://dafx.de/paper-archive/details/FxpIAYIVGkUKoqVYzYCHQg | unbekannt | DAFx-Archiv frei; nicht erreichbar | Punkt 1 |
| `Abel_Valimaki_Smith_2010_AllpassDispersion_adf.m` | http://www.acoustics.hut.fi/publications/papers/spl-adf/adf.m | unbekannt | ohne Lizenzangabe | Punkt 1, Originalcode |
| `Rauhala_2007_Diss_PianoSynthesis.pdf` | nicht gesucht (Aalto) | — | — | Erratum Gl. (7) |
| `Bank_2000_PianoSynthesis_MSc.pdf` | https://home.mit.bme.hu/~bank/thesis/ | unbekannt | Autorenseite | Punkt 1/2 (Ein-Pol-Entwurf S. 87–91 laut Bank & Välimäki 2003 Ref. [7]) |
| `Pakarinen_etal_2005_EnergyTimeVaryingFD_ICASSP.pdf` | https://core.ac.uk/download/28975045.pdf | unbekannt | Edinburgh Research Explorer; 403 im Lauf | Punkt 3 Energie |
| `Rocchesso_2000_FractionallyAddressedDelayLines_TSAP.pdf` | https://arxiv.org/pdf/cs/0005022 | unbekannt | arXiv | Punkt 3 |
| `Maestre_Scavone_Smith_2015_BridgeReflectance_WASPAA.pdf` | https://repositori.upf.edu/items/7c207d44-4ac0-4100-a216-dc814a1abe60 | unbekannt | Open Access (accepted version) | Punkt 4 |
| `Maestre_etal_2013_BridgeAdmittance_SMAC.pdf` | https://repositori.upf.edu/handle/10230/23667 | unbekannt | CC BY 3.0 | Punkt 4 Passivität |
| `Bilbao_etal_2024_RealTimeGuitar_DAFx24.pdf` | https://dafx.de/paper-archive/2024/papers/DAFx24_paper_50.pdf | unbekannt | DAFx frei; nicht erreichbar | Punkt 5 Zeitmessung |
| `Bank_2010_ModalPianoSynth_TASLP18.pdf` | https://home.mit.bme.hu/~bank/publist/taslp10.pdf | unbekannt | Autorenseite | Punkt 5 (modal, Echtzeit) |

## Am Ende: nicht geliefert, und was die LumiViz-Sitzung selbst prüfen muss

- **Nicht geliefert:** Fehlermaße (Cent, Abklingfehler) und Gültigkeitsbereiche für alle Dispersions-Entwürfe;
  Tonhöhenfehler je Ordnung der gebrochenen Verzögerung; Energiefehler bei Bend/Slide; Vergleich der Diskretisierungen
  der Moden über 5 kHz; Echtzeitfaktoren modaler/FD-Saiten auf genannter Hardware.
- **Selbst nachprüfen:** (1) die Faust- und MATLAB-Zeilen sind nur vom Abrufwerkzeug zitiert — vor Übernahme an den
  Originaldateien (`effect.lib`, `adf.m`) und am Papier vergleichen, besonders das Erratum zu Rauhala Gl. (7) und die
  Bedeutung von `trt` in `Ikey`. (2) Ob die foh-Diskretisierung des Kandidaten positiv reell bleibt — Bank & Karjalainen
  belegen es nur für die bilineare Abbildung. (3) Ducceschi 2026, S. 326 „ω_j < 0.2σ_j … discarded“ ist nur aus der
  Textebene.
- Hilfsdatei dieses Laufs: `ergebnisse/2026-11-16_01_umlauf_filter_saite_steg/suche.py` (vor den Helfern unter
  `werkzeuge/` geschrieben, kann entfallen); `zwischenstand.md` ist die Notiz zum Haltepunkt.

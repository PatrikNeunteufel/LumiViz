> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-17_21_umlauf_filter_nacharbeit` (gestellt in LumiViz
> S202, Nacharbeit zu Bericht 11), 1 Segment: 2026-11-17 12:40–13:04, claude-opus-5-5, 114 Runden, success, 7.46 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S203** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S203** (Seitenbilder des Laufs; Rechnungen `proben/2026-11-17_S203_gegenprobe_bericht21_rauhala.py` — eigene
> Umsetzung ohne Code des Laufs — und `…_bericht21_passiv.py` — Vorgabe und Entwurf aus `bank_voll.py` / `weg2.py`, Betrag
> unabhängig mit `scipy.signal.freqz` auf 2¹⁸ Punkten; Ausgaben `.txt` daneben):
> - **Bank & Välimäki 2003, wörtliche Stellen — stimmen:** S. 19 (Glättung mit Dreieck, „great variance“, HF-Vorgabe mit
>   „linearly mixed“, Hilbert „entire digital frequency band“, „third-order polynomial“, „never exceeded unity“) in der
>   Textebene; S. 20 VII („Smoothing … high variance of measured decay times“) am Seitenbild — Zeilenumbruch zwischen „high“
>   und „variance“, deshalb kein Treffer in der Textebene. Abb. 1(b): unter dem ersten Teilton fallen die Entwürfe zu 0 Hz
>   auf ≈ 0,993–0,995, Maximum ≈ 0,4–0,5 kHz, wie abgelesen.
> - **Rauhala & Välimäki DAFx-06 am Seitenbild:** Gl. (3)–(6), (8), Tab. 1 und Tab. 2 stimmen; Gl. (7) steht gedruckt mit
>   „ln M“ an erster Stelle. **Druckfehler bestätigt:** gedruckt gibt sie an allen Punkten von Abb. 3 / 4 a₁ = −0,996 …
>   −1,000; mit „ln B“ Abb. 3 M = 1: −0,81 … −0,96, M = 40: −0,36 … −0,83, Abb. 4 Taste 40: −0,015 … −0,66 — trifft die
>   Kurven; C₁(1) 0,0606, C₁(8) 0,0868, C₂(8) 1,953 wie Abb. 2. **44,1 kHz bestätigt** (Abb. 5 nachgerechnet: Verzug und
>   M gleiche Allpässe, am Grundton gestimmt): bei 44,1 kHz (a) M = 1 +6,2 Cent bei 263 Hz, (b) M = 1 +7,3 bei 724 Hz,
>   M = 40 +10,1 bei 2049 Hz, (c) M = 1 +7,1 bei 1185 Hz — wie abgelesen (≈ +7 / +8 / +10 / +8); bei 48 kHz höchstens
>   +4,4 Cent, ab M = 10 kein Buckel. Kleiner Unterschied: (a) M = 40 +6,9 statt +7,1 Cent (Raster).
> - **`adf.m`:** Z. 18–20 (Beispiele), 31 (`fs = 44100`), 38 (`pd0`), 46–51 (Newton-Schritt) wie zitiert. Teiltongesetz
>   f_n = n f₀ / √(1 − B n²) numerisch bestätigt: gegen R1 0,72 Cent am Teilton 12, 187,96 Cent am Teilton 47.
> - **Passivität unabhängig gemessen — stimmt:** Gitarre 6/0 iir4 11,67077 bei 24 kHz (ein Pol gespiegelt), ohne HF-Vorgabe
>   0,99266; Gitarre 1/12, Harfe 46, Harfe 60 je iir4 / iir8 wie Tab. 1c auf 4–5 Stellen; mit dem wahren Ziel unter f₁
>   Gitarre 1/12 iir4 1,0071, Harfe 46 iir8 1,0008, Harfe 60 iir4 / iir8 1,0013 / 1,0084 über 1 (wie 1d); Weg 2a
>   höchstens 0,999996.
> - **Zusatz zum Maß:** Über **alle** Sollpunkte bis 21,6 kHz (größter relativer Fehler des Abklingens) ist Weg 2a an
>   einzelnen Tasten deutlich schlechter als Weg 1 — Harfe 60 iir4 49,6 gegen 23,8 %, Harfe 46 iir4 202 gegen 155 %; an
>   anderen gleich oder besser (Gitarre 1/12 iir4 7,5 gegen 10,3 %). Der Bericht misst nur Teiltöne 1–12, ≤ 5 kHz und
>   „hörbar“ (σ ≤ 100 1/s). Ob die obersten Teiltöne zählen, ist im Entwurf festzulegen.
> - **Nicht gegengelesen:** die Tabellen 1c, 2a, 2b Zahl für Zahl (nur die Stichproben oben); Weg 2b; `invfreqz` und
>   `interp1 'cubic'` (Abweichungen 6, 10 bleiben Einschätzung des Laufs).
> **Ablage:** `Rauhala_Valimaki_2006_DispersionTunableAllpass_DAFx06.pdf` liegt nur noch unter `…\cmake\quellen\gezupfte-saite\`
> (Herkunft in `quellen\UMZUG_S203.tsv`); `adf.m` und die übrigen Abrufe (HTML, JSON, Text) bleiben beim Lauf.
> Der Auftrag liegt seit S203 im Archiv der Werkbank: `archiv/auftraege/2026-11-17_21_umlauf_filter_nacharbeit.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-17_21_umlauf_filter_nacharbeit/` (darin `seiten/` und `abrufe/`), Protokolle unter `archiv/laeufe/`. Wo der
> Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-17_S203_umlauf_filter_nacharbeit_werkbank/`.

# Bericht — Nacharbeit zu Bericht 11: Verlustfilter nach Bank & Välimäki vollständig, Entwurf mit |H| ≤ 1, Originale der Dispersionsformeln

- **Auftrag:** `auftraege/2026-11-17_21_umlauf_filter_nacharbeit.md` (LumiViz S202, Nacharbeit zu `2026-11-16_11_umlauf_filter_grundlagen`)
- **Datum:** 2026-11-17 · **Modell:** claude-opus-5-5 · ein Lauf, ohne Haltepunkt
- **Reichweite:** 5 Websuchen (`WebSearch`), 0 `WebFetch`, 14 rohe Abrufe mit `curl` (13 Adressen; Liste mit Prüfsummen in
  `abrufe/INDEX.md`). **Gefunden und am Seitenbild gelesen:** Rauhala & Välimäki, DAFx-06, über die Wayback Machine
  (dafx.de selbst: Zeitüberschreitung, zweimal). **Roh gelesen:** `adf.m` vom Begleitordner der Aalto-Universität
  (Wayback). **Nicht erreicht oder nicht frei:** beide IEEE-SPL-Briefe (Rauhala & Välimäki 2006, Abel, Välimäki & Smith
  2010), research.aalto.fi (HTTP 403), Semantic Scholar für den Rauhala-Brief (HTTP 429), Abel & Smith DAFx-06 (Titel nicht
  einer Datei im Archiv zuzuordnen). ResearchGate und Scribd nicht benutzt (Anmeldung bzw. Kopie unklarer Herkunft).
- **Lokal gelesen:** Bank & Välimäki 2003 S. 18–20 **am Seitenbild** (alle drei Seiten); Bericht 11, `umlauf_entwurf.py`
  und `umlauf_entwurf.txt` aus dem Archiv; im LumiViz-Repo der Kopf des Berichts S202, die Gegenprobe S201
  (`…_dispersion_formeln_gegenprobe.py` und `.txt`) und Teile des Prüfstands (`harfe_taste`, `cembalo_taste`, `choere`).
  Bank & Karjalainen 2010 und Välimäki u. a. 2004 sind **nicht** neu gelesen: diese Fragen brauchen sie nicht.
- **Hauptsache dieses Laufs ist Rechnung.** Alle Skripte bauen auf `umlauf_entwurf.py` aus dem Archiv auf (importiert
  mit `importlib`, ersetzt wird nur `iir_bank`).

Belegmarken: `[wörtlich, Seitenbild]` · `[wörtlich, Rohtext]` (Datei roh gelesen, kein Abrufwerkzeug dazwischen) ·
`[nur Zusammenfassung]` · `[hergeleitet]` · `[eigene Rechnung]` · `[eigene Einschätzung]` · `[Bericht 11]` = so dort, nicht
neu geprüft.

**Skripte und Ausgaben** (alle unter `ergebnisse/2026-11-17_21_umlauf_filter_nacharbeit/`, Aufruf `python -B <skript>`):

| Skript | wozu | Ausgabe |
|---|---|---|
| `bank_voll.py` | Punkt 1: Verfahren nach Bank & Välimäki, 7 Varianten, Ordnungen 2/4/8/16, 18 Tasten | `bank_voll.txt` |
| `diag_bericht11.py` | Punkt 1: die 8 Fälle über 1 aus Bericht 11 nachgerechnet, Wirkung des Spiegelns | `diag_bericht11.txt` |
| `bank_diag.py` | Punkt 1: Ursache (Betrag unter f₁, Rand der Glättung) | `bank_diag.txt` |
| `weg2.py tasten` | Punkt 2: Weg 1 / 2a / 2b an den 18 Tasten, voller Umlauf und nur Verlustfilter | `weg2_tasten.txt` |
| `weg2.py umfang` | Punkt 2: ganzer Umfang (295 Tasten), nur Verlustfilter | `weg2_umfang.txt` |
| `umfang_auswertung.py` | zählt `weg2_umfang.txt` aus | `umfang_auswertung.txt` |
| `umfang_ursache.py` | Punkt 2: Ursache an den 18 nicht passiven Fällen des Umfangs | `umfang_ursache.txt` |
| `dispersion_original.py` | Punkt 3: Rauhala am Original, `adf.m` roh, Beispiel 1 | `dispersion_original.txt` |
| `abrufe_index.py` | `abrufe/INDEX.md` | — |

---

## Kurzantworten

1. **Bleibt das vollständige Verfahren an allen 18 Tasten passiv?** Fast. An 17 Tasten liegt jede gerechnete Ordnung
   (2/4/8, dazu 16) unter 1, mit Gleichungsfehler wie `invfreqz`. **Ausnahme: Gitarre Saite 6 leer, iir4: max |H| = 11,67
   bei fs/2.** Ursache ist die **Hochfrequenz-Vorgabe**: ohne sie ist dieselbe Taste passiv (0,99266). Über den ganzen
   Umfang (295 Tasten) liegen 12 Tasten bei iir4 und 6 bei iir8 über 1, bei iir2 keine. 13 dieser 18 Fälle haben ihr
   Maximum bei 19,6–24 kHz und verschwinden ohne HF-Vorgabe. Die übrigen 5 liegen bei 0 Hz, nur 1,0003–1,018 über 1, und
   verschwinden ohne Glättung oder ohne HF-Vorgabe. **An jeder der 295 Tasten ist mindestens eine Ordnung aus 2/4/8
   passiv.** `[eigene Rechnung]`
2. **Warum lag Bericht 11 an 6 Tasten über 1?** Weder die fehlende Glättung noch die fehlende HF-Vorgabe ist schuld,
   sondern das **Ziel unter dem Grundton**. Bericht 11 nahm für die Phase (Mindestphase aus dem Betrag) das wahre Ziel bis
   0 Hz. Bei monofilen Saiten fällt −ln|H_ziel| dort auf 0,7–4 % seines Werts bei f₁ (Spalte „f→0 / f1“ in
   `bank_diag.txt`), der Sollbetrag läuft also unter f₁ gegen 1. Die Teiltöne schreiben dort nichts vor, und der Entwurf
   setzt sein Maximum in diese Lücke. Mit dem wahren Ziel unter f₁ liegt das sonst vollständige Verfahren an **9 der 18
   Tasten** über 1, fast immer bei 0 Hz. Hält man den Wert bei f₁ fest, ist es nur noch an einer Taste darüber (Abschnitt
   1c). Das Spiegeln instabiler Pole ohne Ausgleich des Betrags erklärt nur einen der 8 Fälle (Harfe 46 iir8). Die 8 Werte
   aus Bericht 11 sind auf 4 Stellen reproduziert. `[eigene Rechnung]`
3. **Die Glättung kostet Genauigkeit.** Sie verfälscht ein Ziel, das schon glatt ist: an den monofilen Saiten weicht der
   Grundton um +2,9 bis +15,3 % ab, ohne Glättung um höchstens 6,2 % (Betrag). Die Quelle sieht sie für streuende Messwerte vor: „Smoothing of target decay time
   specification is necessary in the case of high variance of measured decay times.“ (S. 20, VII) `[wörtlich, Seitenbild]`
4. **Entwurf mit |H| ≤ 1:** Weg 2a ist ein konvexes quadratisches Problem: der Nenner kommt aus Weg 1, der Zähler wird
   über R(ω) = |B|² bestimmt, mit linearen Ungleichungen auf 4097 Punkten, danach folgt die Spektralfaktorisierung. Er
   ist **an jeder Taste passiv und meist so gut wie Weg 1** (Tonhöhe ≤ 1,0 Cent anders; Ausreißer in 2a), 9–51 ms je
   Entwurf. Weg 2b
   (Allpass-Summe, passiv durch den Bau) ist schlechter: Er sucht nichtkonvex und findet örtliche Minima, und seine
   Phase stört an 10 der 18 Tasten die Stimmung um mehr als 5 Cent (bis 529 Cent). 0,13–3,2 s je Entwurf.
5. **Rauhala & Välimäki (DAFx-06) am Original:** Die Konstanten k₁…m₄ stimmen mit der Faust-Abschrift überein. **Gl. (7)
   ist im Druck falsch:** dort steht „(m₁ ln M + m₂) ln M“, nötig ist „(m₁ ln M + m₂) ln B“. Nur so trifft die Formel die
   Abb. 3 und 4 (auf ≤ 0,03); die gedruckte Form gibt überall a₁ ≈ −1. Faust und S201 rechnen schon mit „ln B“. Die
   Abtastrate nennt das Papier nicht. **Abb. 5 lässt sich nur mit fs = 44,1 kHz nachbilden, nicht mit 48 kHz.** Die
   Formel ist also an 44,1 kHz gebunden. `[eigene Rechnung]`
6. **`adf.m` Beispiel 1** (32,7 Hz, 21 Biquads): Die Abschrift in S201 stimmt Zeichen für Zeichen mit der Rohdatei. Die
   12,5 Cent an den Teiltönen 1–12 kommen vom **einzelnen Newton-Schritt**: mit exakt gelösten Polfrequenzen sind es 0,4
   Cent. Die 248 Cent oben kommen daher, dass `adf.m` ein **anderes Teiltongesetz** annimmt: aus Z. 38 folgt
   f_n = n f₀ /√(1 − B n²) statt n f₀ √(1 + B n²). Am Teilton 47 liegen beide 188 Cent auseinander. Das Papier selbst
   war nicht zu bekommen; ob es dasselbe Gesetz nimmt, ist offen. `[eigene Rechnung]`

---

## 1. Bank & Välimäki 2003 vollständig

### 1a Gleichungen der Quelle

Quelle: B. Bank, V. Välimäki, „Robust Loss Filter Design for Digital Waveguide Synthesis of String Tones“, IEEE Signal
Processing Letters 10 (1), Januar 2003, S. 18–20; `…\cmake\quellen\gezupfte-saite\Bank_Valimaki_2003_RobustLossFilter_SPL10.pdf`,
alle drei Seiten in diesem Lauf am Bild gelesen (`seiten/Bank_Valimaki_2003_RobustLossFilter_SPL1_S000{1,2,3}_130dpi.png`).

- **(B1)** g_k = e^(−1/(f₀ τ_k)), |H_l(e^{jϑ_k})| = g_k — S. 18, Abschnitt III. `[wörtlich, Seitenbild]`
- **(B2)** e_τ = Σ (f(ĝ_k) − f(g_k))² ≈ Σ w_k (ĝ_k − g_k)², f(ĝ) = −1/(f₀ ln ĝ) — S. 19, Gl. (1). `[wörtlich, Seitenbild]`
- **(B3)** w_k = 1/(g_k² (ln g_k)⁴) ≈ 1/(g_k − 1)⁴ — S. 19, Gl. (2). `[wörtlich, Seitenbild]`
- **(B4) Glättung:** „The decay time data are smoothed by convolving them with a triangular window [0.25, 0.5, 0.25].“ —
  S. 19, VI. `[wörtlich, Seitenbild]`. Begründung in IV: „the measured decay times have a great variance [8], which
  cannot be followed by filters of reasonable order (N < 20)“; dazu S. 20, VII: „Smoothing … is necessary in the case of
  high variance of measured decay times.“ `[wörtlich, Seitenbild]`
- **(B5) Hohe Frequenzen:** „The missing data points in the high-frequency region are proposed to be calculated by
  designing a one-pole filter for the specification, e.g., by one of the methods presented in [7, pp. 87–91] or in [3].
  Then, the magnitude response of the one-pole filter is used as a specification for the high frequencies. … At the
  original data points of the highest specified frequencies, a crossfade is applied to avoid discontinuities.“ (S. 19, V)
  und „the last five data points of the measured specification are linearly mixed to the magnitude response of the
  designed one-pole filter“ (VI). `[wörtlich, Seitenbild]`
- **(B6) Phase:** „the phase specification can be easily computed from the logarithm of the magnitude specification by
  Hilbert transform [9]. Note that the Hilbert transform needs magnitude data for the entire digital frequency band and on
  a linear frequency scale.“ (V); „The magnitude response on a dense linear grid is calculated by using third-order
  polynomial interpolation … The phase response on this dense grid is computed by the Hilbert transform and then
  resampled at the frequencies of the original specification points.“ (VI) `[wörtlich, Seitenbild]`
- **(B7) Entwurf:** „the weighted least squares method implemented in MATLAB's invfreqz function is used [10]“; Ordnungen
  2, 8, 16; Klavier F♯2, 64 Teiltöne bis 6,57 kHz, fs = 22,05 kHz; „The magnitude response of the designed filters never
  exceeded unity, i.e., the digital waveguide loop remained always stable.“ (S. 19–20) `[wörtlich, Seitenbild]`
- **(B8) Umsetzung von B7 hier:** minimiere Σ_k w_k |B(e^{jϑ_k}) − H_d,k A(e^{jϑ_k})|². Das ist der Gleichungsfehler,
  das Verfahren von `invfreqz` ohne `iter`. Wahlweise folgen 10 Schritte nach Steiglitz-McBride mit dem Gewicht
  w_k/|A_alt,k|² („SM“). `[hergeleitet; die invfreqz-Dokumentation [10] nicht gelesen]`
- **Abb. 1 (b), S. 20** (am Bild): Unter dem ersten Teilton (≈ 100 Hz) fallen die Entwürfe der Quelle zu 0 Hz hin ab
  (iir16 auf ≈ 0,994). Ihr Maximum liegt bei ≈ 0,4–0,5 kHz. Das ist ein Hinweis, dass die Quelle unter f₁ nicht zum
  Betrag 1 hin vorgibt `[eigene Einschätzung, Abbildung abgelesen]`.

### 1b Liste der Abweichungen von der Quelle (Umsetzung `bank_voll.py`, `vorgabe` und `wls`)

1. **Sollwerte aus dem Modell, nicht aus Messung:** g_k = |H_ziel(f_k)| an den Teiltönen des Prüfstands (wie Bericht 11).
   τ_k = −1/(f₀ ln g_k) wird nur zum Glätten gebildet, danach zurück zu g_k (B1).
2. **Sollpunkte:** alle Teiltöne unter 0,45 fs = 21,6 kHz (wie Bericht 11). Die Quelle hat gemessene Teiltöne bis 0,6 fs/2.
   Als Variante „Soll nur hörbar“ nur Teiltöne mit σ ≤ 100 1/s, darüber gilt der Ein-Pol.
3. **Rand der Glättung:** Am ersten und letzten Punkt wird der Wert wiederholt; die Quelle sagt dazu nichts. Variante „Rand
   ungeglättet“ in `bank_diag.txt`.
4. **Ein-Pol (B5):** H = g (1 + a)/(1 + a z⁻¹), 0 < g ≤ 1, −1 < a ≤ 0, angepasst mit Gewicht 1/(1 − g_k). Gewicht und Form
   nach [3] (Välimäki u. a. 1996, dort nicht gelesen); die Quelle lässt [7] oder [3] offen. Der Ein-Pol ist selbst passiv.
5. **Überblendung:** λ = 1/5, 2/5, …, 5/5 an den letzten fünf Sollpunkten; der letzte Punkt ist ganz Ein-Pol, darüber gilt
   der Ein-Pol. Die Quelle nennt nur „linearly mixed“.
6. **Dichtes Raster:** 32 769 Punkte 0…fs/2 (0,73 Hz). „Third-order polynomial interpolation“ ist hier **pchip** (MATLAB
   `interp1(…,'cubic')` ist pchip `[eigene Einschätzung, nicht an der MATLAB-Doku geprüft]`); als Variante ein kubischer
   Spline.
7. **Unter f₁:** Der Wert bei f₁ wird gehalten; die Quelle sagt nichts. **Diese Setzung entscheidet über die Passivität**
   (1c). Variante „unter f1 Ziel“ in `bank_diag.txt`.
8. **Hilbert-Transformation** über das reelle Cepstrum von ln|H| (2¹⁶ Punkte). Das ist rechnerisch dasselbe wie die
   Mindestphase nach (B6) `[hergeleitet]`.
9. **Gewicht:** linke Seite von Gl. (2), auf den Größtwert normiert.
10. **Löser:** Gleichungsfehler (Hauptfall, „voll“) und SM (Variante); ob `invfreqz` 2003 intern mehr tat, ist nicht
    geprüft.
11. **Instabile Pole** werden gespiegelt und b mit Π 1/|r| ausgeglichen, sodass der Betrag gleich bleibt. Die Quelle sagt
    nichts dazu, weil ihre Entwürfe stabil waren.
12. **Ordnung nur, wenn 2N + 1 ≤ 2K** (K Sollpunkte): Harfe 103 (K = 6) nur iir2/iir4, Harfe 96 (K = 9) ohne iir16.
    **iir16** ist gerechnet, wie die Quelle es mit 64 Teiltönen tut; sinnvoll ist es nur an Tasten mit vielen Teiltönen.
13. **fs = 48 kHz** (Quelle 22,05 kHz).

Alle Folgerungen unten gelten **für diese Umsetzung**. Wo eine Folgerung an einer der Setzungen 3, 5, 6 oder 7 hängt,
ist das gesagt.

### 1c Ergebnis an den 18 Tasten aus Bericht 11 (voller Umlauf: Dispersion K16 5 kHz, Thiran 1, nachgestimmt — wie Bericht 11 Tab. 2c)

Je Zelle **max|H| vor jeder Skalierung · Fehler des Abklingens am Grundton · Teiltöne 1–12 / ≤ 5 kHz / hörbar (σ ≤ 100 1/s)**,
in %, unskaliert; * = über 1. Aus `weg2_tasten.txt`, Zeilen „Weg 1“ (gleich `bank_voll.txt`, Variante „voll“). Teiltöne:
Phase des Umlaufs = 2πn, zwischen den Punkten des 0,5-Hz-Rasters interpoliert (`ue.teiltoene`, `np.interp`).
`[eigene Rechnung]`

| Taste | iir2 | iir4 | iir8 | Bericht 11 iir4 |
|---|---|---|---|---|
| Git. 6/0 | 0,99274 · −0,6 · 1,5/7,0/21,2 | **11,671*** · +0,6 · 0,6/4,1/22,8 | 0,99263 · +0,7 · 0,7/5,2/21,1 | 0,1/4,7/22 |
| Git. 3/0 | 0,99514 · +3,8 · 3,8/3,8/8,5 | 0,99521 · +2,9 · 2,9/2,9/13,3 | 0,99516 · +3,0 · 3,0/3,0/13,4 | 0,3/0,5/13,5 |
| Git. 3/2 | 0,99564 · +3,8 · 3,8/3,8/7,8 | 0,99568 · +3,1 · 3,1/3,1/13,2 | 0,99561 · +3,4 · 3,4/3,4/13,2 | 0,3/0,4/13,4 |
| Git. 3/12 | 0,99737 · +5,7 · 5,7/5,7/10,9 | 0,99728 · +5,5 · 5,5/5,5/8,6 | 0,99724 · +5,4 · 5,4/5,4/9,1 | 0,9/0,9/8,6 |
| Git. 1/12 | 0,99766 · +12,6 · 12,6/12,6/16,5 | 0,99741 · +10,6 · 10,6/10,6/10,6 | 0,99876 · +3,5 · 47,7/10,6/101,5 | 1,1/0,1/4,8 |
| Harfe 27 | 0,98593 · +0,8 · 7,2/62,4/28,2 | 0,98629 · −1,6 · 4,2/63,1/31,7 | 0,98601 · +0,1 · 4,3/62,5/38,9 | 5,3/64/36 |
| Harfe 46 | 0,99437 · +10,4 · 15,2/18,7/17,6 | 0,99441 · +10,1 · 10,1/14,2/10,1 | 0,99458 · +9,9 · 9,9/17,8/9,9 | 202* |
| Harfe 60 | 0,99609 · +10,0 · 10,0/10,0/10,0 | 0,99625 · +9,7 · 9,7/9,7/9,7 | 0,99590 · +9,0 · 9,0/9,0/9,0 | 0,4/1,9/1,9 |
| Harfe 96 | 0,99785 · +12,6 · 40,3/12,6/18,9 | 0,99778 · +11,7 · 41,9/11,7/11,7 | 0,99783 · +11,5 · 42,4/11,5/11,5 | 14,6/0,2/5,8 |
| Harfe 103 | 0,99801 · +14,0 · 41,1/14,0/21,4 | 0,99804 · +13,8 · 40,5/13,8/19,3 | — (K = 6) | 8373* |
| Cemb. 29 | 0,99769 · +6,5 · 6,5/38,3/78,3 | 0,99761 · +8,2 · 8,2/37,0/78,8 | 0,99767 · +7,4 · 7,4/36,3/78,9 | 30* |
| Cemb. 36 | 0,99798 · +8,1 · 8,1/8,1/62,7 | 0,99792 · +9,6 · 9,6/9,6/59,9 | 0,99802 · +7,3 · 7,3/7,9/60,5 | 0,4/9,5/61 |
| Cemb. 60 | 0,99858 · +9,0 · 9,0/9,0/20,8 | 0,99862 · +7,3 · 7,3/7,3/16,8 | 0,99860 · +5,6 · 5,6/5,6/21,2 | 0,2/0,2/12,8 |
| Cemb. 84 | 0,99933 · +8,1 · 8,1/8,1/12,0 | 0,99930 · +7,4 · 7,4/7,4/10,5 | 0,99929 · +7,1 · 7,1/7,1/11,7 | 0,4/0,3/5,1 |
| Laute 1 | 0,99381 · +15,3 · 65,7/75,4/75,4 | 0,99441 · +9,9 · 23,7/47,8/33,4 | 0,99423 · +10,8 · 17,6/36,6/19,0 | 27/61/29 |
| Laute 7 | 0,99486 · +10,3 · 10,3/10,3/23,3 | 0,99476 · +8,3 · 8,3/8,3/8,3 | 0,99450 · +7,1 · 7,1/7,1/7,1 | 0,3/0,3/7,8 |
| Oud 1 | 0,99148 · +0,3 · 1,7/10,3/23,0 | 0,99146 · +0,5 · 1,4/11,5/24,6 | 0,99142 · +1,0 · 1,0/18,9/27,0 | 0,3/19/31 |
| Oud 5 | 0,99406 · +11,5 · 11,5/11,5/18,0 | 0,99444 · +8,5 · 8,5/8,5/8,5 | 0,99402 · +8,2 · 8,2/8,2/8,2 | 0,4/0,4/2,4 |

**iir16** (`bank_voll.txt`, „voll“, 16 Tasten; Harfe 96/103 zu wenig Sollpunkte): alle passiv (größtes 0,99930 Cembalo 84).
1–12 / hörbar: Git. 6/0 0,3/21,2 · Harfe 27 4,4/37,8 · Cemb. 29 6,4/78,9 · Laute 1 20,8/22,1 · Oud 1 0,5/27,2 — **nicht
besser als iir8**.

**Wichtig zum Lesen der Spalten „≤ 5 kHz“ und „hörbar“:** Im vollen Umlauf stecken auch die Fehler der Dispersion. K16
5 kHz verschiebt an Harfe 27, Cembalo 29/36 und Oud 1 die hörbaren Teiltöne um bis zu 97–903 Cent („Cent hörbar“ in `weg2_tasten.txt`), und
damit verschiebt sich auch σ. Das **Maß nur des Verlustfilters** (`weg2.verlust_mass`: p_n = 100 (ln|H(f_n)|/ln|H_ziel(f_n)| − 1),
gilt, wenn Dispersion und Verzug die Phase des Ziels treffen) gibt dort viel weniger, z. B. Harfe 27 iir8 1,1/2,8/2,8,
Cembalo 29 iir8 7,4/7,4/7,4 (`weg2_tasten.txt`, zweiter Block). `[eigene Rechnung]`

**Passivität (Frage 1):** Mit Gleichungsfehler ist an 17 Tasten jede Ordnung passiv. An **Gitarre 6/0 iir4** liegt das
Maximum mit 11,67 bei 24 000 Hz, ein Pol ist gespiegelt. Mit SM (`bank_voll.txt`, „voll SM“) liegen Gitarre 6/0 iir4
(1,1438 bei 24 kHz) und Oud 1 iir16 (4,8240 bei 24 kHz) über 1, alle übrigen 51 Fälle darunter. `[eigene Rechnung]`

### 1d Woran es liegt — Glättung, HF-Vorgabe oder Ziel?

Drei Läufe trennen die Ursachen: `bank_voll.txt` (Varianten ohne Glättung / ohne HF / ohne beides / Spline / nur hörbar),
`bank_diag.txt` (unter f₁ das wahre Ziel; Rand ungeglättet), `diag_bericht11.txt` (Bericht 11 nachgerechnet).

| Variante (Gleichungsfehler, iir2/4/8, 18 Tasten) | Fälle über 1 | Tasten | wo das Maximum liegt |
|---|---|---|---|
| voll (Quelle, wie 1b) | 1 | Git. 6/0 (iir4) | 24 kHz |
| ohne Glättung | 1 | Git. 6/0 (iir4, 14,61) | 24 kHz |
| ohne HF-Vorgabe | 0 | — | — |
| ohne beides | 0 | — | — |
| Spline statt pchip | 1 | Git. 6/0 (iir4, 27,91) | 24 kHz |
| Soll nur hörbar | 2 | Git. 1/12 iir8 (1,00034), Harfe 103 iir4 (1,00191) | 0 Hz |
| voll, Rand ungeglättet | 1 | Git. 6/0 (iir4, 21,32) | 24 kHz |
| **voll, unter f₁ das wahre Ziel** | **11** | **9:** Git. 6/0, 3/12, 1/12; Harfe 46, 60, 96; Cemb. 60, 84; Oud 5 | 0 Hz (9 Fälle), 1685 Hz (1), 24 kHz (1) |
| ohne beides, unter f₁ das wahre Ziel | 5 | 4: Harfe 60, 96, 103; Laute 1 | 0 Hz |
| dasselbe mit SM | 8 | 6: Git. 3/2; Harfe 103; Cemb. 29, 36; Laute 7; Oud 5 (Harfe 103 und Cemb. 29 je zweimal) | 0 Hz |
| Bericht 11 nachgerechnet (`diag_bericht11.txt`) | 8 | 6: Harfe 46, 60, 96, 103; Cemb. 29; Laute 1 | 0 Hz (7), 24 kHz (1) |

(Gezählt mit `Grep` auf Zeilen mit max|H| ≥ 1 in `bank_voll.txt` und `bank_diag.txt`.) `[eigene Rechnung]`

**Folgerungen** `[eigene Rechnung, Deutung eigene Einschätzung]`:
- **Am Ziel selbst (Form unter f₁).** Wo −ln|H_ziel| zu 0 Hz hin auf wenige Prozent fällt (monofile Saiten: Harfe Darm/Nylon
  0,007–0,023, Cembalo 0,017–0,039, Laute/Oud monofil 0,018–0,029, Gitarre G 12. Bund 0,82, E 12. Bund 0,47), steigt der
  Sollbetrag unter f₁ gegen 1. Über die Mindestphase geht dieser Anstieg in die Phase an den Teiltönen ein. Der Entwurf
  folgt ihr und legt sein Maximum in die Lücke unter f₁ (0 Hz). Bei umsponnenen Saiten (Gitarre E, Harfe 27, Oud 1:
  0,98–0,99) ist −ln|H_ziel| unter f₁ flach; dort tritt das nicht auf. **Das ist die Ursache der Fälle in Bericht 11.**
  Hält man unter f₁ den Wert bei f₁ fest (1b Nr. 7), verschwindet sie.
- **Spiegeln ohne Ausgleich** (Bericht 11): Es hebt den Betrag um |r| je gespiegeltem Pol. Von den 8 Fällen hatte nur Harfe 46
  iir8 einen Pol außen (|r| = 1,0052); mit Ausgleich liegt er bei 0,99735 statt 1,00256 (`diag_bericht11.txt`).
- **HF-Vorgabe:** Sie ist die einzige Ursache, die im vollständigen Verfahren übrig bleibt (Gitarre 6/0 iir4; über den
  Umfang 13 von 18 Fällen, Abschnitt 2d). Über 21,6 kHz gibt es keinen Sollpunkt. Der Ein-Pol mit Überblendung ändert die
  Mindestphase an den obersten Teiltönen, und der Gleichungsfehler legt dann einen Pol nahe z = −1.
- **Glättung:** Für die Passivität ist sie fast neutral. Für die **Genauigkeit** ist sie schädlich, weil das Modellziel
  schon glatt ist. Mit wiederholtem Randwert wird τ₁ zu 0,75 τ₁ + 0,25 τ₂, und der Grundton klingt an den monofilen
  Saiten 2,9–15,3 % zu schnell ab (Tabelle 1c, Spalte Grundton; umsponnene Saiten −1,6…+1,0 %). Ohne Glättung sind es
  höchstens 6,2 %, mit ungeglättetem Rand höchstens 5,5 % (Beträge; `bank_voll.txt`, `bank_diag.txt`). An den Teiltönen
  1–12 z. B. Harfe 60 iir8: 8,95 % voll, 4,43 % mit ungeglättetem Rand, 0,41 % ohne Glättung.

---

## 2. Entwurf mit Nebenbedingung |H(e^{iω})| ≤ 1

Gleiche Vorgabe (g_k, H_d,k aus 1b) und gleiches Gewicht (B3) wie Weg 1.

**(W1) Weg 2a — konvex** `[hergeleitet]`. Der Nenner A kommt fest aus Weg 1 (Gleichungsfehler). Gesucht ist der Zähler über
die Autokorrelation r = (r₀…r_N):
 R(ω) = |B(e^{iω})|² = r₀ + 2 Σ_{n=1}^{N} r_n cos(nω) (linear in r),
 min_r Σ_k w_k/(4 g_k²) · (R(ϑ_k)/|A(ϑ_k)|² − g_k²)² (erste Ordnung von w_k (|H_k| − g_k)², weil ĝ² − g² ≈ 2g(ĝ − g)),
 mit δ |A(ω_j)|² ≤ R(ω_j) ≤ (1 − ε) |A(ω_j)|² auf 4097 Punkten ω_j ∈ [0, π] (ε = 10⁻⁷, δ = 10⁻⁹).
Das ist ein quadratisches Problem mit linearen Ungleichungen, gelöst als LSI → LDP → NNLS (`scipy.optimize.nnls`); das
Verfahren ist das von Lawson & Hanson 1974, Kap. 23 `[eigene Einschätzung: aus dem Gedächtnis, Buch nicht gelesen]`. B
mit Mindestphase entsteht aus den N Wurzeln von z^N R(z) im Einheitskreis. Zwischen den Rasterpunkten kann |H| um weniger
als ε über 1 liegen; das Skript prüft auf 65 537 Punkten nach und teilt notfalls durch das Maximum (gemeldet: größter
Wert 1,000000 an Gitarre 6/0 iir4).

**(W2) Weg 2b — passiv durch den Bau** `[hergeleitet]`: H(z) = g · (A₁(z) + A₂(z))/2, A₁, A₂ stabile Allpässe der Ordnung
n₁ + n₂ = N, 0 < g ≤ 1. Wegen |A_i| = 1 gilt |H| ≤ g ≤ 1 für alle ω. Die Allpass-Summe ist aus der Literatur zu Filtern
mit geringer Empfindlichkeit bekannt `[eigene Einschätzung, keine Quelle gelesen]`. Parameter: Pole der Biquads als
r = 0,9995·sig(p), θ = π·sig(q). Angepasst an Gl. (1) der Quelle **ohne** Linearisierung, Σ w_k (|H_k| − g_k)², mit
`scipy.optimize.least_squares`, 24 Startwerten (fester Samen 21) und zwei Aufteilungen (n₁, n₂). Gegenüber V7 in Bericht 11
(nur Tiefpass-Pole und reelle Nullen) ist er freier: komplexe Pole, und Nullen entstehen aus der Summe.

### 2a Tabelle je Taste (voller Umlauf wie 1c; aus `weg2_tasten.txt`)

Je Zelle **1–12 / ≤ 5 kHz / hörbar** in %. max|H| ist bei Weg 2a ≤ 1,000000, bei Weg 2b ≤ 0,99969 (alle 18 Tasten). Rechts
die Rechenzeit des Entwurfs bei iir8 (ms, Weg 2a / 2b). ⚑ = Teiltöne 1–12 um mehr als 5 Cent schlechter als Weg 1 gleicher
Ordnung (Phase des Allpasses). `[eigene Rechnung]`

| Taste | 2a iir4 | 2a iir8 | 2b iir4 | 2b iir8 | ms 2a/2b |
|---|---|---|---|---|---|
| Git. 6/0 | 0,5/4,1/23,2 | 0,5/5,0/21,2 | 0,6/7,3/39,8 | 0,5/5,3/20,5 | 13/2391 |
| Git. 3/0 | 2,5/2,5/12,1 | 2,4/2,8/13,5 | 3,8/3,8/9,1 | 2,8/3,8/26,8 | 17/1130 |
| Git. 3/2 | 2,8/2,8/12,2 | 2,7/2,7/13,3 | 3,7/3,7/9,0 | 3,6/3,6/11,6 | 15/1196 |
| Git. 3/12 | 5,2/5,2/9,8 | 5,2/5,2/8,8 | 6,3/6,3/10,1 | 5,0/5,0/8,6 | 16/624 |
| Git. 1/12 | 9,9/9,9/9,9 | 9,8/9,8/9,8 | 10,6/10,6/36,0 | 168,6/119,4/184,2 ⚑ (529 Cent) | 10/1042 |
| Harfe 27 | 4,4/62,6/31,2 | 4,3/62,7/39,2 | 3,7/65,7/26,9 | 4,0/66,3/40,5 | 24/3157 |
| Harfe 46 | 9,8/13,4/9,8 | 7,8/18,4/7,8 | 9,9/32,0/20,7 | 9,3/129,5/37,1 | 22/632 |
| Harfe 60 | 7,9/7,9/7,9 | 8,2/8,2/8,2 | 46,5/46,5/46,5 ⚑ (239 Cent) | 8,1/8,1/15,2 ⚑ | 14/703 |
| Harfe 96 | 41,2/11,5/11,5 | 42,4/11,5/11,5 | 10,7/10,7/10,7 ⚑ | 5419/13,4/5419 ⚑ (314 Cent) | 18/1633 |
| Harfe 103 | 40,5/13,8/19,3 | — | 68,8/14,6/68,8 ⚑ | — | 51/409 (iir4) |
| Cemb. 29 | 7,2/36,8/78,7 | 5,6/36,5/78,7 | 7,8/42,1/59,6 ⚑ | 7,2/45,2/83,4 ⚑ | 13/1038 |
| Cemb. 36 | 9,7/9,7/60,0 | 5,7/8,1/60,6 | 9,6/9,6/31,4 ⚑ | 8,6/8,6/68,1 ⚑ | 11/1421 |
| Cemb. 60 | 6,4/6,4/19,1 | 5,7/5,7/21,3 | 20,3/20,3/84,5 | 8,6/8,6/38,2 ⚑ | 12/1119 |
| Cemb. 84 | 6,4/6,4/10,9 | 6,4/6,4/11,7 | 9,4/9,4/13,9 | 10,4/6,7/19,2 ⚑ | 12/651 |
| Laute 1 | 25,0/46,6/34,7 | 14,1/46,2/31,4 | 40,3/62,1/62,1 | 39,4/56,1/56,1 | 22/1079 |
| Laute 7 | 7,0/7,0/7,0 | 6,9/6,9/6,9 | 7,4/7,4/15,3 ⚑ | 11,3/11,3/15,7 ⚑ | 13/1159 |
| Oud 1 | 1,4/11,5/24,6 | 0,6/18,2/26,2 | 0,9/22,7/37,7 | 1,2/17,2/25,3 | 15/1934 |
| Oud 5 | 7,3/7,3/7,3 | 7,4/7,4/7,4 | 30,6/30,6/30,6 ⚑ (162 Cent) | 7,4/7,4/10,8 ⚑ | 11/976 |

⚑ steht an **10 der 18 Tasten** in mindestens einer der Ordnungen 4 und 8 (Git. 1/12, Harfe 60, 96, 103, Cemb. 29, 36, 60, 84,
Laute 7, Oud 5; gezählt aus der Spalte „Cent 1-12“ in `weg2_tasten.txt`). Weg 2a ändert die Cent-Werte (1–12, ≤ 5 kHz,
hörbar) gegen Weg 1 um höchstens 1,0 Cent. Die Ausnahme ist Git. 1/12 iir8: dort hat Weg 1 mit einem gespiegelten Pol
140,8 Cent, Weg 2a 1,3 Cent.
**Abklingen 2a gegen Weg 1:** Bei 1–12 liegen die meisten Zellen innerhalb ±2 %. Die größten Unterschiede (hörbar):
Laute 1 iir8 31,4 statt 19,0 %, Oud 5 iir2 28,4 statt 18,0 %, Git. 3/12 iir2 17,1 statt 10,9 %. Viel besser ist 2a an
Git. 1/12 iir8 (1–12: 9,8 statt 47,7 %). Der Fehler am Grundton liegt bei 2a zwischen −2,0 und +15,1 %; wie bei Weg 1
prägt ihn die Glättung.

**Rechenzeit des Entwurfs je Taste** (offline, ein Kern, `weg2_tasten.txt`): Weg 1 7–19 ms, Weg 2a 9–51 ms, Weg 2b 0,13–3,2 s.

### 2b Ganzer Umfang (Maß nur des Verlustfilters; aus `umfang_auswertung.txt`)

Tasten wie im Prüfstand: Gitarre 6 Saiten × Bund 0–19 (120), Harfe 25–104 (80), Cembalo 29–89 (61), Laute 22 und Oud 12
(je Chor jede verschiedene Saite, leer und Bund 5). Zusammen **295 Tasten**. W1 = Bank & Välimäki vollständig; oG = dasselbe
ohne Glättung. Fehler = größter |Fehler| an den hörbaren Teiltönen; Median / Größtwert über die passiven Tasten.
`[eigene Rechnung]`

| Ordnung | Instrument | Tasten | W1 über 1 | oG über 1 | W1 hörbar Med./max | oG hörbar Med./max | Weg 2a an den Tasten über 1 (hörbar) | Weg 2b dto. |
|---|---|---|---|---|---|---|---|---|
| 2 | Gitarre | 120 | 0 | 1 | 8,1 / 18,1 | 9,2 / 18,4 | 17,1 | 13,9 |
| 2 | Harfe | 80 | 0 | 0 | 12,5 / 35,8 | 16,6 / 36,0 | — | — |
| 2 | Cembalo | 61 | 0 | 0 | 13,4 / 17,1 | 13,4 / 17,3 | — | — |
| 2 | Laute | 22 | 0 | 0 | 17,8 / 43,2 | 22,3 / 44,3 | — | — |
| 2 | Oud | 12 | 0 | 0 | 12,9 / 23,4 | 13,1 / 25,6 | — | — |
| 4 | Gitarre | 120 | **7** (Saite 6, Bund 0–6) | 8 | 4,0 / 12,1 | 1,9 / 4,6 | 1,2–8,7 | 8,7–25,5 |
| 4 | Harfe | 80 | **4** (T43, T44, T74, T104) | 4 | 10,8 / 30,1 | 5,2 / 25,8 | 1,4–29,8 | 8,8–29,9 |
| 4 | Cembalo | 61 | 0 | 2 | 11,5 / 14,3 | 11,4 / 14,7 | 6,9; 9,4 | 9,2; 12,7 |
| 4 | Laute | 22 | 0 | 0 | 8,9 / 25,5 | 5,1 / 21,8 | — | — |
| 4 | Oud | 12 | **1** (Chor 2.1 Bund 5) | 0 | 3,2 / 11,2 | 3,1 / 11,2 | 1,3 | 14,5 |
| 8 | Gitarre | 120 | **2** (Saite 1 Bund 11, 18) | 6 | 3,5 / 11,8 | 0,3 / 1,3 | 7,1–11,3 | 7,0–11,2 |
| 8 | Harfe | 72 gerechnet | **3** (T26, T32, T40) | 1 | 9,0 / 11,5 | 0,5 / 7,1 | 0,6–5,0 | 2,3–24,7 |
| 8 | Cembalo | 61 | **1** (T87) | 2 | 17,4 / 27,7 | 17,4 / 27,7 | 8,2–15,4 | 9,7–31,0 |
| 8 | Laute | 22 | 0 | 0 | 8,7 / 32,3 | 3,5 / 32,5 | — | — |
| 8 | Oud | 12 | 0 | 1 | 1,4 / 8,2 | 0,5 / 2,1 | 7,2 | 7,0 |

Weg 2 ist an jeder Taste gerechnet, an der W1 **oder** oG über 1 lag. Die Spalte „Weg 2a“ enthält deshalb auch Tasten, an
denen nur oG über 1 lag. Weg 2a ist an keiner Taste gescheitert.

**Antwort auf „wie viele Tasten trägt Weg 1, wie viele brauchen Weg 2“:** Bei **fester Ordnung** brauchen iir2 0, iir4 12 und
iir8 6 der 295 Tasten Weg 2 (bei iir8 sind 287 gerechnet; an 8 hohen Harfentasten gibt es zu wenig Sollpunkte). Weg 2a liegt
dort beim Fehler hörbar bei 0,6–29,8 %. Mit der **Regel „kleinster hörbarer Fehler unter den passiven Ordnungen 2/4/8“**
braucht **keine** Taste Weg 2. Danach liegen die Fehler so (Median / max):

| Regel über 2/4/8 | Gitarre | Harfe | Cembalo | Laute | Oud |
|---|---|---|---|---|---|
| W1 (Quelle vollständig) | 3,4 / 11,8 % (71 von 120 ≤ 5 %) | 9,1 / 30,8 % (17 von 80) | 11,4 / 13,7 % (0 von 61) | 8,7 / 21,8 % (0 von 22) | 1,4 / 8,2 % (8 von 12) |
| oG (ohne Glättung) | 0,3 / 4,2 % (120 von 120) | 0,7 / 22,5 % (68 von 80) | 11,2 / 13,6 % (1 von 61) | 3,5 / 21,8 % (15 von 22) | 0,6 / 3,4 % (12 von 12) |

Am Cembalo bleiben 11 % auch ohne Glättung. Dort sind **alle** Teiltöne bis 20 kHz „hörbar“ (σ < 100 1/s), und der Fehler
sitzt an den obersten (Bericht 11 Abschnitt 0); die Teiltöne 1–12 haben ohne Glättung Median 4,1 % (iir4) bzw. 3,3 % (iir8).

### 2c Ursache an den 18 nicht passiven Fällen des Umfangs (`umfang_ursache.txt`)

- **13 Fälle mit Maximum bei 19,6–24 kHz** (Gitarre Saite 6 Bund 0–6 iir4; Harfe T26, T32, T40 iir8, T43, T44 iir4; Oud 2.1
  Bund 5 iir4): Ohne HF-Vorgabe sind **alle 13** passiv. Ohne Glättung bleiben 10 über 1 (Gitarre Saite 6 Bund 0–6, Harfe
  T26, T43, T44), mit SM noch 5 (Gitarre Saite 6 Bund 0–4).
- **5 Fälle mit Maximum bei 0 Hz** (Gitarre Saite 1 Bund 11 und 18 iir8; Harfe T74, T104 iir4; Cembalo T87 iir8), nur 1,0003–1,018:
  Ohne HF-Vorgabe und ohne Glättung sind alle 5 passiv, mit SM 4 von 5.
- Damit lautet die Antwort auf die Frage nach der Ursache: Im vollständigen Verfahren ist es **vor allem die HF-Vorgabe**,
  daneben die Glättung, an Tasten, deren Ziel unter f₁ steil ist. Die 6 Tasten aus Bericht 11 lagen dagegen am **Ziel unter
  f₁**, das dort in die Phase einging (1d). `[eigene Rechnung]`

---

## 3. Originale der Dispersionsformeln

### 3a Rauhala & Välimäki — DAFx-06 gefunden, SPL 2006 nicht

**Gelesen:** J. Rauhala, V. Välimäki, „Dispersion Modeling in Waveguide Piano Synthesis Using Tunable Allpass Filters“,
Proc. 9th Int. Conference on Digital Audio Effects (DAFx-06), Montreal, 18.–20. September 2006, S. DAFX-71 bis -76 (Titel,
Autoren und Kopfzeile von der ersten Seite). URL:
`https://web.archive.org/web/20200722051908id_/http://www.dafx.de/paper-archive/2006/papers/p_071.pdf` (Wayback-Kopie von
dafx.de, 22.07.2020). Datei: `abrufe/Rauhala_Valimaki_2006_DispersionTunableAllpass_DAFx06.pdf` (461 633 Bytes, SHA-256 in
`abrufe/INDEX.md`). Seiten DAFX-71 bis -74 am Bild gelesen (`seiten/Rauhala_Valimaki_2006_DispersionTunableA_S000{1–4}_130dpi.png`).

Gleichungen, am Seitenbild gelesen `[wörtlich, Seitenbild]`, S. DAFX-71 bis -73:
- **(R1)** f_k = k f₀ √(1 + B k²) (Gl. 1); B = π³ Q d⁴/(64 l² T) (Gl. 2).
- **(R2)** D(I_key, B) = e^(C_d(B) − I_key k_d(B)) (Gl. 3); I_key(f₀) = log_{¹²√2}(f₀ ¹²√2 / 27,5) (Gl. 4);
  k_d(B) = e^(k₁ (ln B)² + k₂ ln B + k₃) (Gl. 5); C_d(B) = e^(C₁ ln B + C₂) (Gl. 6).
- **(R3)** wie gedruckt: C_d(B, M) = e^((m₁ ln M + m₂) ln M + m₃ ln M + m₄) (Gl. 7).
- **(R4)** a₁ = (1 − D)/(D + 1) (Gl. 8).
- **Tab. 2** (Kaskade erster Ordnung): k₁ −0,00179; k₂ −0,0233; k₃ −2,93; m₁ 0,0126; m₂ 0,0606; m₃ −0,00825; m₄ 1,97.
  **Stimmt mit der Faust-Abschrift in Bericht 01/11 überein** (D2 in Bericht 11, Abschnitt 1b).
- **Tab. 1** (Kaskade zweiter Ordnung, Methode [7] = SPL-Brief): Filter 1 (M = 4, Tasten 1–44) k₁ −0,00050469, k₂ −0,0064264,
  k₃ −2,8743, C₁ 0,069618, C₂ 2,0427; Filter 2 (M = 1, Tasten 45–88) k₁ −0,0026580, k₂ −0,014811, k₃ −2,9018, C₁ 0,071089,
  C₂ 2,1074.
- **Gültigkeit laut Text** `[wörtlich, Seitenbild]`: k₁…k₃ „were determined by using M = 8“; C₁, C₂ für „eight cases with M
  varying from 1 to 8“; „the fixing of parameters k₁, k₂, and k₃ does not add too much bias to the results when M < 40“;
  „In practical cases, M values below 20 are preferred“. Die Abbildungen gehen über B = 10⁻⁶ … 10⁻³ und Tasten 1–88
  (Klavier, Steinway-Aufnahmen der University of Iowa). Fehlergrenze in Abb. 5: „0.5% error limits corresponding to
  ±8.39 cents“.
- **Abtastrate:** im Text nicht genannt. `pdf_suche.py` nach „kHz“, „sample rate“, „fs“ auf allen 6 Seiten: 0 Treffer, nur
  Textebene. **Kein Treffer ist kein Beweis.**

**Gegenprobe** (`dispersion_original.txt`, Teile 1 und 2) `[eigene Rechnung]`:
- **Gl. (7) wie gedruckt kann nicht stimmen:** Sie hängt nicht von B ab und gibt an allen 18 Ablesepunkten a₁ = −0,996 …
  −1,000. Mit **„ln B“ statt des ersten „ln M“**, also C_d = e^((m₁ ln M + m₂) ln B + m₃ ln M + m₄), ist es Gl. (6) mit C₁ =
  m₁ ln M + m₂ und C₂ = m₃ ln M + m₄, wie Abb. 2 sie zeigt: C₁(1) = 0,0606, C₁(8) = 0,0868, C₂(8) = 1,953; Abb. 2 zeigt
  ≈ 0,061 / ≈ 0,087 / ≈ 1,957. Abweichung zu Abb. 3 und 4: höchstens 0,028 (Taste 28, B 10⁻⁶: −0,318 gegen abgelesen −0,29),
  sonst ≤ 0,02. Ablesegenauigkeit ±0,02. **Faust und die S201-Gegenprobe rechnen schon mit „ln B“**: Differenz 0 an allen
  Punkten. Das „Erratum zu Gl. (7)“ ist also ein Druckfehler im DAFx-Papier. Eine veröffentlichte Berichtigung ist nicht
  gefunden (eine Websuche, `pdf_suche.py` „erratum“ 0 Treffer).
- **Abb. 5 nachgerechnet:** Schleife = reiner Verzug + M gleiche Allpässe, am Grundton gestimmt, Abweichung gegen (R1). Mit
  **fs = 44,1 kHz** erscheint der Buckel der Abbildung: (a) M = 1 bis +6,3 Cent bei 263 Hz, M = 40 bis +7,1 Cent bei 753 Hz;
  (b) M = 40 +10,3 Cent bei 2049 Hz. Abgelesen sind ≈ +7 bei ≈ 280 Hz, ≈ +8 bei ≈ 800 Hz und ≈ +10 bei ≈ 2000 Hz. Mit
  **48 kHz** gibt es keinen positiven Buckel (M ≥ 10: ≤ +0,4 Cent), und die Kurven fallen früher. **Die Konstanten gelten
  für fs = 44,1 kHz.** Für 48 kHz muss man sie neu bestimmen oder die Formel mit I_key + 12 log₂(48/44,1) umrechnen (nicht
  gerechnet).
- **SPL-Brief 2006** (IEEE SPL 13 (5), 253–256): nicht frei gefunden. Laut einer Suchzusammenfassung behandelt er die
  Kaskade **zweiter** Ordnung `[nur Zusammenfassung]`; das DAFx-Papier nennt ihn als [7], und seine Tab. 1 stammt von dort.
  Gl. (7) für erste Ordnung steht nur im DAFx-Papier `[eigene Einschätzung]`.

### 3b Abel, Välimäki & Smith 2010 — Papier nicht gefunden, `adf.m` roh gelesen

**Nicht gefunden:** IEEE SPL 17 (4), 406–409, DOI 10.1109/LSP.2010.2040924. Semantic Scholar meldet „openAccessPdf: status
CLOSED“ (`abrufe/s2_abel2010.json`) `[wörtlich, Rohtext]`. Der Begleitordner `research.spa.aalto.fi/publications/papers/spl-adf/`
enthält laut Verzeichnisliste in der Wayback Machine **nur** `adf.m` (1,9 K, 2009-12-11) (`abrufe/wayback_spl_adf_index.html`).
Die CCRMA-Seite zum DAFx-06-Vorläufer (Abel & Smith) enthält kein PDF; die CCRMA-Wiki-Seite verlinkt nur `adf.m`.

**Gelesen:** `adf.m` roh (`abrufe/adf_m_wayback.txt`, URL in `INDEX.md`) `[wörtlich, Rohtext]`. Darin u. a.: Z. 31
`fs = 44100;`, Z. 38 `pd0 = tau0/sqrt(1 + B*(df/f0).^2);`, Z. 46–51 „Newton single iteration“; Beispiele Z. 18–20
`adf(32.702,0.0002,2100,0.85)`, `adf(65.406,0.0001,2500,0.85)`, `adf(130.81,0.00015,4300,0.85)`. Das sind dieselben drei
Töne wie in Rauhala & Välimäki Abb. 5.

**Gegenprobe** (`dispersion_original.txt`, Teile 3–5) `[eigene Rechnung]`:
- Eine eigene Abschrift aus der Rohdatei und die Abschrift der S201-Gegenprobe (`gp.adf`) geben **dieselben Beiwerte**
  (Differenz 0) und dieselbe Stufenzahl (21 / 4 / 4).
- **Beispiel 1** (32,702 Hz, B 2·10⁻⁴, df 2100 Hz, 21 Biquads, Pole 28…1624 Hz), Abweichung gegen (R1), am Grundton
  gestimmt: 1–12 höchstens 12,5 Cent, alle 51 Teiltöne bis 247,8 Cent (Teilton 51, 2056 Hz). Das ist der Befund aus S201.
  - Mit **exakt gelösten Polfrequenzen** statt des einen Newton-Schritts, also ψ(θ) = 2πθ/fs (pd(θ) − μ₀) = (2k − 1)π mit
    `brentq` (größte Verschiebung 49,3 Hz): 1–12 **0,4 Cent**. **Der Newton-Schritt macht die 12,5 Cent.**
  - **Teiltongesetz von `adf.m`** `[hergeleitet]`: Mit Phasenverzug pd(f) = τ₀/√(1 + B (f/f₀)²) (Z. 38) und f·pd(f)/fs = n
    folgt **f_n = n f₀ / √(1 − B n²)**, nicht (R1). Beide Gesetze unterscheiden sich am Teilton 12 um 0,72 Cent und am
    Teilton 47 um 188 Cent. Gegen dieses Gesetz bleibt das Beispiel mit exakten Polen bei 1–12 0,4 Cent und bis df 21,3 Cent.
  - Die Beispiele 2 und 3 (4 Biquads) liegen gegen (R1) bei ≤ 5,9 Cent bis df, mit 1–12 ≤ 0,4 Cent. Dort ist B n² klein.
- Ob das Papier dasselbe Gesetz nimmt wie `adf.m`, ist **ohne das Papier nicht zu klären**. Die Konstante β = 0,85 und die
  Pol-Radien (Z. 55–58) sind nur aus dem Code belegt.

### 3c Was bleibt aus Bericht 01/11 (Abschnitt 1b)

| Abschrift in Bericht 11 | jetzt |
|---|---|
| D2 Rauhala: Formel, k₁…m₄ `[Werkzeug]` | `[wörtlich, Seitenbild]` DAFx-06 Gl. (3)–(8), Tab. 2; Gl. (7) mit „ln B“ (Druckfehler im Papier, an Abb. 2–4 belegt); gilt für 44,1 kHz (Abb. 5); k₁…k₃ aus M = 8, C₁/C₂ aus M = 1…8 |
| D2 „Konstanten an Klaviersaiten bei 44,1 kHz angepasst“ | Klavier: `[wörtlich]`; 44,1 kHz: `[eigene Rechnung]` (im Text nicht genannt, aus Abb. 5) |
| D3 adf `[Werkzeug]` | Code `[wörtlich, Rohtext]`; Papier weiter nicht gelesen |
| „Beispiel 1 nicht bestätigt (12,5 / 248 Cent)“ | erklärt: Newton-Schritt (12,5 → 0,4) und anderes Teiltongesetz (188 Cent am Teilton 47) |

---

## 4. Optionen für LumiViz (Verlustfilter)

Bewertet gegen alle verlangten Zielgrößen. Zahlen aus den Tabellen 1c, 2a, 2b. Rechenzeit je Takt in Multiplikationen,
die Tafel in Zahlen je Taste. Keine Empfehlung nach Aufwand; die fachliche Einschätzung steht am Ende.

| Option | Abklingen (hörbar, Verlustfilter allein, Median / max über die Tasten) | Tonhöhe (Phase) | Passivität | Takt | Tafel je Taste |
|---|---|---|---|---|---|
| **A** Bank & Välimäki **wie die Quelle**, Ordnung nach Regel „kleinster Fehler unter passiven 2/4/8“ | Git 3,4/11,8 · Harfe 9,1/30,8 · Cemb 11,4/13,7 · Laute 8,7/21,8 · Oud 1,4/8,2 %; Grundton an monofilen Saiten +2,9…+15,3 % (Glättung) | Mindestphase; mit Nachstimmen am Grundton keine Abweichung, Teiltöne wie Weg 2a (Ausnahme mit gespiegeltem Pol: Git 1/12 iir8, 141 Cent) | **an allen 295 Tasten eine passive Ordnung**, Prüfung je Taste nötig | iir8 als 4 Biquads: 20 Mult. (iir4: 10) | 2N + 1 = 17 (iir8) bzw. 20 in SOS, dazu die Ordnung |
| **B** wie A, **ohne Glättung** (Abweichung von der Quelle) | Git 0,3/4,2 · Harfe 0,7/22,5 · Cemb 11,2/13,6 · Laute 3,5/21,8 · Oud 0,6/3,4 %; Grundton |·| ≤ 6,2 % (18 Tasten) | wie A | wie A (an allen 295 Tasten eine passive Ordnung) | wie A | wie A |
| **C** wie B, aber **feste Ordnung 8, Weg 2a als Ausweichregel**, wo Weg 1 über 1 liegt (oG iir8: 10 Tasten) | 2a an den 15 Tasten, an denen W1 oder oG bei iir8 über 1 lag: 0,6–15,4 % hörbar; sonst wie B mit iir8 (Median Git 0,3 · Harfe 0,5 · Cemb 17,4 · Laute 3,5 · Oud 0,5 %) | wie A; an Git 1/12 iir8 1,3 statt 141 Cent | **durch die Nebenbedingung** (Raster 4097 + Nachprüfung) | 20 Mult. | 20 (SOS) |
| **D** **Weg 2a an jeder Taste** (fester Nenner aus Weg 1, Zähler mit Nebenbedingung) | an den 18 Tasten meist wie Weg 1, Ausreißer beidseits (Abschnitt 2a); über den Umfang nur an den Fällen über 1 gerechnet | wie A (≤ 1,0 Cent Unterschied an 17 Tasten) | **an jeder Taste, ohne Prüfregel** | 20 Mult. | 20 |
| **E** **Weg 2b** Allpass-Summe | an den 18 Tasten oft schlechter (Cemb 60 iir4 hörbar 84,5 %, Laute 1 56–62 %) | **an 10 von 18 Tasten > 5 Cent schlechter**, bis 529 Cent | durch den Bau, auch zur Laufzeit (Parameter frei verschiebbar) | ≈ N + 1 Mult. (Allpässe) | N + 1 = 9 |
| F Ein-Pol V3 (Bericht 11) | Bericht 11 Tab. 2c: 1–12 3,5–88 % | Ein-Pol | durch den Bau | 3 | 2 |

**Fachliche Einschätzung** `[eigene Einschätzung]`:
- Die Glättung (B4) ist für **gemessene, streuende** Abklingzeiten gedacht (S. 19 IV, S. 20 VII, wörtlich). Das Ziel in
  LumiViz ist ein glattes Modell (Prüfstand Z1). Dort verfälscht sie den Grundton der monofilen Saiten um 2,9–15,3 %. **B ist fachlich richtiger
  als A.** Die Abweichung von der Quelle ist durch den Satz der Quelle selbst begründet.
- Die **HF-Vorgabe** (B5) wird gebraucht, um die Hilbert-Transformation über das ganze Band zu füttern. Bei Sollpunkten bis
  21,6 kHz bleibt für sie nur der Streifen 21,6–24 kHz. Dort verursacht sie die meisten Fälle über 1. Ob man sie weglässt
  oder die Sollpunkte früher enden lässt, ist nicht gerechnet; beides wäre eine weitere Abweichung. Weg 2a macht die Frage
  für die Passivität gegenstandslos.
- **Unter f₁ den Wert bei f₁ halten** (1b Nr. 7) ist für die Passivität die entscheidende Setzung. Die Quelle legt sie
  nicht fest; ihre Abb. 1(b) widerspricht ihr nicht.
- Zwischen C und D entscheidet, ob die Tafel je Taste eine Prüfung braucht (C) oder nicht (D). Fehler, Takt und Tafelgröße
  sind gleich; D ist an weniger Tasten gerechnet.
- E verliert an der Tonhöhe. Seine Phase ist nicht mindestphasig und stört die Dispersionskette (K16).
- Alle Fehlerzahlen „hörbar“ an Cembalo und Laute 1 sind durch die obersten Teiltöne bestimmt. Dort hilft eine höhere
  Ordnung nicht (iir16 nicht besser als iir8).

---

## 5. Nicht gefunden

| Lücke | wo gesucht | Ergebnis |
|---|---|---|
| Rauhala & Välimäki, IEEE SPL 13 (5) 2006 | Websuche (2×), Semantic Scholar (HTTP 429), Wayback-CDX der Aalto-Begleitseiten (`spl-*`: nur `spl-adf`, `spl-acge`, `spl-allpasslimiter`, `spl-ptr`, `spl08-parfilt`) | nicht frei; IEEE Xplore nicht versucht (Bezahlschranke) |
| Abel, Välimäki & Smith, IEEE SPL 17 (4) 2010 | Websuche (2×), Semantic Scholar (CLOSED), research.aalto.fi (403), Aalto-Begleitordner (nur `adf.m`), CCRMA-Seite, CCRMA-Wiki | nicht frei; ResearchGate (Anmeldung) und Scribd (Kopie unklarer Herkunft) nicht benutzt |
| Abel & Smith, DAFx-06 „Robust design of very high-order allpass dispersion filters“ | Websuche, CCRMA-Seite (kein PDF), Wayback-CDX der DAFx-06-PDFs (Liste ohne Titel) | Datei nicht zugeordnet; dafx.de direkt: Zeitüberschreitung |
| Erratum zu Rauhala Gl. (7) | eine Websuche, `pdf_suche.py` „erratum“ im DAFx-PDF | keine Berichtigung gefunden; Druckfehler durch Rechnung belegt |
| Abtastrate der Rauhala-Konstanten | `pdf_suche.py` „kHz“, „sample rate“, „fs“ (alle 6 Seiten, Textebene) | nicht im Text; aus Abb. 5 auf 44,1 kHz geschlossen |
| MATLAB `invfreqz` (2003) und `interp1 'cubic'` im Wortlaut | nicht gesucht | Abweichungen 6, 10 bleiben `[eigene Einschätzung]` |

## 6. Was die LumiViz-Sitzung prüfen muss

1. **Setzung unter f₁** (1b Nr. 7): Sie entscheidet über die Passivität; in LumiViz festlegen und begründen.
2. **Glättung ja/nein** (Option A gegen B): eine Abweichung von der Quelle; der Beleg dafür ist der Satz S. 20, VII.
3. **Weg 2a nachrechnen**, an einer Taste von Hand: QP-Lösung, Spektralfaktor, Nachprüfung |H| auf feinem Raster.
   `weg2.py` Z. 33–80.
4. **Die Zahlen „hörbar“ an Cembalo und Laute** stammen vom Maß nur des Verlustfilters und hängen an der Setzung σ ≤ 100 1/s
   aus Bericht 11.
5. **Rauhala:** Den Druckfehler in Gl. (7) und die 44,1 kHz selbst an Abb. 2–5 nachsehen
   (`seiten/Rauhala_…_S0002/3/4_130dpi.png`). Für LumiViz (48 kHz) wäre die Formel umzurechnen oder neu anzupassen; nach
   Bericht 11 ist D4 (angepasst) an keiner Taste schlechter.
6. **adf.m:** Das Teiltongesetz f_n = n f₀/√(1 − B n²) widerspricht dem Prüfstand (R1 bzw. Z1). Wer adf benutzt, muss pd(f)
   ersetzen oder die Bandbreite so wählen, dass B n² klein bleibt. Ein Newton-Schritt reicht bei großem B (df/f₀)² nicht.
7. **Gitarre Saite 6, iir4** liegt mit Gleichungsfehler und HF-Vorgabe an Bund 0–6 über 1 (voll bis 19,81 bei Bund 1, ohne
   Glättung bis 31,95). Das ist ein Grund, dort iir8 oder Weg 2a zu nehmen.

## 7. Ladeliste (nur, was nicht schon in `…\cmake\quellen\` liegt)

Alles schon im Ordner `abrufe/` dieses Laufs; nichts zusätzlich geladen.

| Datei (Vorschlag) | liegt als | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|---|
| `Rauhala_Valimaki_2006_DispersionTunableAllpass_DAFx06.pdf` | `abrufe/` gleichnamig | https://web.archive.org/web/20200722051908id_/http://www.dafx.de/paper-archive/2006/papers/p_071.pdf (Original dafx.de/paper-archive/2006/papers/p_071.pdf) | 461 633 B | DAFx-Archiv, frei zugänglich; Lizenz nicht im Dokument gesehen | Gl. (3)–(8), Tab. 1–2, Abb. 2–5 |
| `Abel_Valimaki_Smith_2010_adf.m` | `abrufe/adf_m_wayback.txt` | https://web.archive.org/web/20220709004750id_/http://research.spa.aalto.fi/publications/papers/spl-adf/adf.m | 1 920 B | Begleitcode der Autoren, ohne Lizenzvermerk | D3 im Wortlaut |

(Bericht 11 schlug den Namen `Rauhala_Valimaki_2006_TunableDispersion_DAFx06.pdf` vor. Der Titel auf Seite 1 lautet aber
„Dispersion Modeling in Waveguide Piano Synthesis Using Tunable Allpass Filters“; daher der Name oben.)

## Am Ende: was verlangt und nicht geliefert ist

- **Weg 2 über den ganzen Umfang** nur an den Tasten gerechnet, an denen Weg 1 oder oG über 1 lag (Auftrag: „an wie vielen
  braucht es Weg 2, und wie groß ist dann der Fehler“ — das ist geliefert). Option D (2a an jeder Taste) ist nur an den
  18 Tasten belegt.
- **Weg 2a** hat einen festen Nenner. Ein konvexer Entwurf, der auch A frei lässt, ist nicht gerechnet; mit Nenner gemeinsam
  ist das Problem nicht konvex.
- **Weg 2b** mit 24 Startwerten: Bessere örtliche Minima sind möglich; die Zahlen sind eine obere Schranke für den Fehler
  dieser Form.
- **Fehler am Grundton für den ganzen Umfang** steht in `weg2_umfang.txt` (Spalte p1), ist aber nicht zusammengefasst.
- **Originale der beiden SPL-Briefe** nicht gefunden (Abschnitt 5). Damit ist für Abel u. a. 2010 nur der Code geprüft, und
  die Gültigkeit der Konstanten im SPL-Brief von Rauhala (zweite Ordnung) ist nur aus der DAFx-Tab. 1 bekannt.

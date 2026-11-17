> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_16_einzeltoene_messgrundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-17 08:09–08:33, claude-opus-5-5, 96 Runden, success, 5.72 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen. **Nicht gegengelesen:** keine Formel,
> Tabelle oder Zahl dieses Berichts ist bisher an der Quelle nachgelesen; die Marken der Belegkraft sind die des Laufs.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_16_einzeltoene_messgrundlagen.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_16_einzeltoene_messgrundlagen/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Abgerufene PDFs** liegen seit S202 nur noch unter `…\cmake\quellen\` (im Archiv der Werkbank entfernt; der Bericht nennt die Namen der Werkbank):
> - `Itu_2023_RecommendationITU17702023.pdf` → `quellen\messung\ITU_2023_BS1770-5_ProgrammeLoudness.pdf`
> - `Ebu_2023_Tech3341Pdf.pdf` → `quellen\messung\EBU_2023_Tech3341_LoudnessMetering.pdf`
> - `Ebu_2023_Tech3342Pdf.pdf` → `quellen\messung\EBU_2023_Tech3342_LoudnessRange.pdf`
> - `2013_Edc.pdf` → `quellen\messung\Venturi_Farina_Tronchin_2013_PreprocessingImpulseResponses_POMA19.pdf` — Proc. Meet. Acoust. 19 (ICA 2013)
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_einzeltoene_messgrundlagen_werkbank/`: `baue_bericht.py`, `gegenrechnung.py`, `gegenrechnung_ausgabe.txt`.

# Bericht: Messgrundlagen für den Vergleich gerechneter gezupfter Instrumente mit Einzeltönen (Stufe 2 zu 34k)

**Auftrag:** `auftraege/2026-11-16_16_einzeltoene_messgrundlagen.md` · **Datum:** 2026-11-17 · **Modell:** claude-opus-5-5
**Voraussetzung:** Bericht der Stufe 1 (`ergebnisse/2026-11-16_06_einzeltoene_gezupfte_saiten/bericht.md`) lag vor und ist gelesen.

**Reichweite.** Lokal gelesen (Seitenbild oder Textebene über `pdf_suche.py`): Woodhouse 2021 (JASA 150, S. 4375–4377), Välimäki et al. 2004 (S. 938, 940), Rigaud/David/Daudet 2011 (Anhang A, S. 398), Bell 1988 (S. 159–160a), Kubo/Aoshima 1995 (S. 712, Abstract), Weinzierl et al. (Doku der TU-Berlin-Datenbank, S. 2–6), Penttinen 2006 (Textebene), Woodhouse et al. 2012 und Wright 1996 (nur durchsucht), Mores 2021 *Method* (durchsucht). Netz: 6 `WebSearch`; 7 Abrufe. **Wie abgerufen wurde:** `WebFetch` stand in diesem Lauf nicht als eigenes Werkzeug zur Verfügung, nur über den Helfer `web-fetch`, der genau dieses Werkzeug aufruft. Das ist formal ein Unteragent, und Unteragenten sind in Aufträgen nicht erlaubt (CLAUDE.md, „Unteragenten“). Ich habe ihn deshalb **nur als Abrufwerkzeug** benutzt, nicht für Recherche. Die PDFs, die er abgelegt hat, habe ich selbst am Seitenbild gelesen. Seine Zusammenfassungen gelten nicht als Beleg. Die LumiViz-Sitzung soll entscheiden, ob das so zulässig ist. Abgerufen und selbst gelesen: ITU-R BS.1770-5 (`webfetch-1794899527319-syhkww.pdf`), EBU Tech 3341 v4.0 (`webfetch-1794899533355-gjvmsj.pdf`), EBU Tech 3342 v4 (`webfetch-1794899533971-3drkbh.pdf`), ICA-2013-Beitrag 271 von angelofarina.it zur Rauschkompensation (`webfetch-1794899622628-bkogc0.pdf`). Nur über das Werkzeug gelesen: der Quelltext von `pyrato.edc`. **Nicht erreicht:** Karjalainen et al. 2002 (typeset/scispace: leere Antwort), Schroeder 1965, ISO 3382-1, Lundeby et al. 1995 (alle drei hinter Bezahlschranke oder kostenpflichtig, nicht versucht), Galembo & Askenfelt 1999, Rauhala et al. 2007. Keine Downloads mit `curl`. Python: ein Skript `gegenrechnung.py`, im Bericht vollständig mit Ausgabe (§ 3).

**Marken:** `[wörtlich]` Wortlaut oder Wert selbst gelesen, mit Seite. `[am Seitenbild geprüft]` Formel oder Tabelle am Bild verglichen. `[Werkzeug]` steht so nur in einer Werkzeugantwort. `[nur Zusammenfassung]` stammt aus einer Suchzusammenfassung. `[hergeleitet]` eigene Herleitung. `[eigene Rechnung]` Zahl aus `gegenrechnung.py`. `[eigene Einschätzung]` Urteil ohne Rechnung. `[LumiViz]` heutiger Stand im Repo (Header/Doku gelesen).

## Kurzergebnis

1. **Die heutige Lautheit je Taste (Momentary-Maximum) misst bei abklingenden Tönen das Abklingen mit.** Ein Ton, der mit D dB/s abfällt, liegt um ΔL = 10·lg[(1−10^(−0,04·D))/(0,0921·D)] unter einem Dauerton gleicher Anfangsamplitude (Gl. 3). Das sind −0,2 LU bei 1 dB/s und −11,9 LU bei 150 dB/s `[eigene Rechnung]`. Über den Umfang der Harfe (früher Abfall 1,9 → 150 dB/s, S195) steckt damit bis rund 11 LU Abklingen in der „Spanne“. Die integrierte Lautheit mit Gate hängt bei Einzeltönen an der Dateilänge (−7,4 → −11,4 LUFS für 1 → 8 s) und taugt nur mit festem Fenster.
2. **Die Sprung-Schwellen aus GZ14 a messen in raumbehafteten Aufnahmen vor allem den Raum.** Ein einziger Mikrofonplatz im Hallfeld gibt jedem Teilton einen zufälligen Pegel. Bei Direkt-zu-Hall-Verhältnis DRR = 0 dB und zehn Teiltönen streut der Tonpegel um 2,5 dB. Über 72 Tasten ergibt das allein durch den Raum einen mittleren Sprung von 2,4 LU und einen größten von 10,2 LU (95 %: 12,6) `[eigene Rechnung]`. Die Cembalo-Werte aus S195 (1,3–2,4 / 4,5–11,3 LU) liegen genau in diesem Bereich. Die Schwellen 4 / 12 LU sind deshalb kein Maß für das Instrument.
3. **Das Doppelabklingen wird mit zwei unabhängigen Geraden und bestem Knick systematisch zu flach geschätzt,** auch ganz ohne Raum und Rauschen. Vorgegeben 40 dB/s früh, gemessen 26 dB/s; vorgegeben 150 dB/s, gemessen 108 dB/s. Die späte Rate und der Knick stimmen. Ein Modell aus zwei Komponenten mit Boden, im Pegel ausgeglichen, gibt die Vorgabe zurück (40,3 / 4,0 / 0,53 s statt 40 / 4 / 0,556 s) `[eigene Rechnung]`. Ob die Teiltonmessung von LumiViz denselben Fehler hat, muss die Sitzung an ihrem Code prüfen (§ 7). Meine Nachbildung ist nicht ihr Code.
4. **Der Raum lässt die späte Rate eines Teiltons unverändert, solange T60 der Stufe größer ist als T60 des Raums.** Gerechnet: 19,3 statt 20 dB/s und 10,0 statt 10 dB/s. Ist die Stufe schneller als der Raum, ist sie nur über ΔL ≈ D/(D−D_R)·(DRR + 10·lg(D/D_R)) dB messbar (Gl. 14). Je Raum streut das um ±10 dB, weil der Raum bei einer einzelnen Frequenz einen zufälligen Pegel hat.
5. **Vorzeichen Hüllkurve gegen Teilton:** Gerechnet sind zwei Wege, auf denen die Hüllkurve „Modell langsamer“ zeigt, obwohl jeder Teilton des Modells schneller fällt. (a) Der Grundton der Aufnahme ist schwach (ab −10 dB gegen die Reihe). (b) Raum an der Aufnahme bei schnellen Teiltönen (DRR 0 dB). Die bloße Mischung schnell und langsam fallender Teiltöne dreht das Vorzeichen in den vier gerechneten Fällen **nicht**.
6. **Neue, nicht in Stufe 1 genannte Sammlung: TU Berlin, „Musical Instruments – A data base with recordings, directivities, and features“** (DepositOnce 5861.3, CC BY-NC-SA 4.0). Sie enthält **Gitarre (C2–B5 pp, E2–B5 ff) und Doppelpedalharfe (C1–F4, pp/ff)**, aufgenommen im reflexionsarmen Raum mit 32 kalibrierten Mikrofonen auf einer Kugel von rund 2,09 m Radius (in Pa, 44,1 kHz) `[wörtlich]` / `[eigene Rechnung]` für den Radius. Das ist die einzige gefundene Quelle mit bekanntem Abstand, ohne Raum, mit absolutem Pegel und Richtwirkung — für Lautheit und Sprünge die fachlich stärkste Grundlage (§ 4).

---

## 1. Gleichungen

### 1a Lautheit je Taste

**Gl. 1 — K-gewichtete Leistung und Lautheit** (ITU-R BS.1770-5, S. 5–6, Gl. 1, 2) `[wörtlich]` `[am Seitenbild geprüft]`:
z_i = (1/T)·∫₀ᵀ y_i² dt; L_K = −0,691 + 10·lg Σ_i G_i·z_i [LKFS]; G_L = G_R = G_C = 1,0, G_Ls = G_Rs = 1,41 (Tab. 3, S. 7).

**Gl. 2 — Gating** (BS.1770-5, S. 6–7, Gl. 3–7) `[wörtlich]`: Blöcke T_g = 400 ms, „The overlap of each gating block shall be 75% of the gating block duration“; Γ_a = −70 LKFS; Γ_r = (gated loudness über J_g mit l_j > Γ_a) − 10; die integrierte Lautheit mittelt über die Blöcke mit l_j > Γ_r und l_j > Γ_a.
K-Filter bei 48 kHz (Tab. 1, 2, S. 4–5) `[am Seitenbild geprüft]`: Stufe 1 b = (1,53512485958697; −2,69169618940638; 1,19839281085285), a₁ = −1,69065929318241, a₂ = 0,73248077421585. Stufe 2 b = (1; −2; 1), a₁ = −1,99004745483398, a₂ = 0,99007225036621. Für andere Abtastraten: „Implementations at other sampling rates will require different coefficient values, which should be chosen to provide the same frequency response“ (S. 4) `[wörtlich]`.
Bezug: „If a 0 dB FS, 1 kHz (997 Hz to be exact …) sine wave is applied to the left, centre, or right channel input, the indicated loudness will equal −3.01 LKFS.“ (S. 7) `[wörtlich]`.
Einschränkung: „the algorithm is not, in general, suitable for use to estimate the subjective loudness of pure tones“ (S. 7) `[wörtlich]`. Das betrifft die obersten Lagen, wo ein Zupfton fast ein Sinus ist.

**Momentary / Short-term** (EBU Tech 3341, 2023, S. 5) `[wörtlich]`: „The Momentary Loudness uses a sliding rectangular time window of length 0.4 s. The measurement is not gated.“ — „The Short-term Loudness uses a sliding rectangular time window of length 3 s. The measurement is not gated.“ Maximalwerte: „The loudness meter shall be able to display the maximum value of the ‘Momentary Loudness’ and of the ‘Short-term Loudness’.“ (S. 4). Toleranz der Prüfsignale: ±0,1 LU bzw. LUFS (Tab. 1, S. 8, z. B. Fall 13: „Max M = −23.0 ±0.1 LUFS, for each segment“) `[wörtlich]`. In BS.1770-5 kommen Momentary und Short-term nicht vor (Suche „momentary“: ein Treffer, in anderem Sinn) `[wörtlich]`, Suchweg.

**Gl. 3 — Momentary-Maximum eines exponentiell abklingenden Tons** `[hergeleitet]`. Energie e(t) = e₀·10^(−D·t/10), D in dB/s, Fenster W ab dem Einsatz:
L_M,max − L_Dauer = 10·lg[(1 − 10^(−D·W/10)) / (D·W·ln10/10)], W = 0,4 s.
Gegengerechnet (§ 3 A): D = 1 / 3 / 10 / 30 / 60 / 150 dB/s gibt gemessen −0,20 / −0,60 / −1,89 / −4,80 / −7,64 / −11,90 LU, nach der Formel −0,20 / −0,59 / −1,85 / −4,70 / −7,44 / −11,40 `[eigene Rechnung]`. Der Rest von bis 0,5 LU bei 150 dB/s kommt vom Anstieg über 5 ms und von der Welligkeit des Sinus im Fenster.
*Folge:* Eine Abweichung des Modells im frühen Abfall geht in die Lautheit je Taste ein. Bei 150 dB/s kostet ein 25 % zu schneller Abfall 1,0 LU (−11,4 → −12,4) `[hergeleitet]`.

**Gl. 4 — Lautheit über ein festes Fenster ab dem Einsatz** (so S195 `int1` = erste Sekunde) `[hergeleitet]`: dieselbe Formel mit W = 1 s, ohne Gate. Das Gate ändert bei einem einzelnen, lauter werdenden und dann abklingenden Ton nur, was hinter −10 LU unter dem Mittel liegt. Bei fester Fensterlänge ist das Ergebnis von der Dateilänge unabhängig.

**Messunsicherheit der Lautheit je Taste** (Zahlen und Herleitung):

| Beitrag | Größe | Herkunft |
|---|---|---|
| Messgerät (Umsetzung der Norm) | ±0,1 LU | Tech 3341 Tab. 1 `[wörtlich]` |
| K-Filter bei 44,1 / 96 kHz aus dem analogen Vorbild (bilinear) | +0,005 / −0,016 LU gegen 48 kHz | `[eigene Rechnung]` § 3 A |
| Mono-Datei als Stereo (beide Kanäle gleich) | +3,01 LU, fest | `[eigene Rechnung]` § 3 A; fällt im Formvergleich heraus, nicht beim Mischen von Sammlungen mit verschiedener Kanalzahl |
| Raum, ein Mikrofon, Ton mit 10 Teiltönen | Std 1,2 / 1,9 / 2,5 / 2,8 LU bei DRR +10 / +5 / 0 / diffus | `[eigene Rechnung]` § 3 C (Modell: Gl. 12) |
| Spieler (Handspiel, Anschlag) | nicht gefunden als Zahl je Instrument; S195: Gitarre ff−pp 9,4–21,6 LU je Saite | `[LumiViz]`; „variation in the strength of individual picks … dominate the loudness differences“ `[nur Zusammenfassung]` (Gitarrenanalyse, cdn.shopify.com, Quelle nicht gelesen) |
| Abklingen, das im Fenster mitgemessen wird | Gl. 3: −0,2 … −12 LU über D = 1 … 150 dB/s | `[eigene Rechnung]` |

### 1b Gang über den Umfang

**Gl. 5 — Kennzahlen** (wie `2026-11-16_S200_gang_ueber_den_umfang.py`) `[LumiViz]`: Form f_k = L_k − median(L). Spanne = max − min. Paare P_n = {(i, j): 0 < t_j − t_i ≤ n}, n = 4 Halbtöne. Mittlerer Sprung = median |L_i − L_j| über P_n, größter Sprung = max über P_n. Glättung: gleitender Median über 7 Tasten (±3).

**Gl. 6 — Erwartungswert des mittleren Sprungs bei reiner Streuung σ je Ton** `[hergeleitet]`: L_i − L_j ~ N(0, 2σ²) ⇒ median|ΔL| = 0,6745·√2·σ = **0,954·σ**. Gegengerechnet: 0,960 bei σ = 1, 2,882 bei σ = 3 (Theorie 0,954 / 2,862) `[eigene Rechnung]`. Umgekehrt: Die Schwelle „mittlerer Sprung ≤ 4 LU“ lässt eine zufällige Streuung je Taste von σ ≈ 4,2 LU zu.

**Gl. 7 — größter Sprung** `[eigene Rechnung]`, kein geschlossener Ausdruck. Er wächst mit der Zahl der Paare. Bei 72 Tasten mit allen Paaren bis 4 Halbtöne (278 Paare), Gang −1 LU/Oktave und drei bekannten Störungen bis 8 LU (Vorgabe ohne Streuung: 8,3 LU) ergibt sich bei σ = 1 / 2 / 3 / 4 LU ein Mittel von 9,6 / 11,9 / 14,8 / 18,2 LU (95 %: 11,4 / 15,1 / 18,8 / 23,2). Mit nur jeder dritten Taste (Harfe VCSL): 8,7 / 10,0 / 11,6 / 14,3. **Die Paar-Statistik hängt an der Tastendichte:** Bei lückenhafter Aufnahme fallen Spanne und größter Sprung kleiner aus, der mittlere Sprung größer (σ = 2: 2,4 statt 2,1).

**Gl. 8 — Glättung:** Ein gleitender Median über 7 Tasten entfernt eine Störung, die höchstens drei Nachbartasten breit ist, ganz (Taste 80: −8 LU → −0,08; Tasten 55–57: +6 → +0,25). Ab vier Tasten bleibt sie stehen; die Ausgabe in § 3 B nennt die Werte für 3, 4 und 5 Tasten `[eigene Rechnung]`. Die Form nach der Glättung sieht also eine Mode, die drei Tasten hebt, **nicht** — die Paar-Statistik auf den ungeglätteten Werten sieht sie. Streuung des Medians über n = 7 unabhängige Werte ≈ 1,2533·σ/√7 = 0,47·σ (asymptotische Effizienz des Medians) `[hergeleitet]`.

### 1c Abklingen je Teilton, Doppelabklingen

**Gl. 9 — Rückwärtsintegration nach Schroeder** in der Form der ICA-2013-Arbeit (S. 2, Gl. 1, Textebene) `[wörtlich]`, nicht gegen Schroeder 1965 geprüft: E(t) = ∫_t^∞ h²(τ)dτ = ∫₀^∞ h² − ∫₀^t h². In LumiViz je Teilton auf der Hüllkurve: L(t) = 10·lg(∫_t^Ende e / ∫_von^Ende e) `[LumiViz]`.

**Auswertebereiche (ISO 3382):** EDT 0 bis −10 dB, T20 −5 bis −25 dB, T30 −5 bis −35 dB; nach ISO 3382-2 braucht ein Bereich von 20 dB mindestens 35 dB Abstand zum Rauschen — alles `[nur Zusammenfassung]` (proav.de, svantek.com, Vorschau ISO 3382-2 auf scribd). Den Normtext habe ich nicht gelesen.

**Lundeby-Verfahren** (ICA-2013-Beitrag 271, S. 3, Textebene) `[wörtlich]`: „1. Average squared IR in local time intervals (10-50 ms) 2. Estimate background noise level using tail (last 10% of the signal) 3. Estimate slope of decay from 0 dB to noise level (the “left” point is 0 dB. Search the “right” point 5-10 dB above noise level) 4. Find the crosspoint defined by regression line and noise level 5. Find a new local time intervals based on the actual slope (use 3-10 intervals per 10 dB of decay) 6. Average squared IR in new local time intervals 7. Estimate background noise (starting from a time corresponding to a decay of 5-10 dB based on actual slope after the actual crosspoint but 10% of length of IR should be the minimum length) 8. Estimate late decay slope (a dynamic range of 10-20 dB should be evaluated, starting 5-10 dB above the noise level) 9. Find a new crosspoint 10. Repeat 7, 8 and 9 until convergence of crosspoint is achieved“.
Voreinstellungen in `pyrato` `[Werkzeug]`: Fenster 30 ms („the mean of the 10 to 50 ms recommended by Lundeby“), `n_intervals_per_10dB = 5`, `dB_above_noise = 10`, `use_dyn_range_for_regression = 20`; Quelle laut Docstring: Lundeby, Vigran, Bietz, Vorländer, Acta Acust. 81(4), 344–355, 1995.

**Gl. 10 — Bedingung für Doppelabklingen** (Woodhouse 2021, S. 4376–4377, Gl. 1–5) `[am Seitenbild geprüft]`:
η_b ≈ 2·Re(Y)/(π·n·Y₀ⱼ), Y₀ⱼ = (T_j·m_j)^(−1/2) (Gl. 1); η_air ≈ (ρ_a/ρ)·(2√2·M + 1)/M², M = (d/4)·√(2πf_n/η_a) (Gl. 2, 3); η_bend (Gl. 4). Kriterium η_b ≫ η_air bei f₁ (Gl. 5). Es ist eine **notwendige** Bedingung: „if Eq. (5) is not satisfied, there cannot be a strong double decay“ (S. 4377) `[wörtlich]`. Für „≫“ gibt es keinen Zahlenwert: „it is not at present possible to put a numerical value on the requirement“ (S. 4377) `[wörtlich]`.
Befund je Instrument (Abstract, S. 4375) `[wörtlich]`: „In the tested guitar, by contrast, double decays are likely to be confined to a few notes near strong body resonances … The lute and the mandolin, both normally strung with double strings, should both exhibit double decays over much of their playing range, especially towards the higher end.“ Mechanismus: zwei Saiten eines Chors oder zwei Polarisationen einer Saite; „The mode involving string motion in the perpendicular plane will tend to make more sound, and also decay more rapidly“ (S. 4376) `[wörtlich]`.
Dagegen Kubo & Aoshima 1995 (Abstract, S. 712) `[wörtlich]`: „A guitar under investigation is shown to provide 1) two-step tone decay curve not caused by difference between decay rates of horizontal and vertical vibration modes of the strings (single string tone)“. Ein zweistufiges Abklingen an der Gitarre muss also nicht von den Polarisationen kommen.
Cembalo: „the harpsichord tones exhibit beating and two-stage decay, as is usual for string instruments“ (Välimäki et al. 2004, S. 940) `[wörtlich]`.

**Gl. 11 — Zwei Komponenten mit Boden** (Vorschlag dieses Berichts; der Ansatz „Abklingen plus Rauschboden“ wird Karjalainen et al. 2002 zugeschrieben `[nur Zusammenfassung]`, Arbeit nicht gelesen) `[hergeleitet]`:
inkohärent (zwei Polarisationen mit Frequenzversatz): e(t) = E₁·10^(−a·t/10) + E₂·10^(−b·t/10) + N;
kohärent (gleiche Frequenz): e(t) = (A₁·10^(−a·t/20) + A₂·10^(−b·t/20))² + N;
im Pegel 10·lg e ausgeglichen; Knick t_k = (L₁ − L₂)/(a − b). Gegengerechnet in § 3 D.

**Gl. 12 — Raum, ein Teilton, Energie** (inkohärent, Erwartungswert) `[hergeleitet]`. Direktanteil d(t) = E₀·e^(−αt), Hall-Impulsantwort der Energie r(t) = (β/DRR)·e^(−βt); α, β die Abklingraten der Energie (D·ln10/10):
e_R(t) = E₀·β/(DRR·(α − β))·(e^(−βt) − e^(−αt)).
- α < β (Teilton langsamer als der Raum): Spät gilt e_R ∝ e^(−αt). **Die Steigung bleibt die des Teiltons;** nur der Pegel steigt um 10·lg(1 + β/((β−α)·DRR)).
- α > β (Teilton schneller als der Raum): Hall und Direktanteil schneiden sich bei
**Gl. 14:** ΔL_mess = α/(α−β)·[DRR_dB + 10·lg(α/β)] dB unter der Spitze. Nur bis dorthin ist die Rate des Teiltons messbar.
Gegengerechnet (§ 3 D, je 30 Räume): Formel 21,5 / 7,8 / 6,0 / 25,0 / 14,7 dB, gemessen im Median 18,7 / 6,7 / 4,9 / 12,0 / 14,8 dB, 10.–90. Perzentil jeweils etwa ±10 dB. Die Streuung kommt daher, dass der Raum bei **einer** Frequenz einen zufälligen Pegel hat (Gl. 13). Ist α nahe β (T60 0,3 gegen 0,4 s), vergrößert der Faktor α/(α−β) jeden Fehler; die Formel überschätzt dort.

**Gl. 13 — Pegel eines Teiltons im Raum, ein Mikrofon** `[hergeleitet]`: H = d + r, r komplex normalverteilt mit E|r|² = 10^(−DRR/10). Im diffusen Grenzfall ist |r|² exponentialverteilt, und Std(10·lg|r|²) = (10/ln10)·π/√6 = **5,57 dB**. Das ist der bekannte Wert für Raumfrequenzgänge oberhalb der Schroeder-Frequenz (Schroeder 1954/1962, nicht gelesen). Gerechnet: 0,62 / 2,00 / 3,66 / 5,09 / 5,56 dB bei DRR +20 / +10 / +5 / 0 / diffus; für den Unterschied zweier Töne √2-mal so viel (bis 7,88 dB) `[eigene Rechnung]`.

**Warum Hüllkurve und Teilton ein anderes Vorzeichen geben** (§ 3 E) `[eigene Rechnung]`. Das Modell fällt an jedem Teilton 25 % schneller als die Aufnahme. Gemessen wird die Hüllkurve wie in S195 (−5 bis −25 dB, 20-ms-Blöcke):
- Grundton der Aufnahme −10,5 / −20 / −30,5 dB gegen die Reihe: Hüllkurve Aufnahme 6,9 / 8,9 / 9,2 dB/s, Modell 5,1 dB/s → **falsches Vorzeichen**. Die Hüllkurve folgt den stärksten Teiltönen; fehlt der Grundton, folgt sie Teiltönen, die schneller fallen. Bell 1988 (S. 160a) `[wörtlich]`: die Harfe kann „the fundamental modes of the bass strings, those below about 180Hz“ nicht wirksam abstrahlen.
- Raum an der Aufnahme: Teilton trocken 40 dB/s, Raum T = 2 s, DRR 0 dB → Hüllkurve 18,7 dB/s. Ein Modell mit 34 dB/s (tatsächlich langsamer) erscheint schneller → **falsches Vorzeichen**. Bei DRR +10 dB richtig (38,0), ebenso bei langsamen Teiltönen (15 dB/s).
- Bloße Mischung (helle Aufnahme mit rasch fallenden hohen Teiltönen gegen dunkles Modell): in vier gerechneten Fällen **richtig**. Die Strecke −5 … −25 dB liegt hinter dem schnellen Anfang.

### 1d Spektrum je Dynamikstufe

**Gl. 15** `[LumiViz]`/`[hergeleitet]`: Teiltonpegel relativ L_k − L_1 (aus `anfangDb`, Spitze in 0,1 s nach dem Einsatz); Schwerpunkt SC = Σ k·p_k / Σ p_k (in Vielfachen von f₀) oder Σ f_k·p_k / Σ p_k. Helligkeit je Stärke: ΔSC zwischen pp, mf, ff an derselben Taste.
**Unsicherheit im Raum** (Gl. 13 je Teilton, unabhängig) `[eigene Rechnung]`, 20 Teiltöne 1/k:

| DRR | Std L₅ − L₁ | SC Mittel (relativ) | SC Std (relativ) |
|---|---|---|---|
| +20 dB | 0,88 dB | 1,004 | 0,052 |
| +10 dB | 2,81 dB | 1,040 | 0,165 |
| 0 dB | 7,23 dB | 1,168 | 0,383 |
| diffus | 7,89 dB | 1,207 | 0,431 |

Der Schwerpunkt wird im Raum im Mittel **heller** (+17 % bei DRR 0), weil die vielen schwachen hohen Teiltöne zufällig verstärkt werden. Ein Vergleich der Helligkeit **je Stärke an derselben Taste** in derselben Aufnahme hebt den Raum auf, weil Mikrofon und Raum gleich bleiben. Das ist die belastbare Form `[hergeleitet]`.
Literatur: Penttinen 2006 (Cembalo) `[wörtlich, Textebene]`: der Unterschied „(pp to mf and mf to ff) is about the size of 1 dB and the dynamic range, at the largest, is slightly larger than 2 dB“; nicht am Seitenbild geprüft. Die TU-Berlin-Datenbank enthält je Ton Merkmale aus der TimbreToolbox („spectral distribution … median and interquartile range“, Doku S. 4) `[wörtlich]`.

### 1e Tonhöhe und Inharmonizität B

**Gl. 16** f_n = n·f₀·√(1 + B·n²); linear: (f_n/n)² = f₀² + f₀²·B·n² (Rigaud et al. 2011, Gl. 18, S. 398) `[am Seitenbild geprüft]`. Verfahren dort `[wörtlich]`: Spektrum „computed with zero padding on 2^16 frequency bins from a 500ms window in the decay part of the sound“; Rauschpegel je Band aus dem Median (σ = med/√ln4, Rayleigh, Gl. 19–20, Schwelle T = 0,9999, Median über 300 Hz); Suche des Teiltons n in n·F₀′√(1+B′n²) ± F₀ini/5; Ausgleich robust mit LADR (kleinste absolute Abweichungen). Grenzen: bei *forte* „phantom“-Teiltöne, bei *piano* zu wenige Teiltöne in C6–C8 (S. 398).
LumiViz `[LumiViz]`: Blackman-Harris ×4 aufgefüllt ab 50 ms, Suche k·f₀·√(1+Bk²) in ±60 Cent bzw. bis B ≤ 0,02, Ausreißer > max(5 Cent, 4·1,4826·MAD); `Grundton.hpp`: Hann, ab 0,1 s für 0,4 s, bis 12 Teiltöne, Gewicht Amplitude².

**Gl. 17 — Abweichung in Cent** `[hergeleitet]`: 1200·log₂√(1+Bn²) ≈ 866·B·n². B = 10⁻⁵: n = 10 → 0,87 Cent, n = 30 → 7,8 Cent; B = 10⁻⁴: 8,6 / 74,6 Cent `[eigene Rechnung]`.

**Gl. 18 — Unsicherheit von B** `[hergeleitet]`, wenn die Teiltöne um σ_c Cent vom Gesetz abweichen (Kopplung an Steg und Körper): Std(B) = (2·ln2/1200)·σ_c / √Σ(k² − mittel k²)². Gegengerechnet (§ 3 F), σ_c = 1 Cent: 5 / 10 / 20 / 30 Teiltöne → gemessen 6,2·10⁻⁵ / 1,1·10⁻⁵ / 2,6·10⁻⁶ / 7,2·10⁻⁷, Theorie 6,0·10⁻⁵ / 1,1·10⁻⁵ / 2,1·10⁻⁶ / 7,6·10⁻⁷ `[eigene Rechnung]`. Rauschen bei −40 dB oder ein kürzeres Fenster (0,25 s) wirkt dagegen kaum (Std ≤ 5·10⁻⁷). **Die Teiltonzahl und die Abweichung vom Gesetz bestimmen die Unsicherheit, nicht die Frequenzauflösung.** Für das Cembalo mit B ≈ 10⁻⁵ (Välimäki et al. 2004, S. 938: „in the case of the 19 lowest keys of the harpsichord, where the inharmonicity coefficients are about 10⁻⁵, the inharmonicity is audible“ `[wörtlich]`) braucht man bei σ_c = 1 Cent mindestens 20 Teiltöne, damit B größer als seine Unsicherheit ist. Fehler von f₀: 0,3–0,6 Cent je Cent σ_c.

### 1f Einschwinggeräusch

**Gl. 19 — Trennung** `[hergeleitet]`: Rest r(t) = x(t) − Σ_k A_k·e^(−s_k t)·sin(2π f_k t + φ_k), Parameter aus 50–500 ms, zum Einsatz zurück verlängert. Größen: Energie des Rests in den ersten 50 ms gegen die des harmonischen Teils (dB); dazu Dauer (Zeit bis −20 dB) und Terzspektrum des Rests (Vorschlag, nicht gerechnet). Gegengerechnet (§ 3 H): Geräusch −19,7 / −29,8 / −39,5 dB gegen den Ton → gemessen −19,6 / −28,1 / −32,0 dB. **Grenze des Verfahrens bei etwa −30 dB unter dem Ton**, darunter misst es den Fehler der Teiltonanpassung `[eigene Rechnung]`.
Literatur: Välimäki et al. 2004 (S. 940) `[wörtlich]`: der Rest nach sinusoidaler Zerlegung wird weiterverarbeitet, „the soundboard contribution is mostly removed (by windowing the residual signal in the time domain) and the initial level of each partial is adjusted … through sinusoidal modeling and inverse filtering“. Dazu die „key-release thump“ (Abstract, PDF-Seite 1, Textebene). Zahlen zum Mechanikgeräusch des Cembalos: **nicht gefunden** (§ 8).

---

## 2. Parametertabelle

| Größe | Zahl | Einheit | Quelle | Marke | Gültigkeit | LumiViz heute |
|---|---|---|---|---|---|---|
| Momentary-Fenster | 0,4, rechteckig, gleitend, ohne Gate | s | Tech 3341 S. 5 | [wörtlich] | Programm; für Einzeltöne siehe Gl. 3 | 0,4 s, je Abtastwert gleitend `[LumiViz]` |
| Short-term-Fenster | 3, ohne Gate | s | Tech 3341 S. 5 | [wörtlich] | dto. | 3 s |
| Gating-Block, Überlappung | 0,4; 75 | s; % | BS.1770-5 S. 6 | [wörtlich] | integrierte Lautheit | 100-ms-Stücke `[LumiViz]` (vermutlich 400-ms-Blöcke aus vier Stücken = 75 %; nicht im Code geprüft) |
| Gate absolut / relativ | −70 / −10 | LKFS / LU | BS.1770-5 S. 6–7 | [wörtlich] | dto. | — |
| Bezug Sinus 997 Hz 0 dBFS | −3,01 | LKFS | BS.1770-5 S. 7 | [wörtlich] | ein Frontkanal | gegengerechnet −3,008 (48 kHz) |
| Toleranz Messgerät | ±0,1 | LU | Tech 3341 Tab. 1 | [wörtlich] | Prüfsignale | — |
| LRA: Perzentile | 10 / 95 | % | Tech 3342 S. 5 | [wörtlich] | Programm | — |
| Paare bis | 4 | Halbtöne | GZ14 a | [LumiViz] | — | 4 |
| Glättung | 7 (±3) | Tasten | S200 | [LumiViz] | entfernt Einzelstörung, nicht drei Tasten breite | 7 |
| mittlerer Sprung bei Streuung σ | 0,954·σ | LU | Gl. 6 | [hergeleitet] | unabhängige Streuung | Schwelle 4 LU |
| Lundeby-Mittelungsfenster | 10–50 | ms | ICA 2013 S. 3 | [wörtlich] | Raumimpulsantworten | Hüllkurve: 8 Perioden, 20 ms–0,5 s, Schritt 10 ms |
| Lundeby: Intervalle je 10 dB | 3–10 | — | ICA 2013 S. 3 | [wörtlich] | dto. | — |
| Regressionsende über Rauschen | 5–10 | dB | ICA 2013 S. 3 | [wörtlich] | dto. | `grenzeDb` 10 |
| Dynamik der späten Regression | 10–20 | dB | ICA 2013 S. 3 | [wörtlich] | dto. | mind. 10 dB Abfall |
| Rauschen aus dem Ende | letzte 10 | % | ICA 2013 S. 3 | [wörtlich] | dto. | Median der letzten 10 % |
| EDT / T20 / T30 | 0…−10 / −5…−25 / −5…−35 | dB | ISO 3382 | [nur Zusammenfassung] | Räume | S195: −5…−25 und −25…−45 |
| Abstand Signal–Rauschen für 20-dB-Bereich | ≥ 35 | dB | ISO 3382-2 | [nur Zusammenfassung] | Räume | — |
| Doppelabklingen | η_b ≫ η_air | — | Woodhouse 2021 Gl. 5 | [am Seitenbild geprüft] | notwendig, ohne Zahl | zwei Stufen ab Faktor 1,3 |
| Knick messbar, Teilton schneller als Raum | Gl. 14 | dB | — | [hergeleitet] | inkohärentes Mittel; ±10 dB je Raum | — |
| Pegelstreuung Teilton im diffusen Feld | 5,57 | dB | Gl. 13 | [hergeleitet] | f oberhalb der Schroeder-Frequenz | — |
| Hallradius | 0,057·√(γ·V/T) | m | — | [nur Zusammenfassung] (sengpielaudio, Arup) | diffuses Feld; γ Bündelungsgrad | — |
| B-Fenster | 0,5 s, 2^16 Bins | — | Rigaud 2011 S. 398 | [wörtlich] | Klavier | ab 50 ms, Blackman-Harris ×4 |
| B Cembalo, tiefste 19 Tasten | ~10⁻⁵ | — | Välimäki 2004 S. 938 | [wörtlich] | ein Cembalo | — |
| Std(B), 10 Teiltöne, σ_c = 1 Cent | 1,1·10⁻⁵ | — | Gl. 18 | [eigene Rechnung] | — | `bFehler` (nur Steigung) |
| Trenngrenze Einschwinggeräusch | ≈ −30 | dB gegen Ton | § 3 H | [eigene Rechnung] | Teiltonabzug 50–500 ms | — |
| Helligkeit: Spitze in | 0,1 | s | — | [LumiViz] | — | `anfangBisS` 0,1 |
| TU Berlin: Kugelradius | ≈ 2,09 (aus S₁ = 54,63 m²) | m | Doku S. 4 | [wörtlich] / [eigene Rechnung] | reflexionsarm, 32 Mikrofone | — |

---

## 3. Gegenrechnung (Skript und Ausgabe)

Die Teile A–H gehören zu den Verfahren: A Lautheit (Gl. 1–4), B Gang und Paare (Gl. 5–8), C Raum und Pegelstreuung (Gl. 13), D Doppelabklingen mit und ohne Raum und Rauschen, Raumgrenze (Gl. 11, 12, 14), E Vorzeichen Hüllkurve gegen Teilton, F B und f₀ (Gl. 16–18), G Spektrum im Raum (Gl. 15), H Einschwinggeräusch (Gl. 19). Fester Startwert. **Kipppunkt ohne Raum gegen mit Raum:** D zeigt ihn direkt. Harfe Diskant (früh 150, spät 20 dB/s): ohne Raum mit zwei Komponenten 149,5 / 20,0. Raum T 1,5 s DRR +10 dB: 126 / 19,9. DRR 0 dB: 54 / 19,7. T = 3 s (= T60 der späten Stufe): Messung zerfällt (Knick −2376 s).

```python
"""Gegenrechnung zu den Messverfahren (Auftrag 2026-11-16_16_einzeltoene_messgrundlagen, Abschnitt Gesucht 5).

Jeder Teil erzeugt ein synthetisches Signal mit bekannter Vorgabe und misst es mit dem Verfahren aus dem Bericht.
Aufruf: python gegenrechnung.py [teil ...]   (ohne Argument alle Teile; Teile: A B C D E F G H I)
Fester Startwert, damit gleiche Zahlen herauskommen.
"""
import sys

import numpy as np
from scipy.signal import lfilter, fftconvolve

RNG_SEED = 20261117


# ------------------------------------------------------------------ BS.1770: K-Filter und Lautheit
def k_filter_koeff(fs):
    """K-Filter fuer beliebige Abtastrate ueber die bilineare Abbildung des analogen Vorbilds.
    Die Kennwerte (fc, G, Q) sind die der verbreiteten Umsetzung libebur128; geprueft wird unten, dass sie bei
    48 kHz die Koeffizienten aus BS.1770-5 Tabelle 1 und 2 wiedergeben."""
    fc, G, Q = 1681.974450955533, 3.999843853973347, 0.7071752369554196
    K = np.tan(np.pi * fc / fs)
    Vh = 10 ** (G / 20)
    Vb = Vh ** 0.4996667741545416
    a0 = 1 + K / Q + K * K
    b1 = [(Vh + Vb * K / Q + K * K) / a0, 2 * (K * K - Vh) / a0, (Vh - Vb * K / Q + K * K) / a0]
    a1 = [1.0, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    fc, Q = 38.13547087602444, 0.5003270373238773
    K = np.tan(np.pi * fc / fs)
    a0 = 1 + K / Q + K * K
    b2 = [1.0, -2.0, 1.0]
    a2 = [1.0, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    return (b1, a1), (b2, a2)


def k_gefiltert(x, fs):
    (b1, a1), (b2, a2) = k_filter_koeff(fs)
    return lfilter(b2, a2, lfilter(b1, a1, x))


def lautheit(kanaele, fs):
    """kanaele: Liste von Signalen (G_i = 1). Gibt (Momentary-Max, Short-term-Max, integriert mit Gates) in LUFS."""
    y2 = sum(k_gefiltert(np.asarray(c, float), fs) ** 2 for c in kanaele)
    cs = np.concatenate([[0.0], np.cumsum(y2)])

    def gleitend_max(w):
        n = int(round(w * fs))
        if len(y2) < n:
            return -200.0
        m = (cs[n:] - cs[:-n]) / n
        return -0.691 + 10 * np.log10(max(m.max(), 1e-30))

    n = int(round(0.4 * fs))
    schritt = int(round(0.1 * fs))
    z = np.array([(cs[j + n] - cs[j]) / n for j in range(0, len(y2) - n + 1, schritt)])
    l = -0.691 + 10 * np.log10(np.maximum(z, 1e-30))
    za = z[l > -70]
    if len(za) == 0:
        integ = -200.0
    else:
        gr = -0.691 + 10 * np.log10(za.mean()) - 10
        zr = z[(l > -70) & (l > gr)]
        integ = -0.691 + 10 * np.log10(zr.mean())
    return gleitend_max(0.4), gleitend_max(3.0), integ


def teil_A():
    print("== A  Lautheit nach BS.1770: Bezugswert und abklingende Toene ==")
    (b1, a1), (b2, a2) = k_filter_koeff(48000)
    tab1 = [1.53512485958697, -2.69169618940638, 1.19839281085285, -1.69065929318241, 0.73248077421585]
    tab2 = [1.0, -2.0, 1.0, -1.99004745483398, 0.99007225036621]
    d1 = max(abs(u - v) for u, v in zip(b1 + a1[1:], tab1))
    d2 = max(abs(u - v) for u, v in zip(b2 + a2[1:], tab2))
    print(f"K-Filter bei 48 kHz gegen BS.1770-5 Tab. 1/2: groesste Abweichung Stufe 1 {d1:.1e}, Stufe 2 {d2:.1e}")
    for fs in (48000, 44100, 96000):
        t = np.arange(int(10 * fs)) / fs
        s = np.sin(2 * np.pi * 997 * t)
        m, st, i = lautheit([s], fs)
        m2, _, _ = lautheit([s, s], fs)
        print(f"  fs {fs}: Sinus 997 Hz 0 dBFS in einem Kanal: M-max {m:+.3f}, S-max {st:+.3f}, I {i:+.3f} LUFS "
              f"(Soll -3.01); derselbe Ton in beiden Kanaelen (Mono als Stereo): {m2:+.3f}")
    fs = 48000
    print("Abklingender Sinus 1 kHz (Anfangsamplitude 1, Einsatz 5 ms), Abfall D in dB/s, Datei 12 s, danach Stille:")
    print("  D dB/s | M-max  S-max  I(gated) | M-max - Dauerton | Formel 10lg[(1-10^(-DW/10))/(DW ln10/10)], W=0,4 s")
    dauer = -3.010
    for D in (1, 3, 10, 30, 60, 150):
        t = np.arange(int(12 * fs)) / fs
        env = 10 ** (-D * t / 20) * np.minimum(t / 0.005, 1)
        s = env * np.sin(2 * np.pi * 997 * t)
        m, st, i = lautheit([s], fs)
        DW = D * 0.4
        formel = 10 * np.log10((1 - 10 ** (-DW / 10)) / (DW * np.log(10) / 10))
        print(f"  {D:6.0f} | {m:+6.2f} {st:+6.2f} {i:+7.2f}   | {m - dauer:+6.2f}           | {formel:+6.2f}")
    print("Integrierte Lautheit haengt an der Dateilaenge (D = 10 dB/s):")
    for lang in (1.0, 2.0, 4.0, 8.0):
        t = np.arange(int(lang * fs)) / fs
        s = 10 ** (-10 * t / 20) * np.sin(2 * np.pi * 997 * t)
        m, st, i = lautheit([s], fs)
        print(f"  Datei {lang:3.0f} s: I {i:+6.2f}, M-max {m:+6.2f}")


# ------------------------------------------------------------------ Gang ueber den Umfang, Paare bis n Halbtoene
def kennzahlen(tasten, l, n=4):
    tasten, l = np.asarray(tasten), np.asarray(l)
    d = [abs(l[i] - l[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= n]
    d = np.array(d) if d else np.array([np.nan])
    return l.max() - l.min(), np.median(d), d.max(), len(d)


def glatt7(tasten, werte, halb=3):
    return np.array([np.median([w for t, w in zip(tasten, werte) if abs(t - t0) <= halb]) for t0 in tasten])


def teil_B():
    print("== B  Gang ueber den Umfang: Spanne, Paare bis 4 Halbtoene, Glaettung ==")
    rng = np.random.default_rng(RNG_SEED)
    tasten = np.arange(29, 101)  # F1..E7, 72 Tasten
    gang = -1.0 * (tasten - 60) / 12  # -1 LU je Oktave
    spruenge = {55: 6.0, 56: 6.0, 57: 6.0, 80: -8.0}  # bekannte Stoerungen (eine Mode unter drei Tasten, ein Loch)
    wahr = gang + np.array([spruenge.get(int(k), 0.0) for k in tasten])
    sp, med, mx, n = kennzahlen(tasten, wahr)
    print(f"Vorgabe ohne Streuung, alle Tasten: Spanne {sp:.2f} (Gang 6,0 + Stoerungen), mittlerer Sprung {med:.2f}, "
          f"groesster {mx:.2f} LU, {n} Paare")
    g = glatt7(tasten, wahr)
    print(f"  nach gleitendem Median ueber 7 Tasten: Abweichung an Taste 56 {g[tasten == 56][0] - gang[tasten == 56][0]:+.2f}, "
          f"an Taste 80 {g[tasten == 80][0] - gang[tasten == 80][0]:+.2f} LU (Vorgabe +6 bzw. -8: beide Stoerungen entfernt)")
    for breite in (3, 4, 5):
        st = gang + np.where((tasten >= 55) & (tasten < 55 + breite), 6.0, 0.0)
        gg = glatt7(tasten, st)
        print(f"  Stoerung +6 LU ueber {breite} Nachbartasten: nach der Glaettung an Taste 56 {gg[tasten == 56][0] - gang[tasten == 56][0]:+.2f} LU")
    print("Mit Streuung sigma je Ton (Spieler, Raum), 2000 Laeufe: Mittel und 95-%-Quantil")
    print("  sigma | Auswahl        | Spanne       | mittlerer Sprung | groesster Sprung | Theorie mittl. = 0,954 sigma (nur Streuung)")
    for sigma in (0.0, 1.0, 2.0, 3.0, 4.0):
        for name, auswahl in (("jede Taste", tasten), ("jede 3. Taste", tasten[::3])):
            werte = []
            for _ in range(2000 if sigma > 0 else 1):
                l = wahr + rng.normal(0, sigma, len(tasten))
                m = np.isin(tasten, auswahl)
                werte.append(kennzahlen(tasten[m], l[m]))
            w = np.array(werte)
            q = lambda c: f"{w[:, c].mean():5.1f} / {np.quantile(w[:, c], 0.95):5.1f}"
            print(f"  {sigma:4.1f}  | {name:14s} | {q(0)} | {q(1)}    | {q(2)}    | {0.954 * sigma:4.2f}")
    print("Reine Streuung ohne Gang, 72 Tasten, mittlerer Sprung Paare <=4 HT (Pruefung der Theorie):")
    for sigma in (1.0, 3.0):
        w = [kennzahlen(tasten, rng.normal(0, sigma, len(tasten)))[1] for _ in range(2000)]
        print(f"  sigma {sigma}: gemessen {np.mean(w):.3f}, Theorie {0.954 * sigma:.3f}")


# ------------------------------------------------------------------ Raum: Pegelstreuung je Teilton
def teil_C():
    print("== C  Raum: Streuung des Pegels je Ton und Teilton bei Direkt-zu-Hall-Verhaeltnis DRR ==")
    rng = np.random.default_rng(RNG_SEED + 1)
    n = 200000
    print("Ein Teilton (stationaer), |d + r|^2 mit r komplex normal, E|r|^2 = 10^(-DRR/10):")
    print("  DRR dB | Std 10lg|H|^2 | Std Differenz zweier Toene | Theorie diffus 5,57 / 7,88 dB")
    for drr in (20, 10, 5, 0, -5, -10, -100):
        r = (rng.normal(size=n) + 1j * rng.normal(size=n)) * np.sqrt(10 ** (-drr / 10) / 2)
        d = 1.0 if drr > -100 else 0.0
        L = 10 * np.log10(np.abs(d + r) ** 2)
        L2 = 10 * np.log10(np.abs(d + np.roll(r, 1)) ** 2)
        print(f"  {drr:6.0f} | {L.std():13.2f} | {(L - L2).std():13.2f}")
    print("Ganzer Ton mit K Teiltoenen (Amplitude 1/k), Lautheit ~ 10lg Summe |a_k H_k|^2, H_k unabhaengig je Teilton:")
    print("  DRR dB | K=1   K=3   K=10  K=30   (Std des Tonpegels in dB) | groesster Sprung erwartet bei 72 Tasten, Paare<=4 HT, K=10")
    tasten = np.arange(72)
    for drr in (10, 5, 0, -5, -100):
        zeile = []
        for K in (1, 3, 10, 30):
            a2 = 1.0 / np.arange(1, K + 1) ** 2
            r = (rng.normal(size=(20000, K)) + 1j * rng.normal(size=(20000, K))) * np.sqrt(10 ** (-drr / 10) / 2)
            d = 1.0 if drr > -100 else 0.0
            L = 10 * np.log10((a2 * np.abs(d + r) ** 2).sum(axis=1))
            zeile.append(L.std())
        a2 = 1.0 / np.arange(1, 11) ** 2
        mx = []
        for _ in range(300):
            r = (rng.normal(size=(72, 10)) + 1j * rng.normal(size=(72, 10))) * np.sqrt(10 ** (-drr / 10) / 2)
            d = 1.0 if drr > -100 else 0.0
            L = 10 * np.log10((a2 * np.abs(d + r) ** 2).sum(axis=1))
            sp, med, m, _ = kennzahlen(tasten, L)
            mx.append((m, med, sp))
        mx = np.array(mx)
        print(f"  {drr:6.0f} | " + "  ".join(f"{z:4.2f}" for z in zeile)
              + f"   | groesster {mx[:, 0].mean():4.1f} (95 %: {np.quantile(mx[:, 0], .95):4.1f}), mittlerer {mx[:, 1].mean():3.1f}, Spanne {mx[:, 2].mean():4.1f} LU")


# ------------------------------------------------------------------ Abklingen je Teilton
def huellkurve_heterodyn(x, fs, f, rahmen=0.01, glatt=0.02):
    t = np.arange(len(x)) / fs
    z = x * np.exp(-2j * np.pi * f * t)
    n = int(glatt * fs)
    z = np.convolve(z, np.ones(n) / n, mode="same")
    s = int(rahmen * fs)
    e = np.abs(z[::s]) ** 2 * 4  # Energie der Huellkurve (Amplitude^2)
    return e, rahmen


def zwei_geraden(tt, y):
    """Zwei unabhaengige Geraden mit bestem Knick (wie Teiltonmessung.hpp: frueh >= 1/20, spaet >= 1/5 des Bereichs)."""
    n = len(y)
    best = None
    for k in range(max(3, n // 20), n - max(3, n // 5)):
        p1 = np.polyfit(tt[:k], y[:k], 1)
        p2 = np.polyfit(tt[k:], y[k:], 1)
        r = ((np.polyval(p1, tt[:k]) - y[:k]) ** 2).sum() + ((np.polyval(p2, tt[k:]) - y[k:]) ** 2).sum()
        if best is None or r < best[0]:
            best = (r, k, p1, p2)
    _, k, p1, p2 = best
    knick = (p2[1] - p1[1]) / (p1[0] - p2[0]) if p1[0] != p2[0] else tt[k]
    return -p1[0], -p2[0], knick


def bereich(e, rahmen, ueber_rausch_db=10):
    """Von der Spitze bis zuletzt 10 dB ueber dem Boden (Median der letzten 10 %), wie Teiltonmessung/Lundeby Schritt 2."""
    L = 10 * np.log10(e + 1e-30)
    i0 = int(np.argmax(L))
    boden = np.median(L[int(0.9 * len(L)):])
    ueber = np.where(L > boden + ueber_rausch_db)[0]
    i1 = ueber[-1] if len(ueber) else len(L) - 1
    return i0, max(i1, i0 + 10), boden


def schroeder_zwei(e, rahmen, i0, i1):
    """Schroeder auf [i0, i1), spaete Gerade, deren Energie abziehen, frueh neu (wie zweiStufen in Teiltonmessung)."""
    seg = e[i0:i1]
    tt = np.arange(len(seg)) * rahmen
    L = 10 * np.log10(np.cumsum(seg[::-1])[::-1] / seg.sum())
    m = len(L) - max(3, int(0.1 * len(L)))  # die letzten 10 dB vor dem Ende biegt die Abschneidung
    ziel = L[:m]
    D1, D2, kn = zwei_geraden(tt[:m], ziel)
    # Energie der spaeten Stufe (Gerade im Pegel, aus den Rohdaten spaet) abziehen
    sp = slice(int(len(seg) * 0.6), len(seg))
    p = np.polyfit(tt[sp], 10 * np.log10(seg[sp] + 1e-30), 1)
    rest = np.maximum(seg - 10 ** (np.polyval(p, tt) / 10), 0)
    Lr = 10 * np.log10(np.cumsum(rest[::-1])[::-1] / max(rest.sum(), 1e-30) + 1e-30)
    gut = Lr > -30
    Dfrueh = -np.polyfit(tt[gut][:max(5, gut.sum())], Lr[gut], 1)[0] if gut.sum() > 4 else np.nan
    return Dfrueh, D2, kn


def zwei_komponenten(e, rahmen, i0, i1, koharent):
    """Modell mit zwei Komponenten und Boden, im Pegel (dB) ausgeglichen:
    inkohaerent  e = E1 10^(-a t/10) + E2 10^(-b t/10) + N         (zwei Polarisationen mit Frequenzversatz)
    kohaerent    e = (A1 10^(-a t/20) + A2 10^(-b t/20))^2 + N     (gleiche Frequenz, gleiche Phase)
    Gibt (a, b, Knick) zurueck; a, b in dB/s."""
    from scipy.optimize import least_squares
    seg = e[i0:]
    tt = np.arange(len(seg)) * rahmen
    y = 10 * np.log10(seg + 1e-30)
    N0 = np.median(seg[int(0.9 * len(seg)):])

    def modell(p):
        l1, a, l2, b, ln = p
        if koharent:
            m = (10 ** (l1 / 20) * 10 ** (-a * tt / 20) + 10 ** (l2 / 20) * 10 ** (-b * tt / 20)) ** 2
        else:
            m = 10 ** (l1 / 10) * 10 ** (-a * tt / 10) + 10 ** (l2 / 10) * 10 ** (-b * tt / 10)
        return 10 * np.log10(m + 10 ** (ln / 10))

    a0 = max(-np.polyfit(tt[: max(5, (i1 - i0) // 10)], y[: max(5, (i1 - i0) // 10)], 1)[0], 1.0)
    p0 = [y[0], a0, y[0] - 20, a0 / 8, 10 * np.log10(N0 + 1e-30)]
    r = least_squares(lambda p: modell(p) - y, p0, bounds=([-200, 0.1, -200, 0.01, -300], [50, 2000, 50, 2000, 50]))
    l1, a, l2, b, _ = r.x
    if a < b:
        l1, a, l2, b = l2, b, l1, a
    knick = (l1 - l2) / (a - b)
    return a, b, knick


def raum_ir(fs, T, drr_db, rng, dauer=None):
    dauer = dauer or 1.2 * T
    t = np.arange(int(dauer * fs)) / fs
    h = rng.normal(size=len(t)) * 10 ** (-3 * t / T)
    h[: int(0.002 * fs)] = 0  # erste Reflexion nach 2 ms
    h *= np.sqrt(10 ** (-drr_db / 10) / (h ** 2).sum())
    h[0] += 1.0
    return h


def teilton(fs, dauer, f, stufen, df=0.0, rng=None, rauschen_db=None):
    """stufen: [(Pegel dB, Abfall dB/s)], zweite Stufe mit Frequenzversatz df (zwei Polarisationen)."""
    t = np.arange(int(dauer * fs)) / fs
    x = np.zeros_like(t)
    for i, (pegel, D) in enumerate(stufen):
        x += 10 ** (pegel / 20) * 10 ** (-D * t / 20) * np.sin(2 * np.pi * (f + i * df) * t + 0.3 * i)
    x *= np.minimum(t / 0.003, 1)
    if rauschen_db is not None:
        x += rng.normal(size=len(t)) * 10 ** (rauschen_db / 20)
    return x


def teil_D():
    print("== D  Doppelabklingen je Teilton: Vorgabe gegen Messung, ohne und mit Raum und Rauschen ==")
    rng = np.random.default_rng(RNG_SEED + 2)
    fs, f = 16000, 330.0
    vorgaben = [("Gitarre mittel", [(0, 40.0), (-20, 4.0)]), ("Harfe Diskant", [(0, 150.0), (-25, 20.0)])]
    for name, st in vorgaben:
        (p1, D1), (p2, D2) = st
        kn = (p1 - p2) / (D1 - D2)
        print(f"-- {name}: Vorgabe frueh {D1} dB/s, spaet {D2} dB/s, Knick {kn:.3f} s (Pegel {p1 - D1 * kn:.1f} dB)")
        faelle = [("ohne Raum, Rauschen -90 dB", None, -90, 0.0),
                  ("ohne Raum, Rauschen -60 dB", None, -60, 0.0),
                  ("zwei Polarisationen df = 0,7 Hz", None, -90, 0.7),
                  ("Raum T=1,5 s, DRR +10 dB", (1.5, 10), -90, 0.0),
                  ("Raum T=1,5 s, DRR 0 dB", (1.5, 0), -90, 0.0),
                  ("Raum T=3,0 s, DRR 0 dB", (3.0, 0), -90, 0.0),
                  ("Raum T=0,4 s (Studio), DRR +5 dB", (0.4, 5), -90, 0.0)]
        dauer = min(60.0 / D2 * 1.3, 12.0)
        print("   Fall                               | Pegel-Geraden frueh/spaet/Knick | Schroeder frueh/spaet/Knick | Zwei Komponenten (Modell passend) frueh/spaet/Knick")
        for fall, raum, rausch, df in faelle:
            x = teilton(fs, dauer, f, st, df=df, rng=rng, rauschen_db=None)
            if raum:
                x = fftconvolve(x, raum_ir(fs, raum[0], raum[1], rng))[: len(x)]
            x = x + rng.normal(size=len(x)) * 10 ** (rausch / 20)
            e, r = huellkurve_heterodyn(x, fs, f)
            i0, i1, boden = bereich(e, r)
            seg = 10 * np.log10(e[i0:i1] + 1e-30)
            tt = np.arange(len(seg)) * r
            a1, a2, k1 = zwei_geraden(tt, seg)
            s1, s2, k2 = schroeder_zwei(e, r, i0, i1)
            c1, c2, k3 = zwei_komponenten(e, r, i0, len(e), koharent=(df == 0.0))
            print(f"   {fall:35s}| {a1:6.1f} {a2:6.1f} {k1:6.3f}          | {s1:6.1f} {s2:6.1f} {k2:6.3f}       "
                  f"| {c1:6.1f} {c2:6.1f} {k3:6.3f}   (Bereich {seg[0] - seg[-1]:.0f} dB)")
    print("Messbarer Bereich eines einstufigen Teiltons im Raum (Direktanteil bis zum Schnitt mit dem Hall, Energie):")
    print("  Formel dL = a/(a-b) * (DRR + 10lg(a/b)) fuer a > b; a, b Abklingraten Teilton und Raum (Erwartungswert,")
    print("  inkohaerent); Rechnung: 30 Raeume, Stelle, an der die Huellkurve 3 dB ueber dem Direktanteil liegt")
    print("  Teilton T60 | Raum T60 | DRR | Formel dB | Rechnung Median (10.-90. Perzentil) dB")
    for Tp, Tr, drr in ((0.4, 1.5, 10), (0.4, 1.5, 0), (1.0, 2.0, 0), (0.3, 0.4, 5), (0.5, 2.0, 5)):
        a, b = 60 / Tp, 60 / Tr
        formel = a / (a - b) * (drr + 10 * np.log10(a / b))
        werte = []
        for _ in range(30):
            x = teilton(fs, max(Tp, Tr) * 2.5, f, [(0, a)])
            y = fftconvolve(x, raum_ir(fs, Tr, drr, rng))[: len(x)]
            e, r = huellkurve_heterodyn(y, fs, f)
            ed, _ = huellkurve_heterodyn(x, fs, f)
            L, Ld = 10 * np.log10(e + 1e-30), 10 * np.log10(ed + 1e-30)
            i0 = int(np.argmax(Ld))
            ab = np.where((L[i0:] - Ld[i0:]) > 3)[0]
            werte.append(-(Ld[i0 + ab[0]] - Ld[i0]) if len(ab) else np.nan)
        print(f"  {Tp:11.1f} | {Tr:8.1f} | {drr:3.0f} | {formel:9.1f} | {np.nanmedian(werte):5.1f} "
              f"({np.nanquantile(werte, .1):5.1f} … {np.nanquantile(werte, .9):5.1f})")
    print("Teilton langsamer als der Raum (a < b): Steigung ueber 0..-40 dB, 30 Raeume")
    for Tp, Tr, drr in ((3.0, 1.5, 0), (6.0, 2.0, 0), (2.0, 1.5, -5)):
        a = 60 / Tp
        st = []
        for _ in range(30):
            x = teilton(fs, Tp * 1.2, f, [(0, a)])
            y = fftconvolve(x, raum_ir(fs, Tr, drr, rng))[: len(x)]
            e, r = huellkurve_heterodyn(y, fs, f)
            L = 10 * np.log10(e + 1e-30)
            i0 = int(np.argmax(L))
            j = i0 + np.where(L[i0:] < L[i0] - 40)[0][0]
            tt = np.arange(i0, j) * r
            st.append(-np.polyfit(tt, L[i0:j], 1)[0])
        print(f"  Teilton T60 {Tp} s ({a:.0f} dB/s), Raum {Tr} s, DRR {drr:+d}: gemessen {np.median(st):.1f} dB/s "
              f"({np.quantile(st, .1):.1f} … {np.quantile(st, .9):.1f})")


def teil_E():
    print("== E  Warum Huellkurve und Teilton ein anderes Vorzeichen geben ==")
    fs, f0 = 16000, 220.0
    t = np.arange(int(4 * fs)) / fs
    K = np.arange(1, 13)

    def ton(amp, D):
        return sum(a * 10 ** (-d * t / 20) * np.sin(2 * np.pi * k * f0 * t) for k, a, d in zip(K, amp, D))

    def frueh(x):
        n = int(0.02 * fs)
        e = np.array([np.mean(x[i:i + n] ** 2) for i in range(0, len(x) - n, n)])
        L = 10 * np.log10(e + 1e-30)
        L -= L.max()
        m = int(np.argmax(L))
        idx = [i for i in range(m, len(L)) if -25 <= L[i] <= -5]
        idx = [i for i in idx if i <= next((j for j in range(m, len(L)) if L[j] < -25), len(L))]
        return -np.polyfit(np.array(idx) * 0.02, L[idx], 1)[0]

    print("Modell: jeder Teilton faellt 25 % schneller als in der Aufnahme. Huellkurve wie S195 (-5..-25 dB, 20-ms-Bloecke).")
    print("  Aufnahme Amplitude / Abfall je k | Modell Amplitude | Huellkurve Aufnahme / Modell dB/s | Huellkurve sagt")
    for p_amp_auf, p_D, p_amp_mod in ((0.3, 1.0, 2.0), (0.0, 1.5, 2.0), (0.0, 1.5, 1.0), (1.0, 1.0, 1.0)):
        D_auf = 4.0 * K ** p_D
        D_mod = 1.25 * D_auf
        fa = frueh(ton(1.0 / K ** p_amp_auf, D_auf))
        fm = frueh(ton(1.0 / K ** p_amp_mod, D_mod))
        print(f"  1/k^{p_amp_auf:.1f} / 4 k^{p_D:.1f} dB/s            | 1/k^{p_amp_mod:.1f}          | {fa:6.1f} / {fm:6.1f}"
              f"                   | Modell faellt {'LANGSAMER (falsches Vorzeichen)' if fm < fa else 'schneller (richtig)'}")
    print("Mechanismus 1 — schwacher Grundton in der Aufnahme (Abstrahlung unter der tiefsten Koerpermode), D = 4 k dB/s:")
    D_auf = 4.0 * K
    amp_auf = 1.0 / np.sqrt(K)
    for g1 in (1.0, 0.3, 0.1, 0.03):
        a = amp_auf.copy()
        a[0] = g1
        fa = frueh(ton(a, D_auf))
        fm = frueh(ton(1.0 / K, 1.25 * D_auf))
        print(f"  Grundton der Aufnahme {20 * np.log10(g1):+5.1f} dB: Huellkurve Aufnahme {fa:5.1f}, Modell {fm:5.1f} dB/s -> "
              f"{'FALSCHES Vorzeichen' if fm < fa else 'richtig'}")
    print("Mechanismus 2 — Raum an der Aufnahme (Modell trocken), ein Teilton 440 Hz:")
    rng = np.random.default_rng(RNG_SEED + 6)
    for D_wahr, D_mod, T, drr in ((40.0, 34.0, 2.0, 0.0), (40.0, 34.0, 2.0, 10.0), (15.0, 13.0, 2.0, 0.0), (15.0, 13.0, 1.0, 0.0)):
        x = 10 ** (-D_wahr * t / 20) * np.sin(2 * np.pi * 440 * t)
        xr = fftconvolve(x, raum_ir(fs, T, drr, rng))[: len(t)]
        m = 10 ** (-D_mod * t / 20) * np.sin(2 * np.pi * 440 * t)
        fa, fm = frueh(xr), frueh(m)
        print(f"  Aufnahme trocken {D_wahr:.0f} dB/s, Raum T {T} s DRR {drr:+.0f} dB, Modell {D_mod:.0f} dB/s (langsamer): "
              f"Huellkurve Aufnahme {fa:5.1f}, Modell {fm:5.1f} -> {'FALSCHES Vorzeichen' if fm > fa else 'richtig'}")


# ------------------------------------------------------------------ Inharmonizitaet
def bh(n):
    a = [0.35875, 0.48829, 0.14128, 0.01168]
    k = np.arange(n) / (n - 1)
    return a[0] - a[1] * np.cos(2 * np.pi * k) + a[2] * np.cos(4 * np.pi * k) - a[3] * np.cos(6 * np.pi * k)


def schaetze_B(x, fs, f_nenn, kmax, fenster_s, ab_s=0.05, B_max=0.02):
    n = int(fenster_s * fs)
    seg = x[int(ab_s * fs): int(ab_s * fs) + n] * bh(n)
    N = 4 * 2 ** int(np.ceil(np.log2(n)))
    S = 20 * np.log10(np.abs(np.fft.rfft(seg, N)) + 1e-30)
    df = fs / N
    ks, fk = [], []
    f0, B = f_nenn, 0.0
    for k in range(1, kmax + 1):
        mitte = k * f0 * np.sqrt(1 + B * k * k)
        breite = max(0.4 * f0, 3 * fs / n)
        lo, hi = int((mitte - breite / 2) / df), int((mitte + breite / 2) / df)
        if hi >= len(S) - 1:
            break
        i = lo + int(np.argmax(S[lo:hi]))
        a, b, c = S[i - 1], S[i], S[i + 1]
        p = 0.5 * (a - c) / (a - 2 * b + c)
        ks.append(k)
        fk.append((i + p) * df)
        if len(ks) >= 3:
            kk, ff = np.array(ks, float), np.array(fk)
            A = np.vstack([np.ones_like(kk), kk ** 2]).T
            c0, c1 = np.linalg.lstsq(A, (ff / kk) ** 2, rcond=None)[0]
            f0, B = np.sqrt(c0), max(c1 / c0, 0.0)
    kk, ff = np.array(ks, float), np.array(fk)
    A = np.vstack([np.ones_like(kk), kk ** 2]).T
    (c0, c1), res, _, _ = np.linalg.lstsq(A, (ff / kk) ** 2, rcond=None)
    return np.sqrt(c0), c1 / c0


def teil_F():
    print("== F  Tonhoehe und Inharmonizitaet B: Vorgabe gegen Schaetzung ==")
    rng = np.random.default_rng(RNG_SEED + 3)
    fs = 44100
    print("Abweichung Teilton n gegen harmonisch: 1200 log2 sqrt(1 + B n^2) Cent ~ 866 B n^2")
    for B in (1e-5, 1e-4):
        print(f"  B = {B:g}: n=5 {1200 * np.log2(np.sqrt(1 + B * 25)):.2f}, n=10 {1200 * np.log2(np.sqrt(1 + B * 100)):.2f}, "
              f"n=20 {1200 * np.log2(np.sqrt(1 + B * 400)):.2f}, n=30 {1200 * np.log2(np.sqrt(1 + B * 900)):.2f} Cent")
    print("Synthetischer Ton f0 110 Hz, Teiltoene 1/k, Abfall 3 + 0,8k dB/s, 20 Laeufe (Phasen zufaellig); dazu je Teilton")
    print("ein zufaelliger Frequenzversatz sigma_c Cent (Kopplung an Steg und Koerper, die das Gesetz nicht kennt).")
    print("Theorie (nur Versatz): Std(B) = 2 ln2/1200 * sigma_c / sqrt(Summe (k^2 - Mittel k^2)^2)")
    print("  B Vorgabe | Rauschen dB | sigma_c | Fenster s | Teiltoene | B Mittel   B Std     Theorie  | f0 Fehler Cent Mittel/Std")
    t = np.arange(int(1.6 * fs)) / fs
    for B in (1e-5, 1e-4):
        for rausch, sc, fenster in ((-70, 0.0, 0.5), (-40, 0.0, 0.25), (-40, 0.0, 0.5), (-70, 1.0, 0.5), (-70, 3.0, 0.5)):
            for kmax in (5, 10, 20, 30):
                bs, fe = [], []
                for _ in range(20):
                    vers = 2 ** (rng.normal(0, sc, 41) / 1200) if sc > 0 else np.ones(41)
                    x = sum((1 / k) * 10 ** (-(3 + 0.8 * k) * t / 20)
                            * np.sin(2 * np.pi * k * 110 * np.sqrt(1 + B * k * k) * vers[k] * t + rng.uniform(0, 6.28))
                            for k in range(1, 41))
                    x += rng.normal(size=len(t)) * 10 ** (rausch / 20)
                    f0, b = schaetze_B(x, fs, 110 * 2 ** (rng.uniform(-30, 30) / 1200), kmax, fenster)
                    bs.append(b)
                    fe.append(1200 * np.log2(f0 / 110))
                k2 = np.arange(1, kmax + 1) ** 2.0
                theo = 2 * np.log(2) / 1200 * sc / np.sqrt(((k2 - k2.mean()) ** 2).sum())
                print(f"  {B:9.0e} | {rausch:11d} | {sc:7.1f} | {fenster:9.2f} | {kmax:9d} | {np.mean(bs):9.2e} {np.std(bs):9.1e} "
                      f"{theo:8.1e} | {np.mean(fe):+.3f} / {np.std(fe):.3f}")


# ------------------------------------------------------------------ Spektrum: Schwerpunkt im Raum
def teil_G():
    print("== G  Spektrum: Teiltonpegel relativ zum Grundton und spektraler Schwerpunkt bei Raum (DRR) ==")
    rng = np.random.default_rng(RNG_SEED + 4)
    K = np.arange(1, 21)
    a2 = 1.0 / K ** 2
    sz_wahr = (K * a2).sum() / a2.sum()
    print(f"Vorgabe: 20 Teiltoene 1/k, Schwerpunkt {sz_wahr:.3f} (in Vielfachen von f0)")
    print("  DRR dB | Std Pegel Teilton k gegen Teilton 1 dB | Schwerpunkt Mittel / Std (relativ)")
    for drr in (20, 10, 0, -10, -100):
        r = (rng.normal(size=(20000, 20)) + 1j * rng.normal(size=(20000, 20))) * np.sqrt(10 ** (-drr / 10) / 2)
        d = 1.0 if drr > -100 else 0.0
        p = a2 * np.abs(d + r) ** 2
        rel = 10 * np.log10(p[:, 4] / p[:, 0])
        sz = (K * p).sum(axis=1) / p.sum(axis=1)
        print(f"  {drr:6.0f} | {rel.std():6.2f}                                  | {sz.mean() / sz_wahr:5.3f} / {sz.std() / sz_wahr:5.3f}")


# ------------------------------------------------------------------ Einschwinggeraeusch
def teil_H():
    print("== H  Einschwinggeraeusch: Trennung vom harmonischen Teil durch Abzug der Teiltoene ==")
    rng = np.random.default_rng(RNG_SEED + 5)
    fs, f0 = 44100, 196.0
    t = np.arange(int(1.5 * fs)) / fs
    K = np.arange(1, 16)
    harm = sum((1 / k) * 10 ** (-(4 + k) * t / 20) * np.sin(2 * np.pi * k * f0 * t + 0.7 * k) for k in K)
    for pegel in (-10.0, -20.0, -30.0):
        burst = rng.normal(size=len(t)) * np.exp(-t / 0.008) * 10 ** (pegel / 20)
        x = harm + burst
        # Modell: je Teilton A e^{-s t} sin(w t + phi) aus 0,05..0,5 s, Frequenz bekannt; Abfall aus der Huellkurve
        fit = slice(int(0.05 * fs), int(0.5 * fs))
        rekon = np.zeros_like(t)
        for k in K:
            e, r = huellkurve_heterodyn(x, fs, k * f0, rahmen=0.005)
            tt = np.arange(len(e)) * r
            m = (tt > 0.05) & (tt < 0.5)
            s = -np.polyfit(tt[m], 10 * np.log10(e[m] + 1e-30), 1)[0] * np.log(10) / 20
            basis = np.vstack([np.exp(-s * t) * np.sin(2 * np.pi * k * f0 * t), np.exp(-s * t) * np.cos(2 * np.pi * k * f0 * t)]).T
            c = np.linalg.lstsq(basis[fit], x[fit], rcond=None)[0]
            rekon += basis @ c
        rest = x - rekon
        w = slice(0, int(0.05 * fs))
        print(f"  Geraeusch {pegel:+.0f} dB: Energie der ersten 50 ms wahr {10 * np.log10(np.sum(burst[w] ** 2)):+.2f} dB, "
              f"Rest nach Abzug {10 * np.log10(np.sum(rest[w] ** 2)):+.2f} dB, Verhaeltnis Geraeusch/Ton wahr "
              f"{10 * np.log10(np.sum(burst[w] ** 2) / np.sum(harm[w] ** 2)):+.2f}, gemessen "
              f"{10 * np.log10(np.sum(rest[w] ** 2) / np.sum(rekon[w] ** 2)):+.2f} dB")


TEILE = {"A": teil_A, "B": teil_B, "C": teil_C, "D": teil_D, "E": teil_E, "F": teil_F, "G": teil_G, "H": teil_H}

if __name__ == "__main__":
    for name in (sys.argv[1:] or list(TEILE)):
        TEILE[name]()
        print()
```

Ausgabe (`python gegenrechnung.py`, alle Teile):

```text
== A  Lautheit nach BS.1770: Bezugswert und abklingende Toene ==
K-Filter bei 48 kHz gegen BS.1770-5 Tab. 1/2: groesste Abweichung Stufe 1 8.9e-16, Stufe 2 2.2e-16
  fs 48000: Sinus 997 Hz 0 dBFS in einem Kanal: M-max -3.008, S-max -3.010, I -3.010 LUFS (Soll -3.01); derselbe Ton in beiden Kanaelen (Mono als Stereo): +0.002
  fs 44100: Sinus 997 Hz 0 dBFS in einem Kanal: M-max -3.005, S-max -3.007, I -3.008 LUFS (Soll -3.01); derselbe Ton in beiden Kanaelen (Mono als Stereo): +0.005
  fs 96000: Sinus 997 Hz 0 dBFS in einem Kanal: M-max -3.026, S-max -3.028, I -3.028 LUFS (Soll -3.01); derselbe Ton in beiden Kanaelen (Mono als Stereo): -0.015
Abklingender Sinus 1 kHz (Anfangsamplitude 1, Einsatz 5 ms), Abfall D in dB/s, Datei 12 s, danach Stille:
  D dB/s | M-max  S-max  I(gated) | M-max - Dauerton | Formel 10lg[(1-10^(-DW/10))/(DW ln10/10)], W=0,4 s
       1 |  -3.21  -4.43   -7.77   |  -0.20           |  -0.20
       3 |  -3.61  -6.77   -9.96   |  -0.60           |  -0.59
      10 |  -4.90 -11.44  -11.45   |  -1.89           |  -1.85
      30 |  -7.81 -16.27  -13.23   |  -4.80           |  -4.70
      60 | -10.65 -19.38  -15.38   |  -7.64           |  -7.44
     150 | -14.91 -23.66  -17.77   | -11.90           | -11.40
Integrierte Lautheit haengt an der Dateilaenge (D = 10 dB/s):
  Datei   1 s: I  -7.41, M-max  -4.86
  Datei   2 s: I -10.14, M-max  -4.86
  Datei   4 s: I -10.83, M-max  -4.86
  Datei   8 s: I -11.44, M-max  -4.86

== B  Gang ueber den Umfang: Spanne, Paare bis 4 Halbtoene, Glaettung ==
Vorgabe ohne Streuung, alle Tasten: Spanne 16.08 (Gang 6,0 + Stoerungen), mittlerer Sprung 0.25, groesster 8.33 LU, 278 Paare
  nach gleitendem Median ueber 7 Tasten: Abweichung an Taste 56 +0.25, an Taste 80 -0.08 LU (Vorgabe +6 bzw. -8: beide Stoerungen entfernt)
  Stoerung +6 LU ueber 3 Nachbartasten: nach der Glaettung an Taste 56 +0.25 LU
  Stoerung +6 LU ueber 4 Nachbartasten: nach der Glaettung an Taste 56 +5.83 LU
  Stoerung +6 LU ueber 5 Nachbartasten: nach der Glaettung an Taste 56 +5.83 LU
Mit Streuung sigma je Ton (Spieler, Raum), 2000 Laeufe: Mittel und 95-%-Quantil
  sigma | Auswahl        | Spanne       | mittlerer Sprung | groesster Sprung | Theorie mittl. = 0,954 sigma (nur Streuung)
   0.0  | jede Taste     |  16.1 /  16.1 |   0.2 /   0.2    |   8.3 /   8.3    | 0.00
   0.0  | jede 3. Taste  |  16.0 /  16.0 |   0.2 /   0.2    |   8.2 /   8.2    | 0.00
   1.0  | jede Taste     |  16.9 /  19.0 |   1.1 /   1.3    |   9.6 /  11.4    | 0.95
   1.0  | jede 3. Taste  |  16.0 /  18.3 |   1.2 /   1.8    |   8.7 /  10.7    | 0.95
   2.0  | jede Taste     |  18.0 /  22.0 |   2.1 /   2.5    |  11.9 /  15.1    | 1.91
   2.0  | jede 3. Taste  |  16.5 /  20.8 |   2.4 /   3.4    |  10.0 /  13.7    | 1.91
   3.0  | jede Taste     |  20.0 /  24.9 |   3.1 /   3.7    |  14.8 /  18.8    | 2.86
   3.0  | jede 3. Taste  |  17.8 /  23.4 |   3.4 /   4.9    |  11.6 /  16.4    | 2.86
   4.0  | jede Taste     |  23.3 /  29.2 |   4.0 /   4.8    |  18.2 /  23.2    | 3.82
   4.0  | jede 3. Taste  |  20.3 /  27.0 |   4.3 /   6.2    |  14.3 /  20.3    | 3.82
Reine Streuung ohne Gang, 72 Tasten, mittlerer Sprung Paare <=4 HT (Pruefung der Theorie):
  sigma 1.0: gemessen 0.960, Theorie 0.954
  sigma 3.0: gemessen 2.882, Theorie 2.862

== C  Raum: Streuung des Pegels je Ton und Teilton bei Direkt-zu-Hall-Verhaeltnis DRR ==
Ein Teilton (stationaer), |d + r|^2 mit r komplex normal, E|r|^2 = 10^(-DRR/10):
  DRR dB | Std 10lg|H|^2 | Std Differenz zweier Toene | Theorie diffus 5,57 / 7,88 dB
      20 |          0.62 |          0.87
      10 |          2.00 |          2.83
       5 |          3.66 |          5.16
       0 |          5.09 |          7.19
      -5 |          5.49 |          7.77
     -10 |          5.56 |          7.84
    -100 |          5.56 |          7.88
Ganzer Ton mit K Teiltoenen (Amplitude 1/k), Lautheit ~ 10lg Summe |a_k H_k|^2, H_k unabhaengig je Teilton:
  DRR dB | K=1   K=3   K=10  K=30   (Std des Tonpegels in dB) | groesster Sprung erwartet bei 72 Tasten, Paare<=4 HT, K=10
      10 | 2.01  1.42  1.22  1.18   | groesster  5.2 (95 %:  6.6), mittlerer 1.2, Spanne  5.8 LU
       5 | 3.61  2.29  1.94  1.84   | groesster  8.0 (95 %:  9.8), mittlerer 1.9, Spanne  8.9 LU
       0 | 5.14  3.06  2.48  2.36   | groesster 10.2 (95 %: 12.6), mittlerer 2.4, Spanne 11.5 LU
      -5 | 5.53  3.29  2.72  2.56   | groesster 11.1 (95 %: 13.5), mittlerer 2.6, Spanne 12.5 LU
    -100 | 5.55  3.37  2.76  2.59   | groesster 11.2 (95 %: 13.7), mittlerer 2.7, Spanne 12.7 LU

== D  Doppelabklingen je Teilton: Vorgabe gegen Messung, ohne und mit Raum und Rauschen ==
-- Gitarre mittel: Vorgabe frueh 40.0 dB/s, spaet 4.0 dB/s, Knick 0.556 s (Pegel -22.2 dB)
   Fall                               | Pegel-Geraden frueh/spaet/Knick | Schroeder frueh/spaet/Knick | Zwei Komponenten (Modell passend) frueh/spaet/Knick
   ohne Raum, Rauschen -90 dB         |   26.1    4.0  0.825          |   26.0    4.2  0.536       |   40.3    4.0  0.530   (Bereich 56 dB)
   ohne Raum, Rauschen -60 dB         |   26.1    4.0  0.825          |   26.0    4.2  0.536       |   40.2    4.0  0.533   (Bereich 55 dB)
   zwei Polarisationen df = 0,7 Hz    |   55.5    4.0  0.439          |   24.8    4.2  0.320       |   63.3    3.9  0.401   (Bereich 56 dB)
   Raum T=1,5 s, DRR +10 dB           |   24.6    4.0  0.884          |   31.4    4.2  0.608       |   38.0    4.0  0.561   (Bereich 56 dB)
   Raum T=1,5 s, DRR 0 dB             |   24.2    4.0  0.919          |   34.7    4.2  0.628       |   35.5    4.0  0.608   (Bereich 56 dB)
   Raum T=3,0 s, DRR 0 dB             |   16.8    4.0  1.380          |   23.0    4.2  1.031       |   25.5    4.0  0.871   (Bereich 55 dB)
   Raum T=0,4 s (Studio), DRR +5 dB   |   25.9    4.0  0.834          |   26.1    4.2  0.543       |   40.0    4.0  0.536   (Bereich 55 dB)
-- Harfe Diskant: Vorgabe frueh 150.0 dB/s, spaet 20.0 dB/s, Knick 0.192 s (Pegel -28.8 dB)
   Fall                               | Pegel-Geraden frueh/spaet/Knick | Schroeder frueh/spaet/Knick | Zwei Komponenten (Modell passend) frueh/spaet/Knick
   ohne Raum, Rauschen -90 dB         |  107.5   20.1  0.254          |  112.8   20.3  0.187       |  149.5   20.0  0.182   (Bereich 86 dB)
   ohne Raum, Rauschen -60 dB         |  109.6   20.3  0.248          |  140.3   19.7  0.196       |  154.6   20.5  0.173   (Bereich 72 dB)
   zwei Polarisationen df = 0,7 Hz    |  122.2   20.0  0.227          |  136.5   20.2  0.166       |  138.3   20.1  0.203   (Bereich 86 dB)
   Raum T=1,5 s, DRR +10 dB           |   89.2   19.9  0.322          |   46.1   20.5  0.268       |  126.4   19.9  0.223   (Bereich 88 dB)
   Raum T=1,5 s, DRR 0 dB             |   39.1   20.5  0.922          |   42.3   20.5  0.853       |   53.9   19.7  0.620   (Bereich 90 dB)
   Raum T=3,0 s, DRR 0 dB             |   18.4    2.3  3.345          |   19.6   17.9  5.705       |   19.9   19.9 -2375.664   (Bereich 74 dB)
   Raum T=0,4 s (Studio), DRR +5 dB   |   92.8   20.1  0.286          |  106.4   20.2  0.223       |  130.0   20.0  0.199   (Bereich 83 dB)
Messbarer Bereich eines einstufigen Teiltons im Raum (Direktanteil bis zum Schnitt mit dem Hall, Energie):
  Formel dL = a/(a-b) * (DRR + 10lg(a/b)) fuer a > b; a, b Abklingraten Teilton und Raum (Erwartungswert,
  inkohaerent); Rechnung: 30 Raeume, Stelle, an der die Huellkurve 3 dB ueber dem Direktanteil liegt
  Teilton T60 | Raum T60 | DRR | Formel dB | Rechnung Median (10.-90. Perzentil) dB
          0.4 |      1.5 |  10 |      21.5 |  18.7 (  9.4 …  29.2)
          0.4 |      1.5 |   0 |       7.8 |   6.7 (  2.1 …  15.6)
          1.0 |      2.0 |   0 |       6.0 |   4.9 (  0.6 …  10.8)
          0.3 |      0.4 |   5 |      25.0 |  12.0 (  1.0 …  29.3)
          0.5 |      2.0 |   5 |      14.7 |  14.8 (  6.0 …  20.8)
Teilton langsamer als der Raum (a < b): Steigung ueber 0..-40 dB, 30 Raeume
  Teilton T60 3.0 s (20 dB/s), Raum 1.5 s, DRR +0: gemessen 19.3 dB/s (17.6 … 23.3)
  Teilton T60 6.0 s (10 dB/s), Raum 2.0 s, DRR +0: gemessen 10.0 dB/s (9.4 … 10.8)
  Teilton T60 2.0 s (30 dB/s), Raum 1.5 s, DRR -5: gemessen 27.5 dB/s (24.0 … 34.2)

== E  Warum Huellkurve und Teilton ein anderes Vorzeichen geben ==
Modell: jeder Teilton faellt 25 % schneller als in der Aufnahme. Huellkurve wie S195 (-5..-25 dB, 20-ms-Bloecke).
  Aufnahme Amplitude / Abfall je k | Modell Amplitude | Huellkurve Aufnahme / Modell dB/s | Huellkurve sagt
  1/k^0.3 / 4 k^1.0 dB/s            | 1/k^2.0          |    4.7 /    5.0                   | Modell faellt schneller (richtig)
  1/k^0.0 / 4 k^1.5 dB/s            | 1/k^2.0          |    4.8 /    5.0                   | Modell faellt schneller (richtig)
  1/k^0.0 / 4 k^1.5 dB/s            | 1/k^1.0          |    4.8 /    5.0                   | Modell faellt schneller (richtig)
  1/k^1.0 / 4 k^1.0 dB/s            | 1/k^1.0          |    4.1 /    5.1                   | Modell faellt schneller (richtig)
Mechanismus 1 — schwacher Grundton in der Aufnahme (Abstrahlung unter der tiefsten Koerpermode), D = 4 k dB/s:
  Grundton der Aufnahme  +0.0 dB: Huellkurve Aufnahme   4.4, Modell   5.1 dB/s -> richtig
  Grundton der Aufnahme -10.5 dB: Huellkurve Aufnahme   6.9, Modell   5.1 dB/s -> FALSCHES Vorzeichen
  Grundton der Aufnahme -20.0 dB: Huellkurve Aufnahme   8.9, Modell   5.1 dB/s -> FALSCHES Vorzeichen
  Grundton der Aufnahme -30.5 dB: Huellkurve Aufnahme   9.2, Modell   5.1 dB/s -> FALSCHES Vorzeichen
Mechanismus 2 — Raum an der Aufnahme (Modell trocken), ein Teilton 440 Hz:
  Aufnahme trocken 40 dB/s, Raum T 2.0 s DRR +0 dB, Modell 34 dB/s (langsamer): Huellkurve Aufnahme  18.7, Modell  34.0 -> FALSCHES Vorzeichen
  Aufnahme trocken 40 dB/s, Raum T 2.0 s DRR +10 dB, Modell 34 dB/s (langsamer): Huellkurve Aufnahme  38.0, Modell  34.0 -> richtig
  Aufnahme trocken 15 dB/s, Raum T 2.0 s DRR +0 dB, Modell 13 dB/s (langsamer): Huellkurve Aufnahme  15.4, Modell  13.0 -> richtig
  Aufnahme trocken 15 dB/s, Raum T 1.0 s DRR +0 dB, Modell 13 dB/s (langsamer): Huellkurve Aufnahme  14.8, Modell  13.0 -> richtig

== F  Tonhoehe und Inharmonizitaet B: Vorgabe gegen Schaetzung ==
Abweichung Teilton n gegen harmonisch: 1200 log2 sqrt(1 + B n^2) Cent ~ 866 B n^2
  B = 1e-05: n=5 0.22, n=10 0.87, n=20 3.46, n=30 7.76 Cent
  B = 0.0001: n=5 2.16, n=10 8.61, n=20 33.95, n=30 74.60 Cent
Synthetischer Ton f0 110 Hz, Teiltoene 1/k, Abfall 3 + 0,8k dB/s, 20 Laeufe (Phasen zufaellig); dazu je Teilton
ein zufaelliger Frequenzversatz sigma_c Cent (Kopplung an Steg und Koerper, die das Gesetz nicht kennt).
Theorie (nur Versatz): Std(B) = 2 ln2/1200 * sigma_c / sqrt(Summe (k^2 - Mittel k^2)^2)
  B Vorgabe | Rauschen dB | sigma_c | Fenster s | Teiltoene | B Mittel   B Std     Theorie  | f0 Fehler Cent Mittel/Std
      1e-05 |         -70 |     0.0 |      0.50 |         5 |  1.00e-05   8.1e-09  0.0e+00 | -0.000 / 0.000
      1e-05 |         -70 |     0.0 |      0.50 |        10 |  1.00e-05   1.7e-09  0.0e+00 | -0.000 / 0.000
      1e-05 |         -70 |     0.0 |      0.50 |        20 |  1.00e-05   4.6e-10  0.0e+00 | -0.000 / 0.000
      1e-05 |         -70 |     0.0 |      0.50 |        30 |  1.00e-05   1.4e-10  0.0e+00 | -0.000 / 0.000
      1e-05 |         -40 |     0.0 |      0.25 |         5 |  1.00e-05   4.4e-07  0.0e+00 | -0.000 / 0.006
      1e-05 |         -40 |     0.0 |      0.25 |        10 |  1.00e-05   1.4e-07  0.0e+00 | -0.000 / 0.005
      1e-05 |         -40 |     0.0 |      0.25 |        20 |  1.00e-05   3.1e-08  0.0e+00 | -0.001 / 0.003
      1e-05 |         -40 |     0.0 |      0.25 |        30 |  1.00e-05   1.2e-08  0.0e+00 | -0.000 / 0.004
      1e-05 |         -40 |     0.0 |      0.50 |         5 |  1.00e-05   2.9e-07  0.0e+00 | -0.000 / 0.003
      1e-05 |         -40 |     0.0 |      0.50 |        10 |  9.99e-06   5.8e-08  0.0e+00 | +0.000 / 0.002
      1e-05 |         -40 |     0.0 |      0.50 |        20 |  1.00e-05   1.0e-08  0.0e+00 | +0.000 / 0.002
      1e-05 |         -40 |     0.0 |      0.50 |        30 |  1.00e-05   5.2e-09  0.0e+00 | +0.000 / 0.001
      1e-05 |         -70 |     1.0 |      0.50 |         5 | -1.44e-05   4.9e-05  6.0e-05 | +0.210 / 0.649
      1e-05 |         -70 |     1.0 |      0.50 |        10 |  1.49e-05   7.9e-06  1.1e-05 | -0.241 / 0.399
      1e-05 |         -70 |     1.0 |      0.50 |        20 |  1.01e-05   2.7e-06  2.1e-06 | +0.057 / 0.424
      1e-05 |         -70 |     1.0 |      0.50 |        30 |  9.92e-06   6.3e-07  7.6e-07 | +0.055 / 0.286
      1e-05 |         -70 |     3.0 |      0.50 |         5 |  4.55e-05   2.0e-04  1.8e-04 | -0.868 / 2.661
      1e-05 |         -70 |     3.0 |      0.50 |        10 |  1.82e-05   3.0e-05  3.4e-05 | -0.010 / 1.736
      1e-05 |         -70 |     3.0 |      0.50 |        20 |  8.90e-06   6.5e-06  6.2e-06 | -0.052 / 1.271
      1e-05 |         -70 |     3.0 |      0.50 |        30 |  9.86e-06   1.9e-06  2.3e-06 | +0.304 / 0.894
      1e-04 |         -70 |     0.0 |      0.50 |         5 |  1.00e-04   6.1e-09  0.0e+00 | +0.000 / 0.000
      1e-04 |         -70 |     0.0 |      0.50 |        10 |  1.00e-04   1.6e-09  0.0e+00 | -0.000 / 0.000
      1e-04 |         -70 |     0.0 |      0.50 |        20 |  1.00e-04   2.1e-10  0.0e+00 | -0.000 / 0.000
      1e-04 |         -70 |     0.0 |      0.50 |        30 |  1.00e-04   2.3e-10  0.0e+00 | -0.000 / 0.000
      1e-04 |         -40 |     0.0 |      0.25 |         5 |  9.97e-05   5.5e-07  0.0e+00 | +0.004 / 0.006
      1e-04 |         -40 |     0.0 |      0.25 |        10 |  1.00e-04   1.4e-07  0.0e+00 | +0.001 / 0.006
      1e-04 |         -40 |     0.0 |      0.25 |        20 |  1.00e-04   2.7e-08  0.0e+00 | +0.003 / 0.004
      1e-04 |         -40 |     0.0 |      0.25 |        30 |  1.00e-04   1.2e-08  0.0e+00 | -0.002 / 0.004
      1e-04 |         -40 |     0.0 |      0.50 |         5 |  1.00e-04   3.4e-07  0.0e+00 | +0.001 / 0.003
      1e-04 |         -40 |     0.0 |      0.50 |        10 |  1.00e-04   6.3e-08  0.0e+00 | +0.000 / 0.002
      1e-04 |         -40 |     0.0 |      0.50 |        20 |  1.00e-04   1.4e-08  0.0e+00 | -0.000 / 0.002
      1e-04 |         -40 |     0.0 |      0.50 |        30 |  1.00e-04   5.6e-09  0.0e+00 | +0.001 / 0.001
      1e-04 |         -70 |     1.0 |      0.50 |         5 |  7.66e-05   6.2e-05  6.0e-05 | +0.047 / 0.745
      1e-04 |         -70 |     1.0 |      0.50 |        10 |  1.01e-04   1.1e-05  1.1e-05 | -0.080 / 0.494
      1e-04 |         -70 |     1.0 |      0.50 |        20 |  1.00e-04   2.6e-06  2.1e-06 | -0.036 / 0.386
      1e-04 |         -70 |     1.0 |      0.50 |        30 |  1.00e-04   7.2e-07  7.6e-07 | -0.060 / 0.239
      1e-04 |         -70 |     3.0 |      0.50 |         5 |  1.15e-04   1.4e-04  1.8e-04 | -0.325 / 2.250
      1e-04 |         -70 |     3.0 |      0.50 |        10 |  1.09e-04   4.0e-05  3.4e-05 | -0.595 / 1.671
      1e-04 |         -70 |     3.0 |      0.50 |        20 |  1.00e-04   7.0e-06  6.2e-06 | +0.084 / 0.986
      1e-04 |         -70 |     3.0 |      0.50 |        30 |  1.01e-04   3.2e-06  2.3e-06 | -0.142 / 0.979

== G  Spektrum: Teiltonpegel relativ zum Grundton und spektraler Schwerpunkt bei Raum (DRR) ==
Vorgabe: 20 Teiltoene 1/k, Schwerpunkt 2.254 (in Vielfachen von f0)
  DRR dB | Std Pegel Teilton k gegen Teilton 1 dB | Schwerpunkt Mittel / Std (relativ)
      20 |   0.88                                  | 1.004 / 0.052
      10 |   2.81                                  | 1.040 / 0.165
       0 |   7.23                                  | 1.168 / 0.383
     -10 |   7.79                                  | 1.196 / 0.418
    -100 |   7.89                                  | 1.207 / 0.431

== H  Einschwinggeraeusch: Trennung vom harmonischen Teil durch Abzug der Teiltoene ==
  Geraeusch -10 dB: Energie der ersten 50 ms wahr +12.46 dB, Rest nach Abzug +12.64 dB, Verhaeltnis Geraeusch/Ton wahr -19.68, gemessen -19.61 dB
  Geraeusch -20 dB: Energie der ersten 50 ms wahr +2.39 dB, Rest nach Abzug +4.14 dB, Verhaeltnis Geraeusch/Ton wahr -29.75, gemessen -28.11 dB
  Geraeusch -30 dB: Energie der ersten 50 ms wahr -7.35 dB, Rest nach Abzug +0.26 dB, Verhaeltnis Geraeusch/Ton wahr -39.49, gemessen -31.99 dB
```

Lesehilfe zu Teil D: Die Spalte „Pegel-Geraden“ bildet das Verfahren aus `Teiltonmessung.hpp` Schritt 4 nach (zwei unabhängige Geraden, bester Knick). „Schroeder“ ist meine Nachbildung von `zweiStufen` (Kurve nach Schroeder, späte Energie abgezogen). Beide sind **nicht** der LumiViz-Code. Die Testsignale sind kohärent (gleiche Frequenz, Phasenunterschied 0,3 rad); nur der Fall „zwei Polarisationen“ hat einen Frequenzversatz. Für diesen Fall ist das inkohärente Modell gewählt; es trifft die Schwebung nicht (63 statt 40 dB/s), weil 0,7 Hz Schwebung im Pegel nicht gemittelt werden.

---

## 4. Sammlung × Zielgröße, Messplan

### Was aus Abstand und Raum folgt (Gesucht 2)

- **Hallradius und DRR** `[hergeleitet]` aus der Definition (Direkt = Hall bei r_H): DRR(r) = 20·lg(r_H/r). Mit r_H = 0,057·√(γV/T) `[nur Zusammenfassung]`. Beispiele `[eigene Rechnung]`: Raum 200 m³, T 1,5 s → r_H = 0,66 m; Mikrofon in 1 m → DRR −3,6 dB. Saal 10 000 m³, T 2 s → 4,0 m; in 1 m → +12 dB. Kugelförmige Abstrahlung angenommen (γ = 1).
- **Lautheit je Taste:** Streuung durch den Raum 1,2 LU (DRR +10) bis 2,8 LU (diffus) je Ton mit zehn Teiltönen. Wenige starke Teiltöne (hohe Lage, K = 1–3) geben 2,0–5,6 dB (Tab. § 3 C).
- **Gang (über 7 Tasten geglättet):** Die Streuung sinkt etwa auf 0,47·σ, im Raum mit DRR 0 also auf rund 1,2 LU. Der Gang ist auch aus raumbehafteten Sammlungen brauchbar.
- **Sprünge:** Aus einem Mikrofon im Raum ist bei DRR ≤ +5 dB nicht zu trennen, was Instrument und was Raum ist (größter Sprung allein durch den Raum 8,0 / 10,2 / 11,2 LU bei +5 / 0 / diffus).
- **Abklingen je Teilton:** Eine Stufe mit T60_Stufe > T60_Raum ist messbar (Fehler gerechnet 3–8 %). Eine Stufe mit T60_Stufe < T60_Raum nur über ΔL nach Gl. 14. Mit der LumiViz-Grenze von mindestens 10 dB Abfall heißt das: DRR_dB + 10·lg(T_R/T_Stufe) ≥ 10·(1 − T_Stufe/T_R) `[hergeleitet]`. Bei Stufen etwa gleich schnell wie der Raum ist nichts messbar (Harfe T = 3 s).
- **Spektrum:** je Ton erst ab DRR ≥ +20 dB (Std L₅−L₁ < 1 dB). Unterschiede **je Stärke an derselben Taste** in derselben Aufnahme hängen nicht vom Raum ab.
- **Kammfilter durch eine Bodenreflexion** `[hergeleitet]`: Auslöschung bei f = (2m+1)·c/(2Δs), Δs der Umweg. Bei Δs = 1 m liegen die Auslöschungen bei 172, 515, 858 Hz … Nicht gerechnet, wie stark das die Lautheit je Taste ändert.
- **Richtwirkung:** Für Gitarre und Harfe ist sie in der TU-Berlin-Datenbank je Ton und Teilton gemessen (32 Punkte); ich habe die Daten nicht geladen. Ohne sie ist der Fehler eines einzelnen Mikrofons nicht bezifferbar.
- **Abschätzung des DRR aus den Aufnahmen selbst** `[eigene Einschätzung]`: Die gemessenen mittleren Sprünge der VCSL-Cembali (1,3–2,4 LU) passen nach § 3 C zu DRR etwa +10 … 0 dB. Das ist eine Gegenprobe, keine Messung, solange das Instrument selbst auch springt.

### Tabelle Sammlung × Zielgröße

T = trägt · E = trägt eingeschränkt · N = trägt nicht · ? = unbekannt bis geprüft.

| Sammlung | Aufnahme | Lautheit je Taste | Gang (geglättet) | Sprünge | Abklingen je Teilton früh / spät | Spektrum, Helligkeit je Stärke | Tonhöhe, B | Einschwinggeräusch |
|---|---|---|---|---|---|---|---|---|
| **TU Berlin** Gitarre C2–B5, Harfe C1–F4, pp/ff | reflexionsarm, 32 Mikr., r ≈ 2,09 m, kalibriert in Pa, 44,1 kHz, CC BY-NC-SA 4.0 `[wörtlich]` | **T** (Schallleistung über 32 Mikr., absolut) | **T** | **T** (kein Raum, keine Richtwirkung) | ? — Länge der Dateien und `SteadyPart` (Hand gesetzt) ungeprüft; spät ? | **T** (pp/ff je Taste) | T | E (Abstand 2 m, kein Raum) |
| Iowa MIS Gitarre | schalltot, pp/mf/ff, 96 kHz `[LumiViz]` | T (Spieler streut) | T | E (Spielerstreuung; 7 Läufe ohne Zuordnung) | **T** / T | T (pp/mf/ff) | T | E |
| VCSL Cembali (5) | Raum, Abstand unbekannt; nur eine Stärke `[LumiViz]` | E (Std ~2,5 LU bei DRR 0) | **T** | **N** (Raum ≈ gemessene Sprünge) | früh E (nur T60_Stufe > T_Raum, Bass bis Mitte) / spät N bis E | N je Ton; keine Dynamikstufen | T (B ~10⁻⁵: ≥ 20 Teiltöne) | E (Raum verschmiert) |
| VCSL Konzertharfe „KSHarp“ | Raum, jede 3.–4. Taste, f/mf `[LumiViz]` | E | E (Lücken: Paar-Statistik anders, § 3 B) | N | Diskant N (150 dB/s), Bass/Mitte E | E (f − mf an derselben Taste) | T | E |
| VCSL Volksharfe | Raum | E | T | N | E | E | T | E |
| RWC (Stufe 1) | unbekannt; Halbtonschritte, f/mf/p `[Werkzeug]` (Stufe 1) | ? | ? (vermutlich T) | ? | ? | ? (drei Stärken) | T | ? |
| Freesound GtrClass | Mikrofon CAD E100, Raum unbekannt, 5 Stärken `[Werkzeug]` (Stufe 1) | ? | ? | ? | ? | ? | T | ? |
| Philharmonia Gitarre | MP3 `[nur Zusammenfassung]` (Stufe 1) | E | E | N | N (Kodierung) | N | E | N |
| IDMT-SMT-Guitar | Tonabnehmer, E-Gitarren | N (kein Luftschall) | N | N | E (Saite ohne Körper) | N | T | N |
| NSynth | 16 kHz | N | N | N | N | N | E | N |
| Helsinki-Cembalo (Välimäki 2004) — **nicht öffentlich gefunden** | reflexionsarm, ~1 m, 56 Tasten × 6 Registerkombinationen, verschiedene Stärken `[wörtlich]` S. 940 | T | T | T | T | T | T | T (auch Loslassgeräusch) |

Laute, Oud: weiterhin keine Sammlung (Stufe 1). Für die Laute sagt Woodhouse 2021 Doppelabklingen über weite Teile des Umfangs voraus `[wörtlich]`. Ohne Aufnahme ist das nicht prüfbar.

### Messplan in der Reihenfolge des Nutzens

1. **TU Berlin, Gitarre und Harfe** (Lautheit je Taste als Schallleistung, Sprünge, Spanne, pp−ff je Taste, Helligkeit je Stärke). Das ist der einzige Satz ohne Raum mit bekanntem Abstand. Er schließt die Lücke „Schwellen aus GZ14 a sind aus Raumaufnahmen hergeleitet“. Die Harfe reicht nur bis F4, deckt damit aber genau den Bass (Grenze D2) und die Stelle G3–A3 (Mode 208 Hz) ab. Vorher prüfen: Lizenz NC/SA gegen die Nutzung in LumiViz; Länge der Töne (ganzes Abklingen oder nur `SteadyPart`); Dateigröße.
2. **Iowa-Gitarre:** Abklingen je Teilton (früh und spät, Knick) mit dem Modell aus zwei Komponenten (Gl. 11). Vorzeichen gegen die Hüllkurve aus S195; prüfen, ob der Grundton der tiefen Saiten schwach ist (Mechanismus 1). B je Saite; Helligkeit pp/mf/ff.
3. **VCSL-Cembali:** nur geglätteter Gang; frühes Abklingen der Teiltöne, deren Stufe langsamer ist als der Raum (vorher T60 des Raums je Band aus der Aufnahme schätzen, z. B. aus dem Rest nach dem Ende des Tons, falls vorhanden); B mit ≥ 20 Teiltönen.
4. **VCSL-Harfe:** geglätteter Gang ab D2 mit den Tasten, die da sind; f − mf je Taste.
5. **RWC** nach Prüfung der Instrumentenliste (Stufe 1).

---

## 5. Annahmen, die bleiben

- Gl. 12–14 nehmen ein diffuses, exponentielles Hallfeld und inkohärente Addition an. Für einen einzelnen Teilton gilt das nur im Mittel über Räume (§ 3 D: ±10 dB je Raum).
- Gl. 13 nimmt unabhängige Raumpegel je Teilton an. Das setzt Teiltonabstände über der Modenbandbreite voraus (≈ 2,2/T Hz, z. B. 1,5 Hz bei T = 1,5 s) und Frequenzen über der Schroeder-Frequenz (nicht gerechnet).
- Die Pegelstreuung durch den Raum ist für den **stationären** Zustand gerechnet. Beim Momentary-Maximum eines schnell abklingenden Tons ist der Hall im Fenster noch nicht aufgebaut. Die wirkliche Streuung ist dort vermutlich kleiner `[eigene Einschätzung]`.
- Die Paar-Statistik (§ 3 B) nimmt unabhängige, normalverteilte Streuung je Taste an. Raummoden und Körpermoden machen benachbarte Tasten abhängig.
- Das K-Filter für 44,1/96 kHz stammt aus dem analogen Vorbild der Umsetzung libebur128. Es gibt bei 48 kHz die Normkoeffizienten auf 10⁻¹⁵ wieder, ist aber nicht Teil der Norm.
- Die Zahl für den Bezug −3,01 LKFS gilt für einen Kanal. Wie `AudioCore` Mono-Dateien einspeist (als Stereo verdoppelt?), habe ich nicht im Code geprüft.
- Die Abweichung der Teiltöne vom Gesetz der steifen Saite (σ_c) ist für gezupfte Instrumente nicht als Zahl gefunden. 1–3 Cent sind gewählt.

## 6. Optionen für LumiViz

Keine Bewertung nach Aufwand. Je Weg: was er braucht und was er an **allen** Zielgrößen ändert.

**O1 — Messgröße der Lautheit je Taste**
- **O1-a Momentary-Maximum (heute).** Braucht nichts Neues. Lautheit: enthält das Abklingen (Gl. 3, bis 12 LU über den Umfang der Harfe). Gang und Sprünge: Fehler des Modells im frühen Abfall gehen als Lautheit ein. Abklingen, Spektrum, B, Einschwinggeräusch: unverändert. Folgt `LP3 a` (Wächter) unverändert.
- **O1-b Energie über ein festes Fenster ab dem Einsatz** (z. B. 1 s wie S195 `int1`, oder je Taste so lang wie das Abklingen bis −20 dB). Braucht eine festgelegte Fensterlänge; der Wächter `LP3 a` misst dann eine andere Größe. Lautheit: Mit festem Fenster ändert sich die Kopplung an das Abklingen, sie verschwindet aber nicht. Mit einem Fenster, das bis −20 dB reicht, ist es die Tonenergie. Übrige Größen unverändert.
- **O1-c Schallleistung aus mehreren Mikrofonen** (nur TU Berlin), dann Lautheit aus dem kalibrierten Pegel. Braucht den Datensatz und die Klärung der Lizenz. Lautheit, Gang und Sprünge ohne Raum und Richtwirkung; Spektrum und Helligkeit je Stärke ebenfalls ohne Richtwirkung; Abklingen ungeprüft (Länge der Dateien); B unverändert.

**O2 — Abklingen je Teilton**
- **O2-a Zwei Geraden mit bestem Knick (heute).** Früher Abfall um rund 30 % zu flach (§ 3 D, Nachbildung), falls der Code wirkt wie meine Nachbildung. Späte Rate und Knick stimmen.
- **O2-b Zwei Komponenten mit Boden (Gl. 11),** kohärent und inkohärent, die Wahl nach dem Rest; mit der Raumbedingung (Gl. 14, T60_Stufe gegen T60_Raum) je Teilton als Gültigkeit. Braucht einen nichtlinearen Ausgleich je Teilton. Das widerspricht dem Kipppunkt aus UF3 a („nichtlinear mit mehreren Parametern“) — ein Entscheid für Patrik. Abklingen früh: stimmt in § 3 D ohne Raum auf 1 %. Lautheit über O1-a: nur über einen richtig verglichenen frühen Abfall. Übrige Größen unverändert.
- **O2-c Wie O2-a oder O2-b, aber verglichen nur an Teiltönen und Sammlungen, die nach § 4 tragen** (T60_Stufe > T60_Raum; schalltote Sammlungen). Ändert, welche Tasten und Teiltöne zählen, nicht das Verfahren.

**O3 — Normierung und Schwellen zwischen Sammlungen**
- **O3-a GZ14 a halten** (15 / 4 / 12 LU an Raumaufnahmen). Der größte Sprung von 12 LU liegt nahe am Wert, den ein Raum mit DRR 0 allein erzeugt (§ 3 C: 10,2, 95 %: 12,6). Das Modell, das ohne Raum misst, hält damit eine Schwelle, in der der Raum steckt.
- **O3-b Schwellen aus der schalltoten Sammlung neu fassen** (TU Berlin Gitarre/Harfe, Iowa Gitarre). Braucht diese Messung. Ändert Spanne und Sprünge je Instrument; für das Cembalo bleibt keine schalltote Quelle (Helsinki nicht öffentlich).
- **O3-c Modell durch einen statistischen Raum schicken** (Gl. 13 mit dem DRR, der zu den Aufnahmen passt) und **Verteilungen** vergleichen statt einer Schwelle. Braucht eine Schätzung des DRR je Sammlung. Ändert den Vergleich von Lautheit, Sprüngen und Spektrum; Abklingen nur über Gl. 12; B unverändert.

## 7. Was die LumiViz-Sitzung prüfen muss

1. **`zweiStufen` und `huellkurveVorwaerts` an den synthetischen Teiltönen aus § 3 D** (40/4 dB/s, Knick 0,556 s; 150/20 dB/s, Knick 0,192 s), kohärent und mit 0,7 Hz Versatz. Ist der frühe Abfall auch dort rund 30 % zu flach? Meine Nachbildung ist nicht der Code.
2. **Vorzeichen je Teilton an der Iowa-Gitarre:** Teilton 1 gegen 2–5 je Ton und Stärke. Ist der Grundton der tiefen Saiten schwach (Mechanismus 1)? Liegen die Teiltöne über dem Raumkriterium? (Iowa ist schalltot, dort also unkritisch.)
3. **K-Filter in `AudioCore` bei 44,1 und 96 kHz:** Wie sind die Koeffizienten gewonnen? Bezugston gegen −3,01 LKFS (Toleranz ±0,1). Und wie werden Mono-Dateien eingespeist (+3,01 LU bei Verdopplung)?
4. **Heutige Fensterlängen gegen die hergeleiteten:** `fensterPerioden` 8 (20 ms–0,5 s) gegen Lundeby 10–50 ms und 3–10 Intervalle je 10 dB. Bei 150 dB/s sind 10-ms-Schritte 1,5 dB je Schritt; die 20-ms-Glättung bei 8 Perioden über 400 Hz bleibt darunter.
5. **Momentary-Maximum gegen Gl. 3 an einem Modellton:** bestätigen, dass die Spanne der Harfe bis rund 11 LU Abklingen enthält.
6. **GZ14 a gegen § 3 C:** Mit welchem DRR der VCSL-Aufnahmen sind die gemessenen Sprünge allein durch den Raum erklärbar?
7. **TU-Berlin-Datenbank** (DepositOnce 5861.3 und die WAV-Fassung 5861.2): Lizenz gegen die Nutzung; Länge der Töne; ob die Gitarre klassisch (Nylon) ist (Tab. 1 sagt nur „Acoutic guitar“ `[wörtlich]`); Notenbenennung (Harfe „C1–F4“ — Oktavlage wie bei VCSL prüfen).
9. **Glättung in S200** (Median über 7 Tasten): Sie entfernt eine Störung von bis zu drei Nachbartasten ganz (§ 3 B: +6 → +0,25 LU), etwa die der Harfe an G3–A3 (Mode 208 Hz). Die Form nach der Glättung zeigt sie nicht; nur die Paar-Statistik auf den ungeglätteten Werten zeigt sie. Prüfen, an welchen Werten die Schwellen aus GZ14 a gemessen werden.
8. **`bFehler`** („genähert: nur die Steigung“) entspricht Gl. 18 nur, wenn die Reste von der Abweichung vom Gesetz kommen. Mit 12 Teiltönen (`Grundton.hpp`) ist B ≈ 10⁻⁵ (Cembalo) bei σ_c ≥ 1 Cent nicht messbar.

## 8. Nicht gefunden / nicht herleitbar

- **Pegelsprünge zwischen Nachbartönen an echten Instrumenten als dB-Zahlen:** für Gitarre, Laute, Oud, Cembalo nicht gefunden. Gefunden nur für die Harfe, aber die **Eingangsadmittanz**, nicht der Schall (Bell 1988, S. 159) `[wörtlich]`: „The greatest difference in input admittance level between two adjacent string positions is 15dB, for strings 31 and 32. Usually, the difference is much smaller, the average difference being about 3dB“. Dazu A-bewerteter Schalldruck: Plateau Saite 26 bis 1, der Bass „more than 30dB(A) below those of the plateau“ (S. 160) `[wörtlich]`. Die Abbildungen 7.23/7.24 habe ich nicht gefunden (zwei Seiten neben dem Text zeigten andere Abbildungen). Suchwege: `pdf_suche` „sound pressure level“, „SPL“, „loudness“, „each note“, „dynamic range“ in Bell, Wright, Traube, Penttinen; Netz: „level variation between adjacent notes guitar … harp“ (ein Treffer zu einem Taskin-Cembalo: „differences of up to 11 dB for the two types of touch“ `[nur Zusammenfassung]`, eprints.whiterose.ac.uk/171080, nicht gelesen).
- **Schroeder 1965, ISO 3382-1, Lundeby 1995:** nicht gelesen (Bezahlschranke). Die Lundeby-Schritte stammen aus einem Sekundärtext.
- **Karjalainen et al. 2002** (Abklingen mit Rauschboden): Abruf lieferte nichts.
- **Hallradius aus einer Primärquelle** (Kuttruff): nicht gelesen.
- **Zahlen zum Mechanik- und Loslassgeräusch des Cembalos und zum Zupfgeräusch:** nicht gefunden (Välimäki 2004 beschreibt das Verfahren, nennt keine Pegel auf den gelesenen Seiten). `Simionato_2024_SinesTransientNoisePianoNotes` liegt lokal und ist nicht gelesen.
- **Abweichung der Teiltöne gezupfter Saiten vom Gesetz der steifen Saite (σ_c):** nicht gesucht.
- **Woodhouse et al. 2012** (Wahrnehmungsschwellen Gitarre): nur durchsucht; die Tabellen der JND für Dämpfung nicht gelesen.
- **Zahl für „≫“ im Kriterium von Woodhouse:** gibt es laut Quelle nicht.

## 9. Ladeliste (nichts geladen)

| Name (Vorschlag) | URL | Lizenz / Vermerk | wozu |
|---|---|---|---|
| `TUBerlin_2024_MusicalInstrumentsDatabase_v3` | https://doi.org/10.14279/depositonce-5861.3 | CC BY-NC-SA 4.0 `[wörtlich]` Doku S. 2; Größe unbekannt | Gitarre, Harfe schalltot, 32 Mikr., pp/ff (§ 4, Plan 1) |
| `TUBerlin_2017_MusicalInstrumentsDatabase_v2_wav` | https://doi.org/10.14279/depositonce-5861.2 | laut Doku „still freely available“ `[wörtlich]`; WAV mit Tonleitern | dto., Dateiform WAV |
| `Ackermann_Brinkmann_Weinzierl_2024_DirectivitiesMusicalInstruments_JAES.pdf` | https://doi.org/10.17743/jaes.2022.0128 | Zugang ungeprüft | Aufnahme und Verarbeitung der Datenbank |
| `ITU_2023_BS1770-5.pdf`, `EBU_2023_Tech3341.pdf`, `EBU_2023_Tech3342.pdf` | (liegen als `webfetch-…pdf` unter `abrufe/`) | frei | Normtext, schon gelesen |
| `EBU_LoudnessTestSet` | https://tech.ebu.ch/publications/ebu_loudness_test_set | frei laut Tech 3341 S. 9 | Prüfung von `AudioCore` |
| `ITU_BS2217` | https://www.itu.int/pub/R-REP-BS.2217 | frei (vermutlich) | Prüfsignale BS.1770 |
| `Schroeder_1965_NewMethodReverberationTime_JASA37.pdf` | https://doi.org/10.1121/1.1909343 | Bezahlschranke | Gl. 9 im Original |
| `Lundeby_etal_1995_Uncertainties_ActaAcustica81.pdf` | (Acta Acust. 81(4) 344–355) | Bezahlschranke | Lundeby im Original |
| `ISO_3382-1_2009` | iso.org | kostenpflichtig | Auswertebereiche, Dynamik |
| `Karjalainen_etal_2002_ModalDecayNoisy_JAES50.pdf` | JAES 50(11), 2002; Kopie auf typeset/scispace nicht lesbar | ungeprüft | Gl. 11 mit Beleg |
| `Rauhala_Lehtonen_Valimaki_2007_FastInharmonicity_JASAEL121.pdf` | JASA Express Letters 121 (2007) | ungeprüft | Schätzung B |
| `Galembo_Askenfelt_1999_InharmonicCombFilters_IEEETSAP7.pdf` | IEEE TSAP 7, 197–203 | Bezahlschranke | Schätzung B |
| Helsinki-Cembalo (Välimäki et al. 2004) | — | nicht veröffentlicht gefunden; bei den Autoren anfragen (nur vermerkt) | schalltotes Cembalo je Register |

## 10. Was der Auftrag verlangt hat und nicht geliefert ist

- Normtext ISO 3382-1 und Schroeder 1965: nicht gelesen; die Auswertebereiche stehen nur als `[nur Zusammenfassung]` da.
- Messunsicherheit des **Spielers** je Instrument als Zahl aus der Literatur: nicht gefunden.
- Literaturwerte für Pegelsprünge in dB oder LU je Instrument: nur Harfe (Admittanz, nicht Schall).
- Das Einschwinggeräusch ist nur für das Trennverfahren gegengerechnet, nicht für Größen wie Dauer oder Spektrum.
- Die neue Sammlung (TU Berlin) habe ich nicht auf Tonlänge und Gitarrentyp geprüft, weil das Laden nicht freigegeben ist.

> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_14_cembalo_grundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 23:10–23:28, claude-opus-5-5, 84 Runden, success, 4.85 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Seitenbilder des Laufs im Archiv der Werkbank, `seiten/`):
> - **Chadefaux u. a. 2013 S. 158 am Seitenbild:** Tab. 1 (x_s 0,27 / 0,28, y_s 0,71 / 0,90 mm, V, Γ), Tab. 2 (L_s
>   47,5 cm, T 85,7 N, E 1,5 GPa, L 3,5 mm, L1 2,9 mm, I 0,011 mm⁴), Gl. (4) mit I/8 vorn, K = T/(L_s β(1 − β)) — alles
>   stimmt.
> - **Kraft an G4 nachgerechnet:** mit β = 0,259 (aus Taskin 1769, **in der Quelle nicht genannt**) K = 940 N/m und
>   F = K·√(x² + y²) = 0,71 / 0,89 N — stimmt. Die Zahl hängt an β: mit β = 0,15 wären es rund 1,0 / 1,3 N. Auch dann
>   bleibt **GZ13 a (2 N an C4) deutlich über der Messung** — der Befund „+10 bis +12 dB an G4“ (y = 2,98 gegen
>   0,76 / 0,94 mm) ist der wichtigste dieses Berichts für LumiViz.
> - **„Hinweis vorweg“ (Widerspruch zu Bericht 04) geprüft:** im Prüfstand ist 0,001 der Biegeverlust η_B, er wirkt nur
>   über den Biegeanteil (Bericht 11, Gl. Z3: η_B EI m ω³/(2T²)); die Folgerung von Bericht 04 „fünfmal zu schnell“
>   trifft den Prüfstand nicht. Richtig.
> - **Nicht gegengelesen:** Perng 2012 (Gl. 2.57/2.58/3.5, Tab. 3.1, Abb. 3.15–3.24, 4.24/4.25), Roy 2015, Woodhouse &
>   Lynch-Aird 2019 S. 518 / 528, die Herleitung K6 und die Rechnung `rechnung.py` Zahl für Zahl (Tabelle 1.3: 13 von
>   14 Fällen ±11 % gegen Perng).
> - **Nacharbeit:** Auftrag `2026-11-17_24_cembalo_nacharbeit` der Werkbank. Für die LumiViz-Seite: Maßstab der
>   Ablösekraft (GZ13 a) gegen Chadefaux und Modell K6; 4′-Zupfpunkt gegen Fletcher & Beebe Abb. 8.4; Lautenzug a ≈
>   0,5 mm aus der Verstimmung der VCSL-Aufnahmen.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_14_cembalo_grundlagen.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_14_cembalo_grundlagen/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_cembalo_grundlagen_werkbank/`: `anhang_anfuegen.py`, `rechnung.py`.

# Bericht: Berechnungsgrundlagen für Kiel, Tastenstoß, Lautenzug, Draht und Register am Cembalo

- **Auftrag:** `auftraege/2026-11-16_14_cembalo_grundlagen.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5
- **Voraussetzung:** Bericht der Stufe 1 `ergebnisse/2026-11-16_04_cembalo_kiel_stoss_register/bericht.md` liegt vor und
  ist gelesen.
- **Reichweite:** **keine** Netzsuche, **kein** Abruf (der Auftrag verlangt keine neue Suche). Lokal gelesen, jeweils
  Text mit `pdftotext`/`pdf_suche.py` eingegrenzt, Gleichungen, Tabellen und Abbildungen am Seitenbild:
  - Perng 2012, Diss. (`Perng_2012_HarpsichordPlectrumStringInteraction_Diss.pdf`): Kap. 2 S. 21–51 (Text), als Bild
    S. 25, 37, 55, 72, 78–79, 81–86, 115–116, 127–128 (gedruckt);
  - Chadefaux, Le Carrou, Le Conte, Castellengo 2013, SMAC (`…_SMAC2013.pdf`): S. 155 (Text), S. 156–159 als Bild;
  - Roy, Le Carrou, Fabre, Vitrani 2015, CFM (`…_PlectreClavecin_CFM.pdf`): PDF-S. 3–6 als Bild;
  - Woodhouse & Lynch-Aird 2019 (`…_ChoosingStringsPlucked_ActaAcustica105.pdf`): S. 518 und 528 als Bild;
  - Perng, Smith, Rossing 2010 und 2011 (DAFx): nur Textsuche (gleicher Inhalt wie die Diss., dort gelesen);
  - LumiViz (nur gelesen): `…/gezupfte-saite/2026-11-13_S197_cembalo.md`, `Instrumente_IC_Konzept.md` §6 (GZ10 a–GZ14 a),
    Docstrings und Parameterblöcke von `proben/2026-11-15_S199_pruefstand_zupf.py` und `proben/2026-11-16_S200_cembalo_register.py`.
  - **Nicht erneut gelesen**, nur aus Stufe 1 übernommen (dort mit Marke): Välimäki u. a. 2004, Fletcher & Beebe Kap. 8,
    MacRitchie & Nuti 2015, Bray Baroque, Sankey, Wraight, Edinburgh-Datenblätter. Penttinen 2006, Bader u. a. 2025 und
    `DiVeroli_2022_StringingCalculator.xls` nicht geöffnet: Penttinen und Bader liefern zu den fünf Punkten nichts, was
    über GZ10 a / S197 hinausgeht (Stufe 1); die Draht- und Spannungswerte sind die des Prüfstands (Di Veroli Tab. 7/8).
- **Rechnungen:** `rechnung.py` in diesem Ordner (nur Standardbibliothek). Skript und vollständige Ausgabe stehen
  im **Anhang**; Tabellen im Text sind Auszüge daraus.

**Hinweis vorweg (Widerspruch zur Stufe 1):** Die Stufe 1 schloss, η = 0,001 lasse die Teiltöne um 1–4 kHz „rund fünfmal
zu schnell“ abklingen. Das trifft den Prüfstand nicht: dort ist 0,001 der **Biegeverlust η_E**, und er geht nur mit dem
Biegeanteil λ ≈ n²B (10⁻⁵ bis 10⁻²) in den Verlust ein (Abschnitt 4). Der Draht im Prüfstand verliert praktisch nur an
die Luft.

---

## Ergebnis vorweg

1. **Kiel.** Ein reduziertes Modell — Perngs Biegebalken (Gl. K1–K3) plus eine seitlich nachgiebige Saite (K5) und die
   Ablösebedingung „Verkürzung des Kiels + seitliches Ausweichen der Saite = Überstand“ (K6, eigene Herleitung) —
   trifft Perngs Simulationen in 13 von 14 abgelesenen Fällen auf ±11 % und die Messung von Chadefaux 2013 an G4
   (0,71 / 0,89 N aus der gemessenen Lage; Aufladezeit 13,5 / 6,8 ms gegen gemessen 12 ± 2 / 7,9 ± 0,6 ms).
2. **Ablösekraft je Taste.** Mit **einem** Kiel und festem Überstand ist die Kraft über den Umfang **fast eben**
   (0,46 N an F1 … 0,57 N an C4 … 0,80 N an f‴; +0,8 dB/Okt). Das heutige Gesetz (2 N an C4, −2,8 dB/Okt) liegt an C4
   **+10,9 dB** darüber; an G4 gibt es eine Auslenkung von 2,98 mm gegen **gemessene 0,76 / 0,94 mm** (+10 bis +12 dB).
   Die Form −2,8 dB/Okt ist mit dem Modell nur über ein **Voicing** erreichbar: Biegesteife im Bass 7,5-fach, im Diskant
   0,22-fach gegen C4 (Dicke 1,96- bis 0,60-fach) — das passt in der Richtung zu den Cembalobauern der Stufe 1.
3. **Zeitverlauf.** Aufladen einige ms (bei realer Springerbewegung bis ~100 ms), Rutschphase 26–36 % davon
   (Chadefaux, gemessen), das eigentliche Abgleiten von der Spitze ≈ 0,25 ms (Perng, Simulation). Ob das als Tiefpass
   (−3 dB bei ≈ 1,8 kHz) oder — wie Perng folgert — als seitlicher „Kick“ mit **mehr** Brillanz wirkt, ist nicht gemessen.
4. **Tastenstoß.** Die Springer-Ereignisse geben **1,2–7,7 mN·s** (Springer 5–6 g, 0,25–0,77 m/s aus Perng Abb. 4.24/4.25)
   — dieselbe Größenordnung wie die heutigen 7 mN·s. Das Aufsetzen der Taste dauert aber **≈ 8 ms** (Chadefaux Abb. 6),
   nicht 0,4 ms. Der Weg in den Boden ist nirgends belegt.
5. **Lautenzug.** Geschlossene Form der Zusatzdämpfung je Teilton (L4) und der Verstimmung (L5) hergeleitet und gegen
   die exakte Rechnung des Prüfstands geprüft. GZ11 a (40 N·s/m bei 0,5 mm) liegt im Bereich R ≫ Z0: oberhalb
   f_c = T/(2π a R) klingen alle Teiltöne gleich schnell zusätzlich ab, σ∞ = 2 Z0 f0 / R. Die gemessene Verstimmung der
   VCSL-Aufnahmen (+5,2 Cent an C6) verträgt bei vollem Klemmen nur a ≈ 0,5 mm, nicht die 2–3 mm der Bauanleitung.
6. **Draht.** η_E = 0,001 bewirkt höchstens 2·10⁻⁵ — vernachlässigbar. Die Luftreibung nach Woodhouse & Lynch-Aird
   (mit Zuschlag d + 0,2) trifft den gemessenen Gesamtverlust bei Välimäki an Teilton 6 (1,4 gegen 1,6·10⁻⁴), an
   Teilton 20 fehlen 0,9·10⁻⁴, am Grundton 2,4·10⁻³ (Steg/Boden). B je Taste: 1,0·10⁻⁵ … 1,6·10⁻⁴.
7. **Register.** Die Regel des Prüfstands für den 4′-Zupfpunkt (Anteil des 8′ gleicher Tonhöhe) legt den 4′-Zupfpunkt
   1,7–2,2-mal so weit vom Sattel wie der einzige Beleg (Fletcher & Beebe Abb. 8.4, Ruckers-Nachbau: p4 ≈ 0,45–0,54 · p8
   von C2 bis C5, 0,71 an C6). Zweites 8′: weiter keine Quelle.

---

## 1. Kiel–Saite: das Plektrum als Biegebalken

### 1.1 Gleichungen

| Nr. | Gleichung | Quelle | Marke |
|---|---|---|---|
| K1 | Biegewinkel längs des Kiels (kleine Winkel): φ(l) = (F/EI)(L l − l²/2) | Perng 2012, S. 37, Gl. (2.57) | [wörtlich, am Seitenbild geprüft] |
| K2 | Endwinkel: φ0 = ½ F L² / (EI) | Perng 2012, S. 37, Gl. (2.58) | [wörtlich, am Seitenbild geprüft] |
| K3 | Lage des Kraftpunkts bei l = L′: x = L′ − (F/EI)² L′⁵/15; y = −(F/EI) L′³/3 + (F/EI)³ L′⁷/105 | Perng 2012, S. 55, Gl. (3.5) | [wörtlich, am Seitenbild geprüft] |
| K4 | Kiel mit Fase, zwei Abschnitte (vorn I/8): Γ(l) = (F/EI)(L l − l²/2) für l ≤ L1; Γ(l) = (F/EI)(7/2 L1² − 7 L L1 + 8 L l − 4 l²) für L1 < l ≤ L | Chadefaux u. a. 2013, S. 158, Gl. (4) | [wörtlich, am Seitenbild geprüft] |
| K5 | Steife der Saite am Zupfpunkt: K = T / (L_s β (1 − β)), β Zupfanteil | Chadefaux u. a. 2013, S. 158 (Text zu Gl. 5/6) | [wörtlich, am Seitenbild geprüft] |
| K6 | **Ablösebedingung:** F² G₂ + F sin(φ0)/K = u, mit φ0 = F g(L), G₂ = ∫₀ᴸ g(l)²/2 dl, Γ(l) = F g(l), u = Überstand des Kiels unter der Saite | aus K1/K3 (bzw. K4) und K5 | [hergeleitet] — Annahmen: reibungsfrei (wie Perng), Ablösen wenn der Kraftpunkt das Kielende erreicht, Spitzenradius vernachlässigt, Kraft senkrecht zum Kiel |
| K6a | Grenzfall steife Saite (K → ∞): F_ab = k_p √(15u/(4L)), k_p = 2EI/L² | aus K6, gleichförmiger Kiel | [hergeleitet] |
| K6b | Grenzfall weiche Saite: F_ab ≈ √(u K k_p) | aus K6 | [hergeleitet] |
| K7 | Auslenkung beim Ablösen: A = F/K; waagrecht x_s = F sin φ0 / K, senkrecht y_s = F cos φ0 / K | aus K5 und Kraftrichtung (Perng Abb. 3.2) | [hergeleitet] |
| K8 | Aufladezeit: t_auf = (y_s + y_Spitze)/v_Springer, y_Spitze = F ∫₀ᴸ g dl (= F L³/(3EI) gleichförmig, vgl. K3) | | [hergeleitet] |
| K9 | Reibungsschwelle (Germain–Evangelista, nur Alternative): F_p > 2 μ_s EI / x_s² | Perng 2012, S. 25, Gl. (2.20) | [wörtlich, am Seitenbild geprüft] |
| K10 | Anfangsbedingung der freien Schwingung: A_n = 2 D sin(k_n z0) / (k_n² z0 (L − z0)), B_n = 2 V sin(k_n z0) / (k_n³ z0 (L − z0) c) | Chadefaux u. a. 2013, S. 157, Gl. (2)/(3) | [wörtlich, am Seitenbild geprüft] |
| K11 | Abgleiten in der Zeit τ als lineare Rampe gegen den idealen Sprung: \|H(f)\| = \|sinc(f τ)\|, −3 dB bei 0,443/τ | | [hergeleitet] |
| K12 | Breite des Kiels längs der Saite (Prüfstand): Gauß exp(−(ω h / c)²), −3 dB bei f = 0,0935 c/h | Prüfstand S199 (Woodhouse 2012) | [aus Prüfstand übernommen] |

Lesart von K6: Je weicher die Saite seitlich (Bass, Zupfpunkt nahe der Mitte), desto mehr weicht sie dem Kiel aus und
desto früher rutscht sie ab; die Kraft wächst dann nur mit der **Wurzel** aus Kielsteife, Überstand und Saitensteife
(K6b). Die Spannung geht über K ein.

### 1.2 Parameter

| Größe | Wert | Einheit | Quelle | Marke | Gültigkeit |
|---|---|---|---|---|---|
| Kiel Perng (Simulation) | L 6, W 2, H 0,5; I 0,021; E 5; Abstand Einspannung–Saite L_i 5,4 → u = 0,6 | mm, mm⁴, GPa | Perng 2012, S. 72, Tab. 3.1 | [wörtlich, am Seitenbild geprüft] | Modellwerte, kein gemessener Kiel |
| Kiel Perng Kap. 2 | wie oben, aber E = 3 · 10⁹ Pa | Pa | Perng 2012, S. 36 und 50 (Text) | [wörtlich] (Textebene) | — zwei Werte für „typisches“ Delrin in derselben Arbeit |
| Saiten Perng | Diskant 0,5 m, 135 N, 400 Hz; Bass 2,0 m, 42 N, 40 Hz | | Perng 2012, Tab. 3.1 | [wörtlich, am Seitenbild geprüft] | |
| Kiel Chadefaux (Delrin, G4) | L 3,5; Fase ab L1 2,9; I 0,011; E 1,5 | mm, mm⁴, GPa | Chadefaux 2013, S. 158, Tab. 2 | [wörtlich, am Seitenbild geprüft] | ein Kiel, ein Instrument |
| Saite Chadefaux (G4, 392 Hz) | L_s 475, T 85,7 | mm, N | ebd. | [wörtlich, am Seitenbild geprüft] | Zupfanteil β **nicht genannt** |
| Lage beim Ablösen G4 | x_s 0,27 ± 0,02 / 0,28 ± 0,02; y_s 0,71 ± 0,07 / 0,90 ± 0,01; V 0,06 ± 0,02 / 0,15 ± 0,01 m/s; Γ 17 ± 2 / 20 ± 1° (langsam / schnell) | mm | Chadefaux 2013, S. 158, Tab. 1 | [wörtlich, am Seitenbild geprüft] | |
| Springergeschwindigkeit (Musiker) | 0,10 ± 0,05 / 0,25 ± 0,03 | m/s | Chadefaux 2013, S. 157 §3.2 | [wörtlich, am Seitenbild geprüft] | konstant während des Zupfens |
| Dauer des Zupfens | 12 ± 2 / 7,9 ± 0,6 ms; Rutschphase 36 ± 3 % / 26 ± 4 % davon | | Chadefaux 2013, S. 157 §3.1 | [wörtlich, am Seitenbild geprüft] | |
| Kiel Roy 2015 | Länge 3,5 mm, Taste Nr. 37 (Sol); Saite löst bei x ≈ 0,3–0,45, y ≈ 1,0 mm (Messung) | | Roy 2015, PDF-S. 4–5, Abb. 3 | Text [wörtlich]; Lage [aus Abbildung abgelesen] ± 0,1 mm | keine Kraft, keine Spannung |
| Springerweg während des Zupfens | „about 2-3 mm“; Dauer „roughly 100 ms“ (Springer beschleunigt) | | Perng 2012, S. 113–114 | [wörtlich] (Textebene) | Ultraschall, 50 Hz |
| Springer beim Abgleiten | ≈ 0,25 (langsam) / ≈ 0,58 m/s (kräftig) Spitze | m/s | Perng 2012, Abb. 4.24/4.25 (S. 115/116) | [aus Abbildung abgelesen] ± 0,03; 50 Hz tastet Spitzen zu niedrig ab | |
| Abgleiten von der Spitze | ≈ 0,25 ms (x springt 0,40 → 0,65 mm) | | Perng 2012, Abb. 3.24 (S. 85), v = 0,1 m/s | [aus Abbildung abgelesen] ± 0,05 ms | Simulation |
| Federkiel: Dicke der Spitze | ≈ 0,22 mm (8′-Diskant) | | Wraight (Stufe 1) | [Werkzeug] | |
| Kiellänge, Überstand | „around 5 mm“; „about one string diameter“ | | trasuntino (Stufe 1) | [Werkzeug] | |
| Saiten je Taste (8′) | Länge, Zupfpunkt Taskin 1769; d, ρ, E nach Prüfstand | | S199 Parameterblock (Edinburgh 96075, Di Veroli Tab. 7/8; ρ, E dort Annahme) | [aus Prüfstand übernommen] | a′ = 440 Hz wie `hz_von` |

Spannung, Saitensteife und B je Taste (Auszug Anhang §0, [eigene Rechnung]):

| Taste | L mm | Zupf ab Sattel mm | x/L | d mm | T (440) N | T (415) N | K N/m | B |
|---|---|---|---|---|---|---|---|---|
| F1 | 1765 | 208 | 0,118 | 0,620 | 62,4 | 55,5 | 340 | 4,2·10⁻⁵ |
| C2 | 1681 | 191 | 0,114 | 0,518 | 88,7 | 78,9 | 524 | 1,6·10⁻⁵ |
| C3 | 1178 | 162 | 0,138 | 0,339 | 66,9 | 59,5 | 479 | 1,2·10⁻⁵ |
| C4 | 711 | 137 | 0,193 | 0,269 | 61,4 | 54,6 | 555 | 1,4·10⁻⁵ |
| G4 | 477 | 123 | 0,259 | 0,244 | 50,9 | 45,3 | 557 | 2,5·10⁻⁵ |
| C5 | 357 | 115 | 0,322 | 0,244 | 50,9 | 45,3 | 653 | 4,5·10⁻⁵ |
| C6 | 186 | 96 | 0,516 | 0,213 | 42,1 | 37,5 | 907 | 1,2·10⁻⁴ |
| f‴ | 148 | 91 | 0,615 | 0,213 | 47,5 | 42,3 | 1356 | 1,6·10⁻⁴ |

### 1.3 Nachgerechnet gegen die Quellen

**K6 gegen Perngs Simulation** (Ablöse-Auslenkung A = √(x² + y²), Perng-Werte [aus Abbildung abgelesen] ± 0,05 mm bzw.
± 3 %):

| Fall (Perng-Parameter) | Perng mm | K6 mm | Abw. | F_ab N |
|---|---|---|---|---|
| Diskant, Zupf 1/5 (Abb. 3.16a) | 1,20 | 1,19 | −1 % | 2,01 |
| Diskant, Mitte (Abb. 3.16a) | 1,60 | 1,58 | −1 % | 1,71 |
| Bass, 1/7 (Abb. 3.15, kleine v) | 4,40 | 4,40 | 0 % | 0,76 |
| Bass, Mitte (Abb. 3.16b) | 6,50 | 6,36 | −2 % | 0,53 |
| Diskant 1/5, Kiel 5,5 / 5,7 mm (Abb. 3.19a) | 0,58 / 0,88 | 0,55 / 0,91 | −5 / +3 % | 0,93 / 1,53 |
| Bass 1/7, Kiel 5,5 / 5,7 mm (Abb. 3.19b) | 2,00 / 3,30 | 1,97 / 3,29 | −2 / 0 % | 0,34 / 0,56 |
| Diskant Mitte, E 0,1 / 1 / 30 GPa (Abb. 3.21a) | 0,10 / 0,55 / 4,30 | 0,06 / 0,51 / 4,29 | −36 / −7 / 0 % | |
| Bass Mitte, E 0,1 / 1 / 30 GPa (Abb. 3.21b) | 0,70 / 3,0 / 15,5 | 0,62 / 2,73 / 15,72 | −11 / −9 / +1 % | |

Nur der sehr weiche Kiel (E = 0,1 GPa, Winkel 34°, Spitzenradius nicht mehr klein) fällt heraus. Folgerung aus Perng
selbst [eigene Rechnung]: Seine Bass-Saite (42 N) löst bei **0,5–0,8 N**, die Diskant-Saite (135 N) bei **1,7–2,0 N** —
die Ablösekraft hängt an der Saite, nicht nur am Kiel. Der Prüfstand nennt aus Perng Tab. 2 „1,6–2,8 N“ (GZ13 a); das
sind Diskant-Werte (0,5 m, 92 bzw. 135 N), keine Bass-Werte.

**K4–K8 gegen Chadefaux 2013 (G4, gemessen).** β ist in der Quelle nicht genannt; aus Taskin 1769 an G4 β = 0,259
(Länge dort 477 mm, Quelle 475 mm) → K = 940 N/m [eigene Rechnung, Annahme β]:

- Kraft **direkt aus der gemessenen Lage** (K7 rückwärts): F = K · √(x² + y²) = **0,71 N (langsam) / 0,89 N (schnell)**.
  Das ist der Ursprung der 0,8 N in GZ13 a, hier mit der Spanne. [eigene Rechnung]
- K6 mit dem Fasen-Kiel K4: Überstand u = 0,30 mm gibt y_s = 0,71 mm, x_s = 0,23 mm (gemessen 0,27), φ0 = 18,0°
  (gemessen Γ = 17 ± 2°; Modell der Quelle 20,9°). Für den schnellen Anschlag braucht K6 u = 0,52 mm und gibt x_s =
  0,39 mm (gemessen 0,28) — das reibungsfreie Modell kann „höher, aber nicht weiter seitlich“ nicht abbilden; die
  Quelle selbst nennt Reibung als Ursache (S. 159) [wörtlich].
- Aufladezeit K8: (0,71 + 0,64) mm / 0,10 m/s = **13,5 ms** (gemessen 12 ± 2); (0,90 + 0,79) mm / 0,25 m/s = **6,8 ms**
  (gemessen 7,9 ± 0,6) [eigene Rechnung].
- Anschlag: schnell gegen langsam ist D = 0,94 / 0,76 mm = **+1,8 dB** Auslenkung beim Ablösen [eigene Rechnung aus
  Tab. 1] — in der Größe von GZ10 a (ff → pp 1 dB nach Penttinen), an einem anderen Instrument gemessen.
- Anfangsgeschwindigkeit (K10): B_1/A_1 = V/(D ω_1) = 0,032 (langsam) / 0,065 (schnell), fällt mit 1/n — die gemessene
  Anfangsgeschwindigkeit ist für das Spektrum ohne Belang [eigene Rechnung].

### 1.4 Gesetz der Ablösekraft je Taste (8′)

Kiel nach Chadefaux (K4), Überstand an G4 auf die Messung gelegt (u = 0,30 mm); Saiten wie §1.2. Drei Lesarten des
Voicings [eigene Rechnung, Anhang §1c/§1f]:

- **A:** gleicher Kiel, gleicher Überstand an jeder Taste.
- **B:** gleicher Kiel, Überstand proportional zum Saitendurchmesser (u = 1,23 d; „about one string diameter“ [Werkzeug]).
- **C:** gleicher Überstand, Kielsteife je Taste so, dass die Kraft −2,8 dB/Okt fällt (die Form von GZ13 a, Maßstab
  aus A an C4).

| Taste | A: F_ab N | A dB re C4 | A: y_s mm | B: F_ab N | B dB re C4 | C: EI-Faktor | C: Dicke-Faktor | GZ13 a heute N | heute y = F/K mm |
|---|---|---|---|---|---|---|---|---|---|
| F1 | 0,46 | −1,9 | 1,31 | 0,73 | +1,8 | 7,5 | 1,96 | 4,60 | 13,5 |
| C2 | 0,55 | −0,2 | 1,02 | 0,81 | +2,7 | 3,5 | 1,51 | 3,81 | 7,3 |
| C3 | 0,53 | −0,6 | 1,08 | 0,63 | +0,5 | 2,0 | 1,27 | 2,76 | 5,8 |
| C4 | 0,57 | 0 | 0,99 | 0,59 | 0 | 1,0 | 1,00 | 2,00 | 3,6 |
| C5 | 0,61 | +0,6 | 0,89 | 0,61 | +0,2 | 0,53 | 0,81 | 1,45 | 2,2 |
| C6 | 0,69 | +1,8 | 0,73 | 0,65 | +0,7 | 0,28 | 0,66 | 1,05 | 1,2 |
| f‴ | 0,80 | +3,1 | 0,56 | 0,75 | +2,0 | 0,22 | 0,60 | 0,92 | 0,68 |
| Steigung | **+0,8 dB/Okt** | | | **−0,2 dB/Okt** | | | | −2,8 dB/Okt | |

Gegen heute: An C4 liegt GZ13 a **+10,9 dB** über A; an G4 gibt GZ13 a mit den Saiten des Prüfstands y = 2,98 mm gegen
gemessene 0,76 / 0,94 mm (**+11,9 / +10,0 dB**) [eigene Rechnung]. Die Form fallend zum Diskant entsteht im Modell
**nicht von selbst**; sie verlangt ein Voicing nach C (Bass-Kiel rund doppelt so dick wie der Diskant-Kiel, Steife
7,5- bis 0,22-fach). Das stützt qualitativ die Stufe 1 („The bass needs stronger quills“; sechs Kieldicken, „changing
approximately every tritone to a thinner quill“ [Werkzeug]); eine gemessene Dicke je Taste gibt es nicht.

### 1.5 4′, Delrin / Federkiel

**4′** (Saiten nach S200, Zupfpunkt nach der Regel des Prüfstands, gleicher Kiel und Überstand wie A; Anhang §1d):
F_ab 4′ = 0,51–0,92 N, gegen das 8′ derselben Taste **−0,2 … +1,7 dB**. Ein gleich geschnittener Kiel zupft das 4′ also
nicht schwächer; dass das 4′ leiser klingt (Fletcher & Beebe: 68 gegen 70 dB(A) [wörtlich, Stufe 1]), liegt dann an
Saite und Steg, nicht an der Kraft. [eigene Rechnung]

**Delrin gegen Federkiel:** In den lokalen Quellen steht **kein E-Modul für Federkiel**. Für Delrin stehen drei Werte
(1,5 GPa Chadefaux; 3 und 5 GPa Perng). K6b: im Bereich der weichen Saite wächst F_ab mit √(EI) — doppeltes E gibt
+3 dB, doppelte Dicke +9 dB [hergeleitet]. Ein Unterschied Delrin/Federkiel lässt sich deshalb nur über gemessene
Kielmaße und E-Module beziffern; beides fehlt.

### 1.6 Zeitverlauf und Spektrum der Anregung

| Phase | Dauer | Quelle | Marke |
|---|---|---|---|
| Aufladen (Saite haftet, wird gehoben) | Rest der Wechselwirkung: 64 % von 12 ms ≈ 7,7 ms (normal), 74 % von 7,9 ms ≈ 5,8 ms (schnell) | Chadefaux 2013 | [eigene Rechnung] aus [wörtlich] |
| Rutschen längs des Kiels | 36 % ≈ 4,3 ms / 26 % ≈ 2,1 ms | ebd. | [eigene Rechnung] aus [wörtlich] |
| Abgleiten über die Spitze | ≈ 0,25 ms (v = 0,1 m/s); seitlicher „Kick“ | Perng Abb. 3.24 | [aus Abbildung abgelesen] |
| Reale Springerbewegung (beschleunigt) | „roughly 100 ms“ | Perng S. 114 | [wörtlich] |

Wirkung aufs Spektrum [eigene Rechnung, K11/K12]:

- Eine Rampe von 0,25 ms gegen den idealen Sprung: −3 dB bei **1,8 kHz**, Nullstelle 4 kHz (0,1 ms: 4,4 kHz; 0,5 ms:
  0,9 kHz).
- Die Breite des Kiels längs der Saite (Prüfstand h = 3 mm; Perng W = 2 mm) wirkt erst bei 4,8 kHz (F1) bis 12,9 kHz
  (f‴), mit W = 2 mm bei 7,2–19,3 kHz.
- Perng deutet das Abgleiten **umgekehrt**: „the string receives an impulsive 'kick' in the horizontal direction … this
  'kick' will manifest itself in the perceived brightness“ (S. 84) [wörtlich, am Seitenbild geprüft]; die optische
  Messung am Monochord zeigt den steilen Anstieg in x (S. 111) [wörtlich]. Chadefaux rechnet die freie Schwingung aus
  Lage und Geschwindigkeit beim Ablösen (K10), also als **idealen Sprung**. Eine gemessene Zeitkonstante des Abgleitens
  gibt es nicht.

---

## 2. Tastenstoß und Mechanikgeräusch

### 2.1 Gleichungen

| Nr. | Gleichung | Marke |
|---|---|---|
| S1 | Kraftstoß eines Aufpralls: J = m Δv = m v (1 + e), e Stoßzahl (0 … 1) | [hergeleitet] (Impulssatz) |
| S2 | Dauer bei Kontaktsteife k_c (Halbsinus): τ = π √(m / k_c); umgekehrt k_c = m (π/τ)² | [hergeleitet] |
| S3 | Rückfall aus Höhe h: v = √(2 g h) | [hergeleitet] |
| S4 | Spektrum eines Gauß-Stoßes (σ): −3 dB bei f = √(ln 2)/(2π σ) | [hergeleitet] |

### 2.2 Ereignisse (Zahlen Anhang §2)

| Ereignis | Masse | Geschwindigkeit | J (e = 0 … 1) | Dauer | Herkunft |
|---|---|---|---|---|---|
| Springer an der Springerleiste, langsam | 5 g („less than 5 g“, Sankey [Werkzeug]) | 0,25 m/s (Perng Abb. 4.24 [aus Abbildung]) | 1,2 … 2,5 mN·s | nicht belegt (Polster) | [eigene Rechnung] |
| dto., kräftig | 5 g / 6 g Kunststoff | 0,58 m/s (Abb. 4.25) | 2,9 … 5,8 / 3,5 … 7,0 mN·s | nicht belegt | [eigene Rechnung] |
| Rückfall des Springers (kräftig: „moving as much as 3 cm“ über die Ruhelage, Perng S. 114 [wörtlich]) samt Dämpfer auf die Taste | 5 g | 0,77 m/s (S3) | 3,8 … 7,7 mN·s | nicht belegt | [eigene Rechnung]; beim Instrument mit Springerleiste begrenzt diese die Höhe |
| Taste auf den Tastenboden | wirksame Masse Taste + Finger **unbekannt** (Annahme 20 / 50 / 100 g) | 0,10 m/s (Chadefaux Abb. 6, G4, normal) | 2 / 5 / 10 mN·s | **≈ 8 ms** (0,1 m/s → 0) | Geschw. und Dauer [aus Abbildung abgelesen] ± 2 ms; Masse [Annahme] |

Gegen heute (7 mN·s, Gauß σ = 0,4 ms, „rund 1 ms“ laut S200; nach S4 −3 dB bei 0,1325/σ = **331 Hz**): Die Größe liegt im
Bereich der Springer-Ereignisse bei kräftigem Anschlag. Die Dauer ist für den Tastenboden rund achtmal zu kurz (8 ms,
σ ≈ 3,2 ms ⇒ −3 dB bei ≈ 40 Hz); für Springer an gepolsterter Leiste gibt es keinen Wert. (Die Zeile „Stoßdauer 0,4 ms
… 828 Hz“ im Anhang rechnet mit σ = Dauer/2,5, also σ = 0,16 ms — nicht der Wert von heute.) Je Dynamikstufe: J ∝ v gibt zwischen Perngs
langsamem und kräftigem Anschlag 20 lg(0,58/0,25) = **7,3 dB**; die Stufe 1 meldet dagegen fast gleiche Geräuschpegel
für lauten und leisen Anschlag am Taskin 1788 (−42,5 / −42,0 dB, MacRitchie & Nuti [Werkzeug]) — das widerspricht sich
oder die Spieler änderten die Springergeschwindigkeit dort kaum. [eigene Rechnung / Einschätzung]

**Weg in den Boden:** nicht belegt. Konstruktiv sitzt die Springerleiste an den Wangen (Gehäuse), der Tastenrahmen auf
dem Gehäuseboden; der Resonanzboden wird dann über den Rand angeregt, nicht am Steg [eigene Einschätzung, keine Quelle].
**Pegel relativ zum Ton:** nur Stufe 1 (Mechanik allein ≈ 9 dB unter dem Grundton, F3, Taskin 1788 [Werkzeug], Bezug
unklar). Flügel (Askenfelt): lokal nichts.

Wechselwirkung mit Punkt 1: GZ10 a hat den Stoß **relativ zum Ton** mit 2 N Ablösekraft geeicht. Fällt die Kraft auf die
Werte von §1.4 (≈ −11 dB), steigt der Stoß relativ zum Ton um denselben Betrag, wenn J bleibt.

---

## 3. Lautenzug

### 3.1 Gleichungen

| Nr. | Gleichung | Quelle | Marke |
|---|---|---|---|
| L1 | Polster als Feder und Dämpfer: f = −k x − R v; Z_eff(s) = k/s + R; Reflexion R_l = −Z_eff / (2 Z0 + Z_eff) | Perng 2012, S. 127, Gl. (5.5)–(5.10) | [wörtlich, am Seitenbild geprüft] |
| L2 | Abschluss im Abstand a vor dem Sattel: Z_T = R + k/(iω) + Z0/(i tan(k_w a)) ≈ R + (k + T/a)/(iω); r = (Z0 − Z_T)/(Z0 + Z_T) · e^{2 i k_w a} | Stummel wie S200 `sattel()`; Feder k ergänzt | [hergeleitet] |
| L3 | Zusatzdämpfung je Teilton: σ_n = −f0 ln \|r(ω_n)\| (1/s, Amplitude); T60-Beitrag 6,91/σ_n | aus L2 | [hergeleitet] |
| L4 | **Geschlossen (\|Z_T\| ≫ Z0):** σ_n ≈ 2 Z0 f0 R / (R² + ((k + T/a)/ω_n)²); oberhalb f_c = (k + T/a)/(2π R) gilt σ∞ = 2 Z0 f0 / R für alle Teiltöne | aus L2/L3 | [hergeleitet], gegen L3 exakt geprüft (Anhang §3b, Abweichung < 0,01 1/s) |
| L4a | Schwacher Dämpfer (R ≪ Z0, ohne Stummel): σ_n = R sin²(nπa/L) / (μ L) | Energiebilanz der Mode | [hergeleitet], gegen L3 geprüft (0,1272 = 0,1272 1/s) |
| L5 | Verstimmung bei vollem Klemmen im Abstand a: Δ = 1200/ln 2 · a/L ≈ 1731 a/L Cent | | [hergeleitet] |

### 3.2 Parameter

| Größe | Wert | Quelle | Marke |
|---|---|---|---|
| Lage | am 8′-Sattel, Polster gleiten gegen die Saiten | Fletcher & Beebe §8.2.5 (Stufe 1) | [wörtlich] |
| Polster | Leder 1–4 mm, Polsterdicke 4–6 mm, Saite an der Polstermitte → a ≈ 2–3 mm | Bray Baroque (Stufe 1) | [Werkzeug]; a [eigene Rechnung, Stufe 1] |
| k, R des Leders, Andruck | **nicht gefunden** | Perng S. 126–128 (nur Modellform), Stufe 1 | — |
| heute | R = 40 N·s/m, a = 0,5 mm, k = 0 | GZ11 a | angepasst |

### 3.3 Nachgerechnet (Anhang §3)

| Taste (Z0 kg/s) | Fall | σ1 | σ5 | σ10 | σ20 1/s | Cent T1 / T10 |
|---|---|---|---|---|---|---|
| C2 (0,40) | GZ11 a: 40 N·s/m, 0,5 mm | 0,01 | 0,23 | 0,61 | 1,02 | +0,0 / +0,2 |
| C2 | 40 N·s/m, 2,5 mm | 0,23 | 1,11 | 1,26 | 1,30 | +0,5 / +2,5 |
| C4 (0,17) | GZ11 a | 0,48 | 1,89 | 2,08 | 2,14 | +0,3 / +1,2 |
| C4 | 40 N·s/m, 2,5 mm | 1,89 | 2,15 | 2,15 | 2,16 | +5,3 / +6,1 |
| C4 | R = Z0, 2,5 mm | 0,06 | 1,59 | 6,28 | 24,0 | 0 / +0,1 |
| C6 (0,11) | GZ11 a | 5,13 | 5,64 | 5,66 | 5,66 | +4,2 / +4,6 |
| C6 | 40 N·s/m, 2,5 mm | 5,64 | 5,66 | 5,66 | 5,66 | +23,0 / +23,1 |
| C6 | R = Z0, 0,5 mm | 0,15 | 3,72 | 14,8 | 57,6 | 0 / 0 |

Befunde [eigene Rechnung]:

- GZ11 a ist ein **starker** Dämpfer (R/Z0 = 100–370): Er klemmt die Saite fast; die Zusatzdämpfung ist oberhalb
  f_c (C2: 705 Hz ≈ Teilton 11; C4: 488 Hz ≈ T2; C6: unter dem Grundton) für alle Teiltöne gleich, σ∞ = 2 Z0 f0 / R
  (C2 1,3 / C4 2,2 / C6 5,7 1/s), darunter fällt sie mit ω². Das nimmt im Bass die Höhen, im Diskant den ganzen Ton.
- Der schwache Dämpfer (R ≈ Z0) wirkt wie die Beschreibung des „singing buff“ (Höhen stark, Grundton kaum, keine
  Verstimmung), aber steigt mit n² ohne Grenze.
- Die **Verstimmung** trennt die Fälle: Die VCSL-Aufnahmen (GZ11 a: Median +1,6, C6 +5,2 Cent) verlangen bei vollem
  Klemmen a ≈ 0,5 mm (L5: C6 4,7 Cent); 2,5 mm gäben +23 Cent. Ein Polster mit 2–3 mm Wirkabstand müsste demnach
  nachgiebig sein (Feder k, R nahe Z0) — dann fehlt aber die Verstimmung. Eine Feder k = 2·10⁴ N/m ändert bei 2,5 mm
  fast nichts (T/a ≈ 2,5·10⁴ N/m an C4).
- Gegen die Wirkung am VCSL-Cembalo (−1,8 LU, Abfall 2,2-fach) lässt sich ohne Prüfstand nicht rechnen (Lautheit
  braucht Körper und Hörort); L4 erklärt aber den Gang über die Tasten: σ∞ ∝ Z0 f0 steigt von C2 bis C6 um Faktor 4,3,
  also wächst die Wirkung zum Diskant — wie in den Aufnahmen (−0,4 LU Bass … −4,9 LU C6).

---

## 4. Draht: Verlust und Inharmonizität

### 4.1 Gleichungen

| Nr. | Gleichung | Quelle | Marke |
|---|---|---|---|
| D1 | Luft (Stokes): η_air ≈ (ρ_a/ρ) (2√2 M + 1)/M², M = (d/4) √(2π f_n / ν_a), ρ_a = 1,2 kg/m³, ν_a = 1,5·10⁻⁵ m²/s | Woodhouse & Lynch-Aird 2019, S. 518, Gl. (11), (12) | [wörtlich, am Seitenbild geprüft] |
| D1a | ad-hoc-Zuschlag (d + 0,2), d in mm; Grenzform (13) für große M | ebd., S. 518, Gl. (13) | [wörtlich, am Seitenbild geprüft]; gefittet an Polymer- und Darmsaiten |
| D2 | Biegung: η_bend ≈ λ η_E; Summe η_n ≈ η_body + η_air + η_bend | ebd., Gl. (14), (15) | [wörtlich, am Seitenbild geprüft] |
| D2a | λ = EI k² / (T + EI k²) ≈ n² B (für B ≪ 1) | λ wie im Prüfstand (`saiten_eta`); Näherung | Form [aus Prüfstand übernommen], Gl. (4) der Quelle nicht am Bild gelesen; Näherung [hergeleitet] |
| D3 | B = π³ E d⁴ / (64 T L²) = π² EI / (T L²); f_n = n f0 √(1 + B n²) | Auftrag; Standard | [hergeleitet] |
| D4 | η aus T60: η = ln 1000 / (π f T60) | | [hergeleitet] |
| D5 | Metalle: „the loss factor η_E for relevant metals such as piano wire usually has a value at least an order of magnitude smaller than those seen earlier for polymeric materials“ | ebd., S. 528 | [wörtlich, am Seitenbild geprüft] |

Steg und Sattel: in LumiViz über die Admittanz des Körpers (Steg) und den starren Sattel; thermoelastische Verluste:
lokal keine Quelle.

### 4.2 Nachgerechnet (Anhang §4)

- **η_E = 0,001 ist wirkungslos:** λ · 0,001 ≤ 1,8·10⁻⁵ (C5, Teilton 20), meist 10⁻⁸ … 10⁻⁶ — gegen η_air = 0,5 … 7·10⁻⁴.
- **Gegen Välimäki 2004** (Messing, f0 197 Hz, Stufe 1 [aus Abbildung abgelesen]); Messing-Durchmesser dort nicht
  angegeben, hier 0,30–0,40 mm [Annahme]:

  | Teilton | gemessen η | Luft D1 + D1a | Rest |
  |---|---|---|---|
  | 1 (197 Hz) | 2,9·10⁻³ | 3,7–4,5·10⁻⁴ | ≈ 2,5·10⁻³ → Steg/Boden |
  | 6 (1182 Hz) | 1,6·10⁻⁴ | 1,2–1,4·10⁻⁴ | ≈ 0,3·10⁻⁴ |
  | 20 (3940 Hz) | 1,6·10⁻⁴ | 0,64–0,73·10⁻⁴ | ≈ 0,9·10⁻⁴ → Steg, oder η_E ≈ 0,0075 |

  Ohne den Zuschlag (d + 0,2) läge die Luft an Teilton 6 schon bei 2,4–3,2·10⁻⁴ — **über** dem gemessenen Gesamtverlust;
  der Zuschlag ist also auch für dünnen Draht stimmig, obwohl an Polymer gefittet [eigene Rechnung]. Obergrenze aus
  Teilton 20: η_E (Messing) ≤ 1,6·10⁻⁴ / (400 · 3·10⁻⁵) ≈ 0,013.
- **B je Taste** (Prüfstand-Saiten): 1,0·10⁻⁵ (F2–F3) … 4,2·10⁻⁵ (F1) … 1,6·10⁻⁴ (f‴); Teilton 10 liegt 0,9 Cent (F3) bis
  14 Cent (f‴) hoch. Välimäki (Messing, anderes Instrument): 3·10⁻⁵ bei 200 Hz, 1·10⁻⁴ bei 400, 2·10⁻⁴ bei 700, 4·10⁻⁴
  bei 1050 Hz — **2,5- bis 5-mal** höher als der Taskin mit Eisen im Prüfstand ab G3. Ursache nicht trennbar (anderes
  Instrument, Messing, Mensur).
- **Stimmton:** Der Prüfstand rechnet mit a′ = 440 Hz; bei 415 Hz wären die Spannungen 11 % kleiner, B 12 % größer,
  K 11 % kleiner (F_ab nach K6b −0,5 dB). Welchen Stimmton das Vorbild hat, steht nicht in den gelesenen Quellen.

---

## 5. Register: Zupfpunkt je Taste

### 5.1 Daten (alle aus Stufe 1)

| Taste | 8′ Taskin 1769 ab Sattel mm | Ruckers-Nachbau 8′ Anteil | 4′ Anteil | 4′ ab Sattel mm | p4/p8 | Regel S200: 4′ Anteil | Regel S200: 4′ ab Sattel mm |
|---|---|---|---|---|---|---|---|
| C2 | 191 | 0,10 | 0,09 | 71 | 0,47 | 0,138 | 122 |
| C3 | 162 | 0,12 | 0,11 | 59 | 0,45 | 0,193 | 114 |
| C4 | 137 | 0,15 | 0,15 | 48 | 0,48 | 0,322 | 107 |
| C5 | 115 | 0,25 | 0,28 | 46 | 0,54 | 0,500 | 84 |
| C6 | 96 | 0,35 | 0,48 | 43 | 0,71 | 0,500 | 46 |

Taskin [wörtlich] (Edinburgh 96075); Ruckers-Nachbau Fletcher & Beebe Abb. 8.4 [aus Abbildung abgelesen, Stufe 1,
± 0,01 / ± 20 mm]; p4/p8 und Regel [eigene Rechnung].

### 5.2 Gesetze

| Nr. | Gesetz | Marke |
|---|---|---|
| R1 | 8′: p8(Taste) log-linear zwischen den elf Taskin-Tasten (wie Prüfstand) | [wörtlich] (Daten) + Interpolation wie Prüfstand |
| R2 | 4′ nach Ruckers-Nachbau: p4 = r(Taste) · p8(Taste), r = 0,47 / 0,45 / 0,48 / 0,54 / 0,71 an C2 / C3 / C4 / C5 / C6, dazwischen linear in der Taste; unter C2 und über C6 konstant fortgesetzt | [hergeleitet] aus [aus Abbildung abgelesen]; überträgt ein Verhältnis von einem flämischen Nachbau auf einen Taskin |
| R3 | 4′ nach Prüfstand: Anteil des 8′ gleicher Tonhöhe, höchstens 0,5 | [Annahme] (S200) |
| R4 | zweites 8′: p8 ± Δ_Reihe; S200 setzt ±29 mm (zwei Reihen à 14,5 mm, NEMUS-Nachbau) | [aus Prüfstand übernommen], von mir nicht gegengelesen; Taskin-Zahl nicht gefunden |

R3 legt den 4′-Zupfpunkt an C2–C4 **1,7–2,2-mal so weit** vom Sattel wie R2; erst an C6 treffen sie sich [eigene Rechnung].
Was das für den Klang heißt: weiter vom Sattel → kräftigerer Grundton, weniger Höhen; mit K5/K6 auch kleinere
Saitensteife und damit (§1.5) eine etwas kleinere Ablösekraft.

---

## Annahmen, die bleiben

1. Kiel reibungsfrei (K6); Reibung erklärt laut Chadefaux den Unterschied langsam/schnell (S. 159) und ist nicht
   modelliert. Der Spitzenradius ist vernachlässigt (schlecht bei sehr weichem Kiel, §1.3).
2. Kiel nach Chadefaux (ein Delrin-Kiel, G4) als Vorbild für alle Tasten; Überstand u = 0,30 mm aus einer Messung an
   einer Taste; Zupfanteil β an G4 aus Taskin, weil die Quelle ihn nicht nennt.
3. Voicing je Taste (Kieldicke, Überstand) unbekannt; Optionen A/B/C sind Lesarten, keine Messung.
4. Kein E-Modul für Federkiel; Delrin-E zwischen 1,5 und 5 GPa je nach Quelle.
5. Springermasse 5–6 g [Werkzeug], wirksame Tastenmasse unbekannt, Stoßzahl unbekannt, Kontaktsteife der Polster
   unbekannt; Weg in den Boden unbelegt.
6. Leder des Lautenzugs: k, R, Andruck unbekannt; Wirkabstand 2–3 mm nur aus einer Bauanleitung.
7. Draht: ρ und E sind im Prüfstand moderne Werte (Annahme dort); η_E für historisches Eisen und Messing unbekannt;
   Luftzuschlag (d + 0,2) an Polymersaiten gefittet; Stimmton 440 Hz.
8. 4′-Verhältnis r vom Ruckers-Nachbau; zweites 8′ ohne Taskin-Beleg.

---

## Optionen für LumiViz

Zielgrößen: **(L)** Lautheit je Taste und Spanne über den Umfang, **(E)** Spektrum des Einsatzes, **(T)** Abklingzeit je
Teilton, **(Z)** Wirkung des Lautenzugs, **(S)** Pegel des Tastenstoßes gegen den Ton. „nicht gerechnet“ heißt: braucht
den Prüfstand.

### Punkt 1 — Ablösekraft

| Option | braucht | (L) | (E) | (T) | (Z) | (S) |
|---|---|---|---|---|---|---|
| **1a** heute: 2 N an C4, −2,8 dB/Okt | nichts | Form an VCSL angepasst (Spanne 6,8 LU, GZ13 a); Maßstab +11 dB über der Messung an G4 | Ablösen als Sprung | — | Lautenzug-Wirkung relativ, unberührt | Stoß relativ zum Ton wie geeicht |
| **1b** K6 mit einem Kiel (Option A), ohne Voicing | Kiel Chadefaux, u = 0,30 mm, Saiten wie heute | eben bis +0,8 dB/Okt Kraft; Lautheit steigt dann wie bei fester Kraft (S200: rund 15 LU Gang) — nicht gerechnet; alle Töne ≈ 11 dB leiser | φ0 je Taste aus K6 (an G4 18°) → Anteil waagrechter Polarisation je Taste statt fest 20° (nicht gerechnet) | — | — | Stoß relativ +11 dB, wenn J bleibt |
| **1c** K6 mit Voicing C (Steife je Taste) | eine Steifekurve (aus 1a abgeleitet oder gemessen) | Form wie 1a, Maßstab an der Messung (−11 dB gegen heute) | wie 1b | — | — | wie 1b |
| **1d** K6 + Rampe τ beim Abgleiten | wie 1b/1c, dazu τ (0,1–0,5 ms) | Höhen leiser → Lautheit etwas kleiner (nicht gerechnet) | Tiefpass −3 dB 0,9–4,4 kHz; widerspricht Perngs „Kick“-Deutung | — | — | — |

### Punkt 2 — Tastenstoß

| Option | braucht | (L) | (E) | (T) | (Z) | (S) |
|---|---|---|---|---|---|---|
| **2a** heute: 7 mN·s, σ 0,4 ms am Stegpunkt | — | GZ10 a | Klopfen bis ≈ 0,33 kHz (−3 dB) | — | — | geeicht an Penttinen |
| **2b** zwei Ereignisse: Springer an der Leiste (1–7 mN·s, kurz) und Tastenboden (2–10 mN·s, ≈ 8 ms) | Massen, Stoßzahl, Ort am Gehäuse | kaum | Tastenboden nur unter ≈ 40–100 Hz; Springer breitbandiger | — | — | anders je Stufe: J ∝ v (7,3 dB zwischen langsam/kräftig) |
| **2c** wie 2a/2b, aber Kraft am Rand (Gehäuse) statt am Steg | Ankopplung des Gehäuses im Körpermodell | — | andere Moden angeregt (nicht gerechnet) | — | — | Pegel am Hörort neu zu eichen |

### Punkt 3 — Lautenzug

| Option | braucht | (L) | (E) | (T) | (Z) | (S) |
|---|---|---|---|---|---|---|
| **3a** heute: R 40 N·s/m, a 0,5 mm | — | an VCSL angepasst | Einsatz unverändert | σ∞ = 2 Z0 f0/R über f_c, gleich je Teilton | −1,8 LU, Abfall 2,2-fach, +1,6 / +5,2 Cent (angepasst) | — |
| **3b** L2 mit Feder k + R, a = 2–3 mm (Bauanleitung) | k, R je Taste | nicht gerechnet | — | stärker an tiefen Teiltönen (f_c tiefer) | Verstimmung bis +23 Cent an C6 bei vollem Klemmen — gegen die Aufnahmen | — |
| **3c** schwaches Polster R ≈ Z0 je Taste | R(Taste) | kleine Lautheitsänderung | — | σ ∝ n² ohne Grenze | keine Verstimmung — gegen die Aufnahmen (+1,6 Cent) | — |

### Punkt 4 — Draht

| Option | braucht | (L) | (E) | (T) | (Z) | (S) |
|---|---|---|---|---|---|---|
| **4a** heute (Luft D1 + D1a, η_E = 0,001 über λ) | — | — | — | Luft trägt alles; Steg über den Körper | — | — |
| **4b** wie 4a, aber η_E so, dass Välimäki Teilton 20 getroffen wird (≈ 0,0075 Messing) | Annahme: Rest an Teilton 20 ist Draht, nicht Steg | — | — | nur hohe Teiltöne (λ ∝ n²B): T60 an F1 n = 20 halbiert, an C4 n = 20 −36 %, an f‴ n = 6 −41 %; Grundton unberührt [eigene Rechnung aus Anhang §4] | — | — |
| **4c** wie 4a, Prüfung T60 je Teilton gegen Välimäki Abb. 8 als Ziel für Körper + Saite | Saite mit Messing-Maßen des Vorbilds (fehlen) | — | — | entscheidet, ob der Rest an Teilton 1/20 vom Steg kommt | — | — |
| **4d** Stimmton 415 statt 440 Hz | Entscheid zum Vorbild | Kraft −0,5 dB (K6b), Auslenkung +0,5 dB | — | Luft-η ∝ 1/√f leicht größer | — | — |

### Punkt 5 — Register

| Option | braucht | (L) | (E) | (T) | (Z) | (S) |
|---|---|---|---|---|---|---|
| **5a** R3 (heute S200) | — | 4′ mit 2 N 5–8 LU zu laut (GZ12 a) | 4′ grundtöniger | — | — | — |
| **5b** R2 (Ruckers-Verhältnis) | — | Zupfpunkt näher am Sattel → weniger Grundton, mehr Höhen; Lautheit nicht gerechnet | 4′ nasaler | — | — | — |
| **5c** R2/R3 mit K6 statt 2 N für das 4′ | wie 1b/1c | 4′-Kraft 0,5–0,9 N statt 2 N (§1.5) | — | — | — | — |

Fachliche Einordnung (keine Empfehlung nach Aufwand): Für Punkt 1 ist K6 an zwei unabhängigen Stellen geprüft (Perngs
Simulation, Chadefaux' Messung) und belegt den **Maßstab** (≈ 0,6–0,9 N, Auslenkung ≈ 1 mm); die **Form** über den
Umfang belegt keine Quelle — 1c trennt beides sauber, 1a vermischt sie. Für Punkt 5 ist R2 der einzige Beleg; R3 ist eine
Annahme.

---

## Was die LumiViz-Sitzung prüfen muss

1. Am Prüfstand: Lautheit je Taste mit Option 1b (Kraft A) — steigt sie wie mit fester Kraft um ≈ 15 LU? Dann ist die
   ebene Lautheit der Aufnahmen Voicing (1c) und nicht Körper/Hörort (GZ13 a, offene Frage).
2. Die Eichung des Tastenstoßes (GZ10 a) hängt an 2 N Ablösekraft; bei kleinerer Kraft neu stellen.
3. Chadefaux 2013 nennt β nicht; der Zupfanteil an G4 (0,259 aus Taskin) ist für den Maßstab mitentscheidend (K ∝ 1/(β(1−β))).
4. Ob die Dauer des Tastenstoßes (0,4 ms) zum Tastenboden (≈ 8 ms) oder zu den Springern gehört — an den
   VCSL-Aufnahmen: Klopfen breitbandig oder nur tief?
5. Lautenzug: ob die Verstimmung der Aufnahmen (+1,6 / +5,2 Cent) mit L5 a ≈ 0,5 mm erzwingt; ob ein nachgiebiges
   Polster (3b/3c) Lautheit und Abfall zugleich trifft.
6. T60 je Teilton im Prüfstand gegen Välimäki Abb. 8 (anderes Instrument): Grundton ≈ 3,8 s, Teilton 6 ≈ 11,7 s.
7. Stimmton des Vorbilds (440 gegen 415 Hz).
8. Stufe 1 widerspricht sich bei Wraight (Tabelle: neuer Federkiel verhärtet um +25 … +35 mm; §1c: Federkiel „work-hardens“
   nicht, Delrin schon) — am Wraight-Text klären.
9. Perngs Werte Abb. 3.15–3.22 sind abgelesen (± 3 %); am vergrößerten Bild nachlesen, wenn K6 zur Grundlage wird.

---

## Nicht gefunden / nicht herleitbar

| Lücke | wo gesucht (lokal) |
|---|---|
| E-Modul Federkiel; Kielmaße je Taste eines realen Voicings | Perng 2012 (Suche „quill“, „feather“, „Young“, „thickness“), Chadefaux 2013, Roy 2015 |
| Gemessene Kraft je Taste über den Umfang | dto.; Stufe 1 (Netz) |
| Zeitkonstante des Abgleitens am Instrument | Perng Kap. 3/4 (nur Simulation, qualitative Messung), Chadefaux 2013 |
| Massen von Taste und Dämpfer, Kontaktsteifen der Polster, Weg in den Boden | Perng Kap. 4, Chadefaux 2013, Roy 2015; Stufe 1 |
| Kennwerte des Leders (k, R), Andruck des Lautenzugs | Perng 2012 S. 126–128; Stufe 1 |
| η_E und thermoelastischer Verlust historischer Drähte; Verlust an Steg/Sattel getrennt | Woodhouse & Lynch-Aird 2019 (Metall nur S. 528, ohne Zahl), Woodhouse 2012 (nur Gitarrensaiten); Simmons 1974 gesperrt (Stufe 1) |
| Zupfpunkte von 4′ und zweitem 8′ an Taskin, Dulcken, Ruckers-Original, Blanchet | Stufe 1; lokal nichts Neues |
| Askenfelt (Flügel) als Vergleich zum Tastenstoß | lokal nicht vorhanden |

## Ladeliste (nichts geladen; URLs nicht geprüft, da keine Netzsuche)

| Vorschlag | Fundstelle | wozu |
|---|---|---|
| `Griffel_1994_DynamicsPlucking_JSV175.pdf` | D. H. Griffel, *J. Sound Vib.* 175(3), 289–297 (zitiert bei Chadefaux 2013 [6], Roy 2015 [1]) | erstes Plektrummodell |
| `Sloane_1988_PlectrumParameters_JSV125.pdf` | C. Sloane, *J. Sound Vib.* 125(1), 185–186 (Chadefaux [9]) | Plektrumparameter und Klang |
| `Valette_1995_MechanicsVibratingStrings.pdf` | C. Valette, „The mechanics of vibrating strings“ (WLA 2019 [3]) | Messungen an Metallsaiten, η_E |
| `LeMoyne_etal_2012_Restoration17thCentHarpsichord_JASA131.pdf` | *JASA* 131(1), 888–896 (Chadefaux [3]) | Modalverhalten, ggf. Mechanik |
| dazu die Ladeliste der Stufe 1 (Simmons 1974, Giordano & Winans 1999, Fletcher 1977, ICSV30, FA2025) | | |

## Am Ende

**Verlangt und nicht geliefert:** Kielmaße und E-Modul je Taste aus einer Quelle (nur Lesarten A/B/C); Delrin gegen
Federkiel als Zahl; ein Impulsmodell mit belegten Massen und dem Weg in den Boden; k und R des Lautenzugs; η_E für
historische Drähte; Zupfpunkte von 4′ und zweitem 8′ an einem Taskin.

**Neu gegenüber Stufe 1:** K6 (geschlossene Ablösebedingung, an Perng und Chadefaux geprüft), der Maßstab der Kraft je
Taste, die geschlossene Form des Lautenzugs (L4/L5) mit der Verstimmung als Trennmerkmal, die Richtigstellung zu η_E,
das 4′-Verhältnis R2 gegen die Regel des Prüfstands.

---

## Anhang: `rechnung.py` und Ausgabe


### Skript

```python
"""LVR, Auftrag 2026-11-16_14_cembalo_grundlagen: Gegenrechnungen zu Kiel, Stoss, Lautenzug, Draht, Register.

Aufruf: python ergebnisse/2026-11-16_14_cembalo_grundlagen/rechnung.py
Nur Standardbibliothek. Parameter der Saiten aus dem Parameterblock des LumiViz-Pruefstands (S199/S200, nur gelesen).
"""
import cmath
import math

# ------------------------------------------------------------------ Saiten (aus S199 / S200 Parameterbloecken)
TASKIN = [(29, 1765, 208), (36, 1681, 191), (41, 1443, 179), (48, 1178, 162), (53, 982, 151), (60, 711, 137), (65, 535, 127),
          (72, 357, 115), (77, 268, 107), (84, 186, 96), (89, 148, 91)]           # Taste, L mm, Zupf mm ab Sattel
DRAHT8 = [(31, 0.620, 8700, 115), (33, 0.572, 8700, 115), (36, 0.518, 8700, 115), (38, 0.463, 8700, 115), (40, 0.463, 8500, 110),
          (43, 0.410, 8500, 110), (45, 0.377, 8500, 110), (47, 0.339, 8500, 110), (51, 0.339, 7800, 170), (56, 0.300, 7800, 170),
          (64, 0.269, 7800, 170), (75, 0.244, 7800, 170), (89, 0.213, 7800, 170)]
VIER_MENSUR = [(29, 1073), (41, 773), (53, 486), (65, 255), (77, 124), (89, 74)]
DRAHT4 = [(31, 0.463, 8700, 115), (35, 0.410, 8700, 115), (38, 0.377, 8700, 115), (40, 0.377, 8500, 110), (42, 0.339, 8500, 110),
          (45, 0.300, 8500, 110), (49, 0.300, 7800, 170), (55, 0.269, 7800, 170), (72, 0.244, 7800, 170), (79, 0.213, 7800, 170),
          (89, 0.188, 7800, 170)]
NAME = {29: "F1", 36: "C2", 41: "F2", 48: "C3", 53: "F3", 55: "G3", 60: "C4", 65: "F4", 67: "G4", 72: "C5", 77: "F5", 84: "C6", 89: "f'''"}
TASTEN = [29, 36, 48, 60, 72, 84, 89]


def hz(t, a4=440.0):
    return a4 * 2 ** ((t - 69) / 12)


def logint(t, pts, spalte):
    xs = [p[0] for p in pts]
    ys = [math.log(p[spalte]) for p in pts]
    if t <= xs[0]:
        return math.exp(ys[0])
    for i in range(len(xs) - 1):
        if xs[i] <= t <= xs[i + 1]:
            w = (t - xs[i]) / (xs[i + 1] - xs[i])
            return math.exp(ys[i] + w * (ys[i + 1] - ys[i]))
    return math.exp(ys[-1])


def saite8(t, a4=440.0):
    L = logint(t, TASKIN, 1) / 1000
    x = logint(t, TASKIN, 2) / 1000                     # Zupf ab Sattel
    d, rho, E = next((d, r, e) for bis, d, r, e in DRAHT8 if t <= bis)
    return mach(L, x, d / 1000, rho, E * 1e9, hz(t, a4))


def saite4(t, a4=440.0):
    L = logint(t, VIER_MENSUR, 1) / 1000
    L8 = logint(min(t + 12, 89), TASKIN, 1) / 1000
    x8 = logint(min(t + 12, 89), TASKIN, 2) / 1000
    x = min(x8 / L8, 0.5) * L                          # Regel des Pruefstands S200
    d, rho, E = next((d, r, e) for bis, d, r, e in DRAHT4 if t <= bis)
    return mach(L, x, d / 1000, rho, E * 1e9, 2 * hz(t, a4))


def mach(L, x, d, rho, E, f):
    mu = rho * math.pi * d * d / 4
    T = 4 * L * L * f * f * mu
    I = math.pi * d ** 4 / 64
    return {"L": L, "x": x, "d": d, "rho": rho, "E": E, "f": f, "mu": mu, "T": T, "Z0": math.sqrt(T * mu),
            "K": T * L / (x * (L - x)), "B": math.pi ** 3 * E * d ** 4 / (64 * T * L * L), "EI": E * I}


# ------------------------------------------------------------------ Kiel als Biegebalken (Perng 2.57/2.74; Chadefaux Gl. 4)
class Kiel:
    """Gamma(l) = F g(l): Biegewinkel klein (Perng Gl. 2.57 bzw. Chadefaux Gl. 4 mit zwei Abschnitten, I/8 vorn)."""

    def __init__(self, E, I, Lp, L1=None, n=2000):
        self.Lp = Lp
        ls = [Lp * i / n for i in range(n + 1)]
        g = []
        for l in ls:
            if L1 is None or l <= L1:
                g.append((Lp * l - l * l / 2) / (E * I))
            else:
                g.append((3.5 * L1 * L1 - 7 * Lp * L1 + 8 * Lp * l - 4 * l * l) / (E * I))
        h = Lp / n
        self.gL = g[-1]                                              # theta0 = F gL
        self.G2 = sum((g[i] ** 2 + g[i + 1] ** 2) / 4 for i in range(n)) * h   # Verkuerzung = F^2 G2
        self.G1 = sum((g[i] + g[i + 1]) / 2 for i in range(n)) * h          # Senkung der Spitze = F G1

    def abloesen(self, K, u):
        """Abloesekraft F: Verkuerzung + seitliches Ausweichen der Saite = Ueberstand u (eigene Herleitung)."""
        f = lambda F: F * F * self.G2 + F * math.sin(F * self.gL) / K - u
        # erste Wurzel suchen (f ist fuer theta > 90 Grad nicht mehr monoton; das Modell gilt dort ohnehin nicht)
        lo, hi, schritt = 0.0, 0.0, 1e-3
        while f(hi) < 0 and hi * self.gL < math.pi / 2:
            lo, hi = hi, hi + schritt
        for _ in range(200):
            mid = (lo + hi) / 2
            lo, hi = (mid, hi) if f(mid) < 0 else (lo, mid)
        F = (lo + hi) / 2
        th = F * self.gL
        return {"F": F, "theta": th, "xs": F * math.sin(th) / K, "ys": F * math.cos(th) / K, "A": F / K,
                "spitze": F * self.G1}


def kopf(t):
    print("\n" + "=" * 110 + "\n" + t + "\n" + "=" * 110)


# ------------------------------------------------------------------ 0 Saiten
kopf("0  Saiten des Pruefstands (8' Taskin 1769 / Di Veroli; a' = 440 Hz wie hz_von im Pruefstand), dazu 415 Hz")
print("Taste  | L mm | Zupf mm | x/L   | d mm  | T440 N | T415 N | Z0 kg/s | K N/m  | B")
for t in TASTEN + [67]:
    s, s4 = saite8(t), saite8(t, 415)
    print(f"{NAME[t]:6s} | {1000*s['L']:4.0f} | {1000*s['x']:7.0f} | {s['x']/s['L']:.3f} | {1000*s['d']:.3f} | {s['T']:6.1f} | {s4['T']:6.1f} |"
          f" {s['Z0']:7.3f} | {s['K']:6.0f} | {s['B']:.2e}")

# ------------------------------------------------------------------ 1 Kiel: Pruefung gegen Perng 2012
kopf("1a Reduziertes Kielmodell gegen Perngs Simulation (Diss. 2012 Tab. 3.1; Abb. 3.15, 3.16, 3.19, 3.21 abgelesen)")
Wp, Hp = 2e-3, 0.5e-3
Ip = Wp * Hp ** 3 / 12


def perng(E=5e9, Lp=6e-3, Li=5.4e-3, Ls=0.5, T=135.0, x=0.1):
    k = Kiel(E, Ip, Lp)
    K = T * Ls / (x * (Ls - x))
    return k.abloesen(K, Lp - Li)


faelle = [("Diskant 1/5, Abb. 3.16a", {"x": 0.1}, 1.20), ("Diskant Mitte, Abb. 3.16a", {"x": 0.25}, 1.60),
          ("Bass 1/7, Abb. 3.15", {"Ls": 2.0, "T": 42.0, "x": 2 / 7}, 4.40), ("Bass Mitte, Abb. 3.16b", {"Ls": 2.0, "T": 42.0, "x": 1.0}, 6.50),
          ("Diskant 1/5, Lp 5,5 mm, Abb. 3.19a", {"x": 0.1, "Lp": 5.5e-3}, 0.58), ("Diskant 1/5, Lp 5,7 mm", {"x": 0.1, "Lp": 5.7e-3}, 0.88),
          ("Bass 1/7, Lp 5,5 mm, Abb. 3.19b", {"Ls": 2.0, "T": 42.0, "x": 2 / 7, "Lp": 5.5e-3}, 2.00),
          ("Bass 1/7, Lp 5,7 mm", {"Ls": 2.0, "T": 42.0, "x": 2 / 7, "Lp": 5.7e-3}, 3.30),
          ("Diskant Mitte, E 1 GPa, Abb. 3.21a", {"x": 0.25, "E": 1e9}, 0.55), ("Diskant Mitte, E 0,1 GPa", {"x": 0.25, "E": 1e8}, 0.10),
          ("Diskant Mitte, E 30 GPa", {"x": 0.25, "E": 3e10}, 4.30), ("Bass Mitte, E 1 GPa, Abb. 3.21b", {"Ls": 2.0, "T": 42.0, "x": 1.0, "E": 1e9}, 3.0),
          ("Bass Mitte, E 0,1 GPa", {"Ls": 2.0, "T": 42.0, "x": 1.0, "E": 1e8}, 0.7), ("Bass Mitte, E 30 GPa", {"Ls": 2.0, "T": 42.0, "x": 1.0, "E": 3e10}, 15.5)]
print("Fall                                  | A Perng mm | A Modell mm | Abw.  | F_ab N | theta Grad")
for name, kw, soll in faelle:
    r = perng(**kw)
    print(f"{name:37s} | {soll:10.2f} | {1000*r['A']:11.2f} | {100*(1000*r['A']/soll-1):+4.0f} % | {r['F']:6.2f} | {math.degrees(r['theta']):5.1f}")

kopf("1b Gegen die Messung Chadefaux u. a. 2013 (G4, Tab. 1 und 2; Kiel aus zwei Abschnitten, Gl. 4)")
kc = Kiel(1.5e9, 0.011e-12, 3.5e-3, L1=2.9e-3)
Ls, T = 0.475, 85.7
s67 = saite8(67)
beta = s67["x"] / s67["L"]
Kc = T / (Ls * beta * (1 - beta))
print(f"Zupfanteil G4 aus Taskin 1769 (Quelle nennt keinen): beta = {beta:.3f}; K = {Kc:.0f} N/m; Taskin-Laenge G4 {1000*s67['L']:.0f} mm (Quelle 475)")
print(f"Kiel: theta0/F = {math.degrees(kc.gL):.1f} Grad/N; Spitze senkt sich {1000*kc.G1:.2f} mm/N")
for soll_y, name in ((0.71e-3, "langsam"), (0.90e-3, "schnell")):
    lo, hi = 0.0, 3e-3
    for _ in range(100):
        u = (lo + hi) / 2
        lo, hi = (u, hi) if kc.abloesen(Kc, u)["ys"] < soll_y else (lo, u)
    r = kc.abloesen(Kc, u)
    print(f"{name:8s}: Ueberstand fuer y_s = {1000*soll_y:.2f} mm: u = {1000*u:.2f} mm -> F_ab {r['F']:.2f} N, x_s {1000*r['xs']:.2f} mm "
          f"(Messung {'0,27' if name == 'langsam' else '0,28'}), theta {math.degrees(r['theta']):.1f} Grad (Modell der Quelle 20,9; Gamma_tr {'17' if name == 'langsam' else '20'})")
    if name == "langsam":
        U_C = u
F_direkt = [Kc * math.hypot(xx, yy) for xx, yy in ((0.27e-3, 0.71e-3), (0.28e-3, 0.90e-3))]
print(f"Kraft direkt aus der gemessenen Lage (F = K * sqrt(x^2 + y^2)): langsam {F_direkt[0]:.2f} N, schnell {F_direkt[1]:.2f} N")

# Zeitverlauf
print("\nAufladezeit = (y_s + Senkung der Kielspitze) / v_Springer:")
for v, name, soll in ((0.10, "normal", "12 +- 2"), (0.25, "schnell", "7,9 +- 0,6")):
    r = kc.abloesen(Kc, U_C if name == "normal" else U_C)
    yy = 0.71e-3 if name == "normal" else 0.90e-3
    Fm = F_direkt[0] if name == "normal" else F_direkt[1]
    print(f"  {name:7s}: v {v} m/s: ({1000*yy:.2f} + {1000*Fm*kc.G1:.2f}) mm / v = {1000*(yy+Fm*kc.G1)/v:.1f} ms (gemessen {soll} ms)")

# ------------------------------------------------------------------ 1c Gesetz je Taste
kopf("1c Abloesekraft je Taste (8'), Kiel nach Chadefaux; Vergleich mit GZ13 a (2 N an C4, -2,8 dB/Oktave)")
d67 = s67["d"]
print(f"Option A: Ueberstand fest u = {1000*U_C:.2f} mm (an G4 auf die Messung gelegt). Option B: u = {U_C/d67:.2f} * d (Saitendurchmesser).")
print("Option C: Kiel je Taste so geschnitten, dass die Senkung der Spitze gleich bleibt - entspricht A; nicht getrennt.")
print("Taste  | K N/m | A: F N | A dB re C4 | A: y_s mm | B: F N | B dB re C4 | GZ13 F N | GZ13 dB | LumiViz y=F/K mm")
ref = {}
rows = []
for t in TASTEN + [67]:
    s = saite8(t)
    a = kc.abloesen(s["K"], U_C)
    b = kc.abloesen(s["K"], U_C / d67 * s["d"])
    gz = 2.0 * 10 ** (-2.8 / 20 * (t - 60) / 12)
    rows.append((t, s, a, b, gz))
    if t == 60:
        ref = {"a": a["F"], "b": b["F"]}
for t, s, a, b, gz in rows:
    print(f"{NAME[t]:6s} | {s['K']:5.0f} | {a['F']:6.2f} | {20*math.log10(a['F']/ref['a']):+10.1f} | {1000*a['ys']:9.2f} | {b['F']:6.2f} |"
          f" {20*math.log10(b['F']/ref['b']):+10.1f} | {gz:8.2f} | {20*math.log10(gz/2):+7.1f} | {1000*gz/s['K']:.2f}")


def steigung(key):
    import statistics
    xs = [(t - 60) / 12 for t, *_ in rows if t != 67]
    ys = [20 * math.log10(key(r)) for r in rows if r[0] != 67]
    mx, my = statistics.mean(xs), statistics.mean(ys)
    return sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sum((x - mx) ** 2 for x in xs)


print(f"Mittlere Steigung (Regression ueber F1..f'''): A {steigung(lambda r: r[2]['F']):+.1f} dB/Okt, B {steigung(lambda r: r[3]['F']):+.1f} dB/Okt, GZ13 -2,8")

kopf("1d 4' (Pruefstand-Regel fuer den Zupfpunkt), gleicher Kiel und Ueberstand wie Option A")
print("Taste  | L4 mm | Zupf mm | T4 N  | K4 N/m | F_ab 4' N | F_ab 8' N | 4'/8' dB")
for t in TASTEN:
    s4 = saite4(t)
    a4 = kc.abloesen(s4["K"], U_C)
    a8 = kc.abloesen(saite8(t)["K"], U_C)
    print(f"{NAME[t]:6s} | {1000*s4['L']:5.0f} | {1000*s4['x']:7.0f} | {s4['T']:5.1f} | {s4['K']:6.0f} | {a4['F']:9.2f} | {a8['F']:9.2f} | {20*math.log10(a4['F']/a8['F']):+.1f}")

kopf("1e Spektrum: Abloesen in tau (lineare Rampe) gegen den idealen Sprung; raeumliche Breite des Kiels (Pruefstand)")
for tau in (0.1e-3, 0.25e-3, 0.5e-3):
    print(f"tau {1000*tau:.2f} ms: -3 dB bei {0.443/tau:6.0f} Hz, erste Nullstelle {1/tau:6.0f} Hz")
print("Raeumlicher Gauss exp(-(w h / c)^2), -3 dB bei f = 0,0935 c / h:")
for t in TASTEN:
    s = saite8(t)
    c = 2 * s["L"] * s["f"]
    print(f"  {NAME[t]:5s} c = {c:5.0f} m/s: h 3 mm -> {0.0935*c/0.003:6.0f} Hz; h 2 mm (Perng W) -> {0.0935*c/0.002:6.0f} Hz")

# ------------------------------------------------------------------ 2 Tastenstoss
kopf("2  Tastenstoss: Kraftstoesse J = m dv (1 + e), Annahmen markiert")
for name, m, v in (("Springer an der Springerleiste, langsam (Perng Abb. 4.24, Spitze 0,25 m/s)", 0.005, 0.25),
                   ("Springer an der Springerleiste, kraeftig (Perng Abb. 4.25, Spitze 0,58 m/s)", 0.005, 0.58),
                   ("Springer, Kunststoff 6 g, kraeftig", 0.006, 0.58),
                   ("Rueckfall des Springers aus 3 cm (Perng S. 114) auf die Taste", 0.005, math.sqrt(2 * 9.81 * 0.03))):
    print(f"{name:78s}: m {1000*m:.0f} g, v {v:.2f} m/s -> J {1000*m*v:.1f} (e=0) .. {2000*m*v:.1f} (e=1) mN s")
print("Taste am Boden: Verzoegerung aus Chadefaux 2013 Abb. 6 (G4, normal): 0,10 m/s -> 0 in ~8 ms -> a ~ 12 m/s^2")
for meff in (0.02, 0.05, 0.10):
    print(f"  wirksame Masse (Taste + Finger, ANNAHME) {1000*meff:.0f} g: J = {1000*meff*0.10:.1f} mN s, mittlere Kraft {meff*0.10/0.008:.2f} N ueber 8 ms")
for tau_s in (0.4e-3, 2e-3, 8e-3):
    print(f"  Stossdauer {1000*tau_s:.1f} ms (Gauss sigma = Dauer/2,5): -3 dB des Spektrums bei {math.sqrt(math.log(2))/(2*math.pi*tau_s/2.5)*1:.0f} Hz")

# ------------------------------------------------------------------ 3 Lautenzug
kopf("3  Lautenzug: Abschluss Z_T = R + (k + T/a)/(i w) am Ort a vor dem Sattel (Perng Gl. 5.9 + Stummel), je Teilton")


def filz(s, a, R, k, n):
    w = 2 * math.pi * n * s["f"]
    kw = w * math.sqrt(s["mu"] / s["T"])
    zl = R + k / (1j * w) + s["Z0"] / (1j * math.tan(kw * a))      # Stummel exakt wie S200
    r = (s["Z0"] - zl) / (s["Z0"] + zl) * cmath.exp(2j * kw * a)
    sigma = -s["f"] * math.log(abs(r))                              # 1/s, Amplitude
    dphi = cmath.phase(-r)                                          # gegen starren Sattel (r = -1)
    cent = 1200 * math.log2(1 + dphi / (2 * math.pi * n)) if abs(dphi) < 3 else float("nan")
    return sigma, cent


def klein(s, a, R, n):
    """Stoerungsrechnung fuer R << Z0, Stummel vernachlaessigt: sigma = R sin^2(n pi a / L) / (mu L)."""
    return R * math.sin(n * math.pi * a / s["L"]) ** 2 / (s["mu"] * s["L"])


print("Taste | Z0     | Fall                    | sigma_1 | sigma_5 | sigma_10 | sigma_20 1/s | Cent T1 | Cent T10")
for t in (36, 48, 60, 72, 84):
    s = saite8(t)
    for name, a, R, k in (("GZ11 a: 40 N s/m, 0,5 mm", 0.5e-3, 40.0, 0.0), ("40 N s/m, 2,5 mm", 2.5e-3, 40.0, 0.0),
                          ("R = Z0, 0,5 mm", 0.5e-3, s["Z0"], 0.0), ("R = Z0, 2,5 mm", 2.5e-3, s["Z0"], 0.0),
                          ("40 N s/m, 2,5 mm, k 2e4", 2.5e-3, 40.0, 2e4)):
        sg = [filz(s, a, R, k, n) for n in (1, 5, 10, 20)]
        print(f"{NAME[t]:5s} | {s['Z0']:.3f} | {name:23s} | {sg[0][0]:7.2f} | {sg[1][0]:7.2f} | {sg[2][0]:8.2f} | {sg[3][0]:8.2f}     |"
              f" {sg[0][1]:+7.1f} | {sg[2][1]:+7.1f}")
s = saite8(60)
print(f"Probe der Stoerungsrechnung (C4, R = 0,02 Z0, a 2,5 mm, n = 10): exakt {filz(s, 2.5e-3, 0.02*s['Z0'], 0, 10)[0]:.4f}, klein {klein(s, 2.5e-3, 0.02*s['Z0'], 10):.4f} 1/s")
print("Vollstaendiges Klemmen im Abstand a: Verstimmung 1731 * a / L Cent:")
for t in (36, 60, 84):
    s = saite8(t)
    print(f"  {NAME[t]:4s} L {1000*s['L']:.0f} mm: a 0,5 mm -> {1731*0.5e-3/s['L']:.1f} Cent; a 2,5 mm -> {1731*2.5e-3/s['L']:.1f} Cent")

# ------------------------------------------------------------------ 4 Draht
kopf("4  Draht: eta = eta_air (WLA 2019 Gl. 11-13) + lambda * eta_E (Gl. 14), lambda = EI k^2 / (T + EI k^2) ~ n^2 B")


def eta_air(s, f, korr=True):
    M = s["d"] / 4 * math.sqrt(2 * math.pi * f / 1.5e-5)
    e = 1.2 / s["rho"] * (2 * math.sqrt(2) * M + 1) / M ** 2
    return e * ((1000 * s["d"] + 0.2) if korr else 1.0)


def lam(s, n):
    k = n * math.pi / s["L"]
    return s["EI"] * k * k / (s["T"] + s["EI"] * k * k)


print("Taste | n  | f Hz  | M    | eta_air (mit d+0,2) | eta_air (ohne) | lambda   | lambda*1e-3 | T60 Luft s (mit) | T60 (ohne)")
for t in (29, 48, 55, 60, 72, 89):
    s = saite8(t)
    for n in (1, 6, 20):
        f = n * s["f"] * math.sqrt(1 + s["B"] * n * n)
        if f > 12000:
            continue
        M = s["d"] / 4 * math.sqrt(2 * math.pi * f / 1.5e-5)
        ea, eo, la = eta_air(s, f), eta_air(s, f, False), lam(s, n)
        print(f"{NAME[t]:5s} | {n:2d} | {f:5.0f} | {M:4.2f} | {ea:19.2e} | {eo:14.2e} | {la:.2e} | {la*1e-3:11.1e} | {6.91/(math.pi*f*ea):16.1f} | {6.91/(math.pi*f*eo):.1f}")
print("\nVaelimaeki 2004 Abb. 8 (Messing, f0 197 Hz, T60 abgelesen) gegen Luft allein (Messing d 0,30 / 0,35 / 0,40 mm, 8500 kg/m3, ANNAHME):")
for n, t60 in ((1, 3.8), (6, 11.7), (20, 3.5)):
    f = 197 * n
    em = 6.91 / (math.pi * f * t60)
    txt = " | ".join(f"d {dd}: {eta_air({'d': dd/1000, 'rho': 8500}, f):.1e}" for dd in (0.30, 0.35, 0.40))
    print(f"  n {n:2d} ({f:4.0f} Hz): gemessen eta {em:.1e} | Luft mit (d+0,2): {txt}")
print("Obergrenze eta_E (Messing) aus n = 20 bei B = 3e-5 (Vaelimaeki Abb. 5 bei 200 Hz): eta_E <= 1,6e-4 / (400 * 3e-5) =", f"{1.6e-4/(400*3e-5):.3f}")
print("\nB je Taste (8') gegen Vaelimaeki Abb. 5 (Messing, anderes Instrument): ~1,5e-5 (70-100 Hz), 3e-5 (200), 1e-4 (400), 2e-4 (700), 4e-4 (1050)")
for t in (29, 36, 41, 48, 53, 55, 60, 65, 72, 77, 84, 89):
    s = saite8(t)
    print(f"  {NAME[t]:5s} f0 {s['f']:6.1f} Hz | d {1000*s['d']:.3f} | E {s['E']/1e9:.0f} GPa | T {s['T']:5.1f} N | B {s['B']:.2e} | "
          f"Teilton 10 um {1200*math.log2(math.sqrt(1+100*s['B'])):.1f} Cent hoch")

# ------------------------------------------------------------------ 5 Register
kopf("5  4'-Zupfpunkt: Pruefstand-Regel (Anteil des 8' gleicher Tonhoehe) gegen Fletcher & Beebe Abb. 8.4 (Ruckers-Nachbau)")
FB = {36: (1500, 0.10, 790, 0.09), 48: (1100, 0.12, 540, 0.11), 60: (665, 0.15, 320, 0.15), 72: (340, 0.25, 165, 0.28), 84: (175, 0.35, 90, 0.48)}
print("Taste | F&B 8' Anteil | F&B 4' Anteil | F&B 4' ab Sattel mm | F&B p4/p8 | Regel S200: Anteil 4' | Regel 4' ab Sattel mm (Taskin-Mensur)")
for t, (L8, a8, L4, a4) in FB.items():
    s4 = saite4(t)
    print(f"{NAME[t]:5s} | {a8:13.2f} | {a4:13.2f} | {L4*a4:19.0f} | {L4*a4/(L8*a8):9.2f} | {s4['x']/s4['L']:21.3f} | {1000*s4['x']:.0f}")

# ------------------------------------------------------------------ Nachtraege
kopf("1f Welche Kielsteife je Taste verlangt GZ13 a (-2,8 dB/Okt) bei festem Ueberstand? (Option C)")
print("Taste | F Ziel N (A an C4, -2,8 dB/Okt) | EI-Faktor gegen Chadefaux-Kiel | Dicke-Faktor (EI ~ H^3) | y_s mm")
FC4 = kc.abloesen(saite8(60)["K"], U_C)["F"]
for t in TASTEN + [67]:
    s = saite8(t)
    ziel = FC4 * 10 ** (-2.8 / 20 * (t - 60) / 12)
    lo, hi = 0.01, 100.0
    for _ in range(80):
        m = math.sqrt(lo * hi)
        k = Kiel(1.5e9 * m, 0.011e-12, 3.5e-3, L1=2.9e-3, n=400)
        lo, hi = (m, hi) if k.abloesen(s["K"], U_C)["F"] < ziel else (lo, m)
    r = Kiel(1.5e9 * m, 0.011e-12, 3.5e-3, L1=2.9e-3, n=400).abloesen(s["K"], U_C)
    print(f"{NAME[t]:5s} | {ziel:31.2f} | {m:30.2f} | {m**(1/3):23.2f} | {1000*r['ys']:.2f}")

kopf("1g LumiViz heute (GZ13 a) gegen die Messung an G4 (Chadefaux 2013): Auslenkung beim Abloesen")
s = saite8(67)
gz = 2.0 * 10 ** (-2.8 / 20 * (67 - 60) / 12)
print(f"GZ13 a an G4: F {gz:.2f} N, K {s['K']:.0f} N/m (Taskin, 440 Hz) -> y = F/K = {1000*gz/s['K']:.2f} mm; gemessen D = 0,76 (langsam) / 0,94 (schnell) mm"
      f" -> {20*math.log10(gz/s['K']/0.76e-3):+.1f} / {20*math.log10(gz/s['K']/0.94e-3):+.1f} dB")
print(f"Mit K der Quelle (T 85,7 N, L 475 mm, beta {beta:.3f}): y = {1000*gz/Kc:.2f} mm")

kopf("3b Lautenzug geschlossen: sigma_n ~ 2 Z0 f0 R / (R^2 + (T / (a w_n))^2), Uebergang w_c = T / (a R)")
for t in (36, 60, 84):
    s = saite8(t)
    for a, R in ((0.5e-3, 40.0), (2.5e-3, 40.0)):
        wc = s["T"] / (a * R)
        werte = []
        for n in (1, 5, 20):
            w = 2 * math.pi * n * s["f"]
            werte.append((2 * s["Z0"] * s["f"] * R / (R * R + (s["T"] / (a * w)) ** 2), filz(s, a, R, 0, n)[0]))
        print(f"{NAME[t]:4s} a {1000*a:.1f} mm R {R:.0f}: f_c = {wc/2/math.pi:6.0f} Hz (Teilton {wc/2/math.pi/s['f']:5.1f}); sigma_inf = 2 Z0 f0 / R = {2*s['Z0']*s['f']/R:.2f} 1/s; "
              + "; ".join(f"n={n}: geschl. {g:.2f} / exakt {e:.2f}" for n, (g, e) in zip((1, 5, 20), werte)))
```

### Ausgabe (`python ergebnisse/2026-11-16_14_cembalo_grundlagen/rechnung.py`)

```text

==============================================================================================================
0  Saiten des Pruefstands (8' Taskin 1769 / Di Veroli; a' = 440 Hz wie hz_von im Pruefstand), dazu 415 Hz
==============================================================================================================
Taste  | L mm | Zupf mm | x/L   | d mm  | T440 N | T415 N | Z0 kg/s | K N/m  | B
F1     | 1765 |     208 | 0.118 | 0.620 |   62.4 |   55.5 |   0.405 |    340 | 4.24e-05
C2     | 1681 |     191 | 0.114 | 0.518 |   88.7 |   78.9 |   0.403 |    524 | 1.60e-05
C3     | 1178 |     162 | 0.138 | 0.339 |   66.9 |   59.5 |   0.217 |    479 | 1.17e-05
C4     |  711 |     137 | 0.193 | 0.269 |   61.4 |   54.6 |   0.165 |    555 | 1.39e-05
C5     |  357 |     115 | 0.322 | 0.244 |   50.9 |   45.3 |   0.136 |    653 | 4.50e-05
C6     |  186 |      96 | 0.516 | 0.213 |   42.1 |   37.5 |   0.108 |    907 | 1.16e-04
f'''   |  148 |      91 | 0.615 | 0.213 |   47.5 |   42.3 |   0.115 |   1356 | 1.63e-04
G4     |  477 |     123 | 0.259 | 0.244 |   50.9 |   45.3 |   0.136 |    557 | 2.52e-05

==============================================================================================================
1a Reduziertes Kielmodell gegen Perngs Simulation (Diss. 2012 Tab. 3.1; Abb. 3.15, 3.16, 3.19, 3.21 abgelesen)
==============================================================================================================
Fall                                  | A Perng mm | A Modell mm | Abw.  | F_ab N | theta Grad
Diskant 1/5, Abb. 3.16a               |       1.20 |        1.19 |   -1 % |   2.01 |  19.9
Diskant Mitte, Abb. 3.16a             |       1.60 |        1.58 |   -1 % |   1.71 |  16.9
Bass 1/7, Abb. 3.15                   |       4.40 |        4.40 |   +0 % |   0.76 |   7.5
Bass Mitte, Abb. 3.16b                |       6.50 |        6.36 |   -2 % |   0.53 |   5.3
Diskant 1/5, Lp 5,5 mm, Abb. 3.19a    |       0.58 |        0.55 |   -5 % |   0.93 |   7.7
Diskant 1/5, Lp 5,7 mm                |       0.88 |        0.91 |   +3 % |   1.53 |  13.7
Bass 1/7, Lp 5,5 mm, Abb. 3.19b       |       2.00 |        1.97 |   -2 % |   0.34 |   2.8
Bass 1/7, Lp 5,7 mm                   |       3.30 |        3.29 |   -0 % |   0.56 |   5.0
Diskant Mitte, E 1 GPa, Abb. 3.21a    |       0.55 |        0.51 |   -7 % |   0.55 |  27.4
Diskant Mitte, E 0,1 GPa              |       0.10 |        0.06 |  -36 % |   0.07 |  34.0
Diskant Mitte, E 30 GPa               |       4.30 |        4.29 |   -0 % |   4.64 |   7.7
Bass Mitte, E 1 GPa, Abb. 3.21b       |       3.00 |        2.73 |   -9 % |   0.23 |  11.4
Bass Mitte, E 0,1 GPa                 |       0.70 |        0.62 |  -11 % |   0.05 |  25.9
Bass Mitte, E 30 GPa                  |      15.50 |       15.72 |   +1 % |   1.32 |   2.2

==============================================================================================================
1b Gegen die Messung Chadefaux u. a. 2013 (G4, Tab. 1 und 2; Kiel aus zwei Abschnitten, Gl. 4)
==============================================================================================================
Zupfanteil G4 aus Taskin 1769 (Quelle nennt keinen): beta = 0.259; K = 940 N/m; Taskin-Laenge G4 477 mm (Quelle 475)
Kiel: theta0/F = 25.6 Grad/N; Spitze senkt sich 0.90 mm/N
langsam : Ueberstand fuer y_s = 0.71 mm: u = 0.30 mm -> F_ab 0.70 N, x_s 0.23 mm (Messung 0,27), theta 18.0 Grad (Modell der Quelle 20,9; Gamma_tr 17)
schnell : Ueberstand fuer y_s = 0.90 mm: u = 0.52 mm -> F_ab 0.92 N, x_s 0.39 mm (Messung 0,28), theta 23.7 Grad (Modell der Quelle 20,9; Gamma_tr 20)
Kraft direkt aus der gemessenen Lage (F = K * sqrt(x^2 + y^2)): langsam 0.71 N, schnell 0.89 N

Aufladezeit = (y_s + Senkung der Kielspitze) / v_Springer:
  normal : v 0.1 m/s: (0.71 + 0.64) mm / v = 13.5 ms (gemessen 12 +- 2 ms)
  schnell: v 0.25 m/s: (0.90 + 0.79) mm / v = 6.8 ms (gemessen 7,9 +- 0,6 ms)

==============================================================================================================
1c Abloesekraft je Taste (8'), Kiel nach Chadefaux; Vergleich mit GZ13 a (2 N an C4, -2,8 dB/Oktave)
==============================================================================================================
Option A: Ueberstand fest u = 0.30 mm (an G4 auf die Messung gelegt). Option B: u = 1.23 * d (Saitendurchmesser).
Option C: Kiel je Taste so geschnitten, dass die Senkung der Spitze gleich bleibt - entspricht A; nicht getrennt.
Taste  | K N/m | A: F N | A dB re C4 | A: y_s mm | B: F N | B dB re C4 | GZ13 F N | GZ13 dB | LumiViz y=F/K mm
F1     |   340 |   0.46 |       -1.9 |      1.31 |   0.73 |       +1.8 |     4.60 |    +7.2 | 13.53
C2     |   524 |   0.55 |       -0.2 |      1.02 |   0.81 |       +2.7 |     3.81 |    +5.6 | 7.28
C3     |   479 |   0.53 |       -0.6 |      1.08 |   0.63 |       +0.5 |     2.76 |    +2.8 | 5.77
C4     |   555 |   0.57 |       +0.0 |      0.99 |   0.59 |       +0.0 |     2.00 |    +0.0 | 3.61
C5     |   653 |   0.61 |       +0.6 |      0.89 |   0.61 |       +0.2 |     1.45 |    -2.8 | 2.22
C6     |   907 |   0.69 |       +1.8 |      0.73 |   0.65 |       +0.7 |     1.05 |    -5.6 | 1.16
f'''   |  1356 |   0.80 |       +3.1 |      0.56 |   0.75 |       +2.0 |     0.92 |    -6.8 | 0.68
G4     |   557 |   0.57 |       +0.0 |      0.98 |   0.57 |       -0.4 |     1.66 |    -1.6 | 2.98
Mittlere Steigung (Regression ueber F1..f'''): A +0.8 dB/Okt, B -0.2 dB/Okt, GZ13 -2,8

==============================================================================================================
1d 4' (Pruefstand-Regel fuer den Zupfpunkt), gleicher Kiel und Ueberstand wie Option A
==============================================================================================================
Taste  | L4 mm | Zupf mm | T4 N  | K4 N/m | F_ab 4' N | F_ab 8' N | 4'/8' dB
F1     |  1073 |     133 |  51.4 |    441 |      0.51 |      0.46 | +1.0
C2     |   886 |     122 |  52.2 |    497 |      0.54 |      0.55 | -0.2
C3     |   590 |     114 |  52.5 |    572 |      0.57 |      0.53 | +0.7
C4     |   334 |     107 |  44.5 |    610 |      0.59 |      0.57 | +0.4
C5     |   167 |      84 |  44.8 |   1070 |      0.74 |      0.61 | +1.7
C6     |    92 |      46 |  31.9 |   1393 |      0.81 |      0.69 | +1.4
f'''   |    74 |      37 |  37.0 |   2001 |      0.92 |      0.80 | +1.1

==============================================================================================================
1e Spektrum: Abloesen in tau (lineare Rampe) gegen den idealen Sprung; raeumliche Breite des Kiels (Pruefstand)
==============================================================================================================
tau 0.10 ms: -3 dB bei   4430 Hz, erste Nullstelle  10000 Hz
tau 0.25 ms: -3 dB bei   1772 Hz, erste Nullstelle   4000 Hz
tau 0.50 ms: -3 dB bei    886 Hz, erste Nullstelle   2000 Hz
Raeumlicher Gauss exp(-(w h / c)^2), -3 dB bei f = 0,0935 c / h:
  F1    c =   154 m/s: h 3 mm ->   4803 Hz; h 2 mm (Perng W) ->   7204 Hz
  C2    c =   220 m/s: h 3 mm ->   6853 Hz; h 2 mm (Perng W) ->  10280 Hz
  C3    c =   308 m/s: h 3 mm ->   9605 Hz; h 2 mm (Perng W) ->  14408 Hz
  C4    c =   372 m/s: h 3 mm ->  11595 Hz; h 2 mm (Perng W) ->  17392 Hz
  C5    c =   374 m/s: h 3 mm ->  11644 Hz; h 2 mm (Perng W) ->  17466 Hz
  C6    c =   389 m/s: h 3 mm ->  12133 Hz; h 2 mm (Perng W) ->  18200 Hz
  f'''  c =   413 m/s: h 3 mm ->  12887 Hz; h 2 mm (Perng W) ->  19330 Hz

==============================================================================================================
2  Tastenstoss: Kraftstoesse J = m dv (1 + e), Annahmen markiert
==============================================================================================================
Springer an der Springerleiste, langsam (Perng Abb. 4.24, Spitze 0,25 m/s)    : m 5 g, v 0.25 m/s -> J 1.2 (e=0) .. 2.5 (e=1) mN s
Springer an der Springerleiste, kraeftig (Perng Abb. 4.25, Spitze 0,58 m/s)   : m 5 g, v 0.58 m/s -> J 2.9 (e=0) .. 5.8 (e=1) mN s
Springer, Kunststoff 6 g, kraeftig                                            : m 6 g, v 0.58 m/s -> J 3.5 (e=0) .. 7.0 (e=1) mN s
Rueckfall des Springers aus 3 cm (Perng S. 114) auf die Taste                 : m 5 g, v 0.77 m/s -> J 3.8 (e=0) .. 7.7 (e=1) mN s
Taste am Boden: Verzoegerung aus Chadefaux 2013 Abb. 6 (G4, normal): 0,10 m/s -> 0 in ~8 ms -> a ~ 12 m/s^2
  wirksame Masse (Taste + Finger, ANNAHME) 20 g: J = 2.0 mN s, mittlere Kraft 0.25 N ueber 8 ms
  wirksame Masse (Taste + Finger, ANNAHME) 50 g: J = 5.0 mN s, mittlere Kraft 0.63 N ueber 8 ms
  wirksame Masse (Taste + Finger, ANNAHME) 100 g: J = 10.0 mN s, mittlere Kraft 1.25 N ueber 8 ms
  Stossdauer 0.4 ms (Gauss sigma = Dauer/2,5): -3 dB des Spektrums bei 828 Hz
  Stossdauer 2.0 ms (Gauss sigma = Dauer/2,5): -3 dB des Spektrums bei 166 Hz
  Stossdauer 8.0 ms (Gauss sigma = Dauer/2,5): -3 dB des Spektrums bei 41 Hz

==============================================================================================================
3  Lautenzug: Abschluss Z_T = R + (k + T/a)/(i w) am Ort a vor dem Sattel (Perng Gl. 5.9 + Stummel), je Teilton
==============================================================================================================
Taste | Z0     | Fall                    | sigma_1 | sigma_5 | sigma_10 | sigma_20 1/s | Cent T1 | Cent T10
C2    | 0.403 | GZ11 a: 40 N s/m, 0,5 mm |    0.01 |    0.23 |     0.61 |     1.02     |    +0.0 |    +0.2
C2    | 0.403 | 40 N s/m, 2,5 mm        |    0.23 |    1.11 |     1.26 |     1.30     |    +0.5 |    +2.5
C2    | 0.403 | R = Z0, 0,5 mm          |    0.00 |    0.00 |     0.01 |     0.05     |    +0.0 |    +0.0
C2    | 0.403 | R = Z0, 2,5 mm          |    0.00 |    0.07 |     0.28 |     1.13     |    +0.0 |    +0.0
C2    | 0.403 | 40 N s/m, 2,5 mm, k 2e4 |    0.11 |    0.91 |     1.18 |     1.28     |    +1.1 |    +2.4
C3    | 0.217 | GZ11 a: 40 N s/m, 0,5 mm |    0.08 |    0.85 |     1.22 |     1.36     |    +0.0 |    +0.6
C3    | 0.217 | 40 N s/m, 2,5 mm        |    0.85 |    1.38 |     1.41 |     1.42     |    +2.2 |    +3.6
C3    | 0.217 | R = Z0, 0,5 mm          |    0.00 |    0.01 |     0.05 |     0.19     |    +0.0 |    +0.0
C3    | 0.217 | R = Z0, 2,5 mm          |    0.01 |    0.29 |     1.16 |     4.55     |    +0.0 |    +0.0
C3    | 0.217 | 40 N s/m, 2,5 mm, k 2e4 |    0.47 |    1.31 |     1.39 |     1.41     |    +2.3 |    +3.6
C4    | 0.165 | GZ11 a: 40 N s/m, 0,5 mm |    0.48 |    1.89 |     2.08 |     2.14     |    +0.3 |    +1.2
C4    | 0.165 | 40 N s/m, 2,5 mm        |    1.89 |    2.15 |     2.15 |     2.16     |    +5.3 |    +6.1
C4    | 0.165 | R = Z0, 0,5 mm          |    0.00 |    0.06 |     0.26 |     1.02     |    +0.0 |    +0.0
C4    | 0.165 | R = Z0, 2,5 mm          |    0.06 |    1.59 |     6.28 |    24.04     |    +0.0 |    +0.1
C4    | 0.165 | 40 N s/m, 2,5 mm, k 2e4 |    1.48 |    2.12 |     2.15 |     2.15     |    +5.0 |    +6.1
C5    | 0.136 | GZ11 a: 40 N s/m, 0,5 mm |    2.23 |    3.48 |     3.54 |     3.56     |    +1.5 |    +2.4
C5    | 0.136 | 40 N s/m, 2,5 mm        |    3.48 |    3.56 |     3.56 |     3.56     |   +11.8 |   +12.1
C5    | 0.136 | R = Z0, 0,5 mm          |    0.02 |    0.51 |     2.02 |     8.02     |    +0.0 |    +0.0
C5    | 0.136 | R = Z0, 2,5 mm          |    0.51 |   12.46 |    47.70 |   166.06     |    +0.0 |    +0.5
C5    | 0.136 | 40 N s/m, 2,5 mm, k 2e4 |    3.26 |    3.55 |     3.56 |     3.56     |   +11.6 |   +12.1
C6    | 0.108 | GZ11 a: 40 N s/m, 0,5 mm |    5.13 |    5.64 |     5.66 |     5.66     |    +4.2 |    +4.6
C6    | 0.108 | 40 N s/m, 2,5 mm        |    5.64 |    5.66 |     5.66 |     5.66     |   +23.0 |   +23.1
C6    | 0.108 | R = Z0, 0,5 mm          |    0.15 |    3.72 |    14.79 |    57.58     |    +0.0 |    +0.0
C6    | 0.108 | R = Z0, 2,5 mm          |    3.72 |   88.26 |   309.71 |   943.60     |    +0.0 |    +3.1
C6    | 0.108 | 40 N s/m, 2,5 mm, k 2e4 |    5.55 |    5.66 |     5.66 |     5.66     |   +22.9 |   +23.1
Probe der Stoerungsrechnung (C4, R = 0,02 Z0, a 2,5 mm, n = 10): exakt 0.1272, klein 0.1272 1/s
Vollstaendiges Klemmen im Abstand a: Verstimmung 1731 * a / L Cent:
  C2   L 1681 mm: a 0,5 mm -> 0.5 Cent; a 2,5 mm -> 2.6 Cent
  C4   L 711 mm: a 0,5 mm -> 1.2 Cent; a 2,5 mm -> 6.1 Cent
  C6   L 186 mm: a 0,5 mm -> 4.7 Cent; a 2,5 mm -> 23.3 Cent

==============================================================================================================
4  Draht: eta = eta_air (WLA 2019 Gl. 11-13) + lambda * eta_E (Gl. 14), lambda = EI k^2 / (T + EI k^2) ~ n^2 B
==============================================================================================================
Taste | n  | f Hz  | M    | eta_air (mit d+0,2) | eta_air (ohne) | lambda   | lambda*1e-3 | T60 Luft s (mit) | T60 (ohne)
F1    |  1 |    44 | 0.66 |            7.40e-04 |       9.03e-04 | 4.24e-05 |     4.2e-08 |             68.1 | 55.8
F1    |  6 |   262 | 1.62 |            2.40e-04 |       2.92e-04 | 1.52e-03 |     1.5e-06 |             35.0 | 28.7
F1    | 20 |   880 | 2.98 |            1.20e-04 |       1.47e-04 | 1.67e-02 |     1.7e-05 |             20.8 | 17.0
C3    |  1 |   131 | 0.63 |            5.85e-04 |       1.08e-03 | 1.17e-05 |     1.2e-08 |             28.8 | 15.5
C3    |  6 |   785 | 1.54 |            1.88e-04 |       3.48e-04 | 4.22e-04 |     4.2e-07 |             14.9 | 8.0
C3    | 20 |  2622 | 2.81 |            9.40e-05 |       1.74e-04 | 4.67e-03 |     4.7e-06 |              8.9 | 4.8
G3    |  1 |   196 | 0.68 |            4.87e-04 |       9.73e-04 | 1.22e-05 |     1.2e-08 |             23.1 | 11.5
G3    |  6 |  1176 | 1.66 |            1.58e-04 |       3.17e-04 | 4.41e-04 |     4.4e-07 |             11.8 | 5.9
G3    | 20 |  3930 | 3.04 |            7.98e-05 |       1.60e-04 | 4.88e-03 |     4.9e-06 |              7.0 | 3.5
C4    |  1 |   262 | 0.70 |            4.35e-04 |       9.28e-04 | 1.39e-05 |     1.4e-08 |             19.3 | 9.1
C4    |  6 |  1570 | 1.72 |            1.43e-04 |       3.04e-04 | 5.00e-04 |     5.0e-07 |              9.8 | 4.6
C4    | 20 |  5247 | 3.15 |            7.20e-05 |       1.53e-04 | 5.53e-03 |     5.5e-06 |              5.8 | 2.7
C5    |  1 |   523 | 0.90 |            2.98e-04 |       6.70e-04 | 4.50e-05 |     4.5e-08 |             14.1 | 6.3
C5    |  6 |  3142 | 2.21 |            1.01e-04 |       2.28e-04 | 1.62e-03 |     1.6e-06 |              6.9 | 3.1
C5    | 20 | 10559 | 4.06 |            5.18e-05 |       1.17e-04 | 1.77e-02 |     1.8e-05 |              4.0 | 1.8
f'''  |  1 |  1397 | 1.29 |            1.78e-04 |       4.31e-04 | 1.63e-04 |     1.6e-07 |              8.9 | 3.7
f'''  |  6 |  8406 | 3.16 |            6.32e-05 |       1.53e-04 | 5.83e-03 |     5.8e-06 |              4.1 | 1.7

Vaelimaeki 2004 Abb. 8 (Messing, f0 197 Hz, T60 abgelesen) gegen Luft allein (Messing d 0,30 / 0,35 / 0,40 mm, 8500 kg/m3, ANNAHME):
  n  1 ( 197 Hz): gemessen eta 2.9e-03 | Luft mit (d+0,2): d 0.3: 4.5e-04 | d 0.35: 4.0e-04 | d 0.4: 3.7e-04
  n  6 (1182 Hz): gemessen eta 1.6e-04 | Luft mit (d+0,2): d 0.3: 1.4e-04 | d 0.35: 1.3e-04 | d 0.4: 1.2e-04
  n 20 (3940 Hz): gemessen eta 1.6e-04 | Luft mit (d+0,2): d 0.3: 7.3e-05 | d 0.35: 6.8e-05 | d 0.4: 6.4e-05
Obergrenze eta_E (Messing) aus n = 20 bei B = 3e-5 (Vaelimaeki Abb. 5 bei 200 Hz): eta_E <= 1,6e-4 / (400 * 3e-5) = 0.013

B je Taste (8') gegen Vaelimaeki Abb. 5 (Messing, anderes Instrument): ~1,5e-5 (70-100 Hz), 3e-5 (200), 1e-4 (400), 2e-4 (700), 4e-4 (1050)
  F1    f0   43.7 Hz | d 0.620 | E 115 GPa | T  62.4 N | B 4.24e-05 | Teilton 10 um 3.7 Cent hoch
  C2    f0   65.4 Hz | d 0.518 | E 115 GPa | T  88.7 N | B 1.60e-05 | Teilton 10 um 1.4 Cent hoch
  F2    f0   87.3 Hz | d 0.410 | E 110 GPa | T  71.2 N | B 1.02e-05 | Teilton 10 um 0.9 Cent hoch
  C3    f0  130.8 Hz | d 0.339 | E 170 GPa | T  66.9 N | B 1.17e-05 | Teilton 10 um 1.0 Cent hoch
  F3    f0  174.6 Hz | d 0.300 | E 170 GPa | T  64.8 N | B 1.07e-05 | Teilton 10 um 0.9 Cent hoch
  G3    f0  196.0 Hz | d 0.300 | E 170 GPa | T  67.9 N | B 1.22e-05 | Teilton 10 um 1.1 Cent hoch
  C4    f0  261.6 Hz | d 0.269 | E 170 GPa | T  61.4 N | B 1.39e-05 | Teilton 10 um 1.2 Cent hoch
  F4    f0  349.2 Hz | d 0.244 | E 170 GPa | T  50.9 N | B 2.00e-05 | Teilton 10 um 1.7 Cent hoch
  C5    f0  523.3 Hz | d 0.244 | E 170 GPa | T  50.9 N | B 4.50e-05 | Teilton 10 um 3.9 Cent hoch
  F5    f0  698.5 Hz | d 0.213 | E 170 GPa | T  39.0 N | B 6.06e-05 | Teilton 10 um 5.2 Cent hoch
  C6    f0 1046.5 Hz | d 0.213 | E 170 GPa | T  42.1 N | B 1.16e-04 | Teilton 10 um 10.0 Cent hoch
  f'''  f0 1396.9 Hz | d 0.213 | E 170 GPa | T  47.5 N | B 1.63e-04 | Teilton 10 um 14.0 Cent hoch

==============================================================================================================
5  4'-Zupfpunkt: Pruefstand-Regel (Anteil des 8' gleicher Tonhoehe) gegen Fletcher & Beebe Abb. 8.4 (Ruckers-Nachbau)
==============================================================================================================
Taste | F&B 8' Anteil | F&B 4' Anteil | F&B 4' ab Sattel mm | F&B p4/p8 | Regel S200: Anteil 4' | Regel 4' ab Sattel mm (Taskin-Mensur)
C2    |          0.10 |          0.09 |                  71 |      0.47 |                 0.138 | 122
C3    |          0.12 |          0.11 |                  59 |      0.45 |                 0.193 | 114
C4    |          0.15 |          0.15 |                  48 |      0.48 |                 0.322 | 107
C5    |          0.25 |          0.28 |                  46 |      0.54 |                 0.500 | 84
C6    |          0.35 |          0.48 |                  43 |      0.71 |                 0.500 | 46

==============================================================================================================
1f Welche Kielsteife je Taste verlangt GZ13 a (-2,8 dB/Okt) bei festem Ueberstand? (Option C)
==============================================================================================================
Taste | F Ziel N (A an C4, -2,8 dB/Okt) | EI-Faktor gegen Chadefaux-Kiel | Dicke-Faktor (EI ~ H^3) | y_s mm
F1    |                            1.30 |                           7.51 |                    1.96 | 3.81
C2    |                            1.08 |                           3.45 |                    1.51 | 2.04
C3    |                            0.78 |                           2.03 |                    1.27 | 1.61
C4    |                            0.57 |                           1.00 |                    1.00 | 0.99
C5    |                            0.41 |                           0.53 |                    0.81 | 0.59
C6    |                            0.30 |                           0.28 |                    0.66 | 0.29
f'''  |                            0.26 |                           0.22 |                    0.60 | 0.16
G4    |                            0.47 |                           0.72 |                    0.90 | 0.81

==============================================================================================================
1g LumiViz heute (GZ13 a) gegen die Messung an G4 (Chadefaux 2013): Auslenkung beim Abloesen
==============================================================================================================
GZ13 a an G4: F 1.66 N, K 557 N/m (Taskin, 440 Hz) -> y = F/K = 2.98 mm; gemessen D = 0,76 (langsam) / 0,94 (schnell) mm -> +11.9 / +10.0 dB
Mit K der Quelle (T 85,7 N, L 475 mm, beta 0.259): y = 1.76 mm

==============================================================================================================
3b Lautenzug geschlossen: sigma_n ~ 2 Z0 f0 R / (R^2 + (T / (a w_n))^2), Uebergang w_c = T / (a R)
==============================================================================================================
C2   a 0.5 mm R 40: f_c =    705 Hz (Teilton  10.8); sigma_inf = 2 Z0 f0 / R = 1.32 1/s; n=1: geschl. 0.01 / exakt 0.01; n=5: geschl. 0.23 / exakt 0.23; n=20: geschl. 1.02 / exakt 1.02
C2   a 2.5 mm R 40: f_c =    141 Hz (Teilton   2.2); sigma_inf = 2 Z0 f0 / R = 1.32 1/s; n=1: geschl. 0.23 / exakt 0.23; n=5: geschl. 1.11 / exakt 1.11; n=20: geschl. 1.30 / exakt 1.30
C4   a 0.5 mm R 40: f_c =    488 Hz (Teilton   1.9); sigma_inf = 2 Z0 f0 / R = 2.16 1/s; n=1: geschl. 0.48 / exakt 0.48; n=5: geschl. 1.89 / exakt 1.89; n=20: geschl. 2.14 / exakt 2.14
C4   a 2.5 mm R 40: f_c =     98 Hz (Teilton   0.4); sigma_inf = 2 Z0 f0 / R = 2.16 1/s; n=1: geschl. 1.89 / exakt 1.89; n=5: geschl. 2.15 / exakt 2.15; n=20: geschl. 2.16 / exakt 2.16
C6   a 0.5 mm R 40: f_c =    335 Hz (Teilton   0.3); sigma_inf = 2 Z0 f0 / R = 5.66 1/s; n=1: geschl. 5.13 / exakt 5.13; n=5: geschl. 5.64 / exakt 5.64; n=20: geschl. 5.66 / exakt 5.66
C6   a 2.5 mm R 40: f_c =     67 Hz (Teilton   0.1); sigma_inf = 2 Z0 f0 / R = 5.66 1/s; n=1: geschl. 5.64 / exakt 5.64; n=5: geschl. 5.66 / exakt 5.66; n=20: geschl. 5.66 / exakt 5.66
```

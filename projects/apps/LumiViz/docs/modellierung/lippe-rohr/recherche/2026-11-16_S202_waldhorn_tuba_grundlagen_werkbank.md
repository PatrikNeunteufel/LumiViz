> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_18_waldhorn_tuba_grundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-17 09:35–09:58, claude-opus-5-5, 115 Runden, success, 7.12 USD; Status `fertig`.
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Quellen unter `…\cmake\quellen\lippe-rohr\`):
> - **Wachter 2008 Abb. 52 (S. 55) am Seitenbild:** alle 16 Wertepaare des Mundstücks Ebner stimmen. Textstellen S. 54 /
>   59 / 81 / 82 (Seriennummer 777, „um cirka 1 m“, „mittels Schnur“, „jeweils tiefer“, „Stimmzuges ausschlaggebend“)
>   gefunden.
> - **Silva u. a. (arXiv 0811.3625) Tab. 1:** alle neun Beiwerte, „less than 2%“, „complex conjugation … is needed“
>   gefunden. Hendrie „replace the unflanged pipe … ℒ + 0.6r“ gefunden.
> - **Mattéoli u. a. Tab. 2:** die Quelle gibt **1/µ = 1,1·10⁻¹ m²/kg** (Spalte „µ [m² kg⁻¹]“), w = 1,2·10⁻² m, Q = 7,0;
>   die µ = 9,1 kg/m² des Berichts sind daraus richtig umgerechnet.
> - **Nicht gegengelesen:** die Digitalisierung von Abb. 88 (Bohrung des F-Horns, 296 Punkte) und Abb. 58 / 89,
>   Wachter Tab. 85 (Spitzen 2–15), die Rechnung `horn_rechnung.py` (±1,1 dB, −46 / +0,5 Cent), Keefe-Reihe G3.
> - **Nacharbeit:** Auftrag `2026-11-17_28_waldhorn_tuba_nacharbeit` der Werkbank (Tuba- und Euphonium-Bohrung, Norman
>   2013, Lippenparameter Horn).
> Der Auftrag liegt seit S202 im Archiv der Werkbank (`archiv/auftraege/`, `archiv/ergebnisse/2026-11-16_18_waldhorn_tuba_grundlagen/` mit
> `seiten/` und Textauszügen, `archiv/laeufe/`). Wo der Bericht `ergebnisse/…` oder `eingang/…` nennt, ist
> `archiv/ergebnisse/…` bzw. `archiv/eingang/…` gemeint.
> **Abgerufene PDFs** liegen nur unter `…\cmake\quellen\` (der Bericht nennt die Namen der Werkbank):
> - `Silva_2009_Abstrahlimpedanz_arXiv0811.3625.pdf` → `quellen\lippe-rohr\Silva_etal_2008_RadiationImpedanceCylindricalPipe_arXiv0811.3625.pdf` — arXiv-Fassung vom 21.11.2008
> **Rechnungen und Ausgaben des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_waldhorn_tuba_grundlagen_werkbank/`: `horn_rechnung.py`, `horn_rechnung_ausgabe.txt`, `digitalisiere_abb88.py`, `digitalisiere_kurve.py`, `bericht_bauen.py`, `abb88_roh.txt`, `abb88_roh_becher.txt`, `bohrung_F_kernform.txt`, `bohrung_F_punkte.txt`.

# Bericht: Berechnungsgrundlage für Waldhorn und Tuba aus der Bohrung (Stufe 34m, Stufe 2)

- **Auftrag:** `auftraege/2026-11-16_18_waldhorn_tuba_grundlagen.md`
- **Voraussetzung:** Stufe-1-Bericht `ergebnisse/2026-11-16_08_waldhorn_tuba_bohrung/bericht.md` liegt vor und wurde gelesen.
- **Datum:** 2026-11-17
- **Modell:** claude-opus-5-5
- **Reichweite:** 1 Websuche, 1 Abruf über das Abrufwerkzeug (Silva et al. 2009, arXiv 0811.3625, Sitzungsordner
  `webfetch-1794904979296-68p93d.pdf`). Lokal gelesen, Formeln und Tabellen **am Seitenbild**: Wachter 2008
  (`eingang/2026-11-16_07_alphorn_nachpruefung/Wachter_2008_stopfen_hoerner.pdf`, PDF-S. 32–33, 60, 64, 83, 87),
  Mattéoli et al. (`quellen/lippe-rohr/2207.05395v3.pdf`, S. 2, 3, 5), Hendrie 2007 (`quellen/lippe-rohr/Hendrie DA
  thesis 07.pdf`, PDF-S. 59), Silva et al. 2009 (S. 12). Nur über die Textebene durchsucht: Wachter Kap. 4–6
  (PDF-S. 59–125), Hendrie (Bessel-Horn, Abschnitt 7.2), Mattéoli (Blasdruck), `1611.01025v1.pdf`. Im LumiViz-Repo
  gelesen: `Rohr.hpp`, `Rohr.cpp`, `Rohrschleife.hpp`, `Blech.hpp`, `Blech.cpp` (Z. 30–330), `Vorgaben.cpp`, die
  Recherche `2026-11-16_S201_alphorn_geometrie_werkbank.md` und per Suche `modell.md`, `modell-entwurf.md`, `offen.md`,
  `quellen.md`, `2026-10-26_physik_spielweisen.md`. `test_blech.cpp` **nicht** gelesen.
- **Nicht erreicht / nicht versucht:** keine neue Bohrungssuche (Auftrag: Stufe 1 nicht wiederholen). Was in Stufe 1
  gesperrt war, bleibt gesperrt.

## 0. Ergebnis in Kürze

1. **Die Lücke „Waldhorn-Bohrung“ ist für die F-Seite geschlossen — aus einer Quelle, die schon lokal lag:** Wachter
   2008 zeigt in **Abb. 88 (S. 82) die vollständige Mensur des F-Horns** (E. Schmid Doppelhorn Nr. 777, mit Mundstück,
   Ventilstellung 0) als Vektorgrafik. Sie ist hier digitalisiert (`[abgelesen]`, Pixel 1,22 mm in x, 0,17 mm in r):
   **3,862 m, Hauptrohr Ø 11,4 mm, Becher Ø 300 mm**. Am **selben Instrument** liegen gemessene Spitzen 2–15
   (Tab. 85, `[wörtlich]`) und gemessene Impedanzkurven (Abb. 58, 89; Höhen hier digitalisiert). Das Mundstück
   (Abb. 52, `[wörtlich]`) ist das Messmundstück.
2. **Gegenrechnung (Kernmodell, Abschnitt 4):** Höhen der Spitzen 3–15 innerhalb **±1,1 dB** der Messung (Spitze 2:
   +2,7 dB); **Lage im Mittel −46 Cent** (Streuung 16 Cent). Wachter selbst schreibt, die abgemessene Mensur sei
   wegen der Stimmzugstellung länger als das gemessene Horn und rechne „tiefer“ (S. 82). Mit 0,10 m kürzerem
   Hauptrohr: **Mittel +0,5 Cent, Streuung 18 Cent**. Die Reihe bricht in Rechnung und Messung bei der 16./17. Spitze
   (≈ 700 Hz).
3. **Was die Zahlen am stärksten bewegt:** Ort und Radius der **Ventilzüge** — ein Zug an der Seele (so wie der Kern
   heute Ventile in den Zylinder am Eingang legt) verschiebt die Spitzen um bis zu **110 / 174 / 224 Cent** (1 / 2 / 3 Halbtöne) neben das Soll;
   im Hauptrohr liegt jeder Zug ab der 3. Spitze auf ±16 Cent am Soll. Danach Temperatur (≈ 3 Cent/°C) und Stimmzug. Mundstück als Volumen statt
   Profil: +1 … +6 Cent, −0,1 … −0,8 dB. Abstrahlung Levine–Schwinger statt Kleinsignal-Näherung, Keefe 3. statt
   1. Ordnung, Kegel exakt statt Treppe: je ≤ 1,6 Cent.
4. **B-Seite:** keine Bohrung in einer Quelle. Als **Annahme** (F-Bohrung, Hauptrohr um 1,05 m gekürzt; Wachter:
   „cirka 1 m kürzer“) trifft das Modell die aus Abb. 58 digitalisierten B-Spitzen auf −41 … +36 Cent, ±1,7 dB.
5. **Tuba, Euphonium:** **keine Bohrung herleitbar** (Stufe 1: keine Zahlen; Euphonium nur als abgeschnittene
   Zeichnung). Für die Tuba liegen nur **Lippenparameter** vor (Mattéoli Tab. 2, `[wörtlich]`): µ = 9,1 kg/m², Q = 7,
   w = 12 mm, H = 0,074 m·Hz / f_ℓ — sieben- bis achtmal so viel Masse je Fläche wie die heutige Vorgabe des Kerns
   (1,3 kg/m²). Für das Horn: keine Lippenparameter gefunden.

---

## 1. In welcher Form der Kern eine Bohrung nimmt (LumiViz, nur gelesen)

| Was | Wortlaut / Inhalt | Stelle |
|---|---|---|
| Segment | `struct Abschnitt { double laengeM = 0.1; double radiusEinM = 0.01; double radiusAusM = 0.01; };` — „Kegelstumpf; gleich = Zylinder“ | `gemeinsam/Rohr.hpp:55-61` |
| Einheit | **Radius in m**, Länge in m (Feldnamen `radiusEinM`, `laengeM`) | `Rohr.hpp:58-60` |
| Bohrung | `std::vector<Abschnitt> abschnitte; Abschluss ende = Abschluss::Offen; bool verluste = true;` + Dämpfer, Gitter | `Rohr.hpp:71-86` |
| Kegel → Treppe | „Kegelstuempfe werden als Treppe kurzer Zylinder gerechnet (Stufen hoechstens 1 mm, Radius in der Stufenmitte)“; `constexpr double kStufeM = 1e-3;` | `Rohr.cpp:5-7, 21, 92-98` |
| Verluste | `eps = (1.0 + (luft.gamma - 1.0) / std::sqrt(luft.prandtl)) / (std::sqrt(2.0) * rv); gamma = Komplex(0.0, k) * (1.0 + Komplex(1.0, -1.0) * eps);` — Zc verlustfrei `zc = luft.dichte * luft.schall / flaeche` | `Rohr.cpp:29-43` |
| Abstrahlung | `Komplex rad = zc * Komplex(ka * ka / 4.0, 0.6133 * ka);` (Doku: „gueltig ka < 1“) | `Rohr.cpp:110-112`, `Rohr.hpp:14-15` |
| Luft | ρ = 1,2041, c = 343,2, η = 1,8205·10⁻⁵, γ = 1,4, Pr = 0,71 | `Rohr.hpp:46-53` |
| Instrument | `zylinderRadiusM`, `zylinderGrundM` („Laenge des Zylinders … ohne Ventil“), `zugMaxM`, `std::vector<double> ventilM; ///< Zusatzlaengen je Ventil (im Zylinder)`, `Bohrung rest; ///< hinter dem Zylinder bis zum Schallstueck`, `double kesselM3` | `Blech.hpp:66-71` |
| Reihenfolge | `b.abschnitte.insert(b.abschnitte.begin(), Abschnitt{s.zylinderGrundM + zusatzM, s.zylinderRadiusM, s.zylinderRadiusM});` — **der Zylinder steht immer am Eingang**, Ventil-/Zuglänge wird **dort** angehängt | `Blech.cpp:246-250` |
| Mundstück | „Z = 1/(iωC + 1/Z_Bohrung): der Kessel parallel zum Eingang der Bohrung“, `c = s.kesselM3 / (luft.dichte * luft.schall * luft.schall)` — **nur Volumen**, kein Profil | `Blech.cpp:265-274` |
| Becher | `besselHorn(laengeM, r0, r1, eps, stuecke = 60)`: „r(x) ∝ (d1 + L − x)^(−ε) von r0 auf r1 … in `stuecke` Kegelstuempfe“, d1 aus r0, r1, ε | `Vorgaben.cpp:29-42` |
| Ventile | `s.ventilM = {l * (std::exp2(2.0 / 12.0) - 1.0), …}` mit „Laenge der Bohrung L = 1,355 m (Rechnung — die Verlaengerung echter Ventilzuege und der Fehler der Kombinationen offen)“ | `Vorgaben.cpp:177-180` |
| Horn, Tuba heute | `geruest("Tuba", 28, 65, …)`, `geruest("Horn", 34, 77, "keine Bohrung gefunden …")` | `Vorgaben.cpp:202-212` |
| Lippe | µ = 1,3 kg/m², Q = 5, w = 0,01 m, H0 = 2,9·10⁻⁴ m, P 500 … 14 000 Pa (Posaune, Trompete, Alphorn) | `Vorgaben.cpp:93-100, 183-188` |

**Folge für die Form der Zahlen:** Alle Bohrungsangaben unten stehen als **Radius in m** und als **Kegelstümpfe**
(Datei `bohrung_F_kernform.txt`: je Zeile `laengeM radiusEinM radiusAusM`). Ein Horn passt nur so in die heutige
Struktur: `kesselM3` = Kessel, ein kurzer `zylinder…` an der Seele, **alles andere in `rest`** (wie beim Alphorn,
`Vorgaben.cpp:129`). Die Ventilzüge landen dann aber im Eingangszylinder — Abschnitt 4.4 zeigt, was das kostet.

---

## 2. Gleichungen

Konvention: Zeitabhängigkeit e^{+jωt}, Ausbreitung e^{−Γx}. „Bild“ = am Seitenbild geprüft.

| Nr. | Gleichung | Quelle | Bild | Zahlenbeispiel gegen die Quelle |
|---|---|---|---|---|
| G1 | Zylinder: [p₁; U₁] = [[cosh Γl, Z_c sinh Γl], [sinh Γl / Z_c, cosh Γl]]·[p₂; U₂], Z_c = ρc/(πr²) | Wachter Gl. 3.32–3.37 (Übertragungsmatrix Zylinder); Kern `Rohr.cpp:41-43` | nein (Textebene) | verlustfreier Zylinder 1 m, r 5 mm, ideal offen: 85,77 / 257,44 / 429,00 / 600,56 / 772,23 Hz gegen (2n−1)c/4L = 85,80 / 257,40 / 429,00 / 600,60 / 772,20 Hz (Abweichung ≤ 0,7 Cent = Rasterauflösung der Spitzensuche, 0,25 Hz) `[eigene Rechnung]` |
| G2 | Γ = jk[1 + (1 − j)·b₁/r_v], b₁ = (1 + ψ)/√2, ψ = (γ − 1)/ν, ν = √Pr, r_v = a√(ωρ/η) — **gleich der Formel des Kerns** | Wachter Gl. 3.13, 3.17, 3.20 (Reihe nach Keefe 1984, JASA 75(1) 58–62), erste Ordnung | ja (PDF-S. 32–33) | ν = 0,8410 (Wachter, 300 K) → ψ = 0,4776, b₁ = 1,0447 `[eigene Rechnung]`; Kern: (1 + 0,4/√0,71)/√2 = 1,0428 |
| G3 | Γ = (ω/c)[(b₁/r_v + b₂/r_v² + b₃/r_v³) + j(1 + b₁/r_v − b₃/r_v³)]; Z_c = (ρc/S)[(1 + a₁/r_v − a₃/r_v³) − j(a₁/r_v + a₂/r_v² + a₃/r_v³)]; a₁…b₃ Gl. 3.14–3.19 | Wachter Gl. 3.12–3.20 („gültig bis 6 kHz“) | ja | a₁ = 0,369, a₂ = 1,149, b₂ = 1,080 `[eigene Rechnung]`. **Vorzeichen** des Imaginärteils von Z_c: Wachter druckt „+ j“; für e^{+jωt} hier „− j“ gesetzt `[eigene Einschätzung: Keefe rechnet mit e^{−iωt}]`. Wirkung auf die Spitzen ≤ 1,2 Cent (4.2) |
| G4 | Kegelstumpf exakt: p = (A e^{−Γx} + B e^{Γx})/x (x ab Kegelspitze), U = −S/(ρcΓ)·∂p/∂x; Matrix = M(x₁)·M(x₂)⁻¹ | `[eigene Herleitung]` aus der Webster-Gleichung für S ∝ x²; keine Quelle gelesen | — | gegen Treppe (G1, 1 mm) am Kegel 5→50 mm, 1 m: Abstand ≤ 0,002 Cent (Spitzen 1–5); am Horn ≤ 0,01 Cent |
| G5 | Z_r = Z_c[(ka)²/4 + j·0,6133·ka] (ohne Flansch, ka ≪ 1) | Hendrie 2007 Gl. 2.91 (dort 0,6 statt 0,6133), S. 37; 0,6133 = η bei Silva et al. 2009 Gl. 11 / Tab. 1 | ja (Hendrie PDF-S. 59; Silva S. 12) | Hendrie: „replace the unflanged pipe of length ℒ by an ideally open pipe of length ℒ + 0.6r“ `[wörtlich]`; mit 0,6133: Rohr 1 m, r 2 cm → 1,0123 m `[eigene Rechnung]` |
| G6 | Levine–Schwinger, Näherung Silva et al. 2009 Tab. 1 (ohne Flansch): \|R\| = (1 + a₁(ka)²)/(1 + (β + a₁)(ka)² + a₂(ka)⁴ + a₃(ka)⁶), L/a = η(1 + b₁(ka)²)/(1 + b₂(ka)² + b₃(ka)⁴ + b₄(ka)⁶); β = 1/2, η = 0,6133, a₁ = 0,800, a₂ = 0,266, a₃ = 0,0263, b₁ = 0,0599, b₂ = 0,238, b₃ = −0,0153, b₄ = 0,00150; R = −\|R\|e^{−2jkL} (für e^{+jωt}: Tab. 1 ist „exp(−jωt)“, „complex conjugation of the expressions is needed“), Z_r = Z_c(1 + R)/(1 − R) (Gl. 2, 3) | Silva, Guillemain, Kergomard, Mallaroni, Norris: „Approximation formulae for the acoustic radiation impedance of a cylindrical pipe“, JSV 322 (2009) 255–263; arXiv 0811.3625 | Tab. 1 ja; Gl. 2/3 Textebene | Gültigkeit: „approximation errors … less than 2% for ka < 3“ (S. 10, Textebene). Grenzfall ka → 0 gibt G5 zurück `[eigene Rechnung]`; bei ka = 1: \|R\| = 0,694, L/a = 0,531 `[eigene Rechnung]`. Horn-Becher r = 0,1498 m: ka = 1 bei 365 Hz, ka = 3 bei 1094 Hz |
| G7 | Kessel parallel: Z = 1/(jωC + 1/Z_B), C = V/(ρc²) | Kern `Blech.cpp:267-274`; **keine Quelle gelesen** | — | V = 3,007 cm³ (Mundstück Abb. 52, 0–31,5 mm) `[eigene Rechnung]` |
| G8 | Bessel-Horn r(x) = b·(x₀ − x)^(−γ) | Hendrie Gl. 2.106 „a = a0(−x)−m“ (S. 46); Kern `Vorgaben.cpp:29-34` | nein (Textebene) | Anpassung an den Horn-Becher (3.1): γ = 0,881 |
| G9 | Lippe: d²h/dt² + (ω_ℓ/Q_ℓ)dh/dt + ω_ℓ²(h − H(ω_ℓ)) = (p_m − p)/µ; u = w·h⁺·sgn(p_m − p)·√(2\|p_m − p\|/ρ); H(ω_ℓ) = 2πk/ω_ℓ | Mattéoli et al. Gl. 1, 2 (S. 2–3) | ja | gleiche Form wie `Blech.hpp:9-11` (außer H fest statt H(ω_ℓ)); k = 7,4·10⁻² m·Hz → H = 1,14 mm bei f_ℓ = 65 Hz `[eigene Rechnung]` |
| G10 | Nennlänge L_nom^ec = c₀/(2F_nom) | Mattéoli Tab. 1, Unterschrift (S. 3) | ja | F-Horn F_nom = F1 = 43,65 Hz → 3,931 m; B-Seite 58,27 Hz → 2,945 m `[eigene Rechnung]` |
| G11 | Ventilzug ideal: ΔL = L_eff·(2^{k/12} − 1) | Kern `Vorgaben.cpp:177-180`; **keine Quelle** | — | F: L_eff = 3,975 m → 0,236 / 0,487 / 0,752 m (1/2/3 Halbtöne); B: L_eff = 2,927 m → 0,174 / 0,358 / 0,554 m `[eigene Rechnung]` |

---

## 3. Bohrung je Instrument (in der Form des Kerns)

### 3.1 Waldhorn F-Seite — E. Schmid Doppelhorn Nr. 777 (Wachter 2008)

**Quelle:** Wachter, S.: „Untersuchungen zum Stopfen bei Hörnern“, Diplomarbeit, IWK mdw Wien / IEM Graz 2008.
Instrument „Firma Engelbert Schmid F/B Doppelhorn Seriennummer 777“ (S. 54, Fußnote 91, Textebene). **Abb. 88
„Mensurverlauf F-Horn mit Mundstück, Ventilstellung 0“ (S. 82, PDF-S. 87)**, am Seitenbild gesehen. Entstehung laut
S. 81 `[wörtlich, Textebene]`: „Der Mensurverlauf wurde mittels Schnur für die Messung der Längsachse und mittels
Messschieber zur Messung des Durchmessers ermittelt“; danach mit dem „Optimierer“ (W. Kausel, Rosenbrock-Verfahren)
an die gemessene Impedanz angeglichen. S. 82: „Die berechneten Resonanzfrequenzen sind im Vergleich zum gemessenen
Impedanzverlauf jeweils tiefer … Bei dieser Messung ist die unterschiedliche Position des Stimmzuges ausschlaggebend“
`[wörtlich, Textebene]`. Ob Abb. 88 die Mensur vor oder nach der Angleichung zeigt, sagt die Arbeit nicht eindeutig.

**Wie abgelesen** (`digitalisiere_abb88.py`, Anhang): Seite mit `pdftoppm -r 600` als Ausschnitt; Kalibrierung über
die Gitterlinien (42 senkrechte, 0,1 m; 36 waagrechte, 0,01 m; Restfehler der Geraden 1,3 px bzw. 2,9 px); die
Hochachse ist der **Radius** (Becherrand bei ±0,15 m = Ø 300 mm, Hauptrohr ±5,7 mm); r = halber Abstand von oberer und
unterer Kontur. Strichmarken an Abschnittsgrenzen herausgefiltert (waagrechte Stetigkeit über ±3 Spalten). Becher ab
3,70 m zeilenweise (steil). **Auflösung: 1 px = 1,22 mm in x, 0,17 mm in r** → Radiusstufen von 0,17–0,26 mm im
Hauptrohr (sichtbar als Sprünge Ø 11,39 / 11,90 / 12,24 mm). Marke für alles aus Abb. 88: `[abgelesen]`, Fehler
±1,2 mm (x), ±0,2 mm (r) gegen die Zeichnung; gegen das Instrument unbekannt (Wachter: „Messfehler mussten in Kauf
genommen werden“).

**Mundstück H. Ebner** (Abb. 52, S. 55, PDF-S. 60) `[wörtlich, am Bild]` — trichterförmig, ohne abgesetzten Kessel:

| Länge mm | 0 | 5 | 10 | 15 | 20 | 25 | 30 | 30,5 | 31,5 | 35,9 | 45,9 | 50,9 | 55,9 | 60,9 | 65,9 | 67,38 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Ø mm | 17,5 | 14,77 | 12,77 | 10,3 | 8,4 | 6,23 | 5,2 | 4,75 | 4,65 | 4,7 | 4,73 | 4,73 | 5,04 | 5,5 | 5,8 | 6,3 |

Kesseldurchmesser 17,5 mm (Rand innen), **Seele Ø 4,65 mm bei 31,5 mm**, Rückbohrung 31,5 → 67,38 mm auf Ø 6,3 mm;
Volumen 0–31,5 mm **3,007 cm³**, ganzes Mundstück 3,716 cm³ `[eigene Rechnung, Kegelstümpfe]`. Abb. 53/S. 54: es ist
das Mundstück der Messung am Schmid-Horn.

**Bohrung (verdichtet; vollständig 296 Punkte in `bohrung_F_punkte.txt`, als Kegelstümpfe in
`bohrung_F_kernform.txt`):**

| x [m] ab Mundstückrand | Ø [mm] | Segmentart | Herkunft |
|---|---|---|---|
| 0 → 0,0315 | 17,5 → 4,65 | Kegelstümpfe (Tabelle oben) | Abb. 52 `[wörtlich]` |
| 0,0315 → 0,0674 | 4,65 → 6,3 | Kegelstümpfe | Abb. 52 `[wörtlich]` |
| 0,070 → 0,17 | 7,48 | (Mundrohr, Anfang) | Abb. 88 `[abgelesen ±0,4 mm Ø]` |
| 0,17 → 0,26 | 7,48 → 10,37 | Mundrohr, konisch (in Stufen abgelesen) | Abb. 88 |
| 0,26 → 0,40 | 10,37 | zylindrisch | Abb. 88 |
| 0,40 → 2,72 | 11,39 (Median; Einzelwerte 11,05 … 12,24) | Hauptrohr inkl. Ventile in Stellung 0 | Abb. 88 |
| 2,72 → 3,00 | 12,24 → 16,49 | Beginn Schallstück | Abb. 88 |
| 3,00 / 3,20 / 3,40 / 3,50 / 3,60 / 3,70 | 16,5 / 21,1 / 27,7 / 33,5 / 43,5 / 60,9 | Schallstück | Abb. 88 |
| 3,73 | 67,8 → 78,6 (Sprung in der Zeichnung) | — | Abb. 88 (ob Absatz am abschraubbaren Becher oder Zeichenfehler: offen) |
| 3,76 / 3,78 / 3,80 / 3,82 / 3,84 / 3,862 | 88,5 / 102 / 125 / 166 / 207 / 299,6 | Becher | Abb. 88 (zeilenweise) |

**Becher als Bessel-Form** (G8, Anpassung ab 3,0 m, `[eigene Rechnung]`): **r = 0,0074577·(3,8914 − x)^(−0,8806)** [m];
rms-Fehler 3,9 % in r, größter 13,2 % (am Rand). Die Anpassung überzeichnet den Rand (r(3,862) = 166,5 mm statt 149,8).
In der Form des Kerns (`besselHorn(L, r0, r1, ε)`, das d1 aus den Endradien bildet) wären das L = 0,862 m,
r0 = 8,25 mm, r1 = 149,8 mm, ε = 0,881 — mit **anderen** Zwischenwerten als die Anpassung, weil der Kern die Kurve
durch beide Endpunkte zwingt (nicht nachgerechnet).

**Länge gegen Sollänge:** Bohrung 3,862 m; Nennlänge (G10) 3,931 m. Aus der gemessenen 4. Spitze 173,9 Hz:
L_eff = 2c/f₄ = 3,947 m. Gemessene Spitzen 2–15 gegen die F-Reihe n·43,65 Hz (a′ = 440): −40, +11, −7, +17, +3, 0,
+14, +20, +3, +9, +7, +15, +30, +18 Cent `[eigene Rechnung aus Tab. 85]` — Wachter nennt als Beispiel ein klingendes a1 mit 443 Hz (S. 57); auf a′ = 443 bezogen wären es
das sind +12 Cent.

### 3.2 Waldhorn B-Seite — nur als Annahme

Keine Mensur in einer Quelle. Wachter S. 59 `[wörtlich, Textebene]`: „Die Rohrlänge des B Hornes ist um cirka 1 m
kürzer als die des F Hornes. Das Doppelhorn kann mit einem Daumenventil jeweils zwischen F und B Seite umgeschaltet
werden.“ **Annahme:** F-Bohrung, das Hauptrohr ab 1,0 m um ΔL gekürzt, ΔL so gewählt, dass die mittlere Abweichung
der Spitzen 2–9 gegen Abb. 58 (rot, digitalisiert) null wird: **ΔL = 1,050 m → 2,812 m** `[eigene Rechnung]`. Dass
beim Doppelhorn Mundrohr und Schallstück gemeinsam sind und nur der Mittelteil umgeschaltet wird, ist
`[eigene Einschätzung]`, nicht belegt.

### 3.3 Ventilwege, Umschaltventil

**Nicht gefunden** — weder Längen noch Durchmesser der Züge des Schmid-Horns. Wachter zeigt gemessene Impedanzen
aller Ventilkombinationen V0 … V123 nur als Kurven (Anhang 11.1, S. 133–138, Textebene). Als Ersatz die idealen
Längen nach G11 (Tabelle in Abschnitt 2), Radius = Hauptrohr (Annahme). Ort im Instrument: unbekannt; gerechnet bei
x = 1,5 m.

### 3.4 Tuba und Euphonium

**Nicht herleitbar.** Stufe 1: Tuba ohne eine einzige Zahl; Euphonium (Mattéoli Abb. 8 nach Campbell et al. 2021) nur
bis ≈ 2,2 m, Becher abgeschnitten, keine Züge. Ein Modell daraus wäre zu mehr als der Hälfte Annahme; gerechnet wurde
es nicht. Belegt sind nur die Nennlängen (Mattéoli Tab. 1, `[wörtlich, am Bild]`): B♭-Kontrabasstuba „18-ft B♭“
(Besson BE994), C-Kontrabasstuba „16-ft C“, E♭-Basstuba „13-ft E♭“, F-Basstuba „12-ft F“ (Melton 2250),
B♭-Euphonium „9-ft B♭“ (Besson BE967) — und die gemessene Impedanz der B♭-Kontrabasstuba nur als kleine Kurve
(Abb. 3a, S. 5, nicht ablesbar bei 130 dpi).

---

## 4. Gegenrechnung der Eingangsimpedanz

Skript `horn_rechnung.py` und seine vollständige Ausgabe stehen im Anhang. Luft wie der Kern (20 °C). Spitzen auf
einem Raster von 0,25 Hz, Parabel in log|Z|; Q aus der −3-dB-Breite; Höhe in MΩ (= 10⁶ Pa·s/m³, Wachters Einheit).

### 4.1 Prüffälle

- Verlustfreier Zylinder, offen/geschlossen (Auftrag): siehe G1 — ≤ 0,7 Cent.
- Kegel exakt (G4) gegen Treppe (G1, Kern): ≤ 0,002 Cent.

### 4.2 F-Seite gegen die Messung am selben Instrument

Messwerte: Frequenzen Tab. 85 (S. 78, „offen“, Ventilstellung 0), `[wörtlich, am Bild]`; Höhen aus Abb. 89 (S. 82,
grün, „gemessene Impedanz“, „Horn ohne rechte Hand“) `[abgelesen]`, Kalibrierung an den Achsenteilungen, Ablesung
±1 MΩ und ±2 Hz. Welche Handhaltung Tab. 85 hat, sagt die Tabelle nicht (Kap. 5 misst mit dem Stopfdämpfer;
die Frequenzen der grünen Kurve liegen 1–3 Hz über Tab. 85) — **offen**.

**Kernmodell** (Profil des Mundstücks am Eingang, G1 + G2 + G5):

| n | f gerechnet [Hz] | f Tab. 85 [Hz] | Cent | \|Z\| gerechnet [MΩ] | \|Z\| Abb. 89 [MΩ] | dB | Q gerechnet |
|---|---|---|---|---|---|---|---|
| 2 | 83,71 | 85,3 | −32,5 | 36,7 | 26,9 | +2,7 | 17,4 |
| 3 | 129,26 | 131,8 | −33,7 | 31,8 | 32,3 | −0,1 | 23,0 |
| 4 | 172,69 | 173,9 | −12,1 | 37,9 | 42,5 | −1,0 | 26,9 |
| 5 | 213,97 | 220,4 | −51,2 | 53,6 | 58,6 | −0,8 | 28,8 |
| 6 | 257,50 | 262,4 | −32,6 | 58,3 | 56,0 | +0,4 | 31,7 |
| 7 | 301,15 | 305,6 | −25,4 | 59,5 | 57,8 | +0,2 | 35,1 |
| 8 | 340,40 | 352,1 | −58,5 | 56,6 | 52,1 | +0,7 | 37,1 |
| 9 | 382,20 | 397,5 | −68,0 | 47,6 | 47,9 | −0,0 | 37,9 |
| 10 | 427,14 | 437,4 | −41,1 | 40,7 | 46,1 | −1,1 | 39,1 |
| 11 | 466,82 | 482,8 | −58,3 | 39,7 | 39,0 | +0,2 | 37,3 |
| 12 | 509,65 | 526,0 | −54,7 | 32,1 | 31,6 | +0,2 | 34,5 |
| 13 | 554,92 | 572,5 | −54,0 | 19,9 | 22,7 | −1,1 | 30,8 |
| 14 | 598,35 | 621,8 | −66,6 | 20,3 | 19,9 | +0,2 | 29,8 |
| 15 | 641,35 | 661,8 | −54,3 | 14,7 | 16,6 | −1,1 | 23,9 |

Mittel **−45,9 Cent** (Streuung 15,9), Höhen Mittel **0,0 dB**. Spitze 1 (Pedal) 28,3 Hz gerechnet; Wachter nennt
„Pedalton (25 Hz)“ (S. 63, Textebene). Gemessene Q-Werte gibt die Quelle nicht als Zahl.

**Ursachen der Lage** (Zahlen aus 4.3): Stimmzug (Wachter selbst), Temperatur der Messung unbekannt (+5 °C →
+14,8 Cent), Ablesung des Hauptrohrs (bis 2,5 Cent/%). Hauptrohr auf den Median geglättet: Mittel −44,7, Streuung 17,4
— die Streuung kommt also **nicht** aus den Ablesestufen. **Hauptrohr 0,10 m kürzer (3,762 m): Mittel +0,5 Cent,
Streuung 17,7**; je Spitze +21, +39, +6, 0, +26, +8, −17, −15, −3, −16, −4, −7, −26, −6 Cent.

**Spielbarkeit:** Messung und Rechnung zeigen dasselbe Bild — Spitzen 2–4 schwach (27–43 MΩ), 5–9 am stärksten
(48–60 MΩ), ab der 13. unter 23 MΩ; gerechnet fallen Q und Höhe ab der 16. Spitze (690 Hz) ab, ab der 17. ist keine
−3-dB-Breite mehr bestimmbar. Wachter Kap. 6.3: „Ab 700 Hz wird der Großteil der Energie vom …“ (Satz bricht in der Textebene um), „endet
ungefähr bei 700 Hz“ (Textebene, gekürzt, Seite nicht am Bild geprüft). **Die Reihe bricht bei ≈ 700 Hz, 16./17. Naturton.**

### 4.3 Modellvarianten am selben Horn (Abstand zum Kernmodell)

| Variante | Lage Spitzen 2–15 [Cent] | Höhen [dB] | Q |
|---|---|---|---|
| Kegel exakt (G4) statt Treppe | ≤ 0,01 | 0,0 | ±0,1 |
| Abstrahlung Levine–Schwinger (G6) statt G5 | 0 … +1,6 (wächst mit n) | 0 … +0,8 (Spitzen 12–15) | bis +5,9 (Spitze 15) |
| Keefe 3. Ordnung (G3) statt 1. Ordnung (beide mit G6) | +1,2 (Spitze 2), sonst ≤ 0,1 | −0,15 … +0,15 | −0,7 … 0 |
| Mundstück als Volumen (G7, V = 3,007 cm³) + Seele statt Profil | +1,2 … +5,6 | 0 … −0,8 | +0,3 … +1,4 |
| Becher als Bessel-Anpassung statt Punktliste | −2,3 … +9,7 (Spitzen 2–12) | ≤ 0,1 | — |
| Hauptrohr geglättet (Median Ø 11,39 mm) | −24 … +30 je Spitze, Mittel +1,2 | — | — |

### 4.4 Ventilzüge: Ort und Radius

Ideale Längen (G11) eingefügt (a) **im Hauptrohr** bei 1,5 m mit dessen Radius (5,95 mm), (b) **an der Seele**
(r = 2,33 mm) — das ist, was die heutige Kernform mit `ventilM` im Eingangszylinder täte. Verschiebung der Spitzen
2–12 in Cent; Soll −100·k:

| Zug | Soll | (a) Hauptrohr | (b) an der Seele |
|---|---|---|---|
| +1 HT (0,236 m) | −100 | −122, −104, −100, −100, −104, −102, −90, −103, −103, −91, −104 | −44, −60, −134, −210, −195, −167, −125, −115, −115, −94, −88 |
| +2 HT (0,487 m) | −200 | −246, −208, −203, −198, −207, −205, −184, −197, −210, −189, −196 | −121, −280, −374, −299, −253, −221, −184, −199, −225, −217, −219 |
| +3 HT (0,752 m) | −300 | −371, −312, −308, −296, −309, −309, −285, −288, −312, −294, −290 | −299, −524, −431, −335, −315, −338, −346, −324, −318, −296, −289 |

### 4.5 B-Seite (Annahme 3.2) gegen Abb. 58 rot

Abb. 58 (S. 59) zeigt „F Horn (blau), B Horn (rot)“, Legende „Schmidt mit Bell mit Hand V0“ — also **mit Hand**;
digitalisiert (`digitalisiere_kurve.py`, ±2 Hz, ±1 MΩ). Rechnung **ohne** Hand.

| n | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|---|---|---|---|---|---|---|---|---|---|---|
| f gerechnet [Hz] | 119,5 | 176,2 | 234,5 | 293,5 | 346,4 | 405,8 | 460,5 | 519,8 | 578,6 | 640,7 |
| f Abb. 58 [Hz] | 122,4 | 178,3 | 234,6 | 290,5 | 346,8 | 403,0 | 456,7 | 512,9 | 568,8 | 627,4 |
| Cent | −41 | −20 | −1 | +18 | −2 | +12 | +14 | +23 | +30 | +36 |
| dB | +0,4 | −0,8 | −0,2 | +1,5 | −0,4 | +0,2 | −1,1 | 0,0 | −1,3 | −1,7 |

Gegen die B-Reihe n·58,27 Hz: +44, +14, +10, +13, −16, −9, −21, −15, −12, −1 Cent.

Zum Vergleich F-Seite mit Hand (Abb. 58 blau, digitalisiert): 88,2 / 135,4 / 176,0 / 221,7 / 265,0 / 306,1 / 349,0 /
392,8 / 436,5 Hz, Höhen 30,9 / 33,6 / 43,1 / 60,0 / 57,6 / 58,8 / 51,7 / 44,3 / 43,8 MΩ.

---

## 5. Empfindlichkeit (Kernmodell, ±1 %, zentrale Differenz)

Cent je Prozent (oben) und dB je Prozent (unten), Spitzen 2–12:

| Maß (wie verändert) | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Mundrohr-Länge (0,070–0,31 m gestreckt) | −0,9 | −0,8 | −0,9 | −1,4 | −1,4 | −1,4 | −1,3 | −1,1 | −1,1 | −1,2 | −1,1 |
|  dB/% | +0,01 | +0,03 | +0,05 | +0,04 | +0,02 | 0,00 | −0,04 | −0,05 | −0,01 | −0,05 | −0,08 |
| Mundrohr-Konus (Eingangsradius, zum Ende auslaufend) | −0,6 | −0,5 | −0,4 | −0,4 | 0,0 | +0,4 | +0,6 | +0,6 | +0,5 | +0,5 | +0,2 |
|  dB/% | −0,02 | −0,04 | −0,07 | −0,11 | −0,15 | −0,16 | −0,12 | −0,06 | 0,00 | +0,07 | +0,12 |
| Becherradius (am Ende, ab 3,0 m eingeblendet) | +0,9 | +1,6 | +1,4 | +0,5 | +0,3 | +0,3 | +0,3 | +0,3 | +0,2 | +0,1 | +0,1 |
|  dB/% | +0,01 | +0,01 | +0,01 | 0,00 | 0,00 | 0,00 | −0,01 | −0,01 | −0,02 | −0,02 | −0,03 |
| Bessel-Exponent γ (Endradien fest) | +0,9 | +1,6 | +1,5 | +0,8 | +0,5 | +0,4 | +0,4 | +0,2 | +0,1 | +0,1 | 0,0 |
|  dB/% | +0,01 | +0,01 | +0,01 | 0,00 | 0,00 | 0,00 | −0,01 | −0,02 | −0,03 | −0,04 | −0,04 |
| Mundstück-Kesselvolumen | −0,2 | −0,2 | −0,2 | −0,4 | −0,5 | −0,5 | −0,5 | −0,4 | −0,4 | −0,4 | −0,3 |
|  dB/% | +0,01 | +0,01 | +0,02 | +0,02 | +0,01 | 0,00 | −0,03 | −0,05 | −0,06 | −0,09 | −0,11 |
| Radius Hauptrohr 0,4–2,8 m | −0,3 | +0,6 | +2,5 | +0,4 | −1,2 | −0,5 | +0,4 | 0,0 | 0,0 | +0,8 | +0,4 |
|  dB/% | −0,04 | +0,08 | +0,13 | +0,16 | +0,16 | +0,07 | −0,07 | −0,11 | −0,01 | +0,09 | +0,10 |

**Unsicherheit der Maße → Unsicherheit in Cent** `[eigene Rechnung]`:

| Maß | Unsicherheit (Herkunft) | in % | → Cent (größter Betrag, Spitzen 2–12) | → dB |
|---|---|---|---|---|
| Radius Hauptrohr | ±0,17 mm (1 px) | ±3,0 % | ±7,5 (Spitze 4), sonst ≤ ±3,6 | ±0,5 |
| Mundrohr-Eingangsradius | ±0,17 mm | ±4,5 % | ±2,7 | ±0,7 |
| Mundrohr-Länge | ±1,2 mm (1 px) auf 0,24 m | ±0,5 % | ±0,7 | ±0,04 |
| Becherradius | ±0,17 mm auf 150 mm | ±0,1 % | ±0,2 | 0 |
| Becherkontur | Bessel statt Punkte | — | bis 9,7 | ±0,1 |
| Kesselvolumen | Tabelle auf 0,01 mm | ≈ 0 | 0 | 0 |
| Temperatur der Messung | unbekannt, ±3 °C | — | ±8,9 | — |
| Stimmzug (Wachter S. 82) | unbekannt | — | ≈ −46 (0,10 m) | — |

---

## 6. Lippenparameter

| Größe | Tuba (Mattéoli Tab. 2) | Horn | Euphonium | Kern heute (Posaune, Trompete, Alphorn) |
|---|---|---|---|---|
| Breite w | 1,2·10⁻² m `[wörtlich, am Bild]` | nicht gefunden | wie Tuba (Tab. 2 gilt „throughout the article“, alle sieben Instrumente) | 0,01 m (`Vorgaben.cpp:93`, Willemsen, „nur Bericht“) |
| Güte Q_ℓ | 7,0 `[wörtlich]` | nicht gefunden | wie Tuba | 5 (Petiot 2023 Trompete, übertragen) |
| 1/µ | 1,1·10⁻¹ m²/kg → **µ = 9,09 kg/m²** `[wörtlich / eigene Rechnung]` | nicht gefunden | wie Tuba | µ = 1,3 kg/m² |
| Ruhespalt H | H(ω_ℓ) = 2πk/ω_ℓ = k/f_ℓ, **k = 7,4·10⁻² m·Hz** `[wörtlich]` → 1,90 mm bei 39 Hz, 1,14 mm bei 65 Hz | nicht gefunden | wie Tuba | H0 = 2,9·10⁻⁴ m fest |
| ρ, c₀ | 1,2 kg/m³; 3,4·10² m/s `[wörtlich]` | — | — | 1,2041; 343,2 |
| f_ℓ zum Ton | „optimal threshold frequency of 39 Hz … of the second regime is 65 Hz“, zweites Regime B♭1 (58 Hz) → f_ℓ/f ≈ 1,12 (S. 6, Textebene, **nicht am Bild**) | nicht gefunden | — | je Note gestimmt (`stimme`), Griffwahl bei f_L/f = 0,82 (`Blech.cpp:318`) |
| Blasdruck | Rechnung bis 2 kPa (C-Tuba) bzw. „more than 3 kPa“ (Basstuben) erwähnt (S. 14, Textebene) | nicht gefunden | — | 500 … 14 000 Pa |
| Steifigkeit | nur über ω_ℓ²·µ (G9) | — | — | — |
| Herkunft | „taken from [Mattéoli et al., 2021]“; Modell nach Fletcher 1993 | — | — | — |

Gültigkeitsbereich der Tuba-Werte: ein Satz für sieben Tuben/Saxhörner von 9 bis 18 ft, nicht je Spieler oder Ton
gemessen (Tab. 2: „used throughout the article“). Die Ruhespaltregel H = k/f_ℓ ist eine Modellannahme (S. 3:
„H is assumed to be proportional to the inverse of the lips’ resonance frequency“, `[wörtlich, am Bild]`).

---

## 7. Parametertabellen je Instrument

**Waldhorn F (Schmid 777):**

| Parameter | Zahl | Einheit | Quelle | Marke | Gültig |
|---|---|---|---|---|---|
| Bohrung | 296 Punkte / Kegelstümpfe | m (Radius) | Wachter Abb. 88 + Abb. 52 | `[abgelesen]` / `[wörtlich]` | Ventilstellung 0, ohne Hand |
| Länge | 3,862 | m | Abb. 88 | `[abgelesen ±1,2 mm]` | Stimmzug wie bei der Abmessung |
| Länge, angeglichen | 3,762 | m | Tab. 85 | `[eigene Rechnung]` | Mittel 0 Cent, 20 °C |
| Hauptrohr | Ø 11,39 (Median) | mm | Abb. 88 | `[abgelesen ±0,34 mm Ø]` | 0,40–2,75 m |
| Becher | Ø 299,6 | mm | Abb. 88 | `[abgelesen]` | — |
| Becher Bessel | b = 0,0074577, x₀ = 3,8914 m, γ = 0,8806 | m | Anpassung | `[eigene Rechnung]` | 3,0–3,862 m, rms 3,9 % |
| Kesselvolumen | 3,007 | cm³ | Abb. 52 | `[eigene Rechnung]` | 0–31,5 mm |
| Seele | Ø 4,65 bei 31,5 mm | mm | Abb. 52 | `[wörtlich]` | — |
| Spitzen 2–15 gemessen | 85,3 … 661,8 | Hz | Tab. 85 | `[wörtlich]` | „offen“, Ventil 0 |
| Höhen gemessen | 26,9 … 58,6 | MΩ | Abb. 89 grün | `[abgelesen ±1 MΩ]` | ohne Hand |
| Ventilzüge | 0,236 / 0,487 / 0,752 | m | G11 | `[Annahme]` | — |
| Lippe | — | — | nicht gefunden | — | — |

**Waldhorn B:** Länge 2,812 m `[Annahme, angepasst]`; Spitzen Abb. 58 rot `[abgelesen ±2 Hz]`; Ventilzüge 0,174 /
0,358 / 0,554 m `[Annahme]`.

**Tuba / Euphonium:** Lippe nach Abschnitt 6 `[wörtlich]`; Nennlängen nach 3.4 `[wörtlich]`; Bohrung: keine.

---

## 8. Annahmen, die bleiben (von keiner Quelle belegt)

1. Ob Abb. 88 die abgemessene oder die angeglichene Mensur ist; ihr Bezug zur Stimmzugstellung der Messung.
2. Die Handhaltung zu Tab. 85; die Temperatur aller Messungen.
3. Der Sprung Ø 67,8 → 78,6 mm bei 3,73 m.
4. Die ganze B-Seite (Länge, Ort der Kürzung); alle Ventilzüge (Länge, Radius, Ort, Bögen), das Umschaltventil.
5. Das Vorzeichen des Imaginärteils von Z_c in G3 (Konvention).
6. G4 und G7 ohne gelesene Quelle (G4 numerisch gegen G1 geprüft).
7. Die Hand im Becher (Wachter: verschiebt alle Spitzen „jeweils tiefer“, S. 57) — ohne Modell.
8. Lippenparameter des Horns; Übertragbarkeit der Tuba-Werte auf ein bestimmtes Instrument oder einen Spieler.
9. Tuba- und Euphonium-Bohrung insgesamt.

---

## 9. Optionen für LumiViz (ohne Empfehlung nach Aufwand)

Zahlen aus 4 und 5; „Lage“ = Spitzen 2–15 gegen Tab. 85.

**O1 — Punktliste (Abb. 88 + Mundstückprofil) als `rest`, Kessel aus dem Profil (Kernform), Zylinder kurz an der
Seele.** Daten: `bohrung_F_kernform.txt` (vorhanden). Lage −42 Cent im Mittel (Kernform-Variante) bzw. +0,5 nach
Längenausgleich 0,10 m; Streuung 15–18 Cent; Höhen ±1,9 dB (Spitze 2 +2,7); Q 17–40; Reihe bricht bei ≈ 700 Hz wie
gemessen. **Ventile:** nur mit der heutigen Einfügestelle → bis 110 / 174 / 224 Cent neben dem Soll (4.4 b); Schmettern liefe in einem
1,4-cm-Zylinder statt im Hauptrohr (nicht gerechnet).

**O2 — wie O1, aber der Zylinder des Kerns ist das Hauptrohr (Mundrohr davor als eigener Teil).** Daten wie O1 plus
eine Grenze Mundrohr/Hauptrohr (≈ 0,31 m). Braucht im Kern ein Stück Bohrung **vor** dem Zylinder (heute
`ganzeBohrung` setzt den Zylinder an den Anfang, `Blech.cpp:249`). Lage und Höhen wie O1 (lineare Rechnung gleich);
**Ventile** an richtiger Stelle: −122 … −90 Cent für 1 HT (Soll −100), Abstand zum Soll an Spitze 2: 22 / 46 / 71 Cent, an den Spitzen 3–12
≤ 16 Cent — das ist der Fehler eines echten Ventilsatzes, kein Modellfehler. Schmettern im
Hauptrohr (Radius 5,7 mm, Länge ≈ 2,4 m).

**O3 — Segmentmodell: Mundstück als Volumen, Hauptrohr als ein Zylinder (Ø 11,39 mm), Becher als Bessel-Horn
(γ = 0,881).** Daten: sieben Zahlen statt 296 Punkte. Gegen O1: Becher −2 … +10 Cent, ≤ 0,1 dB; Volumen statt
Profil +1 … +6 Cent, bis −0,8 dB; Hauptrohr geglättet −24 … +30 Cent je Spitze bei gleichem Mittel. Die
Endpunkt-Form von `besselHorn` weicht von der Anpassung ab (nicht nachgerechnet).

**O4 — Abstrahlung und Verluste genauer (unabhängig von O1–O3).** Levine–Schwinger (G6) statt G5: ≤ 1,6 Cent, bis
+0,8 dB und +6 in Q an den Spitzen 12–15; Keefe 3. Ordnung: ≤ 1,2 Cent, ≤ 0,15 dB. G5 ist beim Horn ab 365 Hz
außerhalb seines Bereichs (ka > 1), G6 gilt bis ka = 3 (1094 Hz).

Für alle Optionen gilt: die **Lage** hängt an Stimmzug und Temperatur (zusammen ±50 Cent), nicht an der
Modellwahl (≤ 10 Cent zwischen O1 und O3).

---

## 10. Was die LumiViz-Sitzung prüfen muss

1. `Blech.cpp:246-250`, `Blech.hpp:69`: Ventillängen werden im Eingangszylinder angehängt — beim Horn (und jeder
   Bohrung mit Mundrohr) bis zu 110 / 174 / 224 Cent neben dem Soll (4.4). Für die Trompete (Soares-Zylinder am Eingang mit r = 5 mm)
   prüfen, ob dort dasselbe gilt.
2. `Rohr.cpp:111-112`: Abstrahlung G5 wird beim Horn weit über ka = 1 genutzt; Wirkung hier klein (≤ 1,6 Cent), am
   Posaunenbecher (r = 0,108 m) eigens nachrechnen.
3. `Rohr.hpp:12-15`: „nur aus dem Gedaechtnis, offen“ — G2 ist jetzt belegt (Wachter Gl. 3.13/3.17, Keefe 1984),
   G5 durch Hendrie Gl. 2.91 und Silva Tab. 1 (η = 0,6133).
4. Abb. 88 an höherer Auflösung oder an Kausels Mensurliste nachmessen; den Sprung bei 3,73 m klären.
5. Handhaltung zu Tab. 85; Temperatur der BIAS-Messungen (Wachter Kap. 4.1, nicht gelesen).
6. Die digitalisierten Kurvenwerte (Abb. 58, 89) sind Rasterbilder mit ≈ 2 Hz je Pixel — vor Verwendung als Prüfwerte
   an den Anhangkurven (S. 133–138) gegenprüfen.
7. Lippe Tuba: µ = 9,1 kg/m² gegen 1,3 kg/m² heute — mit `stimme` prüfen, ob die Tuba damit über den Umfang anspricht.
8. `test_blech.cpp` wurde nicht gelesen.

---

## 11. Nicht gefunden / nicht herleitbar

| Lücke | Wo gesucht |
|---|---|
| Bohrung Tuba, Euphonium (vollständig) | Stufe-1-Bericht (alle Wege dort); Mattéoli (Tab. 1, Abb. 8) |
| B-Seite, Ventilzüge, Umschaltventil des Doppelhorns | Wachter Kap. 4, 6, Anhang (Textebene, Abbildungsliste) |
| Lippenparameter Horn | Wachter (Suche „Lippenspannung“, „Polster“: nur qualitativ), Hendrie, Mattéoli (nur Tuben) |
| Gemessene Q-Werte | Wachter (nur Kurven) |
| Gemessene Tuba-Spitzen als Zahl | Mattéoli Abb. 3a (Kurve, zu klein) |
| Quelle für G4 (Kegelmatrix) und G7 (Kessel parallel) | nicht gesucht (Herleitung bzw. Kern) |

---

## 12. Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Silva_2009_Abstrahlimpedanz_arXiv0811.3625.pdf` | https://arxiv.org/pdf/0811.3625 | 358 KB | arXiv-Lizenz nicht geprüft | G6; liegt als `webfetch-1794904979296-68p93d.pdf` im Sitzungsordner |
| `Norman_2013_AppendixB.zip` | https://era.ed.ac.uk/bitstreams/7bc115d6-d92a-480c-8aa1-41b26243fcd4/download | 60,59 KB | wie Stufe 1 | Naturhorn-Bohrungen (unverändert aus Stufe 1) |
| — (Bestellung/Anfrage) | Emmerich, B.: „Der Einfluss des Mundstückes auf die Intonation beim Wiener F Horn“, Diplomarbeit mdw 1999 (Wachter Fn. 93) | — | — | Herkunft Abb. 52, weitere Mundstücke |
| — (Anfrage IWK Wien) | Mensurliste Schmid 777 aus dem „Optimierer“ (Wachter Kap. 6) | — | — | Abb. 88 als Zahlen statt Ablesung |
| — (Kauf) | Campbell, Gilbert, Myers: *The Science of Brass Instruments*, Springer 2021 | Buch | © Verlag | Tuba-/Euphonium-Bohrungen (Stufe 1) |

---

## 13. Was der Auftrag verlangt hat und nicht geliefert ist

- Tuba und Euphonium: keine Bohrung, keine Gegenrechnung, keine Empfindlichkeit (nur Lippe, Nennlängen).
- Doppelhorn B-Seite und Ventilzüge: nur als Annahme.
- Lippenparameter Horn: nicht gefunden.
- Bessel-Parameter in der genauen Form von `besselHorn` (Endpunkte) nicht nachgerechnet.

Hilfsdateien: `bohrung_F_punkte.txt`, `bohrung_F_kernform.txt`, `abb88_roh*.txt`, `horn_rechnung_ausgabe.txt`,
`seiten/` (Seitenbilder), `wachter_*.txt`, `hendrie_199_202.txt`, `silva_2_3.txt` (Textauszüge).

---

## Anhang A — `horn_rechnung.py`

```python
"""Gegenrechnung der Eingangsimpedanz des Doppelhorns E. Schmid Nr. 777 (Wachter 2008) aus der digitalisierten
Mensur (Abb. 88) und dem Mundstueck H. Ebner (Abb. 52) — LumiViz_Research, Auftrag 2026-11-16_18.

Modelle (je Gleichung siehe Bericht, Abschnitt 2):
  Kern  : Treppe aus 1-mm-Zylindern (wie Rohr.cpp), Wandverluste Keefe 1. Ordnung (Gamma), Zc verlustfrei,
          Abstrahlung Zc[(ka)^2/4 + j 0,6133 ka] (wie Rohr.cpp)
  Kegel : Kegelstuempfe exakt in Kugelwellen (Basis e^(-+Gamma x)/x, Matrix numerisch aus der Basis), sonst wie Kern
  LS    : wie Kern, Abstrahlung Levine-Schwinger nach Silva et al. 2009 Tab. 1 (ohne Flansch)
  K3    : wie LS, Gamma und Zc nach Keefe 1984 bis 3. Ordnung (Wachter Gl. 3.12-3.20)
Aufruf: python horn_rechnung.py <ordner mit abb88_roh.txt>
"""
import sys
import os
import numpy as np

ORDNER = sys.argv[1]
RHO, C0, ETA, GAM, PR = 1.2041, 343.2, 1.8205e-5, 1.4, 0.71   # wie ic::Luft (Rohr.hpp:46-53)
F = np.arange(15.0, 1000.0, 0.25)
W = 2 * np.pi * F


def luft_bei(t_c):
    """Schallgeschwindigkeit ~ sqrt(T) (eigene Rechnung, ideales Gas); Bezug 20 Grad C = 343,2 m/s."""
    return C0 * np.sqrt((273.15 + t_c) / 293.15)


# ---------------------------------------------------------------- Geometrie
def mundstueck():
    """Wachter 2008 Abb. 52 (S. 55), Laenge/Durchmesser in mm, am Seitenbild gelesen."""
    l = [0, 5, 10, 15, 20, 25, 30, 30.5, 31.5, 35.9, 45.9, 50.9, 55.9, 60.9, 65.9, 67.38]
    d = [17.5, 14.77, 12.77, 10.3, 8.4, 6.23, 5.2, 4.75, 4.65, 4.7, 4.73, 4.73, 5.04, 5.5, 5.8, 6.3]
    return np.array(l) * 1e-3, np.array(d) * 0.5e-3


def mensur_abb88():
    """Digitalisierte Abb. 88: Spaltenspur bis 3,70 m (geglaettet), darueber Zeilenspur (steiler Becher)."""
    roh = np.loadtxt(os.path.join(ORDNER, "abb88_roh.txt"))
    x, r = roh[:, 0], roh[:, 1] * 1e-3
    bech = np.loadtxt(os.path.join(ORDNER, "abb88_roh_becher.txt"))
    xs = []
    rs = []
    # Mundrohr/Koerper: Median ueber 15 mm, dann alle 20 mm (im Mundrohr alle 10 mm)
    ab = 0.0700
    for p in np.concatenate([np.arange(ab, 0.40, 0.010), np.arange(0.40, 3.70, 0.020), [3.70]]):
        fen = np.abs(x - p) < 0.0075
        if fen.sum() >= 3:
            xs.append(p)
            rs.append(np.median(r[fen]))
    # Becher: Zeilenspur, x = Mittel aus oben/unten, nach x sortiert, r monoton
    bx = 0.5 * (bech[:, 0] + bech[:, 1])
    br = bech[:, 2]
    ordn = np.argsort(br)
    bx, br = bx[ordn], br[ordn]
    sel = br > rs[-1] + 2e-4
    letzt_x = xs[-1]
    for xb, rb in zip(bx[sel], br[sel]):
        if xb > letzt_x + 0.5e-3:
            xs.append(xb)
            rs.append(rb)
            letzt_x = xb
    return np.array(xs), np.array(rs)


def bohrung_F():
    lm, rm = mundstueck()
    xk, rk = mensur_abb88()
    x = np.concatenate([lm, xk])
    r = np.concatenate([rm, rk])
    return x, r


# ---------------------------------------------------------------- Akustik
def gamma_zc(r, w, c=C0, verl="k1"):
    """Wellenzahl Gamma und Wellenimpedanz Zc fuer Radius r (Skalar) und Kreisfrequenz w (Feld)."""
    k = w / c
    S = np.pi * r * r
    zc0 = RHO * c / S
    if verl == "keine":
        return 1j * k, zc0 * np.ones_like(w)
    rv = r * np.sqrt(w * RHO / ETA)
    if verl == "k1":   # Rohr.cpp:29-40, = Keefe 1. Ordnung (Wachter Gl. 3.13, 3.17)
        eps = (1 + (GAM - 1) / np.sqrt(PR)) / (np.sqrt(2) * rv)
        return 1j * k * (1 + (1 - 1j) * eps), zc0 * np.ones_like(w)
    # Keefe bis 3. Ordnung, Wachter Gl. 3.12-3.20; nu = sqrt(Pr)
    nu = np.sqrt(PR)
    psi = (GAM - 1) / nu
    a1 = (1 - psi) / np.sqrt(2)
    a2 = 1 - psi + psi / (2 * nu) + 1.5 * psi ** 2
    a3 = (7 / 8 - psi + psi / (2 * nu) + psi / (8 * nu ** 2) + 1.5 * psi ** 2 - 1.5 * psi ** 2 / nu - 2.5 * psi ** 3) / np.sqrt(2)
    b1 = (1 + psi) / np.sqrt(2)
    b2 = 1 + psi - psi / (2 * nu) - psi ** 2 / 2
    b3 = (7 / 8 + psi - psi / (2 * nu) - psi / (8 * nu ** 2) - psi ** 2 / 2 + psi ** 2 / (2 * nu) + psi ** 3 / 2) / np.sqrt(2)
    g = k * ((b1 / rv + b2 / rv ** 2 + b3 / rv ** 3) + 1j * (1 + b1 / rv - b3 / rv ** 3))
    # Vorzeichen des Imaginaerteils fuer e^{+j w t} (Wachter druckt +j; Keefe rechnet mit e^{-i w t}) — eigene Einschaetzung
    zc = zc0 * ((1 + a1 / rv - a3 / rv ** 3) - 1j * (a1 / rv + a2 / rv ** 2 + a3 / rv ** 3))
    return g, zc


def mal(A, B):
    return (A[0] * B[0] + A[1] * B[2], A[0] * B[1] + A[1] * B[3], A[2] * B[0] + A[3] * B[2], A[2] * B[1] + A[3] * B[3])


def zylinder(r, l, w, c, verl):
    g, zc = gamma_zc(r, w, c, verl)
    ch, sh = np.cosh(g * l), np.sinh(g * l)
    return (ch, zc * sh, sh / zc, ch)


def kegel(r1, r2, l, w, c, verl):
    """Kegelstumpf in Kugelwellen: p = e^(-+Gamma x)/x, U = -S/(rho c Gamma) dp/dx (eigene Herleitung aus der
    Webster-Gleichung fuer S ~ x^2; Gamma mit dem mittleren Radius)."""
    if abs(r2 - r1) < 1e-6:
        return zylinder(0.5 * (r1 + r2), l, w, c, verl)
    g, zc = gamma_zc(0.5 * (r1 + r2), w, c, verl)
    zc_m = zc * np.pi * (0.5 * (r1 + r2)) ** 2   # rho c (ggf. mit Keefe-Korrektur) je Flaeche
    x1 = r1 * l / (r2 - r1)
    x2 = x1 + l

    def basis(x, r):
        S = np.pi * r * r
        pp = np.exp(-g * (x - x1)) / x
        pm = np.exp(g * (x - x1)) / x
        up = -S / (zc_m * g) * (-g - 1 / x) * pp
        um = -S / (zc_m * g) * (g - 1 / x) * pm
        return pp, pm, up, um
    a = basis(x1, r1)
    b = basis(x2, r2)
    # T = M1 * inv(M2)
    det = b[0] * b[3] - b[1] * b[2]
    inv = (b[3] / det, -b[1] / det, -b[2] / det, b[0] / det)
    return mal((a[0], a[1], a[2], a[3]), inv)


def kette(x, r, w, c=C0, verl="k1", art="treppe"):
    E = (np.ones_like(w, dtype=complex), np.zeros_like(w, dtype=complex), np.zeros_like(w, dtype=complex), np.ones_like(w, dtype=complex))
    for i in range(len(x) - 1):
        l = x[i + 1] - x[i]
        if l <= 0:
            continue
        if art == "treppe":
            n = 1 if r[i] == r[i + 1] else max(1, int(np.ceil(l / 1e-3)))
            for s in range(n):
                t = (s + 0.5) / n
                E = mal(E, zylinder(r[i] + t * (r[i + 1] - r[i]), l / n, w, c, verl))
        else:
            E = mal(E, kegel(r[i], r[i + 1], l, w, c, verl))
    return E


def abstrahlung(a, w, c=C0, art="klein"):
    zc = RHO * c / (np.pi * a * a)
    ka = w / c * a
    if art == "klein":   # Rohr.cpp:111-112; Hendrie 2007 Gl. 2.91 (0,6 statt 0,6133)
        return zc * (ka ** 2 / 4 + 1j * 0.6133 * ka)
    if art == "ideal":
        return 0 * zc
    # Silva et al. 2009, Tab. 1, ohne Flansch; R = -|R| e^{-2jkL} fuer e^{+jwt} (konjugiert zu Gl. 2)
    beta, eta = 0.5, 0.6133
    a1, a2, a3 = 0.800, 0.266, 0.0263
    b1, b2, b3, b4 = 0.0599, 0.238, -0.0153, 0.00150
    betr = (1 + a1 * ka ** 2) / (1 + (beta + a1) * ka ** 2 + a2 * ka ** 4 + a3 * ka ** 6)
    L = a * eta * (1 + b1 * ka ** 2) / (1 + b2 * ka ** 2 + b3 * ka ** 4 + b4 * ka ** 6)
    R = -betr * np.exp(-2j * w / c * L)
    return zc * (1 + R) / (1 - R)


def eingang(x, r, w=W, c=C0, verl="k1", art="treppe", rad="klein", kessel=0.0):
    a, b, cc, d = kette(x, r, w, c, verl, art)
    zl = abstrahlung(r[-1], w, c, rad)
    z = (a * zl + b) / (cc * zl + d)
    if kessel > 0:
        z = 1 / (1j * w * kessel / (RHO * c * c) + 1 / z)
    return z


def spitzen(z, f=F):
    m = np.abs(z)
    out = []
    for i in range(1, len(m) - 1):
        if m[i] > m[i - 1] and m[i] >= m[i + 1]:
            # Parabel in log|Z|
            y0, y1, y2 = np.log(m[i - 1]), np.log(m[i]), np.log(m[i + 1])
            dd = 0.5 * (y0 - y2) / (y0 - 2 * y1 + y2)
            fp = f[i] + dd * (f[1] - f[0])
            hp = np.exp(y1 - 0.25 * (y0 - y2) * dd)
            # Halbwertsbreite (-3 dB) linear interpoliert
            ziel = hp / np.sqrt(2)
            j = i
            while j > 0 and m[j] > ziel and not (m[j - 1] > m[j]):
                j -= 1
            k = i
            while k < len(m) - 1 and m[k] > ziel and not (m[k + 1] > m[k]):
                k += 1
            q = np.nan
            if m[j] <= ziel and m[k] <= ziel:
                fl = f[j] + (ziel - m[j]) / (m[j + 1] - m[j]) * (f[1] - f[0])
                fr = f[k - 1] + (m[k - 1] - ziel) / (m[k - 1] - m[k]) * (f[1] - f[0])
                q = fp / (fr - fl)
            out.append((fp, hp, q))
    return out


def cent(a, b):
    return 1200 * np.log2(a / b)


# ---------------------------------------------------------------- Messwerte (Wachter 2008)
TAB85 = [85.3, 131.8, 173.9, 220.4, 262.4, 305.6, 352.1, 397.5, 437.4, 482.8, 526.0, 572.5, 621.8, 661.8]  # 2.-15. max
ABB89_GRUEN = [(87.4, 26.85), (135.6, 32.29), (176.1, 42.53), (220.2, 58.62), (265.7, 55.95), (305.3, 57.83),
               (349.4, 52.14), (394.9, 47.85), (439.5, 46.08), (485.5, 39.02), (526.0, 31.55), (576.9, 22.67),
               (617.0, 19.85), (663.4, 16.61)]   # (Hz, MOhm) digitalisiert
ABB58_ROT_B = [122.4, 178.3, 234.6, 290.5, 346.8, 403.0, 456.7, 512.9, 568.8, 627.4]   # B-Seite mit Hand, 2.-11.
ABB58_ROT_B_H = [44.3, 60.8, 78.5, 70.2, 72.5, 60.0, 54.1, 35.2, 26.2, 20.5]


def zuordnen(sp, ziel, ab=None, n0=2):
    """Nach Nummer: die Zielwerte sind die Spitzen n0, n0+1, ... (Spitze 1 = Pedal, erste Spitze ueber 15 Hz)."""
    sp = sorted(sp, key=lambda t: t[0])
    return [sp[n0 - 1 + i] for i in range(len(ziel))]


def tabelle(titel, sp, n0=1):
    print(titel)
    for i, (fp, hp, q) in enumerate(sp):
        print("  %2d  %7.2f Hz  %6.2f MOhm  %5.1f dB(1 MOhm)  Q %5.1f" % (i + n0, fp, hp / 1e6, 20 * np.log10(hp / 1e6), q))


def main():
    x, r = bohrung_F()
    L = x[-1]
    print("=== Bohrung F-Seite: %d Punkte, Laenge %.4f m, Eingang r %.2f mm, Becher r %.1f mm" % (len(x), L, r[0] * 1e3, r[-1] * 1e3))
    np.savetxt(os.path.join(ORDNER, "bohrung_F_kernform.txt"),
               np.column_stack([np.diff(x), r[:-1], r[1:]]), fmt="%.6f",
               header="Abschnitte wie ic::Abschnitt: laengeM radiusEinM radiusAusM (Mundstueck Abb.52 + Abb.88 digitalisiert)")
    np.savetxt(os.path.join(ORDNER, "bohrung_F_punkte.txt"), np.column_stack([x, r * 2e3]), fmt="%.5f",
               header="x_m d_mm (Durchmesser)")

    # --- 0. Pruefung am geschlossenen Fall
    print("\n=== Pruefung: verlustfreier Zylinder L = 1 m, r = 5 mm, Ende ideal offen (p = 0)")
    xz, rz = np.array([0.0, 1.0]), np.array([0.005, 0.005])
    for art in ("treppe", "kegel"):
        a, b, cc, d = kette(xz, rz, W, C0, "keine", art)
        z = b / d
        sp = spitzen(z)
        print("  %s:" % art, " ".join("%.3f(%+.4f c)" % (s[0], cent(s[0], (2 * n + 1) * C0 / 4)) for n, s in enumerate(sp[:5])))
    print("  Soll (2n-1)c/4L:", " ".join("%.3f" % ((2 * n - 1) * C0 / 4) for n in range(1, 6)))
    # Kegel gegen Treppe an einem echten Kegel (verlustfrei, 1 m, 5 -> 50 mm, ideal offen)
    xk, rk = np.array([0.0, 1.0]), np.array([0.005, 0.05])
    s1 = spitzen(eingang(xk, rk, verl="keine", art="treppe", rad="ideal"))
    s2 = spitzen(eingang(xk, rk, verl="keine", art="kegel", rad="ideal"))
    print("  Kegel 5->50 mm, 1 m: Treppe", ["%.2f" % s[0] for s in s1[:5]], " Kugelwelle", ["%.2f" % s[0] for s in s2[:5]])
    print("  Abstand Treppe-Kugelwelle [Cent]:", ["%.3f" % cent(a[0], b[0]) for a, b in zip(s1[:5], s2[:5])])

    # --- 1. Modelle der F-Seite
    modelle = {
        "Kern (Profil am Eingang)": dict(verl="k1", art="treppe", rad="klein"),
        "Kegel": dict(verl="k1", art="kegel", rad="klein"),
        "LS": dict(verl="k1", art="treppe", rad="ls"),
        "K3+LS": dict(verl="k3", art="treppe", rad="ls"),
    }
    erg = {}
    for name, kw in modelle.items():
        erg[name] = spitzen(eingang(x, r, **kw))
    # Kernform: Kessel als Volumen (Mundstueck 0 ... 31,5 mm), Bohrung ab der Seele
    lm, rm = mundstueck()
    ik = np.searchsorted(x, 0.0315)
    vk = sum(np.pi * (x[i + 1] - x[i]) / 3 * (r[i] ** 2 + r[i] * r[i + 1] + r[i + 1] ** 2) for i in range(ik))
    print("\nKesselvolumen Mundstueck 0-31,5 mm (Kegelstuempfe aus Abb. 52): %.3f cm^3; ganzes Mundstueck 0-67,38 mm: %.3f cm^3"
          % (vk * 1e6, sum(np.pi * (lm[i + 1] - lm[i]) / 3 * (rm[i] ** 2 + rm[i] * rm[i + 1] + rm[i + 1] ** 2) for i in range(len(lm) - 1)) * 1e6))
    erg["Kernform (Kessel V + Seele)"] = spitzen(eingang(x[ik:], r[ik:], verl="k1", art="treppe", rad="klein", kessel=vk))
    erg["Kernform + LS"] = spitzen(eingang(x[ik:], r[ik:], verl="k1", art="treppe", rad="ls", kessel=vk))

    for name, sp in erg.items():
        zu = zuordnen(sp, TAB85)
        dc = [cent(s[0], m) for s, m in zip(zu, TAB85)]
        dh = [20 * np.log10(s[1] / 1e6 / g[1]) for s, g in zip(zu, ABB89_GRUEN)]
        print("\n=== %s: gegen Wachter Tab. 85 (2.-15. Spitze)" % name)
        print("  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q")
        for n, (s, m, c_, g, h_) in enumerate(zip(zu, TAB85, dc, ABB89_GRUEN, dh), start=2):
            print("  %2d %7.2f %7.1f %+6.1f   %6.1f   %6.1f  %+5.1f  %5.1f" % (n, s[0], m, c_, s[1] / 1e6, g[1], h_, s[2]))
        print("  Mittel Cent %.1f, Streuung (Std) %.1f; Mittel dB %.1f" % (np.mean(dc), np.std(dc), np.mean(dh)))
        erste = [s for s in sp if s[0] < 60]
        if erste:
            print("  1. Spitze (Pedal): %.1f Hz (Wachter Text S. 63: Pedalton 25 Hz)" % erste[0][0])
    tabelle("\nKern: alle Spitzen bis 1000 Hz", erg["Kern (Profil am Eingang)"])

    # Hauptrohr geglaettet (die Strichmarken und die 0,17-mm-Stufen der Digitalisierung heraus)
    mg = (x > 0.40) & (x < 2.75)
    rg = r.copy()
    rg[mg] = np.median(r[mg])
    sg = spitzen(eingang(x, rg))
    dcg = [cent(s[0], m) for s, m in zip(zuordnen(sg, TAB85), TAB85)]
    print("\nHauptrohr 0,40-2,75 m auf den Median r = %.3f mm gesetzt: Cent gegen Tab. 85: %s; Mittel %.1f, Std %.1f"
          % (np.median(r[mg]) * 1e3, " ".join("%+.0f" % v for v in dcg), np.mean(dcg), np.std(dcg)))
    # Laengenausgleich: Zylinder im Hauptrohr bei 1,5 m, bis die mittlere Abweichung 0 ist
    i15 = np.searchsorted(x, 1.5)
    for dl in (-0.10, -0.08, -0.06, -0.04):
        xa = np.concatenate([x[:i15], x[i15:] + dl])
        if dl < 0:   # kuerzen: Punkte zwischen 1,5 und 1,5-dl entfernen
            keep = ~((x >= x[i15]) & (x < x[i15] - dl))
            xa = np.concatenate([x[:i15], x[i15:][keep[i15:]] + dl])
            ra = np.concatenate([rg[:i15], rg[i15:][keep[i15:]]])
        sa = spitzen(eingang(xa, ra))
        d_ = [cent(s[0], m) for s, m in zip(zuordnen(sa, TAB85), TAB85)]
        print("  geglaettet, Hauptrohr um %.2f m gekuerzt (Laenge %.3f m): Mittel %+.1f Cent, Std %.1f, je Spitze %s"
              % (-dl, xa[-1], np.mean(d_), np.std(d_), " ".join("%+.0f" % v for v in d_)))

    # Temperatur: welche Temperatur gleicht die mittlere Abweichung aus?
    zt = [spitzen(eingang(x, r, c=luft_bei(t))) for t in (20.0, 25.0)]
    d5 = np.mean([cent(a[0], b[0]) for a, b in zip(zuordnen(zt[1], TAB85), zuordnen(zt[0], TAB85))])
    print("\nTemperatur: +5 Grad C verschiebt die Spitzen 2-15 im Mittel um %+.1f Cent" % d5)

    # --- 2. Becher als Bessel-Horn (Anpassung) ---------------------------------
    print("\n=== Becher als Bessel-Horn r = b (x0 - x)^(-g), angepasst ab x = 3,0 m")
    sel = x >= 3.0
    bestes = None
    for x0 in np.linspace(x[-1] + 0.001, x[-1] + 0.15, 600):
        A = np.column_stack([np.ones(sel.sum()), -np.log(x0 - x[sel])])
        coef, res, *_ = np.linalg.lstsq(A, np.log(r[sel]), rcond=None)
        e = np.sum((A @ coef - np.log(r[sel])) ** 2)
        if bestes is None or e < bestes[0]:
            bestes = (e, x0, np.exp(coef[0]), coef[1])
    e, x0, bb, gg = bestes
    rb = bb * (x0 - x[sel]) ** (-gg)
    print("  x0 = %.4f m (Ende %.4f m), b = %.5g, gamma = %.4f, Restfehler rms %.1f %% in r, max %.1f %%"
          % (x0, x[-1], bb, gg, 100 * np.sqrt(e / sel.sum()), 100 * np.max(np.abs(rb / r[sel] - 1))))
    xb = x.copy()
    rb_all = r.copy()
    rb_all[sel] = rb
    sp_b = spitzen(eingang(xb, rb_all))
    print("  Bessel-Becher statt Punktliste, Spitzen 2-12 [Cent]:",
          ["%+.1f" % cent(a[0], b[0]) for a, b in zip(zuordnen(sp_b, TAB85[:11]), zuordnen(erg["Kern (Profil am Eingang)"], TAB85[:11]))])
    print("  ... Hoehen [dB]:", ["%+.1f" % (20 * np.log10(a[1] / b[1])) for a, b in zip(zuordnen(sp_b, TAB85[:11]), zuordnen(erg["Kern (Profil am Eingang)"], TAB85[:11]))])

    # --- 3. Empfindlichkeit ---------------------------------------------------
    print("\n=== Empfindlichkeit (Kernmodell, zentrale Differenz +-1 %), Spitzen 2-12")
    basis = zuordnen(erg["Kern (Profil am Eingang)"], TAB85[:11])
    i_lp0 = np.searchsorted(x, 0.0674)
    i_lp1 = np.searchsorted(x, 0.30)
    r_mz = np.median(r[(x > 0.5) & (x < 2.5)])
    print("  Mundrohr: x %.3f ... %.3f m, r %.2f -> %.2f mm; Hauptzylinder r (Median 0,5-2,5 m) %.2f mm"
          % (x[i_lp0], x[i_lp1], r[i_lp0] * 1e3, r[i_lp1] * 1e3, r_mz * 1e3))

    def var_mundrohr_laenge(s):
        xn = x.copy()
        d = (x[i_lp1] - x[i_lp0]) * s
        t = np.clip((x - x[i_lp0]) / (x[i_lp1] - x[i_lp0]), 0, 1)
        xn = x + d * t
        return xn, r

    def var_mundrohr_konus(s):   # Eingangsradius des Mundrohrs um s, zum Ende hin auf 0 auslaufend
        t = np.clip((x - x[i_lp0]) / (x[i_lp1] - x[i_lp0]), 0, 1)
        rn = r * np.where((x >= x[i_lp0]) & (x <= x[i_lp1]), 1 + s * (1 - t), 1.0)
        return x, rn

    def var_becherradius(s):
        t = np.clip((x - 3.0) / (x[-1] - 3.0), 0, 1)
        return x, r * (1 + s * t)

    def var_bessel(s):
        g2 = gg * (1 + s)
        # gleiche Endradien bei x = 3,0 m und am Ende: x0 und b neu
        xa, xe = x[sel][0], x[-1]
        ra, re = r[sel][0], r[-1]
        q = (re / ra) ** (1 / g2)   # (x0 - xa)/(x0 - xe) = q
        x0n = (q * xe - xa) / (q - 1)
        bn = re * (x0n - xe) ** g2
        rn = rb_all.copy()
        rn[sel] = bn * (x0n - x[sel]) ** (-g2)
        return x, rn

    def var_kessel(s):
        rn = r.copy()
        rn[:ik + 1] = r[:ik + 1] * np.sqrt(1 + s)
        return x, rn

    def var_zylinder(s):
        m = (x > 0.4) & (x < 2.8)
        return x, r * np.where(m, 1 + s, 1.0)

    varianten = [("Mundrohr-Laenge (0,067-0,30 m)", var_mundrohr_laenge, False),
                 ("Mundrohr-Konus (Eingangsradius)", var_mundrohr_konus, False),
                 ("Becherradius (Ende, ab 3,0 m eingeblendet)", var_becherradius, False),
                 ("Bessel-Exponent gamma", var_bessel, True),
                 ("Mundstueck-Kesselvolumen", var_kessel, False),
                 ("Radius Hauptrohr 0,4-2,8 m", var_zylinder, False)]
    for name, fn, bessel in varianten:
        res = []
        for s in (0.01, -0.01):
            xn, rn = fn(s)
            res.append(zuordnen(spitzen(eingang(xn, rn)), TAB85[:11]))
        bz = zuordnen(spitzen(eingang(xb, rb_all)), TAB85[:11]) if bessel else basis
        cc = [cent(a[0], b[0]) / 2 for a, b in zip(res[0], res[1])]
        db = [20 * np.log10(a[1] / b[1]) / 2 for a, b in zip(res[0], res[1])]
        print("  %-42s Cent/%%: %s" % (name, " ".join("%+5.1f" % v for v in cc)))
        print("  %-42s dB/%%:   %s" % ("", " ".join("%+5.2f" % v for v in db)))

    # --- 4. Ventilzuege: Ort und Radius ----------------------------------------
    print("\n=== Ventilzuege (Annahme: ideale Laengen L_eff (2^(k/12) - 1), L_eff aus der 4. Spitze)")
    f4 = basis[2][0]
    leff = 4 * C0 / (2 * f4)   # f_n = n c / (2 L_eff) fuer die Naturtonreihe (4. Spitze, n = 4)
    print("  L_eff = %.3f m (aus f4 = %.2f Hz, n = 4)" % (leff, f4))
    for k, name in ((1, "Ventil 2"), (2, "Ventil 1"), (3, "Ventil 3")):
        dl = leff * (2 ** (k / 12) - 1)
        # (a) im Hauptrohr bei 1,5 m mit dessen Radius; (b) an der Seele wie die Kernform (Zylinder am Eingang)
        i = np.searchsorted(x, 1.5)
        # Einfuegung: Zylinder mit r(x[i-1]) der Laenge dl zwischen x[i-1] und x[i]
        xa = np.insert(np.concatenate([x[:i], x[i:] + dl]), i, x[i - 1] + dl)
        ra = np.insert(r, i, r[i - 1])
        sa = zuordnen(spitzen(eingang(xa, ra)), TAB85[:11])
        sb =zuordnen(spitzen(eingang(np.concatenate([[0.0], x[ik:] - x[ik] + dl]), np.concatenate([[r[ik]], r[ik:]]), kessel=vk)), TAB85[:11])
        s0 = zuordnen(erg["Kernform (Kessel V + Seele)"], TAB85[:11])
        soll = -100 * k
        ca = [cent(a[0], b[0]) for a, b in zip(sa, basis)]
        cb = [cent(a[0], b[0]) for a, b in zip(sb, s0)]
        print("  %s (+%d HT, dL = %.3f m): Hauptrohr r=%.2f mm: %s" % (name, k, dl, r[i - 1] * 1e3, " ".join("%+5.0f" % v for v in ca)))
        print("  %s                         an der Seele r=%.2f mm: %s" % (" " * len(name), r[ik] * 1e3, " ".join("%+5.0f" % v for v in cb)))

    # --- 5. B-Seite: Hauptrohr kuerzen (Annahme), angepasst an Abb. 58 rot -------
    print("\n=== B-Seite (Annahme: F-Bohrung, Hauptrohr bei 1,0-2,6 m um dL gekuerzt; Ziel Abb. 58 rot, digitalisiert)")

    def kuerze(dl):
        a0 = np.searchsorted(x, 1.0)
        m = (x > 1.0) & (x < 1.0 + dl)
        xn = np.concatenate([x[:a0], x[a0:][x[a0:] >= 1.0 + dl] - dl])
        rn = np.concatenate([r[:a0], r[a0:][x[a0:] >= 1.0 + dl]])
        return xn, rn
    beste = None
    for dl in np.arange(0.80, 1.20, 0.005):
        xn, rn = kuerze(dl)
        zu = zuordnen(spitzen(eingang(xn, rn)), ABB58_ROT_B[:8], ab=90)
        e = np.mean([cent(s[0], m) for s, m in zip(zu, ABB58_ROT_B[:8])])
        if beste is None or abs(e) < abs(beste[1]):
            beste = (dl, e)
    dl = beste[0]
    xn, rn = kuerze(dl)
    spb = spitzen(eingang(xn, rn))
    zu = zuordnen(spb, ABB58_ROT_B, ab=90)
    print("  dL = %.3f m -> Laenge %.3f m" % (dl, xn[-1]))
    print("  n  f_rech  f_Abb58  Cent  |Z| MOhm  Abb58 MOhm  dB")
    for n, (s, m, h_) in enumerate(zip(zu, ABB58_ROT_B, ABB58_ROT_B_H), start=2):
        print("  %2d %7.1f %7.1f %+6.1f %7.1f %7.1f %+5.1f" % (n, s[0], m, cent(s[0], m), s[1] / 1e6, h_, 20 * np.log10(s[1] / 1e6 / h_)))
    harm = [116.54 * n / 2 for n in range(2, 12)]
    print("  gegen B-Reihe (B2 = 116,54 Hz bei a' = 440, n*58,27 Hz) [Cent]:",
          " ".join("%+.0f" % cent(s[0], h) for s, h in zip(zuordnen(spb, harm, ab=90), harm)))
    harmF = [43.654 * n for n in range(2, 16)]
    print("  F-Seite (Kern) gegen F-Reihe (n*43,65 Hz) [Cent]:",
          " ".join("%+.0f" % cent(s[0], h) for s, h in zip(zuordnen(erg["Kern (Profil am Eingang)"], harmF), harmF)))
    print("  F-Seite Messung Tab. 85 gegen F-Reihe [Cent]:", " ".join("%+.0f" % cent(m, h) for m, h in zip(TAB85, harmF)))


main()
```

## Anhang B — Ausgabe von `horn_rechnung.py`

```text
=== Bohrung F-Seite: 296 Punkte, Laenge 3.8620 m, Eingang r 8.75 mm, Becher r 149.8 mm

=== Pruefung: verlustfreier Zylinder L = 1 m, r = 5 mm, Ende ideal offen (p = 0)
  treppe: 85.766(-0.6872 c) 257.436(+0.2430 c) 429.000(+0.0000 c) 600.564(-0.1041 c) 772.234(+0.0763 c)
  kegel: 85.766(-0.6872 c) 257.436(+0.2430 c) 429.000(+0.0000 c) 600.564(-0.1041 c) 772.234(+0.0763 c)
  Soll (2n-1)c/4L: 85.800 257.400 429.000 600.600 772.200
  Kegel 5->50 mm, 1 m: Treppe ['154.96', '312.26', '472.99', '636.50', '802.25']  Kugelwelle ['154.96', '312.26', '472.99', '636.50', '802.25']
  Abstand Treppe-Kugelwelle [Cent]: ['-0.001', '-0.001', '-0.002', '-0.001', '-0.001']

Kesselvolumen Mundstueck 0-31,5 mm (Kegelstuempfe aus Abb. 52): 3.007 cm^3; ganzes Mundstueck 0-67,38 mm: 3.716 cm^3

=== Kern (Profil am Eingang): gegen Wachter Tab. 85 (2.-15. Spitze)
  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q
   2   83.71    85.3  -32.5     36.7     26.9   +2.7   17.4
   3  129.26   131.8  -33.7     31.8     32.3   -0.1   23.0
   4  172.69   173.9  -12.1     37.9     42.5   -1.0   26.9
   5  213.97   220.4  -51.2     53.6     58.6   -0.8   28.8
   6  257.50   262.4  -32.6     58.3     56.0   +0.4   31.7
   7  301.15   305.6  -25.4     59.5     57.8   +0.2   35.1
   8  340.40   352.1  -58.5     56.6     52.1   +0.7   37.1
   9  382.20   397.5  -68.0     47.6     47.9   -0.0   37.9
  10  427.14   437.4  -41.1     40.7     46.1   -1.1   39.1
  11  466.82   482.8  -58.3     39.7     39.0   +0.2   37.3
  12  509.65   526.0  -54.7     32.1     31.6   +0.2   34.5
  13  554.92   572.5  -54.0     19.9     22.7   -1.1   30.8
  14  598.35   621.8  -66.6     20.3     19.9   +0.2   29.8
  15  641.35   661.8  -54.3     14.7     16.6   -1.1   23.9
  Mittel Cent -45.9, Streuung (Std) 15.9; Mittel dB -0.0
  1. Spitze (Pedal): 28.3 Hz (Wachter Text S. 63: Pedalton 25 Hz)

=== Kegel: gegen Wachter Tab. 85 (2.-15. Spitze)
  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q
   2   83.71    85.3  -32.5     36.7     26.9   +2.7   17.4
   3  129.26   131.8  -33.7     31.8     32.3   -0.1   23.0
   4  172.69   173.9  -12.1     37.9     42.5   -1.0   26.9
   5  213.97   220.4  -51.2     53.7     58.6   -0.8   28.8
   6  257.50   262.4  -32.6     58.3     56.0   +0.4   31.7
   7  301.15   305.6  -25.4     59.5     57.8   +0.2   35.1
   8  340.39   352.1  -58.5     56.6     52.1   +0.7   37.1
   9  382.19   397.5  -68.0     47.6     47.9   -0.0   37.9
  10  427.14   437.4  -41.1     40.7     46.1   -1.1   39.1
  11  466.82   482.8  -58.3     39.7     39.0   +0.2   37.3
  12  509.64   526.0  -54.7     32.1     31.6   +0.2   34.5
  13  554.91   572.5  -54.0     19.9     22.7   -1.1   30.7
  14  598.35   621.8  -66.6     20.3     19.9   +0.2   29.8
  15  641.35   661.8  -54.3     14.7     16.6   -1.1   23.9
  Mittel Cent -45.9, Streuung (Std) 16.0; Mittel dB -0.0
  1. Spitze (Pedal): 28.3 Hz (Wachter Text S. 63: Pedalton 25 Hz)

=== LS: gegen Wachter Tab. 85 (2.-15. Spitze)
  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q
   2   83.71    85.3  -32.5     36.7     26.9   +2.7   17.4
   3  129.26   131.8  -33.7     31.8     32.3   -0.1   23.0
   4  172.69   173.9  -12.1     37.9     42.5   -1.0   26.9
   5  213.98   220.4  -51.2     53.7     58.6   -0.8   28.8
   6  257.51   262.4  -32.5     58.3     56.0   +0.4   31.7
   7  301.18   305.6  -25.2     59.6     57.8   +0.3   35.1
   8  340.45   352.1  -58.3     56.8     52.1   +0.7   37.2
   9  382.28   397.5  -67.6     47.8     47.9   -0.0   38.1
  10  427.31   437.4  -40.4     41.2     46.1   -1.0   39.5
  11  467.07   482.8  -57.3     40.4     39.0   +0.3   38.0
  12  509.97   526.0  -53.6     32.9     31.6   +0.4   35.6
  13  555.52   572.5  -52.1     20.9     22.7   -0.7   33.2
  14  598.88   621.8  -65.0     21.7     19.9   +0.8   33.2
  15  642.21   661.8  -52.0     15.8     16.6   -0.4   29.8
  Mittel Cent -45.3, Streuung (Std) 15.6; Mittel dB 0.1
  1. Spitze (Pedal): 28.3 Hz (Wachter Text S. 63: Pedalton 25 Hz)

=== K3+LS: gegen Wachter Tab. 85 (2.-15. Spitze)
  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q
   2   83.77    85.3  -31.3     37.2     26.9   +2.8   17.4
   3  129.31   131.8  -33.0     32.1     32.3   -0.0   22.9
   4  172.73   173.9  -11.7     38.2     42.5   -0.9   26.7
   5  213.99   220.4  -51.1     53.7     58.6   -0.8   28.4
   6  257.51   262.4  -32.6     58.1     56.0   +0.3   31.2
   7  301.17   305.6  -25.3     59.3     57.8   +0.2   34.5
   8  340.43   352.1  -58.4     56.2     52.1   +0.7   36.5
   9  382.26   397.5  -67.7     47.3     47.9   -0.1   37.3
  10  427.30   437.4  -40.5     40.8     46.1   -1.1   38.8
  11  467.06   482.8  -57.4     39.8     39.0   +0.2   37.1
  12  509.98   526.0  -53.5     32.5     31.6   +0.2   34.8
  13  555.54   572.5  -52.1     20.8     22.7   -0.8   32.7
  14  598.90   621.8  -65.0     21.5     19.9   +0.7   32.6
  15  642.22   661.8  -52.0     15.8     16.6   -0.4   29.4
  Mittel Cent -45.1, Streuung (Std) 15.7; Mittel dB 0.1
  1. Spitze (Pedal): 28.3 Hz (Wachter Text S. 63: Pedalton 25 Hz)

=== Kernform (Kessel V + Seele): gegen Wachter Tab. 85 (2.-15. Spitze)
  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q
   2   83.77    85.3  -31.3     36.7     26.9   +2.7   17.7
   3  129.36   131.8  -32.4     31.3     32.3   -0.3   23.5
   4  172.84   173.9  -10.6     36.7     42.5   -1.3   27.6
   5  214.25   220.4  -49.0     51.7     58.6   -1.1   29.7
   6  257.95   262.4  -29.6     55.5     56.0   -0.1   32.8
   7  301.78   305.6  -21.8     55.8     57.8   -0.3   36.3
   8  341.22   352.1  -54.4     53.2     52.1   +0.2   38.3
   9  383.17   397.5  -63.6     44.4     47.9   -0.7   38.9
  10  428.32   437.4  -36.3     37.0     46.1   -1.9   40.1
  11  468.34   482.8  -52.6     36.8     39.0   -0.5   38.3
  12  511.25   526.0  -49.3     30.4     31.6   -0.3   35.3
  13  556.21   572.5  -50.0     18.7     22.7   -1.7   30.6
  14  600.25   621.8  -61.1     18.7     19.9   -0.5   29.7
  15  642.95   661.8  -50.0     13.7     16.6   -1.7   23.2
  Mittel Cent -42.3, Streuung (Std) 14.9; Mittel dB -0.5
  1. Spitze (Pedal): 28.3 Hz (Wachter Text S. 63: Pedalton 25 Hz)

=== Kernform + LS: gegen Wachter Tab. 85 (2.-15. Spitze)
  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q
   2   83.77    85.3  -31.3     36.7     26.9   +2.7   17.7
   3  129.36   131.8  -32.4     31.3     32.3   -0.3   23.5
   4  172.84   173.9  -10.5     36.7     42.5   -1.3   27.6
   5  214.26   220.4  -48.9     51.7     58.6   -1.1   29.7
   6  257.95   262.4  -29.6     55.5     56.0   -0.1   32.8
   7  301.81   305.6  -21.6     55.9     57.8   -0.3   36.4
   8  341.27   352.1  -54.1     53.4     52.1   +0.2   38.4
   9  383.26   397.5  -63.2     44.6     47.9   -0.6   39.1
  10  428.50   437.4  -35.6     37.5     46.1   -1.8   40.5
  11  468.59   482.8  -51.7     37.5     39.0   -0.4   39.0
  12  511.57   526.0  -48.1     31.2     31.6   -0.1   36.4
  13  556.82   572.5  -48.1     19.5     22.7   -1.3   33.1
  14  600.77   621.8  -59.6     20.0     19.9   +0.0   33.2
  15  643.80   661.8  -47.7     14.7     16.6   -1.0   29.2
  Mittel Cent -41.6, Streuung (Std) 14.5; Mittel dB -0.4
  1. Spitze (Pedal): 28.3 Hz (Wachter Text S. 63: Pedalton 25 Hz)

Kern: alle Spitzen bis 1000 Hz
   1    28.28 Hz   52.43 MOhm   34.4 dB(1 MOhm)  Q  10.0
   2    83.71 Hz   36.74 MOhm   31.3 dB(1 MOhm)  Q  17.4
   3   129.26 Hz   31.78 MOhm   30.0 dB(1 MOhm)  Q  23.0
   4   172.69 Hz   37.90 MOhm   31.6 dB(1 MOhm)  Q  26.9
   5   213.97 Hz   53.65 MOhm   34.6 dB(1 MOhm)  Q  28.8
   6   257.50 Hz   58.25 MOhm   35.3 dB(1 MOhm)  Q  31.7
   7   301.15 Hz   59.49 MOhm   35.5 dB(1 MOhm)  Q  35.1
   8   340.40 Hz   56.63 MOhm   35.1 dB(1 MOhm)  Q  37.1
   9   382.20 Hz   47.60 MOhm   33.6 dB(1 MOhm)  Q  37.9
  10   427.14 Hz   40.69 MOhm   32.2 dB(1 MOhm)  Q  39.1
  11   466.82 Hz   39.73 MOhm   32.0 dB(1 MOhm)  Q  37.3
  12   509.65 Hz   32.13 MOhm   30.1 dB(1 MOhm)  Q  34.5
  13   554.92 Hz   19.94 MOhm   26.0 dB(1 MOhm)  Q  30.8
  14   598.35 Hz   20.33 MOhm   26.2 dB(1 MOhm)  Q  29.8
  15   641.35 Hz   14.69 MOhm   23.3 dB(1 MOhm)  Q  23.9
  16   690.49 Hz   14.18 MOhm   23.0 dB(1 MOhm)  Q  24.2
  17   734.16 Hz   11.85 MOhm   21.5 dB(1 MOhm)  Q   nan
  18   780.49 Hz   13.43 MOhm   22.6 dB(1 MOhm)  Q  21.4
  19   828.49 Hz   12.79 MOhm   22.1 dB(1 MOhm)  Q   nan
  20   874.82 Hz   15.84 MOhm   24.0 dB(1 MOhm)  Q   nan
  21   910.92 Hz   14.20 MOhm   23.0 dB(1 MOhm)  Q   nan
  22   959.78 Hz   14.37 MOhm   23.1 dB(1 MOhm)  Q   nan

Hauptrohr 0,40-2,75 m auf den Median r = 5.695 mm gesetzt: Cent gegen Tab. 85: -35 -4 -36 -49 -18 -32 -61 -61 -45 -59 -52 -51 -69 -55; Mittel -44.7, Std 17.4
  geglaettet, Hauptrohr um 0.10 m gekuerzt (Laenge 3.762 m): Mittel +0.5 Cent, Std 17.7, je Spitze +21 +39 +6 +0 +26 +8 -17 -15 -3 -16 -4 -7 -26 -6
  geglaettet, Hauptrohr um 0.08 m gekuerzt (Laenge 3.782 m): Mittel -8.6 Cent, Std 17.6, je Spitze +10 +31 -2 -10 +17 +0 -26 -24 -11 -24 -14 -16 -35 -16
  geglaettet, Hauptrohr um 0.06 m gekuerzt (Laenge 3.802 m): Mittel -17.7 Cent, Std 17.5, je Spitze -1 +22 -11 -20 +8 -8 -35 -34 -20 -33 -23 -25 -44 -26
  geglaettet, Hauptrohr um 0.04 m gekuerzt (Laenge 3.822 m): Mittel -26.8 Cent, Std 17.5, je Spitze -13 +13 -19 -29 -1 -16 -44 -43 -28 -42 -33 -34 -53 -36

Temperatur: +5 Grad C verschiebt die Spitzen 2-15 im Mittel um +14.8 Cent

=== Becher als Bessel-Horn r = b (x0 - x)^(-g), angepasst ab x = 3,0 m
  x0 = 3.8914 m (Ende 3.8620 m), b = 0.0074577, gamma = 0.8806, Restfehler rms 3.9 % in r, max 13.2 %
  Bessel-Becher statt Punktliste, Spitzen 2-12 [Cent]: ['-2.3', '+1.8', '+9.7', '+8.4', '+4.1', '-0.8', '-2.2', '+0.3', '+2.7', '+2.0', '+0.0']
  ... Hoehen [dB]: ['-0.0', '+0.0', '+0.1', '+0.0', '-0.1', '-0.1', '+0.0', '+0.1', '+0.0', '-0.1', '-0.1']

=== Empfindlichkeit (Kernmodell, zentrale Differenz +-1 %), Spitzen 2-12
  Mundrohr: x 0.070 ... 0.310 m, r 3.74 -> 5.18 mm; Hauptzylinder r (Median 0,5-2,5 m) 5.69 mm
  Mundrohr-Laenge (0,067-0,30 m)             Cent/%:  -0.9  -0.8  -0.9  -1.4  -1.4  -1.4  -1.3  -1.1  -1.1  -1.2  -1.1
                                             dB/%:   +0.01 +0.03 +0.05 +0.04 +0.02 +0.00 -0.04 -0.05 -0.01 -0.05 -0.08
  Mundrohr-Konus (Eingangsradius)            Cent/%:  -0.6  -0.5  -0.4  -0.4  -0.0  +0.4  +0.6  +0.6  +0.5  +0.5  +0.2
                                             dB/%:   -0.02 -0.04 -0.07 -0.11 -0.15 -0.16 -0.12 -0.06 -0.00 +0.07 +0.12
  Becherradius (Ende, ab 3,0 m eingeblendet) Cent/%:  +0.9  +1.6  +1.4  +0.5  +0.3  +0.3  +0.3  +0.3  +0.2  +0.1  +0.1
                                             dB/%:   +0.01 +0.01 +0.01 -0.00 -0.00 +0.00 -0.01 -0.01 -0.02 -0.02 -0.03
  Bessel-Exponent gamma                      Cent/%:  +0.9  +1.6  +1.5  +0.8  +0.5  +0.4  +0.4  +0.2  +0.1  +0.1  +0.0
                                             dB/%:   +0.01 +0.01 +0.01 -0.00 -0.00 +0.00 -0.01 -0.02 -0.03 -0.04 -0.04
  Mundstueck-Kesselvolumen                   Cent/%:  -0.2  -0.2  -0.2  -0.4  -0.5  -0.5  -0.5  -0.4  -0.4  -0.4  -0.3
                                             dB/%:   +0.01 +0.01 +0.02 +0.02 +0.01 -0.00 -0.03 -0.05 -0.06 -0.09 -0.11
  Radius Hauptrohr 0,4-2,8 m                 Cent/%:  -0.3  +0.6  +2.5  +0.4  -1.2  -0.5  +0.4  +0.0  +0.0  +0.8  +0.4
                                             dB/%:   -0.04 +0.08 +0.13 +0.16 +0.16 +0.07 -0.07 -0.11 -0.01 +0.09 +0.10

=== Ventilzuege (Annahme: ideale Laengen L_eff (2^(k/12) - 1), L_eff aus der 4. Spitze)
  L_eff = 3.975 m (aus f4 = 172.69 Hz, n = 4)
  Ventil 2 (+1 HT, dL = 0.236 m): Hauptrohr r=5.95 mm:  -122  -104  -100  -100  -104  -102   -90  -103  -103   -91  -104
                                   an der Seele r=2.33 mm:   -44   -60  -134  -210  -195  -167  -125  -115  -115   -94   -88
  Ventil 1 (+2 HT, dL = 0.487 m): Hauptrohr r=5.95 mm:  -246  -208  -203  -198  -207  -205  -184  -197  -210  -189  -196
                                   an der Seele r=2.33 mm:  -121  -280  -374  -299  -253  -221  -184  -199  -225  -217  -219
  Ventil 3 (+3 HT, dL = 0.752 m): Hauptrohr r=5.95 mm:  -371  -312  -308  -296  -309  -309  -285  -288  -312  -294  -290
                                   an der Seele r=2.33 mm:  -299  -524  -431  -335  -315  -338  -346  -324  -318  -296  -289

=== B-Seite (Annahme: F-Bohrung, Hauptrohr bei 1,0-2,6 m um dL gekuerzt; Ziel Abb. 58 rot, digitalisiert)
  dL = 1.050 m -> Laenge 2.812 m
  n  f_rech  f_Abb58  Cent  |Z| MOhm  Abb58 MOhm  dB
   2   119.5   122.4  -41.3    46.5    44.3  +0.4
   3   176.2   178.3  -20.2    55.4    60.8  -0.8
   4   234.5   234.6   -1.0    76.5    78.5  -0.2
   5   293.5   290.5  +17.8    83.4    70.2  +1.5
   6   346.4   346.8   -1.8    69.4    72.5  -0.4
   7   405.8   403.0  +12.1    61.2    60.0  +0.2
   8   460.5   456.7  +14.3    47.9    54.1  -1.1
   9   519.8   512.9  +23.1    35.3    35.2  +0.0
  10   578.6   568.8  +29.6    22.6    26.2  -1.3
  11   640.7   627.4  +36.3    16.8    20.5  -1.7
  gegen B-Reihe (B2 = 116,54 Hz bei a' = 440, n*58,27 Hz) [Cent]: +44 +14 +10 +13 -16 -9 -21 -15 -12 -1
  F-Seite (Kern) gegen F-Reihe (n*43,65 Hz) [Cent]: -73 -23 -19 -34 -29 -25 -44 -48 -38 -49 -48 -39 -37 -36
  F-Seite Messung Tab. 85 gegen F-Reihe [Cent]: -40 +11 -7 +17 +3 +0 +14 +20 +3 +9 +7 +15 +30 +18
```

## Anhang C — `digitalisiere_abb88.py`

Aufruf: `python digitalisiere_abb88.py seiten/w88_600-087.png abb88_roh.txt` (Bild: `pdftoppm -r 600 -f 87 -l 87 -x 660
-y 1440 -W 3640 -H 2160`). Ausgabe der Kalibrierung: „x: Pixel/0,1 m = 82.101, Rest max 1.33 px“, „y: Pixel/0,01 m =
58.825, Rest max 2.85 px“.

```python
"""Digitalisiert Wachter 2008, Abb. 88 (Mensurverlauf F-Horn mit Mundstueck, Ventilstellung 0) aus dem 600-dpi-Bild.

Kalibrierung ueber die Gitterlinien (grau, gestrichelt): senkrecht alle 0,1 m von -0,2 bis 3,9; waagrecht alle
0,01 m von -0,17 bis 0,17. Die rote Kurve: obere und untere Kontur; Radius = halber Abstand (hebt einen Versatz der
Nulllinie auf). Ausgabe: x [m], r [mm] je 5 mm und die Gitterkalibrierung.
"""
import sys
import numpy as np
from PIL import Image

pfad = sys.argv[1]
img = np.asarray(Image.open(pfad).convert("RGB")).astype(int)
R, G, B = img[..., 0], img[..., 1], img[..., 2]
rot = (R > 180) & (G < 110) & (B < 110)
grau = (abs(R - G) < 12) & (abs(G - B) < 12) & (R > 100) & (R < 215)
h, w = rot.shape

def linien(maske, achse, mindest):
    s = maske.sum(axis=achse)
    kand = np.where(s > mindest)[0]
    gruppen = []
    for k in kand:
        if gruppen and k - gruppen[-1][-1] <= 2:
            gruppen[-1].append(k)
        else:
            gruppen.append([k])
    return [float(np.mean(g)) for g in gruppen], s

vx, _ = linien(grau, 0, 0.25 * h)
vx = [v for v in vx if v > 100]  # links davon liegen die Beschriftungen
hy, _ = linien(grau, 1, 0.25 * w)
print("senkrechte Gitterlinien:", len(vx), "waagrechte:", len(hy))
print("vx:", [round(v) for v in vx])
print("hy:", [round(v) for v in hy])

def ordne(pos):
    pos = np.array(pos)
    d = np.median(np.diff(pos))
    idx = np.round((pos - pos[0]) / d)
    # je Index nur eine Linie (die naechste am Raster)
    fit = np.polyfit(idx, pos, 1)
    for _ in range(3):
        rest = abs(np.polyval(fit, idx) - pos)
        ok = rest < 0.2 * d
        fit = np.polyfit(idx[ok], pos[ok], 1)
    return idx[ok], pos[ok], fit

xs, vxo, px_x = ordne(vx)
ys, hyo, px_y = ordne(hy)
vx, hy = vxo, hyo
print("Index x:", xs.min(), xs.max(), " Index y:", ys.min(), ys.max())
print("x: Pixel/0,1 m = %.3f, Rest max %.2f px" % (px_x[0], np.max(abs(np.polyval(px_x, xs) - vx))))
print("y: Pixel/0,01 m = %.3f, Rest max %.2f px" % (px_y[0], np.max(abs(np.polyval(px_y, ys) - hy))))
# erste senkrechte Linie = -0,2 m (falls 42 Linien); erste waagrechte = +0,17 m (oben)
x0_wert = -0.2 if len(vx) >= 41 else None
def x_von_px(p):
    return -0.2 + (p - px_x[1]) / px_x[0] * 0.1
def y_von_px(p):
    return 0.17 - (p - px_y[1]) / px_y[0] * 0.01
# Nulllinie
null_px = px_y[1] + 17 * px_y[0]
print("Nulllinie bei px %.1f" % null_px)

oben = np.full(w, np.nan)
unten = np.full(w, np.nan)
# waagrechte Stetigkeit: ein Pixel zaehlt zur Kurve, wenn in den Spalten c-3 und c+3 in +-2 Zeilen auch Rot liegt
# (die Strichmarken an den Abschnittsgrenzen sind senkrecht und fallen heraus)
from scipy.ndimage import maximum_filter
rot_dil = maximum_filter(rot.astype(np.uint8), size=(5, 1)).astype(bool)
links = np.zeros_like(rot)
rechts = np.zeros_like(rot)
links[:, 3:] = rot_dil[:, :-3]
rechts[:, :-3] = rot_dil[:, 3:]
kurve = rot & links & rechts
for c in range(w):
    zeilen = np.where(kurve[:, c])[0]
    o = zeilen[zeilen < null_px - 2]
    u = zeilen[zeilen > null_px + 2]
    if len(o):
        oben[c] = o.max()  # innerste Kurvenzeile oberhalb der Null
    if len(u):
        unten[c] = u.min()
# Becher (steil): je Zeile die Spalte der roten Pixel rechts von x = 3,45 m
becher = []
c_ab = int(px_x[1] + (3.45 + 0.2) / 0.1 * px_x[0])
for z in range(0, int(null_px) - 2):
    cs = np.where(rot[z, c_ab:])[0]
    zs = np.where(rot[int(2 * null_px) - z, c_ab:])[0] if int(2 * null_px) - z < h else []
    if len(cs) and len(zs):
        becher.append((x_von_px(c_ab + cs.min()), x_von_px(c_ab + zs.min()), 0.17 - (z - px_y[1]) / px_y[0] * 0.01))
np.savetxt(sys.argv[2].replace(".txt", "_becher.txt"), np.array(becher), fmt="%.5f",
           header="x_oben_m x_unten_m r_m (Becher je Bildzeile)")
print("Becher je Zeile (x_oben, x_unten, r) — Auswahl:")
for b in becher[:: max(1, len(becher) // 40)]:
    print("  x_o=%.4f x_u=%.4f r=%.1f mm" % (b[0], b[1], b[2] * 1000))
# Median ueber ein Fenster von 9 Spalten gegen die Strichmarken
def med(a, k=9):
    out = np.full_like(a, np.nan)
    for i in range(len(a)):
        f = a[max(0, i - k // 2): i + k // 2 + 1]
        f = f[~np.isnan(f)]
        if len(f):
            out[i] = np.median(f)
    return out
oben_m, unten_m = med(oben), med(unten)
spalten = np.arange(w)
xm = x_von_px(spalten)
r_mm = (y_von_px(oben_m) - y_von_px(unten_m)) / 2 * 1000
mitte_mm = (y_von_px(oben_m) + y_von_px(unten_m)) / 2 * 1000
gueltig = ~np.isnan(r_mm)
print("x-Bereich der Kurve: %.4f ... %.4f m" % (xm[gueltig].min(), xm[gueltig].max()))
print("Pixel: 1 px = %.2f mm in x, %.3f mm in r" % (0.1 / px_x[0] * 1000, 0.01 / px_y[0] * 1000 / 2 * 2))
np.savetxt(sys.argv[2], np.column_stack([xm[gueltig], r_mm[gueltig], mitte_mm[gueltig]]), fmt="%.5f",
           header="x_m r_mm mitte_mm (Wachter 2008 Abb. 88, digitalisiert)")
# Tabelle je 50 mm (und dichter im Becher)
punkte = list(np.arange(0.0, 3.0, 0.1)) + list(np.arange(3.0, 3.7, 0.05)) + list(np.arange(3.7, 3.87, 0.01))
xv, rv = xm[gueltig], r_mm[gueltig]
for p in punkte:
    i = np.argmin(abs(xv - p))
    if abs(xv[i] - p) < 0.003:
        print("x=%.3f r=%.2f" % (xv[i], rv[i]))
print("letzter Punkt: x=%.4f r=%.2f" % (xv[-1], rv[-1]))
```

## Anhang D — `digitalisiere_kurve.py`

Aufrufe (Kalibrierpunkte aus den Gitterlinien): Abb. 58 (`pdftoppm -r 400 -f 64 … -x 680 -y 2120 -W 2060 -H 760`):
`rot|blau 41.5 0 2014 1500 730 0 48.5 80 20 800`; Abb. 89 (`-f 87 -x 560 -y 2740 -W 2240 -H 1360`):
`gruen 170 0 2148 1800 1256 0 107.5 60000 20 1000` (Höhe in kΩ, im Bericht /1000 = MΩ). Ergebnisse stehen in 4.2
und 4.5.

```python
"""Digitalisiert eine Impedanzkurve aus einem Seitenbild (Wachter 2008 Abb. 58 bzw. 89).

Aufruf: python digitalisiere_kurve.py <png> <farbe rot|blau|gruen> <x0_px> <x0_wert> <x1_px> <x1_wert>
        <y0_px> <y0_wert> <y1_px> <y1_wert> [<fmin> <fmax>]
Die Kalibrierpunkte (Pixel der Achsenteilung) werden vorher am Bild bestimmt (Gitterlinien, siehe Ausgabe
`--gitter`). Ausgabe: je Spalte der hoechste Pixel der Farbe; daraus die oertlichen Maxima (Frequenz, Hoehe).
"""
import sys
import numpy as np
from PIL import Image

img = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(int)
R, G, B = img[..., 0], img[..., 1], img[..., 2]
if sys.argv[2] == "--gitter":
    # dunkle bzw. graue, nicht farbige Pixel je Spalte/Zeile zaehlen
    grau = (abs(R - G) < 25) & (abs(G - B) < 25) & (R < 200)
    s0 = grau.sum(axis=0)
    s1 = grau.sum(axis=1)
    h, w = grau.shape
    print("Spalten mit vielen grauen Pixeln:", [i for i in range(w) if s0[i] > 0.15 * h])
    print("Zeilen mit vielen grauen Pixeln:", [i for i in range(h) if s1[i] > 0.15 * w])
    sys.exit()
farbe = sys.argv[2]
if farbe == "rot":
    m = (R > 170) & (G < 110) & (B < 110)
elif farbe == "blau":
    m = (B > 140) & (R < 90) & (G < 90)
else:
    m = (G > 150) & (R < 190) & (B < 90) & (G - R > 30)
x0p, x0v, x1p, x1v, y0p, y0v, y1p, y1v = map(float, sys.argv[3:11])
fmin, fmax = (float(sys.argv[11]), float(sys.argv[12])) if len(sys.argv) > 12 else (20.0, 900.0)
fx = lambda p: x0v + (p - x0p) * (x1v - x0v) / (x1p - x0p)
fy = lambda p: y0v + (p - y0p) * (y1v - y0v) / (y1p - y0p)
h, w = m.shape
oben = np.full(w, np.nan)
for c in range(w):
    z = np.where(m[:, c])[0]
    if len(z):
        oben[c] = z.min()
f = fx(np.arange(w))
y = fy(oben)
ok = ~np.isnan(y) & (f >= fmin) & (f <= fmax)
f, y = f[ok], y[ok]
# oertliche Maxima: hoeher als alles in +-(Fenster) Hz, Fenster 30 % des Abstands bei 40 Hz
spitzen = []
for i in range(len(f)):
    fen = (abs(f - f[i]) < 12.0)
    if y[i] >= y[fen].max() and y[i] > 3.0:
        if not spitzen or f[i] - spitzen[-1][0] > 15.0:
            # Mitte eines flachen Gipfels
            gipfel = f[fen & (y >= y[i] - 1e-9)]
            spitzen.append((float(gipfel.mean()), float(y[i])))
for fs, ys in spitzen:
    print("%.1f Hz  %.1f" % (fs, ys))
```

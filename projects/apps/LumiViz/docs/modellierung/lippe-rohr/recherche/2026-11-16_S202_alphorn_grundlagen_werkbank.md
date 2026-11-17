> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_17_alphorn_grundlagen` (gestellt in LumiViz
> S201), 2 Segment(e): 2026-11-17 08:42–09:14, claude-opus-5-5, 82 Runden, success, 6.30 USD; 2026-11-17 09:21–09:25, claude-opus-5-5, 6 Runden, success, 8.50 USD; Status `fertig`.
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Quellen unter `…\cmake\quellen\lippe-rohr\`):
> - **Marcinnò u. a. 2026 in der Textebene:** Tab. 1 (Horn 3 u. a. 330,5 / 454 Hz) und Tab. 2 „Reference alphorn“ (alle
>   zehn FEM-Spitzen 94,11 … 537,15 Hz), die zwei Sätze zu Spielton und Spitze (S. 14), „viscothermal“ — gefunden.
>   Soares u. a. 2025 („sufficiently smooth bents … negligible“) gefunden.
> - **Hendrie 2007 (2.102), S. 43 am Seitenbild:** im ersten Nennerterm steht nur Z_ℒ[sin(kℒ + kx̂₁ − kx̂₂)], **ohne**
>   1/(sin θ₁ sin θ₂) — der Druckfehler ist bestätigt. Mit dem Faktor ergibt Z_L = 0 genau (2.103) (nachgerechnet);
>   die Form mit Faktor ist die bei Fletcher & Rossing übliche (F&R nicht lokal). Hendries Textstellen „temperatures close
>   to 300“, „low frequency limit“, „ignore the radiation loss“ gefunden.
> - K5 (Konstanten 1,648·10⁻³ und 3,00·10⁻⁵) nachgerechnet — stimmt.
> - **Nicht gegengelesen:** die Rechnungen `tmm.py` / `rechnung.py` Zahl für Zahl (K1–K6, die Tabellen 4.1–4.6, der
>   Nachbau des LumiViz-Stands mit +242 Cent), Hendrie Abb. 2.12, die Annahmen zum Mundstück.
> - **Wichtigster Befund für LumiViz (nicht nachgerechnet):** mit der Radius-Lesart und der Herstellerlänge 3,47 m
>   liegt die Bohrung im Mittel −12 bis +18 Cent an den gemessenen Spieltönen; die +242 Cent des heutigen Stands
>   kommen zu rund 110 Cent aus R1, Kegel 4, Becher und Mundstück und zu rund 132 Cent aus der Länge.
> - **Nacharbeit:** Auftrag `2026-11-17_27_alphorn_nacharbeit` der Werkbank.
> Der Auftrag liegt seit S202 im Archiv der Werkbank (`archiv/auftraege/`, `archiv/ergebnisse/2026-11-16_17_alphorn_grundlagen/` mit
> `seiten/` und Textauszügen, `archiv/laeufe/`). Wo der Bericht `ergebnisse/…` oder `eingang/…` nennt, ist
> `archiv/ergebnisse/…` bzw. `archiv/eingang/…` gemeint.
> **Rechnungen und Ausgaben des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_alphorn_grundlagen_werkbank/`: `rechnung.py`, `tmm.py`, `pruefung.py`, `zusatz.py`, `inhalt_quellen.py`, `bericht_bauen.py`, `ausgabe_alles.md`, `ausgabe_haupt.md`, `ausgabe_seg.md`.

# Bericht: Berechnungsgrundlage für die Bohrung des Fis-Alphorns

- **Auftrag:** `auftraege/2026-11-16_17_alphorn_grundlagen.md`
- **Datum:** 2026-11-17
- **Modell:** claude-opus-5-5 (Lauf LVR)
- **Voraussetzung:** `ergebnisse/2026-11-16_07_alphorn_nachpruefung/bericht.md` liegt vor. Das Laufprotokoll
  `laeufe/2026-11-16_07_alphorn_nachpruefung_2026-11-16_2120.json` meldet `"subtype": "success"`; der Lauf endete also
  regulär. Wo die Nachprüfung den ersten Bericht (`2026-11-16_alphorn_geometrie`) korrigiert, gilt hier die Nachprüfung:
  - R2–R4 sind **Radien**; die Lesart als Durchmesser ist in der Nachprüfung widerlegt (1.1, 1.2);
  - R1 = 6,5 mm und R5 = 51,5 mm (S. 7);
  - die Becher-Stützpunkte kommen aus der Neuablesung (Abschnitt 3 dort), Bogenlänge 589,5 mm statt 585 mm.
- **Reichweite:**
  - **0 Websuchen, 0 Seitenabrufe.** Der Auftrag verbietet neue Geometrie-Recherche im Netz.
  - Gelesen als Seitenbild:
    - Marcinnò et al. 2026 (`eingang/2026-11-16_07_alphorn_nachpruefung/Marcinno_2026_alphorn.pdf`): S. 5, 6, 10, 11,
      13, 14, 15;
    - Hendrie 2007 (`…\quellen\lippe-rohr\Hendrie DA thesis 07.pdf`): gedruckte S. 28, 29, 34, 37, 38, 39, 42, 43, 44
      (PDF 50, 51, 56, 59, 60, 61, 64, 65, 66); dazu S. 13 nur als Textebene;
    - Soares et al. 2025: S. 3, 4, 5.
  - Nur über die Textebene durchsucht (`werkzeuge/pdf_suche.py`): Mattéoli et al. (2207.05395), Petiot et al. 2025,
    Resch et al. (1611.01025).
  - Im LumiViz-Repo nur gelesen: `ic/modell.md` (Z. 40, 68), `ic/modell-entwurf.md` (Z. 43), `ic/offen.md` (Z. 24),
    `recherche/2026-11-16_S201_alphorn_geometrie_werkbank.md`. Die letzte Datei ist der erste Bericht mit einem Kopf;
    Neues steht darin nicht.
  - **Nicht erreicht oder nicht vorhanden:**
    - Young 2009: Die Bohrung des Littleton-Horns fehlt; die Stufe 1 hat das PDF nicht laden können. Punkt 4 entfällt
      deshalb.
    - Formel für gekrümmte Rohre: in keiner lokalen Quelle (Abschnitt 4.6).
- **Marken:**
  - `[wörtlich]`: am Seitenbild geprüft;
  - `[Werkzeug]`: aus der Textebene oder einer Werkzeugantwort;
  - `[abgelesen]`: selbst an einer Abbildung abgelesen;
  - `[eigene Rechnung]`, `[eigene Einschätzung]`.
- **Dateien in diesem Ordner:**
  - `tmm.py`: Rechenkern;
  - `pruefung.py`: Kontrollen;
  - `rechnung.py`: alle Varianten;
  - `zusatz.py`: Variante mit den Schablonenlängen;
  - `ausgabe_alles.md`: vollständige Ausgabe von `rechnung.py`;
  - `seiten/`: die gelesenen Seitenbilder.

  Die Skripte stehen mit ihrer Ausgabe im Anhang.

## 0. Kurzfassung

1. **Der Rechenweg trägt.** Die eigene Übertragungsmatrix (Kegel mit Kugelwellen, Wandverluste nach Hendrie,
   Abstrahlung nach Hendrie) trifft die geschlossenen Lösungen für Zylinder und Kegel auf < 0,03 Cent. Hendries
   Abb. 2.12 (Spitzenhöhen) trifft sie auf ±1. Marcinnòs verlustfreie FEM-Spitzen des Horns 3 (Tab. 2) trifft sie mit
   derselben Bohrung auf **+6 bis +27 Cent** im Mittel der Naturtöne 2–10; das hängt vom Mundstück und vom
   Abstrahlungsansatz ab.
   - **Hendrie (2.102) ist im Druck fehlerhaft:** Im ersten Nennerterm fehlt der Faktor 1/(sin θ₁ sin θ₂). Wie gedruckt
     weicht die Formel um bis zu 52 % ab, mit dem Faktor um < 10⁻¹⁴.
2. **V1 (Radius-Lesart) gibt eine regelmäßige Reihe, V2 (Durchmesser-Lesart) nicht.**
   - V1 mit Mundstück a: Die Spitzen 3–16 liegen innerhalb einer Spanne von **29 Cent**.
   - V2a und V2b: Die Spanne beträgt **111–200 Cent**. Spitzen ballen sich, Höhen und Güten springen.
   - Die Rechnung bestätigt damit die Nachprüfung, die die Durchmesser-Lesart schon widerlegt hat.
3. **Mit der Länge aus Tab. 3 (3,24 m mit Mundstück) liegt V1 im Mittel +132 Cent über den gemessenen Spieltönen**
   (Tab. 1, Horn 3, Naturtöne 4–10).
   - Marcinnòs eigene FEM für dasselbe Horn liegt +67 bis +126 Cent darüber, im Mittel +95. Die Lücke steckt also schon
     in der Quelle.
   - **Verlängert auf 3,47 m** (Herstellerlänge für Fis) beträgt die Abweichung im Mittel −12 bis +18 Cent, höchstens
     51–56 Cent (beim Naturton 10, dessen Spielton selbst −53 Cent unter dem Zielton liegt). Das hängt davon ab, wo die
     Verlängerung sitzt.
   - Mit den **Schablonenlängen** aus Abb. 3a (L2 = 940, L3 = 825 mm; insgesamt 3,41 m) sind es +47 bis +50 Cent.
4. **Der LumiViz-Befund (Z3) ist nachgebaut.** Der Nachbau des heutigen Stands ergibt:
   - Länge 3,08 m, R1 = 2,75 mm, Kegel 4 als Zylinder, Becher als Kegel über 0,5 m, ohne Mundstück;
   - Spitze 2 bei 104,3 Hz, also **+207 Cent** gegen 92,5 Hz;
   - im Mittel über die Spitzen 2–16 **+242 Cent**, Spitze 3 bei +304 Cent;
   - Spitze 3 liegt 97 Cent über der Reihe auf Spitze 2.

   Zerlegt man den Weg zu V1, verteilt sich die Wirkung so:
   - R1 6,5 mm: −32 Cent;
   - Kegel 4 als Kegel und Becher nach Abb. 3b: −47 Cent;
   - Mundstück: −31 Cent;
   - es bleiben +132 Cent, die nur die Länge schließt (Abschnitt 4.3).
5. **Die Harmonizität bestimmen am stärksten:**
   - die Rückbohrung des Mundstücks (Annahme 20–70 mm: Spitze 2 −3…+2, Spitze 16 −27…+17 Cent);
   - die Frage, ob der Becher über die Bogenlänge oder über x läuft: Über x liegt die Reihe +37…+67 Cent höher, ungleich
     über n verteilt;
   - der Abstrahlungsansatz (geflanscht gegen ungeflanscht: −5…−16 Cent).

   Die Gesamtlänge verschiebt die Reihe dagegen nur als Ganzes, um −16,6 Cent je % Länge (Spanne über n ≈ 1 Cent).
6. **Nicht geliefert:**
   - Littleton-Horn (Young 2009), weil die Bohrung fehlt;
   - eine Krümmungsformel, weil sie lokal fehlt; geführt wird die Annahme „Bogenlänge“.


## 1. Gleichungen

Zeitkonvention e^{+iωt} wie bei Hendrie. Alle Gleichungen sind in `tmm.py` umgesetzt; die Nummer steht dort im Kommentar.

| Nr. | Gleichung | Variablen (Einheit) | Quelle / Seite / Gl. | Marke | Gültigkeit |
|---|---|---|---|---|---|
| G1 | v = √(γ R T_abs / m) | γ = 1,4; R = 8,314 J/(mol K); m = 0,02896 kg/mol; T_abs (K) | Hendrie 2007, S. 13 (PDF 35), (2.2) | [Werkzeug] (Textebene; Gleichung trivial, Zahlen für γ, R, m sind Standardwerte, nicht aus der Quelle) | ideales Gas |
| G2 | r_v = (ωρ/η)^½ a ≈ 632,8 a √(ω/2π) (1 − 0,0031 ΔT); r_t = (ωρC_p/κ)^½ a ≈ 532,8 a √(ω/2π) (1 − 0,0031 ΔT), ΔT = T − 300 K | a Rohrradius (m), ω (rad/s) | Hendrie S. 28 (PDF 50), (2.71)–(2.74), nach Fletcher & Rossing [86] und [88] | [wörtlich] (Seitenbild) | „temperatures close to 300 K (within ±10 K)“ |
| G3 | v ≈ c [1 − 1/(√2 r_v) − (γ−1)/(√2 r_t)]; α ≈ (ω/c) [1/(√2 r_v) + (γ−1)/(√2 r_t)] | v Phasengeschwindigkeit (m/s), α Dämpfung (1/m) | Hendrie S. 28–29, (2.75), (2.76) | [wörtlich] | r_v, r_t ≫ 1 (dünne Grenzschicht); bei a = 2,75 mm und 50 Hz ist r_v ≈ 12 |
| G3' | v ≈ c [1 − 1,65·10⁻³/(a√f)]; α ≈ 3·10⁻⁵ √f / a | f (Hz), a (m) | Hendrie S. 29, (2.77), (2.78) | [wörtlich] | wie G2; Kontrolle K5 |
| G4 | Γ = ω/v − iα ersetzt k | Γ (1/m) | Hendrie S. 29, (2.79) | [wörtlich] | Hendrie S. 43: in Kegeln hängen die Verluste vom Radius ab, „not easily accounted for“; hier: Kegel in Scheiben ≤ 10 mm, je Scheibe Γ beim mittleren Radius [eigene Einschätzung] |
| G5 | Z_c = ρc/S | S Querschnitt (m²) | Hendrie S. 21 (PDF 43), (2.45) | [Werkzeug] (Textebene) | ebene Welle |
| G6 | Kegel: p(x) = (A e^{−iΓx} + B e^{iΓx})/x, x vom Apex; U = S·u, iωρ u = −∂p/∂x; (p₁,U₁) = M(x₁) M(x₂)⁻¹ (p₂,U₂) | x₁ = r₁L/(r₂−r₁), x₂ = x₁ + L (vorzeichenbehaftet, auch für sich verengende Kegel) | eigene Herleitung; geprüft gegen Hendrie S. 43, (2.102) und S. 44, (2.103) (Kontrollen K2, K3) | [eigene Rechnung] | Kugelwellen im Kegel; Übergänge zwischen Kegeln ohne Korrektur (Druck und Fluss stetig) |
| G6a | Z_IN = Z_c1 · [i Z_L sin(kL−θ₂)/sin θ₂ + Z_c2 sin kL] / [Z_L sin(kL+θ₁−θ₂)/(sin θ₁ sin θ₂) − i Z_c2 sin(kL+θ₁)/sin θ₁], θ_j = arctan(k x_j) | Z_L Last am weiten Ende | Hendrie S. 43, (2.102), nach F&R [86] | [wörtlich] **mit Druckfehler**: im Seitenbild fehlt im ersten Nennerterm der Faktor 1/(sin θ₁ sin θ₂). Wie gedruckt weicht (2.102) von G6 um bis zu 52 % ab, mit dem Faktor um < 10⁻¹⁴ (K3) [eigene Rechnung] | verlustfrei |
| G6b | Z_IN = i Z_c1 sin(kL) sin θ₁ / sin(kL + θ₁) (ideal offen) | | Hendrie S. 44, (2.103) | [wörtlich] | verlustfrei, Z_L = 0 |
| G7 | Z_{N−1} = Z_c [Z_N cos kl + i Z_c sin kl] / [i Z_N sin kl + Z_c cos kl] (Zylinder; k → Γ) | | Hendrie S. 39 (PDF 61), (2.93) | [wörtlich] | im Programm mit Z_c = ρω/(ΓS), damit Zylinder- und Kegelscheiben ohne Scheinreflexion aneinanderpassen (Unterschied ~10⁻³) [eigene Einschätzung] |
| G8 | Z_unflanged = Z_c [(ka)²/4 + i·0,6 ka] | a Becherrand-Radius (m) | Hendrie S. 37 (PDF 59), (2.91) | [wörtlich] | „low frequency limit“ ka ≪ 1; am Becher (a = 89,5 mm) ist ka = 0,16 bei 100 Hz, 0,82 bei 500 Hz, 1,3 bei 800 Hz — **ab etwa Spitze 8 außerhalb** |
| G9 | Z_flanged ≈ Z_c [½(ka)² + i (8/3π) ka] | | Hendrie S. 34 (PDF 56), (2.86) | [wörtlich] | wie G8; als zweite Grenze gerechnet |
| G10 | Kessel als Volumen + Verengung: Z_in = 1/(iωV/(ρc²) + 1/(iωρ l/S_e + Z_Rest)) | V Kesselvolumen, l, S_e Länge und Querschnitt der Verengung | keine lokale Quelle; Standardansatz | [eigene Einschätzung] | ωl/c ≪ 1 |
| G11 | Cent = 1200 log₂(f/f_ref) | | — | [eigene Rechnung] | — |
| G12 | Marcinnò-FEM (nur zum Vergleich): Δp + k²p = 0, ∂p/∂n = 0 an der Wand, ∂p/∂n = iω(1+ikr)/(c₀ ikr) p auf der Halbkugel R = 0,447 m; c₀ = 340 m/s, ρ = 1,225 kg/m³, **verlustfrei** | | Marcinnò S. 5–6, (1)–(4) | [wörtlich] | Marcinnò S. 15: „viscothermal losses“ ausdrücklich nicht enthalten |

**Krümmung:** In den lokalen Quellen gibt es **keine Formel**. Soares et al. 2025, S. 3–4: „Some studies have underlined that curved duct bents can induce some mistuning and/or added dissipation in the response of brass instruments [25]. However, for sufficiently smooth bents in relatively narrow ducts, these effects can be considered negligible in a first approximation.“ [wörtlich]; [25] = Félix, Dalmont, Nederveen, JASA 131 (2012) 4164–4172 (nicht lokal). Resch et al. (1611.01025, S. 5): „we justified that the bends of the instrument do not greatly influence the wave propagation“ (Trompete) [Werkzeug, Textebene]. Daher hier die Annahme **„Bogenlänge statt x-Strecke“**.

**Spielton gegen Spitze:** Marcinnò S. 14: „In lip-reed instruments, the oscillation frequency can lock within a finite range around a resonance due to nonlinear interaction between the exciter and the acoustic impedance [6]. Since the present model is linear …, it cannot represent these locking ranges.“ und „we measured the notes effectively on the real alphorn geometry and observed that they are significantly closer to the target frequencies than the corresponding impedance peaks predicted by the numerical model.“ [wörtlich, Seitenbild]. Eine Zahl für den Abstand Spielton–Spitze nennt keine lokale Quelle.


## 2. Parametertabelle

| Größe | Zahl | Einheit | Quelle | Marke | Gültigkeit / Spanne |
|---|---|---|---|---|---|
| L1, L2, L3, L4 Horn 3 | 539,92 / 844,91 / 754,84 / 444,94 | mm | Marcinnò Tab. 3, S. 18 | [wörtlich] (Nachprüfung) | Horn 1: 544,96 / 844,87 / 754,84 / 444,94; Horn 2: 534,95 / 799,86 / 804,85 / 444,94 |
| R2, R3, R4 Horn 3 | 16,00 / 28,50 / 44,00 | mm (Radius) | Tab. 3; Lesart Radius: S. 7 | [wörtlich] | Horn 1, 2: R2 = 13,50 |
| R1 | 6,5 | mm | S. 7, „R_min = 6.5 mm“ | [wörtlich]; Zuordnung R1 = R_min [eigene Einschätzung, eng am Wortlaut] (Nachprüfung A1a) | Empfindlichkeit 4.4 |
| R5 | 51,5 | mm | S. 7, „R_max = 51.5 mm“ | wie R1 | — |
| Becher | 9 Stützpunkte, Bogenlänge 589,5 mm, r 51,5 → 89,5 mm | mm | Abb. 3b, Neuablesung der Nachprüfung; dazu der Punkt x = 305 (Beginn der Aufweitung) | [abgelesen] (Nachprüfung); Bogen [eigene Rechnung] | ±0,5 mm in r; ±3…5 mm in x/y; Bogen ±1 % (Nachprüfung), hier ±5 % gerechnet |
| Schablonenlängen | L2 ≈ 940, L3 ≈ 825 | mm | Abb. 3a, Handschrift „56/28 L 940“, „88/58 L 825“ | [abgelesen] (Nachprüfung 1.2) | Zuordnung zu Kegel 2/3 [eigene Einschätzung]; nur als Variante |
| Länge Fis | 343 / 347 | cm | Thomann / Lechgold | [Werkzeug] (Nachprüfung 4) | ob mit Mundstück, unbekannt |
| Mundstück a | Kessel als Kegel r 9,5 → 2,75 mm über 25 mm; Seele r 2,75 mm über 5 mm; Rückbohrung Kegel 2,75 → R1 über 40 mm (Länge 70 mm) | mm | Kessel 19 mm, Bohrung 5,5 mm, Kesseltiefe 25 mm: Lechgold [Werkzeug]; Kegelform, Seele 5 mm, Rückbohrung 40 mm: **Annahme** | [eigene Einschätzung] | Rückbohrung 20–70 mm gerechnet (Abschnitt 5) |
| Mundstück b | „Kesseltiefe 25 mm“ = Kessel allein: Kegel r 9,5 → 6,0 über 25 mm, Übergang 6,0 → 2,75 über 6 mm, Seele 5 mm, Rückbohrung 40 mm (76 mm) | mm | wie a; Lesart aus Nachprüfung 5 | [eigene Einschätzung] | r 6,0 am Kesselgrund frei gewählt |
| Mundstück c | Volumen V = 3 245 mm³ (= Kessel a) parallel, Verengung l = 10 mm, r = 2,75 mm in Reihe, dann Rückbohrung 40 mm | mm³, mm | G10 | [eigene Einschätzung] | l = 10 mm frei gewählt |
| Mundstück d | gemessenes Waldhorn-Mundstück (Wachter 2008, Abb. 52; Ø 17,5 → 4,65 → 6,3 mm über 67,38 mm), dann Kegel r 3,15 → R1 über 20 mm (87,4 mm) | mm | Profil: erster Bericht, Abschn. 2 [wörtlich]; Anschluss: Annahme | [wörtlich] / [eigene Einschätzung] | anderes Instrument |
| Luft | T = 20 °C; c = 343,23 m/s (G1); ρ = 1,2041 kg/m³ (ideales Gas, p = 101 325 Pa) | | G1; ρ eigene Rechnung | [eigene Rechnung] | T 14,5–30 °C gerechnet: 2,9 Cent/K, gleichmäßig über n |
| Luft bei Marcinnò | c₀ = 340 m/s, ρ = 1,225 kg/m³ | | S. 5 | [wörtlich] | entspricht ≈ 14,5 °C [eigene Rechnung] |
| Stimmton, Bezug A | a′ = 440 Hz; Fis1 = 440·2^(−39/12) = 46,249 Hz | Hz | 440 Hz: Lechgold, Thomann [Werkzeug] | [eigene Rechnung] | Marcinnòs Zielskala (Tab. 1/2): 92, 139, 185, 234, 278, 331, 371, 417, 468, 556 Hz [wörtlich] |
| Bezug B | n·f₂/2 (eigene Reihe auf Spitze 2) | Hz | Auftrag | — | — |
| Gemessene Spieltöne | Horn 3: 185,00 / 234,00 / 278,00 / 330,50 / 371,00 / 417,00 / 454,00 (Naturton 4–10); Horn 1: 180 / 204 / 278 / 310 / 371 / 417 / 463; Horn 2: 165 / 211 / 278 / 311 / 366 / 417 / 448 | Hz | Marcinnò Tab. 1, S. 10 | [wörtlich] (Seitenbild in diesem Lauf) | Naturton-Zuordnung (Do1 = 4 … Mi2 = 10) [eigene Rechnung]: 4·46,25 = 185 |
| FEM-Spitzen Horn 3 | 94,11 / 146,83 / 196,97 / 246,98 / 296,51 / 343,50 / 388,71 / 436,02 / 487,77 / 537,15 | Hz | Marcinnò Tab. 2 „Reference alphorn“, S. 15 | [wörtlich] | verlustfrei, c = 340. Die letzte Spitze steht unter „Sol 556“, ist nach f/n ≈ 48,8 Hz aber der **11.** Naturton [eigene Rechnung] |

**Segmentlisten**: vollständig im Anhang (Ausgabe `rechnung.py`, Abschnitt „Segmentliste“). Kurz:

| Segment | V1 (Radius) | V2a (Durchmesser, Sprung) | V2b (Durchmesser, ohne Sprung) | Quelle / Marke |
|---|---|---|---|---|
| Mundstück | a/b/c/d/0, Ende bei R1 = 6,5 | a/b/c/d/0, Ende bei R1 = 3,25 | wie V2a | Annahme |
| Kegel 1, L1 | 6,5 → 16,0 | 3,25 → 8,0 | 3,25 → 8,0 | Tab. 3 [wörtlich] / halbiert |
| Kegel 2, L2 | 16,0 → 28,5 | 8,0 → 14,25 | 8,0 → 14,25 | dto. |
| Kegel 3, L3 | 28,5 → 44,0 | 14,25 → 22,0 | 14,25 → 22,0 | dto. |
| Kegel 4, L4 | 44,0 → 51,5 | 22,0 → 25,75, dann **Sprung auf 51,5** am Becheransatz (Fläche × 4) | 22,0 → 51,5 | S. 7 [wörtlich]; V2: [eigene Einschätzung] |
| Becher | 8 Kegel über die Stützpunkte, 589,5 mm | dto. | dto. | Abb. 3b [abgelesen] |

Becher-Stützpunkte (s = Streckenzug über (x, y)):

| Punkt | x (mm) | y (mm) | s (mm) | r (mm) |
|---|---|---|---|---|
| 0 | 0 | 0 | 0,0 | 51,5 (Abb.: 51,7; auf R5 gesetzt) |
| 1 | 100 | 0,5 | 100,0 | 51,5 |
| 2 | 200 | 8 | 200,3 | 51,6 |
| 3 | 300 | 28 | 302,3 | 51,8 |
| 4 | 305 | 30,5 | 307,9 | 51,8 (Beginn der Aufweitung) |
| 5 | 350 | 53 | 358,2 | 57,0 |
| 6 | 400 | 92 | 421,6 | 62,0 |
| 7 | 450 | 158 | 504,4 | 71,0 |
| 8 | 492 | 232 | 589,5 | 89,5 |

**Verlängerung auf 3,47 m (Annahme):**
- „3,47 m“ gilt hier einschließlich Mundstück.
- Bei V1 mit Mundstück a fehlen 3 470 − 3 244,1 = **225,9 mm**.
- Drei Orte sind gerechnet:
  - **Mundrohr:** Zylinder r = R1 zwischen Mundstück und Kegel 1;
  - **Kegel 1:** L1 + Δ bei gleichen Radien;
  - **gleichmäßig:** L1…L4 mit demselben Faktor 1,0874.

## 3. Zahlenbeispiele: Gleichungen gegen Werte der Quellen

Skript `pruefung.py`, Ausgabe im Anhang.

| Kontrolle | prüft | Ergebnis | Marke |
|---|---|---|---|
| K5 | G2/G3 → G3′: Konstanten | 1/(√2·632,8) + 0,4/(√2·532,8) = 1,648·10⁻³ (Hendrie: 1,65·10⁻³); 2π·1,648·10⁻³/345 = 3,00·10⁻⁵ (Hendrie: 3·10⁻⁵) | [eigene Rechnung] gegen [wörtlich] |
| K1 | G7, verlustfrei | Zylinder r 20 mm, L 1 m, ideal offen: Spitzen 1–5 auf ≤ 0,021 Cent bei (2n−1)c/4L | [eigene Rechnung] |
| K2 | G6 gegen G6b (2.103) | Kegel r 5 → 50 mm, L 1 m: Spitzen 1–5 auf ≤ 0,003 Cent | [eigene Rechnung] |
| K3 | G6 gegen G6a (2.102), Last G8 | wie gedruckt: Abweichung 0,9 % / 18 % / 52 % / 0,8 % bei 50 / 137 / 300 / 555 Hz; mit dem Faktor 1/(sin θ₁ sin θ₂): < 10⁻¹⁴. **Druckfehler in Hendrie (2.102)**. Die Form mit dem Faktor als Form nach Fletcher & Rossing [86] ist [eigene Einschätzung]; F&R selbst ist nicht lokal | [eigene Rechnung] |
| K4b | G2–G4 + (2.92) gegen Hendrie Abb. 2.12 | Zylinder r 2 cm, L 1 m, c = 345: \|Z/Z_c\| der Spitzen 2–12 = 41,1 / 31,8 / 26,9 / 23,7 / 21,5 / 19,8 / 18,4 / 17,3 / 16,4 / 15,6 / 14,9; abgelesen 40 / 31 / 27 / 24 / 22 / 20 / 18 / 17 / 16 / 15,5 / 15 (±1) | [abgelesen] (S. 38, PDF 60) |
| K4 | G8 mit Strahlungswiderstand | dasselbe Rohr mit (2.91) samt Realteil: Die Spitzen fallen schneller ab (37,7 … 5,8). Abb. 2.12 rechnet also ohne Strahlungsverlust, wie Hendrie S. 37 schreibt („ignore the radiation loss“) | [wörtlich] / [eigene Rechnung] |
| K6 | ganzer Rechenweg gegen Marcinnò Tab. 2 (FEM, verlustfrei, c = 340) | V1 Horn 3, verlustfrei, c = 340, ρ = 1,225; Cent eigene − FEM, Naturton 2–11 (Tabelle unten) | [wörtlich] gegen [eigene Rechnung] |

K6 (Cent = 1200·log₂(f_eigen / f_FEM)):

| Mundstück | Abstrahlung | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | Mittel 2–10 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| a | ungeflanscht | +12 | +14 | +14 | +22 | +20 | +29 | +41 | +46 | +36 | +35 | +26 |
| a | geflanscht | +7 | +9 | +8 | +14 | +9 | +15 | +26 | +31 | +23 | +22 | +16 |
| b | ungeflanscht | +9 | +9 | +8 | +14 | +9 | +18 | +28 | +31 | +18 | +15 | +16 |
| b | geflanscht | +4 | +4 | +2 | +6 | −1 | +5 | +14 | +17 | +6 | +4 | +6 |
| c | ungeflanscht | +12 | +14 | +15 | +23 | +20 | +30 | +42 | +47 | +37 | +36 | +27 |
| d | ungeflanscht | +13 | +16 | +17 | +24 | +21 | +29 | +38 | +38 | +20 | +7 | +24 |
| 0 (ohne) | ungeflanscht | +19 | +27 | +33 | +44 | +44 | +54 | +69 | +76 | +69 | +70 | +48 |

**Einschätzung zu K6** `[eigene Einschätzung]`:
- Bis Naturton 6 liegt die 1D-Rechnung mit Mundstück 2–24 Cent über der FEM, ab Naturton 7 bis +47 Cent.
- Mögliche Ursachen:
  - Marcinnòs gemessenes, uns unbekanntes Mundstück;
  - die 3D-Abstrahlung über die Halbkugel (G12) gegen G8/G9 — bei ka > 0,5 liegen G8/G9 außerhalb ihres Bereichs;
  - der gekrümmte Becher in der FEM.
- Die Lücke FEM–Spielton (≈ +95 Cent, 4.2) ist viel größer als die Lücke 1D–FEM.

## 4. Ergebnistabellen

Grundfall, wenn nichts anderes steht: Wandverluste G2–G4, Abstrahlung ungeflanscht (G8), 20 °C. Je Variante sind alle 16
Spitzen gefunden. Die Güte Q ist bei hohen Spitzen oft nicht bestimmt („–“ bzw. `nan` im Anhang): Die −3-dB-Breite
reichte über das Suchfenster hinaus (≤ 6 Hz und ≤ 0,45 × Spitzenabstand). Das heißt nicht „unendlich“.

### 4.1 Übersicht: Kennzahlen je Variante (Naturtöne 2–16)

- **A:** Cent gegen n·46,249 Hz. **B:** Cent gegen n·f₂/2.
- **Spanne 3–16 (A):** größte minus kleinste Abweichung über die Spitzen 3–16; misst die Unharmonizität unabhängig vom
  Bezug.
- Spitze 1 liegt beim Blech nicht auf der Reihe; f1 steht gesondert.

| Variante | Länge (mm) | f1 (Hz) | f2 (Hz) | Cent f2 (A) | Mittel 2–16 (A) | mittl. Betrag (A) | max. Betrag (A) | mittl. Betrag (B) | max. Betrag (B) | Spanne 3–16 (A) |
|---|---|---|---|---|---|---|---|---|---|---|
| V1 H3 MP-a | 3244 | 48,7 | 94,84 | +43 | +132 | 132 | 145 | 88 | 101 | 29 |
| V1 H3 MP-b | 3250 | 48,7 | 94,70 | +41 | +117 | 117 | 134 | 77 | 94 | 23 |
| V1 H3 MP-c | 3244 | 48,7 | 94,87 | +44 | +133 | 133 | 147 | 90 | 104 | 31 |
| V1 H3 MP-d | 3261 | 48,7 | 94,87 | +44 | +112 | 112 | 144 | 68 | 100 | 54 |
| V1 H3 MP-0 | 3174 | 48,8 | 95,31 | +52 | +163 | 163 | 194 | 111 | 142 | 64 |
| V1 H3 MP-a +3,47 m Mundrohr | 3470 | 48,3 | 92,12 | −7 | +9 | 20 | 34 | 24 | 41 | 65 |
| V1 H3 MP-a +3,47 m Kegel 1 | 3470 | 47,5 | 88,81 | −70 | +12 | 27 | 70 | 82 | 103 | 63 |
| V1 H3 MP-a +3,47 m gleichmäßig | 3470 | 45,5 | 88,52 | −76 | +17 | 28 | 76 | 93 | 109 | 38 |
| V1 Schablonenlängen MP-a (`zusatz.py`) | 3409 | – | 89,84 | −51 | +48 | – | – | – | – | 35 |
| V2a H3 MP-a | 3244 | 54,7 | 104,98 | +219 | +100 | 100 | 219 | 119 | 212 | 137 |
| V2a H3 MP-b | 3250 | 54,6 | 104,13 | +205 | +81 | 86 | 205 | 124 | 225 | 155 |
| V2a H3 MP-0 | 3174 | 55,1 | 106,78 | +249 | +167 | 167 | 249 | 81 | 189 | 146 |
| V2a H3 MP-a +3,47 m gleichmäßig | 3470 | 50,5 | 97,59 | +93 | −12 | 40 | 99 | 105 | 192 | 151 |
| V2b H3 MP-a | 3244 | 58,6 | 103,48 | +194 | +105 | 105 | 194 | 90 | 181 | 117 |
| V2b H3 MP-b | 3250 | 58,4 | 102,93 | +185 | +87 | 88 | 185 | 98 | 192 | 132 |
| V2b H3 MP-a +3,47 m gleichmäßig | 3470 | 54,2 | 97,74 | +95 | −8 | 26 | 96 | 103 | 191 | 111 |
| V1 H1 MP-a | 3249 | 50,1 | 96,77 | +78 | +126 | 126 | 141 | 47 | 63 | 54 |
| V1 H2 MP-a | 3244 | 49,7 | 97,79 | +96 | +129 | 129 | 144 | 34 | 48 | 55 |
| LumiViz heute (Nachbau, Becher Kegel) | 3085 | 51,3 | 104,26 | +207 | +242 | 242 | 304 | 35 | 97 | 83 |
| LumiViz heute (Nachbau, Becher exponentiell) | 3085 | 50,8 | 103,57 | +196 | +244 | 244 | 300 | 48 | 105 | 72 |

Übrige Mundstücke und Verlängerungsorte zu V2: Anhang.

**Bezug B ist schief** `[eigene Einschätzung]`: Spitze 2 liegt bei V1 durchweg 75–110 Cent tiefer als die übrigen
Spitzen; in Marcinnòs FEM (Tab. 2) ebenso (+39 gegen +64…+112 zur Zielskala). Für die Harmonizität ist die Spanne 3–16
das bessere Maß.

### 4.2 Spitzen gegen die gemessenen Spieltöne (Tab. 1), Naturtöne 4–10

Cent = 1200·log₂(f_Spitze / f_gemessen).

| Variante | Horn | n4 | n5 | n6 | n7 | n8 | n9 | n10 | Mittel | max. Betrag |
|---|---|---|---|---|---|---|---|---|---|---|
| **Marcinnò FEM (Tab. 1, c = 340, verlustfrei)** | H3 | +109 | +94 | +112 | +67 | +81 | +77 | +126 | +95 | 126 |
| V1 H3 MP-a | H3 | +129 | +122 | +139 | +104 | +130 | +132 | +170 | +132 | 170 |
| V1 H3 MP-b | H3 | +122 | +114 | +128 | +94 | +118 | +118 | +152 | +121 | 152 |
| V1 H3 MP-d | H3 | +130 | +123 | +139 | +102 | +126 | +123 | +152 | +128 | 152 |
| V1 H3 MP-0 | H3 | +148 | +145 | +164 | +130 | +159 | +163 | +204 | +159 | 204 |
| V1 H3 MP-a +3,47 m Mundrohr | H3 | −31 | −51 | −18 | −36 | −2 | +6 | +48 | −12 | 51 |
| V1 H3 MP-a +3,47 m Kegel 1 | H3 | −10 | −6 | +17 | −13 | +14 | +16 | +52 | +10 | 52 |
| V1 H3 MP-a +3,47 m gleichmäßig | H3 | +8 | +4 | +23 | −7 | +19 | +20 | +56 | +18 | 56 |
| V1 Schablonenlängen MP-a | H3 | +44 | +40 | +55 | +23 | +49 | +50 | +87 | +50 | 87 |
| V1 Schablonenlängen MP-d | H3 | +45 | +41 | +56 | +22 | +46 | +43 | +74 | +47 | 74 |
| V2a H3 MP-a | H3 | +7 | +3 | +65 | +81 | +119 | +60 | +87 | +60 | 119 |
| V2a H3 MP-b | H3 | −20 | −38 | +26 | +48 | +99 | +50 | +68 | +33 | 99 |
| V2a H3 MP-a +3,47 m gleichmäßig | H3 | −86 | −120 | −63 | −47 | +10 | −10 | −26 | −49 | 120 |
| V2b H3 MP-a | H3 | +69 | +109 | +108 | +47 | +89 | +85 | +117 | +89 | 117 |
| V2b H3 MP-a +3,47 m gleichmäßig | H3 | −58 | −14 | +1 | −60 | −22 | −20 | +4 | −24 | 60 |
| V1 H1 MP-a | H1 | +149 | +343 | +132 | +208 | +122 | +124 | +130 | +173 | 343 |
| V1 H2 MP-a | H2 | +304 | +287 | +136 | +206 | +149 | +127 | +189 | +200 | 304 |
| LumiViz heute (Nachbau) | H3 | +269 | +203 | +218 | +192 | +232 | +240 | +274 | +233 | 274 |

Die FEM-Zeile ist aus Tab. 1 gerechnet [eigene Rechnung aus wörtlich]. Bei Horn 1 und 2 liegen schon die gemessenen Töne
bis −238 Cent neben der Zielskala (Tab. 1); diese Hörner trennen die Varianten nicht.

**Spitzen V1 H3 MP-a** (Grundfall; V2a, V2b, Verlängerung und LumiViz-Nachbau im Anhang):

| n | f (Hz) | \|Z\| (MPa·s/m³) | Q | Cent A | Cent B | Cent gg. Tab. 1 |
|---|---|---|---|---|---|---|
| 1 | 48,69 | 12,1 | 52 | +89 | +46 | |
| 2 | 94,84 | 21,8 | 61 | +43 | 0 | |
| 3 | 148,36 | 28,6 | 69 | +116 | +73 | |
| 4 | 199,25 | 33,2 | 73 | +129 | +85 | +129 |
| 5 | 251,15 | 33,7 | 74 | +143 | +100 | +122 |
| 6 | 301,17 | 31,1 | 69 | +142 | +98 | +139 |
| 7 | 351,02 | 25,8 | 61 | +140 | +97 | +104 |
| 8 | 400,01 | 25,4 | 58 | +135 | +92 | +130 |
| 9 | 450,04 | 26,2 | 57 | +135 | +92 | +132 |
| 10 | 500,78 | 28,7 | 59 | +138 | +94 | +170 |
| 11 | 551,21 | 29,8 | 59 | +139 | +96 | |
| 12 | 602,38 | 31,9 | 60 | +142 | +99 | |
| 13 | 653,03 | 33,5 | 61 | +143 | +100 | |
| 14 | 703,90 | 35,6 | – | +145 | +101 | |
| 15 | 753,92 | 36,3 | – | +144 | +101 | |
| 16 | 803,76 | 33,7 | – | +143 | +100 | |

**Spitzen V1 H3 MP-a, gleichmäßig auf 3,47 m verlängert:**

| n | f (Hz) | \|Z\| (MPa·s/m³) | Q | Cent A | Cent gg. Tab. 1 |
|---|---|---|---|---|---|
| 1 | 45,46 | 11,9 | 49 | −30 | |
| 2 | 88,52 | 21,3 | 59 | −76 | |
| 3 | 138,35 | 28,0 | 68 | −5 | |
| 4 | 185,87 | 32,5 | 73 | +8 | +8 |
| 5 | 234,53 | 33,6 | 75 | +24 | +4 |
| 6 | 281,72 | 31,6 | 73 | +26 | +23 |
| 7 | 329,18 | 26,6 | 67 | +29 | −7 |
| 8 | 375,15 | 24,9 | 61 | +24 | +19 |
| 9 | 421,77 | 24,7 | 59 | +23 | +20 |
| 10 | 468,92 | 26,8 | 60 | +24 | +56 |
| 11 | 516,20 | 27,4 | 61 | +25 | |
| 12 | 564,15 | 29,4 | 62 | +28 | |
| 13 | 611,84 | 30,6 | 63 | +30 | |
| 14 | 659,68 | 33,1 | 63 | +32 | |
| 15 | 706,87 | 34,9 | 63 | +32 | |
| 16 | 754,25 | 34,8 | – | +33 | |

**Spieltöne gegen Spitzen:** Wie weit und in welche Richtung der Spielton neben der Spitze liegt, sagt keine lokale Quelle
in Zahlen. Marcinnò S. 14 nennt nur den Fangbereich („lock within a finite range around a resonance“) und den Befund,
dass die gemessenen Töne „significantly closer to the target frequencies“ liegen als die FEM-Spitzen [wörtlich]. Die Spalte
„Cent gg. Tab. 1“ ist daher **Spitze minus Spielton**, nicht der Fehler eines Spielmodells.

**Welche Variante die Messung trägt (Befund der Rechnung, kein Entscheid)** `[eigene Einschätzung aus eigener Rechnung]`:
- **V1 trägt die Messung, wenn das Horn wirksam ≈ 3,41–3,47 m lang ist.** Mit 3,47 m liegen die Spitzen 4–9 auf
  −51…+23 Cent an den Spieltönen, Naturton 10 auf +48…+56 (sein Spielton liegt selbst −53 Cent unter dem Zielton). Die
  Reihe 3–16 bleibt bei gleichmäßiger Verlängerung in 38 Cent Spanne.
- **Mit den Längen aus Tab. 3 (3,24 m) trägt V1 die Messung nicht:** +132 Cent im Mittel. Das teilt V1 mit Marcinnòs
  eigener FEM (+95). Die Lücke ist also keine Folge des 1D-Ansatzes: Tab. 3 und Becher ergeben ein Horn, das gut einen
  Halbton zu hoch klingt.
- **V2 trägt die Messung nicht.** V2a trifft zwar zufällig die Naturtöne 4 und 5 (+7 / +3), die Reihe ist aber
  unregelmäßig (Spanne 137 Cent), Naturton 8 liegt bei +119; auf 3,47 m verlängert liegt Naturton 5 bei −120.

### 4.3 Z3: Der LumiViz-Befund „≈ 3 Halbtöne zu hoch, nicht harmonisch“

Nachbau nach `ic/modell.md` Z. 40: R1 = 2,75 mm; Kegel 1–3 auf 16 / 28,5 / 44 mm; Kegel 4 als Zylinder r 44; Becher
0,5 m von 44 auf 90 mm; ohne Mundstück. Die Becherform steht dort nicht; gerechnet sind Kegel und exponentieller Verlauf.

| Schritt | Länge (mm) | Cent Spitze 2 (A) | Mittel 2–16 (A) | mittl. Betrag (B) | max. Betrag (B) |
|---|---|---|---|---|---|
| 0 LumiViz heute (Nachbau) | 3085 | +207 | +242 | 35 | 97 |
| 1 + R1 = 6,5 mm | 3085 | +151 | +210 | 59 | 90 |
| 2 + Kegel 4 44 → 51,5, Becher 51,5 → 90 über 0,5 m | 3085 | +164 | +206 | 42 | 75 |
| 3 + Becher nach Abb. 3b (Bogen 589,5 mm) | 3174 | +52 | +163 | 111 | 142 |
| 4 + Mundstück a (= V1 H3 MP-a) | 3244 | +43 | +132 | 88 | 101 |
| 5 + gleichmäßig auf 3,47 m | 3470 | −76 | +17 | 93 | 109 |

**Befund** `[eigene Rechnung]`:
- Der Nachbau gibt Spitze 2 bei 104,3 Hz: +207 Cent gegen 92,5 Hz (Fis2), +216 Cent gegen 92 Hz (Marcinnòs Zielton). Die
  übrigen Spitzen liegen +221…+304 Cent zu hoch, Spitze 3 bei +304. „Rund drei Halbtöne“ trifft für Spitze 3 genau; im
  Mittel sind es 2,4 Halbtöne.
- „Nicht harmonisch“: Spitze 3 liegt 97 Cent über der Reihe auf Spitze 2.
- LumiViz nennt +186 Cent für Spitze 2 (`modell.md` Z. 68), 21–30 Cent weniger als der Nachbau; der Unterschied ist
  ungeklärt (Abschnitt 7).
- **Der Befund ist reproduziert.** Ursachen: Die Bohrung ist zu kurz (Becher-Bogenlänge, Mundstück, Restlänge zu
  3,47 m), R1 ist zu eng, und der Becher setzt bei 44 statt bei 51,5 mm an.

### 4.4 Empfindlichkeit (Cent je +1 %, zentrale Differenz ±1 %), Spitzen 2–16

Definitionen: **R1** — Radius am Anfang von Kegel 1, die Rückbohrung endet mit; **Kesselvolumen** — Kesselradien × √1,01;
**Rückbohrung** — Länge +1 % (Volumen ≈ +1 %); **Becher-Randradius** — Radien ab der Aufweitung gleitend bis +1 % am Rand;
**Länge fast zylindrischer Teil** — Teil bis s = 307,9 mm gestreckt, Rest verschoben; **Aufweitungsstelle** — wandert
um +1 % bei fester Becherlänge; **Becher-Bogenlänge** — ganzer Becher; **Gesamtlänge** — L1–L4 und Becher; **alle Radien**
— R1 bis zum Becherrand.

**V1 H3 MP-a**

| Größe | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 | Spanne |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| R1 | −1,1 | −1,3 | −1,1 | −0,6 | −0,2 | −0,1 | 0,0 | +0,1 | +0,2 | +0,3 | +0,4 | +0,5 | +0,5 | +0,5 | +0,4 | 1,8 |
| Kesselvolumen | −0,1 | −0,1 | −0,1 | −0,1 | −0,1 | −0,1 | −0,2 | −0,2 | −0,2 | −0,2 | −0,3 | −0,3 | −0,4 | −0,4 | −0,4 | 0,3 |
| Rückbohrung | −0,1 | −0,1 | −0,1 | −0,1 | −0,1 | −0,1 | −0,2 | −0,2 | −0,2 | −0,2 | −0,3 | −0,3 | −0,3 | −0,4 | −0,4 | 0,3 |
| Becher-Randradius | +1,0 | +0,9 | +0,8 | +0,8 | +0,6 | +0,4 | +0,1 | −0,1 | −0,2 | −0,2 | −0,2 | −0,2 | −0,2 | −0,2 | −0,2 | 1,3 |
| Länge fast zyl. Becherteil | −1,9 | −1,6 | −1,5 | −1,5 | −1,5 | −1,6 | −1,6 | −1,5 | −1,5 | −1,6 | −1,6 | −1,6 | −1,6 | −1,5 | −1,5 | 0,4 |
| Aufweitungsstelle | −0,6 | −0,4 | −0,2 | 0,0 | +0,3 | +0,5 | +0,5 | +0,3 | +0,1 | −0,1 | −0,1 | −0,1 | −0,1 | 0,0 | +0,2 | 1,1 |
| Becher-Bogenlänge | −3,0 | −2,8 | −2,7 | −2,9 | −3,2 | −3,6 | −3,4 | −3,2 | −3,0 | −3,0 | −3,0 | −3,0 | −2,9 | −2,9 | −3,1 | 0,9 |
| Gesamtlänge | −17,2 | −17,1 | −17,0 | −16,9 | −16,8 | −16,7 | −16,6 | −16,6 | −16,5 | −16,4 | −16,4 | −16,3 | −16,1 | −16,0 | −16,0 | 1,2 |
| alle Radien | +0,1 | +0,2 | +0,2 | +0,2 | +0,2 | +0,1 | +0,1 | +0,1 | +0,2 | +0,2 | +0,3 | +0,3 | +0,3 | +0,3 | +0,2 | 0,2 |

**V2a H3 MP-a**

| Größe | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 | Spanne |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| R1 | −1,1 | −0,7 | −0,4 | +0,4 | +0,9 | +0,9 | +0,6 | +0,4 | +0,7 | +0,6 | +0,4 | +0,1 | 0,0 | +0,1 | 0,0 | 2,1 |
| Kesselvolumen | −0,2 | −0,2 | −0,4 | −0,6 | −0,6 | −0,5 | −0,3 | −0,2 | −0,4 | −0,3 | −0,3 | −0,2 | −0,1 | −0,1 | −0,1 | 0,5 |
| Rückbohrung | −0,1 | −0,1 | −0,2 | −0,3 | −0,3 | −0,3 | −0,3 | −0,2 | −0,3 | −0,3 | −0,3 | −0,2 | −0,1 | −0,2 | −0,3 | 0,3 |
| Becher-Randradius | +0,8 | +2,1 | +1,5 | +0,4 | +0,2 | +0,2 | +0,3 | 0,0 | −0,1 | 0,0 | 0,0 | 0,0 | −0,7 | −0,2 | 0,0 | 2,8 |
| Länge fast zyl. Becherteil | −1,4 | −3,7 | −2,8 | −0,8 | −0,4 | −0,6 | −2,1 | −3,4 | −0,9 | −0,5 | −0,6 | −1,4 | −4,6 | −1,3 | −0,5 | 4,2 |
| Aufweitungsstelle | −0,4 | −0,9 | −0,5 | 0,0 | +0,1 | +0,2 | +0,7 | +1,0 | +0,1 | 0,0 | 0,0 | −0,1 | −0,2 | 0,0 | 0,0 | 1,9 |
| Becher-Bogenlänge | −2,2 | −6,3 | −4,8 | −1,5 | −0,9 | −1,3 | −4,8 | −7,5 | −1,8 | −0,9 | −1,0 | −2,6 | −8,6 | −2,5 | −1,0 | 7,7 |
| Gesamtlänge | −17,0 | −16,7 | −16,5 | −16,3 | −16,2 | −16,2 | −16,3 | −16,2 | −16,3 | −16,5 | −16,6 | −16,8 | −16,1 | −16,6 | −16,7 | 0,9 |
| alle Radien | +0,7 | +0,6 | +0,9 | +1,6 | +1,6 | +1,3 | +0,5 | −0,2 | +0,6 | +0,5 | +0,4 | +0,1 | −0,9 | −0,1 | 0,0 | 2,5 |

**Daraus** `[eigene Einschätzung aus eigener Rechnung]`:
- **Je Prozent** bestimmt bei V1 die **Gesamtlänge** die Lage: −16,6 Cent/%, über n fast gleich (Spanne 1,2).
- **Die Harmonizität** (Spanne über n) hängt je Prozent am meisten von **R1** ab (1,8 Cent/%), dann vom
  **Becher-Randradius** (1,3) und der **Aufweitungsstelle** (1,1) — je Prozent kleine Werte.
- Über die **wirklichen Unsicherheiten** gerechnet (Abschnitt 5) dominieren dagegen:
  - die **Rückbohrung**: 20 → 70 mm verschiebt Spitze 2 um −5 und Spitze 16 um −44 Cent;
  - der **Becher über x statt über die Bogenlänge**: +37 bis +67 Cent, am stärksten bei n7–n9;
  - der **Abstrahlungsansatz**: −5 bis −16 Cent.
- **Bei V2a** reagieren die Spitzen sprunghaft (Becher-Bogenlänge −0,9 bis −8,6 Cent/%): das Bild einer Bohrung mit einer
  Querschnittsstufe (× 4 in der Fläche), in der Teilresonanzen zwischen Sprung und Becher die Reihe stören.

### 4.5 Littleton-Horn nach Young 2009

**Nicht gerechnet.** Die Stufe 1 liefert die Bohrung nicht (PDF gesperrt, nur Abstract gelesen; Nachprüfung 2, A2). Ein
Ersatz ist nach dem Auftrag nicht zulässig; die Cent-Tabelle aus dem Abstract lässt sich ohne Bohrung nicht gegenrechnen.

### 4.6 Krümmung

- **Formel:** keine in den lokalen Quellen (Wortlaut der Fundstellen in Abschnitt 1 unter G12).
- **Annahme:** Die Bohrung läuft über die Bogenlänge der Mittellinie.
- **Zahlenbeispiel** (V1 H3 MP-a, Cent gegen den Grundfall mit 589,5 mm Bogen):

| Fall | Gesamtlänge (mm) | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Becher über x (492 mm) statt Bogen | 3147 | +36,6 | +35,0 | +37,3 | +42,1 | +49,7 | +61,7 | +67,0 | +67,4 | +63,2 | +61,3 | +57,1 | +54,7 | +51,4 | +49,6 | +49,6 |
| Bogen +5 % | 3274 | −15,1 | −13,7 | −13,6 | −14,6 | −16,0 | −18,1 | −17,0 | −15,9 | −15,0 | −15,0 | −14,8 | −14,7 | −14,5 | −14,7 | −15,6 |
| Bogen −5 % | 3215 | +15,3 | +13,9 | +13,8 | +14,5 | +15,9 | +18,1 | +17,5 | +16,4 | +15,3 | +15,2 | +14,9 | +14,9 | +14,6 | +14,5 | +15,2 |

- **Größenordnung:** Bogen gegen x: 37–67 Cent. Unsicherheit der Bogenlänge ±5 % → ±14–18 Cent; nach der Nachprüfung
  (±1 %) → ±3 Cent. Einen Krümmungseffekt über die wirksame Länge hinaus beziffern die lokalen Quellen nicht; Soares et
  al. halten ihn „for sufficiently smooth bents in relatively narrow ducts“ für vernachlässigbar.
- **Grenze:** Der Becher ist weder eng noch am Ende sanft gebogen (Mittellinie steigt zwischen x = 400 und 492 mm um
  140 mm). Ob die Aussage dort gilt, ist offen `[eigene Einschätzung]`.

## 5. Annahmen, die bleiben

| Annahme | gerechnete Spanne | Wirkung (Cent, Spitzen 2–16) | Marke |
|---|---|---|---|
| Länge der Rückbohrung | 20 / 40 / 70 mm | 20 mm: +2…+17; 70 mm: −3…−28 (wächst mit n) | [eigene Rechnung] |
| Lesart „Kesseltiefe 25 mm“ | a gegen b | b gegen a: Mittel 2–16 +117 statt +132 | [eigene Rechnung] |
| Mundstück als Profil oder Volumen | a gegen c | c gegen a: 0…+2 (Mittel +133 gegen +132) | [eigene Rechnung] |
| ohne Mundstück | 0 gegen a | Mittel +163 statt +132 | [eigene Rechnung] |
| Ort der Verlängerung auf 3,47 m | Mundrohr / Kegel 1 / gleichmäßig | Tab. 1 im Mittel −12 / +10 / +18; Spitze 2: −7 / −70 / −76 | [eigene Rechnung] |
| ob 3,47 m mit Mundstück gemessen ist | ±70 mm | ≈ ±35 (−16,6 Cent/% × 2,0 %) | [eigene Rechnung] |
| Becher über Bogen statt x | Bogen / x | x: +37…+67 | [eigene Rechnung] |
| Bogenlänge | ±5 % (Nachprüfung: ±1 %) | ∓14…18 (±1 %: ∓3) | [eigene Rechnung] |
| Abstrahlung | ungeflanscht / geflanscht | geflanscht: −5…−16 | [eigene Rechnung] |
| Wandverluste nach Hendrie (glatte Wand) | mit / ohne | ohne: +15 (n2) … +5 (n16). Holzwand mit erhöhten Verlusten nicht gerechnet, kein Faktor belegt | [eigene Rechnung] |
| Lufttemperatur | 14,5 / 20 / 30 °C | −16 / 0 / +29, gleichmäßig über n (2,9 Cent/K) | [eigene Rechnung] |
| R1 = R_min | ±1 % | −1,3…+0,5 Cent je % | [eigene Rechnung] |
| Schablonenlängen statt Tab. 3 | L2 940, L3 825 | Tab. 1 im Mittel +50 statt +132 | [abgelesen] + [eigene Rechnung] |

## 6. Optionen für LumiViz

Alle Zahlen aus der eigenen Rechnung (20 °C, Wandverluste, ungeflanscht). Z1 = Mittel / größter Betrag gegen Tab. 1
(Horn 3, n4–10); Z2 = Mittel / Spanne 3–16 gegen n·46,249 Hz.

| Option | braucht an Daten | Z1 (Tab. 1, H3) | Z2 (Reihe, Bezug A) | Z3 (LumiViz-Befund) |
|---|---|---|---|---|
| **O1 V1 wie belegt** (Tab. 3 + R1/R5 + Becher Abb. 3b + Mundstück a), 3,24 m | Abschnitt 2; Mundstück und Rückbohrung als Annahme | +132 / 170 | +132 / 29; Spitze 2 +43 | von +242 bleiben +132; Spitze 2 bleibt 75–100 Cent unter der Reihe |
| **O2 V1 auf 3,47 m** (gleichmäßig / Kegel 1 / Mundrohr) | dazu: wo die ≈ 226 mm sitzen, ob 3,47 m mit Mundstück | +18 / 56 · +10 / 52 · −12 / 51 | +17 / 38 · +12 / 63 · +9 / 65; Spitze 2 −76 / −70 / −7 | Lage erklärt (+242 → +9…+17) |
| **O3 V1 mit Schablonenlängen** (L2 940, L3 825; 3,41 m) | Lesart der Handschrift Abb. 3a als Kegellängen [abgelesen, unsicher] | +50 / 87 (MP-d: +47 / 74) | +48 / 35; Spitze 2 −51 | Lage zu rund 4/5 erklärt |
| **O4 V2 Durchmesser-Lesart** (V2a mit Sprung / V2b ohne) | nichts Neues, aber gegen den Wortlaut S. 7 und die Schablone (Nachprüfung A1) | +60 / 119 · +89 / 117; mit 3,47 m −49 / 120 · −24 / 60 | Spanne 137 / 117; mit 3,47 m 151 / 111 | Lage teils erklärt, Unharmonizität größer als heute |
| **O5 Mundstück als Volumen + Verengung** (zu O1–O3) | V = 3,2 cm³, Verengung, Rückbohrung | gegen Profil a: 0…+2 Cent | dto. | wie O1–O3 |

**Befund der Rechnung, kein Entscheid:** Nur V1 gibt eine regelmäßige Reihe. Die gemessenen Spieltöne trifft V1 erst bei
einer wirksamen Länge von 3,41–3,47 m. Welche Länge belegt ist, entscheidet die Rechnung nicht: Tab. 3 + Becher ergeben
3,17 m [wörtlich/abgelesen], die Herstellerlänge 3,43–3,47 m [Werkzeug], die Schablone +165 mm in L2/L3 [abgelesen,
Zuordnung unsicher]. Auch Marcinnòs eigene FEM liegt bei der Länge aus Tab. 3 +95 Cent über den gemessenen Tönen.

## 7. Was die LumiViz-Sitzung prüfen muss

1. **Gleiche Bohrung, gleiche Spitzen?** Den LumiViz-Kern mit der Segmentliste V1 H3 MP-a (Anhang) rechnen und mit 4.2
   vergleichen; bei gleicher Physik (Hendrie-Verluste, ungeflanscht, 20 °C) sind wenige Cent zu erwarten. Ebenso den
   Nachbau „LumiViz heute“: LumiViz meldet +186 Cent für Spitze 2, der Nachbau +207 (gegen 92,5 Hz) bzw. +216 (gegen
   92 Hz). Zu klären: Becherform für „0,5 m 44 → 90“, Verlustmodell, Temperatur, Bezug.
2. **Kegel mit Kugelwellen oder Zylinderstufen?** Mit Zylinderstufen prüfen, wie fein sie sein müssen.
3. **Becher über die Bogenlänge oder über x?** Unterschied 37–67 Cent (4.6).
4. **Bezug der Cent-Werte im Kern:** 92 Hz (Marcinnò-Ziel), 92,5 Hz (Fis2 bei 440) oder die eigene Reihe?
5. **Spielfrequenz gegen Spitze im Lippenmodell:** Wie weit verschiebt das LumiViz-Lippenmodell den Spielton gegen die
   Spitze? Keine lokale Quelle nennt eine Zahl. Die +95 Cent zwischen FEM-Spitzen und gemessenen Tönen (Tab.-3-Länge)
   sind so groß, dass ein Lippenmodell sie nicht erklären muss.
6. **Spitze 2 liegt tief** (in allen V1-Varianten 75–110 Cent unter der Reihe der übrigen Spitzen, in der FEM ebenso).
   Prüfen, ob die Hörprobe auf dem zweiten Naturton dazu passt.
7. **Hendrie (2.102)** nur mit dem Faktor 1/(sin θ₁ sin θ₂) verwenden, falls der Kern sie nutzt (K3).
8. Unabhängig von der Rechnung: die Zuordnung der Schablonenlängen (O3) und die Lesart der Herstellerlänge (mit oder ohne
   Mundstück).

## 8. Nicht gefunden / nicht herleitbar

| Lücke | Suchweg | Ergebnis |
|---|---|---|
| Bohrung Littleton-Horn (Young 2009) | Stufe 1 (beide Berichte); `eingang/2026-11-16_07_alphorn_nachpruefung/` (nur Marcinnò, Wachter) | fehlt, PDF gesperrt |
| Krümmungsformel | `pdf_suche.py` in allen fünf lokalen PDFs und Marcinnò: „bend“, „curv“, „curved“, „bends“, „Félix“, „Nederveen“; Seitenbilder Soares S. 3–4 | nur qualitativ (Soares S. 3–4; Resch S. 5); Formelquelle [25] Félix et al. 2012 nicht lokal |
| Kegelmatrix mit Verlusten | Hendrie: „transfer matrix“ (0 Treffer), „conical“, „spherical wave“; Petiot: „transfer matri“ (nur Nennung, keine Formel); Mattéoli, Resch: 0 | nur verlustfrei (2.102/2.103); Verluste in Kegeln laut Hendrie S. 43 „not easily accounted for“ → eigene Scheiben |
| Abstrahlung für ka ≈ 1 (Levine–Schwinger o. ä.) | Hendrie „radiation impedance“ (24 Treffer; gelesen S. 34, 37, 39), „Levine“ (0) | nur Tieffrequenzformeln G8/G9 |
| Abstand Spielton–Impedanzspitze in Zahlen | Marcinnò „player“, „lips“, „discrepanc“; Petiot „playing frequenc“ (S. 3, 6, 7, nur Textebene) | nur qualitativ (Marcinnò S. 14). Petiot S. 6 nennt „+30 cents“ zwischen gerechneter und gemessener Trompete; Zusammenhang nicht am Seitenbild geprüft [Werkzeug] |
| Mundstückmaße Marcinnò (R_inlet, R0, Länge) | Stufe 1; Marcinnò S. 3, 5, 7 | nicht veröffentlicht |
| Wandverluste Holz | Stufe 1; `modell-entwurf.md` Z. 43 („Faktor β, nur Bericht“) | keine Zahl |
| Viskothermik bei Marcinnò | S. 15 [wörtlich]: als künftige Arbeit genannt | FEM verlustfrei |

## 9. Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Lizenz / Vermerk | wozu |
|---|---|---|---|
| `Felix_Dalmont_Nederveen_2012_bends.pdf` | JASA 131 (2012) 4164–4172 (Zitat Soares S. 15); DOI nicht in der Quelle, **nicht geprüft** | Verlag ASA; vermutlich Bezahlschranke | Formel für die Verstimmung durch Bögen (Punkt 5) |
| `Young_2009_alphorn_impedance.pdf` | https://doi.org/10.1121/1.3187533 | © ASA; Cloudflare-Sperre (Nachprüfung) | Littleton-Bohrung für Punkt 4 |
| `Chaigne_Kergomard_2016_Acoustics_of_Musical_Instruments` | Springer (Buch); ISBN nicht geprüft | Kauf | belegte Kegelmatrix mit Verlusten und Abstrahlung bei ka ≈ 1 statt der eigenen Herleitung G6 und G8/G9 |
| `Ernoult_Kergomard_truncated_cone.pdf` | Soares Lit. [32] „Transfer matrix of a truncated cone …“ (Fundstelle dort nicht ganz gelesen) | ungeprüft | Kegelmatrix mit Verlusten aus der Literatur |

Die Mundstückmaße von den Autoren (R_inlet, R0, Länge) und die Schablonenlängen von der Werkstatt Fehlmann wären der
kürzeste Weg, O2 und O3 zu trennen. Nur genannt; keine Anfrage gestellt.

## 10. Was der Auftrag verlangt hat und nicht geliefert ist

- **Punkt 4 (Littleton):** entfällt, die Bohrung fehlt.
- **Punkt 5 (Krümmungsformel):** keine lokale Formel; gerechnet sind die Annahme „Bogenlänge“ und ihre Spannen.
- **Q der hohen Spitzen:** teils nicht bestimmt (Suchfenster); Frequenzen und Höhen sind vollständig.
- **Am Seitenbild geprüft:** alle Hendrie-Gleichungen G2–G4, G6a/b, G7–G9 und Marcinnò G12. G1 und G5 nur über die
  Textebene (einfach, nicht gegengeprüft).
- Der Lauf wurde einmal per Haltepunkt unterbrochen; die Rechnung lief vorher vollständig, nach dem Fortsetzen ist nichts
  neu gerechnet außer `pruefung.py` und `zusatz.py` für den Anhang.


## Anhang: Skripte und Ausgaben


### `tmm.py`

```python
"""Eingangsimpedanz einer Bohrung aus Kegel-/Zylindersegmenten (Uebertragungsmatrix, Kugelwellen im Kegel).

Gleichungen (Nummern wie im Bericht):
  G1  c = sqrt(gamma R T / m)                         Hendrie 2007, S. 13, (2.2)
  G2  r_v = 632.8 a sqrt(f) (1 - 0.0031 dT)            Hendrie S. 28, (2.73)
      r_t = 532.8 a sqrt(f) (1 - 0.0031 dT)            Hendrie S. 28, (2.74)
  G3  v = c [1 - 1/(sqrt2 r_v) - (gamma-1)/(sqrt2 r_t)] Hendrie S. 28, (2.75)
      alpha = (w/c) [1/(sqrt2 r_v) + (gamma-1)/(sqrt2 r_t)]  Hendrie S. 29, (2.76)
  G4  Gamma = w/v - i alpha (ersetzt k)                Hendrie S. 29, (2.79)
  G5  Zc = rho c / S                                   Hendrie S. 21, (2.45)
  G6  Kegel: p = (A e^{-iGx} + B e^{iGx}) / x, x vom Apex; Matrix aus p und U = S u,
      i w rho u = -dp/dx  (eigene Herleitung, geprueft gegen Hendrie (2.102)/(2.103))
  G7  Zylinder: Hendrie S. 39, (2.93)
  G8  Abstrahlung ungeflanscht Z = Zc[(ka)^2/4 + i 0.6 ka]   Hendrie S. 37, (2.91)
  G9  Abstrahlung geflanscht  Z = Zc[(ka)^2/2 + i 8/(3 pi) ka] Hendrie S. 34, (2.86)
Zeitkonvention e^{+i w t} wie bei Hendrie.
"""
import numpy as np

GAMMA = 1.4
R_GAS = 8.314462
M_AIR = 0.0289647
P0 = 101325.0


def luft(T_C=20.0, c=None):
    T = T_C + 273.15
    c_ = np.sqrt(GAMMA * R_GAS * T / M_AIR) if c is None else c   # G1
    rho = P0 * M_AIR / (R_GAS * T)
    return dict(T=T, c=c_, rho=rho)


def gamma_k(f, a, luft_, verlust=True):
    """Ausbreitungskonstante (G2-G4); a in m."""
    w = 2 * np.pi * f
    c = luft_["c"]
    if not verlust:
        return w / c + 0j
    dT = luft_["T"] - 300.0
    rv = 632.8 * a * np.sqrt(f) * (1 - 0.0031 * dT)
    rt = 532.8 * a * np.sqrt(f) * (1 - 0.0031 * dT)
    v = c * (1 - 1 / (np.sqrt(2) * rv) - (GAMMA - 1) / (np.sqrt(2) * rt))
    alpha = (w / c) * (1 / (np.sqrt(2) * rv) + (GAMMA - 1) / (np.sqrt(2) * rt))
    return w / v - 1j * alpha


def _matrix_kegel(G, x, S, w, rho):
    e_m = np.exp(-1j * G * x)
    e_p = np.exp(1j * G * x)
    m11 = e_m / x
    m12 = e_p / x
    m21 = S * e_m * (1 + 1j * G * x) / (1j * w * rho * x * x)
    m22 = S * e_p * (1 - 1j * G * x) / (1j * w * rho * x * x)
    return m11, m12, m21, m22


def _schritt(Z2, f, L, r1, r2, luft_, verlust):
    """Impedanz am Eingang eines Segments (L, r1 -> r2, alles in m) bei Last Z2."""
    w = 2 * np.pi * f
    rho = luft_["rho"]
    a = 0.5 * (r1 + r2)
    G = gamma_k(f, a, luft_, verlust)
    if abs(r2 - r1) < 1e-4 * a:      # Zylinder (G7)
        # Zc = rho w / (Gamma S): im verlustfreien Fall = rho c / S (G5); mit Verlusten derselbe
        # Wellenwiderstand, den die Kegelmatrix (G6) im Grenzfall x -> unendlich liefert (eigene Wahl,
        # damit Zylinder- und Kegelscheiben ohne Scheinreflexion aneinanderpassen; Wirkung ~1e-3).
        Zc = rho * w / (G * np.pi * a * a)
        t = np.tan(G * L)
        return Zc * (Z2 + 1j * Zc * t) / (1j * Z2 * t + Zc)
    x1 = r1 * L / (r2 - r1)
    x2 = x1 + L
    S1 = np.pi * r1 * r1
    S2 = np.pi * r2 * r2
    a11, a12, a21, a22 = _matrix_kegel(G, x1, S1, w, rho)
    b11, b12, b21, b22 = _matrix_kegel(G, x2, S2, w, rho)
    det = b11 * b22 - b12 * b21
    i11, i12, i21, i22 = b22 / det, -b12 / det, -b21 / det, b11 / det
    t11 = a11 * i11 + a12 * i21
    t12 = a11 * i12 + a12 * i22
    t21 = a21 * i11 + a22 * i21
    t22 = a21 * i12 + a22 * i22
    return (t11 * Z2 + t12) / (t21 * Z2 + t22)


def z_abstrahlung(f, a, luft_, art="unflanged"):
    k = 2 * np.pi * f / luft_["c"]
    Zc = luft_["rho"] * luft_["c"] / (np.pi * a * a)
    ka = k * a
    if art == "unflanged":
        return Zc * (ka * ka / 4 + 1j * 0.6 * ka)          # G8
    if art == "flanged":
        return Zc * (ka * ka / 2 + 1j * 8 / (3 * np.pi) * ka)  # G9
    if art == "offen":
        return np.zeros_like(f) + 0j
    raise ValueError(art)


def zin(segmente_mm, f, luft_, verlust=True, abstrahlung="unflanged", scheibe_mm=10.0, lumped=None):
    """segmente_mm: Liste (L, r_ein, r_aus) in mm vom Mundstueck zum Becher.
    lumped: optional (V_mm3, l_eff_mm, r_eng_mm) = Kesselvolumen parallel + Verengung in Reihe am Eingang."""
    f = np.asarray(f, dtype=float)
    r_end = segmente_mm[-1][2] * 1e-3
    Z = z_abstrahlung(f, r_end, luft_, abstrahlung)
    for L, r1, r2 in reversed(segmente_mm):
        n = max(1, int(np.ceil(L / scheibe_mm))) if verlust else 1
        rs = np.linspace(r1, r2, n + 1) * 1e-3
        dl = L * 1e-3 / n
        for j in reversed(range(n)):
            Z = _schritt(Z, f, dl, rs[j], rs[j + 1], luft_, verlust)
    if lumped is not None:
        V, l_eff, r_eng = lumped
        w = 2 * np.pi * f
        C = V * 1e-9 / (luft_["rho"] * luft_["c"] ** 2)
        M = luft_["rho"] * l_eff * 1e-3 / (np.pi * (r_eng * 1e-3) ** 2)
        Z = 1 / (1j * w * C + 1 / (1j * w * M + Z))
    return Z


def spitzen(segmente_mm, luft_, verlust=True, abstrahlung="unflanged", fmax=900.0, lumped=None, n_max=16):
    """Impedanzspitzen: Frequenz (Hz), |Z| (MPa s/m^3 = 1e6 Pa s m^-3), Guete Q (-3 dB)."""
    fg = np.arange(15.0, fmax, 0.2)
    Zg = np.abs(zin(segmente_mm, fg, luft_, verlust, abstrahlung, lumped=lumped))
    idx = [i for i in range(1, len(fg) - 1) if Zg[i] > Zg[i - 1] and Zg[i] >= Zg[i + 1]]
    # schwache Nebenmaxima ausschliessen: Prominenz gegen das tiefere Nachbarminimum
    gut = []
    for i in idx:
        lo = Zg[max(0, i - 400):i].min() if i > 0 else Zg[i]
        hi = Zg[i + 1:i + 400].min() if i + 1 < len(Zg) else Zg[i]
        if Zg[i] > 1.15 * max(lo, hi):
            if gut and fg[i] - fg[gut[-1]] < 5.0:     # zwei Grob-Maxima derselben Spitze
                if Zg[i] > Zg[gut[-1]]:
                    gut[-1] = i
                continue
            gut.append(i)
    erg = []
    gut = gut[:n_max]
    for m, i in enumerate(gut):
        f0 = fg[i]
        nachbarn = [abs(fg[j] - f0) for j in (gut[m - 1] if m > 0 else None, gut[m + 1] if m + 1 < len(gut) else None)
                    if j is not None]
        breite = min([6.0, 0.06 * f0] + [0.45 * d for d in nachbarn])
        fl = np.linspace(f0 - breite, f0 + breite, 4001)
        Zl = np.abs(zin(segmente_mm, fl, luft_, verlust, abstrahlung, lumped=lumped))
        j = int(np.argmax(Zl))
        fp, zp = fl[j], Zl[j]
        h = zp / np.sqrt(2)
        Q = np.nan
        links = np.where(Zl[:j] < h)[0]
        rechts = np.where(Zl[j:] < h)[0]
        if len(links) and len(rechts):
            jl = links[-1]
            jr = j + rechts[0]
            fl1 = np.interp(h, [Zl[jl], Zl[jl + 1]], [fl[jl], fl[jl + 1]])
            fr1 = np.interp(h, [Zl[jr], Zl[jr - 1]], [fl[jr], fl[jr - 1]])
            Q = fp / (fr1 - fl1)
        erg.append((fp, zp / 1e6, Q))
    return erg


def cent(f, ref):
    return 1200 * np.log2(np.asarray(f) / np.asarray(ref))
```


### `pruefung.py`

```python
"""Kontrollen des Rechenwegs (Abschnitt 3 des Berichts): K1-K5."""
import numpy as np
from scipy.optimize import brentq
import tmm

print("== K5 Zahlenbeispiel Verluste: (2.73)/(2.74) in (2.75)/(2.76) -> Konstanten von (2.77)/(2.78)")
k_v = 1 / (np.sqrt(2) * 632.8) + 0.4 / (np.sqrt(2) * 532.8)
print(f"  1/(sqrt2*632.8) + 0.4/(sqrt2*532.8) = {k_v:.4e}   (Hendrie 2.77: 1.65e-3)")
print(f"  2*pi*{k_v:.4e}/345 = {2*np.pi*k_v/345:.3e}            (Hendrie 2.78: 3e-5, mit c = 345 m/s S. 25)")
L300 = tmm.luft(26.85)
print(f"  G1 bei 20 C: c = {tmm.luft(20)['c']:.2f} m/s, rho = {tmm.luft(20)['rho']:.4f} kg/m3;"
      f" bei 300 K: c = {L300['c']:.2f} m/s")

print("\n== K1 verlustfreier Zylinder r = 20 mm, L = 1 m, ideal offen (Z_L = 0): Spitzen bei (2n-1) c / 4L")
lu = tmm.luft(20)
seg = [(1000.0, 20.0, 20.0)]
sp = tmm.spitzen(seg, lu, verlust=False, abstrahlung="offen", fmax=1000, n_max=5)
for n, (f, z, q) in enumerate(sp, 1):
    soll = (2 * n - 1) * lu["c"] / 4
    print(f"  n={n}: TMM {f:9.4f} Hz  geschlossen {soll:9.4f} Hz  Diff {tmm.cent(f, soll):+.4f} Cent")

print("\n== K2 verlustfreier Kegelstumpf r 5 -> 50 mm, L = 1 m, ideal offen: Hendrie (2.103) -> sin(kL + atan(k x1)) = 0")
seg = [(1000.0, 5.0, 50.0)]
x1 = 5.0 * 1000.0 / 45.0 * 1e-3
sp = tmm.spitzen(seg, lu, verlust=False, abstrahlung="offen", fmax=1000, n_max=5)
for n, (f, z, q) in enumerate(sp, 1):
    g = lambda ff: np.sin(2 * np.pi * ff / lu["c"] * 1.0 + np.arctan(2 * np.pi * ff / lu["c"] * x1))
    soll = brentq(g, f - 15, f + 15)
    print(f"  n={n}: TMM {f:9.4f} Hz  (2.103) {soll:9.4f} Hz  Diff {tmm.cent(f, soll):+.4f} Cent")

print("\n== K3 Hendrie (2.102) mit Last (ungeflanscht, G8) gegen eigene Kegelmatrix, Kegel r 5 -> 50 mm, L = 1 m")
f = np.array([50.0, 137.0, 300.0, 555.0])
k = 2 * np.pi * f / lu["c"]
r1, r2, L = 5e-3, 50e-3, 1.0
x1 = r1 * L / (r2 - r1); x2 = x1 + L
t1, t2 = np.arctan(k * x1), np.arctan(k * x2)
Zc1 = lu["rho"] * lu["c"] / (np.pi * r1 ** 2); Zc2 = lu["rho"] * lu["c"] / (np.pi * r2 ** 2)
ZL = tmm.z_abstrahlung(f, r2, lu, "unflanged")
num = 1j * ZL * np.sin(k * L - t2) / np.sin(t2) + Zc2 * np.sin(k * L)
den_druck = ZL * np.sin(k * L + t1 - t2) - 1j * Zc2 * np.sin(k * L + t1) / np.sin(t1)
den_FR = ZL * np.sin(k * L + t1 - t2) / (np.sin(t1) * np.sin(t2)) - 1j * Zc2 * np.sin(k * L + t1) / np.sin(t1)
Z_druck = Zc1 * num / den_druck
Z_FR = Zc1 * num / den_FR
Z_tmm = tmm.zin([(1000.0, 5.0, 50.0)], f, lu, verlust=False, abstrahlung="unflanged")
for i in range(len(f)):
    print(f"  f={f[i]:6.1f}: |Z_TMM|={abs(Z_tmm[i]):.4e}  |(2.102) wie gedruckt|={abs(Z_druck[i]):.4e}"
          f"  rel.Abw {abs(Z_druck[i]-Z_tmm[i])/abs(Z_tmm[i]):.2e}  |mit 1/(sin t1 sin t2)|={abs(Z_FR[i]):.4e}"
          f"  rel.Abw {abs(Z_FR[i]-Z_tmm[i])/abs(Z_tmm[i]):.2e}")

print("\n== K4 Hendrie Abb. 2.12: verlustbehafteter Zylinder r = 2 cm, L = 1 m, ungeflanscht, c = 345 m/s")
lu345 = tmm.luft(26.85, c=345.0)   # Hendrie S. 25: c = 345 m/s; Verlustformeln bei 300 K
seg = [(1000.0, 20.0, 20.0)]
sp = tmm.spitzen(seg, lu345, verlust=True, abstrahlung="unflanged", fmax=2000, n_max=12)
Zc = lu345["rho"] * 345.0 / (np.pi * 0.02 ** 2)
for n, (f, z, q) in enumerate(sp, 1):
    print(f"  n={n:2d}: {f:8.2f} Hz  |Z/Zc| = {z*1e6/Zc:6.1f}  Q = {q:6.1f}   (2.92): {(2*n-1)*345/(4*(1+0.6*0.02)):8.2f} Hz")

print("\n== K4b wie K4, aber so wie Hendrie Abb. 2.12 rechnet: (2.92) = ideal offen bei L + 0.6 r, nur Wandverluste")
abgelesen = [None, 40, 31, 27, 24, 22, 20, 18, 17, 16, 15.5, 15]   # Abb. 2.12 oben, |Z/Zc|; 1. Spitze abgeschnitten (>50)
sp = tmm.spitzen([(1012.0, 20.0, 20.0)], lu345, verlust=True, abstrahlung="offen", fmax=2000, n_max=12)
for n, (f, z, q) in enumerate(sp, 1):
    zz = z * 1e6 / Zc
    print(f"  n={n:2d}: {f:8.2f} Hz  |Z/Zc| = {zz:5.1f} ({20*np.log10(zz):4.1f} dB)  abgelesen {abgelesen[n-1]}")
```


### `rechnung.py`

```python
"""Hauptrechnung: Varianten der Fis-Alphorn-Bohrung, Spitzen 1-16, Cent gegen Reihe und Tab. 1, Empfindlichkeiten.
Ausgabe als Markdown auf stdout und in ausgabe.md."""
import io
import sys
import numpy as np
import tmm

AUS = io.StringIO()


def p(*a):
    s = " ".join(str(x) for x in a)
    print(s)
    AUS.write(s + "\n")


# ---------------- Daten ----------------
F_FIS1 = 440.0 * 2 ** ((30 - 69) / 12)     # Fis1 bei a' = 440 Hz, gleichstufig = 46.249 Hz
TAB3 = {  # Marcinno Tab. 3 (S. 18): L1..L4, R2..R4 (mm)
    "H1": ([544.96, 844.87, 754.84, 444.94], [13.50, 28.50, 44.00]),
    "H2": ([534.95, 799.86, 804.85, 444.94], [13.50, 28.50, 44.00]),
    "H3": ([539.92, 844.91, 754.84, 444.94], [16.00, 28.50, 44.00]),
}
R1, R5 = 6.5, 51.5                           # Marcinno S. 7 (R_min, R_max)
TAB1_MEAS = {  # Marcinno Tab. 1 (S. 10), Naturton n=4..10 (Do1 Mi1 Sol Sib Do2 Re Mi2)
    "H1": [180.00, 204.00, 278.00, 310.00, 371.00, 417.00, 463.00],
    "H2": [165.00, 211.00, 278.00, 311.00, 366.00, 417.00, 448.00],
    "H3": [185.00, 234.00, 278.00, 330.50, 371.00, 417.00, 454.00],
}
TAB2_FEM_H3 = {2: 94.11, 3: 146.83, 4: 196.97, 5: 246.98, 6: 296.51, 7: 343.50, 8: 388.71, 9: 436.02, 10: 487.77,
               11: 537.15}   # Marcinno Tab. 2 (S. 15) "Reference alphorn"; 537.15 steht unter "Sol 556" (siehe Bericht)
# Becher, Abb. 3b (Nachpruefung Abschn. 3): x, y Mittellinie, r (mm); Aufweitung ab x = 305
BECHER = [(0, 0, R5), (100, 0.5, 51.5), (200, 8, 51.6), (300, 28, 51.8), (305, 30.5, 51.8),
          (350, 53, 57.0), (400, 92, 62.0), (450, 158, 71.0), (492, 232, 89.5)]


def becher_punkte(bogen=True, skala_s=1.0, skala_zyl=1.0, verschiebe_aufw=0.0, skala_rand=0.0):
    """liefert Liste (s, r). s = Bogenlaenge (oder x), r Radius. Varianten fuer Empfindlichkeiten."""
    s = [0.0]
    for (x0, y0, _), (x1, y1, _) in zip(BECHER[:-1], BECHER[1:]):
        d = np.hypot(x1 - x0, y1 - y0) if bogen else (x1 - x0)
        s.append(s[-1] + d)
    s = np.array(s)
    r = np.array([b[2] for b in BECHER], dtype=float)
    i_a = 4                                  # Index der Aufweitungsstelle (x = 305)
    s_a, s_e = s[i_a], s[-1]
    # Laenge des fast zylindrischen Teils: Teil vor der Aufweitung strecken, Rest verschieben
    if skala_zyl != 1.0:
        neu = s.copy()
        neu[:i_a + 1] = s[:i_a + 1] * skala_zyl
        neu[i_a + 1:] = s[i_a + 1:] + s_a * (skala_zyl - 1)
        s = neu
    # Aufweitungsstelle verschieben bei gleicher Becherlaenge
    if verschiebe_aufw != 0.0:
        s_a2 = s_a * (1 + verschiebe_aufw)
        neu = s.copy()
        neu[:i_a + 1] = s[:i_a + 1] * (1 + verschiebe_aufw)
        neu[i_a + 1:] = s_a2 + (s[i_a + 1:] - s_a) * (s_e - s_a2) / (s_e - s_a)
        s = neu
    if skala_rand != 0.0:                    # Randradius +eps, gleitend ab der Aufweitung
        w = np.clip((s - s[i_a]) / (s[-1] - s[i_a]), 0, 1)
        r = r * (1 + skala_rand * w)
    s = s * skala_s
    return s, r


def becher_segmente(**kw):
    s, r = becher_punkte(**kw)
    return [(s[i + 1] - s[i], r[i], r[i + 1]) for i in range(len(s) - 1)]


def mundstueck(art, r1, l_bb=40.0, kessel_skala=1.0):
    """Mundstueck-Varianten (alle Zahlen mm). Rueckgabe: (Segmente, lumped, Laenge)."""
    k = np.sqrt(kessel_skala)
    if art == "a":   # Kessel 19 mm, 'Kesseltiefe 25 mm' bis zur Seele, Seele 5,5 mm
        seg = [(25.0, 9.5 * k, 2.75), (5.0, 2.75, 2.75), (l_bb, 2.75, r1)]
        return seg, None, 30.0 + l_bb
    if art == "b":   # 'Kesseltiefe 25 mm' = Kessel allein, dann Uebergang zur Seele
        seg = [(25.0, 9.5 * k, 6.0 * k), (6.0, 6.0 * k, 2.75), (5.0, 2.75, 2.75), (l_bb, 2.75, r1)]
        return seg, None, 36.0 + l_bb
    if art == "c":   # Kesselvolumen wie (a) + Verengung (Masse) + Rueckbohrung
        V = np.pi * 25.0 / 3 * (9.5 ** 2 + 9.5 * 2.75 + 2.75 ** 2) * kessel_skala
        seg = [(l_bb, 2.75, r1)]
        return seg, (V, 10.0, 2.75), 30.0 + l_bb
    if art == "d":   # Waldhorn-Mundstueck Wachter 2008 Abb. 52 (gemessen), dann Kegel 3,15 -> r1 ueber 20 mm
        x = [0, 5, 10, 15, 20, 25, 30, 30.5, 31.5, 35.9, 45.9, 50.9, 55.9, 60.9, 65.9, 67.38]
        d = [17.5, 14.77, 12.77, 10.3, 8.4, 6.23, 5.2, 4.75, 4.65, 4.7, 4.73, 4.73, 5.04, 5.5, 5.8, 6.3]
        seg = [(x[i + 1] - x[i], d[i] / 2 * (k if x[i] < 31.5 else 1), d[i + 1] / 2 * (k if x[i + 1] < 31.5 else 1))
               for i in range(len(x) - 1)]
        seg.append((20.0, 3.15, r1))
        return seg, None, 87.38
    if art == "0":
        return [], None, 0.0
    raise ValueError(art)


def bohrung(lesart="V1", horn="H3", mp="a", verl=None, l_bb=40.0, eps=None):
    """Segmentliste. lesart: V1 (Radius), V2a (Durchmesser, Sprung am Becher), V2b (Durchmesser, Kegel 4 -> R5).
    verl: None | ('mundrohr'|'kegel1'|'gleich', ziel_mm)  eps: dict der Stoergroessen fuer Empfindlichkeiten."""
    e = eps or {}
    L, R = TAB3[horn]
    L = [l * (1 + e.get("laenge", 0)) for l in L]
    if lesart == "V1":
        r1, rr, r5_ende = R1, R, R5
    else:
        r1, rr = R1 / 2, [x / 2 for x in R]
        r5_ende = R5 / 2 if lesart == "V2a" else R5
    sk = 1 + e.get("radien", 0)
    r1 = r1 * (1 + e.get("R1", 0)) * sk
    rr = [x * sk for x in rr]
    r5_ende = r5_ende * sk
    mpseg, lumped, mp_len = mundstueck(mp, r1, l_bb=l_bb * (1 + e.get("rueckbohrung", 0)),
                                       kessel_skala=1 + e.get("kessel", 0))
    bech = becher_segmente(skala_s=(1 + e.get("laenge", 0)) * (1 + e.get("becherbogen", 0)),
                           skala_zyl=1 + e.get("zyl", 0), verschiebe_aufw=e.get("aufw", 0),
                           skala_rand=e.get("rand", 0), bogen=not e.get("x_statt_bogen", False))
    if sk != 1:
        bech = [(l, a * sk, b * sk) for (l, a, b) in bech]
    ext = []
    if verl is not None:
        wo, ziel = verl
        ist = mp_len + sum(L) + sum(b[0] for b in bech)
        d = ziel - ist
        if wo == "mundrohr":
            ext = [(d, r1, r1)]
        elif wo == "kegel1":
            L = [L[0] + d] + L[1:]
        elif wo == "gleich":
            f = (sum(L) + d) / sum(L)
            L = [l * f for l in L]
    kegel = [(L[0], r1, rr[0]), (L[1], rr[0], rr[1]), (L[2], rr[1], rr[2]), (L[3], rr[2], r5_ende)]
    seg = list(mpseg) + ext + kegel + bech
    gesamt = sum(s[0] for s in seg) + (mp_len - sum(s[0] for s in mpseg) if lumped else 0)
    return seg, lumped, gesamt


def lumiviz_heute(becher="kegel"):
    """Nachbau des LumiViz-Stands (modell.md Z. 40): R1 2,75, Kegel 1-3 auf 16/28,5/44, Kegel 4 Zylinder 44,
    Becher 0,5 m 44 -> 90 (Form unbekannt: Kegel oder exponentiell), ohne Mundstueck."""
    L, R = TAB3["H3"]
    seg = [(L[0], 2.75, 16.0), (L[1], 16.0, 28.5), (L[2], 28.5, 44.0), (L[3], 44.0, 44.0)]
    if becher == "kegel":
        seg.append((500.0, 44.0, 90.0))
    else:
        s = np.linspace(0, 500, 26)
        r = 44.0 * (90.0 / 44.0) ** (s / 500)
        seg += [(s[i + 1] - s[i], r[i], r[i + 1]) for i in range(25)]
    return seg, None, sum(x[0] for x in seg)


LUFT = tmm.luft(20.0)


def rechne(seg, lumped, luft=LUFT, verlust=True, abstr="unflanged"):
    return tmm.spitzen(seg, luft, verlust=verlust, abstrahlung=abstr, lumped=lumped, fmax=1100.0, n_max=16)


def kennzahlen(sp):
    f = np.array([x[0] for x in sp])
    n = np.arange(1, len(f) + 1)
    cA = tmm.cent(f, n * F_FIS1)
    cB = tmm.cent(f, n * f[1] / 2) if len(f) > 1 else cA * np.nan
    sel = (n >= 2) & (n <= 16)
    return f, n, cA, cB, sel


def zeile_kurz(name, sp, laenge):
    f, n, cA, cB, sel = kennzahlen(sp)
    a, b = cA[sel], cB[sel]
    return (f"| {name} | {laenge:.0f} | {len(f)} | {f[0]:.1f} | {f[1]:.2f} | {cA[1]:+.0f} | {np.mean(a):+.0f} | "
            f"{np.mean(np.abs(a)):.0f} | {np.max(np.abs(a)):.0f} | {np.mean(np.abs(b)):.0f} | {np.max(np.abs(b)):.0f} | "
            f"{np.max(a[1:]) - np.min(a[1:]):.0f} |")


def tabelle_spitzen(name, sp, horn=None):
    f, n, cA, cB, sel = kennzahlen(sp)
    p(f"\n#### {name}\n")
    p("| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |")
    p("|---|---|---|---|---|---|---|")
    for i, (fp, z, q) in enumerate(sp):
        meas = ""
        if horn and 4 <= n[i] <= 10:
            meas = f"{tmm.cent(fp, TAB1_MEAS[horn][n[i]-4]):+.0f}"
        p(f"| {n[i]} | {fp:.2f} | {z:.2f} | {q:.0f} | {cA[i]:+.0f} | {cB[i]:+.0f} | {meas} |")


def kopf_kurz():
    p("| Variante | Länge (mm) | Spitzen | f1 (Hz) | f2 (Hz) | Cent f2 (A) | Mittel 2–16 (A) | mittl. Betrag (A) | "
      "max. Betrag (A) | mittl. Betrag (B) | max. Betrag (B) | Spanne 3–16 (A) |")
    p("|---|---|---|---|---|---|---|---|---|---|---|---|")


def tab1_vergleich(name, sp, horn):
    f = [x[0] for x in sp]
    c = [tmm.cent(f[n - 1], TAB1_MEAS[horn][n - 4]) for n in range(4, 11)]
    return f"| {name} | {horn} | " + " | ".join(f"{x:+.0f}" for x in c) + f" | {np.mean(c):+.0f} | {np.max(np.abs(c)):.0f} |"


if __name__ == "__main__":
    teil = sys.argv[1] if len(sys.argv) > 1 else "alles"
    p(f"Luft: T = 20 °C, c = {LUFT['c']:.2f} m/s, rho = {LUFT['rho']:.4f} kg/m³; Bezug A: Fis1 = {F_FIS1:.3f} Hz (a' = 440 Hz)")

    # ---- Segmentlisten ----
    if teil in ("alles", "seg"):
        p("\n### Becher-Stützpunkte (Bogenlänge aus Abb. 3b)\n")
        s, r = becher_punkte()
        p("| Punkt | x (mm) | y (mm) | s Bogen (mm) | r (mm) |")
        p("|---|---|---|---|---|")
        for i, b in enumerate(BECHER):
            p(f"| {i} | {b[0]} | {b[1]} | {s[i]:.1f} | {r[i]:.1f} |")
        for les in ("V1", "V2a", "V2b"):
            seg, lu, g = bohrung(les, "H3", "a")
            p(f"\n### Segmentliste {les}, Horn 3, Mundstück a (Gesamtlänge {g:.1f} mm)\n")
            p("| Nr | L (mm) | r ein (mm) | r aus (mm) |")
            p("|---|---|---|---|")
            for i, (L, a, b) in enumerate(seg):
                p(f"| {i+1} | {L:.2f} | {a:.2f} | {b:.2f} |")

    # ---- Hauptvarianten ----
    if teil in ("alles", "haupt"):
        p("\n### Übersicht Varianten (mit Wandverlusten, ungeflanscht, 20 °C)\n")
        kopf_kurz()
        faelle = []
        for les in ("V1", "V2a", "V2b"):
            for mp in ("a", "b", "c", "d", "0"):
                faelle.append((f"{les} H3 MP-{mp}", bohrung(les, "H3", mp)))
            for wo in ("mundrohr", "kegel1", "gleich"):
                faelle.append((f"{les} H3 MP-a +3,47 m {wo}", bohrung(les, "H3", "a", verl=(wo, 3470.0))))
        for h in ("H1", "H2"):
            faelle.append((f"V1 {h} MP-a", bohrung("V1", h, "a")))
        faelle.append(("LumiViz heute (Becher Kegel)", lumiviz_heute("kegel")))
        faelle.append(("LumiViz heute (Becher exp.)", lumiviz_heute("exp")))
        erg = {}
        for name, (seg, lu, g) in faelle:
            sp = rechne(seg, lu)
            erg[name] = sp
            p(zeile_kurz(name, sp, g))
        p("\n### Spitzen gegen Tab. 1 (Cent = 1200·log2(f_Spitze/f_gemessen)), Naturtöne 4–10\n")
        p("| Variante | Horn | n4 | n5 | n6 | n7 | n8 | n9 | n10 | Mittel | max. Betrag |")
        p("|---|---|---|---|---|---|---|---|---|---|---|")
        for name, sp in erg.items():
            horn = "H1" if " H1 " in name else "H2" if " H2 " in name else "H3"
            p(tab1_vergleich(name, sp, horn))
        for name in ("V1 H3 MP-a", "V2a H3 MP-a", "V2b H3 MP-a", "V1 H3 MP-a +3,47 m gleich",
                     "V2a H3 MP-a +3,47 m gleich", "LumiViz heute (Becher Kegel)"):
            tabelle_spitzen(name, erg[name], "H3")

    # ---- Zerlegung: vom LumiViz-Stand zu V1 (welcher Schritt verschiebt wie viel) ----
    if teil in ("alles", "zerlegung"):
        p("\n### Zerlegung LumiViz heute → V1 (Cent gg. n·46,249 Hz, Mittel 2–16 / Spitze 2 / Bezug-B-Betrag)\n")
        L, R = TAB3["H3"]
        bech_lv = [(500.0, 44.0, 90.0)]
        bech_v1 = becher_segmente()
        schritte = [
            ("0 LumiViz heute: R1 2,75, K4 Zylinder 44, Becher Kegel 0,5 m 44→90, ohne MP",
             [(L[0], 2.75, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 44)] + bech_lv, None),
            ("1 + R1 = 6,5", [(L[0], 6.5, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 44)] + bech_lv, None),
            ("2 + Kegel 4 44→51,5, Becher 51,5→90 (0,5 m)",
             [(L[0], 6.5, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 51.5), (500.0, 51.5, 90)], None),
            ("3 + Becher nach Abb. 3b (Bogen 589,5 mm)",
             [(L[0], 6.5, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 51.5)] + bech_v1, None),
            ("4 + Mundstück a (= V1 H3 MP-a)", bohrung("V1", "H3", "a")[0], None),
            ("5 + Verlängerung gleichmäßig auf 3,47 m", bohrung("V1", "H3", "a", verl=("gleich", 3470.0))[0], None),
        ]
        p("| Schritt | Länge (mm) | Cent Spitze 2 (A) | Mittel 2–16 (A) | mittl. Betrag (B) | max. Betrag (B) |")
        p("|---|---|---|---|---|---|")
        for name, seg, lu in schritte:
            f, n, cA, cB, sel = kennzahlen(rechne(seg, lu))
            p(f"| {name} | {sum(s[0] for s in seg):.0f} | {cA[1]:+.0f} | {np.mean(cA[sel]):+.0f} | "
              f"{np.mean(np.abs(cB[sel])):.0f} | {np.max(np.abs(cB[sel])):.0f} |")

    # ---- Vergleich mit Marcinno FEM (verlustfrei, c = 340) ----
    if teil in ("alles", "fem"):
        p("\n### V1 H3 verlustfrei, c = 340 m/s, rho = 1,225 (wie Marcinnò S. 5) gegen Tab. 2 „Reference alphorn“\n")
        lu340 = dict(T=293.15, c=340.0, rho=1.225)
        p("| Variante | Abstrahlung | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | Mittel 2–10 |")
        p("|---|---|---|---|---|---|---|---|---|---|---|---|---|")
        for mp in ("a", "b", "c", "d", "0"):
            for ab in ("unflanged", "flanged"):
                seg, lu, g = bohrung("V1", "H3", mp)
                sp = tmm.spitzen(seg, lu340, verlust=False, abstrahlung=ab, lumped=lu, fmax=700, n_max=12)
                c = [tmm.cent(sp[n - 1][0], TAB2_FEM_H3[n]) for n in range(2, 12)]
                p(f"| V1 H3 MP-{mp} | {ab} | " + " | ".join(f"{x:+.0f}" for x in c) + f" | {np.mean(c[:9]):+.0f} |")

    # ---- Einfluss Verluste, Abstrahlung, Temperatur ----
    if teil in ("alles", "modell"):
        p("\n### Modellwahl bei V1 H3 MP-a: Cent-Verschiebung der Spitzen 2–16 gegen den Grundfall\n")
        seg, lu, g = bohrung("V1", "H3", "a")
        basis = np.array([x[0] for x in rechne(seg, lu)])
        fall = {
            "ohne Wandverluste": rechne(seg, lu, verlust=False),
            "geflanscht (G9)": rechne(seg, lu, abstr="flanged"),
            "T = 30 °C": rechne(seg, lu, luft=tmm.luft(30.0)),
            "T = 14,5 °C (c ≈ 340)": rechne(seg, lu, luft=tmm.luft(14.5)),
        }
        p("| Fall | " + " | ".join(f"n{n}" for n in range(2, 17)) + " |")
        p("|---|" + "---|" * 15)
        for k, sp in fall.items():
            f = np.array([x[0] for x in sp])
            m = min(len(f), len(basis), 16)
            p(f"| {k} | " + " | ".join(f"{tmm.cent(f[i], basis[i]):+.1f}" for i in range(1, m)) + " |")

    # ---- Empfindlichkeiten ----
    if teil in ("alles", "empf"):
        groessen = [("R1", "R1"), ("Kesselvolumen", "kessel"), ("Rückbohrungslänge (Volumen)", "rueckbohrung"),
                    ("Becher-Randradius", "rand"), ("Länge fast zyl. Becherteil", "zyl"),
                    ("Aufweitungsstelle (Becherlänge fest)", "aufw"), ("Becher-Bogenlänge", "becherbogen"),
                    ("Gesamtlänge (L1–L4 + Becher)", "laenge"), ("alle Radien R1–Rand", "radien")]
        for les in ("V1", "V2a"):
            p(f"\n### Empfindlichkeit {les} H3 MP-a: Cent je +1 % (zentrale Differenz ±1 %), Spitzen 2–16\n")
            p("| Größe | " + " | ".join(f"n{n}" for n in range(2, 17)) + " | Spanne n2–16 |")
            p("|---|" + "---|" * 16)
            for titel, key in groessen:
                fp = [x[0] for x in rechne(*bohrung(les, "H3", "a", eps={key: 0.01})[:2])]
                fm = [x[0] for x in rechne(*bohrung(les, "H3", "a", eps={key: -0.01})[:2])]
                m = min(len(fp), len(fm), 16)
                c = [tmm.cent(fp[i], fm[i]) / 2 for i in range(1, m)]
                p(f"| {titel} | " + " | ".join(f"{x:+.1f}" for x in c) + f" | {max(c)-min(c):.1f} |")

    # ---- Annahmen: Rueckbohrung 20/70, Kruemmung x statt Bogen ----
    if teil in ("alles", "annahmen"):
        p("\n### Annahmen-Spannen (V1 H3 MP-a als Grundfall): Cent der Spitzen 2–16 gegen Grundfall\n")
        seg, lu, g = bohrung("V1", "H3", "a")
        basis = np.array([x[0] for x in rechne(seg, lu)])
        fall = {
            "Rückbohrung 20 mm": bohrung("V1", "H3", "a", l_bb=20.0),
            "Rückbohrung 70 mm": bohrung("V1", "H3", "a", l_bb=70.0),
            "Becher x statt Bogen (492 statt 590 mm)": bohrung("V1", "H3", "a", eps={"x_statt_bogen": True}),
            "Becherbogen +5 %": bohrung("V1", "H3", "a", eps={"becherbogen": 0.05}),
            "Becherbogen −5 %": bohrung("V1", "H3", "a", eps={"becherbogen": -0.05}),
        }
        p("| Fall | Gesamtlänge (mm) | " + " | ".join(f"n{n}" for n in range(2, 17)) + " |")
        p("|---|---|" + "---|" * 15)
        for k, (s_, l_, g_) in fall.items():
            f = np.array([x[0] for x in rechne(s_, l_)])
            m = min(len(f), len(basis), 16)
            p(f"| {k} | {g_:.0f} | " + " | ".join(f"{tmm.cent(f[i], basis[i]):+.1f}" for i in range(1, m)) + " |")

    with open(__file__.replace("rechnung.py", f"ausgabe_{teil}.md"), "w", encoding="utf-8") as fh:
        fh.write(AUS.getvalue())
```


### `zusatz.py`

```python
"""Zusatz: V1 mit den Schablonenlaengen aus Abb. 3a (L2 = 940, L3 = 825 mm, Nachpruefung 1.2, [abgelesen])
statt Tab. 3 (844,91 / 754,84) - als Hypothese, warum Tab. 3 + Becher kuerzer ist als ein Fis-Horn."""
import numpy as np
import tmm
import rechnung as R

R.TAB3["H3S"] = ([539.92, 940.0, 825.0, 444.94], [16.0, 28.5, 44.0])
R.TAB1_MEAS["H3S"] = R.TAB1_MEAS["H3"]
print("| Variante | Länge (mm) | f2 | Cent f2 (A) | Mittel 2–16 (A) | Spanne 3–16 (A) | Tab.1 n4..n10 | Mittel Tab.1 |")
print("|---|---|---|---|---|---|---|---|")
for mp in ("a", "d"):
    seg, lu, g = R.bohrung("V1", "H3S", mp)
    sp = R.rechne(seg, lu)
    f, n, cA, cB, sel = R.kennzahlen(sp)
    t1 = [tmm.cent(f[k - 1], R.TAB1_MEAS["H3"][k - 4]) for k in range(4, 11)]
    print(f"| V1 Schablone MP-{mp} | {g:.0f} | {f[1]:.2f} | {cA[1]:+.0f} | {np.mean(cA[sel]):+.0f} | "
          f"{np.max(cA[2:16]) - np.min(cA[2:16]):.0f} | " + " ".join(f"{x:+.0f}" for x in t1) + f" | {np.mean(t1):+.0f} |")
```


### Ausgabe `python pruefung.py`

```text
== K5 Zahlenbeispiel Verluste: (2.73)/(2.74) in (2.75)/(2.76) -> Konstanten von (2.77)/(2.78)
  1/(sqrt2*632.8) + 0.4/(sqrt2*532.8) = 1.6483e-03   (Hendrie 2.77: 1.65e-3)
  2*pi*1.6483e-03/345 = 3.002e-05            (Hendrie 2.78: 3e-5, mit c = 345 m/s S. 25)
  G1 bei 20 C: c = 343.23 m/s, rho = 1.2041 kg/m3; bei 300 K: c = 347.22 m/s

== K1 verlustfreier Zylinder r = 20 mm, L = 1 m, ideal offen (Z_L = 0): Spitzen bei (2n-1) c / 4L
  n=1: TMM   85.8077 Hz  geschlossen   85.8087 Hz  Diff -0.0206 Cent
  n=2: TMM  257.4270 Hz  geschlossen  257.4262 Hz  Diff +0.0052 Cent
  n=3: TMM  429.0450 Hz  geschlossen  429.0437 Hz  Diff +0.0052 Cent
  n=4: TMM  600.6600 Hz  geschlossen  600.6612 Hz  Diff -0.0034 Cent
  n=5: TMM  772.2780 Hz  geschlossen  772.2787 Hz  Diff -0.0015 Cent

== K2 verlustfreier Kegelstumpf r 5 -> 50 mm, L = 1 m, ideal offen: Hendrie (2.103) -> sin(kL + atan(k x1)) = 0
  n=1: TMM  154.9400 Hz  (2.103)  154.9401 Hz  Diff -0.0013 Cent
  n=2: TMM  312.3190 Hz  (2.103)  312.3193 Hz  Diff -0.0015 Cent
  n=3: TMM  473.0030 Hz  (2.103)  473.0038 Hz  Diff -0.0028 Cent
  n=4: TMM  636.5850 Hz  (2.103)  636.5861 Hz  Diff -0.0029 Cent
  n=5: TMM  802.3100 Hz  (2.103)  802.3113 Hz  Diff -0.0028 Cent

== K3 Hendrie (2.102) mit Last (ungeflanscht, G8) gegen eigene Kegelmatrix, Kegel r 5 -> 50 mm, L = 1 m
  f=  50.0: |Z_TMM|=4.9841e+05  |(2.102) wie gedruckt|=4.9378e+05  rel.Abw 9.29e-03  |mit 1/(sin t1 sin t2)|=4.9841e+05  rel.Abw 2.85e-20
  f= 137.0: |Z_TMM|=2.6557e+06  |(2.102) wie gedruckt|=2.1809e+06  rel.Abw 1.79e-01  |mit 1/(sin t1 sin t2)|=2.6557e+06  rel.Abw 1.76e-16
  f= 300.0: |Z_TMM|=2.0484e+07  |(2.102) wie gedruckt|=1.0134e+07  rel.Abw 5.22e-01  |mit 1/(sin t1 sin t2)|=2.0484e+07  rel.Abw 7.53e-15
  f= 555.0: |Z_TMM|=3.5723e+06  |(2.102) wie gedruckt|=3.5444e+06  rel.Abw 7.99e-03  |mit 1/(sin t1 sin t2)|=3.5723e+06  rel.Abw 9.13e-16

== K4 Hendrie Abb. 2.12: verlustbehafteter Zylinder r = 2 cm, L = 1 m, ungeflanscht, c = 345 m/s
  n= 1:    84.46 Hz  |Z/Zc| =   69.8  Q =   55.1   (2.92):    85.23 Hz
  n= 2:   254.36 Hz  |Z/Zc| =   37.7  Q =   89.1   (2.92):   255.68 Hz
  n= 3:   424.44 Hz  |Z/Zc| =   26.8  Q =  105.3   (2.92):   426.14 Hz
  n= 4:   594.61 Hz  |Z/Zc| =   20.5  Q =  113.1   (2.92):   596.59 Hz
  n= 5:   764.83 Hz  |Z/Zc| =   16.4  Q =  116.1   (2.92):   767.05 Hz
  n= 6:   935.10 Hz  |Z/Zc| =   13.4  Q =  116.4   (2.92):   937.50 Hz
  n= 7:  1105.42 Hz  |Z/Zc| =   11.3  Q =  115.3   (2.92):  1107.95 Hz
  n= 8:  1275.77 Hz  |Z/Zc| =    9.6  Q =  113.4   (2.92):  1278.41 Hz
  n= 9:  1446.17 Hz  |Z/Zc| =    8.3  Q =    nan   (2.92):  1448.86 Hz
  n=10:  1616.59 Hz  |Z/Zc| =    7.3  Q =    nan   (2.92):  1619.32 Hz
  n=11:  1787.05 Hz  |Z/Zc| =    6.4  Q =    nan   (2.92):  1789.77 Hz
  n=12:  1957.54 Hz  |Z/Zc| =    5.8  Q =    nan   (2.92):  1960.23 Hz

== K4b wie K4, aber so wie Hendrie Abb. 2.12 rechnet: (2.92) = ideal offen bei L + 0.6 r, nur Wandverluste
  n= 1:    84.46 Hz  |Z/Zc| =  71.0 (37.0 dB)  abgelesen None
  n= 2:   254.36 Hz  |Z/Zc| =  41.1 (32.3 dB)  abgelesen 40
  n= 3:   424.43 Hz  |Z/Zc| =  31.8 (30.1 dB)  abgelesen 31
  n= 4:   594.57 Hz  |Z/Zc| =  26.9 (28.6 dB)  abgelesen 27
  n= 5:   764.75 Hz  |Z/Zc| =  23.7 (27.5 dB)  abgelesen 24
  n= 6:   934.97 Hz  |Z/Zc| =  21.5 (26.6 dB)  abgelesen 22
  n= 7:  1105.21 Hz  |Z/Zc| =  19.8 (25.9 dB)  abgelesen 20
  n= 8:  1275.46 Hz  |Z/Zc| =  18.4 (25.3 dB)  abgelesen 18
  n= 9:  1445.72 Hz  |Z/Zc| =  17.3 (24.8 dB)  abgelesen 17
  n=10:  1615.99 Hz  |Z/Zc| =  16.4 (24.3 dB)  abgelesen 16
  n=11:  1786.28 Hz  |Z/Zc| =  15.6 (23.8 dB)  abgelesen 15.5
  n=12:  1956.57 Hz  |Z/Zc| =  14.9 (23.5 dB)  abgelesen 15
```


### Ausgabe `python zusatz.py`

| Variante | Länge (mm) | f2 | Cent f2 (A) | Mittel 2–16 (A) | Spanne 3–16 (A) | Tab.1 n4..n10 | Mittel Tab.1 |
|---|---|---|---|---|---|---|---|
| V1 Schablone MP-a | 3409 | 89.84 | -51 | +48 | 35 | +44 +40 +55 +23 +49 +50 +87 | +50 |
| V1 Schablone MP-d | 3427 | 89.85 | -50 | +30 | 52 | +45 +41 +56 +22 +46 +43 +74 | +47 |


### Ausgabe `python rechnung.py alles` (Datei `ausgabe_alles.md`, Lauf vom 2026-11-17, ≈ 15 min)

Luft: T = 20 °C, c = 343.23 m/s, rho = 1.2041 kg/m³; Bezug A: Fis1 = 46.249 Hz (a' = 440 Hz)

### Becher-Stützpunkte (Bogenlänge aus Abb. 3b)

| Punkt | x (mm) | y (mm) | s Bogen (mm) | r (mm) |
|---|---|---|---|---|
| 0 | 0 | 0 | 0.0 | 51.5 |
| 1 | 100 | 0.5 | 100.0 | 51.5 |
| 2 | 200 | 8 | 200.3 | 51.6 |
| 3 | 300 | 28 | 302.3 | 51.8 |
| 4 | 305 | 30.5 | 307.9 | 51.8 |
| 5 | 350 | 53 | 358.2 | 57.0 |
| 6 | 400 | 92 | 421.6 | 62.0 |
| 7 | 450 | 158 | 504.4 | 71.0 |
| 8 | 492 | 232 | 589.5 | 89.5 |

### Segmentliste V1, Horn 3, Mundstück a (Gesamtlänge 3244.1 mm)

| Nr | L (mm) | r ein (mm) | r aus (mm) |
|---|---|---|---|
| 1 | 25.00 | 9.50 | 2.75 |
| 2 | 5.00 | 2.75 | 2.75 |
| 3 | 40.00 | 2.75 | 6.50 |
| 4 | 539.92 | 6.50 | 16.00 |
| 5 | 844.91 | 16.00 | 28.50 |
| 6 | 754.84 | 28.50 | 44.00 |
| 7 | 444.94 | 44.00 | 51.50 |
| 8 | 100.00 | 51.50 | 51.50 |
| 9 | 100.28 | 51.50 | 51.60 |
| 10 | 101.98 | 51.60 | 51.80 |
| 11 | 5.59 | 51.80 | 51.80 |
| 12 | 50.31 | 51.80 | 57.00 |
| 13 | 63.41 | 57.00 | 62.00 |
| 14 | 82.80 | 62.00 | 71.00 |
| 15 | 85.09 | 71.00 | 89.50 |

### Segmentliste V2a, Horn 3, Mundstück a (Gesamtlänge 3244.1 mm)

| Nr | L (mm) | r ein (mm) | r aus (mm) |
|---|---|---|---|
| 1 | 25.00 | 9.50 | 2.75 |
| 2 | 5.00 | 2.75 | 2.75 |
| 3 | 40.00 | 2.75 | 3.25 |
| 4 | 539.92 | 3.25 | 8.00 |
| 5 | 844.91 | 8.00 | 14.25 |
| 6 | 754.84 | 14.25 | 22.00 |
| 7 | 444.94 | 22.00 | 25.75 |
| 8 | 100.00 | 51.50 | 51.50 |
| 9 | 100.28 | 51.50 | 51.60 |
| 10 | 101.98 | 51.60 | 51.80 |
| 11 | 5.59 | 51.80 | 51.80 |
| 12 | 50.31 | 51.80 | 57.00 |
| 13 | 63.41 | 57.00 | 62.00 |
| 14 | 82.80 | 62.00 | 71.00 |
| 15 | 85.09 | 71.00 | 89.50 |

### Segmentliste V2b, Horn 3, Mundstück a (Gesamtlänge 3244.1 mm)

| Nr | L (mm) | r ein (mm) | r aus (mm) |
|---|---|---|---|
| 1 | 25.00 | 9.50 | 2.75 |
| 2 | 5.00 | 2.75 | 2.75 |
| 3 | 40.00 | 2.75 | 3.25 |
| 4 | 539.92 | 3.25 | 8.00 |
| 5 | 844.91 | 8.00 | 14.25 |
| 6 | 754.84 | 14.25 | 22.00 |
| 7 | 444.94 | 22.00 | 51.50 |
| 8 | 100.00 | 51.50 | 51.50 |
| 9 | 100.28 | 51.50 | 51.60 |
| 10 | 101.98 | 51.60 | 51.80 |
| 11 | 5.59 | 51.80 | 51.80 |
| 12 | 50.31 | 51.80 | 57.00 |
| 13 | 63.41 | 57.00 | 62.00 |
| 14 | 82.80 | 62.00 | 71.00 |
| 15 | 85.09 | 71.00 | 89.50 |

### Übersicht Varianten (mit Wandverlusten, ungeflanscht, 20 °C)

| Variante | Länge (mm) | Spitzen | f1 (Hz) | f2 (Hz) | Cent f2 (A) | Mittel 2–16 (A) | mittl. Betrag (A) | max. Betrag (A) | mittl. Betrag (B) | max. Betrag (B) | Spanne 3–16 (A) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| V1 H3 MP-a | 3244 | 16 | 48.7 | 94.84 | +43 | +132 | 132 | 145 | 88 | 101 | 29 |
| V1 H3 MP-b | 3250 | 16 | 48.7 | 94.70 | +41 | +117 | 117 | 134 | 77 | 94 | 23 |
| V1 H3 MP-c | 3244 | 16 | 48.7 | 94.87 | +44 | +133 | 133 | 147 | 90 | 104 | 31 |
| V1 H3 MP-d | 3261 | 16 | 48.7 | 94.87 | +44 | +112 | 112 | 144 | 68 | 100 | 54 |
| V1 H3 MP-0 | 3174 | 16 | 48.8 | 95.31 | +52 | +163 | 163 | 194 | 111 | 142 | 64 |
| V1 H3 MP-a +3,47 m mundrohr | 3470 | 16 | 48.3 | 92.12 | -7 | +9 | 20 | 34 | 24 | 41 | 65 |
| V1 H3 MP-a +3,47 m kegel1 | 3470 | 16 | 47.5 | 88.81 | -70 | +12 | 27 | 70 | 82 | 103 | 63 |
| V1 H3 MP-a +3,47 m gleich | 3470 | 16 | 45.5 | 88.52 | -76 | +17 | 28 | 76 | 93 | 109 | 38 |
| V2a H3 MP-a | 3244 | 16 | 54.7 | 104.98 | +219 | +100 | 100 | 219 | 119 | 212 | 137 |
| V2a H3 MP-b | 3250 | 16 | 54.6 | 104.13 | +205 | +81 | 86 | 205 | 124 | 225 | 155 |
| V2a H3 MP-c | 3244 | 16 | 54.8 | 105.12 | +221 | +102 | 102 | 221 | 119 | 210 | 136 |
| V2a H3 MP-d | 3261 | 16 | 54.7 | 104.98 | +219 | +92 | 92 | 219 | 128 | 212 | 137 |
| V2a H3 MP-0 | 3174 | 16 | 55.1 | 106.78 | +249 | +167 | 167 | 249 | 81 | 189 | 146 |
| V2a H3 MP-a +3,47 m mundrohr | 3470 | 16 | 53.8 | 99.98 | +135 | -22 | 49 | 169 | 156 | 303 | 200 |
| V2a H3 MP-a +3,47 m kegel1 | 3470 | 16 | 52.6 | 97.09 | +84 | -18 | 40 | 110 | 101 | 194 | 142 |
| V2a H3 MP-a +3,47 m gleich | 3470 | 16 | 50.5 | 97.59 | +93 | -12 | 40 | 99 | 105 | 192 | 151 |
| V2b H3 MP-a | 3244 | 16 | 58.6 | 103.48 | +194 | +105 | 105 | 194 | 90 | 181 | 117 |
| V2b H3 MP-b | 3250 | 16 | 58.4 | 102.93 | +185 | +87 | 88 | 185 | 98 | 192 | 132 |
| V2b H3 MP-c | 3244 | 16 | 58.6 | 103.57 | +196 | +107 | 107 | 196 | 89 | 180 | 119 |
| V2b H3 MP-d | 3261 | 16 | 58.6 | 103.47 | +194 | +96 | 96 | 194 | 98 | 181 | 114 |
| V2b H3 MP-0 | 3174 | 16 | 59.1 | 104.62 | +213 | +172 | 172 | 213 | 42 | 161 | 159 |
| V2b H3 MP-a +3,47 m mundrohr | 3470 | 16 | 57.5 | 100.05 | +136 | -17 | 41 | 160 | 153 | 296 | 177 |
| V2b H3 MP-a +3,47 m kegel1 | 3470 | 16 | 56.0 | 97.93 | +99 | -13 | 31 | 119 | 111 | 218 | 137 |
| V2b H3 MP-a +3,47 m gleich | 3470 | 16 | 54.2 | 97.74 | +95 | -8 | 26 | 96 | 103 | 191 | 111 |
| V1 H1 MP-a | 3249 | 16 | 50.1 | 96.77 | +78 | +126 | 126 | 141 | 47 | 63 | 54 |
| V1 H2 MP-a | 3244 | 16 | 49.7 | 97.79 | +96 | +129 | 129 | 144 | 34 | 48 | 55 |
| LumiViz heute (Becher Kegel) | 3085 | 16 | 51.3 | 104.26 | +207 | +242 | 242 | 304 | 35 | 97 | 83 |
| LumiViz heute (Becher exp.) | 3085 | 16 | 50.8 | 103.57 | +196 | +244 | 244 | 300 | 48 | 105 | 72 |

### Spitzen gegen Tab. 1 (Cent = 1200·log2(f_Spitze/f_gemessen)), Naturtöne 4–10

| Variante | Horn | n4 | n5 | n6 | n7 | n8 | n9 | n10 | Mittel | max. Betrag |
|---|---|---|---|---|---|---|---|---|---|---|
| V1 H3 MP-a | H3 | +129 | +122 | +139 | +104 | +130 | +132 | +170 | +132 | 170 |
| V1 H3 MP-b | H3 | +122 | +114 | +128 | +94 | +118 | +118 | +152 | +121 | 152 |
| V1 H3 MP-c | H3 | +129 | +124 | +140 | +106 | +132 | +134 | +172 | +134 | 172 |
| V1 H3 MP-d | H3 | +130 | +123 | +139 | +102 | +126 | +123 | +152 | +128 | 152 |
| V1 H3 MP-0 | H3 | +148 | +145 | +164 | +130 | +159 | +163 | +204 | +159 | 204 |
| V1 H3 MP-a +3,47 m mundrohr | H3 | -31 | -51 | -18 | -36 | -2 | +6 | +48 | -12 | 51 |
| V1 H3 MP-a +3,47 m kegel1 | H3 | -10 | -6 | +17 | -13 | +14 | +16 | +52 | +10 | 52 |
| V1 H3 MP-a +3,47 m gleich | H3 | +8 | +4 | +23 | -7 | +19 | +20 | +56 | +18 | 56 |
| V2a H3 MP-a | H3 | +7 | +3 | +65 | +81 | +119 | +60 | +87 | +60 | 119 |
| V2a H3 MP-b | H3 | -20 | -38 | +26 | +48 | +99 | +50 | +68 | +33 | 99 |
| V2a H3 MP-c | H3 | +11 | +8 | +70 | +85 | +122 | +61 | +89 | +64 | 122 |
| V2a H3 MP-d | H3 | +7 | +0 | +60 | +73 | +111 | +53 | +75 | +54 | 111 |
| V2a H3 MP-0 | H3 | +59 | +83 | +159 | +169 | +175 | +118 | +179 | +135 | 179 |
| V2a H3 MP-a +3,47 m mundrohr | H3 | -146 | -189 | -99 | -61 | +13 | -4 | -16 | -72 | 189 |
| V2a H3 MP-a +3,47 m kegel1 | H3 | -102 | -131 | -70 | -53 | +9 | -11 | -27 | -55 | 131 |
| V2a H3 MP-a +3,47 m gleich | H3 | -86 | -120 | -63 | -47 | +10 | -10 | -26 | -49 | 120 |
| V2b H3 MP-a | H3 | +69 | +109 | +108 | +47 | +89 | +85 | +117 | +89 | 117 |
| V2b H3 MP-b | H3 | +29 | +67 | +81 | +20 | +64 | +69 | +101 | +61 | 101 |
| V2b H3 MP-c | H3 | +74 | +114 | +111 | +51 | +92 | +88 | +120 | +93 | 120 |
| V2b H3 MP-d | H3 | +68 | +106 | +104 | +40 | +80 | +76 | +105 | +83 | 106 |
| V2b H3 MP-0 | H3 | +143 | +191 | +173 | +126 | +168 | +156 | +203 | +166 | 203 |
| V2b H3 MP-a +3,47 m mundrohr | H3 | -160 | -88 | -13 | -67 | -27 | -10 | +18 | -50 | 160 |
| V2b H3 MP-a +3,47 m kegel1 | H3 | -90 | -31 | +5 | -61 | -29 | -18 | +6 | -31 | 90 |
| V2b H3 MP-a +3,47 m gleich | H3 | -58 | -14 | +1 | -60 | -22 | -20 | +4 | -24 | 60 |
| V1 H1 MP-a | H1 | +149 | +343 | +132 | +208 | +122 | +124 | +130 | +173 | 343 |
| V1 H2 MP-a | H2 | +304 | +287 | +136 | +206 | +149 | +127 | +189 | +200 | 304 |
| LumiViz heute (Becher Kegel) | H3 | +269 | +203 | +218 | +192 | +232 | +240 | +274 | +233 | 274 |
| LumiViz heute (Becher exp.) | H3 | +278 | +218 | +225 | +195 | +231 | +239 | +275 | +237 | 278 |

#### V1 H3 MP-a

| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |
|---|---|---|---|---|---|---|
| 1 | 48.69 | 12.08 | 52 | +89 | +46 |  |
| 2 | 94.84 | 21.84 | 61 | +43 | +0 |  |
| 3 | 148.36 | 28.59 | 69 | +116 | +73 |  |
| 4 | 199.25 | 33.23 | 73 | +129 | +85 | +129 |
| 5 | 251.15 | 33.65 | 74 | +143 | +100 | +122 |
| 6 | 301.17 | 31.06 | 69 | +142 | +98 | +139 |
| 7 | 351.02 | 25.76 | 61 | +140 | +97 | +104 |
| 8 | 400.01 | 25.44 | 58 | +135 | +92 | +130 |
| 9 | 450.04 | 26.15 | 57 | +135 | +92 | +132 |
| 10 | 500.78 | 28.74 | 59 | +138 | +94 | +170 |
| 11 | 551.21 | 29.79 | 59 | +139 | +96 |  |
| 12 | 602.38 | 31.92 | 60 | +142 | +99 |  |
| 13 | 653.03 | 33.46 | 61 | +143 | +100 |  |
| 14 | 703.90 | 35.58 | nan | +145 | +101 |  |
| 15 | 753.92 | 36.30 | nan | +144 | +101 |  |
| 16 | 803.76 | 33.73 | nan | +143 | +100 |  |

#### V2a H3 MP-a

| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |
|---|---|---|---|---|---|---|
| 1 | 54.72 | 31.65 | 24 | +291 | +72 |  |
| 2 | 104.98 | 56.73 | 32 | +219 | +0 |  |
| 3 | 150.78 | 67.14 | 42 | +144 | -75 |  |
| 4 | 185.79 | 82.80 | 43 | +7 | -212 | +7 |
| 5 | 234.36 | 105.52 | 45 | +23 | -196 | +3 |
| 6 | 288.72 | 103.25 | 51 | +69 | -151 | +65 |
| 7 | 346.24 | 82.74 | 54 | +116 | -103 | +81 |
| 8 | 397.49 | 39.27 | 41 | +124 | -95 | +119 |
| 9 | 431.69 | 25.06 | nan | +63 | -156 | +60 |
| 10 | 477.38 | 46.95 | 53 | +55 | -164 | +87 |
| 11 | 535.26 | 43.37 | 62 | +88 | -131 |  |
| 12 | 595.02 | 33.72 | nan | +121 | -99 |  |
| 13 | 652.79 | 17.97 | nan | +142 | -77 |  |
| 14 | 690.51 | 11.05 | nan | +111 | -108 |  |
| 15 | 733.27 | 18.00 | nan | +96 | -123 |  |
| 16 | 791.46 | 19.00 | nan | +116 | -103 |  |

#### V2b H3 MP-a

| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |
|---|---|---|---|---|---|---|
| 1 | 58.60 | 34.78 | 24 | +410 | +216 |  |
| 2 | 103.48 | 48.39 | 41 | +194 | +0 |  |
| 3 | 139.79 | 72.38 | 39 | +13 | -181 |  |
| 4 | 192.54 | 103.44 | 39 | +69 | -125 | +69 |
| 5 | 249.24 | 105.57 | 46 | +130 | -65 | +109 |
| 6 | 295.90 | 70.46 | 50 | +111 | -83 | +108 |
| 7 | 339.60 | 64.74 | 47 | +83 | -111 | +47 |
| 8 | 390.50 | 55.01 | 47 | +93 | -101 | +89 |
| 9 | 438.06 | 37.72 | 45 | +88 | -106 | +85 |
| 10 | 485.84 | 39.24 | 48 | +85 | -109 | +117 |
| 11 | 540.85 | 31.15 | nan | +106 | -88 |  |
| 12 | 591.89 | 21.22 | nan | +111 | -83 |  |
| 13 | 639.79 | 20.45 | nan | +108 | -87 |  |
| 14 | 693.97 | 19.13 | nan | +120 | -74 |  |
| 15 | 747.01 | 14.53 | nan | +128 | -66 |  |
| 16 | 797.20 | 13.07 | nan | +129 | -65 |  |

#### V1 H3 MP-a +3,47 m gleich

| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |
|---|---|---|---|---|---|---|
| 1 | 45.46 | 11.89 | 49 | -30 | +46 |  |
| 2 | 88.52 | 21.27 | 59 | -76 | +0 |  |
| 3 | 138.35 | 27.97 | 68 | -5 | +71 |  |
| 4 | 185.87 | 32.51 | 73 | +8 | +84 | +8 |
| 5 | 234.53 | 33.60 | 75 | +24 | +100 | +4 |
| 6 | 281.72 | 31.59 | 73 | +26 | +102 | +23 |
| 7 | 329.18 | 26.61 | 67 | +29 | +105 | -7 |
| 8 | 375.15 | 24.91 | 61 | +24 | +100 | +19 |
| 9 | 421.77 | 24.69 | 59 | +23 | +99 | +20 |
| 10 | 468.92 | 26.76 | 60 | +24 | +100 | +56 |
| 11 | 516.20 | 27.38 | 61 | +25 | +101 |  |
| 12 | 564.15 | 29.39 | 62 | +28 | +104 |  |
| 13 | 611.84 | 30.59 | 63 | +30 | +106 |  |
| 14 | 659.68 | 33.09 | 63 | +32 | +108 |  |
| 15 | 706.87 | 34.85 | 63 | +32 | +109 |  |
| 16 | 754.25 | 34.81 | nan | +33 | +109 |  |

#### V2a H3 MP-a +3,47 m gleich

| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |
|---|---|---|---|---|---|---|
| 1 | 50.51 | 30.44 | 23 | +153 | +60 |  |
| 2 | 97.59 | 54.65 | 31 | +93 | +0 |  |
| 3 | 142.92 | 69.11 | 39 | +51 | -42 |  |
| 4 | 176.04 | 74.29 | 44 | -86 | -179 | -86 |
| 5 | 218.36 | 96.94 | 44 | -99 | -192 | -120 |
| 6 | 268.08 | 98.49 | 49 | -60 | -153 | -63 |
| 7 | 321.72 | 85.68 | 54 | -11 | -104 | -47 |
| 8 | 373.13 | 60.60 | 51 | +15 | -78 | +10 |
| 9 | 414.64 | 22.27 | nan | -7 | -100 | -10 |
| 10 | 447.17 | 34.28 | nan | -58 | -151 | -26 |
| 11 | 496.64 | 43.47 | 58 | -42 | -135 |  |
| 12 | 551.48 | 38.92 | 63 | -11 | -104 |  |
| 13 | 606.93 | 29.42 | nan | +16 | -77 |  |
| 14 | 659.52 | 15.88 | nan | +32 | -61 |  |
| 15 | 692.60 | 11.42 | nan | -3 | -96 |  |
| 16 | 735.11 | 17.45 | nan | -11 | -104 |  |

#### LumiViz heute (Becher Kegel)

| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |
|---|---|---|---|---|---|---|
| 1 | 51.27 | 12.29 | 51 | +178 | -29 |  |
| 2 | 104.26 | 20.77 | 62 | +207 | +0 |  |
| 3 | 165.37 | 24.63 | 64 | +304 | +97 |  |
| 4 | 216.04 | 22.93 | 51 | +269 | +61 | +269 |
| 5 | 263.17 | 28.07 | 50 | +224 | +17 | +203 |
| 6 | 315.30 | 40.26 | 58 | +221 | +14 | +218 |
| 7 | 369.36 | 41.24 | 60 | +228 | +21 | +192 |
| 8 | 424.13 | 40.12 | 60 | +236 | +29 | +232 |
| 9 | 479.01 | 35.72 | 54 | +243 | +36 | +240 |
| 10 | 531.94 | 32.53 | nan | +242 | +35 | +274 |
| 11 | 582.65 | 33.85 | nan | +235 | +28 |  |
| 12 | 636.50 | 38.60 | nan | +237 | +30 |  |
| 13 | 691.62 | 37.94 | nan | +242 | +35 |  |
| 14 | 746.86 | 34.11 | nan | +247 | +40 |  |
| 15 | 801.10 | 32.50 | nan | +249 | +42 |  |
| 16 | 854.55 | 30.68 | nan | +249 | +42 |  |

### Zerlegung LumiViz heute → V1 (Cent gg. n·46,249 Hz, Mittel 2–16 / Spitze 2 / Bezug-B-Betrag)

| Schritt | Länge (mm) | Cent Spitze 2 (A) | Mittel 2–16 (A) | mittl. Betrag (B) | max. Betrag (B) |
|---|---|---|---|---|---|
| 0 LumiViz heute: R1 2,75, K4 Zylinder 44, Becher Kegel 0,5 m 44→90, ohne MP | 3085 | +207 | +242 | 35 | 97 |
| 1 + R1 = 6,5 | 3085 | +151 | +210 | 59 | 90 |
| 2 + Kegel 4 44→51,5, Becher 51,5→90 (0,5 m) | 3085 | +164 | +206 | 42 | 75 |
| 3 + Becher nach Abb. 3b (Bogen 589,5 mm) | 3174 | +52 | +163 | 111 | 142 |
| 4 + Mundstück a (= V1 H3 MP-a) | 3244 | +43 | +132 | 88 | 101 |
| 5 + Verlängerung gleichmäßig auf 3,47 m | 3470 | -76 | +17 | 93 | 109 |

### V1 H3 verlustfrei, c = 340 m/s, rho = 1,225 (wie Marcinnò S. 5) gegen Tab. 2 „Reference alphorn“

| Variante | Abstrahlung | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | Mittel 2–10 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| V1 H3 MP-a | unflanged | +12 | +14 | +14 | +22 | +20 | +29 | +41 | +46 | +36 | +35 | +26 |
| V1 H3 MP-a | flanged | +7 | +9 | +8 | +14 | +9 | +15 | +26 | +31 | +23 | +22 | +16 |
| V1 H3 MP-b | unflanged | +9 | +9 | +8 | +14 | +9 | +18 | +28 | +31 | +18 | +15 | +16 |
| V1 H3 MP-b | flanged | +4 | +4 | +2 | +6 | -1 | +5 | +14 | +17 | +6 | +4 | +6 |
| V1 H3 MP-c | unflanged | +12 | +14 | +15 | +23 | +20 | +30 | +42 | +47 | +37 | +36 | +27 |
| V1 H3 MP-c | flanged | +7 | +9 | +9 | +15 | +10 | +16 | +27 | +32 | +24 | +23 | +16 |
| V1 H3 MP-d | unflanged | +13 | +16 | +17 | +24 | +21 | +29 | +38 | +38 | +20 | +7 | +24 |
| V1 H3 MP-d | flanged | +8 | +10 | +10 | +16 | +9 | +13 | +22 | +24 | +8 | -4 | +13 |
| V1 H3 MP-0 | unflanged | +19 | +27 | +33 | +44 | +44 | +54 | +69 | +76 | +69 | +70 | +48 |
| V1 H3 MP-0 | flanged | +14 | +21 | +26 | +36 | +33 | +40 | +54 | +62 | +56 | +58 | +38 |

### Modellwahl bei V1 H3 MP-a: Cent-Verschiebung der Spitzen 2–16 gegen den Grundfall

| Fall | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| ohne Wandverluste | +14.8 | +12.4 | +10.9 | +9.7 | +8.9 | +8.3 | +7.9 | +7.5 | +7.2 | +6.7 | +6.4 | +6.0 | +5.7 | +5.2 | +4.6 |
| geflanscht (G9) | -5.2 | -5.5 | -6.5 | -8.5 | -11.3 | -15.1 | -15.7 | -15.0 | -13.7 | -13.2 | -12.5 | -12.3 | -12.1 | -12.3 | -13.2 |
| T = 30 °C | +28.7 | +28.7 | +28.8 | +28.8 | +28.8 | +28.8 | +28.9 | +28.9 | +28.9 | +28.9 | +28.9 | +28.9 | +28.9 | +28.9 | +28.9 |
| T = 14,5 °C (c ≈ 340) | -16.2 | -16.2 | -16.2 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 | -16.3 |

### Empfindlichkeit V1 H3 MP-a: Cent je +1 % (zentrale Differenz ±1 %), Spitzen 2–16

| Größe | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 | Spanne n2–16 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| R1 | -1.1 | -1.3 | -1.1 | -0.6 | -0.2 | -0.1 | +0.0 | +0.1 | +0.2 | +0.3 | +0.4 | +0.5 | +0.5 | +0.5 | +0.4 | 1.8 |
| Kesselvolumen | -0.1 | -0.1 | -0.1 | -0.1 | -0.1 | -0.1 | -0.2 | -0.2 | -0.2 | -0.2 | -0.3 | -0.3 | -0.4 | -0.4 | -0.4 | 0.3 |
| Rückbohrungslänge (Volumen) | -0.1 | -0.1 | -0.1 | -0.1 | -0.1 | -0.1 | -0.2 | -0.2 | -0.2 | -0.2 | -0.3 | -0.3 | -0.3 | -0.4 | -0.4 | 0.3 |
| Becher-Randradius | +1.0 | +0.9 | +0.8 | +0.8 | +0.6 | +0.4 | +0.1 | -0.1 | -0.2 | -0.2 | -0.2 | -0.2 | -0.2 | -0.2 | -0.2 | 1.3 |
| Länge fast zyl. Becherteil | -1.9 | -1.6 | -1.5 | -1.5 | -1.5 | -1.6 | -1.6 | -1.5 | -1.5 | -1.6 | -1.6 | -1.6 | -1.6 | -1.5 | -1.5 | 0.4 |
| Aufweitungsstelle (Becherlänge fest) | -0.6 | -0.4 | -0.2 | +0.0 | +0.3 | +0.5 | +0.5 | +0.3 | +0.1 | -0.1 | -0.1 | -0.1 | -0.1 | +0.0 | +0.2 | 1.1 |
| Becher-Bogenlänge | -3.0 | -2.8 | -2.7 | -2.9 | -3.2 | -3.6 | -3.4 | -3.2 | -3.0 | -3.0 | -3.0 | -3.0 | -2.9 | -2.9 | -3.1 | 0.9 |
| Gesamtlänge (L1–L4 + Becher) | -17.2 | -17.1 | -17.0 | -16.9 | -16.8 | -16.7 | -16.6 | -16.6 | -16.5 | -16.4 | -16.4 | -16.3 | -16.1 | -16.0 | -16.0 | 1.2 |
| alle Radien R1–Rand | +0.1 | +0.2 | +0.2 | +0.2 | +0.2 | +0.1 | +0.1 | +0.1 | +0.2 | +0.2 | +0.3 | +0.3 | +0.3 | +0.3 | +0.2 | 0.2 |

### Empfindlichkeit V2a H3 MP-a: Cent je +1 % (zentrale Differenz ±1 %), Spitzen 2–16

| Größe | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 | Spanne n2–16 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| R1 | -1.1 | -0.7 | -0.4 | +0.4 | +0.9 | +0.9 | +0.6 | +0.4 | +0.7 | +0.6 | +0.4 | +0.1 | +0.0 | +0.1 | +0.0 | 2.1 |
| Kesselvolumen | -0.2 | -0.2 | -0.4 | -0.6 | -0.6 | -0.5 | -0.3 | -0.2 | -0.4 | -0.3 | -0.3 | -0.2 | -0.1 | -0.1 | -0.1 | 0.5 |
| Rückbohrungslänge (Volumen) | -0.1 | -0.1 | -0.2 | -0.3 | -0.3 | -0.3 | -0.3 | -0.2 | -0.3 | -0.3 | -0.3 | -0.2 | -0.1 | -0.2 | -0.3 | 0.3 |
| Becher-Randradius | +0.8 | +2.1 | +1.5 | +0.4 | +0.2 | +0.2 | +0.3 | +0.0 | -0.1 | -0.0 | -0.0 | +0.0 | -0.7 | -0.2 | -0.0 | 2.8 |
| Länge fast zyl. Becherteil | -1.4 | -3.7 | -2.8 | -0.8 | -0.4 | -0.6 | -2.1 | -3.4 | -0.9 | -0.5 | -0.6 | -1.4 | -4.6 | -1.3 | -0.5 | 4.2 |
| Aufweitungsstelle (Becherlänge fest) | -0.4 | -0.9 | -0.5 | -0.0 | +0.1 | +0.2 | +0.7 | +1.0 | +0.1 | +0.0 | -0.0 | -0.1 | -0.2 | -0.0 | +0.0 | 1.9 |
| Becher-Bogenlänge | -2.2 | -6.3 | -4.8 | -1.5 | -0.9 | -1.3 | -4.8 | -7.5 | -1.8 | -0.9 | -1.0 | -2.6 | -8.6 | -2.5 | -1.0 | 7.7 |
| Gesamtlänge (L1–L4 + Becher) | -17.0 | -16.7 | -16.5 | -16.3 | -16.2 | -16.2 | -16.3 | -16.2 | -16.3 | -16.5 | -16.6 | -16.8 | -16.1 | -16.6 | -16.7 | 0.9 |
| alle Radien R1–Rand | +0.7 | +0.6 | +0.9 | +1.6 | +1.6 | +1.3 | +0.5 | -0.2 | +0.6 | +0.5 | +0.4 | +0.1 | -0.9 | -0.1 | -0.0 | 2.5 |

### Annahmen-Spannen (V1 H3 MP-a als Grundfall): Cent der Spitzen 2–16 gegen Grundfall

| Fall | Gesamtlänge (mm) | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | n12 | n13 | n14 | n15 | n16 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Rückbohrung 20 mm | 3224 | +2.1 | +3.4 | +4.8 | +5.9 | +6.7 | +6.9 | +7.8 | +8.6 | +9.8 | +10.7 | +11.8 | +13.0 | +14.6 | +16.5 | +17.4 |
| Rückbohrung 70 mm | 3274 | -3.2 | -5.4 | -7.8 | -9.7 | -11.1 | -11.6 | -13.4 | -15.2 | -18.0 | -19.9 | -22.7 | -25.1 | -27.8 | -28.5 | -26.9 |
| Becher x statt Bogen (492 statt 590 mm) | 3147 | +36.6 | +35.0 | +37.3 | +42.1 | +49.7 | +61.7 | +67.0 | +67.4 | +63.2 | +61.3 | +57.1 | +54.7 | +51.4 | +49.6 | +49.6 |
| Becherbogen +5 % | 3274 | -15.1 | -13.7 | -13.6 | -14.6 | -16.0 | -18.1 | -17.0 | -15.9 | -15.0 | -15.0 | -14.8 | -14.7 | -14.5 | -14.7 | -15.6 |
| Becherbogen −5 % | 3215 | +15.3 | +13.9 | +13.8 | +14.5 | +15.9 | +18.1 | +17.5 | +16.4 | +15.3 | +15.2 | +14.9 | +14.9 | +14.6 | +14.5 | +15.2 |

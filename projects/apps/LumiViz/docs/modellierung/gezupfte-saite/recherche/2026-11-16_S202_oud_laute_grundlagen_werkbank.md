> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_15_oud_laute_grundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-17 07:44–08:04, claude-opus-5-5, 108 Runden, success, 6.78 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Seitenbilder des Laufs im Archiv der Werkbank, `seiten/`):
> - **Woodhouse 2021 Abb. 9 b (S. 4382) am Seitenbild:** Q rund 840 außerhalb, Spitze 3000 gegenphasig, Senke rund 500
>   gleichphasig, Zone ±0,4 Cent — die gerechneten 844 / 492 / 3000 stimmen, die gerechnete Breite (±0,7 Cent) nicht,
>   wie der Bericht selbst sagt. Text „Q-factor of 3000“, c ≈ 300 N s/m bestätigt. Der Widerspruch in Abb. 8 a (die
>   Moden steigen mit rund 0,44 Hz je Cent statt 0,14) ist auch am Bild da und bleibt offen.
> - **Woodhouse & Lynch-Aird 2019 S. 524:** „essentially the same tension, around 30 N“ — gefunden. (In der Tabelle zu G1
>   steht „0,52 N bei 293,7 Hz“; gemeint ist der Durchmesser 0,52 mm.)
> - **Steg der Rauwolf-Laute (Santa María Bouquet 2020) am Bildausschnitt:** alle Maßzahlen (139 / 134 / 177 / 122,5;
>   10 → 7 breit; 7 / 6 und 8 / 7 hoch; Ohren 25,5 / 17,6; 12 / 10 mit Ohr) stimmen; die Masse (Quader 139 × 8,5 × 7 mm
>   bei 600 kg/m³ = 5,0 g) ist richtig gerechnet. Der Befund „S197 nahm 12 mm, das ist die Breite mit Ohr“ trägt.
> - **Inanli & Altinsoy 2018 S. 924:** wie im Kopf von Bericht 05 (stimmt; das PDF fehlt).
> - **Nicht gegengelesen:** W&LA 2019 Gl. 3–7 und Tab. I / II, Woodhouse 2021 Gl. 6–10, Chadefaux 2012, Scherrer 2013
>   S. 25 / 61 / 62, die Rechnungen am Prüfstand (Basschor doppelt +3,7–4,4 LU; Kernsteife ohne Wirkung).
> - **Nacharbeit:** Auftrag `2026-11-17_25_oud_laute_nacharbeit` der Werkbank.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_15_oud_laute_grundlagen.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_15_oud_laute_grundlagen/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_oud_laute_grundlagen_werkbank/`: `anhang.py`, `rauwolf_steg.py`, `rechnung_basschor.py`, `rechnung_bauplan.py`, `rechnung_choere.py`, `rechnung_rest.py`, `rechnung_saiten.py`.

# Bericht: Berechnungsgrundlagen Oud und Renaissancelaute — Bauplan, Saitensätze, Zupf, Chöre (Stufe 2)

- **Auftrag:** `auftraege/2026-11-16_15_oud_laute_grundlagen.md`
- **Datum:** 2026-11-17
- **Modell:** claude-opus-5-5
- **Voraussetzung:** erfüllt. `ergebnisse/2026-11-16_05_oud_laute_saiten_bau/bericht.md` liegt vor, und
  `laeufe/2026-11-16_05_oud_laute_saiten_bau.zustand.json` meldet „status: fertig“, `ende_art: success`.
- **Reichweite:**
  - **Keine Netzsuche und kein Netzabruf.** Alle Quellen, die die Stufe 1 aus dem Netz gelesen hat, stehen hier nur mit
    ihrer Stufe-1-Marke. Neu am Bild gelesen habe ich nur die Seite 924 von Inanli & Altinsoy 2018, und zwar über das
    Seitenbild, das die Stufe 1 abgelegt hat (`…_saiten_bau/seiten/inanli2018-2.png`).
  - **Lokale Quellen** unter `…\cmake\quellen\gezupfte-saite\`: neun PDFs durchsucht, 21 Seiten als Bild gelesen. Die
    Bilder liegen unter `seiten/`.
    - Woodhouse 2021 (JASA 150), S. 4376–4382;
    - Woodhouse & Lynch-Aird 2019 (Acta Acust. 105), S. 517, 518, 521, 524, 528;
    - Woodhouse u. a. 2012 (Acta Acust. 98), S. 477;
    - Chadefaux u. a. 2012 (JASA 131), S. 847, 850, 851, 852;
    - Scherrer 2013 (Diss. McGill), S. 25, 61, 62;
    - Santa Maria Bouquet 2020 (Rauwolf-Zeichnung): ganze Seite bei 20 dpi, Ausschnitt Steg bei 120 dpi;
    - nur durchsucht: Abdallah u. a. 2026, Perng u. a. 2010/2011, Bader u. a. 2025, Waltham u. a. 2008,
      Ducceschi u. a. 2026.
  - **LumiViz-Repo, nur gelesen:**
    - `Bauplan.hpp`, `make_oud.py`, `make_laute.py`;
    - die Berichte S196 und S197;
    - der Prüfstand `2026-11-15_S199_pruefstand_zupf.py` mit den Ausgaben `…_oud_je_taste.txt` und
      `…_laute_je_taste.txt`;
    - `recherche/2026-11-12_S195_laute_oud_quellen.md` (dort stehen die Werte aus EJ95A).
  - **Prüfstand gerechnet:** Ich habe ihn als Modul importiert, ohne Bytecode (`python -B`, `sys.dont_write_bytecode`). Im
    LumiViz-Repo ist dabei nichts geschrieben worden.
- **Nicht erreicht:** nichts. Netzquellen wurden nicht angefragt (siehe „Nicht Teil des Auftrags“).

**Worauf aufgebaut wird (nicht neu hergeleitet):**

- Woodhouse & Lynch-Aird 2019, Tab. I und II;
- D'Addario EJ95A: Stimmung, Zug und Durchmesser nach S195 O2;
- Abdallah u. a. 2026, Abb. 3, Abb. 6, Tab. 1–3 und der Bauplan, der daraus entstanden ist (S196);
- Woodhouse 2021, Abb. 1, und die Rauwolf-Zeichnung, beide wie in S197 gelesen;
- der Stufe-1-Bericht.

Die Gleichungen aus W&LA 2019 habe ich trotzdem am Seitenbild gegengelesen: Der Prüfstand nutzt sie, und der Auftrag
verlangt je Gleichung ein Zahlenbeispiel.

Marken wie in `CLAUDE.md`, dazu die Marken des Auftrags:

- `[am Seitenbild geprüft]`: Wortlaut oder Formel an der Seite als Bild gelesen;
- `[hergeleitet]`: von mir aus belegten Gleichungen abgeleitet;
- `[übertragen von …]`: belegt an einem anderen Instrument;
- `[Annahme]`;
- `[Stufe 1: …]`: die Marke, die der Stufe-1-Bericht der Angabe gegeben hat.

---

## 0 Kurz

1. **Basschor der Oud:** einzeln, wie er gebaut ist. Doppelt besaitet steigt er im Prüfstand um 3,7–4,4 LU.
   - Die Spanne der Oud sinkt dann von 15,8 auf 12,1 LU.
   - Der größte Sprung (Paare bis 4 Halbtöne) sinkt von 11,2 auf 8,6 LU.
   - Die Grundtöne C2–E2 klingen 1,3–2,0 dB schneller ab (Pegel 1–2 s).

   Fachlich bleibt der Chor einzeln. Alle gelesenen Elf-Saiten-Sätze haben ein einzelnes C (Stufe 1). Ein doppelter Chor
   würde die Zielgröße treffen, indem er das Instrument ändert (§5, Option O1).
2. **Umsponnene Saiten:** Ein belegtes Verlustmodell für Saiten mit Nylonkern gibt es in den gelesenen Quellen nicht.
   Woodhouse u. a. 2012 sagt sogar ausdrücklich, dass es für die Biegesteife keines gibt (§1, G9).
   - Die bisherige Übertragung von der Gitarre ist fachlich besser begründet als ein reines Luftreibungsmodell. Bei den
     umsponnenen Gitarrensaiten liegt der gemessene Verlust am Grundton rund sechsmal über der Luftreibung nach Stokes
     (§1, G10b).
   - Die Biegesteife aus dem Kern allein (23–31·10⁻⁶ N m² statt 50·10⁻⁶) ändert im Prüfstand nichts: Lautheit ±0,1 LU,
     Abklingen ±0,1 dB.
3. **Chöre:** Woodhouse 2021, Gl. 6–10, ist umgesetzt und trifft die Q-Werte seiner Abb. 9 genau (844 / 492 / 3000). Am
   Oud-Chor A2 entscheidet die Kopplungsbreite 2|ε| = 0,63 Hz (≈ 10 Cent) über das Abklingen, nicht die angenommene
   Verstimmung von ±1,5 Cent: der gleichphasige Anteil hat T60 3,7 s, der gegenphasige 12 s.
4. **Bauplan:**
   - Neu belegt ist der **Steg der Rauwolf-Laute** (Maßzahlen der Zeichnung, am Bild gelesen). Daraus ergeben sich rund
     4,5 g gestreckt statt der angenommenen 7 g.
   - Für die Oud gibt Inanli & Altinsoy 2018 eine **zweite, freie Decke mit acht gemessenen Moden**. An ihr lassen sich
     Dicke und Holzwerte prüfen; die heutige Oud-Decke stützt sich dafür nur auf vier Spitzen in Abb. 6.
   - Für keine der anderen Annahmen gibt es eine Zahl aus einer Quelle (§4).

---

## 1 Gleichungen

Je Gleichung: Quelle, gedruckte Seite und Gleichungsnummer, Marke und ein nachgerechnetes Zahlenbeispiel. Die
Skripte und ihre Ausgaben stehen im Anhang.

### Saite

**G1 Grundfrequenz, Zug, Massenbelag** (W&LA 2019, Gl. 3, S. 517) `[am Seitenbild geprüft]`

f₁² ≈ (T/m)·[1/(2L)]², also T = 4 μ L² f₁².

*Beispiel (eigene Rechnung, gegen die Quelle):* die Laute aus Tab. II mit L = 0,625 m, Nylon ρ = 1070 kg/m³.

| Chor | Saite | gerechneter Zug |
|---|---|---|
| 2 | 0,52 N bei 293,7 Hz | 30,6 N |
| 3 | 0,725 N | 33,4 N |
| 4 | 0,900 N | 32,4 N |

Die Quelle schreibt dazu (S. 524): „the 2nd, 3rd and 4th courses (all in nylon) have essentially the same tension,
around 30 N“ `[am Seitenbild geprüft]`. Das passt innerhalb von 2–11 %.

**G2 Biegeanteil λ** (W&LA Gl. 4/7, S. 517) `[am Seitenbild geprüft]`

λ = E π² d² n² / (64 ρ L⁴ f₁² + E π² d² n²).

**G3 Teiltöne** (Gl. 5/6, S. 517) `[am Seitenbild geprüft]`

fₙ² ≈ n² f₁² / (1 − λ), also fₙ ≈ n f₁ (1 + λ/2).

*Beispiel:* Laute Chor 8, Fluorcarbon 1,60 mm: λ₁ = 1,0·10⁻³, λ₁₀ = 0,093. Der zehnte Teilton liegt also um 4,7 %
(≈ 80 Cent) zu hoch.

**G4 Wellenwiderstand** (Gl. 8, S. 517) `[am Seitenbild geprüft]`

Z₀ = √(T m) ≈ π d² ρ L f₁ / 2.

*Beispiele:* Laute Chor 2: 0,083 N s/m. Oud C2: 0,457 N s/m.

**G5 Luftreibung** (Gl. 11, 12, S. 518) `[am Seitenbild geprüft]`

- η_air ≈ (ρ_a/ρ)·(2√2 M + 1)/M², mit M = (d/4)·√(2π fₙ/η_a) und ρ_a = 1,2 kg/m³, η_a = 1,5·10⁻⁵ m²/s.
- Dazu der Zuschlag (S. 518): „applying an ad hoc correction factor (d + 0.2) to η_air, with the string diameter d
  expressed in mm“.
- Gl. 13 ist die Näherung für großes M.

*Beispiel:* Laute Chor 2 am Grundton: η_air = 1,97·10⁻³ (mit Zuschlag).

**G6 Verlust aus der Biegung** (Gl. 14, S. 518) `[am Seitenbild geprüft]`

η_bend ≈ λ η_E, mit η_E = E″/E′.

**G7 Gesamtverlust** (Gl. 15, S. 518) `[am Seitenbild geprüft]`

ηₙ ≈ η_body + η_air + η_bend.

η_body ist dort ein SEA-Mittelwert. In LumiViz kommt er aus der Stegadmittanz (G11).

**G8 Elastizitätsmodul aus der Spannung** (Gl. 16, 17, Tab. I, S. 519–521) `[am Seitenbild geprüft]`

- Nylon: E′ = 4,5 + 39 σ GPa; E″ = 0,25 GPa; ρ = 1070 kg/m³.
- Fluorcarbon: E′ = 3,2 + 41 σ GPa; E″ = 0,18 GPa; ρ = 1800 kg/m³.
- σ in GPa.

*Beispiel:* Oud C4 aus EJ95A, 0,660 mm, L = 0,60 m: σ = 0,106 GPa, also E = 8,6 GPa und EI = 80·10⁻⁶ N m².

> Abweichung im LumiViz-Stand: S197 setzt Fluorcarbon mit 1780 kg/m³ an, der Prüfstand mit 1800 (wie Tab. I).

**G9 Verlust nach Valette** (Woodhouse u. a. 2012, Gl. 4, S. 477) `[am Seitenbild geprüft]`

ηₙ = [T (η_F + η_A/ωₙ) + EI η_B (nπ/L)²] / [T + EI (nπ/L)²].

Auf derselben Seite steht wörtlich:

> „For overwound strings, as normally used for the lower strings of the guitar, the effective value of EI or B cannot
> be reliably predicted, and must be measured.“

*Beispiel* mit den übertragenen Werten (EI 50·10⁻⁶, η_F 5·10⁻⁵, η_B 0,02, η_A 1,1): Oud C2 am Grundton η = 2,73·10⁻³
(Q 367, T60 der Saite allein 12,3 s), beim zehnten Teilton 3,9·10⁻⁴.

**G10 Massenbelag einer umsponnenen Saite aus dem Aufbau** `[hergeleitet]`

Ein Draht der Stärke w ist dicht an dicht um einen Kern d_c gewickelt, D = d_c + 2w:

μ = ρ_K φ π d_c²/4 + ρ_Cu (π w²/4)·π (d_c + w)/w.

Das gilt unter drei Annahmen: ein Draht mit rundem Querschnitt, keine Lücke zwischen den Windungen, Füllgrad φ des
Faserkerns. Woodhouse & Lynch-Aird 2019 nennen als Zweck der Umspinnung (S. 528) „to increase the mass per unit length
while limiting the effects of bending stiffness“ `[am Seitenbild geprüft]`.

*Beispiel:* §2.3.

**G10b Luftreibung nach Stokes am Außendurchmesser** (G5 mit ρ = μ/(πD²/4)) `[übertragen von monofilen Saiten]`

*Beispiel, Oud C2:* η_air = 4,8·10⁻⁴ gegen 2,73·10⁻³ aus der Valette-Übertragung.

*Gegenprobe an der Gitarre* `[eigene Rechnung; Außendurchmesser 1,12 mm angenommen]`: E2-Saite mit m = 6,24 g/m (Woodhouse
u. a. 2012, Tab. I).

- Stokes: η_air ≈ 4,0·10⁻⁴.
- Gemessen: η_F + η_A/ω = 2·10⁻⁵ + 1,2/518 = 2,3·10⁻³, also das **5,9-Fache**.

An der Gitarre steckt also der größere Teil des Verlusts tiefer Töne in der Bauart der Saite und nicht in der Luft. Das
passt zu „tight windings introduce a new damping mechanism associated with dry friction“ (W&LA S. 528, nach Valette).

### Steg und Chöre

**G11 Verlust über den Steg** (Woodhouse 2021, Gl. 1, S. 4376) `[am Seitenbild geprüft]`

η_b ≈ 2 Re(Y) / (π n Y₀ⱼ), mit Y₀ⱼ = (Tⱼ mⱼ)^(−1/2) = 1/Z₀.

*Beispiel:* Oud A2 an `oud.lvinstbody`, Stegpunkt Taste 45. Mit Re Y = 1,25·10⁻² m/(s N) und Z₀ = 0,241 ergibt sich
η_b = 1,9·10⁻³.

**G12 Luft- und Biegeverlust** (Woodhouse 2021, Gl. 2–4, S. 4376) `[am Seitenbild geprüft]`

Dieselbe Form wie G5/G6, aber **ohne den Zuschlag (d + 0,2)**.

**G13 Notwendige Bedingung für zweistufiges Abklingen** (Gl. 5, S. 4377) `[am Seitenbild geprüft]`

η_b ≫ η_air (am Grundton).

Wie groß „≫“ sein muss, ist offen: „it is not at present possible to put a numerical value on the requirement“.

**G14 Chor statt Einzelsaite** (S. 4378) `[am Seitenbild geprüft]`

„The curve is simply raised by 6 dB because the combined admittance of the pair of strings is halved (or the impedance
is doubled).“

Für zwei gleiche Saiten im Einklang gilt also η_b, gleichphasig = 2 η_b, einzeln und η_b, gegenphasig ≈ 0.

**G15 Drei-Freiheitsgrade-Modell** (Woodhouse 2021, Gl. 6–10, S. 4380–4381) `[am Seitenbild geprüft]`

Zwei Saiten und eine Körpermode, mit q = [a, b₁, b₂]ᵗ:

- Gl. 6: wⱼ = bⱼ sin(πx/L) + a x/L;
- Gl. 7: K = diag(k, π²T₁/2L, π²T₂/2L);
- Gl. 8: M = [[m + L(m₁+m₂)/3, m₁L/π, m₂L/π], [m₁L/π, m₁L/2, 0], [m₂L/π, 0, m₂L/2]];
- Gl. 9: C = diag(c, c₁, c₂);
- Gl. 10: cⱼ = π √(Tⱼ mⱼ) / (2 Qⱼ).

Gelöst wird es als Eigenwertproblem erster Ordnung (6×6).

*Gegenprobe an der Quelle* (Fallstudie S. 4381, Abb. 9): Stahl 0,45 mm, L = 0,65 m, B3, Q 3000 der Saite, Dämpfer
c = 300 N s/m.

- Gerechnet: Q = 844 bei Verstimmung (beide Moden), 492 gleichphasig und 3000 gegenphasig im Einklang.
- Abb. 9b zeigt rund 840 / 500 / 3000 `[eigene Ablesung]`.
- **Die Q-Werte stimmen.** Die Breite der Zone, in der sich die beiden Moden trennen, ist es nicht: gerechnet ±0,7 Cent,
  in Abb. 9b rund ±0,4 Cent.
- In Abb. 8 (Körper 0,2 kg, Q 40) steigen die Frequenzen der Moden je Cent rund dreimal steiler, als es eine Verstimmung
  um c Cent erlaubt (0,14 Hz je Cent bei 247 Hz). Wie die Cent-Achse gemeint ist, sagt der Text nicht. **Nicht
  aufgelöst**, siehe §6.

**G16 Zwei Saiten an einer komplexen Stegadmittanz** (Form nach Weinreich) `[hergeleitet]`

Weinreich 1977 habe ich nicht gelesen; Woodhouse 2021 zitiert ihn auf S. 4375 und S. 4380.

1. Eine Saite, deren Ende am Punkt mit Admittanz Y hängt, liefert mit e^{iωt} und kleinem Z₀Y: tan(kL) = i Z₀ Y, also
   ω = ωₙ (1 + i Z₀ Y/(nπ)).
2. Der Imaginärteil gibt genau G11 wieder (Abklingrate σ = ωₙ Z₀ Re Y/(nπ), η = 2σ/ωₙ). Der Realteil verschiebt die
   Frequenz um −ωₙ Z₀ Im Y/(nπ). Eine massenartige Admittanz (Im Y < 0) hebt die Frequenz.
3. Zwei Saiten am selben Punkt:
   - mit ε = i ω̄ Z₀ Y/(nπ) und Δ = (ω₁ − ω₂)/2 gilt λ± = ω̄ + ε ± √(Δ² + ε²);
   - mit den Verlusten der Saiten: sⱼ = ωⱼ (1 + i ηⱼ/2) auf der Diagonalen;
   - im Einklang: λ₊ = ω̄ + 2ε (gleichphasig, doppelte Kopplung, wie G14), λ₋ = ω̄ (gegenphasig);
   - bei |Δ| ≫ |ε| hat jede Saite ihre eigene Kopplung.
4. Die **Kopplungsbreite** ist 2|ε|/2π in Hz. Darunter ist die Verstimmung für das Abklingen fast gleichgültig, darüber
   zählt sie.
5. Mit Y = 1/c gibt diese Form die Werte von G15 exakt wieder: 844 / 492 / 3000 und die Breite 1,47 Cent gesamt.
   Gegenprobe B in `rechnung_choere.py`.

Der Prüfstand rechnet die Chöre schon voll (`ton_mit`: Y-Matrix zwischen den Saiten, zwei Polarisationen). G16 dient
nur als Formel zum Abschätzen und Prüfen.

### Zupf

**G17 Kraftstufe beim Loslassen aus der Ruhe** (Prüfstand, Kopf) `[hergeleitet, Statik]`

F₀ = T d L / (x_p (L − x_p)).

Zur Gegenprobe W&LA Gl. 9 (S. 517) `[am Seitenbild geprüft]`: in der Saitenmitte F_p ≈ 4 T δ / L. Bei x_p = L/2 sind
beide Formeln gleich.

*Beispiel:* Oud C4 mit T = 36,1 N, L = 0,60 m, x_p = 0,16 m, d = 2 mm: F₀ = 36,1·0,002·0,60/(0,16·0,44) = 0,62 N.

**G18 Breite der Kontaktstelle als Tiefpass**

- Woodhouse u. a. 2012 (S. 477) `[am Seitenbild geprüft]`: „proportional to exp(−ω²/K²). Here, K = v/h where v = √(T/m)
  … and the width h was given the value 7.5 mm in order to obtain a reasonably realistic sound.“
- Scherrer 2013 (S. 61) `[am Seitenbild geprüft]`: Grenzfrequenz f_c = c/w, nach Chaigne & Kergomard 2008.
- *Beispiel:* Oud C4, c = √(36,1/0,367·10⁻³) = 314 m/s, h = 7,5 mm: K/2π = 6,7 kHz.

**G19 Dauer des Loslassens als Tiefpass** (Scherrer 2013, S. 61) `[am Seitenbild geprüft]`

Je Teilton wird die Amplitude mit sin(π fₖ t_ex)/(π fₖ t_ex) multipliziert, nach Chaigne & Kergomard 2008. Dort steht
auch „Assuming a typical duration t_ex ≃ 0.02 s … the cut-off frequency is around 50 Hz.“

*Beispiel:* t_ex = 0,5 ms (Gleiten an der Gitarre, Pavlidou nach Chadefaux 2012, S. 850) gibt die erste Nullstelle bei
2 kHz.

**G20 Anfangsform und Anfangsgeschwindigkeit** (Chadefaux u. a. 2012, Gl. 2–4, S. 847) `[am Seitenbild geprüft]`

- Aₙ = 2 x(y₀,0) sin(kₙ y₀) / (kₙ² y₀ (L − y₀)) und Bₙ = 2 ẋ(y₀,0) sin(kₙ y₀) / (kₙ³ y₀ (L − y₀) c).
- Wörtlich: „It is usually assumed that the initial velocity of a string plucked with a thin plectrum is negligible
  (Bₙ = 0), whereas the initial shape is a triangle“.
- Im Prüfstand entspricht das dem Parameter `tempo` in `ton_mit`.

### Körper

**G21 Helmholtz bei starrer Wand** `[Lehrbuchform; Mündung wie Bauplan.hpp]`

f_H = (c/2π) √(Σ Aᵢ/lᵢ / V), mit lᵢ = t + 1,7 rᵢ.

*Beispiele:*

- Oud: 20,61 l, Löcher R 5,3 / 2,0 / 2,0 cm → 155 Hz. Mit den Löchern von Inanli (Ø 85 / 42 / 42 mm) → 148 Hz.
- Laute: 11,0 l, Rose 85 % offen → 151 Hz.

**G22 Zwei gekoppelte Schwinger** (S196 §5) `[aus S196 übernommen, nicht an einer Quelle der Ablage gelesen]`

f₋² + f₊² = f_Luft² + f_Decke².

*Beispiele:*

- Oud gemessen 101 / 203 Hz mit f_Luft = 155 Hz → entkoppelte Decke 165 Hz. Der Bauplan rechnet für die Platte allein
  143 Hz (1,5 mm, S196 §7). Die Formel stimmt hier nur auf rund 15 %; drei Löcher und mehrere Deckenmoden machen sie
  ungenau.
- Laute gemessen 132 / 277 Hz mit f_Luft = 151 Hz → 267 Hz.

---

## 2 Parametertabellen

### 2.1 Bauplan Oud (je Größe aus `Bauplan.hpp`)

„Heute“ ist `make_oud.py`. „A26“ ist Abdallah u. a. 2026, „I18“ ist Inanli & Altinsoy 2018 (DAGA, S. 923–924; eine
türkische Decke, frei gelagert, Erbauer nicht genannt). Zwei verschiedene Instrumente: **keine Größe ist an einem einzigen
Instrument vollständig belegt.**

| Größe (Bauplan.hpp) | heute | Quelle, Ort | Marke | Gültigkeit / Alternative |
|---|---|---|---|---|
| `materials[].density` | 460 kg/m³ | A26 Tab. 1 | [schon belegt, S196] | I18: „Boróka spruce“, ohne Kennwerte [Stufe 1: wörtlich] |
| `ex` / `ey` | 10,34 / 0,80 GPa | A26 Tab. 1 (x = L, y = R) | [schon belegt] | Sitka-Fichte; I18 ohne Werte |
| `gxy` / `gxz` / `gyz` | 0,63 / 0,66 / 0,03 GPa | A26 Tab. 1 | [schon belegt] | gyz = G_RT 0,03 GPa ist sehr klein; der Wert steht so in A26 |
| `nuxy` | 0,467 | A26 Tab. 1 | [schon belegt] | – |
| `lossFactor` | 0 (das Gesetz `damping` gilt) | – | – | Ein Verlustfaktor für Fichte ist in der lokalen Ablage nicht gefunden (§7). Die gemessene modale Dämpfung (Abb. 6) ist fachlich näher am Instrument als ein Holzwert |
| `plate.grainAngle` | 90° (Faser längs) | A26 S. 5 (in S196 zitiert) | [schon belegt] | – |
| `plate.outline` | 50,0 × 37,0 cm, 1463 cm² | A26 Abb. 3 | [schon belegt] | I18: 490 × 360 mm (Maßzahlen Abb. 2) [am Seitenbild geprüft]; Fläche ≈ 5 % kleiner [eigene Rechnung, bei gleicher Form: 49·36 / 50·37] |
| `plate.thickness` | 1,5 mm | – | [Annahme] | I18: „varying between 1.5-2.0 mm“, an der Decke „uniform thickness of 2.0 mm“ [Stufe 1: wörtlich, S. 923] |
| `plate.thicknessGrid` | leer | – | – | I18: „Decreasing thickness profile may also be applicable from bridge location to the edge“, ohne Zahl [Stufe 1: wörtlich] → **bleibt** |
| `plate.holes` | R 5,3 cm bei y 30,0; R 2,0 cm bei y 20,4 und x ±10,3 cm | A26 Abb. 3 | [schon belegt] | I18: Ø 85 / 42 / 42 mm, die kleinen 160 mm auseinander, 80 mm unter dem großen [am Seitenbild geprüft]. Im Raster von Abb. 4 (Beschriftung „mm“, die Zahlen 0–50 sind aber cm): großes Loch bei y ≈ 29–30, kleine bei y ≈ 21–22 und x = ±8 [eigene Ablesung, ±1 cm]. Die Lesart der Stufe 1 ist bestätigt |
| `ribs` (7) | Lage aus A26 Abb. 3, Höhe 16,5 mm in der Mitte, Enden 7–10 mm, Breite 4,2–6,6 mm; Masse 43 g | A26 Tab. 2, Abb. 3 | [schon belegt] | I18 Tab. 1 [am Seitenbild geprüft]: Höhe 12 mm (Balken 3: 5 mm, so in der Tabelle), Enden 6 mm, Breite 5–6 mm, Längen 118–355 mm; L1 = L2 + 2 L3 für alle sieben (geprüft); Masse 49 g bei 460 kg/m³ [eigene Rechnung]. **Lage bei I18 nicht als Zahl** |
| `bridges` | keiner | – | – | Lage: A26 9 cm über dem unteren Ende; I18 Abb. 4 bei y ≈ 8,5–10,5 cm [eigene Ablesung]. **Länge:** I18 Abb. 4 ≈ 7–8 cm, Abb. 2 (Verhältnis zur Breite 360 mm) ≈ 11 cm [eigene Ablesung, ±2 cm]; A26 Abb. 3 / 5 b 14–17 cm (S196). Masse, Höhe, Holz: **nicht gefunden** |
| `bridgePoints` | Tasten 36–60, y 0,09 m, Abstand 11 mm | Stimmung EJ95A | Abstand [Annahme] | Abstand der Chöre: **nicht gefunden** |
| `listeningPoints` | 0,5 m vor der Decke | – | [Annahme] | nicht Gegenstand des Auftrags |
| `backside` | aus | – | fachlich richtig (geschlossene Schale) | – |
| `damping` | α = 0,06 f + 23 1/s | A26 Abb. 6, Halbwertsbreiten | [schon belegt, S196] | über 400 Hz fortgeschrieben; I18 nennt keine Dämpfung |
| `cavity.volume` | 20,61 l | Tiefe 21 cm aus A26 Abb. 3; Querschnitt angenommen | teilweise [Annahme] | Volumen in keiner Quelle; Helmholtz-Gegenprobe G21/G22 |
| `cavity.endCorrection` / `q` | 1,7 / 30 | – | [Annahme] | – |
| `cavity.openings` | keine | – | richtig | – |
| `rim` | eingespannt | – | [Annahme] (wie das FE-Modell von A26) | – |

**Gegenrechnungen** `[eigene Rechnung, rechnung_bauplan.py]`

- **Helmholtz:** starr 155 Hz (A26-Löcher) bzw. 148 Hz (I18-Löcher). Gekoppelt rechnet der Bauplan 99,9 Hz gegen 101 Hz
  gemessen (S196).
- **Masse der Decke:** 93 g (1,5 mm) bzw. 124 g (2,0 mm), Balken 43 bzw. 49 g. **Eine gemessene Deckenmasse ist in keiner
  Quelle gefunden.**

**Ersetzt, teilweise, bleibt (Oud)**

| Annahme heute | Stand |
|---|---|
| Mensur 0,60 m | **teilweise:** 585 (türkisch), 610–620 (arabisch) [Stufe 1, I18]; Pyramid „60 - 62 cm“. Gegenprobe EJ95A [eigene Rechnung]: Der Zug von G3 (Nylon 0,787 mm) passt zu L = 0,594 m (S195), und der Mittelwert über die elf Saiten ist 3,38 kgf. Das stimmt mit Aquilas „3.4 Kg average“ überein. Die Mensur des A26-Instruments bleibt unbekannt |
| Deckendicke 1,5 mm | **teilweise:** I18 1,5–2,0 mm, die dokumentierte Decke 2,0 mm; eine Messung am A26-Instrument fehlt |
| Querschnitt der Schale | **bleibt** |
| fehlender Steg | **teilweise:** Lage doppelt belegt; die Länge widerspricht sich (7–17 cm); Masse und Höhe fehlen |
| Abstand der Chöre | **bleibt** |
| Balken | **teilweise:** A26 ganz (Lage und Querschnitt), I18 nur Querschnitte. Die Spanne der Querschnitte ist jetzt sichtbar: Mitte 12 gegen 16,5 mm, Enden 6 gegen 7–10 mm |
| Schalllöcher | **ersetzt** (A26 Maßzahlen); I18 bestätigt die Anordnung, die Löcher sind dort kleiner (Ø 85 statt 106 mm) |
| Volumen | **bleibt** (Tiefe belegt, Querschnitt nicht) |

### 2.2 Bauplan Laute

„R“ ist die Rauwolf-Zeichnung (Santa Maria Bouquet 2020). „W21“ ist Woodhouse 2021, Abb. 1 (eine andere Laute). Alle
Längen werden mal 625/677,4 = 0,9227 genommen (KQ12 a).

| Größe | heute | Quelle | Marke | Alternative / Stand |
|---|---|---|---|---|
| `materials` spruce | Sitka wie bei der Oud | – | [Annahme, übertragen von A26] | FEMTO 2019 (Laux-Maler, Folie 15): Holzwerte je Teil [Stufe 1: wörtlich, nicht gegengeprüft, nicht lokal] → Ladeliste |
| `materials` pear (Steg) | 700 kg/m³, E 8 GPa | – | [Annahme] | Holz des Stegs: Pflaume, Birne oder Nussbaum [Stufe 1: wörtlich, Van Edwards]; Kennwerte nicht gefunden |
| `plate.outline` | R, gestreckt; 1263 cm² am Original, 1075 cm² gestreckt | R (Vektor) | [schon belegt, S197] | – |
| `plate.thickness` | 1,8 mm | – | [Annahme] | „often about 1.5mm“ [Stufe 1: wörtlich, Van Edwards]; 1,9–2,1 mm an einem Nachbau [Stufe 1: wörtlich, Schreiner] |
| `plate.holes` (Rose) | Ø 110 mm, Mitte 287,4 mm, 85 % offen | R (Maßzahlen) | Lage belegt; offener Anteil [Annahme] | offener Anteil **nicht gefunden** |
| `ribs` (7) | 128 / 190 / 228 / 287,4 / 347 / 392 / 432 mm, 14 mm hoch (der zweite 9,8 mm), Enden 5, Breite 4,5 mm | – | [Annahme] | Jauch-Laute (Barock): Abstände 32 / +76 / +59 / +50 mm, der vierte Balken durch die Mitte der Rose [Stufe 1: wörtlich, Schreiner]; Querschnitte nur am Nachbau (7,5 × 3,9 und 15,8 × 4,2 mm) |
| `bridges` | 139 mm lang, 12 mm breit, 7 g, Höhe aus der Masse | R: Länge; Masse [Annahme] | – | **neu, am Bild gelesen (`seiten/rauwolf_steg.png`)**, siehe den Absatz unter der Tabelle |
| `bridgePoints` | 8 Chöre auf 122,5 mm | R: Spanne der Saitenlöcher (11 Chöre) | Zuordnung [Annahme] | – |
| `damping` | α = 60 1/s | an W21 Abb. 1 angepasst | [schon belegt, S197] | – |
| `cavity.volume` | 14,0 l → 11,0 l gestreckt | R, sieben Querschnitte | [schon belegt, S197] | – |
| `cavity.endCorrection` / `q` | 1,7 / 12 | q an die Höhe bei 132 Hz angepasst | [Annahme] | – |
| `rim` | eingespannt | – | [Annahme] | – |

**Steg der Rauwolf-Laute** (aus der Zeichnung, alle Maße in mm)

| Maß | Bassseite | Diskantseite |
|---|---|---|
| Länge der Basis | 139 | |
| Länge der Oberkante | 134 | |
| Länge mit Ohren | 177 | |
| Spanne der Saitenlöcher | 122,5 | |
| Breite (Draufsicht) | 10 | 7 |
| Höhe vorn | 7 | 6 |
| Höhe hinten | 8 | 7 |
| Ohren | 25,5 | 17,6 |
| Breite mit Ohr | 12 | 10 |

- Dass links die Bassseite ist, schließe ich aus den Saitenlöchern: links sind sie größer, und die Zeichnung vermerkt
  „1mm on the treble to approx 2mm on the bass“ [eigene Lesart].
- **S197 nahm 12 mm als Breite des Stegs. Die 12 ist aber das Maß einschließlich des Ohrs**; der Steg selbst ist 10 → 7 mm
  breit.
- Masse [eigene Rechnung, Quader 139 × 8,5 × 7 mm]: 5,0 / 5,8 / 6,2 g bei 600 / 700 / 750 kg/m³ am Original, **3,9–4,9 g
  gestreckt** (heute 7 g).

**Ersetzt, teilweise, bleibt (Laute)**

| Annahme heute | Stand |
|---|---|
| Deckendicke 1,8 mm | **teilweise** (1,5 mm „often“; 1,9–2,1 mm am Nachbau); ein Verlauf an einer Renaissancelaute fehlt |
| Lage und Maße der sieben Balken | **teilweise, nur die Ordnung** (Jauch, Barock); keine Querschnitte historischer Balken |
| Rose | Durchmesser und Lage **belegt**; offener Anteil **bleibt** |
| Steg | **ersetzt bis auf die Dichte**: Maße aus R, Masse 3,9–4,9 g statt 7 g |
| Holz | **bleibt** (Sitka übertragen) |

### 2.3 Saitensatz Oud (D'Addario EJ95A, arabisch C2 F2 A2 D3 G3 C4; L = 0,60 m [Annahme, Spanne 0,585–0,615])

Quelle der Zahlen: `rechnung_saiten.py`, `rechnung_choere.py`. Die Marken je Spalte:

- Ton, Durchmesser, Zug: [Stufe 1 / S195, Hersteller];
- μ, Z₀, η: [eigene Rechnung mit G1–G10];
- Re Y und η_b: am LumiViz-Körper `oud.lvinstbody`.

| Chor | Ton | Saiten | Werkstoff | D (mm) | Zug (N) | μ (g/m) | ρ_eff (kg/m³) | Z₀ (N s/m) | EI (10⁻⁶ N m²) | η Saite, Grundton | Re Y am Stegpunkt (m/(s N)) | η_b einzeln (G11) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | C4 261,6 Hz | 2 | Nylon | 0,660 | 36,1 (aus G1) | 0,367 | 1070 | 0,115 | 80 | 1,90·10⁻³ (G5+G6) | 1,83·10⁻¹ | 1,3·10⁻² |
| 2 | G3 196,0 Hz | 2 | Nylon | 0,787 | 28,8 (EJ95A 28,2) | 0,521 | 1070 | 0,123 | 128 | 2,11·10⁻³ | 1,16·10⁻² | 9,1·10⁻⁴ |
| 3 | D3 146,8 Hz | 2 | Nylonkern, Cu versilbert | 0,610 | 39,6 | 1,274 | 4370 | 0,225 | 50 übertr. / 25 Kern | 1,24·10⁻³ (G9 übertr.) | 9,8·10⁻³ | 1,4·10⁻³ |
| 4 | A2 110,0 Hz | 2 | ebenso | 0,711 | 31,8 | 1,824 | 4590 | 0,241 | 50 / 29 | 1,64·10⁻³ | 1,25·10⁻² | 1,9·10⁻³ |
| 5 | F2 87,3 Hz | 2 | ebenso | 0,813 | 28,0 | 2,555 | 4930 | 0,268 | 50 / 31 | 2,06·10⁻³ | 6,4·10⁻³ | 1,1·10⁻³ |
| 6 | C2 65,4 Hz | **1** | ebenso | 1,092 | 35,9 | 5,825 | 6220 | 0,457 | 50 / 23 | 2,73·10⁻³ | 8,0·10⁻⁴ | 2,3·10⁻⁴ |

- **μ aus dem Aufbau gegen μ aus T, f, L** (G10, Kern Nylon 1140 kg/m³ mit Füllgrad 0,8, Kupfer 8960 kg/m³; alle drei
  Werte Annahmen) [eigene Rechnung]. Der μ-Wert aus G1 verlangt:

  | Saite | Draht (mm) | Kern (mm) |
  |---|---|---|
  | D3 | 0,10 | 0,40 |
  | A2 | 0,13 | 0,45 |
  | F2 | 0,17 | 0,48 |
  | C2 | 0,35 | 0,40 |

  Die Werte sind plausibel. Bei C2 (Draht dicker als der halbe Kern) liegt eine zweite Lage nahe [eigene Einschätzung].
  **Kerndurchmesser und Drahtstärke stehen in keiner gelesenen Quelle** — die Gegenrechnung ist deshalb keine Prüfung,
  sondern eine Auflösung nach den Unbekannten.
- **Diskant:** EJ95A gibt 2,879 kgf für G3. G1 mit Nylon gibt 2,94 kgf bei 0,60 m und 2,79 kgf bei 0,585 m. Für C4 gibt
  EJ95A „0.0“.
- **Mensur:** zwischen 0,585 und 0,615 m ändert sich μ der umsponnenen Saiten um ±5 % (ρ_eff 4150–4590 bei D3).

### 2.4 Saitensatz Laute (W&LA Tab. II, 8 Chöre in G, L = 0,625 m; Werkstoffe Tab. I)

Alle Zahlen [eigene Rechnung mit G1–G8 aus den Durchmessern der Quelle]. Re Y kommt aus `lute.lvinstbody` (§C in
`rechnung_choere.py`).

| Chor | Saiten (d mm, W, Hz) | Zug (N) | μ (g/m) | EI (10⁻⁶ N m²) | λ₁ / λ₁₀ | η Grundton | η 10. Teilton | Z₀ | Re Y (m/(s N)) | η_b einzeln |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 0,475 N 392,0 | 45,5 | 0,190 | 36 | 2,0·10⁻⁵ / 0,002 | 1,73·10⁻³ | 5,1·10⁻⁴ | 0,093 | 5,1·10⁻³ | 3,0·10⁻⁴ |
| 2 | 2 × 0,52 N 293,7 | 30,6 | 0,227 | 36 | 3,0·10⁻⁵ / 0,003 | 1,97·10⁻³ | 6,1·10⁻⁴ | 0,083 | 2,0·10⁻² | 1,1·10⁻³ |
| 3 | 2 × 0,725 N 220,0 | 33,4 | 0,442 | 104 | 7,9·10⁻⁵ / 0,008 | 2,03·10⁻³ | 8,2·10⁻⁴ | 0,121 | 7,1·10⁻³ | 5,5·10⁻⁴ |
| 4 | 2 × 0,900 N 174,6 | 32,4 | 0,681 | 209 | 1,6·10⁻⁴ / 0,016 | 2,15·10⁻³ | 1,2·10⁻³ | 0,149 | 2,1·10⁻³ | 1,9·10⁻⁴ |
| 5 | 2 × 0,91 F 130,8 | 31,3 | 1,171 | 174 | 1,4·10⁻⁴ / 0,014 | 1,51·10⁻³ | 9,0·10⁻⁴ | 0,191 | 3,1·10⁻² | 3,8·10⁻³ |
| 6 | 1,25 F 98,0 + 0,775 N 196,0 | 33,1 / 30,3 | 2,209 / 0,505 | 516 / 124 | 3,9·10⁻⁴ / 0,038 | 1,62·10⁻³ | 2,0·10⁻³ | 0,271 | 7,6·10⁻⁴ | 1,3·10⁻⁴ |
| 7 | 1,40 F 87,3 + 0,875 N 174,6 | 33,0 / 30,6 | 2,771 / 0,643 | 769 / 187 | 5,9·10⁻⁴ / 0,056 | 1,69·10⁻³ | 2,9·10⁻³ | 0,302 | 4,1·10⁻⁴ | 7,9·10⁻⁵ |
| 8 | 1,60 F 73,4 + 0,925 N 146,8 | 30,5 / 24,2 | 3,619 / 0,719 | 1229 / 212 | 1,0·10⁻³ / 0,093 | 1,82·10⁻³ | 4,9·10⁻³ | 0,332 | 2,1·10⁻⁴ | 4,4·10⁻⁵ |

Die Chöre sind nach W&LA nummeriert (1 = Diskant, einzeln). `rechnung_choere.py` zählt umgekehrt, nach dem Prüfstand
(dort ist Chor 1 = Taste 38). Re Y und η_b sind hier in die Nummerierung der Quelle umgestellt: Chor 1 = Taste 67 …
Chor 8 = Taste 38. η_b gilt für die erste (tiefere) Saite des Chors.

### 2.5 Zupf (Risha und Fingerkuppe)

**Für Oud und Laute ist nichts gemessen gefunden** (Stufe 1, §2.3). Alles in dieser Tabelle ist `[übertragen von …]` oder
eine Annahme.

| Größe | Wert | Quelle, Ort | Marke | wofür |
|---|---|---|---|---|
| Zupfort | 0,16 m vom Steg | Pavlidou 1997 nach Scherrer 2013, Tab. 2.1, S. 25 | [übertragen von Gitarre, am Seitenbild geprüft] | heute für Oud und Laute |
| | 0,33–0,40 der Länge vom Resonanzboden | Chadefaux u. a. 2012, Abb. 7, S. 851 | [übertragen von Harfe, am Seitenbild geprüft] | Fingerkuppe |
| Auslenkung vor dem Loslassen | 0,8 mm „soft“ / 4 mm „strong“ bei 16 cm | Scherrer Tab. 2.1, S. 25 | [übertragen von Gitarre] | Stärke; heute 2 mm |
| Auslenkung / Geschwindigkeit beim Loslassen | Mittel 4 mm und 2 m/s; Grenzen 0,5 mm / 0,2 m/s und 7 mm / 5 m/s; Anteil der Bewegungsenergie 15–37 % | Chadefaux 2012, S. 852 | [übertragen von Harfe, am Seitenbild geprüft] | Fingerkuppe: B_n ≠ 0 (G20) |
| Kraft beim Loslassen | 0,62 N aus G17 mit 2 mm (Oud C4) | – | [eigene Rechnung] | Risha: **nicht gefunden** |
| Breite der Kontaktstelle | h = 7,5 mm (Synthese) | Woodhouse u. a. 2012, S. 477 | [am Seitenbild geprüft; „to obtain a reasonably realistic sound“ — ein gewählter Wert] | heute für alle |
| | Finger an der Harfe ≈ 2 cm, „negligible“ als Filter | Chadefaux 2012, S. 852, Tab. V | [übertragen von Harfe] | Fingerkuppe |
| | Risha 6–10 mm breit, 100–180 mm lang | oudforguitarists.com | [Stufe 1: nur Zusammenfassung] | nicht zitierfähig |
| Winkel zur Decke (Anfangspolarisation) | 7°–43° | Chadefaux 2012, S. 852 | [übertragen von Harfe, am Seitenbild geprüft] | heute 30° |
| | größte Drehung der Saite 7°–82°, Mittel 39° (Arpeggio) / 45° (Akkord) | Chadefaux 2012, Tab. IV, S. 850–851 | [am Seitenbild geprüft] | – |
| Dauer: Haften | Gitarre (Fingerkuppe) 84–170 ms; Synthese 15 ms; Cembalo-Kiel ≈ 150 ms; Harfe 56–431 ms (Tab. II) | Chadefaux 2012, S. 850 (Gitarre nach Pavlidou, Lit. 16; Synthese nach Derveaux u. a., Lit. 1; Cembalo nach Giordano & Winans, Lit. 15) | [am Seitenbild geprüft; Lit. 1/15/16 nicht gelesen] | – |
| Dauer: Gleiten (wirkt als Tiefpass, G19) | Gitarre ≈ 0,5 ms; Synthese 0,4 ms; Cembalo-Kiel ≈ 8 ms; Harfe 1,2–6,1 ms (Tab. III) | wie oben | [am Seitenbild geprüft] | Risha (Plektrum) näher an Gitarre/Kiel; Laute (Kuppe) näher an Harfe? [eigene Einschätzung] |
| Form des Loslassens | Plektrum: B_n = 0, Dreieck; Finger: Auslenkung und Geschwindigkeit, mit Drehung | Chadefaux 2012, S. 844 (Abstract) / S. 847 | [am Seitenbild geprüft (S. 847)] | – |
| Härte / Werkstoff der Risha | – | – | **nicht gefunden** | – |

### 2.6 Chöre

| Größe | Wert | Quelle | Marke |
|---|---|---|---|
| Verstimmung im Einklang, typisch (Cent) | **nicht gefunden** für Oud und Laute. Woodhouse 2021 rechnet ±8 Cent als Bereich, nicht als typischen Wert (S. 4381). Heute ±1,5 (Oud) / ±2 (Laute) Cent | – | [Annahme] |
| Oktavchöre (Laute Chor 6–8) | Oktavsaite aus Nylon, deutlich dünner | W&LA S. 524–525, Tab. II | [am Seitenbild geprüft (Tab. II)] |
| Kopplungsbreite 2|ε|/2π am Grundton (G16) | Oud: C2 1,17 Hz (≈ 31 Cent; fast nur Im Y, verschiebt die Frequenzen statt die Dämpfung zu trennen); A2 0,63 Hz (≈ 9,9 Cent). Laute Chor 5 (C3): 0,66 Hz (≈ 8,7 Cent) | – | [eigene Rechnung mit Re/Im Y der LumiViz-Körper; C2 und Laute Chor 5 von Hand aus der Tabelle in §C] |
| Zweistufiges Abklingen, Grundton, ±1,5 Cent (Oud) / ±2 Cent (Laute): T60 gleichphasig / gegenphasig | Oud: C2 10,5 / 12,3 s; F2 6,0 / 12,2 s; A2 3,7 / 11,6 s; D3 3,7 / 12,0 s; G3 2,9 / 5,3 s; C4 0,29 / 4,4 s. Laute (Nummern nach W&LA): Chor 5 (C3) 1,9 / 8,9 s; Chor 2 (D4) 1,9 / 3,5 s; Chor 3 (A3) 3,2 / 4,8 s; Chor 4 (F3) 5,0 / 5,7 s. Chor 1 ist einzeln, Chöre 6–8 sind Oktavchöre — dort gilt die Rechnung mit zwei gleichen Saiten nicht | `rechnung_choere.py` §C | [eigene Rechnung] |
| Einzelsaite am selben Punkt, T60 | Oud: C2 11,5 s; A2 5,6 s; C4 0,55 s | ebenda | [eigene Rechnung] |

---

## 3 Zahlenbeispiele aus den Skripten (Auszug)

Die Skripte stehen ganz im Anhang, mit ihrer Ausgabe.

**Oud A2 (Taste 45), Verstimmung gegen Abklingen** (G16, Y aus `oud.lvinstbody`)

| Verstimmung (Cent) | T60 (s) | Frequenzabstand (Hz) |
|---|---|---|
| 0 | 3,66 / 12,18 | 0,594 |
| 3 | 3,71 / 11,58 | 0,621 |
| 8 | 3,97 / 9,57 | 0,771 |
| 12 | 4,21 / 8,41 | 0,954 |

Bis rund 10 Cent ändert die Verstimmung das Abklingen wenig. Die Schwebung liegt bei 0,6 Hz und ist durch den
Imaginärteil von Y bestimmt, nicht durch die Verstimmung.

**Dasselbe im 3-FG-Modell (G15)**

- Eingang: nur die am Stegpunkt stärkste Mode zwischen 80 und 200 Hz (168,6 Hz, η 0,031, wirksame Masse 46 g).
- Ergebnis: T60 10,8 / 12,2 s, also fast kein zweistufiges Abklingen.
- **Der Unterschied zu G16 (3,7 s) zeigt: am Oud-Chor A2 entscheidet nicht die eine nächste Mode, sondern die Summe
  aller Moden.** Dazu gehört die Luftmode bei 100 Hz, deren Re Y bei 110 Hz trägt. Das 3-FG-Modell mit einer Mode ist
  hier zu grob; der Prüfstand mit der vollen Y-Matrix ist richtig.

**Basschor C2: einzeln gegen doppelt**

- Nur der Grundton (G16, §D): +6,0 dB am Anfang, +5,9 dB im 400-ms-Fenster, Pegel 1–2 s −6,6 → −7,2 dB.
- Prüfstand (alle Teiltöne, K-Bewertung, BS.1770), `rechnung_basschor.py`:

| Taste | einzeln (LUFS) | doppelt ±1,5 Cent (LUFS) | Δ | Pegel 1–2 s einzeln / doppelt (dB) |
|---|---|---|---|---|
| 36 C2 | −18,4 | −14,0 | +4,4 | −14,9 / −16,9 |
| 37 | −16,2 | −12,5 | +3,7 | −18,3 / −19,0 |
| 38 | −17,9 | −13,7 | +4,2 | −16,5 / −17,8 |
| 39 | −17,4 | −13,0 | +4,4 | −17,5 / −19,2 |
| 40 | −17,9 | −13,5 | +4,4 | −17,0 / −17,9 |

Die einzelne Saite gibt die Werte aus `2026-11-15_S199_pruefstand_oud_je_taste.txt` genau wieder. Im Prüfstand ist der
Gewinn kleiner als die +6 dB des Grundtons, vermutlich weil die Teiltöne über 1 kHz mit ±1,5 Cent Verstimmung nicht mehr
gleichphasig bleiben [eigene Einschätzung, nicht getrennt geprüft].

---

## 4 Annahmen, die bleiben

**Oud**

| Größe | Grund |
|---|---|
| Deckendicke des A26-Instruments | A26 nennt sie nicht (Stufe 1 hat das nachgeprüft); I18 ist ein anderes Instrument |
| Dickenverlauf | nur qualitativ (I18) |
| Querschnitt der Schale, damit das Volumen | in keiner Quelle; nur die Tiefe 21 cm (A26) |
| Steg: Masse, Höhe, Holz; Länge widersprüchlich (7–17 cm) | Zeichnungen ohne Maßzahl |
| Abstand der Chöre; Abstand der Saiten im Chor (S196: „rund 3 mm“) | nicht gefunden |
| Mündungskorrektur 1,7 und Güte 30 der Luft | gesetzt; nur die gekoppelte Luftmode ist gemessen |
| Rand eingespannt, Schale starr | wie das FE-Modell von A26; die Spitze bei 139 Hz bleibt ohne Gegenstück (S196) |
| Holz Sitka | A26 rechnet alles aus einem Werkstoff; I18: Boróka-Fichte ohne Kennwerte |
| Mensur 0,60 | Spanne 0,585–0,62 belegt, das A26-Instrument nicht |
| Biegesteife und Verluste der umsponnenen Saiten | in keiner gelesenen Quelle; laut Woodhouse u. a. 2012 nur durch Messung zu bestimmen (G9) |
| Zupf: Ort, Auslenkung, Winkel, Dauer, Härte der Risha | nichts Oud-Spezifisches gefunden; übertragen von Gitarre und Harfe |
| Verstimmung im Chor (±1,5 Cent) | kein typischer Wert gefunden |

**Laute**

| Größe | Grund |
|---|---|
| Deckendicke 1,8 mm | nur „often about 1.5mm“ und ein Nachbau |
| Balken: Lage, Höhe, Breite | nur die Ordnung an einer Barocklaute (Jauch) |
| offener Anteil der Rose (85 %) | nicht gefunden |
| Holz der Decke und Dichte des Stegs | FEMTO-Folien nicht gegengeprüft, nicht lokal |
| Zuordnung der 8 Chöre auf die 122,5 mm des 11-chörigen Stegs | die Zeichnung zeigt den heutigen Zustand mit elf Chören |
| Rand eingespannt, Muschel starr | die Spitzen bei 150 und 378 Hz bleiben ohne Gegenstück (S197) |
| Dämpfung α = 60 1/s und Güte 12 | angepasst, nicht gemessen |
| Zupf mit der Fingerkuppe | übertragen von Harfe und Gitarre |
| Saiten gedämpft bei der Messung von W21 | Annahme aus S197 |

---

## 5 Optionen für LumiViz

Zielgrößen nach dem Auftrag:

- Lautheit: Spanne ≤ 15 LU, mittlerer Sprung ≤ 4 LU, größter Sprung ≤ 12 LU (Paare bis 4 Halbtöne);
- Stegadmittanz: Lage und Höhe der Spitzen;
- Abklingen.

Heute (`rechnung_basschor.py`, `rechnung_rest.py`, mit dem Maß „Paare bis 4 Halbtöne“; für die Laute dasselbe Maß wie im
Auftrag, also mittlerer Sprung = Median):

| | Spanne | größter Sprung | mittlerer Sprung |
|---|---|---|---|
| Oud | 15,8 LU | 11,2 LU | 2,6 LU |
| Laute | 13,9 LU | 11,8 LU | 2,4 LU |

**O1 Basschor der Oud: einzeln (heute) oder doppelt** — gerechnet im Prüfstand.

| Zielgröße | einzeln | doppelt ±1,5 Cent |
|---|---|---|
| Spanne | 15,8 LU | **12,1 LU** |
| größter Sprung (Paare ≤ 4 HT) | 11,2 LU | 8,6 LU |
| mittlerer Sprung (Median Paare ≤ 4 HT / Median Terzabstand) | 2,6 / 3,2 LU | 2,2 / 2,4 LU |
| Stegadmittanz (Körper) | unverändert | unverändert |
| Abklingen, Pegel 1–2 s, C2–E2 | −14,9 … −18,3 dB | −16,9 … −19,2 dB (1,3–2,0 dB schneller) |

- Daten: keine neuen nötig.
- **Fachlich:** Alle gelesenen Elf-Saiten-Sätze haben ein einzelnes C [Stufe 1, wörtlich]. Den Chor doppelt zu besaiten
  ändert das Instrument und nicht das Modell. Dass die Spanne mit einzelnem C über 15 LU liegt, kann eine echte
  Eigenschaft der Oud sein. Eine gemessene Lautheit je Taste einer Oud ist nicht gefunden, die Schwelle von 15 LU also an
  der Oud nicht belegt [eigene Einschätzung].

**O2 Verluste der umsponnenen Saiten**

- **(a)** weiter von der Gitarre übertragen (Woodhouse u. a. 2012, Tab. I, Saiten 4–6). Das sind gemessene Werte an
  Saiten derselben Bauart: Multifilamentkern mit Metall umsponnen.
- **(b)** ein Aufbau-Modell: Stokes am Außendurchmesser (G10b) und EI aus dem Kern (G10). Das ist eine **untere Grenze**:
  Es kennt keine Reibung zwischen den Windungen, und die macht an der Gitarre das 5,9-Fache der Luftreibung aus (G10b).

Gerechnet:

- EI aus dem Kern (b): Lautheit ±0,0–0,1 LU, Pegel 1–2 s ±0,1 dB (Prüfstand, Tasten 36–40).
- Verlust am Grundton (b) gegen (a): C2 6·10⁻⁴ gegen 2,7·10⁻³, F2 7·10⁻⁴ gegen 2,1·10⁻³ [eigene Rechnung]. Bei C2 dominiert
  der Saitenverlust (η_b = 2,3·10⁻⁴), deshalb ginge T60 des C2-Grundtons von 11,5 s mit (a) auf rund 40 s mit (b)
  [eigene Rechnung: 6,91/(π·65,4·(6,2+2,3)·10⁻⁴) = 40 s].
- Spanne und Sprünge: nicht gerechnet. Erwartet wird wenig, weil die Lautheit im 400-ms-Fenster vom Anfang bestimmt ist
  [eigene Einschätzung]. Stegadmittanz: unverändert.

Ein **belegtes** Modell fehlt. Gebraucht wird die Messung von T60 je Teilton (n = 1 … 10) an einer einzelnen umsponnenen
Oud-Saite auf starrem Gestell, dazu f_n, damit EI aus G3 folgt. Das ist dasselbe Verfahren wie in Woodhouse u. a. 2012 und
W&LA 2019, Abschn. 3.

**O3 Bauplan der Oud: ein Instrument gegen gemischt**

- **(a)** heute: A26-Ersatz. Er trifft vier der sechs gemessenen Spitzen (101 / 203 / 280 / 362 Hz) auf 3 %; Höhe nicht
  getroffen (S196).
- **(b)** Dicke 2,0 mm und Balkenquerschnitte aus I18, sonst A26. Nach S196 §7 (mit Luftraum, Balken 1,4 cm vor dem Rand)
  verschiebt allein die Dicke 1,5 → 2,0 mm die gekoppelten Moden von 100 / 168 / 208 / 268–281 Hz auf
  110 / 201 / 209 / 323–345 Hz. Die Spitzen 280 und 362 Hz wären damit nicht mehr getroffen. Das ist gemischt aus zwei Instrumenten.
- **(c)** **I18 als zweite Prüfung, nicht als Bauplan:** die freie Decke von I18 (Umriss 490 × 360, Löcher, Balken
  Tab. 1, 2,0 mm) in `InstrumentBody` rechnen, mit freiem oder fast freiem Rand (`rim.sections` mit Federn 0). Dann gegen
  die acht gemessenen Moden 31 / 51 / 69 / 115 / 123 / 198 / 292 / 347 Hz vergleichen, mit der Zuordnung (m, n) aus Abb. 7
  [Stufe 1: wörtlich S. 924]. Das prüft Holzwerte (A26 Tab. 1) und Dicke **an einem Instrument mit gemessener Dicke**.
  - Daten: die Lage der Balken bei I18 (nicht als Zahl; aus Abb. 2 nur grob).
  - Wirkung auf die Zielgrößen: nicht nachgerechnet, das braucht `InstrumentBodyTool`. Ich hatte hier kein Werkzeug zum
    Bauen oder Ausführen.
  - Bei einer Abweichung über rund 10 % in den Moden, deren Form gemessen ist, ist der A26-Holzsatz für eine Oud-Decke
    nicht bestätigt [eigene Einschätzung].

**O4 Steg der Laute: Maße der Rauwolf-Zeichnung gegen die heutige Annahme**

- Daten: liegen vor (§2.2, Bild).
- Wirkung, abgeschätzt aus der Reihe von S197 §5 („Steg 6 / 10 / 15 g → Mode unter dem Steg 275 / 262 / 247 Hz, die bei
  476 Hz 516 / 492 / 463 Hz“) [eigene Rechnung, Extrapolation auf 4,5 g, nicht gerechnet]: Mode unter dem Steg ≈ 280 Hz
  (gemessen 277; heute 264 Hz, −4,6 %), die starke Spitze ≈ 525 Hz (gemessen 476; heute 493, +3,6 % → etwa +10 %).
- Eine Zielgröße würde also besser, eine schlechter. Die Höhen sind nicht abgeschätzt.
- Spanne und Sprünge der Lautheit: nicht gerechnet. Die Laute erfüllt die Schwellen heute (13,9 / 11,8 / 2,4 LU), knapp
  beim größten Sprung.
- Fachlich: Die Maße sind gemessen, die 7 g waren gesetzt. Rechnet man mit den gemessenen Maßen und verfehlt danach eine
  Spitze, fehlt an anderer Stelle etwas im Modell. S197 §7 nennt dafür schon die zu kleine Steife am Steg und die starre
  Muschel [eigene Einschätzung].

---

## 6 Was die LumiViz-Sitzung prüfen muss

1. **Oud C4 auf dem Modenhaufen 269–280 Hz.**
   - Befund: Re Y = 0,18 m/(s N) am Stegpunkt Taste 60 bei 261,6 Hz gibt η_b = 1,3·10⁻² (T60 der Einzelsaite 0,55 s). Der
     Prüfstand zeigt dort die lautesten Tasten (−2,6 LUFS) und einen Pegel 1–2 s von −46 dB.
   - Prüfen: die Höhe der gemessenen Kurve A26 Abb. 6 bei 262 Hz gegen die gerechnete Stegadmittanz an diesem Punkt.
     Zwischen 242 (Tal) und 280 Hz (Spitze) ist die Messung steil.
2. **Inanli-Decke frei rechnen** (O3 c): acht Moden gegen I18 S. 924. Prüfgröße: Frequenz und Form (m, n) je Mode.
3. **Woodhouse 2021, Abb. 5 gegen G11:**
   - Gerechnet: Mit Y = 10^(−17,4/20) bei 277 Hz (S197), Z₀ = 0,083 (Chor 2) und doppelter Saite gibt G11 η_b ≈ 1,4·10⁻².
     Gegen η_air ≈ 2·10⁻³ sind das rund 17 dB.
   - Abgelesen: Abb. 5 zeigt dort rund 40 dB [eigene Ablesung].
   - **Rund 20 dB nicht aufgelöst.** Vor einer Abstimmung des Abklingens der Laute gegen Abb. 5 klären. Für die Gitarre in
     Abb. 2 a passt dieselbe Rechnung auf etwa 4 dB.
4. **Cent-Achse in Woodhouse 2021, Abb. 8/9:** Das Modell G15 trifft die Q-Werte, aber nicht die Breite und die Steigung
   über der Cent-Achse (Faktor 2–3). Nicht als Eichung für die Verstimmung im Kern nehmen, bevor das geklärt ist (§1, G15).
5. **Steg der Laute:** S197 nahm 12 mm Breite; die Zeichnung gibt 10 → 7 mm ohne Ohr (§2.2). Prüfgröße: die vier Spitzen
   nach KQ11 a.
6. **Prüfstand, Oktavchöre der Laute:** Die Oktavsaite koppelt am zweiten Teilton der Basssaite. G16 rechnet nur den
   Grundton. Prüfen, ob der Prüfstand für Chor 6–8 die Oktavsaite an denselben Stegpunkt hängt (`choere`: ja, `[punkt] *
   len(saiten)`) und ob eine Verstimmung der Oktave vorgesehen ist (heute nur für den Einklang).
7. **Umsponnene Saiten:** Die Werte unter O2 (b) sind eine untere Grenze. Im Prüfstand nur als Spanne zeigen, nicht als
   Ersatz für (a).

---

## 7 Nicht gefunden / nicht herleitbar

| Was | wo gesucht |
|---|---|
| Deckenmasse einer Oud oder Laute (für die Gegenrechnung) | A26 (`pdf_suche`: „mass“: nur Beschleunigungsaufnehmer und „low effective mass“), I18 (Seitenbild S. 924), Stufe-1-Quellen |
| Verlustfaktor von Fichte, orthotrop | Bader u. a. 2025, Waltham u. a. 2008, Ducceschi u. a. 2026 (`pdf_suche`: „loss factor“, „spruce“, „Young“): keine Zahl |
| Steg der Oud: Masse, Höhe | A26 („bridge“: nur allgemein), I18 Abb. 2/4 |
| Abstand der Chöre | keine lokale Quelle; Stufe 1: nicht gefunden |
| Verstimmung im Chor, typisch, in Cent | Woodhouse 2021 („cent“: drei Treffer, alle zur Fallstudie), W&LA 2019 |
| Verluste und Biegesteife umsponnener Saiten mit Nylon- oder Seidenkern | W&LA 2019 S. 528 („wound“, „overwound“, „silk“: keine Zahl), Woodhouse u. a. 2012 S. 477 (wörtlich: muss gemessen werden) |
| Risha: Kraft, Winkel, Dauer, Härte | A26 („risha“, „plectrum“: 0 Treffer); Stufe 1 |
| Längssteife umsponnener Saiten | nur Scherrer 2013 Tab. 2.2 (√(ES/T) = 5,91–6,30 für Gitarrensaiten IV–VI nach Chaigne 1991) [am Seitenbild geprüft] — für das lineare Modell nicht nötig |
| Weinreich 1977 im Original | nicht lokal; nur über Woodhouse 2021 |

---

## 8 Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Inanli_Altinsoy_2018_VibroacousticsOud_DAGA.pdf` | https://pub.dega-akustik.de/DAGA_2018/data/articles/000581.pdf | 678 476 B [Stufe 1] | DAGA, frei; Lizenz nicht geprüft | O3 c: freie Decke mit acht Moden; lokal ablegen, damit die Prüfung belegbar wird |
| `FEMTO_2019_LauxMalerLute_Folien.pdf` | https://publiweb.femto-st.fr/tntnet/entries/18458/documents/author/data | 3 201 294 B [Stufe 1] | ohne Lizenzangabe | Holzwerte je Teil einer Renaissancelaute (Folie 15) |
| `Weinreich_1977_CoupledPianoStrings_JASA62.pdf` | doi:10.1121/1.381677 [eigene Angabe aus dem Gedächtnis, nicht geprüft] | ? | ASA, hinter der Bezahlschranke | G16 am Original prüfen |
| `Woodhouse_2017_DampingNonlinearityPluckedStrings_ActaAcust103.pdf` | Lit. in Woodhouse 2021 („Influence of damping and nonlinearity in plucked strings“, Acta Acust.) | ? | Verlag | Laute, Dämpfung und Zupf (Woodhouse 2021 S. 4376 verweist für die Laute darauf) |
| `Pavlidou_1997_StringFingerInteraction_PhD.pdf` | – (Lit. 16 bei Chadefaux 2012) | ? | Dissertation, Cardiff | Zupf mit Finger und Nagel, Dauer des Gleitens |
| (anfragen) Messung einer umsponnenen Oud-Saite | – | – | – | O2 |

---

## Was nicht geliefert ist, und was die LumiViz-Sitzung selbst nachprüfen muss

- **Nicht geliefert:**
  - ein Bauplan, bei dem alle Größen am selben Instrument belegt sind (gibt es in der Ablage nicht);
  - ein belegtes Verlustmodell für umsponnene Saiten;
  - Oud-spezifische Zupfwerte;
  - typische Verstimmungen im Chor;
  - die Wirkung von O2 auf die Spanne der Lautheit sowie von O3 und O4 auf alle Zielgrößen: Dafür muss der Körper neu
    gerechnet werden (`InstrumentBodyTool`), und das war hier nicht möglich.
- **Selbst prüfen:**
  - §6, Punkte 1–7;
  - meine Ablesungen an den Bildern: I18 Abb. 4, Rauwolf-Steg und Woodhouse 2021 Abb. 5, 8, 9;
  - die Zuordnung der Bassseite des Rauwolf-Stegs.

---

## Anhang: Skripte und Ausgaben

Die Skripte liegen unter `ergebnisse/2026-11-16_15_oud_laute_grundlagen/`. Alle lesen nur. `rechnung_choere.py` und
`rechnung_basschor.py` importieren den Prüfstand S199 ohne Bytecode.

### A1 `rechnung_saiten.py`

```python
"""Saitensaetze Oud und Laute als Rechenwerte (Auftrag 2026-11-16_15). Schreibt nichts.

Gleichungen: Woodhouse & Lynch-Aird 2019 (W&LA) Gl. 3, 4, 8, 11-17 (S. 517-520, am Seitenbild geprueft);
Valette nach Woodhouse u. a. 2012 Gl. 4 (S. 477). Umsponnene Saiten: Aufbau-Modell [hergeleitet].
"""
import math

RHO_A, NU_A = 1.2, 1.5e-5
MAT = {"N": (4.5, 39.0, 0.25, 1070.0), "F": (3.2, 41.0, 0.18, 1800.0)}  # W&LA Tab. I: E' = a + b*sigma (GPa), E'' GPa, rho


def hz(taste):
    return 440.0 * 2 ** ((taste - 69) / 12)


def monofil(d_mm, f, L, mat):
    a, b, e2, rho = MAT[mat]
    d = d_mm / 1000
    A = math.pi * d * d / 4
    mu = rho * A
    T = 4 * L * L * f * f * mu                      # Gl. 3
    sigma = T / A / 1e9                              # GPa
    E = (a + b * sigma) * 1e9                         # Gl. 16 / 17
    EI = E * math.pi * d ** 4 / 64
    etaE = e2 * 1e9 / E
    out = {"d": d_mm, "f": f, "mu": mu, "T": T, "sigma_GPa": sigma, "E_GPa": E / 1e9, "EI": EI, "etaE": etaE,
           "Z0": math.sqrt(T * mu)}
    for n in (1, 10):
        fn = n * f
        lam = E * math.pi ** 2 * d * d * n * n / (64 * rho * L ** 4 * f * f + E * math.pi ** 2 * d * d * n * n)  # Gl. 4
        M = d / 4 * math.sqrt(2 * math.pi * fn / NU_A)                                                        # Gl. 12
        eta_air = RHO_A / rho * (2 * math.sqrt(2) * M + 1) / M ** 2                                            # Gl. 11
        eta_air_k = eta_air * (d * 1000 + 0.2)                                                                 # Zuschlag S. 518
        out[n] = {"lam": lam, "eta_air": eta_air_k, "eta_bend": lam * etaE, "eta": eta_air_k + lam * etaE}       # Gl. 14, 15
    return out


print("=== Gegenprobe Gl. 3 gegen W&LA S. 524: 'the 2nd, 3rd and 4th courses ... same tension, around 30 N'")
LAUTE = [(1, 392.0, [(0.475, "N", 1)]), (2, 293.7, [(0.52, "N", 1), (0.52, "N", 1)]), (3, 220.0, [(0.725, "N", 1), (0.725, "N", 1)]),
         (4, 174.6, [(0.900, "N", 1), (0.900, "N", 1)]), (5, 130.8, [(0.91, "F", 1), (0.91, "F", 1)]),
         (6, 98.0, [(1.25, "F", 1), (0.775, "N", 2)]), (7, 87.3, [(1.40, "F", 1), (0.875, "N", 2)]), (8, 73.4, [(1.60, "F", 1), (0.925, "N", 2)])]
L_LAUTE = 0.625
print("Laute (W&LA Tab. II, L = 0,625 m)")
print("Chor Saite  f Hz   d mm  W  mu g/m   T N   sigma MPa  E GPa  EI 1e-6 Nm2  lam1     eta_air1  eta_bend1  eta1    Z0 Ns/m  lam10   eta10")
for c, f, saiten in LAUTE:
    for k, (d, m, okt) in enumerate(saiten):
        s = monofil(d, f * okt, L_LAUTE, m)
        print(f"{c:4d} {k + 1:4d} {s['f']:6.1f} {d:6.3f} {m}  {s['mu'] * 1e3:6.3f} {s['T']:6.1f} {s['sigma_GPa'] * 1e3:8.1f} {s['E_GPa']:6.2f} "
              f"{s['EI'] * 1e6:10.2f}  {s[1]['lam']:.2e} {s[1]['eta_air']:.2e} {s[1]['eta_bend']:.2e} {s[1]['eta']:.2e} {s['Z0']:6.3f}  "
              f"{s[10]['lam']:.3f}  {s[10]['eta']:.2e}")

print()
print("=== Oud, Diskant (D'Addario EJ95A: G3 .031 in, C4 .026 in Nylon), drei Mensuren")
for L in (0.585, 0.60, 0.615):
    for t, d in ((55, 0.031 * 25.4), (60, 0.026 * 25.4)):
        s = monofil(d, hz(t), L, "N")
        print(f"L {L:.3f}  Taste {t}  f {s['f']:6.1f}  d {d:.3f} mm  mu {s['mu'] * 1e3:.3f} g/m  T {s['T']:5.1f} N ({s['T'] / 9.80665:.2f} kgf)  "
              f"E {s['E_GPa']:.2f} GPa  EI {s['EI'] * 1e6:.1f}e-6  lam1 {s[1]['lam']:.1e}  eta1 {s[1]['eta']:.2e}  lam10 {s[10]['lam']:.3f}  eta10 {s[10]['eta']:.2e}  Z0 {s['Z0']:.3f}")

print()
print("=== Oud, umsponnen: mu aus Zug (EJ95A, kgf) und mu aus dem Aufbau")
# EJ95A: Chor, Taste, Aussendurchmesser in (Herstellerangabe, S195 O2), Zug kgf
UMSP = [("D3", 50, 0.024, 4.034), ("A2", 45, 0.028, 3.24), ("F2", 41, 0.032, 2.86), ("C2", 36, 0.043, 3.659)]
RHO_KERN, RHO_CU = 1140.0, 8960.0     # Annahmen: Nylon-Faserkern (Polyamid 6.6), Kupfer (Silberauflage vernachlaessigt)
FUELL = 0.80                           # Annahme: Fuellgrad des Faserkerns (Fasern mit Luft dazwischen)
for name, t, d_in, kg in UMSP:
    D = d_in * 25.4e-3
    f = hz(t)
    for L in (0.585, 0.60, 0.615):
        T = kg * 9.80665
        mu = T / (4 * L * L * f * f)                 # Gl. 3, nach mu umgestellt
        rho_eff = mu / (math.pi * D * D / 4)
        # Aufbau: ein Lagen-Rundlitze (Draht w) dicht an dicht um den Kern d_c, D = d_c + 2 w.
        # Masse je Laenge: Kern rho_k*FUELL*pi d_c^2/4 + Draht rho_cu * (pi w^2/4) * pi (d_c + w) / w   [hergeleitet]
        best = None
        for i in range(1, 2000):
            w = D / 2 * i / 2000
            dc = D - 2 * w
            if dc <= 0:
                break
            m = RHO_KERN * FUELL * math.pi * dc * dc / 4 + RHO_CU * math.pi * w * w / 4 * math.pi * (dc + w) / w
            if best is None or abs(m - mu) < abs(best[2] - mu):
                best = (w, dc, m)
        w, dc, m = best
        sigma_k = T / (math.pi * dc * dc / 4 * FUELL) / 1e9
        E_k = (4.5 + 39 * sigma_k) * 1e9            # Gl. 16 fuer den Kern - UEBERTRAGUNG auf Faserkern, obere Grenze
        EI_massiv = E_k * math.pi * dc ** 4 / 64
        lam1 = EI_massiv * (math.pi / L) ** 2 / (T + EI_massiv * (math.pi / L) ** 2)
        lam1_g = 50e-6 * (math.pi / L) ** 2 / (T + 50e-6 * (math.pi / L) ** 2)
        if L == 0.60:
            print(f"{name} f {f:6.1f}  D {D * 1e3:.3f} mm  T {T:5.1f} N  mu {mu * 1e3:.3f} g/m  rho_eff {rho_eff:5.0f} kg/m3  "
                  f"Aufbau: Draht w {w * 1e3:.3f} mm, Kern {dc * 1e3:.3f} mm; Kern-Spannung {sigma_k * 1e3:.0f} MPa; "
                  f"EI Kern massiv {EI_massiv * 1e6:.1f}e-6 (lam1 {lam1:.1e}) gegen uebertragen 50e-6 (lam1 {lam1_g:.1e}); Z0 {math.sqrt(T * mu):.3f}")
        else:
            print(f"   L {L:.3f}: mu {mu * 1e3:.3f} g/m, rho_eff {rho_eff:5.0f}")

print()
print("=== Valette (Woodhouse u. a. 2012 Gl. 4) mit den uebertragenen Werten, Grundton und 10. Teilton, L = 0,60")
for name, t, d_in, kg in UMSP:
    T = kg * 9.80665
    f = hz(t)
    for n in (1, 10):
        w_ = 2 * math.pi * n * f
        kk = n * math.pi / 0.60
        eta = (T * (5e-5 + 1.1 / w_) + 50e-6 * 2.0e-2 * kk * kk) / (T + 50e-6 * kk * kk)
        print(f"{name} n={n:2d} f={n * f:7.1f}  eta {eta:.2e}  (Q {1 / eta:5.0f}; Abklingzeit T60 {6.91 / (eta * math.pi * n * f):5.2f} s ohne Koerper)")
```

Ausgabe:

```text
=== Gegenprobe Gl. 3 gegen W&LA S. 524: 'the 2nd, 3rd and 4th courses ... same tension, around 30 N'
Laute (W&LA Tab. II, L = 0,625 m)
Chor Saite  f Hz   d mm  W  mu g/m   T N   sigma MPa  E GPa  EI 1e-6 Nm2  lam1     eta_air1  eta_bend1  eta1    Z0 Ns/m  lam10   eta10
   1    1  392.0  0.475 N   0.190   45.5    256.9  14.52      36.28  2.01e-05 1.73e-03 3.47e-07 1.73e-03  0.093  0.002  5.12e-04
   2    1  293.7  0.520 N   0.227   30.6    144.2  10.12      36.34  3.00e-05 1.97e-03 7.40e-07 1.97e-03  0.083  0.003  6.14e-04
   2    2  293.7  0.520 N   0.227   30.6    144.2  10.12      36.34  3.00e-05 1.97e-03 7.40e-07 1.97e-03  0.083  0.003  6.14e-04
   3    1  220.0  0.725 N   0.442   33.4     80.9   7.66     103.83  7.85e-05 2.03e-03 2.56e-06 2.03e-03  0.121  0.008  8.22e-04
   3    2  220.0  0.725 N   0.442   33.4     80.9   7.66     103.83  7.85e-05 2.03e-03 2.56e-06 2.03e-03  0.121  0.008  8.22e-04
   4    1  174.6  0.900 N   0.681   32.4     51.0   6.49     208.95  1.63e-04 2.15e-03 6.27e-06 2.15e-03  0.149  0.016  1.22e-03
   4    2  174.6  0.900 N   0.681   32.4     51.0   6.49     208.95  1.63e-04 2.15e-03 6.27e-06 2.15e-03  0.149  0.016  1.22e-03
   5    1  130.8  0.910 F   1.171   31.3     48.1   5.17     174.13  1.41e-04 1.50e-03 4.89e-06 1.51e-03  0.191  0.014  9.02e-04
   5    2  130.8  0.910 F   1.171   31.3     48.1   5.17     174.13  1.41e-04 1.50e-03 4.89e-06 1.51e-03  0.191  0.014  9.02e-04
   6    1   98.0  1.250 F   2.209   33.1     27.0   4.31     516.22  3.93e-04 1.61e-03 1.64e-05 1.62e-03  0.271  0.038  2.04e-03
   6    2  196.0  0.775 N   0.505   30.3     64.2   7.00     124.04  1.03e-04 2.12e-03 3.69e-06 2.12e-03  0.124  0.010  9.58e-04
   7    1   87.3  1.400 F   2.771   33.0     21.4   4.08     769.16  5.89e-04 1.66e-03 2.60e-05 1.69e-03  0.302  0.056  2.93e-03
   7    2  174.6  0.875 N   0.643   30.6     51.0   6.49     186.68  1.54e-04 2.17e-03 5.93e-06 2.17e-03  0.140  0.015  1.19e-03
   8    1   73.4  1.600 F   3.619   30.5     15.2   3.82    1229.29  1.02e-03 1.77e-03 4.80e-05 1.82e-03  0.332  0.093  4.87e-03
   8    2  146.8  0.925 N   0.719   24.2     36.0   5.91     212.21  2.21e-04 2.35e-03 9.37e-06 2.36e-03  0.132  0.022  1.58e-03

=== Oud, Diskant (D'Addario EJ95A: G3 .031 in, C4 .026 in Nylon), drei Mensuren
L 0.585  Taste 55  f  196.0  d 0.787 mm  mu 0.521 g/m  T  27.4 N (2.79 kgf)  E 6.69 GPa  EI 126.3e-6  lam1 1.3e-04  eta1 2.11e-03  lam10 0.013  eta10 1.08e-03  Z0 0.119
L 0.585  Taste 60  f  261.6  d 0.660 mm  mu 0.367 g/m  T  34.3 N (3.50 kgf)  E 8.41 GPa  EI 78.5e-6  lam1 6.6e-05  eta1 1.90e-03  lam10 0.007  eta10 7.26e-04  Z0 0.112
L 0.600  Taste 55  f  196.0  d 0.787 mm  mu 0.521 g/m  T  28.8 N (2.94 kgf)  E 6.81 GPa  EI 128.5e-6  lam1 1.2e-04  eta1 2.11e-03  lam10 0.012  eta10 1.03e-03  Z0 0.123
L 0.600  Taste 60  f  261.6  d 0.660 mm  mu 0.367 g/m  T  36.1 N (3.68 kgf)  E 8.61 GPa  EI 80.4e-6  lam1 6.1e-05  eta1 1.90e-03  lam10 0.006  eta10 7.08e-04  Z0 0.115
L 0.615  Taste 55  f  196.0  d 0.787 mm  mu 0.521 g/m  T  30.3 N (3.09 kgf)  E 6.93 GPa  EI 130.7e-6  lam1 1.1e-04  eta1 2.11e-03  lam10 0.011  eta10 9.92e-04  Z0 0.126
L 0.615  Taste 60  f  261.6  d 0.660 mm  mu 0.367 g/m  T  38.0 N (3.87 kgf)  E 8.82 GPa  EI 82.4e-6  lam1 5.7e-05  eta1 1.90e-03  lam10 0.006  eta10 6.91e-04  Z0 0.118

=== Oud, umsponnen: mu aus Zug (EJ95A, kgf) und mu aus dem Aufbau
   L 0.585: mu 1.340 g/m, rho_eff  4593
D3 f  146.8  D 0.610 mm  T  39.6 N  mu 1.274 g/m  rho_eff  4366 kg/m3  Aufbau: Draht w 0.103 mm, Kern 0.403 mm; Kern-Spannung 388 MPa; EI Kern massiv 25.3e-6 (lam1 1.8e-05) gegen uebertragen 50e-6 (lam1 3.5e-05); Z0 0.225
   L 0.615: mu 1.213 g/m, rho_eff  4155
   L 0.585: mu 1.918 g/m, rho_eff  4829
A2 f  110.0  D 0.711 mm  T  31.8 N  mu 1.824 g/m  rho_eff  4590 kg/m3  Aufbau: Draht w 0.131 mm, Kern 0.449 mm; Kern-Spannung 250 MPa; EI Kern massiv 28.6e-6 (lam1 2.5e-05) gegen uebertragen 50e-6 (lam1 4.3e-05); Z0 0.241
   L 0.615: mu 1.736 g/m, rho_eff  4369
   L 0.585: mu 2.688 g/m, rho_eff  5180
F2 f   87.3  D 0.813 mm  T  28.0 N  mu 2.555 g/m  rho_eff  4925 kg/m3  Aufbau: Draht w 0.168 mm, Kern 0.478 mm; Kern-Spannung 196 MPa; EI Kern massiv 31.0e-6 (lam1 3.0e-05) gegen uebertragen 50e-6 (lam1 4.9e-05); Z0 0.268
   L 0.615: mu 2.432 g/m, rho_eff  4687
   L 0.585: mu 6.127 g/m, rho_eff  6540
C2 f   65.4  D 1.092 mm  T  35.9 N  mu 5.825 g/m  rho_eff  6217 kg/m3  Aufbau: Draht w 0.346 mm, Kern 0.400 mm; Kern-Spannung 357 MPa; EI Kern massiv 23.1e-6 (lam1 1.8e-05) gegen uebertragen 50e-6 (lam1 3.8e-05); Z0 0.457
   L 0.615: mu 5.544 g/m, rho_eff  5917

=== Valette (Woodhouse u. a. 2012 Gl. 4) mit den uebertragenen Werten, Grundton und 10. Teilton, L = 0,60
D3 n= 1 f=  146.8  eta 1.24e-03  (Q   805; Abklingzeit T60 12.05 s ohne Koerper)
D3 n=10 f= 1468.3  eta 2.38e-04  (Q  4207; Abklingzeit T60  6.30 s ohne Koerper)
A2 n= 1 f=  110.0  eta 1.64e-03  (Q   609; Abklingzeit T60 12.18 s ohne Koerper)
A2 n=10 f= 1100.0  eta 2.94e-04  (Q  3399; Abklingzeit T60  6.80 s ohne Koerper)
F2 n= 1 f=   87.3  eta 2.06e-03  (Q   486; Abklingzeit T60 12.25 s ohne Koerper)
F2 n=10 f=  873.1  eta 3.47e-04  (Q  2885; Abklingzeit T60  7.27 s ohne Koerper)
C2 n= 1 f=   65.4  eta 2.73e-03  (Q   367; Abklingzeit T60 12.33 s ohne Koerper)
C2 n=10 f=  654.1  eta 3.93e-04  (Q  2547; Abklingzeit T60  8.57 s ohne Koerper)
```

### A2 `rechnung_choere.py`

```python
"""Choere: zwei Saiten an einem Stegpunkt (Auftrag 2026-11-16_15). Schreibt nichts.

A  Woodhouse 2021 Gl. 6-10 (drei Freiheitsgrade: Koerpermode + zwei Saiten), gegen Abb. 8 und 9 derselben Arbeit.
B  Zwei Saiten an einer komplexen Stegadmittanz Y (hergeleitet, Weinreich-Form): Eigenwerte, Abklingraten, Schwebung.
C  Oud-Choere an der Stegadmittanz des Koerpers `oud.lvinstbody` (LumiViz, nur gelesen), Laute an `lute.lvinstbody`.
D  Basschor einzeln gegen doppelt (nur Grundton): Pegel am Anfang, Lautheit im 400-ms-Fenster, Pegel 1-2 s.
Aufruf: python -B -I <dieses Skript>
"""
import importlib.util
import math
import sys

import numpy as np

sys.dont_write_bytecode = True
PFAD = (r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung"
        r"\gezupfte-saite\proben\2026-11-15_S199_pruefstand_zupf.py")
spec = importlib.util.spec_from_file_location("pruefstand", PFAD)
P = importlib.util.module_from_spec(spec)
spec.loader.exec_module(P)


# ------------------------------------------------------------------ A: Woodhouse 2021 Gl. 7-10
def drei_fg(L, T1, T2, m1, m2, Q1, Q2, mb, kb, cb):
    K = np.diag([kb, math.pi ** 2 * T1 / (2 * L), math.pi ** 2 * T2 / (2 * L)])             # Gl. 7
    M = np.array([[mb + L * (m1 + m2) / 3, m1 * L / math.pi, m2 * L / math.pi],               # Gl. 8
                  [m1 * L / math.pi, m1 * L / 2, 0.0],
                  [m2 * L / math.pi, 0.0, m2 * L / 2]])
    c1 = math.pi * math.sqrt(T1 * m1) / (2 * Q1)                                              # Gl. 10
    c2 = math.pi * math.sqrt(T2 * m2) / (2 * Q2)
    C = np.diag([cb, c1, c2])                                                                 # Gl. 9
    Mi = np.linalg.inv(M)
    A = np.block([[np.zeros((3, 3)), np.eye(3)], [-Mi @ K, -Mi @ C]])
    lam, vec = np.linalg.eig(A)
    moden = []
    for l, v in zip(lam, vec.T):
        if l.imag <= 0:
            continue
        q = v[:3]
        f = l.imag / (2 * math.pi)
        Q = l.imag / (2 * -l.real)
        art = "Koerper" if abs(q[0]) * 3 > max(abs(q[1]), abs(q[2])) and abs(q[1]) < 1e-12 + abs(q[0]) * 30 and False else ""
        moden.append((f, Q, q))
    moden.sort(key=lambda x: x[0])
    return moden


def saiten_moden(moden):
    """Die zwei Moden mit dem kleinsten Koerperanteil; Rueckgabe (f, Q, gleichphasig?)."""
    bewertet = sorted(moden, key=lambda m: abs(m[2][0]) / (abs(m[2][1]) + abs(m[2][2])))[:2]
    aus = []
    for f, Q, q in sorted(bewertet, key=lambda m: m[0]):
        gleich = (q[1] * np.conj(q[2])).real > 0
        aus.append((f, Q, gleich))
    return aus


print("=== A  Woodhouse 2021 Gl. 7-10, Fallstudie S. 4381: Stahl 0,45 mm, L 0,65 m, B3 246,9 Hz, Q der Saite 3000")
L, d = 0.65, 0.45e-3
m = 7850 * math.pi * d * d / 4
T0 = 4 * L * L * 246.9 ** 2 * m
print(f"Saite: m {m * 1e3:.3f} g/m, T {T0:.1f} N, Z0 {math.sqrt(T0 * m):.3f} N s/m  [Stahldichte 7850 = Annahme]")
fall = {"Abb. 9 (Daempfer c = 300 N s/m, m = k = 1e-3)": (1e-3, 1e-3, 300.0)}
for fb in (222.0, 247.0):   # Lage der Koerperspitze aus Abb. 7 abgelesen (durchgezogen ~222, gestrichelt ~247 Hz)
    mb = 0.2
    wb = 2 * math.pi * fb
    fall[f"Abb. 8 (Koerper 0,2 kg, Q 40, {fb:.0f} Hz)"] = (mb, mb * wb * wb, mb * wb / 40)
for name, (mb, kb, cb) in fall.items():
    print(name)
    for cent in (-8, -4, -2, -1, -0.5, 0, 0.5, 1, 2, 4, 8):
        T2 = T0 * 2 ** (2 * cent / 1200)
        sm = saiten_moden(drei_fg(L, T0, T2, m, m, 3000, 3000, mb, kb, cb))
        print(f"   {cent:+5.1f} cent: " + "   ".join(f"{f:7.2f} Hz Q {Q:6.0f} ({'gleich' if g else 'gegen'})" for f, Q, g in sm))


# ------------------------------------------------------------------ B: zwei Saiten an komplexem Y
def zwei_saiten(f1, f2, Z1, Z2, Y, eta1, eta2):
    """Komplexe Kreisfrequenzen (Zeitansatz e^{i w t}): w = w_n (1 + i Z0 Y / pi) je Saite (Grundton, n = 1);
    zwei Saiten am selben Punkt: Kopplung ueber dieselbe Stegbewegung. Eigenwerte der 2x2-Matrix."""
    w1, w2 = 2 * math.pi * f1, 2 * math.pi * f2
    s1 = w1 * (1 + 0.5j * eta1)
    s2 = w2 * (1 + 0.5j * eta2)
    e11, e22 = 1j * w1 * Z1 * Y / math.pi, 1j * w2 * Z2 * Y / math.pi
    e12 = 1j * math.sqrt(w1 * w2 * Z1 * Z2) * Y / math.pi
    A = np.array([[s1 + e11, e12], [e12, s2 + e22]])
    lam, vec = np.linalg.eig(A)
    return lam, vec


print()
print("=== B  Gegenprobe: dieselbe Fallstudie mit Y = 1/c (Daempfer 300 N s/m) in der 2x2-Form")
Z = math.sqrt(T0 * m)
for cent in (0, 0.5, 1, 2, 8):
    f2 = 246.9 * 2 ** (cent / 1200)
    lam, _ = zwei_saiten(246.9, f2, Z, Z * 2 ** (cent / 1200), 1 / 300, 1 / 3000, 1 / 3000)
    print(f"   {cent:+4.1f} cent: " + "   ".join(f"{l.real / 2 / math.pi:7.2f} Hz Q {l.real / (2 * l.imag):6.0f}" for l in sorted(lam, key=lambda x: x.real)))
eps = 2 * math.pi * 246.9 * Z / 300 / math.pi
print(f"   |eps| = w Z0 |Y| / pi = {eps:.3f} 1/s; Bereich der Anti-Veering-Verschmelzung |f2 - f1| < 2|eps|/2pi = "
      f"{2 * eps / 2 / math.pi:.3f} Hz = {1200 * math.log2(1 + 2 * eps / 2 / math.pi / 246.9):.2f} cent")


# ------------------------------------------------------------------ C: Oud und Laute
def y_am(kp, punkt, f):
    w = np.array([2 * math.pi * f + 0j])
    Y, _ = P.koerper_kurven(kp, punkt, w)
    return complex(Y[0])


def eta_saite(sa, f):
    k2 = (2 * math.pi * f) ** 2 * sa["m"] / sa["T"]
    return float(P.saiten_eta(sa, np.array([2 * math.pi * f + 0j]), np.array([k2]))[0].real)


def chor_tabelle(name, cent_gesamt):
    kp = P.lade_koerper({"oud": "oud", "laute": "lute"}[name])
    ch, mensur, cent = P.choere(name)
    print(f"--- {name}: Mensur {mensur} m, Verstimmung im Einklang {cent_gesamt} cent gesamt")
    print("Chor  Ton   f Hz   Z0 Ns/m  Re Y      Im Y      eta_b einzeln  eta Saite | einzeln: T60 s | doppelt, gleichphasig/gegen: T60 s | Schwebung Hz")
    erg = {}
    for c, (t, saiten) in enumerate(ch):
        sa = saiten[0]
        f = sa["hz"]
        Z0 = math.sqrt(sa["T"] * sa["m"])
        Y = y_am(kp, kp["tasten"].index(t), f)
        eta_s = eta_saite(sa, f)
        eta_b = 2 * Y.real * Z0 / math.pi                    # Woodhouse 2021 Gl. 1 (Y0 = 1/Z0)
        lam1, _ = zwei_saiten(f, f, Z0, Z0, Y, eta_s, eta_s)
        einz = 2 * math.pi * f * (1 + 0.5j * eta_s) * (1 + 1j * Z0 * Y / math.pi)
        T60_1 = 6.91 / einz.imag
        f2 = f * 2 ** (cent_gesamt / 1200)
        lam, vec = zwei_saiten(f, f2, Z0, Z0, Y, eta_s, eta_s)
        lam = sorted(lam, key=lambda x: -x.imag)
        T60 = [6.91 / l.imag for l in lam]
        schweb = abs(lam[0].real - lam[1].real) / 2 / math.pi
        erg[t] = (f, Z0, Y, eta_s, einz, lam, vec)
        print(f"{c + 1:4d}  {t:3d} {f:7.1f}  {Z0:6.3f}  {Y.real:.2e}  {Y.imag:+.2e}  {eta_b:.2e}      {eta_s:.2e} | {T60_1:6.2f}"
              f" | {T60[0]:6.2f} / {T60[1]:6.2f} | {schweb:.3f}")
    return kp, erg


print()
print("=== C  Choere an der Stegadmittanz der LumiViz-Koerper (Grundton, alle Moden der Datei und des statistischen Teils)")
kp_oud, erg_oud = chor_tabelle("oud", 3.0)
kp_lau, erg_lau = chor_tabelle("laute", 4.0)

# Verstimmung variiert am Oud-Chor A2 (Taste 45)
print()
print("Oud A2 (Taste 45): Abklingzeiten T60 (s) der beiden Moden und Schwebung gegen die Verstimmung")
f, Z0, Y, eta_s, _, _, _ = erg_oud[45]
for cent in (0, 0.5, 1, 1.5, 2, 3, 5, 8, 12):
    lam, _ = zwei_saiten(f, f * 2 ** (cent / 1200), Z0, Z0, Y, eta_s, eta_s)
    lam = sorted(lam, key=lambda x: -x.imag)
    print(f"   {cent:5.1f} cent: T60 {6.91 / lam[0].imag:6.2f} / {6.91 / lam[1].imag:6.2f} s, Frequenzabstand {abs(lam[0].real - lam[1].real) / 2 / math.pi:.3f} Hz")
eps = 2 * math.pi * f * Z0 * abs(Y) / math.pi
print(f"   |eps| = {eps:.3f} 1/s -> Kopplungsbreite 2|eps|/2pi = {2 * eps / 2 / math.pi:.3f} Hz = {1200 * math.log2(1 + 2 * eps / 2 / math.pi / f):.2f} cent")

# 3-FG-Modell mit der naechsten Koerpermode am Stegpunkt A2
print()
print("Oud A2 im 3-FG-Modell (Woodhouse 2021 Gl. 7-10) mit der Koerpermode, die am Stegpunkt am staerksten ist (80-200 Hz)")
punkt = kp_oud["tasten"].index(45)
phi = kp_oud["steg"][:, punkt]
sel = [i for i in range(len(kp_oud["f"])) if 80 < kp_oud["f"][i] < 200]
i = max(sel, key=lambda i: phi[i] ** 2)
mb = 1 / phi[i] ** 2
wb = 2 * math.pi * kp_oud["f"][i]
sa = P.choere("oud")[0][2][1][0]
print(f"   Mode {kp_oud['f'][i]:.1f} Hz, eta {kp_oud['eta'][i]:.3f}, wirksame Masse am Stegpunkt {mb * 1e3:.1f} g")
for cent in (0, 1.5, 3, 8):
    T2 = sa["T"] * 2 ** (2 * cent / 1200)
    sm = saiten_moden(drei_fg(0.60, sa["T"], T2, sa["m"], sa["m"], 1 / eta_s, 1 / eta_s, mb, mb * wb * wb, mb * wb * kp_oud["eta"][i]))
    print(f"   {cent:4.1f} cent: " + "   ".join(f"{f_:7.2f} Hz Q {Q:5.0f} T60 {6.91 * Q / (math.pi * f_):5.2f} s ({'gleich' if g else 'gegen'})" for f_, Q, g in sm))

# ------------------------------------------------------------------ D: Basschor einzeln gegen doppelt
print()
print("=== D  Oud C2 (Taste 36): einzeln gegen doppelt, nur der Grundton (Kraft am Steg ~ Summe der Saitenkraefte)")
f, Z0, Y, eta_s, einz, _, _ = erg_oud[36]
t = np.arange(0, 3.0, 1 / 4000)


def huelle(lams, vec, start):
    a = np.linalg.solve(vec, start)
    x = sum(a[k] * vec[:, k][:, None] * np.exp(1j * lams[k] * t)[None, :] for k in range(len(lams)))
    return np.abs(x.sum(axis=0))       # Summe der beiden Saitenkraefte (gleiche Impedanz)


def kennzahlen(h):
    p2 = h ** 2
    n = int(0.4 * 4000)
    c = np.concatenate([[0.0], np.cumsum(p2)])
    fen = (c[n:] - c[:-n]) / n
    an = 10 * np.log10(fen.max())
    spaet = 10 * np.log10(np.mean(p2[(t >= 1) & (t < 2)])) - 10 * np.log10(np.mean(p2[t < 0.4]))
    return an, spaet


h1 = np.abs(np.exp(1j * einz * t))
a1, s1 = kennzahlen(h1)
print(f"   einzeln: Abklingzeit T60 {6.91 / einz.imag:.2f} s; Pegel 1-2 s gegen 0-0,4 s {s1:.1f} dB")
for cent in (0.0, 1.0, 3.0, 6.0):
    lam, vec = zwei_saiten(f, f * 2 ** (cent / 1200), Z0, Z0, Y, eta_s, eta_s)
    h2 = huelle(lam, vec, np.array([1.0, 1.0]))
    a2, s2 = kennzahlen(h2)
    print(f"   doppelt {cent:3.1f} cent: Anfang +{20 * np.log10(h2[0] / h1[0]):.1f} dB, 400-ms-Fenster {a2 - a1:+.1f} dB gegen einzeln, "
          f"Pegel 1-2 s gegen 0-0,4 s {s2:.1f} dB, T60 der Moden {6.91 / lam[0].imag:.2f} / {6.91 / lam[1].imag:.2f} s")
```

Ausgabe:

```text
=== A  Woodhouse 2021 Gl. 7-10, Fallstudie S. 4381: Stahl 0,45 mm, L 0,65 m, B3 246,9 Hz, Q der Saite 3000
Saite: m 1.248 g/m, T 128.6 N, Z0 0.401 N s/m  [Stahldichte 7850 = Annahme]
Abb. 9 (Daempfer c = 300 N s/m, m = k = 1e-3)
    -8.0 cent:  245.77 Hz Q    848 (gegen)    246.89 Hz Q    844 (gleich)
    -4.0 cent:  246.35 Hz Q    848 (gegen)    246.88 Hz Q    843 (gleich)
    -2.0 cent:  246.66 Hz Q    850 (gegen)    246.85 Hz Q    841 (gleich)
    -1.0 cent:  246.83 Hz Q   1787 (gegen)    246.83 Hz Q    553 (gleich)
    -0.5 cent:  246.86 Hz Q   2605 (gegen)    246.87 Hz Q    504 (gleich)
    +0.0 cent:  246.90 Hz Q   3000 (gegen)    246.90 Hz Q    492 (gleich)
    +0.5 cent:  246.94 Hz Q   2605 (gegen)    246.94 Hz Q    504 (gleich)
    +1.0 cent:  246.97 Hz Q   1787 (gegen)    246.97 Hz Q    553 (gleich)
    +2.0 cent:  246.95 Hz Q    849 (gegen)    247.14 Hz Q    840 (gleich)
    +4.0 cent:  246.92 Hz Q    846 (gegen)    247.45 Hz Q    842 (gleich)
    +8.0 cent:  246.91 Hz Q    846 (gegen)    248.03 Hz Q    841 (gleich)
Abb. 8 (Koerper 0,2 kg, Q 40, 222 Hz)
    -8.0 cent:  246.08 Hz Q   2023 (gegen)    247.60 Hz Q    927 (gleich)
    -4.0 cent:  246.54 Hz Q   2545 (gegen)    247.70 Hz Q    860 (gleich)
    -2.0 cent:  246.74 Hz Q   2853 (gegen)    247.79 Hz Q    836 (gleich)
    -1.0 cent:  246.82 Hz Q   2960 (gegen)    247.84 Hz Q    830 (gleich)
    -0.5 cent:  246.86 Hz Q   2990 (gegen)    247.87 Hz Q    829 (gleich)
    +0.0 cent:  246.90 Hz Q   3000 (gegen)    247.91 Hz Q    829 (gleich)
    +0.5 cent:  246.93 Hz Q   2990 (gegen)    247.94 Hz Q    831 (gleich)
    +1.0 cent:  246.97 Hz Q   2960 (gegen)    247.98 Hz Q    835 (gleich)
    +2.0 cent:  247.02 Hz Q   2854 (gegen)    248.06 Hz Q    847 (gleich)
    +4.0 cent:  247.11 Hz Q   2549 (gegen)    248.26 Hz Q    884 (gleich)
    +8.0 cent:  247.22 Hz Q   2047 (gegen)    248.72 Hz Q    980 (gleich)
Abb. 8 (Koerper 0,2 kg, Q 40, 247 Hz)
    -8.0 cent:  241.85 Hz Q     85 (gleich)    246.34 Hz Q   1534 (gegen)
    -4.0 cent:  242.01 Hz Q     81 (gleich)    246.62 Hz Q   2419 (gegen)
    -2.0 cent:  246.76 Hz Q   2830 (gegen)    251.62 Hz Q     78 (gleich)
    -1.0 cent:  246.83 Hz Q   2956 (gegen)    251.66 Hz Q     79 (gleich)
    -0.5 cent:  246.86 Hz Q   2989 (gegen)    251.67 Hz Q     79 (gleich)
    +0.0 cent:  246.90 Hz Q   3000 (gegen)    251.69 Hz Q     79 (gleich)
    +0.5 cent:  246.94 Hz Q   2989 (gegen)    251.71 Hz Q     80 (gleich)
    +1.0 cent:  246.97 Hz Q   2956 (gegen)    251.73 Hz Q     80 (gleich)
    +2.0 cent:  247.04 Hz Q   2830 (gegen)    251.77 Hz Q     80 (gleich)
    +4.0 cent:  247.18 Hz Q   2420 (gegen)    251.85 Hz Q     82 (gleich)
    +8.0 cent:  247.46 Hz Q   1538 (gegen)    252.03 Hz Q     85 (gleich)

=== B  Gegenprobe: dieselbe Fallstudie mit Y = 1/c (Daempfer 300 N s/m) in der 2x2-Form
   +0.0 cent:  246.90 Hz Q    492    246.90 Hz Q   3000
   +0.5 cent:  246.94 Hz Q   2605    246.94 Hz Q    504
   +1.0 cent:  246.97 Hz Q   1787    246.97 Hz Q    553
   +2.0 cent:  246.95 Hz Q    845    247.14 Hz Q    844
   +8.0 cent:  246.91 Hz Q    845    248.03 Hz Q    842
   |eps| = w Z0 |Y| / pi = 0.660 1/s; Bereich der Anti-Veering-Verschmelzung |f2 - f1| < 2|eps|/2pi = 0.210 Hz = 1.47 cent

=== C  Choere an der Stegadmittanz der LumiViz-Koerper (Grundton, alle Moden der Datei und des statistischen Teils)
--- oud: Mensur 0.6 m, Verstimmung im Einklang 3.0 cent gesamt
Chor  Ton   f Hz   Z0 Ns/m  Re Y      Im Y      eta_b einzeln  eta Saite | einzeln: T60 s | doppelt, gleichphasig/gegen: T60 s | Schwebung Hz
   1   36    65.4   0.457  8.00e-04  +6.14e-02  2.33e-04      2.73e-03 |  11.45 |  10.53 /  12.31 | 1.176
   2   41    87.3   0.268  6.38e-03  +1.09e-01  1.09e-03      2.06e-03 |   8.06 |   5.96 /  12.21 | 1.636
   3   45   110.0   0.241  1.25e-02  +3.52e-02  1.91e-03      1.64e-03 |   5.63 |   3.71 /  11.58 | 0.621
   4   50   146.8   0.225  9.82e-03  +1.58e-01  1.40e-03      1.24e-03 |   5.69 |   3.70 /  12.00 | 3.338
   5   55   196.0   0.123  1.16e-02  +8.99e-02  9.05e-04      2.11e-03 |   3.73 |   2.88 /   5.25 | 1.417
   6   60   261.6   0.115  1.83e-01  +6.13e-01  1.34e-02      1.90e-03 |   0.55 |   0.29 /   4.39 | 11.763
--- laute: Mensur 0.625 m, Verstimmung im Einklang 4.0 cent gesamt
Chor  Ton   f Hz   Z0 Ns/m  Re Y      Im Y      eta_b einzeln  eta Saite | einzeln: T60 s | doppelt, gleichphasig/gegen: T60 s | Schwebung Hz
   1   38    73.4   0.332  2.08e-04  +1.41e-02  4.40e-05      1.82e-03 |  16.08 |  15.75 /  16.33 | 0.277
   2   41    87.3   0.302  4.10e-04  +1.58e-02  7.90e-05      1.69e-03 |  14.27 |  13.75 /  14.74 | 0.334
   3   43    98.0   0.271  7.55e-04  +1.84e-02  1.30e-04      1.62e-03 |  12.82 |  12.07 /  13.59 | 0.384
   4   48   130.8   0.191  3.09e-02  +2.77e-02  3.77e-03      1.51e-03 |   3.19 |   1.94 /   8.91 | 0.491
   5   53   174.6   0.149  2.05e-03  +3.06e-02  1.94e-04      2.15e-03 |   5.37 |   5.04 /   5.73 | 0.647
   6   57   220.0   0.121  7.12e-03  +7.25e-02  5.51e-04      2.03e-03 |   3.88 |   3.23 /   4.82 | 1.334
   7   62   293.7   0.083  2.01e-02  -6.95e-02  1.07e-03      1.97e-03 |   2.46 |   1.89 /   3.52 | 1.270
   8   67   392.0   0.093  5.05e-03  +3.98e-02  2.99e-04      1.73e-03 |   2.76 |   2.50 /   3.08 | 1.291

Oud A2 (Taste 45): Abklingzeiten T60 (s) der beiden Moden und Schwebung gegen die Verstimmung
     0.0 cent: T60   3.66 /  12.18 s, Frequenzabstand 0.594 Hz
     0.5 cent: T60   3.66 /  12.16 s, Frequenzabstand 0.594 Hz
     1.0 cent: T60   3.66 /  12.10 s, Frequenzabstand 0.597 Hz
     1.5 cent: T60   3.67 /  12.01 s, Frequenzabstand 0.601 Hz
     2.0 cent: T60   3.68 /  11.89 s, Frequenzabstand 0.606 Hz
     3.0 cent: T60   3.71 /  11.58 s, Frequenzabstand 0.621 Hz
     5.0 cent: T60   3.80 /  10.78 s, Frequenzabstand 0.667 Hz
     8.0 cent: T60   3.97 /   9.57 s, Frequenzabstand 0.771 Hz
    12.0 cent: T60   4.21 /   8.41 s, Frequenzabstand 0.954 Hz
   |eps| = 1.978 1/s -> Kopplungsbreite 2|eps|/2pi = 0.630 Hz = 9.88 cent

Oud A2 im 3-FG-Modell (Woodhouse 2021 Gl. 7-10) mit der Koerpermode, die am Stegpunkt am staerksten ist (80-200 Hz)
   Mode 168.6 Hz, eta 0.031, wirksame Masse am Stegpunkt 46.0 g
    0.0 cent:  109.61 Hz Q   536 T60 10.76 s (gleich)    110.00 Hz Q   609 T60 12.18 s (gegen)
    1.5 cent:  109.65 Hz Q   537 T60 10.77 s (gleich)    110.05 Hz Q   608 T60 12.15 s (gegen)
    3.0 cent:  109.68 Hz Q   539 T60 10.81 s (gleich)    110.12 Hz Q   605 T60 12.08 s (gegen)
    8.0 cent:  109.74 Hz Q   548 T60 10.99 s (gleich)    110.38 Hz Q   593 T60 11.81 s (gegen)

=== D  Oud C2 (Taste 36): einzeln gegen doppelt, nur der Grundton (Kraft am Steg ~ Summe der Saitenkraefte)
   einzeln: Abklingzeit T60 11.45 s; Pegel 1-2 s gegen 0-0,4 s -6.6 dB
   doppelt 0.0 cent: Anfang +6.0 dB, 400-ms-Fenster +5.9 dB gegen einzeln, Pegel 1-2 s gegen 0-0,4 s -7.2 dB, T60 der Moden 12.33 / 10.53 s
   doppelt 1.0 cent: Anfang +6.0 dB, 400-ms-Fenster +5.9 dB gegen einzeln, Pegel 1-2 s gegen 0-0,4 s -7.2 dB, T60 der Moden 10.53 / 12.33 s
   doppelt 3.0 cent: Anfang +6.0 dB, 400-ms-Fenster +5.9 dB gegen einzeln, Pegel 1-2 s gegen 0-0,4 s -7.2 dB, T60 der Moden 10.53 / 12.31 s
   doppelt 6.0 cent: Anfang +6.0 dB, 400-ms-Fenster +5.9 dB gegen einzeln, Pegel 1-2 s gegen 0-0,4 s -7.2 dB, T60 der Moden 10.53 / 12.29 s
```

### A3 `rechnung_basschor.py`

```python
"""Oud: Basschor C2 einzeln gegen doppelt, und Varianten der umsponnenen Saite, im Pruefstand S199 (nur gelesen,
importiert ohne Bytecode). Rechnet nur die Tasten 36-40 (Chor 1) neu; jede Taste ist im Pruefstand unabhaengig, die
uebrigen Tasten bleiben wie in `2026-11-15_S199_pruefstand_oud_je_taste.txt`. Schreibt nichts.
Aufruf: python -B -I <dieses Skript>
"""
import importlib.util
import sys

import numpy as np

sys.dont_write_bytecode = True
PFAD = (r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung"
        r"\gezupfte-saite\proben\2026-11-15_S199_pruefstand_zupf.py")
spec = importlib.util.spec_from_file_location("pruefstand", PFAD)
P = importlib.util.module_from_spec(spec)
spec.loader.exec_module(P)

# Lautheit (LUFS) aus 2026-11-15_S199_pruefstand_oud_je_taste.txt, Tasten 36-72
HEUTE = [-18.4, -16.2, -17.9, -17.4, -17.9, -14.7, -14.1, -11.5, -6.7, -10.9, -12.7, -10.1, -7.8, -7.2, -4.2, -5.7, -8.0,
         -11.1, -11.0, -10.3, -3.1, -10.0, -11.2, -7.1, -4.5, -2.9, -2.6, -2.6, -3.9, -5.8, -7.6, -8.8, -8.0, -10.0, -8.5,
         -8.8, -12.1]


def masse(l):
    l = np.asarray(l)
    d3 = np.abs(l[3:] - l[:-3])
    paare = [abs(l[i + k] - l[i]) for k in range(1, 5) for i in range(len(l) - k)]
    return f"Spanne {l.max() - l.min():.1f} LU, Terzabstand: groesster {d3.max():.1f} / mittlerer (Median) {np.median(d3):.1f} LU; " \
           f"Paare bis 4 Halbtoene: groesster {max(paare):.1f} / Mittel {np.mean(paare):.1f} LU"


kp = P.lade_koerper("oud")
w = P.frequenzen()
tan = P.richtung_tan(kp["f"])
ch, mensur, cent = P.choere("oud")
punkt = kp["tasten"].index(36)
einzel = ch[0][1][0]
core = dict(einzel, EI=23.1e-6)            # EI des Kerns als massives Nylon (rechnung_saiten.py), statt 50e-6 uebertragen
varianten = {
    "einzeln (heute)": [einzel],
    "doppelt, +-1,5 cent": [einzel, einzel],
    "einzeln, EI Kern 23e-6": [core],
}
v, e = P.VORLAUF, P.VORLAUF + 0.4
ergebnis = {}
for name, saiten in varianten.items():
    Y, H = P.koerper_matrix(kp, w, tan, [punkt] * len(saiten))
    werte = []
    for taste in range(36, 41):
        bund = taste - 36
        laenge = mensur * 2 ** (-bund / 12)
        laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if len(saiten) == 2 else laenge for i in range(len(saiten))]
        xp = min(P.ZUPFORT, 0.5 * laenge)
        x = P.zeit(P.ton_mit(saiten, laengen, list(range(len(saiten))), xp, 0.002, 30.0, Y, H, w)[0])
        werte.append((taste, P.momentary_max(x), P.rms_db(x, 1, 2) - P.rms_db(x, v, e)))
        print(f"{name:24s} Taste {taste}: {werte[-1][1]:6.1f} LUFS, Pegel 1-2 s gegen Anfang {werte[-1][2]:6.1f} dB", flush=True)
    ergebnis[name] = werte
print()
for name, werte in ergebnis.items():
    l = [z[1] for z in werte] + HEUTE[5:]
    print(f"{name:24s}: {masse(l)}")
```

Ausgabe des Laufs in diesem Auftrag (Hintergrund, rund 5 Minuten):

```text
einzeln (heute)          Taste 36:  -18.4 LUFS, Pegel 1-2 s gegen Anfang  -14.9 dB
einzeln (heute)          Taste 37:  -16.2 LUFS, Pegel 1-2 s gegen Anfang  -18.3 dB
einzeln (heute)          Taste 38:  -17.9 LUFS, Pegel 1-2 s gegen Anfang  -16.5 dB
einzeln (heute)          Taste 39:  -17.4 LUFS, Pegel 1-2 s gegen Anfang  -17.5 dB
einzeln (heute)          Taste 40:  -17.9 LUFS, Pegel 1-2 s gegen Anfang  -17.0 dB
doppelt, +-1,5 cent      Taste 36:  -14.0 LUFS, Pegel 1-2 s gegen Anfang  -16.9 dB
doppelt, +-1,5 cent      Taste 37:  -12.5 LUFS, Pegel 1-2 s gegen Anfang  -19.0 dB
doppelt, +-1,5 cent      Taste 38:  -13.7 LUFS, Pegel 1-2 s gegen Anfang  -17.8 dB
doppelt, +-1,5 cent      Taste 39:  -13.0 LUFS, Pegel 1-2 s gegen Anfang  -19.2 dB
doppelt, +-1,5 cent      Taste 40:  -13.5 LUFS, Pegel 1-2 s gegen Anfang  -17.9 dB
einzeln, EI Kern 23e-6   Taste 36:  -18.4 LUFS, Pegel 1-2 s gegen Anfang  -14.9 dB
einzeln, EI Kern 23e-6   Taste 37:  -16.2 LUFS, Pegel 1-2 s gegen Anfang  -18.2 dB
einzeln, EI Kern 23e-6   Taste 38:  -17.9 LUFS, Pegel 1-2 s gegen Anfang  -16.4 dB
einzeln, EI Kern 23e-6   Taste 39:  -17.4 LUFS, Pegel 1-2 s gegen Anfang  -17.5 dB
einzeln, EI Kern 23e-6   Taste 40:  -17.8 LUFS, Pegel 1-2 s gegen Anfang  -17.0 dB

einzeln (heute)         : Spanne 15.8 LU, Terzabstand: groesster 8.3 / mittlerer (Median) 3.2 LU; Paare bis 4 Halbtoene: groesster 11.2 / Mittel 3.0 LU
doppelt, +-1,5 cent     : Spanne 12.1 LU, Terzabstand: groesster 8.3 / mittlerer (Median) 2.4 LU; Paare bis 4 Halbtoene: groesster 8.6 / Mittel 2.8 LU
einzeln, EI Kern 23e-6  : Spanne 15.8 LU, Terzabstand: groesster 8.3 / mittlerer (Median) 3.2 LU; Paare bis 4 Halbtoene: groesster 11.1 / Mittel 3.0 LU
```

### A4 `rechnung_bauplan.py`

```python
"""Gegenrechnungen zum Bauplan Oud / Laute (Auftrag 2026-11-16_15). Liest nur die Umriss-Dateien im LumiViz-Repo.
Aufruf: python -I <dieses Skript>
"""
import json
import math

PROBEN = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\gezupfte-saite\proben"
C_LUFT = 343.0


def flaeche_stationen(st, links, rechts):
    return sum((st[k + 1] - st[k]) * ((rechts[k] - links[k]) + (rechts[k + 1] - links[k + 1])) / 2 for k in range(len(st) - 1))


def helmholtz(V, loecher, dicke, muendung=1.7):
    """f = c/2pi sqrt( sum A_i / l_i / V ), l_i = Dicke + muendung * r_i (Bauplan.hpp: Muendungskorrektur in Radien)."""
    leit = sum(math.pi * r * r / (dicke + muendung * r) for r in loecher)
    return C_LUFT / (2 * math.pi) * math.sqrt(leit / V)


def entkoppelt(f_minus, f_plus, f_luft):
    """Zwei gekoppelte Schwinger: f-^2 + f+^2 = f_Luft^2 + f_Decke^2 (S196 Par. 5) -> f_Decke."""
    r = f_minus ** 2 + f_plus ** 2 - f_luft ** 2
    return math.sqrt(r) if r > 0 else float("nan")


# ---------------- Oud (Abdallah u. a. 2026, Umriss aus Abb. 3, S195)
o = json.load(open(PROBEN + r"\2026-11-12_S195_oud_umriss.json", encoding="utf-8-sig"))
st = [u[0] / 100 for u in o["umriss"]]
A_oud = flaeche_stationen(st, [u[1] / 100 for u in o["umriss"]], [u[2] / 100 for u in o["umriss"]])
loecher_abd = [0.053, 0.020, 0.020]
loecher_ina = [0.0425, 0.021, 0.021]          # Inanli & Altinsoy 2018 Abb. 2: Durchmesser 85 / 42 / 42 mm
A_l = sum(math.pi * r * r for r in loecher_abd)
print(f"Oud: Umriss {A_oud * 1e4:.0f} cm^2 (Laenge {st[-1] * 100:.1f} cm, groesste Breite {o['groesste_breite_cm']:.1f} cm); "
      f"Loecher Abdallah {A_l * 1e4:.0f} cm^2; netto {(A_oud - A_l) * 1e4:.0f} cm^2")
V = 0.020613
for d in (0.0015, 0.0020):
    m_platte = (A_oud - A_l) * d * 460
    print(f"   Decke {d * 1e3:.1f} mm, Sitka 460 kg/m3: {m_platte * 1e3:.0f} g")
BALKEN = [(26.0, 10.0, 4.2), (34.0, 8.0, 4.2), (33.7, 7.0, 5.0), (9.8, 7.0, 6.6), (9.8, 7.0, 6.6), (25.5, 10.0, 4.2), (22.0, 10.0, 4.2)]
m_balken = sum(L / 100 * ((c + 16.5) / 2 / 1000) * b / 1000 * 460 for L, c, b in BALKEN)   # Hoehe linear c -> 16,5 -> c
print(f"   Balken (make_oud.py: Mitte 16,5 mm, Enden c, Breite Tab. 2): {m_balken * 1e3:.1f} g")
INANLI = [(300, 200, 50, 12, 6, 5), (355, 255, 50, 12, 6, 6), (355, 255, 50, 5, 6, 5), (344, 244, 50, 12, 6, 5), (118, 48, 35, 12, 6, 5),
          (278, 178, 50, 12, 6, 6), (222, 122, 50, 12, 6, 6)]
m_ina = sum((L2 * h1 + 2 * L3 * (h1 + h2) / 2) * w for L1, L2, L3, h1, h2, w in INANLI) * 1e-9 * 460
print(f"   Balken Inanli Tab. 1 (L2 voll, L3 linear auf h2), bei 460 kg/m3: {m_ina * 1e3:.1f} g; Pruefung L1 = L2 + 2 L3: "
      + ", ".join(str(L1 - L2 - 2 * L3) for L1, L2, L3, *_ in INANLI))
for name, l in (("Abdallah (R 5,3 / 2,0 / 2,0 cm)", loecher_abd), ("Inanli (85 / 42 / 42 mm)", loecher_ina)):
    for d in (0.0015, 0.002):
        print(f"   Helmholtz starr, V {V * 1e3:.2f} l, {name}, Wand {d * 1e3:.1f} mm: {helmholtz(V, l, d):.0f} Hz")
fh = helmholtz(V, loecher_abd, 0.0015)
print(f"   gemessen (Abdallah Abb. 6, S196): Luft 101 Hz, erste Deckenmode 203 Hz -> entkoppelte Decke "
      f"{entkoppelt(101, 203, fh):.0f} Hz (mit f_Luft = {fh:.0f} Hz)")
for V_ in (0.016, 0.018, 0.020613, 0.023):
    print(f"   Volumen {V_ * 1e3:.1f} l -> Helmholtz starr {helmholtz(V_, loecher_abd, 0.0015):.0f} Hz")

# ---------------- Laute (Rauwolf, Massstab 625/677,4)
la = json.load(open(PROBEN + r"\2026-11-13_S197_laute_umriss.json", encoding="utf-8-sig"))
s = 625 / 677.4
st = [u[0] / 1000 for u in la["umriss"]]
A_la = flaeche_stationen(st, [u[1] / 1000 for u in la["umriss"]], [u[2] / 1000 for u in la["umriss"]])
r_rose = la["rose"]["durchmesser"] / 2000
print()
print(f"Laute Rauwolf 1:1: Umriss {A_la * 1e4:.0f} cm^2, Rose {math.pi * r_rose ** 2 * 1e4:.0f} cm^2; mal Massstab^2: {A_la * s * s * 1e4:.0f} cm^2")
for d in (0.0015, 0.0018, 0.0020):
    m = (A_la - 0.85 * math.pi * r_rose ** 2) * s * s * d * 460
    print(f"   Decke {d * 1e3:.1f} mm (gestreckt, Rose 85 % offen), 460 kg/m3: {m * 1e3:.0f} g")
quer = [q["flaeche_mm2"] for q in la["querschnitte"]]
print(f"   Querschnitte (mm^2): {[round(q) for q in quer]}")
V1 = 0.0140
for offen in (0.5, 0.85, 1.0):
    for V_, name in ((V1, "1:1, 14,0 l"), (V1 * s ** 3, f"gestreckt {V1 * s ** 3 * 1e3:.1f} l")):
        r = (r_rose * math.sqrt(offen)) * (1 if V_ == V1 else s)
        print(f"   Helmholtz starr, {name}, Rose {offen * 100:.0f} % offen (flaechengleicher Kreis r {r * 1e3:.1f} mm), Decke 1,8 mm: "
              f"{helmholtz(V_, [r], 0.0018):.0f} Hz")
fh = helmholtz(V1 * s ** 3, [r_rose * math.sqrt(0.85) * s], 0.0018)
print(f"   gemessen (Woodhouse 2021 Abb. 1, S197): Luft 132 Hz, Decke 277 Hz -> entkoppelte Decke {entkoppelt(132, 277, fh):.0f} Hz (f_Luft {fh:.0f} Hz)")
# Steg nach der Zeichnung (dieser Lauf, Bild seiten/rauwolf_steg.png): Laenge 139, Breite 10 -> 7, Hoehe 7 -> 6 (vorn) bzw. 8 -> 7 (hinten)
for rho in (600.0, 700.0, 750.0):
    vol = 139e-3 * (10e-3 + 7e-3) / 2 * (7.5e-3 + 6.5e-3) / 2
    print(f"   Steg Rauwolf (Quader-Naeherung 139 x 8,5 x 7 mm = {vol * 1e6:.2f} cm^3), {rho:.0f} kg/m3: {vol * rho * 1e3:.1f} g, "
          f"gestreckt (Massstab^3) {vol * rho * s ** 3 * 1e3:.1f} g")
```

Ausgabe:

```text
Oud: Umriss 1463 cm^2 (Laenge 50.0 cm, groesste Breite 37.0 cm); Loecher Abdallah 113 cm^2; netto 1350 cm^2
   Decke 1.5 mm, Sitka 460 kg/m3: 93 g
   Decke 2.0 mm, Sitka 460 kg/m3: 124 g
   Balken (make_oud.py: Mitte 16,5 mm, Enden c, Breite Tab. 2): 43.0 g
   Balken Inanli Tab. 1 (L2 voll, L3 linear auf h2), bei 460 kg/m3: 49.2 g; Pruefung L1 = L2 + 2 L3: 0, 0, 0, 0, 0, 0, 0
   Helmholtz starr, V 20.61 l, Abdallah (R 5,3 / 2,0 / 2,0 cm), Wand 1.5 mm: 155 Hz
   Helmholtz starr, V 20.61 l, Abdallah (R 5,3 / 2,0 / 2,0 cm), Wand 2.0 mm: 155 Hz
   Helmholtz starr, V 20.61 l, Inanli (85 / 42 / 42 mm), Wand 1.5 mm: 148 Hz
   Helmholtz starr, V 20.61 l, Inanli (85 / 42 / 42 mm), Wand 2.0 mm: 147 Hz
   gemessen (Abdallah Abb. 6, S196): Luft 101 Hz, erste Deckenmode 203 Hz -> entkoppelte Decke 165 Hz (mit f_Luft = 155 Hz)
   Volumen 16.0 l -> Helmholtz starr 176 Hz
   Volumen 18.0 l -> Helmholtz starr 166 Hz
   Volumen 20.6 l -> Helmholtz starr 155 Hz
   Volumen 23.0 l -> Helmholtz starr 147 Hz

Laute Rauwolf 1:1: Umriss 1263 cm^2, Rose 95 cm^2; mal Massstab^2: 1075 cm^2
   Decke 1.5 mm (gestreckt, Rose 85 % offen), 460 kg/m3: 69 g
   Decke 1.8 mm (gestreckt, Rose 85 % offen), 460 kg/m3: 83 g
   Decke 2.0 mm (gestreckt, Rose 85 % offen), 460 kg/m3: 93 g
   Querschnitte (mm^2): [43353, 40438, 30845, 8841, 39124, 5186, 27000]
   Helmholtz starr, 1:1, 14,0 l, Rose 50 % offen (flaechengleicher Kreis r 38.9 mm), Decke 1,8 mm: 122 Hz
   Helmholtz starr, gestreckt 11.0 l, Rose 50 % offen (flaechengleicher Kreis r 35.9 mm), Decke 1,8 mm: 132 Hz
   Helmholtz starr, 1:1, 14,0 l, Rose 85 % offen (flaechengleicher Kreis r 50.7 mm), Decke 1,8 mm: 140 Hz
   Helmholtz starr, gestreckt 11.0 l, Rose 85 % offen (flaechengleicher Kreis r 46.8 mm), Decke 1,8 mm: 151 Hz
   Helmholtz starr, 1:1, 14,0 l, Rose 100 % offen (flaechengleicher Kreis r 55.0 mm), Decke 1,8 mm: 146 Hz
   Helmholtz starr, gestreckt 11.0 l, Rose 100 % offen (flaechengleicher Kreis r 50.7 mm), Decke 1,8 mm: 158 Hz
   gemessen (Woodhouse 2021 Abb. 1, S197): Luft 132 Hz, Decke 277 Hz -> entkoppelte Decke 267 Hz (f_Luft 151 Hz)
   Steg Rauwolf (Quader-Naeherung 139 x 8,5 x 7 mm = 8.27 cm^3), 600 kg/m3: 5.0 g, gestreckt (Massstab^3) 3.9 g
   Steg Rauwolf (Quader-Naeherung 139 x 8,5 x 7 mm = 8.27 cm^3), 700 kg/m3: 5.8 g, gestreckt (Massstab^3) 4.5 g
   Steg Rauwolf (Quader-Naeherung 139 x 8,5 x 7 mm = 8.27 cm^3), 750 kg/m3: 6.2 g, gestreckt (Massstab^3) 4.9 g
```

### A5 `rechnung_rest.py`

```python
"""Sprungmasse der Laute aus der Pruefstand-Ausgabe S199; Luftreibung umsponnener Saiten nach Stokes (W&LA Gl. 11-12)
gegen die Uebertragung (Valette, eta_A = 1,1 1/s). Aufruf: python -I <dieses Skript>"""
import math

import numpy as np

LAUTE = [-16.7, -16.4, -15.8, -16.7, -16.4, -16.5, -15.0, -13.2, -10.0, -8.7, -8.4, -7.9, -9.0, -9.8, -10.9, -15.1, -16.2, -15.2,
         -12.0, -12.2, -7.6, -7.0, -5.0, -4.1, -7.6, -8.1, -7.9, -15.9, -16.2, -18.0, -17.3, -14.8, -11.1, -15.2, -15.8, -12.5,
         -15.7, -16.8, -13.5, -13.4, -16.1, -17.3]          # Tasten 38-79, 2026-11-15_S199_pruefstand_laute_je_taste.txt
l = np.array(LAUTE)
d1 = np.abs(np.diff(l))
d3 = np.abs(l[3:] - l[:-3])
paare = [abs(l[i + k] - l[i]) for k in range(1, 5) for i in range(len(l) - k)]
print(f"Laute: Spanne {l.max() - l.min():.1f}; Nachbarn max {d1.max():.1f} Median {np.median(d1):.1f} Mittel {d1.mean():.1f}; "
      f"Terz max {d3.max():.1f} Median {np.median(d3):.1f}; Paare <= 4 HT max {max(paare):.1f} Mittel {np.mean(paare):.1f} Median {np.median(paare):.1f}")

print()
print("Umsponnene Oud-Saiten: Luftreibung Stokes am Aussendurchmesser (rho = mu / Querschnitt) gegen Valette-Uebertragung")
UMSP = [("D3", 146.83, 0.610, 1.274e-3), ("A2", 110.0, 0.711, 1.824e-3), ("F2", 87.31, 0.813, 2.555e-3), ("C2", 65.41, 1.092, 5.825e-3)]
for name, f, D_mm, mu in UMSP:
    D = D_mm / 1000
    rho = mu / (math.pi * D * D / 4)
    for n in (1, 10):
        fn = n * f
        M = D / 4 * math.sqrt(2 * math.pi * fn / 1.5e-5)
        stokes = 1.2 / rho * (2 * math.sqrt(2) * M + 1) / M ** 2
        valette = 5e-5 + 1.1 / (2 * math.pi * fn)
        print(f"   {name} n={n:2d}: Stokes {stokes:.2e} (mit Zuschlag d+0,2: {stokes * (D_mm + 0.2):.2e})   Valette eta_F + eta_A/w: {valette:.2e}")
```

Ausgabe:

```text
Laute: Spanne 13.9; Nachbarn max 8.0 Median 1.1 Mittel 1.8; Terz max 10.1 Median 3.1; Paare <= 4 HT max 11.8 Mittel 3.0 Median 2.4

Umsponnene Oud-Saiten: Luftreibung Stokes am Aussendurchmesser (rho = mu / Querschnitt) gegen Valette-Uebertragung
   D3 n= 1: Stokes 8.43e-04 (mit Zuschlag d+0,2: 6.83e-04)   Valette eta_F + eta_A/w: 1.24e-03
   D3 n=10: Stokes 2.25e-04 (mit Zuschlag d+0,2: 1.82e-04)   Valette eta_F + eta_A/w: 1.69e-04
   A2 n= 1: Stokes 7.92e-04 (mit Zuschlag d+0,2: 7.21e-04)   Valette eta_F + eta_A/w: 1.64e-03
   A2 n=10: Stokes 2.12e-04 (mit Zuschlag d+0,2: 1.93e-04)   Valette eta_F + eta_A/w: 2.09e-04
   F2 n= 1: Stokes 7.22e-04 (mit Zuschlag d+0,2: 7.32e-04)   Valette eta_F + eta_A/w: 2.06e-03
   F2 n=10: Stokes 1.94e-04 (mit Zuschlag d+0,2: 1.96e-04)   Valette eta_F + eta_A/w: 2.51e-04
   C2 n= 1: Stokes 4.76e-04 (mit Zuschlag d+0,2: 6.15e-04)   Valette eta_F + eta_A/w: 2.73e-03
   C2 n=10: Stokes 1.30e-04 (mit Zuschlag d+0,2: 1.68e-04)   Valette eta_F + eta_A/w: 3.18e-04
```

### A6 `rauwolf_steg.py`

Versuch, die Masszahlen am Steg ueber `pdftotext -bbox` zu finden; die installierte Xpdf-Fassung kennt `-bbox` nicht. Der Ausschnitt `seiten/rauwolf_steg.png` entstand stattdessen direkt mit `pdftoppm -r 120 -x 3960 -y 3170 -W 1040 -H 530` nach einer Uebersicht bei 20 dpi (`seiten/rauwolf_ganz20.png`). Das Skript bleibt als Beleg des Versuchs liegen, ohne Ausgabe.

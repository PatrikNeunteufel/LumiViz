> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_13_gitarre_grundlagen` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 22:45–23:05, claude-opus-5-5, 92 Runden, success, 5.59 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202** (Seitenbilder des Laufs im Archiv der Werkbank, `seiten/`):
> - **Wright 1996 Tab. 5.4 und Abb. 5.5 (gedr. S. 98) am Seitenbild:** alle neun Zeilen (f, Masse, Q, wirksame Fläche)
>   stimmen; Mikrofon 0,9 m vor dem Steg, Antrieb tiefe E, **Schallloch zugeklebt** — bestätigt.
> - **Scherrer 2013 Abb. 3.8 (gedr. S. 58) am Seitenbild und nachgerechnet:** F = T h (1/x₀ + 1/(L − x₀)) mit T = 70 N,
>   L = 0,65 m gibt für h = 1 cm in der Mitte 4,3 N, bei x₀ = 1 cm 71 N; die Kurve zeigt 0,45 N und 7 N. Die Achse
>   passt zu **1 mm**, nicht zu 1 cm — der Befund „um den Faktor 10 falsch beschriftet“ stimmt.
> - **Modendichte — Vergleich in verschiedenen Bändern:** der Bericht stellt Elies 0,018–0,022 /Hz (gemessen 800–2200 Hz)
>   gegen „0,0104 /Hz der Körperdatei“ — das ist `statistical.density` aus den Polen in **2,5–5 kHz**. Selbst gezählt in
>   `classical_guitar.lvinstbody`: **800–2200 Hz 26 Moden = 0,0186 /Hz** (wie Elie), 2,2–5 kHz 0,0111 /Hz. Die Datei
>   stimmt also im Band der Messung mit Elie überein; ihre Dichte **fällt über 2,2 kHz** auf rund die Hälfte, während Elie
>   die Dichte dort „roughly constant“ nennt. Die Folgerung für den statistischen Teil (Dichte nach Elie) bleibt
>   richtig; „fast doppelt so viel wie die Körperdatei“ gilt nur für 2,5–5 kHz.
> - Bank & Karjalainen 2010 Gl. (4)–(6) stimmen (am Seitenbild DAFX-2, Gegenlesen von Bericht 11).
> - **Nicht gegengelesen:** Wright Gl. 4.9–4.17 (S. 65–67), Woodhouse 2021 Gl. 1, die Rechnungen r1–r5 (Nahfeld → 1 m
>   je Mode −11 / −21 dB, Eigenwerte der 2×2-Admittanz), Elie Abb. 9, Mores List (Spaltenbedeutung).
> - **Nacharbeit:** Auftrag `2026-11-17_23_gitarre_nacharbeit` der Werkbank.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_13_gitarre_grundlagen.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_13_gitarre_grundlagen/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_gitarre_grundlagen_werkbank/`: `r1_koerper.py`, `r2_nah_fern.py`, `r3_zwei_kolben.py`, `r4_zwei_richtungen.py`, `r5_luecken_2_3_4.py`.

# Bericht: Klassische Gitarre — Berechnungsgrundlagen für Hörorte in 1 m, zweite Stegrichtung, Zupf und Teil über 5 kHz

- **Auftrag:** `auftraege/2026-11-16_13_gitarre_grundlagen.md` (Stufe 2 zu `2026-11-16_03_gitarre_fernfeld_zweite_richtung`)
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5
- **Voraussetzung:** Stufe-1-Bericht `ergebnisse/2026-11-16_03_gitarre_fernfeld_zweite_richtung/bericht.md` liegt vor und
  wurde ganz gelesen.
- **Reichweite:** 0 Websuchen, 0 Abrufe (keine neue Recherche, wie verlangt). Gelesen: 8 lokale PDFs unter
  `…\cmake\quellen\gezupfte-saite\` (Woodhouse 2021, Wright 1996, Scherrer 2013, Traube 2004, Mores 2021 Method und List,
  Ducceschi 2026 — ohne Treffer zu Holzwerten), dazu zwei PDFs, die die Stufe 1 schon geholt hat
  (`ergebnisse/2026-11-16_03_…/abrufe/2012_ElieetalJasa2012.pdf` = Elie u. a. 2012, `…/abrufe/2010_2307.pdf` = Bank &
  Karjalainen 2010) und die Textauszüge der Stufe 1 (`woodhouse2012.txt`, `scherrer2013.txt`, `traube2004.txt`,
  `elie2012.txt`, `bank2010.txt`). **24 Seiten am Bild geprüft** (unter `seiten/`): Woodhouse 2021 PDF-S. 3, 4, 5, 7, 8, 9;
  Wright 1996 PDF-S. 74–78, 104–106, 108, 148; Scherrer 2013 PDF-S. 91; Elie 2012 S. 9, 10, 11; Bank 2010 S. 2;
  Mores List S. 1. LumiViz (nur gelesen): `2026-11-15_S199_gitarre_modalfit.md`, `2026-11-16_S201_gitarre_senke_780_880.md`,
  `make_gitarre.py` (Kopf), `proben/2026-11-15_S199_pruefstand_zupf.py` (Docstring und Gitarrenteil),
  `classical_guitar.lvinstbody` (Moden, statistischer Teil).
- **Nicht erreicht / nicht vorhanden:** alle Quellen der Ladeliste der Stufe 1 (Perry 2015, Woodhouse 2004 a/b,
  Lambourg & Chaigne 1993 …) liegen weiter nicht vor; dazu fehlen lokal Gough 1981, Christensen & Vistisen 1980 und
  Pavlidou 1997 (Ladeliste unten).
- **Aufgebaut wird auf** (nicht wiederholt): Mores 2021 (Methode, Mikrofonorte), Woodhouse u. a. 2012 (Saitentabelle),
  Wright 1996 Tab. 5.4 (Masse, Q), Scherrer 2013 Abb. A.11, und alle Funde des Stufe-1-Berichts.

**Kurzbefund.**
(1) Für Nahfeld → 1 m gibt es **keine gemessene Terztabelle** an einer klassischen Gitarre in den lesbaren Quellen;
neu belegt ist aber ein **gemessenes Fernfeld in 0,9 m mit Kraft am Steg** an der klassischen Gitarre BR2 (Wright 1996
§5.3, reflexionsarm, Monopolflächen je Mode) und das **Zwei-Oszillator-Modell** (Wright Gl. 4.9–4.17). Daraus
gerechnet: an den Mores-Orten ist das Verhältnis Fern(1 m)/Nah(10 cm) **je Mode verschieden** — rund −11 dB an A0,
rund −21 dB an T1 und darüber (bis 300 Hz), weil Schallloch und Decke im Nah- und Fernfeld verschieden gewichtet
werden; ein Kolben allein gibt −22…−24 dB, unabhängig vom Band bis ka ≈ 1. Ein Quellenmodell lässt sich aus den drei
Mores-Mikrofonen **nicht eindeutig** bestimmen (Begründung §1.1). (2) Die 2×2-Stegadmittanz als Modalsumme ist belegt
(Bank 2010 Gl. 4, 5 am Bild), die Abklingraten beider Polarisationen folgen als **Eigenwerte von Y am Saitenton**
(hergeleitet, an Woodhouse 2021 Gl. 1 und an der exakten Wurzel auf 0–4 % geprüft). Die Bank-Zahlen lassen sich mit
**einem** Winkel je Mode nicht treffen — es braucht stark gestreute Winkel oder eigene Moden der Parallelrichtung
(Bank selbst nimmt volle 2×2-Matrizen je Pol). (3) Zupf: belegt sind Auslenkung 0,8 / 4 mm bei 16 cm (zwei Stufen), die
Kraftstufe als Statik, Fingerbreite als Gauss (Woodhouse 2012) und Zupforte je Klangfarbe (Traube: 5 / 8 / 13 / 20 cm,
„normal“ 12 cm); **Zupfort je Bund an Spielern gemessen: nicht gefunden.** Scherrer Abb. 3.8 ist um den Faktor 10 falsch
beschriftet. (4) Modendichte gemessen (Elie 2012, zwölf Gitarren): **0,018–0,022 Moden/Hz**, fast doppelt so viel wie
die Polzahl von g34 in der Körperdatei (0,0104/Hz). Dämpfung über 5 kHz: nur die g34-Messung selbst (bis 10 kHz).

Hilfsdateien in diesem Ordner: `r1_koerper.py` … `r5_luecken_2_3_4.py` (unten mit Ausgabe), `seiten/` (geprüfte
Seitenbilder).

Marken: `[wörtlich]` aus der Textebene gelesen; `[am Seitenbild geprüft]`; `[aus Abbildung abgelesen]`; `[hergeleitet]`
(eigene Herleitung, Rechenweg angegeben); `[Lehrbuch]` (Standardformel, hier nicht aus einer lokalen Quelle gelesen,
nur über Grenzfälle gegengerechnet); `[eigene Rechnung]`, `[eigene Einschätzung]`, `[Annahme]`.

---

## 1. Übertragung Nahfeld (10 cm) → 1 m

### 1.1 Gleichungen

Zeitkonvention e^{iωt} (wie Wright und der Prüfstand).

| Nr. | Gleichung | Quelle, Stelle | Marke |
|---|---|---|---|
| G1.1 | Monopol: p̂ = (iωρ / 4πr) · U · e^{−ikr}, U = u·A (Kolben der Fläche A mit Schnelle u) | Wright 1996 Gl. 4.17, gedr. S. 67 (PDF-S. 77) | [am Seitenbild geprüft] |
| G1.2 | Dipol: zwei Monopole gleicher Stärke, Phase π, Abstand d — p = (iωρU/4π)·(e^{−ikr₁}/r₁ − e^{−ikr₂}/r₂); je Mode eine Monopol- und eine Dipolfläche | Wright gedr. S. 67 (Text nach Brooke 1992); Formel selbst | [am Seitenbild geprüft] (Text) / [hergeleitet] (Formel) |
| G1.3 | Wirksame Fläche A: „when the area A moves with the same displacement as the driving point, the volume of air displaced is equal to the volume displaced by the whole mode“ — negative Werte möglich | Wright gedr. S. 64 (PDF-S. 74) | [am Seitenbild geprüft] |
| G1.4 | Decke + Schallloch (Christensen & Vistisen): m_t ẍ_t = F − k_t x_t − R_t ẋ_t + A_t ΔP; m_h ẍ_h = A_h ΔP − R_h ẋ_h | Wright Gl. 4.9, 4.10, gedr. S. 65 | [am Seitenbild geprüft] |
| G1.5 | x_t = F D_h / (D_h D_t − α²), x_h = −α F / (D_h D_t − α²), α = μ A_h A_t, D_t = m_t(ω_t² − ω² + iωγ_t), D_h = m_h(ω_h² − ω² + iωγ_h); Steg-Admittanz χ_t = iω D_h / (D_h D_t − α²) | Wright Gl. 4.11–4.16, gedr. S. 66 | [am Seitenbild geprüft] |
| G1.6 | ΔP = −μ (A_t x_t + A_h x_h), μ = ρc²/V; ω_h² = μA_h²/m_h | aus G1.4 (x nach außen positiv); Wright nennt den Ausdruck im gelesenen Teil nicht (Anhang A.1 nicht gelesen) | [hergeleitet] |
| G1.7 | Fernfeld vorn: p/F = iωρ·iω(A_t x_t + A_h x_h) / (4πr) | G1.1 mit G1.5 | [hergeleitet] |
| G1.8 | Nahfeld über einer Fläche in einer Schallwand (Rayleigh): p(**r**) = (iωρ/2π) ∫ v(**r**′) e^{−ikR}/R dS | Standardformel | [Lehrbuch] — Grenzfälle in r2 gegengerechnet: z → 0 gibt Hess Gl. (2) genau (−31,3 dB), Fernfeld gibt G1.1 ×2 |
| G1.9 | Hess Gl. (2): p_F = a/(4r) · p_N (Nahabstand ≪ a, ka < 1, Vollraum) | Hess 2013 S. 2 (Stufe 1, am Bild) | [am Seitenbild geprüft] (Stufe 1); als Grenzfall von G1.8 bestätigt [eigene Rechnung] |
| G1.10 | Kolben, Achse, ka ≪ 1, Nahabstand z: p_N ≈ iωρu(√(z²+a²) − z) ⇒ p_F/p_N = a² / [4r(√(z²+a²) − z)] | aus G1.8 | [hergeleitet]; a = 0,109 m, z = 0,1 m, r = 1 m → −24,2 dB (Rayleigh numerisch: −24,2 dB) |
| G1.11 | Modalform für LumiViz: p_m/F = Σ_j φ_j(x_s) G_jm (−ω²)/(ω_j² − ω² + iη_jω_jω); im Fernfeld eines Monopols **G_j(r) = ρ V_j e^{−ikr} / (4πr)** mit V_j = A_j · φ_j(x_Antrieb) (Nettovolumen je Modalamplitude, m²/√kg) | Form des Prüfstands (`koerper_kurven`) + G1.1, G1.3 | [hergeleitet] |

**Ist ein Quellenmodell aus den drei Mores-Mikrofonen bestimmbar?** [hergeleitet / eigene Einschätzung]

- Messung je Mode: drei komplexe Gewichte G_jm (bass, treble, upper east), alle **in derselben Ebene 10 cm über der
  Decke** (Mores Method PDF-S. 2: „treble: 10 cm from the bridge center to the treble side; bass: 10 cm … to the bass
  side; upper east: 20 cm away from the treble position; all of them 10 cm above the surface of the top plate“
  [wörtlich]; Mores selbst: „This minimum set is very sparse and does not allow near-field reconstruction“ [wörtlich]).
  Die Körperdatei trägt nur bass/treble (`listeningPoints: left, right`); upper east steckt nur in der Messdatei.
- Unbekannte je Mode im kleinsten sinnvollen Modell: Monopol (1 reell, Vorzeichen) + Dipol in der Ebene (2) + Dipol
  senkrecht zur Decke (1) + Ort (2) = 6 reelle Größen gegen 3 komplexe Messwerte, deren Phasen bei Normalmoden
  nahezu fest sind (S201 §3: Rang-1-Anteil 0,95, reelles h) → effektiv **3 reelle Daten gegen 6 Unbekannte:
  unterbestimmt**. Der für vorn/hinten entscheidende **senkrechte Dipol** (Decke gegen Boden) und der Monopol wirken in
  einer einzigen Messebene bei kleinem kR beide nur über den seitlichen Abstand und sind praktisch nicht trennbar.
- Grundsätzlicher: eine Multipolentwicklung um **einen** Mittelpunkt konvergiert nur außerhalb der kleinsten Kugel, die
  alle Quellen umschließt (Korpus 493 mm lang, Mores List S. 1 → Radius ≈ 0,25 m). Die Mikrofone in 10 cm liegen
  **innerhalb** dieser Kugel — dort beschreibt „Monopol + Dipol am Schallloch/Decke“ den Druck nicht; man braucht
  verteilte Quellen (G1.8) mit Kenntnis der Schwingform, die Mores nicht misst. [Lehrbuch / eigene Einschätzung]
- **Bestimmbar** wird es nur mit Vorwissen: Ort und Form der Quellen fest (Kolben Decke am Steg, Kolben Schallloch),
  je Mode nur das Nettovolumen V_j und der Anteil Schallloch — 2 Unbekannte gegen 3 Mikrofone, überbestimmt, prüfbar
  über upper east (näher am Schallloch). Das gilt nur für die tiefen, kompakten Moden (ka < 1, f ≲ 400–500 Hz).

### 1.2 Parametertabelle

| Größe | Wert | Einheit | Quelle, Stelle | Marke | Gültig |
|---|---|---|---|---|---|
| Messabstand Fernfeld | 0,9 m vor dem Steg, auf Steghöhe, reflexionsarm, B&K 4144 | — | Wright gedr. S. 94 | [am Seitenbild geprüft] | — |
| Fernfeldbedingung bei ≈ 1 m | „only satisfies the far-field condition for frequencies of around 300 Hz or more“ | — | Wright gedr. S. 96 | [am Seitenbild geprüft] | — |
| BR2, Antrieb Steg tiefe E, **Schallloch zu**: T(1,1) | 165 Hz, 0,062 kg, Q 36, **A = 0,059 m²** | | Wright Tab. 5.4, gedr. S. 98 | [am Seitenbild geprüft] | bis ≈ 750 Hz (Abb. 5.5) |
| T(2,1) / B(1,2) / T(1,2) / T(3,1) | 204 / 293 / 391 / 432 Hz; 0,201 / 1,920 / 0,612 / 0,726 kg; Q 66 / 50 / 60 / 51; **A = 0,010 / 0,186 / −0,133 / −0,189 m²** | | ebd. | [am Seitenbild geprüft] | |
| T(4,1)? / T(2,2) / ??? / T(3,2) | 497 / 607 / 639 / 717 Hz; 1,001 / 0,209 / 0,500 / 0,300 kg; Q 21 / 25 / 11 / 33; A = −0,290 / −0,017 / −0,044 / −0,024 m² | | ebd. | [am Seitenbild geprüft] | |
| Helmholtz (ungekoppelt) | 133 Hz, Q 30 (Christensen 1982: 122–135 Hz an fünf Gitarren) | | Wright gedr. S. 138 | [am Seitenbild geprüft] | |
| Schalllochfläche, Volumen | A_h = 5,7·10⁻³ m², V = 14·10⁻³ m³ | | Wright gedr. S. 138 | [am Seitenbild geprüft] | |
| g34: Schallloch, Maße | WSH 87 mm (→ 5,9·10⁻³ m²), WT/WW/WB 288/239/377 mm, Zargen 104/108/108 mm, L 493 mm, M 662 mm | mm | Mores List S. 1–2 | [am Seitenbild geprüft]; Spaltenbedeutung (WSH = Schalllochdurchmesser usw.) [eigene Einschätzung] | |
| A/m der ersten Deckenmode, fünf Gitarren | 7 … 14 cm²/g (= 0,7 … 1,4 m²/kg) | | Wright gedr. S. 151 nach Christensen | [wörtlich] (Textebene, nicht am Bild) | |
| Kolbenradius | a = √(A/π): 0,137 m (Wright T(1,1)); 0,109 m (Hess); Schallloch 0,043 m | m | [eigene Rechnung] | | ka < 1: f < 400–500 Hz |
| Abstand Schallloch–Steg | 0,17 m | m | — | [Annahme] (aus Mensur und Lage des 19. Bunds geschätzt) | |
| Fraunhofer-Abstand 2D²/λ für D = 0,49 m | = 1 m bei 700 Hz | — | [eigene Rechnung] | | darüber ist 1 m Übergangsfeld |

### 1.3 Zahlenbeispiele

**r1 — Körperdatei g34 gegen Wrights Fernfeld-Monopol** (`r1_koerper.py`):

```python
"""Lücke 1: tiefe Moden der Körperdatei g34 (nur lesen) und Gegenrechnung gegen Wright 1996 Tab. 5.4 (0,9 m).

p/F je Mode = phi * G * (-w^2)/(wn^2 - w^2 + i eta wn w); an der Resonanz |p/F| = |phi G| / eta.
Monopol im Fernfeld (Wright Gl. 4.17 mit U = A v, v/F = i w phi^2 / D):  G_fern = rho * A_eff * phi_drive / (4 pi r).
"""
import json
import math
import pathlib

P = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\classical_guitar.lvinstbody")
k = json.loads(P.read_text(encoding="utf-8"))
print("Schluessel:", list(k.keys()))
print("Hoerorte:", k.get("listeningPoints"), " Stegpunkte:", k.get("bridgePoints"))
print("statistical:", {a: b for a, b in k["statistical"].items() if a != "materialShares"})
print()
print(" f/Hz    eta     phi(S6)  phi(S1)   |G_left| |G_right|  |p/F| left/right an Resonanz (Saite 6) dB re 1 Pa/N")
for m in k["modes"][:14]:
    f, eta = m["f"], m["eta"]
    phi6, phi1 = m["bridge"][0], m["bridge"][-1]
    g = [abs(complex(*x)) for x in m["radiation"]]
    pf = [abs(phi6) * x / eta for x in g]
    print(f"{f:7.1f} {eta:7.4f} {phi6:8.3f} {phi1:8.3f}   {g[0]:8.4f} {g[1]:8.4f}   "
          + " / ".join(f"{20*math.log10(v):6.1f}" for v in pf))

rho = 1.2
print()
print("Wright 1996 Tab. 5.4 (BR2, Schallloch zu, Antrieb Steg tiefe E, Mikrofon 0,9 m vor dem Steg):")
for name, f, m_eff, Q, A in [("T(1,1)", 165, 0.062, 36, 0.059), ("T(2,1)", 204, 0.201, 66, 0.010),
                             ("B(1,2)", 293, 1.920, 50, 0.186), ("T(1,2)", 391, 0.612, 60, -0.133),
                             ("T(3,1)", 432, 0.726, 51, -0.189)]:
    phi = 1 / math.sqrt(m_eff)
    for r in (0.9, 1.0):
        G = rho * A * phi / (4 * math.pi * r)
        pf = abs(rho * A * Q / (4 * math.pi * r * m_eff))
        print(f"  {name} {f} Hz r={r} m: G_fern = {G:+.4f} sqrt(kg)/m^2, |p/F| Resonanz = {pf:.3f} Pa/N = {20*math.log10(pf):.1f} dB re 1 Pa/N")
```

```
Hoerorte: ['left', 'right']  Stegpunkte: [40, 45, 50, 55, 59, 64]
 f/Hz    eta     phi(S6)  phi(S1)   |G_left| |G_right|  |p/F| left/right an Resonanz (Saite 6) dB re 1 Pa/N
   79.0  0.0508    0.430    0.608     0.0283   0.0214    -12.4 /  -14.8
   98.0  0.0431    1.517    1.447     0.0945   0.1108     10.4 /   11.8
  200.1  0.0788    3.960    3.148     0.1961   0.1674     19.9 /   18.5
  225.4  0.0829    1.047    0.952     0.0632   0.0553     -2.0 /   -3.1
  260.1  0.0348    0.540   -3.283     0.0370   0.1043     -4.8 /    4.2
  ... (bis 606 Hz, siehe Skriptausgabe)
Wright 1996 Tab. 5.4 (BR2, Schallloch zu, Antrieb Steg tiefe E, Mikrofon 0,9 m vor dem Steg):
  T(1,1) 165 Hz r=0.9 m: G_fern = +0.0251 sqrt(kg)/m^2, |p/F| Resonanz = 3.635 Pa/N = 11.2 dB re 1 Pa/N
  T(1,1) 165 Hz r=1.0 m: G_fern = +0.0226 sqrt(kg)/m^2, |p/F| Resonanz = 3.271 Pa/N = 10.3 dB re 1 Pa/N
  T(2,1) 204 Hz r=1.0 m: G_fern = +0.0021 ...   B(1,2) 293 Hz r=1.0 m: G_fern = +0.0128 ...
  T(1,2) 391 Hz r=1.0 m: G_fern = -0.0162 ...   T(3,1) 432 Hz r=1.0 m: G_fern = -0.0212 ...
```

Lesart [eigene Rechnung]: g34 hat an Saite 6 dieselbe wirksame Masse wie Wrights T(1,1) (φ = 3,96 → 0,064 kg gegen
0,062 kg). Wäre auch die Fläche gleich, läge G in 1 m bei 0,0226 √kg/m² gegen 0,196 / 0,167 im Nahfeld: **−18,8 /
−17,4 dB**. Wrights Wert gilt für zugeklebtes Schallloch; offen addiert das Loch bei T1 Volumen (r2: Anteil 0,83, in
Phase), dann −13…−14 dB. Fremde Gitarre — das ist eine Größenordnung, kein Maß.

**r2 — Zwei-Oszillator-Modell in 1 m und Kolben 10 cm gegen 1 m je Terz** (`r2_nah_fern.py`):

```python
import math
import numpy as np
rho, c = 1.2, 343.0
# (A) Wright Gl. 4.9-4.17, BR2 Tab. 5.4 + S. 138
mt, ft, Qt, At = 0.062, 165.0, 36.0, 0.059
fh, Qh, Ah, V = 133.0, 30.0, 5.7e-3, 14e-3
mu = rho * c * c / V
mh = mu * Ah * Ah / (2 * math.pi * fh) ** 2
f = np.linspace(60, 400, 34001); w = 2 * np.pi * f
Dt = mt * ((2 * np.pi * ft) ** 2 - w ** 2 + 1j * w * (2 * np.pi * ft / Qt))
Dh = mh * ((2 * np.pi * fh) ** 2 - w ** 2 + 1j * w * (2 * np.pi * fh / Qh))
al = mu * Ah * At; N = Dh * Dt - al ** 2
xt, xh = Dh / N, -al / N
Y = 1j * w * xt
p1 = 1j * w * rho * 1j * w * (At * xt + Ah * xh) / (4 * np.pi * 1.0)
# ... Spitzen von |Y|, Terzmittel von |p1|^2 (vollständig in r2_nah_fern.py)
# (B) Rayleigh-Integral über eine Kreisscheibe (Schallwand), Fernfeld Vollraum U/(4 pi r) und Halbraum U/(2 pi r)
def rayleigh(a, x0, z0, k, n=500):
    r = (np.arange(n) + 0.5) * a / n
    ph = (np.arange(2 * n) + 0.5) * np.pi / n
    R, P = np.meshgrid(r, ph, indexing="ij")
    X, Yy = R * np.cos(P), R * np.sin(P)
    D = np.sqrt((X - x0) ** 2 + Yy ** 2 + z0 ** 2)
    return np.sum(np.exp(-1j * k * D) / D * R * (a / n) * (np.pi / n)) / (2 * np.pi)
```

```
(A) mu = 1.008e+07 Pa/m^3, m_h = 0.469 g, l_eff = 6.9 cm
    Spitze der Admittanz   76.3 Hz: |Y| = -13.8 dB re 1 m/s/N, |p/F| 1 m = -11.3 dB re 1 Pa/N (nur Decke -5.2), Volumenanteil Loch/Decke = 1.49, Phase +178 Grad
    Spitze der Admittanz  197.7 Hz: |Y| =  -8.8 dB re 1 m/s/N, |p/F| 1 m =  13.4 dB re 1 Pa/N (nur Decke  8.1), Volumenanteil Loch/Decke = 0.83, Phase +2 Grad
    Terz 80/100/125/160/200/250/315 Hz: p/F in 1 m = -15.9 / -22.2 / -19.9 / -11.1 / 5.0 / -10.6 / -15.8 dB re 1 Pa/N
(B) Kolben: a = 0.109 m, 50 Hz: Nah auf Achse z->0: -31.3 dB (Hess a/(4r) = -31.3 dB)
   Terz Hz |  ka  | Achse 10 cm: Voll  Halb | seitl. 10 cm: Voll  Halb
       80 | 0.16 |   -24.2  -18.1    |   -22.5  -16.5
      200 | 0.40 |   -24.1  -18.1    |   -22.5  -16.5
      500 | 1.00 |   -24.1  -18.1    |   -22.1  -16.1
     1000 | 2.00 |   -23.9  -17.9    |   -20.7  -14.7
     2000 | 3.99 |   -23.0  -17.0    |   -14.3   -8.3
     5000 | 9.98 |   -14.9   -8.9    |    -5.4   +0.6
  a = 0.137 m: Achse 10 cm Voll -23.4 dB (80-500 Hz), seitlich -22.2 ... -21.6 dB; 5 kHz +6.8 / -6.3 dB
```

Die volle Terztabelle 80 Hz – 5 kHz für beide Radien steht in der Skriptausgabe; über ka ≈ 1 ist ein gleichförmiger
Kolben **kein** Modell einer Gitarrendecke (die Decke zerfällt in Moden mit kleinem Nettovolumen; das Nahfeld sieht die
örtliche Schnelle, das Fernfeld das Nettovolumen) — die Zahlen über 500 Hz sind nur die Kolbenrechnung.

Gegenprobe: A0 dieses Parametersatzes liegt bei 76 Hz (g34: 98 Hz) — Wrights BR2-Decke mit Christensens Helmholtzwerten
ist nicht g34; T1 bei 198 Hz passt.

**r3 — Zwei Kolben (Decke am Steg, Schallloch 0,17 m davor) an den Mores-Orten** (`r3_zwei_kolben.py`):

```python
# Werte wie r2 (A); Decke: Kreis Fläche A_t, Mitte am Steg; Loch: Kreis A_h, Mitte 0,17 m Richtung Hals (Annahme)
ORTE = {"bass": (0.0, -0.10, 0.10), "treble": (0.0, 0.10, 0.10), "upper east (Annahme)": (0.20, 0.10, 0.10)}
for f in (76.3, 100.0, 125.0, 160.0, 197.7, 250.0, 315.0):
    w = 2 * math.pi * f; k = w / c
    Dt = mt * ((2 * math.pi * ft) ** 2 - w * w + 1j * w * 2 * math.pi * ft / Qt)
    Dh = mh * ((2 * math.pi * fh) ** 2 - w * w + 1j * w * 2 * math.pi * fh / Qh)
    al = mu * Ah * At; N = Dh * Dt - al * al
    ut, uh = 1j * w * Dh / N, 1j * w * (-al) / N
    fern = 1j * w * rho * (At * ut + Ah * uh) / (4 * math.pi)            # 1 m, Vollraum
    for o, pkt in ORTE.items():
        nah = 1j * w * rho * (ut * kolben(at, (0, 0), pkt, k) + uh * kolben(ah, (LOCH, 0), pkt, k))  # Rayleigh
```

```
a_t = 0.137 m, a_h = 0.043 m, Loch 0.17 m vom Steg (Annahme)
 f/Hz  | |p/F| 1 m | bass nah  fern-nah | treble nah  fern-nah | upper east (Annahme) nah  fern-nah
  76.3 |  -11.3 dB |   -0.6  -10.8 |   -0.6  -10.8 |   16.6  -28.0 |
 100.0 |  -22.5 dB |   -6.3  -16.3 |   -6.3  -16.3 |    2.6  -25.1 |
 125.0 |  -20.3 dB |   -1.6  -18.8 |   -1.6  -18.8 |    2.9  -23.2 |
 160.0 |  -13.0 dB |    7.1  -20.2 |    7.1  -20.2 |    8.5  -21.5 |
 197.7 |   13.4 dB |   34.2  -20.9 |   34.2  -20.9 |   33.8  -20.4 |
 250.0 |  -11.5 dB |    9.8  -21.3 |    9.8  -21.3 |    8.0  -19.4 |
 315.0 |  -16.0 dB |    5.5  -21.5 |    5.5  -21.5 |    2.7  -18.7 |
```

### 1.4 Ergebnis Form (a) — „je Terzband ein Maß“ (gerechnet, **nicht gemessen**)

| Terzband | Kolben a = 0,109–0,137 m, bass/treble-Ort (10 cm seitlich, 10 cm hoch), Vollraum | Zwei Kolben (Decke + Loch) | Marke |
|---|---|---|---|
| 80–125 Hz | −22,2 … −22,5 dB | −11 dB an A0 (76 Hz), −16 … −19 dB daneben | [hergeleitet] |
| 160–315 Hz | −22,0 … −22,5 dB | −20 … −21,5 dB | [hergeleitet] |
| 400–1000 Hz | −20,6 … −22,3 dB | — (ka > 1) | [hergeleitet], nur Kolben |
| 1,25–5 kHz | −5 … −20 dB, stark vom Ort abhängig | — | **ungültig** (Kolben kein Deckenmodell) |
| vorn auf der Achse statt seitlich | −23,4 … −24,2 dB (80–630 Hz) | | [hergeleitet] |
| seitlich / hinten in 1 m | **keine Quelle gelesen**; Wright: Dipolmoden „radiate strongly to the sides … very poorly to the front“ (gedr. S. 67) | | [am Seitenbild geprüft] (qualitativ) |

Eine **gemessene** Tabelle (Ort, Winkel, Raum, Anregung, Normierung) gibt es in den lesbaren Quellen nicht; die
einzige Messung p/F in ≈ 1 m an einer klassischen Gitarre ist Wright Abb. 5.5 (0,9 m, vorn, BR2, Schallloch zu) — ohne
Nahfeld an derselben Gitarre.

### 1.5 Fehlerangaben

| Vergleich | Fehler | Marke |
|---|---|---|
| Wright Abb. 5.5: Monopolsumme (gefittete A) gegen Messung in 0,9 m, 150–750 Hz | Spitzen meist ±1–3 dB, Senken bis ≈ 5 dB (560 Hz: Messung ≈ 33 dB, Fit ≈ 38 dB) — **ein Fit, keine Vorhersage** | [aus Abbildung abgelesen] |
| Zwei-Kolben-Modell (Wright-Werte) gegen g34-Nahfeld an T1 | Modell 34,2 dB (Q 36), g34 18,5 / 19,9 dB (Q 13); nach Q-Ausgleich (−8,8 dB) Modell ≈ 6 dB zu hoch | [eigene Rechnung]; fremde Gitarre |
| Kolben 10 cm → 1 m gegen Hess-Grenzfall | 0,0 dB (z → 0) | [eigene Rechnung] |
| Ortsabhängigkeit an A0 (bass/treble gegen upper east) | 17 dB Unterschied im Verhältnis (−10,8 gegen −28,0 dB) | [hergeleitet]; Ort upper east [Annahme] |

### 1.6 Gültigkeitsbereich

- **ka < 1** (a = 0,11–0,14 m): bis ≈ 400–500 Hz trägt ein Kolben je Mode; darüber nicht [hergeleitet; Hess: „bis etwa
  500 Hz“ für a = 10,9 cm, Stufe 1].
- **Fernfeld in 1 m:** kr ≫ 1 erst ab ≈ 300 Hz (Wright gedr. S. 96, [am Seitenbild geprüft]); geometrisch (2D²/λ) ist
  1 m bei einem 0,49 m langen Korpus nur bis ≈ 0,7 kHz Fernfeld [eigene Rechnung] — darüber ist „1 m“ selbst Übergangsfeld,
  ein Hörort in 1 m also abstandsabhängig.
- **Wenige Quellen** beschreiben das Nahfeld in 10 cm grundsätzlich nicht (Mikrofone innerhalb der Quellkugel, §1.1).

### 1.7 Annahmen, die bleiben

Ort des Schalllochs (0,17 m), Kreiskolben statt Schwingform, Schallwand im Nahfeld / Vollraum im Fernfeld, Wrights
BR2-Werte (Schallloch zu) für g34, kein Boden (Wright §4.4 hätte den Bodenkolben, nicht gerechnet), keine Richtwirkung.

---

## 2. Die zweite Stegrichtung

### 2.1 Gleichungen

| Nr. | Gleichung | Quelle, Stelle | Marke |
|---|---|---|---|
| G2.1 | Y(jω) = Σ_r Φ_rᵀΦ_r · jω / [m_r(ω_r² − ω² + 2jσ_rω_rω)], Φ_rᵀΦ_r Rang 1, reell, positiv semidefinit | Bank & Karjalainen 2010 Gl. (4), S. 2 | [am Seitenbild geprüft] |
| G2.2 | Mit Φ_r = φ_r (cos θ_r, sin θ_r) bzw. im Prüfstand φ_r(1, tan θ_r): Y_zz = Σ φ_r² c_r(ω), Y_zy = Σ tan θ_r φ_r² c_r(ω), Y_yy = Σ tan² θ_r φ_r² c_r(ω), c_r = iω/(ω_r² − ω² + iη_rω_rω) | aus G2.1; Woodhouse 2012 PDF-S. 3 („an angle … so that both polarisations of string motion can be included“, Stufe 1) | [hergeleitet] |
| G2.3 | Volle 2×2-Matrix je Pol: Y(z) = Σ_r Y_r H_r(z), Y_r symmetrisch, positiv semidefinit — „allowing maximum K modal shapes for each pole-pair“ | Bank 2010 Gl. (5a, 5b), Abschn. 3, S. 2 | [am Seitenbild geprüft] |
| G2.4 | Saite 0 ≤ x ≤ L, Steg bei 0, Sattel starr; w = **a** sin(k(L−x)); Kraft auf den Steg **F** = T w′(0); Stegschnelle iω**w**(0) = **Y F** ⇒ **[tan(kL) I − i Z₀ Y(ω)] a = 0**, Z₀ = √(Tm) | — | [hergeleitet] |
| G2.5 | Schwache Kopplung: kL = nπ + iZ₀λ ⇒ **ω_± = ω_n (1 + i Z₀ λ_± / (nπ))**, λ_± Eigenwerte von Y(ω_n); Eigenvektoren = Schwingrichtungen der beiden Teiltöne | aus G2.4 | [hergeleitet] |
| G2.6 | **η_b,± = 2 Z₀ Re(λ_±) / (nπ)**; Abklingrate (Amplitude) σ_± = η_±ω_n/2; Aufspaltung **Δf = f_n Z₀ (Im λ_− − Im λ_+) / (nπ)** (Schwebung mit Δf) | aus G2.5 | [hergeleitet]; 1-D-Fall = Woodhouse 2021 Gl. (1) η_b ≈ 2Re(Y)/(πnY₀), S. 4376 (PDF-S. 3) [am Seitenbild geprüft] |
| G2.7 | Luft: η_air ≈ (ρ_a/ρ)(2√2M + 1)/M², M = (d/4)√(2πf_n/η_a); Biegung η_bend ≈ Eπ²d²n²η_E / (64ρL⁴f₁² + Eπ²d²n²) | Woodhouse 2021 Gl. (2)–(4), S. 4376 | [am Seitenbild geprüft] |
| G2.8 | Bedingung für deutliches Doppelabklingen: η_b ≫ η_air (notwendig, nicht hinreichend) | Woodhouse 2021 Gl. (5), S. 4377 | [am Seitenbild geprüft] |
| G2.9 | Gültigkeit: Y darf sich über die Verschiebung kaum ändern; nahe einer Körpermode (geringe Modenüberlappung) **Kurvenabstoßung** (veering, mit Schwebung) oder Anziehung (anti-veering, ohne Schwebung) — dann Drei-Freiheitsgrade-Modell Gl. (6)–(10) | Woodhouse 2021 §III, S. 4380–4382 | [am Seitenbild geprüft] |
| G2.10 | T60 = 6,91 · 2 / (η ω_n) | — | [hergeleitet] |

Woodhouse 2021 zum Ergebnis an Gitarren: „In the tested guitar, by contrast, double decays are likely to be confined to
a few notes near strong body resonances“ (Abstract, S. 4375) [wörtlich]; Messung nur senkrecht zur Decke („in the
direction normal to the soundboard“, S. 4377) [am Seitenbild geprüft], Gitarre = Stahlsaiten (Fylde „Falstaff“).
„a real instrument bridge will always have non-vanishing admittance in the direction parallel to the soundboard. To
characterise this effect, a 2 × 2 admittance matrix must be measured at the bridge (see, for example, Woodhouse,
2004b)“ (S. 4377) [am Seitenbild geprüft].

### 2.2 Parametertabelle je Band

LumiViz-Achsen: z senkrecht, y parallel zur Decke quer zur Saite. Bank-Werte aus dem Stufe-1-Bericht (Abb. 4
abgelesen, ±3 dB, **Stahlsaiten-Gitarre**, Achsen von Bank vertauscht und dort schon umbenannt).

| Band | \|Yyy/Yzz\| | \|Yzy/Yzz\| | √⟨tan²θ⟩ aus Yzy | ⟨tan⁴θ⟩^¼ aus Yyy | Kurtosis ⟨tan⁴⟩/⟨tan²⟩² | Quelle / Marke |
|---|---|---|---|---|---|---|
| ≈ 100 Hz | −15 dB | −20 dB | 0,10 | 0,42 | 316 | Bank [aus Abbildung abgelesen, Stufe 1]; Rest [eigene Rechnung] |
| Hauptspitze ≈ 240 Hz | −30 dB | −20 dB | 0,10 | 0,18 | 10 | ebd. |
| 1–3 kHz | −12 … −15 dB | −15 … −20 dB | 0,13 | 0,46 | 141 | ebd. |
| 3–10 kHz | −12 … −15 dB | −12 … −15 dB | 0,21 | 0,46 | 22 | ebd. |
| LumiViz-Prüfstand heute (Annahme nach Scherrer A.11) | 100 Hz −61, 500 Hz −41, 1 kHz −25, ≥ 1,5 kHz −16 dB | −30 / −20 / −13 / −8 dB | \|tan θ\| = clip(0,03 (f/200)^1,27; 0,03; 0,4) | | 1 (ein Betrag je Frequenz) | `richtung_tan`, Docstring [wörtlich] |
| Scherrer 2013 Abb. A.11 (laut Prüfstand-Docstring) | 30 dB unter den tiefen Spitzen; 5–10 dB unter Yzz ab ≈ 1,5 kHz | — | — | — | — | schon belegt (nicht wiederholt) |
| Meyer 1983 (klassisch) | gemittelte Schallkurve bei senkrechter und paralleler Anregung „scarcely differed“ | — | — | — | — | Stufe 1 [wörtlich] |

Lesart [eigene Rechnung]: Bei **einer** Richtung je Mode und zufälligen Vorzeichen ist \|Yzy/Yzz\|² ≈ ⟨tan²⟩ und
\|Yyy/Yzz\|² ≈ ⟨tan⁴⟩. Die Bank-Zahlen verlangen dann eine Kurtosis von 10–300 — also wenige Moden fast rein
parallel und viele fast rein senkrecht; an einer einzelnen Mode wäre Yyy/Yzz = (Yzy/Yzz)² = −40 dB statt der
abgelesenen −15 … −30 dB. Das passt zu Banks eigenem Weg (G2.3: volle Matrix je Pol). Die heutige Prüfstand-Annahme
(ein Betrag je Frequenz, Kurtosis 1) gibt **Yyy 25–45 dB zu tief unter 1 kHz** gegen Bank und Scherrer.

### 2.3 Zahlenbeispiel — Doppelabklingen an g34 (`r4_zwei_richtungen.py`)

```python
"""Lücke 2: zwei Polarisationen einer Saite am 2x2-Steg — Störungsrechnung gegen exakte Wurzel.
Körper: g34 (classical_guitar.lvinstbody, nur gelesen), tan(theta_n) wie im Prüfstand (Samen 7) — ANNAHME.
Saiten: Woodhouse u. a. 2012 Tab. I wie im Prüfstand (GITARRE)."""
import json, math, pathlib
import numpy as np
k = json.loads(pathlib.Path(r"C:\...\LumiViz\asset\loop\instrument_bodies\classical_guitar.lvinstbody").read_text(encoding="utf-8"))
fn = np.array([m["f"] for m in k["modes"]]); eta = np.array([m["eta"] for m in k["modes"]])
phi = np.array([m["bridge"] for m in k["modes"]]); wn = 2 * np.pi * fn
# richtung_tan(fn) wie im Prüfstand (SplitMix64, Samen 7) -> TAN
def Ymat(w, s, tan=TAN):
    c = 1j * w / (wn ** 2 - w ** 2 + 1j * eta * wn * w) * phi[:, s] ** 2
    return np.array([[np.sum(c), np.sum(c * tan)], [np.sum(c * tan), np.sum(c * tan * tan)]])
GITARRE = [(71.6, 6.24e-3, 1.2), (73.9, 3.61e-3, 0.9), (71.2, 1.95e-3, 1.2), (58.3, 0.90e-3, 1.7),
           (53.4, 0.52e-3, 1.2), (70.3, 0.38e-3, 1.5)]       # T, m, eta_A
L = 0.650
def exakt(s, n, w0):            # Newton auf det[tan(w L / c) I - i Z0 Y(w)] = 0, w komplex
    ...
for s in range(6):
    for n in (1, 2):
        w_n = n * math.pi * cs / L
        lam = np.linalg.eigvals(Ymat(w_n, s))           # sortiert nach Re
        wpm = w_n * (1 + 1j * Z0 * lam / (n * math.pi))  # G2.5
        eb = 2 * Z0 * lam.real / (n * math.pi)           # G2.6
        e_air = etaA / w_n                               # Valette-Term (Woodhouse 2012 Gl. 4), ohne eta_F, Biegung
        t60 = [13.8 / ((e + e_air) * w_n) for e in eb]
        df = (wpm[0].real - wpm[1].real) / (2 * math.pi)
```

```
Saite n | f_n Hz | |Yzz| dB | |Yzy/Yzz| dB | |Yyy/Yzz| dB | eta_b+ eta_b- (Stoerung) | exakt | eta_air | T60+ T60- s | df Hz
6 E2 1 |   82.4 |  -36.5 |  -43.5 |  -51.1 | 1.49e-03 1.12e-06 | 1.53e-03 1.12e-06 | 2.32e-03 |  7.00 11.49 | -0.254
6 E2 2 |  164.8 |  -30.3 |  -29.4 |  -51.1 | 1.39e-03 2.41e-07 | 1.33e-03 2.41e-07 | 1.16e-03 |  5.23 11.50 | -0.522
5 A2 1 |  110.1 |  -46.1 |  -14.8 |  -35.4 | 1.31e-03 3.84e-05 | 1.27e-03 3.84e-05 | 1.30e-03 |  7.64 14.89 | +0.056
5 A2 2 |  220.1 |  -22.8 |  -29.9 |  -53.3 | 9.25e-03 3.82e-07 | 9.30e-03 3.82e-07 | 6.51e-04 |  1.01 15.32 | +0.827
4 D3 1 |  147.0 |  -31.1 |  -29.4 |  -43.0 | 9.01e-04 5.20e-07 | 8.77e-04 5.20e-07 | 1.30e-03 |  6.79 11.50 | -0.478
4 D3 2 |  294.0 |  -33.4 |  -25.0 |  -35.9 | 6.06e-04 2.93e-06 | 6.06e-04 2.93e-06 | 6.50e-04 |  5.95 11.45 | +0.370
3 G3 1 |  195.8 |  -15.5 |  -30.7 |  -53.7 | 2.02e-02 5.11e-07 | 1.66e-02 5.10e-07 | 1.38e-03 |  0.52  8.11 | -1.360
3 G3 2 |  391.6 |  -29.0 |  -28.7 |  -39.7 | 6.52e-04 1.48e-06 | 6.17e-04 1.48e-06 | 6.91e-04 |  4.18  8.10 | +0.497
2 B3 1 |  246.5 |  -32.9 |  -18.2 |  -39.9 | 2.32e-03 4.03e-05 | 2.24e-03 4.03e-05 | 7.75e-04 |  2.88 10.93 | +0.052
2 B3 2 |  493.0 |  -45.7 |  -29.1 |  -21.0 | 1.22e-04 1.65e-06 | 1.22e-04 1.65e-06 | 3.87e-04 |  8.75 11.45 | +0.066
1 E4 1 |  330.9 |  -33.8 |  -37.8 |  -45.2 | 2.17e-04 4.63e-07 | 2.14e-04 4.63e-07 | 7.22e-04 |  7.07  9.19 | +0.353
1 E4 2 |  661.7 |  -44.2 |  -14.5 |  -24.5 | 2.93e-04 1.29e-05 | 2.93e-04 1.29e-05 | 3.61e-04 |  5.08  8.88 | +0.039
Kontrolle 1-D (nur Yzz) gegen Woodhouse 2021 Gl. (1), Saite 6, n = 1:
  Re(Yzz) = 3.504e-03 m/s/N, Y0 = 1.496 -> eta_b = 1.491e-03
```

Lesart [eigene Rechnung]:
- **Die Störungsformel G2.5/G2.6 trifft die exakte Wurzel** auf 0–4 %; nur G3 an T1 (196 gegen 200 Hz, \|Yzz\| −15,5 dB)
  weicht 18 % ab — dort gilt G2.9 (veering), nicht die schwache Kopplung.
- Mit der heutigen Richtungsannahme hat die **parallele** Polarisation praktisch keinen Stegverlust (η_b 10⁻⁷…4·10⁻⁵ ≪
  η_air) — sie klingt mit der Saitendämpfung allein (T60 8–15 s). Die senkrechte erreicht η_b ≳ η_air nur nahe starken
  Körpermoden (A2 n = 2 an 220 Hz, G3 n = 1, B3 n = 1, E2 n = 2), sonst ist sie mit η_air vergleichbar. Das ist das Bild
  von Woodhouse 2021 („confined to a few notes near strong body resonances“). Mit Bank-artigem Yyy (−15 dB bei 100 Hz)
  stiege η_b,y etwa um den Faktor \|Yyy/Yzz\| (z. B. E2: von 10⁻⁶ auf ≈ 2,7·10⁻⁴) — immer noch unter η_air; das
  Doppelabklingen bliebe, die zweite Abklingrate stiege um ≈ 10 %. [eigene Rechnung, Überschlag]
- **Schwebung** Δf: 0,04–1,4 Hz (Periode 0,7–25 s); sie stammt aus Im λ, also aus dem Blindanteil von Y.
- Die Eigenrichtungen weichen bei diesen Annahmen höchstens wenige Grad von z/y ab (\|Yzy\| klein gegen \|Yzz − Yyy\|);
  ein Zupf unter dem Winkel ψ verteilt die Energie also nahezu als cos²ψ / sin²ψ auf schnell und langsam [hergeleitet].

### 2.4 Annahmen, die bleiben

Die Richtungen θ_r an einer **klassischen** Gitarre sind nicht gemessen (Stufe 1: nur Bank, Stahlsaiten; Woodhouse 2004
nicht gelesen); Saite starr am Sattel; Saitenverluste nur der Luftterm η_A/ω; schwache Kopplung außerhalb der
starken Körpermoden.

---

## 3. Zupf

### 3.1 Gleichungen

| Nr. | Gleichung | Quelle, Stelle | Marke |
|---|---|---|---|
| G3.1 | Zwei Phasen: Vorbereitung (Saite wird mitgenommen, „horizontally and vertically“), dann Ablösung („the string slides on the fingertip or the nail and then is left to oscillate freely. The role of the nail is to provide a ramp“) | Scherrer 2013, gedr. S. 48–49 (nach Pavlidou 1997, Taylor 1978) | [wörtlich] (Textebene) |
| G3.2 | Idealisierter Zupf: „a step function of force as the finger or plectrum loses contact with the string … released from rest“; Winkel des Zupfs mischt die zwei Polarisationen | Woodhouse u. a. 2012, PDF-S. 4 | [wörtlich] (Textebene) |
| G3.3 | Kraftstufe aus der Auslenkung h am Ort x₀: **F₀ = T h L / (x₀(L − x₀))** (statisches Gleichgewicht der geknickten Saite) | Prüfstand-Docstring; Kurve Scherrer Abb. 3.8, gedr. S. 58 | [hergeleitet]; Abb. am Bild: **die Kurve passt zu h = 1 mm, nicht zu der beschrifteten 1 cm** (Mitte 0,43 N statt 4,31 N; x₀ = 1 cm: 7,1 N statt 71 N) [am Seitenbild geprüft + eigene Rechnung] |
| G3.4 | Breite des Fingers: Faktor exp(−ω²/K²), K = v/h, v = √(T/m), h = 7,5 mm „to obtain a reasonably realistic sound“ | Woodhouse u. a. 2012, PDF-S. 4 | [wörtlich] (Textebene, nicht am Bild) |
| G3.5 | Breite w als räumliches Filter, Grenze f_c = c/w; mit w = 2 cm: I ≃ 20 kHz … VI ≃ 5 kHz | Scherrer Tab. 3.1, gedr. S. 61 | [wörtlich] (Textebene); c/w nachgerechnet 21,5 / 5,4 kHz [eigene Rechnung] |
| G3.6 | Dauer der Anregung: Teiltöne × sin(πf_k t_ex)/(πf_k t_ex); „Assuming a typical duration t_ex ≃ 0.02 s … the cut-off frequency is around 50 Hz“ | Scherrer gedr. S. 61, nach Chaigne & Kergomard, Pavlidou | [wörtlich]; **wörtlich genommen würde das alle Teiltöne über 50 Hz tilgen** — t_ex meint dort die ganze Mitnahme, nicht die Ablösung [eigene Einschätzung]; nicht als Filter übernehmen |
| G3.7 | Ablösewinkel (AOR) trennt Wechsel- und Anschlagschlag (free/rest stroke), gemessene Bahnen Pavlidou Abb. 6.18 | Scherrer Abb. 3.12, gedr. S. 63 | [wörtlich]; Werte nicht abgelesen (AOR schon belegt, A.11) |
| G3.8 | Ziel nach unten: „Most often, the string is pushed toward the top plate rather than away from it“ | Scherrer gedr. S. 49 | [wörtlich] |

### 3.2 Parametertabelle

| Größe | Wert | Quelle, Stelle | Marke | Gültig |
|---|---|---|---|---|
| Auslenkung „soft“ / „strong“ bei x₀ = 16 cm | 0,8 mm / 4 mm (Steigung 5·10⁻³ / 25·10⁻³) | Scherrer Tab. 2.1, gedr. S. 25, nach Pavlidou 1997 Abb. 6.21 | [wörtlich], Stufe 1 | nur zwei Stufen, Saite nicht genannt |
| daraus F₀ (Saite 1 / Saite 6, T = 70,3 / 71,6 N) | 0,47 N (soft) · 1,17 N (h = 2 mm, Prüfstand) · 2,33–2,37 N (strong) | G3.3 | [eigene Rechnung] | |
| Fingerbreite (Gauss) | h = 7,5 mm → −3 dB bei 5,4 kHz (Saite 1) / 1,3 kHz (Saite 6); −20 dB bei 13,9 / 3,5 kHz | G3.4 | [eigene Rechnung] | |
| Kontaktbreite Nagel | „about twice the size of a regular fingernail“ = 2 cm → Nagel ≈ 1 cm | Scherrer gedr. S. 61 | [wörtlich] | |
| Zupfort je Klangfarbe | ponticello 5 cm, „brassy“ 8 cm, „round“ 13 cm, tasto 20 cm (Experiment, Finger senkrecht bzw. 45°) | Traube 2004 PDF-S. 160/162, gedr. S. 140/142 | [wörtlich] | eine Spielerin/ein Spieler der Studie, offene Saiten |
| „normale“ Zupfstelle | 12 cm vom Steg | Traube PDF-S. 154 („12 cm from the bridge (normal plucking position)“) | [wörtlich] (Textebene) | |
| Spanne | 3–20 cm (1/3 … 1/10 der Saite); Sor ≈ 1/10 | Traube PDF-S. 45, 49 (Stufe 1) | [wörtlich] | |
| Fingerstaffel | Zeigefinger–Ringfinger 2–3 cm, Zeigefinger am weitesten vom Steg | Scherrer gedr. S. 58 | [am Seitenbild geprüft] | |
| Ausgleich bei gegriffenen Tönen | „Skilled performers can compensate … by moving the plucking finger“ (Scherrer gedr. S. 58) gegen Traube: Spieler gleichen meist **nicht** aus (PDF-S. 156) | | [am Seitenbild geprüft] / [wörtlich] | widersprüchlich |
| Anschlagwinkel tiefe Saiten | IV–VI „as perpendicularly as possible“, I–III „< 90°“ | Scherrer gedr. S. 61–62 nach Townsend 1996 | [wörtlich] | |

### 3.3 Zupfort je Bund — zwei Gesetze (keine Messung an Spielern gefunden)

Aus `r5_luecken_2_3_4.py` (Mensur 650 mm; (A) Hand bleibt bei 16 cm = Traube; (B) relatives R fest = Scherrer
„skilled performers“; Prüfstand heute min(16 cm, L/2)):

```
  Bund | Saitenlänge | (A) fest 16 cm: R | (B) R fest = 16/65: x_p cm | min(16, L/2)
      0 |  65.0 cm | R = 0.246 | x_p =  16.0 cm |  16.0 cm
      5 |  48.7 cm | R = 0.329 | x_p =  12.0 cm |  16.0 cm
      7 |  43.4 cm | R = 0.369 | x_p =  10.7 cm |  16.0 cm
     12 |  32.5 cm | R = 0.492 | x_p =   8.0 cm |  16.0 cm
     15 |  27.3 cm | R = 0.585 | x_p =   6.7 cm |  13.7 cm
     19 |  21.7 cm | R = 0.738 | x_p =   5.3 cm |  10.8 cm
```

Die Spieldaten tragen keine Wahl zwischen (A) und (B). Zu beachten: LumiViz rechnet mit 16 cm (Pavlidou-Messort), Traube
nennt 12 cm als „normal“; R = 0,246 (16 cm) gegen 0,185 (12 cm) verschiebt die Nullstellen des Kamms vom 4. auf den
5. Teilton [eigene Rechnung: 1/R].

### 3.4 Kraftverlauf (was belegt ist und was nicht)

Belegt ist nur die **Form**: quasistatisches Mitnehmen bis F₀ (G3.3), dann Ablösung als Stufe (G3.2), gefiltert durch
die Breite (G3.4/G3.5). **Nicht belegt** (in den lokalen Quellen nicht gefunden): Dauer der Ablösung beim Nagel,
Geschwindigkeit beim Ablösen je Dynamik, Kraft über mehr als zwei Stufen. Scherrer verweist für die Anfangsgeschwindigkeit
auf eine Arbeit von 2012 (Textebene Z. 11456–11458, dort „should not, in fact, be neglected“) — vermutlich Chadefaux
(Harfe), nicht geprüft.

### 3.5 Annahmen, die bleiben

Zwischenstufen pp … ff (nur soft/strong belegt), welche Saite Pavlidou zupfte, Zupfort je Bund, Ablösedauer.

---

## 4. Statistischer Teil über 5 kHz

### 4.1 Gleichungen

| Nr. | Gleichung | Quelle, Stelle | Marke |
|---|---|---|---|
| G4.1 | n(ω) = pβ + q√(β/ω), p = S/(4π), q nach Rand (frei: L_x + L_y), β = √(ρh/D) | Elie u. a. 2012 Gl. (23), Tab. III, S. 9 | [am Seitenbild geprüft]; die Werte in Abb. 7b (≈ 0,019) sind **je Hz** = 2π·n(ω): mit Abb. 9 (M ≈ 1,4 kg, D ≈ 75 Nm, S = 0,09 m²) folgt n(f) = Sβ/2 = 0,0205/Hz [eigene Rechnung] |
| G4.2 | Fitregel in Gl. (24) lautet „n₃₀ − pβ + q√β g₃₀“ — Vorzeichen des letzten Terms vermutlich Druckfehler (Gl. 23 hat +) | Elie S. 9 | [am Seitenbild geprüft] / [eigene Einschätzung] |
| G4.3 | D_Eq = ρh/β², ρh = M_Eq/S | Elie Gl. (27), S. 10 | [am Seitenbild geprüft] |
| G4.4 | Kennadmittanz der unendlichen Platte G_C∞ = 1/(8√(ρhD)); Bezug G_Ref = 0,094 m/s/N (ρ = 420 kg/m³, h = 2 mm, D = 2,1 Nm) | Elie Gl. (4) (nicht am Bild gesehen), Zahlen S. 3 | [wörtlich] (Zahlen, Textebene); Formel [Lehrbuch], reproduziert 0,0941 [eigene Rechnung] |
| G4.5 | Orthotrope Platte: n(ω) = A√(ρh)/(8π²) ∫₀^{2π} D(θ)^{−1/2} dθ, D(θ) = D₁cos⁴θ + 2D₃cos²θ sin²θ + D₂sin⁴θ; mit D₃ = √(D₁D₂) **genau** der isotrope Wert mit D = √(D₁D₂) | — | [hergeleitet]; numerisch für D₁/D₂ = 1…16: Faktor 1,0000 |
| G4.6 | Koinzidenz f_c = (c²/2π)·√(ρh/D) | — | [Lehrbuch] |
| G4.7 | Dämpfung: Energie ∝ e^{−αt}, η = α/ω, T60 = 13,8/α; g34: α(f) = 0,0105 f + 147 1/s (2,5–10 kHz gemessen) | LumiViz S199 §11 / Körperdatei `statistical.damping` | gelesen im Repo; Umrechnung [eigene Rechnung] |

### 4.2 Parametertabelle

| Größe | Wert | Quelle, Stelle | Marke | Gültig |
|---|---|---|---|---|
| Modendichte, 12 Gitarren (Ersatzplatte) | 0,018–0,022 /Hz (1 Mode je 46–54 Hz) | Elie Abb. 7b, 9; S = 0,09 m² | [aus Abbildung abgelesen] + [eigene Rechnung] | gemessen bis ≈ 2,5 kHz, Plattenverhalten „roughly constant“ darüber angenommen |
| Ersatzplatte, Gruppe 1 (Handarbeit) | D_Eq ≈ 55–95 Nm, M_Eq ≈ 0,9–2,0 kg | Elie Abb. 9, S. 11 | [aus Abbildung abgelesen] | |
| G_C∞ | −25,5 … −29,5 dB re 0,094 m/s/N = −46 … −50 dB re 1 m/s/N | Elie Abb. 8, S. 10 | [aus Abbildung abgelesen] + [eigene Rechnung] | Mittel der Stegmobilität |
| Bezugsplatte Fichte (unverstrebt) | 420 kg/m³, 2 mm, 2,1 Nm → β = 0,632 s/m²; auf der g34-Decke (≈ 0,143 m², [eigene Rechnung aus Mores List, ±15 %]) 0,045 /Hz | Elie S. 3 | [wörtlich] | Decke allein, ohne Balken und Steg |
| Zeder | **kein Wert in lokalen Quellen** | — | — | — |
| g34, Körperdatei | 0,0104 /Hz (26 Pole in 2,5–5 kHz) | S199 §11, `statistical.density` | gelesen | Ersatzpole (S199 D) |
| Verlustfaktor | 1–2 % bis 2,5 kHz | Elie S. 8, Abb. 7c (Stufe 1) | [wörtlich] / [aus Abbildung abgelesen] | ≤ 2,5 kHz |
| g34 nach α-Gesetz | η = 0,011 / 0,0064 / 0,0040 / 0,0028 bei 2,5 / 5 / 10 / 20 kHz; T60 80 / 69 / 55 / 39 ms | r5 | [eigene Rechnung] | gemessen bis 10 kHz, darüber fortgesetzt |
| Koinzidenz | Bezugsplatte 11,8 kHz; Ersatzplatte (Elie) 7,6–9,1 kHz | r5 | [eigene Rechnung] | Ersatzplatte ≠ Decke; nur Größenordnung |

### 4.3 Was die Quellen zur Abstrahlung über 5 kHz sagen

- Woodhouse 2021: „the guitar admittance tends to a value around –50 dB at higher frequency“ (S. 4382) [am Seitenbild
  geprüft] — deckt sich mit Elie (−46 … −50 dB) und mit der g34-Übertragung im Prüfstand (−50 … −52 dB bei 5–10 kHz,
  `GITARRE_UEBERTRAGUNG`) [eigene Rechnung].
- Woodhouse 2021 Fig. 1 (S. 4378): gemessene Stegadmittanz bis 10 kHz („Good coherence … up to approximately 8 kHz“,
  S. 4377) [am Seitenbild geprüft] — nur senkrecht, Stahlsaiten.
- Abstrahlgrad, Richtwirkung über 5 kHz an einer klassischen Gitarre: **in lokalen Quellen nicht gefunden.** Aus G4.6
  folgt nur: bis f_c ≈ 8–12 kHz strahlt die Decke unterkritisch (Abstrahlgrad < 1, mit f steigend) [Lehrbuch, eigene
  Einschätzung].

### 4.4 Zahlenbeispiele (`r5_luecken_2_3_4.py`, Ausgabe gekürzt auf Lücke 4)

```python
c0 = 343.0
def platte(name, rhoh, D, A):
    beta = math.sqrt(rhoh / D); nf = A * beta / 2
    fc = c0 * c0 / (2 * math.pi) * beta; G = 1 / (8 * math.sqrt(rhoh * D))
    ...
platte("Elie Bezugsplatte Fichte (420 kg/m3, 2 mm, 2,1 Nm), S = 0,09", 420 * 0.002, 2.1, 0.09)
for M, D in ((0.9, 60), (1.4, 75), (2.0, 95)):
    platte(f"Elie Abb. 9 Ersatzplatte M = {M} kg, D = {D} Nm, S = 0,09", M / 0.09, D, 0.09)
th = np.linspace(0, 2 * np.pi, 20001)
for verh in (1, 4, 10, 16):
    D1, D2 = 2.1 * math.sqrt(verh), 2.1 / math.sqrt(verh); D3 = math.sqrt(D1 * D2)
    Dth = D1 * np.cos(th) ** 4 + 2 * D3 * np.cos(th) ** 2 * np.sin(th) ** 2 + D2 * np.sin(th) ** 4
    faktor = np.trapezoid(Dth ** -0.5, th) / (2 * np.pi) * math.sqrt(D3)
for fq in (1000, 2500, 5000, 10000, 20000):
    a = 0.0105 * fq + 147     # g34
```

```
  Elie Bezugsplatte Fichte (420 kg/m3, 2 mm, 2,1 Nm), S = 0,09: beta = 0.632 s/m^2, n(f) = 0.0285 /Hz (1 Mode je  35.1 Hz), f_c =  11.8 kHz, G_inf =  -20.5 dB re 1 m/s/N
  dieselbe auf der Decke von g34 (A ~ 0,143 m^2)  : beta = 0.632 s/m^2, n(f) = 0.0452 /Hz (1 Mode je  22.1 Hz), f_c =  11.8 kHz, G_inf =  -20.5 dB re 1 m/s/N
  Elie Abb. 9 Ersatzplatte M = 0.9 kg, D = 60 Nm, S = 0,09: beta = 0.408 s/m^2, n(f) = 0.0184 /Hz (1 Mode je  54.4 Hz), f_c =   7.6 kHz, G_inf =  -45.8 dB re 1 m/s/N
  Elie Abb. 9 Ersatzplatte M = 1.4 kg, D = 75 Nm, S = 0,09: beta = 0.455 s/m^2, n(f) = 0.0205 /Hz (1 Mode je  48.8 Hz), f_c =   8.5 kHz, G_inf =  -48.7 dB re 1 m/s/N
  Elie Abb. 9 Ersatzplatte M = 2.0 kg, D = 95 Nm, S = 0,09: beta = 0.484 s/m^2, n(f) = 0.0218 /Hz (1 Mode je  45.9 Hz), f_c =   9.1 kHz, G_inf =  -51.3 dB re 1 m/s/N
  orthotrop D1/D2 =  1 / 4 / 10 / 16 (D3 = sqrt(D1 D2)): Modendichte / isotrop(sqrt(D1 D2)) = 1.0000
      1000 Hz: alpha =   158 1/s, eta = 0.0251, T60 = 0.088 s, Halbwertsbreite = 25 Hz, Modenueberlapp (n = 0,0104) = 0.26, (n = 0,019) = 0.48
      2500 Hz: alpha =   173 1/s, eta = 0.0110, T60 = 0.080 s, Halbwertsbreite = 28 Hz, Modenueberlapp (n = 0,0104) = 0.29, (n = 0,019) = 0.52
      5000 Hz: alpha =   200 1/s, eta = 0.0064, T60 = 0.069 s, Halbwertsbreite = 32 Hz, Modenueberlapp (n = 0,0104) = 0.33, (n = 0,019) = 0.60
     10000 Hz: alpha =   252 1/s, eta = 0.0040, T60 = 0.055 s, Halbwertsbreite = 40 Hz, Modenueberlapp (n = 0,0104) = 0.42, (n = 0,019) = 0.76
     20000 Hz: alpha =   357 1/s, eta = 0.0028, T60 = 0.039 s, Halbwertsbreite = 57 Hz, Modenueberlapp (n = 0,0104) = 0.59, (n = 0,019) = 1.08
```

Lesart [eigene Rechnung]: Elies gemessene Dichte (0,019–0,022/Hz) liegt **um den Faktor 1,8–2,1 über** dem Wert der
Körperdatei (0,0104/Hz aus der Polzahl). Mit dem α der Mikrofone ergibt das eine Modenüberlappung von 0,6–0,8 bei
5–10 kHz (Datei: 0,33–0,42). Gegenrechnung LumiViz S199 §11: die gemessene Pegelschwankung 5,5 dB ist der Wert eines
dichten Felds (5,6 dB) — passt besser zur höheren Dichte. S199 Fassung „C mit dreifacher Dichte“ änderte den Terzpegel
um ≤ 0,6 dB (Messung im Repo, nicht hier wiederholt).

### 4.5 Annahmen, die bleiben

Dichte über 2,5 kHz konstant (Elie misst bis 2,5 kHz; Plattentheorie trägt es), Zedernwerte fehlen, Dämpfung über
10 kHz fortgesetzt, Abstrahlgrad über 5 kHz ohne Quelle.

---

## 5. Optionen für LumiViz

Zielgrößen: (Z1) Druck je Kraft am Hörort je Terzband, (Z2) Lautheit über den Umfang (GZ14 a), (Z3) Abklingen und
Doppelabklingen der Teiltöne, (Z4) Klangfarbe über 5 kHz. Keine Empfehlung.

### 5.1 Hörort in 1 m

| Weg | braucht | Z1 | Z2 | Z3 | Z4 |
|---|---|---|---|---|---|
| **H-a** Hörorte bleiben 10 cm über der Decke, Vermerk „Nahfeld“ | nichts | unverändert; absoluter Pegel 11–24 dB über einem 1-m-Ort; A0 gegen T1 um ≈ 10 dB anders gewichtet als in 1 m (r3) | unverändert (S201: 11,9 LU Spanne) | unverändert | unverändert |
| **H-b** fester Abschlag je Band nach Kolben (G1.10/r2: −22 … −24 dB bis 1 kHz) | Kolbenradius | nur Pegelverschiebung, Form gleich | unverändert (Lautheit relativ) | unverändert | über 1 kHz unbelegt (Kolben ungültig) |
| **H-c** je tiefer Mode ein Fernfeld-Gewicht aus dem Zwei-Kolben-Modell (G1.4–G1.11), darüber H-b | Schalllochort, Schalllochanteil je Mode (aus upper east der Messdatei überbestimmt prüfbar, §1.1), A0/T1-Parameter | A0 relativ zu T1 um ≈ +10 dB gegen heute (−11 gegen −21 dB, r3); Bänder 80–125 Hz verschieben sich am stärksten | tiefste Töne (E2–G2, Grundton an A0) lauter gegen die Mitte → Spanne ändert sich, nicht gerechnet | Abklingen unverändert (nur Gewichte) | unverändert |
| **H-d** gemessene Übertragung: Perry 2015 (p/F auf zwei Kugeln, drei klassische Gitarren, 80–2000 Hz) und für Richtwirkung TU-Berlin-Terzdaten | Ladeliste Nr. 1, 3 (Stufe 1) | gemessene Differenz je Terz und Richtung, bis 2 kHz | wie Messung | unverändert | Richtwirkung über 2 kHz nur relativ (TU Berlin, gespielt) |

### 5.2 Zweite Stegrichtung

| Weg | braucht | Z1 | Z2 | Z3 | Z4 |
|---|---|---|---|---|---|
| **R-a** heute: ein Winkel je Mode, \|tan θ\| = clip(0,03 (f/200)^1,27; 0,03; 0,4) | nichts | Yyy 25–45 dB unter Bank/Scherrer unter 1 kHz (§2.2) → parallele Polarisation strahlt kaum | gering | Doppelabklingen auf fast allen Tönen mit zweiter Rate = Saitendämpfung (r4) | Yzy, Yyy ab 1,5 kHz −8/−16 dB |
| **R-b** ein Winkel je Mode, aber Verteilung mit schwerem Rand (wenige fast parallele Moden), so dass ⟨tan²⟩ und ⟨tan⁴⟩ beide Bank treffen | Bank-Ablesung (Stahlsaiten, ±3 dB) | Hy deutlich stärker an wenigen Moden | gering | η_b,y steigt um ≈ \|Yyy/Yzz\| (§2.3) — zweite Rate etwas schneller, Doppelabklingen bleibt | Kreuzterm −12…−20 dB |
| **R-c** volle 2×2-Matrix je Pol (Bank G2.3) bzw. eigene Moden der Parallelrichtung | gemessene Yyy, Yzy einer klassischen Gitarre (Woodhouse 2004, Lambourg & Chaigne 1993 — Ladeliste) | treuester Weg, wenn Daten da | — | Abklingen beider Polarisationen aus Messung | belegbar bis 10 kHz (Bank-Methode) |
| **R-d** zweite Richtung starr (Prüfstand ohne `--winkel`) | nichts | wie heute senkrecht | — | **kein** Doppelabklingen, keine Schwebung | — |

### 5.3 Zupf

| Weg | braucht | Z1 | Z2 | Z3 | Z4 |
|---|---|---|---|---|---|
| **Z-a** Zupfort fest 16 cm (Traube: Hand bleibt) | nichts | — | S201: einzelne Tasten ab Bund 15 bis 3,3 LU anders, Kennzahlen kaum | — | Kammfilter wandert mit R (Tab. §3.3) |
| **Z-b** R fest (Scherrer „skilled performers“) | Wahl R (16/65 oder 12/65) | — | hohe Lagen: x_p 5–8 cm statt 16 → mehr Obertöne, F₀ steigt (G3.3) | — | heller in hohen Lagen |
| **Z-c** heute min(16 cm, L/2) | nichts | — | S201 Zahlen | — | — |
| Dynamik **D-a** h = 0,8 … 4 mm linear in dB (zwei belegte Stufen, Rest Annahme) gegen **D-b** heutiges GZ6 a (Iowa-Neigung) | — | Pegel ±7 dB um 2 mm | Spanne pp–ff 14 dB aus h | — | Neigung: D-a ohne, D-b mit 3 dB/Oktave |
| Breite **B-a** Gauss h = 7,5 mm (heute) gegen **B-b** Nagel w ≈ 1 cm (sinc, f_c = c/w) | — | — | — | — | B-a: Saite 6 −20 dB bei 3,5 kHz; B-b: erste Nullstelle 10,7 kHz (Saite 6) — **über 5 kHz ein großer Unterschied an den Basssaiten** |

### 5.4 Über 5 kHz

| Weg | braucht | Z1 | Z2 | Z3 | Z4 |
|---|---|---|---|---|---|
| **S-a** heute: Dichte 0,0104/Hz, α gemessen | nichts | Terzpegel an der Messung (±1,4 dB) | — | α 200–250 1/s | Feinstruktur grober als gemessen? |
| **S-b** Dichte nach Elie 0,019–0,022/Hz, α gleich, Betrag der Abstrahlung neu gestellt | Elie (Abb. 7b, 9) | Terzpegel gleich (wird gestellt); S199: ≤ 0,6 dB bei ×3 | — | gleich | Überlappung 0,6–0,8 statt 0,3–0,4; Schwankung näher am dichten Feld |
| **S-c** Dichte aus Plattenformel G4.1/G4.5 mit Deckenwerten (Fichte/Zeder, Balken) | E_L, E_R, h, Balkenplan — **fehlt** | — | — | — | — |

---

## 6. Was die LumiViz-Sitzung prüfen muss

1. **Wright Tab. 5.4** gilt für zugeklebtes Schallloch und BR2 — nicht für g34 mit offenem Loch. Die Zahlen in r1/r3
   sind Größenordnungen.
2. **Ort des Schalllochs** (0,17 m vom Steg) und die Lage von upper east (20 cm „toward the upper part“ — Richtung nicht
   genau beschrieben) an Mores Method Abb. 4 nachmessen; davon hängt die Prüfung „Schalllochanteil je Mode“ ab.
3. Ob die Mores-Messdatei upper east tatsächlich enthält und ob sein Gewicht je Mode zum Zwei-Kolben-Modell passt
   (§1.1: überbestimmt, also prüfbar) — nicht hier gerechnet, die Messdatei liegt nicht im Repo.
4. Die Herleitung G2.4–G2.6 (Vorzeichen: Kraft der Saite auf den Steg, e^{iωt}); geprüft ist sie hier nur gegen
   Woodhouse 2021 Gl. (1) (1-D) und gegen die exakte Wurzel (r4).
5. Die Lesart der Kurtosis (§2.2) hängt an Bank-Ablesungen mit ±3 dB und an der Annahme zufälliger Vorzeichen.
6. Scherrer Abb. 3.8 nicht als Zahl übernehmen (Faktor 10, §3.1 G3.3); Scherrers „Grenzfrequenz 50 Hz“ (G3.6) nicht als
   Filter übernehmen.
7. Elie: Seitenzahlen sind die der Vorabfassung (Stufe 1); die Einheit „Hz⁻¹“ in Abb. 7b ist hier aus Abb. 9 erschlossen.
8. Zupfort 16 cm (Pavlidou-Messort) gegen Traubes „normal“ 12 cm.

## 7. Nicht gefunden / nicht herleitbar

| Lücke | Was fehlt | Wo gesucht |
|---|---|---|
| 1 | Gemessene Differenz Nahfeld/1 m je Terz an derselben klassischen Gitarre; seitlich/hinten | Wright 1996 (nur vorn, 0,9 m), Stufe-1-Quellen; Perry 2015 nicht zugänglich (Stufe 1) |
| 1 | Bodenanteil (Wright §4.4 Bodenkolben) — nicht gerechnet, Werte Tab. 5.7/5.8 für Boden vorhanden (PDF-S. 111, nur Textebene gesehen) | Wright |
| 2 | Yyy, Yzy einer klassischen Gitarre; Modalwinkel | Woodhouse 2012/2021 (nur senkrecht), Bank 2010 (Stahlsaiten); Woodhouse 2004, Lambourg & Chaigne 1993 nicht lokal |
| 3 | Kraft/Auslenkung über zwei Dynamikstufen; Ablösedauer; Zupfort je Bund an Spielern | Scherrer 2013, Traube 2004 (Textebene); Pavlidou 1997 nicht lokal |
| 4 | Holzwerte Fichte/Zeder (E_L, E_R, h) für eine Deckenformel; Verlustfaktor einer klassischen Gitarre über 5 kHz außer g34; Abstrahlgrad über 5 kHz | Ducceschi 2026 (keine Treffer), Elie 2012 (bis 2,5 kHz), Woodhouse 2021 |

## Ladeliste (nichts geladen)

Aus der Stufe 1 unverändert gültig: Nr. 1 Perry 2015, Nr. 2 Perry & Richardson 2013, Nr. 3 TU-Berlin-Richtwirkung,
Nr. 8 Woodhouse 2004 a/b, Nr. 9 Lambourg & Chaigne 1993, Nr. 10 Elie u. a. 2013 (HAL) — siehe dort. Neu:

| Nr. | Dateiname (Vorschlag) | URL | Größe | Lizenz / Vermerk | Wozu |
|---|---|---|---|---|---|
| 12 | `Gough_1981_TheoryStringResonances_Acustica49.pdf` | Acustica 49 (1981) 124–141 — keine URL geprüft | — | Verlag | Lücke 2: Primärquelle der Kopplung Saite–Steg (G2.5) |
| 13 | `Christensen_Vistisen_1980_SimpleModelGuitar_JASA68.pdf` | JASA 68 (1980) 758–766 — keine URL geprüft | — | Verlag | Lücke 1: Zwei-Oszillator-Modell im Original |
| 14 | `Pavlidou_1997_StudyGuitarPluck_PhD_Cardiff.pdf` | ORCA Cardiff — keine URL geprüft | — | Repositorium | Lücke 3: Auslenkung je Dynamik (Abb. 6.21), Bahnen (Abb. 6.18), Ablösung |

Bibliografische Angaben 12–14 sind aus dem Gedächtnis des Modells bzw. aus Literaturverweisen, nicht gegen eine Quelle
geprüft [eigene Einschätzung].

## Was der Auftrag verlangt hat und nicht geliefert ist

- Lücke 1 (a): **gemessene** Terztabelle — nur gerechnet (Kolben, Zwei-Kolben); 1 (b) seitlich/hinten — nicht belegt.
- Lücke 2: Parametersatz je Band **an einer klassischen Gitarre** — nur Stahlsaiten (Bank) und die heutige Annahme.
- Lücke 3: Parameter je Dynamik über zwei Stufen; Zupfort je Lage gemessen.
- Lücke 4: Modendichte mit Zahlen für **Zeder**; Dämpfung über 10 kHz.

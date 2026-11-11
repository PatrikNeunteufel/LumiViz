# Vorgaben je Taste der durchschlagenden Zunge (34j-5)

> **Typ:** eigene Herleitung und Rechnung am Prüfstand, ohne App. **Stand:** 2026-11-11, LumiViz S190 — alle vier
> Instrumente über jede Taste gerechnet, Bend untersucht; nichts am Kern geändert. Entscheide: ZV4 a (Kern-Form),
> ZV5 a (Tonhöhe folgt dem Druck), ZV6 a (Bass-Seite 8′ und 4′), ZV7 a (Bend über das Ventil, Kurve und Grenze je
> Taste aus dem Stimmer) — `../../audio/Instrumente_IC_Konzept.md` §6.
> **Prüfstand:** [proben/2026-11-11_S190_pruefstand.cpp](proben/2026-11-11_S190_pruefstand.cpp) (Bau
> `proben/2026-11-11_S190_bau.bat`), rechnet eine Zunge mit den Rechenschritten von `rechneEineZunge` und prüft das in
> jedem Lauf Bit für Bit gegen den Kern (6 Töne). **Ergebnisse:**
> [proben/2026-11-11_S190_pruefstand_ergebnisse.txt](proben/2026-11-11_S190_pruefstand_ergebnisse.txt) — je Block der
> Aufruf. **Quellen:** [2026-11-11_S190_quellenlage_34j-4.md](2026-11-11_S190_quellenlage_34j-4.md).
> **Messkette:** Ton 1 s gehalten, 48 kHz, Wind in 20 ms auf den Druck; Tonhöhe aus den Nulldurchgängen der Auslenkung;
> Lautheit K-bewertet (BS.1770) über die letzten 250 ms; Einschwingen = Zeit bis −5 dB der Endhülle; „spricht“ =
> Amplitude ≥ ¼ der Ruhelage und in den letzten 250 ms nicht mehr als 10 % gewachsen. Jede dritte Taste und die oberste.
> Gestimmt wird f₀ am mittleren Druck (ZV2 a). **Gesetz über den Umfang:** Wert an C4 mal (f / f_C4)^Exponent.

## 1. Wie der Kern rechnet, als Formeln

Aus den Gleichungen des Kerns (Zunge.cpp:29–100), linearisiert um die Ruhelage unter Druck; am Prüfstand bestätigt.

- **Schließdruck** (die Zunge liegt statisch im Schlitz): p_zu = (m / S_r)·ω₀²·x₀, mit der Masse je wirksamer Fläche
  m/S_r (∝ Dicke), der Eigenfrequenz ω₀ und der Aufbiegung x₀. Das ist Fletcher 1993 Gl. 6. Die Zunge spricht aus der
  Ruhe bis rund 0,75·p_zu an.
- **Anfachung:** σ = ω₀·[(p/p_zu)·x₀·g·x/(1 + x²) − ζ], mit g = d/(d² + a²) (d Abstand zum Schlitz unter Druck, a
  Seitenspalt) und x = Strömungswiderstand des Schlitzes / (ω₀ · Trägheit des Kanals). Die Schwelle liegt bestenfalls
  (x = 1) bei 2ζ·p_zu.
- **Folge:** stuft man nur die Länge (Vorlage: Dicke und Aufbiegung fest), wachsen Schließdruck und Schwelle mit f² —
  F3 1,06 kPa, A6 112 kPa. Das ist der Befund S188.

Gegenprobe der Formeln an der gemessenen Harmoniumzunge C3 (Cottingham 1998/1999): mit k = 150 N/m, M = 0,21 g,
S_r = 32 mm², ζ = 0,006 und der Schwelle 0,2 kPa folgt eine Trägheit, die dem Kanal von Puranik & Scavone
(50 mm / 64 mm²) entspricht, wenn die Strömung den ganzen Rand der Zunge nimmt; und p_zu ≈ 2,1 kPa — dort liegt in
Cottingham, Reed, Busha 1999 Abb. 3 das Minimum der Tonhöhe. (Rechnung, kein Beleg.)

## 2. Zwei Modellformen

| | **A — Kern-Form** (die Gleichungen von 34j-2) | **B — Adachi u. a. 2026, Gl. 2–7** |
|---|---|---|
| Strömungsfläche | W·√((y + x₀)² + a²), öffnet beidseits des Schlitzes | (L + W)·√(max(x,0)² + a²), öffnet nur oberhalb |
| Trägheit | fester Kanal L₂/S₂ | Luftpfropf d = 0,5·√(S/π), mit der Öffnung veränderlich |
| Zufuhr | Ventil (Bernoulli) in eine Kanzelle mit Volumen | Druckverlust K_p an der Klappe, ohne Volumen |
| Belegt durch | Puranik & Scavone 2023 (Gl. 1–5), Werte „offen“ | vollständiger Satz der C4-Zunge (Tab. 1), gegen den Versuch von Okada verglichen |

**B ist nachgebaut und trifft die veröffentlichten Zahlen** (3 s, Drucksprung, ρ = 1,17): 1 kPa, K_p = 35 → 265,17 Hz
(Adachi 265,1), Amplitude 1,68 mm (Abb. 6/7: rund 1,6), U bis 793 cm³/s (rund 750); K_p = 150 → 261,65 Hz (261,7);
0,2 kPa 0,42 mm, 1,6 kPa 2,35 mm (Abb. 7). Mit 16-facher statt 4-facher Abtastung gleich auf 0,01 Hz.

Beide über den Umfang, Wind 200 / 566 / 1 600 Pa, 15 Tasten F3–A6:

| Satz | spricht bei min / Mitte / max | Lautheit über den Umfang (LU) | größte Abweichung (Cent) | Einschwingen Mitte | Tonhöhe über Druck (Cent/kPa) | Amplitude |
|---|---|---|---|---|---|---|
| A0 Vorlage | 7 / 11 / 13 | 1,6 / 3,0 / 6,9 | 0,5 / 0 / 29,6 | 145–175 ms | −1,2 … 0,1 | 3–8 % der Länge |
| A1 Kern-Form, Dicke und Aufbiegung ∝ f^−0,7 | **15 / 15 / 15** | 2,0 / 1,8 / 9,3 | 0,2 / 0 / 4,6 | 135–175 ms | −3,4 … 0,1 | 1–7 % |
| B0 Adachi-Form, gestuft wie die Vorlage | 0 / 0 / 0 | — | — | über 600 ms | — | — |
| B1 Adachi-Form mit den Gesetzen von A1 | 0 / 6 / 6 | — / 1,8 / 2,5 | — | — | — | — |
| B2 wie B1, Trägheit vierfach | 11 / 14 / 9 | 8,9 / 23,7 / 14,3 | 2,7 / 0 / 2,9 | 150–270 ms | −3,1 … 0,2 | **9–160 % der Länge** |

**Lesart:** B schwingt mit den veröffentlichten Werten langsam an (an C4 rund 5 1/s, Endwert erst nach 2–3 s) und
startet oberhalb des Schließdrucks aus der Ruhe gar nicht. Gemessen sind an A4 60–90 ms mit dem Finger und rund 400 ms
mit dem Balg (Llanos-Vázquez u. a. 2014). Verstärkt man in B die Trägheit, bis es schnell genug ist, wächst die
Amplitude über die Zungenlänge hinaus — B hat keinen Begrenzer, weil die Öffnung unterhalb der Platte geschlossen
bleibt. A begrenzt die Amplitude auf wenige Ruhelagen (Cottingham, Lilly, Reed 1999: fast konstant über der Schwelle)
und trägt über den ganzen Umfang.

## 3. Akkordeon, Diskant (F3–A6)

Gesetze (Kern-Form), jeder Wert mit Belegkraft:

| Größe | an C4 | Exponent | Belegkraft |
|---|---|---|---|
| Länge | 32,8 mm | −0,42 | C4 *gemessen* (Adachi Tab. 1); Exponent *abgeleitet* aus den Zungen 42 mm / 142,7 Hz und 37 mm / 226,4 Hz (Llanos-Vázquez 2014, Tabla 3) |
| Breite | 3,45 mm | −0,2 | C4 *gemessen*; Exponent *abgeleitet* (Bass 60 × 6 mm, Forum; gibt 5,0 mm an E1) |
| Masse je Fläche m/S_r | 2,92 kg/m² | −0,7 | C4 *gemessen* ((2/3)·0,248 g); Exponent *abgeleitet* aus der Schwelle |
| Aufbiegung x₀ | 0,30 mm | −0,7 | C4 *gemessen*; Exponent *abgeleitet* — hält Aufbiegung ≈ Zungendicke über den Umfang (Werkstattregel, nur Bericht) |
| Seitenspalt a | 0,14 mm | −0,7 | C4 *gemessen*; Exponent *abgeleitet* |
| Dämpfung ζ | 0,003 | 0 | C4 *gemessen* |
| Kanal L₂ / S₂ | 60 mm / 6,9 mm² | −0,42 / 0 | *abgeleitet* aus der Einschwingzeit A4 (60–90 ms, gemessen) |
| Ventil, Kanzelle | D1: offen (300 mm²), 4,3 cm³ · D2: 8 mm², 1 cm³ · D3: 6 mm², 1 cm³ | −0,9 | *abgeleitet* aus der Tonhöhe über dem Druck an C4 (−16 Cent/kPa gemessen, Adachi Abb. 5) |
| Wind | 200–1 600 Pa | — | *Annahme* (Balgdruck nirgends gemessen gefunden; Laborachse bei Adachi 0–2,5 kPa) |

Der Schließdruck läuft damit von 1,9 kPa (F3) auf 7,5 kPa (A6): tiefe Zungen ersticken bei sehr hohem Druck, hohe nie
— wie am Instrument (Llanos-Vázquez 2008).

| Zielgröße | Ziel | D1 Ventil offen | D2 Ventil 8 mm² | D3 Ventil 6 mm² |
|---|---|---|---|---|
| spricht bei 200 / 566 / 1 600 Pa | alle | 15 / 15 / 15 | 15 / 15 / 15 | 15 / 15 / 15 |
| Lautheit über den Umfang | ≤ 15 LU | 5,0 / 4,2 / 6,8 | 1,8 / 1,4 / 4,5 | 2,3 / 1,7 / 4,1 |
| Stimmung über den Windbereich | ±10 Cent (ZV1 a) | +0,1 … −5,1 | +2,9 … **−14,4** | +4,8 … **−16,7** |
| Tonhöhe über Druck an C4 | −16 Cent/kPa (gemessen) | −1,3 | −8,0 | −11,1 |
| … über den Umfang | — | −3,7 … +0,1 | −12,4 … −1,2 | −15,4 … −1,4 |
| Dynamik 200 → 1 600 Pa | Bericht | 18,2–20,4 LU | 13,7–16,5 LU | 12,8–16,4 LU |
| Einschwingen bei 566 Pa | A4 60–90 ms (Finger) | 80–120 ms | 70–95 ms | 70–115 ms |
| Einschwingen bei 200 Pa | — (Balg rund 400 ms) | 165–200 ms | 145–215 ms | 160–350 ms |
| Amplitude an C4, 566 / 1 600 Pa | fast konstant (ZH3 a) | 1,9 / 2,4 mm | 1,3 / 1,2 mm | 1,1 / 0,9 mm |
| Ersticken | tiefe ja, ab C6 nie | F3 bis 1,6 kPa, ab C6 > 4,5 kPa | wie D1 | wie D1 |

**Gewählt (ZV5 a): D3.** Über jede Taste (41): spricht 41 / 41 / 41, Lautheit über den Umfang 2,4 / 1,7 / 4,1 LU,
Einschwingen bei 566 Pa 70–125 ms.

**Zwei Zielgrößen widersprechen sich:** die gemessene Tonhöhe über dem Druck (−16 Cent/kPa an C4) ergibt über einen
Windbereich von 1,4 kPa rund 22 Cent — mehr als die ±10 Cent aus ZV1 a. Entscheid ZV5.

## 4. Akkordeon, Bass (F1–E3)

Setzt man das Diskantgesetz unter F3 fort, fällt der Schließdruck auf 0,8 kPa: bei 1 600 Pa sprechen 2 von 9 Tasten.
Echte Basszungen sind beschwert (Llanos-Vázquez 2014: (f/f′)² = 0,98 + 3,7·M/m, gemessen). Unter F3 deshalb: Masse je
Fläche ∝ f^−1,2 (an C4 gerechnet 2,388 kg/m²), Aufbiegung ∝ f^−0,8 (0,288 mm), Seitenspalt ∝ f^−0,4, sonst wie D3,
stetig an F3 — der Schließdruck bleibt bei 1,9–2,5 kPa. Belegkraft der Exponenten: *abgeleitet*.

| Lage | spricht bei 200 / 566 / 1 600 Pa | Lautheit über den Umfang (LU) | Abweichung (Cent) | Einschwingen bei 566 Pa |
|---|---|---|---|---|
| 8′ — Zungen F1–E3 (29–52), jede Taste | 23 / 24 / 24 von 24 (F1 braucht bei 200 Pa über 1 s) | 1,9 / 0,9 / 3,0 | +4,9 … −18,6 | 75–180 ms |
| 16′ — Zungen F0–E1 (17–28) | **0 / 5 / 5 von 5** | — / 1,8 / 0,9 | bis −16,2 | — |

Die Vorlage führt die Bass-Chöre 16′ + 8′; der 16′ von F1 läge bei 21,8 Hz, die tiefste echte Zunge liegt um E1
(41 Hz; Forum: E1-Zunge 60–80 mm). Entscheid ZV6.

## 5. Harmonium (C3–F6)

Gesetze durch die zwei gemessenen Zungen (C3: Cottingham 1998 — 150 N/m, 0,21 g, 4 mm breit, rund 4 cm, ζ = 0,006;
B5: Puranik & Scavone 2023 — 16 × 2 × 0,4 mm Messing, y₀ = 0,1 mm), Wind 200–1 500 Pa (Puranik & Scavone, *gemessen*
am Instrument):

| Größe | an C4 | Exponent | Belegkraft |
|---|---|---|---|
| Länge | 29,6 mm | −0,463 | *abgeleitet* durch C3 (rund 40 mm) und B5 (16 mm), beide *gemessen* |
| Breite | 3,19 mm | −0,35 | wie Länge (4 mm, 2 mm) |
| Masse je Fläche | 5,81 kg/m² | −0,187 | durch C3 (M/S_r = 6,56) und B5 ((4/3)·ρ·h = 4,53) |
| Aufbiegung | 0,296 mm | −0,81 | B5 *gemessen* (0,1 mm); C3 0,5 mm *abgeleitet* aus Schwelle und Tonhöhenminimum |
| Seitenspalt | 0,10 mm | −0,5 | *Annahme* |
| Dämpfung ζ | 0,00314 | −1,0 | C3 *gemessen* (0,006); der Abfall zur Höhe *Annahme* — siehe unten |
| Kanal L₂ / S₂ | 120 mm / 6,9 mm² | −0,42 / 0 | *abgeleitet* aus dem Einschwingen |
| Ventil, Kanzelle | 4 mm², 1 cm³ | −0,6 / −0,9 | *abgeleitet* aus der Tonhöhe über dem Druck an C3 |

| Zielgröße | Ziel | ζ = 0,006 für alle Tasten | ζ ∝ 1/f (jede Taste, 42) |
|---|---|---|---|
| spricht bei 200 / 548 / 1 500 Pa | alle | 7 / 12 / 15 von 15 | **42 / 42 / 42** |
| Lautheit über den Umfang | ≤ 15 LU | 1,4 / 2,4 / 5,3 | 7,5 / 6,8 / 6,9 |
| Tonhöhe über Druck an C3 | rund −24 Cent/kPa (gemessen) | −22,3 | −22,2 |
| Abweichung über den Windbereich (Berichtsgröße, ZV5 a) | — | +5,0 … −30,8 | +4,9 … −24,0 (C3), oben unter 1 |
| Amplitude an C3, 548 / 1 500 Pa | rund 1,2 mm, fast konstant (gemessen) | — | 1,43 / 1,51 mm |
| Dynamik 200 → 1 500 Pa | Bericht | 18,7–19,6 LU | 18,2–18,9 LU |
| Einschwingen bei 548 Pa | — (Anfachung C3 gemessen rund 11 1/s) | 85–135 ms | 85–275 ms (oben langsam) |

**Die Dämpfung ist der offene Punkt.** Mit der an C3 gemessenen Dämpfung für alle Tasten hat die dicke Messingzunge
B5 im Modell bestenfalls eine Schwelle von 2ζ·p_zu ≈ 210 Pa; im Versuch von Puranik & Scavone klingt sie schon bei
0,2 kPa. Eine zur Höhe fallende Dämpfung (ζ ∝ 1/f: 0,006 an C3, 0,0008 an B5) behebt das; gemessen ist sie nicht.
Das Gehäuse (Tiefpass 3 kHz der Vorlage) ist nicht Teil dieser Herleitung: DAFx23 gibt keine Filterwerte, nur eine
Kurve.

## 6. Mundharmonika (C4–C7) und Melodica (F3–C6)

**Mundharmonika.** Anker ist die Zunge von Millot & Baumann 2007 (Lee Oskar, 444 Hz: 12,95 × 2,1 mm, Steifigkeit
47,9 N/m, Ruhelage 0,528 mm, Spalt 50 µm, Q = 95, Zungendicke 110 µm, Plattendicke 900 µm — Tab. 2, S. 14, am
Seitenbild gelesen; die Ruheöffnung der aufschlagenden Zunge ist dort Ruhelage plus halbe Dicke, 0,583 mm); die
Länge über den Umfang folgt den Schlitzlängen 17 … 7 mm (Forum, geringe Belegkraft): Exponent −0,427, das gibt an
444 Hz 13,6 mm gegen 12,95 gemessen. Länge 16,2 mm an C4, Breite 2,1 mm fest, Masse je Fläche 0,656 kg/m² ∝ f^−0,7,
Aufbiegung 0,765 mm ∝ f^−0,7, Spalt 0,05 mm fest, ζ = 0,00526, Kanal 60 mm ∝ f^−0,42, Mundkanal offen (kein Ventil),
**Atemdruck 150–1 000 Pa — *Annahme*** (Millot rechnet mit 570 und 783 Pa; die gemessene Zunge hat einen Schließdruck
von 1,9 kPa, darüber erstickt sie). Über jede Taste (37): spricht 37 / 37 / 37, Lautheit über den Umfang
0,9 / 0,5 / 0,8 LU, Abweichung +0,4 … −1,9 Cent, Dynamik 17,2–18,6 LU, Einschwingen 35–60 ms, Tonhöhe über Druck
−2,7 … +0,3 Cent/kPa. **Auffällig:** die Amplitude liegt an C4 bei 25–35 % der Zungenlänge, an C7 bei 8–13 % — über
der Plausibilitätsgrenze von 15 % (ZH3 a). Ursache ist die große Ruhelage aus der Quelle; mit den 0,15–0,2 mm der
Werkstätten (Forum) wäre sie kleiner. Bending und Overblow sind nicht Teil von 34j.

**Melodica** (ZH2 a): die Gesetze des Akkordeons D3 auf F3–C6, ein Chor, Atemdruck 200–1 600 Pa wie der Balg —
alles *Annahme*. Über jede Taste (32): spricht 32 / 32 / 32, Lautheit über den Umfang 1,3 / 0,5 / 4,1 LU, Abweichung
+4,8 … −16,7 Cent, Einschwingen 70–85 ms.

## 7. Bend über das Ventil (Akkordeon, Melodica)

Mit dem Ventilgesetz aus D3 senkt eine kleinere Ventilöffnung die Tonhöhe — der Mechanismus von Adachi (halb gedrückte
Taste). Gestimmt mit offenem Ventil am mittleren Druck; Cent gegen die Note / Pegel gegen offen:

| Ventilfläche mal | F3 | C♯4 | A4 | F5 | C♯6 | A6 |
|---|---|---|---|---|---|---|
| 0,7 bei 566 Pa | −7 / −2,7 dB | −6 / −3,0 | −4 / −3,1 | −2 / −3,4 | 0 / −4,0 | spricht nicht |
| 0,5 bei 566 Pa | −17 / −5,7 | −13 / −6,1 | −9 / −6,5 | −4 / −7,7 | spricht nicht | — |
| 0,35 bei 566 Pa | −33 / −9,3 | −25 / −9,9 | spricht nicht | — | — | — |
| 0,5 bei 1 600 Pa | −37 / −6,8 | −34 / −7,4 | −27 / −8,0 | −18 / −9,1 | spricht nicht | — |
| 0,35 bei 1 600 Pa | −61 / −12,2 | −66 / −13,3 | −58 / −16,7 | spricht nicht | — | — |

(Bei 1 600 Pa liegt die Tonhöhe schon mit offenem Ventil 17 / 10 / 7 / 5 Cent unter der Note.) Das deckt sich mit den
Messungen: am Spieler 10–35 Cent, mit Balg und Finger tiefer als mit dem Finger allein (Elejalde-García 2021), an den
Zungen ab C6 gar kein Bend (Llanos-Vázquez 2008). Die Abbildung Cent → Öffnung der Vorlage (−100 Cent → 1 % der
Fläche) trägt nicht: die Tiefe hängt von Taste und Druck ab, und die Zunge erlischt, bevor ein Halbton erreicht ist.
Entscheid ZV7.

## 8. Offen für 34j-6

- Windbereich des Akkordeons (200–1 600 Pa) und Atemdruck (Mundharmonika 150–1 000 Pa, Melodica wie der Balg):
  *Annahme*, nirgends gemessen gefunden.
- Dämpfung des Harmoniums über den Umfang (§5); Amplitude der Mundharmonika (§6); F1 bei 200 Pa (§4).
- Die Gesetze sind Potenzen um C4; die Tafel führt sie als Kurven je Taste (34j-6), die Werte am Kern müssen die
  Zahlen dieses Dokuments wiedergeben (Prüfstand und Kern rechnen gleich).
- Chöre und Schwebung, Gehäuse, Balgwechsel und Vibrato sind am gestimmten Kern in 34j-6/34j-7 nachzumessen.

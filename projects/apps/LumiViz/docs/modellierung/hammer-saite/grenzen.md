# Hammer-Saite — Grenzen beim Setzen aus Quellen (SQ1 a)

> S155, 2026-10-26. Eigene Zusammenstellung aus den Recherchen `recherche/2026-10-26_S155_bereich_*.md`.
> **Prüfstand je Zeile:** „nachgelesen“ = von Claude mit `quelle_suchen.py` in der lokalen Datei gefunden (PDF-Seite /
> gedruckte Seite); „Agent“ = nur im Agentenbericht, nicht selbst gelesen. Entscheid über die Ableitung der Grenzen:
> SQ2 (`docs/audio/Spielweisen_Konzept.md` §4.1).

## Was die Quellen messen

| Größe | Beleg | Instrument | Prüfstand |
|---|---|---|---|
| `decay` | T60 je Taste, **eine** Gerade über den ganzen Ton: „approx. 8 s to 10 s in the bass range to approx. 0.5 s in the treble range“ (Plath 2019, PDF 202 / S. 168) | Konzertflügel (Steinway D, zwei Exemplare) | nachgelesen |
| `decay` | b1 = 0,25 / 1,1 / 9,17 s⁻¹ an C2 / C4 / C7 (Bensa et al. 2003, Tab. I, PDF 2) → T60 = ln 1000 / σ1 ≈ 27,6 / 6,2 / 0,61 s; **ein** Dämpfungswert über den ganzen Ton („a coherent mean damping factor when multiple components are present“, PDF 9) | Yamaha C6 | nachgelesen |
| `decay` | Steigung im Fenster 0,2–2,2 s je Taste, aus Abb. 9 abgelesen (Cheng/Dixon/Mauch 2015) | RWC, Instrument ungenannt | Agent |
| `decay` | Es4: „about 8 dB/sec“ = T60 7,5 s (Weinreich, KTH-Vorlesung) | eine Saite des Chors | Agent |
| `lateDecay` | Es4, eine Saite: Nachklang „less than one-quarter that rate“ → spät/früh > 4 (Weinreich, KTH) | eine Saite | Agent |
| `lateDecay` | spät/früh 2,3–2,5 an einer Saite ≈ 146 Hz (Tan 2017, Abb. 7.14) | Prüfstand mit Bassstegstück | Agent |
| `lateDecay` | ab etwa 160 Hz liegt der Verlust am Steg „by some 20 dB“ über dem der Luft (Woodhouse 2022, PDF 6) → die Gegenlage klingt höchstens so lange wie die Saite allein (Deutung des Agenten, ohne Zahl) | Broadwood | nachgelesen (Zitat), Deutung Agent |
| `highDecay` | b1, b2 an C2 / C4 / C7 (Bensa Tab. I) → highDecay = (b1 + b2·(π/L)²) / (b1 + 100·b2·(π/L)²) = 0,838 / 0,625 / 0,052; Ausgleich b1 = 4,4·10⁻³·f0 − 0,04, b2 = 1,0·10⁻⁶·f0 + 10⁻⁵ (Gl. 37a/b, PDF 9); Streuband b1 × 0,5…3, b2 × 0,5…1,5 aus Abb. 13 | Yamaha C6, am Steg, mit Chor | Tabelle und Gl. nachgelesen, Streuband Agent |
| Verstimmung | „Up to F6 the average detuning is in the range proposed by Kirk with differences between strings of approx. 1-2 cent. In the high treble range differences of up to 5 cent appear.“ (Plath 2019, PDF 194 / S. 160) — Mittel der Saitenpaare | Steinway D | nachgelesen |
| Verstimmung | Kirk 1959: Prüfbereich bis ±3 Cent, bevorzugt 1–2 Cent | — | Agent (nur Abstract) |
| Verstimmung | „0.5 to 4 cents“ (Cuenca & Caussé 2007, PDF 7) | Aussage | Agent |
| δ | „The detuning between the two modes is found to be between 0.1Hz and 0.2Hz“ bei ≈ 188 Hz (Tan 2017, PDF 99 / S. 86) → 0,9–1,8 Cent | Monochord ohne Steg | nachgelesen |
| δ | „detuning could range between 0.2Hz to 0.4Hz“ bei ≈ 146 Hz (Tan, PDF 161 / S. 148) → 2,4–4,7 Cent | Prüfstand mit Bassstegstück | nachgelesen |
| δ | E1: 3,5–7 Cent, umgerechnet aus Mori 2004 über Nishiguchi 2004 | Flügel, E1 | Agent |
| γ | Monochord „+5° and -85°“ (Tan, PDF 99); am Prüfstand „angular offset, γ, of +7°“ (PDF 176 / S. 163); 22° nur in der Simulation „artificially increased from +7° to +22°“ (PDF 171 / S. 158) | Prüfstand | nachgelesen |
| γ | Stegstifte geometrisch 15–20° gegen die Senkrechte (Patent US7884271B2) | Bauweise | Agent |
| u | „a small striking irregularity (y01 =1mm, y02 =0,8mm)“ (Thomas et al. 1998, PDF 5 / S. 4) → u ≈ 0,11; „large“ 0,4 mm → u ≈ 0,43; Una corda u = 1 — **von den Autoren gesetzt, nicht gemessen** | Simulation, Es4, zwei Saiten | nachgelesen (0,8 mm) |

## Gegen *Salamander* (`werte/salamander_c5.txt`)

- **Verstimmung** (`schwebung_cent`, 0,46–3,16 an den Chor-Tasten): alles im belegten Bereich (0–6 Cent).
- **δ** (Einzelsaiter 21–30, `schwebung_cent` 2,2–6,0): im Bereich von Tan und Mori (0,9–7 Cent).
- **highDecay** an den Tasten von Bensa (36 / 60 / 96: 0,968 / 0,894 / 0,121): über der Mitte von Bensa (0,838 / 0,625 /
  0,052), aber im Streuband (oberer Rand, gerechnet mit b1 × 3, b2 × 0,5: 0,969 / 0,908 / 0,218).
- **decay:** Die Quellen messen eine Gerade über den ganzen Ton (Mischung beider Stufen) an anderen Flügeln; *Salamander*
  misst die erste Stufe. Gegen Plath und Bensa liegt *Salamander* im Bass darüber (27–36: 33–42 s gegen 8–28 s), im
  Diskant darunter (78–87: 1,9–3,1 s gegen Cheng ≥ 4 s). Die Größen sind nicht dieselben.
- **lateDecay, u:** keine Zahl aus einer Messung.

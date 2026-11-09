# Modell Membran — gebaut (Klasse 5, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/gemeinsam/Membran` und `rechnung/klassen/05_membran/` rechnen.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Membran.hpp/.cpp` (gemeinsam), `Trommel.hpp/.cpp`,
> `Vorgaben.cpp`. **Tests:** `test_trommel.cpp` (8 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:**
> [offene-fragen.md](offene-fragen.md) F-1 … F-8 a (über FRAGEN_KONSOLIDIERT T6, T7, T8; F-7 a abgewandelt: nichts
> zurückstellen). Spielbar: **Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo** (alle ungemessen).

## 1. Aufbau

**Fell** (Galerkin, eigene Herleitung, vorab gerechnet): je Ordnung m die idealen Moden J_m(j_mn·r/a) als Basis; Steife
T·∫∇φ·∇φ, Masse ∫σ(r)·φφ (ungleich: Tabla); **Luftlast** einer Seite als Rayleigh-Integral im Wellenzahlraum (Fell in
einer Schallwand): Masse aus κ > k₀, Widerstand der Abstrahlung aus κ < k₀, frequenzabhängig je Mode iteriert (wie
Christian et al. 1984, Packer Gl. 4.36 — nur Bericht); die Luft im Kessel als Masse wie ein Halbraum (Näherung, offen)
und als **Luftfeder** auf m = 0 (ρc²/V·g_i·g_j); **zwei Felle** über dasselbe Volumen gekoppelt. Unter 1 kHz mit Luft
und kleiner Basis, darüber ohne Luftmasse mit großer Basis (bis 1,2–4 kHz je Instrument). Dämpfung (d₁ + d₃k²)/(2σ)
(Avanzini & Marogna, nur Bericht) plus die Abstrahlung. **Schlag:** Hertz-Kontakt (`stosse`, 8-fach fein) an der
Schlagstelle. **Ausklingen** mit **Tonhöhengleiten** nach Marogna, Avanzini & Bank 2010 Gl. 20 (**Quelle**):
T_qs/T₀ = E·h/(2πR²(1−ν²)T₀)·E(t)/T₀, je 32 Takte. **Ausgang:** Schall unter 45° — Beschleunigung × Hankel-Transformierte
bei k₀·sin 45° × (−i)^m. **Teppich** (F-6 a, vereinfacht): Rauschen 2–8 kHz, getastet von der Schnelle des Resonanzfells
in der Mitte (ohne Drahtmoden). **Statistischer Teil** über der Grenze: abklingendes Rauschen (30 ms). Spielweisen am
Schritt (F-3 a): Mitte, Rand (0,85·a), Rimshot und Cross-Stick (Klick des Reifens halbphysikalisch), gedämpft (×6);
**Wirbel** (F-4 a): Schläge mit 14 Hz über die Dauer, abwechselnd ±10 %; **Pauke** (F-5 a): die Note skaliert alle
Moden ((1,1) auf der Note), Pedal-Glissando über die Dauer.

**Abweichung vom Entwurf:** die Doubletten der Moden m > 0 (sin-Moden) sind nicht gebaut (nur die angeschlagene
cos-Mode); der Teppich ohne Drahtmoden.

## 2. Größen

| Größe | Tom | Pauke | Bassdrum | Snare | Tabla | Conga / Bongo | Herkunft |
|---|---|---|---|---|---|---|---|
| Radius | 0,165 m | 0,32 m | 0,279 m | 0,178 m | 0,07 m | 0,149 / 0,089 m | Avanzini (R 16–20 cm, Bericht); Packer (Bericht); sonst Händler |
| Spannung T (N/m) | 2000 / 1800 | 5360 | 1200 / 1000 | 3325 / 2500 | 3000 | 3000 | Avanzini / Packer / Bilbao (Bericht); sonst **offen** |
| σ (kg/m²) | 0,27 | 0,262 | 0,27 | 0,26 / 0,12 | 0,3 + Belag | 0,4 | Avanzini, Packer (widersprüchlich: 0,31), Bilbao (Bericht); **offen** |
| Kessel | V = πR²·0,28 m | ⅔πa²·1,2a (L/a = 1,2, Packer S. 48 — Form offen) | πR²·0,4 | πR²·0,14 | πR²·0,25 | offen (ohne Feder, innen ½) | offen |
| Schlägel | Stock 0,05 kg, 60 N/mm^1,5 | 0,028 kg, K 1,6·10⁸, α 2,54 | Filz 0,12 kg | Stock | Finger 0,01 kg | Hand 0,3 / 0,15 kg | Bilbao (Bericht); sonst **offen** |
| v pp … ff | 1 … 8 m/s | 0,5 … 5 | 0,5 … 4 | 1 … 8 | 0,5 … 4 | 0,5 … 4 | Dahl (Akzent ×2, **Quelle**); Torin 5 m/s (Bericht); Weg offen |
| Gleiten E·h, ν | 700 N/m, 0,2 | dto. | dto. | dto. | dto. | dto. | Avanzini & Marogna S. 898 (Bericht) |
| Belag (Tabla) | — | — | — | — | 1 + (σ² − 1)·½(1 − tanh((x − 0,492)/0,091)), σ = 2,57 | — | Sathej & Adhikari (Bericht); Deutung σ² **offen** |

## 3. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Ohne Luft: Bessel-Verhältnisse 1,593 / 2,136 / 2,295 | auf 10⁻³ |
| **Pauke gegen Packer** (gemessen 1,50 : 1,97 : 2,44, (0,1) 0,81) | **1,47 : 1,93 : 2,37, (0,1) 0,79**; (1,1) 188 Hz (gemessen 172 — mit σ = 0,31 wären es 173); T60 (0,1) 0,11 s (< 0,3), (1,1) 0,54 s (0,8), (2,1) 1,18 s (1,7) |
| Zwei Felle: (0,1) spaltet (Wagner an der Snare 188 → 240/366) | Tom 135 / 223 Hz |
| **Tabla harmonisch** (Raman): (2,1), (0,2) ≈ 1,5; (3,1), (1,2) ≈ 2 × (1,1) | 1,49 / 1,53 / 2,00 / 2,01 |
| Kontakt Tom (Dahl: mf 5–8 ms) | **9–11 ms** (länger — offen.md); Kraft 14–127 N (Wagner: 3 bis > 100 N) |
| Gleiten ff: Snare (Wagner ≈ 37 Cent), Pauke („minor“) | 13 Cent / 2 Cent |
| Pauke: (1,1) auf der Note (C3, E3, G3), Glissando | grün |
| Teppich: 3–8 kHz mit gegen ohne | +10,5 dB |
| Spielweisen, Wirbel, alle Instrumente klingen; Determinismus, Abdruck (gcc = clang) | grün |
| Rechenzeit ff 2 s | Tom 3,4 %, Pauke 3,1–3,4 % (Grenze hier 5,5–7 %) |

**Zeit beim Bau** (L7): die Moden je Instrument 0,3–2 s (vorab, einmal je Vorgabe); ein Ton von 2 s 70 ms.

## 4. Hörproben

`ic_hoerproben <Ordner> 5`: je Instrument pp/mf/ff, Mitte, Rand, gedämpft, Wirbel; Snare dazu Rimshot, Cross-Stick,
ohne Teppich; Pauke Glissando — 53 WAVs. Gehört: noch nicht (L5).

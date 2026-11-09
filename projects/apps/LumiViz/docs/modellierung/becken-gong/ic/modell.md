# Modell Becken und Gong — gebaut (Klasse 6, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/06_becken_gong/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Becken.hpp/.cpp`, `Vorgaben.cpp`. **Tests:** `test_becken.cpp`
> (6 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:** [offene-fragen.md](offene-fragen.md) F-1 … F-6 a (über
> FRAGEN_KONSOLIDIERT T6, T7, T8, T9, E1; nichts zurückgestellt). Spielbar: **Tamtam, Gong, Crash, China, Splash, Ride,
> Hi-Hat** — alle als **flache Platte** und ungemessen (Form und Material nach F-5 a an Iowa zu setzen, lokal).

## 1. Aufbau (F-1 a: physikalisch, von Kármán, energieerhaltend, vorab)

**Platte:** freie Kreisplatte, Moden W = J_m(λr/a) + C·I_m(λr/a) aus den freien Randbedingungen (M_r = V_r = 0;
Ableitungen über die Rekursionen der Besselfunktionen), cos-Familie (die angeschlagene), bis `bisHz`. **Spannung:** Airy-
Funktionen eingespannt, Φ = J_m(ζr/a) − J_m(ζ)/I_m(ζ)·I_m(ζr/a), die 50 kleinsten ζ. **Kopplung** H^n_pq = ∫Φ_n·L(W_p,
W_q) dA mit der von-Kármán-Klammer, der Winkel geschlossen integriert (Auswahl m_n = m_p ± m_q → dünn), radial mit
Gauß–Legendre (160 Punkte). Bewegung: q̈ + 2σq̇ + ω²q = −E/(2ρ)·Σ ζ⁻⁴·η_n·(H^n q) + p/(ρh), η_n = qᵀH^n q.
**Zeitschritt SAV** (Bilbao, Ducceschi & Zama — nur Bericht): ψ = √(2V + ε), Kraft ½(ψ^{n+½} + ψ^{n−½})·∇V/ψ,
Sherman–Morrison je Takt — energiestabil ohne Iteration, O(Kopplungen) je Takt. Lineare Teile mit exakter Frequenz.
**Dämpfung** ξ = 8·10⁻³·ω^−0,4 (Humbert, nur Bericht). **Schlag:** Hertz-Kontakt explizit. **Ausgang:** Beschleunigung
an drei Punkten (offen). **Choke** (F-4 a): Dämpfung der Hand ab der Zeit, in 50 ms voll. **Hi-Hat** (F-3 a): Öffnung als
Dämpfung (Kontakt der Schalen nicht gebaut).

**Abweichungen vom Entwurf (begründet):** (1) flache Platte statt Schale — die Form ist ungemessen (F-5 a), die Schale
käme mit dem Fit der Form; (2) Bandbreite der nichtlinearen Moden 1,2–3 kHz je Instrument (die Kosten wachsen mit dem
Quadrat der Modenzahl); darüber nichts (offen); (3) nur die cos-Familie (die sin-Moden blieben aus Symmetrie in Ruhe).

## 2. Größen

| Größe | Tamtam | Gong | Crash | China | Splash | Ride | Hi-Hat | Herkunft |
|---|---|---|---|---|---|---|---|---|
| Durchmesser | 28″ | 17″ | 18″ | 18″ | 10″ | 20″ | 14″ | Iowa / Händler |
| Dicke | 1,6 mm | 2,0 mm | 0,9 mm | 0,8 mm | 0,7 mm | 1,3 mm | 1,0 mm | **offen** (Brezas: 8″ 1,2–1,4 mm, Bericht) |
| Material | B20: 8860 kg/m³, 110 GPa, ν 0,3 | | | | | | | Rechenwerte ohne Herkunft (Sekiguchi & Samejima, Bericht); ν offen |
| Moden / Kopplungen | 82 / 16 376 | 43 / 5 430 | 123 / 30 868 | — | 61 / 9 945 | 106 / 24 457 | 82 / 16 376 | Rechnung |
| Schlägel | Filz 0,25 kg | 0,15 kg | Stock 0,03 kg, 2·10⁷ N/m^1,5, 1–8 m/s | | | | | **offen**; an DAFx-15 (Bericht) gesetzt |

## 3. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Freie Platte gegen Leissa (ν 0,33): λ² 5,253 / 9,084 / 12,23 / 20,52 | 5,262 / 9,069 / 12,244 / 20,513 (±0,5 %) |
| Energie des SAV-Schemas ohne Dämpfung, ff | erhalten auf 10⁻¹³ |
| **Aufblühen**: Anteil über 1 kHz (Crash) | ff 0,31 → 0,35 in 150 ms (0,45 → 0,67 in 0,5 s ohne Dämpfung); pp 0,06 → 0,04 — die Kaskade entsteht nur nichtlinear |
| Kontakt Crash ff (DAFx-15: 0,8–1 ms, 100–300 N) | 0,98 ms, 563 N |
| Choke, Hi-Hat geschlossen/offen, Schlagstellen, alle klingen | grün |
| Determinismus, Abdruck — **gcc = clang trotz Chaos** (E1 a) | grün |
| Rechenzeit Crash (vorab, Grenze 10 × Echtzeit) | 6,3 × (Tamtam 3,5 ×, Gong 1,1 ×) |

## 4. Hörproben

`ic_hoerproben <Ordner> 6`: je Instrument pp/mf/ff (2,5 s), Kuppe, Rand, Choke; Hi-Hat geschlossen, halb — 44 WAVs.
Gehört: noch nicht (L5).

# Modell Durchschlagende Zunge — gebaut (Klasse 10, Grundlagen)

> **Typ:** eigene Ausarbeitung — was `rechnung/klassen/10_zunge/` rechnet.
> **Stand:** 2026-10-30, IC Session 4 (Cloud). **Code:** `Zunge.hpp/.cpp`, `Vorgaben.cpp`, `Stimmung.inc` (erzeugt von
> `ic_stimme_zunge`). **Tests:** `test_zunge.cpp` (6 Testfälle). **Offen:** [offen.md](offen.md). **Entscheide:**
> [offene-fragen.md](offene-fragen.md) F-1 … F-9 a (über FRAGEN_KONSOLIDIERT T1, T2, T3a, T3b, T5, T7, T8, E2). Spielbar:
> **Akkordeon** (Diskant mit Musette, Bass-Seite), **Harmonium**, **Melodica**, **Mundharmonika** (gerade Töne) — alle
> ungemessen.

## 1. Aufbau

**Zunge:** erste Biegemode als Masse-Feder-Dämpfer (Millot & Baumann, nur Bericht), Bezug C4 nach Adachi et al. 2026
Tab. 1 (**Quelle**): Masse ⅔·0,248 g, ζ 0,003, Länge 32,8 mm, Breite 3,45 mm, Ruhelage 0,3 mm, Seitenspalt 0,14 mm; über
den Umfang Länge ∝ 1/√f bei gleicher Dicke (f ∝ h/L², Rechnung). **Strömung** nach Puranik & Scavone 2023 Gl. 1–5
(**Quelle**): Balg → Ventil (Bernoulli) → Kanzelle (Masse erhalten, mit dem Verdrängungsfluss der Zunge) → träge Strömung
(L₂/S₂) → Strahl durch den Schlitz, S_u = W·√((y + y₀)² + a²); die Kraft auf die Zunge aus dem Druck an der Zunge.
Die Trägheit der Strömung treibt die Schwingung (St. Hilaire). Der Fluss wird je Takt geschlossen gelöst (quadratisch).
Vierfach abgetastet. **Ausgang** ρ/(4π)·dU/dt. **Stimmung** (Rechnung, `stimme`): je Note das **Fenster des Drucks**, in dem
die Zunge anspricht (Amplitude ≥ 2 %), Expression geometrisch darin, f₀ so, dass sie in der Mitte auf der Note spielt.
**Chöre** (F-4 a): je Chor eine eigene Zunge, Musette als Schwebung über den Umfang. **Bass-Seite** (F-5 a): unter der
Teilung 16′ + 8′. **Bend** (F-2 a): das Ventil schließt (Adachi), das Modell kappt bei dem, was die Physik hergibt.
**Balgwechsel** (F-3 a): Druckeinbruch 60 ms. **Harmonium:** Wind und Kanal nach Puranik & Scavone, Gehäuse als Tiefpass.

**Abweichung vom Entwurf:** das Gehäusefilter des Harmoniums (10 Biquads, DAFx23) ist nur ein Tiefpass (Werte nicht
da); die Mundharmonika ohne Vokaltrakt und ohne Kopplung von Blas- und Ziehzunge (F-6 a: Bending erst mit Adachi 2026
und einer Aufnahme — gebaut nach T8 nur die geraden Töne).

## 2. Größen

| Größe | Akkordeon | Harmonium | Melodica | Mundharmonika | Herkunft |
|---|---|---|---|---|---|
| Zunge C4 | Adachi Tab. 1 | Masse ×8490/7850 (Messing) | wie Akkordeon | 20 mm, Q 95 | Adachi (**Quelle**); Millot (Bericht); sonst **offen** |
| Kanzelle / Wind | 14,8 × 14,6 × 20 mm | V₁ 1,3 l | wie Akkordeon | wie Akkordeon | Bericht; Puranik Tab. 1 (Bericht); Tiefe **offen** |
| Kanal L₂/S₂ | 30 mm / 6,9 mm² | 50 mm / 64 mm² | dto. | dto. | Puranik (Bericht); Akkordeon **offen** (Befund: mit 5 mm 400 ms Anlauf) |
| Druck | Fenster je Note (300 … 2000 Pa als Suchrahmen) | 200 … 1500 Pa | 300 … 3000 | 300 … 3000 | Puranik „0.2–1.5 kPa“ (**Quelle**); sonst offen |
| Chöre | M + M+ (A4 +2,00 Hz, ∝ √f) | 8′ | 8′ | 8′ | Hergert (**Quelle**) |
| Bass | 16′ + 8′ unter F3 | — | — | — | offen |

## 3. Geprüft (Tests)

| Prüfung | Ergebnis |
|---|---|
| Spricht selbst an, schwingt unter f₀ (Cottingham „somewhat below“) | C4 4,9 Cent darunter; Amplitude 5,5 % der Länge (Cottingham: 15 % — kleiner, offen) |
| Akkordeon: jede dritte Diskantnote bei pp/mf/ff | **42/42** auf ±10 Cent; ff schneller da als pp |
| Musette A4 | Schwebung **2,00 Hz** (Hergert 2,00 Hz) |
| Bend −80 Cent (Ventil) | **−3,7 Cent** — gekappt (Spieler 10–35 Cent, Elejalde-García) — offen.md |
| Bass-Seite, Balgwechsel | grün |
| Harmonium, Melodica, Mundharmonika: Mitte ±10 Cent | grün |
| Determinismus, Abdruck (gcc = clang), Rechenzeit Akkordeon Musette | 2,4 % |

**Zeit beim Setzen (L7):** ein Ton von 0,5 s 50 ms; die Stimmung aller vier Instrumente 1:41 min auf vier Threads —
erzeugte Datei.

## 4. Hörproben

`ic_hoerproben <Ordner> 10`: je Instrument drei Noten × pp/mf/ff, Vibrato, Bend, Balgwechsel; Akkordeon Bass — 49 WAVs.
Gehört: noch nicht (L5).

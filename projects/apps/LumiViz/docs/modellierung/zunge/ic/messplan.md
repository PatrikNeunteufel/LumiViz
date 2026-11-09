# Messplan — Durchschlagende Zunge (Klasse 10)

> **Stand:** 2026-10-26, IC Session 2. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §5;
> Bausteine B1–B8 aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur
> Bericht · Einschätzung.

**Lage:** Das Akkordeon lässt sich an TinySOL setzen (CC BY 4.0, 82 Tonhöhen × pp/mf/ff, nachgelesen), das Harmonium an
der Euterpea-Reihe (CC0, eine Stärke), die Melodica an fese (CC BY 4.0, eine Stärke). Für die Mundharmonika gibt es nur
Blastöne (VCSL, CC0). Bendings, Register einzeln, Balgwechsel und alle Druckmessungen fehlen. Lizenzen in
[aufnahmen.md](aufnahmen.md).

Durchschlagende Zungen geben **gehaltene Töne** mit harmonischem Spektrum („harmonicity is conserved“ auch beim Bend,
Elejalde-García et al., nachgelesen S. 3). Die Teiltonmessung des Flügels (Abklingen) passt deshalb nur für Einsatz und
Ausklingen; für den gehaltenen Teil braucht es die Bausteine für Dauerklänge aus `messkette.md` (Einschätzung).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | f₀ der Zunge, Masse, Steife | Tonhöhe im gehaltenen Teil bei mf | B2 (f0) | TinySOL mf, alle 82 Tonhöhen; Euterpea; fese | ±2 Cent gegen die Stimmung (TinySOL: 35 Dateien nachgestimmt — dort nur ±5 Cent) |
| M2 | Zungenmaße (Obertöne), Quelldruck über Expression, Gehäusefilter | Teiltonpegel und spektraler Schwerpunkt je Stärke; LTAS | B1, Langzeitspektrum | TinySOL pp/mf/ff; Harmonium nur eine Stärke | ±1,5 dB je Teilton bis 10 kHz; Schwerpunkt ±5 % |
| M3 | Tonhöhe über Druck (V₁, L₂/S₂, Dämpfung) | Unterschied f0 zwischen pp und ff an derselben Taste | B2 | TinySOL (gleiche Taste, drei Stärken) | Vorzeichen fallend; Betrag ±5 Cent; Prüfstein −2,5 Hz/kPa an C4 (Adachi, nachgelesen) |
| M4 | Spalt, Ruhelage, Vorvolumen, Einschwingen | Einsatzzeit bis 90 % Pegel, Vorläufer (unregelmäßige erste Perioden) | B5 | TinySOL je Stärke; tiefe Zungen langsamer („slow speech … larger, lower-pitched reeds“, ISMA 2017, nachgelesen) | Einsatzzeit ±20 % |
| M5 | Dämpfung der Zunge | Ausklingen nach Druckende (T60 des Nachklangs) | B6/B7 | TinySOL, wenn die Dateien das Ende enthalten (nicht geprüft) | ±25 % |
| M6 | Ventilöffnung (Bend), Vokaltrakt | Tiefe und Verlauf eines Bends über der Zeit | B3 (f0-Verlauf) | **keine freie Aufnahme** außer einem Bend (Freesound 325377, CC BY 4.0) | Tiefe ±5 Cent; Prüfsteine 10–35 Cent Akkordeon (Elejalde-García), „Max Bend = 2.5 Hz“ (Coyle, nachgelesen) |
| M7 | Musette-Verstimmung | Schwebungsrate je Taste im Musette-Register | B1 (zwei nahe Teiltöne je Harmonische), B3 (AM) | **keine freie Aufnahme** mit ausgewiesenem Register; Prüfstein Hergert (0,5–7 Hz an A4) | ±0,2 Hz |
| M8 | Chöre (Fußlage, Pegel) | Teiltonpegel je Register auf einer Oktave | B1 | Commons „Accordion registers.ogg“ (gemeinfrei, verlustbehaftet, eine Oktave) | ±2 dB (Ogg) |

**Neu für die Messkette** (Einschätzung): Tonhöhe und Pegel eines gehaltenen Tons über pp/mf/ff an derselben Taste
(M3) und der Verlauf eines Bends (M6) — beides ist Dauerklang-Messung, die auch die Bläser brauchen (Klassen 7–9).

**Prüfstein vor dem Setzen:** Trifft eine Zunge mit Maßen aus der Quelle (Akkordeon C4, Harmonium B5) Tonhöhe und
Gradient über dem Druck (M1, M3) ohne Probieren? Puranik & Scavone mussten im Masse-Modell „unrealistic values“ setzen
(nachgelesen S. 1811) — erst wenn das ohne geht, wird über den Umfang gesetzt.

## Forschungsauftrag

1. Eigene Aufnahme eines Akkordeons: je Register einzeln (8′, 16′, 4′, Musette), Zug und Druck getrennt, Balgwechsel,
   Bellows shake, Bend mit halb gedrückter Taste; dazu der Balgdruck (Druckaufnehmer in einer Kanzelle) pp/mf/ff.
2. Mundharmonika diatonisch in C: alle Blas- und Ziehtöne, Bends je Kanzelle in Stufen, Overblows 4–6, Atemdruck.
3. Harmonium und Melodica in mindestens drei Stärken.

## Anfragen (nicht gestellt)

- Puranik/Scavone (McGill): ob die Vibrometer- und Druckmessungen der Harmoniumzunge (FA 2023) freigegeben werden können;
  dazu Klärung des Lizenzsatzes in DAFx23.
- Autor von filipemlins/Harmonica-Dataset: Lizenz der annotierten Phrasen (Bends).

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Musette-Tremolo aus Verstimmung der Zungen **und** Balgvibrato — zwei Träger derselben Schwebung.
2. **B — Urteil hörbar (HW1 a, HW2 a; IC-A2, IC-A3):** Toleranz aus dem Bezug (L1) als Gewicht; **hörbar getroffen**
   ist eine Größe bei |d| ≤ max(τ, S), S aus `rechnung/werte/hoerschwellen.txt` (Skalen Faktor, dB, Ewert; Lesart
   und Übertragung je Klasse in `gemeinsam/hoerschwellen.md`), ohne Zeile nur τ; **eindeutig** ist eine Größe, wenn
   die drei Startpunkte an ihr höchstens max(τ, S) auseinander enden (`ic::setzen::setzeMehrfach`, `eindeutig`).
   **Probe (HW3 a, `ic::setzen::probe`):** Glätte nach ZF8 a (2) an ≥ 5/6 der Noten (`glaette`, 15 Punkte), ≥ 5/6
   der Größen eindeutig (nach ZF1 a nicht erreichbare Noten zählen nicht) und am besten Punkt mehr Größen hörbar
   getroffen als an der Vorgabe — die Schwellen des Bestehens stehen damit vorher fest.
3. **C — Modell** nur bei systematischem Befund nach HW4 a: die nicht hörbar getroffenen Noten einer Größe weichen
   im Vorzeichentest (zweiseitig, exakt) in eine Richtung ab, Holm über alle Größen, α = 0,05; Ausreißer-Noten (|r|
   über dem Dreifachen des Medians) je Größe benannt (`messung::systematik` über `ic::setzen::alsTasten`).
4. **D — hören und setzen:** Hörproben Bezug | Vorgabe | Suche, Urteil mit Datum (L5).

**Glätte des Ziels am Modell (IC-A4, 2026-10-31; `ic_glaette_ziel`, Ergebnis `rechnung/werte/glaette_ziel.txt`):** synthetisches Ziel vom Modell selbst, der Parameter über seinen Bereich; „fest“ = Amplitude bei der festen Frequenz des Ziels (§3a), „Suche“ = Rate und Tiefe mit eigener Suche der Frequenz (§3b (2)); glatt nach ZF8 a (2).
Akkordeon A4, Musette-Schwebung (0,5–8 Hz): **fest wellig** (231 von 231); **Suche glatt** (Rate 1 : 1, Tiefe 11,7–16,3 dB). Messgröße: Rate und Tiefe der AM — so trennt sie auch die zwei Träger (Verstimmung, Balgvibrato).

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

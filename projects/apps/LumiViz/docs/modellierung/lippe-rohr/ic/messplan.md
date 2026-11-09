# Messplan — Lippe und Rohr (Klasse 7)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine der Messkette aus [gemeinsam/messkette.md](../../messung/ic/messkette.md) (B1 Teiltonverfolgung, B2
> Tonhöhenverlauf, B3 Vibrato, B4 Geräusch, B5 Einschwingen). Die Teiltonmessung des Flügels trägt **nicht** (gehaltene
> Töne, A2–A5). **Kennzeichnung:** nachgelesen · nur Bericht · Einschätzung.

## A. An freien Aufnahmen

Bezug: **Iowa MIS** (pp/mf/ff, reflexionsarm; Lizenz nachgelesen); Gegenprüfung **TinySOL** (5 Stufen, CC BY 4.0, nur
Bericht); Dämpfer **VSCO 2 CE** (CC0, nachgelesen).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Lippenresonanz je Note, Bohrung | Tonhöhe des gehaltenen Tons; Abweichung von der Impedanzspitze | B2 auf dem gehaltenen Teil | ohne Vibrato (Iowa `novib`) | ±5 Cent |
| M2 | Expression-Weg (Druck, Lippe; F-R2), μ | Spektrum (Teiltöne relativ zum Grundton) und Pegel über pp/mf/ff | B1, Mittel über den stationären Teil | drei Stufen (Iowa), fünf (TinySOL) | ±2 dB je Teilton bis 10. (Einschätzung; McAdams 1999: spektrale Hüllkurve gut hörbar, nur Bericht) |
| M3 | Güte Q, Einsatz (Mundruck-Sprung) | Anstiegszeit und Reihenfolge der Teiltöne | B5 | Iowa-Einsätze sind gespielt, Zunge unbekannt | ±20 % Anstiegszeit (Einschätzung) |
| M4 | Schmettern | Spektralschwerpunkt über der Stärke (Verhältnis ff/pp) | B1 | Posaune, Trompete | Verhältnis ±10 % (Einschätzung; Myers: 1,5 → 4,0 an der Posaune, nachgelesen) |
| M5 | Dämpfer (Straight, Harmon) | Spektrum mit/ohne Dämpfer bei gleicher Stärke | B1 | VSCO 2 CE (Raum unbekannt — nur Verhältnisse) | ±3 dB je Band (Einschätzung) |
| M6 | Vokaltrakt (Didgeridoo) | Lage der Formanten im Spektrum | Hüllkurve des Spektrums | VCSL Phrasen | ±100 Hz (Einschätzung) |
| M7 | Vibrato-Umsetzung (F-R3) | Rate, Tiefe in Cent, AM je Teilton | B3 | Iowa Trompete `vib` | Rate ±5 %, Tiefe ±3 Cent |

**Prüfstein vor dem Setzen:** Trifft das Modell mit der gesuchten Lippenresonanz die Tonhöhe (M1), bevor M2 gesetzt
wird? Sonst setzt der Ausgleich die Klangfarbe über eine falsche Lippe (Lehre aus 31d-6/31d-7).

## B. Messdaten mit Druck

- **Posaune, 31 Spieler** (Zenodo 18679877, CC BY 4.0, nur Bericht): Mundstückdruck und Druck im Mund gegen Klang —
  setzt den Expression-Weg (F-R2) am echten Spieler; Sensoren mit rund 1,7 kHz (nur Bericht) — für den Druckverlauf,
  nicht für die Schwingung.

## C. Forschungsauftrag (an einem echten Instrument)

1. Bohrungsprofile von Trompete, Horn, Tuba (zehn oder mehr Punkte genügen für B — Myers, nachgelesen S. 680 als
   Bericht des Verfahrens).
2. Eingangsimpedanzen mit Dämpfern (Straight, Cup, Harmon, Wah-Wah) an Trompete und Posaune.
3. Einzeltöne mit Cup-Dämpfer, gestopftes Horn, Crescendo-Aufnahmen.

## D. Anfragen (nicht gestellt)

- UNSW (Joe Wolfe): Lizenz der Impedanzdaten (Posaune Boutin) und der Didgeridoo-Messungen.
- Autoren von TinySOL: keine nötig (CC BY 4.0) — Lizenz am Ort nachprüfen, sobald Zenodo wieder erreichbar ist.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Register (Lippe gegen Rohr), Vibrato der Spieler, Wah (Öffnungsgrad).
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
Posaune B3, Vibrato-Rate (3–8 Hz, verlangt 25 Cent): **fest wellig** (231 von 231); **Suche glatt** (Rate 1 : 1). Befund: am Ton kommen nur 4,8–6,1 Cent an — die Lippe setzt die verlangte Tiefe nicht 1 : 1 um.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

# Messplan — Tonrad (Klasse 11)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §3;
> Bausteine B1–B8 aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur
> Bericht · Einschätzung.

**Lage:** Für den Generator gibt es **keine** freie Aufnahme einzelner Zugriegel; gesetzt wird er zunächst an
Konstruktionswerten (Übersetzungen, Handbuch) und an den M100-Dauerklängen (CC BY 4.0). Der Leslie hat freie Hochlauf-
Aufnahmen (CC0, gemeinfrei) und einen gepaarten Trocken-/Nass-Datensatz (CC BY 4.0). Lizenzen in [aufnahmen.md](aufnahmen.md).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Radfrequenzen | Tonhöhe der Räder | B2 an Dauerklängen | M100-Drones | ±0,5 Cent gegen die Übersetzungen |
| M2 | Radwellen, Zugriegel, Rückfaltung | Teiltonpegel je Registrierung | B1 | eigene Aufnahme (Forschungsauftrag 1) | ±1 dB je Teilton |
| M3 | Übersprechen | Restsignal fremder Räder im Dauerton | B1 (schmalbandig) | M100-Drones | Pegel ±3 dB |
| M4 | Tastenklick | Dauer und Spektrum des Einsatzes | B5 | eigene Aufnahme; Prüfstein Moro et al. (10–60 ms) | Dauer ±20 % |
| M5 | Perkussion | Abklingen der 2./3. Harmonischen fast/slow | B6/B7 | eigene Aufnahme | T60 ±15 % |
| M6 | Scanner | Rate, Tiefe, AM je Teilton V1–C3 | B3 | eigene Aufnahme | Rate ±3 %, Tiefe ±3 Cent |
| M7 | Leslie: Drehzahlen, Zeitkonstanten, Radius | Modulationsrate über der Zeit (Horn oben, Trommel unten getrennt über Bandpässe um 800 Hz) | B3, B7 | Freesound 136653/136652; Commons-Dateien als Gegenprobe | Endrate ±5 %; Zeitkonstante ±20 % |
| M8 | Leslie-Filter | Übertragung trocken → nass je Zweig | Vergleich Ein-/Ausgang | Zenodo 3562442 (Leslie 145) | Band ±2 dB, Modulationstiefe ±20 % |

**Prüfstein vor dem Setzen:** Trifft der Generator mit Konstruktionswerten die Tonhöhen der M100-Räder (M1)? Erst dann
werden Klangfarbe und Leslie gesetzt.

## Forschungsauftrag

1. Eigene Aufnahme einer Tonradorgel direkt (vor dem Leslie): je Zugriegel einzeln chromatisch, Perkussion fast/slow,
   soft/normal, Scanner V1–C3, Tastenklick langsam/schnell.
2. Leslie: Drehzahlen und Zeitkonstanten von Horn und Trommel (Tachometer oder Video), Richtwirkung des Horns.
3. Kennlinie des Vorverstärkers und des Schwellers.

## Anfragen (nicht gestellt)

- Autor von HamOrg (Michael Picher): Lizenz der direkt aufgenommenen Töne.
- CCRMA (Abel): ob die gemessenen Leslie-Impulsantworten über dem Winkel veröffentlicht werden können.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Leslie: Rotor **und** Horn tragen die Modulation gemeinsam; Scanner-Vibrato/Chorus.
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
Rotary langsam, die Weiche bei 800 Hz beachten: A4 über die **Trommel**, A6 über das **Horn** (20–120 U/min): **fest wellig** (746 bzw. 751 von 751); **Suche glatt** (Rate = U/min / 60, 0,33–2 Hz; über 8 s gemessen — 3 s lösen die langsame Drehzahl nicht auf).

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

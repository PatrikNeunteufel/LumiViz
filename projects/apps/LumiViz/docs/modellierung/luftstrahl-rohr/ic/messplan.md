# Messplan — Luftstrahl und Rohr (Klasse 9)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur Bericht ·
> Einschätzung.

## A. An freien Aufnahmen

Bezug: **Iowa MIS** Querflöte und Altflöte (pp/mf/ff, reflexionsarm; Lizenz nachgelesen); **VCSL** Blockflöten (CC0);
**VSCO 2 CE** Vibrato (CC0); Labialpfeifen **VAO** (BY-SA, F-5). Lizenzen siehe [aufnahmen.md](aufnahmen.md).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Bohrung, Pfeife; Ansatz der Querflöte | Tonhöhe je Griff, Drift über pp/mf/ff | B2 | Iowa `nonvib`; VAO-Pfeifen | ±5 Cent; Drift Querflöte ±5 Cent über die Stufen |
| M2 | W, y0, Expression-Weg | Spektrum, Verhältnis H2/H1, Spektralschwerpunkt über pp/mf/ff | B1 | stationärer Teil | ±2 dB je Teilton bis 10. |
| M3 | c_p, α_i, Druckskala | Registerschwellen und Hysterese | am Modell gegen Literatur | Querflöte G4, Blockflöte | Schwellen ±10 % gegen Terrien/Auvray (nur Bericht) |
| M4 | Vibrato-Umsetzung; Tremulant | Rate, Tiefe in Cent, AM je Teilton | B3 | Iowa/VSCO `vib`; Orgel mit Tremulant (Palo, falls freigegeben) | Rate ±5 %, Tiefe ±3 Cent, AM-Verhältnis oben/unten qualitativ (Fletcher: „little change in the fundamental“) |
| M5 | Mundtöne, Einschwingen | Einschwingdauer in Perioden; Vorläufer (2. Mode zuerst?) | B5 | Iowa-Einsätze, VAO-Pfeifen | Dauer im Bereich „25 to 30 periods“ (Flöte) bzw. 50 (Diapason) (nachgelesen) |
| M6 | Rauschanteil | Restsignal nach Abzug der Teiltöne, Pegel je Band | B4 (Iowa `ambient.silence` als Raumboden) | Querflöte, Blockflöte; Shakuhachi am einen Ton | ±3 dB je Band |

**Prüfstein vor dem Setzen:** Hält das Modell mit fester Geometrie und Druck allein die Tonhöhe der Blockflöte über den
Umfang (M1) und springt es an der berichteten Schwelle (M3)? Sonst setzt M2 den Strahl gegen einen falschen Resonator.

## B. Messdaten mit Druck

- **Blockflöte Druck und PIV**, Zenodo 13919050 (CC BY 4.0, nur Bericht): Munddruck, Audio vor dem Labium, Tonlöcher
  TH00–TH15 — setzt Druckskala und Strahl (M3) an einer echten Blockflöte.
- **VAO** (BY-SA, nur Bericht): Pfeife mit Maßen und Aufnahme — setzt das Pfeifenmodell ohne Raten der Geometrie.

## C. Forschungsauftrag

1. Bohrung der Böhm-Querflöte und der Altflöte mit freier Lizenz (oder eigene Messung).
2. Shakuhachi: Bohrung und Einzeltöne über den Umfang, mit meri/kari.
3. Anblasgeräusch: Pegel relativ zum Ton in dB (nicht gefunden).
4. Flatterzunge: Rate und Tiefe (nicht gefunden).

## D. Anfragen (nicht gestellt)

- UNSW: Lizenz der Impedanzdaten (Querflöte, Shakuhachi).
- VAO (Leipzig): ob die Messwerte der Pfeifen getrennt von den Aufnahmen unter einer freieren Lizenz stehen.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Register (Laufzeit des Strahls gegen Rohr), Vibrato der Spieler.
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
Blockflöte D5, Vibrato-Rate (3–8 Hz, verlangt 25 Cent): **fest wellig** (231 von 231); **Suche glatt** (Rate 1 : 1; Tiefe 2,3 Cent — Befund wie Klasse 7: der Druck setzt die Tiefe nicht 1 : 1 um).

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

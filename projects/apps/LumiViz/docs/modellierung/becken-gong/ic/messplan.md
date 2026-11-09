# Messplan — Becken und Gong (Klasse 6)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine B1–B8 aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur
> Bericht · Einschätzung.

Bezug: **Iowa MIS** (Becken, Hi-Hat, Tamtam, Gong; reflexionsarm; pp/mf/ff), **DrumGizmo** (Hi-Hat-Stufen, Ride, Choke;
CC BY 4.0), **VSCO** (Gong, 17 Stärken; CC0), **Beijing Opera Percussion** (Operngongs; CC BY 4.0), **Oslo** (Hängebecken
mit Schlagstelle und Bewegung; CC BY 4.0). Lizenzen in [aufnahmen.md](aufnahmen.md).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Material, Form | Frequenzen der tiefen Moden (leise Schläge) | B6 | Iowa pp, Schlägel; VSCO `gong_hit_pppp` | ±3 % |
| M2 | Dämpfung | Abklingen je Band | B7 | Iowa pp/mf | T60 je Band ±20 % |
| M3 | Schlag, Nichtlinearität | Band über der Zeit: Front der Energie nach oben, Schwelle zum Crash | B7 | Iowa pp/mf/ff, VSCO 17 Stärken | Zeit der Front ±20 %; Schwelle zwischen benachbarten Stufen |
| M4 | Schlagstelle | Spektrum Kuppe/Fläche/Rand | B1, B7 | Iowa `bell`/`normal`/`shoulder`; Oslo Mitte/Rand | ±3 dB je Band |
| M5 | Gleiten | Frequenz der stärksten Mode in den ersten 100–300 ms | B8 | Beijing Opera Daluo/Xiaoluo; Iowa Thai-Gongs | ±20 Cent |
| M6 | Hi-Hat | Abklingen und Spektrum je Öffnung; Fuß-Chick | B7 | DrumGizmo closed/semi-open/open/pedal | ±3 dB je Band; T60 ±20 % |
| M7 | Choke | Abklingen nach dem Greifen | B7 | Iowa `chokecrash`; CrocellKit `CrashLStopped` | Abfallzeit ±20 % |

**Prüfstein vor dem Setzen:** Trifft das lineare Modell (leiser Schlag) die tiefen Moden (M1)? Erst dann wird die
Nichtlinearität (M3) gesetzt — sonst gleicht die Kaskade einen falschen Grundaufbau aus.

**Bei chaotischen Rechnungen** (F-2): verglichen werden Statistiken (Band über der Zeit, Spektralschwerpunkt), nicht
Abtastwerte.

## Forschungsauftrag

1. E-Modul, Dichte und Dickenprofil von B20-Becken; Kuppe und Wölbung gemessener Becken.
2. Moden und Dämpfungen eines Crash und eines Ride (Laservibrometer oder Modalfit an trockenen Einzelschlägen).
3. Hi-Hat: Spalt, Pedalweg, Abschneidezeit.

## Anfragen (nicht gestellt)

- ENSTA (Touzé): Lizenz und Ort des VK-Gong-Codes.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** dichte, aufgespaltene Modenpaare (Schwebung aus Unrundheit und Dicke zugleich).
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
**Nicht prüfbar:** die aufgespaltenen Modenpaare (Unrundheit, Dicke) sind im Modell nicht gebaut (flache Platte, `offen.md`) — es gibt keinen Parameter, der die Schwebung trägt.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

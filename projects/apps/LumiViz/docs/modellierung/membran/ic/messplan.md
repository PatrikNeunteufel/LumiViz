# Messplan — Membran (Klasse 5)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine B1–B8 aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur
> Bericht · Einschätzung.

Bezug: **Drum Tom-Tom Timbre Dataset** (Toms, `cc-by-4.0` nachgelesen), **DrumGizmo** DRSKit/CrocellKit (Kit, CC BY 4.0
nachgelesen; Nahmikrofone, `power` je Anschlag), **VCSL/VSCO** (Pauke, Orchester, Hand-Trommeln, CC0), **Tabla
strokes** (CC BY 4.0). Lizenzen in [aufnahmen.md](aufnahmen.md).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Spannung, Luftlast, Kopplung der Felle | Modenfrequenzen und -paare | B6 (Modalfit ohne Reihe) | Tom-Tom-Datensatz je Stimmung; Pauke VCSL; Packer als Prüfziel | Frequenz ±2 % (Pauke (1,1)…(5,1) gegen Packer ±2 %) |
| M2 | Dämpfung je Mode | T60 je Mode | B6 | wie M1 | ±20 % |
| M3 | Schlag (Masse, Kontakt), Auftreffgeschwindigkeit | Pegel, Spektralschwerpunkt, Anstieg über die Stärke | B1, B7 | DrumGizmo-Nahmikrofon, `power` je Anschlag | ±2 dB je Band; Schwerpunkt ±10 % |
| M4 | Schlagstelle | Spektrum Mitte gegen Rand | B1 | Aasimonster `snare_on_center`/`off_center`; `snare-position` nur nach Lizenzklärung | qualitativ, dann ±3 dB je Band |
| M5 | Spannungsmodulation | Gleiten der stärksten Mode in den ersten 100–300 ms | B8 | laute gegen leise Anschläge | Gleiten ±10 Cent |
| M6 | Teppich | Rauschanteil je Band und seine Hüllkurve | B4, B7 | VCSL Snare `HitSN` gegen `HitNS` | ±3 dB je Band |
| M7 | Wirbel | Abstände und Stärken der Aufschläge | B1 (Einsätze) | VCSL/VSCO-Wirbel | Gruppenabstand im Bereich 250–350 ms (Miura 2023, nachgelesen) |
| M8 | Tabla (Belastung) | Teiltonverhältnisse je Bol | B6 | Tabla strokes | ±10 Cent gegen Raman/Sathej |

**Prüfstein vor dem Setzen:** Trifft das gerechnete Fell mit Luftlast die Modenreihe der Pauke nach Packer (M1)? Sonst
setzt M3 den Schlag gegen falsche Moden.

## Forschungsauftrag

1. Moden und Dämpfungen der Bassdrum und offener Kessel (Conga, Bongo).
2. Kontaktzeiten und Härte von Paukenschlägeln (Stulov; Chaigne & Doutaut).
3. Pauken-Glissando und Bayan-Gleiten: Umfang und Dauer.

## Anfragen (nicht gestellt)

- DrumGizmo-Team: Lizenz von `snare-position.zip` (Schlagstelle × Stärke).
- Tom-Tom-Datensatz (Leal de Castro): ob Kraft oder Pendelhöhe nachträglich bekannt ist.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Aufspaltung von Fell- und Kesselmode (zwei Träger einer Schwebung); Tonhöhe der Pauke über Spannung und Luftlast.
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
Tom, Kesselvolumen (0,5–2 × Vorgabe): **fest wellig** (106 von 131). **Suche:** die Schwebung liegt bei 9,07 Hz, **unabhängig vom Kessel** — das Kesselvolumen trägt sie nicht (Träger klären, etwa Schlag- gegen Resonanzfell); die Tiefe (1,5–3,3 dB) ist stetig, Σ r² hat ein Minimum, verfehlt aber das Sprungkriterium von ZF8 a (2) an einem steilen Rand, nicht an einer Kante.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

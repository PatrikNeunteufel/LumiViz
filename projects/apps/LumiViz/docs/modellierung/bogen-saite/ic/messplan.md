# Messplan — Bogen-Saite (Klasse 3)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine B1–B8 aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur
> Bericht · Einschätzung.

## A. An Aufnahmen

Bezug: **Iowa MIS 2012** (Vl, Va, Vc, Kb; arco/pizz; pp/mf/ff; reflexionsarm) und **TinySOL** (Einzeltöne, Saite je
Ton); Übergänge **TU-Note** (nur messen, BY-ND); Vibrato **VSCO 2 CE** (Kontrabass `SusNV`/`SusVib`); Lizenzen in
[aufnahmen.md](aufnahmen.md).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Saite (Spannung, Dichte, Steife, Dämpfung je Mode) | Tonhöhe, Lage der Teiltöne, Abklingen nach dem Absetzen und beim Pizzicato | B1, B2, B6 | Iowa `pizz`, Ausklang `arco` | ±3 Cent; Abklingen je Teilton ±20 % |
| M3 | Expression-Weg (vB, β, F) | Pegel und Spektralschwerpunkt über pp/mf/ff, je Saite | B1, B7 | stationärer Teil, Iowa und TinySOL | ±2 dB je Teilton bis 10.; Schwerpunkt ±10 % |
| M4 | Korpus (Frequenz, Güte, Stärke je Mode) | Stegmobilität → Moden | B6 (Modalfit) nach dem Verfahren von Maestre et al. 2013 (CC BY 3.0, nachgelesen) | CNSM, 100 Hz–6 kHz; Cello gegen Zhang & Woodhouse Tab. II | Frequenz ±2 %, Güte ±20 %, Stärke ±2 dB; hörbar wird erst „about a semitone (6%)“ Verschiebung einer tiefen Mode (Woodhouse 2014, nachgelesen PDF-S. 42) |
| M5 | Ansatz | Dauer bis zur Helmholtzbewegung, Vorläufergeräusch | B5 | Iowa-Einsätze, TU-Note | höchstens „50 ms (≤10 nominal periods …)“ bzw. „90 ms (≤18 periods)“ (Guettler & Askenfelt 1997, nachgelesen) |
| M6 | Vibrato | Rate, Tiefe (Cent), AM je Teilton | B3 | VSCO Kb `SusVib` gegen `SusNV`; Violine VSCO `ArcoVib` | Rate ±5 %, Tiefe ±3 Cent; AM-Muster qualitativ |
| M7 | Rauschen, Springbogen | Restsignal je Band; Sprungrate | B4; B1 | VSCO `spic`, `trem` | ±3 dB je Band; Sprungrate im Bereich 6–35 Hz (Guettler 2002, nachgelesen) |
| M8 | Übergänge, Bogenwechsel | Dauer und Form von Legato/Portamento, Lücke beim Bogenwechsel | B2, B8 | TU-Note Zweitonfolgen; Pàmies-Vilà Bogenwechsel | Übergangsdauer ±20 % |

## B. Am Modell gegen Messdaten des Bogens

| # | Modellgröße | Messgröße | Daten | Toleranz (Vorschlag) |
|---|---|---|---|---|
| M2 | Reibung, Saite ohne Korpus | Schelleng-Fenster (Fmin, Fmax über β), Stegkraft-Verlauf, Einschwingdauer | Lampis, mdw: Roboterbogen am Monochord, Kraft 0,1–4 N, β 0,02–0,2, vb 0,05–0,2 m/s (`cc-by-4.0` nachgelesen; Inhalt nur Bericht) | Grenzen ±20 %; Stegkraft-Form qualitativ |
| M2b | Ansatz | Einschwingvorgänge über Kraft und Beschleunigung (Guettler-Diagramm) | Lampis 13374477 (`cc-by-4.0` nachgelesen) | Keil der guten Ansätze ±25 % |

**Prüfstein vor dem Setzen:** Trifft die Saite mit Reibstelle **ohne Korpus** das gemessene Schelleng-Fenster am
Monochord (M2)? Sonst setzt M3 den Expression-Weg gegen eine falsche Reibung.

## C. Forschungsauftrag

1. Saitendaten Viola und Kontrabass (Spannung, Dichte, Steife) aus Herstellerangaben mit Quelle.
2. Güte der tiefen Kontrabass-Moden (Askenfelt 1982 im KTH-Archiv suchen).
3. Thermische Stoffwerte (Woodhouse 2003).
4. Freie Einzeltöne mit Sordino und Col legno (bisher nur nicht frei: OrchideaSOL).
5. Streuung von Stimmung und Vibrato in einer Streichergruppe.

## D. Anfragen (nicht gestellt)

- mdw Wien (Lampis, Pàmies-Vilà): ob es gespielte Instrumente mit gemessener Bogenkraft und Mikrofon gibt.
- Philharmonia: ob Messwerte, die an den Samples gesetzt werden, unter „made available ‘as is’“ fallen.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Vibrato (Rate und Tiefe aus der Spur); Wolfton aus Korpusmode und Saite — zwei Träger derselben Schwebung.
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
Violine E5, Vibrato-Rate (3–8 Hz, Tiefe 25 Cent): **fest wellig** (231 von 231); **Suche glatt** (B2/B3: Rate 1 : 1, Tiefe 25,6–26,1 Cent). Messgröße: Rate und Tiefe aus der Spur, nicht die Amplitude bei fester Frequenz.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

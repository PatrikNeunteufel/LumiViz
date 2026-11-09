# Messplan — Hammer-Saite (Rest), Klasse 1

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Vorbild: der Messplan des Flügels in LumiViz
> (`modellierung/hammer-saite/messplan.md`, Teil A entschieden S151: UF1 a–UF7 a) — **dieselbe Messung**
> (`Teiltonmessung.hpp`) trägt Upright, Honky-tonk und CP-70 unverändert, denn die Annahmen A1–A6 aus
> [gemeinsam/messkette.md](../../messung/ic/messkette.md) halten für die Hammer-Saite. Toleranzen wie UF5 a (Abklingzeiten
> ±10 %, B ±5 %, Verstimmung ±0,5 Cent, Dämpfer ±20 %), wo nicht anders vermerkt.
> **Kennzeichnung:** nachgelesen · nur Bericht · Einschätzung.

## A. An freien Aufnahmen

Bezug Upright: **K18** (CC BY 4.0, kleine Terzen ab A0, 6 Stufen, Loslassen; nachgelesen); Gegenprüfung: VCSL
Knight (2 Stufen, Ganztöne) und VCSL Yamaha (3 Stufen, C/G) — beide CC0. Honky-tonk: FreePats „Old Piano FB“ (CC0).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz |
|---|---|---|---|---|---|
| M1 | `inharmonicity` B | Teiltonfrequenzen, Ausgleich (f_k/k)² = f0² + f0²·B·k² | wie Flügel (robust gegen fremde Spitzen) | Bass bis Mitte; Diskant wie beim Flügel nach Rigaud | B ±5 % gegen die gesetzte Kurve; Kurve trifft Messpunkte im Median ±10 % (UF5 a) |
| M2 | `decay`, `lateDecay`, `highDecay` | zwei Stufen (Schroeder), σ(ω) = b1 + b3·ω² | wie Flügel | 6 Stufen; K18: Raum unbekannt — späte Stufe mit Vorsicht (Nachhall) | ±10 % |
| M3 | `detune` (Upright); Honky-tonk-Verstimmung | Schwebungsfrequenz der Teiltöne 1–3, Tiefe der Einbrüche (TB1 a) | wie Flügel | Honky-tonk: Schwebungen bis einige Hz — Fenster der Hüllkurven prüfen (Einschätzung) | ±0,5 Cent |
| M4 | Hammer (Q0, p, Masse), Anschlagstelle | Helligkeit (Teilton n zum Grundton) über die 6 Stufen von K18; Einbrüche im Spektrum bei k·x_H | wie Flügel | 6 Stufen (Salamander 16) — Steigung wird gröber | Steigung je Teilton gegen die Rechnung (wie Flügel) |
| M5 | `damper`, `undampedFrom` | Abklingen nach dem Loslassen | K18 hat Loslass-**Geräusche** je Taste, ob mit Ausklang: an der Datei prüfen (beim Flügel: 0,46 s Mechanik ohne Ausklang, D1 a) | — | ±20 % |
| M6 | `bridgeAngle` | Pegel der Phantomteiltöne im Bass bei der höchsten Stufe | wie Flügel | Bass | wie Flügel |
| M7 | Boden (Bauplan P131) | Terzbänder, Verhältnis je Taste gegen das Mittel (UF6 a) | wie Flügel | 30 Hz – 10 kHz | Streuung je Band berichten |

**Prüfstein vor dem Setzen** (Lehre aus 31d-6/31d-7): Die Saitendaten (Länge, Masse) fehlen (F-7) — gesetzt wird
dann durch ein Modell mit geschätzten Saiten; B und Abklingen bleiben messbar, die Helligkeit über die Stufen hängt aber
an der Hammer-Saite-Paarung. Vor dem Setzen prüfen, ob das Modell mit den geschätzten Daten die Einbrüche (Anschlagstelle)
und das zweistufige Abklingen überhaupt zeigt.

## B. Ohne freie Aufnahme

| Instrument | was fehlt | Vorschlag |
|---|---|---|
| CP-70/80 | jede Aufnahme | nur aus Literatur und Modell (Flügel ohne Boden); als „ungemessen“ kennzeichnen (F-6) |
| Clavinet | jede belastbare Aufnahme | Literaturwerte nach Gabrielli et al. 2013 (CC BY 2.0): B je Taste (Tab. 1), T60, Schwebung, −3 Halbtöne beim Loslassen — gegen die Rechnung prüfen, nicht setzen (F-6) |
| Hackbrett | Stärken, Umfang | Freesound iternetcone nur zum Prüfen der Teiltonpaare (3 : 2) an einzelnen Tönen |
| Cimbalom | alles | Forschungsauftrag (unten) |

## C. An einem echten Instrument — Forschungsauftrag

1. **Upright:** Mensur (Länge, Durchmesser, Wicklung je Taste), Steglinie und Bassteg, Rippen — das, was beim Flügel
   RT-0425 lieferte.
2. **Upright-Pedale:** Einzeltöne mit *Soft* und mit *Practice* (Moderator), je drei Stufen.
3. **CP-70:** Ausgang des Vorverstärkers vor der Klangregelung, je Taste fünf Stufen; Lage der Piezos.
4. **Clavinet D6:** Ausgang je Tonabnehmer, fünf Stufen, Loslassen; Kraft der Tangente.
5. **Cimbalom:** Einzeltöne über den Umfang, drei Stufen, mit und ohne Pedal; Saitendaten.

## D. Anfragen nach vorhandenen Daten (nicht gestellt)

- Rudi Fiasco: Hersteller und Raum des K18; readme.txt (6 oder 7 Stufen).
- Fletcher 1964 (Hamilton-Upright) über eine Bibliothek — Mensur je Taste.
- Fleischer & Fastl 2004 (UniBw, Hackbrett); Pap 2000 (Cimbalom, Guild of American Luthiers).

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Schwebung im Chor aus Verstimmung der Saiten **und** Polarisation (wie beim Flügel); Hackbrett/Cimbalom: Chöre und Mitschwingen.
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
Hackbrett C4, Verstimmung im Chor (0,05–4 Cent, Vorgabe 0,3): **fest wellig** (106 von 261 Größen). **Suche nicht bestimmt:** 0,3 Cent ergeben an C4 eine Schwebung von 0,045 Hz (Periode 22 s) — in 3 s ist sie nicht aufgelöst, die Rate steht bis 1,1 Cent am Rand der Suche. **Vor dem Plan ändern:** die Messgröße über mindestens zwei Perioden (Tondauer) oder die Form der Hülle (§3b (2), L3 gültiger Bereich).

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

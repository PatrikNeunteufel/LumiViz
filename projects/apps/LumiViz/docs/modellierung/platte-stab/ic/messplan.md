# Messplan — Platte und Stab (Klasse 4)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag, nichts entschieden. Größen aus
> [modell-entwurf.md](modell-entwurf.md) §5; Bausteine der Messkette aus
> [gemeinsam/messkette.md](../../messung/ic/messkette.md) (A1–A6, B1–B8); Aufnahmen aus [aufnahmen.md](aufnahmen.md).
> Lehre aus dem Flügel: **je Modellgröße die Messgröße, die sie verursacht**, dieselbe Messung an Aufnahme und Modell,
> Setzen durch das Modell hindurch, gültiger Bereich je Messgröße.
> **Kennzeichnung:** nachgelesen · nur Bericht · Einschätzung.

## A. An freien Aufnahmen

Bezug: **Iowa MIS** (Marimba, Xylophon, Vibraphon, Glockenspiel; reflexionsarm ab 60 Hz, pp/mf/ff, mehrere Schlägel;
Lizenz nachgelesen), zum Gegenprüfen **VCSL** (CC0, nachgelesen). Vorarbeit: die Oktavdateien (pre-2012) an den
Einsätzen in Einzeltöne schneiden; Format an der Datei prüfen (Widerspruch 96 / 44,1 kHz).

| # | Modellgröße | Messgröße | Verfahren (Baustein) | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Verhältnisse f_n/f_1 (Biegung), Torsionsmoden | Frequenzen der Moden je Ton | Modalfit ohne Reihe, ESPRIT/ESTER je Band (B6) — **nicht** die Saitenreihe (A1) | Moden mindestens 20 dB über dem Rauschen; unter 60 Hz Raum nicht reflexionsarm (Iowa) | ±5 Cent je Verhältnis (Einschätzung; Kirkland nennt „0.25% error“ als musikalische Toleranz — nur Bericht — ≈ 4,3 Cent) |
| M2 | Dämpfung σ_n je Mode | Abklingzeit je Mode | Schroeder je Mode (wie Flügel) auf der Spur aus B6/B1; bei Resonatorinstrumenten Grundton getrennt (M5) | ab 10 dB Abfall (wie Flügel); Iowa-Töne sind ausgeschnitten — Ende der Datei prüfen (`endeVorBlende`) | ±10 % (wie UF5 a beim Flügel) |
| M3 | Abstrahlgewicht je Mode, Anschlagstelle | Pegel je Mode relativ zur Mode 1, bei Anschlag in der Mitte | Amplituden aus B6 | nur Töne mit bekannter Anschlagstelle — **bei Iowa nicht dokumentiert** (Einschätzung: Mitte üblich) | ±2 dB (Einschätzung) |
| M4 | Schlägel (Masse, Steife), Anschlaggeschwindigkeit je Stufe | Helligkeit (Pegel der oberen Moden zur Mode 1) über pp/mf/ff, je Schlägelart; Pegel über die Stufen | Pegel zu Beginn, Amplituden B6 (wie die Helligkeit beim Flügel) | drei Stufen je Schlägel (Iowa) | Steigung je Mode ±2 dB (Einschätzung) |
| M5 | Resonator: f_a, ξ_a, Kopplung | Abklingen und Pegel des Grundtons gegen die oberen Moden; zwei Stufen im Grundton (Kopplung Stab–Rohr) | B6 auf dem Grundtonband mit zwei Polen (Stab, Rohr) | Marimba, Vibraphon, Xylophon; ohne Aufnahme ohne Resonator nur relativ | T60 des Grundtons ±10 % |
| M6 | Motor: Drehzahl, Tiefe | Rate und Tiefe der Modulation des Grundtons (AM und FM) | Hüllkurve des Grundtons (B1), Modulationsspektrum (B3) | **nur, wenn eine Aufnahme mit laufendem Motor da ist** — Iowa dokumentiert den Motor nicht | Rate ±5 %, Tiefe ±1 dB (Einschätzung) |
| M7 | Dämpfer (σ_D) | Abklingen nach dem Abdämpfen | Iowa Vibraphon „Dampen“ / „Short Sustain“ (Schroeder ab dem Dämpfen) | Vibraphon | ±20 % (wie Dämpfer beim Flügel) |
| M8 | Rhodes/Wurlitzer: Hammer, Tonabnehmer | Helligkeit und Klirr (Verhältnis der Harmonischen 2, 3 zur Grundschwingung) über die Stärke | B1 (Teiltöne mit fester Frequenz genügen; Tine fast harmonisch) | **keine kommerziell nutzbare Aufnahme mit Stärken** — jRhodes3d nur NC (F-7) | offen |

**Prüfstein vor dem Setzen** (Lehre aus 31d-6/31d-7): Kann das Modell die beobachteten Erscheinungen überhaupt
hervorbringen? Für Stabspiele vor allem: zweistufiges Abklingen des Grundtons (Stab + Rohr) braucht die Kopplung aus
[modell-entwurf.md](modell-entwurf.md) §3; Torsionsmoden im Spektrum brauchen Torsion im Modell — sonst setzt der
Ausgleich falsche Dämpfungen.

## B. An einem echten Instrument — Forschungsauftrag

Ziel: was frei nirgends zu haben ist (Befund IC1), an je einem Instrument messen und frei veröffentlichen (etwa CC BY
4.0).

1. **Vibraphon mit Motor:** Einzeltöne über den Umfang, pp/mf/ff, Motor aus (Klappen offen) und bei drei Drehzahlen,
   Drehzahl mitgeschrieben; Pedal halb.
2. **Modalanalyse von Stäben:** je Oktave ein Stab von Marimba und Vibraphon frei gelagert, Hammer und Laser- oder
   Beschleunigungsaufnehmer: Frequenzen, Dämpfungen, Formen der ersten Biege- und Torsionsmoden; Unterschnittprofil
   vermessen.
3. **Schlägel:** Kontaktzeit und Kraft je Schlägelart bei drei Geschwindigkeiten (Kraftsensor im Stab).
4. **Resonator:** Rohrlänge, Durchmesser, Abstand zum Stab je Note; Antwort mit und ohne Rohr.
5. **Rhodes/Wurlitzer:** Ausgang am Harfenanschluss vor jedem Verstärker, je Taste fünf Stärken; Abstand Tine–
   Tonabnehmer.

Mögliche Partner (nicht angefragt): Arbeitsgruppen, die an Stabspielen gemessen haben (Scavone/McGill, Antunes/Lissabon,
Budapest — Rucz).

## C. Anfragen nach vorhandenen Daten (nicht gestellt)

- Jeff Learman: Erlaubnis, jRhodes3d (CC BY-NC) zum Messen für ein gerechnetes Rhodes zu nutzen (F-7).
- Malloy, Tzanetakis (University of Victoria): dasselbe für SASS-E (CC BY-NC 4.0) (F-7).
- University of Iowa EMS: ob beim Vibraphon der Motor lief.

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Tremolo des Vibraphon-Motors (Rate); Resonator-Stimmung gegen Stab-Mode (Schwebung).
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
Vibraphon F4, Motor (1–10 U/s): **fest wellig** (253 von 261); **Suche glatt** (Rate = 2 × Drehzahl, 2–20 Hz; Tiefe 23–31 dB). Messgröße: Rate und Tiefe der AM.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

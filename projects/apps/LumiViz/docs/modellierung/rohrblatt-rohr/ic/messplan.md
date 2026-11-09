# Messplan — Rohrblatt und Rohr (Klasse 8)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine aus [gemeinsam/messkette.md](../../messung/ic/messkette.md). **Kennzeichnung:** nachgelesen · nur Bericht ·
> Einschätzung.

## A. An freien Aufnahmen

Bezug: **Iowa MIS** (Klarinette, Altsax, Oboe, Fagott; pp/mf/ff; nachgelesen); Gegenprüfung **TinySOL**, Tenorsax
**Good-Sounds**, Vibrato **VSCO 2 CE** / **VCSL** (Lizenzen siehe [aufnahmen.md](aufnahmen.md)).

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | Bohrung, Tonlöcher | Tonhöhe je Griff (ohne Vibrato) | B2 | Iowa `novib` | ±5 Cent |
| M2 | Rohrblatt, Expression-Weg (Druck, Lippe) | Spektrum und Pegel über pp/mf/ff; Spektralschwerpunkt (Almeida 2013: „a very strong correlation between sound level and spectral centroid“, nur Bericht) | B1 | stationärer Teil | ±2 dB je Teilton bis 10. |
| M3 | Zunge, Einsatz | Anstiegsrate in dB/s, Einsatzzeit | B5 | gespielte Einsätze (Iowa) | Rate innerhalb des Bereichs „several tens to several hundreds of dB s−1“ (Li 2016, nur Bericht) |
| M4 | Vibrato-Umsetzung | Rate, Tiefe (Cent), AM je Teilton | B3 | Altsax `vib`, Oboe/Fagott (VSCO) | Rate ±5 %, Tiefe ±3 Cent |
| M5 | Rauschanteil | Restsignal nach Abzug der Teiltöne, Pegel je Band | B4 (Iowa `ambient.silence` als Raumboden) | Klarinette | ±3 dB je Band |
| M6 | Vokaltrakt (Bend) | Bend-Umfang je Register | B2 | keine freie Aufnahme mit Bend | offen |

**Prüfstein vor dem Setzen:** Trifft das Modell mit der Rohrblatt-Vorgabe die Schwelle (Klarinette rund 2–4 kPa, nur
Bericht) und die Tonhöhe je Griff (M1)? Sonst setzt M2 das Rohrblatt gegen ein falsches Rohr.

## B. Messdaten mit Druck

- **MIPCAT** (Klarinette, CC BY 4.0, nur Bericht): Blasdruck, Blattposition, Zungenkontakt — setzt Expression-Weg und
  Zunge am Spieler.
- Fuks & Sundberg (Tabellen im Aufsatz, nur Bericht): Druckbereiche je Instrument als Prüfstein der Expression-Kurve.

## C. Forschungsauftrag

1. Bohrung und Tonlöcher von Altsaxophon, Oboe, Fagott (modern).
2. Einzeltöne Tenorsaxophon pp/mf/ff reflexionsarm; Legato-Ketten; Crescendo.

## D. Anfragen (nicht gestellt)

- UNSW: Lizenz der Impedanzdaten (Klarinette, Saxophon).
- MTG/UPF: welche Lizenz für Good-Sounds gilt (Zenodo CC BY 4.0 gegen Freesound CC BY 3.0).

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Register (Überblasen: Blatt gegen Rohr), Vibrato der Spieler.
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
Klarinette C4, Vibrato-Rate (3–8 Hz, verlangt 25 Cent über den Druck): **fest wellig** (231 von 231). **Suche:** die Rate folgt 1 : 1, aber die Tiefe ist nur **0,10–0,17 Cent** — das Vibrato über den Druck trägt am Modell fast keine FM; die Tiefe ist dadurch verrauscht, Σ r² **nicht glatt** (2 Minima). Vor dem Plan: das Vibrato der Klarinette (Druck oder Kiefer, T3a a) am Modell klären.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

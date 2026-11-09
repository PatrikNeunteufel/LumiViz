# Messplan — Gezupfte Saite (Deluxe) (Klasse 2)

> **Stand:** 2026-10-26, IC Session 2. **Typ:** Vorschlag. Größen aus [modell-entwurf.md](modell-entwurf.md) §4;
> Bausteine B1–B8 aus [gemeinsam/messkette.md](../../messung/ic/messkette.md); in LumiViz vorhanden: die Teiltonmessung
> des Flügels (`SCHNITTSTELLE.md`) und die Messungen an IDMT-SMT (S134–S136). **Kennzeichnung:** nachgelesen · nur
> Bericht · Einschätzung.

**Lage:** Der Korpus der Nylongitarre lässt sich **direkt** an Messungen setzen (Mores, Stegmobilität über 60 Gitarren,
CC BY 4.0) und der Klang an Iowa (schalltot, pp/mf/ff). Cembalo und Harfe haben VCSL (CC0, raumbehaftet). Stahl,
Laute/Oud und Koto haben keine freie Korpusmessung; Oud und Koto auch keinen Klang. Lizenzen in [aufnahmen.md](aufnahmen.md).

Gezupfte Töne klingen ab — die Teiltonmessung des Flügels (Einsatz, f0, Inharmonizität, Hüllkurven, zwei Abklingstufen,
Schwebung) passt hier fast ganz (Einschätzung). Neu ist die **Messung am Korpus selbst** (M1) statt am Ton.

| # | Modellgröße | Messgröße | Verfahren | gültiger Bereich | Toleranz (Vorschlag) |
|---|---|---|---|---|---|
| M1 | 2×2-Admittanz am Steg (Moden, Residuen) | Stegmobilität senkrecht und parallel | B6 (Modalfit ohne Reihe) an `qualified_selected_impulses.mat`; Passivität erzwingen wie Bank & Karjalainen | Mores, Nylon; 70 Hz – 2 kHz (Einschätzung) | Frequenz ±1 %, Güte ±20 %, Betrag ±2 dB |
| M2 | Abstrahlung je Mode | Nahfeld je Mode gegen Stegmobilität | Übertragungsfunktion Steg → Mikrofon | Mores, drei Nahfeldkanäle | ±3 dB |
| M3 | Polarisationswinkel, Doppelchor-Verstimmung | Schwebung und zweistufiges Abklingen je Teilton | Teiltonmessung (Schwebung, zwei Abklingstufen), B1 | Iowa Nylon; VCSL Harfe/Cembalo | Abklingstufen ±20 %, Schwebungsrate ±10 % |
| M4 | Saite (Länge, Zug, Durchmesser) | f0 und Inharmonizität B je Ton | Teiltonmessung | Iowa; VCSL | ±1 Cent; B ±10 % |
| M5 | Kiel bzw. Finger über die Stärke; Tastenstoß | Teiltonpegel und Schwerpunkt je Stärke; Pegel < 300 Hz | B1, B7 | Iowa pp/mf/ff; VCSL Harfe f/mf; Cembalo nur ein Pegel (VCSL) | ±1,5 dB je Teilton; Prüfstein Penttinen (5/3/0,5 dB an C4) |
| M6 | Loslassgeräusch Cembalo | Pegel und Spektrum des Release | B4, B7 | VCSL Releases (getrennt) | ±3 dB |
| M7 | Resonanzboden Cembalo (Bauplan) | Abklingen am Steg; tiefste Luftmode | Prüfung gegen Literatur | Dulcken: T60 235 ms, Helmholtz 37 Hz (nachgelesen) | T60 ±20 %, Helmholtz ±5 % |
| M8 | Register und Zupfpunkt (Cembalo) | Kammfilter-Lücken im Spektrum je Register | B1 | VCSL English Lute/Normal, Flemish High/Low | Lage der Lücke ±1 Teilton |
| M9 | Anschlagort (Stahl) | Kammfilter-Lücken bei „Pick Near Bridge“ gegen „Over the Soundhole“ | B1 | AG-PT-set | wie M8 |

**Prüfstein vor dem Setzen:** Gibt ein Korpus aus M1, an die heutige Waveguide-Saite gehängt, bei einer Nylonnote die
Schwebung und das zweistufige Abklingen, die Iowa an derselben Saite zeigt (M3)? Erst dann werden Saite und Anschlag
gesetzt. Zweiter Prüfstein: *Classical* (Wright TOP26) als Fall des neuen Korpus — dieselben gekoppelten Spitzen wie heute
(115,4 / 191,3 / 220,0 Hz …, LumiViz §5f).

**Neu für die Messkette** (Einschätzung): ein Modalfit an **Stegmessungen** (Impuls, zwei Kanäle) statt an Tönen — B6
liefert ihn, braucht aber eine Passivitätsbedingung für die Residuenmatrizen.

## Forschungsauftrag

1. Stahlsaitengitarre (Dreadnought): Stegmobilität in zwei Richtungen (Impulshammer, Drahtbruch wie Bank & Karjalainen)
   und trockene Einzeltöne pp/mf/ff.
2. Harfe: A0/T1 und Stegmobilität an einigen Punkten der Saitenlinie; Saitentabelle eines Herstellers mit Erlaubnis.
3. Koto und Oud: Einzeltöne je Saite, Oshi-de in Stufen, Risha-Tremolo; Stegmobilität.

## Anfragen (nicht gestellt)

- Ducceschi/Russo/Webb (NEMUS): ob die Modalparameter der 65 Gitarren (DAFx-2026) veröffentlicht werden.
- Le Carrou (Sorbonne, LAM): Messdaten zur Harfe (A0/T1, Mitschwingen).
- Bader, Plath, Kontopidis (arXiv 2511.09037): Impulsantworten des Dulcken-Cembalos (104 Stegpunkte, nach dem Bericht).

## Feinabstimmung und Ausstieg (L15–L18; T9 a, F-B6 a — IC Session 4, 2026-10-30)

Gilt für das Setzen an Aufnahmen (lokal); geplant **vor** dem ersten Lauf, nicht im Nachhinein.

1. **A — Suche in Ordnung:** je Note eigener Löser (L8), Wirkung je Parameter vorab, wirkungslose fest (L9), Schnitt
   je Parameter an wenigen Noten (L14), drei Startpunkte je Note. **Glätte des Ziels** vor dem Plan prüfen
   (`ic::setzen::welligkeit`, L16): ist eine Messgröße in einem Parameter wellig, wird die Messgröße geändert, nicht die
   Suche; Parameter mit derselben Wahrnehmung tragen dasselbe `traegt` und werden nicht einzeln gestaffelt.
   **Heikel in dieser Klasse (Einschätzung):** Schwebung aus den zwei Polarisationen; Doppelchöre (Laute, Oud) tragen dieselbe Schwebung mit.
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
Laute A3, Verstimmung im Doppelchor (0,5–20 Cent, Vorgabe 4): **fest wellig** (246 von 261). **Suche** folgt der Verstimmung erst ab rund 5 Cent (0,62–2,53 Hz); darunter in 3 s nicht aufgelöst — **nicht glatt**. Vor dem Plan wie Klasse 1: Tondauer oder Form der Hülle.

**Anläufe:** einer, und genau ein zweiter nach A–D; Schwellen stehen vorher fest. **Ausstieg (L18):** verfehlt der
zweite Anlauf die Probe, endet das automatische Setzen, nicht das Messen — die Ergebnisse der Suche bleiben
**Vorschlag** und gehen als Hörproben an Patrik, der nach Gehör setzt; „Vorschlag“ und „gesetzt“ werden getrennt
geführt.

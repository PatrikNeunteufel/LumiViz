# Modell-Entwurf — Rohrblatt und Rohr (Klasse 8)

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf. Rahmen: Rohr-Kern [gemeinsam/rohr.md](../../rohr/ic/rohr.md),
> Fragen F-R1 bis F-R5 in [gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md), Bestand in LumiViz (IC-B1 a).
> Bericht: `recherche/2026-10-26_physik_spielweisen.md`. **Kennzeichnung:** **nachgelesen** · **nur Bericht** ·
> **Einschätzung** · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand

Nachgelesen, `Spielweisen_Konzept.md`: SF10 a — Saxophon Rohrblatt × Rohr; SF6 a — Rohrblatt und Rohr: Expression ja,
Vibrato ja, Pitch ja („Bend, Glissando“); SK4 a — Expression als Stärke der Anregung (Blasdruck) samt Klangfarbe;
Zielbild — Holzbläser: „Zungenstoß einfach/doppelt, Atem- und Luftgeräusch, Flatterzunge“ (je Note nach SF8 a); Modell
28a — Vibrato „in Cent 0 … 100“.

## 2. Kern

1. **Rohr vorab je Griff** (Rohr-Kern): Bohrung mit Tonlöchern (Keefe-Modell als T-Glied, nur Bericht) → Moden am
   Eingang; Klarinette zylindrisch, Saxophon/Oboe/Fagott konisch mit Mundstück- bzw. Hülsenvolumen.
2. **Rohrblatt je Note im Zeitbereich:** Masse-Feder mit einem Freiheitsgrad, Anschlag an der Bahn als
   Kontaktkraft, Fluss nach Bernoulli durch den Spalt (Chatziioannou et al. 2019: Hunt–Crossley-Kontakt an Bahn und
   Zunge, nur Bericht). Doppelrohrblatt (Oboe, Fagott): quasistatische Kennlinie mit Druckrückgewinn (Almeida et al.
   2007, nur Bericht).
3. **Legato:** Moden je Griff, beim Wechsel überblendet — „based on linear interpolation of input impedance and sound
   pressure radiation models“ (Maestre, Scavone & Smith, DAFx-18, nachgelesen S. 1); eine Kette ist ein Ton (F-R4).
4. **Zungenstoß:** Zunge als zweiter Kontakt am Rohrblatt (Chatziioannou 2019: Bahn der Zunge stückweise linear, nur
   Bericht); Prüfstein: Anstiegsraten bis „730 dB s−1“ (Li et al. 2016, nachgelesen PDF-S. 5).
5. **Atemgeräusch:** Rauschen moduliert vom Rohrblattspalt (Smith/PASP, STK-Praxis, nur Bericht) — kein belegtes
   physikalisches Modell gefunden.
6. **Vokaltrakt** (F-R5): Null im Normalfall; für Saxophon-Altissimo und Bend (Chen et al. 2011: „in normal playing …
   no consistent tuning“, nachgelesen PDF-S. 1).

## 3. Je Instrument

| Instrument | Rohr | Rohrblatt | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|---|
| **Klarinette** | zylindrisch; Tonlochtabelle Buffet Crampon (Debut et al., Tab. 3 — **vor Übernahme am Original prüfen**, nur Bericht) | Einzelblatt; Werte Chatziioannou Tab. I (m 3,25 mg, k 1765 N/m, …, nur Bericht) | gut | gering |
| Altsaxophon | Kegel + Mundstück; Geometrie **offen** (nur Sopran/Tenor bei Chen 2009: Halbwinkel 1,74° bzw. 1,52°, nur Bericht) | Einzelblatt | Impedanzen je Griff (Maestre), Geometrie fehlt | mittel |
| Tenorsaxophon | wie Alt; Länge 1490 mm (nur Bericht) | Einzelblatt | Aufnahmen dünn | mittel |
| Oboe | Kegel, Halbwinkel 0,71° (nur Bericht); Geometrie **offen** | Doppelrohrblatt, p_M ≈ 33 kPa (nur Bericht) | schwach | mittel–hoch |
| Fagott | Kegel 0,41° (nur Bericht); lange geschlossene Kamine mit Resonanzen nahe 2 kHz (Ernoult & Grothe 2023, nur Bericht); moderne Geometrie **offen** | Doppelrohrblatt, Ruheöffnung ~7 mm² (nur Bericht) | schwach | hoch |

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Bohrung und Tonlöcher | fest (Bauplan je Griff) | Klarinette Debut Tab. 3 (nur Bericht); übrige offen | Impedanz (UNSW Klarinette/Saxophon, ohne Lizenz — nur lokal) |
| Rohrblatt: Masse, Steife, Dämpfung, Ruheöffnung, Bahn | Kurve (Lage) | Chatziioannou Tab. I (nur Bericht) | Spektrum über pp/mf/ff (M2) |
| Blasdruck über Expression | Stärke (F-R2) | Klarinette 2,0–5,4 kPa, Altsax 1,5–7,8, Oboe 3,4–12,2, Fagott 1,2–8,8 (Fuks & Sundberg, nur Bericht, umgerechnet) | Pegel und Schwerpunkt (M2) |
| Lippenkraft über Expression | Stärke (F-R2) | Schwelle 4 → 2 kPa mit steigender Lippenkraft (Almeida 2013, nur Bericht) | Spektrum (M2) |
| Zunge (Kraft, Bahn) | fest je Artikulation | Chatziioannou Tab. I; Li 2016 (Zungenkraft 0,05–0,15 N, nur Bericht) | Anstiegsrate (M3) |
| Vibrato-Umsetzung | Spur (F-R3) | Druckmodulation um 5 Hz (Oboe, Almeida 2016, nur Bericht) | Rate, Tiefe, AM (M4) |
| Rauschanteil | Kurve | offen | Geräuschanteil (M5) |
| Vokaltrakt | (F-R5) | Chen 2009/2011 (nur Bericht) | Bend-Umfang (M6) |

## 5. Rechenzeit

„above 30 times faster than real-time in one core of a laptop computer“ mit im Mittel 32 Resonatoren je Griff bei
48 kHz und Griffwechseln (Maestre et al., nachgelesen S. 7) — also rund 3 % der Echtzeit, unter der Grenze des Flügels.
Mit Rohrblatt-Kontakt (Überabtastung im Anschlag) etwas mehr (Einschätzung).

## 6. Abweichungen vom Rahmen

- Legato als Kette (F-R4), Vibrato als Folge der Druckmodulation (F-R3), Expression als Weg in (Druck, Lippe) (F-R2).
- Bend am Saxophon/Klarinette über den Vokaltrakt (F-R5); SF6 a nennt „Bend“ als Pitch — physikalisch bewegt der Spieler
  den Trakt, nicht das Rohr.

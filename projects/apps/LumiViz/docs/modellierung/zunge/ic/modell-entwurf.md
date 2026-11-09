# Modell-Entwurf — Durchschlagende Zunge (Klasse 10)

> **Stand:** 2026-10-26, IC Session 2. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Bestand in LumiViz (IC-B1 a), gemeinsame Fragen F-R2, F-R3, F-R5 in [gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md).
> Berichte: `recherche/2026-10-26_physik_rechenverfahren.md`, `…_spielweisen_steuerung.md`, `…_aufnahmen.md`.
> **Kennzeichnung:** **nachgelesen** (Seite) · **nur Bericht** · **Einschätzung** · **eigene Rechnung** · **offen**.
> Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand in LumiViz

Nachgelesen, `Spielweisen_Konzept.md` (2026-10-26):
- **SF10 a** nennt als Anregungen „Zupfen, Hammer, Schlägel, Bogen, Lippe, Rohrblatt, Luftstrahl“ und als Schwinger
  „Saite, Platte/Stab, Rohr, dazu das **Tonrad**“. Die durchschlagende Zunge kommt **nicht** vor — weder als Anregung
  noch als Schwinger noch in der Tabelle der Instrumente. Regel dort: „Expression, Vibrato, Pitch nur bei dauernder
  Anregung“. → F-1.
- **SK4 a**: Expression „ein Modell mit dauernder Anregung die Stärke der Anregung (Blasdruck, Bogendruck) samt
  Klangfarbe“.
- **SF8 a**: drei Orte (nie → Ausstattung; über eine Passage → Spur mit Umschaltstellen, etwa „Register“; je Note →
  Artikulation, etwa „Tremolo“).
- **SF5 d**: fünf Spuren Sustain, Sostenuto, Expression, Vibrato, Pitch; Expression zusätzlich am Clip.
- Im Zielbild der Familien steht keine Zungenfamilie; mitgeliefert ist kein Zungeninstrument (`KLASSEN.md`: „— (Pads)“;
  unter `asset/loop/instruments` nur `organ`, `house_organ` als Orgeln, nachgelesen per Dateiliste).

In diesem Projekt: Klasse 4 führt den **Wurlitzer** als „Zunge (Kragbalken, Lötzinn zum Stimmen)“ mit Filzhammer
(`klassen/Klasse 4 - Platte und Stab/modell-entwurf.md`) — dieselbe Zungenform, nur anders angeregt. Klasse 8 rechnet das Rohrblatt
als Masse-Feder mit Bernoulli-Fluss (`klassen/Klasse 8 - Rohrblatt und Rohr/modell-entwurf.md` §2).

## 2. Physik in einem Satz je Teil

- **Zunge:** ein Kragbalken (Messing oder Stahl), der durch einen Schlitz der Stimmplatte **durchschlägt**; die Tonhöhe
  setzt die Zunge selbst, nicht ein Rohr. Sie schwingt „somewhat below the natural vibrating frequency of the reed“
  (Cottingham 2011, nachgelesen PDF-S. 2) mit einer Spitzenamplitude von „15% of the tongue length“ (ebd., PDF-S. 2).
- **Anregung:** Bernoulli allein schwingt nicht an — „this mechanism in isolation cannot develop the asymmetric pressure
  differences“; Treiber ist „the inertial effect of the upstream air flow“ (St. Hilaire, wiedergegeben bei Puranik &
  Scavone 2023, nachgelesen S. 1809). Das ist der Unterschied zum Rohrblatt (Klasse 8), wo das Rohr die Rückkopplung
  liefert.
- **Klang:** „abundant higher harmonics, as the approximate square waveform would suggest“ (Cottingham 2011,
  nachgelesen PDF-S. 3); Akkordeon 8′ ohne Cassotto mit Obertönen „approximately up to 10000 Hz“ (Elejalde-García et
  al. 2021, nachgelesen S. 3). Beim Harmonium bestimmt das Gehäuse die Farbe stärker als die Quelle: „the filter has a
  much stronger influence on the synthesized sound timbre than the source“ (Puranik & Scavone, DAFx23, nachgelesen
  PDF-S. 3, Seite „DAFx.3“).
- **Kanzelle:** wirkt meist schwach, „the cavity volumes are small and the chamber resonances have high frequencies“
  (Cottingham, ISMA 2017, nachgelesen S. 2).

## 3. Kern

1. **Zunge als Moden eines Kragbalkens** — wie der Stab in Klasse 4. Für den Normalfall reicht die **erste Biegemode**
   als Masse-Feder-Dämpfer (Millot & Baumann 2007: „equivalent mass-spring-damper oscillator with a single degree of
   freedom“, nur Bericht); Werte für eine Akkordeon-C4-Zunge: f₀ 267,6 Hz, Masse „(2/3) × 0.248 g“, ζ 0,003, Länge 32,8
   mm, Breite 3,45 mm (Adachi et al. 2026, Tab. 1, nachgelesen PDF-S. 4). Bei hohem Druck kommen Obertöne der Zunge
   selbst dazu: Puranik & Scavone rechnen den Balken verteilt („a grid with N = 40 sections“, „4 × 44100 Hz“,
   nachgelesen S. 1813) und finden „predominantly sinusoidal motion at low bellows pressures“ mit Obertönen bei hohem
   Druck (ebd.) → verteilter Balken als **Deluxe** (Einschätzung). Im Einschwingen treten zweite Biegemode und
   Torsion auf (Cottingham & Wolff 2015, ISMA 2017, nachgelesen Abstracts) — offen, ob hörbar.
2. **Strömung:** Durchflussfläche über Spitze und Seitenspalt, symmetrisch zum Durchschlagspunkt (Puranik & Scavone,
   Gl. auf S. 1812, nur Bericht); Massenerhaltung im Vorvolumen V₁ und Trägheit L₂/S₂ der Strömung (ebd., Gl. 1–5,
   nur Bericht); Quelldruck p₀ aus der Spur Expression (F-2).
3. **Kanzelle und Vorvolumen** als Helmholtz-Mode (Einschätzung) — dieselbe Kopplung „Anreger ↔ Moden des Resonators“
   wie im Rohr-Kern (`gemeinsam/rohr.md`), nur mit einer oder zwei Moden. Bei der Mundharmonika kommt der **Vokaltrakt**
   davor (F-R5, F-6).
4. **Paarzungen und Chöre:** je Note und Richtung eine Zunge; je eingeschaltetem Chor eine weitere, gegeneinander
   verstimmt (Musette). Gerechnet je Zunge und summiert — die Zungen eines Tons sind nur über die gemeinsame
   Windversorgung schwach gekoppelt (Einschätzung; gemessene Kopplung nicht gefunden).
5. **Gehäuse/Abstrahlung:** beim Harmonium ein Filter mit 8–9 Spitzen, approximiert als „10 second order sections“
   (DAFx23, nachgelesen PDF-S. 3); beim Akkordeon Cassotto (beeinflusst die Tonhöhe nicht, Adachi et al., nur Bericht),
   Verdeck, Balg. Ort: Körper der Klasse 0 (`gemeinsam/koerper.md` Zeile 10: „offen (nicht gesucht)“) — **offen**.
6. **Bending** (Akkordeon, Melodica): Druckverlust am halb offenen Ventil lässt den Kanzellendruck im Takt schwanken —
   „a decrease in the effective spring constant“ (Adachi et al., nachgelesen PDF-S. 1); Gradient „−2.5 Hz/kPa“ bei
   offener, „−5.1 Hz/kPa“ bei kleiner Öffnung (ebd., nachgelesen PDF-S. 2). Eine Größe „Ventilöffnung“ (Kp im Modell)
   kommt dazu (F-2).
7. **Mundharmonika-Bending:** Blas- und Ziehzunge derselben Kanzelle wirken zusammen: „the draw reed speaks initially as
   a closing reed but the blow reed takes over as an opening reed“ (Cottingham 2013, Folien, nachgelesen PDF-S. 12);
   „the secondary reed always oscillates“ (Cottingham 2011, nachgelesen PDF-S. 5); entscheidend ist „the configuration of
   the vocal tract, not just its volume“ (ebd., PDF-S. 4). Zwei gekoppelte Zungen plus Vokaltrakt (F-6).

**Rechnung:** je Note im Zeitbereich mit ihrer Spur (Expression → p₀), wie die Bläser — nicht vorab ohne Spur, weil
Druck, Tonhöhe und Farbe zusammen über die Note laufen. Der Ton passt in den Tonspeicher wie ein Bläserton (Einschätzung).

**Z1 (Moden und Waveguides):** kein Waveguide-Modell der durchschlagenden Zunge gefunden (Bericht Physik §7 und §8,
Reichweite dort). Da kein langes Rohr mitschwingt, gibt es für einen Wellenleiter keinen Ort; die Resonatoren
(Kanzelle, Vokaltrakt) sind klein und modal (Einschätzung). Keine eigene Frage.

## 4. Je Instrument

| Instrument | Zunge, Wind | Ausstattung (SF8 a „nie“) | Datenlage | Risiko (Einschätzung) |
|---|---|---|---|---|
| **Akkordeon** (Piano/Knopf, unisonorisch) | Stahlzungen; Balg 0–2,5 kPa im Versuch (Adachi, Achse Fig. 5, nur Bericht) | Chöre 16′/8′/4′, Musette-Stimmung, Cassotto, Bass-System (Stradella) | TinySOL CC BY 4.0, drei Stärken; Zungenwerte C4 (Adachi) | gering–mittel |
| Bandoneon, diatonisches Akkordeon (bisonorisch) | wie Akkordeon; Richtung bestimmt den Ton | dazu Tonbelegung je Richtung | keine eigene Aufnahme | mittel |
| **Harmonium** (Hand-, indisch) | Messingzungen; Kanzelle 0,2–1,5 kPa „in a real harmonium“ (Puranik & Scavone, nachgelesen S. 1810); B5-Zunge 16 × 2 × 0,4 mm, ρ 8490 kg/m³, E 125 GPa (ebd. Tab. 1, nachgelesen S. 1813) | Register, Drones, Gehäusefilter | Euterpea CC0, eine Stärke; Gehäusefilter DAFx23 | gering |
| Harmonium (europäisch, Druckwind; amerik. Saugwind) | Pedalbalg; Expression-Register, Prolongement, Percussion (Grove, nur Bericht) | Register, Expression-Register, Percussion | keine Aufnahme, keine Druckwerte | mittel |
| **Mundharmonika** diatonisch | Blas- und Ziehzunge je Kanzelle, Vokaltrakt | Stimmung (Richter), Tonart | VCSL nur Blastöne; Bends ohne freies Material | **hoch** (Bending) |
| Mundharmonika chromatisch | Schieber, Windklappen | wie oben | VCSL Super 64 Blastöne | mittel |
| **Melodica** | Atem über Mundstück/Schlauch; Ventil je Taste (Wikipedia, nur Bericht) | Tastatur 2–3 Oktaven | fese CC BY 4.0, eine Stärke; keine Messwerte | mittel |

## 5. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| Eigenfrequenz der Zunge f₀ | Kurve (Taste) | aus der Stimmung, Arbeitsfrequenz „somewhat below“ f₀ (nachgelesen) | Tonhöhe bei mf (M1) |
| Zungenmaße L, b, h, Material | Kurve | C4 Akkordeon 32,8 × 3,45 mm (nachgelesen); B5 Harmonium 16 × 2 × 0,4 mm Messing (nachgelesen); über den Umfang **offen** — Faustregel f ∝ h/L² (eigene Rechnung, §6) | f₀ und Spektrum (M1, M2) |
| Masse, Steife (erste Mode) | Kurve | aus Maßen oder direkt: C4 „(2/3) × 0.248 g“ (nachgelesen) | f₀ (M1) |
| Dämpfung | Kurve | C4 ζ = 0,003 (nachgelesen); Mundharmonika Q = 95 (Millot & Baumann, nur Bericht) | Ausklingen nach Druckende (M5) |
| Spalt / Ruhelage | Kurve | C4 a = 0,14 mm, x_rest = 0,3 mm (Adachi Tab. 1, nur Bericht); B5 y₀ = −0,1 mm (nur Bericht) | Ansprache, Schwelle (M4) |
| Vorvolumen V₁, Trägheit L₂/S₂ | Kurve | Harmonium V₁ = 0,0013 m³, L₂ = 50 mm, S₂ = 6,4·10⁻⁵ m² (Puranik & Scavone Tab. 1, nur Bericht); Kanzelle Akkordeon 14,8 × 14,6 mm² (nur Bericht) | Ansprache (M4), Tonhöhe über Druck (M3) |
| Quelldruck p₀ über Expression | Stärke (F-2) | Harmonium 0,2–1,5 kPa (nachgelesen); Akkordeon 0–2,5 kPa nur Laborachse (nur Bericht); **Spielerdrücke offen** | Pegel und Schwerpunkt über pp/mf/ff (M2) |
| Tonhöhe über Druck | folgt aus dem Modell | Akkordeon −2,5 Hz/kPa ≈ −16 ct/kPa (nachgelesen, Umrechnung eigene Rechnung); Harmonium „typically less than 1-2 Hz“ (nachgelesen) | f₀ über pp/mf/ff (M3) |
| Ventilöffnung beim Bend (Kp) | Spur (F-2) | Kp 35 → −2,5 Hz, Kp 150 → −5,9 Hz/kPa (Adachi, nachgelesen PDF-S. 3); Spieler 10–35 ct (Elejalde-García, nachgelesen S. 3); Coyle „Max Bend = 2.5 Hz (1/2 of a semitone)“ (nachgelesen PDF-S. 14) | Bend-Tiefe (M6) — Aufnahme fehlt |
| Musette-Verstimmung | Kurve (Ausstattung, F-4) | Schwebung „between 0.5 Hz and 15 Hz“, „a gradual rise upon pitch is considered desirable“ (Hergert, nachgelesen S. 4455); Stile „Δf = 0.5…7 Hz at A4“ (ebd. S. 4457); Beispiel A4 2,00 Hz = 7,9 ct (ebd. S. 4460) | Schwebung (M7) |
| Chöre (Fußlage, Pegel) | Ausstattung, Registerwahl Spur (F-4) | 16′/8′/4′ (Wikipedia, nur Bericht) | Commons „Accordion registers“ (M8) |
| Gehäuse-/Registerfilter | fest | Harmonium: 10 Biquads (nachgelesen); Akkordeon **offen** | LTAS gegen Aufnahme (M2) |
| Vokaltrakt (Mundharmonika) | Spur abgeleitet (F-6) | **offen** (Bahnson 1998, Adachi JASA 2026 nicht gelesen) | Bend-Tiefe, Overblow (M6) — Aufnahme fehlt |
| Einschwingen (Torsion, 2. Mode) | fest | **offen** (nur Abstracts) | Einsatz (M4) |

## 6. Rechenzeit und Plausibilität

Skript `zungenabschaetzung.py` (Scratchpad der Sitzung, nicht im Projekt; **eigene Rechnung, kein Beleg**):

```python
def f1_kragbalken(L, b, h, E, rho):
    I = b * h**3 / 12.0; A = b * h
    return (1.875104**2 / (2 * math.pi * L**2)) * math.sqrt(E * I / (rho * A))
# Harmonium B5 (Puranik & Scavone Tab. 1): 16 mm, 2 mm, 0,4 mm, 8490 kg/m3, 125 GPa
# Akkordeon C4 (Adachi Tab. 1): 32,8 mm, 3,45 mm, Stahl 200 GPa / 7800 kg/m3 angenommen, Dicke gesucht
# Aufwand: Masse-Feder + Strömung ~107 Operationen je Schritt bei 4 x 48 kHz; Balken N = 40 ~1060
```

Ergebnis:
- Harmonium B5: f₁ = **969 Hz** gegen gemessen ≈ 990 Hz (nachgelesen S. 1810) — der Kragbalken trägt.
- Akkordeon C4: 267,6 Hz verlangt **0,35 mm** Stahl; Masse dann 0,31 g gegen 0,248 g in der Quelle — schlüssig für eine
  verjüngte Zunge (Einschätzung).
- Länge über den Umfang bei 0,35 mm Stahl: 84 mm (E1) … 8 mm (C8) — tiefe Zungen werden in der Praxis beschwert
  (nur Bericht, nicht belegt); die Kurve der Maße bleibt offen.
- **Masse-Feder: ≈ 2 % der Echtzeit je Zunge** (skalar, ohne SIMD); Musette-Note mit drei Zungen ≈ 6 %; ein Akkord mit
  drei Chören und vier Tönen ≈ 25 % — vorab unkritisch, aber über der Grenze des Flügels (4,6 % je Ton) **je Akkord**,
  nicht je Zunge.
- **Verteilter Balken N = 40: ≈ 20 % je Zunge** → Deluxe.

## 7. Risiken

- **Stabilität:** Bei Puranik & Scavone mussten im Masse-Modell einige Größen „unrealistic values for a numerically
  stable synthesis“ annehmen (nachgelesen S. 1811). Für 80–120 Noten braucht es eine Skalierung über den Tonumfang statt
  Probieren je Note (Einschätzung des Berichts).
- **Daten:** Spielerdrücke ungemessen; Musette und Register nur im NC-Material (OrchideaSOL); Mundharmonika-Bending ohne
  freie Aufnahme und ohne gelesene Primärquelle.
- **Rechte:** DAFx23 mit widersprüchlichem Vermerk; AiP 2021 „All rights reserved“ (nur Fakten); euphonics NC-SA.
- **Belegkraft:** Adachi et al. 2026 ist „not reviewed“ (nachgelesen).

## 8. Abweichungen vom Rahmen

- Die Bauart fehlt in SF10 a (F-1).
- Ein Akkordeon klingt mit **mehreren Zungen je Note** und einem **gemeinsamen Balg** für alle Noten — der Balgwechsel
  wirkt auf alle klingenden Noten zugleich (F-3). Das ist kein Ton je Note mit eigener Spur, sondern eine Spur über die
  Passage, die jede Note liest.

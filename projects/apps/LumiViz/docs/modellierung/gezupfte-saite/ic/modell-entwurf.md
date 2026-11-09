# Modell-Entwurf — Gezupfte Saite (Deluxe) (Klasse 2)

> **Stand:** 2026-10-26, IC Session 2. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Bestand in LumiViz (IC-B1 a), gemeinsame Grundlage [gemeinsam/koerper.md](../../koerper/ic/koerper.md) und F-K1.
> Berichte: `recherche/2026-10-26_korpus_deluxe.md`, `…_physik_cembalo_harfe_koto_oud.md`,
> `…_spielweisen_steuerung.md`, `…_aufnahmen.md`, Z1 `…_moden_und_waveguides.md`.
> **Kennzeichnung:** **nachgelesen** (Seite) · **nur Bericht** · **Einschätzung** · **eigene Rechnung** · **offen**.
> Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand in LumiViz (nachgelesen, `Saiten_Konzept.md`, 2026-10-26)

- **Saite:** drei Verfahren — *Classic* (Karplus-Strong, unberührt, S-K2 b), *Extended KS* und *Waveguide* (S-K1 a + b):
  „die Saite als zwei gegenläufige Wellen zwischen Sattel und Steg, Verlust- und Steifheitsfilter am Ende;
  Anschlagstelle als Ort auf der Saite, Anschlag als Form (Plektrum, Finger, weich/hart), Korpus als Resonanzen hinter
  dem Steg“. Regler nach der Physik (S-K3 a): *Pluck position*, *Pluck*, *Ring time*, *High damping*, *Stiffness*,
  *Body*, *String variation*; Tonabnehmer bis zwei (S-K5 a), Mensur (S-U3 a).
- **Korpus:** *Parlor* (Christensen/Vistisen mit Hess 2013 Tab. 1; gekoppelte Spitzen „116 und 219 Hz“, §5e) und
  *Classical* (Wright TOP26, zehn Deckenmoden und B(1,1), Luft 133 Hz, §5f) — „Decke und Luft als gekoppelte
  Schwinger, getrieben von der Kraft am Steg“ (S-K8 a). Werte nur aus veröffentlichten Messungen (S-K4 a), Abgeleitetes
  mit Rechenweg zählt (S-K7 a). **Eine** Polarisation; der Korpus „hört die Saite einen Abtastwert später“ (§5e);
  „ein Semi-Hollow mit Tonabnehmer hört seinen Korpus noch nicht“ (§5e).
- **Ein Ton** wird je Note „vorab und für sich“ gerechnet (§2) — kein Mitschwingen anderer Saiten.
- **Spielweisen** (Stufe 24–26): Strum mit Richtung, Palm mute, Tapping, Harmonic, Legato, Slide, Bend als Kurve an der
  Note (S-L2 a, bis zwei Ganztöne S-L15 b), Akkorde und Griffbilder, Saitenwahl je Note (S-M2 a).
- **Mitgeliefert:** Gitarren, Bässe, Ukulele als Saite; `harp`, `koto`, `angel_harp`, `dulcimer` als **Sample**
  („A recorded harp note with a short sine body.“, „A recorded koto note that rings for three seconds.“ — Dateien
  nachgelesen).
- **Offen in LumiViz:** der Gitarrenkorpus nach KB1 a auf die gemeinsame Grundlage (`modellierung/gezupfte-saite/README.md`
  „Noch offen“, nachgelesen).
- **Z1-Befund (IC Session 1):** Waveguide-Saite plus modaler Korpus als Admittanz ist durch die Literatur gestützt.

## 2. Was „Deluxe“ hier heißt

Drei Stufen, jede auf der vorigen (Einschätzung):

1. **Zwei Richtungen am Steg und Mitschwingen** (Gitarre, Oud): die Waveguide-Saite in zwei Polarisationen, alle Saiten
   an **einem** Steg mit 2×2-Admittanz (senkrecht und parallel zur Decke). Das bringt Schwebung und zweistufiges
   Abklingen und das Mitschwingen leerer Saiten.
2. **Korpus aus Messung** auf der gemeinsamen Form nach KB1 a: Moden mit Masse am Steg und Abstrahlgewicht, gesetzt an
   gemessener Stegmobilität (Nylon: Mores-Archiv).
3. **Neue Instrumente** auf derselben Saite: Cembalo (Kiel, Register, Resonanzboden), Harfe (Finger, offene Saiten,
   Pedale), Laute/Oud (Doppelchöre, bundlos), Koto (Stege, Oshi-de).

## 3. Kern je Stufe

### 3.1 Zwei Polarisationen, 2×2-Admittanz am Steg

- **Form:** Bank & Karjalainen 2010: „The admittance matrix is synthesized as a sum of PR scalar transfer functions
  (second-order filters) multiplied by positive semidefinite matrices. For wave-based modeling, such as digital
  waveguides (DWGs) … the admittance matrix is converted to a reflectance filter.“ (nachgelesen PDF-S. 1). Passiv,
  daher stabil — „the synthesis model remains stable even if the admittance matrix is estimated from less-than-optimal
  … measurements“ (nachgelesen PDF-S. 6).
- **Ordnung:** „even using R = 100 second-order sections is an overkill … filter orders in this range [30] provide a
  good compromise between sound quality and computational efficiency“ (nachgelesen PDF-S. 5).
- **Wirkung:** „strong beating and two-stage decay appears for those partials which are in that frequency range where
  the elements of the admittance matrix are large“ (nachgelesen PDF-S. 5).
- **Gemessen** an „an acoustic guitar bridge (Gibson, from 1960's) near the lowest (E) string“ durch Drahtbruch,
  senkrecht und parallel (nachgelesen PDF-S. 5) — die Werte stehen nur in Abbildungen.
- **Alle sechs Saiten an einem Steg:** in Bank & Karjalainen nur als „future research“ genannt („where all the six strings
  are coupled at the termination“, nachgelesen PDF-S. 6); ihr Weg dorthin: ein WDF-Admittanzelement mit
  Parallel-Adaptoren (PDF-S. 5, nachgelesen). Vorgeführt ist die Kopplung von sechs Saiten mit zwei Polarisationen
  modal (Udwadia–Kalaba, Debut & Antunes 2020, nur Bericht).
- **Anregung:** die Polarisation des Anschlags verteilt die Kraft auf beide Richtungen — Scherrer & Depalle: „The AOR
  [angle of release] … distributes the excitation … over the two transverse directions“ (nachgelesen PDF-S. 1).

### 3.2 Korpus aus Messung (Nylon)

- **Daten:** Mores-Archiv, Stegmobilität in zwei Kanälen von über 60 Gitarren (CC BY 4.0, Lizenz nachgelesen).
- **Weg:** Modalfit (Frequenz, Dämpfung, Residuum je Richtung) → Moden in der Form nach KB1 a → 2×2-Admittanz nach §3.1.
  Ducceschi, Russo & Webb (DAFx-2026) ziehen nach dem Bericht genau solche Modalparameter je Gitarre aus dem Archiv
  (nur Bericht — **lesen**).
- **Bauplan-Weg** (gemeinsame Grundlage, F-K1): Decke aus `InstrumentBody`, Luft nach Wright; nach Brauchler et al. 2023
  bleibt selbst nach Inversion von 35 Parametern ein mittlerer Frequenzrest von 1,55 %, höchstens 4,99 % (nur Bericht).
  Als Prüfstein und für Gitarren ohne Messung (Stahl) — Einschätzung.
- **Stahlsaitengitarre:** keine freie Messung; Hess 2013 Tab. 1 (in LumiViz schon *Parlor*) und Russells Hummingbird
  (103/188/202 Hz, nur Bericht, ohne Güte) — **offen**.

### 3.3 Cembalo

- **Anregung:** Kiel am Springer; Wechselwirkung „12 ± 2 ms and 7.9 ± 0.6 ms for the normal and the fast tempi“,
  Springergeschwindigkeit „0.10 ± 0.05m/s and 0.25 ± 0.03m/s“ (Chadefaux et al. 2013, nachgelesen S. 157); Saite
  47,5 cm, 85,7 N, Kiel E = 1,5 GPa (ebd. Tab. 2, nachgelesen S. 158). Dagegen Paté et al. 2017: „The jack velocity does
  not have an influence on any of the measured quantities.“ (Crossref-Abstract, nachgelesen) → F-4.
- **Dynamik:** Penttinen 2006: Teiltöne 1–3 bei C4 ff gegen pp um „5 dB, 3 dB, and 0.5 dB“ (nachgelesen PDF-S. 2);
  „at least a general 10 dB level difference for the lowest body modes (< 300 Hz) from one dynamic level to another“
  (nachgelesen PDF-S. 3, der Tastenstoß in den Boden); Lautheit größter Mittelwert „2.41“ dB (nachgelesen PDF-S. 4).
- **Register und Zupfpunkt:** „the 8′ registers are called 8′ back and front registers“; „The plucking point changes from
  12% to about 50% of the string length in the bass and in the treble range“ (Välimäki et al. 2004, nachgelesen
  S. 935–936).
- **Rechnung (Literatur):** je Ton Saitenmodell „and a second-order resonator that are excited with a common
  excitation signal“ für die Schwebung; Loslassgeräusch „by triggering a sample“ (Välimäki et al., nachgelesen S. 934,
  936). Hier: Waveguide-Saite mit Kiel als Anfangsbedingung (Auslenkung, Geschwindigkeit, Polarisationswinkel aus
  Chadefaux Tab. 1, nachgelesen S. 158) — Einschätzung.
- **Resonanzboden (Dulcken 1755):** Dicke „around 2.4 mm“ bis „5-6 mm in the bass region“; Helmholtz „37 Hz“ gemessen
  und gerechnet; „T60 = 235 ms“ am 8′-Steg; „In total 3938 N“; „156 strings altogether“ (Bader et al. 2025, CC BY 4.0,
  HTML nachgelesen). Boden eben mit Rippen — „Cembalo … dem Flügel am nächsten“ (`gemeinsam/koerper.md` §2): Bauplan
  in `InstrumentBody` (Einschätzung).

### 3.4 Harfe

- **Anregung (Finger):** Chadefaux et al. 2012 an zehn Harfenisten, Saite D♭2 (138,6 Hz, 260 N, 97,5 cm, 1,96 mm Darm,
  nachgelesen PDF-S. 3); Fingerkraft „up to 15 N“ (nachgelesen PDF-S. 6); Mittelwerte beim Ablösen „Dtr = 4 mm“ und
  „Vtr = 2 m/s“ (nachgelesen PDF-S. 9); Haftphase, Gleitphase, Polarisationswinkel 7°–43° (nur Bericht des
  Spielweisen-Agenten, der die Seiten als Bild las).
- **Offene Saiten:** keine Dämpfer; „Sympathetic string modes in the concert harp“: „35 strings“ an „an equivalent beam
  representing the soundboard“ (Le Carrou et al. 2009, HAL-Abstract, nur Bericht). Mitschwingen ist der eigentliche
  Gewinn (Einschätzung beider Berichte).
- **Körper:** Resonanzboden mit Saitenlinie, Kasten mit Löchern; A0/T1 nach Le Carrou als „two degrees of freedom
  discrete model“ (nur Bericht) — dieselbe Form wie der Gitarrenkorpus *Parlor*. Werte **offen**.

### 3.5 Laute/Oud

- **Doppelchöre:** Doppelabklingen nach Woodhouse 2025 aus Saiteneigenschaften und Stegadmittanz, auch „with single
  strings, from a difference of decay rates between the two polarisations“ (HAL-Abstract, nur Bericht) — trägt §3.1.
- **Decke:** Oud-Decke mit Querbalken, FE und Hammer-Modaltest (Abdallah et al. 2026, CC BY 4.0 nachgelesen; Werte
  nicht gelesen). Muschelboden der Laute: **offen**.
- **Saiten:** Laute nach Gamut-Tabelle (nur Bericht); Oud **offen** (nur Forum).

### 3.6 Koto

- **Oshi-de:** 「絃を演奏者から見て柱の左側の位置上で押すことにより絃の張力を増加させ，柱の右側の絃振動の基本周波数を上昇させる」;
  Halbton „弱押し“, Ganzton „強押し“ (Nishiguchi, Ando, Kochi 2021, J-STAGE-Abstract nachgelesen). Bei fester Länge
  rechts vom Steg verlangt das +12,2 % bzw. +26,0 % Zug (eigene Rechnung). In der Saite: zeitveränderlicher Zug, also
  Verzögerung mit Modulation (Einschätzung).
- **Körper, Saiten:** Paulownia-Körper, 13 Saiten, Stege — Werte **offen** (Adelaide-Arbeiten nicht erreicht).

**Z1 (Moden und Waveguides):** alle Stufen bleiben Waveguide-Saite plus modaler Steg/Korpus — die Form, die der
Z1-Befund für Klasse 2 stützt. Für das Setzen nach SZ3 a (Vorhersage je Teilton) siehe F-3.

## 4. Größen und ihre Herkunft

| Größe | Kurve / fest / Stärke | Herkunft (Vorgabe) | Gesetzt an ([messplan.md](messplan.md)) |
|---|---|---|---|
| 2×2-Admittanz am Steg (Moden, Residuen je Richtung) | fest je Korpus | Form Bank & Karjalainen 2010 (nachgelesen), R ≈ 30; Werte Nylon aus Mores (Fit), Stahl **offen** | Stegmobilität (M1) |
| Abstrahlung je Mode | fest | Mores Nahfeld (drei Kanäle, nur Bericht); unter 300 Hz Monopol, darüber Dipol (Cardiff, nur Bericht) | Nahfeld/Klang (M2) |
| Polarisationswinkel des Anschlags | Stärke/Note | Harfe 7°–43° (nur Bericht), Cembalo 17°/20° (nur Bericht); Gitarre über Rest-/Free-Stroke (Scherrer, nachgelesen) | Schwebung, zweistufiges Abklingen (M3) |
| Saite: Länge, Zug, Durchmesser, Material | Kurve | Cembalo Chadefaux Tab. 2 (nachgelesen), Taskin-Mensuren (nur Bericht); Harfe D♭2 (nachgelesen), Rest **offen**; Laute Gamut (nur Bericht); Oud, Koto **offen** | f0, Inharmonizität (M4) |
| Kiel: Kontaktzeit, Abwurf | Stärke (F-4) | 12 / 7,9 ms; 0,10 / 0,25 m/s (nachgelesen); Einfluss strittig (Paté 2017) | Spektrum über Stärke (M5) |
| Tastenstoß in den Boden (Cembalo) | Stärke | „10 dB“ in den Korpusmoden je Stufe (nachgelesen) | Pegel < 300 Hz (M5) |
| Loslassgeräusch (Cembalo) | je Note | VCSL-Releases (CC0) | Release-Dateien (M6) |
| Finger: Auslenkung, Geschwindigkeit (Harfe) | Stärke | 0,5–7 mm, 0,2–5 m/s (nur Bericht); Mittel 4 mm, 2 m/s (nachgelesen) | Spektrum f/mf (M5) |
| Resonanzboden Cembalo | fest (Bauplan) | Dulcken: Dicken, Helmholtz 37 Hz, T60 235 ms (nachgelesen) | Abklingen am Steg (M7) |
| Harfenkörper A0/T1 | fest | **offen** | — (keine freie Messung) |
| Doppelchor-Verstimmung (Laute, Oud) | Ausstattung | **offen** | Schwebung (M3) — Aufnahme fehlt |
| Oshi-de-Verlauf (Koto) | Note (F-8) | Halbton/Ganzton (nachgelesen); Verlauf **offen** | — (Aufnahme fehlt) |

## 5. Rechenzeit

Skript `zupfabschaetzung.py` (Scratchpad, **eigene Rechnung, kein Beleg**; skalar, 1 GFlop/s, 48 kHz; Waveguide 30
Operationen je Saite und Polarisation, Filter zweiter Ordnung 9 Operationen):

```python
wg_saite, biquad, R = 30, 9, 30
matrix_2x2 = 3 * R * biquad                     # yy, yz, zz
gitarre_deluxe = 6 * 2 * wg_saite + matrix_2x2  # sechs Saiten, zwei Polarisationen, ein Steg
harfe = 47 * 2 * wg_saite + 60 * biquad         # alle Saiten offen, 60 Moden am Steg (Annahme)
cembalo = 156 * wg_saite + 60 * biquad          # Dulcken, alle Saiten
```

| Fall | Operationen je Abtastwert | Anteil der Echtzeit |
|---|---|---|
| Gitarre heute (sechs Saiten, Classical) | 276 | 1,3 % |
| **Gitarre Deluxe** (zwei Polarisationen, 2×2 mit R = 30) | 1170 | **5,6 %** |
| Oud (elf Saiten, zwei Polarisationen) | 1470 | 7,1 % |
| Cembalo, eine Taste mit drei Chören | 630 | 3,0 % |
| Harfe, alle 47 Saiten mitschwingend | 3360 | 16 % |
| Cembalo, alle 156 Saiten mitschwingend | 5220 | 25 % |

Alles vorab unkritisch; das volle Mitschwingen liegt über der Grenze des Flügels (4,6 % je Ton), ist aber **je Clip**
zu rechnen, nicht je Note — die Saiten klingen über die Noten hinweg (§6).

## 6. Risiken und Abweichungen vom Rahmen

- **Ton je Note:** Mitschwingen (Harfe, Cembalo, Gitarre) koppelt Noten über den Steg — wie beim Flügel mit Pedal
  (HA4 a) und beim Tonrad (Klasse 11 F-1). Ein Ton je Note kann nur die eigene Saite mit dem Korpus rechnen (F-3).
- **Setzen (SZ3 a):** Die 2×2-Admittanz liefert keine geschlossene Vorhersage je Teilton; gesetzt wird der Korpus an der
  Stegmobilität, die Saite wie heute (F-3).
- **Daten:** Stahl ohne Korpusmessung; Harfe ohne A0/T1 und Saitentabelle; Koto und Oud ohne Aufnahme.
- **Belegkraft:** viele Korpuswerte nur aus Abstracts (HAL, Acta Acustica gesperrt); DAFx-2026 von Ducceschi et al.
  ungelesen.
- **Harfenpedale, Koto-Bend, Maqam-Intonation** passen nicht glatt in die drei Orte von SF8 a (Bericht Spielweisen;
  F-6, F-8, F-9).

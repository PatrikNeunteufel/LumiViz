# Modell-Entwurf — Hammer-Saite (Rest), Klasse 1

> **Stand:** 2026-10-26, IC Session 1. **Typ:** Entwurf, nichts entschieden. Rahmen: [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md),
> Bestand in LumiViz (IC-B1 a). Berichte: `recherche/2026-10-26_upright_honkytonk_cimbalom.md`,
> `…_cp70_clavinet.md`, `…_aufnahmen.md`.
> **Kennzeichnung je Angabe:** **nachgelesen** (von dieser Sitzung selbst gelesen, mit Ort) · **nur Bericht** ·
> **Einschätzung** (Claude) · **offen**. Fragen: [offene-fragen.md](offene-fragen.md).

## 1. Bestand

- **Zielbild** (nachgelesen, `Spielweisen_Konzept.md` §4.1, S141): „Zuerst Konzertflügel und Klavier (Upright); danach
  kommen ebenso Honky-tonk (verstimmte Saitenchöre), der E-Flügel (wie CP-70: Saiten mit Tonabnehmer), das Clavinet
  mit echter Tangente statt Zupfen, Hackbrett und Cimbalom gerechnet statt aufgenommen. Sie sind Vorlagen derselben
  Bauart (Hammer × Saite, SF10 a)“. Spielweisen: Sustain mit halbem Pedal, Sostenuto, Una corda, „Moderator- oder
  Übepedal (Upright), Abdämpfen mit der Hand und das präparierte Klavier“.
- **Das gebaute Modell** (nachgelesen, `modellierung/hammer-saite/modell.md`): Saite modal nach Bank 2010 (B, zwei
  Abklingstufen, Chor als gekoppelte Moden, Längsschwingung), Hammer F = Q0·Δy^p, Ausgang **Kraft am Steg**
  F_b = cos α · Σ_k (kπT0/L)·Y_k (§3), Dämpfer, Größen als Kurven über die Tastatur (HA1 a); Boden aus Bauplan (BM1 a).
- **Ausstattung** (nachgelesen, HA4 a): Pedale links *Una corda* oder *Soft*, Mitte *Sostenuto*, *Practice*
  (Moderator) oder keins, rechts *Sustain*; „die Vorlagen *Grand* und *Upright* setzen die Werte“; Kipppunkt: wenn sich
  die Pedale des Upright nicht auf diese Plätze abbilden lassen.
- **Achsen** (nachgelesen, HA2 a): *Excitation* (*Pluck*, *Hammer*) × Verfahren; „Clavinet und Bogen-Saite werden neue
  Werte auf denselben Achsen“; Kipppunkt: „wenn beim Clavinet Hammer und Zupfen an der Saite nichts teilen außer dem
  Namen“. SF10 a: „die Tangente des Clavinets als Hammer“. Name des Upright: `upright_piano` (UF14 a).

## 2. Je Instrument

### 2.1 Upright (`upright_piano`, Stufe 32 in LumiViz)

Dieselbe Rechnung wie der Flügel (HS2 a); neu sind Daten und zwei Pedale.

| Teil | Änderung gegen den Flügel | Herkunft |
|---|---|---|
| Saitendaten (Länge, Masse je m) | kürzere Bässe; **keine veröffentlichte Mensur eines modernen Upright gefunden** | offen — Fletcher 1964 (Hamilton-Upright, nur Bericht) nicht erreicht; Basssaite Yamaha U1 46,8″ (nur Bericht) |
| Inharmonizität B | im Bass größer: B ∝ d⁴/(T·L²) — bei 0,6-facher Länge und gleichem d, T etwa das 2,8-fache (Einschätzung, Bericht) | gemessen an K18 durch das Modell hindurch (M1) |
| Boden | Bauplan aus Chaigne, Cotté & Viggiano 2013: Pleyel P131, schwingende Fläche 1,365 m × 0,96 m (nachgelesen S. 2461), einfaches Modell 1,41 × 1,01 m², 9 mm, zehn Rippen 2,1 × 2,5 cm² (nachgelesen S. 2457), Fichte ρ = 440, E_L 15,9 GPa … (Tab. I, nur Bericht), Rippenabstände 11,6–14,6 cm (Tab. III, nur Bericht), η = 1,5 % (nachgelesen S. 2459); **Steglinie und Bassteg fehlen** (nur als Bild) | Bauplan wie Steinway D (BM1 a); Prüfstein: Modendichte ≈ 0,06 /Hz (Ege, nur Bericht), erste Moden des P131 120–220 Hz gegen gemessen (Abb. 12, nur Bericht) |
| Abstrahlung | Rückwand und Gehäuse statt Platte in Schallwand | offen (keine Messung gefunden) — Hörorte vor dem Gehäuse, wie K18 (AB ~30 cm) |
| Hammer | Upright-Mechanik; Werte offen | gemessen an K18 über 6 Stufen (M4) |
| Dämpfer | neben der Anschlagstelle, im Bass schwächer (Wogram, nur Bericht) | Loslassen an K18 (M5) |
| Pedal links *Soft* | Hämmer näher → kürzerer Weg → kleinere Hammergeschwindigkeit je Stufe (Einschätzung); Wirkung „nearly negligible“ (Forum, nur Bericht) | offen |
| Pedal Mitte *Practice* | Filzstreifen zwischen Hammer und Saite: **zweite weiche Feder in Reihe** zum Hammerfilz (Einschätzung; kein Modell in der Literatur gefunden) | offen |
| Pedal Mitte *Bass-Sustain* | hebt nur die Bassdämpfer — **nicht auf HA4 a abbildbar** (Wikipedia, nur Bericht) | F-2 |

### 2.2 Honky-tonk

Nur Daten: die Kurve `detune` (Verstimmung im Chor) weit größer — Stimmerpraxis ±1 Hz bei A4 (±3,9 Cent), „only
detuned the plain wire strings“ (Forum, nur Bericht). Das Modell mit zwei Stufen trägt das schon (Chor als gekoppelte
Moden, `modell.md` §5). Gesetzt an FreePats „Old Piano FB“ (M3). Tack piano (Reißnägel im Hammer): ein harter
Kontakt vor dem Filz — keine Messung, kein Modell gefunden; nicht im Entwurf.

### 2.3 E-Flügel (CP-70/80)

| Teil | Entwurf | Herkunft |
|---|---|---|
| Saiten | kürzer; eine Saite bis MIDI 42, darüber zwei (nur Bericht: smem, Modartt) | Längen offen |
| Hammer | Flügelmechanik, Hämmer mit „synthetic buck-skin“ (M-Modelle; nur Bericht, Prospekt S. 5) | offen |
| Boden | **keiner** (nur Bericht) — Steg auf dem Rahmen: kleinere Stegnachgiebigkeit, andere Kopplung im Chor (Einschätzung) | — |
| Abnahme | Piezo unter dem Steg misst die **Stegkraft** (Einschätzung) — das ist genau der Ausgang des gebauten Modells (F_b, `modell.md` §3, nachgelesen); danach Vorverstärker und Klangregelung | Lage je Saite oder je Stegabschnitt offen (Quellen widersprechen sich) |
| Tremolo, EQ | Effekt nach dem Instrument (0,8–10 Hz, 15–40 %; 7 Bänder ±12 dB — nur Bericht, Prospekt S. 8) | kein Teil des Modells |

Einschätzung: der kleinste Schritt der Klasse — das Modell des Flügels ohne Boden, Abnahme am vorhandenen Ausgang.
Ohne freie Aufnahme bleibt er aber ungemessen (F-6).

### 2.4 Clavinet

Die Tangente ist **Stoß + bleibende Klemme** (nachgelesen, Gabrielli et al. 2013, S. 4: „The rubber tip strikes the
string and traps it against a metal stud, or anvil, for the duration of the note, splitting the string into speaking
and nonspeaking parts“). Beim Loslassen werden beide Teile wieder eine Saite, die Garnwicklung dämpft: „the pitch
decrease after release is three semitones for the whole keyboard“ (S. 6, nachgelesen).

| Teil | Entwurf (Einschätzung) | Herkunft |
|---|---|---|
| Aufschlag | Kontakt wie der Hammer (Q0·Δy^p, weiches Gummi), Tastengeschwindigkeit „1 to 4 m/s“ (nachgelesen S. 11) | Q0, p offen (Gabrielli nimmt einen gefitteten Puls statt eines Kontakts) |
| Halten | **Randbedingung**: Moden der klingenden Länge Amboss–Saitenende, vorab je Taste | B je Taste (Gabrielli Tab. 1, nur Bericht), T60 bis „20 s or more“ (nur Bericht) |
| Loslassen | **zweiter Modensatz** der ganzen Saite mit Garndämpfung; Zustand linear auf ihn projiziert — die −3 Halbtöne entstehen von selbst | Projektion: Einschätzung; −3 Halbtöne: nachgelesen |
| Klopfen | Geräusch der Tangente am Amboss (bei Gabrielli ein Sample) | offen |
| Tonabnehmer | zwei elektromagnetische, je Lage ein Kammfilter, Kennlinie Polynom 4. Ordnung (nur Bericht: Remaggi 2012, Gabrielli Tab. 2) — passt zu den Tonabnehmern der Saite in LumiViz | Lagen: 18,5–6,5 cm und 4 cm vom Saitenende (Remaggi, nur Bericht) |
| Dämpferschieber, Klangschalter | zusätzliche Dämpfung (Ausstattung/Passage); vier Filter (Gabrielli Tab. 3, nur Bericht) | — |
| Kosten | Gabrielli: „Total per string 82“ Multiplikationen, 64 Additionen (nachgelesen S. 15) — Wellenleiter; modal ähnlich klein (Einschätzung) | — |

**Folge für den Bestand:** Die Tangente teilt mit dem Hammer den Aufschlag, mit dem Zupfen nichts; zusätzlich ist sie
eine Randbedingung, die sich beim Loslassen ändert (F-3).

### 2.5 Hackbrett und Cimbalom

- **Physik** (nur Bericht): der Steg teilt die Saiten 3 : 2 (Peterson 1994; de.wikipedia „2 zu 3“) — Modenpaare bei
  jedem dritten Teilton; Chöre von 2–5 Saiten (Cimbalom: vier im Diskant, drei im Bass); Schlägel 8–12 g, Holz mit
  Leder oder Baumwolle; **Hackbrett ohne Dämpfer** — alle Saiten schwingen über Steg und Boden mit; Cimbalom mit
  Dämpferleiste am Pedal; Kasten mit Decke, Boden, Schalllöchern.
- **Entwurf** (Einschätzung): Saite modal mit **zwei Abschnitten über einem Steg** (Kopplung am Steg wie der Chor),
  freie Anschlagstelle als Spielweise, Mitschwingen aller Saiten wie Stufe 31e des Flügels (dort noch offen), Kasten
  als Körper mit Luftraum ([gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md) F-K1).
- **Kosten:** 60–125 Saiten dauernd aktiv — teurer als der Flügel ohne Pedal (Bericht, Einschätzung) → Deluxe.
- **Daten:** fast keine (Saitendaten nur eine Spannungstabelle eines amerikanischen Hackbretts, Peterson 2005; keine
  freie Aufnahme des Cimbaloms). Risiko hoch.

## 3. Größen und ihre Herkunft (Upright, Kern der Klasse)

Die Größen des Flügels (`modell.md` §2) gelten weiter; je Größe, woher sie beim Upright kommt:

| Größe (`.lvinst`) | Vorgabe / Herkunft | Gesetzt an |
|---|---|---|
| `inharmonicity` | offen → K18 durch das Modell hindurch | M1 |
| `decay`, `lateDecay`, `highDecay` | offen → K18 | M2 |
| `detune` | offen → K18; Honky-tonk → FreePats | M3 |
| `hammerMass`, `hammerExponent`, `hammerStiffness` | Startwerte des Flügels (RT-0425) → K18 (6 Stufen) | M4 |
| `strikePosition` | 1/7–1/9 der Länge (Patent, nur Bericht) | Einbrüche im Spektrum (M4) |
| `damper`, `undampedFrom` | offen → K18 Loslassen | M5 |
| `stringLength`, `stringMass`, `longitudinalRatio` | **offen** (keine Mensur) — F-7 | — |
| `bridgeAngle` | offen | Phantomteiltöne (M6) |
| Boden | Bauplan nach Chaigne et al. 2013 (P131), Steglinie geschätzt | Terzbänder (M7) |
| Pedal *Soft*, *Practice* | offen | Aufnahmen fehlen (K18 hat keine) |

## 4. Rechenzeit

Upright, Honky-tonk, CP-70: wie der Flügel (2,3 % der Echtzeit je Ton im Bass, nachgelesen `modell.md` §9) — kürzere
Saiten und weniger Chöre eher weniger. Clavinet: eine Saite je Taste, zwei Modensätze — weniger als der Flügel
(Einschätzung). Hackbrett/Cimbalom: Mitschwingen aller Saiten — Deluxe (Einschätzung).

## 5. Abweichungen vom Rahmen

- **Clavinet:** ein Ton hat zwei Teile — Halten (vorab je Taste und Stufe) und Loslassen (hängt vom Zeitpunkt ab, Weg
  über die Dauer im Namen des Tons wie beim Flügel). Kein Widerspruch zum Rahmen, aber eine neue Anregung (F-3).
- **CP-70:** Ausgang Stegkraft statt Boden und Abstrahlung — der Rahmen trägt das (Körper weglassen).
- **Hackbrett/Cimbalom:** Saite über einen Steg geteilt; Mitschwingen ohne Pedal dauernd.

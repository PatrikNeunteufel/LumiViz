# Prüfung IC3: Klassen 1–4 gegen die Lehren L1–L4 und L7 des Flügels

> **Auftrag:** je Klasse `modell-entwurf.md`, `messplan.md`, `aufnahmen.md` gegen L1–L4 und L7 aus
> [LEHREN_SETZEN_FLUEGEL.md](../LEHREN_SETZEN_FLUEGEL.md) prüfen (erfüllt / teilweise / fehlt, mit Datei:Zeile);
> Größenliste und Datensatzangaben je Klasse.
> **Datum:** 2026-10-27 (IC Session 3).
> **Bearbeitet von:** einem Agenten (Claude, nur lesend) — **Bericht eines Agenten**, unten **unverändert**.
> **Reichweite:** die drei Dateien je Klasse, ganz gelesen; `recherche/` und `quellen/` nicht. Zeilen beziehen sich
> auf den Stand vom 2026-10-27 vor den Nachträgen dieser Sitzung (Klasse 3 `aufnahmen.md` hatte da schon den
> Abschnitt „Geladen“).
> **Von der Sitzung selbst nachgelesen (Stichproben):** Klasse 1 MP:6–7 und MP:17–18 (UF5 a, „6 Stufen; K18: Raum
> unbekannt“), Klasse 2 ME:132–136 (Spalte „Kurve / fest / Stärke“), Klasse 3 ME:59 („am Modell ermittelt, nicht aus
> der Formel“), Klasse 4 MP:18–19 („20 dB über“, „ab 10 dB Abfall“); keine Datei der Klassen enthält „Toleranz
> messbar“, „Aufnahmelänge“ oder „gesucht oder gesetzt“ (Suche über alle `messplan.md`, `aufnahmen.md`,
> `modell-entwurf.md`). Alle Stichproben stimmen.
> **Eingeflossen in:** [../lehren-pruefung.md](../lehren-pruefung.md).

---

Grundlage: LEHREN_SETZEN_FLUEGEL.md und je Klasse die drei Dateien, alle ganz gelesen. Die Pfade unten gelten relativ zu `C:\Users\patri\source\repos\Visuals_Project\cmake\InstrumentsCalculation\klassen\<Klasse>\`. Abkürzungen: ME = modell-entwurf.md, MP = messplan.md, AU = aufnahmen.md.

**Was für alle vier Klassen gleich ist:**
- Keine Klasse gibt die Toleranz als Rezept aus der Streuung des Datensatzes an. Alle nennen geschätzte Zahlen oder übernehmen sie vom Flügel.
- Keine AU vermerkt, welche Streuung ein Datensatz hergibt. Die Spalte „Toleranz messbar aus“ fehlt überall.
- Keine AU nennt die Länge der Aufnahmen. Die Regel „höchstens etwa 3× Aufnahmelänge“ steht nirgends.
- Kein MP plant das Setzen als Ausgleich über alle Noten zugleich und keiner schätzt die Dauer eines Setz-Laufs.
- Kein ME unterscheidet ausdrücklich „gesucht“ und „gesetzt“.

## Klasse 1 – Hammer-Saite

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: Es gibt Zahlen, aber geschätzt bzw. vom Flügel übernommen: „wie UF5 a (Abklingzeiten ±10 %, B ±5 %, Verstimmung ±0,5 Cent, Dämpfer ±20 %)“. Bei M7 steht „Streuung je Band berichten“, als Toleranz ist das kein Rezept. | MP:6-7, MP:17-23 |
| L1 Datensätze | fehlt als Vermerk. Die Angaben zu Stufen und Mikrofonen stehen zwar da, aber nicht als Streuungsquelle. | AU:12-13, AU:24-28 |
| L2 Kurven | teilweise: Nur pauschal heißt es „Größen als Kurven über die Tastatur (HA1 a)“. Die Tabelle der Größen nennt keine Art und keine Stützpunkte. | ME:18, ME:101-112 |
| L2 Setzen | fehlt: Ausgleich über alle Tasten ist nicht geplant, „wie Flügel“ bleibt offen. | MP:17-23 |
| L3 gültiger Bereich | teilweise: Eine Spalte gibt es, aber nur mit Worten („Bass bis Mitte“, „späte Stufe mit Vorsicht (Nachhall)“, „Fenster der Hüllkurven prüfen“). Eine 3×-Regel fehlt, ebenso die Länge der K18-Töne. | MP:17-23 |
| L4 Schwellen | teilweise: Zweistufiges Abklingen und Chor sind genannt. Der Prüfstein fragt, ob das Modell die Einbrüche und das zweistufige Abklingen „überhaupt zeigt“. Es fehlen die Unterscheidung gesucht/gesetzt und die Messung an der Struktur des Bezugs. | ME:16-18, ME:47-48; MP:25-28 |
| L7 Rechenzeit | teilweise: Je Ton beim Spielen stehen „2,3 % der Echtzeit je Ton im Bass“. Keine Zeit je Auswertung, keine Dauer eines Setz-Laufs. | ME:116-118 |

**Größen** (ME:101-112, Art nirgends angegeben):
- `inharmonicity`
- `decay`, `lateDecay`, `highDecay`
- `detune`
- `hammerMass`, `hammerExponent`, `hammerStiffness`
- `strikePosition`
- `damper`, `undampedFrom`
- `stringLength`, `stringMass`, `longitudinalRatio` (offen)
- `bridgeAngle`
- Boden (Bauplan)
- Pedale *Soft* und *Practice* (offen)

**Datensätze** (AU:12-13, 24-28):
- **K18:** 6 Stärken, kleine Terzen ab A0, stereo mit zwei Mikrofonen (AB, etwa 30 cm). Wiederholungen sind nicht genannt.
- **VCSL Knight:** 2 Stärken, Ganztöne.
- **VCSL Yamaha:** 3 Stärken, „meist 2 Aufnahmen je Ton“, nur C und G.
- **FreePats Old Piano FB:** Stärken offen, Zoom H1n.

Einschätzung: K18 erlaubt damit Nachbarstufen und zwei Kanäle wie bei Salamander, Yamaha zusätzlich Wiederholungen.

## Klasse 2 – Gezupfte Saite

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: Spalte „Toleranz (Vorschlag)“ mit Zahlen ohne Herkunft (z. B. ±1 %, ±20 %, ±2 dB, ±1 Cent). Ein Prüfstein ist aus der Literatur genommen (Penttinen 5/3/0,5 dB), ein Rezept aus der Streuung fehlt. | MP:15-25 |
| L1 Datensätze | fehlt als Vermerk | AU:22-24, AU:43, AU:59 |
| L2 Kurven | teilweise: Die Spalte „Kurve / fest / Stärke“ ist vorhanden. Stützpunkte werden nicht genannt. | ME:132-145 |
| L2 Setzen | fehlt | MP:15-29 |
| L3 gültiger Bereich | teilweise: Die Spalte gibt meist Datensatz und Frequenzband an, etwa „70 Hz – 2 kHz (Einschätzung)“. Sie enthält keine Grenze für Abklingzeit oder Schwebung und keine Längen. | MP:17-25 |
| L4 Schwellen | teilweise: Schwebung und zweistufiges Abklingen aus der Admittanz, Kopplung beim Mitschwingen, Register des Cembalos. Der Prüfstein fragt, ob der Korpus die Schwebung „gibt“. Es fehlen gesucht/gesetzt und die Struktur des Bezugs. | ME:55-56, ME:87-89, ME:174-177; MP:27-30 |
| L7 Rechenzeit | teilweise: Je Fall gibt es eine Schätzung beim Spielen (z. B. „Gitarre Deluxe … 5,6 %“). Keine Zeit je Auswertung beim Setzen, keine Laufdauer. | ME:147-170 |

**Größen** (ME:134-145):
- 2×2-Admittanz am Steg: fest je Korpus
- Abstrahlung je Mode: fest
- Polarisationswinkel: Stärke/Note
- Saite (Länge, Zug, Durchmesser, Material): Kurve
- Kiel: Stärke
- Tastenstoß: Stärke
- Loslassgeräusch: je Note
- Finger (Harfe): Stärke
- Resonanzboden des Cembalos: fest
- Harfenkörper A0/T1: fest
- Doppelchor-Verstimmung: Ausstattung
- Oshi-de-Verlauf: Note

**Datensätze:**
- **Iowa Nylon:** pp/mf/ff je Saite, mono 24 bit/96 kHz, 45 Dateien, chromatische Läufe je Datei (AU:22, 59).
- **VCSL:** Harfe mit 2 Stärken; Cembalo mit Registern (Lute/Normal, High/Low) und getrennten Releases, nur ein Pegel (AU:14, 23; MP:21).
- **Mores:** Stegmobilität mit zwei Kanälen und drei Nahfeldkanälen, über 60 Gitarren (AU:43).
- **AG-PT-set:** keine Stärkenstufen (AU:24).

Wiederholungen derselben Note sind nirgends vermerkt.

## Klasse 3 – Bogen-Saite

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: Zahlen als Vorschlag (±3 Cent, ±20 %, ±2 dB …). Teilweise ist eine Literaturgrenze verankert: die Hörschwelle „about a semitone (6%)“ und Guettler 50/90 ms. Ein Rezept aus der Streuung fehlt. | MP:15-21, MP:27-28 |
| L1 Datensätze | fehlt als Vermerk | AU:11-15, AU:21-26 |
| L2 Kurven | teilweise: Die Art-Spalte ist vorhanden, Kurven fast keine (die meisten Größen sind „fest je Saite/Instrument“). Stützpunkte werden nicht genannt. | ME:85-96 |
| L2 Setzen | fehlt | MP:13-31 |
| L3 gültiger Bereich | teilweise: Gemeint ist der Ausschnitt („Ausklang arco“, „stationärer Teil“, „100 Hz–6 kHz“). Keine 3×-Regel, keine Längen. | MP:15-21 |
| L4 Schwellen | teilweise, am deutlichsten von allen: Das Schelleng-Fenster „wird am Modell ermittelt, nicht aus der Formel“. Genannt sind auch der Wolfton (die Helmholtzbewegung „kippt“), Mehrfachgleiten beim Ansatz und chaotische Bereiche. Gesucht/gesetzt wird nicht unterschieden, an der Struktur des Bezugs wird nicht gemessen. | ME:49-62, ME:106-109; MP:30-31 |
| L7 Rechenzeit | teilweise: Spielen „0,19–0,91 der Echtzeit in Matlab“, in C++ „deutlich unter Echtzeit (Einschätzung)“. Setzen und Laufdauer fehlen. | ME:100-102 |

**Größen** (ME:87-96):
- Saite: fest je Saite
- Reibung: fest
- Korpusmoden: fest je Instrument
- Expression → (vB, β, F): Weg
- Ansatz: fest je Artikulation
- Vibrato: Spur + Rate
- Bogenwechsel: aus der Bogenlänge
- Springbogen: je Note
- Dämpfer: Passage
- Rauschen: Kurve

**Datensätze:**
- **Iowa 2012:** pp/mf/ff je Saite, arco/pizz, mono (Ordnername), Violine nur 16 bit (AU:21).
- **TinySOL:** pp/mf/ff, mono, 284–309 Töne je Instrument (AU:22, 36).
- **TU-Note:** vier Stärken, Mikrofone DPA und B&K, WAV mono (AU:23, 37).
- **VSCO 2 CE:** stereo; Sections mit zwei Stärken (AU:15, 24).
- **CNSM:** drei Violinen mit Stegadmittanz (AU:25).
- **Lampis:** Bogenkraft und Bogenbeschleunigung gestuft (AU:26, 40).

Wiederholungen sind nicht vermerkt. Einschätzung: Bei TU-Note würden zwei Mikrofone eine Kanalstreuung erlauben, bei den Stufen von Lampis eine Stufenstreuung.

## Klasse 4 – Platte und Stab

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: Zahlen meist als „(Einschätzung)“ markiert oder „wie UF5 a beim Flügel“. Bei M1 ist eine Literaturgröße angebunden (Kirkland „0.25% error“ ≈ 4,3 Cent), ein Rezept aus der Streuung fehlt. | MP:18-25 |
| L1 Datensätze | fehlt als Vermerk | AU:12-21, AU:27-29 |
| L2 Kurven | teilweise: Die Art-Spalte ist vorhanden, dazu der Satz „Kurven über den Tonumfang wie beim Flügel (Stützpunkte …)“. Die Zahl der Stützpunkte fehlt. | ME:95-112 |
| L2 Setzen | fehlt (nur „sonst setzt der Ausgleich falsche Dämpfungen“) | MP:27-30 |
| L3 gültiger Bereich | teilweise, am konkretesten von allen: „mindestens 20 dB über dem Rauschen“, „unter 60 Hz … nicht reflexionsarm“, „ab 10 dB Abfall“, „Ende der Datei prüfen (`endeVorBlende`)“. Keine 3×-Regel und keine Längen, obwohl die Abklingzeit des Stabs mit „close to 10 s“ ohne Resonator genannt ist. | MP:18-24; ME:55-56 |
| L4 Schwellen | teilweise: zweistufiges Abklingen durch die Kopplung Stab–Rohr, Kopplung der Steeldrum „ab 1/25 times the thickness“, Motor zeitabhängig, Torsion. Der Prüfstein fragt, ob das Modell die Erscheinungen hervorbringen kann. Gesucht/gesetzt wird nicht unterschieden, an der Struktur des Bezugs wird nicht gemessen. | ME:58-70, ME:83; MP:27-30 |
| L7 Rechenzeit | teilweise: Spielen „unter 3 % der Echtzeit“, mit Rechenweg. Setzen und Laufdauer fehlen. | ME:114-126 |

**Größen** (ME:99-112):
- f_n/f_1 der Biegemoden: Kurve
- Torsionsmoden: Kurve
- σ_n: Kurve je Mode
- Abstrahlgewicht: Kurve
- Schlägel: fest je Art
- Anschlaggeschwindigkeit: Stärke
- Anschlagstelle: Spielweise
- Resonator: Kurve
- Motor: Spielweise/Ausstattung
- Dämpfer: Ausstattung
- Rhodes, Wurlitzer, Kalimba: je Kurve
- Steeldrum: Kopplungen offen

**Datensätze:**
- **Iowa MIS:** pp/mf/ff, mehrere Schlägel, Oktavdateien vor 2012. Ab 2011 drei Earthworks im Decca Tree, 2012 nur ff einzeln (AU:12-15, 27).
- **VCSL Marimba:** 10 Töne × soft/med/loud; Vibraphone Soft 11 Töne × 2 (AU:28).
- **VSCO:** eine Stärke (AU:29).
- **Röhrenglocken (Freesound):** p, 6 Mikrofone (AU:31).
- **SASS-E:** viele Stärken und Anschlagstellen, aber NC (AU:37).

Einschätzung: Iowa ab 2011 könnte über drei Kanäle eine Streuung liefern. Ob die Dateien wirklich mehrkanalig sind, ist nicht vermerkt; die Dateien sind „nicht gelesen“ (AU:27, 47).

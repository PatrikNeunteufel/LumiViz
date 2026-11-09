# Prüfung IC3: Klassen 9–11 und 0 gegen die Lehren L1–L4 und L7 des Flügels

> **Auftrag:** je Klasse `modell-entwurf.md`, `messplan.md`, `aufnahmen.md` gegen L1–L4 und L7 aus
> [LEHREN_SETZEN_FLUEGEL.md](../LEHREN_SETZEN_FLUEGEL.md) prüfen (erfüllt / teilweise / fehlt, mit Datei:Zeile);
> für Klasse 0 `koerper.md` als Modellentwurf und `aufnahmen.md`.
> **Datum:** 2026-10-27 (IC Session 3).
> **Bearbeitet von:** einem Agenten (Claude, nur lesend) — **Bericht eines Agenten**, unten **unverändert**.
> **Reichweite:** die drei Dateien je Klasse, `koerper.md`, `gemeinsam/aufnahmen.md`, ganz gelesen; `recherche/` und
> `quellen/` nicht.
> **Von der Sitzung selbst nachgelesen (Stichproben):** Klasse 10 MP:30–32 („erst wenn das ohne geht, wird über den
> Umfang gesetzt“), Klasse 9 MP:18 („2. Mode zuerst?“), Klasse 10 AU:19 (TinySOL Akkordeon). Alle Stichproben stimmen.
> **Eingeflossen in:** [../lehren-pruefung.md](../lehren-pruefung.md).

---

## Prüfbericht L1–L4 und L7 für die Klassen 9, 10, 11 und 0

Ich habe nur gelesen und nichts geschrieben. Die Pfade beginnen bei `C:\Users\patri\source\repos\Visuals_Project\cmake\InstrumentsCalculation\`. Abkürzungen: **ME** = modell-entwurf.md, **MP** = messplan.md, **AU** = aufnahmen.md.

**Für alle drei Klassen gleich:**
- Keine messplan.md enthält ein Rezept, die Toleranz aus der Streuung des Datensatzes zu gewinnen. Alle Toleranzen sind geschätzte Zahlen oder Spannen aus der Literatur.
- Keine aufnahmen.md hat eine Spalte „Toleranz messbar aus“.
- Kein Plan sieht vor, über alle Noten zugleich auszugleichen.
- Die Spalte „gültiger Bereich“ gibt es überall. Sie nennt aber Datensätze und Abschnitte der Aufnahme, keine Grenzen dafür, was die Aufnahme bestimmen kann.

### Klasse 9 – Luftstrahl und Rohr

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: Toleranzen nur geschätzt (±5 Cent, ±2 dB, ±10 %, ±3 dB); M3 und M5 nehmen Spannen aus der Literatur. In AU steht nirgends, ob ein Datensatz die Streuung hergibt. | MP:12–19; AU:6–23 |
| L2 Kurven | teilweise: Spalte „Kurve / fest / Stärke“ vorhanden, Zahl der Stützpunkte nirgends. MP plant das Setzen weder über alle Noten noch Note für Note. | ME:53–63; MP:21–22 |
| L3 Bereich | teilweise: Spalte vorhanden, aber nur mit Datensatzwahl („Iowa `nonvib`“, „stationärer Teil“). AU nennt keine Aufnahmelängen. Abklingzeiten werden nicht gesetzt. | MP:12–19 |
| L4 Schwellen | teilweise: Registerschwellen mit Hysterese, Einschwingen und Mundtöne als eigener Pfad sind genannt; „gesucht“ und „gesetzt“ werden nicht unterschieden. M3 misst Schwellen „am Modell gegen Literatur“; M5 („2. Mode zuerst?“) erkennt selbst, ohne Struktur des Bezugs. | ME:27–34; MP:16, 18 |
| L7 Rechenzeit | fehlt weitgehend: keine Zahl, nur „gleiche Größenordnung“ wie Klasse 8 (Einschätzung). Nichts je Auswertung beim Setzen, keine Laufdauer. | ME:65–69 |

**Größen (ME:53–63):**
- Bohrung, Tonlöcher, Pfeifenmaße: fest
- W, h, y0, H, b: fest bei Blockflöte und Pfeife, Kurve bei der Querflöte
- c_p/U_j, α_i, α_vc: fest
- Blasdruck über Expression: Stärke
- Ansatz der Querflöte: „Weg“ (über die Stärke)
- Vibrato-Umsetzung: Spur
- Mundtöne: Pfad
- Rauschanteil: Kurve
- Tremulant: Werk

**Datensätze laut AU:**
- Iowa MIS: pp/mf/ff, vib und nonvib (Altflöte nur vib), reflexionsarm. Kanäle und Wiederholungen sind nicht angegeben (AU:8, 17).
- VCSL Blockflöte: „keine Stufen“, Sustain/SusVib/Staccato (AU:10).
- VSCO 2: susNV, susvib, expvib (AU:19).
- VAO: Einzelpfeifen mit Messwerten (AU:20).
- Zenodo 13919050: Munddruck und Audio (AU:22).

Zu keinem Datensatz stehen Länge, Kanalzahl oder Wiederholungen.

### Klasse 10 – Durchschlagende Zunge

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: alle geschätzt (±2 Cent, ±1,5 dB, ±20 %, ±25 %, ±0,2 Hz). Einziger Bezug auf die Daten: „35 Dateien nachgestimmt — dort nur ±5 Cent“. AU sagt zu keinem Datensatz, ob er die Streuung hergibt. | MP:16–25, 18 |
| L2 Kurven | teilweise: Spalte vorhanden, viele Größen „Kurve“, keine Stützpunkte. Risiko-Satz: „Skalierung über den Tonumfang statt Probieren je Note“. MP: „erst wenn das ohne geht, wird über den Umfang gesetzt“ – kein Ausgleich über alle Noten geplant. | ME:97–112, 141–142; MP:30–32 |
| L3 Bereich | teilweise: nur Datensatzwahl. M5 „wenn die Dateien das Ende enthalten (nicht geprüft)“. Für die Schwebung (M7, 0,5–7 Hz) fehlt der Bezug zur Fensterlänge. Längen nur bei Nebenquellen: fese ≈ 9 s, Bend 2,4 s, Commons-Register 26,3 s für fünf Register. Für TinySOL fehlt die Länge. | MP:22, 24; AU:23–25 |
| L4 Schwellen | teilweise: Strukturwechsel kommen vor – Anschwingen nur über die Trägheit der Strömung, Obertöne der Zunge erst bei hohem Druck, 2. Mode und Torsion im Einschwingen, Wechsel schließende → öffnende Zunge beim Bend. Nicht als Schwellenliste gefasst, keine Unterscheidung gesucht/gesetzt. M4 erkennt Vorläufer selbst. | ME:33–36, 50–54, 71–74; MP:21 |
| L7 Rechenzeit | teilweise: beim Spielen beziffert – Masse-Feder ≈ 2 % Echtzeit je Zunge, Musette ≈ 6 %, Akkord ≈ 25 %, Balken N = 40 ≈ 20 %. Je Auswertung beim Setzen und Laufdauer in MP fehlen. | ME:133–136 |

**Größen (ME:99–112):**
- Kurve: f₀, Zungenmaße, Masse/Steife, Dämpfung, Spalt/Ruhelage, V₁ und L₂/S₂, Musette-Verstimmung
- Stärke: Quelldruck p₀
- folgt aus dem Modell: Tonhöhe über dem Druck
- Spur: Ventilöffnung Kp; Vokaltrakt (aus der Spur abgeleitet)
- Ausstattung bzw. Spur: Chöre
- fest: Gehäusefilter, Einschwingen (Torsion, 2. Mode)

**Datensätze laut AU:**
- TinySOL Akkordeon: 82 Tonhöhen × pp/mf/ff, p an 8, zusammen 689 Töne; „Instance ID“ 0–5 mit Anzahlen (0: 254 … 5: 3); mono; „Mikrofon nicht dokumentiert“ (AU:19). Ob die Instance IDs Wiederholungen sind, steht nicht da.
- Euterpea-Harmonium: eine Stärke, Stereo, 96 kHz (AU:20).
- VCSL Mundharmonika: Soft/Normal/Accented, nur Blastöne (AU:22).
- fese Melodica: eine Stärke, mono (AU:23).

### Klasse 11 – Tonrad

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | teilweise: geschätzt (±0,5 Cent, ±1 dB, ±3 dB, ±15 %, ±20 %, ±5 %). AU sagt zu keinem Datensatz, ob er die Streuung hergibt. Raum- und Nahmikrofon der M100 und den dreifachen Hochlauf auf Commons nennt AU, aber nicht als Quelle für eine Toleranz. | MP:11–20; AU:19, 21 |
| L2 Kurven | teilweise: Spalte vorhanden mit fest / je Anschlag / Passage / Stärke. Keine Größe ist eine Kurve über den Tonumfang, Stützpunkte gibt es nicht. MP plant keinen Ausgleich über alle Noten. | ME:60–74 |
| L3 Bereich | fehlt in der Sache: nur Datensatzwahl. Perkussion-T60 (ca. 250 / 900 ms) und Leslie-Zeitkonstanten werden gesetzt, aber AU nennt keine Längen. Nur Dateigrößen (11,6 / 8,6 MB) und die Einzeltöne von hammondman „1–2 s“ stehen da. | MP:17, 19; AU:18, 22 |
| L4 Schwellen | teilweise: genannt sind Perkussion („neu nur nach dem Loslassen aller Tasten“), Kontaktfolge des Tastenklicks und Leslie-Hochlauf mit getrennten Zeitkonstanten. Keine Unterscheidung gesucht/gesetzt. M7 trennt Horn und Trommel über Bandpässe, ohne Regel „Erkennung nur am Bezug“. | ME:34–37, 48–49; MP:19 |
| L7 Rechenzeit | teilweise: nur „Gering“ mit Zahl der Operationen (91 Tabellenwellen, 9 Multiplikationen, 18 LC-Glieder). Keine Zeit, nichts zum Setzen. | ME:76–80 |

**Größen (ME:62–74):**
- fest: Radfrequenzen, Radwelle/Filter/Pegel, Zugriegelstufen, Rückfaltung, Übersprechen, Leslie-Zeitkonstanten, Hornradius und Gehäuse, Leslie-Filter
- je Anschlag: Tastenklick
- Passage: Perkussion, Scanner, Leslie-Drehzahlen
- Stärke: Vorverstärker und Schweller

**Datensätze laut AU:**
- Freesound 136653/136652: stereo, je ein Hoch- und Auslauf (AU:18).
- Commons: langsam → schnell → langsam und „dreimal Hochlauf“, Echtheit unklar (AU:19).
- Zenodo 3562442: trocken/nass gepaart (AU:20).
- M100-Drones: Raum- und Nahmikrofon, ein Motorauslauf (AU:21).
- hammondman: 12 Töne d3–g5 (AU:22).

Der Generator selbst hat nur „eigene Aufnahme“ als Bezug (MP:14–18).

### Klasse 0 – gemeinsam (koerper.md als Modellentwurf, aufnahmen.md)

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz | fehlt: es gibt keinen Messplan, AU hat keine Toleranzspalte. Kanäle und Stufen sind aber genannt (siehe unten). | AU:8–15 |
| L2 Kurven | fehlt: keine Einteilung in Kurve/fest/Stärke, keine Stützpunkte. | koerper:14–26 |
| L3 Bereich | fehlt als Kriterium. Bezugslängen: Mores 1 s je Stelle (1 Hz Auflösung), 12–36 ms reflexionsfrei. | AU:10; koerper:129–132 |
| L4 Schwellen | fehlt. Als Strukturgrenze im Frequenzbereich steht nur die Nullstelle bei 400–700 Hz (Moden unten, statistischer Teil oben). Einschätzung: als Schwelle im Sinn von L4 trägt das kaum. | koerper:137–139 |
| L7 Rechenzeit | teilweise: Steinway D 820 Moden, 13 Zerlegungen, 248 s (offline); Netzgröße für andere Körper geschätzt. Weder je Ton noch je Auswertung beim Setzen. | koerper:24–26, 79–91 |

**Größen (koerper:14–16, 24–26):** je Mode Frequenz und Dämpfung, je Einkoppelpunkt eine modale Masse, je Ausgang ein Abstrahlgewicht, dazu der statistische Teil (Modendichte, mittlere Mobilität, Dämpfung) und Material und Dicke der Platte. Eine Art (Kurve/fest/Stärke) ist nicht angegeben, ebenso wenig, was davon gesetzt wird.

**Datensätze laut AU:**
- Mores: 65 Gitarren, 3 Anschlagstellen, 6 Kanäle (Hammer, 2 Aufnehmer, 3 Mikrofone), keine Einzeltöne (AU:10).
- CNSM: drei Violinen, Inhalt nur Bericht (AU:11).
- TU Berlin: 32-Kanal-Array, Note und Stärke je Ton, „Stufen und Töne ungeprüft“, Lizenz offen (AU:12).
- Salamander: „16 Stufen“; Kanäle nennt AU nicht, die zwei Kanäle stehen nur in LEHREN_SETZEN_FLUEGEL.md:33 (AU:15).

**Einschätzung:** Die Kanäle von Mores, das 32-Kanal-Array der TU Berlin, die zwei Mikrofone der M100 und die drei Stärken von TinySOL über 82 Tonhöhen würden eine Toleranz nach L1 hergeben. Keine Datei zieht diesen Schluss.

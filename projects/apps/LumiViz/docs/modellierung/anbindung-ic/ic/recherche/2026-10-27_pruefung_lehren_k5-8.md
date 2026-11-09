# Prüfung IC3: Klassen 5–8 und `gemeinsam/rohr.md` gegen die Lehren L1–L4 und L7 des Flügels

> **Auftrag:** je Klasse `modell-entwurf.md`, `messplan.md`, `aufnahmen.md` gegen L1–L4 und L7 aus
> [LEHREN_SETZEN_FLUEGEL.md](../LEHREN_SETZEN_FLUEGEL.md) prüfen (erfüllt / teilweise / fehlt, mit Datei:Zeile);
> dazu `rohr.md` als Modellentwurf für 7–9.
> **Datum:** 2026-10-27 (IC Session 3).
> **Bearbeitet von:** einem Agenten (Claude, nur lesend) — **Bericht eines Agenten**, unten **unverändert**.
> **Reichweite:** die drei Dateien je Klasse und `rohr.md`, ganz gelesen; `recherche/` und `quellen/` nicht.
> **Von der Sitzung selbst nachgelesen (Stichproben):** Klasse 7 ME:25 und ME:55 („je Note gesucht“), Klasse 6 MP:15
> („Schwelle zwischen benachbarten Stufen“), Klasse 5 AU:14 („2 Round-Robins“). Alle Stichproben stimmen.
> **Eingeflossen in:** [../lehren-pruefung.md](../lehren-pruefung.md).

---

# Prüfbericht: Klassen 5–8 gegen die Lehren L1–L4 und L7

**Ergebnis vorweg:** Keine der vier Klassen erfüllt eine der geprüften Lehren ganz.
- Alle Messpläne haben die Spalten „Toleranz (Vorschlag)“ und „gültiger Bereich“. Die Toleranzen sind aber geschätzte Zahlen und kein Rezept aus der Streuung. „Gültiger Bereich“ nennt Datensätze und Bedingungen statt der Grenzen, in denen die Aufnahme eine Größe bestimmt.
- Stützpunkte, ein Ausgleich über alle Noten, Aufnahmelängen und eine Rechenzeit fürs Setzen fehlen überall.
- Pfade unten sind relativ zu `C:\Users\patri\source\repos\Visuals_Project\cmake\InstrumentsCalculation\klassen\Klasse N - …\`.

## Klasse 5 – Membran

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz messplan | **teilweise** – Toleranzen nur geschätzt (±2 %, ±20 %, ±2 dB, ±10 Cent). Nur M1 hat einen äußeren Prüfwert (Packer ±2 %), kein Rezept aus der Streuung. | messplan.md:11–20 |
| L1 aufnahmen | **fehlt** – Es gibt keine Spalte oder Angabe dazu, ob ein Datensatz eine Streuung hergibt. Stufen und Kanäle stehen nur verstreut im Inhalt. | aufnahmen.md:9–33 |
| L2 Art je Größe | **teilweise** – Die Spalte „Kurve / fest / Stärke“ ist da. Die Zahl der Stützpunkte fehlt. | modell-entwurf.md:65–76 |
| L2 Setzen gemeinsam | **fehlt** – Ob über alle Noten zugleich oder Note für Note gesetzt wird, steht nirgends. | messplan.md |
| L3 gültiger Bereich | **teilweise** – Die Spalte nennt Datensätze und Bedingungen. Nur M5 nennt ein Zeitfenster („ersten 100–300 ms“). Für T60 (M2) steht „wie M1“, keine Grenze zur Aufnahmelänge. | messplan.md:13–17 |
| L3 Aufnahmelängen | **fehlt** – Längen der Bezugsaufnahmen stehen nicht da. Genannt ist nur, dass LumiViz Kit-Klänge höchstens 30 s lädt. | aufnahmen.md; modell-entwurf.md:18 |
| L4 Schwellen | **teilweise** – Gleiten (Spannungsmodulation) und Teppich (Mischform, Rauschanteil) sind beschrieben, aber nicht als Schwellen benannt. Keine Unterscheidung gesucht/gesetzt, kein Messen an der Struktur des Bezugs. | modell-entwurf.md:39–47; messplan.md:17–18 |
| L7 Spielen | **erfüllt** – Rund 2·10⁸ Operationen je s Ton, „deutlich unter Echtzeit“ (Rechnung aus dem Bericht). | modell-entwurf.md:80–81 |
| L7 Setzen / Lauf | **fehlt** | messplan.md |

**Größen (laut modell-entwurf.md:67–76):**
- fest:
  - Fell (Spannung, Dichte, Radius) – fest je Instrument
  - Moden mit Luftlast (f, T60, Form)
  - Kopplung der beiden Felle
  - Schlag (Masse, Steife, Exponent) – fest je Schlägel
- Stärke: Auftreffgeschwindigkeit
- Artikulation: Schlagstelle, Wirbel
- „aus der Energie“: Spannungsmodulation
- Mischform: Teppich
- Kurve: statistischer Teil oben (Herkunft offen)

**Datensätze (laut aufnahmen.md):**
- Tom-Tom-Datensatz: 144 Schläge, 9 Bauweisen × 16 Stimmungen, Pendel, ein Nahmikrofon; Wiederholungen nicht genannt (:11, :26).
- DRSKit: 13 Kanäle, 13–30 Anschläge je Instrument mit `power` (:22).
- CrocellKit: 15 Kanäle, 98 Snare-Anschläge (:23).
- VCSL Timpani: 204 Anschläge, 3 Stärken, 2 Round-Robins (:14).
- VSCO Timpani: 3 Stärken (:14).
- Tabla: 13 Bols × 50, Stärken und Mikrofon unbekannt (:16).

## Klasse 6 – Becken und Gong

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz messplan | **teilweise** – Toleranzen geschätzt (±3 %, ±20 %, ±3 dB). Nur M3 verweist mit „Schwelle zwischen benachbarten Stufen“ auf die Stufen des Datensatzes. | messplan.md:13–19 |
| L1 aufnahmen | **fehlt** – Kein Vermerk zur Streuung. | aufnahmen.md:8–27 |
| L2 Art je Größe | **teilweise** – Die Spalte ist da (meist fest, Schlag nach Stärke). Keine Kurven, keine Stützpunkte. | modell-entwurf.md:50–59 |
| L2 Setzen gemeinsam | **fehlt** – Nur die Reihenfolge ist geplant (linear vor nichtlinear). | messplan.md:21–22 |
| L3 gültiger Bereich | **teilweise** – Die Spalte nennt Datensätze. Nur M5 nennt ein Fenster (100–300 ms). T60 (M2) hat keine Grenze. | messplan.md:13–17 |
| L3 Aufnahmelängen | **fehlt** – Keine Längen angegeben, obwohl das Tamtam laut Entwurf bis 10 s klingt. | aufnahmen.md; modell-entwurf.md:32 |
| L4 Schwellen | **teilweise** – Benannt sind die Schwelle zum Crash und die chaotische Rechnung. Verglichen werden Statistiken statt Abtastwerte. Keine Unterscheidung gesucht/gesetzt, kein Messen an der Struktur des Bezugs. | modell-entwurf.md:55, 68–69; messplan.md:15, 24–25 |
| L7 Spielen | **teilweise** – Gerechnet wird vorab, nicht beim Spielen: Platte 0,15–0,99 s je s Klang, Schale bis 160 s je s. „Sekunden bis Minuten je Stufe“ ist als Einschätzung markiert. | modell-entwurf.md:18–24, 63–64 |
| L7 Setzen / Lauf | **fehlt** | messplan.md |

**Größen (laut modell-entwurf.md:52–59):**
- fest: Material (E, ρ, ν); Form (je Instrument); Dämpfung je Mode
- Stärke: Schlag (Kraft, Kontaktzeit)
- Artikulation: Schlagstelle; Hi-Hat (Öffnung, Kontakt)
- „aus der Nichtlinearität“: Gleiten beim Gong
- Ende der Note: Choke

**Datensätze (laut aufnahmen.md):**
- Iowa: pp/mf/ff je Schlagstelle; Decca Tree, 24 bit/96 kHz – wie viele Kanäle die Dateien haben, steht nicht da (:10, :21).
- VSCO Gong: 17 Stärken (:14).
- DrumGizmo: Nahmikrofon und Overheads; Hi-Hat closed 41, semi-open 36, open 47 Anschläge (:12, :22).
- Oslo: 72 Takes, soft/medium/strong (:26).
- VCSL: Hängebecken pp–fff, Hi-Hat v1–v4 (:24).

## Klasse 7 – Lippe und Rohr

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz messplan | **teilweise** – Toleranzen geschätzt (±5 Cent, ±2 dB, ±20 %), mehrfach ausdrücklich als „Einschätzung“ markiert. Kein Rezept aus der Streuung. | messplan.md:13–21 |
| L1 aufnahmen | **fehlt** – Kein Vermerk zur Streuung. | aufnahmen.md:10–33 |
| L2 Art je Größe | **teilweise** – Die Spalte ist da. Lippenmasse μ ist „Kurve“ ohne Stützpunkte. Die Lippenresonanz wird „je Note gesucht“, also ausdrücklich Note für Note. Bei der Posaune ist ein Raster von Zugstellungen genannt, ohne Zahl. | modell-entwurf.md:20–26, 52–55 |
| L2 Setzen gemeinsam | **fehlt** | messplan.md |
| L3 gültiger Bereich | **teilweise** – Die Spalte nennt Bedingungen (`novib`, Stufen, stationärer Teil). Für die Wandverluste („Abklingen nach dem Ausblasen“) ist kein Bereich angegeben. | messplan.md:15–21; modell-entwurf.md:51 |
| L3 Aufnahmelängen | **fehlt** – Längen nur bei Freesound: Didgeridoo 2:15, Alphorn 30 s. | aufnahmen.md:29–30 |
| L4 Schwellen | **teilweise** – Schmettern ist als Stufe am Rohrausgang beschrieben. „Gesucht“ ist nur für die Lippenresonanz benannt, mit einem Prüfstein vorab. Register und Anblasschwelle fehlen. Kein Messen an der Struktur des Bezugs. | modell-entwurf.md:24–31; messplan.md:23–24 |
| L7 Spielen | **erfüllt** – Modal „above 30 times faster than real-time“ (Maestre); FDTD 0,86 s je s. | modell-entwurf.md:63–66 |
| L7 Setzen / Lauf | **fehlt** | messplan.md |

**Größen (laut modell-entwurf.md:50–59):**
- fest: Bohrung (Bauplan); Wandverluste; Lippengüte Q; Lippenbreite und Ruhespalt
- Kurve: Lippenmasse μ
- je Note gesucht: Lippenresonanz
- Stärke: Mundruck
- „aus Bohrung“: Schmettern
- je Dämpfer ein Satz: Dämpfer
- Kurve „Vokal“: Vokaltrakt (Didgeridoo)

**Datensätze (laut aufnahmen.md):**
- Iowa: pp/mf/ff; Trompete mit und ohne Vibrato; Kanäle nicht genannt (:24).
- TinySOL: 5 Stufen, mono (:25).
- VSCO: Horn `mute` v1–v3 und `sus` v1–v4, Trompete Harmon/Straight (:26).
- Posaune mit 31 Spielern, mit Druckmessung (:27).
- Wiederholungen derselben Note sind bei keinem Datensatz vermerkt.

## Klasse 8 – Rohrblatt und Rohr

| Kriterium | Befund | Beleg |
|---|---|---|
| L1 Toleranz messplan | **teilweise** – Toleranzen geschätzt. M3 nennt einen Bereich aus der Literatur (Li 2016), M6 ist „offen“. | messplan.md:12–19 |
| L1 aufnahmen | **fehlt** – Kein Vermerk zur Streuung. | aufnahmen.md:6–21 |
| L2 Art je Größe | **teilweise** – Die Spalte ist da. Rohrblatt „Kurve (Lage)“ und Rauschanteil „Kurve“ ohne Stützpunkte. Bohrung fest je Griff. | modell-entwurf.md:44–53 |
| L2 Setzen gemeinsam | **fehlt** | messplan.md |
| L3 gültiger Bereich | **teilweise** – Die Spalte nennt Bedingungen; der Raumboden ist über `ambient.silence` vorgesehen (M5). Keine Grenzen. | messplan.md:14–19 |
| L3 Aufnahmelängen | **fehlt** | aufnahmen.md |
| L4 Schwellen | **teilweise** – Anblasschwelle als Prüfstein (2–4 kPa) und die von der Lippenkraft abhängige Schwelle sind benannt; Bend „je Register“. Kein Registerwechsel als Strukturwechsel, keine Unterscheidung gesucht/gesetzt. | messplan.md:19, 21–22; modell-entwurf.md:49 |
| L7 Spielen | **erfüllt** – Rund 3 % der Echtzeit; mit Rohrblatt-Kontakt etwas mehr (Einschätzung). | modell-entwurf.md:57–59 |
| L7 Setzen / Lauf | **fehlt** | messplan.md |

**Größen (laut modell-entwurf.md:46–53):**
- fest je Griff: Bohrung und Tonlöcher
- Kurve: Rohrblatt (Masse, Steife, Dämpfung, Ruheöffnung, Bahn); Rauschanteil
- Stärke: Blasdruck; Lippenkraft
- fest je Artikulation: Zunge
- Spur: Vibrato
- ohne Angabe der Art (nur Verweis auf F-R5): Vokaltrakt

**Datensätze (laut aufnahmen.md):**
- Iowa: pp/mf/ff; Altsaxophon mit und ohne Vibrato; Fagott nur mit Vibrato (:9, :12, :16).
- TinySOL: 5 Stufen (:17).
- Good-Sounds: „good“- und „bad“-Töne (:18).
- VCSL Tenorsaxophon: ohne Stufen (:10).
- MIPCAT: Blasdruck, Zungenkontakt (:21).
- Kanäle und Wiederholungen sind bei keinem Datensatz vermerkt.

## gemeinsam\rohr.md (als Modellentwurf für 7–9)

- **L2:** Die Moden sollen „als **Kurven über den Tonumfang** (je Griff)“ vorliegen (rohr.md:54–57). Die Zahl der Stützpunkte fehlt.
- **L4:** Kipppunkt ist nur die Wahl des Verfahrens – Legato, Halbventil und Schmettern als Grund für FDTD (:70–71). Register und Schwellen beim Setzen sind nicht behandelt.
- **L7:**
  - modal: „10–30 Moden, weit unter 1 % der Echtzeit“, als Einschätzung markiert (:32)
  - FDTD aus der Literatur: 0,86 s je s; 3,9 s für 5 s Klang (:38–45)
  - Vergleich mit der Grenze von LumiViz (:47–50)
  - Eine Rechenzeit fürs Setzen fehlt.
- **L1/L3:** nicht Gegenstand der Datei.

**Einschätzung:** Mit den Angaben aus aufnahmen.md ließe sich die Spalte „Toleranz messbar aus“ am ehesten so füllen:
- **über Stufen:** Iowa (3), TinySOL (5), VSCO Gong (17), DrumGizmo (`power`)
- **über Kanäle:** DrumGizmo (13 bzw. 15)
- **über Wiederholungen:** VCSL Timpani (2 Round-Robins), DrumGizmo (viele Anschläge je Instrument)
- Für Iowa und den Tom-Tom-Datensatz bleiben Kanäle und Wiederholungen ungeklärt.

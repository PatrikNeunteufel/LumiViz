# LumiViz — Klang aus einer Form (Konzept)

> **Version:** 0.2.0
> **Datum:** 2026-10-13 (Session 120)
> **Typ:** Konzept — das Grobkonzept
> [fourier-shape-loops-konzept.md](../fourier-shape-loops-konzept.md) auf
> LumiViz umgeschrieben. Entschieden sind Z1 a bis Z5 a (Patrik S119); alles
> andere ist offen oder Vorschlag und so bezeichnet. Nichts entworfen, nichts
> gebaut, keine Skizze.
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Loop_Konzept.md](Loop_Konzept.md) §4f (Instrumente aus
> Schichten), §4j (Signal path) · `Offene_Punkte.md` §7
> **Sprache:** Deutsch

---

## 1. Ziel

Eine geschlossene Kontur — gezeichnet, aus einem SVG oder aus einem Bild
gewählt — wird zur **Schwingung einer Schicht** im Instrument-Creator: die
Waveform **Shape**. Der Stift, der die Kontur einmal umfährt, ist eine
Periode des Tons; wie genau er sie nachfährt (die Zahl der Harmonischen), ist
der Klangregler. Dieselbe Form soll auch den Visuals dienen: auf beide Seiten
des Stereo-Tons gelegt, zeichnet ein Scope sie wieder.

Der Ton entsteht **in LumiViz**, in C++, als Teil eines Instruments — kein
Werkzeug daneben, kein Umweg über exportierte Dateien (Z1 a).

---

## 2. Was vom Grobkonzept bleibt

Das Grobkonzept beschreibt ein eigenständiges Python-Werkzeug, das
Wavetables für fremde Synths schreibt. Für LumiViz gilt:

| Grobkonzept | In LumiViz |
|---|---|
| §1 Grundidee, §2.2 bis §2.6 die Rechnung | bleibt — §5 hier |
| §1 Modus Stereo-XY | der Haken **Stereo XY** (Z2 a) |
| §1 Modus Mono-Projektion, Winkel θ | der Regler **Angle** (Z2 a) |
| §1 Modus Wavetable, §3.2 Frames zu 2048, `clm`-Chunk | entfällt für den Anfang — der Export für andere Synths ist eine spätere Zugabe (Z1 a) |
| §2.1 Eingabe SVG, PNG, CSV über Python-Bibliotheken | zeichnen, SVG, Konturen aus einem Bild — in C++, §8 hier |
| §2.1 „ein einzelner geschlossener Pfad" | mehrere Konturen gehen beide Wege (Z5 a), §9 hier |
| §3.1 Bandbegrenzung | bleibt, je Note gerechnet (Z4 a) |
| §3.2 Loop-Länge, ganzzahlige Perioden | entfällt — der Ton einer Note wird gerechnet wie jede andere Schwingung, die Phase läuft frei |
| §4 Evolving Loops: K-Morph | wird **Harmonics envelope** (Z4 a) |
| §4 θ-Rotation, Form-Morph | nicht entschieden; nicht in den ersten Stufen |
| §6 Referenz in numpy, §7 Projektstruktur, CLI | entfällt (Z1 a: alles in C++) |
| §8 Validierung | wird zu Tests, §12 hier |
| §9 Echtzeit-Oszillator | nicht nötig: ein Ton wird vorab gerechnet (§3) |
| §10 Offene Entscheide | beantwortet: Ziel ist LumiViz selbst (Z1 a); Stereo-XY und Mono beide (Z2 a); K läuft weich (Z4 a) |

---

## 3. Was es schon gibt

Nachgesehen in S119 und S120, je mit der Stelle.

| Baustein | Stand | Folge für die Form |
|---|---|---|
| **Schwingung einer Schicht** | `enum class Schwingung` in `loop/Instrument.hpp`: Sinus, Dreieck, Saege, Rechteck, Saite, Sample, Rauschen | Shape wird ein achter Wert |
| **Ein Ton wird vorab gerechnet** | `rendereTon(instrument, hoehe, dauerMs, rate)` rechnet je Schicht in einen Puffer und addiert (`Instrument.cpp`); der Rechenkern spielt den fertigen Klang | die Rechnung muss nicht in Echtzeit laufen; Harmonics envelope ist eine Schleife über die Note, kein Live-Regler |
| **Stereo** | der Puffer je Schicht ist verschränkt links/rechts; gerechnete Schwingungen sind auf beiden Seiten gleich, Sample-Schichten dürfen abweichen | Stereo XY passt in den Puffer, wie er ist |
| **Panorama** | eine Schicht hat keines (`Schicht` trägt nur `laut`) | Pan je Schicht ist neu (Z3 a) — neue Fassung der `.lvinst`, heute 7 |
| **Signal path** | hinter Source stehen Filter, Drive, Envelope in freier Folge (`Schicht::reihe`) | die Shape-Schicht geht durch dieselbe Reihe; zu klären, ob Filter und Drive beide Seiten gleich behandeln |
| **Hüllkurve der Schicht** | Attack, Decay, Sustain, Release an der Schicht; die Filter envelope folgt ihr | Harmonics envelope folgt ihr genauso |
| **Vier Schichten** | `kSchichtenMax = 4` | As layers endet bei vier Konturen (Z5 a) |
| **Die Visuals bekommen beide Seiten** | `MainWindow.cpp`, Zufuhr an die Visualizer: spielt das Loop-Panel, kommt die Wellenform vom Mixer des Loops (`loop::tonMixer`), unter dem Composer von dessen Mixer; der Mixer hat zwei Kanäle (`BassEngine::createMixer`), `updateAudioStereo` reicht links und rechts getrennt weiter — **512 Werte je Seite und Bild** | ein XY-Scope hat seine Daten; die Frage „nicht geprüft" aus S119 ist damit beantwortet. Grenze: siehe §10 |
| **Wellenform je Seite in der Effect Chain** | `getWaveformChannel(0/1)`, im Skript `getosc` mit Kanal | ein XY-Scope lässt sich heute schon als Superscope-Skript schreiben (Vorgabe Lissajous gibt es) |
| **Qt Svg** | liegt in der Installation (`C:/Qt/6.10.1/msvc2022_64`: `Qt6Svg.dll`, `lib/cmake/Qt6Svg`), steht nicht in der `Solution.json` | lässt sich dazunehmen; was es für Pfade hergibt, siehe §8 |
| **OpenCV** | liegt nicht im Repo | die Konturerkennung ist selbst zu schreiben |
| **Die Endung `.lvshape`** | **vergeben:** seit S96 die Stück-Presets des Curve Editors im Composer (`composer/FormPresets.hpp`, `asset/composer/shapes/`, `<AppData>/composer/shapes/`; im Benutzerhandbuch genannt) | die Kontur-Datei braucht eine andere Endung, oder die Presets des Composers ziehen um — §7 |
| **Eigene Daten** | Samples und Instrumente liegen im Benutzerordner (`user:<name>`), mit Export und Import (S114) | das Vorbild für die Ablage eigener Formen |

---

## 4. Entscheide

Von Patrik, S119:

| | Entscheid |
|---|---|
| **Z1 a** | In LumiViz, als Waveform einer Schicht im Instrument-Creator, mit einem eigenen Fenster für die Form; alles in C++. Der Export für andere Synths ist eine spätere Zugabe. |
| **Z2 a** | Im Grundsatz: eine Schicht mit dem Regler **Angle** (0° bis 360°, dreht die Form, bevor der Ton abgenommen wird) und dem Haken **Stereo XY** (x links, y rechts). Die Richtungen +x, +y, −x, −y für überlagerte Schichten sind die Stellungen 0°, 90°, 180°, 270°. |
| **Z3 a** | Stereo an seinem Ort: **Pan** je Schicht im Kasten Level, für jede Waveform; Stereo XY an der Shape-Schicht; bewegtes Stereo (Flanger, Chorus) bei den Effekten am Clip. |
| **Z4 a** | **Harmonics** läuft weich (die nächste Harmonische wird eingeblendet, keine Stufen); dazu **Harmonics envelope** wie Filter envelope. Hohe Noten bekommen von selbst weniger Harmonische. |
| **Z5 a** | Mehrere gewählte Konturen gehen beide Wege, im Form-Fenster wählbar, jede in einer Farbe: **As layers** (je Kontur eine Schicht, bis vier) oder **As one path** (nacheinander in einem Umlauf, für eine Schicht). |
| **F1 a** (S120) | Die Form ist eine Datei **`.lvcontour`**. Die Endung `.lvshape` bleibt bei den Stück-Presets des Composers. Im Programm heißt die Waveform weiter **Shape**. |
| ohne Buchstaben | Die Form ist eine **Datei**, die auch die Visuals lesen. **Start** und **Reverse** sind Regler der Schicht. Eingabewege: zeichnen, SVG, Konturen aus einem Bild wählen. Gewünscht: ein Modul für die Effect Chain, das die Form zeigt. |

Offen: §13.

---

## 5. Die Rechnung

### Von der Kontur zu den Koeffizienten (einmal je Form)

1. Die Kontur ist eine geschlossene Folge von Punkten. Sie wird nach
   **Bogenlänge** auf M gleich weit entfernte Punkte umgerechnet (der Stift
   fährt gleich schnell). Die anderen Fahrweisen des Grobkonzepts (§2.2:
   langsamer in Kurven, gleicher Winkel) ändern den Klang und sind nicht
   entschieden.
2. Der Schwerpunkt wird abgezogen — damit trägt der Ton keinen Gleichanteil —
   und die Form auf `max(|x|, |y|) = 1` gebracht.
3. Aus `z[m] = x[m] + i·y[m]` rechnet eine Fourier-Transformation die
   Koeffizienten `C_k`, für `k` von `−K_max` bis `K_max`. `C_0` ist null.

Die Koeffizienten sind die Form, wie der Ton sie braucht. Ob die Datei sie
trägt oder nur die Punkte, gehört zu §7.

### Von den Koeffizienten zum Ton (je Note)

Die Phase `φ` läuft mit der Tonhöhe der Note, wie bei Sinus oder Säge — also
auch mit Pitch sweep, Vibrato, Stimmen und Verstimmung, sofern das so
gewollt ist (Vorschlag, nicht entschieden).

```
z(φ) = Σ g_k · C_k · e^(i·k·φ)        über k = −K … K, k ≠ 0
```

- **Harmonics** `H` (Z4 a): das Gewicht `g_k = clip(H − |k| + 1, 0, 1)`. Bei
  `H = 3,4` sind die Harmonischen 1 bis 3 voll da, die vierte zu 40 %.
- **Harmonics envelope**: `H` wächst mit der Hüllkurve der Schicht um den
  eingestellten Betrag, wie die Filter envelope den Cutoff öffnet; negativ
  beginnt die Note als Ellipse und wächst zur Form.
- **Bandgrenze je Note**: `H` wird bei `0,45 · Rate / Tonhöhe in Hz`
  gekappt. Bei 48 kHz sind das rund 390 Harmonische bei A1, 98 bei A3, 24 bei
  A5. Die Folge ist gewollt (Z4 a) und heißt: dieselbe Form klingt hoch
  gespielt runder — und sieht im Scope runder aus.
- **Angle** `θ`: `z` wird um `θ` gedreht, `z' = e^(−i·θ) · z`.
  - Ohne Stereo XY ist der Ton `Re(z')` auf beiden Seiten: bei 0° die
    x-Bewegung des Stifts, bei 90° die y-Bewegung, bei 180° und 270° dasselbe
    mit umgekehrtem Vorzeichen. Dazwischen ändert sich die Klangfarbe
    stufenlos.
  - Mit **Stereo XY** ist links `Re(z')`, rechts `Im(z')` — Angle dreht dann
    das Bild im Scope (Vorschlag; in S119 nicht besprochen, was Angle bei
    gesetztem Haken tut).
- **Start**: ein anderer Startpunkt auf der Kontur, `C_k · e^(i·k·t0)`.
- **Reverse**: die andere Umlaufrichtung, `C_k` und `C_−k` tauschen.

### Was Start und Reverse hörbar tun — eine Einschränkung

Beide ändern nur die **Phasen** der Teiltöne, nicht ihre Stärke. Ein
gehaltener Ton einer Schicht allein klingt mit jedem Start und in beiden
Richtungen gleich; der Wellenzug sieht anders aus. Hörbar werden sie

- hinter **Drive** (ein Verzerrer hängt an der Gestalt des Wellenzugs),
- über einer **zweiten Schicht derselben Form** (Teiltöne löschen oder
  verstärken sich),
- im Bild: Reverse dreht die Zeichenrichtung im Scope um, Start verschiebt,
  wo der Stift beginnt.

Angle dagegen ändert die Stärke der Teiltöne und ist immer hörbar. Das
gehört in die Tooltips und ins Handbuch, sonst wirken zwei Regler tot.

### Offen an der Rechnung

- **Ecken:** eine abgeschnittene Reihe schwingt an Ecken über, hörbar als
  Zischeln. Das Grobkonzept dämpft mit Lanczos-Faktoren (§2.4). Ob das fest
  eingebaut ist, ein Regler wird oder entfällt, ist nicht entschieden; es
  macht den Klang dunkler und die Ecken im Scope runder.
- **Pegel:** die Spitze des Tons hängt von Form, Harmonics und Angle ab. Ob
  der Ton je Einstellung auf gleiche Spitze gebracht wird oder so laut ist,
  wie er herauskommt, ist offen. Unter einer Harmonics envelope darf sich
  der Pegel nicht sprunghaft ändern.
- **Rechenzeit:** je Wert `2·K` Sinus und Cosinus; ein Ton von 30 Sekunden
  mit 100 Harmonischen sind knapp 300 Millionen. Vorab gerechnet ist das
  möglich, aber nicht geschenkt — vor dem Bau messen. Ein Ausweg: Tabellen
  je ganzer Zahl von Harmonischen, zwischen denen geblendet wird; das ist
  rechnerisch dasselbe wie das Gewicht `g_k`.
- **M und K_max:** wie fein die Kontur abgetastet wird und wie viele
  Koeffizienten die Form trägt (das Grobkonzept nennt M = 8192).

---

## 6. Die Shape-Schicht im Creator

Die Regler, soweit besprochen. Wo sie stehen und wie die Schicht die Form
zeigt, gehört in die Skizze (§13, Punkt 6) — hier steht kein Entwurf.

| Regler | Bereich | Stand |
|---|---|---|
| **Shape** — die Form, mit dem Weg ins Form-Fenster | eine Datei | entschieden im Grundsatz (Z1 a); Name des Knopfs offen |
| **Harmonics** | 1 bis zur Bandgrenze, stufenlos | Z4 a; die obere Zahl offen |
| **Harmonics envelope** | positiv und negativ | Z4 a; der Bereich offen |
| **Angle** | 0° bis 360° | Z2 a |
| **Stereo XY** | Haken | Z2 a |
| **Start** | einmal um die Kontur | Patrik: ja; Einheit offen (Grad, Prozent) |
| **Reverse** | Haken | Patrik: ja |
| **Pan** | im Kasten Level, bei jeder Waveform | Z3 a; §6a |

Zu klären mit der Skizze: was aus **Pan** wird, wenn Stereo XY gesetzt ist
(die Seiten tragen dann verschiedenes — Pan als Balance, oder ausgegraut).

### 6a. Pan je Schicht — eine eigene Stufe davor

Pan gilt für jede Waveform und braucht die Form nicht (Z3 a; schließt U19).
Es ist die kleinste Stufe und kommt zuerst: ein Regler im Kasten Level, ein
Feld an `Schicht`, **Fassung 8 der `.lvinst`** — eine eigene Fassung aus dem
bekannten Grund: ein älteres Programm spielte die Schicht sonst still in der
Mitte. Offen: das Gesetz (gleiche Leistung oder linear), und was Pan an einer
Sample-Schicht mit zwei verschiedenen Seiten tut. Die mitgelieferten
Instrumente dürfen sich dabei nicht ändern (`test_LoopWerte` hält sie fest).

---

## 7. Die Datei

Patrik: die Form ist eine Datei, und die Visuals lesen dieselbe.

**Die Endung `.lvshape` ist vergeben** (§3): seit S96 heißen so die
Stück-Presets des Curve Editors. In S119 ist das nicht aufgefallen. Zwei
Dateiarten unter einer Endung gehen nicht — ein Öffnen-Dialog und der Export
der eigenen Daten könnten sie nicht unterscheiden. **Entschieden (F1 a,
Patrik S120): die Kontur-Datei heißt `.lvcontour`**; verworfen: die Presets
des Composers umbenennen (b), beide Arten unter einer Endung (c).

Noch zu entscheiden (§13, Punkt 1):

- der **Inhalt**: die Konturen als Punkte oder Kurven, ihre Farben, welche
  gewählt sind, As layers oder As one path, die Herkunft (gezeichnet, aus
  welchem SVG, aus welchem Bild und mit welchen Einstellungen der
  Erkennung); ob die Koeffizienten mit in der Datei stehen oder beim Laden
  gerechnet werden;
- der **Ort**: mitgelieferte Formen unter `asset/`, eigene im Benutzerordner
  wie Samples (`user:<name>`), eine Datei an freiem Ort beim Pfad; ob eigene
  Formen in Export und Import der eigenen Daten kommen;
- wie ein **Instrument** die Form nennt, und was geschieht, wenn sie fehlt
  (eine Sample-Schicht ohne Aufnahme schweigt);
- ob ein Instrument die Form **einbettet** statt sie zu nennen — dann klingt
  es weiter, auch wenn die Datei geändert oder gelöscht wird; bei Samples
  ist anders entschieden (Stufe 17, M3 a: alles spielt die neue Fassung).

---

## 8. Die Eingabewege

Alle drei enden im selben Fenster und in derselben Datei: einer Liste
geschlossener Konturen, jede in einer Farbe, einige davon gewählt.

**Zeichnen.** Mit der Maus eine geschlossene Linie ziehen. Offen: frei Hand
oder Punkt für Punkt, glätten, nachträglich Punkte schieben.

**SVG.** Qt Svg liegt in der Installation. Nach meinem Wissen zeichnet es
ein SVG, gibt aber die Pfade nicht als Geometrie heraus — **nicht
nachgeprüft**. Trifft das zu, bleiben zwei Wege: die Pfade der Datei selbst
lesen (die Pfadsprache von SVG in einen `QPainterPath` übersetzen, ohne Qt
Svg), oder das SVG als Bild zeichnen und denselben Weg gehen wie ein Foto.
Der zweite ist weniger Code und verliert die Genauigkeit des SVG. Vorlage
(§13, Punkt 3), davor nachsehen.

**Bild.** Konturen in einem jpg oder png finden, anzeigen, auswählen lassen.
Ohne OpenCV selbst zu schreiben; die Bausteine sind bekannt (nach Helligkeit
oder Farbe trennen, die Ränder der Flächen ablaufen, die Punktfolge
ausdünnen). Offen: welche Filter der Benutzer einstellt, wie viele Konturen
angeboten werden, was mit Löchern und sehr kleinen Flächen geschieht.
Vorlage (§13, Punkt 2).

---

## 9. Mehrere Konturen

**As layers** (Z5 a): je gewählter Kontur eine Schicht in ihrer Farbe, bis
das Instrument voll ist (vier). Offen: ob jede Schicht ihre eigene Datei
bekommt oder alle dieselbe nennen, mit der Nummer der Kontur.

**As one path** (Z5 a): die gewählten Konturen nacheinander in einem Umlauf.
Nur so erscheint im Scope eine Zeichnung aus mehreren Teilen. Offen ist die
**Brücke**: der Stift muss von einer Kontur zur nächsten, und dieser Weg ist
im Scope als Strich zu sehen und im Ton zu hören. Zu klären: wo er ansetzt
(nächste Punkte), in welcher Reihenfolge die Konturen kommen, ob der Stift
auf der Brücke schneller fährt (dann ist der Strich blasser und der Ton
schärfer), und wie viel vom Umlauf jede Kontur bekommt (nach Länge oder
gleich). Vorlage (§13, Punkt 4).

---

## 10. Die Visuals

Zwei Module für die Effect Chain, beide gewünscht, die Reihenfolge offen
(§13, Punkt 5):

1. **XY-Scope** — zeichnet links gegen rechts. Braucht die Form nicht, zeigt
   jeden Stereo-Ton, also auch eine Shape-Schicht mit Stereo XY. Die Daten
   sind da (§3).
2. **Form aus den Koeffizienten** — liest die Datei und zeichnet die Form
   selbst, mit so vielen Harmonischen wie gewünscht; unabhängig vom Ton,
   also auch zu einem Song. Die Epizykel-Zeichnung, die den Anstoß gab,
   wäre hier zu Hause.

**Grenzen des XY-Scope, vor dem Bau zu wissen:**

- Ein Bild bekommt **512 Werte je Seite**. Bei 48 kHz sind das knapp 11 ms:
  ein Ton unter rund 94 Hz zeigt in einem Bild keinen ganzen Umlauf, die Form
  bleibt offen. (Die Rate der Engine habe ich nicht nachgesehen.) Ein Scope
  mit Nachleuchten schließt die Lücke, oder die Zufuhr wird länger.
- Der Scope zeichnet **die Summe** dessen, was spielt: zwei Schichten mit
  Stereo XY ergeben eine Kurve, nicht zwei Formen; andere Spuren, Echo und
  Hall am Clip verwischen das Bild. Sauber steht die Form nur, wenn die
  Shape-Schicht allein und trocken spielt.
- Ein **Filter oder Drive** hinter Source verformt das Bild, und
  **Stimmen** mit Verstimmung lassen es wandern.

---

## 11. Später, nicht in den ersten Stufen

- Export als Wavetable für andere Synths (Z1 a: spätere Zugabe).
- Angle, das von selbst umläuft; ein Übergang zwischen zwei Formen
  (Grobkonzept §4).
- Andere Fahrweisen des Stifts als Regler (Grobkonzept §2.2).
- Aus einem gewünschten Klang eine Form suchen (Grobkonzept §9).

---

## 12. Tests

Aus der Validierung des Grobkonzepts (§8), ohne Ton und ohne
Qt-Anwendung, an gerechneten Formen (Kreis, Quadrat, eine Form mit Ecken):

- ein Kreis ergibt einen reinen Sinus, links und rechts um ein Viertel
  versetzt;
- über der eingestellten Zahl von Harmonischen und über der Bandgrenze liegt
  nichts; kein Gleichanteil;
- die Form, aus ihren Koeffizienten zurückgerechnet, weicht von der Kontur
  um weniger als eine festgelegte Schwelle ab;
- Angle 180° ist Angle 0° mit umgekehrtem Vorzeichen; Start und Reverse
  ändern die Stärke der Teiltöne nicht;
- Harmonics zwischen zwei ganzen Werten liegt zwischen den beiden Tönen; eine
  Harmonics envelope ergibt keinen Sprung;
- die Datei: schreiben, lesen, eine höhere Fassung ablehnen;
- **alte Instrumente klingen Bit für Bit wie vorher** — `test_LoopWerte`
  bleibt unverändert grün, nach Pan und nach Shape.

Dazu wie bisher Bilder aus den Tests (das Form-Fenster, die Shape-Schicht)
und Hörproben in einen Ordner.

---

## 13. Offen — einzeln vorzulegen, in dieser Reihenfolge

1. **Die Datei** `.lvcontour` (Endung entschieden, F1 a): Inhalt, Ort, wie
   ein Instrument sie nennt (§7).
2. **Konturerkennung** aus einem Bild, ohne OpenCV (§8).
3. **SVG:** Pfade selbst lesen oder über das Bild (§8) — davor nachsehen,
   was Qt Svg hergibt.
4. **Die Brücke** bei As one path (§9).
5. **Die zwei Module** für die Effect Chain und welches zuerst (§10).
6. **Die Skizze** des Form-Fensters und der Shape-Schicht — mit dem
   Zeichenwerkzeug, vor jedem Bau; die gebilligte Skizze kommt als
   Beschreibung in dieses Dokument.
7. **Baustufen** (§14).

Daneben, klein, wo sie hingehören: Ecken dämpfen, Pegel, Angle bei gesetztem
Stereo XY, Pan bei gesetztem Stereo XY, ob Stimmen und Pitch sweep für Shape
gelten (§5, §6).

---

## 14. Stufen (Vorschlag, nicht freigegeben)

Die Nummern schließen an `Loop_Konzept.md` §5 an (zuletzt Stufe 17).

| Stufe | Inhalt | hängt an |
|---|---|---|
| 18 | **Pan je Schicht**, `.lvinst` Fassung 8 | nichts — sofort möglich |
| 19a | Form als Daten: Kontur, Koeffizienten, die Datei | Vorlage 1 |
| 19b | die Schwingung Shape im Klangerzeuger: Harmonics, Angle, Stereo XY, Start, Reverse, Harmonics envelope; mit einer Form aus einer Datei, ohne Fenster | 19a |
| 19c | die Shape-Schicht im Creator | Skizze |
| 20a | das Form-Fenster: zeichnen | Skizze |
| 20b | SVG | Vorlage 3 |
| 20c | Konturen aus einem Bild | Vorlage 2 |
| 20d | mehrere Konturen: As layers, As one path | Vorlage 4 |
| 21 | die Module für die Effect Chain | Vorlage 5 |

Nach 19b ist der Klang zu hören und zu messen, bevor ein Fenster steht.

---

## 15. Risiken

- **Die Form im Scope ist empfindlicher als der Klang** (§10): tiefe Töne,
  mehrere Schichten und Effekte zerstören das Bild. Wer das Bild will, muss
  wissen, wie er spielt — das gehört ins Handbuch und entscheidet, ob das
  zweite Modul (Form aus den Koeffizienten) das wichtigere ist.
- **Konturerkennung** ist der Teil mit dem größten Aufwand und dem
  unsichersten Ergebnis; an Fotos wird sie viele kleine Konturen liefern.
- **Rechenzeit** langer Töne mit vielen Harmonischen (§5) — messen vor dem
  Bau der Envelope.
- **Zwei Regler ohne hörbare Wirkung** an einer Schicht allein (Start,
  Reverse, §5).

---

## 16. Changelog

| Version | Datum | Inhalt |
|---|---|---|
| 0.2.0 | 2026-10-13 (S120) | **F1 a (Patrik):** die Kontur-Datei heißt `.lvcontour`, `.lvshape` bleibt beim Composer (§4, §7, §13). Das Datei-Icon dazu ist gebaut (`asset/img/logo/icons/lumiviz_lvcontour.ico`, Marke mit Kontur und Stift). |
| 0.1.0 | 2026-10-13 (S120) | Erste Fassung: das Grobkonzept auf LumiViz umgeschrieben, Entscheide Z1 a bis Z5 a eingetragen. Nachgesehen: die Visuals bekommen vom Loop beide Seiten getrennt (512 Werte je Seite und Bild); Qt Svg liegt in der Installation; die Endung `.lvshape` ist seit S96 vergeben. Festgehalten: Start und Reverse ändern nur Phasen. Stufen als Vorschlag. |

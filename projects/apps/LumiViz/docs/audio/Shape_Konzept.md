# LumiViz — Klang aus einer Form (Konzept)

> **Version:** 0.19.0
> **Datum:** 2026-10-14 (Session 122)
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
| **Vier Schichten** | `kSchichtenMax = 4`; in S120 gemessen und mit 8 probiert — es trägt; **geht mit Stufe 18 auf acht** (Patrik) | As layers reicht dann für acht Konturen (Z5 a nannte vier) |
| **Die Visuals bekommen beide Seiten** | `MainWindow.cpp`, Zufuhr an die Visualizer: spielt das Loop-Panel, kommt die Wellenform vom Mixer des Loops (`loop::tonMixer`), unter dem Composer von dessen Mixer; der Mixer hat zwei Kanäle (`BassEngine::createMixer`), `updateAudioStereo` reicht links und rechts getrennt weiter — **512 Werte je Seite und Bild** | ein XY-Scope hat seine Daten; die Frage „nicht geprüft" aus S119 ist damit beantwortet. Grenze: siehe §10 |
| **Wellenform je Seite in der Effect Chain** | `getWaveformChannel(0/1)`, im Skript `getosc` mit Kanal | die Daten für ein XY-Scope sind da. **Berichtigt (S120):** die Vorgabe Lissajous des Superscope ist eine gerechnete Figur (`sin` gegen `cos` mit zwei Frequenzen), kein Scope von links gegen rechts — in S119 war sie so verstanden worden. Ob sich ein XY-Scope heute schon als Skript schreiben lässt, ist nicht nachgesehen (Vorlage 5) |
| **Qt Svg** | liegt in der Installation (`C:/Qt/6.10.1/msvc2022_64`: `Qt6Svg.dll`, `lib/cmake/Qt6Svg`); seit S120 in der `Solution.json` | gibt die Pfade nicht heraus, zeichnet sie aber auf ein mitschreibendes Gerät — §8 |
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
| **D1 a** (S120) | Eine `.lvcontour` ist eine Zeichnung: alle Konturen mit Farbe und Auswahl, die Reihenfolge, As layers oder As one path. |
| **D2 a** (S120) | Ort und Nennung wie bei Instrumenten und Samples: `asset/contours`, eigene unter `contours` im Benutzerordner als `user:<name>`, freie Dateien beim Pfad; ein Bereich **Shapes** in Export und Import. |
| **D3 a** (S120) | Das Instrument nennt die Form (Datei und Kontur über eine feste Kennung); alles spielt die neue Fassung. Für das Teilen: **Publish** als eigener Punkt (Patrik). |
| **D4 b** (S120) | Konturen als Pfade aus Strecken und Kurven, in den Koordinaten der Zeichnung; Herkunft mit Namen der Quelle, ohne das Bild; keine Koeffizienten in der Datei. |
| **K1 a** (S120) | Konturen aus einem Bild über Flächen; **Split by** Brightness, Colour oder Transparency. |
| **K2 a** (S120) | Regler Split by, Levels, Smooth, Min size, Detail; Löcher sind eigene Konturen, Flächen am Bildrand anfangs nicht gewählt, die größten 64 werden angeboten. |
| **S1 a** (S120) | SVG: Qt Svg zeichnet auf ein mitschreibendes Zeichengerät; Kurven, Lage und Füllfarbe kommen an (belegt). |
| **B2 d** (S120) | As one path: jede Kontur behält ihre eigene Reihe und bleibt geschlossen; nacheinander in einem Umlauf, dazwischen ein kurzer gerader Sprung; Harmonics je Kontur. |
| **B1** (S120) | Wo der Stift springt, ist wählbar: in Reihenfolge der Liste (Vorgabe), auf den kürzesten Wegen, oder von Hand eingezeichnet mit Start- und Endpunkt. |
| **M1 a** (S120) | Ein neuer Knoten **XY Scope**: links gegen rechts in voller Genauigkeit; Gain, Line width, Farbe, Rotate 45°. |
| **M2 b** (S120) | Ein neuer Knoten **Shape**: zeichnet die gewählten Konturen einer `.lvcontour`, mit Harmonics und **Show circles**. Beide Knoten mit den üblichen Skriptfeldern und einigen Presets. |
| **M3 a** (S120) | Shape gleich nach Datei und Koeffizienten, XY Scope nach dem ersten Ton. |
| **Skizzen** (S120) | Das Fenster Edit shape und die Shape-Schicht, gebilligt; die Preview berichtigt (ein Wellenzug hat je Zeitpunkt einen Wert). |
| **G1 a** (S120) | Die Schicht entscheidet über ihre Zeile Contour, was sie spielt; Use as im Fenster sagt, was beim Speichern aus dem Creator geschieht. |
| **G2 a** (S120) | Sprünge von Hand: zwei an einer Kontur erlaubt, ein Ring wird abgelehnt, unerreichte Konturen werden in der Reihenfolge der Liste angehängt. |
| **V1 a bis V4 a** (S121) | Der Knoten Shape: fünf Skriptfelder mit Per point, die Regler, neun Presets; XY Scope bekommt dieselben fünf Felder (§10). |
| **T1 a** (S121) | Der Pegel einer Shape-Schicht: so laut, wie er herauskommt, kein Ausgleich je Einstellung — aber die Form wird für den Ton am **fernsten Punkt** gemessen (Abstand 1 zur Mitte), nicht am größten Ausschlag in x oder y. |
| **T2 a** (S121) | **Angle dreht die Form auch mit Stereo XY**: links die x-Bewegung der gedrehten Form, rechts ihre y-Bewegung. |
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
  gekappt. Bei 44,1 kHz (der Rate der Engine) sind das rund 360 Harmonische
  bei A1, 90 bei A3, 22 bei A5. Die Folge ist gewollt (Z4 a) und heißt: dieselbe Form klingt hoch
  gespielt runder — und sieht im Scope runder aus.
- **Angle** `θ`: `z` wird um `θ` gedreht, `z' = e^(−i·θ) · z`.
  - Ohne Stereo XY ist der Ton `Re(z')` auf beiden Seiten: bei 0° die
    x-Bewegung des Stifts, bei 90° die y-Bewegung, bei 180° und 270° dasselbe
    mit umgekehrtem Vorzeichen. Dazwischen ändert sich die Klangfarbe
    stufenlos.
  - Mit **Stereo XY** ist links `Re(z')`, rechts `Im(z')` — Angle dreht dann
    das Bild im Scope, und im Ton wandert Klangfarbe zwischen den Seiten
    (**T2 a**, Patrik S121). Verworfen: Angle ist mit dem Haken ohne Wirkung
    und ausgegraut (b).
- **Start**: ein anderer Startpunkt auf der Kontur, `C_k · e^(i·k·t0)`.
- **Reverse**: die andere Umlaufrichtung, `C_k` und `C_−k` tauschen.

**Mehrere Konturen in einer Schicht (As one path, B2 d):** die Summe oben
gilt **je Kontur** — jede hat ihre eigenen Koeffizienten und ihr eigenes
Gewicht `g_k`, und anders als bei einer einzelnen Kontur behält sie dabei
ihre Lage in der Zeichnung (`C_0` bleibt). Die Phase eines Umlaufs ist auf
die Konturen verteilt, nach ihrer Länge; dazwischen liegt je ein kurzer
gerader Sprung (§9).

### Wie die Schwingung gebaut ist (Stufe 19c, S121)

- **`shape/Tonform`** (`lumi::shape`, ohne Engine und ohne Qt-Anwendung):
  `tonform(zeichnung, kontur)` bereitet auf — eine Kontur um ihren
  Schwerpunkt, der fernste Punkt im Abstand 1 (T1 a); bei `kontur = 0` alle
  Konturen mit Haken als ein Pfad (§9). `tonwert(form, umlauf, harmonische,
  hoechste)` ist der Ort des Stifts. `tonformFuer(nennung, kontur)` merkt
  sich das Ergebnis je Nennung und Kontur, auch ein vergebliches;
  `vergissTonformen` läuft, wenn eine Zeichnung geschrieben oder gelöscht
  wird. **Nicht gebaut:** dass dabei auch schon gerechnete Töne neu
  entstehen (`zaehleInstrumentFassung`) — mit dem Fenster Edit shape
  nachzuziehen.
- **`Schwingung::Form`** und an der Schicht `form`, `formKontur`,
  `harmonische` (Vorgabe 24), `harmonischeHuelle` (−128 … 128), `winkelGrad`,
  `formStartProzent`, `stereoXY`, `rueckwaerts`. Stimmen, Verstimmung, Vibrato
  und Pitch sweep gelten wie bei Sinus und Säge (so steht es in der
  gebilligten Skizze, §6).
- **`rechneForm`** im Klangerzeuger rechnet je Abtastwert die Reihe selbst,
  keine Tafel: Harmonics darf sich mit der Hüllkurve von Wert zu Wert ändern
  (`H = Harmonics + Envelope · Hüllkurve`, in 1 … 512), und es gibt nur einen
  Rechenweg. Die Bandgrenze je Note ist `0,45 · Rate / Tonhöhe`, mit demselben
  Abstand für Vibrato, Verstimmung und Pitch sweep wie bei den anderen
  Schwingungen. Start und Reverse wirken auf die Phase, Angle dreht `z`;
  Pan gilt wie bei jeder Schicht (jede Seite mit ihrem Faktor — bei Stereo XY
  also als Balance). Eine Schicht ohne Form schweigt in voller Länge.
- **`.lvinst` Fassung 9:** `waveform: shape` und `shape`, `shapeContour`,
  `shapeHarmonics`, `shapeHarmonicsEnvelope`, `shapeAngle`, `shapeStart`,
  `shapeStereoXY`, `shapeReverse` — nur eine Shape-Schicht trägt sie.
  (`harmonics` allein ist seit Fassung 1 die Liste der Obertöne des Sinus.)
  Die mitgelieferten Instrumente bleiben in ihrer Fassung und rechnen Bit für
  Bit wie vorher (`test_LoopWerte`).
- **Im Creator ist Shape noch nicht wählbar** (Stufe 19e); eine Shape-Schicht
  entsteht bis dahin nur in einer Datei von Hand oder im Test.

**Gemessen (S121, `test_ShapeTon` „Messung", Release-clang, je Sekunde Ton):**

| Schicht | Rechenzeit |
|---|---|
| zum Vergleich: eine teure Schicht mit Säge, Filter und Drive | 2,2 ms |
| Herz, A2, 24 Harmonics | 4,1 ms |
| Herz, A2, 64 Harmonics | 8,6 ms |
| Herz, A2, 64 Harmonics, drei Stimmen, Stereo XY | 26 ms |
| Herz, A1, 512 eingestellt (die Bandgrenze lässt 340) | 40 ms |
| Herz und Stern als ein Pfad, A2, 64 Harmonics | 9,4 ms |

Eine Form für den Ton aufbereiten (zwei Konturen): 5 ms, einmal. Unter
Testing-MSVC dauert alles rund vierzigmal so lang (193 ms je Sekunde bei 24
Harmonics) — die Tests zum Ton laufen dort entsprechend. Töne werden vorab
gerechnet; im laufenden Programm ist nicht gemessen.

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
- ~~**Pegel**~~ — entschieden in S121, **T1 a** (Patrik), nach einer Messung
  an fünf mitgelieferten Formen. So wie die Form bis dahin für den Ton
  aufbereitet war (größter Ausschlag in x oder y gleich 1), lag die Spitze
  zwischen 0,62 (Herz, ein Teilton) und 1,41 (Quadrat, auf der Diagonale) —
  rund 7 dB, und das Quadrat 41 % über jeder anderen Schwingung. Seit T1 a
  wird die Form am **fernsten Punkt** gemessen (Abstand 1 zur Mitte): die
  Spitze bleibt bei jedem Winkel und jeder Zahl von Teiltönen bei höchstens
  rund 1, nichts läuft im Ton mit, unter einer Harmonics envelope springt
  nichts. Verworfen: jede Einstellung auf die Spitze 1 gebracht (b — der
  Ausgleich wäre während einer Envelope eine zweite Hüllkurve); ohne
  Ausgleich wie bisher aufbereitet (c). Nicht gehört. Kippt, wenn die
  Unterschiede zwischen den Formen beim Hören stören (das Herz liegt bei 0°
  rund 3 dB unter dem Kreis) — dann b nachziehen, die Datei ändert sich
  dadurch nicht. `normiert` (Kontur.hpp) bleibt, wie es ist; den Ton
  bereitet `shape/Tonform` auf.
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

### Die Skizze der Shape-Schicht (gebilligt, Patrik S120)

Mit dem Zeichenwerkzeug gezeigt; hier als Beschreibung, damit sie nicht neu
gezeichnet werden muss.

- **Zeile Signal path:** der Kasten **Source** zeigt „Shape · <Name der
  Form>"; der Kasten **Level** zeigt Level und Pan („80 % · Pan L 20").
- **Gruppe Oscillator**, von oben nach unten:
  - **Waveform** — die Auswahl, mit dem neuen Eintrag Shape;
  - **Shape** — der Name der Form als Text, daneben **Browse...** (eine
    Form wählen) und **Edit...** (das Fenster Edit shape);
  - **Contour** — eine Kontur der Datei beim Namen („1 Heart", „2 Star")
    oder **All, one path** (G1 a);
  - **Harmonics** — Regler mit Zahl;
  - **Harmonics envelope** — Regler von negativ bis positiv, mit Zahl;
  - **Angle** — 0° bis 360°;
  - **Start** — in Prozent des Umlaufs;
  - die Haken **Stereo XY** und **Reverse** nebeneinander;
  - darunter wie bei den anderen Schwingungen **Voices**, Detune, **Pitch
    sweep**.
- **Gruppe Level:** **Level** und neu **Pan** (L 100 bis R 100), bei jeder
  Waveform.
- **Rechts:** **One cycle** wie bisher (der Wellenzug über der Zeit);
  darunter neu **Shape, as it plays** — die Zeichnung gestrichelt, darüber
  die Form mit der eingestellten Zahl von Harmonischen, und ein Pfeil für
  Angle. Darunter die Legende „Dashed: the drawing · solid: with 24
  harmonics · arrow: Angle".

**Contour entscheidet, was die Schicht spielt (G1 a, Patrik):** eine Kontur
oder alle als ein Pfad. **Use as** im Fenster Edit shape sagt nur, was beim
Speichern aus dem Creator heraus geschieht — bei Layers legt der Creator je
gewählter Kontur eine Schicht an, bei One path stellt er die offene Schicht
auf „All, one path". Danach lässt sich Contour an jeder Schicht ändern.
Verworfen: nur das Fenster entscheidet (b); Use as entfällt, ein Knopf im
Creator (c).

Von Claude in der Skizze gesetzt, mit ihr gebilligt: die Zeile Contour, das
Bild „Shape, as it plays", Start in Prozent. Noch offen, klein: was **Pan**
tut, wenn Stereo XY gesetzt ist (die Seiten tragen dann verschiedenes — Pan
als Balance, oder ausgegraut); die Bereiche von Harmonics und Harmonics
envelope.

### Wie die Shape-Schicht im Creator gebaut ist (Stufe 19e, S121)

Nach der Skizze oben, mit diesen Abweichungen und Zusätzen:

- **Edit...** fehlt noch: das Fenster Edit shape gibt es erst mit Stufe 20a;
  ein Knopf ohne Fenster wäre tot. Neben Shape steht nur **Browse...** — ein
  Menü mit den mitgelieferten Formen, den eigenen („My shapes") und „From
  file...". Eine Datei von irgendwo wird nicht kopiert; die Schicht nennt
  ihren Pfad.
- **Harmonics** läuft am Regler in gleichen Verhältnissen (40 Schritte je
  Verdopplung, 1 bis 512) — unten, wo sich der Klang am stärksten ändert,
  stellt er fein. Der Wert steht mit einer Nachkommastelle unter 10.
- **Harmonics envelope** −128 bis +128, „off" in der Mitte; **Angle** in
  ganzen Grad; **Start** in ganzen Prozent.
- Beim ersten Wählen von Shape bekommt die Schicht die Form `heart`, damit
  gleich etwas klingt.
- **Contour** nennt die Konturen mit Platz und Namen („1 Heart"); eine
  Kennung, die es in der Form nicht (mehr) gibt, steht als „Contour 7
  (missing)" da, und die Schicht schweigt.
- **Shape, as it plays** (`FormBild`): gestrichelt ein Umlauf mit allen 512
  Teiltönen (bei mehreren Konturen samt Sprüngen), durchgezogen derselbe mit
  der eingestellten Zahl, der Pfeil in Richtung Angle (0° nach rechts, 90°
  nach oben). Die Harmonics envelope zeigt das Bild nicht.
- **One cycle** zeigt wie bei jeder Schwingung den gerechneten Ton — bei
  Stereo XY die linke Seite.
- Alles in dieser Liste ist von Claude gesetzt. Kein mitgeliefertes
  Instrument trägt eine Shape-Schicht.

### 6a. Pan je Schicht — eine eigene Stufe davor

Pan gilt für jede Waveform und braucht die Form nicht (Z3 a; schließt U19).
Es ist die kleinste Stufe und kommt zuerst: ein Regler im Kasten Level, ein
Feld an `Schicht`, **Fassung 8 der `.lvinst`** — eine eigene Fassung aus dem
bekannten Grund: ein älteres Programm spielte die Schicht sonst still in der
Mitte. Die mitgelieferten Instrumente dürfen sich dabei nicht ändern
(`test_LoopWerte` hält sie fest).

**Das Gesetz — nachgesehen, nicht neu entschieden:** Spur und Muster haben
schon ein Pan (`Rechenkern.cpp`): links `min(1, 1 − pan)`, rechts
`min(1, 1 + pan)` — in der Mitte beide Seiten voll, zur Seite hin wird die
andere leiser, bis sie schweigt. Die Schicht bekommt **dasselbe Gesetz**; in
der Mitte ändert sich damit kein Wert. Jede Seite wird mit ihrem Faktor
genommen, nichts wird gemischt — bei einer Sample-Schicht mit zwei
verschiedenen Seiten bleibt ganz links nur, was links aufgenommen ist.

**Acht Schichten (Patrik S120):** die Grenze `kSchichtenMax` geht mit dieser
Stufe von vier auf acht, in derselben Fassung 8 — ein älteres Programm läse
ein Instrument mit mehr als vier Schichten sonst still gekürzt. Gemessen:
die Rechenzeit wächst gerade mit der Zahl (Release-clang rund 2 ms je
Sekunde Ton und teurer Schicht). Zu beachten: die Spitze wächst mit (2,52
bei acht Schichten auf Level 1); der Creator zeigt sie als Peak.

---

## 7. Die Datei

Patrik: die Form ist eine Datei, und die Visuals lesen dieselbe.

**Die Endung `.lvshape` ist vergeben** (§3): seit S96 heißen so die
Stück-Presets des Curve Editors. In S119 ist das nicht aufgefallen. Zwei
Dateiarten unter einer Endung gehen nicht — ein Öffnen-Dialog und der Export
der eigenen Daten könnten sie nicht unterscheiden. **Entschieden (F1 a,
Patrik S120): die Kontur-Datei heißt `.lvcontour`**; verworfen: die Presets
des Composers umbenennen (b), beide Arten unter einer Endung (c).

### Eine Datei ist eine Zeichnung (D1 a)

Eine `.lvcontour` hält die ganze Arbeit aus dem Form-Fenster fest: **alle
Konturen**, je mit Farbe und dem Haken, ob sie gewählt ist, dazu ihre
Reihenfolge und die Wahl **As layers** oder **As one path**. Sie lässt sich
wieder öffnen, eine andere Kontur wählen, der Weg wechseln. Eine Datei mit
einer einzigen Kontur ist der einfache Fall. Das Modul der Effect Chain
zeichnet die gewählten Konturen in ihren Farben. Verworfen (b): eine Datei
je Kontur — die nicht gewählten Konturen und die Erkennung wären nach dem
Speichern verloren.

### Ort und Nennung (D2 a)

Wie bei Instrumenten und Samples (P2 a, S1 a):

| | liegt | eine Schicht nennt sie |
|---|---|---|
| mitgeliefert | `asset/contours/<name>.lvcontour` | beim Namen: `heart` |
| eigene | `<Benutzerordner>/contours/<name>.lvcontour` | `user:<name>` |
| an freiem Ort | wo sie liegt | beim Pfad |

Eigene sind ein eigener Raum: ein eigenes `heart` verdrängt das mitgelieferte
nicht. Wird eine Datei an freiem Ort gewählt, fragt das Programm wie bei
Samples, ob es sie zu den eigenen kopiert. Eigene Formen bekommen einen
Bereich **Shapes** in Export und Import der eigenen Daten
(`core/Datensicherung`), mitgelieferte einen Eintrag in `asset/Asset.cmake`.
Der Ordner liegt nicht unter `loop`, weil auch die Visuals ihn lesen.
Verworfen (b): ein Namensraum, in dem die eigene Datei die mitgelieferte
verdrängt, wie bei den Kurven-Presets des Composers — eine eigene `heart`
änderte dann jedes mitgelieferte Instrument, das `heart` spielt.

### Das Instrument nennt die Form (D3 a)

Die Schicht nennt die **Datei** und darin die **Kontur über eine feste
Kennung**, die beim Anlegen der Kontur vergeben wird und sich nicht mehr
ändert — wird eine Kontur gelöscht, verrutschen die anderen nicht. Wird die
Form geändert, spielen alle Instrumente die neue Fassung; das Form-Fenster
nennt vor dem Speichern die eigenen Instrumente, die sie spielen (wie bei
Samples, M3 a). Fehlt die Datei oder die Kontur, schweigt die Schicht, und
der Creator sagt es. Verworfen: eine Kopie im Instrument (b), beides mit der
Kopie als Rückfall (c).

**Das Teilen** ist damit nicht gelöst — eine `.lvinst` allein ist nicht
vollständig, wie heute schon bei Samples. Patriks Lösung dafür, für diesen
Fall und andere: **Publish** — ein Paket `<name>.<type>.lvdata` mit allen
zugehörigen Dateien, der Ordnerstruktur des Benutzerordners und einem
Manifest. Eigener Punkt, `Offene_Punkte.md` §7; hier nichts dazu gebaut.

### Der Inhalt (D4 b)

JSON mit einer Fassung.

- Je **Kontur**: die feste Kennung, ein Name, die Farbe, der Haken gewählt,
  und die Kontur als **geschlossener Pfad aus Stücken** — gerade Strecken und
  Kurven (kubische Bézier-Stücke), in den Koordinaten der Zeichnung, damit
  die Konturen zueinander liegen wie im Bild. Eine Kontur aus einem **SVG**
  behält ihre Kurven; eine aus einem **Bild** besteht nur aus Strecken; eine
  **gezeichnete** aus dem, was das Zeichnen ergibt (Skizze, §13 Punkt 6).
  Zu Punkten wird der Pfad erst beim Laden, für die Rechnung (§5).
- Für die **Zeichnung**: die Reihenfolge der Konturen, As layers oder As one
  path, bei As one path die Art der Sprünge und die von Hand gesetzten
  Brücken mit Start- und Endpunkt (B1, §9), und die Herkunft — gezeichnet, SVG oder Bild, der Name der
  Quelldatei und die Einstellungen der Erkennung.
- **Nicht in der Datei:** das Quellbild — liegt es nicht mehr am Ort,
  bleiben die gefundenen Konturen, neu erkennen geht erst wieder, wenn das
  Bild neu gewählt ist. Und die Koeffizienten: sie werden beim Laden
  gerechnet (eine Fourier-Transformation je Kontur; die Dauer ist geschätzt,
  nicht gemessen).

**Zum Entscheid:** Claude hatte a empfohlen (nur Punktfolgen) und b mit „mehr
Code" abgetan, ohne Beleg. Patrik: „vielleicht besser b … damit es angezeigt
werden kann" — die Form soll im Form-Fenster und im Modul der Effect Chain
als Kurve gezeichnet werden, glatt in jeder Größe. Nachgedacht, b ist
die bessere Wahl: ein Pfad aus Strecken und Kurven schließt die Punktfolge
ein (eine Punktfolge ist ein Pfad nur aus Strecken), das SVG bleibt genau und
in jeder Größe glatt, und ein späteres Bearbeiten an Griffen braucht keine
neue Fassung der Datei. Qt trägt die Arbeit: `QPainterPath` kennt genau diese
Stücke und rechnet sie zu Punkten um — **belegt** (`test_ShapeBelege.cpp`,
Fall „Shape Beleg Pfad"): ohne laufende Qt-Anwendung stehen Strecken und
Kurven im Pfad, wie sie hineinkamen, `toSubpathPolygons` gibt einen
geschlossenen Zug, `length` und `pointAtPercent` geben Punkte in gleichem
Abstand entlang des Pfads. Verworfen: das Quellbild in der Datei (c) — aus
einigen Kilobyte würden Megabyte, in jedem Export.

### Wie es gebaut ist (Stufe 19a, S120)

`shape/Kontur.hpp` — nicht unter `loop`, weil auch die Visuals die Form
lesen. Die Namen in der Datei hat Claude gesetzt:

```json
{
    "format": "lumiviz-contour", "version": 1, "title": "Heart and star",
    "useAs": "onePath",        // oder "layers"
    "jumps": "listOrder",      // oder "shortest", "byHand"
    "nextId": 3,
    "contours": [
        { "id": 1, "name": "Heart", "color": "#00e5ff", "selected": true,
          "path": "M 100 76.667 C 100 35.185 42.963 30 42.963 74.074 … Z" }
    ],
    "bridges": [ { "from": 1, "fromPoint": [60, 20], "to": 2, "toPoint": [100, 10] } ],
    "source": { "kind": "drawn", "file": "", "settings": {} }   // oder "svg", "image"
}
```

- **Der Pfad** steht als Text wie die Pfadsprache von SVG, mit vier
  Befehlen: `M x y`, `L x y`, `C x1 y1 x2 y2 x y`, `Z`. Zahlen so kurz, dass
  sie genau zurückkommen. Koordinaten der Zeichnung, y nach unten.
- **`nextId`** ist die Kennung, die die nächste neue Kontur bekommt — größer
  als jede, die es in der Zeichnung je gab; eine Kennung wird nie neu
  vergeben.
- **Beim Lesen** in seine Grenzen: höchstens 64 Konturen, jede Kennung
  einmal, eine Kontur ohne Pfad fällt weg, ein offener Pfad wird geschlossen,
  eine Brücke nur zwischen zwei verschiedenen Konturen, die es gibt. Eine
  Fassung von morgen liest das Programm nicht und sagt es.
- **Die Rechnung:** `abtasten` (4096 Punkte in gleichem Abstand, y
  umgedreht), `normiert` (um den Schwerpunkt, größter Ausschlag 1), `reihe`
  (Koeffizienten bis zur 512. Harmonischen, direkt gerechnet), `wert` und
  `umriss` (ein Punkt, der ganze Zug, mit weichem Gewicht nach Z4 a). Kurven
  werden vor dem Zerlegen groß gerechnet — Qt zerlegt mit einer festen
  Toleranz in den Einheiten des Pfads, eine kleine Zeichnung würde sonst
  eckig.
- **Mitgeliefert** (`asset/contours`, aus `make_contours.py`): Circle,
  Square, Triangle, Star, Heart, und Heart and star mit zwei Konturen als ein
  Pfad.
- **Tests:** `test_ShapeKontur.cpp`, sechs Fälle; `LUMIVIZ_SHAPE_BILD=<datei.png>`
  legt ein Bild ab — je mitgelieferter Kontur die Form mit 1, 4, 16 und 64
  Teiltönen über der Zeichnung. Gemessen am Quadrat: die größte Abweichung
  ist 0,27 mit einem Teilton, 0,071 mit 8, 0,0089 mit 64, 0,0021 mit 256
  (bei einer halben Kantenlänge von 1).

Noch nicht gebaut: eine Datei von irgendwo zu den eigenen kopieren (wie
`nimmZuEigenen` der Samples) — kommt mit dem Fenster.

---

## 8. Die Eingabewege

Alle drei enden im selben Fenster und in derselben Datei: einer Liste
geschlossener Konturen, jede in einer Farbe, einige davon gewählt.

**Zeichnen (Z1 a, Patrik S122).** Ein Werkzeug **Pen** für beides: Ziehen
mit gedrückter Taste zeichnet frei Hand — beim Loslassen wird der Zug
geglättet und in wenige Kurven umgerechnet; einzelne Klicks setzen Eckpunkte,
verbunden durch gerade Strecken; beides lässt sich in einer Kontur mischen.
Geschlossen wird mit einem Klick auf den ersten Punkt, einem Doppelklick oder
Enter (mit einer geraden Strecke zum Anfang); Esc verwirft die angefangene
Kontur. **Move point** zeigt danach die Ankerpunkte; ziehen schiebt einen
Punkt, die Kurven daneben gehen mit. Wie stark geglättet wird, ist eine feste
Zahl (von Claude gesetzt), kein Regler in 20a. Verworfen: nur frei Hand (b) —
klare Ecken gingen nur mit ruhiger Hand; nur Punkt für Punkt mit Griffen wie
in Inkscape (c) — keine freie Hand, Griffe als eigener Bedienschritt.

**SVG (S1 a, Patrik S120 — mit Beleg).** Nachgesehen: die öffentliche Klasse
`QSvgRenderer` kann laden, zeichnen und die Lage eines Elements nennen; die
Pfade gibt sie nicht heraus. Die private Schnittstelle von Qt Svg kennt sie,
ist aber nicht zugesagt — nicht benutzt.

Der Weg: **Qt Svg zeichnet das SVG auf ein eigenes Zeichengerät, das nichts
malt und jeden Aufruf mitschreibt.** So kommt an, was Qt Svg zeichnen kann —
Pfade, Rechtecke, Kreise, Gruppen, Verschiebungen und Drehungen —, als Pfad
mit Kurven, an seiner Stelle, mit seiner Füllfarbe als Farbe der Kontur. Ein
offener Pfad wird gezeigt und beim Wählen mit einer geraden Strecke
geschlossen.

**Der Beleg** (`test_ShapeBelege.cpp`, Fall „Shape Beleg SVG", unter beiden
Compilern grün): ein SVG mit einem Pfad mit Kurve, einem verschobenen
Rechteck, einem Kreis in einer verschobenen und vergrößerten Gruppe und
einer offenen Linie. Angekommen sind **vier Aufrufe** — zweimal `drawPath`,
einmal `drawRects`, einmal `drawEllipse`:

- der Pfad mit **sechs Elementen**, die Kurve ist eine Kurve geblieben (nicht
  in Strecken zerlegt), Anfang und Ende an (10, 10), gefüllt in seinem Rot;
- das Rechteck an seiner verschobenen Stelle (100, 20, 20 × 10);
- der Kreis mit Verschiebung und Vergrößerung der Gruppe (130, 30, 40 × 40);
- die offene Linie ohne Füllung, mit ihrem Strich in Gelb, nicht geschlossen.

Zu beachten beim Bau: Rechteck und Kreis kommen nicht als Pfad an, sondern
über eigene Aufrufe — das Gerät macht daraus Pfade. Die Lage (Verschiebung,
Drehung, Größe) kommt getrennt als Zustand und wird auf den Pfad angewandt.
**Nicht belegt:** Text, Verweise mit `use`, Stile aus CSS, Verläufe als
Füllung, Dateien aus Inkscape oder Illustrator. Verworfen: die Datei selbst
lesen (b) — bleibt der Rückfall, wenn ein Teil davon nicht ankommt; das SVG
als Bild durch die Konturerkennung (c) — verliert die Kurven.

Qt Svg steht seit S120 in der `Solution.json` (`Svg`); die DLL lag schon
neben der Exe.

**Welche Konturen, welche mit Haken (O2 a, Patrik S122):** jeder Teilpfad
wird eine Kontur, ein Loch ebenso. Sind es mehr als 64, bleiben die 64
längsten (nach Länge des Umrisses — offene Linien haben keine Fläche), und die
Liste sagt „n smaller ones left out". Anfangs haben alle den Haken, wenn es
höchstens acht sind (so viele Schichten trägt ein Instrument); sind es mehr,
nur die längste, wie bei Bildern. Belegt an Patriks 93 Proben in
`asset/shapes`: 72 haben höchstens acht Teilpfade, 4 neun bis 64, 17 mehr
als 64 (die Muster aus `2/` und drei Dateien aus potrace, bis 2363).
Verworfen: immer alle angehakt (b) — bei einem Muster bis zu 63 Haken von
Hand weg; immer nur die längste (c) — bei einfachen Formen jedes Mal die
anderen Teile von Hand dazu.

**Wie es gebaut ist (Stufe 20b, S122):** `shape/SvgKonturen` — das
Zeichengerät aus dem Beleg, `QSvgRenderer` zeichnet in die viewBox selbst
(die Konturen behalten die Maße des SVG). Jeder Teilpfad eines Aufrufs wird
eine Kontur; ein offener wird mit einer Strecke geschlossen. Die Farbe ist
die Füllung, wenn sie zu sehen ist, sonst der Strich; was unsichtbar ist,
fällt weg — QPainter reicht einen Pfad ohne Füllung und Strich gar nicht
erst an das Gerät. **Die Farbe (O3 a, Patrik S122):** haben alle Konturen
eines SVG dieselbe Farbe, bekommen sie die Farben der Reihe wie gezeichnete;
haben sie verschiedene, behält jede ihre. Anlass: aus potrace kommt alles
schwarz — auf der Zeichenfläche kaum, im Knoten Shape auf schwarzem Grund gar
nicht zu sehen (Bild aus dem Test, `1299642.svg`), und gleich gefärbte
Konturen sind nicht zu unterscheiden. Verworfen: nur zu dunkle Farben ersetzen
(b) — gleich gefärbte bleiben ununterscheidbar, die Schwelle wäre gesetzt;
immer die Farbe aus dem SVG (c) — S1 a wörtlich, potrace unsichtbar. Im Fenster **Open SVG...** mit Replace, Add und Cancel
(O1 a); Add passt das SVG mit gleichem Maß in x und y in den Ausschnitt ohne
seinen Rand (die Ansicht durch 1,5); Replace lässt die Kennungen
weiterlaufen; die Zeile „n smaller ones left out" unter der Liste zählt bis
zur nächsten neuen oder geöffneten Zeichnung, auch was eine volle Zeichnung
beim Hinzufügen nicht mehr nimmt. Der Ordner der letzten Datei wird gemerkt.
Gemessen an Patriks 93 Proben (Testing-MSVC): jede gelesen, 78 mit höchstens
acht Konturen, 11 mit mehr als 64, 358 offene Konturen geschlossen, die
langsamste in 17 ms. **Sechs Dateien ohne Kontur:** die `-raw`-Fassungen der
Muster in `2/` nennen ihre Farben über CSS-Variablen
(`stroke="var(--stroke-color, currentColor)"`); Qt Svg kennt sie nicht, die
Pfade sind unsichtbar, das Fenster meldet „holds no visible path". Die
aufgelösten Fassungen daneben kommen an. Alles in diesem Absatz ohne Verweis
auf einen Entscheid ist von Claude gesetzt.

**Bild.** Konturen in einem jpg oder png finden, anzeigen, auswählen lassen —
ohne OpenCV, selbst geschrieben. Entschieden in S120 (K1 a, K2 a), nach
einer Probe an drei Bildern aus `asset/img` (in Python, nur zur Anschauung):

| Bild | nach Helligkeit, 3 Stufen | nach Farbe, 6 Farben | nach Kanten |
|---|---|---|---|
| `logo/original/avs.png` | 11 Konturen, 32 zu klein | 22, 429 zu klein | 17 Linienstücke, nicht geschlossen |
| `logo/original/milkdrop.png` | 5, 22 zu klein | 21, 330 zu klein | 33 Stücke |
| `screenshots/avs/flowers.png` | 2, 5 zu klein | 41, 473 zu klein | 7 Stücke |

Helligkeit gibt wenige, saubere Konturen und findet dunkle farbige Motive
nicht; Farbe findet sie, liefert viel mehr und zieht in weichen Verläufen
Ränder, die keiner sieht; Kanten treffen am ehesten den gesehenen Umriss,
sind aber Bänder und offene Stücke. Beide Flächenverfahren ziehen eine
Kontur um das ganze Bild.

**Das Verfahren (K1 a):** das Bild wird in **Flächen** geteilt, der Rand
jeder Fläche ist eine Kontur — er ist von selbst geschlossen. Die Auswahl
**Split by** stellt ein, wonach geteilt wird:

- **Brightness** — nach Helligkeit, in einstellbar vielen Stufen;
- **Colour** — nach Farbe, in einstellbar vielen Farben;
- **Transparency** — nach der Deckkraft, für ein freigestelltes PNG.

Alle drei benutzen dasselbe Ablaufen und Glätten der Ränder. Verworfen: nur
Helligkeit (b) — versagt an `flowers.png`; Kanten (c) — offene Stücke zu
schließen wäre ein eigenes Verfahren; belegt ist nur, dass sie offen sind,
wie gut ein Schließen gelänge, ist nicht versucht. Als Zugabe zu a denkbar.

**Regler (K2 a):** **Split by**, **Levels** (2 bis 8), **Smooth** (das Bild
vor dem Teilen weichzeichnen), **Min size** (Flächen unter diesem Anteil des
Bildes fallen weg), **Detail** (wie genau der Rand nachgefahren wird). Smooth
und Detail ändern den Klang direkt: ein zackiger Rand hat viele hohe
Harmonische, ein glatter wenige.

**Regeln (K2 a):**

- zum Erkennen wird das Bild auf höchstens 1024 Punkte Kante verkleinert;
- ein Loch in einer Fläche ist eine eigene Kontur;
- eine Fläche, die an den Bildrand stößt, wird gezeigt, ist aber anfangs
  nicht gewählt (verworfen c: sie fällt weg — bei einem angeschnittenen Motiv
  fehlte die Hauptform);
- angeboten werden die größten 64, die Liste sagt, wie viele weggefallen
  sind;
- anfangs gewählt ist die größte Kontur, die nicht an den Rand stößt.

Die Zahlen (1024, 64, 2 bis 8) sind von Claude gesetzt und nicht gemessen;
sie gehören in die Skizze und in den Sichttest. Mit D4 b ist eine Kontur aus
einem Bild ein Pfad nur aus Strecken.

**Was die Regler ändern (R1 a, Patrik S122):** das Fenster merkt sich, welche
Konturen aus dem zuletzt geöffneten Bild stammen. Ein Regler erkennt neu und
ersetzt genau diese (neue Kennungen, Haken nach K2 a); gezeichnete und
SVG-Konturen bleiben. Was man an den Konturen aus dem Bild geändert hat, geht
dabei verloren — jedes Neu-Erkennen ist ein Schritt für Undo. Nach dem
Speichern oder Wiederöffnen ist das Bild nicht mehr da; die Regler sind grau,
bis es neu gewählt ist. **Mit b als Möglichkeit (Patrik S122), der Schalter im
Fenster (R2 a):** in der Gruppe From image der Haken **Apply live** — an gilt
R1 a; aus zeigen die Regler die erkannten Konturen nur als Vorschau in Grau auf
der Zeichenfläche, und der Knopf **Take contours** übernimmt sie (mit O1 a:
Replace, Add, Cancel). Das Programm merkt sich die Stellung. Verworfen: der
Schalter in den Settings (R2 b) — aus dem Fenster heraus nicht zu finden; nur
bis zur ersten Änderung live (R1 c).

**Wie es gebaut ist (Stufe 20c, S122):** `shape/BildKonturen` (ohne Widget).
Das Bild wird auf höchstens 1024 Punkte Kante verkleinert und zweimal mit
einem gleitenden Mittel über 2·Smooth+1 Punkte weichgezeichnet. Durchsichtiges
zählt für Brightness und Colour als Weiß. **Brightness** und
**Transparency** teilen in gleich breite Stufen; **Colour** mit k-means auf
einer Stichprobe von höchstens 40 000 Punkten, die Startmitten ohne Zufall:
die dunkelste Farbe, dann jeweils die, die von allen bisherigen am weitesten
weg liegt (nach Quantilen der Helligkeit fielen zwei Mitten in eine große
Fläche, eine kleine helle bekam keine). Flächen sind zusammenhängende Punkte
einer Klasse (je vier Nachbarn); eine unter Min size geht in ihrem größten
Nachbarn auf (bis zu vier Runden). Die Ränder werden auf den Kanten der Punkte
abgelaufen, die Fläche links; wo sich zwei Punkte nur schräg berühren, bleibt
der Rand getrennt. Ein Rand, den zwei Flächen teilen, kommt einmal. Douglas-
Peucker mit 0,75 Punkten (Detail 100 %) bis 5 Punkten (Detail 0 %). Die
Vorgaben: Brightness, Levels 3, Smooth 1, Min size 0,2 %, Detail 50 %.
Im Fenster: **Open image...** (PNG, JPEG, BMP, GIF, WebP; ein Foto steht, wie
es aufgenommen ist), mit Apply live nach O1 a; ohne Apply live liegt das Bild
neben vorhandenen Konturen eingepasst, Take contours fragt. **Show image**:
das Bild mit 35 % Deckung hinter den Konturen. Die Liste zeigt „edge" für
Konturen am Bildrand. Die Regler erkennen 150 ms nach der letzten Bewegung.
Undo merkt sich mit der Zeichnung, welche Konturen aus dem Bild stammen. Nach
dem Speichern, Wiederöffnen oder New shape ist das Bild weg. An
`asset/img/logo/original/milkdrop.png` mit den Vorgaben: 6 Konturen, das
abgerundete Rechteck mit Haken, der Bildrand „edge" (die Probe in S120: 5).
Die Farben der Konturen sind die der Reihe (wie O3 a). Alles in diesem Absatz
ohne Verweis auf einen Entscheid ist von Claude gesetzt.

---

## 8a. Das Fenster Edit shape (Skizze gebilligt, Patrik S120)

Mit dem Zeichenwerkzeug gezeigt; hier als Beschreibung.

- **Kopf:** „Edit shape – <Name>", daneben der Ort („My shapes"); rechts
  **Open SVG...** und **Open image...** — die Wege, die Konturen von außen
  holen. **Draw entfällt (Z2 a, Patrik S122):** gezeichnet wird jederzeit mit
  **Pen**, auch über geholte Konturen; ein leeres Fenster beginnt mit Pen
  gewählt. Verworfen: Draw als Kürzel für Pen (b); Draw beginnt eine neue
  Zeichnung (c).
  **Ersetzen oder hinzufügen (O1 a, Patrik S122):** ist die Zeichnung leer,
  kommen die Konturen des SVG (in 20c: des Bilds) in ihren eigenen Maßen
  hinein, die Herkunft wird „SVG" mit dem Dateinamen. Stehen schon Konturen
  da, fragt das Fenster: **Replace** (die bisherigen fallen weg, wie bei einer
  leeren Zeichnung), **Add** (die neuen kommen ans Ende der Liste, so
  eingepasst, dass sie den Ausschnitt der Zeichnung füllen; die Herkunft
  bleibt) oder **Cancel**. Beides ist ein Schritt für Undo. Verworfen: immer
  ersetzen (b) — kein Mischen; immer hinzufügen (c) — Ausprobieren mühsam.
- **Zeichenfläche** (links, der größere Teil): das Bild blass im
  Hintergrund, die Konturen in ihren Farben darüber — gewählte kräftig,
  nicht gewählte dünn und gestrichelt; ein Sprung als grauer gestrichelter
  Strich mit zwei Punkten; der Startpunkt einer Kontur als gefüllter Punkt.
  Darunter die Werkzeuge **Pen** (zeichnen), **Move point** (Punkte
  schieben), **Bridge** (einen Sprung von Hand setzen) und rechts der Haken
  **Show image**.
- **Liste Contours** (rechts): Kopf „Contours — 3 of 41 shown"; je Zeile
  Haken (gewählt), Farbpunkt, Nummer und Name, rechts der Anteil am Umlauf
  in Prozent — bei einer Kontur am Bildrand statt dessen „edge". Darunter
  **Rename**, **Remove** und zwei Pfeile für die Reihenfolge; darunter der
  Satz, wie viele kleinere weggefallen sind („38 smaller ones left out").
- **Drei Gruppen nebeneinander, unter beidem:**
  - **From image** — **Split by** (Colour, Brightness, Transparency),
    **Levels**, **Smooth**, **Min size**, **Detail** (K2 a);
  - **Use as** — die Wahl **Layers, one per contour** oder **One path**;
    bei One path die Auswahl **Jumps** mit „In list order", „Shortest",
    „Drawn by hand" (B1); darunter eine Zeile mit den Zahlen („2 contours ·
    2 jumps");
  - **Preview** — der Wellenzug eines Umlaufs **über der Zeit**, links und
    rechts je eine Kurve, mit einem Strich, wo die nächste Kontur beginnt;
    darunter ein Regler **Harmonics** zum Ausprobieren und **Play**.
- **Fuß:** links „Used by: <eigene Instrumente, die die Form spielen>",
  rechts **Cancel**, **Save as...**, **Save** — wie im Fenster Edit sample.

**Berichtigt nach Patriks Einwand:** in der ersten Skizze war der Wellenzug
der Preview von Hand gezeichnet und lief stellenweise zurück — für einen
Zeitpunkt standen bis zu drei Werte da. Das kann kein Wellenzug: er ist eine
Funktion der Zeit und hat je Zeitpunkt genau einen Wert. Die Preview zeigt
deshalb die x-Bewegung des Stifts und die y-Bewegung **je für sich über der
Zeit** (in der berichtigten Skizze aus der Probe gerechnet: Herz und Stern,
je 48 Harmonische). Die Form selbst — in der ein x mehrere y haben darf —
steht auf der Zeichenfläche, nicht in der Preview.

**Sprünge von Hand (G2 a, Patrik):** mit Bridge klickt man einen Punkt auf
einer Kontur und einen auf einer anderen. Zwei Sprünge an einer Kontur sind
erlaubt — der Stift verlässt sie dann unterwegs und kommt zurück. Ein
Sprung, der einen Ring schließen würde, wird nicht angenommen, und das
Fenster sagt warum. Erreichen die Sprünge nicht alle gewählten Konturen,
hängt das Programm die übrigen in der Reihenfolge der Liste an und sagt, wie
viele. Verworfen: unerreichte Konturen spielen nicht mit (b); Speichern erst,
wenn alle erreicht sind (c).

Von Claude in der Skizze gesetzt, mit ihr gebilligt: die Namen (Edit shape,
Pen, Move point, Bridge, Contours, From image, Use as, Jumps und seine drei
Einträge), der Anteil am Umlauf und das Wort „edge" in der Liste, die Gruppe
Preview. Was Pen tut, steht seit S122 in §8 (Z1 a); Draw ist entfallen
(Z2 a). Noch offen, klein: die Sätze der Meldungen.

### Wie das Fenster gebaut ist (Stufe 20a, S122)

Nach der Skizze oben, mit Z1 a und Z2 a. `loop/FormFenster` (das Fenster,
`FormFlaeche`, `FormVerlauf`), die Rechnung am Pfad in `shape/Zeichnen`
(ohne Widget, ohne Qt-Anwendung).

- **Pen:** ein Zug frei Hand wird mit einem gleitenden Mittel über fünf
  Punkte geglättet, mit Douglas-Peucker auf die Punkte vereinfacht, ohne die
  er weiter als **2 Bildpunkte** abwiche, und durch diese Punkte mit Kurven
  (Catmull-Rom als kubische Bezier-Stücke) gelegt. Ein Klick hängt eine
  gerade Strecke an; nach einem Eckpunkt führt eine Strecke zum Anfang des
  nächsten Zugs. Mehr als **4 Bildpunkte** zwischen Drücken und Loslassen
  ist ein Zug, sonst ein Klick. Der Fangkreis um den ersten Punkt hat **8
  Bildpunkte**; ein Zug frei Hand, der darin endet, schließt die Kontur von
  selbst. Schließen geht erst ab drei Punkten (oder zwei mit einer Kurve
  dazwischen). Wer Pen mit etwas Angefangenem verlässt, behält es als
  Kontur, wenn es sich schließen lässt, sonst ist es weg.
- **Move point:** die Ankerpunkte als kleine Quadrate; geschoben gehen die
  beiden Griffe der Kurven daneben um dasselbe Stück mit, bei einem
  geschlossenen Zug der Anfang mit dem Ende. Während des Ziehens wird nur
  gezeichnet, gerechnet (Preview, Anteile) beim Loslassen; ein Zug ist ein
  Schritt für Undo.
- **Die Ansicht** wird beim Öffnen gesetzt: alle Konturen mit einem Rand von
  einem Viertel der längeren Seite; eine leere Zeichnung zeigt 0 bis 200 in
  beiden Richtungen, wie die mitgelieferten Formen. Sie wandert beim
  Zeichnen nicht mit.
- **Contours:** „Contours — 2 of 3 ticked" statt „3 of 41 shown" (das „shown"
  gehört zu Konturen aus einem Bild, 20c); der Anteil am Umlauf ist der der
  Rechnung (§9), eine Kontur ohne Haken zeigt „–". Rename auch mit
  Doppelklick.
- **Preview:** L und R in zwei Farben in einem Kasten; die Grenzen der
  Konturen gestrichelt. Harmonics wie im Creator (40 Schritte je
  Verdopplung), Vorgabe 24. **Play** spielt eine Sekunde aus 220 ganzen
  Umläufen (A3) in Dauerschleife, links x, rechts y, mit dem Pegel 0,4 —
  über `AusschnittQuelle` des Fensters Record sample, ohne Blenden. Eine
  Änderung spielt sofort mit.
- **Undo und Redo** (Ctrl+Z, Ctrl+Y), hundert Schritte — nicht in der
  Skizze.
- **Save** und **Save as...** wie im Fenster Edit sample (Rückfrage mit den
  eigenen Instrumenten, die die Form spielen — neu
  `eigeneInstrumenteMitForm`; ein vergebener Name geht bei Save as... nicht).
  Eine mitgelieferte Form oder eine Datei von irgendwo wird mit Save as... zu
  einer eigenen — das ist das Kopieren, das S121 hierher verschoben hatte.
  Nach dem Schreiben zählt das Fenster `zaehleInstrumentFassung` weiter
  (gerechnete Töne entstehen neu) und `Kontur.cpp` die neue
  `zeichnungFassung` — der Knoten Shape (`rt.shapeFassung`) und der Creator
  lesen danach neu, auch bei gleicher Nennung.
- **Im Creator:** **Edit...** neben Browse... (grau, wenn die Form nicht
  gefunden wird) und **New shape...** am Ende des Menüs Browse... (Antwort auf
  Patriks Frage S122, ob man eine neue beginnen kann). Nach Save oder Save
  as... spielt die gewählte Schicht das Ergebnis; ist es dieselbe Form, bleibt
  die gewählte Kontur, solange es sie gibt, sonst steht Contour auf „All, one
  path".
- **Grau bis zu ihrer Stufe:** Open SVG..., Open image..., Bridge, Show image,
  From image, Use as — Use as zeigt dabei, was in der Datei steht.
- **Preview:** R in der Textfarbe, damit die Kurve keiner Kontur zu gehören
  scheint (im ersten Bild war sie pink wie die zweite Kontur).

Alle Zahlen und die Punkte ohne Verweis auf einen Entscheid sind von Claude
gesetzt. Nicht gebaut: ein Ausschnitt, der sich verschieben oder vergrößern
lässt; Punkte einfügen oder löschen; Griffe einzeln ziehen.

---

## 9. Mehrere Konturen

**As layers** (Z5 a): je gewählter Kontur eine Schicht in ihrer Farbe, bis
das Instrument voll ist (vier). Alle Schichten nennen dieselbe Datei, jede
ihre Kontur über deren feste Kennung (D1 a, D3 a).

**As one path** (Z5 a): die gewählten Konturen nacheinander in einem Umlauf.
Nur so erscheint im Scope eine Zeichnung aus mehreren Teilen. Entschieden in
S120 (B2 d, B1), nach einer Probe.

### Jede Kontur bleibt für sich (B2 d)

Ein Ton ist die Bewegung eines einzigen Stifts; von einer Kontur zur
nächsten muss er einen Weg nehmen. Der Weg ist aber **nicht Teil der Form**:

- jede Kontur hat ihre **eigene Reihe** und bleibt ein eigener geschlossener
  Zug;
- eine Schicht spielt sie **nacheinander in einem Umlauf**; jede bekommt so
  viel vom Umlauf, wie sie lang ist;
- zwischen zwei Konturen **springt** der Stift in einem kurzen geraden Zug;
- **Harmonics gilt je Kontur**: mit wenigen wird jede für sich rund.

Die Probe (Herz und Stern, in Python, nur zur Anschauung; im Scope-Bild mit
16, 48 und 160 Harmonischen):

| Weg | Anteil des Wegs am Umlauf | Ergebnis |
|---|---|---|
| eine Reihe samt Brücke, Stift gleich schnell | 7,1 % | sauber, die Brücke so hell wie die Konturen; bei 16 verschmelzen die Formen; auf 1 % genau ab 36 Harmonischen |
| eine Reihe samt Brücke, Stift springt | 0,6 % | bei 160 ein blasser Strich, bei 48 zittern beide Konturen, bei 16 kreuzt der Strich die Formen; auf 1 % genau erst ab 192 |
| eine Reihe, Brücke zwischen den Startpunkten | 27,6 % | ein langer Strich quer durch das Herz |
| **jede Kontur ihre eigene Reihe, kurzer Sprung** | 0,6 % | bei 16, 48 und 160 getrennt und geschlossen, der Sprung ein blasser Strich, kein Zittern |

Der Anstoß kam von Patrik: „das sind doch eigentlich zwei separate Konturen,
und beide sind geschlossen" — Claude hatte in den ersten drei Zeilen aus
beiden eine gemacht. Verworfen: eine Reihe über den ganzen Umlauf mit Regler
Bridge speed (a), immer gleich schnell (b), immer im Sprung (c).

**Der Preis liegt im Ton:** der Sprung ist eine Kante im Wellenzug, die
Harmonics nicht im Griff hat. In der Probe liegt über der eingestellten Zahl
noch Energie bei −31, −38 und −51 dB (bei 16, 48, 160) — grob gemessen,
**nicht gehört**. In Stufe 19b zu hören und zu messen, auch an hohen Noten;
stört die Kante, ist ein Regler für die Dauer des Sprungs der erste Ausweg,
der Weg a der zweite.

**Gebaut und gemessen in Stufe 19c (S121), nicht gehört:** ein Sprung nimmt
`kSprungAnteil` = 0,3 % des Umlaufs (von Claude gesetzt; die Probe hatte
0,6 % für beide zusammen). Bei A2 sind das gut ein Abtastwert, bei A4 ein
Drittel — der Sprung ist im Ton also eine **harte Stufe**, und eine Stufe
hat Teiltöne über der halben Abtastrate, die als falsche Töne zurückfallen.
Gemessen an Herz und Stern (`test_ShapeTon` „Messung"): bei A2 liegen −33 dB
(16 Harmonics) und −43 dB (48) des Tons über dem, was Harmonics zulässt; bei
**A4 sind es −33 bis −34 dB, gleich bei 16, 48 und 160** — dort ist es fast
ganz der zurückgefallene Anteil, und mehr Harmonics hilft nicht. Hörproben:
`onepath_*` unter `.claude/handover/hoerproben_19c` (lokal). Ob das stört,
entscheidet das Ohr; die Auswege stehen oben.

**Drei und mehr Konturen** (von Claude gesetzt, in 19c gebaut): von der
ersten zur zweiten wird dort gesprungen, wo sie sich am nächsten kommen;
jede weitere wird an dem Punkt betreten, der dem letzten am nächsten liegt,
und an demselben Punkt wieder verlassen (ein geschlossener Zug endet, wo er
begann); von der letzten geht es zurück zum Tor der ersten. Die Arten
„kürzeste Wege" und „von Hand" spielen bis Stufe 20d wie die Reihenfolge der
Liste.

### Wo der Stift springt (B1, Patrik: wählbar)

Im Form-Fenster wählbar, drei Arten:

- **In Reihenfolge der Liste** (b, die Vorgabe): die Konturen kommen, wie sie
  in der Liste stehen; der Stift springt von jeder zur nächsten an der
  Stelle, an der sie der nächsten am nächsten kommt, und von der letzten
  zurück zur ersten. Die Reihenfolge bestimmt, was zuerst im Umlauf klingt.
- **Auf den kürzesten Wegen** (a): das Programm verbindet selbst, so dass
  alle Sprünge zusammen am kürzesten sind.
- **Von Hand** (d, Patrik): die Brücke wird im Form-Fenster eingezeichnet,
  mit Start- und Endpunkt.

Verworfen (c): der Sprung an den gespeicherten Startpunkten. Die Art und die
von Hand gesetzten Brücken stehen in der Datei.

Wie die drei Arten im Fenster heißen und die Regeln für Sprünge von Hand
stehen seit der Skizze fest (§8a, G2 a). Drei und mehr Konturen sind nicht
geprobt.

---

## 10. Die Visuals

Zwei neue Knoten für die Effect Chain, entschieden in S120 (M1 a, M2 b,
M3 a).

1. **XY Scope (M1 a)** — zeichnet die 512 Werte je Seite als Linie, x links
   und y rechts, in voller Genauigkeit (nicht über die auf 8 Bit gerundeten
   Werte der Skripte). Braucht die Form nicht, zeigt jeden Stereo-Ton.
   Regler: **Gain**, **Line width**, Farbe aus der Farbtafel oder einem
   Verlauf, **Rotate 45°** (Mono steht dann senkrecht, wie bei einem
   Goniometer). Nachleuchten kommt aus einem Knoten Fadeout dahinter. Die
   Zufuhr bleibt, wie sie ist. Verworfen: kein Knoten, nur ein Preset mit
   einem Superscope-Skript (b) — 256 Stufen; eine längere, lückenlose
   Zufuhr für alle Visuals (c) — eigener Schritt, gegen die Kalibrierung der
   AVS-Presets zu prüfen.
2. **Shape (M2 b)** — nennt eine `.lvcontour` wie eine Schicht (beim Namen,
   `user:<name>` oder Pfad, D2 a) und zeichnet die gewählten Konturen in
   ihren Farben, unabhängig vom Ton, also auch zu einem Song. Regler:
   **Harmonics**, **Line width**, Lage, Größe, Drehung; dazu der Haken
   **Show circles** — die Kreise und Arme, die die Form zeichnen, laufen
   mit, mit einem Regler **Speed** (die Epizykel-Zeichnung, die den Anstoß
   gab). Fehlt die Datei, zeichnet er nichts, und das Panel sagt es.
   Verworfen: ohne die Kreise (a); die Kontur ins Preset eingebettet wie ein
   Bild bei Picture (c) — eine zweite Fassung neben der Datei, das
   Weitergeben löst Publish.

**Dazu (Patrik, mit dem Entscheid):** die Knoten bekommen **die üblichen
Skriptfelder** — Init, Per beat, Per frame … — und es kommen **einige
Presets** mit. Was das heißt, ist in S121 einzeln entschieden (V1 bis V4).

**V1 a (Patrik S121) — die Skriptfelder von Shape:** Init, Per frame, Per
beat, Song und **Per point**. Die ersten vier steuern alle Regler. Per point
läuft je Punkt der gezeichneten Linie: es liest `i` (Stelle im Umlauf, 0 bis
1), `x` und `y` (der Punkt der Form nach Harmonics, vor Lage, Größe und
Drehung), `contour` (Nummer der Kontur) und `v` (der Wert der Wellenform an
dieser Stelle) und setzt `x`, `y`, `red`, `green`, `blue`, `skip`. Die Kreise
von Show circles zeichnen weiter die gerechnete Form. Die Namen sind die von
SuperScope, `contour` von Claude gesetzt. Verworfen: nur die vier Felder ohne
Per point (b). Die Rechenzeit von Per point ist vor dem Bau des Felds
gemessen (unten, „Gemessen").

**V2 a (Patrik S121) — die Regler von Shape:**

| Im Panel | Skriptname | Bereich, Vorgabe | Anmerkung |
|---|---|---|---|
| Shape, mit Browse... | – | Nennung der `.lvcontour` | nicht aus dem Skript |
| Contour | `contour` | 0 = alle mit Haken, sonst die Kennung; Vorgabe 0 | wie die Zeile an der Schicht |
| Harmonics | `harmonics` | 1 bis 512, Kommazahl; Vorgabe 64 | läuft weich (Z4 a) |
| Points | `points` | 16 bis 4096; Vorgabe 512 | je Kontur; so oft läuft Per point |
| Line width | `linewidth` | 1 bis 20 px; Vorgabe 2 | wie SuperScope |
| X shift, Y shift | `xshift`, `yshift` | −100 bis 100 % der Breite, Höhe; Vorgabe 0 | wie Text und Lyrics |
| Size | `size` | 1 bis 400 %; Vorgabe 80 | 100 = die Form füllt die kürzere Seite |
| Rotation | `rotation` | 0 bis 360°; Vorgabe 0 | aus dem Skript auch darüber hinaus, läuft um |
| Use own color, Color | `useowncolor`, `color_r/_g/_b` | Vorgabe aus | sonst die Farben der Konturen |
| Line blend | `lineblend` | 0 bis 9; Vorgabe 1 (additiv) | wie SuperScope |
| Show circles | `showcircles` | Haken; Vorgabe aus | |
| Speed | `speed` | 0 bis 4 Umläufe je Sekunde; Vorgabe 0,25 | nur mit Show circles |

Contour, Points, die eigene Farbe und Line blend standen nicht in M2 b; sie
kamen mit V2 a dazu. Von Claude gesetzt, mit a gebilligt: alle Vorgaben, die
Bereiche von Size und Speed, die Einheit von Speed, der Name Points.
Verworfen: nur die Liste aus M2 b mit fest 512 Punkten (b).

**V3 a (Patrik S121) — neun Presets** unter `asset/nodepresets/shape/`, je
eines für jede Fähigkeit des Knotens:

| Name | Form | Was es zeigt |
|---|---|---|
| Heart | heart | die Form ohne Skript, 64 Harmonics |
| Epicycles | star | Show circles an, 16 Harmonics, Speed 0,25 |
| Harmonics Sweep | square | Per frame: Harmonics läuft von 1 bis 64 und zurück |
| Beat Harmonics | heart | Per beat: Harmonics fällt auf 3, Per frame: wächst zurück |
| Beat Pulse | circle | Per beat: Size springt, Per frame: klingt ab |
| Bass Spin | triangle | Per frame: Rotation folgt dem Bass |
| Wave Outline | circle | Per point: die Wellenform schiebt jeden Punkt nach außen |
| Rainbow Outline | star | Per point: die Farbe läuft mit `i` um die Linie |
| Contour Switch | heart_and_star | Per beat: die nächste Kontur |

Namen, Formen und Zahlen von Claude gesetzt. Verworfen: drei Presets (b).

**V4 a (Patrik S121) — XY Scope bekommt dieselben fünf Felder** (Stufe 19d):
Init, Per frame, Per beat, Song und Per point, dazu einige Presets. Per point
liest dort `i`, `x` (links) und `y` (rechts) in voller Genauigkeit und setzt
`x`, `y`, `red`, `green`, `blue`, `skip`. Welche Presets, wird bei 19d
vorgelegt. Verworfen: ohne Per point (b), ohne Skriptfelder (c). Fällt die
Messung zu Per point bei Shape schlecht aus, wird V4 neu vorgelegt.

**X1 a (Patrik S121) — die Regler von XY Scope:**

| Im Panel | Skriptname | Bereich, Vorgabe | Anmerkung |
|---|---|---|---|
| Gain | `gain` | 0,1 bis 16; Vorgabe 1 | Vollausschlag füllt bei 1 die Größe |
| Draw | `drawmode` | Lines, Dots; Vorgabe Lines | |
| Line width | `linewidth` | 1 bis 20 px; Vorgabe 2 | bei Dots der Durchmesser |
| Rotate 45° | `rotate45` | Haken; Vorgabe aus | Mono steht senkrecht |
| X shift, Y shift | `xshift`, `yshift` | −100 bis 100 %; Vorgabe 0 | wie Shape |
| Size | `size` | 1 bis 400 %; Vorgabe 80 | 100 = Vollausschlag füllt die kürzere Seite |
| Color from | `colorsource` | Table, Gradient; Vorgabe Table | |
| Colors | – | Farbtafel, Vorgabe Weiß | wandert von Bild zu Bild durch die Tafel |
| Gradient | – | ein mitgelieferter Verlauf; Vorgabe Neon | entlang der Linie |
| Line blend | `lineblend` | 0 bis 9; Vorgabe 1 | wie Shape |

Lage, Größe, Line blend und Dots standen nicht in M1 a. Verworfen: nur die
Liste aus M1 a (b).

**X2 a (Patrik S121) — sechs Presets** unter `asset/nodepresets/xyScope/`:
Goniometer (Rotate 45°, Dots), Lissajous (Lines mit Verlauf), Beat Zoom,
Slow Spin (Per point dreht das Bild), Rainbow Trace (Per point färbt), Corner
Meter (klein in der Ecke). Verworfen: zwei (b).

### Wie der Knoten XY Scope gebaut ist (Stufe 19d, S121)

- **`XyScopeParams`** am Ende der Variante, Typschlüssel `xyScope`;
  **`runXyScope`** gebaut wie `runShape` (eigener Skript-Host nur mit Code,
  die Regler über die Feldtabelle, der Linienzeichner von SuperScope).
- Die Werte kommen aus `getWaveformChannel(0)` und `(1)` als Kommazahlen —
  so viele, wie die Zufuhr liefert (512 je Seite). **Ohne Ton** rechnet der
  Knoten mit Stille: ein Punkt in der Mitte, das Skript läuft.
- **Im Feld Point zeigt `y` nach oben** — es ist der rechte Wert, wie V4 a
  ihn nennt; bei SuperScope und am Knoten Shape zeigt `y` nach unten. Von
  Claude gesetzt, steht im Handbuch.
- Der Verlauf wird einmal je Wahl in eine Tafel mit 256 Farben gerechnet.
- In der Palette steht der Knoten unter „Scopes & Sources".
- **Nicht gesehen:** das Scope zu Musik und zu einer Shape-Schicht mit
  Stereo XY. Im Test zeichnet es gerechnete Blöcke (Kreis, Mono, eine Figur
  aus zwei Frequenzen); das Blatt mit sechs Stellungen und den sechs Presets
  ist angesehen.

### Wie der Knoten Shape gebaut ist (Stufe 19b, S121)

- **`shape/Zeichenform`** (`lumi::shape`, ohne OpenGL und ohne Qt-Anwendung):
  bereitet eine Zeichnung zum Zeichnen auf. Die Reihe für den Klang liegt um
  den Nullpunkt und ist auf vollen Pegel gebracht; ein Bild braucht jede
  Kontur **an ihrer Stelle**, alle im selben Maßstab. `zeichenform` legt die
  ganze Zeichnung — auch die Konturen ohne Haken — in das Quadrat −1 … 1;
  welche Kontur gezeichnet wird, ändert deshalb nicht, wo die anderen stehen.
  `gemeinteKonturen` (0 = jede mit Haken, sonst die eine Kennung, auch ohne
  Haken), `umrissVon` (gemerkt, bis sich Harmonics oder die Punktzahl ändern),
  `zeigerbild` (die Kreise in der Folge +1, −1, +2, −2, …; die Spitze des
  letzten Arms ist der Punkt des Umrisses).
- **`ShapeParams`** in `EffectChain.hpp`, am Ende der Variante; Typschlüssel
  `shape`. Lage, Größe, Drehung und Harmonics sind Kommazahlen, damit ein
  Skript sie stufenlos bewegt.
- **`runShape`** im `MultiEffectVisualizer`: eigener Skript-Host (wie
  SuperScope 3D) nur, wenn der Knoten Code trägt; die Regler kommen über die
  Feldtabelle hinein und zurück. Die Zeichnung wird je Nennung einmal gelesen
  — **eine geänderte Datei sieht der Knoten erst, wenn die Nennung wechselt**
  (für das Fenster Edit shape, Stufe 20a, nachzuziehen). Gezeichnet wird mit
  dem Linienzeichner von SuperScope; ein Set Render Mode davor gewinnt bei
  Mischart und Breite.
- **Im Feld Point** heißt `contour` die Kennung der Kontur, die gerade
  gezeichnet wird; nach den Punkten bekommt die Variable wieder, was Frame und
  Beat hinterlassen haben. So trägt ein Name beide Bedeutungen, ohne dass das
  Feld Point die Auswahl verstellt. `y` zeigt im Skript nach unten wie bei
  SuperScope. Der Zug wird mit dem ersten Punkt geschlossen, das Skript läuft
  dafür nicht ein zweites Mal.
- **Show circles:** die Kreise in 35 %, die Arme in 70 % der Linienfarbe, ein
  Pixel breit; Kreise unter einem Pixel Radius entfallen. Die Linie wird ganz
  gezeichnet, nicht nur bis zur Spitze. Alles von Claude gesetzt.
- **Panel:** die Zeile Shape ist eine Liste (mitgelieferte, eigene mit
  „(own)", eine Datei an freiem Ort) mit **Browse...**; eine andere Form
  setzt Contour auf „All ticked". Die Zeile Contour nennt die Konturen der
  Datei. Der Ernter (`harvest_field_docs.py`) führt `contour` als
  Zusatzfeld, weil die Liste kein `addEnum` ist.
- **Von Claude gesetzt, nicht vorgelegt:** die Drehrichtung (gegen den
  Uhrzeigersinn); `xShift`/`yShift` als Kommazahl statt Ganzzahl wie bei Text;
  die Vorgabe `heart` für einen neuen Knoten; die Texte im Panel; in den
  Presets Size 100 bei Contour Switch (eine von zwei Konturen ist sonst
  klein) und 70 bei den Kreisen.

**Gemessen (S121, `test_ShapeKnoten` „Messung", je Bild und Kontur, die
kürzeste von drei Messungen):**

| | Release-clang | Testing-MSVC |
|---|---|---|
| Per point, 512 Punkte | 0,19 ms | 0,21 ms |
| Per point, 4096 Punkte | 1,5 ms | 1,6 ms |
| Per point, 16384 Punkte (vier Konturen mit 4096) | 6,0 ms | 6,7 ms |
| Umriss, 512 Punkte, 64 Harmonics (die Vorgabe) | 0,09 ms | 2,8 ms |
| Umriss, 512 Punkte, 512 Harmonics | 0,63 ms | 22 ms |
| Umriss, 4096 Punkte, 512 Harmonics | 5,3 ms | 180 ms |

Das Skript war das von Wave Outline und Rainbow Outline zusammen. Ein Bild hat
bei 60 Bildern je Sekunde 16,7 ms. Per point trägt also (V1 a, V4 a bleiben);
teurer als das Skript ist im äußersten Fall der Umriss — und der wird nur neu
gerechnet, wenn sich Harmonics oder Points ändern. Die Testing-Fassung
rechnet den Umriss rund dreißigmal langsamer als die Fassung, die im Programm
läuft; wer dort misst, misst nicht das Programm. Nicht gemessen: im laufenden
Programm, mit anderen Knoten daneben; die Reihe einer Kontur beim ersten
Zeichnen (einmal je Kontur).

**Reihenfolge (M3 a):** jedes Modul kommt dort, wo es als Prüfwerkzeug
gebraucht wird — der Knoten Shape gleich nach Datei und Koeffizienten, vor
dem Klang; das XY Scope, wenn der erste Ton zu hören ist (§14).

**Aufwand, nachgesehen:** ein neuer Knotentyp kostete am jüngsten Beispiel
(Lyrics Trail, S92) rund 630 Zeilen in 15 Dateien — Parameter, Datei,
Rendern, Panel, Palette, Feldtabellen, Tooltips, Tests, Handbuch.

**Grenzen des XY-Scope, vor dem Bau zu wissen:**

- Ein Bild bekommt **512 Werte je Seite**. Die Engine läuft mit 44,1 kHz
  (`BassEngine.hpp`, nachgesehen in S120 — vorher stand hier 48 kHz, ohne
  nachzusehen): das sind 11,6 ms, ein Ton unter rund 86 Hz zeigt in einem
  Bild keinen ganzen Umlauf, die Form bleibt offen. Ein Scope mit
  Nachleuchten schließt die Lücke, oder die Zufuhr wird länger.
- Die Zufuhr läuft mit einem Takt von 33 ms (`MainWindow.cpp`), gezeichnet
  wird mit 60 Bildern je Sekunde: von je 33 ms Ton kommen 11,6 ms bei den
  Visuals an, und zwei Bilder können denselben Block zeigen.
- **Skripte bekommen weniger:** für `getosc` werden die 512 Werte auf 576
  gestreckt und auf 8 Bit gerundet (`buildVisData`). Ein Superscope-Skript
  kann nach dem Code heute schon links gegen rechts zeichnen
  (`x=getosc(i,0,1); y=getosc(i,0,2)` — 1 ist links, 2 rechts), aber in 256
  Stufen; ausgeführt ist das nicht. Die Fließkomma-Werte je Seite bekommt
  nur ein Knoten in C++ (`waveOfChannel`).
- **Nachleuchten** hat kein Scope-Knoten der Chain von sich aus; es entsteht
  aus der Liste ohne „clear every frame" und einem Knoten Fadeout.
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

1. ~~Die Datei~~ — entschieden in S120: F1 a, D1 a, D2 a, D3 a, D4 b (§7).
2. ~~Konturerkennung~~ — entschieden in S120: K1 a, K2 a (§8).
3. ~~SVG~~ — entschieden in S120: S1 a, mit Beleg (§8).
4. ~~Die Brücke~~ — entschieden in S120: B2 d, B1 wählbar (§9).
5. ~~Die zwei Module~~ — entschieden in S120: M1 a, M2 b, M3 a (§10).
6. ~~Die Skizze~~ — gebilligt in S120, mit G1 a und G2 a; als Beschreibung
   in §6 (die Shape-Schicht) und §8a (das Fenster Edit shape).
7. ~~Baustufen~~ — freigegeben in S120 (§14).

Damit sind alle sieben Vorlagen geschlossen.

Daneben, klein, wo sie hingehören: Ecken dämpfen, Pegel, Angle bei gesetztem
Stereo XY, Pan bei gesetztem Stereo XY, ob Stimmen und Pitch sweep für Shape
gelten (§5, §6).

---

## 14. Stufen (freigegeben, Patrik S120)

Die Nummern schließen an `Loop_Konzept.md` §5 an (zuletzt Stufe 17).

| Stufe | Inhalt | hängt an |
|---|---|---|
| 18 | **Pan je Schicht** und **acht Schichten je Instrument** (Patrik S120: „Grenze auf acht"), beides in `.lvinst` Fassung 8 — **gebaut in S120**, im Programm ungesehen und ungehört (`Loop_Konzept.md` §5) | — |
| 19a | Form als Daten: Kontur als Pfad, Koeffizienten, die Datei `.lvcontour`, ein paar mitgelieferte Formen — **gebaut in S120** (§7 „Wie es gebaut ist"); im Programm ist davon nur der Bereich Shapes im Export zu sehen | — |
| 19b | der Knoten **Shape** für die Effect Chain, mit Show circles, Skriptfeldern und Presets (M2 b, M3 a; V1 a bis V3 a) — **gebaut in S121** (§10 „Wie der Knoten Shape gebaut ist"), im Programm ungesehen | 19a |
| 19c | die Schwingung Shape im Klangerzeuger: Harmonics, Angle, Stereo XY, Start, Reverse, Harmonics envelope, mehrere Konturen mit Sprung; mit einer Form aus einer Datei, ohne Fenster — **gebaut in S121** (§5 „Wie die Schwingung gebaut ist", `.lvinst` Fassung 9; T1 a, T2 a). Die Kante des Sprungs ist **gemessen** (§9), **nicht gehört**; offen dazu und zu den Ecken: Patriks Ohr | 19a |
| 19d | der Knoten **XY Scope**, mit Skriptfeldern und Presets (M1 a, M3 a; V4 a, X1 a, X2 a) — **gebaut in S121** (§10 „Wie der Knoten XY Scope gebaut ist"), im Programm ungesehen | — (gebraucht ab 19c) |
| 19e | die Shape-Schicht im Creator — **gebaut in S121** (§6 „Wie die Shape-Schicht im Creator gebaut ist"), ohne den Knopf Edit... (kommt mit 20a); im Programm ungesehen und ungehört | Skizze |
| 20a | das Form-Fenster: zeichnen (Z1 a, Z2 a) — **gebaut in S122** (§8a „Wie das Fenster gebaut ist"), im Programm ungesehen | Skizze |
| 20b | SVG über das mitschreibende Zeichengerät (S1 a, O1 a, O2 a) — **gebaut in S122** (§8 „Wie es gebaut ist"), im Programm ungesehen | 20a |
| 20c | Konturen aus einem Bild (K1 a, K2 a, R1 a, R2 a) — **gebaut in S122** (§8 „Wie es gebaut ist (Stufe 20c)"), im Programm ungesehen | 20a |
| 20d | mehrere Konturen: As layers, As one path, die drei Arten der Sprünge (B1) | 20a |

Nach 19b ist die Form zu sehen, nach 19c zu hören und nach 19d im Scope zu
prüfen — alles, bevor ein Fenster steht.

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
| 0.19.0 | 2026-10-14 (S122) | **Konturen aus einem Bild entschieden und gebaut (Stufe 20c).** Entscheide Patrik: **R1 a** die Regler ersetzen nur die Konturen aus dem Bild, Undo deckt es; mit b als Möglichkeit, **R2 a** der Haken **Apply live** im Fenster (aus: Vorschau in Grau und **Take contours**) (§8). Gebaut: `shape/BildKonturen`, Open image..., From image, Show image, „edge" in der Liste. Im Programm ungesehen. |
| 0.18.0 | 2026-10-14 (S122) | **SVG entschieden und gebaut (Stufe 20b).** Entscheide Patrik: **O1 a** Open SVG... fragt bei vorhandenen Konturen Replace, Add oder Cancel (gilt auch für Open image..., 20c); **O2 a** die 64 längsten, Haken für alle bis acht, sonst nur die längste; **O3 a** teilen alle Konturen eine Farbe, bekommen sie die Farben der Reihe (§8, §8a). Gebaut: `shape/SvgKonturen`, Open SVG... im Fenster, die Zeile „n smaller ones left out". Befund: Farben über CSS-Variablen kommen nicht an (sechs `-raw`-Dateien in `asset/shapes/2`). Im Programm ungesehen. |
| 0.17.0 | 2026-10-14 (S122) | **Das Fenster Edit shape entschieden und gebaut (Stufe 20a).** Entscheide Patrik: **Z1 a** Pen zeichnet frei Hand und Punkt für Punkt gemischt, Move point schiebt Ankerpunkte (§8); **Z2 a** der Knopf Draw entfällt (§8a). Gebaut: `shape/Zeichnen`, `loop/FormFenster`, Edit... und New shape... im Creator, das Neulesen einer geschriebenen Form im Knoten und im Creator (`zeichnungFassung`), Töne neu rechnen (`zaehleInstrumentFassung`) (§8a, §14). Im Programm ungesehen. |
| 0.16.0 | 2026-10-13 (S121) | **Die Shape-Schicht im Creator gebaut (Stufe 19e)**, nach der gebilligten Skizze: Waveform Shape, die Zeilen Shape mit Browse... und Contour, die Regler Harmonics, Harmonics envelope, Angle, Start, die Haken Stereo XY und Reverse, das Bild „Shape, as it plays" (§6, §14). Ohne den Knopf Edit... (Stufe 20a). Damit ist ein Instrument mit einer Form im Programm spielbar; gesehen und gehört ist es nicht. |
| 0.15.0 | 2026-10-13 (S121) | **Der Knoten XY Scope entschieden und gebaut (Stufe 19d).** Entscheide Patrik: **X1 a** die Regler (mit Draw Lines oder Dots, Lage, Größe, Color from, Line blend über M1 a hinaus), **X2 a** sechs Presets; die Reihenfolge der Stufen bleibt (19d vor 19e). Gebaut: `XyScopeParams`, `runXyScope`, Panel, Palette, Feldtabelle, sechs Presets (§10, §14). Im Feld Point zeigt `y` nach oben (der rechte Wert). Im Programm ungesehen. |
| 0.14.0 | 2026-10-13 (S121) | **Die Schwingung Shape entschieden und gebaut (Stufe 19c).** Entscheide Patrik: **T1 a** der Pegel — kein Ausgleich, die Form am fernsten Punkt gemessen (davor gemessen: 0,62 bis 1,41); **T2 a** Angle dreht die Form auch mit Stereo XY (§4, §5). Gebaut: `shape/Tonform`, `Schwingung::Form`, `rechneForm`, `.lvinst` Fassung 9 (§5 „Wie die Schwingung gebaut ist", §14). Gemessen: 4,1 ms je Sekunde Ton bei 24 Harmonics; die Kante des Sprungs liegt bei A4 um −33 dB und ist dort zurückgefallener Anteil (§9). Hörproben abgelegt, nichts gehört; offen: Ecken dämpfen, der Sprung. |
| 0.13.0 | 2026-10-13 (S121) | **Der Knoten Shape entschieden und gebaut (Stufe 19b).** Entscheide Patrik: **V1 a** fünf Skriptfelder — Init, Frame, Beat, Song und Per point; **V2 a** die Regler, mit Contour, Points, eigener Farbe und Line blend über M2 b hinaus; **V3 a** neun Presets; **V4 a** XY Scope bekommt dieselben fünf Felder (§10). Gebaut: `shape/Zeichenform`, `ShapeParams`, `runShape`, Panel, Palette, Feldtabelle, neun Presets (§10 „Wie der Knoten Shape gebaut ist", §14). Gemessen: Per point 0,19 ms bei 512 Punkten, der Umriss 0,09 ms in der Vorgabe (Release-clang). Im Programm ungesehen. |
| 0.12.0 | 2026-10-13 (S120) | **Stufe 19a gebaut:** `shape/Kontur` — die Zeichnung, der Pfad als Text, die Datei `.lvcontour` Fassung 1, die Orte (mitgeliefert, `user:<name>`, Pfad), die Reihe mit weichem Gewicht; sechs mitgelieferte Formen; der Bereich Shapes im Export (§7 „Wie es gebaut ist", §14). Die Namen der Felder von Claude gesetzt. |
| 0.11.0 | 2026-10-13 (S120) | **Stufe 18 gebaut:** Pan je Schicht mit dem Gesetz von Spur und Muster, acht Schichten, `.lvinst` Fassung 8; alte Instrumente rechnen Bit für Bit wie vorher (§14). Im Programm ungesehen und ungehört. |
| 0.10.0 | 2026-10-13 (S120) | **Baustufen freigegeben (Patrik)** — 18, 19a bis 19e, 20a bis 20d (§14); **die Grenze der Schichten geht auf acht (Patrik)**, mit Stufe 18 und in derselben Fassung 8 wie Pan (§6a). Nachgesehen: Spur und Muster haben schon ein Pan mit dem Gesetz `min(1, 1 ∓ pan)`; die Schicht bekommt dasselbe. Alle sieben Vorlagen sind geschlossen. |
| 0.9.0 | 2026-10-13 (S120) | **Die Skizzen gebilligt (Patrik)** — das Fenster Edit shape (neu §8a) und die Shape-Schicht im Creator (§6), beide als Beschreibung eingetragen; **G1 a** die Zeile Contour an der Schicht entscheidet, **G2 a** die Regeln für Sprünge von Hand. **Berichtigt nach Patriks Einwand:** der Wellenzug der Preview war von Hand gezeichnet und hatte für einen Zeitpunkt bis zu drei Werte; die Preview zeigt jetzt links und rechts je für sich über der Zeit. Die sechste der sieben Vorlagen ist geschlossen. |
| 0.8.0 | 2026-10-13 (S120) | **Die Module entschieden (Patrik): M1 a** ein Knoten XY Scope in voller Genauigkeit, **M2 b** ein Knoten Shape mit Harmonics und Show circles, **M3 a** Shape gleich nach den Daten, XY Scope nach dem ersten Ton; dazu von Patrik: die üblichen Skriptfelder und einige Presets (§4, §10, §13). §14: die Stufen neu geordnet (19a bis 19e, 20a bis 20d). Die fünfte der sieben Vorlagen ist geschlossen. |
| 0.7.0 | 2026-10-13 (S120) | **Berichtigt:** die Engine läuft mit 44,1 kHz, nicht 48 (§5, §10). §10 genauer, was die Visuals vom Ton bekommen: 512 Werte je Seite alle 33 ms, für Skripte auf 8 Bit gerundet, kein Nachleuchten von sich aus. §3: acht Schichten gemessen und probiert, die Grenze steht weiter auf vier. |
| 0.6.0 | 2026-10-13 (S120) | **As one path entschieden (Patrik): B2 d** — jede Kontur behält ihre eigene Reihe, nacheinander in einem Umlauf, dazwischen ein kurzer Sprung, Harmonics je Kontur; **B1** — wo der Stift springt, ist wählbar: Reihenfolge der Liste (Vorgabe), kürzeste Wege, von Hand eingezeichnet (§4, §7, §9, §13). Davor eine Probe in vier Zeilen, die Zahlen stehen in §9; die ersten drei Zeilen rechneten beide Konturen samt Brücke in eine Reihe — Patriks Einwand führte zur vierten. Offen: wie die Kante des Sprungs klingt (Stufe 19b). |
| 0.5.0 | 2026-10-13 (S120) | **SVG entschieden (Patrik): S1 a, mit Beleg** — Qt Svg zeichnet auf ein mitschreibendes Zeichengerät; der Beleg zeigt vier Aufrufe, die Kurve bleibt Kurve, Lage und Farbe stimmen (§8). Dazu belegt, was bei D4 b aus dem Gedächtnis stand: `QPainterPath` ohne Qt-Anwendung (§7). Neu `tests/unit/UnitTests/test_ShapeBelege.cpp`; Qt Svg in der `Solution.json`. |
| 0.4.0 | 2026-10-13 (S120) | **Konturerkennung entschieden (Patrik): K1 a** über Flächen, Split by Brightness, Colour oder Transparency; **K2 a** die Regler Levels, Smooth, Min size, Detail und die Regeln für Löcher, Bildrand und Anzahl (§4, §8, §13). Davor eine Probe der drei Verfahren an drei Bildern, die Zahlen stehen in §8. |
| 0.3.0 | 2026-10-13 (S120) | **Die Datei entschieden (Patrik): D1 a** eine Zeichnung mit allen Konturen, **D2 a** Ort und Nennung wie bei Instrumenten und Samples, **D3 a** das Instrument nennt die Form über eine feste Kennung der Kontur, **D4 b** Pfade aus Strecken und Kurven, kein Quellbild, keine Koeffizienten — zuerst a (nur Punktfolgen), dann auf Patriks Einwand b (§4, §7, §9, §13). Für das Teilen Patriks Idee **Publish** — ein Paket `<name>.<type>.lvdata` — als eigener Punkt in `Offene_Punkte.md` §7. |
| 0.2.0 | 2026-10-13 (S120) | **F1 a (Patrik):** die Kontur-Datei heißt `.lvcontour`, `.lvshape` bleibt beim Composer (§4, §7, §13). Das Datei-Icon dazu ist gebaut (`asset/img/logo/icons/lumiviz_lvcontour.ico`, Marke mit Kontur und Stift). |
| 0.1.0 | 2026-10-13 (S120) | Erste Fassung: das Grobkonzept auf LumiViz umgeschrieben, Entscheide Z1 a bis Z5 a eingetragen. Nachgesehen: die Visuals bekommen vom Loop beide Seiten getrennt (512 Werte je Seite und Bild); Qt Svg liegt in der Installation; die Endung `.lvshape` ist seit S96 vergeben. Festgehalten: Start und Reverse ändern nur Phasen. Stufen als Vorschlag. |

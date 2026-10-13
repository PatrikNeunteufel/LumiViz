# LumiViz — Klang aus einer Form (Konzept)

> **Version:** 0.9.0
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
| **Vier Schichten** | `kSchichtenMax = 4`; in S120 gemessen und mit 8 probiert — es trägt (`Offene_Punkte.md`, bei der Datenablage unter `kSchichtenMax`), nicht angehoben | As layers endet bei vier Konturen (Z5 a), mit einer angehobenen Grenze bei acht |
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
    das Bild im Scope (Vorschlag; in S119 nicht besprochen, was Angle bei
    gesetztem Haken tut).
- **Start**: ein anderer Startpunkt auf der Kontur, `C_k · e^(i·k·t0)`.
- **Reverse**: die andere Umlaufrichtung, `C_k` und `C_−k` tauschen.

**Mehrere Konturen in einer Schicht (As one path, B2 d):** die Summe oben
gilt **je Kontur** — jede hat ihre eigenen Koeffizienten und ihr eigenes
Gewicht `g_k`, und anders als bei einer einzelnen Kontur behält sie dabei
ihre Lage in der Zeichnung (`C_0` bleibt). Die Phase eines Umlaufs ist auf
die Konturen verteilt, nach ihrer Länge; dazwischen liegt je ein kurzer
gerader Sprung (§9).

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

Die Namen der Felder legt Stufe 19a fest (von Claude zu setzen, im Changelog
zu nennen).

---

## 8. Die Eingabewege

Alle drei enden im selben Fenster und in derselben Datei: einer Liste
geschlossener Konturen, jede in einer Farbe, einige davon gewählt.

**Zeichnen.** Mit der Maus eine geschlossene Linie ziehen. Offen: frei Hand
oder Punkt für Punkt, glätten, nachträglich Punkte schieben.

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

---

## 8a. Das Fenster Edit shape (Skizze gebilligt, Patrik S120)

Mit dem Zeichenwerkzeug gezeigt; hier als Beschreibung.

- **Kopf:** „Edit shape – <Name>", daneben der Ort („My shapes"); rechts die
  drei Eingabewege **Draw**, **Open SVG...**, **Open image...**
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
Preview. Noch offen, klein: was Draw genau tut (frei Hand oder Punkt für
Punkt, glätten); die Sätze der Meldungen.

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
Presets** mit. Meine Lesart, nicht nachgefragt: das gilt für beide Knoten;
die Regler (Harmonics, Gain, Drehung …) sind damit aus dem Skript und im
Takt steuerbar, wie bei den anderen Knoten der Chain. Welche Felder genau
(auch ein Feld je Punkt?) und welche Presets, legt die Stufe fest.

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
7. **Baustufen** (§14).

Daneben, klein, wo sie hingehören: Ecken dämpfen, Pegel, Angle bei gesetztem
Stereo XY, Pan bei gesetztem Stereo XY, ob Stimmen und Pitch sweep für Shape
gelten (§5, §6).

---

## 14. Stufen (Vorschlag, nicht freigegeben)

Die Nummern schließen an `Loop_Konzept.md` §5 an (zuletzt Stufe 17).

| Stufe | Inhalt | hängt an |
|---|---|---|
| 18 | **Pan je Schicht**, `.lvinst` Fassung 8 — offen, ob dabei die Grenze der Schichten von vier auf acht geht (gemessen in S120, nicht entschieden) | nichts — sofort möglich |
| 19a | Form als Daten: Kontur als Pfad, Koeffizienten, die Datei `.lvcontour`, ein paar mitgelieferte Formen | — |
| 19b | der Knoten **Shape** für die Effect Chain, mit Show circles, Skriptfeldern und Presets (M2 b, M3 a) | 19a |
| 19c | die Schwingung Shape im Klangerzeuger: Harmonics, Angle, Stereo XY, Start, Reverse, Harmonics envelope, mehrere Konturen mit Sprung; mit einer Form aus einer Datei, ohne Fenster. **Hier hören und messen: die Kante des Sprungs** (B2 d) | 19a |
| 19d | der Knoten **XY Scope**, mit Skriptfeldern und Presets (M1 a, M3 a) | — (gebraucht ab 19c) |
| 19e | die Shape-Schicht im Creator | Skizze |
| 20a | das Form-Fenster: zeichnen | Skizze |
| 20b | SVG über das mitschreibende Zeichengerät (S1 a) | 20a |
| 20c | Konturen aus einem Bild (K1 a, K2 a) | 20a |
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
| 0.9.0 | 2026-10-13 (S120) | **Die Skizzen gebilligt (Patrik)** — das Fenster Edit shape (neu §8a) und die Shape-Schicht im Creator (§6), beide als Beschreibung eingetragen; **G1 a** die Zeile Contour an der Schicht entscheidet, **G2 a** die Regeln für Sprünge von Hand. **Berichtigt nach Patriks Einwand:** der Wellenzug der Preview war von Hand gezeichnet und hatte für einen Zeitpunkt bis zu drei Werte; die Preview zeigt jetzt links und rechts je für sich über der Zeit. Die sechste der sieben Vorlagen ist geschlossen. |
| 0.8.0 | 2026-10-13 (S120) | **Die Module entschieden (Patrik): M1 a** ein Knoten XY Scope in voller Genauigkeit, **M2 b** ein Knoten Shape mit Harmonics und Show circles, **M3 a** Shape gleich nach den Daten, XY Scope nach dem ersten Ton; dazu von Patrik: die üblichen Skriptfelder und einige Presets (§4, §10, §13). §14: die Stufen neu geordnet (19a bis 19e, 20a bis 20d). Die fünfte der sieben Vorlagen ist geschlossen. |
| 0.7.0 | 2026-10-13 (S120) | **Berichtigt:** die Engine läuft mit 44,1 kHz, nicht 48 (§5, §10). §10 genauer, was die Visuals vom Ton bekommen: 512 Werte je Seite alle 33 ms, für Skripte auf 8 Bit gerundet, kein Nachleuchten von sich aus. §3: acht Schichten gemessen und probiert, die Grenze steht weiter auf vier. |
| 0.6.0 | 2026-10-13 (S120) | **As one path entschieden (Patrik): B2 d** — jede Kontur behält ihre eigene Reihe, nacheinander in einem Umlauf, dazwischen ein kurzer Sprung, Harmonics je Kontur; **B1** — wo der Stift springt, ist wählbar: Reihenfolge der Liste (Vorgabe), kürzeste Wege, von Hand eingezeichnet (§4, §7, §9, §13). Davor eine Probe in vier Zeilen, die Zahlen stehen in §9; die ersten drei Zeilen rechneten beide Konturen samt Brücke in eine Reihe — Patriks Einwand führte zur vierten. Offen: wie die Kante des Sprungs klingt (Stufe 19b). |
| 0.5.0 | 2026-10-13 (S120) | **SVG entschieden (Patrik): S1 a, mit Beleg** — Qt Svg zeichnet auf ein mitschreibendes Zeichengerät; der Beleg zeigt vier Aufrufe, die Kurve bleibt Kurve, Lage und Farbe stimmen (§8). Dazu belegt, was bei D4 b aus dem Gedächtnis stand: `QPainterPath` ohne Qt-Anwendung (§7). Neu `tests/unit/UnitTests/test_ShapeBelege.cpp`; Qt Svg in der `Solution.json`. |
| 0.4.0 | 2026-10-13 (S120) | **Konturerkennung entschieden (Patrik): K1 a** über Flächen, Split by Brightness, Colour oder Transparency; **K2 a** die Regler Levels, Smooth, Min size, Detail und die Regeln für Löcher, Bildrand und Anzahl (§4, §8, §13). Davor eine Probe der drei Verfahren an drei Bildern, die Zahlen stehen in §8. |
| 0.3.0 | 2026-10-13 (S120) | **Die Datei entschieden (Patrik): D1 a** eine Zeichnung mit allen Konturen, **D2 a** Ort und Nennung wie bei Instrumenten und Samples, **D3 a** das Instrument nennt die Form über eine feste Kennung der Kontur, **D4 b** Pfade aus Strecken und Kurven, kein Quellbild, keine Koeffizienten — zuerst a (nur Punktfolgen), dann auf Patriks Einwand b (§4, §7, §9, §13). Für das Teilen Patriks Idee **Publish** — ein Paket `<name>.<type>.lvdata` — als eigener Punkt in `Offene_Punkte.md` §7. |
| 0.2.0 | 2026-10-13 (S120) | **F1 a (Patrik):** die Kontur-Datei heißt `.lvcontour`, `.lvshape` bleibt beim Composer (§4, §7, §13). Das Datei-Icon dazu ist gebaut (`asset/img/logo/icons/lumiviz_lvcontour.ico`, Marke mit Kontur und Stift). |
| 0.1.0 | 2026-10-13 (S120) | Erste Fassung: das Grobkonzept auf LumiViz umgeschrieben, Entscheide Z1 a bis Z5 a eingetragen. Nachgesehen: die Visuals bekommen vom Loop beide Seiten getrennt (512 Werte je Seite und Bild); Qt Svg liegt in der Installation; die Endung `.lvshape` ist seit S96 vergeben. Festgehalten: Start und Reverse ändern nur Phasen. Stufen als Vorschlag. |

# Kalibrier-Plan: die mitgelieferten Presets

> **Version:** 1.0.0
> **Datum:** 2026-08-09 (Session 73, für Session 74)
> **Typ:** Umsetzungsplan
> **Status:** Entwurf — noch nicht begonnen
> **Sprache:** Deutsch
> **Gültigkeit:** die 10 AVS- und 19 MilkDrop-Presets aus `asset/presets/`
> **Anlass:** Erster Vergleichslauf gegen AvsRef/MilkdropRef in S73 (Auftrag Patrik)

Die 29 mitgelieferten Presets sind das Schaufenster des Projekts — sie liegen im
README und werden bei jedem Build neben die Exe kopiert. Ausgerechnet bei denen
sollte die Treue stimmen. Der erste Vergleichslauf zeigt: bei etwa der Hälfte
tut sie das, bei der anderen nicht.

**Dieser Plan ist bewusst klein gehalten.** Er nimmt sich 29 Presets vor, nicht
den ganzen Korpus. Was dabei gefunden wird, gilt aber meistens breiter.

---

## 0. Vorab: die Messung von S73 ist teilweise unbrauchbar

Bevor irgendetwas gefixt wird, muss die Messung stimmen. Zwei Fehler stecken im
S73-Lauf:

### 0.1 🔴 Der MilkDrop-Stapellauf verunreinigt sich selbst

`MilkdropStandalone --auto <ordner>` rendert alle Presets **in einem Prozess**,
und MilkDrop-Presets **erben das Bild des Vorgängers** — das ist
Original-Verhalten, kein Fehler. Im Vergleichslauf heißt das: Preset N zeigt
Reste von Preset N−1.

**Nachgewiesen an `Helix`:** im Stapellauf stand ein Herz im Bild — das
alphabetisch davor laufende `Dancing Hearts`. Im eigenen Prozess gestartet
verschwindet es (Befund Patrik, S73).

> **Regel für alle künftigen Referenzvergleiche: ein Prozess je Preset.**
> Der AVS-Vergleich (`compare_avsref.py`) macht das schon richtig — er ruft den
> Standalone je Preset einmal auf. Für MilkDrop gibt es kein Gegenstück; das
> ist Aufgabe 1.

**Betroffen sind alle MilkDrop-Zahlen aus S73 außer der jeweils ersten** eines
Laufs. Sie sind als Richtwert brauchbar, nicht als Befund.

### 0.2 🟠 Ein Einzelframe nach 120 Frames misst auch Phasenversatz

MilkDrop ist ein Rückkopplungssystem. Eine winzige Abweichung in Frame 3 kann
bis Frame 120 zu einem völlig anderen Standbild führen, **obwohl beide Seiten
das Preset korrekt zeigen**. Ein Pixel-MAE auf einem späten Einzelbild
vermischt damit zwei Dinge: echten Fehler und Phasenversatz.

Gegenmittel (in dieser Reihenfolge, aufsteigender Aufwand):

1. **Früh messen** — Frame 10/20/30 zusätzlich zu 120. Früh ist der
   Phasenversatz klein, ein echter Fehler aber schon da.
2. **Über eine Sequenz messen** statt über ein Bild — Frame-Hashes wie im
   `--ab`-Modus des Standalones.
3. **Deterministisch takten** — der AVS-Vergleich hat `--beat-period`; für
   MilkDrop fehlt das Gegenstück.

---

## Aufgabe 1 — Ein ehrliches MilkDrop-Vergleichswerkzeug

**Ziel:** ein Skript, das für einen Preset-Ordner belastbare Zahlen liefert.

Anforderungen:

- **Ein Prozess je Preset** auf beiden Seiten (§0.1)
- **Mehrere Frames** je Preset (z. B. 10 / 30 / 120), Metriken je Frame
- Ausgabe wie `compare_avsref.py`: `report.md` + Montagen
  (LumiViz | Referenz | Differenz)
- Die Referenz-Ordnerstruktur automatisch aufbauen
  (`<wurzel>/data` + `<wurzel>/presets`, siehe Werkzeug-Wegleitung §2.5)
- Urteil je Preset statt nur einer Zahl: **OK / PRUEFEN / BEFUND**

`asset/calibration/milkdrop/compare_ref.py` taugt als Vorlage, deckt aber nur
Presets aus einem Triage-Lauf ab. Entweder erweitern oder ein Geschwister
danebenlegen — Entscheid beim Bauen.

**Fertig, wenn:** ein Lauf über die 19 mitgelieferten Presets reproduzierbar
dieselben Zahlen liefert (zweimal laufen lassen, Diff = 0).

---

## Aufgabe 2 — Wellenform-Modus 7 steht auf dem Kopf

**Der einzige Befund aus S73, der schon eingegrenzt ist.** Einzelheiten in
[Offene_Punkte §3](../Offene_Punkte.md).

Kurz: von 19 Presets profitieren genau die vier mit `nWaveMode=7` davon, das
Bild vertikal zu spiegeln. Die Codestelle ist
`MilkdropVisualizer.cpp:3887-3903` (Modus 7: zwei Linien mit Stereo-Trennung).

**Vorgehen:**

1. Den Zweig **Zeile für Zeile** gegen `milkdropfs.cpp` legen — nicht raten.
   Zwei Verdächtige: das Vorzeichen von `perpY` (`ang2 + 1.57` gegen `− 1.57`)
   und der Übergang in Bildschirmkoordinaten.
2. Die Y-Konvention der Datei beachten: **EINE Rechenebene, genau ein Flip im
   Composite** (Kopfkommentar). Ein zweiter Flip an anderer Stelle wäre ein Fix,
   der das Symptom versteckt statt die Ursache zu beheben.
3. Auch Modus 6 prüfen — er teilt sich den Zweig, und `Lasershow` (Modus 6) hat
   MAE 0,085, also nicht null.

**Gegenprobe (zwingend):** die vier Mode-7-Presets müssen ohne Spiegel besser
werden, **die 14 anderen dürfen sich nicht verschlechtern**. Ohne diese zweite
Hälfte ist der Fix nicht abgenommen.

**Vorsicht bei der Beweislage:** Die klarste Messung ist
`Dancing Hearts (2nd edit)` — sie lief als erste im Stapel und ist damit als
einzige unverunreinigt (§0.1). Nach Aufgabe 1 die Zahlen neu erheben.

---

## Aufgabe 3 — Die Farben stimmen nicht

Zweiter Befund an `Dancing Hearts`, unabhängig von der Spiegelung: bei uns
**weiß/rosa**, im Original **gelb/orange**. Auch gespiegelt bleibt MAE bei 0,260.

Das hat noch niemand angesehen. Kandidaten, von wahrscheinlich nach unwahrscheinlich:

- **Sättigung/Clipping im Composite** — unser Bild wirkt ausgewaschen, als würde
  etwas zu hell aufaddiert und in Weiß laufen
- **Farbzuweisung der Wellenform** (`wave_r/g/b` und deren Beat-Modulation)
- **Gamma** im Composite-Pass

**Erster Schritt ist eine Messung, keine Vermutung:** ein Preset mit fester
Farbe und ohne Rückkopplung bauen, dann beide Seiten vergleichen. Steht der
Unterschied schon dort, ist es die Farbzuweisung; erscheint er erst mit
Rückkopplung, ist es Sättigung.

---

## Aufgabe 4 — Die fünf AVS-Befunde

Der AVS-Vergleich ist methodisch sauber (ein Prozess je Preset,
`--beat-period 30`, 320×240). Ergebnis: **5 von 10 unter der Schwelle**,
5 zum Prüfen, 0 Ladefehler.

| Preset | MAE | Auffälligkeit |
|---|---|---|
| `02_color extasy` | 0,119 | dMean 0,120 — großflächiger Helligkeitsunterschied |
| `10_the ring` | 0,113 | **dMaxLuma 0,262** — Spitzlichter zu hell oder zu dunkel |
| `07_movin wall` | 0,073 | dMean 0,084, aber dMaxLuma 0,000 |
| `09_rotating_things` | 0,010 | dMean 0,026 bei kleinem MAE |
| `02_flowers` | 0,015 | **dMaxLuma 0,150** bei kleinem MAE |

**Zwei Muster, die man getrennt angehen sollte:**

- `movin wall` und `color extasy` weichen **flächig** ab (dMean hoch, dMaxLuma
  niedrig bzw. mittel) — Verdacht Blend-Modus oder Helligkeitsskalierung.
- `the ring` und `flowers` weichen **in den Spitzen** ab (dMaxLuma hoch, MAE
  klein) — Verdacht Clipping oder Additiv-Blending.

**Vorgehen je Preset:** Bisektion mit `bisect_avs.py` — Knoten für Knoten
abschalten, bis die Abweichung verschwindet. Das ist das eingeführte Verfahren
([AVS-Kalibrier-Methodik](AVS_Kalibrier_Methodik.md)), und es beantwortet
zuverlässig „welcher Effekt", bevor man „warum" fragt.

**In zwei Größen messen** (Merkregel aus S47): 320×240 versteckt
größenabhängige Fehler. Nach dem Fix zusätzlich bei 1024×768 gegenprüfen.

---

## Aufgabe 5 — Der Puffer beim Preset-Wechsel

**Anlass:** ausgerechnet dieses Verhalten hat in S73 meine Messung verunreinigt
(§0.1) — und dabei fiel auf, dass ein Teil davon nie abgenommen wurde.

MilkDrop löscht den Bildpuffer beim Preset-Wechsel **nie**; jedes Preset startet
auf dem letzten Bild des Vorgängers. Das ist Kern-Ästhetik, kein Fehler.
LumiViz bietet vier Modi (S66, je Knoten und als App-Vorgabe):

| Modus | Verhalten | Stand |
|---|---|---|
| **Behalten** | Original-Semantik, Erbe bleibt | ✅ Sichttest S66 |
| **Löschen** | Rausch-Saat + `rand_preset`-Seed-Reset | ✅ Sichttest S66 |
| **Fading** | einmaliger Mix, Erbe-Regler | ✅ Sichttest S66 |
| **Ausblenden** | Echo-Dämpfung je Frame nach dem Warp, Sekunden-Regler | ⬜ **offen seit S66** |

### 5.1 Den offenen Sichttest nachholen

„Ausblenden" ist der einzige Modus ohne Abnahme. Er dämpft das Erbe über eine
einstellbare Zeit, statt es einmalig zu mischen — zu prüfen ist, ob die
Dämpfung der eingestellten Sekundenzahl entspricht und ob sie nach dem Warp
greift (nicht davor).

### 5.2 Messbar statt nur ansehbar

Seit S67 gibt es `MilkdropStandalone --ab`: erstes Preset N Frames rendern, auf
das letzte wechseln, dann je Frame **FNV-1a-Hash + Mittel-RGB** loggen. Damit
lässt sich jeder der vier Modi als Kurve belegen statt nur beurteilen:

- **Behalten** — Konvergenz gegen den Kaltstart-Verlauf, ab einem Punkt bitgleich
  (in S67 ab Frame 120 gemessen)
- **Löschen** — sofortiger Sprung, danach identisch mit einem echten Kaltstart
- **Fading / Ausblenden** — monotone Annäherung; die Sekundenzahl muss sich in
  der Kurve wiederfinden

**Das ist der eigentliche Ertrag dieser Aufgabe:** vier Modi, die bisher nur
per Auge beurteilt wurden, bekommen je eine reproduzierbare Kurve. Wer später
am Puffer-Pfad etwas ändert, sieht sofort, ob er etwas kaputtgemacht hat.

### 5.3 Gegen die Referenz halten

`MilkdropRef` kennt nur „Behalten" — das Original hat die anderen drei Modi
nicht. Vergleichbar ist also **nur der Behalten-Modus**, und genau der muss
gegen die Referenz konvergieren. Die übrigen drei sind LumiViz-eigene
Erweiterungen und werden gegen sich selbst geprüft (Determinismus, Monotonie,
Zeitkonstante), nicht gegen das Original.

> **Merkregel, die S73 gekostet hat:** Für Referenzvergleiche ist „Behalten"
> die falsche Einstellung, sobald mehrere Presets in einem Lauf gerendert
> werden. Entweder ein Prozess je Preset (§0.1) — oder bewusst auf „Löschen"
> stellen und das im Report vermerken.

---

## Aufgabe 6 — Echte Musik als Audioquelle für die Standalones

**Auftrag Patrik (S73):** die Standalones sollen eine echte Audiodatei
abspielen können statt des synthetischen Signals. Referenzdatei:
`music/David Guetta - Greatest Hits (2012)/05. One Love (feat. Estelle).mp3`.

**Warum das nützt:** das synthetische Signal ist ein Sinus mit Beat-Puls. Presets,
die auf echte Musikdynamik ausgelegt sind — Bass-Einsätze, Refrain-Aufbau,
Stille zwischen Schlägen — zeigen damit nie, was sie können. Für Schaufenster-
Screenshots und für die Beurteilung „sieht das gut aus" ist echtes Material
deutlich aussagekräftiger. Der `--klangfarbe`-Schalter aus S73 ist nur ein
Behelf in diese Richtung.

### Ausgangslage

- **Keiner der beiden Standalones linkt BASS** (`Solution.json`, executables:
  `externals: ["Qt6","lua54"]`). Die App tut es, die Werkzeuge nicht.
- **Qt Multimedia ist aber schon da** (die FFmpeg-DLLs liegen im Build) →
  `QAudioDecoder` ist der billigere Weg als BASS nachzuziehen.
- Gefüttert werden muss `updateAudioStereo(spec, bins, wave, frames, chans)` —
  also braucht es zusätzlich eine **FFT** für das Spektrum; die Wellenform
  kommt direkt aus dem PCM.

### Anforderungen

1. **`--audio-datei <pfad>`** an beiden Standalones; ohne den Schalter bleibt
   alles wie bisher.
2. **Deterministisch**: das Audio wird nach Frame-Index abgetastet
   (`t = frame/60`), nicht nach Echtzeit. Zwei Läufe mit derselben Datei und
   derselben Frame-Zahl müssen bitgleich sein — sonst ist das Werkzeug für
   Vergleiche unbrauchbar.
3. **`--audio-start <sekunden>`**, damit man eine interessante Stelle treffen
   kann statt der Einleitung.
4. Das Bandmodell muss zum bestehenden passen (`MilkLoudness`-Terzen,
   761,2 / 2897,1 Hz), sonst reagieren Presets anders als in der App.

### ⚠️ Das darf NICHT für Referenzvergleiche verwendet werden

`AvsRef` und `MilkdropRef` erzeugen ihr Audio **selbst** — formelgleich zum
synthetischen Signal der Standalones. Genau darauf beruht die Vergleichbarkeit.
Sobald unsere Seite echte Musik hört und die Referenz weiter ihren Sinus,
vergleicht man zwei verschiedene Eingaben, und jede Zahl daraus ist wertlos.

> **Regel: `--audio-datei` ist für Schaufenster und Augenschein.
> Referenzläufe bleiben beim synthetischen Signal.**

Das im Skript aus Aufgabe 1 hart absichern: wird eine Audiodatei gesetzt,
verweigert der Vergleichslauf den Dienst oder markiert den Report unübersehbar.

---

## Reihenfolge und Aufwand

| # | Aufgabe | Aufwand | hängt ab von |
|---|---|---|---|
| 1 | MilkDrop-Vergleichswerkzeug | mittel | — |
| 2 | Wellenform-Modus 7 | klein | 1 (für die Gegenprobe) |
| 4 | AVS-Bisektion, 5 Presets | mittel | — |
| 5 | Puffer beim Preset-Wechsel | klein–mittel | — (nutzt `--ab`, existiert) |
| 6 | Echte Musik als Audioquelle | mittel | — (unabhängig) |
| 3 | Farb-Befund | offen | 1, 2 |

**Empfehlung:** mit **Aufgabe 1** anfangen. Ohne belastbare Zahlen wird jeder
MilkDrop-Fix zum Ratespiel, und Aufgabe 2 braucht sie ohnehin für die
Gegenprobe. **Aufgabe 4** kann parallel laufen — der AVS-Pfad ist unabhängig
und methodisch schon in Ordnung.

**Aufgabe 5 passt gut dazwischen:** sie braucht kein neues Werkzeug (`--ab` gibt
es seit S67), schließt einen seit S66 offenen Sichttest und liefert die
Merkregel, an der Aufgabe 1 ohnehin hängt.

**Aufgabe 3 zuletzt.** Sie ist am schlechtesten umrissen, und möglicherweise
räumt der Mode-7-Fix einen Teil davon mit ab.

## Fertig-Kriterium

Nicht „alle 29 identisch" — das wäre bei Rückkopplungssystemen unehrlich.
Sondern:

- **AVS:** die fünf Befunde erklärt, jeder mit benanntem Knoten und Ursache;
  gefixt oder als bewusste Abweichung dokumentiert
- **MilkDrop:** die 19 mit dem neuen Werkzeug gemessen, jedes Preset mit Urteil
  OK / PRUEFEN / BEFUND; jedes BEFUND-Preset auf einen Effekt eingegrenzt
- **Mode 7** gefixt, mit bestandener Gegenprobe über alle 19
- **Puffer-Modi:** alle vier mit einer Kurve belegt, „Ausblenden" abgenommen —
  der letzte offene Sichttest aus S66

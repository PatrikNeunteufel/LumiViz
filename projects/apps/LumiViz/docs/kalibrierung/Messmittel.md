# Messmittel — die Kalibrier-Raster

> **Version:** 1.0.0
> **Datum:** 2026-08-09 (Session 74)
> **Typ:** Guide
> **Status:** Aktiv — AVS umgesetzt, andere Formate offen
> **Sprache:** Deutsch
> **Anlass:** Vorgabe Patrik S74 — „für Transform-Knoten würde ein
> vordefiniertes Raster helfen"

## Warum überhaupt

Ein Preset gegen die Referenz zu halten beantwortet „stimmt das Ganze". Es
beantwortet nicht, **welcher Knoten** danebenliegt. Dafür misst man einzelne
Knoten — und dabei zeigt sich das Grundproblem:

**Ein Transform-Knoten hat auf schwarzer Fläche nichts zu transformieren.**
Movement, Water, Blur, Fadeout, Mosaic: allein gestartet liefern beide
Renderer schwarz, der Vergleich meldet 0,000, und das liest sich wie
„geprüft". Es heißt aber nur „nichts zu sehen".

Beim Einzelknoten-Lauf in S74 betraf das die **Mehrzahl** aller Knoten. Der
größte AVS-Befund der Session ist genau so aufgeflogen: `Water` (Id 20) maß
ohne Quelle 0,000 — mit Raster **MAE 0,332**.

## Der Vertrag

Ein Messmittel ist ein **normaler Knoten im Preset**, kein Debug-Overlay des
Hosts. Nur so zeichnet die Referenz genau dasselbe; ein Host-Overlay würde die
Referenz gar nicht kennen und den Vergleich sinnlos machen.

Daraus folgen drei Eigenschaften:

1. **Neutral.** Das Muster allein muss gegen die Referenz ~0 liefern.
2. **Sparsam im Sprachumfang.** Nur Grundrechenarten und `%`. Je weniger es
   braucht, desto weniger kann es selbst schiefgehen — es ist Messmittel,
   nicht Messobjekt.
3. **Abgenommen, bevor es misst.** Siehe unten.

## Die Familie (AVS)

Erzeugt mit `asset/calibration/avs/make_raster_avs.py`.

| Muster | Punkte | Deckt auf | Abnahme |
|---|---|---|---|
| `gitter` | g² (32 → 1024) | Verschiebung, Verzerrung, Drehung, Spiegelung — die Farbe kodiert die Herkunft (rot = Spalte, grün = Zeile), ein versetzter Punkt sagt auch **woher** er kommt | MAE 0,001 ✅ |
| `verlauf` | 2·z (240 → 480) | Farbe, Gamma, Sättigung, Blend-Modi — alles, was eine **Fläche** braucht und ein Punktgitter nicht liefern kann | MAE 0,002 ✅ |
| `keil` | 2·z (32 → 64) | Helligkeitsskala, Clipping, Gamma — bewusst grau, damit sich Helligkeits- nicht mit Farbfehlern vermischt | MAE 0,000 ✅ |
| `kreuz` | 10 | Ursprung, Achsrichtung, Seitenverhältnis — die Fragen, an denen man sich sonst verrennt | MAE 0,000 ✅ |

```bash
python asset/calibration/avs/make_raster_avs.py raster.avs --muster gitter --gitter 32
python asset/calibration/avs/make_raster_avs.py verlauf.avs --muster verlauf --gitter 240
```

### Obergrenze der Punktzahl

**Nicht über 4096 Punkte gehen.** LumiViz klemmt die SuperScope-Punktzahl dort,
das Original erst bei 128·1024 = 131072 (Befund S74, `SuperscopeModule.cpp:586`
gegen `r_sscope.cpp:282`). Darüber misst das Messmittel den eigenen Abbruch
mit. Der Generator warnt.

Die Reihe, an der es aufgefallen ist — Gitter allein gegen die Referenz:

| Gitter | Punkte | MAE |
|---|---|---|
| 32 | 1024 | 0,001 |
| 64 | 4096 | 0,001 |
| 72 | 5184 | 0,056 |
| 80 | 6400 | 0,100 |
| 96 | 9216 | 0,165 |

Im Bild: die Referenz füllt das Bild, wir brechen bei ~4096 gezeichneten
Punkten ab.

## Abnahme — die Regel, die eine Session gekostet hat

> **Ein Messmittel wird abgenommen, BEVOR damit gemessen wird.**

Beim `gitter`-Raster habe ich das getan (MAE 0,001, leeres Differenzbild).
Beim dichteren 96er nicht — und ein kompletter Lauf über alle dreizehn
Listen-Blend-Modi meldete daraufhin durchgehend Abweichungen (MAE 0,131–0,184).
Sechs der dreizehn zeigten exakt die Zahl des kaputten Messmittels. Gemessen
wurde überwiegend die Probe, nicht der Blend.

Abnahme heißt: das Muster **allein** gegen die Referenz, und zwar mit **beidem**
— Metrik nahe null UND ein Blick auf die Montage. Eine kleine Zahl allein
beweist nichts (siehe [INDEX](INDEX.md), Regel 5).

## Einsatz

### Einzelknoten mit Quelle

```bash
python asset/calibration/avs/bisect_avs.py "<preset.avs>" "<ziel>" --solo --raster raster.avs
```

Erzeugt je Knoten ein Preset mit **genau diesem Knoten** plus dem Raster als
Quelle.

> **Das Raster gehört IN die Liste, nicht davor.** Vor eine Effektliste gesetzt
> ist es im Ergebnis nicht mehr zu sehen — die Liste räumt die Fläche unter
> sich weg, beide Renderer liefern schwarz. Erst als erstes **Kind** derselben
> Liste steht die Quelle im selben Blend-Kontext wie der Prüf-Knoten. Der erste
> Anlauf in S74 hat genau daran nichts gefunden. Das Werkzeug macht es
> inzwischen richtig.

### Listen-Kontext: Blend, Buffer, Reihenfolge

```bash
python asset/calibration/avs/make_listenprobe_avs.py "<ziel>" --raster raster.avs --knoten "<preset.avs>#<index>" --reihenfolge
```

Baut Proben mit dem Raster als Untergrund und einer Liste darüber, deren Kopf
variiert wird: alle 13 Blend-In-Modi, die Adjustable-Skala, Buffer 0–2 mit und
ohne Invertierung, beide Reihenfolgen.

**Zwei Formatfallen im Listen-Kopf**, beide liefern *falsch-negative*
Ergebnisse — die Probe misst über alle Modi denselben Wert und sieht aus wie
ein bestandener Test:

- Die **Größe der erweiterten Daten** steht im oberen Byte von `mode`
  (`set_extended_datasize(36)`). Fehlt sie, ist `ext = 5`, `load_config` liest
  keinen der acht Kopf-Werte und läuft mitten in die Kinder.
- **`blendout` liegt invertiert im Feld** (`((mode>>16)&31)^1`), und 0 heißt
  nicht „Vorgabe", sondern „Ergebnis nicht zurückschreiben". Richtig ist 1.

### Buffer-Kreislauf

```bash
python asset/calibration/avs/make_bufferprobe_avs.py "<ziel>" --raster gitter.avs --zweit verlauf.avs
```

Baut den vollständigen Kreislauf **Raster → sichern → zweites Bild →
zurückholen** über alle Blend-Modi, alle acht Buffer, plus Kreuzprobe
(in 0 sichern, aus 1 holen), ungeschriebenen Buffer, Nur-Sichern und den
wechselnden Modus.

Ein Kreislauf ist Pflicht: der Listen-Prüfstand deckt `blendin = 12`
(Buffer als Maske) zwar ab, aber mit **unbeschriebenem** Buffer — dort liefern
beide Seiten je ein einziges Bild, die Probe unterscheidet nicht.

**Was der Stand gleich beim ersten Lauf gefunden hat:** die beiden Blend-Modi,
die auf einem Zeilen- oder Pixelraster arbeiten, waren vertikal gespiegelt
(AVS zählt Zeilen von oben, `gl_FragCoord.y` von unten — bei gerader Höhe
kippt die Parität). MAE 0,451. Die Gegenprobe bei **ungerader** Höhe lieferte
0,002 und hat die Ursache gezeigt, bevor eine Codezeile geändert wurde.

> **Merkregel für jeden Effekt mit Zeilen- oder Pixelraster:** einmal bei
> gerader und einmal bei ungerader Höhe messen. Ein Paritätsfehler ist bei
> einer der beiden Höhen unsichtbar.

### Prüf-Knoten muss zeichnen

Ein Blend-Prüfstand mit einem Knoten, der nichts malt, misst nichts — alle
Modi liefern denselben Wert. In S74 sind dabei zwei Anläufe verpufft
(Effekt 21 und Effekt 38 zeichnen für sich genommen nichts). **Als Prüf-Knoten
eignet sich ein zweites Raster**, dessen Muster sich vom Untergrund
unterscheidet.

## Andere Formate

| Format | Stand |
|---|---|
| **AVS** | ✅ vier Muster, alle abgenommen |
| **MilkDrop** | ⬜ offen. `MilkdropStandalone` hat ein Host-Raster (Taste `G`, `render.debugGrid`) — das taugt zum Hinsehen, **nicht** für Referenzvergleiche, weil `MilkdropRef` es nicht kennt. Ein Gegenstück müsste als `.milk`-Preset gebaut werden (Wellenform-Zeichnung mit festen Koordinaten) |
| **Shadertoy / ISF** | ⬜ offen. Ohne Referenz-Renderer gibt es kein Treue-Urteil; die Raster wären hier Regressionsanker gegen uns selbst — dafür genügt ein eingefrorenes Sollbild je Muster |

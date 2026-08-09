# Preset-Anleitung — eigene Visuals bauen

> **Version:** 1.0.0
> **Datum:** 2026-08-09 (Session 73)
> **Typ:** Guide
> **Status:** Aktiv
> **Sprache:** Deutsch
> **Zielgruppe:** Wer über das Ausprobieren hinaus ist und verstehen will,
> wie ein Preset entsteht

Der [Quickstart](Preset_Quickstart.md) bringt dich in fünf Schritten zum ersten
Ergebnis. Hier geht es um das Warum: wie eine Kette denkt, welche Bausteine es
gibt, und wie man aus einem Effekt-Stapel ein Bild macht, das man wiedererkennt.

Die **Bedienung** der Oberfläche steht im [Benutzerhandbuch](Benutzerhandbuch.md),
Kapitel 11. Hier wird sie vorausgesetzt.

## Inhalt

1. [Was ein Preset ist](#1-was-ein-preset-ist)
2. [Die Kette denkt von oben nach unten](#2-die-kette-denkt-von-oben-nach-unten)
3. [Die drei Sorten Knoten](#3-die-drei-sorten-knoten)
4. [Rückkopplung — woher die Tiefe kommt](#4-rückkopplung--woher-die-tiefe-kommt)
5. [Audio hereinholen](#5-audio-hereinholen)
6. [Formeln statt fester Werte](#6-formeln-statt-fester-werte)
7. [Ein Preset von Grund auf](#7-ein-preset-von-grund-auf)
8. [Speichern, ordnen, weitergeben](#8-speichern-ordnen-weitergeben)
9. [Fremde Presets als Steinbruch](#9-fremde-presets-als-steinbruch)
10. [Wenn nichts zu sehen ist](#10-wenn-nichts-zu-sehen-ist)

---

## 1. Was ein Preset ist

Ein Preset in LumiViz ist eine **Effektkette**: eine geordnete Liste von Knoten,
die nacheinander auf dasselbe Bild losgelassen werden. Gespeichert wird sie als
`.lvfx` (JSON).

Es gibt kein „Bild" im Sinne einer Leinwand, die du bemalst. Es gibt einen
**Puffer**, der bei jedem Frame durch die Kette läuft. Was am Ende darin steht,
siehst du. Was im nächsten Frame darin steht, hängt davon ab, was du im
vorherigen hinterlassen hast — daher stammt fast alles, was in Visuals nach
Bewegung und Tiefe aussieht (siehe [§4](#4-rückkopplung--woher-die-tiefe-kommt)).

Daneben lädt LumiViz **fremde Formate** — `.avs` (Winamp AVS) und `.milk`
(MilkDrop). Die werden beim Import in dieselbe Kettenstruktur übersetzt und sind
danach ganz normal bearbeitbar.

## 2. Die Kette denkt von oben nach unten

Der Baum im Effect-Chain-Panel ist die Ausführungsreihenfolge: **oben zuerst.**

Daraus folgt die wichtigste Regel beim Bauen:

> **Die Reihenfolge ist der halbe Look.**

Ein `blur` vor einem `movement` weicht das Bild auf und verzerrt dann das
Weiche. Dasselbe `blur` danach verzerrt scharf und weicht das Ergebnis auf.
Zwei völlig verschiedene Bilder aus denselben zwei Knoten.

Wenn ein Preset nicht so aussieht wie gedacht, ist die erste Frage fast immer:
*steht der Knoten an der richtigen Stelle?* — nicht: *stimmt der Wert?*

**Listen** (`list`) fassen Knoten zu einer Gruppe zusammen. Das ist mehr als
Ordnung: eine Liste kann als Ganzes ausgeblendet werden, und einige
Misch-Einstellungen wirken auf die Liste, nicht auf ihre Kinder.

## 3. Die drei Sorten Knoten

Es gibt 54 Typen. Man kommt weit, wenn man sie in drei Sorten einteilt:

### Zeichner — machen etwas Neues

`superScope` · `simpleScope` · `oscRing` · `oscStar` · `starfield` ·
`dotGrid` · `rotatingStars` · `metaballs3d` · `tentacles3d` · `flame` ·
`fractal2D` · `fractal3D` · `kleinian` · `strangeAttractor` · `lyapunov` ·
`reactionDiffusion`

Der wichtigste ist **`superScope`**: er zeichnet eine Linie oder Punktwolke aus
einer **Formel**. Fast die gesamte AVS-Ästhetik — Wellen, Spiralen, Lissajous,
tanzende Fäden — kommt aus diesem einen Knoten.

### Verwandler — nehmen das Bild und verbiegen es

`movement` · `dynamicMovement` · `meshWarp` · `domainWarp` · `mirror` ·
`mosaic` · `rotoBlitter` · `interleave` · `bump` · `waterBump` ·
`dynamicDistanceModifier` · `dynamicShift` · `fractalZoomer`

**`movement`** ist der Arbeitspferd-Knoten: Zoom, Drehung, Tunnel, Sog. Er
verschiebt Bildpunkte nach einer Formel. Ein einziger Movement über einer
stehenden Figur macht daraus einen Flug.

### Färber und Filter — verändern Farbe oder Schärfe

`blur` · `bloom` · `brightness` · `colorMap` · `colorModifier` · `colorfade` ·
`colorClip` · `channelShift` · `uniqueTone` · `grain` · `convolution` ·
`fadeout` · `addBorders` · `multiFilter` · `pixelFilter`

**`colorMap`** färbt nach Helligkeit über einen Verlauf — der schnellste Weg,
einem fahlen Preset Charakter zu geben. **`blur`** hat einen Nachzieh-Modus und
gehört damit halb zu §4.

Dazu kommen **Steuerknoten**, die nichts zeichnen: `clear`, `onBeatClear`,
`customBpm`, `bufferSave`, `blitterFeedback`.

## 4. Rückkopplung — woher die Tiefe kommt

Ein Preset, das jeden Frame bei Schwarz beginnt, sieht flach aus. Interessant
wird es, wenn ein Frame vom vorherigen erbt.

Drei Wege dahin:

**Nicht löschen.** Wenn kein `clear` in der Kette steht, bleibt das Bild des
letzten Frames stehen und alles Neue wird daraufgemalt. Das allein erzeugt
schon Spuren.

**Verblassen statt löschen.** `fadeout` oder ein `blur` im Nachzieh-Modus
dunkeln das Alte ab, statt es zu entfernen. Ergebnis: ein Schweif, der
ausklingt. Das ist der Klassiker.

**Puffer und Rückspiel.** `bufferSave` legt das Bild in einen von mehreren
Puffern; `blitterFeedback` holt es verzoomt zurück. Damit baut man Tunnel,
Spiegelkaskaden und Endlos-Zooms.

> **Die typische Anfänger-Kette:** ein Zeichner, darunter ein `fadeout`,
> darunter ein `movement` mit leichtem Zoom. Drei Knoten — und es sieht
> sofort nach etwas aus.

Beim **Preset-Wechsel** stellt sich die Frage, was mit dem geerbten Bild
passiert. LumiViz kann es behalten (Original-Verhalten von MilkDrop), löschen
oder überblenden; einstellbar je Knoten und als App-Vorgabe. Einzelheiten im
Benutzerhandbuch §11.

## 5. Audio hereinholen

Ohne Audiobezug ist es eine Animation, kein Visualizer. Es gibt drei Zugänge:

**Fertige Audio-Knoten.** Die Scope-Typen (`superScope`, `simpleScope`,
`oscRing`, `oscStar`, `timescope`) zeichnen die Wellenform oder das Spektrum
von sich aus.

**Beat-Ereignisse.** `onBeatClear` löscht im Takt; viele Knoten haben eigene
`onBeat`-Felder, die auf einen erkannten Schlag hin einen anderen Wert nehmen.
`customBpm` erzwingt einen festen Takt, wenn die Erkennung nicht passt.

**Formeln.** In jedem Skriptfeld stehen die Audiowerte direkt zur Verfügung:
`bass`, `mid`, `treb` als Bandenergien, `getosc()` und `getspec()` für einzelne
Stellen von Wellenform und Spektrum. Damit hängst du **jeden** Parameter ans
Audio, nicht nur die vorgesehenen.

## 6. Formeln statt fester Werte

Der Punkt, an dem aus Basteln Gestaltung wird.

Viele Knoten haben Skriptfelder in vier Etappen:

| Feld | läuft |
|---|---|
| `init` | einmal beim Laden |
| `frame` | einmal je Bild |
| `beat` | bei jedem erkannten Schlag |
| `point` | je gezeichnetem Punkt / Gitterknoten |

Geschrieben wird in **EEL-Schreibweise** (die Sprache der AVS-Presets), die
LumiViz beim Laden nach Lua übersetzt. Wer Lua kann, schreibt direkt Lua.

Ein `superScope`, der einen atmenden Kreis zeichnet:

```
init:   n = 300;
frame:  t = t + 0.01;
point:  d = 0.4 + 0.1 * bass;
        x = cos(i * 6.283 + t) * d;
        y = sin(i * 6.283 + t) * d;
```

`i` läuft dabei von 0 bis 1 über alle Punkte. `bass` kommt aus der Musik — der
Kreis atmet im Takt.

Auch **normale Parameter** lassen sich per Formel steuern, nicht nur die
Skript-Knoten; im Benutzerhandbuch heißt der Abschnitt „Werte per Formel
steuern". Ein Zoomfaktor als `1.01 + 0.03 * bass` macht aus einem gleichmäßigen
Zoom einen, der auf Bässe schiebt.

## 7. Ein Preset von Grund auf

**File → New Effect Chain** (`Ctrl+Shift+N`), dann in dieser Reihenfolge:

1. **Einen Zeichner setzen.** `superScope` mit einer Voreinstellung aus der
   Preset-Zeile im Eigenschafts-Editor — dort liegen die Figuren (Spiral,
   Butterfly, Hypocycloid …).
2. **Nachziehen einbauen.** `fadeout` darunter. Jetzt bleibt eine Spur.
3. **Bewegung geben.** `movement` darunter, leichter Zoom. Aus der Spur wird
   ein Sog.
4. **Farbe geben.** `colorMap` ganz unten, ein Verlauf, der dir gefällt.
5. **Ans Audio hängen.** Im Zeichner einen Wert auf `bass` oder `treb` legen.
6. **Reihenfolge probieren.** `movement` mal über den `fadeout` ziehen. Anderes
   Bild — welches gefällt dir?

Mehr braucht das erste eigene Preset nicht. Alles Weitere entsteht daraus.

## 8. Speichern, ordnen, weitergeben

**File → Save Effect Chain** schreibt eine `.lvfx`. Das Format ist JSON und
enthält die ganze Kette samt Formeln — eine Datei, kein Ordner.

**Voreinstellungen je Knoten** sind das kleinere Werkzeug daneben: Im
Eigenschafts-Editor speichert **Save as…** einen benannten Parametersatz für
diesen einen Knotentyp, und du wählst per Häkchen, **welche Felder** hinein
sollen. So legst du dir „nur die Formeln, nicht die Farbe" ab und setzt sie in
jedes künftige Preset ein. Sie landen im Benutzerdaten-Ordner
(Einstellungen → **Benutzerdaten-Ordner öffnen**).

**Weitergeben:** Eine `.lvfx` läuft überall — außer sie verweist auf Dateien
(Bilder für `picture`/`texer`, Videos). Solche Verweise sollten **relativ**
sein, sonst funktioniert das Preset nur auf deinem Rechner.

## 9. Fremde Presets als Steinbruch

Der schnellste Weg zu eigenem Material führt über fremdes.

- **`.avs` importieren** und die Kette ansehen: das ist die AVS-Denkweise in
  LumiViz-Knoten übersetzt, und man lernt daran mehr als aus jeder Beschreibung.
- **Einzelne Zweige herausklonen** (⧉) und in ein eigenes Preset ziehen.
- **`.milk` importieren**, wenn du an Warp-Feldern und Shader-Denken
  interessiert bist — MilkDrop arbeitet ganz anders als AVS, gitterbasiert.

Ein Hinweis zum Import: Beim AVS-Import legt LumiViz einen **Render-Scale**-
Knoten an, weil klassische AVS-Presets mit festen Pixelgrößen rechnen. Den
Divisor stellst du unter Einstellungen → Panels ein. Wenn ein importiertes
Preset seltsam grob oder streifig aussieht, ist das die erste Stellschraube.

⚖️ **Rechtliches:** Presets sind Werke ihrer Autoren. Fremde Presets zum Lernen
und Umbauen zu nutzen ist eine Sache — sie weiterzugeben eine andere. Wenn du
ein Preset veröffentlichst, das erkennbar auf einem fremden aufbaut, nenne die
Quelle. In der AVS-Szene ist das seit jeher üblich; viele Presets tragen die
Danksagung im Kommentarfeld.

## 10. Wenn nichts zu sehen ist

| Symptom | erste Vermutung |
|---|---|
| komplett schwarz | Kein Zeichner in der Kette — oder ein `clear` steht **unter** allem und löscht zum Schluss |
| Bild steht still | Kein Audio läuft, oder kein Wert hängt am Audio |
| alles weiß / überstrahlt | Rückkopplung ohne Dämpfung — ein `fadeout` fehlt, oder der Zoom im `movement` liegt unter 1,0 und staut auf |
| grob und streifig nach AVS-Import | Render-Scale-Divisor, siehe [§9](#9-fremde-presets-als-steinbruch) |
| ein Effekt wirkt nicht | Auge-Symbol prüfen, dann die **Reihenfolge** — er wird vielleicht vom nächsten Knoten überschrieben |
| Formel tut nichts | Läuft sie in der richtigen Etappe? Ein Wert, der je Punkt gebraucht wird, gehört nach `point`, nicht nach `frame` |

Beim Ausblenden mit dem Auge-Symbol findet man die meisten dieser Fälle in
unter einer Minute. Es ist das schärfste Werkzeug im Editor.

---

## Weiter

- **Alle Parameter je Knoten:** `docs/visuals/Parameter_Reference.md`
- **Eigene Shader schreiben:** die Tutorial-Serie in `docs/tutorials/`
- **Wie die Kette intern funktioniert:** `docs/visuals/Visualizer_Architecture.md`

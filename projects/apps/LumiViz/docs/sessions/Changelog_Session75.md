# Changelog Session 75 (2026-08-10)

> **MilkDrop steht wieder richtig herum.** Aus zwei Kalibrier-Aufträgen wurde
> die Korrektur eines Fehlers, den Patrik seit S73 im Bild gesehen hatte: das
> Herz in `Dancing Hearts` stand auf dem Kopf. Dazu das erste ehrliche
> MilkDrop-Vergleichswerkzeug (Aufgabe 1 des Kalibrier-Plans).
> Version **0.4.0**.

## Teil 1 — Die Y-Achse war invertiert

Shapes und Wellenformen wurden spiegelverkehrt gezeichnet. Betroffen war jedes
Preset mit einer Form, die nicht zufällig y-symmetrisch ist — die anderen 14 der
19 mitgelieferten konnten den Fehler schlicht nicht zeigen. Genau deshalb hatte
S73 daraus „nur `nWaveMode=7` ist betroffen" geschlossen.

**Die Ursache liegt nicht in den Formeln.** Zeile für Zeile gegen
`milkdropfs.cpp` gelegt, stimmen alle drei Stellen wörtlich mit dem Original —
inklusive dessen Sonderfall („`wave_y` … it was backwards (top==1) … so we keep
it that way"). Sie liegt im **Raum**, in den sie zeichnen: unsere
Feedback-Textur hält das Bild **kopfüber**. Der Warp-Mesh legt Gitterzeile 0
(im Original oben) bewusst auf NDC −1, `compositeToScreen` dreht beim
Präsentieren zurück. Wer dort die Referenzformel übernimmt, zeichnet gespiegelt.

Belegt mit einer Sonde aus `Blank.milk`, deren Comp-Shader nur
`ret = float3(uv.y, 0, 0)` rechnet:

| | Rot oben | Rot unten |
|---|---|---|
| Referenz | 0,125 | 0,875 |
| LumiViz (vorher) | 0,875 | 0,125 |

Korrigiert an vier Stellen — Custom-Comp-Quad, Shape-Mittelpunkt, Shape-Ring,
Custom-Wave-Punkte — plus **einer** zentralen Spiegelung für **alle**
Wellenform-Modi (nach dem Modus-Block, vor dem Glätten). Das hält die
Modus-Formeln deckungsgleich zum Original und lässt `wavePosY` unangetastet:
Modus 7 nutzt es nicht additiv, sondern in der Stereo-Trennung
`sep = (wavePosY*0,5+0,5)²`.

**Gegenprobe über alle 19 (f120, saatlos): 7 besser, 0 schlechter.**

| Preset | vorher | nachher |
|---|---|---|
| Dancing Hearts | 0,376 | **0,230** |
| Dancing Hearts (2nd edit) | 0,329 | **0,231** |
| Neomtwister | 0,359 | **0,326** |
| Twisted / Twisted b | 0,288 | **0,251** |
| Lasershow | 0,081 | **0,056** |

Y-Sonde jetzt **MAE 0,000**, und die Gegenprobe „gespiegelt passt besser" ist
umgeschlagen (0,374 statt 0,225). Die verbleibenden 0,230 bei `Dancing Hearts`
sind der **Farbbefund** (bei uns weiß/rosa, im Original gelb/orange) — ein
eigener Punkt.

## Teil 2 — Ein ehrliches MilkDrop-Vergleichswerkzeug

**NEU `asset/calibration/milkdrop/compare_milkref.py`.** Ein Prozess je Preset,
Frame-Marke **und** Startwert; die Referenz-Ordnerstruktur baut es selbst auf;
Montagen + `report.md`; Urteile `OK / PRUEFEN / BEFUND / REF-STUMM /
BEIDE-STUMM / STARTABHAENGIG`.

Warum das nötig war, zeigt ein Zweizeilen-Versuch: ein Ordner mit
`A_Blank.milk` + `B_Starfield.milk`, im Stapel gerendert, meldet für **beide**
denselben Mittelwert `(0.098, 0.041, 0.147)` — das „Starfield-Bild" war das
geerbte Blank-Bild. Im Stapellauf zeigt jedes Preset etwas, aber teils das des
Vorgängers. Deshalb wirkte `MilkdropRef` bis S74 unauffällig.

**Drei Fallen, die das Werkzeug beim Bau selbst aufgedeckt hat:**

- Der Kern rendert seinen **Preset-Browser** ins Bild, wenn er im UI-Modus
  startet — bei `Starfield (Supernova-Edit)` wurde daraus aus einer schwarzen
  Referenz ein Messwert von 0,031, also aus einem Befund ein **Schein-OK**.
- Ein Urteil an einer **Schwellenkante** ist keines: `Starfield` stand mit MAE
  0,002 als BEFUND in der Liste, weil eine Seite knapp unter und die andere
  knapp über der „tot"-Schwelle lag.
- Die **Warmlaufphase** der Referenz ist keine Messung. Sie wird jetzt
  verworfen — aber nur, wenn eine spätere Marke zeigt, dass die Referenz danach
  zeichnet.

## Teil 3 — Kaltstart-Saat: geteilt, mit Seed-Reihe, korrigierte Annahme

**NEU `include/visualizers/KaltstartSaat.hpp`** (Vorbild `SynthAudio.hpp`): App
und `MilkdropRef` teilen die Formel, damit beide Seiten im selben Startzustand
beginnen. `MilkdropRef` bekommt `--saat <seed>` (D3D9-Einspeisung über lockbare
Offscreen-Surface + `StretchRect`).

**Korrektur einer S63-Annahme:** das Original startet **nicht** mit
undefiniertem VRAM. `milkdropfs.cpp` nullt den Feedback-Puffer im ersten Frame
ausdrücklich („on first frame, clear OLD VS"). Presets zünden dort beim
**Preset-Wechsel**, weil der Puffer stehen bleibt. Unsere Rausch-Saat ist damit
eine bewusste **Abweichung**, kein Nachbau — und sie liegt jetzt ebenfalls
hinter dem ersten Bild.

**Über mehrere Startwerte messen** (Vorgabe Patrik) beantwortet eine Frage, die
ein einzelner nicht kann: **hängt das Preset überhaupt am Startzustand?** Der
erste vollständige Lauf stuft **6 von 19** als `STARTABHAENGIG` ein — ihr Bild
reagiert stärker auf den Startwert als auf den Renderer. Damit ist die
S67-Dunkelklasse erstmals gemessen statt vermutet.

> ⚠️ **Falle:** `MilkdropStandalone` setzt `LUMIVIZ_MILKDROP_NOSEED` **selbst**,
> solange `--seed` fehlt. Eine Env-Variable allein bleibt wirkungslos — drei
> „verschiedene" Seeds waren bit-identisch, und eine erste Messung meldete
> fälschlich „die Saat ändert nichts".

## Teil 4 — `07_movin wall` umgedeutet

Die S74-Aufgabe „`g_blendtable` bit-treu nachbilden" hätte an **totem Code**
gearbeitet: der `NO_MMX`-Zweig von `BLEND4` lässt sich nicht einmal übersetzen
(`inblendval` undeklariert, `BLEND_ADJ` dort nicht definiert) — mit einem
`-DNO_MMX`-Build gegengeprüft. AVS führt nur den MMX-Zweig aus, den unser
Shader seit S46 bit-genau nachrechnet.

Das Movement ist entlastet (solo in Rückkopplung **0,001**). Es ist ein
**Verstärker**: sein Warp hebt den 1-Pixel-Kantenversatz eines SuperScope von
0,009 auf **0,109** — über den Wert des Gesamt-Presets. Der eigentliche Befund
ist die SuperScope-Rasterung.

## Teil 5 — Presets und Version

**NEU `Fuck me Im Famous - revisited`** (3, Eigenwerk): In The Spotlight V1/V2,
Rock The House 2024. Mitgeliefert jetzt **32** Presets.

Zwei Mash-Up-Sätze (93 + 243) bleiben für 0.4.0 **bewusst draußen**:
Bearbeitungen Dritter auf Basis eigener Presets, die Erlaubnis des Erstellers
steht aus. Vorgesehen für **0.5.0**.

**Version 0.3.0 → 0.4.0** (`Solution.json`, SSOT).

## Doku

`kalibrierung/INDEX.md` **1.2.0** — Regeln **9–12** (toter Original-Zweig ·
Verstärker im Pfad · Startzustand · Warmlaufphase), MilkDrop-Harness §2.2,
Referenz-Vorbehalte §2.3. Dazu `Offene_Punkte.md`, `Kalibrier_Plan_…` (Aufgabe
1) und `asset/presets/README.md`.

**Tests:** `LumiViz.UnitTests` grün.

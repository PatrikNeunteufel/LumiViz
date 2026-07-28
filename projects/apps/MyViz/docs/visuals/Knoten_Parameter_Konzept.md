# MyViz — Knoten-Parameter, Voreinstellungen und dynamische Felder (Konzept)

> **Version:** 1.16.0
> **Datum:** 2026-07-28
> **Typ:** Konzept — Etappen 1–6 umgesetzt (S53), **Etappe 7 = Strang E umgesetzt
> (S54)**, 8 (Tooltips) und 9 (Basis-Voreinstellungen) offen
> **Status:** Aktiv — Entscheide §8.1–§8.4 gefallen; Strang E hat beim ersten
> Einsatz zwei Befunde an Strang D geliefert (§9.1)
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Import_Modul_Abdeckung.md](Import_Modul_Abdeckung.md) (Knoten-Bestand) ·
> [AVS_Kalibrier_Methodik.md](AVS_Kalibrier_Methodik.md) (der Treue-Vertrag, §2) ·
> [Skript_Variablen_Konzept.md](Skript_Variablen_Konzept.md) (EEL-Träger, §6) ·
> [Vereinheitlichung_Konzept.md](Vereinheitlichung_Konzept.md) (Fernziel Skript-Set) ·
> `MultiEffectPanel` · `ChainSerializer` · `MultiEffectVisualizer`
> **Sprache:** Deutsch

---

## 1. Warum

Vorgabe Patrik (Session 53): Die Effekt-Knoten sollen **einstellbar** sein — nicht nur
dort, wo AVS zufällig einen Konfigurationswert vorsah. Ein Knoten wie `Rotating Stars`
hat heute **null** verstellbare Zahlen; seine fünf Zacken, zwei Sterne und sein
Bahnradius stehen als Literale im Renderer. Dazu sollen **alle** Knoten
Voreinstellungen bekommen, auch die längst vollständigen (`Movement`,
`Dynamic Movement`, `Effect List`).

Die Bestandsaufnahme (S53, skriptgestützt über alle Knotentypen) trennt das in
**drei Lücken**, die verschieden teuer und verschieden riskant sind:

| Lücke | Was fehlt | Aufwand | Risiko |
|---|---|---|---|
| **A** Voreinstellungen | jeder Knoten soll benannte Parametersätze laden/speichern können | **einmalig**, generisch | keins |
| **B** Editor-Felder | Felder, die es im Struct **schon gibt**, haben keine Zeile im Panel | ~20 Stellen | keins |
| **C** Neue Regler | Werte, die im Renderer **festgenagelt** sind, sollen Parameter werden | pro Knoten | **hoch** (§2) |
| **D** Dynamische Felder | Parameter per EEL je Frame/Beat rechnen (wie SuperScope) | pro Knoten | mittel |

## 2. Der Default-Vertrag — die Regel über allem

Seit Session 44 läuft die **Kalibrier-Runde**: Der Wert eines Knotens misst sich am
Referenz-Renderer `AvsRef` (Modul-Matrix 36/41, Sonden 78/80 — gemessen S53/S54;
die früher notierten 37/41 und 79/80 waren zu optimistisch). Jeder
neue Parameter ist eine Gelegenheit, diese Zahlen unbemerkt zu verderben.

> **Vertrag:** Ein neuer Parameter **muss** bei seinem Default-Wert exakt das heutige
> Bild erzeugen — pixelgleich, nicht „ungefähr". Und der AVS-Import **darf** ihn nicht
> setzen: ein importiertes Preset trägt diese Zahl nicht, also bleibt sie auf Default.

Daraus folgt die Einteilung, die die ganze Umsetzung steuert:

**Klasse A — AVS-Portierungen.** Der Renderer bildet Referenz-Quelltext zeilengenau
nach; die Konstanten *sind* die Referenz. Geprüft und belegt: `Dot Plane`
(`r_dotpln.cpp`, 64×64-Gitter, Physik `0.15`, Translation `0/-20/400`), `Bass Spin`
(`r_bspin.cpp`, 44 Bänder, Glättung `0.7/0.3`, Speichenschritt π/6), `Moving Particle`
(`r_parts.cpp`, Federkraft `0.004`, Dämpfung `0.991`), `Scatter`, `Water`,
`Simple Scope`, `Interferences`.
→ Regler hier sind **optionale Abweichungen**. Default = Original, und die Sonde des
Knotens muss danach unverändert grün sein. Wo eine Konstante zugleich die Kalibrierung
trägt, ist der Regler ein Risiko ohne Gegenwert — im Zweifel weglassen.

**Klasse B — unsere eigenen Zahlen.** Nachbauten und Eigenentwicklungen, deren
Geometrie nie aus einem Preset kam: `Rotating Stars`, `Osc Star`, `Osc Ring`,
`Dot Fountain` (Herkunft **noch zu prüfen**, s. §8.3), `FyrewurX`, `Metaballs 3D`,
`Tentacles 3D`, alle Fraktal- und 3D-Knoten.
→ Hier ist alles frei. Der heutige Wert wird Default, mehr ist nicht zu beachten.

## 3. Strang A — Voreinstellungen je Knoten (generisch)

Der Hebel: `ChainSerializer` kann **jetzt schon** einen einzelnen Knoten als JSON
schreiben und lesen (`nodeToJson` / `nodeFromJson`), und `effectTypeKey(params)` liefert
den Typ-Schlüssel. Eine Voreinstellung ist damit nichts weiter als *die `params` eines
Knotens unter einem Namen* — kein knotenspezifischer Code, keine 81 Bibliotheken.

**Oberfläche:** eine Zeile ganz oben im Property-Editor, über `Name`:

```
Voreinstellung  [ (unverändert) ▾ ]  [ Speichern unter… ]  [ 🗑 ]
```

Auswählen ersetzt die `params` des Knotens (eine `mutate`-Operation, damit
undo-fähig über den CommandBus); `Speichern unter…` legt die aktuellen `params` als
Datei ab. Der Typ bleibt immer erhalten — es lassen sich nur Voreinstellungen
*desselben* Knotentyps laden.

**Ablage:**

| Ort | Zweck |
|---|---|
| `asset/nodepresets/<typkey>/<name>.json` | mitgeliefert, im Repo |
| `<Benutzerdaten>/nodepresets/<typkey>/<name>.json` | selbst gespeichert |

Bei Namensgleichheit gewinnt der Benutzer-Ordner. Der Typ-Schlüssel ist der bestehende
`effectTypeKey()` (`superScope`, `movement`, `metaballs3d` …) — damit gilt der
Mechanismus automatisch auch für jeden künftigen Knoten.

**Was in eine Voreinstellung gehört (Entscheid Patrik, §8.2):** die `params` des
Knotens — **einschließlich der EEL-Formeln**, denn die liegen als `initCode`,
`frameCode`, `beatCode`, `pointCode` in eben diesen `params`. Eine Voreinstellung für
`SuperScope` oder `Dynamic Movement` trägt also ihre Formeln mit, genau wie das
Figur-Dropdown sie heute einträgt. **Nicht** enthalten: `displayName`, `children`,
`nodeId`.

**Merge statt Ersatz (Nachtrag Patrik, S53).** Eine Datei muss nicht alle Felder
tragen: beim Laden überschreibt sie genau die, die sie enthält, der Rest bleibt
stehen. Beim **Speichern** wird deshalb je Feld ein Häkchen angeboten (Liste
generisch aus `nodeToJson`, gilt für jeden Typ) — abgewählt heißt „nicht in die
Datei" und damit „beim Laden unangetastet". Das ist die Verallgemeinerung dessen,
was das Figur-Dropdown konnte: eine Figur setzt die Formeln und lässt die Farbe in
Ruhe. `type` bleibt immer in der Datei, daran hängt der Typwächter.

**Folge für Bestehendes (Entscheid Patrik, §8.1):** Die SuperScope-Figur-Bibliothek
(`SuperscopePreset`-`enum` mit hartkodierten EEL-Texten in `SuperscopeModule`) **soll
in Dateien aufgehen** — `asset/nodepresets/superScope/spiral.json` statt Hardcode.
Damit gibt es *einen* Preset-Begriff, und Figuren lassen sich ohne Neubau ergänzen.
Das Figur-Dropdown ist das Vorbild für die Zeile aus §3, nicht ihr Konkurrent: es
verschwindet und wird zur allgemeinen Voreinstellungs-Zeile.
Umsetzung als **eigene Etappe 1b** — das Modul hat eine Nicht-Panel-API
(`SuperscopeModule::setPreset`), die weiter funktionieren muss.

## 4. Strang B — fehlende Editor-Felder

Vollständig ermittelt (Struct-Felder gegen Panel-Blöcke, S53). Kein Risiko, reine
Fleißarbeit — der Wert existiert bereits und wird bereits serialisiert.

| Knoten | Feld | Stand |
|---|---|---|
| `Convolution` | `kernel` | ✅ S53 — **7×7**-Gitter (49 Werte, nicht 5×5 wie in v1.0.0 behauptet), Mitte hervorgehoben, Knopf „Identity" |
| `ColorMap` | `stopPos`, `stopColor` | ✅ S53 — Stützstellenliste (Position 0..255 + Farbe, `+`/`−`); die Reihenfolge ist egal, `buildColorMapLut` sortiert und klemmt selbst |
| `Picture`, `Picture II`, `Texer`, `Texer II` | `filename`, `imageData` | ✅ S53 — Zeile „Image" mit `Choose…`/`Clear` (bettet die Datei base64 ein) + Bilder-Suchordner in den Einstellungen; der S50-Punkt ist damit erledigt |
| `Texer II` | `resizing`, `wrapAround`, `beatCode` | ✅ S53 |
| `Triangle` | `beatCode` | ✅ S53 (die anderen drei Slots waren da) |
| `Movement` | `builtinRemap` | ✅ S53 — als Auswahl `Off / Slight fuzzify / Blocky partial out`; die echten Werte sind 0 · 1 · **7** |
| `Custom BPM` | `skipFirst` | ✅ S53 |
| `Colorfade` | `onBeatFrames` | ✅ S53 |
| `Blur` | `roundUp` | ✅ S53 |
| `Simple Scope`, `Rotating Stars`, `Osc Star`, `Osc Ring` | `colors` | ✅ S53 — Farbtafel über `addColorTable` |
| `Dot Grid` | `colors` | ✅ S53 — zeigte nur Eintrag 0 |

Bewusst **ohne** Editor bleiben: `Invert`, `Normalise`, `Water`, `Scatter` (haben keine
Felder), `ImportNotes.text`, `MilkdropNode.embeddedImages`, `HostGroup.sourceFile`
(Anzeige, kein Eingabewert).

## 5. Strang C — neue Regler

Vorschlag je Knoten. **Default ist immer der heutige Literalwert** (§2). „Klasse" nach
§2; bei A ist der Regler eine bewusste Abweichung von der Referenz.

| Knoten | Kl. | heute festgenagelt | soll Parameter werden |
|---|---|---|---|
| `Rotating Stars` | B | ~~5 Zacken · 2 Sterne · Rotation `0.05` · Orbit `0.5` · Radius `(peak*0.5+0.12)*0.5` · Band 3–14~~ | ✅ **S53:** `points`, `skip`, `stars`, `rotSpeed`, `orbit`, `baseRadius`, `audioGain`, `bandLo/Hi` — der Knoten hatte vorher **null** Parameter |
| `Osc Star` | B | ~~5 Speichen · Rotationsfaktor `0.02` · Amplitude `len*0.5`~~ | ✅ **S53:** `spokes`, `rotScale`, `amplitude` (freie Position: offen) |
| `Osc Ring` | B | ~~80 Punkte · Radius `0.1 + \|v\|*0.9`~~ | ✅ **S53:** `segments`, `baseScale`, `audioScale` (freie Position: offen) |
| `Dot Fountain` | B | 400 Partikel · Gravitation `0.0016` · Spreizung `0.012` · Punktgröße `2.0` · Kamera `2.0`/`1.6` | **zurückgestellt** — der Knoten ist nicht portiert (§8.3); erst portieren, dann Regler |
| `Metaballs 3D` | B | ~~Bahnweite · Perspektive `z+1.2` · Phasenversatz `1.7`~~ | ✅ **S53:** `spread`, `depth`, `phase` (die drei Frequenzen bleiben fest — sie sind teilerfremd gewählt, damit sich die Kugeln nie periodisch treffen) |
| `Tentacles 3D` | B | ~~Schwingung `sin(t + u*3.1)*0.9` · Verjüngung~~ | ✅ **S53:** `sway`, `waves`, `taper` (0 = gleich dick) |
| `FyrewurX` | B | ~~Punktgröße `2` · Farbstreuung `0.6` · Burst-Streuung~~ | ✅ **S53:** `dotSize`, `hueDrift`, `burstSpread` |
| `Triangle` | B | ~~keine Zahlen (nur EEL)~~ | ✅ **S53:** `filled` (Vorgabe **an** = Referenz, S51) + `lineWidth` für den Drahtgitter-Modus |
| `Dot Plane` | **A** | 64×64-Gitter · Kamera `0/-20/400` · Physik `0.15` | ✅ **S53:** `camDistance`, `settle` — das Gitter bleibt fest (Struktur, nicht Parameter) |
| `Bass Spin` | **A** | 44 Bänder · Glättung `0.7/0.3` · π/6 | ✅ **S53:** `smoothing`, `spinStep` (Vorgabe = `3.14159f/6.0f`, **nicht** π/6) |
| `Moving Particle` | **A** | Federkraft `0.004` · Dämpfung `0.991` · Startpunkt | ✅ **S53:** `spring`, `damping` |
| `Scatter`, `Water`, `Simple Scope`, `Interferences` | **A** | Referenz-Arithmetik | **nichts** — Kalibrierung ohne Gegenwert gefährdet |

## 6. Strang D — dynamische (EEL) Felder

22 der 81 Knoten haben bereits Skript-Slots (`SuperScope`, `Dynamic Movement`, `Bump`,
`Color Modifier`, `Triangle`, `Texer II`, alle Fraktale, `Effect List` …). Der Baukasten
steht seit S33: `ScriptSlotHost` (EEL-Quartett init/frame/beat/point) auf einem
gemeinsamen `ScriptContext` (`reg00..reg99` global, `gmegabuf` geteilt).

Für die übrigen Knoten heißt „dynamisch" konkret: einen `ScriptSlotHost` anhängen,
die Parameter als beschreibbare Variablen registrieren und nach dem Frame-Slot
zurücklesen. Zwei Fallen sind aus S51/S52 bekannt und gelten hier unverändert:

- **Neutrale Vorbelegungen gehören VOR den Frame-Slot**, nicht je Punkt (zweimal
  hineingelaufen: `sizex`/`sizey` in S51, die Farbe in S52).
- **Ein Skript-Träger braucht ALLE Variablen, die seine Slots lesen** — sonst schweigt
  der Fehler und das Bild ist nur falsch.

Kosten: ein Skript-Durchlauf je Frame und Knoten. Deshalb **opt-in** — ein leeres
Skriptfeld darf keinen Transpiler- und keinen Lua-Aufruf auslösen.

Reihenfolge-Vorschlag: die Knoten aus §5 Klasse B zuerst (dort ist die Zahl ohnehin
unsere), Klasse A zuletzt oder gar nicht.

**Umgesetzt (S53) — das Fundament.** `MultiEffectVisualizer::runParamScript(rt,
prefix, init, frame, beat, vars)` erledigt für **jeden** Knoten dasselbe: Host bei
Änderung neu bauen, die aktuellen Reglerwerte als Startwerte setzen (Merkregel S52:
Vorbelegung gehört VOR den Frame-Slot), `b`/`w`/`h` + Audio-Satz dazu, Init (nur
beim ersten Lauf) → Frame → Beat, danach die Werte zurücklesen. `vars` ist eine
Liste `{EEL-Name, double*}` — pro Knoten fünf Zeilen.

Zwei Verträge:

- **Opt-in.** Sind alle drei Quellen leer, kehrt die Funktion sofort zurück: kein
  Host, kein Transpiler, kein Lua-Aufruf.
- **Frame-Kopie.** Das Skript rechnet auf lokalen `double`s, nicht auf den `params`
  des Knotens. Sonst wäre der Regler nach einem Frame dauerhaft verstellt und ein
  `size=size+1` würde davonlaufen.

Angeschlossen (S53) — alle Knoten aus Strang C Klasse B:

| Knoten | schreibbare Variablen |
|---|---|
| `Rotating Stars` | `points`, `skip`, `stars`, `rotspeed`, `orbit`, `baseradius`, `audiogain` |
| `Osc Star` | `size`, `rot`, `spokes`, `rotscale`, `amplitude` |
| `Osc Ring` | `size`, `segments`, `basescale`, `audioscale` |
| `Metaballs 3D` | `count`, `radius`, `speed`, `threshold`, `spread`, `depth`, `phase` |
| `Tentacles 3D` | `count`, `segments`, `length`, `thickness`, `speed`, `sway`, `waves`, `taper` |
| `FyrewurX` | `sparks`, `speed`, `gravity`, `life`, `dotsize`, `huedrift`, `burstspread` |

| `Dot Plane` **(A)** | `rotvel`, `angle`, `camdistance`, `settle` |
| `Bass Spin` **(A)** | `mode`, `smoothing`, `spinstep` |
| `Moving Particle` **(A)** | `size`, `size2`, `maxdistance`, `spring`, `damping` |
| `Starfield` | `maxstars`, `warpspeed`, `beatspeed`, `durationframes` |
| `Timescope` | `bands`, `channel` |
| `Dot Grid` | `spacing`, `xmove`, `ymove` |
| `Dot Fountain` | `rotvel`, `angle` |

Dazu überall `b` (Beat), `w`/`h` (Bildmaße) und der Audio-Satz aus `feedAudio`.

**Klasse A und Skripte — die Lücke in der ⚠-Kennzeichnung (S53).** Das Zeichen an
den Reglern vergleicht den *festen* Wert mit dem Referenzwert. Was ein Frame-Skript
je Bild ausrechnet, sieht es nicht — ein Skript hätte die Referenztreue also
unsichtbar aushebeln können. Deshalb tragen die drei Klasse-A-Knoten über ihren
Skriptfeldern eine **Hinweiszeile**: ohne Skript „dieser Knoten bildet AVS
zeilengenau nach, ein Skript weicht davon ab", mit Skript dieselbe Aussage mit ⚠ und
im Perfekt. Damit ist die Abweichung immer sichtbar, egal woher sie kommt.

**Und die Trans-/Bild-Effekte (S53, Block 1+2):** Blitter Feedback · RotoBlitter ·
Interferences · Water Bump · Mosaic · Grain · Fadeout · AddBorders · Color Clip ·
Bloom · Video Delay · Multi Delay · Clear · Brightness · Simple Scope · Unique Tone ·
Interleave · Picture · Picture II · Fast Brightness · Blur · Mirror · OnBeat Clear ·
Colorfade · Movement · Buffer Save · Custom BPM · Buffer Blend · Convolution ·
Multi Filter · Texer · Color Map · Channel Shift · Color Reduction · Multiplier.

**Damit trägt jeder Knoten mit numerischen Parametern sein Skript-Trio** — 47
Renderer rufen `runParamScript`. Ohne Skript kostet keiner davon Rechenzeit.

**Die Ausnahme: `Movement`.** Es bekam die Felder zunächst auch und hat sie wieder
verloren (Befund Patrik an `movement3b.lvfx`, S53). Zwei Gründe, beide strukturell:
Der Knoten hat mit dem `ScriptGridModule` bereits einen eigenen Skript-Träger, und
ein **zweiter** daneben teilt keine Variablennamen mit ihm — `bt=1` im Init ist im
Point-Code unsichtbar. Vor allem aber cacht `applyMovementTable` über den
**Skripttext**: der Point-Code läuft nur bei Größen- oder Textänderung, ein je
Frame gerechneter Wert könnte das Bild also gar nicht bewegen (so auch AVS, das
`r_trans` deshalb ohne Beat-Code lässt).

> **Regel, die daraus folgt:** Ein Knoten hat **einen** Skript-Träger. Wo schon
> einer existiert, kommen neue Slots dort hinein — nie daneben. Damit teilen alle
> Slots eines Knotens ihre Umgebung, und eigene Variablennamen funktionieren über
> Init → Frame → Beat → Point hinweg. Über Knoten- und Trägergrenzen wandern nur
> `reg00..reg99`, `q1..q64` und `gmegabuf`.

**Warum nur drei Knoten die Hinweiszeile tragen.** Bei `Dot Plane`, `Bass Spin` und
`Moving Particle` sind die *Konstanten des Renderers* die Referenz — ein Skript
verlässt dort die nachgebaute Implementierung. Bei `Blur` oder `Movement` sind die
Parameter dagegen Preset-Werte, die AVS selbst kennt; ein Skript verstellt sie wie
eine Hand am Regler. Das ist normale Bedienung und braucht keine Warnung.

## 7. Etappen und Absicherung

| Etappe | Inhalt | Fertig wenn |
|---|---|---|
| **1** ✅ | Strang A — Voreinstellungs-Zeile + Ablage + 3 Beispiel-Presets (S53) | Laden/Speichern/Löschen für einen Knoten jeder Art (Blatt · Liste · Host-Gruppe) |
| **1b** ✅ | SuperScope-Figuren aus dem `enum` in Dateien (§8.1, S53) | alle Figuren als `asset/nodepresets/superScope/*.json`, Modul-API unverändert |
| **2** ✅ | Strang B — die einfachen Felder aus §4 (S53; alles außer Kernel/ColorMap/Bilder) | Panel zeigt jedes Feld, das der Serializer schreibt |
| **3** ✅ | Strang B — `Convolution`-Kernel + `ColorMap`-Stopps (eigene Widgets, S53) | dito |
| **4** ✅ | Strang C — Klasse B **vollständig** (S53): Rotating Stars · Osc Star · Osc Ring · Metaballs 3D · Tentacles 3D · FyrewurX · Triangle | Sichttest je Knoten |
| **3b** ✅ | Bild-Auswahlfelder + Suchordner (S50-Vorgabe, in S53 vorgezogen) | Bild wählbar, Suchordner greift beim Import |
| **5** ✅ | Strang C — Klasse A mit Kennzeichnung (S53) | Sonden unverändert |
| **6** ✅ | Strang D — dynamische Felder für **jeden Knoten mit numerischen Parametern** (47 Renderer, S53) | Skript verstellt die Regler, leeres Feld kostet nichts |
| **7** | Strang E — **Test-Presets je Modul und Feld** (§9) | jedes Feld hat ein Preset, das seine Wirkung zeigt |
| **8** | Strang F — **Tooltip an jedem Feld** (§10) | kein Feld ohne Erklärung; Skriptfelder nennen ihre Variablen |
| **9** | Strang G — **Basis-Voreinstellungen für alle Module** (§11) | jeder Knotentyp hat mindestens einen brauchbaren Startpunkt |

**Absicherung nach *jeder* Etappe** — nicht erst am Ende:

- `MyViz.UnitTests` grün (Serializer-Roundtrip je neuem Feld **erweitern**, sonst
  bewacht nichts die Persistenz).
- **Modul-Sonden und Matrix unverändert** (Stand S53: 78/80 und 36/41). Eine
  Abweichung nach einer Etappe ist ein Fehler dieser Etappe, kein Zufall — und mit
  `--beat-period` messen, sonst ist keiner der beiden Renderer reproduzierbar (S52).
- Ein neues Feld ohne Eintrag in `nodeToJson`/`nodeFromJson` ist ein stiller
  Datenverlust beim Speichern. Das ist der wahrscheinlichste Fehler in Etappe 4–6.
- **Ein Feld, das nicht wirken KANN, ist schlimmer als ein fehlendes** — es lädt
  ein, Zeit hineinzustecken. Movement hat das gezeigt (§6). Deshalb Etappe 7.

## 8. Offene Entscheide

**8.1 SuperScope-Figuren zu Dateien?** ✅ **Entschieden (Patrik, S53): ja.** Die
Bibliothek geht in `asset/nodepresets/superScope/` auf, das Figur-Dropdown wird zur
allgemeinen Voreinstellungs-Zeile. Eigene Etappe 1b (§3, §7) — die Modul-API
`SuperscopeModule::setPreset` muss weiter funktionieren.

**8.2 Was gehört in eine Voreinstellung?** ✅ **Entschieden (Patrik, S53):** nur die
`params`, **einschließlich der EEL-Formeln** (die stecken ohnehin darin). Keine
`children`, kein `displayName`.

**8.3 Herkunft von `Dot Fountain`.** ✅ **Geklärt (S53) — und der Befund ist größer
als die Frage: der Knoten ist gar nicht portiert.**

Referenz `r_dotfnt.cpp` (`ref/vis_avs/avs/vis_avs/`): ein **30×256-Gitter**
(`NUM_ROT_DIV 30`, `NUM_ROT_HEIGHT 256` = 7680 Punkte) — eine **rotierende Höhenwand**
mit derselben 3D-Matrix wie `Dot Plane` (`matrixTranslate(0, -20, 400)`), Höhe je
Punkt aus dem Spektrum (`dr = t/200`), Fallterm `dh += 0.05f`, Rotation `r += rotvel/5`.

Unser Renderer: **400 freie Partikel** mit Zufallswinkel, Gravitation `0.0016`,
Spreizung `0.012`, eigener Projektion (`wz+2.0`, Zoom `1.6`). Der Header sagt es
selbst — *„Simplified particle model here; projection/physics scale is host tuning"*.
Es ist also keine vereinfachte Portierung, sondern **eine andere Konstruktion**.

**Und die Matrix-Zeile bewacht das nicht.** `19_dot_fountain/01_default` misst
dMean 0,005 (320×240) bzw. **0,002** (740×460) und zählt zu den 37/41 — die Montage
zeigt links einen hohen, geordneten Brunnen über die volle Bildhöhe, in der Mitte
einen flachen diffusen Fleck von etwa einem Fünftel der Fläche, und der 4×-Diff ist
praktisch das Referenzbild. Klassischer Fall der Merkregel **„die Metrik lügt bei
dünnen Inhalten"**: beide Bilder sind überwiegend schwarz, also ist der
Mittelwert-Abstand winzig.

→ Für dieses Konzept: **Klasse B** (die Zahlen sind unsere). Aber die eigentlich
richtige Arbeit ist eine **echte Portierung** nach dem Muster von `Dot Plane` — die
3D-Matrix und die Farbtabellen-Arithmetik liegen dort schon zeilengenau vor. Das ist
ein Kalibrier-Befund, kein Parameter-Thema, und gehört in
[Offene_Punkte.md](../Offene_Punkte.md) §1 statt in Etappe 4.

**8.4 Wie weit bei Klasse A?** ✅ **Entschieden (Patrik, S53): bauen, mit
Kennzeichnung.** Umgesetzt in Etappe 5: je zwei Regler, Default = Originalwert, und
`addRefDouble` hängt ein **⚠** an die Beschriftung, sobald der Wert abweicht (Tooltip
nennt den Referenzwert). Abweichend von §5 bekommt `Dot Plane` **nicht** die
Gitterweite — `NUM_WIDTH = 64` steckt in Stack-Arrays und ist die Struktur der
Portierung, kein Parameter; stattdessen Kameradistanz und Absink-Term.

## 9. Strang E — Test-Presets je Modul und Feld (Vorgabe Patrik, S53)

**Warum.** Nach den Etappen 1–6 hat fast jeder Knoten mehr Felder als je zuvor, und
kein einziges davon ist im Betrieb geprüft. `movement3b.lvfx` hat binnen Minuten
gezeigt, wie das ausgeht: drei Felder, die dastehen und nicht wirken können. Ein
Unit-Test findet so etwas nicht — er prüft Persistenz, nicht Wirkung.

**Ziel:** je Modul **und je Feld** ein Preset, das die Wirkung des Feldes sichtbar
macht. Wirklich jedes Modul, wirklich jedes Feld.

**Aufbau.** Ablage `asset/calibration/fields/<typkey>/<feld>.lvfx`, parallel zu den
bestehenden Modul-Sonden. Je Preset:

- **Ein** Feld weicht vom Default ab, alle anderen stehen auf Vorgabe. Wer zwei
  Dinge gleichzeitig ändert, weiß hinterher nicht, welches gewirkt hat.
- Für **Transformationen** (Movement, Blur, Bump, Water, Mirror …) ein klar
  definiertes **statisches Bild** als Untergrund — dasselbe Vorgehen wie auf der
  MilkDrop-Seite. Ohne festen Untergrund ist nicht unterscheidbar, ob der Effekt
  wirkt oder das Ausgangsbild wandert.
- Für **Renderer** ein leerer Hintergrund und ein deterministisches Testsignal
  (`…\cmake\TestAudio`), damit das Bild reproduzierbar ist.
- Für **Skriptfelder** eine Formel, deren Wirkung man sieht, ohne sie zu messen —
  etwa `size = 8 + bass*20` statt `size = size + 0.001`.

**Urteil.** Zwei Läufe je Preset: einmal auf Default, einmal mit gesetztem Feld.
Unterscheiden sich die Bilder **nicht**, ist das Feld wirkungslos — Befund, kein
Messrauschen.

**Nebenprodukt.** Die Sammlung ist zugleich die Grundlage für Strang G: ein Preset,
das ein Feld gut zeigt, ist oft schon eine brauchbare Voreinstellung.

### 9.1 Umsetzung (Session 54)

Die Sammlung wird **erzeugt**, nicht geschrieben — bei 717 Feldern ist eine
handgepflegte Ablage nicht zu halten. Vier Stufen unter `asset/calibration/fields/`:

| Stufe | Werkzeug | liefert |
|---|---|---|
| Inventar | `test_FieldInventory.cpp` → `inventory.json` | alle Typen und Felder mit Vorgabe, als Golden bewacht |
| Ernte | `harvest_field_docs.py` → `inventory_docs.json` | Wertebereiche (Panel-Setter), Enum-Breite, Beschreibungen, echte Skriptvariablen (`runParamScript`) |
| Erzeugung | `make_field_probes.py` | je Typ `_default.lvfx`, je Feld `<feld>.lvfx` |
| Urteil | `run_field_probes.py` | WIRKT / SCHWACH / STUMM + Montage |

**Der Untergrund** (`lvfx_lib.py`) ist der Nachbau des S50-Referenzbilds aus
`make_module_probes.py` — vier Farbfelder plus Diagonale auf `0x101010`. Kein
zweites Testbild: so lässt sich ein Befund zwischen beiden Sonden-Familien
vergleichen. Zwei Läufe desselben Presets sind **bit-identisch** (nachgemessen),
deshalb braucht das Urteil keine Toleranz — jede Abweichung ≠ 0 kommt vom Feld.
Das deterministische Testsignal liefert der Standalone selbst (Sinus + Beat-Puls),
ein WAV aus `…\cmake\TestAudio` wird dafür nicht gebraucht.

**Drei Tabellen tragen das Wissen, das sich nicht ableiten lässt** — jede mit
Begründung im Skript, weil jede aus einem Fehlversuch entstanden ist:

- `HANDWERK` — Gegenwerte, die keine Regel liefert (ein Movement-Ausdruck, ein
  Bildpfad). Auch Bitfelder: `mirror.mode` 4 → 12 addiert nur Bits und sieht aus
  wie die Vorgabe; der Gegenwert muss die **Achse** wechseln.
- `GRUNDKONFIG` — Felder, die nur in Gesellschaft wirken (`mirror.slower` braucht
  `smooth`, `mosaic.onBeat` braucht ein abweichendes `quality2`). Solche Felder
  bekommen einen **eigenen Vergleichsgrund**, sonst unterscheiden sich die zwei
  Läufe in zwei Dingen statt einem.
- `NICHT_PRUEFBAR` — was das Testsignal grundsätzlich nicht zeigen kann. Bisher
  nur die Timescope-Kanalfelder: der Standalone erzeugt das Spektrum für beide
  Kanäle gleich, links/rechts/Mitte sind dort zwangsläufig identisch. Ein
  „STUMM" wäre hier eine Falschaussage über die App.

**Lauflänge (Vorgabe Patrik, S54).** 181 Frames ≈ drei Sekunden bei 60 fps, mit
`--beat-period 30` also sechs Beats — und der letzte Frame ist selbst einer
(`(181-1) % 30 == 0`). Ein Effekt braucht Zeit zu wirken, ein beat-gebundener
braucht Beats, und seine Wirkung muss im **Schlussbild** stehen. Mit den
anfänglichen 40 Frames war `onBeatClear` sechsmal stumm: der Knoten löschte
korrekt, nur zeichnete der Untergrund im Folgeframe wieder darüber.

**Zwei Beschleunigungen wurden gemessen und verworfen** — beide bleiben als
Schalter erhalten, damit niemand den Weg ohne die Zahlen wiederholt:

| Idee | gemessen (97 Sonden) |
|---|---|
| 4 Prozesse gleichzeitig (`--jobs`) | **61 s** gegen 31 s — die GPU ist der Engpass, nicht die Reihenfolge; dazu wich eine Zeile in der 4. Nachkommastelle ab |
| alle Sonden eines Typs in einem Prozess (`--verzeichnis`) | **31,1 s** gegen 31,3 s — kein Unterschied |

Der zweite Fall beruhte auf einem Messfehler: ein *einzelner, kalter* Aufruf
braucht 960 ms und sieht nach hohen Fixkosten aus; im laufenden Sweep liegen
die Qt- und GL-DLLs im Cache und ein Start kostet real ~0,3 s.

**Was der erste Einsatz gefunden hat** (S54, Details im Session-Report): 43 von
129 Skript-Frame-Kopien wurden nie gelesen — das Skript rechnete, sein Ergebnis
verfiel. Dazu der Init-Slot, den die Vorbelegung jedes Frames überschrieb.
Beides behoben; die Sonden belegen den Unterschied Feld für Feld.

## 10. Strang F — Tooltip an jedem Feld (Vorgabe Patrik, S53)

Heute tragen nur einzelne Zeilen einen Tooltip (Farbtafel, Kernel, Bild-Zeile,
Referenz-Regler). Bei allen anderen muss man den Namen deuten — und die
**Variablennamen der Skriptfelder** stehen ausschließlich im Doxygen-Kommentar des
`…Params`-Structs, also dort, wo sie beim Bedienen niemand sieht.

**Ziel:** jedes Feld erklärt sich beim Überfahren. Drei Sorten Text:

| Feldart | Inhalt des Tooltips |
|---|---|
| Zahl/Schalter | was der Wert bewirkt, Einheit, sinnvoller Bereich; bei Klasse A zusätzlich der AVS-Referenzwert (steht dort schon) |
| Enum | was die Einträge unterscheiden, soweit der Name es nicht sagt |
| Skriptfeld | **die schreibbaren Variablennamen dieses Knotens** + `b`/`w`/`h` + Audio-Satz, plus wann der Slot läuft |

**Woher der Text kommt.** Die Beschreibungen existieren bereits als
Doxygen-Kommentare an den Struct-Feldern. Der saubere Weg ist eine Tabelle
`{typkey, feldname} → Text` neben dem Panel statt Text im Code zu verdoppeln —
sonst driften Kommentar und Tooltip auseinander. Ein Gate kann prüfen, dass jedes
Feld aus `fieldNames()` einen Eintrag hat; dann kann kein neues Feld ohne Erklärung
durchrutschen.

## 11. Strang G — Basis-Voreinstellungen für alle Module (Vorgabe Patrik, S53)

Der Mechanismus steht seit Etappe 1 (§3), gefüllt ist er kaum: drei Beispiele für
Metaballs/Tentacles und die 13 SuperScope-Figuren. **Ziel:** jeder Knotentyp hat
mindestens einen brauchbaren Startpunkt, damit ein frisch eingefügter Knoten nicht
als weißes Nichts dasteht.

**Quellen**, in dieser Reihenfolge:

1. **Vorhandene Presets** — die Sammlung unter `…\cmake\VisualsPresets` enthält für
   die meisten AVS-Knoten bewährte Parametersätze. Sie lassen sich aus einer
   importierten Kette per „Speichern unter…" abgreifen; das ist der schnellste Weg
   zu Voreinstellungen, die nach etwas aussehen.
2. **Die Test-Presets aus Strang E**, wo sie mehr zeigen als ein Extremwert.
3. **Von Hand**, wo beides nichts hergibt (die Nachbauten, die 3D-Knoten).

**Offen für S54:** Umfang und Auswahl — wie viele je Knoten, und ob die Namen einer
Konvention folgen (etwa „Standard" als erster Eintrag). Sinnvoll wären
**Teil-Presets** (§3): eine Voreinstellung, die nur die Geometrie setzt und die
Farbtafel des Knotens stehen lässt, ist brauchbarer als eine, die alles überschreibt.

## 12. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 1.15.0 | 2026-07-27 | Session 53 — **drei neue Stränge aufgenommen** (Vorgaben Patrik für S54): §9 Test-Presets je Modul und Feld (ein Feld je Preset, statischer Untergrund für Transformationen), §10 Tooltip an jedem Feld (Skriptfelder nennen ihre Variablen; Tabelle statt Textdopplung + Gate), §11 Basis-Voreinstellungen aus vorhandenen Presets. Etappen 7–9 in §7 |
| 1.14.0 | 2026-07-27 | Session 53 — `Movement` verliert seine Strang-D-Felder wieder (Befund Patrik, `movement3b.lvfx`): zwei Skript-Träger nebeneinander teilen keine Variablen, und die Tabelle cacht ohnehin über den Skripttext. Daraus die Regel „ein Knoten, ein Skript-Träger" — über alle Renderer geprüft, Movement war der einzige Verstoß |
| 1.13.0 | 2026-07-27 | Session 53 — **Strang D vollständig**: auch die 35 Trans-/Bild-Effekte, insgesamt 48 Renderer mit `runParamScript`. Damit sind alle sechs Etappen umgesetzt. Beim maschinellen Einfügen ein Fehler und seine Reparatur: das Muster suchte über Funktionsgrenzen hinweg und legte Blöcke in fremde Funktionen — Gegenmittel ist, erst den Funktionskörper zu isolieren und **danach** darin zu suchen, plus die Zählprobe „genau ein `runParamScript` je Funktion" |
| 1.12.0 | 2026-07-27 | Session 53 — Strang D auf **alle 13 Render-Knoten** vollständig (dazu Starfield · Timescope · Dot Grid · Dot Fountain). Matrix unverändert: starfield und timescope weiter 0,000 |
| 1.16.0 | 2026-07-28 | Session 54 — **Strang E umgesetzt** (§9.1): Inventar-Golden (85 Typen, 717 Felder) mit C++-Gate, Ernte von Bereichen/Doku/Skriptvariablen, Generator und Urteil. Beim ersten Einsatz zwei Befunde an Strang D: **43 von 129 Frame-Kopien wurden nie gelesen**, und der **Init-Slot** war wirkungslos, weil die Vorbelegung jedes Frames ihn überschrieb — beides behoben (Entscheid Patrik: Init setzt eine einmalige Startbelegung, die Frames schreiben sie fort; ein Reglerdreh gewinnt). Dazu: Skriptfehler werden gemeldet (`lastError` wurde nie abgefragt), `timescope.useChannel` macht den im Original toten Kanalregler wahlweise wirksam, Colorfades Beat-Fader sind skriptbar. Matrix 36/41, Sonden 78/80, Tests 481 — keine Regression |
| 1.11.0 | 2026-07-27 | Session 53 — Strang D auch für **Klasse A** (Dot Plane · Bass Spin · Moving Particle). Dabei die Lücke geschlossen, die das offen gehalten hatte: die ⚠ an den Reglern kann ein Frame-Skript nicht sehen, deshalb tragen diese Knoten eine eigene Hinweiszeile über den Skriptfeldern. Matrix unverändert, dot_plane/moving_particle weiter 0,000 |
| 1.10.0 | 2026-07-27 | Session 53 — Strang D auf **alle sechs Klasse-B-Knoten** erweitert (dazu Metaballs 3D, Tentacles 3D, FyrewurX). `runMetaballs3D`/`runTentacles3D` bekamen dafür den `node`-Parameter, den sie als einzige Renderer nicht hatten |
| 1.9.0 | 2026-07-27 | Session 53 — **Etappe 6 begonnen**: Fundament `runParamScript` (opt-in, Frame-Kopie) + die drei Scope-Knoten angeschlossen. Matrix 36/41 und Sonden 78/80 unverändert, die drei Zeilen bitgleich |
| 1.8.0 | 2026-07-27 | Session 53 — **Etappe 5 (Klasse A)**: `Dot Plane` (camDistance/settle), `Bass Spin` (smoothing/spinStep), `Moving Particle` (spring/damping) mit **⚠-Kennzeichnung bei Abweichung** (`addRefDouble`). Am Vorstand belegt: dot_plane 0,000 · bass_spin 0,006 · moving_particle 0,000 — bitgleich. Dabei gefunden: der Default von `spinStep` muss der Original-**Ausdruck** `3.14159f/6.0f` sein, nicht das mathematische π/6 (4,7e-7 Unterschied, summiert sich in der Drehlage auf) |
| 1.7.0 | 2026-07-27 | Session 53 — **Etappe 4 abgeschlossen**: auch Metaballs 3D, Tentacles 3D, FyrewurX und Triangle haben ihre Literale als Regler. Default-Vertrag je Knoten durch einen Serializer-Test gedeckt; Matrix 36/41 und Sonden **78/80** unverändert. Nebenbefund: `6_alloy/paar_original` war schon vor S53 rot (am Vorstand belegt) — der S52-Stand „79/80" war ungenau |
| 1.6.0 | 2026-07-27 | Session 53 — **Etappe 4 (Klasse B) + Etappe 3b**: `Rotating Stars` (hatte null Parameter), `Osc Star` und `Osc Ring` haben ihre Renderer-Literale als Regler bekommen; **Default-Vertrag am Vorstand belegt** (Renderer gestasht, neu gebaut, dieselben drei Matrix-Zeilen: 0,015/0,004/0,003 — bitgleich). Dazu die Bild-Auswahlfelder + Bilder-Suchordner (S50-Vorgabe, vorgezogen) |
| 1.5.0 | 2026-07-27 | Session 53 — **Etappe 3 umgesetzt**: 7×7-Kernel-Gitter (`Convolution`) und Stützstellen-Liste (`ColorMap`) sind editierbar, beide standen vorher als „imported, read-only" da. Korrektur: der Kernel ist **7×7 = 49 Werte**, v1.0.0 sagte 5×5. Damit ist Strang B bis auf die Bild-Felder erledigt |
| 1.4.0 | 2026-07-27 | Session 53 — **Etappe 2 umgesetzt**: vier Farbtafeln (Simple Scope · Rotating Stars · Osc Star · Osc Ring) + Dot Grid vollständig, `Movement.builtinRemap`, `CustomBpm.skipFirst`, `Colorfade.onBeatFrames`, `Blur.roundUp`, `TexerII.resizing/wrapAround/beatCode`, `Triangle.beatCode`. Audit-Rest: nur noch Kernel/ColorMap (Etappe 3) und die Bild-Felder |
| 1.3.0 | 2026-07-27 | Session 53 — **Etappe 1b umgesetzt** (Vorgabe Patrik: *eine* Preset-Liste, nicht zwei): 13 SuperScope-Figuren als Teil-Presets in Dateien, „Figure"-Dropdown + `applySuperScopePreset` + `superscopeFigures()` entfernt. Dazu **Merge-Semantik** beim Laden und **Feldauswahl** beim Speichern. Nebenbefund: `presetName(Starburst)` fehlte in der `switch`-Tabelle (`default` fing es ab → die Figur hieß „Unknown") |
| 1.2.0 | 2026-07-27 | Session 53 — **Etappe 1 umgesetzt**: `NodePresetStore` (Qt-JSON, GL-frei) + Voreinstellungs-Zeile im `MultiEffectPanel` + `asset/nodepresets/` mit drei Beispielen + `test_NodePresetStore` (6 Fälle, u. a. „jede mitgelieferte Datei ist ladbar") |
| 1.1.0 | 2026-07-27 | Session 53 — Entscheide Patrik: §8.1 SuperScope-Figuren werden Dateien (neue Etappe 1b), §8.2 Voreinstellung = `params` inkl. EEL-Formeln. §8.3 am Referenz-Quelltext geklärt: `Dot Fountain` ist **keine Portierung**, Matrix-Zeile falsch grün → aus dem Etappenplan gestrichen, als Kalibrier-Befund abgegeben |
| 1.0.0 | 2026-07-27 | Erstfassung (Session 53) — Bestandsaufnahme über alle 81 Knotentypen, vier Stränge, Default-Vertrag, Etappenplan |

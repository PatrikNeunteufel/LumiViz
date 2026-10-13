# Knoten-Voreinstellungen (mitgeliefert)

Benannte Parametersätze für einzelne Knoten der Effektkette. Eine Voreinstellung ist
**die `params` eines Knotens unter einem Namen** — einschließlich seiner EEL-Formeln,
aber ohne Kinder und ohne Anzeigenamen.

> **Diese Datei ist der Anker der Verzeichnissuche.** `NodePresetStore` sucht vom
> Programmordner aufwärts nach `asset/nodepresets/README.md`. Wird sie entfernt,
> findet die App die mitgelieferten Voreinstellungen nicht mehr.

## Aufbau

```
asset/nodepresets/<typkey>/<Name>.json
```

`<typkey>` ist der Schlüssel aus `effectTypeKey()` — derselbe, der auch in einer
gespeicherten Kette (`.lvfx`) unter `"type"` steht: `superScope`, `movement`,
`dynamicMovement`, `metaballs3d`, `list` …

**Milkdrop-Elemente und -Sektionen** (seit S83) sind keine Knoten, tragen aber
dasselbe Format unter eigenen Schlüsseln: `milkWave` (Custom Wave), `milkShape`
(Custom Shape), `milkSprite` (Sprite) — Feldsatz des Milkdrop-Serializers
(`waveToJson` …) ohne den Rahmen `index`/`enabled`; `milkCode` (`perFrameInit`,
`perFrame`, `perPixel`) und `milkParams` (alle skalaren Felder des Preset-
Dokuments außer Code, Shadern, Element-Listen und Meta). Ein Test prüft jedes
Feld jeder Datei gegen den Serializer — ein Tippfehler fällt dort auf.

Selbst gespeicherte Voreinstellungen landen **nicht** hier, sondern im Benutzerordner
(`<AppData>/LumiViz/nodepresets/<typkey>/`). Bei gleichem Namen gewinnt der
Benutzerordner; mitgelieferte Dateien lassen sich in der App nicht überschreiben oder
löschen.

## Dateiformat

```json
{
  "format": "lumiviz-nodepreset",
  "formatVersion": 1,
  "node": {
    "type": "<typkey>",
    "<parameter>": <wert>
  }
}
```

Der `node`-Block ist derselbe, den `ChainSerializer::nodeToJson` für diesen Knoten
schreiben würde — nur ohne `name`, `description`, `enabled` und `children`. Beim Laden
wird `type` gegen den Knoten geprüft: eine Voreinstellung eines anderen Typs wird
abgelehnt, nicht stillschweigend zum Passthrough.

## Teil-Presets

**Laden ist ein Merge, kein Ersatz:** Eine Datei überschreibt genau die Felder, die sie
enthält — alles andere bleibt stehen. Eine Datei muss also nicht alle Parameter tragen.

Die **SuperScope-Figuren** in `superScope/` sind genau das: sie enthalten nur die vier
EEL-Slots und `pointCount`. Wer eine Figur lädt, behält deshalb seine Farbtafel,
Linienbreite und den Blend-Modus — dasselbe Verhalten wie das frühere
„Figure"-Dropdown, das sie ersetzt haben.

In der App entsteht ein Teil-Preset über die Feldauswahl im Dialog **Speichern
unter…**: abgehakte Felder kommen in die Datei, abgewählte nicht.

## Eigene Voreinstellungen mitliefern

Datei in den passenden `<typkey>`-Ordner legen — mehr ist nicht nötig, die App liest
das Verzeichnis bei jedem Öffnen des Editors neu. Am einfachsten entsteht so eine
Datei in der App selbst (**Speichern unter…** in der Zeile „Voreinstellung"); danach
aus dem Benutzerordner hierher kopieren.

## Katalog der mitgelieferten Voreinstellungen (Stand S70)

Alle Einträge sind **Teil-Presets**: sie setzen nur die Felder, die den Charakter
ausmachen — Farbtafeln, Blends und alles Übrige bleiben beim Nutzer, sofern nicht
ausdrücklich Teil der Vorlage.

| Typ | Vorlage | Was sie tut |
|---|---|---|
| `superScope` | 13 Figuren (Circle, Spiral, Heart, …) | Die klassischen Figure-Dropdown-Formen: nur die vier EEL-Slots + `pointCount` (seit S53) |
| `superScope3d` | 7 Figuren (Spiral Galaxy, Sphere Cloud, Wave Ribbon, Star Flight, Beat Pulse Orb, Lissajous 3D, Spectrum Ring) | Die früheren „Template"-Einträge (S48), seit S83 hier: vier EEL-Slots + `pointCount` |
| `camera3d` | 6 Fahrten (Orbit, Beat Zoom, Handheld Sway, Fly Forward (Fog), Bass Shake, Top-Down Spin) | Die früheren „Template"-Einträge (S48), seit S83 hier: nur Init/Frame/Beat |
| `milkWave` | Oscilloscope Line · Spectrum Circle · Color Cycle | Form + Punkt-Code; Color Cycle nur Frame-Code (S83) |
| `milkShape` | Pulse With Bass · Slow Spin · Slow Fade | Frame-Code, bei Bedarf `sides`/`rad` (S83, Wunschliste S80) |
| `milkCode` | Slow Rotation · Beat Zoom · Wave Color Cycle · Ripple (per pixel) · Twist (per pixel) | Bausteine für per_frame/per_pixel — setzen nur ihr eigenes Feld (S83) |
| `milkParams` | Original · Old LumiViz Look (pre-S80) · Soft Pull Up-Left · No Drift or Blur | Stellungen von Gestaltung und „bisher"-Schaltern (Skript-Zufall, Audio, Seitenverhältnis) — *Original* = alles original-treu, *Old LumiViz Look* = der Look bis S80 in einem Zug (S83) |
| | Radial Swirl · Radial Pull · Slow Fade · Mirror Echo · Gentle Zoom | einzelne Gestaltungs-/Basiswerte |
| | Modern Rules (IEEE) · Legacy Rules (D3D9) | Regelwerk — *Legacy* ist die original-treue Betriebsart, **kein** „bisher"-Schalter |
| `metaballs3d` | Cold Swarm · Few Large Balls | Host-eigene Kugel-Sets (seit S53) |
| `tentacles3d` | Jellyfish | Host-eigenes Tentakel-Set (seit S53) |
| `dynamicMovement` | Gentle Pull | 5 %-Sog je Frame — der klassische Zoom-Trail-Motor |
| | Beat Pulse | Dauer-Zoom + Beat-Kick mit weicher amp-Feder |
| | Whirlpool | innen stärker drehend als außen, leichter Sog |
| | Water Wobble | laufende radiale Welle (sin über d und t) |
| | Tunnel Flight | langsamer Sog + stetige Rotation |
| | Pillow Wave | rechteckige Stauchung mit wandernder Querwelle |
| `movement` | Pull To Center | statischer 8 %-Zoom als Tabelle (billigster Feedback-Sog) |
| | Slight Twist | konstante Drehung + milder Sog |
| | Fisheye | Mitte aufgewölbt, Rand gestaucht (pow d) |
| | Wave Rings | feste konzentrische Verzerrungsringe |
| `convolution` | Blur 3x3 · Gaussian 3x3 | flacher bzw. gewichteter Blur (Referenz-Ganzzahlkerne) |
| | Sharpen | Zentrum 5, Kreuz −1 |
| | Edges (Laplace) | nur Konturen bleiben (der Kalibrier-Kern) |
| | Emboss | Emboss — diagonale Licht-/Schattenkanten |
| `colorMap` | Fire · Ocean · Neon · Inverted Grayscale | Verlaufs-Paletten über `stopPos`/`stopColor` |
| `waterBump` | Fine Rain | kleine flache Zufallstropfen je Beat |
| | Heavy Drop | ein großer Mitteltropfen je Beat |
| | Slow Waves | Referenztropfen, langsamer Abkling (density 8) |
| `dotGrid` | Calm Grid · Diagonal Drift · Dense Flicker | Punktgitter von dezent bis vibrierend (8.8-Festkomma-Drift) |
| `blitterFeedback` | Zoom Echo | leichtes Raus-Zoomen mit 50/50-Echo |
| | Beat Jump | Beat springt nach innen (scale2) und federt zurück |
| | Inward Pull | dauerhaftes Rein-Zoomen, subpixel-weich |
| `rotatingStars` | Star Wreath | sechs Sterne auf der Bahn, halbes Tempo |
| | Big Show Stars | groß + kräftige Audio-Reaktion (Bühnen-Variante) |
| `oscStar` | Fine Star · Wild Star | kompakt/langsam bzw. bildfüllend/schnell |
| `oscRing` | Pulse Ring · Spectrum Ring | Waveform- bzw. Spektrum-getriebener Ring |
| `grain` | Film Grain | lebendiges Korn, additiv |
| | Frozen Grain | statisches Kornmuster (AVS static grain) |
| `fadeout` | Quick Fade-Out | kräftig nach Schwarz — kurze Trails |
| | Blue Afterglow | langsam auf Tiefblau — lange, kalte Trails |
| `bump` | Circling Lamp | Lichtquelle kreist per EEL über dem Relief |
| | Beat Flash | Tiefe springt je Beat hoch und fällt zurück |
| `movement` (AVS-Originale) | AVS Big Swirl Out · Sunburster · Swirl To Center · Bubbling Outward · Tunneling · Spinny Tube · Gridley · 6-way Kaleida | die wörtlichen Builtin-Formeln aus `r_trans.cpp` als benannte Vorlagen |
| `dynamicShift` | Quake | Beat-Ruck in Zufallsrichtung mit Auspendeln (Kamera-Shake) |
| | Floating Drift | langsame Lissajous-Schwebung des ganzen Bildes |
| `dynamicDistanceModifier` | Beat Surge | Radius federt je Beat nach außen |
| | Breathing Ring | sin-Atmung auf d |
| `rotoBlitter` | Slow Rotation | reine Rotation (Zoom neutral = 31) |
| | Rotating Pull | die kalibrierten Matrix-Werte (28/40) — der klassische Wirbel |
| | Beat Reverse | jeder Beat kehrt die Drehrichtung weich um |
| `mirror` | Mirror Quartet | alle vier Achsen — vierfache Symmetrie |
| | Random Mirror | je Beat neue Spiegelkombination, weich geblendet |
| `interferences` | Double Image | zwei rotierende Halbtransparenz-Kopien |
| | Six-Fold Echo | sechs Kopien im Kreis — rotierende Blume |
| `starfield` | Calm Star Ride | gleichmäßiger Untergrund-Flug |
| | Warp On Beat | Beat zündet einen abklingenden Warp-Schub |
| `uniqueTone` | Gold Tone · Ice Blue | Helligkeit auf Warm-Gold bzw. Kaltblau umgefärbt |
| `interleave` | Scanlines | jede zweite Zeile schwarz (Röhren-Look) |
| | Beat Grid | Gitter reißt je Beat grob auf und schließt sich |
| `videoDelay` | Half Second | Bild von vor 30 Frames (Echo-Baustein) |
| | Beat Echo | Verzögerung um genau einen Beat |
| `timescope` | Spectral Curtain | wandernde Spektrum-Spalten |
| `bloom` | Lights Reference | die Referenzwerte aus `lights_demo` (sigma 8, Intensity 1,3, Vignette 0,3) |
| | Soft Shimmer | dezenter Glanz nur auf Glanzlichtern (Threshold 0,35) |
| | Beat Glow | Glow-Intensität zündet je Beat und klingt weich ab |
| `fractal2D` | Seahorse Valley | klassischer Mandelbrot-Ausschnitt (Seahorse Valley, Zoom 90) |
| | Julia Pulse | Julia-Menge; Beat stößt den Zoom an, langsame Drehung |
| | Burning Ship Coast | das „kleine Schiff" des Burning-Ship-Fraktals |
| `fractal3D` | Mandelbulb Orbit | Kamera umkreist den Bulb, Pitch atmet mit der Zeit |
| | Menger Flight | Menger-Schwamm; Bass zieht die Kamera heran |
| | Quaternion Pulse | Quaternion-Julia; Beat holt die Kamera kurz nah heran |
| `domainWarp` | Cloud Chamber | ruhiges Plasma — mittlere Verwerfung, langsame Drift |
| | Bass Surge | Verwerfung hängt am Bass, Tempo an der Lautstärke |
| | Ink Flow | starke, feine Verwerfung — marmorierter Tintenfluss |
| `fractalZoomer` | Elephant Valley | Endlos-Zoom auf das Elephant Valley der Mandelbrot-Menge |
| | Julia Whirl | rotierender Julia-Zoom mit kräftiger Feedback-Schleife |
| | Beat Thrust | Zoomtempo springt je Beat und pendelt zurück |
| `lyapunov` | Zircon City | die berühmte BBBBBBAAAAAA-Ansicht (Markus/Dewdney) |
| | Breathing Window | der Ausschnitt atmet langsam über sin/cos der Zeit |
| `kleinian` | Hyperbolic Flower | {7,3}-Kachelung, langsam drehend |
| | Beat Tiling | {5,4}; jeder Beat schiebt die Morph-Phase weiter |
| `strangeAttractor` | Lorenz Butterfly | der klassische Lorenz-Attraktor, dicht gezeichnet |
| | Clifford Veil | Clifford-Attraktor (a −1,4 · b 1,6 · c 1 · d 0,7) |
| | De Jong Web | Peter-de-Jong-Attraktor (1,4 · −2,3 · 2,4 · −2,1) |
| `flame` | Fire Swirl | Swirl-Variation, drei Abbildungen — rotierende Glut |
| | Horseshoe Nebula | Horseshoe-Variation, drei Abbildungen (mit vier kollabiert die Bahn) |
| `reactionDiffusion` | Coral Growth | Gray-Scott „Coral" (feed 0,0545 / kill 0,062) |
| | Cell Division | „Mitosis" (0,0367 / 0,0649) — teilende Zellen |
| | Worm Trail | „Worms" (0,078 / 0,061) — wandernde Bänder |
| `blur` | Fading Trail | strength 1 ohne round-up — das Bild klingt je Anwendung ab (Trail-Motor) |
| | Static Blur | strength 2 mit round-up — weichzeichnen ohne Abklingen |
| `simpleScope` | Analyzer Bars | solid analyzer unten — die klassischen Spektrum-Balken |
| | Line Scope | line scope mittig — das klassische Oszilloskop |
| | Dot Cloud | dot scope mittig — die Waveform als Punktstaub |
| `customBpm` | Half Tempo | lässt jeden zweiten Beat durch |
| | Quarter Beat | lässt jeden vierten Beat durch |
| | Metronome 500ms | fester Kunst-Beat alle 500 ms (unabhängig vom Audio) |
| `meshWarp` (GPU-Module S69) | Bass Swirl | Drehung um die Mitte, Bass verstärkt — der Palette-Starter als Vorlage |
| | Tunnel Pull | dauerhafter Sog zur Mitte mit Rotations-Drall (Feedback-Tunnel) |
| | Swell | laufende Sinus-Wellen quer durchs Bild (Waveform-Wasser) |
| | Fisheye Breathing | Fischaugen-Wölbung, die mit der Zeit/dem Bass atmet |
| | Hall of Mirrors | Kaleido-Faltung der UV — Inhalt gehört zentriert (Stimm-Befund S69) |
| `gpuParticles` (GPU-Module S69) | Fountain | klassische Partikel-Fontäne (Schwerkraft + Fächer) |
| | Spark Rain | fallende Funken über die Bildbreite |
| | Bass Explosion | radiale Explosion, vom Bass gezündet |
| | Mist Drift | große, langsame, halbtransparente Partikel |
| | Cyclone | Kraftfeld-Wirbel um die Mitte (kraft()-GLSL) |
| `list` (Batch 1, S69) | Beat Gate | Liste nur im Beat-Fenster aktiv (Slot-Akkumulator statt `time`) |
| | Bass Blend | Listen-Blend folgt dem Bass |
| | Pulse Layer | Layer pulst periodisch über den eigenen Akkumulator |
| | A-B Switcher | Liste schaltet je Beat ein/aus — zwei versetzte Kopien ergeben den A/B-Wechsel |
| `brightness` | Bass Boost | Helligkeit folgt dem Bass |
| | Channel Breathing | R/G/B atmen phasenversetzt |
| `colorfade` | Beat Flash | Fader zündet je Beat und klingt ab |
| | Color Drift | langsame Dauerdrift der Farbkanäle |
| `colorModifier` | Contrast Pump | Kontrast pumpt mit dem Bass (Level-Code, recompute) |
| | Gamma Breathing | Gamma atmet langsam über die Zeit |
| `mosaic` | Beat Tiles | Kachelgröße springt je Beat und federt zurück |
| `channelShift` | Beat Rotation | Kanal-Rotation schaltet je Beat weiter |
| `colorClip` | Bass Eater | Clip-Distanz folgt dem Bass |
| `multiFilter` | Chrome Beat | Chrome-Filter je Beat |
| `addBorders` | Pulse Frame | Rahmenbreite pulst mit dem Bass (weißer Rahmen) |
| `onBeatClear` | Every-Fourth Reset | löscht jeden vierten Beat |
| `clear` | Midnight Blue Veil | halbtransparentes Tiefblau je Frame (weicher Trail-Deckel) |
| `bufferSave` | Echo Buffer | Speichern/Wiederherstellen im Wechsel (dir 2, Slot 0) — Echo-Baustein |
| `pixelFilter` (Stilfilter S70) | Take On Me Comic | Bleistift-Rotoskopie à la a-ha: Sobel-Kantenzug, Papier/Tinte, Beat-zitternde Schraffur |
| | Pencil Sketch | XDoG-Strich (Difference-of-Gaussians) + Papierkorn |
| | Posterize Pop Art | kräftige Sättigung, wenige Farbstufen — Stufenzahl atmet mit dem Bass |
| | Newsprint Halftone | Halftone-Punkte auf 45°-Raster, Punktgröße = Dunkelheit |
| | CRT Monitor | Tonnen-Verzerrung, Scanlines, RGB-Lochmaske, Vignette |
| | VHS Tape | Chroma-Versatz, Zeilen-Zittern, wanderndes Störband — Beat verstärkt den Glitch |
| | Kuwahara Oil Painting | Varianz-ärmster 3×3-Quadrant gewinnt — malerische Flächen, stehende Kanten |
| | Sepia Nostalgia | Sepia-Matrix + Vignette + Korn + Projektor-Flackern |
| | Noir Black and White | harte Kontrastkurve, Korn, Vignette — Bass drückt den Kontrast |
| | Thermal Image | Helligkeit auf Thermal-Palette (blau→grün→gelb→rot→weiß), Pegel hebt die Temperatur |
| | Pixel Art | grobe Blöcke + reduzierte Palette, Blockgröße pulsiert mit dem Beat |
| | Duotone Neon | Helligkeit auf zwei driftende Neonfarben, Höhen hellen die Lichter |
| `dynamicMovement` (Import-Ernte S61) | Galaxy Twist | Beat-getriebener Spiralen-Twist (aus „UnConeD — Milkyway") |
| | Wandering Lens | horizontale Zerr-Linse, Ziel wandert je Beat (aus „amphirion — nebulous, skupers remix") |
| | Beat Direction Drift | konstanter Drift, Richtung würfelt je Beat (aus „L1quid — Take the Veil") |
| | Molecule Grid | gefaltetes Sinus-Raster, Beat stößt den Impuls an (aus „EL-VIS — Molecules, S_KuPeRS Remix") |
| | Sine Weave | gekreuzte Sinus-Verwerfung, je Beat neu gestimmt (aus „amphirion — nebulous") |
| | Beat Spiral | Rotation/Zoom im gemessenen Beat-Takt (aus „D&L — Life Is Violated") |
| `text` (S86) | Beat Words | großes, fettes Wort je Beat, additiv, mittig |
| | Slideshow | Wörter im Zeittakt mit Leerbild dazwischen |
| | Random Flash | kurzes Aufblitzen je Beat an zufälliger Stelle, zufälliges Wort |
| | Subtitle | unten mittig, mit Kontur — bleibt auf jedem Bild lesbar |
| | Beat Colours | Beat-Skript würfelt je Wort eine neue Farbe (`color_r/_g/_b`) |
| | Bass Tempo | Frame-Skript: lauter Bass = schnellerer Wortwechsel (`normspeed`) |
| | Beat Jump | Beat-Skript versetzt den Text je Beat (`xshift`, `yshift`) |
| | Now Playing | **setzt den Text**: `[{artist} – ]{title}` — Interpret und Titel des laufenden Stücks, unten mittig mit Kontur |
| `avi` (S86) | Beat Blend | beat-adaptiv: additiv im Beat-Fenster, sonst 50/50 |
| | Slow Motion | Bildwechsel höchstens alle 90 ms |
| | Additive Overlay | Video additiv über dem Bild |
| | Beat Stutter | Beat-Skript würfelt die Bildzeit neu (`speedms`) |
| | Bass Persist | Frame-Skript: Beat-Fenster wächst mit dem Bass (`persist`) |
| `lyrics` (S86) | Karaoke Bottom | laufende Zeile unten mittig mit Kontur, eine Vorschau-Zeile |
| | Big Centre | eine große Zeile in der Bildmitte, additiv, ohne Vorschau |
| | Three Lines Left | linksbündig unten, laufende Zeile plus zwei Vorschau-Zeilen |
| `lyrics` (S87) | Karaoke Top | wie Karaoke Bottom, am oberen Rand |
| | Karaoke Classic | gelbe laufende Zeile, weiße Vorschau (zwei Zeilen), kräftige Kontur |
| | Karaoke Neon | additiv ohne Kontur, Cyan vor gedämpftem Magenta |
| | Karaoke Beat Colours | Beat-Skript: die laufende Zeile bekommt bei jedem Beat eine neue helle Farbe |
| | Karaoke Song Colours | Init- und Song-Skript: jeder Song bekommt seine eigene Farbe |
| | Karaoke Bounce | Beat-Skript: der Text springt bei jedem Beat ein Stück (`jump_x`, `jump_y` in Prozent) |
| | Karaoke Flip | Beat-Skript: der Text wechselt alle `flip_beats` Beats zwischen unten und oben |
| | Subtitle Small | kleine Untertitel-Zeile ohne Vorschau |
| | Verse Sheet | Textblatt oben links mit vier Vorschau-Zeilen; unsynchroner Text rückt alle 8 Beats eine Zeile weiter |
| | Side Right | rechtsbündig auf halber Höhe, zwei Vorschau-Zeilen |

Alle Lyrics-Vorlagen tragen seit S87 den vollen Satz aus Lage, Farben und
Skriptfeldern (auch leere) — ein Wechsel von einer bewegten Variante auf eine
feste lässt kein Skript der vorigen stehen.
| `hostSwitcher` (S87) | Random Runtime | Gruppe wechselt nach einer zufälligen Zahl Beats zwischen `min_runtime` und `max_runtime` (im Init einstellen); jeder Song würfelt neu, Reihenfolge zufällig. Die Gruppenliste bleibt unberührt |
| | Random Runtime per Group | wie Random Runtime, aber jede Gruppe würfelt ihre eigene Laufzeit |
| | Random Seconds | dasselbe in Sekunden statt Beats (`min_seconds`, `max_seconds`) — läuft auch ohne erkannten Beat |
| | Every 16 Beats | fester Takt: alle `beats_per_group` Beats weiter, Gruppen der Reihe nach |
| | Bass Drop | wechselt bei einem Bass-Schlag deutlich über dem laufenden Mittel (`kick`), frühestens nach `min_seconds`, spätestens nach `max_seconds` |
| | One per Song | ohne Skript: jeder Song bekommt eine andere Gruppe |
| `shape` (S121) | Heart | die Form allein, 64 Harmonics, ohne Skript |
| | Epicycles | Stern mit Show circles, 16 Harmonics — die Kreise und Arme zeichnen mit |
| | Harmonics Sweep | Frame-Skript: Harmonics läuft am Quadrat von 1 bis 64 und zurück |
| | Beat Harmonics | Beat-Skript: Harmonics fällt auf 3, das Frame-Skript lässt es zurückwachsen |
| | Beat Pulse | Beat-Skript: Size springt, das Frame-Skript lässt es abklingen |
| | Bass Spin | Frame-Skript: Rotation folgt dem Bass |
| | Wave Outline | Point-Skript: die Wellenform schiebt jeden Punkt des Kreises nach außen |
| | Rainbow Outline | Point-Skript: die Farbe läuft mit `i` um die Linie |
| | Contour Switch | Beat-Skript: `contour` wechselt zwischen den zwei Konturen von Heart and star |

Die Shape-Vorlagen tragen Form, Contour, Harmonics, Points, Größe, Drehung,
Show circles, Speed und alle vier Skriptfelder (auch leere) — ein Wechsel lässt
kein Skript der vorigen stehen. Strichbreite, Farbe, Lage und Line blend
bleiben beim Nutzer.

Die Text- und AVI-Vorlagen lassen den Text bzw. die Videodatei unberührt —
außer „Now Playing", das die Tag-Vorlage als Text mitbringt.

Die Zeilen „Import-Ernte S61" sind wörtlich aus Presets der Sammlung
`VisualsPresets` übernommen (Entscheid S60: Quelle = Sammlung via Import);
die Quell-Presets und ihre Autoren stehen je Zeile in Klammern.

## Umbenennung S83 (deutsch → englisch)

Auch Kommentare im Code, Beschreibungen und Knotennamen sind englisch. Ein
Wächter-Test prüft Dateinamen und Texte gegen eine Liste bekannter deutscher
Namen und Wörter (`projects/apps/LumiViz/tests/unit/UnitTests/deutsche_woerter.txt`)
— wer eine Vorlage mitliefert, schreibt sie englisch.

Seit Session 83 heißen alle mitgelieferten Voreinstellungen und Beispiele
englisch (Entscheid Patrik). Ältere Changelogs, Protokolle und Session-Reports
nennen noch die alten Namen — diese Tabelle führt sie zusammen. Mit „nur
Beispiel“ markierte Zeilen gibt es nur unter `asset/examples/`.

| Typ | alt | neu |
|---|---|---|
| `addBorders` | Puls-Rahmen | Pulse Frame |
| `blitterFeedback` | Beat-Sprung | Beat Jump |
| `blitterFeedback` | Sog nach innen | Inward Pull |
| `blitterFeedback` | Zoom-Echo | Zoom Echo |
| `bloom` | Beat-Glut | Beat Glow |
| `bloom` | Lights-Referenz | Lights Reference |
| `bloom` | Sanfter Schimmer | Soft Shimmer |
| `blur` | Abklingender Trail | Fading Trail |
| `blur` | Stehender Weichzeichner | Static Blur |
| `brightness` | Bass-Boost | Bass Boost |
| `brightness` | Kanal-Atmung | Channel Breathing |
| `bufferSave` | Echo-Speicher | Echo Buffer |
| `bump` | Beat-Blitz | Beat Flash |
| `bump` | Kreisende Lampe | Circling Lamp |
| `channelShift` | Beat-Rotation | Beat Rotation |
| `clear` | Nachtblau-Schleier | Midnight Blue Veil |
| `colorClip` | Bass-Fresser | Bass Eater |
| `colorfade` | Beat-Blitz | Beat Flash |
| `colorfade` | Farbdrift | Color Drift |
| `colorMap` | Feuer | Fire |
| `colorMap` | Graustufen invertiert | Inverted Grayscale |
| `colorMap` | Ozean | Ocean |
| `colorModifier` | Gamma-Atmung | Gamma Breathing |
| `colorModifier` | Kontrast-Pump | Contrast Pump |
| `convolution` | Gauss 3x3 | Gaussian 3x3 |
| `convolution` | Kanten (Laplace) | Edges (Laplace) |
| `convolution` | Relief | Emboss |
| `convolution` | Schaerfen | Sharpen |
| `convolution` | Weichzeichner 3x3 | Blur 3x3 |
| `customBpm` | Halbes Tempo | Half Tempo |
| `customBpm` | Metronom 500ms | Metronome 500ms |
| `customBpm` | Viertel-Beat | Quarter Beat |
| `domainWarp` | Bass-Woge | Bass Surge |
| `domainWarp` | Nebelkammer | Cloud Chamber |
| `domainWarp` | Tintenstrom | Ink Flow |
| `dotGrid` | Diagonal-Drift | Diagonal Drift |
| `dotGrid` | Dichtes Flimmern | Dense Flicker |
| `dotGrid` | Ruhiges Raster | Calm Grid |
| `dynamicDistanceModifier` | Atmender Ring | Breathing Ring |
| `dynamicDistanceModifier` | Beat-Woge | Beat Surge |
| `dynamicMovement` | Beat-Puls | Beat Pulse |
| `dynamicMovement` | Beat-Richtungsdrift | Beat Direction Drift |
| `dynamicMovement` | Galaxien-Drall | Galaxy Twist |
| `dynamicMovement` | Kissen-Welle | Pillow Wave |
| `dynamicMovement` | Molekuel-Raster | Molecule Grid |
| `dynamicMovement` | Sanfter Sog | Gentle Pull |
| `dynamicMovement` | Sinus-Flechtwerk | Sine Weave |
| `dynamicMovement` | Strudel | Whirlpool |
| `dynamicMovement` | Takt-Spirale | Beat Spiral |
| `dynamicMovement` | Tunnelflug | Tunnel Flight |
| `dynamicMovement` | Wandernde Linse | Wandering Lens |
| `dynamicMovement` | Wasser-Wobble | Water Wobble |
| `dynamicShift` | Beben | Quake |
| `dynamicShift` | Schwebedrift | Floating Drift |
| `fadeout` | Blaues Nachgluehen | Blue Afterglow |
| `fadeout` | Schneller Ausklang | Quick Fade-Out |
| `flame` | Feuerwirbel | Fire Swirl |
| `flame` | Hufeisen-Nebel | Horseshoe Nebula |
| `fractal2D` | Burning-Ship-Kueste | Burning Ship Coast |
| `fractal2D` | Julia-Puls | Julia Pulse |
| `fractal2D` | Seepferdchen-Tal | Seahorse Valley |
| `fractal3D` | Mandelbulb-Orbit | Mandelbulb Orbit |
| `fractal3D` | Menger-Flug | Menger Flight |
| `fractal3D` | Quaternion-Puls | Quaternion Pulse |
| `fractalZoomer` | Beat-Schub | Beat Thrust |
| `fractalZoomer` | Elefanten-Tal | Elephant Valley |
| `fractalZoomer` | Julia-Strudel | Julia Whirl |
| `gpuParticles` | Bass-Explosion | Bass Explosion |
| `gpuParticles` | Feuerfontaene | Fire Fountain (nur Beispiel) |
| `gpuParticles` | Fontaene | Fountain |
| `gpuParticles` | Funkenregen | Spark Rain |
| `gpuParticles` | Nebel-Drift | Mist Drift |
| `gpuParticles` | Wirbelnebel | Swirl Mist (nur Beispiel) |
| `gpuParticles` | Wirbelsturm | Cyclone |
| `grain` | Eingefrorenes Korn | Frozen Grain |
| `grain` | Filmkorn | Film Grain |
| `interferences` | Doppelbild | Double Image |
| `interferences` | Sechser-Echo | Six-Fold Echo |
| `interleave` | Beat-Gitter | Beat Grid |
| `kleinian` | Beat-Kachelwerk | Beat Tiling |
| `kleinian` | Hyperbolische Blume | Hyperbolic Flower |
| `list` | AB-Wechsler | A-B Switcher |
| `list` | Bass-Blende | Bass Blend |
| `list` | Beat-Gate | Beat Gate |
| `list` | Puls-Layer | Pulse Layer |
| `lyapunov` | Atmendes Fenster | Breathing Window |
| `meshWarp` | Bass-Swirl | Bass Swirl |
| `meshWarp` | Bass-Tunnel | Bass Tunnel (nur Beispiel) |
| `meshWarp` | Fischaugen-Atmung | Fisheye Breathing |
| `meshWarp` | Spiegelkabinett | Hall of Mirrors |
| `meshWarp` | Spiegelkabinett2 | Hall of Mirrors 2 (nur Beispiel) |
| `meshWarp` | Spiegelkabinett3 | Hall of Mirrors 3 (nur Beispiel) |
| `meshWarp` | Tunnel-Sog | Tunnel Pull |
| `meshWarp` | Wellengang | Swell |
| `metaballs3d` | Kalter Schwarm | Cold Swarm |
| `metaballs3d` | Wenige grosse Kugeln | Few Large Balls |
| `mirror` | Spiegel-Quartett | Mirror Quartet |
| `mirror` | Zufallsspiegel | Random Mirror |
| `mosaic` | Beat-Kachel | Beat Tiles |
| `movement` | Fischauge | Fisheye |
| `movement` | Leichter Drall | Slight Twist |
| `movement` | Sog zur Mitte | Pull To Center |
| `movement` | Wellenringe | Wave Rings |
| `multiFilter` | Chrome-Beat | Chrome Beat |
| `onBeatClear` | Vierer-Reset | Every-Fourth Reset |
| `oscRing` | Puls-Ring | Pulse Ring |
| `oscRing` | Spektrum-Ring | Spectrum Ring |
| `oscStar` | Feiner Stern | Fine Star |
| `oscStar` | Wilder Stern | Wild Star |
| `pixelFilter` | Bleistift-Skizze | Pencil Sketch |
| `pixelFilter` | CRT-Monitor | CRT Monitor |
| `pixelFilter` | Duotone-Neon | Duotone Neon |
| `pixelFilter` | Kuwahara-Oelbild | Kuwahara Oil Painting |
| `pixelFilter` | Noir-Schwarzweiss | Noir Black and White |
| `pixelFilter` | Pixel-Art | Pixel Art |
| `pixelFilter` | Posterize-PopArt | Posterize Pop Art |
| `pixelFilter` | Sepia-Nostalgie | Sepia Nostalgia |
| `pixelFilter` | Take-On-Me | Take On Me (nur Beispiel) |
| `pixelFilter` | Take-On-Me-Comic | Take On Me Comic |
| `pixelFilter` | VHS-Band | VHS Tape |
| `pixelFilter` | Waermebild | Thermal Image |
| `pixelFilter` | Zeitungsdruck-Halftone | Newsprint Halftone |
| `reactionDiffusion` | Korallenwachstum | Coral Growth |
| `reactionDiffusion` | Wurmspur | Worm Trail |
| `reactionDiffusion` | Zellteilung | Cell Division |
| `rotatingStars` | Grosse Show-Sterne | Big Show Stars |
| `rotatingStars` | Sternenkranz | Star Wreath |
| `rotoBlitter` | Beat-Wende | Beat Reverse |
| `rotoBlitter` | Langsame Drehung | Slow Rotation |
| `rotoBlitter` | Rotierender Sog | Rotating Pull |
| `simpleScope` | Analyzer-Balken | Analyzer Bars |
| `simpleScope` | Linien-Oszi | Line Scope |
| `simpleScope` | Punktwolke | Dot Cloud |
| `starfield` | Ruhige Sternfahrt | Calm Star Ride |
| `starfield` | Warp auf Beat | Warp On Beat |
| `strangeAttractor` | Clifford-Schleier | Clifford Veil |
| `strangeAttractor` | De-Jong-Gespinst | De Jong Web |
| `strangeAttractor` | Lorenz-Schmetterling | Lorenz Butterfly |
| `tentacles3d` | Qualle | Jellyfish |
| `timescope` | Spektral-Vorhang | Spectral Curtain |
| `uniqueTone` | Eisblau | Ice Blue |
| `uniqueTone` | Goldton | Gold Tone |
| `videoDelay` | Beat-Echo | Beat Echo |
| `videoDelay` | Halbe Sekunde | Half Second |
| `waterBump` | Feiner Regen | Fine Rain |
| `waterBump` | Schwerer Tropfen | Heavy Drop |
| `waterBump` | Traege Wellen | Slow Waves |

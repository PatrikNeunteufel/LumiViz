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
| `metaballs3d` | Kalter Schwarm · Wenige grosse Kugeln | Host-eigene Kugel-Sets (seit S53) |
| `tentacles3d` | Qualle | Host-eigenes Tentakel-Set (seit S53) |
| `dynamicMovement` | Sanfter Sog | 5 %-Sog je Frame — der klassische Zoom-Trail-Motor |
| | Beat-Puls | Dauer-Zoom + Beat-Kick mit weicher amp-Feder |
| | Strudel | innen stärker drehend als außen, leichter Sog |
| | Wasser-Wobble | laufende radiale Welle (sin über d und t) |
| | Tunnelflug | langsamer Sog + stetige Rotation |
| | Kissen-Welle | rechteckige Stauchung mit wandernder Querwelle |
| `movement` | Sog zur Mitte | statischer 8 %-Zoom als Tabelle (billigster Feedback-Sog) |
| | Leichter Drall | konstante Drehung + milder Sog |
| | Fischauge | Mitte aufgewölbt, Rand gestaucht (pow d) |
| | Wellenringe | feste konzentrische Verzerrungsringe |
| `convolution` | Weichzeichner 3x3 · Gauss 3x3 | flacher bzw. gewichteter Blur (Referenz-Ganzzahlkerne) |
| | Schaerfen | Zentrum 5, Kreuz −1 |
| | Kanten (Laplace) | nur Konturen bleiben (der Kalibrier-Kern) |
| | Relief | Emboss — diagonale Licht-/Schattenkanten |
| `colorMap` | Feuer · Ozean · Neon · Graustufen invertiert | Verlaufs-Paletten über `stopPos`/`stopColor` |
| `waterBump` | Feiner Regen | kleine flache Zufallstropfen je Beat |
| | Schwerer Tropfen | ein großer Mitteltropfen je Beat |
| | Traege Wellen | Referenztropfen, langsamer Abkling (density 8) |
| `dotGrid` | Ruhiges Raster · Diagonal-Drift · Dichtes Flimmern | Punktgitter von dezent bis vibrierend (8.8-Festkomma-Drift) |
| `blitterFeedback` | Zoom-Echo | leichtes Raus-Zoomen mit 50/50-Echo |
| | Beat-Sprung | Beat springt nach innen (scale2) und federt zurück |
| | Sog nach innen | dauerhaftes Rein-Zoomen, subpixel-weich |
| `rotatingStars` | Sternenkranz | sechs Sterne auf der Bahn, halbes Tempo |
| | Grosse Show-Sterne | groß + kräftige Audio-Reaktion (Bühnen-Variante) |
| `oscStar` | Feiner Stern · Wilder Stern | kompakt/langsam bzw. bildfüllend/schnell |
| `oscRing` | Puls-Ring · Spektrum-Ring | Waveform- bzw. Spektrum-getriebener Ring |
| `grain` | Filmkorn | lebendiges Korn, additiv |
| | Eingefrorenes Korn | statisches Kornmuster (AVS static grain) |
| `fadeout` | Schneller Ausklang | kräftig nach Schwarz — kurze Trails |
| | Blaues Nachgluehen | langsam auf Tiefblau — lange, kalte Trails |
| `bump` | Kreisende Lampe | Lichtquelle kreist per EEL über dem Relief |
| | Beat-Blitz | Tiefe springt je Beat hoch und fällt zurück |
| `movement` (AVS-Originale) | AVS Big Swirl Out · Sunburster · Swirl To Center · Bubbling Outward · Tunneling · Spinny Tube · Gridley · 6-way Kaleida | die wörtlichen Builtin-Formeln aus `r_trans.cpp` als benannte Vorlagen |
| `dynamicShift` | Beben | Beat-Ruck in Zufallsrichtung mit Auspendeln (Kamera-Shake) |
| | Schwebedrift | langsame Lissajous-Schwebung des ganzen Bildes |
| `dynamicDistanceModifier` | Beat-Woge | Radius federt je Beat nach außen |
| | Atmender Ring | sin-Atmung auf d |
| `rotoBlitter` | Langsame Drehung | reine Rotation (Zoom neutral = 31) |
| | Rotierender Sog | die kalibrierten Matrix-Werte (28/40) — der klassische Wirbel |
| | Beat-Wende | jeder Beat kehrt die Drehrichtung weich um |
| `mirror` | Spiegel-Quartett | alle vier Achsen — vierfache Symmetrie |
| | Zufallsspiegel | je Beat neue Spiegelkombination, weich geblendet |
| `interferences` | Doppelbild | zwei rotierende Halbtransparenz-Kopien |
| | Sechser-Echo | sechs Kopien im Kreis — rotierende Blume |
| `starfield` | Ruhige Sternfahrt | gleichmäßiger Untergrund-Flug |
| | Warp auf Beat | Beat zündet einen abklingenden Warp-Schub |
| `uniqueTone` | Goldton · Eisblau | Helligkeit auf Warm-Gold bzw. Kaltblau umgefärbt |
| `interleave` | Scanlines | jede zweite Zeile schwarz (Röhren-Look) |
| | Beat-Gitter | Gitter reißt je Beat grob auf und schließt sich |
| `videoDelay` | Halbe Sekunde | Bild von vor 30 Frames (Echo-Baustein) |
| | Beat-Echo | Verzögerung um genau einen Beat |
| `timescope` | Spektral-Vorhang | wandernde Spektrum-Spalten |
| `bloom` | Lights-Referenz | die Referenzwerte aus `lights_demo` (sigma 8, Intensity 1,3, Vignette 0,3) |
| | Sanfter Schimmer | dezenter Glanz nur auf Glanzlichtern (Threshold 0,35) |
| | Beat-Glut | Glow-Intensität zündet je Beat und klingt weich ab |
| `fractal2D` | Seepferdchen-Tal | klassischer Mandelbrot-Ausschnitt (Seahorse Valley, Zoom 90) |
| | Julia-Puls | Julia-Menge; Beat stößt den Zoom an, langsame Drehung |
| | Burning-Ship-Kueste | das „kleine Schiff" des Burning-Ship-Fraktals |
| `fractal3D` | Mandelbulb-Orbit | Kamera umkreist den Bulb, Pitch atmet mit der Zeit |
| | Menger-Flug | Menger-Schwamm; Bass zieht die Kamera heran |
| | Quaternion-Puls | Quaternion-Julia; Beat holt die Kamera kurz nah heran |
| `domainWarp` | Nebelkammer | ruhiges Plasma — mittlere Verwerfung, langsame Drift |
| | Bass-Woge | Verwerfung hängt am Bass, Tempo an der Lautstärke |
| | Tintenstrom | starke, feine Verwerfung — marmorierter Tintenfluss |
| `fractalZoomer` | Elefanten-Tal | Endlos-Zoom auf das Elephant Valley der Mandelbrot-Menge |
| | Julia-Strudel | rotierender Julia-Zoom mit kräftiger Feedback-Schleife |
| | Beat-Schub | Zoomtempo springt je Beat und pendelt zurück |
| `lyapunov` | Zircon City | die berühmte BBBBBBAAAAAA-Ansicht (Markus/Dewdney) |
| | Atmendes Fenster | der Ausschnitt atmet langsam über sin/cos der Zeit |
| `kleinian` | Hyperbolische Blume | {7,3}-Kachelung, langsam drehend |
| | Beat-Kachelwerk | {5,4}; jeder Beat schiebt die Morph-Phase weiter |
| `strangeAttractor` | Lorenz-Schmetterling | der klassische Lorenz-Attraktor, dicht gezeichnet |
| | Clifford-Schleier | Clifford-Attraktor (a −1,4 · b 1,6 · c 1 · d 0,7) |
| | De-Jong-Gespinst | Peter-de-Jong-Attraktor (1,4 · −2,3 · 2,4 · −2,1) |
| `flame` | Feuerwirbel | Swirl-Variation, drei Abbildungen — rotierende Glut |
| | Hufeisen-Nebel | Horseshoe-Variation, drei Abbildungen (mit vier kollabiert die Bahn) |
| `reactionDiffusion` | Korallenwachstum | Gray-Scott „Coral" (feed 0,0545 / kill 0,062) |
| | Zellteilung | „Mitosis" (0,0367 / 0,0649) — teilende Zellen |
| | Wurmspur | „Worms" (0,078 / 0,061) — wandernde Bänder |
| `blur` | Abklingender Trail | strength 1 ohne round-up — das Bild klingt je Anwendung ab (Trail-Motor) |
| | Stehender Weichzeichner | strength 2 mit round-up — weichzeichnen ohne Abklingen |
| `simpleScope` | Analyzer-Balken | solid analyzer unten — die klassischen Spektrum-Balken |
| | Linien-Oszi | line scope mittig — das klassische Oszilloskop |
| | Punktwolke | dot scope mittig — die Waveform als Punktstaub |
| `customBpm` | Halbes Tempo | lässt jeden zweiten Beat durch |
| | Viertel-Beat | lässt jeden vierten Beat durch |
| | Metronom 500ms | fester Kunst-Beat alle 500 ms (unabhängig vom Audio) |
| `meshWarp` (GPU-Module S69) | Bass-Swirl | Drehung um die Mitte, Bass verstärkt — der Palette-Starter als Vorlage |
| | Tunnel-Sog | dauerhafter Sog zur Mitte mit Rotations-Drall (Feedback-Tunnel) |
| | Wellengang | laufende Sinus-Wellen quer durchs Bild (Waveform-Wasser) |
| | Fischaugen-Atmung | Fischaugen-Wölbung, die mit der Zeit/dem Bass atmet |
| | Spiegelkabinett | Kaleido-Faltung der UV — Inhalt gehört zentriert (Stimm-Befund S69) |
| `gpuParticles` (GPU-Module S69) | Fontaene | klassische Partikel-Fontäne (Schwerkraft + Fächer) |
| | Funkenregen | fallende Funken über die Bildbreite |
| | Bass-Explosion | radiale Explosion, vom Bass gezündet |
| | Nebel-Drift | große, langsame, halbtransparente Partikel |
| | Wirbelsturm | Kraftfeld-Wirbel um die Mitte (kraft()-GLSL) |
| `list` (Batch 1, S69) | Beat-Gate | Liste nur im Beat-Fenster aktiv (Slot-Akkumulator statt `time`) |
| | Bass-Blende | Listen-Blend folgt dem Bass |
| | Puls-Layer | Layer pulst periodisch über den eigenen Akkumulator |
| | AB-Wechsler | Liste schaltet je Beat ein/aus — zwei versetzte Kopien ergeben den A/B-Wechsel |
| `brightness` | Bass-Boost | Helligkeit folgt dem Bass |
| | Kanal-Atmung | R/G/B atmen phasenversetzt |
| `colorfade` | Beat-Blitz | Fader zündet je Beat und klingt ab |
| | Farbdrift | langsame Dauerdrift der Farbkanäle |
| `colorModifier` | Kontrast-Pump | Kontrast pumpt mit dem Bass (Level-Code, recompute) |
| | Gamma-Atmung | Gamma atmet langsam über die Zeit |
| `mosaic` | Beat-Kachel | Kachelgröße springt je Beat und federt zurück |
| `channelShift` | Beat-Rotation | Kanal-Rotation schaltet je Beat weiter |
| `colorClip` | Bass-Fresser | Clip-Distanz folgt dem Bass |
| `multiFilter` | Chrome-Beat | Chrome-Filter je Beat |
| `addBorders` | Puls-Rahmen | Rahmenbreite pulst mit dem Bass (weißer Rahmen) |
| `onBeatClear` | Vierer-Reset | löscht jeden vierten Beat |
| `clear` | Nachtblau-Schleier | halbtransparentes Tiefblau je Frame (weicher Trail-Deckel) |
| `bufferSave` | Echo-Speicher | Speichern/Wiederherstellen im Wechsel (dir 2, Slot 0) — Echo-Baustein |
| `pixelFilter` (Stilfilter S70) | Take-On-Me-Comic | Bleistift-Rotoskopie à la a-ha: Sobel-Kantenzug, Papier/Tinte, Beat-zitternde Schraffur |
| | Bleistift-Skizze | XDoG-Strich (Difference-of-Gaussians) + Papierkorn |
| | Posterize-PopArt | kräftige Sättigung, wenige Farbstufen — Stufenzahl atmet mit dem Bass |
| | Zeitungsdruck-Halftone | Halftone-Punkte auf 45°-Raster, Punktgröße = Dunkelheit |
| | CRT-Monitor | Tonnen-Verzerrung, Scanlines, RGB-Lochmaske, Vignette |
| | VHS-Band | Chroma-Versatz, Zeilen-Zittern, wanderndes Störband — Beat verstärkt den Glitch |
| | Kuwahara-Oelbild | Varianz-ärmster 3×3-Quadrant gewinnt — malerische Flächen, stehende Kanten |
| | Sepia-Nostalgie | Sepia-Matrix + Vignette + Korn + Projektor-Flackern |
| | Noir-Schwarzweiss | harte Kontrastkurve, Korn, Vignette — Bass drückt den Kontrast |
| | Waermebild | Helligkeit auf Thermal-Palette (blau→grün→gelb→rot→weiß), Pegel hebt die Temperatur |
| | Pixel-Art | grobe Blöcke + reduzierte Palette, Blockgröße pulsiert mit dem Beat |
| | Duotone-Neon | Helligkeit auf zwei driftende Neonfarben, Höhen hellen die Lichter |
| `dynamicMovement` (Import-Ernte S61) | Galaxien-Drall | Beat-getriebener Spiralen-Twist (aus „UnConeD — Milkyway") |
| | Wandernde Linse | horizontale Zerr-Linse, Ziel wandert je Beat (aus „amphirion — nebulous, skupers remix") |
| | Beat-Richtungsdrift | konstanter Drift, Richtung würfelt je Beat (aus „L1quid — Take the Veil") |
| | Molekuel-Raster | gefaltetes Sinus-Raster, Beat stößt den Impuls an (aus „EL-VIS — Molecules, S_KuPeRS Remix") |
| | Sinus-Flechtwerk | gekreuzte Sinus-Verwerfung, je Beat neu gestimmt (aus „amphirion — nebulous") |
| | Takt-Spirale | Rotation/Zoom im gemessenen Beat-Takt (aus „D&L — Life Is Violated") |

Die Zeilen „Import-Ernte S61" sind wörtlich aus Presets der Sammlung
`VisualsPresets` übernommen (Entscheid S60: Quelle = Sammlung via Import);
die Quell-Presets und ihre Autoren stehen je Zeile in Klammern.

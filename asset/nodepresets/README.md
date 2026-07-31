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
(`<AppData>/MyViz/nodepresets/<typkey>/`). Bei gleichem Namen gewinnt der
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

## Katalog der mitgelieferten Voreinstellungen (Stand S60)

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

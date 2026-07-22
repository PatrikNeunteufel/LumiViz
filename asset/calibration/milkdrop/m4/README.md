# Kalibrier-Presets M4 — Custom Waves / Shapes / Motion Vectors

> **Zweck:** isolierte Sichttests für die M4-Bausteine (Session 39). Vorher
> M3 (01_orientierung!) kalibrieren — alles hier baut darauf auf. Die
> Basis-Waveform ist in diesen Presets per `fWaveAlpha=0.001` praktisch
> ausgeblendet, damit nur der Prüfling sichtbar ist.
> Stellschrauben: `drawCustomWaves`/`drawCustomShapes`/`drawMotionVectors`
> in `src/visualizers/MilkdropVisualizer.cpp`.

## 01_wave_geometrie.milk — per_point-Mapping (audio-unabhängig)

**Bild:** eine grüne, STATISCHE Sinuslinie über die volle Breite (Geometrie
kommt rein aus `sample`, Audio egal).
**Erwartung:** exakt EINE volle Sinusperiode, beginnt links (sample=0) auf
Mittelhöhe, Amplitude ±25 % der Bildhöhe, keine Bewegung, keine Lücken.
**Abweichung:** horizontal gestaucht/gespiegelt → sample/x-Mapping;
vertikal gespiegelt → y-Formel (`py*-2+1`); Amplitude falsch → NDC-Mapping
(InvAspect) in `drawCustomWaves`.

## 02_wave_pegel.milk — kWavePortScale-KALIBRIERUNG (Waveform)

**Bild:** weiße Linie, y folgt dem rohen Audiosignal (value1) bei scaling=1.
**Erwartung (Referenzverhalten):** bei normal lauter Musik schlägt die Linie
etwa **±¼ der Bildhöhe** aus; bei Stille flach; Vollaussteuerung erreicht
knapp den halben Bildschirm, clippt aber nicht dauerhaft.
**Abweichung:** → `kWavePortScale` in `drawCustomWaves` proportional anpassen.
Zu zahm → erhöhen, übersteuert → senken. **Guten Wert bitte zurückmelden!**
*(Kalibrier-Runde 1: 128 → 192, zusammen mit dem Aspect-Fix — bitte neu bewerten.)*

## 03_wave_spektrum.milk — kSpecPortScale-KALIBRIERUNG (Spektrum)

**Bild:** oranges Spektrum am unteren Rand (links Bass, rechts Höhen),
Ausschläge nach OBEN.
**Erwartung (Referenzverhalten):** Bass-Ausschläge links deutlich sichtbar
(~10–40 % Bildhöhe bei normaler Musik), Höhen rechts kleiner, nichts clippt
dauerhaft am oberen Rand, bei Stille flache Linie bei 90 % Höhe.
**Abweichung:** → `kSpecPortScale` anpassen. **Guten Wert bitte zurückmelden!**
*(Kalibrier-Runde 1: 32 → 8 — Bass lief oben aus dem Bild; bitte neu bewerten.)*

## 04_shape_verlauf.milk — Shape-Fan, Farbverlauf, Border

**Bild:** statisches Sechseck in der Mitte, Radius 0.3.
**Erwartung:** Zentrum satt ROT, weich nach GRÜN (halbtransparent) am Rand
auslaufend (Gouraud-Verlauf); weiße, DICKE Kontur (thickOutline) exakt auf
dem Rand, geschlossen. Sechseck gleichseitig (nicht elliptisch verzerrt),
eine Ecke bei 45° rechts oben (Original-Winkelbasis +π/4).
**Abweichung:** Verlauf falsch herum → Center/Edge-Farben im Fan; Ellipse →
Aspect auf cos-Term; Kontur offen/versetzt → Ring-Schluss (`v[sides+1]=v[1]`).

## 05_shape_instanzen.milk — num_inst + instance-Variable

**Bild:** 8 gelbe Rauten gleichmäßig auf einem Kreis (Radius 0.3) um die Mitte.
**Erwartung:** exakt 8 Stück, gleichmäßige Abstände (45°-Schritte), erste Raute
rechts (instance=0 → Winkel 0). Der Ring darf im Breitbild **elliptisch** sein —
Skript-x/y sind Bildschirm-Anteile, das Original verhält sich genauso (Presets
korrigieren selbst mit aspecty, wenn sie runde Ringe wollen). Die Rauten SELBST
müssen quadratisch sein (rad-Aspect-Korrektur).
**Abweichung:** alle 8 übereinander → `instance` wird nicht pro Instanz
gesetzt; weniger als 8 → num_inst-Schleife.

## 06_shape_textur.milk — textured Shape (Vorframe-Sampling)

**Bild:** großes 32-Eck (fast Kreis) in der Mitte, textured mit tex_zoom=0.5;
drumherum läuft ein normales Feedback (Kreis-Wave, leichter Zoom+Warp).
**Erwartung:** IM Shape erscheint der Bildinhalt (vergrößert, tex_zoom<1 =
reingezoomt) — eine Art Lupe/Glaslinse, die das Feedback zeigt. Nicht schwarz,
nicht einfarbig. Inhalt aufrecht (nicht gespiegelt gegenüber dem Umfeld).
**Abweichung:** schwarz → Textur-Bind; gespiegelt → tv-Flip im Shape-UV;
Inhalt „hinkt" minimal nach → bekannt (PORT: sampelt Vorframe, ok).

## 07_motion_vectors.milk — MV-Gitter + Rückverfolgung

**Bild:** 12×9 weiße Vektorlinien über blassem Zoom-Feedback (zoom=1.03).
**Erwartung:** gleichmäßiges Gitter; alle Linien zeigen radial **nach innen
Richtung Mitte** (die Rückverfolgung zeigt, WOHER der Pixel kam — bei
Zoom-nach-außen liegt die Herkunft weiter innen). Länge wächst zum Rand hin.
In der exakten Mitte ~Punktlänge (Mindestlänge).
**Abweichung:** Linien zeigen nach außen → reversePropagate-Richtung;
Gitter verschoben → mv_dx/dy-Offsets; einzelne Riesenlinien → Mindestlängen-
Klemmung.

## 08_t_und_q_vertrag.milk — t1–t8/q-Snapshots im Zusammenspiel

**Bild:** violette Wellenlinie, beginnend bei x=0.25 (aus q1, per_frame_init),
Grundhöhe 30 % (aus t1, Wave-Init), leichtes Auf/Ab im Sekundentakt (q2).
**Erwartung — der eigentliche Test:** die Linie bleibt STABIL auf ~30 % Höhe
(vom oberen Rand gemessen), obwohl der Wave-per_frame-Code t1 jeden Frame um
+10 hochzählt. Wenn die Linie **nach unten/oben davonläuft oder verschwindet**,
akkumuliert t1 → t-Snapshot-Restore kaputt. Wenn sie nicht bei x=0.25 beginnt
→ q1-Init-Snapshot kaputt. Wenn das Auf/Ab fehlt → q2 kommt nicht an.
**Hinweis:** Läuft die Linie über den Bildrand, erscheinen am gegenüberliegenden
Rand blasse Kopien — das ist Texture-Wrap (`bTexWrap=1`, Original-Verhalten),
kein Artefakt.

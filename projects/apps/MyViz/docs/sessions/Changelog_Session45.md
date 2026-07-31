# MyViz — Changelog Session 45 (2026-07-24)

> **Thema:** Die Treue-Arbeit gegen das Original bekommt ein Fundament: eine
> deterministische Test-Audio-Suite, minimale Prüf-Presets mit eingefrorenen
> Übersetzungs-Zwillingen, und ein Prüfplan für die Seite-an-Seite-Sichtung.
> Auf diesem Fundament sind zwei geplante Befunde gefallen — und ein dritter,
> der erst durch das Hinsehen gefunden wurde.

## Neu: das Kalibrier-Fundament

**Fünfzehn deterministische Testsignale** (außerhalb des Repos, `TestAudio/`):
Stille, reine Töne, Bass in zwei Stärken, ein Frequenz-Durchlauf, Kick-Muster,
Stereo-Wechsel, Gegenphase, Rauschen, eine Amplituden-Rampe und ein Stück
Pseudo-Musik — jedes Signal mit festem Zweck („was prüft man damit"), als
WAV-Master plus MP3, dazu eine Playlist. Damit sind Messungen wiederholbar:
gleiche Datei, gleiches Bild.

**22 minimale Prüf-Presets** (`asset/calibration/avs/`), je eines pro
Befund-Cluster — echte `.avs`-Dateien, die auch im Original-AVS laufen. Zu jedem
gehört ein **eingefrorener Übersetzungs-Zwilling**: die von uns übersetzte
Effektkette, festgeschrieben als Datei. Ändert sich die Übersetzung eines
Presets, schlägt die Prüfung an — gewollt bei Fixes, Alarm bei Regressionen.
Der Prüfstand fing noch am selben Tag seinen ersten Fehler (ein
Spektrum-Schalter wurde falsch geschrieben).

## Behoben

**Movement rechnet Polarkoordinaten jetzt im Pixel-Raum des Originals.**
Abstand und Winkel eines Punkts werden wie im Original aus Pixel-Offsets
gebildet, nicht aus normierten Koordinaten. Sichtbar am Prüfbild: aus einer
verschmierten Ellipse wurde ein formstabiler Kreisring.

**Set Render Mode wird beim Listen-Eintritt gesichert und am Ende
wiederhergestellt** — wie im Original. Und der eingestellte Linien-Mischmodus
wirkt jetzt auf *allen* Zeichenwegen: den einfachen Scopes, den Punktrastern,
Moving Particle und Timescope. Mehrere davon mischten bisher hart additiv, wo
das Original „ersetzen" oder den eingestellten Modus nimmt.

**Zufall ist jetzt wirklich Zufall.** Alle Skript-Engines starteten mit
demselben festen Startwert — zwei „zufällige" Scopes im selben Preset zogen
damit exakt dieselbe Folge. In einem Preset löschten sich zwei Scopes deshalb
gegenseitig pixelgenau aus und das Bild blieb schwarz. Jetzt bekommt jede
Engine eine eigene Note in den Startwert; das Preset rendert (am Gerät
bestätigt).

**Werkzeug-Fixes:** Screenshots unterscheiden `.avs` und Zwilling im Dateinamen
(vorher überschrieb einer den anderen) und werden ohne Alpha-Kanal gespeichert —
durchsichtige Pixel erschienen im Viewer als „weiße" Phantom-Linien und haben
eine lange Geisterjagd gekostet.

## Angefangen

**Die Seite-an-Seite-Runde** (Original neben MyViz, gleiche Musik, gleiche
Presets) hat einen festen Prüfplan mit sieben Positionen bekommen. Die erste
In-App-Sichtung lieferte prompt den Zufalls-Befund oben. Notiert und offen: das
Original zeichnet Scopes im Seitenverhältnis des Fensters (ein Kreis-Skript
ergibt bei 4:3 eine Ellipse), wir zeichnen aspektquadratisch — Urteil nach der
nächsten Sichtung.

## Stand

- Tests: **413 von 413 grün**, alle Bauarten fehlerfrei
- Übersetzungs-Zwillinge: **22 von 22 stimmen**

# Changelog — Session 53 (2026-07-27)

> Aus einer kleinen Vorgabe wurde der größte Bedien-Ausbau seit langem: **jeder**
> Knoten der Effektkette hat jetzt Voreinstellungen, alle seine Felder sind
> erreichbar, festgenagelte Werte sind Regler geworden, und fast jeder Knoten
> lässt seine Regler per Formel berechnen. Dabei fielen drei Befunde an, die
> ältere Notizen berichtigen.

## Bedienung

- **Jeder Knoten hat Voreinstellungen.** Ganz oben im Eigenschafts-Editor steht
  die Zeile **Preset** — für Effect List genauso wie für Movement, SuperScope
  oder eine Host-Gruppe. Auswählen lädt, **Save as…** speichert unter einem
  Namen, 🗑 löscht (nur Eigenes).
- **Beim Speichern lassen sich Felder abwählen.** Der Dialog zeigt jedes Feld des
  Knotens mit einem Häkchen; was du abwählst, kommt nicht in die Datei — und
  bleibt beim Laden unangetastet. So speicherst du „nur die Formeln, nicht die
  Farbe".
- **Die SuperScope-Figuren sind Voreinstellungen geworden.** Das Dropdown
  „Figure" ist entfallen; Spiral, Butterfly, Hypocycloid und die anderen stehen
  in derselben Preset-Liste wie alles andere. Da sie nur die Formeln enthalten,
  behältst du beim Laden deine Farbtafel und Linienbreite. (Nebenbei: eine Figur
  hieß in der alten Liste „Unknown" — `Starburst` fehlte in der Namenstabelle.)
- **Fast jeder Knoten rechnet seine Regler auf Wunsch selbst aus.** Unten im
  Editor stehen **Init**, **Frame** und **Beat**; darin lassen sich die eigenen
  Parameter berechnen, etwa bei Rotating Stars `points = 5 + bass*8`. Ein leeres
  Feld kostet nichts.
- **`Metaballs 3D` und `Tentacles 3D` sind einstellbar** (die Startvorgabe): beide
  mit Kugel-/Tentakelzahl, Größe, Tempo, Blend und Farbtafel — und beide jetzt
  auch von Hand einfügbar.
- **File → New Effect Chain** (`Ctrl+Shift+N`) beginnt mit einer leeren Kette.
- **Einstellungen → Panels:** *Benutzerdaten öffnen* (dort liegen deine eigenen
  Voreinstellungen) und ein **Bilder-Suchordner**, den der Import als letzte
  Zuflucht durchsucht.
- **Bilder lassen sich auswählen.** Picture, Picture II, Texer und Texer II
  zeigten bisher nur, *ob* ein Bild eingebettet ist — jetzt gibt es
  **Choose…**/**Clear**. Das Bild wird eingebettet, die Kette bleibt für sich
  allein lauffähig.

## Einstellbar, was vorher nicht einstellbar war

- **Farbtafeln** bei Simple Scope, Rotating Stars, Oscilliscope Star und Ring —
  die gab es im Editor überhaupt nicht. Dot Grid zeigte nur den ersten Eintrag.
- **Convolution:** das ganze 7×7-Gitter (mit Knopf „Identity"), vorher
  „imported, read-only".
- **Color Map:** die Gradient-Stützstellen (Position + Farbe, hinzufügen/
  entfernen), ebenfalls vorher nur Text.
- **Rotating Stars hatte außer Farben keinen einzigen Parameter.** Jetzt: Zacken,
  Sprungweite (2 = Pentagramm), Sternzahl, Tempo, Bahnradius, Grundgröße,
  Audio-Verstärkung, Spektralband.
- Ebenso freigemacht: Osc Star, Osc Ring, Metaballs, Tentacles, FyrewurX,
  Triangle (gefüllt/Drahtgitter) sowie Dot Plane, Bass Spin und Moving Particle.
- Kleinkram, der fehlte: `Blur`-Rundung, `Colorfade`-Beatdauer, `Custom BPM`
  „erste N Beats überspringen", `Movement`-Builtin-Umlegung, Texer-II-Optionen
  und der Beat-Slot bei Texer II und Triangle.

## Damit die Referenztreue sichtbar bleibt

Drei Knoten bilden AVS zeilengenau nach — bei ihnen **sind** die Zahlen die
Referenz. Weicht dort ein Wert ab, hängt ein **⚠** an der Beschriftung und der
Tooltip nennt den Originalwert. Bei allen anderen ist Verstellen normal.

Kein einziger neuer Regler verändert das Bild, solange er auf seiner Vorgabe
steht: Dot Plane und Moving Particle stehen weiterhin auf 0,000 gegenüber der
Referenz, gemessen vor und nach dem Umbau.

## Befunde

- **`Dot Fountain` ist keine Portierung.** Das Original ist ein 30×256-Gitter —
  eine rotierende Höhenwand — wir zeichnen 400 freie Partikel. Der Messwert sagt
  trotzdem „gut" (0,002), weil beide Bilder überwiegend schwarz sind; die
  Bildmontage zeigt es sofort. Die Abdeckung führt den Knoten jetzt als ◐.
- **Zwei Bilanzen waren zu optimistisch notiert:** die Modul-Matrix steht bei
  **36/41** (nicht 37), die Modul-Sonden bei **78/80** (nicht 79). Beide Male
  wurde am Vorstand nachgewiesen, dass es Altbestand ist.
- **Movement kann keine zeitabhängige Bewegung.** Es baut eine statische Tabelle
  (wie AVS `r_trans`) — ein Beat-Zähler bewegt dort nichts. Der Editor sagt das
  jetzt an und verweist auf **Dynamic Movement**, das je Frame rechnet.

## Bekannte Grenzen

Die neuen Felder sind noch nicht Feld für Feld im Betrieb geprüft — dafür
entstehen in Session 54 Test-Presets für jedes Modul und jedes Feld. Ebenfalls
geplant: ein Tooltip an jedem Feld und mitgelieferte Voreinstellungen für alle
Module.

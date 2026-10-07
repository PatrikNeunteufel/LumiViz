# Changelog Session 100 (2026-10-07)

> **Die ganze Komposition hat eine eigene Pegelkurve — die Zeile MASTER.** Sie
> liegt unter dem Lineal und lässt sich bedienen wie die Kurve eines Clips.
> Dazu: Kanäle haben eigene Farben, lassen sich verschieben und nach Art
> ordnen; Kompositionen und die App selbst haben ein Icon. Der Sichttest der
> Session 99 ist ausgewertet (31 bestanden, 3 mit Bemerkung, 1 durchgefallen,
> 17 nicht gespielt), der Befund daraus ist behoben.
> **Im Programm gesehen** ist von dieser Session nichts: alles ist gebaut und
> per Test belegt.

## Neu

- **Die Zeile MASTER** (Composer, direkt unter dem Lineal): die Pegelkurve der
  ganzen Komposition als ein Balken über ihre Länge. Anklicken, die Griffe von
  *Fade in* und *Fade out* ziehen, per Rechtsklick eine Form oder ein Preset
  wählen, Stücke anlegen (*Add shape here*), den Curve Editor öffnen, den
  Grundpegel setzen — alles wie an einem Clip, auf Bild und Ton.
- **Ein Stück der Zeile MASTER liegt an fester Zeit:** wird die Komposition
  länger oder kürzer, bleibt es an seiner Stelle. Hinter dem Ende wirkt es
  nicht und ist wieder da, sobald die Komposition wieder so lang ist.
- **Das feste Ende ziehen:** ist *Fixed end* angehakt, lässt sich die rechte
  Kante des Balkens der Zeile MASTER ziehen.
- **Kanalfarben:** jeder Kanal trägt seine eigene Farbe — Bild-Kanäle kühle
  Töne, Audio-Kanäle warme. Rechtsklick auf die Zeile → *Channel color...* und
  *Reset channel color*. Die Farbe gehört dem Kanal und steht in der Datei.
- **Kanäle verschieben:** am Kanalnamen ziehen oder Rechtsklick → *Move channel
  up* / *Move channel down*. Die Reihenfolge der Bild-Kanäle ist die
  Reihenfolge, in der gemischt wird: der oberste liegt im Bild zuunterst.
- **Kanäle nach Art ordnen:** Rechtsklick → *Sort channels* → *Visuals first* /
  *Audio first*. Innerhalb der Art bleibt die Reihenfolge, das Bild ändert
  sich nicht.
- **Stück-Preset Late fade in:** ein Clip läuft unsichtbar an, blendet später
  weich ein und bleibt oben. Als Stück an den Clip-Anfang gelegt; ohne
  *Flipped*.
- **Ein Icon für Kompositionen** (`.lvcomp`): ein Fächer aus drei
  LumiViz-Icons mit Zeitstrahl und roter Abspielmarke. Der **Effect Browser**
  zeigt Kompositionen damit, ein Doppelklick öffnet sie; im Panel Composer
  steht es vor dem Dateinamen.
- **Das Icon der App:** `LumiViz.exe` trägt das LumiViz-Icon als Ressource
  (Explorer, Verknüpfungen, Taskleiste).
- **Benutzerhandbuch §16 „Dateitypen und ihre Icons":** alle Dateitypen mit
  ihrem Icon als Bild.
- **Skript `asset\img\logo\icons\register_file_icons.ps1`:** trägt die
  Datei-Icons für den angemeldeten Benutzer im Windows-Explorer ein oder nimmt
  sie wieder heraus. Ohne Schalter zeigt es nur an.

## Geändert

- **Panel Player nach „Composer off":** war das Panel beim Ausschalten nicht
  zu sehen (ein anderer Reiter davor, geschlossen), zeigte es nach dem
  Einblenden weiter „Composer - <Song>". Jetzt zeigt es „No track loaded".
- **Die Keile im Lineal** für *Fade in* / *Fade out* sind entfallen — die Zeile
  MASTER zeigt die Blenden.
- **Überdecken sich *Fade in* und *Fade out*** (zusammen länger als die
  Komposition), teilen sie den Platz wie am Clip. Bisher galt an jeder Stelle
  der kleinere der zwei Werte.
- **Audio-Kanäle** haben nicht mehr alle denselben Farbton; **Bild-Kanäle** ab
  dem zweiten haben andere Farben als bisher.
- **Das Panel Composer** braucht eine Zeile mehr Höhe (die Zeile MASTER).

## Entschieden

- **Pegelkurve der Komposition:** alles, was ein Clip hat, in einer eigenen
  Zeile (F7 c); ein Stück liegt an fester Zeit (F7-1 a).
- **Spätes Einblenden:** ein mitgeliefertes Preset und ein Absatz im Handbuch,
  kein neuer Wert am Clip.
- **Kanalfarben:** zwei Farbreihen, die Art bleibt an der Farbe erkennbar; die
  Farbe gehört dem Kanal (K1 b).
- **Kanäle verschieben:** eine Reihenfolge — sie bleibt die Mischreihenfolge
  (V1 a).
- **Sortieren:** nur nach Art, innerhalb der Art bleibt die Reihenfolge (S1 a).
- **Icon der Komposition:** im Effect Browser und im Panel Composer (I1 a); der
  Fächer (I2), mit Zeitstrahl und Abspielmarke (I2b a).
- **Ziel:** alle Datei-Icons sollen im Explorer erscheinen, vorerst Windows.
- Aus dem Sichttest S99, was gesetzt war und gebilligt ist: im selben Kanal
  liegt der Clip des Anfangs über dem des Endes; Sprung ins hintere Fenster mit
  beiden Stellen, ins vordere ohne das Ende; Halt im Fenster hält beide; Loop
  aus im Fenster spielt das Ende aus; Vorwärmen und „kein Sprung" am Umschlag
  bei jeder Schleife; die Karaoke-Ebene blendet nicht mit.

## Technik

- **Modell:** `Komposition::einblendeForm` / `ausblendeForm` / `grundpegel` /
  `gesamtStuecke` (`GesamtStueck`), `kurveAlsClip` / `setzeKurveAusClip` —
  `blendeBei` rechnet wie die Pegelkurve eines Clips über [0, Länge).
  `Kanal::farbe`, `reihenFarbe`, `freieFarbe`, `vergibFarben`, `farbeVon`.
  `Bearbeiten::verschiebeKanal`, `platzNachVerschieben`, `sortiereNachArt`.
- **Datei:** `fade { inShape, outShape }`, `level { base, shapes [ at, width,
  flipped, shape ] }`, `channels[].color` als `#rrggbb` — jeweils nur, wenn es
  von der Vorgabe abweicht; ein älterer Stand überliest sie.
- **Zeitachse:** die Zeile MASTER ist ein Clip über die ganze Länge mit dem
  „Kanal" `kMaster` — `clipIn` / `schreibeClip`, sodass Zeichnen, Griffe,
  Rechtsklick und Curve Editor die Wege des Clips gehen. `kKanaeleOben`: die
  Kanäle beginnen eine Zeile tiefer.
- **Naht N5, Anzeige, sechste Stelle** (`da5339f`): `PlayerPanel::onActivate`
  geht ohne Titel im Player und ohne laufenden Song des Composers in den
  Ruhezustand.
- **Naht N8 wächst** (`ef57574`): `PresetTypeIcons.hpp` (Endung `lvcomp`),
  `ImportBrowserPanel` (`Type_Komposition`, Doppelklick meldet
  `OpenCompositionEvent`).
- **Exe-Icon:** `projects/apps/LumiViz/main/LumiViz.rc`, eingetragen in
  `main/Source.cmake` (nur Windows). Am Build-System nichts geändert.
- **Icons:** `lumiviz_lvcomp.ico`, `lumiviz_app.ico` (erzeugt in
  `build_lvfx2_icons.py`); `build_previews.py` legt die Vorschaubilder nach
  `docs/bilder/handbuch/icons/`.
- **943 Cases** im Zweig `composer` (Start der Session: 939).
- Doku: `Composer_Konzept.md` 0.43.1 → 0.52.1, `Benutzerhandbuch.md` 1.98.0 →
  1.106.0, `Offene_Punkte.md` 1.95.0 → 1.105.0.

## Offen

- **Im Programm nicht gesehen:** alles aus dieser Session — Sichttest
  `Sichttests_S100.md` (70 Punkte).
- **Befund A2 aus dem Sichttest S99:** behoben ist der Fall „Panel Player beim
  Ausschalten verdeckt". Ob es am 06.10. dieser Fall war, ist nicht belegt.
- **Von mir gesetzt, ohne Entscheid:** Name, Ort und Höhe der Zeile MASTER; die
  zwölf Kanalfarben; Zurücksetzen gibt die erste freie Farbe der Art; ein Stück
  am Rand rückt hinein; die Texte des Curve Editors sprechen an der Zeile
  MASTER weiter vom Clip.
- **Wunsch:** neben der Zeile MASTER ein **Audio-Master** und ein
  **Visual-Master** — etwa, um die Musik für eine Durchsage kurz stumm zu
  schalten, ohne das Bild anzufassen.
- **Zoom und Schwenken der Zeitachse**, mit Reset über Ctrl+0.
- **Icons im Explorer:** das Skript ist nur im Anzeige-Modus gelaufen; ein
  Doppelklick im Explorer öffnet die Datei noch nicht in LumiViz (die App
  wertet ihre Startargumente nicht aus). Taskleiste und Fenster-Icon der App
  sind nicht geprüft.
- **Handbuch neben der Exe:** dort liegt kein Bilderordner, die Bilder
  erscheinen nur im Repo.
- Aus dem Sichttest S99 nicht gespielt: *Visuals overlap themselves* (Abschnitt
  G), Text-Vorlage, Wechsel am Kreuzungspunkt, Kopieren ohne Edit, Klick in der
  Überlagerung, Neustart ohne Resume, Host Switcher.
- **Nicht gemessen:** Bildzeit und Grafikspeicher von *Visuals overlap
  themselves* kurz vor dem Umschlag.
- Weiteres in `Offene_Punkte.md` §7.

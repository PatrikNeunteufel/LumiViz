# Changelog Session 92 (2026-10-03)

> **Erste Sichttest-Runde zum Lyrics-Block, und der Trail als eigenes Preset.**
> Vier Befunde aus dem Sichttest sind behoben, das Panel Lyrics Sync hat einen
> einstellbaren Vorlauf, eine Sprungleiste und ein Kontextmenü bekommen. Die
> Trail-Liste der Karaoke-Ebene lässt sich für sich als Preset halten — acht
> liegen bei — und in eine Effektkette übernehmen.

## Neu

- **Trail-Presets (Karaoke-Panel, Reiter Lyrics Trail):** die Zeile **Trail
  preset** hält die Trail-Liste als Ganzes fest. Eine Auswahl ersetzt nur den
  Trail — Schrift, Farben und Lage der Zeile bleiben, **Trail on** ebenso.
  Acht mitgelieferte: **Drift Up** (das bisherige Aussehen), **Fall Back**,
  **Zoom Burst**, **Swirl**, **Smoke**, **Echo**, **Wave**, **Melt**. **Save
  As…** legt eigene an, **Delete** löscht eigene. Weicht die Liste vom
  gewählten Preset ab, steht **(unsaved) – Name** im Dropdown.
- **Trail-Presets in der Effektkette:** Rechtsklick im Baum des Panels Effect
  Chain → **Add trail preset ▸ Name** fügt das Preset als Effect List ein
  (mischt Additive); **Save as trail preset…** an einer Effect List speichert
  sie als eigenes Trail-Preset. Eine Effect List mit einem Lyrics Trail darin
  trägt im Baum ein eigenes Icon (das LumiViz-Icon mit einem Kometen im Kreis).
- **Lyrics / Karaoke — Word spacing (%):** Abstand zwischen den Worten in
  Prozent des Leerzeichens der Schrift (50–400). Gilt für jede Zeile, auch die
  Vorschau. Ein weiterer Abstand gibt dem vergrößerten Wort Luft zu seinen
  Nachbarn, vor allem mit **Word layout** Overlay. Im Skript: `wordspacing`.
- **Lyrics Sync — Pre-roll:** wie weit vor einer Zeile der Doppelklick ansetzt
  (0–10 s, Vorgabe 1,5 s), über den Neustart gemerkt.
- **Lyrics Sync — Sprungleiste:** unter der Wellenform, über den ganzen Titel.
  Klicken oder Ziehen springt dorthin — auch an Stellen, an denen noch keine
  Zeile eine Zeit hat.
- **Lyrics Sync — Kontextmenü:** Rechtsklick auf eine Zeile oder auf ein Wort
  im Streifen. Das Menü zeigt nur, was für die Zeile bzw. das Wort geht
  (springen, klopfen, verschieben, Zeit löschen, Stimme, Pre-roll, Words,
  Follow playback, Undo / Redo).

## Geändert

- **Lyrics Sync — Enter und T** klopfen im ganzen Panel, nicht mehr nur in der
  Zeilenliste — auch nach einem Klick in den Wort-Streifen, auf **Words** oder
  einen Schieber. Eingabefelder (**Track offset**, **Pre-roll**, Zeit per F2)
  behalten ihre Tasten.
- **Karaoke-Preset per Doppelklick:** ein Doppelklick auf ein `.lvkar` im
  Effect Browser schaltet die Karaoke-Ebene ein, wenn sie aus ist.
- **Effect Browser:** `current.lvkar` (die laufende Karaoke-Ebene) steht nicht
  mehr in der Liste — sie ist kein Preset.
- **Ctrl+K** (*Karaoke On / Off*) ruht, solange der Cursor in einem Textfeld
  oder im Skript-Editor steht.
- **Benutzerhandbuch, Skript-Beispiele:** der Puls auf den Beat klingt mit
  0.93 statt 0.85 ab; neues Beispiel „eine Zeile auf jeden vierten Beat".

## Behoben

- **Lyrics Sync:** Enter und T gingen ins Leere, sobald der Fokus nicht in der
  Zeilenliste lag (Befund aus dem Sichttest).

## Unter der Haube

- Ein Test prüft jetzt jedes Skriptbeispiel des Benutzerhandbuchs: es muss
  sich übersetzen lassen und als „in einem Test belegt" oder ausdrücklich als
  „nur übersetzt" eingeordnet sein (20 Beispiele, 14 belegt).

## Noch nicht im laufenden Programm gesehen

Abgenommen im Sichttest sind aus S90/S91: Trail zeigt das Bild, das stand
(A1), Word scale mit Reflow (D1), Schieber Speed (J1, J3). Alles oben aus
dieser Session ist per Test, an gerenderten Bildern und Panel-Schnappschüssen
geprüft, aber nicht im laufenden Programm — besonders die acht Trail-Presets
mit echter Schrift und Musik, eine Trail-Liste im Bild einer Effektkette, das
Icon im Baum, die Kontextmenüs und Dialoge, das Ziehen an der Sprungleiste
bei laufendem Ton und Word spacing mit Overlay.

# Changelog Session 109 (2026-10-11)

> **Der Loop lässt sich bauen.** Aus dem Panel, das in Session 108 nur laden
> und spielen konnte, ist ein Arrangement geworden: Kanäle, Clips, ein
> Drum-Editor mit Schritt-Raster. Ein Loop liegt außerdem als Clip im Composer
> und zählt dort als Song. **Im Programm hat es noch niemand gesehen oder
> gehört** — geprüft ist es mit Unit-Tests und mit sehr leisem Ton aus dem Test.

## Neu

- **Das Panel Loop als Arrangement:** Kanäle untereinander, eine Achse in
  Takten, Clips als Blöcke. Ein Rechtsklick legt einen leeren Drumkit-Clip
  (4 Takte) an; Clips lassen sich verschieben — auch in einen anderen Kanal —,
  an den Kanten in ganzen Mustern verlängern und kürzen, kopieren, löschen.
  Kopien bleiben **verbunden** und tragen dieselbe Farbe. Kanäle anlegen,
  umbenennen, aus- und einschalten, löschen.
- **Datei-Zeile:** **New**, **Load…**, **Save**, **Save As…**; ein Stern zeigt
  Ungespeichertes. **BPM** als Feld. Jede Änderung geht über **Undo / Redo**
  und ist beim Spielen sofort zu hören.
- **Zoomen und Rollen wie im Composer:** Ctrl+Mausrad, Alt+Mausrad,
  Shift+Mausrad, mittlere Maustaste, **Fit**.
- **Der Drum-Editor** (Doppelklick auf einen Clip oder *Edit base*): ein
  eigenes Fenster mit dem Schritt-Raster. Je Spur Stumm, Name, Klang (aus dem
  Kit oder eine eigene Datei), Lautstärke und Panorama; ein Klick schaltet
  einen Schritt, Ziehen malt; darunter die Lautstärke je Schritt. **Bars**
  stellt die Länge des Musters, **Play pattern** spielt nur dieses Muster,
  **Peak** zeigt rot, wenn der Loop übersteuert. Das Raster des ganzen Loops
  (Schläge je Takt, Schritte je Schlag, Swing) steht im selben Fenster.
- ***Make new base and edit*:** löst einen Clip von seinen Kopien — er
  bekommt ein eigenes Muster.
- **Der Audio Browser zeigt Loops** (`.lvloop`) hinter den Songs; ein
  Doppelklick lädt den Loop ins Panel Loop.
- **Ein Loop als Clip im Composer:** eine `.lvloop` fällt in einen Audio-Kanal
  wie ein Song. Der Clip ist einen Durchlauf lang und wiederholt sich, wenn
  man ihn länger zieht. Er zählt als **laufender Song** — das Panel Player
  zeigt seinen Titel.
- **Tags für Loops:** Titel, Künstler, Album, Jahr, Genre, Nummer und Songtext
  stehen in der `.lvloop`, wie bei einem Song in der Datei. Knopf **Tags…** im
  Panel Loop.
- **Panel Loop und Composer hängen zusammen:** hat das Panel dieselbe Datei
  offen, spielt der Clip im Composer, was dort steht — auch Ungespeichertes.
- **Benutzerhandbuch §16 „Loop"**; die bisherigen §16 und §17 sind §17 und §18.

## Geändert

- Das Panel Loop ist breiter und höher als in Session 108 und rollt selbst.
- Die Zeitachse des Composers teilt sich Maßstab, Zoom und Rollen jetzt mit
  der Achse des Loop-Panels. An ihrem Verhalten und Aussehen ändert das
  nichts: 33 Bilder der Zeitachse vor und nach dem Umbau sind bytegleich.

## Für Entwickler

- Konzept mit allen Entscheiden, Regeln und Messungen:
  [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.12.0 — neu §4c (Arrangement),
  §4d (Drum-Editor), §4e (Clip im Composer), §6a (Messung gegen die Wanduhr).
- Neue gemeinsame Basis `lumi::ui::Achse` (`UI/widgets/Achse.hpp`) für
  `composer::Zeitachse` und `loop::Taktachse`.
- `composer/Tonfuehrung` spielt Loops über je einen Klangerzeuger im Mixer;
  `composer::istLoop`, `loop/Offen.hpp`, `loop::LoopTags`.
- [Audio_System.md](../audio/Audio_System.md) 1.3.0: §6a beschreibt den Mixer
  und wer den Ton hat.
- Neues Ereignis `LoadLoopEvent`.
- Sichttest-Vorlage `asset/sichttest/composer_K_loop.lvcomp` mit
  `asset/sichttest/loop/beat_A.lvloop`.

## Offen

- Der erste Blick ins Programm und der erste Hörtest.
- Beim Beenden fragt das Panel Loop nicht nach Ungespeichertem.
- Ein eigenes Icon für `.lvloop`; das Kit im Release-Paket; Instrument-Clips.
- Debug-MSVC nicht gebaut; der volle Testlauf lief nicht unter clang.

## Tests

- 1025 Cases (23 neu), voller Lauf 1025 / 1025, 0 Skips (Testing-MSVC); die
  Fälle zu Loop, Achse und Tonführung auch unter Release-clang grün. Zwei der
  neuen Fälle spielen zusammen rund neun Sekunden sehr leisen Ton.

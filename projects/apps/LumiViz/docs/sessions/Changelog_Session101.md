# Changelog Session 101 (2026-10-07)

> **Der Composer bekommt drei weitere Master-Zeilen — VISUAL, AUDIO und
> KARAOKE — und es gibt einen Mute für spontane Durchsagen.** Dazu lässt sich
> LumiViz mit einer Datei starten, und es läuft immer nur eines. Ein Befund
> aus dem Programm ist behoben (das Panel Karaoke lud bei laufendem Composer
> nichts), der erste Teil des Sichttests der Session 100 ist ausgewertet (23
> bestanden, 1 mit Bemerkung, 0 durchgefallen, 46 nicht gespielt).
> **Im Programm gesehen oder gehört** ist von dieser Session nichts: alles ist
> gebaut und per Test belegt.

## Neu

- **Die Zeilen VISUAL und AUDIO** (Composer, unter der Zeile MASTER): je eine
  Pegelkurve nur für das Bild und nur für den Ton, bedienbar wie MASTER —
  Blenden am Griff, Formen, Stücke an fester Zeit, Curve Editor, Grundpegel.
  Die Kurven werden malgenommen: Bild = Clip × MASTER × VISUAL, Ton = Clip ×
  MASTER × AUDIO. Damit geht etwa „Musik für einen Moment weg, das Bild läuft
  weiter" oder „das Bild vor dem Ton einblenden".
- **Die Zeile KARAOKE:** die Pegelkurve der Karaoke-Ebene — wie stark der
  Songtext über dem Bild liegt, über der Zeit. Sie steht für sich: MASTER und
  VISUAL blenden den Songtext nicht mit.
- **Der Haken Karaoke im Panel Composer:** derselbe Schalter wie *Karaoke on*
  im Panel Karaoke, *View → Karaoke* und Ctrl+K — wo immer geschaltet wird,
  zeigen es alle.
- **Mute für Durchsagen:** ein Knopf **Mute** rechts neben dem
  Lautstärke-Regler im Panel Player und derselbe im Panel Composer. Er senkt
  den Ton weich ab und holt ihn weich zurück — bei der Playlist ebenso wie
  bei laufender Komposition. Der Lautsprecher-Knopf bleibt das einfache,
  sofortige Stummschalten.
- **Mute-Profile:** wie der Ton weggeht und wiederkommt — der Pegel, auf den
  abgesenkt wird (0 bis 100 %), Dauer und Kurvenform des Aus- und des
  Einblendens, und ob die Visuals die Absenkung sehen (*Visuals follow*,
  Vorgabe aus). Rechtsklick auf den Knopf wählt das Profil; *Edit mute
  profiles...* öffnet den Dialog zum Einstellen, Speichern und Löschen.
  Mitgeliefert: *Announcement* (auf 20 %), *Soft mute*, *Hard mute*.
- **`LumiViz.exe <datei>`:** LumiViz lässt sich mit einer Datei starten (im
  Explorer *Öffnen mit*, oder die Datei auf die Exe ziehen). Ein Preset läuft
  statt des zuletzt aktiven, ein Karaoke-Preset kommt in die Karaoke-Ebene,
  eine Komposition wird geöffnet.
- **Immer nur ein LumiViz:** läuft LumiViz schon, gibt ein weiterer Start
  seine Datei an das laufende Fenster ab und beendet sich; das Fenster kommt
  nach vorn. Die Regel gilt je Programmdatei (zwei Builds laufen
  nebeneinander); `--new-instance` startet bewusst ein weiteres.

## Geändert

- **Die Master-Zeilen sind niedriger als ein Kanal** (60 %) — Urteil aus dem
  Sichttest der Session 100. Mit den drei neuen Zeilen liegen vier
  Master-Zeilen unter dem Lineal.
- **Kompositionen** (`.lvcomp`) tragen die Kurven der neuen Zeilen in drei
  neuen Abschnitten (`visualMaster`, `audioMaster`, `karaokeMaster`) — nur,
  wenn eine Kurve eingestellt ist. Ältere Dateien lesen sich wie bisher; ein
  älteres LumiViz überliest die neuen Abschnitte.
- **Die Karaoke-Ebene im Composer** folgt jetzt jeder Änderung im Panel
  Karaoke sofort. Bisher las der Composer die Ebene nur einmal, beim
  Einschalten.

## Behoben

- **Das Panel Karaoke lud bei laufendem Composer nichts** („Could not load:
  <Pfad>", ohne Grund; im Panel stand „works with the Multi Effect
  visualizer"). Es ist jetzt bei laufendem Composer bedienbar: Laden,
  Speichern, Default, Einstellen und die Trail-Presets wirken, und das Bild
  zeigt es sofort.

## Bekannte Grenzen

- Ein Teil des Songtexts mit der Mischart *XOR* kennt keine Stärke: in der
  Zeile KARAOKE ist er voll da, bis die Kurve null erreicht.
- Antwortet ein laufendes LumiViz drei Sekunden nicht, startet ein weiterer
  Aufruf als eigenes Fenster. Kommt eine zweite Datei, während das erste
  LumiViz noch hochfährt, ist nicht festgelegt, welche danach läuft.
- Eine Musikdatei als Argument wird noch nicht angenommen. Der Eintrag „Öffnen
  mit LumiViz" im Explorer-Skript fehlt noch.
- Der Mute hat kein Tastenkürzel und wird nicht über den Neustart gemerkt.

## Dokumentation

- Benutzerhandbuch 1.110.0: §2 *Mute für Durchsagen*, §15 *Die Zeilen VISUAL
  und AUDIO* und *Die Zeile KARAOKE*, §17 (neu) *Mit einer Datei starten — und
  immer nur ein LumiViz*.
- Composer_Konzept 0.61.1: Nähte N16, N17, N18; R2, N8 und N13 wachsen;
  Rückbau-Liste.
- Offene_Punkte 1.115.0.

## Tests

957 Cases im Zweig `composer` (Start der Session: 943); letzter voller Lauf
957 / 957, 0 Skips (MSVC). App und Tests bauen mit MSVC und clang.

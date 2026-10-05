# Changelog Session 95 (2026-10-05)

> **Der Composer hat sein Panel — und in ihm lässt sich eine Komposition ganz
> bauen.** Ansehen und Abspielen mit Transport, Clips verschieben und in der
> Länge ziehen, Presets aus dem Effect Browser hineinziehen, Kanäle anlegen,
> Ein- und Ausblenden mit sieben Formen samt Bézier, und die Kette eines Clips
> im Panel Effect Chain ändern, während man sie gemischt sieht. Der Composer
> bleibt im Zweig `composer`. Im Programm gesehen sind das Panel und das
> Bearbeiten vorhandener Clips; alles Weitere wartet auf den Sichttest.

## Neu

- **Panel Composer** (*View → Panels → Composer*): Schalter **Composer on**,
  Play / Pause, Stopp, **Loop** über die ganze Komposition, **Resume after
  restart**, Stelle und Länge als Zeit; Zeitachse mit Lineal und Abspielmarke
  (Klick und Ziehen springen), die Kanäle als Zeilen — CH1 oben —, die Clips
  als Blöcke mit ihrer Pegelkurve; darunter der angeklickte Clip mit Kanal,
  Datei, Zeiten und Mischart.
- **Geladen ist nicht an:** eine Komposition bleibt im Panel, auch wenn der
  Composer aus ist und die Effektkette das Bild liefert. So lässt sich etwas
  vorbereiten; **Composer on** schaltet es ins Bild.
- **Bearbeiten** (Haken **Edit**, nach jedem Start aus): Clips verschieben —
  auch in einen anderen Kanal —, Kanten ziehen, **Entf**, Mischart wählen,
  *Edit → Undo* / *Redo*. Trifft der gezogene Clip auf einen Nachbarn, hält er
  an; mit **Ctrl** wird der Nachbar gekürzt, mit **Ctrl+Shift** rücken die
  Nachbarn mit. Ziehen rastet auf Clip-Kanten, Abspielmarke und Raster ein,
  mit **Alt** frei.
- **Neues hinein:** **New** beginnt eine leere Komposition; Presets aus dem
  Effect Browser oder dem Explorer fallen als Clips in einen Kanal, unter der
  letzten Zeile entsteht ein neuer Kanal; Rechtsklick für *Add channel*,
  *Rename channel...*, *Channel enabled*, *Delete channel*, *Delete clip*.
- **Ein- und Ausblenden:** zwei Griffe oben im Clip für die Länge; Rechtsklick
  → *Fade in* / *Fade out* für die Form — Linear, S-curve, Ease in, Ease out,
  Exponential, Hold und neu **Bezier** mit zwei ziehbaren Anfassern.
- **Sync on Effect Chain:** solange eine Komposition das Bild liefert, zeigt
  das Panel Effect Chain die Kette des angeklickten Clips — Änderungen wirken
  sofort im gemischten Bild — oder ruht.
- **Die Kette eines Clips speichern:** über das gewohnte Speichern von Effect
  Chain (Ctrl+S), dann immer unter neuem Namen; das Original bleibt, der Clip
  wechselt auf die neue Datei, weitere Clips mit demselben Preset nach Auswahl.
- **Speichern der Komposition:** **Save** / **Save As...** im Panel, ein
  Stern zeigt Ungespeichertes; Rückfrage vor Open, Close, New und dem Beenden.
- **Festes Ende:** das Ende der Komposition folgt dem letzten Clip; mit
  **Fixed end** und einer Zeit steht es fest, mit eigener Marke im Lineal.
- **Über den Neustart:** war eine Komposition geladen, liegt sie nach dem
  Start wieder im Panel; mit **Resume after restart** kommt alles so zurück,
  wie es beim Beenden war.
- **Settings → Panels:** **Open Composition** (spielt sofort oder hält am
  Anfang) und **End of a Composition** (halten, an den Anfang, Composer aus).
- **Benutzerhandbuch §15 Composer** — auch *Open / Close Composition* aus
  Session 94, die dort fehlten, und der Aufbau einer `.lvcomp`.

## Geändert

- **Die Kanäle stehen in der Reihenfolge der Datei, CH1 oben.** Im Bild liegt
  der erste Kanal weiter zuunterst.
- **Beim Bearbeiten einer laufenden Komposition laufen die Presets weiter;**
  ein Clip behält seinen Renderer, auch an anderem Platz oder in anderem Kanal.
- **Die Zeitachse zeigt Luft hinter dem Ende,** damit sich darüber hinaus
  ziehen und dahinter ablegen lässt.
- **Close Composition** nimmt die Komposition auch aus dem Panel.
- **Datei `.lvcomp`:** neue Kurvenform `curve` 6 (Bézier) mit dem Feld
  `bezier`; alte Dateien lesen sich unverändert.

## Behoben

- **Das Ende wanderte nicht mit** (Befund Sichttest): wurden Clips verschoben
  oder verkürzt, lief die Komposition bis zur alten Endzeit schwarz weiter, und
  Loop griff erst dort. Eine Länge, die genau das Ende des letzten Clips ist,
  gilt jetzt als „folgt dem letzten Clip".
- **Absturz beim ersten Clip in einem eben entstandenen Kanal** (im Test
  gefunden, nie ausgeliefert).

## Intern

- **Composer, Stufen C2b, C3a, C3b, C3c und C4 Teil 1** (Zweig `composer`):
  `composer/Sitzung` (geladene Komposition, Composer on, Transport, Ende,
  Neustart, Bindung von Effect Chain), `composer/ComposerPanel` mit
  `Zeitachse`, `composer/Bearbeiten` (reines Modell). Nähte: N8 erweitert
  (`acb8a9b`), N4 (`1815da7`), N9 (`4ac6be9`), N10 (`4826db1`), N11
  (`356a868`, mit eigenem Code in einem Commit). Das Binden von Effect Chain
  an einen Clip braucht keine Naht (`VisualizerChangedEvent`).
- **Ein Renderer, an dem Effect Chain hängt, wird nie unter ihr gelöscht:**
  er wird geparkt und erst nach der Freigabe auf dem Render-Thread gelöscht;
  nach dem Verlust des GL-Kontexts bekommt der Clip ihn samt geänderter Kette
  wieder.
- **Steuerdokument** `docs/ui/Composer_Konzept.md` 0.16.1 — Entscheide der
  Session in §6 bis §8, geplantes Modul **Composer Variables** in §9e, die
  Pegelkurve in drei Teilen mit Presets in §9, Rückbau-Liste nachgeführt.
- **Tests:** 894 Cases (Start der Session: 877). Neu: `test_ComposerSitzung`,
  `test_ComposerPanel`, `test_ComposerBearbeiten`; erweitert:
  `test_ComposerWiedergabe`, `test_ComposerBildquelle`.
- **Vergleichslauf:** nicht gefahren — die Bildberechnung des Renderers ist
  nicht berührt.

## Noch nicht gesehen

- **Sichttest S95c** (`.claude/handover/Sichttests_S95c.md`): das Ende nach
  dem Fix, New und Hineinziehen, Kanäle, Blenden, das Bearbeiten im Lauf, Sync
  on Effect Chain und das Speichern der Kette eines Clips.
- **Alt beim Ziehen** (ohne Einrasten) im echten Programm.

## Bekannt offen

- Keine Rückfrage, wenn mit ungespeicherten Änderungen an der Kette eines
  Clips ein anderer Clip angeklickt oder der Composer ausgeschaltet wird.
- Im Ruhezustand zeigt Effect Chain ihren allgemeinen Hinweistext; ihre
  Preset-Zeile kennt nur das Preset der Kette der App.
- Eine Pegelkurve mit eigenen Punkten mitten im Clip lässt sich im Panel noch
  nicht bearbeiten — der Punkte-Editor ist der nächste Schritt.
- **Movement Table** steht in der App unter *Settings → Panels*; Handbuch und
  Changelog 94 nennen *Performance* (`Offene_Punkte.md` §8).

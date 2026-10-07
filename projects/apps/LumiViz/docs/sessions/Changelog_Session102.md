# Changelog Session 102 (2026-10-07)

> **Die Zeitachse des Composers lässt sich zoomen und rollen** — die Zeit und
> die Höhe der Zeilen, jede für sich oder beide zusammen. Das Lineal und die
> Spalte der Kanalnamen bleiben dabei stehen, und hineingezoomt folgt der
> Ausschnitt der laufenden Abspielmarke.
> **Im Programm gesehen** ist von dieser Session nichts: alles ist gebaut und
> per Test belegt.

## Neu

- **Zeit-Zoom:** **Ctrl + Mausrad** zoomt die Zeit um den Mauszeiger — von
  der ganzen Länge bis zu einer Millisekunde je Pixel. Das Lineal teilt sich
  dabei bis auf Zehntelsekunden. Eine frisch geladene Komposition beginnt
  mit der ganzen Länge.
- **Höhen-Zoom:** **Alt + Mausrad** ändert die Höhe der Zeilen in Schritten
  von 10 %, von 50 % bis 300 % — kleiner, um mehr Kanäle zu sehen, größer, um
  die Pegelkurve feiner zu ziehen. Die Zeilen MASTER, VISUAL, AUDIO und
  KARAOKE bleiben immer 60 % so hoch wie ein Kanal. Die Höhe bleibt über den
  Neustart.
- **Beides zusammen:** **Ctrl + Alt + Mausrad**.
- **Rollen:** das Mausrad rollt nach oben und unten, **Shift + Mausrad** zur
  Seite; Rollbalken an beiden Rändern erscheinen, wenn es etwas zu rollen
  gibt. Das **Lineal bleibt oben stehen**, die **Spalte der Kanalnamen
  links**.
- **Schwenken:** mit gedrückter **mittlerer Maustaste** lässt sich der
  Ausschnitt in beide Richtungen verschieben.
- **Fit und Ctrl+0:** der Knopf **Fit** (rechts in der Zeile mit *New*,
  *Load...*, *Save*) und **Ctrl+0** setzen zurück — die ganze Länge auf der
  Breite, die Zeilen in Normalhöhe. Der Tooltip von *Fit* zählt die Gesten
  auf; dieselbe Liste steht im Rechtsklick-Menü der Zeitachse unter *Zoom*.
- **Scroll with playhead:** ein Schalter neben *Fit* (Standard: an).
  Hineingezoomt folgt der Ausschnitt der Abspielmarke, solange die
  Komposition spielt — **Smooth** (Standard: die Marke bleibt in der Mitte,
  die Clips ziehen unter ihr durch) oder **By page** (der Ausschnitt
  blättert, wenn die Marke ihn verlässt). Die Art steht in
  *Settings → Panels → Scroll with playhead*. Solange eine Maustaste in der
  Zeitachse gedrückt ist oder ein Rollbalken gezogen wird, bewegt sich der
  Ausschnitt nicht von selbst.

## Geändert

- **Die Zeitachse rollt selbst.** Bisher rollte sie als Ganzes in einer
  rollenden Fläche, und das Lineal wanderte mit hinaus. Nicht gerollt und
  ohne Zoom sieht sie aus wie bisher.
- **Rechtsklick in die Zeitachse ohne *Edit*:** öffnet jetzt ein Menü — nur
  mit *Zoom*. Bisher öffnete sich ohne *Edit* nichts.
- **Niedrige Zeilen:** ein Name, der nicht mehr in seine Zeile passt, wird
  nicht gezeichnet; der Tooltip nennt ihn. Bei 50 % und 60 % Höhe lassen sich
  die Punkte der Pegelkurve nicht ziehen und sind nicht zu sehen — Clips
  verschieben, an den Kanten ziehen und der Doppelklick in den Curve Editor
  gehen in jeder Höhe.
- **Ein Clip, dessen Anfang hinausgerollt ist,** zeigt seinen Namen am
  linken Rand der Zeitachse.

## Grenzen

- Ziehst du einen Clip an den Rand des Ausschnitts, rollt die Zeitachse
  nicht von selbst mit. Das Mausrad rollt auch während des Ziehens.
- Der Curve Editor zoomt nicht.
- Der Ausschnitt läuft höchstens 30-mal je Sekunde mit (der Takt des
  Panels).

## Einstellungen

| Wo | Einstellung | Standard |
|---|---|---|
| *Settings → Panels* | **Scroll with playhead** — *Smooth* oder *By page* | *Smooth* |
| Panel Composer | Schalter **Scroll with playhead** (über den Neustart gemerkt) | an |
| Panel Composer | die Höhe der Zeilen (über den Neustart gemerkt) | 100 % |

## Unter der Haube

- Eigener Code des Composers in bestehenden Dateien (`ComposerPanel`,
  `Sitzung`); eine neue Klasse `ZeitachsenRahmen` für die Rollbalken. Keine
  neue Datei.
- **Eine Naht wächst:** N9, die Auswahl *Scroll with playhead* in
  `UI/panels/SettingsPanel.cpp` — als eigener Commit, getrennt vom Eigenen.
- Gezeichnet wird nur das sichtbare Stück der Zeit (Striche, Clips,
  Wellenform, Pegelkurve): beim feinsten Maßstab im Test 6 ms je Bild.
- Nach Alt + Mausrad bleibt die Menüleiste in Ruhe (unter Windows schaltet
  Alt allein sie sonst beim Loslassen auf die Tastatur).
- Der Vergleichslauf nach den zwei Eingriffen am Renderer aus Session 101
  entfällt, mit Begründung (`Offene_Punkte.md`); es bleibt der Lauf vor dem
  Holen nach `master`.

## Tests

- **966 Cases** (vorher 957), letzter voller Lauf 966 / 966, 0 Skips.
- Neun neue Fälle in `test_ComposerPanel.cpp`: Rollen, Zeit-Zoom,
  Höhen-Zoom, die Menüleiste nach Alt + Mausrad, die Höhe über den Neustart
  mit *Fit* und Ctrl+0, Schwenken, Mitlaufen, das Panel mit laufender
  Komposition, die Naht N9 am Settings-Panel.
- **Nicht durch Tests gedeckt:** Mausrad und mittlere Taste am Gerät, Ctrl+0
  als Taste durch das Hauptfenster, das Kontextmenü, Aussehen und Rollgefühl.

## Doku

- Benutzerhandbuch 1.111.0 — §15 neuer Abschnitt *Zoomen und Rollen*.
- Composer_Konzept 0.68.1 — §7 Zoom und Rollen (neun Entscheide), §12.2 N9,
  §12.4 Rückbau-Liste.
- Offene_Punkte 1.124.0.

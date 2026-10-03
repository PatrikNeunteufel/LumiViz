# Changelog Session 90 (2026-10-02 / 2026-10-03)

> **Songtext Wort für Wort.** Der Lyrics-Knoten und die Karaoke-Ebene heben
> das Wort hervor, das gerade gesungen wird, und folgen ihren Skripten jetzt
> in jedem Frame — ohne dass dafür neu gezeichnet werden muss. Der Lyrics
> Trail zeigt genau das Bild, das stand. Unsynchroner Text lässt sich per
> Skript weiterrücken, und im Panel Lyrics Sync lassen sich Stimmen für
> Duette setzen und Wort-Zeiten klopfen.

## Neu

- **Das laufende Wort hervorheben** (Lyrics-Knoten und Karaoke-Ebene, bei
  Songtext mit Wort-Zeiten): **Word scale (%)** macht es größer, **Own word
  color** / **Word color** geben ihm eine eigene Farbe. **Word layout** wählt,
  ob die Nachbarn zur Seite rücken (*Reflow*) oder die Zeile stehen bleibt
  und das Wort über ihnen liegt (*Overlay*). **Word flow (ms)** lässt die
  Größe gleiten: das neue Wort wächst, während das vorige schrumpft.
- **Vorschau-Zeilen eigens gestalten:** **Preview size (%)**, **Preview
  weight**, **Preview X shift / Y shift (%)** — alles relativ zur laufenden
  Zeile — und **Line spacing (%)** für den Zeilenabstand.
- **Skriptfeld Word:** läuft einmal je Wortwechsel und liest `word` (das
  Wort, das gerade gesungen wird) und `wordcount`; beide Variablen stehen in
  allen Skriptfeldern bereit.
- **`linetrigger` und `wordtrigger`:** rücken unsynchronen Text per Skript
  eine Zeile bzw. ein Wort weiter (oder zurück). `linetrigger = b;` rückt auf
  jeden Beat eine Zeile, `wordtrigger = b;` führt Wort für Wort durch den
  ganzen Text. Der erste `wordtrigger` eines Titels schaltet den Wort-Stand
  des Blocks ein (gesungen / gerade jetzt / noch nicht).
- **Lyrics Sync — Duett:** Spalte **Voice**, Knöpfe **Voice 1 / 2 / Both**
  und die Tasten 1, 2, 3 setzen, wer die markierten Zeilen singt.
- **Lyrics Sync — Wort-Zeiten klopfen:** der Schalter **Words** zeigt die
  Worte der markierten Zeile als Streifen; **Tap** gibt dem gewählten Wort
  die Zeit und rückt weiter, nach dem letzten Wort zur nächsten Zeile.
  **Clear words** nimmt einer Zeile nur die Wort-Zeiten.

## Geändert

- **Lyrics Trail zeigt das Bild, das stand.** Der Lyrics-Knoten hebt das Bild
  der laufenden Zeile auf und gibt es beim Zeilenwechsel an den Trail weiter
  — Wort für Wort Gesungenes, eine verkleinerte lange Zeile und die Lage
  bleiben, wie sie waren. Mit **Own color** und ohne laufenden Lyrics-Knoten
  zeichnet der Trail wie bisher neu.
- **Skripte am Lyrics-Knoten wirken in jedem Frame.** Lage, Größe und Farben
  folgen dem Skript jetzt aus jedem Feld, auch aus Frame; ein Knoten mit
  Skript setzt seine Zeilen dafür auf der Grafikkarte aus einmal gezeichneten
  Worten zusammen. Ein Puls auf den Beat ist damit möglich (Beispiel im
  Benutzerhandbuch). Nur Weight, Italic, Outline size und Preview weight
  folgen weiter allein aus Init, Song, Beat, Line und Word. Die Kontur wächst
  auf diesem Weg mit der Schrift.
- **Eine Schriftgröße je Song.** Passt die längste Zeile des Songtexts nicht
  in die Breite, wird die Schrift für alle Zeilen kleiner gesetzt (höchstens
  auf die Hälfte), und der Zeilenabstand geht mit. Bisher wurde jede Zeile
  einzeln verkleinert, der Abstand blieb bei der eingestellten Schrift.
- **Lyrics Sync:** Wort-Zeiten gehen mit, wenn eine Zeile verschoben oder neu
  geklopft wird. Gespeichert werden sie je Zeile nur, wenn alle ihre Worte
  eine Zeit haben.

## Behoben

- **Lyrics Trail nach einer Pause:** eine Zeile vor einer Pause-Zeile (Zeit
  ohne Text) wehte zweimal weg — beim Beginn der Pause und noch einmal beim
  Beginn der nächsten Textzeile.
- **Sprung der Schrift beim Zeilenwechsel:** ein Frame-Skript, das Lage,
  Größe oder Farbe setzte, schlug genau einmal je Zeilenwechsel durch.
- **AVI-Knoten unsichtbar:** seit Session 89 zeichnete ein AVI-Knoten nichts,
  wenn kein Text- oder Lyrics-Knoten vor ihm in der Kette stand.

## Noch nicht im laufenden Programm gesehen

Alles oben ist per Test und an gerenderten Bildern geprüft, aber nicht im
laufenden Programm — besonders das Gleiten der Wortgröße mit echter Musik,
der Wort-Weg in der Karaoke-Ebene mit Render Scale, das Klopfen von
Wort-Zeiten im Takt und die Tasten 1, 2, 3 im Panel Lyrics Sync.

## Nicht in dieser Session

- **Langsamer abspielen** im Panel Lyrics Sync (Schieber) — geplant für
  Session 91.

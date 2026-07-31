# MyViz — Changelog Session 56 (2026-07-29)

> **Thema:** Die „13 bestätigten Editier-Befunde" aus der Vorsession waren
> weder 13, noch alle Befunde: **sieben waren echt** (alle behoben), acht
> lagen an der Messung selbst. Daraus wurde ein Tag über die Frage, wann eine
> Messung überhaupt etwas beweist — mit einem Ergebnis, das sich sehen lassen
> kann: **jedes der 717 Panel-Felder hat jetzt eine Erklärung**, und die Zahl
> der stummen Prüf-Sonden fiel von 115 auf 15.

## Behoben

**Ein neues Texer-Bild wirkte erst nach Speichern und Neuladen.** Der Effekt
behielt seine einmal hochgeladene Textur, ein Bildwechsel im Panel kam nie an.
Jetzt wird bei geänderten Bilddaten neu hochgeladen — Bildwechsel wirken
sofort.

**Drei MilkDrop-Regler (Netzauflösung, Prüfraster) wirkten erst nach dem
Neuladen.** Sie standen in einem Codeblock, der nur bei Preset-, Skript- oder
Shader-Änderungen läuft — ein Panel-Edit zählt dort nicht dazu. Jetzt werden
sie je Frame übernommen.

**Roto Blitter konnte ein einfarbig gelbes Bild liefern** — und mit ihm
Blitter Feedback und der einfache Scope. Die Weiche für alte Dateiformate
erkannte ein Alt-Dokument an der *Abwesenheit* des neuen Felds statt an der
*Anwesenheit* des alten. Ein frisches Preset ohne das neue Feld geriet damit
in den Migrationspfad und bekam einen Zoom von Null — das Bild bestand aus
einem einzigen Pixel, unendlich vergrößert. Alle drei Weichen prüfen jetzt auf
das alte Feld.

**Timescope zeichnete unter dem Untergrund.** Der Effekt malt eine ein Pixel
breite Spalte je Frame; der Prüf-Untergrund übermalte sie im nächsten Frame
wieder — im Schlussbild stand eine Spalte von 320, die Messung sah 1/320 der
Wirkung. Der Prüfstand kennt jetzt je Effekt-Typ den passenden Untergrund.
Dazu erbt der Misch-Modus des Timescope beim Laden jetzt die AVS-Vorgabe.

**Kleinians Farbregler war mit ganzen Zahlen wirkungslos.** Die Färbung
entsteht aus dem Nachkommateil von „Spiegelungszahl × Regler" — und die
Spiegelungszahl ist eine ganze Zahl. Jeder ganzzahlige Reglerwert ergab damit
exakt null: eine einfarbige Scheibe, egal ob 1,0 oder 4,0. Die Vorgabe steht
jetzt auf 0,17 und zeigt die gedachte Kachelung.

## Jedes Feld erklärt sich jetzt selbst

**717 von 717 Feldern haben einen Tooltip** — Quelle ist die Dokumentation
direkt am Datenfeld, eine erzeugte Tabelle bringt sie ins Panel, auf
Bedienelement *und* Beschriftung. Zwei Wächter passen auf, dass keine
Erklärung wieder verloren geht: ein harter Test ohne Ausnahmeliste und eine
Schlüssel-Prüfung im Erzeuger — die fand gleich fünf Verweise ins Leere, die
sonst niemandem aufgefallen wären.

## Unter der Haube: eine Quelle für Vorgabewerte

Jeder Vorgabewert stand bisher doppelt — einmal am Datenfeld, einmal im
Datei-Leser. Wer den einen änderte und den anderen vergaß, änderte nichts
(genau das war einem Fix zuvor passiert). Jetzt gibt es **eine** Quelle: der
Leser greift auf die Feld-Vorgabe zurück. 415 Stellen umgestellt,
bild-neutral gehalten und mit Roundtrip-Tests abgesichert.

## Was die Messung gelernt hat

Von rund 100 durchgesehenen stummen Sonden waren **zwei** ein Befund an der
App — der Rest lag am Testaufbau. Das neue Urteil **VERDECKT** unterscheidet
jetzt sauber: „der Wert kommt nicht an" (Befund) von „der Wert kommt an, aber
ein früherer Zustand verdeckt ihn" (kein Befund). Entschieden wird das durch
die Gegenrichtung der Messung plus Positivkontrollen — eine Regel, die beide
Ursachen gleich behandelt, ist kein Quercheck.

## Stand

- **Stumme Feld-Sonden: 115 → 15**, Editier-Prüfung ohne offene Befunde
- **Tooltips: 717/717**, zwei Wächter aktiv
- Tests grün, alle Bauarten fehlerfrei

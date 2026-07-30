# MyViz — Changelog Session 57 (2026-07-30)

> **Thema:** Die letzten 15 Felder, an denen keine Messung anschlug, sind
> aufgelöst — und **sechs davon waren echte Fehler**: Regler, die im Panel
> standen, sich verstellen ließen und von keinem Effekt gelesen wurden. Alle
> sechs sind behoben.
>
> Bemerkenswert ist die Verteilung: von den ersten hundert durchgesehenen
> Sonden lagen 98 am Messaufbau. Von den letzten fünfzehn waren es neun. Die
> leichten Fälle standen vorn — wer nach hundert Fehlalarmen aufhört, lässt die
> Befunde liegen.

## Behoben

**Blur: die Rundung fehlte, und die Voreinstellung war verkehrt.** Der
Weichzeichner des Originals rechnet in ganzen Zahlen und schneidet dabei ab —
das Bild wird bei jeder Anwendung ein wenig dunkler. Der Schalter „Round up"
gleicht das aus. Wir rechneten stattdessen exakt, also ohne Verlust *und* ohne
Ausgleich, und der Schalter tat schlicht nichts. Jetzt rechnet der Blur wie das
Original, mit dem Ausgleich, den es je Stärke vorsieht; die Voreinstellung steht
wie dort auf **aus**.

Das ist in Ketten mit Rückkopplung deutlich zu sehen: dort entscheidet die
Rundung, ob ein Nachleuchten langsam ausbleicht oder stehen bleibt. Zwei neue
Prüfbilder bewachen beide Richtungen, und beide stimmen jetzt mit der Referenz
überein. Sie sind auch der Grund, warum der Fehler so lange unentdeckt blieb:
das bisherige Blur-Prüfbild zeichnet jeden Frame neu, dort ist der Unterschied
kleiner als die Anzeige.

**Grain: das Korn stand still, obwohl „Static" ausgeschaltet war.** Der Effekt
zeichnete immer dasselbe Rauschmuster — die Voreinstellung versprach das
Gegenteil, und der Schalter war ohne Wirkung. Jetzt flimmert das Korn je Frame,
wenn „Static" aus ist, und steht still, wenn es an ist.

**Osc Ring und Osc Star haben die Kanalwahl ignoriert.** „Links", „Rechts" und
„Mitte" zeichneten dasselbe Bild; beide Effekte nahmen immer den gemischten
Kanal. Jetzt wirkt die Wahl. Bestehende Presets sehen unverändert aus — die
Voreinstellung *ist* „Mitte", und genau das haben sie bisher gezeichnet.

**Texer: von drei Mischarten gab es nur zwei.** „Additiv" und „50/50" waren
intern derselbe Zustand, 50/50 also nicht zu haben, und „Ersetzen" mischte
additiv statt zu ersetzen. Jetzt mischt ein Sprite genau so wie eine Linie im
selben Modus.

**Texer II: „Wrap around" war ohne Funktion.** Sprites am Bildrand wurden
abgeschnitten, egal wie der Schalter stand. Jetzt setzen sie sich auf der
Gegenseite fort.

**Interferences: der Startwinkel ließ sich nur beim Laden setzen.** Der Effekt
dreht seine Kopien fortlaufend weiter; der eingestellte Startwinkel wurde einmal
beim Aufbau übernommen und danach nie wieder. Am Regler drehen brachte deshalb
nichts — erst Speichern und neu Laden. Jetzt wirkt er sofort. (Gefunden von der
zweiten Prüffamilie, die genau diese Frage stellt: wirkt ein Feld auch beim
Editieren, nicht nur beim Laden? Sie hat alle 702 Felder geprüft und genau diesen
einen Fall gefunden.)

## Vier Befunde aus dem Bildvergleich mit dem Original

**Der Fractal Zoomer hatte einen Regler, der nur zwei Stellungen kannte.** „Trail
persistence" verspricht eine Stärke von 0 bis 1; tatsächlich schaltete jeder Wert
über 0,01 dasselbe halbe Nachleuchten ein — 0,3 und 1,0 ergaben Pixel für Pixel
dasselbe Bild. Jetzt ist es eine echte Stärke: 0 keine Schleife, 1 Standbild. Die
Voreinstellung 0,5 entspricht genau dem bisherigen Verhalten, bestehende Presets
sehen also unverändert aus.

**Die Farbkennlinie der Color Map stimmt jetzt exakt.** Sie lag seit Monaten
„etwa eins daneben" — auf einem Schwarz-Weiß-Verlauf war *jeder* Wert um eins zu
hoch. Der Grund liegt tiefer, als es aussah: das Original rechnet die Kennlinie
in drei ganzzahligen Schritten, und schon die Schrittweite selbst verliert dabei
etwas. Deshalb waren glatte Spannweiten (16, 64, 128) immer richtig und krumme
(200) nicht. Über sechs Spannweiten geprüft: **922 von 922 Messpunkten stimmen**.

**Colorfade kann jetzt, was das Original kann.** Zwei Betriebsarten fehlten
komplett, weil beim Import nur ein einziges Bit von dreien gelesen wurde:
*langsames Nachziehen* (die Farbregler wandern über mehrere Frames auf ihren Wert
zu, statt sofort dort zu stehen) und *Zufallsfarben auf den Beat*. Beide sind da
und im Panel bedienbar.

Eine Folge davon ist wichtig für bestehende Presets: **die Beat-Regler wirken nur
mit eingeschaltetem „Slow fade"**. Das ist im Original genauso — ohne diese
Betriebsart kommt der Beat-Zweig dort gar nicht erst zum Zug. Wer die Beat-Regler
nutzen will, schaltet also zusätzlich „Slow fade" ein.

Das Feld *Beat frames* bleibt als LumiViz-Zusatz erhalten und hält den
Beat-Zustand länger, als das Original es täte. Die Voreinstellung 1 entspricht
dem Original — ein importiertes Preset ändert sich dadurch nicht.

**Interferences stimmt jetzt mit dem Original überein.** Der Effekt legt mehrere
verschobene Kopien des Bildes übereinander. Ragte eine Kopie über den Bildrand
hinaus, haben wir dort den Randpixel wiederholt und nach innen verschmiert — das
Original lässt sie schlicht weg. Dazu rechnet es die Versätze in ganzen Pixeln
und gewichtet die Kopien über eine Ganzzahl-Tabelle, wir taten beides in
Fließkomma. Alles drei korrigiert: **die Prüfzeile ist grün**.

**Water: derselbe Fehler, deutlich näher am Original.** Auch die Wasser-Wellen
lesen ihre Nachbarpixel — und bei einem Effekt, der sich selbst zurückspeist,
bleibt ein Randfehler nicht am Rand, sondern wandert über die Frames ins Bild.
Die mittlere Abweichung fiel von 0,029 auf **0,002**. Was übrig bleibt, ist kein
Strukturfehler: beide Bilder sind deckungsgleich, der Rest ist feines Rauschen —
ein Wellensystem verstärkt kleinste Rundungsunterschiede, genau wie beim Korn.

**Water Bump bleibt offen.** Wellenkern und Pufferwechsel stimmen; eine
Randkorrektur brachte nichts. Der nächste Verdacht ist notiert.

## Was das Prüfwerkzeug gelernt hat

Neun der fünfzehn Felder lagen nicht am Programm, sondern daran, wie gemessen
wurde. Die Ursachen sind der eigentliche Gewinn, weil jede von ihnen eine ganze
Klasse von Fehlmessungen erklärt:

- **Ein Wert, der zweimal in derselben Tabelle steht, gilt nur einmal** — und
  zwar in der späteren Zeile. Eine Korrektur an der 3D-Kamera war deshalb ein
  halbes Jahr wirkungslos, obwohl sie im Werkzeug stand. Dieselbe Falle hatte
  die Vorsession schon einmal erwischt.
- **Die Nummer einer Mischart ist je Effekt eine andere.** „Adjustable" heißt
  beim Set Render Mode 7, beim Buffer Save 10 und bei der Color Map 9. Geraten
  war überall 10 — beim Set Render Mode wurde daraus stillschweigend
  „Minimum", und der zugehörige Alpha-Regler galt zwei Prüfläufe lang als tot.
- **Eine Farbtafel mit fünf Stützstellen braucht fünf Gegenfarben.** Zwei
  reichten nicht, und die zweite traf zufällig die Voreinstellung — geändert
  hat sich also genau eine von fünf, in einem Bereich, den das Testsignal nicht
  trifft.
- **Weiß gehört nicht in eine Gegen-Palette.** Es ist der Ersatzwert, den
  mehrere Effekte für eine *leere* Farbtafel einsetzen — eine Tafel mit Weiß
  darin kann deshalb genau das Bild der Voreinstellung treffen.
- **Ein Faltungskern ist keine Farbtafel.** Als solche behandelt, bekam der
  Convolution-Effekt Gewichte in Millionenhöhe; das Bild übersteuerte
  vollständig, und die Prüfung meldete „wirkt", ohne etwas gemessen zu haben.
- **Ein stehendes Bild kann nichts zeigen, was sich über die Zeit aufbaut.**
  Das gilt für die Länge eines Beat-Fensters ebenso wie für einen Glow, dessen
  Wirkung erst der nächste Frame sieht.

Ein Feld ist als **nicht prüfbar** festgeschrieben statt gelöst: die obere
Grenze des Frequenzbereichs bei Rotating Stars. Der Effekt nimmt daraus nur den
höchsten Wert, und das Testsignal fällt von unten nach oben gleichmäßig ab — der
höchste Wert liegt damit immer am Anfang des Bereichs, unabhängig davon, wo er
endet. Mit sechs verschiedenen Bereichen gemessen, jedes Mal Pixel für Pixel
dasselbe Bild.

## Stand

- **Feld-Prüfung: kein stummes Feld mehr** (zuletzt 16, davor 115) — mit einem
  Gesamtdurchlauf über alle 702 Prüfbilder belegt. Gegen den Stand der Vorsession
  ist **kein einziges Feld schlechter** geworden und 109 sind besser
- **Editier-Prüfung: kein wirkungsloses Feld mehr** — ebenfalls ein
  Gesamtdurchlauf; der eine Befund daraus ist behoben (Interferences, s. o.)
- **Kalibrierung unverändert**: Modul-Matrix 36/41, Modul-Sonden 78/80 — nach
  sechs Eingriffen in die Effekte also **keine Verschlechterung**; dazu zwei
  neue Blur-Prüfbilder, beide grün
- **Tests: 485 von 485 grün**, beide Bauarten ohne Fehler

## Noch offen

Am Bildvergleich mit dem Original stehen weiter die zwölf bekannten Befunde
(Alternate Reality, Picture II, Bright Light District, Rotor, Tie Tunnel DM und
die übrigen). Neu dazugekommen ist nichts.

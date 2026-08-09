# LumiViz — Changelog Session 58 (2026-07-30)

> **Thema:** Die vier Presets, die im Bildvergleich mit dem Original seit
> Wochen unangetastet lagen, sind abgearbeitet — **alle vier stimmen jetzt
> überein**. Dazu kam ein fünftes, das im Vollbild schwarz blieb.
>
> Der Kern von vier der vierzehn Befunde ist derselbe Satz: **ein Wert, den das
> Original einmal je Bild oder einmal je Effektliste festlegt, haben wir für
> jeden einzelnen Bildpunkt oder gleich für die ganze Kette gesetzt.** Farbe,
> Takt, Startwert, Zufallszahl — überall dieselbe Verwechslung.

## Das Ergebnis in Zahlen

Abstand zum Original, 0 = gleich (kleinere Zahl ist besser), je in zwei
Bildgrössen gemessen:

| Preset | vorher | jetzt |
|---|---|---|
| Alternate Reality | 0,62 · 0,87 | **0,03 · 0,02** |
| Picture II („The Real Impressionist") | 0,52 · 0,45 | **0,01 · 0,01** |
| Rotor | 0,46 | **0,02 · 0,01** |
| Bright Light District | 0,25 · 0,21 | **0,02 · 0,02** |
| Lost Cause | 0,18 · 0,26 | **0,01 · 0,00** |

## Erst musste die Vergleichsseite selbst in Ordnung

Das Vergleichswerkzeug lädt für einige Effekte fremde Zusatzmodule aus der
Preset-Sammlung. Eines davon würfelt beim Laden seinen Zufallsgenerator neu —
aus der **Uhrzeit**. Acht Läufe im Sekundenabstand ergaben **sechs
verschiedene Bilder**. Jedes Preset mit dem Effekt „Channel Shift auf Takt" war
damit unmessbar; ein guter Teil des Abstands bei *Alternate Reality* war nichts
als das. Das Werkzeug benutzt jetzt für diese fünf Effekte seinen eigenen
eingebauten Code und liefert bei jedem Lauf dasselbe Bild.

## Behoben

**Der Takt gehört der Effektliste, nicht dem ganzen Bild.** Filtert ein „Custom
BPM" innerhalb einer Liste jeden vierten Schlag heraus, gilt das im Original nur
für die Effekte **hinter ihm in derselben Liste**. Bei uns galt es für die
gesamte Kette. Alles, was danach kam und auf Takt reagiert, kam damit viermal
seltener zum Zug — und teilte sich die Zufallszahlen mit dem Rest des Presets,
sodass auch die auseinanderliefen. Das war der grösste Einzelschritt der
Session.

**Die Farbe gehört dem Bild, nicht dem Bildpunkt.** Ein Scope legt seine Farbe
im Bild-Abschnitt seines Skripts fest; das Original setzt die Farbtafel
**einmal je Bild**, davor. Wir setzten sie vor jedem einzelnen Punkt neu und
überschrieben damit, was das Skript wollte. In *Lost Cause* sollten vier Scopes
nur auf dem Schlag aufblitzen — bei uns leuchteten sie durchgehend, und das Bild
lief nach Weiss aus.

**Zwei Effekte rechneten wir grundsätzlich anders als das Original.** Die
Faltung („Convolution") rechnet dort ganzzahlig, und ihr „Bias" zählt in
Schritten von 256 — schon der Wert 1 hebt jeden Bildpunkt über die Obergrenze.
Dazu liefert „absolute" bei negativem Ergebnis nicht den Betrag, sondern Weiss,
und „two pass" verdoppelt, statt zweimal zu falten. Der „Dynamic Shift"
verschiebt in **ganzen** Pixeln plus einem Bruchteil und macht die frei
werdenden Zeilen und Spalten hart schwarz — bei uns blieb dort ein Saum stehen.

**Vier Fehler beim Zeichnen der Scopes.** Ein Liniensegment ist im Original
**einfarbig** (die Farbe des Endpunkts), wir liessen die Farbe entlang der
Strecke verlaufen. Die Punkte müssen in voller Rechengenauigkeit geführt werden,
sonst fehlt die letzte Bildspalte. Die Linien lagen durchgehend **eine Spalte zu
weit links**. Und ein Punkt, der als „übersprungen" markiert ist, bleibt im
Original trotzdem Ankerpunkt für das nächste Segment — bei uns verschwand er
ganz, und ein Preset, das das im Wechsel nutzt, zeichnete **gar nichts**: in
*Bright Light District* fehlten zwei von drei Flügeln.

**Drei Fehler beim Import.** „Picture II" liest seinen Dateinamen aus einem Feld
fester Länge — variabel gelesen war der Name zwar richtig, aber alle sechs
Einstellungen dahinter waren Müll, und das Bild kam nie an. Derselbe Effekt hat
**sechs** Mischarten, nicht drei: Maximum und Minimum hatten wir pauschal als
Mittelwert genommen. Und ein „Movement" mit der Einstellung „none" ist im
Original ein ausdrückliches Nichts-Tun — wir meldeten dafür beim Import einen
Fehler.

## Was noch nicht stimmt

- **`el-vis_hypno07_FTL01_v2`** zeichnet bei uns etwa die halbe Dichte. Das
  Preset **misst seine eigene Bildrate** und stellt daraus Tempo und
  Empfindlichkeit ein — ein Preset, das sich selbst nachregelt, reagiert auf
  jede Bremse doppelt. Die Ursache ist noch nicht benannt.
- **`splendora`** lässt sich jetzt fehlerfrei importieren, das Bild weicht aber
  noch ab.
- **`el-visVR09(war)`** ist **kein** Fehler: unser Bild ist mit dem des
  Originals identisch — beide sind mit dem Testsignal schwarz.
- Am Rand der Faltung bleibt ein Rest von rund 550 Bildpunkten (letzte Spalte,
  erste Zeile); die drei Prüfbilder dazu melden ihn.

## Verifikation

Alle **485** Testfälle grün, keine übersprungen. Bildvergleich mit dem Original:
**40 von 43** Prüfbildern stimmen (unverändert), **81 von 88** Einzelsonden —
sechs Sonden sind neu dazugekommen, und drei bestehende melden seit der
Zeilen-Korrektur einen kleinen Rest, den der alte Fehler zugedeckt hatte.

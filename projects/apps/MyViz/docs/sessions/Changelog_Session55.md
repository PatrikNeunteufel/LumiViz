# MyViz — Changelog Session 55 (2026-07-28)

> **Thema:** Ein Feld muss beim **Editieren** dasselbe tun wie nach dem Laden —
> das tat es nicht. Ein neues Messwerkzeug prüft das jetzt für jedes Feld jedes
> Knotens und fand **15 Felder**, bei denen ein Reglerdreh wirkungslos bleibt.
> Vier Effekt-Befunde sind behoben, dazu zwei Ärgernisse beim Bedienen.

## Behoben

**Videos mit 24 Bit wurden stillschweigend verworfen.** Der AVI-Effekt öffnete
die Datei, las Frames — und zeichnete nichts. Betroffen war der **Normalfall**
bei unkomprimierten AVIs; von außen sah das aus wie eine fehlende Datei. Jetzt
werden 24 und 32 Bit gezeichnet, und jede andere Farbtiefe wird **gemeldet**
statt verschluckt.

**Ein neuer Dateipfad am AVI-Effekt kam nie an.** Das Video wurde einmal
geöffnet und blieb es; ein Pfadwechsel im Panel wirkte erst nach Speichern und
neu Laden. Jetzt wird bei jedem Wechsel neu geöffnet.

**Movement: „Source mapped" ließ sich verstellen, ohne dass etwas geschah.** Der
Schalter wurde nur beim Laden übernommen. Jetzt gewinnt ein Reglerdreh sofort —
und der beat-weise Wechsel zwischen den beiden Zuständen bleibt erhalten.

**Multi Delay: Die Verzögerung gehört jetzt dem Puffer, nicht dem einzelnen
Knoten** — wie im Original. Vorher bestimmte der *lesende* Knoten mit seinem
eigenen Wert, was herauskam, und die Einstellung am *schreibenden* Knoten war
unsichtbar. Ein Ausgabe-Knoten liefert jetzt immer das älteste Bild des Rings.
Bestehende Presets sind nicht betroffen: im Original kann es gar keine
ungleichen Werte geben, und in der Referenzsammlung nutzen zwei Presets den
Effekt.

## Bedienung

**Neue Effekte landen dort, wo man arbeitet.** Bisher wurden sie immer ganz
unten in der obersten Liste eingefügt — bei einer verschachtelten Kette weit weg
von der markierten Stelle. Jetzt gilt: ist ein Effekt markiert, kommt der neue
**direkt darunter**; ist eine Liste oder Host-Gruppe markiert, als **letztes
Element darin**; ist nichts markiert, ans Ende wie bisher.

**Der neue Effekt ist sofort bearbeitbar.** Er wird markiert und sein Editor
aufgebaut — vorher musste man erst einen anderen Knoten anklicken und wieder
zurück.

## Neu (Werkzeug)

- **Edit-Sonden.** Sie messen für jedes Feld, ob eine Änderung zur Laufzeit
  dasselbe Bild ergibt wie dasselbe Preset frisch geladen. Stand: **591 gleich ·
  101 teilweise · 15 wirkungslos** von 707 Feldern. Die verbleibenden 13 stehen
  mit Messwert in `docs/Offene_Punkte.md §1b`.
- **Test-Presets für alle Skript-Slots.** Die großen Knoten (Superscope,
  Dynamic Movement, Color Modifier, Terrain, Glow Orbs, Texer II) hatten für
  ihre Skriptfelder bisher keine Sonde — jetzt schon: 675 → **707** Sonden,
  keines ohne prüfbaren Gegenwert.
- **Kanal-Testsignal.** Der Testrenderer kann linken und rechten Kanal
  unterschiedlich füllen; damit sind die Kanalfelder des Timescope überhaupt
  erst messbar.

## Unverändert

Modul-Matrix und Modul-Sonden ohne Bewegung. Unit-Tests **482 grün**.
Feld-Sonden **560 wirkt · 31 schwach · 116 stumm** von 707.

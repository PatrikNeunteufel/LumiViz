# Changelog Session 107 (2026-10-10)

> **Am Programm hat sich nichts geändert.** Der Befund „MilkDrop-Presets
> stocken manchmal" ist geschlossen — die Ursache liegt außerhalb der App —,
> und LumiViz bezieht die Sichttest-Steuerung in Version 0.3.4.

## Geändert

- **Paket der Sichttest-Steuerung:** `sichttest.pin` nennt jetzt v0.3.4
  (vorher v0.3.3). Neu im Tester: ein abgeschlossener Lauf wird nicht mehr
  fortgesetzt, sondern als neuer Lauf begonnen; der Zeitstempel der Exe steht
  im Testlog im ISO-Format; die Angaben zum Build kommen aus der Meldung der
  Anwendung. Schnittstelle und Köpfe sind unverändert, am Quelltext von
  LumiViz war nichts zu tun.

## Befund

- **Das Stocken der MilkDrop-Presets kommt vom Rechner, nicht von LumiViz:**
  es drosselt der Temperaturschutz des Prozessors am Entwicklungsrechner. In
  einem Lauf mit `martin - glassworks 3` lagen die Einbrüche des Prozessortakts
  und die langen Bilder in denselben Sekunden. Der Punkt ist in
  [Offene_Punkte.md](../Offene_Punkte.md) §3 geschlossen; offen bleibt dort
  nur, die Übergabe der Variablen ans Skript billiger zu machen (Stufe 3).

## Offen

- Der neue Tester ist mit LumiViz am Bildschirm noch nicht gesehen.
- Debug-MSVC nicht gebaut.

## Tests

- 974 Cases, ein voller Lauf 974 / 974, 0 Skips (Testing-MSVC); dazu die
  Probe der Steuerung (DLL 0.3.4 geladen, mit dem Tester verbunden).

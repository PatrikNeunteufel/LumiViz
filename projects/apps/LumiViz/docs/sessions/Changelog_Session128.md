# Changelog Session 128 (2026-10-16)

> **Echte Cowbell und Clave, Loop-Kleinkram abgeschlossen.** Cowbell und Clave
> sind jetzt Aufnahmen statt gerechneter Klänge — als Samples, als Instrumente
> und in zwei Kits. Im Code hängt der Loop nicht mehr am Composer. Konzepte:
> `docs/audio/Loop_Konzept.md` („Loop-Kleinkram“), `docs/ui/Composer_Konzept.md`
> §9f. **Geprüft mit dem vollen Testlauf unter beiden Compilern.** Gehört und im
> Programm gesehen ist nichts.

## Neu

- **Drei Samples:** `cowbell` und `cowbell_tip` (TamTam Drum Kits aus der
  OLPC-Sammlung, CC BY 3.0) und `clave` — ein Schlag aus einer Aufnahme der
  Musikhochschule ESMUC auf Wikimedia Commons (CC0). Gruppe **Samples** in der
  Auswahl Sound; Urheber in `samples\CREDITS.md`. Handbuch §16.
- **Kits:** **Reggaeton** hat die Cowbell als perc 3, **Hand drums** die Clave
  als perc 4.

## Geändert

- **Die Instrumente Cowbell und Clave spielen die Aufnahmen.** Auf A5 (Cowbell)
  und C6 (Clave) klingen sie so, wie sie aufgenommen sind; andere Noten machen
  den Schlag höher und kürzer oder tiefer und länger. Die Beispiele Afrobeat,
  Reggaeton und Bossa Nova klingen damit anders, gleich laut wie vorher.
  Handbuch §16.

## Intern

- Der Loop fragt den Composer über eine eigene Schnittstelle; die Farbreihen der
  Kanäle liegen bei der gemeinsamen Achse. Ein Test hält die Richtung fest.
- Voller Testlauf unter Release-clang und Testing-MSVC grün (1279 Fälle).

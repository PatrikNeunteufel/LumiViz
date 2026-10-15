# Changelog Session 126 (2026-10-15)

> **Lautstärke im Composer.** Ein selbst gebauter Loop war neben einem fertigen
> Song viel zu leise, und ein Clip konnte nicht lauter werden als seine Datei.
> Jetzt hat jeder Ton-Clip ein **Gain**; dazu einzelne Clips ausschalten und,
> vorbereitet, eine Messung der Lautheit und ein Limiter. Konzept:
> `docs/ui/Composer_Konzept.md` §9g, §9h. **Geprüft mit Unit-Tests unter beiden
> Compilern.** Im Programm gesehen ist nichts.

## Neu

- **Gain am Ton-Clip:** Unten rechts im Panel Composer steht bei einem Song
  oder Loop das Feld **Gain** (−24 bis +24 dB) statt *Blend*. Es ist getrennt
  von der Pegelkurve — die Blenden laufen weiter von 0 bis 1. Ein Clip mit Gain
  trägt rechts eine Marke („+6.0 dB“), Rechtsklick → **Reset gain** setzt ihn
  zurück. Handbuch §15.
- **Einzelne Clips ausschalten:** Rechtsklick → **Clip enabled** oder
  **Ctrl+E** (mit *Edit*). Ein ausgeschalteter Clip bleibt stehen und lässt
  sich bearbeiten, spielt aber nicht: kein Ton, kein Bild, kein laufender Song,
  Next / Previous springen an ihm vorbei. Er trägt „(off)“. Handbuch §15.
- In der `.lvcomp` die Felder `gain` und `enabled` am Clip — nur geschrieben,
  wenn sie von der Vorgabe abweichen; ältere Dateien lesen sich unverändert.

## Behoben

- Die Audio-Engine nahm keine Lautstärke über 1 an — sie war für das Gain
  nötig und fiel erst beim Beleg vor dem Limiter auf (der Test des Gains lief
  gegen eine Engine aus Pappe).

## Intern

- Messung der Lautheit nach EBU R128 (`composer/Lautheit`), geprüft an den
  Prüftönen aus EBU Tech 3341 — Grundlage für den kommenden Dialog **Match
  loudness**.
- Ein Limiter für den Mixer (`audio/Begrenzer`): hält Spitzen unter −0,1 dBFS,
  was darunter bleibt, geht Bit für Bit durch. Der Composer schaltet ihn noch
  nicht ein — der Haken **Limiter** in der Zeile der Komposition kommt in der
  nächsten Sitzung.

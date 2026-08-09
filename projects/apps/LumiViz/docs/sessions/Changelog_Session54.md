# LumiViz — Changelog Session 54 (2026-07-28)

> **Thema:** Test-Presets für jedes Feld jedes Knotens (Knoten-Parameter-Konzept
> §9, „Strang E"). Das Werkzeug fand beim ersten Einsatz **vier Felder, die
> nicht wirken konnten** — alle vier sind behoben.

## Behoben

**Parameter-Skripte kamen nicht an.** 43 der 129 Größen, die ein Knoten seinem
Skript anbietet, wurden nach dem Rechnen nie gelesen — das Skript arbeitete, das
Ergebnis verfiel. Betroffen waren 25 der 47 Renderer, darunter Blur, Colorfade,
Convolution, Simple Scope, Buffer Save und Color Map. Ohne Skript ändert sich
nichts: die Regler waren und bleiben maßgeblich.

**Der Init-Slot hatte keine Wirkung.** Was ein Init-Skript einmalig setzte, wurde
im nächsten Frame von den Reglerwerten überschrieben. Jetzt gilt: **Init setzt
den Startwert, die Frames schreiben ihn fort** — und wer am Regler dreht,
gewinnt weiterhin. Als Folge wirkt auch ein Beat-Skript dauerhaft, statt nur im
Beat-Frame zu blitzen; das entspricht dem Verhalten von AVS.

**Bloom führte sein Skript nie aus.** In der Vorgabe-Einstellung („post") stieg
der Effekt vor dem Skript aus und rechnete den Schein aus den Reglerwerten. Die
drei Skriptfelder standen wirkungslos im Panel.

**„Bilinear filtering" war in zwei Effekten tot.** Bei *Dynamic Distance
Modifier* und *Dynamic Shift* ließ sich der Schalter setzen, speichern und
laden — nur passierte nichts. Vier verwandte Effekte (Movement, Dynamic
Movement, Blitter Feedback, Roto Blitter) hatten ihn längst. Jetzt wirkt er
überall, und zwar mit derselben Rechenart wie das Original. **Die Vorgabe bei
Distance Modifier war zusätzlich verkehrt herum** und folgt jetzt AVS (aus);
Dynamic Shift bleibt an. Bestehende Effektketten laden unverändert.

**Skriptfehler blieben still.** Ein Tippfehler im Parameter-Skript verhielt sich
exakt wie ein leeres Feld. Fehler werden jetzt gemeldet — mit Slot und Zeile,
einmal je Fehler statt je Bild.

## Neu

- **Timescope: „Apply channel".** Der Kanalregler ist im Original wirkungslos
  (AVS berechnet ihn und benutzt ihn nicht). Er bleibt in der Vorgabe genauso —
  wer will, schaltet ihn jetzt wirksam.
- **Colorfade: die Beat-Regler sind skriptbar** (`beatfaderr`, `beatfaderg`,
  `beatfaderb`). Bestehende Presets ändern sich nicht.
- **Formel-Hilfe vollständiger.** Die Konstanten-Übersicht im Skript-Editor
  nennt jetzt auch `$PI`, `$E` und `$PHI` (den goldenen Schnitt) — die
  AVS-Schreibweise, die vorher fehlte.
- **Ältere Presets dürfen `pi` selbst definieren.** Der Editor markierte das als
  Fehler; tatsächlich tun es **629 von 3586** Presets der Sammlung, weil `$PI`
  in AVS erst später dazukam. Zwanzig davon meinen mit `pi` nicht einmal die
  Kreiszahl. Die Markierung ist weg, das Verhalten war immer richtig.
- **Hinweistexte** an allen sechs „Bilinear filtering"-Schaltern — sie erklären
  den Unterschied zwischen harten Kanten und weichen Übergängen.

## Unverändert

Modul-Matrix 36/41 und Modul-Sonden 78/80 — keine Bildveränderung an Presets
ohne Skript. Unit-Tests 482 grün.

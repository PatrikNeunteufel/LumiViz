# Changelog Session 129 (2026-10-16)

> **Aufräumen und nachmessen.** Die Arbeitsliste ist von Widersprüchen
> bereinigt, der Bildvergleich seit dem Stand vor dem Composer ist nachgeholt
> (nichts hat sich verändert), Bild ab / Bild auf laufen um, und Mess- und
> Testfenster nehmen keinen Fokus mehr. Gemessen wurde dabei ein Fehler im
> MilkDrop-Port: der Warp-Shader sieht die Y-Richtung gespiegelt — noch nicht
> behoben. Konzepte: `docs/ui/Hotkey_Konzept.md` §9, `docs/audio/Loop_Konzept.md`
> („Loop-Kleinkram“ 7e), `docs/Offene_Punkte.md`. Im Programm ist nichts gesehen.

## Geändert

- **Bild ab / Bild auf laufen am Ende des Ordners um** — nach dem letzten
  Preset kommt das erste, rückwärts vor dem ersten das letzte; die Statuszeile
  des Effect Browsers sagt es. Liegt im Ordner nur das eine Preset, das schon
  läuft, geschieht nichts. Handbuch §9.
- **F11 wirkt auch im Vollbild** und lässt sich unter *Settings → Hotkeys*
  umbelegen. Das Menü **View** zeigt bei *Fullscreen* und *Karaoke* die gerade
  eingestellte Taste. Handbuch §9.

## Intern

- Mess- und Testfenster auf Bildschirm 2 erscheinen ohne den Fokus zu nehmen
  (Standalones und Unit-Tests).
- Die Tests über „jeden mitgelieferten Loop“ lesen die Liste aus
  `asset/Asset.cmake` — Arbeitsdateien unter `asset/loop` prüfen sie nicht mehr.
- Vergleichslauf `vor-composer` gegen heute: AVS 252 / 252 und MilkDrop
  747 / 748 Bilder bitgleich (das eine ein Ausreißer eines abgebrochenen Laufs).
- Neue Sonden `asset/calibration/milkdrop/ysonde/` — die Warp-Sonde zeigt, dass
  der Warp-Shader `uv.y` gespiegelt sieht (Befund, offen).
- Blur gegen das Kalibrier-Raster nachgemessen: Abweichung nur in Blau (+8
  Stufen), Ursache offen.
- `Offene_Punkte.md` 1.249.0: 16 Widersprüche bereinigt; neue Ideen
  Saiteninstrumente, weitere Loop-Effekte, Vocals unterdrücken oder allein
  behalten.
- Voller Testlauf nach der Fokus-Änderung unter Release-clang und Testing-MSVC
  grün (1279 Fälle); danach ein Fall mehr (1280), die betroffenen 99 unter
  beiden Compilern grün; Debug-MSVC baut.

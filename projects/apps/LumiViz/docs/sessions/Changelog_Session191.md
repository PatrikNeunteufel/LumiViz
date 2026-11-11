# Changelog Session 191 (2026-11-11/12)

> Der Kern der durchschlagenden Zunge rechnet mit Vorgaben je Taste (34j-6) und liest den Wind über die Zeit (34j-7,
> zur Hälfte). Noch kein mitgeliefertes Instrument nutzt ihn (die Dateien kommen mit 34j-8).

## Neu

- `freeReed` Rechenfassung 2: jede Größe der Zunge, des Kanals, der Kanzelle und des Ventils als Kurve je Taste mit
  Belegkraft und Quellen; ein Windbereich je Instrument; f₀ am mittleren Druck gestimmt; Bass-Seite 8′ und 4′.
- Bend über das Ventil je Taste: der Stimmer misst die Kurve Öffnung → Cent und ihre Grenze, `pitch` folgt ihr.
- Laufender Wind: Expression, Pitch, Vibrato und Balgrichtung wirken über die Zeit; ein Instrument mit dauernder
  Anregung nimmt die Expression als Wind, nicht als Pegel.
- Elf Quellen der Klasse 10 im Quellenregister.

## Entschieden

- ZV8 a: Bend auf dem Raster 2^(−1/4) mit Grenze; ZV9 a: `pitch` wirkt schon am Beginn der Note.
- ZV10 a: die Anschlagstufe setzt die Stärke je Taste an der gemessenen Kurve (noch zu bauen).
- ZV11 a: der Nachklang wird je Taste gemessen (noch zu bauen).
- ZV12 a: der Befund „lauter wird dunkler“ wird vor 34j-8 untersucht.

## Bekannt

- Die tiefste Basszunge F1 schwingt bei 200 Pa nach 1 s noch an (benannte Ausnahme im Wächter).
- Mit mehr Wind wird der Ton an vielen Tasten dunkler statt heller — Befund, Untersuchung ZV12 a.
- Die Stufe wirkt am Kern noch nicht, der Nachklang ist noch fest 1 s.

## Geprüft

- Der Kern gibt die Zahlen des Prüfstands S190 wieder; Messung `docs/modellierung/zunge/proben/2026-11-11_S191_messung_34j6.md`.
- Eng 99 / 99 unter Release-clang und Testing-MSVC. Kein voller Lauf (kommt mit 34j-10).

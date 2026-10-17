# Changelog Session 130 (2026-10-16 / 17)

> **Befunde aus dem Sichttest 11 behoben, und MilkDrop-Shader sehen oben und unten wie im
> Original.** Der Limiter knackt nicht mehr, ausgeschaltete Clips stehen im Panel Player, Loops
> öffnen sich aus dem Composer, mehrfach gespielte Loops zeigen jede Stelle. Im MilkDrop-Port
> lasen Warp- und Comp-Shader die Y-Richtung verkehrt — behoben, der alte Look bleibt je Pass
> wählbar. Konzepte: `docs/ui/Composer_Konzept.md` §9f–§9h, `docs/audio/Loop_Konzept.md`
> („Die Marke läuft mit“), `docs/Offene_Punkte.md` §3. Im Programm ist nichts gesehen.

## Neu

- **Rechtsklick auf einen Loop-Clip → *Open in Loop panel*** — auch ohne *Edit*. Hat das Panel
  Loop Ungespeichertes, fragt es wie bei *Load...*. Handbuch §15.
- **Ausgeschaltete Clips im Panel Player:** liegt an der Stelle nur ein ausgeschalteter Clip,
  steht dort „Composer - <Titel> (off)“, der Balken läuft durch ihn. Tag View und Karaoke bleiben
  beim letzten gespielten Song. Handbuch §15.
- **MilkDrop: *Warp shader Y* und *Comp shader Y*** — die Shader sehen `y` wie MilkDrop
  (Standard); je Pass lässt sich der Look bis Session 129 zurückholen. Hat ein Preset beide
  Shader, beide Zeilen gleich stellen. Handbuch, Sammelliste der „bisher“-Schalter.

## Geändert

- **Der Limiter schaut 5 ms voraus** und knackt nicht mehr; mit gesetztem Haken kommt der Ton
  5 ms später. Handbuch §15.
- **Mehrfach gespielte Loops:** spielt der Composer denselben Loop mehrfach zugleich, zeigen Panel
  Loop und Editoren jede Stelle, die leiseren blasser. Liegt ein Muster im Loop zugleich in
  mehreren Kanälen, zeigt sein Editor jede Stelle — voll die des zuletzt begonnenen Clips.
  Handbuch §16.
- **MilkDrop-Presets mit eigenen Shadern** sehen anders aus — näher am Original: Verläufe,
  Wirbel und Versätze im Warp zeigen in die richtige Richtung, alles, was ein Comp-Shader liest,
  steht aufrecht.

## Behoben

- Der Knopf **Resume after restart** im Panel Composer ließ sich nicht ausschalten (er blieb
  blau).

## Intern

- Neun Y-Sonden unter `asset/calibration/milkdrop/ysonde/` mit `ysonde_auswerten.py` — Urteil am
  Bild, weil die MAE ein gespiegeltes kleines Quadrat als „OK“ meldet.
- MilkDrop-Vergleichslauf (22 Presets, 6 Formate): mit den alten Schaltern 748 / 748 Bilder
  bitgleich zum Stand von S129; mit dem neuen Standard die Starfield-2-Familie nicht mehr
  formatabhängig, Twisted und Playaround deutlich näher an der Referenz.
- HlslTranspiler 1.6.0 (`referenzRaum`); Feld-Inventar um `warpYOriginal`/`compYOriginal`.
- Sichttest-Vorlage L wiederhergestellt.
- Voller Testlauf unter Release-clang und Testing-MSVC grün (1288 Fälle).
- `Offene_Punkte.md` 1.250.0: neu 🔴 *Starfield (Supernova-Lens-Effect edit)* in beiden Ständen
  fast schwarz.

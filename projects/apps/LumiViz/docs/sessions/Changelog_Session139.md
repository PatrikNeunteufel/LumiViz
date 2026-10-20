# Changelog Session 139 (2026-10-19 bis 2026-10-20)

> **Gleich laut.** Jedes mitgelieferte Instrument klingt jetzt gleich laut: Volume 1 an einem
> Muster heißt bei jedem dasselbe. Gemischt wird mit einem Fader je Kanal im Loop-Panel, ein
> Limiter am Ausgang fängt Spitzen ab, und *Match* im Creator bringt eigene Instrumente auf
> denselben Stand. Die mitgelieferten Loops klingen wie vorher; die Kits sind untereinander gleich
> laut. Dazu ein neues Konzept für die Spielweisen der anderen Instrumentfamilien (Pedale, Dämpfer,
> Vibrato …).
> Steuerdokumente: `docs/audio/Loop_Konzept.md` (§4n LL1 a bis LL14 a, §3.2),
> `docs/audio/Spielweisen_Konzept.md` (neu, SF1 c bis SF5 d).

## Neu

- **Fader je Kanal** im Loop-Panel: rechts im Kopf jedes Kanals, von −∞ bis +24 dB. Senkrecht
  ziehen oder das Mausrad (0,5 dB, mit Shift 0,1 dB), Doppelklick zum Tippen, Ctrl+Klick setzt
  0 dB; die Zeile unter der Achse nennt die Gesten. Er wirkt nach den Effekten der Clips.
- **Limiter** im Loop-Panel (Haken vor *Scroll with playhead*, Vorgabe an, gehört zum Loop): hält
  die Summe unter 0 dB, solange das Panel spielt. Im Composer gilt dessen eigener Limiter.
- **Match** im Instrument-Creator neben *Level*: stellt das Instrument so laut wie die anderen;
  rechts in der Zeile *Tuning* steht, wie weit es abweicht. Beim **Save** und **Save as...** fragt
  der Creator, wenn es mehr als 1 dB abweicht: *Match and save*, *Save as it is*, *Cancel*.
- Im Fenster **Instruments** steht neben eigenen Instrumenten, die mehr als 1 dB abweichen, die
  Abweichung (etwa „+6.0 dB").

## Geändert

- **Die mitgelieferten Instrumente sind gleich laut** (gemessen nach EBU R128 mit einer Tonleiter
  und einem Akkord). Die mitgelieferten Loops klingen dabei wie vorher; wo ein Muster dafür lauter
  als 1 sein müsste, steht der Überschuss im Fader seines Kanals.
- **Die mitgelieferten Kits sind gleich laut**: die Kick jedes Kits gleich stark, jede andere Rolle
  im selben Abstand zur Kick wie in den anderen Kits. Drei Klänge erreichen ihr Ziel nicht (Crash in
  Rock'n'Roll, Snare in Strange, Clap in Lo-fi) und klingen etwas zurückgenommen.
- **Level** des ganzen Instruments im Creator in dB, bis +24 dB (vorher bis 100 %).
- `.lvloop` Fassung 11 (`gain` je Kanal, `limiter`), `.lvinst` Fassung 13 (`level` über 1).
- Benutzerhandbuch 1.196.0: Fader, Limiter, Match, Kits gleich laut, Level in dB.

## Intern

- Die Lautheitsmessung liegt unter `audio/Lautheit` (vorher `composer/`) und kennt das Maximum der
  Momentary- und Short-term-Lautheit (EBU Tech 3341, Fälle 1, 2, 9, 10, 12, 13).
- `loop/Pegel` (Standardprobe, Drum-Probe, Abstand zum Bezug); `lumi::ui::Achse::kopfBreite()`.
- `asset/loop/instrument_pegel.py` (Pegel der mitgelieferten Instrumente, Nachziehen der Loops);
  `asset/loop/kits/make_kits.py` misst nach R128 (`--messung`).
- Neue Tests `test_LoopPegel`, `test_LoopFader`; Fälle in `test_ComposerLautheit`,
  `test_LoopArrangement`, `test_LoopSpieler`, `test_LoopKits`; 1412 Fälle. Zuletzt Loop-Bereich
  360 / 360 und Composer 150 / 150 unter beiden Compilern; **kein voller Lauf** (Patrik: noch nicht).
- `Loop_Konzept.md` 0.99.0, `Spielweisen_Konzept.md` 0.5.0, `Composer_Konzept.md` 0.98.0,
  `INDEX.md` 2.5.0, `Offene_Punkte.md` 1.266.0.

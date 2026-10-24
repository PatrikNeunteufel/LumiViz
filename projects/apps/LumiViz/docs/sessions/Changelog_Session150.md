# Changelog Session 150 (2026-10-24)

> **Der Flügel klingt durch seinen Resonanzboden.** Der gerechnete Boden des Steinway D ist geprüft, und ein Ton der
> Hammer-Saite kann jetzt durch ihn klingen — in Stereo, an zwei Hörorten über den Saiten. Ein mitgeliefertes
> Instrument nutzt das noch nicht (das kommt mit `grand_piano` in 31h).
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (PF2 a, PF3 a, PS2 a, PS3 a, KV2 a, PS4 a, FF1 a).

## Neu

- **Resonanzboden im Klang:** eine Hammer-Saite kann einen Körper nennen (`lib:steinway_d`); ihr Ton wird dann mit der
  Antwort des Bodens gefaltet, klingt links und rechts verschieden und um die Nachklangzeit des Bodens länger. Die
  Antwort einer Taste entsteht beim ersten Ton und liegt danach im Tonspeicher.
- **Instrumente Fassung 17:** der Resonanzboden trägt nur noch den Verweis auf den Körper (`soundboard.body`); das
  frühere `amount` fällt weg (es wirkte nie), Dateien der Fassung 16 bleiben lesbar. Fehlt ein Körper, sagt der Bericht
  „Body not found“, und der Ton klingt ohne Boden.

## Intern

- **pocketfft** (BSD-3) als External für die Faltung, hinter der eigenen Schnittstelle `loop/Fft`; Vermerk mit vollem
  Lizenztext in `THIRD_PARTY_NOTICES.md` (geht in die App).
- Bibliothek `InstrumentBody`: `Antwort.hpp` (Druck am Hörort je Kraft am Steg, im Spektrum der FFT); `kDiskantAb`
  = 41 (Modul-Doku 0.12.0).
- **Prüfstein des Bodens:** Prüffall nach MAESSTRO unter den Testdaten (`make_steinway_d.py --maesstro`, zwei Stege,
  17 Rippen), Konvergenz gegen eine Rechnung höherer Ordnung, Vergleich mit Messungen; Bezugskurven aus MAESSTRO
  Abb. 5 und 6 (`maesstro_ablesen.py`). Recherche zum Quellcode von MAESSTRO unter `docs/modellierung/`.
- Neue Tests `test_LoopKoerperPruefstein` (3), `test_LoopFft` (4), `test_LoopKoerperKlang` (6), Fälle in
  `test_LoopKoerperBauplan` und `test_LoopHammerSaite`; `werte/klangfassung.txt` neu (nur Abdrücke); 1576 Fälle.
  Eng grün unter Release-clang, Testing-MSVC baut; **kein voller Lauf** (kommt mit dem Abschluss von 31c, 31c-9).

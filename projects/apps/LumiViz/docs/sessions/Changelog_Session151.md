# Changelog Session 151 (2026-10-24/25)

> **Der Resonanzboden ist fertig, und der Rest des Flügels ist geplant.** Stufe 31c ist abgeschlossen: zwei Böden aus
> Glas kommen als gedachte Instrumente dazu, die Antwort des Bodens klingt bis zum Ende aus und entsteht rund
> fünfzehnmal schneller. Danach ist der Weg bis zum fertigen Flügel (31d–31h) Schritt für Schritt festgelegt.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (RS1 a, AW2 a, AW3-1 a, UF1 a bis UF17 a, IK1).

## Neu

- **Zwei Resonanzböden aus Glas** (mitgeliefert): der Steinway D mit einer Platte aus Floatglas, einmal so dick wie
  Holz, einmal so schwer wie Holz (`steinway_d_glass`, `steinway_d_glass_same_mass`). Gedachte Instrumente — Bruch
  unter der Saitenlast ist nicht gerechnet. Ein Instrument nutzt sie ab 31h.

## Verbessert

- **Die Antwort des Bodens klingt bis −60 dB aus**, ohne Grenze bei 8 s: ein Boden aus Glas wurde vorher hart
  abgeschnitten, und was länger klang, faltete sich auf den Anfang zurück.
- **Die Antwort entsteht 14- bis 19-mal schneller** (auf einem Raster, schmale Resonanzen exakt; Abweichung unter
  −96 dB): alle 88 Tasten des Steinway in 3,4 s statt 43 s, des dünnen Glasbodens in 18 s statt 11 min.

## Intern

- Eigenlöser: eine Mode, deren Rest über der Grenze liegt, wird nachgeschärft statt abzubrechen; das Werkzeug meldet
  die Zahl („sharpened“).
- Varianten des MAESSTRO-Prüffalls (MP1–MP3) als Testdaten gerechnet; die Richtungen aus MAESSTRO bestehen.
- `make_steinway_d.py` schreibt Glasböden und Varianten mit; `InstrumentBody.md` 0.14.0.
- Neue Fälle in `test_LoopKoerperBauplan`, `test_LoopKoerperEigen`, `test_LoopKoerperPruefstein`,
  `test_LoopKoerperKlang`; Messteil und Hörproben für alle Körper; 1582 Fälle.
- **Voller Lauf 1582 / 1582 unter Release-clang und Testing-MSVC** (je rund 6 min) — der erste seit Stufe 26.
- Planung 31d–31h: Messung am *Salamander Grand Piano* (CC BY 3.0), Mitschwingen, Spielweisen, Creator, Abschluss mit
  `grand_piano`, Glasflügeln und drei Chopin-Loops.

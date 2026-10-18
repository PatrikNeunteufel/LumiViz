# Changelog Session 133 (2026-10-18)

> **Saiteninstrumente, zweiter großer Teil: die Oberfläche und die mitgelieferten
> Instrumente.** Der Instrument-Creator stellt jetzt Stimmung, Saite, Korpus und Tonabnehmer
> ein, der Instrument-Editor zeigt, auf welcher Saite jede Note spielt. Gitarren, Bässe und das
> Clavinet spielen auf den neuen Saiten; die alten bleiben als „(old)“. Dazu kommen ein zweiter
> Korpus, sechs neue Instrumente und Loops zum Vergleichen. Steuerdokument:
> `docs/audio/Saiten_Konzept.md`.

## Neu

- **Korpus Classical guitar:** Decke, Boden und Luft nach den Werten aus H. Wrights Thesis
  (1996, Satz TOP26). Seine Spitzen liegen dort, wo Wright sie nennt.
- **Im Instrument-Creator:**
  - die Zeile **Tuning** mit Strings, den Tönen der leeren Saiten, Capo, Scale und String choice;
  - **Model** unter Waveform: Classic, Extended KS, Waveguide;
  - die Gruppe **String** mit einem Saitenbild, in dem sich Anschlag und Tonabnehmer mit der Maus
    ziehen lassen;
  - die Gruppen **Body** und **Pickups** (Pickup select, Type, Position, Cable mit Resonanz).
- **Im Instrument-Editor**, bei einem Instrument mit Saiten:
  - **String choice** für das Muster;
  - die Nummer der Saite in jeder Note, gezählt wie am Instrument (1 = die hohe Saite);
  - eine Nadel, wenn du die Saite gesetzt hast;
  - nicht greifbare Noten rot schraffiert mit „!“;
  - Punkte auf den Tasten der leeren Saiten;
  - im Menü einer Note das Untermenü **String**.
- **Sechs neue Instrumente:** Jazz Guitar, Finger Bass, Ukulele, Drop D Guitar, Pizzicato Violin,
  Piezo Guitar.
- **Loops:** vier Beispiele (`jazz_guitar_demo`, `ukulele_demo`, `drop_d_demo`,
  `pizzicato_demo`) und unter `asset\loop\vergleich` fünf Vergleiche, die dasselbe Stück erst mit
  den alten, dann mit den neuen Instrumenten spielen.

## Geändert

- **Guitar, Nylon Guitar, Clean Guitar, Muted Guitar, Overdrive Guitar, Upright Bass, Slap Bass
  und Clavinet** spielen auf den neuen Saiten, je Spur so laut wie vorher. Die Fassungen davor
  heißen „… (old)“ und klingen wie bisher.
- **Tonabnehmer** sind so laut wie die Saite am Steg; vorher waren sie durch einen Fehler 17 bis
  20 dB leiser.
- Benutzerhandbuch 1.189.0.

## Intern

- Neue Erzeuger `asset/loop/make_saiten_instrumente.py` und `make_saiten_loops.py`;
  `instrument_katalog.py` behält jetzt die Saiten einer Datei.
- Neue Tests: `test_LoopCreatorTuning`, `test_LoopCreatorSaite`, `test_LoopEditorSaiten`; dazu
  Fälle in `test_LoopKorpus` und `test_LoopSaiteWg`. 1334 Fälle.
- Loop Werte: nur die geänderten und neuen Instrumente und Loops sind neu, die alten Fassungen
  Bit für Bit wie vorher.
- `Offene_Punkte.md` 1.260.0. Der volle Lauf kommt, wenn Tabs mit Akkorden möglich sind.

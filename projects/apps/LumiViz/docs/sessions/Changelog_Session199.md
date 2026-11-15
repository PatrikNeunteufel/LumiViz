# Changelog Session 199 (2026-11-15)

> Die Körper der fünf gezupften Instrumente sind vollständig: die Harfe hat an jeder Taste einen Körper, die Gitarre
> ist als erster gemessener Körper dazugekommen. Stufe 34k steht damit bei 34k-6 von elf, der Herleitung des
> Rechengesetzes; sie läuft an einem Prüfstand außerhalb des Programms. Im Programm ist davon noch nichts zu hören —
> kein Instrument benutzt die neuen Körper.

## Neu

- **Die Gitarre als gemessener Körper** (KQ23 a–KQ26 a): `asset/loop/instrument_bodies/classical_guitar.lvinstbody`
  mit der Beschreibung `classical_guitar.lvinstbodyfit`, geschrieben von `make_gitarre.py <qualified_selected_impulses.mat>`
  (numpy, scipy; rund 3,5 min; die Messdatei liegt im Archiv). Modalfit an der Gitarre g34 des Archivs Mores 2021
  (Manuel Contreras, Madrid 1971): 75 Moden bis 5 kHz, sechs Stegpunkte (je leere Saite), Hörorte `left` / `right`
  10 cm über der Decke. Über 5 kHz ein statistischer Teil bis 20 kHz, dessen Größen an der Messung bestimmt sind.
- **`.lvinstbody` Fassung 4** (KQ25 a): `origin` (`plan` oder `measurement`) und `measurement` (Datei, Abdruck,
  Fassung des Fits), beides immer geschrieben; Fassung 3 liest sich als `plan`. Ein gemessener Körper hängt an
  seiner Beschreibung und an `kFitFassung` (2); der Wächter prüft beides.
- `InstrumentBodyTool <körper> --rewrite [--out <datei>]` schreibt einen Körper in der heutigen Fassung neu, ohne zu
  rechnen.
- **Prüfstand für 34k-6** unter `docs/modellierung/gezupfte-saite/proben/`: `2026-11-15_S199_pruefstand_zupf.py`
  rechnet den Ton einer Note geschlossen je Frequenz (Saite, Zupf, Steg und Abstrahlung aus der Körperdatei, zwei
  Polarisationen, Chöre, Mitschwingen) für Gitarre, Laute, Oud, Harfe und Cembalo; Gegenproben in
  `…_pruefstand_pruefen.py`; dazu Proben zu Ausklang und Stärke je Teilton an den Iowa-Aufnahmen und zu den Formen
  des Stegs.
- Bericht `docs/modellierung/gezupfte-saite/2026-11-15_S199_gitarre_modalfit.md`.

## Geändert

- **Die Harfe trägt je chromatischer Taste einen Stegpunkt** (KQ22 a): 80 Punkte an 46 Orten, Tasten 25–104; die
  schwarze Taste an der Öse der Saite darunter. Vorher hätten 34 Tasten im Umfang keinen Körper gehabt.
- Die sieben Körper mit Bauplan stehen in Fassung 4 (gleich bis auf `version`, `origin`, `measurement`).
- Der Wächter über die mitgelieferten Körper verlangt `classical_guitar` und prüft einen statistischen Teil eines
  gemessenen Körpers gegen `ziehe`.
- `InstrumentBody` 0.22.1, Konzept 0.43.0, Offene Punkte 1.369.0.

## Berichtigt

- „Der Ton fällt am Körper 1,5–2,2-mal so schnell wie die Iowa-Gitarre“ (Stand vom Nachmittag) stammte aus der
  Hüllkurve und gilt nicht: je Teilton gemessen fällt das Modell bei den Teiltönen 2–8 langsamer als die Aufnahme.
- Die erste Zahl des Prüfstands zur Gitarre (13,3 LU) maß ein Artefakt am Fensterende.

## Entscheide

KQ22 a (Stegpunkt je Taste der Harfe), KQ23 a (Vorbild g34), KQ24 a (Abstrahlung der Gitarre gemessen und geteilt),
KQ25 a (gemessener Körper, Fassung 4), KQ26 a (statistischer Teil der Gitarre an der Messung), KQ27 a (die Form des
Körpers im Kern leitet 34k-6 her; 34k-5 gilt mit den fünf Körpern als fertig), GZ5 a (alle Moden der Körperdatei in
der Saitenschleife, der Kern gibt die Stegkraft aus, die App faltet wie beim Flügel), GZ5-1 a (über 5 kHz ein
Dämpfer aus den Größen des statistischen Teils), GZ6 a (Gitarre: die Stufe setzt die Auslenkung und kippt das
Spektrum des Zupfs), GZ7 a (die App reicht die Moden des Körpers mit dem Tonauftrag; kein Stegmoden-Werkzeug).

## Tests

doctest zählt 1920 Fälle. Kein voller Lauf: geändert sind Lesen und Schreiben der Körperdatei, die Körper und der
Wächter — das lesen nur die Körper-Tests und die Töne mit Boden. Gelaufen unter Release-clang und Testing-MSVC:
`*Loop Koerper*,*Loop Werte*,*Klangfassung*,*Hammer*` ohne `*KQ18*,*KQ20*` 108 / 108, danach `*31c-5d*` 3 / 3 (zweimal,
nach KQ25 a und nach KQ26 a). Die Herleitung 34k-6 ändert am Programm nichts.

## Offen

- 34k-6: Tastenstoß des Cembalos, Mitschwingen und Auswahl der Harfensaiten, Zeitbereichs-Kandidat (Rechenzeit,
  Dämpfer, Bend und Slide ±10 Cent), Vorlagen je Instrument (Schwellen für Spanne und Sprünge, Zupf), Dokument der
  Herleitung.
- Gitarre: zweite Richtung am Steg, Nahfeld gegen 1 m, dichte Paare über 300 Hz, Stegadmittanz an den mittleren
  Saiten über 2,5 kHz, die Gitarre in der Körper-Auswahl der App.
- Harfe: Diskantende, S(1,1)−, höhere Luftmoden; Zug der Metallsaiten.

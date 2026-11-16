# Changelog Session 200 (2026-11-16)

> Die Herleitung des Rechengesetzes der fünf gezupften Instrumente (Stufe 34k-6) ist ein großes Stück weiter: das
> Cembalo hat Tastenstoß, Lautenzug und ein Kraftgesetz über den Umfang, das Mitschwingen der Harfe hat eine Regel, und
> ein Zeitbereichs-Kandidat für den Kern trifft den Prüfstand. Fertig ist sie nicht. Im Programm ist davon noch nichts
> zu hören — die Sitzung hat nur Proben und Entscheide geschrieben, kein Programmcode ist geändert.

## Neu

- **Proben zum Cembalo** unter `docs/modellierung/gezupfte-saite/proben/`: `2026-11-16_S200_cembalo_register.py`
  (Tastenstoß, Lautenzug, zweites 8′ und 4′) und `…_cembalo_4fuss_plan.py` (Probe-Bauplan mit Stegpunkten auf beiden
  Stegen).
- **Mitschwingen der Harfe** gegen alle 46 Saiten: `…_harfe_mitschwingen.py`.
- **Zeitbereichs-Kandidat** für den Kern: `…_zeitbereich_kandidat.py` (Saitenschleife im Zeitschritt mit allen Moden
  der Körperdatei am Steg), `…_zeitbereich_rechenzeit.cpp` (Rechenzeit), `…_zeitbereich_bend.py` (Bend und Slide).
- **Messungen an Aufnahmen:** `…_vcsl_cembalo_stoss_lautenzug.py` (Tastenstoß, Tonhöhe des Lautenzugs),
  `…_grundton_gegen_teiltoene.py` (Grundton der Gitarre nahe den Körpermoden).
- `…_umlauf_filter.py` (der Umlauf der Saite mit wenigen Filterwerten), `…_gang_ueber_den_umfang.py` (Form der
  Lautheit über den Umfang, Modell gegen Aufnahmen).
- **Recherche-Werkbank** außerhalb des Repos (`…\cmake\LumiViz_Research`): Aufträge der Sitzung, Läufe über die
  Kommandozeile von Claude Code, Berichte zum Abholen. Erster Auftrag: Geometrie des Alphorns (Vorbereitung 34m).
  Noch kein Lauf — die Abrechnung über das Guthaben ist ungeklärt.

## Geändert

- Konzept `docs/audio/Instrumente_IC_Konzept.md` 0.46.0, Offene Punkte 1.378.0 (§6, Zeile 34k, Punkte (8)–(20)).

## Entscheide

GZ8 a (Harfe schwingt mit Oktaven, Quinten und Quarten mit; die Rechenzeit-Grenze 4,6 % gilt für eine Saite ohne
Mitschwingen), GZ9 a (Haltepunkt Bend bestanden), GZ10 a (Tastenstoß des Cembalos 7 mN s bei ff, 10 dB je Stufe;
Ablösekraft 2 N), GZ11 a (Lautenzug als Dämpfer am Sattel, 40 N s/m bei 0,5 mm), GZ12 a (`harpsichord` mit einem 8′;
weitere Register mit 31g), GZ13 a (Ablösekraft des Cembalos −2,8 dB je Oktave), GZ14 a (Spanne ≤ 15 LU — Harfe ab D2 —,
mittlerer Sprung ≤ 4 LU, größter ≤ 12 LU; ersetzt die Schwellen 15 LU / 6 LU aus GZ2 a).

## Befunde

- Der Tastenstoß lässt sich aus Penttinen 2006 nicht eichen; die VCSL-Aufnahmen entsprechen rund 5 mN s.
- Die Diskretisierung der Stegmoden im heutigen Kern trägt den Dämpfer über 5 kHz nicht; „genau für stückweise lineare
  Kraft“ trifft den Prüfstand.
- Rechenzeit (Anhalt): eine Saite 0,8–3,5 % der Echtzeit; mit Mitschwingen liegt nur die Gitarre unter 4,6 %.
- Die echte Gitarre verstimmt den Grundton nahe ihren Körpermoden um 10–17 Cent, das Modell rund 1,7-mal so stark.
- Harfe: drei Tasten um G3 sind 8–13 LU zu laut (Körpermode bei 208 Hz). Gitarre: der größte Sprung (13,8 LU) folgt
  der Abstrahlung des Körpers zwischen 880 und 988 Hz.

## Offen

Harfe (Mode 208 Hz), Gitarre (Abstrahlung 700–1000 Hz, Nahfeld gegen 1 m, Zupfort an hohen Bünden), Entwurf des
Umlauf-Filters je Taste, das Dokument der Herleitung; danach 34k-7 (Kern Rechenfassung 2).

## Tests

Keine gelaufen: am Programm ist nichts geändert. doctest zählt weiter 1920 Cases (Stand S199).

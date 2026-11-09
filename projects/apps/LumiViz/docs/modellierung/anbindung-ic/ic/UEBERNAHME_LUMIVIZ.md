# Übernahme nach LumiViz — was ignoriert werden muss, wo Lizenzen zu nennen sind

> **Angelegt:** 2026-10-26, IC Session 2, auf Anweisung Patrik: Dieses Repo ist privat, wird nie öffentlich und ist nur
> vorübergehend; es wird später in LumiViz integriert. Diese Datei hält fest, was bei der Integration in LumiViz **nicht
> versioniert** werden darf und **wo Lizenzen genannt** werden müssen. **Je neuem fremdem Datensatz eine Zeile in §3.**
> **Geändert 2026-10-26 (A-1 a):** Audio und große Messdaten liegen **nicht mehr im Repo**, sondern lokal unter
> `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` — außerhalb von InstrumentsCalculation und LumiViz, für beide
> erreichbar. Im Repo stehen je Datensatz nur `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`.

## 1. Regeln

1. **Fremde Dateien werden in LumiViz nicht versioniert** — PDFs, Audio, Messdaten. Gründe: Rechtslage (viele nicht
   frei weitergebbar), Größe. Versioniert werden nur die eigenen Texte (Berichte, `quellen.md`, `aufnahmen.md`,
   `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`), damit sich jede Datei wiederbeschaffen und prüfen lässt.
2. **Lizenzen nennen**, sobald etwas aus einer fremden Datei in LumiViz **mitgeliefert** wird (Klang, Werte in einer
   Instrument- oder Korpusdatei, Abbildung): im `CREDITS.md` des Asset-Ordners, der es ausliefert (Vorbild:
   `asset/loop/samples/CREDITS.md`, `asset/loop/chords/CREDITS.md`), mit Quelle, Autor, Lizenz und was verändert wurde.
   Für Werte, die nur in Dokumenten stehen, genügt die Angabe in der `quellen.md` der Klasse (wie heute).
3. **CC0** verlangt keine Nennung — sie wird trotzdem in `CREDITS.md` geführt (Herkunft). **CC BY** verlangt Nennung
   von Autor, Titel, Lizenz und Änderungen. **NC, ND, SA, „All rights reserved“** → nichts davon ausliefern; nur Verweis.

## 2. Ignorieren in LumiViz (`.gitignore`)

**Schon da** (LumiViz `.gitignore`, Zeile 470–471, nachgelesen 2026-10-26):

```
# fremde Arbeiten (PDFs) zur Modellierung: nur lokal, je Klasse unter quellen/ (S146)
/projects/apps/LumiViz/docs/modellierung/**/quellen/
```

→ Alles unter `klassen/<Klasse n - Name>/quellen/` und `gemeinsam/quellen/` fällt bei der Integration nach
`docs/modellierung/<klasse>/quellen/` von selbst heraus.

**Audio und Messdaten:** liegen nicht im Repo, also auch nach der Integration nicht in LumiViz — nur die Begleitdateien
in `aufnahmen/<Datensatz>/` und `daten/<Datensatz>/` gehen mit und dürfen versioniert werden. Als Sicherung gegen ein
versehentliches Hineinkopieren (Vorschlag, wie hier in `.gitignore`):

```
# Audio und Messdaten zur Modellierung liegen unter ...\cmake\samples\ (IC, A-1 a); hier nur Begleitdateien
/projects/apps/LumiViz/docs/modellierung/**/aufnahmen/**
/projects/apps/LumiViz/docs/modellierung/**/daten/**
!/projects/apps/LumiViz/docs/modellierung/**/aufnahmen/**/
!/projects/apps/LumiViz/docs/modellierung/**/daten/**/
!/projects/apps/LumiViz/docs/modellierung/**/HERKUNFT.md
!/projects/apps/LumiViz/docs/modellierung/**/SHA256SUMS
!/projects/apps/LumiViz/docs/modellierung/**/LADELISTE.tsv
```

Die Regel trifft nur die **Ordner** `aufnahmen/` und `daten/`, nicht die Datei `aufnahmen.md`. Vor dem Einspielen mit
`git check-ignore -v <pfad>` prüfen. Die Klassenordner heißen hier „Klasse n - Name“; in LumiViz heißen sie heute
`modellierung/hammer-saite/` usw. — beim Zusammenführen entscheiden, welche Benennung gilt.

## 3. Fremde Dateien und Datensätze

| Ort | Inhalt | Lizenz (Wortlaut/Feld, Ort) | In LumiViz ignorieren | Lizenz nennen wo, wenn mitgeliefert |
|---|---|---|---|---|
| Repo `gemeinsam/quellen/Mores_2021_Method_…pdf`, `…_Comments_…pdf`, `…_ListOfGuitarsPhysicalMeasures_…pdf` | Doku des Mores-Archivs | CC BY 4.0 (Zenodo-API `cc-by-4.0`) | ja (Regel `quellen/`) | nur Verweis in `quellen.md`; ausgeliefert wird nichts davon |
| Repo `gemeinsam/quellen/Weinzierl_et_al_2017_…_DepositOnce19858.pdf` | Doku TU-Berlin-Datenbank | CC BY-NC-SA 4.0 (Titelblatt, S. 2); Datensatz „In Copyright“ | ja | **nichts ausliefern** (NC; F-D1 offen) |
| Repo `klassen/Klasse 2 - Gezupfte Saite/quellen/Chadefaux_…_SMAC2013.pdf` | Kiel am Cembalo | CC BY 3.0 (S. 155) | ja | `CREDITS.md` des Asset-Ordners, falls Kielwerte in einer mitgelieferten Datei stehen |
| Repo `klassen/Klasse 2 - Gezupfte Saite/quellen/Bader_Plath_Kontopidis_2025_…_arXiv2511.09037.pdf` | Dulcken-Cembalo, Resonanzboden | CC BY 4.0 (arXiv-HTML-Kopf) | ja | `CREDITS.md` des Korpus-Ordners, falls ein Cembaloboden nach diesen Werten ausgeliefert wird |
| Repo `klassen/Klasse 3 - Bogen-Saite/quellen/Russo_Ducceschi_Bilbao_2022_…_DAFx20in22.pdf` | Bogen-Saite modal | CC BY 4.0 (S. 122) | ja | `CREDITS.md` des Asset-Ordners, falls Werte daraus ausgeliefert werden |
| Repo `klassen/Klasse 3 - Bogen-Saite/quellen/Maestre_Scavone_Smith_2013_…_SMAC2013.pdf` | Stegadmittanz Violinfamilie (Verfahren) | CC BY 3.0 (S. 1) | ja | wie oben |
| `samples/Klasse 1 - Hammer-Saite/K18_Upright_Piano/` (212 WAV, `.sfz`, `readme.txt`; 940 MB) | Upright, 6 Stärken | CC BY 4.0 (archive.org `licenseurl`; `readme.txt` „Licence: CC BY 4.0“) | liegt außerhalb; Begleitdateien im Repo | **Pflicht**, sobald ein daran gesetztes Upright (Stufe 32, `upright_piano`) ausgeliefert wird: `CREDITS.md` des Instrument-Ordners — „K18 Upright Piano by Rudi Fiasco, archive.org/details/K18UprightPiano, CC BY 4.0; used for measurement“ (wie Salamander beim Flügel) |
| `samples/Klasse 1 - Hammer-Saite/VCSL/` (Upright Knight, Yamaha; 290 Dateien, 638 MB) | Gegenprüfung Upright | CC0 1.0 | liegt außerhalb | keine Pflicht; Herkunft in `CREDITS.md`, falls etwas daraus ausgeliefert wird |
| `samples/Klasse 2 - Gezupfte Saite/Mores_2021_Zenodo4604577/` (`qualified_selected_impulses.mat`, PDFs, `.m`) | Stegmobilität über 60 klassischer Gitarren | CC BY 4.0 (Zenodo-API) | liegt außerhalb | **Pflicht**, sobald ein gefitteter Korpus ausgeliefert wird (Klasse 2 F-2 a): „Robert Mores, Archive … bridge mobility, Zenodo 10.5281/zenodo.4604576, CC BY 4.0; modal parameters fitted by LumiViz“ |
| `samples/Klasse 2 - Gezupfte Saite/VCSL/` (Harfen, Cembali; 566 Dateien, 716 MB) | Bezugsaufnahmen | CC0 1.0 (`LICENSE`, GitHub `spdx_id`) | liegt außerhalb | keine Pflicht; Herkunft in `CREDITS.md`, falls etwas daraus ausgeliefert wird |
| `samples/Klasse 2 - Gezupfte Saite/Iowa_MIS_Guitar_mono2496/` (45 AIFF, 828 MB) | Nylongitarre, schalltot, pp/mf/ff | keine Standardlizenz: „may be downloaded and used for any projects, without restrictions“ (`MIS.html`) | liegt außerhalb | vor jeder Auslieferung schriftliche Bestätigung von Iowa einholen; dann „University of Iowa Electronic Music Studios, Musical Instrument Samples“ nennen |
| `samples/Klasse 3 - Bogen-Saite/Iowa_MIS_Strings2012_stereo2496/` (264 AIFF, 8,09 GB; ersetzt mono, das als Mittel (L+R)/2 ableitbar ist — F-L5-1 a) | Vl, Va, Vc, Kb, schalltot, arco/pizz, 2 Kanäle | keine Standardlizenz: „without restrictions“ (`MIS.html`) | liegt außerhalb | wie Iowa-Gitarre: vor jeder Auslieferung schriftliche Bestätigung von Iowa |
| `samples/Klasse 3 - Bogen-Saite/TinySOL_Strings/` (1 193 WAV, CSV; 754 MB) | Ircam-Einzeltöne Streicher | CC BY 4.0 (Zenodo-API) | liegt außerhalb | **Pflicht**: Cella, Ghisi, Lostanlen, Lévy, Fineberg, Maresz, „TinySOL“, Zenodo 3685367 |
| `samples/Klasse 3 - Bogen-Saite/TU-Note_Violin/` (2 080 Dateien; 2,26 GB) | Violine, Übergänge, Segmentierung | CC BY-ND 4.0 (`dc.rights.uri`) | liegt außerhalb | **nichts ausliefern** (ND) — nur messen |
| `samples/Klasse 3 - Bogen-Saite/VSCO-2-CE_Strings/` (744 WAV; 720 MB) | Solo Violin/Contrabass, Sections | CC0 1.0 (`LICENSE`, GitHub `spdx_id`) | liegt außerhalb | keine Pflicht; Herkunft in `CREDITS.md`, falls etwas daraus ausgeliefert wird |
| `samples/Klasse 3 - Bogen-Saite/CNSM_Zenodo18696786/` (462 Dateien; 1,83 GB) | Stegadmittanz dreier Violinen, Bewegung, Audio | CC BY 4.0 (Zenodo-API) | liegt außerhalb | **Pflicht**, sobald ein Violinkorpus nach diesen Werten ausgeliefert wird: Pauget Ballesteros, „CNSM Dataset“, Zenodo 18696786 |
| `samples/Klasse 3 - Bogen-Saite/Lampis_Zenodo13374477/` (37 295 WAV; 5,10 GB) | Stegkraft beim Anstreichen, Cello-G-Saiten | CC BY 4.0 (Zenodo-API) | liegt außerhalb | **Pflicht**, sobald Reibwerte daraus ausgeliefert werden: Lampis, Zenodo 13374477 |
| Repo `klassen/Klasse 1 - Hammer-Saite/quellen/Gabrielli_et_al_2013_EURASIP_Clavinet.pdf` | Clavinet (Tonabnehmer, Verstärker), abgelegt IC Session 4 | CC BY 2.0 (S. 1) | ja (Regel `quellen/`) | `CREDITS.md` des Instrument-Ordners, sobald das Clavinet mit diesen Werten ausgeliefert wird: Gabrielli, Välimäki, Penttinen, Squartini, Bilbao, EURASIP JASP 2013:103, CC BY 2.0 |
| Repo `klassen/Klasse 7 - Lippe und Rohr/quellen/Soares_et_al_2025_ActaAcustica_9_68_Trompete.pdf` | Bohrung der Trompete (Abb. 4), abgelegt IC Session 4 | CC BY 4.0 (S. 1) | ja | **Pflicht**, sobald die Trompete ausgeliefert wird: Soares, Cochelin, Fréour, Vergez, Arimoto, Acta Acustica 9, 68 (2025), CC BY 4.0; Bohrung vereinfacht |
| Repo `klassen/Klasse 7 - Lippe und Rohr/quellen/Petiot_Freour_Arimoto_2025_FA_Trompete_Mundrohr.pdf` | Mundrohr der Trompete (nur Skizze), abgelegt IC Session 4 | CC BY 4.0 (HAL, Deckblatt); Fußnote S. 1 CC BY 3.0 | ja | nur Verweis — keine Werte daraus gebaut |
| Repo `klassen/Klasse 8 - Rohrblatt und Rohr/quellen/Petersen_et_al_2020_ActaAcustica_4_13_Saxophon.pdf` | Kegel des Altsaxophons (Tab. A1), abgelegt IC Session 4 | CC BY 4.0 (S. 1) | ja | **Pflicht**, sobald das Altsaxophon ausgeliefert wird: Petersen, Colinot, Kergomard, Guillemain, Acta Acustica 4, 13 (2020), CC BY 4.0 |
| Repo `klassen/Klasse 8 - Rohrblatt und Rohr/quellen/Ukshini_Dirckx_2025_Acoustics_7_15_Altsaxophon.pdf` | Länge des Altsaxophons, abgelegt IC Session 4 | CC BY (S. 1) | ja | mit Petersen zusammen: Ukshini, Dirckx, Acoustics 7, 15 (2025), CC BY |
| Repo `rechnung/externals/doctest/` (`doctest.h` 2.4.11, `LICENSE.txt`) | Testtreiber der Rechenkerne (AUFTRAG_BAU §3), geladen IC Session 4 (2026-10-29) von `raw.githubusercontent.com/doctest/doctest/v2.4.11/`, SHA-256 `44faa038…2e039fb` | MIT („The MIT License (MIT) Copyright (c) 2016-2023 Viktor Kirilov“, `LICENSE.txt`) | nur Tests | Lizenztext beilegen, wenn die Tests mitgehen; die Anwendung enthält es nicht |
| Repo `rechnung/externals/pocketfft/` (`pocketfft_hdronly.h`, Zweig `cpp`, `LICENSE.md`) | FFT hinter `gemeinsam/Fft.hpp` (wie in LumiViz), geladen IC Session 4 (2026-10-29) von `raw.githubusercontent.com/mreineck/pocketfft/cpp/`, SHA-256 `518a95c5…744c86d` | BSD-3-Clause („Copyright (C) 2010-2018 Max-Planck-Society … Redistribution and use in source and binary forms … are permitted …“, `LICENSE.md`) | — (LumiViz hat pocketfft schon) | wie in LumiViz: Lizenz in den Hinweisen der Anwendung |

**Nicht geladen** (Stand 2026-10-26): Brauchler et al. 2023 (Sci. Rep., CC BY 4.0) und Abdallah et al. 2026 (Meccanica,
CC BY 4.0) — die Verlage lieferten eine Bot-Seite statt des PDFs.

**In der Git-Historie dieses Repos** stehen die Audiodateien noch (Commits `60bdc39`, `d965732`; zusammen rund 2 GB).
Auf GitHub wird das Repo erst kleiner, wenn die Historie umgeschrieben wird (`git filter-repo`, Force-Push — macht
Patrik).

## 4. Prüfen vor der Integration

- Für jede Zeile in §3: Liegt die Datei unter einem ignorierten Pfad oder außerhalb? (`git check-ignore -v`)
- Steht in LumiViz an jeder Stelle, die Werte aus einer CC-BY-Quelle ausliefert, der Eintrag in `CREDITS.md`?
- `SHA256SUMS` und `LADELISTE.tsv` erlauben es, die Dateien unter `samples\` zu prüfen oder neu zu laden.

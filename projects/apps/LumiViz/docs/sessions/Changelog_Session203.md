# Changelog Session 203 (2026-11-17)

> Die letzten Rechercheberichte der Werkbank sind abgeholt und gegengelesen. Die Befunde daraus sind an LumiViz geprüft
> und entschieden (KQ30 a–KQ35 a). Die Teiltonmessung misst das Abklingen in zwei Stufen jetzt richtig, auch bei
> flachem Knick; die Ziele des Flügels sind damit neu gemessen. Neue Grenzen für Lautheit und Sprünge stammen aus
> reflexionsarmen Aufnahmen.

## Neu

- **Berichte der Research-Werkbank 21–28**, unverändert mit Kopf und Befund des Gegenlesens:
  `docs/modellierung/gezupfte-saite/recherche/2026-11-17_S203_*_werkbank.md` (Umlauf-Filter, Harfe, Gitarre, Cembalo,
  Oud / Laute, Einzeltöne) und `docs/modellierung/lippe-rohr/recherche/2026-11-17_S203_*_werkbank.md` (Alphorn,
  Waldhorn / Tuba), je mit den Rechnungen der Läufe.
- **Teiltonmessung (KQ32 a):** zwei Stufen aus zwei abklingenden Komponenten mit Boden, an die Kurve nach Schroeder
  angepasst; vorher bei flachem Knick 10–20 % zu langsam. Zwei Testfälle „flacher Knick“, Messfall
  `LUMIVIZ_KQ32_TASTE` (je Ebene einer Salamander-Taste).
- **Reflexionsarme Einzeltöne** der TU Berlin (Gitarre, Doppelpedalharfe) im Archiv, Begleitdateien unter
  `docs/modellierung/gezupfte-saite/aufnahmen/TU_Berlin_Weinzierl_DepositOnce19858/`, gemessen
  (`proben/2026-11-17_S203_kq33_tu_*`).
- Proben: Gegenproben zu den Berichten 21 und 22, Harfe an Bells Ort 17, Bilder der Zerlegung an Salamander, Gitarre
  gegen die TU-Gitarre.

## Geändert

- `werte/salamander_c5.txt`, `salamander_struktur.txt`, `salamander_ziel.txt` mit der neuen Messung (Abklingzeiten im
  Median −5 … −11 %, an Bildern geprüft).
- `docs/audio/Instrumente_IC_Konzept.md` 0.53.0: KQ30 a (Ablösekraft 1,07 N, Stoß 3,8 mN s), KQ31 a (Harfe über
  200 Hz untersuchen), KQ32 a, KQ33 a / KQ34 a (Grenzen: Harfe ab D2 ≤ 22 LU, mittlerer Sprung ≤ 3 LU), KQ35 a
  (Gitarre nach Elie).
- `docs/Offene_Punkte.md` 1.394.0: Punkt (22) ganz geprüft.
- Köpfe: Bericht 12 (S202) mit Nachtrag zu Bell 7.22; Modul-Doku `InstrumentMeasure.md`.
- `docs/archiv/Index_quellen.md` neu.

## Außerhalb des Repos

- `…\cmake\quellen\`: 36 Dokumente neu (gezupfte-saite, messung, lippe-rohr, bogen-saite), Herkunft in `UMZUG_S203.tsv`.
- Research-Werkbank: Aufträge 21–28 im Archiv; neue Reihen `platte-stab` und `membran` (Klassen 4 und 5).
- `E:\samples\Klasse 2 - Gezupfte Saite\TU_Berlin_Weinzierl_DepositOnce19858\` (3,7 GB).

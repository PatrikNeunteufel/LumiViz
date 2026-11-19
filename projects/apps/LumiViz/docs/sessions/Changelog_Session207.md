# Changelog Session 207 (2026-11-19)

> Die Sprünge der Lautheit je Taste über KQ34 a sind geklärt: das Modell hörte an ein bis zwei Punkten, die Schwelle
> stammt aus dem Mittel über 32 Mikrofone in 2 m. Die gezupften Instrumente werden jetzt gemessen wie die Aufnahme.
> An der Oud zeigt sich dabei ein Fehler des Bauplans. Kein Programmcode.

## Neu

- **KQ36 a:** KQ34 a gilt für das Leistungsmittel über viele Hörorte in rund 2 m. Ein Körper aus einem Bauplan wird
  dafür mit 16 Hörorten auf der vorderen Halbkugel in 2,06 m gerechnet (Prüfkörper, nicht der mitgelieferte); ein
  gemessener Körper (Gitarre) wird gegen die Verteilung der TU-Aufnahmen bei gleicher Zahl Mikrofone geprüft
  (größter Sprung ≤ 13,3 LU, mittlerer ≤ 3,0 LU).
- **KQ37 a:** die Oud bleibt bekannte Abweichung mit Ursache, bis der Bericht zu Auftrag
  `2026-11-19_35_oud_basschor_einzeltoene` der Research-Werkbank da ist.
- **KQ38 a:** der Bauplan der Oud bekommt zwei weitere Ziele (die Spitze um 203 Hz am Antwortpunkt 3 die stärkste
  unter 400 Hz; Hauptspitze der Abstrahlung beim 1,7- bis 2,0-Fachen der Luftmode) und wird nach dem Bericht neu
  gesucht.
- Bericht `docs/modellierung/gezupfte-saite/2026-11-19_S207_spruenge_je_taste.md` und Proben
  `docs/modellierung/gezupfte-saite/proben/2026-11-19_S207_*` (Zerlegungen, Gitarre gegen g34, TU je Mikrofon,
  Prüfkörper mit 16 Hörorten, Oud gegen Değirmenli 2017, Formen der Oud-Moden).

## Geändert

- `docs/audio/Instrumente_IC_Konzept.md` 0.70.0, `docs/Offene_Punkte.md` 1.412.0.

## Zahlen

- Ursache: der Grundton trägt den Ton, und |p/F| des Körpers am Grundton schwankt an einem Punkt stark — Gitarre
  68 / 71: −3,9 / −0,2 dB gegen +11,0 / +10,0 dB (an g34 gemessen +10,8 dB, Modell +11,3 dB); Harfe 78 / 80: −39
  gegen −16 dB.
- TU-Aufnahmen (ff) je Zahl der Mikrofone, größter / mittlerer Sprung: ein Mikrofon Harfe 13,9 / 3,1 LU, Gitarre
  11,1 / 2,8 LU; alle 32: 11,1 / 2,5 und 10,3 / 2,5 LU.
- Nach KQ36 a (Spanne / größter / mittlerer Sprung): Gitarre 12,7 / 12,7 / 1,7 LU, Laute 12,9 / 8,4 / 2,4 LU,
  Cembalo mit GZ13 a 5,8 / 3,0 / 0,6 LU — halten. Harfe ab D2 20,0 / 12,8 / 3,0 LU — der größte Sprung 0,8 LU über
  der Schwelle (43 / 47). Oud 18,8 / 14,7 / 1,8 LU — verfehlt am einzelnen Basschor (ohne ihn 11,4 / 10,3 / 1,6 LU).
- Oud: das Tal der Abstrahlung unter 200 Hz ist gemessen (Değirmenli 2017: 26 dB unter der Hauptspitze, Modell
  33 dB); die gleichsinnige Deckenmode 209 Hz bewegt den Steg nur mit 28 % ihres größten Werts, gemessen ist die
  Spitze um 203 Hz die stärkste (rund 20 dB Abstand an dieser Größe).
- **Offen:** Harfe 43 / 47 und rund 2 LU über der echten Harfe bei jeder Zahl von Orten; Bauplan der Oud; Höhe der
  Bäuche an der Harfe.

## Tests

Keine (kein Programmcode); **1923 Cases** unverändert.

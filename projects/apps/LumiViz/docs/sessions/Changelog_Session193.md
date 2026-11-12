# Changelog Session 193 (2026-11-12)

> Am Programm ändert sich nichts. Die Stufe 34k (gezupfte Saiten: Cembalo, Harfe, Laute, Oud, Gitarre) ist bis zum
> Abschluss geplant.

## Gemessen

- Warum die Vorlage aus InstrumentsCalculation über den Umfang so ungleich laut ist (18,6–36,1 LU): der Zupf ist dort
  ein Kraftstoß fester Dauer, und der Steg trägt wenige scharfe Moden. Mit einem Stoß von einer Viertelperiode und einem
  glatten Prüfsteg fallen die Sprünge zwischen Nachbarn von bis zu 21,8 LU auf 1,9–4,2 LU.
- Mitschwingende Saiten lassen sich je Note rechnen (Saiten und Steg sind linear, Unterschied zum Clip −184 dB): an der
  Gitarre 1,1–1,3 % der Echtzeit, an der Harfe mit allen 47 Saiten 9,4–9,9 %, mit einer Auswahl 3,7–4,9 %.
- Bericht `docs/modellierung/gezupfte-saite/2026-11-12_S193_gegenprobe_ursachen.md`, Programme und Rohausgaben unter
  `proben/`.

## Entschieden

- GZ1 a: der Kern `pluckedString` liefert `harpsichord`, `concert_harp`, `lute`, `oud`; `harp`, `guitar`,
  `nylon_guitar` bleiben.
- GZ1-1 a: die Spielweisen kommen in den Kern — Laute, Oud und eine neue `classical_guitar` tragen einen Saitensatz
  (Akkorde, Tabs, Strum, Up und Down, Palm mute, Harmonic, Bend, Slide, Legato).
- GZ2 a: erst der Umzug Bit für Bit als Beleg, dann Rechenfassung 2 mit Quellen (Zupf als Auslenkung und Loslassen,
  Saiten aus Mensuren); ausgeliefert wird ein Instrument erst, wenn es die Zielgrößen hält.
- GZ3 a: Mitschwingen je Note schon in 34k.
- GZ4 a: Steg und Abstrahlung aus einem Körper (Moden mit Wert am Stegpunkt, Luftraum) statt aus angenommenen
  Stegmoden; berichtigt IK2-10 a für diese Klasse.

## Geplant

- Teilplan 34k-1 bis 34k-11 in `docs/audio/Instrumente_IC_Konzept.md` §6 (Version 0.25.0), mit Haltepunkten nach der
  Recherche (34k-4), am Körper (34k-5) und in der Herleitung (34k-6).

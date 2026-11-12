# Changelog Session 195 (2026-11-12 / 13)

> Am Programm ändert sich nichts. Die Sitzung legt für die gezupften Saiten (Stufe 34k) fest, auf welchen Quellen jedes
> der fünf Instrumente steht und woher sein Körper kommt, und beginnt mit dem Körper. Die Stufe steht bei 34k-5 von elf.

## Neu (Doku, Proben, Quellen)

- **Ziele an Aufnahmen gemessen** (`docs/modellierung/gezupfte-saite/2026-11-12_S195_ziele_aufnahmen.md`, Prüfstand unter
  `proben/`): echte Cembali halten 15 LU über den Umfang (6,1–13,6 LU), die Konzertharfe über den vollen Umfang nicht
  (17,8 LU; ab D2 10,8 LU); Sprünge zwischen Nachbarn an Aufnahmen bis 11,3 LU.
- **Recherche je Instrument** (vier Berichte unter `recherche/`), eine Ladeliste, **35 Quellen geladen** nach
  `…\cmake\quellen\gezupfte-saite\` (außerhalb des Repos, mit `SHA256SUMS`), der Index der Ablagen neu.
- **Gegenlesen am Original** (`recherche/2026-11-12_S195_gegenlesen.md`): die tragenden Tabellen stimmen. Neu gefunden:
  Bell 1988 beschreibt die Salvi „Orchestra“ vollständig (46 Saiten samt Aufbau der Metallsaiten, Boden, Leisten,
  Kasten), Le Carrou 2006 die Camac Atlantide (47 Saiten mit Länge, Durchmesser, Winkel, Lage).
- **Probe zum Körper** (`2026-11-12_S195_probe_statistik_ohne_bauplan.md`): der statistische Teil von `InstrumentBody`
  liefert ohne Bauplan keine Moden; Plattenformeln treffen nur die Größenordnung.
- **Erster Fall Oud** (`2026-11-12_S195_oud_erster_fall.md`): ein Bauplan aus der Abbildung der Quelle rechnet in unter
  einer Sekunde; die erste gemessene Mode trifft er, alle vier innerhalb von 10 % mit keiner Deckendicke.

## Entschieden

- QZ1 a: Quellen der Stufen 1 und 2 laden.
- KQ1 a: Vorbild der Harfe ist die Salvi „Orchestra“ nach Bell 1988 — **46 Saiten** statt 47, Körper aus dem Bauplan.
- KQ2 a: Vorbild des Cembalos ist der Taskin 1769.
- KQ3 a: Stegmoden der Gitarre aus einem Modalfit am Archiv Mores 2021.
- KQ4 a, KQ5 a: Laute und Oud werden gebaut; was übertragen oder angenommen ist, steht je Wert dabei.
- KQ6 a: Cembalo, Laute und Oud bekommen einen Ersatz-Bauplan, `InstrumentBody` rechnet ihre Moden.
- KQ7 a: `InstrumentBody` bekommt Löcher in der Platte; danach die Oud neu.

## Offen

- Löcher in `InstrumentBody`, die Oud mit Löchern, Bauplan der Harfe, Modalfit der Gitarre, Laute, Cembalo (34k-5).
- Nicht gegengelesen: Di Veroli, Penttinen, Scherrer, Ducceschi, die Pegel des Mitschwingens. Roy 2015 (Thèse) fehlt im
  Quellenordner. Für Laute und Oud gibt es keine frei lizenzierten Aufnahmen.

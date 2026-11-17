# Changelog Session 202 (2026-11-17)

> Die Rechercheberichte der Werkbank zu den gezupften Instrumenten (34k) und zu den ersten Blechbläsern (34m) liegen
> jetzt im Repo und sind an ihren Quellen gegengelesen; was nicht trägt, steht im Kopf jedes Berichts. Am Programm
> hat sich nichts geändert. Für die nächsten Instrumentenklassen sind Recherchen gestellt.

## Neu

- **Berichte der Research-Werkbank, unverändert mit Kopf** (Herkunft, Lauf, Gegenlesen, Ort der PDFs):
  - gezupfte Saite (34k) unter `docs/modellierung/gezupfte-saite/recherche/2026-11-16_S202_*_werkbank.md`: Harfe
    besaitet, Gitarre Fernfeld und zweite Stegrichtung, Cembalo Kiel und Register, Oud und Laute, Einzeltöne — je
    Recherche und Berechnungsgrundlage (Aufträge 02–06, 11–16), mit den Rechnungen der Läufe in gleichnamigen Ordnern;
  - Lippe-Rohr (34m) unter `docs/modellierung/lippe-rohr/recherche/2026-11-16_S202_*_werkbank.md`: Alphorn
    (Nachprüfung, Grundlagen), Waldhorn und Tuba (Bohrung, Grundlagen).
- **Gegenprobe** `docs/modellierung/gezupfte-saite/proben/2026-11-17_S202_gegenprobe_bericht11.py` mit Rohausgabe
  (Umlauf-Filter: foh-Diskretisierung, Ein-Pol, Lagrange, Passivität des Bank-Entwurfs).

## Geändert

- Köpfe der Berichte aus S201 (Umlauf-Filter, Alphorn-Geometrie): Ort der PDFs und des Archivs, Verweis auf das
  Gegenlesen.
- `docs/archiv/Index_quellen.md` und `Index_samples.md` neu geschrieben (20 PDFs neu in der Ablage der Quellen, neuer
  Ordner `messung`).
- `docs/Offene_Punkte.md` 1.386.0: in §6 Zeile 34k Punkt (22) mit fünf Befunden aus dem Gegenlesen.

## Befunde aus dem Gegenlesen (für die nächsten Schritte)

- **Cembalo:** die einzige Messung der Ablösekraft des Kiels (Chadefaux 2013, G4) gibt 0,7–0,9 N; der heutige
  Maßstab (2 N an C4) liegt an G4 10–12 dB darüber.
- **Harfe:** Bells Abb. 7.22 trägt keinen Bezug an der Achse und steigt mit 6 dB je Oktave; an der besaiteten Camac
  klingen die tiefen Moden schwächer gedämpft ab (Q 22 / 36) als im heutigen Körper (Q 13); der Querwiderstand der
  Saiten trägt den Faktor 0,75, nicht 0,25.
- **Umlauf-Filter:** die Zitate stimmen; der Befund „Bank-Entwurf nicht passiv“ gilt für eine Umsetzung ohne Glättung
  und Hochfrequenz-Vorgabe der Quelle.
- **Messung:** das Maximum der Momentary-Lautheit misst das Abklingen mit; Sprünge in Aufnahmen mit Raum messen vor
  allem den Raum.
- **Alphorn:** die Lesart „Radius“ der Bohrung ist an der Quelle belegt; ein Druckfehler in Hendries Gl. (2.102) ist
  bestätigt.

## Außerhalb des Repos (Research-Werkbank)

Archiv für abgeholte Aufträge, PDFs nur noch in `…\cmake\quellen\`, Regeln aus dem Gegenlesen in der `CLAUDE.md` der
Werkbank, rohes Lesen mit Ablage, Nacharbeit 21–28, eine Reihe je Instrumentenklasse mit ersten Aufträgen (Lippe-Rohr,
Bogen-Saite, Rohrblatt, Luftstrahl, Becken und Gong, Tonrad), Suchhelfer für Formelsatz und fehlende Ligaturen.

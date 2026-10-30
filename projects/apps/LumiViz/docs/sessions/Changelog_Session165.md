# Changelog Session 165 (2026-10-30)

> **Der zweite Anlauf des automatischen Setzens ist gebaut und geprüft — und verfehlt die Probe an der Glätte.** Die
> Suche rechnet jetzt je Taste mit eigener Schrittweite, lässt Parameter ohne Wirkung stehen, gibt die Drehung der
> Schwingungsebene an allen Tasten frei und setzt die Verstimmung des Chors vorher an die Schwebung der Aufnahme. Ohne
> diese letzte Stufe trifft sie an sechs Probetasten so viel wie der beste Stand aus Session 163; mit ihr weniger. Drei
> weitere Wege für diese Stufe ändern nichts daran, dass jede Größe, die die Schwebung erzeugt, im Ziel wellig ist.
> Patrik hat entschieden: kein Lauf über Nacht, die Vorgabe wird nach Gehör gesetzt (ZZ1 a). Am Klang des Programms hat
> sich nichts geändert. Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 („7.5.12 gebaut (S165)“, „A4-Wege
> (S165)“).

## Intern

- **Suche je Taste:** `levenbergMarquardtJeBlock` (eigenes λ, eigene Annahme, eigenes Ende je Taste, gemeinsame
  Auswertung auf dem Pool; kein Schritt unter 0,1 Toleranzen Wirkung) — je Block bitgleich wie die Suche je Block allein.
- **Suchraum:** δ an allen Tasten frei ab 0,01 Cent (keine Entartung mehr), Startpunkt 3, bester Lauf je Taste;
  Optionen für die Wege der Verstimmungsstufe.
- **Verstimmungsstufe (A4):** Schwebungsfrequenz des Modells mit derselben Messkette wie das Ziel, Raster und Goldener
  Schnitt; dazu die gemeinsame Stufe für Verstimmung und Drehung.
- **Hörschwellen angewandt:** hörbar getroffen, eindeutig, Vorzeichentest mit Holm, Ausreißer (HW1 a–HW4 a),
  gegengerechnet an der Suche aus Session 162.
- **Werkzeuge:** `LUMIVIZ_75_SUCHE2` (mit den Wegen `…_A4`, `…_DELTA_START`), `LUMIVIZ_75_A4`, `LUMIVIZ_75_SCHNITT_X`,
  `LUMIVIZ_75_PROBE12`, `LUMIVIZ_75_G_AM`. Berichte unter `docs/modellierung/hammer-saite/proben/S165_*`.
- **Probelauf an sechs Tasten:** mit A4 Σ r² 2565 (34 / 104 getroffen), ohne A4 2257 (50 / 104); die Wege „nur Start“,
  „gemeinsam“, „δ startet unten“ 2471 / 2320 / 4325; Glätte an keinem Weg.
- Tests: 1676 Cases; `*7.5.9*,*7.5.10*,*7.5.11*,*7.5.12*` 27 / 27 unter Release-clang; Testing-MSVC gebaut.

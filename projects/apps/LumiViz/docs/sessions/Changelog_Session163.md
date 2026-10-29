# Changelog Session 163 (2026-10-29)

> **Die Probe des automatischen Setzens ist ausgewertet: verfehlt, aber an Stellen, die weder das Modell noch das Ziel
> beurteilen.** Die Suche aus Session 162 hat Parameter mitgeschleppt, die nichts bewirken, hat die 30 Tasten über eine
> gemeinsame Dämpfung aneinander gekettet, und der Mechanismus, der den langen Nachklang eines Flügels erzeugt (die
> Drehung der Schwingungsebene nach Weinreich), war an allen Chortasten abgeschaltet. Mit ihm eingeschaltet trifft das
> Modell an vier von sechs geprüften Tasten deutlich mehr. Patrik hat einen zweiten, vorläufig letzten Anlauf nach einem
> Plan in vier Phasen entschieden. Am Klang des Programms hat sich nichts geändert. Steuerdokument:
> `docs/audio/Spielweisen_Konzept.md` §4.1 (Probe 7.5.11, V2 a, Teilplan 7.5.12–7.5.14).

## Intern

- **Vorfragen zur Suche:** Stützpunkte ohne Wirkung wandern unter der Marquardt-Dämpfung über ihren ganzen Bereich
  (bestätigt); γ und h sind ohne δ im Chor exakt wirkungslos, und bei h = 1 rauscht die entartete Eigenzerlegung;
  Startpunkt 2 lohnt keine Wiederaufnahme — der Hebel ist die Entkopplung der Tasten (bester Stand je Taste 9086 statt
  11 290, 204 statt 149 Treffer).
- **Werkzeuge:** Schnitt je Parameter mit 15 Punkten (ZF8 a (2)), Levenberg–Marquardt je Taste allein, Schalter δ auch im
  Chor (`LUMIVIZ_75_SCHNITT`, `LUMIVIZ_75_TASTE`, `LUMIVIZ_75_DELTA_CHOR`). Berichte unter
  `docs/modellierung/hammer-saite/proben/S163_*`.
- **Probe 7.5.11:** Glätte an 6 von 6 Tasten verfehlt (`detune` wellig), Eindeutigkeit 71 / 456, `highDecay` an 19 von
  30 Tasten am Bensa-Band.
- **Plan A–D (V2 a):** A Suche in Ordnung (δ frei, je Taste eigenes λ, Wanderer still, `detune` gestaffelt, drei
  Startpunkte), B Urteil hörbar (Hörschwellen aus der Literatur, Eindeutigkeit als Gleichheit), C Modell nur bei Befund,
  D hören und setzen. Nummern 7.5.12–7.5.14.
- **InstrumentsCalculation:** Lehren L8–L15, Kopie des Flügel-Vorbilds, Bauauftrag für eine Cloud-Sitzung
  (`AUFTRAG_BAU.md`).
- Tests: 1662 Cases; `*7.5.9*,*7.5.10*,*7.5.11*` 13 / 13 unter Release-clang; Testing-MSVC gebaut.

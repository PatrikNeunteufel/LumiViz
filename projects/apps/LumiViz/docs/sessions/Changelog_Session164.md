# Changelog Session 164 (2026-10-30)

> **Das Urteil über das automatische Setzen hat jetzt Hörschwellen aus der Literatur — und der zweite Anlauf endet nach
> dem vorher gesetzten Ausstieg.** Recherchiert wurde, ab welchem Unterschied man Abklingzeit, Schwebungstiefe und Pegel
> hört; ein Hörversuch an Klaviertönen existiert nicht, der nächste Beleg ist eine gezupfte Saite. Daraus entstanden vier
> Entscheide für das Urteil und der Bauauftrag für Session 165. Nach deren Probelauf (jede Größe, die die Schwebung
> trägt, ist im Ziel wellig) gilt der Ausstieg: kein automatisches Setzen; die Ergebnisse der Suche gehen als Hörproben
> an Patrik, der nach Gehör setzt. Am Klang des Programms hat sich nichts geändert. Steuerdokument:
> `docs/audio/Spielweisen_Konzept.md` §4.1 („7.5.13 B1“, HW1 a–HW4 a, PV1 a).

## Intern

- **Recherche B1:** Bericht `docs/modellierung/hammer-saite/recherche/2026-10-30_S164_hoerschwellen.md` (Agent,
  tragende Stellen selbst nachgelesen); sieben Quellen-PDFs lokal unter `quellen/`.
- **HW1 a:** Hörschwellen je Größe in `tests/unit/UnitTests/werte/hoerschwellen.txt` — Zeiten nach Tolonen &
  Järveläinen 2000 (Faktor 0,7923 / 1,4309), Knicktiefe 0,5 dB, Schwebungstiefe nach Ewert 2002.
- **HW2 a:** hörbar getroffen und eindeutig bei max(Toleranz, Hörschwelle). **HW3 a:** Bestehen der Probe 7.5.12
  (Glätte an 5 von 6, Eindeutigkeit an 5/6 der Größen, besser als die Vorgabe). **HW4 a:** systematisch nach
  Vorzeichentest mit Holm; Auslöser für C1.
- **Bauauftrag 7.5.12** (`.claude/handover/auftrag_7.5.12.md`) für Session 165; dazu die Varianten der
  Verstimmungsstufe nachgerechnet (Patrik „a: Varianten rechnen“).
- **PV1 a:** Probe 7.5.12 an der Glätte verfehlt, ZZ1 a, kein Lauf über Nacht; Hörproben an 33, 60, 84 als Vorschlag.
- **InstrumentsCalculation:** Lehren L16–L18 (L16 berichtigt L12).
- Tests: keine Änderung an Code oder Tests in dieser Session (Stand nach Session 165: 1676 Cases).

# Changelog Session 153 (2026-10-25)

> **Jede Saite des Flügels schwingt jetzt in zwei Ebenen.** Ein echter Flügelton klingt zuerst schnell, dann langsam
> ab, weil die Saite nicht nur senkrecht schwingt: ein Teil der Bewegung wandert in eine Richtung, die der Steg kaum
> abnimmt. Das Modell kann das jetzt rechnen — die tiefen Einbrüche, in denen der gerechnete Ton fast verstummte,
> füllen sich damit wie bei einem echten Flügel. Eingestellt wird die zweite Ebene erst in der nächsten Sitzung an der
> Aufnahme; bis dahin klingt jedes Instrument wie vorher.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (P1 a bis P8 a, ZS1 a, P3-1 a, RZ1-1 a, WK1 a).

## Neu

- **Die Hammer-Saite rechnet zwei Schwingungsebenen** je Saite, gegeneinander gedreht und leicht verstimmt, mit
  verschiedener Kopplung an den Steg (nach Tan 2017 und Cuenca & Caussé 2007). In einer Probe an C4 sinkt der tiefste
  Einbruch von 31 auf 10 dB.
- Drei neue Kurven der Hammer-Saite in der `.lvinst` (Fassung 18): *Polarization angle*, *Polarization detune*,
  *Horizontal ratio* — vorerst überall 0, ältere Dateien bekommen die Vorgabe.

## Verbessert

- Die Phantomteiltöne kommen aus der Spannung jeder einzelnen Saite in beiden Ebenen, nicht mehr aus dem Mittel des
  Chors — im Nachklang bleiben sie hörbar.

## Intern

- Eigener Löser für Eigenwerte kleiner komplexer Matrizen (`loop/Eigenzerlegung`), geprüft gegen Eigen.
- Bezug der Hammer-Töne vor dem Umbau (`werte/hammer_bezug_*`): Tasten mit einer Saite rechnen Bit für Bit wie vorher.
- Klangfassung 2 (eingefrorene Clips mit Hammer-Saite werden neu gerechnet).
- Recherche zur zweiten Polarisation unter `docs/modellierung/hammer-saite/recherche/`.
- **Voller Lauf 1611 / 1611 unter Release-clang und Testing-MSVC.**

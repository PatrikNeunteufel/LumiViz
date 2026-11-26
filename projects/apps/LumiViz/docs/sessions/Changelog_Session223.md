# Changelog Session 223 (2026-11-26)

> Diese Sitzung hat die Tafel des Umlauf-Filters für alle fünf Instrumente fertig gerechnet (34k-7d-3 erledigt) und vor
> 34k-7d-4 eine Lücke zwischen Bezug und Kern geschlossen: die Richtung am Steg der Körper ohne Messung (SR1 a). Am Kern
> selbst ist noch nichts gebaut.

## Entscheide (Konzept 0.91.0)

- **SR1 a:** Für einen Körper ohne Richtung am Steg (alle `bridgeAngle` 0 — Harfe, Cembalo, Laute, Oud) setzt der Bau
  des `Steg` aus der Körperdatei (`InstrumentBody`) die Annahme nach Scherrer ein, wie `richtung_von` des Prüfstands; die
  Körperdatei bleibt, wie sie gerechnet ist. Nachgerechnet: der Bezug mit Winkel 0 statt der Annahme weicht an Cembalo 84
  im späten Pegel bis 16,6 dB ab, an Harfe 91 um 1,35 dB (1–2 s), an Laute und Oud bis 6,9 Cent; die Gitarre trägt eine
  gemessene Richtung und bleibt gleich.

## Neu

- `tests/unit/UnitTests/werte/zupf_umlauf.json` vollständig: Harfe 80 Tasten, Cembalo T29–T89, Gitarre 6 Saiten,
  Laute 8 Chöre, Oud 6 Chöre. Jede Stelle hält UL2 a außer Laute Chor 8.1 (bekannt, 7,5–12,2 Cent). Cembalo T87–T89
  nachgerechnet, nachdem `QSaveFile::commit` einmal fehlschlug (Ursache ungeklärt, die Datei blieb heil).
- Proben `docs/modellierung/gezupfte-saite/proben/2026-11-26_S223_bezug_ohne_richtung.py` (Bezug mit Winkel 0),
  `…_vergleiche_bezug.py` (zwei Bezugsdateien je Fall), Ergebnis `…_bezug_ohne_richtung.txt`.

## Geändert

- Konzept 0.91.0 (SR1 a), Offene Punkte 1.434.0 (Zeile 34k: 34k-7d-3 erledigt, nächster Schritt 34k-7d-4).

## Verifikation

- Testing-MSVC neu gebaut; `*34k-7c*,*34k-7d*,*34k-3*,*34c Pool*` grün bis auf den Leser der Tafel, solange Tasten
  fehlten.
- Leser `*34k-7d-3 Tafel aus*` mit der vollständigen Tafel: Release-clang 1807 / 1807 (30 s), Testing-MSVC 1807 / 1807
  (27 s).
- Kein voller Lauf; **1945 Cases**.

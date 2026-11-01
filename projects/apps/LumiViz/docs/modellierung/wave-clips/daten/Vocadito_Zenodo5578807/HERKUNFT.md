# Herkunft — vocadito (einstimmiger Gesang mit beschrifteter Tonhöhe)

- **Quelle:** Zenodo, Datensatz 5578807 (veröffentlicht 2021-10-11), DOI 10.5281/zenodo.5578807, „vocadito: A dataset
  of solo vocals with f0, note, and lyric Annotations“, Autoren laut Zenodo-API Rachel Bittner, Katherine Pasalo, Juan
  José Bosch, Gabriel Meseguer Brocal, David Rubinstein. `vocadito.zip` (58 492 257 B, MD5 der Quelle
  `dea40fd18f14d899643c4ba221b33a46` geprüft) geladen am **2026-11-01** (LumiViz S169, Entscheid GA1 a), ausgepackt
  ohne die Mac-Beifiles `__MACOSX/` (AppleDouble `._*`), Archiv danach gelöscht.
- **Lizenz:** Zenodo-API `cc-by-4.0` (nachgelesen 2026-11-01) — Namensnennung nötig.
- **Beschreibung laut Zenodo:** 40 kurze Ausschnitte einstimmigen Gesangs, 7 Sprachen, Sänger verschieden geschult,
  auf verschiedenen Geräten aufgenommen; Tonhöhe je Frame, zwei Fassungen der Noten (zwei Beschrifter), Liedtext,
  Sprache; die Beschriftung von geschulten Musikern.
- **Inhalt:** 202 Dateien:
  - `Audio/` (40): `vocadito_<n>.wav`, mono 16 bit 44,1 kHz (alle 40 geprüft), zusammen 817,1 s, je 8,7 … 38,7 s.
  - `Annotations/F0/` (40): `vocadito_<n>_f0.csv`, Spalten Zeit in s und f0 in Hz (0 = keine Tonhöhe), Schritt
    5,805 ms (256 / 44 100).
  - `Annotations/Notes/` (80): `vocadito_<n>_notesA1.csv`, `…_notesA2.csv` — Anfang s, Tonhöhe Hz, Dauer s.
  - `Annotations/Lyrics/` (40), `Annotations/README.txt`, `vocadito_metadata.csv` (`track_id, singer_id,
    average_pitch, language`).
- **Gelesen 2026-11-01 (an der f0-Beschriftung):** gesungen 45–81 % der Frames je Ausschnitt; Umfang MIDI 37 bis 70;
  in allen 40 Ausschnitten Gleiten (21 bis 300 Stellen mit mehr als 150 Cent in 50 ms ohne Pause); Schwanken um den
  linearen Verlauf von mehr als 20 Cent (Effektivwert über 200 ms) in 5 bis 79 % der Fenster je Ausschnitt.
- **Prüfsummen:** `SHA256SUMS` (202 Dateien).
- **Zweck:** Messfall der Tonhöhe mit pYIN an echtem Gesang (LumiViz Stufe 15g, `Loop_Konzept.md` §4p, GA1 a):
  Anteil der Frames innerhalb 50 Cent, Erkennung gesungen / nicht gesungen, Oktavfehler, je Ausschnitt.

## Ablage (GA1 a, Patrik 2026-11-01; wie A-1 a bei InstrumentsCalculation)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Gesang\Vocadito_Zenodo5578807\`; wenn sie
nicht mehr gebraucht werden, archiviert Patrik sie auf die externe Festplatte. Im Repo von LumiViz bleiben diese Datei,
`SHA256SUMS` und `LADELISTE.tsv` unter `projects/apps/LumiViz/docs/modellierung/wave-clips/daten/Vocadito_Zenodo5578807/`
— damit lässt sich jede Datei wiederbeschaffen und prüfen.

# Changelog Session 166 (2026-10-30)

> **Hörproben für das Setzen nach Gehör liegen bereit.** Nach dem Ausstieg aus dem automatischen Setzen (ZZ1 a, PV1 a)
> setzt Patrik den ungleichen Stoß der Saiten, den Winkel und die Verstimmung der zweiten Schwingungsebene nach Gehör an
> drei Tasten. Dafür gibt es je Taste A1, C4 und C6 und je Stärke p, mf und ff eine Datei mit der Aufnahme
> *Salamander*, dem heutigen Flügel und dem Vorschlag der Suche, alle gleich laut, dazu eine Liste mit dem, worauf je
> Taste zu achten ist (Länge, Nachklang, Schwebung). Weil drei Klänge hintereinander schwer zu vergleichen sind, gibt
> es dazu einen kleinen Hörtest im Browser: je zwei Klänge blind als A und B, mit gezielten Fragen je Klang. Am Klang
> des Programms hat sich nichts geändert. Steuerdokument:
> `docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 31d-8, Absatz „8.5 Hörproben (S166, nach PV1 a)“.

## Intern

- **Werkzeug `LUMIVIZ_PV1_WAV`** (mit `LUMIVIZ_SALAMANDER`) in `test_LoopSalamanderHoerproben.cpp`: je Taste 33, 60, 84
  und Ebene der Suche (v6, v10, v16) *Salamander* | Vorgabe | Suche 7.5.12 „ohne A4“, gleich laut auf −23 LUFS; der
  Punkt aus `proben/S165_suche2_probe_ohne_a4/ergebnis_1.txt`, der Raum aus der Zeile `optionen`. Der Fall prüft, dass
  Kurven und Σ r² je Taste die Ergebnisdatei wiedergeben, und schreibt je Taste die Größen des Ziels gegen beide
  Fassungen mit „hörbar getroffen“ nach HW2 a.
- **Befund:** γ an A1 liegt im Vorschlag der Suche mit 41° außerhalb SQ2 a (0–25°); Patrik setzt innerhalb SQ2 a.
- **Hörtest blind, paarweise** (Wunsch Patrik nach den ersten zwei Dateien: drei Fassungen hintereinander sind schwer
  zu vergleichen): neu `tools/hoertest/` — `hoertest.py` (lokaler Server auf 127.0.0.1, nur Standardbibliothek,
  Antworten je Paar in `antworten.jsonl`, `--auswertung` ordnet sie den Fassungen zu) und `index.html` (A und B
  zufällig, laufen abwechselnd bis „Weiter“, nur A / nur B, ganzer Ton oder nur der Anfang, je Klang sechs Fragen
  auf 1–5 und welcher mehr nach Flügel klingt). Das Werkzeug `LUMIVIZ_PV1_WAV` schreibt dazu jede Fassung einzeln und
  `hoertest.json`: 27 Paare und drei Kontrollpaare (zweimal derselbe Klang).
- Tests: 1677 Cases (+1 Werkzeug); der neue Fall unter Release-clang grün (271 Prüfungen, rund 25 s), die Datei 4 / 4;
  Testing-MSVC gebaut, kein Lauf (UF17-1 a); kein Rechenkern berührt.

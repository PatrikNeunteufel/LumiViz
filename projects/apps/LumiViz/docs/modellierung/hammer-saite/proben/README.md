# Proben der Hammer-Saite — Berichte

Berichte der Proben und Gegenproben, unverändert so abgelegt, wie die Werkzeuge in `test_LoopSalamanderProbe.cpp` sie
schreiben (PB1 a, Patrik S156). Je Lauf ein Ordner mit der Sitzung im Namen. Die Urteile und ihre Gründe stehen im
`docs/audio/Spielweisen_Konzept.md` §4.1 (Teilplan 7.5); hier liegen die Messwerte dahinter. Sensorverläufe der Taktprobe
und Logs der Läufe bleiben lokal.

| Ordner | Sitzung | Werkzeug | Was | Ergebnis |
|---|---|---|---|---|
| `S155_probe75_abgebrochen/` | S155 | `LUMIVIZ_75_PROBE`, `…_TEIL=1` | Probe 7.5.4, erster Lauf; `probe75_zeit.txt` nur Teil (1) mit Kurven je Ebene | brach nach Teil (2) an Taste 33 ab |
| `S156_zeit75/` | S156 | `LUMIVIZ_75_ZEIT` | Zeit je Phase eines Tons und Durchsatz je Zahl der Threads: (1) ohne Taktprobe, (2) mit, (3) nach der flachen Längskraft | Wärmegrenze; 11 Threads am schnellsten |
| `S156_probe75/` | S156 | `LUMIVIZ_75_PROBE` mit `LUMIVIZ_SALAMANDER`; `LUMIVIZ_75_SPRUNG` | Probe 7.5.4 auf dem Pool; `spruenge75.txt` die Gegenprobe je Kurve an den größten Sprüngen | (2), (3), (4), (7) verfehlt; Sprung an 48 aus Regel (b) → GP1 a, RV1 a |
| `S156_probe75_rv1/` | S156 | `LUMIVIZ_75_PROBE` mit `LUMIVIZ_SALAMANDER` | Probe 7.5.4 nach RV1 a, dieselben Schwellen | verfehlt wieder → SM4 b (GP1 a) |
| `S162_nullprobe/` | S162 | `LUMIVIZ_75_NULLPROBE` | Nullprobe 7.5.9: Vorgabe, Startpunkt 1 und 2 der Suche gegen `salamander_ziel.txt` (Σ r², ZF5 a, UG1 a), je Taste und Größe | Σ r² 63 947 / 42 854 / 61 363, getroffen 86 / 128 / 116 von 456; am weitesten weg die späte Stufe |
| `S162_nm/` | S162 | `LUMIVIZ_75_NM` | Nelder–Mead (≤ 150 Auswertungen) gegen Levenberg–Marquardt an 33, 60, 84 je allein, ab Startpunkt 1 (Zeiten unter paralleler Last) | LM tiefer an 60 (149 gegen 700) und 84 (18 gegen 69), an 33 gleichauf (164 gegen 154) mit 58 statt 151 Auswertungen |
| `S162_suche/` | S162 | `LUMIVIZ_75_SUCHE` | Suche 7.5.10 an allen 30 Tasten von beiden Startpunkten; je Iteration eine Datei, `ergebnis_<n>.txt`, `vergleich.txt`, `kurzfassung.txt` (93 min unter paralleler Last) | Rest 42 854 → 11 290 (Start 1, 15 Iterationen) und 61 363 → 16 541 (Start 2, 20); beide getroffen an 71 von 456 — nach SP3 a nicht eindeutig |
| `S165_suche2_probe/` | S165 | `LUMIVIZ_75_SUCHE2` mit `…_TASTEN=33,48,60,66,81,84`, `…_START=1` | Probelauf 7.5.12 (A1–A5) an sechs Tasten: `a4_1.txt`, je Iteration eine Datei, `ergebnis_1.txt`, `ergebnis_bester.txt`, `kurzfassung.txt` (8,5 min) | Σ r² 2565, getroffen 34 / 104 — schlechter als S163 „δ frei“ an 60, 81, 84 |
| `S165_suche2_probe_ohne_a4/` | S165 | wie oben mit `LUMIVIZ_75_SUCHE2_OHNE_A4=1` | Gegenprobe: `detune` frei in der Suche statt fest nach A4 (7,7 min) | Σ r² 2257, getroffen 50 / 104 — wie S163 „δ frei“; die Verschlechterung kommt aus A4 |
| `S165_a4_v1/`, `S165_a4_v2/`, `S165_a4_v3/` | S165 | `LUMIVIZ_75_SUCHE2_A4=start`, `=gemeinsam`, `…_DELTA_START=unten`; je mit `LUMIVIZ_75_SCHNITT_X` (`schnitt.txt`), `LUMIVIZ_75_PROBE12` (`probe.txt`), `LUMIVIZ_75_G_AM` | A4-Wege: (1) A4 nur als Start, (2) `detune` und δ gemeinsam, (3) δ startet unten; die beiden Ordner darüber haben Schnitt und Probe ebenso | Σ r² 2471 / 2320 / 4325, getroffen 42 / 33 / 34; Glätte an keinem Weg (0 / 6) — Tabelle im Konzept „A4-Wege (S165)“ |

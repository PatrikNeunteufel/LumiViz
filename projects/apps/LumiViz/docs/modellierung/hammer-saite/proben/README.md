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

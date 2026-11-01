# pYIN an vocadito (S170, nach PY2 a — Talregel in Stufe 1)

Seit PY2 a nimmt Stufe 1 je Schwelle das erste Tal von d′ im Bereich des HMM (wie librosa `pyin`); der Stand davor steht in `S169_pyin_vocadito.md`.

Geschrieben von `test_LoopPYin.cpp` (Fall „Messfall an vocadito“, `LUMIVIZ_VOCADITO`). Quelle: vocadito, Zenodo 5578807, CC BY 4.0 (`docs/modellierung/wave-clips/daten/Vocadito_Zenodo5578807/HERKUNFT.md`). Rahmen 2048, Schritt 256 bei 44,1 kHz — dasselbe Raster wie die Beschriftung; Rahmen k ist um k · 256 zentriert. Gemessen gegen `Annotations/F0` (0 Hz = nicht gesungen).

- **RPA 50 / 100:** Anteil der gesungenen Rahmen, die gesungen erkannt sind und auf 50 / 100 Cent stimmen (100 Cent: das Mass des Papers, „within one semitone“).
- **Erkannt:** gesungen erkannt unter den gesungenen; **Fehlalarm:** gesungen erkannt unter den stummen.
- **Oktave:** unter den beiderseits gesungenen Rahmen der Anteil mit einer ganzen Oktave daneben (±50 Cent).

| Ausschnitt | Dauer s | RPA 50 % (0,10 / 0,15 / 0,20) | RPA 100 % | Erkannt % | Fehlalarm % | Oktave % |
|---|---|---|---|---|---|---|
| 1 | 33.2 | 92.4 / 95.2 / 96.3 | 92.4 | 92.7 | 0.8 | 0.12 |
| 2 | 35.2 | 90.5 / 95.2 / 91.5 | 90.5 | 92.9 | 0.4 | 2.52 |
| 3 | 21.7 | 91.1 / 93.1 / 94.4 | 91.1 | 91.1 | 1.1 | 0.00 |
| 4 | 18.5 | 73.4 / 82.5 / 87.2 | 73.4 | 73.4 | 0.3 | 0.00 |
| 5 | 23.2 | 91.0 / 93.1 / 95.6 | 91.1 | 91.1 | 3.7 | 0.00 |
| 6 | 23.8 | 87.4 / 94.7 / 97.5 | 87.4 | 87.4 | 1.7 | 0.00 |
| 7 | 19.5 | 94.3 / 95.7 / 97.1 | 94.3 | 94.3 | 13.0 | 0.00 |
| 8 | 17.0 | 88.7 / 90.2 / 92.7 | 88.7 | 88.7 | 0.1 | 0.00 |
| 9 | 20.4 | 91.4 / 94.6 / 96.6 | 92.0 | 92.1 | 9.6 | 0.00 |
| 10 | 9.1 | 81.8 / 88.4 / 90.7 | 82.0 | 82.0 | 12.9 | 0.00 |
| 11 | 13.0 | 93.8 / 96.5 / 97.9 | 93.8 | 93.8 | 10.2 | 0.00 |
| 12 | 15.2 | 98.2 / 99.0 / 99.3 | 98.2 | 98.2 | 9.9 | 0.00 |
| 13 | 22.4 | 77.3 / 89.2 / 93.0 | 77.3 | 77.3 | 1.2 | 0.00 |
| 14 | 12.2 | 92.2 / 94.8 / 96.4 | 92.4 | 92.4 | 10.2 | 0.00 |
| 15 | 20.3 | 93.1 / 95.2 / 95.8 | 93.3 | 93.3 | 17.6 | 0.00 |
| 16 | 17.3 | 94.3 / 96.4 / 97.9 | 94.3 | 94.3 | 6.2 | 0.00 |
| 17 | 23.6 | 96.4 / 98.1 / 99.2 | 96.6 | 96.6 | 2.8 | 0.00 |
| 18 | 29.1 | 94.5 / 95.5 / 96.3 | 94.6 | 94.6 | 2.2 | 0.00 |
| 19 | 30.5 | 85.8 / 90.8 / 93.7 | 85.8 | 85.8 | 3.6 | 0.00 |
| 20 | 8.7 | 91.0 / 93.2 / 94.9 | 91.2 | 91.2 | 2.5 | 0.00 |
| 21 | 22.0 | 76.7 / 76.9 / 78.4 | 76.7 | 76.7 | 1.2 | 0.00 |
| 22 | 18.3 | 88.6 / 91.6 / 93.9 | 88.6 | 88.6 | 1.2 | 0.00 |
| 23 | 18.0 | 58.9 / 82.0 / 87.3 | 58.9 | 58.9 | 0.1 | 0.00 |
| 24 | 14.8 | 93.6 / 95.7 / 95.9 | 94.1 | 94.2 | 15.5 | 0.00 |
| 25 | 14.7 | 93.6 / 96.1 / 98.8 | 93.6 | 93.6 | 14.0 | 0.00 |
| 26 | 12.9 | 70.7 / 80.2 / 87.2 | 74.5 | 74.7 | 8.7 | 0.00 |
| 27 | 14.0 | 91.0 / 92.3 / 94.5 | 91.4 | 91.4 | 1.6 | 0.00 |
| 28 | 12.9 | 83.0 / 87.5 / 88.6 | 84.0 | 84.0 | 2.0 | 0.00 |
| 29 | 14.9 | 92.9 / 94.3 / 95.5 | 92.9 | 92.9 | 10.2 | 0.00 |
| 30 | 13.0 | 66.0 / 79.5 / 80.7 | 66.0 | 67.1 | 8.7 | 1.59 |
| 31 | 14.0 | 79.2 / 87.0 / 90.8 | 79.4 | 79.4 | 5.1 | 0.00 |
| 32 | 25.0 | 95.4 / 96.6 / 97.7 | 95.4 | 95.4 | 13.1 | 0.00 |
| 33 | 30.0 | 80.7 / 89.0 / 93.4 | 80.7 | 81.2 | 5.3 | 0.55 |
| 34 | 24.9 | 40.1 / 69.6 / 84.8 | 40.1 | 40.1 | 0.0 | 0.00 |
| 35 | 13.8 | 77.8 / 86.8 / 96.2 | 77.8 | 77.8 | 0.7 | 0.00 |
| 36 | 23.3 | 72.7 / 90.3 / 93.5 | 72.7 | 72.7 | 0.5 | 0.00 |
| 37 | 38.7 | 98.5 / 98.9 / 99.2 | 98.5 | 98.5 | 2.6 | 0.00 |
| 38 | 27.8 | 84.9 / 89.5 / 93.5 | 84.9 | 84.9 | 2.4 | 0.00 |
| 39 | 31.7 | 96.7 / 97.8 / 97.8 | 96.7 | 96.7 | 4.2 | 0.00 |
| 40 | 18.6 | 94.7 / 96.3 / 97.1 | 94.7 | 95.3 | 9.9 | 0.67 |

(je Ausschnitt RPA 100, Erkannt, Fehlalarm und Oktave mit Mittelwert 0,10)

## Gesamt

| Mittelwert der Schwellen | RPA 50 % | RPA 100 % | Erkannt % | Fehlalarm % | Oktave % | Praezision % | F % |
|---|---|---|---|---|---|---|---|
| 0.10 | 86.29 | 86.42 | 86.60 | 4.83 | 0.189 | 97.01 | 91.41 |
| 0.15 | 91.65 | 91.90 | 92.36 | 7.54 | 0.271 | 95.49 | 93.66 |
| 0.20 | 93.97 | 94.24 | 95.12 | 9.73 | 0.495 | 94.12 | 94.18 |

Rahmen gesungen / stumm laut Beschriftung: 92966 / 47816

## Versatz der Rahmen (Gegenprobe der Zeiten, Mittelwert 0,10)

RPA 50 %, wenn die Messung um d Rahmen gegen die Beschriftung verschoben wird — der Gipfel muss bei 0 liegen:

| d | -3 | -2 | -1 | 0 | 1 | 2 | 3 | 
|---|---|---|---|---|---|---|---|
| RPA 50 % | 79.89 | 83.66 | 85.82 | 86.29 | 86.17 | 84.36 | 80.70 | 

**Rechenzeit:** 34.4 s fuer 3 × 817.1 s Ton (1.40 % der Echtzeit je Messung), Build siehe Lauf.

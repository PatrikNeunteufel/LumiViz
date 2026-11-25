# 34k-7c Entwurf des Umlauf-Filters in C++

> test_InstrumentMeasureUmlauf, `LUMIVIZ_34K7C_MESSUNG`. Saiten des Pruefstands S199; UL2 a: 3 Cent, Abklingen −30,1 … +26,2 %. Zeit je Taste bzw. Saite auf einem Thread (Taste) bzw. mit dem Pool (Saite).

## Gitarre (je Saite gebunden)

| Saite | Biquads | Start | Daempfung | Buende | halb | Achtel | fremd Cent | fremd Abklingen % | haelt | s |
|---|---|---|---|---|---|---|---|---|---|---|
| Saite 6 | 20 | 1.02 | 0.05 | 0.06 | 0.04 | 0.05 | 0.06 | 7.52 | ja | 1161.1 |
| Saite 5 | 14 | 1.02 | 0.05 | 0.07 | 0.07 | 0.07 | 0.08 | 3.70 | ja | 1487.5 |
| Saite 4 | 13 | 1.30 | 0.05 | 0.32 | 0.30 | 0.37 | 0.45 | 3.91 | ja | 717.9 |
| Saite 3 | 9 | 1.02 | 0.05 | 0.13 | 0.11 | 0.12 | 0.12 | 4.29 | ja | 91.6 |
| Saite 2 | 8 | 1.02 | 0.10 | 0.18 | 0.10 | 0.21 | 0.23 | 3.55 | ja | 237.9 |
| Saite 1 | 8 | 1.02 | 0.10 | 1.11 | 1.07 | 1.45 | 1.59 | 4.42 | ja | 240.0 |

Gitarre: Entwurf zusammen 3935.9 s.

## Laute (je Saite gebunden)

| Saite | Biquads | Start | Daempfung | Buende | halb | Achtel | fremd Cent | fremd Abklingen % | haelt | s |
|---|---|---|---|---|---|---|---|---|---|---|
| Chor 1.1 | 8 | 1.30 | 0.10 | 0.11 | 0.08 | 0.39 | 0.39 | 12.04 | ja | 23.7 |
| Chor 1.2 | 8 | 1.30 | 0.05 | 0.02 | 0.01 | 0.04 | 0.04 | 2.65 | ja | 39.6 |
| Chor 2.1 | 8 | 1.30 | 0.05 | 0.07 | 0.07 | 0.17 | 0.16 | 7.72 | ja | 52.6 |
| Chor 2.2 | 8 | 1.30 | 0.05 | 0.10 | 0.07 | 0.09 | 0.10 | 3.46 | ja | 65.5 |
| Chor 3.1 | 8 | 1.30 | 0.05 | 0.11 | 0.09 | 0.13 | 0.13 | 8.65 | ja | 42.1 |
| Chor 3.2 | 8 | 1.30 | 0.05 | 0.11 | 0.07 | 0.10 | 0.10 | 2.77 | ja | 159.2 |
| Chor 4.1 | 8 | 1.30 | 0.05 | 0.13 | 0.11 | 0.12 | 0.13 | 1.91 | ja | 70.7 |
| Chor 5.1 | 8 | 1.30 | 0.05 | 0.04 | 0.05 | 0.04 | 0.05 | 2.30 | ja | 71.2 |
| Chor 6.1 | 8 | 1.30 | 0.10 | 0.16 | 0.30 | 0.25 | 0.27 | 3.30 | ja | 32.6 |
| Chor 7.1 | 8 | 1.30 | 0.10 | 1.13 | 0.73 | 1.83 | 2.22 | 3.41 | ja | 95.7 |
| Chor 8.1 | 8 | 1.02 | 0.05 | 7.51 | 9.45 | 10.32 | 12.17 | 21.88 | **nein** | 119.7 |

Laute: Entwurf zusammen 772.8 s.

## Oud (je Saite gebunden)

| Saite | Biquads | Start | Daempfung | Buende | halb | Achtel | fremd Cent | fremd Abklingen % | haelt | s |
|---|---|---|---|---|---|---|---|---|---|---|
| Chor 1.1 | 21 | 1.02 | 0.05 | 0.10 | 0.06 | 0.10 | 0.10 | 3.07 | ja | 723.0 |
| Chor 2.1 | 17 | 1.30 | 0.05 | 0.07 | 0.05 | 0.07 | 0.07 | 1.34 | ja | 321.6 |
| Chor 3.1 | 15 | 1.30 | 0.05 | 0.08 | 0.05 | 0.08 | 0.08 | 3.67 | ja | 354.2 |
| Chor 4.1 | 12 | 1.30 | 0.10 | 0.13 | 0.10 | 0.13 | 0.13 | 1.04 | ja | 207.1 |
| Chor 5.1 | 8 | 1.30 | 0.05 | 0.10 | 0.10 | 0.08 | 0.09 | 3.99 | ja | 88.8 |
| Chor 6.1 | 8 | 1.02 | 0.05 | 0.19 | 0.11 | 0.16 | 0.17 | 3.65 | ja | 119.9 |

Oud: Entwurf zusammen 1814.8 s.

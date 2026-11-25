# 34k-7c Entwurf des Umlauf-Filters in C++

> test_InstrumentMeasureUmlauf, `LUMIVIZ_34K7C_MESSUNG`. Saiten des Pruefstands S199; UL2 a: 3 Cent, Abklingen −30,1 … +26,2 %. Zeit je Taste bzw. Saite auf einem Thread (Taste) bzw. mit dem Pool (Saite). Rangverlust: beide; Tasten / Saiten: T32.

## Cembalo (Tafel je Taste)

| Taste | f1 Hz | zaehlend | Abel-Smith | Glied | Biquads | Cent | Abklingen % | haelt | s |
|---|---|---|---|---|---|---|---|---|---|
| 32 (TK2 a) | 51.91 | 239 | 88 | T5 | 88 | 1.22 | 3.78 | ja | 793.2 |
| 32 (scipy) | 51.91 | 239 | 88 | T5 | 88 | 1.23 | 4.11 | ja | 254.6 |

Cembalo: 2 von 2 Tasten halten UL2 a; Entwurf zusammen 1047.7 s.

## Rechenaufwand (RZ2 a)

| Instrument | Taste / Saite | Fassung | Laeufe | Schritte | verworfen | Auswertungen | Ableitungen | SVD | Rangverluste | s Residuen | s Ableitungen | s SVD | s Anpassung | s Rest | s gesamt |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Cembalo | T32 | TK2 a | 26 | 6708 | 1864 | 8598 | 6734 | 6710 | 6710 | 6.8 | 4.7 | 780.0 | 792.9 | 0.3 | 793.2 |
| Cembalo | T32 | scipy | 26 | 1832 | 780 | 2638 | 1858 | 1832 | 1832 | 2.0 | 1.2 | 250.8 | 254.3 | 0.3 | 254.6 |

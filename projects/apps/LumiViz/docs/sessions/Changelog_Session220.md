# Changelog Session 220 (2026-11-24)

> Diese Sitzung hat die Vorlagen aus Herleitung §12 abgeschlossen (KQ75 a), Stufe 34k-7 geplant (ZK1 a, ZK2 a) und
> die ersten drei Schritte gebaut: 34k-7a (Bezug der Zielgrößen), 34k-7b (der Körper am Steg im Tonauftrag), 34k-7c
> (Umlauf-Filter der gezupften Saite: Glieder und Entwurf in C++). Im Messfall über alle Tasten verfehlte nur Cembalo
> T73 die Schwelle; die Ursache war ein Fehler im Vertrauensbereich der Anpassung bei Rangverlust, behoben mit TK2 a.

## Neu

- **KQ75 a** (Commit `f57240ed`): Tastenstoß des Cembalos an Penttinen 2006 Abb. 6 nachgelesen — GZ10 a bleibt;
  Lautenzug gegen den Werkbank-Befund geprüft, 4′ bleibt bei 31g.
- **ZK1 a:** die Tafel des Umlauf-Filters ist ein gestimmter Wert (`InstrumentMeasure` entwirft, `InstrumentTuneTool`
  schreibt in die `.lvinst`). **ZK2 a:** Saite und Bund in `KettenNote` schon in 34k-7. Teilplan 34k-7a–f im Konzept §6.
- **34k-7a:** Herleitung §11 auf die Messung nach KQ36 a umgestellt; die Harfe am Körper der Fassung 8 an 16 Hörorten
  neu (ab D2 mit Mitschwingen 12,4 / 11,6 / 1,6 LU, hält GZ14 a; `proben/2026-11-24_S220_harfe16*`).
- **34k-7b:** `InstrumentCore` 0.7.0 — `Steg.hpp` (Moden mit Richtung, Stegpunkte mit Formwert und Streuung, Dichte
  des statistischen Teils), `Tonauftrag::steg`, `KettenNote::saite` / `bund`, `KernAuskunft::stegImAuftrag`,
  `Stegkraft`, `Tonkern::rechneAmSteg` (Journal J79). Kein Kern liest das Neue.
- **34k-7c:** `InstrumentModels` 0.8.0 `gemeinsam/Umlauf` (Ziel der Saite, Thiran, Biquad-Kette, Verlust, im Takt),
  `InstrumentMeasure` 0.5.0 `UmlaufEntwurf` (`entwirfJeTaste`, `entwirfJeSaite`, `bewerte`, `filterAn`), angepasst
  wie scipy `least_squares` trf mit eigener SVD (Journal J80). Neu `test_InstrumentMeasureUmlauf` (fünf Fälle, einer
  der Messfall `LUMIVIZ_34K7C_MESSUNG=<datei.md>`, mit `LUMIVIZ_34K7C_NUR=Harfe,Cembalo` auf Instrumente beschränkt).
- **TK1 a / TK2 a:** Cembalo T73 — läuft ein Winkel der Kette nach π, ist eine Spalte der Jacobi-Matrix exakt null;
  scipy rechnet dann α < 0 und einen Schritt, der das Modell vergrößert. Behoben: Schritt kleinster Norm mit α = 0,
  Logit-Parameter auf ±40 gehalten.

## Geändert

- `Solution.json`: `InstrumentCore` 0.7.0, `InstrumentModels` 0.8.0, `InstrumentMeasure` 0.5.0.
- Konzept 0.89.0, Herleitung 1.4.0 (§9, §11, §12), Journal 0.21.0 (J79, J80), Offene Punkte 1.431.0, Modul-Docs
  `InstrumentCore.md` 0.6.0, `InstrumentModels.md` 0.8.0, `InstrumentMeasure.md` 0.5.0. **1938 Cases** (1930 + 3 in
  `test_InstrumentCore` + 5 in `test_InstrumentMeasureUmlauf`).

## Zahlen

| Instrument | Messfall (vor TK2 a) | mit TK2 a | Entwurf |
|---|---|---|---|
| Harfe | 80 / 80 Tasten | 80 / 80 | 24 s |
| Cembalo | 60 / 61 (T73 3,27 Cent) | 61 / 61 (T73 0,01 Cent / 0,21 %) | 50 min (T32 allein 917 s) |
| Gitarre | alle Saiten (fremd ≤ 1,59 Cent) | steht aus | 50 min |
| Laute | alle außer Chor 8.1 (bekannt, UL3-1 a) | steht aus | 11 min |
| Oud | alle Saiten (fremd ≤ 0,17 Cent) | steht aus | 27 min |

Mit TK2 a ändern sich 17 Tasten von Harfe und Cembalo, alle bleiben in der Schwelle (Harfe T98 jetzt 2,44 Cent /
3,24 % mit 2 statt 4 Biquads). Tafeln: `proben/2026-11-24_S220_umlauf_cpp.md`, `…_umlauf_cpp_tk2.md`.

## Offen

- Gitarre, Laute, Oud mit TK2 a messen (rund 1,5 h); die Rechenzeit des Entwurfs aufschlüsseln (Cembalo T32 vor
  TK2 a 257 s, danach 917 s — Wärme nicht getrennt).
- 34k-7d (Kern, Rechenfassung 2), 34k-7e (App), 34k-7f (Abschluss); mit dem Stufenabschluss die Doku-Lücke `zupf/`,
  `ZupfStimmen`, `Grundton` in den Modul-Docs.

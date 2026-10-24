# Changelog Session 148 (2026-10-23 bis 2026-10-24)

> **Der Resonanzboden des Steinway D ist gerechnet.** Aus dem Bauplan entsteht ein Körper: die Schwingungen des Bodens
> bis 5 kHz, darüber eine statistische Fortsetzung bis 20 kHz, je Schwingung wie stark sie an jedem Punkt des Stegs
> wirkt und wie laut man sie an zwei Hörorten über dem Flügel hört. Das Programm liest den Körper erst mit dem Klang
> (31c-7); zu hören ist noch nichts.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (EG1 a, EG2 a, BM6 a, BM6-1 a, PL31c-5 a, ST1 a).

## Neu

- **Der Körper `.lvinstbody`** des Steinway D, mitgeliefert (2,9 MB): 820 gerechnete Schwingungen bis 5 kHz, darüber
  rund 5900 statistisch fortgesetzte bis 20 kHz; je Schwingung Frequenz, Dämpfung, Wert an den 88 Stegpunkten und das
  Gewicht zu den Hörorten.
- **Hörorte im Bauplan:** ein Stereopaar 12 cm über den Saiten, wie die Mikrofone der Aufnahmen von *Salamander*.
- **Werkzeug `InstrumentBodyTool`** (nicht Teil der App): rechnet aus einem Bauplan den Körper, rund 22 Minuten für
  den Steinway.

## Geändert

- Der Bauplan `.lvinstbodyplan` steht in Fassung 2 (Hörorte); Fassung 1 bleibt lesbar.

## Intern

- Bibliothek `InstrumentBody`: Eigenlöser (`Eigenloeser`, `EigenloeserSpectra` — Zählung nach Sylvester, Teilfenster,
  Deflation), `Umsetzungen` (jede Umsetzung an einer Stelle), `Auswertung` (Stegpunkte, Dämpfung nach RT-0425 Gl. 1,
  Rayleigh), `Statistik` (Ziehung nach Boutillon & Ege, eigener Zufall), `Koerper`; Modul-Doku 0.8.0.
- `loop/Koerper` (die Datei, Abdruck des Bauplans); ein Wächter meldet einen Körper, der nicht mehr zu Bauplan oder
  Rechnung passt.
- Neues External, nur für Tests und Werkzeug: **Spectra 1.2.0** (MPL-2.0); `THIRD_PARTY_NOTICES.md` nachgezogen.
- Neue Tests `test_LoopKoerperEigen` (6), `test_LoopKoerperAuswertung` (5), `test_LoopKoerperStatistik` (3),
  `test_LoopKoerperDatei` (3); 1555 Fälle. Unter Release-clang grün, Testing-MSVC baut; **kein voller Lauf**.
- Bauzeit gemessen: unter MSVC kompilieren die Ziele ohne `/MP` (Unit-Tests 14,7 statt 3,1 min) — wird in CMakeCraft
  behoben.

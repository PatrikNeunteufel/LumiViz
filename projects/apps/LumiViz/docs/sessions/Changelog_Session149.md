# Changelog Session 149 (2026-10-24)

> **Der Rest des Resonanzbodens ist fest geplant, zwei Teile des Prüfsteins sind gebaut.** Bevor weitergebaut wurde,
> sind alle Fragen geklärt, die den Umfang der übrigen Schritte von 31c bestimmen; der Plan bis zum Abschluss steht
> fest. Dazu: das Fenster *Instruments* öffnet sofort, und die Dämpfung des Bodens folgt jetzt seinen Werkstoffen.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (BM7 a, PL31 a, PS1 a, PF1 a, KV1 a, AW1 a, FT1 a, PG1 a,
> SB1 a, DM1 a, GL1 a, VA1 a, ZA1 a, EM1 a); `docs/audio/Loop_Konzept.md` §4n (IP1 a).

## Geändert

- **Fenster *Instruments*:** öffnet sofort; die Abweichung der eigenen Instrumente vom Bezug wird im Hintergrund
  gemessen und gemerkt — der Visualizer bleibt dabei nicht mehr stehen (IP1).
- **Körper `.lvinstbody`** Fassung 2: je Schwingung, wie viel ihrer Energie in welchem Werkstoff steckt; daraus die
  Dämpfung (für Fichte und Buche des Steinway unverändert, für einen Glasboden später verschieden). Fassung 1 bleibt
  lesbar. Der Steinway D ist neu gerechnet.

## Neu

- **Werkzeug `InstrumentBodyTool --report`** (nicht Teil der App): schreibt die Mobilität des Stegs je Stegpunkt und je
  Terzband — für die Prüfung gegen Messungen.

## Intern

- Bibliothek `InstrumentBody`: `Verformungsenergie` (Energie je Werkstoff), `Mobilitaet` (MAESSTRO Gl. 10 und 12,
  Terzbänder); Modul-Doku 0.10.0.
- Build-System **CMakeCraft v0.13.1**: unter MSVC parallel übersetzt (`/MP8`), die Konfiguration Testing jetzt optimiert
  mit Asserts und Debug-Info und mit derselben C-Laufzeit wie Qt (`/MD`), `/bigobj` für alle Ziele; das oberste
  `CMakeLists.txt` trägt `project(LumiViz LANGUAGES NONE)`.
- Recherchen: Mobilität am Steg oberhalb 1 kHz, Werkstoffwerte für Glas (`docs/modellierung/hammer-saite/recherche/`).
- Neue Tests `test_LoopKoerperMobilitaet` (3), dazu Fälle in `test_LoopKoerperAuswertung` (3) und `test_LoopPegel` (1);
  1562 Fälle. Unter Release-clang eng grün, `*Loop Koerper*` auch unter Testing-MSVC; **kein voller Lauf** (kommt mit
  dem Abschluss von 31c).

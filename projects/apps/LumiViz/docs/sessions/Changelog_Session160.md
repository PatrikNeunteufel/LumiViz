# Changelog Session 160 (2026-10-28)

> **Die Nocturne atmet enger, und der Boden des Flügels klingt im Bass sauberer an.** Die Dynamik-Tafel der
> Nocturne spannte 30 dB, wo ein Pianist rund 21 dB spielt; sie ist nach gemessenen Hammergeschwindigkeiten neu
> gesetzt. Der Resonanzboden bekommt die an echten Böden gemessene Dämpfung der tiefsten Moden: C2 setzt in 54 ms
> statt 133 ms ein, die tiefen Moden klingen in jedem Ton 4–7 dB leiser mit. Der vermutete Kurzschluss der
> Abstrahlung (Platte ohne Schallwand) ist gebaut, geprobt und als wirkungslos an den Hörorten erkannt — er bleibt
> als Option für Hörorte im Fernfeld. Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (Befund HU4, HU4 a,
> HU4-1 a, HU4-2 b, DY2 a).

## Geändert

- **Nocturne Op. 9 Nr. 2** (`nocturne_demo`, DY2 a): Tafel ppp −21, pp −18, p −14, mp −11, mf −8, f −4, ff 0 dB der
  Anschlagstufe (vorher −30 … 0), Akzent und Gabel 4 dB statt 6, Fader +1,2 dB (Spitze −1,0 dBFS). Grundlage:
  gespielte Töne reichen von 0,37 bis 4,07 m/s Hammergeschwindigkeit (Goebl, Bresin & Galembo 2004), das Modell
  spreizt 0,9–1,4 dB je dB Stufe. Loudness Range 21,6 → 15,5 LU.
- **Resonanzboden des Flügels** (HU4-1 a): das Dämpfungsgesetz α = a·f² + b·f + c steht jetzt im Bauplan
  (`.lvinstbodyplan` Fassung 3, Block `damping`) und trägt für den Steinway D c = 4,5 1/s — die tiefsten Moden fallen
  mit 2–3 % statt 1,1 % (Ege, Boutillon & Rébillat 2013). `steinway_d` und `steinway_d_glass` sind neu gerechnet;
  *Grand Piano* Level 4,2418 (Pegel neu gemessen).

## Intern

- Befund HU4: der Überschuss des Modells im Bass gegen *Salamander* sitzt in einzelnen Moden (38, 61, 165 Hz mit
  3- bis 8-fachem Abstrahlgewicht), nicht in einem Frequenzgang; Hochpass, Dämpfung und Streckstimmung an Kraft ×
  Antwort gerechnet; A0 (24 ms) ist mit keinem Holzboden erreichbar, solange ein Teilton auf einer Mode sitzt.
- HU4 a gebaut und geprobt: Rückseite der Platte im Rayleigh-Integral (`Rueckseite`, Block `backside`,
  `naechsterRandpunkt`), Probe am Steinway: 0 dB an den Hörorten 12 cm über den Saiten (Nahfeld); bleibt im Format,
  Vorgabe aus (HU4-2 b). Test „Loop Koerper HU4 Rueckseite: Kolben ohne Schallwand“.
- `steinway_d_glass_same_mass` ist noch mit dem alten Gesetz gerechnet (1,3 h; S161) — der Wächter meldet ihn.
- Klangfassung 5; `loop_werte.txt` (Zeilen des Flügels und der Nocturne), `klangfassung.txt` neu; Test
  „31c-7 Antwort: Laenge am Steinway“ auf die kürzere Antwort gesetzt.
- Tests: 1644 Cases (+1); Bereich unter Release-clang 219 / 220 — rot nur der Wächter des dritten Körpers; kein
  voller Lauf (steht seit HU3-1 c aus), kein Testing-MSVC.
- Skripte lokal unter `.claude/handover/hoerproben_31d/messung/` (`hu4_optionen.py`, `hu4_probe_messen.py`,
  `terz_trend.py`, `spreizung_*.py`, `nocturne_lautheit.py`), Ergebnisse unter `messung/hu4/`.

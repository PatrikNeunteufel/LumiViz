# Changelog Session 157 (2026-10-27)

> **Der Flügel ist spielbar.** Das gerechnete Grand Piano (Hammer-Saite, drei Saiten je Taste, zwei Abklingstufen,
> durch den Boden des Steinway D) liegt als mitgeliefertes Instrument *Grand Piano* im Fenster *Instruments*, auf dem
> Pegel der anderen Instrumente, mit einem kurzen Klassik-Loop zum Anhören. Es klingt mit dem besten gemessenen Stand
> (an *Salamander* gesetzt, Session 152); die Größen ohne Quelle und ohne Messung stellt Patrik als Nächstes nach Gehör
> ein, die Hörproben dafür liegen bereit. Das automatische Einstellen an der Aufnahme folgt danach als Feinabstimmung.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (SP1 a bis SP3 a, Teilplan 31d-8).

## Neu

- **Instrument *Grand Piano*** (`asset/loop/instruments/grand_piano.lvinst`, Art *Keys*, Genre *Classical*): die
  Vorgabe der Hammer-Saite mit dem Boden `lib:steinway_d` und der Tastatur A0–C8 (ungedämpft ab F6, Pedale Una corda,
  Sostenuto, Sustain). Vom Programm geschrieben, nicht von Hand.
- **Loop *Grand piano demo*** (`asset/loop/grand_piano_demo.lvloop`): acht Takte, 72 BPM, gebrochene Akkorde links,
  Melodie rechts; erzeugt mit `asset/loop/make_fluegel_loops.py`.

## Intern

- Werkzeug und Wächter für `grand_piano.lvinst` in `test_LoopHammerSaite.cpp` (`LUMIVIZ_GRAND_PIANO_SCHREIBEN=1`
  schreibt die Datei aus der Vorgabe; ohne die Variable meldet der Fall eine veraltete Datei).
- `instrument_katalog.ergaenze` trägt die Tastatur (`keyboard`) mit durch — vorher fiel sie beim Schreiben still weg.
- Pegel des Flügels gemessen (−25,8 LUFS bei Level 1), Level 4,1788 (rund +12,4 dB) auf den Bezug −13,4 LUFS.
- `werte/loop_werte.txt` und `werte/klangfassung.txt`: nur vier Einträge dazu (drei Töne, der Loop); bestehende
  Zeilen unverändert.
- Hörproben-Werkzeug `LUMIVIZ_31D8_WAV` in `test_LoopSalamanderHoerproben.cpp` (u, γ, δ gegen *Salamander*); die
  gemeinsamen Hilfen der Hörproben-Fälle in den Namensraum gezogen.
- **Befund:** der Winkel der zweiten Schwingungsebene (γ) wirkt im Modell nur zusammen mit ihrer Verstimmung (δ) —
  an Tasten mit Chor (δ = 0) sind alle Werte von γ Bit für Bit gleich.
- Konzept: Lage S157, SP1 a (spielbar vor der Feinabstimmung), SP2 a (Suche über die Stützpunkte der Kurven), SP3 a
  (Eindeutigkeit an den Messgrößen), Teilplan 31d-8.
- Zwei ältere Tests nachgezogen, deren Regeln vor der Hammer-Saite geschrieben wurden (`inALegato` je Schicht;
  ein Loop mit Hammer plant mit der Auskunft anders) — das Programm blieb.
- Tests: 1634 Cases (+2: Wächter `grand_piano`, Hörproben 31d-8); Testing-MSVC voller Lauf 1634 / 1634, 0 Skips;
  Release-clang voller Lauf 1632 / 1634 vor den zwei Nachträgen, danach beide Dateien grün.

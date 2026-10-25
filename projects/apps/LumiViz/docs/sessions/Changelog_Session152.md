# Changelog Session 152 (2026-10-25)

> **Der gerechnete Flügel ist an einer echten Aufnahme gemessen.** Das *Salamander Grand Piano* (Yamaha C5, CC BY 3.0)
> ist Ton für Ton vermessen, das Modell genauso, und die Größen ohne Quelle sind danach gesetzt: Abklingzeiten,
> Verstimmung, die Steifheit der Basssaiten, der Winkel am Steg, die Tasten ohne Dämpfer. Dabei zeigte sich, dass die
> gekoppelten Saiten des Modells an manchen Stellen fast vollständig verstummen, wo ein echter Flügel nur leiser wird —
> die nächste Stufe gibt jeder Saite eine zweite Schwingungsebene.
> Steuerdokument: `docs/audio/Spielweisen_Konzept.md` §4.1 (WL1 a, D1 a, V1 a, S1 a bis S3 a, N1 a, N2 a).

## Verbessert

- **Die Hammer-Saite klingt nach einem gemessenen Flügel:** Abklingzeit, Abklingen der hohen Teiltöne, Verstimmung im
  Chor, Inharmonizität im Bass und der Winkel am Steg sind an *Salamander* gemessen und durch das Modell hindurch gesetzt.
  Inharmonizität und Winkel halten auch die strengere Messung; die Abklingzeiten und die Verstimmung werden nach der
  zweiten Schwingungsebene (31d-7) neu gesetzt — die erste Setzung stützte sich auf eine Messung, die an tiefen
  Einbrüchen falsch lag.
- **Die obersten 20 Tasten haben keinen Dämpfer** (ab F6, wie der aufgenommene Flügel; vorher ab G6, ohne Beleg).

## Intern

- WAV-Dateien werden ohne Qt gelesen (`audio/WavFormat`); `audio::liesWav` öffnet nur noch die Datei.
- Messung eines Flügeltons ohne Qt (`Teiltonmessung.hpp`, `Fluegelmessung.hpp`): Teiltöne, Inharmonizität, zwei
  Abklingstufen (nach Schroeder, robust gegen Auslöschungen), Schwebung, Helligkeit, Pegel, Direktschall, Terzbänder,
  Phantomteiltöne; eine Blende am Ende einer Aufnahme wird erkannt.
- `werte/salamander_c5.txt`: die Messung an *Salamander* (nur Zahlen, Nennung CC BY 3.0).
- Werkzeuge in den Tests: Messung von *Salamander* (`LUMIVIZ_SALAMANDER`), Vergleich mit dem Modell
  (`LUMIVIZ_31D3_MODELL`), Setzen durch das Modell hindurch (`LUMIVIZ_31D4_SETZEN`), Hörproben gegen die Aufnahme
  (`LUMIVIZ_31D_WAV`); ein dauerhafter Test prüft das Modell an der Messung.
- **Voller Lauf 1601 / 1601 unter Release-clang und Testing-MSVC** am Ende jedes Teilschritts.
- Neu im Plan: 31d-7, die zweite Schwingungsebene der Saiten (Weinreich), danach werden die Abklingzeiten neu gesetzt.

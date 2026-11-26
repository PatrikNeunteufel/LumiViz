# Changelog Session 222 (2026-11-26)

> Diese Sitzung hat 34k-7d geschnitten (Schnittfolge 7d-1 bis -6) und die ersten drei Schritte begonnen: die Bezugsdatei
> des Prüfstands, die gestimmte Liste im Core, die Saiten und die Tafel des Umlauf-Filters als Liste. Der Entwurf der
> Tafel für alle Instrumente ist zweimal am Absturz des Rechners gescheitert und läuft in der nächsten Sitzung in
> Stücken weiter.

## Entscheide (Konzept 0.90.0)

- **ZB1 a:** Rechenfassung 2 ersetzt Rechenfassung 1 im Schritt 34k-7d-4; `zupf_ic.txt` trägt danach die Abdrücke von
  Rechenfassung 2, vom Kern geschrieben, mit Vermerk.
- **ZU1 a:** die Quelle des Zupfs je Note genau aus der Frequenzantwort, mit einer FFT.
- **ZW1 a:** Gitarre, Laute, Oud zupfen 16 cm vom Steg, für alle Bünde gleich. Gegen die Iowa-Gitarre (mf, 65 Töne)
  Teiltonprofil 6,72 dB bei 16 cm gegen 7,94 / 7,88 bei 10 / 6,5 cm; über Bund 12 fest 8,06 dB gegen 22,72 mit der Kappe
  auf die halbe Saite (gerade Teiltöne −29 … −35 dB).
- **TH1 a:** der lange Entwurf mit weniger Threads (6) und nur auf CCD2 (logische Prozessoren 16–31, geprüft mit Last
  und den Sensoren).

## Neu

- **34k-7d-1:** `proben/2026-11-26_S222_bezug_zupf.py` schreibt `tests/unit/UnitTests/werte/zupf_pruefstand.txt` — 17
  Fälle aller fünf Instrumente: Eingaben je Saite, Gewichte des Dämpfers (sieben breite Moden, nur senkrecht),
  Teiltöne der Stegkraft, Pegel, Lautheit am Hörort. Gegenprobe im Zeitschritt des Kandidaten S200
  (`…_bezug_zupf_zeitschritt.txt`): an dessen Stichtasten wie damals; die Abweichung kurzer Saiten (Harfe G6 −0,32 LU)
  liegt am FIR des Kandidaten (`…_umlauf_fir_kurz`: |E| 30,7 dB/s, FIR 34,7 dB/s).
- **Befund Harfe:** der Prüfstand rechnet das Ablösetempo als Rampe der Stegkraft, die nie endet (nach 3 s rund 10⁴ N);
  Bezug und Kern rechnen es als verteilten Kraftstoß über das Dreieck — an den Teiltönen ≤ 0,04 dB, am Hörort ≤ 0,02 LU
  gleich (`…_harfe_tempo`, Herleitung 1.5.0 §3).
- **34k-7d-2:** `InstrumentCore` 0.8.0 — ein gestimmter Wert darf eine Liste sein; `InstrumentMeasure` 0.6.0 —
  `GestimmteWerte` je Name ein `kern::Wert`.
- **34k-7d-3 (in Arbeit):** `InstrumentModels` 0.9.0 — in `gemeinsam/Umlauf` die Teile zur Laufzeit (Raster,
  `grundtonDerSaite`, `stimmeAmGrundton`, `Saitentafel`, `filterAn`, `sigmoid`, `legendre`), `zupf/Saiten` (die Saiten
  von Rechenfassung 2), `zupf/UmlaufTafel` (die Tafel als Liste); `InstrumentMeasure` 0.7.0 — der Entwurf rechnet mit
  ihnen Bit für Bit wie vorher, `entwirfUmlaufJeTaste` / `entwirfUmlaufJeSaite`; `InstrumentCore` 0.9.0 —
  `setzePoolHoechstens` (eine Grenze der Threads für den Prozess).
- `test_InstrumentModelsZupfBezug.cpp`: die Saiten gegen den Bezug, die Tafel als Liste hin und zurück, der Schreiber
  (`LUMIVIZ_ZUPF_UMLAUF_SCHREIBEN=<instrument>[:<von>-<bis>],…`, `LUMIVIZ_POOL_THREADS`; schreibt nach jeder Taste bzw.
  jedem Chor) und der Leser der Tafel. `werte/zupf_umlauf.json`: Harfe (80 Tasten, alle halten UL2 a), Cembalo T29–T31.
- Probe `proben/2026-11-26_S222_zupfort_gitarre.py` mit Ausgaben.

## Geändert

- Konzept 0.90.0 (Entscheide, Schnittfolge), Herleitung 1.5.0, Offene Punkte 1.433.0; `test_InstrumentMeasureUmlauf`
  nimmt die Saiten aus `zupf/Saiten` und prüft, dass Glied und gebundene Tafel zur Laufzeit dasselbe Filter geben wie der
  Entwurf; `test_InstrumentCore`, `test_InstrumentStimmen` je ein Fall mehr. **1945 Cases**.

## Verifikation

- Release-clang: `*34k-7c*,*34k-7d*,*34k-3*` grün; die Meldungen von `*34k-7c*` Zahl für Zahl wie S221; nach der
  Grenze im Pool `*34c Pool*` und die Fälle 34k-7d grün.
- Testing-MSVC: `*34k-7c*,*34k-7d*,*34k-3*` 16 / 16 vor der Grenze im Pool und dem geteilten Schreiber — **danach nicht
  neu gebaut**.
- Kein voller Lauf.

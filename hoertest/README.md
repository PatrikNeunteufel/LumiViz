# Hörtests

Hier liegen die Hörtests von LumiViz: Kataloge der Merkmale, Listen und Antworten. Durchgeführt werden sie mit dem
Werkzeug **HoerTest_Helper** (`…\Visuals_Project\cmake\HoerTest_Helper`, Konzept dort `docs/Konzept.md`; Entscheide
HT1 a bis HT9 a, S166).

**Versioniert** (HT8 a):
- Kataloge (`merkmale/`), Listen (`listen/<liste>/`), Antworten und Auswertungen (`ergebnisse/<liste>/`);
- je Liste `klaenge.sha256`: Abdruck jedes Klangs und der Befehl, der sie schreibt.

**Nicht im Repo — der Hörort** (seit S182, AU3, AU3-1 a): die Klänge liegen mit einer Kopie der Liste unter
`…\Visuals_Project\cmake\samples\Hoerproben\<LumiViz|IC>\<liste>\` (Liste, `klaenge.sha256`, `klaenge/` beisammen,
wie das Konzept §11/§12 es verlangt); gehört wird dort, mit `…\Hoerproben\_werkzeug\Hoertest\Hoertest.exe`. Die Fälle,
die eine Liste schreiben, schreiben an den Hörort und legen `liste.json` und `klaenge.sha256` danach hierher
(`HoerlisteAblage.hpp`); der Fall `*Hoerlisten AU3-1*` prüft, dass beide gleich sind. Ist ein Test gehört und
ausgewertet, kommt `abgeschlossen.pass` in den Ordner am Hörort, dann darf er ins Archiv. Der Abdruck bindet jede
Antwort an die gehörten Klänge.

## Bestand

| Liste | Inhalt | Antworten |
|---|---|---|
| `membran_34h` | die sieben Trommeln des Kerns `membrane` (Stufe 34h, S179), Stärken und Spielweisen, IC gegen Synth | — |
| `klasse1_34i` | Upright, Honky-tonk, CP-70, Clavinet (Stufe 34i, S181), Paare zum Namen des Clavinets | — |
| `ic/<klasse>` | die elf Listen aus InstrumentsCalculation (592 Klänge, übergeben S182; offene Befunde und Reihenfolge in `docs/modellierung/anbindung-ic/ic/UEBERGABE_LUMIVIZ_HOERTEST.md`) | — |
| `stab_34g` | die neun Stabspiele des Kerns `bar` (Stufe 34g, S178): je Instrument drei Lagen bei a-30 / a-12 / a0 (81 Einzelpunkte mit Frage), dazu 9 blinde Paare IC gegen `synth_marimba` / `synth_vibraphone` / `synth_kalimba` (IK2-8 a); alle auf −27,2 LUFS, 5 pp-Klänge in der Höhe still (Befund der ungemessenen Vorgaben, wie in IC). Format des Helpers (§12), geschrieben mit `LUMIVIZ_34G_WAV=…\cmake\samples\Hoerproben\LumiViz\stab_34g LumiViz.UnitTests --test-case="Loop Stab 34g Hoerproben*"`, geprüft mit `Hoertest.exe --liste-pruefen` | — |
| `pv1` | Flügel nach PV1 a: A1, C4, C6 je p / mf / ff, *Salamander* / Vorgabe / Suche „ohne A4“, 27 Paare und 3 Kontrollen (`Spielweisen_Konzept.md` §4.1 „8.5 Hörproben (S166)“) | 4 von 30 (2026-10-30, Patrik), mit dem Behelf `tools/hoertest/` |

Die Liste `pv1` steht noch im Format des Behelfs (`hoertest_behelf.json`, Antworten `antworten_behelf.jsonl`). Mit
Stufe S2 von HoerTest_Helper wird sie ins Listenformat übertragen; dann entfällt `tools/hoertest/` (Offene_Punkte
31d-8 · 8.5-HT). Die Klänge liegen am Hörort unter `…\cmake\samples\Hoerproben\LumiViz\hoerproben_pv1\` (bis S182 unter
`.claude/handover/`).

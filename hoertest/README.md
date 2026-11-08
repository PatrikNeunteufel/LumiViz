# Hörtests

Hier liegen die Hörtests von LumiViz: Kataloge der Merkmale, Listen und Antworten. Durchgeführt werden sie mit dem
Werkzeug **HörTest_Helper** (`…\Visuals_Project\cmake\HörTest_Helper`, Konzept dort `docs/Konzept.md`; Entscheide
HT1 a bis HT9 a, S166).

**Versioniert** (HT8 a):
- Kataloge (`merkmale/`), Listen (`listen/<liste>/`), Antworten und Auswertungen (`ergebnisse/<liste>/`);
- je Liste `klaenge.sha256`: Abdruck jedes Klangs und der Befehl, der sie schreibt.

**Nicht versioniert:**
- `listen/<liste>/klaenge/` und `beispiele/`. Sie schreibt das Werkzeug des Tests aus dem Code des Commits neu; der
  Abdruck bindet jede Antwort an die gehörten Klänge.

## Bestand

| Liste | Inhalt | Antworten |
|---|---|---|
| `stab_34g` | die neun Stabspiele des Kerns `bar` (Stufe 34g, S178): je Instrument drei Lagen bei a-30 / a-12 / a0 (81 Einzelpunkte mit Frage), dazu 9 blinde Paare IC gegen `synth_marimba` / `synth_vibraphone` / `synth_kalimba` (IK2-8 a); alle auf −27,2 LUFS, 5 pp-Klänge in der Höhe still (Befund der ungemessenen Vorgaben, wie in IC). Format des Helpers (§12), geschrieben mit `LUMIVIZ_34G_WAV=hoertest/listen/stab_34g LumiViz.UnitTests --test-case="Loop Stab 34g Hoerproben*"`, geprüft mit `Hoertest.exe --liste-pruefen` | — |
| `pv1` | Flügel nach PV1 a: A1, C4, C6 je p / mf / ff, *Salamander* / Vorgabe / Suche „ohne A4“, 27 Paare und 3 Kontrollen (`Spielweisen_Konzept.md` §4.1 „8.5 Hörproben (S166)“) | 4 von 30 (2026-10-30, Patrik), mit dem Behelf `tools/hoertest/` |

Die Liste `pv1` steht noch im Format des Behelfs (`hoertest_behelf.json`, Antworten `antworten_behelf.jsonl`). Mit
Stufe S2 von HörTest_Helper wird sie ins Listenformat übertragen; dann entfällt `tools/hoertest/` (Offene_Punkte
31d-8 · 8.5-HT). Die Klänge liegen lokal unter `.claude/handover/hoerproben_pv1/`.

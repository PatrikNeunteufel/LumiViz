# Herkunft — Lampis, Stegkraft beim Anstreichen von Cello-G-Saiten (mdw Wien)

- **Quelle:** Zenodo, Datensatz 13374477 (2024-08-26), DOI 10.5281/zenodo.13374477, „Bridge force waveforms obtained by
  systematically bowing various cello G-string models“, Alessio Lampis; zugehöriger Aufsatz laut Beschreibung JASA
  Express Letters, https://doi.org/10.1121/10.0034330. `waveforms.zip` (3 007 564 144 B, MD5 der Quelle `ccdf7dec…`
  geprüft) geladen am **2026-10-27** (IC Session 3, Klasse 3 F-9 a), außerhalb ausgepackt (Ordner `waveforms/`
  weggelassen), Archiv danach gelöscht.
- **Lizenz:** Zenodo-API `cc-by-4.0` (nachgelesen 2026-10-27) — Namensnennung nötig.
- **Inhalt (Zenodo-Beschreibung, nachgelesen):** Stegkraft „at different bow forces and bow accelerations“ für vier
  Saitenmodelle A–D, je Probe 1 oder 2 (`model_<X>_<Y>/`); darunter je Messsitzung ein Ordner `yyyy-mm-dd(_r)` („_r“ =
  umgekehrte Reihenfolge); Dateien `Fb_<Kraft>_a_<Beschleunigung>.wav` (Kraft in N, Beschleunigung in m/s²). „To obtain
  the bridge force in Newton, it is sufficient to multiply the signals by 10.“ Im Aufsatz nur die Proben 2 ausgewertet.
- **Umfang:** 37 295 WAV in sieben Ordnern (`model_C_1` fehlt im Archiv), 5 099 754 336 B. Geprüft: alle mono,
  **64-bit-Gleitkomma**, 50 000 Hz — wie die Beschreibung („sampled at 50kHz at a 64 bit rate“).
- **Prüfsummen:** `SHA256SUMS`.
- **Zweck:** Einschwingen und Anstreichbarkeit (Guettler-Diagramm), Reibmodell ([messplan.md](../../messplan.md) M2, M5).
- **Übernahme nach LumiViz:** siehe [UEBERNAHME_LUMIVIZ.md](../../../../UEBERNAHME_LUMIVIZ.md).

## Ablage (A-1 a, Patrik 2026-10-26)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Klasse 3 - Bogen-Saite\Lampis_Zenodo13374477\` (auch für LumiViz erreichbar). Im Repo bleiben diese Datei, `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei) — damit lässt sich jede Datei wiederbeschaffen und prüfen, auch in einer Cloud-Sitzung (Domain der Quelle in der Umgebung freischalten).

# Herkunft — CNSM Dataset (drei Violinen: Stegadmittanz, Bewegung, Audio)

- **Quelle:** Zenodo, Datensatz 18696786 (Version 1.0.0, 2026-02-19; der in [aufnahmen.md](../../aufnahmen.md) genannte
  Eintrag 18696785 ist der Elterneintrag und leitet hierher), DOI 10.5281/zenodo.18696786, „CNSM Dataset“, Autor laut
  Zenodo-API Hugo Pauget Ballesteros. `dataset.zip` (654 025 212 B, MD5 der Quelle `75f02506…` geprüft) geladen am
  **2026-10-27** (IC Session 3, Klasse 3 F-9 a und F-7 a), außerhalb ausgepackt, Archiv danach gelöscht.
- **Lizenz:** Zenodo-API `cc-by-4.0` (nachgelesen 2026-10-27) — Namensnennung nötig.
- **Beschreibung:** Der Zenodo-Eintrag hat **keine** Beschreibung (Feld fehlt, nachgelesen 2026-10-27); was folgt, ist
  aus den Dateien selbst gelesen.
- **Inhalt:** 462 Dateien, 1 826 065 628 B, drei Violinen `Klimke`, `Levaggi`, `Stoppani`:
  - `admittances/` (22): `dataset.csv` (Violine, Phase, Messung, Pfad) und je Messung `measurement_<n>.csv` mit den
    Spalten `f, H_real, H_imag, Force_real, Force_imag, Velocity_real, Velocity_imag` (Schritt 1,5625 Hz, 16 385
    Zeilen) — die **Stegadmittanz**; Phase 1 je vier, Phase 2 zwei bis fünf Messungen.
  - `mocap/` (203): Phasen 1–2, je Violine Stücke (Bach, Glazounov, Mozart, Sibelius, Tchaikovsky), Tonleiter, langer
    Ton, leere Saiten, je Aufnahme `markers.csv` (Kopf „Capture Frame Rate,120.000000“, „Length Units,Millimeters“) und
    `recording.wav` (80 WAV).
  - `recordings/` (237): Sessions 1–3, je Violine und Spieler (`Player <n>`) Tonleitern `scale_<n>.wav` (236 WAV).
- **Format geprüft:** `recordings/` 236 WAV mono 16 bit 48 kHz; `mocap/` 80 WAV stereo 32 bit 44,1 kHz
  (`WAVE_FORMAT_EXTENSIBLE`).
- **Prüfsummen:** `SHA256SUMS`.
- **Zweck:** Korpus der Violine (Klasse 3 F-7 a; [messplan.md](../../messplan.md)).
- **Übernahme nach LumiViz:** siehe [UEBERNAHME_LUMIVIZ.md](../../../../UEBERNAHME_LUMIVIZ.md).

## Ablage (A-1 a, Patrik 2026-10-26)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Klasse 3 - Bogen-Saite\CNSM_Zenodo18696786\` (auch für LumiViz erreichbar). Im Repo bleiben diese Datei, `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei) — damit lässt sich jede Datei wiederbeschaffen und prüfen, auch in einer Cloud-Sitzung (Domain der Quelle in der Umgebung freischalten).

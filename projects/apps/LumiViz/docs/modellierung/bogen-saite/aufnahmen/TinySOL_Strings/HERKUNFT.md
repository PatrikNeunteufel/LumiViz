# Herkunft — TinySOL, nur Streicher

- **Quelle:** Zenodo, Datensatz 3685367, DOI 10.5281/zenodo.3685367, „TinySOL: an audio dataset of isolated musical
  notes“, Version 6.0 (2020-01-31); Autoren laut Zenodo-API Carmine Emanuele Cella, Daniele Ghisi, Vincent Lostanlen,
  Fabien Lévy, Joshua Fineberg, Yan Maresz. `TinySOL.tar.gz` (1 026 917 185 B, MD5 der Quelle `36030a7f…` geprüft) und
  `TinySOL_metadata.csv` (MD5 geprüft) geladen am **2026-10-27** (IC Session 3, Klasse 3 F-9 a). Aus dem Archiv nur
  `./Strings/` ausgepackt (Python `tarfile`, Filter `data`), `.DS_Store` entfernt, Archiv danach gelöscht.
- **Lizenz:** Zenodo-API `cc-by-4.0` (nachgelesen 2026-10-27) — Namensnennung nötig.
- **Aufnahme (Zenodo-Beschreibung):** „originally recorded at Ircam in Paris (France) between 1996 and 1999, as part of
  a larger project named Studio On Line (SOL)“; nur „‚ordinary‘ style, and in absence of mute“.
- **Inhalt:** 1 193 WAV unter `<Instrument>/ordinario/` — Violin 284, Viola 309, Violoncello 291, Contrabass 309 —
  dazu `TinySOL_metadata.csv` (alle 14 Instrumente, auch die nicht ausgepackten); zusammen 753 901 478 B. Geprüft: alle
  WAV mono, 16 bit, 44 100 Hz.
- **Nicht ausgepackt:** Blech, Holz, Akkordeon. Wird TinySOL für eine andere Klasse gebraucht (etwa Klasse 10
  Akkordeon), das Archiv neu laden — Prüfsumme in `LADELISTE.tsv`.
- **Prüfsummen:** `SHA256SUMS`.
- **Zweck:** Bezug und Gegenprüfung der Einzeltöne ([aufnahmen.md](../../aufnahmen.md)).
- **Übernahme nach LumiViz:** siehe [UEBERNAHME_LUMIVIZ.md](../../../../UEBERNAHME_LUMIVIZ.md).

## Ablage (A-1 a, Patrik 2026-10-26)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Klasse 3 - Bogen-Saite\TinySOL_Strings\` (auch für LumiViz erreichbar). Im Repo bleiben diese Datei, `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei) — damit lässt sich jede Datei wiederbeschaffen und prüfen, auch in einer Cloud-Sitzung (Domain der Quelle in der Umgebung freischalten).

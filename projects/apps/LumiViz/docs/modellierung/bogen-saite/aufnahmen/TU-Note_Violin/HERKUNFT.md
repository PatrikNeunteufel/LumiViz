# Herkunft — TU-Note Violin Sample Library (TU Berlin)

- **Quelle:** DepositOnce (TU Berlin), Handle 11303/7527, DOI 10.14279/depositonce-6747, „TU-Note Violin Sample
  Library“, Henrik von Coler, Jonas Margraf, Paul Schuladen, 2018 (Metadaten der API nachgelesen 2026-10-27).
  `TU-Note_Violin.zip` (1 605 101 158 B) und `TU-Note_Violin_Package_Insert.pdf` (215 920 B) über die API geladen am
  **2026-10-27** (IC Session 3, Klasse 3 F-9 a), beide gegen die MD5 der Quelle geprüft; Zip außerhalb ausgepackt,
  danach gelöscht.
- **Lizenz:** `dc.rights.uri` „https://creativecommons.org/licenses/by-nd/4.0/“ (nachgelesen 2026-10-27) —
  **nur messen**: Nennung nötig, **keine bearbeitete Fassung weitergeben**; nichts davon in LumiViz ausliefern (F-9 a).
- **Aufnahme (Beipackzettel, `TU-Note_Violin_Package_Insert.pdf`):** schalltoter Raum am SIM Berlin, zwei Tage, eine
  Geigerin; zwei Mikrofone (DPA 4099, Brüel & Kjær 4006, Ordner `DPA/` und `BuK/`). Teile: 336 Einzeltöne in vier
  Stärken, 344 Zweitonfolgen (Übergänge), sechs Tonleitern und sechs Stücke; zu jeder Datei eine Segmentierung (Einsatz,
  stationär, Übergang, Ausklang) unter `Segments/`, Listen unter `File_Lists/`.
- **Inhalt:** 2 080 Dateien — 1 384 WAV unter `WAV/`, 695 Text (Segmente, Listen), der Beipackzettel; zusammen
  2 259 328 060 B.
- **Format geprüft — weicht vom Beipackzettel ab:** Der Zettel nennt für die Aufnahme „96 kHz and a depth of 24 Bit“; die
  gelieferten WAV sind alle mono, **16 bit**, 96 000 Hz.
- **Prüfsummen:** `SHA256SUMS`.
- **Zweck:** Legato, Einschwingen, Übergänge ([aufnahmen.md](../../aufnahmen.md), [messplan.md](../../messplan.md)).
- **Übernahme nach LumiViz:** siehe [UEBERNAHME_LUMIVIZ.md](../../../../UEBERNAHME_LUMIVIZ.md).

## Ablage (A-1 a, Patrik 2026-10-26)

Die Dateien liegen **nicht im Repo**, sondern lokal unter `…\cmake\samples\Klasse 3 - Bogen-Saite\TU-Note_Violin\` (auch für LumiViz erreichbar). Im Repo bleiben diese Datei, `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei) — damit lässt sich jede Datei wiederbeschaffen und prüfen, auch in einer Cloud-Sitzung (Domain der Quelle in der Umgebung freischalten).

# Aufnahmen und Messdaten — Bogen-Saite (Klasse 3)

> **Stand:** 2026-10-26, IC Session 1; Downloads 2026-10-27, IC Session 3 (Abschnitt „Geladen“). Grundlage:
> `recherche/2026-10-26_aufnahmen.md` (Agent). **Kennzeichnung:** **nachgelesen** (Lizenzfeld von der Sitzung selbst gelesen, 2026-10-26) · **nur
> Bericht**.

## Je Instrument

| Instrument | beste gefundene Aufnahme | Lizenz | Lage für die Messung |
|---|---|---|---|
| Violine | Iowa MIS 2012 (reflexionsarm, arco/pizz, pp/mf/ff, je Saite, 24 bit/96 kHz — Violine nur 16 bit, geprüft 2026-10-27) + TinySOL (284 Töne) + TU-Note (vier Stärken, Zweitonfolgen) | Iowa „without restrictions“ (nachgelesen); TinySOL `cc-by-4.0` (nachgelesen); TU-Note CC BY-ND 4.0 (nachgelesen) | **gut** |
| Viola | Iowa MIS 2012 + TinySOL (309 Töne) | wie oben | gut |
| Cello | Iowa MIS 2012 + TinySOL (291 Töne); Bogenmessungen mdw Wien (Lampis, Pàmies-Vilà) | Iowa frei; mdw `cc-by-4.0` (nachgelesen) | **gut**, dazu die besten Bogendaten |
| Kontrabass | Iowa MIS + TinySOL (309 Töne); VSCO 2 CE Solo Contrabass `SusNV`/`SusVib` | Iowa frei; VSCO CC0 (nachgelesen) | gut; einzige freie Trennung mit/ohne Vibrato |
| Ensemble | VSCO 2 CE Sections (Violinen, Viola, Cello: sus mit Vibrato, pizz, spic, trem) | CC0 (nachgelesen) | mittel — zwei Stärken, Raum ungenannt |

## Geladen (2026-10-27, IC Session 3, Freigabe Patrik, F-9 a) — Dateien unter `…\cmake\samples\Klasse 3 - Bogen-Saite\`, im Repo nur Begleitdateien

| Ordner | Inhalt | Größe | Lizenz | Herkunft, Prüfsummen |
|---|---|---|---|---|
| [aufnahmen/Iowa_MIS_Strings2012_stereo2496/](../aufnahmen/Iowa_MIS_Strings2012_stereo2496/HERKUNFT.md) (2026-10-28, F-L5-1 a) | Vl 72, Va 66, Vc 48, Kb 78 AIFF; arco, pizz; pp/mf/ff je Saite; schalltot; **2 Kanäle, zwei Mikrofonorte**, alle 24 bit, 96 kHz (11 Vl × 48 kHz) — geprüft; 17 Vl-pizz doppelt mono | 8 086 826 202 B | „without restrictions“ (keine Standardlizenz) | `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv` |
| [aufnahmen/Iowa_MIS_Strings2012_mono2496/](../aufnahmen/Iowa_MIS_Strings2012_mono2496/HERKUNFT.md) — **unter `samples` gelöscht** 2026-10-28 | war: dieselben 264 Läufe mono; ist **das Mittel (L+R)/2** der Stereofassung (Vl auf 16 bit abgeschnitten) — an allen Paaren geprüft, F-L5-1 a | (3 764 056 512 B) | wie oben | Begleitdateien bleiben als Verweis |
| [aufnahmen/TinySOL_Strings/](../aufnahmen/TinySOL_Strings/HERKUNFT.md) | Vl 284, Va 309, Vc 291, Kb 309 WAV, „ordinario“, mono 16 bit/44,1 kHz; `TinySOL_metadata.csv` | 753 901 478 B | CC BY 4.0 | wie oben |
| [aufnahmen/TU-Note_Violin/](../aufnahmen/TU-Note_Violin/HERKUNFT.md) | Einzeltöne, Zweitonfolgen, Solo; DPA und B&K; Segmentierung; Beipackzettel; WAV mono **16 bit**/96 kHz (Zettel: 24 bit) | 2 259 328 060 B | CC BY-ND 4.0 — **nur messen** | wie oben |
| [aufnahmen/VSCO-2-CE_Strings/](../aufnahmen/VSCO-2-CE_Strings/HERKUNFT.md) | Solo Violin, Solo Contrabass (`SusNV`, `SusVib`, …), Violin/Viola/Cello Section; stereo 44,1 kHz | 744 WAV, 719 932 928 B | CC0 1.0 | wie oben |
| [daten/CNSM_Zenodo18696786/](../daten/CNSM_Zenodo18696786/HERKUNFT.md) | drei Violinen: Stegadmittanz (CSV, 1,5625 Hz), Bewegung (120 Hz) mit Audio, Tonleitern | 462 Dateien, 1 826 065 628 B | CC BY 4.0 | wie oben |
| [daten/Lampis_Zenodo13374477/](../daten/Lampis_Zenodo13374477/HERKUNFT.md) | Stegkraft beim Anstreichen, Cello-G-Saiten A–D, je Bogenkraft und -beschleunigung; mono 64-bit-Gleitkomma, 50 kHz | 37 295 WAV, 5 099 754 336 B | CC BY 4.0 | wie oben |

**Noch nicht geladen:** die Saitenantworten (Zenodo 17749111, kleinster Teil 14 083 546 513 B) — vor dem Laden fragen
(Platz auf C). Übernahme nach LumiViz: [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md).

## Alle Datensätze

| Datensatz | Lizenz am Ort | Inhalt | Größe | Eignung | Übernahme nach LumiViz | Belegkraft |
|---|---|---|---|---|---|---|
| **Iowa MIS** (alt und 2012) | „may be downloaded and used for any projects, without restrictions“ (nachgelesen, Klasse 4) | Vl, Va, Vc, Kb; nur arco und pizz; pp/mf/ff; je Saite; Tonfolgen je Datei (schneiden); reflexionsarm; Vibrato nicht getrennt | 0,16–0,38 GB (alt); bis 1 GB je Zip (2012) | **Bezug** Einzeltöne | frei | Lizenz nachgelesen; Inhalt nur Bericht |
| **TinySOL**, Zenodo 3685367 | `cc-by-4.0` (Zenodo-API, nachgelesen) | Vl 284, Va 309, Vc 291, Kb 309 Töne; nur „ordinario“, ohne Dämpfer; pp/mf/ff; Saite je Ton; Ircam 1996–1999 | 1 027 234 761 B (nachgelesen) | Bezug, Gegenprüfung | Namensnennung | Lizenz und Größe nachgelesen |
| **TU-Note Violin**, depositonce 11303/7527 | `creativecommons.org/licenses/by-nd/4.0` (Seite, nachgelesen) | reflexionsarm, vier Stärken, **Zweitonfolgen** (Übergänge), Stücke, Segmentierung | 1,49 GB (nur Bericht) | Legato, Einschwingen | Namensnennung, **keine Bearbeitung weitergeben** — nur messen | Lizenz nachgelesen |
| **VSCO 2 CE** | `LICENSE` CC0 (nachgelesen, Klasse 4) | Solo Violin (ArcoVib, Pizz, Trem, spic), Solo Contrabass (**SusNV, SusVib**, Pizz, Spic, Trem), Sections | Streicher ≈ 0,7 GB (nur Bericht) | Vibrato Kb, Ensemble | frei | Lizenz nachgelesen |
| **CNSM Dataset**, Zenodo 18696785 | `cc-by-4.0` (nachgelesen, Klasse 0) | drei Violinen: **Stegmobilität** (Laser-Vibrometer, Impulshammer), Bewegung von Geige und Bogen (120 Hz), Audio „in a large dry room“ (nur Bericht); keine Bogenkraft | 654 025 212 B | **Korpus der Violine** (F-7) | Namensnennung | Lizenz nachgelesen |
| **Lampis, mdw Wien**: „A collection of bowed responses of cello strings (part 1)“, Zenodo 17749111 (+ Teile) | `cc-by-4.0` (nachgelesen) | Roboterbogen am Monochord, Cello-G-Saiten, β 0,02–0,2, Bogenkraft 0,1–4 N, vb 0,05/0,1/0,2 m/s, Steg- und Sattelkraft, 50 kHz (nur Bericht) | 43 993 793 651 B Teil 1 (nachgelesen); acht Teile | **Reibmodell** (M2) | Namensnennung | Lizenz und Größe nachgelesen |
| Lampis: Stegkraft beim Anstreichen, Zenodo 13374477 | `cc-by-4.0` (nachgelesen) | Einschwingvorgänge, verschiedene Kräfte und Beschleunigungen | 3 007 564 144 B (nachgelesen) | Einschwingen (M5) | Namensnennung | nachgelesen |
| Lampis: Sensorsteg 10619541; Reibkraft 15879128 | `cc-by-4.0` (nur Bericht) | statische/dynamische Stegkraft; rekonstruierte Reibkraft | 1,3 GB; 47 MB (nur Bericht) | Reibmodell | Namensnennung | nur Bericht |
| **Pàmies-Vilà, mdw Wien**: Bogentechniken am Cello, Zenodo 17207713 | `cc-by-4.0` (nachgelesen) | sechs Cellisten, leere Saiten, Bewegung 240 fps, Audio 50 kHz, u. a. col legno tratto (nur Bericht) | 824 279 394 B (nachgelesen) | Bogenwege, Col legno | Namensnennung | nachgelesen |
| Pàmies-Vilà: Bogenwechsel 17201350; Spieler/Roboter 10696680 | `cc-by-4.0` (nur Bericht) | Bogenwechsel auf leeren Saiten | — | Bogenwechsel (F-4) | Namensnennung | nur Bericht |
| Seven Violin Dataset, Zenodo 3478442 | `cc-by-4.0` (nachgelesen) | sieben Töne F5–G♭6, reflexionsarm | 3 209 234 B | Stichprobe | Namensnennung | nachgelesen |
| URMP (Rochester), Dryad 10.5061/dryad.ng3r749 | „CC0-1.0“ (Dryad-API, nur Bericht) | 44 Kammermusikstücke, Einzelspuren | 12,1 GB | Phrasen, Ensemble | frei | nur Bericht |
| Philharmonia Orchestra Sound Samples | „must not be sold or made available ‘as is’“ (nur Bericht; Seite antwortete der Sitzung nicht) | Streicher-Paket, Inhalt ungeprüft | ? | Gegenprüfung | nur messen, nicht ausliefern | nur Bericht |
| OrchideaSOL, Zenodo 3686252 | Audio unter Ircam-Forum-Lizenz, „utilisation non commerciale“ (nur Bericht) | alle Spielweisen inkl. Sordina, col legno, ponticello | — | Planung | **nicht** | nur Bericht |
| Good-Sounds v1.1, Zenodo 4588740 | Feld `cc-by-4.0`, Beschreibung „Attribution-NonCommercial 4.0“ (nur Bericht) | Vl, Vc mit Klassen für Bogengeschwindigkeit und Kontaktstelle | 13,9 GB | — | **nicht** (NC gilt) | nur Bericht |
| MTG Vibrato Dataset, Zenodo 597285/1016050 | Feld „apgl-v3“, Teil „non commercial“ (nur Bericht) | Violine ohne/langsam/normal/schnell Vibrato | 0,23/0,24 GB | Vibrato | ungeklärt — nicht | nur Bericht |

**Nicht gefunden** (Reichweite im Bericht): frei lizenzierte Einzeltöne mit Sordino, Spiccato, Col legno außer VSCO
(Spic, Trem); Solo-Viola und Solo-Cello in VSCO; Streicher in VCSL; Rohdaten von Demoucron/Schoonderwaldt/Maestre mit
Bogenkraft; ein gespieltes Instrument mit gemessener Bogenkraft **und** Abstrahlschall.

**Freigaben:** [offene-fragen.md](offene-fragen.md) F-9 a; geladen siehe oben.

# Auflösung von InstrumentsCalculation — Plan des Umzugs

> **Version:** 0.1.0 · **Datum:** 2026-11-09 (Session 182)
> **Anlass:** Entscheid Patrik LP2 b (S182): IC wird aufgelöst, LumiViz ist die einzige Quelle für Rechenkerne,
> Vorgaben, Recherche, Messpläne und Quellen (vermerkt in beiden `CLAUDE.md`).
> **Status:** **ausgeführt (S182)** — U0–U7 erledigt; offen nur, dass Patrik das IC-Repo archiviert. Entschieden AU1 a
> (Doku je Klasse unter `<klasse>/ic/`), AU1-1 a (gemeinsame IC-Dokumente nach Thema), AU2 a (Code nach `harvest/`), AU3
> (Hörproben nach `cmake\samples\Hoerproben\`), AU3-1 a (Liste versioniert im Repo, Kopie mit den Klängen am Hörort,
> Wächter auf Gleichheit), HP1 a (Arbeitsordner mit fremdem Material zu den Hörproben)
> **Bezug:** [Instrumente_IC_Konzept.md](../../audio/Instrumente_IC_Konzept.md) §6 (Stufe 34) ·
> [Uebernahme_Journal.md](Uebernahme_Journal.md) · Index der Ablagen [../../archiv/](../../archiv/)

## 1 Grundsätze

1. **Nichts geht verloren, nichts doppelt:** jede Datei aus IC hat genau ein Ziel oder steht in §4 als „entfällt“ mit
   Grund. Das IC-Repo selbst bleibt unverändert liegen, bis Patrik es archiviert (Git macht Patrik).
2. **Fremde Dokumente nie ins Repo:** Paper, Bücher, Noten, Tabs nach `…\cmake\quellen\<Thema>\` (archiviert auf der
   Wechselplatte, Index `docs/archiv/Index_quellen.md`).
3. **Aufnahmen und Messdaten** bleiben unter `…\cmake\samples` (heute `E:\samples`); ins Repo nur die Begleitdateien
   (`HERKUNFT.md`, `LADELISTE.tsv`, `SHA256SUMS`, Muster S169).
4. **Eigene Dokumente werden eingeordnet, nicht umgeschrieben:** Wortlaut bleibt, nur Links werden auf die neuen Pfade
   gerichtet. Was überholt ist, bekommt oben einen Vermerk, statt still zu verschwinden.
5. **Je Schritt prüfen:** Zahl der Dateien und SHA-256 je Datei am Ziel gleich der Quelle, erst dann wird eine Quelle
   außerhalb von IC gelöscht (nur Dateien, die nicht in Git sind — siehe Schritt U1).

## 2 Bestand (aufgenommen 2026-11-09)

| Teil in IC | Umfang | Art |
|---|---|---|
| Wurzel: `AUFGABE.md`, `AUFTRAG_BAU.md`, `BERICHT_BAU.md`, `FRAGEN_KONSOLIDIERT.md`, `KLASSEN.md`, `LEHREN_SETZEN_FLUEGEL.md`, `README.md`, `SCHNITTSTELLE.md`, `STAND.md`, `UEBERNAHME_LUMIVIZ.md`, `VORBILD_FLUEGEL.md` | 11 Dateien | eigene Doku (Auftrag, Stand, Schnittstelle, Lehren) |
| `klassen/Klasse 1–11 …/` | 170 Dateien, 34 MB | je Klasse `README`, `modell-entwurf`, `modell`, `messplan`, `offen`, `offene-fragen`, `quellen.md`, `aufnahmen.md`, `recherche/`; `quellen/` (9 PDF); `aufnahmen/`, `daten/` (nur Begleitdateien) |
| `gemeinsam/` | 108 Dateien | `aufnahmen`, `hoerschwellen`, `koerper`, `lehren-pruefung`, `messkette`, `offene-fragen`, `quellen`, `rohr` (.md), `recherche/` (11), `quellen/` (4 PDF), `cloud-global/` (4), `vorbild-lumiviz/` (80) |
| `pruefungen/` | 2 | Prüfberichte LV an IC |
| `.claude/journal/` | 3 | Journal je Tag (eigene Arbeit) |
| `dld/` | 13 PDF, 32 MB | fremde Dokumente |
| `rechnung/` | 129 Dateien (ohne Bau) | C++20: `gemeinsam/`, `klassen/01…11`, `messung/`, `setzen/`, `werkzeuge/`, `werte/` (Abdrücke), `externals/` (doctest, pocketfft) |
| `hoertest/listen/01…11/` | 22 Dateien + `klaenge/` 237 MB | Hörlisten (`liste.json`, `klaenge.sha256`); die WAVs sind gerechnet, nicht in Git |
| `.vs/`, `rechnung/out*` | — | Bau und IDE |

Übernommen sind bisher (Stufe 34c–34i, Journal J1–J59): Klasse 1 (Flügel, Upright, Honky-tonk, CP-70, Clavinet),
Klasse 4 (Stabspiele), Klasse 5 (Membran), dazu `gemeinsam/` (Balken, Reibung, Membran, Eigenzerlegung, Fft) und die
Messkette. **Nicht übernommen:** Klassen 2, 3, 6–11 (34j/34k planen 10 und 2; 3, 6–9, 11 sind ohne Stufe).

Fremde Dokumente außerhalb von IC, die dieselbe Regel trifft (in LumiViz, durch `.gitignore` nicht in Git):

| Ort in LumiViz | Inhalt |
|---|---|
| `docs/modellierung/hammer-saite/quellen/` | 67 PDF (darunter die vier HAL-Arbeiten zum Pleyel P131) |
| `docs/modellierung/koerper/quellen/` | 5 PDF |
| `docs/modellierung/gezupfte-saite/quellen/`, `wave-clips/quellen/` | je 1 PDF |
| `dld/` | 7 PDF: sechs IMSLP-Scans (Chopin), ein Paper |
| `asset/free_Tabs/` | 20 PDF: Tabs bekannter Songs (urheberrechtlich geschützt) |

## 3 Ziele

| Quelle | Ziel | Bemerkung |
|---|---|---|
| IC `klassen/Klasse N …/*.md`, `recherche/` | `docs/modellierung/<klasse>/ic/` (**AU1 a, Patrik S182**) | Ordner der Klassen wie heute (`hammer-saite`, `gezupfte-saite`, `platte-stab`, `membran`), neu `bogen-saite`, `becken-gong`, `lippe-rohr`, `rohrblatt-rohr`, `luftstrahl-rohr`, `zunge`, `tonrad` |
| IC `klassen/*/aufnahmen/`, `daten/` (Begleitdateien) | `docs/modellierung/<klasse>/aufnahmen/<Quelle>/` | die Daten liegen schon unter `cmake\samples\Klasse N …\<Quelle>` (E:); abgleichen: `Iowa_MIS_Strings2012_mono2496` steht in IC, fehlt im Archiv |
| IC `klassen/*/quellen/`, `gemeinsam/quellen/`, `dld/` | `…\cmake\quellen\<klasse>\` bzw. `…\quellen\gemeinsam\` | fremde Dokumente |
| IC `gemeinsam/koerper.md`, `recherche/…_koerper_abstrahlung.md` | `docs/modellierung/koerper/ic/` (**AU1-1 a, Patrik S182**) | Ordner gibt es schon (KB1 a) |
| IC `gemeinsam/rohr.md`, `recherche/…_rohr_verfahren.md`, `…_moden_und_waveguides_rohr.md`, `…_aufnahmen_blaeser.md` | neuer Themenordner `docs/modellierung/rohr/` (AU1-1 a) | Klassen 7–9; deren README verweisen darauf |
| IC `gemeinsam/messkette.md`, `hoerschwellen.md`, `recherche/…_messkette_verfahren.md` | neuer Themenordner `docs/modellierung/messung/` (AU1-1 a) | alle Klassen; Bibliothek `InstrumentMeasure` |
| IC `recherche/…_aufnahmen_schlagwerk.md`, `…_spielweisen_schlagwerk.md` | `docs/modellierung/membran/ic/recherche/` (AU1-1 a) | Klassen 5–6; `becken-gong` verweist darauf |
| IC `recherche/…_moden_und_waveguides_saite.md` | `docs/modellierung/hammer-saite/ic/recherche/` (AU1-1 a) | Klassen 1–3; `gezupfte-saite`, `bogen-saite` verweisen darauf |
| IC `gemeinsam/lehren-pruefung.md`, `recherche/…_pruefung_lehren_k*.md` (3) | `docs/modellierung/anbindung-ic/ic/` (AU1-1 a) | gehört zum Bau in IC |
| IC `gemeinsam/README.md`, `aufnahmen.md`, `quellen.md`, `offene-fragen.md` | `docs/modellierung/gemeinsam/` (AU1-1 a) | Klasse 0 selbst |
| IC Wurzel (11 Dateien), `pruefungen/`, `.claude/journal/` | `docs/modellierung/anbindung-ic/ic/` | als Geschichte; `STAND.md` und `AUFTRAG_BAU.md` mit Vermerk „aufgelöst S182“ |
| IC `rechnung/` (Quellen, `werte/`) | `harvest/instruments-calculation/rechnung/` (**AU2 a, Patrik S182**) | Vorlage für 34j, 34k und spätere Klassen; nicht gebaut; je übernommene Klasse wird der Teil dort entfernt |
| IC `hoertest/listen/*/liste.json`, `klaenge.sha256` | `hoertest/listen/ic/<klasse>/` | Listen für HörTest_Helper |
| IC `hoertest/listen/<klasse>/` ganz (`liste.json`, `klaenge.sha256`, `klaenge/*.wav`) | `…\cmake\samples\Hoerproben\IC\<klasse>\` (**AU3, Patrik S182**) | gerechnet, 237 MB, die einzige Fassung; Liste und Klänge bleiben beisammen (Konzept §11/§12: Pfade relativ zur `liste.json`) |
| LumiViz `hoertest/listen/<liste>/klaenge/` (stab_34g, membran_34h, klasse1_34i; 96 MB) | `…\samples\Hoerproben\LumiViz\<liste>\` mit Kopie von `liste.json` und `klaenge.sha256` (AU3) | **AU3-1 a:** `liste.json` und `klaenge.sha256` bleiben versioniert unter `hoertest/listen/<liste>/` (IC: `hoertest/listen/ic/<klasse>/`), Antworten unter `hoertest/ergebnisse/`; die schreibenden Fälle legen beide Kopien an, ein Wächter prüft sie auf Gleichheit (meldet nur, wenn die Ablage fehlt) |
| LumiViz `.claude/handover/hoerproben_*` (rund 35 Ordner, 2,6 GB, aus der Zeit vor HörTest_Helper, teils mit `hoertest.json` des Behelfs) | `…\samples\Hoerproben\LumiViz\<ordner>\` unverändert | dazu einzeln prüfen: `salamander_31d*`, `nocturne_31d9`, `lautheit_s139`, `stil_tabs`, `titanium` (gemischt, teils fremde Aufnahmen) |
| HörTest_Helper `out/build/windows-ninja-release-clang/exec/Hoertest/bin/Release/` | `…\samples\Hoerproben\_werkzeug\Hoertest\` | ausführbare Fassung mit Qt-DLLs; Commit von HörTest_Helper in einer `FASSUNG.txt` |
| LumiViz `docs/modellierung/*/quellen/` | `…\cmake\quellen\<klasse>\` | Links in den Berichten anpassen (`quellen/<datei>` → Name und Index) |
| LumiViz `dld/` | `…\cmake\quellen\noten\` (IMSLP) bzw. `…\quellen\<thema>\` (Paper) | |
| LumiViz `asset/free_Tabs/` | `…\cmake\quellen\noten\tabs\` | `.gitignore`-Zeile bleibt als Schutz |

## 4 Entfällt

| Teil | Grund |
|---|---|
| IC `gemeinsam/vorbild-lumiviz/` (80) | Kopie von LumiViz-Code als Vorbild für die Cloud-Sitzung — das Original ist hier |
| IC `gemeinsam/cloud-global/` (4) | Regeln und Werkzeuge der Cloud-Sitzung, die es nicht mehr gibt (Werkzeuge liegen unter `~/.claude/tools`) |
| IC `rechnung/externals/` | doctest und pocketfft sind in LumiViz schon da (pocketfft gepinnt `4d865cb`) |
| IC `.vs/`, `rechnung/out*` | Bau und IDE |

## 5 Reihenfolge

| Schritt | Inhalt | Prüfung |
|---|---|---|
| U0 | Patrik committet den offenen Stand von IC: Session 5 (2026-10-30/31: Hörschwellen, Probe, Glätte, Hörlisten, Prüfbericht S166) ist nie committet, dazu der Prüfbericht S172 (2026-11-02) und der Vermerk der Auflösung (S182) — 27 geänderte, 24 neue Dateien; letzter Commit `bb7e6d6` (2026-10-30) — **erledigt 2026-11-09 (`4409088`, mit dem Abschluss IC Session 6 und der Übergabe `UEBERGABE_LUMIVIZ_HOERTEST.md`)** | `git status` in IC leer; erst danach umziehen, damit die Quelle jedes Umzugs ein Commit ist |
| U1 | Fremde Dokumente: alle PDF aus §2 nach `cmake\quellen\…` kopieren; Index neu (`tools/archiv_index.py`); dann die Kopien in **LumiViz** löschen (nicht in Git) — in IC nichts löschen — **erledigt 2026-11-09** (`tools/quellen_umzug_s182.py`): 127 Quellen, 122 Dateien angekommen (5 Doppel aus IC `dld/`), Liste `cmake\quellen\UMZUG_S182.tsv` (sha256, Ziel, Quelle — Grundlage für U2); 101 Dateien in LumiViz gelöscht, `dld/`, `asset/free_Tabs/`, `modellierung/*/quellen/` leer und entfernt; die `.gitignore`-Zeilen bleiben als Schutz | Zahl und SHA-256 je Datei gleich; Index nennt jede Datei — 122 Dateien im Index |
| U2 | Links in den Berichten unter `docs/modellierung/` von `quellen/<datei>` auf „`cmake\quellen\<klasse>\<datei>` (Index)“ — **erledigt 2026-11-09:** kein echter Markdown-Link zeigte auf `quellen/`, `dld/` oder `free_Tabs/`; die 20 Pfadangaben stehen in Code-Spans fast nur in Berichten unter `recherche/`, die unverändert bleiben (Regel der Modellierung). Gerichtet sind die führenden Dokumente: `modellierung/README.md` (Aufbau, Regel; ein alter Pfad `quellen/<datei>` meint die neue Ablage), die Quellen-Abschnitte in `hammer-saite`, `gezupfte-saite`, `koerper`, `Bibliothek_offen.md` (Ablage nach dem Beschaffen), `Saiten_Konzept.md` (Pfad zu Wright 1996); Erinnerung nachgezogen | keine Links ins Leere (Prüfskript) — 0 Markdown-Links auf alte Orte |
| U3 | IC-Doku (Klassen, gemeinsam, Wurzel, Prüfungen, Journal) nach §3 kopieren, Links richten, Vermerke — **erledigt 2026-11-09** (`tools/ic_doku_umzug_s182.py`, Stand IC `4409088`): 159 Dokumente nach `<ordner>/ic/` (je Klasse, Themenordner `rohr`, `messung`, `gemeinsam`, `koerper`; Wurzel, Prüfungen, Journal nach `anbindung-ic/ic/`; IC `CLAUDE.md` als `IC_CLAUDE.md`, damit sie nicht als Anweisung geladen wird), 257 Links gerichtet; README für zehn neue Ordner, Abschnitt „Aus InstrumentsCalculation“ in sechs vorhandenen, Tabelle in `modellierung/README.md` (mit `membran`, das fehlte); Vermerk „aufgelöst“ oben in `AUFTRAG_BAU.md` | 521 Links aufgelöst, 12 zeigen auf Begleitdateien aus U4, 0 ins Leere; 62 Links in den README, 0 ins Leere |
| U4 | Begleitdateien der Aufnahmen; Abgleich mit dem Archiv (fehlendes `mono2496` klären) — **erledigt 2026-11-09:** 36 Begleitdateien (12 Quellen, je `HERKUNFT.md`, `LADELISTE.tsv`, `SHA256SUMS`) nach `modellierung/<klasse>/aufnahmen|daten/<Quelle>/`; `mono2496` fehlt im Archiv mit Absicht — in IC am 2026-10-28 gelöscht (F-L5-1 a), an allen 264 Dateien das Mittel (L+R)/2 der Stereofassung, die vollständig im Archiv liegt (vermerkt oben in ihrer `HERKUNFT.md`) | 11 Quellen am Archiv `E:\samples` Datei für Datei: 42 861 Dateien, 0 fehlen, 0 abweichend, `SHA256SUMS` gleich; die 12 Links aus U3 lösen auf (533 aufgelöst, 0 ins Leere) |
| U5 | `rechnung/` nach `harvest/instruments-calculation/` (ohne Bau, ohne externals); README dort: was übernommen ist, was nicht — **erledigt 2026-11-09:** 125 Dateien aus IC `4409088` (ohne `externals/`), README mit Stand je Teil (übernommen: Klassen 1, 4, 5, `gemeinsam` Balken/Membran/Reibung/Eigenzerlegung/Fft/Kurve/Modal, `messung`, `setzen`, `stimme_stab`; offen: Klassen 2, 3, 6–11, Rohr, übrige Stimmer), Zeile in `harvest/README.md`; Regel: je übernommene Klasse ihr Teil löschen | 125 Dateien, je SHA-256 gleich; kein `Source.cmake`, keine `Solution.json`, kein Werkzeug liest `harvest/` |
| U6 | Hörproben nach `cmake\samples\Hoerproben\` (AU3): IC je Klasse, LumiViz-Listen, `.claude/handover/hoerproben_*`, `_werkzeug\Hoertest`; ein abgeschlossener Test bekommt eine Datei `abgeschlossen.pass` (Datum, Ergebnis im Repo, Commit) — erst dann darf sein Ordner auf die Wechselplatte — **erledigt 2026-11-09** (`tools/hoerproben_umzug_s182.py`, Liste `Hoerproben\UMZUG_S182.tsv`): 1 830 Dateien (2,9 GB) — die 11 IC-Listen nach `IC\<klasse>\` (Text zusätzlich versioniert nach `hoertest/listen/ic/`), `stab_34g`, `membran_34h`, `klasse1_34i` nach `LumiViz\<liste>\`, die `hoerproben_*` und `salamander_31d3`/`_31d6` aus `.claude/handover/`, `Hoertest.exe` mit `FASSUNG.txt` (HörTest_Helper `446e685a`); danach 1 167 Dateien in LumiViz gelöscht (Klänge der LV-Listen, die Ordner aus dem Handover); AU3-1 a: die drei schreibenden Fälle schreiben am Hörort und legen die Liste ins Repo (`HoerlisteAblage.hpp`), Wächter `*Hoerlisten AU3-1*`; `hoertest/README.md`. Danach HP1 a (Patrik): `stil_tabs` und `titanium` ganz (Nachbildungen geschützter Songs) und die zwei WAVs aus `lautheit_s139/hoerproben/` nach `Hoerproben\LumiViz\<ordner>\` (64 Dateien, 228 MB, in der Liste angehängt); die Arbeitsstände `nocturne_31d9` und der Rest von `lautheit_s139` bleiben im Handover | `--liste-pruefen` je Liste am neuen Ort: 14 von 14 gültig, 0 Fehler (Hinweise = die Befunde von HT, dazu gleiche Abdrücke in `stab_34g` Kalimba und `klasse1_34i` Upright/Honky-tonk); SHA-256 je Datei gleich; Wächter 14 / 14 gleich unter Release-clang und Testing-MSVC; `membran_34h` am Hörort neu geschrieben, die Kopie im Repo blieb Byte für Byte gleich; Index nennt jede Liste mit Zustand |
| U7 | Konzept §6 nachziehen: 34l ohne „Auftrag an IC“; Steckbrief, Memory; Patrik archiviert IC (Git) — **erledigt 2026-11-09** bis auf das Archivieren: `Instrumente_IC_Konzept.md` 0.14.0 (Vermerk im Kopf, Bezug, 34j/34k mit Vorlage unter `harvest/`, 34l ohne Auftrag an IC, TP1 a überholt), `anbindung-ic/README.md`, `CLAUDE.md` (Umzug erledigt), Erinnerungen (S182); **offen bei Patrik: IC archivieren** | — |

Alle Ziele sind entschieden (S182); U1–U7 können der Reihe nach ausgeführt werden, je Schritt mit seiner Prüfung.

## 6 Offene Entscheide

Siehe die Vorlage im Gespräch S182 (AU1 Ablage der IC-Doku, AU2 Ablage des IC-Codes, AU3 die gerechneten Hörproben —
AU3 neu gefasst nach der Übergabe von IC, s. §3).

## 7 Hörtest aus IC

Übergabe: IC `UEBERGABE_LUMIVIZ_HOERTEST.md` (IC Session 6, 2026-11-09; im Sync `IC-20261109-1529-…`). LV übernimmt die
11 Listen und die offenen Punkte: Befund HT 1 (gleiche Abdrücke bei verschiedener Stärke), Befund HT 2 (Klänge abseits
des Pegels; IC misst unter 0,4 s ohne Gate, HT gar nicht), IC-3 (Bezüge auf 48 kHz), L5 (592 Klänge ungehört),
Alphorn A2 (NaN). Die Reihenfolge der Übergabe (§4 dort: erst die Listen richtig, dann hören) gilt als Vorschlag für U6.

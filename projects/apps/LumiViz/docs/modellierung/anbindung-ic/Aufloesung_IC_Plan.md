# Auflösung von InstrumentsCalculation — Plan des Umzugs

> **Version:** 0.1.0 · **Datum:** 2026-11-09 (Session 182)
> **Anlass:** Entscheid Patrik LP2 b (S182): IC wird aufgelöst, LumiViz ist die einzige Quelle für Rechenkerne,
> Vorgaben, Recherche, Messpläne und Quellen (vermerkt in beiden `CLAUDE.md`).
> **Status:** Entwurf — Bestand aufgenommen, Ziele vorgeschlagen; **ausgeführt ist nichts**, offen sind AU1–AU3
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
| IC `klassen/Klasse N …/*.md`, `recherche/` | `docs/modellierung/<klasse>/ic/` (AU1 a) | Ordner der Klassen wie heute (`hammer-saite`, `gezupfte-saite`, `platte-stab`, `membran`), neu `bogen-saite`, `becken-gong`, `lippe-rohr`, `rohrblatt-rohr`, `luftstrahl-rohr`, `zunge`, `tonrad` |
| IC `klassen/*/aufnahmen/`, `daten/` (Begleitdateien) | `docs/modellierung/<klasse>/aufnahmen/<Quelle>/` | die Daten liegen schon unter `cmake\samples\Klasse N …\<Quelle>` (E:); abgleichen: `Iowa_MIS_Strings2012_mono2496` steht in IC, fehlt im Archiv |
| IC `klassen/*/quellen/`, `gemeinsam/quellen/`, `dld/` | `…\cmake\quellen\<klasse>\` bzw. `…\quellen\gemeinsam\` | fremde Dokumente |
| IC `gemeinsam/*.md`, `gemeinsam/recherche/` | `docs/modellierung/gemeinsam/` | Messkette, Hörschwellen, Körper, Rohr, Lehren |
| IC Wurzel (11 Dateien), `pruefungen/`, `.claude/journal/` | `docs/modellierung/anbindung-ic/ic/` | als Geschichte; `STAND.md` und `AUFTRAG_BAU.md` mit Vermerk „aufgelöst S182“ |
| IC `rechnung/` (Quellen, `werte/`) | `harvest/instruments-calculation/rechnung/` (AU2 a) | Vorlage für 34j, 34k und spätere Klassen; nicht gebaut; je übernommene Klasse wird der Teil dort entfernt |
| IC `hoertest/listen/*/liste.json`, `klaenge.sha256` | `hoertest/listen/ic/<klasse>/` | Listen für HörTest_Helper |
| IC `hoertest/listen/*/klaenge/*.wav` | `…\cmake\samples\IC_Hoerproben\<klasse>\` (AU3 a) | gerechnet, 237 MB |
| LumiViz `docs/modellierung/*/quellen/` | `…\cmake\quellen\<klasse>\` | Links in den Berichten anpassen (`quellen/<datei>` → Name und Index) |
| LumiViz `dld/` | `…\cmake\quellen\noten\` (IMSLP) bzw. `…\quellen\<thema>\` (Paper) | |
| LumiViz `asset/free_Tabs/` | `…\cmake\quellen\noten\tabs\` | `.gitignore`-Zeile danach entfernen |

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
| U1 | Fremde Dokumente: alle PDF aus §2 nach `cmake\quellen\…` kopieren; Index neu (`tools/archiv_index.py`); dann die Kopien in **LumiViz** löschen (nicht in Git) — in IC nichts löschen | Zahl und SHA-256 je Datei gleich; Index nennt jede Datei |
| U2 | Links in den Berichten unter `docs/modellierung/` von `quellen/<datei>` auf „`cmake\quellen\<klasse>\<datei>` (Index)“ | keine Links ins Leere (Prüfskript) |
| U3 | IC-Doku (Klassen, gemeinsam, Wurzel, Prüfungen, Journal) nach §3 kopieren, Links richten, Vermerke | Zahl der Dateien; jeder relative Link aufgelöst |
| U4 | Begleitdateien der Aufnahmen; Abgleich mit dem Archiv (fehlendes `mono2496` klären) | SHA256SUMS am Archiv geprüft, sobald eingesteckt |
| U5 | `rechnung/` nach `harvest/instruments-calculation/` (ohne Bau, ohne externals); README dort: was übernommen ist, was nicht | Dateizahl; kein `Source.cmake` zeigt hinein |
| U6 | Hörlisten nach `hoertest/listen/ic/`, WAVs nach `cmake\samples\IC_Hoerproben\` | `Hoertest.exe --liste-pruefen` je Liste |
| U7 | Konzept §6 nachziehen: 34l ohne „Auftrag an IC“; Steckbrief, Memory; Patrik archiviert IC (Git) | — |

U1 und U2 lassen sich sofort ausführen; U3–U6 brauchen AU1–AU3.

## 6 Offene Entscheide

Siehe die Vorlage im Gespräch S182 (AU1 Ablage der IC-Doku, AU2 Ablage des IC-Codes, AU3 die gerechneten Hörproben).

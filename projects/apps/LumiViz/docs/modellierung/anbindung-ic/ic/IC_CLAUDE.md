# InstrumentsCalculation — Projektkontext für Claude

> **Dieses Repo wird aufgelöst (Entscheid Patrik 2026-11-09, LumiViz S182, LP2 b).** Hier wird nichts mehr gebaut,
> gerechnet oder entschieden — auch nicht in einer Cloud-Sitzung; `AUFTRAG_BAU.md` gilt nicht mehr. Rechenkerne,
> Vorgaben, Recherche, Messpläne und Quellen pflegt ab jetzt **LumiViz**
> (`…\Visuals_Project\cmake\LumiViz`, Abschnitt „InstrumentsCalculation wird aufgelöst“ in dessen `CLAUDE.md`).
> Was noch gebraucht wird, zieht nach dem Plan
> `LumiViz/projects/apps/LumiViz/docs/modellierung/anbindung-ic/Aufloesung_IC_Plan.md` um; bis dahin dieses Repo
> nur **lesen**. Fremde Dokumente gehen nach `…\cmake\quellen`, Aufnahmen und Messdaten liegen unter
> `…\cmake\samples` (archiviert auf der Wechselplatte, vor Gebrauch bei Patrik anfragen).

Vorarbeiten für gerechnete Instrumente (Recherche, Modellentwürfe, Messpläne) für LumiViz, nach dem Muster des Flügels.
**Der Auftrag für die Recherche steht in [AUFGABE.md](AUFGABE.md); der Auftrag für den Bau der Rechenkerne (Cloud-Sitzung
ab 2026-10-29) in [AUFTRAG_BAU.md](AUFTRAG_BAU.md) — vor jeder Arbeit lesen.** Stand der Arbeit in `STAND.md`.

**Cloud-Sitzung (keine `~/.claude/CLAUDE.md` vorhanden):** dann gilt gemeinsam/cloud-global/CLAUDE.md (*Kopie fuer die Cloud-Sitzung, entfaellt mit der Aufloesung (S182)*)
wie die globale Regel — **vor jeder Arbeit lesen**; Werkzeuge (`patchlib.py`, `quelle_suchen.py`) von
`gemeinsam/cloud-global/tools/`; Lesart in gemeinsam/cloud-global/README.md (*Kopie fuer die Cloud-Sitzung, entfaellt mit der Aufloesung (S182)*). Lokal
gilt `~/.claude` (G-1 a, Patrik 2026-10-29).

## Projekt-Steckbrief
- **Projekt:** InstrumentsCalculation — Vorab-Recherche je Instrumentklasse (erledigt für 0–11) und seit 2026-10-29
  der Bau der Rechenkerne unter `rechnung/` (AUFTRAG_BAU.md)
- **Session-Prefix:** IC
- **Sprachen:** Deutsch (Dokumente, Kommentare); C++20 ohne Qt unter `rechnung/` (Bezeichner deutsch ohne Umlaute wie
  im Flügel-Vorbild)
- **Konventionen:** Ablage nach `README.md` (Aufbau je Klasse); Belegkraft kennzeichnen; Fragen an Patrik im Format aus
  `AUFGABE.md`
- **Architektur-Doku:** `KLASSEN.md`, `SCHNITTSTELLE.md` (unverbindlich), `VORBILD_FLUEGEL.md`
- **Handover:** `STAND.md`
- **Session-Reports / Journal:** `.claude/journal/JJJJ-MM-TT.md`, eine Datei je Tag (J-1 a, Patrik 2026-10-29): **nach jedem
  Teilschritt** ein datierter Eintrag — was gemacht, Befunde mit Zahlen, Irrwege und warum, Lehren — dann committen und
  pushen, damit beim Zusammenfassen des Kontexts nichts verloren geht
- **Changelog/Logbuch:** —
- **Offene Punkte:** je Klasse `klassen/<klasse>/offene-fragen.md`; gebündelt in `FRAGEN_KONSOLIDIERT.md` (sobald da,
  AUFTRAG_BAU.md §2); je gebauter Klasse `offen.md` (Tabelle des Offenen)
- **Build:** `rechnung/` — CMake, C++20, Linux (Cloud) und Windows: `cmake -S rechnung -B rechnung/out && cmake --build
  rechnung/out` (sobald da)
- **Tests:** doctest unter `rechnung/`, `ctest --test-dir rechnung/out`; Abdrücke je Compiler unter `rechnung/werte/`
- **Sensible Daten:** keine. Repo **privat** auf GitHub; fremde PDFs unter `klassen/<klasse>/quellen/` werden
  **versioniert** (Arbeit auch offline/auf GitHub), Rechtslage je Datei genau in `quellen.md` für eine spätere
  Übernahme nach LumiViz (README.md „Ablage und Lizenzen“); Aufnahmen und Messdaten nach Freigabe **lokal** unter
  `…\cmake\samples\<Klasse n - Name>\`, im Repo nur `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv` (A-1 a, Patrik
  2026-10-26); Klassenordner `klassen/<Klasse n - Name>/`. Downloads von Aufnahmen **nur lokal**, nie in einer
  Cloud-Sitzung; der Ort `samples\` kann auf eine externe Festplatte wandern (Patrik 2026-10-29) — im Code nie fest,
  sondern über `IC_SAMPLES` (AUFTRAG_BAU §0)
- **Bewusste Vereinfachungen:** keine Abstimmung mit LumiViz bis zum Entscheid nach IK1; in der Cloud kein Setzen an
  Aufnahmen (liegen lokal) — nur Grundlagen (L6)
- **Deliverables:** je Klasse die sieben Lieferungen aus `AUFGABE.md`; für den Bau die Lieferungen aus
  `AUFTRAG_BAU.md` §4
- **Domänen-Skills (lokal):** —
- **Bezug:** LumiViz unter `…\Visuals_Project\cmake\LumiViz` — **nur lesen, nichts ändern**; für Sitzungen ohne Zugriff
  die Kopie des Flügel-Vorbilds unter `gemeinsam/vorbild-lumiviz/` (Stand S163, nicht gepflegt, nicht ändern)

# harvest/prozess — Muster für die Prozess-Doku (zweite Achse)

**Quelle:** `NewViz2025/doc/report/`. **Ziel: Phase 2** — Vorlage für `.claude/sessions/` im neuen Setup.

| Datei | Was es ist |
|---|---|
| `Chat-Index.md` | **Projektplan als Chat-Index**: pro Sprint-Tag Titel, ursprünglicher Prompt und Referenz-Dokumente — macht KI-Sessions nachvollziehbar und wiederauffindbar |
| `sprint_01_tag_01_report.md` | Beispiel eines Session-/Tagesreports |
| `commandBus extend.md` | Beispiel einer Erweiterungs-Notiz zwischen Sessions |

## Wiederverwertung

Diese Struktur war der Vorläufer dessen, was jetzt formalisiert wird:

- **Produkt-Doku** → `docs/` (beschreibt das Werk)
- **Prozess-Doku** → `.claude/sessions/` (Session-Reports), `.claude/handover/` (Übergaben),
  Chat-Index-Muster als Session-Verzeichnis-Index
- Session-Reports weiter nach dem Muster `Changelog_Session<N>.md` bzw. Session-ID
  `<Prefix>_Session<N>_<Datum>_<fokus>` (siehe session-start/session-abschluss-Skills).

Weitere Sprint-Reports liegen als docx/pdf im Archiv (`_archive/NewViz2025/doc/report/sprint1/`).

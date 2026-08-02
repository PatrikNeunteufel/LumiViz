# Changelog — Session 63 (2026-08-01/02)

Auftakt der **MilkDrop-Kalibrier-Runde** — und gleich der erste große Fang:
Die Ursache des „viele Presets sind schwarz/monochrom"-Befunds (Patrik) ist
gefunden und gefixt. Dafür wurde **MilkdropRef** gebaut: ein
Ground-Truth-Renderer um den originalen MilkDrop3-Kern, das MilkDrop-Pendant
zu AvsRef. Ergebnis der Fix-Runde: **10 Presets geheilt, 0 Regressionen**
(OK-Quote der 311 Pack-Presets: 279 → 289). Tests **491/491**.

## Neu für Benutzer

- **MilkDrop-Presets erben wieder ihr Bild:** Beim Durchklicken im
  Import-Browser übernimmt jedes .milk-Preset das Bild des Vorgängers —
  wie im Original, wo der Hauptpuffer beim Wechsel nie gelöscht wird.
  Verstärker-Presets (die nur ererbten Pufferinhalt iterieren, z. B.
  „Fractopia") waren bei uns deshalb dauerhaft schwarz.
- **Kaltstart zündet:** Das allererste Bild startet mit einer
  deterministischen Rausch-Saat statt Schwarz (Gegenstück zum undefinierten
  VRAM-Inhalt, mit dem das Original startet) — reproduzierbar für die
  Prüfstände, unsichtbar bei normalen Presets (Decay frisst die Saat in
  Sekunden).

## Werkzeuge

- **NEU `tools/MilkdropRef/`** (nach AvsRef-Muster, nur lokal): rendert
  .milk-Presets mit dem ORIGINALEN MilkDrop3-Kern (D3D9, BSD-3) headless-
  fähig, mit synthetischem Audio formelgleich zur LumiViz-Seite, BMP +
  Statistik im AvsStandalone-Format. d3dx9 kommt aus dem offiziellen
  Microsoft-NuGet (lokal entpackt, keine System-Installation). Der
  Referenzbaum bleibt unangetastet; `patched/` enthält genau eine Datei
  (bekannte UCRT-Fehlerklassen aus dem AvsRef-Bau).
- **Triage-Kette `asset/calibration/milkdrop/`:** `triage_presets.py`
  (Batch über eine Preset-Sammlung, crash-isoliert, resümierbar, JSONL +
  Screenshots), `make_triage_report.py` (Fehlerklassen SCHWARZ/MONOCHROM/
  SCHWACH/… + Montagen), `compare_ref.py` (LumiViz vs. Referenz, Urteil
  PORT-BUG / PRESET-IST-SO je Preset, Seite-an-Seite-Montagen).
- **MilkdropStandalone:** High-DPI-Fix (Effekt füllte bei Windows-Skalierung
  nur ein Teilrechteck des Fensters; `--size` ist jetzt exakt), neue Flags
  `--silence` und `--dump-shaders`. **AvsStandalone** lädt jetzt auch .milk
  (Host-Pfad batchfähig).

## Befund-Geschichte (Kurzfassung)

1. Triage über alle 311 Pack-Presets: 32 auffällig, **kein einziger**
   Lade- oder GL-Fehler — die Ausfälle sind semantisch.
2. MilkdropRef-Gegenprobe: **29 der 32 sind Port-Bugs** (Referenz lebendig),
   3 sind wirklich so (2× beidseitig schwarz, 1× zeigt die Referenz schwarz
   und wir zu viel). Urteile über zwei Läufe identisch.
3. Ketten-Bisektion am einfachsten Schwarz-Fall (Fractopia) + Warm-Start-
   Beweis: Der Custom-Warp-Shader ist ein reiner Verstärker; die Übersetzung
   war korrekt, aber unsere genullten Feedback-Puffer ließen ihm nichts zum
   Verstärken. Zwei Fixes: Kaltstart-Saat (`seedFeedbackNoise`) +
   Feedback-Erhalt beim Preset-Wechsel im Host
   (`replaceMilkdropPresetInPlace`).

## Offen

22 Presets bleiben auffällig (Hell-Flach-Übersteuerung, Rest-Schwarz,
Dunkel-Flach — andere Wurzeln, Werkzeugweg steht) · Prüfpunkt R239/R239b
(leben jetzt von der Saat, Referenz ist schwarz — Langzeitlauf) ·
myPresets/ (41) ungeprüft · Details: `Offene_Punkte.md` §3.

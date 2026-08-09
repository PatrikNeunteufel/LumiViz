# Changelog — Session 62 (2026-08-01)

Die **Grafikkarten-Auswahl** ist da (Settings → Performance, Änderung startet
die App sofort neu) — abgesichert durch einen kompletten Vorher/Nachher-
Messlauf auf beiden Karten mit dem besten denkbaren Ergebnis: **die
Prüfstände verschieben sich beim GPU-Wechsel nicht.** Dazu ein Befund von
Patrik an den domainWarp-Demos, der einen **neuen Prüfstand** (Chain-Skript-
Lint) und drei Ernter-Fixes nach sich zog. Tests **491/491**.

## Neu für Benutzer

- **Grafikkarten-Auswahl** (Settings → Performance): „Automatic /
  High Performance / Power Saving", dazu die Anzeige der **tatsächlich
  genutzten GPU** (Tooltip: alle erkannten Karten). Die Wahl wird pro
  Anwendung in Windows gespeichert (`UserGpuPreferences` — derselbe
  Mechanismus wie die Windows-Grafikeinstellungen) und **eine Änderung
  startet LumiViz sofort neu**, weil der Eintrag erst beim Prozessstart greift.
  Von Patrik im Klick-Sichttest abgenommen.
- **domainWarp-Vorlagen leben:** „Tintenstrom" kehrt die Driftrichtung auf
  jedem Beat um, „Nebelkammer" pulst mit abklingendem Warp-Stoß — die
  Beispiel-Presets demonstrieren jetzt auch die Skriptfelder.

## Technik

- Neues Kernmodul **`core/GpuPreference`** (Registry lesen/schreiben,
  Token-Logik pur + getestet). Das wirkungslose Alt-Trio
  `gpu.ini`/`GpuSelector`/`NvOptimusEnablement`-Export ist entfernt — die
  Registry ist die einzige Steuerung.
- **§8-Messlauf beidseitig:** Baseline AMD Radeon 610M und Nachmessung
  RTX 4090 — Matrix 41/43 zeilenidentisch, Modul-Sonden 91/91, Feld-Sonden
  identische Urteile, die Bit-Identitäts-Sonden sogar **karten-übergreifend
  SHA256-identisch**. Reports: `out/gpu_baseline_radeon610m/` +
  `out/gpu_after_rtx4090/`.

## Treue-Fixes

- **domain.lvfx/domain2.lvfx: die Beat-Richtungsumkehr war ein stummer
  No-op** (Befund Patrik) — sie schrieb `dx`/`dy`, Variablen, die der
  domainWarp-Vertrag nie liest. Jetzt über das `speed`-Vorzeichen + `oy`-Pan
  (alt: MAE 0,0000 mit/ohne Beats, neu: 0,1817).

## Werkzeuge

- **Neuer Prüfstand Strang G:** `lint_chain_scripts.py` findet stumme
  Skript-Zuweisungen in den Asset-Chains (geschrieben, aber weder vom
  Modul-Vertrag noch im Knoten gelesen). Der Erstlauf deckte **drei
  Vertragslücken im Feld-Ernter** auf (Attractor-`b` pauschal verworfen,
  Movements Feld `code` ungefiltert, Nur-Lese-Ziele wie Texer-II-`x`/`y`
  unsichtbar) — alle behoben, `inventory_docs.json` +9 Variablen, Sonden der
  vier betroffenen Typen regeneriert (0 STUMM). Übrig: 9 tote Variablen in
  importierten Original-Presets (bleiben wörtlich).

## Nächste Session

**MilkDrop-Kalibrierung** (Vorgabe Patrik) — Vergleich über
Statistik/Montagen (§9: GPU-Rendering nicht bit-deterministisch);
MilkdropRef-Sondierung bei Bedarf reaktivieren.

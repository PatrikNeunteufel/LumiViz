# MilkDrop-Referenzläufe — Harnesse und Sonden

Die Skripte in diesem Ordner stellen **MilkdropRef** (originaler MilkDrop3-Kern,
[`tools/MilkdropRef/README.md`](../../../tools/MilkdropRef/README.md)) und
**MilkdropStandalone** (unser Renderpfad,
[`projects/exec/MilkdropStandalone/README.md`](../../../projects/exec/MilkdropStandalone/README.md))
nebeneinander. Methodik, Regeln und Vorbehalte:
`projects/apps/LumiViz/docs/kalibrierung/INDEX.md` (§2.2 MilkDrop-Lauf).

**Voraussetzungen:** beide Exes gebaut — MilkdropRef als Win32-Release
(`tools/MilkdropRef/build/Release/`), der Standalone als Release-clang
(`out/build/windows-ninja-release-clang/exec/MilkdropStandalone/bin/Release/`).
Die Pfade stehen einmal in `compare_milkref.py` (`REF_EXE`, `LUMI_EXE`); die
Sonden importieren sie von dort. Aufrufe vom Repo-Root.

## Fester Bildschirm für alle Läufe

Beide Exes lesen `LUMIVIZ_REF_BILDSCHIRM` (S81) und öffnen ihr Fenster dann
oben links auf diesem Monitor. Die Skripte reichen die Umgebung durch — einmal
setzen genügt:

```bash
LUMIVIZ_REF_BILDSCHIRM=DISPLAY2 python asset/calibration/milkdrop/compare_milkref.py
```

Wert: Gerätename `DISPLAYn` (Ziffer aus Windows „Anzeige → Identifizieren“)
oder Index (0-basiert). Unbekannter Wert → Warnung mit Monitorliste,
Standardort. Einzelheiten in der MilkdropRef-README.

**Keine Konsolenfenster (S81):** Alle Skripte starten die Exes über
`asset/calibration/messlauf.py` (`messlauf.run`) mit `CREATE_NO_WINDOW`. Ohne
das Flag öffnet Windows für jedes Konsolenprogramm ein eigenes Fenster, wenn
das Skript selbst keine Konsole hat (Claude-Shell, geplante Aufgaben) — am
Standardort, also nicht auf dem gewählten Monitor. Neue Skripte starten
Werkzeuge deshalb nur über `messlauf.run`.

## Harnesse

### `compare_milkref.py` — der Vergleichslauf

Ein Prozess je Preset und Seite, mehrere Frame-Marken, Montage
(Referenz | LumiViz | Differenz×4) und Urteil je Preset.

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `presets …` | die 19 aus `asset/presets/milkdrop` | einzelne `.milk`-Dateien |
| `--presets DIR` | — | ganzen Ordner vergleichen |
| `--frames 10,30,120` | 10,30,120 | Frame-Marken (Komma); früh wiegt schwerer |
| `--size WxH` | 640x480 | Rendergröße beider Seiten |
| `--formate LISTE` | — (ein Format, `--size`) | derselbe Vergleich in mehreren Formaten, Abschnitt unten; ersetzt `--size` |
| `--audio-muster klassisch\|musik` | klassisch | synthetisches Signal (beide Seiten gleich) |
| `--silence` | aus | Null-Signal |
| `--saaten LISTE` | 0 | Startwerte des Feedback-Puffers; 0 = saatlos. **Werte ≠ 0 sind ungültig** (S81): MilkdropRef nimmt `--saat` an, speist die Saat aber nicht ein — nur unsere Seite säte, der Lauf meldete STARTABHAENGIG überall. Bis das behoben ist, warnt der Harness |
| `--zufall-lumiviz` | aus | Referenz würfelt wie LumiViz — für Presets mit Zufall Pflicht |
| `--timeout S` | 180 | Sekunden je Lauf |
| `--out DIR` | `out/milkref_compare` | Ausgabe |

#### Formate als Messachse (`--formate`, S81)

Mehrere Frame-Marken trennen Phasenversatz von echtem Fehler — mehrere
**Formate** trennen Fehler im Umgang mit Form und Größe von allen anderen.
Anlass: den Custom-Waves fehlte der Aspect-Faktor des Originals; quadratisch
lag die Abweichung bei 0,003, bei 4:3 bei 0,122. Ein Lauf in einem Format
hätte das nie gezeigt.

```bash
LUMIVIZ_REF_BILDSCHIRM=DISPLAY2 python asset/calibration/milkdrop/compare_milkref.py --formate aspekt,monitor --zufall-lumiviz
```

| Baustein | Formate | Was es aufdeckt |
|---|---|---|
| `aspekt` | 1:1 560×560 · 4:3 640×480 · 16:9 736×416 · 3,4:1 1024×304 · 3:4 hoch 480×640 | Fehler an der **Form**: Aspect-Faktoren (Waves, Shapes, `aspectx/y`, `rad`/`ang`), Hochformat-Zweig (`aspectX < 1`). Alle mit ~gleicher Pixelzahl (~307 000), damit sich nur die Form ändert; Kanten Vielfache von 16 |
| `monitor` | physische Größe des Monitors aus `LUMIVIZ_REF_BILDSCHIRM` (hier 2560×734) | Fehler an der **Größe**: feste Pixelmaße (Versatz, Blur-Radius, Punkt- und Liniengröße), krumme Kanten (der Kern rundet 734 → 736) |
| `monitor:DISPLAYn` | ein bestimmter Monitor | wie oben, anderer Monitor |
| `WxH` | ein festes Format | frei |

Ausgabe: je Format ein Unterordner mit eigenem `report.md` und Montagen, dazu
`formate.md` — je Preset Urteil und MAE der frühesten Marke in jedem Format.
**FORMATABHAENGIG** heißt: in einem Format grün (≤ 0,08), in einem anderen
Befund (> 0,15). Die Spalte „Puffer“ meldet, wo der Kern seinen Puffer anders
groß anlegt als wir.

**Vorbehalte:** Große Formate kosten Zeit (jede Marke ist ein Prozess je
Seite). Der Kern taktet an der Wanduhr — schafft er in großen Formaten keine
60 fps, läuft seine Zeit langsamer, und Unterschiede in späten Marken können
daher rühren. Ein Format, in dem beide Seiten nichts zeigen (`BEIDE-STUMM`,
z. B. weil eine Kurve aus dem Bild gestreckt wird), zählt nicht mit.

### `triage_presets.py` — Stapel über eine Sammlung (nur LumiViz)

`--exe`, `--presets DIR` (`asset/Milkdrop3/presets`), `--out DIR`
(`out/milkdrop_triage`), `--frames N` (240), `--size WxH` (640x480),
`--timeout S` (120), `--limit N` (0 = alle), `--nur-schwarz-zweitpass`.
Resümierbar; danach `make_triage_report.py [--dir DIR]` (Fehlerklassen) und
`compare_ref.py [--dir DIR]` (Problem-Presets gegen Referenz-BMPs).

## Sonden (S80)

Jede Sonde erzeugt ihre Test-Presets selbst und schreibt nach `out/milkref_<name>/`.

| Skript | Frage | Argumente |
|---|---|---|
| `spektrum_wave.py` | Liefern Custom-Waves mit `bSpectrum=1` dieselben `value1/value2`? Zeichnet die Wave sichtbar, misst die Kurvenhöhe je Spalte | `--frames N` (60), `--size WxH` (640x480) |
| `comp_quellen.py` | Welche Bildquelle des Comp-Shaders weicht ab (GetPixel/Blur1/Blur2)? | `[preset.milk]` (Vorgabe `stratospheric turbulences`), `--frames N` (30), `--size WxH` (640x480) |
| `gitter/gitter_sonde.py` | In welcher Reihenfolge rechnen die Gitterpunkte `per_pixel`? | keine; Env `GITTER_FRAMES` (30) |
| `drift/drift_sonde.py` | Steht ein Shape-Blitz ohne Bewegung still? | `[marken]` (3,10,30,60,120) |
| `make_eelprobe_milk.py` | Liefert jede ns-eel2-Funktion denselben Wert? | `<zielordner>`; die Läufe danach je Probe von Hand (Kopfkommentar) |

Die Unterordner `c1/`, `m3/`–`m5/`, `s1/` sind Kalibrier-Sätze früherer
Stufen, je mit eigener README.

## Umgebungsvariablen für Messläufe

Die Exes lesen weitere Variablen (Saat, Zufall, Audio, Trace, Dumps) — die
vollständigen Tabellen stehen in den beiden READMEs oben. Am häufigsten:
`LUMIVIZ_REF_BILDSCHIRM` (beide), `MILKDROPREF_AUDIO_DUMP`/`_SEQ` (Referenz),
`LUMIVIZ_MILKDROP_AUDIO` und `LUMIVIZ_MILKDROP_EEL_RAND` (unsere Seite).

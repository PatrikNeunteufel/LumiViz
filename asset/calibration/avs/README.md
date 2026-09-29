# AVS-Kalibrier-Presets

Minimal-Presets je Befund-Cluster aus `docs/visuals/AVS_Sichttest_Protokoll.md`
(Pendant zu `../milkdrop/`). Jedes Preset existiert doppelt:

- **`.avs`** (binär, Format 0.2) — läuft in LumiViz **und in echtem AVS/Winamp**
  (Seite-an-Seite-Urteile). Erzeugt von `make_calibration_presets.py`
  (deterministisch; Layout-Referenz: `projects/libs/AvsParser`).
- **`.lvfx`** (Zwilling, gleicher Basisname) — die eingefrorene übersetzte Chain
  (`chainToJson` pur), erzeugt von `freeze_lvfx_twins.py`. Das ist der
  **Parser-/Translator-Prüfstand**: `--verify` dumpt jedes `.avs` neu und difft
  gegen den Zwilling. Abweichung = Übersetzung hat sich geändert — gewollt
  (Fix → nach Review `--refreeze`) oder Regression.

## Referenzläufe — Harnesse

Die Harnesse stellen **AvsRef** (originaler vis_avs-Kern,
[`tools/AvsRef/README.md`](../../../tools/AvsRef/README.md)) und
**AvsStandalone** (unser Renderpfad,
[`projects/exec/AvsStandalone/README.md`](../../../projects/exec/AvsStandalone/README.md))
nebeneinander. Methodik: `projects/apps/LumiViz/docs/visuals/AVS_Kalibrier_Methodik.md`.

**Voraussetzungen:** beide Exes gebaut — AvsRef als Win32-Release
(`tools/AvsRef/build/Release/`), der Standalone als Release-clang
(`out/build/windows-ninja-release-clang/exec/AvsStandalone/bin/Release/`).
Die Pfade stehen in `compare_avsref.py` (`REF_EXE`, `LUMI_EXE`);
`run_matrix.py` und `run_module_probes.py` importieren sie von dort.

### Fester Bildschirm für alle Läufe

AvsStandalone liest `LUMIVIZ_REF_BILDSCHIRM` (S81) und öffnet sein Fenster
dann oben links auf diesem Monitor. AvsRef rendert fensterlos und meldet die
Variable nur („ohne Wirkung"). Die Skripte reichen die Umgebung durch —
einmal setzen genügt:

```bash
LUMIVIZ_REF_BILDSCHIRM=DISPLAY2 python asset/calibration/avs/compare_avsref.py
```

Wert: Gerätename `DISPLAYn` (Ziffer aus Windows „Anzeige → Identifizieren“)
oder Index (0-basiert). Unbekannter Wert → Warnung mit Monitorliste,
Standardort. Einzelheiten in der MilkdropRef-README
([`tools/MilkdropRef/README.md`](../../../tools/MilkdropRef/README.md)).

### Start ohne Konsolenfenster

Alle Werkzeugstarts der Skripte hier und unter `../fields/` laufen über
`messlauf.run` ([`../messlauf.py`](../messlauf.py), S81) — `subprocess.run`
mit `CREATE_NO_WINDOW`. Ohne das öffnet Windows für jedes gestartete
Konsolenprogramm (AvsRef, AvsStandalone) ein leeres Konsolenfenster, sobald
das Skript selbst ohne Konsole läuft (Claude-Shell, geplante Aufgabe). Das
Renderfenster des Standalones bleibt sichtbar (Merkregel S44). Neue Skripte:
`messlauf.run` statt `subprocess.run`; wer `compare_avsref as ca` importiert
und dessen `run_ref`/`run_lumi` benutzt, ist schon abgedeckt.

### `compare_avsref.py` — der Vergleichslauf

Je Preset ein AvsRef- und ein AvsStandalone-Prozess, Endbild-Vergleich
(dMean, dMaxLuma, MAE), Montage (Referenz | LumiViz | Differenz×4),
`report.md` im Ausgabeordner.

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `presets …` | alle `s*/**/*.avs` hier | einzelne `.avs`-Dateien |
| `--tests` | aus | zusätzlich `../tests/*.avs` (P-Presets) |
| `--frames N` | 120 | Frames je Preset |
| `--size WxH` | 320x240 | Rendergröße beider Seiten |
| `--formate LISTE` | aus | S81: derselbe Vergleich in mehreren Formaten (ersetzt `--size`), s. u. |
| `--ape-dir DIR` | `…\cmake\VisualsPresets\avs` | APE-Sammlung für AvsRef (leer = keine APEs) |
| `--beat-period N` | 30 | deterministischer Beat auf beiden Seiten; 0 = eigene Detektoren (**nicht reproduzierbar**) |
| `--tick-hz N` | 60 | gettime()-Frame-Uhr für AvsRef; 0 = Wanduhr (bei gettime-Presets nicht vergleichbar) |
| `--audio-muster klassisch\|musik` | klassisch | synthetisches Signal (beide Seiten gleich) |
| `--out DIR` | `out/avsref_compare` | Ausgabe |

**Formate (S81).** `--formate` nimmt Komma-getrennte Bausteine aus
[`../messlauf.py`](../messlauf.py): `aspekt` (1:1, 4:3, 16:9, 3,4:1, 3:4 hoch
bei ~gleicher Pixelzahl — nur die Form ändert sich), `monitor` (physische
Größe des Monitors aus `LUMIVIZ_REF_BILDSCHIRM`), `monitor:DISPLAYn`, `WxH`.
Je Format ein vollständiger Lauf mit `report.md` in `<out>/<WxH>/`, danach
`<out>/formate.md`: Preset × Format mit Urteil und MAE, dazu das
Format-Urteil — **FORMATABHAENGIG**, wenn ein Format grün ist
(MAE ≤ `OK_MAE` 0,03) und ein anderes ein Befund (MAE > `BEFUND_MAE` 0,06),
sonst die Spanne der MAE; Fehlergründe je Format in der Spalte Hinweise.
Ohne `--formate` verhält sich das Skript wie vorher.

```bash
LUMIVIZ_REF_BILDSCHIRM=DISPLAY2 python asset/calibration/avs/compare_avsref.py <presets …> --formate aspekt,monitor
```

- **Kein Render-Scale-Divisor.** Beide Seiten rendern nativ in der
  verlangten Größe; AvsRef kennt keinen Divisor, AvsStandalone bleibt bei
  `--render-scale 1`. Das ist Absicht — die Falle 2.2 der Werkzeug-Wegleitung
  betrifft den Nachbau des *App*-Bildes, nicht den Vergleich mit dem Original.
- **Größen werden geprüft.** Jedes Bild muss genau die verlangte Größe
  haben, sonst `FEHLER` mit Grund statt einer Zahl. Gemessene Grenzen (S81):
  AvsRef verlangt mindestens 64×64 und hat nach oben keine Grenze
  (2560×734 läuft); AvsStandalone kann schmale Fenster nicht — Windows
  verbreitert sie auf die Mindestbreite der Titelleiste (96×64 kam als
  184×64 an, 192×64 geht). 2560×734 auf dem gleich großen DISPLAY2 kommt
  pixelgenau an.
- **Laufzeit.** Bei 120 Frames rund 1,5 s je Preset in 320×240 und rund
  4 s in 2560×734 (beide Seiten zusammen, vier Matrix-Presets, S81); die
  volle Reihe `aspekt,monitor` kostet grob das Sieben- bis Neunfache eines
  320×240-Laufs (Probe: zwei Presets, 60 Frames, `aspekt,monitor` = 21 s).

### Weitere Läufe gegen AvsRef

| Skript | Zweck | Argumente |
|---|---|---|
| `run_matrix.py` | Modul-Matrix (`matrix/`) in **zwei** Größen (320x240 + 740x460) | `[präfixe …]` (Matrix-Ordner, z. B. `29`), `--frames N` (120), `--beat-period N` (20), `--out DIR` (`out/matrix_compare`) |
| `run_module_probes.py` | Modul-Sonden (`probes/`), Urteil über Vordergrund-Pixel statt dMean | `[ordner …]` (Unterordner von `probes/`), `--frames N` (60), `--size WxH` (320x240), `--beat-period N` (30), `--out DIR` (`out/module_probes`) |
| `analyse_colormap.py` | Color-Map-Kennlinie je Renderer messen | keine |
| `bisect_avs.py` | Teil-Presets eines `.avs` erzeugen (kumulativ oder `--solo`), danach durch `compare_avsref.py` | `<preset.avs> <zielordner> [indexpfad] [--solo] [--raster raster.avs]` |

Die Erzeuger (`make_*.py`) und `freeze_lvfx_twins.py` stehen unten unter
„Werkzeuge"; Aufrufe jeweils im Kopfkommentar.

## Werkzeuge

```
python make_calibration_presets.py     # .avs (neu) erzeugen
python freeze_lvfx_twins.py            # fehlende .lvfx-Zwillinge einfrieren
python freeze_lvfx_twins.py --verify   # Prüfstand: Dump vs. Zwilling (GRUEN/ROT)
python freeze_lvfx_twins.py --refreeze # nach beabsichtigten Fixes neu einfrieren
python make_colormap_probes.py         # Color-Map-Sonden (+ Sweep nach out/)
python analyse_colormap.py             # APE-Kennlinie messen und vergleichen
```

**APEs (S49):** `compare_avsref.py` gibt AvsRef per `--ape-dir` eine echte
APE-Sammlung mit (Default: `…\cmake\VisualsPresets\avs`). Ohne das lädt der
Referenzkern **keine** Plugin-DLLs (er zeigt bewusst ins Leere,
`avsref_main.cpp:349-357`) — Presets mit Color Map, Texer & Co. wären dann
unvergleichbar, weil nur *wir* den Effekt rendern. Fehlt die Sammlung, schaltet
`AvsStandalone --no-ape` unsere APE-Knoten ab, damit der Rest vergleichbar
bleibt.

**Reproduzierbarkeit (S52) — die wichtigste Messregel:** `compare_avsref.py` fährt
seit Session 52 mit **`--beat-period 30`** als Vorgabe. Grund: mit dem jeweils
eigenen Beat-Detektor ist **keiner** der beiden Renderer reproduzierbar — beide
hängen an der Wanduhr (`bpm.cpp` ruft `GetTickCount`, unser BeatEstimator ebenso).
Gemessen über vier Läufe, jeder Renderer mit **sich selbst** verglichen:

| | mit eigenem Detektor | mit `--beat-period` |
|---|---|---|
| AvsStandalone (wir) | bis 0,21 MAE | **0,0000** |
| AvsRef, Preset ohne APEs | — | **0,0000** |
| AvsRef, Preset **mit** APE-DLL | 0,055–0,064 | 0,018–0,070 |

Der Rest bei AvsRef sind die **APE-DLLs**: Fremdcode mit eigenem CRT, den wir nicht
seeden können. AvsRef selbst setzt seit S52 nach jedem `__LoadPreset` ein festes
`srand()` — das neutralisiert den `srand(time(0))` aus `r_chanshift.cpp:load_config`,
aber nicht die DLLs. **Presets mit APEs deshalb mehrfach messen**, Rauschgrenze ~0,06.

**Kennlinien messen statt raten:** Für APEs ohne Quelltext liefern die
Color-Map-Sonden das Muster — einfarbige Bilder je Eingangswert (der Eingang
ist damit exakt bekannt, eine gezeichnete Rampe streut über die Rasterung),
Auswertung **je Renderer** aus den Bildpaaren.

Sicht-Sweep (Screenshots + Schwarz-Statistik, Fenster muss sichtbar sein):

```
AvsStandalone asset/calibration/avs/<ordner> --auto --frames 240 --out <dir>
```

Test-Audio passend dazu: `…\cmake\TestAudio\` (WAV = Master; z. B.
`10_stereo_wechsel_LR` für s10, `08_kick_120bpm` für Beat-Effekte).

## Ordner

| Ordner | Befund | Inhalt |
|---|---|---|
| `s2_movement/` | S2 ✅ gefixt (S45) — Regressionsschutz | Kreis-Scope + Zoom (Kontrollfälle) und Rotation (Diskriminator). Korrekt: formstabiler Kreisring; NDC-Bug: verschmierte Ellipse |
| `s3_srm/` | S3 ✅ gefixt (S45) — Regressionsschutz | 01: Frame-Reset → statisches Bild (Bug: Weiß-Drift). 02: Listen-Restore → Subtract-Diagonale schneidet schwarz durchs weiße Band (Bug: Additiv-Leck) |
| `s9_blend/` | S9 gefixt — Regressionsschutz | Je ein Preset pro BLEND_LINE-Modus 0–9: grauer Sinus-BG (REPLACE) + SRM + weiße Diagonale. Erwartung u. a.: 03 MAX = weiße Linie, 05/06 Sub = dunkle Linie, 07 Mul ≈ Linie verschwindet auf Weiß-Anteilen, 10 MIN = Linie nur auf BG sichtbar |
| `s10_superscope/` | S10 gefixt — Regressionsschutz | which_ch-Bitfeld: Kanal L/R/Center × Waveform/Spektrum (Flag-Wert 4). Mit `10_stereo_wechsel_LR.wav`: L-Ton ⇒ nur `*links*` lebt |
| `s7_listen/` | S7 Urteil offen | Struktur-Replikat der „don't make a mess"-Familie: Liste blendIn=Xor, blendOut=50/50, innen Movement-Zoom + OscStar. Für Patriks Seite-an-Seite-Vergleich |

Hinweise: `.avs`-Farben sind COLORREF (0x00BBGGRR) — der Translator tauscht
nach RGB. Presets mit Root-Clear=aus **erben beim Preset-Wechsel im Standalone
den Framebuffer des Vorgängers** (AVS-Verhalten) — für saubere Screenshots
einzeln laden. Screenshot-Namen enthalten seit S45 die Endung
(`…_avs_auto.png` vs. `…_lvfx_auto.png` — vorher überschrieb der Zwilling das
.avs-Bild) und sind RGB ohne Alpha. Offener Seite-an-Seite-Punkt S13: unser
SuperScope zeichnet aspektquadratisch (Kreis bleibt Kreis), echtes AVS
NDC-skaliert (Kreis-Skript = 4:3-Ellipse).

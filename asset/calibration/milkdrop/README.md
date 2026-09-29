# MilkDrop-Referenzläufe — Harnesse und Sonden

Die Skripte in diesem Ordner stellen **MilkdropRef** (originaler MilkDrop3-Kern,
[`tools/MilkdropRef/README.md`](../../../tools/MilkdropRef/README.md)) und
**MilkdropStandalone** (unser Renderpfad,
[`projects/exec/MilkdropStandalone/README.md`](../../../projects/exec/MilkdropStandalone/README.md))
nebeneinander. Methodik, Regeln und Vorbehalte:
`projects/apps/LumiViz/docs/kalibrierung/INDEX.md` (§2.2 MilkDrop-Lauf).

**Python:** Die Mess-Skripte mit dem **normalen** Python starten,
`C:\Users\patri\AppData\Local\Programs\Python\Python312\python.exe` — nicht mit
der Store-Fassung (`…\WindowsApps\…python.exe`, Standard im PATH). Die reicht
die GPU-Wahl nicht an die gestarteten Werkzeuge weiter; der Harness bricht mit
ihr ab.

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
| `--saaten LISTE` | 0,0x5EED63 | Startwerte des Feedback-Puffers, auf beiden Seiten gesetzt; 0 = saatlos. Seit S81 speist MilkdropRef die Saat vor dem ersten Warp ein (vorher einen Frame zu spät). Vorbehalt: die Anzeige der Referenz zeichnet ihren Puffer leicht weich (1 px gestreckt, bilinear) — gesätes Rauschen früher Marken an der Montage beurteilen |
| `--zufall-lumiviz` | aus | Referenz würfelt wie LumiViz — für Presets mit Zufall Pflicht |
| `--karte 4090\|610m` | 4090 | Grafikkarte für Referenz **und** Prüfling (immer beide gleich), Abschnitt unten |
| `--fortsetzen` | aus | abgebrochenen Lauf mit derselben `--out` fortsetzen: Marken mit Bild **und** Begleitdatei (`*_ref.json`/`*_auto.json`, gleiche Karte) werden übernommen |
| `--kerne alle\|ccd1\|ccd2` | alle | Referenz **und** Prüfling auf einen CCD beschränken, Abschnitt unten |
| `--timeout S` | 180 | Sekunden je Lauf |
| `--out DIR` | `out/milkref_compare` | Ausgabe |

**Jeder Lauf zusätzlich (S81):** hält das Gerät wach (kein Standby, solange
er läuft — Vorsorge), schreibt `gpu_protokoll_<Datum_Zeit>.csv` (je Sekunde Temperatur,
Leistung, Takt, Auslastung, Drosselgründe; dazu CPU-Temperatur Tctl,
CPU-Paketleistung, CPU-Last und GPU-Hotspot aus **LibreHardwareMonitor**, wenn
dessen Webserver läuft — `C:\Users\patri\LibreHardwareMonitor`, als
Administrator starten, Options → Remote Web Server → Run, Port 8085; sonst
bleiben diese Spalten leer. Jede Zeile sofort auf die Platte, überlebt also
einen Absturz) und bremst den Prüfling auf 60 Bilder/s
(`--fps-grenze 60`; sonst bis 240 am 240-Hz-Schirm, Bild bitgleich).
Anlass: harter Absturz mitten im Kalibrierlauf (29.09. 16:21, Kernel-Power 41
ohne Bugcheck, im Betrieb unter Last auf der 4090, kein Standby) — Verdacht
Notabschaltung durch Hitze/Stromversorgung; die 4090 meldet schon bei
geringer Last „SW Thermal Slowdown“.

#### Grafikkarte und Shader-Stufen (`--karte`, S81)

Referenz und Prüfling laufen **immer auf derselben Karte**, Standard ist die
RTX 4090. Beide Exes (`MilkdropRef.exe`, der Release-clang-
`MilkdropStandalone.exe`) stehen dafür dauerhaft auf `GpuPreference=2` in
`HKCU\Software\Microsoft\DirectX\UserGpuPreferences`. `--karte 610m` stellt
beide für den Lauf auf die Radeon 610M und danach zurück.

Jeder Werkzeugstart prüft seine Karte an der Log-Zeile (Referenz
„Adapter:“, Standalone „GL initialisiert: … | Renderer“). Läuft eine Seite auf
der falschen Karte, **bricht der ganze Lauf ab** (Exit 3) — keine Zahlen aus
gemischten Karten. Der Bericht nennt die gesehenen Adapter.

Die Referenz meldet ihre Shader-Stufen (`Shader: karte K, max M, Preset warp
W comp C`; 2 = ps_2_0, 3 = ps_2_x, 4 = ps_3_0). Spalte „Shader Ref“ im
Bericht. **`REF-SHADER`** (kein Urteil) heißt: das Preset verlangt mehr, als der
Kern nutzt oder die Karte kann, ein Shader fehlt — oder die Karte zeichnet
ps_3_0 mit dem vs_1_1 des Kerns nicht (bekannt: Radeon 610M; dort blieben
`Helix`, `Beauty` und die Starfield-edit-Familie schwarz, die 4090 zeichnet
sie).

Die Sonden (`spektrum_wave.py`, `comp_quellen.py`, `gitter_sonde.py`,
`drift_sonde.py`) starten die Referenz über dieselbe Prüfung
(`cm.ref_starten`) und erwarten die Standardkarte 4090.

#### Kernwahl (`--kerne`, S82)

Windows legt die leichte, fast einfädige Last beider Werkzeuge auf die
Vorzugskerne von CCD1; der läuft dann an der Regelgrenze, CCD2 bleibt kühl, und
Tctl meldet den heißeren. `--kerne ccd2` beschränkt den Harness per Affinität
auf CCD2 — die gestarteten Werkzeuge erben sie. Die CCDs kommen aus den
L3-Caches (`messlauf.ccd_masken()`; 7945HX: 0–15 / 16–31). Die Wahl steht in
der Zeile „Karte: … — Kerne: …“; das Protokoll führt `ccd1_C`/`ccd2_C`.

Kontrollmessung S82 (`Lasershow` + `Beauty and the Math`, 4:3, je ~50 s):

| Lauf | Tctl Mittel/Max | CCD1 Mittel | CCD2 Mittel | MAE |
|---|---|---|---|---|
| alle (1) | 98,4 / 100,0 | 92,6 | 70,0 | Referenz |
| **ccd2** | **95,5** / 100,1 | 88,4 | 78,2 | Abstand ≤ 0,002 |
| alle (2) | 99,3 / 101,0 | 95,2 | 73,7 | Abstand ≤ 0,035 (Lasershow f120 saatlos) |

Die Zahlen ändern sich nicht (Abstand kleiner als zwischen zwei gleichen
Läufen); die Entlastung ist **mäßig** (Tctl im Mittel etwa 3–4 °C tiefer): CCD1
bleibt durch den Rest des Systems warm. Vorgabe bleibt `alle`.

#### Formate als Messachse (`--formate`, S81)

Mehrere Frame-Marken trennen Phasenversatz von echtem Fehler — mehrere
**Formate** trennen Fehler im Umgang mit Form und Größe von allen anderen.
Anlass: den Custom-Waves fehlte der Aspect-Faktor des Originals; quadratisch
lag die Abweichung bei 0,003, bei 4:3 bei 0,122. Ein Lauf in einem Format
hätte das nie gezeigt.

```bash
LUMIVIZ_REF_BILDSCHIRM=DISPLAY2 /c/Users/patri/AppData/Local/Programs/Python/Python312/python.exe asset/calibration/milkdrop/compare_milkref.py --formate aspekt,monitor --zufall-lumiviz
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

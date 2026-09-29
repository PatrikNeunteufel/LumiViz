# AvsStandalone — der LumiViz-AVS-Renderpfad ohne App

**Zweck:** Treibt den echten `MultiEffectVisualizer` (`loadAvsFile` →
übersetzte Chain) in einem eigenen GL-3.3-Fenster, ohne Docking, Panels oder
Render-Thread (Session 43). Er ist unsere Seite jedes AVS-Referenzvergleichs;
die Gegenseite ist [`tools/AvsRef`](../../../tools/AvsRef/README.md) (der
originale vis_avs-Kern). Beide binden dasselbe synthetische Audio ein
(`projects/exec/common/SynthAudio.hpp`).

**Exe (Release-clang, wie die Harnesse sie erwarten):**
`out/build/windows-ninja-release-clang/exec/AvsStandalone/bin/Release/AvsStandalone.exe`

Welches Werkzeug wofür, und welche Fallen es gibt: `projects/apps/LumiViz/docs/Werkzeug_Wegleitung.md`.
Die Mess-Skripte, die diese Exe aufrufen: [`asset/calibration/avs/README.md`](../../../asset/calibration/avs/README.md).

## Aufruf

```bash
AvsStandalone [preset.avs|ordner] [--auto] [--frames N] [--out DIR] [--size WxH] [Optionen]
```

Ohne Preset nimmt er den Referenz-Korpus (`../ref/vis_avs/avs/vis_avs/presets`,
vom Exe-Pfad aufwärts gesucht). Ein Ordner liefert alle `.avs`, `.lvfx`,
`.lvfx2` und `.milk` darin (nach Namen sortiert); die Endung entscheidet den
Ladeweg (`.lvfx*` = eingefrorene Chain, `.milk` = MilkDrop-Knoten im Host wie
in der App). `--help` listet alle Optionen mit Kurztext.

**Interaktiv** (ohne `--auto`): ←/→ Preset wechseln · R neu laden ·
S Screenshot · Esc beenden.

**Fenster sichtbar lassen:** Der Stapellauf braucht ein sichtbares Fenster
(Merkregel S44) — die Harnesse lassen es kurz aufblitzen. Mit
`LUMIVIZ_REF_BILDSCHIRM` (unten) blitzt es immer auf demselben Monitor.

## Argumente

### Lauf und Ausgabe

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `preset` | Referenz-Korpus | eine Datei oder ein Ordner (s. oben) |
| `--auto` | aus | Stapel: jedes Preset N Frames, Screenshot `<datei_mit_unterstrich>_auto.png` + Pixel-Statistik + Import-Report, dann Ende. Exit 1, wenn ein Preset nicht lädt |
| `--frames N` | 120 | Frames je Preset (`--auto`) |
| `--out DIR` | `%TEMP%/lumiviz_avs_standalone` | Screenshot-Verzeichnis |
| `--size WxH` | 800x600 | Fenstergröße. **Logische** Pixel: ohne `QT_ENABLE_HIGHDPI_SCALING=0` rendert das Werkzeug auf einem skalierten Monitor größer (150 % → 480×360 für 320x240). Die Harnesse setzen die Variable |
| `--save-every M` | 0 | im `--auto`-Lauf zusätzlich jeden M-ten Frame speichern (`…_f<NNNN>.png`, Zählung wie AvsRef) |
| `--dump` | aus | übersetzte Chain nach dem Laden als JSON ausgeben (Quelle der `.lvfx`-Zwillinge) |
| `--render-scale N` | 1 | Divisor des Import-Render-Scale-Knotens — Gegenstück zur App-Einstellung `import/avsRenderScaleDivisor` (S73); klassische Presets brauchen 2 oder 4, sonst zerfällt das Bild bei großen Fenstern |
| `--edit-nach DATEI` | — | nach der halben Lauflänge die Parameter aus DATEI übernehmen, ohne Runtime-Reset (bildet einen Panel-Edit nach, S55); Struktur muss gleich sein |

### Beat und Audio

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `--beat-period N` | 0 = Detektor | deterministischer Beat alle N Frames — Gegenstück zu AvsRef `--beat-period` |
| `--audio-muster klassisch\|musik` | klassisch | synthetisches Signal; **beide** Muster erzeugt AvsRef identisch. Im Muster `musik` kommt der Beat aus der Beat-Spur der Vorlage (sofern kein `--beat-period`) |
| `--stereo-spektrum` | aus | linken/rechten Spektrumkanal verschieden füllen (für Kanalfelder, S55). **Kein Gegenstück in AvsRef** |
| `--audio-datei PFAD` | — | echte Musik. **Nur für Augenschein — jeder Referenzvergleich damit ist wertlos** (AvsRef erzeugt sein Audio selbst; `compare_avsref.py` sperrt den Schalter) |
| `--audio-start SEK` | 0 | Startversatz in der Datei |
| `--audio-gain F` | 1 | Faktor auf Wellenform und Spektrum der Datei |
| `--audio-stumm` | aus | Datei nicht hörbar ausgeben (im `--auto`-Lauf ohnehin) |
| `--audio-profil-schreiben DATEI` | — | aus `--audio-datei` ein Musik-Profil als C++-Kopf schreiben (Ziel `projects/exec/common/MusikProfil.hpp`), dann Ende — kein Fenster |
| `--audio-profil-dauer SEK` | 20 | Länge des Ausschnitts dafür (auf ganze Schläge gerundet) |

Für Referenzvergleiche taugen nur `--audio-muster` und `--beat-period`: Nur
sie haben ein Gegenstück in AvsRef.

### Messläufe Kamera/Video (S71)

| Argument | Vorgabe | Wirkung |
|---|---|---|
| `--kamera-freigeben` | aus | Kamera für diesen Lauf freigeben (nur Messläufe; ohne bleibt der Standalone gerätefrei) |
| `--feed-teardown-messen` | aus | Dauer von `LiveVideoFeed::herunterfahren()` ausgeben |

**Nicht vorhanden:** `--no-ape`. Die Methode `setNoApe()` und die Ausgabe
„--no-ape: … APE-Knoten abgeschaltet" stehen im Code, der Schalter ist aber
nicht im Parser registriert — ein Aufruf mit `--no-ape` bricht mit „Unknown
option" ab (Stand S81).

## Umgebungsvariablen

| Variable | Wirkung |
|---|---|
| `LUMIVIZ_REF_BILDSCHIRM` | Fenster auf einen **festen Monitor** (S81): `DISPLAY2` (Gerätename, bevorzugt) oder Index `1` (0-basiert). Oben links im Arbeitsbereich; Log `[Standalone] Bildschirm DISPLAY2 (…)`. Unbekannter Wert → Warnung mit Monitorliste, Standardort. Derselbe Wert wie bei MilkdropRef/MilkdropStandalone; Bestimmung des Monitors in [`tools/MilkdropRef/README.md`](../../../tools/MilkdropRef/README.md) |
| `QT_ENABLE_HIGHDPI_SCALING=0` | liest Qt, nicht das Werkzeug: logische = physische Pixel. Anders als der MilkdropStandalone setzt AvsStandalone sie **nicht** selbst — `compare_avsref.py` tut es für seine Läufe |
| `LUMIVIZ_MEDIEN_THREAD=1` | Medien-Pipeline des Video-/Kamera-Feeds auf eigenem Thread (Vorgabe: Main-Thread; Befund S71) |
| `LUMIVIZ_MILKDROP_*` | gelten für `.milk`-Presets (derselbe `MilkdropVisualizer`), Tabelle in [`projects/exec/MilkdropStandalone/README.md`](../MilkdropStandalone/README.md). Achtung: AvsStandalone setzt `LUMIVIZ_MILKDROP_NOSEED` **nicht** selbst — `.milk` läuft hier mit App-Saat |

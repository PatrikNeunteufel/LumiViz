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

## Werkzeuge

```
python make_calibration_presets.py     # .avs (neu) erzeugen
python freeze_lvfx_twins.py            # fehlende .lvfx-Zwillinge einfrieren
python freeze_lvfx_twins.py --verify   # Prüfstand: Dump vs. Zwilling (GRUEN/ROT)
python freeze_lvfx_twins.py --refreeze # nach beabsichtigten Fixes neu einfrieren
```

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

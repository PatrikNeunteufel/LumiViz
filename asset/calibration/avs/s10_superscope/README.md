# s10_superscope — which_ch-Bitfeld (Befund S10, gefixt — Regressionsschutz)

Referenz: r_sscope.cpp:232-240 — Bits 0-1 = Kanal (0 L, 1 R, ≥2 Center),
Flag-Wert 4 = Spektrum statt Waveform als v-Quelle. Jedes Preset zeichnet
`x=2*i-1; y=-v*0.8` (n=200), Clear je Frame.

| Preset | which_ch | Quelle | Farbe |
|---|---|---|---|
| 01_links_wave | 0 | Waveform L | grün |
| 02_rechts_wave | 1 | Waveform R | rot |
| 03_center_wave | 2 | Waveform Center | weiß |
| 04_links_spektrum | 4 | Spektrum L | grün |
| 05_rechts_spektrum | 5 | Spektrum R | rot |
| 06_center_spektrum | 6 | Spektrum Center | weiß |

Prüfung mit TestAudio `10_stereo_wechsel_LR.wav` (2 s nur L 440 Hz, 2 s nur R
660 Hz): L-Phase ⇒ nur `*links*` lebt, R-Phase ⇒ nur `*rechts*`; Center = Mix.
Spektrum-Stille ⇒ v = −1 (Linie am unteren/oberen Rand, S12-Vertrag), NICHT 0.

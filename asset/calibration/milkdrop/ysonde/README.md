# Y-Sonden — liegt oben oben?

Zwei Sonden für die Y-Konvention des MilkDrop-Ports (Offene_Punkte §3,
„Y-Achse war invertiert“, S75; Warp-Sonde nachgeholt S129):

| Datei | schreibt `uv.y` als Rot in … | prüft |
|---|---|---|
| `Y_comp.milk` | den Comp-Shader | das Composite (S75 behoben) |
| `Y_warp.milk` | den Warp-Shader, also den Feedback-Puffer; der Comp-Shader reicht nur durch | ob der Warp-Shader dieselbe Spiegelung braucht |

Ausgewertet wird das Mittel von Rot im oberen und im unteren Zehntel des
Bildes, Referenz gegen LumiViz. Gleich auf beiden Seiten heißt: die Y-Richtung
stimmt. Lauf:

```
python asset/calibration/milkdrop/compare_milkref.py --presets asset/calibration/milkdrop/ysonde --size 640x480 --out out/ysonde
```

Waves, Rahmen und Motion Vectors sind aus (`fWaveAlpha=0`, `ob_a`, `ib_a`,
`mv_a`), `zoom=1`, `warp=0` — im Bild steht nur, was der Shader schreibt.

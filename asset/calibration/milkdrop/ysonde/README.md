# Y-Sonden — liegt oben oben?

Sonden für die Y-Konvention des MilkDrop-Ports (Offene_Punkte §3, „Y-Achse war
invertiert“, S75; Warp-Sonde S129; zerlegt und erweitert S130, Entscheide
WP1 a, WP1-R a, WP2 b).

| Datei | was im Bild steht | prüft |
|---|---|---|
| `Y_comp.milk` | `uv.y` als Rot, geschrieben im Comp-Shader | Wert von `uv.y` im Comp (S75 behoben) |
| `Y_comp_samp.milk` | rotes Quadrat in der Mitte, der Comp liest bei `uv + (0, 0.15)` | Leserichtung im Comp — die Mitte verrät eine Spiegelung NICHT, siehe `Y_shape_custom` |
| `Y_warp.milk` | `uv.y` als Rot, geschrieben im Warp-Shader; der Comp reicht durch und wird als Standard erkannt (MD1-Weg) | Wert von `uv.y` im Warp |
| `Y_warp_orig.milk` | dasselbe mit `uv_orig.y` | Wert von `uv_orig.y` (und damit `ang`) im Warp |
| `Y_warp_custom.milk` | wie `Y_warp`, der Comp `* 0.999` (nicht als Standard erkannt) | Warp und Comp zusammen |
| `Y_warp_samp.milk` | rotes Quadrat in der Mitte, der Warp liest bei `uv + (0, 0.01)`: eine Spur | Leserichtung im Warp (Referenz: die Spur läuft nach oben) |
| `Y_dy.milk` | rotes Quadrat, `per_frame dy=0.01` (Mesh, ohne Shader) | Bewegung über das Mesh (CPU) |
| `Y_shape_md1.milk` | rotes Quadrat bei x 0,3 / y 0,8, der Comp reicht durch (MD1-Weg) | Lage des Puffers über den MD1-Weg |
| `Y_shape_custom.milk` | dasselbe, der Comp `* 0.999` (Custom-Weg) | Lage des Puffers über einen eigenen Comp-Shader |

**Urteil am Bild, nicht an der MAE.** Die Sonden zeichnen wenig; ein
gespiegeltes Quadrat ergibt eine winzige MAE, und `compare_milkref.py` meldet
„OK“. Deshalb danach `ysonde_auswerten.py`: Rot oben/unten für die Verläufe,
Schwerpunkt des Rots für die Quadrate, Referenz gegen LumiViz (Grenze 0,03).

```
python asset/calibration/milkdrop/compare_milkref.py --presets asset/calibration/milkdrop/ysonde --size 640x480 --saaten 0 --out out/ysonde
python asset/calibration/milkdrop/ysonde/ysonde_auswerten.py out/ysonde --size 640x480
```

Den Look bis S129 zum Vergleich: `LUMIVIZ_MILKDROP_WARP_Y=bisher` und
`LUMIVIZ_MILKDROP_COMP_Y=bisher` vor dem Lauf setzen (Felder `warpYOriginal`,
`compYOriginal` des MilkDrop-Knotens).

**Stand S130 vor dem Umbau:** stimmen `Y_comp`, `Y_comp_samp`, `Y_dy`,
`Y_shape_md1`; gespiegelt `Y_warp`, `Y_warp_orig`, `Y_warp_samp` (Spur nach
unten statt oben) und `Y_shape_custom` (Quadrat unten statt oben). Ursache: alle
Texturen (Puffer, Blur, geladene Bilder) liegen in GL-Lage, y nach oben. Der
Warp-Shader bekam `uv` in GL-Lage; der Comp-Shader bekam `uv` in D3D-Lage
(S75), las damit aber direkt in den GL-Texturen. Seit S130 sieht jeder Shader
den Raum der Referenz, und jedes Lesen geht über eine Umrechnung (`lv_koord`
in der Preamble, `TranspileOptions::referenzRaum`).

Waves, Rahmen und Motion Vectors sind aus (`fWaveAlpha=0`, `ob_a`, `ib_a`,
`mv_a`), `zoom=1`, `warp=0` — im Bild steht nur, was die Sonde zeichnet.

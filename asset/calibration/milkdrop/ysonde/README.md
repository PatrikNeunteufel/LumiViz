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
| `R_warp.milk` | `rad/2` als Rot, `ang/2π + 0,5` als Grün, geschrieben im Warp-Shader (S131) | `rad`/`ang` im Warp wie das Mesh der Referenz (plugin.cpp:2289/2293) |
| `R_comp.milk` | `rad` als Rot, `ang/2π` als Grün, geschrieben im Comp-Shader (S131) | `rad`/`ang` im Comp wie `UvToMathSpace` (milkdropfs.cpp:3779) |
| `W_comp.milk` | der Warp schreibt `uv.x` als Rot, der Comp liest bei `bTexWrap=0` an `uv + (0,5, 0)` (S131) | `sampler_main` wiederholt wie in der Referenz: Sägezahn statt rechts voller Hälfte |
| `M_comp_skalar.milk` | der Comp zeigt `mul(uv − 0,5, float2x2(1, 0,5, 0, 1))` als Rot/Grün (S131) | quadratische Matrix aus Skalaren zeilenweise wie HLSL: Rot hängt nur an x |
| `M_comp_zeilen.milk` | dasselbe mit `mul(float2x2(float2, float2), uv − 0,5)` (S131) | quadratische Matrix aus Zeilen-Vektoren: Grün hängt nur an y |
| `N_comp.milk` | der Comp zeigt `sampler_pw_noise_lq` ganzflächig (S131) | Rauschtextur in richtiger Lage — **mit `--zufall-lumiviz` fahren**, sonst würfeln die Seiten verschieden |
| `N_warp.milk` | dasselbe im Warp, der Comp reicht durch (S131) | Rauschtextur im Warp — ebenso mit `--zufall-lumiviz` |

Die drei Sonden aus S131 (Entscheid Patrik S1 a) zeichnen ganzflächig, dort
greift die MAE: mit beiden Schaltern auf „bisher“ `R_comp` 0,193, `W_comp`
0,125, `R_warp` 0,057 — `compare_milkref.py` nennt den letzten noch „OK“,
das Bild zeigt das halbe `rad` aber ganzflächig; Original alle drei 0,001.

Die vier Sonden aus S131 zu Matrizen und Rauschen (Entscheide S2 a, S3 a):
vor den Fixes `M_comp_*` 0,076 — von `compare_milkref.py` „OK“ genannt, im
Bild aber die vertauschte Achse — und `N_comp` unter „Original“ 0,340 (unter
„bisher“ 0,013: der Comp las Rauschen erst seit S130 gespiegelt). Der ganze
Ordner mit `--zufall-lumiviz` gefahren schadet den übrigen Sonden nicht.

**Urteil am Bild, nicht an der MAE.** Die Sonden zeichnen wenig; ein
gespiegeltes Quadrat ergibt eine winzige MAE, und `compare_milkref.py` meldet
„OK“. Deshalb danach `ysonde_auswerten.py`: Rot oben/unten für die Verläufe,
Schwerpunkt des Rots für die Quadrate, Referenz gegen LumiViz (Grenze 0,03).

```
python asset/calibration/milkdrop/compare_milkref.py --presets asset/calibration/milkdrop/ysonde --size 640x480 --saaten 0 --zufall-lumiviz --out out/ysonde
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

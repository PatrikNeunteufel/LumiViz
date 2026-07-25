# s15_y_richtung — AVS-y-Konvention am Skript-Rand (Befund A, Session 46)

AVS-Skripte leben im **Screen-Raum: y+ = unten** (r_sscope.cpp:
`y = (fy*h/2) + h/2`). LumiViz rendert in GL (y+ = oben) — übersetzt wird
genau am Modulrand (SuperscopeModule-Punktausgabe, ScriptGridModule-Sicht
inkl. r-Drehsinn). Vor dem S46-Fix war die gesamte Skript-Welt vertikal
gespiegelt (in sich konsistent, deshalb nie im Solo-Sichttest aufgefallen).

## Erwartungsbilder

| Preset | Erwartung (Referenz = AvsRef) |
|---|---|
| `01_linie_oben.avs` | Statische weiße Linie (Skript-y=−0.8) bei **10 % Höhe, OBEN** (200×100: Zeile ~9–10) |
| `02_dm_schweif_unten.avs` | Dieselbe Linie; DM `y=y-0.3` (rect) lässt den Schweif **nach UNTEN** wachsen (Inhalt wandert Richtung Screen-y+) |

Gespiegelte Ausgabe (Linie unten bzw. Schweif nach oben) = Regression der
Rand-Konvention.

Messtipp: `compare_avsref.py` + Zeilenprofil (`img.mean(axis=1)`) — die
mean/maxLuma-Metriken allein sind spiegelungsinvariant!

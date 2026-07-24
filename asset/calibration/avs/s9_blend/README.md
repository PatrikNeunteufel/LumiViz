# s9_blend — volle BLEND_LINE-Tabelle (Befund S9, gefixt — Regressionsschutz)

Referenz: r_defs.h:267-283 (10 Modi). Aufbau je Preset: Clear je Frame →
grauer Sinus-Scope (Default REPLACE) → SRM Modus m (Breite 2) → weiße Diagonale.

| # | Modus | Erwartetes Bild der Diagonale |
|---|---|---|
| 01 | 0 Replace | weiß, überschreibt BG |
| 02 | 1 Additiv | weiß; auf BG-Kreuzungen sättigt |
| 03 | 2 MAX | weiß (max(BG, Weiß) = Weiß) |
| 04 | 3 50/50 | hellgrau; auf BG etwas heller |
| 05 | 4 Sub dest−src | schwarz (BG − Weiß clampt auf 0) |
| 06 | 5 Sub src−dest | Weiß minus BG → invertierter BG-Verlauf auf der Linie |
| 07 | 6 Mul | Linie ≈ unsichtbar auf Schwarz, BG bleibt auf Kreuzungen |
| 08 | 7 Adjustable (128) | halbtransparent |
| 09 | 8 XOR | weiß auf Schwarz, invertiert auf BG (aktuell Additiv-Fallback — Merkposten) |
| 10 | 9 MIN | unsichtbar auf Schwarz (min(BG, Weiß) = BG) |

Seit dem S3-Fix (Session 45) gelten die Erwartungen streng (BG zeichnet jeden
Frame REPLACE). Sweep-Messwerte (240 Frames, Session 45): maxLuma je Modus —
01/02/03/06/09 = 1,0 · 04 = 0,749 · 08 = 0,753 · 05/07/10 = 0,502; `.avs` und
`.lvfx`-Zwilling identisch.

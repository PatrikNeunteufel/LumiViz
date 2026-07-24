# s2_movement — d/r-Koordinatenraum (Befund S2 — ✅ gefixt Session 45)

Referenz: r_dmove.cpp:324-332 / r_trans.cpp:459-464 — `d` = Pixel-Abstand /
halbe Diagonale, `r` = atan2 über Pixel-Offsets; nur `x`/`y` sind NDC. Quelle
in allen Presets: grüner Kreis-SuperScope (unser Scope zeichnet
aspektquadratisch → echter Kreis; s. Befund **S13**: echtes AVS zeichnet
dieselbe Form als 4:3-Ellipse), Root-Clear aus (Feedback-Trails).

**Methodik:** Reine d-Skalierung (`d=d*k`) ist in NDC- und Pixel-Konvention
IDENTISCH (die Normierung kürzt sich) — 01/02 sind Kontrollfälle. Den S2-Bug
zeigten Rotation (03) und absolute d-Werte.

| Preset | Effekt | Korrekt (Pixel-Raum, seit Fix) | NDC-Bug-Signatur (vor Fix) |
|---|---|---|---|
| 01_dmove_zoom_kreis | Dynamic Movement `d=d*0.95` | konzentrische Kreisringe | identisch — Kontrollfall |
| 02_movement_zoom_kreis | Movement-User-Skript `d=d*0.95` | wie 01 (r_trans-Pfad) | identisch — Kontrollfall |
| 03_dmove_rotation_kreis | Dynamic Movement `r=r+0.05` | dünner, **formstabiler** Kreisring (starre Rotation) | dicker, horizontal verschmierter Ring (Rotation im normierten Quadrat) |

Unit-Gate: `test_ScriptModules.cpp` — „d/r im PIXEL-Raum auf nicht-quadratischer
Fläche (S2)" (200×100: Rotation rechts→oben = v 2,0; absolutes d=0,5 → u 0,559).
Sichtbeleg: Vorher/Nachher-Screenshots Session 45 (03: verschmierte Ellipse →
stabiler Kreis).

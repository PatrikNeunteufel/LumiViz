# s3_srm — Set-Render-Mode-Zustand (Befund S3 — ✅ gefixt Session 45)

Referenz: r_list.cpp:433/440/693-694/744 — das Original setzt
`g_line_blend_mode` je Frame zurück und rettet ihn um Effect Lists (Eintritt:
Reset auf REPLACE, Ende: Restore des äußeren Modus). Frame-Reset gab es bei uns
schon (S44); Session 45 ergänzte Save/Reset/Restore um Listen (und
Host-Gruppen). Root-Clear aus (damit sich Fehler sichtbar aufsummieren).

| Preset | Aufbau | Korrekt (seit Fix) | Bug-Signatur (vor Fix) |
|---|---|---|---|
| 01_frame_reset | Grau-Sinus-Scope · SRM=Additiv · weiße Diagonale | statisches Bild (BG zeichnet jeden Frame REPLACE) | BG erbt Additiv ab Frame 2 → Weiß-Drift |
| 02_liste_restore | SRM=**Subtract** · Liste[SRM=Additiv + Grau-Sinus] · Diagonale NACH der Liste | Band sättigt weiß (Listen-intern additiv), Diagonale zeichnet mit dem **restaurierten Subtract**: schwarzer Schnitt IM Band, außerhalb unsichtbar | Additiv leckt aus der Liste → Diagonale weiß auf Schwarz |

**Screenshot-Falle (Session 45):** Subtract-Modi schrieben früher Alpha-0-Pixel
in den FBO — im PNG-Viewer wirkten die als „weiße" Linien. Gefixt doppelt:
`applyLineBlend` hält den Alpha-Kanal per Separate-Blend auf dst, und
AvsStandalone speichert Screenshots als RGB ohne Alpha.

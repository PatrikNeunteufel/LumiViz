# Kalibrier-Satz s1 — Preset-Sprites (MilkDrop2077-[SPRITEn]-Sektionen)

Sichttest für den Sprite-Port (DrawUserSprites, milkdropfs.cpp:3432-3777).
Import über den Import-Browser (landet als Milkdrop-Node im MultiEffect-Host)
oder direkt im `MilkdropStandalone`. Testbild: `textures/calib.png`
(Orientierungs-Marker = rotes Feld **oben links**).

| Preset | Erwartung |
|---|---|
| `01_sprite_basis` | Ruhige blaue Wave; das Kalibrier-Bild steht STATISCH und AUFRECHT in der Mitte (halbe Größe, Alpha-Blend). Marker oben links → Orientierung stimmt. Bild verzerrt nicht mit dem Fenster (Breiten-Normierung). |
| `02_sprite_puls` | Sprite 1: additiv, kreist um die Mitte, pulsiert mit `bass_att` (Größe) und `sin(time*2)` (Alpha); `SpriteSpeed=1.5` beschleunigt die Bahn. Sprite 2 (Layer 1, ÜBER Sprite 1): Colorkey-Modus — SCHWARZE Pixel des Testbilds sind durchsichtig; dreht langsam um die eigene Mitte. |
| `03_sprite_burn` | `burn=1`: das wandernde Sprite brennt sich in den Feedback-Loop ein — der Warp (zoom+rot) zieht SPUREN hinter dem Sprite her; das Sprite selbst bleibt zusätzlich scharf obenauf. |

**Bekannte PORT-Abweichungen** (dokumentiert in MilkdropPresetState.hpp):
`SpriteSpeed` skaliert die sprite-lokale Zeit (2077 ohne Quell-Referenz —
Annahme); `SpriteLayer` ist reiner Sortier-Schlüssel; der 4:3-Burn-Aspekt der
Referenz entfällt (unser Feedback-Buffer ist fenstergroß).

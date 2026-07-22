# Kalibrier-Presets C1/C2 — transpilierte Shader (exaktes Sollverhalten)

> **Zweck:** Sichttest der Stufen C1 (HLSL→GLSL) und C2 (Noise/Texturen) OHNE
> echte Pack-Presets — jedes Preset hat ein präzises Soll. **Alle 8 laden
> DIALOGFREI** (nur ℹ-Bestätigungen; erscheint ein Hinweis-Dialog, ist das
> selbst ein Befund). Stellschrauben: `prepareCustomShaders`/
> `feedCustomUniforms`/`ensureNoiseTextures` in MilkdropVisualizer.cpp,
> Transpiler in `projects/libs/HlslTranspiler/`.

## 01_warp_drift.milk — Custom-WARP-Shader läuft

**Soll:** weiße Wave-Trails **driften gleichmäßig nach LINKS** und klingen ab
(decay 0.97 steckt im Shader). Kein Preset-Zoom/Warp aktiv.
**Abweichung:** Drift nach rechts → uv-Offset-Vorzeichen/Screen-Achse; kein
Drift → Warp-Programm nicht aktiv (Fallback, `m_customGlError` prüfen).

## 02_comp_invert.milk — Custom-COMP-Shader läuft

**Soll:** Vollbild-**Invertierung**: Hintergrund WEISS, Wave als dunkle Linie
mit hellem Trail-Verlauf. Eindeutigste Ja/Nein-Probe.
**Abweichung:** schwarzer Hintergrund → Comp-Programm nicht aktiv.

## 03_comp_diagonal.milk — uv-Swizzle/Achsen

**Soll:** Die Wave-Linie (ohne Shader horizontal auf ~¾ **Höhe**) erscheint
als **VERTIKALE** Linie auf ~¾ **Breite** (uv.yx-Spiegelung an der Diagonale).
**Abweichung:** Position spiegelverkehrt (¼ statt ¾) → Achsen-Orientierung.

## 04_q_puls.milk — q-Transport per_frame → Shader

**Soll:** Gesamtbild **pulsiert** in der Helligkeit, Periode **2 s**, nie ganz
schwarz (Minimum 20 %).
**Abweichung:** kein Pulsieren → q1-Uniform-Feed (`fv.qVals`); falsche
Periode → time-Uniform.

## 05_noise_lq.milk — Noise-Textur roh (C2)

**Soll:** STATISCHES, dichtes **Bunt-Rauschen** über den ganzen Screen;
Korngröße ≈ Fensterbreite/256 (bilinear weichgezeichnete Körner). Kein Flackern.
**Abweichung:** schwarz → Noise-Textur nicht gebunden.

## 06_noise_mq.milk — Noise-Glättung (C2-Cubic!)

**Soll:** wie 05, aber **deutlich gröbere, WEICHE Wolkenstruktur** (Strukturen
ca. 4× größer, keine harten Körner) — direkte Probe des kubischen
AddNoiseTex-Ports.
**Abweichung:** sieht aus wie 05 (hartes Korn) → Zoom-Glättung (`catmull`/
X-Hauptzeilen-Y-Spalten-Pässe).

## 07_textur.milk — Custom-Textur-Lader + Orientierung (C2)

**Soll:** Das Testbild `textures/calib.png` vollflächig: Farbverlauf (Rot
wächst nach RECHTS, Grün nach UNTEN, Blau konstant), weißes Gitter 8×8,
**ROTES Quadrat OBEN LINKS**. Kein Hinweis-Dialog (ℹ Textur geladen).
**Abweichung:** rotes Quadrat unten links → Bild-V-Flip (`mirrored()` beim
Upload); Grau statt Bild → Suchpfad (`<preset>/textures`).

## 08_funktion_rotation.milk — #define + Funktion + Matrix + Cast (C1)

**Soll:** Das Bild (Wave + Trails) erscheint um **~29° gedreht** um die
Bildmitte; außerhalb geclampte Ränder (fc_main = Clamp) ziehen Streifen.
**Abweichung:** ungedreht → Makro-/Funktions-/mul-Pfad im Transpiler;
Chaos-Muster an den Rändern → Clamp-Sampler (fc) nicht gebunden.

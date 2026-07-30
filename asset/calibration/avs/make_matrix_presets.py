# -*- coding: utf-8 -*-
"""Modul-Matrix (S47): je Builtin-Effekt ein .avs-Testpreset mit Material.

Ziel: flaechendeckender AvsRef-Vergleich ueber ALLE importierbaren Builtins,
IMMER in zwei Groessen (320x240 + 740x460 — kleine Flaechen maskieren
Groessen-Bugs, Merkregel S46). Lauf: run_matrix.py.

Material (deterministisch, ohne rand):
- MAT_STATIC: ClearScreen-Basis (ersetzt jeden Frame) + Farbverlaufs-Spirale
  (per-Point-Farben: fangen Spiegel-/Kanal-Bugs) + Audio-Wave. Fuer Farb-/
  Pixel-/Warp-Effekte — Urteil rein intra-frame.
- MAT_TRAIL: dieselben Renderer OHNE Basis — Akkumulations-Effekte (Fadeout,
  Blitter, Roto, Water ...) zeigen ihre Trails.

Bewusst ausgelassen (README): 10 SVP (externe .svp-DLL), 21 Comment (No-op),
28 Text (GDI- vs. QPainter-Rasterung, ◐), 32 AVI / 34 Picture (externe
Dateien), 33 Custom BPM (zeit- statt frame-basiert).
Erwartete Grob-Urteile trotz Korrektheit: 16 Scatter / 24 Grain / 27 Starfield
/ 8 Moving Particle (rand()-basiert, zwischen Engines nicht bit-stabil).
"""
from pathlib import Path

from avs_preset_lib import (bass_spin, blitter_feedback, blur, brightness,
                            buffer_save, bump, clear_screen, color_clip,
                            color_modifier, colorfade, ddm, dot_fountain,
                            dot_grid, dot_plane, dynamic_movement,
                            dynamic_shift, fadeout, fast_brightness, grain,
                            interferences, interleave, invert, mirror, mosaic,
                            movement_user, moving_particle, onbeat_clear,
                            osc_ring, osc_star, preset, rotating_stars,
                            roto_blitter, scatter, set_render_mode, simple,
                            starfield, superscope, timescope, unique_tone,
                            water, water_bump)

ROOT = Path(__file__).parent
OUT = ROOT / "matrix"

# ---------------------------------------------------------------- Material

BASE = clear_screen(color=0x202020)
SPIRAL = superscope(
    point=("d=i*0.85; r=i*18.85+t*0.2; x=cos(r)*d*0.9; y=sin(r)*d*0.7+0.1;\n"
           "red=i; green=1-i; blue=0.4+0.3*sin(i*12);"),
    frame="t=t+1;",
    init="n=500; t=0;")
WAVE = superscope(point="x=2*i-1; y=v*0.5-0.2;", init="n=288",
                  colors=(0xC0C0C0,))

MAT_STATIC = BASE + SPIRAL + WAVE
MAT_TRAIL = SPIRAL + WAVE


def write(rel: str, data: bytes) -> None:
    path = OUT / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    print(f"  matrix/{rel} ({len(data)} B)")


# ---------------------------------------------------------------- Die Matrix
# (id, ordner, datei, preset-bytes) — ein Eintrag je Builtin-Variante.

print("Erzeuge Modul-Matrix ->", OUT)

# --- Render-Effekte (eigenes Bild, Root-Clear je Frame) -------------------------
write("00_simple/01_default.avs", preset(simple(), clear_every_frame=True))
write("01_dot_plane/01_default.avs", preset(dot_plane(), clear_every_frame=True))
write("02_osc_star/01_default.avs", preset(osc_star(), clear_every_frame=True))
write("07_bass_spin/01_triangles.avs", preset(bass_spin(), clear_every_frame=True))
write("08_moving_particle/01_default.avs",
      preset(moving_particle(), clear_every_frame=True))
write("13_rotating_stars/01_default.avs",
      preset(rotating_stars(), clear_every_frame=True))
write("14_osc_ring/01_default.avs", preset(osc_ring(), clear_every_frame=True))
write("17_dot_grid/01_default.avs", preset(dot_grid(), clear_every_frame=True))
write("19_dot_fountain/01_default.avs",
      preset(dot_fountain(), clear_every_frame=True))
write("27_starfield/01_default.avs", preset(starfield(), clear_every_frame=True))
write("36_superscope/01_spirale_pointfarben.avs",
      preset(SPIRAL, clear_every_frame=True))
write("39_timescope/01_center.avs", preset(timescope()))

# --- Farb-/Pixel-Effekte (statisches Material, Urteil intra-frame) --------------
write("06_blur/01_normal.avs", preset(MAT_STATIC + blur()))
# Der Blur RUNDET, und die Richtung sieht man erst ueber die Frames: AVS rechnet
# in 8-Bit und schneidet jeden Teilterm ab, das Bild klingt also bei jeder
# Anwendung ab; `roundmode` legt einen festen Ausgleich obendrauf (+4/+5/+3 je
# Staerke). Auf statischem Material, das jeden Frame neu gezeichnet wird, ist der
# Unterschied eine Stelle hinter der Anzeige — die Zeile 01 misst mit und ohne
# Ausgleich dasselbe (MAE 0,003, S57 nachgemessen). Erst OHNE Basis
# (`MAT_TRAIL`) akkumuliert die Rundung sichtbar.
#
# Diese zwei Zeilen sind der Grund, warum `roundUp` bis S57 unentdeckt blieb: die
# Matrix hatte keinen Blur mit Trail, und das Feld wurde von KEINEM Renderer
# gelesen. Jetzt bewachen sie beide Richtungen.
write("06_blur/02_trail_rounddown.avs", preset(MAT_TRAIL + blur(roundmode=0)))
write("06_blur/03_trail_roundup.avs", preset(MAT_TRAIL + blur(roundmode=1)))
write("11_colorfade/01_default.avs", preset(MAT_STATIC + colorfade()))
write("12_color_clip/01_below.avs", preset(MAT_STATIC + color_clip()))
write("22_brightness/01_plus800.avs", preset(MAT_STATIC + brightness()))
write("23_interleave/01_4x4.avs", preset(MAT_STATIC + interleave()))
write("24_grain/01_static100.avs", preset(MAT_STATIC + grain()))
write("25_clear_screen/01_gruen_replace.avs",
      preset(MAT_TRAIL + clear_screen(color=0x004020)))
write("37_invert/01_default.avs", preset(MAT_STATIC + invert()))
write("38_unique_tone/01_gruen.avs", preset(MAT_STATIC + unique_tone()))
write("44_fast_brightness/01_mal2.avs", preset(MAT_STATIC + fast_brightness()))
write("45_color_modifier/01_invert_level.avs", preset(
    MAT_STATIC + color_modifier("red=1-red; green=1-green; blue=1-blue;")))

# --- Warp-/Geometrie-Effekte (statisches Material) ------------------------------
write("15_movement/01_user_zoom.avs",
      preset(MAT_STATIC + movement_user("d=d*0.95")))
write("16_scatter/01_default.avs", preset(MAT_STATIC + scatter()))
write("26_mirror/01_links_nach_rechts.avs", preset(MAT_STATIC + mirror()))
write("30_mosaic/01_q20.avs", preset(MAT_STATIC + mosaic()))
write("35_ddm/01_zoom.avs", preset(MAT_STATIC + ddm("d=d*0.9")))
write("41_interferences/01_vier_additiv.avs",
      preset(MAT_STATIC + interferences()))
write("42_dynamic_shift/01_statisch_10_5.avs", preset(
    MAT_STATIC + dynamic_shift(init="x=10; y=5;")))
write("43_dynamic_movement/01_rect_zoom.avs", preset(
    MAT_STATIC + dynamic_movement("x=x*0.9; y=y*0.9;", rectcoords=1)))
write("29_bump/01_licht_fest.avs", preset(MAT_STATIC + bump()))
write("29_bump/02_invert.avs", preset(MAT_STATIC + bump(invert=1)))

# --- Akkumulations-Effekte (Trail-Material, Feedback ueber Frames) --------------
write("03_fadeout/01_len16.avs", preset(MAT_TRAIL + fadeout()))
write("04_blitter_feedback/01_zoom_out.avs",
      preset(MAT_TRAIL + blitter_feedback(scale=100)))
write("05_onbeat_clear/01_rot_jeden_beat.avs",
      preset(MAT_TRAIL + onbeat_clear(color=0x400000)))
write("09_roto_blitter/01_rotation.avs",
      preset(MAT_TRAIL + roto_blitter(zoom_scale=28, rot_dir=40,
                                      zoom_scale2=28)))
write("20_water/01_default.avs", preset(MAT_TRAIL + water()))
write("31_water_bump/01_fester_tropfen.avs", preset(MAT_TRAIL + water_bump()))

# --- Struktur/Steuerung ---------------------------------------------------------
write("18_buffer_save/01_save_restore.avs", preset(
    MAT_STATIC
    + buffer_save(direction=0, which=0)          # Frame -> Buffer 1
    + clear_screen(color=0x000000)               # Bild wegwerfen
    + buffer_save(direction=1, which=0)))        # Buffer 1 -> Frame
write("40_set_render_mode/01_breite5_additiv.avs", preset(
    BASE + set_render_mode(mode=1, width=5) + SPIRAL,
    clear_every_frame=False))

print("Fertig.")

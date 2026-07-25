# -*- coding: utf-8 -*-
"""Erzeugt die binaeren .avs-Kalibrier-Presets (Format "Nullsoft AVS Preset 0.2").

Layout-Referenz: projects/libs/AvsParser (1:1-Transkription der save/load_config
aus ref/vis_avs) — Container: 24-Byte-Signatur, Root-Mode-Byte, dann Eintraege
[int32 id][int32 len][blob]. Die Dateien laufen in LumiViz UND in echtem
AVS/Winamp (Seite-an-Seite-Urteile).

Aufruf: python make_calibration_presets.py   (schreibt in die Unterordner)
"""
from pathlib import Path

# Gemeinsame Bausteine (Container + Effekt-Blobs) — seit S47 in der Lib,
# geteilt mit make_matrix_presets.py. Ausgabe byte-identisch zum alten Stand.
from avs_preset_lib import (dynamic_movement, effect_list, movement_user,
                            osc_star, preset, set_render_mode, superscope)

ROOT = Path(__file__).parent


def write(rel: str, data: bytes) -> None:
    path = ROOT / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    print(f"  {rel} ({len(data)} B)")


# ---------------------------------------------------------------- Bausteine

CIRCLE = superscope(
    point="x=0.5*cos(i*$PI*2); y=0.5*sin(i*$PI*2)",
    init="n=64",
    colors=(0x00FF80,))

BG_WAVES = superscope(
    point="x=2*i-1; y=0.5*sin(i*18.85+t)",
    frame="t=t+0.04",
    init="n=128; t=0",
    colors=(0x808080,))

DIAGONAL = superscope(point="x=2*i-1; y=2*i-1", init="n=2",
                      colors=(0xFFFFFF,))

V_SCOPE_COLORS = {0: 0x40FF40, 1: 0xFF4040, 2: 0xFFFFFF,
                  4: 0x40FF40, 5: 0xFF4040, 6: 0xFFFFFF}


def v_scope(which_ch: int) -> bytes:
    return superscope(point="x=2*i-1; y=-v*0.8", init="n=200",
                      which_ch=which_ch, colors=(V_SCOPE_COLORS[which_ch],))


print("Erzeuge Kalibrier-Presets ->", ROOT)

# --- s2_movement: d/r-Koordinatenraum (Befund S2, r_dmove.cpp:307-329) ----------
# Korrekt (Pixel-Raum): Kreis-Feedback bleibt KREISFOERMIG (800x600).
# NDC-Bug: Ellipsen/Wobble, aspektabhaengig.
write("s2_movement/01_dmove_zoom_kreis.avs", preset(
    CIRCLE, dynamic_movement(point="d=d*0.95")))
write("s2_movement/02_movement_zoom_kreis.avs", preset(
    CIRCLE, movement_user("d=d*0.95")))
write("s2_movement/03_dmove_rotation_kreis.avs", preset(
    CIRCLE, dynamic_movement(point="r=r+0.05")))

# --- s3_srm: SRM-Reset je Frame + Save/Restore um Listen (Befund S3) ------------
# 01: Original setzt den Linien-Blend je Frame zurueck -> BG-Scope zeichnet
#     jeden Frame REPLACE (statisches Bild). Bug: Additiv-Leck -> Weiss-Drift.
write("s3_srm/01_frame_reset.avs", preset(
    BG_WAVES,
    set_render_mode(mode=1),          # 1 = additiv
    DIAGONAL))
# 02: Original rettet den Modus um Listen -> Scope NACH der Liste zeichnet
#     SUBTRACT (vor der Liste gesetzt): schwarze Diagonale in der weissen
#     Flaeche. Bug: Additiv leckt aus der Liste -> Diagonale unsichtbar (weiss
#     auf weiss).
write("s3_srm/02_liste_restore.avs", preset(
    set_render_mode(mode=4),          # 4 = Subtract dest-src (aussen gesetzt)
    effect_list(
        set_render_mode(mode=1) + BG_WAVES,   # additiv NUR fuer die Liste
        blend_in=1, blend_out=1),
    DIAGONAL))

# --- s9_blend: volle BLEND_LINE-Tabelle, ein Preset je Modus (Befund S9) --------
# Struktur: Clear je Frame -> BG-Scope (Grau, Default REPLACE) -> SRM Modus m ->
# weisse Diagonale. Erwartung je Modus siehe README.
S9_NAMES = ["replace", "add", "max", "avg5050", "sub_dest_minus_src",
            "sub_src_minus_dest", "mul", "adjustable", "xor", "min"]
for m, name in enumerate(S9_NAMES):
    write(f"s9_blend/{m + 1:02d}_{name}.avs", preset(
        BG_WAVES, set_render_mode(mode=m), DIAGONAL,
        clear_every_frame=True))

# --- s10_superscope: which_ch-Bitfeld (Befund S10) ------------------------------
# Bits 0-1 Kanal (0 L, 1 R, 2 Center), Flag-Wert 4 = Spektrum statt Waveform.
# Mit TestAudio 10_stereo_wechsel_LR: links-only-Ton -> nur die L-Variante lebt.
for name, ch in [("01_links_wave", 0), ("02_rechts_wave", 1),
                 ("03_center_wave", 2), ("04_links_spektrum", 4),
                 ("05_rechts_spektrum", 5), ("06_center_spektrum", 6)]:
    write(f"s10_superscope/{name}.avs", preset(
        v_scope(ch), clear_every_frame=True))

# --- s15_y_richtung: AVS-y-Konvention am Skript-Rand (Befund A, S46) ------------
# AVS: Skript-y+ = Bildschirm-UNTEN (r_sscope y=(fy*h/2)+h/2). Gate gegen die
# GL-Spiegelung: 01 statische Linie bei y=-0.8 -> liegt bei 10 % Hoehe (OBEN);
# 02 DM y=y-0.3 (rect) -> Schweif der oben gezeichneten Linie waechst nach
# UNTEN. Vor dem S46-Fix renderte LumiViz beide exakt vertikal gespiegelt.
write("s15_y_richtung/01_linie_oben.avs", preset(
    superscope("x=2*i-1; y=-0.8", init="n=100")))
write("s15_y_richtung/02_dm_schweif_unten.avs", preset(
    superscope("x=2*i-1; y=-0.8", init="n=100"),
    dynamic_movement("y=y-0.3", rectcoords=1)))

# --- s7_listen: XOR/50-50-Listen-Familie (Urteil offen, Befund S7) --------------
# Struktur-Replikat des "don't make a mess"-Verursachers: Effect List mit
# blendIn=Xor, blendOut=50/50, innen Movement-Zoom + OscStar.
write("s7_listen/01_xor_5050_liste.avs", preset(
    effect_list(
        movement_user("d=d*0.9") + osc_star(),
        blend_in=9, blend_out=2)))

print("Fertig.")

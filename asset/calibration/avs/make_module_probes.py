# -*- coding: utf-8 -*-
"""Modul-Sonden (S50): jedes Modul EINZELN gegen AvsRef, dann schrittweise
zusammengebaut.

Ergaenzt die Modul-Matrix, die nur Builtins mit DEFAULT-Parametern prueft.
Keiner der fuenf Befunde aus Session 50 lag dort: sie steckten in APEs
(Texer II), in skriptgesteuerten Varianten (linesize je Punkt, n=0) und in der
KOPPLUNG zweier Module (reg-Register, Listen-EEL). Genau diese drei Klassen
deckt diese Datei ab.

Aufbau in Stufen — bewusst so, dass ein Befund IMMER genau einem Modul
zuzuordnen ist:
  1_render/  ein Zeichner auf definiertem Grund, sonst nichts
  2_trans/   ein Transformator auf einem KLAREN Referenzbild, sonst nichts
  3_script/  dasselbe Modul, aber die skriptbaren Groessen variiert
  4_kopplung/ zwei Module, die nur ueber reg/Puffer/Listen-EEL zusammenhaengen

Urteil: NICHT ueber dMean allein. Duenne Vordergruende bewegen den Mittelwert
nicht — in Session 50 meldete die Leiter viermal "OK", waehrend wir sichtbar
nichts zeichneten. run_module_probes.py vergleicht deshalb zusaetzlich die
Zahl gezeichneter Pixel und deren Schwerpunkt.
"""
from pathlib import Path

from avs_preset_lib import (blur, clear_screen, convolution, dynamic_movement,
                            effect_list, effect_list_ex, invert, list_config,
                            movement_user, preset, set_render_mode, superscope,
                            texer2)

ROOT = Path(__file__).parent
OUT = ROOT / "probes"

# --------------------------------------------------------------- Referenzbild
# Deterministisch, ohne rand und ohne Audio: vier Farbfelder + Diagonale.
# Die Farbfelder trennen Kanal- und Spiegelfehler, die Diagonale zeigt Warps.
GRUND = clear_screen(color=0x101010)
FELDER = set_render_mode(0, width=1) + b"".join(
    superscope(
        point=(f"x={x0}+i*0.3; y={y0};"
               f"red={r};green={g};blue={b};linesize=18;"),
        init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)
    for x0, y0, r, g, b in (
        (-0.9, -0.6, 1, 0, 0), (-0.9, -0.2, 0, 1, 0),
        (-0.9, 0.2, 0, 0, 1), (-0.9, 0.6, 1, 1, 1)))
DIAGONALE = superscope(point="x=i*1.8-0.9; y=i*1.6-0.8; red=1;green=1;blue=0;linesize=2;",
                       init="n=40", which_ch=2, colors=(0xFFFFFF,), drawmode=1)
REFBILD = GRUND + FELDER + DIAGONALE


def write(rel: str, data: bytes) -> None:
    p = OUT / rel
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_bytes(data)


def main() -> None:
    # ---------------------------------------------------- 1: Zeichner einzeln
    write("1_render/texer2_default.avs", preset(
        GRUND, texer2(point="x=(i-0.5)*1.6; y=sin(i*6.28)*0.5;", init="n=12")))
    write("1_render/texer2_size.avs", preset(
        GRUND, texer2(point="x=(i-0.5)*1.6; y=0; sizex=0.5+i*3; sizey=0.5+i*3;",
                      init="n=6")))
    write("1_render/texer2_n0.avs", preset(
        GRUND, texer2(point="x=0;y=0;sizex=3;sizey=3;", init="n=0")))
    write("1_render/texer2_skip.avs", preset(
        GRUND, texer2(point="x=(i-0.5)*1.6; y=0; skip=below(i,0.5);", init="n=10")))
    write("1_render/scope_linien.avs", preset(
        GRUND, set_render_mode(0, width=2),
        superscope(point="x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7; red=1;green=1;blue=1;",
                   init="n=64", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("1_render/scope_punkte.avs", preset(
        GRUND, set_render_mode(0, width=2),
        superscope(point="x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7; red=1;green=1;blue=1;",
                   init="n=32", which_ch=2, colors=(0xFFFFFF,), drawmode=0)))

    # -------------------------------------- 2: Transformatoren auf Referenzbild
    write("2_trans/refbild.avs", preset(REFBILD))            # Nullprobe
    write("2_trans/movement_zoom.avs", preset(REFBILD, movement_user("d=d*0.8")))
    write("2_trans/dmove_zoom.avs", preset(
        REFBILD, dynamic_movement(point="d=d*0.8", rectcoords=0, xres=16, yres=16)))
    write("2_trans/blur.avs", preset(REFBILD, blur()))
    write("2_trans/invert.avs", preset(REFBILD, invert()))
    write("2_trans/convolution_id.avs", preset(REFBILD, convolution()))
    write("2_trans/convolution_blur.avs", preset(
        REFBILD, convolution(kernel=[1] * 49, scale=49)))
    write("2_trans/convolution_kante.avs", preset(
        REFBILD, convolution(kernel=[0] * 24 + [8] + [0] * 24, scale=1,
                             edge_mode=1)))

    # ------------------------------------------ 3: skriptbare Groessen variiert
    write("3_script/linesize_keil.avs", preset(
        GRUND, set_render_mode(0, width=1),
        superscope(point="x=i*1.8-0.9;y=0;linesize=1+i*10;red=1;green=1;blue=1;",
                   init="n=40", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("3_script/drawmode_wechsel.avs", preset(
        GRUND, set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=sin(i*9)*0.5;drawmode=below(i,0.5);"
                         "red=1;green=1;blue=1;",
                   init="n=30", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("3_script/skip.avs", preset(
        GRUND, set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=0;skip=band(floor(i*8),1);"
                         "red=1;green=1;blue=1;",
                   init="n=40", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("3_script/n_aus_skript.avs", preset(
        GRUND, set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=i*0.4-0.2;red=1;green=1;blue=1;",
                   init="n=7", which_ch=2, colors=(0xFFFFFF,), drawmode=0)))

    # ------------------------------------------------------------ 4: Kopplung
    # reg: Dynamic Movement rechnet, SuperScope liest — das Muster, an dem in
    # "Mister Santa" der komplette Vordergrund haengt.
    write("4_kopplung/reg_dm_zu_scope.avs", preset(
        GRUND,
        dynamic_movement(point="", init="", frame="reg00=0.5; reg01=-0.4;",
                         rectcoords=1, xres=8, yres=8),
        set_render_mode(0, width=3),
        superscope(point="x=reg00; y=reg01+i*0.8; red=1;green=1;blue=1;",
                   init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    # reg in die Gegenrichtung: Scope schreibt, Texer liest.
    write("4_kopplung/reg_scope_zu_texer.avs", preset(
        GRUND, set_render_mode(0, width=1),
        superscope(point="reg05=0.6; x=-2; y=-2;", init="n=2",
                   which_ch=2, colors=(0xFFFFFF,), drawmode=1),
        texer2(point="x=reg05; y=(i-0.5)*1.2; sizex=2; sizey=2;", init="n=4")))
    # Einmal-Idiom: ausgeschaltete Liste schaltet sich per EEL selbst ein und
    # fuellt einen Puffer, den ein spaeterer Knoten liest.
    write("4_kopplung/liste_einmal.avs", preset(
        GRUND,
        effect_list_ex(list_config(init="lw=-1;lh=-1;",
                                   frame="enabled=bnot(equal(lw,w)*equal(lh,h));"
                                         "lw=w;lh=h;")
                       + clear_screen(color=0xFFFFFF),
                       enabled=False, blend_out=1),
        set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=0;red=1;green=0;blue=0;", init="n=2",
                   which_ch=2, colors=(0xFF0000,), drawmode=1)))
    # Liste mit Ausgabe-Blend Maximum ueber dem Referenzbild.
    write("4_kopplung/liste_blendout_max.avs", preset(
        REFBILD,
        effect_list(set_render_mode(0, width=20)
                    + superscope(point="x=i*1.4-0.7;y=0.4;red=0;green=0.6;blue=0;",
                                 init="n=2", which_ch=2, colors=(0xFFFFFF,),
                                 drawmode=1),
                    clear=True, blend_in=0, blend_out=3)))

    n = len(list(OUT.rglob("*.avs")))
    print(f"{n} Sonden nach {OUT}")


if __name__ == "__main__":
    main()

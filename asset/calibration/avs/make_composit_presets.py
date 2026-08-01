# -*- coding: utf-8 -*-
"""Komposit-Presets (S61): 200 GANZE Presets in asset/composits/, 10 Themen.

Komplexe Kompositionen im Geist von el-vis/UnConeD (Whacko): Buffer-Tunnel,
Feedback-Schleifen, Beat-Physik (ti-Abkling-Idiom), 3D-Projektionen in purem
EEL. Acht Themen als .avs (Original-EEL-konform: above/below statt </>, kein
mod()), zwei als .lvfx (Host-Module: Fraktale, Bloom, DomainWarp ...).

Je Thema ein Unterordner mit Hauptthema und 20 Varianten; die Variation ist
seeded (random.Random(name)) — der Lauf ist reproduzierbar.

Aufruf:  python make_composit_presets.py       # schreibt asset/composits/*
"""
import json
import random
import sys
from pathlib import Path

from avs_preset_lib import (ape, blitter_feedback, blur, buffer_save, bump,
                            clear_screen, colorfade, custom_bpm, convolution,
                            dot_grid, dot_plane, dynamic_movement,
                            dynamic_shift, effect_list, entry, fadeout,
                            fast_brightness, grain, i32, interferences,
                            interleave, ints, mirror, mosaic, movement_user,
                            onbeat_clear, osc_ring, preset, rotating_stars,
                            roto_blitter, starfield, superscope, timescope,
                            unique_tone, water, water_bump)

for _s in (sys.stdout, sys.stderr):
    if hasattr(_s, "reconfigure"):
        _s.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).parent
OUT = (ROOT / "../../composits").resolve()


def swap_rb(c):
    return ((c & 0xFF) << 16) | (c & 0x00FF00) | ((c >> 16) & 0xFF)


def col(c):
    """Palette (0xRRGGBB) -> Datei-COLORREF."""
    return swap_rb(c)


def chanshift(mode=1023, onbeat=1):
    return ape("Channel Shift", ints(mode, onbeat))


# Paletten (RGB) — je Thema eine Grundstimmung, Varianten mischen.
PAL = {
    "glut": [0xFF4000, 0xFFB000, 0xFFE080, 0x802000],
    "eis": [0x40C0FF, 0x80FFFF, 0xC0E8FF, 0x2040A0],
    "gift": [0x40FF40, 0xC0FF20, 0x20A040, 0xE0FF80],
    "neon": [0xFF00A0, 0x00FFD0, 0x8040FF, 0xFFFF40],
    "abgrund": [0x0030A0, 0x0080C0, 0x00F0C0, 0x004060],
    "gold": [0xFFC020, 0xFF8000, 0xFFF0A0, 0xA06000],
    "violett": [0xA040FF, 0xFF40C0, 0x6020C0, 0xE0A0FF],
    "stahl": [0xC0C8D0, 0x8090A0, 0xFFFFFF, 0x506070],
}
PALS = list(PAL.values())


# ---------------------------------------------------------------- Bausteine
# Beat-Physik-Idiom (UnConeD): ti springt am Beat, klingt je Frame ab.

def kamera_frame(sp):
    """Wandernde Kamera: ox/oy als verschraenkte Sinuspfade + Beat-Schub."""
    return (f"t=t+{sp}+ti*0.02;ti=ti*0.86;"
            "ox=0.4*sin(t*0.71)*cos(t*0.343);"
            "oy=0.3*cos(t*0.53)*sin(t*0.271);")


def scope3d_frame(s1, s2):
    """Rotationsmatrix fuer 3D-Scopes (zwei Achsen + Beat-Schub)."""
    return (f"rx=rx+{s1}+ti*0.03;ry=ry+{s2}+ti*0.02;ti=ti*0.88;"
            "cx=cos(rx);sx=sin(rx);cy=cos(ry);sy=sin(ry);")


PROJEKT_3D = (
    "y1=py*cx-pz*sx;z1=py*sx+pz*cx;"
    "x1=px*cy-z1*sy;z2=px*sy+z1*cy;"
    "dd=1.6/(z2+2.4);x=x1*dd;y=y1*dd;")


# ---------------------------------------------------------------- 8 AVS-Themen


def thema_wurmloecher():
    """Buffer-Tunnel: Band-Liste rendert in Slot 0, eine Dynamic Movement
    raytraced daraus den Flug — die Tie-Tunnel-Architektur, frei variiert."""
    out = []
    for k in range(20):
        rng = random.Random(f"wurm{k}")
        pal = PALS[k % len(PALS)]
        sp = round(rng.uniform(0.010, 0.035), 3)
        twist = round(rng.uniform(-0.6, 0.6), 2)
        zoom = round(rng.uniform(0.18, 0.34), 2)
        mos = rng.choice([0, 8, 12, 18])
        # Bandquelle: zwei springende Balken + Ring, Feedback + Mosaik.
        bands = superscope(
            point="x=bx;y=i*2-1;", frame="",
            beat="bx=(rand(180)-90)*0.01;", init="n=2;bx=0;",
            colors=(col(pal[0]), col(pal[1])))
        bands += superscope(
            point="x=i*2-1;y=by;", beat="by=(rand(140)-70)*0.01;",
            init="n=2;by=0;", colors=(col(pal[2]),))
        inner = bands + blitter_feedback(scale=rng.choice([26, 30, 34]),
                                         scale2=30, blend=1)
        inner += chanshift(onbeat=1)
        if mos:
            inner += mosaic(quality=mos, quality2=mos)
        inner += buffer_save(direction=0, which=0)
        quelle = effect_list(inner, clear=False, blend_in=1, blend_out=0)
        tunnel = dynamic_movement(
            point=("a=atan2(y-oy,x-ox);rr=sqrt(sqr(x-ox)+sqr(y-oy));"
                   f"x=a*0.3183+{twist}*rr;"
                   f"y={zoom}/(rr+0.06)-t;"),
            frame=kamera_frame(sp), beat="ti=1;", init="t=0;ti=0;",
            rectcoords=1, wrap=1, buffern=1, xres=24, yres=24)
        post = colorfade(*rng.choice([(6, -4, -8), (-6, 4, 6), (4, 6, -6)]))
        out.append((f"{k+1:02d}_wurmloch", preset(quelle, tunnel, post)))
    return out


def thema_plasmanebel():
    """Organische Nebel: Punktwolken + Water + gekreuzte Sinus-Verwerfung."""
    out = []
    for k in range(20):
        rng = random.Random(f"nebel{k}")
        pal = PALS[(k + 3) % len(PALS)]
        f1 = round(rng.uniform(1.5, 4.0), 2)
        f2 = round(rng.uniform(1.5, 4.0), 2)
        amp = round(rng.uniform(0.02, 0.05), 3)
        wolke = superscope(
            point=("pp=i*6.2832;x=0.7*sin(pp*3+t)*cos(pp+t*0.37)+0.15*v;"
                   "y=0.7*cos(pp*2+t*0.71)*sin(pp*5-t*0.53);"
                   "red=0.5+0.5*sin(pp+t);green=0.5+0.5*sin(pp+t+2.09);"
                   "blue=0.5+0.5*sin(pp+t+4.19);"),
            frame="t=t+0.013+ti*0.05;ti=ti*0.9;", beat="ti=1;",
            init="n=380;t=0;ti=0;", drawmode=0)
        kette = fadeout(fadelen=rng.choice([8, 12, 16])) + wolke
        if rng.random() > 0.4:
            kette += water()
        kette += dynamic_movement(
            point=(f"x=x+{amp}*sin(y*{f1}+t1);"
                   f"y=y+{amp}*sin(x*{f2}+t2);"),
            frame="t1=t1+0.021;t2=t2-0.017;", beat="t1=t1+rand(100)*0.01;",
            init="t1=0;t2=0;", rectcoords=1, xres=20, yres=20)
        kette += colorfade(*rng.choice([(2, 4, -6), (-4, 2, 6), (6, -2, 2)]))
        if k % 5 == 0:
            kette += chanshift(onbeat=1)
        _ = pal
        out.append((f"{k+1:02d}_nebelkammer", preset(kette)))
    return out


def thema_kaleidoskop():
    """Spiegelwelten: asymmetrische Quelle -> Drall -> Mehrfach-Symmetrie."""
    out = []
    for k in range(20):
        rng = random.Random(f"kaleido{k}")
        pal = PALS[(k + 1) % len(PALS)]
        arme = rng.choice([2, 3, 4, 6])
        quelle = superscope(
            point=(f"pp=i*6.2832;r0=0.25+0.45*sin(pp*{arme}+t)+0.15*v;"
                   "aa=pp+t*0.4;x=cos(aa)*r0+0.2;y=sin(aa)*r0*0.8+0.1;"
                   "red=0.6+0.4*sin(t+pp);green=0.5+0.5*cos(t*0.7+pp*2);"
                   "blue=0.7+0.3*sin(t*1.3-pp);"),
            frame="t=t+0.02+ti*0.06;ti=ti*0.85;", beat="ti=1;",
            init=f"n={rng.choice([220, 300, 420])};t=0;ti=0;")
        kette = fadeout(fadelen=rng.choice([10, 14, 20]),
                        color=col(rng.choice([0x000000, 0x000818])))
        kette += quelle
        kette += dynamic_movement(
            point=f"d=d*{round(rng.uniform(0.955, 0.985), 3)};"
                  f"r=r+{round(rng.uniform(0.01, 0.05), 3)};",
            rectcoords=0, xres=16, yres=16)
        kette += interferences(n_points=arme + 1, rotationinc=rng.choice([1, 2, 3]),
                               distance=rng.choice([10, 16, 24]), alpha=110,
                               blend=1, onbeat=1, speed=0.25)
        kette += mirror(mode=rng.choice([5, 10, 12, 15]), onbeat=1, smooth=1,
                        slower=6)
        _ = pal
        out.append((f"{k+1:02d}_facette", preset(kette)))
    return out


def thema_sternenstaub():
    """Weltraum: Sternenfeld + rotierende Ebenen + Beat-Warp."""
    out = []
    for k in range(20):
        rng = random.Random(f"stern{k}")
        pal = PALS[(k + 5) % len(PALS)]
        kette = fadeout(fadelen=rng.choice([16, 24, 32]))
        kette += starfield(color=col(pal[2]),
                           warp_speed=round(rng.uniform(2.0, 8.0), 1),
                           max_stars=rng.choice([250, 400, 600]), blend=1)
        if k % 2 == 0:
            kette += dot_plane(rotvel=rng.choice([-24, -12, 12, 20]),
                               colors=tuple(col(c) for c in pal[:4])
                               + (col(pal[0]),),
                               angle=rng.choice([-30, -18, 24]))
        else:
            kette += rotating_stars(colors=(col(pal[1]), col(pal[3])))
        kette += roto_blitter(zoom_scale=rng.choice([29, 30, 33]),
                              rot_dir=rng.choice([26, 31, 36]),
                              blend=1, subpixel=1)
        if rng.random() > 0.5:
            kette += grain(smax=18, staticgrain=0, blend=1)
        out.append((f"{k+1:02d}_sternfeld", preset(kette)))
    return out


def thema_maschinenraum():
    """Industrie: harte Gitter, Mosaik-Stanzen, Faltungs-Kanten, Takt-Filter."""
    KANTE = [0] * 49
    KANTE[17], KANTE[23], KANTE[25], KANTE[31] = -1, -1, -1, -1
    KANTE[24] = 5
    out = []
    for k in range(20):
        rng = random.Random(f"maschine{k}")
        pal = PAL["stahl"] if k % 3 else PAL["glut"]
        kette = custom_bpm(skip=1, skipval=rng.choice([0, 1, 3]))
        kette += onbeat_clear(color=col(rng.choice(pal)), nf=rng.choice([1, 2, 4]))
        kette += fadeout(fadelen=rng.choice([6, 10, 14]))
        # Kolben: springende Rechteck-Balken im Takt.
        kette += superscope(
            point=("sp=floor(i*8);lok=i*8-sp;"
                   "x=-0.9+sp*0.257+0.02*sin(lok*6.28);"
                   "y=hub*(0.5+0.5*sin(sp*2.1+ph))* (lok*2-1);"),
            frame="ph=ph+0.05;hub=hub*0.9+0.1*zug;",
            beat="zug=0.4+rand(60)*0.01;",
            init="n=160;ph=0;hub=0.3;zug=0.5;",
            colors=(col(pal[0]), col(pal[1])))
        kette += interleave(x=rng.choice([0, 2, 4]), y=rng.choice([1, 2, 3]),
                            color=col(0x000000), blend=0)
        kette += mosaic(quality=rng.choice([100, 100, 40]),
                        quality2=rng.choice([8, 12, 20]), onbeat=1,
                        dur_frames=10)
        if k % 4 == 0:
            kette += convolution(kernel=list(KANTE), scale=1)
        kette += dynamic_shift(
            init="x=0;y=0;", beat="x=rand(9)-4;y=rand(7)-3;",
            frame="x=x*0.7;y=y*0.7;", subpixel=1)
        out.append((f"{k+1:02d}_takthammer", preset(kette)))
    return out


def thema_drahtgeister():
    """3D-Skulpturen aus purem EEL: Torusknoten, Lissajous-Kaefige, Spindeln."""
    out = []
    formen = [
        # Torusknoten (p,q variieren)
        lambda p, q: (f"u=i*6.2832*{p};px=cos(u)*(1+0.38*cos(u*{q}/{p}));"
                      f"py=sin(u)*(1+0.38*cos(u*{q}/{p}));"
                      f"pz=0.5*sin(u*{q}/{p});"),
        # Lissajous-Kaefig
        lambda p, q: (f"u=i*6.2832;px=sin(u*{p}+t);"
                      f"py=sin(u*{q});pz=cos(u*{p+1});"),
        # Spindel (Rotationskoerper mit Hüllkurve)
        lambda p, q: (f"u=i*6.2832*{p};hh=i*2-1;"
                      f"rr=0.55*(1-hh*hh)*(1+0.3*sin(hh*{q * 3}+t));"
                      "px=cos(u)*rr;py=hh;pz=sin(u)*rr;"),
    ]
    for k in range(20):
        rng = random.Random(f"draht{k}")
        pal = PALS[(k + 2) % len(PALS)]
        p, q = rng.choice([(2, 3), (3, 4), (2, 5), (3, 7), (4, 5)])
        basis = formen[k % 3](p, q)
        kette = fadeout(fadelen=rng.choice([8, 12, 18]))
        kette += superscope(
            point=basis + "px=px*0.8;py=py*0.8;pz=pz*0.8;" + PROJEKT_3D +
            ("red=0.5+0.5*sin(u+rx);green=0.5+0.5*sin(u+rx+2.09);"
             "blue=0.5+0.5*sin(u+rx+4.19);"),
            frame=scope3d_frame(round(rng.uniform(0.006, 0.016), 3),
                                round(rng.uniform(0.009, 0.021), 3)) +
            "t=t+0.02;",
            beat="ti=1;", init=f"n={rng.choice([360, 480, 640])};"
                               "rx=0;ry=0;ti=0;t=0;")
        kette += dynamic_movement(
            point=f"d=d*{round(rng.uniform(0.96, 0.99), 3)};"
                  "r=r+0.008;", xres=14, yres=14)
        if k % 5 == 2:
            kette += mirror(mode=12, onbeat=0, smooth=1, slower=4)
        out.append((f"{k+1:02d}_drahtgeist", preset(kette)))
    return out


def thema_tiefsee():
    """Biolumineszenz: treibende Leuchtpartikel, Wasserbrechung, Abgrund."""
    out = []
    for k in range(20):
        rng = random.Random(f"tiefsee{k}")
        pal = PAL["abgrund"] if k % 3 else PAL["gift"]
        kette = fadeout(fadelen=rng.choice([12, 18, 26]),
                        color=col(0x000410))
        # Planktonschwarm: Punkte treiben auf Stroemungsfeldern.
        kette += superscope(
            point=("pp=i*97.41;x=sin(pp+dx+t*0.31)*0.85;"
                   "y=cos(pp*1.7+dy-t*0.23)*0.75;"
                   "hell=0.4+0.6*abs(sin(pp*3+t));"
                   "red=hell*0.2;green=hell;blue=hell*0.8;"),
            frame="t=t+0.012+ti*0.04;ti=ti*0.92;dx=dx+0.003;dy=dy+0.002;",
            beat="ti=1;", init="n=260;t=0;ti=0;dx=0;dy=0;", drawmode=0)
        # Quallenglocke
        kette += superscope(
            point=("u=i*6.2832;rr=0.3+0.1*sin(t*2)+0.05*sin(u*6+t*3);"
                   "x=cos(u)*rr+qx;y=sin(u)*rr*0.7+qy;"),
            frame="t=t+0.03;qx=0.5*sin(t*0.11);qy=0.4*cos(t*0.07);",
            init="n=140;t=0;qx=0;qy=0;",
            colors=(col(pal[2]), col(pal[0])))
        kette += water_bump(density=rng.choice([6, 8]), depth=rng.choice([400, 700]),
                            random_drop=1, drop_radius=rng.choice([30, 45, 60]))
        kette += water()
        kette += colorfade(-4, 2, 6)
        if k % 4 == 1:
            kette += osc_ring(effect=(2 << 2) | (2 << 4), size=12,
                              colors=(col(pal[1]),))
        out.append((f"{k+1:02d}_leuchttiefe", preset(kette)))
    return out


def thema_farbenrausch():
    """Farbzyklen: Swirl-Feedback durch wandernde Paletten gejagt."""
    out = []
    for k in range(20):
        rng = random.Random(f"farbe{k}")
        pal = PALS[k % len(PALS)]
        stops = [(0, 0x000000), (80, pal[0]), (160, pal[1]), (255, pal[2])]
        kette = superscope(
            point=("pp=i*6.2832;r0=0.3+0.4*v+0.2*sin(pp*5+t);"
                   "x=cos(pp+t*0.5)*r0;y=sin(pp+t*0.5)*r0;"),
            frame="t=t+0.025;", init="n=240;t=0;",
            colors=tuple(col(c) for c in pal))
        kette += movement_user(
            script=f"r=r+{round(rng.uniform(0.02, 0.07), 3)};"
                   f"d=d*{round(rng.uniform(0.94, 0.99), 3)};",
            subpixel=1)
        blob = i32(rng.choice([0, 3, 4])) + i32(0) + i32(0)
        blob += bytes([128, 0, 0, 0])
        stops_srt = [(p, col(c)) for p, c in stops]
        for m in range(8):
            cnt = len(stops_srt) if m == 0 else 0
            blob += i32(1 if m == 0 else 0) + i32(cnt) + i32(m) + b"\x00" * 48
        for pos, c in stops_srt:
            blob += i32(pos) + i32(c) + i32(0)
        kette += ape("Color Map", blob)
        kette += colorfade(*rng.choice([(8, -6, -4), (-6, 8, -4), (-4, -6, 8)]))
        if k % 3 == 0:
            kette += chanshift(onbeat=1)
        if k % 5 == 4:
            kette += unique_tone(color=col(pal[3]), blend=1)
        kette += fast_brightness(direction=rng.choice([0, 2, 2]))
        out.append((f"{k+1:02d}_farbrausch", preset(kette)))
    return out


# ---------------------------------------------------------------- 2 LVFX-Themen


def lvfx(children, clear, desc):
    return {
        "header": {"formatVersion": 1, "generator": "LumiViz MultiEffect"},
        "root": {"type": "list", "enabled": True, "name": "root",
                 "description": desc, "clearEveryFrame": clear,
                 "children": children},
    }


def knoten(typ, name, **felder):
    n = {"type": typ, "enabled": True, "name": name}
    n.update(felder)
    return n


def thema_fraktaltraeume():
    """Host-Fraktale in Feedback-Ketten mit Bloom — geht nur in LumiViz."""
    out = []
    grads = ["Neon", "Fire", "Ocean", "Galaxy", "Ice", "Lava", "Sunset",
             "Rainbow", "Forest", "Monochrome"]
    for k in range(20):
        rng = random.Random(f"fraktal{k}")
        grad = grads[k % len(grads)]
        art = k % 4
        kinder = []
        if art == 0:      # Julia-Puls mit Warp-Trail
            kinder.append(knoten(
                "dynamicMovement", "Trail-Sog",
                pointCode=f"d=d*{round(rng.uniform(0.96, 0.99), 3)};"
                          "r=r+0.01;", xres=16, yres=16, subpixel=True))
            kinder.append(knoten(
                "fractal2D", "Julia", ftype=1, blend=2,
                juliaX=round(rng.uniform(-0.9, -0.6), 3),
                juliaY=round(rng.uniform(0.1, 0.3), 3),
                maxIter=180, gradientPreset=grad,
                initCode="z0=1.1", beatCode="z0=1.5",
                frameCode="z0=z0+(1.1-z0)*0.07;zoom=z0;"
                          "rot=rot+0.002+bass*0.004;"))
        elif art == 1:    # Mandelbulb-Kamera am Bass
            kinder.append(knoten(
                "fractal3D", "Bulb", ftype=rng.choice([0, 0, 2, 4]),
                power=rng.choice([6.0, 8.0, 10.0]), gradientPreset=grad,
                frameCode="yaw=yaw+0.004;dist=3.1-bass*0.6;"
                          "pitch=0.3+0.15*sin(time*0.19);"))
        elif art == 2:    # Endlos-Zoom + Kleinian-Overlay
            kinder.append(knoten(
                "fractalZoomer", "Zoom", ftype=rng.choice([0, 1]),
                centerX=-0.743643887, centerY=0.131825904,
                juliaX=-0.7269, juliaY=0.1889, maxIter=220,
                zoomSpeed=round(rng.uniform(1.008, 1.02), 3),
                rotationSpeed=round(rng.uniform(-0.004, 0.004), 4),
                feedback=round(rng.uniform(0.4, 0.7), 2),
                gradientPreset=grad))
            kinder.append(knoten(
                "kleinian", "Kachel", p=rng.choice([5, 7]), q=rng.choice([3, 4]),
                colorScale=0.17, blend=1, gradientPreset=grad,
                frameCode="rot=rot+0.0015;", beatCode="morph=morph+0.3;"))
        else:             # Attraktor-Wolke ueber Reaktions-Diffusion
            kinder.append(knoten(
                "reactionDiffusion", "Grund",
                feed=rng.choice([0.0545, 0.0367, 0.03]),
                kill=rng.choice([0.062, 0.0649]), stepsPerFrame=6,
                gradientPreset=grad, blend=0))
            kinder.append(knoten(
                "strangeAttractor", "Bahn", ftype=rng.choice([1, 2]),
                a=round(rng.uniform(-1.6, 1.6), 2), b=round(rng.uniform(-2.4, 1.8), 2),
                c=round(rng.uniform(0.6, 2.4), 2), d=round(rng.uniform(-2.2, 0.9), 2),
                points=9000, scale=0.35, blend=1, gradientPreset=grad,
                rotationSpeed=0.05))
        kinder.append(knoten("bloom", "Glanz", downsample=2,
                             radius=rng.choice([6, 8]),
                             intensity=round(rng.uniform(0.8, 1.4), 2),
                             threshold=0.2, vignette=(k % 2 == 0),
                             vignetteStrength=0.3, post=True))
        out.append((f"{k+1:02d}_traumtiefe",
                    lvfx(kinder, clear=False,
                         desc=f"Fraktaltraum {k+1} ({grad})")))
    return out


def thema_lichterstadt():
    """Lights-Aesthetik: Plasma-Grund, tanzende Formen, Bloom + Vignette."""
    out = []
    grads = ["Neon", "Galaxy", "Sunset", "Ice", "Lava"]
    for k in range(20):
        rng = random.Random(f"licht{k}")
        grad = grads[k % len(grads)]
        kinder = [knoten(
            "domainWarp", "Grundnebel", octaves=5,
            scale=round(rng.uniform(1.8, 3.4), 2),
            warp=round(rng.uniform(0.4, 1.1), 2),
            speed=round(rng.uniform(0.05, 0.2), 2),
            colorScale=round(rng.uniform(0.8, 1.5), 2),
            gradientPreset=grad, blend=0,
            frameCode="warp=0.4+bass*0.7;")]
        art = k % 3
        if art == 0:
            kinder.append(knoten(
                "flame", "Funkenflug", variation=rng.choice([2, 3]),
                functions=3, points=18000, scale=0.45, blend=1,
                gradientPreset=grad, rotationSpeed=0.05, dotSize=1.5))
        elif art == 1:
            kinder.append(knoten(
                "metaballs3d", "Lichtkugeln"))
        else:
            kinder.append(knoten(
                "superScope", "Lichtfaden",
                pointCode=("u=i*6.2832;px=sin(u*3+t);py=sin(u*4);"
                           "pz=cos(u*2+t*0.7);"
                           "y1=py*cx-pz*sx;z1=py*sx+pz*cx;"
                           "x1=px*cy-z1*sy;z2=px*sy+z1*cy;"
                           "dd=1.5/(z2+2.5);x=x1*dd;y=y1*dd;"
                           "red=0.9;green=0.8;blue=1;"),
                frameCode=("rx=rx+0.012;ry=ry+0.017;cx=cos(rx);sx=sin(rx);"
                           "cy=cos(ry);sy=sin(ry);t=t+0.02;"),
                initCode="n=420;rx=0;ry=0;t=0;", pointCount=420))
        kinder.append(knoten("fadeout", "Nachleuchten",
                             fadeLen=rng.choice([8, 12])))
        kinder.append(knoten("bloom", "Stadtglanz", downsample=2, radius=8,
                             intensity=1.3, threshold=0.0, vignette=True,
                             vignetteStrength=round(rng.uniform(0.25, 0.5), 2),
                             post=True))
        out.append((f"{k+1:02d}_lichtermeer",
                    lvfx(kinder, clear=False,
                         desc=f"Lichterstadt {k+1} ({grad})")))
    return out


# ---------------------------------------------------------------- Hauptlauf

THEMEN = [
    ("01_wurmloecher", thema_wurmloecher, "avs"),
    ("02_plasmanebel", thema_plasmanebel, "avs"),
    ("03_kaleidoskop", thema_kaleidoskop, "avs"),
    ("04_sternenstaub", thema_sternenstaub, "avs"),
    ("05_maschinenraum", thema_maschinenraum, "avs"),
    ("06_drahtgeister", thema_drahtgeister, "avs"),
    ("07_tiefsee", thema_tiefsee, "avs"),
    ("08_farbenrausch", thema_farbenrausch, "avs"),
    ("09_fraktaltraeume", thema_fraktaltraeume, "lvfx"),
    ("10_lichterstadt", thema_lichterstadt, "lvfx"),
]


def main():
    gesamt = 0
    for ordner, fabrik, art in THEMEN:
        ziel = OUT / ordner
        ziel.mkdir(parents=True, exist_ok=True)
        eintraege = fabrik()
        for name, daten in eintraege:
            if art == "avs":
                (ziel / f"{name}.avs").write_bytes(daten)
            else:
                (ziel / f"{name}.lvfx").write_text(
                    json.dumps(daten, indent=4, ensure_ascii=False) + "\n",
                    encoding="utf-8")
        gesamt += len(eintraege)
        print(f"  {ordner}: {len(eintraege)} ({art})")
    print(f"\n{gesamt} Komposit-Presets -> {OUT}")


if __name__ == "__main__":
    main()

# -*- coding: utf-8 -*-
"""Beispiel-Presets je Basis-Voreinstellung (S61): asset/examples/, flach.

Fuer JEDE Vorlage aus asset/nodepresets/ entsteht ein Preset mit passendem
Render-Material, damit der Effekt sauber sichtbar ist:

- **.avs**, wenn sich ALLE gesetzten Felder der Vorlage verlustfrei in das
  AVS-Dateiformat abbilden lassen (Rueckabbildung des kalibrierten Imports,
  AvsChainTranslator.cpp als Feld-Referenz). Diese Beispiele sind direkt mit
  `compare_avsref.py` gegen die Referenz messbar.
- **.lvfx** sonst (host-eigene Typen wie Fraktale/Bloom oder Vorlagen, die
  freigemachte Host-Konstanten setzen).

Dateiname: `<typkey> - <Vorlagenname>.avs|.lvfx` — Typ und Name der
Voreinstellung stehen im Namen, keine Unterordner (Vorgabe Patrik S61).

Material wie die Modul-Matrix (deterministisch, ohne rand): Farbverlaufs-
Spirale + Audio-Wave; Trans-Effekte als MAT_STATIC-Szene (Clear-Basis),
Akkumulations-Effekte als MAT_TRAIL (ohne Basis).

Aufruf:  python make_example_presets.py        # schreibt asset/examples/*
"""
import json
import struct
import sys
from pathlib import Path

from avs_preset_lib import (ape, blitter_feedback, bump, clear_screen,
                            convolution, custom_bpm, ddm, dot_grid,
                            dynamic_movement, dynamic_shift, entry, f32bits,
                            fadeout, grain, i32, ints, lstr, mirror,
                            movement_user, onbeat_clear, osc_ring, osc_star,
                            preset, rotating_stars, superscope, timescope,
                            unique_tone, water_bump)

for _s in (sys.stdout, sys.stderr):
    if hasattr(_s, "reconfigure"):
        _s.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).parent
NODEPRESETS = (ROOT / "../../nodepresets").resolve()
OUT = (ROOT / "../../examples").resolve()

# ---------------------------------------------------------------- Material
# Identisch zur Modul-Matrix (make_matrix_presets.py) — kalibriert, ohne rand.

BASE = clear_screen(color=0x202020)
SPIRAL_POINT = ("d=i*0.85; r=i*18.85+t*0.2; x=cos(r)*d*0.9; y=sin(r)*d*0.7+0.1;\n"
                "red=i; green=1-i; blue=0.4+0.3*sin(i*12);")
SPIRAL = superscope(point=SPIRAL_POINT, frame="t=t+1;", init="n=500; t=0;")
WAVE = superscope(point="x=2*i-1; y=v*0.5-0.2;", init="n=288",
                  colors=(0xC0C0C0,))


def swap_rb(c: int) -> int:
    """AVS-Preset-Farben sind BEREITS 0x00RRGGBB — kein COLORREF!
    `avsColor` im Translator ist ein No-op (Beweis S46: GR_SelectColor
    tauscht den Dialog-COLORREF bei Ein- UND Ausgang, gespeichert wird der
    Framebuffer-Wert). Der fruehere R/B-Tausch hier verdrehte ALLE Farben
    der generierten Beispiele (S61-Befund Patrik: Feuer wurde blau) —
    und der AvsRef-Vergleich blieb gruen, weil beide Seiten dieselbe
    verdrehte Datei lasen. Name bleibt fuer die Aufrufstellen."""
    return c & 0xFFFFFF


def blend_pair(host: int):
    """Host-Blend 0/1/2 (replace/add/50-50) -> (blend, blendavg) der Datei."""
    return (1, 0) if host == 1 else ((0, 1) if host == 2 else (0, 0))


# ---------------------------------------------------------------- .avs-Mapper
# Ein Mapper bildet die VORLAGE (LumiViz-Felder) auf den AVS-Blob ab und gibt
# None zurueck, wenn ein gesetztes Feld dort nicht existiert (-> .lvfx).
# Feld-Referenz je Typ: AvsChainTranslator.cpp (die Import-Richtung).

CODE = ("initCode", "frameCode", "beatCode", "pointCode")


def _extra(n: dict, allowed: set) -> set:
    return set(n) - {"type"} - allowed


def m_superscope(n):
    if _extra(n, {*CODE, "pointCount", "audioChannel", "spectrumSource",
                  "renderMode", "colors"}):
        return None
    init = n.get("initCode", "")
    if "n=" not in init:
        init = f"n={n.get('pointCount', 100)};" + init
    which = (4 if n.get("spectrumSource") else 0) | \
        min(2, int(n.get("audioChannel", 2)))
    cols = tuple(swap_rb(c) for c in n.get("colors", [0xFFFFFF]))
    return superscope(point=n.get("pointCode", ""), init=init,
                      frame=n.get("frameCode", ""), beat=n.get("beatCode", ""),
                      which_ch=which, colors=cols,
                      drawmode=1 if n.get("renderMode", 1) else 0)


def m_simple_scope(n):
    if _extra(n, {"mode", "channel", "position", "colors"}):
        return None
    mode, ch, pos = n.get("mode", 3), n.get("channel", 2), n.get("position", 2)
    if mode >= 4:                       # Dot-Modus: Bit 6, Bit 1 = Scope
        effect = (1 << 6) | (2 if mode == 5 else 0)
    else:
        effect = mode & 3
    effect |= (ch << 2) | (pos << 4)
    cols = [swap_rb(c) for c in n.get("colors", [0xFFFFFF])]
    return entry(0, i32(effect) + i32(len(cols)) +
                 b"".join(i32(c) for c in cols))


def m_osc_star(n):
    if _extra(n, {"channel", "position", "size", "rot", "colors", "amplitude"}):
        return None
    if abs(n.get("amplitude", 1.0) - 1.0) > 1e-9:   # Referenz-Default
        return None
    effect = (n.get("channel", 2) << 2) | (n.get("position", 2) << 4)
    cols = tuple(swap_rb(c) for c in n.get("colors", [0xFFFFFF]))
    return osc_star(effect=effect, colors=cols, size=n.get("size", 8),
                    rot=n.get("rot", 3))


def m_osc_ring(n):
    if _extra(n, {"channel", "position", "size", "source", "colors"}):
        return None
    effect = (n.get("channel", 2) << 2) | (n.get("position", 2) << 4)
    cols = tuple(swap_rb(c) for c in n.get("colors", [0xFFFFFF]))
    return osc_ring(effect=effect, colors=cols, size=n.get("size", 10),
                    source=n.get("source", 0))


def m_rotating_stars(n):
    if _extra(n, {"colors"}):
        return None
    return rotating_stars(tuple(swap_rb(c) for c in n.get("colors",
                                                          [0xE0D060])))


def m_dot_grid(n):
    if _extra(n, {"colors", "spacing", "xMove", "yMove", "blend"}):
        return None
    return dot_grid(colors=tuple(swap_rb(c) for c in n.get("colors",
                                                           [0x4080FF])),
                    spacing=n.get("spacing", 8), x_move=n.get("xMove", 128),
                    y_move=n.get("yMove", 128), blend=n.get("blend", 0))


def m_starfield(n):
    if _extra(n, {"color", "warpSpeed", "maxStars", "onBeat", "beatSpeed",
                  "durationFrames", "blend"}):
        return None
    b, ba = blend_pair(n.get("blend", 0))
    blob = ints(1, swap_rb(n.get("color", 0xFFFFFF)), b, ba)
    blob += f32bits(float(n.get("warpSpeed", 6.0)))
    blob += ints(n.get("maxStars", 350), 1 if n.get("onBeat") else 0)
    blob += f32bits(float(n.get("beatSpeed", 4.0)))
    blob += i32(n.get("durationFrames", 15))
    return entry(27, blob)


def m_timescope(n):
    if _extra(n, {"color", "channel", "bands", "blend"}):
        return None
    host = n.get("blend", 3)            # Host-Default 3 = Default Blend
    blend = 2 if host == 3 else (1 if host == 1 else 0)
    blendavg = 1 if host == 2 else 0
    return timescope(color=swap_rb(n.get("color", 0xFFFFFF)),
                     which_ch=n.get("channel", 2), nbands=n.get("bands", 576),
                     blend=blend, blendavg=blendavg)


def m_fadeout(n):
    if _extra(n, {"fadeLen", "color"}):
        return None
    return fadeout(fadelen=n.get("fadeLen", 16),
                   color=swap_rb(n.get("color", 0)))


def m_blur(n):
    if _extra(n, {"strength", "roundUp"}):
        return None
    return entry(6, ints(n.get("strength", 1), 1 if n.get("roundUp") else 0))


def m_mirror(n):
    if _extra(n, {"mode", "onBeatRandom", "smooth", "slower"}):
        return None
    return mirror(mode=n.get("mode", 4),
                  onbeat=1 if n.get("onBeatRandom") else 0,
                  smooth=1 if n.get("smooth") else 0,
                  slower=int(n.get("slower", 4)))


def m_unique_tone(n):
    if _extra(n, {"color", "invert", "blend"}):
        return None
    b, ba = blend_pair(n.get("blend", 0))
    return unique_tone(color=swap_rb(n.get("color", 0x40FF80)), blend=b,
                       blendavg=ba, inv=1 if n.get("invert") else 0)


def m_interleave(n):
    if _extra(n, {"x", "y", "color", "blend", "onBeat", "x2", "y2",
                  "beatDuration"}):
        return None
    b, ba = blend_pair(n.get("blend", 0))
    x, y = n.get("x", 4), n.get("y", 4)
    return entry(23, ints(1, x, y, swap_rb(n.get("color", 0)), b, ba,
                          1 if n.get("onBeat") else 0, n.get("x2", x),
                          n.get("y2", y), n.get("beatDuration", 4)))


def m_grain(n):
    if _extra(n, {"amount", "staticGrain", "blend"}):
        return None
    b, ba = blend_pair(n.get("blend", 0))
    return entry(24, ints(1, b, ba, n.get("amount", 100),
                          1 if n.get("staticGrain") else 0))


def m_bump(n):
    if _extra(n, {"depth", "depth2", "onBeat", "durationFrames", "invert",
                  "oldStyle", "blend", "initCode", "frameCode", "beatCode"}):
        return None
    b, ba = blend_pair(n.get("blend", 0))
    return bump(depth=n.get("depth", 30), depth2=n.get("depth2", 100),
                onbeat=1 if n.get("onBeat") else 0,
                dur_frames=n.get("durationFrames", 15), blend=b, blendavg=ba,
                frame=n.get("frameCode", ""), beat=n.get("beatCode", ""),
                init=n.get("initCode", ""),
                invert=1 if n.get("invert") else 0,
                oldstyle=1 if n.get("oldStyle") else 0)


def m_interferences(n):
    if _extra(n, {"points", "distance", "alpha", "rotation", "rotationInc",
                  "distance2", "alpha2", "rotationInc2", "rgb", "onBeat",
                  "speed", "blend"}):
        return None
    b, ba = blend_pair(n.get("blend", 0))
    blob = ints(1, n.get("points", 2), n.get("rotation", 0),
                n.get("distance", 10), n.get("alpha", 128),
                n.get("rotationInc", 0), b, ba, n.get("distance2", 32),
                n.get("alpha2", 192), n.get("rotationInc2", 25),
                1 if n.get("rgb") else 0, 1 if n.get("onBeat") else 0)
    return entry(41, blob + f32bits(float(n.get("speed", 0.2))))


def m_movement(n):
    if _extra(n, {"code", "rectCoords", "wrap", "blend", "subpixel",
                  "sourceMapped"}):
        return None
    if not n.get("code"):
        return None
    return movement_user(script=n["code"],
                         blend=1 if n.get("blend") else 0,
                         sourcemapped=n.get("sourceMapped", 0),
                         rectangular=1 if n.get("rectCoords") else 0,
                         subpixel=1 if n.get("subpixel", True) else 0,
                         wrap=1 if n.get("wrap") else 0)


def m_dynamic_movement(n):
    if _extra(n, {*CODE, "rectCoords", "wrap", "blend", "subpixel", "xres",
                  "yres", "nomove", "buffern"}):
        return None
    if n.get("buffern", 0):     # Beispiel ohne Buffer-Verbund
        return None
    # Import rechnet Datei+1 (r_dmove wertet xres+1 Stuetzen aus).
    return dynamic_movement(point=n.get("pointCode", ""),
                            init=n.get("initCode", ""),
                            frame=n.get("frameCode", ""),
                            beat=n.get("beatCode", ""),
                            subpixel=1 if n.get("subpixel", True) else 0,
                            rectcoords=1 if n.get("rectCoords") else 0,
                            xres=max(1, n.get("xres", 17) - 1),
                            yres=max(1, n.get("yres", 13) - 1),
                            blend=1 if n.get("blend") else 0,
                            wrap=1 if n.get("wrap") else 0,
                            nomove=1 if n.get("nomove") else 0)


def m_dynamic_shift(n):
    if _extra(n, {"initCode", "frameCode", "beatCode", "blend", "subpixel"}):
        return None
    return dynamic_shift(init=n.get("initCode", ""),
                         frame=n.get("frameCode", ""),
                         beat=n.get("beatCode", ""),
                         blend=1 if n.get("blend") else 0,
                         subpixel=1 if n.get("subpixel", True) else 0)


def m_ddm(n):
    if _extra(n, {"initCode", "frameCode", "beatCode", "pixelCode", "blend",
                  "subpixel"}):
        return None
    return ddm(point=n.get("pixelCode", ""), frame=n.get("frameCode", ""),
               beat=n.get("beatCode", ""), init=n.get("initCode", ""),
               blend=1 if n.get("blend") else 0,
               subpixel=1 if n.get("subpixel", True) else 0)


def m_roto_blitter(n):
    if _extra(n, {"zoomScale", "zoomScale2", "rotDir", "blend", "beatReverse",
                  "beatReverseSpeed", "beatZoomJump", "subpixel"}):
        return None
    return entry(9, ints(n.get("zoomScale", 31), n.get("rotDir", 31),
                         1 if n.get("blend") else 0,
                         1 if n.get("beatReverse") else 0,
                         n.get("beatReverseSpeed", 0),
                         n.get("zoomScale2", 31),
                         1 if n.get("beatZoomJump") else 0,
                         1 if n.get("subpixel", True) else 0))


def m_blitter_feedback(n):
    if _extra(n, {"scale", "scale2", "blend", "onBeat", "subpixel"}):
        return None
    return blitter_feedback(scale=n.get("scale", 30),
                            scale2=n.get("scale2", n.get("scale", 30)),
                            blend=1 if n.get("blend") else 0,
                            beatch=1 if n.get("onBeat") else 0,
                            subpixel=1 if n.get("subpixel", True) else 0)


def m_water_bump(n):
    if _extra(n, {"density", "depth", "randomDrop", "dropX", "dropY",
                  "dropRadius"}):
        return None
    return water_bump(density=n.get("density", 5), depth=n.get("depth", 600),
                      random_drop=1 if n.get("randomDrop", True) else 0,
                      drop_x=n.get("dropX", 1), drop_y=n.get("dropY", 1),
                      drop_radius=n.get("dropRadius", 40))


def m_custom_bpm(n):
    if _extra(n, {"arbitrary", "arbitraryMs", "skip", "skipCount", "invert",
                  "skipFirst"}):
        return None
    return custom_bpm(enabled=1, arbitrary=1 if n.get("arbitrary") else 0,
                      skip=1 if n.get("skip") else 0,
                      invert=1 if n.get("invert") else 0,
                      arbval=n.get("arbitraryMs", 500),
                      skipval=n.get("skipCount", 1),
                      skipfirst=n.get("skipFirst", 0))


def m_convolution(n):
    if _extra(n, {"kernel", "scale", "edgeMode", "absolute", "twoPass",
                  "bias"}):
        return None
    return convolution(kernel=n.get("kernel"),
                       edge_mode=n.get("edgeMode", 0),
                       absolute=1 if n.get("absolute") else 0,
                       two_pass=1 if n.get("twoPass") else 0,
                       bias=n.get("bias", 0), scale=n.get("scale", 0))


def m_color_map(n):
    if _extra(n, {"stopPos", "stopColor", "key", "blendMode", "adjustBlend"}):
        return None
    stops = list(zip(n.get("stopPos", []),
                     (swap_rb(c) for c in n.get("stopColor", []))))
    blob = i32(n.get("key", 0)) + i32(n.get("blendMode", 0)) + i32(0)
    blob += bytes([n.get("adjustBlend", 128) & 0xFF, 0, 0, 0])
    for m in range(8):
        cnt = len(stops) if m == 0 else 0
        blob += i32(1 if m == 0 else 0) + i32(cnt) + i32(m) + b"\x00" * 48
    for pos, col in stops:
        blob += i32(pos) + i32(col) + i32(0)
    return ape("Color Map", blob)


def m_video_delay(n):
    if _extra(n, {"useBeats", "delay"}):
        return None
    return ape("Holden04: Video Delay",
               ints(1, 1 if n.get("useBeats") else 0, n.get("delay", 30)))


MAPPER = {
    "superScope": m_superscope, "simpleScope": m_simple_scope,
    "oscStar": m_osc_star, "oscRing": m_osc_ring,
    "rotatingStars": m_rotating_stars, "dotGrid": m_dot_grid,
    "starfield": m_starfield, "timescope": m_timescope,
    "fadeout": m_fadeout, "blur": m_blur, "mirror": m_mirror,
    "uniqueTone": m_unique_tone, "interleave": m_interleave,
    "grain": m_grain, "bump": m_bump, "interferences": m_interferences,
    "movement": m_movement, "dynamicMovement": m_dynamic_movement,
    "dynamicShift": m_dynamic_shift, "dynamicDistanceModifier": m_ddm,
    "rotoBlitter": m_roto_blitter, "blitterFeedback": m_blitter_feedback,
    "waterBump": m_water_bump, "customBpm": m_custom_bpm,
    "convolution": m_convolution, "colorMap": m_color_map,
    "videoDelay": m_video_delay,
}

# Szenen: Render-Typen stehen allein (Root-Clear), Akkumulations-Typen laufen
# als Trail VOR den Quellen, alles uebrige als Filter HINTER der Static-Szene.
RENDER_TYPES = {"superScope", "simpleScope", "oscStar", "oscRing",
                "rotatingStars", "dotGrid", "starfield", "timescope"}
TRAIL_TYPES = {"movement", "dynamicMovement", "rotoBlitter",
               "blitterFeedback", "fadeout", "blur"}
SCENE_OVERRIDE = {("blur", "Stehender Weichzeichner"): "filter"}


def avs_scene(typ: str, name: str, node_entry: bytes) -> bytes:
    kind = SCENE_OVERRIDE.get((typ, name))
    if typ == "customBpm":
        # Beat-Filter sichtbar machen: Weiss-Blitz je (gefiltertem) Beat,
        # Fadeout laesst ihn abklingen, die Wave zeigt den Takt.
        return preset(node_entry, onbeat_clear(color=0xFFFFFF),
                      fadeout(fadelen=24), WAVE)
    if typ in RENDER_TYPES and kind is None:
        return preset(node_entry, clear_every_frame=True)
    if typ in TRAIL_TYPES and kind != "filter":
        return preset(node_entry, SPIRAL, WAVE)
    return preset(BASE, SPIRAL, WAVE, node_entry)


# ---------------------------------------------------------------- .lvfx-Weg

LVFX_SPIRAL = {
    "type": "superScope", "enabled": True, "name": "Quelle: Farb-Spirale",
    "pointCode": SPIRAL_POINT, "frameCode": "t=t+1;",
    "initCode": "n=500; t=0;", "pointCount": 500,
}
# Bloom braucht eine HELLE, gebuendelte Quelle: eine 1-px-Spirale verliert im
# Glow-Downsample (1/4-Aufloesung) fast alle Energie, der Threshold frisst den
# Rest — "bloom zeigt gar nichts" (S61-Befund Patrik). Der weisse 3D-Lichtfaden
# mit Trail ist die klassische Bloom-Buehne (Lights-Aesthetik).
LVFX_LICHTFADEN = {
    "type": "superScope", "enabled": True, "name": "Quelle: Lichtfaden",
    "pointCode": ("u=i*6.2832;px=sin(u*3+t);py=sin(u*4);pz=cos(u*2+t*0.7);"
                  "y1=py*cx-pz*sx;z1=py*sx+pz*cx;"
                  "x1=px*cy-z1*sy;z2=px*sy+z1*cy;"
                  "dd=1.5/(z2+2.5);x=x1*dd;y=y1*dd;"
                  "red=1;green=0.95;blue=0.85;"),
    "frameCode": ("rx=rx+0.012;ry=ry+0.017;cx=cos(rx);sx=sin(rx);"
                  "cy=cos(ry);sy=sin(ry);t=t+0.02;"),
    "initCode": "n=420;rx=0;ry=0;t=0;", "pointCount": 420,
}
LVFX_TRAIL = {"type": "fadeout", "enabled": True, "name": "Trail",
              "fadeLen": 10}
# Host-Typen, die selbst zeichnen — sie stehen allein.
LVFX_RENDER = RENDER_TYPES | {
    "fractal2D", "fractal3D", "domainWarp", "fractalZoomer", "lyapunov",
    "kleinian", "strangeAttractor", "flame", "reactionDiffusion",
    "metaballs3d", "tentacles3d",
}


def lvfx_scene(typ: str, name: str, node: dict) -> dict:
    child = {"type": typ, "enabled": True, "name": f"{name} ({typ})"}
    child.update({k: v for k, v in node.items() if k != "type"})
    if typ == "bloom":          # helle Lichtfaden-Buehne mit Trail
        children, clear = [dict(LVFX_TRAIL), dict(LVFX_LICHTFADEN), child], False
    elif typ in LVFX_RENDER:
        children, clear = [child], True
    elif typ in TRAIL_TYPES:
        children, clear = [child, dict(LVFX_SPIRAL)], False
    else:                       # Filter hinter der Quelle
        children, clear = [dict(LVFX_SPIRAL), child], True
    return {
        "header": {"formatVersion": 1, "generator": "LumiViz MultiEffect"},
        "root": {"type": "list", "enabled": True, "name": "root",
                 "description": f"Beispiel fuer Voreinstellung "
                                f"'{name}' ({typ})",
                 "clearEveryFrame": clear, "children": children},
    }


# ---------------------------------------------------------------- Hauptlauf


def nur_avs_eel(node: dict) -> bool:
    """Original-EEL kennt weder `<`/`>` noch `mod()` — LumiViz-Dialekt-Code
    (S53-Figuren) faellt auf .lvfx zurueck, sonst waere AvsRef stumm-schwarz."""
    for k in (*CODE, "code", "pixelCode"):
        c = node.get(k, "")
        if "<" in c or ">" in c or "mod(" in c:
            return False
    return True


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    stats = {"avs": 0, "lvfx": 0}
    for typedir in sorted(NODEPRESETS.iterdir()):
        if not typedir.is_dir():
            continue
        typ = typedir.name
        for f in sorted(typedir.glob("*.json")):
            name = f.stem
            node = json.loads(f.read_text(encoding="utf-8"))["node"]
            data = None
            mapper = MAPPER.get(typ)
            if mapper is not None and nur_avs_eel(node):
                data = mapper(node)
            if data is not None:
                (OUT / f"{typ} - {name}.avs").write_bytes(
                    avs_scene(typ, name, data))
                stats["avs"] += 1
                print(f"  avs   {typ} - {name}")
            else:
                doc = lvfx_scene(typ, name, node)
                (OUT / f"{typ} - {name}.lvfx").write_text(
                    json.dumps(doc, indent=4, ensure_ascii=False) + "\n",
                    encoding="utf-8")
                stats["lvfx"] += 1
                print(f"  lvfx  {typ} - {name}")
    print(f"\n{stats['avs']} .avs + {stats['lvfx']} .lvfx -> {OUT}")


if __name__ == "__main__":
    main()

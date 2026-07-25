# -*- coding: utf-8 -*-
"""Gemeinsame Bausteine zum Schreiben binaerer .avs-Presets (Format 0.2).

Layout-Referenz: projects/libs/AvsParser/include/AvsParserEffects.hpp
(1:1-Transkription der load/save_config aus ref/vis_avs). Container:
24-Byte-Signatur, Root-Mode-Byte, dann Eintraege [int32 id][int32 len][blob].

Genutzt von make_calibration_presets.py (Kalibrier-Gates) und
make_matrix_presets.py (Modul-Matrix, S47).
"""
import struct

# ---------------------------------------------------------------- Grundbausteine


def i32(v: int) -> bytes:
    return struct.pack("<i", v)


def u32(v: int) -> bytes:
    return struct.pack("<I", v & 0xFFFFFFFF)


def f32bits(v: float) -> bytes:
    """float32 als Roh-Bits (Starfield warpSpeed/beatSpeed)."""
    return struct.pack("<f", v)


def lstr(s: str) -> bytes:
    """SizeString wie C_RBASE::load_string: int32-Laenge (inkl. NUL) + Bytes."""
    raw = s.encode("latin-1") + b"\x00"
    return i32(len(raw)) + raw


def ints(*vals: int) -> bytes:
    return b"".join(i32(v) for v in vals)


def quartet(point: str = "", frame: str = "", beat: str = "", init: str = "") -> bytes:
    """Code-Quartett neues Format: Versionsbyte 1 + 4 SizeStrings in Datei-
    Reihenfolge [0]=point/level, [1]=frame, [2]=beat, [3]=init."""
    return b"\x01" + lstr(point) + lstr(frame) + lstr(beat) + lstr(init)


def entry(effect_id: int, blob: bytes) -> bytes:
    return i32(effect_id) + i32(len(blob)) + blob


def preset(*entries: bytes, clear_every_frame: bool = False) -> bytes:
    """Signatur 0.2 + Root-Mode-Byte (Bit 0 = Clear je Frame) + Eintraege."""
    body = b"".join(entries)
    return (b"Nullsoft AVS Preset 0.2\x1a"
            + bytes([1 if clear_every_frame else 0]) + body)


# ---------------------------------------------------------------- Render-Effekte


def simple(effect: int = 0, colors=(0xFFFFFF,)) -> bytes:
    """id 0 — effect (Bitfeld Modus/Kanal), num_colors, colors[] (r_simple)."""
    blob = i32(effect) + i32(len(colors)) + b"".join(i32(c) for c in colors)
    return entry(0, blob)


def dot_plane(rotvel: int = 16, colors=(0x1C6B18, 0xFF0A23, 0x2C9F27, 0x000000,
                                        0x1DC5A3), angle: int = -20) -> bytes:
    """id 1 — rotvel, 5 Farben, angle (r_dotpln; r_raw bleibt Datei-Default)."""
    blob = i32(rotvel) + b"".join(i32(c) for c in colors) + i32(angle)
    return entry(1, blob)


def osc_star(effect: int = 0, colors=(0xFFFFFF,), size: int = 10,
             rot: int = 5) -> bytes:
    """id 2 — effect, num_colors, colors[], size, rot (r_oscstar.cpp)."""
    blob = i32(effect) + i32(len(colors))
    for c in colors:
        blob += i32(c)
    return entry(2, blob + i32(size) + i32(rot))


def bass_spin(enabled: int = 3, color0: int = 0xFF8000, color1: int = 0x0080FF,
              mode: int = 1) -> bytes:
    """id 7 — enabled (Bit 0/1 = Kanal L/R), Farben, mode 0=Linien 1=Dreiecke."""
    return entry(7, ints(enabled, color0, color1, mode))


def moving_particle(color: int = 0xFFFFFF, maxdist: int = 16, size: int = 8,
                    size2: int = 8, blend: int = 1) -> bytes:
    """id 8 — enabled, colors(=Farbe), maxdist, size, size2, blend (r_parts)."""
    return entry(8, ints(1, color, maxdist, size, size2, blend))


def rotating_stars(colors=(0xE0D060,)) -> bytes:
    """id 13 — num_colors + colors[] (r_rotstar.cpp)."""
    return entry(13, i32(len(colors)) + b"".join(i32(c) for c in colors))


def osc_ring(effect: int = 0, colors=(0xFFFFFF,), size: int = 10,
             source: int = 0) -> bytes:
    """id 14 — effect (Platzierung/Kanal-Bits), Farben, size, source (r_oscring)."""
    blob = i32(effect) + i32(len(colors)) + b"".join(i32(c) for c in colors)
    return entry(14, blob + i32(size) + i32(source))


def dot_grid(colors=(0x4080FF,), spacing: int = 8, x_move: int = 128,
             y_move: int = 128, blend: int = 0) -> bytes:
    """id 17 — num_colors+colors[], spacing, x_move, y_move, blend (r_dotgrid;
    Bewegung in 1/256-Pixel je Frame)."""
    blob = i32(len(colors)) + b"".join(i32(c) for c in colors)
    return entry(17, blob + ints(spacing, x_move, y_move, blend))


def dot_fountain(rotvel: int = 16, colors=(0x1C6B18, 0xFF0A23, 0x2C9F27,
                                           0x000000, 0x1DC5A3),
                 angle: int = -20) -> bytes:
    """id 19 — Layout wie Dot Plane (r_dotfnt.cpp)."""
    blob = i32(rotvel) + b"".join(i32(c) for c in colors) + i32(angle)
    return entry(19, blob)


def starfield(color: int = 0xFFFFFF, warp_speed: float = 6.0,
              max_stars: int = 350, blend: int = 0, blendavg: int = 0) -> bytes:
    """id 27 — enabled, color, blend, blendavg, warpSpeed(float-Bits!),
    maxStars, onbeat, beatSpeed(float-Bits), durFrames (r_stars.cpp).
    ACHTUNG: Sternpositionen sind rand()-basiert — Diff-Urteil nur grob."""
    blob = ints(1, color, blend, blendavg) + f32bits(warp_speed)
    blob += ints(max_stars, 0) + f32bits(warp_speed) + i32(15)
    return entry(27, blob)


def timescope(color: int = 0xFFFFFF, which_ch: int = 2,
              nbands: int = 576, blend: int = 2, blendavg: int = 0) -> bytes:
    """id 39 — enabled, color, blend, blendavg, which_ch, nbands (r_timescope)."""
    return entry(39, ints(1, color, blend, blendavg, which_ch, nbands))


def superscope(point: str, init: str = "", frame: str = "", beat: str = "",
               which_ch: int = 0, colors=(0xFFFFFF,), drawmode: int = 1) -> bytes:
    """id 36 — quartet, which_ch (BITFELD: Bits 0-1 Kanal 0=L/1=R/>=2=Center,
    Flag-Wert 4 = Spektrum statt Waveform — r_sscope.cpp:232-240),
    num_colors, colors[] (Framebuffer-Format 0x00RRGGBB — GR_SelectColor
    konvertiert den Dialog-COLORREF beidseitig, Beweis S46 via AvsRef),
    drawmode (Bit 0: 1=Linien)."""
    blob = quartet(point, frame, beat, init) + i32(which_ch) + i32(len(colors))
    for c in colors:
        blob += i32(c)
    return entry(36, blob + i32(drawmode))


# ---------------------------------------------------------------- Trans-Effekte


def fadeout(fadelen: int = 16, color: int = 0x000000) -> bytes:
    """id 3 — fadelen, Zielfarbe (r_fadeout.cpp)."""
    return entry(3, ints(fadelen, color))


def blitter_feedback(scale: int = 30, scale2: int = 30, blend: int = 0,
                     beatch: int = 0, subpixel: int = 1) -> bytes:
    """id 4 — scale, scale2, blend, beatch, subpixel (r_blit.cpp)."""
    return entry(4, ints(scale, scale2, blend, beatch, subpixel))


def onbeat_clear(color: int = 0x000000, blend: int = 0, nf: int = 1) -> bytes:
    """id 5 — color, blend, nf (= alle n Beats) (r_nfclr.cpp)."""
    return entry(5, ints(color, blend, nf))


def blur(roundmode: int = 0) -> bytes:
    """id 6 — enabled(1=leicht,2=normal,3=stark? enabled-Wert!), roundmode.
    r_blur: enabled traegt den Modus (1 = normal)."""
    return entry(6, ints(1, roundmode))


def roto_blitter(zoom_scale: int = 31, rot_dir: int = 31, blend: int = 0,
                 beatch: int = 0, beatch_speed: int = 0, zoom_scale2: int = 31,
                 beatch_scale: int = 0, subpixel: int = 1) -> bytes:
    """id 9 — zoom_scale, rot_dir, blend, beatch, beatch_speed, zoom_scale2,
    beatch_scale, subpixel (r_rotblit.cpp; 31 = neutraler Zoom)."""
    return entry(9, ints(zoom_scale, rot_dir, blend, beatch, beatch_speed,
                         zoom_scale2, beatch_scale, subpixel))


def colorfade(r: int = 8, g: int = -8, b: int = -8) -> bytes:
    """id 11 — enabled, fader r/g/b + Beat-Fader (hier identisch) (r_colorfade)."""
    return entry(11, ints(1, r, g, b, r, g, b))


def color_clip(clip: int = 0x202020, clip_out: int = 0x4080FF,
               dist: int = 10) -> bytes:
    """id 12 — enabled(=Modus 1 below), color_clip, color_clip_out, color_dist."""
    return entry(12, ints(1, clip, clip_out, dist))


def movement_user(script: str, blend: int = 0, sourcemapped: int = 0,
                  rectangular: int = 0, subpixel: int = 1, wrap: int = 0) -> bytes:
    """id 15, User-Skript (effect=32767): Versionsbyte 1 + SizeString,
    dann blend/sourcemapped/rectangular/subpixel/wrap (r_trans.cpp)."""
    blob = i32(32767) + b"\x01" + lstr(script)
    blob += i32(blend) + i32(sourcemapped) + i32(rectangular)
    blob += i32(subpixel) + i32(wrap)
    return entry(15, blob)


def scatter() -> bytes:
    """id 16 — enabled (r_scat.cpp; 4px-Zufallsversatz — Diff nur grob)."""
    return entry(16, ints(1))


def buffer_save(direction: int = 0, which: int = 0, blend: int = 0,
                adjblend_val: int = 128) -> bytes:
    """id 18 — dir (0=save,1=restore,2=alt,3=beides), which (0-7), blend,
    adjblend_val (r_stack.cpp)."""
    return entry(18, ints(direction, which, blend, adjblend_val))


def water() -> bytes:
    """id 20 — enabled (r_water.cpp; rein deterministischer Nachbar-Mittelwert)."""
    return entry(20, ints(1))


def brightness(redp: int = 800, greenp: int = 800, bluep: int = 800,
               blend: int = 0, blendavg: int = 0, dissoc: int = 0) -> bytes:
    """id 22 — enabled, blend, blendavg, redp/greenp/bluep (-4096..4096),
    dissoc, color, exclude, distance (r_bright.cpp)."""
    return entry(22, ints(1, blend, blendavg, redp, greenp, bluep,
                          dissoc, 0, 0, 16))


def interleave(x: int = 4, y: int = 4, color: int = 0x4080FF,
               blend: int = 0) -> bytes:
    """id 23 — enabled, x, y, color, blend, blendavg, onbeat, x2, y2, beatdur."""
    return entry(23, ints(1, x, y, color, blend, 0, 0, x, y, 4))


def grain(smax: int = 100, staticgrain: int = 1, blend: int = 0) -> bytes:
    """id 24 — enabled, blend, blendavg, smax, static (r_grain.cpp;
    Korn ist rand()-basiert — Diff nur grob)."""
    return entry(24, ints(1, blend, 0, smax, staticgrain))


def clear_screen(color: int = 0x202020, blend: int = 0, blendavg: int = 0,
                 onlyfirst: int = 0) -> bytes:
    """id 25 — enabled, color, blend, blendavg, onlyfirst (r_clear.cpp)."""
    return entry(25, ints(1, color, blend, blendavg, onlyfirst))


def mirror(mode: int = 1, onbeat: int = 0, smooth: int = 0,
           slower: int = 4) -> bytes:
    """id 26 — enabled, mode (Bitfeld: 1 l->r, 2 r->l, 4 o->u, 8 u->o),
    onbeat, smooth, slower (r_mirror.cpp)."""
    return entry(26, ints(1, mode, onbeat, smooth, slower))


def bump(depth: int = 60, depth2: int = 100, onbeat: int = 0,
         dur_frames: int = 15, blend: int = 0, blendavg: int = 0,
         frame: str = "x=0.35; y=0.3;", beat: str = "", init: str = "",
         showlight: int = 0, invert: int = 0, oldstyle: int = 0,
         buffern: int = 0) -> bytes:
    """id 29 — enabled..blendavg, 3 SizeStrings (frame, beat, init),
    showlight, invert, oldstyle, buffern (r_bump.cpp)."""
    blob = ints(1, onbeat, dur_frames, depth, depth2, blend, blendavg)
    blob += lstr(frame) + lstr(beat) + lstr(init)
    blob += ints(showlight, invert, oldstyle, buffern)
    return entry(29, blob)


def mosaic(quality: int = 20, quality2: int = 100, onbeat: int = 0,
           blend: int = 0, dur_frames: int = 15) -> bytes:
    """id 30 — enabled, quality, quality2, blend, blendavg, onbeat, durFrames."""
    return entry(30, ints(1, quality, quality2, blend, 0, onbeat, dur_frames))


def water_bump(density: int = 6, depth: int = 600, random_drop: int = 0,
               drop_x: int = 1, drop_y: int = 1, drop_radius: int = 40,
               method: int = 0) -> bytes:
    """id 31 — enabled, density, depth, random_drop, drop_x, drop_y,
    drop_radius, method (r_waterbump.cpp; Tropfen kommen auf BEAT)."""
    return entry(31, ints(1, density, depth, random_drop, drop_x, drop_y,
                          drop_radius, method))


def ddm(point: str, frame: str = "", beat: str = "", init: str = "",
        blend: int = 0, subpixel: int = 1) -> bytes:
    """id 35 — quartet [point, frame, beat, init], blend, subpixel (r_ddm.cpp)."""
    return entry(35, quartet(point, frame, beat, init) + ints(blend, subpixel))


def invert() -> bytes:
    """id 37 — enabled (r_invert.cpp)."""
    return entry(37, ints(1))


def unique_tone(color: int = 0x40FF80, blend: int = 0, blendavg: int = 0,
                inv: int = 0) -> bytes:
    """id 38 — enabled, color, blend, blendavg, invert (r_onetone.cpp)."""
    return entry(38, ints(1, color, blend, blendavg, inv))


def set_render_mode(mode: int, width: int = 2, adjustable: int = 128,
                    enabled: bool = True) -> bytes:
    """id 40 — ein gepacktes int32: Bits 0-7 BLEND_LINE-Modus, 8-15 Adjustable-
    Wert, 16-23 Linienbreite, Bit 31 enabled (r_linemode.cpp)."""
    packed = (mode & 0xFF) | ((adjustable & 0xFF) << 8) | ((width & 0xFF) << 16)
    if enabled:
        packed |= 0x80000000
    return entry(40, u32(packed))


def interferences(n_points: int = 4, rotation: int = 0, distance: int = 20,
                  alpha: int = 128, rotationinc: int = 4, blend: int = 1,
                  rgb: int = 0, onbeat: int = 0, speed: float = 0.2) -> bytes:
    """id 41 — enabled, nPoints, rotation, distance, alpha, rotationinc, blend,
    blendavg, distance2, alpha2, rotationinc2, rgb, onbeat, speed (float-Bits)."""
    blob = ints(1, n_points, rotation, distance, alpha, rotationinc, blend, 0,
                32, 192, 25, rgb, onbeat)
    return entry(41, blob + f32bits(speed))


def dynamic_shift(init: str = "", frame: str = "", beat: str = "",
                  blend: int = 0, subpixel: int = 1) -> bytes:
    """id 42 — Marker 1 + 3 SizeStrings [init, frame, beat], blend, subpixel."""
    blob = b"\x01" + lstr(init) + lstr(frame) + lstr(beat)
    return entry(42, blob + ints(blend, subpixel))


def dynamic_movement(point: str, init: str = "", frame: str = "", beat: str = "",
                     subpixel: int = 1, rectcoords: int = 0, xres: int = 16,
                     yres: int = 12, blend: int = 0, wrap: int = 0,
                     buffern: int = 0, nomove: int = 0) -> bytes:
    """id 43 — quartet + subpixel/rectcoords/xres/yres/blend/wrap/buffern/nomove
    (r_dmove.cpp)."""
    blob = quartet(point, frame, beat, init)
    blob += i32(subpixel) + i32(rectcoords) + i32(xres) + i32(yres)
    blob += i32(blend) + i32(wrap) + i32(buffern) + i32(nomove)
    return entry(43, blob)


def fast_brightness(direction: int = 0) -> bytes:
    """id 44 — dir: 0 = x2, 1 = /2, 2 = aus (r_fastbright.cpp)."""
    return entry(44, ints(direction))


def color_modifier(level: str, frame: str = "", beat: str = "", init: str = "",
                   recompute: int = 1) -> bytes:
    """id 45 — quartet [level, frame, beat, init] + recompute (r_dcolormod)."""
    return entry(45, quartet(level, frame, beat, init) + i32(recompute))


def effect_list(children: bytes, clear: bool = False, blend_in: int = 1,
                blend_out: int = 1, in_adjust: int = 128,
                out_adjust: int = 128) -> bytes:
    """id -2 — Mode-Byte (0x80 -> volle 32 Bit), 24 Byte Extended-Data
    (inBlendVal/outBlendVal/bufferIn/bufferOut/inInvert/outInvert), Kinder.
    Mode-Bits (r_list.h): 0 clear, 1 disabled, 8-12 blendIn,
    16-20 blendOut XOR 1, 24-31 Extended-Groesse."""
    mode = (1 if clear else 0) | ((blend_in & 31) << 8)
    mode |= ((blend_out ^ 1) & 31) << 16
    mode |= 24 << 24                      # 6 x int32 Extended-Data
    blob = bytes([0x80 | (mode & 0x7F)]) + i32(mode & ~0x7F)
    blob += i32(in_adjust) + i32(out_adjust)          # Adjustable-Alphas
    blob += i32(0) + i32(0) + i32(0) + i32(0)         # bufferIn/Out, Invert
    return entry(-2, blob + children)

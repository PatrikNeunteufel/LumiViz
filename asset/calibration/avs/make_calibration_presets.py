# -*- coding: utf-8 -*-
"""Erzeugt die binaeren .avs-Kalibrier-Presets (Format "Nullsoft AVS Preset 0.2").

Layout-Referenz: projects/libs/AvsParser (1:1-Transkription der save/load_config
aus ref/vis_avs) — Container: 24-Byte-Signatur, Root-Mode-Byte, dann Eintraege
[int32 id][int32 len][blob]. Die Dateien laufen in LumiViz UND in echtem
AVS/Winamp (Seite-an-Seite-Urteile).

Aufruf: python make_calibration_presets.py   (schreibt in die Unterordner)
"""
import struct
from pathlib import Path

ROOT = Path(__file__).parent

# ---------------------------------------------------------------- Grundbausteine

def i32(v: int) -> bytes:
    return struct.pack("<i", v)


def u32(v: int) -> bytes:
    return struct.pack("<I", v & 0xFFFFFFFF)


def lstr(s: str) -> bytes:
    """SizeString wie C_RBASE::load_string: int32-Laenge (inkl. NUL) + Bytes."""
    raw = s.encode("latin-1") + b"\x00"
    return i32(len(raw)) + raw


def quartet(point: str = "", frame: str = "", beat: str = "", init: str = "") -> bytes:
    """Code-Quartett neues Format: Versionsbyte 1 + 4 SizeStrings in Datei-Reihenfolge
    [0]=point, [1]=frame, [2]=beat, [3]=init (r_sscope/r_dmove save_config)."""
    return b"\x01" + lstr(point) + lstr(frame) + lstr(beat) + lstr(init)


def entry(effect_id: int, blob: bytes) -> bytes:
    return i32(effect_id) + i32(len(blob)) + blob


# ---------------------------------------------------------------- Effekt-Blobs

def superscope(point: str, init: str = "", frame: str = "", beat: str = "",
               which_ch: int = 0, colors=(0xFFFFFF,), drawmode: int = 1) -> bytes:
    """id 36 — quartet, which_ch (BITFELD: Bits 0-1 Kanal 0=L/1=R/>=2=Center,
    Flag-Wert 4 = Spektrum statt Waveform — r_sscope.cpp:232-240),
    num_colors, colors[] (COLORREF 0x00BBGGRR!), drawmode (Bit 0: 1=Linien)."""
    blob = quartet(point, frame, beat, init) + i32(which_ch) + i32(len(colors))
    for c in colors:
        blob += i32(c)
    return entry(36, blob + i32(drawmode))


def set_render_mode(mode: int, width: int = 2, adjustable: int = 128,
                    enabled: bool = True) -> bytes:
    """id 40 — ein gepacktes int32: Bits 0-7 BLEND_LINE-Modus, 8-15 Adjustable-
    Wert, 16-23 Linienbreite, Bit 31 enabled (r_linemode.cpp)."""
    packed = (mode & 0xFF) | ((adjustable & 0xFF) << 8) | ((width & 0xFF) << 16)
    if enabled:
        packed |= 0x80000000
    return entry(40, u32(packed))


def movement_user(script: str, blend: int = 0, sourcemapped: int = 0,
                  rectangular: int = 0, subpixel: int = 1, wrap: int = 0) -> bytes:
    """id 15, User-Skript (effect=32767): Versionsbyte 1 + SizeString,
    dann blend/sourcemapped/rectangular/subpixel/wrap (r_trans.cpp)."""
    blob = i32(32767) + b"\x01" + lstr(script)
    blob += i32(blend) + i32(sourcemapped) + i32(rectangular)
    blob += i32(subpixel) + i32(wrap)
    return entry(15, blob)


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


def osc_star(effect: int = 0, colors=(0xFFFFFF,), size: int = 10,
             rot: int = 5) -> bytes:
    """id 2 — effect, num_colors, colors[], size, rot (r_oscstar.cpp)."""
    blob = i32(effect) + i32(len(colors))
    for c in colors:
        blob += i32(c)
    return entry(2, blob + i32(size) + i32(rot))


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


def preset(*entries: bytes, clear_every_frame: bool = False) -> bytes:
    """Signatur 0.2 + Root-Mode-Byte (Bit 0 = Clear je Frame) + Eintraege."""
    body = b"".join(entries)
    return (b"Nullsoft AVS Preset 0.2\x1a"
            + bytes([1 if clear_every_frame else 0]) + body)


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

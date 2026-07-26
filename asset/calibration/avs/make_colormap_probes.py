"""Sonden-Presets fuer den Color-Map-APE (S49).

Der Color-Map-Effekt ist ein APE (colormap.ape) — wir haben keinen Quelltext,
also wird seine Kennlinie GEMESSEN: ein Farbverlauf (je Bildspalte eine andere
RGB-Kombination) wird einmal ohne und einmal mit Color Map gerendert. Aus den
Paaren (Eingangsfarbe, Ausgangsfarbe) faellt die komplette Abbildung heraus —
Key-Funktion, Stuetzstellen-Interpolation, Verhalten hinter der letzten
Stuetzstelle, Blend-Modus.

  python make_colormap_probes.py        # erzeugt colormap_probe/*.avs

Referenzlauf braucht AvsRef MIT echten APEs:
  AvsRef colormap_probe/<x>.avs --frames 2 --size 256x256 --ape-dir <sammlung>
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from avs_preset_lib import clear_screen, i32, preset, superscope  # noqa: E402

OUT = Path(__file__).parent / "colormap_probe"
# Der Durchmess-Sweep (einfarbige Bilder je Eingangswert) ist Wegwerf-Material
# und landet unter out/ — nur die 18 strukturierten Sonden bleiben im Repo.
SWEEP = (Path(__file__).parent / "../../../out/colormap_probe_solid").resolve()
APE_ID = 16384  # kApeIdBase — AvsParser akzeptiert jede id >= 16384


def ape(name: str, blob: bytes) -> bytes:
    """APE-Eintrag: [id>=16384][32-Byte-Name][len][blob]."""
    raw = name.encode("ascii")[:32]
    return i32(APE_ID) + raw + b"\x00" * (32 - len(raw)) + i32(len(blob)) + blob


def color_map(stops, key: int = 0, blend: int = 0, adjust: int = 128) -> bytes:
    """Color-Map-Blob nach decodeColorMap (AvsParserEffects.hpp:709)."""
    blob = i32(key) + i32(blend) + i32(0)          # key, blendMode, mapCycleMode
    blob += bytes([adjust & 0xFF, 0, 0, 0])        # adjustBlend, null, dsfb, speed
    for m in range(8):                             # 8 feste Map-Koepfe
        n = len(stops) if m == 0 else 0
        blob += i32(1 if m == 0 else 0) + i32(n) + i32(m) + b"\x00" * 48
    for pos, col in stops:                         # nur Map 0 hat Eintraege
        blob += i32(pos) + i32(col) + i32(0)
    return ape("Color Map", blob)


# Farbwolke 128x128: R laeuft ueber die Spalten, G gegenlaeufig, B ueber die
# Zeilen — alle drei Kanaele variieren unabhaengig, damit sich die Key-Varianten
# (R / G / B / (R+G+B)/2 / MAX / (R+G+B)/3) eindeutig unterscheiden lassen.
# Punktwolke statt Linienzug: Linien interpolieren Farben zwischen den
# Stuetzpunkten, und die Zeichenwege beider Renderer muessten dafuer
# deckungsgleich sein — Punkte sind voneinander unabhaengig. Die Kennlinie
# wird ohnehin JE RENDERER aus dem Paar (Rampe, Rampe+Map) abgeleitet.
# 256 Spalten x 64 Zeilen: R deckt LUECKENLOS 0..255 ab (sonst bleiben
# Kennlinien-Stuetzwerte ungemessen), G gegenlaeufig, B ueber die Zeilen.
RAMP_INIT = "n=16384"
RAMP_POINT = (
    "k=i*(n-1);gy=floor(k/256);gx=k-256*gy;"
    "x=-1+2*gx/255;y=-1+2*gy/63;"
    "red=gx/255;green=1-gx/255;blue=gy/63"
)


def ramp() -> bytes:
    return superscope(RAMP_POINT, init=RAMP_INIT, drawmode=0)  # Punkte


def main() -> None:
    OUT.mkdir(exist_ok=True)
    # Identitaets-Verlauf schwarz->weiss: das Ergebnis IST die Key-Funktion.
    ident = [(0, 0x000000), (255, 0xFFFFFF)]
    files = {}
    for key in range(6):
        files[f"01_key{key}_ident"] = preset(ramp(), color_map(ident, key=key),
                                             clear_every_frame=True)
    # Referenz ohne Color Map (liefert die Eingangsfarben je Pixel)
    files["00_rampe"] = preset(ramp(), clear_every_frame=True)
    # Stuetzstellen-Interpolation + Verhalten hinter der letzten Stelle
    # key=0: der Index ist R und deckt 0..255 ab -> die ganze Kennlinie inkl.
    # des Bereichs HINTER der letzten Stuetzstelle (140) wird sichtbar.
    files["02_stops_bis140"] = preset(
        ramp(),
        color_map([(0, 0x000000), (65, 0xFF8040), (92, 0xFFFF80), (140, 0x000000)],
                  key=0),
        clear_every_frame=True)
    # Einzelspannen schwarz->weiss: aus obs(v) faellt das Interpolations-Gesetz
    # der APE direkt heraus (Spannweite variiert, key 0 -> Index = R).
    for span in (16, 64, 128, 200, 254, 255):
        files[f"04_span{span:03d}"] = preset(
            ramp(), color_map([(0, 0x000000), (span, 0xFFFFFF)], key=0),
            clear_every_frame=True)
    # Drei Stuetzstellen: zeigt, ob nur die LETZTE Spanne anders geteilt wird.
    files["05_drei_stops"] = preset(
        ramp(), color_map([(0, 0x000000), (64, 0xFFFFFF), (255, 0x000000)], key=0),
        clear_every_frame=True)
    # Blend-Modi auf demselben Verlauf (0 replace, 1 additiv, 4 50/50)
    for blend in (0, 1, 4):
        files[f"03_blend{blend}"] = preset(ramp(), color_map(ident, key=0, blend=blend),
                                           clear_every_frame=True)
    # Sweep: EINFARBIGE Bilder. Der Eingangswert ist damit exakt bekannt (die
    # Punktwolke oben streut ueber die Rasterung) — das ist die Sonde, an der
    # Kennlinie und Blend-Modi wirklich haengen.
    SWEEP.mkdir(parents=True, exist_ok=True)
    for f in SWEEP.glob("*.avs"):
        f.unlink()
    vals = sorted(set(list(range(0, 256, 8)) + [1, 2, 3, 63, 65, 127, 129, 199,
                                                201, 253, 254, 255]))
    sweep = 0
    for span in (16, 64, 200, 254, 255):
        for v in vals:
            (SWEEP / f"s{span:03d}_v{v:03d}.avs").write_bytes(preset(
                clear_screen(color=(v << 16) | (v << 8) | v),
                color_map([(0, 0x000000), (span, 0xFFFFFF)], key=0),
                clear_every_frame=True))
            sweep += 1
    for blend in range(10):
        for v in (0, 32, 64, 96, 128, 160, 192, 224, 255):
            (SWEEP / f"b{blend}_v{v:03d}.avs").write_bytes(preset(
                clear_screen(color=(v << 16) | (v << 8) | v),
                color_map([(0, 0x000000), (64, 0xFFFFFF)], key=0, blend=blend,
                          adjust=128),
                clear_every_frame=True))
            sweep += 1
    print(f"{sweep} Sweep-Sonden -> {SWEEP}")

    for name, data in files.items():
        (OUT / f"{name}.avs").write_bytes(data)
        print(f"  {name}.avs  ({len(data)} Bytes)")
    print(f"{len(files)} Sonden -> {OUT}")


if __name__ == "__main__":
    main()

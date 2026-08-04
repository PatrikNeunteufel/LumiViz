#!/usr/bin/env python3
"""EEL-Divisions-Sonde fuer AvsRef (S67): baut sonde_div.avs.

Klaert den AVS-EEL-Divisions-Vertrag (Pendant zur MilkdropRef-Sonde
asset/Milkdrop3/sonden/): Ein Superscope zeichnet eine Linie, deren Farbe
das Ergebnis von 2/0 kodiert —
  ROT   = inf   (above(abs(v), 1e6))
  GRUEN = exakt 0 (equal(v, 0))
  BLAU  = NaN   (equal(z/z, z/z) ist dann 0)
Auswertung: mean-RGB-Verhaeltnis von AvsRef vs. AvsStandalone.
Format nach AvsParser (readCodeQuartet: Version 1 + 4 laengen-praefixierte
Strings [point, frame, beat, init], dann which_ch/num_colors/colors/drawmode).
"""
from pathlib import Path
import struct

HERE = Path(__file__).parent

INIT = b"n=200;"
FRAME = b"zz=0; vv=2/zz; nanv=zz/zz;"
BEAT = b""
POINT = (b"x=i*2-1; y=0;"
         b"red=if(above(abs(vv),1000000),1,0);"
         b"green=if(equal(vv,0),1,0);"
         b"blue=if(equal(nanv,nanv),0,1);")


def s(code: bytes) -> bytes:
    """save_string-Format: int32-Laenge inkl. NUL + Bytes + NUL."""
    return struct.pack("<i", len(code) + 1) + code + b"\x00"


def main() -> None:
    blob = b"\x01" + s(POINT) + s(FRAME) + s(BEAT) + s(INIT)
    blob += struct.pack("<i", 0)          # which_ch (Quelle)
    blob += struct.pack("<i", 1)          # num_colors
    blob += struct.pack("<i", 0xFFFFFF)   # Farbe (wird per point ueberschrieben)
    blob += struct.pack("<i", 1)          # drawmode: Linien
    entry = struct.pack("<ii", 36, len(blob)) + blob   # 36 = SuperScope

    preset = b"Nullsoft AVS Preset 0.2\x1a" + b"\x01" + entry  # Mode 1 = clear
    out = HERE / "sonde_div.avs"
    out.write_bytes(preset)
    print(f"{out} ({len(preset)} Bytes)")


if __name__ == "__main__":
    main()

# -*- coding: utf-8 -*-
"""Erzeugt `testvideo.avi` — das Test-Asset des AVI-Knotens (Strang E, S54).

Der AVI-Effekt dekodiert ueber **Video for Windows** (`AVIFileOpenA` /
`AVIStreamGetFrame`), dieselbe Schnittstelle wie das Original — deshalb genuegt
ein UNKOMPRIMIERTES RGB-AVI, das sich von Hand schreiben laesst. Keine externe
Bibliothek, kein Codec.

Aufbau (RIFF):
    RIFF 'AVI '
      LIST 'hdrl'  avih (Kopf) + LIST 'strl' (strh Stream, strf Bitmap)
      LIST 'movi'  je Frame ein '00db'-Block, unkomprimiert, bottom-up
      idx1         Index — VfW braucht ihn, sonst liefert AVIStreamGetFrame nichts

Inhalt: acht Frames, in denen ein heller Balken von links nach rechts wandert,
auf wechselndem Grund. So zeigt sich BEIDES: dass ueberhaupt ein Bild kommt
(Balken) und dass die Zeit laeuft (Position) — damit werden `speedMs`,
`persist` und `adapt` unterscheidbar.

Erzeugt werden ZWEI Dateien, 32- und 24-bittig. 24 Bit ist bei unkomprimierten
AVIs der Normalfall, und genau die verwarf `runAvi` bis S55 stillschweigend
(`bih->biBitCount == 32`): der Knoten oeffnete die Datei, las Frames und
zeichnete nichts. VfW liefert solche Dateien einwandfrei — nachgemessen ueber
ctypes, AVIFileOpenA/AVIFileGetStream/AVIStreamGetFrame geben alle acht Frames
heraus. Seit dem Fix zeichnet der Renderer beide Tiefen; `testvideo24.avi` ist
der Waechter dafuer (s. HANDWERK `avi.filename` in make_field_probes.py).

Aufruf:  python make_testvideo.py
"""
from __future__ import annotations

import struct
from pathlib import Path

BREITE, HOEHE, FRAMES, FPS = 32, 32, 8, 10


def zeilenlaenge(bits: int) -> int:
    """DIB-Zeilen sind auf 4 Bytes aufgerundet."""
    return ((BREITE * (bits // 8)) + 3) & ~3


def frame_bytes(i: int, bits: int) -> bytes:
    """Ein Frame als bottom-up BGR(X) — wandernder Balken auf Wechselgrund."""
    x0 = (i * BREITE) // FRAMES
    grund = (32, 32, 48) if i % 2 == 0 else (48, 32, 32)
    fuell = b"\x00" * (zeilenlaenge(bits) - BREITE * (bits // 8))
    zeilen = []
    for _ in range(HOEHE):
        px = bytearray()
        for x in range(BREITE):
            r, g, b = (255, 240, 80) if x0 <= x < x0 + 6 else grund
            px += bytes((b, g, r, 0)) if bits == 32 else bytes((b, g, r))
        zeilen.append(bytes(px) + fuell)
    return b"".join(reversed(zeilen))  # bottom-up


def liste(kennung: bytes, inhalt: bytes) -> bytes:
    return b"LIST" + struct.pack("<I", len(inhalt) + 4) + kennung + inhalt


def block(kennung: bytes, inhalt: bytes) -> bytes:
    pad = b"\x00" if len(inhalt) % 2 else b""
    return kennung + struct.pack("<I", len(inhalt)) + inhalt + pad


def schreibe(bits: int, ziel: Path) -> None:
    BILD = zeilenlaenge(bits) * HOEHE
    avih = struct.pack("<14I", 1000000 // FPS, BILD * FPS, 0,
                       0x10,            # AVIF_HASINDEX
                       FRAMES, 0, 1, BILD, BREITE, HOEHE, 0, 0, 0, 0)
    # AVIStreamHeader ohne die beiden FOURCC am Anfang: dwFlags, wPriority,
    # wLanguage, dwInitialFrames, dwScale, dwRate, dwStart, dwLength,
    # dwSuggestedBufferSize, dwQuality, dwSampleSize — dann rcFrame (4 short).
    strh = (b"vids" + b"DIB " +
            struct.pack("<I2H8I", 0, 0, 0, 0, 1, FPS, 0, FRAMES, BILD,
                        0xFFFFFFFF, 0)
            + struct.pack("<4h", 0, 0, BREITE, HOEHE))
    strf = struct.pack("<I2i2H6I", 40, BREITE, HOEHE, 1, bits, 0, BILD, 0, 0, 0, 0)
    hdrl = liste(b"hdrl", block(b"avih", avih) +
                 liste(b"strl", block(b"strh", strh) + block(b"strf", strf)))

    daten, index, versatz = b"", b"", 4
    for i in range(FRAMES):
        roh = frame_bytes(i, bits)
        daten += block(b"00db", roh)
        index += b"00db" + struct.pack("<3I", 0x10, versatz, len(roh))  # AVIIF_KEYFRAME
        versatz += len(roh) + 8

    rumpf = b"AVI " + hdrl + liste(b"movi", daten) + block(b"idx1", index)
    ziel.write_bytes(b"RIFF" + struct.pack("<I", len(rumpf)) + rumpf)
    print(f"{ziel.name}: {ziel.stat().st_size} Bytes, {bits} Bit, "
          f"{FRAMES} Frames {BREITE}x{HOEHE} @ {FPS} fps")


def main() -> None:
    hier = Path(__file__).parent
    schreibe(32, hier / "testvideo.avi")
    schreibe(24, hier / "testvideo24.avi")


if __name__ == "__main__":
    main()

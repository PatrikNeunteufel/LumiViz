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

Das Video ist **32-bittig**. Erster Versuch waren 24 Bit — VfW nahm die Datei
anstandslos (belegt ueber ctypes: AVIFileOpenA, AVIFileGetStream,
AVIStreamGetFrame liefern alle acht Frames), aber `runAvi` prueft
`bih->biBitCount == 32` und ueberspringt jedes andere Format ohne Meldung.
Solange das so ist, MUSS das Testvideo 32 Bit haben.

Aufruf:  python make_testvideo.py
"""
from __future__ import annotations

import struct
from pathlib import Path

BREITE, HOEHE, FRAMES, FPS = 32, 32, 8, 10
# 32 Bit (BGRX), NICHT 24: `runAvi` verarbeitet nur `biBitCount == 32`
# und ueberspringt alles andere stillschweigend (Befund S54).
ZEILE = BREITE * 4
BILD = ZEILE * HOEHE


def frame_bytes(i: int) -> bytes:
    """Ein Frame als bottom-up BGR — der wandernde Balken auf Wechselgrund."""
    x0 = (i * BREITE) // FRAMES
    grund = (32, 32, 48) if i % 2 == 0 else (48, 32, 32)
    zeilen = []
    for _ in range(HOEHE):
        px = bytearray()
        for x in range(BREITE):
            r, g, b = (255, 240, 80) if x0 <= x < x0 + 6 else grund
            px += bytes((b, g, r, 0))  # BGRX, wie der 32-Bit-DIB es will
        zeilen.append(bytes(px))
    return b"".join(reversed(zeilen))  # bottom-up


def liste(kennung: bytes, inhalt: bytes) -> bytes:
    return b"LIST" + struct.pack("<I", len(inhalt) + 4) + kennung + inhalt


def block(kennung: bytes, inhalt: bytes) -> bytes:
    pad = b"\x00" if len(inhalt) % 2 else b""
    return kennung + struct.pack("<I", len(inhalt)) + inhalt + pad


def main() -> None:
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
    strf = struct.pack("<I2i2H6I", 40, BREITE, HOEHE, 1, 32, 0, BILD, 0, 0, 0, 0)
    hdrl = liste(b"hdrl", block(b"avih", avih) +
                 liste(b"strl", block(b"strh", strh) + block(b"strf", strf)))

    daten, index, versatz = b"", b"", 4
    for i in range(FRAMES):
        roh = frame_bytes(i)
        daten += block(b"00db", roh)
        index += b"00db" + struct.pack("<3I", 0x10, versatz, len(roh))  # AVIIF_KEYFRAME
        versatz += len(roh) + 8

    rumpf = b"AVI " + hdrl + liste(b"movi", daten) + block(b"idx1", index)
    ziel = Path(__file__).parent / "testvideo.avi"
    ziel.write_bytes(b"RIFF" + struct.pack("<I", len(rumpf)) + rumpf)
    print(f"{ziel.name}: {ziel.stat().st_size} Bytes, "
          f"{FRAMES} Frames {BREITE}x{HOEHE} @ {FPS} fps")


if __name__ == "__main__":
    main()

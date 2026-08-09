# -*- coding: utf-8 -*-
"""Prüfstand für die globalen Buffer: Sichern, Zurückholen, Blend (S74).

Aufruf:
  python make_bufferprobe_avs.py <zielordner> --raster gitter.avs --zweit verlauf.avs

Warum eigens
------------
Der Listen-Prüfstand deckt `blendin = 12` ab (Buffer als Maske), aber mit
**unbeschriebenem** Buffer — beide Renderer liefern dort je ein einziges Bild,
die Probe unterscheidet nicht (gemessen S74). Ohne Kreislauf ist über den
Buffer-Pfad nichts ausgesagt.

Ein Kreislauf braucht drei Schritte, und genau die baut dieser Generator:

    Raster  ->  BufferSave(sichern, N)  ->  zweites Bild  ->  BufferSave(zurückholen, N, blend)

Der erste Schritt legt einen bekannten Inhalt ab, der zweite verändert das
sichtbare Bild, der dritte holt das Abgelegte zurück. Weicht das Ergebnis von
der Referenz ab, liegt es am Buffer-Pfad — Raster und zweites Bild sind einzeln
abgenommen.

Format (r_stack.cpp load_config): Entry `[id=18][len][dir][which][blend][adjblend_val]`
  dir   0 = Framebuffer -> Buffer (sichern) · 1 = Buffer -> Framebuffer
        (zurückholen) · >=2 = bei jedem Aufruf abwechselnd
  which Buffer-Nummer 0..7 (NBUF)
  blend 0 = ersetzen · 1 = 50/50 · 2 = additiv · 3 = jeder zweite Pixel
        (Schachbrett, zeilenversetzt) · 4 = subtraktiv · 5 = jede zweite Zeile

Die beiden RASTER-Modi (3 und 5) zaehlen Zeilen von OBEN. Wer sie in GLSL ueber
`gl_FragCoord.y` nachbaut, trifft bei gerader Bildhoehe genau die Gegenpixel —
gemessen S74: MAE 0,451 bei Hoehe 240, 0,002 bei 241.

**Ungeschriebener Buffer:** das Original legt die globalen Buffer genullt an.
Was ein Zurückholen ohne vorheriges Sichern liefert, ist damit selbst ein
Vertrag — und genau solche Startzustände laufen zwischen Nachbau und Original
gern auseinander. Die Probe `leer_*` prüft ihn ausdrücklich.
"""
import argparse
import struct
from pathlib import Path

SIG = b"Nullsoft AVS Preset 0.2\x1a"
BUFSAVE_ID = 18

# Blend-Codes aus r_stack.cpp. ACHTUNG bei 3: das ist NICHT der einstellbare
# Blend, sondern ein zeilenversetztes Schachbrett — `adjblend_val` spielt dort
# keine Rolle. Die Fehlbenennung im ersten Anlauf (S74) hat zu der falschen
# Aussage gefuehrt, "der Wert wirkt nicht".
BLEND_NAMEN = {0: "ersetzen", 1: "50_50", 2: "additiv", 3: "jeder_2_pixel",
               4: "subtraktiv", 5: "jede_2_zeile"}


def bufsave_entry(dir_: int, which: int, blend: int = 0,
                  adjblend: int = 128) -> bytes:
    blob = (struct.pack("<i", dir_) + struct.pack("<i", which)
            + struct.pack("<i", blend) + struct.pack("<i", adjblend))
    return struct.pack("<i", BUFSAVE_ID) + struct.pack("<i", len(blob)) + blob


def entries_von(pfad: Path) -> bytes:
    """Rohbytes der Top-Level-Entries eines .avs (ohne Signatur/Modus-Byte)."""
    data = pfad.read_bytes()
    assert data.startswith(SIG), f"kein .avs: {pfad}"
    return data[len(SIG) + 1:]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ziel", type=Path)
    ap.add_argument("--raster", type=Path, required=True,
                    help="Inhalt, der gesichert wird (abgenommenes Messmittel)")
    ap.add_argument("--zweit", type=Path, required=True,
                    help="zweites Bild, das dazwischen gezeichnet wird")
    args = ap.parse_args()

    raster = entries_von(args.raster)
    zweit = entries_von(args.zweit)
    args.ziel.mkdir(parents=True, exist_ok=True)
    gebaut = 0

    def schreibe(name: str, body: bytes):
        nonlocal gebaut
        (args.ziel / f"{name}.avs").write_bytes(SIG + b"\x00" + body)
        gebaut += 1

    # --- Voller Kreislauf je Blend-Modus ------------------------------------------
    for blend, bname in sorted(BLEND_NAMEN.items()):
        schreibe(f"kreis_b{blend}_{bname}",
                 raster + bufsave_entry(0, 0) + zweit
                 + bufsave_entry(1, 0, blend))

    # --- Alle acht Buffer: schreibt jeder auf seinen eigenen Platz? ----------------
    for n in range(8):
        schreibe(f"buffer_{n}",
                 raster + bufsave_entry(0, n) + zweit + bufsave_entry(1, n))

    # --- Kreuzprobe: in 0 sichern, aus 1 holen — muss LEER sein --------------------
    schreibe("kreuz_0_nach_1",
             raster + bufsave_entry(0, 0) + zweit + bufsave_entry(1, 1))

    # --- Ungeschriebener Buffer: was liefert ein Zurückholen ohne Sichern? ---------
    schreibe("leer_holen", raster + bufsave_entry(1, 5))
    schreibe("leer_holen_5050", raster + bufsave_entry(1, 5, 1))

    # --- Nur sichern: darf das sichtbare Bild NICHT verändern ----------------------
    schreibe("nur_sichern", raster + bufsave_entry(0, 0))

    # --- dir >= 2: wechselt je Aufruf die Richtung ---------------------------------
    schreibe("wechselnd", raster + bufsave_entry(2, 0) + zweit
             + bufsave_entry(2, 0))

    print(f"{gebaut} Proben -> {args.ziel}")
    print("Erwartung: jede Probe ~0 gegen AvsRef. Zusaetzlich pruefen, ob die "
          "Proben sich UNTEREINANDER unterscheiden — sonst regt der Prüfstand "
          "den Buffer-Pfad nicht an (Regel 6).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

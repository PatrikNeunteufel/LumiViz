# -*- coding: utf-8 -*-
"""Prüfstand für Effektlisten: Blend In/Out, Buffer und Reihenfolge (S74).

Aufruf:
  python make_listenprobe_avs.py <zielordner> --raster raster.avs --knoten <preset.avs>#<index>

Warum eigens
------------
Der Einzelknoten-Lauf (`bisect_avs.py --solo`) beantwortet „stimmt dieser
Effekt". Er beantwortet NICHT, ob er richtig AUFGETRAGEN wird. Genau daran
hing in S74 der erste Raster-Anlauf: das Raster vor eine Liste gesetzt war im
Ergebnis verschwunden, weil die Liste mit `blendin = 1` (Replace) die Fläche
unter sich überschreibt — beide Renderer lieferten schwarz, gemessen wurde
nichts. Der Listen-Kopf ist also kein Beiwerk, er entscheidet, ob überhaupt
etwas zu sehen ist.

`02_color extasy` und `07_movin wall` sind die Fälle, die das brauchen: kein
einziger Knoten für sich auffällig, das ganze Preset aber deutlich daneben.

Was geprüft wird
----------------
Aufbau je Probe: **Raster auf Wurzelebene**, darüber eine Liste, die den
Prüf-Knoten enthält. Variiert wird der Listen-Kopf. Weicht eine Probe ab,
liegt es am Auftragen, nicht am Effekt.

Listen-Kopf (r_list.cpp load_config):
  mode-Byte; Bit 0x80 gesetzt -> zusaetzliches int32, das mit mode verodert wird
  danach inblendval, outblendval, bufferin, bufferout, ininvert, outinvert,
  beat_render, beat_render_frames (je int32)
  blendin  = (mode >> 8)  & 31
  blendout = ((mode >> 16) & 31) ^ 1

Blend-Modi (switch use_blendin, r_list.cpp):
  1 Replace · 2 50/50 · 3 Maximum · 4 Additiv · 5 Subtraktiv 1 ·
  6 Subtraktiv 2 · 7 Multiplikativ-Kette · 8 Adjustable-Variante ·
  9 XOR · 10 Adjustable (nutzt inblendval) · 11 Multiply ·
  12 Buffer (nutzt bufferin/ininvert) · 13 Minimum
"""
import argparse
import struct
from pathlib import Path

SIG = b"Nullsoft AVS Preset 0.2\x1a"
LIST_ID = -2

BLEND_NAMEN = {
    1: "replace", 2: "50_50", 3: "maximum", 4: "additiv",
    5: "subtraktiv1", 6: "subtraktiv2", 7: "multi_kette", 8: "adj_variante",
    9: "xor", 10: "adjustable", 11: "multiply", 12: "buffer", 13: "minimum",
}


def list_entry(blendin: int, blendout: int = 1, inblendval: int = 128,
               outblendval: int = 128, bufferin: int = 0, bufferout: int = 0,
               ininvert: int = 0, outinvert: int = 0,
               kinder: bytes = b"") -> bytes:
    """Listen-Entry mit ausgeschriebenem Kopf (immer erweitertes Format).

    **Das obere Byte von `mode` traegt die Groesse der erweiterten Daten** —
    `set_extended_datasize(36)` im Original, und `load_config` liest die acht
    Kopf-Werte NUR, solange `pos < get_extended_datasize()+5`. Ohne dieses Byte
    ist `ext = 5`, kein einziger Wert wird gelesen, und der Parser laeuft
    mitten in die Kinder (Symptom S74: „abgeschnittener Config-Blob, deklariert
    128, vorhanden 52" — beide Renderer verwarfen die Liste stillschweigend und
    lieferten deshalb identische, nichtssagende Messwerte).

    36 ist die Groesse der erweiterten Daten „+4 cause we fucked up"
    (Originalkommentar); der Kopf ist damit 1 + 4 + 8*4 = 37 Bytes.

    **`blendout` ist im Feld invertiert abgelegt** (`blendout() = ((mode>>16)&31)^1`)
    und die Vorgabe hier ist 1 = Replace. Mit 0 rendert die Liste zwar intern,
    ihr Ergebnis wird aber nie zurueckgeschrieben — sichtbar bleibt nur, was
    vorher da war. Zweite Falle derselben Sorte wie die fehlende
    Datengroesse: die Probe misst dann brav ueber alle Blend-Modi hinweg immer
    denselben Wert und sieht dabei aus wie ein bestandener Test.
    """
    mode = (((blendin & 31) << 8) | (((blendout ^ 1) & 31) << 16)
            | (36 << 24))
    kopf = (bytes([(mode & 0xFF) | 0x80])
            + struct.pack("<i", mode)
            + struct.pack("<i", inblendval)
            + struct.pack("<i", outblendval)
            + struct.pack("<i", bufferin)
            + struct.pack("<i", bufferout)
            + struct.pack("<i", ininvert)
            + struct.pack("<i", outinvert)
            + struct.pack("<i", 0)      # beat_render
            + struct.pack("<i", 0))     # beat_render_frames
    body = kopf + kinder
    return struct.pack("<i", LIST_ID) + struct.pack("<i", len(body)) + body


def parse_entries(data: bytes, pos: int, end: int):
    """Wie bisect_avs.parse_entries — hier nur zum Herausschneiden eines Knotens."""
    entries = []
    while pos + 8 <= end:
        eid = struct.unpack_from("<i", data, pos)[0]
        hdr = pos + 4
        if eid >= 16384:
            hdr += 32
        length = struct.unpack_from("<i", data, hdr)[0]
        blob_end = hdr + 4 + length
        if length < 0 or blob_end > end:
            break
        entries.append((eid, data[pos:blob_end]))
        pos = blob_end
    return entries


def knoten_holen(spec: str) -> bytes:
    """`<preset.avs>#<index>` -> Rohbytes des Top-Level-Entries."""
    pfad, _, idx = spec.rpartition("#")
    data = Path(pfad).read_bytes()
    assert data.startswith(SIG), f"kein .avs: {pfad}"
    entries = parse_entries(data, len(SIG) + 1, len(data))
    return entries[int(idx)][1]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ziel", type=Path)
    ap.add_argument("--raster", type=Path, required=True)
    ap.add_argument("--knoten", required=True,
                    help="<preset.avs>#<top-level-index> — der Prüf-Knoten")
    ap.add_argument("--reihenfolge", action="store_true",
                    help="zusätzlich beide Reihenfolgen Raster/Knoten erzeugen")
    args = ap.parse_args()

    rdata = args.raster.read_bytes()
    assert rdata.startswith(SIG), "Raster ist kein .avs"
    raster = rdata[len(SIG) + 1:]          # Signatur + Wurzel-Modus-Byte weg
    knoten = knoten_holen(args.knoten)
    args.ziel.mkdir(parents=True, exist_ok=True)

    gebaut = []
    # --- Blend In: Raster auf Wurzelebene, Liste mit dem Knoten darüber -----------
    for bi, name in sorted(BLEND_NAMEN.items()):
        datei = args.ziel / f"blendin_{bi:02d}_{name}.avs"
        datei.write_bytes(SIG + b"\x00" + raster
                          + list_entry(blendin=bi, kinder=knoten))
        gebaut.append(datei.name)

    # --- Adjustable über die ganze Skala: derselbe Modus, andere Staerke ----------
    for val in (0, 64, 128, 192, 255):
        datei = args.ziel / f"adjustable_{val:03d}.avs"
        datei.write_bytes(SIG + b"\x00" + raster
                          + list_entry(blendin=10, inblendval=val, kinder=knoten))
        gebaut.append(datei.name)

    # --- Buffer: blendin 12 zieht die Maske aus globalem Buffer N -----------------
    # Der Buffer ist hier UNBESCHRIEBEN. Das ist Absicht: was ein ungesetzter
    # Buffer liefert, ist selbst ein Vertrag (Original: genullt), und genau
    # solche Startzustaende laufen zwischen Nachbau und Original gern
    # auseinander. Buffer-Kreislaeufe mit Save/Restore kommen als eigener
    # Pruefstand, sobald dieser hier gruen ist.
    for buf in (0, 1, 2):
        for inv in (0, 1):
            datei = args.ziel / f"buffer_{buf}_inv{inv}.avs"
            datei.write_bytes(SIG + b"\x00" + raster
                              + list_entry(blendin=12, bufferin=buf,
                                           ininvert=inv, kinder=knoten))
            gebaut.append(datei.name)

    # --- Reihenfolge: beide Anordnungen im SELBEN Listen-Kontext ------------------
    if args.reihenfolge:
        for bi in (1, 2, 4):
            for tag, kinder in (("raster_dann_knoten", raster + knoten),
                                ("knoten_dann_raster", knoten + raster)):
                datei = args.ziel / f"reihenfolge_{bi:02d}_{tag}.avs"
                datei.write_bytes(SIG + b"\x00" + list_entry(blendin=bi,
                                                             kinder=kinder))
                gebaut.append(datei.name)

    print(f"{len(gebaut)} Proben -> {args.ziel}")
    print("Erwartung: JEDE Probe muss gegen AvsRef ~0 liefern. Wo nicht, liegt "
          "der Unterschied im Auftragen, nicht im Effekt.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

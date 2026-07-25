# -*- coding: utf-8 -*-
"""Preset-Bisektion (S46): kumulative Teil-Presets aus einem beliebigen .avs.

Aufruf:
  python bisect_avs.py <preset.avs> <zielordner>            # Top-Level + 1. Liste
  python bisect_avs.py <preset.avs> <zielordner> 1,2        # Liste am Index-Pfad

Erzeugt stage_t<k>.avs (Top-Level-Praefixe), stage_l<k>.avs (erste Liste mit
nur den ersten k Kindern) bzw. stage_p<k>.avs (Pfad-Modus). Die Stufen danach
durch compare_avsref.py jagen -> erste divergente Stufe = Taeter. ACHTUNG:
Stufen, die einen Buffer-Kreislauf abschneiden (BufferSave am Listenende),
sind nicht isoliert bewertbar. Format: Entries [id:int32][len:int32][blob];
Listen (id=-2) tragen Mode-Byte (0x80 -> +int32 mode, dann Extended-Bytes
inkl. "+4 fucked up"-Regel) vor den Kind-Entries (r_list.cpp load_config).
"""
import struct
import sys
from pathlib import Path

SIG = b"Nullsoft AVS Preset 0.2\x1a"
DLLRENDERBASE = 16384


def parse_entries(data, pos, end):
    """Liefert Liste von (id, roh_bytes_des_gesamten_entries, blob_start, blob_end)."""
    entries = []
    while pos + 8 <= end:
        eid = struct.unpack_from("<i", data, pos)[0]
        hdr = pos + 4
        if eid >= DLLRENDERBASE:
            hdr += 32  # APE-Id-String
        length = struct.unpack_from("<i", data, hdr)[0]
        blob_start = hdr + 4
        blob_end = blob_start + length
        if length < 0 or blob_end > end:
            break
        entries.append((eid, data[pos:blob_end], blob_start, blob_end))
        pos = blob_end
    return entries


def list_children(data, blob_start, blob_end):
    """Zerlegt einen Listen-Blob in (praefix_bytes, kind_entries)."""
    # Exakt r_list.cpp::load_config: ext = extsize+5; 6 Felder solange
    # rel<ext, dann 2 Felder solange rel<ext-4 ("+4 cause we fucked up").
    pos = blob_start
    b0 = data[pos]
    pos += 1
    if b0 & 0x80:
        mode = struct.unpack_from("<i", data, pos)[0]
        pos += 4
        ext = ((mode >> 24) & 0xFF) + 5
        rel = 5
        for _ in range(6):
            if rel < ext:
                rel += 4
        for _ in range(2):
            if rel < ext - 4:
                rel += 4
        pos = blob_start + rel
    prefix = data[blob_start:pos]
    return prefix, parse_entries(data, pos, blob_end)


def build_list_entry(prefix, child_blobs):
    body = prefix + b"".join(child_blobs)
    return struct.pack("<i", -2) + struct.pack("<i", len(body)) + body


def bisect_path(data, root_mode, top, path, out):
    """Bisektiert die Kinder der Liste am Index-Pfad (z. B. [1, 2]):
    aeussere Struktur bleibt, die Ziel-Liste bekommt nur die ersten k Kinder."""
    # Zum Ziel absteigen, dabei je Ebene (prefix_entries, listen_prefix, kids) merken
    levels = []
    entries = top
    for idx in path:
        eid, raw, bs, be = entries[idx]
        assert eid == -2, f"Pfad-Index {idx} ist keine Liste (id={eid})"
        lp, kids = list_children(data, bs, be)
        levels.append((entries, idx, lp))
        entries = kids
    target_kids = entries
    print(f"Ziel-Liste @{path}: {len(target_kids)} Kinder: "
          f"{[k[0] for k in target_kids]}")
    for k in range(1, len(target_kids) + 1):
        # Ziel-Liste mit k Kindern bauen, dann rueckwaerts einwickeln
        inner = build_list_entry(levels[-1][2], [c[1] for c in target_kids[:k]])
        for entries_lvl, idx, lp in reversed(levels[:-1]):
            siblings = [e[1] for e in entries_lvl[:idx]] + [inner]
            inner = build_list_entry(lp, siblings)
        top_entries, top_idx, _ = levels[0]
        if len(levels) == 1:
            body = b"".join(e[1] for e in top[:top_idx]) + inner
        else:
            body = b"".join(e[1] for e in top[:levels[0][1]]) + inner
        (out / f"stage_p{k:02d}.avs").write_bytes(SIG + root_mode + body)


def main():
    src = Path(sys.argv[1])
    out = Path(sys.argv[2])
    path = [int(x) for x in sys.argv[3].split(",")] if len(sys.argv) > 3 else None
    out.mkdir(parents=True, exist_ok=True)
    data = src.read_bytes()
    assert data.startswith(SIG)
    root_mode = data[len(SIG):len(SIG) + 1]
    top = parse_entries(data, len(SIG) + 1, len(data))
    print(f"Top-Level: {len(top)} Entries: {[e[0] for e in top]}")
    if path is not None:
        bisect_path(data, root_mode, top, path, out)
        print("ok ->", out)
        return

    # Stufen T: Top-Level-Praefixe
    for k in range(1, len(top) + 1):
        body = b"".join(e[1] for e in top[:k])
        (out / f"stage_t{k:02d}.avs").write_bytes(SIG + root_mode + body)

    # Stufen L: erste Liste (id -2) mit nur k Kindern, plus alles davor
    for idx, (eid, raw, bs, be) in enumerate(top):
        if eid == -2:
            prefix_entries = b"".join(e[1] for e in top[:idx])
            lp, kids = list_children(data, bs, be)
            print(f"Liste bei Top-Index {idx}: {len(kids)} Kinder: "
                  f"{[k[0] for k in kids]}")
            for k in range(1, len(kids) + 1):
                le = build_list_entry(lp, [c[1] for c in kids[:k]])
                (out / f"stage_l{k:02d}.avs").write_bytes(
                    SIG + root_mode + prefix_entries + le)
            break
    print("ok ->", out)


if __name__ == "__main__":
    main()

# -*- coding: utf-8 -*-
"""Preset-Bisektion (S46): kumulative Teil-Presets aus einem beliebigen .avs.

Aufruf:
  python bisect_avs.py <preset.avs> <zielordner>            # Top-Level + 1. Liste
  python bisect_avs.py <preset.avs> <zielordner> 1,2        # Liste am Index-Pfad
  python bisect_avs.py <preset.avs> <zielordner> --solo     # JEDER Knoten EINZELN
  python bisect_avs.py <preset.avs> <ziel> --solo --raster raster.avs

Erzeugt stage_t<k>.avs (Top-Level-Praefixe), stage_l<k>.avs (erste Liste mit
nur den ersten k Kindern) bzw. stage_p<k>.avs (Pfad-Modus). Die Stufen danach
durch compare_avsref.py jagen -> erste divergente Stufe = Taeter.

SOLO-MODUS (--solo, Vorgabe Patrik S74: "pruefe alle separat, bevor du
Kombinationen testest"): erzeugt solo_t<k>.avs / solo_l<k>.avs mit GENAU EINEM
Knoten. Die kumulativen Stufen beantworten "ab wo laeuft es auseinander" —
aber sie beantworten es falsch, sobald ein frueher Knoten fuer sich schon
abweicht und die Metrik das verschluckt. Genau das ist bei `10_the ring`
passiert: Stufe t01 (SuperScope allein) meldete MAE 0,001, obwohl die Referenz
dort SCHWARZ ist und wir eine Linie zeichnen — eine 1-Pixel-Linie bewegt den
Mittelwert nicht. Der Sprung erschien erst bei t02 und zeigte auf das falsche
Modul. Einzelmessung zuerst, dann Kombinationen. ACHTUNG:
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


def solo_stages(data, root_mode, top, out, raster=b""):
    """Je Knoten EIN Preset, das nur diesen Knoten enthaelt.

    Top-Level-Knoten wandern unveraendert ins Wurzel-Preset. Kinder der ersten
    Liste bleiben in IHRER Liste (mit deren Praefix) — sonst misst man den
    Wechsel des Blend-Kontexts mit statt des Knotens.

    `raster` (Rohbytes der Entries aus make_raster_avs.py) liefert die Quelle.
    Ohne sie ist ein Transform-Knoten nicht messbar: auf der schwarzen
    Startflaeche liefern beide Renderer schwarz, und der Vergleich meldet
    0,000 — was nur „nichts zu sehen" heisst, nicht „geprueft". Beim
    Einzelknoten-Lauf von S74 betraf das die Mehrzahl aller Knoten (Vorgabe
    Patrik: fuer Transform-Knoten ein vordefiniertes Raster).

    **Das Raster gehoert IN die Liste, nicht davor.** Am laufenden Paar
    nachgemessen (S74): vor die Liste gesetzt ist es im Ergebnis nicht mehr zu
    sehen — beide Renderer liefern schwarz, die Liste raeumt die Flaeche unter
    sich weg. Nur als erstes KIND derselben Liste steht die Quelle im selben
    Blend-Kontext wie der Knoten, der gemessen werden soll. Top-Level-Knoten
    bekommen es entsprechend auf Wurzelebene davor.
    """
    namen = []
    for idx, (eid, raw, bs, be) in enumerate(top):
        if eid == -2:
            continue  # Listen kommen ueber ihre Kinder dran
        (out / f"solo_t{idx + 1:02d}.avs").write_bytes(SIG + root_mode + raster + raw)
        namen.append((f"solo_t{idx + 1:02d}", eid))
    for idx, (eid, raw, bs, be) in enumerate(top):
        if eid != -2:
            continue
        lp, kids = list_children(data, bs, be)
        for k, (kid_id, kid_raw, _, _) in enumerate(kids):
            le = build_list_entry(lp, ([raster] if raster else []) + [kid_raw])
            (out / f"solo_l{k + 1:02d}.avs").write_bytes(SIG + root_mode + le)
            namen.append((f"solo_l{k + 1:02d}", kid_id))
        break
    for name, eid in namen:
        print(f"  {name}.avs  <- Effekt-Id {eid}")
    return namen


def main():
    src = Path(sys.argv[1])
    out = Path(sys.argv[2])
    argv = sys.argv[3:]
    solo = "--solo" in argv
    raster = b""
    if "--raster" in argv:
        i = argv.index("--raster")
        rdata = Path(argv[i + 1]).read_bytes()
        assert rdata.startswith(SIG), "Raster ist kein .avs"
        raster = rdata[len(SIG) + 1:]   # Signatur + Wurzel-Modus-Byte weg
        del argv[i:i + 2]
    rest = [a for a in argv if not a.startswith("--")]
    path = [int(x) for x in rest[0].split(",")] if rest else None
    out.mkdir(parents=True, exist_ok=True)
    data = src.read_bytes()
    assert data.startswith(SIG)
    root_mode = data[len(SIG):len(SIG) + 1]
    top = parse_entries(data, len(SIG) + 1, len(data))
    print(f"Top-Level: {len(top)} Entries: {[e[0] for e in top]}")
    if solo:
        solo_stages(data, root_mode, top, out, raster)
        print("ok ->", out)
        return
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

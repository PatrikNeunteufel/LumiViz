#!/usr/bin/env python3
"""Generiert .lvfx-Vorlagen aus den portablen Shadertoy-.glsl-Dateien (S65).

SSOT sind die .glsl-Dateien in diesem Ordner (reiner Shadertoy-Code,
Standard-Uniforms). Dieses Skript wickelt sie in Ein-Node-Chains unter
asset/effectchain/shadertoys/ — dort per Import Browser doppelklicken oder
mit AvsStandalone --auto vermessen. Generierte .lvfx NICHT von Hand editieren.

Konventionen:
  NN_name.glsl                       Single-Pass, iChannel0 = Audio
  NN_name.image.glsl + .bufferA.glsl Multipass: Buffer A liest sich selbst auf
                                     iChannel0 + Audio auf iChannel1; Image
                                     liest Buffer A auf iChannel0

Aufruf (Repo-Root oder hier): python asset/shadertoys/make_lvfx.py
"""

from __future__ import annotations

import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
OUT = HERE.parent / "effectchain" / "shadertoys"

# Kanal-Kodierung des Shadertoy-Nodes (EffectChain.hpp): -1 nichts,
# 0..3 = Buffer A..D, 4 = Audio.
AUDIO = 4
NONE = -1


def chain_doc(name: str, description: str, image_code: str,
              image_input: list[int], buffers: list[dict]) -> dict:
    node: dict = {
        "type": "shadertoy",
        "name": name,
        "description": description,
        "imageInput": image_input,
        "blend": 0,
        "code": image_code,
    }
    if buffers:
        node["buffers"] = buffers
    return {
        "header": {"formatVersion": 1, "generator": "LumiViz make_lvfx (S65)"},
        "root": {"type": "list", "clearEveryFrame": False, "children": [node]},
    }


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    singles = sorted(p for p in HERE.glob("*.glsl")
                     if ".image." not in p.name and ".bufferA." not in p.name)
    images = sorted(HERE.glob("*.image.glsl"))
    count = 0

    for src in singles:
        stem = src.stem
        doc = chain_doc(
            stem,
            "Generiert aus asset/shadertoys/%s (SSOT dort; Shadertoy-portabel)."
            % src.name,
            src.read_text(encoding="utf-8"),
            [AUDIO, NONE, NONE, NONE], [])
        (OUT / (stem + ".lvfx")).write_text(
            json.dumps(doc, ensure_ascii=False, indent=2), encoding="utf-8")
        count += 1

    for img in images:
        stem = img.name.replace(".image.glsl", "")
        buf = HERE / (stem + ".bufferA.glsl")
        if not buf.exists():
            print(f"WARNUNG: {img.name} ohne {stem}.bufferA.glsl — übersprungen")
            continue
        doc = chain_doc(
            stem,
            "Generiert aus asset/shadertoys/%s.{image,bufferA}.glsl (Multipass)."
            % stem,
            img.read_text(encoding="utf-8"),
            [0, NONE, NONE, NONE],
            [{"code": buf.read_text(encoding="utf-8"),
              "input": [0, AUDIO, NONE, NONE]}])
        (OUT / (stem + ".lvfx")).write_text(
            json.dumps(doc, ensure_ascii=False, indent=2), encoding="utf-8")
        count += 1

    print(f"[make_lvfx] {count} Vorlagen -> {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Pyramid-Spiral-Tutorials.

SSOT ist das Tutorial-Markdown (../PyramidSpiral-tutorial.md):
Jeder ```glsl-Block mit `void mainImage` ist ein vollstaendiger Schritt-Shader
(Schritte 1-16 + Anhang A1-A3, in Dokumentreihenfolge). Dieses Skript
extrahiert sie und schreibt je Schritt eine Ein-Node-.lvfx-Chain
(Shadertoy-Node, Muster: asset/shadertoys/make_lvfx.py) in dieses Verzeichnis.

Screenshots fuer das Tutorial (nach ../pyramid_spiral_bilder/):
  AvsStandalone.exe pyramid_spiral_schritte --auto --frames 300 \
      --size 800x450 --out ../pyramid_spiral_bilder
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
TUTORIAL = HIER.parent / "PyramidSpiral-tutorial.md"

# (Dateiname, Beschreibung, braucht Audio auf iChannel0)
SCHRITTE = [
    ("schritt_01", "Schritt 1 - Der erste Pixel: UV-Koordinaten", False),
    ("schritt_02", "Schritt 2 - Zentrieren und Seitenverhaeltnis", False),
    ("schritt_03", "Schritt 3 - Raymarching: Strahl trifft Kugel", False),
    ("schritt_04", "Schritt 4 - Tiefe sichtbar machen", False),
    ("schritt_05", "Schritt 5 - Das Oktaeder", False),
    ("schritt_06", "Schritt 6 - Normalen und erstes Licht", False),
    ("schritt_07", "Schritt 7 - Den Raum wiederholen", False),
    ("schritt_08", "Schritt 8 - Der unendliche Flug", False),
    ("schritt_09", "Schritt 9 - Das Kaleidoskop", False),
    ("schritt_10", "Schritt 10 - Der Twist", False),
    ("schritt_11", "Schritt 11 - Animation", False),
    ("schritt_12", "Schritt 12 - Cosinus-Palette", False),
    ("schritt_13", "Schritt 13 - Fresnel und Specular", False),
    ("schritt_14", "Schritt 14 - Glow", False),
    ("schritt_15", "Schritt 15 - Nebel und Stabilitaet", False),
    ("schritt_16", "Schritt 16 - Politur (das Original)", False),
    ("anhang_a1", "Anhang A1 - Audiosignal sichtbar machen", True),
    ("anhang_a2", "Anhang A2 - Frequenzbaender", True),
    ("anhang_a3", "Anhang A3 - Die Spirale hoert zu", True),
    ("anhang_b1", "Anhang B1 - Formen und Groessen streuen", False),
    ("anhang_b2", "Anhang B2 - Equalizer-Ebene", True),
    ("anhang_b3", "Anhang B3 - Beat-Blitze", True),
]


def main() -> int:
    text = TUTORIAL.read_text(encoding="utf-8")
    bloecke = re.findall(r"```glsl\n(.*?)```", text, re.DOTALL)
    shader = [b for b in bloecke if "void mainImage" in b]
    if len(shader) != len(SCHRITTE):
        print(f"FEHLER: {len(shader)} mainImage-Bloecke gefunden, "
              f"erwartet {len(SCHRITTE)} - Tutorial geaendert?")
        return 1

    for (name, beschreibung, audio), code in zip(SCHRITTE, shader):
        chain = {
            "header": {
                "formatVersion": 1,
                "generator": "LumiViz make_schritte (Pyramid-Spiral-Tutorial)",
            },
            "root": {
                "type": "list",
                "clearEveryFrame": False,
                "children": [
                    {
                        "type": "shadertoy",
                        "name": name,
                        "description": beschreibung
                        + " (generiert aus PyramidSpiral-tutorial.md, SSOT dort)",
                        "imageInput": [4 if audio else -1, -1, -1, -1],
                        "blend": 0,
                        "code": code,
                    }
                ],
            },
        }
        ziel = HIER / f"{name}.lvfx"
        ziel.write_text(
            json.dumps(chain, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        print(f"{ziel.name}: {len(code)} Zeichen"
              + (" [Audio iChannel0]" if audio else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())

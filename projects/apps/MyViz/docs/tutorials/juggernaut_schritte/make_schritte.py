#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Juggernaut-Tutorials.

SSOT ist das Tutorial-Markdown (../Juggernaut-tutorial.md). Anders als beim
Pyramid-Spiral-Tutorial stehen dort ab Schritt 9 nur noch Diffs (Sammelpunkte:
Zwischenstand nach Schritt 10, Gesamtlisting in Schritt 14, Anhang A3 =
Gesamtlisting + Einbau-Diffs) - die Shader lassen sich also nicht mehr per
Regex aus dem Markdown ziehen. Die .glsl-Dateien in diesem Verzeichnis sind
die **materialisierte Rekonstruktion** jedes Schritts (Diffs kumulativ auf den
jeweils vorigen Stand angewandt, Volllistings als Fixpunkte); dieses Skript
verpackt sie je in eine Ein-Node-.lvfx-Chain (Shadertoy-Node, Muster:
pyramid_spiral_schritte/make_schritte.py).

Screenshots fuer das Tutorial (nach ../juggernaut_bilder/, Aufruf aus ../):
  AvsStandalone.exe juggernaut_schritte --auto --frames 300 \
      --size 800x450 --out juggernaut_bilder
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent

# (Dateiname, Beschreibung, braucht Audio auf iChannel0)
SCHRITTE = [
    ("schritt_01", "Schritt 1 - Die Buehne: eine Silhouette, die das Bild sprengt", False),
    ("schritt_02", "Schritt 2 - Das Raymarch-Geruest: die nackte Riesenkugel", False),
    ("schritt_03", "Schritt 3 - Die Kamera winzig, der Moloch riesig", False),
    ("schritt_04", "Schritt 4 - Greebles I: das Panel-Gitter (und smin/smax)", False),
    ("schritt_05", "Schritt 5 - Greebles II: Detail-Oktaven", False),
    ("schritt_06", "Schritt 6 - Die schiefe Achse: der Moloch dreht sich", False),
    ("schritt_07", "Schritt 7 - Licht-Setup DARK: Gegenlicht und Silhouette", False),
    ("schritt_08", "Schritt 8 - Licht-Setup BRIGHTER: warmes Streiflicht", False),
    ("schritt_09", "Schritt 9 - Die STIMMUNGs-Blende (Default: dark)", False),
    ("schritt_10", "Schritt 10 - Positionslichter: der Moloch ist bewohnt", False),
    ("schritt_11", "Schritt 11 - God-Rays I: der volumetrische Glow", False),
    ("schritt_12", "Schritt 12 - God-Rays II: die Streu-Sonne mit 27 Keulen", False),
    ("schritt_13", "Schritt 13 - Die Kamera-Choreografie: Orbit mit Pendeln", False),
    ("schritt_14", "Schritt 14 - Politur: der fertige Shader", False),
    ("anhang_a1", "Anhang A1 - bandLevel und Beat-Gate", True),
    ("anhang_a3", "Anhang A3 - Der Moloch hoert zu", True),
]


def main() -> int:
    fehler = 0
    for name, beschreibung, audio in SCHRITTE:
        quelle = HIER / f"{name}.glsl"
        if not quelle.is_file():
            print(f"FEHLER: {quelle.name} fehlt")
            fehler += 1
            continue
        code = quelle.read_text(encoding="utf-8")
        if "void mainImage" not in code:
            print(f"FEHLER: {quelle.name} ohne mainImage")
            fehler += 1
            continue
        chain = {
            "header": {
                "formatVersion": 1,
                "generator": "LumiViz make_schritte (Juggernaut-Tutorial)",
            },
            "root": {
                "type": "list",
                "clearEveryFrame": False,
                "children": [
                    {
                        "type": "shadertoy",
                        "name": name,
                        "description": beschreibung
                        + " (rekonstruiert aus Juggernaut-tutorial.md, SSOT dort)",
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
    return 1 if fehler else 0


if __name__ == "__main__":
    sys.exit(main())

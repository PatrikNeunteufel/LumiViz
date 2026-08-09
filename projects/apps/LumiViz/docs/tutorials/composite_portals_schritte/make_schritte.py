#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Composite-Portals-Tutorials.

SSOT ist das Tutorial-Markdown (../CompositePortals-tutorial.md). Anders als
beim Pyramid-Spiral-Tutorial zeigt es ab Schritt 4 nur noch Diffs (der letzte
Vollstand bleibt gueltig, Schritt 12 ist das Gesamtlisting als Fixpunkt) -
die vollstaendigen Schritt-Shader liegen deshalb als MATERIALISIERTE
REKONSTRUKTION (`schritt_NN.glsl` / `anhang_*.glsl`) in diesem Verzeichnis:
wortgleich aus den Markdown-Bloecken zusammengesetzt, Diffs kumulativ
angewendet. Wer das Tutorial aendert, zieht zuerst die .glsl nach.
Schritt 11 ist inhaltsgleich mit Schritt 12 (Schritt 12 fuegt nichts hinzu);
Anhang A2 ist ein Katalog ohne Shader und hat darum keine Chain.

Dieses Skript packt jede .glsl in eine Ein-Node-.lvfx-Chain
(Shadertoy-Node, Muster: asset/shadertoys/make_lvfx.py bzw.
../pyramid_spiral_schritte/make_schritte.py).

Screenshots fuer das Tutorial (nach ../composite_portals_bilder/):
  AvsStandalone.exe composite_portals_schritte --auto --frames 300 \
      --size 800x450 --out composite_portals_bilder
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent

# (Dateiname, Beschreibung, braucht Audio auf iChannel0)
SCHRITTE = [
    ("schritt_01", "Schritt 1 - Kondensieren I: das Tunnel-Skelett", False),
    ("schritt_02", "Schritt 2 - Kondensieren II: das Debris-Skelett", False),
    ("schritt_03", "Schritt 3 - Namespacing: zwei Welten, Split-Screen", False),
    ("schritt_04", "Schritt 4 - Das Portal: der Fensterstrahl wechselt die Welt", False),
    ("schritt_05", "Schritt 5 - Massstab & Weltrahmen: das Fenster als Diorama", False),
    ("schritt_06", "Schritt 6 - Portal-Politur: Atmen und Neon-Rahmen", False),
    ("schritt_07", "Schritt 7 - Material-Id: map() lernt zwei Antworten", False),
    ("schritt_08", "Schritt 8 - Der Kristallboden: ein Hoehenfeld im min()", False),
    ("schritt_09", "Schritt 9 - Kristall-Shading: Brechung, Lampen, Absorption", False),
    ("schritt_10", "Schritt 10 - Anti-Aliasing: fwidth und 2x2-Supersampling", False),
    ("schritt_11", "Schritt 11 - Kohaerenz: eine Uhr, eine Palette, ein Tonemapping", False),
    ("schritt_12", "Schritt 12 - Das Gesamtlisting", False),
    ("anhang_a1", "Anhang A1 - Das Beat-Gate", True),
    ("anhang_a3", "Anhang A3 - Das Composite hoert zu", True),
]


def main() -> int:
    fehlt = [n for n, _, _ in SCHRITTE if not (HIER / f"{n}.glsl").is_file()]
    if fehlt:
        print(f"FEHLER: .glsl fehlt fuer: {', '.join(fehlt)} - "
              "Rekonstruktion nachziehen (SSOT: ../CompositePortals-tutorial.md)")
        return 1

    for name, beschreibung, audio in SCHRITTE:
        code = (HIER / f"{name}.glsl").read_text(encoding="utf-8")
        chain = {
            "header": {
                "formatVersion": 1,
                "generator": "LumiViz make_schritte (Composite-Portals-Tutorial)",
            },
            "root": {
                "type": "list",
                "clearEveryFrame": False,
                "children": [
                    {
                        "type": "shadertoy",
                        "name": name,
                        "description": beschreibung
                        + " (aus CompositePortals-tutorial.md, SSOT dort;"
                        " .glsl = materialisierte Rekonstruktion)",
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

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Space-Debris-Tutorials.

SSOT ist das Tutorial-Markdown (../SpaceDebris-tutorial.md). Anders als beim
Pyramid-Spiral-Tutorial sind dessen Code-Bloecke ab Schritt 5/6 im DIFF-Modus
("nur die geaenderten Funktionen") - die Schritte lassen sich also nicht per
Regex extrahieren. Die .glsl-Dateien in diesem Verzeichnis sind deshalb die
MATERIALISIERTE REKONSTRUKTION: jeder Schritt kumulativ zum vollstaendig
kompilierbaren Shader ausgebaut (Schritte 1-4 und 14 woertlich aus dem
Tutorial, 5-13 Diffs eingearbeitet, A3 = Schritt 14 + Einbau-Diffs aus A3(c)).
Abweichungen fuers Rendern (z. B. der Test-Kipp der provisorischen Kamera in
Schritt 7/8) sind in den .glsl-Dateien kommentiert.

Dieses Skript verpackt jede .glsl in eine Ein-Node-.lvfx-Chain
(Shadertoy-Node, Muster: asset/shadertoys/make_lvfx.py).

Screenshots fuer das Tutorial (nach ../space_debris_bilder/):
  AvsStandalone.exe space_debris_schritte --auto --frames 300 \
      --size 800x450 --out space_debris_bilder
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent

# (Dateiname, Beschreibung, braucht Audio auf iChannel0)
SCHRITTE = [
    ("schritt_01", "Schritt 1 - Die Buehne: Sternenhimmel aus der Blickrichtung", False),
    ("schritt_02", "Schritt 2 - Raymarching-Geruest: eine einzelne Kugel", False),
    ("schritt_03", "Schritt 3 - Domain-Repetition: aus einer Kugel wird ein Feld", False),
    ("schritt_04", "Schritt 4 - Ausduennung & Varianz, Zellwand-Klammer", False),
    ("schritt_05", "Schritt 5 - Formbibliothek: Brocken, Platten, Traeger, Ringe", False),
    ("schritt_06", "Schritt 6 - Taumeln: jede Zelle rotiert um ihre eigene Achse", False),
    ("schritt_07", "Schritt 7 - Der Planet: gluehender Grund unter dem Feld", False),
    ("schritt_08", "Schritt 8 - Atmosphaere & Wolken", False),
    ("schritt_09", "Schritt 9 - Hartes Sonnenlicht", False),
    ("schritt_10", "Schritt 10 - Das Gluehen von unten: Planet als zweite Lichtquelle", False),
    ("schritt_11", "Schritt 11 - Blinklichter: Signalfarben je Truemmerteil", False),
    ("schritt_12", "Schritt 12 - Kamera: Drift, Umkehr, Rollen, Nicken", False),
    ("schritt_13", "Schritt 13 - Kamera-Blase und Sternen-Parallaxe", False),
    ("schritt_14", "Schritt 14 - Politur: Dunst, Farbdrift, Tonemapping (fertig)", False),
    ("anhang_a1", "Anhang A1 - Beat-Gate und Signal-Lampen: der Werkzeugtest", True),
    ("anhang_a3", "Anhang A3 - Das Truemmerfeld hoert zu", True),
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
        chain = {
            "header": {
                "formatVersion": 1,
                "generator": "LumiViz make_schritte (Space-Debris-Tutorial)",
            },
            "root": {
                "type": "list",
                "clearEveryFrame": False,
                "children": [
                    {
                        "type": "shadertoy",
                        "name": name,
                        "description": beschreibung
                        + " (rekonstruiert aus SpaceDebris-tutorial.md, SSOT dort)",
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

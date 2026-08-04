#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Crystal-Lights-Tutorials.

SSOT ist das Tutorial-Markdown (../CrystalLights-tutorial.md). Anders als beim
Pyramid-Spiral-Tutorial enthaelt das Markdown ab Schritt 6 bewusst nur noch
Diffs ("GEAENDERT/NEU"-Funktionen) statt Voll-Listings; die .glsl-Dateien in
diesem Verzeichnis sind die materialisierte Rekonstruktion der Diff-Schritte
(kumulativ auf den jeweiligen Vorgaenger angewandt; Schritt 14 = Gesamtlisting
des Tutorials woertlich, Anhang A3 = Gesamtlisting + Einbau-Diffs (a)/(b)/(c)
aus Anhang A). Bei Aenderungen am Tutorial zuerst die .glsl nachziehen, dann
dieses Skript laufen lassen.

Anhang A2 ist im Tutorial ausdruecklich "kein neuer Shader" (Mapping-Katalog,
Tab. 3) und hat daher weder .glsl noch Chain.

Dieses Skript liest die .glsl-Dateien und schreibt je Schritt eine
Ein-Node-.lvfx-Chain (Shadertoy-Node, Schema wie
pyramid_spiral_schritte/make_schritte.py) in dieses Verzeichnis.

Screenshots fuer das Tutorial (aus dem tutorials-Ordner, nach
../crystal_lights_bilder/):
  AvsStandalone.exe crystal_lights_schritte --auto --frames 300 \
      --size 800x450 --out crystal_lights_bilder
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent

# (Dateiname, Beschreibung, braucht Audio auf iChannel0)
SCHRITTE = [
    ("schritt_01", "Schritt 1 - Die Buehne: Bildaufteilung", False),
    ("schritt_02", "Schritt 2 - Die Kamera: Strahlen auf eine Bodenebene", False),
    ("schritt_03", "Schritt 3 - Hoehenfeld-Raymarching", False),
    ("schritt_04", "Schritt 4 - FBM: aus Sinus wird Terrain", False),
    ("schritt_05", "Schritt 5 - Normalen und kaltes Licht", False),
    ("schritt_06", "Schritt 6 - Kristall-Facetten: Voronoi-Platten", False),
    ("schritt_07", "Schritt 7 - Halbliquid: das Liquiditaetsfeld", False),
    ("schritt_08", "Schritt 8 - Luecken im Terrain", False),
    ("schritt_09", "Schritt 9 - Die Leuchtkoerper", False),
    ("schritt_10", "Schritt 10 - Transparenz: Brechung und Absorption", False),
    ("schritt_11", "Schritt 11 - Glow und Sparkle", False),
    ("schritt_12", "Schritt 12 - Isometrie/Perspektive", False),
    ("schritt_13", "Schritt 13 - Die Kamerafahrt", False),
    ("schritt_14", "Schritt 14 - Politur: der fertige Shader", False),
    ("anhang_a1", "Anhang A1 - Das Beat-Gate", True),
    ("anhang_a3", "Anhang A3 - Das Kristallfeld hoert zu", True),
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
                "generator": "LumiViz make_schritte (Crystal-Lights-Tutorial)",
            },
            "root": {
                "type": "list",
                "clearEveryFrame": False,
                "children": [
                    {
                        "type": "shadertoy",
                        "name": name,
                        "description": beschreibung
                        + " (rekonstruiert aus CrystalLights-tutorial.md, SSOT dort)",
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

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Stratospheric-Tunnel-Tutorials.

SSOT ist das Tutorial-Markdown (../StratosphericTunnel-tutorial.md).
Anders als beim Pyramid-Spiral-Tutorial zeigen die Schritte 6-12 dort nur
noch Diffs ("Ab jetzt zeigen die Schritte nur noch die geaenderten bzw.
neuen Funktionen") - die .glsl-Dateien in diesem Verzeichnis sind die
MATERIALISIERTE REKONSTRUKTION dieser Diff-Schritte (Diffs kumulativ auf
Schritt 5 angewandt; anhang_a3 = Gesamtlisting Schritt 13 + die sechs
Audio-Diffs aus A3). Fuer die Voll-Listings (Schritte 1-5, 13, Anhang A1)
prueft dieses Skript, dass die .glsl byte-gleich mit dem Markdown-Block
sind - weicht das Tutorial ab, bricht es ab.

Dieses Skript wickelt jede .glsl in eine Ein-Node-.lvfx-Chain
(Shadertoy-Node, Muster: pyramid_spiral_schritte/make_schritte.py).

Screenshots fuer das Tutorial (nach ../stratospheric_tunnel_bilder/):
  AvsStandalone.exe stratospheric_tunnel_schritte --auto --frames 300 \
      --size 800x450 --out stratospheric_tunnel_bilder
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
TUTORIAL = HIER.parent / "StratosphericTunnel-tutorial.md"

# (Dateiname, Beschreibung, braucht Audio auf iChannel0)
SCHRITTE = [
    ("schritt_01", "Schritt 1 - Die Buehne: der Polar-Blick in die Roehre", False),
    ("schritt_02", "Schritt 2 - Raymarch: der echte Zylinder", False),
    ("schritt_03", "Schritt 3 - Die Wand-Karte und der Scheinwerfer", False),
    ("schritt_04", "Schritt 4 - Roehren und Spanten", False),
    ("schritt_05", "Schritt 5 - Rausch-Relief", False),
    ("schritt_06", "Schritt 6 - Fenster: Loecher in der Wand", False),
    ("schritt_07", "Schritt 7 - Der Aussenraum: Sterne und Horizontgluehen", False),
    ("schritt_08", "Schritt 8 - Neon-Streifen: Licht in den Fugen", False),
    ("schritt_09", "Schritt 9 - Ring-Lichter und einfallendes Fensterlicht", False),
    ("schritt_10", "Schritt 10 - Der Pfad: der Tunnel macht Kurven", False),
    ("schritt_11", "Schritt 11 - Vortrieb mit Umkehr und Banking", False),
    ("schritt_12", "Schritt 12 - Vergabelungen: der Tunnel teilt sich", False),
    ("schritt_13", "Schritt 13 - Politur: der fertige Shader", False),
    ("anhang_a1", "Anhang A1 - bandLevel und Beat-Gate", True),
    ("anhang_a3", "Anhang A3 - Der Tunnel hoert zu", True),
]

# Diese Schritte stehen als Voll-Listing im Markdown (Index in der Reihen-
# folge aller ```glsl-Bloecke MIT void mainImage: 1-5 sind die Bloecke 0-4,
# Schritt 13 ist Block 5, Anhang A1 ist Block 6).
VOLL_LISTINGS = {
    "schritt_01": 0,
    "schritt_02": 1,
    "schritt_03": 2,
    "schritt_04": 3,
    "schritt_05": 4,
    "schritt_13": 5,
    "anhang_a1": 6,
}


def main() -> int:
    text = TUTORIAL.read_text(encoding="utf-8")
    bloecke = re.findall(r"```glsl\n(.*?)```", text, re.DOTALL)
    voll = [b for b in bloecke if "void mainImage" in b]
    if len(voll) != len(VOLL_LISTINGS):
        print(f"FEHLER: {len(voll)} mainImage-Bloecke gefunden, "
              f"erwartet {len(VOLL_LISTINGS)} - Tutorial geaendert?")
        return 1

    rc = 0
    for name, beschreibung, audio in SCHRITTE:
        quelle = HIER / f"{name}.glsl"
        if not quelle.is_file():
            print(f"FEHLER: {quelle.name} fehlt (Rekonstruktion unvollstaendig)")
            rc = 1
            continue
        code = quelle.read_text(encoding="utf-8")

        if name in VOLL_LISTINGS and code != voll[VOLL_LISTINGS[name]]:
            print(f"FEHLER: {quelle.name} weicht vom Markdown-Voll-Listing ab "
                  f"- SSOT ist das Tutorial, .glsl nachziehen!")
            rc = 1
            continue

        chain = {
            "header": {
                "formatVersion": 1,
                "generator": "LumiViz make_schritte (Stratospheric-Tunnel-Tutorial)",
            },
            "root": {
                "type": "list",
                "clearEveryFrame": False,
                "children": [
                    {
                        "type": "shadertoy",
                        "name": name,
                        "description": beschreibung
                        + " (rekonstruiert aus StratosphericTunnel-tutorial.md,"
                        " SSOT dort; Schritte 6-12 und A3 sind materialisierte"
                        " Diff-Schritte)",
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
    return rc


if __name__ == "__main__":
    sys.exit(main())

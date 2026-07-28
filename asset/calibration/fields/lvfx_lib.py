# -*- coding: utf-8 -*-
"""Bausteine fuer .lvfx-Ketten aus Python — und der Untergrund-Vertrag (§9).

Warum ueberhaupt .lvfx und nicht .avs wie die Modul-Sonden: die Felder, um die
es in Strang E geht, sind zum grossen Teil NEU (S53) und haben kein AVS-
Gegenstueck. Ein .avs-Container kann sie nicht tragen. `AvsStandalone` laedt
beide Formate (main.cpp:246), das Messwerkzeug bleibt also dasselbe.

------------------------------------------------------------------ Untergrund
Transformationen (Movement, Blur, Bump, Mirror, Water …) lassen sich nicht auf
Schwarz beurteilen: ohne Bild ist nicht unterscheidbar, ob der Effekt wirkt
oder nichts da war. §9 verlangt dafuer ein "klar definiertes statisches Bild".

Dieses Bild ist NICHT neu erfunden: es ist der Nachbau des Referenzbilds aus
`../avs/make_module_probes.py` (S50) — vier Farbfelder plus Diagonale auf
`0x101010`. Die Farbfelder trennen Kanal- und Spiegelfehler, die Diagonale
zeigt Warps. Erprobt, und ein Befund laesst sich zwischen beiden Sonden-Familien
vergleichen.

Statisch heisst hier woertlich: kein `t`, kein `rand`, kein Audio, kein Beat.
Zwei Laeufe desselben Presets muessen Pixel fuer Pixel dasselbe liefern, sonst
misst das Urteil Rauschen statt Wirkung.
"""
from __future__ import annotations

import json
from pathlib import Path

FORMAT_VERSION = 1
GENERATOR = "LumiViz MultiEffect"

# Grundfarbe des Untergrunds — dunkel, aber nicht schwarz: `drawn()` in
# run_module_probes.py nimmt den haeufigsten Farbwert als Hintergrund, ein
# echtes Schwarz wuerde jeden dunklen Effekt verschlucken.
GRUND_FARBE = 0x101010


def node(typ: str, name: str = "", **params) -> dict:
    """Ein Knoten. `enabled` ist immer true — abgeschaltete Knoten messen nichts."""
    n = {"type": typ, "enabled": True, "name": name or typ}
    n.update(params)
    return n


def chain(*children, **listparams) -> dict:
    """Ein ganzes Kettendokument mit Wurzel-Liste."""
    root = node("list", "Root", children=list(children), clearEveryFrame=False)
    root.update(listparams)
    return {"header": {"formatVersion": FORMAT_VERSION, "generator": GENERATOR},
            "root": root}


def write(path: Path, doc: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(doc, indent=2, ensure_ascii=False) + "\n",
                    encoding="utf-8")


def _balken(y: float, r: int, g: int, b: int) -> dict:
    """Ein waagrechtes Farbfeld — dicke Linie, feste Farbe, keine Zeitabhaengigkeit."""
    return node("superScope", f"Feld {r}{g}{b}",
                initCode="n=2",
                pointCode=f"x=-0.9+i*0.6; y={y}; red={r}; green={g}; blue={b};",
                pointCount=2, renderMode=1, lineWidth=18, colors=[0xFFFFFF],
                spectrumSource=False, colorCycleFrames=0)


def untergrund() -> list[dict]:
    """Die Knoten des statischen Untergrunds, in Zeichenreihenfolge."""
    return [
        node("clear", "Grund", color=GRUND_FARBE, onlyFirst=False, blend=0),
        _balken(-0.6, 1, 0, 0),
        _balken(-0.2, 0, 1, 0),
        _balken(0.2, 0, 0, 1),
        _balken(0.6, 1, 1, 1),
        node("superScope", "Diagonale",
             initCode="n=40",
             pointCode="x=i*1.8-0.9; y=i*1.6-0.8; red=1; green=1; blue=0;",
             pointCount=40, renderMode=1, lineWidth=2, colors=[0xFFFFFF],
             spectrumSource=False, colorCycleFrames=0),
    ]

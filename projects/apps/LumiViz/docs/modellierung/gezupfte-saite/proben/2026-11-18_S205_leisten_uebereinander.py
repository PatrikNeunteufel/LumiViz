"""S205 - KQ31 a: in welchen mitgelieferten Bauplaenen liegen ein Steg und eine Rippe (oder zwei Rippen) uebereinander?
`steifeBei` (InstrumentBody/Platte.hpp) nimmt an einem Punkt nur die erste Leiste, Stege vor Rippen. Das Skript tastet
jede Leiste laengs ihrer Linie ab (Schritt 2 mm laengs, 1 mm quer) und zaehlt die Flaeche, an der eine weitere Leiste
liegt, die der Kern dort nicht rechnet, und deren Volumen (Flaeche mal Hoehe der verdeckten Leiste).

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript>
Schreibt nichts.
"""
import json
import pathlib

import numpy as np

ORDNER = pathlib.Path("asset/loop/instrument_bodies")


def auf(l, p):
    """Hoehe der Leiste am Punkt p oder 0 - wie `aufLeiste` (naechstes Stueck der Linie)."""
    best, bd = 0.0, np.inf
    for k in range(len(l["line"]) - 1):
        a, b = np.array(l["line"][k]), np.array(l["line"][k + 1])
        d = b - a
        l2 = d @ d
        if l2 <= 0:
            continue
        t = min(1.0, max(0.0, ((p - a) @ d) / l2))
        ab = np.hypot(*(p - (a + t * d)))
        if ab >= bd:
            continue
        bd = ab
        breite = l["width"][k] + t * (l["width"][k + 1] - l["width"][k])
        best = l["height"][k] + t * (l["height"][k + 1] - l["height"][k]) if ab <= breite / 2 else 0.0
    return best


for datei in sorted(ORDNER.glob("*.lvinstbodyplan")):
    plan = json.loads(datei.read_text(encoding="utf-8"))
    leisten = [("Steg", l) for l in plan.get("bridges", [])] + [("Rippe", l) for l in plan.get("ribs", [])]
    zeilen, gesamt = [], 0.0
    for i, (art, l) in enumerate(leisten):          # l geht vor allen spaeteren
        flaeche = vol = eigen = 0.0
        for k in range(len(l["line"]) - 1):
            a, b = np.array(l["line"][k]), np.array(l["line"][k + 1])
            lang = np.hypot(*(b - a))
            if lang <= 0:
                continue
            e = (b - a) / lang
            n = np.array([-e[1], e[0]])
            for s in np.arange(0.001, lang, 0.002):
                w = l["width"][k] + s / lang * (l["width"][k + 1] - l["width"][k])
                for q in np.arange(-w / 2 + 0.0005, w / 2, 0.001):
                    p = a + s * e + q * n
                    eigen += 2e-6
                    h = max((auf(m, p) for _, m in leisten[i + 1:]), default=0.0)
                    if h > 0:
                        flaeche += 2e-6
                        vol += 2e-6 * h
        if flaeche > 0:
            zeilen.append(f"    {art} '{l.get('name', i)}': {flaeche * 1e4:7.1f} cm² von {eigen * 1e4:7.1f} cm² verdecken eine weitere Leiste, "
                          f"deren Volumen dort {vol * 1e6:7.1f} cm³")
            gesamt += vol
    print(f"{datei.name}: {len(plan.get('bridges', []))} Stege, {len(plan.get('ribs', []))} Rippen; verdecktes Volumen {gesamt * 1e6:.1f} cm³")
    print("\n".join(zeilen) if zeilen else "    keine Leisten uebereinander")

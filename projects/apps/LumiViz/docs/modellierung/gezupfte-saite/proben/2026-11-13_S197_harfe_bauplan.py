"""Bauplan der Harfe Salvi „Orchestra“ nach Bell 1988 — Probe mit Schaltern (34k-5, S197; KQ1 a, KQ17 a).

    python harfe_bauplan.py <ziel.lvinstbodyplan> [Schalter]

Koordinaten: x quer (Mitte 0), y laengs vom Bassende (0) zum Diskant (1,31 m), Meter. Faser des Bodens quer (x).

Schalter:
  --stufe frei|leisten|kasten   frei: nur die Fichtenplatte (Tab. 3.1/3.2, freier Rand, ohne Hohlraum);
                                leisten: Platte mit Deck- und Zugleiste und den zwei Leisten (Tab. 3.3 ff.);
                                kasten: dazu der Hohlraum mit den sechs Oeffnungen (Tab. 6.1)   (Vorgabe kasten)
  --el <GPa>      E des Bodens laengs der Faser (quer zum Boden)                     (11)
  --et <GPa>      E des Bodens quer zur Faser (laengs des Bodens), wirksam mit Furnier (0.65)
  --g <GPa>       Schubmodul in der Ebene                                              (0.66)
  --dichte <kg/m3>                                                                      (500)
  --zuschlag <mm> Dicke zusaetzlich zur gezeichneten (Furnier 1 mm)                     (1.0)
  --muendung <k>  Muendungskorrektur der Oeffnungen in Radien                           (1.7)
  --volumen <l>   Luft im Kasten                                                        (27)
  --guete <Q>     Guete der Luft                                                        (16)
"""
import json
import sys

LAENGE = 1.31
BREITE_BASS, BREITE_DISKANT = 0.41, 0.10
DICKE_BASS, DICKE_DISKANT = 0.010, 0.003
# Oeffnungen (Tab. 4.1): Flaeche m2, Lippe m; Lage laengs aus Abb. 2.8 (Anteil der Laenge vom Bassende)
OEFFNUNGEN = [(1, 91e-4, 0.016, 0.19), (2, 93e-4, 0.015, 0.356), (3, 77e-4, 0.015, 0.524), (4, 68e-4, 0.013, 0.694),
              (5, 58e-4, 0.012, 0.87), (6, 67e-4, 0.021, 0.03)]
# Zugleiste (Abb. 2.4 d, aus der Zeichnung abgegriffen): y vom Bassende m, Hoehe m, Breite m
ZUGLEISTE = [(0.0, 0.020, 0.032), (0.29, 0.025, 0.036), (0.55, 0.019, 0.039), (0.80, 0.013, 0.034), (1.05, 0.0072, 0.027), (1.28, 0.0056, 0.020)]


def schalter(args, name, vorgabe, typ=float):
    return typ(args[args.index(name) + 1]) if name in args else vorgabe


def breite(y):
    return BREITE_BASS + (BREITE_DISKANT - BREITE_BASS) * y / LAENGE


def baue(args):
    stufe = schalter(args, "--stufe", "kasten", str)
    el, et, g = schalter(args, "--el", 11.0) * 1e9, schalter(args, "--et", 0.65) * 1e9, schalter(args, "--g", 0.66) * 1e9
    dichte = schalter(args, "--dichte", 500.0)
    zuschlag = schalter(args, "--zuschlag", 1.0) / 1000.0
    umriss = [[-BREITE_BASS / 2, 0.0], [BREITE_BASS / 2, 0.0], [BREITE_DISKANT / 2, LAENGE], [-BREITE_DISKANT / 2, LAENGE]]
    schritt = 0.01
    nx, ny = int(round(BREITE_BASS / schritt)) + 1, int(round(LAENGE / schritt)) + 1
    werte = []
    for j in range(ny):
        y = min(j * schritt, LAENGE)
        h = DICKE_BASS + (DICKE_DISKANT - DICKE_BASS) * y / LAENGE + zuschlag
        werte.extend([round(h, 6)] * nx)
    rippen, stege = [], []
    if stufe in ("mitte", "leisten", "kasten"):
        # Deckleiste aussen in der Mitte (Abb. 2.4 b): 131 cm, 33 -> 18 mm breit, 5 -> 3 mm hoch
        stege.append({"name": "cover bar", "material": "beech", "line": [[0.0, 0.005], [0.0, LAENGE - 0.005]],
                      "height": [0.005, 0.003], "width": [0.033, 0.018]})
        # Zugleiste innen in der Mitte (Abb. 2.4 d)
        rippen.append({"name": "straining bar", "material": "beech",
                       "line": [[0.0, max(0.005, p[0])] for p in ZUGLEISTE],
                       "height": [p[1] for p in ZUGLEISTE], "width": [p[2] for p in ZUGLEISTE]})
        # zwei Fichtenleisten (Abb. 2.4 c): 105 cm, 15 -> 7 mm hoch, 11,5 -> 5 mm breit, auf halbem Weg zum Rand
        for seite in ((-1, 1) if stufe != "mitte" else ()):
            y0, y1 = 0.10, 1.15
            rippen.append({"name": "harmonic bar " + ("bass side" if seite < 0 else "treble side"), "material": "spruce",
                           "line": [[seite * breite(y0) / 4, y0], [seite * breite(y1) / 4, y1]],
                           "height": [0.015, 0.007], "width": [0.0115, 0.005]})
    hohlraum = {"volume": 0.0, "endCorrection": schalter(args, "--muendung", 1.7), "q": schalter(args, "--guete", 16.0),
                "source": "probe", "openings": []}
    if stufe == "kasten":
        hohlraum["volume"] = schalter(args, "--volumen", 27.0) / 1000.0
        for nr, flaeche, lippe, anteil in OEFFNUNGEN:
            y = anteil * LAENGE
            hohlraum["openings"].append({"x": 0.0, "y": round(y, 4), "z": round(-breite(y) / 2, 4), "area": flaeche, "lipLength": lippe,
                                         "source": f"aperture {nr} (Bell 1988 Table 4.1; position from Fig. 2.8)"})
    saiten = json.load(open(schalter(args, "--saiten", "", str), encoding="utf-8"))["saiten"] if "--saiten" in args else []
    punkte = [{"key": 100 - k, "x": 0.0, "y": s["ort"], "source": "string " + s["name"]} for k, s in enumerate(saiten)]
    punkte.sort(key=lambda p: p["key"])
    if not punkte:
        punkte = [{"key": 40 + k, "x": 0.0, "y": round(0.1 + 0.1 * k, 3), "source": "probe"} for k in range(12)]
    return {
        "format": "lumiviz-instrument-body-plan", "version": 5, "title": "Concert harp (probe)",
        "description": "Probe.", "source": "Bell 1988", "units": "SI: m, kg/m^3, Pa",
        "materials": [
            {"name": "spruce", "kind": "orthotropic", "density": dichte, "ex": el, "ey": et, "gxy": g, "gxz": 1.2e9, "gyz": 0.042e9,
             "nuxy": 0.3, "lossFactor": 0.0, "source": "probe"},
            {"name": "beech", "kind": "orthotropic", "density": 750.0, "ex": 13.7e9, "ey": 2.24e9, "gxy": 1.61e9, "gxz": 1.06e9,
             "gyz": 0.46e9, "nuxy": 0.3, "lossFactor": 0.0, "source": "assumed: Chabassier, Joly & Chaigne 2013 Table I"}],
        "plate": {"material": "spruce", "grainAngle": 0.0, "outline": umriss, "thickness": 0.006,
                  "thicknessGrid": {"origin": [-BREITE_BASS / 2, 0.0], "step": schritt, "nx": nx, "ny": ny, "values": werte},
                  "holes": []},
        "ribs": rippen, "bridges": stege, "bridgePoints": punkte,
        "listeningPoints": [{"name": "front", "x": 0.0, "y": 0.6, "z": 1.0, "source": "assumed"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off"},
        "damping": {"a": 0.0, "b": 0.0, "c": 40.0, "source": "probe"},
        "cavity": hohlraum,
    }


if __name__ == "__main__":
    plan = baue(sys.argv[2:])
    open(sys.argv[1], "w", encoding="utf-8", newline="\n").write(json.dumps(plan, indent=1) + "\n")
    print(sys.argv[1], f"({len(plan['ribs'])} ribs, {len(plan['bridges'])} bridges, cavity {plan['cavity']['volume'] * 1000:.1f} l, "
          f"{len(plan['cavity']['openings'])} openings)")

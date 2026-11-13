"""Ersatz-Bauplan einer Laute (34k-5, S197; KQ4 a, KQ6 a) aus den Massen der Rauwolf-Laute 1598 — Probe mit Schaltern.

    python laute_bauplan.py <laute_umriss.json> <ziel.lvinstbodyplan> [Schalter]

Schalter (Vorgaben in Klammern):
  --massstab <s>        alle Laengen mal s (1.0); 0.923 = Mensur 0,625 m statt 0,677 m
  --dicke <mm>          Decke (1.5)
  --hoehe <mm>          Balken, Hoehe in der Mitte (14)
  --ende <mm>           Balken, Hoehe an den Enden (5)
  --breite <mm>         Balken (4.5)
  --balken <liste>      Lagen der Balken in mm ab unterem Ende am Original, Komma (Vorgabe unten); ein Balken, der die
                        Rose schneidet, wird an ihrem Rand geteilt
  --klein <mm>          Hoehe der kurzen Balken unter dem Steg (0 = keine)
  --volumen <l>         Hohlraum am Original (aus den Querschnitten: 14.0)
  --muendung <k>        Muendungskorrektur in Radien (1.7)
  --offen <anteil>      offener Anteil der Rose 0..1 (1.0): das Loch bekommt die Flaeche Anteil mal Kreis
  --steg <g>            Stegleiste mit dieser Masse in Gramm (0 = keine)
  --holz sitka|fichte
"""
import json
import math
import sys

BALKEN = [128.0, 190.0, 228.0, 287.4, 347.0, 392.0, 432.0]
STEG = 89.0          # mm, Vorderkante des Stegs ueber dem unteren Ende (Masszahl der Zeichnung)
STEG_LAENGE = 139.0  # mm zwischen den Ohren (Masszahl)
SAITEN_BREITE = 122.5  # mm, Spanne der Saitenloecher (Masszahl)
CHOERE = [(38, "D2"), (41, "F2"), (43, "G2"), (48, "C3"), (53, "F3"), (57, "A3"), (62, "D4"), (67, "G4")]

HOLZ = {
    "sitka": {"name": "spruce", "kind": "orthotropic", "density": 460.0, "ex": 10.34e9, "ey": 0.80e9, "gxy": 0.63e9,
              "gxz": 0.66e9, "gyz": 0.03e9, "nuxy": 0.467, "lossFactor": 0.0,
              "source": "assumed: Sitka spruce as in Abdallah et al. 2026 Table 1 (the wood of the lute is not measured)"},
    "fichte": {"name": "spruce", "kind": "orthotropic", "density": 380.0, "ex": 11.0e9, "ey": 0.65e9, "gxy": 0.66e9,
               "gxz": 1.2e9, "gyz": 0.042e9, "nuxy": 0.26, "lossFactor": 0.0,
               "source": "assumed: spruce as in Chabassier, Joly & Chaigne 2013 Table I (the wood of the lute is not measured)"},
}


def schalter(args, name, vorgabe, typ=float):
    return typ(args[args.index(name) + 1]) if name in args else vorgabe


def baue(masse, args):
    s = schalter(args, "--massstab", 1.0)
    dicke = schalter(args, "--dicke", 1.5) / 1000
    hoehe = schalter(args, "--hoehe", 14.0) / 1000
    ende = schalter(args, "--ende", 5.0) / 1000
    breite = schalter(args, "--breite", 4.5) / 1000
    klein = schalter(args, "--klein", 0.0) / 1000
    volumen = schalter(args, "--volumen", 14.0) / 1000 * s ** 3
    muendung = schalter(args, "--muendung", 1.7)
    offen = schalter(args, "--offen", 1.0)
    steg_g = schalter(args, "--steg", 0.0)
    holz = HOLZ[schalter(args, "--holz", "sitka", str)]
    lagen = [float(v) for v in schalter(args, "--balken", "", str).split(",")] if "--balken" in args else BALKEN

    u = masse["umriss"]
    halb = [(p[0], (p[2] - p[1]) / 2.0) for p in u]           # symmetrisch: halbe Breite je Station (mm)

    def halbe_breite(y):
        for k in range(len(halb) - 1):
            if halb[k][0] <= y <= halb[k + 1][0]:
                t = (y - halb[k][0]) / (halb[k + 1][0] - halb[k][0])
                return halb[k][1] + t * (halb[k + 1][1] - halb[k][1])
        return 0.0

    # Umriss: unten rund (feiner), dann alle 10 mm, oben gerade abgeschnitten (Halsansatz)
    stationen = [0.0, 2.5, 5.0, 7.5, 10.0, 15.0, 20.0, 30.0] + [float(v) for v in range(40, 470, 10)] + [472.5]
    rechts = [(max(halbe_breite(y), 0.0), y) for y in stationen]
    rechts[0] = (0.0, 0.0)
    oben = masse["laenge"]
    punkte = rechts + [(rechts[-1][0], oben), (-rechts[-1][0], oben)] + [(-x, y) for x, y in reversed(rechts[1:])]
    umriss = [[round(x * s / 1000, 5), round(y * s / 1000, 5)] for x, y in punkte]

    rose_y, rose_r = masse["rose"]["x"], masse["rose"]["durchmesser"] / 2.0
    loch_r = rose_r * math.sqrt(offen)
    ecken = 48
    rr = loch_r * math.sqrt(2.0 * math.pi / (ecken * math.sin(2.0 * math.pi / ecken)))
    loch = [[round(rr * math.cos(2 * math.pi * k / ecken) * s / 1000, 6),
             round((rose_y + rr * math.sin(2 * math.pi * k / ecken)) * s / 1000, 6)] for k in range(ecken)]

    def leiste(name, y, x0, x1, h_mitte, h_ende):
        xm = (x0 + x1) / 2
        return {"name": name, "material": "spruce",
                "line": [[round(x0 * s / 1000, 5), round(y * s / 1000, 5)], [round(xm * s / 1000, 5), round(y * s / 1000, 5)],
                         [round(x1 * s / 1000, 5), round(y * s / 1000, 5)]],
                "height": [h_ende * s, h_mitte * s, h_ende * s], "width": [breite * s] * 3}

    rippen = []
    faktoren = [float(v) for v in schalter(args, "--hoehen", "", str).split(",")] if "--hoehen" in args else [1.0] * len(lagen)
    grund = hoehe
    for n, y in enumerate(lagen, 1):
        hoehe = grund * faktoren[n - 1]   # --hoehen: Faktor je Balken auf die Hoehe in der Mitte
        b = halbe_breite(y) - 1.0           # bis an die Muschel (1 mm vor dem Umriss, damit die Linie in der Platte liegt)
        if abs(y - rose_y) < loch_r + 2.0:  # schneidet die Rose: geteilt, 3 mm vor dem Lochrand endend
            a = math.sqrt(max((loch_r + 3.0) ** 2 - (y - rose_y) ** 2, 0.0))
            rippen.append(leiste(f"bar {n} bass", y, -b, -a, hoehe, ende))
            rippen.append(leiste(f"bar {n} treble", y, a, b, hoehe, ende))
        else:
            rippen.append(leiste(f"bar {n}", y, -b, b, hoehe, ende))
    if klein > 0.0:
        # zwei kurze Balken unter dem Steg, quer (Vereinfachung der Faecherbalken und des J-Balkens)
        for n, y in enumerate((35.0, 62.0), 1):
            b = 0.6 * halbe_breite(y)
            rippen.append(leiste(f"small bar {n}", y, -b, b, klein, klein * 0.5))

    stege = []
    if steg_g > 0.0:
        # Birnbaum-Leiste 139 x 12 mm, Hoehe aus der Masse (Dichte 700)
        h = steg_g / 1000.0 / (700.0 * 0.139 * 0.012)
        stege.append({"name": "bridge", "material": "pear",
                      "line": [[round(-STEG_LAENGE / 2 * s / 1000, 5), round((STEG + 6) * s / 1000, 5)],
                               [round(STEG_LAENGE / 2 * s / 1000, 5), round((STEG + 6) * s / 1000, 5)]],
                      "height": [h, h], "width": [0.012 * s, 0.012 * s]})

    werkstoffe = [holz]
    if stege:
        werkstoffe.append({"name": "pear", "kind": "orthotropic", "density": 700.0, "ex": 8.0e9, "ey": 1.0e9, "gxy": 0.8e9,
                           "gxz": 0.8e9, "gyz": 0.3e9, "nuxy": 0.35, "lossFactor": 0.0, "source": "assumed: pearwood"})

    abstand = SAITEN_BREITE / (len(CHOERE) - 1)
    mitte = (len(CHOERE) - 1) / 2.0
    return {
        "format": "lumiviz-instrument-body-plan", "version": 4, "title": "Lute",
        "description": "Substitute plan of a Renaissance lute soundboard with its air cavity (KQ4 a, KQ6 a) - probe.",
        "source": masse["quelle"], "units": "SI: m, kg/m^3, Pa",
        "materials": werkstoffe,
        "plate": {"material": "spruce", "grainAngle": 90.0, "outline": umriss, "thickness": dicke, "holes": [loch]},
        "ribs": rippen, "bridges": stege,
        "bridgePoints": [{"key": taste, "x": round((k - mitte) * abstand * s / 1000, 5), "y": round((STEG + 6) * s / 1000, 5),
                          "source": f"course {len(CHOERE) - k} ({name}, open)"} for k, (taste, name) in enumerate(CHOERE)],
        "listeningPoints": [{"name": "front", "x": 0.0, "y": round(0.25 * s, 4), "z": 0.5, "source": "assumed"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off: the soundboard closes the bowl"},
        "damping": {"a": 0.0, "b": 0.06, "c": 23.0, "source": "probe: as for the oud"},
        "cavity": {"volume": round(volumen, 6), "endCorrection": muendung, "q": 30.0, "source": "probe"},
    }


if __name__ == "__main__":
    plan = baue(json.load(open(sys.argv[1], encoding="utf-8")), sys.argv[3:])
    open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(json.dumps(plan, indent=1) + "\n")
    print(sys.argv[2], f"cavity {plan['cavity']['volume'] * 1000:.2f} l, {len(plan['ribs'])} bars")

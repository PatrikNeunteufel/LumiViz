"""Schreibt den Bauplan des Cembalos `harpsichord.lvinstbodyplan` (Stufe 34k-5, S197; KQ2 a, KQ13 a, KQ14 a).

Aufruf (vom Projekt-Root):
    python -I -B asset/loop/instrument_bodies/make_cembalo.py [--geometrie <json>] [--dicke <json>] [--ziel <ordner>]
                                                              [--tiefe <m>] [--offen <anteil>]

**Massgebend ist die Datei im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie schreiben und
vergleichen. Danach den Koerper neu rechnen (README.md hier).

Der Boden ist der des Cembalos von Johann Daniel Dulcken, Antwerpen 1755 (MK&G Hamburg, 2000.526), nach
R. Bader, N. Plath & K. Kontopidis, „Dulcken 1755 harpsichord …“, arXiv 2511.09037 (2025), CC BY 4.0:
  Abb. 2  Eingabe ihres Rechenmodells: Umriss, Stegstifte 8' und 4', Anhangleiste des 4', Querleiste, vier Rippen
          (gelesen mit `…/gezupfte-saite/proben/2026-11-13_S197_dulcken_geometrie.py`, 14 px je cm)
  Abb. 1  gemessene Dicke des Bodens, 497 Punkte, Farbstufen von 0,5 mm (`…_dulcken_dicke.py`)
  Text    Rippen und Querleiste „about 2 cm“, Stege „1.9 - 1.2 cm“, Anhangleiste 1 cm; Holz der Rechnung:
          E laengs 11 GPa, laengs/quer 8, Dichte 430, Querzahl 0,3; gemessen: Luftresonanz 37 Hz, Abklingzeit des
          Bodens am 8'-Steg 235 ms
Die Saiten des `harpsichord` sind die des Taskin 1769 (KQ2 a) — der Koerper ist der eines anderen Instruments.

Was woher kommt:
  Umriss, Stege, Stifte, Leisten   Abb. 2; die Treppe des Rechenmodells geglaettet
  Dicke                            Abb. 1 als Raster von 2 cm (`plate.thicknessGrid`); wo die Karte nichts zeigt, der
                                   naechste gelesene Wert
  Hoehe der Leisten                Text (2 cm; Anhangleiste 1 cm); Breiten: Anhangleiste aus Abb. 2 (5,5 cm),
                                   Rippen 1 cm, Querleiste 1,5 cm — Annahme
  Stege                            8' 19 mm (Bass) bis 12 mm (Diskant) hoch (Lesart von „1.9 - 1.2 cm“), 13 bis 9 mm
                                   breit; 4' 12 bis 9 mm hoch, 9 bis 7 mm breit; Buche — Annahme
  Holz                             wie die Rechnung der Quelle; Schubmoduln im Verhaeltnis wie bei Chabassier 2013
  Rose                             Lage und Durchmesser (7,7 cm) aus dem Foto unter Abb. 1; offener Anteil — Annahme
  Hohlraum                         Flaeche des Bodens mal Tiefe; das Instrument ist 256 mm hoch (Sammlung online
                                   MK&G), die Tiefe unter dem Boden ist nicht belegt — Annahme 20,7 cm (KQ15 a):
                                   Helmholtz bei starrer Wand 37,0 Hz wie die Rechnung der Quelle; mit dem
                                   nachgiebigen Boden liegt die Luftmode bei 31,7 Hz, 14 % unter den gemessenen 37 Hz
  Stegpunkte                       je Taste des Dulcken (H1 bis d''', 35 bis 86) ein Punkt auf der Linie des 8'-Stegs,
                                   quer gleichmaessig verteilt wie die Stifte des 4'-Stegs (13,0 mm); die neun
                                   Tasten des Taskin ausserhalb (29 bis 34, 87 bis 89) am Punkt der naechsten
                                   Randtaste (KQ16 a)
  Daempfung                        alpha = 13,8 / T60 mit T60 = 235 ms (gemessen am 8'-Steg), fuer alle Frequenzen
"""
import json
import math
import pathlib
import sys

PROBEN = "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/"
GEOMETRIE = PROBEN + "2026-11-13_S197_dulcken_geometrie.json"
DICKE = PROBEN + "2026-11-13_S197_dulcken_dicke.json"
QUELLE = ("R. Bader, N. Plath, K. Kontopidis, Dulcken 1755 harpsichord, arXiv 2511.09037 (2025), CC BY 4.0: Fig. 2 "
          "(geometry), Fig. 1 (measured thickness), both read from the figures; overall size from the online "
          "collection of the Museum fuer Kunst und Gewerbe Hamburg (2000.526)")

TIEFE = 0.207           # m, Hohlraum unter dem Boden — Annahme (KQ15 a): mit ihr gibt die Formel fuer starre Waende
                        # die 37 Hz, die die Quelle aus Lochradius und Volumen selbst rechnet
TASKIN_VON, TASKIN_BIS = 29, 89   # F1 bis f''' — der Umfang des `harpsichord` (KQ2 a)
ROSE = (1.402, 0.209)   # m, Mitte
ROSE_RADIUS = 0.0385    # m
ROSE_OFFEN = 1.0        # Anteil der Kreisflaeche
T60 = 0.235             # s, gemessen (Bader u. a. 2025, 3.1)
ERSTE_TASTE = 35        # H1 (HH), 52 Tasten bis d''' (86)
TASTEN = 52


def ausgeduennt(linie, jeder):
    aus = linie[::jeder]
    if aus[-1] != linie[-1]:
        aus.append(linie[-1])
    return aus


def geglaettet(linie, halb=2):
    """Gleitendes Mittel entlang der Stiftreihe (die Stifte springen im Bild um ein Pixel)."""
    aus = []
    for k in range(len(linie)):
        teil = linie[max(0, k - halb):k + halb + 1]
        aus.append([sum(p[0] for p in teil) / len(teil), sum(p[1] for p in teil) / len(teil)])
    aus[0], aus[-1] = list(linie[0]), list(linie[-1])
    return aus


def leiste(name, material, linie, hoehe_von, hoehe_bis, breite_von, breite_bis):
    n = len(linie)
    return {"name": name, "material": material,
            "line": [[round(p[0], 4), round(p[1], 4)] for p in linie],
            "height": [round(hoehe_von + (hoehe_bis - hoehe_von) * k / (n - 1), 5) for k in range(n)],
            "width": [round(breite_von + (breite_bis - breite_von) * k / (n - 1), 5) for k in range(n)]}


def tasten_am_steg(steg8, steg4, tasten):
    """Je Taste ein Punkt auf der Linie des 8'-Stegs. Die Saiten laufen laengs und liegen quer gleich weit
    auseinander; die Flecken des 8'-Stegs lassen sich im Bild nicht einzelnen Tasten zuordnen (im Bass zwei
    Stifte je Taste, sonst einer). Quer liegt Taste k deshalb dort, wo die Reihe des 4'-Stegs (ein Stift je
    Taste) sie hinlegt: gleichmaessig zwischen ihrem ersten und letzten Stift."""
    y0, y1 = steg4[0][1], steg4[-1][1]
    linie = sorted(steg8, key=lambda p: p[1])
    aus = []
    for k in range(tasten):
        y = y0 + (y1 - y0) * k / (tasten - 1)
        for a, b in zip(linie, linie[1:]):
            if a[1] <= y <= b[1] and b[1] > a[1]:
                t = (y - a[1]) / (b[1] - a[1])
                aus.append([a[0] + t * (b[0] - a[0]), y])
                break
    assert len(aus) == tasten
    return aus


def raster(dicke):
    """Das gelesene Raster in mm -> m; Luecken mit dem naechsten gelesenen Wert gefuellt."""
    nx, ny, w = dicke["nx"], dicke["ny"], dicke["werte"]
    bekannt = [(i, j, w[j][i]) for j in range(ny) for i in range(nx) if w[j][i] is not None]
    werte = []
    for j in range(ny):
        for i in range(nx):
            v = w[j][i]
            if v is None:
                v = min(bekannt, key=lambda b: (b[0] - i) ** 2 + (b[1] - j) ** 2)[2]
            werte.append(round(v / 1000.0, 5))
    return {"origin": [0.0, 0.0], "step": dicke["schritt"], "nx": nx, "ny": ny, "values": werte}


def baue(geo, dicke, tiefe, offen):
    umriss = [[round(p[0], 4), round(p[1], 4)] for p in geo["umriss"]]
    steg8 = geo["steg8"]
    steg4 = geo["steg4"]
    tasten = tasten_am_steg(steg8, steg4, TASTEN)
    ecken = 32
    r = ROSE_RADIUS * math.sqrt(offen) * math.sqrt(2.0 * math.pi / (ecken * math.sin(2.0 * math.pi / ecken)))
    loch = [[round(ROSE[0] + r * math.cos(2 * math.pi * k / ecken), 5), round(ROSE[1] + r * math.sin(2 * math.pi * k / ecken), 5)]
            for k in range(ecken)]
    rippen = [leiste("4' hitchpin rail", "spruce", ausgeduennt(geglaettet(geo["anhangleiste4"]["linie"], 1), 1),
                     0.010, 0.010, geo["anhangleiste4"]["breite"], geo["anhangleiste4"]["breite"]),
              leiste("cutoff bar", "spruce", geo["querleiste"], 0.020, 0.020, 0.015, 0.015)]
    for n, rp in enumerate(geo["rippen"], 1):
        # die Rippe endet 1 cm vor der Querleiste und 5 mm vor der Wand (die Linie muss in der Platte liegen)
        (x0, y0), (x1, y1) = rp
        lang = math.dist(rp[0], rp[1])
        a, b = 0.005 / lang, 1.0 - 0.012 / lang
        rippen.append(leiste(f"rib {n}", "spruce", [[x0 + (x1 - x0) * a, y0 + (y1 - y0) * a], [x0 + (x1 - x0) * b, y0 + (y1 - y0) * b]],
                             0.020, 0.020, 0.010, 0.010))
    stege = [leiste("8' bridge", "beech", ausgeduennt(geglaettet(steg8, 3), 4), 0.019, 0.012, 0.013, 0.009),
             leiste("4' bridge", "beech", ausgeduennt(geglaettet(steg4), 3), 0.012, 0.009, 0.009, 0.007)]
    punkte = [{"key": ERSTE_TASTE + k, "x": round(p[0], 4), "y": round(p[1], 4),
               "source": f"key {k + 1} of 52 of the Dulcken 1755 on the line of the 8' bridge (Bader et al. 2025 Fig. 2, read "
                         "from the figure); across the strings evenly spaced as the pins of the 4' bridge"}
              for k, p in enumerate(tasten)]
    # KQ16 a: die neun Tasten des Taskin ausserhalb des Dulcken bekommen den Punkt der naechsten Randtaste
    rand = "outside the 52 keys of the Dulcken 1755 (KQ16 a): the point of key {} of 52, the nearest one; the soundboard ends there"
    unten = [{"key": t, "x": punkte[0]["x"], "y": punkte[0]["y"], "source": rand.format(1)} for t in range(TASKIN_VON, ERSTE_TASTE)]
    oben = [{"key": t, "x": punkte[-1]["x"], "y": punkte[-1]["y"], "source": rand.format(TASTEN)}
            for t in range(ERSTE_TASTE + TASTEN, TASKIN_BIS + 1)]
    punkte = unten + punkte + oben
    return {
        "format": "lumiviz-instrument-body-plan",
        "version": 11,
        "title": "Harpsichord (soundboard of the Dulcken 1755)",
        "description": "Soundboard of the harpsichord by Johann Daniel Dulcken, Antwerp 1755 (MK&G Hamburg), with its air "
                       "cavity (KQ13 a): outline, both bridges with their pins, the 4' hitchpin rail, cutoff bar and "
                       "four ribs from Fig. 2 of Bader, Plath & Kontopidis 2025, the measured thickness from Fig. 1 as "
                       "a grid. Assumptions: widths of the bars, cross-sections and wood of the bridges, shear moduli, "
                       "open share of the rose, depth of the cavity, rim clamped and case rigid. The strings of the "
                       "instrument `harpsichord` are those of the Taskin 1769 (KQ2 a): another instrument.",
        "source": QUELLE,
        "units": "SI: m, kg/m^3, Pa",
        "materials": [
            {"name": "spruce", "kind": "orthotropic", "density": 430.0,
             "ex": 11.0e9, "ey": 11.0e9 / 8.0, "gxy": 0.66e9, "gxz": 1.2e9, "gyz": 0.042e9, "nuxy": 0.3, "lossFactor": 0.0,
             "source": "Bader et al. 2025, Sect. 2.2 (assumed there: Ex 11 GPa, Ex/Ey 8, density 430, Poisson 0.3); "
                       "shear moduli assumed as in Chabassier, Joly & Chaigne 2013 Table I"},
            {"name": "beech", "kind": "orthotropic", "density": 750.0,
             "ex": 13.7e9, "ey": 2.24e9, "gxy": 1.61e9, "gxz": 1.06e9, "gyz": 0.46e9, "nuxy": 0.3, "lossFactor": 0.0,
             "source": "assumed: beech as in Chabassier, Joly & Chaigne 2013 Table I (the source computes all parts as spruce)"}],
        "plate": {"material": "spruce", "grainAngle": 0.0, "outline": umriss, "thickness": 0.0043,
                  "thicknessGrid": raster(dicke), "holes": [loch]},
        "ribs": rippen,
        "bridges": stege,
        "bridgePoints": punkte,
        "listeningPoints": [{"name": "above", "x": 0.9, "y": 0.3, "z": 1.0,
                             "source": "assumed: 1 m above the soundboard (as the microphones in Valimaki et al. 2004)"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off: the soundboard closes the case"},
        "damping": {"a": 0.0, "b": 0.0, "c": round(6.0 * math.log(10.0) / T60, 2),
                    "source": "decay time of the soundboard measured at the 8' bridge, T60 = 235 ms (Bader et al. 2025, "
                              "Sect. 3.1): alpha = 6 ln 10 / T60 for all frequencies"},
        "cavity": {"volume": round(geo["flaeche"] * tiefe, 5), "endCorrection": 1.7, "q": 30.0,
                   "source": f"area of the soundboard ({geo['flaeche']} m2, Fig. 2) times an assumed depth of {tiefe} m "
                             "(the instrument is 256 mm high); rose 7.7 cm wide (photo under Fig. 1); end correction and "
                             "Q of the air assumed. Measured air resonance: 37 Hz (Bader et al. 2025, Sect. 3.2); with this "
                             "depth the rigid-wall Helmholtz frequency is 37 Hz as the source computes it, the mode coupled "
                             "to the soundboard lies 14 % lower (KQ15 a: known deviation)",
                   "openings": [], "kind": "singlePressure", "tube": {"sections": [], "q": 30.0, "source": ""},
                   "openingModel": "endCorrection", "surroundings": "wall",
                   "innerLoss": {"openings": [], "points": [], "source": ""}},
        "rim": {"support": "clamped", "rotationalStiffness": 0.0, "source": "assumed: rim clamped (every plan up to version 5 was computed this way)", "sections": []},
        "barOverlap": "first",
        "pointSprings": [],
    }


if __name__ == "__main__":
    args = sys.argv[1:]

    def wert(name, vorgabe):
        return args[args.index(name) + 1] if name in args else vorgabe

    geo = json.load(open(wert("--geometrie", GEOMETRIE), encoding="utf-8-sig"))
    dicke = json.load(open(wert("--dicke", DICKE), encoding="utf-8-sig"))
    ziel = pathlib.Path(wert("--ziel", str(pathlib.Path(__file__).resolve().parent)))
    plan = baue(geo, dicke, float(wert("--tiefe", TIEFE)), float(wert("--offen", ROSE_OFFEN)))
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "harpsichord.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    print(datei, f"(cavity {plan['cavity']['volume'] * 1000:.1f} l, {len(plan['bridgePoints'])} bridge points, alpha {plan['damping']['c']} 1/s)")

"""Schreibt den Bauplan der Oud `oud.lvinstbodyplan` (Stufe 34k-5; S196: KQ5 a, KQ6 a, KQ10 a; neu gesucht in S209:
KQ38 a, KQ41 a, KQ42 a–KQ45 a).

Aufruf (vom Projekt-Root):
    python -I -B asset/loop/instrument_bodies/make_oud.py [--umriss <oud_umriss.json>] [--ziel <ordner>]

**Massgebend ist die Datei im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie schreiben und
vergleichen. Danach den Koerper neu rechnen (README.md hier).

Quelle: M. Abdallah, E. Dreik, A. Sassi, M. Paurobally, S. Sassi, „Vibroacoustic sensitivity of an oud soundboard
to bracing geometry“, Meccanica 61:89 (2026), CC BY 4.0. Ein **Ersatz-Bauplan** (KQ6 a): belegte Masse gehen ein,
jedes fehlende Mass steht als Annahme mit Vermerk im Bauplan. Die Annahmen sind so gesetzt, dass die ersten fuenf
Moden die fuenf Spitzen der gemessenen Kurve (Abb. 6: 139 / 203 / 280 / 362 / 385 Hz) treffen, **wenn der
Aufnehmer der Messung (5,8 g am Antwortpunkt 3) mitgerechnet wird** (KQ43 a). Der Bauplan beschreibt das Instrument
ohne Aufnehmer: seine Moden liegen bei 141 / 214 / 275 / 373 / 389 Hz (mit Aufnehmer 139 / 203 / 275 / 351 / 389 Hz:
+0,3 / +0,2 / −1,9 / −3,0 / +1,1 %).
Lesart (KQ42 a): 139 Hz ist die Mode der Luft, 203 Hz die erste Deckenmode; die Spitze bei 101 Hz hat kein
Gegenstueck (vermutlich Hals oder Schale, die hier starr sind).
Herleitung: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-19_S207_spruenge_je_taste.md` §12;
Suchgeruest `…/proben/2026-11-19_S209_oud_*`; der erste Stand `…/2026-11-13_S196_oud_zweiter_fall.md`.

Was woher kommt:
  Umriss            aus Abb. 3 gelesen (S195, `…/proben/2026-11-12_S195_oud_umriss.json`; Breite 37, Laenge 50 cm)
  Holz              Tab. 1 (Sitka-Fichte, orthotrop; die Quelle rechnet alles aus einem Werkstoff)
  Balken            Lage aus Abb. 3 gelesen, Masse Tab. 2. **Enden 2 mm vor dem Rand — Annahme (KQ42 a):** Abb. 3
                    zeichnet sie 1,0–1,65 cm davor; so gebaut liegt der Bauch der strahlenden Mode im Lochfeld
                    statt am Steg. Die inneren Enden der Balken 4 und 5 am grossen Loch wie gezeichnet
  Loecher           Radien nach den Masszahlen in Abb. 3 (R 5,3 und R 2,0 cm), Lage aus Abb. 3 gelesen
  Rosetten          Abb. 4 und 5 a zeigen Rosetten in allen drei Loechern; ihre Masse ist nicht belegt.
                    **Annahme (KQ44 a):** Masse am Lochrand, 11,4 g am grossen und je 1 g an den kleinen Loechern,
                    als weicher Ring (5 mm breit, Mitte 3 mm ausserhalb des Lochrands, 10 mm hoch, E 10 MPa, die
                    Dichte aus der Masse) — traegt Masse und fast keine Steife
  Schale            21 cm tief (Masszahl in Abb. 3); Annahme: jeder Querschnitt eine halbe Ellipse, ihre Tiefe
                    proportional zur Breite der Decke an der Stelle
  Deckendicke       Annahme 2,23 mm (nicht in der Quelle; angepasst)
  Steg              Lage 41 cm unter dem oberen Ende des Korpus (Masszahl in Abb. 3), Laenge 17 cm aus Abb. 3
                    gelesen; Querschnitt 25,8 x 8,6 mm bei 650 kg/m3 und E 10 GPa (24,5 g) — Annahme, angepasst
  Stegpunkte        je Chor einer, Taste = die leere Saite in arabischer Stimmung C2 F2 A2 D3 G3 C4 (D'Addario
                    EJ95A, KQ5 a); Abstand der Choere 11 mm — Annahme
  Daempfung         aus den Halbwertsbreiten der frei stehenden Spitzen in Abb. 6 (139 / 280 / 362 / 385 Hz:
                    η 3,4 / 3,3 / 1,6 / 1,5 %; `…/proben/2026-11-13_S196_oud_abb6_daempfung.py`), kleinste Quadrate
                    fuer α = b·f + c; ueber 400 Hz fortgeschrieben
  Luft im Hohlraum  Muendungskorrektur 1,52 Radien (angepasst; mit den Rosetten in den Loechern), Guete 30 gesetzt
"""
import json
import math
import pathlib
import sys

UMRISS = "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-12_S195_oud_umriss.json"
QUELLE = ("M. Abdallah, E. Dreik, A. Sassi, M. Paurobally, S. Sassi, Vibroacoustic sensitivity of an oud soundboard "
          "to bracing geometry, Meccanica 61:89 (2026), CC BY 4.0")

DICKE = 0.00223       # m, Annahme (KQ45 a; angepasst)
TIEFE = 0.21          # m, Schale (Abb. 3)
STEG_Y = 0.09         # m vom unteren Ende: 50 − 41 cm (Abb. 3)
STEG = (0.17, 0.0258, 0.0086)  # m: Laenge (Abb. 3), Breite, Hoehe (Annahme, angepasst)
STEG_DICHTE = 650.0   # kg/m3, Annahme
STEG_E = 10.0e9       # Pa, Annahme
MUENDUNG = 1.52       # Radien, angepasst
RAND_ABSTAND = 0.002  # m, Balkenenden vor dem Rand (KQ42 a, Annahme)
CHOR_ABSTAND = 0.011  # m, Annahme
LEERE_SAITEN = [(36, "C2"), (41, "F2"), (45, "A2"), (50, "D3"), (55, "G3"), (60, "C4")]

BALKEN = [  # Name, y cm, x von, x bis (cm; None = bis RAND_ABSTAND vor den Rand), c mm, Breite mm (Tab. 2)
    ("brace 1", 4.11, None, None, 10.0, 4.2),
    ("brace 2", 17.53, None, None, 8.0, 4.2),
    ("brace 3", 23.68, None, None, 7.0, 5.0),
    ("brace 5", 30.05, None, -5.8, 7.0, 6.6),
    ("brace 4", 30.05, 5.8, None, 7.0, 6.6),
    ("brace 6", 36.38, None, None, 10.0, 4.2),
    ("brace 7", 39.82, None, None, 10.0, 4.2),
]
BALKEN_MITTE = 16.5  # mm, Hoehe b in der Mitte (Tab. 2)

LOECHER = [  # x cm, y cm, Radius cm, Ecken
    (0.0, 30.0, 5.3, 48),
    (-10.3, 20.4, 2.0, 24),
    (10.3, 20.4, 2.0, 24),
]
ROSETTEN = [("rosette", 11.4), ("rosette left", 1.0), ("rosette right", 1.0)]  # Name, Masse g (KQ44 a), je Loch
RING = (0.003, 0.005, 0.010, 1.0e7, 48)  # m ausserhalb des Lochrands, Breite, Hoehe, E in Pa, Ecken


def halbe_breite(umriss, y):
    """Halbe Breite der Decke auf der Hoehe y (m), aus dem Umriss."""
    xs = []
    for a, b in zip(umriss, umriss[1:] + umriss[:1]):
        if (a[1] - y) * (b[1] - y) <= 0 and a[1] != b[1]:
            xs.append(a[0] + (y - a[1]) / (b[1] - a[1]) * (b[0] - a[0]))
    return max(xs)


def leiste(umriss, name, y, x0, x1, c, breite):
    rand = halbe_breite(umriss, y / 100) - RAND_ABSTAND
    a = -rand if x0 is None else x0 / 100
    b = rand if x1 is None else x1 / 100
    return {"name": name, "material": "sitka",
            "line": [[round(a, 5), y / 100], [round((a + b) / 2, 5), y / 100], [round(b, 5), y / 100]],
            "height": [c / 1000, BALKEN_MITTE / 1000, c / 1000],
            "width": [breite / 1000] * 3}


def loch(x, y, r, ecken):
    # Vieleck mit der Flaeche des Kreises: R² · n/2 · sin(2π/n) = π r²
    radius = r / 100.0 * math.sqrt(2.0 * math.pi / (ecken * math.sin(2.0 * math.pi / ecken)))
    return [[round(x / 100.0 + radius * math.cos(2.0 * math.pi * k / ecken), 6),
             round(y / 100.0 + radius * math.sin(2.0 * math.pi * k / ecken), 6)] for k in range(ecken)]


def ring_radius(r):
    return r / 100.0 + RING[0]


def rosette_werkstoff(name, gramm, r):
    _, breite, hoehe, e, _ = RING
    dichte = gramm / 1000.0 / (2.0 * math.pi * ring_radius(r) * breite * hoehe)
    return {"name": name, "kind": "orthotropic", "density": round(dichte, 2),
            "ex": e, "ey": e, "gxy": 0.4 * e, "gxz": 0.4 * e, "gyz": 0.4 * e, "nuxy": 0.3, "lossFactor": 0.0,
            "source": f"assumed (KQ44 a): carries the mass of the rosette in the hole ({gramm:g} g, not given in the "
                      "source) as a soft ring at the rim of the hole; the density follows from the mass, the modulus "
                      "is set so low that the ring adds almost no stiffness"}


def rosette_leiste(name, x, y, r):
    _, breite, hoehe, _, ecken = RING
    rr = ring_radius(r)
    linie = [[round(x / 100.0 + rr * math.cos(2.0 * math.pi * k / ecken), 6),
              round(y / 100.0 + rr * math.sin(2.0 * math.pi * k / ecken), 6)] for k in range(ecken + 1)]
    return {"name": name, "material": name, "line": linie, "height": [hoehe] * (ecken + 1), "width": [breite] * (ecken + 1)}


def baue(masse):
    # Umriss: rechts hinauf, links herunter; x quer (Mitte 0), y laengs vom unteren Ende, Meter
    rechts = [(u[2] / 100.0, u[0] / 100.0) for u in masse["umriss"]]
    links = [(u[1] / 100.0, u[0] / 100.0) for u in reversed(masse["umriss"])]
    umriss = [[round(x, 5), round(y, 5)] for x, y in rechts + links]

    # Volumen der Schale: V = (π/2) · (Tiefe / halbe groesste Breite) · ∫ b(y)² dy, b = halbe Breite
    halb = [(u[0] / 100.0, (u[2] - u[1]) / 200.0) for u in masse["umriss"]]
    groesste = max(b for _, b in halb)
    integral = sum((halb[k + 1][0] - halb[k][0]) * (halb[k][1] ** 2 + halb[k + 1][1] ** 2) / 2.0 for k in range(len(halb) - 1))
    volumen = math.pi / 2.0 * (TIEFE / groesste) * integral

    mitte = (len(LEERE_SAITEN) - 1) / 2.0
    steg_g = STEG[0] * STEG[1] * STEG[2] * STEG_DICHTE * 1000.0
    return {
        "format": "lumiviz-instrument-body-plan",
        "version": 11,
        "title": "Oud",
        "description": "Substitute plan of an Arabic oud soundboard with its air cavity (KQ6 a; searched anew in S209: "
                       "KQ42 a-KQ45 a). Outline, brace positions, sound holes, bowl depth, bridge position and length "
                       "from Abdallah et al. 2026 Fig. 3; brace sizes Table 2; wood Table 1. Assumptions: plate "
                       "thickness 2.23 mm, braces ending 2 mm before the rim (Fig. 3 draws 10-16.5 mm), cross-section "
                       "and wood of the bridge, mass of the rosettes in the three holes (carried as soft rings at the "
                       "rims of the holes), cross-section of the bowl, end correction of the holes, rim clamped and "
                       "bowl rigid (as in the source's FE), spacing of the courses at the bridge. Set so that the "
                       "first five modes meet the peaks of the measured response (Fig. 6: 139 / 203 / 280 / 362 / "
                       "385 Hz) within 3 % when the accelerometer of the measurement (5.8 g at response point 3) is "
                       "added; the plan describes the instrument without it (141 / 214 / 275 / 373 / 389 Hz). 139 Hz "
                       "is the air mode, 203 Hz the first mode of the soundboard; the measured peak at 101 Hz has no "
                       "counterpart (neck and bowl are rigid here).",
        "source": QUELLE,
        "units": "SI: m, kg/m^3, Pa",
        "materials": [{
            "name": "sitka", "kind": "orthotropic", "density": 460.0,
            "ex": 10.34e9, "ey": 0.80e9, "gxy": 0.63e9, "gxz": 0.66e9, "gyz": 0.03e9, "nuxy": 0.467,
            "lossFactor": 0.0,
            "source": "Abdallah et al. 2026 Table 1 (x = L, y = R, z = T)"}, {
            "name": "bridgewood", "kind": "orthotropic", "density": STEG_DICHTE,
            "ex": STEG_E, "ey": STEG_E, "gxy": round(STEG_E / 2.6, 1), "gxz": round(STEG_E / 2.6, 1), "gyz": round(STEG_E / 2.6, 1),
            "nuxy": 0.3, "lossFactor": 0.0,
            "source": "assumed: a hardwood bridge taken as isotropic (density and modulus not given in the source)"}]
                     + [rosette_werkstoff(name, gramm, l[2]) for (name, gramm), l in zip(ROSETTEN, LOECHER)],
        "plate": {"material": "sitka", "grainAngle": 90.0, "outline": umriss, "thickness": DICKE,
                  "holes": [loch(*l) for l in LOECHER]},
        "ribs": [leiste(umriss, *b) for b in BALKEN]
                + [rosette_leiste(name, l[0], l[1], l[2]) for (name, _), l in zip(ROSETTEN, LOECHER)],
        "bridges": [{"name": "bridge", "material": "bridgewood",
                     "line": [[-STEG[0] / 2, STEG_Y], [0.0, STEG_Y], [STEG[0] / 2, STEG_Y]],
                     "height": [STEG[2]] * 3, "width": [STEG[1]] * 3}],
        "bridgePoints": [{"key": taste, "x": round((k - mitte) * CHOR_ABSTAND, 5), "y": STEG_Y,
                          "source": f"course {len(LEERE_SAITEN) - k} ({name}, open string; Arabic tuning, D'Addario EJ95A); "
                                    "bridge 41 cm below the top of the body (Fig. 3); courses 11 mm apart (assumed)"}
                         for k, (taste, name) in enumerate(LEERE_SAITEN)],
        "listeningPoints": [{"name": "front", "x": 0.0, "y": 0.25, "z": 0.5, "source": "assumed: 0.5 m in front of the soundboard"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off: the soundboard closes the bowl"},
        "damping": {"a": 0.0, "b": 0.06, "c": 23.0,
                    "source": "half-power widths of the isolated peaks of the measured response (Abdallah et al. 2026 "
                              "Fig. 6, read from the figure): eta 3.4 / 3.3 / 1.6 / 1.5 % at 139 / 280 / 362 / 385 Hz; "
                              "least squares for alpha = b f + c; extrapolated above 400 Hz"},
        "cavity": {"volume": round(volumen, 6), "endCorrection": MUENDUNG, "q": 30.0,
                   "source": "bowl 21 cm deep (Abdallah et al. 2026 Fig. 3, side view); assumed: every cross-section half "
                             "an ellipse with its depth proportional to the width of the soundboard there; end correction "
                             "fitted to the measured air mode at 139 Hz (the holes carry rosettes), Q of the air assumed",
                   "openings": [], "kind": "singlePressure", "tube": {"sections": [], "q": 30.0, "source": ""},
                   "openingModel": "endCorrection", "surroundings": "wall",
                   "innerLoss": {"openings": [], "points": [], "source": ""}},
        "rim": {"support": "clamped", "rotationalStiffness": 0.0, "source": "rim clamped and bowl rigid, as in the FE model of the source (Abdallah et al. 2026); assumed", "sections": []},
        "barOverlap": "first",
        "pointSprings": [],
        "_steg_g": steg_g,
    }


if __name__ == "__main__":
    args = sys.argv[1:]
    umriss = args[args.index("--umriss") + 1] if "--umriss" in args else UMRISS
    ziel = pathlib.Path(args[args.index("--ziel") + 1]) if "--ziel" in args else pathlib.Path(__file__).resolve().parent
    plan = baue(json.load(open(umriss, encoding="utf-8-sig")))
    steg_g = plan.pop("_steg_g")
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "oud.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    print(datei, f"(cavity {plan['cavity']['volume'] * 1000:.2f} l, bridge {steg_g:.1f} g)")

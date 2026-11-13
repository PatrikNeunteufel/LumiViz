"""Schreibt den Bauplan der Oud `oud.lvinstbodyplan` (Stufe 34k-5, S196; KQ5 a, KQ6 a, KQ10 a).

Aufruf (vom Projekt-Root):
    python -I -B asset/loop/instrument_bodies/make_oud.py [--umriss <oud_umriss.json>] [--ziel <ordner>]

**Massgebend ist die Datei im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie schreiben und
vergleichen. Danach den Koerper neu rechnen (README.md hier).

Quelle: M. Abdallah, E. Dreik, A. Sassi, M. Paurobally, S. Sassi, „Vibroacoustic sensitivity of an oud soundboard
to bracing geometry“, Meccanica 61:89 (2026), CC BY 4.0. Ein **Ersatz-Bauplan** (KQ6 a): belegte Masse gehen ein,
jedes fehlende Mass steht als Annahme mit Vermerk im Bauplan; die Annahmen sind so gesetzt, dass die gerechneten
Moden die Spitzen der gemessenen Kurve (Abb. 6) treffen (KQ10 a: 101 / 203 / 280 / 362 Hz innerhalb von 3 %).
Herleitung und Gegenproben: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S196_oud_zweiter_fall.md`.

Was woher kommt:
  Umriss            aus Abb. 3 gelesen (S195, `…/proben/2026-11-12_S195_oud_umriss.json`; Breite 37, Laenge 50 cm)
  Holz              Tab. 1 (Sitka-Fichte, orthotrop; die Quelle rechnet alles aus einem Werkstoff)
  Balken            Lage und Enden aus Abb. 3 gelesen (sie enden 1,3–1,5 cm vor dem Rand, KQ10 a), Masse Tab. 2
  Loecher           Radien nach den Masszahlen in Abb. 3 (R 5,3 und R 2,0 cm), Lage aus Abb. 3 gelesen
  Schale            21 cm tief (Masszahl in Abb. 3); Annahme: jeder Querschnitt eine halbe Ellipse, ihre Tiefe
                    proportional zur Breite der Decke an der Stelle
  Deckendicke       Annahme 1,5 mm (nicht in der Quelle; KQ10 a)
  Steg              Lage 41 cm unter dem oberen Ende des Korpus (Masszahl in Abb. 3); keine Stegleiste (Masse nicht
                    belegt, Gegenprobe im Bericht §8)
  Stegpunkte        je Chor einer, Taste = die leere Saite in arabischer Stimmung C2 F2 A2 D3 G3 C4 (D'Addario
                    EJ95A, KQ5 a); Abstand der Choere 11 mm — Annahme
  Daempfung         aus den Halbwertsbreiten der frei stehenden Spitzen in Abb. 6 (139 / 280 / 362 / 385 Hz:
                    η 3,4 / 3,3 / 1,6 / 1,5 %; `…/proben/2026-11-13_S196_oud_abb6_daempfung.py`), kleinste Quadrate
                    fuer α = b·f + c; ueber 400 Hz fortgeschrieben
  Luft im Hohlraum  Muendungskorrektur 1,7 Radien (Loch in duenner, breiter Wand), Guete 30 — beide gesetzt
"""
import json
import math
import pathlib
import sys

UMRISS = "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-12_S195_oud_umriss.json"
QUELLE = ("M. Abdallah, E. Dreik, A. Sassi, M. Paurobally, S. Sassi, Vibroacoustic sensitivity of an oud soundboard "
          "to bracing geometry, Meccanica 61:89 (2026), CC BY 4.0")

DICKE = 0.0015        # m, Annahme (KQ10 a)
TIEFE = 0.21          # m, Schale (Abb. 3)
STEG_Y = 0.09         # m vom unteren Ende: 50 − 41 cm (Abb. 3)
CHOR_ABSTAND = 0.011  # m, Annahme
LEERE_SAITEN = [(36, "C2"), (41, "F2"), (45, "A2"), (50, "D3"), (55, "G3"), (60, "C4")]

BALKEN = [  # Name, y cm, x von, x bis (cm, aus Abb. 3 gelesen), c mm, Breite mm (Tab. 2)
    ("brace 1", 4.11, -13.0, 13.0, 10.0, 4.2),
    ("brace 2", 17.53, -17.0, 17.0, 8.0, 4.2),
    ("brace 3", 23.68, -16.85, 16.85, 7.0, 5.0),
    ("brace 5", 30.05, -15.6, -5.8, 7.0, 6.6),
    ("brace 4", 30.05, 5.8, 15.6, 7.0, 6.6),
    ("brace 6", 36.38, -12.75, 12.75, 10.0, 4.2),
    ("brace 7", 39.82, -11.0, 11.0, 10.0, 4.2),
]
BALKEN_MITTE = 16.5  # mm, Hoehe b in der Mitte (Tab. 2)

LOECHER = [  # x cm, y cm, Radius cm, Ecken
    (0.0, 30.0, 5.3, 48),
    (-10.3, 20.4, 2.0, 24),
    (10.3, 20.4, 2.0, 24),
]


def leiste(name, y, x0, x1, c, breite):
    xm = (x0 + x1) / 2
    return {"name": name, "material": "sitka",
            "line": [[x0 / 100, y / 100], [xm / 100, y / 100], [x1 / 100, y / 100]],
            "height": [c / 1000, BALKEN_MITTE / 1000, c / 1000],
            "width": [breite / 1000] * 3}


def loch(x, y, r, ecken):
    # Vieleck mit der Flaeche des Kreises: R² · n/2 · sin(2π/n) = π r²
    radius = r / 100.0 * math.sqrt(2.0 * math.pi / (ecken * math.sin(2.0 * math.pi / ecken)))
    return [[round(x / 100.0 + radius * math.cos(2.0 * math.pi * k / ecken), 6),
             round(y / 100.0 + radius * math.sin(2.0 * math.pi * k / ecken), 6)] for k in range(ecken)]


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
    return {
        "format": "lumiviz-instrument-body-plan",
        "version": 4,
        "title": "Oud",
        "description": "Substitute plan of an Arabic oud soundboard with its air cavity (KQ6 a, KQ10 a). Outline, brace "
                       "positions and ends, sound holes, bowl depth and bridge position from Abdallah et al. 2026 Fig. 3; "
                       "brace sizes Table 2; wood Table 1. Assumptions: plate thickness 1.5 mm (not given in the source), "
                       "cross-section of the bowl, no bridge bar, rim clamped and bowl rigid (as in the source's FE), "
                       "hole rims free, spacing of the courses at the bridge. Set so that the computed modes meet the "
                       "peaks of the measured response (Fig. 6): 101 / 203 / 280 / 362 Hz within 3 %; the measured peak "
                       "at 139 Hz has no counterpart.",
        "source": QUELLE,
        "units": "SI: m, kg/m^3, Pa",
        "materials": [{
            "name": "sitka", "kind": "orthotropic", "density": 460.0,
            "ex": 10.34e9, "ey": 0.80e9, "gxy": 0.63e9, "gxz": 0.66e9, "gyz": 0.03e9, "nuxy": 0.467,
            "lossFactor": 0.0,
            "source": "Abdallah et al. 2026 Table 1 (x = L, y = R, z = T)"}],
        "plate": {"material": "sitka", "grainAngle": 90.0, "outline": umriss, "thickness": DICKE,
                  "holes": [loch(*l) for l in LOECHER]},
        "ribs": [leiste(*b) for b in BALKEN],
        "bridges": [],
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
        "cavity": {"volume": round(volumen, 6), "endCorrection": 1.7, "q": 30.0,
                   "source": "bowl 21 cm deep (Abdallah et al. 2026 Fig. 3, side view); assumed: every cross-section half "
                             "an ellipse with its depth proportional to the width of the soundboard there; end correction "
                             "of a hole in a thin wide wall and Q of the air assumed"},
    }


if __name__ == "__main__":
    args = sys.argv[1:]
    umriss = args[args.index("--umriss") + 1] if "--umriss" in args else UMRISS
    ziel = pathlib.Path(args[args.index("--ziel") + 1]) if "--ziel" in args else pathlib.Path(__file__).resolve().parent
    plan = baue(json.load(open(umriss, encoding="utf-8-sig")))
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "oud.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    print(datei, f"(cavity {plan['cavity']['volume'] * 1000:.2f} l)")

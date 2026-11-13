"""Schreibt den Ersatz-Bauplan der Oud nach Abdallah u. a. 2026 mit ihren drei Schallloechern
(34k-5, KQ7 a; Fortsetzung von 2026-11-12_S195_oud_bauplan.py, Bauplan Fassung 4).

Aufruf: <oud_umriss.json> <zieldatei.lvinstbodyplan> <dicke_mm> [--ohne-loecher] [--ohne-balken]
        [--steg <laenge_cm> <breite_mm> <hoehe_mm>] [--faser <grad>] [--balken-bis-rand]
        [--hohlraum [<tiefe_cm>]] [--loecher-gelesen]

--hohlraum: der Luftraum der Schale (F-K1 a, KQ8 a). Tiefe 21 cm nach der Masszahl in Abb. 3 (Seitenansicht);
Annahme: jeder Querschnitt eine halbe Ellipse, ihre Tiefe proportional zur Breite der Decke an der Stelle —
V = (π/2) · (Tiefe / halbe groesste Breite) · ∫ b(y)² dy.
Lochradien nach den Masszahlen in Abb. 3 (R 5.3 und R 2.0 cm); --loecher-gelesen nimmt die in S195 aus dem Bild
gelesenen Durchmesser 10,5 und 3,9 cm.

Loecher (aus Abb. 3 gelesen, S195): Durchmesser 10,5 cm in 30,0 cm Hoehe auf der Mitte; zwei mit
Durchmesser 3,9 cm in 20,4 cm Hoehe, 10,3 cm links und rechts der Mitte. Je Loch ein Vieleck mit
der Flaeche des Kreises."""
import json
import math
import pathlib
import sys

masse = json.load(open(sys.argv[1], encoding="utf-8-sig"))
ziel = pathlib.Path(sys.argv[2])
dicke = float(sys.argv[3]) / 1000.0
schalter = sys.argv[4:]
mit_loechern = "--ohne-loecher" not in schalter
mit_balken = "--ohne-balken" not in schalter
steg = None
if "--steg" in schalter:
    i = schalter.index("--steg")
    steg = [float(v) for v in schalter[i + 1:i + 4]]
# Faser der Decke gegen die x-Achse (quer): 90 = laengs (wie gebaut), 0 = quer
faser = float(schalter[schalter.index("--faser") + 1]) if "--faser" in schalter else 90.0

# Umriss: rechts hinauf, links herunter; x quer (Mitte 0), y laengs vom unteren Ende, Meter
rechts = [(u[2] / 100.0, u[0] / 100.0) for u in masse["umriss"]]
links = [(u[1] / 100.0, u[0] / 100.0) for u in reversed(masse["umriss"])]
umriss = [[round(x, 5), round(y, 5)] for x, y in rechts + links]

# Balken: Lage aus Abb. 3 gelesen, Masse aus Tab. 2 (wie S195)
BALKEN = [  # name, y cm, x von, x bis (cm), c mm, breite mm
    ("brace 1", 4.11, -13.0, 13.0, 10.0, 4.2),
    ("brace 2", 17.53, -17.0, 17.0, 8.0, 4.2),
    ("brace 3", 23.68, -16.85, 16.85, 7.0, 5.0),
    ("brace 5", 30.05, -15.6, -5.8, 7.0, 6.6),
    ("brace 4", 30.05, 5.8, 15.6, 7.0, 6.6),
    ("brace 6", 36.38, -12.75, 12.75, 10.0, 4.2),
    ("brace 7", 39.82, -11.0, 11.0, 10.0, 4.2),
]
B_MITTE = 16.5
# Gegenprobe --balken-bis-rand: die aeusseren Enden jedes Balkens bis ueber den Umriss verlaengert
# (das Netz schneidet am Umriss ab) — der Balken endet dann am eingespannten Rand statt 1–1,5 cm davor
if "--balken-bis-rand" in schalter:
    BALKEN = [(n, y, -20.0 if x0 < -8.0 else x0, 20.0 if x1 > 8.0 else x1, c, b) for n, y, x0, x1, c, b in BALKEN]

# --balken-luecke <cm>: die aeusseren Enden jedes Balkens enden so weit vor dem Rand (0 = am Rand;
# in Abb. 3 gelesen 1,3–1,5 cm). Die Breite der Decke an der Hoehe des Balkens aus dem Umriss.
if "--balken-luecke" in schalter:
    luecke = float(schalter[schalter.index("--balken-luecke") + 1])

    def rand(y):
        zeilen = masse["umriss"]
        for k in range(len(zeilen) - 1):
            if zeilen[k][0] <= y <= zeilen[k + 1][0]:
                t = (y - zeilen[k][0]) / (zeilen[k + 1][0] - zeilen[k][0])
                return (zeilen[k][1] + t * (zeilen[k + 1][1] - zeilen[k][1]), zeilen[k][2] + t * (zeilen[k + 1][2] - zeilen[k][2]))
        raise ValueError(y)

    BALKEN = [(n, y, rand(y)[0] + luecke if x0 < -8.0 else x0, rand(y)[1] - luecke if x1 > 8.0 else x1, c, b)
              for n, y, x0, x1, c, b in BALKEN]

GELESEN = "--loecher-gelesen" in schalter
LOECHER = [  # x cm, y cm, durchmesser cm, ecken
    (0.0, 30.0, 10.5 if GELESEN else 10.6, 48),
    (-10.3, 20.4, 3.9 if GELESEN else 4.0, 24),
    (10.3, 20.4, 3.9 if GELESEN else 4.0, 24),
]

# der Hohlraum: Volumen der Schale aus dem Umriss und der Tiefe
volumen = 0.0
tiefe_cm = 0.0
if "--hohlraum" in schalter:
    i = schalter.index("--hohlraum")
    tiefe_cm = float(schalter[i + 1]) if i + 1 < len(schalter) and not schalter[i + 1].startswith("--") else 21.0
    halb = [(u[0] / 100.0, (u[2] - u[1]) / 200.0) for u in masse["umriss"]]  # y, halbe Breite (m)
    groesste = max(b for _, b in halb)
    integral = sum((halb[k + 1][0] - halb[k][0]) * (halb[k][1] ** 2 + halb[k + 1][1] ** 2) / 2.0 for k in range(len(halb) - 1))
    volumen = math.pi / 2.0 * (tiefe_cm / 100.0 / groesste) * integral


def leiste(name, y, x0, x1, c, breite):
    xm = (x0 + x1) / 2
    return {"name": name, "material": "sitka",
            "line": [[x0 / 100, y / 100], [xm / 100, y / 100], [x1 / 100, y / 100]],
            "height": [c / 1000, B_MITTE / 1000, c / 1000],
            "width": [breite / 1000] * 3}


def loch(x, y, d, ecken):
    # Vieleck mit der Flaeche des Kreises: R² · n/2 · sin(2π/n) = π r²
    r = d / 200.0
    radius = r * math.sqrt(2.0 * math.pi / (ecken * math.sin(2.0 * math.pi / ecken)))
    return [[round(x / 100.0 + radius * math.cos(2.0 * math.pi * k / ecken), 6),
             round(y / 100.0 + radius * math.sin(2.0 * math.pi * k / ecken), 6)] for k in range(ecken)]


plan = {
    "format": "lumiviz-instrument-body-plan",
    "version": 4,
    "title": "Oud probe" + ("" if mit_loechern else " (no holes)") + ("" if mit_balken else " (no braces)"),
    "description": "Substitute plan of an oud soundboard for the probe 34k-5 (KQ6 a, KQ7 a): outline, brace positions "
                   "and sound holes read from Abdallah et al. 2026 Fig. 3; brace sizes Table 2; wood Table 1. "
                   f"Assumptions: plate thickness {dicke * 1000:.2f} mm (not given in the source), "
                   f"grain of the plate at {faser:g} degrees to the x axis (x across the soundboard), "
                   + ("no bridge bar" if steg is None else f"bridge bar {steg[0]:g} cm x {steg[1]:g} mm x {steg[2]:g} mm (assumed)")
                   + ", rim clamped (as in the source's FE), hole rims free.",
    "source": "M. Abdallah, E. Dreik, A. Sassi, M. Paurobally, S. Sassi, Vibroacoustic sensitivity of an oud "
              "soundboard to bracing geometry, Meccanica 61:89 (2026), CC BY 4.0",
    "units": "SI: m, kg/m^3, Pa",
    "materials": [{
        "name": "sitka", "kind": "orthotropic", "density": 460.0,
        "ex": 10.34e9, "ey": 0.80e9, "gxy": 0.63e9, "gxz": 0.66e9, "gyz": 0.03e9, "nuxy": 0.467,
        "lossFactor": 0.0,
        "source": "Abdallah et al. 2026 Table 1 (x = L, y = R, z = T)"}],
    "plate": {"material": "sitka", "grainAngle": faser, "outline": umriss, "thickness": dicke,
              "holes": [loch(*l) for l in LOECHER] if mit_loechern else []},
    "ribs": [leiste(*b) for b in BALKEN] if mit_balken else [],
    "bridges": [] if steg is None else [{
        "name": "bridge", "material": "sitka",
        "line": [[-steg[0] / 200, 0.09], [0.0, 0.09], [steg[0] / 200, 0.09]],
        "height": [steg[2] / 1000] * 3, "width": [steg[1] / 1000] * 3}],
    "bridgePoints": [{"key": 40 + k, "x": round(-0.04 + 0.016 * k, 4), "y": 0.09,
                      "source": "bridge 41 cm below the top of the body (Fig. 3), courses spread over 8 cm (assumed)"}
                     for k in range(6)],
    "listeningPoints": [{"name": "front", "x": 0.0, "y": 0.25, "z": 0.5, "source": "assumed, 0.5 m in front"}],
    "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off"},
    "damping": {"a": 2e-05, "b": 0.07, "c": 4.5, "source": "as the Steinway D plan (placeholder for the probe)"},
    "cavity": {"volume": round(volumen, 6), "endCorrection": 1.7, "q": 30.0,
               "source": "none" if volumen == 0.0 else
               f"bowl {tiefe_cm:g} cm deep (Abdallah et al. 2026 Fig. 3, side view); assumed: every cross-section half an "
               "ellipse with its depth proportional to the width of the soundboard there; end correction of a hole in a "
               "thin wide wall, Q of the air assumed"},
}

ziel.parent.mkdir(parents=True, exist_ok=True)
ziel.write_text(json.dumps(plan, indent=1), encoding="utf-8")
print(ziel)

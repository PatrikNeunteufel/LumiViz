"""Schreibt Ersatz-Bauplaene der Oud nach Abdallah u. a. 2026 (Probe 34k-5, erster Fall KQ6 a).
Aufruf: make_oud.py <oud_umriss.json> <zielordner> <dicke_mm>
Schreibt oud_ohne_balken.lvinstbodyplan und oud_mit_balken.lvinstbodyplan."""
import json
import pathlib
import sys

masse = json.load(open(sys.argv[1], encoding="utf-8-sig"))
ziel = pathlib.Path(sys.argv[2])
dicke = float(sys.argv[3]) / 1000.0
ziel.mkdir(parents=True, exist_ok=True)

# Umriss: rechts hinauf, links herunter; x quer (Mitte 0), y laengs vom unteren Ende, Meter
rechts = [(u[2] / 100.0, u[0] / 100.0) for u in masse["umriss"]]
links = [(u[1] / 100.0, u[0] / 100.0) for u in reversed(masse["umriss"])]
umriss = [[round(x, 5), round(y, 5)] for x, y in rechts + links]

# Balken: Lage (Hoehe ueber dem unteren Ende) aus Abb. 3 gelesen, Masse aus Tab. 2
# (a Laenge, b Hoehe in der Mitte, c Hoehe am Ende, "Thickness" als Breite)
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


def leiste(name, y, x0, x1, c, breite):
    xm = (x0 + x1) / 2
    return {"name": name, "material": "sitka",
            "line": [[x0 / 100, y / 100], [xm / 100, y / 100], [x1 / 100, y / 100]],
            "height": [c / 1000, B_MITTE / 1000, c / 1000],
            "width": [breite / 1000] * 3}


def plan(titel, mit_balken):
    return {
        "format": "lumiviz-instrument-body-plan",
        "version": 3,
        "title": titel,
        "description": "Substitute plan of an oud soundboard for the probe 34k-5 (KQ6 a): outline, brace positions "
                       "read from Abdallah et al. 2026 Fig. 3; brace sizes Table 2; wood Table 1. Assumptions: "
                       f"plate thickness {dicke * 1000:.2f} mm (not given in the source), no sound holes "
                       "(InstrumentBody has no inner free edges), no bridge bar, rim clamped (as in the source's FE).",
        "source": "M. Abdallah, E. Dreik, A. Sassi, M. Paurobally, S. Sassi, Vibroacoustic sensitivity of an oud "
                  "soundboard to bracing geometry, Meccanica 61:89 (2026), CC BY 4.0",
        "units": "SI: m, kg/m^3, Pa",
        "materials": [{
            "name": "sitka", "kind": "orthotropic", "density": 460.0,
            "ex": 10.34e9, "ey": 0.80e9, "gxy": 0.63e9, "gxz": 0.66e9, "gyz": 0.03e9, "nuxy": 0.467,
            "lossFactor": 0.0,
            "source": "Abdallah et al. 2026 Table 1 (x = L, y = R, z = T)"}],
        "plate": {"material": "sitka", "grainAngle": 90.0, "outline": umriss, "thickness": dicke},
        "ribs": [leiste(*b) for b in BALKEN] if mit_balken else [],
        "bridges": ([] if len(sys.argv) < 7 else [{
            "name": "bridge", "material": "sitka",
            "line": [[-float(sys.argv[4]) / 200, 0.09], [0.0, 0.09], [float(sys.argv[4]) / 200, 0.09]],
            "height": [float(sys.argv[6]) / 1000] * 3, "width": [float(sys.argv[5]) / 1000] * 3}]),
        "bridgePoints": [{"key": 40 + k, "x": round(-0.04 + 0.016 * k, 4), "y": 0.09,
                          "source": "bridge 41 cm below the top of the body (Fig. 3), courses spread over 8 cm (assumed)"}
                         for k in range(6)],
        "listeningPoints": [{"name": "front", "x": 0.0, "y": 0.25, "z": 0.5, "source": "assumed, 0.5 m in front"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off"},
        "damping": {"a": 2e-05, "b": 0.07, "c": 4.5, "source": "as the Steinway D plan (placeholder for the probe)"},
    }


for name, mit in (("oud_ohne_balken", False), ("oud_mit_balken", True)):
    (ziel / f"{name}.lvinstbodyplan").write_text(json.dumps(plan(f"Oud probe ({name})", mit), indent=1), encoding="utf-8")
    print(ziel / f"{name}.lvinstbodyplan")

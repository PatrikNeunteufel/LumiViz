"""S209 - jede Stellgroesse des Oud-Bauplans einzeln abtasten (Empfindlichkeit gegen Z1-Z3)."""
import copy
import sys

import oud_suche as s

B = s.basis()


def v(name, aendere):
    p = copy.deepcopy(B)
    aendere(p)
    return s.rechne(p, name)


def dicke(d):
    return lambda p: p["plate"].__setitem__("thickness", d)


def rand(halt, feder=0.0):
    def f(p):
        p["rim"]["support"] = halt
        p["rim"]["rotationalStiffness"] = feder
    return f


def balken_hoehe(namen, faktor):
    def f(p):
        for r in p["ribs"]:
            if r["name"] in namen:
                r["height"] = [h * faktor for h in r["height"]]
    return f


def balken_weg(namen):
    return lambda p: p.__setitem__("ribs", [r for r in p["ribs"] if r["name"] not in namen])


def steg(laenge, breite, hoehe, dichte=None, ex=None):
    def f(p):
        mat = "sitka"
        if dichte:
            p["materials"].append({"name": "bridgewood", "kind": "isotropic", "density": dichte, "e": ex, "nu": 0.3, "lossFactor": 0.0, "source": "probe"})
            mat = "bridgewood"
        p["bridges"] = [{"name": "bridge", "material": mat, "line": [[-laenge / 2, 0.09], [0.0, 0.09], [laenge / 2, 0.09]],
                         "height": [hoehe] * 3, "width": [breite] * 3}]
    return f


def hohlraum(**kw):
    return lambda p: p["cavity"].update(kw)


def steg_y(y):
    def f(p):
        for b in p["bridgePoints"]:
            b["y"] = y
    return f


gruppe = sys.argv[1] if len(sys.argv) > 1 else "alle"
if gruppe in ("alle", "dicke"):
    for d in (1.2, 1.8, 2.0):
        v(f"dicke_{d}", dicke(d / 1000))
if gruppe in ("alle", "rand"):
    v("rand_gestuetzt", rand("elastic", 0.0))
    for k in (30, 300, 3000):
        v(f"rand_feder_{k}", rand("elastic", float(k)))
if gruppe in ("alle", "balken"):
    for n in ("brace 1", "brace 2", "brace 3"):
        v(f"{n}_halb", balken_hoehe([n], 0.5))
        v(f"{n}_weg", balken_weg([n]))
    v("brace 2_x1.5", balken_hoehe(["brace 2"], 1.5))
    v("brace 1_x1.5", balken_hoehe(["brace 1"], 1.5))
if gruppe in ("alle", "steg"):
    v("steg_fichte_14x10x6", steg(0.14, 0.010, 0.006))
    v("steg_nuss_17x15x10", steg(0.17, 0.015, 0.010, 650.0, 11e9))
    v("steg_masse_17x15x10", steg(0.17, 0.015, 0.010, 650.0, 0.2e9))
if gruppe in ("alle", "luft"):
    v("volumen_15l", hohlraum(volume=0.015))
    v("volumen_26l", hohlraum(volume=0.026))
    v("muendung_1.0", hohlraum(endCorrection=1.0))
    v("muendung_2.4", hohlraum(endCorrection=2.4))
if gruppe in ("alle", "stegy"):
    for y in (0.07, 0.11, 0.13):
        v(f"stegpunkte_y{y}", steg_y(y))

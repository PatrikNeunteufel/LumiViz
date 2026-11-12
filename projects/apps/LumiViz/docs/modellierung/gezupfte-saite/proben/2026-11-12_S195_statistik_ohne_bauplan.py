"""Probe 34k-5 (S195): Liefert der statistische Teil von InstrumentBody Moden aus Plattenwerten allein?

`schaetze` (InstrumentBody/Statistik.hpp) nimmt Dichte, Streuung am Stegpunkt und Abstrahlbetrag aus *gerechneten*
Moden — ohne Bauplan gibt es keine, der Teil bleibt leer. Diese Probe fragt deshalb das Naechste: lassen sich die drei
Groessen aus Plattenwerten (Flaeche, Dicke, Dichte, Biegesteife, Masse) vorhersagen? Geprueft an den drei mitgelieferten
Fluegelkoerpern, deren Moden gerechnet vorliegen:

    Modendichte der duennen orthotropen Platte   n = (A / 2) * sqrt(rho h / sqrt(Dx Dy))        [Moden je Hz]
    Formwert am Stegpunkt, quadratisches Mittel  phi = 1 / sqrt(M)                               [1/sqrt(kg)]

Aufruf:  python 2026-11-12_S195_statistik_ohne_bauplan.py <ordner asset/loop/instrument_bodies>
Liest nur, schreibt nichts.
"""
import json
import math
import pathlib
import sys

BAENDER = [(0, 250), (250, 500), (500, 1000), (1000, 2500), (2500, 5000)]


def innen(umriss, x, y):
    drin = False
    n = len(umriss)
    j = n - 1
    for k in range(n):
        ax, ay = umriss[k]
        bx, by = umriss[j]
        if (ay > y) != (by > y) and x < (bx - ax) * (y - ay) / (by - ay) + ax:
            drin = not drin
        j = k
    return drin


def flaeche(umriss):
    s = 0.0
    for k in range(len(umriss)):
        ax, ay = umriss[k]
        bx, by = umriss[(k + 1) % len(umriss)]
        s += ax * by - bx * ay
    return abs(s) / 2.0


def leistenmasse(leiste, dichte):
    s = 0.0
    for k in range(len(leiste["line"]) - 1):
        (ax, ay), (bx, by) = leiste["line"][k], leiste["line"][k + 1]
        q0 = leiste["height"][k] * leiste["width"][k]
        q1 = leiste["height"][k + 1] * leiste["width"][k + 1]
        s += math.hypot(bx - ax, by - ay) * (q0 + q1) / 2.0
    return s * dichte


def rms(werte):
    return math.sqrt(sum(v * v for v in werte) / len(werte)) if werte else float("nan")


def probe(plan_datei, koerper_datei):
    plan = json.load(open(plan_datei, encoding="utf-8"))
    koerper = json.load(open(koerper_datei, encoding="utf-8"))
    stoffe = {m["name"]: m for m in plan["materials"]}
    platte = plan["plate"]
    stoff = stoffe[platte["material"]]
    umriss = platte["outline"]
    a = flaeche(umriss)

    # mittlere Dicke und Plattenmasse auf einem 5-mm-Raster wie plattenMasse()
    gitter = platte.get("thicknessGrid") or {}
    xs = [p[0] for p in umriss]
    ys = [p[1] for p in umriss]
    schritt = 0.005
    volumen, zellen = 0.0, 0
    y = min(ys) + schritt / 2
    while y < max(ys):
        x = min(xs) + schritt / 2
        while x < max(xs):
            if innen(umriss, x, y):
                h = platte["thickness"]
                if gitter.get("values"):
                    i = min(max(round((x - gitter["origin"][0]) / gitter["step"]), 0), gitter["nx"] - 1)
                    j = min(max(round((y - gitter["origin"][1]) / gitter["step"]), 0), gitter["ny"] - 1)
                    v = gitter["values"][j * gitter["nx"] + i]
                    if v is not None:
                        h = v
                volumen += h * schritt * schritt
                zellen += 1
            x += schritt
        y += schritt
    h_mittel = volumen / (zellen * schritt * schritt)
    m_platte = volumen * stoff["density"]
    m_rippen = sum(leistenmasse(l, stoffe[l["material"]]["density"]) for l in plan["ribs"])
    m_stege = sum(leistenmasse(l, stoffe[l["material"]]["density"]) for l in plan["bridges"])
    m_ganz = m_platte + m_rippen + m_stege

    ex, ey, nu = stoff["ex"], stoff.get("ey") or stoff["ex"], stoff["nuxy"]
    if stoff.get("kind") == "isotropic":
        ey = ex
    nenner = 1.0 - nu * nu * ey / ex
    dx = ex * h_mittel ** 3 / (12.0 * nenner)
    dy = ey * h_mittel ** 3 / (12.0 * nenner)
    n_platte = a / 2.0 * math.sqrt(stoff["density"] * h_mittel / math.sqrt(dx * dy))
    n_verschmiert = a / 2.0 * math.sqrt((m_ganz / a) / math.sqrt(dx * dy))

    print(f"== {pathlib.Path(koerper_datei).name}")
    print(f"   Flaeche {a:.3f} m2, mittlere Dicke {h_mittel * 1000:.2f} mm, Werkstoff {stoff['name']} "
          f"(rho {stoff['density']:.0f}, Ex {ex / 1e9:.1f} GPa, Ey {ey / 1e9:.2f} GPa)")
    print(f"   Masse: Platte {m_platte:.2f} kg, Rippen {m_rippen:.2f} kg, Stege {m_stege:.2f} kg, zusammen {m_ganz:.2f} kg")
    print(f"   Vorhersage Dichte: Platte allein {n_platte:.4f} /Hz, mit verschmierter Masse {n_verschmiert:.4f} /Hz")
    print(f"   Vorhersage Formwert: 1/sqrt(M Platte) {1 / math.sqrt(m_platte):.4f}, 1/sqrt(M ganz) {1 / math.sqrt(m_ganz):.4f} 1/sqrt(kg)")
    st = koerper.get("statistical", {})
    if st:
        bs = st["bridgeSpread"]
        print(f"   Datei, statistischer Teil ab {st['fromHz']} Hz: Dichte {st['density']:.4f} /Hz (Anstieg {st['rise']:.2e}), "
              f"Formwert Mittel {sum(bs) / len(bs):.4f} (min {min(bs):.4f}, max {max(bs):.4f}), "
              f"Abstrahlbetrag {', '.join(f'{v:.5f}' for v in st['radiation'])}")
    print("   gerechnete Moden je Band:   Zahl   Dichte /Hz  gegen Platte  Formwert rms (min..max je Stegpunkt)   gegen 1/sqrt(M ganz)   |G| rms")
    for von, bis in BAENDER:
        band = [m for m in koerper["modes"] if von <= m["f"] < bis]
        if not band:
            continue
        ns = len(band[0]["bridge"])
        je_punkt = [rms([m["bridge"][j] for m in band]) for j in range(ns)]
        alle = rms([v for m in band for v in m["bridge"]])
        g = []
        for m in band:
            for r in m.get("radiation", []):
                g.append(math.hypot(r[0], r[1]) if isinstance(r, list) else abs(r))
        dichte = len(band) / (bis - von)
        print(f"   {von:5d}-{bis:5d} Hz            {len(band):5d}   {dichte:8.4f}    {dichte / n_platte:6.2f}        "
              f"{alle:.4f} ({min(je_punkt):.4f}..{max(je_punkt):.4f})            {alle * math.sqrt(m_ganz):6.2f}              "
              f"{rms(g):.5f}")
    print()


def main():
    ordner = pathlib.Path(sys.argv[1])
    for name in ("steinway_d", "steinway_d_glass", "steinway_d_glass_same_mass"):
        probe(ordner / f"{name}.lvinstbodyplan", ordner / f"{name}.lvinstbody")


if __name__ == "__main__":
    main()

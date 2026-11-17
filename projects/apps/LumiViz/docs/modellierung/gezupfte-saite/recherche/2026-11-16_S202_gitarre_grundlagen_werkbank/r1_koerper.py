"""Lücke 1: tiefe Moden der Körperdatei g34 (nur lesen) und Gegenrechnung gegen Wright 1996 Tab. 5.4 (0,9 m).

p/F je Mode = phi * G * (-w^2)/(wn^2 - w^2 + i eta wn w); an der Resonanz |p/F| = |phi G| / eta.
Monopol im Fernfeld (Wright Gl. 4.17 mit U = A v, v/F = i w phi^2 / D):  G_fern = rho * A_eff * phi_drive / (4 pi r).
"""
import json
import math
import pathlib

P = pathlib.Path(r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\classical_guitar.lvinstbody")
k = json.loads(P.read_text(encoding="utf-8"))
print("Schluessel:", list(k.keys()))
print("Hoerorte:", k.get("listeningPoints"), " Stegpunkte:", k.get("bridgePoints"))
print("statistical:", {a: b for a, b in k["statistical"].items() if a != "materialShares"})
print()
print(" f/Hz    eta     phi(S6)  phi(S1)   |G_left| |G_right|  |p/F| left/right an Resonanz (Saite 6) dB re 1 Pa/N")
for m in k["modes"][:14]:
    f, eta = m["f"], m["eta"]
    phi6, phi1 = m["bridge"][0], m["bridge"][-1]
    g = [abs(complex(*x)) for x in m["radiation"]]
    pf = [abs(phi6) * x / eta for x in g]
    print(f"{f:7.1f} {eta:7.4f} {phi6:8.3f} {phi1:8.3f}   {g[0]:8.4f} {g[1]:8.4f}   "
          + " / ".join(f"{20*math.log10(v):6.1f}" for v in pf))

rho = 1.2
print()
print("Wright 1996 Tab. 5.4 (BR2, Schallloch zu, Antrieb Steg tiefe E, Mikrofon 0,9 m vor dem Steg):")
for name, f, m_eff, Q, A in [("T(1,1)", 165, 0.062, 36, 0.059), ("T(2,1)", 204, 0.201, 66, 0.010),
                             ("B(1,2)", 293, 1.920, 50, 0.186), ("T(1,2)", 391, 0.612, 60, -0.133),
                             ("T(3,1)", 432, 0.726, 51, -0.189)]:
    phi = 1 / math.sqrt(m_eff)
    for r in (0.9, 1.0):
        G = rho * A * phi / (4 * math.pi * r)
        pf = abs(rho * A * Q / (4 * math.pi * r * m_eff))
        print(f"  {name} {f} Hz r={r} m: G_fern = {G:+.4f} sqrt(kg)/m^2, |p/F| Resonanz = {pf:.3f} Pa/N = {20*math.log10(pf):.1f} dB re 1 Pa/N")

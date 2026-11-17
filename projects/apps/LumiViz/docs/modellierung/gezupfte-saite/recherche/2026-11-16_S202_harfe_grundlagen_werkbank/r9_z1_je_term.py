"""R9: (1) Wirkung jedes Saitenterms auf |Y| an den Oesen der Saiten 33 / 21 / 12 bei ihrer Frequenz (Z1), durch
Neusynthese im Modenraum (diagonale Naeherung); (2) Diskantende: Y_inf bei duennerem Boden, Balkenende (Gl. P1-P3, E1)."""
import json, math, cmath, io, contextlib
import importlib.util
spec = importlib.util.spec_from_file_location("r1", r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\r1_saiten.py")
r1 = importlib.util.module_from_spec(spec)
with contextlib.redirect_stdout(io.StringIO()):
    spec.loader.exec_module(r1)
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
kd = json.load(open(p, encoding="utf-8"))
modes, keys = kd["modes"], kd["bridgePoints"]
def key_of(hz): return int(round(69 + 12 * math.log2(hz / 440.0))) + 1
oesen = {n: keys.index(key_of(s["f"])) for n, s in r1.daten.items()}
# je Mode: Zusatzsteife (Laengsfeder, Hals starr) und Zusatzwiderstand (gedaempfte Saite, Z0 cos2) - diagonal
add = []
for m in modes:
    dk = dr = 0.0
    for n, s in r1.daten.items():
        phi2 = m["bridge"][oesen[n]] ** 2
        dk += s["kl"] * r1.S2 * phi2; dr += s["Z0"] * r1.C2 * phi2
    add.append((dk, dr))
def Y(b, f, feder=0.0, widerstand=0.0):
    w = 2 * math.pi * f; s = 0j
    for m, (dk, dr) in zip(modes, add):
        wk2 = (2 * math.pi * m["f"]) ** 2
        s += m["bridge"][b] ** 2 / (wk2 + feder * dk - w * w + 1j * m["eta"] * wk2 + 1j * w * widerstand * dr)
    return 1j * w * s
dB = lambda y: 20 * math.log10(abs(y)) + 116
print("Saite f   | ohne | +Laengsfeder (Hals starr) | +0,4 x Feder (Hals nachgiebig) | +gedaempft Z0cos2 | beides | Bell 7.22")
BELL = {"33": 80, "21": 95, "12": 103}
for n in ("33", "21", "12"):
    s = r1.daten[n]; b = oesen[n]; f = s["f"]
    # die eigene Saite: Zusatzimpedanz direkt am Punkt (Laengs- und Querterm, gedaempft)
    y0 = Y(b, f); y1 = Y(b, f, 1.0); y2 = Y(b, f, 0.4); y3 = Y(b, f, 0, 1.0); y4 = Y(b, f, 0.4, 1.0)
    print(f"{n:>4} {f:4d} | {dB(y0):5.1f} | {dB(y1):5.1f} ({dB(y1)-dB(y0):+.1f}) | {dB(y2):5.1f} ({dB(y2)-dB(y0):+.1f}) | "
          f"{dB(y3):5.1f} ({dB(y3)-dB(y0):+.1f}) | {dB(y4):5.1f} ({dB(y4)-dB(y0):+.1f}) | {BELL[n]}")
# Median ueber alle Saiten 33-10 (die Oesen von Bell 7.22)
F = [104, 116, 123, 139, 155, 165, 185, 207, 233, 247, 277, 311, 330, 370, 415, 466, 494, 554, 622, 659, 740, 830, 932, 988]
d1, d3, d4 = [], [], []
for f in F:
    b = keys.index(key_of(f)); y0 = dB(Y(b, f))
    d1.append(dB(Y(b, f, 1.0)) - y0); d3.append(dB(Y(b, f, 0, 1.0)) - y0); d4.append(dB(Y(b, f, 0.4, 1.0)) - y0)
md = lambda v: sorted(v)[len(v) // 2]
print(f"Saiten 33-10: Median der Aenderung: Feder {md(d1):+.1f} dB (Bereich {min(d1):+.1f}..{max(d1):+.1f}), "
      f"gedaempft {md(d3):+.1f} ({min(d3):+.1f}..{max(d3):+.1f}), beides {md(d4):+.1f} ({min(d4):+.1f}..{max(d4):+.1f})")

# (2) Diskantende
RHO_F, EX, EY, NU, RHO_B, EB = 500.0, 7.3e9, 0.855e9, 0.3, 750.0, 13.7e9
def Yp(h): m2 = RHO_F * h; D = math.sqrt(EX * EY) * h**3 / 12 / (1 - NU**2); return 1 / (8 * math.sqrt(D * m2))
print("\nDiskant: Y_inf der Platte (ohne Leisten), dB re Bell")
for h_holz in (0.003, 0.0025, 0.002):
    print(f"  Holz {h_holz*1e3:.1f} mm + 1 mm Furnier: {dB(Yp(h_holz+0.001)):.1f} dB")
# Balken an der Oese der Saite 5 (y 1,118 m, 1661 Hz): Abstand zum Ende der Zugleiste (1,28 m) und der Deckleiste (1,305 m)
EI, mp = 120.0, 0.30   # grob aus r2 bei y ~1,12 m extrapoliert (y 0,951: 164,6 N m2 / 0,359 kg/m)
for f, y in ((1661, 1.118), (988, 0.999), (2960, 1.246)):
    w = 2 * math.pi * f; kB = (mp * w * w / EI) ** 0.25; lam = 2 * math.pi / kB
    x = 1.28 - y
    # halbunendlicher Balken mit freiem Ende, Antrieb im Abstand x (Euler-Bernoulli, Cremer/Heckl): Faktor gegen unendlich
    # Y(x)/Y_inf = 1 + [ (1-j) e^{-2jkx} ... ] - hier nur die Huellkurve: 1 + 3 e^{-k x}   (x=0: 4, x>>: 1)
    fak = 1 + 3 * math.exp(-kB * x)
    print(f"  {f} Hz an y {y}: lambda_B {lam:.2f} m, Abstand zum Leistenende {x:.3f} m, Huelle |Y|/|Y_inf| bis {20*math.log10(fak):.1f} dB")

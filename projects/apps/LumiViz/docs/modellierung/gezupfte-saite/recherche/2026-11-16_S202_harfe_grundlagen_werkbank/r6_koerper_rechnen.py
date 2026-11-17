"""R6: aus concert_harp.lvinstbody (nur gelesen): Punkt-Admittanz an den Oesen, p/F am Hoerort bis 2 kHz,
Verlustfaktor je Mode zerlegt, Abstrahlungsdaempfung je Mode (Gl. D1-D4, A1-A3)."""
import json, math, cmath
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
k = json.load(open(p, encoding="utf-8"))
print("units:", k["units"])
print("Hoerort:", k["listeningPoints"])
RHO, C = k["air"]["density"], k["air"]["speedOfSound"]
modes = k["modes"]
keys = k["bridgePoints"]

def key_of(hz):
    return int(round(69 + 12 * math.log2(hz / 440.0))) + 1

def Y(b, f):
    w = 2 * math.pi * f
    s = 0j
    for m in modes:
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        s += phi * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return 1j * w * s

def pF(b, f, nur_unter=None):
    w = 2 * math.pi * f
    s = 0j
    for m in modes:
        if nur_unter and m["f"] > nur_unter: continue
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        r = complex(m["radiation"][0][0], m["radiation"][0][1])
        s += r * (-w * w) * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return s

dB = lambda y: 20 * math.log10(abs(y)) + 116
# 1) Gegenprobe gegen S201 (Saiten 33, 26, 21, 12, 10)
for s, hz in (("33", 104), ("26", 207), ("21", 330), ("12", 830), ("10", 988)):
    b = keys.index(key_of(hz))
    print(f"Saite {s} {hz} Hz Taste {key_of(hz)}: |Y| = {dB(Y(b, hz)):.1f} dB (S201: 69.7 / 88.3 / 68.2 / 68.7 / 66.8)")

# 2) Verlustfaktor je Mode zerlegt: eta = (1 - airShare) * alpha/(2 pi f) + airShare / Q_luft
d = k["damping"]
print("\nMode   f[Hz]   eta    Q   airShare  eta_Platte(Gesetz) Q_luft_rueck  eta_rad(Halbraum) eta_rad(Vollraum)")
for m in modes[:8]:
    f = m["f"]; eta_law = (d["a"] * f * f + d["b"] * f + d["c"]) / (2 * math.pi * f)
    a = m["airShare"]
    ql = a / (m["eta"] - (1 - a) * eta_law) if m["eta"] > (1 - a) * eta_law and a > 1e-6 else float("nan")
    r = abs(complex(*m["radiation"][0]))
    w = 2 * math.pi * f
    # Monopol: p(1 m) = rho * qdd * V / (2 pi r) (Halbraum, Rayleigh) bzw. /(4 pi r) (Vollraum); V = Volumen je Modalkoordinate
    V_h = 2 * math.pi * 1.0 * r / RHO; V_v = 4 * math.pi * 1.0 * r / RHO
    eta_h = RHO * w * V_h ** 2 / (2 * math.pi * C)    # Halbraum: P = rho w^2 U^2 /(4 pi c)
    eta_v = RHO * w * V_v ** 2 / (4 * math.pi * C)    # Vollraum: P = rho w^2 U^2 /(8 pi c)
    print(f"{f:7.1f} {m['eta']:.4f} {1/m['eta']:5.1f} {a:7.3f}  {eta_law:.4f}          {ql:6.1f}      {eta_h:.2e}  {eta_v:.2e}")

# 3) p/F am Hoerort (1 m vor dem Boden), Anregung an den Oesen der Saiten 31 / 30 (Le Carrou 2010: zwischen 30 und 31)
b31 = keys.index(key_of(123)); b30 = keys.index(key_of(139))
LC2010 = [(134, 40), (148, 57), (166, 60), (195, 62), (290, 26), (384, 58), (460, 50), (582, 46), (644, 54), (688, 41),
          (756, 53), (850, 26), (892, 48), (1000, 42)]
print("\nf     p/F(1 m) dB re 2e-5 Pa/N, Oese 31 | Oese 30 | auf 2,35 m (-7,4 dB) Mittel | Le Carrou 2010 (2,35 m, Camac)")
for f, lc in LC2010:
    a1 = abs(pF(b31, f)); a0 = abs(pF(b30, f))
    L1 = 20 * math.log10(a1 / 2e-5); L0 = 20 * math.log10(a0 / 2e-5)
    m = 20 * math.log10((a1 + a0) / 2 / 2e-5) - 20 * math.log10(2.35)
    print(f"{f:5d}  {L1:6.1f}  {L0:6.1f}   {m:6.1f}    {lc}")
# Z4: 204 Hz gegen 240-250 Hz, gemittelt ueber 1/12-Oktave
def band(b, f0):
    fs = [f0 * 2 ** (i / 240) for i in range(-10, 11)]
    return 10 * math.log10(sum(abs(pF(b, f)) ** 2 for f in fs) / len(fs))
for b, n in ((b31, "31"), (b30, "30")):
    z4 = 20 * math.log10(abs(pF(b, 204))) - 10 * math.log10(sum(abs(pF(b, f)) ** 2 for f in range(240, 251)) / 11)
    print(f"Z4 Oese {n}: p/F(204 Hz) - mittel(240-250 Hz) = {z4:+.1f} dB")
# p/F-Verlauf in Terzbaendern 63 Hz - 2 kHz (Oese 31)
print("\nTerzband  p/F Oese 31 [dB re 2e-5 Pa/N, 1 m]  davon Moden <= 300 Hz")
for fm in (63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000):
    fs = [fm * 2 ** (i / 60) for i in range(-10, 11)]
    e = sum(abs(pF(b31, f)) ** 2 for f in fs) / len(fs)
    e3 = sum(abs(pF(b31, f, 300)) ** 2 for f in fs) / len(fs)
    print(f"{fm:6d}  {10*math.log10(e/4e-10):6.1f}   {10*math.log10(e3/4e-10):6.1f}")

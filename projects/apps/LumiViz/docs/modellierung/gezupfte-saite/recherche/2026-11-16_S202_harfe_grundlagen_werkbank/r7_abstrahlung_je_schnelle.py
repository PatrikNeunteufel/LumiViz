"""R7: Abstrahlung je Schnelle p/v und wirksame Flaeche S_eff (Gl. A1, A2) - Modell (Koerperdatei) gegen Le Carrou
(Thèse Abb. 3.5 Mobilitaet an Punkt 34; Le Carrou 2007 Abb. 7 p/F bei Anregung an 34, 2,35 m); dazu Abstrahlungsguete
je Mode mit der gemessenen S_eff (Gl. D3)."""
import json, math
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
k = json.load(open(p, encoding="utf-8"))
modes, keys = k["modes"], k["bridgePoints"]
RHO, C = 1.2, 343.0
def key_of(hz): return int(round(69 + 12 * math.log2(hz / 440.0))) + 1
def Y(b, f):
    w = 2 * math.pi * f; s = 0j
    for m in modes:
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        s += phi * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return 1j * w * s
def pF(b, f):
    w = 2 * math.pi * f; s = 0j
    for m in modes:
        wk = 2 * math.pi * m["f"]; phi = m["bridge"][b]
        s += complex(*m["radiation"][0]) * (-w * w) * phi / (wk * wk - w * w + 1j * m["eta"] * wk * wk)
    return s
# Punkt 34 liegt zwischen den Saiten A-flat / B-flat (103,8 / 116,5 Hz) = Bell-Saiten 33 / 32
b33, b32 = keys.index(key_of(104)), keys.index(key_of(116))
# Le Carrou: (f, |v34/F| [s/kg] Thèse Abb. 3.5 (Stufe 1), |p/F| [Pa/N] Le Carrou 2007 Abb. 7 bei 2,35 m (S197 Par. 14))
LC = [(152, 6.8e-3, 14e-3), (168, 4.6e-3, 14e-3), (204, 5.8e-3, 27e-3), (245, 0.8e-3, 4e-3), (295, 0.4e-3, 2e-3)]
print("f    LC: p/v[Pa s/m]@2,35m  S_eff[m2] | Modell: |Y|dB  p/F@1m  p/v@2,35m  S_eff[m2]")
for f, v, pp in LC:
    w = 2 * math.pi * f
    pv = pp / v; S = pv * 4 * math.pi * 2.35 / (RHO * w)
    ym = (abs(Y(b33, f)) + abs(Y(b32, f))) / 2; pm = (abs(pF(b33, f)) + abs(pF(b32, f))) / 2
    pvm = pm / 2.35 / ym   # 1/r von 1 m auf 2,35 m (Fernfeld angenommen)
    Sm = pvm * 4 * math.pi * 2.35 / (RHO * w)
    print(f"{f:4d}  {pv:6.2f}               {S:.3f}    | {20*math.log10(ym)+116:5.1f}  {pm:.3f}  {pvm:6.2f}    {Sm:.3f}")
# Abstrahlungsguete je Mode, Punktmasse an der Oese der Saite 32 (1/phi^2), R = rho w^2 S^2 /(4 pi c) (Vollraum)
print("\nMode f   M_Punkt(Saite 32)[kg]  Q_rad mit S_eff 0,05 / 0,09 m2   Q gesamt (Datei)")
for m in modes[:6]:
    f = m["f"]; w = 2 * math.pi * f; phi = m["bridge"][b32]
    M = 1 / phi ** 2
    q = [w * M / (RHO * w * w * S * S / (4 * math.pi * C)) for S in (0.05, 0.09)]
    print(f"{f:6.1f}  {M:8.2f}      {q[0]:8.0f} / {q[1]:6.0f}      {1/m['eta']:.1f}")
# Le Carrou T1: Punktmasse aus Abb. 3.5 (Q 22, 6,8e-3 s/kg bei 152 Hz) und aus Tab. II (200 g senkt 153,5 -> 148,5 Hz)
print("\nLe Carrou T1: M = Q/(w Y) =", round(22 / (2 * math.pi * 152 * 6.8e-3), 2), "kg;  Massenprobe: M = dm/((f/fm)^2-1) =",
      round(0.2 / ((153.5 / 148.5) ** 2 - 1), 2), "kg (200 g) bzw.", round(0.4 / ((153.5 / 148.5) ** 2 - 1), 2), "kg (2 x 200 g)")

"""R8: (a) Bell Abb. 7.22 gegen eine Reihenfeder Y = w/k (Gl. M1); (b) Saiten als Zusatz an allen 46 Oesen, im
Modenraum der Koerperdatei: Frequenzverschiebung durch Laengsfeder, Querfeder (Vorspannung), Guete durch den
Wellenwiderstand gedaempfter Saiten (Gl. S5-S7); gegen Bells gemessene Verschiebungen (S197 Par. 8)."""
import json, math, sys
sys.path.insert(0, r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen")
import importlib.util
spec = importlib.util.spec_from_file_location("r1", r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz_Research\ergebnisse\2026-11-16_12_harfe_grundlagen\r1_saiten.py")

# (a) Reihenfeder
F = [104, 116, 123, 139, 155, 165, 185, 207, 233, 247, 277, 311, 330, 370, 415, 466, 494, 554, 622, 659, 740, 830, 932, 988]
B = [80, 89, 74, 79, 81, 82, 85, 86, 84, 87, 88, 89, 95, 85, 89, 89, 91, 90, 93, 93, 99, 103, 98, 99]
ks = sorted(2 * math.pi * f / 10 ** ((L - 116) / 20) for f, L in zip(F, B))
k = ks[len(ks) // 2]
res = [L - (20 * math.log10(2 * math.pi * f / k) + 116) for f, L in zip(F, B)]
sl = (sum((math.log10(f) - 2.5) * L for f, L in zip(F, B)) / sum((math.log10(f) - 2.5) ** 2 for f in F))
mF = sum(math.log10(f) for f in F) / len(F); mB = sum(B) / len(B)
steig = sum((math.log10(f) - mF) * (L - mB) for f, L in zip(F, B)) / sum((math.log10(f) - mF) ** 2 for f in F)
print(f"(a) Reihenfeder k = {k:.3g} N/m (Median); Rest Bell - Feder: Mittel {sum(res)/len(res):+.1f} dB, "
      f"Streuung {math.sqrt(sum(r*r for r in res)/len(res)):.1f} dB, groesster {max(res, key=abs):+.1f} dB")
print(f"    Steigung von Bell 7.22 (Regression): {steig:.1f} dB/Dekade = {steig*math.log10(2):.1f} dB/Oktave (Feder: 20 / 6,0)")

# (b) Saiten auf die Moden
import r1_saiten as r1  # druckt die Tabelle erneut; nur Daten verwenden
p = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\asset\loop\instrument_bodies\concert_harp.lvinstbody"
kd = json.load(open(p, encoding="utf-8"))
modes, keys = kd["modes"], kd["bridgePoints"]
def key_of(hz): return int(round(69 + 12 * math.log2(hz / 440.0))) + 1
S2, C2 = r1.S2, r1.C2
print("\n(b) Mode  f    | Laengsfeder Hals starr: f' (df%) | Querfeder T/L: df% | gedaempft R=Z0 cos2: Q vorher -> nachher")
for m in modes[:12]:
    wk = 2 * math.pi * m["f"]
    dk_l = dk_t = dr = 0.0
    for name, s in r1.daten.items():
        b = keys.index(key_of(s["f"]))
        phi2 = m["bridge"][b] ** 2
        dk_l += s["kl"] * S2 * phi2
        dk_t += s["T"] / s["L"] * C2 * phi2
        dr += s["Z0"] * C2 * phi2
    f_l = math.sqrt(wk * wk + dk_l) / (2 * math.pi)
    f_t = math.sqrt(wk * wk + dk_t) / (2 * math.pi)
    eta2 = m["eta"] + dr / wk
    print(f"     {m['f']:6.1f} | {f_l:6.1f} ({100*(f_l/m['f']-1):+5.1f} %)         | {100*(f_t/m['f']-1):+5.2f} %   | "
          f"{1/m['eta']:5.1f} -> {1/eta2:5.1f}")

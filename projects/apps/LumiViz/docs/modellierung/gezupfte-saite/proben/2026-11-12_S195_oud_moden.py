import json, math, sys

k = json.load(open(sys.argv[1], encoding="utf-8"))
teile = []
for m in k["modes"][:8]:
    g = m["radiation"][0]
    betrag = math.hypot(g[0], g[1]) if isinstance(g, list) else abs(g)
    steg = math.sqrt(sum(v * v for v in m["bridge"]) / len(m["bridge"]))
    teile.append(f"{m['f']:.0f} Hz (G {betrag:.3f}, Steg {steg:.2f})")
print("; ".join(teile))

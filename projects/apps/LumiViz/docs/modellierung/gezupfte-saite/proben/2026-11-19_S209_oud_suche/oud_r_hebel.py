"""S209 - Familie R (KQ42 a): Hebel einzeln um die beste Fassung der Feinsuche; eine Zeile je Fassung."""
import copy

import oud_kombi as k
import oud_raster_b as rb
import oud_suche as s

BASIS = dict(dicke=0.00195, abstand=0.002, steg=(0.17, 0.020, 0.0092, 650.0, 10e9), volumen=0.0239, muendung=1.38)


def lauf(name, **kw):
    a = copy.deepcopy(BASIS)
    a.update(kw)
    e = s.rechne(k.fassung(**a), "r_hebel", zeigen=False)
    if e:
        print(rb.zeile(e, name), flush=True)


print("Fassung\tgroesste Lage %\tLagen 139 203 280 362 %\tZ2 Raster\tZ2 Punkt\tLuft\tHaupt\tZ3\tLuft dB\tTal dB\tT2\tT3\tModen (Luftanteil|Steg rel.)\tSpitzen am Antwortpunkt (Raster)")
lauf("beste R")
for n in ("brace 1", "brace 2", "brace 3", "brace 6", "brace 7"):
    for f in (0.7, 1.3):
        lauf(f"{n} x{f}", balken={n: f})
lauf("brace 4+5 x0.7", balken={"brace 4": 0.7, "brace 5": 0.7})
lauf("brace 4+5 x1.3", balken={"brace 4": 1.3, "brace 5": 1.3})
for L, b, h, rho in ((0.13, 0.020, 0.0092, 650.0), (0.17, 0.020, 0.0092, 900.0), (0.17, 0.012, 0.0092, 1080.0), (0.17, 0.030, 0.0062, 650.0), (0.17, 0.020, 0.0130, 460.0), (0.20, 0.020, 0.0092, 650.0)):
    lauf(f"Steg {L * 100:.0f} cm x {b * 1000:.0f} x {h * 1000:.1f} mm, {rho:.0f} ({1000 * L * b * h * rho:.0f} g)", steg=(L, b, h, rho, 10e9))
for ab in (0.001, 0.003):
    lauf(f"Abstand {ab * 1000:.0f} mm", abstand=ab)
for d in (0.0018, 0.0021):
    lauf(f"Decke {d * 1000:.2f} mm", dicke=d)
for halt, feder in (("elastic", 3000.0), ("elastic", 300.0)):
    lauf(f"Rand Drehfeder {feder:.0f} N", halt=halt, feder=feder)

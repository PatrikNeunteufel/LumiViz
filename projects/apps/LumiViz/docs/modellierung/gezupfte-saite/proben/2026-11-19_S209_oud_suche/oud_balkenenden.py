"""S209 - Abstand der Balkenenden (Laengen aus Tab. 2) zum Umriss (aus Abb. 3 gelesen) je Balken."""
import oud_kombi as k

p = k.B
for r in p["ribs"]:
    y = r["line"][0][1]
    hb = k.halbe_breite(p, y)
    aussen = max(abs(r["line"][0][0]), abs(r["line"][2][0]))
    print(f"{r['name']}: y {100 * y:5.2f} cm, halbe Breite des Umrisses {100 * hb:5.2f} cm, Balken bis {100 * aussen:5.2f} cm, Abstand {1000 * (hb - aussen):5.1f} mm")
u = p["plate"]["outline"]
print("Umriss: Breite", round(100 * (max(q[0] for q in u) - min(q[0] for q in u)), 2), "cm, Laenge", round(100 * (max(q[1] for q in u) - min(q[1] for q in u)), 2), "cm")

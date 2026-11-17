"""Cent-Rechnungen fuer den Bericht (eigene Rechnung)."""
from math import log2

def c(r):
    return 1200 * log2(r)

print("Halbton-Faktor-1:", 2 ** (1 / 12) - 1)
paare = {
    "Schneider Fis 340 + 19 cm": (340 + 19, 340),
    "Lechgold F 360 / Fis 347": (360, 347),
    "Thomann F 363 / Fis 343": (363, 343),
    "Thomann Handrohr 118 / 98 -> Gesamt": (363, 343),
    "resunar F 361 / Fis 344": (361, 344),
    "Lechgold Fis 347 / resunar Fis 344 (o. MS)": (347, 344),
    "Lechgold Fis 347 / Schneider Fis 340": (347, 340),
    "Fis 347 / Tab.3 324,4": (347, 324.41),
}
for k, (a, b) in paare.items():
    print(f"{k}: {c(a / b):+.1f} Cent, Delta {a - b:.1f} cm, proportional noetig {b * (2 ** (1 / 12) - 1):.1f} cm")
for p in (1.1, -12.1, 8.1, -10.0, 8.9, -0.4, -0.6):
    print(f"{p:+.1f} % = {c(1 + p / 100):+.1f} Cent")

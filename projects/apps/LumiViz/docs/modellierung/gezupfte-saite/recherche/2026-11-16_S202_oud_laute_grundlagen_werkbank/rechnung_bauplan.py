"""Gegenrechnungen zum Bauplan Oud / Laute (Auftrag 2026-11-16_15). Liest nur die Umriss-Dateien im LumiViz-Repo.
Aufruf: python -I <dieses Skript>
"""
import json
import math

PROBEN = r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\gezupfte-saite\proben"
C_LUFT = 343.0


def flaeche_stationen(st, links, rechts):
    return sum((st[k + 1] - st[k]) * ((rechts[k] - links[k]) + (rechts[k + 1] - links[k + 1])) / 2 for k in range(len(st) - 1))


def helmholtz(V, loecher, dicke, muendung=1.7):
    """f = c/2pi sqrt( sum A_i / l_i / V ), l_i = Dicke + muendung * r_i (Bauplan.hpp: Muendungskorrektur in Radien)."""
    leit = sum(math.pi * r * r / (dicke + muendung * r) for r in loecher)
    return C_LUFT / (2 * math.pi) * math.sqrt(leit / V)


def entkoppelt(f_minus, f_plus, f_luft):
    """Zwei gekoppelte Schwinger: f-^2 + f+^2 = f_Luft^2 + f_Decke^2 (S196 Par. 5) -> f_Decke."""
    r = f_minus ** 2 + f_plus ** 2 - f_luft ** 2
    return math.sqrt(r) if r > 0 else float("nan")


# ---------------- Oud (Abdallah u. a. 2026, Umriss aus Abb. 3, S195)
o = json.load(open(PROBEN + r"\2026-11-12_S195_oud_umriss.json", encoding="utf-8-sig"))
st = [u[0] / 100 for u in o["umriss"]]
A_oud = flaeche_stationen(st, [u[1] / 100 for u in o["umriss"]], [u[2] / 100 for u in o["umriss"]])
loecher_abd = [0.053, 0.020, 0.020]
loecher_ina = [0.0425, 0.021, 0.021]          # Inanli & Altinsoy 2018 Abb. 2: Durchmesser 85 / 42 / 42 mm
A_l = sum(math.pi * r * r for r in loecher_abd)
print(f"Oud: Umriss {A_oud * 1e4:.0f} cm^2 (Laenge {st[-1] * 100:.1f} cm, groesste Breite {o['groesste_breite_cm']:.1f} cm); "
      f"Loecher Abdallah {A_l * 1e4:.0f} cm^2; netto {(A_oud - A_l) * 1e4:.0f} cm^2")
V = 0.020613
for d in (0.0015, 0.0020):
    m_platte = (A_oud - A_l) * d * 460
    print(f"   Decke {d * 1e3:.1f} mm, Sitka 460 kg/m3: {m_platte * 1e3:.0f} g")
BALKEN = [(26.0, 10.0, 4.2), (34.0, 8.0, 4.2), (33.7, 7.0, 5.0), (9.8, 7.0, 6.6), (9.8, 7.0, 6.6), (25.5, 10.0, 4.2), (22.0, 10.0, 4.2)]
m_balken = sum(L / 100 * ((c + 16.5) / 2 / 1000) * b / 1000 * 460 for L, c, b in BALKEN)   # Hoehe linear c -> 16,5 -> c
print(f"   Balken (make_oud.py: Mitte 16,5 mm, Enden c, Breite Tab. 2): {m_balken * 1e3:.1f} g")
INANLI = [(300, 200, 50, 12, 6, 5), (355, 255, 50, 12, 6, 6), (355, 255, 50, 5, 6, 5), (344, 244, 50, 12, 6, 5), (118, 48, 35, 12, 6, 5),
          (278, 178, 50, 12, 6, 6), (222, 122, 50, 12, 6, 6)]
m_ina = sum((L2 * h1 + 2 * L3 * (h1 + h2) / 2) * w for L1, L2, L3, h1, h2, w in INANLI) * 1e-9 * 460
print(f"   Balken Inanli Tab. 1 (L2 voll, L3 linear auf h2), bei 460 kg/m3: {m_ina * 1e3:.1f} g; Pruefung L1 = L2 + 2 L3: "
      + ", ".join(str(L1 - L2 - 2 * L3) for L1, L2, L3, *_ in INANLI))
for name, l in (("Abdallah (R 5,3 / 2,0 / 2,0 cm)", loecher_abd), ("Inanli (85 / 42 / 42 mm)", loecher_ina)):
    for d in (0.0015, 0.002):
        print(f"   Helmholtz starr, V {V * 1e3:.2f} l, {name}, Wand {d * 1e3:.1f} mm: {helmholtz(V, l, d):.0f} Hz")
fh = helmholtz(V, loecher_abd, 0.0015)
print(f"   gemessen (Abdallah Abb. 6, S196): Luft 101 Hz, erste Deckenmode 203 Hz -> entkoppelte Decke "
      f"{entkoppelt(101, 203, fh):.0f} Hz (mit f_Luft = {fh:.0f} Hz)")
for V_ in (0.016, 0.018, 0.020613, 0.023):
    print(f"   Volumen {V_ * 1e3:.1f} l -> Helmholtz starr {helmholtz(V_, loecher_abd, 0.0015):.0f} Hz")

# ---------------- Laute (Rauwolf, Massstab 625/677,4)
la = json.load(open(PROBEN + r"\2026-11-13_S197_laute_umriss.json", encoding="utf-8-sig"))
s = 625 / 677.4
st = [u[0] / 1000 for u in la["umriss"]]
A_la = flaeche_stationen(st, [u[1] / 1000 for u in la["umriss"]], [u[2] / 1000 for u in la["umriss"]])
r_rose = la["rose"]["durchmesser"] / 2000
print()
print(f"Laute Rauwolf 1:1: Umriss {A_la * 1e4:.0f} cm^2, Rose {math.pi * r_rose ** 2 * 1e4:.0f} cm^2; mal Massstab^2: {A_la * s * s * 1e4:.0f} cm^2")
for d in (0.0015, 0.0018, 0.0020):
    m = (A_la - 0.85 * math.pi * r_rose ** 2) * s * s * d * 460
    print(f"   Decke {d * 1e3:.1f} mm (gestreckt, Rose 85 % offen), 460 kg/m3: {m * 1e3:.0f} g")
quer = [q["flaeche_mm2"] for q in la["querschnitte"]]
print(f"   Querschnitte (mm^2): {[round(q) for q in quer]}")
V1 = 0.0140
for offen in (0.5, 0.85, 1.0):
    for V_, name in ((V1, "1:1, 14,0 l"), (V1 * s ** 3, f"gestreckt {V1 * s ** 3 * 1e3:.1f} l")):
        r = (r_rose * math.sqrt(offen)) * (1 if V_ == V1 else s)
        print(f"   Helmholtz starr, {name}, Rose {offen * 100:.0f} % offen (flaechengleicher Kreis r {r * 1e3:.1f} mm), Decke 1,8 mm: "
              f"{helmholtz(V_, [r], 0.0018):.0f} Hz")
fh = helmholtz(V1 * s ** 3, [r_rose * math.sqrt(0.85) * s], 0.0018)
print(f"   gemessen (Woodhouse 2021 Abb. 1, S197): Luft 132 Hz, Decke 277 Hz -> entkoppelte Decke {entkoppelt(132, 277, fh):.0f} Hz (f_Luft {fh:.0f} Hz)")
# Steg nach der Zeichnung (dieser Lauf, Bild seiten/rauwolf_steg.png): Laenge 139, Breite 10 -> 7, Hoehe 7 -> 6 (vorn) bzw. 8 -> 7 (hinten)
for rho in (600.0, 700.0, 750.0):
    vol = 139e-3 * (10e-3 + 7e-3) / 2 * (7.5e-3 + 6.5e-3) / 2
    print(f"   Steg Rauwolf (Quader-Naeherung 139 x 8,5 x 7 mm = {vol * 1e6:.2f} cm^3), {rho:.0f} kg/m3: {vol * rho * 1e3:.1f} g, "
          f"gestreckt (Massstab^3) {vol * rho * s ** 3 * 1e3:.1f} g")

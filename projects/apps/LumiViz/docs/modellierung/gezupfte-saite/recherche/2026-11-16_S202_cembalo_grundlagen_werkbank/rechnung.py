"""LVR, Auftrag 2026-11-16_14_cembalo_grundlagen: Gegenrechnungen zu Kiel, Stoss, Lautenzug, Draht, Register.

Aufruf: python ergebnisse/2026-11-16_14_cembalo_grundlagen/rechnung.py
Nur Standardbibliothek. Parameter der Saiten aus dem Parameterblock des LumiViz-Pruefstands (S199/S200, nur gelesen).
"""
import cmath
import math

# ------------------------------------------------------------------ Saiten (aus S199 / S200 Parameterbloecken)
TASKIN = [(29, 1765, 208), (36, 1681, 191), (41, 1443, 179), (48, 1178, 162), (53, 982, 151), (60, 711, 137), (65, 535, 127),
          (72, 357, 115), (77, 268, 107), (84, 186, 96), (89, 148, 91)]           # Taste, L mm, Zupf mm ab Sattel
DRAHT8 = [(31, 0.620, 8700, 115), (33, 0.572, 8700, 115), (36, 0.518, 8700, 115), (38, 0.463, 8700, 115), (40, 0.463, 8500, 110),
          (43, 0.410, 8500, 110), (45, 0.377, 8500, 110), (47, 0.339, 8500, 110), (51, 0.339, 7800, 170), (56, 0.300, 7800, 170),
          (64, 0.269, 7800, 170), (75, 0.244, 7800, 170), (89, 0.213, 7800, 170)]
VIER_MENSUR = [(29, 1073), (41, 773), (53, 486), (65, 255), (77, 124), (89, 74)]
DRAHT4 = [(31, 0.463, 8700, 115), (35, 0.410, 8700, 115), (38, 0.377, 8700, 115), (40, 0.377, 8500, 110), (42, 0.339, 8500, 110),
          (45, 0.300, 8500, 110), (49, 0.300, 7800, 170), (55, 0.269, 7800, 170), (72, 0.244, 7800, 170), (79, 0.213, 7800, 170),
          (89, 0.188, 7800, 170)]
NAME = {29: "F1", 36: "C2", 41: "F2", 48: "C3", 53: "F3", 55: "G3", 60: "C4", 65: "F4", 67: "G4", 72: "C5", 77: "F5", 84: "C6", 89: "f'''"}
TASTEN = [29, 36, 48, 60, 72, 84, 89]


def hz(t, a4=440.0):
    return a4 * 2 ** ((t - 69) / 12)


def logint(t, pts, spalte):
    xs = [p[0] for p in pts]
    ys = [math.log(p[spalte]) for p in pts]
    if t <= xs[0]:
        return math.exp(ys[0])
    for i in range(len(xs) - 1):
        if xs[i] <= t <= xs[i + 1]:
            w = (t - xs[i]) / (xs[i + 1] - xs[i])
            return math.exp(ys[i] + w * (ys[i + 1] - ys[i]))
    return math.exp(ys[-1])


def saite8(t, a4=440.0):
    L = logint(t, TASKIN, 1) / 1000
    x = logint(t, TASKIN, 2) / 1000                     # Zupf ab Sattel
    d, rho, E = next((d, r, e) for bis, d, r, e in DRAHT8 if t <= bis)
    return mach(L, x, d / 1000, rho, E * 1e9, hz(t, a4))


def saite4(t, a4=440.0):
    L = logint(t, VIER_MENSUR, 1) / 1000
    L8 = logint(min(t + 12, 89), TASKIN, 1) / 1000
    x8 = logint(min(t + 12, 89), TASKIN, 2) / 1000
    x = min(x8 / L8, 0.5) * L                          # Regel des Pruefstands S200
    d, rho, E = next((d, r, e) for bis, d, r, e in DRAHT4 if t <= bis)
    return mach(L, x, d / 1000, rho, E * 1e9, 2 * hz(t, a4))


def mach(L, x, d, rho, E, f):
    mu = rho * math.pi * d * d / 4
    T = 4 * L * L * f * f * mu
    I = math.pi * d ** 4 / 64
    return {"L": L, "x": x, "d": d, "rho": rho, "E": E, "f": f, "mu": mu, "T": T, "Z0": math.sqrt(T * mu),
            "K": T * L / (x * (L - x)), "B": math.pi ** 3 * E * d ** 4 / (64 * T * L * L), "EI": E * I}


# ------------------------------------------------------------------ Kiel als Biegebalken (Perng 2.57/2.74; Chadefaux Gl. 4)
class Kiel:
    """Gamma(l) = F g(l): Biegewinkel klein (Perng Gl. 2.57 bzw. Chadefaux Gl. 4 mit zwei Abschnitten, I/8 vorn)."""

    def __init__(self, E, I, Lp, L1=None, n=2000):
        self.Lp = Lp
        ls = [Lp * i / n for i in range(n + 1)]
        g = []
        for l in ls:
            if L1 is None or l <= L1:
                g.append((Lp * l - l * l / 2) / (E * I))
            else:
                g.append((3.5 * L1 * L1 - 7 * Lp * L1 + 8 * Lp * l - 4 * l * l) / (E * I))
        h = Lp / n
        self.gL = g[-1]                                              # theta0 = F gL
        self.G2 = sum((g[i] ** 2 + g[i + 1] ** 2) / 4 for i in range(n)) * h   # Verkuerzung = F^2 G2
        self.G1 = sum((g[i] + g[i + 1]) / 2 for i in range(n)) * h          # Senkung der Spitze = F G1

    def abloesen(self, K, u):
        """Abloesekraft F: Verkuerzung + seitliches Ausweichen der Saite = Ueberstand u (eigene Herleitung)."""
        f = lambda F: F * F * self.G2 + F * math.sin(F * self.gL) / K - u
        # erste Wurzel suchen (f ist fuer theta > 90 Grad nicht mehr monoton; das Modell gilt dort ohnehin nicht)
        lo, hi, schritt = 0.0, 0.0, 1e-3
        while f(hi) < 0 and hi * self.gL < math.pi / 2:
            lo, hi = hi, hi + schritt
        for _ in range(200):
            mid = (lo + hi) / 2
            lo, hi = (mid, hi) if f(mid) < 0 else (lo, mid)
        F = (lo + hi) / 2
        th = F * self.gL
        return {"F": F, "theta": th, "xs": F * math.sin(th) / K, "ys": F * math.cos(th) / K, "A": F / K,
                "spitze": F * self.G1}


def kopf(t):
    print("\n" + "=" * 110 + "\n" + t + "\n" + "=" * 110)


# ------------------------------------------------------------------ 0 Saiten
kopf("0  Saiten des Pruefstands (8' Taskin 1769 / Di Veroli; a' = 440 Hz wie hz_von im Pruefstand), dazu 415 Hz")
print("Taste  | L mm | Zupf mm | x/L   | d mm  | T440 N | T415 N | Z0 kg/s | K N/m  | B")
for t in TASTEN + [67]:
    s, s4 = saite8(t), saite8(t, 415)
    print(f"{NAME[t]:6s} | {1000*s['L']:4.0f} | {1000*s['x']:7.0f} | {s['x']/s['L']:.3f} | {1000*s['d']:.3f} | {s['T']:6.1f} | {s4['T']:6.1f} |"
          f" {s['Z0']:7.3f} | {s['K']:6.0f} | {s['B']:.2e}")

# ------------------------------------------------------------------ 1 Kiel: Pruefung gegen Perng 2012
kopf("1a Reduziertes Kielmodell gegen Perngs Simulation (Diss. 2012 Tab. 3.1; Abb. 3.15, 3.16, 3.19, 3.21 abgelesen)")
Wp, Hp = 2e-3, 0.5e-3
Ip = Wp * Hp ** 3 / 12


def perng(E=5e9, Lp=6e-3, Li=5.4e-3, Ls=0.5, T=135.0, x=0.1):
    k = Kiel(E, Ip, Lp)
    K = T * Ls / (x * (Ls - x))
    return k.abloesen(K, Lp - Li)


faelle = [("Diskant 1/5, Abb. 3.16a", {"x": 0.1}, 1.20), ("Diskant Mitte, Abb. 3.16a", {"x": 0.25}, 1.60),
          ("Bass 1/7, Abb. 3.15", {"Ls": 2.0, "T": 42.0, "x": 2 / 7}, 4.40), ("Bass Mitte, Abb. 3.16b", {"Ls": 2.0, "T": 42.0, "x": 1.0}, 6.50),
          ("Diskant 1/5, Lp 5,5 mm, Abb. 3.19a", {"x": 0.1, "Lp": 5.5e-3}, 0.58), ("Diskant 1/5, Lp 5,7 mm", {"x": 0.1, "Lp": 5.7e-3}, 0.88),
          ("Bass 1/7, Lp 5,5 mm, Abb. 3.19b", {"Ls": 2.0, "T": 42.0, "x": 2 / 7, "Lp": 5.5e-3}, 2.00),
          ("Bass 1/7, Lp 5,7 mm", {"Ls": 2.0, "T": 42.0, "x": 2 / 7, "Lp": 5.7e-3}, 3.30),
          ("Diskant Mitte, E 1 GPa, Abb. 3.21a", {"x": 0.25, "E": 1e9}, 0.55), ("Diskant Mitte, E 0,1 GPa", {"x": 0.25, "E": 1e8}, 0.10),
          ("Diskant Mitte, E 30 GPa", {"x": 0.25, "E": 3e10}, 4.30), ("Bass Mitte, E 1 GPa, Abb. 3.21b", {"Ls": 2.0, "T": 42.0, "x": 1.0, "E": 1e9}, 3.0),
          ("Bass Mitte, E 0,1 GPa", {"Ls": 2.0, "T": 42.0, "x": 1.0, "E": 1e8}, 0.7), ("Bass Mitte, E 30 GPa", {"Ls": 2.0, "T": 42.0, "x": 1.0, "E": 3e10}, 15.5)]
print("Fall                                  | A Perng mm | A Modell mm | Abw.  | F_ab N | theta Grad")
for name, kw, soll in faelle:
    r = perng(**kw)
    print(f"{name:37s} | {soll:10.2f} | {1000*r['A']:11.2f} | {100*(1000*r['A']/soll-1):+4.0f} % | {r['F']:6.2f} | {math.degrees(r['theta']):5.1f}")

kopf("1b Gegen die Messung Chadefaux u. a. 2013 (G4, Tab. 1 und 2; Kiel aus zwei Abschnitten, Gl. 4)")
kc = Kiel(1.5e9, 0.011e-12, 3.5e-3, L1=2.9e-3)
Ls, T = 0.475, 85.7
s67 = saite8(67)
beta = s67["x"] / s67["L"]
Kc = T / (Ls * beta * (1 - beta))
print(f"Zupfanteil G4 aus Taskin 1769 (Quelle nennt keinen): beta = {beta:.3f}; K = {Kc:.0f} N/m; Taskin-Laenge G4 {1000*s67['L']:.0f} mm (Quelle 475)")
print(f"Kiel: theta0/F = {math.degrees(kc.gL):.1f} Grad/N; Spitze senkt sich {1000*kc.G1:.2f} mm/N")
for soll_y, name in ((0.71e-3, "langsam"), (0.90e-3, "schnell")):
    lo, hi = 0.0, 3e-3
    for _ in range(100):
        u = (lo + hi) / 2
        lo, hi = (u, hi) if kc.abloesen(Kc, u)["ys"] < soll_y else (lo, u)
    r = kc.abloesen(Kc, u)
    print(f"{name:8s}: Ueberstand fuer y_s = {1000*soll_y:.2f} mm: u = {1000*u:.2f} mm -> F_ab {r['F']:.2f} N, x_s {1000*r['xs']:.2f} mm "
          f"(Messung {'0,27' if name == 'langsam' else '0,28'}), theta {math.degrees(r['theta']):.1f} Grad (Modell der Quelle 20,9; Gamma_tr {'17' if name == 'langsam' else '20'})")
    if name == "langsam":
        U_C = u
F_direkt = [Kc * math.hypot(xx, yy) for xx, yy in ((0.27e-3, 0.71e-3), (0.28e-3, 0.90e-3))]
print(f"Kraft direkt aus der gemessenen Lage (F = K * sqrt(x^2 + y^2)): langsam {F_direkt[0]:.2f} N, schnell {F_direkt[1]:.2f} N")

# Zeitverlauf
print("\nAufladezeit = (y_s + Senkung der Kielspitze) / v_Springer:")
for v, name, soll in ((0.10, "normal", "12 +- 2"), (0.25, "schnell", "7,9 +- 0,6")):
    r = kc.abloesen(Kc, U_C if name == "normal" else U_C)
    yy = 0.71e-3 if name == "normal" else 0.90e-3
    Fm = F_direkt[0] if name == "normal" else F_direkt[1]
    print(f"  {name:7s}: v {v} m/s: ({1000*yy:.2f} + {1000*Fm*kc.G1:.2f}) mm / v = {1000*(yy+Fm*kc.G1)/v:.1f} ms (gemessen {soll} ms)")

# ------------------------------------------------------------------ 1c Gesetz je Taste
kopf("1c Abloesekraft je Taste (8'), Kiel nach Chadefaux; Vergleich mit GZ13 a (2 N an C4, -2,8 dB/Oktave)")
d67 = s67["d"]
print(f"Option A: Ueberstand fest u = {1000*U_C:.2f} mm (an G4 auf die Messung gelegt). Option B: u = {U_C/d67:.2f} * d (Saitendurchmesser).")
print("Option C: Kiel je Taste so geschnitten, dass die Senkung der Spitze gleich bleibt - entspricht A; nicht getrennt.")
print("Taste  | K N/m | A: F N | A dB re C4 | A: y_s mm | B: F N | B dB re C4 | GZ13 F N | GZ13 dB | LumiViz y=F/K mm")
ref = {}
rows = []
for t in TASTEN + [67]:
    s = saite8(t)
    a = kc.abloesen(s["K"], U_C)
    b = kc.abloesen(s["K"], U_C / d67 * s["d"])
    gz = 2.0 * 10 ** (-2.8 / 20 * (t - 60) / 12)
    rows.append((t, s, a, b, gz))
    if t == 60:
        ref = {"a": a["F"], "b": b["F"]}
for t, s, a, b, gz in rows:
    print(f"{NAME[t]:6s} | {s['K']:5.0f} | {a['F']:6.2f} | {20*math.log10(a['F']/ref['a']):+10.1f} | {1000*a['ys']:9.2f} | {b['F']:6.2f} |"
          f" {20*math.log10(b['F']/ref['b']):+10.1f} | {gz:8.2f} | {20*math.log10(gz/2):+7.1f} | {1000*gz/s['K']:.2f}")


def steigung(key):
    import statistics
    xs = [(t - 60) / 12 for t, *_ in rows if t != 67]
    ys = [20 * math.log10(key(r)) for r in rows if r[0] != 67]
    mx, my = statistics.mean(xs), statistics.mean(ys)
    return sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sum((x - mx) ** 2 for x in xs)


print(f"Mittlere Steigung (Regression ueber F1..f'''): A {steigung(lambda r: r[2]['F']):+.1f} dB/Okt, B {steigung(lambda r: r[3]['F']):+.1f} dB/Okt, GZ13 -2,8")

kopf("1d 4' (Pruefstand-Regel fuer den Zupfpunkt), gleicher Kiel und Ueberstand wie Option A")
print("Taste  | L4 mm | Zupf mm | T4 N  | K4 N/m | F_ab 4' N | F_ab 8' N | 4'/8' dB")
for t in TASTEN:
    s4 = saite4(t)
    a4 = kc.abloesen(s4["K"], U_C)
    a8 = kc.abloesen(saite8(t)["K"], U_C)
    print(f"{NAME[t]:6s} | {1000*s4['L']:5.0f} | {1000*s4['x']:7.0f} | {s4['T']:5.1f} | {s4['K']:6.0f} | {a4['F']:9.2f} | {a8['F']:9.2f} | {20*math.log10(a4['F']/a8['F']):+.1f}")

kopf("1e Spektrum: Abloesen in tau (lineare Rampe) gegen den idealen Sprung; raeumliche Breite des Kiels (Pruefstand)")
for tau in (0.1e-3, 0.25e-3, 0.5e-3):
    print(f"tau {1000*tau:.2f} ms: -3 dB bei {0.443/tau:6.0f} Hz, erste Nullstelle {1/tau:6.0f} Hz")
print("Raeumlicher Gauss exp(-(w h / c)^2), -3 dB bei f = 0,0935 c / h:")
for t in TASTEN:
    s = saite8(t)
    c = 2 * s["L"] * s["f"]
    print(f"  {NAME[t]:5s} c = {c:5.0f} m/s: h 3 mm -> {0.0935*c/0.003:6.0f} Hz; h 2 mm (Perng W) -> {0.0935*c/0.002:6.0f} Hz")

# ------------------------------------------------------------------ 2 Tastenstoss
kopf("2  Tastenstoss: Kraftstoesse J = m dv (1 + e), Annahmen markiert")
for name, m, v in (("Springer an der Springerleiste, langsam (Perng Abb. 4.24, Spitze 0,25 m/s)", 0.005, 0.25),
                   ("Springer an der Springerleiste, kraeftig (Perng Abb. 4.25, Spitze 0,58 m/s)", 0.005, 0.58),
                   ("Springer, Kunststoff 6 g, kraeftig", 0.006, 0.58),
                   ("Rueckfall des Springers aus 3 cm (Perng S. 114) auf die Taste", 0.005, math.sqrt(2 * 9.81 * 0.03))):
    print(f"{name:78s}: m {1000*m:.0f} g, v {v:.2f} m/s -> J {1000*m*v:.1f} (e=0) .. {2000*m*v:.1f} (e=1) mN s")
print("Taste am Boden: Verzoegerung aus Chadefaux 2013 Abb. 6 (G4, normal): 0,10 m/s -> 0 in ~8 ms -> a ~ 12 m/s^2")
for meff in (0.02, 0.05, 0.10):
    print(f"  wirksame Masse (Taste + Finger, ANNAHME) {1000*meff:.0f} g: J = {1000*meff*0.10:.1f} mN s, mittlere Kraft {meff*0.10/0.008:.2f} N ueber 8 ms")
for tau_s in (0.4e-3, 2e-3, 8e-3):
    print(f"  Stossdauer {1000*tau_s:.1f} ms (Gauss sigma = Dauer/2,5): -3 dB des Spektrums bei {math.sqrt(math.log(2))/(2*math.pi*tau_s/2.5)*1:.0f} Hz")

# ------------------------------------------------------------------ 3 Lautenzug
kopf("3  Lautenzug: Abschluss Z_T = R + (k + T/a)/(i w) am Ort a vor dem Sattel (Perng Gl. 5.9 + Stummel), je Teilton")


def filz(s, a, R, k, n):
    w = 2 * math.pi * n * s["f"]
    kw = w * math.sqrt(s["mu"] / s["T"])
    zl = R + k / (1j * w) + s["Z0"] / (1j * math.tan(kw * a))      # Stummel exakt wie S200
    r = (s["Z0"] - zl) / (s["Z0"] + zl) * cmath.exp(2j * kw * a)
    sigma = -s["f"] * math.log(abs(r))                              # 1/s, Amplitude
    dphi = cmath.phase(-r)                                          # gegen starren Sattel (r = -1)
    cent = 1200 * math.log2(1 + dphi / (2 * math.pi * n)) if abs(dphi) < 3 else float("nan")
    return sigma, cent


def klein(s, a, R, n):
    """Stoerungsrechnung fuer R << Z0, Stummel vernachlaessigt: sigma = R sin^2(n pi a / L) / (mu L)."""
    return R * math.sin(n * math.pi * a / s["L"]) ** 2 / (s["mu"] * s["L"])


print("Taste | Z0     | Fall                    | sigma_1 | sigma_5 | sigma_10 | sigma_20 1/s | Cent T1 | Cent T10")
for t in (36, 48, 60, 72, 84):
    s = saite8(t)
    for name, a, R, k in (("GZ11 a: 40 N s/m, 0,5 mm", 0.5e-3, 40.0, 0.0), ("40 N s/m, 2,5 mm", 2.5e-3, 40.0, 0.0),
                          ("R = Z0, 0,5 mm", 0.5e-3, s["Z0"], 0.0), ("R = Z0, 2,5 mm", 2.5e-3, s["Z0"], 0.0),
                          ("40 N s/m, 2,5 mm, k 2e4", 2.5e-3, 40.0, 2e4)):
        sg = [filz(s, a, R, k, n) for n in (1, 5, 10, 20)]
        print(f"{NAME[t]:5s} | {s['Z0']:.3f} | {name:23s} | {sg[0][0]:7.2f} | {sg[1][0]:7.2f} | {sg[2][0]:8.2f} | {sg[3][0]:8.2f}     |"
              f" {sg[0][1]:+7.1f} | {sg[2][1]:+7.1f}")
s = saite8(60)
print(f"Probe der Stoerungsrechnung (C4, R = 0,02 Z0, a 2,5 mm, n = 10): exakt {filz(s, 2.5e-3, 0.02*s['Z0'], 0, 10)[0]:.4f}, klein {klein(s, 2.5e-3, 0.02*s['Z0'], 10):.4f} 1/s")
print("Vollstaendiges Klemmen im Abstand a: Verstimmung 1731 * a / L Cent:")
for t in (36, 60, 84):
    s = saite8(t)
    print(f"  {NAME[t]:4s} L {1000*s['L']:.0f} mm: a 0,5 mm -> {1731*0.5e-3/s['L']:.1f} Cent; a 2,5 mm -> {1731*2.5e-3/s['L']:.1f} Cent")

# ------------------------------------------------------------------ 4 Draht
kopf("4  Draht: eta = eta_air (WLA 2019 Gl. 11-13) + lambda * eta_E (Gl. 14), lambda = EI k^2 / (T + EI k^2) ~ n^2 B")


def eta_air(s, f, korr=True):
    M = s["d"] / 4 * math.sqrt(2 * math.pi * f / 1.5e-5)
    e = 1.2 / s["rho"] * (2 * math.sqrt(2) * M + 1) / M ** 2
    return e * ((1000 * s["d"] + 0.2) if korr else 1.0)


def lam(s, n):
    k = n * math.pi / s["L"]
    return s["EI"] * k * k / (s["T"] + s["EI"] * k * k)


print("Taste | n  | f Hz  | M    | eta_air (mit d+0,2) | eta_air (ohne) | lambda   | lambda*1e-3 | T60 Luft s (mit) | T60 (ohne)")
for t in (29, 48, 55, 60, 72, 89):
    s = saite8(t)
    for n in (1, 6, 20):
        f = n * s["f"] * math.sqrt(1 + s["B"] * n * n)
        if f > 12000:
            continue
        M = s["d"] / 4 * math.sqrt(2 * math.pi * f / 1.5e-5)
        ea, eo, la = eta_air(s, f), eta_air(s, f, False), lam(s, n)
        print(f"{NAME[t]:5s} | {n:2d} | {f:5.0f} | {M:4.2f} | {ea:19.2e} | {eo:14.2e} | {la:.2e} | {la*1e-3:11.1e} | {6.91/(math.pi*f*ea):16.1f} | {6.91/(math.pi*f*eo):.1f}")
print("\nVaelimaeki 2004 Abb. 8 (Messing, f0 197 Hz, T60 abgelesen) gegen Luft allein (Messing d 0,30 / 0,35 / 0,40 mm, 8500 kg/m3, ANNAHME):")
for n, t60 in ((1, 3.8), (6, 11.7), (20, 3.5)):
    f = 197 * n
    em = 6.91 / (math.pi * f * t60)
    txt = " | ".join(f"d {dd}: {eta_air({'d': dd/1000, 'rho': 8500}, f):.1e}" for dd in (0.30, 0.35, 0.40))
    print(f"  n {n:2d} ({f:4.0f} Hz): gemessen eta {em:.1e} | Luft mit (d+0,2): {txt}")
print("Obergrenze eta_E (Messing) aus n = 20 bei B = 3e-5 (Vaelimaeki Abb. 5 bei 200 Hz): eta_E <= 1,6e-4 / (400 * 3e-5) =", f"{1.6e-4/(400*3e-5):.3f}")
print("\nB je Taste (8') gegen Vaelimaeki Abb. 5 (Messing, anderes Instrument): ~1,5e-5 (70-100 Hz), 3e-5 (200), 1e-4 (400), 2e-4 (700), 4e-4 (1050)")
for t in (29, 36, 41, 48, 53, 55, 60, 65, 72, 77, 84, 89):
    s = saite8(t)
    print(f"  {NAME[t]:5s} f0 {s['f']:6.1f} Hz | d {1000*s['d']:.3f} | E {s['E']/1e9:.0f} GPa | T {s['T']:5.1f} N | B {s['B']:.2e} | "
          f"Teilton 10 um {1200*math.log2(math.sqrt(1+100*s['B'])):.1f} Cent hoch")

# ------------------------------------------------------------------ 5 Register
kopf("5  4'-Zupfpunkt: Pruefstand-Regel (Anteil des 8' gleicher Tonhoehe) gegen Fletcher & Beebe Abb. 8.4 (Ruckers-Nachbau)")
FB = {36: (1500, 0.10, 790, 0.09), 48: (1100, 0.12, 540, 0.11), 60: (665, 0.15, 320, 0.15), 72: (340, 0.25, 165, 0.28), 84: (175, 0.35, 90, 0.48)}
print("Taste | F&B 8' Anteil | F&B 4' Anteil | F&B 4' ab Sattel mm | F&B p4/p8 | Regel S200: Anteil 4' | Regel 4' ab Sattel mm (Taskin-Mensur)")
for t, (L8, a8, L4, a4) in FB.items():
    s4 = saite4(t)
    print(f"{NAME[t]:5s} | {a8:13.2f} | {a4:13.2f} | {L4*a4:19.0f} | {L4*a4/(L8*a8):9.2f} | {s4['x']/s4['L']:21.3f} | {1000*s4['x']:.0f}")

# ------------------------------------------------------------------ Nachtraege
kopf("1f Welche Kielsteife je Taste verlangt GZ13 a (-2,8 dB/Okt) bei festem Ueberstand? (Option C)")
print("Taste | F Ziel N (A an C4, -2,8 dB/Okt) | EI-Faktor gegen Chadefaux-Kiel | Dicke-Faktor (EI ~ H^3) | y_s mm")
FC4 = kc.abloesen(saite8(60)["K"], U_C)["F"]
for t in TASTEN + [67]:
    s = saite8(t)
    ziel = FC4 * 10 ** (-2.8 / 20 * (t - 60) / 12)
    lo, hi = 0.01, 100.0
    for _ in range(80):
        m = math.sqrt(lo * hi)
        k = Kiel(1.5e9 * m, 0.011e-12, 3.5e-3, L1=2.9e-3, n=400)
        lo, hi = (m, hi) if k.abloesen(s["K"], U_C)["F"] < ziel else (lo, m)
    r = Kiel(1.5e9 * m, 0.011e-12, 3.5e-3, L1=2.9e-3, n=400).abloesen(s["K"], U_C)
    print(f"{NAME[t]:5s} | {ziel:31.2f} | {m:30.2f} | {m**(1/3):23.2f} | {1000*r['ys']:.2f}")

kopf("1g LumiViz heute (GZ13 a) gegen die Messung an G4 (Chadefaux 2013): Auslenkung beim Abloesen")
s = saite8(67)
gz = 2.0 * 10 ** (-2.8 / 20 * (67 - 60) / 12)
print(f"GZ13 a an G4: F {gz:.2f} N, K {s['K']:.0f} N/m (Taskin, 440 Hz) -> y = F/K = {1000*gz/s['K']:.2f} mm; gemessen D = 0,76 (langsam) / 0,94 (schnell) mm"
      f" -> {20*math.log10(gz/s['K']/0.76e-3):+.1f} / {20*math.log10(gz/s['K']/0.94e-3):+.1f} dB")
print(f"Mit K der Quelle (T 85,7 N, L 475 mm, beta {beta:.3f}): y = {1000*gz/Kc:.2f} mm")

kopf("3b Lautenzug geschlossen: sigma_n ~ 2 Z0 f0 R / (R^2 + (T / (a w_n))^2), Uebergang w_c = T / (a R)")
for t in (36, 60, 84):
    s = saite8(t)
    for a, R in ((0.5e-3, 40.0), (2.5e-3, 40.0)):
        wc = s["T"] / (a * R)
        werte = []
        for n in (1, 5, 20):
            w = 2 * math.pi * n * s["f"]
            werte.append((2 * s["Z0"] * s["f"] * R / (R * R + (s["T"] / (a * w)) ** 2), filz(s, a, R, 0, n)[0]))
        print(f"{NAME[t]:4s} a {1000*a:.1f} mm R {R:.0f}: f_c = {wc/2/math.pi:6.0f} Hz (Teilton {wc/2/math.pi/s['f']:5.1f}); sigma_inf = 2 Z0 f0 / R = {2*s['Z0']*s['f']/R:.2f} 1/s; "
              + "; ".join(f"n={n}: geschl. {g:.2f} / exakt {e:.2f}" for n, (g, e) in zip((1, 5, 20), werte)))

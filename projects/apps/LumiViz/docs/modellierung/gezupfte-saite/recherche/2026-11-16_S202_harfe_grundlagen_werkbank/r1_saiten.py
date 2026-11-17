"""R1: Saitentabelle (Bell 1988 Tab. 2.1/2.2, Abb. 2.10), Zug, Impedanzen, Federn an der Oese (Gl. S1-S6)."""
import math

# Bell Tab. 2.1 / 2.2 (am Bild): Saite, f [Hz], Durchmesser [mm], Laenge [m]; Werkstoff
TAB = [("00", 2960, 0.52, 0.071, "N"), ("0", 2637, 0.52, 0.082, "N"), ("1", 2489, 0.56, 0.088, "N"),
       ("2", 2218, 0.56, 0.095, "N"), ("3", 1976, 0.61, 0.105, "N"), ("4", 1864, 0.62, 0.113, "N"),
       ("5", 1661, 0.65, 0.123, "N"), ("6", 1480, 0.69, 0.132, "N"), ("7", 1318, 0.70, 0.143, "N"),
       ("8", 1244, 0.69, 0.154, "G"), ("9", 1109, 0.70, 0.164, "G"), ("10", 988, 0.73, 0.177, "G"),
       ("11", 932, 0.76, 0.192, "G"), ("12", 830, 0.80, 0.208, "G"), ("13", 740, 0.85, 0.225, "G"),
       ("14", 659, 0.90, 0.242, "G"), ("15", 622, 0.95, 0.265, "G"), ("16", 554, 1.00, 0.285, "G"),
       ("17", 494, 1.06, 0.327, "G"), ("18", 466, 1.12, 0.354, "G"), ("19", 415, 1.18, 0.385, "G"),
       ("20", 370, 1.24, 0.421, "G"), ("21", 330, 1.30, 0.455, "G"), ("22", 311, 1.36, 0.498, "G"),
       ("23", 277, 1.44, 0.542, "G"), ("24", 247, 1.52, 0.591, "G"), ("25", 233, 1.60, 0.644, "G"),
       ("26", 207, 1.68, 0.699, "G"), ("27", 185, 1.76, 0.753, "G"), ("28", 165, 1.84, 0.813, "G"),
       ("29", 155, 1.92, 0.880, "G"), ("30", 139, 2.00, 0.945, "G"), ("31", 123, 2.10, 0.999, "G"),
       ("32", 116, 2.20, 1.053, "G"), ("33", 104, 2.35, 1.103, "G")]
# Tab. 2.2 Metall: Saite, f, Kern, Umspinnung, gesamt [mm], Laenge
MET = [("34", 92, 0.80, 0.24, 1.45, 1.150), ("35", 82, 0.80, 0.32, 1.55, 1.180), ("36", 77, 0.80, 0.35, 1.65, 1.222),
       ("37", 69, 0.85, 0.40, 1.75, 1.257), ("38", 61, 0.90, 0.43, 1.92, 1.299), ("39", 58, 0.95, 0.45, 2.05, 1.330),
       ("40", 52, 1.00, 0.50, 2.15, 1.359), ("41", 46, 1.10, 0.55, 2.30, 1.397), ("42", 41, 1.10, 0.60, 2.45, 1.421),
       ("43", 39, 1.25, 0.65, 2.60, 1.445), ("44", 35, 1.30, 0.68, 2.75, 1.475)]
# Bell Abb. 2.10, am Bild abgelesen (+-8 N); 43 fehlt im Bild (interpoliert)
ZUG_ABB = {"00": 53, "0": 55, "1": 55, "2": 55, "3": 69, "4": 59, "5": 68, "6": 78, "7": 63, "8": 89, "9": 89,
           "10": 90, "11": 103, "12": 89, "13": 105, "14": 113, "15": 117, "16": 105, "17": 160, "18": 148,
           "19": 151, "20": 165, "21": 163, "22": 177, "23": 184, "24": 200, "25": 237, "26": 239, "27": 227,
           "28": 251, "29": 260, "30": 289, "31": 280, "32": 322, "33": 279, "34": 433, "35": 446, "36": 463,
           "37": 447, "38": 410, "39": 438, "40": 397, "41": 424, "42": 384, "43": 380, "44": 377}
RHO = {"N": 1140.0, "G": 1320.0}     # Lynch-Aird & Woodhouse 2017 Tab. (1067-1160), 2018 Tab. 1 (1320-1325)
E = {"N": 6e9, "G": 6e9}              # 2018 S. 7: Darm "about 5.5-6.5 GPa"; Nylon 2017: 4-10 GPa (Annahme 6)
THETA = math.radians(30.0)            # Bell S. 157: "meet the soundboard at an angle of 30 deg"
S2, C2 = math.sin(THETA) ** 2, math.cos(THETA) ** 2

zeilen = []
for name, f, d, L, w in TAB:
    A = math.pi * (d * 1e-3) ** 2 / 4
    mu = RHO[w] * A
    zeilen.append((name, f, L, mu, E[w] * A, w))
for name, f, dk, du, dg, L in MET:
    Ak = math.pi * (dk * 1e-3) ** 2 / 4
    Ag = math.pi * (dg * 1e-3) ** 2 / 4
    # Annahme: Stahlkern 7850 kg/m3, Kupferumspinnung 8900 kg/m3 mit Fuellgrad pi/4 (Runddraht auf Kern)
    mu = 7850 * Ak + 8900 * (math.pi / 4) * (Ag - Ak)
    zeilen.append((name, f, L, mu, 200e9 * Ak, "M"))

print("Saite  f    L     mu[g/m] T_rech T_Abb  Z0[Ns/m] EA/L[kN/m] T/L[N/m] f_l1[Hz]")
summe_r = summe_a = sumZ0 = sumk = 0.0
daten = {}
for name, f, L, mu, EA, w in zeilen:
    T = 4 * mu * L * L * f * f
    Ta = ZUG_ABB[name]
    Z0 = math.sqrt(Ta * mu)
    kl = EA / L
    fl1 = math.sqrt(EA / mu) / (2 * L)
    summe_r += T; summe_a += Ta; sumZ0 += Z0; sumk += kl * S2
    daten[name] = dict(f=f, L=L, mu=mu, T=Ta, Z0=Z0, kl=kl, EA=EA, fl1=fl1)
    print(f"{name:>4} {f:5d} {L:5.3f} {mu*1e3:6.2f} {T:6.0f} {Ta:5d} {Z0:7.2f} {kl/1e3:8.1f} {Ta/L:7.0f} {fl1:7.0f}")
print(f"Summe Zug gerechnet {summe_r:.0f} N, aus Abb. 2.10 {summe_a:.0f} N (Bell S. 38: 9350 N)")
print(f"Summe Z0 = {sumZ0:.1f} Ns/m; Summe EA/L sin^2(30) = {sumk/1e6:.2f} MN/m; Normallast sum T sin30 = {summe_a*0.5:.0f} N")

# S5: Zusatzimpedanz an der Oese in Normalrichtung, Saite am Hals starr: Z_n = sin^2 Z_l + cos^2 Z_t
def z_t(s, f, eta=0.01):
    """Querimpedanz am Ende einer am anderen Ende festen Saite, Z = -j Z0 cot(kL), Verlust ueber k komplex."""
    import cmath
    k = 2 * math.pi * f / math.sqrt(s["T"] / s["mu"]) * complex(1, -eta / 2)
    return -1j * s["Z0"] / cmath.tan(k * s["L"])

def z_l(s, f, eta=0.02):
    import cmath
    c = math.sqrt(s["EA"] / s["mu"])
    k = 2 * math.pi * f / c * complex(1, -eta / 2)
    return -1j * math.sqrt(s["EA"] * s["mu"]) / cmath.tan(k * s["L"])

print("\nZusatzimpedanz an der eigenen Oese bei der Frequenz der Saite (Hals starr):")
print("Saite f    |Z_l| sin2 [Ns/m]  Z_t gedaempft (Z0) cos2 | Z_t ungedaempft eta=0.01 cos2 | Feder EA/L sin2 als |Z|=k/w")
for n in ("33", "26", "21", "12", "10"):
    s = daten[n]; f = s["f"]
    zl = z_l(s, f) * S2
    zt_d = s["Z0"] * C2
    zt_u = z_t(s, f, 0.01) * C2
    print(f"{n:>4} {f:5d} {abs(zl):10.1f}  {zt_d:8.2f}  {abs(zt_u):10.1f}  {s['kl']*S2/(2*math.pi*f):8.1f}")

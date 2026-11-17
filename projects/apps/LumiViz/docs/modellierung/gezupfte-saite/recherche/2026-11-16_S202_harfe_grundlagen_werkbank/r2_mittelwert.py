"""R2: Mittelwert der Punkt-Admittanz an den Oesen (unendliche Platte, unendlicher Balken) gegen Bell 7.22,
Modell (S201) und Le Carrou; Gl. P1-P4. Werte des Bauplans aus make_harfe.py (nur gelesen)."""
import math

L = 1.31
def lin(y, a, b): return a + (b - a) * y / L
ZUG = [(0.0, 0.020, 0.032), (0.29, 0.025, 0.036), (0.55, 0.019, 0.039), (0.80, 0.013, 0.034), (1.05, 0.0072, 0.027),
       (1.28, 0.0056, 0.020)]
def zug(y):
    for (y0, h0, b0), (y1, h1, b1) in zip(ZUG, ZUG[1:]):
        if y <= y1:
            t = (y - y0) / (y1 - y0); return h0 + t * (h1 - h0), b0 + t * (b1 - b0)
    return ZUG[-1][1], ZUG[-1][2]
RHO_F, EX, EY, NU = 500.0, 7.3e9, 0.855e9, 0.3     # Fichte geeicht (Bauplan)
RHO_B, EB = 750.0, 13.7e9                           # Buche, Faser laengs der Leiste (Bauplan, Chabassier)

def dB(Y): return 20 * math.log10(Y) + 116.0      # Bell S. 133: 116 dB = 1 s/kg

def platte(y):
    h = lin(y, 0.010, 0.003) + 0.001
    m2 = RHO_F * h
    Dx, Dy = EX * h**3 / 12 / (1 - NU**2), EY * h**3 / 12 / (1 - NU**2)
    return h, m2, math.sqrt(Dx * Dy)

def balken(y, flansch=0.0):
    """Deckleiste aussen + Platte (Flanschbreite) + Zugleiste innen; EI um die eigene Nullinie, m'."""
    h, m2, _ = platte(y)
    bc, hc = lin(y, 0.033, 0.018), lin(y, 0.005, 0.003)
    hs, bs = zug(y)
    bf = max(flansch, bc, bs)
    teile = [(EB, bs, hs, -hs / 2), (EY, bf, h, h / 2), (EB, bc, hc, h + hc / 2)]
    EA = sum(E * b * t for E, b, t, z in teile)
    zn = sum(E * b * t * z for E, b, t, z in teile) / EA
    EI = sum(E * (b * t**3 / 12 + b * t * (z - zn) ** 2) for E, b, t, z in teile)
    mp = RHO_B * (bc * hc + bs * hs) + RHO_F * bf * h
    return EI, mp

def Y_platte(y):                                     # P1: 1/(8 sqrt(D m''))  (Cremer/Heckl, orthotrop: D = sqrt(Dx Dy))
    _, m2, D = platte(y); return 1 / (8 * math.sqrt(D * m2))

def Z_balken(y, f, flansch=0.0):                     # P2: Z = 2 (1+j) m' c_B, c_B = (EI/m')^(1/4) sqrt(w)
    EI, mp = balken(y, flansch); w = 2 * math.pi * f
    cB = (EI / mp) ** 0.25 * math.sqrt(w)
    return 2 * (1 + 1j) * mp * cB

ORTE = {"33": 0.393, "32": 0.420, "31": 0.450, "30": 0.476, "29": 0.506, "28": 0.534, "27": 0.559, "26": 0.581,
        "25": 0.609, "24": 0.634, "23": 0.666, "22": 0.692, "21": 0.723, "20": 0.745, "19": 0.773, "18": 0.798,
        "17": 0.823, "16": 0.850, "15": 0.877, "14": 0.905, "13": 0.929, "12": 0.951, "11": 0.976, "10": 0.999}
F = {"33": 104, "32": 116, "31": 123, "30": 139, "29": 155, "28": 165, "27": 185, "26": 207, "25": 233, "24": 247,
     "23": 277, "22": 311, "21": 330, "20": 370, "19": 415, "18": 466, "17": 494, "16": 554, "15": 622, "14": 659,
     "13": 740, "12": 830, "11": 932, "10": 988}
BELL = dict(zip([str(s) for s in range(33, 9, -1)],
                [80, 89, 74, 79, 81, 82, 85, 86, 84, 87, 88, 89, 95, 85, 89, 89, 91, 90, 93, 93, 99, 103, 98, 99]))
MODELL = {"33": 69.7, "32": 74.8, "31": 80.2, "30": 74.0, "29": 56.8, "28": 58.9, "27": 72.7, "26": 88.3, "25": 74.1,
          "24": 70.5, "23": 64.6, "22": 51.3, "21": 68.2, "20": 72.0, "19": 59.5, "18": 60.6, "17": 57.1, "16": 62.2,
          "15": 72.5, "14": 74.4, "13": 67.0, "12": 68.7, "11": 70.2, "10": 66.8}

print("Saite  f   y     Platte  Balken  Balken+Platte | Bell 7.22  Modell(S201) | Bell-B+P  Modell-B+P")
d_b, d_m = [], []
for s in ORTE:
    y, f = ORTE[s], F[s]
    Yp = Y_platte(y)
    Zb = Z_balken(y, f)
    Ybp = 1 / (Zb + 1 / Yp)
    d_b.append(BELL[s] - dB(abs(Ybp))); d_m.append(MODELL[s] - dB(abs(Ybp)))
    print(f"{s:>4} {f:4d} {y:5.3f} {dB(Yp):6.1f} {dB(abs(1/Zb)):6.1f} {dB(abs(Ybp)):8.1f}      | {BELL[s]:5.1f} {MODELL[s]:8.1f}     | "
          f"{d_b[-1]:+6.1f} {d_m[-1]:+8.1f}")
med = lambda v: sorted(v)[len(v) // 2]
print(f"Median: Bell - (Balken+Platte) {med(d_b):+.1f} dB, Modell - (Balken+Platte) {med(d_m):+.1f} dB")
for y in (0.393, 0.723, 0.951):
    EI, mp = balken(y); h, m2, D = platte(y)
    print(f"y={y}: EI={EI:.1f} N m2, m'={mp:.3f} kg/m, h={h*1e3:.1f} mm, m''={m2:.2f} kg/m2, D={D:.2f} N m")
# Le Carrou Punkt 34 (Camac), 104/117 Hz: 0,6/1,0e-3 s/kg
print(f"Le Carrou P34: {dB(0.6e-3):.1f} / {dB(1.0e-3):.1f} dB")

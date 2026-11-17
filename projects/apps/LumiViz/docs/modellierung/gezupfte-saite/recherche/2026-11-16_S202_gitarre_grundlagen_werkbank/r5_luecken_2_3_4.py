"""Gegenrechnungen zu den Lücken 2, 3, 4 (Zahlen der Quellen, siehe Bericht)."""
import math

import numpy as np

print("=== Lücke 2: was die Bank-Ablesungen über die Richtungen je Mode sagen ===")
# Stufe-1-Bericht, Bank 2010 Abb. 4 abgelesen (+-3 dB), LumiViz-Achsen: Band, Yyy-Yzz dB, Yzy-Yzz dB
for band, dyy, dzy in [("~100 Hz", -15, -20), ("Hauptspitze ~240 Hz", -30, -20), ("1-3 kHz", -13.5, -17.5), ("3-10 kHz", -13.5, -13.5)]:
    t2 = 10 ** (dzy / 10)          # <tan^2>  wenn Yzy = Summe tan * (Zz-Term), Vorzeichen zufällig: |Yzy|^2/|Yzz|^2 ~ <tan^2>
    t4 = 10 ** (dyy / 10)          # <tan^4>  ebenso |Yyy|^2/|Yzz|^2 ~ <tan^4>
    print(f"  {band:20s}: <tan^2>^0.5 = {math.sqrt(t2):.3f}, <tan^4>^0.25 = {t4**0.25:.3f}, Kurtosis <tan^4>/<tan^2>^2 = {t4/t2**2:6.1f}"
          f"  (eine Mode allein: Yyy/Yzz = (Yzy/Yzz)^2 -> {2*dzy:+.0f} dB statt {dyy:+.0f} dB)")
f = np.array([100, 240, 500, 1000, 1500, 2000, 4000])
b = np.clip(0.03 * (f / 200.0) ** 1.27, 0.03, 0.4)
print("  Prüfstand-Annahme |tan| = clip(0,03 (f/200)^1,27; 0,03; 0,4):",
      ", ".join(f"{x} Hz {y:.3f} (Yzy {20*math.log10(y):+.0f} dB, Yyy {40*math.log10(y):+.0f} dB)" for x, y in zip(f, b)))

print()
print("=== Lücke 3: Zupf ===")
T = {"1 E4": (70.3, 0.38e-3), "6 E2": (71.6, 6.24e-3)}
L = 0.650
for name, (Tn, m) in T.items():
    c = math.sqrt(Tn / m)
    for h in (0.8e-3, 2e-3, 4e-3):
        F0 = Tn * h * L / (0.16 * (L - 0.16))
        print(f"  Saite {name}: c = {c:6.1f} m/s, h = {h*1e3:.1f} mm bei x0 = 16 cm -> F0 = {F0:.2f} N")
    print(f"     Breite w = 2 cm: c/w = {c/0.02/1000:.1f} kHz (Scherrer Tab. 3.1);  Gauss h = 7,5 mm (Woodhouse 2012): "
          f"-3 dB bei {math.sqrt(math.log(10**0.15)) * c / 0.0075 / (2*math.pi) / 1000:.2f} kHz, -20 dB bei "
          f"{math.sqrt(math.log(10)) * c / 0.0075 / (2*math.pi) / 1000:.2f} kHz")
x0 = 0.325
print(f"  Scherrer Abb. 3.8 Mitte (x0/L = 0,5, T = 70 N): mit h = 1 cm {70*0.01*L/(x0*(L-x0)):.2f} N, mit h = 1 mm {70*0.001*L/(x0*(L-x0)):.2f} N"
      f"; bei x0 = 1 cm: {70*0.01*L/(0.01*(L-0.01)):.1f} N / {70*0.001*L/(0.01*(L-0.01)):.2f} N (Abbildung: ~0,43 N und ~7 N)")
print("  sin(pi f t)/(pi f t) mit t_ex = 0,02 s: erste Nullstelle", 1 / 0.02, "Hz, -3 dB bei", round(0.443 / 0.02, 1), "Hz")
print("  Zupfort je Bund (Mensur 650 mm): Bund | Saitenlänge | (A) fest 16 cm: R | (B) R fest = 16/65: x_p cm | min(16, L/2)")
for n in (0, 2, 5, 7, 9, 12, 15, 17, 19):
    Ln = L * 2 ** (-n / 12)
    print(f"     {n:2d} | {Ln*100:5.1f} cm | R = {0.16/Ln:.3f} | x_p = {0.16/L*Ln*100:5.1f} cm | {min(0.16, Ln/2)*100:5.1f} cm")

print()
print("=== Lücke 4: Modendichte, Koinzidenz, Dämpfung ===")
c0 = 343.0
def platte(name, rhoh, D, A):
    beta = math.sqrt(rhoh / D)
    nf = A * beta / 2
    fc = c0 * c0 / (2 * math.pi) * beta
    G = 1 / (8 * math.sqrt(rhoh * D))
    print(f"  {name:48s}: beta = {beta:.3f} s/m^2, n(f) = {nf:.4f} /Hz (1 Mode je {1/nf:5.1f} Hz), "
          f"f_c = {fc/1000:5.1f} kHz, G_inf = {20*math.log10(G):6.1f} dB re 1 m/s/N")
platte("Elie Bezugsplatte Fichte (420 kg/m3, 2 mm, 2,1 Nm), S = 0,09", 420 * 0.002, 2.1, 0.09)
platte("dieselbe auf der Decke von g34 (A ~ 0,143 m^2)", 420 * 0.002, 2.1, 0.143)
for M, D in ((0.9, 60), (1.4, 75), (2.0, 95)):
    platte(f"Elie Abb. 9 Ersatzplatte M = {M} kg, D = {D} Nm, S = 0,09", M / 0.09, D, 0.09)
# orthotrope Platte: n(w) = A sqrt(rho h) / (8 pi^2) Int_0^2pi D(theta)^-1/2 dtheta
th = np.linspace(0, 2 * np.pi, 20001)
for verh in (1, 4, 10, 16):
    D1, D2 = 2.1 * math.sqrt(verh), 2.1 / math.sqrt(verh)      # gleiches geometrisches Mittel wie die Bezugsplatte
    D3 = math.sqrt(D1 * D2)
    Dth = D1 * np.cos(th) ** 4 + 2 * D3 * np.cos(th) ** 2 * np.sin(th) ** 2 + D2 * np.sin(th) ** 4
    faktor = np.trapezoid(Dth ** -0.5, th) / (2 * np.pi) * math.sqrt(D3)
    print(f"  orthotrop D1/D2 = {verh:2d} (D3 = sqrt(D1 D2)): Modendichte / isotrop(sqrt(D1 D2)) = {faktor:.4f}")
print("  g34 (LumiViz): Polzahl 2,5-5 kHz 26 -> 0,0104 /Hz; Daempfung alpha = 0,0105 f + 147 1/s (Energie):")
for fq in (1000, 2500, 5000, 10000, 20000):
    a = 0.0105 * fq + 147
    print(f"     {fq:5d} Hz: alpha = {a:5.0f} 1/s, eta = alpha/w = {a/(2*math.pi*fq):.4f}, T60 = {6.91*2/a:.3f} s, "
          f"Halbwertsbreite = {a/(2*math.pi):.0f} Hz, Modenueberlapp (n = 0,0104) = {a/(2*math.pi)*0.0104:.2f}, (n = 0,019) = {a/(2*math.pi)*0.019:.2f}")

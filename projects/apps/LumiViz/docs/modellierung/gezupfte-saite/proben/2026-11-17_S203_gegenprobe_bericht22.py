"""Gegenprobe S203 zu Bericht 22: Reihenfeder (1.4), Steigungen ueber die Moden (1.3), Gueten (2). Eigene Rechnung
aus den Tabellenwerten des Berichts (die Ablesung selbst wird am Bild stichprobenweise geprueft)."""
import numpy as np

Y0 = 1.58e-6           # 0 dB = 1,58e-6 s/kg (Bell S. 133); 116 dB = 1 s/kg (S. 149)
print(f"Bezug: 20 lg(1/1,58e-6) = {20*np.log10(1/Y0):.2f} dB")


def db(y):
    return 20 * np.log10(y / Y0)


# 1.4 Reihenfeder: Linie omega/k, k fuer den kleinsten Wert
moden = [("S(1,1)-", 187, 68.2), ("S(1,1)+", 207, 76.8), ("S(1,2)", 257, 72.5), ("S(1,2)'", 284, 75.1),
         ("S(1,3)", 334, 80.7), ("S(1,4)", 430, 80.7), ("S(1,5)", 555, 81.7), ("S(1,6)", 697, 85.3),
         ("S(1,7)", 753, 85.8), ("S(1,8)", 895, 86.9)]
print("\n1.4 Reihenfeder")
kmin = []
for n, f, m in moden:
    w = 2 * np.pi * f
    lin = db(w / 5.2e4)
    k = w / (Y0 * 10 ** (m / 20))
    kmin.append(k)
    print(f"  {n:8s} {f:4d} Hz: Linie 5,2e4 {lin:5.1f} dB, Abstand {m-lin:+5.1f}, k >= {k:.2e}")
print(f"  weichste zulaessige Feder: {min(kmin):.2e} N/m")
for f, bell in [(104, 80), (207, 86), (330, 95), (494, 91), (740, 99), (988, 99)]:
    print(f"  Beitrag k=1,2e5 bei {f} Hz: {db(2*np.pi*f/1.2e5):5.1f} dB (Bell 7.22 {bell})")

# 1.3 Steigung der Extremwerte ueber log2 f
ohne = [(170, 68.0, 86.3), (216, 66.4, 81.6), (249, 65.7, 96.0), (316, 75.7, 104.5), (432, 73.5, 105.3),
        (545, 73.2, 101.5), (709, 75.3, 100.7), (757, 79.3, 99.7), (905, 81.0, 102.2)]
mit = [(f, mi, ma) for (_, f, _), (mi, ma) in zip(moden, [(68.2, 79.3), (76.8, 86.5), (72.5, 92.3), (75.1, 91.5),
                                                         (80.7, 101.0), (80.7, 99.7), (81.7, 102.7), (85.3, 103.0),
                                                         (85.8, 100.8), (86.9, 103.1)])]
print("\n1.3 Steigung (dB/Oktave) Minima / Maxima")
for name, t in (("ohne Saiten", ohne), ("mit Saiten", mit)):
    x = np.log2([r[0] for r in t])
    print(f"  {name:12s}: {np.polyfit(x,[r[1] for r in t],1)[0]:+.1f} / {np.polyfit(x,[r[2] for r in t],1)[0]:+.1f}")

# 2 Gueten
print("\n2 Gueten")
for n, s in (("T1", complex(-25.3, 968.4)), ("A0", complex(-27.4, 1069.2))):
    print(f"  Le Carrou Tab. 4.1 {n}: f {s.imag/2/np.pi:.1f} Hz, Q |s|/(2|Re s|) = {abs(s)/(2*abs(s.real)):.1f}")
for z in (5.5, 4.8, 2.5, 2.3, 0.9, 1.4):
    print(f"  zeta {z} % -> Q {1/(2*z/100):.1f}")

# 1.2 Abb. 7.6 Regression aus der Tabelle des Berichts (nur die 30-Hz-Stuetzstellen, nicht die volle Spalte)
f = np.array([140, 170, 200, 230, 260, 290, 320, 350, 380, 410, 440, 470, 500, 530, 560, 590, 620, 650, 680, 710, 740, 770, 790])
mit76 = np.array([76.3, 79.1, 81.6, 84.9, 86.1, 77.6, 86.9, 78.0, 81.2, 83.3, 81.0, 84.0, 85.2, 88.4, 82.8, 84.5, 86.2, 87.0, 89.1, 88.1, 89.1, 89.7, 90.3])
ohne76 = np.array([78.6, 69.8, 78.6, 84.7, 88.5, 67.6, 83.5, 78.3, 64.9, 69.2, 80.2, 71.4, 73.6, 78.3, 85.1, 77.7, 68.9, 73.3, 74.6, 77.6, 77.4, 76.9, 77.4])
m = f >= 200
print("\n1.2 Abb. 7.6, Stuetzstellen der Tabelle, 200-790 Hz: mit {:+.1f}, ohne {:+.1f} dB/Okt".format(
    np.polyfit(np.log2(f[m]), mit76[m], 1)[0], np.polyfit(np.log2(f[m]), ohne76[m], 1)[0]))
e = lambda v: 10 * np.log10(np.mean(10 ** (v / 10)))
b = f >= 400
print(f"  Energiemittel 400-790 Hz (Stuetzstellen): mit {e(mit76[b]):.1f}, ohne {e(ohne76[b]):.1f}, Differenz {e(mit76[b])-e(ohne76[b]):+.1f} dB")

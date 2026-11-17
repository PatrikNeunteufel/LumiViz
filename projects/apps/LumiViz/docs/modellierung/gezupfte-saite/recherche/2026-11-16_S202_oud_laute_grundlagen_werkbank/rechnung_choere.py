"""Choere: zwei Saiten an einem Stegpunkt (Auftrag 2026-11-16_15). Schreibt nichts.

A  Woodhouse 2021 Gl. 6-10 (drei Freiheitsgrade: Koerpermode + zwei Saiten), gegen Abb. 8 und 9 derselben Arbeit.
B  Zwei Saiten an einer komplexen Stegadmittanz Y (hergeleitet, Weinreich-Form): Eigenwerte, Abklingraten, Schwebung.
C  Oud-Choere an der Stegadmittanz des Koerpers `oud.lvinstbody` (LumiViz, nur gelesen), Laute an `lute.lvinstbody`.
D  Basschor einzeln gegen doppelt (nur Grundton): Pegel am Anfang, Lautheit im 400-ms-Fenster, Pegel 1-2 s.
Aufruf: python -B -I <dieses Skript>
"""
import importlib.util
import math
import sys

import numpy as np

sys.dont_write_bytecode = True
PFAD = (r"C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung"
        r"\gezupfte-saite\proben\2026-11-15_S199_pruefstand_zupf.py")
spec = importlib.util.spec_from_file_location("pruefstand", PFAD)
P = importlib.util.module_from_spec(spec)
spec.loader.exec_module(P)


# ------------------------------------------------------------------ A: Woodhouse 2021 Gl. 7-10
def drei_fg(L, T1, T2, m1, m2, Q1, Q2, mb, kb, cb):
    K = np.diag([kb, math.pi ** 2 * T1 / (2 * L), math.pi ** 2 * T2 / (2 * L)])             # Gl. 7
    M = np.array([[mb + L * (m1 + m2) / 3, m1 * L / math.pi, m2 * L / math.pi],               # Gl. 8
                  [m1 * L / math.pi, m1 * L / 2, 0.0],
                  [m2 * L / math.pi, 0.0, m2 * L / 2]])
    c1 = math.pi * math.sqrt(T1 * m1) / (2 * Q1)                                              # Gl. 10
    c2 = math.pi * math.sqrt(T2 * m2) / (2 * Q2)
    C = np.diag([cb, c1, c2])                                                                 # Gl. 9
    Mi = np.linalg.inv(M)
    A = np.block([[np.zeros((3, 3)), np.eye(3)], [-Mi @ K, -Mi @ C]])
    lam, vec = np.linalg.eig(A)
    moden = []
    for l, v in zip(lam, vec.T):
        if l.imag <= 0:
            continue
        q = v[:3]
        f = l.imag / (2 * math.pi)
        Q = l.imag / (2 * -l.real)
        art = "Koerper" if abs(q[0]) * 3 > max(abs(q[1]), abs(q[2])) and abs(q[1]) < 1e-12 + abs(q[0]) * 30 and False else ""
        moden.append((f, Q, q))
    moden.sort(key=lambda x: x[0])
    return moden


def saiten_moden(moden):
    """Die zwei Moden mit dem kleinsten Koerperanteil; Rueckgabe (f, Q, gleichphasig?)."""
    bewertet = sorted(moden, key=lambda m: abs(m[2][0]) / (abs(m[2][1]) + abs(m[2][2])))[:2]
    aus = []
    for f, Q, q in sorted(bewertet, key=lambda m: m[0]):
        gleich = (q[1] * np.conj(q[2])).real > 0
        aus.append((f, Q, gleich))
    return aus


print("=== A  Woodhouse 2021 Gl. 7-10, Fallstudie S. 4381: Stahl 0,45 mm, L 0,65 m, B3 246,9 Hz, Q der Saite 3000")
L, d = 0.65, 0.45e-3
m = 7850 * math.pi * d * d / 4
T0 = 4 * L * L * 246.9 ** 2 * m
print(f"Saite: m {m * 1e3:.3f} g/m, T {T0:.1f} N, Z0 {math.sqrt(T0 * m):.3f} N s/m  [Stahldichte 7850 = Annahme]")
fall = {"Abb. 9 (Daempfer c = 300 N s/m, m = k = 1e-3)": (1e-3, 1e-3, 300.0)}
for fb in (222.0, 247.0):   # Lage der Koerperspitze aus Abb. 7 abgelesen (durchgezogen ~222, gestrichelt ~247 Hz)
    mb = 0.2
    wb = 2 * math.pi * fb
    fall[f"Abb. 8 (Koerper 0,2 kg, Q 40, {fb:.0f} Hz)"] = (mb, mb * wb * wb, mb * wb / 40)
for name, (mb, kb, cb) in fall.items():
    print(name)
    for cent in (-8, -4, -2, -1, -0.5, 0, 0.5, 1, 2, 4, 8):
        T2 = T0 * 2 ** (2 * cent / 1200)
        sm = saiten_moden(drei_fg(L, T0, T2, m, m, 3000, 3000, mb, kb, cb))
        print(f"   {cent:+5.1f} cent: " + "   ".join(f"{f:7.2f} Hz Q {Q:6.0f} ({'gleich' if g else 'gegen'})" for f, Q, g in sm))


# ------------------------------------------------------------------ B: zwei Saiten an komplexem Y
def zwei_saiten(f1, f2, Z1, Z2, Y, eta1, eta2):
    """Komplexe Kreisfrequenzen (Zeitansatz e^{i w t}): w = w_n (1 + i Z0 Y / pi) je Saite (Grundton, n = 1);
    zwei Saiten am selben Punkt: Kopplung ueber dieselbe Stegbewegung. Eigenwerte der 2x2-Matrix."""
    w1, w2 = 2 * math.pi * f1, 2 * math.pi * f2
    s1 = w1 * (1 + 0.5j * eta1)
    s2 = w2 * (1 + 0.5j * eta2)
    e11, e22 = 1j * w1 * Z1 * Y / math.pi, 1j * w2 * Z2 * Y / math.pi
    e12 = 1j * math.sqrt(w1 * w2 * Z1 * Z2) * Y / math.pi
    A = np.array([[s1 + e11, e12], [e12, s2 + e22]])
    lam, vec = np.linalg.eig(A)
    return lam, vec


print()
print("=== B  Gegenprobe: dieselbe Fallstudie mit Y = 1/c (Daempfer 300 N s/m) in der 2x2-Form")
Z = math.sqrt(T0 * m)
for cent in (0, 0.5, 1, 2, 8):
    f2 = 246.9 * 2 ** (cent / 1200)
    lam, _ = zwei_saiten(246.9, f2, Z, Z * 2 ** (cent / 1200), 1 / 300, 1 / 3000, 1 / 3000)
    print(f"   {cent:+4.1f} cent: " + "   ".join(f"{l.real / 2 / math.pi:7.2f} Hz Q {l.real / (2 * l.imag):6.0f}" for l in sorted(lam, key=lambda x: x.real)))
eps = 2 * math.pi * 246.9 * Z / 300 / math.pi
print(f"   |eps| = w Z0 |Y| / pi = {eps:.3f} 1/s; Bereich der Anti-Veering-Verschmelzung |f2 - f1| < 2|eps|/2pi = "
      f"{2 * eps / 2 / math.pi:.3f} Hz = {1200 * math.log2(1 + 2 * eps / 2 / math.pi / 246.9):.2f} cent")


# ------------------------------------------------------------------ C: Oud und Laute
def y_am(kp, punkt, f):
    w = np.array([2 * math.pi * f + 0j])
    Y, _ = P.koerper_kurven(kp, punkt, w)
    return complex(Y[0])


def eta_saite(sa, f):
    k2 = (2 * math.pi * f) ** 2 * sa["m"] / sa["T"]
    return float(P.saiten_eta(sa, np.array([2 * math.pi * f + 0j]), np.array([k2]))[0].real)


def chor_tabelle(name, cent_gesamt):
    kp = P.lade_koerper({"oud": "oud", "laute": "lute"}[name])
    ch, mensur, cent = P.choere(name)
    print(f"--- {name}: Mensur {mensur} m, Verstimmung im Einklang {cent_gesamt} cent gesamt")
    print("Chor  Ton   f Hz   Z0 Ns/m  Re Y      Im Y      eta_b einzeln  eta Saite | einzeln: T60 s | doppelt, gleichphasig/gegen: T60 s | Schwebung Hz")
    erg = {}
    for c, (t, saiten) in enumerate(ch):
        sa = saiten[0]
        f = sa["hz"]
        Z0 = math.sqrt(sa["T"] * sa["m"])
        Y = y_am(kp, kp["tasten"].index(t), f)
        eta_s = eta_saite(sa, f)
        eta_b = 2 * Y.real * Z0 / math.pi                    # Woodhouse 2021 Gl. 1 (Y0 = 1/Z0)
        lam1, _ = zwei_saiten(f, f, Z0, Z0, Y, eta_s, eta_s)
        einz = 2 * math.pi * f * (1 + 0.5j * eta_s) * (1 + 1j * Z0 * Y / math.pi)
        T60_1 = 6.91 / einz.imag
        f2 = f * 2 ** (cent_gesamt / 1200)
        lam, vec = zwei_saiten(f, f2, Z0, Z0, Y, eta_s, eta_s)
        lam = sorted(lam, key=lambda x: -x.imag)
        T60 = [6.91 / l.imag for l in lam]
        schweb = abs(lam[0].real - lam[1].real) / 2 / math.pi
        erg[t] = (f, Z0, Y, eta_s, einz, lam, vec)
        print(f"{c + 1:4d}  {t:3d} {f:7.1f}  {Z0:6.3f}  {Y.real:.2e}  {Y.imag:+.2e}  {eta_b:.2e}      {eta_s:.2e} | {T60_1:6.2f}"
              f" | {T60[0]:6.2f} / {T60[1]:6.2f} | {schweb:.3f}")
    return kp, erg


print()
print("=== C  Choere an der Stegadmittanz der LumiViz-Koerper (Grundton, alle Moden der Datei und des statistischen Teils)")
kp_oud, erg_oud = chor_tabelle("oud", 3.0)
kp_lau, erg_lau = chor_tabelle("laute", 4.0)

# Verstimmung variiert am Oud-Chor A2 (Taste 45)
print()
print("Oud A2 (Taste 45): Abklingzeiten T60 (s) der beiden Moden und Schwebung gegen die Verstimmung")
f, Z0, Y, eta_s, _, _, _ = erg_oud[45]
for cent in (0, 0.5, 1, 1.5, 2, 3, 5, 8, 12):
    lam, _ = zwei_saiten(f, f * 2 ** (cent / 1200), Z0, Z0, Y, eta_s, eta_s)
    lam = sorted(lam, key=lambda x: -x.imag)
    print(f"   {cent:5.1f} cent: T60 {6.91 / lam[0].imag:6.2f} / {6.91 / lam[1].imag:6.2f} s, Frequenzabstand {abs(lam[0].real - lam[1].real) / 2 / math.pi:.3f} Hz")
eps = 2 * math.pi * f * Z0 * abs(Y) / math.pi
print(f"   |eps| = {eps:.3f} 1/s -> Kopplungsbreite 2|eps|/2pi = {2 * eps / 2 / math.pi:.3f} Hz = {1200 * math.log2(1 + 2 * eps / 2 / math.pi / f):.2f} cent")

# 3-FG-Modell mit der naechsten Koerpermode am Stegpunkt A2
print()
print("Oud A2 im 3-FG-Modell (Woodhouse 2021 Gl. 7-10) mit der Koerpermode, die am Stegpunkt am staerksten ist (80-200 Hz)")
punkt = kp_oud["tasten"].index(45)
phi = kp_oud["steg"][:, punkt]
sel = [i for i in range(len(kp_oud["f"])) if 80 < kp_oud["f"][i] < 200]
i = max(sel, key=lambda i: phi[i] ** 2)
mb = 1 / phi[i] ** 2
wb = 2 * math.pi * kp_oud["f"][i]
sa = P.choere("oud")[0][2][1][0]
print(f"   Mode {kp_oud['f'][i]:.1f} Hz, eta {kp_oud['eta'][i]:.3f}, wirksame Masse am Stegpunkt {mb * 1e3:.1f} g")
for cent in (0, 1.5, 3, 8):
    T2 = sa["T"] * 2 ** (2 * cent / 1200)
    sm = saiten_moden(drei_fg(0.60, sa["T"], T2, sa["m"], sa["m"], 1 / eta_s, 1 / eta_s, mb, mb * wb * wb, mb * wb * kp_oud["eta"][i]))
    print(f"   {cent:4.1f} cent: " + "   ".join(f"{f_:7.2f} Hz Q {Q:5.0f} T60 {6.91 * Q / (math.pi * f_):5.2f} s ({'gleich' if g else 'gegen'})" for f_, Q, g in sm))

# ------------------------------------------------------------------ D: Basschor einzeln gegen doppelt
print()
print("=== D  Oud C2 (Taste 36): einzeln gegen doppelt, nur der Grundton (Kraft am Steg ~ Summe der Saitenkraefte)")
f, Z0, Y, eta_s, einz, _, _ = erg_oud[36]
t = np.arange(0, 3.0, 1 / 4000)


def huelle(lams, vec, start):
    a = np.linalg.solve(vec, start)
    x = sum(a[k] * vec[:, k][:, None] * np.exp(1j * lams[k] * t)[None, :] for k in range(len(lams)))
    return np.abs(x.sum(axis=0))       # Summe der beiden Saitenkraefte (gleiche Impedanz)


def kennzahlen(h):
    p2 = h ** 2
    n = int(0.4 * 4000)
    c = np.concatenate([[0.0], np.cumsum(p2)])
    fen = (c[n:] - c[:-n]) / n
    an = 10 * np.log10(fen.max())
    spaet = 10 * np.log10(np.mean(p2[(t >= 1) & (t < 2)])) - 10 * np.log10(np.mean(p2[t < 0.4]))
    return an, spaet


h1 = np.abs(np.exp(1j * einz * t))
a1, s1 = kennzahlen(h1)
print(f"   einzeln: Abklingzeit T60 {6.91 / einz.imag:.2f} s; Pegel 1-2 s gegen 0-0,4 s {s1:.1f} dB")
for cent in (0.0, 1.0, 3.0, 6.0):
    lam, vec = zwei_saiten(f, f * 2 ** (cent / 1200), Z0, Z0, Y, eta_s, eta_s)
    h2 = huelle(lam, vec, np.array([1.0, 1.0]))
    a2, s2 = kennzahlen(h2)
    print(f"   doppelt {cent:3.1f} cent: Anfang +{20 * np.log10(h2[0] / h1[0]):.1f} dB, 400-ms-Fenster {a2 - a1:+.1f} dB gegen einzeln, "
          f"Pegel 1-2 s gegen 0-0,4 s {s2:.1f} dB, T60 der Moden {6.91 / lam[0].imag:.2f} / {6.91 / lam[1].imag:.2f} s")

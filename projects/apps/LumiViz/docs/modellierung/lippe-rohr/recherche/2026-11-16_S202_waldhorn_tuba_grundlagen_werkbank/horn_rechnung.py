"""Gegenrechnung der Eingangsimpedanz des Doppelhorns E. Schmid Nr. 777 (Wachter 2008) aus der digitalisierten
Mensur (Abb. 88) und dem Mundstueck H. Ebner (Abb. 52) — LumiViz_Research, Auftrag 2026-11-16_18.

Modelle (je Gleichung siehe Bericht, Abschnitt 2):
  Kern  : Treppe aus 1-mm-Zylindern (wie Rohr.cpp), Wandverluste Keefe 1. Ordnung (Gamma), Zc verlustfrei,
          Abstrahlung Zc[(ka)^2/4 + j 0,6133 ka] (wie Rohr.cpp)
  Kegel : Kegelstuempfe exakt in Kugelwellen (Basis e^(-+Gamma x)/x, Matrix numerisch aus der Basis), sonst wie Kern
  LS    : wie Kern, Abstrahlung Levine-Schwinger nach Silva et al. 2009 Tab. 1 (ohne Flansch)
  K3    : wie LS, Gamma und Zc nach Keefe 1984 bis 3. Ordnung (Wachter Gl. 3.12-3.20)
Aufruf: python horn_rechnung.py <ordner mit abb88_roh.txt>
"""
import sys
import os
import numpy as np

ORDNER = sys.argv[1]
RHO, C0, ETA, GAM, PR = 1.2041, 343.2, 1.8205e-5, 1.4, 0.71   # wie ic::Luft (Rohr.hpp:46-53)
F = np.arange(15.0, 1000.0, 0.25)
W = 2 * np.pi * F


def luft_bei(t_c):
    """Schallgeschwindigkeit ~ sqrt(T) (eigene Rechnung, ideales Gas); Bezug 20 Grad C = 343,2 m/s."""
    return C0 * np.sqrt((273.15 + t_c) / 293.15)


# ---------------------------------------------------------------- Geometrie
def mundstueck():
    """Wachter 2008 Abb. 52 (S. 55), Laenge/Durchmesser in mm, am Seitenbild gelesen."""
    l = [0, 5, 10, 15, 20, 25, 30, 30.5, 31.5, 35.9, 45.9, 50.9, 55.9, 60.9, 65.9, 67.38]
    d = [17.5, 14.77, 12.77, 10.3, 8.4, 6.23, 5.2, 4.75, 4.65, 4.7, 4.73, 4.73, 5.04, 5.5, 5.8, 6.3]
    return np.array(l) * 1e-3, np.array(d) * 0.5e-3


def mensur_abb88():
    """Digitalisierte Abb. 88: Spaltenspur bis 3,70 m (geglaettet), darueber Zeilenspur (steiler Becher)."""
    roh = np.loadtxt(os.path.join(ORDNER, "abb88_roh.txt"))
    x, r = roh[:, 0], roh[:, 1] * 1e-3
    bech = np.loadtxt(os.path.join(ORDNER, "abb88_roh_becher.txt"))
    xs = []
    rs = []
    # Mundrohr/Koerper: Median ueber 15 mm, dann alle 20 mm (im Mundrohr alle 10 mm)
    ab = 0.0700
    for p in np.concatenate([np.arange(ab, 0.40, 0.010), np.arange(0.40, 3.70, 0.020), [3.70]]):
        fen = np.abs(x - p) < 0.0075
        if fen.sum() >= 3:
            xs.append(p)
            rs.append(np.median(r[fen]))
    # Becher: Zeilenspur, x = Mittel aus oben/unten, nach x sortiert, r monoton
    bx = 0.5 * (bech[:, 0] + bech[:, 1])
    br = bech[:, 2]
    ordn = np.argsort(br)
    bx, br = bx[ordn], br[ordn]
    sel = br > rs[-1] + 2e-4
    letzt_x = xs[-1]
    for xb, rb in zip(bx[sel], br[sel]):
        if xb > letzt_x + 0.5e-3:
            xs.append(xb)
            rs.append(rb)
            letzt_x = xb
    return np.array(xs), np.array(rs)


def bohrung_F():
    lm, rm = mundstueck()
    xk, rk = mensur_abb88()
    x = np.concatenate([lm, xk])
    r = np.concatenate([rm, rk])
    return x, r


# ---------------------------------------------------------------- Akustik
def gamma_zc(r, w, c=C0, verl="k1"):
    """Wellenzahl Gamma und Wellenimpedanz Zc fuer Radius r (Skalar) und Kreisfrequenz w (Feld)."""
    k = w / c
    S = np.pi * r * r
    zc0 = RHO * c / S
    if verl == "keine":
        return 1j * k, zc0 * np.ones_like(w)
    rv = r * np.sqrt(w * RHO / ETA)
    if verl == "k1":   # Rohr.cpp:29-40, = Keefe 1. Ordnung (Wachter Gl. 3.13, 3.17)
        eps = (1 + (GAM - 1) / np.sqrt(PR)) / (np.sqrt(2) * rv)
        return 1j * k * (1 + (1 - 1j) * eps), zc0 * np.ones_like(w)
    # Keefe bis 3. Ordnung, Wachter Gl. 3.12-3.20; nu = sqrt(Pr)
    nu = np.sqrt(PR)
    psi = (GAM - 1) / nu
    a1 = (1 - psi) / np.sqrt(2)
    a2 = 1 - psi + psi / (2 * nu) + 1.5 * psi ** 2
    a3 = (7 / 8 - psi + psi / (2 * nu) + psi / (8 * nu ** 2) + 1.5 * psi ** 2 - 1.5 * psi ** 2 / nu - 2.5 * psi ** 3) / np.sqrt(2)
    b1 = (1 + psi) / np.sqrt(2)
    b2 = 1 + psi - psi / (2 * nu) - psi ** 2 / 2
    b3 = (7 / 8 + psi - psi / (2 * nu) - psi / (8 * nu ** 2) - psi ** 2 / 2 + psi ** 2 / (2 * nu) + psi ** 3 / 2) / np.sqrt(2)
    g = k * ((b1 / rv + b2 / rv ** 2 + b3 / rv ** 3) + 1j * (1 + b1 / rv - b3 / rv ** 3))
    # Vorzeichen des Imaginaerteils fuer e^{+j w t} (Wachter druckt +j; Keefe rechnet mit e^{-i w t}) — eigene Einschaetzung
    zc = zc0 * ((1 + a1 / rv - a3 / rv ** 3) - 1j * (a1 / rv + a2 / rv ** 2 + a3 / rv ** 3))
    return g, zc


def mal(A, B):
    return (A[0] * B[0] + A[1] * B[2], A[0] * B[1] + A[1] * B[3], A[2] * B[0] + A[3] * B[2], A[2] * B[1] + A[3] * B[3])


def zylinder(r, l, w, c, verl):
    g, zc = gamma_zc(r, w, c, verl)
    ch, sh = np.cosh(g * l), np.sinh(g * l)
    return (ch, zc * sh, sh / zc, ch)


def kegel(r1, r2, l, w, c, verl):
    """Kegelstumpf in Kugelwellen: p = e^(-+Gamma x)/x, U = -S/(rho c Gamma) dp/dx (eigene Herleitung aus der
    Webster-Gleichung fuer S ~ x^2; Gamma mit dem mittleren Radius)."""
    if abs(r2 - r1) < 1e-6:
        return zylinder(0.5 * (r1 + r2), l, w, c, verl)
    g, zc = gamma_zc(0.5 * (r1 + r2), w, c, verl)
    zc_m = zc * np.pi * (0.5 * (r1 + r2)) ** 2   # rho c (ggf. mit Keefe-Korrektur) je Flaeche
    x1 = r1 * l / (r2 - r1)
    x2 = x1 + l

    def basis(x, r):
        S = np.pi * r * r
        pp = np.exp(-g * (x - x1)) / x
        pm = np.exp(g * (x - x1)) / x
        up = -S / (zc_m * g) * (-g - 1 / x) * pp
        um = -S / (zc_m * g) * (g - 1 / x) * pm
        return pp, pm, up, um
    a = basis(x1, r1)
    b = basis(x2, r2)
    # T = M1 * inv(M2)
    det = b[0] * b[3] - b[1] * b[2]
    inv = (b[3] / det, -b[1] / det, -b[2] / det, b[0] / det)
    return mal((a[0], a[1], a[2], a[3]), inv)


def kette(x, r, w, c=C0, verl="k1", art="treppe"):
    E = (np.ones_like(w, dtype=complex), np.zeros_like(w, dtype=complex), np.zeros_like(w, dtype=complex), np.ones_like(w, dtype=complex))
    for i in range(len(x) - 1):
        l = x[i + 1] - x[i]
        if l <= 0:
            continue
        if art == "treppe":
            n = 1 if r[i] == r[i + 1] else max(1, int(np.ceil(l / 1e-3)))
            for s in range(n):
                t = (s + 0.5) / n
                E = mal(E, zylinder(r[i] + t * (r[i + 1] - r[i]), l / n, w, c, verl))
        else:
            E = mal(E, kegel(r[i], r[i + 1], l, w, c, verl))
    return E


def abstrahlung(a, w, c=C0, art="klein"):
    zc = RHO * c / (np.pi * a * a)
    ka = w / c * a
    if art == "klein":   # Rohr.cpp:111-112; Hendrie 2007 Gl. 2.91 (0,6 statt 0,6133)
        return zc * (ka ** 2 / 4 + 1j * 0.6133 * ka)
    if art == "ideal":
        return 0 * zc
    # Silva et al. 2009, Tab. 1, ohne Flansch; R = -|R| e^{-2jkL} fuer e^{+jwt} (konjugiert zu Gl. 2)
    beta, eta = 0.5, 0.6133
    a1, a2, a3 = 0.800, 0.266, 0.0263
    b1, b2, b3, b4 = 0.0599, 0.238, -0.0153, 0.00150
    betr = (1 + a1 * ka ** 2) / (1 + (beta + a1) * ka ** 2 + a2 * ka ** 4 + a3 * ka ** 6)
    L = a * eta * (1 + b1 * ka ** 2) / (1 + b2 * ka ** 2 + b3 * ka ** 4 + b4 * ka ** 6)
    R = -betr * np.exp(-2j * w / c * L)
    return zc * (1 + R) / (1 - R)


def eingang(x, r, w=W, c=C0, verl="k1", art="treppe", rad="klein", kessel=0.0):
    a, b, cc, d = kette(x, r, w, c, verl, art)
    zl = abstrahlung(r[-1], w, c, rad)
    z = (a * zl + b) / (cc * zl + d)
    if kessel > 0:
        z = 1 / (1j * w * kessel / (RHO * c * c) + 1 / z)
    return z


def spitzen(z, f=F):
    m = np.abs(z)
    out = []
    for i in range(1, len(m) - 1):
        if m[i] > m[i - 1] and m[i] >= m[i + 1]:
            # Parabel in log|Z|
            y0, y1, y2 = np.log(m[i - 1]), np.log(m[i]), np.log(m[i + 1])
            dd = 0.5 * (y0 - y2) / (y0 - 2 * y1 + y2)
            fp = f[i] + dd * (f[1] - f[0])
            hp = np.exp(y1 - 0.25 * (y0 - y2) * dd)
            # Halbwertsbreite (-3 dB) linear interpoliert
            ziel = hp / np.sqrt(2)
            j = i
            while j > 0 and m[j] > ziel and not (m[j - 1] > m[j]):
                j -= 1
            k = i
            while k < len(m) - 1 and m[k] > ziel and not (m[k + 1] > m[k]):
                k += 1
            q = np.nan
            if m[j] <= ziel and m[k] <= ziel:
                fl = f[j] + (ziel - m[j]) / (m[j + 1] - m[j]) * (f[1] - f[0])
                fr = f[k - 1] + (m[k - 1] - ziel) / (m[k - 1] - m[k]) * (f[1] - f[0])
                q = fp / (fr - fl)
            out.append((fp, hp, q))
    return out


def cent(a, b):
    return 1200 * np.log2(a / b)


# ---------------------------------------------------------------- Messwerte (Wachter 2008)
TAB85 = [85.3, 131.8, 173.9, 220.4, 262.4, 305.6, 352.1, 397.5, 437.4, 482.8, 526.0, 572.5, 621.8, 661.8]  # 2.-15. max
ABB89_GRUEN = [(87.4, 26.85), (135.6, 32.29), (176.1, 42.53), (220.2, 58.62), (265.7, 55.95), (305.3, 57.83),
               (349.4, 52.14), (394.9, 47.85), (439.5, 46.08), (485.5, 39.02), (526.0, 31.55), (576.9, 22.67),
               (617.0, 19.85), (663.4, 16.61)]   # (Hz, MOhm) digitalisiert
ABB58_ROT_B = [122.4, 178.3, 234.6, 290.5, 346.8, 403.0, 456.7, 512.9, 568.8, 627.4]   # B-Seite mit Hand, 2.-11.
ABB58_ROT_B_H = [44.3, 60.8, 78.5, 70.2, 72.5, 60.0, 54.1, 35.2, 26.2, 20.5]


def zuordnen(sp, ziel, ab=None, n0=2):
    """Nach Nummer: die Zielwerte sind die Spitzen n0, n0+1, ... (Spitze 1 = Pedal, erste Spitze ueber 15 Hz)."""
    sp = sorted(sp, key=lambda t: t[0])
    return [sp[n0 - 1 + i] for i in range(len(ziel))]


def tabelle(titel, sp, n0=1):
    print(titel)
    for i, (fp, hp, q) in enumerate(sp):
        print("  %2d  %7.2f Hz  %6.2f MOhm  %5.1f dB(1 MOhm)  Q %5.1f" % (i + n0, fp, hp / 1e6, 20 * np.log10(hp / 1e6), q))


def main():
    x, r = bohrung_F()
    L = x[-1]
    print("=== Bohrung F-Seite: %d Punkte, Laenge %.4f m, Eingang r %.2f mm, Becher r %.1f mm" % (len(x), L, r[0] * 1e3, r[-1] * 1e3))
    np.savetxt(os.path.join(ORDNER, "bohrung_F_kernform.txt"),
               np.column_stack([np.diff(x), r[:-1], r[1:]]), fmt="%.6f",
               header="Abschnitte wie ic::Abschnitt: laengeM radiusEinM radiusAusM (Mundstueck Abb.52 + Abb.88 digitalisiert)")
    np.savetxt(os.path.join(ORDNER, "bohrung_F_punkte.txt"), np.column_stack([x, r * 2e3]), fmt="%.5f",
               header="x_m d_mm (Durchmesser)")

    # --- 0. Pruefung am geschlossenen Fall
    print("\n=== Pruefung: verlustfreier Zylinder L = 1 m, r = 5 mm, Ende ideal offen (p = 0)")
    xz, rz = np.array([0.0, 1.0]), np.array([0.005, 0.005])
    for art in ("treppe", "kegel"):
        a, b, cc, d = kette(xz, rz, W, C0, "keine", art)
        z = b / d
        sp = spitzen(z)
        print("  %s:" % art, " ".join("%.3f(%+.4f c)" % (s[0], cent(s[0], (2 * n + 1) * C0 / 4)) for n, s in enumerate(sp[:5])))
    print("  Soll (2n-1)c/4L:", " ".join("%.3f" % ((2 * n - 1) * C0 / 4) for n in range(1, 6)))
    # Kegel gegen Treppe an einem echten Kegel (verlustfrei, 1 m, 5 -> 50 mm, ideal offen)
    xk, rk = np.array([0.0, 1.0]), np.array([0.005, 0.05])
    s1 = spitzen(eingang(xk, rk, verl="keine", art="treppe", rad="ideal"))
    s2 = spitzen(eingang(xk, rk, verl="keine", art="kegel", rad="ideal"))
    print("  Kegel 5->50 mm, 1 m: Treppe", ["%.2f" % s[0] for s in s1[:5]], " Kugelwelle", ["%.2f" % s[0] for s in s2[:5]])
    print("  Abstand Treppe-Kugelwelle [Cent]:", ["%.3f" % cent(a[0], b[0]) for a, b in zip(s1[:5], s2[:5])])

    # --- 1. Modelle der F-Seite
    modelle = {
        "Kern (Profil am Eingang)": dict(verl="k1", art="treppe", rad="klein"),
        "Kegel": dict(verl="k1", art="kegel", rad="klein"),
        "LS": dict(verl="k1", art="treppe", rad="ls"),
        "K3+LS": dict(verl="k3", art="treppe", rad="ls"),
    }
    erg = {}
    for name, kw in modelle.items():
        erg[name] = spitzen(eingang(x, r, **kw))
    # Kernform: Kessel als Volumen (Mundstueck 0 ... 31,5 mm), Bohrung ab der Seele
    lm, rm = mundstueck()
    ik = np.searchsorted(x, 0.0315)
    vk = sum(np.pi * (x[i + 1] - x[i]) / 3 * (r[i] ** 2 + r[i] * r[i + 1] + r[i + 1] ** 2) for i in range(ik))
    print("\nKesselvolumen Mundstueck 0-31,5 mm (Kegelstuempfe aus Abb. 52): %.3f cm^3; ganzes Mundstueck 0-67,38 mm: %.3f cm^3"
          % (vk * 1e6, sum(np.pi * (lm[i + 1] - lm[i]) / 3 * (rm[i] ** 2 + rm[i] * rm[i + 1] + rm[i + 1] ** 2) for i in range(len(lm) - 1)) * 1e6))
    erg["Kernform (Kessel V + Seele)"] = spitzen(eingang(x[ik:], r[ik:], verl="k1", art="treppe", rad="klein", kessel=vk))
    erg["Kernform + LS"] = spitzen(eingang(x[ik:], r[ik:], verl="k1", art="treppe", rad="ls", kessel=vk))

    for name, sp in erg.items():
        zu = zuordnen(sp, TAB85)
        dc = [cent(s[0], m) for s, m in zip(zu, TAB85)]
        dh = [20 * np.log10(s[1] / 1e6 / g[1]) for s, g in zip(zu, ABB89_GRUEN)]
        print("\n=== %s: gegen Wachter Tab. 85 (2.-15. Spitze)" % name)
        print("  n  f_rech   f_mess   Cent   |Z| MOhm  gem.MOhm  dB    Q")
        for n, (s, m, c_, g, h_) in enumerate(zip(zu, TAB85, dc, ABB89_GRUEN, dh), start=2):
            print("  %2d %7.2f %7.1f %+6.1f   %6.1f   %6.1f  %+5.1f  %5.1f" % (n, s[0], m, c_, s[1] / 1e6, g[1], h_, s[2]))
        print("  Mittel Cent %.1f, Streuung (Std) %.1f; Mittel dB %.1f" % (np.mean(dc), np.std(dc), np.mean(dh)))
        erste = [s for s in sp if s[0] < 60]
        if erste:
            print("  1. Spitze (Pedal): %.1f Hz (Wachter Text S. 63: Pedalton 25 Hz)" % erste[0][0])
    tabelle("\nKern: alle Spitzen bis 1000 Hz", erg["Kern (Profil am Eingang)"])

    # Hauptrohr geglaettet (die Strichmarken und die 0,17-mm-Stufen der Digitalisierung heraus)
    mg = (x > 0.40) & (x < 2.75)
    rg = r.copy()
    rg[mg] = np.median(r[mg])
    sg = spitzen(eingang(x, rg))
    dcg = [cent(s[0], m) for s, m in zip(zuordnen(sg, TAB85), TAB85)]
    print("\nHauptrohr 0,40-2,75 m auf den Median r = %.3f mm gesetzt: Cent gegen Tab. 85: %s; Mittel %.1f, Std %.1f"
          % (np.median(r[mg]) * 1e3, " ".join("%+.0f" % v for v in dcg), np.mean(dcg), np.std(dcg)))
    # Laengenausgleich: Zylinder im Hauptrohr bei 1,5 m, bis die mittlere Abweichung 0 ist
    i15 = np.searchsorted(x, 1.5)
    for dl in (-0.10, -0.08, -0.06, -0.04):
        xa = np.concatenate([x[:i15], x[i15:] + dl])
        if dl < 0:   # kuerzen: Punkte zwischen 1,5 und 1,5-dl entfernen
            keep = ~((x >= x[i15]) & (x < x[i15] - dl))
            xa = np.concatenate([x[:i15], x[i15:][keep[i15:]] + dl])
            ra = np.concatenate([rg[:i15], rg[i15:][keep[i15:]]])
        sa = spitzen(eingang(xa, ra))
        d_ = [cent(s[0], m) for s, m in zip(zuordnen(sa, TAB85), TAB85)]
        print("  geglaettet, Hauptrohr um %.2f m gekuerzt (Laenge %.3f m): Mittel %+.1f Cent, Std %.1f, je Spitze %s"
              % (-dl, xa[-1], np.mean(d_), np.std(d_), " ".join("%+.0f" % v for v in d_)))

    # Temperatur: welche Temperatur gleicht die mittlere Abweichung aus?
    zt = [spitzen(eingang(x, r, c=luft_bei(t))) for t in (20.0, 25.0)]
    d5 = np.mean([cent(a[0], b[0]) for a, b in zip(zuordnen(zt[1], TAB85), zuordnen(zt[0], TAB85))])
    print("\nTemperatur: +5 Grad C verschiebt die Spitzen 2-15 im Mittel um %+.1f Cent" % d5)

    # --- 2. Becher als Bessel-Horn (Anpassung) ---------------------------------
    print("\n=== Becher als Bessel-Horn r = b (x0 - x)^(-g), angepasst ab x = 3,0 m")
    sel = x >= 3.0
    bestes = None
    for x0 in np.linspace(x[-1] + 0.001, x[-1] + 0.15, 600):
        A = np.column_stack([np.ones(sel.sum()), -np.log(x0 - x[sel])])
        coef, res, *_ = np.linalg.lstsq(A, np.log(r[sel]), rcond=None)
        e = np.sum((A @ coef - np.log(r[sel])) ** 2)
        if bestes is None or e < bestes[0]:
            bestes = (e, x0, np.exp(coef[0]), coef[1])
    e, x0, bb, gg = bestes
    rb = bb * (x0 - x[sel]) ** (-gg)
    print("  x0 = %.4f m (Ende %.4f m), b = %.5g, gamma = %.4f, Restfehler rms %.1f %% in r, max %.1f %%"
          % (x0, x[-1], bb, gg, 100 * np.sqrt(e / sel.sum()), 100 * np.max(np.abs(rb / r[sel] - 1))))
    xb = x.copy()
    rb_all = r.copy()
    rb_all[sel] = rb
    sp_b = spitzen(eingang(xb, rb_all))
    print("  Bessel-Becher statt Punktliste, Spitzen 2-12 [Cent]:",
          ["%+.1f" % cent(a[0], b[0]) for a, b in zip(zuordnen(sp_b, TAB85[:11]), zuordnen(erg["Kern (Profil am Eingang)"], TAB85[:11]))])
    print("  ... Hoehen [dB]:", ["%+.1f" % (20 * np.log10(a[1] / b[1])) for a, b in zip(zuordnen(sp_b, TAB85[:11]), zuordnen(erg["Kern (Profil am Eingang)"], TAB85[:11]))])

    # --- 3. Empfindlichkeit ---------------------------------------------------
    print("\n=== Empfindlichkeit (Kernmodell, zentrale Differenz +-1 %), Spitzen 2-12")
    basis = zuordnen(erg["Kern (Profil am Eingang)"], TAB85[:11])
    i_lp0 = np.searchsorted(x, 0.0674)
    i_lp1 = np.searchsorted(x, 0.30)
    r_mz = np.median(r[(x > 0.5) & (x < 2.5)])
    print("  Mundrohr: x %.3f ... %.3f m, r %.2f -> %.2f mm; Hauptzylinder r (Median 0,5-2,5 m) %.2f mm"
          % (x[i_lp0], x[i_lp1], r[i_lp0] * 1e3, r[i_lp1] * 1e3, r_mz * 1e3))

    def var_mundrohr_laenge(s):
        xn = x.copy()
        d = (x[i_lp1] - x[i_lp0]) * s
        t = np.clip((x - x[i_lp0]) / (x[i_lp1] - x[i_lp0]), 0, 1)
        xn = x + d * t
        return xn, r

    def var_mundrohr_konus(s):   # Eingangsradius des Mundrohrs um s, zum Ende hin auf 0 auslaufend
        t = np.clip((x - x[i_lp0]) / (x[i_lp1] - x[i_lp0]), 0, 1)
        rn = r * np.where((x >= x[i_lp0]) & (x <= x[i_lp1]), 1 + s * (1 - t), 1.0)
        return x, rn

    def var_becherradius(s):
        t = np.clip((x - 3.0) / (x[-1] - 3.0), 0, 1)
        return x, r * (1 + s * t)

    def var_bessel(s):
        g2 = gg * (1 + s)
        # gleiche Endradien bei x = 3,0 m und am Ende: x0 und b neu
        xa, xe = x[sel][0], x[-1]
        ra, re = r[sel][0], r[-1]
        q = (re / ra) ** (1 / g2)   # (x0 - xa)/(x0 - xe) = q
        x0n = (q * xe - xa) / (q - 1)
        bn = re * (x0n - xe) ** g2
        rn = rb_all.copy()
        rn[sel] = bn * (x0n - x[sel]) ** (-g2)
        return x, rn

    def var_kessel(s):
        rn = r.copy()
        rn[:ik + 1] = r[:ik + 1] * np.sqrt(1 + s)
        return x, rn

    def var_zylinder(s):
        m = (x > 0.4) & (x < 2.8)
        return x, r * np.where(m, 1 + s, 1.0)

    varianten = [("Mundrohr-Laenge (0,067-0,30 m)", var_mundrohr_laenge, False),
                 ("Mundrohr-Konus (Eingangsradius)", var_mundrohr_konus, False),
                 ("Becherradius (Ende, ab 3,0 m eingeblendet)", var_becherradius, False),
                 ("Bessel-Exponent gamma", var_bessel, True),
                 ("Mundstueck-Kesselvolumen", var_kessel, False),
                 ("Radius Hauptrohr 0,4-2,8 m", var_zylinder, False)]
    for name, fn, bessel in varianten:
        res = []
        for s in (0.01, -0.01):
            xn, rn = fn(s)
            res.append(zuordnen(spitzen(eingang(xn, rn)), TAB85[:11]))
        bz = zuordnen(spitzen(eingang(xb, rb_all)), TAB85[:11]) if bessel else basis
        cc = [cent(a[0], b[0]) / 2 for a, b in zip(res[0], res[1])]
        db = [20 * np.log10(a[1] / b[1]) / 2 for a, b in zip(res[0], res[1])]
        print("  %-42s Cent/%%: %s" % (name, " ".join("%+5.1f" % v for v in cc)))
        print("  %-42s dB/%%:   %s" % ("", " ".join("%+5.2f" % v for v in db)))

    # --- 4. Ventilzuege: Ort und Radius ----------------------------------------
    print("\n=== Ventilzuege (Annahme: ideale Laengen L_eff (2^(k/12) - 1), L_eff aus der 4. Spitze)")
    f4 = basis[2][0]
    leff = 4 * C0 / (2 * f4)   # f_n = n c / (2 L_eff) fuer die Naturtonreihe (4. Spitze, n = 4)
    print("  L_eff = %.3f m (aus f4 = %.2f Hz, n = 4)" % (leff, f4))
    for k, name in ((1, "Ventil 2"), (2, "Ventil 1"), (3, "Ventil 3")):
        dl = leff * (2 ** (k / 12) - 1)
        # (a) im Hauptrohr bei 1,5 m mit dessen Radius; (b) an der Seele wie die Kernform (Zylinder am Eingang)
        i = np.searchsorted(x, 1.5)
        # Einfuegung: Zylinder mit r(x[i-1]) der Laenge dl zwischen x[i-1] und x[i]
        xa = np.insert(np.concatenate([x[:i], x[i:] + dl]), i, x[i - 1] + dl)
        ra = np.insert(r, i, r[i - 1])
        sa = zuordnen(spitzen(eingang(xa, ra)), TAB85[:11])
        sb =zuordnen(spitzen(eingang(np.concatenate([[0.0], x[ik:] - x[ik] + dl]), np.concatenate([[r[ik]], r[ik:]]), kessel=vk)), TAB85[:11])
        s0 = zuordnen(erg["Kernform (Kessel V + Seele)"], TAB85[:11])
        soll = -100 * k
        ca = [cent(a[0], b[0]) for a, b in zip(sa, basis)]
        cb = [cent(a[0], b[0]) for a, b in zip(sb, s0)]
        print("  %s (+%d HT, dL = %.3f m): Hauptrohr r=%.2f mm: %s" % (name, k, dl, r[i - 1] * 1e3, " ".join("%+5.0f" % v for v in ca)))
        print("  %s                         an der Seele r=%.2f mm: %s" % (" " * len(name), r[ik] * 1e3, " ".join("%+5.0f" % v for v in cb)))

    # --- 5. B-Seite: Hauptrohr kuerzen (Annahme), angepasst an Abb. 58 rot -------
    print("\n=== B-Seite (Annahme: F-Bohrung, Hauptrohr bei 1,0-2,6 m um dL gekuerzt; Ziel Abb. 58 rot, digitalisiert)")

    def kuerze(dl):
        a0 = np.searchsorted(x, 1.0)
        m = (x > 1.0) & (x < 1.0 + dl)
        xn = np.concatenate([x[:a0], x[a0:][x[a0:] >= 1.0 + dl] - dl])
        rn = np.concatenate([r[:a0], r[a0:][x[a0:] >= 1.0 + dl]])
        return xn, rn
    beste = None
    for dl in np.arange(0.80, 1.20, 0.005):
        xn, rn = kuerze(dl)
        zu = zuordnen(spitzen(eingang(xn, rn)), ABB58_ROT_B[:8], ab=90)
        e = np.mean([cent(s[0], m) for s, m in zip(zu, ABB58_ROT_B[:8])])
        if beste is None or abs(e) < abs(beste[1]):
            beste = (dl, e)
    dl = beste[0]
    xn, rn = kuerze(dl)
    spb = spitzen(eingang(xn, rn))
    zu = zuordnen(spb, ABB58_ROT_B, ab=90)
    print("  dL = %.3f m -> Laenge %.3f m" % (dl, xn[-1]))
    print("  n  f_rech  f_Abb58  Cent  |Z| MOhm  Abb58 MOhm  dB")
    for n, (s, m, h_) in enumerate(zip(zu, ABB58_ROT_B, ABB58_ROT_B_H), start=2):
        print("  %2d %7.1f %7.1f %+6.1f %7.1f %7.1f %+5.1f" % (n, s[0], m, cent(s[0], m), s[1] / 1e6, h_, 20 * np.log10(s[1] / 1e6 / h_)))
    harm = [116.54 * n / 2 for n in range(2, 12)]
    print("  gegen B-Reihe (B2 = 116,54 Hz bei a' = 440, n*58,27 Hz) [Cent]:",
          " ".join("%+.0f" % cent(s[0], h) for s, h in zip(zuordnen(spb, harm, ab=90), harm)))
    harmF = [43.654 * n for n in range(2, 16)]
    print("  F-Seite (Kern) gegen F-Reihe (n*43,65 Hz) [Cent]:",
          " ".join("%+.0f" % cent(s[0], h) for s, h in zip(zuordnen(erg["Kern (Profil am Eingang)"], harmF), harmF)))
    print("  F-Seite Messung Tab. 85 gegen F-Reihe [Cent]:", " ".join("%+.0f" % cent(m, h) for m, h in zip(TAB85, harmF)))


main()

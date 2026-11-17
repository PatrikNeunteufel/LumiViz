"""Eingangsimpedanz einer Bohrung aus Kegel-/Zylindersegmenten (Uebertragungsmatrix, Kugelwellen im Kegel).

Gleichungen (Nummern wie im Bericht):
  G1  c = sqrt(gamma R T / m)                         Hendrie 2007, S. 13, (2.2)
  G2  r_v = 632.8 a sqrt(f) (1 - 0.0031 dT)            Hendrie S. 28, (2.73)
      r_t = 532.8 a sqrt(f) (1 - 0.0031 dT)            Hendrie S. 28, (2.74)
  G3  v = c [1 - 1/(sqrt2 r_v) - (gamma-1)/(sqrt2 r_t)] Hendrie S. 28, (2.75)
      alpha = (w/c) [1/(sqrt2 r_v) + (gamma-1)/(sqrt2 r_t)]  Hendrie S. 29, (2.76)
  G4  Gamma = w/v - i alpha (ersetzt k)                Hendrie S. 29, (2.79)
  G5  Zc = rho c / S                                   Hendrie S. 21, (2.45)
  G6  Kegel: p = (A e^{-iGx} + B e^{iGx}) / x, x vom Apex; Matrix aus p und U = S u,
      i w rho u = -dp/dx  (eigene Herleitung, geprueft gegen Hendrie (2.102)/(2.103))
  G7  Zylinder: Hendrie S. 39, (2.93)
  G8  Abstrahlung ungeflanscht Z = Zc[(ka)^2/4 + i 0.6 ka]   Hendrie S. 37, (2.91)
  G9  Abstrahlung geflanscht  Z = Zc[(ka)^2/2 + i 8/(3 pi) ka] Hendrie S. 34, (2.86)
Zeitkonvention e^{+i w t} wie bei Hendrie.
"""
import numpy as np

GAMMA = 1.4
R_GAS = 8.314462
M_AIR = 0.0289647
P0 = 101325.0


def luft(T_C=20.0, c=None):
    T = T_C + 273.15
    c_ = np.sqrt(GAMMA * R_GAS * T / M_AIR) if c is None else c   # G1
    rho = P0 * M_AIR / (R_GAS * T)
    return dict(T=T, c=c_, rho=rho)


def gamma_k(f, a, luft_, verlust=True):
    """Ausbreitungskonstante (G2-G4); a in m."""
    w = 2 * np.pi * f
    c = luft_["c"]
    if not verlust:
        return w / c + 0j
    dT = luft_["T"] - 300.0
    rv = 632.8 * a * np.sqrt(f) * (1 - 0.0031 * dT)
    rt = 532.8 * a * np.sqrt(f) * (1 - 0.0031 * dT)
    v = c * (1 - 1 / (np.sqrt(2) * rv) - (GAMMA - 1) / (np.sqrt(2) * rt))
    alpha = (w / c) * (1 / (np.sqrt(2) * rv) + (GAMMA - 1) / (np.sqrt(2) * rt))
    return w / v - 1j * alpha


def _matrix_kegel(G, x, S, w, rho):
    e_m = np.exp(-1j * G * x)
    e_p = np.exp(1j * G * x)
    m11 = e_m / x
    m12 = e_p / x
    m21 = S * e_m * (1 + 1j * G * x) / (1j * w * rho * x * x)
    m22 = S * e_p * (1 - 1j * G * x) / (1j * w * rho * x * x)
    return m11, m12, m21, m22


def _schritt(Z2, f, L, r1, r2, luft_, verlust):
    """Impedanz am Eingang eines Segments (L, r1 -> r2, alles in m) bei Last Z2."""
    w = 2 * np.pi * f
    rho = luft_["rho"]
    a = 0.5 * (r1 + r2)
    G = gamma_k(f, a, luft_, verlust)
    if abs(r2 - r1) < 1e-4 * a:      # Zylinder (G7)
        # Zc = rho w / (Gamma S): im verlustfreien Fall = rho c / S (G5); mit Verlusten derselbe
        # Wellenwiderstand, den die Kegelmatrix (G6) im Grenzfall x -> unendlich liefert (eigene Wahl,
        # damit Zylinder- und Kegelscheiben ohne Scheinreflexion aneinanderpassen; Wirkung ~1e-3).
        Zc = rho * w / (G * np.pi * a * a)
        t = np.tan(G * L)
        return Zc * (Z2 + 1j * Zc * t) / (1j * Z2 * t + Zc)
    x1 = r1 * L / (r2 - r1)
    x2 = x1 + L
    S1 = np.pi * r1 * r1
    S2 = np.pi * r2 * r2
    a11, a12, a21, a22 = _matrix_kegel(G, x1, S1, w, rho)
    b11, b12, b21, b22 = _matrix_kegel(G, x2, S2, w, rho)
    det = b11 * b22 - b12 * b21
    i11, i12, i21, i22 = b22 / det, -b12 / det, -b21 / det, b11 / det
    t11 = a11 * i11 + a12 * i21
    t12 = a11 * i12 + a12 * i22
    t21 = a21 * i11 + a22 * i21
    t22 = a21 * i12 + a22 * i22
    return (t11 * Z2 + t12) / (t21 * Z2 + t22)


def z_abstrahlung(f, a, luft_, art="unflanged"):
    k = 2 * np.pi * f / luft_["c"]
    Zc = luft_["rho"] * luft_["c"] / (np.pi * a * a)
    ka = k * a
    if art == "unflanged":
        return Zc * (ka * ka / 4 + 1j * 0.6 * ka)          # G8
    if art == "flanged":
        return Zc * (ka * ka / 2 + 1j * 8 / (3 * np.pi) * ka)  # G9
    if art == "offen":
        return np.zeros_like(f) + 0j
    raise ValueError(art)


def zin(segmente_mm, f, luft_, verlust=True, abstrahlung="unflanged", scheibe_mm=10.0, lumped=None):
    """segmente_mm: Liste (L, r_ein, r_aus) in mm vom Mundstueck zum Becher.
    lumped: optional (V_mm3, l_eff_mm, r_eng_mm) = Kesselvolumen parallel + Verengung in Reihe am Eingang."""
    f = np.asarray(f, dtype=float)
    r_end = segmente_mm[-1][2] * 1e-3
    Z = z_abstrahlung(f, r_end, luft_, abstrahlung)
    for L, r1, r2 in reversed(segmente_mm):
        n = max(1, int(np.ceil(L / scheibe_mm))) if verlust else 1
        rs = np.linspace(r1, r2, n + 1) * 1e-3
        dl = L * 1e-3 / n
        for j in reversed(range(n)):
            Z = _schritt(Z, f, dl, rs[j], rs[j + 1], luft_, verlust)
    if lumped is not None:
        V, l_eff, r_eng = lumped
        w = 2 * np.pi * f
        C = V * 1e-9 / (luft_["rho"] * luft_["c"] ** 2)
        M = luft_["rho"] * l_eff * 1e-3 / (np.pi * (r_eng * 1e-3) ** 2)
        Z = 1 / (1j * w * C + 1 / (1j * w * M + Z))
    return Z


def spitzen(segmente_mm, luft_, verlust=True, abstrahlung="unflanged", fmax=900.0, lumped=None, n_max=16):
    """Impedanzspitzen: Frequenz (Hz), |Z| (MPa s/m^3 = 1e6 Pa s m^-3), Guete Q (-3 dB)."""
    fg = np.arange(15.0, fmax, 0.2)
    Zg = np.abs(zin(segmente_mm, fg, luft_, verlust, abstrahlung, lumped=lumped))
    idx = [i for i in range(1, len(fg) - 1) if Zg[i] > Zg[i - 1] and Zg[i] >= Zg[i + 1]]
    # schwache Nebenmaxima ausschliessen: Prominenz gegen das tiefere Nachbarminimum
    gut = []
    for i in idx:
        lo = Zg[max(0, i - 400):i].min() if i > 0 else Zg[i]
        hi = Zg[i + 1:i + 400].min() if i + 1 < len(Zg) else Zg[i]
        if Zg[i] > 1.15 * max(lo, hi):
            if gut and fg[i] - fg[gut[-1]] < 5.0:     # zwei Grob-Maxima derselben Spitze
                if Zg[i] > Zg[gut[-1]]:
                    gut[-1] = i
                continue
            gut.append(i)
    erg = []
    gut = gut[:n_max]
    for m, i in enumerate(gut):
        f0 = fg[i]
        nachbarn = [abs(fg[j] - f0) for j in (gut[m - 1] if m > 0 else None, gut[m + 1] if m + 1 < len(gut) else None)
                    if j is not None]
        breite = min([6.0, 0.06 * f0] + [0.45 * d for d in nachbarn])
        fl = np.linspace(f0 - breite, f0 + breite, 4001)
        Zl = np.abs(zin(segmente_mm, fl, luft_, verlust, abstrahlung, lumped=lumped))
        j = int(np.argmax(Zl))
        fp, zp = fl[j], Zl[j]
        h = zp / np.sqrt(2)
        Q = np.nan
        links = np.where(Zl[:j] < h)[0]
        rechts = np.where(Zl[j:] < h)[0]
        if len(links) and len(rechts):
            jl = links[-1]
            jr = j + rechts[0]
            fl1 = np.interp(h, [Zl[jl], Zl[jl + 1]], [fl[jl], fl[jl + 1]])
            fr1 = np.interp(h, [Zl[jr], Zl[jr - 1]], [fl[jr], fl[jr - 1]])
            Q = fp / (fr1 - fl1)
        erg.append((fp, zp / 1e6, Q))
    return erg


def cent(f, ref):
    return 1200 * np.log2(np.asarray(f) / np.asarray(ref))

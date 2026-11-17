"""LVR Auftrag 11, Punkt 4: eine Mode der Stegadmittanz Y(s) = s / (s^2 + eta w_n s + w_n^2) (phi^2 = 1, Q = 1/eta) bei
fs = 48 kHz diskretisiert. Pol lambda = -eta w_n / 2 + i w_d, Residuum R = lambda / (2 i w_d); Y(s) = R/(s-lambda) + c.c.

  ii       impulsinvariant:          H(z) = T [R / (1 - p z^-1) + c.c.],               p = exp(lambda T)
  trapez   ii mit halbem Durchgriff:  H(z) = ii - T h(0) / 2,  h(0) = 1  (die heutige Form des Kerns)
  bilin    bilinear:                 s = (2/T)(1 - z^-1)/(1 + z^-1)
  bilin_v  bilinear, vorverzerrt:    s = (w_n / tan(w_n T / 2)) (1 - z^-1)/(1 + z^-1)
  foh      Kraft stueckweise linear: H(z) = R [(p - 1 - x) + (1 - p + x p) z^-1] / (lambda x (1 - p z^-1)) + c.c.,  x = lambda T

Gemessen gegen Y(i w): Lage der Spitze von |Y| (Hz und Cent), Guete aus der -3-dB-Breite, Hoehe der Spitze (dB),
Fehler von |Y| im Band f_n/2 ... min(2 f_n, 23,9 kHz) (groesster Betrag dB), kleinster Re Y_d auf 0..fs/2 relativ zur
Spitze von Re Y (Passivitaet: muss >= 0 sein), Durchgriff Y_d(z = inf) (muss >= 0 sein, damit I + Z0 Y_i invertierbar
bleibt). Aufruf: python -B moden_diskret.py
"""
import numpy as np

FS = 48000.0
T = 1 / FS
f = np.linspace(1.0, FS / 2 - 1.0, 400001)
w = 2 * np.pi * f
zi = np.exp(-1j * w * T)            # z^-1


def kont(fn, Q):
    wn = 2 * np.pi * fn
    s = 1j * w
    return s / (s * s + wn / Q * s + wn * wn)


def pol_res(fn, Q):
    wn = 2 * np.pi * fn
    eta = 1 / Q
    wd = wn * np.sqrt(1 - eta * eta / 4)
    lam = -eta * wn / 2 + 1j * wd
    return lam, lam / (2j * wd)


def diskret(fn, Q, art):
    lam, R = pol_res(fn, Q)
    wn = 2 * np.pi * fn
    if art in ("ii", "trapez"):
        p = np.exp(lam * T)
        H = T * (R / (1 - p * zi) + np.conj(R) / (1 - np.conj(p) * zi))
        d0 = T * 2 * R.real
        if art == "trapez":
            H = H - T / 2
            d0 -= T / 2
        return H, d0
    if art in ("bilin", "bilin_v"):
        c = 2 / T if art == "bilin" else wn / np.tan(wn * T / 2)
        s = c * (1 - zi) / (1 + zi)
        H = s / (s * s + wn / Q * s + wn * wn)
        d0 = c / (c * c + wn / Q * c + wn * wn)         # z^-1 -> 0: s -> c
        return H, d0
    if art == "foh":
        p = np.exp(lam * T)
        x = lam * T
        b0 = R * (p - 1 - x) / (lam * x)
        b1 = R * (1 - p + x * p) / (lam * x)
        H =(b0 + b1 * zi) / (1 - p * zi) + (np.conj(b0) + np.conj(b1) * zi) / (1 - np.conj(p) * zi)
        return H, 2 * b0.real
    raise ValueError(art)


def kennwerte(Y):
    a = np.abs(Y)
    i = int(np.argmax(a))
    fp = f[i]
    h = a[i] / np.sqrt(2)
    lo = np.where(a[:i] < h)[0]
    hi = np.where(a[i:] < h)[0]
    if len(lo) == 0 or len(hi) == 0:
        return fp, float("nan"), 20 * np.log10(a[i])
    bw = f[i + hi[0]] - f[lo[-1]]
    return fp, fp / bw, 20 * np.log10(a[i])


ARTEN = ("ii", "trapez", "bilin", "bilin_v", "foh")
print("f_n kHz  Q | Art     | Spitze Hz (Cent)        | Q (Fehler %)   | Hoehe dB-Fehler | |Y| groesster dB-Fehler im Band | min Re Y_d / max Re Y | Durchgriff Y_i (s/kg, x 1e6)")
for fn in (5000.0, 8000.0, 12000.0, 16000.0):
    for Q in (5.0, 10.0, 30.0):
        Yc = kont(fn, Q)
        fpc, Qc, hc = kennwerte(Yc)
        band = (f >= fn / 2) & (f <= min(2 * fn, 23900.0))
        print(f"{fn / 1000:5.0f} {Q:3.0f} | stetig  | {fpc:8.1f}                | {Qc:6.2f}         |                 |")
        for art in ARTEN:
            Y, d0 = diskret(fn, Q, art)
            fp, Qd, hd = kennwerte(Y)
            fehl = np.abs(20 * np.log10(np.abs(Y[band]) / np.abs(Yc[band]))).max()
            pr = Y.real.min() / Yc.real.max()
            print(f"         | {art:7s} | {fp:8.1f} ({1200 * np.log2(fp / fpc):+7.1f}) | {Qd:6.2f} ({100 * (Qd / Qc - 1):+6.1f}) | {hd - hc:+7.2f}         | {fehl:6.2f}"
                  f"                         | {pr:+.2e}           | {d0 * 1e6:+.3f}")
# Durchgriff-Gegenprobe Kern: trapez B0 = c dt/2 -> Re = dt/2 = 10,4 us; foh: 2 Re b0
print("T/2 =", T / 2 * 1e6, "us")

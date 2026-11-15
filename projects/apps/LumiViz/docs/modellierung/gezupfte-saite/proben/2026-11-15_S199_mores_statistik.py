"""S199, 34k-5 — die Gitarre ueber 5 kHz: drei Fassungen gegen die Messung (g34, Mores 2021).

Aufruf: python -B 2026-11-15_S199_mores_statistik.py <qualified_selected_impulses.mat> <classical_guitar.lvinstbody>
            [<fit_bis_10khz.npz>]

A  der Koerper von heute: 75 Moden bis 5 kHz, darueber nur ihre Auslaeufer
B  statistischer Teil 5-20 kHz, geschaetzt wie bei den Koerpern mit Bauplan (`schaetze` in InstrumentBody/Statistik.hpp)
   aus den gefitteten Moden im Band 2,5-5 kHz; Daempfungsgesetz alpha(f) = a f^2 + b f + c an die Moden angepasst
C  statistischer Teil 5-20 kHz, Groessen an der Messung bestimmt: Daempfung aus dem Abklingen je Terzband an den
   Mikrofonen (2,5-10 kHz), Betrag der Abstrahlung aus dem Pegel der Mikrofone (6,3-10 kHz); Streuung am Steg und
   Dichte wie B. C3: dasselbe mit dreifacher Dichte.
   Gegenprobe zu B: die Moden 2,5-5 kHz durch gezogene mit den geschaetzten Groessen ersetzt.
D  (mit npz) Modalfit bis 10 kHz: nur die Stegkurven (Aufnehmer), zum Vergleich der Pegel

`schaetze` und `ziehe` sind hier nachgebaut (SplitMix64, Rayleigh, Box-Muller wie im C++). Schreibt nichts.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.io as sio
import scipy.signal as sig
from scipy.optimize import nnls

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("make_gitarre", HIER.parents[7] / "asset/loop/instrument_bodies/make_gitarre.py")
mg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mg)

FS = mg.FS
MITTEN = [2000 * 2 ** (k / 3) for k in range(0, 11)]          # 2 kHz ... 20 kHz
M64 = (1 << 64) - 1


class Zufall:
    def __init__(self, samen):
        self.z = samen

    def naechste(self):
        self.z = (self.z + 0x9E3779B97F4A7C15) & M64
        z = self.z
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & M64
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & M64
        return z ^ (z >> 31)

    def gleich(self):
        return ((self.naechste() >> 11) + 0.5) / 9007199254740992.0

    def rayleigh(self):
        return np.sqrt(-4.0 * np.log(self.gleich()) / np.pi)

    def normal(self):
        u = self.gleich()
        v = self.gleich()
        return np.sqrt(-2.0 * np.log(u)) * np.cos(2.0 * np.pi * v)


def schaetze(f, steg, G, band_von, band_bis, von):
    m = (f >= band_von) & (f < band_bis)
    breite = (band_bis - band_von) / 10
    x = band_von + (np.arange(10) + 0.5) * breite
    y = np.array([np.sum((f >= band_von + i * breite) & (f < band_von + (i + 1) * breite)) for i in range(10)]) / breite
    steigung, achse = np.polyfit(x, y, 1)
    dichte = achse + steigung * von if steigung > 0 else y.mean()
    return {"dichte": float(dichte), "anstieg": float(max(0.0, steigung)), "moden": int(m.sum()),
            "steg": np.sqrt(np.mean(steg[m] ** 2, axis=0)), "hoer": np.sqrt(np.mean(np.abs(G[m]) ** 2, axis=0))}


def ziehe(st, gesetz, von, bis, samen=1):
    z = Zufall(samen)
    f, fs_, eta, steg, G = von, [], [], [], []
    while True:
        f += z.rayleigh() / (st["dichte"] + st["anstieg"] * (f - von))
        if f >= bis:
            break
        fs_.append(f)
        eta.append((gesetz[0] * f * f + gesetz[1] * f + gesetz[2]) / (2 * np.pi * f))
        steg.append([s * z.normal() for s in st["steg"]])
        G.append([b * np.exp(1j * 2 * np.pi * z.gleich()) for b in st["hoer"]])
    return np.array(fs_), np.array(eta), np.array(steg), np.array(G)


def terz(hz, p):
    """Terzpegel aus einem Leistungsspektrum."""
    return np.array([10 * np.log10(np.mean(p[(hz >= m / 2 ** (1 / 6)) & (hz < m * 2 ** (1 / 6))]) + 1e-300) for m in MITTEN])


def kern(hz, f, eta):
    w = 2 * np.pi * hz[:, None]
    wn = 2 * np.pi * f[None, :]
    return -w ** 2 / (wn ** 2 - w ** 2 + 1j * eta[None, :] * wn * w)


def pegel(hz, f, eta, steg, G):
    """Mittlere Leistung von p/F ueber die sechs Saiten je Hoerort; Uebertragung a/F zwischen verschiedenen Saiten."""
    B = kern(hz, f, eta)
    p = [np.mean([np.abs(B @ (steg[:, k] * G[:, h])) ** 2 for k in range(6)], axis=0) for h in range(2)]
    paare = [(0, 5), (1, 4), (0, 3), (2, 5), (1, 3), (2, 4)]
    a = np.mean([np.abs(B @ (steg[:, i] * steg[:, j])) ** 2 for i, j in paare], axis=0)
    schwank = [np.std(20 * np.log10(np.abs(B @ (steg[:, 2] * G[:, h]))[(hz >= 6000) & (hz < 10000)])) for h in range(2)]
    return p, a, schwank


def abklingen(x, ip, mitte):
    """alpha = 2 sigma (1/s) aus der Schroeder-Kurve des Terzbands (Energie ~ exp(-alpha t)), Rauschen abgezogen."""
    sos = sig.butter(4, [mitte / 2 ** (1 / 6), min(mitte * 2 ** (1 / 6), 0.49 * FS)], btype="band", fs=FS, output="sos")
    y = sig.sosfilt(sos, x)[ip:] ** 2
    rausch = np.mean(y[-4800:])
    n = int(0.25 * FS)
    e = np.cumsum((y[:n] - rausch)[::-1])[::-1]
    db = 10 * np.log10(np.maximum(e, 1e-300) / e[0])
    t = np.arange(n) / FS
    ende = int(np.argmax(db < -25)) if (db < -25).any() else n
    m = (db[:ende] <= -5)
    if m.sum() < 10:
        return float("nan")
    return float(-np.polyfit(t[:ende][m], db[:ende][m], 1)[0] / (10 * np.log10(np.e)))


def zeile(name, werte, fmt="{:6.1f}"):
    print(f"{name:<34}" + " ".join(fmt.format(v) for v in werte))


if __name__ == "__main__":
    d = sio.loadmat(sys.argv[1])["qualified_selected_impulses"][mg.GITARRE - 1]
    koerper = json.loads(pathlib.Path(sys.argv[2]).read_text(encoding="utf-8"))
    daten = {ort: d[o * FS:(o + 1) * FS, :] for o, ort in enumerate(mg.ORTE)}
    hz, H = mg.frfs(daten)
    sel = (hz >= 1500) & (hz <= 23000)
    hzs = hz[sel]
    print("Terzmitten Hz                     " + " ".join(f"{m:6.0f}" for m in MITTEN))

    mess_p = [terz(hzs, np.mean([np.abs(H[(o, mik)][sel]) ** 2 for o in mg.ORTE], axis=0)) for _, mik in mg.MIKROFONE]
    mess_a = terz(hzs, np.mean([np.abs(H[p][sel]) ** 2 for p in mg.PAARE], axis=0))
    mess_schw = [np.mean([np.std(20 * np.log10(np.abs(H[(o, mik)])[(hz >= 6000) & (hz < 10000)])) for o in mg.ORTE])
                 for _, mik in mg.MIKROFONE]
    zeile("Messung p/F left (dB re 1 Pa/N)", mess_p[0])
    zeile("Messung p/F right", mess_p[1])
    zeile("Messung a/F (dB re 1 m/s2/N)", mess_a)

    # Abklingen je Terzband, Mittel ueber Schlaege; Mikrofone und Aufnehmer getrennt
    sig_m, sig_a = [], []
    for m in MITTEN:
        wm, wa = [], []
        for ort, x in daten.items():
            ip = int(np.argmax(np.abs(x[:, 0])))
            wm += [abklingen(x[:, k], ip, m) for k in (4, 5)]
            wa += [abklingen(x[:, k], ip, m) for k in (1, 2)]
        sig_m.append(np.nanmedian(wm))
        sig_a.append(np.nanmedian(wa))
    zeile("alpha Mikrofone 1/s", sig_m, "{:6.0f}")
    zeile("alpha Aufnehmer 1/s", sig_a, "{:6.0f}")

    f = np.array([m["f"] for m in koerper["modes"]])
    eta = np.array([m["eta"] for m in koerper["modes"]])
    steg = np.array([m["bridge"] for m in koerper["modes"]])
    G = np.array([[complex(*g) for g in m["radiation"]] for m in koerper["modes"]])
    sigma = eta * 2 * np.pi * f / 2
    oben = f >= 2500
    zeile("alpha Moden (Median im Band) 1/s",
          [np.median(2 * sigma[(f >= m / 2 ** (1 / 6)) & (f < m * 2 ** (1 / 6))]) if ((f >= m / 2 ** (1 / 6)) & (f < m * 2 ** (1 / 6))).any()
           else float("nan") for m in MITTEN], "{:6.0f}")

    def sigma_modell(f_, eta_, steg_, G_):
        voll = np.fft.rfftfreq(FS, 1 / FS)
        Hm = kern(voll[1:], f_, eta_) @ (steg_[:, 2] * G_[:, 0])
        h = np.fft.irfft(np.concatenate([[0], Hm]), FS)
        return [abklingen(np.concatenate([np.zeros(200), h[:FS - 200]]), 200, m) for m in MITTEN]

    def bericht(name, f_, eta_, steg_, G_):
        p, a, schw = pegel(hzs, f_, eta_, steg_, G_)
        zeile(f"\n{name}: alpha 1/s", sigma_modell(f_, eta_, steg_, G_), "{:6.0f}")
        print(f"\n{name}: {len(f_)} Moden, laengstes Abklingen T60 = {6.908 / np.min(eta_ * np.pi * f_):.2f} s, "
              f"Schwankung 6-10 kHz {schw[0]:.1f} / {schw[1]:.1f} dB (Messung {mess_schw[0]:.1f} / {mess_schw[1]:.1f})")
        zeile("   p/F left  - Messung", terz(hzs, p[0]) - mess_p[0])
        zeile("   p/F right - Messung", terz(hzs, p[1]) - mess_p[1])
        zeile("   a/F       - Messung", terz(hzs, a) - mess_a)

    bericht("A heute", f, eta, steg, G)

    # B: wie die Koerper mit Bauplan
    st = schaetze(f, steg, G, 2500, 5000, 5000)
    A = np.stack([f[oben] ** 2, f[oben], np.ones(oben.sum())], axis=1)
    gesetz_b = nnls(A / (2 * sigma[oben])[:, None], np.ones(oben.sum()))[0]
    print(f"\nB Groessen: Dichte {st['dichte']:.4f} 1/Hz, Anstieg {st['anstieg']:.2e}, Band {st['moden']} Moden, Gesetz a b c = "
          f"{gesetz_b[0]:.3g} {gesetz_b[1]:.3g} {gesetz_b[2]:.3g}, Steg {np.round(st['steg'], 3)}, Hoer {np.round(st['hoer'], 3)}")
    fz, ez, sz, gz = ziehe(st, gesetz_b, 5000, 20000)
    bericht("B statistisch aus den Moden", np.concatenate([f, fz]), np.concatenate([eta, ez]), np.concatenate([steg, sz]),
            np.concatenate([G, gz]))

    # Gegenprobe zu B: die gefitteten Moden 2,5-5 kHz durch gezogene ersetzt
    staerke = np.sum(steg[oben] ** 2, axis=1)
    print(f"\nBand 2,5-5 kHz: Korrelation lg(Formwert^2) gegen lg(sigma) = "
          f"{np.corrcoef(np.log(staerke), np.log(sigma[oben]))[0, 1]:.2f}; quadratisches Mittel des Formwerts "
          f"{np.sqrt(np.mean(staerke) / 6):.2f}, mit 1/sigma gewichtet {np.sqrt(np.sum(staerke / sigma[oben]) / np.sum(1 / sigma[oben]) / 6):.2f}; "
          f"alpha im Mittel {np.mean(2 * sigma[oben]):.0f}, harmonisch {len(sigma[oben]) / np.sum(1 / (2 * sigma[oben])):.0f} 1/s")
    fz, ez, sz, gz = ziehe(st, gesetz_b, 2500, 5000)
    bericht("B-Gegenprobe (2,5-5 kHz gezogen)", np.concatenate([f[~oben], fz]), np.concatenate([eta[~oben], ez]),
            np.concatenate([steg[~oben], sz]), np.concatenate([G[~oben], gz]))

    # C: an der Messung
    mb = [i for i, m in enumerate(MITTEN) if 2400 < m < 10500]
    fm = np.array([MITTEN[i] for i in mb])
    al = np.array([sig_m[i] for i in mb])
    gesetz_c = nnls(np.stack([fm ** 2, fm, np.ones(len(fm))], axis=1) / al[:, None], np.ones(len(fm)))[0]
    ziel = [i for i, m in enumerate(MITTEN) if 6000 < m < 10500]
    for name, mal in (("C statistisch an der Messung", 1.0), ("C3 mit dreifacher Dichte", 3.0)):
        stc = dict(st)
        stc["dichte"] = st["dichte"] * mal
        stc["hoer"] = np.array(st["hoer"], dtype=float)
        for _ in range(8):
            fz, ez, sz, gz = ziehe(stc, gesetz_c, 5000, 20000)
            fa, ea, sa, ga = np.concatenate([f, fz]), np.concatenate([eta, ez]), np.concatenate([steg, sz]), np.concatenate([G, gz])
            p, _, _ = pegel(hzs, fa, ea, sa, ga)
            for h in range(2):
                stc["hoer"][h] *= 10 ** (np.mean((mess_p[h] - terz(hzs, p[h]))[ziel]) / 20)
        fz, ez, sz, gz = ziehe(stc, gesetz_c, 5000, 20000)
        print(f"\n{name} Groessen: Dichte {stc['dichte']:.4f} 1/Hz, Gesetz a b c = {gesetz_c[0]:.3g} {gesetz_c[1]:.3g} "
              f"{gesetz_c[2]:.3g}, Steg {np.round(stc['steg'], 3)}, Hoer {np.round(stc['hoer'], 4)}")
        bericht(name, np.concatenate([f, fz]), np.concatenate([eta, ez]), np.concatenate([steg, sz]), np.concatenate([G, gz]))

    if len(sys.argv) > 3:
        z = np.load(sys.argv[3])
        f2, s2 = z["f"], z["s"]
        steg2 = np.array([z["a"] + z["b"] * x + z["c"] * x * x for x in mg.X_SAITEN]).T
        B = kern(hzs, f2, 2 * s2 / (2 * np.pi * f2))
        paare = [(0, 5), (1, 4), (0, 3), (2, 5), (1, 3), (2, 4)]
        a = np.mean([np.abs(B @ (steg2[:, i] * steg2[:, j])) ** 2 for i, j in paare], axis=0)
        print(f"\nD Fit bis 10 kHz: {len(f2)} Moden ({int((f2 >= 5000).sum())} ueber 5 kHz)")
        zeile("   a/F       - Messung", terz(hzs, a) - mess_a)

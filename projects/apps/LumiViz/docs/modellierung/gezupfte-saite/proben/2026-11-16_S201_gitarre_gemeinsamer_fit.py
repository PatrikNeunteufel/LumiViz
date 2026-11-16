"""S201, 34k-6 Punkt (20) — Gitarre g34: die Formen am Steg an Steg- UND Schallmessung gemeinsam gestellt.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <qualified_selected_impulses.mat> [--gewicht <g>] [--ohne-toene]

Befund davor (`…_S201_gitarre_abstrahlung_700_1000.py`, `…_wege.py`): die Koerperdatei liegt am Schlagort treble in
740-960 Hz 9-15 dB unter der Messung; die Pole reichen (je Schlagort frei gestellt 1,4-2,1 dB), und die freien Gewichte
sind je Mode ein Produkt h(Schlag) G(Mikrofon) mit reellem h (Rang-1-Anteil 0,95) - aber h gleicht bei mehreren Moden
nicht der Form phi(x) aus Schritt 4 von `make_gitarre.py`. Schritt 4 sieht nur die zwei Aufnehmer an den Enden der
Stegeinlage; die neun Schallkurven (drei Schlagorte, drei Mikrofone) tragen dieselben Formwerte an den Schlagorten.

Hier: Pole wie in der Datei. Die Form phi_j(x) = a + b x + c x^2 so, dass zugleich die sechs Stegkurven
(R = phi(x_Schlag) phi(x_Aufnehmer)) und die neun Schallkurven (phi(x_Schlag) G_jm) getroffen werden - abwechselnd
G je Mikrofon (linear) und a, b, c (Gauss-Newton), Fehler relativ (1 / |Y|), die Schallkurven mit `--gewicht` (Vorgabe 1).
Der Fit sieht, was gespeichert wird: kein Restterm an den Schallkurven, der statistische Teil der Datei fest.

Wege (Zielgroessen fuer jeden):
  0  die Datei (FIT_FASSUNG 2)
  1  Formen gemeinsam, Abstrahlung wie Schritt 5 heute (Restterm im Fit, ab 1 kHz Betrag je Terzband, KQ24 a)
  2  Formen gemeinsam, je Mode ein komplexes Gewicht ueber den ganzen Bereich, ohne Restterm und ohne Grenze
  3  Formen der Datei, Gewichte wie in 2 (trennt die Wirkung der Formen von der der Grenze)

Zielgroessen: Fehler der Stegkurven je Bereich; wirksame Masse an Saite 6 (85-240 Hz, Wright 0,062 kg); Fehler des
Terzpegels der Schallkurven je Bereich und Hoerort; Fehler im Halbtonband der Tasten 40-83 (Effektivwert, groesster);
und mit dem Pruefstand die Lautheit je Taste 40-83 (Spanne, groesster und mittlerer Sprung ueber Paare bis 4 Halbtoene,
GZ14 a) - mit dem Zupfort min(16 cm, halbe Saite) wie bisher und mit festen 16 cm.

Schreibt nichts.
"""
import importlib.util
import pathlib
import sys
import time

import numpy as np
from scipy.optimize import least_squares

HIER = pathlib.Path(__file__).resolve().parent
ROOT = HIER.parents[6]


def modul(pfad, name):
    spec = importlib.util.spec_from_file_location(name, pfad)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


mg = modul(ROOT / "asset/loop/instrument_bodies/make_gitarre.py", "make_gitarre")
ps = modul(HIER / "2026-11-15_S199_pruefstand_zupf.py", "pruefstand")

args = sys.argv[1:]
GEWICHT = float(args[args.index("--gewicht") + 1]) if "--gewicht" in args else 1.0
MIT_TOENEN = "--ohne-toene" not in args
ORTE = mg.ORTE
XH = np.array([mg.X_SCHLAG[o] for o in ORTE])
XS = np.array(mg.X_SAITEN)
MIKS = ("mbass", "mtreble", "east")
SKALA = np.array([1.0, 30.0, 900.0])
TASTEN = list(range(40, 84))
BEREICHE = (("60-400", 60, 400), ("400-1000", 400, 1000), ("1000-2500", 1000, 2500), ("2500-5000", 2500, 5000))
TERZ = (("100-400", 95, 450), ("500-1000", 450, 1120), ("1250-2500", 1120, 2800), ("3150-5000", 2800, 5700))

start = time.time()
kp = ps.lade_koerper("classical_guitar")
d = mg.sio.loadmat(args[0])["qualified_selected_impulses"][mg.GITARRE - 1]
daten = {ort: d[o * mg.FS:(o + 1) * mg.FS, :] for o, ort in enumerate(ORTE)}
hz_alle, H = mg.frfs(daten)
sel = (hz_alle >= mg.VON) & (hz_alle <= mg.BIS)
hz = hz_alle[sel]
w = 2 * np.pi * hz
n = len(hz)
J = int(np.sum(kp["f"] <= mg.BIS))
f = kp["f"][:J]
s = kp["eta"][:J] * 2 * np.pi * f / 2
mob = [H[p][sel] / (1j * w) for p in mg.PAARE]
Bm = mg.basis(hz, f, s)                                   # Mobilitaet je Mode
Bp = Bm * (1j * w[:, None])                               # Druck: -w^2 / (...)
wn_st = 2 * np.pi * kp["f"][J:]
Bst = (-w[:, None] ** 2) / (wn_st[None, :] ** 2 - w[:, None] ** 2 + 1j * kp["eta"][None, J:] * wn_st[None, :] * w[:, None])
koeff_datei = np.polyfit(XS, kp["steg"][:J].T, 2)[::-1]   # (3, J): a, b, c
koeff_st = np.polyfit(XS, kp["steg"][J:].T, 2)[::-1]
rest_b = np.stack([1 / (1j * w), 1j * w * 1e-8], axis=1)
xh_p = np.array([mg.X_SCHLAG[p[0]] for p in mg.PAARE])
xs_p = np.array([mg.X_AUFN[p[1]] for p in mg.PAARE])
Ysteg = np.array(mob)
gew_steg = 1.0 / np.maximum(np.abs(Ysteg), 1e-6)
Ymik = {(h, m): H[(ORTE[h], MIKS[m])][sel] for h in range(3) for m in range(3)}


def form(k, x):
    return k[0][:, None] + k[1][:, None] * np.asarray(x)[None, :] + k[2][:, None] * np.asarray(x)[None, :] ** 2   # (J, Orte)


def fest_stat(h, m):
    """Der statistische Teil der Datei am Schlagort h (nur fuer left / right bekannt; east: 0)."""
    if m > 1:
        return np.zeros(n, dtype=complex)
    return Bst @ (form(koeff_st, [XH[h]])[:, 0] * kp["G"][J:, m])


FEST = {(h, m): fest_stat(h, m) for h in range(3) for m in range(3)}


def gewichte_linear(k, mit_rest=False):
    """Je Mikrofon ein komplexes Gewicht je Mode fuer alle drei Schlagorte (kleinste Quadrate, relativ)."""
    ph = form(k, XH)
    G = np.zeros((J, 3), dtype=complex)
    for m in range(3):
        Y = np.concatenate([Ymik[(h, m)] - FEST[(h, m)] for h in range(3)])
        gw = np.concatenate([1.0 / np.maximum(np.abs(Ymik[(h, m)]), 1e-9) for h in range(3)])
        A = np.concatenate([Bp * ph[:, h][None, :] for h in range(3)])
        if mit_rest:
            rest = np.zeros((3 * n, 6), dtype=complex)
            for h in range(3):
                rest[h * n:(h + 1) * n, 2 * h] = 1.0
                rest[h * n:(h + 1) * n, 2 * h + 1] = 1e4 / w ** 2
            A = np.concatenate([A, rest], axis=1)
        G[:, m] = np.linalg.lstsq(A * gw[:, None], Y * gw, rcond=None)[0][:J]
    return G


def steg_modell(k, rest):
    return (form(k, xh_p) * form(k, xs_p)).T @ Bm.T + rest @ rest_b.T


def steg_rest(k):
    """Massen- und Steifeterm je Stegkurve, linear."""
    kern = (form(k, xh_p) * form(k, xs_p)).T @ Bm.T
    rest = np.zeros((len(mg.PAARE), 2))
    for q in range(len(mg.PAARE)):
        A = rest_b * gew_steg[q][:, None]
        r = (Ysteg[q] - kern[q]) * gew_steg[q]
        rest[q] = np.linalg.lstsq(np.concatenate([A.real, A.imag]), np.concatenate([r.real, r.imag]), rcond=None)[0]
    return rest


def formen_schritt(k, G, schritt=2):
    """a, b, c je Mode (und der Rest der Stegkurven) gegen Steg- und Schallkurven, G fest. Jede `schritt`-te Frequenz."""
    i = slice(None, None, schritt)
    nn = len(hz[i])
    nk = len(mg.PAARE)
    ph_h = np.stack([xh_p ** e * SKALA[e] for e in range(3)])
    ph_s = np.stack([xs_p ** e * SKALA[e] for e in range(3)])
    pm_h = np.stack([XH ** e * SKALA[e] for e in range(3)])
    B1, B2, rb = Bm[i], Bp[i], rest_b[i]
    gs = gew_steg[:, i]
    paare_m = [(h, m) for h in range(3) for m in range(3)]
    gm = np.array([np.sqrt(GEWICHT) / np.maximum(np.abs(Ymik[q][i]), 1e-9) for q in paare_m])
    ym = np.array([(Ymik[q] - FEST[q])[i] for q in paare_m])
    ys = Ysteg[:, i]

    def teile(p):
        return p[:3 * J].reshape(3, J), p[3 * J:].reshape(nk, 2)

    def res(p):
        kk, rest = teile(p)
        fh, fs_ = kk.T @ ph_h, kk.T @ ph_s
        ds = ((fh * fs_).T @ B1.T + rest @ rb.T - ys) * gs
        fm = kk.T @ pm_h                                                   # (J, 3 Schlagorte)
        dm = np.array([(B2 @ (fm[:, h] * G[:, m]) - ym[q]) * gm[q] for q, (h, m) in enumerate(paare_m)])
        return np.concatenate([ds.real.ravel(), ds.imag.ravel(), dm.real.ravel(), dm.imag.ravel()])

    def jac(p):
        kk, _ = teile(p)
        fh, fs_ = kk.T @ ph_h, kk.T @ ph_s
        Js = np.zeros((2, nk, nn, 3 * J + 2 * nk))
        for q in range(nk):
            gb = gs[q][:, None] * B1
            for e in range(3):
                block = gb * (ph_h[e, q] * fs_[:, q] + ph_s[e, q] * fh[:, q])[None, :]
                Js[0, q, :, e * J:(e + 1) * J] = block.real
                Js[1, q, :, e * J:(e + 1) * J] = block.imag
            r = gs[q][:, None] * rb
            Js[0, q, :, 3 * J + 2 * q:3 * J + 2 * q + 2] = r.real
            Js[1, q, :, 3 * J + 2 * q:3 * J + 2 * q + 2] = r.imag
        Jm = np.zeros((2, len(paare_m), nn, 3 * J + 2 * nk))
        for q, (h, m) in enumerate(paare_m):
            gb = gm[q][:, None] * B2 * G[:, m][None, :]
            for e in range(3):
                Jm[0, q, :, e * J:(e + 1) * J] = (gb * pm_h[e, h]).real
                Jm[1, q, :, e * J:(e + 1) * J] = (gb * pm_h[e, h]).imag
        return np.concatenate([Js.reshape(2 * nk * nn, -1), Jm.reshape(2 * len(paare_m) * nn, -1)])

    p0 = np.concatenate([(k / SKALA[:, None]).ravel(), steg_rest(k).ravel()])
    los = least_squares(res, p0, jac=jac, method="trf", max_nfev=25, x_scale="jac")
    kk, _ = teile(los.x)
    return kk * SKALA[:, None], float(np.sqrt(np.mean(los.fun ** 2)))


# ---------------------------------------------------------------- Zielgroessen
def terzfehler(G2, k):
    """Fehler des Terzpegels je Bereich (Effektivwert ueber Baender und Schlagorte), left und right; mit dem statistischen Teil."""
    mitten = np.array([m for m in (63 * 2 ** (i / 3) for i in range(20)) if m * 2 ** (1 / 6) <= mg.BIS * 1.02])
    ph = form(k, XH)
    aus = []
    for m in range(2):
        dd = []
        for h in range(3):
            syn = Bp @ (ph[:, h] * G2[:, m]) + FEST[(h, m)]
            dd.append([10 * np.log10(np.mean(np.abs(syn[b]) ** 2) / np.mean(np.abs(Ymik[(h, m)][b]) ** 2))
                       for b in ((hz >= mi / 2 ** (1 / 6)) & (hz < mi * 2 ** (1 / 6)) for mi in mitten)])
        dd = np.array(dd)
        aus.append([float(np.sqrt(np.mean(dd[:, (mitten >= lo) & (mitten < hi)] ** 2))) for _, lo, hi in TERZ])
    return aus


def halbtonfehler(G2, k):
    ph = form(k, XH)
    aus = []
    for m in range(2):
        dd = np.zeros((len(TASTEN), 3))
        for h in range(3):
            syn = Bp @ (ph[:, h] * G2[:, m]) + FEST[(h, m)]
            for i, t in enumerate(TASTEN):
                f0 = 440.0 * 2 ** ((t - 69) / 12)
                b = (hz >= f0 * 2 ** (-1 / 24)) & (hz < f0 * 2 ** (1 / 24))
                dd[i, h] = 10 * np.log10(np.mean(np.abs(syn[b]) ** 2) / np.mean(np.abs(Ymik[(h, m)][b]) ** 2))
        i, h = np.unravel_index(np.argmax(np.abs(dd)), dd.shape)
        aus.append((float(np.sqrt(np.mean(dd ** 2))), float(dd[i, h]), TASTEN[i], ORTE[h], float(np.sqrt(np.mean(dd[24:, 2] ** 2)))))
    return aus


def stegfehler(k):
    fits = list(steg_modell(k, steg_rest(k)))
    return [float(np.sqrt(np.mean(np.concatenate([20 * np.log10(np.abs(yf[(hz >= lo) & (hz < hi)]) / np.abs(y[(hz >= lo) & (hz < hi)]))
                                                  for y, yf in zip(Ysteg, fits)]) ** 2))) for _, lo, hi in BEREICHE]


def masse6(k):
    m6 = (f > 85) & (f < 240)
    return float(1 / np.sum(form(k, [XS[0]])[m6, 0] ** 2))


def toene(k, G2, fest_zupfort):
    kv = dict(kp)
    kv["steg"] = np.concatenate([form(k, XS), kp["steg"][J:]])
    kv["G"] = np.concatenate([G2[:, :2], kp["G"][J:]])
    wf = ps.frequenzen()
    kurven, l = {}, []
    for taste in TASTEN:
        si = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= taste)
        bund = taste - ps.GITARRE[si]["taste"]
        if si not in kurven:
            kurven[si] = ps.koerper_kurven(kv, kv["tasten"].index(ps.GITARRE[si]["taste"]), wf)
        laenge = ps.MENSUR * 2 ** (-bund / 12)
        xp = ps.ZUPFORT if fest_zupfort else min(ps.ZUPFORT, 0.5 * laenge)
        l.append(ps.momentary_max(ps.zeit(ps.ton(ps.GITARRE[si], laenge, xp, 0.002, *kurven[si], wf))))
    l = np.array(l)
    d3 = np.array([abs(l[i] - l[j]) for i in range(len(l)) for j in range(i + 1, min(i + 5, len(l)))])
    paar = max(((abs(l[i] - l[j]), TASTEN[i], TASTEN[j]) for i in range(len(l)) for j in range(i + 1, min(i + 5, len(l)))))
    return l, (float(l.max() - l.min()), float(d3.max()), float(np.median(d3)), paar[1], paar[2])


def grenze_wie_heute(k):
    a, b, c = k
    grenze, G, _ = mg.abstrahlung(hz_alle, H, f, s, a, b, c, sel)
    return grenze, np.array(G + [np.zeros(J, dtype=complex)]).T


# ---------------------------------------------------------------- die Wege
print(f"g34, {J} Moden; Gewicht der Schallkurven {GEWICHT}", flush=True)
k_neu = koeff_datei.copy()
for runde in range(8):
    G_r = gewichte_linear(k_neu)
    k_neu, rms = formen_schritt(k_neu, G_r)
    print(f"  Runde {runde + 1}: Effektivwert des relativen Fehlers {rms:.4f}  ({time.time() - start:.0f} s)", flush=True)

G_datei = np.concatenate([kp["G"][:J], np.zeros((J, 1), dtype=complex)], axis=1)
gr1, G1 = grenze_wie_heute(k_neu)
wege = [("0 Datei", koeff_datei, G_datei),
        (f"1 Formen neu, Grenze {gr1:.0f} Hz", k_neu, G1),
        ("2 Formen neu, je Mode", k_neu, gewichte_linear(k_neu)),
        ("3 Formen der Datei, je Mode", koeff_datei, gewichte_linear(koeff_datei))]

print("\nFormen an den Schlagorten (auf 1 normiert), Moden 500-1100 Hz: Datei -> neu")
for j in np.where((f > 500) & (f < 1100))[0]:
    a_, b_ = form(koeff_datei, XH)[j], form(k_neu, XH)[j]
    print(f"  {f[j]:7.1f} Hz  " + " ".join(f"{v:+.2f}" for v in a_ / np.linalg.norm(a_)) + "  ->  " + " ".join(f"{v:+.2f}" for v in b_ / np.linalg.norm(b_))
          + f"   phi an Saite 1: {form(koeff_datei, [XS[5]])[j, 0]:+.2f} -> {form(k_neu, [XS[5]])[j, 0]:+.2f}")

print("\nWeg                          | Steg dB " + " / ".join(n_ for n_, _, _ in BEREICHE) + " | Masse S6 kg | Terzpegel left " + " / ".join(n_ for n_, _, _ in TERZ) + " | right")
for name, k, G2 in wege:
    sf, (tl, tr) = stegfehler(k), terzfehler(G2, k)
    print(f"{name:28s} | " + " / ".join(f"{v:.2f}" for v in sf) + f" | {masse6(k):.3f} | " + " / ".join(f"{v:.2f}" for v in tl) + " | " + " / ".join(f"{v:.2f}" for v in tr))

print("\nHalbtonbaender der Tasten 40-83, drei Schlagorte: Effektivwert, groesster (Taste, Schlag), Tasten 64-83 am Schlag treble")
for name, k, G2 in wege:
    hl, hr = halbtonfehler(G2, k)
    print(f"{name:28s} | left {hl[0]:.1f}, {hl[1]:+.1f} ({hl[2]}, {hl[3]}), {hl[4]:.1f} | right {hr[0]:.1f}, {hr[1]:+.1f} ({hr[2]}, {hr[3]}), {hr[4]:.1f}")

if MIT_TOENEN:
    print("\nPruefstand, Lautheit je Taste 40-83 (Momentary-Maximum): Spanne, groesster Sprung (Paar), mittlerer Sprung - Paare bis 4 Halbtoene")
    reihen = {}
    for name, k, G2 in wege:
        for fest in (False, True):
            l, (sp, gr, mi, t1, t2) = toene(k, G2, fest)
            reihen[(name, fest)] = l
            print(f"{name:28s} | Zupfort {'16 cm fest          ' if fest else 'min(16 cm, halbe Saite)'} | {sp:5.1f} LU, {gr:5.1f} LU ({t1}/{t2}), {mi:4.1f} LU   ({time.time() - start:.0f} s)", flush=True)
    print("\nLautheit je Taste 70-83 (LUFS), Zupfort 16 cm fest / mit Klemme:")
    print("Taste | " + " | ".join(name[:22] for name, _, _ in wege))
    for i, t in enumerate(TASTEN):
        if t >= 70:
            print(f"{t:4d}  | " + " | ".join(f"{reihen[(name, True)][i]:6.1f} / {reihen[(name, False)][i]:6.1f}      " for name, _, _ in wege))

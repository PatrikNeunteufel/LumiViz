"""Modalfit an einer Gitarre des Archivs Mores 2021 (KQ3 a; Weg nach Ducceschi, Russo & Webb 2026 Par. 3.4,
mit zwei Abweichungen: Pole aus ESPRIT ueber alle sechs Stegkanaele statt Spitzensuche und -3-dB-Breite; das volle
Signal mit Exponentialfenster statt 62,5 ms).

Aufruf: python -I mores_fit.py <mat> <gitarre 1..65> <ziel-stamm> [--bild]

Kanaele (Mores, Method Par. 4): Schlag bass / center / treble je 1 s; 1 Hammer, 2 Aufnehmer treble, 3 Aufnehmer bass,
4 Mikro upper east, 5 Mikro treble, 6 Mikro bass. Sechs Steg-FRF: (Schlag, Aufnehmer).

Modell der Mobilitaet: Y_k(w) = sum_j R_kj i w / (w_j^2 - w^2 + 2 i s_j w) + 1/(i w M_k) + i w / K_k, R reell.
Fuer eine Normalmode gilt R = phi(Schlag) phi(Aufnehmer): R_bt^2 = R_bb R_tt — das prueft `rang1`.
"""
import json
import sys

import numpy as np
import scipy.io as sio
import scipy.signal as sig

FS = 48000
N_VOLL = (10000 / 92.90) * 4.4482
A_VOLL = (10000 / 10.64) * 9.80665
P_VOLL = 10000 / 50.0
ORTE = ("bass", "center", "treble")
FENSTER_S = 3.0          # 1/s, Exponentialfenster; wird von der Daempfung abgezogen
VON, BIS = 60.0, 5000.0
PAARE = [("bass", "bass"), ("bass", "treble"), ("center", "bass"), ("center", "treble"), ("treble", "bass"),
         ("treble", "treble")]


def lade(pfad, g):
    d = sio.loadmat(pfad)["qualified_selected_impulses"][g - 1]
    return {ort: d[o * FS:(o + 1) * FS, :] for o, ort in enumerate(ORTE)}


def frfs(daten):
    """Accelerance A/F und Druck P/F je Schlagort, mit Exponentialfenster ab dem Schlag. -> hz, dict"""
    aus = {}
    hz = np.fft.rfftfreq(FS, 1 / FS)
    for ort, x in daten.items():
        ip = int(np.argmax(np.abs(x[:, 0])))
        start = max(0, ip - 48)
        t = (np.arange(FS) - start) / FS
        w = np.where(t >= 0, np.exp(-FENSTER_S * t), 0.0)
        # die Kraft nur um den Schlag (5 ms), sonst traegt ihr Rauschen in den Nenner
        fw = np.zeros(FS)
        fw[start:start + 240] = sig.windows.tukey(240, 0.3)
        F = np.fft.rfft(x[:, 0] * fw * N_VOLL)
        for kanal, name in ((1, "treble"), (2, "bass")):
            aus[(ort, name)] = np.fft.rfft(x[:, kanal] * w * A_VOLL) / F
        for kanal, name in ((3, "east"), (4, "mtreble"), (5, "mbass")):
            aus[(ort, name)] = np.fft.rfft(x[:, kanal] * w * P_VOLL) / F
    return hz, aus


def esprit(x, k):
    """Mehrkanal-ESPRIT: x (kanaele, n) komplex -> k Pole z."""
    kan, n = x.shape
    l = n // 2
    h = np.concatenate([np.lib.stride_tricks.sliding_window_view(x[c], l).T for c in range(kan)], axis=1)  # (l, kan*(n-l+1))
    u, s, _ = np.linalg.svd(h, full_matrices=False)
    k = min(k, u.shape[1] - 1)
    us = u[:, :k]
    phi = np.linalg.lstsq(us[:-1], us[1:], rcond=None)[0]
    return np.linalg.eigvals(phi), s


def pole_im_band(hz, acc, fl, fh, ordnung):
    """Pole (Hz, 1/s) im Band [fl, fh] aus den Impulsantworten der sechs Accelerance-FRF."""
    fc, bw = 0.5 * (fl + fh), fh - fl
    rate = max(4.0 * bw, 200.0)
    dez = max(1, int(FS // rate))
    rate = FS / dez
    t = np.arange(FS) / FS
    kan = []
    for y in acc:
        h = np.fft.irfft(y, FS)
        hb = h * np.exp(-2j * np.pi * fc * t)
        tp = sig.firwin(8 * dez + 1, 0.8 / dez) if dez > 1 else np.array([1.0])
        hb = sig.lfilter(tp, 1.0, hb)[4 * dez::dez] if dez > 1 else hb
        n = int(min(len(hb), 0.35 * rate))
        kan.append(hb[2:n + 2])
    z, _ = esprit(np.array(kan), ordnung)
    f = fc + np.angle(z) * rate / (2 * np.pi)
    s = -np.log(np.abs(z)) * rate - FENSTER_S
    gut = (f >= fl) & (f < fh) & (s > 0.5) & (s < 0.25 * 2 * np.pi * f)
    return f[gut], s[gut]


def basis(hz, f, s):
    w = 2 * np.pi * hz[:, None]
    wj = 2 * np.pi * f[None, :]
    return 1j * w / (wj ** 2 - w ** 2 + 2j * s[None, :] * w)


def residuen(hz, y, f, s, sel):
    """Reelle Residuen plus Massen- und Steifeterm, kleinste Quadrate mit Gewicht 1/|Y| (Fehler in dB)."""
    w = 2 * np.pi * hz[sel]
    b = np.concatenate([basis(hz[sel], f, s), (1 / (1j * w))[:, None], (1j * w)[:, None] * 1e-8], axis=1)
    gew = 1.0 / np.maximum(np.abs(y[sel]), 1e-6)
    a = np.concatenate([(b * gew[:, None]).real, (b * gew[:, None]).imag])
    r = np.concatenate([(y[sel] * gew).real, (y[sel] * gew).imag])
    x = np.linalg.lstsq(a, r, rcond=None)[0]
    return x, b @ x


def fit(pfad, g):
    daten = lade(pfad, g)
    hz, H = frfs(daten)
    w = 2 * np.pi * np.maximum(hz, 1e-9)
    acc = [H[p] for p in PAARE]
    mob = [a / (1j * w) for a in acc]
    kanten = [VON * 2 ** (k / 3) for k in range(int(np.ceil(3 * np.log2(BIS / VON))) + 1)]
    kanten[-1] = BIS
    fs_, ss_ = [], []
    for fl, fh in zip(kanten, kanten[1:]):
        rand = 0.08 * (fh - fl)
        f, s = pole_im_band(hz, acc, fl - rand, fh + rand, ordnung=int(np.clip(6 + 0.02 * (fh - fl) * 3, 8, 40)))
        innen = (f >= fl) & (f < fh)
        fs_.extend(f[innen]); ss_.extend(s[innen])
    f, s = np.array(fs_), np.array(ss_)
    ordn = np.argsort(f)
    f, s = f[ordn], s[ordn]
    sel = (hz >= VON) & (hz <= BIS)
    # Auslese: eine Mode bleibt, wenn sie in einem der sechs Kanaele bis 25 dB an den Pegel ihres Bands heranreicht
    for _ in range(3):
        R = np.array([residuen(hz, y, f, s, sel)[0][:len(f)] for y in mob])
        spitze = np.abs(R) / (2 * s[None, :])                      # |Y| der Mode an ihrer Resonanz
        band = np.array([[np.sqrt(np.mean(np.abs(y[(hz > fj / 1.12) & (hz < fj * 1.12)]) ** 2)) for fj in f] for y in mob])
        bleibt = (spitze / band).max(axis=0) > 10 ** (-25 / 20)
        # doppelte Pole (naeher als ein Viertel der Breite) zusammenlegen: den staerkeren behalten
        for i in range(len(f) - 1):
            if bleibt[i] and bleibt[i + 1] and (f[i + 1] - f[i]) < 0.25 * (s[i] + s[i + 1]) / (2 * np.pi):
                schwach = i if spitze[:, i].max() < spitze[:, i + 1].max() else i + 1
                bleibt[schwach] = False
        if bleibt.all():
            break
        f, s = f[bleibt], s[bleibt]
    fitk, Rk = [], []
    for y in mob:
        x, yf = residuen(hz, y, f, s, sel)
        Rk.append(x); fitk.append(yf)
    R = np.array(Rk)[:, :len(f)]
    return dict(hz=hz, sel=sel, mob=mob, fit=fitk, f=f, s=s, R=R, rest=np.array(Rk)[:, len(f):], H=H)


# Lage laengs der Stegeinlage, m von der Mitte, Bass negativ (Mores, Method Abb. 3: Schlag zwischen den Saiten 6/5,
# 4/3, 2/1; Aufnehmer an den Enden der Einlage knapp ausserhalb der Saiten 6 und 1). Saitenabstand 11,6 mm — Annahme
# (uebliches Mass), Aufnehmer +-36 mm am Bild geschaetzt (+-3 mm).
X_SCHLAG = {"bass": -0.0232, "center": 0.0, "treble": 0.0232}
X_AUFN = {"bass": -0.036, "treble": 0.036}
X_SAITEN = [-0.029, -0.0174, -0.0058, 0.0058, 0.0174, 0.029]   # Saite 6 (E2) ... 1 (E4)


def stegfit(e):
    """Der Steg als starrer Stab: phi_j(x) = a_j + b_j x; R(Schlag, Aufnehmer) = phi(x_h) phi(x_s) >= Normalmoden.
    Pole fest, a und b je Mode und die Restterme je Kanal mit Gauss-Newton (scipy least_squares)."""
    from scipy.optimize import least_squares
    hz, sel, f, s = e["hz"], e["sel"], e["f"], e["s"]
    J = len(f)
    B = basis(hz[sel], f, s)                                   # (n, J)
    w = 2 * np.pi * hz[sel]
    rest_b = np.stack([1 / (1j * w), 1j * w * 1e-8], axis=1)   # (n, 2)
    xh = np.array([X_SCHLAG[p[0]] for p in PAARE])
    xs = np.array([X_AUFN[p[1]] for p in PAARE])
    Y = np.array([y[sel] for y in e["mob"]])                   # (6, n)
    gew = 1.0 / np.maximum(np.abs(Y), 1e-6)

    # Start: aus den freien Residuen je Mode die Gerade, die sie am besten trifft (beide Vorzeichen probiert)
    R = e["R"]
    ib, it = PAARE.index(("bass", "bass")), PAARE.index(("treble", "treble"))
    a0, b0 = np.zeros(J), np.zeros(J)
    for j in range(J):
        best = None
        pb, pt = np.sqrt(abs(R[ib, j])), np.sqrt(abs(R[it, j]))
        for vz in (1.0, -1.0):
            b = (vz * pt - pb) / (X_AUFN["treble"] - X_AUFN["bass"])
            a = pb - b * X_AUFN["bass"]
            fehl = np.sum(((a + b * xh) * (a + b * xs) - R[:, j]) ** 2)
            if best is None or fehl < best[0]:
                best = (fehl, a, b)
        a0[j], b0[j] = best[1], best[2]
    # Form laengs des Stegs: phi(x) = a + b x (+ c x^2 mit --biegung); Spalten so skaliert, dass sie bei 3 cm gleich wiegen
    biegung = "--biegung" in sys.argv
    grad = 3 if biegung else 2
    skala = np.array([1.0, 30.0, 900.0])[:grad]
    ph = np.stack([xh ** k * skala[k] for k in range(grad)])   # (grad, 6)
    ps = np.stack([xs ** k * skala[k] for k in range(grad)])
    p0 = np.concatenate([a0, b0 / skala[1]] + ([np.zeros(J)] if biegung else []) + [e["rest"].ravel()])
    nk, n = len(PAARE), B.shape[0]

    def formen(p):
        k = p[:grad * J].reshape(grad, J)
        return k.T @ ph, k.T @ ps                               # (J, 6) am Schlag, am Aufnehmer

    def modell(p):
        fh, fs_ = formen(p)
        rest = p[grad * J:].reshape(nk, 2)
        Rk = (fh * fs_).T                                       # (6, J)
        return Rk @ B.T + rest @ rest_b.T, Rk

    def res(p):
        ym, _ = modell(p)
        d = (ym - Y) * gew
        return np.concatenate([d.real.ravel(), d.imag.ravel()])

    def jac(p):
        fh, fs_ = formen(p)
        Jm = np.zeros((2, nk, n, grad * J + 2 * nk))
        for q in range(nk):
            gb = gew[q][:, None] * B                            # (n, J)
            for k in range(grad):
                dR = ph[k, q] * fs_[:, q] + ps[k, q] * fh[:, q]  # (J,)
                block = gb * dR[None, :]
                Jm[0, q, :, k * J:(k + 1) * J] = block.real
                Jm[1, q, :, k * J:(k + 1) * J] = block.imag
            rb = gew[q][:, None] * rest_b
            Jm[0, q, :, grad * J + 2 * q:grad * J + 2 * q + 2] = rb.real
            Jm[1, q, :, grad * J + 2 * q:grad * J + 2 * q + 2] = rb.imag
        return Jm.reshape(2 * nk * n, -1)

    los = least_squares(res, p0, jac=jac, method="trf", max_nfev=200, x_scale="jac")
    ym, Rk = modell(los.x)
    k = los.x[:grad * J].reshape(grad, J) * skala[:, None]
    return dict(a=k[0], b=k[1], c=(k[2] if biegung else np.zeros(J)), fit=list(ym), R=Rk)


def fehler_je_bereich(e, fitk):
    hz, sel = e["hz"], e["sel"]
    aus = {}
    for name, a, b in (("60-400", 60, 400), ("400-1000", 400, 1000), ("1000-2500", 1000, 2500), ("2500-5000", 2500, 5000)):
        m = (hz[sel] >= a) & (hz[sel] < b)
        d = [20 * np.log10(np.abs(yf[m]) / np.abs(y[sel][m])) for y, yf in zip(e["mob"], fitk)]
        aus[name] = round(float(np.sqrt(np.mean(np.concatenate(d) ** 2))), 2)
    return aus


def bericht(e, g):
    hz, sel, f, s, R = e["hz"], e["sel"], e["f"], e["s"], e["R"]
    ib, it, ibt, itb = PAARE.index(("bass", "bass")), PAARE.index(("treble", "treble")), PAARE.index(("bass", "treble")), \
        PAARE.index(("treble", "bass"))
    aus = {"gitarre": g, "moden": int(len(f)), "moden_ueber_1k": int((f > 1000).sum())}
    # Fehler je Bereich: Effektivwert von 20 lg |fit/mess| ueber die sechs Kanaele
    for name, a, b in (("60-400", 60, 400), ("400-1000", 400, 1000), ("1000-2500", 1000, 2500), ("2500-5000", 2500, 5000)):
        m = (hz[sel] >= a) & (hz[sel] < b)
        d = [20 * np.log10(np.abs(yf[m]) / np.abs(y[sel][m])) for y, yf in zip(e["mob"], e["fit"])]
        aus[f"fehler_dB_{name}"] = round(float(np.sqrt(np.mean(np.concatenate(d) ** 2))), 2)
    # Rang 1: Antriebspunkte positiv, R_bt = R_tb, R_bt^2 = R_bb R_tt
    pos = (R[ib] > 0) & (R[it] > 0)
    aus["antrieb_positiv"] = round(float(pos.mean()), 3)
    kreuz = 0.5 * (R[ibt] + R[itb])
    rezi = np.abs(R[ibt] - R[itb]) / (np.abs(R[ibt]) + np.abs(R[itb]) + 1e-30)
    aus["reziprok_median"] = round(float(np.median(rezi)), 3)
    with np.errstate(invalid="ignore", divide="ignore"):
        verh = np.abs(kreuz[pos]) / np.sqrt(R[ib][pos] * R[it][pos])
    aus["rang1_median"] = round(float(np.median(verh)), 3)
    aus["rang1_in_3dB"] = round(float(((verh > 0.708) & (verh < 1.413)).mean()), 3)
    for name, a, b in (("unter_1k", 0, 1000), ("ueber_1k", 1000, 6000)):
        m = (f[pos] >= a) & (f[pos] < b)
        aus[f"rang1_in_3dB_{name}"] = round(float(((verh[m] > 0.708) & (verh[m] < 1.413)).mean()), 3) if m.any() else None
    q = np.pi * f / s
    aus["Q_median"] = round(float(np.median(q)), 1)
    aus["Q_median_ueber_1k"] = round(float(np.median(q[f > 1000])), 1) if (f > 1000).any() else None
    tief = [(round(float(fj), 1), round(float(qj), 1), round(float(1 / rb), 3) if rb > 0 else None,
             round(float(1 / rt), 3) if rt > 0 else None) for fj, qj, rb, rt in zip(f, q, R[ib], R[it]) if fj < 450]
    aus["tief_Hz_Q_Mbass_kg_Mtreble_kg"] = tief
    return aus


def bild(e, g, datei):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    hz, sel = e["hz"], e["sel"]
    fig, achsen = plt.subplots(3, 2, figsize=(15, 9), sharex=True, sharey=True)
    for ax, p, y, yf in zip(achsen.ravel(), PAARE, e["mob"], e["fit"]):
        ax.loglog(hz[sel], np.abs(y[sel]), "k-", lw=0.7, label="measured")
        ax.loglog(hz[sel], np.abs(yf), "r-", lw=0.7, label="fit")
        ax.set_title(f"g{g:02d} hit {p[0]} / sensor {p[1]}", fontsize=9)
        ax.set_ylim(5e-4, 1.5)
        ax.grid(True, which="both", alpha=0.3)
    for fj in e["f"]:
        achsen[0, 0].axvline(fj, color="b", lw=0.2, alpha=0.5)
    achsen[0, 0].legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(datei, dpi=80)


if __name__ == "__main__":
    pfad, g, stamm = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    e = fit(pfad, g)
    b = bericht(e, g)
    if "--steg" in sys.argv:
        st = stegfit(e)
        b["steg_fehler_dB"] = fehler_je_bereich(e, st["fit"])
        f, s = e["f"], e["s"]
        mitte = st["a"]
        # wirksame Masse an der Stegmitte (1/a^2) der staerksten Moden unter 450 Hz
        b["steg_tief_Hz_Q_Mmitte_kg_kipp"] = [
            (round(float(fj), 1), round(float(np.pi * fj / sj), 1), round(float(1 / aj ** 2), 3) if abs(aj) > 1e-6 else None,
             round(float(bj * 0.029 / aj), 2) if abs(aj) > 1e-6 else None)
            for fj, sj, aj, bj in zip(f, s, st["a"], st["b"]) if fj < 450 and (aj ** 2 / (2 * sj)) > 0.004]
        e_st = dict(e)
        e_st["fit"] = st["fit"]
        if "--bild" in sys.argv:
            bild(e_st, g, f"{stamm}_steg_g{g:02d}.png")
        np.savez(f"{stamm}_steg_g{g:02d}.npz", f=f, s=s, a=st["a"], b=st["b"], c=st["c"])
    print(json.dumps(b, ensure_ascii=False))
    if "--bild" in sys.argv:
        bild(e, g, f"{stamm}_g{g:02d}.png")

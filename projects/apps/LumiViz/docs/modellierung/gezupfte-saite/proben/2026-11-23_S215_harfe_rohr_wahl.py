"""S215 (KQ59 a) - Probe: was der Bau des Rohrs festlegen muss, gegen Bell Kap. 4 gerechnet.

Drei Wahlpunkte vor dem Bau in `InstrumentBody`:
  Querschnitt laengs   S ~ w^2 (Halbkegel, Bell Abb. 2.7: Tiefe = halbe Breite), S ~ w (gleiche Tiefe), S konstant
  Zahl der Laengsmoden 5, 10, 20, 40, alle bis 5 kHz
  Guete der Luft       eine Guete fuer alle Luftenergie (Probe S214) gegen getrennt: Pfropfen der Oeffnungen / Luft
                       laengs im Rohr (je Mode ihr Anteil an der Bewegungsenergie)
Je Variante wird die Muendung so gestellt, dass die Helmholtz-Mode mit allen Oeffnungen bei 190 Hz liegt (KQ60 a).

Bell 1988 (am Seitenbild, Abb. 4.8 PDF 160 f., Abb. 4.11-4.15 PDF 165-169; Kasten A, starre Waende im Sand):
  nur Oeffnung 6 offen  75 / 168 / 283 / 414 / 545 / 680 Hz,  Q 10 / 10 / 12 / 23 / 30 / 36
  alle offen           190 / 292 / 423 / 530 / 620 / 720 Hz,  Q  6 /  6 /  7 / 10 / 15 / 30

  rohr <vak> <gebaut>        Rohr allein: alle Varianten gegen Bell (Lagen, Gueten)
  gekoppelt <vak> <gebaut>   die Varianten an der Platte: Paar, Lagen, Abstrahlung (wie `2026-11-23_S214_harfe_rohr.py raster`)

Aufruf vom Projekt-Root mit Python 3.12 (numpy).
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("rohr", HIER / "2026-11-23_S214_harfe_rohr.py")
ro = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(ro)
kp = ro.kp

BELL_6 = np.array([75, 168, 283, 414, 545, 680.0])
BELL_6_Q = np.array([10, 10, 12, 23, 30, 36.0])
BELL_ALLE = np.array([190, 292, 423, 530, 620, 720.0])
BELL_ALLE_Q = np.array([6, 6, 7, 10, 15, 30.0])

_breite = ro.breite
GESETZE = {"w^2": 1.0, "w^1": 0.5, "konst": 0.0}


def setze_gesetz(p):
    # rohrmoden quadriert die Breite: w^(2p) ergibt S ~ w^(2p)
    ro.breite = lambda plan, y: _breite(plan, y) ** p if p > 0 else np.ones_like(np.asarray(y, dtype=float))


# Laenge der Oeffnungen laengs des Kastens (Bell Abb. 2.8, A in cm; Nr. 6 oval im Bodenblock)
LAENGE_OEFFNUNG = np.array([16.5, 18.0, 18.4, 18.4, 17.2, 11.5]) / 100.0
_rohrmoden = ro.rohrmoden
SCHLITZ = {"an": False}


def setze_schlitz(an, gp):
    """an: der Pfropfen sieht den Druck gemittelt ueber seine Laenge (psi am Ort ersetzt durch das Mittel)."""
    SCHLITZ["an"] = an
    if not an:
        ro.rohrmoden = _rohrmoden
        return
    oy = [o["y"] for o in gp["cavity"]["openings"]]

    def gemittelt(plan, V, c, zahl):
        L, y, wn, psi = _rohrmoden(plan, V, c, zahl)
        psi = psi.copy()
        dy = y[1] - y[0]
        for yo, lo in zip(oy, LAENGE_OEFFNUNG):
            z = int(np.clip(yo / L * ro.ZELLEN, 0, ro.ZELLEN - 1))
            sel = np.abs(y - yo) <= lo / 2 + 1e-12
            if sel.sum() < 1:
                continue
            psi[:, z] = psi[:, sel].mean(axis=1)
        return L, y, wn, psi

    ro.rohrmoden = gemittelt


def zahl_bis(gp, cav, c, hz):
    _, _, wn, _ = ro.rohrmoden(gp, cav["volume"], c, 120)
    return int(np.sum(wn[1:] / (2 * np.pi) < hz))


def muendung_fuer(v, B, cav, gp, moden, ziel=190.0):
    lo, hi = 0.5, 4.0
    for _ in range(50):
        mu = (lo + hi) / 2
        f = ro.kopple(v, B, cav, gp, mu, cav["q"], moden, starr=True)["f"]
        if f[f > 20][0] > ziel:
            lo = mu
        else:
            hi = mu
    return (lo + hi) / 2


def anteile(e):
    """Je Mode der Anteil der Bewegungsenergie in den Pfropfen und in der Luft laengs."""
    ma = np.array([kp_rho * L / A for *_, A, L in e["lo"]])
    pf = (e["u"] ** 2 * ma[None, :]).sum(axis=1)
    return pf, e["luft"] - pf


kp_rho = 1.2


def rohr_allein(v, B, cav, gp, moden, mu):
    aus = {}
    for name, offen in (("6", [5]), ("alle", None)):
        e = ro.kopple(v, B, cav, gp, mu, cav["q"], moden, starr=True, offen=offen)
        gut = e["f"] > 20
        f = e["f"][gut][:6]
        pf, rr = anteile(e)
        aus[name] = (f, pf[gut][:6], rr[gut][:6])
    return aus


def passe_gueten(aus):
    """eta = pf / Qp + rr / Qr, kleinste Quadrate in log Q gegen Bell (12 Werte); dazu eine Guete fuer alles."""
    pf = np.concatenate([aus["6"][1], aus["alle"][1]])
    rr = np.concatenate([aus["6"][2], aus["alle"][2]])
    q = np.concatenate([BELL_6_Q, BELL_ALLE_Q])
    best = None
    for qp in np.geomspace(2, 100, 300):
        for qr in np.geomspace(2, 1000, 400):
            qm = 1 / (pf / qp + rr / qr)
            s = np.sum(np.log(qm / q) ** 2)
            if best is None or s < best[0]:
                best = (s, qp, qr, qm)
    eins = None
    for qe in np.geomspace(2, 100, 600):
        qm = 1 / ((pf + rr) / qe)
        s = np.sum(np.log(qm / q) ** 2)
        if eins is None or s < eins[0]:
            eins = (s, qe, qm)
    return best, eins


def profile(v, cav, gp, mu, moden, offen=None, bis=800.0):
    """Rohr allein, starr: je Mode Frequenz, Anteil der Pfropfen und der Druck laengs p(y) (wie `kopple`, n = 0)."""
    rho, c = v.rho, v.c
    V = cav["volume"]
    alpha = rho * c * c / V
    L, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
    lo = [o for i, o in enumerate(cav["openings"]) if offen is None or i in offen]
    Ma = np.array([rho * (o["lipLength"] + mu * np.sqrt(o["area"] / np.pi)) / o["area"] for o in lo])
    zl = np.clip((np.array([o["y"] for o in lo]) / L * ro.ZELLEN).astype(int), 0, ro.ZELLEN - 1)
    no = len(lo)
    N = no + moden
    K = np.zeros((N, N))
    C = np.zeros((moden + 1, N))
    for j in range(moden + 1):
        C[j, :no] = psi[j, zl]
        if j >= 1:
            C[j, no + j - 1] = -1.0
        K += alpha * np.outer(C[j], C[j])
    M = np.concatenate([Ma, alpha / wn[1:] ** 2])
    lam, vec = np.linalg.eigh(K / np.sqrt(np.outer(M, M)))
    aus = []
    for m in range(N):
        f = np.sqrt(max(lam[m], 0.0)) / (2 * np.pi)
        if f < 20 or f > bis:
            continue
        x = vec[:, m] / np.sqrt(M)                         # Koordinaten u, s
        Pn = -alpha * (C @ x)                              # Druck je Rohrmode
        p = Pn @ psi
        pf = float((vec[:no, m] ** 2).sum())
        db = 20 * np.log10(np.abs(p) / np.abs(p).max() + 1e-6)
        knoten = int(np.sum(np.diff(np.sign(p)) != 0))
        aus.append((f, pf, knoten, y, db))
    return aus


def proz(a, b):
    return " ".join(f"{x:+.0f}" for x in 100 * (a / b - 1))


def main():
    global kp_rho
    was = sys.argv[1]
    v = kp.Vakuum(sys.argv[2])
    kp_rho = v.rho
    gp = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbodyplan")
    gk = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbody")
    cav = gp["cavity"]
    L = max(p[1] for p in gp["plate"]["outline"])
    B = ro.volumen_laengs(pathlib.Path(sys.argv[2]) / "formen.txt", L)
    if was == "rohr":
        print("Bell nur 6 offen:", " ".join(f"{x:.0f}" for x in BELL_6), " Q", " ".join(f"{x:.0f}" for x in BELL_6_Q))
        print("Bell alle offen :", " ".join(f"{x:.0f}" for x in BELL_ALLE), " Q", " ".join(f"{x:.0f}" for x in BELL_ALLE_Q))
        for schlitz, gesetz, p in [(False, g, p) for g, p in GESETZE.items()] + [(True, "w^2", 1.0)]:
            setze_gesetz(p)
            setze_schlitz(schlitz, gp)
            if schlitz:
                gesetz = "w^2 S"
            alle = zahl_bis(gp, cav, v.c, 5000.0)
            for moden in ((5, 10, 20, 40, alle) if not schlitz else (10, 40)):
                mu = muendung_fuer(v, B, cav, gp, moden)
                aus = rohr_allein(v, B, cav, gp, moden, mu)
                f6, fa = aus["6"][0], aus["alle"][0]
                best, eins = passe_gueten(aus)
                fehl = np.concatenate([f6 / BELL_6 - 1, fa / BELL_ALLE - 1])
                print(f"{gesetz:5s} Moden {moden:3d}  mu {mu:.2f} | nur 6: {proz(f6, BELL_6)} % | alle: {proz(fa, BELL_ALLE)} %"
                      f" | Lage rms {100 * np.sqrt(np.mean(fehl ** 2)):.1f} %, groesste {100 * np.max(abs(fehl)):.0f} %")
                print(f"      Pfropfen-Anteil nur 6: {' '.join(f'{x:.2f}' for x in aus['6'][1])} | alle: {' '.join(f'{x:.2f}' for x in aus['alle'][1])}")
                print(f"      getrennt Qp {best[1]:.1f} Qr {best[2]:.0f}: Q {' '.join(f'{x:.0f}' for x in best[3])}  (log-rms {np.sqrt(best[0] / 12):.2f})")
                print(f"      eine Guete {eins[1]:.1f}:       Q {' '.join(f'{x:.0f}' for x in eins[2])}  (log-rms {np.sqrt(eins[0] / 12):.2f})")
        return
    if was == "volumen":
        # Bell Tab. 4.1: (2,7 +- 0,2) x 10^-2 m^3 - wie haengen die Moden mit allen Oeffnungen am Volumen?
        setze_gesetz(1.0)
        v0 = cav["volume"]
        for vol in (0.023, 0.025, 0.027, 0.029):
            cav["volume"] = vol
            mu = muendung_fuer(v, B, cav, gp, 40)
            aus = rohr_allein(v, B, cav, gp, 40, mu)
            print(f"V {vol * 1e3:.0f} l  mu {mu:.2f} | nur 6: {proz(aus['6'][0], BELL_6)} % | alle: {proz(aus['alle'][0], BELL_ALLE)} %")
        cav["volume"] = v0
        return
    if was == "profile":
        setze_gesetz(1.0)
        mu = muendung_fuer(v, B, cav, gp, 40)
        for name, offen in (("nur 6 offen", [5]), ("alle offen", None)):
            print(f"{name} (w^2, 40 Moden, mu {mu:.2f}); Druck laengs in dB unter dem groessten, y = 0,1 … 1,2 m:")
            for f, pf, kn, y, db in profile(v, cav, gp, mu, 40, offen):
                stellen = [int(np.argmin(abs(y - yy))) for yy in np.arange(0.1, 1.25, 0.1)]
                print(f"  {f:6.1f} Hz  Pfropfen {pf:.2f}  Vorzeichenwechsel {kn}  |" + " ".join(f"{db[s]:4.0f}" for s in stellen))
        return
    if was == "gekoppelt":
        q0 = cav["q"]
        faelle = [("w^2", 1.0, 10, ("eine", q0)), ("w^2", 1.0, None, ("eine", q0)),
                  ("w^2", 1.0, None, ("eine", 13.4)), ("w^2", 1.0, None, ("getrennt", (4.4, 28.0))),
                  ("w^2", 1.0, 10, ("getrennt", (4.4, 28.0))),
                  ("w^1", 0.5, None, ("getrennt", (4.1, 30.0))), ("konst", 0.0, None, ("getrennt", (3.7, 38.0)))]
        for gesetz, p, moden, (art, q) in faelle:
            setze_gesetz(p)
            if moden is None:
                moden = zahl_bis(gp, cav, v.c, 5000.0)
            mu = muendung_fuer(v, B, cav, gp, moden)
            e = ro.kopple(v, B, cav, gp, mu, q0, moden)
            pf, rr = anteile(e)
            rest = e["eta"] - e["luft"] / q0                 # Platte und Federn
            e["eta"] = rest + (e["luft"] / q if art == "eine" else pf / q[0] + rr / q[1])
            e = ro.nur_hoerbar(e)
            name = f"{gesetz} Moden {moden} mu {mu:.2f} " + (f"Q {q:g}" if art == "eine" else f"Qp {q[0]:g} Qr {q[1]:g}")
            kp.zeile(name, e, kp.koerper(v, gk, e), gp)
            ro.zeige_paar(e, bis=480.0)
        return


if __name__ == "__main__":
    main()

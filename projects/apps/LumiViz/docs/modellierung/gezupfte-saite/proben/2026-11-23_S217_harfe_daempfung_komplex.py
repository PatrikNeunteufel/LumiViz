"""S217 - Probe: die Guete der Plattenmoden der Harfe mit komplexen Eigenwerten statt nach Energieanteilen.

`Luftraum.hpp` (`mitLuftraum`) rechnet die gekoppelten Moden ohne Daempfung und gibt jeder Mode den Verlust nach ihren
Energieanteilen: eta = Platte x eta_Platte + Pfropfen / Q_Oeffnung + Luft laengs / Q_laengs. Das gilt, solange die
Daempfung die Formen nicht aendert. Hier sind die Partner sehr ungleich gedaempft: Pfropfen der Oeffnungen Q 4,4,
Plattenmoden Q 40-50. Liegt eine Plattenmode auf einer Luftmode, mischt die Rechnung ohne Daempfung beide zu gleichen
Teilen (Q rund 2 x Q der Luft); mit Daempfung mischen sie nur, wenn die Kopplung groesser ist als die halbe Differenz
der Halbwertsbreiten - sonst bleibt die Plattenmode eine Plattenmode mit wenig zusaetzlichem Verlust.

Probe: dasselbe System (Plattenmoden im Vakuum, Rohr, Pfropfen mit Massenmatrix; starre Waende), die Daempfung als
Matrix mit denselben drei Verlusten (bei der Frequenz der betrachteten Mode ausgewertet), komplexe Eigenwerte.
Ohne Mischung durch die Daempfung geben beide Wege dieselbe Guete - der Unterschied ist allein die Wirkung der
ungleichen Daempfung.

  python <probe> <vak> <gebaut> [q_oeffnung=4.4] [q_laengs=28]

Aufruf vom Projekt-Root mit Python 3.12 (numpy, scipy).
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.linalg

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("gegen", HIER / "2026-11-23_S216_harfe_oeffnungen_gegenseitig.py")
ge = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(ge)
ro, kp = ge.ro, ge.kp

BELL_OHNE = {1: "11 / 16", 2: "39", 3: "45", 4: "43", 5: "31"}
BELL_MIT = {1: "-", 2: "32", 3: "27", 4: "28", 5: "19"}


def baue(v, cav, gp, L, Bvol, moden=38):
    rho, c = v.rho, v.c
    V = cav["volume"]
    alpha = rho * c * c / V
    _, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
    oe = cav["openings"]
    n, no = len(v.f), len(oe)
    Mo = ge.massen(rho, oe, 1.0, 1.0)[0]
    Pl = np.zeros((moden + 1, no))
    for i, o in enumerate(oe):
        sel = np.abs(y - o["y"]) <= ge.LAENGE[i] / 2
        Pl[:, i] = psi[:, sel].mean(axis=1)
    N = n + no + moden
    K = np.zeros((N, N)); M = np.zeros((N, N))
    K[np.arange(n), np.arange(n)] = (2 * np.pi * v.f) ** 2
    Bn = Bvol @ psi.T
    for j in range(moden + 1):
        cv = np.zeros(N); cv[:n] = Bn[:, j]; cv[n:n + no] = Pl[j]
        if j >= 1:
            cv[n + no + j - 1] = -1.0
        K += alpha * np.outer(cv, cv)
    M[np.arange(n), np.arange(n)] = 1.0
    M[n:n + no, n:n + no] = Mo
    a0 = n + no
    M[np.arange(a0, N), np.arange(a0, N)] = alpha / wn[1:] ** 2
    return K, M, n, no


Q_INNEN_F = np.array([75.0, 168, 283, 414, 545, 680])      # Bell Abb. 4.8, 4.11-4.15: nur Oeffnung 6 offen
Q_INNEN = np.array([10.0, 10, 12, 23, 30, 36])
BELL_ALLE_F = np.array([190.0, 292, 423, 530, 620, 720])
BELL_ALLE_Q = np.array([6.0, 6, 7, 10, 15, 30])


def strahlung(rho, c, oe, w0, wand):
    """Strahlungswiderstand der Oeffnungen als Matrix (Volumenfluss -> Druck): rho c k^2 / (2 pi) sinc(k d), Wand;
    halb so gross im freien Raum. Auf der Diagonale derselbe Ausdruck (k a << 1)."""
    k = w0 / c
    y = np.array([o["y"] for o in oe]); z = np.array([o["z"] for o in oe])
    d = np.hypot(y[:, None] - y[None, :], z[:, None] - z[None, :])
    return (1.0 if wand else 0.5) * rho * c * k * k / (2 * np.pi) * np.sinc(k * d / np.pi)


def daempfung(v, cav, M, n, no, w0, wand, innen=1.0):
    """Platte: eta je Form; Luft: Abstrahlung der Oeffnungen (Matrix) und der innere Verlust nach 'nur Oeffnung 6 offen'."""
    N = M.shape[0]
    a0 = n + no
    qi = np.interp(w0 / (2 * np.pi), Q_INNEN_F, Q_INNEN) / innen
    C = np.zeros((N, N))
    C[np.arange(n), np.arange(n)] = v.eta * w0
    C[n:a0, n:a0] = strahlung(v.rho, v.c, cav["openings"], w0, wand) + w0 * M[n:a0, n:a0] / qi
    C[np.arange(a0, N), np.arange(a0, N)] = w0 * np.diag(M)[a0:] / qi
    return C


def komplex(K, M, C, ix):
    Kx, Mx, Cx = K[np.ix_(ix, ix)], M[np.ix_(ix, ix)], C[np.ix_(ix, ix)]
    Mi = np.linalg.inv(Mx)
    nn = len(ix)
    ew, ev = np.linalg.eig(np.block([[np.zeros((nn, nn)), np.eye(nn)], [-Mi @ Kx, -Mi @ Cx]]))
    return ew, ev[:nn]


def strahlung_main(v, cav, gp, K, M, n, no, wand, innen):
    N = K.shape[0]
    a0 = n + no
    print(f"\nAbstrahlung der Oeffnungen als Matrix ({'Wand' if wand else 'freier Raum'}), innerer Verlust nach 'nur 6 offen' x {innen:g}")
    # Luft allein, alle offen
    luft = np.arange(n, N)
    text = []
    for fb, qb in zip(BELL_ALLE_F, BELL_ALLE_Q):
        lam, _ = scipy.linalg.eigh(K[np.ix_(luft, luft)], M[np.ix_(luft, luft)])
        fu = np.sqrt(np.maximum(lam, 0)) / (2 * np.pi)
        f0 = fu[np.argmin(np.abs(fu - fb * np.where(fb > 250, 0.92, 1.0)))]
        C = daempfung(v, cav, M, n, no, 2 * np.pi * f0, wand, innen)
        ew, _ = komplex(K, M, C, luft)
        s = np.argmin(np.abs(ew.imag - 2 * np.pi * f0) + 1e9 * (ew.imag <= 0))
        text.append(f"{ew[s].imag / (2 * np.pi):.0f} Hz Q {ew[s].imag / (-2 * ew[s].real):.1f} (Bell {fb:.0f} Hz Q {qb:.0f})")
    print("  Luft allein, alle offen: " + "  ".join(text))
    lam, vec = scipy.linalg.eigh(K, M)
    f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    for k in range(5):
        kand = [m for m in range(N) if 100 < f[m] < 800 and vec[k, m] ** 2 > 0.15]
        m = max(kand, key=lambda j: vec[k, j] ** 2)
        w0 = 2 * np.pi * f[m]
        C = daempfung(v, cav, M, n, no, w0, wand, innen)
        ew, ev = komplex(K, M, C, np.arange(N))
        zeilen = []
        for s in np.where((ew.imag > 0.7 * w0) & (ew.imag < 1.3 * w0))[0]:
            z = ev[:, s]
            ges = (np.abs(z[:n]) ** 2).sum() + np.abs(z[n:a0].conj() @ M[n:a0, n:a0] @ z[n:a0]) + (np.abs(z[a0:]) ** 2 * np.diag(M)[a0:]).sum()
            form = np.abs(z[k]) ** 2 / ges
            if form > 0.10:
                zeilen.append((ew[s].imag / (2 * np.pi), ew[s].imag / (-2 * ew[s].real), (np.abs(z[:n]) ** 2).sum() / ges, form))
        print(f"  S(1,{k + 1}) | Bell ohne Saiten {BELL_OHNE[k + 1]}, besaitet {BELL_MIT[k + 1]} | "
              + "  ".join(f"{fz:6.1f} Hz Q {qz:5.1f} (Platte {ant:.2f}, Form {form:.2f})" for fz, qz, ant, form in sorted(zeilen)))


def main():
    if sys.argv[1] == "strahlung":
        v = kp.Vakuum(sys.argv[2])
        gp = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbodyplan")
        L = max(p[1] for p in gp["plate"]["outline"])
        Bvol = ro.volumen_laengs(pathlib.Path(sys.argv[2]) / "formen.txt", L)
        K, M, n, no = baue(v, gp["cavity"], gp, L, Bvol)
        print(f"Platte im Vakuum {' / '.join(f'{x:.0f}' for x in v.f[:5])} Hz, Q der Platte allein {' / '.join(f'{1 / x:.0f}' for x in v.eta[:5])}")
        for wand in (True, False):
            for innen in (1.0, 0.5):
                strahlung_main(v, gp["cavity"], gp, K, M, n, no, wand, innen)
        return
    arg = dict(a.split("=") for a in sys.argv[3:])
    qo, ql = float(arg.get("q_oeffnung", 4.4)), float(arg.get("q_laengs", 28.0))
    v = kp.Vakuum(sys.argv[1])
    gp = kp.lade(pathlib.Path(sys.argv[2]) / "concert_harp.lvinstbodyplan")
    cav = gp["cavity"]
    L = max(p[1] for p in gp["plate"]["outline"])
    Bvol = ro.volumen_laengs(pathlib.Path(sys.argv[1]) / "formen.txt", L)
    K, M, n, no = baue(v, cav, gp, L, Bvol)
    N = K.shape[0]
    a0 = n + no
    lam, vec = scipy.linalg.eigh(K, M)
    f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    Mi = np.linalg.inv(M)
    print(f"Platte im Vakuum {' / '.join(f'{x:.0f}' for x in v.f[:5])} Hz, Q der Platte allein {' / '.join(f'{1 / x:.0f}' for x in v.eta[:5])};"
          f" Q der Oeffnungen {qo}, laengs {ql}")
    print("je Plattenform k: die ungedaempften Moden mit Anteil > 0,15 dieser Form -> f, Anteile Platte / Pfropfen / laengs, Q nach Anteilen;"
          " darunter die gedaempften Moden mit dieser Form")
    for k in range(5):
        kand = [m for m in range(N) if 100 < f[m] < 800 and vec[k, m] ** 2 > 0.15]
        for m in kand:
            x = vec[:, m]
            pl = float((x[:n] ** 2).sum())
            pf = float(x[n:a0] @ M[n:a0, n:a0] @ x[n:a0])
            rr = float((x[a0:] ** 2 * np.diag(M)[a0:]).sum())
            eta_pl = float((x[:n] ** 2 * v.eta).sum())
            q_anteil = 1.0 / (eta_pl + pf / qo + rr / ql)
            w0 = 2 * np.pi * f[m]
            C = np.zeros((N, N))
            C[np.arange(n), np.arange(n)] = v.eta * w0
            C[n:a0, n:a0] = w0 * M[n:a0, n:a0] / qo
            C[np.arange(a0, N), np.arange(a0, N)] = w0 * np.diag(M)[a0:] / ql
            A = np.block([[np.zeros((N, N)), np.eye(N)], [-Mi @ K, -Mi @ C]])
            ew, ev = np.linalg.eig(A)
            sel = np.where((ew.imag > 0.7 * w0) & (ew.imag < 1.3 * w0))[0]
            zeilen = []
            for s in sel:
                z = ev[:N, s]
                en = np.abs(z) ** 2 * np.diag(M)
                en[n:a0] = np.abs(z[n:a0].conj() @ M[n:a0, n:a0] @ z[n:a0]) / no
                ant = (np.abs(z[:n]) ** 2).sum() / en.sum()
                form = np.abs(z[k]) ** 2 / en.sum()
                if form > 0.10:
                    zeilen.append((ew[s].imag / (2 * np.pi), ew[s].imag / (-2 * ew[s].real), ant, form))
            text = "  ".join(f"{fz:6.1f} Hz Q {qz:5.1f} (Platte {ant:.2f}, Form {form:.2f})" for fz, qz, ant, form in sorted(zeilen))
            print(f"  S(1,{k + 1}): ohne Daempfung {f[m]:6.1f} Hz, Platte {pl:.2f} Pfropfen {pf:.2f} laengs {rr:.2f}, Q nach Anteilen {q_anteil:5.1f}"
                  f" | Bell ohne Saiten {BELL_OHNE[k + 1]}, besaitet {BELL_MIT[k + 1]}\n        gedaempft: {text}")


if __name__ == "__main__":
    main()

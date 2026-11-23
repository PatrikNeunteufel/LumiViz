"""S216 (KQ67 a) - Probe: die Luftpfropfen der sechs Oeffnungen belasten sich gegenseitig.

Befund S215 (Harfenbericht 23.2): das Rohr trifft Bell Kap. 4 mit nur Oeffnung 6 offen auf 3 %, mit allen Oeffnungen
liegen A2-A5 um 6-13 % zu tief, und die Helmholtz-Mode der Oeffnung 6 allein liegt 9 % zu tief. `Luftraum.hpp` gibt
jedem Pfropfen seine eigene Masse rho (Lippe + mu r) / A; mu ist so gestellt, dass die Helmholtz-Mode mit allen
Oeffnungen bei 190 Hz liegt.

Vermutung: die Oeffnungen 1-5 sind 16,5-18,4 cm lang und liegen im Abstand von 22 cm - fast ein durchgehender Schlitz.
Die Luft vor der Schale gehoert zu allen gemeinsam: die mitbewegte Masse aussen ist eine Matrix

    M_ij = rho kappa / (2 pi A_i A_j)  Int Int dS dS' / |r - r'|          (Kolben in einer Wand, k d << 1)

(i = j: die uebliche Muendungskorrektur aussen, fuer den Kreis 8 r / 3 pi). Schwingen alle gleichphasig (Helmholtz),
kommt die Nachbarschaft dazu; wechselt das Vorzeichen von Oeffnung zu Oeffnung (A2-A5), geht sie ab. Ein mu, das auf
die Helmholtz-Mode gestellt ist, ist dann fuer die Oeffnung 6 allein und fuer A2-A5 zu gross - das Vorzeichen des
Befunds. kappa = 1: unendliche Wand; 0,5: freier Raum (die Schale ist 0,1-0,4 m breit, Wellenlaenge 0,5-1,8 m).

Innen: die Wechselwirkung laengs traegt das Rohr selbst; als Nahfeld bleibt je Oeffnung gamma mal ihre eigene
Korrektur (dieselbe Flaechenformel).

  rohr <vak> <gebaut>     Rohr allein, starre Waende, gegen Bells zwoelf Luftmoden: heutiges Modell und die Matrix
  raster <vak> <gebaut>   kappa x gamma

Oeffnungen als Rechtecke Laenge A (Bell Abb. 2.8) mal Flaeche / A (Tab. 4.1), auf der Mittellinie der Rueckseite in
einer Ebene (die Kruemmung der Schale und die Lage der Oeffnung 6 im Bodenblock sind nicht nachgebildet).
Aufruf vom Projekt-Root mit Python 3.12 (numpy, scipy).
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.linalg

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("wahl", HIER / "2026-11-23_S215_harfe_rohr_wahl.py")
wa = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(wa)
ro, kp = wa.ro, wa.kp

LAENGE = wa.LAENGE_OEFFNUNG          # Reihenfolge des Bauplans: 1-5, dann 6
ZELLE = 0.005
EIGEN_QUADRAT = 2.9732095            # Int Int dS dS' / r ueber das Einheitsquadrat


def zellen(oeffnungen):
    """Je Oeffnung die Mittelpunkte ihrer Zellen (x quer, y laengs) und die Zellflaeche."""
    aus = []
    for o, la in zip(oeffnungen, LAENGE):
        br = o["area"] / la
        ny, nx = max(1, round(la / ZELLE)), max(1, round(br / ZELLE))
        hy, hx = la / ny, br / nx
        yy = o["y"] - la / 2 + (np.arange(ny) + 0.5) * hy
        xx = -br / 2 + (np.arange(nx) + 0.5) * hx
        X, Y = np.meshgrid(xx, yy)
        aus.append((np.column_stack([X.ravel(), Y.ravel()]), hx * hy, hx, hy))
    return aus


def integral_matrix(oeffnungen):
    """G_ij = Int Int dS dS' / |r - r'| ueber die Oeffnungen i und j."""
    z = zellen(oeffnungen)
    n = len(z)
    G = np.zeros((n, n))
    for i in range(n):
        pi, ai, hx, hy = z[i]
        for j in range(i, n):
            pj, aj, _, _ = z[j]
            d = np.linalg.norm(pi[:, None, :] - pj[None, :, :], axis=2)
            if i == j:
                h = np.sqrt(hx * hy)
                np.fill_diagonal(d, np.inf)
                G[i, i] = (1.0 / d).sum() * ai * ai + len(pi) * EIGEN_QUADRAT * h ** 3
            else:
                G[i, j] = G[j, i] = (1.0 / d).sum() * ai * aj
    return G


def massen(rho, oeffnungen, kappa, gamma, selbst=1.0):
    """Massenmatrix der Pfropfen: Lippe + innen gamma x eigene Korrektur + aussen selbst x eigene + kappa x Nachbarn."""
    G = integral_matrix(oeffnungen)
    A = np.array([o["area"] for o in oeffnungen])
    aussen = rho * G / (2 * np.pi * np.outer(A, A))
    M = kappa * aussen
    eigen = np.diag(aussen).copy()
    M[np.diag_indices_from(M)] = rho * np.array([o["lipLength"] for o in oeffnungen]) / A + (gamma + selbst) * eigen
    return M, eigen * A / rho


def rohr(v, cav, gp, moden, Mo, offen, schlitz=True):
    """Rohr allein, starr, mit der Massenmatrix Mo der offenen Oeffnungen. Zurueck f, Pfropfen-Anteil, Vorzeichenwechsel."""
    rho, c = v.rho, v.c
    V = cav["volume"]
    alpha = rho * c * c / V
    L, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
    lo = [cav["openings"][i] for i in offen]
    no = len(lo)
    P = np.zeros((moden + 1, no))
    for i, k in enumerate(offen):
        o = cav["openings"][k]
        sel = np.abs(y - o["y"]) <= LAENGE[k] / 2 + 1e-12 if schlitz else np.zeros(len(y), bool)
        if sel.sum() < 1:
            sel = np.zeros(len(y), bool)
            sel[int(np.clip(o["y"] / L * ro.ZELLEN, 0, ro.ZELLEN - 1))] = True
        P[:, i] = psi[:, sel].mean(axis=1)
    N = no + moden
    K = np.zeros((N, N))
    C = np.zeros((moden + 1, N))
    for j in range(moden + 1):
        C[j, :no] = P[j]
        if j >= 1:
            C[j, no + j - 1] = -1.0
        K += alpha * np.outer(C[j], C[j])
    M = np.zeros((N, N))
    M[:no, :no] = Mo[np.ix_(offen, offen)]
    M[np.arange(no, N), np.arange(no, N)] = alpha / wn[1:] ** 2
    lam, vec = scipy.linalg.eigh(K, M)
    aus = []
    for m in range(N):
        f = np.sqrt(max(lam[m], 0.0)) / (2 * np.pi)
        if f < 20:
            continue
        x = vec[:, m]
        p = (-alpha * (C @ x)) @ psi
        pf = float(x[:no] @ M[:no, :no] @ x[:no])
        aus.append((f, pf, int(np.sum(np.diff(np.sign(p)) != 0)), x[:no].copy()))
    return aus[:6]


def zeige(name, v, cav, gp, Mo, moden=40, schlitz=True, lang=False):
    s6 = rohr(v, cav, gp, moden, Mo, [5], schlitz)
    al = rohr(v, cav, gp, moden, Mo, list(range(6)), schlitz)
    f6 = np.array([m[0] for m in s6])
    fa = np.array([m[0] for m in al])
    fehl = np.concatenate([f6 / wa.BELL_6 - 1, fa / wa.BELL_ALLE - 1])
    print(f"{name:34s} | nur 6: {wa.proz(f6, wa.BELL_6)} % | alle: {wa.proz(fa, wa.BELL_ALLE)} %"
          f" | rms {100 * np.sqrt(np.mean(fehl ** 2)):.1f} %, groesste {100 * np.max(abs(fehl)):.0f} %")
    if lang:
        for m in al:
            print(f"      {m[0]:6.1f} Hz  Pfropfen {m[1]:.2f}  Wechsel {m[2]}  u 1-5, 6: " + " ".join(f"{u / np.abs(m[3]).max():+.2f}" for u in m[3]))
    return np.sqrt(np.mean(fehl ** 2))


def main():
    was = sys.argv[1]
    v = kp.Vakuum(sys.argv[2])
    gp = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbodyplan")
    cav = gp["cavity"]
    oe = cav["openings"]
    rho = v.rho
    A = np.array([o["area"] for o in oe])
    r = np.sqrt(A / np.pi)
    print("Bell nur 6 offen:", " ".join(f"{x:.0f}" for x in wa.BELL_6), "| alle offen:", " ".join(f"{x:.0f}" for x in wa.BELL_ALLE))
    M1, eigen = massen(rho, oe, 1.0, 1.0)
    print("eigene Korrektur aussen / r (Kreis in der Wand 0,85):", " ".join(f"{x:.2f}" for x in eigen / r))
    print("Nachbarn (kappa 1) je Oeffnung, gleichphasig, in Teilen der eigenen Masse (gamma 1):",
          " ".join(f"{x:.2f}" for x in (M1.sum(axis=1) - np.diag(M1)) / np.diag(M1)))
    if was == "rohr":
        for mu in (1.83, 1.46):
            Mh = np.diag(rho * (np.array([o["lipLength"] for o in oe]) + mu * r) / A)
            zeige(f"heute, jeder fuer sich, mu {mu:.2f}", v, cav, gp, Mh, schlitz=False, lang=mu == 1.83)
        for kappa, gamma, selbst in ((1.0, 1.0, 1.0), (0.5, 1.0, 0.72), (0.7, 1.0, 0.85), (0.0, 1.0, 1.0)):
            Mo, _ = massen(rho, oe, kappa, gamma, selbst)
            zeige(f"Matrix kappa {kappa:.1f} innen {gamma:.1f} aussen {selbst:.2f}", v, cav, gp, Mo, lang=True)
            zeige("  dasselbe, Druck am Ort", v, cav, gp, Mo, schlitz=False)
        return
    if was == "raster":
        for selbst in (0.72, 1.0):
            for kappa in (0.0, 0.3, 0.5, 0.7, 1.0, 1.3):
                for gamma in (0.4, 0.7, 1.0, 1.3):
                    Mo, _ = massen(rho, oe, kappa, gamma, selbst)
                    zeige(f"kappa {kappa:.1f} innen {gamma:.1f} aussen {selbst:.2f}", v, cav, gp, Mo)
        return


if __name__ == "__main__":
    main()

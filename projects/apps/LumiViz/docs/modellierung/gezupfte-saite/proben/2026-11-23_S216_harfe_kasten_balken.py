"""S216 (KQ67 a) - Probe: der Kasten der Harfe als Balken, auf dem der Boden sitzt.

`InstrumentBody` haelt den Rand des Bodens auf einem starren, ruhenden Rahmen. Bell 1988 beschreibt die tiefen Moden
des Kastens als Ganzkoerperschwingungen (S. 107 f.: bei S(1,1) bewegen sich Bass- und Diskantende, bei S(1,1)+
"the front and back of the box move in unison", bei S(1,2) "a bending motion of the box as well as the dipolar
mode"; S. 119, 121: "whole body resonances", "not just modes of a held board"), und das Paar mit Luft haengt an der
Halterung: Kasten G lose 153 / 200 Hz, in der Harfe 170 / 216 Hz, besaitet 187 / 207 Hz (Kap. 7). Aus Bells Paar am
losen Kasten A (176 / 201 Hz, Helmholtz 190 Hz) folgt eine Versteifung der ersten Bodenmode durch die Luft von
25 Hz; das Modell hat 54 Hz (ein Druck) und 36,5 Hz (Rohr).

Probe: der Kasten (Schale, Leisten, Bloecke) als Euler-Bernoulli-Balken laengs y mit Masse mu(y) ~ Breite und
Biegesteife EI(y) ~ Breite^3; jeder Querschnitt bewegt sich als Ganzes mit w_b(y). Der Boden schwingt mit seinen Moden
im Vakuum relativ dazu:

    T = 1/2 sum q_k'^2 + sum_k q_k' Int P_k(y) w_b' dy + 1/2 Int (mu + m_Boden) w_b'^2 dy      P_k = Int sigma phi_k dx
    U = 1/2 sum w_k^2 q_k^2 + 1/2 Int EI w_b''^2 dy + Luft(q)                                  (nur q aendert das Volumen)

Die Luft wie in `2026-11-23_S216_harfe_oeffnungen_gegenseitig.py` (Rohr, Massenmatrix der Oeffnungen). Halterung:
frei (loser Kasten) oder beide Enden gehalten (in der Harfe). Stellgroessen: Masse des Kastens ohne Boden und die
erste Biegefrequenz des leeren, freien Kastens. Die Flaechenmasse des Bodens ist geschaetzt (Dicke 10 -> 3 mm,
Leisten auf der Mittellinie) und so gestellt, dass die Formen im Mittel die modale Masse 1 haben.

  raster <vak> <gebaut>

Aufruf vom Projekt-Root mit Python 3.12 (numpy, scipy). Die Platte im Vakuum unter `<vak>` traegt die Saiten als
Federn (S213) - die Probe zeigt Richtung und Groesse, keine Lagen fuer einen Bauplan.
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

NE = 131                      # Balkenelemente (1 cm), je Element zwei Zellen des Rohrs
KOPPELN = 10                  # so viele Plattenmoden tragen ihren Impuls auf den Balken


def lies_formen(pfad, zahl):
    z = pathlib.Path(pfad).read_text(encoding="utf-8").split("\n")
    n = int(z[1].split()[1])
    xy = np.array([[float(v) for v in z[2 + i].split()] for i in range(n)])
    t = int(z[2 + n].split()[1])
    tri = np.array([[int(v) for v in z[3 + n + i].split()] for i in range(t)])
    w = np.array([np.array(z[5 + n + t + 2 * i].split(), dtype=float) for i in range(zahl)])
    return xy, tri, w


def bodenmasse(xy, tri, w, L):
    """Flaechenmasse je Dreieck (geschaetzt, zwei Faktoren an die modale Masse 1 gestellt) -> P_k je Zelle, Masse je Zelle."""
    a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
    fl = 0.5 * np.abs((b[:, 0] - a[:, 0]) * (c[:, 1] - a[:, 1]) - (c[:, 0] - a[:, 0]) * (b[:, 1] - a[:, 1]))
    xm = (a[:, 0] + b[:, 0] + c[:, 0]) / 3
    ym = (a[:, 1] + b[:, 1] + c[:, 1]) / 3
    platte = 450.0 * (0.010 + (0.003 - 0.010) * ym / L)                    # kg/m^2
    leiste = np.where(np.abs(xm) < 0.03, (0.77 + 0.10) / 0.06, 0.0)        # Zug- und Deckleiste, Buche
    wt2 = (w[:, tri] ** 2).mean(axis=2)                                    # [k, t]
    A = np.column_stack([(wt2 * (fl * platte)).sum(axis=1), (wt2 * (fl * leiste)).sum(axis=1)])
    s, *_ = np.linalg.lstsq(A, np.ones(len(w)), rcond=None)
    s = np.maximum(s, 0.0)
    sigma = s[0] * platte + s[1] * leiste
    print(f"Bodenmasse: Faktor Platte {s[0]:.2f}, Leisten {s[1]:.2f}; modale Massen der ersten Formen:",
          " ".join(f"{x:.2f}" for x in (A @ s)[:8]), f"| Boden gesamt {(fl * sigma).sum():.2f} kg")
    zelle = np.clip((ym / L * ro.ZELLEN).astype(int), 0, ro.ZELLEN - 1)
    wt = w[:, tri].mean(axis=2)
    P = np.zeros((len(w), ro.ZELLEN))
    for k in range(len(w)):
        np.add.at(P[k], zelle, fl * sigma * wt[k])
    m = np.zeros(ro.ZELLEN)
    np.add.at(m, zelle, fl * sigma)
    # Die geschaetzte Flaechenmasse ist nicht die der Rechnung: je Form auf die modale Masse 1 stellen und nur die
    # ersten KOPPELN Formen an den Balken haengen - sonst ist die Massenmatrix (Boden auf dem Balken minus C^T C)
    # nicht mehr positiv, und es entstehen Scheinmoden.
    P /= np.sqrt(A @ s)[:, None]
    P[KOPPELN:] = 0.0
    return P, m


def hermite(xi, le):
    return np.array([1 - 3 * xi ** 2 + 2 * xi ** 3, le * (xi - 2 * xi ** 2 + xi ** 3), 3 * xi ** 2 - 2 * xi ** 3, le * (-xi ** 2 + xi ** 3)])


def balken(L, breite, masse, fb, P, mboden):
    """Balkenmatrizen (frei), dazu die Kopplung C[k, dof] und die Masse des Bodens auf dem Balken."""
    le = L / NE
    nd = 2 * (NE + 1)
    K = np.zeros((nd, nd)); M = np.zeros((nd, nd)); Mb = np.zeros((nd, nd)); C = np.zeros((P.shape[0], nd))
    ym = (np.arange(NE) + 0.5) * le
    w = breite(ym)
    mu = w / (w.sum() * le) * masse * 0.67                                 # Schale und Leisten ~ Breite
    ei = w ** 3
    ke = np.array([[12, 6 * le, -12, 6 * le], [6 * le, 4 * le ** 2, -6 * le, 2 * le ** 2],
                   [-12, -6 * le, 12, -6 * le], [6 * le, 2 * le ** 2, -6 * le, 4 * le ** 2]]) / le ** 3
    me = np.array([[156, 22 * le, 54, -13 * le], [22 * le, 4 * le ** 2, 13 * le, -3 * le ** 2],
                   [54, 13 * le, 156, -22 * le], [-13 * le, -3 * le ** 2, -22 * le, 4 * le ** 2]]) * le / 420
    je = ro.ZELLEN // NE
    dz = L / ro.ZELLEN
    for e in range(NE):
        ix = np.arange(2 * e, 2 * e + 4)
        K[np.ix_(ix, ix)] += ei[e] * ke
        M[np.ix_(ix, ix)] += mu[e] * me
        for j in range(je):
            z = e * je + j
            N = hermite((j + 0.5) / je, le)
            C[:, ix] += np.outer(P[:, z], N)
            Mb[np.ix_(ix, ix)] += mboden[z] * np.outer(N, N)
    M[0, 0] += 0.25 * masse                                                # Bodenblock
    M[nd - 2, nd - 2] += 0.08 * masse                                      # oberer Block
    lam = scipy.linalg.eigh(K, M, eigvals_only=True)
    f1 = np.sqrt(lam[lam > 1e-6 * lam[-1]][0]) / (2 * np.pi)              # erste Biegemode, EI-Skala 1
    return K * (fb / f1) ** 2, M, Mb, C


def system(v, cav, gp, Pk, mboden, masse, fb, halt, moden=38, starr=False):
    rho, c = v.rho, v.c
    V = cav["volume"]
    alpha = rho * c * c / V
    L = max(p[1] for p in gp["plate"]["outline"])
    _, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
    oe = cav["openings"]
    Mo = ge.massen(rho, oe, 1.0, 1.0)[0]
    n, no = len(v.f), len(oe)
    Pl = np.zeros((moden + 1, no))
    for i, o in enumerate(oe):
        sel = np.abs(y - o["y"]) <= ge.LAENGE[i] / 2
        Pl[:, i] = psi[:, sel].mean(axis=1)
    Kb, Mbal, Mbod, C = balken(L, lambda yy: ro.breite(gp, yy), masse, fb, Pk[:n], mboden)
    nd = Kb.shape[0]
    frei = np.ones(nd, bool)
    if starr:
        frei[:] = False
    elif halt == "gehalten":
        frei[0] = frei[nd - 2] = False
    ib = np.where(frei)[0]
    nb = len(ib)
    N = n + no + moden + nb
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
    M[np.arange(a0, a0 + moden), np.arange(a0, a0 + moden)] = alpha / wn[1:] ** 2
    b0 = a0 + moden
    K[b0:, b0:] = Kb[np.ix_(ib, ib)]
    M[b0:, b0:] = (Mbal + Mbod)[np.ix_(ib, ib)]
    M[:n, b0:] = C[:, ib]
    M[b0:, :n] = C[:, ib].T
    kleinster = np.linalg.eigvalsh(M[b0:, b0:] - M[b0:, :n] @ M[:n, b0:]).min() if nb else 1.0
    if kleinster <= 0.0:
        print(f"   Massenmatrix nicht positiv ({kleinster:.2e})")
    lam, vec = scipy.linalg.eigh(K, M)
    f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    aus = []
    for m in np.where((f > 100) & (f < 480))[0]:
        x = vec[:, m]
        luft = float(x[n:n + no] @ Mo @ x[n:n + no] + (x[a0:b0] ** 2 * np.diag(M)[a0:b0]).sum())
        kasten = float(x[b0:] @ M[b0:, b0:] @ x[b0:]) if nb else 0.0
        k = int(np.argmax(np.abs(x[:n])))
        aus.append((f[m], luft, kasten, k + 1, x[k] ** 2))
    return aus


def versteifung(fm, fp, fh=190.0):
    """Bells Rechnung (S. 114 f.) am Paar: Versteifung der Bodenmode durch die Luft in Hz."""
    return np.sqrt(max(fp ** 2 + fm ** 2 - fh ** 2 - (fp * fm / fh) ** 2, 0.0))


def main():
    global Bvol
    v = kp.Vakuum(sys.argv[2])
    gp = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbodyplan")
    cav = gp["cavity"]
    L = max(p[1] for p in gp["plate"]["outline"])
    Bvol = ro.volumen_laengs(pathlib.Path(sys.argv[2]) / "formen.txt", L)
    xy, tri, w = lies_formen(pathlib.Path(sys.argv[2]) / "formen.txt", len(v.f))
    Pk, mboden = bodenmasse(xy, tri, w, L)
    print("Bell: Kasten A lose 176 / 201 (Versteifung 25 Hz), 250, 364 | G lose 153 / 200, in der Harfe 170 / 216, besaitet 187 / 207, 257, 334")
    print("je Zeile die Moden 100-480 Hz: Hz (Luftanteil, Anteil des Kastens an der Bewegungsenergie, staerkste Plattenmode)")

    def zeile(name, e):
        paar = [x for x in e if x[1] > 0.15 and x[3] == 1][:2]
        vs = f" Versteifung {versteifung(paar[0][0], paar[1][0]):4.1f} Hz" if len(paar) == 2 else ""
        print(f"{name:34s}|{vs} | " + "  ".join(f"{x[0]:.0f} ({x[1]:.2f} {x[2]:.2f} P{x[3]})" for x in e if x[0] < 400))

    zeile("starrer Rahmen (heute)", system(v, cav, gp, Pk, mboden, 4.0, 300.0, "frei", starr=True))
    for masse in (2.0, 4.0, 8.0):
        for fb in (150.0, 250.0, 400.0, 800.0, 5000.0):
            for halt in ("frei", "gehalten"):
                zeile(f"Kasten {masse:.0f} kg, {fb:.0f} Hz, {halt}", system(v, cav, gp, Pk, mboden, masse, fb, halt))


if __name__ == "__main__":
    main()

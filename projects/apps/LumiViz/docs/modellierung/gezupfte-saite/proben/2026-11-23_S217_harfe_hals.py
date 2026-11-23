"""S217 - Probe: das obere Ende der Saiten als Schwinger (Hals) statt als fester Punkt.

Bell 1988, Kasten G: in der Harfe ohne Saiten 170 / 216 / 249 / 316 Hz, besaitet (Saiten gedaempft) 187 / 207 / 257 /
334 Hz - beim Besaiten steigt die untere Mode des Paars um 17 Hz, die obere FAELLT um 9 Hz. Federn gegen einen festen
Punkt (KQ46 a, 0,3 E A / L) koennen keine Mode senken. Das heutige Modell: ohne Saiten 165 / 211 / 233 / 320 Hz,
besaitet 174 / 218 / 249 / 332 Hz.

Probe: die Saiten als Federn zwischen ihren Oesen und EINEM Freiheitsgrad des Halses (Masse m_N, eigene Frequenz f_N
ohne Saiten). Liegt f_N knapp ueber dem Paar, drueckt der Hals die obere Mode nach unten - derselbe Mechanismus wie
die Biegung des losen Kastens (`2026-11-23_S217_harfe_kasten_balken.py`). Ein Freiheitsgrad fuer den ganzen Hals ist
grob; die Probe zeigt, ob die Richtung und die Groesse stimmen koennen.

  python <probe> <vak ohne Saiten> <vak mit Saiten> <gebaut>

Aufruf vom Projekt-Root mit Python 3.12 (numpy, scipy).
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.linalg
import scipy.spatial

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("komplex", HIER / "2026-11-23_S217_harfe_daempfung_komplex.py")
dk = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(dk)
ro, kp = dk.ro, dk.kp
_spec = importlib.util.spec_from_file_location("balken216", HIER / "2026-11-23_S216_harfe_kasten_balken.py")
kb = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(kb)

BELL_OHNE = [170.0, 216, 249, 316]
BELL_MIT = [187.0, 207, 257, 334]


def moden(K, M, n, no, von=120.0, bis=360.0):
    lam, vec = scipy.linalg.eigh(K, M)
    f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    aus = []
    a0 = n + no
    for m in np.where((f > von) & (f < bis))[0]:
        x = vec[:, m]
        luft = float(x[n:a0] @ M[n:a0, n:a0] @ x[n:a0] + (x[a0:a0 + 38] ** 2 * np.diag(M)[a0:a0 + 38]).sum())
        hals = float(x[-1] ** 2 * M[-1, -1]) if K.shape[0] > a0 + 38 else 0.0
        k = int(np.argmax(np.abs(x[:4])))
        aus.append(f"{f[m]:5.1f} (L {luft:.2f} H {hals:.2f} P{k + 1})")
    return "  ".join(aus)


def main():
    ohne, mit, gebaut = sys.argv[1], sys.argv[2], sys.argv[3]
    v = kp.Vakuum(ohne)
    gp = kp.lade(pathlib.Path(gebaut) / "concert_harp.lvinstbodyplan")
    L = max(p[1] for p in gp["plate"]["outline"])
    Bvol = ro.volumen_laengs(pathlib.Path(ohne) / "formen.txt", L)
    K, M, n, no = dk.baue(v, gp["cavity"], gp, L, Bvol)
    N = K.shape[0]
    xy, tri, w = kb.lies_formen(pathlib.Path(ohne) / "formen.txt", n)
    federn = kp.lade(pathlib.Path(mit) / "concert_harp.lvinstbodyplan")["pointSprings"]
    _, kn = scipy.spatial.cKDTree(xy).query(np.array([[s["x"], s["y"]] for s in federn]))
    Phi = w[:, kn]                                   # [k, Feder]
    ks = np.array([s["stiffness"] for s in federn])
    print(f"Bell Kasten G in der Harfe: ohne Saiten {BELL_OHNE}, besaitet {BELL_MIT}")
    print(f"Federn im Bauplan: {len(ks)}, Summe {ks.sum():.3g} N/m (= 0,3 E A / L)")
    print("Moden 120-360 Hz (Luftanteil, Anteil des Halses, staerkste Plattenform)")
    print(f"  ohne Saiten:                     {moden(K, M, n, no)}")
    Kf = K.copy()
    Kf[:n, :n] += (Phi * ks) @ Phi.T
    print(f"  Federn gegen fest (heute, 0,3):  {moden(Kf, M, n, no)}")
    for beta in (1.0, 2.0, 10.0 / 3.0):
        for mN in (1.0, 3.0):
            for fN in (200.0, 230.0, 260.0, 300.0, 400.0):
                K2 = np.zeros((N + 1, N + 1)); M2 = np.zeros((N + 1, N + 1))
                K2[:N, :N] = K; M2[:N, :N] = M
                k = beta * ks
                K2[:n, :n] += (Phi * k) @ Phi.T
                K2[:n, N] = K2[N, :n] = -(Phi * k).sum(axis=1)
                K2[N, N] = k.sum() + mN * (2 * np.pi * fN) ** 2
                M2[N, N] = mN
                print(f"  Federn x {beta:.2f}, Hals {mN:.0f} kg, {fN:.0f} Hz: {moden(K2, M2, n, no)}")


if __name__ == "__main__":
    main()

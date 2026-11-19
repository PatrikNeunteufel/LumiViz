"""S209 - Gegenproben zu KQ43 a / KQ44 a an der besten Fassung der Familie R (fein3 mit 7 2):
 1. Netz: Moden unter 450 Hz mit dem Netz fuer 450 Hz gegen das Netz fuer 2500 und 5000 Hz (Steg als Leiste!).
 2. Aufnehmer 5,8 g am Antwortpunkt und Rosetten 7 g + 2 x 1 g am Lochrand als Massenaenderung im Modenraum
    (alle Moden bis 2500 Hz: K = diag(w^2), M = I + Summe m_k phi(k) phi(k)^T; dicht geloest) gegen die Rechnung
    erster Ordnung.
"""
import json
import subprocess

import numpy as np
from scipy.linalg import eigh

import oud_kombi as k
import oud_suche as s

X = (1.806, 5.767, 19.841, 21.674, 1.376)
plan = k.fassung(dicke=X[0] / 1000, abstand=0.002, steg=(0.17, X[2] / 1000, X[1] / 1000, 650.0, 10e9), volumen=X[3] / 1000, muendung=X[4])
pd = s.HIER / "gegenprobe.lvinstbodyplan"
s.HIER.mkdir(parents=True, exist_ok=True)
pd.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")


def lauf(to, netz, name):
    fd = s.HIER / f"gegenprobe_{name}_formen.txt"
    r = subprocess.run([str(s.TOOL), str(pd), "--to", str(to), "--mesh-to", str(netz), "--statistics-to", "0", "--out", str(s.HIER / f"gegenprobe_{name}.lvinstbody"),
                        "--shapes", str(fd)], capture_output=True, text=True, encoding="utf-8", errors="replace")
    zeilen = [z for z in r.stdout.split("\n") if z.startswith("Mesh") or z.startswith("Plate")]
    return s.formen(fd), zeilen


print("1. Netz")
for netz in (450, 2500, 5000):
    (kn, dr, hz, w), zeilen = lauf(450, netz, f"netz{netz}")
    print(f"   Netz fuer {netz} Hz ({len(kn)} Knoten): " + " / ".join(f"{f:.1f}" for f in hz if f < 430))
    for z in zeilen:
        print("      " + z.strip()[:150])

print("2. Massen im Modenraum (Netz und Moden bis 2500 Hz)")
(kn, dr, hz, w), _ = lauf(2500, 2500, "voll")
print(f"   {len(hz)} Moden bis {hz[-1]:.0f} Hz")
nae = lambda x, y: int(np.argmin((kn[:, 0] - x) ** 2 + (kn[:, 1] - y) ** 2))
ap = nae(0.075, 0.135)


def rand(x, y, r):
    d = np.hypot(kn[:, 0] - x, kn[:, 1] - y)
    return np.where(np.abs(d - r) < 0.004)[0]


gross, kl, kr = rand(0.0, 0.30, 0.053), rand(-0.103, 0.204, 0.020), rand(0.103, 0.204, 0.020)


def mit_massen(massen, bis):
    """massen: Liste (Knoten, kg); Moden bis `bis` Hz als Basis."""
    n = int(np.sum(hz <= bis))
    M = np.eye(n)
    for kn_, m in massen:
        v = w[:n, kn_]
        M += m * np.outer(v, v)
    lam = eigh(np.diag((2 * np.pi * hz[:n]) ** 2), M, eigvals_only=True)
    return np.sqrt(lam) / (2 * np.pi)


aufn = [(ap, 0.0058)]
ros = [(i, 0.007 / len(gross)) for i in gross] + [(i, 0.001 / len(kl)) for i in kl] + [(i, 0.001 / len(kr)) for i in kr]
for was, massen in (("Aufnehmer 5,8 g", aufn), ("Rosetten 7 + 1 + 1 g", ros), ("Rosetten und Aufnehmer", ros + aufn)):
    erste = hz * (1 - 0.5 * sum(m * w[:, i] ** 2 for i, m in massen))
    print(f"   {was}:")
    print("      ohne Masse:        " + " / ".join(f"{f:6.1f}" for f in hz[:6]))
    print("      erste Ordnung:     " + " / ".join(f"{f:6.1f}" for f in erste[:6]))
    for bis in (450, 1000, 2500):
        print(f"      Modenraum bis {bis:4d}: " + " / ".join(f"{f:6.1f}" for f in mit_massen(massen, bis)[:6]))

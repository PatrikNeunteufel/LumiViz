"""S214 (KQ58 a) - Probe: die Luft im Kasten der Harfe als Rohr mit Oeffnungen laengs.

`Luftraum.hpp` rechnet einen Druck fuer den ganzen Hohlraum. Der Kasten der Harfe ist 1,3 m lang, die erste Laengsmode
der eingeschlossenen Luft liegt bei geschlossenen Oeffnungen um 150 Hz - unter dem Paar mit Luft. Die Probe nimmt den
Druck laengs des Kastens mit: Moden eines Rohrs mit dem Querschnitt S(y) = k w(y)^2 (w = Breite des Bodens, k aus dem
Volumen), beide Enden zu; je Oeffnung ein Luftpfropfen an seinem Ort; die Plattenmoden im Vakuum treiben das Rohr mit
ihrem Volumen je Laenge b_k(y).

  Koordinaten  q_k (Plattenmoden), u_i (Volumen je Oeffnung nach aussen), s_n (Hilfsgroesse je Rohrmode n >= 1)
  Druck        P_n = -alpha (D_n - s_n),  D_n = sum_k B_nk q_k + sum_i psi_n(y_i) u_i,  alpha = rho c^2 / V
  Energie      U = 1/2 sum_k w_k^2 q_k^2 + 1/2 alpha sum_n (D_n - s_n)^2 (s_0 = 0)
               T = 1/2 sum q'^2 + 1/2 sum M_i u_i'^2 + 1/2 sum (alpha / w_n^2) s_n'^2
  (aus S/(rho c^2) p_tt - (S p_y)_y / rho = -d_tt, entwickelt nach den Rohrmoden; mit n = 0 allein ist es `Luftraum.hpp`.)

  rohr <vak> <gebaut>                      Rohr allein, starre Waende: Helmholtz und Laengsmoden gegen Bell Kap. 4
  raster <vak> <gebaut>                    Muendung x Zahl der Rohrmoden: Paar, Lagen, Abstrahlung
  zu <vak> <gebaut> <muendung>             Oeffnungen verschlossen (Bell S. 106, Le Carrou 2007 Tab. II)
  schreibe <vak> <gebaut> <ziel> <muendung> [<moden> [<f1-faktor>]]   Koerper fuer die Lautheit

Naeherungen wie in der Probe S213 (Abstrahlung der Platte je Mode bei ihrer Frequenz im Vakuum, Laufzeit nachgefuehrt;
Verlust aus Gesetz und Guete der Luft nach Anteilen). Ein Rohr kennt keine Quermoden und kein Nahfeld um die Oeffnung
im Innern (das steckt in der Muendungskorrektur). Aufruf vom Projekt-Root mit Python 3.12 (numpy).
"""
import importlib.util
import math
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("kopplung", HIER / "2026-11-22_S213_harfe_kopplung.py")
kp = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(kp)

ZELLEN = 262          # 5 mm


def volumen_laengs(pfad, laenge):
    """b_k(y): Volumen der Form je Zelle laengs (Summe = a_k)."""
    z = pathlib.Path(pfad).read_text(encoding="utf-8").split("\n")
    n = int(z[1].split()[1])
    xy = np.array([[float(v) for v in z[2 + i].split()] for i in range(n)])
    t = int(z[2 + n].split()[1])
    tri = np.array([[int(v) for v in z[3 + n + i].split()] for i in range(t)])
    m = int(z[3 + n + t].split()[1])
    a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
    fl = 0.5 * np.abs((b[:, 0] - a[:, 0]) * (c[:, 1] - a[:, 1]) - (c[:, 0] - a[:, 0]) * (b[:, 1] - a[:, 1]))
    zelle = np.clip(((a[:, 1] + b[:, 1] + c[:, 1]) / 3 / laenge * ZELLEN).astype(int), 0, ZELLEN - 1)
    B = np.zeros((m, ZELLEN))
    for i in range(m):
        w = np.array(z[5 + n + t + 2 * i].split(), dtype=float)
        np.add.at(B[i], zelle, fl * w[tri].mean(axis=1))
    return B


def breite(plan, y):
    u = plan["plate"]["outline"]
    rechts = sorted((p[1], p[0]) for p in u if p[0] > 0)
    return 2 * np.interp(y, [p[0] for p in rechts], [p[1] for p in rechts])


def rohrmoden(plan, V, c, zahl):
    """Moden des beidseitig geschlossenen Rohrs; psi je Zelle, normiert auf sum S psi^2 dy = V."""
    L = max(p[1] for p in plan["plate"]["outline"])
    dy = L / ZELLEN
    y = (np.arange(ZELLEN) + 0.5) * dy
    S = breite(plan, y) ** 2
    S *= V / (S.sum() * dy)
    Sk = np.sqrt(S[:-1] * S[1:])                         # Querschnitt an den Zellgrenzen
    K = np.zeros((ZELLEN, ZELLEN))
    for j in range(ZELLEN - 1):
        g = Sk[j] / dy
        K[j, j] += g
        K[j + 1, j + 1] += g
        K[j, j + 1] -= g
        K[j + 1, j] -= g
    m = S * dy / (c * c)
    lam, vec = np.linalg.eigh(K / np.sqrt(np.outer(m, m)))
    psi = (vec / np.sqrt(m)[:, None]).T[: zahl + 1]
    psi /= np.sqrt((S * dy * psi ** 2).sum(axis=1) / V)[:, None]
    if psi[0].mean() < 0:
        psi[0] = -psi[0]
    return L, y, np.sqrt(np.maximum(lam[: zahl + 1], 0.0)), psi


def kopple(v, B, cav, plan, muendung, guete, moden, beta=1.0, f1=1.0, offen=None, starr=False):
    rho, c = v.rho, v.c
    V = cav["volume"]
    alpha = rho * c * c / V
    L, y, wn, psi = rohrmoden(plan, V, c, moden)
    lo = []
    for i, o in enumerate(cav["openings"]):
        if offen is not None and i not in offen:
            continue
        lo.append((o["x"], o["y"], o["z"], o["area"], o["lipLength"] + muendung * math.sqrt(o["area"] / math.pi)))
    Ma = np.array([rho * Lk / A for *_, A, Lk in lo])
    zl = np.clip((np.array([o[1] for o in lo]) / L * ZELLEN).astype(int), 0, ZELLEN - 1) if lo else np.zeros(0, int)
    fv = v.f.copy()
    fv[0] *= f1
    n = 0 if starr else len(fv)
    no, ns = len(lo), moden
    N = n + no + ns
    K = np.zeros((N, N))
    K[np.arange(n), np.arange(n)] = (2 * np.pi * fv[:n]) ** 2
    Bn = beta * (B[:n] @ psi.T)                           # [k, n]
    for j in range(moden + 1):
        cvec = np.zeros(N)
        cvec[:n] = Bn[:, j]
        cvec[n:n + no] = psi[j, zl]
        if j >= 1:
            cvec[n + no + j - 1] = -1.0
        K += alpha * np.outer(cvec, cvec)
    M = np.concatenate([np.ones(n), Ma, alpha / wn[1:] ** 2])
    lam, vec = np.linalg.eigh(K / np.sqrt(np.outer(M, M)))
    fneu = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    Z = vec.T                                            # je Mode die massennormierten Anteile
    C = Z[:, :n]
    luft = (Z[:, n:] ** 2).sum(axis=1)
    U = Z[:, n:n + no] / np.sqrt(Ma)[None, :]
    e = {"f": fneu, "luft": luft, "u": U, "C": C, "psi": psi, "wn": wn, "y": y, "lo": lo}
    if starr:
        return e
    steg = C @ v.steg
    R = np.linalg.norm(v.orte - v.mitte, axis=1)
    G = np.zeros((N, len(R)), dtype=complex)
    dp = np.zeros((N, len(R)), dtype=complex)
    rl = [np.linalg.norm(v.orte - np.array([x, yy, zz]), axis=1) for x, yy, zz, *_ in lo]
    for m in range(N):
        dk = 2 * np.pi * (fneu[m] - v.f) / c
        G[m] = (C[m][:, None] * v.G * np.exp(-1j * np.outer(dk, R))).sum(axis=0)
        k = 2 * np.pi * fneu[m] / c
        for i, r in enumerate(rl):
            dp[m] += rho / (4 * np.pi) * U[m, i] / r * np.exp(-1j * k * r)
    dm = v.k["damping"]
    en = C ** 2 * (2 * np.pi * v.f) ** 2
    en = en / np.maximum(en.sum(axis=1, keepdims=True), 1e-300)
    feder = en @ v.feder
    stoff = en @ v.stoff
    gesetz = (dm["a"] * fneu ** 2 + dm["b"] * fneu + dm["c"]) / (2 * np.pi * np.maximum(fneu, 1e-9))
    eta = (1 - luft) * (1 - feder) * gesetz + luft / guete
    hh = kopple(v, B, cav, plan, muendung, guete, moden, offen=offen, starr=True)["f"]
    e.update({"eta": eta, "steg": steg, "G": G + dp, "feder": feder, "stoff": stoff,
              "helmholtz": float(hh[hh > 20][0]) if np.any(hh > 20) else 0.0, "fc": 0.0})
    return e


def nur_hoerbar(e, ab=30.0):
    gut = e["f"] > ab
    return {k: (w[gut] if isinstance(w, np.ndarray) and len(w) == len(gut) and k not in ("psi", "wn", "y") else w) for k, w in e.items()}


def zeige_paar(e, bis=320.0):
    for m in np.where((e["f"] > 100) & (e["f"] < bis))[0]:
        if e["luft"][m] > 0.02 or abs(e["C"][m][:3]).max() > 0.3:
            pl = " ".join(f"{x:+.2f}" for x in e["C"][m][:4])
            uu = " ".join(f"{x * 1e3:+.1f}" for x in e["u"][m])
            print(f"      {e['f'][m]:7.1f} Hz  Luft {e['luft'][m]:.2f}  Q {1 / e['eta'][m]:5.1f}  Platte 1-4: {pl}  u (1e-3): {uu}")


def main():
    was = sys.argv[1]
    v = kp.Vakuum(sys.argv[2])
    gp = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbodyplan")
    gk = kp.lade(pathlib.Path(sys.argv[3]) / "concert_harp.lvinstbody")
    cav = gp["cavity"]
    L = max(p[1] for p in gp["plate"]["outline"])
    B = volumen_laengs(pathlib.Path(sys.argv[2]) / "formen.txt", L)
    assert np.allclose(B.sum(axis=1), v.a, rtol=1e-6, atol=1e-9), "Volumen laengs passt nicht zu a_k"
    if was == "rohr":
        _, y, wn, psi = rohrmoden(gp, cav["volume"], v.c, 8)
        print("Rohr zu, starr (Hz):", " ".join(f"{w / (2 * np.pi):.0f}" for w in wn[:8]))
        print("Bell Kap. 4 (Sand): Helmholtz alle offen 190 +- 1,5 Hz; nur Oeffnung 6 offen: A2 283, A3 414, A4 545, A5 680 Hz")
        for mu in (1.0, 1.4, 1.7, 2.0, 2.3):
            a = kopple(v, B, cav, gp, mu, cav["q"], 40, starr=True)["f"]
            s = kopple(v, B, cav, gp, mu, cav["q"], 40, starr=True, offen=[5])["f"]
            e0 = kopple(v, B, cav, gp, mu, cav["q"], 0, starr=True)["f"]
            print(f"  Muendung {mu:.1f}: ein Druck {e0[e0 > 20][0]:6.1f} | Rohr alle offen " + " ".join(f"{x:.0f}" for x in a[a > 20][:6])
                  + " | nur 6 offen " + " ".join(f"{x:.0f}" for x in s[s > 20][:6]))
        return
    if was == "raster":
        for moden in (0, 1, 2, 4, 10, 40):
            for mu in (1.4, 1.7, 2.0, 2.3):
                e = nur_hoerbar(kopple(v, B, cav, gp, mu, cav["q"], moden))
                kp.zeile(f"Rohrmoden {moden:2d} mu {mu:.1f}", e, kp.koerper(v, gk, e), gp)
                if moden in (0, 40):
                    zeige_paar(e)
        return
    if was == "rest":
        # was nach dem Rohr fehlt: Volumen der Plattenmoden x beta, erste Plattenmode x f1 (Muendung fuer Helmholtz 190 Hz)
        mu = float(sys.argv[4])
        for beta in (1.0, 0.8, 0.7, 0.6, 0.5):
            for f1 in (1.0, 0.97, 0.94):
                e = nur_hoerbar(kopple(v, B, cav, gp, mu, cav["q"], 40, beta=beta, f1=f1))
                kp.zeile(f"Rohr beta {beta:.1f} f1 {f1:.2f}", e, kp.koerper(v, gk, e), gp)
        return
    if was == "zu":
        mu = float(sys.argv[4])
        for name, off in (("alle offen", None), ("alle zu", [])):
            e = nur_hoerbar(kopple(v, B, cav, gp, mu, cav["q"], 40, offen=off))
            print(name)
            zeige_paar(e)
        return
    if was == "schreibe":
        import json
        zi = pathlib.Path(sys.argv[4])
        mu = float(sys.argv[5])
        moden = int(sys.argv[6]) if len(sys.argv) > 6 else 40
        f1 = float(sys.argv[7]) if len(sys.argv) > 7 else 1.0
        e = nur_hoerbar(kopple(v, B, cav, gp, mu, cav["q"], moden, f1=f1))
        k = kp.koerper(v, gk, e)
        zi.mkdir(parents=True, exist_ok=True)
        (zi / "concert_harp.lvinstbody").write_text(json.dumps(k) + "\n", encoding="utf-8", newline="\n")
        (zi / "concert_harp.lvinstbodyplan").write_text(json.dumps(gp, indent=1) + "\n", encoding="utf-8", newline="\n")
        kp.zeile(f"Rohr {moden} mu {mu:.1f} f1 {f1:g}", e, k, gp)
        zeige_paar(e)
        return


if __name__ == "__main__":
    main()

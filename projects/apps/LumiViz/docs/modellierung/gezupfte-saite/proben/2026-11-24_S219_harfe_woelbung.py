"""S219 (KQ73 a) - Probe: die Woelbung des Bodens der Harfe unter dem Saitenzug (Platte mit grosser Auslenkung).

Senkrecht zum Boden ziehen die 46 Saiten mit rund 9350 N x sin 33 Grad = 5,1 kN laengs der Mittellinie; linear gerechnet
biegt sich der Boden darunter um etwa seine Dicke durch. Dann gilt die lineare Platte nicht mehr: die Mittelflaeche
dehnt sich, und um den gewoelbten Zustand schwingt der Boden wie eine flache Schale (von Karman).

  w(x, y) = sum_k a_k phi_k         die Moden des Bodens im Vakuum **ohne Saiten** als Ansatz (massennormiert)
  u(x, y)                           Verschiebung in der Ebene, lineare Dreiecke auf dem Netz der Formen
  eps = sym grad u + 1/2 grad w (x) grad w        Dehnung der Mittelflaeche, je Dreieck konstant
  U = 1/2 sum w_k^2 a_k^2 + 1/2 sum_e A_e eps^T (C h) eps + Leisten als Staebe (E A, eps = u' + 1/2 w'^2)

Last: je Saite T sin(Winkel) senkrecht und T cos(Winkel) in der Ebene zum Diskant an der Oese; die Zuege aus Frequenz,
Laenge und Dichte, auf Bells Summe 9350 N gestellt. Gleichgewicht mit Newton (u je Schritt herausgeloest), dann die
Steife um den gewoelbten Zustand (Traegheit in der Ebene vernachlaessigt) -> Moden im Vakuum, ihr verdraengtes Volumen,
und mit den Saiten als Federn (0,3 E A / L) und der Luft (Rohr, Massenmatrix der Oeffnungen, starrer Rahmen) das Paar.

Rand in der Ebene, drei Faelle:
  gehalten   u = 0 am ganzen Rand (obere Schranke)
  schale     an den langen Seiten nur laengs gehalten (die Schale ist laengs steif, quer weich), Enden gehalten
  frei       lange Seiten frei, Enden gehalten (untere Schranke)

Nicht enthalten: die Aussermittigkeit der Leisten, das Furnier, Kriechen des Holzes unter Dauerlast, die Rueckwirkung
der Woelbung auf die Saitenwinkel.

Aufruf vom Projekt-Root: python -X utf8 -B <skript> <vak ohne Saiten> <gebaut> [last=1.0,0.5] [pruefe=1]
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.linalg
import scipy.sparse as sp
import scipy.sparse.linalg as spl
import scipy.spatial

HIER = pathlib.Path(__file__).resolve().parent


def _lade(name, datei):
    spec = importlib.util.spec_from_file_location(name, HIER / datei)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


k17 = _lade("k17w", "2026-11-23_S217_harfe_kasten_balken.py")
sd = _lade("sdw", "2026-11-23_S214_harfe_saitendaempfung.py")
kb, ge, ro, kp = k17.kb, k17.ge, k17.ro, k17.kp
WINKEL = np.arcsin(np.sqrt(0.3))
ZUG_BELL = 9350.0


class Boden:
    def __init__(self, vak, gebaut):
        self.ka = ka = k17.Kasten(vak, gebaut)
        gp = self.gp = ka.gp
        self.n = n = len(ka.v.f)
        xy, tri = ka.xy, ka.tri
        self.w = ka.w[:n]
        self.nn = nn = len(xy)
        a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
        det = (b[:, 0] - a[:, 0]) * (c[:, 1] - a[:, 1]) - (c[:, 0] - a[:, 0]) * (b[:, 1] - a[:, 1])
        um = det < 0
        tri = tri.copy(); tri[um] = tri[um][:, [0, 2, 1]]
        a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
        det = np.abs(det)
        self.tri, self.fl = tri, det / 2
        # Gradienten der drei Formfunktionen je Dreieck
        gr = np.stack([np.column_stack([b[:, 1] - c[:, 1], c[:, 0] - b[:, 0]]),
                       np.column_stack([c[:, 1] - a[:, 1], a[:, 0] - c[:, 0]]),
                       np.column_stack([a[:, 1] - b[:, 1], b[:, 0] - a[:, 0]])], axis=1) / det[:, None, None]
        ne = len(tri)
        self.G = np.einsum("ejd,kej->edk", gr, self.w[:, tri])             # (ne, 2, n): Gradient der Form k
        B = np.zeros((ne, 3, 6))
        for j in range(3):
            B[:, 0, 2 * j] = gr[:, j, 0]; B[:, 1, 2 * j + 1] = gr[:, j, 1]
            B[:, 2, 2 * j] = gr[:, j, 1]; B[:, 2, 2 * j + 1] = gr[:, j, 0]
        self.B = B
        self.dof = np.stack([2 * tri[:, 0], 2 * tri[:, 0] + 1, 2 * tri[:, 1], 2 * tri[:, 1] + 1, 2 * tri[:, 2], 2 * tri[:, 2] + 1], axis=1)
        st = {m["name"]: m for m in gp["materials"]}
        s = st[gp["plate"]["material"]]
        g = gp["plate"]["thicknessGrid"]
        werte = np.array(g["values"]).reshape(g["ny"], g["nx"])
        xm, ym = (a[:, 0] + b[:, 0] + c[:, 0]) / 3, (a[:, 1] + b[:, 1] + c[:, 1]) / 3
        ix = np.clip(np.round((xm - g["origin"][0]) / g["step"]).astype(int), 0, g["nx"] - 1)
        iy = np.clip(np.round((ym - g["origin"][1]) / g["step"]).astype(int), 0, g["ny"] - 1)
        self.h = h = werte[iy, ix]
        nyx = s["nuxy"] * s["ey"] / s["ex"]
        nen = 1 - s["nuxy"] * nyx
        C0 = np.array([[s["ex"] / nen, nyx * s["ex"] / nen, 0], [nyx * s["ex"] / nen, s["ey"] / nen, 0], [0, 0, s["gxy"]]])
        self.C = h[:, None, None] * C0[None]
        # Leisten als Staebe laengs ihrer Linie
        baum = scipy.spatial.cKDTree(xy)
        st_i, st_j, st_ea, st_l = [], [], [], []
        for rib in gp["ribs"]:
            linie = np.array(rib["line"])
            sl = np.concatenate([[0], np.cumsum(np.hypot(*np.diff(linie, axis=0).T))])
            ss = np.arange(0.0, sl[-1] + 1e-9, 0.01)
            px, py = np.interp(ss, sl, linie[:, 0]), np.interp(ss, sl, linie[:, 1])
            _, kn = baum.query(np.column_stack([px, py]))
            sh = sl if len(rib["height"]) == len(sl) else np.linspace(0, sl[-1], len(rib["height"]))
            sw = sl if len(rib["width"]) == len(sl) else np.linspace(0, sl[-1], len(rib["width"]))
            for i in range(len(kn) - 1):
                if kn[i] == kn[i + 1]:
                    continue
                sm = (ss[i] + ss[i + 1]) / 2
                st_i.append(kn[i]); st_j.append(kn[i + 1])
                st_ea.append(st[rib["material"]]["ex"] * np.interp(sm, sh, rib["height"]) * np.interp(sm, sw, rib["width"]))
                st_l.append(np.hypot(*(xy[kn[i + 1]] - xy[kn[i]])))
        self.si, self.sj = np.array(st_i), np.array(st_j)
        self.sea, self.sl = np.array(st_ea), np.array(st_l)
        self.sr = (xy[self.sj] - xy[self.si]) / self.sl[:, None]           # Richtung
        self.sg = (self.w[:, self.sj] - self.w[:, self.si]).T / self.sl[:, None]   # (ns, n): w' der Form k laengs
        self.sdof = np.stack([2 * self.si, 2 * self.si + 1, 2 * self.sj, 2 * self.sj + 1], axis=1)
        self.sB = np.concatenate([-self.sr, self.sr], axis=1) / self.sl[:, None]   # (ns, 4)
        # K_uu
        ke = np.einsum("eai,eab,ebj->eij", B, self.C * self.fl[:, None, None], B)
        rows = np.repeat(self.dof, 6, axis=1).ravel(); cols = np.tile(self.dof, (1, 6)).ravel()
        K = sp.coo_matrix((ke.ravel(), (rows, cols)), shape=(2 * nn, 2 * nn))
        kse = np.einsum("ei,e,ej->eij", self.sB, self.sea * self.sl, self.sB)
        rows = np.repeat(self.sdof, 4, axis=1).ravel(); cols = np.tile(self.sdof, (1, 4)).ravel()
        self.Kuu = (K + sp.coo_matrix((kse.ravel(), (rows, cols)), shape=(2 * nn, 2 * nn))).tocsc()
        # Rand
        kanten = np.sort(np.concatenate([tri[:, [0, 1]], tri[:, [1, 2]], tri[:, [2, 0]]]), axis=1)
        uni, zahl = np.unique(kanten, axis=0, return_counts=True)
        rand = np.unique(uni[zahl == 1])
        L = ka.L
        self.ende = rand[(xy[rand, 1] < 1e-6) | (xy[rand, 1] > L - 1e-6)]
        self.seite = np.setdiff1d(rand, self.ende)
        # Saiten
        federn = sorted(gp["pointSprings"], key=lambda q: q["y"])
        z = sd.wellenwiderstaende()
        zug = np.array([q[2] for q in z]); zug *= ZUG_BELL / zug.sum()
        _, oese = baum.query(np.array([[q["x"], q["y"]] for q in federn]))
        self.oese = oese
        self.f_a = (zug * np.sin(WINKEL)) @ self.w[:, oese].T
        self.f_u = np.zeros(2 * nn)
        np.add.at(self.f_u, 2 * oese + 1, zug * np.cos(WINKEL))
        self.k_feder = np.array([q["stiffness"] for q in federn])
        self.w2 = (2 * np.pi * ka.v.f[:n]) ** 2

    def rand(self, fall):
        fest = [2 * self.ende, 2 * self.ende + 1]
        if fall == "gehalten":
            fest += [2 * self.seite, 2 * self.seite + 1]
        elif fall == "schale":
            fest += [2 * self.seite + 1]
        frei = np.setdiff1d(np.arange(2 * self.nn), np.concatenate(fest))
        return frei, spl.splu(self.Kuu[frei][:, frei].tocsc())

    def zustand(self, a, frei, lu, last, tangente=False):
        """Rueckstellkraft in a (u herausgeloest), Energie, auf Wunsch die Steife um den Zustand."""
        g = self.G @ a                                              # (ne, 2)
        q = np.stack([0.5 * g[:, 0] ** 2, 0.5 * g[:, 1] ** 2, g[:, 0] * g[:, 1]], axis=1)
        gs = self.sg @ a
        qs = 0.5 * gs ** 2
        ru = last * self.f_u.copy()
        np.subtract.at(ru, self.dof, np.einsum("eai,ea->ei", self.B, np.einsum("eab,eb->ea", self.C, q) * self.fl[:, None]))
        np.subtract.at(ru, self.sdof, self.sB * (self.sea * self.sl * qs)[:, None])
        u = np.zeros(2 * self.nn)
        u[frei] = lu.solve(ru[frei])
        eps = np.einsum("eai,ei->ea", self.B, u[self.dof]) + q
        N = np.einsum("eab,eb->ea", self.C, eps)
        es = np.einsum("ei,ei->e", self.sB, u[self.sdof]) + qs
        Ns = self.sea * es
        Ng = np.stack([N[:, 0] * g[:, 0] + N[:, 2] * g[:, 1], N[:, 2] * g[:, 0] + N[:, 1] * g[:, 1]], axis=1)
        R = self.w2 * a + np.einsum("edk,ed->k", self.G, Ng * self.fl[:, None]) + self.sg.T @ (Ns * gs * self.sl) - last * self.f_a
        U = (0.5 * self.w2 @ a ** 2 + 0.5 * np.sum(self.fl * np.einsum("ea,ea->e", eps, N)) + 0.5 * np.sum(self.sl * es * Ns)
             - last * self.f_a @ a - last * self.f_u @ u)
        if not tangente:
            return R, U, None, N
        n = self.n
        Gx, Gy = self.G[:, 0, :], self.G[:, 1, :]
        E = np.stack([g[:, 0, None] * Gx, g[:, 1, None] * Gy, g[:, 1, None] * Gx + g[:, 0, None] * Gy], axis=1)   # (ne, 3, n)
        CE = np.einsum("eab,ebk->eak", self.C, E) * self.fl[:, None, None]
        S = np.diag(self.w2) + np.einsum("eak,eal->kl", E, CE)
        S += np.einsum("ek,e,el->kl", Gx, N[:, 0] * self.fl, Gx) + np.einsum("ek,e,el->kl", Gy, N[:, 1] * self.fl, Gy)
        t = np.einsum("ek,e,el->kl", Gx, N[:, 2] * self.fl, Gy)
        S += t + t.T
        Es = gs[:, None] * self.sg
        S += np.einsum("ek,e,el->kl", Es, self.sea * self.sl, Es) + np.einsum("ek,e,el->kl", self.sg, Ns * self.sl, self.sg)
        Kua = np.zeros((2 * self.nn, n))
        np.add.at(Kua, self.dof, np.einsum("eai,eak->eik", self.B, CE))
        np.add.at(Kua, self.sdof, self.sB[:, :, None] * (self.sea * self.sl)[:, None, None] * Es[:, None, :])
        X = lu.solve(Kua[frei])
        S -= Kua[frei].T @ X
        return R, U, 0.5 * (S + S.T), N

    def loese(self, fall, last, schritte=8):
        frei, lu = self.rand(fall)
        a = np.zeros(self.n)
        for s in range(1, schritte + 1):
            for it in range(40):
                R, U, S, _ = self.zustand(a, frei, lu, last * s / schritte, True)
                da = np.linalg.solve(S, -R)
                a = a + da
                if np.linalg.norm(da) < 1e-10 * max(np.linalg.norm(a), 1e-12):
                    break
            else:
                raise RuntimeError("Newton laeuft nicht zusammen")
        R, U, S, N = self.zustand(a, frei, lu, last, True)
        return a, S, N, frei, lu

    def luft(self, Kp, offen=True, moden=38):
        """Paar mit Luft: starrer Rahmen wie `Kasten.system(starr=True)`, aber die Platte mit voller Steifematrix."""
        ka = self.ka
        v, gp, cav = ka.v, ka.gp, ka.cav
        rho, c, V = v.rho, v.c, cav["volume"]
        alpha = rho * c * c / V
        _, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
        oe = cav["openings"] if offen else []
        n, no = self.n, len(oe)
        Mo = ge.massen(rho, cav["openings"], 1.0, 1.0)[0] if no else np.zeros((0, 0))
        Pl = np.zeros((moden + 1, no))
        for i, o in enumerate(oe):
            sel = np.abs(y - o["y"]) <= ge.LAENGE[i] / 2
            Pl[:, i] = psi[:, sel].mean(axis=1)
        a0 = n + no
        N = a0 + moden
        K = np.zeros((N, N)); M = np.zeros((N, N))
        K[:n, :n] = Kp
        Bn = ka.Bvol[:n] @ psi.T
        for j in range(moden + 1):
            cv = np.zeros(N); cv[:n] = Bn[:, j]; cv[n:a0] = Pl[j]
            if j >= 1:
                cv[a0 + j - 1] = -1.0
            K += alpha * np.outer(cv, cv)
        M[np.arange(n), np.arange(n)] = 1.0
        M[n:a0, n:a0] = Mo
        M[np.arange(a0, N), np.arange(a0, N)] = alpha / wn[1:] ** 2
        lam, vec = scipy.linalg.eigh(K, M)
        f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
        aus = []
        for m in np.where((f > 100) & (f < 360))[0]:
            x = vec[:, m]
            lu = float(x[n:a0] @ Mo @ x[n:a0] + (x[a0:] ** 2 * np.diag(M)[a0:]).sum())
            aus.append((f[m], lu, x[:n].copy()))
        return aus


def bericht(bo, name, S):
    n = bo.n
    vol = bo.ka.Bvol[:n].sum(axis=1)
    phi_o = bo.w[:, bo.oese]                                         # (n, Saiten)
    Kf = (phi_o * bo.k_feder) @ phi_o.T
    for text, Kp in (("ohne Federn", S), ("mit Federn 0,3 E A / L", S + Kf)):
        lam, X = np.linalg.eigh(Kp)
        f = np.sqrt(np.maximum(lam, 0)) / (2 * np.pi)
        v = X.T @ vol
        print(f"  {name}, {text}: Vakuum " + "  ".join(f"{f[i]:.1f} (V {abs(v[i]) * 1e3:.2f})" for i in range(6)))
        offen = bo.luft(Kp)
        zu = bo.luft(Kp, offen=False)
        print("      mit Luft:   " + "  ".join(f"{a:.1f} (L {b:.2f})" for a, b, _ in offen))
        print("      Oeffnungen zu: " + "  ".join(f"{a:.1f} (L {b:.2f})" for a, b, _ in zu if a < 300))
        # Bells Zwei-Schwinger-Rechnung an den zwei Moden unter 240 Hz mit dem groessten Anteil der tiefsten Vakuum-Mode
        x1 = X[:, 0]
        paar = sorted(sorted([m for m in offen if m[0] < 245], key=lambda m: -(m[2] @ x1) ** 2)[:2])
        bz = max([m for m in zu if m[0] < 280], key=lambda m: (m[2] @ x1) ** 2)
        fh, fa = k17.bell(paar[0][0], paar[1][0], bz[0])
        print(f"      Bell-Rechnung: Paar {paar[0][0]:.1f} / {paar[1][0]:.1f}, zu {bz[0]:.1f}, f_H {fh:.0f}, f_a {fa:.0f} Hz")


def main():
    bo = Boden(sys.argv[1], sys.argv[2])
    opt = dict(a.split("=") for a in sys.argv[3:])
    xy = bo.ka.xy
    print(f"Netz {bo.nn} Knoten, {len(bo.tri)} Dreiecke, {len(bo.si)} Stabstuecke der Leisten, {bo.n} Formen; "
          f"Last senkrecht {ZUG_BELL * np.sin(WINKEL):.0f} N, in der Ebene {ZUG_BELL * np.cos(WINKEL):.0f} N")
    print("Bell Kasten G in der Harfe: ohne Saiten 170 / 216 / 249 / 316, besaitet 187 / 207 / 257 / 284 / 334 Hz; "
          "V = verdraengtes Volumen der Mode in 10^-3 m^3 / sqrt(kg)")
    if opt.get("pruefe") == "1":
        frei, lu = bo.rand("schale")
        rng = np.random.default_rng(1)
        a = rng.normal(size=bo.n) * 2e-3 / (1 + np.arange(bo.n))
        R, U, S, _ = bo.zustand(a, frei, lu, 1.0, True)
        d = rng.normal(size=bo.n) * 1e-7 / (1 + np.arange(bo.n))
        Rp, Up, _, _ = bo.zustand(a + d, frei, lu, 1.0)
        Rm, Um, _, _ = bo.zustand(a - d, frei, lu, 1.0)
        print(f"Pruefung: dU {Up - Um:.6e} gegen 2 R.d {2 * R @ d:.6e}; |dR - 2 S d| / |dR| = "
              f"{np.linalg.norm(Rp - Rm - 2 * S @ d) / np.linalg.norm(Rp - Rm):.2e}")
    wl = (bo.f_a / bo.w2) @ bo.w
    print(f"linear: groesste Auslenkung {abs(wl).max() * 1e3:.2f} mm bei y = {xy[np.argmax(abs(wl)), 1]:.2f} m")
    bericht(bo, "ohne Last", np.diag(bo.w2))
    for fall in ("frei", "schale", "gehalten"):
        for last in [float(x) for x in opt.get("last", "1.0").split(",")]:
            a, S, N, frei, lu = bo.loese(fall, last)
            w0 = a @ bo.w
            i = int(np.argmax(abs(w0)))
            mitte = np.abs(xy[:, 0]) < 0.004
            prof = " ".join(f"{yy:.1f}:{np.interp(yy, xy[mitte, 1][np.argsort(xy[mitte, 1])], w0[mitte][np.argsort(xy[mitte, 1])]) * 1e3:.1f}"
                            for yy in (0.2, 0.4, 0.6, 0.8, 1.0, 1.2))
            print(f"\n=== Rand {fall}, Last x {last:g}: Woelbung {w0[i] * 1e3:.2f} mm bei y = {xy[i, 1]:.2f} m, x = {xy[i, 0]:.2f} "
                  f"(Dicke dort {np.interp(xy[i, 1], [0, bo.ka.L], [10, 3]):.1f} mm); Mittellinie mm {prof}")
            print(f"    Kraefte der Mittelflaeche: N_x {N[:, 0].min() / 1e3:.1f} .. {N[:, 0].max() / 1e3:.1f} kN/m, "
                  f"N_y {N[:, 1].min() / 1e3:.1f} .. {N[:, 1].max() / 1e3:.1f} kN/m")
            bericht(bo, f"{fall} x {last:g}", S)


if __name__ == "__main__":
    main()

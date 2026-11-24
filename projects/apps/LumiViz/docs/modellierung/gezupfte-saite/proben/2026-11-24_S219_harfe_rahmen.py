"""S219 (KQ70 a) - Probe: die Harfe als ebener Rahmen - Kasten, Sockel, Saeule, Hals - mit Saiten zwischen Oese und Hals.

Frage (KQ70 a): was aendert das Besaiten? Bell 1988 Kasten G in der Harfe: ohne Saiten B2 135, 170 / 216 / 249 / 316 Hz,
besaitet B2 rund 110 (Abb. 7.18), 187 / 207 / 257 / 284 / 334 Hz. Federn gegen einen festen Punkt (`InstrumentBody`
heute) heben jede Mode; ein Hals als ein Schwinger genuegte nicht (`2026-11-23_S217_harfe_hals.py`).

Modell, alles in der Ebene der Saiten:
  Kasten   Balken laengs des Bodens mit EI(y) und Masse aus der Geometrie (`2026-11-23_S217_harfe_kasten_balken.py`),
           dazu Dehnsteife; der Boden schwingt mit seinen Moden im Vakuum **ohne Saiten** relativ dazu, die Luft (Rohr,
           Massenmatrix der Oeffnungen) haengt an der relativen Bewegung.
  Sockel   steifes Stueck vom Fuss des Kastens zum Fuss der Saeule, Masse angenommen.
  Saeule   senkrecht, Rohr aus Schichtholz; Hals: Balken durch die Stimmnaegel (Oese + Saitenlaenge senkrecht darueber)
           zum oberen Ende des Kastens. Querschnitte und Massen **angenommen** (Bell nennt nur das Material).
  Saiten   je Saite eine Feder E A / L laengs der Saite zwischen Oese (Kasten + Boden relativ) und Nagel am Hals, quer
           die Feder T / L; der Winkel Saite-Boden 33,2 Grad (sin^2 = 0,3 wie KQ52 a).
  Zug      die Saitenzuege (Bell: 9350 N) als statische Last; aus den Laengskraeften der Staebe die geometrische Steife.
  Die Harfe haengt frei (Bell S. 148: "suspended").

Je Fall die Moden 60-360 Hz mit Anteilen (Luft, Rahmen = Kasten + Sockel + Saeule + Hals, davon Kasten; erste und zweite
Plattenform), dazu ob sich der Kasten in einer Phase bewegt (Bell S. 155 zu B1 / B2).

Aufruf vom Projekt-Root: python -X utf8 -B <skript> <vak ohne Saiten> <gebaut> [raster]
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.linalg

HIER = pathlib.Path(__file__).resolve().parent


def _lade(name, datei):
    spec = importlib.util.spec_from_file_location(name, HIER / datei)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


k17 = _lade("k17", "2026-11-23_S217_harfe_kasten_balken.py")
sd = _lade("sd219", "2026-11-23_S214_harfe_saitendaempfung.py")
kb, ge, ro, kp = k17.kb, k17.ge, k17.ro, k17.kp
NE = k17.NE

WINKEL = np.arcsin(np.sqrt(0.3))                 # Saite gegen die Ebene des Bodens
T_AX = np.array([np.sin(WINKEL), np.cos(WINKEL)])    # laengs des Kastens, vom Bass zum Diskant (x waagrecht, z hoch)
N_AX = np.array([-np.cos(WINKEL), np.sin(WINKEL)])   # senkrecht zum Boden, zur Saitenseite
D_AX = np.array([0.0, 1.0])                      # Saiten senkrecht
X_AX = np.array([1.0, 0.0])
EA_KASTEN = 3.0e7                                # Schale + Randleisten, grob (N)

# angenommen: Schichtholz E 12 GPa, 750 kg/m^3
VORGABE = dict(e=12.0e9, rho=750.0,
               saeule_aussen=0.065, saeule_innen=0.030, saeule_x=-0.08,
               hals_dicke=0.040, hals_hoehe=0.090, naegel=1.5,      # kg Stimm- und Stegnaegel laengs des Halses
               sockel=3.0, ueber=0.08)                               # kg; Saeule ragt so weit ueber den tiefsten Nagel


def saiten(plan):
    """Je Saite vom Bass: y der Oese, E A / L, Zug T, Laenge L."""
    federn = sorted(plan["pointSprings"], key=lambda s: s["y"])
    ps = sd.ps
    lang = sorted([(hz, L) for hz, _, L in ps.HARFE_MONO] + [(hz, L) for hz, _, _, L in ps.HARFE_METALL])
    z = sd.wellenwiderstaende()
    assert len(federn) == len(lang) == len(z)
    return [(s["y"], s["stiffness"] / 0.3, q[2], l[1]) for s, q, l in zip(federn, z, lang)]


def stab(xi, xj, ea, ei, mu):
    """Ebener Balkenstab, global (X, Z, Theta) je Knoten: K, M, und die Drehung fuer die geometrische Steife."""
    d = xj - xi
    l = float(np.hypot(*d))
    c, s = d / l
    R = np.zeros((6, 6))
    for a in (0, 3):
        R[a:a + 3, a:a + 3] = [[c, s, 0], [-s, c, 0], [0, 0, 1]]
    k = np.zeros((6, 6)); m = np.zeros((6, 6))
    k[np.ix_([0, 3], [0, 3])] = ea / l * np.array([[1, -1], [-1, 1]])
    b = [1, 2, 4, 5]
    k[np.ix_(b, b)] = ei / l ** 3 * np.array([[12, 6 * l, -12, 6 * l], [6 * l, 4 * l * l, -6 * l, 2 * l * l],
                                               [-12, -6 * l, 12, -6 * l], [6 * l, 2 * l * l, -6 * l, 4 * l * l]])
    m[np.ix_([0, 3], [0, 3])] = mu * l / 6 * np.array([[2, 1], [1, 2]])
    m[np.ix_(b, b)] = mu * l / 420 * np.array([[156, 22 * l, 54, -13 * l], [22 * l, 4 * l * l, 13 * l, -3 * l * l],
                                               [54, 13 * l, 156, -22 * l], [-13 * l, -3 * l * l, -22 * l, 4 * l * l]])
    return R.T @ k @ R, R.T @ m @ R, R, l


def geometrisch(R, l, kraft):
    """Geometrische Steife eines Stabs mit Laengskraft `kraft` (Zug positiv), global."""
    b = [1, 2, 4, 5]
    g = np.zeros((6, 6))
    g[np.ix_(b, b)] = kraft / (30 * l) * np.array([[36, 3 * l, -36, 3 * l], [3 * l, 4 * l * l, -3 * l, -l * l],
                                                    [-36, -3 * l, 36, -3 * l], [3 * l, -l * l, -3 * l, 4 * l * l]])
    return R.T @ g @ R


class Rahmen:
    def __init__(self, vak, gebaut):
        self.ka = k17.Kasten(vak, gebaut)
        self.sa = saiten(self.ka.gp)

    def rechne(self, besaitet=False, zug=False, steife=1.0, rahmen=1.0, hals=1.0, starr=False, offen=True, feder=1.0,
               moden=38, **mehr):
        ka = self.ka
        p = dict(VORGABE, **mehr)
        v, gp, cav, L = ka.v, ka.gp, ka.cav, ka.L
        rho, c, V = v.rho, v.c, cav["volume"]
        le = L / NE
        # --- Knoten: Kasten 0..NE, Hals (Naegel vom Diskant zum Bass), Saeulenkopf, Saeule abwaerts, Fuss
        kn = [j * le * T_AX for j in range(NE + 1)]
        nagel = [y * T_AX + l * D_AX for y, _, _, l in self.sa]
        hals_kn = []
        for q in nagel[::-1]:
            hals_kn.append(len(kn)); kn.append(q)
        kopf = np.array([p["saeule_x"], nagel[0][1] + p["ueber"]])
        i_kopf = len(kn); kn.append(kopf)
        ns = 16
        saeule_kn = [i_kopf]
        for j in range(1, ns + 1):
            saeule_kn.append(len(kn)); kn.append(np.array([p["saeule_x"], kopf[1] * (1 - j / ns)]))
        kn = np.array(kn)
        nk = len(kn)
        nf = 3 * nk
        Kf = np.zeros((nf, nf)); Mf = np.zeros((nf, nf))
        staebe = []                                         # (dofs, R, l, EA) fuer die Laengskraefte

        def setze(i, j, ea, ei, mu):
            k, m, R, l = stab(kn[i], kn[j], ea, ei, mu)
            ix = np.r_[3 * i:3 * i + 3, 3 * j:3 * j + 3]
            Kf[np.ix_(ix, ix)] += k; Mf[np.ix_(ix, ix)] += m
            staebe.append((ix, R, l, ea))

        e = p["e"] * rahmen
        a_s = np.pi / 4 * (p["saeule_aussen"] ** 2 - p["saeule_innen"] ** 2)
        i_s = np.pi / 64 * (p["saeule_aussen"] ** 4 - p["saeule_innen"] ** 4)
        a_h = p["hals_dicke"] * p["hals_hoehe"]
        i_h = p["hals_dicke"] * p["hals_hoehe"] ** 3 / 12 * hals
        kette = [NE] + hals_kn + [i_kopf]
        lh = sum(np.hypot(*(kn[b] - kn[a])) for a, b in zip(kette[:-1], kette[1:]))
        for a, b in zip(kette[:-1], kette[1:]):
            setze(a, b, e * a_h, e * i_h, p["rho"] * a_h + p["naegel"] / lh)
        for a, b in zip(saeule_kn[:-1], saeule_kn[1:]):
            setze(a, b, e * a_s, e * i_s, p["rho"] * a_s)
        setze(saeule_kn[-1], 0, 1e9, 1e6, 0.0)                          # Sockel: steif
        for i in (saeule_kn[-1], 0):
            Mf[3 * i, 3 * i] += p["sockel"] / 2; Mf[3 * i + 1, 3 * i + 1] += p["sockel"] / 2
        # --- Kasten: quer aus der Probe S217, laengs als Staebe
        ei, mu = k17.querschnitt(gp, ka.ym, 6e9, True, steife)
        n = len(v.f)
        Kb, Mbal, Mbod, C = k17.balken(L, ei, mu, ka.P[:n], ka.mboden)
        nd = Kb.shape[0]
        Tb = np.zeros((nd, nf))
        for j in range(NE + 1):
            Tb[2 * j, 3 * j:3 * j + 2] = N_AX
            Tb[2 * j + 1, 3 * j + 2] = 1.0
        if starr:
            Kb = Kb * 1e4
        Kf += Tb.T @ Kb @ Tb
        Mf += Tb.T @ (Mbal + Mbod) @ Tb
        je = ro.ZELLEN // NE
        kasten_staebe = []
        for j in range(NE):
            ix = np.r_[3 * j:3 * j + 2, 3 * j + 3:3 * j + 5]
            tt = np.concatenate([-T_AX, T_AX])
            Kf[np.ix_(ix, ix)] += EA_KASTEN / le * np.outer(tt, tt)
            m_ax = mu[j] * le + ka.mboden[j * je:(j + 1) * je].sum()
            for a in (j, j + 1):
                Mf[np.ix_([3 * a, 3 * a + 1], [3 * a, 3 * a + 1])] += m_ax / 2 * np.outer(T_AX, T_AX)
            kasten_staebe.append((j, tt, ix))
        Mf[np.ix_([0, 1], [0, 1])] += k17.M_BLOCK_BASS * np.outer(T_AX, T_AX)
        # --- Luft und Platte wie in der Probe S217
        alpha = rho * c * c / V
        _, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
        oe = cav["openings"] if offen else []
        no = len(oe)
        Mo = ge.massen(rho, cav["openings"], 1.0, 1.0)[0] if no else np.zeros((0, 0))
        Pl = np.zeros((moden + 1, no))
        for i, o in enumerate(oe):
            sel = np.abs(y - o["y"]) <= ge.LAENGE[i] / 2
            Pl[:, i] = psi[:, sel].mean(axis=1)
        a0 = n + no
        b0 = a0 + moden
        N = b0 + nf
        K = np.zeros((N, N)); M = np.zeros((N, N))
        K[np.arange(n), np.arange(n)] = (2 * np.pi * v.f) ** 2
        Bn = ka.Bvol @ psi.T
        for j in range(moden + 1):
            cv = np.zeros(N); cv[:n] = Bn[:, j]; cv[n:a0] = Pl[j]
            if j >= 1:
                cv[a0 + j - 1] = -1.0
            K += alpha * np.outer(cv, cv)
        M[np.arange(n), np.arange(n)] = 1.0
        M[n:a0, n:a0] = Mo
        M[np.arange(a0, b0), np.arange(a0, b0)] = alpha / wn[1:] ** 2
        K[b0:, b0:] = Kf
        M[b0:, b0:] = Mf
        M[:n, b0:] = C @ Tb
        M[b0:, :n] = (C @ Tb).T
        # --- Saiten
        last = np.zeros(nf)
        zeilen = []
        for (ys, ea_l, zugkraft, ls), ih in zip(self.sa, hals_kn[::-1]):
            j = min(int(ys / le), NE - 1)
            xi = ys / le - j
            Nh = kb.hermite(xi, le)
            phi = np.array([np.interp(ys, ka.yz, ka.mitte[k]) for k in range(n)])
            for richt, kf in ((D_AX, feder * ea_l), (X_AX, zugkraft / ls)):
                a = np.zeros(N)
                a[:n] = (richt @ N_AX) * phi
                fr = np.zeros(nf)
                for kk, jj in ((0, j), (2, j + 1)):
                    fr[3 * jj:3 * jj + 2] += (richt @ N_AX) * Nh[kk] * N_AX
                    fr[3 * jj + 2] += (richt @ N_AX) * Nh[kk + 1]
                fr[3 * j:3 * j + 2] += (richt @ T_AX) * (1 - xi) * T_AX
                fr[3 * j + 3:3 * j + 5] += (richt @ T_AX) * xi * T_AX
                fr[3 * ih:3 * ih + 2] -= richt
                a[b0:] = fr
                zeilen.append((kf, a))
                if richt is D_AX:
                    last += zugkraft * fr
        if besaitet:
            for kf, a in zeilen:
                K += kf * np.outer(a, a)
        if zug:
            fest = [3 * saeule_kn[-1], 3 * saeule_kn[-1] + 1, 1]       # drei Auflager gegen Starrkoerperbewegung
            frei = np.setdiff1d(np.arange(nf), fest)
            u = np.zeros(nf)
            u[frei] = np.linalg.solve(Kf[np.ix_(frei, frei)], last[frei])
            Kg = np.zeros((nf, nf))
            for ix, R, l, ea in staebe:
                ul = R @ u[ix]
                Kg[np.ix_(ix, ix)] += geometrisch(R, l, ea * (ul[3] - ul[0]) / l)
            kraft_kasten = []
            for j, tt, ix in kasten_staebe:
                kr = EA_KASTEN / le * (tt @ u[ix])
                kraft_kasten.append(kr)
                gl = kr / (30 * le) * np.array([[36, 3 * le, -36, 3 * le], [3 * le, 4 * le * le, -3 * le, -le * le],
                                                [-36, -3 * le, 36, -3 * le], [3 * le, -le * le, -3 * le, 4 * le * le]])
                Kg += Tb[2 * j:2 * j + 4].T @ gl @ Tb[2 * j:2 * j + 4]
            K[b0:, b0:] += Kg
            self.kraefte = dict(saeule=min(ea * (R @ u[ix])[3] / l - ea * (R @ u[ix])[0] / l for ix, R, l, ea in staebe[len(kette) - 1:-1]),
                                kasten=min(kraft_kasten), summe=sum(q[2] for q in self.sa))
        lam, vec = scipy.linalg.eigh(K, M)
        f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
        aus = []
        kd = np.arange(b0, b0 + 3 * (NE + 1))
        for m in np.where((f > 40) & (f < 360))[0]:
            x = vec[:, m]
            lu = float(x[n:a0] @ Mo @ x[n:a0] + (x[a0:b0] ** 2 * np.diag(M)[a0:b0]).sum())
            ra = float(x[b0:] @ Mf @ x[b0:])
            kas = float(x[kd] @ M[np.ix_(kd, kd)] @ x[kd])
            wb = Tb[0::2] @ x[b0:]
            mitte = wb[10:-10]
            phase = abs(np.sum(np.sign(mitte))) / len(mitte)
            aus.append(dict(f=f[m], luft=lu, rahmen=ra, kasten=kas, p1=abs(x[0]), p2=abs(x[1]), phase=phase))
        return aus


def zeige(name, e):
    print(f"{name}")
    print("     " + "  ".join(f"{m['f']:5.1f} (L {m['luft']:.2f} R {m['rahmen']:.2f}/{m['kasten']:.2f} P1 {m['p1']:.2f} P2 {m['p2']:.2f}"
                              f"{' *' if m['phase'] > 0.98 and m['kasten'] > 0.15 else ''})" for m in e if m["f"] > 60))


def main():
    r = Rahmen(sys.argv[1], sys.argv[2])
    print("Bell Kasten G in der Harfe: ohne Saiten B1 85 (quer zur Ebene), B2 135, 170 / 216 / 249 / 316; "
          "besaitet B1 92, B2 ~110, 187 / 207 / 257 / 284 / 334 Hz")
    print("Angaben je Mode: Luft, Rahmen / davon Kasten, erste und zweite Plattenform; * = Kasten in einer Phase")
    was = sys.argv[3] if len(sys.argv) > 3 else "grund"
    faelle = [("Vorgabe", {})]
    if was == "raster":
        faelle += [("Kasten Steife x 0,7", dict(steife=0.7)), ("Kasten Steife x 1,4", dict(steife=1.4)),
                   ("Kasten starr", dict(starr=True)),
                   ("Rahmen E x 0,5", dict(rahmen=0.5)), ("Rahmen E x 2", dict(rahmen=2.0)),
                   ("Hals EI x 4", dict(hals=4.0)), ("Hals EI x 0,25", dict(hals=0.25)),
                   ("Saeule 50 / 25 mm", dict(saeule_aussen=0.050, saeule_innen=0.025)),
                   ("Saeule 80 / 30 mm", dict(saeule_aussen=0.080)),
                   ("Sockel 8 kg", dict(sockel=8.0)), ("Naegel 3 kg", dict(naegel=3.0)),
                   ("Federn x 2", dict(feder=2.0))]
    for name, kw in faelle:
        print(f"\n=== {name}")
        zeige("  ohne Saiten", r.rechne(**kw))
        zeige("  besaitet, nur Federn", r.rechne(besaitet=True, **kw))
        zeige("  besaitet, Federn und Zug", r.rechne(besaitet=True, zug=True, **kw))
        k = r.kraefte
        print(f"     Zug: Summe {k['summe']:.0f} N, groesster Druck Saeule {-k['saeule']:.0f} N, Kasten {-k['kasten']:.0f} N")


if __name__ == "__main__":
    main()

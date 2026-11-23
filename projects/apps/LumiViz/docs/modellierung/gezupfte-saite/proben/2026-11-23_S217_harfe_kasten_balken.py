"""S217 (KQ68 a) - Probe: der Kasten der Harfe als Balken mit der Biegesteife aus der Geometrie.

Die Balken-Probe aus S216 (`2026-11-23_S216_harfe_kasten_balken.py`) stellte die Biegesteife so, dass "die erste
Biegemode" bei 150 ... 5000 Hz liegt - sie griff dabei eine hohe Mode: der Balken mit "250 Hz" hat seine erste
Biegemode bei 2,3 Hz, die Steife war rund 10^4-mal zu klein. Ihr Befund (Harfenbericht 24.6) gilt nur fuer einen
schlaffen Kasten, in dem jeder Querschnitt fuer sich zurueckstoesst.

Hier: jeder Querschnitt bewegt sich als Ganzes mit w_b(y) (die Ring-Probe `2026-11-23_S217_harfe_ring.py` zeigt, dass
die Verformung des Querschnitts das Volumen nicht aendert). Biegesteife und Masse je Laenge aus den Bauteilen:

  Schale        Halbkreis, Radius = halbe Breite, 3 mm, E laengs als Stellgroesse (Fichtenlaminat)
  Randleisten   je Seite Rand- und Halteleiste (Bell 1988 Abb. 2.6), Buche, Querschnitt angenommen
  Innenleisten  zwei neben den Oeffnungen (Abb. 2.7 E), Querschnitt angenommen
  Boden         als Gurt: E laengs (ey des Bauplans) x Dicke x Breite; die Zugleiste als Gurt wahlweise
  Bloecke       Bodenblock und oberer Block als Massen an den Enden (angenommen)

Der Boden schwingt mit seinen Moden im Vakuum relativ zum Kasten (Flaechenmasse aus dem Bauplan: Dickenraster und
Leisten; geprueft an der modalen Masse 1 der Formen). Luft wie in S216 (Rohr, Massenmatrix der Oeffnungen).

Gemessen wird wie bei Bell (S. 106, 114 f.): das Paar f-, f+, die Bodenmode bei verschlossenen Oeffnungen f_zu,
daraus die Helmholtz-Mode ohne Kopplung f_H = sqrt(f-^2 + f+^2 - f_zu^2) und die Versteifung der Bodenmode durch die
eingeschlossene Luft f_a = sqrt((f_zu f_H)^2 - (f- f+)^2) / f_H. Bell: Kasten A lose 176 / 201, zu 191, f_a 26 Hz;
Kasten F lose 185 / 204, zu 196, f_a 24 Hz.

  raster <vak> <gebaut>        Stellgroessen x Halterung
  formen <vak> <gebaut> [E]    die Moden des Paars laengs: Boden relativ, Kasten, zusammen (gegen Abb. 5.4-5.8)

Aufruf vom Projekt-Root mit Python 3.12 (numpy, scipy). Die Platte im Vakuum unter `<vak>` traegt die Saiten als
Federn (S213).
"""
import importlib.util
import pathlib
import sys

import numpy as np
import scipy.linalg
import scipy.spatial

HIER = pathlib.Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("balken216", HIER / "2026-11-23_S216_harfe_kasten_balken.py")
kb = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(kb)
ge, ro, kp = kb.ge, kb.ro, kb.kp

NE = 131
T_SCHALE, RHO_SCHALE = 0.003, 480.0
A_RAND, E_RAND, RHO_RAND = 2 * 8.0e-4, 13.7e9, 750.0      # beide Seiten zusammen: Rand- und Halteleiste
A_INNEN, E_INNEN, RHO_INNEN = 2 * 2.0e-4, 10.0e9, 500.0
M_BLOCK_BASS, M_BLOCK_DISKANT = 1.2, 0.15
M_SPANTEN = 0.05                                          # je Spant, vier Stueck (Abb. 2.7 C)


def flaechenmasse(plan, xy, tri, w, L):
    """P_k je Zelle (Impuls der Form je Laenge), Masse des Bodens je Zelle, modale Massen zur Pruefung."""
    stoff = {m["name"]: m for m in plan["materials"]}
    g = plan["plate"]["thicknessGrid"]
    werte = np.array(g["values"]).reshape(g["ny"], g["nx"])
    a, b, c = xy[tri[:, 0]], xy[tri[:, 1]], xy[tri[:, 2]]
    fl = 0.5 * np.abs((b[:, 0] - a[:, 0]) * (c[:, 1] - a[:, 1]) - (c[:, 0] - a[:, 0]) * (b[:, 1] - a[:, 1]))
    xm, ym = (a[:, 0] + b[:, 0] + c[:, 0]) / 3, (a[:, 1] + b[:, 1] + c[:, 1]) / 3
    ix = np.clip(np.round((xm - g["origin"][0]) / g["step"]).astype(int), 0, g["nx"] - 1)
    iy = np.clip(np.round((ym - g["origin"][1]) / g["step"]).astype(int), 0, g["ny"] - 1)
    sigma = stoff[plan["plate"]["material"]]["density"] * werte[iy, ix]
    n = len(w)
    wt = w[:, tri].mean(axis=2)
    wt2 = (w[:, tri] ** 2).mean(axis=2)
    modal = (wt2 * (fl * sigma)).sum(axis=1)
    zelle = np.clip((ym / L * ro.ZELLEN).astype(int), 0, ro.ZELLEN - 1)
    P = np.zeros((n, ro.ZELLEN))
    m = np.zeros(ro.ZELLEN)
    for k in range(n):
        np.add.at(P[k], zelle, fl * sigma * wt[k])
    np.add.at(m, zelle, fl * sigma)
    baum = scipy.spatial.cKDTree(xy)
    for rib in plan["ribs"]:
        linie = np.array(rib["line"])
        s = np.concatenate([[0], np.cumsum(np.hypot(*np.diff(linie, axis=0).T))])
        ss = np.arange(0.0025, s[-1], 0.005)
        px, py = np.interp(ss, s, linie[:, 0]), np.interp(ss, s, linie[:, 1])
        hoehe = np.interp(ss, np.linspace(0, s[-1], len(rib["height"])) if len(rib["height"]) != len(s) else s, rib["height"])
        breite = np.interp(ss, np.linspace(0, s[-1], len(rib["width"])) if len(rib["width"]) != len(s) else s, rib["width"])
        dm = stoff[rib["material"]]["density"] * hoehe * breite * 0.005
        _, knoten = baum.query(np.column_stack([px, py]))
        z = np.clip((py / L * ro.ZELLEN).astype(int), 0, ro.ZELLEN - 1)
        for k in range(n):
            np.add.at(P[k], z, dm * w[k, knoten])
        modal += (w[:, knoten] ** 2 * dm).sum(axis=1)
        np.add.at(m, z, dm)
    return P, m, modal


def querschnitt(plan, ym, e_schale, gurt, steife=1.0):
    """EI und Masse je Laenge des Kastens ohne Boden (dessen Masse kommt ueber die Flaechenmasse)."""
    stoff = {m["name"]: m for m in plan["materials"]}
    R = ro.breite(plan, ym) / 2
    g = plan["plate"]["thicknessGrid"]
    werte = np.array(g["values"]).reshape(g["ny"], g["nx"])
    h = werte[np.clip(np.round(ym / g["step"]).astype(int), 0, g["ny"] - 1), g["nx"] // 2]
    teile = [  # (E A, z, E I_eigen)
        (e_schale * np.pi * R * T_SCHALE, -2 * R / np.pi, e_schale * R ** 3 * T_SCHALE * (np.pi / 2 - 4 / np.pi)),
        (E_RAND * A_RAND * np.ones_like(R), 0 * R, 0 * R),
        (E_INNEN * A_INNEN * np.ones_like(R), -R, 0 * R),
        (stoff["spruce"]["ey"] * h * 2 * R, 0 * R, 0 * R),
    ]
    if gurt:
        rib = plan["ribs"][0]
        s = np.array([p[1] for p in rib["line"]])
        fl = np.interp(ym, s, np.array(rib["height"]) * np.array(rib["width"]))
        teile.append((stoff[rib["material"]]["ex"] * fl, 0 * R, 0 * R))
    ea = sum(t[0] for t in teile)
    zs = sum(t[0] * t[1] for t in teile) / ea
    ei = sum(t[2] + t[0] * (t[1] - zs) ** 2 for t in teile)
    mu = RHO_SCHALE * np.pi * R * T_SCHALE + RHO_RAND * A_RAND + RHO_INNEN * A_INNEN
    return steife * ei, mu


def balken(L, ei, mu, P, mboden, masse_faktor=1.0):
    le = L / NE
    nd = 2 * (NE + 1)
    K = np.zeros((nd, nd)); M = np.zeros((nd, nd)); Mb = np.zeros((nd, nd)); C = np.zeros((P.shape[0], nd))
    ke = np.array([[12, 6 * le, -12, 6 * le], [6 * le, 4 * le ** 2, -6 * le, 2 * le ** 2],
                   [-12, -6 * le, 12, -6 * le], [6 * le, 2 * le ** 2, -6 * le, 4 * le ** 2]]) / le ** 3
    me = np.array([[156, 22 * le, 54, -13 * le], [22 * le, 4 * le ** 2, 13 * le, -3 * le ** 2],
                   [54, 13 * le, 156, -22 * le], [-13 * le, -3 * le ** 2, -22 * le, 4 * le ** 2]]) * le / 420
    je = ro.ZELLEN // NE
    for e in range(NE):
        ix = np.arange(2 * e, 2 * e + 4)
        K[np.ix_(ix, ix)] += ei[e] * ke
        M[np.ix_(ix, ix)] += masse_faktor * mu[e] * me
        for j in range(je):
            z = e * je + j
            N = kb.hermite((j + 0.5) / je, le)
            C[:, ix] += np.outer(P[:, z], N)
            Mb[np.ix_(ix, ix)] += mboden[z] * np.outer(N, N)
    M[0, 0] += masse_faktor * M_BLOCK_BASS
    M[nd - 2, nd - 2] += masse_faktor * M_BLOCK_DISKANT
    for y in (0.36, 0.58, 0.80, 1.02):
        k = 2 * int(round(y / le))
        M[k, k] += masse_faktor * M_SPANTEN
    return K, M, Mb, C


class Kasten:
    def __init__(self, vak, gebaut):
        self.v = kp.Vakuum(vak)
        self.gp = kp.lade(pathlib.Path(gebaut) / "concert_harp.lvinstbodyplan")
        self.cav = self.gp["cavity"]
        self.L = max(p[1] for p in self.gp["plate"]["outline"])
        self.Bvol = ro.volumen_laengs(pathlib.Path(vak) / "formen.txt", self.L)
        self.xy, self.tri, self.w = kb.lies_formen(pathlib.Path(vak) / "formen.txt", len(self.v.f))
        self.P, self.mboden, self.modal = flaechenmasse(self.gp, self.xy, self.tri, self.w, self.L)
        baum = scipy.spatial.cKDTree(self.xy)
        self.yz = (np.arange(ro.ZELLEN) + 0.5) * self.L / ro.ZELLEN
        _, kn = baum.query(np.column_stack([np.zeros(ro.ZELLEN), self.yz]))
        self.mitte = self.w[:, kn]                         # Formen auf der Mittellinie je Zelle
        self.ym = (np.arange(NE) + 0.5) * self.L / NE

    def system(self, e_schale, gurt, halt, offen=True, luft=True, steife=1.0, masse_faktor=1.0, starr=False, moden=38):
        v, cav, gp = self.v, self.cav, self.gp
        rho, c = v.rho, v.c
        V = cav["volume"]
        alpha = rho * c * c / V if luft else 0.0
        _, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
        oe = cav["openings"] if offen else []
        n, no = len(v.f), len(oe)
        Mo = ge.massen(rho, cav["openings"], 1.0, 1.0)[0] if no else np.zeros((0, 0))
        Pl = np.zeros((moden + 1, no))
        for i, o in enumerate(oe):
            sel = np.abs(y - o["y"]) <= ge.LAENGE[i] / 2
            Pl[:, i] = psi[:, sel].mean(axis=1)
        ei, mu = querschnitt(gp, self.ym, e_schale, gurt, steife)
        Kb, Mbal, Mbod, C = balken(self.L, ei, mu, self.P[:n], self.mboden, masse_faktor)
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
        Bn = self.Bvol @ psi.T
        for j in range(moden + 1):
            cv = np.zeros(N); cv[:n] = Bn[:, j]; cv[n:n + no] = Pl[j]
            if j >= 1:
                cv[n + no + j - 1] = -1.0
            K += alpha * np.outer(cv, cv)
        M[np.arange(n), np.arange(n)] = 1.0
        M[n:n + no, n:n + no] = Mo
        a0 = n + no
        M[np.arange(a0, a0 + moden), np.arange(a0, a0 + moden)] = (rho * c * c / V) / wn[1:] ** 2
        if not luft:
            K[np.arange(a0, a0 + moden), np.arange(a0, a0 + moden)] = 1.0     # Luft abgehaengt
        b0 = a0 + moden
        K[b0:, b0:] = Kb[np.ix_(ib, ib)]
        M[b0:, b0:] = (Mbal + Mbod)[np.ix_(ib, ib)]
        M[:n, b0:] = C[:, ib]
        M[b0:, :n] = C[:, ib].T
        lam, vec = scipy.linalg.eigh(K, M)
        f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
        aus = []
        for m in np.where((f > 90) & (f < 480))[0]:
            x = vec[:, m]
            lu = float(x[n:a0] @ Mo @ x[n:a0] + (x[a0:b0] ** 2 * np.diag(M)[a0:b0]).sum()) if luft else 0.0
            ka = float(x[b0:] @ M[b0:, b0:] @ x[b0:]) if nb else 0.0
            wb = np.zeros(nd); wb[ib] = x[b0:]
            aus.append(dict(f=f[m], luft=lu, kasten=ka, q=x[:n].copy(), wb=wb[0::2].copy()))
        leer = None
        if nb:
            l2 = scipy.linalg.eigh(Kb[np.ix_(ib, ib)], (Mbal + Mbod)[np.ix_(ib, ib)], eigvals_only=True)
            fl = np.sqrt(np.maximum(l2, 0.0)) / (2 * np.pi)
            leer = fl[fl > 1.0][:2]
        return aus, leer, float((mu * self.L / NE).sum() * masse_faktor + masse_faktor * (M_BLOCK_BASS + M_BLOCK_DISKANT + 4 * M_SPANTEN))

    def laengs(self, mode):
        """Boden relativ auf der Mittellinie, Kasten, je Zelle."""
        rel = mode["q"] @ self.mitte[:len(mode["q"])]
        kasten = np.interp(self.yz, np.linspace(0, self.L, NE + 1), mode["wb"])
        return rel, kasten


def bodenmoden(e, zahl=2):
    """Die Moden, in denen die erste Plattenform am staerksten ist (nach q_1^2), tiefste zuerst."""
    kand = sorted(e, key=lambda m: -m["q"][0] ** 2)[:zahl]
    return sorted(kand, key=lambda m: m["f"])


def bell(fm, fp, fzu):
    fh2 = fm ** 2 + fp ** 2 - fzu ** 2
    if fh2 <= 0:
        return float("nan"), float("nan")
    k4 = fzu ** 2 * fh2 - (fm * fp) ** 2
    return np.sqrt(fh2), np.sqrt(max(k4, 0.0)) / np.sqrt(fh2)


def liste(e, von=110.0, bis=320.0):
    return "  ".join(f"{m['f']:5.1f} (L {m['luft']:.2f} K {m['kasten']:.2f} P1 {abs(m['q'][0]):.2f})" for m in e if von < m["f"] < bis)


def zeile(name, ka, **kw):
    """Je Fall die Moden 110-320 Hz (Luftanteil, Kasten, Anteil der ersten Plattenform), ohne Zuordnung; dazu Bells
    Rechnung an den zwei Moden unter 240 Hz mit dem groessten Anteil der ersten Plattenform."""
    e, leer, masse = ka.system(**kw)
    zu, _, _ = ka.system(offen=False, **kw)
    lt = "starr" if leer is None else f"Kasten leer mit Boden {leer[0]:4.0f} Hz, {masse:.1f} kg"
    tief = [m for m in e if 120 < m["f"] < 240 and m["kasten"] < 0.5]
    tz = [m for m in zu if 120 < m["f"] < 260 and m["kasten"] < 0.5]
    text = ""
    if len(tief) >= 2 and tz:
        paar = sorted(sorted(tief, key=lambda m: -m["q"][0] ** 2)[:2], key=lambda m: m["f"])
        bz = max(tz, key=lambda m: m["q"][0] ** 2)
        fh, fa = bell(paar[0]["f"], paar[1]["f"], bz["f"])
        text = f" | Bell: {paar[0]['f']:.0f} / {paar[1]['f']:.0f}, zu {bz['f']:.0f}, f_H {fh:.0f}, f_a {fa:.0f} Hz"
    print(f"{name:26s}| {lt}{text}\n      offen: {liste(e)}\n      zu:    {liste(zu)}")
    return e


def main():
    was = sys.argv[1]
    ka = Kasten(sys.argv[2], sys.argv[3])
    print("modale Massen der ersten Formen aus der Flaechenmasse des Bauplans (soll 1):", " ".join(f"{x:.2f}" for x in ka.modal[:10]),
          f"| Boden {ka.mboden.sum():.2f} kg")
    print("Bell: Kasten A lose 176 / 201, zu 191, f_a 26 Hz | F lose 185 / 204, zu 196, f_a 24 Hz | G lose 153 / 200, in der Harfe 170 / 216,"
          " besaitet 187 / 207")
    if was == "raster":
        zeile("starrer Rahmen (heute)", ka, e_schale=6e9, gurt=True, halt="frei", starr=True)
        for halt in ("frei", "gehalten"):
            for steife in (0.5, 0.7, 1.0, 1.4, 2.0, 3.0, 4.0, 8.0, 1000.0):
                zeile(f"Steife x {steife:g}, {halt}", ka, e_schale=6e9, gurt=True, halt=halt, steife=steife)
        for mf in (0.6, 1.5):
            for halt in ("frei", "gehalten"):
                zeile(f"Masse x {mf}, {halt}", ka, e_schale=6e9, gurt=True, halt=halt, masse_faktor=mf)
        return
    if was == "formen":
        e_schale = float(sys.argv[4]) * 1e9 if len(sys.argv) > 4 else 6e9
        steife = float(sys.argv[5]) if len(sys.argv) > 5 else 1.0
        for halt in ("frei", "gehalten"):
            e, leer, masse = ka.system(e_schale=e_schale, gurt=True, halt=halt, steife=steife)
            print(f"\n{halt}: Moden 90-480 Hz (Luft, Kasten, staerkste Plattenformen)")
            for m in e:
                k = np.argsort(-np.abs(m["q"]))[:3]
                print(f"  {m['f']:6.1f} Hz  Luft {m['luft']:.2f}  Kasten {m['kasten']:.2f}  " + " ".join(f"P{i + 1} {m['q'][i]:+.2f}" for i in k))
            for m in [m for m in e if 150 < m["f"] < 320 and abs(m["q"][0]) > 0.25]:
                rel, kas = ka.laengs(m)
                s = np.abs(rel).max()
                print(f"  {m['f']:.1f} Hz laengs (y in m: Boden relativ / Kasten / zusammen, auf den groessten relativen Ausschlag bezogen)")
                for j in range(0, ro.ZELLEN, 13):
                    print(f"     {ka.yz[j]:.2f}: {rel[j] / s:+.2f} / {kas[j] / s:+.2f} / {(rel[j] + kas[j]) / s:+.2f}")
                ges = rel + kas
                wechsel = ka.yz[:-1][np.sign(ges[:-1]) != np.sign(ges[1:])]
                print("     Knoten der Bewegung des Bodens auf der Mittellinie bei y =", " ".join(f"{x:.2f}" for x in wechsel))
        return


if __name__ == "__main__":
    main()

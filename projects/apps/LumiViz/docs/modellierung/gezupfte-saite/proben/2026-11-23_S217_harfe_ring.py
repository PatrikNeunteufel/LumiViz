"""S217 (KQ68 a) - Probe: der Querschnitt der Harfe als Ring aus Boden und Schale.

Frage: schwaecht eine elastische Schale die Kopplung des Bodens an die eingeschlossene Luft? Gesucht ist in der
Energie rund ein Drittel (Harfenbericht 24.3, 24.4).

Modell: eine Scheibe des Kastens der Laenge 1 m laengs, ohne Halt laengs - Spanten, Bloecke und die Steife der Schale
laengs fehlen. Das ist die weichste denkbare Schale; was sie nicht kann, kann die gebaute erst recht nicht. Der
Querschnitt als ebener Rahmen (Balkenelemente mit Dehnung und Biegung):

  Boden      Streifen quer, Faser quer (Bauplan: ex 7,3 GPa, 500 kg/m^3), Dicke h, in der Mitte die Leisten als Masse
  Schale     Halbellipse, Halbachsen Breite / 2 und Tiefe; 3 mm Fichtenlaminat (E in Umfangsrichtung als Stellgroesse)
  Randleiste je Seite eine Masse am Stoss von Boden und Schale; der Boden sitzt dort gelenkig oder biegesteif
  Innenleisten zwei Massen auf der Schale neben den Oeffnungen

Groessen je Mode im Vakuum (auf die Masse 1 normiert): f, a = Aenderung der eingeschlossenen Flaeche (das Volumen je
Laenge), Anteil des Bodens an der Bewegungsenergie. Die Luft sieht den Kasten ueber sum a_m^2 / (w_m^2 - w^2); die
Versteifung einer Mode durch die eingeschlossene Luft ist f_a = a sqrt(rho c^2 / S) / 2 pi.

Faelle: Schale starr (heute), Raender gehalten, Ring frei.

  python <probe> [breite=0.30] [dicke=0.007] [tiefe=0.5]

Aufruf mit Python 3.12 (numpy, scipy). Massen der Leisten angenommen (Bell 1988 Abb. 2.6, 2.7 ohne Masse).
"""
import sys

import numpy as np
import scipy.linalg

RHO_LUFT, C_LUFT = 1.2, 343.0
E_BODEN, RHO_BODEN = 7.3e9, 500.0
T_SCHALE, RHO_SCHALE = 0.003, 480.0
M_MITTE = 0.87            # Zug- und Deckleiste, kg je m laengs (S216)
M_RAND = 0.60             # Randleiste und Halteleiste je Seite, angenommen
M_INNEN = 0.20            # Innenleiste je Seite, angenommen
NB, NS = 40, 80


def element(E, t, rho, l):
    """Rahmenelement je 1 m laengs, lokal (u1, v1, th1, u2, v2, th2)."""
    EA, EI, m = E * t, E * t ** 3 / 12, rho * t
    k = np.zeros((6, 6))
    k[np.ix_([0, 3], [0, 3])] = EA / l * np.array([[1, -1], [-1, 1]])
    kb = EI / l ** 3 * np.array([[12, 6 * l, -12, 6 * l], [6 * l, 4 * l * l, -6 * l, 2 * l * l],
                                 [-12, -6 * l, 12, -6 * l], [6 * l, 2 * l * l, -6 * l, 4 * l * l]])
    k[np.ix_([1, 2, 4, 5], [1, 2, 4, 5])] = kb
    mm = np.zeros((6, 6))
    mm[np.ix_([0, 3], [0, 3])] = m * l / 6 * np.array([[2, 1], [1, 2]])
    mb = m * l / 420 * np.array([[156, 22 * l, 54, -13 * l], [22 * l, 4 * l * l, 13 * l, -3 * l * l],
                                 [54, 13 * l, 156, -22 * l], [-13 * l, -3 * l * l, -22 * l, 4 * l * l]])
    mm[np.ix_([1, 2, 4, 5], [1, 2, 4, 5])] = mb
    return k, mm


def ring(breite, dicke, tiefe, e_schale, gelenk):
    """Knoten gegen den Uhrzeigersinn: Boden von rechts nach links, Schale von links unten herum nach rechts."""
    R, d = breite / 2, tiefe * breite
    xb = np.linspace(R, -R, NB + 1)
    pb = np.column_stack([xb, np.zeros(NB + 1)])
    th = np.linspace(np.pi, 2 * np.pi, NS + 1)
    ps = np.column_stack([R * np.cos(th), d * np.sin(th)])
    # Freiheitsgrade: Schale 3 je Knoten; Boden innen 3 je Knoten; Bodenenden teilen ux, uz mit dem Rand,
    # die Drehung nur ohne Gelenk
    ns_dof = 3 * (NS + 1)
    zaehler = [ns_dof]

    def neu(n):
        a = zaehler[0]
        zaehler[0] += n
        return list(range(a, a + n))

    rand_l, rand_r = [0, 1, 2], [3 * NS, 3 * NS + 1, 3 * NS + 2]
    bd = []
    for i in range(NB + 1):
        if i == 0:
            bd.append(rand_r[:2] + (rand_r[2:] if not gelenk else neu(1)))
        elif i == NB:
            bd.append(rand_l[:2] + (rand_l[2:] if not gelenk else neu(1)))
        else:
            bd.append(neu(3))
    sd = [[3 * i, 3 * i + 1, 3 * i + 2] for i in range(NS + 1)]
    N = zaehler[0]
    K = np.zeros((N, N)); M = np.zeros((N, N)); A = np.zeros(N)
    teil = np.zeros(N, int)                      # 0 Schale, 1 Boden (nach dem Massenanteil unten)
    Mb = np.zeros((N, N))

    def baue(p, dofs, E, t, rho, boden):
        for i in range(len(p) - 1):
            dx, dz = p[i + 1] - p[i]
            l = np.hypot(dx, dz)
            c, s = dx / l, dz / l
            T = np.zeros((6, 6))
            for o in (0, 3):
                T[o:o + 3, o:o + 3] = [[c, s, 0], [-s, c, 0], [0, 0, 1]]
            k, m = element(E, t, rho, l)
            ix = dofs[i] + dofs[i + 1]
            K[np.ix_(ix, ix)] += T.T @ k @ T
            M[np.ix_(ix, ix)] += T.T @ m @ T
            if boden:
                Mb[np.ix_(ix, ix)] += T.T @ m @ T
            # Flaeche: aussen = -lokal quer; Int v ds = l/2 (v1 + v2) + l^2/12 (th1 - th2)
            lok = np.array([0, l / 2, l * l / 12, 0, l / 2, -l * l / 12])
            A[ix] += -(lok @ T)

    baue(pb, bd, E_BODEN, dicke, RHO_BODEN, True)
    baue(ps, sd, e_schale, T_SCHALE, RHO_SCHALE, False)
    mitte = bd[NB // 2]
    for dof in mitte[:2]:
        M[dof, dof] += M_MITTE
        Mb[dof, dof] += M_MITTE
    for r in (rand_l, rand_r):
        for dof in r[:2]:
            M[dof, dof] += M_RAND
    # Innenleisten 4 cm neben dem Scheitel
    s_bogen = np.concatenate([[0], np.cumsum(np.hypot(*np.diff(ps, axis=0).T))])
    for ziel in (s_bogen[-1] / 2 - 0.04, s_bogen[-1] / 2 + 0.04):
        i = int(np.argmin(np.abs(s_bogen - ziel)))
        for dof in sd[i][:2]:
            M[dof, dof] += M_INNEN
    flaeche = np.pi * R * d / 2
    schale_dofs = np.arange(ns_dof)
    boden_dofs = np.array(sorted(set(sum(bd, [])) - set(rand_l[:2] + rand_r[:2]) - (set() if gelenk else set(rand_l[2:] + rand_r[2:]))))
    return dict(K=K, M=M, Mb=Mb, A=A, S=flaeche, schale=schale_dofs, boden=boden_dofs, rand=np.array(rand_l[:2] + rand_r[:2]),
                rand_dreh=np.array(rand_l[2:] + rand_r[2:]), N=N)


def moden(r, frei_dofs, bis=900.0):
    K, M = r["K"][np.ix_(frei_dofs, frei_dofs)], r["M"][np.ix_(frei_dofs, frei_dofs)]
    lam, vec = scipy.linalg.eigh(K, M)
    f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
    A = r["A"][frei_dofs]
    Mb = r["Mb"][np.ix_(frei_dofs, frei_dofs)]
    aus = []
    for m in np.where((f > 5.0) & (f < bis))[0]:
        x = vec[:, m]
        aus.append((f[m], float(A @ x), float(x @ Mb @ x)))
    return aus


def zeige(name, r, frei_dofs, kappa, a0=None):
    e = moden(r, frei_dofs)
    zeilen = [m for m in e if abs(m[1]) > 1e-4]
    text = "  ".join(f"{f:5.0f} Hz a {a:+.3f} (f_a {abs(a) * np.sqrt(kappa) / (2 * np.pi):4.0f} Hz, Boden {b:.2f})" for f, a, b in zeilen[:6])
    haupt = max(zeilen, key=lambda m: m[2] * (m[0] < 700)) if zeilen else None
    extra = ""
    if haupt and a0:
        extra = f" | Bodenmode a^2 / a0^2 = {haupt[1] ** 2 / a0 ** 2:.2f}"
    print(f"  {name:38s}{extra}\n      {text}")
    return haupt


def helmholtz(r, kappa):
    """Nachgiebigkeit der Schale allein unter Druck (Boden und Raender fest) gegen die der Luft."""
    frei = np.setdiff1d(r["schale"], np.concatenate([r["rand"], r["rand_dreh"]]))
    K = r["K"][np.ix_(frei, frei)]
    A = r["A"][frei]
    c_wand = float(A @ np.linalg.solve(K, A))
    return 1.0 / np.sqrt(1.0 + c_wand * kappa)


def main():
    arg = dict(a.split("=") for a in sys.argv[1:])
    dicke = float(arg.get("dicke", 0.007))
    tiefe = float(arg.get("tiefe", 0.5))
    breiten = [float(arg["breite"])] if "breite" in arg else [0.40, 0.30, 0.20]
    print(f"Scheibe 1 m, Boden {1000 * dicke:.0f} mm, Schale {1000 * T_SCHALE:.0f} mm, Tiefe {tiefe:.2f} x Breite;"
          f" a in m^2 je Wurzel(kg/m); f_a = Versteifung durch die eingeschlossene Luft")
    for breite in breiten:
        for gelenk in (True, False):
            print(f"\nBreite {breite:.2f} m, Boden am Rand {'gelenkig' if gelenk else 'biegesteif'}")
            a0 = None
            for e_schale in (1e13, 2e9, 5e9, 10e9):
                r = ring(breite, dicke, tiefe, e_schale, gelenk)
                kappa = RHO_LUFT * C_LUFT ** 2 / r["S"]
                alle = np.arange(r["N"])
                if e_schale > 1e12:
                    frei = np.setdiff1d(alle, np.concatenate([r["schale"]]))
                    if not gelenk:
                        frei = np.setdiff1d(frei, r["rand_dreh"])
                    h = zeige("Schale starr (heute)", r, frei, kappa)
                    a0 = h[1]
                    continue
                print(f"  -- Schale E = {e_schale / 1e9:.0f} GPa; Helmholtz-Mode durch die nachgiebige Schale x {helmholtz(r, kappa):.4f}")
                zeige("Raender gehalten", r, np.setdiff1d(alle, r["rand"]), kappa, a0)
                zeige("Ring frei", r, alle, kappa, a0)


if __name__ == "__main__":
    main()

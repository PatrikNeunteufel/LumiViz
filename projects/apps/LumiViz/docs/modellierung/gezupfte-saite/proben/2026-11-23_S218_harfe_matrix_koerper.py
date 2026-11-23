"""S218 (KQ71 a) - Probe: die Harfe mit Massen- und Strahlungsmatrix der Oeffnungen als Koerper, an allen Zielgroessen.

KQ71 a (Patrik S217): bevor die Massenmatrix der Oeffnungen (Harfenbericht 24.2) und ihr Strahlungswiderstand als
Matrix (25.6) in `Luftraum.hpp` kommen, werden Lautheit der Tasten 50-62, Abstrahlung an den Saiten 33-22, Gueten und
der Waechter gegen Bells 36 Werte fuer diese Rechnung bestimmt.

Das System wie in den Proben S216 / S217: Plattenmoden im Vakuum (q_k), je Oeffnung ein Pfropfen (u_i, Volumen nach
aussen) mit der Massenmatrix aus der Geometrie, das Rohr laengs (s_n), starre Waende; der Druck am Pfropfen ueber die
Laenge der Oeffnung gemittelt. Kein angepasster Wert an der Luft (keine Muendung, keine Guete der Luft).

Verlust je ungedaempfter Mode m (massennormiert, Frequenz w_m) in erster Ordnung:
    eta_m = Platte (wie `Luftraum.hpp`: Anteil der Platte x Gesetz des Bauplans x (1 - Federn))
          + u^T R(w_m) u / w_m                         Abstrahlung der Oeffnungen als Matrix
          + (Pfropfen- und Rohranteil) / Q_innen(f_m)   innerer Verlust
Q_innen aus Bells Gueten mit nur Oeffnung 6 offen (Abb. 4.8, 4.11-4.15), **abzueglich der Abstrahlung der Oeffnung 6**
(die Probe S217 nahm Bells Gueten ganz als inneren Verlust).

Zwei Faelle, je in sich gleich: 'wand' (Kolben in einer unendlichen Wand: Masse aussen kappa 1, Strahlungswiderstand
rho c k^2 / 2 pi, Schall an den Hoerorten sqrt 2 x rho / 4 pi, damit die Leistung zum Widerstand passt) und 'frei' (freier Raum: kappa 0,5 und eigene Korrektur 0,72,
Widerstand halb, Schall rho / 4 pi wie `Luftraum.hpp` heute).

  rohr <vak> <gebaut>                    Luft allein, starre Waende: Bells 36 Lagen, Gueten alle offen, Q_innen
  guete <vak> <gebaut>                   Plattenmoden S(1,1)-S(1,5): Guete erster Ordnung gegen komplexe Eigenwerte
  schreibe <vak> <gebaut> <ziel> <fall>  Koerper nach <ziel> (Lautheit mit 2026-11-19_S208_harfe_mitschwingen_16.py)

<vak> = Platte im Vakuum (mit Saiten `.claude/handover/s213/vak`, ohne `.claude/handover/s217/vak_ohne`);
<gebaut> = gebauter Koerper mit Rohr und 16 Hoerorten (`.claude/handover/s215/laut`). Aufruf vom Projekt-Root mit
Python 3.12 (numpy, scipy).
"""
import importlib.util
import json
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


ge = _lade("gegen", "2026-11-23_S216_harfe_oeffnungen_gegenseitig.py")
dk = _lade("dkomplex", "2026-11-23_S217_harfe_daempfung_komplex.py")
ro, kp, wa = ge.ro, ge.kp, ge.wa

# feld: Faktor auf rho / 4 pi an den Hoerorten. In der Wand strahlt ein Kolben die doppelte Leistung (Widerstand x 2);
# die Hoerorte liegen rundum, die Richtung der Wand ist unbekannt -> gleichmaessig sqrt 2, damit das Leistungsmittel
# ueber die Hoerorte zum Widerstand passt (ein Faktor 2 ueberall gaebe die vierfache Leistung).
FAELLE = {"wand": {"kappa": 1.0, "selbst": 1.0, "rad": 1.0, "feld": 2.0 ** 0.5},
          "frei": {"kappa": 0.5, "selbst": 0.72, "rad": 0.5, "feld": 1.0}}

# Bell Abb. 4.8, 4.11-4.15 (Harfenbericht 24.1): offen -> Helmholtz, A1-A5; Oeffnungen im Bauplan 1-5, dann 6
BELL_36 = [((5,), [75, 168, 283, 414, 545, 680]),
           ((4, 5), [92.5, 220, 364, 474, 593, 691]),
           ((3, 4, 5), [100, 250, 400, 490, 600, 690]),
           ((2, 3, 4, 5), [123, 290, 403, 507, 617, 707]),
           ((1, 2, 3, 4, 5), [164, 277, 422, 520, 610, 717]),
           ((0, 1, 2, 3, 4, 5), [190, 292, 423, 530, 620, 720])]
Q6_F, Q6 = dk.Q_INNEN_F, dk.Q_INNEN            # nur Oeffnung 6 offen: 75 ... 680 Hz, Q 10 ... 36
QA_F, QA = dk.BELL_ALLE_F, dk.BELL_ALLE_Q      # alle offen
BELL_OHNE, BELL_MIT = dk.BELL_OHNE, dk.BELL_MIT


class System:
    """K, M des gekoppelten Systems; offen = Indizes der offenen Oeffnungen; platte = False: Luft allein."""

    def __init__(self, v, gp, fall, offen=None, platte=True, Bvol=None):
        cav = gp["cavity"]
        self.v, self.fall = v, FAELLE[fall]
        rho, c = v.rho, v.c
        V = cav["volume"]
        self.alpha = rho * c * c / V
        moden = int(np.sum(np.arange(200) * c / (2 * max(p[1] for p in gp["plate"]["outline"])) <= max(v.f))) - 1
        _, y, wn, psi = ro.rohrmoden(gp, V, c, moden)
        self.offen = list(range(len(cav["openings"]))) if offen is None else list(offen)
        self.oe = [cav["openings"][i] for i in self.offen]
        alle_M = ge.massen(rho, cav["openings"], self.fall["kappa"], 1.0, self.fall["selbst"])[0]
        Mo = alle_M[np.ix_(self.offen, self.offen)]
        n = len(v.f) if platte else 0
        no = len(self.oe)
        Pl = np.zeros((moden + 1, no))
        for i, k in enumerate(self.offen):
            sel = np.abs(y - cav["openings"][k]["y"]) <= ge.LAENGE[k] / 2 + 1e-12
            Pl[:, i] = psi[:, sel].mean(axis=1)
        N = n + no + moden
        K = np.zeros((N, N))
        M = np.zeros((N, N))
        K[np.arange(n), np.arange(n)] = (2 * np.pi * v.f[:n]) ** 2
        Bn = Bvol[:n] @ psi.T if n else np.zeros((0, moden + 1))
        Cv = np.zeros((moden + 1, N))
        for j in range(moden + 1):
            Cv[j, :n] = Bn[:, j]
            Cv[j, n:n + no] = Pl[j]
            if j >= 1:
                Cv[j, n + no + j - 1] = -1.0
        K += self.alpha * Cv.T @ Cv
        M[np.arange(n), np.arange(n)] = 1.0
        M[n:n + no, n:n + no] = Mo
        M[np.arange(n + no, N), np.arange(n + no, N)] = self.alpha / wn[1:] ** 2
        self.K, self.M, self.n, self.no, self.N, self.moden = K, M, n, no, N, moden
        lam, vec = scipy.linalg.eigh(K, M)
        self.f = np.sqrt(np.maximum(lam, 0.0)) / (2 * np.pi)
        self.x = vec                                           # M-normiert

    def R(self, w):
        return dk.strahlung(self.v.rho, self.v.c, self.oe, w, self.fall["rad"] == 1.0)

    def anteile(self, m):
        x, n, a0 = self.x[:, m], self.n, self.n + self.no
        pf = float(x[n:a0] @ self.M[n:a0, n:a0] @ x[n:a0])
        rr = float((x[a0:] ** 2 * np.diag(self.M)[a0:]).sum())
        return float((x[:n] ** 2).sum()), pf, rr

    def eta_luft(self, m, qinnen):
        """Abstrahlung der Oeffnungen und innerer Verlust in erster Ordnung (ohne Platte)."""
        w = 2 * np.pi * self.f[m]
        u = self.x[self.n:self.n + self.no, m]
        _, pf, rr = self.anteile(m)
        return float(u @ self.R(w) @ u) / w, (pf + rr) / qinnen(self.f[m])

    def daempfung(self, w0, qinnen, eta_platte):
        """Dieselben Verluste als Matrix bei w0 (fuer die komplexen Eigenwerte)."""
        n, a0, N = self.n, self.n + self.no, self.N
        C = np.zeros((N, N))
        C[np.arange(n), np.arange(n)] = eta_platte * w0
        qi = qinnen(w0 / (2 * np.pi))
        C[n:a0, n:a0] = self.R(w0) + w0 * self.M[n:a0, n:a0] / qi
        C[np.arange(a0, N), np.arange(a0, N)] = w0 * np.diag(self.M)[a0:] / qi
        return C


def q_innen(v, gp, fall, zeigen=False):
    """Innerer Verlust: Bells Gueten mit nur Oeffnung 6 offen, abzueglich der Abstrahlung der Oeffnung 6 im Modell."""
    s = System(v, gp, fall, offen=[5], platte=False)
    inv = []
    zeilen = []
    for fb, qb in zip(Q6_F, Q6):
        m = int(np.argmin(np.abs(s.f - fb)))
        w = 2 * np.pi * s.f[m]
        u = s.x[s.n:s.n + s.no, m]
        q_rad = w / float(u @ s.R(w) @ u)
        rest = 1.0 / qb - 1.0 / q_rad
        inv.append(max(rest, 0.0))
        zeilen.append(f"{fb:.0f} Hz (Modell {s.f[m]:.0f}): Q Bell {qb:.0f}, Abstrahlung 6 allein Q {q_rad:.0f}, innen Q "
                      + (f"{1 / rest:.1f}" if rest > 0 else "-- (Abstrahlung allein schon zu viel)"))
    inv = np.array(inv)
    if zeigen:
        print(f"  innerer Verlust ({fall}): " + "; ".join(zeilen))

    def qi(f):
        return 1.0 / max(np.interp(f, Q6_F, inv), 1e-6)
    return qi


def rohr_main(v, gp):
    for fall in FAELLE:
        print(f"\n== Luft allein, starre Waende, {fall}")
        qi = q_innen(v, gp, fall, zeigen=True)
        fehl = []
        for offen, bell in BELL_36:
            s = System(v, gp, fall, offen=offen, platte=False)
            fm = s.f[s.f > 20][:6]
            fehl += list(fm / np.array(bell) - 1)
            print(f"  offen {','.join(str(i + 1) for i in offen):12s} " + " ".join(f"{x:6.1f}" for x in fm)
                  + "  | Bell " + " ".join(f"{x:5.0f}" for x in bell) + "  | " + " ".join(f"{100 * (a / b - 1):+5.1f}" for a, b in zip(fm, bell)) + " %")
        fehl = np.array(fehl)
        print(f"  36 Werte rms {100 * np.sqrt(np.mean(fehl ** 2)):.1f} %, groesste {100 * np.max(np.abs(fehl)):.1f} %")
        s = System(v, gp, fall, platte=False)
        zeilen = []
        hoerbar = np.where(s.f > 20)[0]
        for i, (fb, qb) in enumerate(zip(QA_F, QA)):
            m = int(hoerbar[i])                       # der Reihe nach, wie die 36 Lagen
            er, ei = s.eta_luft(m, qi)
            w0 = 2 * np.pi * s.f[m]
            ew, _ = dk.komplex(s.K, s.M, s.daempfung(w0, qi, np.zeros(0)), np.arange(s.N))
            k = np.argmin(np.abs(ew.imag - w0) + 1e9 * (ew.imag <= 0))
            zeilen.append(f"{s.f[m]:.0f} Hz Q {1 / (er + ei):4.1f} (Abstr. {1 / er:5.1f}, innen {1 / ei:5.1f}; komplex {ew[k].imag / (-2 * ew[k].real):4.1f}) Bell {qb:.0f}")
        print("  Gueten alle offen, erste Ordnung:\n    " + "\n    ".join(zeilen))


def platte_eta(v, x, f):
    """Verlust der Platte wie `Luftraum.hpp` / Probe S214: Gesetz bei f, Federn nach Verformungsenergie, x = Anteile."""
    en = x ** 2 * (2 * np.pi * v.f) ** 2
    en = en / max(en.sum(), 1e-300)
    dm = v.k["damping"]
    gesetz = (dm["a"] * f ** 2 + dm["b"] * f + dm["c"]) / (2 * np.pi * max(f, 1e-9))
    return float((x ** 2).sum()) * (1 - float(en @ v.feder)) * gesetz, en


def guete_main(v, gp, Bvol):
    for fall in FAELLE:
        qi = q_innen(v, gp, fall)
        s = System(v, gp, fall, Bvol=Bvol)
        print(f"\n== {fall}: je Plattenform k die Moden mit Anteil > 0,15 -> f, Platte / Pfropfen / laengs, "
              "Q erster Ordnung (Platte nach v.eta wie im komplexen Fall) gegen komplex")
        for k in range(5):
            kand = [m for m in range(s.N) if 100 < s.f[m] < 800 and s.x[k, m] ** 2 > 0.15]
            for m in kand:
                pl, pf, rr = s.anteile(m)
                er, ei = s.eta_luft(m, qi)
                ep = float((s.x[:s.n, m] ** 2 * v.eta).sum())
                w0 = 2 * np.pi * s.f[m]
                ew, ev = dk.komplex(s.K, s.M, s.daempfung(w0, qi, v.eta), np.arange(s.N))
                j = np.argmin(np.abs(ew - (-0.5 * w0 * (ep + er + ei) + 1j * w0)))
                print(f"  S(1,{k + 1}) {s.f[m]:6.1f} Hz  Platte {pl:.2f} Pfropfen {pf:.2f} laengs {rr:.2f}  "
                      f"Q {1 / (ep + er + ei):5.1f} (Platte {1 / ep:5.1f}, Abstr. {1 / max(er, 1e-12):6.1f}, innen {1 / max(ei, 1e-12):6.1f})"
                      f"  komplex {ew[j].imag / (2 * np.pi):6.1f} Hz Q {ew[j].imag / (-2 * ew[j].real):5.1f}"
                      f"  | Bell ohne {BELL_OHNE[k + 1]}, besaitet {BELL_MIT[k + 1]}")


def koerper_aus(v, gp, gk, fall, Bvol):
    qi = q_innen(v, gp, fall)
    s = System(v, gp, fall, Bvol=Bvol)
    n, no, a0 = s.n, s.no, s.n + s.no
    rho, c = v.rho, v.c
    R = np.linalg.norm(v.orte - v.mitte, axis=1)
    rl = [np.linalg.norm(v.orte - np.array([o["x"], o["y"], o["z"]]), axis=1) for o in s.oe]
    modes = []
    for m in range(s.N):
        f = s.f[m]
        if f <= 30.0:
            continue
        x = s.x[:, m]
        Cp = x[:n]
        pl, pf, rr = s.anteile(m)
        ep, en = platte_eta(v, Cp, f)
        er, ei = s.eta_luft(m, qi)
        dkk = 2 * np.pi * (f - v.f) / c
        G = (Cp[:, None] * v.G * np.exp(-1j * np.outer(dkk, R))).sum(axis=0)
        k = 2 * np.pi * f / c
        for i, r in enumerate(rl):
            G = G + s.fall["feld"] * rho / (4 * np.pi) * x[n + i] / r * np.exp(-1j * k * r)
        modes.append({"airShare": pf + rr, "bridge": [float(b) for b in Cp @ v.steg], "bridgeAngle": 0,
                      "eta": ep + er + ei, "f": float(f), "materialShares": [float(b) for b in en @ v.stoff],
                      "radiation": [[float(g.real), float(g.imag)] for g in G], "springShare": float(en @ v.feder),
                      "tubeShare": rr})
    k = json.loads(json.dumps(gk))
    k["modes"] = modes
    return k


def main():
    was = sys.argv[1]
    v = kp.Vakuum(sys.argv[2])
    gebaut = pathlib.Path(sys.argv[3])
    gp = kp.lade(gebaut / "concert_harp.lvinstbodyplan")
    L = max(p[1] for p in gp["plate"]["outline"])
    Bvol = ro.volumen_laengs(pathlib.Path(sys.argv[2]) / "formen.txt", L)
    print(f"Platte im Vakuum {' / '.join(f'{x:.1f}' for x in v.f[:5])} Hz, {len(v.f)} Moden bis {v.f[-1]:.0f} Hz")
    if was == "rohr":
        rohr_main(v, gp)
    elif was == "guete":
        guete_main(v, gp, Bvol)
    elif was == "schreibe":
        zi, fall = pathlib.Path(sys.argv[4]), sys.argv[5]
        gk = kp.lade(gebaut / "concert_harp.lvinstbody")
        k = koerper_aus(v, gp, gk, fall, Bvol)
        zi.mkdir(parents=True, exist_ok=True)
        (zi / "concert_harp.lvinstbody").write_text(json.dumps(k) + "\n", encoding="utf-8", newline="\n")
        (zi / "concert_harp.lvinstbodyplan").write_text(json.dumps(gp, indent=1) + "\n", encoding="utf-8", newline="\n")
        e = {"helmholtz": float("nan"), "fc": float("nan")}
        kp.zeile(f"Matrix {fall}", e, k, gp)
        print(f"  {len(k['modes'])} Moden nach {zi}")


if __name__ == "__main__":
    main()

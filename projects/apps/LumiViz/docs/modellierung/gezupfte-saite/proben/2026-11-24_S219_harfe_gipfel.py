"""S219 (KQ70 a) - Probe: die Harfe so gemessen, wie Bell misst - Gipfel der Eingangsadmittanz je Messort.

Bell 1988 Kap. 7 liest die Lagen der Moden aus Kurven der Eingangsadmittanz an 40 Messorten laengs der Deckleiste
(Abb. 7.6, 7.8-7.16). Besaitet sind S(1,1)- und S(1,1)+ dort flach (Abb. 7.8 / 7.9: Guete "indeterminate"), die Saiten
sind mit Band gedaempft. Das Modell vergleicht bisher Eigenfrequenzen. Diese Probe legt beide auf dieselbe Messkette:

    (w_n^2 (1 + i eta_n) - w^2) q_n + i w sum_m C_nm q_m = phi_n(p) F,   Y(p) = i w sum_n phi_n(p) q_n / F
    C_nm = sum_Saiten 0,67 Z0_i phi_n(y_i) phi_m(y_i)      (Saiten als Daempfer, Probe S214; 0 = ohne)
    p(Ort) / F = - w^2 sum_n G_n q_n / F                    (Schall je Kraft am Hoerort)

Je Messort die oertlichen Maxima von |Y| in 120-360 Hz; dazu, wie viele Messorte einen Gipfel je 5-Hz-Band zeigen,
das Profil von |Y| und des Schalls laengs der Messorte bei gewaehlten Frequenzen (Bell: 170 / 216 ohne, 187 / 207
besaitet) und die Kurve am Messort 17 (Abb. 7.6).

Aufruf (vom Projekt-Root): python -X utf8 -B <skript> <ordner mit Koerper und Bauplan> [daempfer=1] [f=187,207]
       [hebe=<Hz>:<dB>[:<Q>],...]   die Mode bei <Hz> in |Y| um <dB> staerker (und mit Guete Q) - Gegenprobe: wie liest
       man das Paar, wenn S(1,2) so hoch steht wie bei Bell (Abb. 7.10: 93 dB, Q 32)?
Messorte nach Abb. 6.3: y = 0,035 (40 - Ort) - 0,057 m.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("sd", HIER / "2026-11-23_S214_harfe_saitendaempfung.py")
sd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sd)


def lade(o):
    k = json.loads((o / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
    p = json.loads((o / "concert_harp.lvinstbodyplan").read_text(encoding="utf-8"))
    ys = {}
    for i, b in enumerate(p["bridgePoints"]):
        ys.setdefault(round(b["y"], 4), i)
    yy = np.array(sorted(ys))
    idx = [ys[y] for y in yy]
    m = [q for q in k["modes"] if q["f"] < 2500]
    f = np.array([q["f"] for q in m])
    eta = np.array([q["eta"] for q in m])
    phi = np.array([np.array(q["bridge"])[idx] for q in m])              # Mode x Ort
    G = np.array([[complex(a, b) for a, b in q["radiation"]] for q in m])  # Mode x Hoerort
    federn = sorted(p["pointSprings"], key=lambda s: s["y"])
    return f, eta, phi, G, yy, np.array([s["y"] for s in federn])


def antwort(f, eta, phi, G, C, fr):
    """q je Frequenz und Anregungsort: (len(fr), Mode, Ort)."""
    w0 = 2 * np.pi * f
    Y = np.empty((len(fr), phi.shape[1]), complex)
    P = np.empty((len(fr), phi.shape[1], G.shape[1]), complex)
    for i, x in enumerate(fr):
        w = 2 * np.pi * x
        A = np.diag(w0 ** 2 * (1 + 1j * eta) - w * w) + 1j * w * C
        q = np.linalg.solve(A, phi)                                     # Mode x Ort
        Y[i] = 1j * w * np.einsum("mp,mp->p", phi, q)
        P[i] = -w * w * (q.T @ G)
    return Y, P


def gipfel(db, fr, tiefe=1.0):
    """Oertliche Maxima, die beidseits um mindestens `tiefe` dB abfallen, bevor die Kurve hoeher steigt."""
    out = []
    for i in range(1, len(db) - 1):
        if db[i] >= db[i - 1] and db[i] > db[i + 1]:
            li = db[i] - db[:i][::-1]
            re = db[i] - db[i + 1:]
            lo = np.argmax(li < 0) if np.any(li < 0) else len(li)
            ro = np.argmax(re < 0) if np.any(re < 0) else len(re)
            tl = li[:lo].max() if lo else 0.0
            tr = re[:ro].max() if ro else 0.0
            if min(tl, tr) >= tiefe:
                out.append((fr[i], db[i], min(tl, tr)))
    return out


def main():
    o = pathlib.Path(sys.argv[1])
    opt = dict(a.split("=") for a in sys.argv[2:])
    f, eta, phi, G, yy, ys = lade(o)
    for teil in [t for t in opt.get("hebe", "").split(",") if t]:      # hebe=<Hz>:<dB>[:<Q>] - eine Mode staerker
        hz, dbp, *q = [float(x) for x in teil.split(":")]
        m = int(np.argmin(np.abs(f - hz)))
        phi[m] *= 10 ** (dbp / 40)
        G[m] *= 10 ** (dbp / 40)
        if q:
            eta[m] = 1 / q[0]
    C = np.zeros((len(f), len(f)))
    if opt.get("daempfer", "0") == "1":
        Z0 = np.array([q[1] for q in sd.wellenwiderstaende()])
        assert len(Z0) == len(ys)
        ps = np.array([np.interp(ys, yy, phi[m]) for m in range(len(f))])  # Mode x Saite
        C = (ps * (sd.ANTEIL * Z0)) @ ps.T
    fr = np.arange(100.0, 400.01, 0.5)
    Y, P = antwort(f, eta, phi, G, C, fr)
    db = 20 * np.log10(np.abs(Y)) + 116.0                              # Bells Bezug: 116 dB = 1 s/kg
    ort = 40 - (yy + 0.057) / 0.035
    print(f"{o}  Daempfer {'an' if C.any() else 'aus'}; Moden bis 360 Hz: "
          + "  ".join(f"{a:.1f} (Q {1 / b:.0f})" for a, b in zip(f, eta) if a < 360))
    print("Gipfel von |Y| je Messort (Hz, Hoehe in dB nach Bell, Tiefe des flacheren Tals), 120-360 Hz, Tiefe >= 1 dB")
    zaehl = {}
    for j in range(len(yy)):
        if not 1.5 <= ort[j] <= 37:
            continue
        g = [x for x in gipfel(db[:, j], fr) if 120 <= x[0] <= 360]
        for x in g:
            zaehl.setdefault(5 * round(x[0] / 5), []).append(ort[j])
        print(f"  Ort {ort[j]:5.1f} (y {yy[j]:.3f}): " + "  ".join(f"{a:.0f} ({b:.0f}, {c:.0f})" for a, b, c in g))
    print("Messorte mit Gipfel je 5-Hz-Band: " + "  ".join(f"{b}: {len(v)}" for b, v in sorted(zaehl.items())))
    mittel = 10 * np.log10(np.mean(np.abs(Y[:, (ort >= 1.5) & (ort <= 37)]) ** 2, axis=1)) + 116.0
    print("Gipfel des Mittels von |Y|^2 ueber die Messorte: "
          + "  ".join(f"{a:.0f} ({b:.0f}, {c:.1f})" for a, b, c in gipfel(mittel, fr, 0.3) if 120 <= a <= 360))
    schall = 10 * np.log10(np.mean(np.abs(P) ** 2, axis=2))            # Leistungsmittel der Hoerorte, je Ort der Kraft
    sm = 10 * np.log10(np.mean(np.abs(P[:, (ort >= 1.5) & (ort <= 37)]) ** 2, axis=(1, 2)))
    print("Gipfel des Schalls je Kraft (Mittel ueber Messorte und Hoerorte): "
          + "  ".join(f"{a:.0f} ({c:.1f})" for a, b, c in gipfel(sm, fr, 0.3) if 120 <= a <= 360))
    j17 = int(np.argmin(np.abs(ort - 17)))
    print(f"Kurve am Messort {ort[j17]:.1f} (Abb. 7.6), alle 10 Hz: "
          + " ".join(f"{x:.0f}:{db[int((x - 100) * 2), j17]:.0f}" for x in np.arange(100, 400, 10)))
    for x in [float(s) for s in opt.get("f", "170,187,207,216").split(",")]:
        i = int(round((x - 100) * 2))
        vz = np.sign(np.real(P[i, :, 0] * np.conj(P[i, int(np.argmax(np.abs(P[i, :, 0]))), 0])))
        print(f"Profil bei {x:.0f} Hz, Messort: |Y| dB / Schall dB (Vorzeichen gegen den staerksten Ort)")
        print("   " + "  ".join(f"{ort[j]:.0f}: {db[i, j]:.0f} / {schall[i, j] - schall[i].max():+.0f}{'+' if vz[j] > 0 else '-'}"
                                for j in range(0, len(yy), 2) if 1.5 <= ort[j] <= 37))


if __name__ == "__main__":
    main()

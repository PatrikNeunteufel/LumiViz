"""S200, 34k-6 — Zeitbereichs-Kandidat fuer den Kern (Rechenfassung 2) gegen den Pruefstand.

Aufruf (vom Projekt-Root): python -B <dieses Skript> gitarre | cembalo [trapez|foh ...]

Der Pruefstand (`2026-11-15_S199_pruefstand_zupf.py`) rechnet den Ton geschlossen je Frequenz. Hier dieselbe Physik im
Zeitschritt bei 48 kHz, wie ein Kern sie rechnen muss:

  Schleife   v+[n] = g[n] - sum_k h[k] (u - v+)[n - k]: h ist der Umlauf Steg -> Sattel -> Steg der steifen Saite mit
             Verlusten (exp(-2 i k L)) als FIR — in dieser Stufe GENAU (aus der Frequenzantwort, ueber 20 kHz
             ausgeblendet), damit nur der Steg im Zeitschritt geprueft wird; g ist die Quelle des Zupfs am Steg
             (ebenfalls genau).
  Steg       alle Moden der Koerperdatei, zwei Richtungen: je Mode z[n] = pol z[n-1] + B1 (e F)[n-1] + B0 (e F)[n],
             u = Re sum e z. Zwei Diskretisierungen:
               trapez  B0 = c dt / 2, B1 = pol c dt / 2 (Impulsinvarianz mit halbem Durchgriff, wie der Kern heute)
               foh     genau fuer stueckweise lineare Kraft (B0 = c a0, B1 = c a1)
             Der Durchgriff macht den Steg in einem Schritt loesbar: u = (I + z0 M)^-1 (u_frei + 2 z0 M v+).
  Daempfer   GZ5-1 a ueber 5 kHz: Re Y = n(f) phi_rms^2 / 4 als wenige breite Moden (Q = 1,5) — passiv, im selben
             Rechenwerk; Gewichte nichtnegativ an die Kurve angepasst.

Verglichen wird die Stegkraft senkrecht zur Decke (GZ5 a: das gibt der Kern aus) und der Ton am Hoerort (mit der Antwort
des ganzen Koerpers gefaltet): Fehler der Stegkraft im Zeitverlauf, Lautheit, Pegel 1-2 s, je Teilton Frequenz und
Abklingen.

Schreibt nichts.
"""
import importlib.util
import json
import pathlib
import sys
import time

import numpy as np
import scipy.optimize as opt

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.parent / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)


def stegkraft_frequenz(saite, L, xp, d, winkel, Y, w, breite):
    """Wie `ton2` des Pruefstands, gibt aber die Stegkraft (Frequenz x 2) und die Quelle g, den Umlauf E zurueck."""
    T, m = saite["T"], saite["m"]
    k, z0 = ps.saite_wellen(saite, w)
    f0 = T * d * L / (xp * (L - xp))
    f = -f0 / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2)
    g1 = f / (2 * z0) * (np.exp(-1j * k * xp) - np.exp(-1j * k * (2 * L - xp)))
    E = np.exp(-2j * k * L)
    n = len(w)
    A = np.zeros((n, 2, 2), dtype=complex)
    A[:, 0, 0] = 1 + z0 * Y[:, 0]
    A[:, 0, 1] = A[:, 1, 0] = z0 * Y[:, 1]
    A[:, 1, 1] = 1 + z0 * Y[:, 2]
    Ainv = np.linalg.inv(A)
    eins = np.broadcast_to(np.eye(2), (n, 2, 2))
    Rb = Ainv @ (A - 2 * eins)
    a = np.radians(winkel)
    g = g1[:, None] * np.array([np.cos(a), np.sin(a)])[None, :]
    vplus = np.linalg.solve(eins + Rb * E[:, None, None], g[:, :, None])[:, :, 0]
    F = 2 * z0[:, None] * (Ainv @ vplus[:, :, None])[:, :, 0]
    return F, g


def umlauf_fir(saite, L, nh=16384):
    """Der Umlauf exp(-2 i k L) als FIR ab Verzug 1; ueber 20 kHz ausgeblendet."""
    f = np.fft.rfftfreq(nh, 1 / ps.FS)
    w = 2 * np.pi * np.maximum(f, 1e-3) + 0j
    k, _ = ps.saite_wellen(saite, w)
    E = np.exp(-2j * k * L)
    E *= np.where(f < 20000, 1.0, 0.5 * (1 + np.cos(np.pi * np.clip((f - 20000) / 3500, 0, 1))))
    h = np.fft.irfft(E, nh)
    mitte = int(np.argmax(np.abs(h)))
    ende = min(nh // 2, mitte + 200)
    h[0] = 0.0
    return h[:ende + 1].copy(), mitte


def moden_diskret(f, eta, e, verfahren):
    """pol, B0, B1 je Mode (komplex), e (Moden x 2) die Richtung mal Formwert."""
    dt = 1.0 / ps.FS
    wn = 2 * np.pi * f
    beta = eta * wn
    wd = np.sqrt(np.maximum(1e-12, wn * wn - 0.25 * beta * beta))
    lam = -0.5 * beta + 1j * wd
    c = lam / (1j * wd)                     # h(t) = Re c exp(lam t), h(0) = 1
    pol = np.exp(lam * dt)
    if verfahren == "trapez":
        B0 = 0.5 * c * dt
        B1 = 0.5 * c * dt * pol
    else:
        x = lam * dt
        B0 = c * (pol - 1 - x) / (lam * x)
        B1 = c * (pol * (x - 1) + 1) / (lam * x)
    return pol, B0, B1


def schleife(g, h, z0, pol, B0, B1, e, M):
    """Ohne numba: je Takt in numpy (die Moden als Vektor, der Umlauf als ein Skalarprodukt)."""
    n, K = g.shape[0], h.shape[0] - 1
    aus = np.zeros((n + K, 2))                      # um K Takte versetzt: aus[K + i] gehoert zum Takt i
    F = np.zeros((n, 2))
    z = np.zeros(pol.shape[0], dtype=complex)
    hr = h[:0:-1].copy()                            # h[K] ... h[1]
    A = np.linalg.inv(np.eye(2) + z0 * M)
    zM = 2.0 * z0 * M
    eT = np.ascontiguousarray(e.T)
    fv = np.zeros(2)
    for i in range(n):
        v = g[i] - hr @ aus[i:i + K]
        z = z * pol + B1 * (e @ fv)
        u = A @ (eT @ z.real + zM @ v)
        fv = z0 * (2.0 * v - u)
        z += B0 * (e @ fv)
        aus[K + i] = u - v
        F[i] = fv
    return F


def daempfer_moden(dichte_von, dichte, anstieg, streuung):
    """GZ5-1 a als breite Moden: Mitten 6-22 kHz, Q 1,5; Gewichte phi^2 >= 0 an Re Y = n(f) phi_rms^2 / 4 (5-20 kHz)
    angepasst, darunter soll Re Y klein bleiben (mit ins Ziel: 0 bei 0,5-4,5 kHz)."""
    mitten = np.array([6000.0, 7500.0, 9500.0, 12000.0, 15000.0, 19000.0, 23000.0])
    eta = np.full(len(mitten), 1 / 1.5)
    f = np.concatenate([np.arange(500.0, 4501.0, 250.0), np.arange(5250.0, 20001.0, 250.0)])
    ziel = np.where(f >= dichte_von, (dichte + anstieg * (f - dichte_von)) * streuung ** 2 / 4, 0.0)
    w, wn = 2 * np.pi * f[:, None], 2 * np.pi * mitten[None, :]
    A = np.real(1j * w / (wn ** 2 - w ** 2 + 1j * eta[None, :] * wn * w))
    gew = np.where(f < dichte_von, 3.0, 1.0)
    phi2, _ = opt.nnls(A * gew[:, None], ziel * gew)
    return mitten, eta, np.sqrt(phi2), f, ziel, A @ phi2


def spektrum_s0(x):
    """Zeitverlauf (wie aus `zeit`) zurueck auf die Gerade w - i s0 des Pruefstands."""
    n = int(ps.DAUER * ps.FS)
    y = np.zeros((n, x.shape[1]))
    y[:len(x)] = x * np.exp(-ps.S0 * np.arange(len(x)) / ps.FS)[:, None]
    return np.fft.rfft(y, axis=0) / ps.FS * np.exp(1j * ps.frequenzen() * ps.VORLAUF)[:, None]


def teiltoene(x, f0, anzahl=12):
    """Je Teilton Frequenz (Hz) aus einem Hann-Spektrum 0,1-1,1 s und der Pegel frueh (0,05-0,45 s) und spaet (1-2 s)."""
    a, e = int(0.1 * ps.FS), int(1.1 * ps.FS)
    S = np.abs(np.fft.rfft(x[a:e] * np.hanning(e - a), 1 << 20))
    hz = np.fft.rfftfreq(1 << 20, 1 / ps.FS)
    aus = []
    for n in range(1, anzahl + 1):
        band = (hz > f0 * (n - 0.3)) & (hz < f0 * (n + 0.3))
        if not band.any() or f0 * n > 12000:
            break
        i = int(np.argmax(np.where(band, S, 0)))
        y0, y1, y2 = np.log(S[i - 1] + 1e-300), np.log(S[i] + 1e-300), np.log(S[i + 1] + 1e-300)
        fn = hz[i] + 0.5 * (y0 - y2) / (y0 - 2 * y1 + y2) * (hz[1] - hz[0])
        t = np.arange(len(x)) / ps.FS
        z = x * np.exp(-2j * np.pi * fn * t)
        pegel = []
        for von, bis in ((0.05, 0.45), (1.0, 2.0)):
            s = slice(int(von * ps.FS), int(bis * ps.FS))
            pegel.append(20 * np.log10(np.abs(np.mean(z[s] * np.hanning(s.stop - s.start))) + 1e-300))
        aus.append((fn, pegel[0], pegel[1]))
    return aus


def vergleiche(name, saite, L, xp, d, winkel, breite, kp, punkt, datei, st, verfahren_liste, mit_daempfer):
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    f, eta = kp["f"][:datei], kp["eta"][:datei]
    phi = kp["steg"][:datei, punkt]
    e = np.stack([phi, phi * tan[:datei]], axis=1)
    nur = {"f": f, "eta": eta, "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    Y, _, _ = ps.koerper_kurven2(nur, punkt, w, tan[:datei])
    _, Hz, Hy = ps.koerper_kurven2(kp, punkt, w, tan)             # Abstrahlung: alle Moden
    if mit_daempfer:
        dm, de, dphi, _, _, _ = daempfer_moden(st["fromHz"], st["density"], st["rise"], st["bridgeSpread"][punkt])
        # im Pruefstand dieselben breiten Moden (der Vergleich prueft den Zeitschritt, nicht die Anpassung)
        wn = 2 * np.pi * dm
        yd = (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * de[None, :] * wn[None, :] * w[:, None])) @ (dphi ** 2)
        Y = Y.copy()
        Y[:, 0] += yd
        Y[:, 2] += 0.16 * yd
        f = np.concatenate([f, dm, dm])
        eta = np.concatenate([eta, de, de])
        e = np.concatenate([e, np.stack([dphi, 0 * dphi], axis=1), np.stack([0 * dphi, 0.4 * dphi], axis=1)])
    Fw, gw = stegkraft_frequenz(saite, L, xp, d, winkel, Y, w, breite)
    bezug = ps.zeit(Fw)
    g = ps.zeit(gw)
    h, mitte = umlauf_fir(saite, L)
    z0 = float(np.sqrt(saite["T"] * saite["m"]))
    f0 = saite["hz_ton"]
    pb = ps.zeit(spektrum_s0(bezug)[:, 0:1] * Hz + spektrum_s0(bezug)[:, 1:2] * Hy)
    tb = teiltoene(bezug[:, 0], f0)
    v, en = ps.VORLAUF, ps.VORLAUF + 0.4
    print(f"{name}: {len(f)} Moden am Steg, Umlauf {mitte} Takte, FIR {len(h)} Werte, z0 {z0:.3f} N s/m; Bezug (Pruefstand): "
          f"{ps.momentary_max(pb):.2f} LUFS, Pegel 1-2 s {ps.rms_db(pb, 1, 2) - ps.rms_db(pb, v, en):.2f} dB")
    for verfahren in verfahren_liste:
        pol, B0, B1 = moden_diskret(f, eta, e, verfahren)
        M = np.einsum("n,na,nb->ab", B0.real, e, e)
        t0 = time.time()
        F = schleife(np.ascontiguousarray(g), h, z0, pol, B0, B1, np.ascontiguousarray(e), M)
        dauer = time.time() - t0
        fehler = []
        for von, bis in ((v, v + 0.1), (v, v + 1.0), (1.0, 3.0)):
            s = slice(int(von * ps.FS), int(bis * ps.FS))
            fehler.append(20 * np.log10(np.sqrt(np.mean((F[s, 0] - bezug[s, 0]) ** 2) / np.mean(bezug[s, 0] ** 2))))
        Sp = spektrum_s0(F)
        p = ps.zeit(Sp[:, 0:1] * Hz + Sp[:, 1:2] * Hy)
        tk = teiltoene(F[:, 0], f0)
        cent = [1200 * np.log2(a[0] / b[0]) for a, b in zip(tk, tb)]
        abkl = [(a[1] - a[2]) - (b[1] - b[2]) for a, b in zip(tk, tb) if b[2] > b[1] - 60]
        print(f"  {verfahren:6s}: Fehler der Stegkraft 0-0,1 s / 0-1 s / 1-3 s {fehler[0]:6.1f} / {fehler[1]:6.1f} / {fehler[2]:6.1f} dB | "
              f"Lautheit {ps.momentary_max(p) - ps.momentary_max(pb):+.3f} LU, Pegel 1-2 s "
              f"{(ps.rms_db(p, 1, 2) - ps.rms_db(p, v, en)) - (ps.rms_db(pb, 1, 2) - ps.rms_db(pb, v, en)):+.3f} dB | "
              f"Teiltoene 1-{len(cent)}: Frequenz groesster Betrag {np.abs(cent).max():.3f} Cent (Grundton {cent[0]:+.3f}), "
              f"Abfall frueh->spaet groesster Betrag {np.abs(abkl).max() if abkl else float('nan'):.2f} dB | {dauer:.1f} s", flush=True)


def gitarre(verfahren):
    kp = ps.lade_koerper("classical_guitar")
    roh = json.loads((ps.KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))
    for taste in (40, 57, 76):
        s = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= taste and taste - sa["taste"] <= 12)
        sa = dict(ps.GITARRE[s])
        bund = taste - sa["taste"]
        L = ps.MENSUR * 2 ** (-bund / 12)
        sa["hz_ton"] = ps.hz_von(taste)
        punkt = kp["tasten"].index(sa["taste"])
        for mit in (False, True):
            vergleiche(f"Gitarre Taste {taste} (Saite {6 - s}, Bund {bund})" + (" mit Daempfer" if mit else ""), sa, L, min(ps.ZUPFORT, 0.5 * L),
                       0.002, 30.0, 0.0075, kp, punkt, len(roh["modes"]), roh["statistical"], verfahren, mit)


def cembalo(verfahren):
    kp = ps.lade_koerper("harpsichord")
    roh = json.loads((ps.KOERPER / "harpsichord.lvinstbody").read_text(encoding="utf-8"))
    for taste in (36, 60, 84):
        sa, L, xp, d, winkel, breite, _ = ps.cembalo_taste(taste)
        sa = dict(sa)
        sa["hz_ton"] = ps.hz_von(taste)
        punkt = kp["tasten"].index(taste)
        for mit in (False, True):
            vergleiche(f"Cembalo Taste {taste}" + (" mit Daempfer" if mit else ""), sa, L, xp, d, winkel, breite, kp, punkt,
                       len(roh["modes"]), roh["statistical"], verfahren, mit)


if __name__ == "__main__":
    was = sys.argv[1] if len(sys.argv) > 1 else ""
    verfahren = sys.argv[2:] or ["trapez", "foh"]
    if was == "gitarre":
        gitarre(verfahren)
    elif was == "cembalo":
        cembalo(verfahren)
    elif was == "daempfer":
        roh = json.loads((ps.KOERPER / "harpsichord.lvinstbody").read_text(encoding="utf-8"))
        st = roh["statistical"]
        m, e, phi, f, ziel, ist = daempfer_moden(st["fromHz"], st["density"], st["rise"], st["bridgeSpread"][30])
        print("Daempfer als breite Moden (Cembalo, Stegpunkt 30): phi^2 =", np.round(phi ** 2, 6))
        for fi, z, i in zip(f[::4], ziel[::4], ist[::4]):
            print(f"  {fi:7.0f} Hz  Ziel {z:.3e}  ist {i:.3e}")
    else:
        sys.exit(__doc__)

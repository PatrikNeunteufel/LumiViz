"""S200, 34k-6 — Cembalo im Pruefstand: Tastenstoss, zweites 8', 4', Lautenzug.

Aufruf (vom Projekt-Root): python -B <dieses Skript> stoss
                           python -B <dieses Skript> lautenzug
                           python -B <dieses Skript> register <harpsichord_4fuss.lvinstbody> [--je-taste]
                           python -B <dieses Skript> pruefen

Baut auf `2026-11-15_S199_pruefstand_zupf.py` (Saite, Koerper, Messung); neu ist `ton_viele`: mehrere Saiten an einem
Steg, je Saite eigener Zupfort, eigene Auslenkung, eigener Verzug und ein eigener Abschluss am Sattel.

stoss      Der Tastenstoss nach Penttinen 2006 (DAFx-06): mit kraeftigem Anschlag werden die Moden des Bodens "through
           mechanical coupling, separate from the string plucking mechanism" angeregt. Hier: ein Kraftstoss J (N s) senk-
           recht auf den Boden, Gauss in der Zeit (STOSS_SIGMA - ANNAHME), am Stegpunkt der Taste (ANNAHME: der Weg von
           Taste und Springerleiste ueber das Gehaeuse in den Boden ist nicht belegt), ohne Rueckwirkung der Saiten.
           Geeicht an Abb. 4 und 5 (am Seitenbild abgelesen, +-2 dB): im Betragsspektrum des Tons liegt die staerkste
           Bodenmode unter 300 Hz bei ff 4,5 dB (C6) bzw. 9 dB (C3, unter dem Grundton) unter der Spitze des Grundtons;
           ff -> mf 8-12 dB, mf -> pp 12-20 dB. Das Fenster der Analyse nennt die Quelle nicht: gerechnet mit 1,4 s
           (Laenge ihrer Huellkurven) und zur Probe mit 0,5 s. Mikrofon dort 1 m ueber dem Boden wie der Hoerort hier.
lautenzug  Ein Filz am Sattel als Daempfer R (N s/m) im Abstand a vom Sattel: Abschluss r = (Z0 - ZL) / (Z0 + ZL) mit
           ZL = R + Z0 / (i tan(k a)). Keine Quelle fuer R und a; Ziel aus den Aufnahmen S195 (englisches Cembalo VCSL,
           Lute gegen Normal, 27 Tasten): Lautheit im Median -1,8 LU (-0,4 im Bass bis -4,9 in C6), Abfall frueh im
           Median 2,3-mal so schnell (1,9-3,2 je Oktave).
register   8' wie im Datenblatt (Edinburgh 96075), das zweite 8' eine Springerreihe mal zwei weiter (+-29 mm: der
           Abstand der Reihen 14,5 mm nach dem NEMUS-Nachbau, das 4' dazwischen - welche Reihe das Datenblatt misst,
           sagt es nicht), beide 8' am selben Stegpunkt, +-1 Cent gegeneinander (ANNAHME); 4': Mensur nach Hubbard /
           Di Veroli (f''' 74, f'' 124, f' 255, f 486, F 773, FF 1073 mm; passt zu Taskin 1769 c 605, c' 337, c'' 163),
           Draht des Taskin 1769 nach Di Veroli Tab. 8, Zupfpunkt = der Anteil des 8' gleicher Tonhoehe, hoechstens
           0,5 (ANNAHME; der Sattel des 4' laege damit 89 mm (Bass) bis 68 mm (Diskant) hinter dem des 8'), am 4'-Steg
           (Koerper der Probe `2026-11-16_S200_cembalo_4fuss_plan.py`, dort Tasten + 100). Abloesekraft 2 N fuer alle.
pruefen    R = 0 gibt den starren Sattel; eine Saite ueber `ton_viele` gegen `ton_mit`.

Schreibt nichts.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.signal as sig

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.parent / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

STOSS_SIGMA = 0.0004        # s, ANNAHME: ein weicher Schlag von rund 1 ms (Tuch auf Holz)
FENSTER = (1.4, 0.5, 0.2, 0.1, 0.05)   # s, Fenster der Spektren fuer die Eichung (die Quelle nennt ihres nicht)
REIHE = 0.029              # m, zwei Springerreihen (2 x 14,5 mm)
VIER_MENSUR = [(29, 1073), (41, 773), (53, 486), (65, 255), (77, 124), (89, 74)]
# Taskin 1769, 4' (Di Veroli Tab. 8; Durchmesser Tab. 7): bis Taste, d mm, Dichte, E GPa (Dichte und E wie beim 8': Annahme)
VIER_DRAHT = [(31, 0.463, 8700, 115), (35, 0.410, 8700, 115), (38, 0.377, 8700, 115), (40, 0.377, 8500, 110), (42, 0.339, 8500, 110),
              (45, 0.300, 8500, 110), (49, 0.300, 7800, 170), (55, 0.269, 7800, 170), (72, 0.244, 7800, 170), (79, 0.213, 7800, 170),
              (89, 0.188, 7800, 170)]


def sattel(k, z0, a, R):
    """Abschluss am Sattel (auf die Sattelebene bezogen): Daempfer R im Abstand a davor."""
    zl = R + z0 / (1j * np.tan(k * a))
    return (z0 - zl) / (z0 + zl) * np.exp(2j * k * a)


def ton_viele(saiten, Y, H, w):
    """`saiten`: je Saite dict mit sa, L, und fuer gezupfte xp (vom Steg), d, winkel, breite; dazu wahlweise verzug (s)
    und filz = (a, R). Y (Frequenz x 2S x 2S), H (Frequenz x 2S x Hoerorte) aus `koerper_matrix`."""
    S, n = len(saiten), len(w)
    z = np.empty((n, 2 * S), dtype=complex)
    E = np.empty((n, 2 * S), dtype=complex)
    g = np.zeros((n, 2 * S), dtype=complex)
    for i, s in enumerate(saiten):
        sa, L = s["sa"], s["L"]
        k, z0 = ps.saite_wellen(sa, w)
        rn = sattel(k, z0, *s["filz"]) if s.get("filz") else -1.0
        z[:, 2 * i] = z[:, 2 * i + 1] = z0
        E[:, 2 * i] = E[:, 2 * i + 1] = -rn * np.exp(-2j * k * L)
        if s.get("d"):
            T, m, xp = sa["T"], sa["m"], s["xp"]
            f = -(T * s["d"] * L / (xp * (L - xp))) / (1j * w) * np.exp(-(w * s.get("breite", 0.003) / np.sqrt(T / m)) ** 2)
            gi = f / (2 * z0) * (np.exp(-1j * k * xp) + rn * np.exp(-1j * k * (2 * L - xp))) * np.exp(-1j * w * s.get("verzug", 0.0))
            a = np.radians(s.get("winkel", 20.0))
            g[:, 2 * i], g[:, 2 * i + 1] = gi * np.cos(a), gi * np.sin(a)
    p = np.empty((n, H.shape[2]), dtype=complex)
    eins = np.eye(2 * S)
    for a in range(0, n, 8192):
        b = slice(a, a + 8192)
        YZ = Y[b] * z[b][:, None, :]
        M = np.linalg.solve(eins[None] + YZ, 2 * YZ)
        vplus = np.linalg.solve(eins[None] + E[b][:, :, None] * (M - eins[None]), g[b][:, :, None])[:, :, 0]
        F = z[b] * (2 * vplus - (M @ vplus[:, :, None])[:, :, 0])
        p[b] = np.einsum("fa,fao->fo", F, H[b])
    return p


def acht(taste, plus=0.0, cent=0.0):
    """Eine 8'-Saite der Taste: die Angaben des Pruefstands, der Zupfpunkt um `plus` (m) weiter vom Sattel."""
    sa, L, xp, d, winkel, breite, _ = ps.cembalo_taste(taste)
    zupf = (L - xp) + plus                      # ab Sattel
    L2 = L * 2 ** (-cent / 1200)
    return {"sa": sa, "L": L2, "xp": L2 - zupf, "d": ps.CEMBALO_KRAFT * zupf * (L2 - zupf) / (sa["T"] * L2), "winkel": winkel, "breite": breite}


def vier(taste):
    t = [z[0] for z in VIER_MENSUR]
    L = float(np.exp(np.interp(taste, t, np.log([z[1] for z in VIER_MENSUR])))) / 1000
    _, L8, xp8, *_ = ps.cembalo_taste(min(taste + 12, 89))
    anteil = min((L8 - xp8) / L8, 0.5)
    zupf = anteil * L
    d, rho, e = next((d, rho, e) for bis, d, rho, e in VIER_DRAHT if taste <= bis)
    d /= 1000
    m = rho * np.pi * d * d / 4
    hz = 2 * ps.hz_von(taste)
    sa = {"T": 4 * L * L * hz * hz * m, "m": m, "EI": e * 1e9 * np.pi * d ** 4 / 64, "etaB": 1e-3, "d": d, "rho": rho, "hz": hz}
    return {"sa": sa, "L": L, "xp": L - zupf, "d": ps.CEMBALO_KRAFT * zupf * (L - zupf) / (sa["T"] * L), "winkel": 20.0, "breite": 0.003}


def spruenge(l, je_taste):
    l = np.array(l)
    d3 = np.abs(l[3:] - l[:-3]) if je_taste else np.abs(np.diff(l))
    return f"Spanne {l.max() - l.min():5.1f} LU, groesster Sprung im Terzabstand {d3.max():4.1f} LU, mittlerer {np.median(d3):.1f} LU"


# ---------------------------------------------------------------- Tastenstoss
def spektrum(x, sekunden):
    a = int(ps.VORLAUF * ps.FS)
    teil = x[a:a + int(sekunden * ps.FS), 0]
    S = np.fft.rfft(teil, 1 << 19)
    return np.fft.rfftfreq(1 << 19, 1 / ps.FS), S


def stoss():
    kp = ps.lade_koerper("harpsichord")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    gauss = np.exp(-0.5 * (w * STOSS_SIGMA) ** 2)
    ziel = {48: -9.0, 84: -4.5}
    print("Tastenstoss: Kraftstoss J senkrecht am Stegpunkt der Taste, Gauss sigma", STOSS_SIGMA * 1000, "ms")
    print("Taste | ohne Stoss: staerkste Bodenmode unter der Grenze gegen den Grundton (dB), je Fenster " +
          " / ".join(f"{s:g}" for s in FENSTER) + " s | J fuer das Ziel (mN s), je Fenster")
    geeicht = {}
    daten = {}
    for taste in (36, 48, 60, 72, 84):
        punkt = kp["tasten"].index(taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [punkt])
        x = ps.zeit(ton_viele([acht(taste)], Y, H, w))
        xs = ps.zeit(gauss[:, None] * H[:, 0, :])           # J = 1 N s
        daten[taste] = (x, xs)
        f0 = ps.hz_von(taste)
        grenze = min(300.0, 0.8 * f0)
        zeile = f"{taste:3d} | "
        js = []
        for sek in FENSTER:
            f, S = spektrum(x, sek)
            _, K = spektrum(xs, sek)
            grund = (f > 0.97 * f0) & (f < 1.03 * f0)
            band = (f > 30.0) & (f < grenze)

            def pegel(J):
                G = S + J * K
                return 20 * np.log10(np.abs(G[band]).max() / np.abs(G[grund]).max())
            zeile += f"{pegel(0.0):6.1f} "
            if taste in ziel:
                lo, hi = 1e-7, 1.0
                for _ in range(60):
                    mi = np.sqrt(lo * hi)
                    lo, hi = (mi, hi) if pegel(mi) < ziel[taste] else (lo, mi)
                js.append(hi)
        if js:
            zeile += "| " + "  ".join(f"{1000 * j:7.2f}" for j in js)
            geeicht[taste] = js
        print(zeile, flush=True)
    werte = (0.080, 0.040, 0.020, 0.010, 0.005, 0.0025)
    print("Taste | Lautheit ohne Stoss (LUFS) | mit Stoss J = " + " / ".join(f"{1000 * j:g}" for j in werte) + " mN s: Unterschied LU | "
          "Band 30-300 Hz in den ersten 0,2 s gegen den ganzen Ton dort (dB): ohne / je J")
    sos = sig.butter(4, [30.0, 300.0], btype="bandpass", fs=ps.FS, output="sos")
    for taste, (x, xs) in daten.items():
        l0 = ps.momentary_max(x)
        aus, band = [], []
        for J in (0.0,) + werte:
            y = x + J * xs
            if J:
                aus.append(ps.momentary_max(y) - l0)
            a, e = int(ps.VORLAUF * ps.FS), int((ps.VORLAUF + 0.2) * ps.FS)
            tief = sig.sosfilt(sos, y, axis=0)
            band.append(10 * np.log10(np.mean(tief[a:e] ** 2) / np.mean(y[a:e] ** 2)))
        print(f"{taste:3d} | {l0:6.1f} | " + " / ".join(f"{v:+5.2f}" for v in aus) + " | " + " / ".join(f"{v:6.1f}" for v in band), flush=True)
    x, xs = daten[60]
    print(f"Stoss allein, J = 10 mN s (Taste 60): Lautheit {ps.momentary_max(0.01 * xs):.1f} LUFS, Spitze "
          f"{20 * np.log10(np.abs(0.01 * xs).max()):.1f} dB re 1 Pa; Abfall frueh {ps.abfall(xs)[0]:.0f} dB/s")


# ---------------------------------------------------------------- Lautenzug
def lautenzug():
    kp = ps.lade_koerper("harpsichord")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    tasten = (36, 48, 60, 72, 84)
    # schwacher Daempfer (R = r Z0, r <= 3) und starker (R fest in N s/m, R >> Z0: die Saite endet am Filz, der Verlust je
    # Umlauf 2 Z0 / R haengt nicht von der Frequenz ab)
    faelle = [(a, r) for a in (0.003, 0.006, 0.012) for r in (0.1, 0.3, 1.0, 3.0)]
    faelle += [(a, -R) for a in (0.001, 0.003) for R in (20.0, 40.0, 80.0, 160.0)]
    print("Lautenzug: Daempfer im Abstand a vom Sattel, R = r * Z0 oder fest (Spalte 'R' in N s/m). Je Taste: Lautheit gegen ohne "
          "(LU) / Abfall frueh mal / Grundton in Cent")
    print("a mm  r / R | " + " | ".join(f"Taste {t}" for t in tasten) + " | Median")
    ohne = {}
    kurven = {}
    for t in tasten:
        kurven[t] = ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(t)])
        x = ps.zeit(ton_viele([acht(t)], *kurven[t], w))
        ohne[t] = (ps.momentary_max(x), ps.abfall(x)[0])
    print("ohne      | " + " | ".join(f"{ohne[t][0]:6.1f} LUFS {ohne[t][1]:5.1f} dB/s" for t in tasten))
    for a, r in faelle:
        zeile, dl, va = f"{a * 1000:4.0f} " + (f"{r:4.1f} Z0" if r > 0 else f"R {-r:4.0f}") + " | ", [], []
        for t in tasten:
            s = acht(t)
            z0 = np.sqrt(s["sa"]["T"] * s["sa"]["m"]) if r > 0 else -1.0
            s["filz"] = (a, r * z0)
            x = ps.zeit(ton_viele([s], *kurven[t], w))
            # Verstimmung des Grundtons aus der Phase des Abschlusses
            w1 = np.array([2 * np.pi * ps.hz_von(t) + 0j])
            k1, z1 = ps.saite_wellen(s["sa"], w1)
            cent = 1200 * np.log2(1 + np.angle(-sattel(k1, z1, a, r * z0))[0] / (2 * np.pi))
            dl.append(ps.momentary_max(x) - ohne[t][0])
            va.append(ps.abfall(x)[0] / ohne[t][1])
            zeile += f"{dl[-1]:+5.1f} / {va[-1]:4.1f} / {cent:+5.1f} | "
        print(zeile + f"{np.median(dl):+5.1f} LU, mal {np.median(va):.1f}", flush=True)


# ---------------------------------------------------------------- Register
def register(datei, je_taste):
    ps.KOERPER = pathlib.Path(datei).resolve().parent
    name = pathlib.Path(datei).stem
    kp = ps.lade_koerper(name)
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    print(f"Koerper {name}: {len(kp['f'])} Moden, {len(kp['tasten'])} Stegpunkte")
    sp = {n: [] for n in ("8' I", "8' II +", "8' II -", "8' + 8'", "4'", "8' + 4'", "8' + 8' + 4'")}
    print("Taste | Lautheit LUFS: " + " | ".join(sp) + " | Abfall frueh 8' I / 4' dB/s")
    for taste in range(29, 90, 1 if je_taste else 3):
        p8, p4 = kp["tasten"].index(taste), kp["tasten"].index(taste + 100)
        Y, H = ps.koerper_matrix(kp, w, tan, [p8, p8, p4])
        e8, e4 = slice(0, 2), slice(4, 6)
        e88, alle = slice(0, 4), slice(0, 6)
        a1, v4 = acht(taste, 0.0, +1.0), vier(taste)
        t = {}
        t["8' I"] = ton_viele([acht(taste)], Y[:, e8, e8], H[:, e8], w)
        t["8' II +"] = ton_viele([acht(taste, +REIHE)], Y[:, e8, e8], H[:, e8], w)
        t["8' II -"] = ton_viele([acht(taste, -REIHE)], Y[:, e8, e8], H[:, e8], w)
        t["8' + 8'"] = ton_viele([a1, acht(taste, +REIHE, -1.0)], Y[:, e88, e88], H[:, e88], w)
        t["4'"] = ton_viele([v4], Y[:, e4, e4], H[:, e4], w)
        idx = [0, 1, 4, 5]
        t["8' + 4'"] = ton_viele([acht(taste), v4], Y[:, idx][:, :, idx], H[:, idx], w)
        t["8' + 8' + 4'"] = ton_viele([a1, acht(taste, +REIHE, -1.0), v4], Y[:, alle, alle], H[:, alle], w)
        x = {n: ps.zeit(v) for n, v in t.items()}
        for n in sp:
            sp[n].append(ps.momentary_max(x[n]))
        print(f"{taste:3d} | " + " | ".join(f"{sp[n][-1]:6.1f}" for n in sp) + f" | {ps.abfall(x[chr(56) + chr(39) + ' I'])[0]:5.1f} / {ps.abfall(x[chr(52) + chr(39)])[0]:5.1f}",
              flush=True)
    for n in sp:
        print(f"{n:13s}: {spruenge(sp[n], je_taste)}; gegen 8' I im Median {np.median(np.array(sp[n]) - np.array(sp[chr(56) + chr(39) + ' I'])):+.1f} LU "
              f"(von {np.min(np.array(sp[n]) - np.array(sp[chr(56) + chr(39) + ' I'])):+.1f} bis {np.max(np.array(sp[n]) - np.array(sp[chr(56) + chr(39) + ' I'])):+.1f})")


def pruefen():
    kp = ps.lade_koerper("harpsichord")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    Y, H = ps.koerper_matrix(kp, w, tan, [kp["tasten"].index(60)])
    sa, L, xp, d, winkel, breite, tempo = ps.cembalo_taste(60)
    alt = ps.ton_mit([sa], [L], 0, xp, d, winkel, Y, H, w, breite, tempo)[0]
    neu = ton_viele([acht(60)], Y, H, w)
    s = acht(60)
    s["filz"] = (0.006, 0.0)
    null = ton_viele([s], Y, H, w)
    print(f"eine Saite, ton_viele gegen ton_mit: {np.abs(neu - alt).max() / np.abs(alt).max():.2e}")
    print(f"Filz mit R = 0 gegen den starren Sattel: {np.abs(null - neu).max() / np.abs(neu).max():.2e}")
    for t in (29, 41, 53, 65, 77, 89):
        v, a = vier(t), acht(t)
        print(f"Taste {t}: 4' L {v['L']:.3f} m, Zug {v['sa']['T']:5.1f} N, Zupf {v['L'] - v['xp']:.3f} m ab Sattel, Auslenkung {1000 * v['d']:.2f} mm | "
              f"8' L {a['L']:.3f} m, Zug {a['sa']['T']:5.1f} N, Zupf {a['L'] - a['xp']:.3f} m, Auslenkung {1000 * a['d']:.2f} mm")


if __name__ == "__main__":
    was = sys.argv[1] if len(sys.argv) > 1 else ""
    if was == "stoss":
        stoss()
    elif was == "lautenzug":
        lautenzug()
    elif was == "register":
        register(sys.argv[2], "--je-taste" in sys.argv)
    elif was == "pruefen":
        pruefen()
    else:
        sys.exit(__doc__)

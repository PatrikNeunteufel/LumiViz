"""S205 - KQ35 a: der Gitarrenkoerper ueber 2,2 kHz (Dichte nach Elie 2011) und die zweite Stegrichtung (Elie 2012).

Aufruf (vom Projekt-Root):
  python -X utf8 -B <dieses Skript> dichte <qualified_selected_impulses.mat> <zielordner> [samen=1] [laeufe=8] [strahl=voll]
      Fuellt den mitgelieferten Koerper `classical_guitar` auf und prueft gegen die Messung g34 (Mores):
        2,2-5 kHz   die 31 gefitteten Moden bleiben; dazu gezogene Moden bis zur Dichte der Gitarre vom Gitarrenbauer
                    (Elie 2011 Abb. 4b, abgelesen in Bericht 23: 0,027 /Hz bis 3 kHz, 0,026 bis 4 kHz, 0,0235 bis 5 kHz).
                    Groesse der gezogenen Moden: ESPRIT findet die Moden, die an den Messpunkten stark sind. Sind die
                    Formwerte normalverteilt und fehlt der Anteil 1 - r der Moden (r = gefittet / Soll), dann fehlen die
                    mit |z| < t, P(|z| > t) = r; ihr Anteil an der Energie ist m = erf(t / sqrt 2) - t sqrt(2 / pi)
                    exp(-t^2 / 2). Eine gezogene Mode ist eine zufaellig gewaehlte gefittete Mode desselben Bands (Form
                    laengs der Stegeinlage und Abstrahlgewicht bleiben beisammen), ihr Formwert mal
                    sqrt((m / (1 - r)) / ((1 - m) / r)), Vorzeichen gezogen; Daempfung nach dem Gesetz von KQ26 a.
                    Abstrahlgewicht der gezogenen Moden: Phase gezogen, Betrag je Band so gestellt, dass der Terzpegel
                    Druck je Kraft (2,5 kHz; 3,15 und 4 kHz; 5 kHz) wieder so liegt wie mit den gefitteten Moden
                    allein (KQ35 a: „gestellt am Terzpegel von g34“); mit `strahl=voll` bleibt der Betrag der
                    gefitteten Mode.
        ab 5 kHz    statistischer Teil mit der Dichte 0,0215 /Hz (Elie 5-6 kHz; darueber fortgesetzt - Annahme);
                    Streuung am Steg = die der gefitteten Moden 2,5-5 kHz mal sqrt(r / (1 - m)); der Betrag der
                    Abstrahlung wie in `make_gitarre.py` am Terzpegel 6,3 / 8 / 10 kHz gestellt.
      Gibt aus: Terzpegel Druck je Kraft Modell - Messung (2,5 ... 10 kHz), Fehler ueber die sechs Stegkurven
      (rms von 20 lg |Modell / Messung|, mit zwei Resttermen je Kurve) - je fuer den Koerper heute und aufgefuellt,
      aufgefuellt als Mittel ueber `laeufe` Samen. Schreibt <zielordner>/classical_guitar.lvinstbody (Samen `samen`).
  python -X utf8 -B <dieses Skript> richtung [<koerperordner>]
      Yzy - Yzz und Yyy - Yzz je Band an der Saite 1 (E4) fuer drei Fassungen der Richtung:
        scherrer   tan(theta) wie im Pruefstand heute (0,03 (f / 200)^1,27, 0,03 ... 0,4)
        elie       |tan(theta)| je Band fest = 10^((Yzy - Yzz) / 20) nach Elie 2012 Abb. 3.17 (Bericht 23 Tab. 2.2, eigene
                   Digitalisierung in S203 auf 0,2 dB gleich), Vorzeichen gezogen (Samen 7). Unter 150 Hz die Werte des
                   Bands 150-300 Hz, ueber 4 kHz die von 1-4 kHz - Annahme. (Eine log-normale Verteilung von tan mit den
                   zwei Momenten aus Yzy und Yyy streut bei so wenigen Moden je Band um 10-30 dB zwischen den Samen.)
        elie-y     dazu je Mode eine zweite, die den Steg nur parallel zur Decke bewegt (siehe `mit_y_moden`) - sie
                   traegt, was Yyy ueber einem Winkel je Mode liegt. Annahme: Zahl, Lage und Daempfung dieser Moden.
  python -X utf8 -B <dieses Skript> ton <koerperordner> <scherrer|elie|elie-y> [winkel=30]
      Der Pruefstand `2026-11-15_S199_pruefstand_zupf.py gitarre --je-taste --winkel` an dieser Koerperdatei mit dieser
      Richtung, dazu die Kennzahlen nach KQ34 a.
Schreibt nur die Koerperdatei in den Zielordner.
"""
import contextlib
import importlib.util
import io
import json
import math
import pathlib
import sys

import numpy as np

P = pathlib.Path(__file__).parent
KOERPER = pathlib.Path("asset/loop/instrument_bodies")
SOLL = [(2200.0, 3000.0, 0.027), (3000.0, 4000.0, 0.026), (4000.0, 5000.0, 0.0235)]   # Elie 2011 Abb. 4b, Gitarrenbauer
SOLL_AB_5K = 0.0215
# Elie 2012 Abb. 3.17 (Bericht 23 Tab. 2.2): von, bis, Yzy - Yzz, Yyy - Yzz (dB)
ELIE = [(150, 300, -14.2, -22.1), (300, 500, -18.3, -23.7), (500, 1000, -13.5, -10.4), (1000, 1500, -19.3, -19.2),
        (1500, 2000, -9.5, -11.7), (2000, 2500, -13.5, -15.1), (2500, 3000, -16.9, -16.2), (3000, 3500, -12.9, -18.9),
        (3500, 4000, -9.2, -14.2)]
ELIE_GESAMT = (-12.9, -15.6)          # 1-4 kHz


def modul(pfad, name):
    spec = importlib.util.spec_from_file_location(name, pfad)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


def abgeschnitten(r):
    """t mit P(|z| > t) = r und der Energieanteil m der Moden mit |z| < t."""
    lo, hi = 0.0, 8.0
    for _ in range(60):
        t = 0.5 * (lo + hi)
        lo, hi = (t, hi) if math.erfc(t / math.sqrt(2)) > r else (lo, t)
    return t, math.erf(t / math.sqrt(2)) - t * math.sqrt(2 / math.pi) * math.exp(-t * t / 2)


# ---------------------------------------------------------------- dichte
def dichte(mat, ziel, samen, laeufe, strahl):
    import scipy.io as sio
    mg = modul(KOERPER / "make_gitarre.py", "make_gitarre")
    k = json.loads((KOERPER / "classical_guitar.lvinstbody").read_text(encoding="utf-8"))
    f = np.array([m["f"] for m in k["modes"]])
    s = np.array([m["eta"] for m in k["modes"]]) * np.pi * f
    steg = np.array([m["bridge"] for m in k["modes"]])
    G = np.array([[complex(*g) for g in m["radiation"]] for m in k["modes"]])
    xs = np.array(mg.X_SAITEN)
    abc = np.array([np.polyfit(xs, z, 2)[::-1] for z in steg])                 # a, b, c je Mode
    assert np.abs(abc @ np.stack([np.ones(6), xs, xs * xs]) - steg).max() < 1e-9, "die Formwerte sind quadratisch in x"
    st0 = k["statistical"]
    gesetz = [st0["damping"]["a"], st0["damping"]["b"], st0["damping"]["c"]]

    d = sio.loadmat(mat)["qualified_selected_impulses"][mg.GITARRE - 1]
    daten = {ort: d[o * mg.FS:(o + 1) * mg.FS, :] for o, ort in enumerate(mg.ORTE)}
    hz, H = mg.frfs(daten)
    w = 2 * np.pi * np.maximum(hz, 1e-9)
    mob = [H[p] / (1j * w) for p in mg.PAARE]
    xh = np.array([mg.X_SCHLAG[p[0]] for p in mg.PAARE])
    xa = np.array([mg.X_AUFN[p[1]] for p in mg.PAARE])
    form = lambda q, x: q[:, 0:1] + q[:, 1:2] * x[None, :] + q[:, 2:3] * x[None, :] ** 2      # noqa: E731

    def stegfehler(fa, sa, qa, von, bis):
        sel = (hz >= von) & (hz < bis)
        B = mg.basis(hz[sel], fa, sa)
        ws = w[sel]
        rest = np.stack([1 / (1j * ws), 1j * ws * 1e-8], axis=1)
        aus = []
        for q, y in enumerate(mob):
            yf = B @ (form(qa, xh[q:q + 1])[:, 0] * form(qa, xa[q:q + 1])[:, 0])
            gew = 1.0 / np.maximum(np.abs(y[sel]), 1e-6)
            A = np.concatenate([(rest * gew[:, None]).real, (rest * gew[:, None]).imag])
            r = np.concatenate([((y[sel] - yf) * gew).real, ((y[sel] - yf) * gew).imag])
            yf = yf + rest @ np.linalg.lstsq(A, r, rcond=None)[0]
            aus.append(20 * np.log10(np.abs(yf) / np.abs(y[sel])))
        return float(np.sqrt(np.mean(np.concatenate(aus) ** 2)))

    mitten = [1000 * 2 ** (i / 3) for i in range(4, 11)]                       # 2,5 ... 10 kHz
    selp = (hz >= mitten[0] / 2 ** (1 / 6)) & (hz < mitten[-1] * 2 ** (1 / 6))
    terz = lambda p: np.array([10 * np.log10(np.mean(p[(hz[selp] >= m / 2 ** (1 / 6)) & (hz[selp] < m * 2 ** (1 / 6))])) for m in mitten])  # noqa: E731
    mess = [terz(np.mean([np.abs(H[(o, mik)][selp]) ** 2 for o in mg.ORTE], axis=0)) for _, mik in mg.MIKROFONE]

    def pegel(fa, sa, phi, g):
        B = mg.basis(hz[selp], fa, sa) * (1j * w[selp, None])
        return [terz(np.mean([np.abs(B @ (phi[:, i] * g[:, h])) ** 2 for i in range(6)], axis=0)) for h in range(2)]

    def mit_statistik(fa, sa, qa, ga, st):
        fz, sz, stegz, gz = mg.ziehe(st)
        return (np.concatenate([fa, fz]), np.concatenate([sa, sz]),
                np.concatenate([form(qa, xs), stegz]), np.concatenate([ga, gz]))

    st_heute = {**st0, "seed": int(st0["seed"]), "damping": gesetz}
    heute = mit_statistik(f, s, abc, G, st_heute)
    p0 = pegel(*heute)
    print("Koerper heute: 75 Moden, statistisch ab 5 kHz mit 0,0104 /Hz")
    print("  Terzpegel p/F Modell - Messung, 2,5 / 3,15 / 4 / 5 / 6,3 / 8 / 10 kHz:")
    for h, (name, _) in enumerate(mg.MIKROFONE):
        print(f"    {name:5s}: " + " ".join(f"{v:+5.1f}" for v in p0[h] - mess[h]))
    baender = ((1000, 2200), (2200, 3000), (3000, 4000), (4000, 5000), (2500, 5000))
    e0 = [stegfehler(f, s, abc, lo, hi) for lo, hi in baender]
    print("  Stegkurven, rms dB in 1-2,2 / 2,2-3 / 3-4 / 4-5 / 2,5-5 kHz: " + " / ".join(f"{v:.2f}" for v in e0))

    print("\nAuffuellen 2,2-5 kHz:")
    plan = []
    for lo, hi, soll in SOLL:
        im = np.where((f >= lo) & (f < hi))[0]
        r = len(im) / (soll * (hi - lo))
        t, m = abgeschnitten(r)
        rho = (m / (1 - r)) / ((1 - m) / r)
        plan.append((lo, hi, soll, im, r, t, m, rho))
        print(f"  {lo:.0f}-{hi:.0f} Hz: {len(im)} gefittet von {soll * (hi - lo):.1f} (r = {r:.2f}), t = {t:.2f}, fehlender Energieanteil "
              f"{100 * m:.1f} % ({-10 * math.log10(1 - m):+.2f} dB), Formwert² der gezogenen gegen die gefitteten {10 * math.log10(rho):+.1f} dB")
    band = (f >= 2500) & (f < 5000)
    r_st = band.sum() / sum(soll * (min(hi, 5000) - max(lo, 2500)) for lo, hi, soll in SOLL)
    t_st, m_st = abgeschnitten(r_st)
    faktor = math.sqrt(r_st / (1 - m_st))
    print(f"  statistischer Teil: Dichte {SOLL_AB_5K} /Hz, Streuung am Steg mal {faktor:.3f} (r = {r_st:.2f}, m = {100 * m_st:.1f} %)")

    def fuelle(sa_):
        z = mg.Zufall(sa_)
        fn, sn, qn, gn, bn = [], [], [], [], []
        for nr, (lo, hi, soll, im, r, t, m, rho) in enumerate(plan):
            x = lo
            zus = soll - len(im) / (hi - lo)
            while True:
                x += z.rayleigh() / zus
                if x >= hi:
                    break
                j = im[int(z.gleich() * len(im)) % len(im)]
                vz = 1.0 if z.gleich() < 0.5 else -1.0
                fn.append(x)
                sn.append(0.5 * (gesetz[0] * x * x + gesetz[1] * x + gesetz[2]))
                qn.append(abc[j] * vz * math.sqrt(rho))
                ph = 2 * math.pi * z.gleich()
                gn.append(G[j] * complex(math.cos(ph), math.sin(ph)))
                bn.append(nr)
        fa, sa2 = np.concatenate([f, fn]), np.concatenate([s, sn])
        qa, gn, bn = np.concatenate([abc, np.array(qn)]), np.array(gn), np.array(bn)
        skala = np.ones(len(plan))
        if strahl != "voll":                                                   # Betrag je Band am Terzpegel
            for _ in range(12):
                syn = pegel(fa, sa2, form(qa, xs), np.concatenate([G, gn * skala[bn][:, None]]))
                ab = np.mean([syn[h] - nur[h] for h in range(2)], axis=0)       # gegen die gefitteten Moden allein
                for b, tb in enumerate(([0], [1, 2], [3])):
                    skala[b] = max(0.02, skala[b] * 10 ** (-3 * np.mean(ab[tb]) / 20))
        ga = np.concatenate([G, gn * skala[bn][:, None]])
        st = {**st_heute, "density": SOLL_AB_5K, "seed": sa_,
              "bridgeSpread": [float(v) for v in np.sqrt(np.mean(steg[band] ** 2, axis=0)) * faktor],
              "radiation": list(st0["radiation"])}
        ziel_b = slice(4, 7)                                                   # 6,3 / 8 / 10 kHz
        for _ in range(8):
            syn = pegel(*mit_statistik(fa, sa2, qa, ga, st))
            for h in range(2):
                st["radiation"][h] *= float(10 ** (np.mean((mess[h] - syn[h])[ziel_b]) / 20))
        return fa, sa2, qa, ga, st, len(fn), skala

    nur = pegel(f, s, form(abc, xs), G)                                       # die gefitteten Moden allein
    pe, ee, zahl, sk = [], [], [], []
    for lauf in range(laeufe):
        fa, sa2, qa, ga, st, n, skala = fuelle(samen + lauf)
        sk.append(skala)
        pe.append(pegel(*mit_statistik(fa, sa2, qa, ga, st)))
        ee.append([stegfehler(fa, sa2, qa, lo, hi) for lo, hi in baender])
        zahl.append(n)
    pe, ee = np.array(pe), np.array(ee)
    print("  Betrag der Abstrahlung der gezogenen Moden gegen die gefitteten, je Band (Median ueber die Samen, dB): "
          + " / ".join(f"{20 * math.log10(v):+.1f}" for v in np.median(sk, axis=0)))
    print(f"\nAufgefuellt ({laeufe} Samen ab {samen}; gezogene Moden 2,2-5 kHz: {min(zahl)}-{max(zahl)}):")
    print("  Terzpegel p/F Modell - Messung, 2,5 / 3,15 / 4 / 5 / 6,3 / 8 / 10 kHz (Mittel, in Klammern die Streuung):")
    for h, (name, _) in enumerate(mg.MIKROFONE):
        print(f"    {name:5s}: " + " ".join(f"{v:+5.1f}" for v in pe[:, h].mean(axis=0) - mess[h])
              + "   (" + " ".join(f"{v:.1f}" for v in pe[:, h].std(axis=0)) + ")")
    print("  Stegkurven, rms dB in 1-2,2 / 2,2-3 / 3-4 / 4-5 / 2,5-5 kHz: " + " / ".join(f"{v:.2f}" for v in ee.mean(axis=0))
          + "   (groesster Lauf " + " / ".join(f"{v:.2f}" for v in ee.max(axis=0)) + ")")

    fa, sa2, qa, ga, st, n, _ = fuelle(samen)
    neu = json.loads(json.dumps(k))
    for j in range(len(f), len(fa)):
        neu["modes"].append({"f": float(fa[j]), "eta": float(2 * sa2[j] / (2 * np.pi * fa[j])), "airShare": 0.0, "materialShares": [],
                             "bridge": [float(v) for v in form(qa[j:j + 1], xs)[0]],
                             "radiation": [[float(g.real), float(g.imag)] for g in ga[j]]})
    neu["modes"].sort(key=lambda m: m["f"])
    neu["statistical"].update({"density": st["density"], "bridgeSpread": st["bridgeSpread"], "radiation": st["radiation"],
                               "seed": str(st["seed"])})
    pathlib.Path(ziel).mkdir(parents=True, exist_ok=True)
    (pathlib.Path(ziel) / "classical_guitar.lvinstbody").write_text(json.dumps(neu, indent=1) + "\n", encoding="utf-8")
    print(f"\n{ziel}/classical_guitar.lvinstbody: {len(neu['modes'])} Moden bis 5 kHz ({n} gezogen), statistisch {st['density']} /Hz, "
          f"Streuung am Steg {' / '.join(f'{v:.2f}' for v in st['bridgeSpread'])}, Abstrahlung {' / '.join(f'{v:.5f}' for v in st['radiation'])}")


# ---------------------------------------------------------------- richtung
def elie_band(x):
    return next(((a, b) for lo, hi, a, b in ELIE if lo <= x < hi), ELIE[0][2:] if x < 150 else ELIE_GESAMT)


def tan_elie(ps, f, samen=7):
    """|tan| je Band fest aus dem Kreuzterm, Vorzeichen gezogen. Eine Mode mit Formwert unter 1e-2 ist eine y-Mode
    (siehe `mit_y_moden`): ihr tan ist 1 / EPS."""
    z = ps.Zufall(samen)
    return np.array([(1.0 if z.gleich() < 0.5 else -1.0) * 10 ** (elie_band(x)[0] / 20) for x in f])


EPS = 1e-4


def mit_y_moden(kp):
    """Haengt je Mode eine zweite an, die den Steg nur parallel zur Decke bewegt (auf halbem Weg zur naechsten Mode,
    Daempfung wie die Mode, kein Abstrahlgewicht): Formwert parallel = Formwert der Mode mal kappa^(1/4) mit
    kappa = 10^(dyy / 10) - 10^(2 dzy / 10) des Bands - so traegt Yyy im Band, was ein Winkel je Mode nicht gibt.
    Im Pruefstand als Mode mit Formwert EPS * phi_y und tan = 1 / EPS (Yzz und Yzy bleiben auf EPS gleich)."""
    f, n = kp["f"], len(kp["f"])
    ordnung = np.argsort(f)
    fy, ey, sy = [], [], []
    for a, b in zip(ordnung[:-1], ordnung[1:]):
        dzy, dyy = elie_band(f[a])
        kappa = max(0.0, 10 ** (dyy / 10) - 10 ** (2 * dzy / 10))
        fy.append(0.5 * (f[a] + f[b])); ey.append(kp["eta"][a]); sy.append(EPS * kappa ** 0.25 * kp["steg"][a])
    neu = {"tasten": kp["tasten"], "orte": kp["orte"], "f": np.concatenate([f, fy]), "eta": np.concatenate([kp["eta"], ey]),
           "steg": np.concatenate([kp["steg"], np.array(sy)]), "G": np.concatenate([kp["G"], np.zeros((len(fy), kp["G"].shape[1]))])}
    return neu, n


def richtungen(ps, kp, welche):
    """-> (Koerper, tan je Mode)"""
    if welche == "scherrer":
        return kp, ps.richtung_tan(kp["f"])
    if welche == "elie":
        return kp, tan_elie(ps, kp["f"])
    neu, n = mit_y_moden(kp)
    return neu, np.concatenate([tan_elie(ps, kp["f"]), np.full(len(neu["f"]) - n, 1.0 / EPS)])


def richtung(ordner):
    ps = modul(P / "2026-11-15_S199_pruefstand_zupf.py", "pruefstand")
    if ordner:
        ps.KOERPER = pathlib.Path(ordner)
    kp = ps.lade_koerper("classical_guitar")
    punkt = kp["tasten"].index(64)
    hz = np.arange(60.0, 5000.0, 0.5)
    w = 2 * np.pi * hz
    e = lambda y, m: 10 * np.log10(np.mean(np.abs(y[m]) ** 2))                # noqa: E731
    namen = ("scherrer", "elie", "elie-y")
    werte = {}
    for n in namen:
        k2, t = richtungen(ps, kp, n)
        werte[n] = ps.koerper_kurven2(k2, punkt, w, t)[0]
    print("Saite 1 (E4). Yzy - Yzz, Yyy - Yzz in dB je Band: Elie 2012 | scherrer (Pruefstand heute) | elie (|tan| je Band aus dem "
          "Kreuzterm) | elie-y (dazu Moden nur parallel zur Decke)")
    for lo, hi, dzy, dyy in ELIE + [(1000, 4000) + ELIE_GESAMT]:
        m = (hz >= lo) & (hz < hi)
        print(f"{lo:5d}-{hi:5d} | {dzy:6.1f} {dyy:6.1f} | " + " | ".join(
            f"{e(werte[n][:, 1], m) - e(werte[n][:, 0], m):6.1f} {e(werte[n][:, 2], m) - e(werte[n][:, 0], m):6.1f}" for n in namen))
    m = (hz > 150) & (hz < 300)
    for n in namen:
        Y = werte[n]
        i = np.where(m)[0][np.argmax(np.abs(Y[m, 0]))]
        print(f"staerkste Spitze 150-300 Hz ({hz[i]:.0f} Hz), {n}: Yzy - Yzz {20 * np.log10(abs(Y[i, 1]) / abs(Y[i, 0])):6.1f} dB, "
              f"Yyy - Yzz {20 * np.log10(abs(Y[i, 2]) / abs(Y[i, 0])):6.1f} dB   (Elie an T1 -15,4 / -22,5)")
    print("Re(Yyy) < 0 irgendwo: " + ", ".join(f"{n} {'ja' if (werte[n][:, 2].real < 0).any() else 'nein'}" for n in namen))


# ---------------------------------------------------------------- ton
def ton(ordner, welche, winkel):
    ps = modul(P / "2026-11-15_S199_pruefstand_zupf.py", "pruefstand")
    ps.KOERPER = pathlib.Path(ordner)
    if welche != "scherrer":
        lade = ps.lade_koerper
        zustand = {}

        def lade_neu(name):
            zustand["kp"], zustand["tan"] = richtungen(ps, lade(name), welche)
            return zustand["kp"]

        ps.lade_koerper = lade_neu
        ps.richtung_tan = lambda f, samen=7: zustand["tan"]
    puffer = io.StringIO()
    with contextlib.redirect_stdout(puffer):
        ps.gitarre(True, False, winkel)
    text = puffer.getvalue()
    print(text, end="")
    z = [x.split() for x in text.splitlines() if x.startswith("Taste")]
    tasten, l = [int(x[1]) for x in z], np.array([float(x[6]) for x in z])
    frueh, spaet = np.array([float(x[13]) for x in z]), np.array([float(x[15]) for x in z])
    paare = [(abs(l[i] - l[j]), tasten[i], tasten[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= 4]
    g = max(paare)
    print(f"KQ34 a: Spanne {l.max() - l.min():.1f} LU (leiseste Taste {tasten[int(l.argmin())]}, lauteste {tasten[int(l.argmax())]}), "
          f"groesster Sprung {g[0]:.1f} LU ({g[1]}/{g[2]}), mittlerer {np.median([p[0] for p in paare]):.1f} LU; "
          f"Lautheit im Mittel {l.mean():.2f} LUFS; Abfall frueh / spaet Median {np.nanmedian(frueh):.1f} / {np.nanmedian(spaet):.1f} dB/s")
    print("je Taste (Lautheit LUFS, Abfall frueh, spaet): " + "; ".join(f"{t} {a:.1f} {b:.1f} {c:.1f}" for t, a, b, c in zip(tasten, l, frueh, spaet)))


if __name__ == "__main__":
    opts = dict(a.split("=") for a in sys.argv[1:] if "=" in a)
    args = [a for a in sys.argv[1:] if "=" not in a]
    if args[0] == "dichte":
        dichte(args[1], args[2], int(opts.get("samen", 1)), int(opts.get("laeufe", 8)), opts.get("strahl", "terz"))
    elif args[0] == "richtung":
        richtung(args[1] if len(args) > 1 else None)
    elif args[0] == "ton":
        ton(args[1], args[2], float(opts.get("winkel", 30)))

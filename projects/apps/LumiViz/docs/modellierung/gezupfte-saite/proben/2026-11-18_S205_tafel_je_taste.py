"""S205 - UL3 a / UL3-1 a: die Tafel je Taste fuer Harfe und Cembalo ueber den ganzen Umfang, mit der Fassung fuer kurze
Umlaeufe aus `2026-11-17_S204_kurzer_umlauf.py` (das Glied fuer den gebrochenen Verzug steht MIT in der Anpassung,
viele Startwerte) und dem Glied nach UL3-1 a (Thiran 5).

Aufruf: python -X utf8 -B <dieses Skript> <Harfe|Cembalo> [glied=T5,T3,T1] [tasten=96,104]     -> Konsole

Je Taste (ein Bund, keine Lage laengs der Saite - die Tafel traegt alle Zahlen der Taste):
  Verlust  wie UL3 a (Ordnung 8, sonst 4 / 2)
  Kette    M Biquad-Allpaesse; M steigt (Abel-Smith-Zahl, dann + 2, * 1,5, * 2; bei kurzen Umlaeufen 1, 2, 3, 4, 6, 8), bis
           alle zaehlenden Teiltoene in 3 Cent liegen und das Abklingen in der Schwelle (UL2 a)
  Glied    der Reihe nach T5, T3, T1 - das erste, das die Schwelle haelt, zaehlt. Ein Glied hoeherer Ordnung braucht
           mehr Verzug (Thiran N: N - 0,5 Takte und mehr); an den hoechsten Tasten bleibt dafuer neben der Kette kein Platz.
           An Harfe und Cembalo gibt es keine Lage laengs der Saite, das Glied darf also je Taste verschieden sein.
  Starts   Abel & Smith bis zur 1,02- und 1,3-fachen Grenze; bei M <= 4 dazu 10 zufaellige (fester Samen)
  Anpassung wie `2026-11-18_S205_gebunden_stuetzstellen.py` mit einer einzigen Stuetzstelle und Grad 0; angepasst wird
           bis zur 1,5-fachen Grenze von UL1 a, gezaehlt bis 100 1/s. RMAX 0,99.
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
import time
sys.dont_write_bytecode = True
import numpy as np

_spec = importlib.util.spec_from_file_location("gs", pathlib.Path(__file__).with_name("2026-11-18_S205_gebunden_stuetzstellen.py"))
gs = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(gs)
bq, ue, uu = gs.bq, gs.ue, gs.uu
weg2 = bq.weg2
FS = gs.FS
gs.RAND = 1.5
ZIEL = 3.0
FEHLER = {}                            # Starts, die nicht rechneten (Meldung -> Zahl), am Ende ausgegeben


def versuche(s, M, g, rng):
    """beste Anpassung mit M Biquads -> (Cent, Abklingen %, alle in der Schwelle, r max) oder None"""
    f_d = min(s["fz"][-1] * 1.02, 19500.0)
    starts = [bq.start(s["rest"], f_d, M), bq.start(s["rest"], min(s["fz"][-1] * 1.3, 19500.0), M)]
    if M <= 4:
        for _ in range(10):
            starts.append((np.clip(bq.RMAX / (1 + np.exp(-rng.normal(-0.5, 1.2, M))), 0.02, bq.RMAX - 1e-3),
                           np.clip(np.pi / (1 + np.exp(-rng.normal(-1.0, 1.5, M))), 1e-3, np.pi - 1e-3)))
    best = None
    for r0, th0 in starts:
        try:
            f, _ = gs.gebunden([s], M, 0, r0, th0, g)
        except (np.linalg.LinAlgError, ValueError) as ex:  # ein Start, der nicht rechnet, zaehlt nicht
            FEHLER[type(ex).__name__ + ": " + str(ex)[:80]] = FEHLER.get(type(ex).__name__ + ": " + str(ex)[:80], 0) + 1
            continue
        r, th = f(0.0)
        v = gs.werte(s, r, th, g)
        if v is None:
            continue
        e = (v[0], v[3], v[1] == v[2], float(r.max()), r, th)
        if best is None or (not best[2], best[0]) > (not e[2], e[0]):
            best = e
    return best


def suche(s, GL):
    """-> (Glied, M, (Cent, Abklingen %, in der Schwelle, r max, r, th)) oder None; dazu die Abel-Smith-Zahl"""
    nap = len(uu.as_biquads(s["rest"], min(s["fz"][-1] * 1.02, 19500.0))[0])
    leiter = [1, 2, 3, 4, 6, 8] if nap <= 1 else sorted({nap, nap + 2, int(np.ceil(1.5 * (nap + 2))), 2 * (nap + 2)})
    rng = np.random.default_rng(205)
    bestes = None
    for g in GL:
        for M in leiter:
            e = versuche(s, M, g, rng)
            if e is None:
                continue
            if bestes is None or (not bestes[2][2], bestes[2][0]) > (not e[2], e[0]):
                bestes = (g, M, e)
            if e[0] <= ZIEL and e[2]:
                break
        if bestes is not None and bestes[2][0] <= ZIEL and bestes[2][2]:
            break
    return bestes, nap


def entwurf(sa, L, GL=(("T", 5), ("T", 3), ("T", 1))):
    """Die Tafel einer Taste zum Einsetzen in einen Pruefstand: b, a (Verlust), r, th (Kette), N (ganzzahliger Verzug),
    bt, at (Glied), dazu Glied, Cent am starren Steg."""
    s = gs.stelle(sa, L, 0.0, 1, None)
    bestes, _ = suche(s, GL)
    g, M, e = bestes
    r, th = e[4], e[5]
    rest2 = s["rest"] - ue.phase_von(ue.biquads(-r, np.cos(th)))
    f1 = s["fn"][0]
    w1 = 2 * np.pi * f1 / FS
    tot = np.interp(f1, gs.F, rest2)
    N = int(np.floor(tot / w1 - gs.unten(g)))
    soll = tot - w1 * N
    Dl = tot / w1 - N
    pf = lambda D: ue.phase_von(ue.antwort_ba(*gs.glied(g, D), np.array([1e-6, w1])))[1]  # noqa: E731
    for _ in range(8):
        Dl -= (pf(Dl) - soll) / ((pf(Dl + 1e-6) - pf(Dl - 1e-6)) / 2e-6)
    bt, at = gs.glied(g, Dl)
    return dict(b=s["b"], a=s["a"], r=r, th=th, N=N, bt=bt, at=at, glied=g, cent=e[0], abkl=e[1], stelle=s)


def taste(inst, name, sa, L, GL):
    s = gs.stelle(sa, L, 0.0, 1, None)
    if s["b"] is None or len(s["fz"]) == 0:
        print(f"{inst} {name}: nicht rechenbar (zu wenige Teiltoene)", flush=True)
        return None
    umlauf = FS / s["fn"][0]
    t0 = time.perf_counter()
    bestes, nap = suche(s, GL)
    if bestes is None:
        print(f"{inst} {name}: f1 {s['fn'][0]:8.2f} Hz, Umlauf {umlauf:6.1f} | passt mit keinem Glied in den Umlauf", flush=True)
        return None
    g, M, e = bestes
    ok = e[0] <= ZIEL and e[2]
    print(f"{inst} {name:5s}: f1 {s['fn'][0]:8.2f} Hz, Umlauf {umlauf:6.1f}, zaehlend {int((s['fz'] > 0).sum()):3d} (mit Zugabe) bis {s['fz'][-1]:6.0f} Hz, "
          f"Verlust Ordnung {len(s['b']) - 1}, Abel-Smith {nap:3d} | {g[0]}{g[1]}, M {M:3d}: {e[0]:6.2f} Cent, Abklingen {e[1]:6.2f} % "
          f"{'in der Schwelle' if e[2] else 'AUSSERHALB'}, r max {e[3]:.3f} | {'ok' if ok else 'AUSNAHME'} | {time.perf_counter() - t0:4.0f} s", flush=True)
    return name, g, M, e, ok


if __name__ == "__main__":
    inst = sys.argv[1]
    opts = dict(a.split("=") for a in sys.argv[2:])
    GL = [(x[0], int(x[1:])) for x in opts.get("glied", "T5,T3,T1").split(",")]
    NUR = {"T" + x for x in opts["tasten"].split(",")} if "tasten" in opts else None
    print(f"{inst}: Tafel je Taste, Glieder der Reihe nach {' '.join(g[0] + str(g[1]) for g in GL)}, Ziel {ZIEL} Cent, RMAX {bq.RMAX}, RAND {gs.RAND}")
    erg = []
    for i, name, sa, L in weg2.tasten_umfang():
        if i != inst or (NUR and name not in NUR):
            continue
        x = taste(i, name, sa, L, GL)
        if x is not None:
            erg.append(x)
    ok = [x for x in erg if x[4]]
    print(f"\n===== {inst}: {len(erg)} Tasten, in 3 Cent und Abklingen in der Schwelle: {len(ok)}")
    for gl in GL:
        z = [x for x in ok if x[1] == gl]
        if z:
            ms = np.array([x[2] for x in z])
            print(f"  {gl[0]}{gl[1]}: {len(z)} Tasten ({z[0][0]} bis {z[-1][0]}), Biquads Median {np.median(ms):.0f}, hoechstens {ms.max()}, "
                  f"groesster Fehler {max(x[3][0] for x in z):.2f} Cent, Abklingen groesster Fehler {max(x[3][1] for x in z):.2f} %")
    for k, n in FEHLER.items():
        print(f"  {n} Starts rechneten nicht: {k}")
    for x in erg:
        if not x[4]:
            print(f"  AUSNAHME {x[0]}: {x[1][0]}{x[1][1]}, M {x[2]}, {x[3][0]:.2f} Cent, Abklingen {x[3][1]:.2f} % {'in der Schwelle' if x[3][2] else 'ausserhalb'}")

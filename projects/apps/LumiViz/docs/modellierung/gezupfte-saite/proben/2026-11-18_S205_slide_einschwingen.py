"""S205 - Kipppunkt von UL3-1 a: Einschwingen des Glieds fuer den gebrochenen Verzug, wenn der ganzzahlige Verzug
waehrend eines Slides umspringt. Zeitbereich, starrer Steg, fs 48 kHz.

Aufruf: python -X utf8 -B <dieses Skript> [saiten=1,4] [glied=T1,T3,T5] [tempo=50,10] [von=5] [weite=2]
        saiten: Gitarrensaiten (1 = hohe E-Saite); tempo: Millisekunden je Halbton; von: Bund am Anfang; weite: Buende

Schleife:  v[n] = Verlust( Kette( Glied( v[n - M(n) - k] ) ) ) + Anregung[n]
  Kette     Biquad-Allpaesse, gebunden angepasst wie in `2026-11-18_S205_gebunden_stuetzstellen.py` (Grad 3), Beiwerte
            jeden Takt aus der Lage; Direktform 1
  Verlust   Ordnung 8, b und a jeden Takt linear zwischen den Nachbarbuenden; Direktform 1
  Glied     Thiran N, Beiwerte jeden Takt aus dem Verzug, Direktform 1 (Eingang aus dem Speicher der Schleife - beim
            Umspringen von M stimmt nur das Gedaechtnis des Ausgangs nicht mehr)
            einfach   ein Glied, Verzug in [N - 0,5 ; N + 0,5), M springt ohne Korrektur
            doppelt   zwei Glieder nebeneinander, das zweite mit Verzug in [N ; N + 1), ueberblendet: jedes Glied hat
                      das Gewicht 0, wenn sein M springt (nach Van Duyne, Jaffe, Scandalis & Stilson 1997 - aus dem
                      Gedaechtnis zitiert, nicht nachgelesen)
Lage:      von Bund `von` in `weite` * tempo ms nach `von + weite`, gehalten, zurueck (Beginn 0,15 s und 0,35 s);
           der Verzug folgt dem Grundton Takt fuer Takt.
Gemessen:  Band ueber 1,15 * dem hoechsten zaehlenden Teilton (UL1 a) - dort ist der Ton nach 0,15 s verklungen, was
           dort in den Rampen steht, ist Stoerung. Pegel des Bands in den Rampen gegen den Pegel des ganzen Signals
           im selben Fenster (Mittel und groesster 1-ms-Wert), dazu derselbe Wert ohne Slide (Boden) und der Pegel
           nach dem Slide gegen den Ton ohne Slide (0,45-0,55 s).
           Einschwingen ueber das ganze Band (nur "einfach"): der Ausgang des Glieds im Lauf gegen ein Glied, das je
           Abschnitt mit festem M schon 400 Takte vorher mitlaeuft (eingeschwungen), am selben Eingang; Pegel der
           Differenz gegen den Ausgang in den Rampen (Mittel und groesster 1-ms-Wert). Das ist die Stoerung, die je
           Sprung in die Schleife gelangt; sie laeuft danach um und klingt mit dem Ton ab.
Schreibt nichts.
"""
import sys
import pathlib
import importlib.util
import time
sys.dont_write_bytecode = True
import numpy as np
import scipy.signal as sg

_spec = importlib.util.spec_from_file_location("gs", pathlib.Path(__file__).with_name("2026-11-18_S205_gebunden_stuetzstellen.py"))
gs = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(gs)
bq, ue, uu, ps = gs.bq, gs.ue, gs.uu, gs.ps
FS, F = gs.FS, gs.F
NT = int(0.6 * FS)
T = np.arange(NT) / FS
O = 4096


def lage(von, weite, ms):
    d = weite * ms / 1000.0
    return von + weite * (np.clip((T - 0.15) / d, 0, 1) - np.clip((T - 0.35) / d, 0, 1)), d


def verzug(s, r, th):
    """Verzug in Takten, den Speicher und Glied am Grundton zusammen stellen muessen"""
    rest2 = s["rest"] - ue.phase_von(ue.biquads(-r, np.cos(th)))
    f1 = s["fn"][0]
    return np.interp(f1, F, rest2) / (2 * np.pi * f1 / FS)


def vorbereiten(sa, mens, nb, bund, f, g, u):
    """-> Beiwerte je Takt: Kette (A1, A2), Verlust (B, A), Verzug gesamt"""
    ue_, inv = np.unique(u, return_inverse=True)
    rt = [f(x / nb) for x in ue_]
    R = np.array([x[0] for x in rt])[inv]
    TH = np.array([x[1] for x in rt])[inv]
    k = np.minimum(np.floor(u).astype(int), nb - 1)
    t = (u - k)[:, None]
    bb = np.array([s["b"] / s["a"][0] for s in bund])
    aa = np.array([s["a"] / s["a"][0] for s in bund])
    B = (1 - t) * bb[k] + t * bb[k + 1]
    A = (1 - t) * aa[k] + t * aa[k + 1]
    ug = np.arange(np.floor(u.min()), np.ceil(u.max()) + 1e-9, 0.125)
    dg = []
    for x in ug:
        s = gs.stelle(sa, mens, float(x), nb, bund)
        dg.append(verzug(s, *f(s["x"])))
    return -2 * R * np.cos(TH), R * R, B, A, np.interp(u, ug, dg)


def glied_tafel(D, N, lo):
    """-> M je Takt, Beiwerte a je Takt (Thiran N, Verzug in [lo ; lo + 1))"""
    M = np.floor(D - lo).astype(int)
    d = D - M
    du, inv = np.unique(np.round(d, 9), return_inverse=True)
    a = np.array([ue.thiran(x, N)[1] for x in du])[inv]
    return M.tolist(), a.tolist(), d


def laufe(A1, A2, B, A, D, N, doppelt, f0):
    nbq = A1.shape[1]
    A1, A2, B, A = A1.tolist(), A2.tolist(), B.tolist(), A.tolist()
    Ma, Ta, da = glied_tafel(D, N, N - 0.5)
    if doppelt:
        Mb, Tb, _ = glied_tafel(D, N, float(N))
        wa = (1 - 2 * np.abs(da - N)).tolist()
    anr = np.exp(-0.5 * ((T - 0.002) / 0.00008) ** 2) - np.exp(-0.5 * ((T - 0.002 - 0.3 / f0) / 0.00008) ** 2)
    anr = anr.tolist()
    puf = [0.0] * (NT + O)
    ya = [0.0] * N
    yb = [0.0] * N
    x1 = [0.0] * nbq
    x2 = [0.0] * nbq
    y1 = [0.0] * nbq
    y2 = [0.0] * nbq
    nl = len(B[0])
    xl = [0.0] * nl
    yl = [0.0] * nl
    v = np.zeros(NT)
    yg = np.zeros(NT)
    for n in range(NT):
        a = Ta[n]
        base = O + n - Ma[n]
        acc = 0.0
        for k in range(N + 1):
            acc += a[N - k] * puf[base - k]
        for k in range(1, N + 1):
            acc -= a[k] * ya[k - 1]
        ya.insert(0, acc)
        ya.pop()
        y = acc
        yg[n] = acc
        if doppelt:
            a = Tb[n]
            base = O + n - Mb[n]
            acc = 0.0
            for k in range(N + 1):
                acc += a[N - k] * puf[base - k]
            for k in range(1, N + 1):
                acc -= a[k] * yb[k - 1]
            yb.insert(0, acc)
            yb.pop()
            y = wa[n] * y + (1 - wa[n]) * acc
        c1 = A1[n]
        c2 = A2[n]
        for i in range(nbq):
            out = c2[i] * y + c1[i] * x1[i] + x2[i] - c1[i] * y1[i] - c2[i] * y2[i]
            x2[i] = x1[i]
            x1[i] = y
            y2[i] = y1[i]
            y1[i] = out
            y = out
        b = B[n]
        a = A[n]
        xl.insert(0, y)
        xl.pop()
        acc = 0.0
        for k in range(nl):
            acc += b[k] * xl[k]
        for k in range(1, nl):
            acc -= a[k] * yl[k - 1]
        yl.insert(0, acc)
        yl.pop()
        v[n] = acc + anr[n]
        puf[O + n] = v[n]
    return v, yg


def einschwingen(v, yg, D, N, rampen):
    """Differenz des Glieds im Lauf gegen ein je Abschnitt eingeschwungenes Glied -> dB Mittel, groesster 1-ms-Wert"""
    puf = np.concatenate([np.zeros(O), v]).tolist()
    M = np.floor(D - (N - 0.5)).astype(int)
    gr = np.concatenate([[0], np.nonzero(np.diff(M))[0] + 1, [NT]])
    e = np.zeros(NT)
    for s, t in zip(gr[:-1], gr[1:]):
        m = int(M[s])
        ya = [0.0] * N
        for n in range(max(0, s - 400), t):
            a = ue.thiran(min(max(D[n] - m, N - 0.9), N + 3.0), N)[1]
            base = O + n - m
            acc = 0.0
            for k in range(N + 1):
                acc += a[N - k] * puf[base - k]
            for k in range(1, N + 1):
                acc -= a[k] * ya[k - 1]
            ya.insert(0, acc)
            ya.pop()
            if n >= s:
                e[n] = yg[n] - acc
    mm = int(0.001 * FS)
    kurz = np.convolve(e ** 2, np.ones(mm) / mm, mode="same")
    mi, sp = [], []
    for a_, b_ in rampen:
        sl = slice(int(a_ * FS), int(b_ * FS))
        ges = np.mean(yg[sl] ** 2)
        mi.append(db(np.mean(e[sl] ** 2) / ges))
        sp.append(db(kurz[sl].max() / ges))
    return max(mi), max(sp)


def db(x):
    return 10 * np.log10(x + 1e-300)


def messe(v, hp, rampen):
    e = sg.sosfilt(hp, v)
    m = int(0.001 * FS)
    mittel, spitze = [], []
    for a_, b_ in rampen:
        sl = slice(int(a_ * FS), int(b_ * FS))
        ges = np.mean(v[sl] ** 2)
        mittel.append(db(np.mean(e[sl] ** 2) / ges))
        kurz = np.convolve(e ** 2, np.ones(m) / m, mode="same")[sl]
        spitze.append(db(kurz.max() / ges))
    sl = slice(int(0.45 * FS), int(0.55 * FS))
    return max(mittel), max(spitze), db(np.mean(v[sl] ** 2))


if __name__ == "__main__":
    opts = dict(a.split("=") for a in sys.argv[1:])
    SAITEN = [int(x) for x in opts.get("saiten", "1,4").split(",")]
    GL = [("T", int(x[1:])) for x in opts.get("glied", "T1,T3,T5").split(",")]
    TEMPO = [float(x) for x in opts.get("tempo", "50,10").split(",")]
    VON = int(opts.get("von", "5"))
    WEITE = int(opts.get("weite", "2"))
    alle = gs.saiten("Gitarre")
    print(f"Slide Bund {VON} -> {VON + WEITE} -> {VON}; Stoerung = Band ueber 1,15 * hoechstem zaehlendem Teilton, dB gegen das ganze Signal im Fenster der Rampe")
    for nr in SAITEN:
        name, sa, mens, nb = alle[6 - nr]
        bund = [gs.stelle(sa, mens, float(k), nb, None) for k in range(nb + 1)]
        halb = [gs.stelle(sa, mens, k + 0.5, nb, bund) for k in range(nb)]
        st = sorted(bund + halb, key=lambda s: s["u"])
        nap = [len(uu.as_biquads(t["rest"], min(t["fz"][-1] * 1.02, 19500.0))[0]) for t in bund]
        M = max(8, max(nap) + 2)
        r0, th0 = bq.start(bund[0]["rest"], min(bund[0]["fz"][-1] * 1.02, 19500.0), M)
        fg = min(1.15 * max(bund[k]["fz"][-1] for k in range(VON, VON + WEITE + 1)), 21000.0)
        hp = sg.butter(8, fg, "highpass", fs=FS, output="sos")
        f0 = bund[VON]["fn"][0]
        print(f"\n### Gitarre {name}: f1 {f0:.1f} -> {bund[VON + WEITE]['fn'][0]:.1f} Hz, Kette {M} Biquads, Band ueber {fg:.0f} Hz", flush=True)
        print("  Glied Art     | ms je Halbton | Spruenge von M | Stoerung in den Rampen: Mittel / groesster 1-ms-Wert dB | ohne Slide (Boden) dB | Pegel nach dem Slide gegen ohne Slide dB")
        for g in GL:
            t0 = time.perf_counter()
            f, _ = gs.gebunden(st, M, 3, r0, th0, g)
            N = g[1]
            u0 = np.full(NT, float(VON))
            fest = vorbereiten(sa, mens, nb, bund, f, g, u0)
            print(f"  (T{N}: Anpassung {time.perf_counter() - t0:.0f} s)", flush=True)
            for ms in TEMPO:
                u, d = lage(VON, WEITE, ms)
                bew = vorbereiten(sa, mens, nb, bund, f, g, u)
                rampen = [(0.15, 0.15 + d + 0.005), (0.35, 0.35 + d + 0.005)]
                spr = int(np.sum(np.diff(np.floor(bew[4] - (N - 0.5))) != 0))
                for art, dop in (("einfach", False), ("doppelt", True)):
                    vs, _ = laufe(*fest, N, dop, f0)
                    vb, yg = laufe(*bew, N, dop, f0)
                    ein = "" if dop else " | Einschwingen ganzes Band %7.1f / %7.1f dB" % einschwingen(vb, yg, bew[4], N, rampen)
                    bm, bs_, bp = messe(vs, hp, rampen)
                    mm, msp, mp = messe(vb, hp, rampen)
                    print(f"  T{N}    {art} | {ms:5.0f} | {spr:3d} | {mm:7.1f} / {msp:7.1f} | {bm:7.1f} / {bs_:7.1f} | {mp - bp:+6.2f}{ein}", flush=True)

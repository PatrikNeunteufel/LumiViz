"""Gegenrechnung zu den Messverfahren (Auftrag 2026-11-16_16_einzeltoene_messgrundlagen, Abschnitt Gesucht 5).

Jeder Teil erzeugt ein synthetisches Signal mit bekannter Vorgabe und misst es mit dem Verfahren aus dem Bericht.
Aufruf: python gegenrechnung.py [teil ...]   (ohne Argument alle Teile; Teile: A B C D E F G H I)
Fester Startwert, damit gleiche Zahlen herauskommen.
"""
import sys

import numpy as np
from scipy.signal import lfilter, fftconvolve

RNG_SEED = 20261117


# ------------------------------------------------------------------ BS.1770: K-Filter und Lautheit
def k_filter_koeff(fs):
    """K-Filter fuer beliebige Abtastrate ueber die bilineare Abbildung des analogen Vorbilds.
    Die Kennwerte (fc, G, Q) sind die der verbreiteten Umsetzung libebur128; geprueft wird unten, dass sie bei
    48 kHz die Koeffizienten aus BS.1770-5 Tabelle 1 und 2 wiedergeben."""
    fc, G, Q = 1681.974450955533, 3.999843853973347, 0.7071752369554196
    K = np.tan(np.pi * fc / fs)
    Vh = 10 ** (G / 20)
    Vb = Vh ** 0.4996667741545416
    a0 = 1 + K / Q + K * K
    b1 = [(Vh + Vb * K / Q + K * K) / a0, 2 * (K * K - Vh) / a0, (Vh - Vb * K / Q + K * K) / a0]
    a1 = [1.0, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    fc, Q = 38.13547087602444, 0.5003270373238773
    K = np.tan(np.pi * fc / fs)
    a0 = 1 + K / Q + K * K
    b2 = [1.0, -2.0, 1.0]
    a2 = [1.0, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    return (b1, a1), (b2, a2)


def k_gefiltert(x, fs):
    (b1, a1), (b2, a2) = k_filter_koeff(fs)
    return lfilter(b2, a2, lfilter(b1, a1, x))


def lautheit(kanaele, fs):
    """kanaele: Liste von Signalen (G_i = 1). Gibt (Momentary-Max, Short-term-Max, integriert mit Gates) in LUFS."""
    y2 = sum(k_gefiltert(np.asarray(c, float), fs) ** 2 for c in kanaele)
    cs = np.concatenate([[0.0], np.cumsum(y2)])

    def gleitend_max(w):
        n = int(round(w * fs))
        if len(y2) < n:
            return -200.0
        m = (cs[n:] - cs[:-n]) / n
        return -0.691 + 10 * np.log10(max(m.max(), 1e-30))

    n = int(round(0.4 * fs))
    schritt = int(round(0.1 * fs))
    z = np.array([(cs[j + n] - cs[j]) / n for j in range(0, len(y2) - n + 1, schritt)])
    l = -0.691 + 10 * np.log10(np.maximum(z, 1e-30))
    za = z[l > -70]
    if len(za) == 0:
        integ = -200.0
    else:
        gr = -0.691 + 10 * np.log10(za.mean()) - 10
        zr = z[(l > -70) & (l > gr)]
        integ = -0.691 + 10 * np.log10(zr.mean())
    return gleitend_max(0.4), gleitend_max(3.0), integ


def teil_A():
    print("== A  Lautheit nach BS.1770: Bezugswert und abklingende Toene ==")
    (b1, a1), (b2, a2) = k_filter_koeff(48000)
    tab1 = [1.53512485958697, -2.69169618940638, 1.19839281085285, -1.69065929318241, 0.73248077421585]
    tab2 = [1.0, -2.0, 1.0, -1.99004745483398, 0.99007225036621]
    d1 = max(abs(u - v) for u, v in zip(b1 + a1[1:], tab1))
    d2 = max(abs(u - v) for u, v in zip(b2 + a2[1:], tab2))
    print(f"K-Filter bei 48 kHz gegen BS.1770-5 Tab. 1/2: groesste Abweichung Stufe 1 {d1:.1e}, Stufe 2 {d2:.1e}")
    for fs in (48000, 44100, 96000):
        t = np.arange(int(10 * fs)) / fs
        s = np.sin(2 * np.pi * 997 * t)
        m, st, i = lautheit([s], fs)
        m2, _, _ = lautheit([s, s], fs)
        print(f"  fs {fs}: Sinus 997 Hz 0 dBFS in einem Kanal: M-max {m:+.3f}, S-max {st:+.3f}, I {i:+.3f} LUFS "
              f"(Soll -3.01); derselbe Ton in beiden Kanaelen (Mono als Stereo): {m2:+.3f}")
    fs = 48000
    print("Abklingender Sinus 1 kHz (Anfangsamplitude 1, Einsatz 5 ms), Abfall D in dB/s, Datei 12 s, danach Stille:")
    print("  D dB/s | M-max  S-max  I(gated) | M-max - Dauerton | Formel 10lg[(1-10^(-DW/10))/(DW ln10/10)], W=0,4 s")
    dauer = -3.010
    for D in (1, 3, 10, 30, 60, 150):
        t = np.arange(int(12 * fs)) / fs
        env = 10 ** (-D * t / 20) * np.minimum(t / 0.005, 1)
        s = env * np.sin(2 * np.pi * 997 * t)
        m, st, i = lautheit([s], fs)
        DW = D * 0.4
        formel = 10 * np.log10((1 - 10 ** (-DW / 10)) / (DW * np.log(10) / 10))
        print(f"  {D:6.0f} | {m:+6.2f} {st:+6.2f} {i:+7.2f}   | {m - dauer:+6.2f}           | {formel:+6.2f}")
    print("Integrierte Lautheit haengt an der Dateilaenge (D = 10 dB/s):")
    for lang in (1.0, 2.0, 4.0, 8.0):
        t = np.arange(int(lang * fs)) / fs
        s = 10 ** (-10 * t / 20) * np.sin(2 * np.pi * 997 * t)
        m, st, i = lautheit([s], fs)
        print(f"  Datei {lang:3.0f} s: I {i:+6.2f}, M-max {m:+6.2f}")


# ------------------------------------------------------------------ Gang ueber den Umfang, Paare bis n Halbtoene
def kennzahlen(tasten, l, n=4):
    tasten, l = np.asarray(tasten), np.asarray(l)
    d = [abs(l[i] - l[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= n]
    d = np.array(d) if d else np.array([np.nan])
    return l.max() - l.min(), np.median(d), d.max(), len(d)


def glatt7(tasten, werte, halb=3):
    return np.array([np.median([w for t, w in zip(tasten, werte) if abs(t - t0) <= halb]) for t0 in tasten])


def teil_B():
    print("== B  Gang ueber den Umfang: Spanne, Paare bis 4 Halbtoene, Glaettung ==")
    rng = np.random.default_rng(RNG_SEED)
    tasten = np.arange(29, 101)  # F1..E7, 72 Tasten
    gang = -1.0 * (tasten - 60) / 12  # -1 LU je Oktave
    spruenge = {55: 6.0, 56: 6.0, 57: 6.0, 80: -8.0}  # bekannte Stoerungen (eine Mode unter drei Tasten, ein Loch)
    wahr = gang + np.array([spruenge.get(int(k), 0.0) for k in tasten])
    sp, med, mx, n = kennzahlen(tasten, wahr)
    print(f"Vorgabe ohne Streuung, alle Tasten: Spanne {sp:.2f} (Gang 6,0 + Stoerungen), mittlerer Sprung {med:.2f}, "
          f"groesster {mx:.2f} LU, {n} Paare")
    g = glatt7(tasten, wahr)
    print(f"  nach gleitendem Median ueber 7 Tasten: Abweichung an Taste 56 {g[tasten == 56][0] - gang[tasten == 56][0]:+.2f}, "
          f"an Taste 80 {g[tasten == 80][0] - gang[tasten == 80][0]:+.2f} LU (Vorgabe +6 bzw. -8: beide Stoerungen entfernt)")
    for breite in (3, 4, 5):
        st = gang + np.where((tasten >= 55) & (tasten < 55 + breite), 6.0, 0.0)
        gg = glatt7(tasten, st)
        print(f"  Stoerung +6 LU ueber {breite} Nachbartasten: nach der Glaettung an Taste 56 {gg[tasten == 56][0] - gang[tasten == 56][0]:+.2f} LU")
    print("Mit Streuung sigma je Ton (Spieler, Raum), 2000 Laeufe: Mittel und 95-%-Quantil")
    print("  sigma | Auswahl        | Spanne       | mittlerer Sprung | groesster Sprung | Theorie mittl. = 0,954 sigma (nur Streuung)")
    for sigma in (0.0, 1.0, 2.0, 3.0, 4.0):
        for name, auswahl in (("jede Taste", tasten), ("jede 3. Taste", tasten[::3])):
            werte = []
            for _ in range(2000 if sigma > 0 else 1):
                l = wahr + rng.normal(0, sigma, len(tasten))
                m = np.isin(tasten, auswahl)
                werte.append(kennzahlen(tasten[m], l[m]))
            w = np.array(werte)
            q = lambda c: f"{w[:, c].mean():5.1f} / {np.quantile(w[:, c], 0.95):5.1f}"
            print(f"  {sigma:4.1f}  | {name:14s} | {q(0)} | {q(1)}    | {q(2)}    | {0.954 * sigma:4.2f}")
    print("Reine Streuung ohne Gang, 72 Tasten, mittlerer Sprung Paare <=4 HT (Pruefung der Theorie):")
    for sigma in (1.0, 3.0):
        w = [kennzahlen(tasten, rng.normal(0, sigma, len(tasten)))[1] for _ in range(2000)]
        print(f"  sigma {sigma}: gemessen {np.mean(w):.3f}, Theorie {0.954 * sigma:.3f}")


# ------------------------------------------------------------------ Raum: Pegelstreuung je Teilton
def teil_C():
    print("== C  Raum: Streuung des Pegels je Ton und Teilton bei Direkt-zu-Hall-Verhaeltnis DRR ==")
    rng = np.random.default_rng(RNG_SEED + 1)
    n = 200000
    print("Ein Teilton (stationaer), |d + r|^2 mit r komplex normal, E|r|^2 = 10^(-DRR/10):")
    print("  DRR dB | Std 10lg|H|^2 | Std Differenz zweier Toene | Theorie diffus 5,57 / 7,88 dB")
    for drr in (20, 10, 5, 0, -5, -10, -100):
        r = (rng.normal(size=n) + 1j * rng.normal(size=n)) * np.sqrt(10 ** (-drr / 10) / 2)
        d = 1.0 if drr > -100 else 0.0
        L = 10 * np.log10(np.abs(d + r) ** 2)
        L2 = 10 * np.log10(np.abs(d + np.roll(r, 1)) ** 2)
        print(f"  {drr:6.0f} | {L.std():13.2f} | {(L - L2).std():13.2f}")
    print("Ganzer Ton mit K Teiltoenen (Amplitude 1/k), Lautheit ~ 10lg Summe |a_k H_k|^2, H_k unabhaengig je Teilton:")
    print("  DRR dB | K=1   K=3   K=10  K=30   (Std des Tonpegels in dB) | groesster Sprung erwartet bei 72 Tasten, Paare<=4 HT, K=10")
    tasten = np.arange(72)
    for drr in (10, 5, 0, -5, -100):
        zeile = []
        for K in (1, 3, 10, 30):
            a2 = 1.0 / np.arange(1, K + 1) ** 2
            r = (rng.normal(size=(20000, K)) + 1j * rng.normal(size=(20000, K))) * np.sqrt(10 ** (-drr / 10) / 2)
            d = 1.0 if drr > -100 else 0.0
            L = 10 * np.log10((a2 * np.abs(d + r) ** 2).sum(axis=1))
            zeile.append(L.std())
        a2 = 1.0 / np.arange(1, 11) ** 2
        mx = []
        for _ in range(300):
            r = (rng.normal(size=(72, 10)) + 1j * rng.normal(size=(72, 10))) * np.sqrt(10 ** (-drr / 10) / 2)
            d = 1.0 if drr > -100 else 0.0
            L = 10 * np.log10((a2 * np.abs(d + r) ** 2).sum(axis=1))
            sp, med, m, _ = kennzahlen(tasten, L)
            mx.append((m, med, sp))
        mx = np.array(mx)
        print(f"  {drr:6.0f} | " + "  ".join(f"{z:4.2f}" for z in zeile)
              + f"   | groesster {mx[:, 0].mean():4.1f} (95 %: {np.quantile(mx[:, 0], .95):4.1f}), mittlerer {mx[:, 1].mean():3.1f}, Spanne {mx[:, 2].mean():4.1f} LU")


# ------------------------------------------------------------------ Abklingen je Teilton
def huellkurve_heterodyn(x, fs, f, rahmen=0.01, glatt=0.02):
    t = np.arange(len(x)) / fs
    z = x * np.exp(-2j * np.pi * f * t)
    n = int(glatt * fs)
    z = np.convolve(z, np.ones(n) / n, mode="same")
    s = int(rahmen * fs)
    e = np.abs(z[::s]) ** 2 * 4  # Energie der Huellkurve (Amplitude^2)
    return e, rahmen


def zwei_geraden(tt, y):
    """Zwei unabhaengige Geraden mit bestem Knick (wie Teiltonmessung.hpp: frueh >= 1/20, spaet >= 1/5 des Bereichs)."""
    n = len(y)
    best = None
    for k in range(max(3, n // 20), n - max(3, n // 5)):
        p1 = np.polyfit(tt[:k], y[:k], 1)
        p2 = np.polyfit(tt[k:], y[k:], 1)
        r = ((np.polyval(p1, tt[:k]) - y[:k]) ** 2).sum() + ((np.polyval(p2, tt[k:]) - y[k:]) ** 2).sum()
        if best is None or r < best[0]:
            best = (r, k, p1, p2)
    _, k, p1, p2 = best
    knick = (p2[1] - p1[1]) / (p1[0] - p2[0]) if p1[0] != p2[0] else tt[k]
    return -p1[0], -p2[0], knick


def bereich(e, rahmen, ueber_rausch_db=10):
    """Von der Spitze bis zuletzt 10 dB ueber dem Boden (Median der letzten 10 %), wie Teiltonmessung/Lundeby Schritt 2."""
    L = 10 * np.log10(e + 1e-30)
    i0 = int(np.argmax(L))
    boden = np.median(L[int(0.9 * len(L)):])
    ueber = np.where(L > boden + ueber_rausch_db)[0]
    i1 = ueber[-1] if len(ueber) else len(L) - 1
    return i0, max(i1, i0 + 10), boden


def schroeder_zwei(e, rahmen, i0, i1):
    """Schroeder auf [i0, i1), spaete Gerade, deren Energie abziehen, frueh neu (wie zweiStufen in Teiltonmessung)."""
    seg = e[i0:i1]
    tt = np.arange(len(seg)) * rahmen
    L = 10 * np.log10(np.cumsum(seg[::-1])[::-1] / seg.sum())
    m = len(L) - max(3, int(0.1 * len(L)))  # die letzten 10 dB vor dem Ende biegt die Abschneidung
    ziel = L[:m]
    D1, D2, kn = zwei_geraden(tt[:m], ziel)
    # Energie der spaeten Stufe (Gerade im Pegel, aus den Rohdaten spaet) abziehen
    sp = slice(int(len(seg) * 0.6), len(seg))
    p = np.polyfit(tt[sp], 10 * np.log10(seg[sp] + 1e-30), 1)
    rest = np.maximum(seg - 10 ** (np.polyval(p, tt) / 10), 0)
    Lr = 10 * np.log10(np.cumsum(rest[::-1])[::-1] / max(rest.sum(), 1e-30) + 1e-30)
    gut = Lr > -30
    Dfrueh = -np.polyfit(tt[gut][:max(5, gut.sum())], Lr[gut], 1)[0] if gut.sum() > 4 else np.nan
    return Dfrueh, D2, kn


def zwei_komponenten(e, rahmen, i0, i1, koharent):
    """Modell mit zwei Komponenten und Boden, im Pegel (dB) ausgeglichen:
    inkohaerent  e = E1 10^(-a t/10) + E2 10^(-b t/10) + N         (zwei Polarisationen mit Frequenzversatz)
    kohaerent    e = (A1 10^(-a t/20) + A2 10^(-b t/20))^2 + N     (gleiche Frequenz, gleiche Phase)
    Gibt (a, b, Knick) zurueck; a, b in dB/s."""
    from scipy.optimize import least_squares
    seg = e[i0:]
    tt = np.arange(len(seg)) * rahmen
    y = 10 * np.log10(seg + 1e-30)
    N0 = np.median(seg[int(0.9 * len(seg)):])

    def modell(p):
        l1, a, l2, b, ln = p
        if koharent:
            m = (10 ** (l1 / 20) * 10 ** (-a * tt / 20) + 10 ** (l2 / 20) * 10 ** (-b * tt / 20)) ** 2
        else:
            m = 10 ** (l1 / 10) * 10 ** (-a * tt / 10) + 10 ** (l2 / 10) * 10 ** (-b * tt / 10)
        return 10 * np.log10(m + 10 ** (ln / 10))

    a0 = max(-np.polyfit(tt[: max(5, (i1 - i0) // 10)], y[: max(5, (i1 - i0) // 10)], 1)[0], 1.0)
    p0 = [y[0], a0, y[0] - 20, a0 / 8, 10 * np.log10(N0 + 1e-30)]
    r = least_squares(lambda p: modell(p) - y, p0, bounds=([-200, 0.1, -200, 0.01, -300], [50, 2000, 50, 2000, 50]))
    l1, a, l2, b, _ = r.x
    if a < b:
        l1, a, l2, b = l2, b, l1, a
    knick = (l1 - l2) / (a - b)
    return a, b, knick


def raum_ir(fs, T, drr_db, rng, dauer=None):
    dauer = dauer or 1.2 * T
    t = np.arange(int(dauer * fs)) / fs
    h = rng.normal(size=len(t)) * 10 ** (-3 * t / T)
    h[: int(0.002 * fs)] = 0  # erste Reflexion nach 2 ms
    h *= np.sqrt(10 ** (-drr_db / 10) / (h ** 2).sum())
    h[0] += 1.0
    return h


def teilton(fs, dauer, f, stufen, df=0.0, rng=None, rauschen_db=None):
    """stufen: [(Pegel dB, Abfall dB/s)], zweite Stufe mit Frequenzversatz df (zwei Polarisationen)."""
    t = np.arange(int(dauer * fs)) / fs
    x = np.zeros_like(t)
    for i, (pegel, D) in enumerate(stufen):
        x += 10 ** (pegel / 20) * 10 ** (-D * t / 20) * np.sin(2 * np.pi * (f + i * df) * t + 0.3 * i)
    x *= np.minimum(t / 0.003, 1)
    if rauschen_db is not None:
        x += rng.normal(size=len(t)) * 10 ** (rauschen_db / 20)
    return x


def teil_D():
    print("== D  Doppelabklingen je Teilton: Vorgabe gegen Messung, ohne und mit Raum und Rauschen ==")
    rng = np.random.default_rng(RNG_SEED + 2)
    fs, f = 16000, 330.0
    vorgaben = [("Gitarre mittel", [(0, 40.0), (-20, 4.0)]), ("Harfe Diskant", [(0, 150.0), (-25, 20.0)])]
    for name, st in vorgaben:
        (p1, D1), (p2, D2) = st
        kn = (p1 - p2) / (D1 - D2)
        print(f"-- {name}: Vorgabe frueh {D1} dB/s, spaet {D2} dB/s, Knick {kn:.3f} s (Pegel {p1 - D1 * kn:.1f} dB)")
        faelle = [("ohne Raum, Rauschen -90 dB", None, -90, 0.0),
                  ("ohne Raum, Rauschen -60 dB", None, -60, 0.0),
                  ("zwei Polarisationen df = 0,7 Hz", None, -90, 0.7),
                  ("Raum T=1,5 s, DRR +10 dB", (1.5, 10), -90, 0.0),
                  ("Raum T=1,5 s, DRR 0 dB", (1.5, 0), -90, 0.0),
                  ("Raum T=3,0 s, DRR 0 dB", (3.0, 0), -90, 0.0),
                  ("Raum T=0,4 s (Studio), DRR +5 dB", (0.4, 5), -90, 0.0)]
        dauer = min(60.0 / D2 * 1.3, 12.0)
        print("   Fall                               | Pegel-Geraden frueh/spaet/Knick | Schroeder frueh/spaet/Knick | Zwei Komponenten (Modell passend) frueh/spaet/Knick")
        for fall, raum, rausch, df in faelle:
            x = teilton(fs, dauer, f, st, df=df, rng=rng, rauschen_db=None)
            if raum:
                x = fftconvolve(x, raum_ir(fs, raum[0], raum[1], rng))[: len(x)]
            x = x + rng.normal(size=len(x)) * 10 ** (rausch / 20)
            e, r = huellkurve_heterodyn(x, fs, f)
            i0, i1, boden = bereich(e, r)
            seg = 10 * np.log10(e[i0:i1] + 1e-30)
            tt = np.arange(len(seg)) * r
            a1, a2, k1 = zwei_geraden(tt, seg)
            s1, s2, k2 = schroeder_zwei(e, r, i0, i1)
            c1, c2, k3 = zwei_komponenten(e, r, i0, len(e), koharent=(df == 0.0))
            print(f"   {fall:35s}| {a1:6.1f} {a2:6.1f} {k1:6.3f}          | {s1:6.1f} {s2:6.1f} {k2:6.3f}       "
                  f"| {c1:6.1f} {c2:6.1f} {k3:6.3f}   (Bereich {seg[0] - seg[-1]:.0f} dB)")
    print("Messbarer Bereich eines einstufigen Teiltons im Raum (Direktanteil bis zum Schnitt mit dem Hall, Energie):")
    print("  Formel dL = a/(a-b) * (DRR + 10lg(a/b)) fuer a > b; a, b Abklingraten Teilton und Raum (Erwartungswert,")
    print("  inkohaerent); Rechnung: 30 Raeume, Stelle, an der die Huellkurve 3 dB ueber dem Direktanteil liegt")
    print("  Teilton T60 | Raum T60 | DRR | Formel dB | Rechnung Median (10.-90. Perzentil) dB")
    for Tp, Tr, drr in ((0.4, 1.5, 10), (0.4, 1.5, 0), (1.0, 2.0, 0), (0.3, 0.4, 5), (0.5, 2.0, 5)):
        a, b = 60 / Tp, 60 / Tr
        formel = a / (a - b) * (drr + 10 * np.log10(a / b))
        werte = []
        for _ in range(30):
            x = teilton(fs, max(Tp, Tr) * 2.5, f, [(0, a)])
            y = fftconvolve(x, raum_ir(fs, Tr, drr, rng))[: len(x)]
            e, r = huellkurve_heterodyn(y, fs, f)
            ed, _ = huellkurve_heterodyn(x, fs, f)
            L, Ld = 10 * np.log10(e + 1e-30), 10 * np.log10(ed + 1e-30)
            i0 = int(np.argmax(Ld))
            ab = np.where((L[i0:] - Ld[i0:]) > 3)[0]
            werte.append(-(Ld[i0 + ab[0]] - Ld[i0]) if len(ab) else np.nan)
        print(f"  {Tp:11.1f} | {Tr:8.1f} | {drr:3.0f} | {formel:9.1f} | {np.nanmedian(werte):5.1f} "
              f"({np.nanquantile(werte, .1):5.1f} … {np.nanquantile(werte, .9):5.1f})")
    print("Teilton langsamer als der Raum (a < b): Steigung ueber 0..-40 dB, 30 Raeume")
    for Tp, Tr, drr in ((3.0, 1.5, 0), (6.0, 2.0, 0), (2.0, 1.5, -5)):
        a = 60 / Tp
        st = []
        for _ in range(30):
            x = teilton(fs, Tp * 1.2, f, [(0, a)])
            y = fftconvolve(x, raum_ir(fs, Tr, drr, rng))[: len(x)]
            e, r = huellkurve_heterodyn(y, fs, f)
            L = 10 * np.log10(e + 1e-30)
            i0 = int(np.argmax(L))
            j = i0 + np.where(L[i0:] < L[i0] - 40)[0][0]
            tt = np.arange(i0, j) * r
            st.append(-np.polyfit(tt, L[i0:j], 1)[0])
        print(f"  Teilton T60 {Tp} s ({a:.0f} dB/s), Raum {Tr} s, DRR {drr:+d}: gemessen {np.median(st):.1f} dB/s "
              f"({np.quantile(st, .1):.1f} … {np.quantile(st, .9):.1f})")


def teil_E():
    print("== E  Warum Huellkurve und Teilton ein anderes Vorzeichen geben ==")
    fs, f0 = 16000, 220.0
    t = np.arange(int(4 * fs)) / fs
    K = np.arange(1, 13)

    def ton(amp, D):
        return sum(a * 10 ** (-d * t / 20) * np.sin(2 * np.pi * k * f0 * t) for k, a, d in zip(K, amp, D))

    def frueh(x):
        n = int(0.02 * fs)
        e = np.array([np.mean(x[i:i + n] ** 2) for i in range(0, len(x) - n, n)])
        L = 10 * np.log10(e + 1e-30)
        L -= L.max()
        m = int(np.argmax(L))
        idx = [i for i in range(m, len(L)) if -25 <= L[i] <= -5]
        idx = [i for i in idx if i <= next((j for j in range(m, len(L)) if L[j] < -25), len(L))]
        return -np.polyfit(np.array(idx) * 0.02, L[idx], 1)[0]

    print("Modell: jeder Teilton faellt 25 % schneller als in der Aufnahme. Huellkurve wie S195 (-5..-25 dB, 20-ms-Bloecke).")
    print("  Aufnahme Amplitude / Abfall je k | Modell Amplitude | Huellkurve Aufnahme / Modell dB/s | Huellkurve sagt")
    for p_amp_auf, p_D, p_amp_mod in ((0.3, 1.0, 2.0), (0.0, 1.5, 2.0), (0.0, 1.5, 1.0), (1.0, 1.0, 1.0)):
        D_auf = 4.0 * K ** p_D
        D_mod = 1.25 * D_auf
        fa = frueh(ton(1.0 / K ** p_amp_auf, D_auf))
        fm = frueh(ton(1.0 / K ** p_amp_mod, D_mod))
        print(f"  1/k^{p_amp_auf:.1f} / 4 k^{p_D:.1f} dB/s            | 1/k^{p_amp_mod:.1f}          | {fa:6.1f} / {fm:6.1f}"
              f"                   | Modell faellt {'LANGSAMER (falsches Vorzeichen)' if fm < fa else 'schneller (richtig)'}")
    print("Mechanismus 1 — schwacher Grundton in der Aufnahme (Abstrahlung unter der tiefsten Koerpermode), D = 4 k dB/s:")
    D_auf = 4.0 * K
    amp_auf = 1.0 / np.sqrt(K)
    for g1 in (1.0, 0.3, 0.1, 0.03):
        a = amp_auf.copy()
        a[0] = g1
        fa = frueh(ton(a, D_auf))
        fm = frueh(ton(1.0 / K, 1.25 * D_auf))
        print(f"  Grundton der Aufnahme {20 * np.log10(g1):+5.1f} dB: Huellkurve Aufnahme {fa:5.1f}, Modell {fm:5.1f} dB/s -> "
              f"{'FALSCHES Vorzeichen' if fm < fa else 'richtig'}")
    print("Mechanismus 2 — Raum an der Aufnahme (Modell trocken), ein Teilton 440 Hz:")
    rng = np.random.default_rng(RNG_SEED + 6)
    for D_wahr, D_mod, T, drr in ((40.0, 34.0, 2.0, 0.0), (40.0, 34.0, 2.0, 10.0), (15.0, 13.0, 2.0, 0.0), (15.0, 13.0, 1.0, 0.0)):
        x = 10 ** (-D_wahr * t / 20) * np.sin(2 * np.pi * 440 * t)
        xr = fftconvolve(x, raum_ir(fs, T, drr, rng))[: len(t)]
        m = 10 ** (-D_mod * t / 20) * np.sin(2 * np.pi * 440 * t)
        fa, fm = frueh(xr), frueh(m)
        print(f"  Aufnahme trocken {D_wahr:.0f} dB/s, Raum T {T} s DRR {drr:+.0f} dB, Modell {D_mod:.0f} dB/s (langsamer): "
              f"Huellkurve Aufnahme {fa:5.1f}, Modell {fm:5.1f} -> {'FALSCHES Vorzeichen' if fm > fa else 'richtig'}")


# ------------------------------------------------------------------ Inharmonizitaet
def bh(n):
    a = [0.35875, 0.48829, 0.14128, 0.01168]
    k = np.arange(n) / (n - 1)
    return a[0] - a[1] * np.cos(2 * np.pi * k) + a[2] * np.cos(4 * np.pi * k) - a[3] * np.cos(6 * np.pi * k)


def schaetze_B(x, fs, f_nenn, kmax, fenster_s, ab_s=0.05, B_max=0.02):
    n = int(fenster_s * fs)
    seg = x[int(ab_s * fs): int(ab_s * fs) + n] * bh(n)
    N = 4 * 2 ** int(np.ceil(np.log2(n)))
    S = 20 * np.log10(np.abs(np.fft.rfft(seg, N)) + 1e-30)
    df = fs / N
    ks, fk = [], []
    f0, B = f_nenn, 0.0
    for k in range(1, kmax + 1):
        mitte = k * f0 * np.sqrt(1 + B * k * k)
        breite = max(0.4 * f0, 3 * fs / n)
        lo, hi = int((mitte - breite / 2) / df), int((mitte + breite / 2) / df)
        if hi >= len(S) - 1:
            break
        i = lo + int(np.argmax(S[lo:hi]))
        a, b, c = S[i - 1], S[i], S[i + 1]
        p = 0.5 * (a - c) / (a - 2 * b + c)
        ks.append(k)
        fk.append((i + p) * df)
        if len(ks) >= 3:
            kk, ff = np.array(ks, float), np.array(fk)
            A = np.vstack([np.ones_like(kk), kk ** 2]).T
            c0, c1 = np.linalg.lstsq(A, (ff / kk) ** 2, rcond=None)[0]
            f0, B = np.sqrt(c0), max(c1 / c0, 0.0)
    kk, ff = np.array(ks, float), np.array(fk)
    A = np.vstack([np.ones_like(kk), kk ** 2]).T
    (c0, c1), res, _, _ = np.linalg.lstsq(A, (ff / kk) ** 2, rcond=None)
    return np.sqrt(c0), c1 / c0


def teil_F():
    print("== F  Tonhoehe und Inharmonizitaet B: Vorgabe gegen Schaetzung ==")
    rng = np.random.default_rng(RNG_SEED + 3)
    fs = 44100
    print("Abweichung Teilton n gegen harmonisch: 1200 log2 sqrt(1 + B n^2) Cent ~ 866 B n^2")
    for B in (1e-5, 1e-4):
        print(f"  B = {B:g}: n=5 {1200 * np.log2(np.sqrt(1 + B * 25)):.2f}, n=10 {1200 * np.log2(np.sqrt(1 + B * 100)):.2f}, "
              f"n=20 {1200 * np.log2(np.sqrt(1 + B * 400)):.2f}, n=30 {1200 * np.log2(np.sqrt(1 + B * 900)):.2f} Cent")
    print("Synthetischer Ton f0 110 Hz, Teiltoene 1/k, Abfall 3 + 0,8k dB/s, 20 Laeufe (Phasen zufaellig); dazu je Teilton")
    print("ein zufaelliger Frequenzversatz sigma_c Cent (Kopplung an Steg und Koerper, die das Gesetz nicht kennt).")
    print("Theorie (nur Versatz): Std(B) = 2 ln2/1200 * sigma_c / sqrt(Summe (k^2 - Mittel k^2)^2)")
    print("  B Vorgabe | Rauschen dB | sigma_c | Fenster s | Teiltoene | B Mittel   B Std     Theorie  | f0 Fehler Cent Mittel/Std")
    t = np.arange(int(1.6 * fs)) / fs
    for B in (1e-5, 1e-4):
        for rausch, sc, fenster in ((-70, 0.0, 0.5), (-40, 0.0, 0.25), (-40, 0.0, 0.5), (-70, 1.0, 0.5), (-70, 3.0, 0.5)):
            for kmax in (5, 10, 20, 30):
                bs, fe = [], []
                for _ in range(20):
                    vers = 2 ** (rng.normal(0, sc, 41) / 1200) if sc > 0 else np.ones(41)
                    x = sum((1 / k) * 10 ** (-(3 + 0.8 * k) * t / 20)
                            * np.sin(2 * np.pi * k * 110 * np.sqrt(1 + B * k * k) * vers[k] * t + rng.uniform(0, 6.28))
                            for k in range(1, 41))
                    x += rng.normal(size=len(t)) * 10 ** (rausch / 20)
                    f0, b = schaetze_B(x, fs, 110 * 2 ** (rng.uniform(-30, 30) / 1200), kmax, fenster)
                    bs.append(b)
                    fe.append(1200 * np.log2(f0 / 110))
                k2 = np.arange(1, kmax + 1) ** 2.0
                theo = 2 * np.log(2) / 1200 * sc / np.sqrt(((k2 - k2.mean()) ** 2).sum())
                print(f"  {B:9.0e} | {rausch:11d} | {sc:7.1f} | {fenster:9.2f} | {kmax:9d} | {np.mean(bs):9.2e} {np.std(bs):9.1e} "
                      f"{theo:8.1e} | {np.mean(fe):+.3f} / {np.std(fe):.3f}")


# ------------------------------------------------------------------ Spektrum: Schwerpunkt im Raum
def teil_G():
    print("== G  Spektrum: Teiltonpegel relativ zum Grundton und spektraler Schwerpunkt bei Raum (DRR) ==")
    rng = np.random.default_rng(RNG_SEED + 4)
    K = np.arange(1, 21)
    a2 = 1.0 / K ** 2
    sz_wahr = (K * a2).sum() / a2.sum()
    print(f"Vorgabe: 20 Teiltoene 1/k, Schwerpunkt {sz_wahr:.3f} (in Vielfachen von f0)")
    print("  DRR dB | Std Pegel Teilton k gegen Teilton 1 dB | Schwerpunkt Mittel / Std (relativ)")
    for drr in (20, 10, 0, -10, -100):
        r = (rng.normal(size=(20000, 20)) + 1j * rng.normal(size=(20000, 20))) * np.sqrt(10 ** (-drr / 10) / 2)
        d = 1.0 if drr > -100 else 0.0
        p = a2 * np.abs(d + r) ** 2
        rel = 10 * np.log10(p[:, 4] / p[:, 0])
        sz = (K * p).sum(axis=1) / p.sum(axis=1)
        print(f"  {drr:6.0f} | {rel.std():6.2f}                                  | {sz.mean() / sz_wahr:5.3f} / {sz.std() / sz_wahr:5.3f}")


# ------------------------------------------------------------------ Einschwinggeraeusch
def teil_H():
    print("== H  Einschwinggeraeusch: Trennung vom harmonischen Teil durch Abzug der Teiltoene ==")
    rng = np.random.default_rng(RNG_SEED + 5)
    fs, f0 = 44100, 196.0
    t = np.arange(int(1.5 * fs)) / fs
    K = np.arange(1, 16)
    harm = sum((1 / k) * 10 ** (-(4 + k) * t / 20) * np.sin(2 * np.pi * k * f0 * t + 0.7 * k) for k in K)
    for pegel in (-10.0, -20.0, -30.0):
        burst = rng.normal(size=len(t)) * np.exp(-t / 0.008) * 10 ** (pegel / 20)
        x = harm + burst
        # Modell: je Teilton A e^{-s t} sin(w t + phi) aus 0,05..0,5 s, Frequenz bekannt; Abfall aus der Huellkurve
        fit = slice(int(0.05 * fs), int(0.5 * fs))
        rekon = np.zeros_like(t)
        for k in K:
            e, r = huellkurve_heterodyn(x, fs, k * f0, rahmen=0.005)
            tt = np.arange(len(e)) * r
            m = (tt > 0.05) & (tt < 0.5)
            s = -np.polyfit(tt[m], 10 * np.log10(e[m] + 1e-30), 1)[0] * np.log(10) / 20
            basis = np.vstack([np.exp(-s * t) * np.sin(2 * np.pi * k * f0 * t), np.exp(-s * t) * np.cos(2 * np.pi * k * f0 * t)]).T
            c = np.linalg.lstsq(basis[fit], x[fit], rcond=None)[0]
            rekon += basis @ c
        rest = x - rekon
        w = slice(0, int(0.05 * fs))
        print(f"  Geraeusch {pegel:+.0f} dB: Energie der ersten 50 ms wahr {10 * np.log10(np.sum(burst[w] ** 2)):+.2f} dB, "
              f"Rest nach Abzug {10 * np.log10(np.sum(rest[w] ** 2)):+.2f} dB, Verhaeltnis Geraeusch/Ton wahr "
              f"{10 * np.log10(np.sum(burst[w] ** 2) / np.sum(harm[w] ** 2)):+.2f}, gemessen "
              f"{10 * np.log10(np.sum(rest[w] ** 2) / np.sum(rekon[w] ** 2)):+.2f} dB")


TEILE = {"A": teil_A, "B": teil_B, "C": teil_C, "D": teil_D, "E": teil_E, "F": teil_F, "G": teil_G, "H": teil_H}

if __name__ == "__main__":
    for name in (sys.argv[1:] or list(TEILE)):
        TEILE[name]()
        print()

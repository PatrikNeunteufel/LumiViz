"""S222, 34k-7d-1 — die Bezugsdatei des Pruefstands fuer den Kern `pluckedString`, Rechenfassung 2.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> [<ausgabe>]
    Ohne Angabe schreibt es `projects/apps/LumiViz/tests/unit/UnitTests/werte/zupf_pruefstand.txt`.
                           python -X utf8 -B <dieses Skript> zeitschritt <fall> [<fall> ...]
    Die Pruefung von 7d-1: die Faelle (eine Saite) im Zeitschritt wie der Kandidat S200 gegen den Bezug.

Was der Kern in 34k-7d rechnen soll, hier geschlossen je Frequenz (Physik des Pruefstands
`2026-11-15_S199_pruefstand_zupf.py`, dessen Funktionen es nimmt; die Vorgaben stehen HIER, nach den Entscheiden, und
weichen an zwei Stellen von den festen Zahlen des Pruefstands ab, deren Ausgaben gueltig bleiben sollen):

  Saite(n)   je Fall die Werte der gespielten Saite bzw. der Saiten eines Chors (Laute, Oud: am selben Stegpunkt,
             zusammen gezupft; im Einklang die Laenge um -+halbe Verstimmung, Laute 4, Oud 3 Cent; Oktavsaite gleiche
             Laenge). Umlauf genau exp(-2 i k L) (der Kern: das Umlauf-Filter, UL2 a).
  Zupf       Kraftstufe F0 = T d L / (x_p (L - x_p)) je Saite, Fingerbreite als Gauss-Tiefpass, Winkel teilt auf die
             zwei Polarisationen (ZU1 a: der Kern rechnet die Quelle genau so, mit einer FFT). Vorgaben:
               Gitarre  d = 4 mm * 10^(Stufe/20), Neigung 0,16 dB/Okt je dB Stufe um 1 kHz (GZ6 a), 16 cm fest
                        (ZW1 a), 30 Grad, Breite 7,5 mm        [der Pruefstand rechnet 2 mm und mit Kappe]
               Laute, Oud  d = 2 mm, 16 cm fest, 30 Grad, 7,5 mm
               Harfe    0,36 L, d = 4 mm, 2 m/s beim Abloesen, 25 Grad, 7,5 mm (Chadefaux u. a. 2012); die
                        Geschwindigkeit als verteilter Kraftstoss ueber das Dreieck (`quelle`, `impuls`)
                                                         [der Pruefstand: eine Rampe, die nie endet — an den Teiltoenen
                                                          gleich, `2026-11-26_S222_harfe_tempo.py`]
               Cembalo  Kraft 1,07 N an C4 bei ff, -2,8 dB je Oktave (KQ30 a, GZ13 a), Zupfpunkt Taskin, 20 Grad,
                        3 mm                                     [der Pruefstand rechnet 2 N fest]
  Steg       in der Schleife die Moden der Koerperdatei (GZ5 a) mit ihrer Richtung (`bridgeAngle`, sonst die Annahme
             nach Scherrer wie der Pruefstand: `richtung_von`), dazu der Daempfer ueber 5 kHz als sieben breite Moden
             (Q 1,5; Mitten 6 ... 23 kHz) NUR senkrecht, Gewichte phi^2 >= 0 mit NNLS an
             Re Y = n(f) phi_rms^2 / 4 angepasst (GZ5-1 a; phi_rms senkrecht = bridgeSpread * cos(bridgeAngle) des
             statistischen Teils). Wie der Kandidat S200 (`daempfer_moden`), dort mit einer zweiten Richtung (0,4) —
             der `Steg` aus 34k-7b traegt keine; ueber 5 kHz strahlt nur die Senkrechte.
  Ausgang    die Stegkraft senkrecht zur Decke (Summe ueber die Saiten am Punkt) — das gibt der Kern aus (GZ5 a).
             Daraus am Hoerort p = Fz * Hz mit ALLEN Moden (samt gezogenem statistischem Teil wie `alleModen`).

Je Fall stehen in der Datei: die Eingaben (zum Pruefen der Vorgaben des Kerns), die Gewichte des Daempfers, je Teilton
1-12 der Stegkraft Frequenz und Pegel frueh (0,05-0,45 s) / spaet (1-2 s) wie der Kandidat (`teiltoene`), der Pegel der
Stegkraft in 0-0,1 / 0-1 / 1-3 s (dB re 1 N, Effektivwert), derselbe Pegel im Band 20 Hz - 20 kHz (S224, PK1 a: ueber das
ganze Band traegt ihn die statische Last unter 20 Hz — an der Gitarre 1-3 s 21-40 dB ueber dem Band; `kraftband`), die Lautheit am Hoerort (Momentary-Maximum,
BS.1770) und der Pegel 1-2 s gegen 0-0,4 s. Zahlen mit 17 Stellen.
"""
import importlib.util
import json
import pathlib
import sys

import numpy as np
import scipy.optimize as opt

ROOT = pathlib.Path.cwd()
P = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben"
spec = importlib.util.spec_from_file_location("pruefstand", P / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)
spec = importlib.util.spec_from_file_location("kandidat", P / "2026-11-16_S200_zeitbereich_kandidat.py")
kd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(kd)

ZUPFSTELLE = 0.16                      # ZW1 a
DAEMPFER_MITTEN = np.array([6000.0, 7500.0, 9500.0, 12000.0, 15000.0, 19000.0, 23000.0])
DAEMPFER_ETA = 1 / 1.5


def daempfer(st, punkt):
    """GZ5-1 a als sieben breite Moden, wie `daempfer_moden` des Kandidaten S200 (dasselbe Raster und Gewicht)."""
    streuung = st["bridgeSpread"][punkt] * np.cos(np.radians(st.get("bridgeAngle", 0.0)))
    f = np.concatenate([np.arange(500.0, 4501.0, 250.0), np.arange(5250.0, 20001.0, 250.0)])
    ziel = np.where(f >= st["fromHz"], (st["density"] + st["rise"] * (f - st["fromHz"])) * streuung ** 2 / 4, 0.0)
    w, wn = 2 * np.pi * f[:, None], 2 * np.pi * DAEMPFER_MITTEN[None, :]
    A = np.real(1j * w / (wn ** 2 - w ** 2 + 1j * DAEMPFER_ETA * wn * w))
    gew = np.where(f < st["fromHz"], 3.0, 1.0)
    phi2, _ = opt.nnls(A * gew[:, None], ziel * gew)
    return streuung, phi2


def dreieck_integral(kappa, a, L, d):
    """Integral_0^L y0(x) e^(-i kappa x) dx fuer das Dreieck y0 (Spitze d bei a, null an beiden Enden), geschlossen."""
    Ea, EL = np.exp(-1j * kappa * a), np.exp(-1j * kappa * L)
    return d * (Ea - 1) / (a * kappa ** 2) - d * (EL - Ea) / ((L - a) * kappa ** 2)


def quelle(sa, L, xp, d, breite, tempo, stufe, w, geschwindigkeit="impuls"):
    """Die Quelle g einer Saite am Steg (ZU1 a: so rechnet der Kern sie, mit einer FFT): Kraftstufe am Zupfpunkt,
    Fingerbreite, Neigung je Stufe, dazu die Geschwindigkeit beim Abloesen (siehe `stegkraft`)."""
    k, z0 = ps.saite_wellen(sa, w)
    T, m = sa["T"], sa["m"]
    breit = np.exp(-(w * breite / np.sqrt(T / m)) ** 2) * (ps.staerke_faktor(w, stufe) if stufe else 1.0)
    f = -(T * d * L / (xp * (L - xp))) / (1j * w) * breit
    if tempo and geschwindigkeit == "rampe":
        f = f * (1 - (tempo / d) / (1j * w))
    g = f / (2 * z0) * (np.exp(-1j * k * xp) - np.exp(-1j * k * (2 * L - xp)))
    if tempo and geschwindigkeit == "impuls":
        q = -m * (tempo / d) * breit                          # Kraftstoss je Laenge und Auslenkung: mu v0(x) = q y0(x)
        g = g + q / (2 * z0) * (dreieck_integral(k, xp, L, d) - np.exp(-2j * k * L) * dreieck_integral(-k, xp, L, d))
    return g


def stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, stufe, Y, w, geschwindigkeit="impuls"):
    """Die Kraft am Steg je Saite und Richtung (Frequenz x 2S), wie `ton_mit` des Pruefstands, mit der Neigung je Stufe
    (`staerke_faktor`, GZ6 a) und der Kraftstufe je Saite aus derselben Auslenkung.

    Die Geschwindigkeit beim Abloesen (`tempo`, Harfe): `impuls` (Vorgabe, S222) — die Anfangsgeschwindigkeit
    v0(x) = -(tempo / d) y0(x) (zur Ruhelage hin, dieselbe Form wie die Auslenkung) als verteilter Kraftstoss
    mu v0(x) delta(t), ueber das Dreieck geschlossen integriert (`dreieck_integral`); `rampe` — wie der Pruefstand
    f (1 - (tempo/d) / (i w)): gleich an den Teiltoenen, dazu eine Rampe aus der statischen Last, die nie endet
    (`2026-11-26_S222_harfe_tempo.py`)."""
    S, n = len(saiten), len(w)
    z = np.empty((n, 2 * S), dtype=complex)
    E = np.empty((n, 2 * S), dtype=complex)
    g = np.zeros((n, 2 * S), dtype=complex)
    a = np.radians(winkel)
    for i, (sa, L) in enumerate(zip(saiten, laengen)):
        k, z0 = ps.saite_wellen(sa, w)
        z[:, 2 * i] = z[:, 2 * i + 1] = z0
        E[:, 2 * i] = E[:, 2 * i + 1] = np.exp(-2j * k * L)
        gi = quelle(sa, L, xp, d, breite, tempo, stufe, w, geschwindigkeit)
        g[:, 2 * i], g[:, 2 * i + 1] = gi * np.cos(a), gi * np.sin(a)
    F = np.empty((n, 2 * S), dtype=complex)
    eins = np.eye(2 * S)
    for b0 in range(0, n, 8192):
        b = slice(b0, b0 + 8192)
        YZ = Y[b] * z[b][:, None, :]
        M = np.linalg.solve(eins[None] + YZ, 2 * YZ)
        vplus = np.linalg.solve(eins[None] + E[b][:, :, None] * (M - eins[None]), g[b][:, :, None])[:, :, 0]
        F[b] = z[b] * (2 * vplus - (M @ vplus[:, :, None])[:, :, 0])
    return F


KOERPER = {"gitarre": "classical_guitar", "laute": "lute", "oud": "oud", "harfe": "concert_harp", "cembalo": "harpsichord"}


def faelle():
    """(Name, Instrument, Taste, Stufe): Stichtasten tief / mitte / hoch je Instrument, an der Gitarre dazu Bund 19
    (ZW1 a) und eine leise Stufe (Neigung, GZ6 a). Die Laute ohne Chor 8 (UL2 a: Ausnahme Chor 8.1)."""
    return [("gitarre_40", "gitarre", 40, 0.0), ("gitarre_57", "gitarre", 57, 0.0), ("gitarre_57_pp", "gitarre", 57, -18.0),
            ("gitarre_76", "gitarre", 76, 0.0), ("gitarre_83", "gitarre", 83, 0.0),
            ("laute_38", "laute", 38, 0.0), ("laute_53", "laute", 53, 0.0), ("laute_65", "laute", 65, 0.0),
            ("oud_36", "oud", 36, 0.0), ("oud_45", "oud", 45, 0.0), ("oud_62", "oud", 62, 0.0),
            ("harfe_36", "harfe", 36, 0.0), ("harfe_60", "harfe", 60, 0.0), ("harfe_91", "harfe", 91, 0.0),
            ("cembalo_36", "cembalo", 36, 0.0), ("cembalo_60", "cembalo", 60, 0.0), ("cembalo_84", "cembalo", 84, 0.0)]


def eingaben(instrument, taste, stufe):
    """(Saiten, Laengen, Stegpunkt-Taste, x_p, d, Winkel, Breite, Tempo, Stufe der Neigung, Kraft F0 je Saite)."""
    if instrument == "gitarre":
        s = max(i for i, sa in enumerate(ps.GITARRE) if sa["taste"] <= taste)
        sa = dict(ps.GITARRE[s])
        L = ps.MENSUR * 2 ** (-(taste - sa["taste"]) / 12)
        d = 0.004 * 10 ** (stufe / 20)
        return [sa], [L], sa["taste"], ZUPFSTELLE, d, 30.0, 0.0075, 0.0, stufe
    if instrument in ("laute", "oud"):
        ch, mensur, cent = ps.choere(instrument)
        c = max(i for i, (t, _) in enumerate(ch) if t <= taste)
        saiten = ch[c][1]
        laenge = mensur * 2 ** (-(taste - ch[c][0]) / 12)
        gleich = len(saiten) == 2 and abs(saiten[0]["hz"] - saiten[1]["hz"]) < 1e-9
        laengen = [laenge * 2 ** ((-0.5 if i == 0 else 0.5) * cent / 1200) if gleich else laenge for i in range(len(saiten))]
        return saiten, laengen, ch[c][0], ZUPFSTELLE, 0.002, 30.0, 0.0075, 0.0, 0.0
    if instrument == "harfe":
        sa, L, xp, d, winkel, breite, tempo = ps.harfe_taste(taste)
        return [sa], [L], taste, xp, d, winkel, breite, tempo, 0.0
    sa, L, xp, _, winkel, breite, _ = ps.cembalo_taste(taste)
    kraft = 1.07 * 10 ** (-2.8 / 20 * (taste - 60) / 12)          # KQ30 a, GZ13 a
    d = kraft * xp * (L - xp) / (sa["T"] * L)
    return [sa], [L], taste, xp, d, winkel, breite, 0.0, 0.0


def zahl(x):
    return repr(float(x))


BAND_N = 1 << 18          # das Band ueber eine FFT auf 2^18 Werten (der Ton hat 180 000), wie der Test des Kerns


def im_band(x, von=20.0, bis=20000.0):
    """Der Zeitverlauf nur mit den Frequenzen von <= f <= bis (S224, PK1 a), auf BAND_N Werten, auf die Laenge von x."""
    X = np.fft.rfft(x, BAND_N)
    hz = np.fft.rfftfreq(BAND_N, 1 / ps.FS)
    X[(hz < von) | (hz > bis)] = 0.0
    return np.fft.irfft(X, BAND_N)[:len(x)]


def rechne(name, instrument, taste, stufe, kp, roh, w, aus):
    saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, neigung = eingaben(instrument, taste, stufe)
    punkt = kp["tasten"].index(stegtaste)
    datei = len(roh["modes"])
    tan = ps.richtung_von(kp)
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    S = len(saiten)
    Y, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt] * S)
    streuung, phi2 = daempfer(roh["statistical"], punkt)
    wn = 2 * np.pi * DAEMPFER_MITTEN
    yd = (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * DAEMPFER_ETA * wn[None, :] * w[:, None])) @ phi2
    for i in range(S):
        for j in range(S):
            Y[:, 2 * i, 2 * j] += yd
    F = stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, neigung, Y, w)
    Fz = F[:, 0::2].sum(axis=1)
    _, Hz, _ = ps.koerper_kurven2(kp, punkt, w, tan)                # Abstrahlung: alle Moden, nur die Senkrechte
    kraft = ps.zeit(Fz[:, None])[:, 0]
    p = ps.zeit(Fz[:, None] * Hz)
    f0 = 440.0 * 2 ** ((taste - 69) / 12)
    tt = kd.teiltoene(kraft, f0)
    v, e = ps.VORLAUF, ps.VORLAUF + 0.4

    def rms(von, bis):
        x = kraft[int(von * ps.FS):int(bis * ps.FS)]
        return 10 * np.log10(np.mean(x ** 2) + 1e-300)

    aus.append(f"fall {name} {instrument} taste {taste} stufe {zahl(stufe)} stegpunkt {stegtaste} saiten {S}")
    for i, (sa, L) in enumerate(zip(saiten, laengen)):
        werte = " ".join(f"{k} {zahl(sa[k])}" for k in ("T", "m", "EI", "etaB", "etaF", "etaA", "d", "rho") if k in sa)
        f0n = sa["T"] * d * L / (xp * (L - xp))
        aus.append(f"saite {i} laenge {zahl(L)} {werte} kraft {zahl(f0n)}")
    aus.append(f"zupf ort {zahl(xp)} auslenkung {zahl(d)} winkel {zahl(winkel)} breite {zahl(breite)} tempo {zahl(tempo)} "
               f"neigung {zahl(neigung)}")
    aus.append(f"daempfer streuung {zahl(streuung)} " + " ".join(zahl(x) for x in phi2))
    for n, (hz, frueh, spaet) in enumerate(tt, 1):
        aus.append(f"teilton {n} {zahl(hz)} {zahl(frueh)} {zahl(spaet)}")
    aus.append(f"kraft {zahl(rms(v, v + 0.1))} {zahl(rms(v, v + 1.0))} {zahl(rms(1.0, 3.0))}")
    band = im_band(kraft)

    def rms_band(von, bis):
        x = band[int(von * ps.FS):int(bis * ps.FS)]
        return 10 * np.log10(np.mean(x ** 2) + 1e-300)

    aus.append(f"kraftband {zahl(rms_band(v, v + 0.1))} {zahl(rms_band(v, v + 1.0))} {zahl(rms_band(1.0, 3.0))}")
    aus.append(f"hoerort {zahl(ps.momentary_max(p))} {zahl(ps.rms_db(p, 1, 2) - ps.rms_db(p, v, e))}")
    print(f"{name}: {S} Saite(n), Stegpunkt {stegtaste}, x_p {xp:.4f} m, F0 {saiten[0]['T'] * d * laengen[0] / (xp * (laengen[0] - xp)):.4f} N, "
          f"{len(tt)} Teiltoene, Grundton {tt[0][0]:.3f} Hz, Hoerort {ps.momentary_max(p):.2f} LUFS", flush=True)


def gegen_zeitschritt(namen):
    """Pruefung 7d-1: derselbe Fall im Zeitschritt wie der Kandidat S200 (`schleife`: Moden „genau fuer stueckweise lineare
    Kraft“ mit Durchgriff, Umlauf genau als FIR, die Quelle g genau) — nur eine Saite. Gibt je Fall die Abweichung der
    Teiltoene (Cent, Pegel frueh / spaet), der Lautheit am Hoerort und des Pegels der Stegkraft."""
    w = ps.frequenzen()
    for name, instrument, taste, stufe in [f for f in faelle() if f[0] in namen]:
        saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, neigung = eingaben(instrument, taste, stufe)
        if len(saiten) != 1:
            print(f"{name}: Chor - der Kandidat rechnet eine Saite, ausgelassen")
            continue
        koerper = KOERPER[instrument]
        kp = ps.lade_koerper(koerper)
        roh = json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8"))
        punkt = kp["tasten"].index(stegtaste)
        datei = len(roh["modes"])
        tan = ps.richtung_von(kp)
        phi = kp["steg"][:datei, punkt]
        f, eta = kp["f"][:datei], kp["eta"][:datei]
        e = np.stack([phi, phi * tan[:datei]], axis=1)
        nur = {"f": f, "eta": eta, "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
        Y, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt])
        _, phi2 = daempfer(roh["statistical"], punkt)
        wn = 2 * np.pi * DAEMPFER_MITTEN
        Y[:, 0, 0] += (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * DAEMPFER_ETA * wn[None, :] * w[:, None])) @ phi2
        f = np.concatenate([f, DAEMPFER_MITTEN])
        eta = np.concatenate([eta, np.full(len(DAEMPFER_MITTEN), DAEMPFER_ETA)])
        e = np.concatenate([e, np.stack([np.sqrt(phi2), 0 * phi2], axis=1)])
        # Bezug: geschlossen
        F = stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, neigung, Y, w)
        bezug = ps.zeit(F[:, 0:1])[:, 0]
        # die Quelle g wie in `stegkraft`, in die Zeit
        sa, L = saiten[0], laengen[0]
        T, m = sa["T"], sa["m"]
        g1 = quelle(sa, L, xp, d, breite, tempo, neigung, w)
        a = np.radians(winkel)
        g = ps.zeit(g1[:, None] * np.array([np.cos(a), np.sin(a)])[None, :])
        h, _ = kd.umlauf_fir(sa, L)
        z0 = float(np.sqrt(T * m))
        pol, B0, B1 = kd.moden_diskret(f, eta, e, "foh")
        M = np.einsum("n,na,nb->ab", B0.real, e, e)
        Fzeit = kd.schleife(np.ascontiguousarray(g), h, z0, pol, B0, B1, np.ascontiguousarray(e), M)[:, 0]
        _, Hz, _ = ps.koerper_kurven2(kp, punkt, w, tan)
        pb = ps.zeit(kd.spektrum_s0(bezug[:, None]) * Hz)
        pz = ps.zeit(kd.spektrum_s0(Fzeit[:, None]) * Hz)
        f0 = 440.0 * 2 ** ((taste - 69) / 12)
        tb, tz = kd.teiltoene(bezug, f0), kd.teiltoene(Fzeit, f0)
        cent = [1200 * np.log2(x[0] / y[0]) for x, y in zip(tz, tb)]
        dfr = [x[1] - y[1] for x, y in zip(tz, tb)]
        dsp = [x[2] - y[2] for x, y in zip(tz, tb) if y[2] > y[1] - 60]
        v = ps.VORLAUF

        def rms(x, von, bis):
            s = x[int(von * ps.FS):int(bis * ps.FS)]
            return 10 * np.log10(np.mean(s ** 2) + 1e-300)

        def baender(x, von, bis):
            s = x[int(von * ps.FS):int(bis * ps.FS)]
            S2 = np.abs(np.fft.rfft(s)) ** 2
            hz = np.fft.rfftfreq(len(s), 1 / ps.FS)
            return [10 * np.log10(S2[(hz >= lo) & (hz < hi)].sum() + 1e-300)
                    for lo, hi in ((0, 20), (20, 5000), (5000, 12000), (12000, 20000), (20000, 24001))]

        print(f"{name}: je Teilton n: Hz, Pegel frueh / spaet Bezug, Zeitschritt minus Bezug (dB)")
        for n, (x, y) in enumerate(zip(tz, tb), 1):
            print(f"    {n:2d} {y[0]:9.2f} Hz  {y[1]:7.1f} / {y[2]:7.1f}   {x[1] - y[1]:+6.2f} / {x[2] - y[2]:+6.2f}")
        bb, bz = baender(bezug, 1, 3), baender(Fzeit, 1, 3)
        print(f"{name}: 1-3 s je Band <20 / 20-5k / 5-12k / 12-20k / >20k Hz: Bezug " + " / ".join(f"{x:6.1f}" for x in bb) + "  Zeitschritt "
              + " / ".join(f"{x:6.1f}" for x in bz) + " dB")
        print(f"{name}: Lautheit am Hoerort {ps.momentary_max(pz) - ps.momentary_max(pb):+.3f} LU | Teiltoene 1-{len(cent)}: "
              f"groesster Betrag {np.abs(cent).max():.3f} Cent (Grundton {cent[0]:+.3f}), Pegel frueh {np.abs(dfr).max():.2f} dB, "
              f"spaet {np.abs(dsp).max() if dsp else float('nan'):.2f} dB | Pegel der Stegkraft 0-0,1 / 0-1 / 1-3 s "
              f"{rms(Fzeit, v, v + 0.1) - rms(bezug, v, v + 0.1):+.3f} / {rms(Fzeit, v, v + 1) - rms(bezug, v, v + 1):+.3f} / "
              f"{rms(Fzeit, 1, 3) - rms(bezug, 1, 3):+.3f} dB", flush=True)


def main():
    ziel = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "projects/apps/LumiViz/tests/unit/UnitTests/werte/zupf_pruefstand.txt"
    w = ps.frequenzen()
    aus = ["# Bezug des Kerns pluckedString, Rechenfassung 2 (34k-7d-1, S222) - nicht von Hand aendern",
           "# geschrieben von docs/modellierung/gezupfte-saite/proben/2026-11-26_S222_bezug_zupf.py (dort die Vorgaben und die Rechnung)",
           f"# Pruefstand: {ps.FS} Hz, {ps.DAUER} s, s0 {ps.S0} 1/s, Vorlauf {ps.VORLAUF} s; Daempfer-Mitten "
           + " ".join(f"{m:.0f}" for m in DAEMPFER_MITTEN) + f" Hz, eta {DAEMPFER_ETA!r}",
           "# fall <name> <instrument> taste <t> stufe <dB> stegpunkt <taste> saiten <n>",
           "# saite <i> laenge <m> T m EI etaB [etaF etaA | d rho] kraft <F0 N>",
           "# zupf ort <m> auslenkung <m> winkel <grad> breite <m> tempo <m/s> neigung <stufe dB>",
           "# daempfer streuung <phi_rms senkrecht> <phi^2 je Mitte, 7 Werte>",
           "# teilton <n> <Hz> <Pegel frueh dB> <Pegel spaet dB>   (Stegkraft senkrecht)",
           "# kraft <dB re 1 N: 0-0,1 s> <0-1 s> <1-3 s>   (ab dem Vorlauf)",
           "# kraftband <dieselben im Band 20 Hz - 20 kHz: FFT auf 2^18 Werten, ausserhalb null>",
           "# hoerort <Momentary-Maximum LUFS> <Pegel 1-2 s gegen 0-0,4 s dB>"]
    geladen = {}
    for name, instrument, taste, stufe in faelle():
        if instrument not in geladen:
            koerper = KOERPER[instrument]
            geladen[instrument] = (ps.lade_koerper(koerper),
                                   json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8")))
        rechne(name, instrument, taste, stufe, *geladen[instrument], w, aus)
    ziel.write_text("\n".join(aus) + "\n", encoding="utf-8", newline="\n")
    print(f"geschrieben: {ziel}")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "zeitschritt":
        gegen_zeitschritt(sys.argv[2:])
    else:
        main()

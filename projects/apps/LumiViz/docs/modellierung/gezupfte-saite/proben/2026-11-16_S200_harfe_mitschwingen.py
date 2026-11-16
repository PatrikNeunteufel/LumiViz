"""S200, 34k-6 — Harfe im Pruefstand: Mitschwingen je Note, die Regel der Auswahl gegen alle 46 Saiten (GZ3 a).

Aufruf (vom Projekt-Root): python -B <dieses Skript> [<Taste> ...]      (ohne Angabe: 36 43 48 55 60 67 72 79 84 96)

Alle 46 Saiten der Salvi "Orchestra" (Bell 1988 Tab. 2.1 / 2.2, wie im Pruefstand) haengen frei am Boden, jede an ihrer
Oese, zwei Polarisationen; die Pedale in der Mitte (jede Saite um einen Halbton verkuerzt: C-Dur), gespielt wird eine
weisse Taste (Zupf nach Chadefaux 2012 wie im Pruefstand). Die volle Matrix zwischen den 92 Freiheitsgraden passt nicht
in den Speicher (96001 Frequenzen x 92 x 92): sie wird je Frequenzblock gebaut und sofort geloest — fuer jede Auswahl
im selben Block.

Auswahlen (Saiten nach der Stufe gegen die gespielte, 7 Stufen = Oktave):
  keine        nur die gespielte Saite
  Oktaven      Stufe 0 mod 7
  O+Q          dazu Quinten und Quarten (Stufen 3, 4 mod 7 — die Regel der IC-Vorlage)
  O+Q nah      wie O+Q, hoechstens zwei Oktaven von der gespielten entfernt
  O+Q+T        dazu Terzen und Sexten (Stufen 2, 5 mod 7)
  alle         alle 46

Je Auswahl gegen "alle": Lautheit der Note (Momentary-Maximum) und Nachklang (Pegel 1-2 s gegen die ersten 0,4 s).
Gesetzte Grenzen (S193, ohne Quelle): Nachklang hoechstens 3 dB, Lautheit hoechstens 2 LU daneben.

Schreibt nichts.
"""
import importlib.util
import pathlib
import sys
import time

import numpy as np

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("pruefstand", HIER.parent / "2026-11-15_S199_pruefstand_zupf.py")
ps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ps)

BLOCK = 256
GRENZEN = []          # --grenzen <Hz> ...: O+Q mit gekuerzter Kopplung (siehe `rechne`)
EINWEG = []           # --einweg: O+Q mit Rueckwirkung nur der gespielten Saite (siehe `rechne`)


def saiten():
    """Die 46 Saiten aufsteigend: (Taste der leeren Saite, Saite, Laenge mit dem Pedal in der Mitte)."""
    aus = []
    for i, zeile in enumerate(ps.HARFE_MONO + ps.HARFE_METALL):
        if i < len(ps.HARFE_MONO):
            hz, d, L = zeile
            sa = ps.monofil(d, hz, L, "N" if i <= 9 else "D")
        else:
            hz, kern, gesamt, L = zeile
            m = 7850 * np.pi * (kern / 1000) ** 2 / 4 + 8900 * np.pi * ((gesamt / 1000) ** 2 - (kern / 1000) ** 2) / 4
            sa = {"T": 4 * L * L * hz * hz * m, "m": m, "EI": 200e9 * np.pi * (kern / 1000) ** 4 / 64, "etaF": 5e-5, "etaB": 2.0e-2,
                  "etaA": 1.1, "hz": hz}
        aus.append((int(round(69 + 12 * np.log2(hz / 440.0))), sa, L * 2 ** (-1 / 12)))
    return sorted(aus, key=lambda z: z[0])


def auswahlen(s, n):
    stufe = lambda j: (j - s) % 7
    return {
        "keine": [s],
        "Oktaven": [j for j in range(n) if stufe(j) == 0],
        "O+Q": [j for j in range(n) if stufe(j) in (0, 3, 4)],
        "O+Q nah": [j for j in range(n) if stufe(j) in (0, 3, 4) and abs(j - s) <= 14],
        "O+Q+T": [j for j in range(n) if stufe(j) in (0, 2, 3, 4, 5)],
        "alle": list(range(n)),
    }


def rechne(kp, tan, alle, s, w):
    """Der Ton der gespielten Saite `s` fuer jede Auswahl: {Name: (Ton, Zahl der Saiten)}."""
    S, n = len(alle), len(w)
    punkte = [kp["tasten"].index(t + 1) for t, _, _ in alle]
    z = np.empty((n, 2 * S), dtype=complex)
    E = np.empty((n, 2 * S), dtype=complex)
    g = np.zeros((n, 2 * S), dtype=complex)
    for i, (_, sa, L) in enumerate(alle):
        k, z0 = ps.saite_wellen(sa, w)
        z[:, 2 * i] = z[:, 2 * i + 1] = z0
        E[:, 2 * i] = E[:, 2 * i + 1] = np.exp(-2j * k * L)
        if i == s:
            T, m = sa["T"], sa["m"]
            xp, d, tempo, breite = 0.36 * L, 0.004, 2.0, 0.0075
            f = -(T * d * L / (xp * (L - xp))) / (1j * w) * np.exp(-(w * breite / np.sqrt(T / m)) ** 2) * (1 - (tempo / d) / (1j * w))
            gi = f / (2 * z0) * (np.exp(-1j * k * xp) - np.exp(-1j * k * (2 * L - xp)))
            a = np.radians(25.0)
            g[:, 2 * i], g[:, 2 * i + 1] = gi * np.cos(a), gi * np.sin(a)
    wahl = auswahlen(s, S)
    if GRENZEN:
        # nur "alle" (Bezug) und O+Q; dazu O+Q mit gekuerzter Kopplung: die Moden ueber der Grenze spuert nur die
        # gespielte Saite, die uebrigen Stegpunkte kennen sie nicht (Y bleibt positiv semidefinit)
        wahl = {"alle": wahl["alle"], "O+Q": wahl["O+Q"], **{f"O+Q bis {gr:g} Hz": wahl["O+Q"] for gr in GRENZEN}}
    if EINWEG:
        wahl = {"alle": wahl["alle"], "O+Q": wahl["O+Q"], "O+Q einweg": wahl["O+Q"], "O+Q einweg + eigener Steg": wahl["O+Q"]}
    frei = {name: np.array([q for j in idx for q in (2 * j, 2 * j + 1)]) for name, idx in wahl.items()}
    p = {name: np.empty((n, kp["G"].shape[1]), dtype=complex) for name in wahl}
    # Y = Phi^T diag(c) Phi je Frequenz als EIN Matrixprodukt: c (Block x Moden) mal den Produkten phi_a phi_b (Moden x D D)
    steg = kp["steg"][:, punkte]
    phi = np.empty((len(kp["f"]), 2 * S))
    phi[:, 0::2] = steg
    phi[:, 1::2] = steg * tan[:, None]
    D, O = 2 * S, kp["G"].shape[1]
    PP = (phi[:, :, None] * phi[:, None, :]).reshape(len(phi), D * D).astype(complex)
    PG = (phi[:, :, None] * kp["G"][:, None, :]).reshape(len(phi), D * O)
    wn = 2 * np.pi * kp["f"]
    for a in range(0, n, BLOCK):
        b = slice(a, a + BLOCK)
        ws = w[b][:, None]
        nenner = 1.0 / (wn[None, :] ** 2 - ws ** 2 + 1j * kp["eta"][None, :] * wn[None, :] * ws)
        Y = ((1j * ws * nenner) @ PP).reshape(-1, D, D)
        H = ((-ws ** 2 * nenner) @ PG).reshape(-1, D, O)
        tief = {}
        for gr in GRENZEN:
            Yt = ((1j * ws * nenner[:, kp["f"] <= gr]) @ PP[kp["f"] <= gr]).reshape(-1, D, D)
            Yt[:, 2 * s:2 * s + 2, 2 * s:2 * s + 2] = Y[:, 2 * s:2 * s + 2, 2 * s:2 * s + 2]
            tief[f"O+Q bis {gr:g} Hz"] = Yt
        if EINWEG:
            # Rueckwirkung nur der gespielten Saite: die uebrigen Stegpunkte folgen ihrer Kraft, ihre eigenen Saiten
            # wirken nicht auf den Koerper zurueck (Spalte der gespielten Saite); zum Vergleich dazu mit dem eigenen
            # Stegpunkt je Saite (Diagonale), aber ohne Kopplung der uebrigen untereinander
            Ye = np.zeros_like(Y)
            Ye[:, :, 2 * s:2 * s + 2] = Y[:, :, 2 * s:2 * s + 2]
            tief["O+Q einweg"] = Ye
            Yd = Ye.copy()
            for j in range(S):
                Yd[:, 2 * j:2 * j + 2, 2 * j:2 * j + 2] = Y[:, 2 * j:2 * j + 2, 2 * j:2 * j + 2]
            tief["O+Q einweg + eigener Steg"] = Yd
        for name, q in frei.items():
            eins = np.eye(len(q))
            Yn = tief.get(name, Y)
            YZ = Yn[:, q][:, :, q] * z[b][:, None, q]
            M = np.linalg.solve(eins[None] + YZ, 2 * YZ)
            vplus = np.linalg.solve(eins[None] + E[b][:, q, None] * (M - eins[None]), g[b][:, q, None])[:, :, 0]
            F = z[b][:, q] * (2 * vplus - (M @ vplus[:, :, None])[:, :, 0])
            p[name][b] = np.einsum("fa,fao->fo", F, H[:, q])
    return {name: (ps.zeit(p[name]), len(wahl[name])) for name in wahl}


if __name__ == "__main__":
    kp = ps.lade_koerper("concert_harp")
    tan = ps.richtung_tan(kp["f"])
    w = ps.frequenzen()
    alle = saiten()
    if sys.argv[1:] == ["pruefen"]:
        # der Block gegen `koerper_matrix` und die Auswahl "keine" gegen `ton_mit` mit einer Saite
        s = [t for t, _, _ in alle].index(59)
        w = w[20000:20512]
        punkt = [kp["tasten"].index(60)]
        Y, H = ps.koerper_matrix(kp, w, tan, punkt)
        sa, L = alle[s][1], alle[s][2]
        alt = ps.ton_mit([sa], [L], 0, 0.36 * L, 0.004, 25.0, Y, H, w, 0.0075, 2.0)[0]
        n = len(w)
        ps.zeit = lambda x: x          # nur das Spektrum vergleichen
        neu = rechne(kp, tan, alle, s, w)
        print(f"Auswahl 'keine' gegen ton_mit: {np.abs(neu['keine'][0] - alt).max() / np.abs(alt).max():.2e}")
        print(f"alle gegen keine in diesem Band (darf abweichen): {np.abs(neu['alle'][0] - alt).max() / np.abs(alt).max():.2e}")
        sys.exit(0)
    if "--einweg" in sys.argv:
        sys.argv.remove("--einweg")
        EINWEG.append(True)
    if "--grenzen" in sys.argv:
        i = sys.argv.index("--grenzen")
        GRENZEN[:] = [float(a) for a in sys.argv[i + 1:]]
        del sys.argv[i:]
    tasten = [int(a) for a in sys.argv[1:]] or [36, 43, 48, 55, 60, 67, 72, 79, 84, 96]
    print(f"Harfe: {len(alle)} Saiten, {len(kp['f'])} Moden; leere Saiten {alle[0][0]} bis {alle[-1][0]}", flush=True)
    print("Taste | je Auswahl: Zahl der Saiten, Lautheit LUFS (gegen alle LU), Nachklang dB (gegen alle dB)")
    summe = {}
    for taste in tasten:
        offen = [t for t, _, _ in alle]
        if taste - 1 not in offen:
            print(f"{taste:3d} | keine weisse Taste einer Saite")
            continue
        s = offen.index(taste - 1)
        t0 = time.time()
        erg = rechne(kp, tan, alle, s, w)
        v, e = ps.VORLAUF, ps.VORLAUF + 0.4
        mass = {name: (ps.momentary_max(x), ps.rms_db(x, 1, 2) - ps.rms_db(x, v, e), anz) for name, (x, anz) in erg.items()}
        zeile = f"{taste:3d} | "
        for name, (l, nk, anz) in mass.items():
            dl, dn = l - mass["alle"][0], nk - mass["alle"][1]
            summe.setdefault(name, []).append((dl, dn, anz))
            zeile += f"{name} {anz:2d}: {l:6.1f} ({dl:+5.2f}) {nk:6.1f} ({dn:+5.1f}) | "
        print(zeile + f"{time.time() - t0:.0f} s", flush=True)
    print("Groesster Betrag gegen alle 46: Lautheit LU / Nachklang dB (Zahl der Saiten von - bis)")
    for name, werte in summe.items():
        a = np.array(werte)
        print(f"  {name:8s}: {np.abs(a[:, 0]).max():5.2f} LU / {np.abs(a[:, 1]).max():5.1f} dB   ({int(a[:, 2].min())}-{int(a[:, 2].max())} Saiten)")

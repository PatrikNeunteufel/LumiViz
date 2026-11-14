"""Eingangsadmittanz der Harfe laengs der Deckleiste: gerechnet (Formen aus `InstrumentBodyTool --shapes`) gegen
Bell 1988 Abb. 7.8–7.16 (Kasten G, in der Harfe ohne Saiten und besaitet) — Lage, Hoehe, Ort und Guete je Mode (S198).

    python …_harfe_admittanz.py <formen.txt> <saitenorte.json> [--eta <wert>] [--last <faktor>] [--bild <datei.png>]

Punktadmittanz senkrecht zum Boden an den 40 Messorten der Mittellinie (Ort 40 am Bass, Ort 1 am Diskant, gleichmaessig
ueber die 131 cm verteilt — Bell S. 127 nennt 3,5 cm Abstand, die genaue Lage steht nicht im gelesenen Text; ±1 Ort):

    (ω_n² (1 + i η) − ω²) q_n + i ω Σ_m C_nm q_m = φ_n(p) F,     Y(p) = i ω Σ_n φ_n(p) q_n / F
    C_nm = Σ_Saiten R_i φ_n(p_i) φ_m(p_i),     R_i = last · Z_i,  Z_i = μ_i · 2 L_i f_i  (Wellenwiderstand quer)

φ massennormiert (wie das Werkzeug sie schreibt). Die Saiten sind bei Bell mit Band stark gedaempft: je Saite ein
Widerstand an ihrer Oese. `last` fasst den Winkel der Saite zum Boden zusammen (cos² des Winkels zwischen Saite und
Bodenebene) — Annahme; 0 = ohne Saiten. Saiten: Bell Tab. 2.1 / 2.2 (Frequenz, Durchmesser, Laenge; am Bild gelesen);
Dichten angenommen: Nylon 1140, Darm 1300, Stahl 7850, Kupfer 8900 kg/m³ (Umwicklung als voller Ring gerechnet).

Pegel wie bei Bell: 116 dB = 1 s/kg, hier in dB re 1 m/s/N ausgegeben (Bell-Wert − 116).
"""
import importlib.util
import json
import os
import sys

import numpy as np

HIER = os.path.dirname(os.path.abspath(__file__))
LAENGE = 1.31
BEZUG = 116.0

# Bell Tab. 2.1 (Nylon 00–8, Darm 9–33): Nr., Hz, Durchmesser mm, Laenge m
NYLON_DARM = [("00", 2960, 0.52, 0.071), ("0", 2637, 0.52, 0.082), ("1", 2489, 0.56, 0.088), ("2", 2218, 0.56, 0.095),
              ("3", 1976, 0.61, 0.105), ("4", 1864, 0.62, 0.113), ("5", 1661, 0.65, 0.123), ("6", 1480, 0.69, 0.132),
              ("7", 1318, 0.70, 0.143), ("8", 1244, 0.69, 0.154), ("9", 1109, 0.70, 0.164), ("10", 988, 0.73, 0.177),
              ("11", 932, 0.76, 0.192), ("12", 830, 0.80, 0.208), ("13", 740, 0.85, 0.225), ("14", 659, 0.90, 0.242),
              ("15", 622, 0.95, 0.265), ("16", 554, 1.00, 0.285), ("17", 494, 1.06, 0.327), ("18", 466, 1.12, 0.354),
              ("19", 415, 1.18, 0.385), ("20", 370, 1.24, 0.421), ("21", 330, 1.30, 0.455), ("22", 311, 1.36, 0.498),
              ("23", 277, 1.44, 0.542), ("24", 247, 1.52, 0.591), ("25", 233, 1.60, 0.644), ("26", 207, 1.68, 0.699),
              ("27", 185, 1.76, 0.753), ("28", 165, 1.84, 0.813), ("29", 155, 1.92, 0.880), ("30", 139, 2.00, 0.945),
              ("31", 123, 2.10, 0.999), ("32", 116, 2.20, 1.053), ("33", 104, 2.35, 1.103)]
# Bell Tab. 2.2 (Metall 34–44): Nr., Hz, Kern mm, Umwicklung mm, gesamt mm, Laenge m
METALL = [("34", 92, 0.80, 0.24, 1.45, 1.150), ("35", 82, 0.80, 0.32, 1.55, 1.180), ("36", 77, 0.80, 0.35, 1.65, 1.222),
          ("37", 69, 0.85, 0.40, 1.75, 1.257), ("38", 61, 0.90, 0.43, 1.92, 1.299), ("39", 58, 0.95, 0.45, 2.05, 1.330),
          ("40", 52, 1.00, 0.50, 2.15, 1.359), ("41", 46, 1.10, 0.55, 2.30, 1.397), ("42", 41, 1.10, 0.60, 2.45, 1.421),
          ("43", 39, 1.25, 0.65, 2.60, 1.445), ("44", 35, 1.30, 0.68, 2.75, 1.475)]

# Bell Abb. 7.8–7.16, am Bild gelesen (±1–2 dB, ±1 Ort): Mode, Hz, Guete, groesster Pegel dB, Ort — je Zustand
MESSUNG = {
    "in der Harfe, ohne Saiten": [("S(1,2)", 249, 39, 96.0, 12), ("S(1,3)", 316, 45, 104.0, 8), ("S(1,4)", 432, 43, 105.5, 5),
                                  ("S(1,5)", 545, 31, 101.5, 3), ("S(1,6)", 709, 10, 100.5, 3), ("S(1,7)", 757, 33, 99.5, 3),
                                  ("S(1,8)", 905, 18, 102.0, 4)],
    "in der Harfe, besaitet": [("S(1,2)", 257, 32, 92.5, 12), ("S(1,3)", 334, 27, 101.0, 8), ("S(1,4)", 430, 28, 99.5, 4),
                               ("S(1,5)", 555, 19, 103.0, 3), ("S(1,6)", 697, 12, 103.0, 2), ("S(1,7)", 753, 17, 101.0, 2),
                               ("S(1,8)", 895, 18, 103.5, 3)],
}


def lade(name, datei):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HIER, datei))
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)
    return modul


def wellenwiderstaende():
    """Nr. -> Z in N s/m (quer), aus Bell Tab. 2.1 / 2.2."""
    z = {}
    for k, (nr, hz, d, laenge) in enumerate(NYLON_DARM):
        dichte = 1140.0 if k <= 9 else 1300.0
        mu = dichte * np.pi * (d * 1e-3) ** 2 / 4.0
        z[nr] = mu * 2.0 * laenge * hz
    for nr, hz, kern, _, gesamt, laenge in METALL:
        mu = 7850.0 * np.pi * (kern * 1e-3) ** 2 / 4.0 + 8900.0 * np.pi * ((gesamt * 1e-3) ** 2 - (kern * 1e-3) ** 2) / 4.0
        z[nr] = mu * 2.0 * laenge * hz
    return z


def ort_y(ort):
    """Messort 1 (Diskant) … 40 (Bass) -> y vom Bassende in m."""
    return LAENGE * (40.0 - ort + 0.5) / 40.0


def knoten_bei(knoten, y):
    return int(np.argmin(knoten[:, 0] ** 2 + (knoten[:, 1] - y) ** 2))


def admittanz(hz, phi_orte, phi_saiten, r, f, eta):
    """|Y| in dB re 1 m/s/N: Feld [Frequenz, Messort]."""
    c = (phi_saiten * r[None, :]) @ phi_saiten.T if len(r) else np.zeros((len(hz), len(hz)))
    omn2 = (2.0 * np.pi * hz) ** 2
    aus = np.empty((len(f), phi_orte.shape[1]))
    for k, fk in enumerate(f):
        om = 2.0 * np.pi * fk
        a = np.diag(omn2 * (1.0 + 1j * eta) - om * om) + 1j * om * c
        q = np.linalg.solve(a, phi_orte)
        aus[k] = 20.0 * np.log10(np.abs(1j * om * np.einsum("np,np->p", phi_orte, q)))
    return aus, c


def main():
    args = sys.argv[1:]

    def wert(name, vorgabe, typ=float):
        return typ(args[args.index(name) + 1]) if name in args else vorgabe

    eta, last, bild = wert("--eta", 0.027), wert("--last", 0.0), wert("--bild", "", str)
    hm = lade("harfe_moden", "2026-11-13_S197_harfe_moden.py")
    knoten, hz, w = hm.formen(args[0])
    saiten = json.load(open(args[1], encoding="utf-8"))["saiten"]
    z = wellenwiderstaende()
    r = np.array([last * z[s["name"]] for s in saiten]) if last > 0 else np.array([])
    phi_saiten = np.array([w[:, knoten_bei(knoten, s["ort"])] for s in saiten]).T if last > 0 else np.zeros((len(hz), 0))
    orte = np.arange(1, 41)
    phi_orte = np.array([w[:, knoten_bei(knoten, ort_y(o))] for o in orte]).T
    f = np.arange(150.0, 1000.0, 1.0)
    d, c = admittanz(hz, phi_orte, phi_saiten, r, f, eta)
    print(f"{len(hz)} Moden bis {hz[-1]:.0f} Hz, eta {eta}, Saitenlast {last} x Z (Summe Z {sum(z.values()):.1f} N s/m, "
          f"{min(z.values()):.2f} … {max(z.values()):.2f})")
    guete = hz * 2.0 * np.pi / (eta * 2.0 * np.pi * hz + np.diag(c) / 1.0) if last > 0 else np.full(len(hz), 1.0 / eta)
    for titel, ziele in MESSUNG.items():
        print(titel)
        for name, mhz, mq, mdb, mort in ziele:
            fenster = (f > mhz * 0.90) & (f < mhz * 1.10)
            teil = d[fenster]
            i, j = np.unravel_index(np.argmax(teil), teil.shape)
            fhz = f[fenster][i]
            n = int(np.argmin(np.abs(hz - fhz)))
            # Hoehe am gemessenen Ort (±1 Ort), groesster Wert im Fenster
            jm = [k for k, o in enumerate(orte) if abs(o - mort) <= 1]
            am_ort = teil[:, jm].max()
            print(f"   {name}: gemessen {mhz} Hz, Q {mq}, {mdb - BEZUG:+.1f} dB am Ort {mort} | gerechnet {fhz:.0f} Hz "
                  f"({100 * (fhz / mhz - 1):+.0f} %), Q {guete[n]:.0f}, {teil[i, j]:+.1f} dB am Ort {orte[j]} "
                  f"({teil[i, j] - (mdb - BEZUG):+.1f} dB), am gemessenen Ort {am_ort:+.1f} dB ({am_ort - (mdb - BEZUG):+.1f} dB)")
    if bild:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(figsize=(7, 9))
        bildfeld = ax.pcolormesh(f, orte, d.T + BEZUG, shading="nearest", cmap="viridis", vmin=64, vmax=108)
        ax.set_ylim(40, 1)
        ax.set_xlabel("Hz")
        ax.set_ylabel("Messort (1 Diskant, 40 Bass)")
        fig.colorbar(bildfeld, label="dB (116 dB = 1 s/kg), wie Bell Abb. 7.2–7.4")
        fig.savefig(bild, dpi=110)
        print(bild)


if __name__ == "__main__":
    main()

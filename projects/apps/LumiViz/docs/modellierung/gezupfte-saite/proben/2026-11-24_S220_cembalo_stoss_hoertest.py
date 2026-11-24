"""S220, Herleitung §12 — der Tastenstoss des Cembalos (GZ10 a, KQ30 a) gegen Penttinen 2006 Abb. 6 am Seitenbild.

Aufruf (vom Projekt-Root): python -B <dieses Skript>

Penttinen 2006 (DAFx-06) S. 4, Abb. 6: Hoertest mit sechs Hoerern, Paare pp -> ff (a), pp -> mf (b), mf -> ff (c) an
C2 ... C6, Schieber in 0,5-dB-Schritten. Am Seitenbild (400 dpi, vergroessert) abgelesen sind die MEDIANE der Kaesten
(die Quelle nennt Mittelwerte nur fuer C3 pp -> ff 2,41 und C6 pp -> mf 0,42, sonst Tab. 1 ueber alle Toene:
pp -> ff 1,68, pp -> mf 1,08, mf -> ff 0,98). GZ10 a hatte (a) grob mit 1,3 / 2,4 / 2,2 / 1,4 / 1,3 gelesen.

Modell wie im Pruefstand S200 (`2026-11-16_S200_cembalo_register.py`, Saite und Stoss am Stegpunkt der Taste), dazu:
- Abloesekraft nach GZ13 a mit dem Massstab aus KQ30 a: 1,07 N an C4 bei ff, -2,8 dB je Oktave (der Pruefstand rechnet
  mit `CEMBALO_KRAFT` und ist in der Kraft linear: die Saite wird mit F / CEMBALO_KRAFT skaliert);
- Stufen nach GZ10 a: Saite ff / mf / pp 0 / -0,5 / -1 dB (mf: Mitte, ANNAHME), Stoss 0 / -9 / -18 dB;
- Lautheit wie bei GZ10 a: Momentary-Maximum. Die Hoerer achteten nach Penttinen vor allem auf den ausklingenden Teil;
  zum Vergleich daher auch die Lautheit ab 0,2 s nach dem Anschlag (Momentary-Maximum des Rests).
- Raster: Saite je Stufe 0,5 / 0,75 / 1,0 dB gegen Stoss 0 ... 5 mN s, gegen die Mediane und gegen Tab. 1.

Ausgabe von S220: `2026-11-24_S220_cembalo_stoss_hoertest.txt` (rund 1 min). Schreibt nichts.
"""
import importlib.util
import pathlib

import numpy as np

HIER = pathlib.Path(__file__).resolve()
spec = importlib.util.spec_from_file_location("register", HIER.parent / "2026-11-16_S200_cembalo_register.py")
rg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rg)
ps = rg.ps

TASTEN = (36, 48, 60, 72, 84)
# Mediane aus Abb. 6, am Seitenbild abgelesen (Raster 0,25 dB)
ZIEL = {
    "pp-ff": (1.25, 2.50, 2.00, 1.25, 1.25),
    "pp-mf": (1.00, 1.25, 2.00, 1.00, 0.50),
    "mf-ff": (0.50, 1.50, 1.50, 1.00, 0.25),
}
SAITE_DB = {"ff": 0.0, "mf": -0.5, "pp": -1.0}
STOSS_DB = {"ff": 0.0, "mf": -9.0, "pp": -18.0}
JS = (0.0, 0.002, 0.003, 0.0038, 0.005, 0.007, 0.010)


def kraft(taste):
    return 1.07 * 10 ** (-2.8 * (taste - 60) / 12 / 20)


def main():
    kp = ps.lade_koerper("harpsichord")
    w = ps.frequenzen()
    tan = ps.richtung_tan(kp["f"])
    gauss = np.exp(-0.5 * (w * rg.STOSS_SIGMA) ** 2)
    daten = {}
    for taste in TASTEN:
        punkt = kp["tasten"].index(taste)
        Y, H = ps.koerper_matrix(kp, w, tan, [punkt])
        x = ps.zeit(rg.ton_viele([rg.acht(taste)], Y, H, w)) * (kraft(taste) / ps.CEMBALO_KRAFT)
        xs = ps.zeit(gauss[:, None] * H[:, 0, :])
        daten[taste] = (x, xs)
        print(f"Taste {taste}: Kraft {kraft(taste):.2f} N, gerechnet", flush=True)
    rest = int((ps.VORLAUF + 0.2) * ps.FS)

    def laut(taste, stufe, J, spaet):
        x, xs = daten[taste]
        y = x * 10 ** (SAITE_DB[stufe] / 20) + J * 10 ** (STOSS_DB[stufe] / 20) * xs
        if spaet:
            y = y[rest:]
        return ps.momentary_max(y)

    for spaet in (False, True):
        print()
        print("Lautheit " + ("ab 0,2 s nach dem Anschlag" if spaet else "ganzer Ton") + " (Momentary-Maximum), Unterschied LU je Paar")
        print("J ff (mN s) | " + " | ".join(f"{k}: C2 C3 C4 C5 C6" for k in ZIEL) + " | rms gegen die Mediane: je Paar / alle")
        for J in JS:
            zeile = f"{1000 * J:5.1f}       | "
            fehl_alle = []
            rms = []
            for paar, ziel in ZIEL.items():
                a, b = paar.split("-")
                d = [laut(t, b, J, spaet) - laut(t, a, J, spaet) for t in TASTEN]
                f = [di - zi for di, zi in zip(d, ziel)]
                fehl_alle += f
                rms.append(float(np.sqrt(np.mean(np.square(f)))))
                zeile += " ".join(f"{v:4.2f}" for v in d) + " | "
            zeile += " / ".join(f"{r:.2f}" for r in rms) + f" / {np.sqrt(np.mean(np.square(fehl_alle))):.2f}"
            print(zeile, flush=True)
    print()
    print("Ziel (Mediane) | " + " | ".join(f"{k}: " + " ".join(f"{v:4.2f}" for v in z) for k, z in ZIEL.items()))

    # Raster: die Saite je Stufe (pp -> ff = zwei Stufen) und der Stoss; gegen die Mediane und gegen Tab. 1
    # (Mittel ueber alle Toene: pp -> ff 1,68, pp -> mf 1,08, mf -> ff 0,98)
    tab1 = {"pp-ff": 1.68, "pp-mf": 1.08, "mf-ff": 0.98}
    print()
    print("Raster, ganzer Ton: Saite je Stufe (dB) | J ff (mN s) | Mittel ueber C2-C6 je Paar pp-ff / pp-mf / mf-ff | "
          "rms gegen Tab. 1 | rms gegen die Mediane (alle 15)")
    for stufe_db in (0.5, 0.75, 1.0):
        SAITE_DB.update({"ff": 0.0, "mf": -stufe_db, "pp": -2 * stufe_db})
        for J in (0.0, 0.001, 0.002, 0.003, 0.0038, 0.005):
            mittel, f15, f3 = [], [], []
            for paar, ziel in ZIEL.items():
                a, b = paar.split("-")
                d = [laut(t, b, J, False) - laut(t, a, J, False) for t in TASTEN]
                mittel.append(float(np.mean(d)))
                f15 += [di - zi for di, zi in zip(d, ziel)]
                f3.append(mittel[-1] - tab1[paar])
            print(f"{stufe_db:4.2f} | {1000 * J:4.1f} | " + " / ".join(f"{m:4.2f}" for m in mittel) +
                  f" | {np.sqrt(np.mean(np.square(f3))):.2f} | {np.sqrt(np.mean(np.square(f15))):.2f}", flush=True)


if __name__ == "__main__":
    main()

"""S200, 34k-6 — der Gang der Lautheit ueber den Umfang: Modell gegen Aufnahmen, Cembalo und Harfe.

Aufruf (vom Projekt-Root): python -B <dieses Skript> cembalo | harfe

Liest die Rohausgaben: Modell je Taste (`2026-11-15_S199_pruefstand_<instrument>_je_taste.txt`, Lautheit mit allen
Moden) und Aufnahmen (`2026-11-12_S195_ziele_aufnahmen.txt`, Momentary-Maximum je Ton; die Namen der Cembali und der
Volksharfe eine Oktave zu tief, die Konzertharfe richtig — S195). Die Pegel der Sammlungen sind untereinander und gegen
das Modell nicht vergleichbar: verglichen wird die FORM — je Instrument die Lautheit minus ihr Median, dann je Taste der
Median ueber die Instrumente, geglaettet (gleitender Median ueber 7 Tasten).

Das Modell ist linear: eine andere Zupfstaerke je Taste verschiebt die Lautheit um 20 lg des Verhaeltnisses. Ausgegeben
wird, welche Kurve der Zupfstaerke (Gerade bzw. Parabel in der Taste, kleinste Quadrate) das Modell auf die Form der
Aufnahmen bringt, und was danach an Spanne und Spruengen bleibt.
"""
import pathlib
import re
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent

GRUPPEN = {
    "cembalo": [("Harpsichord, French/Sustains", 12), ("Harpsichord, Unk/Sustains", 12), ("Harpsichord, English/Sustains/Normal", 12),
                ("Harpsichord, Flemish/Sustains/Low", 12), ("Harpsichord, Italian/Sustains/stop1", 12)],
    "harfe": [("Concert Harp", 0)],
}


def aufnahmen(name):
    je = {}
    for z in (HIER / "2026-11-12_S195_ziele_aufnahmen.txt").read_text(encoding="utf-8").splitlines():
        t = z.split("\t")
        if len(t) < 12 or not t[-1].endswith(".wav"):
            continue
        for gruppe, okt in GRUPPEN[name]:
            if t[0] == gruppe and (name != "harfe" or t[1].endswith("_f")):
                je.setdefault(gruppe, {}).setdefault(int(t[-12]) + okt, []).append(float(t[-11]))
    return {g: {k: float(np.mean(v)) for k, v in d.items()} for g, d in je.items()}


def modell(name):
    aus = {}
    for z in (HIER / f"2026-11-15_S199_pruefstand_{name}_je_taste.txt").read_text(encoding="utf-8").splitlines():
        m = re.match(r"\s*(\d+) \|[^|]*\|\s*(-?\d+\.\d+) /", z)
        if m:
            aus[int(m.group(1))] = float(m.group(2))
    return aus


def glatt(tasten, werte, halb=3):
    return np.array([np.median([w for t, w in zip(tasten, werte) if abs(t - t0) <= halb]) for t0 in tasten])


def kennzahlen(tasten, l):
    l = np.asarray(l)
    d3 = np.array([abs(l[i] - l[j]) for i in range(len(l)) for j in range(i + 1, len(l)) if 0 < tasten[j] - tasten[i] <= 4])
    return f"Spanne {l.max() - l.min():5.1f} LU, 10.-90. Perzentil {np.percentile(l, 90) - np.percentile(l, 10):5.1f} LU, groesster Sprung (<= 4 Halbtoene) {d3.max():4.1f} LU, mittlerer {np.median(d3):.1f} LU"


if __name__ == "__main__":
    name = sys.argv[1]
    auf, mod = aufnahmen(name), modell(name)
    tm = np.array(sorted(mod))
    lm = np.array([mod[t] for t in tm])
    print(f"{name}: Modell {len(tm)} Tasten {tm[0]}-{tm[-1]}: {kennzahlen(tm, lm)}")
    form = {}
    for g, d in auf.items():
        t = np.array(sorted(d))
        l = np.array([d[k] for k in t])
        print(f"  Aufnahme {g:40s} {len(t)} Toene {t[0]}-{t[-1]}: {kennzahlen(t, l)}")
        for k, v in zip(t, l - np.median(l)):
            form.setdefault(int(k), []).append(v)
    ta = np.array(sorted(form))
    fa = glatt(ta, [np.median(form[k]) for k in ta])
    fm = glatt(tm, lm - np.median(lm))
    # auf den Tasten des Modells: Form der Aufnahmen (interpoliert) minus Form des Modells = noetige Aenderung in dB
    noetig = np.interp(tm, ta, fa) - fm
    noetig -= np.median(noetig)
    print("Form (Lautheit minus Median, geglaettet) je Oktave: Aufnahmen / Modell / noetige Aenderung der Zupfstaerke dB")
    for von in range(24, 108, 12):
        m = (tm >= von) & (tm < von + 12)
        if m.any():
            print(f"  C{von // 12 - 1}: {np.median(np.interp(tm[m], ta, fa)):+5.1f} / {np.median(fm[m]):+5.1f} / {np.median(noetig[m]):+5.1f}")
    x = (tm - 60) / 12.0
    for grad, titel in ((0, "feste Zupfstaerke"), (1, "Gerade in der Taste"), (2, "Parabel in der Taste")):
        k = np.polyfit(x, noetig, grad)
        neu = lm + np.polyval(k, x)
        rest = (neu - np.median(neu)) - np.interp(tm, ta, fa)
        print(f"{titel:22s}: dB je Oktave um C4 {' '.join(f'{c:+.2f}' for c in k[::-1])}  -> Modell {kennzahlen(tm, neu)}; "
              f"Abstand zur Form der Aufnahmen rms {np.sqrt(np.mean(rest ** 2)):.1f} LU; Faktor am tiefsten / hoechsten Ton "
              f"{10 ** (np.polyval(k, x[0]) / 20):.2f} / {10 ** (np.polyval(k, x[-1]) / 20):.2f} (gegen C4 {10 ** (np.polyval(k, 0) / 20):.2f})")

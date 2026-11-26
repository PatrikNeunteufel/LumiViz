"""S222, 34k-7d-1 — das Ablösetempo der Harfe: Rampe im Pruefstand gegen Anfangsgeschwindigkeit ohne statischen Anteil.

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <taste> [<taste> ...]

Der Pruefstand (`ton_mit`, Harfe mit `tempo` 2 m/s, Chadefaux u. a. 2012) rechnet die Geschwindigkeit beim Abloesen als
f (1 - (v/d) / (i w)) — die Kraftstufe mal eine Rampe. Herleitung der richtigen Form: ist y_d die Antwort auf die
Auslenkung (aus der Ruhe losgelassen) und y_v die auf die Anfangsgeschwindigkeit (v/d) * y0 mit derselben Form, dann gilt
y_v(t) = (v/d) * Integral_0^t y(tau) d tau mit der WAHREN Bewegung y = r_d + y0 (r_d die Antwort relativ zur Lage vor dem
Loslassen, wie sie der Pruefstand rechnet; y0 die statische Auslenkung). Am Steg: F_v = (v/d) * Integral (F_d + F_stat),
F_stat die statische Last vor dem Loslassen. Der Pruefstand integriert F_d allein — der Gleichanteil -F_stat wird eine
Rampe, die nie endet (nach 3 s an der Harfe G6 rund 10^4 N).

Mit Daempfung gilt die Identitaet nicht ganz (der Daempfungsterm laesst einen statischen Rest). Genau ist die
Anfangsgeschwindigkeit als verteilter Kraftstoss mu v0(x) delta(t) ueber das Dreieck (`stegkraft(..., "impuls")` in der
Bezugsdatei).

Rechnet je Taste die Stegkraft senkrecht (wie `2026-11-26_S222_bezug_zupf.py`) in vier Fassungen:
  ohne    nur die Auslenkung (tempo 0)
  rampe   wie der Pruefstand
  richtig F_d + (v/d) / (i w) * (F_d + F_stat / (i w)), F_stat aus dem Mittel von F_d spaet (geprueft gegen T d / x_p cos a)
  impuls  der verteilte Kraftstoss
und vergleicht: Gleichanteil spaet, Lautheit am Hoerort, Teiltoene, Pegel 1-2 s.
"""
import importlib.util
import pathlib
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
P = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben"
spec = importlib.util.spec_from_file_location("bezug", P / "2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
ps, kd = bz.ps, bz.kd


def rechne(taste):
    import json
    w = ps.frequenzen()
    kp = ps.lade_koerper("concert_harp")
    roh = json.loads((ps.KOERPER / "concert_harp.lvinstbody").read_text(encoding="utf-8"))
    saiten, laengen, stegtaste, xp, d, winkel, breite, tempo, _ = bz.eingaben("harfe", taste, 0.0)
    punkt = kp["tasten"].index(stegtaste)
    datei = len(roh["modes"])
    tan = ps.richtung_von(kp)
    nur = {"f": kp["f"][:datei], "eta": kp["eta"][:datei], "steg": kp["steg"][:datei], "G": kp["G"][:datei]}
    Y, _ = ps.koerper_matrix(nur, w, tan[:datei], [punkt])
    _, phi2 = bz.daempfer(roh["statistical"], punkt)
    wn = 2 * np.pi * bz.DAEMPFER_MITTEN
    Y[:, 0, 0] += (1j * w[:, None] / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * bz.DAEMPFER_ETA * wn[None, :] * w[:, None])) @ phi2
    _, Hz, _ = ps.koerper_kurven2(kp, punkt, w, tan)
    Fd = bz.stegkraft(saiten, laengen, xp, d, winkel, breite, 0.0, 0.0, Y, w)[:, 0]
    Fr = bz.stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, 0.0, Y, w, "rampe")[:, 0]
    Fi = bz.stegkraft(saiten, laengen, xp, d, winkel, breite, tempo, 0.0, Y, w, "impuls")[:, 0]
    fd = ps.zeit(Fd[:, None])[:, 0]
    stat_gemessen = -np.mean(fd[int(2.5 * ps.FS):int(3.5 * ps.FS)])
    sa, L = saiten[0], laengen[0]
    stat_formel = sa["T"] * d / xp * np.cos(np.radians(winkel))
    Fv = Fd + (tempo / d) / (1j * w) * (Fd + stat_gemessen / (1j * w))
    print(f"Harfe Taste {taste}: x_p {xp:.4f} m, L {L:.4f} m, F_stat gemessen {stat_gemessen:.6f} N, T d / x_p cos a {stat_formel:.6f} N "
          f"(Verhaeltnis {stat_gemessen / stat_formel:.6f})")
    f0 = 440.0 * 2 ** ((taste - 69) / 12)
    v, e = ps.VORLAUF, ps.VORLAUF + 0.4
    werte = {}
    for name, F in (("ohne", Fd), ("rampe", Fr), ("richtig", Fv), ("impuls", Fi)):
        x = ps.zeit(F[:, None])[:, 0]
        p = ps.zeit(F[:, None] * Hz)
        werte[name] = (ps.momentary_max(p), ps.rms_db(p, 1, 2) - ps.rms_db(p, v, e), kd.teiltoene(x, f0),
                       np.mean(x[int(2.5 * ps.FS):int(3.5 * ps.FS)]), np.max(np.abs(x)))
        lu, pg, tt, mittel, spitze = werte[name]
        print(f"  {name:7s}: Hoerort {lu:7.2f} LUFS, Pegel 1-2 s {pg:6.2f} dB | Stegkraft Mittel 2,5-3,5 s {mittel:+.4e} N, Spitze {spitze:.3e} N | "
              f"Teiltoene frueh 1-6 " + " ".join(f"{t[1]:6.1f}" for t in tt[:6]) + " dB", flush=True)
    for a, b in (("rampe", "impuls"), ("richtig", "impuls"), ("ohne", "impuls")):
        ta, tb = werte[a][2], werte[b][2]
        print(f"  {a} - {b}: Hoerort {werte[a][0] - werte[b][0]:+.3f} LU, Pegel 1-2 s {werte[a][1] - werte[b][1]:+.2f} dB, Teiltoene frueh "
              + " ".join(f"{x[1] - y[1]:+5.2f}" for x, y in zip(ta[:6], tb[:6])) + " dB, spaet "
              + " ".join(f"{x[2] - y[2]:+5.2f}" for x, y in zip(ta[:6], tb[:6])) + " dB")


if __name__ == "__main__":
    for t in sys.argv[1:]:
        rechne(int(t))

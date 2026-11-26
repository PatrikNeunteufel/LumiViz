"""S222, 34k-7d-1 — die Zupfstelle der Gitarre: 16 cm (Pavlidous Messort) gegen kleinere Abstaende vom Steg.

Aufruf (vom Projekt-Root):
    python -X utf8 -B <dieses Skript> laut <ort_m> [kappe=ja|nein]
        Lautheit je Taste nach KQ34 a am Pruefstand (wie `2026-11-18_S205_gitarre_kq35.py ton <koerper> datei`), die
        Zupfstelle `ort_m` vom Steg; `kappe=ja` (heute) an hohen Buenden hoechstens die halbe Saite, `kappe=nein` immer
        `ort_m` (die Hand bleibt stehen, Traube 2004).
    python -X utf8 -B <dieses Skript> iowa <Ordner Iowa_MIS_Guitar_mono2496> <ort_m> [<ort_m> ...] [staerke=mf]
            [kappe=ja|nein] [buende=0-12]
        Das Teiltonprofil je Ton gegen die Iowa-Gitarre (Raimundo 118, schalltot): je Ton der Staerke die Pegel der Spitze
        der Teiltoene 1-12 (bis 6 kHz, Messung wie `2026-11-15_S199_ausklang_je_teilton.py`), am Pruefstand dieselbe
        Saite und derselbe Bund mit jeder Zupfstelle. Der Pegel je Ton ist unbekannt (Abstand, Verstaerkung) — verglichen
        wird das Profil: Modell minus Iowa je Teilton, ohne den Median je Ton. Ausgabe je Zupfstelle der Median des
        Betrags (dB), ueber alle Toene und je Gruppe der Teiltoene, und der mittlere Rest je Teilton (zeigt eine Kerbe).

Warum das Profil: die Zupfstelle setzt Kerben im Spektrum bei n = L / x_p (16 cm an der leeren Saite: n = 4, 8, 12;
6,5 cm: n = 10). Die Lautheit ueber den Umfang trennt die Stellen kaum (alle unter den Schwellen nach KQ34 a).

Quellen der Stellen: Traube 2004 Abschn. 3.1 („between a third and a tenth of the string length (i.e. 3 to 20 cm)“),
Abschn. 3.3 (Sor: 1/10); Scherrer 2013 Abb. 3.7 (Biberian: „normal“ rund 1/10 der leeren Saite); Pavlidou 1997 nach
Scherrer Tab. 2.1 (Auslenkung gemessen bei 16 cm); Traube 2004 PDF-S. 156: die Hand gleicht den Bund meist nicht aus
(also fester Abstand vom Steg).

Schreibt nichts.
"""
import contextlib
import importlib.util
import io
import pathlib
import re
import sys

import numpy as np

ROOT = pathlib.Path.cwd()
P = ROOT / "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben"


def modul(pfad, name):
    spec = importlib.util.spec_from_file_location(name, pfad)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


def laut(ort, kappe):
    kq = modul(P / "2026-11-18_S205_gitarre_kq35.py", "kq35")
    alt = kq.modul

    def mit_ort(pfad, name):
        m = alt(pfad, name)
        if name == "pruefstand":
            m.ZUPFORT = ort
            if not kappe:  # die Hand bleibt stehen (Traube 2004): immer `ort` vom Steg, auch ueber die halbe Saite
                ton2 = m.ton2
                m.ton2 = lambda saite, laenge, x_p, *rest, **kw: ton2(saite, laenge, ort, *rest, **kw)
        return m

    kq.modul = mit_ort
    print(f"Zupfstelle {ort * 100:.1f} cm vom Steg" + (" (hoechstens die halbe Saite)" if kappe else " (fest)"), flush=True)
    kq.ton(str(ROOT / "asset/loop/instrument_bodies"), "datei", 30.0)


def iowa(ordner, orte, staerke, kappe=True, buende=(0, 12)):
    au = modul(P / "2026-11-15_S199_ausklang_je_teilton.py", "ausklang")
    ps = au.ps
    kp = ps.lade_koerper("classical_guitar")
    tan = ps.richtung_von(kp)
    w = ps.frequenzen()
    kurven = {}
    reste = {o: [] for o in orte}                 # je Ort: (Saite, Bund, n, Rest in dB)
    toene = 0
    print(f"Iowa {staerke}, Zupfstellen {', '.join(f'{o * 100:.1f} cm' for o in orte)}", flush=True)
    for pfad in sorted(pathlib.Path(ordner).glob(f"Guitar.{staerke}.*.aif")):
        m = re.fullmatch(rf"Guitar\.{staerke}\.(sul_?[A-G])\.([A-G]b?\d)([A-G]b?\d)?\.mono\.aif", pfad.name)
        if not m:
            continue
        s = au.SAITEN[m.group(1)]
        von, bis = au.midi(m.group(2)), au.midi(m.group(3) or m.group(2))
        x, fs = au.lies_aiff(pfad)
        st = au.stuecke(x, fs)
        if len(st) != bis - von + 1:
            print(f"# {pfad.name}: {len(st)} Stuecke, erwartet {bis - von + 1} - ausgelassen", flush=True)
            continue
        for (a, e), taste in zip(st, range(von, bis + 1)):
            bund = taste - ps.GITARRE[s]["taste"]
            if not buende[0] <= bund <= buende[1]:
                continue
            gem = {n: db for n, _, db, _, _ in au.teiltoene(x[a:e], fs, 440.0 * 2 ** ((taste - 69) / 12))}
            if len(gem) < 4:
                continue
            if s not in kurven:
                kurven[s] = ps.koerper_kurven2(kp, kp["tasten"].index(ps.GITARRE[s]["taste"]), w, tan)  # wie `gitarre`
            laenge = ps.MENSUR * 2 ** (-bund / 12)
            f0 = np.sqrt(ps.GITARRE[s]["T"] / ps.GITARRE[s]["m"]) / (2 * laenge)
            zeile = []
            for o in orte:
                y = ps.zeit(ps.ton2(ps.GITARRE[s], laenge, min(o, 0.5 * laenge) if kappe else o, 0.002, 30.0, *kurven[s], w))
                mod = {n: db for n, _, db, _, _ in au.teiltoene(y[:, 0] + y[:, 1], ps.FS, f0)}
                gemeinsam = [n for n in gem if n in mod]
                d = np.array([mod[n] - gem[n] for n in gemeinsam])
                d -= np.median(d)
                reste[o].extend((s, bund, n, r) for n, r in zip(gemeinsam, d))
                zeile.append(f"{o * 100:4.1f} cm {np.median(np.abs(d)):4.1f} dB")
            toene += 1
            print(f"Saite {6 - s} Bund {bund:2d} Taste {taste:3d} | " + " | ".join(zeile), flush=True)
    print(f"\n{toene} Toene. Median |Modell - Iowa| je Teilton, ohne den Median je Ton (dB):")
    gruppen = (("alle", 1, 12), ("n 1-3", 1, 3), ("n 4-6", 4, 6), ("n 7-9", 7, 9), ("n 10-12", 10, 12))
    print("Zupfstelle  " + "  ".join(f"{g:>8s}" for g, _, _ in gruppen))
    for o in orte:
        z = np.array(reste[o])
        teile = []
        for _, lo, hi in gruppen:
            g = z[(z[:, 2] >= lo) & (z[:, 2] <= hi)]
            teile.append(f"{np.median(np.abs(g[:, 3])):8.2f}" if len(g) else "       -")
        print(f"{o * 100:6.1f} cm  " + "  ".join(teile))
    print("\nMittlerer Rest je Teilton (Modell - Iowa, dB; negativ = Kerbe im Modell, die Iowa nicht hat):")
    print("Zupfstelle  " + " ".join(f"{n:6d}" for n in range(1, 13)))
    for o in orte:
        z = np.array(reste[o])
        print(f"{o * 100:6.1f} cm  " + " ".join(f"{np.mean(z[z[:, 2] == n, 3]):6.1f}" if np.any(z[:, 2] == n) else "     -" for n in range(1, 13)))


if __name__ == "__main__":
    opts = dict(a.split("=") for a in sys.argv[1:] if "=" in a)
    args = [a for a in sys.argv[1:] if "=" not in a]
    kappe = opts.get("kappe", "ja") == "ja"
    if len(args) >= 2 and args[0] == "laut":
        laut(float(args[1]), kappe)
    elif len(args) >= 3 and args[0] == "iowa":
        von, bis = (int(b) for b in opts.get("buende", "0-12").split("-"))
        iowa(args[1], [float(a) for a in args[2:]], opts.get("staerke", "mf"), kappe, (von, bis))
    else:
        sys.exit(__doc__)

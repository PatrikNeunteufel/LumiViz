"""Reihe ueber Annahmen der Laute: je Fall Bauplan, Formen, Admittanz am Messpunkt, Abstand zu den vier Zielen (KQ11 a).

    python 2026-11-13_S197_laute_reihe.py <faelle.txt> <arbeitsordner>      je Zeile: <name> <Schalter fuer laute_bauplan.py ...>
"""
import importlib.util
import pathlib
import subprocess
import sys

import numpy as np

S = pathlib.Path(__file__).resolve().parent
TOOL = "out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe"  # vom Projekt-Root aus
spec = importlib.util.spec_from_file_location("adm", S / "2026-11-13_S197_laute_admittanz.py")
adm = importlib.util.module_from_spec(spec)
spec.loader.exec_module(adm)
adm.SAITEN = 2.47
ZIELE = [(132.2, -28.0), (277.1, -17.4), (340.6, -19.6), (475.6, -11.8)]
MASSSTAB = 0.923
PUNKT = (0.0525 * MASSSTAB, 0.095 * MASSSTAB)


def fall(name, schalter):
    plan = pathlib.Path(sys.argv[2]) / f"{name}.lvinstbodyplan"
    subprocess.run([sys.executable, "-X", "utf8", "-I", str(S / "2026-11-13_S197_laute_bauplan.py"), str(S / "2026-11-13_S197_laute_umriss.json"), str(plan),
                    "--massstab", str(MASSSTAB)] + schalter, check=True, capture_output=True)
    r = subprocess.run([TOOL, str(plan), "--to", "1000", "--statistics-to", "0", "--out", str(plan.with_suffix(".lvinstbody")),
                        "--shapes", str(plan.with_suffix(".txt"))], capture_output=True)
    if r.returncode != 0:
        return f"{name}: FEHLER {r.stdout[-300:]!r}"
    knoten, hz, w = adm.formen(str(plan.with_suffix(".txt")))
    f = np.arange(70.0, 1000.0, 0.25)
    d, _ = adm.admittanz(knoten, hz, w, f, PUNKT[0], PUNKT[1], 0.03)
    ger = [p for p in adm.spitzen(f, d, 2.0) if p[0] < 650.0]
    teile, schlecht_f, schlecht_d = [], 0.0, 0.0
    for zf, zd in ZIELE:
        # die staerkste gerechnete Spitze innerhalb von 8 % um das Ziel, sonst die naechste
        nah = [p for p in ger if abs(p[0] / zf - 1.0) <= 0.08]
        q = max(nah, key=lambda p: p[1]) if nah else min(ger, key=lambda p: abs(np.log(p[0] / zf)))
        df, dd = 100.0 * (q[0] / zf - 1.0), q[1] - zd
        schlecht_f, schlecht_d = max(schlecht_f, abs(df)), max(schlecht_d, abs(dd))
        teile.append(f"{q[0]:.0f} ({df:+.1f} %, {dd:+.1f} dB)")
    marke = "OK " if schlecht_f <= 5.0 and schlecht_d <= 6.0 else "   "
    return f"{marke}{name}: " + "  ".join(teile) + f"   | max {schlecht_f:.1f} % {schlecht_d:.1f} dB | alle: " + " ".join(f"{p[0]:.0f}/{p[1]:.0f}" for p in ger)


if __name__ == "__main__":
    for zeile in open(sys.argv[1], encoding="utf-8"):
        t = zeile.split()
        if t and not t[0].startswith("#"):
            print(fall(t[0], t[1:]), flush=True)

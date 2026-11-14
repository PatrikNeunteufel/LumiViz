"""KQ18 a (34k-5, S198): die Drehfeder am Rand des Harfenbodens mit dem echten Element — `InstrumentBodyTool
--supported-rim --rim-spring <N> [--rim-spring-end <N>]` je Feder, die Formen den Familien zugeordnet, gegen Bell 1988.

    python …_harfe_rand_werkzeug.py <InstrumentBodyTool.exe> <plan.lvinstbodyplan> <arbeitsordner> <bass:diskant> …

Je Angabe `bass:diskant` ein Lauf: Feder in N am Bassende (y = 0) und am Diskantende; `bass` allein = gleiche Feder;
`0` = Rand gestuetzt. Zwischen den Enden laeuft 1 / Feder linear (die Breite des Bodens laeuft linear: Feder ∝ 1 / Breite
ist `k:k*4.1`).
"""
import importlib.util
import os
import subprocess
import sys

import numpy as np

HIER = os.path.dirname(os.path.abspath(__file__))


def lade(name, datei):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HIER, datei))
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)
    return modul


hm = lade("harfe_moden", "2026-11-13_S197_harfe_moden.py")
feder = lade("harfe_rand_feder", "2026-11-13_S198_harfe_rand_feder.py")

if __name__ == "__main__":
    werkzeug, plan, ordner = sys.argv[1:4]
    for angabe in sys.argv[4:]:
        teile = [float(v) for v in angabe.split(":")]
        bass, diskant = teile[0], teile[-1]
        formen = os.path.join(ordner, f"rand_{angabe.replace(':', '_')}.txt")
        befehl = [werkzeug, plan, "--supported-rim", "--to", "1100", "--mesh-to", "5000", "--statistics-to", "0", "--shapes", formen,
                  "--out", os.path.join(ordner, "rand_probe.lvinstbody")]
        if bass > 0:
            befehl += ["--rim-spring", str(bass), "--rim-spring-end", str(diskant)]
        lauf = subprocess.run(befehl, capture_output=True, text=True)
        if lauf.returncode != 0:
            print(angabe, "Fehler:", lauf.stderr.strip())
            continue
        knoten, hz, w = hm.formen(formen)
        gefunden = feder.familien(knoten, hz, w, 1100.0)
        print(f"Feder {bass:.0f} N am Bass, {diskant:.0f} N am Diskant")
        for titel, ziel in feder.ZIELE.items():
            print(f"    {titel}: {feder.zeile(gefunden, ziel)}")
        sys.stdout.flush()

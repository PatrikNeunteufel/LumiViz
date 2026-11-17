"""Liest umlauf_entwurf.txt und gibt je Taste drei kompakte Zeilen aus (Cent 1-12/<=5k/hoerbar je Dispersion;
Abklingen % 1-12/<=5k/hoerbar und max|H| je Verlust). Aufruf: python -B kompakt.py"""
import pathlib
import re

txt = (pathlib.Path(__file__).resolve().parent / "umlauf_entwurf.txt").read_text(encoding="utf-8").splitlines()
block = None
for z in txt:
    if z.startswith("###"):
        m = re.match(r"### (.*?): f1 ([\d.]+) Hz.*B ([\d.e+-]+), Umlauf ([\d.]+) Takte, Teiltoene bis 20 kHz (\d+), davon mit sigma <= 100 1/s (\d+) \(bis (\d+) Hz\)", z)
        print(f"\n{m.group(1)} | f1 {m.group(2)} | B {m.group(3)} | D {m.group(4)} | n {m.group(5)}, hoerbar {m.group(6)} bis {m.group(7)} Hz")
        a, b = [], []
        block = (a, b)
        continue
    t = z.split("|")
    if block is None or len(t) < 4:
        continue
    name = t[0].strip()
    if len(t) == 6 and name not in ("pol1", "pol1p", "iir2", "iir4", "iir8"):
        c = t[2].split()
        h = t[5].split()
        block[0].append(f"{name}: {c[1]}/{c[2]}/{h[0]}")
        if name.startswith("adf"):
            print("  Cent 1-12/<=5k/hoerbar: " + "; ".join(block[0]))
    elif len(t) == 6:
        p = t[2].split()
        h = t[5].split()
        mx = t[1].split()
        block[1].append(f"{name}: {p[1]}/{p[2]}/{h[0]} ({mx[0]}{'*' if len(mx) > 1 else ''})")
        if name.startswith("iir8"):
            print("  Abkl % 1-12/<=5k/hoerbar (max|H|): " + "; ".join(block[1]))
    elif len(t) == 4:
        print(f"  {name}: Grundton {t[1].strip()} | hoerbar Cent/% {t[2].strip()} | unmoeglich {t[3].strip()}")

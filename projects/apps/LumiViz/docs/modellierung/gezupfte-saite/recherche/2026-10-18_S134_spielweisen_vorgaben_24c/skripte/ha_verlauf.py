"""Verlauf der Teiltoene der leeren Saite, die keinen Knoten am Finger haben, in Flageolett-Noten.

    python -I ha_verlauf.py <ds2-ordner>
Je Note: Pegel (dB, relativ zum Flageolett-Grundton n*f0 im selben Rahmen) des leeren-Saite-Grundtons f0
(kein Knoten am Finger) zu den Zeiten 0..400 ms nach dem Einsatz, Fenster 2048 (46 ms), Mitte des Fensters.
Dazu der Pegel des Flageolett-Grundtons selbst relativ zu seinem Wert bei 25 ms.
"""
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from messe_idmt import FS, lies_noten, lies_wav  # noqa: E402

HA = {"XII": 2, "VII": 3, "V": 4}
LEER = {1: 40, 2: 45, 3: 50, 4: 55, 5: 59, 6: 64}
ZEITEN = (25, 50, 75, 100, 150, 200, 300, 400)


def pegel(x, a, f, t_ms, N=2048):
    i = int((a + t_ms / 1000) * FS) - N // 2
    s = x[max(0, i):max(0, i) + N] * np.hanning(N)
    X = np.abs(np.fft.rfft(s, 1 << 15))
    fr = np.fft.rfftfreq(1 << 15, 1 / FS)
    b = (fr > f * 0.96) & (fr < f * 1.04)
    return 20 * np.log10(X[b].max() + 1e-12)


def main():
    ds = Path(sys.argv[1])
    print("| Gitarre | Bund (n) | Saite | f0 leer | " + " | ".join(f"{t} ms" for t in ZEITEN) + " | HA-Grundton 25->400 ms |")
    print("|---|---|---|---|" + "---|" * len(ZEITEN) + "---|")
    for g in ("AR", "FS", "LP"):
        for h, n in HA.items():
            x = lies_wav(ds / "audio" / f"{g}_NH_{h}.wav")
            for no in lies_noten(ds / "annotation" / f"{g}_NH_{h}.xml"):
                if no["string"] > 3:
                    continue
                f0 = 440 * 2 ** ((LEER[no["string"]] - 69) / 12)
                # tatsaechlicher Flageolett-Grundton aus der Note
                fh = n * f0
                a = no["onset"]
                rel = [pegel(x, a, f0, t) - pegel(x, a, fh, t) for t in ZEITEN]
                hv = pegel(x, a, fh, 400) - pegel(x, a, fh, 25)
                print(f"| {g} | {h} ({n}) | {no['string']} | {f0:.0f} | " + " | ".join(f"{v:+.0f}" for v in rel) + f" | {hv:+.1f} |")


if __name__ == "__main__":
    main()

"""Normale Noten am 5. Bund aus dataset1 (2,3 s je Note): T60 je Teilton.

    python -I normal_ds1.py <ds1-ordner>
"""
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from messe_idmt import lies_noten, lies_wav, miss_note  # noqa: E402


def t60(r):
    return float("inf") if np.isnan(r) or r >= 0 else 60 / -r


def main():
    ds = Path(sys.argv[1])
    print("| Gitarre | pitch | f0 | exc | T60 k=1 | k=2 | k=3 | Median k=4..8 | Spitze dBFS |")
    print("|---|---|---|---|---|---|---|---|---|")
    for g in sorted(ds.iterdir()):
        for w in sorted((g / "audio").glob("*.wav")):
            n = lies_noten(g / "annotation" / (w.stem + ".xml"))[0]
            x = lies_wav(w)
            r = miss_note(x, n["onset"], n["offset"] - 0.02, n["pitch"])
            t = r["teil"]
            h = np.median([t60(q[3]) for q in t if 4 <= q[0] <= 8])
            print(f"| {g.name} | {n['pitch']} | {r['f0']:.0f} | {n['exc']} | {t60(t[0][3]):.2f} | {t60(t[1][3]):.2f} | {t60(t[2][3]):.2f} | {h:.2f} | {r['spitze']:.1f} |")


if __name__ == "__main__":
    main()

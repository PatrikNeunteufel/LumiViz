"""IDMT-SMT-Bass: MU gegen FS (Abklingen), HA gegen FS (Pegel).

    python -I messe_bass.py <wav-ordner>
Eine Note je Datei; Einsatz = erster 10-ms-Rahmen 30 dB ueber dem Ruhepegel am Anfang.
"""
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from messe_idmt import FS, lies_wav, miss_note  # noqa: E402

LEER = {1: 28, 2: 33, 3: 38, 4: 43}  # E1 A1 D2 G2 (MIDI)
NAME = {1: "Fame Baphomet NTB", 2: "Warwick", 3: "Yamaha BB604"}
HA_N = {12: 2, 7: 3, 5: 4}


def einsatz(x):
    r = 441
    m = len(x) // r
    db = 20 * np.log10(np.sqrt(np.mean(x[: m * r].reshape(m, r) ** 2, axis=1)) + 1e-12)
    ruhe = np.median(db[:5])
    i = int(np.argmax(db > ruhe + 30))
    return i * r / FS


def miss(pfad, pitch):
    x = lies_wav(pfad)
    a = max(0.0, einsatz(x) - 0.005)
    return miss_note(x, a, len(x) / FS - 0.02, pitch)


def t60(r):
    return float("inf") if np.isnan(r) or r >= 0 else 60 / -r


def hoch(teil, lo=4, hi=8):
    v = [t60(t[3]) for t in teil if lo <= t[0] <= hi]
    return float(np.median(v)) if v else float("nan")


def main():
    d = Path(sys.argv[1])
    print("## Bass: muted (MU) gegen finger-style (FS), Tonabnehmer neck+bridge")
    print("| Bass | Saite | Bund | f0 | T60 FS k=1 | T60 MU k=1 | Verh. | T60 FS k4..8 | T60 MU k4..8 | Verh. | Spitze MU-FS dB | Laenge FS / MU s |")
    print("|---|---|---|---|---|---|---|---|---|---|---|---|")
    v1, vh = [], []
    for b in (1, 2, 3):
        for s in (1, 2, 3, 4):
            for f in (0, 5):
                p = LEER[s] + f
                rf = miss(d / f"BS_{b}_EQ_2_FS_NO_{s}_{f}.wav", p)
                rm = miss(d / f"BS_{b}_EQ_2_MU_NO_{s}_{f}.wav", p)
                a1, b1 = t60(rf["teil"][0][3]), t60(rm["teil"][0][3])
                ah, bh = hoch(rf["teil"]), hoch(rm["teil"])
                v1.append(b1 / a1)
                vh.append(bh / ah)
                print(f"| {NAME[b]} | {s} | {f} | {rf['f0']:.0f} | {a1:.2f} | {b1:.2f} | {b1/a1:.3f} | {ah:.2f} | {bh:.2f} | {bh/ah:.3f} | {rm['spitze']-rf['spitze']:+.1f} | {rf['dauer']:.1f} / {rm['dauer']:.1f} |")
    v1, vh = np.array(v1), np.array(vh)
    print(f"\nMedian Verhaeltnis T60 MU/FS: Grundton {np.median(v1):.3f} ({v1.min():.3f}..{v1.max():.3f}), k4..8 {np.nanmedian(vh):.3f} ({np.nanmin(vh):.3f}..{np.nanmax(vh):.3f})")
    print("\n## Bass: Flageolett (FS HA) gegen leere Saite (FS NO Bund 0) und gegriffen gleiche Hoehe")
    print("| Bass | Saite | Bund (n) | Spitze gegen leer dB | E0,5 gegen leer dB | Spitze gegen gegriffen dB | T60 HA | T60 gegriffen |")
    print("|---|---|---|---|---|---|---|---|")
    sp, en = {2: [], 3: [], 4: []}, {2: [], 3: [], 4: []}
    for b in (1, 2, 3):
        for s in (1, 2, 3, 4):
            r0 = miss(d / f"BS_{b}_EQ_2_FS_NO_{s}_0.wav", LEER[s])
            r12 = miss(d / f"BS_{b}_EQ_2_FS_NO_{s}_12.wav", LEER[s] + 12)
            for f, n in HA_N.items():
                p = LEER[s] + round(12 * np.log2(n))
                rh = miss(d / f"BS_{b}_EQ_2_FS_HA_{s}_{f}.wav", p)
                g = f"{rh['spitze'] - r12['spitze']:+.1f}" if n == 2 else "—"
                gt = f"{t60(r12['teil'][0][3]):.1f}" if n == 2 else "—"
                sp[n].append(rh["spitze"] - r0["spitze"])
                en[n].append(rh["e05"] - r0["e05"])
                print(f"| {NAME[b]} | {s} | {f} ({n}) | {rh['spitze'] - r0['spitze']:+.1f} | {rh['e05'] - r0['e05']:+.1f} | {g} | {t60(rh['teil'][0][3]):.1f} | {gt} |")
    for n in (2, 3, 4):
        print(f"n={n}: Spitze gegen leer Median {np.median(sp[n]):+.1f} dB ({min(sp[n]):+.1f}..{max(sp[n]):+.1f}), E0,5 Median {np.median(en[n]):+.1f} dB ({min(en[n]):+.1f}..{max(en[n]):+.1f})")


if __name__ == "__main__":
    main()

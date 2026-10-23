"""Zusammenfassung der IDMT-Messung: Tabellen fuer den Bericht.

    python -I fasse_idmt.py <ds2-ordner>
"""
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).parent))
from messe_idmt import FS, fenster, lies_noten, lies_wav, miss_note  # noqa: E402

SAITEN = ("E", "A", "D", "G", "B", "E1")
HA = {"XII": 2, "VII": 3, "V": 4, "IV": 5, "IX": 5}
NAME = {"AR": "Aristides 010", "FS": "Fender Stratocaster", "LP": "Gibson Les Paul"}


def t60(rate):
    return float("inf") if not rate or rate >= 0 or np.isnan(rate) else 60.0 / -rate


def hoch(teil):
    r = [t[3] for t in teil if 4 <= t[0] <= 8 and not np.isnan(t[3])]
    return float(np.median(r)) if r else float("nan")


def teilpegel(x, a, f, t_ms, N=4096):
    i = int((a + t_ms / 1000) * FS)
    s = x[i:i + N] * np.hanning(N)
    X = np.abs(np.fft.rfft(s, 1 << 15))
    fr = np.fft.rfftfreq(1 << 15, 1 / FS)
    b = (fr > f * 0.97) & (fr < f * 1.03)
    return 20 * np.log10(X[b].max() + 1e-12)


def main():
    ds = Path(sys.argv[1])
    print("## Palm mute: T60 in s (Grundton k=1 / Median k=4..8), Spitzenpegel gegen normal in dB, Energie 0,5 s gegen normal in dB")
    alle = {i: [] for i in range(5)}
    for g in ("AR", "FS", "LP"):
        print(f"\n### {NAME[g]} ({g})")
        print("| Saite | f0 | normal | M1 | M2 | M3 | M4 | M5 | Spitze M1..M5 | E0,5 M1..M5 | f0 M gegen normal (Cent) |")
        print("|---|---|---|---|---|---|---|---|---|---|---|")
        for s in SAITEN:
            xp = lies_wav(ds / "audio" / f"{g}_{s}_fret_0-20.wav")
            np_ = lies_noten(ds / "annotation" / f"{g}_{s}_fret_0-20.xml")
            xm = lies_wav(ds / "audio" / f"{g}_{s}_muted5.wav")
            nm = lies_noten(ds / "annotation" / f"{g}_{s}_muted5.xml")
            ip = [i for i, n in enumerate(np_) if n["fret"] == nm[0]["fret"]][0]
            a, e = fenster(np_, ip, len(xp))
            rn = miss_note(xp, a, e, np_[ip]["pitch"])
            zellen = [f"{t60(rn['teil'][0][3]):.2f} / {t60(hoch(rn['teil'])):.2f}"]
            sp, en, ce = [], [], []
            for i in range(len(nm)):
                a, e = fenster(nm, i, len(xm))
                r = miss_note(xm, a, e, nm[i]["pitch"])
                if r is None:
                    zellen.append("—")
                    continue
                zellen.append(f"{t60(r['teil'][0][3]):.2f} / {t60(hoch(r['teil'])):.2f}")
                sp.append(f"{r['spitze'] - rn['spitze']:+.0f}")
                en.append(f"{r['e05'] - rn['e05']:+.0f}")
                ce.append(1200 * np.log2(r["f0"] / rn["f0"]))
                alle[i].append((t60(r['teil'][0][3]) / t60(rn['teil'][0][3]), t60(hoch(r['teil'])) / t60(hoch(rn['teil'])),
                                t60(r['teil'][0][3]), rn["f0"]))
            print(f"| {s} (str {nm[0]['string']}, Bund {nm[0]['fret']}) | {rn['f0']:.0f} | " + " | ".join(zellen)
                  + f" | {' '.join(sp)} | {' '.join(en)} | {np.median(ce):+.0f} |")
    print("\n### Ueber alle 18 Saiten: Median (Spanne) des Verhaeltnisses T60 muted / T60 normal")
    for i in range(5):
        v1 = np.array([x[0] for x in alle[i]])
        vh = np.array([x[1] for x in alle[i]])
        ta = np.array([x[2] for x in alle[i]])
        print(f"M{i+1}: Grundton {np.median(v1):.3f} ({v1.min():.3f}..{v1.max():.3f}), k4-8 {np.nanmedian(vh):.3f} ({np.nanmin(vh):.3f}..{np.nanmax(vh):.3f}), T60 Grundton absolut Median {np.median(ta):.2f} s ({ta.min():.2f}..{ta.max():.2f})")

    print("\n## Flageoletts: Pegel gegen den normalen Ton der leeren Saite (Bund 0) und gegen den gegriffenen Ton gleicher Hoehe")
    for g in ("AR", "FS", "LP"):
        print(f"\n### {NAME[g]} ({g})")
        print("| Bund (n) | Saite | Spitze gegen leer | E0,5 gegen leer | Spitze gegen gleiche Hoehe gegriffen | T60 Flageolett-Grundton | T60 gegriffen gleiche Hoehe | Rest leere-Saite-Grundton bei 50 / 300 ms (dB unter Flageolett-Grundton) |")
        print("|---|---|---|---|---|---|---|---|")
        normal = {}
        for s in SAITEN:
            xp = lies_wav(ds / "audio" / f"{g}_{s}_fret_0-20.wav")
            np_ = lies_noten(ds / "annotation" / f"{g}_{s}_fret_0-20.xml")
            normal[np_[0]["string"]] = (xp, np_)
        for h, n in HA.items():
            x = lies_wav(ds / "audio" / f"{g}_NH_{h}.wav")
            nn = lies_noten(ds / "annotation" / f"{g}_NH_{h}.xml")
            for i, no in enumerate(nn):
                a, e = fenster(nn, i, len(x))
                r = miss_note(x, a, e, no["pitch"])
                xp, np_ = normal[no["string"]]
                i0 = [k for k, m in enumerate(np_) if m["fret"] == 0][0]
                a0, e0 = fenster(np_, i0, len(xp))
                r0 = miss_note(xp, a0, e0, np_[i0]["pitch"])
                gl = [k for k, m in enumerate(np_) if m["pitch"] == no["pitch"]]
                if gl:
                    ag, eg = fenster(np_, gl[0], len(xp))
                    rg = miss_note(xp, ag, eg, np_[gl[0]]["pitch"])
                    gls = f"{r['spitze'] - rg['spitze']:+.1f} (Bund {np_[gl[0]]['fret']})"
                    glt = f"{t60(rg['teil'][0][3]):.1f}"
                else:
                    gls, glt = "—", "—"
                f_leer = r["f0"] / n
                rest = [teilpegel(x, a, f_leer, t) - teilpegel(x, a, r["f0"], t) for t in (50, 300)]
                print(f"| {h} ({n}) | {no['string']} | {r['spitze'] - r0['spitze']:+.1f} | {r['e05'] - r0['e05']:+.1f} | {gls} | {t60(r['teil'][0][3]):.1f} | {glt} | {rest[0]:+.0f} / {rest[1]:+.0f} |")


if __name__ == "__main__":
    main()

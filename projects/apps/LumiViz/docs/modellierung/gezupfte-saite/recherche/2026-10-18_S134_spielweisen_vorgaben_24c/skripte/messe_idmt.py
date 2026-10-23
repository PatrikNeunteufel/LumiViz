"""Messung an IDMT-SMT-Guitar, dataset2 (Init-Aufnahmen): muted vs. normal, Flageolett vs. normal.

    python -I messe_idmt.py <ds2-ordner>

Je Note: Spitzenpegel (RMS 10 ms, max in den ersten 80 ms), Abklingrate gesamt (Regression dB/s
ueber das Fenster), je Teilton k=1..10 Abklingrate (STFT 4096/512), Startpegel je Teilton,
Grundton gemessen.
"""
import sys
import re
import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np

FS = 44100


def lies_wav(pfad):
    import wave
    with wave.open(str(pfad)) as w:
        n, sw, ch = w.getnframes(), w.getsampwidth(), w.getnchannels()
        roh = w.readframes(n)
    if sw == 3:
        a = np.frombuffer(roh, dtype=np.uint8).reshape(-1, 3)
        x = (a[:, 0].astype(np.int32) | (a[:, 1].astype(np.int32) << 8) | (a[:, 2].astype(np.int32) << 16))
        x = np.where(x >= 1 << 23, x - (1 << 24), x).astype(np.float64) / (1 << 23)
    else:
        x = np.frombuffer(roh, dtype=np.int16).astype(np.float64) / 32768
    if ch > 1:
        x = x.reshape(-1, ch).mean(axis=1)
    return x


def lies_noten(pfad):
    wurzel = ET.parse(pfad).getroot()
    noten = []
    for e in wurzel.iter("event"):
        d = {c.tag: c.text for c in e}
        noten.append(dict(
            pitch=int(d["pitch"]), onset=float(d["onsetSec"]),
            offset=float(d.get("offsetSec") or 0), fret=int(d.get("fretNumber") or -1),
            string=int(d.get("stringNumber") or -1), exc=d.get("excitationStyle"),
            expr=d.get("expressionStyle")))
    noten.sort(key=lambda n: n["onset"])
    return noten


def fenster(noten, i, laenge):
    a = noten[i]["onset"]
    e = noten[i + 1]["onset"] if i + 1 < len(noten) else laenge / FS
    if noten[i]["offset"] > a:
        e = min(e, noten[i]["offset"])
    return a, e - 0.02


def regress(t, y):
    if len(t) < 4:
        return float("nan")
    p = np.polyfit(t, y, 1)
    return p[0]


def miss_note(x, a, e, pitch):
    i0, i1 = int(a * FS), int(e * FS)
    s = x[i0:i1]
    # RMS-Huellkurve 10 ms
    rahmen = 441
    m = len(s) // rahmen
    rms = np.sqrt(np.mean(s[: m * rahmen].reshape(m, rahmen) ** 2, axis=1) + 1e-20)
    db = 20 * np.log10(rms)
    kspitze = int(np.argmax(db[:8]))
    spitze = db[kspitze]
    # Abklingrate gesamt: ab Spitze + 30 ms, bis 45 dB unter Spitze oder Fensterende
    t = (np.arange(m) + 0.5) * rahmen / FS
    k0 = kspitze + 3
    k1 = m
    unter = np.where(db[k0:] < spitze - 45)[0]
    if len(unter):
        k1 = k0 + unter[0]
    rate_ges = regress(t[k0:k1], db[k0:k1])
    # Teiltoene
    f0_soll = 440 * 2 ** ((pitch - 69) / 12)
    N, H = 2048, 256
    win = np.hanning(N)
    fr = []
    for st in range(0, len(s) - N, H):
        X = np.abs(np.fft.rfft(s[st:st + N] * win))
        fr.append(X)
    if not fr:
        return None
    S = np.array(fr)
    frq = np.fft.rfftfreq(N, 1 / FS)
    tt = (np.arange(len(S)) * H + N / 2) / FS
    # Grundton aus langem Spektrum
    lang = s[: min(len(s), 32768)]
    XL = np.abs(np.fft.rfft(lang * np.hanning(len(lang)), 1 << 18))
    fL = np.fft.rfftfreq(1 << 18, 1 / FS)
    band = (fL > f0_soll * 0.94) & (fL < f0_soll * 1.06)
    f0 = fL[band][np.argmax(XL[band])]
    teil = []
    for k in range(1, 11):
        fk = k * f0
        if fk > 8000:
            break
        breite = max(fk * 0.03, 1.5 * FS / N)
        b = (frq > fk - breite) & (frq < fk + breite)
        lvl = 20 * np.log10(S[:, b].max(axis=1) + 1e-12)
        start = lvl[0]
        # Regression bis 40 dB unter Start oder Ende
        j1 = len(lvl)
        u = np.where(lvl < start - 40)[0]
        if len(u):
            j1 = u[0]
        r = regress(tt[:j1], lvl[:j1])
        teil.append((k, round(fk, 1), round(start, 1), round(r, 1)))
    # Energie in den ersten 0.5 s
    e05 = 10 * np.log10(np.sum(s[: int(0.5 * FS)] ** 2) + 1e-20)
    return dict(spitze=round(spitze, 1), rate=round(rate_ges, 1), dauer=round(e - a, 2),
                f0=round(f0, 2), f0soll=round(f0_soll, 2), e05=round(e05, 1), teil=teil)


def main():
    ds = Path(sys.argv[1])
    for g in ("AR", "FS", "LP"):
        print(f"=================== Gitarre {g}")
        for s in ("E", "A", "D", "G", "B", "E1"):
            xp = lies_wav(ds / "audio" / f"{g}_{s}_fret_0-20.wav")
            np_ = lies_noten(ds / "annotation" / f"{g}_{s}_fret_0-20.xml")
            xm = lies_wav(ds / "audio" / f"{g}_{s}_muted5.wav")
            nm = lies_noten(ds / "annotation" / f"{g}_{s}_muted5.xml")
            iplain = [i for i, n in enumerate(np_) if n["fret"] == nm[0]["fret"]]
            print(f"--- Saite {s} (string {nm[0]['string']}), Bund {nm[0]['fret']}, pitch {nm[0]['pitch']}, exc normal {np_[iplain[0]]['exc'] if iplain else '?'} / mute {nm[0]['exc']}")
            for i in iplain:
                a, e = fenster(np_, i, len(xp))
                r = miss_note(xp, a, e, np_[i]["pitch"])
                print("  normal", {k: v for k, v in r.items() if k != "teil"})
                print("     teil", r["teil"])
            for i in range(len(nm)):
                a, e = fenster(nm, i, len(xm))
                r = miss_note(xm, a, e, nm[i]["pitch"])
                print("  muted ", {k: v for k, v in r.items() if k != "teil"})
                print("     teil", r["teil"])
        # Flageoletts
        for h in ("XII", "VII", "V", "IV", "IX"):
            p = ds / "audio" / f"{g}_NH_{h}.wav"
            x = lies_wav(p)
            nn = lies_noten(ds / "annotation" / f"{g}_NH_{h}.xml")
            print(f"--- Flageolett {h}")
            for i, n in enumerate(nn):
                a, e = fenster(nn, i, len(x))
                r = miss_note(x, a, e, n["pitch"])
                print(f"  HA string {n['string']} fret {n['fret']} pitch {n['pitch']}", {k: v for k, v in r.items() if k != "teil"})
                print("     teil", r["teil"][:6])


if __name__ == "__main__":
    main()

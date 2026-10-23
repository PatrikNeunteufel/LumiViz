"""Messung an IDMT-SMT-Guitar dataset2: Bend, Slide, Vibrato (Einzelaufnahmen G-Saite Bund 5) und
Bend/Slide-Ereignisse in den Licks; Bezug: normaler Ton derselben Saite aus <g>_G_fret_0-20.

    python -I messe_bend_slide.py <ds2-ordner mit audio/ und annotation/> <ausgabeordner>

Methode
- Grundton: YIN (Cheveigne/Kawahara 2002), Fenster 1024 (23 ms), Schritt 220 (5 ms), Schwelle 0,15,
  parabolische Verfeinerung; Suchbereich 150..900 Hz. Cent gegen 440 Hz * 2^((pitch-69)/12) der Annotation.
- Pegel: RMS ueber 10 ms (441 Werte), in dB.
- Bend: Hub = 95.-Perzentil der Cent-Abweichung (geglaettet, Median 5 Rahmen) minus Anfangswert
  (Median 20..60 ms nach Onset). Anstieg 10 %/90 % des Hubs: erste Ueberschreitung. Kurvenform: Lage
  der 25/50/75-%-Punkte im Anstieg (0 = 10-%-Punkt, 1 = 90-%-Punkt). Rueckgang: erste Unterschreitung
  von 50 % des Hubs nach dem Hoechstwert.
- Pegel beim Bend: Abklingrate (Regression dB/s) im Fenster vor dem Bend (Onset+30 ms .. Beginn) und
  waehrend des Haltens (90-%-Punkt .. Rueckgang oder Ende), dazu Rate des normalen Tons derselben Hoehe
  (Bund 5) im gleichen Zeitabstand nach seinem Onset.
- Slide: Startwert/Endwert = Median der ersten 15 % / letzten 25 % der Cent-Kurve des Ereignisses;
  Beginn = erster Rahmen > 25 Cent vom Startwert in Richtung Ziel, Ende = erster Rahmen danach
  innerhalb 25 Cent des Endwerts. Stufigkeit: Anteil der Rahmen waehrend des Gleitens, die innerhalb
  +-20 Cent eines ganzen Halbtons liegen (gleichverteilt erwartet: 40 %). Linearitaet: R^2 der
  Geraden in Cent ueber t und in Hz ueber t. Pegel: RMS 30 ms vor Beginn und 30 ms nach Ende; Abfall
  gegen die Abklingrate des normalen Tons ueber dieselbe Dauer = Mehrverlust.
"""
import sys
import wave
import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np

FS = 44100
N_FENSTER = 1024
SCHRITT = 220


def lies_wav(pfad):
    with wave.open(str(pfad)) as w:
        n, sw, ch, fs = w.getnframes(), w.getsampwidth(), w.getnchannels(), w.getframerate()
        roh = w.readframes(n)
    assert fs == FS, fs
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
    noten = []
    for e in ET.parse(pfad).getroot().iter("event"):
        d = {c.tag: (c.text or "").strip() for c in e}
        noten.append(dict(
            pitch=int(d["pitch"]), onset=float(d["onsetSec"]), offset=float(d.get("offsetSec") or 0),
            fret=int(d.get("fretNumber") or -1), string=int(d.get("stringNumber") or -1),
            exc=d.get("excitationStyle"), expr=d.get("expressionStyle"),
            mf=float(d.get("modulationFrequency") or 0), mr=float(d.get("modulationFrequencyRange") or 0)))
    noten.sort(key=lambda n: n["onset"])
    return noten


def yin(s, fmin=150.0, fmax=900.0, schwelle=0.15):
    tmin, tmax = int(FS / fmax), int(FS / fmin)
    f0 = []
    for st in range(0, len(s) - N_FENSTER - tmax, SCHRITT):
        r = s[st:st + N_FENSTER + tmax]
        x0 = r[:N_FENSTER]
        e0 = np.sum(x0 ** 2)
        if e0 < 1e-10:
            f0.append(np.nan)
            continue
        d = np.empty(tmax + 1)
        d[0] = 0
        for tau in range(1, tmax + 1):
            dx = x0 - r[tau:tau + N_FENSTER]
            d[tau] = np.dot(dx, dx)
        cm = d[1:] * np.arange(1, tmax + 1) / np.maximum(np.cumsum(d[1:]), 1e-20)
        cm = np.concatenate([[1.0], cm])
        kand = np.where(cm[tmin:] < schwelle)[0]
        if len(kand) == 0:
            f0.append(np.nan)
            continue
        t = kand[0] + tmin
        while t + 1 <= tmax and cm[t + 1] < cm[t]:
            t += 1
        if 1 <= t < tmax:
            a, b, c = cm[t - 1], cm[t], cm[t + 1]
            nenner = a - 2 * b + c
            dt = 0.5 * (a - c) / nenner if nenner != 0 else 0
        else:
            dt = 0
        f0.append(FS / (t + dt))
    return np.array(f0)


def rms_db(s, rahmen=441):
    m = len(s) // rahmen
    r = np.sqrt(np.mean(s[: m * rahmen].reshape(m, rahmen) ** 2, axis=1) + 1e-20)
    return 20 * np.log10(r), (np.arange(m) + 0.5) * rahmen / FS


def glatt(y, k=5):
    out = y.copy()
    h = k // 2
    for i in range(len(y)):
        w = y[max(0, i - h): i + h + 1]
        w = w[~np.isnan(w)]
        out[i] = np.median(w) if len(w) else np.nan
    return out


def rate(t, db, a, e):
    m = (t >= a) & (t <= e)
    if m.sum() < 4:
        return float("nan")
    return float(np.polyfit(t[m], db[m], 1)[0])


def soll_hz(pitch):
    return 440.0 * 2 ** ((pitch - 69) / 12)


def kurve(x, n, ende_max=None):
    a = n["onset"]
    e = n["offset"] if n["offset"] > a else a + 2
    if ende_max is not None:
        e = min(e, ende_max)
    s = x[int(a * FS): int(e * FS)]
    f0s = soll_hz(n["pitch"])
    f = yin(s, fmin=0.6 * f0s, fmax=1.7 * f0s)
    t = (np.arange(len(f)) * SCHRITT + N_FENSTER / 2) / FS
    c = 1200 * np.log2(f / soll_hz(n["pitch"]))
    db, tdb = rms_db(s)
    return t, glatt(c), f, db, tdb


def normal_bezug(ds, g, pitch):
    """Rate (dB/s) des normalen Tons gleicher Hoehe auf der G-Saite, als Funktion: rate(a, e)."""
    x = lies_wav(ds / "audio" / f"{g}_G_fret_0-20.wav")
    nn = lies_noten(ds / "annotation" / f"{g}_G_fret_0-20.xml")
    kand = [i for i, n in enumerate(nn) if n["pitch"] == pitch]
    if not kand:
        return None
    i = kand[0]
    n = nn[i]
    e = nn[i + 1]["onset"] if i + 1 < len(nn) else len(x) / FS
    s = x[int(n["onset"] * FS): int((e - 0.02) * FS)]
    db, t = rms_db(s)
    return dict(db=db, t=t, spitze=float(db[:8].max()), fret=n["fret"])


def bend_auswerten(t, c, db, tdb, bezug):
    anfang = np.nanmedian(c[(t > 0.02) & (t < 0.06)])
    gueltig = ~np.isnan(c)
    if gueltig.sum() < 10:
        return None
    hub = np.nanpercentile(c, 95) - anfang
    if hub < 30:
        return dict(hub=round(float(hub), 1), kein_bend=True)
    rel = (c - anfang) / hub
    def erst(bed, ab=0):
        k = np.where(bed[ab:])[0]
        return ab + k[0] if len(k) else None
    k10 = erst(rel >= 0.1)
    k90 = erst(rel >= 0.9, k10 or 0)
    kmax = int(np.nanargmax(np.where(gueltig, rel, -9)))
    krueck = erst(rel < 0.5, max(kmax, k90 or 0))
    r = dict(anfang_cent=round(float(anfang), 1), hub_cent=round(float(hub), 1),
             t10_ms=round(t[k10] * 1000) if k10 is not None else None,
             t90_ms=round(t[k90] * 1000) if k90 is not None else None)
    if k10 is not None and k90 is not None:
        r["anstieg_ms"] = round((t[k90] - t[k10]) * 1000)
        form = []
        for q in (0.25, 0.5, 0.75):
            kq = erst(rel >= q, k10)
            form.append(round((t[kq] - t[k10]) / max(t[k90] - t[k10], 1e-9), 2) if kq is not None else None)
        r["form_25_50_75"] = form
        # Ueberschwingen: Maximum gegen Haltewert (Median 90-%-Punkt .. +300 ms)
        halt = np.nanmedian(c[(t >= t[k90]) & (t <= t[k90] + 0.3)])
        r["halt_cent"] = round(float(halt - anfang), 1)
        r["ueberschwinger_cent"] = round(float(np.nanmax(c[k10:k90 + 40]) - halt), 1)
        r["streuung_halt_cent"] = round(float(np.nanstd(c[(t >= t[k90]) & (t <= t[k90] + 0.3)])), 1)
        ende_halt = t[krueck] if krueck is not None else t[-1]
        r["rueck_ms"] = round(t[krueck] * 1000) if krueck is not None else None
        r["rate_vor_dbs"] = round(rate(tdb, db, 0.03, t[k10]), 1)
        r["rate_halt_dbs"] = round(rate(tdb, db, t[k90], ende_halt), 1)
        # Pegelsprung beim Bend: RMS 30 ms vor 10 % gegen 30 ms nach 90 %, minus Normalton ueber dieselbe Spanne
        vor = np.mean(db[(tdb >= t[k10] - 0.03) & (tdb < t[k10])])
        nach = np.mean(db[(tdb > t[k90]) & (tdb <= t[k90] + 0.03)])
        r["pegel_diff_db"] = round(float(nach - vor), 1)
        if bezug is not None:
            bv = np.mean(bezug["db"][(bezug["t"] >= t[k10] - 0.03) & (bezug["t"] < t[k10])])
            bn = np.mean(bezug["db"][(bezug["t"] > t[k90]) & (bezug["t"] <= t[k90] + 0.03)])
            r["normal_diff_db"] = round(float(bn - bv), 1)
            rn = rate(bezug["t"], bezug["db"], 0.1, 0.6)
            r["normal_rate_0_1_0_6_dbs"] = round(rn, 1)
            r["mehrverlust_db"] = round(float((nach - vor) - rn * (t[k90] - t[k10] + 0.03)), 1)
    return r


def slide_auswerten(t, c, f, db, tdb, bezug):
    g = ~np.isnan(c)
    if g.sum() < 10:
        return None
    idx = np.where(g)[0]
    n = len(idx)
    anf = (t >= 0.02) & (t <= 0.07) & g
    start = np.median(c[anf]) if anf.sum() >= 3 else np.median(c[idx[:3]])
    ende = np.median(c[idx[-max(3, int(0.25 * n)):]])
    weg = ende - start
    if abs(weg) < 50:
        return dict(weg_cent=round(float(weg), 1), kein_slide=True)
    sg = np.sign(weg)
    kb = None
    for k in idx:
        if sg * (c[k] - start) > 25:
            kb = k
            break
    ke = None
    if kb is not None:
        for k in idx[idx > kb]:
            if abs(c[k] - ende) < 25:
                ke = k
                break
    r = dict(start_cent=round(float(start), 1), weg_cent=round(float(weg), 1))
    if kb is None or ke is None:
        return r
    # Beginn genauer: letzter Rahmen vor kb mit Abweichung < 10 Cent
    r["beginn_ms"] = round(t[kb] * 1000)
    r["gleit_ms"] = round((t[ke] - t[kb]) * 1000)
    r["cent_pro_s"] = round(float(abs(weg) / max(t[ke] - t[kb], 1e-3)))
    seg = (np.arange(len(c)) >= kb) & (np.arange(len(c)) <= ke) & g
    cs, ts, fs_ = c[seg], t[seg], f[seg]
    if len(cs) >= 4:
        def r2(y):
            p = np.polyfit(ts, y, 1)
            res = y - np.polyval(p, ts)
            return 1 - np.sum(res ** 2) / max(np.sum((y - y.mean()) ** 2), 1e-12)
        r["r2_cent"] = round(float(r2(cs)), 3)
        r["r2_hz"] = round(float(r2(fs_)), 3)
        abst = np.abs(((cs - start) / 100) - np.round((cs - start) / 100)) * 100
        r["stufig_anteil"] = round(float(np.mean(abst <= 20)), 2)
        r["rahmen"] = int(len(cs))
    vor = np.mean(db[(tdb >= t[kb] - 0.03) & (tdb < t[kb])])
    nach = np.mean(db[(tdb > t[ke]) & (tdb <= t[ke] + 0.03)])
    r["pegel_diff_db"] = round(float(nach - vor), 1)
    if bezug is not None:
        rn = rate(bezug["t"], bezug["db"], 0.1, 0.6)
        r["normal_rate_dbs"] = round(rn, 1)
        r["mehrverlust_db"] = round(float((nach - vor) - rn * (t[ke] - t[kb] + 0.03)), 1)
    return r


def vibrato_auswerten(t, c):
    g = ~np.isnan(c)
    if g.sum() < 40:
        return None
    m = (t > 0.15) & g
    y = c[m] - np.nanmean(c[m])
    tt = t[m]
    if len(y) < 40:
        return None
    # Rate aus Nulldurchgaengen, Tiefe als halbe Spanne 5..95 %
    nd = np.where(np.diff(np.sign(y)) != 0)[0]
    dauer = tt[-1] - tt[0]
    rate_hz = len(nd) / 2 / dauer if dauer > 0 else float("nan")
    tiefe = (np.percentile(y, 95) - np.percentile(y, 5)) / 2
    return dict(rate_hz=round(float(rate_hz), 2), tiefe_cent=round(float(tiefe), 1),
                mittel_cent=round(float(np.nanmean(c[m])), 1))


def main():
    ds, aus = Path(sys.argv[1]), Path(sys.argv[2])
    aus.mkdir(parents=True, exist_ok=True)
    zeilen = []
    kurven = ["datei\tereignis\tt_ms\tcent\tdb"]
    for g in ("AR", "FS", "LP"):
        bezug60 = normal_bezug(ds, g, 60)
        print(f"=== {g}: Normalton G-Saite pitch 60 Bund {bezug60['fret']}, Spitze {bezug60['spitze']:.1f} dB, "
              f"Rate 0,05..1,5 s {rate(bezug60['t'], bezug60['db'], 0.05, 1.5):.1f} dB/s")
        for art in ("bending", "slide"):
            name = f"{g}_G_V_{art}"
            x = lies_wav(ds / "audio" / f"{name}.wav")
            nn = lies_noten(ds / "annotation" / f"{name}.xml")
            for j, n in enumerate(nn):
                t, c, f, db, tdb = kurve(x, n)
                if art == "bending":
                    r = bend_auswerten(t, c, db, tdb, bezug60)
                else:
                    r = slide_auswerten(t, c, f, db, tdb, bezug60)
                spitze = float(db[:8].max())
                r = dict(r or {}, spitze_gegen_normal_db=round(spitze - bezug60["spitze"], 1),
                         annot_mr=n["mr"], annot_mf=n["mf"], dauer_s=round(n["offset"] - n["onset"], 2))
                print(f"  {name} #{j}", r)
                zeilen.append((name, j, art, r))
                for k in range(0, len(t), 4):
                    kd = min(int(t[k] * 100), len(db) - 1)
                    kurven.append(f"{name}\t{j}\t{round(t[k]*1000)}\t{'' if np.isnan(c[k]) else round(float(c[k]),1)}\t{round(float(db[kd]),1)}")
        # Vibrato
        name = f"{g}_V_vibrato_slow-fast"
        x = lies_wav(ds / "audio" / f"{name}.wav")
        nn = lies_noten(ds / "annotation" / f"{name}.xml")
        for j, n in enumerate(nn):
            t, c, f, db, tdb = kurve(x, n)
            r = vibrato_auswerten(t, c)
            r = dict(r or {}, pitch=n["pitch"], annot_mf=n["mf"], annot_mr=n["mr"], dauer_s=round(n["offset"] - n["onset"], 2))
            print(f"  {name} #{j}", r)
            zeilen.append((name, j, "vibrato", r))
    # Licks: Bend- und Slide-Ereignisse
    for p in sorted((ds / "audio").glob("*_Lick*.wav")):
        nn = lies_noten(ds / "annotation" / f"{p.stem}.xml")
        ziel = [(i, n) for i, n in enumerate(nn) if n["expr"] in ("BE", "SL")]
        if not ziel:
            continue
        x = lies_wav(p)
        for i, n in ziel:
            if n["string"] < 1:
                continue
            ende = nn[i + 1]["onset"] if i + 1 < len(nn) else None
            nxt = nn[i + 1] if i + 1 < len(nn) else None
            t, c, f, db, tdb = kurve(x, n)
            if n["expr"] == "BE":
                r = bend_auswerten(t, c, db, tdb, None)
                art = "lick_bend"
            else:
                r = slide_auswerten(t, c, f, db, tdb, None)
                art = "lick_slide"
            r = dict(r or {}, pitch=n["pitch"], string=n["string"], fret=n["fret"], exc=n["exc"],
                     dauer_ms=round((n["offset"] - n["onset"]) * 1000), annot_mr=n["mr"],
                     naechste=(nxt["pitch"], nxt["string"], round((nxt["onset"] - n["onset"]) * 1000)) if nxt else None)
            print(f"  {p.stem} #{i} {art}", r)
            zeilen.append((p.stem, i, art, r))
            for k in range(0, len(t), 2):
                kd = min(int(t[k] * 100), len(db) - 1)
                kurven.append(f"{p.stem}\t{i}\t{round(t[k]*1000)}\t{'' if np.isnan(c[k]) else round(float(c[k]),1)}\t{round(float(db[kd]),1)}")
    with open(aus / "bend_slide_roh.tsv", "w", encoding="utf-8") as fh:
        for name, j, art, r in zeilen:
            fh.write(f"{name}\t{j}\t{art}\t{r}\n")
    (aus / "bend_slide_kurven.tsv").write_text("\n".join(kurven) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()

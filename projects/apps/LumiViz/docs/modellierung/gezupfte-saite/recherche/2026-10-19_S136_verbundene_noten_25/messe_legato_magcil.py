"""Hammer-on / Pull-off in den Uebungen des Datensatzes Mitsou et al. 2023 (Zenodo 10.5281/zenodo.10075352).

    python -I messe_legato_magcil.py <wav> <hammer|pull> [<wav> <hammer|pull> ...]

Die Uebung (Blatt Hammer-On_Exercise_1 / Pull-Off_Exercise_1): Achtelpaare auf einer Saite,
angeschlagen ein Ton, gebunden der Nachbarton (Hammer: aufwaerts, Pull-off: abwaerts).
Methode
- Mono (Mittel der Kanaele), 48 kHz. Grundton per YIN (Fenster 1024, Schritt 240 = 5 ms,
  70..1300 Hz, Schwelle 0,15). Halbton-Raster: Mediane Tonhoehe in MIDI, gerundet.
- Tonwechsel: Rahmen, ab dem die gerundete Tonhoehe fuer >= 30 ms (6 Rahmen) einen neuen Wert
  1..3 Halbtoene vom vorigen haelt. Richtung = gebunden (Hammer: hoch, Pull: runter); die
  Gegenrichtung ist der neue Anschlag (Re-Pluck).
- Pegel: RMS ueber 5 ms in dB. Je gebundenem Wechsel: 'vor' = Mittel 10..2 ms vor dem Wechsel,
  'spitze' = Max 0..40 ms nach dem Wechsel; der vorige Anschlag: 'anschlag' = Max 0..40 ms nach
  dem vorigen Wechsel in Gegenrichtung (bzw. Signalbeginn).
- Verzerrungsprobe: Spitzenfaktor (Peak/RMS) des ganzen Signals und Abklingrate des angeschlagenen
  Tons zwischen +40 ms und dem gebundenen Wechsel (dB/s).
"""
import sys
import wave

import numpy as np

FS = 48000
N = 1024
H = 240


def lies(p):
    with wave.open(p) as w:
        fs, ch, sw, n = w.getframerate(), w.getnchannels(), w.getsampwidth(), w.getnframes()
        roh = w.readframes(n)
    assert fs == FS and sw == 2, (fs, sw)
    x = np.frombuffer(roh, dtype=np.int16).astype(np.float64) / 32768
    if ch > 1:
        x = x.reshape(-1, ch).mean(axis=1)
    return x


def yin(x, fmin=70.0, fmax=1300.0, schwelle=0.15):
    tmin, tmax = int(FS / fmax), int(FS / fmin)
    out = []
    for st in range(0, len(x) - N - tmax, H):
        r = x[st:st + N + tmax]
        x0 = r[:N]
        if np.sum(x0 ** 2) < 1e-8:
            out.append(np.nan)
            continue
        # Differenzfunktion ueber FFT-Korrelation
        e0 = np.sum(x0 ** 2)
        kum = np.concatenate([[0.0], np.cumsum(r ** 2)])
        etau = kum[np.arange(tmax + 1) + N] - kum[np.arange(tmax + 1)]
        L = 1 << int(np.ceil(np.log2(N + tmax + N)))
        kor = np.fft.irfft(np.fft.rfft(r, L) * np.conj(np.fft.rfft(x0, L)), L)[: tmax + 1]
        d = e0 + etau - 2 * kor
        d[0] = 0
        cm = np.ones_like(d)
        cs = np.cumsum(d[1:])
        cm[1:] = d[1:] * np.arange(1, tmax + 1) / np.maximum(cs, 1e-20)
        k = np.where(cm[tmin:] < schwelle)[0]
        if len(k) == 0:
            out.append(np.nan)
            continue
        t = k[0] + tmin
        while t + 1 <= tmax and cm[t + 1] < cm[t]:
            t += 1
        out.append(FS / t)
    return np.array(out)


def rms_db(x, r=240):
    m = len(x) // r
    return 20 * np.log10(np.sqrt(np.mean(x[: m * r].reshape(m, r) ** 2, axis=1)) + 1e-12)


def main():
    args = sys.argv[1:]
    alle = {"hammer": [], "pull": []}
    for i in range(0, len(args), 2):
        pfad, art = args[i], args[i + 1]
        x = lies(pfad)
        f = yin(x)
        db = rms_db(x)
        crest = 20 * np.log10(np.max(np.abs(x)) / np.sqrt(np.mean(x ** 2)))
        clip = np.mean(np.abs(x) > 0.99 * np.max(np.abs(x)))
        midi = np.round(69 + 12 * np.log2(f / 440.0))
        # stabile Tonhoehen-Segmente
        wechsel = []
        akt = None
        k = 0
        while k < len(midi) - 6:
            m = midi[k]
            if np.isnan(m):
                k += 1
                continue
            if akt is None:
                akt = m
            elif m != akt and np.all(midi[k:k + 6] == m) and 1 <= abs(m - akt) <= 3:
                wechsel.append((k, akt, m))
                akt = m
                k += 6
                continue
            elif m != akt and np.all(midi[k:k + 6] == m):
                akt = m
            k += 1
        gebunden_hoch = art == "hammer"
        letzter_anschlag = 0
        erg = []
        for (k, a, b) in wechsel:
            hoch = b > a
            if hoch != gebunden_hoch:
                letzter_anschlag = k
                continue
            vor = np.mean(db[max(0, k - 2):k])  # 10..0 ms vorher (5-ms-Rahmen; YIN-Rahmenmitte beachtet)
            spitze = np.max(db[k:k + 8])
            anschlag = np.max(db[letzter_anschlag:letzter_anschlag + 8])
            dauer = (k - letzter_anschlag) * H / FS
            rate = (vor - anschlag) / dauer if dauer > 0.06 else np.nan
            erg.append((k * H / FS, int(a), int(b), round(spitze - vor, 1), round(spitze - anschlag, 1), round(rate, 1)))
        print(f"== {pfad.split('/')[-1]} ({art}): Spitzenfaktor {crest:.1f} dB, Anteil >99 % Spitze {clip*100:.3f} %, Wechsel {len(wechsel)}, gebunden {len(erg)}")
        for e in erg[:12]:
            print("   t %.2f s  %d->%d  Sprung %+.1f dB  gegen Anschlag %+.1f dB  Abklingen davor %s dB/s" % e)
        alle[art] += erg
    for art, erg in alle.items():
        if not erg:
            continue
        e = np.array([(s, g, r) for _, _, _, s, g, r in erg], dtype=float)
        print(f"## {art}: n={len(e)}, Sprung median {np.nanmedian(e[:,0]):+.1f} dB (P10 {np.nanpercentile(e[:,0],10):+.1f}, P90 {np.nanpercentile(e[:,0],90):+.1f}); "
              f"gegen Anschlag median {np.nanmedian(e[:,1]):+.1f} dB (P10 {np.nanpercentile(e[:,1],10):+.1f}, P90 {np.nanpercentile(e[:,1],90):+.1f}); "
              f"Abklingen davor median {np.nanmedian(e[:,2]):.1f} dB/s")


if __name__ == "__main__":
    main()

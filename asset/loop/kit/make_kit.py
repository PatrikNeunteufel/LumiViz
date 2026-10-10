#!/usr/bin/env python3
"""Rechnet das mitgelieferte Drum-Kit des Loops (docs/audio/Loop_Konzept.md, L5 a).

Aufruf:  python make_kit.py [zielordner]      (Vorgabe: der Ordner dieses Skripts)

Schreibt je Klang eine WAV-Datei (44100 Hz, 16 Bit, Mono) und gibt Laenge und
SHA-256 aus. Kein Zufall von aussen: das Rauschen kommt aus einem eigenen
Zaehlwerk mit fester Saat, jeder Lauf ergibt dieselben Dateien. Die Klaenge
sind gerechnet, nicht aufgenommen — es gibt keine fremden Rechte daran.

Der Name der Datei ist der Name im Loop: `kick.wav` ist `kit:kick`.
"""

import hashlib
import math
import sys
import wave
from pathlib import Path

import numpy as np

RATE = 44100
SPITZE = 0.9          # groesster Betrag nach dem Angleichen
AUSBLENDE_S = 0.003   # die letzten Millisekunden laufen auf null


def zeit(sekunden):
    return np.arange(int(round(sekunden * RATE))) / RATE


def rauschen(anzahl, saat):
    """Gleichverteiltes Rauschen in [-1, 1) aus einem festen Zaehlwerk (LCG)."""
    werte = np.empty(anzahl)
    x = saat & 0xFFFFFFFF
    for i in range(anzahl):
        x = (x * 1664525 + 1013904223) & 0xFFFFFFFF
        werte[i] = (x >> 8) / 8388608.0 - 1.0
    return werte


def biquad(x, b0, b1, b2, a1, a2):
    y = np.empty_like(x)
    x1 = x2 = y1 = y2 = 0.0
    for i, wert in enumerate(x):
        aus = b0 * wert + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2
        x2, x1 = x1, wert
        y2, y1 = y1, aus
        y[i] = aus
    return y


def hochpass(x, frequenz, guete=0.7071):
    w = 2.0 * math.pi * frequenz / RATE
    alpha = math.sin(w) / (2.0 * guete)
    c = math.cos(w)
    a0 = 1.0 + alpha
    return biquad(x, (1 + c) / 2 / a0, -(1 + c) / a0, (1 + c) / 2 / a0, -2 * c / a0, (1 - alpha) / a0)


def bandpass(x, frequenz, guete):
    w = 2.0 * math.pi * frequenz / RATE
    alpha = math.sin(w) / (2.0 * guete)
    c = math.cos(w)
    a0 = 1.0 + alpha
    return biquad(x, alpha / a0, 0.0, -alpha / a0, -2 * c / a0, (1 - alpha) / a0)


def gleitton(t, von_hz, bis_hz, tau):
    """Sinus, dessen Frequenz von `von_hz` auf `bis_hz` faellt."""
    frequenz = bis_hz + (von_hz - bis_hz) * np.exp(-t / tau)
    phase = 2.0 * math.pi * np.cumsum(frequenz) / RATE
    return np.sin(phase)


def kick():
    t = zeit(0.50)
    ton = gleitton(t, 160.0, 45.0, 0.035) * np.exp(-t / 0.16)
    klick = rauschen(len(t), 11) * np.exp(-t / 0.0012) * 0.25
    return np.tanh(1.5 * (ton + klick))


def snare():
    t = zeit(0.30)
    koerper = 0.60 * np.sin(2 * math.pi * 185.0 * t) * np.exp(-t / 0.060)
    koerper += 0.35 * np.sin(2 * math.pi * 330.0 * t) * np.exp(-t / 0.035)
    teppich = hochpass(rauschen(len(t), 22), 1800.0) * np.exp(-t / 0.085) * 0.9
    return koerper + teppich


def becken(sekunden, tau, saat):
    """Hi-Hat: sechs Rechteck-Toene in unharmonischem Abstand und Rauschen, nur die Hoehen."""
    t = zeit(sekunden)
    metall = np.zeros(len(t))
    for frequenz in (205.3, 304.4, 369.6, 522.7, 540.0, 800.0):
        metall += np.sign(np.sin(2 * math.pi * frequenz * 8.0 * t))
    klang = hochpass(metall / 6.0 + 0.6 * rauschen(len(t), saat), 7000.0)
    return klang * np.exp(-t / tau)


def hat_closed():
    return becken(0.09, 0.022, 33)


def hat_open():
    return becken(0.50, 0.150, 34)


def clap():
    t = zeit(0.35)
    klang = bandpass(hochpass(rauschen(len(t), 44), 600.0), 1200.0, 1.2)
    huelle = np.zeros(len(t))
    for beginn in (0.0, 0.010, 0.020):  # drei kurze Schlaege, dann der Ausklang
        nach = t - beginn
        huelle = np.maximum(huelle, np.where(nach >= 0, np.exp(-np.maximum(nach, 0) / 0.004), 0.0))
    nach = t - 0.030
    huelle = np.maximum(huelle, np.where(nach >= 0, np.exp(-np.maximum(nach, 0) / 0.070), 0.0))
    return klang * huelle


def tom():
    t = zeit(0.45)
    ton = gleitton(t, 200.0, 110.0, 0.040) * np.exp(-t / 0.14)
    klick = rauschen(len(t), 55) * np.exp(-t / 0.0015) * 0.15
    return ton + klick


KLAENGE = {
    "kick": kick,
    "snare": snare,
    "hat_closed": hat_closed,
    "hat_open": hat_open,
    "clap": clap,
    "tom": tom,
}


def fertig(klang):
    """Auf die Spitze angleichen, das Ende ausblenden, auf 16 Bit runden."""
    klang = klang * (SPITZE / np.max(np.abs(klang)))
    n = int(AUSBLENDE_S * RATE)
    klang[-n:] *= np.linspace(1.0, 0.0, n)
    return np.round(klang * 32767.0).astype("<i2")


def main():
    ziel = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent
    ziel.mkdir(parents=True, exist_ok=True)
    for name, rechne in KLAENGE.items():
        daten = fertig(rechne()).tobytes()
        with wave.open(str(ziel / f"{name}.wav"), "wb") as datei:
            datei.setnchannels(1)
            datei.setsampwidth(2)
            datei.setframerate(RATE)
            datei.writeframes(daten)
        frames = len(daten) // 2
        summe = hashlib.sha256((ziel / f"{name}.wav").read_bytes()).hexdigest()
        print(f"{name:11s} {frames:6d} Abtastwerte  {1000.0 * frames / RATE:6.1f} ms  sha256 {summe}")


if __name__ == "__main__":
    main()

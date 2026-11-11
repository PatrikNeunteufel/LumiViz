"""HS-4 a: Filzgesetze an der endlichen idealen Saite (Wellenleiter, drei Saiten, x_H 0,12), Werte der Saite aus den
Kurven der Vorgabe (RT-0425). Je Gesetz: Helligkeit je Staerke (am Kraftpuls bei n f1, Steigung je dB Teilton 1 ueber
Stufen -30..0), Umfang ppp 0,45 -> ff 5 m/s an den Teiltoenen 1-8 (|F(n f1)| sin(n pi xh)), Kontaktzeit 0,63 / 1,26 /
5 m/s, Unterschied T2-T1 von 0,63 zu 5 m/s (Russell C6: +13,1 dB).

Gesetze (Q0, p, tau0, gamma nach Kartofelev & Stulov 2014 = S2):
  S2        F = F0 [phi - (g/tau0) I]                         (wie im Kern)
  R(g,T)    S2 plus eine zweite Relaxation: F = S2 - Q0 * gR/TR * I_R ... als Standard-Linear-Solid in phi:
            F = Q0 [phi + alpha dphi/dt] + K2 [phi - (1/TR) I_TR]   (K2 = Q0 * k2: steifer bei kurzem Kontakt)
  V(q)      F = Q0 [phi + alpha (vref/|du|)^q dphi/dt]       (Zaehigkeit faellt mit der Geschwindigkeit)
"""
import math
import sys
import numpy as np

FS = 48000 * 32
XH = 0.12


def loglin(punkte, t):
    for (a, ya), (b, yb) in zip(punkte, punkte[1:]):
        if a <= t <= b:
            return math.exp(math.log(ya) + (t - a) / (b - a) * (math.log(yb) - math.log(ya)))
    return punkte[-1][1]


LAENGE = [(24, 2.007), (31, 1.842), (37, 1.548), (39, 1.442), (40, 1.378), (41, 1.837), (42, 1.757), (45, 1.482),
          (52, 1.013), (74, 0.308), (107, 0.052), (108, 0.052)]
MASSE = [(24, 99.41), (28, 66.53), (33, 31.57), (35, 23.63), (39, 15.32), (40, 14.30), (41, 7.729), (54, 6.733),
         (70, 5.459), (82, 4.972), (94, 4.392), (107, 3.404), (108, 3.404)]


class Taste:
    def __init__(self, midi):
        self.midi = midi
        N = midi - 20
        self.f1 = 440 * 2 ** ((midi - 69) / 12)
        self.L = loglin(LAENGE, midi)
        self.mu = loglin(MASSE, midi) * 1e-3
        self.T = 4 * self.L ** 2 * self.f1 ** 2 * self.mu
        self.Z = math.sqrt(self.T * self.mu)
        self.c = math.sqrt(self.T / self.mu)
        self.p = 3.7 + 0.015 * N
        self.tau = (2.72 - 0.02 * N + 0.00009 * N * N) * 1e-6
        self.m = (11.074 - 0.074 * N + 1e-4 * N * N) * 1e-3
        self.g = 0.9894 + 0.000088 * N
        self.Q0 = 15500 * math.exp(0.059 * N) * (1 - self.g)
        self.alpha = self.g * self.tau / (1 - self.g)


def puls(t, v0, gesetz):
    art = gesetz[0]
    dt = 1.0 / FS
    M = int(round(t.L / (t.c * dt)))
    h = int(round(XH * M))
    r = np.zeros(M + 1)
    l = np.zeros(M + 1)
    y = w = 0.0
    v = v0
    vs = 0.0
    F0 = t.Q0 / (1 - t.g)
    kern = math.exp(-dt / t.tau)
    I = 0.0
    I2 = 0.0
    phi_alt = 0.0
    F = []
    erster = None
    letzter = 0
    if art == "R":
        _, k2, TR = gesetz
        kern2 = math.exp(-dt / TR)
    for i in range(int(0.012 / dt)):
        d = (y - w) * 1000.0
        phi = d ** t.p if d > 0 else 0.0
        if art in ("S2", "R"):
            I = I * kern + phi * t.tau * (1 - kern)
            f = F0 * (phi - t.g * I / t.tau)
            if art == "R":
                I2 = I2 * kern2 + phi * TR * (1 - kern2)
                f += t.Q0 * k2 * (phi - I2 / TR)
        elif art == "V":
            _, q, vref = gesetz
            du = abs(v - vs)
            a = t.alpha * (vref / max(du, 1e-3)) ** q
            f = t.Q0 * (phi + a * (phi - phi_alt) / dt) if d > 0 else 0.0
        f = max(f, 0.0)
        phi_alt = phi
        if f > 0:
            if erster is None:
                erster = i
            letzter = i
        F.append(f)
        v -= f / t.m * dt
        y += v * dt
        r[1:] = r[:-1].copy()
        l[:-1] = l[1:].copy()
        r[0] = -l[0]
        l[M] = -r[M]
        vs = r[h] + l[h] + (f / 3) / (2 * t.Z)
        r[h], l[h] = vs - l[h], vs - r[h]
        w += vs * dt
        if i > 100 and f == 0.0 and v < 0 and (y - w) < 0:
            break
    kontakt = (letzter - (erster or 0)) * dt
    return np.array(F), dt, kontakt


def spektrum(t, F, dt):
    zeit = np.arange(len(F)) * dt
    return np.array([abs(np.sum(F * np.exp(-2j * math.pi * k * t.f1 * zeit))) * dt for k in range(1, 9)])


def auswerten(t, gesetz):
    xs, ys = [], [[] for _ in range(7)]
    for stufe in range(-30, 1, 2):
        F, dt, _ = puls(t, 5.0 * 10 ** (stufe / 20), gesetz)
        s = spektrum(t, F, dt)
        g1 = 20 * math.log10(s[0])
        xs.append(g1)
        for j in range(7):
            ys[j].append(20 * math.log10(s[j + 1]) - g1)
    n = min(7, int(0.45 * 48000 / t.f1) - 1)
    hell = [float(np.polyfit(xs, yy, 1)[0]) for yy in ys[:n]]
    gew = np.array([math.sin(k * math.pi * XH) for k in range(1, 9)])
    res = {}
    for v0 in (0.45, 0.63, 1.26, 5.0):
        F, dt, k = puls(t, v0, gesetz)
        s = spektrum(t, F, dt)
        res[v0] = (10 * math.log10(np.sum((gew * s)[: n + 1] ** 2)), k, 20 * math.log10(s[1] / s[0]))
    umfang = res[5.0][0] - res[0.45][0]
    kontakt = [res[v][1] * 1e3 for v in (0.63, 1.26, 5.0)]
    t21 = res[5.0][2] - res[0.63][2]
    return hell, umfang, kontakt, t21


SAL_HELL = {60: [0.20, 0.25, 0.69, 0.98, 0.97, 1.65, 1.92], 72: [0.26, 0.92, 1.15, 1.52, 1.53, 1.45, 1.78],
            84: [1.17, 0.73, 2.18, 1.48, 1.53, 1.81, 2.55], 96: [0.38, 0.80, 0.57]}
SAL_UMFANG = {60: 17.6, 72: 21.9, 84: 23.7, 96: 29.7, 108: 38.7}
ASKENFELT = {84: (2.5, 1.1, 0.85), 96: (0.9, 0.65, 0.55), 108: (1.2, 0.55, 0.45)}


def bericht(midi, gesetze):
    t = Taste(midi)
    print(f"Taste {midi}: L {t.L:.3f} m, mu {t.mu*1e3:.2f} g/m, alpha {t.alpha*1e6:.0f} us, Q0 {t.Q0:.0f}, p {t.p:.2f}")
    print(f"  Salamander hell {SAL_HELL.get(midi)}  Umfang {SAL_UMFANG.get(midi)}  Askenfelt {ASKENFELT.get(midi)}"
          + ("  Russell T2-T1 +13,1" if midi == 84 else ""))
    for name, g in gesetze:
        hell, umfang, kontakt, t21 = auswerten(t, g)
        sal = SAL_HELL.get(midi)
        rms = math.sqrt(sum((a - b) ** 2 for a, b in zip(hell, sal)) / len(hell)) if sal else float("nan")
        print(f"  {name:28s} hell {' '.join(f'{x:5.2f}' for x in hell)} (RMS {rms:4.2f})  Umfang {umfang:5.1f}  "
              f"Kontakt {' / '.join(f'{k:4.2f}' for k in kontakt)}  T2-T1 {t21:+5.1f}")


if __name__ == "__main__":
    tasten = [int(a) for a in sys.argv[1:]] or [84]
    gesetze = [("S2", ("S2",))]
    for k2 in (1.0, 3.0, 10.0):
        for TR in (1e-4, 3e-4, 1e-3):
            gesetze.append((f"R k2 {k2:g} TR {TR*1e3:g} ms", ("R", k2, TR)))
    for q in (0.5, 1.0):
        for vref in (0.5, 2.0):
            gesetze.append((f"V q {q:g} vref {vref:g}", ("V", q, vref)))
    for midi in tasten:
        bericht(midi, gesetze)

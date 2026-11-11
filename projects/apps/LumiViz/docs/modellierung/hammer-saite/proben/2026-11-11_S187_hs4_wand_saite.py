"""HS-4: Helligkeit je Staerke des Kraftpulses an Wand und unendlicher Saite, C6 (N = 64).
Gleiche Groesse wie an Salamander: Steigung von (Teilton n - Teilton 1) je dB Teilton 1, hier am Kraftspektrum
bei n * f1. Filz wie im Kern: Rekursion mit phi je Schritt konstant (HammerTon.cpp)."""
import math
import numpy as np

N = 64
f1 = 440 * 2 ** ((84 - 69) / 12)


def satz(name):
    p = 3.7 + 0.015 * N
    tau = (2.72 - 0.02 * N + 0.00009 * N * N) * 1e-6
    m = (11.074 - 0.074 * N + 1e-4 * N * N) * 1e-3
    if name == "S2":
        g = 0.9894 + 0.000088 * N
        q0 = 15500 * math.exp(0.059 * N) * (1 - g)
    else:
        g = 0.9894 - 8.8e-5 * N * N
        q0 = 183 * math.exp(0.045 * N)
    return p, g, tau, q0, m


def puls(name, v0, rs, dt=48000 * 32):
    p, g, tau, q0, m = satz(name)
    dt = 1.0 / dt
    f0 = q0 / (1 - g)
    kern = math.exp(-dt / tau)
    y = v = 0.0
    w = 0.0
    v = v0
    I = 0.0
    F = []
    for i in range(int(0.01 / dt)):
        d = (y - w) * 1000.0
        phi = d ** p if d > 0 else 0.0
        I = I * kern + phi * tau * (1 - kern)
        f = max(0.0, f0 * (phi - g * I / tau))
        F.append(f)
        v -= f / m * dt
        y += v * dt
        if rs is not None:
            w += f / rs * dt
        if i > 100 and f == 0.0 and v < 0 and (y - w) < 0:
            break
    return np.array(F), dt


def relativ(F, dt):
    t = np.arange(len(F)) * dt
    a = [abs(np.sum(F * np.exp(-2j * math.pi * k * f1 * t))) * dt for k in range(1, 9)]
    db = [20 * math.log10(x + 1e-300) for x in a]
    return db[0], [d - db[0] for d in db[1:]]


def steigungen(name, rs):
    xs, ys = [], [[] for _ in range(7)]
    for stufe in range(-30, 1, 2):
        v0 = 5.0 * 10 ** (stufe / 20)
        F, dt = puls(name, v0, rs)
        g, r = relativ(F, dt)
        xs.append(g)
        for j in range(7):
            ys[j].append(r[j])
    return [float(np.polyfit(xs, y, 1)[0]) for y in ys]


def puls_endlich(name, v0, Z, L, c, xh, chor=3, fs=48000 * 32):
    """Ideale endliche Saite als Wellenleiter (feste Enden, ohne Modenschnitt); Hammer bei xh·L, Kraft je Saite F/chor."""
    p, g, tau, q0, m = satz(name)
    dt = 1.0 / fs
    M = int(round(L / (c * dt)))
    h = int(round(xh * M))
    r = np.zeros(M + 1)  # nach rechts laufende Geschwindigkeitswelle an jedem Punkt
    l = np.zeros(M + 1)
    f0 = q0 / (1 - g)
    kern = math.exp(-dt / tau)
    y = 0.0
    v = v0
    w = 0.0
    I = 0.0
    F = []
    for i in range(int(0.01 / dt)):
        d = (y - w) * 1000.0
        phi = d ** p if d > 0 else 0.0
        I = I * kern + phi * tau * (1 - kern)
        f = max(0.0, f0 * (phi - g * I / tau))
        F.append(f)
        v -= f / m * dt
        y += v * dt
        # Schritt der Saite: Wellen laufen eine Zelle, feste Enden kehren um
        r[1:] = r[:-1].copy()
        l[:-1] = l[1:].copy()
        r[0] = -l[0]
        l[M] = -r[M]
        vs = r[h] + l[h] + (f / chor) / (2 * Z)
        r_aus, l_aus = vs - l[h], vs - r[h]
        r[h], l[h] = r_aus, l_aus
        w += vs * dt
        if i > 100 and f == 0.0 and v < 0 and (y - w) < 0:
            break
    return np.array(F), dt


def steigungen_endlich(name, Z, L, c, xh):
    xs, ys = [], [[] for _ in range(7)]
    for stufe in range(-30, 1, 2):
        F, dt = puls_endlich(name, 5.0 * 10 ** (stufe / 20), Z, L, c, xh)
        gg, rr = relativ(F, dt)
        xs.append(gg)
        for j in range(7):
            ys[j].append(rr[j])
    return [float(np.polyfit(xs, yy, 1)[0]) for yy in ys]


# Wellenwiderstand der Saite an C6: mu ~ 5 g/m, T aus f1 und L (RT-0425 bei MIDI 84 ~0,12 m)
mu = 4.97e-3
L = 0.123
T = 4 * L * L * f1 * f1 * mu
Z = math.sqrt(T * mu)
for name in ("S2", "vorher"):
    print(name, "Wand          ", " ".join(f"{s:5.2f}" for s in steigungen(name, None)))
    print(name, "Saite unendl.3", " ".join(f"{s:5.2f}" for s in steigungen(name, 2 * Z * 3)))
c = math.sqrt(T / mu)
for name in ("S2", "vorher"):
    print(name, "Saite endlich  ", " ".join(f"{s:5.2f}" for s in steigungen_endlich(name, Z, L, c, 0.12)))
print("Salamander C6       ", "1.17  0.73  2.18  1.48  1.53  1.81  2.55")
print("Z", Z, "T", T)

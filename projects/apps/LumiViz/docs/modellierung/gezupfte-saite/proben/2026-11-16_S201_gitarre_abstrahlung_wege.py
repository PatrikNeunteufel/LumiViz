"""S201, 34k-6 Punkt (20) — woher kommt die Senke der Koerperdatei bei 780-880 Hz, und welcher Weg des Fits hebt sie?

Aufruf (vom Projekt-Root): python -X utf8 -B <dieses Skript> <qualified_selected_impulses.mat>

Die Koerperdatei traegt je Mode Formwerte am Steg (phi(x) = a + b x + c x^2) und je Hoerort ein komplexes Gewicht G.
`make_gitarre.py` Schritt 5 stellt G je Mikrofon so, dass die Summe mit einem Restterm je Schlagort (Konstante und
1/w^2) alle drei Schlagorte trifft; danach ersetzt es die Gewichte ab 1 kHz durch Betraege je Terzband mit gezogenem
Vorzeichen (KQ24 a) und laesst den Restterm weg - die Datei traegt ihn nicht. Verglichen werden an den 75 gefitteten
Moden der Datei (Pole und Formen fest):

  0  die Datei, wie sie ist (mit dem statistischen Teil)
  1  Fit wie Schritt 5, mit Restterm                       (was der Fit sieht)
  2  dieselben Gewichte ohne Restterm                      (was davon in eine Datei kaeme)
  3  Gewichte unter 1 kHz neu gestellt gegen das, was die Datei traegt: die Moden ab 1 kHz und der statistische Teil
     fest wie in der Datei, kein Restterm                  (Kandidat: der Fit sieht, was gespeichert wird)
  4  wie 3, dazu je Mode und Schlagort ein eigenes Gewicht (kein Produkt phi(x) G) - die Grenze dessen, was die Pole
     tragen; in das heutige Format passt das nicht

Zielgroessen je Weg, Halbtonbaender der Tasten 40-83 (82-988 Hz), drei Schlagorte:
  Fehler des Pegels in dB (Effektivwert, groesster Betrag, Ort) je Hoerort; der Gang Taste 83 gegen 81 am Schlag treble
  und an der ersten Saite (Leistungssumme beider Hoerorte) gegen die Messung am Schlag treble; und der groesste Schritt
  von Halbton zu Halbton an der ersten Saite, Tasten 64-83 (Leistungssumme).

Schreibt nichts.
"""
import importlib.util
import pathlib
import sys

import numpy as np

HIER = pathlib.Path(__file__).resolve().parent


def modul(datei):
    spec = importlib.util.spec_from_file_location(datei.replace("-", "_"), HIER / datei)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


mf = modul("2026-11-15_S199_mores_fit.py")
ps = modul("2026-11-15_S199_pruefstand_zupf.py")

SAITENABSTAND = 0.0116
ORTE = ("bass", "center", "treble")
X_SCHLAG = {"bass": -2 * SAITENABSTAND, "center": 0.0, "treble": 2 * SAITENABSTAND}
X_SAITEN = np.array([(k - 2.5) * SAITENABSTAND for k in range(6)])
MIK = (("left", "mbass"), ("right", "mtreble"))
GRENZE = 1000.0
TASTEN = list(range(40, 84))


def hz_von(taste):
    return 440.0 * 2 ** ((taste - 69) / 12)


def halbton(hz, y):
    """Pegel je Halbtonband der TASTEN; y (Frequenz) oder (Frequenz, k)."""
    return np.array([10 * np.log10(np.mean(np.abs(y[(hz >= hz_von(t) * 2 ** (-1 / 24)) & (hz < hz_von(t) * 2 ** (1 / 24))]) ** 2, axis=0))
                     for t in TASTEN])


kp = ps.lade_koerper("classical_guitar")
hz_alle, H = mf.frfs(mf.lade(sys.argv[1], 34))
sel = (hz_alle >= 60.0) & (hz_alle <= 5000.0)
hz = hz_alle[sel]
w = 2 * np.pi * hz
wn = 2 * np.pi * kp["f"]
B = (-w[:, None] ** 2) / (wn[None, :] ** 2 - w[:, None] ** 2 + 1j * kp["eta"][None, :] * wn[None, :] * w[:, None])
koeff = np.polyfit(X_SAITEN, kp["steg"].T, 2)


def phi_an(x):
    return koeff[0] * x * x + koeff[1] * x + koeff[2]


phi_h = np.array([phi_an(X_SCHLAG[o]) for o in ORTE])           # (3, Moden)
phi_1 = phi_an(float(X_SAITEN[5]))
J = int(np.sum(kp["f"] <= 5000.0))                              # die gefitteten Moden; dahinter der statistische Teil
tief = np.arange(len(kp["f"])) < J
unten = tief & (kp["f"] < GRENZE)
n = len(hz)

wege = {}
for o, (ort, mik) in enumerate(MIK):
    Y = np.concatenate([H[(s, mik)][sel] for s in ORTE])
    gew = 1.0 / np.maximum(np.abs(Y), 1e-9)
    mess = Y.reshape(3, n)
    G_datei = kp["G"][:, o]

    def summe(G, phis=phi_h):
        return np.array([B @ (p * G) for p in phis])

    # 0 die Datei
    wege.setdefault("0 Datei", []).append((summe(G_datei), B @ (phi_1 * G_datei)))

    # 1 / 2 Fit wie Schritt 5 an den 75 Moden mit Restterm
    A = np.concatenate([B[:, tief] * phi_h[k][None, tief] for k in range(3)])
    rest = np.zeros((3 * n, 6), dtype=complex)
    for k in range(3):
        rest[k * n:(k + 1) * n, 2 * k] = 1.0
        rest[k * n:(k + 1) * n, 2 * k + 1] = 1e4 / w ** 2
    M = np.concatenate([A, rest], axis=1)
    x = np.linalg.lstsq(M * gew[:, None], Y * gew, rcond=None)[0]
    G1 = np.zeros(len(kp["f"]), dtype=complex)
    G1[tief] = x[:J]
    mit_rest = (M @ x).reshape(3, n)
    wege.setdefault("1 Fit mit Rest", []).append((mit_rest, None))
    wege.setdefault("2 ohne Rest", []).append((summe(G1), B @ (phi_1 * G1)))

    # 3 unter der Grenze neu, der Rest der Datei fest
    fest = np.concatenate([B[:, ~unten] @ (phi_h[k][~unten] * G_datei[~unten]) for k in range(3)])
    A3 = np.concatenate([B[:, unten] * phi_h[k][None, unten] for k in range(3)])
    x3 = np.linalg.lstsq(A3 * gew[:, None], (Y - fest) * gew, rcond=None)[0]
    G3 = G_datei.copy()
    G3[unten] = x3
    wege.setdefault("3 neu gegen die Datei", []).append((summe(G3), B @ (phi_1 * G3)))

    # 4 je Schlagort ein eigenes Gewicht (Grenze der Pole); an der Saite 1 die Parabel durch die drei Schlagorte
    P = []
    for k in range(3):
        yk, gk = mess[k] - fest[k * n:(k + 1) * n], gew[k * n:(k + 1) * n]
        P.append(np.linalg.lstsq(B[:, unten] * gk[:, None], yk * gk, rcond=None)[0])
    P = np.array(P)                                             # (3, Moden unten): Produkt phi(x_h) G
    frei = np.array([B[:, unten] @ P[k] + fest[k * n:(k + 1) * n] for k in range(3)])
    pk = np.polyfit(np.array(list(X_SCHLAG.values())), P.real, 2) + 1j * np.polyfit(np.array(list(X_SCHLAG.values())), P.imag, 2)
    x1 = float(X_SAITEN[5])
    P1 = pk[0] * x1 * x1 + pk[1] * x1 + pk[2]
    wege.setdefault("4 frei je Schlagort", []).append((frei, B[:, unten] @ P1 + B[:, ~unten] @ (phi_1[~unten] * G_datei[~unten])))
    wege.setdefault("Messung", []).append((mess, None))

i81, i83 = TASTEN.index(81), TASTEN.index(83)
mess_p = [halbton(hz, wege["Messung"][o][0].T) for o in range(2)]            # (Tasten, 3)
mess_sum = 10 * np.log10(10 ** (mess_p[0] / 10) + 10 ** (mess_p[1] / 10))
print(f"g34, {J} gefittete Moden, {int(unten.sum())} unter {GRENZE:.0f} Hz. Halbtonbaender der Tasten 40-83.")
print(f"Messung, Schlag treble: Gang 83 gegen 81 = {mess_sum[i83, 2] - mess_sum[i81, 2]:+.1f} dB (Leistungssumme), "
      f"left {mess_p[0][i83, 2] - mess_p[0][i81, 2]:+.1f}, right {mess_p[1][i83, 2] - mess_p[1][i81, 2]:+.1f}; "
      f"groesster Schritt Tasten 64-83: {np.abs(np.diff(mess_sum[24:, 2])).max():.1f} dB")
print()
print("Weg                    | Fehler left: rms  max (Taste, Schlag) | right: rms  max (Taste, Schlag) | Tasten 76-83 treble: left right | "
      "Gang 83-81 Schlag treble | Saite 1: Gang 83-81, groesster Schritt 64-83 (Tasten)")
for name, je in wege.items():
    if name == "Messung":
        continue
    zeile = f"{name:22s} |"
    summen = []
    for o in range(2):
        p = halbton(hz, je[o][0].T)
        d = p - mess_p[o]
        i, k = np.unravel_index(np.argmax(np.abs(d)), d.shape)
        zeile += f"  {np.sqrt(np.mean(d ** 2)):4.1f}  {d[i, k]:+5.1f} ({TASTEN[i]}, {ORTE[k]:6s}) |"
        summen.append(p)
    hoch = [np.sqrt(np.mean((summen[o] - mess_p[o])[36:, 2] ** 2)) for o in range(2)]
    zeile += f"  {hoch[0]:4.1f}  {hoch[1]:4.1f} |"
    s = 10 * np.log10(10 ** (summen[0] / 10) + 10 ** (summen[1] / 10))
    zeile += f"  {s[i83, 2] - s[i81, 2]:+5.1f} |"
    if je[0][1] is not None:
        s1 = 10 * np.log10(10 ** (halbton(hz, je[0][1]) / 10) + 10 ** (halbton(hz, je[1][1]) / 10))
        dd = np.abs(np.diff(s1[24:]))
        zeile += f"  {s1[i83] - s1[i81]:+5.1f}, {dd.max():4.1f} ({64 + int(np.argmax(dd))}/{65 + int(np.argmax(dd))})"
    print(zeile)

print("\nPegel an der ersten Saite (Leistungssumme, dB re 1 Pa/N), Tasten 76-83; Messung am Schlag treble daneben:")
print("Taste  Messung treble | " + " | ".join(n_ for n_ in wege if n_ not in ("Messung", "1 Fit mit Rest")))
for t in range(76, 84):
    i = TASTEN.index(t)
    zeile = f"{t:4d}     {mess_sum[i, 2]:6.1f}      |"
    for name, je in wege.items():
        if name in ("Messung", "1 Fit mit Rest"):
            continue
        s1 = 10 * np.log10(10 ** (halbton(hz, je[0][1]) / 10) + 10 ** (halbton(hz, je[1][1]) / 10))
        zeile += f" {s1[i]:6.1f} |"
    print(zeile)

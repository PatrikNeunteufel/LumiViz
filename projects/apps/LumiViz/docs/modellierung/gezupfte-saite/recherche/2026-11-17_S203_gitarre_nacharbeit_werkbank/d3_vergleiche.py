"""Vergleichsrechnungen fuer den Bericht.
L1: Mores DAGA 2021 Abb. 2 (g20, Faustino Conde 1964, p/F in 1 m, Mobilitaet treble) gegen Bericht 13 r2 (Wright BR2 +
    Christensen-Helmholtz, 1 m). Abgelesene Werte (log. Achse, +-15 %), Modellwerte aus Bericht 13 r2-Ausgabe.
L2: Elie 2012 These Abb. 3.17 (GI18, E4) aus d1_elie_abb317.py: Test 'ein Winkel je Mode' -> Yyy/Yzz = (Yzy/Yzz)^2.
L3: Elie 2011 Abb. 4b abgelesen gegen Koerperdatei (0,0111 /Hz in 2,2-5 kHz, S202-Kopf)."""
import math


def db(x):
    return 20 * math.log10(x)


print("L1  Fernfeld je Kraft und je Stegschnelle in 1 m")
mess = {"A0": (0.65, 0.13), "T1": (2.4, 0.145)}          # p/F Pa/N, |Y| m/(N s) (treble tap/sense), abgelesen
modell = {"A0": (10 ** (-11.3 / 20), 10 ** (-13.8 / 20)), "T1": (10 ** (13.4 / 20), 10 ** (-8.8 / 20))}
for name, (p, y) in mess.items():
    pm, ym = modell[name]
    print(f"  {name}: gemessen p/F = {db(p):+5.1f} dB re 1 Pa/N, p/(F Y) = {p / y:5.2f} Pa s/m | "
          f"Modell p/F = {db(pm):+5.1f} dB, p/(F Y) = {pm / ym:5.2f} Pa s/m")
r_mess = db((mess["T1"][0] / mess["T1"][1]) / (mess["A0"][0] / mess["A0"][1]))
r_mod = db((modell["T1"][0] / modell["T1"][1]) / (modell["A0"][0] / modell["A0"][1]))
print(f"  T1 gegen A0 (p je Stegschnelle): gemessen {r_mess:+.1f} dB, Modell {r_mod:+.1f} dB, Differenz {r_mod - r_mess:+.1f} dB")
r_messF = db(mess["T1"][0] / mess["A0"][0])
r_modF = db(modell["T1"][0] / modell["A0"][0])
print(f"  T1 gegen A0 (p je Kraft):        gemessen {r_messF:+.1f} dB, Modell {r_modF:+.1f} dB, Differenz {r_modF - r_messF:+.1f} dB")

print("\nL2  Ein Winkel je Mode: Vorhersage Yyy/Yzz = 2 * (Yzy/Yzz) in dB")
elie = [("Spitze 230 Hz", -14.5, -22.4), ("150-300 Hz", -14.2, -22.1), ("300-500 Hz", -18.3, -23.7),
        ("500-1000 Hz", -13.5, -10.4), ("1-1,5 kHz", -19.3, -19.2), ("1,5-2 kHz", -9.5, -11.7),
        ("2-2,5 kHz", -13.5, -15.1), ("2,5-3 kHz", -16.9, -16.2), ("3-3,5 kHz", -12.9, -18.9),
        ("3,5-4 kHz", -9.2, -14.2), ("1-4 kHz", -12.9, -15.6)]
for band, kreuz, yy in elie:
    print(f"  {band:14s}: Yzy-Yzz {kreuz:6.1f} dB -> Vorhersage Yyy-Yzz {2 * kreuz:6.1f} dB, gemessen {yy:6.1f} dB, "
          f"Abweichung {yy - 2 * kreuz:+5.1f} dB")

print("\nL3  Modendichte: Elie 2011 Abb. 4b gegen Koerperdatei 0,0111 /Hz (2,2-5 kHz)")
for name, n in (("Industriegitarre 2-6 kHz", 0.0187), ("Luthier-Gitarre 2,2-3 kHz", 0.028),
                ("Luthier-Gitarre 3-4 kHz", 0.026), ("Luthier-Gitarre 4-5 kHz", 0.023), ("Luthier-Gitarre 5-6 kHz", 0.0215)):
    print(f"  {name:26s}: {n:.4f} /Hz = Faktor {n / 0.0111:.2f} ueber der Datei, 1 Mode je {1 / n:.0f} Hz")

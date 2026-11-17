# Bell 1988, Abb. 7.8-7.16 (PDF 294-303): Spannweite der Modenprofile ohne / mit Saiten
# (Harfe G fertig gebaut), am Seitenbild abgelesen, +-1,5 dB. Bezug 116 dB = 1 s/kg (Bell S. 149).
# Prueft die Hypothese "Reihenfeder k im Messweg": dann |Y_gemessen| >= ~ omega/k an jedem Messort
# (ohne Ausloeschung). Aus dem kleinsten besaiteten Wert folgt eine untere Schranke fuer k.
# Aufruf: python d3_profile_reihenfeder.py
import math

K_BERICHT12 = 5.2e4  # N/m, Reihenfeder, die Abb. 7.22 im Mittel trifft (Bericht 12, R8)

# (PDF-Seite, Mode, f_ohne, Q_ohne, min_ohne, max_ohne, f_mit, Q_mit, min_mit, max_mit)
PROFILE = [
    (294, "S(1,1)-", 170, "11", 68.0, 86.3, 187, "unbest.", 68.2, 79.3),
    (295, "S(1,1)+", 216, "16", 66.4, 81.6, 207, "unbest.", 76.8, 86.5),
    (296, "S(1,2)", 249, "39", 65.7, 96.0, 257, "32", 72.5, 92.3),
    (297, "S(1,2)'", 249, "39", 65.7, 96.0, 284, "30", 75.1, 91.5),
    (298, "S(1,3)", 316, "45", 75.7, 104.5, 334, "27", 80.7, 101.0),
    (299, "S(1,4)", 432, "43", 73.5, 105.3, 430, "28", 80.7, 99.7),
    (300, "S(1,5)", 545, "31", 73.2, 101.5, 555, "19", 81.7, 102.7),
    (301, "S(1,6)", 709, "10", 75.3, 100.7, 697, "12", 85.3, 103.0),
    (302, "S(1,7)", 757, "33", 79.3, 99.7, 753, "17", 85.8, 100.8),
    (303, "S(1,8)", 905, "18", 81.0, 102.2, 895, "18", 86.9, 103.1),
]

Y = lambda db: 10 ** ((db - 116.0) / 20)
DB = lambda y: 116.0 + 20 * math.log10(y)

print("PDF | Mode | ohne: f Q min..max | mit: f Q min..max | d_min d_max | Feder 5,2e4 bei f_mit | k >= [N/m]")
schranken = []
for s, m, fo, qo, lo, ho, fm, qm, lm, hm in PROFILE:
    w = 2 * math.pi * fm
    feder = DB(w / K_BERICHT12)
    k_min = w / Y(lm)
    schranken.append(k_min)
    print(f"{s} | {m:8s} | {fo:4d} {qo:>3s} {lo:5.1f}..{ho:5.1f} | {fm:4d} {qm:>7s} {lm:5.1f}..{hm:5.1f} | "
          f"{lm - lo:+5.1f} {hm - ho:+5.1f} | {feder:5.1f} (min mit {lm - feder:+5.1f}) | {k_min:9.3g}")

k_unten = min(schranken)
print()
print(f"Kleinste Schranke: k >= {k_unten:.3g} N/m  (das {k_unten / K_BERICHT12:.1f}-fache von 5,2e4)")
print("Beitrag einer solchen Feder omega/k an den Saiten aus Abb. 7.22 (Bericht 12, Abschn. 3.4 d):")
for f, bell in ((104, 80), (207, 86), (330, 95), (494, 91), (740, 99), (988, 99)):
    print(f"  {f:4d} Hz: Feder {DB(2 * math.pi * f / k_unten):5.1f} dB, Bell 7.22 {bell} dB, Abstand {bell - DB(2 * math.pi * f / k_unten):+5.1f}")

# Steigung der Minima/Maxima ueber log2 f
def steigung(pts):
    n = len(pts)
    mx = sum(p[0] for p in pts) / n
    my = sum(p[1] for p in pts) / n
    return sum((p[0] - mx) * (p[1] - my) for p in pts) / sum((p[0] - mx) ** 2 for p in pts)

print()
for name, idx_f, idx in (("ohne, Minima", 2, 4), ("ohne, Maxima", 2, 5), ("mit, Minima", 6, 8), ("mit, Maxima", 6, 9)):
    pts = [(math.log2(p[idx_f]), p[idx]) for p in PROFILE if p[1] != "S(1,2)'" or idx_f == 6]
    print(f"Steigung {name}: {steigung(pts):+5.2f} dB/Oktave ({len(pts)} Profile)")

"""Hauptrechnung: Varianten der Fis-Alphorn-Bohrung, Spitzen 1-16, Cent gegen Reihe und Tab. 1, Empfindlichkeiten.
Ausgabe als Markdown auf stdout und in ausgabe.md."""
import io
import sys
import numpy as np
import tmm

AUS = io.StringIO()


def p(*a):
    s = " ".join(str(x) for x in a)
    print(s)
    AUS.write(s + "\n")


# ---------------- Daten ----------------
F_FIS1 = 440.0 * 2 ** ((30 - 69) / 12)     # Fis1 bei a' = 440 Hz, gleichstufig = 46.249 Hz
TAB3 = {  # Marcinno Tab. 3 (S. 18): L1..L4, R2..R4 (mm)
    "H1": ([544.96, 844.87, 754.84, 444.94], [13.50, 28.50, 44.00]),
    "H2": ([534.95, 799.86, 804.85, 444.94], [13.50, 28.50, 44.00]),
    "H3": ([539.92, 844.91, 754.84, 444.94], [16.00, 28.50, 44.00]),
}
R1, R5 = 6.5, 51.5                           # Marcinno S. 7 (R_min, R_max)
TAB1_MEAS = {  # Marcinno Tab. 1 (S. 10), Naturton n=4..10 (Do1 Mi1 Sol Sib Do2 Re Mi2)
    "H1": [180.00, 204.00, 278.00, 310.00, 371.00, 417.00, 463.00],
    "H2": [165.00, 211.00, 278.00, 311.00, 366.00, 417.00, 448.00],
    "H3": [185.00, 234.00, 278.00, 330.50, 371.00, 417.00, 454.00],
}
TAB2_FEM_H3 = {2: 94.11, 3: 146.83, 4: 196.97, 5: 246.98, 6: 296.51, 7: 343.50, 8: 388.71, 9: 436.02, 10: 487.77,
               11: 537.15}   # Marcinno Tab. 2 (S. 15) "Reference alphorn"; 537.15 steht unter "Sol 556" (siehe Bericht)
# Becher, Abb. 3b (Nachpruefung Abschn. 3): x, y Mittellinie, r (mm); Aufweitung ab x = 305
BECHER = [(0, 0, R5), (100, 0.5, 51.5), (200, 8, 51.6), (300, 28, 51.8), (305, 30.5, 51.8),
          (350, 53, 57.0), (400, 92, 62.0), (450, 158, 71.0), (492, 232, 89.5)]


def becher_punkte(bogen=True, skala_s=1.0, skala_zyl=1.0, verschiebe_aufw=0.0, skala_rand=0.0):
    """liefert Liste (s, r). s = Bogenlaenge (oder x), r Radius. Varianten fuer Empfindlichkeiten."""
    s = [0.0]
    for (x0, y0, _), (x1, y1, _) in zip(BECHER[:-1], BECHER[1:]):
        d = np.hypot(x1 - x0, y1 - y0) if bogen else (x1 - x0)
        s.append(s[-1] + d)
    s = np.array(s)
    r = np.array([b[2] for b in BECHER], dtype=float)
    i_a = 4                                  # Index der Aufweitungsstelle (x = 305)
    s_a, s_e = s[i_a], s[-1]
    # Laenge des fast zylindrischen Teils: Teil vor der Aufweitung strecken, Rest verschieben
    if skala_zyl != 1.0:
        neu = s.copy()
        neu[:i_a + 1] = s[:i_a + 1] * skala_zyl
        neu[i_a + 1:] = s[i_a + 1:] + s_a * (skala_zyl - 1)
        s = neu
    # Aufweitungsstelle verschieben bei gleicher Becherlaenge
    if verschiebe_aufw != 0.0:
        s_a2 = s_a * (1 + verschiebe_aufw)
        neu = s.copy()
        neu[:i_a + 1] = s[:i_a + 1] * (1 + verschiebe_aufw)
        neu[i_a + 1:] = s_a2 + (s[i_a + 1:] - s_a) * (s_e - s_a2) / (s_e - s_a)
        s = neu
    if skala_rand != 0.0:                    # Randradius +eps, gleitend ab der Aufweitung
        w = np.clip((s - s[i_a]) / (s[-1] - s[i_a]), 0, 1)
        r = r * (1 + skala_rand * w)
    s = s * skala_s
    return s, r


def becher_segmente(**kw):
    s, r = becher_punkte(**kw)
    return [(s[i + 1] - s[i], r[i], r[i + 1]) for i in range(len(s) - 1)]


def mundstueck(art, r1, l_bb=40.0, kessel_skala=1.0):
    """Mundstueck-Varianten (alle Zahlen mm). Rueckgabe: (Segmente, lumped, Laenge)."""
    k = np.sqrt(kessel_skala)
    if art == "a":   # Kessel 19 mm, 'Kesseltiefe 25 mm' bis zur Seele, Seele 5,5 mm
        seg = [(25.0, 9.5 * k, 2.75), (5.0, 2.75, 2.75), (l_bb, 2.75, r1)]
        return seg, None, 30.0 + l_bb
    if art == "b":   # 'Kesseltiefe 25 mm' = Kessel allein, dann Uebergang zur Seele
        seg = [(25.0, 9.5 * k, 6.0 * k), (6.0, 6.0 * k, 2.75), (5.0, 2.75, 2.75), (l_bb, 2.75, r1)]
        return seg, None, 36.0 + l_bb
    if art == "c":   # Kesselvolumen wie (a) + Verengung (Masse) + Rueckbohrung
        V = np.pi * 25.0 / 3 * (9.5 ** 2 + 9.5 * 2.75 + 2.75 ** 2) * kessel_skala
        seg = [(l_bb, 2.75, r1)]
        return seg, (V, 10.0, 2.75), 30.0 + l_bb
    if art == "d":   # Waldhorn-Mundstueck Wachter 2008 Abb. 52 (gemessen), dann Kegel 3,15 -> r1 ueber 20 mm
        x = [0, 5, 10, 15, 20, 25, 30, 30.5, 31.5, 35.9, 45.9, 50.9, 55.9, 60.9, 65.9, 67.38]
        d = [17.5, 14.77, 12.77, 10.3, 8.4, 6.23, 5.2, 4.75, 4.65, 4.7, 4.73, 4.73, 5.04, 5.5, 5.8, 6.3]
        seg = [(x[i + 1] - x[i], d[i] / 2 * (k if x[i] < 31.5 else 1), d[i + 1] / 2 * (k if x[i + 1] < 31.5 else 1))
               for i in range(len(x) - 1)]
        seg.append((20.0, 3.15, r1))
        return seg, None, 87.38
    if art == "0":
        return [], None, 0.0
    raise ValueError(art)


def bohrung(lesart="V1", horn="H3", mp="a", verl=None, l_bb=40.0, eps=None):
    """Segmentliste. lesart: V1 (Radius), V2a (Durchmesser, Sprung am Becher), V2b (Durchmesser, Kegel 4 -> R5).
    verl: None | ('mundrohr'|'kegel1'|'gleich', ziel_mm)  eps: dict der Stoergroessen fuer Empfindlichkeiten."""
    e = eps or {}
    L, R = TAB3[horn]
    L = [l * (1 + e.get("laenge", 0)) for l in L]
    if lesart == "V1":
        r1, rr, r5_ende = R1, R, R5
    else:
        r1, rr = R1 / 2, [x / 2 for x in R]
        r5_ende = R5 / 2 if lesart == "V2a" else R5
    sk = 1 + e.get("radien", 0)
    r1 = r1 * (1 + e.get("R1", 0)) * sk
    rr = [x * sk for x in rr]
    r5_ende = r5_ende * sk
    mpseg, lumped, mp_len = mundstueck(mp, r1, l_bb=l_bb * (1 + e.get("rueckbohrung", 0)),
                                       kessel_skala=1 + e.get("kessel", 0))
    bech = becher_segmente(skala_s=(1 + e.get("laenge", 0)) * (1 + e.get("becherbogen", 0)),
                           skala_zyl=1 + e.get("zyl", 0), verschiebe_aufw=e.get("aufw", 0),
                           skala_rand=e.get("rand", 0), bogen=not e.get("x_statt_bogen", False))
    if sk != 1:
        bech = [(l, a * sk, b * sk) for (l, a, b) in bech]
    ext = []
    if verl is not None:
        wo, ziel = verl
        ist = mp_len + sum(L) + sum(b[0] for b in bech)
        d = ziel - ist
        if wo == "mundrohr":
            ext = [(d, r1, r1)]
        elif wo == "kegel1":
            L = [L[0] + d] + L[1:]
        elif wo == "gleich":
            f = (sum(L) + d) / sum(L)
            L = [l * f for l in L]
    kegel = [(L[0], r1, rr[0]), (L[1], rr[0], rr[1]), (L[2], rr[1], rr[2]), (L[3], rr[2], r5_ende)]
    seg = list(mpseg) + ext + kegel + bech
    gesamt = sum(s[0] for s in seg) + (mp_len - sum(s[0] for s in mpseg) if lumped else 0)
    return seg, lumped, gesamt


def lumiviz_heute(becher="kegel"):
    """Nachbau des LumiViz-Stands (modell.md Z. 40): R1 2,75, Kegel 1-3 auf 16/28,5/44, Kegel 4 Zylinder 44,
    Becher 0,5 m 44 -> 90 (Form unbekannt: Kegel oder exponentiell), ohne Mundstueck."""
    L, R = TAB3["H3"]
    seg = [(L[0], 2.75, 16.0), (L[1], 16.0, 28.5), (L[2], 28.5, 44.0), (L[3], 44.0, 44.0)]
    if becher == "kegel":
        seg.append((500.0, 44.0, 90.0))
    else:
        s = np.linspace(0, 500, 26)
        r = 44.0 * (90.0 / 44.0) ** (s / 500)
        seg += [(s[i + 1] - s[i], r[i], r[i + 1]) for i in range(25)]
    return seg, None, sum(x[0] for x in seg)


LUFT = tmm.luft(20.0)


def rechne(seg, lumped, luft=LUFT, verlust=True, abstr="unflanged"):
    return tmm.spitzen(seg, luft, verlust=verlust, abstrahlung=abstr, lumped=lumped, fmax=1100.0, n_max=16)


def kennzahlen(sp):
    f = np.array([x[0] for x in sp])
    n = np.arange(1, len(f) + 1)
    cA = tmm.cent(f, n * F_FIS1)
    cB = tmm.cent(f, n * f[1] / 2) if len(f) > 1 else cA * np.nan
    sel = (n >= 2) & (n <= 16)
    return f, n, cA, cB, sel


def zeile_kurz(name, sp, laenge):
    f, n, cA, cB, sel = kennzahlen(sp)
    a, b = cA[sel], cB[sel]
    return (f"| {name} | {laenge:.0f} | {len(f)} | {f[0]:.1f} | {f[1]:.2f} | {cA[1]:+.0f} | {np.mean(a):+.0f} | "
            f"{np.mean(np.abs(a)):.0f} | {np.max(np.abs(a)):.0f} | {np.mean(np.abs(b)):.0f} | {np.max(np.abs(b)):.0f} | "
            f"{np.max(a[1:]) - np.min(a[1:]):.0f} |")


def tabelle_spitzen(name, sp, horn=None):
    f, n, cA, cB, sel = kennzahlen(sp)
    p(f"\n#### {name}\n")
    p("| n | f (Hz) | abs(Z) (MPa·s/m³) | Q | Cent gg. n·46,249 Hz (Bezug A) | Cent gg. n·f2/2 (Bezug B) | Cent gg. Tab. 1 gemessen |")
    p("|---|---|---|---|---|---|---|")
    for i, (fp, z, q) in enumerate(sp):
        meas = ""
        if horn and 4 <= n[i] <= 10:
            meas = f"{tmm.cent(fp, TAB1_MEAS[horn][n[i]-4]):+.0f}"
        p(f"| {n[i]} | {fp:.2f} | {z:.2f} | {q:.0f} | {cA[i]:+.0f} | {cB[i]:+.0f} | {meas} |")


def kopf_kurz():
    p("| Variante | Länge (mm) | Spitzen | f1 (Hz) | f2 (Hz) | Cent f2 (A) | Mittel 2–16 (A) | mittl. Betrag (A) | "
      "max. Betrag (A) | mittl. Betrag (B) | max. Betrag (B) | Spanne 3–16 (A) |")
    p("|---|---|---|---|---|---|---|---|---|---|---|---|")


def tab1_vergleich(name, sp, horn):
    f = [x[0] for x in sp]
    c = [tmm.cent(f[n - 1], TAB1_MEAS[horn][n - 4]) for n in range(4, 11)]
    return f"| {name} | {horn} | " + " | ".join(f"{x:+.0f}" for x in c) + f" | {np.mean(c):+.0f} | {np.max(np.abs(c)):.0f} |"


if __name__ == "__main__":
    teil = sys.argv[1] if len(sys.argv) > 1 else "alles"
    p(f"Luft: T = 20 °C, c = {LUFT['c']:.2f} m/s, rho = {LUFT['rho']:.4f} kg/m³; Bezug A: Fis1 = {F_FIS1:.3f} Hz (a' = 440 Hz)")

    # ---- Segmentlisten ----
    if teil in ("alles", "seg"):
        p("\n### Becher-Stützpunkte (Bogenlänge aus Abb. 3b)\n")
        s, r = becher_punkte()
        p("| Punkt | x (mm) | y (mm) | s Bogen (mm) | r (mm) |")
        p("|---|---|---|---|---|")
        for i, b in enumerate(BECHER):
            p(f"| {i} | {b[0]} | {b[1]} | {s[i]:.1f} | {r[i]:.1f} |")
        for les in ("V1", "V2a", "V2b"):
            seg, lu, g = bohrung(les, "H3", "a")
            p(f"\n### Segmentliste {les}, Horn 3, Mundstück a (Gesamtlänge {g:.1f} mm)\n")
            p("| Nr | L (mm) | r ein (mm) | r aus (mm) |")
            p("|---|---|---|---|")
            for i, (L, a, b) in enumerate(seg):
                p(f"| {i+1} | {L:.2f} | {a:.2f} | {b:.2f} |")

    # ---- Hauptvarianten ----
    if teil in ("alles", "haupt"):
        p("\n### Übersicht Varianten (mit Wandverlusten, ungeflanscht, 20 °C)\n")
        kopf_kurz()
        faelle = []
        for les in ("V1", "V2a", "V2b"):
            for mp in ("a", "b", "c", "d", "0"):
                faelle.append((f"{les} H3 MP-{mp}", bohrung(les, "H3", mp)))
            for wo in ("mundrohr", "kegel1", "gleich"):
                faelle.append((f"{les} H3 MP-a +3,47 m {wo}", bohrung(les, "H3", "a", verl=(wo, 3470.0))))
        for h in ("H1", "H2"):
            faelle.append((f"V1 {h} MP-a", bohrung("V1", h, "a")))
        faelle.append(("LumiViz heute (Becher Kegel)", lumiviz_heute("kegel")))
        faelle.append(("LumiViz heute (Becher exp.)", lumiviz_heute("exp")))
        erg = {}
        for name, (seg, lu, g) in faelle:
            sp = rechne(seg, lu)
            erg[name] = sp
            p(zeile_kurz(name, sp, g))
        p("\n### Spitzen gegen Tab. 1 (Cent = 1200·log2(f_Spitze/f_gemessen)), Naturtöne 4–10\n")
        p("| Variante | Horn | n4 | n5 | n6 | n7 | n8 | n9 | n10 | Mittel | max. Betrag |")
        p("|---|---|---|---|---|---|---|---|---|---|---|")
        for name, sp in erg.items():
            horn = "H1" if " H1 " in name else "H2" if " H2 " in name else "H3"
            p(tab1_vergleich(name, sp, horn))
        for name in ("V1 H3 MP-a", "V2a H3 MP-a", "V2b H3 MP-a", "V1 H3 MP-a +3,47 m gleich",
                     "V2a H3 MP-a +3,47 m gleich", "LumiViz heute (Becher Kegel)"):
            tabelle_spitzen(name, erg[name], "H3")

    # ---- Zerlegung: vom LumiViz-Stand zu V1 (welcher Schritt verschiebt wie viel) ----
    if teil in ("alles", "zerlegung"):
        p("\n### Zerlegung LumiViz heute → V1 (Cent gg. n·46,249 Hz, Mittel 2–16 / Spitze 2 / Bezug-B-Betrag)\n")
        L, R = TAB3["H3"]
        bech_lv = [(500.0, 44.0, 90.0)]
        bech_v1 = becher_segmente()
        schritte = [
            ("0 LumiViz heute: R1 2,75, K4 Zylinder 44, Becher Kegel 0,5 m 44→90, ohne MP",
             [(L[0], 2.75, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 44)] + bech_lv, None),
            ("1 + R1 = 6,5", [(L[0], 6.5, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 44)] + bech_lv, None),
            ("2 + Kegel 4 44→51,5, Becher 51,5→90 (0,5 m)",
             [(L[0], 6.5, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 51.5), (500.0, 51.5, 90)], None),
            ("3 + Becher nach Abb. 3b (Bogen 589,5 mm)",
             [(L[0], 6.5, 16), (L[1], 16, 28.5), (L[2], 28.5, 44), (L[3], 44, 51.5)] + bech_v1, None),
            ("4 + Mundstück a (= V1 H3 MP-a)", bohrung("V1", "H3", "a")[0], None),
            ("5 + Verlängerung gleichmäßig auf 3,47 m", bohrung("V1", "H3", "a", verl=("gleich", 3470.0))[0], None),
        ]
        p("| Schritt | Länge (mm) | Cent Spitze 2 (A) | Mittel 2–16 (A) | mittl. Betrag (B) | max. Betrag (B) |")
        p("|---|---|---|---|---|---|")
        for name, seg, lu in schritte:
            f, n, cA, cB, sel = kennzahlen(rechne(seg, lu))
            p(f"| {name} | {sum(s[0] for s in seg):.0f} | {cA[1]:+.0f} | {np.mean(cA[sel]):+.0f} | "
              f"{np.mean(np.abs(cB[sel])):.0f} | {np.max(np.abs(cB[sel])):.0f} |")

    # ---- Vergleich mit Marcinno FEM (verlustfrei, c = 340) ----
    if teil in ("alles", "fem"):
        p("\n### V1 H3 verlustfrei, c = 340 m/s, rho = 1,225 (wie Marcinnò S. 5) gegen Tab. 2 „Reference alphorn“\n")
        lu340 = dict(T=293.15, c=340.0, rho=1.225)
        p("| Variante | Abstrahlung | n2 | n3 | n4 | n5 | n6 | n7 | n8 | n9 | n10 | n11 | Mittel 2–10 |")
        p("|---|---|---|---|---|---|---|---|---|---|---|---|---|")
        for mp in ("a", "b", "c", "d", "0"):
            for ab in ("unflanged", "flanged"):
                seg, lu, g = bohrung("V1", "H3", mp)
                sp = tmm.spitzen(seg, lu340, verlust=False, abstrahlung=ab, lumped=lu, fmax=700, n_max=12)
                c = [tmm.cent(sp[n - 1][0], TAB2_FEM_H3[n]) for n in range(2, 12)]
                p(f"| V1 H3 MP-{mp} | {ab} | " + " | ".join(f"{x:+.0f}" for x in c) + f" | {np.mean(c[:9]):+.0f} |")

    # ---- Einfluss Verluste, Abstrahlung, Temperatur ----
    if teil in ("alles", "modell"):
        p("\n### Modellwahl bei V1 H3 MP-a: Cent-Verschiebung der Spitzen 2–16 gegen den Grundfall\n")
        seg, lu, g = bohrung("V1", "H3", "a")
        basis = np.array([x[0] for x in rechne(seg, lu)])
        fall = {
            "ohne Wandverluste": rechne(seg, lu, verlust=False),
            "geflanscht (G9)": rechne(seg, lu, abstr="flanged"),
            "T = 30 °C": rechne(seg, lu, luft=tmm.luft(30.0)),
            "T = 14,5 °C (c ≈ 340)": rechne(seg, lu, luft=tmm.luft(14.5)),
        }
        p("| Fall | " + " | ".join(f"n{n}" for n in range(2, 17)) + " |")
        p("|---|" + "---|" * 15)
        for k, sp in fall.items():
            f = np.array([x[0] for x in sp])
            m = min(len(f), len(basis), 16)
            p(f"| {k} | " + " | ".join(f"{tmm.cent(f[i], basis[i]):+.1f}" for i in range(1, m)) + " |")

    # ---- Empfindlichkeiten ----
    if teil in ("alles", "empf"):
        groessen = [("R1", "R1"), ("Kesselvolumen", "kessel"), ("Rückbohrungslänge (Volumen)", "rueckbohrung"),
                    ("Becher-Randradius", "rand"), ("Länge fast zyl. Becherteil", "zyl"),
                    ("Aufweitungsstelle (Becherlänge fest)", "aufw"), ("Becher-Bogenlänge", "becherbogen"),
                    ("Gesamtlänge (L1–L4 + Becher)", "laenge"), ("alle Radien R1–Rand", "radien")]
        for les in ("V1", "V2a"):
            p(f"\n### Empfindlichkeit {les} H3 MP-a: Cent je +1 % (zentrale Differenz ±1 %), Spitzen 2–16\n")
            p("| Größe | " + " | ".join(f"n{n}" for n in range(2, 17)) + " | Spanne n2–16 |")
            p("|---|" + "---|" * 16)
            for titel, key in groessen:
                fp = [x[0] for x in rechne(*bohrung(les, "H3", "a", eps={key: 0.01})[:2])]
                fm = [x[0] for x in rechne(*bohrung(les, "H3", "a", eps={key: -0.01})[:2])]
                m = min(len(fp), len(fm), 16)
                c = [tmm.cent(fp[i], fm[i]) / 2 for i in range(1, m)]
                p(f"| {titel} | " + " | ".join(f"{x:+.1f}" for x in c) + f" | {max(c)-min(c):.1f} |")

    # ---- Annahmen: Rueckbohrung 20/70, Kruemmung x statt Bogen ----
    if teil in ("alles", "annahmen"):
        p("\n### Annahmen-Spannen (V1 H3 MP-a als Grundfall): Cent der Spitzen 2–16 gegen Grundfall\n")
        seg, lu, g = bohrung("V1", "H3", "a")
        basis = np.array([x[0] for x in rechne(seg, lu)])
        fall = {
            "Rückbohrung 20 mm": bohrung("V1", "H3", "a", l_bb=20.0),
            "Rückbohrung 70 mm": bohrung("V1", "H3", "a", l_bb=70.0),
            "Becher x statt Bogen (492 statt 590 mm)": bohrung("V1", "H3", "a", eps={"x_statt_bogen": True}),
            "Becherbogen +5 %": bohrung("V1", "H3", "a", eps={"becherbogen": 0.05}),
            "Becherbogen −5 %": bohrung("V1", "H3", "a", eps={"becherbogen": -0.05}),
        }
        p("| Fall | Gesamtlänge (mm) | " + " | ".join(f"n{n}" for n in range(2, 17)) + " |")
        p("|---|---|" + "---|" * 15)
        for k, (s_, l_, g_) in fall.items():
            f = np.array([x[0] for x in rechne(s_, l_)])
            m = min(len(f), len(basis), 16)
            p(f"| {k} | {g_:.0f} | " + " | ".join(f"{tmm.cent(f[i], basis[i]):+.1f}" for i in range(1, m)) + " |")

    with open(__file__.replace("rechnung.py", f"ausgabe_{teil}.md"), "w", encoding="utf-8") as fh:
        fh.write(AUS.getvalue())

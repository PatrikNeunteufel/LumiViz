"""Schreibt den Bauplan der Konzertharfe `concert_harp.lvinstbodyplan` (Stufe 34k-5, S198; KQ1 a, KQ17 a, KQ18 a, KQ19 a, KQ20 a,
KQ21 a; S199: KQ22 a; S211: KQ46 a, KQ49 a, KQ50 a, KQ51 a, KQ52 a).

Aufruf (vom Projekt-Root):
    python -I -B asset/loop/instrument_bodies/make_harfe.py [--saiten <saitenorte.json>] [--ziel <ordner>]

**Massgebend ist die Datei im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie schreiben und
vergleichen. Danach den Koerper neu rechnen (README.md hier).

Quelle: A. J. Bell, „An Acoustical Investigation of the Concert Harp“, PhD, University of St Andrews 1988 — die Salvi
„Orchestra“ (46 Saiten). Alle Zahlen am Bild gelesen (die Textebene der Datei ist OCR). Herleitung:
`projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md`.

Koordinaten: x quer (Mitte 0), y laengs vom Bassende (0) zum Diskant (1,31 m). Faser des Bodens quer (x).

Was woher kommt:
  Boden             Abb. 2.4 a: 131 cm, 41 -> 10 cm breit, 10 -> 3 mm dick, Faser quer; vorn 1 mm Furnier (Zuschlag)
  Holz des Bodens   am freien Boden ohne Leisten geeicht (Tab. 3.1 / 3.2, elf Moden innerhalb 3 %): 500 kg/m3,
                    E 7,3 GPa quer zum Boden, 0,855 GPa laengs, G 0,45 GPa; Masse 1,371 kg (gemessen 1,28-1,51)
  Deckleiste        Abb. 2.4 b: Buche, aussen in der Mitte, 33 -> 18 mm breit, 5 -> 3 mm hoch
  Zugleiste         Abb. 2.4 d: Buche, innen in der Mitte, aus der Zeichnung abgegriffen (+-1 mm Hoehe, +-2 mm Breite)
  zwei Leisten      Abb. 2.4 c: Fichte, 105 cm, 15 -> 7 mm hoch, 11,5 -> 5 mm breit, auf halbem Weg zum Rand
  Buche             Chabassier, Joly & Chaigne 2013 Tab. I — Annahme
  Hohlraum          Tab. 4.1: 27 l, sechs Oeffnungen in der Rueckseite der Schale (Flaeche, Lippe); Lage laengs aus
                    Abb. 2.8 (grob); Muendungskorrektur 2,3 Radien (Feinsuche S210, Fassung F4, KQ51 a); Guete der
                    Luft 8,5 (KQ29-1 a)
  Rand              gestuetzt (elastisch mit Feder 0): die Auslenkung gehalten, die Drehung frei — an der Lage der
                    Baeuche laengs der Deckleiste geprueft (Abb. 7.8-7.16; Bericht Par. 11)
  Bassseiten        KQ50 a, KQ51 a: die beiden Seiten vom Bassende bis y = 0,6 m als eigene Abschnitte des Rands mit
                    Drehfeder 7000 N, die Auslenkung gehalten (Wegfeder 1e12 N/m2, wie in der Feinsuche) — angepasst,
                    ohne Quelle: der Boden ist im Bassteil sonst zu weich (S(1,1)- ohne Luft 146 Hz; Bericht Par. 19-20)
  Saiten            KQ46 a, KQ49 a, KQ52 a: je Saite eine Feder an ihrer Oese, k = 0,3 E A / L, ohne Verlust;
                    Durchmesser und Laengen Bell Tab. 2.1 / 2.2, E Nylon 4, Darm 5,5, Stahlkern 200 GPa (Annahmen);
                    0,3 gibt die fuenf Verschiebungen beim Besaiten (Abb. 7.8-7.16) mit der kleinsten Summe der
                    Quadrate wieder (Bericht Par. 20.6)
  Diskantkante      ein Abschnitt des Rands (KQ20 a, KQ21 a): die Auslenkung nicht gehalten, Wegfeder 3e6 N/m2 —
                    angepasst, nicht gemessen: damit liegt S(1,7) an den Oesen in der Schwelle (-5,4 statt
                    -7,4 dB), der Pegel an Bells Messort 2 steigt um 5-10 dB (Abb. 6.6; Bericht Par. 12)
  Daempfung         alpha = a f^2 + c aus den Gueten des Kastens G in der Harfe ohne Saiten (Abb. 7.10-7.16:
                    249 / 316 / 432 / 545 / 757 / 905 Hz mit Q 39 / 45 / 43 / 31 / 33 / 18; ohne S(1,6), Q 10);
                    eta = alpha / (2 pi f); ueber 1 kHz fortgeschrieben
  Stegpunkte        je Saite einer, an ihrer Oese in der Mitte der Deckleiste (Abb. 2.9, von Hand am Bild, +-0,5 cm);
                    Taste = die Saite mit dem Pedal in der Mitte (Bell Tab. 2.1 / 2.2 nennt die Saiten in der
                    Grundstellung, einen Halbton tiefer) — Annahme: D1 ... G7, die weissen Tasten 26 ... 103
  Tasten            KQ22 a: je chromatischer Taste 25 ... 104 ein Stegpunkt, 80 Punkte an 46 Orten — die schwarze
                    Taste an der Oese der Saite darunter (Pedal unten), die Taste 25 an der tiefsten Saite (Pedal
                    oben); der Koerper gibt damit jeder Taste, die eine der 46 Saiten erreicht, eine Antwort.
                    Benachbarte Oesen unterscheiden sich im Mittel um 1,5 dB je Terzband, 0,4 dB breitbandig
Zielgroessen nach KQ40 a und KQ31-2 a (ohne Saiten, Kasten G in der Harfe): beide Moden mit Luft und S(1,2) ... S(1,6)
in der Lage <= 7 %, Ort des staerksten Bauchs <= 2 Messorte, Hoehe der Baeuche <= 6 dB; die bekannten Abweichungen
haelt der Test „Luftraum die mitgelieferte Harfe“ mit Wert. Messorte nach Abb. 6.3: y = 0,035 (40 - Ort) - 0,057 m.
"""
import json
import math
import pathlib
import sys

SAITEN = "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-14_S198_harfe_saitenorte.json"
QUELLE = ("A. J. Bell, An Acoustical Investigation of the Concert Harp, PhD thesis, University of St Andrews 1988 "
          "(Salvi Orchestra, 46 strings); all numbers read from the figures and tables")

LAENGE = 1.31
BREITE_BASS, BREITE_DISKANT = 0.41, 0.10
DICKE_BASS, DICKE_DISKANT = 0.010, 0.003
FURNIER = 0.001
SCHRITT = 0.01
# Oeffnungen (Tab. 4.1): Nr., Flaeche m2, Lippe m; Lage laengs (Anteil der Laenge vom Bassende, Abb. 2.8)
OEFFNUNGEN = [(1, 91e-4, 0.016, 0.19), (2, 93e-4, 0.015, 0.356), (3, 77e-4, 0.015, 0.524), (4, 68e-4, 0.013, 0.694),
              (5, 58e-4, 0.012, 0.87), (6, 67e-4, 0.021, 0.03)]
# Zugleiste (Abb. 2.4 d): y vom Bassende m, Hoehe m, Breite m
ZUGLEISTE = [(0.0, 0.020, 0.032), (0.29, 0.025, 0.036), (0.55, 0.019, 0.039), (0.80, 0.013, 0.034), (1.05, 0.0072, 0.027),
             (1.28, 0.0056, 0.020)]
# Frequenzen der 46 Saiten in der Grundstellung (Tab. 2.1 / 2.2), Saite 00 ... 44
SAITEN_HZ = [2960, 2637, 2489, 2218, 1976, 1864, 1661, 1480, 1318, 1244, 1109, 988, 932, 830, 740, 659, 622, 554, 494, 466,
             415, 370, 330, 311, 277, 247, 233, 207, 185, 165, 155, 139, 123, 116, 104, 92, 82, 77, 69, 61, 58, 52, 46, 41, 39,
             35]
NAMEN = ["00", "0"] + [str(k) for k in range(1, 45)]
# Saiten (Tab. 2.1 / 2.2, am Bild gelesen, S198), Saite 00 ... 34: Durchmesser mm, Laenge m (00 ... 8 Nylon, ab 9 Darm);
# Saite 35 ... 44: Durchmesser des Stahlkerns mm, Laenge m
DARM_AB = 10
SAITEN_MONO = [(0.52, 0.071), (0.52, 0.082), (0.56, 0.088), (0.56, 0.095), (0.61, 0.105), (0.62, 0.113), (0.65, 0.123),
               (0.69, 0.132), (0.70, 0.143), (0.69, 0.154), (0.70, 0.164), (0.73, 0.177), (0.76, 0.192), (0.80, 0.208),
               (0.85, 0.225), (0.90, 0.242), (0.95, 0.265), (1.00, 0.285), (1.06, 0.327), (1.12, 0.354), (1.18, 0.385),
               (1.24, 0.421), (1.30, 0.455), (1.36, 0.498), (1.44, 0.542), (1.52, 0.591), (1.60, 0.644), (1.68, 0.699),
               (1.76, 0.753), (1.84, 0.813), (1.92, 0.880), (2.00, 0.945), (2.10, 0.999), (2.20, 1.053), (2.35, 1.103)]
SAITEN_METALL = [(0.80, 1.150), (0.80, 1.180), (0.80, 1.222), (0.85, 1.257), (0.90, 1.299), (0.95, 1.330), (1.00, 1.359),
                 (1.10, 1.397), (1.10, 1.421), (1.25, 1.445), (1.30, 1.475)]
E_NYLON, E_DARM, E_STAHL = 4.0e9, 5.5e9, 200.0e9
FAKTOR_SAITEN = 0.3
# Bassseiten (KQ50 a, KQ51 a)
BASSSEITEN_BIS = 0.6
BASSSEITEN_DREHFEDER = 7000.0
GEHALTEN = 1.0e12


def saitenfedern():
    """E A / L in N/m je Saite 00 ... 44 und der Werkstoff."""
    aus = []
    for i, (d, L) in enumerate(SAITEN_MONO):
        e, art = (E_NYLON, "nylon, E 4 GPa") if i < DARM_AB else (E_DARM, "gut, E 5.5 GPa")
        aus.append((e * math.pi * (d / 2000) ** 2 / L, f"{art}, {d} mm, {L} m"))
    for kern, L in SAITEN_METALL:
        aus.append((E_STAHL * math.pi * (kern / 2000) ** 2 / L, f"steel core, E 200 GPa, core {kern} mm, {L} m"))
    assert len(aus) == len(SAITEN_HZ)
    return aus
WEISS = {0, 2, 4, 5, 7, 9, 11}


def breite(y):
    return BREITE_BASS + (BREITE_DISKANT - BREITE_BASS) * y / LAENGE


def tasten():
    """Taste je Saite: die Grundstellung (Bell) plus ein Halbton — das Pedal in der Mitte."""
    aus = []
    for hz in SAITEN_HZ:
        grund = int(round(69.0 + 12.0 * math.log2(hz / 440.0)))
        aus.append(grund + 1)
    assert all(t % 12 in WEISS for t in aus), "jede Saite liegt mit dem Pedal in der Mitte auf einer weissen Taste"
    assert all(a > b for a, b in zip(aus, aus[1:])), "die Saiten fallen von 00 nach 44"
    assert aus[0] == 103 and aus[-1] == 26
    return aus


def baue(saiten):
    assert [s["name"] for s in saiten] == NAMEN
    # Kanten: 0 Bassende, 1 rechte Bassseite, 2 rechte Seite, 3 Diskantkante, 4 linke Seite, 5 linke Bassseite
    xb = round(breite(BASSSEITEN_BIS) / 2, 5)
    umriss = [[-BREITE_BASS / 2, 0.0], [BREITE_BASS / 2, 0.0], [xb, BASSSEITEN_BIS], [BREITE_DISKANT / 2, LAENGE],
              [-BREITE_DISKANT / 2, LAENGE], [-xb, BASSSEITEN_BIS]]
    nx, ny = int(round(BREITE_BASS / SCHRITT)) + 1, int(round(LAENGE / SCHRITT)) + 1
    werte = []
    for j in range(ny):
        y = min(j * SCHRITT, LAENGE)
        werte.extend([round(DICKE_BASS + (DICKE_DISKANT - DICKE_BASS) * y / LAENGE + FURNIER, 6)] * nx)
    rippen = [{"name": "straining bar", "material": "beech",
               "line": [[0.0, max(0.005, p[0])] for p in ZUGLEISTE],
               "height": [p[1] for p in ZUGLEISTE], "width": [p[2] for p in ZUGLEISTE]}]
    for seite, name in ((-1, "left"), (1, "right")):
        y0, y1 = 0.10, 1.15
        rippen.append({"name": "harmonic bar " + name, "material": "spruce",
                       "line": [[round(seite * breite(y0) / 4, 5), y0], [round(seite * breite(y1) / 4, 5), y1]],
                       "height": [0.015, 0.007], "width": [0.0115, 0.005]})
    # KQ22 a: je chromatischer Taste ein Stegpunkt — die weisse Taste an der Oese ihrer Saite (Pedal in der Mitte),
    # die schwarze an der Oese der Saite darunter (Pedal unten), die Taste 25 an der tiefsten Saite (Pedal oben)
    punkte = []
    for s, hz, taste in zip(saiten, SAITEN_HZ, tasten()):
        saite = f"string {s['name']} (Bell 1988 Table 2.1 / 2.2: {hz} Hz with the pedal up)"
        oese = "eyelet on the centre line of the cover bar, read by eye from Fig. 2.9"
        punkte.append({"key": taste, "x": 0.0, "y": s["ort"],
                       "source": f"{saite}; the key is the string with the pedal in the middle - assumed; {oese}"})
        if (taste + 1) % 12 not in WEISS:
            punkte.append({"key": taste + 1, "x": 0.0, "y": s["ort"],
                           "source": f"{saite}; the key is the string with the pedal down - assigned (KQ22 a): a black "
                                     f"key sits on the string below; {oese}"})
        if taste == 26:
            punkte.append({"key": 25, "x": 0.0, "y": s["ort"],
                           "source": f"{saite}; the key is the string with the pedal up - assigned (KQ22 a): the lowest "
                                     f"key sits on the lowest string; {oese}"})
    punkte.sort(key=lambda p: p["key"])
    assert [p["key"] for p in punkte] == list(range(25, 105)), "je Taste 25 ... 104 genau ein Stegpunkt"
    assert len({p["y"] for p in punkte}) == len(saiten), "80 Tasten an 46 Orten"
    return {
        "format": "lumiviz-instrument-body-plan",
        "version": 9,
        "title": "Concert harp",
        "description": "Soundboard of the Salvi Orchestra concert harp with the air of its soundbox (Bell 1988; KQ1 a, "
                       "KQ17 a to KQ21 a). Spruce board 131 cm, 41 to 10 cm wide, 10 to 3 mm thick, grain across, "
                       "with the beech cover bar outside, the beech straining bar and two spruce bars inside. The wood "
                       "of the board is calibrated on the free board without bars (Tables 3.1 and 3.2). Cavity of 27 l "
                       "with six apertures in the back of the shell. The rim is supported (deflection held, rotation "
                       "free): checked on the position of the antinodes along the cover bar (Figs. 7.8 to 7.16). "
                       "The treble edge is held by a spring only, the bass sides up to 0.6 m carry a rotational "
                       "spring (both fitted, not measured; KQ50 a, KQ51 a). The body is the strung harp (KQ46 a): each "
                       "string is a spring at its eyelet, 0.3 E A / L (KQ52 a). "
                       "Targets without strings (KQ40 a, KQ31-2 a): both modes with air within 7 % of 170 / 216 Hz, "
                       "S(1,2) to S(1,6) within 7 %, the strongest antinode within two testing positions, the "
                       "antinodes within 6 dB; known deviations are held with their values in the test. The shell is "
                       "rigid; only the Helmholtz mode of the air is computed.",
        "source": QUELLE,
        "units": "SI: m, kg/m^3, Pa",
        "materials": [
            {"name": "spruce", "kind": "orthotropic", "density": 500.0, "ex": 7.3e9, "ey": 0.855e9, "gxy": 0.45e9,
             "gxz": 1.2e9, "gyz": 0.042e9, "nuxy": 0.3, "lossFactor": 0.0,
             "source": "calibrated on the free soundboard without bars (Bell 1988 Tables 3.1 and 3.2: eleven modes within "
                       "3 %, mass 1.371 kg against 1.28-1.51 kg measured); ex across the board (along the grain), ey "
                       "along the board with the veneer; transverse shear assumed"},
            {"name": "beech", "kind": "orthotropic", "density": 750.0, "ex": 13.7e9, "ey": 2.24e9, "gxy": 1.61e9,
             "gxz": 1.06e9, "gyz": 0.46e9, "nuxy": 0.3, "lossFactor": 0.0,
             "source": "assumed: Chabassier, Joly & Chaigne 2013 Table I"}],
        "plate": {"material": "spruce", "grainAngle": 0.0, "outline": umriss, "thickness": 0.006,
                  "thicknessGrid": {"origin": [-BREITE_BASS / 2, 0.0], "step": SCHRITT, "nx": nx, "ny": ny, "values": werte},
                  "holes": []},
        "ribs": rippen,
        "bridges": [{"name": "cover bar", "material": "beech", "line": [[0.0, 0.005], [0.0, LAENGE - 0.005]],
                     "height": [0.005, 0.003], "width": [0.033, 0.018]}],
        "bridgePoints": punkte,
        "listeningPoints": [{"name": "front", "x": 0.0, "y": 0.6, "z": 1.0, "source": "assumed: 1 m in front of the soundboard"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off: the soundboard closes the soundbox"},
        "damping": {"a": 2.6e-4, "b": 0.0, "c": 20.5,
                    "source": "fit to the Q of soundbox G in the harp without strings (Bell 1988 Figs. 7.10-7.16: 249 / 316 / "
                              "432 / 545 / 757 / 905 Hz with Q 39 / 45 / 43 / 31 / 33 / 18; S(1,6) with Q 10 left out); "
                              "extrapolated above 1 kHz"},
        "cavity": {"volume": 0.027, "endCorrection": 2.3, "q": 8.5,
                   "source": "volume and apertures: Bell 1988 Table 4.1; end correction 2.3 from the fine search S210 "
                             "(version F4, KQ51 a: both modes with air and S(1,2) to S(1,6) within 7 %); Q of the air "
                             "fitted to both Q of soundbox G fitted to the harp, without strings (Bell 1988 Figs. 7.8, "
                             "7.9: 11 and 16) in the least-squares sense of the logarithm - with both bars on the centre "
                             "line (KQ31-1 a) the two modes share the air unequally (0.28 / 0.67) and one value cannot "
                             "give both: the model has 18.2 and 11.5 (KQ29-1 a) - it also stands for losses in the "
                             "frame of the harp; the isolated box F has 18 and 14 (Table 6.1)",
                   "openings": [{"x": 0.0, "y": round(anteil * LAENGE, 4), "z": round(-breite(anteil * LAENGE) / 2, 4),
                                 "area": flaeche, "lipLength": lippe,
                                 "source": f"aperture {nr} (Bell 1988 Table 4.1; position along the box from Fig. 2.8, "
                                           "depth assumed as half the width of the board)"}
                                for nr, flaeche, lippe, anteil in OEFFNUNGEN]},
        "rim": {"support": "elastic", "rotationalStiffness": 0.0,
                "source": "supported: the board sits between the holding bar and the rim bar (Bell 1988 Fig. 2.6); with a "
                          "rotational spring or a clamped rim the antinodes lie 6-9 testing positions too far towards the "
                          "bass (Figs. 7.8-7.16)",
                "sections": [{"name": "treble edge", "edges": [3, 3], "translationalStiffness": 3.0e6,
                              "rotationalStiffness": 0.0,
                              "source": "fitted, not measured: with this spring S(1,7) lies within 6 dB at the eyelets "
                                        "(-5.4 instead of -7.4 dB) and the level at Bell's testing position 2 rises by "
                                        "5 to 10 dB towards the measurement (Bell 1988 Fig. 6.6); the edge sits on the "
                                        "top block of the shell"}] + [
                    {"name": name, "edges": [kante, kante], "translationalStiffness": GEHALTEN,
                     "rotationalStiffness": BASSSEITEN_DREHFEDER,
                     "source": f"fitted, without a source (KQ50 a, KQ51 a): the side from the bass end to y = "
                               f"{BASSSEITEN_BIS} m held (deflection held by a stiff spring of {GEHALTEN:g} N/m2, as in "
                               f"the fine search) with a rotational spring of {BASSSEITEN_DREHFEDER:g} N; without it "
                               f"the board is too soft in the bass (S(1,1)- without air at 146 Hz); version F4 of "
                               f"the fine search S210, checked against all targets"}
                    for name, kante in (("right bass side", 1), ("left bass side", 5))]},
        "barOverlap": "add",
        "pointSprings": [
            {"x": 0.0, "y": s["ort"], "stiffness": round(FAKTOR_SAITEN * ea_l, 3), "lossFactor": 0.0,
             "source": f"string {s['name']} as a spring at its eyelet (KQ46 a, KQ52 a): k = {FAKTOR_SAITEN} E A / L, "
                       f"{text} (Bell 1988 Table 2.1 / 2.2; E assumed); the factor fits the five shifts on stringing "
                       f"(Figs. 7.8-7.16) with the least sum of squares; without loss (KQ49 a: assumed, as the probe "
                       f"S209)"}
            for s, (ea_l, text) in zip(saiten, saitenfedern())],
    }


if __name__ == "__main__":
    args = sys.argv[1:]
    quelle = args[args.index("--saiten") + 1] if "--saiten" in args else SAITEN
    ziel = pathlib.Path(args[args.index("--ziel") + 1]) if "--ziel" in args else pathlib.Path(__file__).resolve().parent
    plan = baue(json.load(open(quelle, encoding="utf-8"))["saiten"])
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "concert_harp.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    print(datei, f"({len(plan['bridgePoints'])} bridge points, keys {plan['bridgePoints'][0]['key']}-{plan['bridgePoints'][-1]['key']}, "
          f"cavity {plan['cavity']['volume'] * 1000:.0f} l, {len(plan['cavity']['openings'])} openings)")

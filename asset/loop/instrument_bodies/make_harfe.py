"""Schreibt den Bauplan der Konzertharfe `concert_harp.lvinstbodyplan` (Stufe 34k-5, S198; KQ1 a, KQ17 a, KQ18 a, KQ19 a, KQ20 a,
KQ21 a; S199: KQ22 a).

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
                    Abb. 2.8 (grob); Muendungskorrektur 2,6 Radien, damit die Helmholtz-Mode bei starrer Wand bei
                    185 Hz liegt (Bell: 190 Hz); Guete der Luft 16 aus S(1,1)-/+ (Tab. 6.1: 18, 14)
  Rand              gestuetzt (elastisch mit Feder 0): die Auslenkung gehalten, die Drehung frei — an der Lage der
                    Baeuche laengs der Deckleiste geprueft (Abb. 7.8-7.16; Bericht Par. 11)
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
Zielgroessen nach KQ19 a (S(1,2) ... S(1,7), Kasten G in der Harfe): Lage <= 10 % ohne Saiten und besaitet, Ort des
staerksten Bauchs <= 2 Messorte, Hoehe <= 6 dB ohne Saiten. Bekannte Abweichungen: S(1,1)-, S(1,8) und die Lage von
S(1,2) gegen den Kasten ohne Saiten (+10,4 %). Messorte nach Abb. 6.3: y = 0,035 (40 - Ort) - 0,057 m.
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
    umriss = [[-BREITE_BASS / 2, 0.0], [BREITE_BASS / 2, 0.0], [BREITE_DISKANT / 2, LAENGE], [-BREITE_DISKANT / 2, LAENGE]]
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
        "version": 7,
        "title": "Concert harp",
        "description": "Soundboard of the Salvi Orchestra concert harp with the air of its soundbox (Bell 1988; KQ1 a, "
                       "KQ17 a to KQ21 a). Spruce board 131 cm, 41 to 10 cm wide, 10 to 3 mm thick, grain across, "
                       "with the beech cover bar outside, the beech straining bar and two spruce bars inside. The wood "
                       "of the board is calibrated on the free board without bars (Tables 3.1 and 3.2). Cavity of 27 l "
                       "with six apertures in the back of the shell. The rim is supported (deflection held, rotation "
                       "free): checked on the position of the antinodes along the cover bar (Figs. 7.8 to 7.16). "
                       "The treble edge is held by a spring only (fitted, not measured). "
                       "Targets, soundbox G in the harp, S(1,2) to S(1,7): frequency within 10 % without and with "
                       "strings, the strongest antinode within two testing positions, height within 6 dB without "
                       "strings. Known deviations: S(1,1)- lies at 129 Hz (measured 153 / 170 / 187 Hz isolated, in the "
                       "harp, strung: the whole soundbox moves), S(1,8) is 10 dB too low. The shell is rigid; only the "
                       "Helmholtz mode of the air is computed.",
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
        "cavity": {"volume": 0.027, "endCorrection": 2.6, "q": 5.5,
                   "source": "volume and apertures: Bell 1988 Table 4.1; end correction set so that the Helmholtz mode with "
                             "rigid walls lies at 185 Hz (Bell: 190 Hz; the apertures lie close together); Q of the air "
                             "fitted so that S(1,1)-/+ get Q 13 in the harp (Bell 1988 Figs. 7.8, 7.9: 11 and 16 on "
                             "soundbox G fitted to the harp, without strings; KQ29 a) - it also stands for losses in the "
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
                "sections": [{"name": "treble edge", "edges": [2, 2], "translationalStiffness": 3.0e6,
                              "rotationalStiffness": 0.0,
                              "source": "fitted, not measured: with this spring S(1,7) lies within 6 dB at the eyelets "
                                        "(-5.4 instead of -7.4 dB) and the level at Bell's testing position 2 rises by "
                                        "5 to 10 dB towards the measurement (Bell 1988 Fig. 6.6); the edge sits on the "
                                        "top block of the shell"}]},
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

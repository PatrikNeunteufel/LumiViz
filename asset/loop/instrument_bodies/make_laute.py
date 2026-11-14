"""Schreibt den Bauplan der Laute `lute.lvinstbodyplan` (Stufe 34k-5, S197; KQ4 a, KQ6 a, KQ11 a, KQ12 a).

Aufruf (vom Projekt-Root):
    python -I -B asset/loop/instrument_bodies/make_laute.py [--umriss <laute_umriss.json>] [--ziel <ordner>]

**Massgebend ist die Datei im Repo, nicht ein neuer Lauf** — zuerst mit `--ziel` in eine Kopie schreiben und
vergleichen. Danach den Koerper neu rechnen (README.md hier).

Ein **Ersatz-Bauplan** (KQ6 a) aus zwei Quellen, die verschiedene Lauten beschreiben:
  Form     J. Santa Maria Bouquet, Messzeichnung der Laute von Sixtus Rauwolf 1598 (CL.93), Musikmuseet Kopenhagen,
           2020 (Vektor-PDF 1:1; gelesen mit `…/gezupfte-saite/proben/2026-11-13_S197_laute_umriss.py`)
  Ziel     J. Woodhouse, „The acoustics of double decays“, JASA 150 (2021), Abb. 1: gemessene Stegadmittanz einer
           8-choerigen Renaissancelaute mit 0,625 m Mensur (aus dem Bild gelesen, `…_laute_abb1_kurve.txt`)
Die Rauwolf-Laute hat 677,4 mm Mensur (Masszahl Steg–Sattel); nach KQ12 a gehen alle Laengen mal 625/677,4 ein.
Zielgroessen nach KQ11 a: die Spitzen 132 / 277 / 341 / 476 Hz in Lage (5 %) und Hoehe (±6 dB), gerechnet am
Messpunkt der Quelle (Steg, zwischen Chor 1 und 2) mit der Last der gedaempften Saiten (2,47 N s/m).
Herleitung: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S197_laute.md`.

Was woher kommt:
  Umriss, Rose      Rauwolf-Zeichnung (475 x 326,5 mm; Rose 110 mm, Mitte 287,4 mm ueber dem unteren Ende), mal Massstab
  Steg              Lage 89 mm ueber dem unteren Ende, 139 mm lang (Masszahlen der Zeichnung); Masse 7 g — Annahme
  Hohlraum          aus den sieben Querschnitten der Zeichnung 14,0 l, mal Massstab hoch drei
  Rose              zu 85 % offen gerechnet (das Schnitzwerk der Rose ist nicht vermessen) — Annahme
  Decke             1,8 mm — Annahme (die Zeichnung zeigt das Innere nicht)
  Balken            sieben Querbalken bis an die Muschel, 14 mm hoch (der zweite 9,8 mm), an den Enden 5 mm,
                    4,5 mm breit; der Balken durch die Rose an ihrem Rand geteilt — Annahme (uebliches Balkenbild)
  Holz              Sitka-Fichte wie bei der Oud (Abdallah u. a. 2026 Tab. 1) — Annahme
  Stegpunkte        je Chor einer, Taste = die leere Saite (8-choerig in G: Woodhouse & Lynch-Aird 2019 Tab. II);
                    Spanne der Saiten 122,5 mm (Masszahl am Steg der Zeichnung, dort fuer elf Choere) — Annahme
  Daempfung         alpha = 60 1/s (eta = alpha / 2 pi f) und Guete der Luft 12: kleinster Abstand der vier Hoehen zur gemessenen Kurve
                    (`…/proben/2026-11-13_S197_laute_daempfung.py`); ueber 500 Hz fortgeschrieben
"""
import json
import math
import pathlib
import sys

UMRISS = "projects/apps/LumiViz/docs/modellierung/gezupfte-saite/proben/2026-11-13_S197_laute_umriss.json"
QUELLE = ("Shape: J. Santa Maria Bouquet, measured drawing of the lute by Sixtus Rauwolf 1598 (CL.93), Musikmuseet "
          "Copenhagen, 2020, scaled by 625/677.4. Targets: J. Woodhouse, The acoustics of double decays, JASA 150 "
          "(2021), Fig. 1 (bridge admittance of an 8-course Renaissance lute, CC BY 4.0), read from the figure")

MASSSTAB = 625.0 / 677.4   # Mensur der gemessenen Laute durch die der Rauwolf-Laute (KQ12 a)
DICKE = 1.8                # mm, Annahme
BALKEN = [(128.0, 14.0), (190.0, 9.8), (228.0, 14.0), (287.4, 14.0), (347.0, 14.0), (392.0, 14.0), (432.0, 14.0)]  # mm am Original
BALKEN_ENDE = 5.0          # mm
BALKEN_BREITE = 4.5        # mm
ROSE_OFFEN = 0.85          # Anteil der Kreisflaeche, Annahme
VOLUMEN = 14.0             # l am Original (Querschnitte der Zeichnung)
STEG_Y = 89.0 + 6.0        # mm: Vorderkante 89 (Masszahl), Leiste 12 mm breit
STEG_LAENGE = 139.0        # mm (Masszahl)
STEG_BREITE = 12.0         # mm (Masszahl)
STEG_MASSE = 0.007         # kg, Annahme
STEG_DICHTE = 700.0        # kg/m3, Birnbaum, Annahme
SAITEN_SPANNE = 122.5      # mm (Masszahl)
LEERE_SAITEN = [(38, "D2"), (41, "F2"), (43, "G2"), (48, "C3"), (53, "F3"), (57, "A3"), (62, "D4"), (67, "G4")]


def baue(masse):
    s = MASSSTAB
    halb = [(p[0], (p[2] - p[1]) / 2.0) for p in masse["umriss"]]   # halbe Breite je Station (mm), symmetrisch

    def halbe_breite(y):
        for k in range(len(halb) - 1):
            if halb[k][0] <= y <= halb[k + 1][0]:
                t = (y - halb[k][0]) / (halb[k + 1][0] - halb[k][0])
                return halb[k][1] + t * (halb[k + 1][1] - halb[k][1])
        return 0.0

    def m(v):
        return round(v * s / 1000.0, 5)

    # Umriss: unten rund (feiner), dann alle 10 mm, oben am Halsansatz gerade
    stationen = [0.0, 2.5, 5.0, 7.5, 10.0, 15.0, 20.0, 30.0] + [float(v) for v in range(40, 470, 10)] + [472.5]
    rechts = [(max(halbe_breite(y), 0.0), y) for y in stationen]
    rechts[0] = (0.0, 0.0)
    oben = masse["laenge"]
    punkte = rechts + [(rechts[-1][0], oben), (-rechts[-1][0], oben)] + [(-x, y) for x, y in reversed(rechts[1:])]
    umriss = [[m(x), m(y)] for x, y in punkte]

    rose_y, rose_r = masse["rose"]["x"], masse["rose"]["durchmesser"] / 2.0
    loch_r = rose_r * math.sqrt(ROSE_OFFEN)
    ecken = 48
    rr = loch_r * math.sqrt(2.0 * math.pi / (ecken * math.sin(2.0 * math.pi / ecken)))  # Vieleck mit der Kreisflaeche
    loch = [[round(rr * math.cos(2 * math.pi * k / ecken) * s / 1000, 6),
             round((rose_y + rr * math.sin(2 * math.pi * k / ecken)) * s / 1000, 6)] for k in range(ecken)]

    def leiste(name, y, x0, x1, hoehe):
        return {"name": name, "material": "spruce",
                "line": [[m(x0), m(y)], [m((x0 + x1) / 2), m(y)], [m(x1), m(y)]],
                "height": [BALKEN_ENDE * s / 1000, hoehe * s / 1000, BALKEN_ENDE * s / 1000],
                "width": [BALKEN_BREITE * s / 1000] * 3}

    rippen = []
    for n, (y, hoehe) in enumerate(BALKEN, 1):
        b = halbe_breite(y) - 1.0             # bis an die Muschel (1 mm vor dem Umriss)
        if abs(y - rose_y) < loch_r + 2.0:    # schneidet die Rose: geteilt, 3 mm vor ihrem Rand endend
            a = math.sqrt(max((loch_r + 3.0) ** 2 - (y - rose_y) ** 2, 0.0))
            rippen.append(leiste(f"bar {n} bass", y, -b, -a, hoehe))
            rippen.append(leiste(f"bar {n} treble", y, a, b, hoehe))
        else:
            rippen.append(leiste(f"bar {n}", y, -b, b, hoehe))

    steg_hoehe = STEG_MASSE / (STEG_DICHTE * STEG_LAENGE / 1000 * STEG_BREITE / 1000)
    abstand = SAITEN_SPANNE / (len(LEERE_SAITEN) - 1)
    mitte = (len(LEERE_SAITEN) - 1) / 2.0
    return {
        "format": "lumiviz-instrument-body-plan",
        "version": 7,
        "title": "Lute",
        "description": "Substitute plan of a Renaissance lute soundboard with its air cavity (KQ6 a, KQ11 a, KQ12 a). "
                       "Outline, rose, bridge position and length and the volume of the bowl from the measured drawing "
                       "of the Rauwolf lute of 1598, all lengths scaled by 625/677.4 to the string length of the lute "
                       "whose bridge admittance was measured (Woodhouse 2021 Fig. 1). Assumptions: plate thickness "
                       "1.8 mm, seven transverse bars reaching the bowl, rose 85 % open, bridge bar of 7 g, wood, rim "
                       "clamped and bowl rigid, spacing of the courses. Set so that the computed bridge admittance "
                       "between courses 1 and 2, loaded with the damped strings (2.47 N s/m), meets the measured peaks "
                       "at 132 / 277 / 341 / 476 Hz within 5 % and 6 dB; the measured peaks at 150 and 378 Hz have no "
                       "counterpart.",
        "source": QUELLE,
        "units": "SI: m, kg/m^3, Pa",
        "materials": [
            {"name": "spruce", "kind": "orthotropic", "density": 460.0,
             "ex": 10.34e9, "ey": 0.80e9, "gxy": 0.63e9, "gxz": 0.66e9, "gyz": 0.03e9, "nuxy": 0.467, "lossFactor": 0.0,
             "source": "assumed: Sitka spruce as in Abdallah et al. 2026 Table 1 (the wood of the lute is not measured)"},
            {"name": "pear", "kind": "orthotropic", "density": STEG_DICHTE,
             "ex": 8.0e9, "ey": 1.0e9, "gxy": 0.8e9, "gxz": 0.8e9, "gyz": 0.3e9, "nuxy": 0.35, "lossFactor": 0.0,
             "source": "assumed: pearwood, round values"}],
        "plate": {"material": "spruce", "grainAngle": 90.0, "outline": umriss, "thickness": DICKE / 1000, "holes": [loch]},
        "ribs": rippen,
        "bridges": [{"name": "bridge", "material": "pear",
                     "line": [[m(-STEG_LAENGE / 2), m(STEG_Y)], [m(STEG_LAENGE / 2), m(STEG_Y)]],
                     "height": [round(steg_hoehe, 6)] * 2, "width": [STEG_BREITE * s / 1000] * 2}],
        "bridgePoints": [{"key": taste, "x": m((k - mitte) * abstand), "y": m(STEG_Y),
                          "source": f"course {len(LEERE_SAITEN) - k} ({name}, open string; 8-course lute in G, Woodhouse & "
                                    "Lynch-Aird 2019 Table II); bridge 89 mm above the lower end and string span 122.5 mm "
                                    "(Rauwolf drawing, scaled; the span is that of its eleven courses - assumed)"}
                         for k, (taste, name) in enumerate(LEERE_SAITEN)],
        "listeningPoints": [{"name": "front", "x": 0.0, "y": 0.25, "z": 0.5, "source": "assumed: 0.5 m in front of the soundboard"}],
        "backside": {"rimDepth": 0.0, "shadowHz": 0.0, "source": "off: the soundboard closes the bowl"},
        "damping": {"a": 0.0, "b": 0.0, "c": 60.0,
                    "source": "least distance of the four peak heights to the measured bridge admittance (Woodhouse 2021 "
                              "Fig. 1, read from the figure), computed with the load of the damped strings; "
                              "extrapolated above 500 Hz"},
        "cavity": {"volume": round(VOLUMEN / 1000 * s ** 3, 6), "endCorrection": 1.7, "q": 12.0,
                   "source": "volume from the seven cross-sections of the bowl in the Rauwolf drawing (14.0 l), scaled; "
                             "rose computed as a hole of 85 % of its circle (assumed); end correction of a hole in a "
                             "thin wide wall assumed; Q of the air from the height of the measured peak at 132 Hz",
                   "openings": []},
        "rim": {"support": "clamped", "rotationalStiffness": 0.0, "source": "assumed: rim clamped (every plan up to version 5 was computed this way)", "sections": []},
    }


if __name__ == "__main__":
    args = sys.argv[1:]
    umriss = args[args.index("--umriss") + 1] if "--umriss" in args else UMRISS
    ziel = pathlib.Path(args[args.index("--ziel") + 1]) if "--ziel" in args else pathlib.Path(__file__).resolve().parent
    plan = baue(json.load(open(umriss, encoding="utf-8-sig")))
    ziel.mkdir(parents=True, exist_ok=True)
    datei = ziel / "lute.lvinstbodyplan"
    datei.write_text(json.dumps(plan, indent=1) + "\n", encoding="utf-8", newline="\n")
    print(datei, f"(cavity {plan['cavity']['volume'] * 1000:.2f} l, {len(plan['ribs'])} bars)")

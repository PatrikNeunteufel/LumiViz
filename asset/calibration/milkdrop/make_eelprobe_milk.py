# -*- coding: utf-8 -*-
"""Prüfstand: liefert JEDE ns-eel2-Funktion in LumiViz denselben Wert wie in MilkDrop? (S74)

Aufruf:
  python make_eelprobe_milk.py <zielordner>

Danach je Probe EIN Prozess auf beiden Seiten (Presets erben sonst das Bild des
Vorgängers — Kalibrier-Plan §0.1):

  MilkdropStandalone <probe.milk> --auto --frames 30 --size 320x240 --out <lumi>
  MilkdropRef        <probe.milk> --frames 30 --size 320x240 --out <ref>

Gegenstück zu `asset/calibration/avs/make_eelprobe_avs.py` für den AVS-Zweig.

Warum die Bauform anders ist als bei AVS
----------------------------------------
Bei AVS trägt ein SuperScope den Wert über 320 Abtaststellen ins Bild. MilkDrop
hat kein Gegenstück dazu: der Per-Pixel-Code steuert die Verzerrung, nicht die
Farbe. Was hier bleibt, sind die **Rahmenfarben** (`ob_*` außen, `ib_*` innen) —
zwei solide Flächen, deren Farbe der Per-Frame-Code setzt.

Daraus folgt: **drei Abtaststellen je Datei** (die drei Kanäle des äußeren
Rahmens), 8 Bit je Kanal. Je Funktion werden deshalb ZWEI Dateien erzeugt
(`_a` und `_b`) — zusammen sechs Stellen. Grob genug, um einen falschen
Funktionswert zu finden; zu grob, um Rundungsfragen zu klären. Für Letzteres
ist der AVS-Zweig da — die Funktionen sind dieselben Bibliotheksaufrufe.

**Nur der ÄUSSERE Rahmen wird benutzt** (`ob_size = 0.5`, füllt die Fläche).
Der innere war der erste Versuch und ist als Messmittel durchgefallen: die
Referenz füllt den Bereich mit einem konstanten Rot (0,78/0/0), das bei
verschiedenen Presets gleich bleibt und damit kein berechneter Wert ist
(gemessen S74). Der äußere Rahmen dagegen stimmt auf 8-Bit-Rundung genau —
Regel 1: nur mit dem abgenommenen Teil messen.

Zeitunabhängig gebaut (feste Argumente, kein `time`): `MilkdropRef` misst die
WANDUHR, ein zeitabhängiger Wert wäre zwischen den Seiten nie vergleichbar
(§9 der MilkDrop-Notizen).

Neutral gestellt sind Gamma, Brighten/Darken, Solarize, Invert, Echo und
Wellen — der Rahmen soll unverfälscht durchkommen.
"""
import argparse
from pathlib import Path

# Je Eintrag: (Ausdruck mit `a` als Argument, Normierung auf 0..1, Bemerkung)
# Sechs Argumente a1..a6 tasten den Definitionsbereich ab.
PROBEN = {
    "sin":     ("sin(a)",        "(r+1)/2",             ""),
    "cos":     ("cos(a)",        "(r+1)/2",             ""),
    "tan":     ("tan(a*0.4)",    "(r+2)/4",             "Pole gemieden"),
    "asin":    ("asin(a*0.3)",   "(r+1.5708)/3.14159",  ""),
    "acos":    ("acos(a*0.3)",   "r/3.14159",           ""),
    "atan":    ("atan(a)",       "(r+1.5708)/3.14159",  ""),
    "atan2":   ("atan2(a,0.5)",  "(r+3.14159)/6.28318", ""),
    "sqr":     ("sqr(a*0.5)",    "r/4",                 ""),
    "sqrt":    ("sqrt(abs(a))",  "r/2",                 ""),
    "pow":     ("pow(abs(a),3)", "r/8",                 ""),
    "exp":     ("exp(a*0.5)",    "r/8",                 ""),
    "log":     ("log(abs(a)+0.1)", "(r+2.303)/4.606",   ""),
    "log10":   ("log10(abs(a)+0.1)", "(r+1)/2",         ""),
    "abs":     ("abs(a)",        "r/3",                 ""),
    "min":     ("min(a,0.5)",    "(r+3)/6",             ""),
    "max":     ("max(a,0.5)",    "(r+3)/6",             ""),
    "sigmoid": ("sigmoid(a,1)",  "(r+1)/2",             ""),
    "sign":    ("sign(a)",       "(r+1)/2",             ""),
    "band":    ("band(a,1)",     "r",                   ""),
    "bor":     ("bor(a,0)",      "r",                   ""),
    "floor":   ("floor(a)",      "(r+3)/6",             ""),
    "ceil":    ("ceil(a)",       "(r+3)/6",             ""),
    "invsqrt": ("invsqrt(abs(a)+0.25)", "r/2",
                "NAEHERUNG — kleine Abweichung erwartet"),
    "exec2":   ("exec2(a,a*0.5)", "(r+1.5)/3",          ""),
    "exec3":   ("exec3(a,a*2,a*0.5)", "(r+1.5)/3",      ""),
    "rand":    ("rand(2)*0",     "r",
                "ZUFALL — auf 0 gezwungen; geprueft wird nur, dass beide laufen"),
    "megabuf": ("megabuf(8)",    "r",
                "Speicher: vorher beschrieben, s. Vorbereitung"),
    "gmegabuf": ("gmegabuf(8)",  "r",
                 "globaler Speicher"),
    # ns-eel2-eigen, in AVS-EEL nicht vorhanden
    "while":   ("q1",            "r/8",
                "NUR ns-eel2 — Schleife bis Bedingung 0"),
    "memset":  ("megabuf(20)",   "r",  "NUR ns-eel2"),
    "memcpy":  ("megabuf(28)",   "r",  "NUR ns-eel2 — liest, was memcpy kopiert hat"),
    "freembuf": ("megabuf(8)",   "r",
                 "NUR ns-eel2 — nach freembuf(0) muss der Wert noch stehen"),
}

# Sechs Abtaststellen, bewusst asymmetrisch (Vorzeichenfehler sollen auffallen),
# aufgeteilt auf zwei Dateien zu je drei Kanaelen
ARGUMENTE_A = [-2.5, -0.75, -0.1]
ARGUMENTE_B = [0.3, 1.4, 2.8]

KOPF = """MILKDROP_PRESET_VERSION=201
PSVERSION=2
PSVERSION_WARP=2
PSVERSION_COMP=2
[preset00]
fRating=5.000
fGammaAdj=1.000
fDecay=1.000
fVideoEchoZoom=1.000
fVideoEchoAlpha=0.000
nVideoEchoOrientation=0
nWaveMode=0
bAdditiveWaves=0
bWaveDots=0
bWaveThick=0
bModWaveAlphaByVolume=0
bMaximizeWaveColor=0
bTexWrap=0
bDarkenCenter=0
bRedBlueStereo=0
bBrighten=0
bDarken=0
bSolarize=0
bInvert=0
fWaveAlpha=0.000
fWaveScale=1.000
fWaveSmoothing=0.000
fWaveParam=0.000
fModWaveAlphaStart=0.710
fModWaveAlphaEnd=1.300
fWarpAnimSpeed=1.000
fWarpScale=1.000
fZoomExponent=1.00000
fShader=0.000
zoom=1.00000
rot=0.00000
cx=0.500
cy=0.500
dx=0.00000
dy=0.00000
warp=0.00000
sx=1.00000
sy=1.00000
wave_r=0.000
wave_g=0.000
wave_b=0.000
wave_x=0.500
wave_y=0.500
ob_size=0.500
ob_a=1.000
ib_size=0.000
ib_a=0.000
ib_r=0.000
ib_g=0.000
ib_b=0.000
nMotionVectorsX=0.000
nMotionVectorsY=0.000
mv_dx=0.000
mv_dy=0.000
mv_l=0.000
mv_r=0.000
mv_g=0.000
mv_b=0.000
mv_a=0.000
"""


def probe_text(ausdruck: str, norm: str, argumente) -> str:
    """Per-Frame-Code, der die Funktion an drei Stellen auswertet.

    Die drei Werte landen in den Kanaelen des aeusseren Rahmens, der die
    ganze Flaeche fuellt. Klemmung auf 0..1, damit ein Ausreisser nicht als
    schwarz/weiss verschwindet, sondern an der Grenze sichtbar bleibt.
    """
    zeilen = []
    n = 1

    def add(code: str):
        nonlocal n
        zeilen.append(f"per_frame_{n}={code}")
        n += 1

    # Speicher vorbereiten, damit megabuf/gmegabuf/memset/memcpy etwas vorfinden
    add("megabuf(8) = 0.375;")
    add("gmegabuf(8) = 0.625;")
    add("megabuf(9) = 0.375;")
    add("memset(20, 0.5, 4);")
    add("memcpy(28, 8, 4);")
    # KEIN Nachsetzen von megabuf(28)! Im ersten Anlauf stand hier
    # `megabuf(31) = megabuf(8)` — damit las die memcpy-Probe einen Wert, den
    # sie sich selbst hingelegt hatte, und meldete faelschlich OK (S74).
    # while-Zaehler (nur ns-eel2)
    add("freembuf(0);")
    add("q1 = 0;")
    add("while( exec2( q1 = q1 + 1, below(q1, 5) ) );")

    ziele = ["ob_r", "ob_g", "ob_b"]
    for ziel, a in zip(ziele, argumente):
        add(f"a = {a};")
        add(f"r = {ausdruck};")
        add(f"v = {norm};")
        add("v = if(below(v,0), 0, if(above(v,1), 1, v));")
        add(f"{ziel} = v;")
    return "\n".join(zeilen) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ziel", type=Path)
    args = ap.parse_args()
    args.ziel.mkdir(parents=True, exist_ok=True)

    zahl = 0
    for name, (ausdruck, norm, bemerkung) in sorted(PROBEN.items()):
        for suffix, argumente in (("a", ARGUMENTE_A), ("b", ARGUMENTE_B)):
            text = KOPF + probe_text(ausdruck, norm, argumente)
            (args.ziel / f"eel_{name}_{suffix}.milk").write_text(text, encoding="ascii")
            zahl += 1
        if bemerkung:
            print(f"  eel_{name:<10} {bemerkung}")
    print(f"{zahl} Funktions-Proben ({len(PROBEN)} Funktionen x 2) -> {args.ziel}")
    print("WICHTIG: ein Prozess je Preset — MilkDrop-Presets erben sonst das "
          "Bild des Vorgaengers (Kalibrier-Plan §0.1).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

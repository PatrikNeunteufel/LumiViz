# -*- coding: utf-8 -*-
"""Prüfstand: liefert JEDE EEL-Funktion in LumiViz denselben Wert wie in AVS? (S74)

Aufruf:
  python make_eelprobe_avs.py <zielordner>
  python compare_avsref.py <zielordner>/*.avs --frames 2 --size 320x240 --beat-period 30 --out <out>

Warum
-----
Ein einzelner falscher Funktionswert verstellt ein ganzes Preset, ohne dass man
ihm ansieht, woher es kommt. In S74 hat genau eine Funktion (`atan`) ein Preset
komplett schwarz gerendert — und zwar auf der REFERENZ-Seite, wegen eines
Linker-Schadens im JIT. Solche Fälle findet man nur, wenn man alle Funktionen
einzeln durchmisst, statt sie in Presets zu vermuten.

Abgedeckt sind alle 40 Funktionen, die AVS kennt: 33 aus der ns-eel-Kerntabelle
(`nseel-compiler.c` fnTable1) und 7, die AVS selbst registriert
(`avs_eelif.cpp:215-222`).

Verfahren
---------
Je Funktion ein SuperScope mit `n` Punkten, die von links nach rechts laufen —
jeder Punkt tastet die Funktion an einer anderen Stelle ihres Definitions-
bereichs ab. Ein Unterschied an IRGENDEINER Stelle wird damit sichtbar, nicht
nur an einem Stützpunkt.

Der Funktionswert wird auf 0..1 normiert und dann **über drei Kanäle** kodiert:

    red = v · green = Rest(v·256) · blue = Rest(v·65536)

Das ergibt rund 24 Bit statt der 8 Bit eines einzelnen Kanals — Auflösung etwa
1e-7 statt 1/255. Die Kodierung selbst benutzt nur `*`, `-` und `floor`, damit
sie nicht ihrerseits zum Messobjekt wird.

Nicht vergleichbar (mit Begründung im Eintrag): `rand` (Zufall),
`setmousepos` (Seiteneffekt ohne Rückgabe), `getkbmouse` (Eingabegerät).
Diese werden trotzdem erzeugt — sie müssen wenigstens ohne Absturz laufen.
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from make_raster_avs import SIG, superscope_entry   # noqa: E402

# Je Eintrag: (Vorbereitung, Ergebnis-Ausdruck, Normierung auf 0..1, Bemerkung)
# `i` laeuft 0..1 ueber die Punkte. `t` ist das Argument, `r` das Ergebnis.
PROBEN = {
    # --- Kerntabelle nseel-compiler.c fnTable1 -------------------------------------
    "sin":      ("t=i*6.28318-3.14159", "sin(t)",           "(r+1)/2",        ""),
    "cos":      ("t=i*6.28318-3.14159", "cos(t)",           "(r+1)/2",        ""),
    "tan":      ("t=i*2-1",             "tan(t)",           "(r+2)/4",        "Pole gemieden"),
    "asin":     ("t=i*2-1",             "asin(t)",          "(r+1.5708)/3.14159", ""),
    "acos":     ("t=i*2-1",             "acos(t)",          "r/3.14159",      ""),
    "atan":     ("t=i*20-10",           "atan(t)",          "(r+1.5708)/3.14159", "S74-Befund"),
    "atan2":    ("t=i*2-1",             "atan2(t,0.5)",     "(r+3.14159)/6.28318", ""),
    "sqr":      ("t=i*2-1",             "sqr(t)",           "r",              ""),
    "sqrt":     ("t=i*4",               "sqrt(t)",          "r/2",            ""),
    "pow":      ("t=i*2",               "pow(t,3)",         "r/8",            ""),
    "exp":      ("t=i*4-2",             "exp(t)",           "r/7.4",          ""),
    "log":      ("t=i*9.9+0.1",         "log(t)",           "(r+2.303)/4.606", "S74-Befund"),
    "log10":    ("t=i*99+1",            "log10(t)",         "r/2",            ""),
    "abs":      ("t=i*2-1",             "abs(t)",           "r",              ""),
    "min":      ("t=i*2-1",             "min(t,0.25)",      "(r+1)/2",        ""),
    "max":      ("t=i*2-1",             "max(t,0.25)",      "(r+1)/2",        ""),
    "sigmoid":  ("t=i*8-4",             "sigmoid(t,1)",     "(r+1)/2",        ""),
    "sign":     ("t=i*2-1",             "sign(t)",          "(r+1)/2",        ""),
    "band":     ("t=i*2-1",             "band(t,1)",        "r",              ""),
    "bor":      ("t=i*2-1",             "bor(t,0)",         "r",              ""),
    "bnot":     ("t=i*2-1",             "bnot(t)",          "r",              ""),
    "equal":    ("t=floor(i*4)/4",      "equal(t,0.5)",     "r",              ""),
    "below":    ("t=i*2-1",             "below(t,0.25)",    "r",              ""),
    "above":    ("t=i*2-1",             "above(t,0.25)",    "r",              ""),
    "floor":    ("t=i*10-5",            "floor(t)",         "(r+5)/10",       ""),
    "ceil":     ("t=i*10-5",            "ceil(t)",          "(r+5)/10",       ""),
    "invsqrt":  ("t=i*4+0.25",          "invsqrt(t)",       "r/2",
                 "NAEHERUNG (Bit-Trick + 1 Newton) — kleine Abweichung erwartet"),
    "if":       ("t=i*2-1",             "if(above(t,0),t,0-t)", "r",          ""),
    "assign":   ("t=i*2-1;q=0",         "assign(q,t)",      "(r+1)/2",        ""),
    # AVS-LEGALE Form: KEINE Zuweisung im Argument. Die erste Fassung nutzte
    # `exec2(q=t*2,q+1)` — das lehnt der AVS-Parser ab, die Referenz rechnete
    # gar nichts, und die Probe mass die Grammatik statt der Funktion (S74).
    "exec2":    ("t=i*2-1",             "exec2(t,t*0.5)",      "(r+1)/2",     ""),
    "exec3":    ("t=i*2-1",             "exec3(t,t*2,t*0.25)", "(r+1)/2",     ""),
    "loop":     ("t=i*2-1",             "exec2(loop(4,t),t)",  "(r+1)/2",
                 "NUR mit wirkungslosem Rumpf pruefbar: AVS laesst im "
                 "Schleifenrumpf keine Zuweisung zu, und ohne Zuweisung kann "
                 "`loop` nichts ansammeln — die Funktion ist in AVS-EEL "
                 "praktisch unbenutzbar"),
    "rand":     ("t=i",                 "rand(2)*0",        "r",
                 "ZUFALL — Wert nicht vergleichbar, hier auf 0 gezwungen; "
                 "geprueft wird nur, dass beide Seiten laufen"),
    # --- Von AVS registriert, avs_eelif.cpp:215-222 --------------------------------
    "getosc":   ("t=i",                 "getosc(t,0.1,0)",  "(r+1)/2",
                 "haengt am synthetischen Audio — beide Seiten erzeugen es gleich"),
    "getspec":  ("t=i",                 "getspec(t,0.1,0)", "r",
                 "haengt am synthetischen Audio"),
    "gettime":  ("t=0",                 "gettime(0)*0",     "r",
                 "WANDUHR — mit *0 neutralisiert; --tick-hz macht sie zur Frame-Uhr"),
    "getkbmouse": ("t=i",               "getkbmouse(1)",    "(r+1)/2",
                   "EINGABEGERAET — ohne Maus/Tastatur auf beiden Seiten 0"),
    "setmousepos": ("t=i",              "exec2(setmousepos(0,0),t)", "(r+1)/2",
                    "SEITENEFFEKT ohne sinnvollen Rueckgabewert — geprueft wird "
                    "nur, dass der Aufruf nicht stoert"),
    "megabuf":  ("t=i;q=0",             "exec2(assign(megabuf(8),t),megabuf(8))", "r",
                 "Speicherzugriff"),
    "gmegabuf": ("t=i;q=0",             "exec2(assign(gmegabuf(8),t),gmegabuf(8))", "r",
                 "globaler Speicher"),
}


def probe_code(vorbereitung: str, ausdruck: str, norm: str, punkte: int) -> tuple:
    """SuperScope, der die Funktion ueber die Bildbreite abtastet.

    Waagrechter Durchlauf (x von -1 nach 1, y fest), damit jede Spalte eine
    andere Stelle des Definitionsbereichs zeigt. Die Farbe traegt den Wert.
    """
    init = f"n={punkte}"
    point = (f"{vorbereitung};"
             f"r={ausdruck};"
             f"v={norm};"
             "v=if(below(v,0),0,if(above(v,1),1,v));"
             "x=i*2-1;y=0;"
             "red=v;"
             "g2=v*256;green=g2-floor(g2);"
             "b2=v*65536;blue=b2-floor(b2)")
    return point, "", "", init


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ziel", type=Path)
    ap.add_argument("--punkte", type=int, default=320,
                    help="Abtaststellen je Funktion (Vorgabe 320 = eine je Spalte)")
    args = ap.parse_args()

    args.ziel.mkdir(parents=True, exist_ok=True)
    for name, (vorb, ausdruck, norm, bemerkung) in sorted(PROBEN.items()):
        point, frame, beat, init = probe_code(vorb, ausdruck, norm, args.punkte)
        entry = superscope_entry(point, frame, beat, init, drawmode=0)  # Punkte
        (args.ziel / f"eel_{name}.avs").write_bytes(SIG + b"\x00" + entry)
        if bemerkung:
            print(f"  eel_{name:<12} {bemerkung}")
    print(f"{len(PROBEN)} Funktions-Proben -> {args.ziel}")
    print("Erwartung: alle ~0 gegen AvsRef. Ausnahmen sind oben benannt.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

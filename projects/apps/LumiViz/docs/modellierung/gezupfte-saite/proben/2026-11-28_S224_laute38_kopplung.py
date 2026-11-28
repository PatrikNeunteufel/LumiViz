"""S224, nach VL7 a: Wie stark haengt der spaete Pegel von laute_38 an der Stimmung zwischen den Saiten des Chors?

Befund im Test "Schleife gegen den Bezug": laute_38 liegt im Band 1-3 s 0,64 dB unter dem Bezug (Grenze 0,5 dB), die
Teiltoene der Schleife treffen den Bezug auf <= 0,19 Cent. Der Bezug selbst stieg durch VL2-VL7 dort um 0,83 dB, bei
Verschiebungen der Teiltoene um Hundertstel bis gut ein Cent. Laute 38 ist Chor 1 mit Oktavsaite: Teilton 1 der
Oktavsaite liegt auf Teilton 2 der Grundsaite, beide koppeln ueber den Steg.

Probe: im Bezug die eine Saite um delta Cent verstimmt (Zug mal 2^(2 delta / 1200)), sonst alles wie `rechne`.
Ausgabe: je delta der Pegel der Kraft im Band (0-0,1 / 0-1 / 1-3 s) und die spaeten Pegel der Teiltoene 1-6.

Ergebnis (zuerst mit dem Fehler unten gerechnet, die .txt mit dem berichtigten Bezug): die Verstimmung traegt es nicht —
0,04 Cent aendern das Band 1-3 s um <= 0,01 dB, 1 Cent um 0,05 dB (Teilton 2 spaet allerdings 0,1 dB je 0,04 Cent, die
Oktave des Chors). Die Ursache lag im Bezug: ohne VL7 a war er der alte (Teilton 1 spaet -31,23 dB, die Schleife -31,24),
mit VL7 a -30,75 dB, obwohl Verlust und Laufzeit auf der reellen Achse um <= 0,15 % gingen. `wellen_kausal` nahm dk an
|Re w| und fuegte es zu k'(T', EI') auf der Geraden w - i s0; dort fehlte -i s0 d(dk)/dw gegen den Anteil des
angepassten Zugs (Laute 38 Saite 0: T'/T 1,041, die Steigung von k 2 % kleiner, s0 * 2,2e-4 = 3e-4 gegen Im k 4,6e-3).
Berichtigt (S224): dk + i Im k auf der Geraden aus dem gedaempften Cepstrum (`kausal_verlust_auf`, im Kern
`umlauf::kausalerVerlustAuf`) — laute_38 Teilton 1 spaet -31,24 dB, das Band 1-3 s gegen die Schleife 0,025 statt 0,64 dB.

Aufruf: python -X utf8 -B 2026-11-28_S224_laute38_kopplung.py  (schreibt die .txt daneben)
"""
import importlib.util
import json
import pathlib

HIER = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("bz", HIER / "2026-11-26_S222_bezug_zupf.py")
bz = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bz)
ps = bz.ps

ORIGINAL = bz.eingaben


def verstimmt(saite, delta):
    def eingaben(instrument, taste, stufe):
        e = list(ORIGINAL(instrument, taste, stufe))
        saiten = [dict(s) for s in e[0]]
        saiten[saite]["T"] = saiten[saite]["T"] * 2 ** (2 * delta / 1200)
        e[0] = saiten
        return tuple(e)
    return eingaben


def main():
    w = ps.frequenzen()
    koerper = bz.KOERPER["laute"]
    kp = ps.lade_koerper(koerper)
    roh = json.loads((ps.KOERPER / f"{koerper}.lvinstbody").read_text(encoding="utf-8"))
    zeilen = ["# laute_38: eine Saite des Chors um delta Cent verstimmt (Zug), Bezug sonst wie 2026-11-26_S222_bezug_zupf.py",
              "# saite delta | kraftband 0-0,1 / 0-1 / 1-3 s dB | Teiltoene 1-6: Hz und spaet dB"]
    for saite, delta in [(0, 0.0), (0, 0.04), (0, -0.04), (0, 0.2), (0, -0.2), (0, 1.0), (1, 0.04), (1, -0.04)]:
        bz.eingaben = verstimmt(saite, delta)
        aus = []
        bz.rechne("laute_38", "laute", 38, 0.0, kp, roh, w, aus)
        band = next(z for z in aus if z.startswith("kraftband")).split()[1:]
        tt = [z.split() for z in aus if z.startswith("teilton")][:6]
        zeilen.append(f"{saite} {delta:+.2f} | " + " / ".join(f"{float(x):.3f}" for x in band) + " | "
                      + "  ".join(f"{float(t[2]):.3f} Hz {float(t[4]):.2f}" for t in tt))
        print(zeilen[-1], flush=True)
    bz.eingaben = ORIGINAL
    (HIER / "2026-11-28_S224_laute38_kopplung.txt").write_text("\n".join(zeilen) + "\n", encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()

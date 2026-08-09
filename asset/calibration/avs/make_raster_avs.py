# -*- coding: utf-8 -*-
"""Erzeugt ein Kalibrier-RASTER als echtes .avs (S74, Vorgabe Patrik).

Aufruf:
  python make_raster_avs.py raster.avs [--gitter 32]

Warum das gebraucht wird
------------------------
Transform-Knoten (Movement, Dynamic Movement, Blur, Fadeout, Mosaic, …) lassen
sich EINZELN nicht messen: allein auf der schwarzen Startflaeche liefern beide
Renderer schwarz, und der Vergleich meldet 0,000 — „kein Unterschied" heisst
dort nur „nichts zu sehen". Beim Einzelknoten-Lauf von S74 betraf das die
Mehrzahl aller Knoten.

Ein Transform braucht eine Quelle. Diese hier ist bewusst KEIN Debug-Overlay
des Hosts, sondern ein normaler SuperScope im Preset — nur so zeichnet die
Referenz (`AvsRef`) genau dasselbe Raster, und nur dann ist der Vergleich
gueltig.

Was das Raster kann
-------------------
Ein Punktgitter, dessen FARBE die Herkunft verraet: rot = Spaltenanteil,
gruen = Zeilenanteil, blau fest. Ein verschobener Punkt sagt damit nicht nur
„hier ist etwas", sondern auch „ich komme von dort" — Verschiebung, Drehung
und Spiegelung sind am Farbverlauf direkt ablesbar, ohne Differenzbild.

Bewusst nur Grundrechenarten und `%` im Skript: das Raster ist Messmittel,
nicht Messobjekt. Je weniger Sprachumfang es braucht, desto weniger kann es
selbst schiefgehen. Ob es taugt, wird trotzdem GEMESSEN — das Raster allein
muss gegen die Referenz ~0 liefern, sonst ist es als Grundlage unbrauchbar.

Format (r_sscope.cpp load_config): Entry `[id=36][len][blob]`,
blob = 0x01 + 4 Laengen-Strings (point, frame, beat, init) + which_ch
+ num_colors + Farben + drawmode. Datei = Signatur + Wurzel-Modus-Byte + Entries.
"""
import argparse
import struct
from pathlib import Path

SIG = b"Nullsoft AVS Preset 0.2\x1a"
SSCOPE_ID = 36


def avs_string(text: str) -> bytes:
    """Laengen-praefixierter, NUL-terminierter String wie Reader::loadString."""
    raw = text.encode("ascii") + b"\x00"
    return struct.pack("<i", len(raw)) + raw


def superscope_entry(point: str, frame: str, beat: str, init: str,
                     which_ch: int = 1, drawmode: int = 1) -> bytes:
    """Ein SuperScope-Entry.

    drawmode Bit 0 GESETZT = Linien, geloescht = Punkte (am laufenden Paar
    nachgemessen, S74 — die Kommentarlage im Uebersetzer legt das Gegenteil
    nahe). Linien sind fuer einen Transform-Pruefstand ohnehin die bessere
    Wahl: eine verbogene Linie zeigt die Verzerrung als Form, ein verschobener
    Punkt nur als Ort.
    """
    blob = (b"\x01"
            + avs_string(point) + avs_string(frame)
            + avs_string(beat) + avs_string(init)
            + struct.pack("<i", which_ch)
            + struct.pack("<i", 1) + struct.pack("<i", 0x00FFFFFF)
            + struct.pack("<i", drawmode))
    return struct.pack("<i", SSCOPE_ID) + struct.pack("<i", len(blob)) + blob


def verlauf_code(zeilen: int):
    """Vollflaechiger Farbverlauf: je Zeile eine waagrechte Linie von Rand zu Rand.

    Deckt auf, was ein Punktgitter nicht kann: Farbfehler, Gamma, Saettigung,
    Blend-Modi — alles, was eine FLAECHE braucht. Zwei Punkte je Zeile, also
    sparsam (240 Zeilen = 480 Punkte).
    """
    init = f"n={zeilen * 2};z={zeilen}"
    frame = "k=-1"
    point = ("k=k+1;"
             "zi=(k-k%2)/2;"
             "x=(k%2)*2-1;"
             "y=zi/(z-1)*2-1;"
             "red=zi/(z-1);"
             "green=1-zi/(z-1);"
             "blue=0.5")
    return point, frame, "", init


def keil_code(stufen: int):
    """Graukeil in `stufen` Stufen — fuer Gamma, Clipping und Helligkeitsskalen.

    Bewusst ohne Farbe: ein Helligkeitsfehler ist in Graustufen ablesbar, in
    Buntwerten vermischt er sich mit Farbfehlern.
    """
    init = f"n={stufen * 2};z={stufen}"
    frame = "k=-1"
    point = ("k=k+1;"
             "zi=(k-k%2)/2;"
             "x=(k%2)*2-1;"
             "y=zi/(z-1)*2-1;"
             "red=zi/(z-1);"
             "green=zi/(z-1);"
             "blue=zi/(z-1)")
    return point, frame, "", init


def kreuz_code():
    """Fadenkreuz + Rahmen — Ursprung, Achsrichtung und Seitenverhaeltnis.

    Beantwortet die Fragen, an denen ich mich in S74 verrannt habe: liegt der
    Ursprung in der Mitte, zeigt y nach oben oder unten, und wird das Bild
    entzerrt? Zehn Punkte, ein Polygonzug.
    """
    init = "n=10"
    frame = ""
    point = ("k=if(equal(i,0),0,k+1);"
             "x=if(below(k,1),-1,if(below(k,2),1,if(below(k,3),1,"
             "if(below(k,4),-1,if(below(k,5),-1,if(below(k,6),1,"
             "if(below(k,7),0,if(below(k,8),0,if(below(k,9),0,0)))))))));"
             "y=if(below(k,1),-1,if(below(k,2),-1,if(below(k,3),1,"
             "if(below(k,4),1,if(below(k,5),-1,if(below(k,6),1,"
             "if(below(k,7),1,if(below(k,8),-1,if(below(k,9),0,0)))))))));"
             "red=1;green=1;blue=1")
    return point, frame, "", init


def raster_code(gitter: int):
    """Punktgitter gitter x gitter, Farbe kodiert die Herkunft.

    `k` zaehlt die Punkte des Frames durch (im Frame-Code auf -1 gesetzt).
    Spalte/Zeile ohne floor(): cx = k%g, cy = (k-cx)/g — beides exakt in
    Ganzzahlen, damit keine Rundungsfrage dazwischenkommt.
    """
    init = f"n={gitter * gitter};g={gitter}"
    frame = "k=-1"
    point = ("k=k+1;"
             "cx=k%g;"
             "cy=(k-cx)/g;"
             "x=cx/(g-1)*2-1;"
             "y=cy/(g-1)*2-1;"
             "red=cx/(g-1);"
             "green=cy/(g-1);"
             "blue=0.5")
    return point, frame, "", init


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("ziel", type=Path)
    ap.add_argument("--muster", default="gitter",
                    choices=["gitter", "verlauf", "keil", "kreuz"],
                    help="gitter = Punktgitter mit Herkunftsfarbe (Verschiebung, "
                         "Verzerrung) · verlauf = vollflaechiger Farbverlauf "
                         "(Farbe, Gamma, Blend) · keil = Graukeil (Helligkeit, "
                         "Clipping) · kreuz = Fadenkreuz + Rahmen (Ursprung, "
                         "Achsrichtung, Seitenverhaeltnis)")
    ap.add_argument("--gitter", type=int, default=32,
                    help="Punkte je Achse bzw. Zeilen/Stufen (Vorgabe 32)")
    ap.add_argument("--punkte", action="store_true",
                    help="Punkte statt Linien (drawmode 0) — wenn die "
                         "Linien-Verbinder am Zeilenende stoeren")
    args = ap.parse_args()

    if args.gitter < 2:
        print("FEHLER: --gitter braucht mindestens 2")
        return 2

    if args.muster == "gitter":
        point, frame, beat, init = raster_code(args.gitter)
        punkte = args.gitter * args.gitter
    elif args.muster == "verlauf":
        point, frame, beat, init = verlauf_code(args.gitter)
        punkte = args.gitter * 2
    elif args.muster == "keil":
        point, frame, beat, init = keil_code(args.gitter)
        punkte = args.gitter * 2
    else:
        point, frame, beat, init = kreuz_code()
        punkte = 10

    # Ein Messmittel darf nicht in einen bekannten Fehler laufen: LumiViz
    # klemmt die Punktzahl bei 4096, das Original erst bei 128*1024 (Befund
    # S74). Oberhalb misst man den Abbruch statt des Effekts.
    if punkte > 4096:
        print(f"WARNUNG: {punkte} Punkte — LumiViz klemmt bei 4096, das "
              f"Original erst bei 131072. Das Messmittel misst dann den "
              f"eigenen Abbruch mit (S74-Befund).")
    entry = superscope_entry(point, frame, beat, init,
                             drawmode=0 if args.punkte else 1)
    args.ziel.parent.mkdir(parents=True, exist_ok=True)
    args.ziel.write_bytes(SIG + b"\x00" + entry)
    print(f"{args.ziel.name}: Muster '{args.muster}', {punkte} Punkte, "
          f"{'Punkte' if args.punkte else 'Linien'}")
    print("PRUEFEN, bevor damit gemessen wird: das Raster ALLEIN muss gegen "
          "AvsRef ~0 liefern.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

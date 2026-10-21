"""Ein Beispiel-Loop zu den Spuren ueber der Zeit aus Stufe 28 (S141, Stufe 28f;
docs/audio/Spielweisen_Konzept.md SF5 d, SK1 a bis SK13 a).

    python asset/loop/make_spuren_loops.py

Schreibt `asset/loop/lanes_demo.lvloop` — „Slow Swell", ein ruhiges Stueck in C, in dem jede
Spur etwas zu tun hat:

- **Keys** (Piano): kurze, gebrochene Akkorde; das **Sustain**-Pedal haelt sie, je Takt
  getreten und kurz vor dem naechsten gewechselt.
- **Pad** (Warm Pad): liegende Akkorde mit einer **Expression**-Spur (zwei Bogen je Durchlauf)
  und dazu eine **Kurve am Clip**: der Clip blendet ueber die ersten zwei Takte ein (SK8 a).
- **Lead** (Lead): eine Melodie; **Vibrato** waechst in den langen Toenen, **Pitch** zieht den
  ersten Ton von unten an und laesst den letzten fallen.
- **Guitar** (Clean Guitar, Tonabnehmer): Volume-Swells — **Expression** steigt in jedem Ton
  von 0 an, ueber den Tonabnehmer (Stufe 28c-2b).
- **Bass** (Round Bass): Grundtoene, am Ende ein Fall ueber **Pitch** (auf *Pitch range* gekappt).

Fassung 12 mit allen Feldern, die das Programm schreibt (Fassung 11: `limiter`, `gain` je Kanal;
12: `lanes` je Instrument-Muster, `expression` je Clip) — eine Fassungsnummer ist eine Zusage
ueber die ganze Datei. Gemischt gegen die Pegel der Instrumente seit S139 (alle auf dem Bezug):
`instrument_pegel.py --loops` laesst die Datei, wie sie ist. Zwei Laeufe ergeben dieselbe Datei.
Alles nach Rechnung gesetzt, nicht nach Gehoer.
"""

from make_examples import HIER, als_text
from make_genres import Bau
from make_spielweisen_loops import saiten_muster

TAKT = 16  # Schritte je Takt (4/4, Sechzehntel)
SPUREN = ("sustain", "sostenuto", "expression", "vibrato", "pitch")


def setze_spuren(b, muster, **spuren):
    """Die Spuren eines Musters setzen: je Name `[[stelle, wert], …]`, Stellen in Schritten."""
    for m in b.patterns:
        if m["name"] == muster:
            assert set(spuren) <= set(SPUREN), spuren
            for name, punkte in spuren.items():
                # ein Punkt am Ende des Musters fiele beim Richten weg (Modell 28a)
                assert all(0 <= s < m["bars"] * TAKT for s, _ in punkte), (muster, name)
            m["lanes"] = {name: [list(p) for p in spuren.get(name, [])] for name in SPUREN}
            m["lanes"]["switches"] = {}
            return
    raise KeyError(muster)


def fertig(b, kurven):
    """Fassung 12 mit allen Feldern; `kurven`: je Muster die Kurve seines Clips (in Takten)."""
    loop = b.fertig()
    loop["version"] = 12
    loop["limiter"] = True
    for m in loop["patterns"]:
        if m["kind"] == "instrument":
            m.setdefault("lanes", dict({name: [] for name in SPUREN}, switches={}))
    for k in loop["channels"]:
        k["gain"] = 0.0
        for c in k["clips"]:
            c["expression"] = [list(p) for p in kurven.get(c["pattern"], [])]
    return loop


def lanes():
    b = Bau("Slow Swell", "Ambient", 84, 0.45)
    # die Akkorde: Cmaj7, Am7, Fmaj7, G — je ein Takt
    akkorde = [(48, (52, 55, 59)), (45, (52, 55, 60)), (41, (57, 60, 64)), (43, (55, 59, 62))]

    # Keys: gebrochen, kurz — das Pedal haelt sie; je Takt getreten, kurz vor dem naechsten gewechselt
    keys = [(TAKT * t + s, 2, h, 0.6) for t, (_, toene) in enumerate(akkorde)
            for s, h in zip((0, 2, 4, 6), (toene[0], toene[1], toene[2], toene[1] + 12))]
    b.noten("Keys", "piano", keys, 0.7, -0.15)
    setze_spuren(b, "Keys", sustain=[p for t in range(4) for p in ((TAKT * t + 0.5, 1.0), (TAKT * t + 15.0, 0.0))])

    # Pad: liegend, zwei Bogen Expression je Durchlauf; der Clip blendet ueber zwei Takte ein
    pad = [(TAKT * t, TAKT, h, 0.6) for t, (_, toene) in enumerate(akkorde) for h in toene]
    b.noten("Pad", "warm_pad", pad, 0.6, 0.2)
    setze_spuren(b, "Pad", expression=[(0, 0.35), (24, 1.0), (32, 0.5), (56, 1.0), (63, 0.7)])

    # Lead: Vibrato waechst in den langen Toenen, Pitch zieht den ersten an und laesst den letzten fallen
    melodie = [(0, 8, 67, 0.75), (8, 4, 64, 0.7), (12, 4, 62, 0.7), (16, 12, 64, 0.8), (28, 4, 60, 0.7),
               (32, 8, 65, 0.75), (40, 8, 64, 0.7), (48, 12, 62, 0.8), (60, 3, 59, 0.6)]
    b.noten("Lead", "lead", melodie, 0.55, 0.0)
    setze_spuren(b, "Lead",
                 vibrato=[(0, 0), (4, 35), (8, 0), (16, 0), (22, 45), (28, 0), (48, 0), (54, 50), (60, 0)],
                 pitch=[(0, -1.0), (1.5, 0.0), (60, 0.0), (62.5, -2.0)])

    # Guitar: Volume-Swells ueber den Tonabnehmer — in jedem Ton von 0 an
    gitarre = [(TAKT * t + s, 8, h, 0.7) for t, (_, toene) in enumerate(akkorde)
               for s, h in ((0, toene[2] + 12), (8, toene[1] + 12))]
    saiten_muster(b, "Guitar", "clean_guitar", gitarre, 0.6, -0.3)
    setze_spuren(b, "Guitar", expression=[p for s in range(0, 4 * TAKT, 8) for p in ((s, 0.0), (s + 5, 1.0), (s + 7.5, 0.0))])

    # Bass: Grundtoene, am Ende ein Fall (die Spur will 5 Halbtoene, der Bass hat 2)
    bass = [(TAKT * t + s, 6, g, 0.8) for t, (g, _) in enumerate(akkorde) for s in (0, 8)]
    b.noten("Bass", "round_bass", bass, 0.75, 0.0)
    setze_spuren(b, "Bass", pitch=[(56, 0.0), (62, -5.0)])

    beat = b.drums("Brush", [("Kick", "kit:kick", 0.35, 0.0, "x.........x....."),
                             ("Snare", "kit:snare", 0.25, 0.0, "....x.......x..."),
                             ("Hat", "kit:hat_closed", 0.2, 0.3, "x.x.x.x.x.x.x.xx")])
    for kanal, muster in (("Drums", beat), ("Keys", "Keys"), ("Pad", "Pad"), ("Lead", "Lead"), ("Guitar", "Guitar"),
                          ("Bass", "Bass")):
        b.kanal(kanal, (muster, 0, 2))
    return fertig(b, {"Pad": [(0.0, 0.2), (2.0, 1.0)]})


BEISPIELE = {"lanes_demo": lanes}


def main():
    for name, bau in BEISPIELE.items():
        (HIER / (name + ".lvloop")).write_text(als_text(bau()), encoding="utf-8", newline="\n")
    print(len(BEISPIELE), "Beispiel zu den Spuren")


if __name__ == "__main__":
    main()

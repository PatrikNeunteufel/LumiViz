# -*- coding: utf-8 -*-
"""Automatische Stichproben zum Sichttest-Protokoll S79/S80 (Teile B und D).

Was ein Mensch im Protokoll mit „Skript eintragen, hinsehen" prueft, rechnet
dieses Skript nach: es baut je Stichprobe eine kleine `.lvfx`-Kette, rendert
sie mit `AvsStandalone` (deterministisches Audio, fester Beat alle 30 Frames)
als Bildserie und urteilt an Zahlen. Zu jeder Stichprobe entsteht ein
Kontaktbogen (Bildserie nebeneinander, bei Vergleichen Variante ueber
Kontrolle) — das Urteil faellt an den Zahlen, der Bogen ist zum Ansehen.

Die UI-Seite (Palette, Drag & Drop, Editor-Farben — Teile A und C) pruefen die
Unit-Tests `test_PaletteUi.cpp`; dieses Skript deckt nur, was ein Bild braucht.

Aufruf:   python sichttest_auto.py [kennung ...]     # Vorgabe: alle
Ausgabe:  out/sichttest_auto/  (Ketten, Bilder, Kontaktboegen, report.md)

Grenze: das synthetische Signal ist kein Musikstueck. „Reagiert auf Bass"
heisst hier: das Bild aendert sich mit dem Signal, gegen eine Kontrolle ohne
Skript. Ob es SCHOEN reagiert, bleibt ein Urteil am Kontaktbogen.
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable

import numpy as np
from PIL import Image, ImageDraw

for _s in (sys.stdout, sys.stderr):
    if hasattr(_s, "reconfigure"):
        _s.reconfigure(encoding="utf-8", errors="replace")

HIER = Path(__file__).parent
REPO = (HIER / "../../..").resolve()
sys.path.insert(0, str(HIER.parent / "fields"))
sys.path.insert(0, str(HIER.parent))  # messlauf: Start ohne Konsole (S81)
import messlauf  # noqa: E402
import lvfx_lib as L  # noqa: E402

EXE = (REPO / "out/build/windows-ninja-release-clang/exec/AvsStandalone/bin/"
       "Release/AvsStandalone.exe")
TESTS = (REPO / "out/build/windows-ninja-release-clang/exec/LumiViz.UnitTests/bin/"
         "Release/LumiViz.UnitTests.exe")
OUT = REPO / "out/sichttest_auto"
SIZE = "320x240"
BEAT = 30  # Beat-Periode in Frames (wie die Feld-Sonden)

# Milkdrop-Knoten der blanken Pipeline — aus Patriks Kette S80 (`202609_g_milk`)
# ohne Shape: genau das, was „New Effect Chain + Milkdrop einfuegen" erzeugt.
_MILK_VORLAGE = REPO / "asset/effectchain/202609_g_milk.lvfx"


def milk_knoten(**preset_aenderung) -> dict:
    doc = json.loads(_MILK_VORLAGE.read_text(encoding="utf-8"))
    n = doc["root"]["children"][0]
    n["preset"]["preset"]["shapes"] = []
    n["preset"]["preset"].update(preset_aenderung)
    return n


# --------------------------------------------------------------------- Messen
def lade(p: Path) -> np.ndarray:
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.float64) / 255.0


def luma(img: np.ndarray) -> np.ndarray:
    return 0.2126 * img[..., 0] + 0.7152 * img[..., 1] + 0.0722 * img[..., 2]


def mae(a: np.ndarray, b: np.ndarray) -> float:
    return float(np.abs(a - b).mean())


# --------------------------------------------------------------------- Rendern
def rendern(name: str, doc: dict, frames: int, jede: int) -> list[Path]:
    """Kette schreiben, rendern, Bildserie (aufsteigend) zurueckgeben."""
    ordner = OUT / name
    ordner.mkdir(parents=True, exist_ok=True)
    for alt in ordner.glob("*.png"):
        alt.unlink()
    lvfx = ordner / f"{name}.lvfx"
    L.write(lvfx, doc)
    env = dict(os.environ, QT_ENABLE_HIGHDPI_SCALING="0")
    proc = messlauf.run(
        [str(EXE), str(lvfx), "--auto", "--frames", str(frames), "--save-every",
         str(jede), "--beat-period", str(BEAT), "--size", SIZE, "--out", str(ordner)],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=300, env=env)
    bilder = sorted(ordner.glob(f"{name}_lvfx_f*.png"))
    if not bilder:
        raise RuntimeError(f"keine Bilder fuer {name}\n{proc.stdout[-800:]}{proc.stderr[-800:]}")
    return bilder


def framenummer(p: Path) -> int:
    return int(p.stem.rsplit("_f", 1)[1])


def kontaktbogen(zeilen: list[tuple[str, list[Path]]], ziel: Path) -> None:
    """Je Zeile eine Bildserie; links der Zeilenname, oben die Framenummern."""
    if not zeilen:
        return
    w, h = Image.open(zeilen[0][1][0]).size
    k = 0.5
    tw, th = int(w * k), int(h * k)
    spalten = max(len(z[1]) for z in zeilen)
    rand = 90
    bogen = Image.new("RGB", (rand + spalten * (tw + 4), 16 + len(zeilen) * (th + 4)),
                      (40, 40, 40))
    d = ImageDraw.Draw(bogen)
    for i, p in enumerate(zeilen[0][1]):
        d.text((rand + i * (tw + 4), 2), f"f{framenummer(p)}", fill=(200, 200, 200))
    for r, (titel, bilder) in enumerate(zeilen):
        y = 16 + r * (th + 4)
        d.text((4, y + th // 2 - 6), titel, fill=(230, 230, 230))
        for i, p in enumerate(bilder):
            bogen.paste(Image.open(p).convert("RGB").resize((tw, th)),
                        (rand + i * (tw + 4), y))
    bogen.save(ziel)


# ------------------------------------------------------------------ Stichproben
@dataclass
class Ergebnis:
    kennung: str
    was: str
    ok: bool
    zahlen: str
    bogen: str = ""


@dataclass
class Stichprobe:
    kennung: str
    was: str
    pruefen: Callable[["Stichprobe"], Ergebnis]
    extra: dict = field(default_factory=dict)


def vergleich(kennung: str, was: str, variante: dict, kontrolle: dict,
              frames: int = 91, jede: int = 15, schwelle: float = 0.01) -> Ergebnis:
    """Variante gegen Kontrolle: wirkt das Skript? (MAE in mindestens einem Frame)"""
    bv = rendern(f"{kennung}_variante", variante, frames, jede)
    bk = rendern(f"{kennung}_kontrolle", kontrolle, frames, jede)
    werte = [mae(lade(a), lade(b)) for a, b in zip(bv, bk)]
    bogen = OUT / f"{kennung}.png"
    kontaktbogen([("Skript", bv), ("Kontrolle", bk)], bogen)
    return Ergebnis(kennung, was, max(werte) > schwelle,
                    "MAE je Frame " + " ".join(f"{x:.3f}" for x in werte), bogen.name)


def serie(kennung: str, doc: dict, frames: int, jede: int) -> tuple[list[Path], Path]:
    bilder = rendern(kennung, doc, frames, jede)
    bogen = OUT / f"{kennung}.png"
    kontaktbogen([(kennung, bilder)], bogen)
    return bilder, bogen


def b1(_: Stichprobe) -> Ergebnis:
    doc = L.chain(L.node("clear", "Clear", color=0, frameCode="color_r=sin(time)*0.5+0.5"))
    bilder, bogen = serie("B1", doc, 241, 20)
    mittel = [lade(p).reshape(-1, 3).mean(axis=0) for p in bilder]
    rot = [m[0] for m in mittel]
    gb = max(max(m[1], m[2]) for m in mittel)
    ok = (max(rot) - min(rot)) > 0.5 and gb < 0.01
    return Ergebnis("B1", "Clear: color_r pulsiert, G/B bleiben", ok,
                    f"Rot {min(rot):.2f}..{max(rot):.2f}, G/B max {gb:.3f}", bogen.name)


def b2(_: Stichprobe) -> Ergebnis:
    doc = L.chain(L.node("clear", "Clear", color=0,
                         frameCode="color_r=1; color_g=0; color_b=bass"))
    bilder, bogen = serie("B2", doc, 91, 5)
    mittel = [lade(p).reshape(-1, 3).mean(axis=0) for p in bilder]
    rot = min(m[0] for m in mittel)
    gruen = max(m[1] for m in mittel)
    blau = [m[2] for m in mittel]
    ok = rot > 0.99 and gruen < 0.01 and max(blau) - min(blau) > 0.05
    return Ergebnis("B2", "Clear: Rot fest, Blau folgt dem Bass", ok,
                    f"Rot min {rot:.2f}, Gruen max {gruen:.3f}, "
                    f"Blau {min(blau):.2f}..{max(blau):.2f}", bogen.name)


def b3(_: Stichprobe) -> Ergebnis:
    return vergleich("B3", "Fractal 2D: colorscale folgt dem Bass",
                     L.chain(L.node("fractal2D", frameCode="colorscale=1+bass*3")),
                     L.chain(L.node("fractal2D", frameCode="colorscale=1")))


def b4(_: Stichprobe) -> Ergebnis:
    return vergleich("B4", "Fractal 2D: colorphase springt am Beat",
                     L.chain(L.node("fractal2D", beatCode="colorphase=colorphase+0.25")),
                     L.chain(L.node("fractal2D")))


def b5(_: Stichprobe) -> Ergebnis:
    # `octaves` ist ganzzahlig. Das synthetische Signal liefert bass ≈ 0,16–0,27;
    # mit der Protokoll-Formel `2+bass*6` bleibt das stets unter 3, also bei 2
    # wie die Kontrolle (erster Lauf S80: MAE 0). Mit *20 landet es bei 5–7.
    return vergleich("B5", "Domain Warp: octaves folgt dem Bass",
                     L.chain(L.node("domainWarp", frameCode="octaves=2+bass*20")),
                     L.chain(L.node("domainWarp", frameCode="octaves=2")))


_SHADER = ("uniform float zoom;\n"
           "void mainImage(out vec4 fragColor, in vec2 fragCoord)\n{\n"
           "    vec2 uv = fragCoord / iResolution.xy * zoom;\n"
           "    fragColor = vec4(fract(uv * 4.0), 0.5, 1.0);\n}\n")


def b6(_: Stichprobe) -> Ergebnis:
    r = vergleich("B6", "Shadertoy: eigenes Uniform zoom folgt dem Bass",
                  L.chain(L.node("shadertoy", code=_SHADER, frameCode="zoom=1+bass*2")),
                  L.chain(L.node("shadertoy", code=_SHADER, frameCode="zoom=1")))
    # Ohne Skript steht zoom auf 0: uv = 0 -> das ganze Bild eine Farbe
    ohne = rendern("B6_ohne_skript", L.chain(L.node("shadertoy", code=_SHADER)), 31, 30)
    streu = float(lade(ohne[-1]).reshape(-1, 3).std(axis=0).max())
    r.ok = r.ok and streu < 0.01
    r.zahlen += f" · ohne Skript flach (Streuung {streu:.4f})"
    return r


def _hostgroup(**skript) -> dict:
    kind = L.node("superScope", "Kind", pointCount=128, colors=[0xFFFFFF],
                  pointCode="x=cos(i*6.28)*0.5; y=sin(i*6.28)*0.5; "
                            "red=1; green=1; blue=1", renderMode=1, lineWidth=6)
    return L.chain(L.node("clear", "Grund", color=0, blend=0),
                   # crossfadeSeconds=0: eine Gruppe blendet beim Laden sonst
                   # 2 s ein und mischt solange mit EIGENER Deckkraft —
                   # `outadjustalpha` greift erst danach (erster Lauf S80:
                   # Variante und Kontrolle bitgleich, beide nur Einblendung)
                   L.node("hostgroup", "Host Group", children=[kind],
                          blendOut=10, outAdjustAlpha=128, crossfadeSeconds=0,
                          **skript))


def b7_b8(_: Stichprobe) -> Ergebnis:
    bv = rendern("B78_variante", _hostgroup(beatCode="outadjustalpha=255",
                                            frameCode="outadjustalpha=outadjustalpha*0.9"),
                 61, 1)
    bk = rendern("B78_kontrolle", _hostgroup(), 61, 1)
    hv = [float(luma(lade(p)).max()) for p in bv]
    hk = [float(luma(lade(p)).max()) for p in bk]
    beat_i = int(np.argmax(hv))
    danach = hv[min(beat_i + 10, len(hv) - 1)]
    ok = hv[beat_i] > 0.9 and danach < hv[beat_i] * 0.6 and (max(hk) - min(hk)) < 0.02
    auswahl = list(range(max(0, beat_i - 2), min(len(bv), beat_i + 16), 3))
    bogen = OUT / "B78.png"
    kontaktbogen([("Skript", [bv[i] for i in auswahl]),
                  ("Kontrolle", [bk[i] for i in auswahl])], bogen)
    return Ergebnis("B7/B8", "Host Group: Beat blitzt auf 255, Frame blendet aus", ok,
                    f"Spitze {hv[beat_i]:.2f} bei f{framenummer(bv[beat_i])}, "
                    f"+10 Frames {danach:.2f}; Kontrolle {min(hk):.2f}..{max(hk):.2f}",
                    bogen.name)


def d1(_: Stichprobe) -> Ergebnis:
    bilder, bogen = serie("D1", L.chain(), 61, 30)
    hell = max(float(luma(lade(p)).max()) for p in bilder)
    return Ergebnis("D1", "Neue Kette: schwarz", hell < 0.01, f"Luma max {hell:.3f}",
                    bogen.name)


def d2(_: Stichprobe) -> Ergebnis:
    bilder, bogen = serie("D2", L.chain(milk_knoten()), 121, 30)
    hell = max(float(luma(lade(p)).max()) for p in bilder)
    return Ergebnis("D2", "Kette + Milkdrop: bleibt schwarz", hell < 0.01,
                    f"Luma max {hell:.3f}", bogen.name)


def d3(_: Stichprobe) -> Ergebnis:
    r = vergleich("D3", "Milkdrop: Basis-Wave an zeichnet, aus nicht",
                  L.chain(milk_knoten(basicWaveEnabled=True)),
                  L.chain(milk_knoten(basicWaveEnabled=False)), frames=61, jede=15)
    aus = rendern("D3_aus", L.chain(milk_knoten(basicWaveEnabled=False)), 61, 60)
    hell = float(luma(lade(aus[-1])).max())
    r.ok = r.ok and hell < 0.01
    r.zahlen += f" · aus: Luma max {hell:.3f}"
    return r


def d4(_: Stichprobe) -> Ergebnis:
    # Duenne Linien: der Bildmittelwert sagt wenig (MAE ~0,003), die Spitze alles.
    an = milk_knoten()
    an["debugMotionVectors"] = True
    bv = rendern("D4_an", L.chain(an), 31, 15)
    bk = rendern("D4_aus", L.chain(milk_knoten()), 31, 15)
    hv = max(float(luma(lade(p)).max()) for p in bv)
    hk = max(float(luma(lade(p)).max()) for p in bk)
    bogen = OUT / "D4.png"
    kontaktbogen([("an", bv), ("aus", bk)], bogen)
    return Ergebnis("D4", "Milkdrop: Motion-Vector-Overlay an/aus", hv > 0.5 and hk < 0.01,
                    f"Luma max an {hv:.2f}, aus {hk:.3f}", bogen.name)


def d7(_: Stichprobe) -> Ergebnis:
    """Keine Drift, kein Weichzeichnen — die Drift-Sonde als Kette (S80).

    Patriks Kette selbst taugt nicht als Messung: ihre Shape wandert gewollt
    mit dem Audio. Deshalb derselbe Knoten mit einer weissen Shape, die nur in
    den Frames 1-3 leuchtet (Zaehler ueber q1). Danach muss ihr Nachbild
    stehen: Schwerpunkt fest, Energie erhalten (vor dem Fix: 0,5 px je Frame
    nach oben links, -29 % bis Frame 120). Patriks Kette laeuft als Bogen mit.
    """
    knoten = milk_knoten(perFrame="cnt=cnt+1;\nq1=cnt;")
    vorlage = json.loads(_MILK_VORLAGE.read_text(encoding="utf-8"))
    shape = dict(vorlage["root"]["children"][0]["preset"]["preset"]["shapes"][0])
    shape.update(x=0.5, y=0.5, r=1, g=1, b=1, r2=1, g2=1, b2=1, a=1, a2=1, borderA=0,
                 frameCode="a=below(q1,3.5);\na2=a;")
    knoten["preset"]["preset"]["shapes"] = [shape]
    bilder = rendern("D7", L.chain(knoten), 121, 30)

    def mass(p: Path) -> tuple[float, float, float]:
        lum = luma(lade(p))
        s = float(lum.sum())
        ys, xs = np.mgrid[0:lum.shape[0], 0:lum.shape[1]]
        return s, float((lum * xs).sum() / s), float((lum * ys).sum() / s)

    s0, x0, y0 = mass(bilder[1])   # f31: Blitz vorbei
    s1, x1, y1 = mass(bilder[-1])  # f121
    weg = float(np.hypot(x1 - x0, y1 - y0))
    verlust = 1.0 - s1 / s0 if s0 > 0 else 1.0
    patrik, _ = serie("D7_patrik", vorlage, 121, 30)
    bogen = OUT / "D7.png"
    kontaktbogen([("Sonde", bilder), ("Patrik", patrik)], bogen)
    return Ergebnis("D7", "Keine Drift, kein Weichzeichnen (Shape-Blitz steht)",
                    s0 > 0 and weg <= 1.0 and verlust <= 0.05,
                    f"f31→f121: Wanderung {weg:.2f} px, Energieverlust {verlust * 100:.1f} %",
                    bogen.name)


def g1(_: Stichprobe) -> Ergebnis:
    """Gestaltung (S80): Richtungen von Linear-/Radial-Drift und Skript-Steuerung.

    Eine Shape blitzt rechts der Mitte auf (Frames 1-3); danach wandert ihr
    Nachbild je Einstellung 30 Frames lang. Gemessen am Schwerpunkt (Bild-y
    waechst nach unten).
    """
    vorlage = json.loads(_MILK_VORLAGE.read_text(encoding="utf-8"))
    shape0 = vorlage["root"]["children"][0]["preset"]["preset"]["shapes"][0]

    def kette(per_frame: str = "", **gestalt) -> dict:
        k = milk_knoten(perFrame="cnt=cnt+1;\nq1=cnt;\n" + per_frame)
        shape = dict(shape0)
        shape.update(x=0.75, y=0.5, r=1, g=1, b=1, r2=1, g2=1, b2=1, a=1, a2=1,
                     borderA=0, rad=0.05, frameCode="a=below(q1,3.5);\na2=a;")
        k["preset"]["preset"]["shapes"] = [shape]
        k["preset"]["preset"].update(gestalt)
        return L.chain(k)

    def weg(bilder: list[Path]) -> tuple[float, float]:
        def sp(p: Path) -> tuple[float, float]:
            lum = luma(lade(p))
            s = lum.sum()
            ys, xs = np.mgrid[0:lum.shape[0], 0:lum.shape[1]]
            return float((lum * xs).sum() / s), float((lum * ys).sum() / s)
        (x0, y0), (x1, y1) = sp(bilder[1]), sp(bilder[-1])
        return x1 - x0, y1 - y0

    lin = dict(gestaltLinAn=True, gestaltLinStaerke=1.0)
    rad = dict(gestaltRadAn=True, gestaltRadStaerke=1.0)
    # (Name, Einstellung, per_frame, Pruefung auf (dx, dy_bild))
    faelle = [
        ("aus", {}, "", lambda dx, dy: abs(dx) < 0.5 and abs(dy) < 0.5),
        ("lin 0°", dict(lin, gestaltLinRichtung=0.0), "", lambda dx, dy: dx > 25 and abs(dy) < 1),
        ("lin 90°", dict(lin, gestaltLinRichtung=90.0), "", lambda dx, dy: dy < -25 and abs(dx) < 1),
        ("rad 0°", dict(rad, gestaltRadWinkel=0.0), "", lambda dx, dy: dx > 25 and abs(dy) < 1),
        ("rad 90°", dict(rad, gestaltRadWinkel=90.0), "", lambda dx, dy: dy < -20 and dx < 0),
        ("rad 180°", dict(rad, gestaltRadWinkel=180.0), "", lambda dx, dy: dx < -25 and abs(dy) < 1),
        ("Skript π", dict(lin, gestaltLinRichtung=0.0), "drift_dir = 3.14159265;",
         lambda dx, dy: dx < -25 and abs(dy) < 1),
    ]
    zeilen, teile, ok = [], [], True
    for i, (name, g, pf, pruef) in enumerate(faelle):
        bilder = rendern(f"G1_{i}", kette(pf, **g), 41, 10)
        dx, dy = weg(bilder)
        gut = pruef(dx, dy)
        ok = ok and gut
        teile.append(f"{name}: {dx:+.0f}/{-dy:+.0f}{'' if gut else ' ❌'}")
        zeilen.append((name, bilder))
    bogen = OUT / "G1.png"
    kontaktbogen(zeilen, bogen)
    return Ergebnis("G1", "Gestaltung: Richtungen (Weg x/y in px, y nach oben)", ok,
                    " · ".join(teile), bogen.name)


STICHPROBEN = [
    Stichprobe("B1", "", b1), Stichprobe("B2", "", b2), Stichprobe("B3", "", b3),
    Stichprobe("B4", "", b4), Stichprobe("B5", "", b5), Stichprobe("B6", "", b6),
    Stichprobe("B7", "", b7_b8), Stichprobe("D1", "", d1), Stichprobe("D2", "", d2),
    Stichprobe("D3", "", d3), Stichprobe("D4", "", d4), Stichprobe("D7", "", d7),
    Stichprobe("G1", "", g1),
]


def ui_tests() -> list[Ergebnis]:
    """Die UI-Tests der Teile A und C (`test_PaletteUi`) — damit der Bericht
    den GANZEN automatischen Stand zeigt, nicht nur die Bild-Stichproben.
    Screenshots der Widgets landen dabei in `OUT/ui/`."""
    import re
    import xml.etree.ElementTree as ET
    if not TESTS.exists():
        return [Ergebnis("A/C", "(UI-Tests)", False, f"{TESTS.name} fehlt — bauen")]
    xml = OUT / "ui_tests.xml"
    env = dict(os.environ, LUMIVIZ_UI_SCHNAPPSCHUSS=str(OUT / "ui"))
    messlauf.run([str(TESTS), "-tc=PaletteUi*", "-r=junit", f"--out={xml}"],
                   capture_output=True, text=True, encoding="utf-8", errors="replace",
                   timeout=600, env=env)
    if not xml.exists():
        return [Ergebnis("A/C", "(UI-Tests)", False, "kein Testbericht entstanden")]
    out = []
    for tc in ET.parse(xml).getroot().iter("testcase"):
        name = tc.get("name", "")
        # „PaletteUi A12: …" = Kettenbaum allein, „PaletteUi Panel A12: …" =
        # echtes Panel mit Host (zaehlt an der Kette) — beide behalten ihr Etikett
        m = re.match(r"PaletteUi ([^:]*):\s*(.*)", name)
        punkt, was = (m.group(1), m.group(2)) if m else (name, "")
        if not punkt.startswith("Panel") and punkt[:3] in ("A12", "A13", "A14", "A16"):
            punkt = "Baum " + punkt
        fehler = list(tc.iter("failure")) + list(tc.iter("error"))
        zahl = f"{len(fehler)} Fehler: " + fehler[0].get("message", "")[:120] if fehler else ""
        out.append(Ergebnis(punkt, was, not fehler, zahl))
    return out


def main() -> int:
    if not EXE.exists():
        print(f"FEHLER: {EXE} fehlt — Release-clang bauen")
        return 2
    wahl = {a.upper() for a in sys.argv[1:]}
    OUT.mkdir(parents=True, exist_ok=True)
    ui = [] if wahl else ui_tests()
    for e in ui:
        print(f"  {'OK  ' if e.ok else 'FEHL'} {e.kennung:6s} {e.was} {e.zahlen}", flush=True)
    ergebnisse = []
    for s in STICHPROBEN:
        if wahl and s.kennung not in wahl:
            continue
        try:
            e = s.pruefen(s)
        except Exception as ex:  # noqa: BLE001 — je Stichprobe weiter
            e = Ergebnis(s.kennung, "(Fehler)", False, str(ex).splitlines()[0])
        ergebnisse.append(e)
        print(f"  {'OK  ' if e.ok else 'FEHL'} {e.kennung:6s} {e.was} — {e.zahlen}", flush=True)
    with (OUT / "report.md").open("w", encoding="utf-8") as f:
        f.write("# Sichttest S79/S80 — automatischer Stand\n\n")
        f.write("Protokoll mit Handrest: `projects/apps/LumiViz/docs/"
                "Sichttest_Protokoll_S79.md`.\n\n")
        if ui:
            f.write(f"## Teile A und C — UI-Tests (`test_PaletteUi`)\n\n"
                    f"Offscreen, Palette und echtes MultiEffectPanel mit Host. "
                    f"Widget-Screenshots: `ui/`.\n\n")
            f.write("| Punkt | Was | Urteil | Meldung |\n|---|---|---|---|\n")
            for e in ui:
                f.write(f"| {e.kennung} | {e.was} | {'✅' if e.ok else '❌'} | {e.zahlen} |\n")
            f.write("\n")
        f.write("## Teile B und D — Ketten-Stichproben\n\n")
        f.write(f"`AvsStandalone`, {SIZE}, Beat alle {BEAT} Frames, synthetisches Signal.\n\n")
        f.write("| Punkt | Was | Urteil | Zahlen | Kontaktbogen |\n|---|---|---|---|---|\n")
        for e in ergebnisse:
            f.write(f"| {e.kennung} | {e.was} | {'✅' if e.ok else '❌'} | {e.zahlen} | "
                    f"{e.bogen} |\n")
    alle = ui + ergebnisse
    fehl = sum(not e.ok for e in alle)
    print(f"\n{len(alle) - fehl}/{len(alle)} bestanden "
          f"(UI {len(ui)}, Stichproben {len(ergebnisse)}) — {OUT / 'report.md'}")
    return 1 if fehl else 0


if __name__ == "__main__":
    sys.exit(main())

# -*- coding: utf-8 -*-
"""Wirkt ein Feld beim EDITIEREN genauso wie nach Speichern+Laden? (S55)

Verdacht Patrik: „der wirkliche Effekt wird erst sichtbar, wenn man das Preset
gespeichert hat und es wieder geladen wird" — bemerkt an Movement.

Der Mechanismus dahinter ist belegt, nicht vermutet:

  Laden      `loadChainFile` setzt `m_pendingRuntimeReset` -> `resetRuntimes()`;
             JEDER Knoten baut seine Runtime (Gitter, LUT, Skript-Traeger,
             Texturen) frisch auf.
  Editieren  Das Panel mutiert die Parameter und ruft `recompileChain()`, und
             das ist schlicht `compileChain(m_root)` — KEIN Runtime-Reset. Was
             ein Knoten nur beim Aufbau liest, bleibt auf dem alten Stand,
             solange sein Schnappschuss sich nicht aendert.

Gemessen wird deshalb dasselbe Feld auf zwei Wegen, mit identischer Lauflaenge:

  GELADEN   `<feld>.lvfx`                       — Wert steht von Anfang an drin
  EDITIERT  `_default.lvfx --edit-nach <feld>`  — Wert wird nach der halben
                                                  Lauflaenge gesetzt

Unterscheiden sich die beiden, ist das ALLEIN noch kein Befund: ein Effekt mit
Verlauf — fliegende Sterne, Trails, Puffer, laufende Rotation — muss abweichen,
weil die erste Haelfte des Laufs noch unter der Vorgabe gerechnet wurde. Der
erste Anlauf (S55) zaehlte deshalb 50 „Abweichungen", die fast alle keine
waren.

Die Frage laesst sich aber messen statt schaetzen, mit einem DRITTEN Bild:

  VORGABE   derselbe Lauf ohne Edit

Damit wird aus der Vermutung ein Urteil:

  WIRKUNGSLOS  editiert == Vorgabe, Pixel fuer Pixel. Der Edit hat NICHTS
               bewirkt — das ist der harte Befund, egal ob der Knoten einen
               Verlauf hat oder nicht.
  TEILWEISE    editiert liegt zwischen Vorgabe und geladen: der Edit wirkt,
               das Bild traegt aber noch die Vorgeschichte. Bei Effekten mit
               Verlauf ist das der Normalfall und in Ordnung.
  GLEICH       editiert == geladen.

Aufruf:
  python run_edit_probes.py [typkey ...] [--frames N] [--size WxH]
"""
from __future__ import annotations

import argparse
import sys
from datetime import datetime
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "avs"))

import compare_avsref as ca  # noqa: E402  (Pfad muss vorher stehen)

from run_field_probes import (ARGS_JE_FELD, FRAMES_JE_FELD,  # noqa: E402
                              FRAMES_JE_TYP)

HIER = Path(__file__).parent
PROBES = HIER / "probes"

# Knoten, die Zustand ueber die Frames mitschleppen. Bei ihnen ist ein
# Unterschied zwischen „geladen" und „editiert" KEIN Befund, sondern die
# zwangslaeufige Folge davon, dass die erste Haelfte des Laufs mit der Vorgabe
# gerechnet wurde. Wer hier trotzdem urteilen will, braucht eine Sonde, die den
# Verlauf zuruecksetzt — die gibt es nicht.
MIT_VERLAUF = {
    "multiDelay",        # Verzoegerungsring
    "videoDelay",
    "bufferSave",        # Puffer-Inhalt
    "blitterFeedback",   # Rueckkopplung
    "rotoBlitter",
    "waterBump", "water",
    "fyrewurX",          # Partikel
    "movingParticle",
    "bassSpin",
    "timescope",         # wandernde Spalte
    "avi",               # Frame-Index laeuft mit der Uhr
    "customBpm",
    "reactionDiffusion", "fractalZoomer",  # iterative Zustandsfelder
    "milkdrop",
}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("typkeys", nargs="*", help="nur diese Typen (Default: alle)")
    ap.add_argument("--frames", type=int, default=181)
    ap.add_argument("--size", default="320x240")
    ap.add_argument("--beat-period", type=int, default=30)
    ap.add_argument("--out", type=Path, default=HIER / "../../../out/edit_probes")
    args = ap.parse_args()

    typen = sorted(d for d in PROBES.iterdir() if d.is_dir()) if PROBES.exists() else []
    if args.typkeys:
        typen = [d for d in typen if d.name in args.typkeys]
    if not typen:
        print("FEHLER: keine Sonden — erst make_field_probes.py laufen lassen")
        return 2

    out: Path = args.out.resolve()
    (out / "bilder").mkdir(parents=True, exist_ok=True)
    (out / "montage").mkdir(exist_ok=True)

    def frames_fuer(typ: str, feld: str) -> int:
        return FRAMES_JE_FELD.get(f"{typ}.{feld}",
                                  FRAMES_JE_TYP.get(typ, args.frames))

    rows: list[tuple[str, str, float, str]] = []
    for tdir in typen:
        basis = tdir / "_default.lvfx"
        if not basis.exists():
            continue
        ziel = out / "bilder" / tdir.name
        ziel.mkdir(parents=True, exist_ok=True)
        for alt in ziel.glob("*.png"):
            alt.unlink()

        for sonde in sorted(tdir.glob("*.lvfx")):
            if sonde.name.startswith("_"):
                continue
            feld = sonde.stem
            n = frames_fuer(tdir.name, feld)
            extra = list(ARGS_JE_FELD.get(f"{tdir.name}.{feld}", []))
            # Ein eigener Vergleichsgrund heisst: das Feld wirkt nur in
            # Gesellschaft. Der Edit muss dann von DIESEM Grund ausgehen.
            grund = tdir / f"_grund_{feld}.lvfx"
            start = grund if grund.exists() else basis
            try:
                geladen = ca.load_rgb(ca.run_lumi(
                    sonde, n, args.size, ziel / f"{feld}_geladen",
                    args.beat_period, extra))
                editiert = ca.load_rgb(ca.run_lumi(
                    start, n, args.size, ziel / f"{feld}_editiert",
                    args.beat_period, extra + ["--edit-nach", str(sonde)]))
                mae = ca.compare(geladen, editiert)["mae"]
                if mae == 0.0:
                    urteil = "GLEICH"
                else:
                    # Das dritte Bild NUR wenn noetig — es kostet einen
                    # Renderlauf und beantwortet genau eine Frage: hat der Edit
                    # ueberhaupt etwas bewirkt?
                    vorgabe = ca.load_rgb(ca.run_lumi(
                        start, n, args.size, ziel / f"{feld}_vorgabe",
                        args.beat_period, extra))
                    if ca.compare(vorgabe, editiert)["mae"] == 0.0:
                        urteil = "WIRKUNGSLOS"
                        ca.montage(geladen, editiert,
                                   out / "montage" / f"{tdir.name}_{feld}.png")
                    else:
                        urteil = "TEILWEISE"
            except Exception as e:  # noqa: BLE001 — je Sonde weitermachen
                mae, urteil = 0.0, "FEHLER"
                print(f"  FEHLER  {tdir.name}.{feld}: {e}")
            rows.append((tdir.name, feld, mae, urteil))
            if urteil not in ("GLEICH", "TEILWEISE"):
                print(f"  {urteil:12s} {tdir.name}.{feld:24s} MAE {mae:.4f}")

    # Jeder Lauf wird ARCHIVIERT (s. run_field_probes.py, gleicher Grund):
    # `logs/<zeitstempel>_…md` bleibt stehen, `report.md` ist der letzte Stand,
    # und ein Teillauf traegt seine Typen im Namen.
    zahl = {u: sum(1 for r in rows if r[3] == u) for u in
            ("GLEICH", "TEILWEISE", "WIRKUNGSLOS", "FEHLER")}
    teil = "_".join(sorted(args.typkeys))[:60]
    name = f"report{'_' + teil if teil else ''}.md"
    stempel = datetime.now().strftime("%Y-%m-%d_%H%M%S")

    text = ("# Edit-Sonden (S55) — wirkt ein Feld beim Editieren wie nach "
            "dem Laden?\n\n"
            f"**Lauf:** {datetime.now():%Y-%m-%d %H:%M:%S} · "
            f"{args.frames} Frames, {args.size} · "
            f"`--beat-period {args.beat_period}` · "
            f"{'alle Typen' if not teil else 'nur ' + ', '.join(sorted(args.typkeys))}\n\n"
            "**Ergebnis:** "
            + " · ".join(f"{k} {v}" for k, v in zahl.items() if v) + "\n\n"
            "`GELADEN` = Wert steht im Preset · `EDITIERT` = Wert wird nach "
            "der halben Lauflaenge gesetzt (`--edit-nach`, bildet den "
            "Panel-Weg nach: Params tauschen + `recompileChain()`, KEIN "
            "Runtime-Reset).\n\n"
            "`WIRKUNGSLOS` = editiert ist Pixel fuer Pixel die Vorgabe, der "
            "Edit hat also nichts bewirkt (Befund). `TEILWEISE` = der Edit "
            "wirkt, das Bild traegt aber noch die Vorgeschichte — bei "
            "Effekten mit Verlauf der Normalfall.\n\n"
            "| Typ | Feld | MAE | Urteil |\n|---|---|---|---|\n"
            + "".join(f"| {typ} | {feld} | {mae:.4f} | {urteil} |\n"
                      for typ, feld, mae, urteil in rows))

    (out / "logs").mkdir(exist_ok=True)
    for ziel in (out / name, out / "logs" / f"{stempel}_{name}"):
        ziel.write_text(text, encoding="utf-8", newline="")

    print(f"\nReport: {out / name}")
    print(f"Log:    {out / 'logs' / f'{stempel}_{name}'}")
    print("  " + " · ".join(f"{k} {v}" for k, v in zahl.items() if v))
    return 0 if zahl["WIRKUNGSLOS"] == 0 and zahl["FEHLER"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

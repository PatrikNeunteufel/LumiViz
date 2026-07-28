# -*- coding: utf-8 -*-
"""Faellt das Strang-E-Urteil: wirkt jedes Feld? (Konzept §9)

Je Knotentyp wird `_default.lvfx` EINMAL gerendert und dann gegen jede
Feld-Sonde desselben Typs gehalten. Beide Bilder teilen Untergrund, Frameszahl,
Groesse und das synthetische Audio des Standalone — es unterscheidet sie also
nichts ausser dem einen Feld.

Das Urteil nutzt aus, dass zwei Laeufe desselben Presets BIT-IDENTISCH sind
(nachgemessen S54). Deshalb ist hier, anders als beim AvsRef-Vergleich, keine
Toleranz noetig:

  WIRKT    die Bilder unterscheiden sich deutlich
  SCHWACH  sie unterscheiden sich, aber kaum (MAE < 0,001) — ansehen: entweder
           ein zu zaghafter Gegenwert oder ein Feld mit Rand-Wirkung
  STUMM    Pixel fuer Pixel gleich. Das Feld kann so nicht wirken. Ob das am
           Feld liegt oder am Gegenwert, entscheidet die Montage — deshalb wird
           sie fuer JEDE stumme Sonde geschrieben.

Aufruf:
  python run_field_probes.py [typkey ...] [--frames N] [--size WxH] [--jobs N]

--------------------------------------------- Warum es zwei tote Schalter gibt
Beide Beschleunigungs-Ideen wurden gemessen und beide bringen nichts. Sie
bleiben als Schalter erhalten, damit niemand denselben Weg noch einmal geht,
ohne die Zahlen zu kennen.

`--verzeichnis` (alle Sonden eines Typs in EINEM Prozess): 31,1 s gegen 31,3 s
bei 97 Sonden — kein Unterschied. Die Rechnung dahinter war falsch: ein
EINZELNER, kalter Aufruf braucht 960 ms und wirkt wie hohe Fixkosten, im
laufenden Sweep sind die Qt- und GL-DLLs aber im Cache und ein Start kostet
real nur ~0,3 s. Nebenbei wichen 3 von 97 Zeilen in den Nachkommastellen ab
(kein anderes Urteil) — im selben Prozess nacheinander gerendert ist eben nicht
dasselbe wie frisch gestartet.

------------------------------------------------------ Warum `--jobs` 1 bleibt
Die Zeitbasis des Standalone ist frame-gebunden (`m_time += 1/60` je Frame,
main.cpp:189), parallele Laeufe waeren also grundsaetzlich zulaessig. Gemessen
(S54, 97 Sonden, AMD Radeon 610M) bringt es aber nichts:

| seriell | 4 gleichzeitig |
|---|---|
| 31 s | 61 s |

Die GPU ist der Engpass, nicht die Reihenfolge — mehrere GL-Kontexte bremsen
sich gegenseitig aus. Schlimmer: EINE der 97 Zeilen wich in der vierten
Nachkommastelle ab (0,0305 gegen 0,0304). Das Urteil blieb zwar gleich, aber
die Bit-Identitaet zweier Laeufe ist genau die Grundlage von „STUMM = MAE exakt
null". Wer `--jobs` erhoeht, muss beides neu messen: die Zeit UND ob derselbe
Report herauskommt.
"""
from __future__ import annotations

import argparse
import os
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "avs"))

import compare_avsref as ca  # noqa: E402  (Pfad muss vorher stehen)

HIER = Path(__file__).parent
PROBES = HIER / "probes"

# Lauflaenge je Typ, wo die Vorgabe nicht passt. Drei Sekunden sind gut fuer
# beat-gebundene Effekte und schlecht fuer AKKUMULIERENDE: Colorfade addiert je
# Frame auf die Kanaele, nach 181 Frames stehen sie am Anschlag und kein
# Fader-Wert macht mehr einen Unterschied (S54).
#
# Die Zahl muss weiter auf einem Beat enden — `(frames-1) % beat_period == 0`.
# Bei Periode 30 sind das 31, 61, 91, 121 …
FRAMES_JE_TYP: dict[str, int] = {
    "colorfade": 61,        # eine Sekunde, zwei Beats
    "fadeout": 61,          # dito: blendet sonst vollstaendig zur Zielfarbe
    "brightness": 61,
    "fastBrightness": 61,
}

# Lauflaenge je EINZELNER Sonde. Der Schlussframe ist sonst immer ein Beat —
# gut fuer beat-gebundene Felder, toedlich fuer ihre Gegenstuecke:
#
# Colorfade ersetzt im Beat-Fenster die normalen Fader durch die Beat-Fader.
# Im Beat-Frame sind `faderR/G/B` deshalb GRUNDSAETZLICH unsichtbar, egal was
# man einstellt — sie brauchen einen Schlussframe ohne Beat (60 Frames: der
# letzte Beat liegt auf Index 30, der Schlussframe ist 59). Das Fenster laesst
# sich nicht schliessen: `onBeatFrames = 0` hebt der Validator auf 1
# (EffectChain.hpp:2255). `onBeatFrames` selbst braucht umgekehrt einen
# Schlussframe INNERHALB des Fensters der Sonde (3) und ausserhalb dessen des
# Vergleichs (1) — Index 32, also zwei Frames nach dem Beat auf 30.
#
# Das Vergleichsbild wird mit derselben Zahl gerendert; die Bilder liegen je
# Lauflaenge in einem eigenen Unterordner, sonst uberschreiben sie einander.
#
# `avi.persist` ist dieselbe Bauart wie `colorfade.onBeatFrames`, nur mit
# anderen Zahlen: gemessen wird zehn Frames nach dem Beat auf Index 30 — das
# Fenster der Vorgabe (6) ist dann zu, das der Sonde (32) noch offen.
FRAMES_JE_FELD: dict[str, int] = {
    "colorfade.faderR": 60,
    "colorfade.faderG": 60,
    "colorfade.faderB": 60,
    "colorfade.onBeatFrames": 33,
    "avi.persist": 41,
}

# Sonden, die ein anderes TESTSIGNAL brauchen. Das Standard-Signal des
# Standalone fuellt beide Spektrumkanaele gleich — Absicht, denn an ihm haengen
# Matrix, Modul-Sonden und alle Feld-Sonden. Kanalfelder koennen daran aber
# grundsaetzlich nichts zeigen: links, rechts und Mitte sind zwangslaeufig
# identisch. `--stereo-spektrum` macht die Kanaele unterscheidbar, und zwar
# weiterhin deterministisch (nur eine andere Formel, kein echtes Material).
# Sonde UND Vergleichsbild laufen mit denselben Schaltern, die Bilder liegen
# in einem eigenen Ordner (s. bildordner()).
ARGS_JE_FELD: dict[str, list[str]] = {
    "timescope.channel": ["--stereo-spektrum"],
    "timescope.useChannel": ["--stereo-spektrum"],
}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("typkeys", nargs="*", help="nur diese Typen (Default: alle)")
    # 181 Frames = rund drei Sekunden bei 60 fps, mit `--beat-period 30` also
    # SECHS Beats — und der letzte gerenderte Frame ist selbst einer
    # ((181-1) % 30 == 0). Beides zusammen ist noetig (Vorgabe Patrik, S54):
    # ein beat-gebundener Effekt braucht Beats IM Lauf, und seine Wirkung muss
    # im Schlussbild stehen. Mit 40 Frames war `onBeatClear` sechsmal stumm —
    # der Knoten loeschte brav, nur wurde der Untergrund im Folgeframe wieder
    # darueber gezeichnet. Mit 181 wirken alle sechs Felder.
    ap.add_argument("--frames", type=int, default=181)
    ap.add_argument("--size", default="320x240")
    ap.add_argument("--beat-period", type=int, default=30)
    ap.add_argument("--out", type=Path, default=HIER / "../../../out/field_probes")
    ap.add_argument("--verzeichnis", action="store_true",
                    help="alle Sonden eines Typs in EINEM Prozess. Gemessen kein "
                         "Zeitgewinn (s. Kopf der Datei) — nur fuer Gegenproben")
    ap.add_argument("--jobs", type=int, default=1,
                    help="gleichzeitige Renderprozesse. Vorgabe 1 — MESSEN, bevor "
                         "man das erhoeht (s. Kopf der Datei)")
    args = ap.parse_args()

    if args.beat_period > 0 and (args.frames - 1) % args.beat_period != 0:
        naechste = ((args.frames - 1) // args.beat_period + 1) * args.beat_period + 1
        print(f"WARNUNG: der letzte Frame ({args.frames - 1}) ist kein Beat — "
              f"beat-gebundene Felder erscheinen dann stumm. "
              f"Passend waeren {naechste} Frames.")

    typen = sorted(d for d in PROBES.iterdir() if d.is_dir()) if PROBES.exists() else []
    if args.typkeys:
        typen = [d for d in typen if d.name in args.typkeys]
    if not typen:
        print("FEHLER: keine Sonden — erst make_field_probes.py laufen lassen")
        return 2

    out: Path = args.out.resolve()
    (out / "bilder").mkdir(parents=True, exist_ok=True)
    (out / "montage").mkdir(exist_ok=True)

    # ---------------------------------------------------------------- rendern
    # Alle Bilder ZUERST und PARALLEL. Das ist gefahrlos, weil die Zeitbasis des
    # Standalone frame-gebunden ist (`m_time += 1/60` je Frame, main.cpp:189) —
    # ein Lauf rechnet dieselben Bilder, egal wie schnell er drankommt. Nur
    # `customBpm.arbitrary` liest die echte Uhr; seine Sonde ist entsprechend
    # gekennzeichnet. Das Urteil selbst bleibt sequenziell, es ist billig.
    def frames_fuer(typ: str, feld: str | None = None) -> int:
        if feld is not None and f"{typ}.{feld}" in FRAMES_JE_FELD:
            return FRAMES_JE_FELD[f"{typ}.{feld}"]
        return FRAMES_JE_TYP.get(typ, args.frames)

    def args_fuer(typ: str, feld: str | None = None) -> list[str]:
        return ARGS_JE_FELD.get(f"{typ}.{feld}", []) if feld is not None else []

    def vergleichsdatei(tdir: Path, feld: str) -> Path:
        """Womit die Sonde gehalten wird: eigener Grund, sonst `_default`."""
        eigen = tdir / f"_grund_{feld}.lvfx"
        return eigen if eigen.exists() else tdir / "_default.lvfx"

    def bildordner(typ: str, n: int, args_: list[str]) -> str:
        """Ein Ordner je (Lauflaenge, Testsignal) — sonst ueberschreiben sich
        zwei Laeufe DERSELBEN Datei unter verschiedenen Bedingungen."""
        marke = "".join(a.lstrip("-")[:6] for a in args_)
        return f"f{n}{'_' + marke if marke else ''}"

    # Ein Auftrag ist (Preset, Lauflaenge) — dieselbe Datei kann unter ZWEI
    # Laengen gebraucht werden (`_default` als Vergleich fuer eine Sonde mit
    # eigener Lauflaenge). Deshalb landet jedes Bild in `bilder/<typ>/f<N>/`.
    aufgaben: list[tuple[Path, int, tuple]] = []
    gesehen: set[tuple[Path, int, tuple]] = set()
    for tdir in typen:
        bilder = out / "bilder" / tdir.name
        bilder.mkdir(parents=True, exist_ok=True)
        for alt in bilder.rglob("*.png"):
            alt.unlink()          # nie auf Bildern einer frueheren Fassung urteilen
        for sonde in sorted(tdir.glob("*.lvfx")):
            if sonde.name.startswith("_"):
                continue          # _default und die _grund_-Vergleichsbilder
            n = frames_fuer(tdir.name, sonde.stem)
            a = tuple(args_fuer(tdir.name, sonde.stem))
            for datei in (sonde, vergleichsdatei(tdir, sonde.stem)):
                if (datei, n, a) not in gesehen:
                    gesehen.add((datei, n, a))
                    aufgaben.append((datei, n, a))

    fertig: dict[tuple[Path, int, tuple], Path] = {}
    fehler: dict[tuple[Path, int, tuple], str] = {}

    def rendere(auftrag: tuple[Path, int, tuple]) -> None:
        lvfx, n, a = auftrag
        try:
            fertig[auftrag] = ca.run_lumi(
                lvfx, n, args.size,
                out / "bilder" / lvfx.parent.name / bildordner(lvfx.parent.name, n, list(a)),
                args.beat_period, list(a))
        except Exception as e:  # noqa: BLE001 — je Sonde weitermachen
            fehler[auftrag] = str(e)

    def rendere_ordner(tdir: Path) -> None:
        """Alle Sonden EINES Typs in EINEM Prozess (`--auto` nimmt Verzeichnisse).

        Der Prozessstart kostet ~800 ms, das Rendern von 40 Frames nur ~160 ms
        (gemessen S54) — je Sonde ein eigener Prozess verschenkt also den
        Grossteil der Laufzeit an Qt- und GL-Initialisierung.
        """
        n = frames_fuer(tdir.name)
        ziel = out / "bilder" / tdir.name / bildordner(tdir.name, n, [])
        try:
            ca.run_lumi_dir(tdir, n, args.size, ziel, args.beat_period)
        except Exception as e:  # noqa: BLE001 — je Typ weitermachen
            for f in tdir.glob("*.lvfx"):
                fehler[(f, n, ())] = str(e)
            return
        for f in tdir.glob("*.lvfx"):
            png = ziel / f"{f.name.replace('.', '_')}_auto.png"
            if png.exists():
                fertig[(f, n, ())] = png
            else:
                fehler[(f, n, ())] = f"kein Bild: {png.name}"

    if args.verzeichnis:
        # Ein Verzeichnis = EINE Lauflaenge. Felder mit eigener Laenge wuerden
        # still unter der falschen gemessen — lieber abbrechen als falsch
        # urteilen.
        betroffen = sorted(k for k in FRAMES_JE_FELD
                           if k.split(".", 1)[0] in {t.name for t in typen})
        if betroffen:
            print("FEHLER: --verzeichnis kann keine feldweisen Lauflaengen "
                  "(FRAMES_JE_FELD): " + ", ".join(betroffen))
            return 2
        print(f"Rendern: {len(aufgaben)} Sonden in {len(typen)} Prozessen "
              f"(ein Verzeichnis je Typ) …")
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            list(pool.map(rendere_ordner, typen))
    else:
        print(f"Rendern: {len(aufgaben)} Bilder einzeln, {args.jobs} gleichzeitig …")
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            list(pool.map(rendere, aufgaben))
    if fehler:
        print(f"  {len(fehler)} Bilder konnten nicht gerendert werden")

    def bild(p: Path, n: int, a: tuple = ()):
        if (p, n, a) in fehler or (p, n, a) not in fertig:
            raise RuntimeError(fehler.get((p, n, a), f"kein Bild: {p.name}"))
        return ca.load_rgb(fertig[(p, n, a)])

    # ----------------------------------------------------------------- urteilen
    rows: list[tuple[str, str, int, float, str]] = []
    for tdir in typen:
        if not (tdir / "_default.lvfx").exists():
            print(f"  FEHLER  {tdir.name}: _default.lvfx fehlt")
            continue

        for sonde in sorted(tdir.glob("*.lvfx")):
            if sonde.name.startswith("_"):
                continue           # _default und die _grund_-Vergleichsbilder
            feld = sonde.stem
            n = frames_fuer(tdir.name, feld)
            a = tuple(args_fuer(tdir.name, feld))
            try:
                gemessen = bild(sonde, n, a)
                # Felder, die nur in Gesellschaft wirken, haben einen eigenen
                # Vergleichsgrund (make_field_probes: GRUNDKONFIG) — sonst
                # zaehlte der Nachbar als zweiter Unterschied mit.
                vergleich = bild(vergleichsdatei(tdir, feld), n, a)
                mae = ca.compare(vergleich, gemessen)["mae"]
                urteil = ("STUMM" if mae == 0.0 else
                          "SCHWACH" if mae < 0.001 else "WIRKT")
                if urteil != "WIRKT":
                    ca.montage(vergleich, gemessen,
                               out / "montage" / f"{tdir.name}_{feld}.png")
            except Exception as e:  # noqa: BLE001
                mae, urteil = 0.0, "FEHLER"
                print(f"  FEHLER  {tdir.name}.{feld}: {e}")
            rows.append((tdir.name, feld, n, mae, urteil))
            zusatz = "" if n == frames_fuer(tdir.name) else f"  [{n} Frames]"
            print(f"  {urteil:7s} {tdir.name}.{feld:24s} MAE {mae:.4f}{zusatz}")

    with (out / "report.md").open("w", encoding="utf-8") as f:
        f.write(f"# Feld-Sonden (Strang E) — {args.frames} Frames, {args.size}\n\n")
        f.write("Urteil gegen `_default` desselben Typs; beide Laeufe teilen "
                "Untergrund und Audio. Die Spalte `Frames` weicht ab, wo Typ "
                "oder Feld eine eigene Lauflaenge brauchen "
                "(FRAMES_JE_TYP / FRAMES_JE_FELD).\n\n")
        f.write("| Typ | Feld | Frames | MAE | Urteil |\n|---|---|---|---|---|\n")
        for typ, feld, n, mae, urteil in rows:
            f.write(f"| {typ} | {feld} | {n} | {mae:.4f} | {urteil} |\n")

    zahl = {u: sum(1 for r in rows if r[4] == u) for u in
            ("WIRKT", "SCHWACH", "STUMM", "FEHLER")}
    print(f"\nReport: {out / 'report.md'}")
    print("  " + " · ".join(f"{k} {v}" for k, v in zahl.items() if v))
    return 0 if zahl["STUMM"] == 0 and zahl["FEHLER"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

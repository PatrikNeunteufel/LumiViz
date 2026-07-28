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
    aufgaben: list[Path] = []
    for tdir in typen:
        bilder = out / "bilder" / tdir.name
        for alt in bilder.glob("*.png"):
            alt.unlink()          # nie auf Bildern einer frueheren Fassung urteilen
        aufgaben += sorted(tdir.glob("*.lvfx"))

    fertig: dict[Path, Path] = {}
    fehler: dict[Path, str] = {}

    def rendere(lvfx: Path) -> None:
        try:
            fertig[lvfx] = ca.run_lumi(lvfx, args.frames, args.size,
                                       out / "bilder" / lvfx.parent.name,
                                       args.beat_period)
        except Exception as e:  # noqa: BLE001 — je Sonde weitermachen
            fehler[lvfx] = str(e)

    def rendere_ordner(tdir: Path) -> None:
        """Alle Sonden EINES Typs in EINEM Prozess (`--auto` nimmt Verzeichnisse).

        Der Prozessstart kostet ~800 ms, das Rendern von 40 Frames nur ~160 ms
        (gemessen S54) — je Sonde ein eigener Prozess verschenkt also den
        Grossteil der Laufzeit an Qt- und GL-Initialisierung.
        """
        ziel = out / "bilder" / tdir.name
        try:
            ca.run_lumi_dir(tdir, args.frames, args.size, ziel, args.beat_period)
        except Exception as e:  # noqa: BLE001 — je Typ weitermachen
            for f in tdir.glob("*.lvfx"):
                fehler[f] = str(e)
            return
        for f in tdir.glob("*.lvfx"):
            png = ziel / f"{f.name.replace('.', '_')}_auto.png"
            if png.exists():
                fertig[f] = png
            else:
                fehler[f] = f"kein Bild: {png.name}"

    if args.verzeichnis:
        print(f"Rendern: {len(aufgaben)} Sonden in {len(typen)} Prozessen "
              f"(ein Verzeichnis je Typ) …")
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            list(pool.map(rendere_ordner, typen))
    else:
        print(f"Rendern: {len(aufgaben)} Sonden einzeln, {args.jobs} gleichzeitig …")
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            list(pool.map(rendere, aufgaben))
    if fehler:
        print(f"  {len(fehler)} Sonden konnten nicht gerendert werden")

    def bild(p: Path):
        if p in fehler or p not in fertig:
            raise RuntimeError(fehler.get(p, f"kein Bild: {p.name}"))
        return ca.load_rgb(fertig[p])

    # ----------------------------------------------------------------- urteilen
    rows: list[tuple[str, str, float, str]] = []
    for tdir in typen:
        basis_datei = tdir / "_default.lvfx"
        if not basis_datei.exists():
            print(f"  FEHLER  {tdir.name}: _default.lvfx fehlt")
            continue
        try:
            basis = bild(basis_datei)
        except Exception as e:  # noqa: BLE001 — je Typ weitermachen
            print(f"  FEHLER  {tdir.name}/_default: {e}")
            rows.append((tdir.name, "_default", 0.0, "FEHLER"))
            continue

        for sonde in sorted(tdir.glob("*.lvfx")):
            if sonde.name.startswith("_"):
                continue           # _default und die _grund_-Vergleichsbilder
            feld = sonde.stem
            try:
                gemessen = bild(sonde)
                # Felder, die nur in Gesellschaft wirken, haben einen eigenen
                # Vergleichsgrund (make_field_probes: GRUNDKONFIG) — sonst
                # zaehlte der Nachbar als zweiter Unterschied mit.
                eigen = tdir / f"_grund_{feld}.lvfx"
                vergleich = bild(eigen) if eigen.exists() else basis
                mae = ca.compare(vergleich, gemessen)["mae"]
                urteil = ("STUMM" if mae == 0.0 else
                          "SCHWACH" if mae < 0.001 else "WIRKT")
                if urteil != "WIRKT":
                    ca.montage(vergleich, gemessen,
                               out / "montage" / f"{tdir.name}_{feld}.png")
            except Exception as e:  # noqa: BLE001
                mae, urteil = 0.0, "FEHLER"
                print(f"  FEHLER  {tdir.name}.{feld}: {e}")
            rows.append((tdir.name, feld, mae, urteil))
            print(f"  {urteil:7s} {tdir.name}.{feld:24s} MAE {mae:.4f}")

    with (out / "report.md").open("w", encoding="utf-8") as f:
        f.write(f"# Feld-Sonden (Strang E) — {args.frames} Frames, {args.size}\n\n")
        f.write("Urteil gegen `_default` desselben Typs; beide Laeufe teilen "
                "Untergrund und Audio.\n\n")
        f.write("| Typ | Feld | MAE | Urteil |\n|---|---|---|---|\n")
        for typ, feld, mae, urteil in rows:
            f.write(f"| {typ} | {feld} | {mae:.4f} | {urteil} |\n")

    zahl = {u: sum(1 for r in rows if r[3] == u) for u in
            ("WIRKT", "SCHWACH", "STUMM", "FEHLER")}
    print(f"\nReport: {out / 'report.md'}")
    print("  " + " · ".join(f"{k} {v}" for k, v in zahl.items() if v))
    return 0 if zahl["STUMM"] == 0 and zahl["FEHLER"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

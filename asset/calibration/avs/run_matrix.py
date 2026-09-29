# -*- coding: utf-8 -*-
"""Modul-Matrix-Lauf (S47): alle matrix/-Presets durch den AvsRef-Vergleich,
IMMER in mindestens zwei Groessen (Vorgabe 320x240 + 740x460),
deterministischer Beat.

Aufruf:
  python run_matrix.py                 # ganze Matrix
  python run_matrix.py 29 43           # nur Effekt-Ordner, die so beginnen
  python run_matrix.py --frames 120
  python run_matrix.py --formate aspekt,monitor   # Formate statt der zwei Groessen (S81)

Ergebnis: out/matrix_compare/report.md — eine Zeile je Preset mit den
Metriken ALLER Groessen/Formate; Urteil = schlechtestes Format, dazu das
Format-Urteil (FORMATABHAENGIG wie in compare_avsref.py). Montagen liegen je
Groesse unter out/matrix_compare/<size>/montage/.
"""
import argparse
import sys
from pathlib import Path

import compare_avsref as ca

ROOT = Path(__file__).parent
SIZES = ("320x240", "740x460")   # Merkregel S46: IMMER zwei Groessen


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("prefixes", nargs="*",
                    help="nur matrix/-Ordner mit diesem Praefix (z.B. 29)")
    ap.add_argument("--frames", type=int, default=120)
    ap.add_argument("--beat-period", type=int, default=20)
    ap.add_argument("--formate", default=None,
                    help="S81: statt der zwei Vorgabe-Groessen diese Formate "
                         "(Bausteine wie compare_avsref.py: aspekt, monitor, "
                         "monitor:DISPLAYn, WxH; asset/calibration/messlauf.py)")
    ap.add_argument("--out", type=Path,
                    default=ROOT / "../../../out/matrix_compare")
    args = ap.parse_args()

    presets = sorted((ROOT / "matrix").glob("*/*.avs"))
    if args.prefixes:
        presets = [p for p in presets
                   if any(p.parent.name.startswith(pre) for pre in args.prefixes)]
    if not presets:
        print("FEHLER: keine Matrix-Presets — erst make_matrix_presets.py")
        return 2

    sizes = list(SIZES)
    if args.formate:
        try:
            sizes = [g for _bez, g in ca.messlauf.formate_aufloesen(args.formate)]
        except ValueError as e:
            print(f"FEHLER: {e}")
            return 2
        if len(sizes) < 2:
            print("FEHLER: --formate braucht mindestens zwei Formate "
                  "(Merkregel S46: nie nur eine Groesse)")
            return 2

    out: Path = args.out.resolve()
    rows = []
    failures = 0
    for avs in presets:
        rel = f"{avs.parent.name}/{avs.name}"
        per_size = {}
        for size in sizes:
            sdir = out / size
            (sdir / "ref").mkdir(parents=True, exist_ok=True)
            (sdir / "lumi").mkdir(exist_ok=True)
            (sdir / "montage").mkdir(exist_ok=True)
            try:
                # Rendern ueber den ASCII-sicheren Pfad (AvsRef ist ANSI, s.
                # compare_avsref.ascii_safe); Originalname bleibt im Bericht.
                src = ca.ascii_safe(avs, out / "_ascii")
                ref = ca.load_rgb(ca.run_ref(src, args.frames, size,
                                             sdir / "ref", args.beat_period))
                lumi = ca.load_rgb(ca.run_lumi(src, args.frames, size,
                                               sdir / "lumi", args.beat_period))
                # S81: beide Seiten muessen GENAU das Format liefern
                ca.pruefe_groesse(ref, size, "AvsRef")
                ca.pruefe_groesse(lumi, size, "AvsStandalone")
                d = ca.compare(ref, lumi)
                # Flaechen-Urteil (S59): die Mittelwert-Metrik luegt bei
                # duennen Inhalten — der nie portierte Dot Fountain mass 0,002,
                # weil beide Bilder ueberwiegend schwarz waren (S53/S57).
                # Deshalb zusaetzlich: Wie unterscheiden sich die LEUCHTENDEN
                # Flaechen? `menge` = relative Abweichung der Pixelzahl,
                # `deckung` = Schnitt/Vereinigung (0 = voellig woanders).
                ref_an = ca.luma(ref) > (8.0 / 255.0)
                lumi_an = ca.luma(lumi) > (8.0 / 255.0)
                n_ref = int(ref_an.sum())
                n_lumi = int(lumi_an.sum())
                d["menge"] = (abs(n_ref - n_lumi) / max(n_ref, 1)
                              if max(n_ref, n_lumi) > 0 else 0.0)
                vereint = int((ref_an | lumi_an).sum())
                d["deckung"] = (int((ref_an & lumi_an).sum()) / vereint
                                if vereint > 0 else 1.0)
                ca.montage(ref, lumi, sdir / "montage" / f"{avs.stem}_{avs.parent.name}.png")
                per_size[size] = d
            except Exception as e:  # noqa: BLE001 — je Preset weitermachen
                per_size[size] = None
                print(f"  FEHLER  {rel} @{size}: {e}")
        if any(v is None for v in per_size.values()):
            failures += 1
            rows.append((rel, per_size, "FEHLER", "–"))
            continue
        # Format-Urteil (S81), gleiche Logik wie compare_avsref.formatbericht
        maes = [v["mae"] for v in per_size.values()]
        if min(maes) <= ca.OK_MAE and max(maes) > ca.BEFUND_MAE:
            format_urteil = "FORMATABHAENGIG"
        else:
            format_urteil = f"Spanne {max(maes) - min(maes):.3f}"
        worst_dmean = max(v["d_mean"] for v in per_size.values())
        worst_mae = max(v["mae"] for v in per_size.values())
        worst_dml = max(v["d_maxluma"] for v in per_size.values())
        worst_menge = max(v["menge"] for v in per_size.values())
        worst_deckung = min(v["deckung"] for v in per_size.values())
        # Schwellen der Flaechen-Pruefung bewusst grob (rand()-Effekte
        # verschieben einzelne Pixel): Menge ±25 %, Deckung >= 0,5. Der
        # unportierte Dot Fountain haette hier Menge ~0,8 / Deckung ~0,1
        # gemessen — die Zeile war mit 0,002 MAE trotzdem "gruen".
        urteil = ("OK" if (worst_dmean <= 0.02 and worst_dml <= 0.10
                           and worst_mae <= 0.03 and worst_menge <= 0.25
                           and worst_deckung >= 0.5) else "PRUEFEN")
        rows.append((rel, per_size, urteil, format_urteil))
        parts = " · ".join(
            f"{s}: dMean={v['d_mean']:.3f} MAE={v['mae']:.3f}"
            for s, v in per_size.items())
        print(f"  {urteil:7s} {rel}  {parts}  Menge={worst_menge:.2f} "
              f"Deckung={worst_deckung:.2f}  {format_urteil}")

    report = out / "report.md"
    with report.open("w", encoding="utf-8") as f:
        f.write(f"# Modul-Matrix — {args.frames} Frames, Beat alle "
                f"{args.beat_period}, Groessen {' + '.join(sizes)}\n\n")
        f.write("| Preset | " + " | ".join(
            f"{s} dMean/dMaxLuma/MAE" for s in sizes)
            + " | Urteil | Format-Urteil |\n")
        f.write("|---|" + "---|" * (len(sizes) + 2) + "\n")
        for rel, per_size, urteil, format_urteil in rows:
            cells = []
            for s in sizes:
                v = per_size.get(s)
                cells.append("–" if v is None else
                             f"{v['d_mean']:.3f} / {v['d_maxluma']:.3f} / "
                             f"{v['mae']:.3f}")
            f.write(f"| {rel} | " + " | ".join(cells)
                    + f" | {urteil} | {format_urteil} |\n")
        f.write("\nMontagen: `<size>/montage/` — Urteil = schlechteste "
                "Groesse. rand()-Effekte (Scatter/Grain/Starfield/Particle) "
                "sind nicht bit-stabil: dort zaehlt die Montage.\n"
                f"Format-Urteil: FORMATABHAENGIG = MAE <= {ca.OK_MAE} in "
                f"einem, > {ca.BEFUND_MAE} in einem anderen Format.\n")
    print(f"\nReport: {report}")
    abh = sum(1 for r in rows if r[3] == "FORMATABHAENGIG")
    print(f"{abh} formatabhaengig")
    ok = sum(1 for r in rows if r[2] == "OK")
    print(f"{ok}/{len(rows)} OK, {len(rows) - ok} zu pruefen"
          f" (davon {failures} Fehler)")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

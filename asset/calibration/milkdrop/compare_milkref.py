# -*- coding: utf-8 -*-
"""Diff-Harness MilkDrop: MilkdropRef (Original-Kern) vs. MilkdropStandalone.

Aufgabe 1 des Kalibrier-Plans (`docs/visuals/Kalibrier_Plan_Mitgelieferte_Presets.md`).
Gegenstueck zu `compare_avsref.py` fuer den MilkDrop-Zweig.

Warum ein eigenes Werkzeug — zwei Fehler machten alle bisherigen Stapelzahlen
zu blossen Richtwerten:

1. **Ein Prozess je Preset.** `--auto <ordner>` rendert alle Presets in EINEM
   Prozess, und MilkDrop-Presets erben das Bild des Vorgaengers (Original-
   Verhalten, kein Fehler). Im Stapel zeigte `Helix` ein Herz aus dem
   alphabetisch davor laufenden `Dancing Hearts` (Befund Patrik, S73). Dieses
   Werkzeug startet **beide** Seiten je Preset neu.
2. **Mehrere Frame-Marken.** MilkDrop ist ein Rueckkopplungssystem: eine
   winzige Abweichung in Frame 3 kann bis Frame 120 ein voellig anderes
   Standbild ergeben, obwohl beide Seiten das Preset korrekt zeigen. Ein
   Einzelframe vermischt echten Fehler und Phasenversatz. Gemessen wird
   deshalb bei 10/30/120 (`--frames`); frueh wiegt schwerer.

`compare_ref.py` bleibt daneben bestehen — es wertet Screenshots eines
Triage-Laufs aus und rendert selbst nichts.

Aufruf:
  python compare_milkref.py                          # die 19 mitgelieferten
  python compare_milkref.py pfad/preset.milk ...     # einzelne Presets
  python compare_milkref.py --presets DIR            # ganzer Ordner
  python compare_milkref.py --frames 10,30,120 --size 640x480 --out DIR

Hinweise:
- Die Referenz braucht die MilkDrop-Ordnerstruktur (`<wurzel>/data/include.fx`
  neben `<wurzel>/presets/`, Werkzeug-Wegleitung 2.5). Dieses Skript baut sie
  im Ausgabeverzeichnis selbst auf.
- Die Standalones rendern im GUI-Thread — Fenster blitzen auf und muessen
  stehen bleiben (Wegleitung 2.6).
- `--audio-datei` ist gesperrt: die Referenz erzeugt ihr Audio selbst
  (Wegleitung 2.7). Mehr Dynamik gibt es ueber `--audio-muster musik`.
- **Pixelgleichheit ist hier nicht zu haben** (D3D9 gegen GL, und der
  Original-Kern taktet an der WANDUHR). Das Urteil faellt an der Montage; die
  Zahlen ordnen ein.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

for _stream in (sys.stdout, sys.stderr):
    if hasattr(_stream, "reconfigure"):
        _stream.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).parent
REPO = (ROOT / "../../..").resolve()
LUMI_EXE = (REPO / "out/build/windows-ninja-release-clang/exec/"
            "MilkdropStandalone/bin/Release/MilkdropStandalone.exe").resolve()
REF_EXE = (REPO / "tools/MilkdropRef/build/Release/MilkdropRef.exe").resolve()
ASSET = (REPO / "asset/Milkdrop3").resolve()
DEFAULT_PRESETS = (REPO / "asset/presets/milkdrop").resolve()

# Schwellen. Bewusst grosszuegiger als bei AVS: dort stehen zwei
# Software-Rasterizer nebeneinander, hier D3D9 gegen OpenGL — schon
# Texturfilterung und Rundung trennen die Seiten sichtbar, ohne dass ein
# Port-Fehler vorliegt.
OK_MAE = 0.08
BEFUND_MAE = 0.15


# „Tot" heisst hier WIRKLICH schwarz, nicht „dunkel". Die erste Fassung nahm
# lumaMax < 0.05 und modalFrac > 0.995 — damit stand `Starfield` mit MAE 0.002
# als BEFUND in der Liste, weil die eine Seite knapp unter und die andere knapp
# ueber der Schwelle lag (S75). Ein Urteil an einer Schwellenkante ist kein
# Urteil. Ein echter „eine Seite zeichnet nicht"-Befund braucht deshalb ZWEI
# Bedingungen: die eine Seite praktisch schwarz UND die andere deutlich hell.
TOT_MAX = 0.02
LEBT_MAX = 0.20


def ist_tot(m: dict) -> bool:
    """Bild ohne verwertbaren Inhalt — praktisch schwarz."""
    return m["lumaMax"] < TOT_MAX


def kennzahlen(img: np.ndarray) -> dict:
    lum = 0.2126 * img[..., 0] + 0.7152 * img[..., 1] + 0.0722 * img[..., 2]
    quant = (img * 15).astype(np.uint8).reshape(-1, 3)
    _, counts = np.unique(quant, axis=0, return_counts=True)
    return {
        "lumaMean": float(lum.mean()),
        "lumaMax": float(lum.max()),
        "modalFrac": float(counts.max() / quant.shape[0]),
    }


def refwurzel_bauen(ziel: Path) -> Path:
    """`<ziel>/data` + `<ziel>/textures` + `<ziel>/presets` anlegen.

    Der Original-Kern sucht seine Datendatei unter `<presetordner>/../data/`
    und bricht sonst mit `FEHLER: PluginInitialize` ab (Wegleitung 2.5).
    `sprites/` und `textures/` kommen mit, weil MilkdropRef
    `m_szMilkdrop2Path` auf den Grosseltern-Ordner des Presets setzt.
    """
    ziel.mkdir(parents=True, exist_ok=True)
    for teil in ("data", "textures", "sprites"):
        quelle = ASSET / teil
        if quelle.is_dir() and not (ziel / teil).exists():
            shutil.copytree(quelle, ziel / teil)
    presets = ziel / "presets"
    presets.mkdir(exist_ok=True)
    return presets


def run_ref(milk: Path, frames: int, size: str, out: Path, refroot: Path,
            muster: str, silence: bool, timeout: int,
            seed: int = 0) -> Path:
    """MilkdropRef rendern (eigener Prozess); liefert den BMP-Pfad."""
    ziel_presets = refwurzel_bauen(refroot)
    kopie = ziel_presets / milk.name
    if not kopie.exists() or kopie.stat().st_mtime < milk.stat().st_mtime:
        shutil.copy2(milk, kopie)
    extra = ["--audio-muster", muster] if muster != "klassisch" else []
    if silence:
        extra.append("--silence")
    extra += ["--saat", hex(seed)]
    out.mkdir(parents=True, exist_ok=True)
    proc = subprocess.run(
        [str(REF_EXE), str(kopie), "--frames", str(frames), "--size", size,
         "--out", str(out)] + extra,
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=timeout)
    bmp = out / f"{milk.stem}_ref.bmp"
    if not bmp.exists():
        raise RuntimeError(f"MilkdropRef rc={proc.returncode}, BMP fehlt\n"
                           f"{proc.stdout}{proc.stderr}")
    # Regel 8: die Referenz ist selbst ein Messgeraet. Konnte sie das Preset
    # nicht laden, faellt der Kern in seinen Preset-Browser und zeichnet ein
    # Verzeichnis-Overlay — jede Zahl dagegen waere Unsinn (Befund S75).
    stumm = "PRESET-NICHT-GELADEN" in (proc.stdout or "")
    # Puffergroesse der Referenz mitlesen. Weicht sie von unserer ab, ergibt
    # DERSELBE Seed verschiedene Startbilder — und weil MilkDrop rueckkoppelt,
    # zieht sich das durch alle Frames: ein Scheinbefund ueber jedes Preset
    # (Frage Patrik, S75). Der Aufrufer bricht dann den Saat-Vergleich ab.
    puffer = None
    m = re.search(r"Feedback-Puffer: (\d+)x(\d+)", proc.stdout or "")
    if m:
        puffer = (int(m.group(1)), int(m.group(2)))
    return bmp, stumm, puffer


def run_lumi(milk: Path, frames: int, size: str, out: Path, muster: str,
             silence: bool, timeout: int, seed: int = 0) -> Path:
    """MilkdropStandalone rendern (eigener Prozess); liefert den PNG-Pfad.

    `seed` steuert die Kaltstart-Saat — DERSELBE Wert geht an `MilkdropRef
    --saat`, damit beide Seiten im selben Startzustand beginnen. 0 = saatlos,
    also der Kaltstart des Originals (das nullt den Puffer im ersten Frame
    ausdruecklich, `milkdropfs.cpp`).
    """
    env = dict(os.environ)
    env["QT_ENABLE_HIGHDPI_SCALING"] = "0"  # logische == physische Pixel
    saat_args = []
    if seed == 0:
        env["LUMIVIZ_MILKDROP_NOSEED"] = "1"
    else:
        # ACHTUNG: der Standalone setzt `LUMIVIZ_MILKDROP_NOSEED` SELBST,
        # solange `--seed` fehlt (main.cpp: saatlos ist sein Vertrag seit S64).
        # Eine Env-Variable allein bleibt deshalb wirkungslos — erst das Flag
        # schaltet die Saat frei, den Startwert waehlt dann die Variable.
        # (S75: ohne das Flag waren drei „verschiedene" Seeds bit-identisch.)
        env.pop("LUMIVIZ_MILKDROP_NOSEED", None)
        env["LUMIVIZ_MILKDROP_SEED"] = hex(seed)
        saat_args = ["--seed"]
    extra = ["--audio-muster", muster] if muster != "klassisch" else []
    if silence:
        extra.append("--silence")
    out.mkdir(parents=True, exist_ok=True)
    proc = subprocess.run(
        [str(LUMI_EXE), str(milk), "--auto", "--frames", str(frames),
         "--size", size, "--out", str(out)] + extra + saat_args,
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=timeout, env=env)
    png = out / f"{milk.stem}_auto.png"
    if not png.exists():
        raise RuntimeError(f"MilkdropStandalone rc={proc.returncode}, PNG fehlt\n"
                           f"{proc.stdout}{proc.stderr}")
    return png


def load_rgb(path: Path) -> np.ndarray:
    return np.asarray(Image.open(path).convert("RGB"), dtype=np.float64) / 255.0


def montage(ref: np.ndarray, lumi: np.ndarray, out_png: Path) -> None:
    """ref | lumiviz | absdiff (Diff 4x verstaerkt)."""
    diff = np.clip(np.abs(ref - lumi) * 4.0, 0.0, 1.0)
    gap = np.ones((ref.shape[0], 4, 3)) * 0.5
    Image.fromarray(
        (np.concatenate([ref, gap, lumi, gap, diff], axis=1) * 255)
        .astype(np.uint8)).save(out_png)


def urteil_bilden(marken: list[dict]) -> tuple[str, str]:
    """Urteil je Preset aus den Frame-Marken. Liefert (Urteil, Begruendung).

    Die FRUEHESTE Marke wiegt am schwersten: dort ist der Phasenversatz noch
    klein, ein echter Fehler aber schon da (Kalibrier-Plan 0.2).
    """
    if not marken:
        return "FEHLER", "keine Marke gemessen"
    frueh = marken[0]
    # Die Referenz konnte das Preset nicht laden -> es gibt nichts zu messen.
    if any(m["ref_stumm"] for m in marken):
        return "REF-STUMM", ("MilkdropRef hat das Preset nicht geladen — "
                             "kein Referenzbild, kein Urteil")
    # Zeigt KEINE Seite etwas, prueft die Probe nichts. Das ist ausdruecklich
    # kein Bestehen: ein untauglicher Pruefstand sieht aus wie ein gruener.
    if all(m["tot_ref"] and m["tot_lumi"] for m in marken):
        return "BEIDE-STUMM", ("beide Seiten zeichnen nichts — kein Urteil "
                               "moeglich, Preset oder Anregung pruefen")
    # Eine Seite tot, die andere nicht — das ist immer ein Befund, unabhaengig
    # von der Metrik: ein schwarzes gegen ein gezeichnetes Bild kann eine
    # kleine MAE haben (Regel 5 der Kalibrier-Regeln).
    # ANLAUF: der Original-Kern braucht Frames, bis ueberhaupt etwas im Bild
    # steht — bei `Starfield` ist er in Frame 10 noch schwarz und liefert ab
    # Frame 30 MAE 0.005. Eine Marke, in der die Referenz noch gar nicht
    # zeichnet, ist keine Messung, sondern ihre Warmlaufphase; sie als Befund
    # zu fuehren, macht aus dem Prueflauf eine Liste von Scheinfunden (S75:
    # 0/19 OK im ersten Durchgang). Verworfen wird nur, wenn eine SPAETERE
    # Marke zeigt, dass die Referenz danach zeichnet — bleibt sie durchgehend
    # stumm, ist der Unterschied echt.
    ausgewertet = []
    for i, m in enumerate(marken):
        if m["tot_ref"] and any(x["hell_ref"] for x in marken[i + 1:]):
            continue  # Anlauf der Referenz
        ausgewertet.append(m)
    if not ausgewertet:
        return "PRUEFEN", "nur Anlaufphase gemessen — spaeter messen"
    verworfen = len(marken) - len(ausgewertet)
    anlauf = (f" (Frame {marken[0]['frames']} verworfen: Anlauf der Referenz)"
              if verworfen else "")
    marken, frueh = ausgewertet, ausgewertet[0]

    for m in marken:
        if m["tot_ref"] and m["hell_lumi"]:
            return "BEFUND", (f"bei Frame {m['frames']} zeichnet die Referenz "
                              f"nicht, wir schon{anlauf}")
        if m["tot_lumi"] and m["hell_ref"]:
            return "BEFUND", (f"bei Frame {m['frames']} zeichnet LumiViz "
                              f"nicht, die Referenz schon{anlauf}")
    if frueh["mae"] > BEFUND_MAE:
        return "BEFUND", (f"schon Frame {frueh['frames']}: "
                          f"MAE {frueh['mae']:.3f}{anlauf}")
    if all(m["mae"] <= OK_MAE for m in marken):
        return "OK", anlauf.strip()
    if frueh["mae"] <= OK_MAE:
        return "PRUEFEN", (f"Frame {frueh['frames']} gruen "
                           f"({frueh['mae']:.3f}), spaeter nicht — "
                           f"Phasenversatz oder echter Fehler, Montage ansehen"
                           f"{anlauf}")
    return "PRUEFEN", f"Frame {frueh['frames']}: MAE {frueh['mae']:.3f}{anlauf}"


def streuung(bilder: dict, saaten: list, marken: list, idx: int) -> float:
    """Wie stark aendert sich EINE Seite, wenn nur der Startwert wechselt.

    @param idx  0 = Referenz, 1 = LumiViz.
    """
    werte = []
    for n in marken:
        vorhanden = [bilder[(s, n)][idx] for s in saaten if (s, n) in bilder]
        for a, b in zip(vorhanden, vorhanden[1:]):
            werte.append(float(np.abs(a - b).mean()))
    return max(werte) if werte else 0.0


def gesamturteil(je_saat: dict, bilder: dict, saaten: list,
                 marken: list) -> tuple:
    """Fasst die Seed-Laeufe zusammen — inkl. der Frage nach Startabhaengigkeit.

    Der Kern der Seed-Reihe (Vorgabe Patrik, S75): ein einzelner Startwert
    beantwortet nur „stimmen die Bilder bei diesem einen Rauschmuster". Erst
    mehrere zeigen, **ob das Preset ueberhaupt ein stabiles Bild hat**. Streut
    eine Seite ueber die Startwerte staerker, als die beiden Renderer
    voneinander abweichen, misst man Startzustand statt Renderpfad — dann ist
    das Preset schlicht nicht pixelvergleichbar (die Dunkelklasse aus S67,
    hier gemessen statt vermutet).
    """
    if not je_saat:
        return "FEHLER", "nichts gemessen"
    urteile = [je_saat[s][0] for s in saaten if s in je_saat]

    if len(saaten) > 1:
        str_ref = streuung(bilder, saaten, marken, 0)
        str_lumi = streuung(bilder, saaten, marken, 1)
        unterschied = max(
            (w["mae"] for s in je_saat for w in je_saat[s][2]), default=0.0)
        if max(str_ref, str_lumi) > max(unterschied, OK_MAE):
            return "STARTABHAENGIG", (
                f"Streuung ueber die Startwerte (Ref {str_ref:.3f} / "
                f"LumiViz {str_lumi:.3f}) uebertrifft den Unterschied der "
                f"Renderer ({unterschied:.3f}) — kein Treue-Urteil moeglich")

    # Sonst das schlechteste Einzelurteil, mit dem Startwert, der es ausloest.
    rang = ["FEHLER", "REF-STUMM", "BEFUND", "PRUEFEN", "BEIDE-STUMM", "OK"]
    schlecht = min(urteile, key=lambda u: rang.index(u) if u in rang else 0)
    for s in saaten:
        if s in je_saat and je_saat[s][0] == schlecht:
            grund = je_saat[s][1]
            zusatz = f" [Saat 0x{s:X}]" if len(saaten) > 1 else ""
            return schlecht, (grund + zusatz if grund else zusatz.strip())
    return schlecht, ""


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("presets", nargs="*", type=Path)
    ap.add_argument("--presets", dest="preset_dir", type=Path, default=None,
                    help="ganzen Ordner vergleichen (Vorgabe: die 19 "
                         "mitgelieferten aus asset/presets/milkdrop)")
    ap.add_argument("--frames", default="10,30,120",
                    help="Frame-Marken je Preset (Komma). Frueh wiegt schwerer: "
                         "dort ist der Phasenversatz noch klein")
    ap.add_argument("--size", default="640x480")
    ap.add_argument("--audio-muster", default="klassisch",
                    choices=["klassisch", "musik"],
                    help="synthetisches Signal; beide Seiten binden dieselbe "
                         "SynthAudio.hpp ein, der Vergleich bleibt gueltig")
    ap.add_argument("--silence", action="store_true",
                    help="Null-Signal (Hunger-Test: was zeichnet ein Preset "
                         "ohne Ton)")
    ap.add_argument("--saaten", default="0,0x5EED63",
                    help="Startwerte des Feedback-Puffers (Komma), auf BEIDEN "
                         "Seiten gesetzt. 0 = saatlos (Kaltstart des "
                         "Originals), 0x5EED63 = App-Vorgabe. Mehr Werte "
                         "zeigen, ob ein Preset ueberhaupt ein "
                         "startzustands-unabhaengiges Bild hat")
    ap.add_argument("--timeout", type=int, default=180, help="Sekunden je Lauf")
    ap.add_argument("--out", type=Path, default=REPO / "out/milkref_compare")
    args = ap.parse_args()

    for exe, name in ((REF_EXE, "MilkdropRef (tools/MilkdropRef, -A Win32)"),
                      (LUMI_EXE, "MilkdropStandalone (build-ninja-release-clang)")):
        if not exe.exists():
            print(f"FEHLER: {name} fehlt: {exe}")
            return 2

    presets = [p for p in args.presets if p.suffix.lower() == ".milk"]
    for p in args.presets:
        if p.is_dir():
            presets += sorted(p.rglob("*.milk"))
    if not presets:
        quelle = args.preset_dir or DEFAULT_PRESETS
        # rekursiv: die mitgelieferten liegen in einem Sammlungs-Unterordner
        presets = sorted(Path(quelle).rglob("*.milk"))
    if not presets:
        print("FEHLER: keine .milk-Presets gefunden")
        return 2

    marken = [int(f) for f in str(args.frames).split(",") if f.strip()]
    saaten = [int(s, 0) for s in str(args.saaten).split(",") if s.strip()]
    out: Path = args.out.resolve()
    refroot = out / "_refwurzel"
    (out / "montage").mkdir(parents=True, exist_ok=True)

    zeilen = []
    fehler = 0
    for milk in presets:
        je_saat = {}       # seed -> (urteil, grund, werte)
        bilder = {}        # (seed, marke) -> (ref_img, lumi_img)
        urteil, grund = "FEHLER", ""
        try:
            for seed in saaten:
                werte = []
                for n in marken:
                    ref_bmp, ref_stumm, ref_puffer = run_ref(
                        milk, n, args.size, out / f"ref_s{seed:x}_f{n}",
                        refroot, args.audio_muster, args.silence,
                        args.timeout, seed)
                    ref_img = load_rgb(ref_bmp)
                    lumi_img = load_rgb(run_lumi(
                        milk, n, args.size, out / f"lumi_s{seed:x}_f{n}",
                        args.audio_muster, args.silence, args.timeout, seed))
                    if ref_img.shape != lumi_img.shape:
                        raise RuntimeError(
                            f"Groessen ungleich: ref{ref_img.shape} vs "
                            f"lumi{lumi_img.shape} — DPI-Skalierung?")
                    # Saat nur gueltig, wenn BEIDE Feedback-Puffer gleich gross
                    # sind — sonst ergibt derselbe Seed verschiedene
                    # Startbilder und jedes Preset meldet einen Scheinbefund.
                    if seed != 0 and ref_puffer is not None:
                        unser = (lumi_img.shape[1], lumi_img.shape[0])
                        if ref_puffer != unser:
                            raise RuntimeError(
                                f"Feedback-Puffer ungleich: Referenz "
                                f"{ref_puffer[0]}x{ref_puffer[1]} vs "
                                f"{unser[0]}x{unser[1]} — mit Saat nicht "
                                "vergleichbar")
                    kr, kl = kennzahlen(ref_img), kennzahlen(lumi_img)
                    werte.append({
                        "frames": n,
                        "mae": float(np.abs(ref_img - lumi_img).mean()),
                        "d_mean": float(abs(kr["lumaMean"] - kl["lumaMean"])),
                        "tot_ref": ist_tot(kr),
                        "tot_lumi": ist_tot(kl),
                        "hell_ref": kr["lumaMax"] > LEBT_MAX,
                        "hell_lumi": kl["lumaMax"] > LEBT_MAX,
                        "ref_stumm": ref_stumm,
                    })
                    bilder[(seed, n)] = (ref_img, lumi_img)
                    montage(ref_img, lumi_img,
                            out / "montage" / f"{milk.stem}_s{seed:x}_f{n}.png")
                je_saat[seed] = urteil_bilden(werte) + (werte,)
            urteil, grund = gesamturteil(je_saat, bilder, saaten, marken)
        except Exception as e:  # noqa: BLE001 — je Preset weitermachen
            fehler += 1
            urteil, grund = "FEHLER", str(e).splitlines()[0]
        zeilen.append((milk.stem, je_saat, urteil, grund))
        zahlen = " · ".join(
            f"s{seed:x}:" + ",".join(f"{w['mae']:.3f}" for w in je_saat[seed][2])
            for seed in saaten if seed in je_saat)
        print(f"  {urteil:14s} {milk.stem}  {zahlen}"
              f"{'  — ' + grund if grund else ''}")

    report = out / "report.md"
    with report.open("w", encoding="utf-8") as f:
        f.write(f"# MilkdropRef-Vergleich — Marken {marken}, {args.size}, "
                f"Muster {args.audio_muster}\n\n")
        f.write("Ein Prozess je Preset, Marke und Startwert (Aufgabe 1 des "
                "Kalibrier-Plans). Urteil an der Montage, Zahlen ordnen ein.\n\n")
        f.write("Startwerte: " + ", ".join(f"`0x{s:X}`" for s in saaten)
                + " — auf BEIDEN Seiten gesetzt (`--saat` bzw. "
                  "`LUMIVIZ_MILKDROP_SEED`). `0x0` ist der Kaltstart des "
                  "Originals: der Kern nullt den Feedback-Puffer im ersten "
                  "Frame ausdruecklich.\n\n")
        f.write("`STARTABHAENGIG` heisst: das Bild haengt staerker am "
                "Startzustand als die Renderer voneinander abweichen — dort "
                "ist kein Treue-Urteil zu holen.\n\n")
        kopf = [f"MAE 0x{s:X} f{n}" for s in saaten for n in marken]
        f.write("| Preset | " + " | ".join(kopf) + " | Urteil | Anmerkung |\n")
        f.write("|---" * (len(kopf) + 3) + "|\n")
        for stem, je_saat, urteil, grund in zeilen:
            spalten = []
            for s in saaten:
                maes = ({w["frames"]: w["mae"] for w in je_saat[s][2]}
                        if s in je_saat else {})
                spalten += [f"{maes[n]:.3f}" if n in maes else "–"
                            for n in marken]
            f.write(f"| {stem} | " + " | ".join(spalten)
                    + f" | {urteil} | {grund} |\n")
        f.write("\nMontagen (ref | lumiviz | 4x-Diff): `montage/`\n")
    print(f"\nReport: {report}")
    ok = sum(1 for z in zeilen if z[2] == "OK")
    befunde = sum(1 for z in zeilen if z[2] == "BEFUND")
    stumm = sum(1 for z in zeilen if z[2] in ("REF-STUMM", "BEIDE-STUMM"))
    print(f"{ok}/{len(zeilen)} OK, {befunde} Befunde, {stumm} ohne Urteil "
          f"(stumm), {fehler} Fehler")
    return 1 if fehler else 0


if __name__ == "__main__":
    sys.exit(main())

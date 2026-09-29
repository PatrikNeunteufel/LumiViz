# -*- coding: utf-8 -*-
"""Diff-Harness: AvsRef (originaler vis_avs-Kern) vs. AvsStandalone (LumiViz).

Rendert jedes Kalibrier-.avs mit BEIDEN Renderern (gleiche Frames, gleiche
Groesse, byte-identisches synthetisches Audio) und vergleicht die Endbilder:
mean-RGB-/Luma-Differenzen plus Seite-an-Seite-Montage (ref | lumiviz | diff)
je Preset. Ergebnis: Konsolen-Tabelle + report.md im Ausgabeverzeichnis.

Aufruf:
  python compare_avsref.py                     # alle s*-Kalibrier-Presets
  python compare_avsref.py --tests             # zusaetzlich ../tests/*.avs (P-Presets)
  python compare_avsref.py pfad/zu/preset.avs  # einzelne Presets
  python compare_avsref.py --frames 120 --size 320x240 --out DIR
  python compare_avsref.py --formate aspekt,monitor  # Formate nebeneinander (S81)

Hinweise:
- AvsStandalone braucht ein SICHTBARES Fenster (Merkregel S44) — Fenster
  blitzen kurz auf. QT_ENABLE_HIGHDPI_SCALING=0 haelt logisch==physisch.
- Beide Seiten erkennen Beats selbst (Original-Detektor vs. bpm.cpp-Port) —
  bei Beat-Drift koennen Einzelframes abweichen; Urteil primaer ueber die
  Montagen und die mean/max-Metriken.
- Presets mit rand() sind zwischen Laeufen nicht bit-stabil.
- Beide Seiten rendern NATIV in der verlangten Groesse: AvsRef kennt keinen
  Render-Scale-Divisor, deshalb bleibt AvsStandalone hier bei `--render-scale`
  1 (Vorgabe). Den Divisor der App (Werkzeug-Wegleitung, Falle 2.2) gibt es
  im Original nicht — mit ihm verglichen hiesse Aepfel mit Birnen.
- Werkzeuge starten ohne Konsolenfenster (S81, `../messlauf.py`).
"""
import argparse
import hashlib
import re
import shutil
import sys
import unicodedata
from pathlib import Path

import numpy as np
from PIL import Image

# Konsole auf UTF-8 zwingen: die Standardkodierung ist hier cp1252, und ein
# Preset-Name mit "š" liess frueher schon die FEHLER-Ausgabe abstuerzen — was
# den GANZEN Sweep mitriss (Befund S50). errors="replace" statt Absturz.
for _stream in (sys.stdout, sys.stderr):
    if hasattr(_stream, "reconfigure"):
        _stream.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).parent
# Gemeinsame Bausteine (S81): Start ohne Konsolenfenster, Formate. Wer dieses
# Modul als `ca` importiert, startet ueber `ca.messlauf.run` ebenso.
sys.path.insert(0, str(ROOT.parent))
import messlauf  # noqa: E402
LUMI_EXE = (ROOT / "../../../out/build/windows-ninja-release-clang/exec/"
            "AvsStandalone/bin/Release/AvsStandalone.exe").resolve()
REF_EXE = (ROOT / "../../../tools/AvsRef/build/Release/AvsRef.exe").resolve()
# APE-Sammlung: AvsRef laedt ohne --ape-dir KEINE Plugin-DLLs — Presets mit APEs
# (Color Map, Texer, ...) waeren dann unvergleichbar, weil nur WIR den Effekt
# rendern. Default: die externe Preset-Sammlung, falls vorhanden (S49).
APE_DIR = (ROOT / "../../../../VisualsPresets/avs").resolve()

# Urteilsschwellen: bewusst grob (GL- vs. Software-Rasterizer rastern Linien
# verschieden) — Feinurteil ueber die Montage. OK nur, wenn alle drei halten.
OK_DMEAN = 0.02
OK_DMAXLUMA = 0.10
OK_MAE = 0.03
# Nur fuer das Format-Urteil (S81): ab hier ist ein Format ein Befund. Der
# AVS-Lauf kennt sonst nur OK/PRUEFEN; doppelte OK-Schwelle, damit ein
# knappes PRUEFEN neben einem knappen OK nicht schon als formatabhaengig gilt.
# Entscheid Patrik a (S81): 0,06 bleibt, bis Formatlaeufe viele knappe Faelle
# zwischen 0,03 und 0,06 zeigen — dann aus einer Messreihe ableiten.
BEFUND_MAE = 0.06

STATS_RE = re.compile(
    r"mean RGB=\(([\d.]+), ([\d.]+), ([\d.]+)\), Luma min=([\d.]+) max=([\d.]+)")


def ascii_safe(avs: Path, stage: Path) -> Path:
    """Pfad, den beide Renderer sicher oeffnen koennen.

    AvsRef ist ein ANSI-Programm (`main(int, char**)`): Windows setzt die
    UTF-16-Kommandozeile in die ANSI-Codepage um, und alles ausserhalb von
    cp1252 wird dabei zu "?" — die Datei ist dann schlicht nicht auffindbar
    (Befund S50: "06 - fšk - Wtf I'm Lost.avs"). Statt uns auf 8.3-Kurznamen zu
    verlassen (auf Volumes mit NtfsDisable8dot3NameCreation gibt es keine),
    legen wir eine Kopie mit reinem ASCII-Namen an. Der Hash haelt Namen
    auseinander, die sich erst durch die entfernten Zeichen gleichen wuerden.

    Reine ASCII-Pfade werden unveraendert durchgereicht — kein Kopieren im
    Normalfall. Der Aufrufer behaelt den ORIGINALPFAD fuer Bericht und
    Montage-Namen; nur die Renderer sehen die Kopie.
    """
    if str(avs).isascii():
        return avs
    flat = unicodedata.normalize("NFKD", avs.name).encode("ascii", "ignore").decode()
    flat = re.sub(r"[^A-Za-z0-9._ -]", "", flat) or "preset.avs"
    if not flat.lower().endswith(".avs"):
        flat += ".avs"
    tag = hashlib.sha1(str(avs).encode("utf-8")).hexdigest()[:8]
    stage.mkdir(parents=True, exist_ok=True)
    dst = stage / f"{Path(flat).stem}_{tag}.avs"
    if not dst.exists():
        shutil.copy2(avs, dst)
    return dst


def run_ref(avs: Path, frames: int, size: str, out: Path, beat_period: int = 0,
            ape_dir: Path = None, tick_hz: int = 0, muster: str = "klassisch") -> Path:
    """AvsRef rendern; liefert den BMP-Pfad des letzten Frames.

    `muster` waehlt das synthetische Signal (S74). Beide Renderer binden
    dieselbe `SynthAudio.hpp` ein — der Vergleich bleibt damit in JEDEM Muster
    gueltig, anders als bei `--audio-datei` (echte Musik, gesperrt).
    """
    extra = ["--beat-period", str(beat_period)] if beat_period > 0 else []
    if muster != "klassisch":
        extra += ["--audio-muster", muster]
    if tick_hz > 0:
        extra += ["--tick-hz", str(tick_hz)]
    if ape_dir and Path(ape_dir).is_dir():
        extra += ["--ape-dir", str(ape_dir)]
    proc = messlauf.run(
        [str(REF_EXE), str(avs), "--frames", str(frames), "--size", size,
         "--out", str(out)] + extra,
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=300)
    if proc.returncode != 0:
        raise RuntimeError(f"AvsRef rc={proc.returncode}\n{proc.stdout}{proc.stderr}")
    base = avs.name.replace(".", "_")
    bmp = out / f"{base}_f{frames:04d}_ref.bmp"
    if not bmp.exists():
        raise RuntimeError(f"AvsRef-BMP fehlt: {bmp}\n{proc.stdout}")
    return bmp


def run_lumi_dir(ordner: Path, frames: int, size: str, out: Path,
                 beat_period: int = 0) -> None:
    """Ein GANZES Verzeichnis in EINEM Prozess rendern (`--auto` kann das).

    Der Prozessstart kostet rund 800 ms, 40 Frames rendern nur rund 160 ms
    (gemessen S54) — wer je Preset einen Prozess startet, verschenkt den
    Grossteil der Laufzeit an Qt- und GL-Initialisierung. Die Bilder heissen
    wie beim Einzelaufruf: `<datei_mit_unterstrich>_auto.png`.
    """
    import os
    env = dict(os.environ)
    env["QT_ENABLE_HIGHDPI_SCALING"] = "0"
    extra = ["--beat-period", str(beat_period)] if beat_period > 0 else []
    out.mkdir(parents=True, exist_ok=True)
    proc = messlauf.run(
        [str(LUMI_EXE), str(ordner), "--auto", "--frames", str(frames),
         "--size", size, "--out", str(out)] + extra,
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=1800, env=env)
    if proc.returncode != 0:
        raise RuntimeError(f"AvsStandalone rc={proc.returncode}\n"
                           f"{proc.stdout}{proc.stderr}")


def run_lumi(avs: Path, frames: int, size: str, out: Path, beat_period: int = 0,
             extra_args: list[str] | None = None, muster: str = "klassisch") -> Path:
    """AvsStandalone --auto rendern; liefert den PNG-Pfad des Screenshots.

    `extra_args` reicht weitere Schalter durch — gedacht fuer Sonden, die ein
    anderes Testsignal brauchen als der Rest (`--stereo-spektrum`, S55). Wer so
    etwas benutzt, misst NICHT mehr gegen dieselbe Grundlage wie die uebrigen
    Laeufe; das gehoert je Sonde begruendet.

    `--audio-datei` ist hier HART gesperrt (S74): AvsRef erzeugt sein Audio
    selbst, formelgleich zum synthetischen Signal des Standalones. Hoert nur
    unsere Seite echte Musik, vergleicht man zwei verschiedene Eingaben — jede
    Zahl daraus waere wertlos, sieht aber genauso serioes aus wie eine echte.
    """
    if any(str(a).startswith("--audio-datei") for a in (extra_args or [])):
        raise RuntimeError(
            "--audio-datei ist im Referenzvergleich gesperrt: AvsRef erzeugt "
            "sein Audio selbst (synthetisch). Echte Musik nur fuer Schaufenster "
            "und Augenschein — siehe Kalibrier_Plan_Mitgelieferte_Presets.md, "
            "Aufgabe 6.")
    import os
    env = dict(os.environ)
    env["QT_ENABLE_HIGHDPI_SCALING"] = "0"  # logische == physische Pixel
    extra = ["--beat-period", str(beat_period)] if beat_period > 0 else []
    if muster != "klassisch":
        extra += ["--audio-muster", muster]
    extra += list(extra_args or [])
    proc = messlauf.run(
        [str(LUMI_EXE), str(avs), "--auto", "--frames", str(frames),
         "--size", size, "--out", str(out)] + extra,
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        timeout=300, env=env)
    if proc.returncode != 0:
        raise RuntimeError(f"AvsStandalone rc={proc.returncode}\n"
                           f"{proc.stdout}{proc.stderr}")
    base = avs.name.replace(".", "_")
    png = out / f"{base}_auto.png"
    if not png.exists():
        raise RuntimeError(f"AvsStandalone-PNG fehlt: {png}\n{proc.stdout}")
    return png


def load_rgb(path: Path) -> np.ndarray:
    return np.asarray(Image.open(path).convert("RGB"), dtype=np.float64) / 255.0


def luma(img: np.ndarray) -> np.ndarray:
    return 0.2126 * img[..., 0] + 0.7152 * img[..., 1] + 0.0722 * img[..., 2]


def compare(ref: np.ndarray, lumi: np.ndarray) -> dict:
    d = {}
    d["mean_ref"] = ref.mean(axis=(0, 1))
    d["mean_lumi"] = lumi.mean(axis=(0, 1))
    d["d_mean"] = float(np.abs(d["mean_ref"] - d["mean_lumi"]).max())
    d["d_maxluma"] = float(abs(luma(ref).max() - luma(lumi).max()))
    d["mae"] = float(np.abs(ref - lumi).mean())
    return d


def montage(ref: np.ndarray, lumi: np.ndarray, out_png: Path) -> None:
    """ref | lumiviz | absdiff (Diff 4x verstaerkt) nebeneinander."""
    diff = np.clip(np.abs(ref - lumi) * 4.0, 0.0, 1.0)
    gap = np.ones((ref.shape[0], 4, 3)) * 0.5
    row = np.concatenate([ref, gap, lumi, gap, diff], axis=1)
    Image.fromarray((row * 255).astype(np.uint8)).save(out_png)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("presets", nargs="*", type=Path)
    ap.add_argument("--tests", action="store_true",
                    help="auch ../tests/*.avs (P-Presets) vergleichen")
    ap.add_argument("--frames", type=int, default=120)
    ap.add_argument("--size", default="320x240")
    ap.add_argument("--formate", default=None,
                    help="S81: denselben Vergleich in mehreren Formaten, dazu "
                         "formate.md mit Format-Urteil (FORMATABHAENGIG). "
                         "Bausteine, Komma: aspekt (1:1, 4:3, 16:9, 3,4:1, "
                         "3:4 hoch bei ~gleicher Pixelzahl), monitor (physische "
                         "Groesse des Monitors aus LUMIVIZ_REF_BILDSCHIRM), "
                         "monitor:DISPLAYn, WxH. Ersetzt --size")
    ap.add_argument("--ape-dir", default=str(APE_DIR),
                    help="APE-Verzeichnis fuer AvsRef (leer = keine APEs)")
    # Vorgabe 30 statt 0 (S52): mit dem EIGENEN Beat-Detektor ist keiner der
    # beiden Renderer reproduzierbar — beide Detektoren haengen an der Wanduhr
    # (bpm.cpp ruft GetTickCount, unser BeatEstimator ebenso). Gemessen ueber
    # vier Laeufe, jeder Renderer mit sich selbst verglichen: mit `0` streuten
    # wir bis 0,21 MAE, mit erzwungenem Beat sind wir **0,0000** ueber alle
    # Paarungen. Wer den echten Detektor messen will, setzt ausdruecklich `0` —
    # muss die Werte dann aber mehrfach erheben.
    ap.add_argument("--beat-period", type=int, default=30,
                    help="deterministischer Beat alle N Frames (beide Renderer); "
                         "0 = jeder Renderer erkennt selbst (NICHT reproduzierbar)")
    # Vorgabe 60 (S59): AvsRefs EEL-gettime() liest sonst die WANDUHR — der
    # Batch-Renderer schafft 120 Frames in Sekundenbruchteilen, und ein Preset,
    # das seine eigene Bildrate misst (el-vis_hypno07: 1s-Zaehlfenster ->
    # Laufmittel-Laenge), sieht dort fps=0 statt 60. Unsere Seite tickt fest
    # mit frame/60 (LuaScriptEngine::m_scriptTime) — nur mit derselben
    # Frame-Uhr messen beide dasselbe Preset-Verhalten.
    ap.add_argument("--tick-hz", type=int, default=60,
                    help="gettime()-Frame-Uhr fuer AvsRef (0 = Wanduhr wie im "
                         "Original — bei gettime-Presets NICHT vergleichbar)")
    # S74: das synthetische Signal gibt es in zwei Mustern. `musik` leitet
    # Bandhuellkurven und Beat-Spur aus einer echten Aufnahme ab und
    # synthetisiert daraus neu — BEIDE Renderer binden dazu dieselbe
    # SynthAudio.hpp ein, der Vergleich bleibt also gueltig. Das ist etwas
    # ganz anderes als `--audio-datei` (echte Musik, in run_lumi gesperrt).
    ap.add_argument("--audio-muster", default="klassisch",
                    choices=["klassisch", "musik"],
                    help="synthetisches Signal: klassisch (Sinus+Beat-Puls, "
                         "Vorgabe) oder musik (aus einer Aufnahme abgeleitet). "
                         "Im Muster musik kommt der Beat aus der Beat-SPUR der "
                         "Vorlage, --beat-period wird dann nicht gebraucht")
    ap.add_argument("--out", type=Path,
                    default=ROOT / "../../../out/avsref_compare")
    args = ap.parse_args()

    for exe, name in ((REF_EXE, "AvsRef (tools/AvsRef, -A Win32 bauen)"),
                      (LUMI_EXE, "AvsStandalone (build-ninja-release-clang)")):
        if not exe.exists():
            print(f"FEHLER: {name} fehlt: {exe}")
            return 2

    presets = list(args.presets)
    if not presets:
        presets = sorted(ROOT.glob("s*/**/*.avs"))
        if args.tests:
            presets += sorted((ROOT / "../tests").glob("*.avs"))
    if not presets:
        print("FEHLER: keine Presets gefunden")
        return 2

    out: Path = args.out.resolve()
    if not args.formate:
        _, failures = lauf(presets, args.size, args, out)
        return 1 if failures else 0

    # --formate (S81): derselbe Vergleich je Format, danach EIN Bericht, der
    # die Formate nebeneinanderstellt (messlauf.py erklaert die Reihe)
    try:
        formate = messlauf.formate_aufloesen(args.formate)
    except ValueError as e:
        print(f"FEHLER: {e}")
        return 2
    ergebnisse = {}
    failures = 0
    for bez, groesse in formate:
        print(f"\n=== Format {bez} ({groesse})", flush=True)
        rows, f = lauf(presets, groesse, args, out / groesse)
        failures += f
        ergebnisse[groesse] = {r[0]: r for r in rows}
    formatbericht(presets, formate, ergebnisse, out)
    return 1 if failures else 0


def formatbericht(presets: list, formate: list, ergebnisse: dict,
                  out: Path) -> None:
    """Formate nebeneinander: Urteil + MAE je Format, dazu das Format-Urteil.
    FORMATABHAENGIG = in mindestens einem Format gruen (MAE <= OK_MAE), in
    einem anderen ein Befund (MAE > BEFUND_MAE) — der Fehler haengt an Form
    oder Groesse, nicht am Preset allein. Gleiche Logik wie compare_milkref.py,
    mit den AVS-Schwellen."""
    zeilen_md = []
    abh = 0
    for avs in presets:
        zellen, maes, hinweise = [], [], []
        for _bez, groesse in formate:
            r = ergebnisse.get(groesse, {}).get(avs.name)
            if r is None:
                zellen.append("–")
                continue
            _rel, _dm, _dl, mae, urteil, grund = r
            zellen.append(f"{urteil} {mae:.3f}" if mae is not None else urteil)
            if mae is not None:
                maes.append(mae)
            if grund:
                hinweise.append(f"{groesse}: {grund}")
        if len(maes) >= 2 and min(maes) <= OK_MAE and max(maes) > BEFUND_MAE:
            fu, abh = "FORMATABHAENGIG", abh + 1
        elif len(maes) >= 2:
            fu = f"Spanne {max(maes) - min(maes):.3f}"
        else:
            fu = "–"
        hinweis = "; ".join(hinweise).replace("|", "/")
        zeilen_md.append(f"| {avs.name} | " + " | ".join(zellen)
                         + f" | {fu} | {hinweis} |")
        print(f"  {fu:16s} {avs.name}  " + " · ".join(zellen), flush=True)
    bericht = out / "formate.md"
    with bericht.open("w", encoding="utf-8") as f:
        f.write("# AvsRef-Vergleich über Formate\n\n")
        f.write("Je Zelle: Urteil des Einzellaufs und MAE des Endbilds. "
                f"**FORMATABHAENGIG** = in einem Format grün (MAE ≤ {OK_MAE}), "
                f"in einem anderen Befund (MAE > {BEFUND_MAE}): der Fehler hängt "
                "an Form oder Größe. Beide Seiten rendern nativ (kein "
                "Render-Scale-Divisor). Einzelberichte je Format in den "
                "Unterordnern.\n\n")
        f.write("| Preset | " + " | ".join(f"{b} ({g})" for b, g in formate)
                + " | Format-Urteil | Hinweise |\n")
        f.write("|---" * (len(formate) + 3) + "|\n")
        f.write("\n".join(zeilen_md) + "\n")
    print(f"\nFormat-Bericht: {bericht} — {abh} formatabhaengig")


def pruefe_groesse(img: np.ndarray, size: str, wer: str) -> None:
    """Das Bild muss GENAU die verlangte Groesse haben (S81). Sonst waere die
    Zahl ein Vergleich zweier anderer Formate — sauber melden statt messen.
    Typisch: das Standalone-Fenster passt nicht auf den Monitor (Windows
    kuerzt es auf die Arbeitsflaeche) oder High-DPI skaliert."""
    w, h = (int(v) for v in size.lower().split("x"))
    if img.shape[1] != w or img.shape[0] != h:
        raise RuntimeError(
            f"{wer} lieferte {img.shape[1]}x{img.shape[0]} statt {w}x{h}"
            + (" — Fenstergroesse vom System geaendert (Mindestbreite der "
               "Titelleiste, Monitorgrenze)? DPI-Skalierung?"
               if wer == "AvsStandalone" else ""))


def lauf(presets: list, size: str, args, out: Path) -> tuple[list, int]:
    """EIN Vergleichslauf in EINEM Format; schreibt report.md.

    @return (rows, failures) — rows = [(name, dMean, dMaxLuma, MAE, urteil,
            grund)]; Metriken None bei FEHLER, grund = Fehlertext (kurz).
    """
    (out / "ref").mkdir(parents=True, exist_ok=True)
    (out / "lumi").mkdir(exist_ok=True)
    (out / "montage").mkdir(exist_ok=True)

    rows = []
    failures = 0
    # ASCII-Kopien teilen sich alle Formate — sie haengen nicht an der Groesse
    stage = args.out.resolve() / "_ascii"
    for avs in presets:
        rel = avs.name
        try:
            # Rendern IMMER ueber den ASCII-sicheren Pfad (s. ascii_safe); der
            # Originalname bleibt fuer Bericht und Montage.
            src = ascii_safe(avs, stage)
            ref_img = load_rgb(run_ref(src, args.frames, size, out / "ref",
                                       args.beat_period, args.ape_dir,
                                       args.tick_hz,
                                       muster=args.audio_muster))
            lumi_img = load_rgb(run_lumi(src, args.frames, size, out / "lumi",
                                         args.beat_period,
                                         muster=args.audio_muster))
            pruefe_groesse(ref_img, size, "AvsRef")
            pruefe_groesse(lumi_img, size, "AvsStandalone")
            d = compare(ref_img, lumi_img)
            mont = out / "montage" / f"{avs.stem}.png"
            montage(ref_img, lumi_img, mont)
            urteil = "OK" if (d["d_mean"] <= OK_DMEAN
                              and d["d_maxluma"] <= OK_DMAXLUMA
                              and d["mae"] <= OK_MAE) else "PRUEFEN"
            rows.append((rel, d["d_mean"], d["d_maxluma"], d["mae"], urteil, ""))
            print(f"  {urteil:7s} {rel}  dMean={d['d_mean']:.3f} "
                  f"dMaxLuma={d['d_maxluma']:.3f} MAE={d['mae']:.3f}")
        except Exception as e:  # noqa: BLE001 — je Preset weitermachen
            failures += 1
            # Kurzgrund fuer formate.md: die ersten zwei Zeilen (rc + Meldung)
            teile = [z.strip() for z in str(e).splitlines() if z.strip()]
            grund = " / ".join(teile[:2])[:160] or type(e).__name__
            rows.append((rel, None, None, None, "FEHLER", grund))
            print(f"  FEHLER  {rel}: {e}")

    report = out / "report.md"
    with report.open("w", encoding="utf-8") as f:
        f.write(f"# AvsRef-Vergleich — {args.frames} Frames, {size}\n\n")
        f.write("| Preset | dMean | dMaxLuma | MAE | Urteil |\n|---|---|---|---|---|\n")
        for rel, dm, dl, mae, urteil, _grund in rows:
            if dm is None:
                f.write(f"| {rel} | – | – | – | {urteil} |\n")
            else:
                f.write(f"| {rel} | {dm:.3f} | {dl:.3f} | {mae:.3f} | {urteil} |\n")
        f.write("\nMontagen (ref | lumiviz | 4x-Diff): `montage/`\n")
    print(f"\nReport: {report}")
    pruefen = sum(1 for r in rows if r[4] != "OK")
    print(f"{len(rows) - pruefen}/{len(rows)} OK, {pruefen} zu pruefen"
          f" (davon {failures} Fehler)")
    return rows, failures


if __name__ == "__main__":
    sys.exit(main())

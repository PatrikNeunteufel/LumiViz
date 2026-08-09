#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Pimped-Kaleidoscope-Tutorials.

SSOT ist das Tutorial-Markdown (../PimpedKaleidoscope-tutorial.md):
Alle ```glsl-Bloecke werden in Dokumentreihenfolge extrahiert (22 Bloecke,
das Skript prueft die Zahl). Anders als beim Pyramid-Spiral-Vorbild sind die
Schritte ab 6 DIFFS ("nur die geaenderten Funktionen") - dieses Skript baut
daraus die KUMULATIVEN Pass-Shader zusammen: Diff-Konstanten wandern in den
STELLSCHRAUBEN-Block, neue Helfer vor mainImage, das neue mainImage ersetzt
das alte. Schritt 13 (Common + Buffer A + Image) und Anhang A3 werden nach
der Tutorial-Regel "Common wird jedem Pass vorangestellt" komponiert - der
Shadertoy-Node der App hat KEIN eigenes Common-Feld (Schema-Stand S65/S67).

Je Schritt entstehen:
  schritt_NN.glsl                     (Single-Pass, Schritte 1-2, Anhang A1)
  schritt_NN.bufferA.glsl + .image.glsl  (Multipass ab Schritt 3)
  schritt_NN.lvfx                     Multipass-Chain, Schema wie verifiziert:
      Node "shadertoy": code = Image-Pass, imageInput = 4 Kanalbindungen,
      buffers = [{code, input}] fuer Buffer A (Kodierung: -1 nichts,
      0..3 Buffer A..D, 4 Audio; Selbstreferenz liest das VORFRAME).

Screenshots fuer das Tutorial (nach ../pimped_kaleidoscope_bilder/):
  AvsStandalone.exe pimped_kaleidoscope_schritte --auto --frames 300 \
      --size 800x450 --out ../pimped_kaleidoscope_bilder
  (danach <name>_lvfx_auto.png -> <name>.png umbenennen)
Feedback-Hinweis: 300 Frames Anlaufzeit sind Teil des Bildes - der Buffer
startet schwarz (Kaltstart) und braucht ~100 Frames bis zum Gleichgewicht.

LumiViz-Anpassung (BILINEAR_HELFER, dokumentiert im Shader-Kopf): Die
Buffer-FBOs der App filtern NEAREST (Qt-Default) - der bilineare Tiefpass,
den Schritt 6 als Sharpen-Stabilisator voraussetzt, wird deshalb in allen
Paessen mit transformierten Reads (uvZuTex) manuell nachgebaut. Auf
shadertoy.com ist der Tutorial-Code unveraendert korrekt.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
TUTORIAL = HIER.parent / "PimpedKaleidoscope-tutorial.md"

ERWARTETE_BLOECKE = 22

# Kanal-Kodierung des Shadertoy-Nodes (EffectChain.hpp / ShadertoyWrapper.md):
AUDIO = 4
NICHTS = -1

STELL_RE = re.compile(r"^// ---- STELLSCHRAUBEN")
DASH_RE = re.compile(r"^// ----+$")


def finde_mainimage(zeilen: list[str]) -> int:
    """Index der ersten Zeile von mainImage inkl. direkt davorstehender
    Kommentarzeilen (die '// GEAENDERT: ...'-Vorspanne der Diffs)."""
    for i, z in enumerate(zeilen):
        if z.startswith("void mainImage"):
            j = i
            while j > 0 and zeilen[j - 1].startswith("//"):
                j -= 1
            return j
    raise ValueError("kein mainImage gefunden")


def zerlege_diff(block: str) -> tuple[list[str], list[str], list[str]]:
    """Diff-Block -> (Stellschrauben-Zeilen, Helfer-Zeilen, mainImage-Zeilen)."""
    zeilen = block.splitlines()
    konsts: list[str] = []
    rest = zeilen
    for i, z in enumerate(zeilen):
        if STELL_RE.match(z):
            for j in range(i + 1, len(zeilen)):
                if DASH_RE.match(zeilen[j]):
                    konsts = zeilen[i + 1:j]
                    rest = zeilen[j + 1:]
                    break
            break
    mi = finde_mainimage(rest)
    helfer = rest[:mi]
    while helfer and not helfer[0].strip():
        helfer.pop(0)
    while helfer and not helfer[-1].strip():
        helfer.pop()
    return konsts, helfer, rest[mi:]


def wende_diff_an(basis: str, diff: str, entferne_konst: str | None = None) -> str:
    """Kumulativer Schritt: Diff-Konstanten in den STELLSCHRAUBEN-Block der
    Basis, Helfer vor mainImage, mainImage ersetzt. entferne_konst loescht
    eine obsolet gewordene Konstante (Schritt 11: DREH entfaellt)."""
    konsts, helfer, mainimg = zerlege_diff(diff)
    zeilen = basis.splitlines()

    if konsts:
        schluss = None
        gefunden = False
        for i, z in enumerate(zeilen):
            if STELL_RE.match(z):
                gefunden = True
            elif gefunden and DASH_RE.match(z):
                schluss = i
                break
        if schluss is None:
            raise ValueError("Basis ohne STELLSCHRAUBEN-Block")
        zeilen[schluss:schluss] = konsts
    if entferne_konst:
        zeilen = [z for z in zeilen if entferne_konst not in z]

    mi = finde_mainimage(zeilen)
    neu = zeilen[:mi]
    while neu and not neu[-1].strip():
        neu.pop()
    if helfer:
        neu += [""] + helfer
    neu += [""] + mainimg
    return "\n".join(neu).rstrip() + "\n"


BILINEAR_HELFER = """\
// ---- LumiViz-Anpassung (NICHT im Tutorial-Text) ----------------------------
// Die Buffer-Ping-Pongs des Shadertoy-Nodes filtern derzeit NEAREST
// (Qt-FBO-Default, MultiEffectVisualizer::runShadertoy) - der bilineare
// Tiefpass, den Schritt 6 als Stabilisator des Sharpen voraussetzt (und den
// Shadertoy-Buffer mitbringen), fehlt damit: das System kocht in Pixelgriess
// hoch (exakt die "Probe aufs Exempel" aus Schritt 6). lesBilinear() stellt
// die Shadertoy-Lesesemantik im Shader selbst her. Auf shadertoy.com ist
// diese Funktion UNNOETIG - dort steht im Tutorial schlicht texture().
vec3 lesBilinear(vec2 st)
{
    vec2 p  = st * iResolution.xy - 0.5;
    vec2 i  = floor(p);
    vec2 fr = p - i;
    vec2 px = 1.0 / iResolution.xy;
    vec2 b  = (i + 0.5) * px;
    vec3 c00 = texture(iChannel0, b).rgb;
    vec3 c10 = texture(iChannel0, b + vec2(px.x, 0.0)).rgb;
    vec3 c01 = texture(iChannel0, b + vec2(0.0, px.y)).rgb;
    vec3 c11 = texture(iChannel0, b + px).rgb;
    return mix(mix(c00, c10, fr.x), mix(c01, c11, fr.x), fr.y);
}
// ---- Ende LumiViz-Anpassung ------------------------------------------------
"""

TEX0_RE = re.compile(
    r"texture\(iChannel0,\s*((?:[^()]|\([^()]*\))*)\)\.rgb")


def lumiviz_bilinear_fix(code: str) -> str:
    """Ersetzt transformierte iChannel0-Reads durch manuelles bilineares
    Lesen (nur Paesse mit uvZuTex - Passthrough-Reads auf Texelzentren und
    Audio-Reads auf iChannel1 bleiben unberuehrt)."""
    if "uvZuTex" not in code:
        return code
    neu = TEX0_RE.sub(r"lesBilinear(\1)", code)
    return BILINEAR_HELFER + "\n" + neu


def mit_common(common: str, passcode: str) -> str:
    """Common jedem Pass voranstellen (der Node hat kein Common-Feld)."""
    trenner = ("// ==== Ende Common - ab hier der Pass-eigene Code "
               "=========================\n")
    return common.rstrip() + "\n\n" + trenner + "\n" + passcode.rstrip() + "\n"


def ersetze(text: str, alt: str, neu: str, wo: str) -> str:
    if alt not in text:
        raise ValueError(f"Anker nicht gefunden ({wo}): {alt[:60]!r}")
    return text.replace(alt, neu, 1)


def chain(name: str, beschreibung: str, image_code: str, image_input: list[int],
          buffer_code: str | None = None,
          buffer_input: list[int] | None = None) -> dict:
    node: dict = {
        "type": "shadertoy",
        "name": name,
        "description": beschreibung
        + " (generiert aus PimpedKaleidoscope-tutorial.md, SSOT dort)",
        "imageInput": image_input,
        "blend": 0,
        "code": image_code,
    }
    if buffer_code is not None:
        node["buffers"] = [{"code": buffer_code, "input": buffer_input}]
    return {
        "header": {
            "formatVersion": 1,
            "generator": "LumiViz make_schritte (Pimped-Kaleidoscope-Tutorial)",
        },
        "root": {"type": "list", "clearEveryFrame": False, "children": [node]},
    }


def main() -> int:
    text = TUTORIAL.read_text(encoding="utf-8")
    b = re.findall(r"```glsl\n(.*?)```", text, re.DOTALL)
    if len(b) != ERWARTETE_BLOECKE:
        print(f"FEHLER: {len(b)} glsl-Bloecke gefunden, erwartet "
              f"{ERWARTETE_BLOECKE} - Tutorial geaendert?")
        return 1
    b = [""] + b  # 1-basiert wie im Dokument gezaehlt

    # ---- kumulative Pass-Staende -------------------------------------------
    buf3, img3 = b[3], b[4]                       # Schritt 3 (naiv, brennt aus)
    buf4, img4 = b[5], b[6]                       # Schritt 4 (Decay)
    buf5 = b[7]                                   # Schritt 5 (Zoom/Drehung)
    buf6 = wende_diff_an(buf5, b[8])              # Schritt 6 (Sharpen)
    buf7 = wende_diff_an(buf6, b[9])              # Schritt 7 (Dither)
    img8 = b[10]                                  # Schritt 8 (Winkel-Faltung)
    img9 = wende_diff_an(img8, b[11])             # Schritt 9 (Spiegel-Kachel)
    img10 = wende_diff_an(img9, b[12])            # Schritt 10 (anz-Schleife)
    buf11 = wende_diff_an(buf7, b[13],            # Schritt 11 (Kamera);
                          entferne_konst="const float DREH")  # DREH entfaellt
    img12 = wende_diff_an(img10, b[14])           # Schritt 12 (Politur)

    common13, buf13, img13 = b[15], b[16], b[17]  # Schritt 13 (Gesamtlisting)
    buf13v = mit_common(common13, buf13)
    img13v = mit_common(common13, img13)

    # ---- Anhang A3: Audio-Mappings auf dem Schritt-13-Stand ----------------
    common_a3 = common13.rstrip() + "\n\n" + b[19].rstrip() + "\n"
    buf_a3 = buf13
    buf_a3 = ersetze(
        buf_a3,
        "// ---- der Kreislauf",
        "// ---- Audio (Anhang A3) -----------------------------------------"
        "------------\n\n"
        "// bandLevel fuer DIESEN Pass (BAND-Makro aus dem Common setzt den "
        "Kanal ein)\n"
        "float bandLevel(float lo, float hi) { BAND(iChannel1, lo, hi, 12) "
        "return sum; }\n\n"
        + b[21].rstrip() + "\n\n"
        + "// ---- der Kreislauf",
        "A3 Buffer: Audio-Helfer")
    buf_a3 = ersetze(
        buf_a3,
        "    float zt = iTime * TEMPO;\n",
        "    float zt = iTime * TEMPO;\n"
        "\n"
        "    // LumiViz-Anpassung (Regel aus Anhang B2, NICHT der Shadertoy-\n"
        "    // Text): App-Uniforms statt FFT-Absolutpegel. Die dB-FFT-Zeile\n"
        "    // des Standalone-Testsignals saettigt bei 1.0 (Sonde S67) -\n"
        "    // Absolut-Schwellen wie smoothstep(0.60, 0.75, ...) koennen\n"
        "    // dort nie mehr schalten; `beat` ersetzt das handkalibrierte\n"
        "    // Gate, exakt wie B2 es vorschreibt. Auf shadertoy.com gilt\n"
        "    // weiter der Tutorial-Text (bandLevel + Schwellen).\n"
        "    float gBass = bass;\n"
        "    float gMid  = mid;\n"
        "    float gTreb = treb;\n"
        "    float gVol  = vol;\n"
        "    float gGate = beat;\n"
        "\n"
        "    // [2] Decay atmet mit der Lautheit (Klammer nach oben ist "
        "Pflicht!)\n"
        "    float decay = min(DECAY + 0.07 * gVol, 0.97);\n",
        "A3 Buffer: Pegel")
    buf_a3 = ersetze(
        buf_a3,
        "    alt += (alt - blur4(st)) * SHARPEN;\n",
        "    // [3] Sharpen mit den Hoehen\n"
        "    alt += (alt - blur4(st)) * SHARPEN * (0.5 + 1.8 * gTreb);\n",
        "A3 Buffer: Sharpen")
    buf_a3 = ersetze(
        buf_a3,
        "    vec3 neu = max(alt * DECAY + seeds(uv), 0.0);\n",
        "    // [1]+[6] Saat: Beat-Zuendung + Wellenform-Seed\n"
        "    vec3 saat = seeds(uv) * (1.0 + 2.5 * gGate);\n"
        "    saat += waveSeed(uv, fragCoord);\n"
        "\n"
        "    vec3 neu = max(alt * decay + saat, 0.0);\n",
        "A3 Buffer: Saat")
    img_a3 = b[17]
    img_a3 = ersetze(
        img_a3,
        "// ---- Anzeige + Politur",
        "// bandLevel fuer DIESEN Pass (BAND-Makro aus dem Common)\n"
        "float bandLevel(float lo, float hi) { BAND(iChannel1, lo, hi, 12) "
        "return sum; }\n\n"
        "// ---- Anzeige + Politur",
        "A3 Image: bandLevel")
    img_a3 = ersetze(
        img_a3,
        "    // Farbrotation ueber die Zeit\n"
        "    col *= 0.80 + 0.20 * cos(iTime * 0.07 + vec3(0.0, 2.1, 4.2));\n",
        "    // [4b] Farbrotation ueber die Zeit - hoert auf die Mitten\n"
        "    // (LumiViz-Anpassung wie in Buffer A: mid-Uniform statt "
        "bandLevel)\n"
        "    float gMid = mid;\n"
        "    col *= 0.80 + 0.20 * cos(iTime * 0.07 + gMid * 2.0 "
        "+ vec3(0.0, 2.1, 4.2));\n",
        "A3 Image: Farbrotation")
    buf_a3v = mit_common(common_a3, buf_a3)
    img_a3v = mit_common(common_a3, img_a3)

    # ---- Ausgabe -----------------------------------------------------------
    SELBST = [0, NICHTS, NICHTS, NICHTS]          # Buffer A liest sich selbst
    LIEST_A = [0, NICHTS, NICHTS, NICHTS]         # Image liest Buffer A
    A3_KANAELE = [0, AUDIO, NICHTS, NICHTS]       # + Audio auf iChannel1

    schritte: list[tuple[str, str, str, list[int], str | None, list[int] | None]] = [
        ("schritt_01", "Schritt 1 - Die Buehne: ein wanderndes Licht",
         b[1], [NICHTS] * 4, None, None),
        ("schritt_02", "Schritt 2 - Die Lichtsaat: Punkte-Paar, Ring, Palette",
         b[2], [NICHTS] * 4, None, None),
        ("schritt_03", "Schritt 3 - Buffer A: Gedaechtnis ohne Bilanz "
         "(brennt absichtlich aus)", img3, LIEST_A, buf3, SELBST),
        ("schritt_04", "Schritt 4 - Decay: die Vergessenskurve",
         img4, LIEST_A, buf4, SELBST),
        ("schritt_05", "Schritt 5 - Lese-Transformation: Zoom und Drehung",
         img4, LIEST_A, buf5, SELBST),
        ("schritt_06", "Schritt 6 - Sharpen: die Unsharp Mask",
         img4, LIEST_A, buf6, SELBST),
        ("schritt_07", "Schritt 7 - Die Dither-Saat",
         img4, LIEST_A, buf7, SELBST),
        ("schritt_08", "Schritt 8 - Winkel-Faltung: das klassische Kaleidoskop",
         img8, LIEST_A, buf7, SELBST),
        ("schritt_09", "Schritt 9 - Die Spiegel-Kachel",
         img9, LIEST_A, buf7, SELBST),
        ("schritt_10", "Schritt 10 - Rotations-Ueberlagerung: die anz-Schleife",
         img10, LIEST_A, buf7, SELBST),
        ("schritt_11", "Schritt 11 - Die unsichtbare Kamera",
         img10, LIEST_A, buf11, SELBST),
        ("schritt_12", "Schritt 12 - Politur: Farbrotation, Entsaettigung, "
         "Tonemapping", img12, LIEST_A, buf11, SELBST),
        ("schritt_13", "Schritt 13 - Der fertige Shader (Common in beide "
         "Paesse kopiert)", img13v, LIEST_A, buf13v, SELBST),
        ("anhang_a1", "Anhang A1 - Das Beat-Gate (Rock-The-House-Trick)",
         b[18], [AUDIO, NICHTS, NICHTS, NICHTS], None, None),
        ("anhang_a3", "Anhang A3 - Das Kaleidoskop hoert zu (Audio-Mappings "
         "+ Wellenform-Seed)", img_a3v, A3_KANAELE, buf_a3v, A3_KANAELE),
    ]

    for name, beschr, img, img_in, buf, buf_in in schritte:
        img = lumiviz_bilinear_fix(img)
        if buf is not None:
            buf = lumiviz_bilinear_fix(buf)
        if buf is None:
            (HIER / f"{name}.glsl").write_text(img, encoding="utf-8")
        else:
            (HIER / f"{name}.bufferA.glsl").write_text(buf, encoding="utf-8")
            (HIER / f"{name}.image.glsl").write_text(img, encoding="utf-8")
        doc = chain(name, beschr, img, img_in, buf, buf_in)
        (HIER / f"{name}.lvfx").write_text(
            json.dumps(doc, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8")
        art = "Single-Pass" if buf is None else "Multipass A+Image"
        print(f"{name}.lvfx: {art}, Image {len(img)} Z."
              + (f", Buffer {len(buf)} Z." if buf else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())

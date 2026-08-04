#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Composite-Postfx-Tutorials.

SSOT ist das Tutorial-Markdown (../CompositePostfx-tutorial.md): Alle
```glsl-Bloecke werden in Dokumentreihenfolge extrahiert (39 Bloecke, das
Skript prueft die Zahl). Das Tutorial arbeitet ab Schritt 4 mit Diffs bzw.
Teil-Bloecken ("nur die geaenderten Tabs"); dieses Skript setzt daraus die
KUMULATIVEN Pass-Staende zusammen (Muster:
../pimped_kaleidoscope_schritte/make_schritte.py). Schritt 13 (Common +
Buffer A + B + C + Image) ist der Fixpunkt und wird woertlich uebernommen;
Anhang A3 wird als Diff auf dem Schritt-13-Stand aufgebaut. Der Common-Tab
wird nach der Tutorial-Regel jedem Pass vorangestellt - der Shadertoy-Node
der App hat KEIN eigenes Common-Feld (Schema-Stand S65/S67).

Je Schritt entstehen:
  schritt_NN.glsl                        (Single-Pass: Schritt 1, Anhang A1)
  schritt_NN.bufferA/.bufferB/.bufferC/.image.glsl  (Multipass ab Schritt 2)
  schritt_NN.lvfx                        Multipass-Chain, Schema wie
      verifiziert: Node "shadertoy", code = Image-Pass, imageInput =
      4 Kanalbindungen, buffers = [{code, input}] in Reihenfolge A -> C
      (Kodierung: -1 nichts, 0..3 Buffer A..D, 4 Audio; Selbstreferenz
      liest das VORFRAME, Frame 0 schwarz).

Topologie des Endstands: Buffer A liest SICH SELBST (Temporal, ab
Schritt 10), Buffer B liest A, Buffer C liest B, Image liest A (iChannel0)
und C (iChannel1); Anhang A3 zusaetzlich Audio auf iChannel3 in A, B, Image.

Screenshots fuer das Tutorial (aus dem tutorials-Ordner):
  AvsStandalone.exe composite_postfx_schritte --auto --frames 300 \
      --size 800x450 --out composite_postfx_bilder
  (danach <name>_lvfx_auto.png -> <name>.png umbenennen)

RENDER-STAENDE (dokumentierte Abweichungen vom nackten Kumulativ-Stand,
damit das Bild zeigt, was der Schritt-Text beschreibt - die Markdown-
Codebloecke bleiben unveraendert):
  - Schritt 3: ANSICHT = 1 (Kuechen-Monitor Tiefe - das "Ergebnis" des
    Schritts ist das Tiefenrelief).
  - Schritt 4/5: ANSICHT = 2 (Bloom-Leitung - Bright-Pass bzw. H-Blur;
    mit ANSICHT = 0 waere das Bild laut Tutorial "noch alles beim Alten").
  - Schritt 9: FALT_VOR_BLOOM = 1.0 (der neue Schalter AN; Schritt 8 zeigt
    bereits die 0.0-Variante).
  - Ab Schritt 10: FINISH = 0 (Schritt 8 setzt im Common FINISH = 1; die
    Schritt-Texte 10-12 beschreiben aber das ungefaltete Bild, und der
    Schritt-13-Fixpunkt steht auf 0 - der Wechsel ist hier verortet).
  - SCHWELLE 0.7 -> 0.3 in ALLEN Chains ab Schritt 4 (Kalibrier-Befund des
    Gegen-Renderns, Schritt-Texte unveraendert): In der dark-Stimmung liegt
    KEINE Lichtquelle lum-gewichtet ueber 0.7 - die Fenster (1.4, 0.17,
    0.11) haben lum 0.53, die Korona-Spitze 0.50; die "~1.4" des Tutorial-
    Texts ist der Rot-KANAL, nicht die Leuchtdichte. Gemessen: Bright-Pass
    mit 0.7 lieferte mean 0.000 / max 0.47 (nur Glow-Spikes), nach dem
    Blur max 0.05 - das Bloom war unsichtbar. 0.3 ist die im Schritt-4-
    Experimentier-Kasten genannte Variante; damit gluehen Fenster (Faktor
    ~0.35) und Silhouetten-Glow sichtbar.

LumiViz-Anpassungen (NUR in den generierten Dateien, im Shader-Kopf
dokumentiert; die Markdown-Bloecke bleiben Shadertoy-treu):
  1. lesBilinear0/1(): Die Buffer-FBOs der App filtern NEAREST (Qt-Default)
     - Shadertoy liest bilinear. Alle Lesungen mit Zwischenpositionen
     (Gauss-Taps in Promille-Abstaenden, DOF-Gather, gefaltete Koordinaten)
     lesen deshalb manuell bilinear. Texelzentren-Lesungen (Passthrough,
     Temporal-Selbstlesung) bleiben texture().
  2. Anhang A1/A3: App-Audio-Uniforms (bass/mid/treb/vol/beat) statt der
     FFT-Absolutschwellen - die dB-FFT des Standalone-Testsignals saettigt
     bei 1.0, Absolut-Gates wie smoothstep(0.60, 0.75, gBass) koennen dort
     nie schalten. Anhang A1 bekommt zusaetzlich ein Sichtpruefstand-
     mainImage (der Tutorial-Block ist reine Infrastruktur ohne mainImage).
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
TUTORIAL = HIER.parent / "CompositePostfx-tutorial.md"

ERWARTETE_BLOECKE = 39

# Kanal-Kodierung des Shadertoy-Nodes (EffectChain / ShadertoyWrapper.md):
AUDIO = 4
NICHTS = -1
KEINE = [NICHTS, NICHTS, NICHTS, NICHTS]


def ersetze(text: str, alt: str, neu: str, wo: str) -> str:
    if alt not in text:
        raise ValueError(f"Anker nicht gefunden ({wo}): {alt[:70]!r}")
    if text.count(alt) != 1:
        raise ValueError(f"Anker mehrdeutig ({wo}): {alt[:70]!r}")
    return text.replace(alt, neu, 1)


def einfuegen_nach(text: str, anker: str, block: str, wo: str) -> str:
    """Fuegt block (mit eigener Zeile) direkt nach der Anker-Zeile ein."""
    return ersetze(text, anker, anker + "\n" + block.rstrip("\n"), wo)


# ---- LumiViz-Anpassung 1: manuelles bilineares Lesen ------------------------

BILINEAR_KOPF = """\
// ---- LumiViz-Anpassung (NICHT im Tutorial-Text) ----------------------------
// Die Buffer-Ping-Pongs des Shadertoy-Nodes filtern derzeit NEAREST
// (Qt-FBO-Default, MultiEffectVisualizer::runShadertoy) - shadertoy.com
// liest bilinear. Lesungen dieses Passes mit Zwischenpositionen (Gauss-Taps
// in Promille-Abstaenden, DOF-Gather, gefaltete Koordinaten) laufen deshalb
// ueber ein manuelles bilineares Lesen. Auf shadertoy.com ist das UNNOETIG -
// dort steht im Tutorial schlicht texture().
"""

BILINEAR_FUNK = """\
vec4 lesBilinear{k}(vec2 st)
{{
    vec2 p  = st * iResolution.xy - 0.5;
    vec2 i  = floor(p);
    vec2 fr = p - i;
    vec2 px = 1.0 / iResolution.xy;
    vec2 b  = (i + 0.5) * px;
    vec4 c00 = texture(iChannel{k}, b);
    vec4 c10 = texture(iChannel{k}, b + vec2(px.x, 0.0));
    vec4 c01 = texture(iChannel{k}, b + vec2(0.0, px.y));
    vec4 c11 = texture(iChannel{k}, b + px);
    return mix(mix(c00, c10, fr.x), mix(c01, c11, fr.x), fr.y);
}}
"""

BILINEAR_ENDE = ("// ---- Ende LumiViz-Anpassung ------------------------"
                 "------------------------\n")


def bilinear_fix(code: str, kanaele: str) -> str:
    """Ersetzt texture(iChannelK, ...)-Lesungen (K in kanaele) durch
    lesBilinearK(...) - balancierte Klammern, damit verschachtelte
    Aufrufe wie uvZuTex(... vec2(...) ...) sauber erfasst werden - und
    stellt die Helfer voran. Nur fuer Paesse mit transformierten Reads
    aufrufen; Texelzentren-Paesse bleiben unberuehrt."""
    marker = "texture(iChannel"
    teile: list[str] = []
    pos = 0
    benutzt: set[str] = set()
    while True:
        j = code.find(marker, pos)
        if j < 0:
            teile.append(code[pos:])
            break
        teile.append(code[pos:j])
        kanal = code[j + len(marker)]
        start = j + len("texture")          # Index der oeffnenden Klammer
        tiefe = 0
        ende = -1
        for idx in range(start, len(code)):
            if code[idx] == "(":
                tiefe += 1
            elif code[idx] == ")":
                tiefe -= 1
                if tiefe == 0:
                    ende = idx
                    break
        if ende < 0:
            raise ValueError("unbalancierte Klammern bei texture(...)")
        if kanal in kanaele:
            args = code[start + 1:ende].split(",", 1)[1].strip()
            teile.append(f"lesBilinear{kanal}({args})")
            benutzt.add(kanal)
        else:
            teile.append(code[j:ende + 1])
        pos = ende + 1
    if not benutzt:
        raise ValueError(f"bilinear_fix: keine iChannel[{kanaele}]-Lesung "
                         "gefunden - Kanal-Zuordnung pruefen")
    helfer = BILINEAR_KOPF + "".join(
        BILINEAR_FUNK.format(k=k) for k in sorted(benutzt)) + BILINEAR_ENDE
    return helfer + "\n" + "".join(teile)


def mit_common(common: str, passcode: str) -> str:
    """Common jedem Pass voranstellen (der Node hat kein Common-Feld)."""
    trenner = ("// ==== Ende Common - ab hier der Pass-eigene Code "
               "=========================\n")
    return common.rstrip() + "\n\n" + trenner + "\n" + passcode.rstrip() + "\n"


def chain(name: str, beschreibung: str, image_code: str, image_input: list[int],
          buffers: list[tuple[str, list[int]]]) -> dict:
    node: dict = {
        "type": "shadertoy",
        "name": name,
        "description": beschreibung
        + " (generiert aus CompositePostfx-tutorial.md, SSOT dort)",
        "imageInput": image_input,
        "blend": 0,
        "code": image_code,
    }
    if buffers:
        node["buffers"] = [{"code": c, "input": inp} for c, inp in buffers]
    return {
        "header": {
            "formatVersion": 1,
            "generator": "LumiViz make_schritte (Composite-Postfx-Tutorial)",
        },
        "root": {"type": "list", "clearEveryFrame": False, "children": [node]},
    }


# ---- LumiViz-Anpassung 2: App-Audio-Uniforms (Anhang A1/A3) -----------------

GGATE_ZEILE = ("    gGate = smoothstep(0.60, 0.75, gBass);   "
               "// Beat-Gate (Skala: Handarbeit, s. Schablone)")

AUDIO_OVERRIDE = GGATE_ZEILE + """

    // LumiViz-Anpassung (die B-Regel der Serie, NICHT der Shadertoy-Text):
    // App-Uniforms statt FFT-Absolutpegel. Die dB-FFT-Zeile des Standalone-
    // Testsignals saettigt bei 1.0 (Sonde S67) - Absolut-Schwellen wie
    // smoothstep(0.60, 0.75, ...) koennen dort nie mehr schalten; `beat`
    // ersetzt das handkalibrierte Gate. Auf shadertoy.com gilt der
    // Tutorial-Text oben unveraendert.
    gBass = bass;
    gMid  = mid;
    gTreb = treb;
    gVol  = vol;
    gGate = beat;
}"""

A1_KOPF = """\
// ============================================================================
// Anhang A1 - die Audio-Infrastruktur (Common-Block des Tutorials) am
// Sichtpruefstand. Der Tutorial-Block definiert nur Globals + bandLevel +
// audioFuellen; das mainImage unten ist eine LumiViz-Zugabe (NICHT im
// Tutorial-Text), damit die Chain etwas zeigt: fuenf Balken fuer
// gBass/gMid/gTreb/gVol/gGate, darueber die rohe FFT-Zeile als Kurve
// (die im Standalone-Testsignal dB-gesaettigt am Deckel klebt - die
// Skalen-Falle aus dem Tutorial-Text, live).
// ============================================================================
"""

A1_MAIN = """\
// ---- LumiViz-Sichtpruefstand (NICHT im Tutorial-Text) ----------------------
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 st = fragCoord / iResolution.xy;

    audioFuellen();

    float wert = st.x < 0.2 ? gBass : st.x < 0.4 ? gMid : st.x < 0.6 ? gTreb
               : st.x < 0.8 ? gVol  : gGate;

    float f = fract(st.x * 5.0);
    float balken = step(st.y, wert) * step(0.03, f) * step(f, 0.97);

    // rohe FFT-Zeile als Referenzkurve (Zeile 0 der 512x2-Audio-Textur)
    float fft = texture(iChannel3, vec2(st.x, 0.25)).x;
    float kurve = smoothstep(0.008, 0.0, abs(st.y - fft));

    vec3 farbe = mix(vec3(0.20, 0.45, 1.00), vec3(1.00, 0.55, 0.15), st.x);
    fragColor = vec4(farbe * balken + vec3(0.9, 0.95, 1.0) * kurve * 0.5
                     + 0.02, 1.0);
}
"""


def main() -> int:
    text = TUTORIAL.read_text(encoding="utf-8")
    b = re.findall(r"```glsl\n(.*?)```", text, re.DOTALL)
    if len(b) != ERWARTETE_BLOECKE:
        print(f"FEHLER: {len(b)} glsl-Bloecke gefunden, erwartet "
              f"{ERWARTETE_BLOECKE} - Tutorial geaendert?")
        return 1
    b = [""] + b  # 1-basiert wie im Dokument gezaehlt

    # Stichproben, dass die Block-Zuordnung noch stimmt (Struktur normativ):
    proben = {
        1: "// Schritt 1: das Szenen-Skelett",
        2: "// COMMON - wird jedem Pass vorangestellt",
        8: "// BUFFER B (v1)",
        11: "// BUFFER B (v2)",
        13: "// BUFFER C",
        17: "// GEAENDERT: mainImage - der Anfang bis zur Bloom-Mischung",
        23: "// GEAENDERT: mainImage - Temporal-Mix",
        30: '// "Composite: PostFX"',
        31: "// BUFFER A - die Szene: der kondensierte Moloch.",
        35: "// ---- AUDIO (Common)",
    }
    for nr, muster in proben.items():
        if muster not in b[nr]:
            print(f"FEHLER: Block {nr} traegt nicht {muster!r} - "
                  "Reihenfolge im Tutorial geaendert?")
            return 1

    STELL_ENDE = ("// ------------------------------------------------------"
                  "----------------------")

    # ---- Common: kumulative Staende ----------------------------------------
    c = {}
    c[2] = b[2]
    c[3] = ersetze(
        c[2],
        "// ---- STELLSCHRAUBEN ------------------------------------------"
        "--------------\n// Szene (Buffer A)",
        "// ---- STELLSCHRAUBEN ------------------------------------------"
        "--------------\n" + b[5].rstrip() + "\n// Szene (Buffer A)",
        "c3 ANSICHT")
    c[4] = ersetze(c[3], "// 0 = fertiges Bild, 1 = Tiefe (Debug)",
                   "// 0 = fertig, 1 = Tiefe, 2 = Bloom-Leitung", "c4 Monitor")
    c[4] = ersetze(c[4], "\n" + STELL_ENDE,
                   "\n" + b[9].rstrip() + "\n" + STELL_ENDE, "c4 Bloom")

    SCHWELLE_ALT = ('const float SCHWELLE  = 0.7;   // Bright-Pass: ab '
                    'dieser Leuchtdichte "ueberstrahlt"')
    SCHWELLE_NEU = ('const float SCHWELLE  = 0.3;   // Bright-Pass: ab '
                    'dieser Leuchtdichte "ueberstrahlt"  '
                    '[Chain-Kalibrierung: Original 0.7, s. make_schritte.py]')
    c[4] = ersetze(c[4], SCHWELLE_ALT, SCHWELLE_NEU, "c4 Kalibrierung")
    c[5] = einfuegen_nach(
        c[4], "const float KNIE      = 0.5;   // weicher Uebergang oberhalb "
        "der Schwelle", b[12], "c5 Blur")
    c[6] = einfuegen_nach(
        c[5], "const float STREU     = 2.5;   // Tap-Abstand in PROMILLE der "
        "Bildhoehe", b[15], "c6 Staerke")
    dof_konst, dof_taps = b[16].split("\n// fester Abtast-Satz", 1)
    dof_taps = "// fester Abtast-Satz" + dof_taps
    c[7] = einfuegen_nach(
        c[6], "const float BLOOM_STAERKE = 0.7; // Anteil des Blooms im "
        "Endbild", dof_konst, "c7 DOF")
    c[7] = c[7].rstrip() + "\n\n" + dof_taps.rstrip() + "\n"
    falt_konst, falt_funk = b[18].split("\n\n// Winkel-Faltung", 1)
    falt_funk = "// Winkel-Faltung" + falt_funk
    c[8] = einfuegen_nach(
        c[7], "const float COC_MAX   = 0.015; // Sicherheitsdeckel des "
        "Zerstreuungskreises", falt_konst, "c8 Finish")
    c[8] = ersetze(c[8], "// fester Abtast-Satz",
                   falt_funk.rstrip() + "\n\n// fester Abtast-Satz",
                   "c8 Faltungen")
    c[9] = einfuegen_nach(
        c[8], "const float KACHEL    = 1.2;   // Kacheln pro Bildhoehe "
        "(FINISH 2)", b[20], "c9 Weiche")
    c[10] = einfuegen_nach(
        c[9], 'const float TIEFE_MAX = 40.0;  // Marsch-Limit ("unendlich '
        'weit weg")', b[24], "c10 Temporal")
    # Render-Stand ab Schritt 10: FINISH wieder aus (Schritt-Texte 10-12
    # beschreiben das ungefaltete Bild; Fixpunkt Schritt 13 steht auf 0).
    c[10] = ersetze(c[10], "const int   FINISH    = 1;",
                    "const int   FINISH    = 0;", "c10 FINISH aus")
    c[11] = einfuegen_nach(
        c[10], "const float FALT_VOR_BLOOM = 0.0; // 1.0 = das Bloom sieht "
        "das GEFALTETE Bild", b[25], "c11 Politur")
    c[12] = ersetze(c[11], STELL_ENDE + "\n",
                    STELL_ENDE + "\n\n" + b[27].rstrip() + "\n", "c12 eff")
    c[13] = ersetze(b[30], SCHWELLE_ALT, SCHWELLE_NEU, "c13 Kalibrierung")

    # ---- Buffer A: kumulative Staende --------------------------------------
    i0 = b[1].index("float smin(")
    i1 = b[1].index("// ---- Hauptprogramm")
    szenen_kern = b[1][i0:i1].rstrip()
    a_kopf, _, a_main = b[3].partition("void mainImage")
    a = {}
    a[2] = (a_kopf.rstrip() + "\n\n" + szenen_kern + "\n\n"
            + "void mainImage" + a_main.rstrip() + "\n")
    a[3] = a[2][:a[2].index("// ---- die ganze Szene")] + b[6].rstrip() + "\n"
    a[10] = (a[3][:a[3].index("// GEAENDERT: die Tiefe faehrt")]
             + b[23].rstrip() + "\n")
    a[13] = b[31]

    # ---- Buffer B ----------------------------------------------------------
    bb = {}
    bb[4] = b[8]
    bb[5] = b[11]
    bb[9] = ersetze(
        bb[5],
        "// die Schwelle aus Schritt 4, jetzt als Funktion je Abgriff\n"
        "vec3 hell(vec2 uv)\n"
        "{\n"
        "    vec3 c = texture(iChannel0, uvZuTex(uv)).rgb;\n"
        "    return c * smoothstep(SCHWELLE, SCHWELLE + KNIE, lum(c));\n"
        "}",
        b[21].rstrip(), "bb9 hell")
    bb[12] = ersetze(
        bb[9],
        "    return c * smoothstep(SCHWELLE, SCHWELLE + KNIE, lum(c));",
        b[28].rstrip("\n"), "bb12 effSchwelle")
    bb[13] = b[32]

    # ---- Buffer C ----------------------------------------------------------
    bc6 = b[13]     # mit "iChannel0 = Buffer B"-Kommentar
    bc13 = b[33]

    # ---- Image: kumulative Staende -----------------------------------------
    i = {}
    i[2] = b[4]
    i[3] = b[7]
    i[4] = ersetze(
        i[3],
        "    // Kuechen-Monitor: ANSICHT = 1 zeigt den Tiefenkanal\n"
        "    if (ANSICHT == 1) { fragColor = vec4(vec3(tiefe / TIEFE_MAX), "
        "1.0); return; }",
        "    vec3 bloomLeitung = texture(iChannel1, uvZuTex(uv)).rgb;\n"
        "\n"
        "    if (ANSICHT == 1) { fragColor = vec4(vec3(tiefe / TIEFE_MAX), "
        "1.0); return; }\n"
        "    if (ANSICHT == 2) { fragColor = vec4(bloomLeitung, 1.0); "
        "return; }",
        "i4 Monitor")
    i[6] = ersetze(i[4], "    vec3 col   = roh.rgb;\n    float tiefe = roh.a;",
                   "    float tiefe = roh.a;", "i6 roh")
    i[6] = ersetze(
        i[6], "    vec3 bloomLeitung = texture(iChannel1, uvZuTex(uv)).rgb;",
        "    vec3 bloom = texture(iChannel1, uvZuTex(uv)).rgb;\n"
        "    vec3 col   = roh.rgb + bloom * BLOOM_STAERKE;", "i6 bloom")
    i[6] = ersetze(i[6], "vec4(bloomLeitung, 1.0)", "vec4(bloom, 1.0)",
                   "i6 Monitor2")
    schluss = i[6][i[6].index("    if (ANSICHT == 1)"):]
    i[7] = (b[17][:b[17].index("    // ... Monitor + Not-Politur")]
            + schluss)
    i[8] = (b[19][:b[19].index("    // ... Monitor + Not-Politur")]
            + schluss)
    i[9] = ersetze(
        i[8], "    vec3 bloom = texture(iChannel1, uvZuTex(k)).rgb;",
        "    vec2 kBloom = (FALT_VOR_BLOOM > 0.5) ? uv : k;\n"
        "    vec3 bloom  = texture(iChannel1, uvZuTex(kBloom)).rgb;",
        "i9 kBloom")
    anker = "    szeneCol /= float(DOF_TAPS);"
    i[11] = (i[9][:i[9].index(anker) + len(anker)] + "\n\n"
             + b[26].rstrip() + "\n")
    i[12] = ersetze(
        i[11],
        "    float coc = min(BLENDE * abs(tiefe - fokus) / max(tiefe, 1.0), "
        "COC_MAX);",
        "    float coc = min(effBlende() * abs(tiefe - fokus) / "
        "max(tiefe, 1.0), COC_MAX);", "i12 Blende")
    i[12] = ersetze(i[12],
                    "    float dichte = NEBEL * mix(0.0035, 0.0012, "
                    "STIMMUNG);",
                    "    float dichte = effNebelDichte();", "i12 Nebel")
    i[12] = ersetze(i[12],
                    "    vec3 col    = szeneCol + bloom * BLOOM_STAERKE;",
                    "    vec3 col    = szeneCol + bloom * effBloomStaerke();",
                    "i12 Staerke")
    i[13] = b[34]

    # ---- Anhang A1: Audio-Infrastruktur + Sichtpruefstand ------------------
    audio_block = ersetze(b[35].rstrip(), GGATE_ZEILE + "\n}", AUDIO_OVERRIDE,
                          "A1 Uniform-Override")
    a1_code = A1_KOPF + "\n" + audio_block + "\n\n" + A1_MAIN

    # ---- Anhang A3: Diffs auf dem Schritt-13-Stand -------------------------
    c_a3 = einfuegen_nach(
        c[13], "float effNebelDichte()  { return NEBEL * mix(0.0035, 0.0012, "
        "STIMMUNG); }", "\n" + audio_block, "A3 Common Audio")
    c_a3 = ersetze(
        c_a3, "    ang = mod(ang, sektor);",
        "    ang = mod(ang + gMid * 0.5, sektor);     "
        "// [4] Mitten drehen das Mandala", "A3 Mandala")

    MAIN_ANFANG = ("void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
                   "{\n    vec2 uv")

    def mit_audiofuellen(code: str, wo: str) -> str:
        return ersetze(
            code, MAIN_ANFANG,
            "void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
            "{\n    audioFuellen();"
            "                                           "
            "// iChannel3 = Music!\n\n    vec2 uv", wo)

    a_a3 = mit_audiofuellen(a[13], "A3 BufferA audio")
    a_a3 = ersetze(
        a_a3, "    col = mix(col, alt, NACHZIEH);",
        "    float nz = NACHZIEH * (1.0 - 0.6 * min(gVol * 2.0, 1.0));  "
        "// [5] Lautheit kuerzt\n"
        "    col = mix(col, alt, nz);                                   "
        "//     das Gedaechtnis", "A3 Nachzieh")
    bb_a3 = mit_audiofuellen(bb[13], "A3 BufferB audio")
    bb_a3 = ersetze(
        bb_a3,
        "    return c * smoothstep(effSchwelle(), effSchwelle() + KNIE, "
        "lum(c));",
        "    float schwelle = effSchwelle() * (1.0 - 0.5 * gTreb);      "
        "// [3] Glitzer-Explosion\n"
        "    return c * smoothstep(schwelle, schwelle + KNIE, lum(c));",
        "A3 Schwelle")
    i_a3 = mit_audiofuellen(i[13], "A3 Image audio")
    i_a3 = ersetze(
        i_a3, "    float fokus = FOKUS + FOKUS_HUB * sin(iTime * 0.11);",
        "    float fokus = FOKUS + FOKUS_HUB * sin(iTime * 0.11)\n"
        "                - gGate * 2.5;                                 "
        "// [2] Fokus-Kick", "A3 Fokus")
    i_a3 = ersetze(
        i_a3, "    vec3 col    = szeneCol + bloom * effBloomStaerke();",
        "    vec3 col    = szeneCol + bloom * effBloomStaerke()\n"
        "                * (0.5 + 1.5 * gBass);                         "
        "// [1] Bass pumpt Bloom", "A3 Bass")

    # ---- Render-Staende (Begruendung im Docstring) -------------------------
    def ansicht(common: str, wert: int, kommentar: str) -> str:
        return ersetze(common, "const int   ANSICHT   = 0;",
                       f"const int   ANSICHT   = {wert};", kommentar)

    c3_render = ansicht(c[3], 1, "s3 Monitor")
    c4_render = ansicht(c[4], 2, "s4 Monitor")
    c5_render = ansicht(c[5], 2, "s5 Monitor")
    c9_render = ersetze(c[9], "const float FALT_VOR_BLOOM = 0.0;",
                        "const float FALT_VOR_BLOOM = 1.0;", "s9 Schalter")

    # ---- die Schritte ------------------------------------------------------
    # (name, beschreibung, common | None, [(passname, code, input, kanaele)],
    #  image_code, image_input, image_kanaele)
    A_SELBST = [0, NICHTS, NICHTS, NICHTS]
    B_LIEST_A = [0, NICHTS, NICHTS, NICHTS]
    C_LIEST_B = [1, NICHTS, NICHTS, NICHTS]
    IMG_A = [0, NICHTS, NICHTS, NICHTS]
    IMG_AB = [0, 1, NICHTS, NICHTS]
    IMG_AC = [0, 2, NICHTS, NICHTS]

    schritte = [
        ("schritt_01", "Schritt 1 - Kondensieren: das Szenen-Skelett "
         "(Single-Pass-Rohware)",
         None, [], b[1], KEINE, ""),
        ("schritt_02", "Schritt 2 - Der Umzug: Buffer A + Image + Common-SSOT "
         "(Bild unveraendert)",
         c[2], [("bufferA", a[2], KEINE, "")], i[2], IMG_A, ""),
        ("schritt_03", "Schritt 3 - Der Nebenkanal: Tiefe im Alpha "
         "(Render-Stand ANSICHT = 1: Kuechen-Monitor Tiefe)",
         c3_render, [("bufferA", a[3], KEINE, "")], i[3], IMG_A, ""),
        ("schritt_04", "Schritt 4 - Bright-Pass: die Schwelle "
         "(Render-Stand ANSICHT = 2: Bloom-Leitung)",
         c4_render, [("bufferA", a[3], KEINE, ""),
                     ("bufferB", bb[4], B_LIEST_A, "")], i[4], IMG_AB, ""),
        ("schritt_05", "Schritt 5 - Separierbarer Blur I: horizontal "
         "(Render-Stand ANSICHT = 2: Bloom-Leitung)",
         c5_render, [("bufferA", a[3], KEINE, ""),
                     ("bufferB", bb[5], B_LIEST_A, "0")], i[4], IMG_AB, ""),
        ("schritt_06", "Schritt 6 - Separierbarer Blur II: vertikal - "
         "das Bloom steht",
         c[6], [("bufferA", a[3], KEINE, ""),
                ("bufferB", bb[5], B_LIEST_A, "0"),
                ("bufferC", bc6, C_LIEST_B, "0")], i[6], IMG_AC, ""),
        ("schritt_07", "Schritt 7 - Depth of Field: die Tiefe wird Blende "
         "(Gather-DOF light)",
         c[7], [("bufferA", a[3], KEINE, ""),
                ("bufferB", bb[5], B_LIEST_A, "0"),
                ("bufferC", bc6, C_LIEST_B, "0")], i[7], IMG_AC, "01"),
        ("schritt_08", "Schritt 8 - Kaleidoskop als Post I: das Finish "
         "(FINISH = 1 wie im Schritt-Common)",
         c[8], [("bufferA", a[3], KEINE, ""),
                ("bufferB", bb[5], B_LIEST_A, "0"),
                ("bufferC", bc6, C_LIEST_B, "0")], i[8], IMG_AC, "01"),
        ("schritt_09", "Schritt 9 - Faltung vor oder nach dem Bloom? "
         "(Render-Stand FALT_VOR_BLOOM = 1.0)",
         c9_render, [("bufferA", a[3], KEINE, ""),
                     ("bufferB", bb[9], B_LIEST_A, "0"),
                     ("bufferC", bc6, C_LIEST_B, "0")], i[9], IMG_AC, "01"),
        ("schritt_10", "Schritt 10 - Temporal-Glaettung: Buffer A liest sein "
         "Vorframe (FINISH ab hier wieder 0)",
         c[10], [("bufferA", a[10], A_SELBST, ""),
                 ("bufferB", bb[9], B_LIEST_A, "0"),
                 ("bufferC", bc6, C_LIEST_B, "0")], i[9], IMG_AC, "01"),
        ("schritt_11", "Schritt 11 - Die Anrichte: Politur ans Ende der Kette",
         c[11], [("bufferA", a[10], A_SELBST, ""),
                 ("bufferB", bb[9], B_LIEST_A, "0"),
                 ("bufferC", bc6, C_LIEST_B, "0")], i[11], IMG_AC, "01"),
        ("schritt_12", "Schritt 12 - Die STIMMUNGs-Kopplung: eine Blende fuer "
         "die ganze Kueche",
         c[12], [("bufferA", a[10], A_SELBST, ""),
                 ("bufferB", bb[12], B_LIEST_A, "0"),
                 ("bufferC", bc6, C_LIEST_B, "0")], i[12], IMG_AC, "01"),
        ("schritt_13", "Schritt 13 - Der fertige Shader (Gesamtlisting, "
         "Common in alle Paesse kopiert)",
         c[13], [("bufferA", a[13], A_SELBST, ""),
                 ("bufferB", bb[13], B_LIEST_A, "0"),
                 ("bufferC", bc13, C_LIEST_B, "0")], i[13], IMG_AC, "01"),
        ("anhang_a1", "Anhang A1 - Die Audio-Infrastruktur am "
         "Sichtpruefstand (mainImage = LumiViz-Zugabe)",
         None, [], a1_code, [NICHTS, NICHTS, NICHTS, AUDIO], ""),
        ("anhang_a3", "Anhang A3 - Die Kueche hoert zu (Mappings 1-5; "
         "LumiViz: App-Uniforms statt FFT-Schwellen)",
         c_a3, [("bufferA", a_a3, [0, NICHTS, NICHTS, AUDIO], ""),
                ("bufferB", bb_a3, [0, NICHTS, NICHTS, AUDIO], "0"),
                ("bufferC", bc13, C_LIEST_B, "0")],
         i_a3, [0, 2, NICHTS, AUDIO], "01"),
    ]

    for (name, beschr, common, buffer_liste, img, img_in,
         img_kanaele) in schritte:
        passdateien: list[tuple[str, str]] = []
        buffers: list[tuple[str, list[int]]] = []
        for passname, code, inp, kanaele in buffer_liste:
            voll = mit_common(common, code)
            if kanaele:
                voll = bilinear_fix(voll, kanaele)
            passdateien.append((f"{name}.{passname}.glsl", voll))
            buffers.append((voll, inp))
        img_voll = mit_common(common, img) if common is not None else img
        if img_kanaele:
            img_voll = bilinear_fix(img_voll, img_kanaele)
        if buffers:
            passdateien.append((f"{name}.image.glsl", img_voll))
        else:
            passdateien.append((f"{name}.glsl", img_voll))
        for datei, inhalt in passdateien:
            (HIER / datei).write_text(inhalt, encoding="utf-8")
        doc = chain(name, beschr, img_voll, img_in, buffers)
        (HIER / f"{name}.lvfx").write_text(
            json.dumps(doc, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8")
        art = ("Single-Pass" if not buffers
               else f"Multipass A{'+B' if len(buffers) > 1 else ''}"
                    f"{'+C' if len(buffers) > 2 else ''}+Image")
        print(f"{name}.lvfx: {art}, Image {len(img_voll)} Z."
              + "".join(f", {pn.split('.')[1]} {len(cd)} Z."
                        for (pn, cd), _ in zip(passdateien, buffers)))
    return 0


if __name__ == "__main__":
    sys.exit(main())

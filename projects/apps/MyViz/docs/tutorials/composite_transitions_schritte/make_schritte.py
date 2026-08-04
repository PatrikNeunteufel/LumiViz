#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generiert die Schritt-Chains des Composite-Transitions-Tutorials.

SSOT ist das Tutorial-Markdown (../CompositeTransitions-tutorial.md):
Alle ```glsl-Bloecke werden in Dokumentreihenfolge extrahiert (28 Bloecke,
das Skript prueft die Zahl). Die Schritte 3-10 sind Single-Pass und werden
nach den Montage-Regeln des Tutorials kumulativ zusammengesetzt (Schritt 3:
"Schritt 1 + Schritt 2 in einer Datei, #define R nur einmal"; ab Schritt 5
Diff-Modus mit woertlichen Ankern; die Volllistings — Sammelpunkt Schritt 10
und Gesamtlisting Schritt 12 — dienen als Fixpunkte). Ab Schritt 11 ist die
Chain Multipass: Buffer A (1-Pixel-Zustandsmaschine, liest sich selbst =
Vorframe) + Image; das Common des Gesamtlistings wird nach der Tutorial-
Regel "Common wird jedem Pass vorangestellt" komponiert — der Shadertoy-
Node der App hat KEIN eigenes Common-Feld (Schema-Stand S65/S67).

Je Schritt entstehen:
  schritt_NN.glsl                        (Single-Pass, Schritte 1-10)
  schritt_NN.bufferA.glsl + .image.glsl  (Multipass ab Schritt 11, Anhang A)
  schritt_NN.lvfx                        Chain, Schema wie verifiziert:
      Node "shadertoy": code = Image-Pass, imageInput = 4 Kanalbindungen,
      buffers = [{code, input}] fuer Buffer A (Kodierung: -1 nichts,
      0..3 Buffer A..D, 4 Audio; Selbstreferenz liest das VORFRAME).

Screenshots fuer das Tutorial (nach ../composite_transitions_bilder/):
Die Uebergangs-Schritte zeigen ihren Charakter nur MITTEN in der Blende —
deshalb rendert jeder Schritt mit einem eigens gerechneten Frame-Stand
(Sim-Uhr des Standalone: fest 1/60 s je Frame; Screenshot nach --frames N
zeigt iTime = (N-1)/60). Gebuendelte --auto-Laeufe je Frame-Gruppe:

  Schritt          --frames  iTime    Begruendung
  01, 02              300     5.0 s   Skelette eingeschwungen
  04                  390     6.5 s   Dreieck t=0.5 — das naive Doppelbild
  05, 06, 08, 09, 10  780    13.0 s   sin-Uhr-Blende mitten drin (t=0.5)
  07                  810    13.5 s   dieselbe Blende bei t=0.31 (BLENDART
                                      bleibt 1 wie im Text — anderer Frame,
                                      damit sich das Bild von 06 abhebt)
  03, 11, 12         1440    24.0 s   03: nach dem Schnitt (Welt B);
                                      11/12: nach vollzogenem Weltwechsel
                                      (13 s), mitten in der 2. Blende

  AvsStandalone.exe <gruppe> --auto --frames N --size 800x450 \
      --out ../composite_transitions_bilder
  (danach <name>_lvfx_auto.png -> <name>.png umbenennen)

  Beat-Laeufe (a1, a3): NICHT run-deterministisch (der Beat-Detektor der
  App verschiebt die Wechsel-Zeitpunkte von Lauf zu Lauf um Frames) —
  deshalb kommt ihr Bild aus einem --save-every-Lauf desselben Chains mit
  dokumentierter Frame-Wahl aus der mean-RGB-Reihe:
    a1: --frames 1200 --save-every 30 -> f0931 (15.5 s, Phase-Balken hoch,
        mitten in der zweiten Blende nach einem vollzogenen Weltwechsel)
    a3: --frames 1440 --save-every 30 -> f1111 (18.5 s, mitten in einem
        spaeteren beat-getriggerten Wechsel, Kick sichtbar)

LumiViz-Anpassung (NUR in den generierten Anhang-Dateien, die Markdown-
Codebloecke bleiben Shadertoy-treu): Die dB-FFT-Zeile des Standalone-
Testsignals saettigt bei 1.0 (Sonde S67) — der adaptive Beat-Trigger
"bass > glatt*1.35 + 0.02" kann an einem konstant gesaettigten Band nie
feuern. Anhang B1 nennt den Ausweg (glatt/schlag/env aus den App-Uniforms
speisen): `schlag` kommt in den A1-/A3-Chains aus dem `beat`-Uniform der
App. Auf shadertoy.com gilt weiter der Tutorial-Text.
"""

from __future__ import annotations

import json
import re
import sys

from pathlib import Path

HIER = Path(__file__).resolve().parent
TUTORIAL = HIER.parent / "CompositeTransitions-tutorial.md"

ERWARTETE_BLOECKE = 28

# Kanal-Kodierung des Shadertoy-Nodes (EffectChain.hpp / ShadertoyWrapper.md):
AUDIO = 4
NICHTS = -1

STELL_RE = re.compile(r"^// ---- STELLSCHRAUBEN")
DASH_RE = re.compile(r"^// ----+$")


def ersetze(text: str, alt: str, neu: str, wo: str) -> str:
    if alt not in text:
        raise ValueError(f"Anker nicht gefunden ({wo}): {alt[:70]!r}")
    return text.replace(alt, neu, 1)


def split_stell(block: str, wo: str) -> tuple[list[str], str]:
    """Diff-Block -> (Konstanten-Zeilen des STELLSCHRAUBEN-Blocks, Rest)."""
    zeilen = block.splitlines()
    for i, z in enumerate(zeilen):
        if STELL_RE.match(z):
            for j in range(i + 1, len(zeilen)):
                if DASH_RE.match(zeilen[j]):
                    rest = "\n".join(zeilen[j + 1:]).strip("\n")
                    return zeilen[i + 1:j], rest
    raise ValueError(f"kein STELLSCHRAUBEN-Block in {wo}")


def stell_block(zeilen: list[str]) -> str:
    return ("// ---- STELLSCHRAUBEN "
            + "-" * 59 + "\n" + "\n".join(zeilen) + "\n" + "// " + "-" * 76)


def ohne_mainimage(code: str) -> str:
    """Alles vor mainImage (inkl. direkt davorstehender Kommentarzeilen)."""
    zeilen = code.splitlines()
    for i, z in enumerate(zeilen):
        if z.startswith("void mainImage"):
            j = i
            while j > 0 and zeilen[j - 1].startswith("//"):
                j -= 1
            while j > 0 and not zeilen[j - 1].strip():
                j -= 1
            return "\n".join(zeilen[:j]).rstrip() + "\n"
    raise ValueError("kein mainImage gefunden")


def funktion(code: str, kopf: str, wo: str) -> str:
    """Funktionstext von `kopf` bis zur ersten schliessenden Klammer in Sp. 0."""
    m = re.search(re.escape(kopf) + r".*?\n\}\n", code, re.DOTALL)
    if m is None:
        raise ValueError(f"Funktion nicht gefunden ({wo}): {kopf!r}")
    return m.group(0)


def fuege(*teile: str) -> str:
    return "\n".join(t.strip("\n") for t in teile if t.strip()) + "\n"


def chain(name: str, beschreibung: str, image_code: str, image_input: list[int],
          buffer_code: str | None = None,
          buffer_input: list[int] | None = None) -> dict:
    node: dict = {
        "type": "shadertoy",
        "name": name,
        "description": beschreibung
        + " (generiert aus CompositeTransitions-tutorial.md, SSOT dort)",
        "imageInput": image_input,
        "blend": 0,
        "code": image_code,
    }
    if buffer_code is not None:
        node["buffers"] = [{"code": buffer_code, "input": buffer_input}]
    return {
        "header": {
            "formatVersion": 1,
            "generator": "LumiViz make_schritte (Composite-Transitions-Tutorial)",
        },
        "root": {"type": "list", "clearEveryFrame": False, "children": [node]},
    }


def mit_common(common: str, passcode: str) -> str:
    """Common jedem Pass voranstellen (der Node hat kein Common-Feld)."""
    trenner = ("// ==== Ende Common - ab hier der Pass-eigene Code "
               "=========================\n")
    return common.rstrip() + "\n\n" + trenner + "\n" + passcode.rstrip() + "\n"


BEAT_ANPASSUNG_ALT = (
    "    float glatt = mix(audio.x, bass, 0.10);            "
    "// Tiefpass ueber die Zeit\n"
    "    float schlag = step(glatt * 1.35 + 0.02, bass);    "
    "// adaptiver Beat-Trigger\n")
BEAT_ANPASSUNG_NEU = (
    "    float glatt = mix(audio.x, bass, 0.10);            "
    "// Tiefpass ueber die Zeit\n"
    "    // LumiViz-Anpassung (Regel aus Anhang B1, NICHT der Shadertoy-Text):\n"
    "    // Die dB-FFT-Zeile des Standalone-Testsignals saettigt bei 1.0\n"
    "    // (Sonde S67) - `bass` steht damit konstant auf 1.0, und der\n"
    "    // adaptive Trigger `bass > glatt*1.35 + 0.02` kann nie feuern\n"
    "    // (im Probe-Lauf blieb nur der Timer-Fallback). B1 nennt den\n"
    "    // Ausweg: schlag aus dem App-Uniform `beat` speisen (0/1 je Frame,\n"
    "    // BeatEstimator der App). Auf shadertoy.com gilt weiter der\n"
    "    // Tutorial-Text (adaptiver Trigger auf der Music-Textur).\n"
    "    float schlag = step(0.5, beat);\n")


def main() -> int:
    text = TUTORIAL.read_text(encoding="utf-8")
    bl = re.findall(r"```glsl\n(.*?)```", text, re.DOTALL)
    if len(bl) != ERWARTETE_BLOECKE:
        print(f"FEHLER: {len(bl)} glsl-Bloecke gefunden, erwartet "
              f"{ERWARTETE_BLOECKE} - Tutorial geaendert?")
        return 1
    b = [""] + bl  # 1-basiert wie im Dokument gezaehlt

    # ---- Schritt 3: die Montage-Regel des Tutorials ------------------------
    teil_a = ohne_mainimage(b[1])
    teil_b = ohne_mainimage(b[2])
    teil_b = ersetze(teil_b,
                     "#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))\n\n",
                     "", "Schritt 3: doppeltes R raus")
    basis38 = fuege(teil_a, teil_b)          # Welten-Basis der Schritte 3-8
    schritt_03 = fuege(basis38, b[3])

    konst3, main3 = split_stell(b[3], "Block 3")
    del main3  # nur der Vollstaendigkeit halber zerlegt
    schritt_04 = fuege(basis38, stell_block(konst3), b[4])

    # ---- Schritt 5: Phasen-Funktion statt Dreieck --------------------------
    konst5, uebergang_fn = split_stell(b[5], "Block 5")
    main5 = ersetze(
        b[4],
        "    // Dreieck statt Schalter: 0 -> 1 -> 0 ueber die Periode, LINEAR\n"
        "    float u = fract(iTime / PERIODE);\n"
        "    float t = 1.0 - abs(2.0 * u - 1.0);\n",
        b[6],
        "Schritt 5: Dreieck -> uebergang()")
    schritt_05 = fuege(basis38, stell_block(konst5), uebergang_fn, main5)

    # ---- Schritt 6: Maske + Gluehsaum --------------------------------------
    konst6, funcs6 = split_stell(b[7], "Block 7")
    main6 = ersetze(
        main5,
        "    vec3 col = mix(colA, colB, t);          // DER naive Crossfade\n",
        b[8],
        "Schritt 6: Maske statt globalem Mix")
    schritt_06 = fuege(basis38, stell_block(konst5 + konst6),
                       uebergang_fn, funcs6, main6)

    # ---- Schritt 7: Blendarten ---------------------------------------------
    konst7, funcs7 = split_stell(b[9], "Block 9")
    schritt_07 = fuege(basis38, stell_block(konst5 + konst6 + konst7),
                       uebergang_fn, funcs7, main6)

    # ---- Schritt 8: Masken-Early-Out ---------------------------------------
    anfang = main6.index("    // beide Welten, beide Kameras")
    ende_marke = "    col += saumGlut(uv, t);\n"
    ende = main6.index(ende_marke) + len(ende_marke)
    main8 = main6[:anfang] + b[10].rstrip("\n") + "\n" + main6[ende:]
    schritt_08 = fuege(basis38, stell_block(konst5 + konst6 + konst7),
                       uebergang_fn, funcs7, main8)

    # ---- Schritte 9/10 aus dem Gesamtlisting-Common (Morph-Fixpunkt) -------
    konst20, rest20 = split_stell(b[20], "Block 20 (Common)")
    raus = ("HALTEDAUER", "BLENDEDAUER", "COOLDOWN", "KICK")
    konst_morph = konst5 + [z for z in konst20
                            if not any(k in z for k in raus)]

    kopf_9 = ("// =========================================================="
              "==================\n"
              "// COMPOSITE: TRANSITIONS - Schritt 9: Parameter-Morph "
              "(Single-Pass-Stand)\n"
              "// =========================================================="
              "==================\n")
    kopf_10 = kopf_9.replace(
        "Schritt 9: Parameter-Morph (Single-Pass-Stand)",
        "Schritt 10: Kamera-Kontinuitaet (Sammelpunkt)")

    kamera_kopf = ("// ===================================================="
                   "========================\n"
                   "// EINE Kamera fuer beide Welten")
    rest9 = rest20[:rest20.index(kamera_kopf)].rstrip() + "\n"
    a_kam = funktion(b[1], "void a_kamera", "Block 1")
    b_kam = funktion(b[2], "void b_kamera", "Block 2")
    main9 = ("void mainImage(out vec4 fragColor, in vec2 fragCoord)\n"
             "{\n"
             "    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;"
             "\n\n"
             + b[12].rstrip("\n") + "\n\n"
             "    fragColor = vec4(col, 1.0);\n"
             "}\n")
    schritt_09 = fuege(kopf_9, stell_block(konst_morph), rest9,
                       a_kam, b_kam, uebergang_fn, funcs7, main9)
    schritt_10 = fuege(kopf_10, stell_block(konst_morph), rest20,
                       uebergang_fn, funcs7, b[15])

    # ---- Schritt 11: Zustandsmaschine (Gesamtlisting minus Politur) --------
    common11 = ersetze(b[20],
                       "// Morph und Zustandsmaschine. Endstand des Tutorials "
                       "(Schritt 12).",
                       "// Morph und Zustandsmaschine. Stand Schritt 11 "
                       "(ohne die Politur aus 12).",
                       "Schritt 11: Common-Kopf")
    common11 = ersetze(common11,
                       "const float KICK        = 0.35;  "
                       "// Belichtungs-Kick im Wechselmoment\n",
                       "", "Schritt 11: KICK raus")
    buffer11 = b[17]
    image11 = ersetze(b[22],
                      "    // (3) Wechsel-Akzent: Kick und Dunst-Puls am "
                      "selben Faden\n"
                      "    float puls = 4.0 * t * (1.0 - t);\n"
                      "    belichtung *= 1.0 + KICK * puls;\n"
                      "    gDunst     *= 1.0 - 0.45 * puls;\n\n",
                      "", "Schritt 11: Akzent raus")
    image11 = ersetze(image11,
                      "    // (7) EIN Abschluss fuer beides: Drift, "
                      "Tonemapping, Gamma, Vignette\n"
                      "    col *= 0.92 + 0.08 * cos(iTime * 0.04 "
                      "+ vec3(0.0, 2.1, 4.2));\n",
                      "    // (7) EIN Abschluss fuer beides: Tonemapping, "
                      "Gamma, Vignette\n",
                      "Schritt 11: Drift raus")

    # ---- Schritt 12: das Gesamtlisting (Fixpunkt, woertlich) ---------------
    common12, buffer12, image12 = b[20], b[21], b[22]

    # ---- Anhang A1: Pruefstand (Beat-Trigger aus dem App-Uniform) ----------
    buffer_a1 = ersetze(b[23], BEAT_ANPASSUNG_ALT, BEAT_ANPASSUNG_NEU,
                        "A1: Beat-Anpassung")
    image_a1 = b[24]

    # ---- Anhang A3: Einbau-Diffs auf dem Gesamtlisting ---------------------
    common_a3 = ersetze(b[20],
                        "const float HALTEDAUER  = 9.0;   "
                        "// s Verweilzeit je Welt (Timer-Trigger)\n",
                        "const float HALTEDAUER  = 24.0;  "
                        "// GEAENDERT: Timer ist jetzt Fallback bei Stille\n"
                        "const float MIN_HALTE   = 3.0;   "
                        "// NEU: Mindest-Verweilzeit (Hysterese)\n",
                        "A3: HALTEDAUER/MIN_HALTE")
    common_a3 = ersetze(common_a3,
                        "vec3  b_sonne     = vec3(0.0, 0.3, 1.0);       "
                        "// setzt kamera() je Frame\n",
                        "vec3  b_sonne     = vec3(0.0, 0.3, 1.0);       "
                        "// setzt kamera() je Frame\n"
                        "float gLautheit   = 0.0;                       "
                        "// traege Lautheit (setzt das Image aus Buffer A)\n",
                        "A3: gLautheit")

    buffer_a3 = buffer_a1
    for zeile in ("const float HALTEDAUER  = 24.0;  "
                  "// Timer ist nur noch FALLBACK (greift bei Stille)\n",
                  "const float BLENDEDAUER = 4.0;\n",
                  "const float COOLDOWN    = 2.0;\n",
                  "const float MIN_HALTE   = 3.0;   "
                  "// Mindest-Verweilzeit: Hysterese gegen Geflacker\n"):
        buffer_a3 = ersetze(buffer_a3, zeile, "", "A3: lokale Konstante raus")
    buffer_a3 = ersetze(buffer_a3,
                        "        phase += dt / BLENDEDAUER;\n",
                        "        // Mapping [3]: das Blendtempo atmet mit dem "
                        "(geglaetteten!) Bass\n"
                        "        phase += dt / BLENDEDAUER "
                        "* (0.6 + 1.2 * min(glatt * 2.5, 1.5));\n",
                        "A3: Mapping 3")

    image_a3 = ersetze(b[22],
                       "    float t = mix(kurve, 1.0 - kurve, z.y);          "
                       "// 0 = Welt A .. 1 = Welt B\n",
                       "    float t = mix(kurve, 1.0 - kurve, z.y);          "
                       "// 0 = Welt A .. 1 = Welt B\n"
                       "    vec4 audio = texelFetch(iChannel0, ivec2(1, 0), 0);"
                       "\n"
                       "    gLautheit = audio.w;                             "
                       "// [4] Saum-Breite\n",
                       "A3: Audio-Read")
    image_a3 = ersetze(image_a3,
                       "    belichtung *= 1.0 + KICK * puls;\n",
                       "    belichtung *= 1.0 + KICK * puls + 0.10 * audio.y;"
                       "        // [5] Envelope-Zucken\n",
                       "A3: Mapping 5")
    image_a3 = ersetze(image_a3,
                       "    if (BLENDART == 0) return t;\n"
                       "    float s = mix(-2.0 * SAUM, 1.0 + 2.0 * SAUM, t);\n"
                       "    return 1.0 - smoothstep(s - SAUM, s + SAUM, "
                       "front(uv));\n",
                       "    if (BLENDART == 0) return t;\n"
                       "    float saum = SAUM * (0.7 + 2.0 * gLautheit);     "
                       "        // [4] Lautheit\n"
                       "    float s = mix(-2.0 * saum, 1.0 + 2.0 * saum, t);\n"
                       "    return 1.0 - smoothstep(s - saum, s + saum, "
                       "front(uv));\n",
                       "A3: maske() Saum")
    image_a3 = ersetze(image_a3,
                       "    if (BLENDART == 0) return vec3(0.0);\n"
                       "    float s = mix(-2.0 * SAUM, 1.0 + 2.0 * SAUM, t);\n",
                       "    if (BLENDART == 0) return vec3(0.0);\n"
                       "    float saum = SAUM * (0.7 + 2.0 * gLautheit);     "
                       "        // [4] Lautheit\n"
                       "    float s = mix(-2.0 * saum, 1.0 + 2.0 * saum, t);\n",
                       "A3: saumGlut() Saum")

    # ---- Ausgabe -----------------------------------------------------------
    KEINE = [NICHTS] * 4
    SELBST = [0, NICHTS, NICHTS, NICHTS]     # Buffer A liest sich selbst
    LIEST_A = [0, NICHTS, NICHTS, NICHTS]    # Image liest Buffer A
    SELBST_AUDIO = [0, AUDIO, NICHTS, NICHTS]  # + Audio auf iChannel1

    single: list[tuple[str, str, str]] = [
        ("schritt_01", "Schritt 1 - Welt A als Skelett (Kristall-Terrain)",
         b[1]),
        ("schritt_02", "Schritt 2 - Welt B als Skelett (Juggernaut, dark)",
         b[2]),
        ("schritt_03", "Schritt 3 - Beide in einer Datei: der harte Schnitt",
         schritt_03),
        ("schritt_04", "Schritt 4 - Der naive Crossfade (lehrreicher "
         "Fehlstart)", schritt_04),
        ("schritt_05", "Schritt 5 - Uebergangs-Kurven: Halten, Blenden, "
         "Halten", schritt_05),
        ("schritt_06", "Schritt 6 - Noise-Wipe mit Gluehsaum", schritt_06),
        ("schritt_07", "Schritt 7 - Blendarten: Radial- und Richtungs-Wipe",
         schritt_07),
        ("schritt_08", "Schritt 8 - Masken-Early-Out", schritt_08),
        ("schritt_09", "Schritt 9 - Parameter-Morph", schritt_09),
        ("schritt_10", "Schritt 10 - Kamera-Kontinuitaet (Sammelpunkt)",
         schritt_10),
    ]
    multi: list[tuple[str, str, str, str, list[int], list[int]]] = [
        ("schritt_11", "Schritt 11 - Zustandsmaschine in Buffer A "
         "(Timer-Modus)", common11, buffer11, LIEST_A, SELBST),
        ("schritt_12", "Schritt 12 - Kohaerenz-Politur: das fertige Werk",
         common12, buffer12, LIEST_A, SELBST),
        ("anhang_a3", "Anhang A3 - Das Werk hoert zu (Beat-Wechsel, "
         "Einbau-Diffs)", common_a3, buffer_a3, LIEST_A, SELBST_AUDIO),
    ]
    # Schritt 12: image12 unten je Schleife gewaehlt
    multi_images = {"schritt_11": image11, "schritt_12": image12,
                    "anhang_a3": image_a3}

    for name, beschr, code in single:
        (HIER / f"{name}.glsl").write_text(code, encoding="utf-8")
        doc = chain(name, beschr, code, KEINE)
        (HIER / f"{name}.lvfx").write_text(
            json.dumps(doc, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8")
        print(f"{name}.lvfx: Single-Pass, {len(code.splitlines())} Z.")

    for name, beschr, common, buf, img_in, buf_in in multi:
        img = multi_images[name]
        buf_v = mit_common(common, buf)
        img_v = mit_common(common, img)
        (HIER / f"{name}.bufferA.glsl").write_text(buf_v, encoding="utf-8")
        (HIER / f"{name}.image.glsl").write_text(img_v, encoding="utf-8")
        doc = chain(name, beschr, img_v, img_in, buf_v, buf_in)
        (HIER / f"{name}.lvfx").write_text(
            json.dumps(doc, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8")
        print(f"{name}.lvfx: Multipass A+Image, Image "
              f"{len(img_v.splitlines())} Z., Buffer "
              f"{len(buf_v.splitlines())} Z.")

    # Anhang A1: Pruefstand — Buffer mit Audio, Image nur Anzeige (kein Common)
    (HIER / "anhang_a1.bufferA.glsl").write_text(buffer_a1, encoding="utf-8")
    (HIER / "anhang_a1.image.glsl").write_text(image_a1, encoding="utf-8")
    doc = chain("anhang_a1", "Anhang A1 - Beat-Envelope-Pruefstand "
                "(Zustandsmaschine + Audio-Zustand)",
                image_a1, LIEST_A, buffer_a1, SELBST_AUDIO)
    (HIER / "anhang_a1.lvfx").write_text(
        json.dumps(doc, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8")
    print(f"anhang_a1.lvfx: Multipass A+Image, Image "
          f"{len(image_a1.splitlines())} Z., Buffer "
          f"{len(buffer_a1.splitlines())} Z.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

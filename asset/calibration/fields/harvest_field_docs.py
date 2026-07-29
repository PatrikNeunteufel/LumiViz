# -*- coding: utf-8 -*-
"""Reichert das Feld-Inventar (inventory.json) um Bedien- und Dokuwissen an.

Das Inventar selbst kommt aus dem C++-Gate (`test_FieldInventory.cpp`) und weiss,
WELCHE Felder es gibt und was ihre Vorgabe ist. Fuer Strang E fehlen zwei Dinge,
die nur im Quelltext stehen:

  1. **Wertebereich** — der Generator braucht einen Gegenwert, der wirkt UND
     zulaessig ist. Der Header nennt Bereiche nur 13-mal; die Panel-Zeilen
     (`addInt`/`addDouble`/`addRefDouble`) nennen lo/hi bei JEDER Zahl.
  2. **Beschreibung + schreibbare Skriptvariablen** — stehen als Doxygen an den
     Struct-Feldern in `EffectChain.hpp`. Fuer Strang F (§10) ist das die
     Tooltip-Quelle, fuer Strang E die Formel-Quelle: steht im Kommentar
     "Parameter-Skript (Strang D): `strength` + `b`/`w`/`h`", dann ist
     `strength = <gegenwert>` eine Formel, deren Wirkung man SIEHT.

Zuordnung Feld -> Panel-Zeile bewusst ueber den **Setter**, nicht ueber das
Label: `[](ChainNode& n, int v) { std::get<BlurParams>(n.params).strength = v; }`
nennt Struct und Feld eindeutig, waehrend das Label ("Level") frei uebersetzt ist.

Beide Ernten isolieren erst den Klammerbereich und suchen DANN darin — ein `.*?`
ueber Funktionsgrenzen hinweg hat in Session 53 Bloecke in fremde Funktionen
geschrieben.

Ausgabe:
  inventory_docs.json  je Feld: doc, lo/hi, panel-Art, Skriptvariablen
  Konsole              die zwei Luecken: Feld ohne Panel-Zeile (nicht bedienbar)
                       und Feld ohne Beschreibung (§10 kann es nicht erklaeren)

Aufruf:  python harvest_field_docs.py [--quiet]
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
HEADER = ROOT / "projects/apps/MyViz/include/visualizers/multieffect/EffectChain.hpp"
PANEL = ROOT / "projects/apps/MyViz/src/UI/panels/MultiEffectPanel.cpp"
SERIALIZER = ROOT / "projects/apps/MyViz/src/visualizers/ChainSerializer.cpp"
VISUALIZER = ROOT / "projects/apps/MyViz/src/visualizers/MultiEffectVisualizer.cpp"
# Die Skript-Traeger, an die Renderer ihre Slots abgeben. Ihre `number("…")`
# sind die Variablen, die der Effekt aus dem Skript herausliest — und nur die
# koennen wirken (s. harvest_script_vars, dritter Mechanismus).
MODUL_QUELLEN = {
    "ScriptGridModule": ROOT / "projects/apps/MyViz/src/visualizers/modules/ScriptGridModule.cpp",
    "ScriptLutModule": ROOT / "projects/apps/MyViz/src/visualizers/modules/ScriptLutModule.cpp",
    "SuperscopeModule": ROOT / "projects/apps/MyViz/src/visualizers/modules/SuperscopeModule.cpp",
}
INVENTORY = Path(__file__).parent / "inventory.json"
OUT = Path(__file__).parent / "inventory_docs.json"
# Die Tooltip-Tabelle des Panels (§10) — erzeugt, nicht von Hand gepflegt.
FIELDDOCS_CPP = ROOT / "projects/apps/MyViz/src/UI/panels/FieldDocs.cpp"

# Panel-Helfer, die einen Wertebereich tragen (Argument 3 und 4 sind lo/hi).
RANGED = {"addInt": "int", "addDouble": "double", "addRefDouble": "double"}
# Panel-Helfer ohne Bereich — belegen nur, DASS das Feld bedienbar ist.
PLAIN = {"addBool": "bool", "addColor": "farbe", "addEnum": "enum",
         "addColorTable": "farbtafel", "addScript": "skript", "addText": "text",
         "addGradient": "gradient", "addLine": "text", "addKernelGrid": "kernel",
         "addImageRow": "bild", "addGradientStops": "stuetzstellen"}

# Sammelwidgets (S53, Weg 3 in harvest_panel): Member-Funktionen, die ihre
# Felder als Referenz bekommen — im Aufruf steht `p->filename`, nicht
# `std::get<PictureParams>(…)`. Belegt an den Aufrufstellen in
# MultiEffectPanel.cpp (addGradientStops 2949, addKernelGrid 3016,
# addImageRow 3138/3151/3165/3184).
# Felder, die bewusst KEINE generische Panel-Zeile haben — geprueft S54, damit
# der Lueckenbericht sie nicht jedes Mal erneut als Befund vorlegt:
#   milkdrop.*     der Knoten hat die eigene Sektionsansicht (MultiEffectPanel
#                  zweigt vor der Feldliste ab: `milkSection`/`milkElem`)
#   passthrough.*  Platzhalter fuer nicht implementierte Effekte, read-only
#                  (MultiEffectPanel.cpp:4695)
#   importNotes.*  Import-Bericht, read-only (MultiEffectPanel.cpp:4683)
OHNE_PANEL_ERKLAERT = {
    "milkdrop.debugGrid", "milkdrop.meshX", "milkdrop.meshY", "milkdrop.preset",
    "milkdrop.presetDir", "passthrough.note", "passthrough.sourceId",
    "importNotes.text",
}

SAMMELWIDGETS = [
    ("ColorMapParams", ["stopPos", "stopColor"], "stuetzstellen"),
    ("ConvolutionParams", ["kernel"], "kernel"),
    ("PictureParams", ["filename", "imageData"], "bild"),
    ("PictureIIParams", ["filename", "imageData"], "bild"),
    ("TexerParams", ["filename", "imageData"], "bild"),
    ("TexerIIParams", ["filename", "imageData"], "bild"),
]


def balanced(src: str, open_pos: int) -> str:
    """Inhalt der Klammer, die bei `open_pos` ('(') beginnt — mit Zaehlung.

    Zeichen- und Zeichenketten-Literale werden uebersprungen, sonst kippt ein
    `'('` im Text die Bilanz.
    """
    depth, i, n = 0, open_pos, len(src)
    while i < n:
        c = src[i]
        if c in "\"'":
            quote, i = c, i + 1
            while i < n and src[i] != quote:
                i += 2 if src[i] == "\\" else 1
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return src[open_pos + 1:i]
        i += 1
    return ""


def split_args(body: str) -> list[str]:
    """Argumente auf oberster Ebene trennen (Klammern/Klammern/Literale achten)."""
    out, depth, cur, i, n = [], 0, [], 0, len(body)
    while i < n:
        c = body[i]
        if c in "\"'":
            quote, cur = c, cur + [c]
            i += 1
            while i < n and body[i] != quote:
                if body[i] == "\\":
                    cur.append(body[i]); i += 1
                cur.append(body[i]); i += 1
            cur.append(quote)
        elif c in "([{":
            depth += 1; cur.append(c)
        elif c in ")]}":
            depth -= 1; cur.append(c)
        elif c == "," and depth == 0:
            out.append("".join(cur).strip()); cur = []
        else:
            cur.append(c)
        i += 1
    if cur:
        out.append("".join(cur).strip())
    return out


def harvest_panel() -> dict[tuple[str, str], dict]:
    """(StructName, feld) -> {art, lo, hi} aus den Panel-Zeilen.

    Drei Wege, absichtlich in dieser Reihenfolge:
      1. `addXxx(...)`-Lambda mit `std::get<X>(n.params).feld` im Setter — nur
         hier gibt es auch lo/hi.
      2. irgendein anderes `std::get<X>(n.params).feld` im Panel — so werden
         Bitfelder bedient (`mirror.mode` verteilt sich auf vier Kaestchen,
         also gibt es keinen Ein-Feld-Setter).
      3. die Sammelwidgets (`addImageRow`, `addKernelGrid`, `addGradientStops`)
         sind Member-Funktionen und bekommen ihre Felder als REFERENZ
         (`addImageRow(form, path, p->filename, p->imageData, …)`) — dort steht
         kein `std::get`, das Feld ist trotzdem bedienbar. Diese Zuordnung ist
         die einzige von Hand gepflegte Stelle; sie ist unten begruendet.
    """
    src = PANEL.read_text(encoding="utf-8")
    found: dict[tuple[str, str], dict] = {}
    helpers = {**RANGED, **PLAIN}
    for m in re.finditer(r"\b(add[A-Za-z]+)\s*\(", src):
        name = m.group(1)
        if name not in helpers:
            continue
        body = balanced(src, m.end() - 1)
        if not body:
            continue
        # Das Ziel steht im Setter: std::get<XParams>(n.params).feld
        tgt = re.search(r"std::get<\s*(\w+Params)\s*>\s*\(\s*\w+\.params\s*\)\s*\.\s*(\w+)", body)
        if not tgt:
            continue
        key = (tgt.group(1), tgt.group(2))
        entry = {"panel": helpers[name], "helfer": name}
        if name == "addEnum":
            # Die Auswahl-Liste IST der Wertebereich: ohne sie waehlt der
            # Generator "Vorgabe mal drei" und landet ausserhalb, wo der
            # Renderer auf die Vorgabe zurueckklemmt — die Sonde misst dann
            # zwei gleiche Bilder (Befund timescope.channel, S54).
            liste = re.search(r"\{([^{}]*)\}", body)
            if liste:
                entry["enumWerte"] = len(re.findall(r'"[^"]*"', liste.group(1)))
            # Manche Zeilen zeigen einen VERSATZ: `addEnum(…, p->strength - 1,
            # {"Light","Normal","Heavy"}, … strength = v + 1)`. Der Auswahl-Index
            # ist dann nicht der Feldwert. Ohne diesen Versatz erzeugt der
            # Generator `strength = 2`, wo er „Heavy" (3) meint (Befund S54).
            versatz = re.search(r"->\s*" + re.escape(tgt.group(2)) + r"\s*-\s*(\d+)", body)
            if versatz:
                entry["enumVersatz"] = int(versatz.group(1))
        if name in RANGED:
            # Seit S56 ist Argument 0 der FELDNAME (§10), lo/hi stehen also an
            # Position 3 und 4 statt 2 und 3. Bis das nachgezogen war, fand der
            # Ernter GAR KEINEN Bereich mehr — jede Zahl fiel auf die Regel
            # "0 -> 1, sonst das Dreifache", und die Gegenwerte wurden still
            # schlechter (aufgefallen an `text.normSpeed`).
            args = split_args(body)
            if len(args) >= 5:
                lo, hi = args[3], args[4]
                if re.fullmatch(r"-?[\d.]+f?", lo) and re.fullmatch(r"-?[\d.]+f?", hi):
                    entry["lo"] = float(lo.rstrip("f"))
                    entry["hi"] = float(hi.rstrip("f"))
        # Erste Zeile gewinnt: ein Feld kann in mehreren Zweigen auftauchen,
        # der erste Treffer ist der des eigenen Knotentyps.
        found.setdefault(key, entry)

    # (2) alles Uebrige, das irgendwo im Panel geschrieben wird
    for sm in re.finditer(r"std::get<\s*(\w+Params)\s*>\s*\(\s*\w+\.params\s*\)\s*\.\s*(\w+)", src):
        found.setdefault((sm.group(1), sm.group(2)), {"panel": "sonderzeile"})

    # (3) Sammelwidgets, die ihr Feld als Referenz bekommen
    for struct, fields, widget in SAMMELWIDGETS:
        for f in fields:
            found.setdefault((struct, f), {"panel": widget})
    return found


def cpp_literal(text: str) -> str:
    """Ein C++-Zeichenkettenliteral, das auf JEDEM Compiler dasselbe bedeutet.

    Nicht-ASCII wird als OKTAL-Escape der UTF-8-Bytes geschrieben, nicht als
    `\\x`: ein Hex-Escape in C++ frisst beliebig viele Ziffern weiter, `"\\xE4z"`
    waere also ein einziges (viel zu grosses) Zeichen. Oktal endet nach drei
    Ziffern. So braucht die Datei weder /utf-8 noch eine BOM.
    """
    out = []
    for b in text.encode("utf-8"):
        c = chr(b)
        if c == '"':
            out.append('\\"')
        elif c == "\\":
            out.append("\\\\")
        elif b < 0x20 or b >= 0x7F:
            out.append(f"\\{b:03o}")
        else:
            out.append(c)
    return '"' + "".join(out) + '"'


def schreibe_fielddocs(typen_out: list[dict]) -> int:
    """Die Tooltip-Tabelle als C++ erzeugen (Knoten-Parameter-Konzept §10).

    Erzeugt, nicht von Hand gepflegt: die Quelle ist der Doxygen-Kommentar am
    Struct-Feld. Waere der Text im Panel abgeschrieben, wuerden Kommentar und
    Tooltip auseinanderdriften — genau das soll die Tabelle verhindern.
    """
    eintraege = sorted((f"{t['typkey']}.{f['name']}", f["doc"])
                       for t in typen_out for f in t["felder"] if f.get("doc"))
    zeilen = "".join(f"    {{{cpp_literal(k)},\n     {cpp_literal(v)}}},\n"
                     for k, v in eintraege)
    FIELDDOCS_CPP.write_text(
        "// ERZEUGT von asset/calibration/fields/harvest_field_docs.py — nicht\n"
        "// von Hand aendern. Quelle sind die Doxygen-Kommentare an den Feldern\n"
        "// der `…Params`-Structs in EffectChain.hpp; wer einen Text aendern\n"
        "// will, aendert ihn dort und laesst den Ernter neu laufen.\n"
        "//\n"
        "// Knoten-Parameter-Konzept §10 — Tooltip an jedem Feld.\n"
        "\n"
        '#include "UI/panels/FieldDocs.hpp"\n'
        "\n"
        "#include <algorithm>\n"
        "#include <array>\n"
        "#include <string_view>\n"
        "#include <utility>\n"
        "\n"
        "namespace lumi::multieffect::fielddocs\n"
        "{\n"
        "namespace\n"
        "{\n"
        "// Nach Schluessel sortiert — die Suche unten setzt das voraus.\n"
        "constexpr std::array<std::pair<std::string_view, std::string_view>,\n"
        f"                     {len(eintraege)}>\n"
        "    kDocs{{\n"
        f"{zeilen}"
        "}};\n"
        "}  // namespace\n"
        "\n"
        "QString tooltip(const QString& typeKey, const QString& field)\n"
        "{\n"
        "    const std::string key = (typeKey + QLatin1Char('.') + field).toStdString();\n"
        "    const std::string_view needle{key};\n"
        "    const auto it = std::lower_bound(\n"
        "        kDocs.begin(), kDocs.end(), needle,\n"
        "        [](const auto& e, std::string_view n) { return e.first < n; });\n"
        "    if (it == kDocs.end() || it->first != needle) return {};\n"
        "    return QString::fromUtf8(it->second.data(),\n"
        "                             static_cast<int>(it->second.size()));\n"
        "}\n"
        "\n"
        "QStringList documentedKeys()\n"
        "{\n"
        "    QStringList out;\n"
        "    out.reserve(static_cast<int>(kDocs.size()));\n"
        "    for (const auto& [key, text] : kDocs)\n"
        "        out << QString::fromUtf8(key.data(), static_cast<int>(key.size()));\n"
        "    return out;\n"
        "}\n"
        "\n"
        "}  // namespace lumi::multieffect::fielddocs\n",
        encoding="utf-8", newline="")
    return len(eintraege)


def pruefe_vorgabe_literale() -> list[str]:
    """Vorgabe-Literale im Deserialisierer — die abgeschaffte zweite Quelle.

    Bis S56 stand jede Vorgabe doppelt: als Initialisierer im `…Params`-Struct
    und als dritter Parameter im Leser (`getInt(o, "x", 5)`). Liefen sie
    auseinander, hing der Wert davon ab, WOHER der Knoten kam — ein frisch
    eingefuegter trug den Struct-Wert, ein geladener den des Lesers. Genau daran
    hing der Kleinian-Befund: die Struct-Vorgabe war korrigiert und gebaut, die
    Sonden sahen es trotzdem nicht, weil sie aus einem Preset laden.

    Seit dem Umbau bezieht der Leser die Vorgabe aus dem Ziel selbst
    (`getInt(o, "x", p.x)`). Diese Pruefung haelt das fest: ein Literal an
    dieser Stelle ist eine neue zweite Quelle.

    ERLAUBT bleibt, wo der Wert bewusst fuer ALTE DATEIEN steht und nicht die
    Vorgabe des Knotens ist — dann traegt die Zeile ihre Begruendung im Code.
    """
    erlaubt = {("StarfieldParams", "blend")}  # legacy files rendered additively
    src = SERIALIZER.read_text(encoding="utf-8")
    grenzen = [(m.group(1), m.start())
               for m in re.finditer(r"\b(\w+Params)\s+p;", src)]
    grenzen.append((None, len(src)))
    # Der Vorgabewert wird NACH dem Treffer geprueft, nicht per Lookahead: ein
    #  kann null Leerzeichen nehmen, und dann steht das Leerzeichen noch vor
    # dem  — der Lookahead griff daneben und meldete jede umgestellte Zeile.
    muster = re.compile(r'p(?:\.\w+)+\s*=\s*(?:static_cast<[^>]+>\s*\(\s*)?'
                        r'get(?:Int|Double|Bool|Color|Str)\s*\(\s*o\s*,\s*'
                        r'"(\w+)"\s*,\s*([^),]+)\)')
    out: list[str] = []
    for (struct, a), (_, b) in zip(grenzen, grenzen[1:]):
        for m in muster.finditer(src[a:b]):
            wert = m.group(2).strip()
            # Bezieht die Vorgabe aus dem Struct — richtig so. Der Cast davor
            # ist beliebig (`static_cast<double>(p.spinStep)`).
            if re.match(r"^(?:static_cast<[^>]+>\s*\(\s*)?p\.", wert):
                continue
            if (struct, m.group(1)) in erlaubt:
                continue
            zeile = src[:a + m.start()].count("\n") + 1
            out.append(f"{struct}.{m.group(1)} = {m.group(2).strip()} "
                       f"(ChainSerializer.cpp:{zeile})")
    return out


def pruefe_panel_schluessel(felder_je_struct: dict[str, set[str]]) -> list[str]:
    """Panel-Zeilen, deren Feldschluessel auf KEIN Feld zeigt (§10).

    Jede `add*`-Zeile nennt seit S56 ihren Feldnamen als erstes Argument; er ist
    der Schluessel in die Tooltip-Tabelle. Ein falscher Schluessel faellt sonst
    NIRGENDS auf — das Panel zeigt einfach keinen Hinweis, und der Wachhund in
    C++ prueft nur die Tabelle gegen `fieldNames()`, nicht die Zeilen.

    Gesehen ist das an den fuenf Feldern, deren JSON-Name vom Feldnamen
    abweicht (`o["ftype"] = p.type`, `o["overrideBlend"] = p.enabled`): der
    maschinell eingesetzte Schluessel war der Struct-Name und traf ins Leere.

    Geprueft wird nur, was sich einem Struct zuordnen laesst — die
    Milkdrop-Preset- und Sprite-Zeilen schreiben in Unterstrukturen und haben
    im Feld-Inventar zu Recht keinen Eintrag.
    """
    src = PANEL.read_text(encoding="utf-8")
    helfer = "|".join(("addInt", "addDouble", "addRefDouble", "addBool", "addColor",
                       "addEnum", "addColorTable", "addScript", "addGradient",
                       "addText"))
    schlecht: list[str] = []
    for m in re.finditer(r"\b(?:" + helfer + r")\s*\(\s*\"(\w+)\"", src):
        body = balanced(src, src.index("(", m.start()))
        tgt = re.search(r"std::get<\s*(\w+Params)\s*>\s*\(\s*\w+\.params\s*\)", body)
        if not tgt:
            continue
        struct, key = tgt.group(1), m.group(1)
        bekannt = felder_je_struct.get(struct)
        if bekannt and key not in bekannt:
            zeile = src[:m.start()].count("\n") + 1
            schlecht.append(f"{struct}.{key} (MultiEffectPanel.cpp:{zeile})")
    return schlecht


def join_declarations(body: str) -> list[str]:
    """Zeilen eines Struct-Koerpers, mehrzeilige Deklarationen zusammengefasst.

    Die Feldsuche unten arbeitet ZEILENWEISE. Eine Deklaration, die sich ueber
    mehrere Zeilen zieht — `std::array<int, 49> kernel = {0, 0, …};` steht in
    vier — wurde damit ueberhaupt nicht als Feld erkannt: kein Bereich, keine
    Beschreibung, und der Lueckenbericht meldete sie als „ohne Kommentar",
    obwohl direkt daneben einer stand (Befund S56).

    Zusammengefasst wird bis zum `;` auf oberster Ebene; Kommentarzeilen bleiben
    fuer sich, damit `pending` weiter funktioniert.
    """
    out: list[str] = []
    puffer, tiefe = "", 0
    for line in body.splitlines():
        s = line.strip()
        if not puffer and (not s or s.startswith("//") or s.startswith("/*")
                           or s.startswith("*")):
            out.append(line)
            continue
        puffer = f"{puffer} {s}" if puffer else s
        tiefe += s.count("{") + s.count("(") - s.count("}") - s.count(")")
        if tiefe <= 0 and ";" in s:
            out.append(puffer)
            puffer, tiefe = "", 0
    if puffer:
        out.append(puffer)
    return out


def harvest_header() -> dict[tuple[str, str], dict]:
    """(StructName, feld) -> {doc, skriptvars} aus den Doxygen-Kommentaren."""
    src = HEADER.read_text(encoding="utf-8")
    out: dict[tuple[str, str], dict] = {}

    for sm in re.finditer(r"\bstruct\s+(\w+Params)\s*\{", src):
        struct = sm.group(1)
        i, depth = sm.end(), 1
        while depth and i < len(src):
            if src[i] == "{":
                depth += 1
            elif src[i] == "}":
                depth -= 1
            i += 1
        body = src[sm.end():i - 1]           # <- erst isolieren, dann suchen

        pending: list[str] = []              # `///`-Zeilen VOR dem Feld
        im_block = False                     # in einem /** … */ ueber dem Feld
        for line in join_declarations(body):
            s = line.strip()
            if s.startswith("///<"):
                continue                     # gehoert zur Feldzeile, unten geholt
            if s.startswith("///"):
                pending.append(s.lstrip("/").strip())
                continue
            # Ein `/** … */`-Block ueber dem Feld zaehlt genauso — er stand hier
            # bis S56 im selben Topf wie jeder andere Kommentar und wurde
            # verworfen, obwohl er oft die AUSFUEHRLICHSTE Beschreibung traegt
            # (`timescope.useChannel` erklaert dort auf zehn Zeilen, warum der
            # Regler im Original tot ist). Uebernommen wird der erste Absatz:
            # der Rest ist Begruendung fuer den Leser des Codes, kein Tooltip.
            if s.startswith("/**"):
                im_block, pending = True, []
                s = s[3:].strip()
                if s and not s.startswith("*/"):
                    pending.append(s)
                continue
            if im_block:
                if s.startswith("*/"):
                    im_block = False
                    continue
                rein = s.lstrip("*").strip()
                if rein.startswith("@"):     # @brief/@param … gehoert nicht ins Panel
                    continue
                if not rein:                 # Absatzende — der erste reicht
                    if pending:
                        im_block = False
                    continue
                pending.append(rein)
                continue
            if s.startswith("//") or s.startswith("/*") or s.startswith("*"):
                continue
            # Der Vorbelegungsteil darf ALLES enthalten, auch ein `;` — das steht
            # in `std::string pointCode = "x=(i*2)-1; y=0;";` mitten in der
            # Zeichenkette und liess die Zeile bis S56 durchfallen. Und ein Feld
            # darf eine Feldlaenge tragen (`uint32_t colors[5]`), sonst fehlen
            # die Farbtafeln von Dot Plane, Dot Grid, Osc Ring/Star … Die
            # Kommentargruppe hinten ist entfallen; `trail` sucht ohnehin selbst.
            # Auch die Klammer-Vorbelegung ohne `=` gehoert dazu
            # (`std::vector<uint32_t> colors{0xFFFFFF};` — so stehen die
            # Farbtafeln von Dot Grid, Osc Ring/Star, Rotating Stars,
            # Simple Scope da).
            fm = re.match(r"^[\w:<>,\s\*&]+?\b(\w+)\s*(?:\[[^\]]*\])?"
                          r"\s*(?:\{[^;]*\})?\s*(?:=.*)?;", s)
            if not fm:
                if s:
                    pending = []
                continue
            field = fm.group(1)
            trail = re.search(r"///<\s*(.*)$", line)
            doc = trail.group(1).strip() if trail else " ".join(pending).strip()
            entry: dict = {}
            if doc:
                entry["doc"] = doc
            # Schreibbare Variablen des Strang-D-Kommentars: "…: `a`, `b` + `w`/`h`"
            vars_ = re.findall(r"`(\w+)`", doc)
            if vars_ and ("Parameter-Skript" in doc or "Skript" in doc):
                entry["skriptvars"] = [v for v in vars_ if v not in ("b", "w", "h")]
            if entry:
                out[(struct, field)] = entry
            # Ein `///`-Block ueber MEHREREN Feldern gilt fuer alle: der
            # Strang-D-Kommentar steht einmal ueber `initCode` und meint
            # `frameCode`/`beatCode` mit. Wuerde `pending` hier geleert, faenden
            # zwei Drittel der 141 Skriptfelder keine Beschreibung.
            if not (doc and field.endswith("Code") and pending):
                pending = []
    return out


def harvest_script_vars() -> dict[str, dict]:
    """Struct -> {"vars": [...], "tot": [...]} aus den `runParamScript`-Aufrufen.

    Die schreibbaren Variablen heissen NICHT wie die Felder: `fadeLen` ist im
    Skript `fadelen`, `faderR` ist `faderr` — AVS-Kleinschreibung. Der
    Doxygen-Kommentar nennt mal die eine, mal die andere Schreibweise; die
    einzige verbindliche Quelle ist die ParamVar-Liste im Aufruf selbst.

    "tot" sind die Variablen, deren Frame-Kopie nach dem Aufruf NIE gelesen
    wird — das Skript darf sie beschreiben, es aendert nur nichts mehr.
    """
    src = VISUALIZER.read_text(encoding="utf-8")
    out: dict[str, dict] = {}
    for m in re.finditer(r"\bvoid\s+MultiEffectVisualizer::\w+\s*\(([^)]*)\)\s*\{", src, re.S):
        sig = m.group(1)
        sm = re.search(r"const\s+(\w+Params)\s*&", sig)
        if not sm:
            continue
        i, depth = m.end(), 1
        while depth and i < len(src):
            if src[i] == "{":
                depth += 1
            elif src[i] == "}":
                depth -= 1
            i += 1
        body = src[m.end():i - 1]  # <- erst isolieren, dann suchen

        call = re.search(r"runParamScript\s*\(", body)
        if not call:
            continue
        j, depth = call.end() - 1, 0
        while j < len(body):
            if body[j] == "(":
                depth += 1
            elif body[j] == ")":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        args, rest = body[call.end():j], body[j:]
        paare = re.findall(r'\{\s*"(\w+)"\s*,\s*&(\w+)\s*\}', args)
        if not paare:
            continue
        out[sm.group(1)] = {
            "vars": [n for n, _ in paare],
            "tot": [n for n, v in paare if not re.search(r"\b" + re.escape(v) + r"\b", rest)],
        }

    # Zweiter Mechanismus: Knoten mit eigenem ScriptSlotHost (Fractal 2D/3D,
    # Flame, Domain Warp …) rufen kein `runParamScript`, sondern setzen ihre
    # Groessen direkt — `e.setNumber("cx", cx)` … `e.number("cx")`. Dieselbe
    # Semantik, andere Schreibweise; ohne sie blieben 104 Skriptfelder ohne
    # erzeugbare Sonde (S54).
    for m in re.finditer(r"\bvoid\s+MultiEffectVisualizer::\w+\s*\(([^)]*)\)\s*\{", src, re.S):
        sm = re.search(r"const\s+(\w+Params)\s*&", m.group(1))
        if not sm or sm.group(1) in out:
            continue
        i, depth = m.end(), 1
        while depth and i < len(src):
            if src[i] == "{":
                depth += 1
            elif src[i] == "}":
                depth -= 1
            i += 1
        body = src[m.end():i - 1]
        gesetzt = re.findall(r'\.setNumber\s*\(\s*"(\w+)"', body)
        # `b`/`w`/`h` und der Audio-Satz sind EINGABEN des Hosts, keine Ziele.
        eingaben = {"b", "w", "h", "bass", "mid", "treb", "vol", "beat", "time"}
        vars_ = [v for v in dict.fromkeys(gesetzt) if v not in eingaben]
        if not vars_:
            continue
        out[sm.group(1)] = {
            "vars": vars_,
            "tot": [v for v in vars_
                    if not re.search(r'\.number\s*\(\s*"' + re.escape(v) + r'"', body)],
        }

    # Dritter Mechanismus (S55): EFFEKT-Skripte. Superscope, Color Modifier,
    # Dynamic Movement & Co. rechnen nicht ihre Felder aus, sondern den Effekt
    # selbst — je Punkt, je Gitterpunkt, je Tabelleneintrag. Sie SETZEN im
    # Skript die Eingaben (i, v, n …) und LESEN danach das Ergebnis heraus.
    # Deshalb greifen die beiden Mechanismen oben nicht: sie gehen von
    # `setNumber` aus, hier steht die Wahrheit aber im Lesen.
    #
    # Die Regel ist dieselbe wie beim S54-Befund „43 tote Frame-Kopien":
    #   Eine Variable wirkt genau dann, wenn der Effekt sie nach dem Slot-Lauf
    #   AUSLIEST.
    # Also ist `number("…")` die Quelle — im Renderer selbst (eigener
    # ScriptSlotHost) und, wo er die Arbeit an ein Modul abgibt, in dessen
    # Quelle. Ohne das blieben die Slots der groessten Knoten ohne Sonde und
    # muessten von Hand in HANDWERK gepflegt werden (39 Felder, S55).
    modul_vars: dict[str, list[str]] = {}
    for modul, datei in MODUL_QUELLEN.items():
        if datei.exists():
            modul_vars[modul] = sorted(set(
                re.findall(r'number\s*\(\s*"(\w+)"', datei.read_text(encoding="utf-8"))))

    for m in re.finditer(r"\bvoid\s+MultiEffectVisualizer::\w+\s*\(([^)]*)\)\s*\{", src, re.S):
        sm = re.search(r"const\s+(\w+Params)\s*&", m.group(1))
        if not sm or sm.group(1) in out:
            continue
        i, depth = m.end(), 1
        while depth and i < len(src):
            if src[i] == "{":
                depth += 1
            elif src[i] == "}":
                depth -= 1
            i += 1
        body = src[m.end():i - 1]  # <- erst isolieren, dann suchen

        gelesen = set(re.findall(r'->number\s*\(\s*"(\w+)"', body))
        gelesen |= set(re.findall(r'\.number\s*\(\s*"(\w+)"', body))
        for modul, vars_ in modul_vars.items():
            if re.search(r"\b" + re.escape(modul) + r"\b", body):
                gelesen |= set(vars_)
        # Der Audio-Satz und die Bildmasse kommen VOM Host, ein Skript kann sie
        # nicht sinnvoll setzen.
        gelesen -= {"b", "w", "h", "bass", "mid", "treb", "vol", "beat", "time",
                    "sw", "sh", "i", "v"}
        if not gelesen:
            continue
        # Kein "tot": diese Menge IST die der gelesenen Variablen.
        out[sm.group(1)] = {"vars": sorted(gelesen), "tot": []}
    return out


def harvest_aliases() -> dict[tuple[str, str], str]:
    """(Struct, JSON-Name) -> Struct-Feldname, aus dem WriteVisitor.

    Der JSON-Name ist NICHT immer der Feldname: `o["ftype"] = p.type` (der
    Knotentyp belegt `type` bereits), `o["overrideBlend"] = p.enabled`. Ohne
    diese Tabelle meldet die Ernte solche Felder als unbedienbar, obwohl sie
    eine Panel-Zeile haben — fuenf Phantom-Befunde in der ersten Fassung.
    """
    src = SERIALIZER.read_text(encoding="utf-8")
    out: dict[tuple[str, str], str] = {}
    for m in re.finditer(r"\boperator\(\)\s*\(\s*const\s+(\w+Params)\s*&\s*(\w+)\s*\)", src):
        struct, var = m.group(1), m.group(2)
        brace = src.find("{", m.end())
        if brace < 0:
            continue
        i, depth = brace + 1, 1
        while depth and i < len(src):
            if src[i] == "{":
                depth += 1
            elif src[i] == "}":
                depth -= 1
            i += 1
        body = src[brace + 1:i - 1]  # <- erst isolieren, dann suchen
        for am in re.finditer(r'o\s*\[\s*"(\w+)"\s*\]\s*=\s*' + re.escape(var) + r"\.(\w+)", body):
            if am.group(1) != am.group(2):
                out[(struct, am.group(1))] = am.group(2)
    return out


def struct_by_typekey() -> dict[str, str]:
    """typkey -> Struct-Name, geerntet aus der `effectTypeKey`-Tabelle.

    Nicht aus dem Anzeigenamen geraten: "AVI" heisst `AviParams`, "Camera 3D"
    heisst `Camera3DParams`, "Global Variables" heisst `JherikoGlobalParams` —
    15 der 85 Typen brechen jede Namensregel. Die Visitor-Tabelle im
    ChainSerializer ist die einzige Stelle, die beides sicher verbindet.
    """
    src = SERIALIZER.read_text(encoding="utf-8")
    m = re.search(r"QString\s+effectTypeKey\s*\([^)]*\)\s*\{", src)
    if not m:
        return {}
    i, depth = m.end(), 1
    while depth and i < len(src):
        if src[i] == "{":
            depth += 1
        elif src[i] == "}":
            depth -= 1
        i += 1
    body = src[m.end():i - 1]  # <- erst isolieren, dann suchen
    return {key: struct for struct, key in
            re.findall(r"operator\(\)\s*\(\s*const\s+(\w+Params)\s*&[^)]*\)\s*const\s*"
                       r"\{\s*return\s+\"([^\"]+)\"", body)}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--quiet", action="store_true", help="nur die Bilanz ausgeben")
    args = ap.parse_args()

    if not INVENTORY.exists():
        print("FEHLER: inventory.json fehlt — erst MyViz.UnitTests mit "
              "LUMIVIZ_UPDATE_FIELD_INVENTORY=1 laufen lassen")
        return 2

    inv = json.loads(INVENTORY.read_text(encoding="utf-8"))
    panel = harvest_panel()
    header = harvest_header()
    structs = struct_by_typekey()
    aliases = harvest_aliases()
    skript = harvest_script_vars()

    ohne_panel: list[str] = []
    ohne_doc: list[str] = []
    ohne_struct: list[str] = []
    typen_out = []

    for t in inv["typen"]:
        struct = structs.get(t["typkey"], "")
        if not struct:
            ohne_struct.append(t["typkey"])
        felder = []
        for f in t["felder"]:
            # JSON-Name zuerst, dann der Struct-Feldname hinter dem Alias.
            key = (struct, f["name"])
            alias = aliases.get(key)
            keys = [key] + ([(struct, alias)] if alias else [])
            e = dict(f)
            if alias:
                e["feld"] = alias
            for k in keys:
                e.update(panel.get(k, {}))
                e.update(header.get(k, {}))
            # Skript-Slots bekommen die ECHTEN Variablennamen (Kleinschreibung
            # im AVS-Stil) — der Doxygen-Name traegt sie nur ungefaehr.
            # S55: auch die Punkt-/Kurven-Slots. Sie sind derselbe Fall — nur
            # laufen sie je Punkt statt je Frame, und ihre Variablen stehen im
            # dritten Erntemechanismus (was der Effekt herausliest).
            if (f["name"] in ("initCode", "frameCode", "beatCode", "pointCode",
                              "levelCode", "pixelCode")
                    and struct in skript):
                e["skriptvars"] = skript[struct]["vars"]
                if skript[struct]["tot"]:
                    e["skriptvars_wirkungslos"] = skript[struct]["tot"]
            voll = f"{t['typkey']}.{f['name']}"
            if "panel" not in e and voll not in OHNE_PANEL_ERKLAERT:
                ohne_panel.append(voll)
            if "doc" not in e:
                ohne_doc.append(f"{t['typkey']}.{f['name']}")
            felder.append(e)
        typen_out.append({"typkey": t["typkey"], "name": t["name"],
                          "struct": struct, "felder": felder})

    doc = {"schema": 1,
           "quelle": {"inventar": "inventory.json (C++-Gate)",
                      "bereiche": "MultiEffectPanel.cpp (Setter-Zuordnung)",
                      "beschreibung": "EffectChain.hpp (Doxygen)"},
           "summe": {"typen": len(typen_out),
                     "felder": sum(len(t["felder"]) for t in typen_out),
                     "ohne_panel": len(ohne_panel),
                     "ohne_doc": len(ohne_doc)},
           "typen": typen_out}
    OUT.write_text(json.dumps(doc, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    n_cpp = schreibe_fielddocs(typen_out)
    felder_je_struct = {t["struct"]: {f["name"] for f in t["felder"]}
                        for t in typen_out if t["struct"]}
    falsche_schluessel = pruefe_panel_schluessel(felder_je_struct)
    literale = pruefe_vorgabe_literale()

    print(f"Inventar angereichert: {doc['summe']['typen']} Typen, "
          f"{doc['summe']['felder']} Felder -> {OUT.name}")
    print(f"  Tooltip-Tabelle: {n_cpp} Eintraege -> {FIELDDOCS_CPP.name}")
    print(f"  ohne Panel-Zeile (nicht bedienbar): {len(ohne_panel)}")
    print(f"  ohne Beschreibung (§10-Luecke):     {len(ohne_doc)}")
    print(f"  Panel-Schluessel ohne Feld:         {len(falsche_schluessel)}")
    print(f"  Vorgabe-Literale im Leser:          {len(literale)}")
    for x in literale:
        print(f"    ZWEITE QUELLE: {x}")
    for x in falsche_schluessel:
        print(f"    FALSCH: {x}")
    if ohne_struct:
        print(f"  Typ ohne Struct-Zuordnung ({len(ohne_struct)}): "
              f"{', '.join(ohne_struct[:8])}")
    if not args.quiet:
        if ohne_panel:
            print("\n-- ohne Panel-Zeile --")
            for x in ohne_panel:
                print("   ", x)
        if ohne_doc:
            print("\n-- ohne Beschreibung --")
            for x in ohne_doc:
                print("   ", x)
    return 0


if __name__ == "__main__":
    sys.exit(main())

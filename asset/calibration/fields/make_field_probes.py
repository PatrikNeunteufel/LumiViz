# -*- coding: utf-8 -*-
"""Erzeugt die Feld-Sonden (Strang E, Konzept §9): je Modul und je Feld ein Preset.

Eingabe ist `inventory_docs.json` (harvest_field_docs.py) — also die maschinell
gewonnene Liste ALLER Felder mit Vorgabe, Wertebereich und, bei Skriptfeldern,
den schreibbaren Variablen.

Je Knotentyp entsteht:
  <typkey>/_default.lvfx   Untergrund + Pruefling, alle Felder auf Vorgabe
  <typkey>/<feld>.lvfx     dasselbe, GENAU ein Feld abweichend

Das Urteil faellt `run_field_probes.py`: unterscheiden sich die zwei Bilder
nicht, kann das Feld nicht wirken. Weil beide Laeufe denselben Untergrund und
dasselbe synthetische Audio sehen, kuerzt sich alles heraus ausser dem Feld.

--------------------------------------------------------------- Der Gegenwert
Ein Preset ist nur so gut wie sein Gegenwert. Die Regeln, in dieser Reihenfolge:

  bool          umgedreht
  Zahl mit Bereich   der zulaessige Wert mit dem GROESSTEN Abstand zur Vorgabe
                     (lo oder hi) — kleine Schritte verschwinden im Rauschen
  Zahl ohne Bereich  0 -> 1, sonst das Dreifache
  Farbe         0xFF00FF (gegen jede uebliche Vorgabe deutlich)
  Farbtafel     zwei kraeftige Farben
  Skriptfeld    `<variable> = <deren Gegenwert>` — die Variablen stehen im
                Doxygen des Structs, ihr Bereich im Panel. Damit ist die Formel
                nicht geraten, sondern aus demselben Wissen abgeleitet wie der
                Gegenwert des Feldes selbst.
  Text/Sonstiges  nur ueber HANDWERK (unten) — ein Bildpfad oder ein
                  Movement-Ausdruck laesst sich nicht ableiten.

Wo eine Regel danebengreift, meldet der Runner "kein Unterschied". Das ist dann
KEIN Befund am Feld, sondern einer am Gegenwert — er gehoert nach HANDWERK.
Diese Unterscheidung ist der Grund, warum der Report zwei Urteile kennt.

Aufruf:  python make_field_probes.py [typkey ...] [--out DIR]
"""
from __future__ import annotations

import argparse
import base64
import json
import sys
from pathlib import Path

import lvfx_lib as L

# Das Test-Asset der Bild-Knoten: 16x16 mit vier Quadranten (rot/gruen/blau/weiss).
# Bewusst winzig und eingebettet — `imageData` traegt die Bilddatei base64, eine
# Kette steht damit fuer sich (Entscheid S53) und die Sonde braucht keinen
# Suchpfad. Die vier Quadranten machen Spiegelungen und Kanalfehler sichtbar,
# genau wie die Farbfelder des Untergrunds.
TESTBILD_B64 = base64.b64encode((Path(__file__).parent / "testbild.png")
                                .read_bytes()).decode("ascii")

# Das Test-Video (make_testvideo.py). Der AVI-Knoten laedt ueber Video for
# Windows von der Platte, nicht eingebettet — also ein absoluter Pfad.
# 32 Bit ist Pflicht: `runAvi` prueft `biBitCount == 32` und ueberspringt
# alles andere ohne Meldung (Befund S54).
TESTVIDEO = (Path(__file__).parent / "testvideo.avi").resolve().as_posix()

HIER = Path(__file__).parent
DOCS = HIER / "inventory_docs.json"
OUT = HIER / "probes"

# Felder, deren Gegenwert sich NICHT ableiten laesst — hier von Hand, mit
# Begruendung. Leerer Wert = bewusst uebersprungen (nicht sinnvoll pruefbar).
HANDWERK: dict[str, object] = {
    # Movement: der Ausdruck IST das Feld. `d=d*0.5` zieht das Bild zur Mitte —
    # auf dem Untergrund an den Farbfeldern sofort sichtbar.
    "movement.code": "d=d*0.5",
    # Dynamic Movement / Distance Modifier / Shift: dieselbe Logik.
    "dynamicMovement.pointCode": "d=d*0.5",
    # Dynamic Distance Modifier: das Feld heisst `pixelCode`, nicht `code` —
    # daran scheiterte die Sonde zuerst, nicht an der Formel. `d` ist eine
    # normierte Distanz (rein: Ziel, raus: Quelle), also ist `d=d` die
    # Identitaet und `d=d*0.6` eine Stauchung. Beides am Original belegt
    # (r_ddm.cpp:287-289) und gemessen: leer und `d=d` liefern dasselbe Bild,
    # `d=d*0.6` ein anderes.
    "dynamicDistanceModifier.pixelCode": "d=d*0.6",
    # Die Slot-Sonden dieser beiden Knoten schreiben in `reg00` — ihr
    # Punkt-Code liest es (s. GRUNDKONFIG). `reg00` ist preset-global und
    # ueberlebt den Slot-Wechsel, eine lokale Variable taete das nicht.
    **{f"dynamicDistanceModifier.{f}": "reg00=1.4"
       for f in ("initCode", "frameCode", "beatCode")},
    **{f"triangle.{f}": "reg00=1" for f in ("initCode", "frameCode", "beatCode")},
    # Global Variables setzt preset-weite Register; sichtbar wird das erst
    # an einem Knoten dahinter, der sie liest (s. NACHFOLGER).
    **{f"jherikoGlobal.{f}": "reg00=0.9"
       for f in ("initCode", "frameCode", "beatCode")},
    "dynamicShift.frameCode": "x=x+0.2; y=y+0.1",
    # Bilder: `imageData` traegt die Datei selbst, also ist das Testbild der
    # Gegenwert. `filename` ist dagegen nur die Herkunftsnotiz — es zeichnet
    # nichts und bekommt deshalb keine Sonde.
    "picture.imageData": TESTBILD_B64,
    "pictureII.imageData": TESTBILD_B64,
    "texer.imageData": TESTBILD_B64,
    "texerII.imageData": TESTBILD_B64,
    "picture.filename": "",
    "pictureII.filename": "",
    "texer.filename": "",
    "texerII.filename": "",
    # BITFELDER: der Regelwert (groesster Abstand im Bereich) addiert nur Bits.
    # `mirror.mode` 4 -> 12 heisst "links→rechts UND rechts→links" und sieht
    # auf dem Untergrund aus wie die Vorgabe. Ein Gegenwert muss hier die
    # ACHSE wechseln, nicht die Bitzahl erhoehen (Befund S54).
    "mirror.mode": 1,  # 4 = links→rechts, 1 = oben→unten
    # 3D-Kamera: Vorgabe 0 bei Position und Blickziel, Panel-Bereich ±1000. Der
    # Randwert schiebt die Kamera aus der Szene — dann ist BEIDES leer und die
    # Sonde meldet "stumm". Diese Werte verschieben sichtbar, ohne die Szene zu
    # verlieren (Blickziel steht im Ursprung, Objekt fuellt [-1,1]).
    "camera3d.px": 1.5, "camera3d.py": 1.5,
    "camera3d.tx": 0.6, "camera3d.ty": 0.6, "camera3d.tz": 0.6,
    "camera3d.fogStart": 2.0, "camera3d.fogEnd": 5.0,
    "avi.filename": TESTVIDEO,
    "avi.resolvedPath": TESTVIDEO,
    # Reine Notizfelder — sie sollen nichts bewirken.
    "comment.text": "",
    "importNotes.text": "",
    "passthrough.note": "",
}


TRI_PUNKTE = ("x1=-0.6;y1=-0.5; x2=0.6;y2=-0.5; x3=0;y3=0.6; "
              "green=0.6;blue=0")

# GRUNDKONFIGURATION: Felder, die nur in Gesellschaft wirken koennen.
#
# `mirror.slower` steuert die Schrittweite einer RAMPE — ohne `smooth` gibt es
# keine Rampe, das Feld ist dann zu Recht wirkungslos. Ein Preset, das nur
# `slower` verstellt, meldet "STUMM" und sieht aus wie ein Befund, ist aber
# einer am Testaufbau. Die Zusatzfelder gehen in BEIDE Presets des Paares
# (Vorgabe UND gesetzt), damit sich weiterhin nur das eine Feld unterscheidet.
GRUNDKONFIG: dict[str, dict] = {
    "mirror.slower": {"smooth": True},
    # Dieselbe Logik wie bei Movement, andere Knoten: ohne Ausdruck ist der
    # Effekt die Identitaet, seine uebrigen Felder koennen nichts zeigen.
    # `bilinear` (Zwischenwerte beim Abtasten) zeigt sich nur an einer Abbildung,
    # die zwischen die Bildpunkte trifft — eine glatte Stauchung tut das.
    **{f"dynamicDistanceModifier.{f}": {"pixelCode": "d=d*0.63"}
       for f in ("subpixel", "blend")},
    # Bei einem Punkt-Knoten koennen Init/Frame/Beat nur ueber eine GETEILTE
    # Variable wirken: der Punkt-Code laeuft je Pixel und ueberschreibt `d`
    # ohnehin. Also setzt der gepruefte Slot `reg00`, und der Punkt-Code liest
    # es. So misst die Sonde wirklich den Slot und nicht den Punkt-Code.
    **{f"dynamicDistanceModifier.{f}": {"pixelCode": "d=reg00"}
       for f in ("initCode", "frameCode", "beatCode")},
    "triangle.filled": {"pointCode": TRI_PUNKTE},
    # `lineWidth` zeichnet nur den UMRISS — ein gefuelltes Dreieck hat keinen.
    "triangle.lineWidth": {"pointCode": TRI_PUNKTE, "filled": False},
    **{f"triangle.{f}": {"pointCode": TRI_PUNKTE + "; red=reg00"}
       for f in ("initCode", "frameCode", "beatCode")},
    # Color Modifier ist eine Nachschlagetabelle: ohne Kurve keine Aenderung.
    # `recompute` steuert, OB je Frame neu gerechnet wird — das zeigt sich nur
    # an einer Kurve, die sich ueber die Zeit aendert. Mit einer statischen
    # ergibt beides dasselbe Bild, und die Sonde meldet zu Recht "stumm".
    # `loadMode` bestimmt, wann der Init-Slot erneut laeuft — ohne Init-Code
    # gibt es nichts zu wiederholen. Der Ausdruck muss ausserdem von der
    # Zeit abhaengen, sonst liefert jeder Durchlauf denselben Wert.
    "jherikoGlobal.loadMode": {"initCode": "reg00=0.2+0.7*sin(time)"},
    # Vorgabe ist `mode = 0` (= aus). Erst als Schreiber (1) hat der Knoten
    # ueberhaupt eine Aufgabe, an der sich Puffer und Verzoegerung zeigen.
    **{f"multiDelay.{f}": {"mode": 1}
       for f in ("buffer", "delay", "useBeats", "initCode", "frameCode",
                 "beatCode")},
    "colorModifier.recompute": {"levelCode": "red=red*(0.5+0.5*sin(time)); "
                                             "green=green*0.5"},
    **{f"colorModifier.{f}": {"levelCode": "red=1-red; green=green*0.5"}
       for f in ("initCode", "frameCode", "beatCode")},
    # Movement ist ohne Abbildung die Identitaet — Randbehandlung, Rechenart
    # und Zwischenwerte koennen dann nichts zeigen.
    # `wrap` zeigt sich nur, wenn etwas ueber den Rand hinauslaeuft — eine
    # Abbildung nach INNEN (d*0.5) kann es nie ausloesen. Und der Ausdruck muss
    # POLAR sein: ohne `rectCoords` liest Movement `d`/`r`, ein `x=x+0.6`
    # bewegt dort nichts (Befund S54).
    "movement.wrap": {"code": "d=d*2.2"},
    "movement.rectCoords": {"code": "x=x+0.2; y=y*0.8"},
    "movement.subpixel": {"code": "d=d*0.97; r=r+0.05"},
    "movement.sourceMapped": {"code": "d=d*0.5"},
    "movement.blend": {"code": "d=d*0.5"},
    # Mosaic blendet nur auf Beat zurueck; ohne onBeat laeuft die Rampe nie.
    "mosaic.quality2": {"onBeat": True},
    "mosaic.durationFrames": {"onBeat": True},
    # Umgekehrt: der Schalter selbst zeigt sich nur, wenn die Beat-Qualitaet
    # von der normalen ABWEICHT — sonst schaltet er auf denselben Wert um.
    "mosaic.onBeat": {"quality2": 4},
    # Der Text-Knoten zeichnet ohne Inhalt nichts — dann kann keine Schriftart,
    # keine Ausrichtung und keine Farbe etwas zeigen (20 von 20 stumm, S54).
    **{f"text.{f}": {"text": "LumiViz"}
       for f in ("blend", "color", "fontHeight", "fontWeight", "hAlign", "vAlign",
                 "italic", "insertBlank", "normalizeSize", "onBeat", "outline",
                 "outlineColor", "randomPos", "shadow", "shiftSpeed", "speed",
                 "wordWrap", "x", "y", "beatFrames", "fontFamily", "initCode",
                 "frameCode", "beatCode")},
    # Textfelder, die eine zweite Zutat brauchen: eine Kontur hat nur, wer sie
    # einschaltet; eine Verschiebung sieht man nur mit Tempo; ein Beat-Verhalten
    # nur, wenn es an ist; ein Zufallswort nur bei mehreren Woertern.
    "text.outlineColor": {"text": "LumiViz", "outline": True},
    "text.outlineSize": {"text": "LumiViz", "outline": True},
    "text.underline": {"text": "LumiViz", "fontHeight": 40},
    "text.xShift": {"text": "LumiViz", "shiftSpeed": 8},
    "text.yShift": {"text": "LumiViz", "shiftSpeed": 8},
    "text.onBeatSpeed": {"text": "LumiViz", "onBeat": True},
    "text.normSpeed": {"text": "LumiViz", "normalizeSize": True},
    "text.randomWord": {"text": "Lumi Viz Test Wort"},
    "text.onBeat": {"text": "LumiViz", "onBeatSpeed": 20},
    # Ohne Video zeichnet der AVI-Knoten nichts, dann kann kein Regler wirken.
    **{f"avi.{f}": {"filename": TESTVIDEO, "resolvedPath": TESTVIDEO}
       for f in ("adapt", "blend", "persist", "speedMs")},
    # Seit die Sonden im Beat-Frame enden (181 Frames), gelten dort die
    # BEAT-Fader — die normalen sind dann verdeckt. Fuer sie muss das
    # Beat-Fenster geschlossen sein.
    **{f"colorfade.{f}": {"onBeatFrames": 0}
       for f in ("faderR", "faderG", "faderB")},
    # Interleave springt nur auf Beat auf x2/y2.
    "interleave.x2": {"onBeat": True},
    "interleave.y2": {"onBeat": True},
    "interleave.beatDuration": {"onBeat": True},
    # Timescope: der Kanal wirkt erst, wenn er angewandt wird (S54).
    "timescope.channel": {"useChannel": True},
    # Nebel ist aus, solange `fogStart >= fogEnd` (so der Header) — ohne ein
    # Ende dahinter kann weder Anfang noch Farbe etwas zeigen.
    "camera3d.fogStart": {"fogEnd": 8.0},
    "camera3d.fogColor": {"fogStart": 2.0, "fogEnd": 5.0},
    # --- aus dem ersten Vollauf (S54) ---
    # Julia-Saat zaehlt nur in den Julia-artigen Typen; bei Mandelbrot (Vorgabe)
    # liest der Shader sie nicht.
    "fractal2D.juliaX": {"type": 1}, "fractal2D.juliaY": {"type": 1},
    "fractalZoomer.juliaX": {"type": 1}, "fractalZoomer.juliaY": {"type": 1},
    "fractal3D.juliaX": {"type": 1}, "fractal3D.juliaY": {"type": 1},
    "fractal3D.juliaZ": {"type": 1}, "fractal3D.juliaW": {"type": 1},
    # Vignetten-Staerke ohne eingeschaltete Vignette.
    "bloom.vignetteStrength": {"vignette": True},
    # SuperScope zeichnet ohne Punkt-Code nichts — Farbe, Punktgroesse und
    # Zeichenart koennen dann nichts zeigen.
    "superScope.colors": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7"},
    "superScope.dotSize": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7",
                           "renderMode": 0},
    "superScope.renderMode": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7"},
    "superScope.colorCycleFrames": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7",
                                    "colors": [0xFF0000, 0x00FF00]},
    "superScope.colorBlend": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7"},
    # Bild-Knoten: JEDES andere Feld braucht das Bild, sonst zeichnet der
    # Knoten nichts und der Regler kann nichts zeigen.
    **{f"{t}.{f}": {"imageData": TESTBILD_B64}
       for t in ("picture", "pictureII", "texer", "texerII")
       for f in ("blend", "keepAspect", "x", "y", "ratio", "adjustBlend",
                 "onBeatSizeChange", "onBeatSize", "colors", "particles",
                 "initCode", "frameCode", "beatCode", "pointCode", "sizex",
                 "sizey", "resizing", "wrapAround", "maskEnabled", "numParticles")},
    # Colorfade: die Beat-Fader zaehlen nur waehrend des Beat-Fensters.
    "colorfade.beatFaderR": {"onBeatFrames": 60},
    "colorfade.beatFaderG": {"onBeatFrames": 60},
    "colorfade.beatFaderB": {"onBeatFrames": 60},
}


# NICHT PRUEFBAR mit dem synthetischen Audio des Standalone — mit Grund.
# Der Runner faellt hier KEIN Urteil: ein "STUMM" waere eine Falschmeldung
# ueber die App, obwohl die Grenze am Messaufbau liegt.
NICHT_PRUEFBAR: dict[str, str] = {
    **{f"passthrough.{f}":
       "Passthrough reicht das Bild unveraendert weiter — er SOLL nichts "
       "bewirken. Seine Felder sind Import-Notizen (welcher AVS-Effekt hier "
       "stand), keine Regler."
       for f in ("note", "sourceId")},
    "timescope.channel":
        "Standalone erzeugt das Spektrum fuer BEIDE Kanaele gleich "
        "(main.cpp: spec[b*2+0] == spec[b*2+1]); links/rechts/Mitte sind "
        "zwangslaeufig identisch. Braucht echtes Stereo-Material (TestAudio).",
    "timescope.useChannel":
        "s. timescope.channel — der Schalter kann nichts umschalten, solange "
        "beide Spektrumkanaele denselben Inhalt haben.",
}


# NACHFOLGER: Knoten, die NACH dem Pruefling in die Kette gehoeren.
#
# Ein Zustandsknoten zeichnet selbst nichts — er stellt etwas für die
# FOLGENDEN Knoten ein. Ohne Nachfolger ist im Bild nichts zu sehen, egal wie
# man ihn verstellt; im ersten Vollauf war deshalb bei 16 Typen jedes Feld
# stumm (S54). Der Nachfolger steht in BEIDEN Presets des Paares, er gehoert
# zum Aufbau und nicht zum Unterschied.
_SCOPE = {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7; red=1; green=1; blue=1",
          "pointCount": 128, "colors": [0xFFFFFF]}
NACHFOLGER: dict[str, list[dict]] = {
    # Set Render Mode setzt Linienbreite/Blend fuer die folgenden Zeichner.
    "setRenderMode": [L.node("superScope", "Zeuge", **_SCOPE)],
    # Global Variables setzt reg-Register; sichtbar erst, wenn sie jemand liest.
    "jherikoGlobal": [L.node("superScope", "Zeuge",
                             pointCode="x=cos(i*6.28)*(0.3+reg00*0.5); "
                                       "y=sin(i*6.28)*(0.3+reg00*0.5); red=1;green=1;blue=1",
                             pointCount=128, colors=[0xFFFFFF])],
    # Die 3D-Kamera wirkt auf 3D-Module, nicht auf das Bild — sie zeichnet
    # selbst nichts, das ist ihre Aufgabe. Sichtbar wird sie nur an einem
    # 3D-Modul dahinter. (Der Schluessel heisst `superScope3d` mit kleinem d;
    # mit `superScope3D` wurde daraus still ein Passthrough — s. pruefe_typen.)
    "camera3d": [L.node("superScope3d", "Zeuge",
                        pointCode="x=cos(i*6.28)*0.6; y=sin(i*6.28)*0.6; z=i*2-1; "
                                  "red=1; green=1; blue=1",
                        pointCount=128)],
    # Buffer Save speichert nur; sichtbar wird es beim Zurueckholen.
    "bufferSave": [L.node("clear", "Leeren", color=0x000000, onlyFirst=False),
                   L.node("bufferSave", "Zurueck", dir=1)],
    # Custom BPM faelscht den Beat — ein beat-reaktiver Nachfolger zeigt es.
    "customBpm": [L.node("onBeatClear", "Zeuge", everyNBeats=1)],
    # Multi Delay braucht ZWEI Knoten: einer schreibt in den geteilten
    # Puffer (mode 1), einer liest ihn zurueck (mode 2). Der Pruefling ist
    # der schreibende, dieser hier der lesende — sonst bleibt der Puffer
    # unbenutzt und jedes Feld ist stumm (S54).
    "multiDelay": [L.node("multiDelay", "Zurueck", mode=2, buffer=0)],
}

# VORLAUF: Knoten, die VOR dem Pruefling stehen.
#
# Ein Verzoegerungsspeicher zeigt ein Bild von vor N Frames. Auf unserem
# STATISCHEN Untergrund ist das dasselbe Bild — die Sonde sah nichts, obwohl
# der Effekt arbeitete (S54, `videoDelay`/`multiDelay` je 5-7 Felder stumm).
# Dieser Zeiger dreht sich, also unterscheidet sich jeder Frame vom vorigen.
BEWEGT = L.node("superScope", "Bewegt",
                pointCode="x=cos(time*3)*i*0.8; y=sin(time*3)*i*0.8; "
                          "red=1; green=1; blue=0.2",
                pointCount=64, renderMode=1, lineWidth=6, colors=[0xFFFFFF],
                colorCycleFrames=0)
# Geprueft S54 ueber ALLE Puffer- und Rueckkopplungsknoten: gebraucht wird der
# Vorlauf nur von den beiden echten Verzoegerungsspeichern. Blitter Feedback
# (1/8 stumm), Buffer Save (2/7) und Buffer Blend (2/6) verrechnen im SELBEN
# Frame und zeigen ihren Unterschied auch auf statischem Untergrund.
VORLAUF: dict[str, list[dict]] = {
    "videoDelay": [BEWEGT],
    "multiDelay": [BEWEGT],
}

# KINDER: Container zeichnen selbst nichts. Ihre Blend- und Pufferparameter
# regeln, WIE das Ergebnis der Kinder ins Bild kommt — ohne Kind gibt es kein
# Ergebnis und jeder ihrer Regler ist wirkungslos (S54: `list` 12 von 14 stumm).
KINDER: dict[str, list[dict]] = {
    "list": [L.node("superScope", "Kind",
                    pointCode="x=cos(i*6.28)*0.5; y=sin(i*6.28)*0.5; "
                              "red=1; green=0.2; blue=0.2",
                    pointCount=128, colors=[0xFFFFFF])],
    "hostgroup": [L.node("superScope", "Kind",
                         pointCode="x=cos(i*6.28)*0.5; y=sin(i*6.28)*0.5; "
                                   "red=0.2; green=1; blue=0.2",
                         pointCount=128, colors=[0xFFFFFF])],
}


def gegenwert(feld: dict, typkey: str, alle: dict[str, dict]):
    """Der abweichende Wert eines Feldes — oder None, wenn nicht ableitbar."""
    voll = f"{typkey}.{feld['name']}"
    if voll in HANDWERK:
        wert = HANDWERK[voll]
        return wert if wert != "" else None

    art = feld["art"]
    vorgabe = feld["default"]
    panel = feld.get("panel", "")

    if art == "bool":
        return not vorgabe

    if art == "zahl":
        if panel == "farbe":
            return 0xFF00FF
        if feld.get("enumWerte"):
            # Der entfernteste ZULAESSIGE Eintrag der Auswahl-Liste — plus dem
            # Versatz, falls das Panel Index und Feldwert verschiebt.
            versatz = int(feld.get("enumVersatz", 0))
            letzter = int(feld["enumWerte"]) - 1 + versatz
            erster = versatz
            return erster if vorgabe > (erster + letzter) / 2 else letzter
        lo, hi = feld.get("lo"), feld.get("hi")
        # Ein Panel-Bereich kann viel weiter sein als der BRAUCHBARE: `camera3d.pz`
        # darf -1000..1000, sinnvoll ist die Gegend um 3,7 — der Randwert schiebt
        # die Kamera aus der Szene, beide Bilder werden leer, die Sonde meldet
        # faelschlich "stumm".
        #
        # Die Daempfung greift NUR bei einer Vorgabe ungleich null, und zwar aus
        # Erfahrung: in der ersten Fassung galt sie auch bei Vorgabe 0, und dann
        # ist jeder Bereich "weit" (max(|0|,1) = 1). `brightness.red` (Vorgabe 0,
        # Bereich ±4096) bekam damit den Gegenwert 1 — ein Helligkeitsfaktor von
        # 1/256, unsichtbar. Der Lauf verlor dadurch neun wirksame Felder (S54).
        # Wo die Vorgabe 0 ist, gehoert ein brauchbarer Gegenwert nach HANDWERK.
        weit = (lo is not None and hi is not None and vorgabe != 0
                and (hi - lo) > 100 * abs(vorgabe))
        if lo is not None and hi is not None and not weit:
            wert = hi if abs(hi - vorgabe) >= abs(vorgabe - lo) else lo
        else:
            wert = 1 if vorgabe == 0 else vorgabe * 3
        # Ganzzahl bleibt Ganzzahl — ein QSpinBox-Feld mit 2.5 ist kein Test.
        if feld.get("helfer") == "addInt" or isinstance(vorgabe, int):
            wert = int(wert)
        return wert if wert != vorgabe else None

    if art == "liste":
        return [0xFF0000, 0x00FFFF]

    if art == "text":
        vars_ = feld.get("skriptvars") or []
        if not vars_:
            return None
        # ALLE Variablen des Knotens setzen, nicht nur die erste: waere die
        # erste eine der 43 wirkungslosen (S54), meldete der Runner "stumm"
        # fuer das ganze Skriptfeld, obwohl eine andere Variable sehr wohl
        # ankommt. So heisst stumm: KEINE wirkt.
        teile = []
        for ziel in vars_:
            # Der Bereich der Variablen ist der des gleichnamigen Feldes —
            # die Schreibweisen unterscheiden sich (`fadelen` vs `fadeLen`).
            zielfeld = alle.get(ziel) or next(
                (v for k, v in alle.items() if k.lower() == ziel.lower()), None)
            wert = gegenwert(zielfeld, typkey, alle) if zielfeld else 1
            if wert is None or isinstance(wert, (list, str)):
                wert = 1
            if isinstance(wert, bool):
                wert = 1 if wert else 0
            teile.append(f"{ziel} = {wert}")
        return "; ".join(teile)

    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("typkeys", nargs="*", help="nur diese Typen (Default: alle)")
    ap.add_argument("--out", type=Path, default=OUT)
    args = ap.parse_args()

    if not DOCS.exists():
        print("FEHLER: inventory_docs.json fehlt — erst harvest_field_docs.py laufen lassen")
        return 2
    docs = json.loads(DOCS.read_text(encoding="utf-8"))

    # Ein vertippter Typschluessel faellt sonst NIE auf: der Deserialisierer
    # macht aus unbekannten Typen bewusst einen Passthrough (damit ein neueres
    # Preset auf einem aelteren Build laedt). Ein solcher „Zeuge" zeichnet
    # nichts — und die Sonde meldet den Pruefling als stumm, statt den Tippfehler
    # zu zeigen (`superScope3D` statt `superScope3d`, S54).
    bekannt = {t["typkey"] for t in docs["typen"]}
    falsch = sorted({n["type"] for kette in NACHFOLGER.values() for n in kette}
                    - bekannt)
    if falsch:
        print("FEHLER: unbekannte Typschluessel in NACHFOLGER: " + ", ".join(falsch))
        return 2

    gebaut = uebersprungen = mit_grund = 0
    offen: list[str] = []
    nicht_pruefbar: list[str] = []
    for t in docs["typen"]:
        if args.typkeys and t["typkey"] not in args.typkeys:
            continue
        if not t["felder"]:
            continue
        typkey = t["typkey"]
        alle = {f["name"]: f for f in t["felder"]}
        kinder = KINDER.get(typkey, [])
        pruefling = L.node(typkey, t["name"], **({"children": kinder} if kinder else {}))

        nachfolger = NACHFOLGER.get(typkey, [])
        vorlauf = VORLAUF.get(typkey, [])
        L.write(args.out / typkey / "_default.lvfx",
                L.chain(*L.untergrund(), *vorlauf, pruefling, *nachfolger))

        for f in t["felder"]:
            wert = gegenwert(f, typkey, alle)
            if wert is None:
                uebersprungen += 1
                offen.append(f"{typkey}.{f['name']} ({f['art']})")
                continue
            voll = f"{typkey}.{f['name']}"
            if voll in NICHT_PRUEFBAR:
                nicht_pruefbar.append(voll)
                continue
            grund = GRUNDKONFIG.get(voll, {})
            knoten = L.node(typkey, t["name"],
                            **({"children": kinder} if kinder else {}),
                            **{**grund, f["name"]: wert})
            L.write(args.out / typkey / f"{f['name']}.lvfx",
                    L.chain(*L.untergrund(), *vorlauf, knoten, *nachfolger))
            if grund:
                # Eigener Vergleichsgrund: der Nachbar ist auch hier gesetzt,
                # sonst misst das Paar ZWEI Unterschiede statt einem.
                L.write(args.out / typkey / f"_grund_{f['name']}.lvfx",
                        L.chain(*L.untergrund(), *vorlauf,
                                L.node(typkey, t["name"],
                                       **({"children": kinder} if kinder else {}),
                                       **grund),
                                *nachfolger))
                mit_grund += 1
            gebaut += 1

    print(f"Feld-Sonden: {gebaut} gebaut ({mit_grund} mit eigener "
          f"Grundkonfiguration), {uebersprungen} ohne ableitbaren Gegenwert, "
          f"{len(nicht_pruefbar)} mit diesem Testsignal nicht pruefbar")
    for x in nicht_pruefbar:
        print(f"    nicht pruefbar: {x} — {NICHT_PRUEFBAR[x]}")
    if offen:
        print("\n-- kein Gegenwert (HANDWERK ergaenzen) --")
        for x in offen[:40]:
            print("   ", x)
        if len(offen) > 40:
            print(f"    … und {len(offen) - 40} weitere")
    return 0


if __name__ == "__main__":
    sys.exit(main())

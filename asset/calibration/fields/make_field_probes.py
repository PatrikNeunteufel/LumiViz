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
  Farbtafel     kraeftige Farben, an JEDER Stelle abweichend — bei einer Tafel
                mit fester Laenge so viele, wie sie Stellen hat (s. u.)
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

# Die Test-Videos (make_testvideo.py). Der AVI-Knoten laedt ueber Video for
# Windows von der Platte, nicht eingebettet.
#
# BLANKER NAME, kein absoluter Pfad (S73): `loadChainFile` ruft seit S73 auch
# `resolveAviPaths` auf — die Aufwaertssuche findet die Datei vom Sondenordner
# aus zwei Ebenen hoeher. Vorher stand hier `.resolve()`, und die erzeugten
# Sonden trugen den absoluten Pfad des Erstellers: auf jedem anderen Rechner
# tot, und der Benutzername stand im oeffentlichen Repo.
TESTVIDEO = "testvideo.avi"
# 24 Bit, der Normalfall bei unkomprimierten AVIs — bis S55 verwarf `runAvi`
# genau diese stillschweigend. Die beiden Pfad-Sonden laufen deshalb darauf:
# faellt der Fix, meldet `avi.filename` sofort STUMM.
TESTVIDEO24 = "testvideo24.avi"

# AUSNAHME `avi.resolvedPath`: dieses Feld IST der absolute Pfad — die Sonde
# setzt es ohne `filename`, und `resolveAviPaths` fasst einen gesetzten
# `resolvedPath` bewusst nicht an. Ein relativer Wert waere dort wirkungslos,
# darum bleibt es beim aufgeloesten Pfad. Diese eine Sonde ist damit an den
# Rechner gebunden, auf dem sie erzeugt wurde, und vor Gebrauch neu zu bauen.
TESTVIDEO24_ABS = (Path(__file__).parent / "testvideo24.avi").resolve().as_posix()

HIER = Path(__file__).parent
DOCS = HIER / "inventory_docs.json"
OUT = HIER / "probes"

# Felder, deren Gegenwert sich NICHT ableiten laesst — hier von Hand, mit
# Begruendung. Leerer Wert = bewusst uebersprungen (nicht sinnvoll pruefbar).
HANDWERK: dict[str, object] = {
    # Timescope: der Gegenwert 0 war Pixel fuer Pixel dieselbe Betriebsart wie
    # die Vorgabe — die Sonde meldete STUMM, obwohl das Feld wirkt. Gemessen
    # (S56): blend 0 gegen Vorgabe 0,0000 · blend 1 0,0249 · blend 2 0,1815.
    #
    # Der Grund ist NICHT, dass die Struct-Vorgabe 3 ("folge dem Set Render
    # Mode") auf dessen Vorgabe 0 hinauslaeuft — das war meine erste, ungenaue
    # Erklaerung. Der Deserialisierer traegt fuer dieses Feld eine EIGENE
    # Vorgabe, naemlich 0; eine geladene Sonde bekommt also direkt 0. Eine von
    # 20 solchen Doppelpflegen, seit S56 bewacht (`test_FieldInventory.cpp`,
    # "die Vorgaben sind nur EINMAL gepflegt") und in `Offene_Punkte.md §1d`
    # als Arbeitsliste gefuehrt.
    #
    # Genommen wird 2 (50/50), weil es als einziges auch dann nicht mit der
    # Vorgabe zusammenfaellt, wenn spaeter ein Set Render Mode mit anderem
    # `lineBlend` in den Untergrund geraet.
    "timescope.blend": 2,
    # Der Text-Knoten schaltet das Wort nach `normSpeed` Frames weiter. Der
    # Regelwert (Bereich 1..600 -> 600) wechselt in 181 Frames NIE, und die
    # Vorgabe 15 landet nach 12 Wechseln wieder auf Wort 0 von vieren — beide
    # zeigen dasselbe Wort. 60 gibt drei Wechsel und damit Wort 3 (S56).
    "text.normSpeed": 60,
    # `feedback` heisst "trail persistence 0..1", der Shader liest es aber nur
    # als SCHALTER (`params.feedback > 0.01f ? 2 : 0`,
    # MultiEffectVisualizer.cpp:10506). Der Regelwert 1,0 ist damit dieselbe
    # Betriebsart wie die Vorgabe 0,5. Nur 0 wechselt sie. Dass das Feld eine
    # Staerke verspricht und einen Schalter liefert, steht als Beobachtung in
    # `Offene_Punkte.md` — hier geht es nur um einen wirksamen Gegenwert.
    "fractalZoomer.feedback": 0.0,
    # Kleinian faerbt ueber `fract(Spiegelungszahl * colorScale)`, und die
    # Spiegelungszahl ist eine GANZE Zahl: jeder ganzzahlige Wert ergibt exakt
    # 0, also eine einfarbige Scheibe. Die Regel waehlt im Bereich 0,01..8,0 den
    # groessten Abstand zur Vorgabe — und landete damit auf 8,0, dem naechsten
    # ganzzahligen Wert. Zwei gleiche Bilder, Urteil „stumm" (S56).
    "kleinian.colorScale": 0.45,
    # ---------------------------------------------------------------- S56
    # Interferences: `rotation` ist eine GANZE Umdrehung in 0..255 — der
    # Regelwert 255 ist dieselbe Lage wie die Vorgabe 0.
    "interferences.rotation": 64,
    # Die Inversionsschleife bricht ab, sobald der Punkt einmal AUSSERHALB des
    # Inversionskreises liegt (`else break`) — sie laeuft also nur wenige Runden,
    # und die Obergrenze 200 (der Regelwert bei Vorgabe 30) aendert nichts mehr.
    # Ein Gegenwert muss die Kachelung ABSCHNEIDEN, nicht verlaengern.
    "kleinian.iterations": 1,
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
    "texerII.initCode": "reg00=0.1",
    # Blend-Regler, deren Regelwert (groesster Abstand) auf eine Betriebsart
    # faellt, die auf unserem Untergrund wie die Vorgabe aussieht. 50/50 ist
    # der Modus, der sich immer sichtbar unterscheidet.
    "starfield.blend": 2,
    "bufferSave.blend": 2,
    "texer.blend": 2,
    "dotGrid.blend": 2,
    # Movement: die beiden eingebauten Umbauten sind 1 ("slight fuzzify",
    # kaum sichtbar) und 7 ("blocky partial out", deutlich). Der Regelwert
    # nimmt 1 und verschwindet im Rauschen.
    "movement.builtinRemap": 7,
    # SuperScope: 1 (Linien) gegen 2 (dicke Linien) sieht bei gleicher
    # Strichstaerke fast identisch aus — der Sprung auf PUNKTE ist der
    # sichtbare Wechsel der Zeichenart.
    "superScope.renderMode": 0,
    # (`rotatingStars.bandHi` stand hier mit dem Gegenwert 5 — es ist mit
    # diesem Testsignal grundsaetzlich nicht prueefbar und deshalb nach
    # NICHT_PRUEFBAR umgezogen, samt Nachweis.)
    # 3D-Kamera: das Blickziel in der Tiefe. Sie braucht ZUSAETZLICH ein
    # Blickziel neben der Achse (s. GRUNDKONFIG) — sonst ist jeder Wert
    # dieselbe Blickrichtung.
    "camera3d.tz": 3.0,
    # Convolution: ein FALTUNGSKERN ist keine Farbtafel. Die Listen-Regel
    # fuellte alle 49 Stellen mit Palettenfarben (Gewichte um 16 Millionen) —
    # das Bild uebersteuert vollstaendig und die Sonde meldet „wirkt", ohne
    # etwas gemessen zu haben. Der Laplace-Kern ist derselbe, den die
    # Grundkonfiguration der Nachbarfelder benutzt (S57).
    "convolution.kernel": ([0] * 16 + [0, -1, 0] + [0] * 4 + [-1, 4, -1] +
                           [0] * 4 + [0, -1, 0] + [0] * 16),
    # Effect List: die LAENGE des Beat-Fensters in Frames. Die Daempfung fuer
    # weite Bereiche (1..200 gegen Vorgabe 1) machte daraus das Dreifache, also
    # 3 — und drei Frames sind bei einem Schlussframe fuenf Frames nach dem
    # Beat (186, s. Runner) genauso abgelaufen wie einer. Beide Fenster zu,
    # beide Bilder gleich, Urteil „stumm" (S57). Bei einer Frame-ANZAHL ist der
    # weite Wert der richtige: 200 haelt das Fenster ueber den ganzen Lauf offen.
    "list.onBeatFrames": 200,
    # Multi Delay: der NACHFOLGER liest den geteilten Puffer zurueck (mode 2),
    # also muss der Pruefling ihn SCHREIBEN. Der Regelwert 2 machte beide zu
    # Lesern — der Ring blieb leer, und "aus" (0) wie "lesen" (2) sind auf
    # einem leeren Ring derselbe No-op (S57).
    "multiDelay.mode": 1,
    # Buffer Blend: `8` heisst "der aktuelle Frame", `0..7` ein Speicherplatz
    # (`bufA >= 8 ? cur : poolTexture(bufA)`). Der Ernter findet fuer diese zwei
    # Felder keinen Bereich, also griff die Notregel "das Dreifache" — und 24
    # ist genau wie die Vorgabe 8 der aktuelle Frame. Zwei gleiche Bilder.
    "bufferBlend.bufferA": 0,
    "bufferBlend.bufferB": 0,
    # Effect List: die Variablen ihrer Slots sind `enabled`, `clear`, `beat`,
    # `alphain`, `alphaout` — ein abgeleiteter Gegenwert setzt eines davon im
    # INIT-Slot, und der Frame-Slot ueberschreibt es eine Zeile spaeter wieder.
    # Der Init-Slot kann hier also nur ueber eine geteilte Variable wirken
    # (dieselbe Bauart wie Triangle/DDM): er setzt `reg00`, der Frame-Slot der
    # Grundkonfiguration liest es als `enabled`.
    "list.initCode": "reg00=1",
    # Triangle: der abgeleitete Gegenwert setzt Variablen, aber keine ECKEN —
    # damit zeichnet der Knoten nichts, und der Vergleich laeuft gegen einen
    # Grund, der ebenfalls nichts zeichnet. Ein Punkt-Code IST hier die Geometrie
    # (dieselbe Logik wie bei Movement).
    "triangle.pointCode": ("x1=-0.8;y1=-0.6; x2=0.8;y2=-0.6; x3=0;y3=0.8; "
                           "red=1;green=0.3;blue=0"),
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
    # (`*.filename` steht jetzt in NICHT_PRUEFBAR — es ist eine Herkunftsnotiz,
    # geladen wird aus `imageData`. Als leerer HANDWERK-Eintrag sah es wie
    # offene Arbeit aus, obwohl es fertig ist.)
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
    # `tz` steht bewusst NICHT hier: es hatte einen zweiten Eintrag weiter oben
    # (3,0), und im Python-Dict gewinnt der SPAETERE — die 0,6 machte die
    # Korrektur still wirkungslos. Genau das Eigentor aus S56, nur andersherum
    # (die neue Zeile stand vor der alten).
    "camera3d.tx": 0.6, "camera3d.ty": 0.6,
    "camera3d.fogStart": 2.0, "camera3d.fogEnd": 5.0,
    # Colorfade sortiert die drei Fader je Pixel nach der Kanalfolge um: der
    # ZWEITE landet immer auf dem groessten Kanal (r_colorfade.cpp:176-186).
    # Der ist auf unseren Testbalken 255 — ein POSITIVER Gegenwert wird dort
    # weggeschnitten und die Sonde meldet „schwach". Vorgabe ist -8, die
    # Abstandsregel waehlt deshalb +32; hier zaehlt aber die Richtung, nicht
    # der Abstand (Befund S55). `faderG` braucht den Eintrag nicht: seine
    # Vorgabe ist +8, der Regelwert also ohnehin -32.
    "colorfade.beatFaderG": -32,
    # Beat-FENSTER: der Regelwert waere das Dreifache der Vorgabe, also 3 — bei
    # einer Lauflaenge von 55 (24 Frames hinter dem Beat) ist ein Fenster von 3
    # genauso abgelaufen wie eines von 1. 30 haelt bis zum Schluss.
    "colorfade.onBeatFrames": 30,
    "avi.filename": TESTVIDEO24,
    "avi.resolvedPath": TESTVIDEO24_ABS,
    # ------------------------------------------------------------------------
    # EFFEKT-Skripte (S55). Nicht zu verwechseln mit den PARAMETER-Skripten des
    # Strangs D: die rechnen einmal je Frame die Felder des Knotens aus, und
    # ihre Variablen liest der Ernter selbst aus `runParamScript`. Die hier
    # laufen je Punkt / Gitterpunkt / Tabelleneintrag, haben einen ganz anderen
    # Variablensatz und einen eigenen Traeger (ScriptSlotHost, ScriptGrid,
    # ScriptLut) — es gibt also nichts abzuleiten, nur einzusetzen. Ein
    # Gegenwert ist hier schlicht ein ANDERER Ausdruck.
    # Die Punkt-/Kurven-Slots selbst braucht HANDWERK NICHT mehr: seit S55
    # erntet `harvest_field_docs.py` die Variablen dieser Traeger (dritter
    # Mechanismus, `number("…")` im Renderer bzw. im Modul), und der Generator
    # baut daraus dieselben Sonden wie fuer die Parameter-Skripte.
    #
    # Init/Frame/Beat DIESER Knoten bleiben Handarbeit, und zwar aus einem
    # Grund, den keine Ernte kennen kann: der Punkt-/Kurven-Code laeuft
    # DANACH und je Element — er ueberschreibt alles, was ein Frame-Slot in
    # dieselbe Variable geschrieben hat. Gemessen: mit dem automatischen
    # Gegenwert meldeten alle drei Slots von Color Modifier und Dynamic
    # Movement „stumm". Also schreibt der geprueste Slot `reg00`, und der
    # Punkt-Code aus der GRUNDKONFIG liest es — dieselbe Bauart wie bei
    # Triangle und DDM seit S54.
    **{f"superScope.{f}": "reg00=0.75"
       for f in ("initCode", "frameCode", "beatCode")},
    **{f"dynamicMovement.{f}": "reg00=1.6"
       for f in ("initCode", "frameCode", "beatCode")},
    **{f"colorModifier.{f}": "reg00=0.15"
       for f in ("initCode", "frameCode", "beatCode")},
    # ------------------------------------------------------------------------
    # Farbverlaeufe: der Gegenwert ist ein anderer gueltiger Name aus der
    # Registry (ColorGradientModule: Fire · Ocean · Neon · Rainbow · Sunset ·
    # Forest · Ice · Lava · Galaxy · Monochrome). `Monochrome` ist der
    # deutlichste Gegensatz zu jeder bunten Vorgabe — nur fuer den Knoten, der
    # selbst monochrom vorbelegt waere, braeuchte es einen anderen.
    **{f"{t}.gradientPreset": "Monochrome"
       for t in ("domainWarp", "flame", "fractal2D", "fractal3D",
                 "fractalZoomer", "kleinian", "lyapunov", "reactionDiffusion",
                 "strangeAttractor", "superScope")},
    # Text-Knoten: die Vorgabe ist leer, also ist JEDER Text der Gegenwert.
    # Die Schriftart braucht einen Text, sonst steht nichts da (s. GRUNDKONFIG).
    "text.text": "LumiViz;Sonde",
    "text.fontFace": "Impact",
    # Lyapunov: die Folge ist das Muster selbst (A/B), Vorgabe `AB`.
    "lyapunov.sequence": "AABAB",
    # Vorgabe 0 und kein deklarierter Bereich — die Abstandsregel hat nichts,
    # woran sie sich halten koennte. 0,05 rad/Frame dreht sichtbar, ohne zu
    # ueberdrehen.
    "fractalZoomer.rotationSpeed": 0.05,
    # (Die reinen Notizfelder stehen jetzt ebenfalls in NICHT_PRUEFBAR.)
}


TRI_PUNKTE = ("x1=-0.6;y1=-0.5; x2=0.6;y2=-0.5; x3=0;y3=0.6; "
              "green=0.6;blue=0")

# UNTERGRUND JE TYP: wer je Frame nur einen Bruchteil des Bildes zeichnet,
# braucht einen Untergrund, der nur EINMAL loescht.
#
# Timescope zeichnet eine EIN PIXEL breite Spalte je Frame und schiebt sie um
# eins weiter (`rt.timescopeX`). Der Untergrund malt sie im Folgeframe wieder
# zu — im Schlussbild steht damit genau EINE Spalte von 320. Alle acht Felder
# lagen deshalb unter der SCHWACH-Schwelle (MAE 0,0004..0,0009), obwohl sie
# sichtbar wirken: die Sonde mass 1/320 ihrer Wirkung (Befund S55).
#
# Mit `onlyFirst` sammeln sich die Spalten ueber die ganze Lauflaenge. Die
# Balken und die Diagonale werden weiter je Frame gezeichnet — sie bleiben also
# als Orientierung stehen und schneiden nur ihre eigenen Zeilen aus.
UNTERGRUND_JE_TYP: dict[str, bool] = {
    "timescope": True,
}

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
    # Der geprueste Slot schreibt `reg00`, die Kurve liest es — sonst misst die
    # Sonde die Kurve statt den Slot (S55, dieselbe Bauart wie Triangle/DDM).
    **{f"colorModifier.{f}": {"levelCode": "red=reg00; green=green*0.5"}
       for f in ("initCode", "frameCode", "beatCode")},
    # Punkt-Knoten: ohne Punkt-Code zeichnen sie nichts, und ihre Init-/Frame-/
    # Beat-Slots koennen nur ueber eine geteilte Variable wirken.
    **{f"superScope.{f}": {"pointCode": "x=i*2-1; y=reg00*sin(i*6.283); "
                                        "red=1; green=1; blue=0.2"}
       for f in ("initCode", "frameCode", "beatCode")},
    **{f"dynamicMovement.{f}": {"pointCode": "d=d*reg00"}
       for f in ("initCode", "frameCode", "beatCode")},
    # Ohne Text zeichnet der Knoten nichts, dann kann keine Schriftart wirken.
    "text.fontFace": {"text": "LumiViz"},
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
    # `shiftSpeed` und `normalizeSize` gibt es bei diesem Knoten NICHT — die
    # drei Eintraege standen bis S56 mit einem erfundenen Nachbarn da und waren
    # damit wirkungslos (Befund des Tabellen-Waechters oben). Und wo ein Wort
    # gegen ein anderes getauscht wird, braucht es MEHRERE: mit nur "LumiViz"
    # zeigt jeder Wortwechsel wieder dasselbe Wort.
    #
    # Der Trenner ist ein SEMIKOLON (`r_text.cpp`, s. `TextParams::text`) —
    # "Lumi;Viz;Test;Wort" ist EIN Wort mit Leerzeichen, und die vier
    # Wortwechsel-Felder blieben damit zu Recht stumm (S56, zweiter Anlauf).
    "text.xShift": {"text": "LumiViz"},
    "text.yShift": {"text": "LumiViz"},
    "text.onBeatSpeed": {"text": "Lumi;Viz;Test;Wort", "onBeat": True},
    "text.normSpeed": {"text": "Lumi;Viz;Test;Wort"},
    "text.randomWord": {"text": "Lumi;Viz;Test;Wort"},
    "text.onBeat": {"text": "Lumi;Viz;Test;Wort", "onBeatSpeed": 20},
    # Ohne Video zeichnet der AVI-Knoten nichts, dann kann kein Regler wirken.
    **{f"avi.{f}": {"filename": TESTVIDEO, "resolvedPath": TESTVIDEO}
       for f in ("adapt", "blend", "speedMs")},
    # `persist` ist die LAENGE des Beat-Fensters, und dieses Fenster wird nur
    # abgefragt, wenn `adapt` an ist (`runAvi`: blend = adapt ? … : params.blend).
    # Ohne den Schalter ist das Feld zu Recht wirkungslos. Zusaetzlich braucht es
    # einen Schlussframe ohne Beat — im Beat-Frame ist das Fenster IMMER offen
    # (`m_frameBeat || aviPersistLeft > 0`), also s. FRAMES_JE_FELD.
    "avi.persist": {"filename": TESTVIDEO, "resolvedPath": TESTVIDEO,
                    "adapt": True},
    # Die normalen Fader brauchen KEINE Grundkonfiguration, sondern einen
    # Schlussframe ohne Beat — siehe FRAMES_JE_FELD im Runner. Der Versuch,
    # das Beat-Fenster hier mit `onBeatFrames: 0` zu schliessen, war
    # wirkungslos: der Validator hebt jede 0 auf 1 (EffectChain.hpp:2255,
    # Vertrag `>= 1`, Panel-Bereich 1..200). Die Sonde stand damit auf
    # demselben Fenster wie ihr Vergleichsbild und mass im Beat-Frame nur die
    # BEAT-Fader — dreimal „STUMM", das keiner war (Befund S55).
    # Bump: `depth2` und `durationFrames` beschreiben die Beat-RAMPE — ohne
    # `onBeat` gibt es sie nicht. Und ohne bewegte Lichtquelle bleibt das Bild
    # ueber die Frames gleich, dann zeigt auch die Rampe nichts. Beides stand
    # bis S56 nur deshalb nicht hier, weil der Struct einen Demo-Frame-Code als
    # Vorgabe trug; der ist mit der SSOT-Umstellung entfallen (§1d), und die
    # Sonde muss ihn jetzt selbst mitbringen.
    **{f"bump.{f}": {"onBeat": True,
                     "frameCode": "x=0.5+cos(t)*0.3; y=0.5+sin(t)*0.3; t=t+0.1;"}
       for f in ("depth2", "durationFrames")},
    # ---------------------------------------------------------------- S56
    # Effect List: der Container zeichnet nichts selbst, seine Regler bestimmen
    # nur, WIE das Ergebnis der Kinder ins Bild kommt. Jeder von ihnen setzt
    # eine Betriebsart voraus.
    "list.inAdjustAlpha": {"blendIn": 10},    # 10 = Adjustable
    "list.outAdjustAlpha": {"blendOut": 10},
    "list.bufferIn": {"blendIn": 12},         # 12 = Buffer
    "list.bufferInInvert": {"blendIn": 12},
    "list.bufferOut": {"blendOut": 12},
    "list.bufferOutInvert": {"blendOut": 12},
    "list.onBeatFrames": {"onBeatRender": True},
    # Die EEL-Slots der Liste laufen nur mit `useCode`; der Frame-Slot schaltet
    # ueber `enabled`, der Init-Slot kann nur ueber eine geteilte Variable
    # wirken (dieselbe Bauart wie Triangle/DDM).
    "list.useCode": {"frameCode": "enabled=0"},
    "list.frameCode": {"useCode": True},
    "list.initCode": {"useCode": True, "frameCode": "enabled=reg00"},
    # Das Beat-Fenster der Effect List steuert AUSSCHLIESSLICH statisch
    # deaktivierte Listen — so steht es im Renderer und so ist es referenztreu
    # (`r_list enabled() = !bit1 || fake_enabled`; eine eingeschaltete Liste
    # rendert immer, `beat_render` hat auf sie keine Wirkung). Ohne
    # `enabled: false` sind beide Felder per Entwurf wirkungslos (S56).
    "list.onBeatRender": {"enabled": False},
    "list.onBeatFrames": {"enabled": False, "onBeatRender": True},
    # Interferences: die `*2`-Werte sind die BEAT-Ziele, `speed` die Dauer des
    # Uebergangs dorthin — ohne `onBeat` gibt es den Uebergang nicht.
    **{f"interferences.{f}": {"onBeat": True}
       for f in ("alpha2", "distance2", "rotationInc2", "speed")},
    # Color Clip: `distance` ist der Trefferradius des Modus 3 ("near"); die
    # Modi 1/2 vergleichen nur gegen die Schwelle.
    **{f"colorClip.{f}": {"mode": 3}
       for f in ("distance", "initCode", "frameCode", "beatCode")},
    # Brightness: ohne eine Aenderung an den Kanaelen aendert der Knoten nichts,
    # dann kann auch die Ausnahme nichts ausnehmen.
    "brightness.exclude": {"red": 2000},
    "brightness.color": {"exclude": True, "red": 2000},
    "brightness.distance": {"exclude": True, "red": 2000},
    # Convolution: mit dem Identitaets-Kern (Vorgabe) bleibt das Bild gleich —
    # Betrag, Randart und Doppelanwendung koennen dann nichts zeigen.
    **{f"convolution.{f}": {"kernel": [0]*16 + [0, -1, 0] + [0]*4 + [-1, 4, -1] + [0]*4 + [0, -1, 0] + [0]*16}
       for f in ("absolute", "twoPass")},
    # edgeMode ("wrap") = psubw statt psubusw — wirkt NUR bei scale >= 2 (bei
    # scale 1 emittiert der Original-JIT keinen Divisionspfad und wrap ist
    # wirkungslos; an der APE vermessen S60). Der Laplace-Kern liefert die
    # noetigen NEGATIVEN Zwischenwerte, scale 2 den Divisionspfad.
    "convolution.edgeMode": {"kernel": [0]*16 + [0, -1, 0] + [0]*4 +
                                       [-1, 4, -1] + [0]*4 + [0, -1, 0] + [0]*16,
                             "scale": 2},
    # 3D-Kamera: die Kamera steht auf der z-Achse (`pz` = 3,73) und blickt auf
    # (0, 0, `tz`). Die Blickrichtung wird NORMALISIERT — fuer jedes tz < pz
    # ist sie deshalb exakt (0, 0, -1), der Betrag kuerzt sich heraus. Die
    # Tiefe des Blickziels wird erst zu einem WINKEL, wenn das Ziel neben der
    # Achse liegt (S57: mit tx = 0,6 dann MAE 0,0345).
    "camera3d.tz": {"tx": 0.6},
    # Terrain 3D: die Palette laeuft ueber die Hoehe (`colorLow` im Tal,
    # `colorHigh` am Gipfel). Mit der Vorgabe `ringAmp` = 1 schiebt das
    # Spektrum das ganze Gelaende nach oben — dann ist NUR die Gipfelfarbe im
    # Bild und die Talfarbe kann nichts zeigen. Ohne Ring-Injektion und mit
    # weiter Hoehenspanne sind beide zu sehen (S57: MAE 0,0580).
    "terrain3d.colorLow": {"ringAmp": 0.0, "baseAmp": 0.5},
    # Water Bump setzt den Tropfen an eine ZUFAELLIGE Stelle, solange
    # `randomDrop` an ist — die feste Position wird dann gar nicht gelesen.
    "waterBump.dropX": {"randomDrop": False},
    "waterBump.dropY": {"randomDrop": False},
    # Roto Blitter: 31 ist bei Zoom und Drehung der neutrale Wert. Ein
    # Beat-Sprung auf denselben Wert ist keiner, und eine Richtungsumkehr
    # braucht eine Richtung.
    "rotoBlitter.zoomScale2": {"beatZoomJump": True},
    "rotoBlitter.beatZoomJump": {"zoomScale2": 12},
    "rotoBlitter.beatReverse": {"rotDir": 45},
    "rotoBlitter.beatReverseSpeed": {"rotDir": 45, "beatReverse": True},
    # Starfield / Moving Particle / Mosaic: dieselbe Bauart — ein Beat-Zielwert
    # ohne eingeschalteten Beat-Sprung, und ein Beat-Sprung auf denselben Wert.
    "starfield.beatSpeed": {"onBeat": True},
    "starfield.durationFrames": {"onBeat": True, "beatSpeed": 24.0},
    "movingParticle.size2": {"onBeatSize": True},
    "movingParticle.onBeatSize": {"size2": 24},
    # Set Render Mode / Buffer Save / Color Map: die Adjustable-Alpha wirkt nur
    # in der Adjustable-Betriebsart — und deren NUMMER ist je Knoten eine
    # andere, weil es drei verschiedene Aufzaehlungen sind. Wer eine davon
    # raet, klemmt still daneben: `lineBlend` wird auf 0..9 geklemmt
    # (`runSetRenderMode`), aus der geratenen 10 wurde also 9 = Minimum, und
    # `adjustAlpha` galt zwei Vollaeufe lang als stumm (S57).
    #   setRenderMode.lineBlend  Panel-Liste 0..9, Adjustable = 7
    #   bufferSave.blend         enum BlendMode,   Adjustable = 10
    #   colorMap.blendMode       eigene Liste,     Adjustable = 9
    "setRenderMode.adjustAlpha": {"lineBlend": 7},
    "setRenderMode.overrideBlend": {"lineBlend": 9},   # 9 = Minimum, deutlich
    "bufferSave.adjustAlpha": {"blend": 10},
    "colorMap.adjustBlend": {"blendMode": 9},
    # Farbverlauf-Stuetzstellen: Position und Farbe sind ein PAAR. Eine Liste
    # ohne die andere laesst der Knoten fallen.
    "colorMap.stopPos": {"stopColor": [0xFF0000, 0x0000FF]},
    "colorMap.stopColor": {"stopPos": [0, 255]},
    # Host-Gruppe: dito Adjustable.
    "hostgroup.outAdjustAlpha": {"blendOut": 10},
    # SuperScope: die Farbtafel wird nur in der Tabellen-Betriebsart gelesen —
    # bei `colorBlend = 0` (Vorgabe) gewinnt der Farbverlauf.
    "superScope.colors": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7",
                          "colorBlend": 1},
    "superScope.colorCycleFrames": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7",
                                    "colorBlend": 1,
                                    "colors": [0xFF0000, 0x00FF00]},
    # SuperScope 3D zeichnet ohne Punkt-Code nichts.
    **{f"superScope3d.{f}": {"pointCode": "x=cos(i*6.28)*0.6; y=sin(i*6.28)*0.6; "
                                          "z=0; red=1; green=1; blue=1"}
       for f in ("pointCount", "audioChannel", "spectrumSource")},
    # Texer II: jedes andere Feld braucht ein Bild und Punkte, sonst zeichnet
    # der Knoten nichts.
    **{f"texerII.{f}": {"imageData": TESTBILD_B64,
                        "pointCode": "x=(i*2)-1; y=sin(i*6.28)*0.5; "
                                     "sizex=2; sizey=2; red=1; green=0.5; blue=0.2"}
       for f in ("colorFiltering", "resizing", "wrapAround")},
    "texerII.imageData": {"pointCode": "x=(i*2)-1; y=sin(i*6.28)*0.5"},
    "texerII.pointCode": {"imageData": TESTBILD_B64},
    # Texer II zeichnet je Punkt ein Sprite — ohne Punkt-Code keine Punkte.
    # Der Punkt-Code steht seit S56 nicht mehr als Vorgabe im Struct (§1d),
    # also bringt die Sonde ihn selbst mit.
    #
    # Und er braucht ein `n`: anders als SuperScope hat Texer II KEIN
    # `pointCount`-Feld — die Zahl der Sprites kommt allein aus dem Init-Slot.
    # Ohne ihn laeuft der Punkt-Code null mal. Gemessen (S56): der Knoten mit
    # Bild und Punkt-Code, aber ohne `n`, ist Pixel fuer Pixel dasselbe Bild wie
    # GAR KEIN Knoten (MAE 0,0000) — deshalb waren alle fuenf Felder stumm.
    **{f"texerII.{f}": {"imageData": TESTBILD_B64,
                        "initCode": "n=48",
                        "pointCode": "x=(i*2)-1; y=sin(i*6.28)*0.5; "
                                     "sizex=2; sizey=2; "
                                     "red=1; green=0.5; blue=0.2"}
       for f in ("blend", "colorFiltering", "resizing", "wrapAround",
                 "frameCode", "beatCode")},
    # Der Init-Slot selbst wird geprueft — sein `n` darf dann nicht schon im
    # Grund stehen; der Frame-Slot bringt die Punkte mit.
    "texerII.initCode": {"imageData": TESTBILD_B64,
                         "frameCode": "n=48",
                         "pointCode": "x=(i*2)-1; y=sin(i*6.28)*0.5; "
                                      "sizex=2; sizey=2; red=reg00"},
    "texerII.imageData": {"initCode": "n=48",
                          "pointCode": "x=(i*2)-1; y=sin(i*6.28)*0.5; "
                                       "sizex=2; sizey=2"},
    "texerII.pointCode": {"imageData": TESTBILD_B64, "initCode": "n=48"},
    # Blitter Feedback: `scale2` ist der BEAT-Zielwert, `onBeat` der Sprung
    # dorthin — ein Sprung auf denselben Wert ist keiner.
    "blitterFeedback.scale2": {"onBeat": True},
    "blitterFeedback.onBeat": {"scale2": 120},
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
    # Julia-Saat und Formbeiwerte zaehlen nur in den Typen, die sie lesen; bei
    # der Vorgabe (Mandelbrot bzw. Mandelbulb) liest der Shader sie nicht.
    #
    # Der Schluessel heisst `ftype`, NICHT `type` — `type` ist im Knoten-JSON
    # der KNOTENTYP. Bis S56 stand hier `{"type": 1}`, und das machte aus dem
    # Fractal-Knoten den unbekannten Typ "1": der Deserialisierer baut daraus
    # bewusst einen Passthrough, beide Bilder des Paares blieben leer, und acht
    # Sonden meldeten STUMM. Genau die Falle, vor der der Kommentar bei
    # NACHFOLGER warnt — jetzt bewacht (s. pruefe_tabellen unten).
    "fractal2D.juliaX": {"ftype": 1},     # 1 = Julia
    "fractal2D.juliaY": {"ftype": 1},
    "fractal2D.power": {"ftype": 4},      # 4 = Multibrot (Exponent)
    "fractalZoomer.juliaX": {"ftype": 1},
    "fractalZoomer.juliaY": {"ftype": 1},
    "fractal3D.juliaX": {"ftype": 3},     # 3 = Quaternion-Julia, nicht 1
    "fractal3D.juliaY": {"ftype": 3},
    "fractal3D.juliaZ": {"ftype": 3},
    "fractal3D.juliaW": {"ftype": 3},
    "fractal3D.scale": {"ftype": 1},      # 1 = Mandelbox
    "fractal3D.fold": {"ftype": 1},
    # Vignetten-Staerke ohne eingeschaltete Vignette.
    "bloom.vignetteStrength": {"vignette": True},
    # SuperScope zeichnet ohne Punkt-Code nichts — Farbe, Punktgroesse und
    # Zeichenart koennen dann nichts zeigen.
    # `colorBlend = 0` (Vorgabe) faerbt ueber den VERLAUF — die Farbtafel
    # wird dann gar nicht gelesen, und Tafel wie Umlauf sind zu Recht
    # stumm. Erst Betriebsart 1 (Tabelle) macht sie messbar (S56).
    "superScope.colors": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7", "colorBlend": 1},
    "superScope.dotSize": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7",
                           "renderMode": 0},
    "superScope.renderMode": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7", "lineWidth": 8},
    "superScope.colorCycleFrames": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7",
                                    "colorBlend": 1,
                                    "colors": [0xFF0000, 0x00FF00]},
    "superScope.colorBlend": {"pointCode": "x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7"},
    # Bild-Knoten: JEDES andere Feld braucht das Bild, sonst zeichnet der
    # Knoten nichts und der Regler kann nichts zeigen.
    **{f"{t}.{f}": {"imageData": TESTBILD_B64}
       for t in ("picture", "pictureII", "texer")
       for f in ("blend", "keepAspect", "x", "y", "ratio", "adjustBlend",
                 "onBeatSizeChange", "onBeatSize", "colors", "particles",
                 "initCode", "frameCode", "beatCode", "pointCode", "sizex",
                 "sizey", "resizing", "wrapAround", "maskEnabled", "numParticles")},
    # Colorfade: die Beat-Fader zaehlen nur waehrend des Beat-Fensters — UND nur
    # mit `slowFade`. Ohne dieses Bit ist der Beat-Zweig im Original gar nicht
    # erreichbar (`if (!(enabled&4)) … else if (isBeat) …`, r_colorfade.cpp:149),
    # die Fader stehen dann sofort auf ihrem Normalwert. Seit der Portierung
    # (S57) gilt das auch bei uns, und damit brauchen alle vier beat-bezogenen
    # Felder das Bit in ihrer Grundkonfiguration.
    **{f"colorfade.{f}": {"slowFade": True}
       for f in ("beatFaderR", "beatFaderG", "beatFaderB")},
    "colorfade.onBeatRandom": {"slowFade": True},
    # Das Beat-FENSTER (LumiViz-Erweiterung) braucht zwei Dinge: `slowFade`,
    # sonst gibt es keinen Beat-Zustand zum Halten — und Beat-Fader, die vom
    # Nachziehziel ABWEICHEN. Mit den Vorgaben tun sie das nicht: das Nachziehen
    # vertauscht Gruen und Blau (`faderpos[1]` folgt `faders[2]`), und damit ist
    # sein Ziel (8, -8, 8) zufaellig genau die Beat-Fader-Belegung. Der Zustand
    # steht nach einem Beat also schon am Ziel, und die Fensterlaenge aendert
    # kein Pixel (S57).
    # `beatFaderG`, nicht `beatFaderR`: Colorfade sortiert die drei Fader je Pixel
    # nach der Kanalfolge, und der ZWEITE landet auf dem groessten Kanal
    # (r_colorfade.cpp:176-186, Merkregel S55). Auf unseren Testbalken ist der
    # kleinste Kanal 0 — ein negativer Fader wird dort weggeklemmt, und der
    # Unterschied zwischen gehaltenem und nachgezogenem Wert war unsichtbar.
    "colorfade.onBeatFrames": {"slowFade": True, "beatFaderG": -32},
    # Ein BEAT-Zielwert, der dem Normalwert gleicht, ist kein Ziel — und ein
    # Uebergang dorthin nicht sichtbar. Dieselbe Bauart bei vier Knoten (S56).
    "interleave.onBeat": {"x2": 8, "y2": 8},
    "interleave.beatDuration": {"onBeat": True, "x2": 8, "y2": 8},
    "mosaic.durationFrames": {"onBeat": True, "quality2": 4},
    "interferences.alpha2": {"onBeat": True, "distance2": 40},
    "interferences.distance2": {"onBeat": True, "alpha2": 250},
    # Mirror: eine RAMPE braucht ein wechselndes Ziel, und die Zufallswahl
    # braucht mehrere Achsen zur Auswahl (`mode` ist ein Bitfeld).
    "mirror.smooth": {"onBeatRandom": True, "mode": 15},
    "mirror.onBeatRandom": {"mode": 15},
    # Custom BPM: die drei Betriebsarten schliessen einander aus, und jede
    # liest NUR ihren eigenen Wert (r_bpm.cpp, Befund S52).
    "customBpm.arbitraryMs": {"arbitrary": True},
    "customBpm.skipCount": {"skip": True},
    **{f"customBpm.{f}": {"skip": True}
       for f in ("initCode", "frameCode", "beatCode")},
    # Kanal- und Quellenfelder wirken nur, wenn das Skript den Audiowert
    # ueberhaupt LIEST — mein erster Punkt-Code fuer SuperScope 3D benutzte `v`
    # gar nicht, damit war die Quelle gleichgueltig (S56).
    **{f"superScope3d.{f}": {"pointCode": "x=cos(i*6.28)*0.6; y=v*0.9; "
                                          "z=v*0.5; red=1; green=1; blue=1"}
       for f in ("audioChannel", "spectrumSource")},
    # Osc Ring liest bei `source = 0` die WELLENFORM; unser Testsignal ist dort
    # in beiden Kanaelen gleich. Ueber das Spektrum ist der Kanal messbar
    # (`--stereo-spektrum`, s. Runner).
    "oscRing.channel": {"source": 1},
    # Strange Attractor: die feste Farbe wird nur gelesen, wenn NICHT ueber den
    # Verlauf eingefaerbt wird (`useGradient`, Vorgabe an).
    # Und Betriebsart 0 ist "ersetzen" — der Shader liest dort AUSSCHLIESSLICH
    # `b` (`if (uMode == 0) r = b;`). Puffer A kann damit per Entwurf nichts
    # bewirken; erst eine Betriebsart, die beide Quellen verrechnet, macht ihn
    # messbar (3 = 50/50).
    "bufferBlend.bufferA": {"mode": 3},
    "strangeAttractor.color": {"useGradient": False},
    # Und `d` ist der vierte Formelbeiwert — Lorenz (Vorgabe, `ftype` 0) nutzt
    # ihn gar nicht, erst Clifford/De Jong/Aizawa lesen ihn.
    "strangeAttractor.d": {"ftype": 1},
    # Dynamic Movement: der Unterschied zwischen polar und kartesisch zeigt sich
    # nur an einem Ausdruck, der in BEIDEN Systemen etwas anderes bedeutet.
    # Ohne `rectCoords` liest der Knoten `d`/`r`, ein `x=x+0.3` bewegt dort
    # nichts (dieselbe Falle wie bei `movement.rectCoords`, S54).
    "dynamicMovement.rectCoords": {"pointCode": "x=x+0.3; y=y*0.7"},
    # Texer II: der Randumlauf zeigt sich nur an Sprites, die ueber den Rand
    # hinausragen.
    "texerII.wrapAround": {"imageData": TESTBILD_B64,
                           "initCode": "n=24",
                           "pointCode": "x=(i*4)-2; y=sin(i*6.28)*1.4; "
                                        "sizex=3; sizey=3"},
    # Blur: der Rundungs-Bias verschiebt je Pixel um hoechstens eine Stufe —
    # bei der schwaechsten Stufe (Vorgabe) verschwindet das. Mit `strength = 3`
    # summiert sich die Rundung ueber mehr Taps.
    "blur.roundUp": {"strength": 3},
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
    # Dieselbe Sorte: Text, den niemand zeichnet. Sie standen bis S55 in der
    # Restliste „kein Gegenwert" und sahen dort wie Arbeit aus — sie sind aber
    # fertig, nur eben unprueefbar, weil sie per Entwurf nichts bewirken.
    "comment.text": "Reines Notizfeld am Kommentar-Knoten — es wird nirgends "
                    "gezeichnet und SOLL nichts bewirken.",
    "importNotes.text": "Reines Notizfeld (was der Import nicht abbilden "
                        "konnte) — es wird nirgends gezeichnet.",
    "camera3d.fogColor":
        "Der Nebel DAEMPFT Sprites nur, er faerbt sie nicht — so steht es am "
        "Feld (`fogColor`: '0x00RRGGBB (Sprites daempfen nur)'). Unser Zeuge "
        "ist ein SuperScope 3D, also ausschliesslich Sprites: die Farbe kann "
        "dort per Entwurf nichts bewirken, nur `fogStart`/`fogEnd` wirken. Ein "
        "STUMM waere eine Falschaussage ueber die App.",
    "rotatingStars.bandHi":
        "Die OBERE Grenze des ausgewerteten Spektralbereichs. Der Knoten nimmt "
        "aus dem Fenster nur die SPITZE (`peak = max(spec[lo..hi))`), und das "
        "Spektrum des Standalone-Testsignals faellt ab jeder Stelle monoton — "
        "das Maximum liegt also immer im ERSTEN Band des Fensters, unabhaengig "
        "davon, wo es endet. Gemessen mit sechs Fenstern ([0,·), [3,·), [4,·), "
        "[12,·), [40,·), [120,·)): jedes schmalste Fenster gab Pixel fuer Pixel "
        "dasselbe Bild wie das weite (S57). Dass die Grenzen ankommen, zeigt "
        "`bandLo` — fuenf einzelne Baender liefern fuenf verschiedene Bilder. "
        "Messbar waere `bandHi` nur mit einem Signal, dessen Spektrum irgendwo "
        "STEIGT.",
    "rotatingStars.bandLo":
        "Seit dem exakten r_rotstar-Port (S60) zaehlt ein Band nur noch als "
        "LOKALER Peak — es muss BEIDE Nachbarn um mehr als 4 uebersteigen "
        "(signed char). Das monoton fallende Standalone-Spektrum hat keinen "
        "einzigen solchen Peak, s ist in jedem Frame und jedem Fenster 0, und "
        "jede Fensterlage liefert dasselbe Bild. (Vor S60 nahm der Port das "
        "blanke Maximum — da zeigte `bandLo` fuenf verschiedene Bilder; diese "
        "Messung belegte, dass die Grenzen ankommen.) Messbar nur mit einem "
        "Signal, das einen lokalen Spektral-Peak > 4 traegt.",
    "rotatingStars.audioGain":
        "Skaliert den Audio-Anteil der Sterngroesse — also `s/255` aus der "
        "Lokal-Peak-Suche (s. `bandLo`). Auf dem Standalone-Signal ist s "
        "IMMER 0, der Faktor multipliziert eine Null und kein Wert kann etwas "
        "aendern. Dass die Groessenformel selbst wirkt, zeigt `baseRadius` "
        "(WIRKT); referenz-belegt ist der Audio-Zweig ueber die pixelgenaue "
        "Matrix-Zeile 13 gegen AvsRef (S60: Menge 0,00 / Deckung 1,00).",
    "customBpm.arbitraryMs":
        "Der Frei-Takt haengt an der WANDUHR (`steadyNowMs`), nicht am Frame. "
        "Ein Sondenlauf rendert 181 Frames in Millisekunden — in dieser Zeit "
        "loest weder die Vorgabe 500 ms noch der Gegenwert 5000 ms einen Takt "
        "aus, beide Bilder sind zwangslaeufig gleich. Messbar waere das nur "
        "mit einer Uhr, die der Sondenlauf stellt.",
    **{f"hostgroup.{f}":
       "Ein- und Ausgangskurve des Gruppenwechsels. Implementiert ist bisher "
       "NUR linear (der Header sagt es: '0 = linear, weitere Kurven mit HG2') "
       "— es gibt also keinen zweiten Wert, der etwas anderes taete. Und "
       "gemessen wird ein einzelner Knoten, nicht ein Gruppenwechsel."
       for f in ("curveIn", "curveOut")},
    **{f"{t}.filename":
       "Herkunftsnotiz aus dem Preset. Das Bild selbst traegt `imageData`, "
       "geladen wird nie von diesem Pfad — das Feld kann also nichts bewirken."
       for t in ("picture", "pictureII", "texer", "texerII")},
    "milkdrop.preset":
        "Der uebersetzte .milk-Inhalt selbst (ganzes Dokument samt HLSL). Ein "
        "'Gegenwert' waere ein zweites vollstaendiges MilkDrop-Preset — das "
        "misst dann nicht mehr das Feld, sondern zwei verschiedene Presets.",
    "milkdrop.presetDir":
        "Suchbasis fuer Texturen. Unsere Test-Presets laden keine, also kann "
        "der Pfad nichts bewirken; mit Texturen waere es ein Datei-Test, kein "
        "Feld-Test.",
    # (`timescope.channel`/`useChannel` standen bis S55 hier: der Standalone
    # fuellte beide Spektrumkanaele gleich, links/rechts/Mitte waren also
    # zwangslaeufig identisch. Seit `--stereo-spektrum` sind sie messbar —
    # s. ARGS_JE_FELD im Runner. Echtes Material aus TestAudio braucht es
    # nicht, im Gegenteil: synthetisch bleibt deterministisch.)
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
# Ein gefuellter Puffer: die Slot- und Puffer-Regler von Effect List und Buffer
# Blend waehlen zwischen Speicherplaetzen. Sind alle leer, mischen sie samt und
# sonders gegen Schwarz, und die Wahl ist gleichgueltig (S56).
# Der Puffer muss etwas ANDERES enthalten als das aktuelle Bild — sonst waehlt
# ein Slot-Regler zwischen zwei gleichen Bildern (S56). Also: invertieren,
# speichern, zurueck-invertieren.
_PUFFER_FUELLEN = [L.node("invert", "kurz invertiert"),
                   L.node("bufferSave", "Puffer 0 fuellen", slot=0, dir=0),
                   L.node("invert", "und zurueck")]

VORLAUF: dict[str, list[dict]] = {
    "videoDelay": [BEWEGT],
    "multiDelay": [BEWEGT],
    "list": _PUFFER_FUELLEN,
    "bufferBlend": _PUFFER_FUELLEN,
}

# KINDER: Container zeichnen selbst nichts. Ihre Blend- und Pufferparameter
# regeln, WIE das Ergebnis der Kinder ins Bild kommt — ohne Kind gibt es kein
# Ergebnis und jeder ihrer Regler ist wirkungslos (S54: `list` 12 von 14 stumm).
KINDER: dict[str, list[dict]] = {
    # Das Kind der Effect List PULSIERT, es steht nicht still. Grund ist
    # derselbe wie beim Vorlauf der Verzoegerungsspeicher: ein Kind, das jeden
    # Frame dasselbe zeichnet, kann keine ZEITABHAENGIGE Eigenschaft der Liste
    # zeigen. `onBeatFrames` ist die LAENGE des Beat-Fensters — bei Vorgabe 1
    # blendet die Liste nach dem Beat einmal, bei 200 noch zweihundertmal. Mit
    # einem stehenden Kind sind das dieselben Pixel, und die Sonde meldete
    # STUMM, obwohl die Lauflaenge (186, s. Runner) laengst hinter dem Beat
    # endete (S56/S57).
    "list": [L.node("superScope", "Kind",
                    pointCode="x=cos(i*6.28)*(0.25+0.25*sin(time*4)); "
                              "y=sin(i*6.28)*(0.25+0.25*sin(time*4)); "
                              "red=1; green=0.2; blue=0.2",
                    pointCount=128, colors=[0xFFFFFF])],
    "hostgroup": [L.node("superScope", "Kind",
                         pointCode="x=cos(i*6.28)*0.5; y=sin(i*6.28)*0.5; "
                                   "red=0.2; green=1; blue=0.2",
                         pointCount=128, colors=[0xFFFFFF])],
}

# VORLAUF JE FELD: dieselbe Sache wie VORLAUF, aber fuer EIN Feld statt einen
# ganzen Typ. Es gibt Felder, die einen anderen Bildinhalt brauchen als ihre
# Geschwister — dann darf nicht der ganze Typ umgebaut werden, sonst aendern
# sich die Messwerte aller anderen Sonden mit.
#
# `convolution.edgeMode` waehlt, was der Kern JENSEITS des Bildrandes liest
# (festklemmen oder umlaufen). Der Untergrund ist am Rand ueberall 0x101010 —
# Balken und Diagonale ruehren ihn nicht an. Bei einfarbigem Rand liefern beide
# Arten dasselbe, und zwar exakt: die Sonde stand mit MAE 0,0000 als „stumm" da
# (S55/S56). Zwei breite Diagonalen von Ecke zu Ecke machen den Rand ungleich,
# dann trennt das Feld (S57: MAE 0,0014).
_RANDVOLL = [
    L.node("superScope", "Rand A", initCode="n=2",
           pointCode="x=i*2-1; y=i*2-1; red=1; green=0.2; blue=0",
           pointCount=2, renderMode=1, lineWidth=60, colors=[0xFFFFFF],
           spectrumSource=False, colorCycleFrames=0),
    L.node("superScope", "Rand B", initCode="n=2",
           pointCode="x=i*2-1; y=1-i*2; red=0; green=0.6; blue=1",
           pointCount=2, renderMode=1, lineWidth=60, colors=[0xFFFFFF],
           spectrumSource=False, colorCycleFrames=0),
]

VORLAUF_JE_FELD: dict[str, list[dict]] = {
    "convolution.edgeMode": _RANDVOLL,
}

# UNTERGRUND JE FELD: wie UNTERGRUND_JE_TYP, aber fuer EIN Feld.
#
# `bloom.post` entscheidet, WO der Glow entsteht: beim Present (Vorgabe, die
# Kette bleibt unberuehrt) oder in der Kette selbst. Beide Wege erzeugen
# denselben Glow aus derselben Quelle — der Unterschied lebt allein davon, dass
# der naechste Frame ihn sieht. Auf einem Untergrund, der jeden Frame loescht,
# gibt es kein „naechster Frame sieht ihn", und die Sonde meldete STUMM
# (S55/S56). Mit `onlyFirst` klingt der Unterschied auf (S57: MAE 0,7997).
#
# Nur fuer dieses eine Feld: mit Rueckkopplung saettigt der additive Glow ueber
# 181 Frames (S48-Befund), und auf einem gesaettigten Bild koennten `intensity`
# und `radius` ihrerseits nichts mehr zeigen.
UNTERGRUND_JE_FELD: dict[str, bool] = {
    "bloom.post": True,
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
        # Eine Tafel mit FESTER Laenge wird an allen Stellen gelesen. Zwei
        # Farben fuellten nur die ersten zwei — und Dot Plane las trotzdem alle
        # fuenf (`params.colors[t]`, t bis 4). Schlimmer: die zweite Farbe traf
        # die Vorgabe (`0x00FFFF` steht dort an Stelle 1), also aenderte sich
        # genau EINE Stuetzstelle von fuenf, und die lag im Segment, das das
        # Testsignal nicht trifft. Die Sonde meldete STUMM fuer ein Feld, das
        # sehr wohl ankommt (S57: mit fuenf Farben MAE 0,0160).
        #
        # Deshalb: so viele Stellen wie die Tafel hat, und an jeder Stelle ein
        # anderer Wert als die Vorgabe dort. Die Palette ist bewusst
        # hochgesaettigt — sie muss sich gegen JEDE uebliche Vorgabe abheben.
        #
        # WEISS und Graustufen gehoeren NICHT hinein, auch nicht als
        # Ausweichfarbe: `0xFFFFFF` ist der Ersatzwert, den mehrere Renderer
        # fuer eine LEERE Tafel einsetzen (`cycleScopeColor`, `paletteRgb`).
        # Eine Tafel, die irgendwo Weiss enthaelt, kann dort also genau das
        # Bild der Vorgabe treffen — in der ersten Fassung dieser Regel fiel
        # `superScope.colors` (Vorgabe: leer) deshalb von WIRKT auf STUMM, und
        # drei weitere Tafeln wurden schwaecher.
        PALETTE = [0xFF00FF, 0x00FF00, 0x0000FF, 0xFFFF00,
                   0xFF0000, 0x00FFFF, 0xFF8000, 0x8000FF]
        laenge = int(feld.get("laenge") or 0)
        stellen = laenge if laenge >= 2 and len(vorgabe or []) == laenge else 2
        tafel: list[int] = []
        for i in range(stellen):
            alt = vorgabe[i] if isinstance(vorgabe, list) and i < len(vorgabe) else None
            # Die erste Palettenfarbe, die an DIESER Stelle etwas aendert.
            tafel.append(next(c for c in PALETTE[i:] + PALETTE if c != alt))
        return tafel

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

    # DIESELBE Falle eine Ebene tiefer: ein Feldname, den es nicht gibt, landet
    # als zusaetzlicher JSON-Schluessel im Knoten und wird beim Laden schlicht
    # ignoriert — die Sonde misst dann zwei gleiche Bilder und meldet STUMM.
    # Bis S56 stand in GRUNDKONFIG `{"type": 1}` fuer die Julia-Saat; `type` ist
    # im Knoten-JSON aber der KNOTENTYP (der Fraktal-Typ heisst `ftype`). Der
    # Eintrag machte aus dem Knoten den unbekannten Typ "1" -> Passthrough ->
    # acht Sonden stumm, zwei Vollaufe lang.
    # Getrennt nach SCHADENSWIRKUNG, nicht nach Herkunft:
    #
    #   Zusatzfeld  (der innere Schluessel einer GRUNDKONFIG) landet als JSON im
    #               erzeugten Knoten. Ein falscher Name richtet dort Schaden an
    #               — `type` traf den KNOTENTYP. FEHLER.
    #   Feldname    (der aeussere Schluessel) waehlt nur aus, WANN eine Tabelle
    #               greift. Zeigt er ins Leere, wird der Eintrag nie
    #               nachgeschlagen; die Kreuzprodukte unten erzeugen solche
    #               Kombinationen absichtlich ("dieses Feld, falls der Typ es
    #               hat"). Nur ein HINWEIS mit Zahl.
    # `enabled`/`name`/`description` sind RAHMEN-Felder des Knotens: gueltige
    # JSON-Schluessel, die `fieldNames()` bewusst herausnimmt. Eine
    # Grundkonfiguration darf sie setzen (S56: das Beat-Fenster der Effect List
    # greift laut r_list NUR bei einer statisch deaktivierten Liste).
    RAHMEN = {"enabled", "name", "description"}
    felder_je_typ = {t["typkey"]: {f["name"] for f in t["felder"]} | RAHMEN
                     for t in docs["typen"]}
    schlecht: list[str] = []
    for voll, extra in GRUNDKONFIG.items():
        typkey = voll.split(".")[0]
        if typkey not in felder_je_typ:
            schlecht.append(f"GRUNDKONFIG {voll}: unbekannter Typ")
            continue
        for schluessel in extra:
            if schluessel not in felder_je_typ[typkey]:
                schlecht.append(f"GRUNDKONFIG {voll}: Zusatzfeld "
                                f"'{schluessel}' gibt es bei {typkey} nicht")
    if schlecht:
        print("FEHLER: eine Grundkonfiguration schreibt ein Feld, das es nicht "
              "gibt — es landet im Preset und wird still ignoriert:")
        for x in schlecht:
            print("   ", x)
        return 2

    tot = sorted(voll for tabelle in (HANDWERK, GRUNDKONFIG, NICHT_PRUEFBAR)
                 for voll in tabelle
                 if voll.split(".")[0] in felder_je_typ
                 and voll.split(".", 1)[1] not in felder_je_typ[voll.split(".")[0]])

    gebaut = uebersprungen = mit_grund = 0
    verwaist: list[str] = []
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
        grundbild = L.untergrund(UNTERGRUND_JE_TYP.get(typkey, False))
        # Was dieser Lauf NICHT mehr erzeugt, muss weg. Sonst laeuft eine Sonde
        # zu einem entfernten Feld weiter mit und wird weiter beurteilt —
        # `hostgroup.curveIn`/`curveOut` standen so noch als „stumm" im Report,
        # obwohl sie laengst als nicht pruefbar erklaert waren (Befund S56).
        erzeugt: set[str] = set()
        L.write(args.out / typkey / "_default.lvfx",
                L.chain(*grundbild, *vorlauf, pruefling, *nachfolger))
        erzeugt.add("_default.lvfx")

        for f in t["felder"]:
            voll = f"{typkey}.{f['name']}"
            # ZUERST fragen, ob das Feld ueberhaupt prueefbar ist — sonst
            # landet ein Feld, das per Entwurf nichts bewirkt, in der
            # Restliste „HANDWERK ergaenzen" und sieht dort wie offene Arbeit
            # aus. Neun Felder standen aus genau diesem Grund jahrelang… nun,
            # seit S54 dort (S55).
            if voll in NICHT_PRUEFBAR:
                nicht_pruefbar.append(voll)
                continue
            wert = gegenwert(f, typkey, alle)
            if wert is None:
                uebersprungen += 1
                offen.append(f"{typkey}.{f['name']} ({f['art']})")
                continue
            grund = GRUNDKONFIG.get(voll, {})
            # Aufbau JE FELD: ein anderer Vorlauf oder ein anderer Untergrund
            # als beim Rest des Typs. Beides erzwingt einen eigenen
            # Vergleichsgrund — sonst haelte der Runner die Sonde gegen
            # `_default`, und der Vergleich traege ZWEI Unterschiede.
            f_vorlauf = vorlauf + VORLAUF_JE_FELD.get(voll, [])
            f_grundbild = (L.untergrund(True) if UNTERGRUND_JE_FELD.get(voll, False)
                           else grundbild)
            eigener_grund = (bool(grund) or voll in VORLAUF_JE_FELD
                             or voll in UNTERGRUND_JE_FELD)
            knoten = L.node(typkey, t["name"],
                            **({"children": kinder} if kinder else {}),
                            **{**grund, f["name"]: wert})
            L.write(args.out / typkey / f"{f['name']}.lvfx",
                    L.chain(*f_grundbild, *f_vorlauf, knoten, *nachfolger))
            erzeugt.add(f"{f['name']}.lvfx")
            if eigener_grund:
                # Eigener Vergleichsgrund: der Nachbar ist auch hier gesetzt,
                # sonst misst das Paar ZWEI Unterschiede statt einem. Aus
                # demselben Grund traegt er denselben Vorlauf und denselben
                # Untergrund wie die Sonde.
                erzeugt.add(f"_grund_{f['name']}.lvfx")
                L.write(args.out / typkey / f"_grund_{f['name']}.lvfx",
                        L.chain(*f_grundbild, *f_vorlauf,
                                L.node(typkey, t["name"],
                                       **({"children": kinder} if kinder else {}),
                                       **grund),
                                *nachfolger))
                mit_grund += 1
            gebaut += 1
        for alt_lvfx in sorted((args.out / typkey).glob("*.lvfx")):
            if alt_lvfx.name not in erzeugt:
                alt_lvfx.unlink()
                verwaist.append(f"{typkey}/{alt_lvfx.name}")

    if verwaist:
        print(f"Verwaiste Sonden geloescht: {len(verwaist)} "
              f"({', '.join(verwaist[:6])}{' …' if len(verwaist) > 6 else ''})")
    if tot:
        print(f"Hinweis: {len(tot)} Tabelleneintraege zeigen auf ein Feld, "
              f"das ihr Typ nicht hat (folgenlos, aber Rauschen)")
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

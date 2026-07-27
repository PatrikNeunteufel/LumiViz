# -*- coding: utf-8 -*-
"""Modul-Sonden (S50): jedes Modul EINZELN gegen AvsRef, dann schrittweise
zusammengebaut.

Ergaenzt die Modul-Matrix, die nur Builtins mit DEFAULT-Parametern prueft.
Keiner der fuenf Befunde aus Session 50 lag dort: sie steckten in APEs
(Texer II), in skriptgesteuerten Varianten (linesize je Punkt, n=0) und in der
KOPPLUNG zweier Module (reg-Register, Listen-EEL). Genau diese drei Klassen
deckt diese Datei ab.

Aufbau in Stufen — bewusst so, dass ein Befund IMMER genau einem Modul
zuzuordnen ist:
  1_render/  ein Zeichner auf definiertem Grund, sonst nichts
  2_trans/   ein Transformator auf einem KLAREN Referenzbild, sonst nichts
  3_script/  dasselbe Modul, aber die skriptbaren Groessen variiert
  4_kopplung/ zwei Module, die nur ueber reg/Puffer/Listen-EEL zusammenhaengen
  5_vars/    PAAR-Sonden: welche Variablen stellt der Host dem Skript bereit?

Stufe 5 arbeitet anders als die uebrigen: je zwei Sonden rechnen dasselbe
Ergebnis, die eine ueber eine Host-Variable (w, h), die andere ueber das
Literal, das bei 320x240 herauskommt. Stimmen die REFERENZbilder des Paares
ueberein, ist nicht nur die Existenz der Variablen belegt, sondern ihr WERT.
Deshalb sind diese Sonden an die Groesse 320x240 gebunden — die
"immer zwei Groessen"-Regel gilt hier ausdruecklich NICHT.

Urteil: NICHT ueber dMean allein. Duenne Vordergruende bewegen den Mittelwert
nicht — in Session 50 meldete die Leiter viermal "OK", waehrend wir sichtbar
nichts zeichneten. run_module_probes.py vergleicht deshalb zusaetzlich die
Zahl gezeichneter Pixel und deren Schwerpunkt.
"""
from pathlib import Path

from avs_preset_lib import (blur, clear_screen, colorfade, convolution,
                            dynamic_movement, effect_list, effect_list_ex,
                            invert, list_config, movement_user, preset,
                            set_render_mode, superscope, texer2, triangle)

ROOT = Path(__file__).parent
OUT = ROOT / "probes"

# --------------------------------------------------------------- Referenzbild
# Deterministisch, ohne rand und ohne Audio: vier Farbfelder + Diagonale.
# Die Farbfelder trennen Kanal- und Spiegelfehler, die Diagonale zeigt Warps.
GRUND = clear_screen(color=0x101010)
FELDER = set_render_mode(0, width=1) + b"".join(
    superscope(
        point=(f"x={x0}+i*0.3; y={y0};"
               f"red={r};green={g};blue={b};linesize=18;"),
        init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)
    for x0, y0, r, g, b in (
        (-0.9, -0.6, 1, 0, 0), (-0.9, -0.2, 0, 1, 0),
        (-0.9, 0.2, 0, 0, 1), (-0.9, 0.6, 1, 1, 1)))
DIAGONALE = superscope(point="x=i*1.8-0.9; y=i*1.6-0.8; red=1;green=1;blue=0;linesize=2;",
                       init="n=40", which_ch=2, colors=(0xFFFFFF,), drawmode=1)
REFBILD = GRUND + FELDER + DIAGONALE


def write(rel: str, data: bytes) -> None:
    p = OUT / rel
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_bytes(data)


def main() -> None:
    # ---------------------------------------------------- 1: Zeichner einzeln
    write("1_render/texer2_default.avs", preset(
        GRUND, texer2(point="x=(i-0.5)*1.6; y=sin(i*6.28)*0.5;", init="n=12")))
    write("1_render/texer2_size.avs", preset(
        GRUND, texer2(point="x=(i-0.5)*1.6; y=0; sizex=0.5+i*3; sizey=0.5+i*3;",
                      init="n=6")))
    write("1_render/texer2_n0.avs", preset(
        GRUND, texer2(point="x=0;y=0;sizex=3;sizey=3;", init="n=0")))
    write("1_render/texer2_skip.avs", preset(
        GRUND, texer2(point="x=(i-0.5)*1.6; y=0; skip=below(i,0.5);", init="n=10")))
    write("1_render/scope_linien.avs", preset(
        GRUND, set_render_mode(0, width=2),
        superscope(point="x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7; red=1;green=1;blue=1;",
                   init="n=64", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("1_render/scope_punkte.avs", preset(
        GRUND, set_render_mode(0, width=2),
        superscope(point="x=cos(i*6.28)*0.7; y=sin(i*6.28)*0.7; red=1;green=1;blue=1;",
                   init="n=32", which_ch=2, colors=(0xFFFFFF,), drawmode=0)))

    # -------------------------------------- 2: Transformatoren auf Referenzbild
    write("2_trans/refbild.avs", preset(REFBILD))            # Nullprobe
    write("2_trans/movement_zoom.avs", preset(REFBILD, movement_user("d=d*0.8")))
    write("2_trans/dmove_zoom.avs", preset(
        REFBILD, dynamic_movement(point="d=d*0.8", rectcoords=0, xres=16, yres=16)))
    write("2_trans/blur.avs", preset(REFBILD, blur()))
    write("2_trans/invert.avs", preset(REFBILD, invert()))
    write("2_trans/convolution_id.avs", preset(REFBILD, convolution()))
    write("2_trans/convolution_blur.avs", preset(
        REFBILD, convolution(kernel=[1] * 49, scale=49)))
    # Die Dynamic Movement aus "Alien Alloy" VERBATIM auf dem Referenzbild
    # (S51). Im Preset transportiert sie den Inhalt von den Raendern ueber die
    # ganze Flaeche; bei uns blieb er am Rand kleben, waehrend die
    # Standard-Sonde dmove_zoom gruen ist. Unterschied: Swirl in Polarform mit
    # Seitenverhaeltnis, wrap=1, Gitter 25x25.
    AA_FRAME = ("vol=vol*0.9+getspec(0.5,1,0);"
                "swrlstr=0.02+min(1.5,vol)*0.08;"
                "sftstr=0.02+min(1.5,vol)*0.08;"
                "t=t-(0.005+vol*0.015);"
                "cx=cos(t*.97)*.5;"
                "cy=sin(t*.77)*cos(t*1.03)*.5;"
                "ro=ro+sin(t*0.71)*cos(t*0.453)*sin(cos(t*.391))*.1;"
                "xo=cos(ro)*.3;yo=sin(ro)*.3;"
                "asp1=h/w;asp2=1/asp1")
    AA_POINT = ("xx=x;yy=y;"
                "x1=x-cx-xo;y1=(y-cy-yo)*asp1;"
                "r1=atan2(y1,x1);d1=sqrt(sqr(x1)+sqr(y1));"
                "r1=r1+swrlstr/d1;x1=cos(r1)*d1+cx+cos(ro)*sftstr;"
                "y1=sin(r1)*d1*asp2+cy+sin(ro)*sftstr;"
                "x2=x-cx+xo;y2=(y-cy+yo)*asp1;"
                "r2=atan2(y2,x2);d2=sqrt(sqr(x2)+sqr(y2));"
                "r2=r2-swrlstr/d2;x2=cos(r2)*d2+cx-cos(ro)*sftstr;"
                "y2=sin(r2)*d2*asp2+cy-sin(ro)*sftstr;"
                "x=(x1+x2)*.506;y=(y1+y2)*.506;")
    write("2_trans/dmove_alienalloy.avs", preset(
        REFBILD, dynamic_movement(point=AA_POINT, frame=AA_FRAME,
                                  init="t=rand(100)/50;", rectcoords=1,
                                  xres=25, yres=25, wrap=1)))
    # Dieselbe DM, aber gesaet wie im Preset: EINE Linie am unteren Rand, sonst
    # nichts, Clear nur im ersten Frame. Jetzt entscheidet allein der Transport,
    # wie weit der Inhalt ins Bild wandert — die Fassung auf dem Referenzbild
    # sagt darueber nichts, weil sie jeden Frame die ganze Flaeche neu saet.
    write("2_trans/dmove_transport_rand.avs", preset(
        clear_screen(color=0x000000, onlyfirst=1),
        dynamic_movement(point=AA_POINT, frame=AA_FRAME,
                         init="t=rand(100)/50;", rectcoords=1,
                         xres=25, yres=25, wrap=1),
        set_render_mode(0, width=6),
        superscope(point="x=i*1.9-0.95; y=0.95; red=1;green=1;blue=1;",
                   init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))

    # Derselbe Swirl, aber STATISCH: feste Staerke, kein Zeitlauf, keine
    # Verschiebung, kein Audio. Damit ist die Gesamtdrehung nach N Frames eine
    # reine Funktion der Warp-Mathematik — weicht sie ab, liegt es nicht am
    # Zustand des Skripts, sondern am Gitter oder der Abtastung.
    # Wie dmove_transport_rand, aber t FEST statt t=rand(100)/50: trennt den
    # Startwert des Skripts von der Warp-Mathematik. Weicht diese Fassung nicht
    # ab, liegt der ganze Unterschied im rand-Zieher des Init-Slots.
    write("2_trans/dmove_transport_fix.avs", preset(
        clear_screen(color=0x000000, onlyfirst=1),
        dynamic_movement(point=AA_POINT, frame=AA_FRAME, init="t=1;",
                         rectcoords=1, xres=25, yres=25, wrap=1),
        set_render_mode(0, width=6),
        superscope(point="x=i*1.9-0.95; y=0.95; red=1;green=1;blue=1;",
                   init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))

    write("2_trans/dmove_swirl_statisch.avs", preset(
        clear_screen(color=0x000000, onlyfirst=1),
        dynamic_movement(point=("x1=x;y1=y*asp1;"
                                "r1=atan2(y1,x1);d1=sqrt(sqr(x1)+sqr(y1));"
                                "r1=r1+0.1/max(d1,0.05);"
                                "x=cos(r1)*d1;y=sin(r1)*d1*asp2;"),
                         frame="asp1=h/w;asp2=1/asp1",
                         rectcoords=1, xres=25, yres=25, wrap=1),
        set_render_mode(0, width=6),
        superscope(point="x=i*1.9-0.95; y=0.95; red=1;green=1;blue=1;",
                   init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))

    # Die Luecke zwischen dem exakten statischen Swirl und der abweichenden
    # Alien-Alloy-Fassung in drei Schritten zubauen — erste divergente Stufe =
    # Taeter. Alle Groessen konstant, damit nur die Formel variiert.
    SEED = (clear_screen(color=0x000000, onlyfirst=1),)
    LINIE = (set_render_mode(0, width=6),
             superscope(point="x=i*1.9-0.95; y=0.95; red=1;green=1;blue=1;",
                        init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1))
    def stufe(name: str, point: str, frame: str = "asp1=h/w;asp2=1/asp1;s=0.1") -> None:
        write(f"2_trans/dmove_stufe_{name}.avs", preset(
            *SEED,
            dynamic_movement(point=point, frame=frame, rectcoords=1,
                             xres=25, yres=25, wrap=1),
            *LINIE))

    # Stufe A: GEGENLAEUFIGES Paar um denselben Mittelpunkt, gemittelt mit *.506
    # (prueft zugleich das Zahlenliteral ohne fuehrende Null).
    stufe("a_paar",
          "x1=x;y1=y*asp1;r1=atan2(y1,x1);d1=sqrt(sqr(x1)+sqr(y1));"
          "r1=r1+s/max(d1,0.05);x1=cos(r1)*d1;y1=sin(r1)*d1*asp2;"
          "x2=x;y2=y*asp1;r2=atan2(y2,x2);d2=sqrt(sqr(x2)+sqr(y2));"
          "r2=r2-s/max(d2,0.05);x2=cos(r2)*d2;y2=sin(r2)*d2*asp2;"
          "x=(x1+x2)*.506;y=(y1+y2)*.506;")
    # Stufe B: zwei VERSCHIEDENE Mittelpunkte (xo konstant), sonst wie A.
    stufe("b_zwei_zentren",
          "x1=x-xo;y1=y*asp1;r1=atan2(y1,x1);d1=sqrt(sqr(x1)+sqr(y1));"
          "r1=r1+s/max(d1,0.05);x1=cos(r1)*d1+xo;y1=sin(r1)*d1*asp2;"
          "x2=x+xo;y2=y*asp1;r2=atan2(y2,x2);d2=sqrt(sqr(x2)+sqr(y2));"
          "r2=r2-s/max(d2,0.05);x2=cos(r2)*d2-xo;y2=sin(r2)*d2*asp2;"
          "x=(x1+x2)*.506;y=(y1+y2)*.506;",
          frame="asp1=h/w;asp2=1/asp1;s=0.1;xo=0.3")
    # Stufe C: wie B, aber s/d OHNE max()-Schutz — Division durch 0 im Zentrum.
    stufe("c_ohne_schutz",
          "x1=x-xo;y1=y*asp1;r1=atan2(y1,x1);d1=sqrt(sqr(x1)+sqr(y1));"
          "r1=r1+s/d1;x1=cos(r1)*d1+xo;y1=sin(r1)*d1*asp2;"
          "x2=x+xo;y2=y*asp1;r2=atan2(y2,x2);d2=sqrt(sqr(x2)+sqr(y2));"
          "r2=r2-s/d2;x2=cos(r2)*d2-xo;y2=sin(r2)*d2*asp2;"
          "x=(x1+x2)*.506;y=(y1+y2)*.506;",
          frame="asp1=h/w;asp2=1/asp1;s=0.1;xo=0.3")

    # Der akkumulierende Zustand des Alien-Alloy-Frame-Codes als Zeilenlage:
    # die DM rechnet (Punkt-Code Identitaet, sie warpt also nicht) und legt
    # t, vol und ro in reg ab, ein Scope zeichnet sie. Damit ist ablesbar, ob
    # der Zustand ueber die Frames gleich laeuft — die Formeln selbst sind ab
    # Stufe C bewiesen.
    write("4_kopplung/dmove_zustand.avs", preset(
        GRUND,
        dynamic_movement(point="x=x;y=y;", frame=AA_FRAME + ";reg00=t*0.5;"
                                                 "reg01=vol*0.5;reg02=ro*0.5",
                         init="t=1;", rectcoords=1, xres=8, yres=8),
        set_render_mode(0, width=1),
        *[superscope(point=f"x={x0}+i*0.5; y=reg{k:02d};"
                           "red=1;green=1;blue=1;linesize=3;",
                     init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)
          for k, x0 in ((0, -0.9), (1, -0.25), (2, 0.4))]))

    # Colorfade mit ASYMMETRISCHEN Werten wie in "Alien Alloy" (+16/+11/-25).
    # Die Matrix prueft nur die Vorgabe (8/-8/-8). Verdacht (S51): in der
    # Referenz hebt Colorfade auch fast-schwarze Pixel ueber die Schwelle und
    # erzeugt so die Flaeche, die uns fehlt (Referenz 75126 gezeichnete Pixel
    # gegen unsere 4685 bei gleicher Spitzenhelligkeit).
    write("2_trans/colorfade_asymmetrisch.avs", preset(
        REFBILD, colorfade(r=16, g=11, b=-25)))
    # Dasselbe im operativen Bereich: fast schwarzer Grund, wenige helle Punkte.
    write("2_trans/colorfade_fastschwarz.avs", preset(
        clear_screen(color=0x080808, onlyfirst=1),
        set_render_mode(0, width=4),
        superscope(point="x=i*1.6-0.8;y=0;red=1;green=1;blue=1;", init="n=2",
                   which_ch=2, colors=(0xFFFFFF,), drawmode=0),
        colorfade(r=16, g=11, b=-25)))

    write("2_trans/convolution_kante.avs", preset(
        REFBILD, convolution(kernel=[0] * 24 + [8] + [0] * 24, scale=1,
                             edge_mode=1)))

    # ------------------------------------------ 3: skriptbare Groessen variiert
    write("3_script/linesize_keil.avs", preset(
        GRUND, set_render_mode(0, width=1),
        superscope(point="x=i*1.8-0.9;y=0;linesize=1+i*10;red=1;green=1;blue=1;",
                   init="n=40", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("3_script/drawmode_wechsel.avs", preset(
        GRUND, set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=sin(i*9)*0.5;drawmode=below(i,0.5);"
                         "red=1;green=1;blue=1;",
                   init="n=30", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("3_script/skip.avs", preset(
        GRUND, set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=0;skip=band(floor(i*8),1);"
                         "red=1;green=1;blue=1;",
                   init="n=40", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("3_script/n_aus_skript.avs", preset(
        GRUND, set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=i*0.4-0.2;red=1;green=1;blue=1;",
                   init="n=7", which_ch=2, colors=(0xFFFFFF,), drawmode=0)))

    # ------------------------------------------------------------ 4: Kopplung
    # reg: Dynamic Movement rechnet, SuperScope liest — das Muster, an dem in
    # "Mister Santa" der komplette Vordergrund haengt.
    write("4_kopplung/reg_dm_zu_scope.avs", preset(
        GRUND,
        dynamic_movement(point="", init="", frame="reg00=0.5; reg01=-0.4;",
                         rectcoords=1, xres=8, yres=8),
        set_render_mode(0, width=3),
        superscope(point="x=reg00; y=reg01+i*0.8; red=1;green=1;blue=1;",
                   init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    # reg in die Gegenrichtung: Scope schreibt, Texer liest.
    write("4_kopplung/reg_scope_zu_texer.avs", preset(
        GRUND, set_render_mode(0, width=1),
        superscope(point="reg05=0.6; x=-2; y=-2;", init="n=2",
                   which_ch=2, colors=(0xFFFFFF,), drawmode=1),
        texer2(point="x=reg05; y=(i-0.5)*1.2; sizex=2; sizey=2;", init="n=4")))
    # Einmal-Idiom: ausgeschaltete Liste schaltet sich per EEL selbst ein und
    # fuellt einen Puffer, den ein spaeterer Knoten liest.
    write("4_kopplung/liste_einmal.avs", preset(
        GRUND,
        effect_list_ex(list_config(init="lw=-1;lh=-1;",
                                   frame="enabled=bnot(equal(lw,w)*equal(lh,h));"
                                         "lw=w;lh=h;")
                       + clear_screen(color=0xFFFFFF),
                       enabled=False, blend_out=1),
        set_render_mode(0, width=3),
        superscope(point="x=i*1.8-0.9;y=0;red=1;green=0;blue=0;", init="n=2",
                   which_ch=2, colors=(0xFF0000,), drawmode=1)))
    # Liste mit Ausgabe-Blend Maximum ueber dem Referenzbild.
    write("4_kopplung/liste_blendout_max.avs", preset(
        REFBILD,
        effect_list(set_render_mode(0, width=20)
                    + superscope(point="x=i*1.4-0.7;y=0.4;red=0;green=0.6;blue=0;",
                                 init="n=2", which_ch=2, colors=(0xFFFFFF,),
                                 drawmode=1),
                    clear=True, blend_in=0, blend_out=3)))

    # ------------------------------------------- 5: Host-Variablen (Paar-Sonden)
    # Anlass (S51): "Alien Alloy" setzt seine Sprite-Zahl im FRAME-Slot auf
    # n=w*0.1 und die Sprite-Groesse auf reg00=h/280. Bekam der Texer-Host kein
    # w/h, war n=0 -> alle vier Texer zeichneten nichts und das Preset blieb
    # schwarz. Texer II und Triangle sind APEs ohne Quelltext, also messen wir.
    # Bei 320x240: w*0.1 = 32, h/120 = 2.
    TX_PT = "x=(i-0.5)*1.6; y=(i-0.5)*1.2;"
    write("5_vars/texer2_n_aus_w.avs", preset(
        GRUND, texer2(point=TX_PT, frame="n=w*0.1")))
    write("5_vars/texer2_n_literal.avs", preset(
        GRUND, texer2(point=TX_PT, frame="n=32")))
    TX_SZ = "x=(i-0.5)*1.4; y=0;"
    write("5_vars/texer2_size_aus_h.avs", preset(
        GRUND, texer2(point=TX_SZ, frame="n=6; sizex=h/120; sizey=h/120")))
    write("5_vars/texer2_size_literal.avs", preset(
        GRUND, texer2(point=TX_SZ, frame="n=6; sizex=2; sizey=2")))
    # n im INIT statt im Frame: trennt "Variable fehlt" von "Slot-Reihenfolge
    # falsch". Steht n im Init, braucht es w schon dort.
    write("5_vars/texer2_n_aus_w_init.avs", preset(
        GRUND, texer2(point=TX_PT, init="n=w*0.1")))
    # Nur im INIT gesetzte Groesse: haelt sie ueber die Frames? Entscheidet,
    # ob die neutrale Vorbelegung sizex=sizey=1 je Frame oder nur einmal beim
    # Init gehoert. Zeichnet die Referenz hier so gross wie bei size_literal,
    # darf sie NICHT je Frame zurueckgesetzt werden.
    write("5_vars/texer2_size_aus_init.avs", preset(
        GRUND, texer2(point=TX_SZ, init="n=6; sizex=2; sizey=2")))
    # --- Alien-Alloy-Restluecke (S52): fehlt MENGE, nicht Transport ----------
    # Der Preset-Text sagt, woran es haengen kann. Alle vier Texer rechnen
    #     n = w*0.1  und  sizex = reg00*0.75 ,
    # aber NUR der erste setzt  reg00 = h/280  — die anderen drei LESEN es.
    # `reg00..reg99` ist in AVS global (S50), also braucht es hier zweierlei:
    # dass reg00 von einem Texer zum naechsten traegt, und dass unser
    # sizex->Pixel-Vertrag bei genau diesem Wert stimmt (bei 240 Zeilen ist
    # reg00 = 0.857142 und damit sizex = 0.642857 — also KLEINER als 1, ein
    # Bereich, den die Paare oben mit sizex=2 nicht abdecken).
    ALLOY_PT = "x=(i-0.5)*1.6; y=0;"
    ALLOY_N = "n=8"
    # (a) Geber + Nehmer wie im Original: Texer 1 setzt reg00, Texer 2 liest es.
    #     Zeichnen beide gleich viel, traegt reg00 ueber die Knotengrenze.
    write("6_alloy/reg00_geber_nehmer.avs", preset(
        GRUND,
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; reg00=h/280; "
                                     "sizex=reg00*0.75; sizey=sizex"),
        texer2(point="x=(i-0.5)*1.6; y=0.5;",
               frame=f"{ALLOY_N}; sizex=reg00*0.75; sizey=sizex")))
    # (b) Dasselbe Bild, aber der zweite Texer rechnet OHNE reg00 — das Literal,
    #     das bei 240 Zeilen herauskommt. Stimmen die REFERENZbilder von (a) und
    #     (b) ueberein, ist reg00 nachweislich uebergetragen worden.
    write("6_alloy/reg00_literal.avs", preset(
        GRUND,
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; reg00=h/280; "
                                     "sizex=reg00*0.75; sizey=sizex"),
        texer2(point="x=(i-0.5)*1.6; y=0.5;",
               frame=f"{ALLOY_N}; sizex=0.642857; sizey=sizex")))
    # (c) Der sizex-Vertrag allein, ohne reg00: EIN Texer, Groesse als Literal.
    #     Trennt "reg00 kommt nicht an" von "sizex rechnet falsch in Pixel um".
    write("6_alloy/size_klein_literal.avs", preset(
        GRUND, texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; "
                                            "sizex=0.642857; sizey=sizex")))
    # (d) Gegenprobe mit sizex=1 (neutral) — die Bezugsgroesse fuer (c).
    #     Aus (c)/(d) faellt der Flaechenfaktor ab, den sizex bewirkt.
    write("6_alloy/size_eins.avs", preset(
        GRUND, texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    # (e) Sprite-Geometrie am RAND: das Original setzt y=0.98 (und -0.98), also
    #     knapp ausserhalb der halben Sprite-Hoehe. Wird dort geklemmt,
    #     abgeschnitten oder gar nicht gezeichnet? Drei Reihen zum Vergleich.
    write("6_alloy/rand_y098.avs", preset(
        GRUND,
        texer2(point="x=(i-0.5)*1.6; y=0.98;", frame=f"{ALLOY_N}; sizex=2; sizey=2"),
        texer2(point="x=(i-0.5)*1.6; y=-0.98;", frame=f"{ALLOY_N}; sizex=2; sizey=2"),
        texer2(point="x=(i-0.5)*1.6; y=0;", frame=f"{ALLOY_N}; sizex=2; sizey=2")))

    # (f) Der eigentliche Befund (S52): in "Alien Alloy" ist NICHT zu wenig
    #     gezeichnet — unser Bild ist SCHWARZ bis auf die Sprites, waehrend die
    #     Referenz das volle Wirbelfeld zeigt. Die Bisektion sagt es genau:
    #     Stufe l05 (bis einschliesslich Dynamic Movement) ist 0.000, l06
    #     (+ EIN Texer) kippt, und laesst man aus l06 die DM weg, ist es wieder
    #     gut. Also loescht ein Zeichner NACH einer Dynamic Movement den
    #     Hintergrund. Diese drei Sonden trennen "liegt am Texer" von "liegt an
    #     jedem Zeichner nach einer DM".
    DM_ZOOM = "d=d*0.98"
    write("6_alloy/dm_dann_texer.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM),
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    write("6_alloy/dm_dann_scope.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM),
        superscope(point="x=(i-0.5)*1.6; y=0; red=1;green=1;blue=1;linesize=4;",
                   init="n=8", which_ch=2, colors=(0xFFFFFF,), drawmode=1)))
    write("6_alloy/dm_allein.avs", preset(REFBILD, dynamic_movement(point=DM_ZOOM)))
    write("6_alloy/texer_ohne_dm.avs", preset(
        REFBILD, texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    # (g) Mit den FLAGS der echten DM aus "Alien Alloy": rectCoords=1, wrap=1,
    #     Gitter 25x25 (Vorgabe 16x12). Eine davon macht den Unterschied — die
    #     Sonde (f) mit den Vorgabewerten ist sauber, die Kette nicht.
    ALLOY_DM = dict(rectcoords=1, wrap=1, xres=25, yres=25, subpixel=1)
    write("6_alloy/dmflags_dann_texer.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM, **ALLOY_DM),
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    write("6_alloy/dmflags_allein.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM, **ALLOY_DM)))
    # Je Flag einzeln, damit der Taeter benannt ist statt nur eingekreist.
    write("6_alloy/dmrect_dann_texer.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM, rectcoords=1),
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    write("6_alloy/dmwrap_dann_texer.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM, wrap=1),
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    write("6_alloy/dmgitter_dann_texer.avs", preset(
        REFBILD, dynamic_movement(point=DM_ZOOM, xres=25, yres=25),
        texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))
    # (h) Der verbleibende Unterschied zur Kette: dort steht VOR den Texern ein
    #     Set Render Mode mit BLEND_LINE-Modus 2 (MAX). Die Sonden oben liefen
    #     alle im Vorgabe-Modus REPLACE. Da die Divergenz ueber die Frames
    #     WAECHST (Frame 1: 0.010, Frame 120: 0.568), muss der Zeichner etwas
    #     im Puffer hinterlassen, das die Ruecklese der DM im naechsten Frame
    #     mitnimmt — genau dort ist der Blend-Modus die Stellschraube. Ein Paar,
    #     das sich nur darin unterscheidet.
    for mode, tag in ((2, "max"), (0, "replace"), (1, "additiv")):
        write(f"6_alloy/srm{mode}_{tag}_dm_texer.avs", preset(
            REFBILD, set_render_mode(mode, width=1),
            dynamic_movement(point=DM_ZOOM, **ALLOY_DM),
            texer2(point=ALLOY_PT, frame=f"{ALLOY_N}; sizex=1; sizey=1")))

    # (i) Der Minimalfall aus der Bisektion: NUR die echte Dynamic Movement und
    #     der echte Texer aus "Alien Alloy" weichen schon ab (0.120 ueber 240
    #     Frames). Wichtig — und der Grund, warum die Sonden (f)-(h) sauber
    #     sind: die saeen mit REFBILD JEDEN Frame die ganze Flaeche neu und sind
    #     damit blind fuer Verluste in der Rueckkopplung (Merkregel S51). Hier
    #     sind die Sprites am Rand die EINZIGE Energiequelle, alles andere baut
    #     der Wirbel ueber die Frames auf. Vier Fassungen, die sich in genau
    #     einer Zutat unterscheiden.
    DM_INIT = "t=rand(100)/50;"
    DM_FRAME = (
        "vol=vol*0.9+getspec(0.5,1,0) ;\n"
        "swrlstr=0.02+min(1.5,vol)*0.08 ;\n"
        "sftstr=0.02+min(1.5,vol)*0.08 ;\n"
        "t=t-(0.005+vol*0.015) ;\n"
        "cx=cos(t*.97)*.5;\n"
        "cy=sin(t*.77)*cos(t*1.03)*.5;\n"
        "ro=ro+sin(t*0.71)*cos(t*0.453)*sin(cos(t*.391))*.1;\n"
        "xo=cos(ro)*.3;yo=sin(ro)*.3;\n"
        "asp1=h/w ;\nasp2=1/asp1")
    DM_POINT = (
        "xx=x;yy=y;\n"
        "x1=x-cx-xo;y1=(y-cy-yo)*asp1;\n"
        "r1=atan2(y1,x1);d1=sqrt(sqr(x1)+sqr(y1));\n"
        "r1=r1+swrlstr/d1;x1=cos(r1)*d1+cx+cos(ro)*sftstr;"
        "y1=sin(r1)*d1*asp2+cy+sin(ro)*sftstr;\n"
        "x2=x-cx+xo;y2=(y-cy+yo)*asp1;\n"
        "r2=atan2(y2,x2);d2=sqrt(sqr(x2)+sqr(y2));\n"
        "r2=r2-swrlstr/d2;x2=cos(r2)*d2+cx-cos(ro)*sftstr;"
        "y2=sin(r2)*d2*asp2+cy-sin(ro)*sftstr;\n"
        "x=(x1+x2)*.506;y=(y1+y2)*.506;")
    TX_FRAME = ("n=w*0.1 ;\nct=ct+cts ;\n"
                "red=sin(ct+2.07)*0.5+0.5 ;\ngreen=sin(ct+4.18)*0.5+0.5 ;\n"
                "blue=sin(ct)*0.5+0.5 ;\nreg00=h/280 ;\n"
                "sizex=reg00*0.75 ;\nsizey=sizex")
    TX_BEAT = "cts=(rand(51)-25)*0.01"
    TX_POINT = ("x=sin(i*$pi*2+ct*0.713)*2-1 ;\n"
                "y=0.98-sqr(getosc(v,0,2))*0.2")

    def alloy_paar(name: str, dm_init: str = DM_INIT, dm_frame: str = DM_FRAME,
                   tx_frame: str = TX_FRAME, tx_beat: str = TX_BEAT) -> None:
        write(f"6_alloy/{name}.avs", preset(
            dynamic_movement(point=DM_POINT, init=dm_init, frame=dm_frame,
                             **ALLOY_DM),
            texer2(point=TX_POINT, frame=tx_frame, beat=tx_beat)))

    # (j) Der Beweis, isoliert: dieselbe Farbe einmal im FRAME-, einmal im
    #     POINT-Slot gesetzt. Stimmen die REFERENZbilder ueberein, ueberlebt die
    #     Frame-Farbe die Punktschleife — dann darf unsere neutrale Vorbelegung
    #     nicht je Punkt laufen (Gegenstueck zu sizex/sizey aus S51).
    FARB_PT = "x=(i-0.5)*1.6; y=0;"
    write("6_alloy/texer_farbe_frame.avs", preset(
        GRUND, texer2(point=FARB_PT,
                      frame="n=8; sizex=2; sizey=2; red=1; green=0.25; blue=0")))
    write("6_alloy/texer_farbe_point.avs", preset(
        GRUND, texer2(point=FARB_PT + " red=1; green=0.25; blue=0;",
                      frame="n=8; sizex=2; sizey=2")))

    alloy_paar("paar_original")
    # Ohne den rand-Zieher im Init: startet der Wirbel bei beiden in derselben
    # Phase, faellt eine Zufalls-Ausrichtung als Ursache weg.
    alloy_paar("paar_fest_t", dm_init="t=1;")
    # Ohne Audio-Abhaengigkeit: `vol` treibt Wirbelstaerke UND Zeitschritt.
    alloy_paar("paar_fest_vol",
               dm_frame=DM_FRAME.replace("vol=vol*0.9+getspec(0.5,1,0) ;",
                                         "vol=0.5 ;"))
    # Ohne die Farb-/Zeitakkumulation des Texers (ct, cts und deren rand).
    alloy_paar("paar_fixfarbe",
               tx_frame=("n=w*0.1 ;\nred=1 ;\ngreen=1 ;\nblue=1 ;\n"
                         "reg00=h/280 ;\nsizex=reg00*0.75 ;\nsizey=sizex"),
               tx_beat="")

    # Audio-Werte als Zeilenlage: die Zeile, in der das Segment liegt, IST der
    # Wert. Anlass (S51): "Alien Alloy" leitet Swirl-Staerke UND Zeitschritt aus
    # vol=getspec(0.5,1,0) ab — weicht der Wert zwischen den Renderern ab,
    # transportiert die Dynamic Movement unterschiedlich weit.
    AUDIO_PROBEN = (("getspec(0.5,1,0)", -0.9, -0.5),
                    ("getspec(0,1,0)", -0.3, 0.1),
                    ("getosc(0,0,0)", 0.5, 0.9))
    write("5_vars/audio_werte.avs", preset(
        GRUND, set_render_mode(0, width=1),
        *[superscope(point=f"x={x0}+i*{x1 - x0}; y={expr}*1.6-0.8;"
                           "red=1;green=1;blue=1;linesize=3;",
                     init="n=2", which_ch=2, colors=(0xFFFFFF,), drawmode=1)
          for expr, x0, x1 in AUDIO_PROBEN]))

    TRI_PT = ("x1=(i-0.5)*1.6; y1=-0.5; x2=(i-0.5)*1.6+0.1; y2=0.5;"
              "x3=(i-0.5)*1.6-0.1; y3=0.5; red=1;green=1;blue=1;")
    write("5_vars/triangle_n_aus_w.avs", preset(
        GRUND, set_render_mode(0, width=1),
        triangle(point=TRI_PT, frame="n=w*0.05")))
    write("5_vars/triangle_n_literal.avs", preset(
        GRUND, set_render_mode(0, width=1),
        triangle(point=TRI_PT, frame="n=16")))

    n = len(list(OUT.rglob("*.avs")))
    print(f"{n} Sonden nach {OUT}")


if __name__ == "__main__":
    main()

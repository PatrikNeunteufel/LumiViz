# Changelog Session 124 (2026-10-14)

> **Das Fenster Kits und vier akustische Kits.** Im Drum-Editor öffnet
> **Edit kits...** (im Menü des Knopfs Kit) jetzt ein eigenes Fenster: alle Kits
> auf einen Blick, eigene Kopien machen, Klang, Volume und Pan je Rolle ändern,
> Rollen dazu und weg, anhören. Dazu Rock'n'Roll, Hardrock, Heavy Metal und Jazz
> Brushes aus echten Schlagzeug-Aufnahmen. **Geprüft mit Unit-Tests unter beiden
> Compilern, am Bild aus dem Test und mit Messungen.** Im Programm gesehen oder
> gehört ist nichts.

## Neu

- **Ein Takt reicht:** Ein Muster kann jetzt 1 bis 64 Takte lang sein (neue
  beginnen weiter mit 4). Neben **Bars** der Haken **Repeat when
  lengthening**: wer einen Takt komponiert und dann auf 4 stellt, bekommt ihn
  viermal.
- **Effekte an und aus:** Jeder Clip hat einen Schalter für alle seine
  Effekte — **FX** im Drum- und Instrument-Editor (beim Abspielen umschaltbar),
  **Effects on** im Fenster Clip effects. Aus, klingt der Clip trocken, auch im
  ganzen Loop; die Marke **fx** bleibt, aber matt.
- **Springen:** Ein Klick ins **Lineal** des Panels Loop springt an die
  Stelle, wenn der Loop spielt; sonst setzt er eine grüne Marke, an der Play
  beginnt. In beiden Editoren setzt ein Klick in die Taktzeile die
  **Abspielstelle** (grüne Flagge) — dort beginnt Play pattern, und spielt das
  Muster, springt es dorthin.
- **Vier Kits mehr** (Stufe 21d): **Rock'n'Roll** und **Hardrock** aus einem
  echten, aufgenommenen Schlagzeug (DRSKit), **Heavy Metal** aus dem Kit einer
  Metal-Band (CrocellKit, mit Doppelpedal, vier Toms, drei Crashes, China und
  Splash), **Jazz Brushes** mit gerechneten Besen. Dazu der Loop **Brush
  Swing** (Jazz, Swing in Triolen).
- **37 akustische Samples** aus DrumGizmo (CC BY 4.0) in Stereo; die
  Nennung steht in `asset\loop\samples\CREDITS.md`. Wer sie in einem Stück
  verwendet, nennt sie dort, wo auch andere Mitwirkende genannt werden.
- **Drei Besen** als Instrumente: Brush Tap, Brush Slap, Brush Sweep.
- **Kitwechsel mit Wahl:** im Menü des Knopfs Kit zwei Haken, die das
  Programm merkt (beide zuerst an): **Add all roles of the kit** legt für jede
  Rolle des Kits ohne Spur eine an — auf einem leeren Muster also das ganze
  Kit —, **Remove empty tracks the kit doesn't have** nimmt leere Spuren weg,
  die zum neuen Kit nicht passen. Eine solche Spur mit Schritten bleibt und
  trägt ein **\*** in Bernstein; eine Zeile unter dem Raster sagt, was es heißt.
- **Ein Icon für Kits** (`.lvkit`): die Marke mit einer Snare und zwei
  gekreuzten Sticks — am Knopf Kit, in Import data, im Explorer nach
  `register_file_icons.ps1 -Apply`.
- **Benutzerhandbuch** §16 „Kits" (Rollen, Knopf Kit, die Haken, das \*, das
  Fenster Kits) und §17 `.lvkit`.
- **Fenster Kits** (Stufe 21c, Skizze gebilligt S123, K7 a, K8 a):
  - links die Liste: *Built-in kits* mit Schloss, *My kits*; darunter
    **Duplicate** (eine eigene Kopie, „808 copy"), **Rename** (nur der Titel —
    Loops finden das Kit weiter) und **Delete** (fragt nach);
  - rechts das gewählte Kit: je Rolle **Sound**, **Volume**, **Pan** und ▶;
    darunter **Add role ▾**, **Remove role** und **Play all** (je Rolle ein
    Schlag, im Tempo des offenen Loops);
  - ein mitgeliefertes Kit ist schreibgeschützt: „Built-in, read-only —
    Duplicate makes your own copy";
  - **Undo** und **Redo** im Kopf (Ctrl+Z, Ctrl+Y) für die Änderungen am
    offenen Kit; **Save** schreibt es; wer wechselt oder schließt, wird nach
    ungesicherten Änderungen gefragt;
  - im Fuß **Used by**: welche Loops das Kit nennen — die mitgelieferten, die
    zehn zuletzt geöffneten und der offene.
- Der Drum-Editor zeigt nach Save, Rename, Duplicate oder Delete den Knopf
  Kit neu — „(changed)" misst gegen das Kit, wie es jetzt gespeichert ist.

## Geändert

- **Die Einfügemarke** in den Editoren setzt jetzt **Shift**+Klick in die
  Taktzeile (der Klick allein setzt die Abspielstelle).
- **Kopieren im Instrument-Editor:** Mit einem Rahmen markiert, nimmt die
  Kopie den ganzen Zeitraum des Rahmens mit — auch leere Stellen am Rand. So
  landet sie beim Einfügen dort, wo sie hingehört (vorher fehlte der Abstand
  bis zur ersten Note).
- **Play pattern spielt mit Effekten** — denen des im Loop markierten Clips
  dieser Basis, sonst ihres ersten (vorher klang es im Editor immer trocken).
- Eine `.lvloop` wird als **Fassung 7** geschrieben; ein älteres LumiViz liest
  sie, meldet aber, dass sie neuer ist.

- **Die Auswahl Sound** im Drum-Editor: ein zu langer Name endet jetzt mit
  „…" und steht ganz im Tooltip; die aufgeklappte Liste ist so breit wie ihr
  längster Eintrag (Anmerkung Patrik zur Skizze). Dieselbe Auswahl steht im
  Fenster Kits.

## Offen

- Patriks Sichttest mit `.claude\handover\Sichttest_Kits.md`.
- Gehört ist von den neuen Kits noch nichts im Programm; Hörproben liegen
  bereit.
- Kosmetisch: in der gewählten Zeile der Tabelle scheinen schmale Striche der
  Auswahlfarbe am Rand der Zellen durch.
- Die von Claude gesetzten Regeln stehen in `audio/Loop_Konzept.md` §4l „Wie
  21c gebaut ist".

## Tests

- K10/K11: ein Fall in `test_LoopKits.cpp` (das Modell), einer in
  `test_LoopDrumEditor.cpp` (`LUMIVIZ_DRUM_K11_BILD=<datei.png>` legt ein Bild
  ab — angesehen); der Fall zu 21b prüft K2 a mit beiden Haken aus. Danach
  1215 Cases, 242 Bereichsfälle unter beiden Compilern grün. Zum Abschluss von
  Stufe 21 der volle Lauf: 1215 / 1215, 0 Skips (Testing-MSVC, Bildschirm 2).
- Stufe 21d: in fünf Tests die Zahlen nachgestellt (75 Samples, 93
  Instrumente, 20 Drums, 14 Kits); zehn Prüfwerte dazu (die Besen, Brush
  Swing), keiner geändert.
- Neu `test_LoopKitFenster.cpp`, drei Fälle (`LUMIVIZ_KIT_FENSTER_BILD=<datei.png>`
  legt zwei Bilder ab); in `test_LoopDrumEditor.cpp` ist Edit kits... jetzt an.
- 1213 Cases; 240 Fälle zu Loop, Instrument, Drum, Sample, Kits,
  Datensicherung, Shape, Handbuch, Taktachse und Sichttest unter Testing-MSVC
  und Release-clang grün. Der volle Lauf kommt am Ende von Stufe 21.

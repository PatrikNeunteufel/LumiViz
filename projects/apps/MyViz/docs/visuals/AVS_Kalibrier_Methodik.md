# AVS-Kalibrier- und Fehlersuch-Methodik

> Stand: Session 50 · Gilt für die AVS-Treue-Arbeit (AvsRef als Referenz).
> Ergänzt `AVS_Sichttest_Protokoll.md` (was geprüft wurde) um das **Wie**.

Diese Methodik ist nicht ausgedacht, sondern aus dem destilliert, was in den
Sessions 44–50 tatsächlich Befunde geliefert hat — und aus den Umwegen, die
sich im Nachhinein als vermeidbar erwiesen.

---

## 1. Die Metrik lügt bei dünnen Inhalten

**Das wichtigste Einzelergebnis von Session 50.** `dMean` und `MAE` mitteln über
die ganze Fläche. Ein Vordergrund, der 2 % des Bildes bedeckt, bewegt sie nicht.

In Session 50 meldete die Bisektionsleiter **viermal „OK"**, während wir
sichtbar **gar nichts** zeichneten:

| Fall | dMean | Wirklichkeit |
|---|---|---|
| Santa, Stufe t09 | 0,009 | Schnee, Handschuhe, Sack fehlen komplett |
| Preset 30, Stufe t02 | 0,005 | der ganze Quell-Balken fehlt |
| Sonde `U4` | 0,000 | **beide** Seiten zeichneten nichts — Sonde wertlos |
| Sonde `Y1..Y4` | 0,000 | flache Kurve in beiden, Unterschied unsichtbar |

**Regel:** Ein grünes dMean ist kein Beweis. Vor jedem Urteil messen, ob
überhaupt und *wo* gezeichnet wurde:

- **Pixelzahl** — alles, was sich um >8 Stufen vom Hintergrund abhebt
- **Schwerpunkt** (Zeile/Spalte) — fängt „richtige Menge, falsche Stelle"
- **Leer-Ungleichheit** — einer zeichnet, der andere nicht = sofort rot

Umgesetzt in `run_module_probes.py`. Für die großen Sweeps gilt bis auf
Weiteres: **Montage ansehen**, nicht der Zahl glauben.

**Auch das Ersatz-Urteil hat einen blinden Fleck.** Es bestimmt den
Hintergrund als *häufigsten Farbwert des Bildes*. Ein Effekt, der die
**ganze Fläche gleichmäßig** verändert (Verstärkung, Helligkeit, Farbkurve),
verschiebt damit den Hintergrund mit — die Pixelzahl bleibt gleich und die
Sonde meldet fälschlich „unverändert". In S50 hielt ich die Convolution
deshalb erst für ein No-op; die Montage zeigte dann einen Kanten-Unterschied.

Für flächige Effekte deshalb: **fester Hintergrundwert** statt geschätztem,
oder zusätzlich Histogramm/Mittelwert vergleichen. Und in jedem Fall: bei
einem auffälligen Ergebnis **die Montage öffnen**, bevor man es deutet.

**Sonderfall:** Rendern wir schwarz, misst dMean nur noch die *Referenz*-
Helligkeit. Zwei Läufe mit 0,172 und 0,283 waren beide reines Rauschen — unser
Bild war in beiden leer.

---

## 2. Sonden statt Theorie

Sobald zwei Erklärungen plausibel sind: **Probe-Preset bauen, nicht weiter
nachdenken.** (Regel seit S48, in S50 durchgehend bestätigt.)

Eine Sonde ist ein minimales `.avs` mit **einem** fraglichen Modul auf
definiertem Grund, gebaut über `avs_preset_lib.py`. Das Urteil ist eine
**Position oder Menge im Bild**, nicht eine Zahl im Debugger.

Bewährte Sondenformen:

- **Wert als Linienlage** — `x=i*2-1; y=<ausdruck>*0.7`. Die Zeile, in der die
  Linie liegt, *ist* der Wert. So wurden alle zehn Kamera-Register auf einmal
  geprüft (Befund: alle lasen 0).
- **Wert als Kurvenform** — hochkontrastig binär: `y=above(dt,0)*1.6-0.8`.
  Zeigt, *welche* Punkte eine Bedingung erfüllen.
- **Keil** — eine Größe über die Punkte hochlaufen lassen
  (`linesize=1+i*10`), dann die Dicke an drei Spalten messen. Deckt
  „je Frame statt je Punkt ausgewertet" auf.
- **Akkumulation** — dasselbe N Frames lang zeichnen und die Bildenergie
  vergleichen. Deckt falsche Blend-Modi auf (Texer II war fest additiv statt
  Replace: nach 6 Frames 1,86-fache Energie).

**Fallen beim Sondenbau** (alle in S50 hineingetappt):

- **Degenerierte Sonde** — beide Punkte auf derselben Stelle = Segment der
  Länge 0 = nichts gezeichnet. Immer prüfen, ob die Sonde in der *Referenz*
  etwas zeichnet, bevor man unser Ergebnis bewertet.
- **Off-Screen** — unbegrenzte Zwischenwerte (`x3`, `pz`) liegen außerhalb
  von [-1,1]. Entweder klemmen (`max(-0.9,min(0.9,q))`) oder monoton
  abbilden (`q/(abs(q)+1)`).
- **Übersättigung** — in einem `clear=False`-Puffer läuft über 60 Frames alles
  auf Vollbild. Wenige Frames oder Fadeout einbauen.

---

## 3. Bisektion — nur über die Top-Level-Leiter

`bisect_avs.py` baut kumulative Präfixe; die erste divergente Stufe ist der
Täter. Das hat in S50 den Effect-List-Befund geliefert (Stufe 4 sprang von
0,000 auf 0,636).

**Einschränkungen:**

- Der **Pfad-Modus** (`bisect_avs.py preset out 1,2`) rekonstruiert derzeit
  **nicht verlustfrei** — dieselbe Konstruktion ergab in der Referenz einmal
  240, einmal 4 Pixel. Bis das geklärt ist: nur die Top-Level-Leiter nutzen.
- Stufen, die einen **Puffer-Kreislauf** abschneiden (BufferSave am
  Listenende), sind nicht isoliert bewertbar.
- Die Stufenwerte steigen auch dann, wenn *nur die Referenz* Inhalt aufbaut —
  eine wachsende Kurve heißt nicht „verteilte Feinabweichung", sondern
  vielleicht „wir zeichnen ab hier gar nichts". Erst Pixel zählen, dann deuten.

---

## 4. Layouts empirisch pinnen statt raten

Für APEs gibt es keinen Quelltext. Statt zu raten: **über den ganzen Korpus
messen.**

Beispiel Texer II (S50): über **579 Blobs** der Preset-Sammlung das Offset
gesucht, ab dem vier längenpräfixierte Zeichenketten den Rest *exakt* füllen.
Ergebnis in allen 579 identisch — 280 Byte Vorlauf. Damit war bewiesen, dass
der Bildname ein fester 260-Byte-Puffer ist und keine längenpräfixierte
Zeichenkette.

Dasselbe Muster für **Kennlinien**: Sonden mit exakt bekanntem Eingang
rendern, Ausgang je Renderer vermessen, Gesetz rekonstruieren
(`make_colormap_probes.py`, Sprite-Profil-Extraktion S50: das Default-Sprite
von Texer II wurde aus einem Referenz-Rendering ausgelesen, 20×20, Peak 252).

---

## 5. Reihenfolge der Prüfung: einzeln, dann zusammenbauen

Vier von fünf Befunden aus S50 lagen **nicht** in der 41er-Modul-Matrix, weil
die nur Builtins mit **Default-Parametern** prüft. Die Sonden-Suite
(`make_module_probes.py`) deckt die drei blinden Flecken ab:

1. **`1_render/`** — ein Zeichner auf definiertem Grund, sonst nichts
2. **`2_trans/`** — ein Transformator auf **klarem Referenzbild**
   (vier Farbfelder + Diagonale, deterministisch ohne rand und ohne Audio);
   die Farbfelder trennen Kanal- und Spiegelfehler, die Diagonale zeigt Warps
3. **`3_script/`** — dasselbe Modul, aber die **skriptbaren** Größen variiert
   (`linesize`, `drawmode`, `skip`, `n`) — genau hier saßen die Befunde
4. **`4_kopplung/`** — zwei Module, die nur über `reg`/Puffer/Listen-EEL
   zusammenhängen; erst danach ganze Presets

Erster Lauf: 19/22 grün, drei Modulfehler gefunden, die die Matrix nicht sah.

---

## 6. Wenn ein Preset kaputt aussieht

Reihenfolge, die sich bewährt hat:

1. **Chain-Dump** (`AvsStandalone --dump`) — kommen Parameter und Skripte
   überhaupt an? (Texer II lieferte leere Slots — der Decoder war schuld,
   nicht der Renderer.)
2. **Top-Level-Bisektion** — welcher Knoten bricht?
3. **Knoten isoliert nachbauen** — Skripte aus dem Dump verbatim in eine
   Sonde. Läuft er dort exakt, liegt es an der Umgebung (so kam der
   ScriptContext-Befund).
4. **Dump-Vergleich** — funktionierende Nachbildung gegen Originaldatei Feld
   für Feld diffen. Der Effect-List-Befund war ein *einziges* Feld
   (`enabled=False` gegen `True`).
5. Erst dann in den Renderer schauen.

---

## 7. Verifikationsgürtel vor jedem Commit

- `MyViz.UnitTests` — **grün heißt SUCCESS und 0 Skips**
- **Modul-Matrix** `run_matrix.py` — zwei Größen, Referenzstand 37/41
- **Modul-Sonden** `run_module_probes.py`
- Bei breit wirkenden Änderungen (geteilter Kontext, Blend, Scope-Renderer)
  ist die Matrix der wichtigste Wächter — sie betrifft jeden Scope.
- Für jeden Befund ein **Gate**. Der Texer-II-Decoder war ein Jahr lang kaputt,
  weil der einzige „Texer II"-Test absichtlich den Unbekannt-Pfad prüfte.

**Ein Fix darf nicht über seinen Geltungsbereich hinausgreifen.** Der
Multieffekt-Host bedient AVS-Importe **und** eigene LumiViz-Ketten. Eine
AVS-Treue-Korrektur, die unbedingt gilt, ändert damit auch Verhalten, das
niemand gemeldet hat.

Beispiel S50: der AVS-treue Punkt-Modus (1 Pixel je Punkt) wurde erst für den
ganzen Pfad eingeschaltet und riss den Bloom-GL-Test mit, der einen Punkt mit
`dotSize=8` als Energiequelle nutzt — dieselbe Änderung hätte jede eigene
Kette getroffen, in der `dotSize` ein bewusst gesetzter Panel-Wert ist. Die
Lösung war, die Semantik an ihre Bedingung zu binden (`dotSize <= 1`, was der
Translator nur für Importe setzt).

**Regel:** Vor einer Änderung im gemeinsamen Renderpfad fragen, woran die
AVS-Semantik erkennbar ist, und genau daran binden. Ein fehlschlagender
bestehender Test ist hier ein Fund, kein Hindernis.

---

## 8. Merkregeln zur Messung selbst

- **Zwei Größen messen** (320×240 + 740×460) — kleine Flächen maskieren
  größenabhängige Fehler.
- **`--beat-period N`** für frame-exakte Diffs, sonst driften die Beat-Detektoren.
- **rand-Presets sind zwischen Läufen nicht bit-stabil** — ±0,05 ist Rauschen.
  Vor „Regression!" den Vorstand nachmessen.
- **rand-Ausrichtung ist pro Preset alles-oder-nichts**: ein Zieher zu viel
  kippt das ganze Preset. Deshalb kann ein *korrekter* Fix einzelne Presets
  verschlechtern — das legt eine bestehende Fehlausrichtung frei.
- **Nicht-ASCII-Preset-Namen**: AvsRef ist ein ANSI-Programm; die Werkzeuge
  staged solche Presets automatisch (`compare_avsref.ascii_safe`).

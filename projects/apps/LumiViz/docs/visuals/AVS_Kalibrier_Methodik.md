# AVS-Kalibrier- und Fehlersuch-Methodik

> Stand: Session 51 · Gilt für die AVS-Treue-Arbeit (AvsRef als Referenz).
> Ergänzt `AVS_Sichttest_Protokoll.md` (was geprüft wurde) um das **Wie**.

Diese Methodik ist nicht ausgedacht, sondern aus dem destilliert, was in den
Sessions 44–51 tatsächlich Befunde geliefert hat — und aus den Umwegen, die
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
5. **`5_vars/`** (S51) — **Paar-Sonden**: welche Variablen stellt der Host dem
   Skript überhaupt bereit? Je zwei Sonden rechnen dasselbe Ergebnis, die eine
   über die Host-Variable (`n=w*0.1`), die andere über das Literal, das bei
   320×240 herauskommt (`n=32`). Stimmen die **Referenz**bilder des Paares
   überein, ist nicht nur die *Existenz* der Variablen belegt, sondern ihr
   **Wert**. Diese Sonden sind deshalb an 320×240 gebunden — die
   „immer zwei Größen"-Regel gilt hier ausdrücklich nicht.

   Dieselbe Paar-Bauweise pinnt auch **Slot-Reihenfolgen**: dieselbe Zuweisung
   einmal im Init-, einmal im Frame-Slot. In S51 wich die Referenz auseinander
   (2247 gegen 8945 Pixel) und bewies damit, dass AVS `sizex/sizey` je Frame
   neu vorbelegt — wir taten es *nach* dem Frame-Slot und löschten dessen
   Ergebnis.

Erster Lauf: 19/22 grün, drei Modulfehler gefunden, die die Matrix nicht sah.

---

## 6. Import-Roundtrip: importieren, als `.lvfx` speichern, lesen

**Der billigste Parser-Test, der existiert** — und in S51 der Weg zum
Texer-Befund. Preset importieren, im Panel als `.lvfx` speichern, das JSON
lesen. `.lvfx` ist `chainToJson` pur: was dort steht, ist genau das, was der
Renderer sieht.

Was sofort auffällt, ohne einen einzigen Frame zu rendern:

- **leere Felder**, die nicht leer sein dürfen (Dateiname, EEL-Slots — so war
  der Texer-II-Decoder in S50 als Täter erkennbar),
- **Defaults, wo Werte stehen müssten** (ein Feld, das der Decoder nie füllt,
  behält seinen Struct-Default),
- **Skripte, die Variablen benutzen, die wir nicht setzen.** Genau das war der
  S51-Befund: alle vier Texer in „Alien Alloy" rechnen `n=w*0.1` im
  **Frame**-Slot, und der Texer-Host bekam nie ein `w` → `n=0` → kein einziges
  Sprite. Vorher war die Vermutung „leerer Init", weil niemand in den
  Frame-Slot gesehen hatte.

Daraus die Anschlussprüfung, die sich als eigene Fehlerklasse erwiesen hat:
**welche Variablen liest das Skript, und setzt der zuständige Host sie alle?**
`grep '"w"' src include` listet die Setzer; jeder Skript-Träger, der fehlt, ist
ein Verdacht. Gegenprobe für Module *mit* Quelltext direkt in `ref/vis_avs`
(`registerVar`) — bei DDM und Bump war das fehlende `w`/`h` dort korrekt, es
gibt sie in AVS nicht. Für APEs ohne Quelltext entscheidet die Paar-Sonde.

Der Roundtrip prüft in der Rückrichtung zusätzlich den Serializer:
`.lvfx` neu laden und mit `freeze_lvfx_twins.py` gegen den eingefrorenen
Zwilling stellen. So fiel in S51 auf, dass der Set-Render-Mode-Knoten sein
Override-Flag unter demselben JSON-Schlüssel `enabled` ablegte wie der Knoten
selbst — der Auge-Zustand ging beim Speichern verloren.

**Wo das Import-Protokoll steht:** seit S51 hängt der Importer einen
**„Import Notes"-Knoten** in die Kette (erstes Kind nach Render Scale), sobald
es etwas zu protokollieren gibt — Zusammenfassung, Probleme, Hinweise. Das
Meldungsfenster erscheint nur noch bei **Problemen**: vorher liefen die
planmäßigen `_p`-Umbenennungen mit durch den Dialog, und ein Preset wie
„Milky Way Xtreme" produzierte 22 Zeilen Rauschen, in dem echte Probleme
untergingen (Entscheid Patrik, S51). Für die Fehlersuche heißt das: **den Knoten
lesen**, nicht auf den Dialog warten.

**Achtung beim Prüfen der Kollisionsregel:** Modul-Matrix und Sonden enthalten
*keine* Presets mit kollidierenden Namen — sie können eine Regression der Regel
strukturell nicht sehen. Wächter ist allein ein Sweep über echte Presets
(`VisualsPresets/avs/…`), wo `vol`/`time`/`dt` tatsächlich vorkommen.

---

## 7. Wenn ein Preset kaputt aussieht

Reihenfolge, die sich bewährt hat:

1. **Import-Roundtrip** (§6) oder **Chain-Dump** (`AvsStandalone --dump`) —
   kommen Parameter und Skripte überhaupt an, und *benutzen* die Skripte
   Variablen, die wir nie setzen? (Texer II lieferte leere Slots — der Decoder
   war schuld, nicht der Renderer; in S51 fehlte `w`.)
2. **Top-Level-Bisektion** — welcher Knoten bricht?
3. **Knoten isoliert nachbauen** — Skripte aus dem Dump verbatim in eine
   Sonde. Läuft er dort exakt, liegt es an der Umgebung (so kam der
   ScriptContext-Befund).
4. **Dump-Vergleich** — funktionierende Nachbildung gegen Originaldatei Feld
   für Feld diffen. Der Effect-List-Befund war ein *einziges* Feld
   (`enabled=False` gegen `True`).
5. Erst dann in den Renderer schauen.

---

## 8. Verifikationsgürtel vor jedem Commit

- `LumiViz.UnitTests` — **grün heißt SUCCESS und 0 Skips**
- **Modul-Matrix** `run_matrix.py` — zwei Größen, Referenzstand **36/41** (S54
  nachgemessen; die früher notierten 37/41 waren zu optimistisch)
- **Modul-Sonden** `run_module_probes.py` — Referenzstand **78/80** (S53/S54);
  `5_vars` läuft nur bei 320×240, seine Literale sind auf diese Größe gepinnt
- **Feld-Sonden** `fields/run_field_probes.py` (neu S54) — beantwortet als
  einzige Familie **nicht** „rechnen wir wie das Original?", sondern „wirkt
  dieses Feld überhaupt?". Kein Referenz-Renderer: verglichen werden zwei Läufe
  unseres eigenen Renderers, Vorgabe gegen ein einzelnes abweichendes Feld.
  Gehört in den Gürtel, sobald Felder dazukommen oder Renderer-Parameter ihren
  Weg ändern; ein `STUMM` ist erst nach Blick auf die Montage ein Befund
  (`asset/calibration/fields/README.md`)
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

## 9. Merkregeln zur Messung selbst

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
- **Ein Preset kann mehrere Schichten kaputt haben.** In S51 zeichneten nach
  dem Texer-Fix die Sprites wieder, das Preset blieb aber falsch, weil der
  *Transport* (Movement-Kette) den Inhalt nicht verteilt. Ein Fix, der die Zahl
  nur wenig bewegt, ist nicht deshalb falsch — die Montage sagt, ob die
  behandelte Schicht jetzt stimmt, und die Zahl gehört der nächsten.

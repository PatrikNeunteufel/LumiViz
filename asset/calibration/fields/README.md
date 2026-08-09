# Feld-Sonden (Strang E) — wirkt jedes Feld?

Die dritte Sonden-Familie neben `../avs/matrix` (Modul mit Vorgabewerten gegen
AvsRef) und `../avs/probes` (Modul einzeln gegen AvsRef). Diese hier stellt eine
andere Frage — nicht *„rechnen wir wie das Original?"*, sondern:

> **Bewirkt dieses Feld überhaupt etwas?**

Deshalb gibt es hier **keinen Referenz-Renderer**. Verglichen werden zwei Läufe
unseres eigenen Renderers: einmal mit Vorgabewerten, einmal mit genau einem
abweichenden Feld. Sind die Bilder gleich, kann das Feld nicht wirken.

Der Anlass: Session 53 hat 47 Renderern je drei Skriptfelder gegeben, und
`movement3b.lvfx` zeigte binnen Minuten, dass ein Feld dastehen kann, ohne
wirken zu können. Ein Unit-Test findet das nicht — er prüft Persistenz, nicht
Wirkung.

## Ablauf

```bash
python harvest_field_docs.py     # inventory.json -> inventory_docs.json
python make_field_probes.py      # erzeugt probes/<typkey>/*.lvfx
python run_field_probes.py       # rendert, urteilt, schreibt den Report
```

Einzelne Typen: `python run_field_probes.py blur mirror mosaic`

`inventory.json` kommt **nicht** aus einem dieser Skripte, sondern aus dem
C++-Gate `test_FieldInventory.cpp` — es ist die einzige Stelle, die alle Felder
sicher kennt. Neu schreiben nach einer Feldänderung:

```bash
LUMIVIZ_UPDATE_FIELD_INVENTORY=1 LumiViz.UnitTests.exe -tc="Feld-Inventar*"
```

**Die Reihenfolge ist eine Kette, und sie hat nur eine Richtung:**

```
Struct (C++)  ->  inventory.json  ->  inventory_docs.json  ->  probes/*.lvfx
                  (Golden, Gate)      (harvest)               (make)
```

Wer eine **Vorgabe** im Struct ändert, muss sie in dieser Reihenfolge
durchziehen. Der Ernter vor dem Golden gelaufen heisst: der Generator rechnet mit
der ALTEN Vorgabe. In S57 wurde `blur.roundUp` von `true` auf `false` gestellt,
der Ernter lief eine Minute zu früh — der abgeleitete Gegenwert `not true` war
`false`, also genau die neue Vorgabe, und die Sonde meldete weiter „stumm" für
ein Feld, das gerade erst implementiert worden war.

## Lauflänge und Beat

Vorgabe sind **181 Frames** (rund drei Sekunden bei 60 fps) mit
`--beat-period 30`. Beides zusammen ist nötig:

- **Drei Sekunden**, damit ein Effekt Zeit hat zu wirken — Rampen, Nachlauf-
  fenster und Puffer brauchen mehr als ein paar Frames.
- **Sechs Beats darin**, damit beat-gebundene Felder überhaupt auslösen.
- **Der letzte Frame ist selbst ein Beat** (`(181-1) % 30 == 0`), damit die
  Wirkung im Schlussbild steht.

Der letzte Punkt ist der wichtigste und war lange übersehen: mit 40 Frames war
`onBeatClear` sechsmal stumm. Der Knoten löschte korrekt — nur wurde der
Untergrund im Folgeframe wieder darübergezeichnet, und der Screenshot entsteht
am Ende. Mit 181 Frames wirken alle sechs Felder (`color` mit MAE 0,667).

Wer `--frames` oder `--beat-period` ändert, bekommt eine Warnung, sobald der
letzte Frame kein Beat mehr ist.

Genau dieser Vorteil ist für manche Felder ein Nachteil, und zwar unheilbar:
Colorfade **ersetzt** im Beat-Fenster die normalen Fader durch die Beat-Fader —
im Beat-Frame sind `faderR/G/B` deshalb grundsätzlich unsichtbar, egal was man
einstellt. Das Fenster lässt sich auch nicht schließen, `onBeatFrames = 0` hebt
der Validator auf 1 (`EffectChain.hpp`, Vertrag `>= 1`). Solche Felder brauchen
eine **eigene Lauflänge**: `FRAMES_JE_FELD` im Runner, je Sonde statt je Typ.
Das Vergleichsbild wird mit derselben Länge gerendert, und die Bilder liegen je
Länge in `bilder/<typ>/f<N>/` — sonst überschreiben zwei Läufe derselben Datei
einander. In der Konsole steht die abweichende Länge hinter dem Messwert, im
Report als eigene Spalte.

Drei Sekunden sind auch für **akkumulierende** Effekte zu lang — sie stehen dann
am Anschlag und kein Wert macht mehr einen Unterschied. Dafür gibt es
`FRAMES_JE_TYP` (Colorfade, Fadeout, Brightness: 61). Beide Zahlen müssen weiter
auf einem Beat enden, außer die Sonde will ausdrücklich daneben liegen.

## Das Urteil

| | heißt |
|---|---|
| `WIRKT` | die Bilder unterscheiden sich deutlich |
| `SCHWACH` | Unterschied unter MAE 0,001 — ansehen: zaghafter Gegenwert oder Rand-Wirkung |
| `STUMM` | Pixel für Pixel gleich. Das Feld kann so nicht wirken |

Zwei Läufe desselben Presets sind **bit-identisch** (nachgemessen S54), deshalb
braucht `STUMM` keine Toleranz: jede Abweichung ≠ 0 kommt vom Feld.

**`STUMM` ist nicht automatisch ein Befund an der App.** Es kann genauso am
Testaufbau liegen — an einem Gegenwert, der nichts bewegt, oder an einem Feld,
das nur in Gesellschaft wirkt. Deshalb schreibt der Runner für jede stumme Sonde
eine **Montage**: erst ansehen, dann urteilen. Diese Reihenfolge hat sich in S54
dreimal ausgezahlt.

## Der Aufbau je FELD (S57)

Drei Tabellen wirken je Typ (`UNTERGRUND_JE_TYP`, `VORLAUF`, `KINDER`), zwei je
**Feld**. Der Unterschied ist wichtig: ein Feld, das einen anderen Bildinhalt
braucht als seine Geschwister, darf nicht den ganzen Typ umbauen — sonst
verschieben sich die Messwerte aller anderen Sonden mit.

- **`VORLAUF_JE_FELD`** — Knoten vor dem Prüfling, nur für dieses Feld.
  `convolution.edgeMode` wählt, was der Kern *jenseits* des Bildrandes liest. Der
  Untergrund ist am Rand überall 0x101010; bei einfarbigem Rand liefern
  Festklemmen und Umlaufen dasselbe, und zwar **exakt** — MAE 0,0000, zwei
  Vollaufe lang „stumm". Zwei breite Diagonalen von Ecke zu Ecke machen den Rand
  ungleich, dann trennt das Feld (0,0014).
- **`UNTERGRUND_JE_FELD`** — ein Untergrund, der nur einmal löscht, nur für
  dieses Feld. `bloom.post` entscheidet, WO der Glow entsteht: beim Present oder
  in der Kette. Beide Wege erzeugen denselben Glow aus derselben Quelle, der
  Unterschied lebt allein davon, dass der **nächste Frame** ihn sieht. Ohne
  Rückkopplung gibt es kein nächstes Mal (0,0000 → 0,7997). Bewusst nur für
  dieses Feld: mit Rückkopplung sättigt der additive Glow über 181 Frames
  (S48-Befund), und auf einem gesättigten Bild könnten `intensity` und `radius`
  ihrerseits nichts mehr zeigen.

Beide erzwingen einen **eigenen Vergleichsgrund**, der denselben Vorlauf und
denselben Untergrund trägt — sonst hielte der Runner die Sonde gegen `_default`,
und der Vergleich trüge zwei Unterschiede statt einem.

## Die drei Tabellen in `make_field_probes.py`

Jede trägt Wissen, das sich nicht ableiten lässt, und jede ist aus einem
Fehlversuch entstanden:

- **`HANDWERK`** — Gegenwerte, für die es keine Regel gibt: ein Movement-Ausdruck,
  ein Bildpfad. Auch **Bitfelder**: `mirror.mode` 4 → 12 addiert nur Bits und
  sieht aus wie die Vorgabe; der Gegenwert muss die *Achse* wechseln. Und Fälle,
  in denen die **Richtung** zählt statt des Abstands: Colorfade legt den zweiten
  Fader immer auf den größten Kanal, der auf unseren Testbalken 255 ist — ein
  positiver Wert wird dort weggeschnitten (`beatFaderG` −32 statt +32). Und
  Vorgaben, die auf einen **anderen Knoten verweisen**: `timescope.blend = 3`
  heißt „folge dem Set Render Mode", dessen Vorgabe 0 = Ersetzen ist — der
  abgeleitete Gegenwert 0 war dieselbe Betriebsart (S56).
- **`GRUNDKONFIG`** — Felder, die nur in Gesellschaft wirken. `mirror.slower`
  steuert eine Rampe, die es ohne `smooth` nicht gibt. Solche Felder bekommen
  einen **eigenen Vergleichsgrund** (`_grund_<feld>.lvfx`), sonst unterscheiden
  sich die zwei Läufe in zwei Dingen statt einem.
- **`NICHT_PRUEFBAR`** — was dieses Testsignal grundsätzlich nicht zeigen kann,
  mit Begründung. Bisher die Timescope-Kanalfelder: der Standalone erzeugt das
  Spektrum für beide Kanäle gleich, links/rechts/Mitte sind dort zwangsläufig
  identisch. Ein `STUMM` wäre hier eine Falschaussage über die App.

## Der Untergrund

`lvfx_lib.py` baut ihn: vier Farbfelder plus Diagonale auf `0x101010` — der
Nachbau des Referenzbilds aus `../avs/make_module_probes.py` (S50). Bewusst
**kein** zweites Testbild, damit ein Befund zwischen beiden Sonden-Familien
vergleichbar bleibt. Die Farbfelder trennen Kanal- und Spiegelfehler, die
Diagonale zeigt Warps.

Statisch heißt wörtlich: kein `t`, kein `rand`, kein Audio, kein Beat. Das
Testsignal für die Renderer liefert der Standalone selbst (Sinus + Beat-Puls,
`main.cpp`) — deterministisch, ein WAV wird nicht gebraucht.

**Wer je Frame nur einen Bruchteil des Bildes zeichnet, braucht einen
Untergrund, der nur einmal löscht** (`UNTERGRUND_JE_TYP`). Timescope zeichnet
eine ein Pixel breite Spalte je Frame und schiebt sie weiter; der Untergrund
malte sie im Folgeframe wieder zu, und im Schlussbild stand genau eine Spalte
von 320. Alle acht Felder lagen deshalb unter der SCHWACH-Schwelle, obwohl sie
sichtbar wirken — die Sonde maß 1/320 ihrer Wirkung (S55 gesehen, S56 behoben:
7 SCHWACH → 7 WIRKT, MAE 0,0009 → 0,127). Mit `onlyFirst` sammeln sich die
Spalten über die Lauflänge; Balken und Diagonale werden weiter je Frame
gezeichnet und bleiben als Orientierung stehen.

## Strang G — Skript-Lint über die Asset-Chains (`lint_chain_scripts.py`, S62)

Die Feld-Sonden prüfen die MODULE; die handgeschriebenen Chains in
`asset/effectchain|examples|composits` prüft niemand. Anlass war
`domain.lvfx`: die Beat-Richtungsumkehr lief über `dx`/`dy` — Variablen, die
der domainWarp-Vertrag nie liest. Der Flip war ein stummer No-op (MAE
mit/ohne Beats exakt 0,0000; nach dem Fix 0,1817).

```bash
python lint_chain_scripts.py                # die drei Asset-Sammlungen
python lint_chain_scripts.py datei.lvfx     # einzeln
```

Befund = eine Variable wird in einem Skript-Slot GESCHRIEBEN, aber weder vom
Modul (skriptvars aus `inventory_docs.json`) noch von einem Slot desselben
Knotens GELESEN. `reg00…`/`q…` sind knotenübergreifend und nie ein Befund;
EEL ist case-insensitiv. Keine transitive Analyse (`a=…; b=a;` mit
ungelesenem `b` meldet nur `b`).

**Was der Erstlauf (S62) über den ERNTER fand:** drei Vertragslücken in
`harvest_field_docs.py` — (1) `b` wurde pauschal als Beat-Eingabe verworfen,
beim Attraktor ist es aber der De-Jong-Parameter (jetzt konditional: nur wenn
der Host `b` selbst mit dem Beat-Flag belegt); (2) Movements Skriptfeld heißt
schlicht `code` und fehlte im Feldnamen-Filter; (3) Mechanismus 2 sah nur die
`setNumber`-Seeds — Nur-Lese-Ziele wie Texer-IIs `x`/`y` fehlten (jetzt Union
mit den `number()`-Reads, gleiche Regel wie Mechanismus 3). Danach blieben
als echte Befunde nur tote Variablen in importierten Originalen
(alien alloy `xx`/`yy`, test1 `m`/`u`/`tm`, 13c `pi`) — wörtliche Importe,
bleiben unangetastet.

## Strang F — wirkt das Feld auch beim EDITIEREN? (`run_edit_probes.py`)

Dieselben Sonden, eine andere Frage. Anlass war Patriks Beobachtung, ein Feld
wirke erst nach Speichern und Laden. Der Mechanismus dahinter ist belegt:
**Laden** setzt `m_pendingRuntimeReset` → `resetRuntimes()`, jeder Knoten baut
seine Runtime frisch auf; **Editieren** ruft nur `recompileChain()` — **kein**
Reset. Was ein Knoten einmalig beim Aufbau übernimmt, hängt danach fest.

```bash
python run_edit_probes.py [typkey ...]
```

Gemessen wird dasselbe Feld auf zwei Wegen: `GELADEN` = der Wert steht im
Preset · `EDITIERT` = `_default.lvfx --edit-nach <feld>`, der Wert wird nach der
halben Lauflänge gesetzt.

| | heißt |
|---|---|
| `GLEICH` | editiert == geladen |
| `TEILWEISE` | editiert liegt dazwischen: wirkt, trägt aber die Vorgeschichte — bei Effekten mit Verlauf der Normalfall |
| `VERDECKT` | editiert == Vorgabe, aber die Gegenrichtung ändert das Bild: der Wert kommt an, diese Sonde kann ihn nur nicht zeigen. **Kein Befund** |
| `WIRKUNGSLOS` | editiert == Vorgabe **und** die Gegenrichtung ändert nichts: der Wert kommt nicht an. **Befund** |

Das dritte Bild (`VORGABE`, derselbe Lauf ohne Edit) trennt „wirkt gar nicht"
von „trägt Vorgeschichte" — ohne es zählte der erste Anlauf 50 Abweichungen,
von denen fast keine eine war.

Das **vierte** Bild (Gegenrichtung: Start = Sonde, Edit → Grund) ist die
Korrektur aus S56. „Editiert == Vorgabe" hat nämlich zwei Ursachen: der Wert
kommt nicht an (Befund) — oder er kommt an und wird vom Zustand der ersten
Hälfte verdeckt (Puffer hält seinen Inhalt, Rampe ist abgelaufen, Zähler steht
anderswo). Auf dem Rückweg ist der Zustand der des *gesetzten* Feldes und kann
nicht auf dieselbe Weise verdecken. S55 hatte diese Unterscheidung nicht und
führte acht Messartefakte als bestätigte Befunde. Beide Ursachen sind an
Positivkontrollen belegt: `bufferSave.slot` mit dem Leser auf dem leeren Slot 7
fällt von WIRKUNGSLOS auf GLEICH, `mirror.slower` mit `onBeatRandom` auf
TEILWEISE. Das vierte Bild kostet nur im Verdachtsfall einen Renderlauf.

## Bekannte Lücken

Keine offenen mehr — Strang A hat die drei alten geschlossen (S55): die
Skriptfelder von Knoten ohne `runParamScript` über einen dritten
Erntemechanismus (geerntet wird jetzt auch das **Lesen** von `number("…")` im
Renderer und in dessen Modul), die Bildfelder über `testbild.png`, die
Kanalfelder über `--stereo-spektrum` im Standalone. 0 Felder ohne erzeugbare
Sonde.

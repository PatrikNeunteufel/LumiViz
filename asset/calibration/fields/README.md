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
LUMIVIZ_UPDATE_FIELD_INVENTORY=1 MyViz.UnitTests.exe -tc="Feld-Inventar*"
```

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

## Die drei Tabellen in `make_field_probes.py`

Jede trägt Wissen, das sich nicht ableiten lässt, und jede ist aus einem
Fehlversuch entstanden:

- **`HANDWERK`** — Gegenwerte, für die es keine Regel gibt: ein Movement-Ausdruck,
  ein Bildpfad. Auch **Bitfelder**: `mirror.mode` 4 → 12 addiert nur Bits und
  sieht aus wie die Vorgabe; der Gegenwert muss die *Achse* wechseln.
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

## Bekannte Lücken

- **104 Felder ohne erzeugbare Sonde**: Skriptfelder von Knoten ohne
  `runParamScript` (Fractal 2D/3D, Flame, Fractal Zoomer, Domain Warp …). Sie
  haben eigene Slots, deren schreibbare Variablen der Ernter nicht kennt.
- **Bildfelder** (`picture`, `texer`) brauchen ein festgelegtes Test-Asset.
- **Kanalabhängige Felder** brauchen echtes Stereo-Material (`…\cmake\TestAudio`).

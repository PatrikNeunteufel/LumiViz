# asset/loop — Kit und Beispiel für den Loop

Konzept: `projects/apps/LumiViz/docs/audio/Loop_Konzept.md` (Entscheid L5 a:
das mitgelieferte Kit wird gerechnet, nicht bezogen).

## kit/

Sechs Klänge, 44100 Hz, 16 Bit, Mono. Der Dateiname ist der Name im Loop:
`kick.wav` ist `kit:kick`.

| Datei | Im Loop | Länge | Wie gerechnet |
|---|---|---|---|
| `kick.wav` | `kit:kick` | 500 ms | Sinus, der von 160 auf 45 Hz fällt, kurzer Klick, leicht gesättigt |
| `snare.wav` | `kit:snare` | 300 ms | zwei Töne (185 und 330 Hz) und Rauschen über 1800 Hz |
| `hat_closed.wav` | `kit:hat_closed` | 90 ms | sechs Rechteck-Töne in unharmonischem Abstand und Rauschen, nur über 7 kHz |
| `hat_open.wav` | `kit:hat_open` | 500 ms | wie die geschlossene, klingt länger aus |
| `clap.wav` | `kit:clap` | 350 ms | Rauschen um 1200 Hz, drei kurze Schläge, dann der Ausklang |
| `tom.wav` | `kit:tom` | 450 ms | Sinus, der von 200 auf 110 Hz fällt |

Erzeugt mit `kit/make_kit.py` (Python 3 mit numpy):

```
python asset/loop/kit/make_kit.py
```

Das Skript nimmt keinen Zufall von außen; zwei Läufe ergaben bytegleiche
Dateien (S108, Python 3.12.0, numpy 2.5.0). Auf einer anderen Fassung von
numpy kann die letzte Stelle einer Rechnung abweichen — maßgebend sind die
Dateien im Repo, nicht ein neuer Lauf.

An den Klängen gibt es keine fremden Rechte: sie sind gerechnet, nicht
aufgenommen.

## drums_demo.lvloop

Das Beispiel aus dem Konzept mit allen sechs Klängen des Kits: drei Muster
(Grund, Fill, Break), Abfolge über acht Takte, 120 BPM.
`test_LoopKern.cpp` liest die Datei und prüft, dass jeder ihrer Klänge im
Kit liegt.

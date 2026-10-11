# asset/loop — Kit, Instrumente und Beispiele für den Loop

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

## instruments/

Die neunundvierzig mitgelieferten Instrumente, je eine Datei `.lvinst` (JSON). Der
Dateiname ist der Name im Loop: `piano.lvinst` ist das Instrument `piano`.
Im Programm steht kein Instrument — maßgebend sind diese Dateien (Entscheide
Patrik S110, I2 b und I3 a). Die Felder beschreibt das Konzept §4f.

| Datei | Titel | Wie gerechnet |
|---|---|---|
| `bass.lvinst` | Bass | Säge mit Tiefpass, den die Hüllkurve kurz öffnet |
| `guitar.lvinst` | Guitar | die gezupfte Saite |
| `lead.lvinst` | Lead | Rechteck mit Vibrato |
| `organ.lvinst` | Organ | Sinus mit Obertönen, steht, solange die Note dauert |
| `piano.lvinst` | Piano | Sinus mit fünf Obertönen, harter Einsatz, klingt ab — wie ein E-Piano |
| `strings.lvinst` | Strings | drei verstimmte Sägen, weicher Einsatz, leichtes Vibrato |
| `violin.lvinst` | Violin | eine Säge, heller, mit deutlichem Vibrato |

Dazu sieben für **House** (Wunsch Patrik S110) — wie alle nach Rechnung
eingestellt, nicht nach Gehör:

| Datei | Titel | Wie gerechnet | Wofür |
|---|---|---|---|
| `house_piano.lvinst` | House Piano | Sinus mit sieben Obertönen, harter Einsatz, klingt in gut einer halben Sekunde aus | Akkord-Stabs |
| `house_organ.lvinst` | House Organ | Sinus mit Oktave und Quinte, perkussiv | Basslinien, kurze Akkorde |
| `chord_stab.lvinst` | Chord Stab | drei verstimmte Sägen, der Filter schnappt mit dem Anschlag auf und zu | Akkord-Stabs |
| `warm_pad.lvinst` | Warm Pad | drei verstimmte Sägen, sehr weicher Einsatz, langer Ausklang | Flächen |
| `sub_bass.lvinst` | Sub Bass | fast reiner Sinus | der Bass unter allem |
| `acid_bass.lvinst` | Acid Bass | Säge, deren Filter mit jeder Note zugeht — ohne Resonanz, die kann der Filter nicht | Basslinien in Sechzehnteln |
| `pluck.lvinst` | Pluck | Rechteck, kurz und spitz | Arpeggios |

Dazu sechzehn aus **mehreren Schichten** (S111, Fassung 2 der Datei; Wunsch
Patrik: weitere Instrumente, auch ganz ungewöhnliche) — erzeugt mit
`make_examples.py`, nach Rechnung eingestellt, nicht nach Gehör:

| Datei | Titel | Schichten | Wie gerechnet |
|---|---|---|---|
| `bell.lvinst` | Bell | Strike, Partial, Ping | Sinus, der lange ausklingt; darüber Oktave plus Quinte mit Schwebung und ein kurzer Ton zwei Oktaven höher |
| `tine_piano.lvinst` | Tine Piano | Body, Tine | Sinus mit Obertönen, leicht verzerrt; dazu ein kurzer Ton zwei Oktaven höher |
| `brass.lvinst` | Brass | Section, Edge | zwei verstimmte Sägen, deren Filter mit dem Anschlag aufgeht; ein Rechteck setzt 20 ms später ein |
| `flute.lvinst` | Flute | Tone, Air, Chiff | Dreieck mit Vibrato, ein leiser Sinus eine Oktave höher, eine sehr kurze Saite als Anblasgeräusch |
| `super_saw.lvinst` | Super Saw | Wide, Tight, Top | neun Sägen: dreimal drei, verschieden weit verstimmt, eine Gruppe eine Oktave höher |
| `fifth_pad.lvinst` | Fifth Pad | Root, Fifth, Low | Fläche aus Sägen, die Quinte setzt 120 ms später ein, darunter ein Sinus eine Oktave tiefer |
| `overdrive_guitar.lvinst` | Overdrive Guitar | String, Fifth, Octave | drei gezupfte Saiten — Grundton, Quinte, Oktave — durch den Verzerrer: jede Note ist ein Power-Chord |
| `kalimba.lvinst` | Kalimba | Tine, Knock, Ring | kurzer Sinus, eine sehr kurze Saite als Anschlag, ein Schimmer zwei Oktaven höher |
| `marimba.lvinst` | Marimba | Bar, Mallet | Sinus mit dem vierten Teilton, dazu ein Dreieck von 40 ms als Schlägel |

Sieben davon sind **ungewöhnlich**:

| Datei | Titel | Schichten | Was sie tun |
|---|---|---|---|
| `cascade.lvinst` | Cascade | One bis Four | jede Note spielt vier Töne nacheinander: Grundton, Oktave, Oktave plus Quinte, zwei Oktaven — je 90 ms später |
| `siren.lvinst` | Siren | Wail, Shadow | Rechteck, dessen Tonhöhe langsam um einen Halbton auf und ab geht; eine zweite Stimme folgt 150 ms später |
| `alien_choir.lvinst` | Alien Choir | Vowel, Whisper, Throat | Sinus nur mit ungeraden Teiltönen, weit verstimmt, drei verschiedene Vibrati gegeneinander |
| `broken_radio.lvinst` | Broken Radio | Carrier, Static, Hum | Rechteck mit dem schnellsten und tiefsten Vibrato, voll verzerrt; dazu eine verzerrte Saite zwei Oktaven höher |
| `ghost.lvinst` | Ghost | Wail, Breath | setzt in knapp einer Sekunde ein, schwankt langsam und weit, klingt anderthalb Sekunden nach |
| `growl_bass.lvinst` | Growl Bass | Growl, Grit, Sub | verzerrte Säge mit schnellem Vibrato, ein verzerrtes Rechteck darüber, ein Sinus darunter |
| `reverse_swell.lvinst` | Reverse Swell | Swell, Shine | schwillt in 0,8 s an, der Filter geht dabei auf, und bricht mit dem Ende der Note ab — wie rückwärts gespielt |

Neunzehn weitere kamen mit den **Genre-Beispielen** (S111, Wunsch Patrik:
fehlende Instrumente dazu) — erzeugt mit `make_genres.py`, nach Rechnung
eingestellt, nicht nach Gehör:

| Datei | Titel | Schichten | Wie gerechnet |
|---|---|---|---|
| `trance_lead.lvinst` | Trance Lead | Main, Octave, Bite | sechs verstimmte Sägen über zwei Oktaven, dazu ein kurzes verzerrtes Rechteck |
| `trance_pluck.lvinst` | Trance Pluck | Pluck, Shine, Body | Sägen, deren Filter mit dem Anschlag zuschnappt; ein Sinus darunter |
| `rolling_bass.lvinst` | Rolling Bass | Roll, Sub | sehr kurze, leicht verzerrte Säge über einem Sinus |
| `hoover.lvinst` | Hoover | Howl, High, Edge | weit verstimmte, verzerrte Sägen mit Vibrato, eine Oktave darüber, ein Rechteck |
| `clavinet.lvinst` | Clavinet | String, Bark | kurze, verzerrte Saite und ein Rechteck, dessen Filter zuschnappt |
| `slap_bass.lvinst` | Slap Bass | String, Thump, Pop | Saite, Sinus darunter, ein Knall eine Oktave höher |
| `clean_guitar.lvinst` | Clean Guitar | String, Shimmer | zwei Saiten, die zweite eine Oktave höher und 10 ms später |
| `muted_guitar.lvinst` | Muted Guitar | Mute, Thud | stark gedämpfte Saite und ein Dreieck von 50 ms |
| `round_bass.lvinst` | Round Bass | Round, Finger | Sinus mit zwei Obertönen, ein kurzes Dreieck als Anschlag |
| `bass_808.lvinst` | 808 Bass | Boom, Click | verzerrter Sinus, der fast eine Sekunde ausklingt |
| `reese_bass.lvinst` | Reese Bass | Reese, Sub | drei weit verstimmte Sägen mit tiefem Filter über einem Sinus |
| `upright_bass.lvinst` | Upright Bass | String, Wood | Saite und ein Sinus, der mitschwingt |
| `nylon_guitar.lvinst` | Nylon Guitar | String, Warmth | Saite und ein gefiltertes Dreieck |
| `sax.lvinst` | Sax | Reed, Breath | leicht verzerrtes Rechteck und eine Säge, gefiltert, mit spätem Vibrato |
| `vibraphone.lvinst` | Vibraphone | Bar, Strike | Sinus mit dem vierten Teilton, klingt lange aus, leichtes Vibrato |
| `steel_drum.lvinst` | Steel Drum | Pan, Ring, Hit | Sinus mit Oktave und viertem Teilton, eine Schwebung darüber |
| `cowbell.lvinst` | Cowbell | Low, High | zwei kurze Rechtecke im Abstand einer Quinte — Schlagwerk: die Tonhöhe der Note ist die des Schlags |
| `conga.lvinst` | Conga | Skin, Slap | kurzer Sinus und eine sehr kurze Saite — Schlagwerk |
| `clave.lvinst` | Clave | Wood, Tick | Sinus von 45 ms — Schlagwerk |

Ein eigenes Instrument ist eine solche Datei an beliebigem Ort; ein Muster
nennt sie beim Pfad statt beim Namen. `test_LoopInstrument.cpp` liest den
Ordner, rechnet je Instrument einen Ton und prüft Tonhöhe und Länge.

## drums_demo.lvloop

Ein Beispiel mit allen sechs Klängen des Kits, 120 BPM, 16 Takte (32 s):

- Kanal **Drums:** die Muster Grund, Fill, Grund, Break, je vier Takte.
- Kanal **Percussion:** das Muster Claps (Clap und Tom) ab Takt 9, zweimal.

Lautstärke des Loops 0,35: die Summe erreicht damit höchstens 0,84.
`test_LoopKern.cpp` liest die Datei und prüft, dass jeder ihrer Klänge im
Kit liegt; `test_LoopWiedergabe.cpp` rechnet einen Durchlauf und prüft, dass
die Summe unter 1 bleibt.

## house_demo.lvloop

Ein Beispiel mit den sieben Instrumenten für House, 124 BPM, 24 Takte
(46,5 s), Akkorde Am – Am – F – G:

- Kanal **Drums:** Kick auf jedem Schlag, Clap auf 2 und 4, Hi-Hat dazwischen.
- Kanal **Bass:** House Organ (Takt 1 bis 12), Sub Bass (13 bis 16), Acid
  Bass (17 bis 24).
- Kanal **Keys:** House Piano (Takt 5 bis 12), Chord Stab (13 bis 24).
- Kanal **Pad:** Warm Pad ab Takt 9.
- Kanal **Lead:** Pluck als Arpeggio ab Takt 17.

Lautstärke des Loops 0,3: die Summe erreicht höchstens 0,52
(`test_LoopInstrument.cpp` rechnet einen Durchlauf).

## Fünf Beispiele mit den Instrumenten aus Schichten (S111)

Erzeugt mit `make_examples.py` (Python 3, ohne weitere Pakete); je 16 Takte.
`test_LoopInstrument.cpp` lädt jede Datei, rechnet einen Durchlauf, prüft,
dass die Summe unter 1 bleibt, und dass jedes der sechzehn Instrumente in
einem der Beispiele vorkommt. Mit `LUMIVIZ_INSTRUMENT_WAV=<Ordner>` legt der
Fall je Beispiel eine Hörprobe ab.

| Datei | Titel | BPM | Was spielt |
|---|---|---|---|
| `bells_demo.lvloop` | Bell garden | 92 | Fifth Pad, Kalimba, Marimba, Bell, Sub Bass, leichtes Schlagzeug; C – Am – F – G |
| `synthwave_demo.lvloop` | Night drive | 108 | Super Saw, Bass, Tine Piano, Brass, Schlagzeug; Am – F – C – G |
| `rock_demo.lvloop` | Garage | 132 | Overdrive Guitar, Growl Bass, Organ, Lead, Schlagzeug; E – G – A – E |
| `strange_demo.lvloop` | Strange machines | 100 | Broken Radio, Alien Choir, Cascade, Siren, Growl Bass, Schlagzeug mit Swing; d-Moll |
| `ambient_demo.lvloop` | Slow tide | 72 | ohne Schlagzeug: Fifth Pad, Reverse Swell, Ghost, Flute, Bell, Cascade; Dm – B – F – C |

Gerechnet erreicht die Summe 0,49 / 0,44 / 0,50 / 0,41 / 0,36.

```
python asset/loop/make_examples.py
```

Maßgebend sind die Dateien im Repo, nicht ein neuer Lauf des Skripts: wer ein
Instrument im Creator nachstellt und speichert, ändert die Datei.

## genres/ — neunzehn Beispiele nach Genre (S111)

Wunsch Patrik: Loops für Trance, Funk, Reggaeton, Reggae und viele mehr.
Erzeugt mit `make_genres.py`. Rhythmus, Tonart und Besetzung folgen dem, was
das Genre üblicherweise ausmacht — **gerechnet, nicht gehört**: ob es so
klingt, muss ein Mensch hören. Das Kit hat nur sechs Klänge; Cowbell, Congas
und Clave sind deshalb Instrumente.

| Datei | Titel | Genre | BPM | Was spielt |
|---|---|---|---|---|
| `trance_uplifting.lvloop` | Sunrise | Trance | 138 | 32 Takte mit Breakdown und Wirbel: Rolling Bass auf den Offbeats, Trance Pluck in Sechzehnteln, gegatete Super-Saw-Akkorde, Warm Pad, Trance Lead |
| `trance_acid.lvloop` | Acid line | Acid Trance | 140 | Acid Bass in Sechzehnteln, Rolling Bass, Hoover, Siren als Steigerung |
| `trance_psy.lvloop` | Full on | Psytrance | 145 | Bass auf den drei Sechzehnteln nach jeder Kick, phrygische Linie, Broken Radio, Alien Choir, Cascade |
| `funk.lvloop` | Pocket | Funk | 104 | Slap Bass, Clavinet, Muted Guitar, Bläser-Einwürfe; leichter Swing |
| `disco.lvloop` | Mirror ball | Disco | 120 | Oktav-Bass, Clean Guitar, Tine Piano, Strings, Bläser-Melodie |
| `reggae.lvloop` | One drop | Reggae | 76 | Kick und Rim nur auf der Drei, Round Bass, Gitarre auf Zwei und Vier, Orgel dazwischen, Bläser, eine Dub-Sirene |
| `ska.lvloop` | Rude step | Ska | 152 | Gitarre und Piano auf jedem Offbeat, laufender Kontrabass, Bläser, Sax |
| `reggaeton.lvloop` | Dembow | Reggaeton | 94 | der Dembow-Rhythmus, 808 Bass, Nylon Guitar, Steel Drum, Warm Pad, Cowbell |
| `hiphop.lvloop` | Boom bap | Hip-Hop | 90 | Kontrabass, Tine Piano mit Septakkorden, Vibraphone; starker Swing |
| `trap.lvloop` | Half time | Trap | 140 | Snare nur auf der Drei, Hi-Hat-Wirbel, 808 Bass, Bell, Kalimba, Ghost |
| `dnb.lvloop` | Roller | Drum and Bass | 174 | Breakbeat, Reese Bass, Warm Pad, Tine Piano, Vibraphone |
| `techno.lvloop` | Warehouse | Techno | 130 | Rolling Bass, Chord Stab, Broken Radio, Hoover, Toms |
| `bossa.lvloop` | Praia | Bossa Nova | 132 | Clave über zwei Takte, Kontrabass, Nylon Guitar, Flute |
| `blues.lvloop` | Twelve bars | Blues | 96 | zwölf Takte in A im Dreier-Raster (drei Schritte je Schlag): Boogie auf Bass und Gitarre, Orgel, Sax |
| `waltz.lvloop` | Three steps | Jazz Waltz | 138 | drei Schläge je Takt: Kontrabass auf der Eins, Nylon Guitar auf Zwei und Drei, Vibraphone, Flute |
| `lofi.lvloop` | Rainy window | Lo-Fi | 78 | Tine Piano mit Septakkorden, Round Bass, Vibraphone, Kalimba; Swing |
| `synthpop.lvloop` | Neon heart | Synth-Pop | 118 | Bass in Achteln, Super Saw, Pluck-Arpeggio, Lead, Bläser-Motiv |
| `afrobeat.lvloop` | Lagos night | Afrobeat | 108 | Cowbell und Congas als Instrumente, Round Bass, Clean Guitar, Orgel, Bläser |
| `metal.lvloop` | Iron road | Metal | 160 | Doppel-Kick in Sechzehnteln, Overdrive Guitar, Growl Bass, Strings, Solo in harmonisch Moll |

Je 16 Takte, außer Sunrise (32) und dem Blues (12). `test_LoopInstrument.cpp`
lädt jede `.lvloop` des Ordners, rechnet einen Durchlauf und prüft, dass die
Summe zwischen 0,3 und 0,9 bleibt (gerechnet: 0,35 bis 0,58); mit
`LUMIVIZ_INSTRUMENT_WAV=<Ordner>` legt der Fall je Beispiel eine Hörprobe
unter `<Ordner>/genres` ab.

```
python asset/loop/make_genres.py
```

## instrument_demo.lvloop

Ein Beispiel mit Instrument-Mustern (Konzept §4f, seit S110), 100 BPM,
16 Takte (38,4 s), Akkorde Am – F – C – G:

- Kanal **Chords:** Strings, über alle 16 Takte.
- Kanal **Drums** und Kanal **Bass:** ab Takt 5.
- Kanal **Lead:** nacheinander Piano (Melody), Guitar (Pluck), Violin (Solo),
  je vier Takte ab Takt 5.

Die Töne sind gerechnet (`loop/Instrument.hpp`), in der Datei stehen nur die
Noten. `test_LoopInstrument.cpp` lädt die Datei, rechnet einen Durchlauf und
prüft, dass die Summe unter 1 bleibt; mit `LUMIVIZ_INSTRUMENT_WAV=<Ordner>`
legt der Fall Hörproben ab — das Beispiel und je Instrument eine Tonleiter
mit Akkord.

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

Die neunzig mitgelieferten Instrumente (bis S122 fünfundfünfzig; die fünfunddreißig
aus S123 stehen im Abschnitt S123 unten), je eine Datei `.lvinst` (JSON). Der
Dateiname ist der Name im Loop: `piano.lvinst` ist das Instrument `piano`.
Im Programm steht kein Instrument — maßgebend sind diese Dateien (Entscheide
Patrik S110, I2 b und I3 a). Die Felder beschreibt das Konzept §4f.

**Sechs davon sind Drums** (`drum_kick`, `drum_tom`, `drum_snare`, `drum_clap`,
`drum_hat_closed`, `drum_hat_open`; S117, Stufe 11f): gerechnete Schläge neben
dem Kit, erzeugt mit `make_drums.py`, Fassung 6 — **Kick und Clap seit S118
(Stufe 16e) in Fassung 7**: sie nutzen die Reihe einer Schicht (Signal path),
der Kick mit Drive hinter der Envelope, der Clap mit Hochpass und Bandpass
hintereinander. Ein LumiViz vor S118 liest diese beiden nicht. Sie sind **für die Note C2
gebaut** — eine andere Note verschiebt den ganzen Schlag — und leiser als ein
Klang des Kits. Gegen das Kit gemessen, nicht gehört (Konzept §4h).

Seit S114 stehen die übrigen neunundvierzig in **Fassung 3**, und alle tragen `category`
(eine von neun Arten), `genres` und `description` — für das Fenster
„Instruments" (Konzept §4h, Entscheid F3 b). Die drei Angaben pflegt
**`instrument_katalog.py`**: Art und Beschreibung stehen dort in einer Tabelle
(nach Titel und Aufbau gesetzt, nicht nach Gehör), die Genres zählt es aus den
mitgelieferten Loops — ein Instrument trägt das Genre jedes Loops, in dem es
steht (Guitar und Violin stehen in keinem). Wer ein Instrument dazulegt, trägt
es dort ein und ruft `python asset/loop/instrument_katalog.py`;
`make_examples.py`, `make_genres.py` und `make_drums.py` rufen es von selbst. Eigene
Instrumente gehören nicht hierher: sie liegen im Benutzerordner der App unter
`instruments` und heißen im Loop `user:<name>` (Entscheid P2 a).

**Pegel (S139, Stufe 27c):** jedes mitgelieferte Instrument außer den Drums misst mit der
Standardprobe gleich laut (−13,4 LUFS, Loop_Konzept §4n). Das `level` dafür steht in
**`instrument_pegel.py`** (`PEGEL`, je Instrument alt und neu); `instrument_katalog.ergaenze`
setzt es, auch wenn ein Skript das Instrument neu schreibt; die Datei behält ihre Fassung. Wer ein
Instrument im Klang ändert: messen (`LUMIVIZ_PEGEL_DATEI=<datei.tsv>`, Fall „Loop Pegel:
Messung“ in `test_LoopPegel.cpp`), dann `python asset/loop/instrument_pegel.py --messung
<datei.tsv>`. Ein Loop-Skript, das neu läuft, schreibt seine Loops mit den alten
Muster-Lautstärken (Fassung unter 11) — danach `python asset/loop/instrument_pegel.py --loops`:
es zieht sie nach (Muster mal alt/neu, der Überschuss in den Fader des Kanals) und setzt
Fassung 11.

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

## effects_demo.lvloop

Das Beispiel `house_demo.lvloop` mit **Effekten an fünf Clips** (S113, Stufe
12b) — Fassung 3 der `.lvloop`. Muster, Instrumente und Lautstärken sind
dieselben; zum Vergleich beide nacheinander laden.

| Kanal | Clip | Effekt |
|---|---|---|
| Drums | Beat, Takt 1 bis 20 | keiner |
| Drums | Beat, Takt 21 bis 24 (dieselbe Basis) | Reverb: Size 0,4, Decay 1,2 s, Mix 20 % |
| Bass | Acid, Takt 17 bis 24 | Distortion: Drive 0,5, Tone 0,6, Mix 70 % |
| Keys | Piano stabs, Takt 5 bis 12 | Echo: punktierte Achtel, Feedback 0,45, Mix 30 % |
| Keys | Stabs, Takt 13 bis 24 | Reverb: Size 0,7, Decay 2,5 s, Mix 30 % |
| Lead | Arp, Takt 17 bis 24 | Echo: Achtel, Feedback 0,5, Mix 30 %; Reverb: Decay 1,8 s, Mix 20 % |

Von Hand aus `house_demo.lvloop` abgeleitet, nicht aus einem der
Erzeuger-Skripte. Die Werte sind **nach Rechnung gesetzt, nicht gehört**. Die
Summe erreicht höchstens 0,52 (`test_LoopInstrument.cpp` rechnet einen
Durchlauf durch den Klangerzeuger; die ersten vier Takte sind auf den Wert
gleich wie ohne Effekte). `LUMIVIZ_INSTRUMENT_WAV=<Ordner>` legt
`effects_demo.wav` ab.

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

## kits/ — die mitgelieferten Kits (S123, Stufe 21a)

Zehn Dateien `.lvkit` (Konzept §4l, Entscheide K1 a bis K9 a): Classic, 808,
909 House, Electro, Hand drums, Beatbox, Lo-fi, Reggae, Reggaeton, Strange.
Der Dateiname ist der Name im Loop: `909_house.lvkit` ist das Kit
`909_house`. Je Rolle ein Klang (`kit:`, `lib:` oder ein Instrument `drum_*`
auf C2) mit Volume und Pan; was in welchem Kit steckt, steht im Konzept §4l.
Erzeugt mit `kits/make_kits.py`; **Volume ist gemessen** — der Pegel jedes
Klangs steht im Skript (`PEGEL_DB`), gemessen mit `LUMIVIZ_KIT_PEGEL=1` in
`test_LoopKits.cpp`. Wer einen Klang tauscht, misst neu. Die Rock-Kits und
Jazz Brushes kommen in Stufe 21d.

## S123: mitgelieferte Samples, Instrumente aus Formen und Samples, neue Drums, neun Loops

Wunsch Patrik S123: mehr Vorlagen für Shape, Samples, Kits und Loops.
Entscheide 1 a (mitgelieferte Samples unter `samples/`, im Loop `lib:<name>`),
2 a (nur CC0, CC BY oder gemeinfrei, mit Nennung) und 3 a (Kits als eigene
Einheit — die Kits selbst kommen nach der Skizze).

### samples/

**Achtunddreißig Aufnahmen** aus der OLPC-Sammlung (One Laptop per Child,
archive.org, **CC BY 3.0**): Stimmen (Frauen: ha, hum, uh, aah, oo, ah,
gliss; Männer: aah, oo, la, hey), Beatbox (kick, snare, zwei Hi-Hats),
Drum-Maschinen und Schlagwerk (909, 808, 707, Tambourine, Djembe, Conga,
Darbuka, Kick, Snap, Rim) und Instrumente mit Ton (Harfe, Koto, Shakuhachi,
Hackbrett, Daumenklavier, Altsaxophon, Altflöte). Der Dateiname ist der Name
im Loop: `harp.wav` ist `lib:harp`. Mono, 16 Bit, 44100 Hz, Stille an den
Rändern weg, Spitze 0,89.

- **`samples/CREDITS.md`** nennt je Datei Urheber, Eintrag und Datei im ZIP —
  sie geht mit neben die Exe (`asset/Asset.cmake`), die Lizenz verlangt es.
  `THIRD_PARTY_NOTICES.md` führt die Samples.
- **`samples/make_samples.py <Ordner der Originale> <Clave_son_2-3.wav>`**
  schreibt alle neu und gibt die gemessene Tonhöhe aus. Die Originale liegen
  **nicht** im Repo (seit S128 unter `…\Visuals_Project\cmake\OLPC` und
  `…\cmake\CommonsClave`); ihre Adressen stehen im Skript.
- **S128 (Entscheid Patrik C1 a):** dazu `cowbell` und `cowbell_tip` aus
  TamTam Drum Kits (OLPC, CC BY 3.0) und `clave` — ein Schlag aus *Clave son
  2-3* der ESMUC auf Wikimedia Commons (**CC0**; in OLPC gibt es keine Clave,
  gesucht in allen 78 Einträgen). Achtundsiebzig Samples insgesamt.
- **Gemessen, nicht gehört:** `voice_male_aah` und `voice_male_oo` gleiten in
  1,5 s um rund fünfzehn Halbtöne nach unten, `voice_male_la` schwankt um
  ±0,7 — für gehaltene Töne taugen sie nicht, es gibt deshalb keinen
  Männerchor. Harfe und Hackbrett lassen Saiten mitklingen; ihre Tonhöhe steht
  im Dateinamen und ist am Spektrum bestätigt.

### Instrumente

| Skript | Was | Fassung |
|---|---|---|
| `make_shapes.py` | elf aus Formen (`shape_*`): Heart Pad, Rings Choir, Infinity Drone, Gear Bass, Moon Bass, Star Lead, Bolt Pluck, Flower Flute, Smiley Keys, Atom Bell, Cat | 9 |
| `make_sample_instruments.py` | dreizehn aus Samples: Female Choir, Hum Pad, Vocal Lead, Vocal Chop, Harp, Koto, Dulcimer, Thumb Piano, Shakuhachi, Alto Sax, Alto Flute, Angel Harp, Wind Voices — und fünf geschichtete Drums (Punch Kick, Fat Snare, Clap Stack, Hat Shake, Beatbox Kick) | 8 |
| `make_drums.py` | sechs weitere gerechnete Drums: 808 Kick, Low Tom, High Tom, Shaker, Ride, Crash | 6 und 7 |

Neunzig mitgelieferte Instrumente. Grundton und Fine tune der Samples sind
gemessen; die Schleifen gehaltener Klänge liegen auf aufsteigenden
Nulldurchgängen. Wo eine Stimme in den Ton schleift (Hum 300 ms, Ha 250 ms
Atem, Shakuhachi überbläst die ersten 150 ms), beginnt sie später (Sample
start). Smiley Keys und Atom Bell spielen je Kontur eine Schicht, mit Pan;
Rings Choir und Infinity Drone spielen Stereo XY — sie sind die ersten
mitgelieferten Instrumente, die nicht auf beiden Seiten gleich klingen.

### genres/ — neun Loops (`make_showcase.py`)

| Datei | Genre | BPM | Was zeigt er |
|---|---|---|---|
| `vocal_house.lvloop` | House | 124 | Vocal Chop, Female Choir, Vocal Lead über 909, 808 und 707 aus `samples/` |
| `angel_steps.lvloop` | Chillout | 90 | Angel Harp, Hum Pad, Kick, Rim, Snap, Shaker |
| `beatbox_groove.lvloop` | Hip-Hop | 92 | Beatbox-Samples, Beatbox Kick, „hey", Moon Bass, Smiley Keys |
| `desert_caravan.lvloop` | World | 100 | Darbuka, Djembe, Tambourine; Hackbrett, Koto, Shakuhachi in D-Hijaz |
| `zen_garden.lvloop` | Ambient | 70 | Koto, Wind Voices, Thumb Piano, Shakuhachi |
| `flute_tropics.lvloop` | Afrobeat | 108 | Djembe, Conga, Shaker; Thumb Piano, Alto Flute, Steel Drum |
| `sax_after_dark.lvloop` | Jazz | 84 | Alto Sax, Harp, Upright Bass, Ride, Rim |
| `shape_synthwave.lvloop` | Synthwave | 110 | Gear Bass, Heart Pad, Bolt Pluck, Star Lead; 808 Kick, Fat Snare, Crash |
| `geometry.lvloop` | Electronica | 122 | Atom Bell, Smiley Keys, Rings Choir, Infinity Drone, Cat; Punch Kick, Clap Stack, Ride, Toms |

Drum-Spuren nennen ein Sample (`lib:`), einen Klang des Kits oder ein
Instrument der Art Drums (Note C2). Nach Rechnung gesetzt, nicht gehört.
Hörproben: `LUMIVIZ_LOOP_WAV=<datei.lvloop oder Ordner>` mit
`LUMIVIZ_LOOP_WAV_ZIEL=<Ordner>` (`test_LoopWerte.cpp`).

## S124, Stufe 21d: akustische Drums, Besen, vier Kits, Brush Swing

### samples/ — siebenunddreißig Anschläge aus DrumGizmo (CC BY 4.0)

`samples/make_drumgizmo.py <ordner>` in drei Schritten: `verzeichnis` liest das
Inhaltsverzeichnis der beiden ZIPs (DRSKit 2.1, 2,80 GB; CrocellKit 1.1,
5,65 GB) über Teil-Abrufe, dazu die XMLs der Instrumente; `holen` lädt nur die
gewählten Anschläge (`WAHL`, 310,5 MB, Entscheide D1 a, D1b, D4 a); `schreiben`
mischt sie zu Stereo (D2 a) und lässt sie ganz — nur die Stille vorn fällt weg (D6 a). Die Originale liegen
nicht im Repo (S124: `…\Visuals_Project\cmake\DrumGizmo`). Nennung in
`samples/CREDITS.md`, Abschnitt *Acoustic drums — DrumGizmo*.

| Präfix | Kit | Was |
|---|---|---|
| `drs_*` (22) | DRSKit | Kick leise/mittel/voll, Snare mittel/voll, Rim, Hat mittel/voll/Fuß, offene Hat mittel/voll, Toms 1–3 mittel/voll, Ride leise/mittel, Crash links mittel/voll, Crash rechts mit dem Schaft voll |
| `crocell_*` (15) | CrocellKit | Kick links/rechts (Doppelpedal), Snare, Hat, offene Hat, Toms 1–4, Ride, Crash 1–3, China, Splash — alle voll |

### Instrumente

Drei gerechnete Besen (`make_drums.py`): `drum_brush_tap`, `drum_brush_slap`,
`drum_brush_sweep` — Rauschen durch Filter, nach Rechnung, nicht gehört.

### kits/ — vier Kits mehr (vierzehn)

Rock'n'Roll und Hardrock aus dem DRSKit (mittel bzw. voll), Heavy Metal aus dem
CrocellKit, Jazz Brushes mit den Besen und leisem DRSKit. Volume gemessen wie
in S123 (`make_kits.py`, `PEGEL_DB`); Hats und Becken der Stereo-Samples auf
Pan 0 — sie tragen ihr Bild aus den Overheads.

### genres/brush_swing.lvloop

Jazz, 132 BPM, Swing in Triolen (zwölf Schritte je Takt), Kit `jazz_brushes`
mit Rollen an den Spuren; Upright Bass als Walking Bass über Dm7 G7 Cmaj7 A7.

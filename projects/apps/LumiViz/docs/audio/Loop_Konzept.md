# LumiViz — Loop (Konzept)

> **Version:** 0.18.0
> **Datum:** 2026-10-11 (Session 110)
> **Typ:** Konzept — Entscheide L1 bis L8 gefallen (Patrik S108); gebaut sind
> Beleg, Kern (Fassung 2: Arrangement aus Kanälen und Clips), Kit und die
> Wiedergabe mit einem ersten Panel, das nur lädt und spielt (Stufen 2 bis
> 5b) — im Programm noch nicht gehört und nicht gesehen
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Composer_Konzept.md](../ui/Composer_Konzept.md) §4 (Uhr,
> Tonführung, Mixer) · [Audio_System.md](Audio_System.md)
> **Sprache:** Deutsch

---

## 1. Ziel

Ein **Loop** ist ein kleines Arrangement in Takten, gebaut im **Loop-Panel**:
Kanäle untereinander, darin Clips. Ein Clip spielt ein Muster, das in einem
eigenen Editor-Fenster entsteht — für Drums ein Schritt-Raster nach Art des
Channel Rack von FL Studio. Was dort entsteht, liegt als Datei `.lvloop` vor
und lässt sich als **Clip in einen Audio-Kanal des Composers** legen.

Vorgaben (Patrik S108):

- **Loop-Panel:** Kanäle anlegen wie im Composer. Im Kanal entsteht ein
  leerer **Drumkit-Clip**, zu Beginn in der kürzesten Länge von 4 Takten.
  **Edit** öffnet seinen Editor; wird das Muster dort länger, wird es der
  Clip auch.
- **Sequenz:** Clips verlängern, kürzen, kopieren, verschieben. Ein Clip ist
  immer ein **Vielfaches seines Musters** lang.
- **Kopien bleiben verbunden** (L8 a): der Rechtsklick auf einen Clip bietet
  an, **die Basis zu bearbeiten** (gilt für alle verbundenen Clips) oder den
  Clip **in eine neue Basis umzuwandeln** (das Muster wird kopiert, das
  Original bleibt).
- **Später weitere Arten von Clips:** ein **Instrument-Clip**, dessen Editor
  je Zeile eine Tonhöhe hat und in dem jede Note ihre eigene Länge trägt —
  für Melodien (Piano und anderes).
- **Klangquelle der Drums:** ein mitgeliefertes Kit **und** eigene Samples je
  Spur.
- **Raster:** einstellbar — Schläge je Takt, Schritte je Schlag, Lautstärke
  je Schritt, Swing.
- **Tempo:** einstellbar, in BPM, für den ganzen Loop.
- **Als Clip im Composer:** die `.lvloop` spielt ihr ganzes Arrangement; wird
  der Clip länger gezogen, wiederholt es sich bis zum Clip-Ende.
- **Zweck:** eine Erweiterung für **eigene Songs**. Drums sind der Anfang;
  später Instrumente, Funktionsgeneratoren, Modulation, möglicherweise MIDI.
- **Allein spielbar:** das Panel hat ein eigenes Play.
- **Ablage:** die `.lvloop` ist **das ganze Arrangement** mit allen
  Einstellungen aus den Editoren — nicht ein einzelner Clip.

---

## 2. Was es schon gibt

| Baustein | Stand | Nutzen für den Loop |
|---|---|---|
| **Mixer der Engine** (Naht N12) | `createMixer`, `mixerAddSource` — eine Quelle ist heute eine **Datei** (`BassEngine.cpp`) | der Loop wird eine zweite Art Quelle |
| **Tonführung** | die Uhr führt, Quellen laufen frei, nachgesetzt wird ab 80 ms (`Tonfuehrung.hpp`) | hängt für `.lvloop` den Klangerzeuger ein statt einer Datei |
| **Clip** | ein Typ für Bild und Ton, verweist auf eine Datei; die Endung sagt, was er ist (`Komposition.hpp`) | ein Loop-Clip braucht keinen neuen Kanal-Typ und kein neues Feld |
| **Pfade in `.lvcomp`** | relativ unter dem Ordner der Datei, sonst absolut; „File not found" beim Laden (`Komposition.cpp`) | dieselbe Regel für Samples in `.lvloop` |
| **Spektrum des Tonführenden** (Naht N13) | die Visuals lesen vom Player oder vom Mixer des Composers (`MainWindow.cpp`) | wächst auf drei: dazu der Loop |

Nicht vorhanden: Sequencer, Sampler, ein Tempo oder Taktraster (die
Komposition kennt nur `rasterSekunden`), eigener Code im Ton-Thread.

---

## 3. Entscheide

| Nr. | Entscheid (Patrik S108) |
|---|---|
| **L1 a** | **Lebender Klangerzeuger als Quelle im Mixer.** Der Loop rechnet seinen Ton blockweise im Ton-Thread, jeder Schlag sitzt auf das Sample genau, eine Änderung im Panel gilt ab dem nächsten Block. Der Rechenkern ist eine reine Funktion „Muster plus Stelle ergibt Samples". |
| **L2 b** | **Das BPM steht in der `.lvloop`.** Der Clip spielt die Datei ab wie einen eigenen Song; der Composer kennt kein Tempo. Zwei Loops laufen zusammen, wenn in beiden dasselbe BPM steht. |
| **L3 neu** | **Die Datei ist ein Arrangement in Takten:** Kanäle, darin Clips; ein Clip spielt ein Muster und ist immer ein Vielfaches davon lang. Das Muster trägt sein Kit und seine Schritte. Der Clip im Composer spielt das ganze Arrangement und wiederholt es; der `Clip` des Composers und die `.lvcomp` bleiben unverändert. (Löst L3 c vom selben Tag ab: „Kit, Muster und Abfolge als Liste".) |
| **L8 a** | **Kopien eines Clips bleiben verbunden.** Das Muster liegt einmal in der Datei, die Clips nennen es beim Namen. Der Rechtsklick auf einen Clip bietet beides: die Basis bearbeiten (gilt für alle) oder den Clip in eine neue Basis umwandeln (das Muster wird kopiert, das Original bleibt). |
| **L4 a** | **Wer spielt, hat den Ton — Rangfolge Composer, Loop-Panel, Player.** Play im Loop-Panel hält den Player an und gibt ihn bei Stop zurück; die Visuals reagieren auf den Loop. Hat der Composer den Ton, schweigt das Loop-Panel. |
| **L5 a** | **Das mitgelieferte Kit wird gerechnet**, nicht bezogen: ein Skript erzeugt die WAV-Dateien, sie liegen unter `asset/`. Jede Spur kann stattdessen eine eigene Sample-Datei laden. |
| **L7 a** | **Play im Loop-Panel ist gesperrt, solange der Composer den Ton hat**, und sagt, warum. Liegt der Loop als Clip in der Komposition, ist jede Änderung über den Composer zu hören. Spielte das Loop-Panel gerade allein, hält es an. |
| **L6 a** | **Luft nach oben statt Begrenzer.** Die Spuren addieren sich, der Kern begrenzt nicht — wie zwei Songs im Mixer des Composers. Ein neuer Loop beginnt mit Lautstärke 0,5; das Panel bekommt eine Anzeige, die rot wird, sobald die Summe über 1 geht (Stufe 6). |
| **W1 b** (S109) | **Das Loop-Panel und der Composer teilen sich die Achse.** Maßstab, Zoom, Rollen, Mausrad, Schwenken und Mitlaufen liegen in einer gemeinsamen Basis (`lumi::ui::Achse`, `UI/widgets/Achse.hpp`), herausgezogen aus der `Zeitachse` des Composers; die Achse des Loop-Panels erbt davon und rechnet in Takten. Grund (Patrik): kurz sind nur die Drums — Instrumente werden länger, und kommt Gesang dazu, sind es mehrere Minuten; dafür braucht das Panel den vollen Zoom. |
| **T1 a** (S109) | **Ein Loop-Clip bleibt an der Uhr wie ein Song.** Gesetzt wird er beim Einstieg, nach einem Sprung der Uhr und bei mehr als 80 ms Abweichung — verglichen wird die Stelle des Klangerzeugers selbst. So folgen alle Quellen einem Herrn: nach einem Aussetzer landen Songs und Loops wieder an derselben Uhr. Gemessen (§6a): der Abstand schwankt um wenige Millisekunden, aus Drift wären 80 ms erst nach 40 bis 90 Minuten erreicht. |
| **T2** (S109) | **Ein Loop-Clip zählt als laufender Song**, und die `.lvloop` trägt dasselbe Tag-Schema wie ein Song (Titel, Künstler, Album, Jahr, Genre, Nummer, Songtext) — das Player-Panel, die Text-Vorlagen und Karaoke lesen dort, was sie bei einer MP3 aus der Datei lesen. |
| **I1 c** (S110) | **Der Klang eines Instrument-Clips kommt aus beiden Quellen: gerechnet oder aus einem Sample — zuerst gerechnet.** Ein gerechnetes Instrument ist ein Satz Einstellungen für einen Klangerzeuger: Schwingung (der Funktionsgenerator), Hüllkurve, Filter, Vibrato, Verstimmung, dazu eine gezupfte Saite als eigene Art. Ein Instrument aus einem Sample ist eine Tondatei, die je Note in der Tonhöhe umgerechnet wird. Die Piano-Rolle ist für beide dieselbe. Der **Instrument-Creator** ist der Editor der gerechneten Art; ein Funktionsgenerator allein ist nur deren erster Baustein. |
| **I2 b** (S110) | **Ein eigenes Instrument ist eine eigene Datei `.lvinst`** — abgelegt wie die Klänge des Kits (Patrik: „wie die Kit"). Das Muster nennt sie, wie eine Spur ihren Klang nennt: den Namen eines mitgelieferten Instruments oder den Pfad einer Datei, relativ unter dem Ordner der `.lvloop`, sonst absolut. Fehlt die Datei, schweigt das Muster, und der Bericht sagt es. (Patrik hatte zuerst a gesagt und es im selben Zug berichtigt.) |
| **I3 a** (S110) | **Auch die sieben mitgelieferten Instrumente sind Dateien**: `asset/loop/instruments/<name>.lvinst`, wie die Klänge unter `asset/loop/kit`. Das Programm liest den Ordner; im Code steht kein Instrument mehr. Ein Muster nennt ein mitgeliefertes beim Namen (`piano`), ein eigenes beim Pfad. Verworfen: b — die mitgelieferten bleiben im Code (sie klängen auch ohne den Ordner `asset`, aber es gäbe zwei Arten von Instrumenten). **Folge:** fehlt `asset/loop` neben der Exe — wie heute im Release-Paket —, schweigen alle Instrument-Clips; dieselbe offene Aufgabe wie beim Kit. |

### 3.1 Verworfen, mit Grund

- **Ein eigenes Instrument in der `.lvloop` selbst**, mit Export und Import
  (I2 a, meine Empfehlung): der Loop bliebe allein spielbar — aber ein
  Instrument lebte nicht von selbst in mehreren Loops, und es läge anders ab
  als die Klänge des Kits.
- **Eine Sammlung des Programms** (I2 c): derselbe Loop klänge auf zwei
  Rechnern verschieden, ohne dass die Datei es verrät.
- **Ein Loop-Clip wird nie wegen Abweichung nachgesetzt** (T1 b): kein
  Stolperer aus Drift, und zwei Loop-Clips blieben auf den Abtastwert genau
  zueinander. Aber ein Song daneben wird nach einem Aussetzer nachgesetzt und
  der Loop nicht — die zwei blieben für den Rest der Komposition versetzt.
- **Nachsetzen erst ab 250 ms** (T1 c): seltener, aber dreimal so groß.
- **Eigene Achse für das Loop-Panel, der Composer bleibt unberührt** (W1 a):
  Geometrie, Zoom und Rollen entstünden ein zweites Mal — tragbar nur,
  solange ein Loop kurz bleibt.
- **Die `Zeitachse` selbst benutzen und den Loop in eine `Komposition`
  übersetzen** (W1 c): ein Clip hat dort eine freie Länge in Millisekunden,
  der Loop verlangt ganze Vielfache eines Musters; jede Geste wäre
  zurückzuübersetzen, und die Achse bekäme Schalter für alles, was der Loop
  nicht hat (Master-Zeilen, Kurven, Stoß-Arten).

- **Schläge einzeln aus dem Zeitgeber des Composers auslösen** (L1 c): der
  Zeitgeber läuft mit 33 ms im UI-Thread; bei 120 BPM dauert ein Sechzehntel
  125 ms. Gerechnet, nicht gemessen.
- **Vorab in einen Puffer rechnen** (L1 b): der kleinere Schritt, aber ohne
  Weg zu live gespieltem MIDI. Der Rechenkern aus L1 a rechnet auch in einen
  Puffer — für Tests und einen späteren Export.
- **Loop in die laufende Summe des Players mischen** (L4 c): der Player läuft
  nicht über einen Mixer; denselben Fall deckt der Composer mit zwei
  Audio-Kanälen.
- **Freies Sample-Paket aus dem Netz** (L5 b): Lizenz je Paket zu prüfen;
  bleibt offen, falls das gerechnete Kit nicht gefällt.
- **Weiches Begrenzen im Kern** (L6 b): färbt den Klang ungefragt, und die
  Summe aus Loop und Song kann im Mixer trotzdem über 1 gehen. Als
  abschaltbarer Effekt später denkbar.
- **Selbsttätig angleichen** (L6 c): jede neue Spur machte den ganzen Loop
  leiser.

### 3.2 Zurückgestellt (Patrik S108)

- **Tempo der Komposition** (L2 a): später zuschaltbar, an der Komposition.
- **Instrument-Clip** mit Piano-Rolle und einem Klangerzeuger für Töne: eine
  eigene Stufe nach den Drums.
- Funktionsgeneratoren, Modulation, MIDI.

### 3.3 Noch offen

- **Das Kit ist klein** (Patrik S108: klingt gut, ist aber noch eine kleine
  Sammlung): weitere Klänge mit demselben Skript — welche, nennt Patrik.
- **Der Drum-Editor** als eigenes Fenster: die Skizze ist gebilligt (Patrik
  S109), gebaut in Stufe 6b (§4d) — im Programm noch nicht gesehen. Mit der
  Skizze entschieden: das Panorama als zweiter Schieber in jeder Spurzeile;
  lange Muster rollen, sie stauchen nicht.
- **Das Loop-Panel mit Kanälen und Clips:** die Skizze ist gebilligt
  (Patrik S109), gebaut in Stufe 6a (§4c) — im Programm noch nicht gesehen.
- **Der Audio Browser zeigt auch `.lvloop`** (Wunsch Patrik S108): gebaut in
  Stufe 6c — die Loops des Ordners stehen hinter den Songs, der Doppelklick
  lädt ins Loop-Panel (`LoadLoopEvent`; das Panel fragt nach Ungespeichertem
  und kommt nach vorn). Ein Loop lässt sich schon ziehen (als Datei-URL);
  **annehmen** wird ihn erst der Audio-Kanal des Composers (Stufe 7) —
  Playlist, Player und Visualisierung nehmen ihn nicht, er ist kein Song.

---

## 4. Datei `.lvloop` und Rechenkern (gebaut)

Gebaut in `include/loop` und `src/loop` (`Loop.hpp`, `Rechenkern.hpp`),
Namensraum `lumi::loop`. **Fassung 2** (S108): das Arrangement aus Kanälen
und Clips, das Kit im Muster. Fassung 1 (Kit am Loop, Abfolge als Liste) gab
es nur einen Tag; sie wird nicht gelesen. JSON, Schlüssel englisch wie in
`.lvcomp`:

```json
{
  "format": "lumiviz-loop",
  "version": 2,
  "bpm": 120,
  "beatsPerBar": 4,
  "stepsPerBeat": 4,
  "swing": 0,
  "volume": 0.5,
  "patterns": [
    {
      "name": "Grund",
      "kind": "drumkit",
      "bars": 4,
      "tracks": [
        { "name": "Kick",    "sound": "kit:kick",         "volume": 1,   "pan": 0,    "mute": false },
        { "name": "My Clap", "sound": "samples/clap.wav", "volume": 0.8, "pan": -0.2, "mute": false }
      ],
      "rows": [
        [1, 0, 0, 0, 1, 0, 0, 0, "… 64 Schritte, je Spur eine Zeile"],
        [0, 0, 0, 0, 1, 0, 0, 0, "…"]
      ]
    }
  ],
  "channels": [
    { "name": "Drums", "enabled": true, "clips": [
        { "pattern": "Grund", "start": 0, "repeat": 3 },
        { "pattern": "Grund", "start": 16, "repeat": 1 } ] }
  ]
}
```

| Feld | Bedeutung | Grenzen |
|---|---|---|
| `bpm` | Tempo des ganzen Loops | 20 bis 400 |
| `beatsPerBar`, `stepsPerBeat` | Raster des ganzen Loops; ihr Produkt sind die Schritte je Takt | je 1 bis 16 |
| `swing` | verzögert jeden zweiten Schritt eines Schlags, bei 1 um einen halben Schritt | 0 bis 1 |
| `volume` | Lautstärke des ganzen Loops, vor dem Pegel des Clips im Composer; Vorgabe 0,5 (L6 a) | 0 bis 1 |
| `tags` | die Tags des Loops, dasselbe Schema wie bei einem Song (T2): `title`, `artist`, `album`, `year`, `genre`, `track`, `lyrics` (Songtext, darf LRC-Zeitmarken tragen). Alle freiwillig; der Block fehlt, wenn keins gesetzt ist. Ohne Titel gilt der Dateiname. | — |
| `patterns[]` | die Muster („Basen"): Name, Art, Länge und alles, was ihr Editor einstellt | — |
| `patterns[].kind` | Art des Musters, bestimmt den Editor: `drumkit` oder `instrument` (Noten statt Spuren, §4f) | — |
| `patterns[].bars` | Länge in Takten | 4 bis 64 |
| `patterns[].tracks[].sound` | `kit:<Name>` = Klang aus dem mitgelieferten Kit; sonst ein Pfad, relativ zur Datei, wenn das Sample unter ihrem Ordner liegt, sonst absolut | — |
| `patterns[].tracks[].volume`, `pan`, `mute` | je Spur; Panorama -1 (links) bis 1 (rechts): in der Mitte beide Seiten voll, zur Seite fällt die andere ab | 0 bis 1, -1 bis 1 |
| `patterns[].rows` | je Spur eine Zeile, je Schritt eine Lautstärke, 0 = aus | 0 bis 1 |
| `channels[]` | Kanäle mit Name und Schalter; ein Kanal nimmt Clips jeder Art auf | — |
| `channels[].clips[]` | `pattern` = Name des Musters, `start` = Takt (0 = der erste), `repeat` = wie oft hintereinander | `repeat` ab 1 |

Die Datei schreibt die Schritte einer Spur auf **eine Zeile** (Qt setzte
sonst jede Zahl auf eine eigene).

**Verbunden** (L8 a): Clips nennen ihr Muster beim Namen. Zwei Clips mit
demselben Namen spielen dasselbe Muster; wer es ändert, ändert beide. „In
eine neue Basis umwandeln" kopiert das Muster unter einem neuen Namen.

**Ein Durchlauf** endet am Ende des letzten Clips, gleich ob sein Kanal an
ist; Lücken sind Stille. Ohne Clips ist er ein Takt lang.

**Beim Laden** (`Loop::richte`): Werte kommen in ihre Grenzen, jede Zeile auf
volle Länge, ein Muster ohne Namen heißt „Pattern N"; Clips werden nach Start
sortiert, bei Überlappung im selben Kanal beginnt der spätere, wo der frühere
endet. Im Bericht steht: `File not found: …` (Sample fehlt, die Spur bleibt
stumm), `Pattern not found: …` (ein Clip ohne Muster — er wird entfernt),
`Pattern name used twice: …` (das erste gilt), `Unknown pattern kind …` (ein
Muster einer Art, die diese Fassung nicht kennt — es bleibt weg).

**Rechenkern:**

- `plane(loop, rate)` übersetzt den Loop in einen `Ablauf`: je **Stimme** die
  Schläge eines Durchlaufs mit Stelle in Abtastwerten und Lautstärke links
  und rechts. Eine Stimme ist eine Spur eines Musters in einem Kanal. Der
  Ablauf nennt die Klänge, die er braucht (`klangNamen`), jeden einmal.
  Ändert sich der Loop, entsteht ein neuer Ablauf.
- `rechne(ablauf, klaenge, von, frames, aus)` füllt einen Block. **Ohne
  Gedächtnis:** derselbe Bereich ergibt dieselben Werte, gleich in welchen
  Blöcken und von wo gerechnet wird — so lässt sich die Stelle frei setzen
  (§6). Kein Speicher, keine Datei: darf im Ton-Thread laufen.
- Die Klänge bekommt der Kern fertig in der Abtastrate des Ablaufs
  (`Klang`, Stereo), je Name einen.

**Von mir gesetzt, nicht entschieden:**

- Ein neuer Schlag schneidet den klingenden **derselben Stimme** ab, mit
  einer Blende über 64 Abtastwerte (`kSchnittFrames`) — wie eine
  Drum-Maschine; auch über die Wiederholungen eines Clips und über Clips
  desselben Musters im Kanal hinweg. Verschiedene Stimmen addieren sich.
- Vor dem Abtastwert 0 gibt es nichts: der erste Durchlauf beginnt ohne
  Ausklang eines früheren; ab dem zweiten reicht der Ausklang über die Grenze.
- Die Summe wird nicht begrenzt (L6 a); sie kann über 1 liegen.
- Ein Muster ist höchstens 64 Takte lang.
- Ein Kanal hat Name und Schalter, noch keine eigene Lautstärke.

**Bewusst nicht drin** (das Format trägt eine Versionsnummer): Tonhöhe je
Spur oder Schritt, Abschneiden zwischen Spuren (offene Hi-Hat),
Wahrscheinlichkeit je Schritt, Solo, Effekte, verschiedene Schrittzahlen je
Spur, ein eigenes Raster je Muster.

---

## 4a. Wiedergabe: Naht, Klangerzeuger, Kit (gebaut, Stufe 5a)

- **Naht in der Engine** (`IAudioEngine.hpp`): `IAudioGenerator` mit
  `render(out, frames)` — im Ton-Thread gerufen; `mixerAddGenerator(mixer,
  generator)` hängt ihn **angehalten** in den Mixer, mit
  `BASS_MIXER_CHAN_NORAMPIN`; angehalten, fortgesetzt und entfernt wird er wie
  jede Quelle (`setSourcePaused`, `mixerRemoveSource`). `decodeStereo(datei,
  rate, maxSekunden)` dekodiert eine Datei ganz, Stereo, auf die Abtastrate
  umgerechnet.
- **`Klangerzeuger`** (`loop/Klangerzeuger.hpp`): hält ein **Programm**
  (übersetzter Loop und seine Klänge, unveränderlich) und die **Stelle**.
  Ein neues Programm gilt ab dem nächsten Block. Ändert sich das Tempo, bleibt
  die Stelle im Takt (auf die neue Schrittlänge umgerechnet). Der Ton-Thread
  wartet nie auf das Panel; das abgelöste Programm wird nicht im Ton-Thread
  freigegeben.
- **`Kit`** (`loop/Kit.hpp`): `kit:<Name>` wird zu
  `asset/loop/kit/<Name>.wav`, der Ordner vom Programmordner aufwärts gesucht
  (Anker `asset/loop/README.md`, wie die anderen Asset-Ordner); erlaubt sind
  a–z, 0–9 und `_`. `ladeKlaenge` lädt je Spur ihren Klang, höchstens 30 s;
  was sich nicht laden lässt, steht als `Sound not loaded: …` im Bericht, die
  Spur schweigt.

**Gemessen** (S108, ein Lauf): der Klangerzeuger stand nach 1210 ms bei
1210 ms Ton; `kick.wav` kommt bei 44100 Hz mit 22050 Abtastwerten an, bei
48000 Hz mit 24000.

**Nicht geprüft:** dass `mixerRemoveSource` wartet, bis ein laufender Rückruf
fertig ist — der Test sieht nur, dass danach niemand mehr ruft. Das Kit im
Release-Paket: `asset/` liegt dort nicht neben der Exe.

---

## 4b. Der Loop allein in der App (gebaut, Stufe 5b)

- **`Spieler`** (`loop/Spieler.hpp`): übernimmt den Ton für das Loop-Panel —
  ein eigener Mixer, darin der Klangerzeuger. **Rangfolge Composer,
  Loop-Panel, Player** (L4): führt der Composer den Ton, lehnt `starte` ab,
  und ein laufender Loop hält im nächsten Takt an. Solange der Loop spielt,
  schweigt der Player; beim Anhalten spielt er weiter, wenn er vorher spielte.
  Lautstärke, Mute und Pegel des Players gelten für den Mixer des Loops.
- **Die Visuals** lesen Spektrum und Wellenform vom Mixer des Loops, solange
  er spielt (`MainWindow.cpp`, neben der Composer-Naht N13;
  `lumi::loop::tonMixer()`).
- **Panel „Loop"** (`loop/LoopPanel.hpp`, View → Panels → Loop): eine
  Datei-Zeile mit **Load…**, **Play / Stop**, eine Zeile mit BPM, Spuren,
  Takten und Dauer, die Stelle als „Bar · Step", darunter der Bericht. Der
  zuletzt geladene Loop steht nach dem Neustart wieder im Panel; gespielt
  wird nicht von selbst. Führt der Composer den Ton, ist Play gesperrt und
  sagt, warum (L7 a).
- **Play beginnt immer von vorn.** Ein geänderter Loop gilt ab dem nächsten
  Block; die Klänge werden nur neu geladen, wenn sich die Klänge der Spuren
  ändern.

**Von mir gesetzt, nicht entschieden:**

- Startet jemand den Player, während der Loop spielt, hält der Loop ihn im
  nächsten Takt wieder an (wie der Composer es tut) — der Loop hört nicht auf.
- Übernimmt der Composer, während der Loop spielt, spielt der Player nach dem
  Ausschalten des Composers nicht von selbst weiter.
- Der Spieler hängt die Playlist nicht aus (der Composer tut es): ein
  angehaltener Player wechselt den Titel nicht.

---

## 4c. Das Loop-Panel als Arrangement (gebaut, Stufe 6a)

**Aufbau** (Skizze gebilligt, Patrik S109):

- **Datei-Zeile:** Name der Datei, mit Stern bei Ungespeichertem
  („untitled", solange nie gespeichert), **New**, **Load…**, **Save**,
  **Save As…**. New, Load und das Beenden der App fragen nach, wenn
  Ungespeichertes verloren ginge — **Save / Discard / Cancel**, dieselbe
  Frage wie im Composer (`LoopPanel::darfErsetzen`; das Hauptfenster fragt
  über `loop/Offen.hpp`, `darfBeenden`). Save schreibt in die Datei des
  Loops, bei „untitled" mit dem Dialog von Save As.
- **Abspiel-Zeile:** **Play / Stop**, **BPM** als Feld, die Zeile mit Kanälen,
  Takten und Dauer, die Stelle als „Bar · Step", **Fit** (auch Ctrl+0).
- **Achse** (`loop/Taktachse.hpp`): Lineal in Takten, Kanäle als Zeilen, Clips
  als Blöcke, darunter eine Zeile „New channel". Zoom, Rollen, Mausrad,
  Schwenken und Mitlaufen wie im Composer (W1 b, Basis `lumi::ui::Achse`); die
  Zeilenhöhe merkt die App.
- **Clip-Zeile:** Basis, Takte, Wiederholungen und wie viele Clips die Basis
  spielen.

**Bedienung:**

| Geste | Wirkung |
|---|---|
| Rechtsklick auf eine freie Stelle im Kanal | **New drumkit clip** — eine neue, leere Basis „Pattern N", 4 Takte; grau, wenn dort keine 4 Takte frei sind. Dazu **Add channel** und **Zoom** |
| Rechtsklick auf einen Clip | **Edit base**, **Make new base and edit** (beide öffnen den Editor, §4d), **Duplicate**, **Rename base…**, **Delete** |
| Doppelklick auf einen Clip | wie **Edit base** |
| Rechtsklick auf einen Kanalnamen | **Rename channel…**, **Turn channel off / on**, **Delete channel** |
| Clip ziehen | verschieben in ganzen Takten, auch in einen anderen Kanal |
| Kante eines Clips ziehen | verlängern und kürzen in ganzen Mustern; links bleibt das Ende stehen |
| Entf, Ctrl+D, Esc | Clip löschen, verbundene Kopie anlegen, einen Zug abbrechen |

Jede Änderung läuft über den CommandBus der App (**Undo / Redo**) und gilt
beim Spielen ab dem nächsten Block. Der Stern vergleicht mit dem Stand der
Datei: wer zurück auf diesen Stand geht, hat nichts Ungespeichertes mehr.

**Regeln des Arrangements** (`loop/Bearbeiten.hpp`, reine Funktionen):

- Clips liegen auf ganzen Takten und überlappen sich im Kanal nicht.
- **Verschieben:** ist am Wunsch kein Platz, rückt der Clip an die nächste
  freie Kante eines Clips, der im Weg liegt — davor oder dahinter, was näher
  liegt. Er kann so über einen Clip hinwegspringen.
- **Kanten:** die Länge rastet auf das nächste Vielfache des Musters,
  mindestens eins, höchstens bis zum Nachbarn.
- **Duplicate:** die Kopie liegt hinter dem Clip, an der ersten Stelle, an die
  sie passt, und bleibt verbunden (L8 a).
- **Delete:** spielt danach kein Clip mehr die Basis, geht sie mit — sie wäre
  im Panel nicht mehr zu erreichen. Dasselbe beim Löschen eines Kanals.
- **Rename base:** gilt für alle Clips der Basis; ein vergebener Name wird
  abgelehnt.

**Von mir gesetzt, nicht entschieden:**

- **Farbe je Basis** statt je Kanal: verbundene Kopien tragen eine Farbe
  (zwölf Töne, die Farbreihen des Composers). In der Skizze gebilligt.
- Kein Schalter Edit: das Panel ist immer bearbeitbar.
- Ein Klick ins Lineal setzt nichts — Play beginnt immer von vorn.
- Beim Spielen blättert der Ausschnitt seitenweise mit der Abspielmarke; einen
  Schalter dafür gibt es nicht.
- Ein Tick der Achse ist der 960. Teil eines Takts; der feinste Maßstab sind
  240 Pixel je Takt.
- Die Achse zeigt mindestens 16 Takte und hinter dem Ende ein Viertel Luft.
- Ein neuer Kanal heißt „Channel N", New legt einen an.
- Die Rückfrage bietet seit S110 auch vor New und Load **Save** an (vorher
  nur Discard und Cancel) — angeglichen an den Composer, als die Frage beim
  Beenden dazukam.

**Nicht drin:** Kanäle am Namen umsortieren; Kopieren und
Einfügen über die Ablage; mehrere Clips zugleich wählen; ein Tooltip für
Namen, die nicht in ihre Zeile passen.

---

## 4d. Der Drum-Editor (gebaut, Stufe 6b)

Ein eigenes Fenster (`loop/DrumEditor.hpp`), geöffnet über **Edit base** am
Clip oder mit einem Doppelklick auf ihn. Es gibt **ein** Editor-Fenster: eine
andere Basis wechselt in ihm.

**Aufbau** (Skizze gebilligt, Patrik S109):

- **Kopf:** die Basis und wie viele Clips sie spielen; **Bars** (4 bis 64);
  **Play pattern**; **Peak**.
- **Grid of the whole loop:** Beats per bar, Steps per beat, Swing — sie
  gelten für alle Muster des Loops.
- **Spuren:** je Spur Stumm (M), Name, Klang (ein Klang des Kits oder
  „Sample file…"), Lautstärke, Panorama — und rechts ihre Schritte, jeder
  zweite Schlag heller, die Takte im Kopf. Lange Muster rollen.
- **Step volume:** unter den Spuren die Lautstärke je Schritt der
  angeklickten Spur, als Balken.

**Bedienung:**

| Geste | Wirkung |
|---|---|
| Klick auf einen Schritt | an (volle Lautstärke) oder aus |
| Mit gedrückter Taste weiterziehen | die überfahrenen Schritte derselben Spur bekommen denselben Zustand; gemeldet wird einmal, beim Loslassen |
| In der Zeile Step volume klicken oder ziehen | die Lautstärke des Schritts; ganz unten ist er aus, ein freier Schritt geht dabei an |
| Klick in eine Spurzeile | wählt die Spur (ihre Lautstärken stehen unten) |
| Rechtsklick auf eine Spur | **Rename track…**, **Remove track** |
| **Add track** | eine Spur „Track N" mit dem ersten Klang des Kits, den das Muster noch nicht spielt |
| **Play pattern** | spielt nur dieses Muster im Kreis; Play im Loop-Panel spielt wieder den ganzen Loop |

Jede Änderung läuft über dasselbe Undo wie im Loop-Panel und gilt beim
Spielen ab dem nächsten Block. Ein Schieber meldet beim Loslassen.

**Make new base and edit** (L8 a): das Muster wird unter „<Name> 2" (der
ersten freien Zahl) kopiert, der Clip spielt die Kopie, der Editor zeigt sie.
Spielt nur dieser eine Clip die Basis, öffnet sich einfach ihr Editor.

**Regeln** (`loop/Bearbeiten.hpp`):

- **Bars:** länger hängt Stille an, kürzer schneidet hinten ab (Undo holt es
  zurück). Die Clips der Basis werden mit ihr länger; ein Clip dahinter, in
  den einer hineinwüchse, rückt nach hinten.
- **Raster:** jeder Schritt bleibt an seiner Stelle im Takt, wo das neue
  Raster sie kennt. Ein Schlag, den es nicht mehr gibt, und ein Schritt, der
  zwischen zwei neue fällt, gehen verloren.
- **Peak** (L6 a): der größte Betrag der Summe, seit Play gedrückt oder
  zuletzt etwas geändert wurde — vor der Lautstärke des Players, gemessen im
  Klangerzeuger. Über 1 steht er rot. Ohne Wiedergabe steht ein Strich.
- Wird die Basis im Arrangement umbenannt, folgt der Editor ihr; gibt es sie
  nicht mehr (ihr letzter Clip gelöscht, ein anderer Loop geladen), schließt
  er sich.

**Von mir gesetzt, nicht entschieden:**

- Die Lautstärke je Schritt steht in einer eigenen Zeile unten, nicht im
  Schritt selbst; im Raster ist die Helligkeit des Schritts seine Lautstärke.
- „Play pattern" spielt einen Loop aus einem Kanal mit einem Clip des
  Musters; die Abspielmarke läuft dann im Editor, nicht im Arrangement.
- Der Editor ist ein Werkzeugfenster über dem Hauptfenster; seine Größe und
  Lage merkt die App **nicht**.
- Die Spitze misst nur, was gerade spielt: bei „Play pattern" das eine
  Muster, nicht den ganzen Loop.

**Nicht drin:** Spuren umsortieren; ein Schritt zum Vorhören beim Klick;
Kopieren von Takten; mehrere Editor-Fenster.

---

## 4e. Der Loop als Clip im Composer (gebaut, Stufe 7)

Eine `.lvloop` liegt in einem **Audio-Kanal** des Composers wie ein Song: aus
dem Audio Browser hineingezogen, verschoben, an den Kanten gezogen, mit
Pegelkurve und Blenden. `Clip` und `.lvcomp` sind unverändert (L3) — was ein
Clip ist, sagt die Endung seiner Datei (`composer::istLoop`).

- **Länge:** ein neuer Loop-Clip ist einen Durchlauf lang. Er hat kein Ende:
  länger gezogen wiederholt er sich, bis der Clip endet. Er trägt deshalb nie
  eine Song-Länge (`quellMs` 0); die linke Kante verschiebt den Einsatz im
  ersten Durchlauf.
- **Im Mixer** (`composer/Tonfuehrung`): je Loop-Clip ein eigener
  Klangerzeuger als Quelle, zwei Sekunden vor dem Einsatz angehalten
  eingehängt; seine Klänge werden dabei geladen. Der Pegel des Clips ist die
  Lautstärke der Quelle, wie bei einem Song.
- **An der Uhr** (T1 a): gesetzt wird die Stelle des Klangerzeugers selbst —
  auf den Abtastwert genau, im Kreis des Durchlaufs gerechnet.
- **Laufender Song** (T2): der Loop meldet sich mit den Tags seiner Datei;
  seine Länge ist ein Durchlauf, seine Stelle die im laufenden Durchlauf.
- **Mit dem Loop-Panel verbunden** (L7 a, `loop/Offen.hpp`): hat das Panel
  dieselbe Datei offen, spielt der Clip dessen Fassung — auch die
  ungespeicherte — und folgt jeder Änderung ab dem nächsten Takt. Ändert sich
  die Länge des Durchlaufs (Tempo, Clips), wird der Loop neu an die Uhr
  gesetzt; ändern sich die Tags, wird der Song neu gemeldet. Play im
  Loop-Panel bleibt gesperrt, solange der Composer den Ton hat.
- **In der Zeitachse:** statt einer Wellenform steht ein gestrichelter Strich
  an jeder Stelle, an der der Loop von vorn beginnt.
- **Tags bearbeiten:** Knopf **Tags…** in der Datei-Zeile des Loop-Panels,
  über Undo.

**Von mir gesetzt, nicht entschieden:**

- Ein Loop-Clip rastet in der Zeitachse wie ein Song (Kanten anderer Clips,
  Raster in Sekunden) — nicht auf die Takte des Loops.
- Der Klangerzeuger einer entfernten Quelle lebt noch vier Takte weiter, weil
  nicht belegt ist, dass `mixerRemoveSource` auf einen laufenden Rückruf wartet.
- Der Composer hängt jetzt am Namensraum `loop` und umgekehrt (das Loop-Panel
  fragt, ob der Composer den Ton hat; die Taktachse nimmt seine Farbreihen).

**Nicht drin:** ein Tempo der Komposition, an dem Loops einrasten (L2 a,
zurückgestellt); ein Loop, der allein im Loop-Panel spielt, meldet sich nicht
als Song; ein Cover.

---

## 4f. Der Instrument-Clip (Stufe 9)

Entscheid I1 c: Instrumente sind gerechnet oder kommen aus einem Sample —
zuerst gerechnet. **Gebaut sind 9a** (Modell, Datei, die gerechneten
Instrumente, der Rechenkern) **und 9b** (der Editor und **New instrument
clip**) — im Programm noch nicht gesehen und nicht gehört.

### Der Instrument-Editor (Skizze gebilligt, Patrik S110 — gebaut, 9b)

`loop/InstrumentEditor.hpp`: das `NotenRaster` (es rollt selbst, Klaviatur,
Kopf und die Zeile der Lautstärken bleiben stehen) und das Fenster
`InstrumentEditor`. Das Loop-Panel wählt den Editor nach der Art der Basis
(`LoopPanel::oeffneEditor`).

Ein eigenes Fenster wie der Drum-Editor, geöffnet über **Edit base** an einem
Instrument-Clip oder mit einem Doppelklick auf ihn; es heißt „Instrument
pattern: <Basis>".

- **Kopf:** die Basis und wie viele Clips sie spielen; **Bars**; **Play
  pattern**; **Peak** — wie im Drum-Editor.
- **Instrument-Zeile:** die Auswahl **Instrument** (Piano, Strings, Violin,
  Guitar, Bass, Organ, Lead), daneben **Volume**, **Pan** und **M** (stumm).
  Ein Muster spielt genau ein Instrument, mehrere Töne zugleich.
- **Klaviatur und Raster:** links die Tasten (schwarze dunkel, die C
  beschriftet), rechts die Schritte des Loops mit den Takten im Kopf. Eine
  Note ist ein Balken in der Zeile ihrer Tonhöhe, so lang wie sie dauert.
  Das Raster rollt waagrecht (lange Muster) und senkrecht: **sechs Oktaven,
  C1 bis C7** (Patrik S110: mehrere Oktaven); das Fenster geht bei C3 bis C5
  auf.
- **Note volume:** unter dem Raster die Lautstärke je Note als Balken, wie
  Step volume im Drum-Editor.

| Geste | Wirkung |
|---|---|
| Klick auf eine freie Stelle | eine Note von einem Schritt Länge |
| Rechte Kante einer Note ziehen | ihre Länge, in ganzen Schritten |
| Note ziehen | verschieben in Zeit und Tonhöhe |
| Rechtsklick auf eine Note, Entf | löschen |
| Klick auf eine Taste | spielt den Ton an |
| In der Zeile Note volume klicken oder ziehen | die Lautstärke der Note |

Im Loop-Panel bietet der Rechtsklick auf eine freie Stelle neben **New
drumkit clip** auch **New instrument clip**.

**Gebaut wie die Skizze, dazu von mir gesetzt, nicht entschieden:**

- Beim Setzen einer Note weitergezogen wird sie gleich länger.
- Eine neue Note hat volle Lautstärke; leiser als 0,05 wird keine (ohne
  Lautstärke gäbe es sie nicht).
- Die Zeile unten stellt alle Noten, die an dem Schritt beginnen — außer eine
  von ihnen ist angeklickt, dann nur sie. Sie heißt im Fenster „Note vol.":
  die Spalte der Klaviatur ist für „Note volume" zu schmal.
- Das Mausrad rollt die Tonhöhen, mit Shift die Zeit; zoomen lässt sich das
  Raster nicht (ein Schritt ist 16 Pixel breit, eine Tonhöhe 14 hoch).
- Eine Note lässt sich nicht auf den Einsatz einer anderen derselben Tonhöhe
  schieben; überlappen dürfen sich zwei Noten einer Tonhöhe.
- **Anspielen einer Taste:** der Spieler des Panels spielt für einen Schlag
  einen Loop aus dieser einen Note — der Player pausiert so lange, wie bei
  Play. Spielt gerade der Loop oder ein Muster, oder hat der Composer den
  Ton, geschieht nichts. Beim Setzen oder Verschieben einer Note klingt
  nichts.
- Je Art gibt es **ein** Editor-Fenster: Drum-Editor und Instrument-Editor
  können zugleich offen sein.
- Das Raster des ganzen Loops (Schläge, Schritte, Swing) steht nur im
  Drum-Editor.
- Größe und Lage des Fensters merkt die App nicht.

**Nachgebessert (S110, Rückmeldung Patrik nach dem ersten Blick: Tasten
anspielen und Notenlängen ziehen):** beides war gebaut, aber schwer zu
finden. Der Griff am Ende einer Note ist jetzt ein Drittel der Note breit
(mindestens 5, höchstens 9 Pixel) und der Mauszeiger zeigt ihn (Doppelpfeil
an der Kante, Kreuz auf der Note, Hand auf der Klaviatur); eine gedrückte
Taste leuchtet; und neben **M** steht, warum die Tasten gerade stumm bleiben
(„The Composer plays the audio — keys and Play pattern are silent", „Keys are
silent while the loop plays"). Ob es bei Patrik daran lag, ist nicht
geklärt — im Programm nicht nachgestellt.

**Nicht drin:** mehrere Noten zugleich wählen; Kopieren und Einfügen; ein
Zoom; Vorhören beim Setzen; die Abspielmarke rollt den Ausschnitt nicht mit.

**Nicht gemessen:** beim Bearbeiten im laufenden Loop lädt der Spieler bei
jeder Änderung, die einen neuen Ton braucht, **alle** Klänge neu (auch die
des Kits) — ob das zu hören ist, weiß ich nicht.

### Der Instrument-Creator (Stufe 10 — Skizze gebilligt, Patrik S110; noch nicht gebaut)

Ein eigenes Fenster „Instrument: <Name>", geöffnet im Instrument-Editor über
**Edit instrument...** neben der Auswahl. Ein mitgeliefertes Instrument wird
dabei zur Vorlage einer eigenen Kopie. Ablage nach I2 b: eine Datei
`.lvinst`, die das Muster beim Pfad nennt.

- **Kopf:** **Name**; „based on <Vorlage>"; **Reset** (zurück auf die
  Vorlage); **Save as...** (schreibt die `.lvinst`); **Peak**.
- **Oscillator:** **Waveform** (Sine, Triangle, Saw, Square, Plucked string),
  **Voices** (1 bis 3), **Detune**. Bei Sine stehen statt Voices und Detune
  die Regler der Obertöne, bei Plucked string ein Regler **Damping**.
- **Envelope:** **Attack**, **Decay**, **Sustain**, **Release**.
- **Filter:** **Cutoff** (als Vielfaches des Grundtons), **Envelope** (wie
  weit die Hüllkurve ihn öffnet).
- **Vibrato:** **Rate**, **Depth**, **Delay**; darunter **Level**.
- **Rechts:** „One cycle" — das Bild einer Schwingung; „Envelope of a note" —
  die Hüllkurve mit der Stelle, an der die Note endet; darunter eine kleine
  Klaviatur (C3 bis C5): ein Klick spielt den Ton mit den Einstellungen, wie
  sie gerade stehen.

### Modell und Datei (gebaut, 9a)

Ein Muster der Art `instrument` trägt statt Spuren und Zeilen:

```json
{
  "name": "Melody",
  "kind": "instrument",
  "bars": 4,
  "instrument": "piano",
  "volume": 1,
  "pan": 0.1,
  "mute": false,
  "notes": [
    [0, 4, 76, 0.9],
    [4, 2, 72, 0.7]
  ]
}
```

| Feld | Bedeutung | Grenzen |
|---|---|---|
| `instrument` | der Name eines mitgelieferten Instruments (`piano`, `strings`, `violin`, `guitar`, `bass`, `organ`, `lead` — die Dateien unter `asset/loop/instruments`) **oder** der Pfad einer eigenen Datei `.lvinst`, relativ, wenn sie unter dem Ordner der `.lvloop` liegt, sonst absolut (I2 b); fehlt die Angabe, gilt `piano` | — |
| `volume`, `pan`, `mute` | für das ganze Muster, wie bei einer Spur | 0 bis 1, -1 bis 1 |
| `notes[]` | je Note eine Zeile: **Schritt** (0 = der erste des Musters), **Länge** in Schritten, **Tonhöhe** als MIDI-Nummer (60 = C4, 69 = A4), **Lautstärke** | Länge ab 1, Tonhöhe 24 bis 96 (C1 bis C7), Lautstärke über 0 bis 1 |

Die Fassung der Datei bleibt 2: eine ältere LumiViz-Fassung meldet „Unknown
pattern kind 'instrument'" und lässt das Muster weg.

**Beim Laden** (`Loop::richte`): Noten kommen in ihre Grenzen und stehen nach
Schritt, dann Tonhöhe sortiert; je Schritt und Tonhöhe bleibt eine; eine
Note, die über das Ende des Musters reicht, wird kürzer; eine, die dahinter
beginnt oder keine Lautstärke hat, geht. Ein unbekanntes Instrument bleibt in
der Datei stehen und schweigt: „Instrument not found: …" im Bericht.

**Raster ändern** (`setzeRaster`): der Einsatz einer Note bleibt an seiner
Stelle im Takt, wo das neue Raster sie kennt — sonst geht die Note; ihre
Länge bleibt dieselbe Zeit, auf ganze Schritte gerundet.

### Die gerechneten Instrumente (gebaut, 9a)

`loop/Instrument.hpp`. Ein Instrument ist ein Satz Einstellungen — das, was
der Instrument-Creator (Stufe 10b) offenlegt — und seit Stufe 10a **eine
Datei `.lvinst`** (I2 b, I3 a):

```json
{
  "format": "lumiviz-instrument",
  "version": 1,
  "title": "Strings",
  "waveform": "saw",
  "harmonics": [],
  "voices": 3,
  "detune": 9,
  "attack": 160, "decay": 400, "sustain": 0.85, "release": 420,
  "filterCutoff": 6, "filterEnvelope": 0,
  "vibratoRate": 5, "vibratoDepth": 6, "vibratoDelay": 300,
  "damping": 0.996,
  "level": 0.7
}
```

| Feld | Bedeutung | Grenzen |
|---|---|---|
| `title` | wie das Instrument im Programm steht; ohne Titel der Dateiname | — |
| `waveform` | `sine`, `triangle`, `saw`, `square` oder `string` (die gezupfte Saite) | — |
| `harmonics` | nur `sine`: die Anteile des 2., 3., … Teiltons | höchstens 16, je -1 bis 1 |
| `voices`, `detune` | gleiche Schwingungen übereinander; so viele Cent liegen die äußeren daneben | 1 bis 3; 0 bis 100 |
| `attack`, `decay`, `release` | Hüllkurve, in Millisekunden | 0 bis 5000, `decay` bis 10000 |
| `sustain` | Pegel, solange die Note dauert | 0 bis 1 |
| `filterCutoff`, `filterEnvelope` | Tiefpass als Vielfaches des Grundtons (0 = offen); so viele Vielfache öffnet die Hüllkurve dazu | 0 bis 64 |
| `vibratoRate`, `vibratoDepth`, `vibratoDelay` | Hertz, Cent, Millisekunden bis zur vollen Tiefe | 0 bis 20; 0 bis 100; 0 bis 5000 |
| `damping` | nur `string`: größer klingt länger | 0,9 bis 0,9999 |
| `level` | Ausgleich zwischen den Instrumenten | 0 bis 1 |

Die mitgelieferten liest das Programm einmal aus `asset/loop/instruments`
(nach Titel sortiert); eine eigene Datei liest es jedes Mal, wenn Klänge
geladen werden. Wird eine Instrument-Datei im Programm gespeichert
(`speichereInstrument`), zählt `instrumentFassung()` weiter — Spieler und
Tonführung rechnen die Töne dann neu, auch wenn die Noten gleich blieben.
Eine von außen geänderte Datei bemerkt das Programm erst beim nächsten Laden
der Klänge.

Die Bausteine:

| Baustein | Einstellungen |
|---|---|
| **Schwingung** (der Funktionsgenerator) | Sinus mit frei gesetzten Obertönen, Dreieck, Säge, Rechteck — als Wellenzug ohne Teiltöne über der halben Abtastrate; oder die **gezupfte Saite** (Karplus-Strong) |
| **Stimmen** | eine bis drei gleiche Schwingungen, leicht gegeneinander verstimmt |
| **Hüllkurve** | Anstieg, Abfall, Halten, Ausklang |
| **Filter** | Tiefpass, dessen Grenze der Tonhöhe folgt; die Hüllkurve kann ihn öffnen |
| **Vibrato** | Geschwindigkeit, Tiefe, Einsatz |

| Instrument | Wie gerechnet |
|---|---|
| Piano | Sinus mit fünf Obertönen, harter Einsatz, klingt über anderthalb Sekunden ab — klingt nach E-Piano |
| Strings | drei verstimmte Sägen, weicher Einsatz, langer Ausklang, leichtes Vibrato |
| Violin | eine Säge, heller, mit deutlichem Vibrato, das nach dem Einsatz anwächst |
| Guitar | die gezupfte Saite |
| Bass | Säge mit Tiefpass, den die Hüllkurve kurz öffnet |
| Organ | Sinus mit Obertönen wie Zugriegel, steht, solange die Note dauert |
| Lead | Rechteck mit Vibrato |

Dazu sieben Instrumente für **House** (Wunsch Patrik S110, als Dateien
dazugelegt — am Programm war dafür nichts zu ändern): House Piano, House
Organ, Chord Stab, Warm Pad, Sub Bass, Acid Bass, Pluck; Beschreibung in
`asset/loop/README.md`, Beispiel `asset/loop/house_demo.lvloop` (124 BPM,
24 Takte). Der Acid Bass hat **keine Resonanz** — der Filter des
Klangerzeugers kennt keine; das wäre ein neuer Baustein.

**Im Rechenkern:** jede Note wird **vorab** in einen Klang gerechnet
(`rendereTon`: so lang wie die Note, danach der Ausklang, am Ende bei null)
und vom Kern gespielt wie der Schlag eines Drumkits — der Kern bleibt ohne
Gedächtnis, obwohl Filter und Saite eines haben. Ein Ton heißt unter den
Klängen `inst:<Instrument>:<Tonhöhe>:<Dauer in ms>`; je Tonhöhe und Dauer
gibt es einen. Jeder Ton ist eine eigene Stimme: ein Akkord klingt zusammen,
nur derselbe Ton schneidet sich selbst ab. Die Dauer hängt am Tempo — ändert
sich das BPM, werden die Töne neu gerechnet.

**Von mir gesetzt, nicht entschieden:**

- Der Umfang C1 bis C7; ein Ton ist höchstens 30 s lang.
- Die Einstellungen der sieben Instrumente — nach Rechnung, **nicht nach
  Gehör**: ich höre sie nicht. Hörproben schreibt der Test mit
  `LUMIVIZ_INSTRUMENT_WAV=<Ordner>`.
- Eine Note endet spätestens am Ende ihres Musters; nur ihr Ausklang reicht
  darüber.
- Beide Seiten eines Tons sind gleich (kein Stereo im Instrument); das
  Panorama gilt für das ganze Muster.
- Der Pegel eines Tons liegt bei höchstens rund 0,5: Akkorde addieren sich
  (L6 a).

**Nicht gemessen:** wie lange das Rechnen der Töne bei einem großen Muster
dauert (es läuft im UI-Thread, bei jedem geänderten Tempo neu).

**Beispiel:** `asset/loop/instrument_demo.lvloop` — Drums, Strings-Akkorde,
Bass, dazu nacheinander Piano, Guitar und Violin; 100 BPM, 16 Takte.

---

## 5. Stufen

| Stufe | Inhalt | Stand |
|---|---|---|
| 1 | Dieses Konzept | S108 |
| 2 | **Beleg:** BASS nimmt eine selbst rechnende Quelle im Mixer an, die Schläge kommen auf das Sample genau (`test_LoopBelegeTon.cpp`). Scheitert er, kommt L1 noch einmal auf den Tisch. | S108, siehe §6 |
| 3 | **Kern ohne Ton und ohne Fenster:** Modell, Lesen und Schreiben der `.lvloop`, Rechenkern; Unit-Tests über die gerechneten Samples | S108 gebaut; **am selben Tag auf das Arrangement umgebaut** (Fassung 2, §4); `test_LoopKern.cpp`, zwölf Fälle |
| 4 | **Kit:** Skript und WAV-Dateien unter `asset/` | S108: `asset/loop/kit/` (sechs Klänge, `make_kit.py`), Beispiel `asset/loop/drums_demo.lvloop`; an einer Hörprobe gehört (Patrik: klingt gut) |
| 5a | **Wiedergabe, Unterbau:** Naht in `IAudioEngine` (`IAudioGenerator`, `mixerAddGenerator`, `decodeStereo`), `Klangerzeuger` (die Quelle im Mixer), `Kit` (Klänge finden und laden) | S108 gebaut, §4a; `test_LoopWiedergabe.cpp`, sechs Fälle; in der App noch nicht zu hören |
| 5b | **Wiedergabe in der App:** der Loop übernimmt den Ton (Rangfolge aus L4), die Visuals lesen von ihm; dazu ein erstes Panel mit Laden und Play | S108 gebaut, §4b; `test_LoopSpieler.cpp`, acht Fälle; **im Programm noch nicht gehört und nicht gesehen** |
| 6a | **Loop-Panel als Arrangement:** Kanäle, Zeitachse in Takten, Drumkit-Clips anlegen, verschieben, verlängern, kürzen, kopieren; Datei-Zeile mit New, Save, Save As; Undo | S109 gebaut, §4c: die gemeinsame Basis `Achse` (W1 b; 33 Bilder der Zeitachse vor und nach dem Umbau bytegleich, `test_AchseBilder.cpp`), darauf `Taktachse`, `Bearbeiten` und das Panel; `test_LoopArrangement.cpp`, sieben Fälle; **im Programm noch nicht gesehen** |
| 6b | **Drum-Editor als eigenes Fenster:** das Raster, Kit je Muster, Lautstärke je Schritt; „Basis bearbeiten" und „in neue Basis umwandeln" (L8 a); Anzeige der Summe (L6 a) | S109 gebaut, §4d; `test_LoopDrumEditor.cpp`, sieben Fälle, dazu zwei in `test_LoopSpieler.cpp`; **im Programm noch nicht gesehen und nicht gehört** |
| 6c | **Audio Browser** zeigt `.lvloop`, Doppelklick lädt ins Loop-Panel | S109 gebaut (§3.3); ein Fall in `test_PaletteUi.cpp`, einer in `test_LoopArrangement.cpp`; **im Programm noch nicht gesehen** |
| 7 | **Composer:** `.lvloop` als Clip im Audio-Kanal, Wiederholen bis zum Clip-Ende, Zeichnung des Clips, Ziehen aus dem Audio Browser; dazu Tags (T2) und die Verbindung zum Loop-Panel (L7 a) | S109 gebaut, §4e, Messung §6a; zwei Fälle in `test_LoopSpieler.cpp` (ohne Ton), zwei in `test_LoopWiedergabe.cpp` (mit sehr leisem Ton), dazu Tags und Verbindung im Panel-Fall von `test_LoopArrangement.cpp`; **im Programm noch nicht gesehen und nicht gehört** |
| 8 | **Abschluss:** Doku, Benutzerhandbuch, Vorlage für den Sichttest | S109: Benutzerhandbuch 1.115.0 (§16 Loop neu, Audio Browser, Loops als Clips), `Audio_System.md` 1.3.0 (§6a Mixer und wer den Ton hat), Vorlage `asset/sichttest/composer_K_loop.lvcomp` mit `asset/sichttest/loop/beat_A.lvloop` (an `test_ComposerVorlagen.cpp` gebunden), Liste `Sichttest_Composer_10_Loop.md` (lokal, 56 Punkte). S110: die Rückfrage beim Beenden (§4c) und drei Aktionen der Sichttest-Steuerung für das Panel Loop (`loop_laden`, `loop_wiedergabe`, `loop_editor`; `loop/SichttestAktionen`, ein Fall in `test_LoopSpieler.cpp`). Offen: ein eigenes Icon für `.lvloop`; das Kit im Release-Paket |
| 9a | **Instrument-Clip, Kern:** Noten im Modell und in der Datei, sieben gerechnete Instrumente (I1 c; Strings, Violine, Piano, Gitarre …, Wunsch Patrik S110), Noten im Rechenkern | S110 gebaut, §4f; `test_LoopInstrument.cpp`, sechs Fälle ohne Ton; Beispiel `asset/loop/instrument_demo.lvloop`; **nicht gehört** — die Instrumente sind nach Rechnung eingestellt |
| 9b | **Instrument-Editor:** die Piano-Rolle als eigenes Fenster (Skizze gebilligt, §4f), **New instrument clip** im Loop-Panel, eine Taste spielt den Ton an | S110 gebaut, §4f; `test_LoopInstrumentEditor.cpp` (zwei Fälle ohne Fenster; `LUMIVIZ_INSTRUMENT_EDITOR_BILD=<datei.png>` legt ein Bild ab), ein Fall in `test_LoopSpieler.cpp`; Benutzerhandbuch §16; **im Programm noch nicht gesehen und nicht gehört** |
| 10a | **Instrumente als Dateien** (I2 b, I3 a): das Format `.lvinst`, die sieben mitgelieferten unter `asset/loop/instruments`, ein Muster nennt ein eigenes beim Pfad | S110 gebaut, §4f; `test_LoopInstrument.cpp` (ein Fall dazu, die übrigen lesen jetzt die Dateien); die Hörproben und das Beispiel klingen wie vorher — die Werte sind dieselben |
| 10b | **Instrument-Creator:** das Fenster, das die Bausteine offenlegt (Skizze gebilligt, §4f), **Edit instrument...** und **Load instrument...** im Instrument-Editor | offen |
| 11 | **Instrumente aus Samples:** eine Tondatei, je Note in der Tonhöhe umgerechnet (I1 c) | später |

---

## 6. Beleg aus Stufe 2

`test_LoopBelegeTon.cpp`, zwei Fälle, grün in Testing-MSVC
(S108, ein Lauf einzeln, danach im vollen Lauf). BASS 2.4.17. **L1 a trägt.**

**Ohne Gerät, im dekodierenden Mixer** (die Quelle rechnet Klicks im Abstand
von 5513 Abtastwerten):

| Frage | Ergebnis |
|---|---|
| Nimmt BASSmix eine Quelle mit Rückruf (`BASS_StreamCreate`) an? | ja |
| Kommen die Klicks auf den Abtastwert genau an? | ja: 25876 Abtastwerte in fünf ungleichen Stücken gelesen, 5 Klicks, 0 falsche Werte; die Blöcke des Rückrufs waren 3 bis 8167 Abtastwerte groß |
| Addieren sich zwei solche Quellen? | ja: 12000 Abtastwerte, 0 falsch |
| Holt der Mixer voraus? | nein: nach 3000 gelesenen Abtastwerten stand die Quelle bei 3000 |
| Wer führt die Stelle? | **die Quelle selbst.** Setzt sie ihre Stelle um, liegt der nächste Klick auf den Abtastwert dort, wo er soll (100 erwartet, 100 gemessen) |
| Was weiß BASSmix von der Stelle? | `BASS_Mixer_ChannelGetPosition` zählt mit (3000). `BASS_Mixer_ChannelSetPosition` **meldet Erfolg, ändert an der Quelle aber nichts** |

**Mit Gerät, im spielenden Mixer der `BassEngine`** (die Quelle rechnet einen
Sinus, sehr leise; Mixer 44100 Hz, Gerät 48000 Hz):

| Frage | Ergebnis |
|---|---|
| In welchem Thread läuft der Rückruf? | nicht in dem, der den Mixer startet |
| Wie groß sind die Blöcke? | 220 bis 441 Abtastwerte, also 5 bis 10 ms; 105 Rückrufe in einer Sekunde |
| Holt der Mixer, was er spielt? | in 1000 ms 45872 Abtastwerte = 1040 ms Ton |
| Wie lange braucht eine Änderung bis in den Pegel des Mixers? | rund 48 ms (ein Lauf) |

**Folgen für den Bau:**

- Die neue Naht in `IAudioEngine` (Stufe 5) nimmt eine Quelle mit Rückruf.
  `setSourcePositionMs` der Engine wirkt auf sie **nicht** — die Stelle setzt
  der Klangerzeuger selbst; die Tonführung muss das für Loop-Clips wissen
  (Stufe 7).
- Die Quelle wird mit `BASS_MIXER_CHAN_NORAMPIN` eingehängt, sonst blendet
  der Mixer ihren Anfang ein und der erste Schlag verliert seinen Einsatz
  (so im Test; ohne die Marke nicht gemessen).
- Der Rechenkern muss mit jeder Blockgröße zurechtkommen (gesehen: 3 bis
  8167).

**Nicht gemessen:** der Verzug bei gedrosseltem Rechner; mehr als ein Lauf
für die 48 ms; Release-clang (dort nur gebaut, nicht gelaufen).

---

## 6a. Messung aus Stufe 7: der Klangerzeuger gegen die Wanduhr

`test_LoopWiedergabe.cpp`, Fall „Loop Messung" (S109, Testing-MSVC, das
Ausgabegerät des Rechners, 44100 Hz). Je 33 ms: gerechneter Ton minus
Wanduhr. `LUMIVIZ_LOOP_MESSUNG_S=<Sekunden>` misst länger.

| Lauf | Grundabstand | kleinster / größter | größter Schritt | Drift | Proben über 80 ms |
|---|---|---|---|---|---|
| 6 s | −1,1 ms | −6,4 / +3,5 ms | 7,0 ms | −144 ppm | 0 |
| 60 s | −1,0 ms | −7,9 / +3,8 ms | 7,0 ms | −14 ppm | 0 |
| 60 s | −1,1 ms | −7,6 / +3,5 ms | 7,0 ms | −35 ppm | 0 |

Der Abstand ist ruhig und liegt weit unter der Toleranz der Tonführung; bei
14 bis 35 ppm wären 80 ms erst nach rund 40 bis 90 Minuten erreicht (die
144 ppm des kurzen Laufs sind Rauschen der kurzen Strecke). Über die
Tonführung selbst, mit dem Beispiel als Clip: 40 s, kein Nachsetzen.

**Nicht gemessen:** ein Lauf, in dem der Rechner drosselt (in keinem kam der
Messfaden länger als 35 ms nicht dran); ein anderes Ausgabegerät (über Funk
ist die Latenz ein Vielfaches); Release-clang.

---

## 7. Risiken

- **Nachsetzen der Tonführung:** eine Quelle wird nachgesetzt, wenn sie mehr
  als 80 ms von der Uhr abweicht (`Tonfuehrung::kToleranzMs`). Bei einem Song
  ist das ein Ruck, bei Drums ein Stolperer. Gemessen in Stufe 7 (§6a) und
  entschieden mit T1 a: aus Drift selten; offen bleibt, wie oft es in den
  Schüben vorkommt, in denen der Rechner drosselt. Wäre es zu hören, ist der
  nächste Schritt, nur an einer Durchlauf-Grenze nachzusetzen.
- **Code im Ton-Thread:** der erste in der App. Was das Panel ändert, muss
  ohne Sperre in den Rechenkern gelangen; der Kern darf im Rückruf weder
  Speicher holen noch Dateien lesen.
- **Der Entwicklungsrechner drosselt** (S107, Temperaturschutz): in den
  Einbrüchen kann der Ton aussetzen. Das ist dann kein Befund am Loop —
  zuerst die Taktprobe daneben.
- **Klang des gerechneten Kits:** erst zu beurteilen, wenn es da ist. Seit
  S108 liegt es vor (`asset/loop/README.md`); Patriks Urteil steht aus.
- **Die Summe übersteuert leicht** (S108, an der Hörprobe gerechnet): das
  Beispiel mit sechs Spuren erreicht bei Lautstärke 0,8 des Loops einen
  Betrag von 1,45, bei 0,5 sind es 0,91. Der Kern begrenzt nicht (§4).
  Entschieden mit L6 a: Luft nach oben, Anzeige im Panel.

---

## 8. Changelog

- **0.18.0** (S110, 2026-10-11): Entscheid I3 a (auch die mitgelieferten
  Instrumente sind Dateien); Stufe 10 geteilt, 10a gebaut — das Format
  `.lvinst` (§4f), `asset/loop/instruments`; sieben Instrumente für House
  und das Beispiel `house_demo.lvloop`.
- **0.17.0** (S110, 2026-10-11): Entscheid I2 b (ein eigenes Instrument ist
  eine eigene Datei `.lvinst`, wie die Klänge des Kits); die gebilligte Skizze des
  Instrument-Creators in §4f; Instrument-Editor nachgebessert (Griff,
  Mauszeiger, Hinweis bei stummen Tasten).
- **0.16.0** (S110, 2026-10-11): Stufe 9b gebaut — der Instrument-Editor,
  New instrument clip, Anspielen einer Taste (§4f).
- **0.15.0** (S110, 2026-10-11): Stufe 9 geteilt, 9a gebaut — §4f neu: die
  gebilligte Skizze des Instrument-Editors (sechs Oktaven), Noten in Modell
  und Datei, sieben gerechnete Instrumente, Noten im Rechenkern.
- **0.14.0** (S110, 2026-10-11): Entscheid I1 c — Instrumente gerechnet
  oder aus Samples, zuerst gerechnet; Stufen 10 (Instrument-Creator) und 11
  (Samples) dazu; drei Aktionen der Sichttest-Steuerung für das Panel Loop.
- **0.13.0** (S110, 2026-10-11): das Loop-Panel fragt beim Beenden der App
  nach Ungespeichertem (§4c); die Rückfrage überall Save / Discard / Cancel.
- **0.12.0** (S109, 2026-10-11): Stufe 8 — Benutzerhandbuch, `Audio_System.md`,
  Vorlage und Liste für den Sichttest.
- **0.11.0** (S109, 2026-10-11): Stufe 7 gebaut — der Loop als Clip im
  Composer (§4e neu), Messung §6a; Entscheide T1 a (an der Uhr wie ein Song)
  und T2 (zählt als laufender Song, Tags in der `.lvloop`); L7 a umgesetzt.
- **0.10.0** (S109, 2026-10-11): Stufe 6c gebaut — der Audio Browser zeigt
  `.lvloop`, der Doppelklick lädt ins Loop-Panel.
- **0.9.0** (S109, 2026-10-11): Stufe 6b gebaut — der Drum-Editor (§4d
  neu): Schritt-Raster, Spuren, Raster des Loops, Play pattern, Peak.
- **0.8.0** (S109, 2026-10-11): Stufe 6a gebaut — das Loop-Panel als
  Arrangement (§4c neu): Taktachse, Regeln des Arrangements, Datei-Zeile,
  BPM, Undo.
- **0.7.0** (S109, 2026-10-11): Entscheid W1 b — Loop-Panel und Composer
  teilen sich die Achse (`lumi::ui::Achse`); W1 a und c verworfen (§3.1);
  Stufe 6a, Schritt 1 gebaut.
- **0.6.0** (S108, 2026-10-11): das Zielbild neu gefasst (Patrik): der Loop
  ist ein Arrangement in Takten — Kanäle mit Clips, Editoren als eigene
  Fenster, später Instrument-Clips; L3 neu, L8 a; §1 und §4 neu, Datei in
  Fassung 2; Stufe 6 geteilt, Stufe 9 dazu; Audio Browser als Wunsch.
- **0.5.0** (S108, 2026-10-10): Entscheid L7 a; Stufe 5b gebaut (§4b):
  Spieler mit Rangfolge, Visuals lesen vom Loop, erstes Panel.
- **0.4.0** (S108, 2026-10-10): Entscheid L6 a (Luft nach oben, Vorgabe
  0,5); Stufe 5 geteilt, 5a gebaut (§4a); „das Kit ist klein" als offener
  Punkt.
- **0.3.0** (S108, 2026-10-10): Stufe 4 (Kit und Beispiel unter
  `asset/loop`) eingetragen; Risiko „die Summe übersteuert" mit Zahlen.
- **0.2.0** (S108, 2026-10-10): Stufe 2 (Beleg, §6) und Stufe 3 (Kern, §4)
  eingetragen; §4 von der Skizze zum gebauten Format.
- **0.1.0** (S108, 2026-10-10): angelegt — Ziel, Bestand, Entscheide L1 bis
  L5, Skizze der Datei, Stufen, Risiken.

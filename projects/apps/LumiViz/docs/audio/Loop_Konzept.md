# LumiViz — Loop (Konzept)

> **Version:** 0.26.0
> **Datum:** 2026-10-11 (Session 113)
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
| **F1 a** (S111) | **Schichten stehen in der `.lvinst` als Liste `layers`, die Datei trägt Fassung 2.** Oben stehen nur noch Titel und Level des ganzen Instruments; jede Schicht ist ein Eintrag mit den bisherigen Feldern, dazu `name`, `on`, `transpose`, `startOffset`, `drive`. Fassung 1 liest das Programm als Instrument mit einer Schicht; gespeichert wird immer Fassung 2; eine höhere Fassung als die eigene lehnt es ab und sagt es im Bericht. |
| **E1 a** (S111) | **Der Verzerrer gehört zum Instrument, Hall und Echo gehören ans Muster.** Der Verzerrer formt nur den Augenblickswert um und passt in den vorab gerechneten Ton: ein Regler **Drive** je Schicht, gebaut mit Stufe 10c. Hall und Echo klingen nach und stehen am Muster neben Volume und Pan — sie gelten damit auch für Drum-Muster; dafür bekommt der Rechenkern erstmals ein Gedächtnis. Das ist eine eigene Stufe nach 10c mit eigener Skizze (Stufe 12); ob sie je Muster oder je Kanal des Loop-Panels sitzen, klärt jene Skizze. **Mit E2 b (S113) überholt, was den Ort angeht:** Hall und Echo sitzen am Clip, nicht am Muster. |
| **E2 b** (S113) | **Jeder Clip im Loop-Panel trägt eigene Effekte mit allen Einstellungen** — für Drumkit- und Instrument-Clips gleich; ein Fenster für den ganzen Loop gibt es nicht. Patrik: „im Loop soll jeder Clip für sich selbst Effekte erhalten können, nicht der ganze Loop"; dafür lassen sich mehrere Clips zugleich markieren. Die Effekte sind **Reverb**, **Echo** und **Distortion**, später **Autotune** (§3.2). Skizze und Beschreibung in §4g. |
| **F2 a** (S113) | **Die Effekte stehen als Feld `effects` am Clip, die `.lvloop` trägt Fassung 3.** Je Effekt ein Eintrag mit `on` und seinen Werten (`distortion`, `echo`, `reverb`); ein Clip ohne Effekte trägt das Feld nicht. Fassung 2 wird weiter gelesen, gespeichert wird immer Fassung 3. Ein älteres LumiViz spielt eine solche Datei ohne Effekte und meldet „File was written by a newer version". Verworfen: b — die Fassung bleibt 2 (eine Datei ohne Effekte bliebe bytegleich, aber ein älteres Programm spielte einen Loop mit Effekten trocken, ohne es zu sagen). |
| **G1 b** (S113) | **Die Effekte eines Clips rechnen im Ton-Thread mit, Block für Block.** Jeder gerade klingende Clip mit Effekten hat im Klangerzeuger einen eigenen Verzerrer, ein eigenes Echo und einen eigenen Hall. Eine Änderung am Muster oder an den Effekten gilt ab dem nächsten Block. Springt die Stelle um weniger als 200 ms (das Nachsetzen nach T1 a), klingt der Nachklang unverändert weiter; springt sie weiter, werden Echo und Hall geleert — der Nachklang der Schläge davor fehlt, bis er sich neu aufgebaut hat, längstens so lange wie Decay. Der Rechenkern selbst bleibt ohne Gedächtnis; es sitzt in den Effekten. Grund (Patrik): wer gerade am Muster eines Clips arbeitet, soll nicht auf ein Neurechnen warten. Die 200 ms sind von mir gesetzt. |
| **P1 a** (S113) | **Instrumente wählt man in einem Palettenfenster wie bei der Effektkette**, nicht mehr in einer Dropdown-Liste: ein Baum nach Art, Suchfeld, Favorites, Recently used, ein Satz Beschreibung, Anspielen per Klick. Dazu Patrik: auch **Sortieren oder Filtern nach Genre** muss gehen. Beschrieben in §4h; eigene Stufe 14. |

### 3.1 Verworfen, mit Grund

- **Jeden Clip mit Effekten vorab als fertigen Klang rechnen** (G1 a, meine
  erste Empfehlung): der Nachklang stimmte nach jedem Sprung auf den
  Abtastwert, und der Ton-Thread rechnete keinen Hall — aber jede Änderung am
  Muster rechnete alle Clips der Basis neu, die Effekte haben; beim
  Bearbeiten im laufenden Loop käme die Änderung verzögert (Einwand Patrik).
  Die Last im Ton-Thread, gegen die a gedacht war, ist gemessen klein (§4g).
- **Im Ton-Thread rechnen und nach einem Sprung vorrechnen** (G1 c): bei 8 s
  Decay wären acht Sekunden Ton auf einmal im Ton-Thread zu rechnen. Als
  Nachrüstung außerhalb des Ton-Threads bleibt es möglich, falls der fehlende
  Nachklang nach einem weiten Sprung stört.

- **Ein Hall und ein Echo für den ganzen Loop, Anteile je Clip oder je
  Muster und Drum-Spur** (E2 a, meine Empfehlung in zwei Fassungen): alles
  klänge in einem Raum, eine Drum-Spur könnte trocken bleiben, und der
  Ton-Thread rechnete jeden Effekt nur einmal — aber Patrik will, dass ein
  Clip einen eigenen Raum und eine eigene Echo-Zeit haben kann, unabhängig
  vom Rest des Loops.
- **Effekte je Muster** (erste Fassung, b): ein Muster nähme seinen Klang
  mit — aber zwei Clips derselben Basis klängen immer gleich.
- **Effekte je Kanal des Loop-Panels** (erste Fassung, c): der Nachklang
  liefe über Clip-Grenzen im Kanal von selbst weiter — aber ein Drum- und
  ein Instrument-Clip im selben Kanal teilten sich ungefragt die Einstellung.
- **Die Dropdown-Liste der Instrumente bleibt, mit Gruppen** (P1 b): kostet
  fast nichts und öffnet kein weiteres Fenster — aber 49 Einträge lassen sich
  so weder suchen noch anspielen, und die Liste wächst.

- **Die erste Schicht bleibt flach, weitere kommen in `layers` dazu** (F1 b):
  eine ältere Programmfassung spielte noch die erste Schicht — aber die erste
  hätte eine Sonderrolle, jedes Umsortieren, Löschen und Stummschalten müsste
  Felder zwischen oben und Liste umschichten, und die zwei Level fielen für
  sie in ein Feld zusammen. Eine ältere Fassung, die `.lvinst` kennt, gibt es
  außer dem Stand aus S110 nicht.
- **Eine Schicht weiter flach als Fassung 1 schreiben** (F1 c): die
  mitgelieferten Dateien blieben beim Speichern unverändert — dafür zwei
  Schreibweisen, beide zu testen.
- **Alle Effekte im Instrument** (E1 b): ein Instrument klänge fertig, wo
  immer man es lädt — aber Hall je Instrument ergibt in einem Loop mit fünf
  Instrumenten fünf Räume, jeder Ton wäre um die Hallfahne länger zu rechnen,
  ein Echo kennt das Tempo des Loops nicht, und die Drums blieben ohne.
- **Alle Effekte am Muster, auch der Verzerrer** (E1 c): eine Stelle für
  alles — aber der Verzerrer fehlte als Klangfarbe einer einzelnen Schicht;
  auf dem ganzen Muster reiben die Töne eines Akkords aneinander, je Ton
  bleibt er sauber.

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
- **Autotune als Effekt am Clip** (Patrik S113: darf warten): es braucht eine
  einzelne erkennbare Tonhöhe — bei einem Drumkit-Clip oder einem Akkord gibt
  es keine, und ein Instrument-Clip aus Noten ist schon gestimmt. Es kommt
  mit den **Wave-Clips** (Tonspuren wie Gesang, Wunsch Patrik S113), Stufe 15.
  Die Liste der Effekte am Clip wird so angelegt, dass es ohne Umbau
  dazukommt; die Messung des Grundtons teilt es mit **Detect** der
  Sample-Instrumente (§4h).

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
es nur einen Tag; sie wird nicht gelesen. **Fassung 3** (S113, F2 a): die
Effekte am Clip, siehe unter der Tabelle; Fassung 2 wird weiter gelesen,
geschrieben wird immer 3 — das Beispiel hier zeigt den Stand der Fassung 2.
JSON, Schlüssel englisch wie in `.lvcomp`:

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

**Effekte am Clip** (Fassung 3, gebaut in Stufe 12a — Modell und Datei;
gerechnet wird davon noch nichts):

```json
{ "pattern": "Grund", "start": 12, "repeat": 1,
  "effects": {
    "distortion": { "on": true,  "drive": 0.7, "tone": 0.2, "mix": 0.5 },
    "echo":       { "on": true,  "time": "1/4", "feedback": 0.6, "damping": 0.1, "mix": 0.8 },
    "reverb":     { "on": false, "size": 0.5, "decay": 3.5, "damping": 0.4, "mix": 0.25 }
  } }
```

| Feld | Bedeutung | Grenzen, Vorgabe |
|---|---|---|
| `distortion.drive`, `tone`, `mix` | Stärke, Klangfarbe (0 dunkel, 1 hell), Anteil | je 0 bis 1; 0,3 · 0,5 · 1 |
| `echo.time` | Notenwert: `1/16`, `1/8`, `1/8d` (punktiert), `1/4`, `1/2` | Vorgabe `1/8d` |
| `echo.feedback`, `damping`, `mix` | was jede Wiederholung weitergibt, Abdunkeln, Anteil | 0 bis 0,9 · 0 bis 1 · 0 bis 1; 0,4 · 0,3 · 0,35 |
| `reverb.size`, `decay`, `damping`, `mix` | Raumgröße, Nachklang in Sekunden, Abdunkeln, Anteil | 0 bis 1 · 0,2 bis 8 · 0 bis 1 · 0 bis 1; 0,5 · 1,8 · 0,4 · 0,25 |
| `on` | je Effekt; fehlt es, ist er aus | — |

Geschrieben wird nur, was von der Vorgabe abweicht: ein Clip ohne Effekte
trägt `effects` nicht, ein Effekt in Vorgabe-Stellung seinen Eintrag nicht;
ein **ausgeschalteter** Effekt mit eigenen Werten behält sie. Beim Laden
kommen die Werte in ihre Grenzen; eine unbekannte Echo-Zeit wird zur Vorgabe
und steht als `Unknown echo time '…'` im Bericht. **Von mir gesetzt, nicht
entschieden:** alle Grenzen und Vorgaben dieser Tabelle (die Skizze nannte
Platzhalter), auch die Obergrenze 0,9 für Feedback — darüber klänge ein Echo
nie aus. Die mitgelieferten Loops und ihre Erzeuger (`make_examples.py`,
`make_genres.py`) bleiben bei Fassung 2.

Im Code ist `LoopClip` seit 12a kein Aggregat mehr, sondern hat einen
Konstruktor (Muster, Start, Wiederholungen, Effekte freiwillig) — sonst
warnte clang an jeder Stelle, die einen Clip aus drei Werten anlegt.

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

### Der Instrument-Creator (Stufe 10b — Skizze gebilligt, Patrik S110; gebaut)

`loop/InstrumentCreator.hpp`. Der Creator arbeitet an einer **Kopie**; erst
**Save** schreibt die Datei. Speichert er, nennt die Basis des
Instrument-Editors danach seine Datei (ein Schritt für Undo: „Change
instrument"); wer den Loop gerade spielt, rechnet die Töne neu.

**Gebaut wie die Skizze, dazu von mir gesetzt, nicht entschieden:**

- **Save** neben Save as...: es schreibt in die eigene Datei und ist grau,
  solange es keine gibt — ein mitgeliefertes Instrument lässt sich nur unter
  neuem Namen speichern.
- Bei **Sine** stehen die Regler der Obertöne (2. bis 9. Teilton) **neben**
  Voices und Detune, nicht statt ihrer: der Klangerzeuger kennt beides
  zugleich. Bei **Plucked string** bleiben nur Attack, Release, Damping und
  Level — den Rest kennt die Saite nicht.
- Die zwei Bilder zeigen **eine** Stimme ohne Vibrato: drei verstimmte
  Stimmen schweben, im Bild sähe man die Schwebung statt Wellenzug und
  Hüllkurve. **Peak** gilt für den Ton, wie er klingt (C4, 600 ms).
- **Anspielen:** die Kopie kommt — auch ungespeichert — in eine
  Vorhör-Datei im Ordner für Zwischendateien, und der Spieler des Panels
  spielt sie einen Schlag lang (wie die Tasten des Instrument-Editors: der
  Player pausiert so lange; spielt der Loop, bleibt es stumm).
- Im Creator gibt es kein Undo, nur **Reset**. Beim Schließen mit
  Ungespeichertem fragt er (Save / Discard / Cancel).
- Zeigt der Instrument-Editor eine andere Basis, schließt sich der Creator.
- Die Regler reichen weniger weit als die Datei erlaubt (Attack bis 1 s,
  Decay bis 5 s, Release bis 3 s, Cutoff bis zum 32-fachen); ein Wert
  außerhalb oder zwischen zwei Stellungen bleibt stehen, bis jemand an
  seinem Regler zieht.
- **Load instrument...** im Instrument-Editor nimmt eine `.lvinst` für die
  Basis; in der Auswahl steht sie als „Titel (Dateiname)".

**Nicht drin:** hören, was man einstellt, **während** das Muster spielt (die
Töne des laufenden Loops ändern sich erst mit Save); eine Resonanz am
Filter; ein eigener Ordner des Benutzers in der Auswahl.

Die Skizze:

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

### Instrumente aus Schichten (Stufe 10c — Skizze gebilligt, Patrik S111; gebaut)

Wunsch Patrik S110: mehrere Charakteristiken in einem Instrument überlagern.
Ein Instrument trägt dafür **Schichten** — jede ein voller Satz der heutigen
Einstellungen; beim Rechnen eines Tons werden sie addiert. Das Format
`.lvinst` dazu: Entscheid **F1 a** (§3), beschrieben unter „Die gerechneten
Instrumente".

**Gebaut wie die Skizze, dazu von mir gesetzt, nicht entschieden:**

- **Drive** steht in der Gruppe Filter, unter Envelope; 0 ist aus, der Regler
  reicht bis 100 %. Gerechnet als `tanh(8·Drive·x) / tanh(8·Drive)` auf dem
  Wert nach dem Filter und vor der Hüllkurve: der Betrag wächst nicht, der
  Klang wird dichter. Er gilt auch für die gezupfte Saite.
- **Transpose** −24 bis +24 Halbtöne; **Start offset** 0 bis 1000 ms, der
  Regler in Schritten von 5 ms.
- Eine Schicht mit Start offset **endet mit der Note**: sie klingt um den
  Versatz kürzer. Ist die Note kürzer als der Versatz, bleibt von der Schicht
  ein Anschlag von einer Millisekunde und ihr Ausklang.
- Der Klang ist so lang wie die längste Schicht. Ist **keine** Schicht an,
  ist er still und so lang wie die Note.
- **Add layer from instrument...** öffnet ein Menü: die mitgelieferten
  Instrumente, darunter **From file...**. Eine Schicht ohne Namen bekommt den
  Titel ihres Instruments; die Lautstärke des ganzen Quell-Instruments wandert
  in die Lautstärke der Schicht. Was über vier Schichten ginge, bleibt weg.
- Bei vier Schichten sind Add layer, Add layer from instrument... und
  Duplicate grau; bei einer ist Remove grau.
- Die Legende unter den Bildern („Solid: all layers · dashed: selected
  layer") und der Haken Selected layer only stehen nur bei mehr als einer
  Schicht zur Verfügung. Beide Kurven teilen sich einen Maßstab.
- In der Liste steht eine Schicht ohne Namen als „Layer n", mit Namen als
  „n Name".
- Der Regler **Level** der Schicht behält die Kennung `creatorLevel`; der des
  ganzen Instruments heißt `creatorMasterLevel`.

**Nicht gemessen:** die Rechenzeit — jede Schicht wird je Ton einzeln
gerechnet, ein Instrument mit vier Schichten kostet beim Laden der Klänge
rund das Vierfache.

**Sechzehn Instrumente aus Schichten** (Wunsch Patrik S111: weitere, auch
ganz ungewöhnliche) liegen unter `asset/loop/instruments`, dazu fünf
Beispiel-Loops; erzeugt von `asset/loop/make_examples.py`, beschrieben in
`asset/loop/README.md`. Wie alle nach Rechnung eingestellt, nicht nach Gehör.

**Neunzehn Beispiel-Loops nach Genre** (Wunsch Patrik S111) liegen unter
`asset/loop/genres`, mit **neunzehn weiteren Instrumenten** — darunter
Cowbell, Conga und Clave als Schlagwerk, weil das Kit nur sechs Klänge hat;
erzeugt von `asset/loop/make_genres.py`. Mitgeliefert sind damit
neunundvierzig Instrumente.

Die Skizze — das Fenster des Instrument-Creators, links um eine Spalte
erweitert:

- **Kopf:** wie heute Name, „based on <Vorlage>", Reset, Save, Save as...;
  dazu **Level** für das ganze Instrument neben **Peak**. Peak gilt für die
  Summe aller eingeschalteten Schichten.
- **Links, „Layers":** die Liste der Schichten. Je Zeile ein **Haken**
  (schaltet die Schicht stumm, ohne sie zu löschen), Nummer und **Name**, die
  Schwingung, darunter Lautstärke und Versatz („Level 35 % · +12 st"). Ein
  Klick auf eine Zeile wählt die Schicht; die Mitte zeigt ihre Regler.
  Darunter:
  - **Add layer** — eine neue Schicht mit Grundeinstellung (Sine, eine
    Stimme).
  - **Add layer from instrument...** — die Einstellungen eines vorhandenen
    Instruments als neue Schicht, aus der Liste der mitgelieferten oder aus
    einer `.lvinst`. Kopiert, nicht verbunden; hat das gewählte Instrument
    selbst mehrere Schichten, kommen alle dazu.
  - **Duplicate**, **Remove** (grau bei nur einer Schicht), **▲ ▼** für die
    Reihenfolge.
- **Mitte, „Layer n":** was nur die Schicht betrifft — **Name**, **Level**,
  **Transpose** (−24 bis +24 Halbtöne), **Start offset** (die Schicht setzt
  so viele Millisekunden nach dem Anschlag ein; kein Echo). Darunter die
  heutigen Gruppen Oscillator, Envelope, Filter, Vibrato unverändert, jetzt
  je Schicht; dazu **Drive** (der Verzerrer, E1 a).
- **Rechts:** „One cycle" und „Envelope of a note" zeigen die **Summe** aller
  Schichten als Linie und die gewählte Schicht gestrichelt, mit Legende
  („all layers", „selected"). Die Klaviatur C3 bis C5 spielt alle
  eingeschalteten Schichten; der Haken **Selected layer only** darunter lässt
  die gewählte allein hören.

**Mit der Skizze gebilligt (von mir vorgeschlagen):** Start offset je
Schicht; höchstens **vier** Schichten (jede wird je Ton einzeln gerechnet —
die Rechenzeit ist nicht gemessen); kein Fein-Versatz in Cent je Schicht
(Detune gibt es innerhalb einer Schicht); kein Panorama je Schicht; ein
Instrument mit einer Schicht sieht aus wie heute, nur mit der Liste links,
und die vierzehn mitgelieferten klingen wie vorher.

**Nicht in der Skizze gezeigt, mit E1 a dazugekommen:** der Regler Drive —
wo er in der Mitte steht und wie weit er reicht, setze ich beim Bau.

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

`loop/Instrument.hpp`. Ein Instrument besteht aus einer bis vier
**Schichten** (`Schicht`, Stufe 10c); jede ist ein Satz Einstellungen — das,
was der Instrument-Creator offenlegt. Seit Stufe 10a ist es **eine Datei
`.lvinst`** (I2 b, I3 a), seit 10c in **Fassung 2** (F1 a):

```json
{
  "format": "lumiviz-instrument",
  "version": 2,
  "title": "House Piano Wide",
  "level": 0.8,
  "layers": [
    {
      "name": "Body", "on": true, "level": 1, "transpose": 0, "startOffset": 0, "drive": 0,
      "waveform": "saw",
      "harmonics": [],
      "voices": 3,
      "detune": 9,
      "attack": 160, "decay": 400, "sustain": 0.85, "release": 420,
      "filterCutoff": 6, "filterEnvelope": 0,
      "vibratoRate": 5, "vibratoDepth": 6, "vibratoDelay": 300,
      "damping": 0.996
    }
  ]
}
```

**Fassung 1** trägt die Felder einer Schicht flach auf oberster Ebene, ohne
`layers`. Das Programm liest sie als Instrument mit **einer** Schicht; ihr
`level` wird die Lautstärke der Schicht, das Instrument selbst steht auf 1 —
der Klang ist zahlengleich. Die vierzehn Instrumente aus S110 liegen weiter
in Fassung 1 vor. **Geschrieben wird immer Fassung 2.** Eine **höhere**
Fassung als die eigene liest das Programm nicht: das Muster schweigt, im
Bericht steht „Instrument needs a newer LumiViz: …" (`kInstrumentFassung`).
Das Programm aus S110 prüfte die Nummer nicht — es spielt eine Datei der
Fassung 2 ohne Meldung als reinen Sinus.

| Feld | Bedeutung | Grenzen |
|---|---|---|
| `title` | wie das Instrument im Programm steht; ohne Titel der Dateiname | — |
| `level` (oben) | Lautstärke des ganzen Instruments | 0 bis 1 |
| `layers[]` | die Schichten; fehlt die Liste, gilt eine mit Grundeinstellung | 1 bis 4, weitere bleiben weg |
| `name` | wie die Schicht in der Liste steht; leer = „Layer n" | — |
| `on` | aus = stumm, bleibt im Instrument | — |
| `transpose` | so viele Halbtöne liegt die Schicht neben der Note | -24 bis 24 |
| `startOffset` | so viele Millisekunden nach dem Anschlag setzt sie ein | 0 bis 1000 |
| `drive` | der Verzerrer (E1 a), 0 = aus | 0 bis 1 |
| `waveform` | `sine`, `triangle`, `saw`, `square` oder `string` (die gezupfte Saite) | — |
| `harmonics` | nur `sine`: die Anteile des 2., 3., … Teiltons | höchstens 16, je -1 bis 1 |
| `voices`, `detune` | gleiche Schwingungen übereinander; so viele Cent liegen die äußeren daneben | 1 bis 3; 0 bis 100 |
| `attack`, `decay`, `release` | Hüllkurve, in Millisekunden | 0 bis 5000, `decay` bis 10000 |
| `sustain` | Pegel, solange die Note dauert | 0 bis 1 |
| `filterCutoff`, `filterEnvelope` | Tiefpass als Vielfaches des Grundtons (0 = offen); so viele Vielfache öffnet die Hüllkurve dazu | 0 bis 64 |
| `vibratoRate`, `vibratoDepth`, `vibratoDelay` | Hertz, Cent, Millisekunden bis zur vollen Tiefe | 0 bis 20; 0 bis 100; 0 bis 5000 |
| `damping` | nur `string`: größer klingt länger | 0,9 bis 0,9999 |
| `level` (in der Schicht) | Lautstärke der Schicht | 0 bis 1 |

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

## 4g. Effekte am Clip (Stufe 12 — Skizze gebilligt, Patrik S113; nicht gebaut)

Entscheid **E2 b**: jeder Clip im Loop-Panel trägt eigene Effekte, gleich ob
Drumkit oder Instrument. Die Skizze (zweite Fassung; die erste zeigte einen
Raum für den ganzen Loop und ist verworfen, §3.1):

**Loop-Panel:**

- **Markieren:** ein Klick markiert einen Clip, **Ctrl+Klick** nimmt einen
  dazu oder heraus, Ziehen auf freier Fläche spannt einen Rahmen auf.
  Markierte Clips tragen einen Rand.
- **Rechtsklick auf einen markierten Clip:** das Menü bekommt **Effects...**
  (zwischen Make new base and edit und Duplicate). Es öffnet das Fenster
  „Clip effects" für alle markierten Clips.
- **Marke „fx":** ein Clip, an dem ein Effekt eingeschaltet ist, zeigt sie
  rechts in seinem Block.
- **Clip-Zeile unten:** bei mehreren markierten Clips steht dort ihre Zahl
  und welche Effekte sie haben — bei verschiedenen Ständen „Effects: mixed".

**Fenster „Clip effects":**

- **Kopf:** wie viele Clips, welche (Basis und Takt), rechts **Peak**.
- **Distortion:** **Drive**, **Tone** (dunkler bis heller), **Mix**.
- **Echo:** **Time** als Notenwert (1/16, 1/8, 1/8 dotted, 1/4, 1/2 — folgt
  dem BPM des Loops), **Feedback** (wie viel jede Wiederholung an die nächste
  weitergibt), **Damping**, **Mix**.
- **Reverb:** **Size** (Größe des Raums), **Decay** (Nachklang in Sekunden),
  **Damping** (wie schnell die Höhen im Nachklang verschwinden), **Mix**.
- **Haken je Gruppe:** schaltet den Effekt für die markierten Clips ein oder
  aus.
- **Mix:** der Anteil des bearbeiteten Tons; bei 0 % klingt der Clip
  unverändert.
- **„mixed"** steht an einem Regler, wenn die markierten Clips dort
  verschiedene Werte haben; wer ihn bewegt, setzt den Wert für alle.
- **Unten:** die Zeile „Order: Distortion, Echo, Reverb"; **Copy**, **Paste**
  (die Einstellungen eines Clips auf andere übertragen), **Reset** (alles
  aus).

Die Namen sind Reverb, Echo und Distortion; „Delay" kommt nicht vor (in S111
wurde es als Echo gelesen, wo ein später Einsatz gemeint war).

**Mit der Skizze gebilligt (von mir gesetzt):**

- Die Reihenfolge ist fest: Distortion, dann Echo, dann Reverb. Autotune
  käme später davor.
- Die Effekte gehören zum **Clip**, nicht zur Basis. **Duplicate** nimmt sie
  mit; danach sind die zwei Clips in den Effekten unabhängig, im Muster
  weiter verbunden (L8 a).
- Echo und Hall klingen über das Ende des Clips hinaus aus.
- **Play pattern** im Editor spielt das Muster **ohne** Clip-Effekte — eine
  Basis kann mehrere Clips mit verschiedenen Effekten haben.
- Jede Änderung läuft über dasselbe Undo wie das Arrangement.
- Verschieben, Löschen und Duplicate gelten für alle markierten Clips
  zugleich; eine Kante ziehen bleibt bei einem Clip.
- Die Wertebereiche in der Skizze sind Platzhalter.

**Nicht drin:** freie Reihenfolge der Effekte; ein Effekt zweimal am selben
Clip; gespeicherte Effekt-Vorlagen; Effekte je Kanal oder je Drum-Spur.

**Folgen von E2 b** (Patrik mit der Skizze vorgelegt):

- „Kick trocken, Snare mit Hall" geht innerhalb eines Drumkit-Clips nicht:
  der Effekt trifft den ganzen Clip. Der Weg dahin sind zwei Muster in zwei
  Kanälen.
- Jeder gerade klingende Clip mit Effekt rechnet seinen eigenen Hall im
  Ton-Thread. **Nicht gemessen** — beim Bau zu messen und vorzulegen.
- **Distortion am Clip** verzerrt die Summe des Clips, bei einem Akkord
  reiben die Töne aneinander; **Drive** je Schicht im Instrument (E1 a)
  bleibt daneben und verzerrt je Ton.

**Vor dem Bau geklärt:** das Dateiformat (F2 a, §4) und wo gerechnet wird
(G1 b, §3).

### Die Rechenbausteine (gebaut, 12b Teil 1)

`loop/Effekte.hpp`: `Verzerrer`, `EchoEffekt`, `HallEffekt` und die
`EffektKette` eines Clips (Verzerrer, dann Echo, dann Hall). Sie kennen weder
den Loop noch die Engine; **im Klangerzeuger hängen sie noch nicht** — zu
hören ist nichts. `rechne` und `leere` holen keinen Speicher und dürfen im
Ton-Thread laufen, `richteEin` nicht. Dieselbe Eingabe ergibt dieselbe
Ausgabe, gleich in welchen Blöcken gerechnet wird (geprüft mit Blöcken von 1
bis 8167 Abtastwerten).

- **Verzerrer:** die Kennlinie von Drive je Schicht
  (`tanh(8·Drive·x) / tanh(8·Drive)`), danach ein Tiefpass für **Tone** (0 =
  800 Hz, 1 = 18 kHz). Leises wird lauter, eine Spitze bis 1 bleibt bis 1.
- **Echo:** eine Verzögerung je Seite, so lang wie der Notenwert im Tempo des
  Loops; jede Wiederholung um Feedback leiser; **Damping** legt einen Tiefpass
  in die Rückführung (0 = offen, 1 = 1 kHz).
- **Hall:** die Bauart von Freeverb (acht gedämpfte Kammfilter, vier
  Allpässe je Seite; gemeinfrei). **Decay** bestimmt die Rückführung jedes
  Kamms so, dass er nach dieser Zeit um 60 dB gefallen ist; **Size** streckt
  die Längen (0,6- bis 1,4-fach); **Damping** nimmt die Höhen im Nachklang.
- **Mix** blendet bei allen dreien zwischen unbearbeitet und bearbeitet: bei
  100 % ist nur der bearbeitete Ton zu hören, bei Echo und Hall also kein
  direkter mehr.
- `nachklangSekunden`: wie lange ein Clip nach seinem letzten Ton noch klingt
  — Decay, dazu beim Echo so viele Wiederholungen, bis sie um 60 dB gefallen
  sind.

**Von mir gesetzt, nicht entschieden:** die Bauart des Halls, die
Frequenzen hinter Tone und Damping, die Streckung hinter Size, und dass Mix
überblendet statt dazumischt. Alles nach Rechnung, **nichts gehört**.

**Gemessen** (S113, `test_LoopEffekte.cpp`, Fall „Loop Effekte Messung": 5 s
Rauschen in Blöcken von 441 Abtastwerten, 44100 Hz, je drei Läufe, die
Taktprobe davor und danach ruhig bei rund 10 ms — der Rechner drosselte
nicht):

| Bauart | nur Hall | Verzerrer, Echo und Hall |
|---|---|---|
| Release-clang | 0,93 bis 0,97 ms je Sekunde Ton (rund 0,1 % eines Kerns) | 1,9 bis 2,4 ms (rund 0,2 %) |
| Testing-MSVC | 5,6 ms (rund 0,6 %) | 7,4 bis 7,8 ms (rund 0,8 %) |

Der Hall fällt in der eingestellten Zeit: bei Decay 2 s liegen zwischen zwei
Fenstern im Abstand von 2 s 59,9 bis 60,1 dB (Size 0, 0,5 und 1).
**Nicht gemessen:** ein Lauf, in dem der Rechner drosselt (auf ein Achtel
gerechnet wären es unter Release-clang rund 2 % je Clip — gerechnet, nicht
gemessen); mehrere Ketten zugleich; die Kette im Ton-Thread selbst;
Debug-MSVC.

### Die Ketten im Klangerzeuger (gebaut, 12b Teil 2)

Ein Clip mit Effekten ist jetzt zu hören — im Loop-Panel wie als Clip im
Composer (beide spielen über den Klangerzeuger). Einstellen lässt er sich
noch nur in der Datei; Beispiel: `asset/loop/effects_demo.lvloop`.

- **Rechenkern** (`Rechenkern.hpp`): `plane` legt je Clip mit eingeschaltetem
  Effekt in einem Kanal, der an ist, einen **Bus** an (`Ablauf::busse`: Kanal,
  Takt, Effekte); jeder Schlag nennt seinen Bus. `rechne` schreibt mit
  `busAus` die Schläge eines solchen Clips unbearbeitet in dessen eigenen
  Block statt in die Summe — auch die Schnitt-Blende eines Schlags bleibt in
  seinem Bus. Ohne `busAus` rechnet `rechne` wie bisher alles trocken in die
  Summe; der Kern bleibt ohne Gedächtnis.
- **Klangerzeuger:** ein Programm trägt das Gedächtnis seiner Effekte
  (`EffektZustand`: je Bus eine Kette und ein Block), angelegt mit dem
  Programm außerhalb des Ton-Threads. `render` rechnet in Stücken von
  höchstens 1024 Abtastwerten, schickt jeden Bus durch seine Kette und mischt
  ihn zur Summe. Ohne Clip mit Effekten läuft der Weg von vorher, auf den
  Wert gleich.
- **Verklungen:** kam in einen Bus so lange kein Ton, wie sein Nachklang
  dauert (`nachklangSekunden`), wird er nicht mehr gerechnet, bis wieder Ton
  kommt. Ein Clip kostet also nur, solange er klingt oder nachklingt.
- **Über das Clip-Ende und das Ende des Durchlaufs** klingt der Nachklang
  weiter: der Bus rechnet, solange etwas zu hören ist, gleich wo die Stelle
  steht. Ein Nachklang, der länger ist als ein Durchlauf, ist damit kein
  Sonderfall.
- **Sprung** (G1 b): weicht der Anfang eines Blocks um mehr als 200 ms
  (`kSprungMs`) von dem ab, was auf den letzten folgte, werden alle Ketten
  geleert. Play im Loop-Panel beginnt von vorn und ist ein solcher Sprung —
  ein Rest vom letzten Abspielen ist dann weg.
- **Neues Programm** (jede Änderung am Loop): der Ton-Thread übergibt je Clip,
  den es vorher schon gab — erkannt an Kanal und Takt —, das Gedächtnis an
  die neue Kette. Was passt, geht mit: ändert sich nur das Muster, Mix,
  Feedback, Decay oder Damping, reißt der Nachklang nicht ab. Ändert sich die
  Echo-Zeit oder das Tempo, beginnt das Echo leer; ändert sich Size, der Hall.
  Ein Clip, der verschoben wird, gilt als neuer Clip.
- **Freigeben:** das abgelöste Programm lässt der Ton-Thread nicht selbst
  los, er gibt es ab (`m_abgegeben`); das nächste `setzeProgramm` räumt es
  weg.

**Von mir gesetzt, nicht entschieden:** woran ein Clip wiedererkannt wird
(Kanal und Takt); dass ein verschobener Clip seinen Nachklang verliert; dass
ein Regler, der die Länge einer Verzögerung ändert (Echo-Zeit, Size, Tempo),
den Nachklang dieses Effekts abschneidet; dass ein Bus nach seinem Nachklang
(60 dB) hart aufhört; die Stückgröße 1024.

**Nicht gemessen:** die Last im Ton-Thread selbst und mit vielen Clips
zugleich (die Zahlen oben gelten je Kette, außerhalb); ob das Abschneiden
eines Nachklangs bei 60 dB oder bei einer Änderung von Size zu hören ist.
**Nicht geprüft:** der Weg über den Composer mit Effekten (derselbe
Klangerzeuger, aber kein eigener Fall); zwei Wechsel des Programms innerhalb
eines Blocks. **Nichts gehört** — auch `effects_demo.lvloop` ist nach
Rechnung gesetzt.

---

## 4h. Vorgemerkt (S113, nichts davon gebaut, noch ohne Skizze)

### Instrumente aus Samples (Stufe 11) — Idee, von Patrik nicht verworfen

Patrik: Instrumente sollen sich auch aus Samples erstellen lassen, „es muss
einfach die Tonlage richtig eingestellt werden können".

- Ein Sample ist eine weitere **Art von Schicht** im Instrument-Creator:
  neben Sine, Triangle, Saw, Square und Plucked string steht **Sample**. Es
  lässt sich so mit gerechneten Schichten mischen; Envelope, Filter und Drive
  gelten weiter.
- Für die Tonlage zwei Felder: **Root note** (welcher Ton die Aufnahme ist)
  und **Fine tune** in Cent. Eine Note spielt das Sample um den Abstand zur
  Root note schneller oder langsamer.
- Die Root note stellt man auf drei Wegen ein: **Detect** (das Programm misst
  den Grundton und schlägt vor, etwa „A3 +12 ct"; ohne klaren Ton sagt es
  das), **nach Gehör** (die Klaviatur spielt das Sample, ein Haken
  **Reference tone** legt einen Sinus derselben Note daneben), oder von Hand.
- Dazu **Start** (ab wo in der Datei) und **Loop** mit einem Bereich für
  gehaltene Töne — ein Sample wird bei hohen Noten kürzer.
- **Multisamples** (Patrik S113: gut notieren, bei Bedarf vorziehen): weiter
  als etwa eine Oktave von der Root note klingt ein Sample verfärbt. Abhilfe
  sind mehrere Samples je Tonbereich in einer Schicht. Im ersten Schritt
  nicht drin; **Tests an Sample-Instrumenten sollen zeigen, ob ein Sample
  über mehrere Oktaven trägt** — wenn nicht, werden Multisamples vorgezogen.
  Die „eine Oktave" ist eine Faustregel, nicht gemessen.

### Die Palette der Instrumente (Stufe 14, P1 a)

Im Instrument-Editor steht neben dem Namen des Instruments **Browse...** und
öffnet das Fenster „Instruments" — gebaut nach dem Vorbild des
Palettenfensters der Effektkette (`UI/panels/EffectPaletteWindow.hpp`, S79).
Es bleibt offen und zeigt die Instrumente als Baum nach Art (vorgeschlagen:
Keys, Bass, Lead, Pads and strings, Plucked, Wind, Percussion, Unusual, My
files), mit Suchfeld, Favorites, Recently used und einem Satz Beschreibung.
Ein Klick spielt das Instrument an, Doppelklick oder **Use** setzt es in das
Muster des offenen Instrument-Editors. **Dazu Patrik:** Sortieren oder
Filtern nach **Genre**. Nicht nachgesehen: welche Felder die `.lvinst` dafür
schon trägt — Art, Genre und Kurztext kämen wohl dazu. Die Skizze steht aus.

### Mehrfachauswahl in den Editoren (Stufe 13, Wunsch Patrik S113)

Im Drum-Editor und im Instrument-Editor mehrere Schritte oder Noten zugleich
markieren, die Auswahl kopieren und als Teilmuster an anderer Stelle
einfügen, oder mehrere zusammen verschieben. Heute steht beides in §4d und
§4f unter „Nicht drin". Die Skizze steht aus.

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
| 10b | **Instrument-Creator:** das Fenster, das die Bausteine offenlegt (Skizze gebilligt, §4f), **Edit instrument...** und **Load instrument...** im Instrument-Editor | S110 gebaut, §4f; zwei Fälle in `test_LoopInstrumentEditor.cpp` (ohne Fenster; `LUMIVIZ_INSTRUMENT_CREATOR_BILD=<datei.png>` legt ein Bild ab), dazu das Anspielen im Panel-Fall von `test_LoopSpieler.cpp`; Benutzerhandbuch §16; **im Programm noch nicht gesehen und nicht gehört** |
| 10c | **Instrumente aus Schichten** (Wunsch Patrik S110): mehrere Sätze Einstellungen in einem Instrument, je Schicht Level, Transpose, Start offset und Drive (E1 a); die Spalte Layers im Creator; Fassung 2 der `.lvinst` (F1 a) | S111 gebaut, §4f; `test_LoopInstrument.cpp` (ein Fall zu Schichten und Fassung 2, einer zu den fünf Beispielen), `test_LoopInstrumentEditor.cpp` (ein Fall zum Creator mit Schichten); sechzehn Instrumente aus Schichten und fünf Beispiel-Loops (`asset/loop/make_examples.py`); **im Programm noch nicht gesehen und nicht gehört** |
| 11 | **Instrumente aus Samples:** eine Tondatei, je Note in der Tonhöhe umgerechnet (I1 c); als Art von Schicht, mit Root note, Fine tune und Detect (§4h) | später, davor die Skizze; Multisamples vorgemerkt — vorziehen, wenn Tests an Sample-Instrumenten es verlangen |
| 12 | **Effekte am Clip** (E2 b): Reverb, Echo, Distortion je Clip, für Drumkit- und Instrument-Clips; mehrere Clips markieren; der Rechenkern bekommt dafür ein Gedächtnis | S113: Skizze gebilligt (§4g), Format entschieden (F2 a). **12a gebaut:** Modell und Datei in Fassung 3 (§4), ein Fall mit sechs Teilen in `test_LoopKern.cpp`; nichts davon zu hören oder zu sehen. **12b Teil 1 gebaut:** die Rechenbausteine `loop/Effekte` und die Messung ihrer Last (G1 b, §4g), `test_LoopEffekte.cpp`, sieben Fälle ohne Ton; **12b Teil 2 gebaut:** Busse im Rechenkern, die Ketten im Klangerzeuger mit Sprung-Regel und Übergabe des Nachklangs (§4g), fünf Fälle dazu in `test_LoopEffekte.cpp`, das Beispiel `asset/loop/effects_demo.lvloop` mit einem Fall in `test_LoopInstrument.cpp`; **zu hören, aber nicht gehört** — einstellen lässt es sich noch nur in der Datei. **Offen:** 12c Markieren mehrerer Clips, 12d Fenster Clip effects und Marke fx, 12e Doku und Sichttest-Liste |
| 13 | **Mehrfachauswahl in Drum- und Instrument-Editor:** mehrere Schritte oder Noten markieren, kopieren, einfügen, verschieben (§4h) | später, davor die Skizze |
| 14 | **Palette der Instrumente** (P1 a): Fenster Instruments nach Art, mit Suche, Genre, Anspielen (§4h) | später, davor die Skizze |
| 15 | **Wave-Clips** (Tonspuren wie Gesang) und **Autotune** am Clip (§3.2) | später |

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

- **0.26.0** (S113, 2026-10-11): 12b Teil 2 gebaut — Busse im Rechenkern,
  die Ketten im Klangerzeuger (§4g): Clips mit Effekten sind zu hören;
  Beispiel `asset/loop/effects_demo.lvloop`.

- **0.25.0** (S113, 2026-10-11): Entscheid G1 b (die Effekte rechnen im
  Ton-Thread mit, mit einer Regel für den Sprung), a und c verworfen; 12b
  Teil 1 gebaut — die Rechenbausteine Verzerrer, Echo, Hall und die Messung
  ihrer Last (§4g).

- **0.24.0** (S113, 2026-10-11): Entscheid F2 a (Feld `effects` am Clip,
  Fassung 3 der `.lvloop`), b verworfen; Stufe 12a gebaut — die Effekte im
  Modell und in der Datei (§4), Grenzen und Vorgaben von mir gesetzt; Stufe
  12 in 12a bis 12e geteilt.

- **0.23.0** (S113, 2026-10-11): Entscheid E2 b — jeder Clip trägt eigene
  Effekte (Reverb, Echo, Distortion), a und die Fassungen je Muster und je
  Kanal verworfen; die gebilligte Skizze für Stufe 12 (§4g neu). Entscheid
  P1 a — Palettenfenster für Instrumente, mit Genre. Autotune zurückgestellt
  bis zu den Wave-Clips (§3.2). Vorgemerkt in §4h neu: Samples als Schicht
  mit Root note und Detect, Multisamples, Mehrfachauswahl in den Editoren.
  Stufen 11 und 12 neu gefasst, 13 bis 15 dazu. Nichts gebaut.

- **0.22.0** (S111, 2026-10-11): neunzehn Beispiel-Loops nach Genre unter
  `asset/loop/genres` und neunzehn Instrumente dazu (Wunsch Patrik),
  erzeugt von `asset/loop/make_genres.py`, beschrieben in
  `asset/loop/README.md`; am Programm war dafür nichts zu ändern. Der Blues
  nutzt drei Schritte je Schlag, der Walzer drei Schläge je Takt.
- **0.21.1** (S111, 2026-10-11): Drum-Editor (§4d) — Befund Patrik: bei
  einem Muster ohne Spuren und hohem Fenster standen Kopf, Raster-Zeile und
  Spuren weit auseinander. Die Luft bleibt jetzt unten; ein leeres Muster
  zeigt den Hinweis „This pattern has no tracks yet — Add track creates one."
- **0.21.0** (S111, 2026-10-11): Entscheid F1 a (Fassung 2 der `.lvinst`
  mit der Liste `layers`), b und c verworfen; Stufe 10c gebaut — Schichten
  in Modell, Datei, Rechnen und Creator, Drive (§4f); sechzehn Instrumente
  aus Schichten, fünf Beispiel-Loops.
- **0.20.0** (S111, 2026-10-11): die gebilligte Skizze für Stufe 10c —
  Instrumente aus Schichten (§4f); Entscheid E1 a (Verzerrer im Instrument,
  Hall und Echo am Muster), b und c verworfen; Stufen 10c und 12.
- **0.19.0** (S110, 2026-10-11): Stufe 10b gebaut — der Instrument-Creator,
  Edit instrument..., Load instrument... (§4f).
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

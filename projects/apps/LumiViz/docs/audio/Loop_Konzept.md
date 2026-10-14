# LumiViz — Loop (Konzept)

> **Version:** 0.72.0
> **Datum:** 2026-10-14 (Session 124)
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
| **V1 a** (S113) | **Einfügen im Drum-Editor ersetzt den Bereich.** Ctrl+V überschreibt am Ziel genau den kopierten Bereich — wo die Kopie einen Schritt hat, steht danach einer, wo sie leer ist, ist es danach leer. Im Menü des Rechtsklicks zusätzlich **Paste and keep existing steps**, das nur dazulegt. Grund: wer Takt 1 nach Takt 3 kopiert, erwartet dort dasselbe, keine Mischung. Skizze der Stufe 13 in §4i. |
| **P1 a** (S113) | **Instrumente wählt man in einem Palettenfenster wie bei der Effektkette**, nicht mehr in einer Dropdown-Liste: ein Baum nach Art, Suchfeld, Favorites, Recently used, ein Satz Beschreibung, Anspielen per Klick. Dazu Patrik: auch **Sortieren oder Filtern nach Genre** muss gehen. Beschrieben in §4h; eigene Stufe 14. |

### 3.1 Verworfen, mit Grund

- **Einfügen im Drum-Editor legt dazu** (V1 b): nichts ginge verloren, was man
  nicht gesehen hat, und zwei Figuren ließen sich übereinanderlegen — aber
  ein kopierter Takt ergäbe am Ziel eine Mischung, und Undo holt das
  Überschriebene bei a zurück. Als zweiter Eintrag im Menü bleibt es.

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

### Mehrere Clips markieren (gebaut, 12c)

`loop/Taktachse` und, als reine Regeln, `loop/Bearbeiten.hpp` (`ClipOrt`,
`versetzeClips`, `dupliziereClips`, `loescheClips`). Gebaut wie die Skizze:
Klick, **Ctrl+Klick**, Rahmen auf freier Fläche; markierte Clips tragen den
Rand in der Markierfarbe; die Marke **fx** an jedem Clip mit eingeschaltetem
Effekt; die Clip-Zeile nennt bei mehreren ihre Zahl und wie viele davon
Effekte haben. **Effects...** im Menü fehlt noch (12d).

**Dazu von mir gesetzt, nicht entschieden:**

- **Gruppe ziehen:** alle markierten rücken um dieselben Takte und Kanäle,
  ihre Lage zueinander bleibt. Anders als ein einzelner Clip springt die
  Gruppe **nicht** an eine freie Kante: ist unter der Maus für einen von
  ihnen kein Platz, bleibt sie an der letzten Stelle stehen, an der Platz
  war. Der Versatz endet an Takt 1 und am ersten und letzten Kanal.
- **Duplicate** bei mehreren: die Gruppe als Ganzes, an die erste Stelle
  hinter ihr, an der alle Kopien Platz haben; markiert sind danach die
  Kopien. Die Effekte gehen mit, die Kopien bleiben verbunden.
- **Ein Klick** auf einen von mehreren markierten lässt beim Loslassen nur
  ihn markiert; wird stattdessen gezogen, zieht die Gruppe.
- **Mit Ctrl** kommt auch ein Rahmen zu den Marken dazu. **Ctrl+A** und
  **Select all clips** im Menü der freien Fläche markieren alle.
- **Der Rechtsklick** auf einen von mehreren markierten lässt die Marken
  stehen; das Menü nennt dann „Duplicate n clips" und „Delete n clips",
  **Edit base**, **Make new base and edit** und **Rename base...** sind grau
  — eine Basis gehört zu einem Clip.
- Ohne Auswahl sagt die Clip-Zeile, wie man mehrere markiert („Ctrl+click or
  drag a frame on empty space to select several clips").
- Ein Clip gilt im Programm als „derselbe" über Kanal und Starttakt
  (`ClipOrt`). Ändert eine Bearbeitung von außen die Lage markierter Clips
  (etwa **Bars** im Editor, das Clips nach hinten schiebt), fallen sie aus
  den Marken.

**Nicht drin:** Shift+Klick für einen Bereich; die Gruppe an den Kanten
gemeinsam verlängern; Kopieren und Einfügen über die Ablage.

Geprüft in `test_LoopArrangement.cpp` (ein Fall zu den Regeln, einer zur
Achse mit sieben Teilen); `LUMIVIZ_LOOP_MARKEN_BILD=<datei.png>` legt ein
Bild der Achse mit zwei markierten Clips und einer Marke fx ab — angesehen.
**Nicht geprüft:** das Menü des Rechtsklicks (es öffnet ein Fenster); die
Clip-Zeile des Panels bei mehreren. **Im Programm nicht gesehen.**

### Das Fenster „Clip effects" (gebaut, 12d)

`loop/ClipEffektFenster.hpp`, ein Werkzeugfenster wie die Editoren. Geöffnet
über **Effects...** im Menü eines Clips (bei mehreren markierten „Effects of
n clips..."; `Taktachse::verlangeEffekte`, `LoopPanel::oeffneEffekte`).
Gebaut wie die Skizze: Kopf mit Zahl, Clips (Basis und Takt) und Peak; die
drei Gruppen mit Haken und Reglern; „mixed"; unten die Reihenfolge, **Copy**,
**Paste**, **Reset**. Jede Änderung läuft als „Clip effects" über das Undo
des Panels und gilt beim Spielen ab dem nächsten Block.

**Dazu von mir gesetzt, nicht entschieden:**

- Das Fenster **folgt den Marken**, solange es offen ist: wer einen anderen
  Clip anklickt, sieht dessen Effekte. Ohne markierten Clip ist es gesperrt
  („No clip selected — Select one or more clips in the Loop panel").
- Ein Haken, der bei mehreren Clips „teils an" zeigt, schaltet beim Klick
  alle an.
- Ein Schieber meldet beim Loslassen; im Zug läuft nur seine Anzeige mit.
  Tasten, Mausrad und ein Klick in die Bahn melden sofort.
- Die Regler: Drive, Tone, Mix, Damping und Size in Prozent; Feedback bis
  90 %; Decay 0,2 bis 8,0 s in Zehnteln; **Time** als Auswahl (1/16, 1/8,
  1/8 dotted, 1/4, 1/2) mit den Millisekunden im Tempo des Loops daneben.
- **Copy** merkt die Effekte des **ersten** gezeigten Clips, nur für dieses
  Fenster und bis es geschlossen wird — nicht in der Ablage des Systems.
  **Reset** stellt alle drei Effekte der markierten Clips auf die Vorgabe
  (aus).
- Ein Regler wirkt auch, wenn sein Effekt aus ist: der Wert steht dann
  bereit, zu hören ist nichts.
- Größe und Lage des Fensters merkt die App nicht (wie bei den Editoren).

**Nicht drin:** Vorhören eines einzelnen Clips aus dem Fenster; die Effekte
eines einzelnen Clips in der Clip-Zeile des Panels (dort steht nur bei
mehreren, wie viele Effekte haben).

Geprüft in `test_LoopClipEffekte.cpp` (ein Fall zum Fenster mit fünf Teilen,
einer zum Weg durchs Panel mit Undo); `LUMIVIZ_CLIP_EFFEKTE_BILD=<datei.png>`
legt ein Bild des Fensters ab — angesehen. **Nicht geprüft:** das Menü des
Rechtsklicks selbst; das gezeigte Fenster (Fokus, Lage über dem Panel).
**Im Programm nicht gesehen und nicht gehört.**

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
noch nur in der Datei; Beispiel: `asset/loop/effects_demo.lvloop`. (Seit
12d über das Fenster „Clip effects".)

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

**Skizze gebilligt (Patrik, S115) — der Creator mit einer Sample-Schicht.**
Nichts davon ist gebaut. Der Creator bleibt, wie er ist; neu ist nur, was
eine Schicht der Art Sample braucht:

- In **Waveform** steht als sechster Eintrag **Sample**. In der Liste der
  Schichten heißt sie „1 Cello · Sample".
- Unter Waveform, nur bei Sample: **File** (Name der Datei und ihre Länge,
  „cello_a3.wav · 2.4 s") mit dem Knopf **File...**; **Root note** (Auswahl
  der Tonhöhe) mit dem Knopf **Detect**, darunter das Ergebnis „Detected: A3
  +12 ct" mit **Apply** — erst Apply übernimmt Note und Fine tune; ohne
  klaren Ton steht dort „No clear pitch found"; **Fine tune** in Cent;
  **Sample start** in Millisekunden; der Haken **Sustain loop** („repeat a
  part while the note is held") mit **Loop from** und **Loop to** in
  Sekunden.
- Bei einer Sample-Schicht ausgeblendet: Voices, Detune, Harmonics, Damping,
  Vibrato. Level, Transpose, Start offset, Envelope, Filter und Drive gelten
  weiter.
- Rechts oben zeigt das Bild statt „One cycle" die Aufnahme („The sample"):
  eine rote Marke für Sample start, ein blau hinterlegter Bereich für den
  Sustain loop; beide lassen sich im Bild ziehen und stehen zugleich als
  Regler in der Mitte. Darunter wie bisher die Hüllkurve einer Note, die
  Klaviatur und „Selected layer only".
- Neu unter der Klaviatur der Haken **Reference tone**: beim Anspielen
  erklingt ein Sinus derselben Note neben dem Sample, zum Stimmen nach Gehör.

**Namen, von Claude gesetzt und mit der Skizze gebilligt:** „Sustain loop"
statt „Loop" (Loop heißt im Programm das Panel und die Datei), „Sample
start" statt „Start" („Start offset" gibt es an der Schicht schon und meint
den späteren Einsatz).

**Entscheid S1 a (Patrik, S115) — wo eigene Samples liegen:** als
`<name>.wav` im Benutzerordner unter `samples`; eine Schicht oder eine
Drum-Spur nennt sie als `user:<name>` — dieselbe Regel wie bei den eigenen
Instrumenten (P2 a). Export und Import bekommen einen Bereich „Samples".
Eine Datei an freiem Ort bleibt möglich und steht beim Pfad. Verworfen: b
(das Sample liegt in einem Ordner beim Instrument — für Drum-Spuren gäbe es
keinen Ort), c (kein fester Ort, Save fragt nach der Datei).

**Entscheid S2 a (Patrik, S115) — eine über File... gewählte Datei:** ein
kleiner Dialog fragt „Copy the file to My samples (recommended)" oder „Use
it where it is"; die Kopie heißt `user:<name>`, bei einem vergebenen Namen
mit anderem Inhalt mit einer Zahl dahinter; ein Haken „Don't ask again"
merkt die Wahl, in den Settings zurückzustellen. Dasselbe gilt für „Sample
file..." im Drum-Editor (dort nachzuziehen). Verworfen: b (immer kopieren),
c (nie kopieren, wie heute bei den Drums).

**Idee Patrik (S115) — „Record sample":** ein Sample aufnehmen, in
Dauerschleife anhören und zuschneiden, bis es passt; gespeichert wird unter
`samples`. Auch für Drum-Spuren. Skizze gezeigt, **noch nicht gebilligt**:
ein Fenster „Record sample" mit **Input** (Auswahl des Eingangs) und Pegel,
**Record** und **Stop** („3.2 s recorded · up to 30 s"), dem Wellenbild mit
zwei Griffen für Anfang und Ende (das Graue fällt beim Speichern weg),
**Play** mit dem Haken **Repeat**, „Trimmed: 0.46 s to 2.33 s · 1.87 s",
den Haken **Trim silence at both ends** und **Normalize level**, **Name**,
darunter „File: My files · samples · table_knock.wav", **Cancel** und
**Save and use**. Geöffnet aus dem Creator (**Record...** neben File...)
und aus dem Drum-Editor (in der Klangliste einer Spur **Record sample...**,
dazu ein Eintrag **My samples** mit den eigenen). Befund dazu: Ton aufnehmen
kann das Programm heute nicht (nur die Kamera-Testaufnahme); dass BASS es
kann, ist hier nicht belegt — vor dem Bau ein Beleg-Test.

**Skizze „Record sample" gebilligt (Patrik, S115)** — das Fenster und der
Weg aus dem Creator. **Offen ist der Teil zum Drum-Editor:** Patrik fragt,
ob der Drum-Editor nicht dem Creator angepasst werden oder beide „gar eins
werden" sollen, „mit etwas unterschiedlichen Optionen". Vor 11c als
Entscheid mit Skizze vorzulegen; bis dahin wird am Drum-Editor nichts
gebaut.

Dazu festgehalten (S115, auf Patriks Frage, warum die Drums als gerechnete
WAV-Dateien entstanden und nicht in einem Creator): es lag an der
Reihenfolge — in S108 gab es noch nichts, das Töne rechnet, und L5 fragte
„rechnen oder beziehen", nicht „Datei oder Creator"; dass die Drums den
Klangerzeuger der Instrumente (S110) nutzen könnten, wurde danach nicht
vorgelegt. Im Kern sind Drum-Schlag und Ton schon dasselbe (ein vorab
gerechneter `Klang`). Dem Creator fehlen für das heutige Kit
(`asset/loop/kit/make_kit.py`): Rauschen (Snare, Hi-Hat, Clap, der Klick
der Kick), eine fallende Tonhöhe (Kick, Tom), Hochpass und Bandpass
(Hi-Hat, Clap). Die drei kurzen Schläge des Clap gingen ohne neuen Regler
als vier Schichten mit je eigener Hüllkurve und Start offset — das füllt
`kSchichtenMax`. **Drum-Ausbau, als Vorschlag, nichts davon in Arbeit:**
A — eine Drum-Spur kann ein Instrument nennen (bearbeitet im Creator);
B — Rauschen als Schwingung und eine fallende Tonhöhe; C — Hochpass und
Bandpass. Nicht bekannt: ob ein im Creator nachgebautes Kit klingt wie das
heutige, das Patrik in S108 für gut befand.

**Entscheid K1 a (Patrik, S115) — Drums und Creator, „gleich das ganze":**
eine Drum-Spur nennt einen Klang des Kits, ein eigenes Sample oder ein
Instrument; an jeder Spur macht **Edit sound...** aus dem, was sie spielt,
ein Instrument (ein Kit-Klang oder Sample wird ein eigenes Instrument mit
einer Sample-Schicht) und öffnet den Creator. Das Kit bleibt als WAV, alte
Loops lesen weiter. Dazu gehören — nicht als Möglichkeit für später, sondern
als Plan — die Stufen **11d** (Rauschen, fallende Tonhöhe), **11e**
(Hochpass, Bandpass) und **11f** (sechs gerechnete Drum-Instrumente neben
dem Kit, mit Hörprobe gegen das heutige). Verworfen: die drei Quellen ohne
Edit sound..., nur eigene Samples, und „ganz eins" (nur noch Instrumente,
das WAV-Kit ersetzt). Skizze der Klangliste gezeigt (Kit, My samples,
Instrument..., Sample file..., Record sample...; an einer Instrument-Spur
ein Knopf „…" mit Note, Length, Play, Browse..., Edit instrument...) —
**Skizze gebilligt (Patrik, S115).** Dafür noch zu lösen, nicht geprüft: eine
Sample-Schicht spielt halb so laut wie die Datei (L6 a), das Umwandeln muss
das ausgleichen; eine Sample-Schicht kann noch keinen Klang des Kits nennen
(`kit:<name>` käme als dritte Art dazu). Die `.lvloop` bekäme Fassung 4.

**Gebaut — 11f (S117): sechs gerechnete Drum-Instrumente neben dem Kit.**
`asset/loop/instruments/drum_kick`, `drum_tom`, `drum_snare`, `drum_clap`,
`drum_hat_closed`, `drum_hat_open` (`.lvinst` Fassung 6, Art `drums`),
geschrieben von `asset/loop/make_drums.py`; im Fenster Instruments stehen sie
unter **Drums** — der Kopf erscheint damit zum ersten Mal. Gebaut nur aus dem,
was eine Schicht des Creators kann; Vorbild sind die sechs Klänge des Kits.
**Alle sechs sind für die Note C2 gebaut** (was eine Drum-Spur der Art Drums
vorbelegt, K2 a): wohin der Schlag gehört, steht als Transpose in der Schicht,
weil der Filter der Tonhöhe folgt und bei C2 selbst nur bis 4,2 kHz reichte.
Eine andere Note verschiebt den ganzen Schlag. Aufbau: Kick — Sinus bei F#1,
fällt aus 22 Halbtönen in 80 ms, Drive 19 %, dazu ein Klick aus Rauschen;
Tom — Sinus bei A2, fällt aus 10 Halbtönen in 120 ms, Klick; Snare — Sinus bei
185 Hz, ein zweiter bei 165 Hz mit seinem zweiten Teilton (330 Hz), Rauschen
über 1,8 kHz; Clap — vier Schichten Rauschen durch einen Bandpass um 1,7 kHz,
je 10 ms später, die letzte klingt aus, Drive 40 % hebt den Pegel; Hi-Hats —
eine Schicht Rauschen über 5,8 kHz, Decay 70 und 450 ms. Die langen Schläge
tragen ein großes Release (bis 1 s), damit sie nach den 250 ms der Spur
ausklingen statt abzubrechen.
**Gegen das Kit gemessen, nicht gehört** (das Programm rechnet den Schlag bei
C2 und 250 ms, `.claude/handover/hoerproben_11f/vergleich.py` misst gegen
`asset/loop/kit/*.wav`; Hüllkurve: 1 = gleicher Verlauf; Bänder: Abstand der
Pegel in Drittel-Oktaven, mittel / größter):

| | Hüllkurve | Bänder | klingt (Kit / Instrument) |
|---|---|---|---|
| Kick | 0,91 | 4,9 / 15,2 dB | 439 / 360 ms |
| Tom | 0,94 | 0,6 / 1,3 dB | 315 / 308 ms |
| Snare | 1,00 | 1,6 / 13,6 dB | 182 / 163 ms |
| Clap | 0,97 | 4,7 / 12,5 dB | 200 / 186 ms |
| Hi-Hat Closed | 1,00 | 3,6 / 6,1 dB | 50 / 56 ms |
| Hi-Hat Open | 1,00 | 3,3 / 6,0 dB | 398 / 387 ms |

**Stand S118 (Stufe 16e): Kick und Clap sind mit der Reihe einer Schicht neu
gebaut** — Aufbau und die neuen Zahlen neben diesen stehen in §4j. Was hier
zu Kick und Clap steht, ist der Stand von S117.

Die Werte sind aus Reihen gewählt (Kick 18, Tom 18, Snare 6, Clap 54, Hi-Hats
21 Varianten), nach diesen Zahlen. **Leiser als das Kit:** die Spitze eines
Schlags liegt bei 0,42 bis 0,58, die eines Kit-Klangs bei 0,9 — eine Schicht
bei vollem Level kommt nicht höher. **Anders als das Kit, weil eine Schicht es
nicht kann:** der Kick klingt kürzer und ist in den Bändern am weitesten weg;
der Clap hat einen Filter statt zwei; die Hi-Hats sind Rauschen allein, ohne
die sechs Rechtecke (deren Frequenzen liegen von C2 aus außerhalb von
Transpose); der Klick dauert 10 ms statt 3,6. Ob dafür die drei vorgemerkten
Bausteine nötig sind (Drive hinter der Hüllkurve, zwei Filter, die
Hi-Hat-Schwingung), hängt weiter an Patriks Ohr — Hörproben unter
`.claude/handover/hoerproben_11f/` (`vergleich_<klang>.wav`: zweimal das Kit,
zweimal das Instrument, auf dieselbe Spitze gebracht). Von Claude gesetzt,
nicht entschieden: Namen (`drum_…`) und Titel (Kick, Tom, Snare, Clap, Hi-Hat
Closed, Hi-Hat Open), alle für C2, Snare mit Level 60 % (sonst fast doppelt so
laut wie die anderen), die Beschreibungen. Die mitgelieferten Loops spielen
weiter das Kit. `instrument_katalog.py` lässt einer Datei jetzt ihre Fassung,
wenn sie über 3 liegt. Im Programm nicht gesehen und nicht gehört.

**Entscheid K2 a (Patrik, S115) — Note und Länge einer Instrument-Spur:**
jede solche Spur trägt **Note** und **Length** in Millisekunden; vorbelegt
die Note nach der Art des Instruments (Bass C2, sonst C4) und die Länge mit
250 ms; Edit sound... stellt die Note auf den Root note der Schicht und die
Länge auf die der Aufnahme. Ein Schlag klingt bei jedem Tempo gleich.
Verworfen: b (Länge in Schritten), c (keine Länge, der Klang spielt aus).

**Rechenversuch (S115, auf Patriks Frage: lassen sich alle Klänge des Kits
im Creator bauen?)** — ein Modell in Python, nicht der Code des Programms
(lokal: `.claude/handover/hoerproben_drumkit/kit_aus_creator.py`, daneben
die Hörproben). Jeder Klang aus `make_kit.py` wurde aus Schichten
nachgebaut, wie der Creator sie heute hat, dazu Rauschen, fallende Tonhöhe,
Hochpass und Bandpass. Verglichen sind die Hüllkurve (Korrelation) und der
Pegel in 27 Drittel-Oktaven (Abstand in dB); zur Einordnung: die Snare gegen
sich selbst mit anderem Rauschen ergibt 0,997 und 0,4 dB.

| Klang | Aufbau | Hüllkurve | Bänder, mittel / größter Abstand |
|---|---|---|---|
| tom | Sinus mit fallender Tonhöhe, Rauschen als Klick | 1,000 | 0,0 / 0,0 dB |
| snare | zwei Sinus (F#3 und Transpose +10), Rauschen mit Hochpass | 1,000 | 0,0 / 0,6 dB |
| kick | wie tom, mit Drive | 0,990 | 5,6 / 15,2 dB |
| kick, Drive hinter der Hüllkurve | gibt es nicht | 0,998 | 0,6 / 1,5 dB |
| clap | vier Schichten Rauschen mit Bandpass, je später einsetzend | 0,998 | 6,9 / 29,0 dB |
| clap, Hochpass und Bandpass hintereinander | gibt es nicht | 0,999 | 0,0 / 0,0 dB |
| hat, nur Rauschen mit Hochpass | eine Schicht | 0,996 | 4,2 / 10,1 dB (offen 3,7 / 9,0) |
| hat, drei Rechtecke und Rauschen | vier Schichten | 0,998 | 4,3 / 13,2 dB (offen 4,0 / 11,2) |
| hat, sechs Rechtecke in einer Schicht | gibt es nicht | 0,999 | 0,1 / 0,2 dB |

(Die Hüllkurve der Kick ist in Stücken von 25 ms gemessen; in Stücken von
2 ms ergibt sie 0,82 bis 0,87 — ein Stück ist dann kürzer als eine
Schwingung von 45 Hz, und der Wert folgt der Phase.)
**Ergebnis:** Tom und Snare gehen mit 11d und 11e vollständig. Für die
übrigen drei fehlt je ein Baustein, der in 11d und 11e nicht vorgesehen
war: **Drive hinter der Hüllkurve** (Kick — der Verzerrer des Creators
wirkt vor der Hüllkurve und lässt deshalb nicht mit dem Ton nach), **zwei
Filter hintereinander** (Clap — ohne den Hochpass vor dem Bandpass sind die
Tiefen bis 29 dB zu laut), **sechs unharmonische Rechtecke in einer
Schicht** oder mehr als vier Schichten (Hi-Hats). Dazu Kleineres: die Kick
endet bei 45 Hz, zwischen zwei Halbtönen (46,2 Hz im Nachbau); die Regler
Decay und Attack stehen in Schritten von 10 ms bzw. 1 ms, ein Klick von
3,6 ms oder ein Schlag von 12 ms lässt sich in der Datei, aber nicht am
Regler stellen. **Nicht bekannt:** wie die Abstände klingen — die Zahlen
sagen, wo es abweicht, nicht ob es stört; dafür sind die Hörproben da.

**Entscheid K3 a (Patrik, S115) — eine neunte Art „Drums":** das Fenster
Instruments und der Creator bekommen die Art **Drums**, zwischen Percussion
und Unusual; Edit sound... legt neue Instrumente mit dieser Art an; öffnet
eine Drum-Spur das Fenster, steht es auf Drums, die anderen Arten bleiben
wählbar; Preview note für Drums ist C2. Verworfen: b (unter Percussion),
c (keine eigene Art).

**Vorgemerkt (Patrik, S115: „kann man das, was als fehlt gelistet ist, noch
hinzufügen? vielleicht später?") — drei Bausteine über 11d und 11e hinaus,
entschieden wird nach den Hörproben:**
- **Ein zweiter Filter je Schicht**, hinter dem ersten (für den Clap:
  Hochpass, dann Bandpass). Patrik: eine Kaskade von Filtern oder Effekten
  wäre ein Add-on. Gemeint ist ein Filter in der Schicht des Instruments —
  nicht die Effekte am Clip (Distortion, Echo, Reverb, Stufe 12), die den
  fertigen Clip bearbeiten.
- **Drive hinter der Hüllkurve** als Wahl neben dem heutigen Drive davor
  (für die Kick: der Verzerrer lässt dann mit dem Ton nach).
- **Eine Schwingung aus sechs Rechtecken in unharmonischem Abstand** (für
  die Hi-Hats) — verwandt mit den Harmonics des Sinus, aber mit festen,
  nicht ganzzahligen Verhältnissen, deshalb ohne klare Tonhöhe, metallisch.

**Wunsch Patrik (S115) — die Kaskade als Kette, „in einer Art Node-Editor",
auch für die Effekte:** nicht zwei feste Plätze für Filter, sondern eine
Folge von Bausteinen, die sich hinzufügen, entfernen und umordnen lassen —
je Schicht im Creator, und ebenso für die Effekte am Clip (heute drei feste
in fester Reihenfolge). Vorgemerkt als eigene Stufe, Vorlage mit Skizze
nach 11c. Dazu festgehalten: innerhalb einer Schicht läuft der Ton heute
durch feste Stationen — Schwingung, Filter, Drive, Hüllkurve, Level
(`rechneSchwingung`); die Schichten selbst haben keine klangliche
Reihenfolge, sie werden addiert. Mit einer Kette je Schicht wären zwei der
drei fehlenden Bausteine von selbst da: ein zweiter Filter ist ein weiterer
Baustein, „Drive hinter der Hüllkurve" heißt, Drive ans Ende zu ziehen.
Nicht geprüft: was vom Unterbau der Effect Chain der Visuals sich für Ton
wiederverwenden lässt.

**Gebaut — 11c, Schritt 1 (S115), der Kern ohne Fenster:** die `Spur`
trägt neben `klang` jetzt `instrument`, `hoehe` (Note) und `dauerMs`
(Length, 1 bis 30000, vorbelegt 250); `Spur::spielt()` nennt, was der
Rechenkern für sie spielt — den Klang oder den Ton ihres Instruments
(`inst:<Instrument>:<Note>:<Länge>`, derselbe Weg wie eine Note eines
Instrument-Musters). `klang` kann ein eigenes Sample nennen (`user:<name>`,
`istEigenerKlang`); `klangDatei` findet seine Datei. **`.lvloop` in
Fassung 4:** `sound` wie bisher (ein eigenes Sample steht da, wie es heißt,
nicht als Pfad), bei einer Instrument-Spur dazu `instrument` (eine Datei
relativ zum Loop, wenn sie darunter liegt), `note`, `length` — `sound`
bleibt daneben stehen, sodass ein älteres Programm den Klang spielt.
Fassung 2 und 3 lesen weiter. Im Bericht: „Sample not found: user:…",
„File not found: …" für die Datei eines Instruments. Eine Sample-Schicht
kann einen Klang des Kits nennen (`kit:kick`, `sampleDatei`). Die Art
**Drums** steht als neunte zwischen Percussion und Unusual, ihre Preview
note ist C2.
**GEÄNDERT gegenüber 11a:** eine Sample-Schicht spielt die Aufnahme so
laut, wie sie ist — nicht mehr halb so laut wie gerechnete Schichten. Bei
der Note ihres Grundtons, voll gehalten und so lang wie die Aufnahme,
ergibt sie dieselben Werte wie die Datei (im Test Abtastwert für
Abtastwert). Das ist die Voraussetzung dafür, dass eine Spur nach „Edit
sound..." klingt wie vorher. Folge: ein Instrument mit einer Sample-Schicht
ist doppelt so laut wie nach 11a; mit weiteren Schichten kann die Summe
über 1 gehen — dafür gibt es Level.
Von Claude gesetzt, nicht entschieden: die Änderung der Lautstärke oben;
`sound` bleibt an einer Instrument-Spur stehen; jede `.lvloop` wird als
Fassung 4 geschrieben; die Grenzen von Length. Nicht gebaut: alles im
Drum-Editor (er zeigt und stellt weiter nur `klang`), Edit sound...,
das Fenster Instruments aus einer Drum-Spur. Nicht geprüft: eine echte
Datei des Kits durch eine Sample-Schicht bei einer anderen Abtastrate als
48000 Hz (dort wird geradlinig umgerechnet, die Werte sind dann nicht
dieselben wie beim direkten Dekodieren). Im Programm ist nur die Art Drums
zu sehen: in der Liste Type des Creators.

**Gebaut — 11c, Schritt 2 (S116), der Drum-Editor:** die Auswahl **Sound**
einer Spur ist jetzt die Klangliste der gebilligten Skizze. Von oben nach
unten: die Überschrift **Kit** mit den Klängen des Kits; **My samples** mit
den eigenen Samples (nur wenn es welche gibt; ein genanntes, das fehlt, steht
als „<name> (not found)" da); **File** mit der Tondatei an freiem Ort, wenn
die Spur eine nennt; nach einer Trennlinie die drei Wege **Instrument...**,
**Sample file...**, **Record sample...**. Die Überschriften sind grau und
lassen sich nicht wählen. Spielt die Spur ein Instrument, steht ganz oben
sein Titel (wie im Instrument-Editor, etwa „Slap Bass" oder „Mein Schlag (My
files)"), und er ist gewählt.
- **Instrument...** öffnet das Fenster Instruments für diese Spur; die Zeile
  dort heißt „For track: <Spur> (<Muster>)". **Use** setzt das Instrument in
  die Spur: die Note nach seiner Art (Bass und Drums C2, sonst C4), Length
  250 ms (K2 a). Hatte die Spur schon ein Instrument, bleiben Note und Length.
- **Sample file...** fragt nach einer Tondatei und dann wie der Creator, ob
  sie kopiert wird oder am Ort bleibt (S2 a; dieselbe gemerkte Wahl). Die
  Frage steht jetzt einmal in `loop/SampleFrage`, Creator und Drum-Editor
  stellen sie beide.
- **Record sample...** öffnet das Fenster Record sample; **Save and use**
  setzt die Aufnahme als Klang der Spur.
- Wer einen Klang aus der Liste wählt, nimmt der Spur ihr Instrument: sie
  spielt wieder den Klang, Note und Length stehen auf ihrer Vorgabe.
- **Der Knopf „…"** steht nur an einer Spur, die ein Instrument spielt (sein
  Platz bleibt sonst frei, damit die Regler in einer Flucht stehen). Er
  öffnet ein kleines Feld unter sich: oben „<Spur> plays <Instrument>",
  darunter **Note** (C1 bis C7), **Length** (1 bis 30000 ms, in Schritten von
  10) und **Play**, darunter **Browse...** (das Fenster Instruments) und
  **Edit instrument...** (der Creator; was er speichert, nennt die Spur
  danach — wenn sie dann noch ein Instrument spielt). Ein Klick daneben
  schließt das Feld.
- **Play** spielt einen Schlag der Spur: das Panel baut ein Muster aus dieser
  einen Spur mit einem Schlag am Anfang (`LoopPanel::spieleSpur`), mit ihrer
  Lautstärke und ihrem Panorama, so lang, dass es nicht von vorn beginnt,
  bevor der Ton verklungen ist. Spielt der Loop, bleibt Play stumm.
- Der Drum-Editor hat sein eigenes Fenster Instruments, seinen eigenen
  Creator und sein eigenes Fenster Record sample — wie der Instrument-Editor
  die seinen. Favorites und Recently used liest ein Fenster Instruments
  deshalb bei jedem Öffnen neu aus den Einstellungen.
- Fällt eine Spur weg (auch durch Undo), gelten die offenen Fenster keiner
  Spur mehr: Use im Fenster Instruments ist grau, das Feld schließt.
Von Claude gesetzt, nicht entschieden: das Feld hinter „…" als Feld, das sich
bei einem Klick daneben schließt (die Skizze nannte nur den Knopf und seinen
Inhalt); die Überschrift **File**; „(not found)"; Note im Feld nur C1 bis C7
(die Datei erlaubt 0 bis 127 — eine Note von außerhalb steht da, wie sie ist);
ein anderes Instrument lässt Note und Length stehen; die Spalte Sound ist
124 statt 100 Punkte breit, das Fenster entsprechend breiter; die Wörter für
Undo („Change sound", „Change note", „Change length"). Nicht gebaut: **Edit
sound...** und das Fenster Instruments, das sich aus einer Drum-Spur auf
Drums stellt (Schritt 3). Nicht geprüft: alles im Programm — die Liste, das
Feld, die drei Fenster aus dem Drum-Editor und das Anspielen sind nur in
Tests ohne Fenster und gegen die Engine aus Pappe gelaufen.

**Gebaut — 11c, Schritt 3 (S116), Edit sound... und Instruments auf Drums;
11c ist damit gebaut:**
- **Edit sound...** steht im Menü des Rechtsklicks auf eine Spur, über
  Rename track... und Remove track; grau, wenn die Spur nichts spielt. Der
  Hinweis darauf steht im Tooltip der Auswahl Sound.
- Spielt die Spur einen Klang des Kits, ein eigenes Sample oder eine
  Tondatei, öffnet sich der Creator mit einem **neuen Instrument**
  (`DrumEditor::instrumentAusKlang`): der Titel aus dem Namen des Klangs
  („table_knock" wird „Table knock"), die Art Drums (K3 a), eine
  Sample-Schicht mit dieser Aufnahme — Root note C4, Attack 0, Sustain 100 %,
  sodass sie klingt wie die Datei. **Am Loop ändert sich nichts**, bis der
  Creator speichert. Save fragt nach dem Namen und schlägt den Titel vor
  (ohne Zahl dahinter — es gibt keine Vorlage, die so heißt).
- Beim ersten Save wird die Spur zur Instrument-Spur: **Note** steht auf dem
  Root note der Sample-Schicht, **Length** auf der Länge der Aufnahme ab
  Sample start, aufgerundet auf Millisekunden (K2 a). Der Klang bleibt
  daneben stehen. Für Undo heißt der Schritt „Edit sound". Spielt die Spur
  beim Save etwas anderes als beim Edit sound... (inzwischen umgestellt),
  bleibt sie dabei — das Instrument ist trotzdem gespeichert.
- An einer Spur, die schon ein Instrument spielt, ist Edit sound... dasselbe
  wie Edit instrument....
- **Instruments aus einer Drum-Spur:** spielt die Spur noch kein Instrument,
  stellt sich das Fenster auf **Drums** — der Kopf klappt auf und steht oben
  im Baum (`InstrumentPalette::zeigeArt`), die anderen Arten bleiben, wo sie
  sind. **Bis S116 war davon nichts zu sehen** (kein mitgeliefertes
  Instrument unter Drums, ein Kopf ohne Einträge fehlt im Baum); **seit 11f
  (S117) stehen dort sechs.** Eigene Instrumente — auch die aus Edit
  sound... — stehen weiter nur unter My files (so gesetzt in S114).
Von Claude gesetzt, nicht entschieden: der Ort von Edit sound... (das Menü
des Rechtsklicks — die Skizze der Klangliste nannte ihn nicht); der Titel des
neuen Instruments; Length aus der Länge ab Sample start, ohne Fine tune zu
rechnen; fehlt der Schicht beim Save die Aufnahme, gelten 250 ms; hat das
Instrument beim Save keine Sample-Schicht mehr, gelten C4 und 250 ms.
Nicht geprüft: alles im Programm; eine echte Aufnahme durch Edit sound...
gegen die Datei gehört (im Test steht nur, was die Spur danach nennt — dass
eine solche Schicht Abtastwert für Abtastwert die Datei ergibt, belegt
`test_LoopDrumQuellen` aus Schritt 1); ein Klang des Kits bei einer anderen
Abtastrate als 48000 Hz.

**Stufe 11d — Rauschen und Pitch sweep (S116).** Skizze gezeigt und
**gebilligt (Patrik, S116)**: die mittlere Spalte des Creators, zweimal.
Links eine Sinus-Schicht „a kick body": in der Gruppe Oscillator unter
Waveform, Voices und Detune zwei neue Zeilen — ein Regler für den Weg der
Tonhöhe (in der Skizze „Pitch drop", +22 st) und einer für seine Zeit („Drop
time", 105 ms), darunter die Obertöne und die Hüllkurve mit Decay 480 ms und
Sustain 0. Rechts eine Schicht „the click": in Waveform der neue Eintrag
**Noise** vor Sine, Triangle, Saw, Square, Plucked string, Sample; darunter
der Satz, dass Rauschen keine Tonhöhe hat und Voices, Detune, der neue
Regler, Harmonics und Vibrato fehlen; dann Hüllkurve (Decay 10 ms) und
Filter mit Drive. Das Bild oben rechts heißt bei Noise „Noise" und zeigt ein
kurzes Stück.
**Entscheid N1 (Patrik, S116) — beide Richtungen, gleich:** die Note darf
über ihrer Tonhöhe beginnen und fallen oder darunter beginnen und steigen.
Vorgelegt war a (nur fallend, „Pitch drop", 0 bis +48 st) und b (beide,
−24 bis +48 st); Patrik erst a mit dem Kern für beide Richtungen, dann auf
seine Frage „warum nicht gleich" b. Zum Namen (Patrik: „negativer drop wäre
ja kein drop"): die Zeile heißt **Pitch sweep**, die zweite **Sweep time**;
der Wert zeigt die Richtung als Pfeil („↓ 22 st" fällt, „↑ 12 st" steigt,
„off") — von Claude gesetzt, nachdem „falls 22 st" für die Spalte der Werte
zu breit war (im Bild aus dem Test gesehen).
**Entscheid N2 a (Patrik, S116) — auch für eine Sample-Schicht:** die
Aufnahme läuft am Anfang entsprechend schneller oder langsamer. Verworfen:
b (nur gerechnete Schwingungen). Plucked string und Noise kennen den Regler
nicht.
**Gebaut — 11d (S116):**
- **Modell:** `Schwingung::Rauschen`; an jeder Schicht `tonhoeheStartHt`
  (−24 bis +48 Halbtöne, 0 = aus) und `tonhoeheZeitMs` (0 bis 2000,
  vorbelegt 100).
- **Rechnen:** der Weg ist linear in der Frequenz, wie der Gleitton des Kits
  (`make_kit.py`): `f = ziel + (start − ziel) · e^(−t/τ)`, τ ein Drittel der
  Zeit — nach Sweep time fehlt noch ein Zwanzigstel des Wegs, dieselbe Regel
  wie bei Decay. Für Sinus, Dreieck, Säge, Rechteck (`rechneSchwingung`; die
  Tafel trägt nur Teiltöne, die auch am höchsten Punkt unter der halben
  Abtastrate bleiben — eine Säge mit großem Weg ist deshalb auch nach der
  Ankunft etwas dumpfer) und für Sample (`rechneSample`: der Schritt durch
  die Aufnahme folgt demselben Verlauf; sie ist dadurch früher oder später
  zu Ende). Der Filter folgt weiter der Note, nicht dem Weg.
- **Rauschen** (`rechneRauschen`): gleichverteilt, aus dem Zählwerk des Kits
  mit fester Saat — bei jeder Note dieselbe Folge, derselbe Ton ergibt
  dieselben Werte; zwei Rauschen-Schichten tragen dieselbe Folge (wie die
  vier Schläge des Clap im Kit). Halb ausgesteuert wie jede gerechnete
  Schwingung; danach Filter, Drive, Hüllkurve, Level. Die Note (mit
  Transpose) bewegt nur den Filter.
- **Datei:** `.lvinst` in **Fassung 5** — `waveform: noise`, an jeder
  Schicht `pitchStart` und `pitchTime`. Fassung 1 bis 4 lesen weiter (ohne
  die Felder: kein Weg). Die 49 mitgelieferten bleiben, wie sie sind
  (Fassung 3).
- **Creator:** in Waveform **Noise** zwischen Square und Plucked string;
  unter Detune **Pitch sweep** (−24 bis +48 st) und **Sweep time** (0 bis
  500 ms in Schritten von 5; grau bei „off"). Bei einer Sample-Schicht stehen
  die beiden unter Loop to (N2 nannte „unter Fine tune" — die Zeilen stehen
  für alle Schwingungen an einer Stelle des Gitters). Bei Noise fehlen
  Voices, Detune, Pitch sweep, Sweep time, Harmonics, Vibrato. Das Bild
  „One cycle" heißt bei Noise „Noise"; die Bilder zeigen eine Schicht ohne
  Pitch sweep (den Ton, auf dem die Note ankommt).
Von Claude gesetzt, nicht entschieden: der Verlauf (linear in der Frequenz,
Zeit = 3 τ); die Vorgabe 100 ms; die Saat und dass alle Rauschen-Schichten
dieselbe Folge tragen; der Platz von Noise in der Liste; jede `.lvinst` wird
als Fassung 5 geschrieben; die Pfeile im Wert. Beim Aufräumen: drei
Warnungen von clang im Creator behoben (eine neue, zwei alte — `this` in
einer Lambda, die es nicht braucht).
Nicht angefasst: Decay bleibt bei Schritten von 10 ms — der Klick der Kick
(3,6 ms im Kit) lässt sich am Regler nicht genau stellen, nur in der Datei.
**Hörproben** (lokal, `.claude/handover/hoerproben_11d/`, aus dem Test mit
`LUMIVIZ_11D_WAV`): eine Kick (Sinus, ↓ 22 st in 105 ms, dazu Rauschen als
Klick), dieselbe mit Drive, ein Tom (↓ 10 st in 120 ms), ein steigender Ton
(Säge, ↑ 24 st), Rauschen allein. Gerechnet, **nicht gehört**.
Nicht geprüft: alles im Programm; ob die Kick aus dem Creator klingt wie die
des Kits (der Rechenversuch aus S115 war ein Modell in Python — verglichen
gegen den Rechenkern ist hier nichts); das Fenster ist mit einer
Sinus-Schicht zwei Zeilen höher geworden (im Bild aus dem Test bei 150 %
rund 1400 Bildpunkte hoch, zuvor rund 1325) — ob es auf Patriks Bildschirm
passt, ist nicht gesehen.

**Stufe 11e — Hochpass, Bandpass, Resonanz (S116).** Skizze gezeigt und
**gebilligt (Patrik, S116)**: die Gruppe Filter des Creators, zweimal. Links
„a layer as today": oben neu die Auswahl **Type** (Low-pass, High-pass,
Band-pass), darunter wie bisher Cutoff (× 6.0), Envelope, Drive; dazu der
Satz, dass Low-pass der Filter von heute ist und jedes vorhandene Instrument
sich damit öffnet. Rechts „a noise layer — a clap": Type Band-pass, Cutoff
× 4.6, darunter ein weiterer Regler für die Breite (in der Skizze „Width",
60 %), dann Envelope und Drive.
**Entscheid H1 b (Patrik, S116) — Resonance an allen drei Arten:** unter
Cutoff steht bei jeder Art **Resonance**; beim Bandpass bestimmt sie die
Breite (mehr = schmaler). Bei 0 ist der Tiefpass der von früher. Vorgelegt
und verworfen: a (nur „Width" am Bandpass; meine Empfehlung, wegen der Höhe
des Fensters). Patrik dazu: die Gruppen lassen sich ja einklappen — siehe
unten —, und während des Baus: „tiefpass und resonanz jetzt".
**Gebaut — 11e (S116):**
- **Modell:** an jeder Schicht `filterArt` (Tiefpass, Hochpass, Bandpass)
  und `filterResonanz` (0 bis 1).
- **Rechnen** (`Filter` in `Instrument.cpp`, je Seite des Tons einer): ein
  Tiefpass ohne Resonanz ist der Filter bis 11d, unverändert — zwei Stufen
  erster Ordnung, dieselben Werte wie zuvor (im Test nachgerechnet). Alles
  andere rechnet ein Biquad mit den Formeln des Kits (`make_kit.py`):
  Hochpass, Bandpass mit gleichbleibender Spitze, Tiefpass mit Resonanz. Die
  Güte wächst von 0,707 (Resonance 0) auf rund 11 (100 %); bei Tiefpass und
  Hochpass wird der Ton dabei leiser gestellt (um die Wurzel des Zuwachses),
  damit die Spitze an der Grenze nicht davonläuft — bei 100 % ist sie rund
  2,8-fach, der Rest ein Viertel so laut. Für Schwingungen, Noise und Sample;
  die Saite hat keinen Filter. Cutoff und Envelope wie bisher (Vielfache der
  Note); Cutoff auf „open" heißt: kein Filter, gleich welcher Art.
- **Datei:** `.lvinst` in **Fassung 6** — je Schicht `filterType`
  (`lowpass`, `highpass`, `bandpass`; unbekannt gilt als Tiefpass) und
  `filterResonance`. Fassung 1 bis 5 lesen weiter.
- **Creator:** in der Gruppe Filter oben **Type**, unter Cutoff
  **Resonance** (0 bis 100 %); beide grau, solange Cutoff auf „open" steht,
  und weg bei Plucked string.
Von Claude gesetzt, nicht entschieden: der Bereich der Güte und das
Leiserstellen; ein Regler in Prozent statt einer Güte; „unbekannt gilt als
Tiefpass"; jede `.lvinst` wird als Fassung 6 geschrieben. **Der Haken an
H1 b, wie vorgelegt:** zwischen Resonance 0 und 1 % wechselt der Tiefpass
die Rechnung — an der Grenze lässt er dann 0,71 statt der Hälfte durch (im
Test gemessen). Es bleibt bei einem Filter je Schicht; Hochpass und Bandpass
hintereinander (der Clap des Kits) gehören zur vorgemerkten Kette.
**Hörproben** (lokal, `.claude/handover/hoerproben_11e/`, aus dem Test mit
`LUMIVIZ_11E_WAV`): Snare (zwei Sinus und Rauschen durch den Hochpass), Clap
(vier Schichten Rauschen durch den Bandpass), Hi-Hat zu und offen (Rauschen
durch den Hochpass), ein Bass mit Säge, Tiefpass, 70 % Resonanz und
Filter-Hüllkurve, derselbe ohne Resonanz. Gerechnet, **nicht gehört**.

**Gruppen ein- und ausklappen (Wunsch Patrik, S116; gebaut mit 11e):** die
Überschriften Oscillator, Envelope, Filter und Vibrato sind Knöpfe mit einem
Pfeil; ein Klick klappt die Zeilen der Gruppe ein (Pfeil nach rechts) und
wieder aus (Pfeil nach unten). Die Stellung merkt das Programm
(`loop/creatorFolded/<gruppe>`); sie gilt für jede Schicht und jedes
Instrument, nicht je Schicht. Was die Schwingung nicht kennt, bleibt auch in
einer offenen Gruppe weg. Ohne Skizze gebaut, nach Patriks Beschreibung; im
Bild aus dem Test gesehen. Von Claude gesetzt: eine Stellung für alle
Schichten; die Zeilen über Oscillator (Name, Level, Transpose, Start offset)
gehören zu keiner Gruppe und bleiben immer stehen; das Fenster wird beim
Einklappen nicht von selbst kleiner. In den Tests wird die Stellung weder
gelesen noch gemerkt (`InstrumentCreator::setzeGruppenMerken`).
**Befund Patrik (S116, im Programm gesehen) — behoben:** mit jeder
aufgeklappten Gruppe wuchs der Creator, bis über die Höhe des Bildschirms.
Ursache: die Spalte der Regler bestimmte die Mindesthöhe des Fensters. Die
Regler der Schicht stehen jetzt in einem **Feld, das rollt**
(`creatorLayerScroll`): das Fenster behält seine Größe, gleich was auf- oder
zugeklappt ist; passt nicht alles hinein, erscheint rechts der Rollbalken.
Das Feld behält dabei die Höhe, die seine Zeilen brauchen — beim ersten
Anlauf drückte das Fenster die Zeilen zusammen, statt zu rollen (im Bild aus
dem Test gesehen und nachgestellt). Das **Mausrad** über einem Regler oder
einer Auswahl im Feld rollt das Feld und verstellt nichts mehr (von Claude
gesetzt: sonst verstellte man beim Rollen den Regler unter dem Zeiger).
Nicht geprüft: das Rollen mit dem Mausrad im Programm (der Test belegt nur,
dass der Regler das Rad nicht nimmt).

**Repeat note im Creator (Wunsch Patrik, S116; gebaut):** unter der
Klaviatur der Haken **Repeat note** und daneben die Auswahl **every beat /
every 2 / 4 / 8 / 16 beats** (vorbelegt 2). Mit dem Haken spielt der zuletzt
angeklickte Ton (anfangs C4) immer wieder, mit den Einstellungen, wie sie
gerade stehen — was ein Regler ändert, ist beim nächsten Ton zu hören. Ein
Klick auf eine Taste wechselt die Tonhöhe und spielt sie sofort; die Taste
ist auf der Klaviatur **hervorgehoben, solange Repeat läuft**, sonst nicht.
Die Dauer (Patrik während des Baus: „vielleicht sollte da auch eine Dauer
einstellbar sein, bei delay usw will man etwas mehr hören"): der Abstand
zählt in Schlägen im Tempo des Loops (der Editor nennt es dem Creator), nie
dichter als 250 ms — und **so lang darf der Ton klingen**: das Anspielen
hält dann nicht 600 ms nach dem Ende der Note, sondern erst, wenn der
nächste Ton kommt (`tonLangVerlangt`, `LoopPanel::spieleInstrumentTon` mit
`klingtMs`; das Muster des Anspielens ist so lang, dass es nicht vorher von
vorn beginnt). Die Note selbst bleibt einen Schlag lang. Es endet, wenn der
Haken herausgenommen wird oder das Fenster zugeht. Spielt der Loop, bleibt
es stumm und setzt danach wieder ein (zuvor angekündigt: „es hört auf, wenn
der Loop zu spielen beginnt" — der Creator erfährt davon nichts, das
Anspielen wird in der Zeit nur nicht ausgeführt). Ohne Skizze gebaut, nach
Patriks Beschreibung; das Bild aus dem Test angesehen. Von Claude gesetzt:
die fünf Abstände in Schlägen statt Sekunden; die Auswahl wird nicht
gemerkt; die Farbe der hervorgehobenen Taste (die der Auswahl, heller).
Nicht geprüft: alles im Programm — auch, ob die Effekte am Clip (Echo, Hall)
beim Anspielen aus dem Creator überhaupt zu hören sind: der Creator spielt
ein Instrument an, kein Clip mit seinen Effekten.

**Stufen (Go für 11a, Patrik S115):** 11a Sample als Schicht — in drei
Schritten: der Kern ohne Fenster, der Creator nach der Skizze mit File...
und der Frage aus S2 a, Detect und Reference tone; 11b das Fenster Record
sample (Beleg-Test davor); 11c der Drum-Editor (nach dem Entscheid oben).

**Gebaut — 11a, Schritt 1 (S115), der Kern ohne Fenster:**
`loop/Sample` — der Ordner `samples` im Benutzerordner
(`eigenerSampleOrdner`), `user:<name>` (`istEigenesSample`,
`eigeneSampleDatei`, `eigeneSamples`, `sampleDatei`, `sampleTitel`), das
Dazukopieren für S2 a (`nimmZuEigenen`: Name aus dem Dateinamen, dieselbe
Datei wird wiedergenommen, bei anderem Inhalt eine Zahl dahinter),
`loescheEigenesSample`; die dekodierten Daten einmal gemerkt, in 48000 Hz
Stereo (`sampleDaten`, `merkeSampleDaten`, `vergissSampleDaten`) — sie
gelten nicht mehr, sobald die Datei eine andere ist. `Schwingung::Sample`
und an der Schicht `sample`, `grundton`, `feinCent`, `sampleStartMs`,
`schleife`, `schleifeVonMs`, `schleifeBisMs`. `rendereTon` spielt die
Aufnahme je Note schneller oder langsamer (Abstand zum Grundton, Fine tune
abgezogen, geradlinig zwischen den Abtastwerten gelesen); Sample start;
der Sustain loop wiederholt sein Stück, solange die Note dauert, und
blendet an der Naht bis 10 ms über, danach läuft die Aufnahme bis an ihr
Ende; Envelope, Filter und Drive gelten, Voices, Detune und Vibrato nicht;
Stereo bleibt Stereo. Ist nichts gemerkt, schweigt die Schicht.
`ladeKlaenge` dekodiert die Aufnahmen der eingeschalteten Sample-Schichten
einmal über die Audio-Engine; fehlt eine, steht „Sample not loaded: …" im
Bericht. **`.lvinst` in Fassung 4** (`waveform: sample`, dazu `sample`,
`rootNote`, `fineTune`, `sampleStart`, `sustainLoop`, `loopFrom`,
`loopTo`, nur an einer Sample-Schicht geschrieben); Fassung 1 bis 3 lesen
weiter. Export und Import: neuer Bereich „Samples".
Von Claude gesetzt, nicht entschieden: eine hineinkopierte Datei behält
ihre Endung (S1 a nannte `<name>.wav` — das gilt für Aufnahmen), der Name
ohne Endung ist im Ordner eindeutig; als Sample gelten wav, flac, mp3, ogg,
aiff, aif; eine Datei an freiem Ort steht mit vollem Pfad in der `.lvinst`
(nicht relativ zu ihr); jedes Instrument wird als Fassung 4 geschrieben,
auch ohne Sample-Schicht — ein älteres LumiViz liest es dann nicht mehr;
die Überblendung an der Naht und ihre 10 ms; der Pegel einer Sample-Schicht
ist wie bei gerechneten halbiert (L6 a).
Nicht gebaut: alles im Creator (er bietet Sample noch nicht an), Detect,
Reference tone, die Frage aus S2 a, relative Pfade. Nicht geprüft: eine
echte Tondatei durch die BassEngine in eine Sample-Schicht (die Tests
nehmen einen gerechneten Sinus und die Engine aus Pappe). Im Programm ist
nichts zu sehen außer der Zeile „Samples" im Export-Dialog.

**Gebaut — 11a, Schritt 2 (S115), der Creator nach der Skizze:** in
**Waveform** steht **Sample**. Bei einer Sample-Schicht stehen darunter
**File** (Name, Länge, bei einem eigenen „· My samples"; „(none)" oder
„… (not found)") mit **File...**, **Root note** (C-1 bis G9), **Fine tune**
(−100 bis +100 ct), **Sample start** (ms), der Haken **Sustain loop**
(„repeat a part while the note is held") mit **Loop from** und **Loop to**
(Sekunden). Die drei Stellen reichen so weit wie die Aufnahme; Loop from
bleibt unter Loop to. Voices, Detune, Harmonics, Damping und Vibrato samt
seiner Überschrift fehlen. **File...** öffnet ein Menü mit den eigenen
Samples („My samples") und **From file...**; gibt es keine eigenen, gleich
den Dateidialog. Nach From file... fragt der Dialog aus S2 a („Where should
… be kept?" — **Copy the file to My samples (recommended)**, **Use it where
it is**, Cancel, Haken „Don't ask again"); liegt die Datei schon im Ordner
`samples`, fragt er nicht. Die gemerkte Wahl (`loop/sampleCopyChoice`) steht
unter Settings → Panels als **Chosen Sample Files** (Ask each time, Copy to
My samples, Use it where it is). Das obere Bild heißt bei einer
Sample-Schicht **The sample** und zeigt die Aufnahme als Fläche um die
Nulllinie, mit dem roten Griff für Sample start und dem blauen Bereich für
den Sustain loop; die Griffe tragen oben eine Fahne, der Zeiger wird in
ihrer Nähe zum Doppelpfeil, darunter steht „Red: sample start · blue:
sustain loop — drag them". Eine andere Aufnahme beginnt von vorn (Sample
start 0, Sustain loop aus, der Grundton bleibt). Beim ersten Einschalten
liegt der Sustain loop auf dem mittleren Teil (ein Viertel bis drei
Viertel). Die Daten für Bild und Länge verlangt der Creator über
`sampleVerlangt`; der Editor reicht es ans Panel, das mit der Audio-Engine
dekodiert und merkt. Lässt sich eine Aufnahme nicht laden, fragt er nicht
bei jedem Zug an einem Regler wieder.
Von Claude gesetzt, nicht entschieden: beim ersten Wechsel einer Schicht auf
Sample geht Sustain auf 100 %; das Menü hinter File... (die Skizze zeigte nur
den Knopf); die Wahl aus S2 a geht nicht mit „Settings" in einen Export; die
Überschrift Vibrato verschwindet auch bei Plucked string (sie stand dort
über nichts). Nicht gebaut: Detect, Reference tone (Schritt 3), Record...
(11b). Nicht geprüft: das Menü, der Dateidialog und die Frage im gezeigten
Fenster; eine echte Tondatei; der Zeigerwechsel.

**Gebaut — 11a, Schritt 3 (S115), Detect und Reference tone:** neu
`loop/Grundton` — `messeGrundton` misst die Periode einer Aufnahme an fünf
Stellen hinter dem Anschlag (die Abweichung von sich selbst je Verschiebung,
auf ihren laufenden Mittelwert bezogen, das Verfahren „YIN"; Bereich 40 bis
2000 Hz) und nimmt den mittleren Wert, wenn wenigstens die Hälfte der
Stellen auf einen halben Halbton übereinstimmt; sonst gibt es keinen klaren
Ton. Im Creator steht neben Root note **Detect**; darunter erscheint
„Detected: A3 +12 ct" mit **Apply** oder „No clear pitch found" ohne
Apply. Gemessen wird ab Sample start; erst Apply setzt Root note und Fine
tune (auf ganze Cent). Das Ergebnis gilt für die Schicht und die Aufnahme,
an der gemessen wurde, und verschwindet mit einer anderen. Unter der
Klaviatur steht bei einer Sample-Schicht der Haken **Reference tone**: die
Tasten spielen dann die gewählte Sample-Schicht allein und dazu einen Sinus
derselben Note (mit dem Transpose der Schicht, halb so laut).
**Damit ist 11a gebaut.**
Von Claude gesetzt, nicht entschieden: mit Reference tone spielt nur die
gewählte Schicht (wie „Selected layer only") — ein Instrument trägt
höchstens vier Schichten, der Sinus wäre sonst die fünfte; die Schwellen
der Messung (0,15 für eine Periode, ein halber Halbton für „einig"); Töne
unter 40 Hz und über 2000 Hz findet Detect nicht. Nicht geprüft: Detect an
einer echten Aufnahme (die Tests messen gerechnete Töne: Sinus, Töne mit
lauterem zweitem Teilton, die gezupfte Saite, einen Sägezahn, Rauschen);
ein Akkord. Die Messung des Grundtons ist dieselbe, die später das Autotune
(§3.2) braucht.

**Beleg vor 11b (S115) — BASS nimmt auf** (`test_LoopBelegeAufnahme.cpp`,
gebaut ist nichts): an Patriks Rechner nennt BASS drei Eingänge — „Default",
„Microphone Array (Realtek(R) Audio)" und „Lautsprecher (Realtek(R) Audio)"
als **Mitschnitt der Ausgabe** (`BASS_DEVICE_LOOPBACK`). Neben der laufenden
`BassEngine` (sie hält ihr Ausgabegerät) öffnet `BASS_RecordInit(-1)` den
Standard-Eingang; `BASS_RecordStart` mit 48000 Hz, Stereo, float liefert
genau dieses Format, der Rückruf läuft in einem eigenen Thread, die Blöcke
sind 4800 Abtastwerte (100 ms), in einer Sekunde kamen 48000 Abtastwerte an
(zwei Läufe, größter Betrag 0,07 und 0,04 — der Eingang liefert Ton); nach
dem Stop kommt nichts mehr. Bis zum ersten Block vergehen rund 200 ms
(1204 und 1248 ms Wanduhr für 1000 ms Ton). Festgestellt dabei: der Stop
gibt einen Aufnahme-Kanal frei — nach `BASS_ChannelStop` gibt
`BASS_ChannelGetInfo` keine Auskunft mehr. **Folge für 11b:** die Aufnahme
geht über BASS, ohne weitere Bibliothek; der Pegel im Fenster kann höchstens
alle 100 ms neu kommen, wenn die Blockgröße bleibt (ein kürzerer Takt lässt
sich bei `BASS_RecordStart` verlangen — nicht erprobt); der Mitschnitt der
Ausgabe steht von selbst in der Liste der Eingänge. Der zweite Fall nimmt
auf und läuft deshalb nur mit `LUMIVIZ_AUFNAHME_BELEG=1`. Nicht erprobt:
ein Rechner ohne Eingang, ein von Windows gesperrtes Mikrofon, das Aufnehmen
vom Mitschnitt, während das Programm selbst spielt.

**Gebaut — 11b, Schritt 1 (S115), der Kern ohne Fenster:**
`IAudioEngine` bekommt die Aufnahme — `getRecordDevices` (die Eingänge,
`isLoopback` = Mitschnitt der Ausgabe), `recordStart(deviceId, sampleRate,
maxSeconds)` (Stereo, float; endet von selbst, wenn die Menge voll ist),
`isRecording`, `recordedFrames`, `recordLevel` (der größte Betrag seit dem
letzten Aufruf, für die Pegelanzeige), `recordStop` (beendet, gibt den
Eingang frei und liefert das Aufgenommene). Vorgabe überall: die Engine
kann es nicht. Die `BassEngine` setzt es mit `BASS_RecordInit`,
`BASS_RecordStart` und einem Rückruf um, der unter einem Schloss sammelt;
die Blöcke kommen alle 20 ms (verlangt 25 ms, gemessen 960 Abtastwerte an
48000 Hz). Eine Aufnahme läuft neben der Ausgabe, mit oder ohne
`initialize`; `shutdown` gibt einen offenen Eingang frei.
Neu `loop/Aufnahme` — `ohneStille` (der Ausschnitt ohne die Stille an den
Rändern: still ist, was unter 2 % der Spitze liegt; 5 ms davor und 30 ms
danach bleiben stehen), `spitzeIn`, `schneideAus` (der Ausschnitt mit 2 ms
Blende an beiden Enden, auf Wunsch auf einen größten Betrag von 0,9
gehoben), `schreibeWav` (Stereo, 16 Bit), `vorschlagAufnahmeTitel`
(„Recording", „Recording 2", …), `speichereAufnahme` (als `<name>.wav` zu
den eigenen Samples; ein vorhandenes Sample des Namens wird ersetzt, was
dazu gemerkt war, ist vergessen, Töne werden neu gerechnet).
Gemessen mit `LUMIVIZ_AUFNAHME_BELEG=1`: eine halbe Sekunde verlangt, zu
Ende nach 504 ms mit genau 24000 Abtastwerten; nach 300 ms abgebrochen:
14400; eine geschriebene WAV liest die BassEngine auf 16 Bit genau wieder
(größte Abweichung 0,00003).
Von Claude gesetzt, nicht entschieden: die Aufnahme wird als 16-Bit-WAV
geschrieben (das liest jedes Programm; die Engine nimmt in float auf); die
Schwelle für Stille und die Luft davor und danach; die Blende von 2 ms;
Normalize hebt auf 0,9 wie die Klänge des Kits; ein vorhandenes Sample
desselben Namens wird ohne Frage ersetzt — die Rückfrage gehört ins Fenster
(Schritt 2). Nicht gebaut: das Fenster, Play und Repeat, Record... im
Creator. Im Programm ist nichts zu sehen.

**Gebaut — 11b, Schritt 2 (S115), das Fenster „Record sample":**
`loop/AufnahmeFenster` nach der gebilligten Skizze, als Werkzeugfenster.
Oben **Input** (die Eingänge der Engine; der Mitschnitt der Ausgabe heißt
„… — what the computer plays"; gewählt ist der Standard-Eingang, die Wahl
wird gemerkt, `loop/recordInput`) und **Level** (der größte Betrag seit dem
letzten Takt, −48 bis 0 dB auf die Breite gelegt). **● Record** und
**Stop**, daneben „Press Record — up to 30 s", „1.2 s recorded · up to
30 s", „2.0 s recorded". Lässt sich der Eingang nicht öffnen, steht dort
„Could not open this input. Is the microphone allowed for desktop apps in
the Windows privacy settings?", kommt nichts an, „Nothing arrived from this
input." — ohne eigenes Meldungsfenster. Nach 30 Sekunden endet die Aufnahme
von selbst. Das Bild zeigt die Aufnahme als Fläche (auf ihre Spitze
bezogen, auch eine leise ist zu sehen) mit zwei roten Griffen; was
außerhalb liegt, ist grau überdeckt; eine blaue Linie zeigt, wo gespielt
wird. Darunter „Drag the two handles to trim · grey parts are cut off on
save · blue: play position". **▶ Play** (wird zu „■ Stop playing") spielt
den Ausschnitt über einen eigenen Mixer, mit **Repeat** in Dauerschleife;
ein gezogener Griff gilt ab dem nächsten Block, zu hören ist, was
gespeichert würde (Blende an den Enden, Normalize). Daneben „Trimmed:
1.00 s to 1.50 s · 0.50 s"; ein Ausschnitt wird nicht kürzer als 10 ms.
**Trim silence at both ends** (anfangs an: nach jeder Aufnahme stehen die
Griffe an den Rändern des Tons; aus: die ganze Aufnahme) und **Normalize
level**. **Name** mit dem Vorschlag „Recording", darunter „File: My files ·
samples · <name>.wav", bei einem vergebenen Namen „— replaces the one
there". **Save and use** fragt bei einem vergebenen Namen nach, schreibt
den Ausschnitt, meldet `gespeichert(user:<name>)` und schließt; **Cancel**
und das Schließen fragen, wenn eine Aufnahme ungespeichert ist.
`KurvenBild` kann dafür den Bereich als Zuschnitt zeichnen (`setAussenGrau`)
und eine Linie ohne Griff (`setZeiger`). Neu `AusschnittQuelle`: die Quelle
im Mixer, die den Ausschnitt spielt.
Von Claude gesetzt, nicht entschieden: während der Aufnahme zeigt das Bild
noch nichts (nur Pegel und Dauer); der Player und der Loop werden beim
Aufnehmen und Anhören nicht angehalten — spielt gerade Musik, nimmt ein
Mikrofon sie mit auf; die Skala des Pegels; die 10 ms. Nicht gebaut: der Weg
ins Fenster — **niemand öffnet es noch** (Schritt 3: Record... im Creator).
Nicht geprüft: das Fenster mit der echten Engine (die Tests nehmen eine
Engine aus Pappe), der Zeitgeber, die Rückfragen, der Pegel an einem echten
Eingang, das Anhören über die Lautsprecher.

**Gebaut — 11b, Schritt 3 (S115), Record... im Creator:** bei einer
Sample-Schicht steht unter der Zeile File neben **File...** der Knopf
**Record...** (`creatorSampleRecord`). Er öffnet das Fenster „Record sample"
von vorn (keine Aufnahme, der nächste freie Name); es ist ein Fenster je
Creator und wird wiederverwendet. **Save and use** dort setzt die Aufnahme
in die gewählte Sample-Schicht (`user:<name>`); wie bei jeder anderen
Aufnahme beginnt sie von vorn (Sample start 0, Sustain loop aus). Wird
unter einem vorhandenen Namen neu aufgenommen, nennt die Schicht dasselbe,
und Bild, Länge und Töne folgen der neuen Aufnahme; ein Ergebnis von Detect
gilt nicht weiter. Die Audio-Engine bekommt das Fenster beim Öffnen: das
Panel nennt sie dem Editor (`setEngineGeber`), der dem Creator.
**Damit ist 11b gebaut und im Programm erreichbar:** Loop-Panel → Clip
eines Instrument-Musters → Edit instrument... → eine Schicht mit Waveform
Sample → Record....
Von Claude gesetzt: File... und Record... stehen in einer eigenen Zeile
unter dem Dateinamen (die Skizze zeigte den Namen und File... in einer
Zeile — zwei Knöpfe hatten daneben keinen Platz). Nicht geprüft: alles im
Programm — aufgenommen, angehört und gespeichert ist bisher nur gegen die
Engine aus Pappe und, im Kern, über die BassEngine ohne Fenster.

### Die Palette der Instrumente (Stufe 14, P1 a)

Im Instrument-Editor steht neben dem Namen des Instruments **Browse...** und
öffnet das Fenster „Instruments" — gebaut nach dem Vorbild des
Palettenfensters der Effektkette (`UI/panels/EffectPaletteWindow.hpp`, S79).
Es bleibt offen und zeigt die Instrumente als Baum nach Art (vorgeschlagen:
Keys, Bass, Lead, Pads and strings, Plucked, Wind, Percussion, Unusual, My
files), mit Suchfeld, Favorites, Recently used und einem Satz Beschreibung.
Ein Klick spielt das Instrument an, Doppelklick oder **Use** setzt es in das
Muster des offenen Instrument-Editors. **Dazu Patrik:** Sortieren oder
Filtern nach **Genre**.

**Die Skizze (gebilligt, Patrik S114; nicht gebaut):**

- **Im Instrument-Editor** fällt die Dropdown-Liste weg. In der Zeile steht
  der Name des Instruments als Text, daneben **Browse...**, dann wie bisher
  **Edit instrument...**, **Load instrument...**, Volume, Pan, M.
- **Das Fenster „Instruments"** ist nicht modal und bleibt offen. Oben
  **Search** und rechts **Genre** (Auswahlfeld, „All" und die Genres).
  Darunter die Zeile „For pattern: <Name des Musters>". Dann der Baum:
  **★ Favorites**, **Recently used**, die Arten mit Zahl (Keys, Bass, Lead,
  Pads and strings, Plucked, Wind, Percussion, Unusual) und **My files**.
  Unten der Detailbereich: Titel, Art und Genres in einer Zeile, ein Satz
  Beschreibung, dann **Preview note** (Auswahl der Tonhöhe), **Play**,
  **★** (Favorit; bis S115 „Favorite") und rechts **Use**; darunter der Hinweis „Click plays the
  instrument · double-click uses it in the pattern".
- **Klick** spielt einen Ton, einen Schlag lang; **Play** noch einmal.
  **Preview note** stellt sich je Art vor (Bass tief, sonst C4).
  **Doppelklick**, Enter oder **Use** setzt das Instrument ins Muster (ein
  Schritt für Undo) und rückt es in „Recently used". **Search** sucht in
  Titel, Art, Genre und Beschreibung; **Genre** filtert, Arten ohne Treffer
  verschwinden. Ohne offenen Instrument-Editor ist **Use** grau und statt
  „For pattern" steht „Open an instrument pattern to use an instrument".
- **Die 49 Instrumente nach Art** (von Claude nach dem Titel zugeordnet,
  nicht nach Klang): Keys 7 (Piano, House Piano, Tine Piano, Organ, House
  Organ, Clavinet, Chord Stab) · Bass 10 (Bass, Acid, Sub, Growl, Rolling,
  Slap, 808, Reese, Round, Upright) · Lead 4 (Lead, Trance Lead, Super Saw,
  Hoover) · Pads and strings 4 (Strings, Violin, Warm Pad, Fifth Pad) ·
  Plucked 7 (Guitar, Clean, Muted, Nylon, Overdrive Guitar, Pluck, Trance
  Pluck) · Wind 3 (Brass, Flute, Sax) · Percussion 8 (Bell, Kalimba,
  Marimba, Steel Drum, Vibraphone, Clave, Conga, Cowbell) · Unusual 6 (Alien
  Choir, Cascade, Siren, Broken Radio, Ghost, Reverse Swell).
- **Von Claude gesetzt, nicht entschieden:** Genre ist ein Filter, der Baum
  bleibt nach Art; ein Instrument trägt mehrere Genres, die erste Zuordnung
  aus den Loops unter `asset/loop/genres` abgelesen (ausgezählt in S114:
  Slap Bass etwa nur in Funk), die Loops direkt unter `asset/loop` noch
  nicht; die Dropdown-Liste fällt ganz weg. Nicht in der Skizze: ein
  Instrument auf einen Clip im Panel Loop ziehen.

**Entscheid F3 b (Patrik S114):** die `.lvinst` bekommt drei Felder, die
fehlen dürfen — `category` (eine der acht Arten), `genres` (Liste),
`description` (ein Satz) — und geht auf **Fassung 3**; Fassung 1 und 2
werden weiter gelesen, ein älteres LumiViz lehnt eine neu gespeicherte Datei
ab. Ohne `category` steht ein Instrument unter „Unusual", ohne `genres` nur
bei „All". Der Creator bekommt **Type**, **Genres**, **Description**; die 49
Dateien werden von `make_examples.py` und `make_genres.py` neu geschrieben.
Verworfen a (dieselben Felder, Fassung bleibt 2), c (eine Katalogdatei
daneben).

**„My files" und die Reihenfolge (Entscheid D1 b, Patrik S114):** woher
eigene Instrumente kommen, wurde mit der Datenablage als Ganzes geklärt und
gebaut, bevor Stufe 14 begann —
[Datenablage_Konzept.md](../core-services/Datenablage_Konzept.md).

**Entscheid P2 a (Patrik S114) — eigene Namen sind ein eigener Raum:** ein
eigenes Instrument liegt als `<name>.lvinst` im Benutzerordner unter
`instruments`, und ein Muster nennt es als **`user:<name>`** (Vorbild `kit:`).
Ein mitgeliefertes heißt weiter `piano`; ein eigenes `piano` verdrängt es
**nicht** — ein Loop klingt, wie er gespeichert wurde. Im Fenster steht das
eigene unter „My files". **Save** im Creator schreibt unter einem Namen in
den Benutzerordner (bei einem mitgelieferten als eigene Kopie), **Save as...**
mit Dateidialog bleibt, eine Datei an freiem Ort steht wie bisher beim Pfad
im Muster. Verworfen b (wie bei den Presets: das eigene verdrängt — ein
Instrument wird bei jedem Abspielen nachgeschlagen, ein eigenes `piano`
färbte jeden Loop um, auch nach einem Import auf einem anderen Rechner).
**Vorgemerkt (Patrik: „oder optional Schalter für c"):** im Creator bei
einem mitgelieferten **Replace the built-in one for all loops** — nur dann
verdrängt dieses eine; nicht gebaut.

**GEÄNDERT — Entscheide E1 a und E1b c (Patrik S117):** Anlass: drei
Instrumente lagen nach **Save as...** lose in der Wurzel des Benutzerordners,
der Dateidialog hatte dort begonnen, und „My files" zeigte sie nicht.
**E1 a:** Save as... fragt nur noch nach dem Namen — derselbe Dialog „Save
instrument" wie Save bei einem mitgelieferten — und schreibt immer eine Kopie
nach `instruments` (der Ordner wird angelegt); das Muster nennt danach
`user:<name>`. Einen Dateidialog zum Speichern gibt es im Creator nicht mehr;
der Satz „Save as... mit Dateidialog bleibt" oben gilt nicht mehr. Lesen
bleibt: Load instrument... und From file... nehmen eine Datei von jedem Ort,
und **Save** schreibt eine so geladene an ihren Ort. **E1b c:** ein
mitgeliefertes Instrument wird nie überschrieben und nicht verdrängt — der
Dialog sagt es nur („… is an original LumiViz instrument. Originals are not
overwritten …"), die Kopie kommt zu My files, der vorgeschlagene Name lässt
sich behalten oder ändern. Verworfen bei E1b: a (ersetzen über eine Fassung
unter `instruments\replaced` mit Restore original — Patrik: „wollen wir, dass
das Original überschrieben wird? eigentlich nicht") und b (die Datei neben
der Exe schreiben — jeder Bau stellte sie wieder her). Der vorgemerkte
Schalter **Replace the built-in one for all loops** ist damit **nicht** mehr
vorgemerkt. Gebaut S117: `InstrumentCreator::kopieSatz` (drei Sätze: neu,
mitgeliefert, schon eine eigene Datei), `frageNachDatei` entfällt,
`speichereUnter` bleibt für die geladene Datei. Von Claude gesetzt: der
Wortlaut der Sätze und des Tooltips („Save a copy to My files under a new
name"). Im Programm nicht gesehen.

**Entscheid E2 a (Patrik S117):** die drei losen Dateien liegen jetzt unter
`instruments` (von Hand verschoben, kein Code). **Entscheid E3 a (Patrik
S117) — Load instrument... holt die Datei zu den eigenen:** der Dateidialog
bleibt (eine fremde Datei liegt irgendwo; beim ersten Mal beginnt er in den
Dokumenten), danach die Frage „Where should <datei> be kept?" mit **Copy
the file to My files (recommended)** (vorbelegt), **Use it where it is**,
**Cancel** — dieselbe wie bei Sample file... (S2 a). Kopieren:
`nimmZuEigenenInstrumenten` legt die Datei Byte für Byte unter `instruments`
ab, sie heißt nach ihrer Datei, bei vergebenem Namen mit `_2`, `_3` …; liegt
dieselbe Datei schon da, entsteht keine zweite; das Muster nennt
`user:<name>`. Am Ort lassen: wie bisher der Pfad im Muster. Liegt die
gewählte Datei schon im Ordner der eigenen, wird nicht gefragt.
`InstrumentEditor::nimmInstrumentDatei`. Verworfen: b (nur der Startordner
`instruments` — den zeigt Browse... schon, und der Pfad im Muster bliebe),
c (nichts). Von Claude gesetzt, nicht entschieden: kein „Don't ask again"
wie bei den Samples; der Wortlaut; From file... im Creator bleibt ohne
Frage (es liest nur eine Schicht heraus); was kein Instrument ist, wird
nicht kopiert. Nicht geprüft: die Frage im gezeigten Fenster. Im Programm
nicht gesehen.

**Gebaut — 14a (S114), das Modell:** `Instrument` trägt `art`, `genres`,
`beschreibung`; `kInstrumentFassung` 3 (1 und 2 werden gelesen, die Felder
dürfen fehlen; eine unbekannte Art gilt als `unusual`, bleibt in der Datei
aber stehen). `instrumentArten`, `gueltigeArt`, `artTitel`. Eigene:
`kEigenVorsatz`, `eigenerInstrumentOrdner`, `eigenerName` (aus dem Titel:
klein, `a`–`z`, Ziffern, `_`), `istEigenerName`, `eigeneDatei`,
`eigeneInstrumente` (jedes Mal neu gelesen), `speichereEigenes`,
`loescheEigenes`; `holeInstrument` kennt `user:<name>` und sucht nur Namen,
die `eigenerName` bildet — ein Muster kann so nichts außerhalb des Ordners
nennen. Fehlt ein eigenes, meldet es der Bericht des Kits wie ein fehlendes
mitgeliefertes („Instrument not found"). **Die 49 Dateien** stehen in
Fassung 3: `asset/loop/instrument_katalog.py` trägt Art und Beschreibung in
einer Tabelle und zählt die Genres aus den mitgelieferten Loops (24 Genres;
Guitar und Violin stehen in keinem Loop und tragen keins). Die vierzehn
Dateien aus S110 waren noch Fassung 1 (flach) und sind dabei zu einer
Schicht geworden — gegen den Stand im Repo geprüft: Titel, Lautstärke und
jeder Wert jeder Schicht gleich. Im Programm ist davon nichts zu sehen.

**Gebaut — 14b (S114), der Creator; Skizze gebilligt, Patrik S114:** unter
der Kopfzeile eine zweite: **Type** (Auswahl der acht Arten), **Genres**
(Textfeld, mit Kommas getrennt; es schlägt beim Tippen die vorhandenen
Genres für das Stück hinter dem letzten Komma vor), **Description** (ein
Satz, höchstens 200 Zeichen). Eine Änderung dort zählt wie jede andere
(Stern, Reset). **Save** ist wählbar, sobald etwas geändert ist: bei einem
eigenen Instrument schreibt es an seinen Ort (im Benutzerordner bleibt es
`user:<name>`); bei einem mitgelieferten oder neuen öffnet es den Dialog
„Save instrument" — ein Satz, dass die Kopie zu „My files" kommt und das
mitgelieferte bleibt; das Feld **Name** mit dem Vorschlag; darunter „File:
<name>.lvinst", mit „— replaces the one in My files", wenn es den Namen
schon gibt; **Cancel**, **Save**; danach bei vergebenem Namen die Rückfrage
„Replace it?". **Save as...** bleibt der Weg an einen freien Ort (seit S117
nicht mehr: derselbe Dialog, siehe E1 a oben). In der
ersten Zeile steht nach dem Speichern „My files: <datei>". Der Creator
meldet `gespeichert(nennung)` — `user:<name>` oder den Pfad —, der Editor
setzt sie ins Muster. **GEÄNDERT gegenüber S110:** Save war bei einem
mitgelieferten Instrument grau. Von Claude gesetzt: der Vorschlag ist der
Titel mit der ersten freien Zahl, solange er der der Vorlage ist; ein
umbenannter, freier Titel gilt, wie er ist; beim Schließen mit
Ungespeichertem führt **Save** bei einem mitgelieferten jetzt in denselben
Dialog (vorher der Dateidialog). Nicht geprüft: der Dialog selbst und die
Vorschläge im gezeigten Fenster.

**Gebaut — 14c (S114), das Fenster und Browse...:** `loop/InstrumentPalette`
nach der Skizze oben. Im Instrument-Editor steht der Name des Instruments
fett als Text — bei einem eigenen „Titel (My files)", bei einer Datei
„Titel (datei.lvinst)", sonst „name (not found)" —, daneben **Browse...**;
die Dropdown-Liste ist weg. Das Fenster gehört dem Editor, folgt der Basis,
die er zeigt, und wählt beim Öffnen ihr Instrument, ohne es anzuspielen.
Der Baum: **Favorites** und **Recently used** (nur wenn sie etwas tragen,
höchstens acht zuletzt benutzte), die acht Arten mit der Zahl dahinter
(„Bass  (10)"), **My files**; ohne eigenes Instrument steht dort „Save an
instrument in the creator to see it here". Ein Kopf ohne Treffer
verschwindet. Unten „**Slap Bass** · Bass · Funk", bei einem eigenen dazu
„My files (name.lvinst)", darunter der Satz und die Zahl der Schichten.
**Preview note** bietet C1 bis C6 und stellt sich bei jeder Wahl auf die
Art (Bass C2, sonst C4). Ein Rechtsklick auf ein Instrument: Use, Play, Add
to / Remove from Favorites; bei einem eigenen dazu **Show in folder** und
**Delete** (mit Rückfrage; es verschwindet auch aus Favorites und Recently
used). Aus der Suche führt Pfeil hinunter in den Baum, Enter verwendet.
Speichert der Creator, liest das Fenster neu und wählt das Gespeicherte.
Favoriten und zuletzt benutzte merkt die App als Namen, wie ein Muster sie
nennt (`instrumentPalette/favorites`, `/recent`); sie gehen mit „Settings"
in einen Export. Das Anspielen läuft über `instrumentTonVerlangt`, denselben
Weg wie im Creator. **GEÄNDERT gegenüber S110:** keine Dropdown-Liste mehr.
Von Claude gesetzt: ein eigenes Instrument steht nur unter „My files",
nicht zusätzlich unter seiner Art; die Suche nimmt mehrere Worte, alle
müssen vorkommen; ein gewähltes, das der Filter verbirgt, ist nicht mehr
gewählt. Nicht gebaut: der Schalter aus P2 c. Nicht geprüft: das Menü, die
Tasten und das Fenster neben dem Editor im Programm. **Stufe 14 ist damit
gebaut.**

**Befund und Entscheid Z1 a (Patrik, S115) — Ctrl+Z im Fenster:** im
Programm nahm Ctrl+Z nach **Use** nichts zurück, erst nach einem Klick ins
Panel Loop. Ursache: Ctrl+Z ist das Kürzel von Edit → Undo im Hauptfenster
und gilt nur, solange dieses aktiv ist; das Fenster Instruments ist ein
eigenes Fenster (`Qt::Window`) und nach Use das aktive. Gebaut: das Fenster
trägt selbst zwei Kürzel (Undo: Ctrl+Z; Redo: Ctrl+Y, Ctrl+Shift+Z) und
bittet über `rueckgaengigVerlangt` den Editor, der das Panel, das Panel
ruft Undo bzw. Redo des Programms (`ICommandBus`) — denselben Schritt wie
Edit → Undo, also den letzten des ganzen Programms, nicht nur den am Loop.
In der Suche gehören die Tasten dem Eingabefeld. Verworfen: b (das Fenster
als Werkzeugfenster am Hauptfenster — reicht alle Kürzel durch, ändert aber
das Verhalten des Fensters), c (nur beschreiben). Die Wahl im Fenster folgt
einem Undo nicht: markiert bleibt, was zuletzt angeklickt war. Nicht
geprüft: die Taste im gezeigten Fenster (der Test löst das Kürzel aus, er
drückt es nicht); ob Ctrl+Z im Instrument-Creator ankommt, der genauso am
Editor hängt.

### Mehrfachauswahl in den Editoren (Stufe 13, Wunsch Patrik S113)

Die Skizze ist gebilligt und steht in §4i.

---

## 4i. Mehrfachauswahl in den Editoren (Stufe 13 — Skizze gebilligt, Patrik S113; nicht gebaut)

Wunsch Patrik S113: im Drum-Editor und im Instrument-Editor mehrere Schritte
oder Noten zugleich markieren, kopieren, das Teilmuster an anderer Stelle
einfügen, oder mehrere zusammen verschieben. Bisher stand beides in §4d und
§4f unter „Nicht drin".

**Das Markieren.** Im Loop-Panel zieht man den Rahmen auf freier Fläche; in
den Editoren setzt ein Klick dort schon einen Schritt oder eine Note, und
Ziehen malt Schritte oder verlängert die neue Note. Deshalb:

| Geste | Drum-Editor | Instrument-Editor |
|---|---|---|
| **Shift+Ziehen** | ein Rahmen über Schritte, auch über mehrere Spuren; markiert ist der ganze Bereich, auch seine leeren Schritte | ein Rahmen über Noten; markiert sind die Noten, die er berührt |
| **Ctrl+Klick** | nimmt einen Schritt dazu oder heraus | nimmt eine Note dazu oder heraus |
| **Ctrl+A** | alle Schritte des Musters | alle Noten |
| **Esc**, Klick ins Leere | hebt die Marken auf | hebt die Marken auf |

Markierte Schritte und Noten tragen den Rand in der Markierfarbe. Unter dem
Raster steht eine Zeile: ohne Auswahl, wie man markiert („Shift+drag
selects"); mit Auswahl, wie viel markiert ist und was die Tasten tun — im
Drum-Editor etwa „16 steps in 2 tracks selected · Ctrl+C copy · Ctrl+V paste
at the marker · arrow keys move", im Instrument-Editor „3 notes selected ·
drag moves them together · Ctrl+C copy · Ctrl+V paste at the marker · Ctrl+D
duplicate".

**Was mit dem Markierten geht:**

- **Ctrl+C**, **Ctrl+X**, **Ctrl+V** — kopieren, ausschneiden, einfügen; dazu
  Einträge im Menü des Rechtsklicks.
- **Die Einfügemarke:** ein Klick in die Kopfzeile mit den Taktnummern setzt
  eine gestrichelte Marke (in der Skizze mit „paste" beschriftet). Ctrl+V
  fügt dort ein; die Marke rückt danach um die eingefügte Länge weiter —
  mehrmaliges Ctrl+V füllt so mehrere Takte. Im Menü des Rechtsklicks
  außerdem **Paste here** für die Stelle unter der Maus.
- **Ctrl+D** — die Auswahl direkt hinter sich selbst noch einmal einfügen.
- **Verschieben:** im Instrument-Editor eine der markierten Noten ziehen —
  alle gehen in Zeit und Tonhöhe mit. In beiden Editoren die Pfeiltasten:
  links und rechts um einen Schritt, hinauf und hinunter um eine Spur oder
  einen Halbton, mit Shift um eine Oktave.
- **Entf** — löscht die markierten Noten oder leert die markierten Schritte.
- **Zwischen Mustern:** das Kopierte bleibt gemerkt, wenn der Editor eine
  andere Basis zeigt — so kommt ein Teilmuster von einer Basis in eine andere.
- Die **Lautstärken** je Schritt und je Note gehen mit.
- Jede dieser Aktionen ist **ein Schritt für Undo**.

**Einfügen im Drum-Editor — Entscheid V1 a (§3):** Ctrl+V **ersetzt** am Ziel
genau den kopierten Bereich: wo die Kopie einen Schritt hat, steht danach
einer; wo sie leer ist, ist es danach leer. Im Menü des Rechtsklicks
zusätzlich **Paste and keep existing steps**, das nur dazulegt. Im
Instrument-Editor kommen Noten dazu; nur eine Note auf demselben Schritt und
derselben Tonhöhe wird ersetzt.

**Mit der Skizze gebilligt (von mir vorgeschlagen):**

- Im Drum-Editor wird nach **Spurplatz** eingefügt: die erste kopierte Spur
  landet in der Spur, in der die Auswahl begann. Hat das Zielmuster weniger
  Spuren, fallen die überzähligen weg.
- Im Instrument-Editor behalten eingefügte Noten ihre **Tonhöhe**; sie sind
  danach markiert und lassen sich als Gruppe ziehen.
- Was über das Ende des Musters hinausreicht, wird abgeschnitten — der
  Editor **sagt dann, wie viel nicht gepasst hat**, statt es still
  wegzulassen.
- Die Einfügemarke ist nur eine Stelle im Fenster; sie steht nicht in der
  Datei.

**Nicht drin:** Kopieren in die Ablage des Systems (also nicht zwischen zwei
LumiViz-Fenstern); einen Bereich von Schritten im Drum-Editor mit der Maus
verschieben (dort nur Pfeiltasten — Ziehen malt); die Lautstärke aller
markierten Noten auf einmal stellen.

**Dazugekommen beim Bau (Wunsch Patrik S113, „mit Tastenkombination c
möglich?"):** **Alt+Pfeil links / rechts** dreht die Auswahl in der Zeit **im
Kreis** — was hinten über das Ende des Musters geht, kommt vorn wieder
herein. Die Pfeiltasten ohne Alt bleiben beim Verschieben mit Anhalten am
Rand (V3 a). Eine Note, die nach dem Drehen über das Ende reichte, wird dort
gekürzt (eine Note endet mit ihrem Muster). Die Tasten Alt+Pfeil sind von mir
gesetzt.

### Die Regeln (gebaut, 13a)

`loop/Auswahl.hpp`, reine Funktionen auf dem Modell — die Editoren rufen sie
noch nicht, **im Programm ist nichts davon zu sehen**:

- **Schritte** (`SchrittOrt`: Spur und Schritt): `kopiereSchritte` (jeder
  Teil mit seiner Lage zur oberen linken Ecke der Auswahl und seiner
  Lautstärke; leere Schritte zählen mit), `fuegeSchritteEin` (ersetzen nach
  V1 a oder dazulegen; gibt zurück, wie viele gesetzte Schritte keinen Platz
  hatten), `leereSchritte`, `versetzeSchritte` (V3 a), `dreheSchritte`.
- **Noten** (`NotenOrt`: Schritt und Tonhöhe): `kopiereNoten`,
  `fuegeNotenEin` (gibt zurück, wie viele Noten hinter dem Ende begonnen
  hätten und wie viele gekürzt wurden), `loescheNoten`, `versetzeNoten`
  (V3 a), `dreheNoten`.

**Von mir gesetzt, nicht entschieden:** beim Einfügen zählen als „hat nicht
gepasst" nur **gesetzte** Schritte — ein leerer Schritt der Kopie, der über
das Ende fällt, wird nicht gemeldet. Beim Versetzen und Drehen ersetzt ein
markierter Schritt, was an seinem Ziel stand; eine markierte Note ersetzt
eine nicht markierte auf demselben Schritt und derselben Tonhöhe.

Geprüft in `test_LoopAuswahl.cpp`, fünf Fälle ohne Fenster.

### Im Drum-Editor (gebaut, 13b)

`SchrittRaster` in `loop/DrumEditor.hpp` trägt die Marken, die Einfügemarke
und das Gemerkte; der Editor zeigt darunter die Hinweiszeile
(`drumSelection`). Gebaut wie die Skizze. **Dazu von mir gesetzt, nicht
entschieden:**

- **Die Einfügemarke gibt es immer** (am Anfang auf Takt 1): eine
  gestrichelte Linie mit einem kleinen Dreieck im Kopf. Ein Klick in die
  Zeile der Takte setzt sie auf den Schritt unter der Maus, hinter dem
  letzten Schritt auf das Ende.
- **Eingefügt wird in der Spur, in der die kopierte Auswahl begann** — auch
  nach dem Wechsel in eine andere Basis. **Paste here** nimmt die Spur und
  den Schritt unter der Maus.
- **Ein Klick ohne Taste setzt weiter einen Schritt; die Marken bleiben
  dabei stehen.** Aufgehoben werden sie mit Esc oder einem Klick unter die
  Spuren.
- **Ctrl+Shift+V** legt dazu, wie **Paste and keep existing steps** im Menü.
- **Mehrere Pfeiltasten hintereinander** rechnen von der Stelle, an der die
  Auswahl beim ersten Druck stand: ein Schritt, den sie unterwegs überfährt,
  ist danach wieder da. Nur wo sie am Ende liegt, ersetzt sie. Jeder
  Tastendruck ist ein Schritt für Undo.
- Die Meldung „n step(s) did not fit into the pattern" steht vorn in der
  Hinweiszeile, bis die nächste Aktion sie ablöst.
- Mit Shift gezogen kommt bei gehaltener Ctrl-Taste der Rahmen zu den Marken
  dazu.
- Ändert sich das Muster von außen (Bars, Raster, Undo), bleiben die Marken,
  wo es ihre Schritte noch gibt — sie wandern nicht mit dem Inhalt.

**Nicht drin:** Shift+Ziehen in der Zeile Step volume; die Marken nach Undo
dorthin zurücksetzen, wo sie vorher standen.

Geprüft in `test_LoopDrumEditor.cpp` (ein Fall mit sieben Teilen, über Maus
und Tasten am Raster); `LUMIVIZ_DRUM_MARKEN_BILD=<datei.png>` legt ein Bild
des Editors mit markiertem Bereich und Einfügemarke ab — angesehen. **Nicht
geprüft:** das Menü des Rechtsklicks; ob die Tasten im gezeigten Fenster beim
Raster ankommen (es bekommt den Fokus mit dem Klick). **Im Programm nicht
gesehen.**

### Im Instrument-Editor (gebaut, 13c)

`NotenRaster` in `loop/InstrumentEditor.hpp` trägt die Marken, die
Einfügemarke und das Gemerkte; der Editor zeigt darunter die Hinweiszeile
(`instrumentSelection`). Gebaut wie die Skizze: Shift+Ziehen zieht einen
Rahmen, Ctrl+Klick nimmt eine Note dazu oder heraus, Ctrl+A, Esc; Ctrl+C,
Ctrl+X, Ctrl+V an der Einfügemarke, Ctrl+D, Entf; eine von mehreren
markierten ziehen bewegt alle in Zeit und Tonhöhe; Pfeiltasten (hinauf und
hinunter ein Halbton, mit Shift eine Oktave), Alt+Pfeil dreht im Kreis.
**Dazu von mir gesetzt, nicht entschieden:**

- **Der Rechtsklick auf eine Note** löschte sie in 13c weiter wie seit S110,
  das Menü (Copy, Cut, Paste here, Duplicate, Delete, Select all) öffnete
  sich nur auf einer freien Stelle oder einer von mehreren markierten Noten.
  **Mit 13d abgelöst** durch den Haken „Right-click deletes", siehe unten.
- **Ein Klick auf eine Note markiert nur sie**; ein Klick auf eine freie
  Stelle setzt weiter eine neue Note, die danach markiert ist.
- **Ein Klick ohne Zug** auf eine von mehreren markierten lässt beim
  Loslassen nur sie markiert; die Kante einer Note gehört immer ihr allein.
- **Gruppe ziehen:** ist für eine der Noten kein Platz mehr (ein Rand),
  bleibt die Gruppe an der letzten Stelle, an der Platz war. Eine nicht
  markierte Note auf demselben Schritt und derselben Tonhöhe wird ersetzt.
- **Mehrere Pfeiltasten hintereinander** rechnen vom Stand beim ersten Druck:
  eine Note, die die Auswahl unterwegs überdeckt, ist danach wieder da.
- **Paste here** fügt ab dem Schritt unter der Maus ein, in den Tonhöhen der
  Kopie.
- Die Meldung nennt beides getrennt: „n note(s) did not fit into the
  pattern" und „n note(s) shortened at the end of the pattern" — auch nach
  dem Drehen, wenn eine Note am Ende gekürzt wurde. Zurückgedreht bekommt sie
  ihre Länge wieder, solange dieselbe Auswahl gedreht wird.
- Bei einer einzelnen markierten Note nennt die Hinweiszeile auch
  „Shift+drag selects several".

**Nicht drin:** die Lautstärke aller markierten Noten auf einmal (die Zeile
Note vol. gilt weiter der gewählten Note oder allen an einem Schritt); der
Rahmen rollt den Ausschnitt nicht mit.

Geprüft in `test_LoopInstrumentEditor.cpp` (ein Fall mit sechs Teilen, über
Maus und Tasten am Raster); `LUMIVIZ_INSTRUMENT_MARKEN_BILD=<datei.png>` legt
ein Bild des Rasters mit drei markierten Noten und der Einfügemarke ab —
angesehen. **Nicht geprüft:** das Menü selbst; die Tasten im gezeigten
Fenster. **Im Programm nicht gesehen.**

### Der Haken „Right-click deletes" (gebaut, 13d — Wunsch Patrik S113)

Beim Bau von 13c stieß die Skizze (Einträge im Menü des Rechtsklicks) auf das
Verhalten seit S110 (der Rechtsklick auf eine Note löscht sie). Patrik: ein
Haken im Editor schaltet das Löschen mit Rechtsklick ein; sonst steht im Menü
ein Eintrag dafür — in beiden Editoren.

- **Haken aus (Vorgabe):** der Rechtsklick öffnet immer das Menü. Im
  Instrument-Editor markiert er dabei die Note unter der Maus, das Menü hat
  **Delete**. Im Drum-Editor hat es **Clear steps** für die markierten — und
  ohne Marken **Clear this step** für den Schritt unter der Maus.
- **Haken an:** der Rechtsklick löscht die Note oder leert den gesetzten
  Schritt unter der Maus sofort, ohne Menü. Auf einer von mehreren markierten
  und auf einer freien Stelle öffnet sich weiter das Menü.
- Der Haken steht rechts neben der Hinweiszeile unter dem Raster
  (`drumRightClickDeletes`, `instrumentRightClickDeletes`).

**Von mir gesetzt, nicht entschieden:** anfangs aus — damit löscht der
Rechtsklick im Instrument-Editor **nicht mehr von selbst**, anders als seit
S110; die App merkt die Stellung **je Editor** (`loop/drumRightClickDeletes`,
`loop/notesRightClickDeletes`), nicht für beide gemeinsam.

Geprüft: der Haken, seine Wirkung und das Merken in `test_LoopDrumEditor.cpp`
(ein Teil mehr im Fall zu den Marken); im Instrument-Editor die Wirkung am
Raster in zwei Fällen von `test_LoopInstrumentEditor.cpp` — der Haken selbst
dort nur auf sein Dasein (der Fall hat keine eigenen Einstellungen und soll
die des Rechners nicht verstellen).

**Vor dem Bau geklärt** (Patrik S113):

- **V2 a — verstreute Schritte:** kopiert werden genau die markierten
  Schritte mit ihrer Lage zueinander. Einfügen ersetzt am Ziel nur diese
  Stellen; was dazwischen liegt, bleibt. Ein Rahmen ist nur der Fall, in dem
  alle Schritte eines Rechtecks markiert sind. Verworfen: b — das
  umschließende Rechteck zählt (ein Ctrl+Klick auf zwei entfernte Schritte
  leerte beim Einfügen alles dazwischen, ohne dass es markiert war).
- **V3 a — Pfeiltasten am Rand:** stößt eines der markierten Teile an einen
  Rand (Anfang oder Ende des Musters, oberste oder unterste Spur, C1 oder
  C7), bewegt sich die ganze Auswahl in diese Richtung nicht weiter; nichts
  geht verloren. Verworfen: b — abschneiden (bei gehaltener Taste ein
  schleichender Verlust); c — in der Zeit im Kreis laufen (ein nützlicher
  Handgriff, aber ein anderer als Verschieben; später als eigener Befehl
  denkbar).

---

## 4j. Signal path: Filter und Effekte als Kette (Stufe 16 — Skizzen gezeigt, Entscheide K1 a bis K4 a, Patrik S117; Baustufen freigegeben S118, B1 a; 16a gebaut)

Anlass: Wunsch Patrik S115 (§4h, „die Kaskade als Kette, in einer Art
Node-Editor, auch für die Effekte"). Vorgelegt in S117 mit zwei Skizzen aus
dem Zeichenwerkzeug; Patrik hat zu jeder der vier Fragen Option a genommen.
Ein ausdrückliches „Skizze gebilligt" gibt es nicht — vor dem Bau des
Creators (16c) noch einmal zeigen.

**Befund zum Unterbau der Effect Chain der Visuals** (Köpfe gelesen:
`EffectChain.hpp`, `MultiEffectPanel.hpp`; nicht der ganze Code): sie ist ein
Baum (`ChainTreeWidget`) mit Ziehen und Ablegen, kein Graph — eine Fläche mit
Knoten und Leitungen gibt es im Programm nirgends. Ihr Modell (Parametersätze
je Bild-Modul, Zeichnen mit OpenGL) ist für Ton nicht übertragbar; übertragbar
ist das Bedienmuster (aus einer Palette holen, per Ziehen umordnen, die Regler
des gewählten Bausteins daneben, Undo über denselben Weg).

**Entscheid K1 a — die Form: eine Reihe von Bausteinen.** Kein freier Graph
mit Leitungen (b), kein Baum wie im Panel Effect Chain (c). Verworfen b, weil
das Nebeneinander schon die Schichten leisten und jeder Effekt einen
Mix-Regler hat, und weil es eine neue Zeichenfläche, ein neues Rechenmodell
und den Umbau der gebilligten Schichten-Liste verlangte; eine Reihe ist ein
Graph ohne Abzweigung, b bleibt möglich. Verworfen c, weil im Creator die
Höhe knapp ist und eine waagrechte Reihe sich als Weg des Tons liest.

**Die Skizze als Beschreibung — Creator, je Schicht:** eine Zeile **Signal
path — Layer n "Name"**. Von links nach rechts Kästen, mit Pfeilen
verbunden: fest und grau **Source** (zweite Zeile: die Waveform, etwa
„Noise" oder „Sine ↓ 22 st"), dann die Bausteine — je ein Kasten mit Name und
einer Zeile Kurzwert („Filter / High-pass ×2.3", „Drive / 40 %", „Envelope /
Decay 210 ms") —, dann der Knopf **+ Add**, dann fest und grau **Level**
(„100 %"). Der gewählte Baustein trägt einen kräftigen Rand; **unter der
Reihe** stehen seine Regler (bei Filter: Type, Cutoff, Resonance) und drei
Knöpfe **← Move**, **Move →**, **Remove**. Ziehen ordnet um. Envelope ist
gestrichelt umrandet. Hinter Add: **Filter**, **Drive**, und grau **Envelope
— already in the path**.

**Die Skizze als Beschreibung — Fenster Clip effects:** dieselbe Zeile,
**Signal path — 2 clips: Beat (bar 21), Mel (bar 21)**: fest **Clip**
(„dry"), die Bausteine („Echo / 1/8 dotted · 35 %", „Distortion / Drive 30 %",
„Reverb / 1.8 s · 25 %"), **+ Add**, fest **Out** („Peak 0.62"). Sie ersetzt
die drei festen Gruppen und die Zeile „Order: Distortion, Echo, Reverb".

**Entscheid K2 a — die Bausteine einer Schicht:** **Source** (fest): alles,
was den Ton erzeugt — Waveform, Voices, Detune, Harmonics, Damping, die
Sample-Felder, Pitch sweep, Vibrato, dazu Transpose und Start offset.
**Filter** beliebig oft (Type, Cutoff, Resonance, Filter envelope — sie folgt
weiter der Hüllkurve der Schicht). **Drive** beliebig oft. **Envelope** genau
einmal, verschiebbar, nicht entfernbar. **Level** (fest). Eine alte Datei
öffnet als Filter → Drive → Envelope und klingt Wert für Wert wie bisher; ein
Filter mit Cutoff „open" und ein Drive mit 0 % erscheinen nicht als Baustein;
eine neue Schicht hat nur Envelope. Damit sind zwei der drei vorgemerkten
Drum-Bausteine (§4h) Stellungen der Reihe: zwei Filter (Clap), Envelope vor
Drive (Kick). Verworfen b (Envelope fest am Ende, Drive mit Haken „After
envelope" — die Reihe zeigte dann nicht, was gerechnet wird), c (auch Pitch
sweep und Vibrato als Bausteine — sie sind keine Stationen des Tons). Die
Gruppen Envelope und Filter des Creators fallen weg, Oscillator und Vibrato
werden zu den Reglern von Source. Die Schwingung aus sechs Rechtecken
(Hi-Hats) ist kein Glied der Kette, sondern eine Waveform — weiter offen,
nach Patriks Ohr.

**Entscheid K3 a — die Bausteine am Clip:** **Filter**, **Distortion**,
**Echo**, **Reverb**, jeder beliebig oft, in freier Reihenfolge. Der Filter
am Clip hat Type und Resonance wie an der Schicht, aber **Cutoff in Hertz**
(40 Hz bis 18 kHz) und keinen Filter envelope. An der Schicht gibt es Echo
und Reverb nicht; Drive (Schicht) und Distortion (Clip) bleiben zwei
Bausteine. Autotune käme später als weiterer Baustein am Clip. Verworfen b
(ein Satz für beide Stellen: Echo und Reverb je Schicht, Drive und Distortion
vereint — nicht gemessen, was ein Nachklang von bis zu acht Sekunden an jedem
vorgerechneten Ton kostet; nicht geprüft, ob ein alter Drive als Distortion
gleich klingt; lässt sich später aufsetzen), c (kein Filter am Clip). Der
Haken je Effekt bleibt als Haken am Baustein: ausgeschaltet steht er in der
Reihe und wird übersprungen. Copy, Paste und Reset gelten für die ganze
Reihe.

**Entscheid K4 a — die Obergrenze:** höchstens **acht** Bausteine je Reihe,
an Schicht und Clip gleich (`kBausteineMax`); Envelope zählt mit,
ausgeschaltete auch. Ab sechs bricht die Reihe in eine zweite Zeile um; bei
acht ist Add grau („8 of 8"); eine Datei mit mehr wird nach dem achten
abgeschnitten. Grundlage: am Clip kostet Hall rund 1 ms je Sekunde Ton
(§4g, gemessen S113) — acht davon gerechnet unter 1 % eines Kerns; an der
Schicht ist nichts gemessen. Verworfen b (fünf und sechs, was in eine Zeile
passt), c (keine Grenze). Die vier Schichten je Instrument bleiben.

**Von Claude gesetzt, nicht entschieden:** der Name **Signal path** („Effect
Chain" heißt schon das Panel der Visuals); Source und Level fest an den
Enden; die Regler unter der Reihe; der Bereich von Cutoff am Clip; dass ein
Filter vor oder hinter Envelope kaum anders klingt, steht als Satz im
Handbuch.

**Baustufen — vorgelegt S117, freigegeben S118 (Entscheid Patrik B1 a: in
dieser Reihenfolge, die Drums erst, wenn Creator und Clip effects die Reihe
zeigen; verworfen b — Drums vorgezogen, mit gesperrtem Creator für Kick und
Clap —, c — nur 16a und 16b):**

| Stufe | Inhalt | Beleg |
|---|---|---|
| 16a | **Modell und Dateien, ohne Klang und ohne Fenster:** die Schicht trägt eine Reihe von Bausteinen statt der festen Felder für Filter und Drive; der Clip eine Reihe statt dreier fester Effekte. `.lvinst` Fassung 7, `.lvloop` Fassung 5; Fassungen davor lesen als die heutige Reihenfolge | jede mitgelieferte `.lvinst` und `.lvloop` liest zu derselben Reihe, die sie heute fest hat; Schreiben und Lesen ergeben dasselbe |
| 16b | **Der Klangerzeuger rechnet die Reihe:** an der Schicht für Schwingung, Rauschen und Sample; am Clip in freier Reihenfolge, mit dem Filter in Hertz | **vor dem Umbau** je mitgeliefertem Instrument und Beispiel-Loop die gerechneten Werte festhalten — danach dieselben Werte; Messung der Last: eine Schicht mit acht Bausteinen, ein Clip mit acht |
| 16c | **Creator:** die Zeile Signal path je Schicht, die Regler darunter, Add, Move, Remove, Ziehen; die Gruppen Envelope und Filter entfallen | die Skizze vorher noch einmal zeigen; das Bild aus dem Test ansehen; Ziehen mit Zeigerwechsel und Hinweis (Bedienung auffindbar) |
| 16d | **Fenster Clip effects:** die Zeile statt der drei Gruppen, Filter als vierter Baustein | davor **K5** entscheiden (unten); Bild aus dem Test |
| 16e | **Die Drums nachziehen und Abschluss:** Kick mit Drive hinter Envelope, Clap mit zwei Filtern — neu gegen das Kit gemessen, neue Hörproben; Benutzerhandbuch, Sichttest-Liste, voller Lauf | die Tabelle aus 11f mit den neuen Zahlen daneben |

**16a gebaut (S118) — Modell und Dateien.**

- **Schicht** (`loop/Instrument.hpp`): `reihe`, eine Liste aus
  `SchichtFilter` (Art, Cutoff als Vielfaches des Grundtons, Filter envelope,
  Resonance), `SchichtDrive` und `SchichtHuelle`. Die festen Felder für
  Filter und Drive gibt es nicht mehr; Attack, Decay, Sustain und Release
  bleiben an der Schicht, `SchichtHuelle` ist nur die Stelle. `richteReihe`:
  genau eine Envelope, höchstens `kBausteineMax` (`loop/Bausteine.hpp`).
- **Clip** (`loop/Loop.hpp`): `ClipEffekte::reihe`, eine Liste aus
  `ClipFilter` (neu: Art, Cutoff in Hertz, Resonance, Haken), `ClipVerzerrung`,
  `ClipEcho`, `ClipHall`. Ein Clip ohne Effekte hat eine leere Reihe.
- **`.lvinst` Fassung 7:** je Schicht die Liste `path` mit Einträgen
  `{"type": "filter", "filterType", "cutoff", "envelope", "resonance"}`,
  `{"type": "drive", "amount"}`, `{"type": "envelope"}`; die flachen Felder
  `drive`, `filterCutoff`, `filterEnvelope`, `filterType`, `filterResonance`
  entfallen. Eine Datei bis Fassung 6 liest sich als Filter, Drive, Envelope —
  ein Filter mit Cutoff 0 und ein Drive mit 0 werden kein Baustein.
- **`.lvloop` Fassung 5:** je Clip die Liste `path` mit Einträgen
  `{"type": "filter" | "distortion" | "echo" | "reverb", "on", …}` statt des
  Felds `effects`. Eine Datei bis Fassung 4 liest ihre `effects` als
  Distortion, Echo, Reverb; was dort nicht stand, wird kein Baustein. Einen
  unbekannten Baustein lässt das Programm weg und nennt ihn im Bericht
  („Unknown effect '…'").
- **Die mitgelieferten Dateien bleiben, wie sie sind** (Fassung 3 bis 6, 2
  bis 4); das Programm schreibt die neuen Fassungen erst beim Speichern.
- **Übergang bis 16b, 16c, 16d** (der Klangerzeuger seit 16b nicht mehr,
  siehe unten): Klangerzeuger, Creator und das Fenster Clip
  effects arbeiten noch mit dem **ersten** Baustein je Art in der alten Folge
  (`Schicht::filter`, `drive`, `holeFilter`, `holeDrive`;
  `ClipEffekte::wert`, `hole`). Für jede Datei bis Fassung 6 und 4 ist das die
  ganze Reihe — sie klingt und zeigt sich wie bisher. Eine Reihe mit zwei
  Filtern oder einer anderen Folge wird gelesen, gespeichert und behalten,
  aber noch nicht so gerechnet und nicht gezeigt. Der Filter am Clip steht
  im Modell und in der Datei, gerechnet wird er ab 16b.
- **Beleg** (`test_LoopBausteine.cpp`, fünf Fälle): jede der 55
  mitgelieferten `.lvinst` (113 Schichten, davon 48 mit Filter, 22 mit Drive)
  und jede der 34 `.lvloop` (210 Clips, davon 12 mit Effekten) liest zu der
  Reihe, die sie fest hatte; Schreiben und Lesen ergeben dasselbe.
- **Von Claude gesetzt, nicht entschieden:** die Namen in den Dateien (`path`,
  `type`, `amount`, `cutoff`, `envelope`, `resonance`); ein Baustein der
  Schicht hat keinen Haken zum Ausschalten; die Vorgabe des Filters am Clip
  (aus, Low-pass, 1000 Hz); eine `.lvloop` der Fassung 5 trägt das alte Feld
  `effects` nicht daneben — ein älteres LumiViz spielt ihre Clips ohne
  Effekte und meldet die neuere Fassung; im alten Creator bleibt ein Filter,
  dessen Cutoff auf „open" zurückgestellt wird, als Baustein in der Reihe.

**16b gebaut (S118) — der Klangerzeuger rechnet die Reihe.**

- **Schicht** (`Instrument.cpp`, `Reihe`): je Abtastwert läuft der Ton durch
  die Bausteine von vorn nach hinten — jeder Filter mit eigenem Gedächtnis
  (bei einer Aufnahme je Seite), Drive, und die Stelle, an der die Hüllkurve
  ihn formt. Filter envelope folgt weiter der Hüllkurve der Schicht, gleich
  wo der Filter steht. Ein offener Filter und ein Drive mit 0 kommen nicht
  vor. Damit gehen zwei Filter (Clap) und Drive hinter Envelope (Kick).
- **Clip** (`Effekte.cpp`, `EffektKette`): je Baustein der Reihe ein Glied,
  in ihrer Folge; ein ausgeschalteter wird übersprungen. Neu `FilterEffekt`:
  Low-pass, High-pass, Band-pass zweiter Ordnung mit der Grenze in Hertz,
  Resonance wie am Filter der Schicht. Beim Wechsel des Programms gehört der
  erste Baustein einer Art zum ersten derselben Art davor, der zweite zum
  zweiten: wer einen Baustein verschiebt oder einen anderer Art dazusetzt,
  reißt keinen Nachklang ab.
- **Beleg, alte Dateien** (`test_LoopWerte.cpp`): vor dem Umbau je
  mitgeliefertem Instrument drei Töne und je `.lvloop` ein Durchlauf durch
  den Klangerzeuger festgehalten — 199 Einträge, je Compiler eine Prüfsumme
  über jeden Abtastwert. Nach dem Umbau **199 von 199 Bit für Bit gleich**,
  unter Testing-MSVC und unter Release-clang. Die Zahlen (Abtastwerte,
  Spitze, Summe der Beträge) liegen als `werte/loop_werte.txt` neben den
  Tests und werden bei jedem Lauf auf ein Tausendstel verglichen; nach einer
  gewollten Änderung am Klang schreibt `LUMIVIZ_LOOP_WERTE_SCHREIBEN=1` sie
  neu (fällig mit 16e).
- **Messung der Last** (ein Rechner, je drei Läufe, der beste; die Taktprobe
  blieb gleich):

  | | Release-clang | Testing-MSVC |
  |---|---|---|
  | Schicht mit Filter, Drive, Envelope, je Sekunde Ton | 1,9 ms | 4,4 ms |
  | Schicht mit acht Bausteinen (vier Filter, drei Drive) | 3,7 ms | 9,5 ms |
  | Clip mit Distortion, Echo, Reverb, je Sekunde Ton | 1,7 ms (0,17 % eines Kerns) | 7,2 ms (0,72 %) |
  | Clip mit acht Reverb | 7,5 ms (0,75 %) | 45 ms (4,5 %) |
  | Clip mit acht Filtern | 1,6 ms (0,16 %) | 3,8 ms (0,38 %) |

  Ein Ton der Schicht wird vorab gerechnet, nicht im Ton-Thread; acht
  Bausteine kosten dort rund das Doppelte. Die Annahme aus K4 a (acht Reverb
  unter 1 % eines Kerns) hält im Release-Bau; im Testing-Bau sind es 4,5 %.
- **Von Claude gesetzt, nicht entschieden:** die Saite überspringt die Filter
  der Reihe weiter (sie hatte nie einen — sonst klängen alte Instrumente mit
  Saite und Cutoff anders), Drive und die Stelle der Envelope gelten für sie;
  steht ein Filter hinter der Envelope, werden die letzten 5 ms des Tons auf
  null geführt (er schwingt nach, wenn die Hüllkurve schon zu ist); der
  Low-pass am Clip rechnet auch ohne Resonance als Filter zweiter Ordnung
  (an der Schicht ist er dann der alte, flachere); der Clip klingt mit einem
  Filter um dessen Ausschwingen länger nach (bei 40 Hz und voller Resonance
  0,6 s).
- **Im Programm** ist davon nichts zu erreichen, bis Creator (16c) und Clip
  effects (16d) die Reihe zeigen; eine Datei mit einer anderen Reihe klingt
  aber schon so.

**16c gebaut (S118) — der Creator zeigt die Reihe. Skizze gebilligt (Patrik
S118, Entscheid S1 a), mit drei Ergänzungen:** Source und Level sind
anklickbar und zeigen ihre Regler unter der Reihe; ein neuer Baustein kommt
hinter den gewählten; beim Öffnen einer Schicht ist Source gewählt.
Verworfen b (Level, Transpose und Start offset fest über der Reihe), c
(Änderungen).

- **Die Zeile** ist ein eigenes Widget (`loop/SignalWeg`): Kästen mit Name
  und Kurzwert, Pfeile, der Knopf + Add vor dem letzten Kasten. Es kennt
  weder Schicht noch Clip — das Fenster Clip effects benutzt es in 16d
  wieder. Jeder Kasten ist so breit wie sein Text; passt die Reihe nicht in
  die Breite, bricht sie um (im Fenster in seiner Mindestbreite stehen vier
  Kästen in einer Zeile, acht Bausteine brauchen drei).
- **Unter der Zeile** der Name des gewählten Kastens, bei einem Baustein
  **← Move**, **Move →**, **Remove** (bei Envelope grau), darunter nur seine
  Regler: Source — Transpose, Start offset, Oscillator, Vibrato; Filter —
  Type, Cutoff, Resonance, Filter envelope; Drive; Envelope — Attack bis
  Release; Level. Die Regler von Filter und Drive stellen den Baustein, der
  gewählt ist: zwei Filter haben je ihre Werte.
- **Ziehen** ordnet um, auch die Envelope: der Zeiger wird über einem
  Baustein zur offenen Hand, beim Ziehen zur geschlossenen, ein Strich zeigt
  den Platz; neben dem Titel steht „click a block · drag to move it“, jeder
  Kasten hat einen Tooltip.
- **Add:** ein Filter kommt als Low-pass mit Cutoff × 4, ein Drive mit 30 % —
  beide sind gleich zu hören. Bei acht steht „8 of 8“.
- **Die Gruppen Envelope und Filter** gibt es nicht mehr; einklappen lassen
  sich Oscillator und Vibrato (unter Source). Der Regler „Envelope“ des
  Filters heißt jetzt **Filter envelope**.
- **Bild aus dem Test** (`LUMIVIZ_CREATOR_PATH_BILD`) angesehen, zweimal
  nachgebessert: die Kurzwerte waren abgeschnitten („High-pass …“) — die
  Kästen richten sich jetzt nach ihrem Text; der gestrichelte Rand der
  Envelope war im dunklen Design nicht zu sehen.
- **Von Claude gesetzt, nicht entschieden:** die Vorgaben eines neuen
  Filters und Drive; der Hinweis steht neben dem Titel statt in einer eigenen
  Zeile (jede Zeile macht das Fenster höher); ein Filter mit Cutoff „open“
  und einer an einer Saite stehen matt in der Zeile („open“, „skipped“);
  Remove an der Envelope ist grau statt weg; Titel der Zeile mit dem Namen
  der Schicht in Anführungszeichen.

**Entscheid K5 a (Patrik S118, mit Skizze) — mehrere markierte Clips mit
verschiedenen Reihen:** haben alle markierten Clips dieselbe Folge von
Bausteinen (gleich viele, je Platz dieselbe Art), zeigt das Fenster sie
gemeinsam wie bisher („mixed", wo Werte abweichen). Sonst zeigt die Zeile die
Reihe **eines** Clips, wählbar unter **Showing**; Regler, Add, Move und
Remove gelten für alle markierten Clips mit derselben Folge. Ein Hinweis
nennt, wie viele das sind und welche bleiben, wie sie sind. **Use this path
for all** setzt die gezeigte Reihe mit ihren Werten auf alle markierten
Clips. Copy, Paste und Reset gelten weiter für alle. Verworfen b (bei
verschiedenen Reihen nur angleichen), c (jede Änderung gleicht an — ein Dreh
am Regler ersetzte die Effekte anderer Clips ohne Warnung), d (nach Art
zuordnen: ein Regler am ersten Echo stellt in jedem Clip dessen erstes Echo —
dem alten Gebrauch am nächsten, aber die Zeile zeigte für einen Teil der
Clips eine Folge, die sie nicht haben; kippt dorthin, wenn Patrik oft viele
Clips mit verschiedenen Effekten zugleich nachzieht). Das trifft auch alte
Loops: ein Clip nur mit Echo und einer nur mit Reverb haben verschiedene
Reihen.

**Die Skizze als Beschreibung — Clip effects bei verschiedenen Reihen:** im
Kopf rechts **Showing** mit einer Auswahl der markierten Clips („Beat (bar
21)"); darunter ein Kasten mit dem Hinweis und rechts darin der Knopf **Use
this path for all 3**; darunter die Zeile des gezeigten Clips. Bei gleicher
Folge fehlen Showing und der Hinweis.

**16d gebaut (S118) — das Fenster Clip effects zeigt die Reihe.**

- **Die Zeile** (`SignalWeg`, wie im Creator): fest **Clip** („dry"), die
  Bausteine mit Kurzwert („1/8 dotted · 35 %", „1.8 s · 25 %", „Low-pass
  800 Hz", „Drive 30 %"), **+ Add**, fest **Out** („Peak 0.62"). Sie ersetzt
  die drei festen Gruppen und die Zeile „Order: …".
- **Unter der Zeile** der gewählte Baustein: sein Haken mit seinem Namen
  (aus: er steht grau in der Zeile und wird übersprungen), **← Move**,
  **Move →**, **Remove**, darunter seine Regler — je Art eine Seite, das
  Fenster ist so hoch wie die höchste und springt beim Wechsel nicht.
- **Filter** als vierter Baustein: Type, Cutoff (40 Hz bis 18 kHz, der
  Schieber geht gleichmäßig in Oktaven), Resonance.
- **Add** holt einen Baustein eingeschaltet, mit seinen Vorgaben, hinter den
  gewählten.
- **K5 a:** Showing, der Hinweis und Use this path for all, wie entschieden.
  Der Kurzwert eines Kastens sagt, welcher Teil abweicht („mixed · 35 %").
- **Abweichung im Kasten (S119; Befund Patrik zu X5 mit drei Bildern,
  Entscheid X5 a):** der Kurzwert zeigt nur zwei Werte (beim Echo Time und
  Mix) — wich Feedback oder Damping ab, sah man es erst am Regler des
  gewählten Bausteins. Jetzt: weicht **irgendein** Wert des Bausteins unter
  den Clips mit derselben Folge ab (auch an/aus), steht vor dem Kurzwert
  „≠" (`WegKasten::weichtAb`), und der Tooltip nennt die Regler, wie sie im
  Fenster heißen: „Differs between the clips: Feedback, Damping". Verworfen
  b (nur die Farbe), c („· mixed" anhängen).
  **Die Fläche trägt die Farbe (Vorschlag Patrik, X5b):** der Kasten ist
  bernsteinfarben getönt (`SignalWeg::abweichFlaeche`: sein Grund, zu 22 %
  im Ton `abweichFarbe`), der Rand bleibt frei für die Wahl — die Tönung ist
  auch am gewählten Kasten zu sehen. Zuerst gebaut und im Bild verglichen:
  ein bernsteinfarbener Rand am nicht gewählten Kasten; er sagte am gewählten
  nichts. **Der Kurzwert muss auf der Fläche sauber zu lesen sein (Patrik):**
  er steht in `abweichSchrift`, einem hellen Bernstein im dunklen Design und
  einem dunklen im hellen; der Test misst den Abstand der Helligkeiten nach
  WCAG und verlangt 7 zu 1 (gemessen 7,3 dunkel, 8,1 hell, 7,1 mit den Farben
  des Testlaufs). Von Claude gesetzt: die Töne, die 22 %, die Schwelle 7 zu 1,
  „On/off" als Name für den Haken. Das Bild aus dem Test angesehen; das helle
  Design nur gerechnet, nicht im Bild.
- **Reset** entfernt jetzt die Bausteine (vorher: alle drei aus, Werte auf
  Vorgabe) — ein Clip ohne Effekte hat eine leere Reihe.
- **Bild aus dem Test** (`LUMIVIZ_CLIP_EFFEKTE_BILD`) angesehen und
  nachgebessert: der umbrochene Hinweis war abgeschnitten (jetzt zwei feste
  Zeilen: „2 of 3 clips have this path." / „Not changed from here: …"), der
  Kopf war gekürzt (jetzt kürzt die Liste der Namen), ein Kasten zeigte nur
  „mixed".
- **Von Claude gesetzt, nicht entschieden:** der Wortlaut des Hinweises;
  der Kurzwert je Baustein; die Mindestbreite des Fensters (520); andere
  Clips markieren wählt den ersten Baustein des ersten Clips; die Spitze
  steht im Kopf und im Kasten Out; Copy merkt den gezeigten Clip.

**16e gebaut (S118) — die Drums nachgezogen.**

- **Kick** (`drum_kick`, `.lvinst` Fassung 7): die Schicht Body mit der Reihe
  **Envelope → Drive 19 %** statt Drive → Envelope. Der Verzerrer wirkt damit
  auf den Ton, wie die Hüllkurve ihn formt: hart am Anschlag, und er lässt mit
  dem Ton nach — wie das `tanh(1,5 · Ton)` des Kits (8 · 0,19 ≈ 1,5). Sweep
  time 105 ms statt 80 (das Dreifache der 35 ms des Kits).
- **Clap** (`drum_clap`, Fassung 7): jede der vier Schichten mit der Reihe
  **High-pass ×2,0 → Band-pass ×4,6 (Resonance 19 %) → Envelope → Drive 40 %**
  statt eines Bandpasses um 1,7 kHz. Von C4 aus sind das 520 Hz und 1,2 kHz
  mit Güte 1,2 — die beiden Filter des Kits. Drive hebt weiter den Pegel,
  jetzt hinter der Envelope; der Ausklang hat Decay 190 ms statt 210, weil
  Drive ihn länger hält.
- Tom, Snare und die Hi-Hats sind unverändert (Fassung 6). Die **Schwingung
  aus sechs Rechtecken** für die Hi-Hats ist kein Baustein der Reihe und
  bleibt offen, nach Patriks Ohr.
- **Gegen das Kit gemessen, nicht gehört** — dieselbe Messung wie in 11f
  (§4h), die alten Zahlen daneben:

  | | Hüllkurve | Bänder, mittel / größter | klingt (Kit / Instrument) | Spitze |
  |---|---|---|---|---|
  | Kick, 11f | 0,91 | 4,9 / 15,2 dB | 439 / 360 ms | 0,57 |
  | **Kick, 16e** | 0,83 (in Stücken von 25 ms: 1,00) | **0,9 / 1,6 dB** | 439 / 457 ms | 0,58 |
  | Clap, 11f | 0,97 | 4,7 / 12,5 dB | 200 / 186 ms | 0,42 |
  | **Clap, 16e** | 0,97 | **1,1 / 3,5 dB** | 200 / 198 ms | 0,43 |

  Die Hüllkurve des Kick ist in Stücken von 2 ms schlechter geworden: ein
  Stück ist kürzer als eine Schwingung von 45 Hz, der Wert folgt dann der
  Phase (§4h, Rechenversuch) — mit der längeren Sweep time liegt sie anders.
  In Stücken von 25 ms sind es 0,997.
- **Aus Reihen gewählt:** Kick 42 Varianten (Release, Drive, Sweep time, Drive
  vor und hinter der Envelope), Clap 173 (Hochpass, Bandpass, Resonance,
  Drive ohne, davor, dahinter, die Länge des Ausklangs). Der Clap ohne Drive
  liegt in den Bändern gleich gut (1,0 / 2,9 dB), ist aber halb so laut
  (Spitze 0,21) — genommen ist der mit Drive.
- **Die festgehaltenen Werte** (`werte/loop_werte.txt`) sind für Kick und Clap
  neu geschrieben: sechs von 199 Einträgen, die übrigen 193 sind Bit für Bit
  geblieben. Die mitgelieferten Loops spielen weiter das Kit.
- **Hörproben:** `.claude/handover/hoerproben_16e/` — je Klang
  `vergleich_<klang>.wav` (zweimal das Kit, zweimal das Instrument) und für
  Kick und Clap `alt_neu_<klang>.wav` (zweimal 11f, zweimal 16e).
- **Von Claude gesetzt, nicht entschieden:** welche Variante genommen ist
  (nach den Zahlen); der Clap mit Drive um des Pegels willen; der Klick des
  Kick bleibt ohne Drive.

**Offen, vor dem jeweiligen Schritt zu entscheiden:**
- **Die Hi-Hat-Schwingung** und ob die gerechneten Drums so laut sein sollen
  wie das Kit — nach Patriks Ohr (Liste 10, U64, U65).
- **K5 d** (Clips mit verschiedenen Effekten doch zusammen stellen) — nach
  Patriks Gebrauch (Liste 10, U69).

---

## 4k. Ein Sample bearbeiten: Edit... im Creator (Stufe 17 — Skizze gebilligt, Entscheide M1 a, M2 b, M2b a, M3 a, M4 a, Patrik S119; gebaut)

Wunsch Patrik (S118): an einer Sample-Schicht ein bestehendes Sample öffnen
wie nach einer Aufnahme, zuschneiden, speichern; Mitgeliefertes ist
schreibgeschützt.

**Entscheide (Patrik S119):**
- **M1 a — was der Knopf öffnet:** was die Schicht gerade spielt, gleich
  welcher Art. Ein eigenes Sample (`user:<name>`) lässt sich bearbeiten und
  speichern. Ein Klang des Kits (`kit:<name>`) und eine Tondatei an freiem Ort
  sind schreibgeschützt: das Ergebnis kommt unter einem neuen Namen zu den
  eigenen Samples, das Original bleibt. Verworfen b (freie Datei grau), c (nur
  eigene) — über **Edit sound...** im Drum-Editor nennt eine Sample-Schicht
  meist einen Klang des Kits, dort wäre der Knopf grau.
- **M2 b — überschreiben oder kopieren:** zwei Knöpfe wie beim Instrument.
  **Save** überschreibt das eigene Sample, **Save as...** fragt nach einem
  Namen und kopiert. Verworfen a (der Name im Feld entscheidet), c (immer eine
  Kopie).
- **M2b a — nach einer frischen Aufnahme:** das Fenster sieht in beiden
  Zuständen gleich aus. Das Feld für den Namen und **Save and use** entfallen;
  unten stehen **Cancel**, **Save as...**, **Save**. Nach einer Aufnahme ist
  Save grau. Dass der Name im Feld still entschied, ob etwas ersetzt wird, hat
  Patrik gestört. Verworfen b (die Aufnahme bleibt, wie sie ist).
- **M3 a — was das Sample schon spielt:** alles, was es nennt, spielt die neue
  Fassung (wie bisher beim Ersetzen, `speichereAufnahme`). Die Rückfrage von
  Save nennt die eigenen Instrumente, die es spielen. In der Schicht, aus der
  das Fenster geöffnet wurde, wird **Sample start** um das vorn
  Weggeschnittene zurückgenommen (nicht unter null); andere Instrumente
  bleiben. Verworfen b (allgemeine Rückfrage, Sample start bleibt), c (Sample
  start in allen eigenen Instrumenten nachziehen — Save schriebe Dateien, die
  nicht offen sind).
- **M4 a — die Namen:** der Knopf heißt **Edit...**, das Fenster **Edit sample
  – <Name>**, über Record... geöffnet weiter **Record sample**. Input, Pegel,
  Record und Stop bleiben im Bearbeiten-Zustand. Verworfen b (Trim..., ohne
  Record), c (Edit sample...).

**Die Skizze als Beschreibung (gebilligt, S2 a):**

*Im Creator,* Zeile Sample einer Sample-Schicht: der Name der Datei (grau),
dann **File...**, **Record...**, **Edit...**. Edit... ist grau, solange die
Schicht kein Sample nennt oder seine Daten nicht geladen sind. Tooltip: „Trim
the sample this layer plays and save it".

*Das Fenster,* von oben nach unten:
1. Titel **Edit sample – <Name>** (der Titel des Samples, bei einer Datei ihr
   Name).
2. **Input** mit der Auswahl der Eingänge und dem Pegel, wie bisher.
3. **Record**, **Stop**, daneben statt „… recorded · up to 30 s" die Länge und
   die Herkunft: „0.84 s · My samples", „0.44 s · Built-in, read-only",
   „1.20 s · File, read-only".
4. Das Bild des Samples mit den zwei Griffen; beim Öffnen stehen sie an beiden
   Enden. Das Graue fällt beim Speichern weg.
5. **Play**, **Repeat**, daneben der Ausschnitt in Sekunden.
6. **Trim silence at both ends** und **Normalize level** — beim Öffnen eines
   bestehenden Samples beide aus, damit Öffnen und Speichern nichts ändert.
   Nach einer Aufnahme ist Trim silence an, wie bisher.
7. Rechts unten **Cancel**, **Save as...**, **Save**.

*Save* ist an, wenn ein eigenes Sample offen ist. Es fragt: „Replace %1? What
you cut away is gone." und, wenn eigene Instrumente das Sample spielen, „Used
by: Table Kit, Knock Bass and 2 more — they will play the new version." Die
Datei wird `<name>.wav` (48 kHz, 16 Bit), auch wenn sie vorher eine andere
Endung trug.

*Save as...* öffnet einen kleinen Dialog mit dem Namen — vorgeschlagen der
erste freie („kick 2", nach einer Aufnahme „Recording 2") — und nimmt keinen
Namen an, den es unter den eigenen Samples schon gibt. Danach spielt die
Schicht das neue Sample.

*Record im Bearbeiten-Zustand* ersetzt das Bild durch die neue Aufnahme; der
Name im Titel bleibt, Save schreibt sie unter diesem Namen.

*Schließen mit ungespeicherter Änderung* fragt wie heute nach einer Aufnahme
(„The recording is not saved. Discard it?"), im Bearbeiten-Zustand mit „Your
changes are not saved. Discard them?".

**Von Claude gesetzt, in der Skizze genannt und nicht beanstandet:** die Folge
Cancel, Save as..., Save; die Griffe beim Öffnen an den Enden; Edit... nimmt
nur Samples bis 30 Sekunden (`AufnahmeFenster::kHoechstensSekunden`) und
meldet sonst „This file is longer than 30 s. Edit... works on samples up to
30 s.". **Gesetzt, nicht gezeigt:** der Wortlaut der Rückfrage beim Schließen;
die Herkunft in der Zeile neben Record.

**Grenzen:** die Liste „Used by" kennt die eigenen Instrumente (ihr Ordner
ist bekannt), nicht Loops an freiem Ort, deren Drum-Spur das Sample nennt.
Eine Drum-Spur trägt keinen eigenen Einsatz (`Spur` in `Loop.hpp`) — dort ist
nichts nachzuziehen.

**Baustufen (freigegeben S119, B2 a — Beginn nach Patriks Sichttest):**
- **17a — das Fenster (gebaut, S119):** `AufnahmeFenster::bearbeite` öffnet
  ein Sample mit seiner Nennung und seinen Daten; **Save** (`speichere`) und
  **Save as...** (`speichereAls`, mit einem kleinen Dialog) statt Namensfeld
  und Save and use; die Zeile mit Länge und Herkunft; die Rückfragen
  (`ersetzenFrage`); im Kern `eigeneInstrumenteMitSample` und
  `vorschlagKopieTitel`. Das Signal `gespeichert` trägt jetzt mit, wie viel
  vorn weggeschnitten ist (für 17b). Im Einzelnen:
  - **Geblendet wird nur, wo geschnitten ist.** Nach einer Aufnahme blendet
    der Ausschnitt an beiden Enden 2 ms (wie bisher). An einem geöffneten
    Sample bleibt ein Ende, dessen Griff am Rand steht, ungeblendet
    (`Blenden`, `schneideAus`) — sonst verlöre ein Schlag aus dem Kit beim
    Kürzen hinten seinen Anschlag vorn. Öffnen und Speichern ohne Änderung
    gibt dieselben Abtastwerte (bis auf das Schreiben in 16 Bit).
  - **Über einem geöffneten Sample aufgenommen:** bei einem eigenen bleibt es
    offen, Save schreibt die Aufnahme unter seinem Namen und fragt „Replace
    %1 with the new recording?"; bei Kit und Datei wird das Fenster wieder
    Record sample, ohne Namen.
  - **Save as... nimmt keinen vergebenen Namen** — der Dialog sagt „My
    samples already holds %1 — choose another name", sein Save ist grau. Der
    Dialog Save instrument fragt an dieser Stelle „Replace it?"; in der
    Vorlage zu M2b stand irrtümlich, er weigere sich.
  - **Folge für den Drum-Editor:** dort gibt es Record sample..., aber kein
    Edit.... Ein vorhandenes eigenes Sample lässt sich von dort aus nicht
    mehr ersetzen (früher über denselben Namen im Feld) — nur über Edit... im
    Creator (17b).
  - **Von Claude gesetzt, nicht vorgelegt:** die Blenden-Regel; die Sätze im
    Dialog Save sample as („Built-in sounds stay as they are. This saves a
    copy to My samples and uses it." und zwei Geschwister); die Tooltips an
    Save; in „Used by" höchstens drei Titel beim Namen; die Länge in der
    Zeile mit zwei Stellen.
  - Tests: `test_LoopAufnahmeFenster` umgestellt, ein Fall mit fünf Teilen
    neu (`LUMIVIZ_EDIT_SAMPLE_BILD` legt zwei Bilder ab);
    `test_LoopDrumEditor`, `test_LoopInstrumentEditor` umgestellt. **Im
    Programm:** Record... zeigt die neuen Knöpfe; Edit... gibt es noch nicht.
- **17b — der Creator (gebaut, S119):** der Knopf **Edit...** neben File...
  und Record... (`oeffneBearbeiten`), grau ohne geladenes Sample; er gibt dem
  Fenster das Sample der gewählten Schicht, Record... läuft über dasselbe
  Fenster. Nach Save oder Save as... spielt die Schicht das Ergebnis
  (`uebernimmSample`), **Sample start** rückt um das vorn Weggeschnittene
  zurück, nicht unter null. Ein Sample über 30 Sekunden öffnet Edit... nicht
  und meldet „This sample is longer than 30 s. Edit... works on samples up to
  30 s." (`bearbeitenHindernis`; in der Skizze „This file…" — es trifft auch
  eigene Samples).
  - **Von Claude gesetzt, über M3 a hinaus:** auch **Loop from** und **Loop
    to** rücken mit — sie sind Stellen im Sample wie Sample start; ist die
    Schleife ganz weggeschnitten, geht Sustain loop aus. Und: unter demselben
    Namen **neu aufgenommen** gehen Sample start und Schleife auf null wie
    bei jedem anderen neuen Sample (bisher blieben sie in diesem einen Fall
    stehen).
  - Mitgezogen wird nur, wenn die gewählte Schicht beim Speichern noch das
    Sample spielt, für das Edit... gedrückt wurde; sonst gilt das Ergebnis
    als anderer Klang und beginnt von vorn.
  - Tests: `test_LoopInstrumentEditor.cpp`, im Fall zur Sample-Schicht — der
    Knopf, Save mit Sample start und Schleife, Save as... als Kopie, ein
    Klang des Kits, Record... nach Edit..., über 30 Sekunden, ohne Sample.
    **Im Programm ungesehen.**
- **17c — Abschluss (gemacht, S119):** Benutzerhandbuch 1.148.0 (§16: Edit...
  in der Tabelle der Sample-Schicht, „Ein Sample bearbeiten" mit der Tabelle
  je Herkunft, Record sample ohne Namensfeld, der Drum-Editor); die Liste
  `Sichttest_Composer_10_Loop.md` (lokal): Abschnitt **D** (D0 bis D10), U72
  bis U75, R1, R6, R7 neu gefasst; voller Lauf 1153 / 1153, 0 Skips
  (Testing-MSVC). **Stufe 17 ist gebaut — im Programm ungesehen.**

## 4l. Mitgelieferte Samples und Kits (S123 — Entscheide 1 a, 2 a, 3 a, Patrik S123)

Anlass: Patriks Wunsch nach mehr Vorlagen — Samples aus freien Quellen (auch
Stimmen), Instrumente daraus, ein Drumkit aus zusätzlichen Drums, Loops.

| Entscheid | Inhalt |
|---|---|
| **1 a** | Mitgelieferte Samples liegen unter `asset/loop/samples/` und heißen im Loop **`lib:<name>`** — dritte Art neben `kit:` und `user:`. In der Klangliste des Drum-Editors und im Sample-Menü des Creators eine eigene Gruppe **Samples** zwischen Kit und My samples. Schreibgeschützt wie das Kit. Verworfen: b — die Samples unter `kit:` (Stimmen stünden zwischen Kick und Snare); c — keine mitgelieferten Samples. |
| **2 a** | Nur CC0, CC BY und gemeinfreie Aufnahmen mit direktem Download; je Datei Quelle, Urheber und Lizenz in `samples/CREDITS.md`, die mit neben die Exe geht. Vocals aus Freesound lädt Patrik selbst (Konto nötig). Verworfen: b — nur CC0. |
| **3 a** | Ein **Kit wird eine eigene Einheit** (mehrere Kits, je eine Datei mit Namen und Klängen; im Drum-Editor füllt die Wahl die Spuren); neue Drum-Klänge entstehen dafür. Skizze vorher. Verworfen: b — nur mehr einzelne Klänge. |

**Gebaut (S123):** `istBibliotheksKlang`, `istMitgelieferterKlang` (`loop/Loop.hpp`);
`bibliothekOrdner`, `bibliotheksKlaenge`, `klangDatei` kennt `lib:` (`loop/Kit.hpp`);
`sampleDatei` und `sampleTitel` kennen `lib:` (`loop/Sample.hpp`); die Datei
nennt ein mitgeliefertes Sample beim Namen, nie als Pfad; ein fehlendes steht
im Bericht als „Sample not found: lib:<name>". Die Gruppe **Samples** im
Drum-Editor (`DrumEditor::fuelleKlang`) und im Menü **File...** des Creators;
dort steht hinter dem Namen „· built-in". Das Fenster Edit sample behandelt
`lib:` wie das Kit (Herkunft Mitgeliefert: eine Kopie geht zu My samples).
Achtunddreißig Samples aus der OLPC-Sammlung (CC BY 3.0); Instrumente daraus
und die Loops: `asset/loop/README.md`, Abschnitt S123. Keine neue Fassung der
`.lvloop` oder `.lvinst`: `lib:` ist ein Wert in Feldern, die es gibt — ein
älteres LumiViz meldet das Sample als nicht gefunden.

### Die Skizze der Kits (gezeigt S123, noch nicht gebilligt)

Mit dem Zeichenwerkzeug gezeigt; hier als Beschreibung:

- **Drum-Editor, Kopfzeile:** rechts neben dem Namen des Musters ein Knopf
  **Kit: <Name> ▾**, daneben Add track.
- **Das Menü des Knopfs:** Abschnitt *Built-in kits* (Classic, 808, 909 House,
  Electro, Hand drums, Beatbox, Lo-fi; das gewählte mit Haken), Abschnitt *My
  kits*, Trennlinie, **Save as kit...**, **Edit kits...**.
- **Spuren:** links vor der Auswahl Sound ein Etikett **Role** (kick, snare,
  clap, hat, open hat …).
- **Zeile unter dem Raster:** „Switching the kit keeps the steps: each track
  takes the sound of its role from the new kit. Tracks without a match keep
  their sound."

### Die Entscheide zu den Kits (Patrik S123)

| Entscheid | Inhalt |
|---|---|
| **K1 a** | Jede Drum-Spur trägt eine **Rolle**. Bei alten Loops geraten aus dem Klang (`kit:kick`, `lib:snare_909`, `drum_ride`) und dem Namen der Spur; per Klick auf das Etikett änderbar. Ein Kit nennt je Rolle einen Klang. Verworfen: der Reihe nach (b), alle Spuren ersetzen (c). |
| **K2 a** | Eine Spur, deren Rolle das Kit nicht hat, behält ihren Klang (Etikett „not in kit"). Rollen des Kits ohne Spur werden nicht angelegt, stehen aber oben in **Add track** („Clap — from 808"). Verworfen: fehlende Rollen gleich anhängen (b), Spuren stummschalten (c). |
| **K3 a** | Datei **`.lvkit`** (JSON: Titel, Beschreibung, Rollen); je Rolle ein Klang jeder Art, die eine Spur kennt, dazu Volume und Pan; ein Wechsel übernimmt alle drei. Mitgeliefert unter `asset/loop/kits`, eigene im Benutzerordner unter `kits` (mit Export und Import). Verworfen: nur Klänge (b), Kit als Muster in einer `.lvloop` (c). |
| **K4 a** | Ein Drum-Muster trägt das freiwillige Feld `kit` (ohne neue Fassung der `.lvloop`); der Knopf zeigt „Kit: 808", nach einer Änderung von Hand „Kit: 808 (changed)", ohne Feld „Kit ▾". Verworfen: kein Gedächtnis (b). |
| **K5 a** | Feste Rollen, nummeriert mehrfach: kick, snare, clap, rim, hat, open hat, ride, crash, tom (tom 1, tom 2 …), perc (perc 1, perc 2 …), fx. Fehlt die Nummer im neuen Kit, nimmt die Spur die höchste vorhandene derselben Rolle, sonst K2 a. Verworfen: jede Rolle einmal (b), freie Namen (c). |
| **K6 a, K6b a, K6c a** | Vierzehn mitgelieferte Kits. Sofort aus vorhandenen Klängen: Classic, 808, 909 House, Electro, Hand drums, Beatbox, Lo-fi, Reggae, Reggaeton, Strange (Inhalt je Rolle: Tabelle unten). Im zweiten Schritt aus akustischen Aufnahmen (CC BY 4.0, DrumGizmo; Downloads vorher zur Freigabe): Rock'n'Roll und Hardrock aus dem **DRSKit**, Heavy Metal aus dem **CrocellKit**. **Jazz Brushes**: Kick, Ride, Hi-Hat leise aus dem DRSKit, die Besen gerechnet (`drum_brush_sweep`, `drum_brush_tap`, `drum_brush_slap`); ein Paket mit Besen-Aufnahmen kann sie später ersetzen, wenn seine Lizenz CC0 oder CC BY ist. Dazu ein Loop „Brush Swing". Inhalt: Tabelle unten. |
| **K7 a** | **Edit kits...** öffnet das Fenster **Kits**. |
| **K8 a** | Die Skizze des Fensters Kits ist gebilligt (unten). |

### Der Inhalt der mitgelieferten Kits (K6 a, K6b a, K6c a)

Volume nach Messung angeglichen (gleiche Lautheit je Rolle über die Kits);
Pan: Hats leicht rechts, Toms und Percussion verteilt, sonst Mitte. Nach
Rechnung gesetzt, nicht gehört.

| Kit | kick | snare | clap / rim | hat | open hat | weitere |
|---|---|---|---|---|---|---|
| Classic | `kit:kick` | `kit:snare` | clap `kit:clap` | `kit:hat_closed` | `kit:hat_open` | tom 1 `kit:tom` |
| 808 | `drum_kick_808` | `lib:snare_808` | clap `drum_clap` | `lib:hat_808_closed` | `drum_hat_open` | tom 1 `drum_tom_low`, tom 2 `drum_tom_high`, crash `drum_crash` |
| 909 House | `lib:kick_909` | `lib:snare_909` | clap `lib:clap_707` | `drum_hat_closed` | `lib:hat_707_open` | ride `drum_ride`, crash `drum_crash`, perc 1 `lib:tambourine` |
| Electro | `drum_kick_punch` | `drum_snare_fat` | clap `drum_clap_stack` | `drum_hat_shake` | `drum_hat_open` | tom 1 `drum_tom_low`, tom 2 `drum_tom_high`, crash `drum_crash` |
| Hand drums | `lib:djembe_low` | `lib:djembe_slap` | — | `drum_shaker` | `lib:tambourine` | perc 1 `lib:conga_high`, perc 2 `lib:darbuka_doom`, perc 3 `lib:darbuka_tak` |
| Beatbox | `drum_beatbox_kick` | `lib:beatbox_snare` | — | `lib:beatbox_hat_closed` | `lib:beatbox_hat_open` | fx `lib:voice_male_hey` |
| Lo-fi | `lib:kick_deep` | `lib:snare_808` | rim `lib:rim`, clap `lib:snap` | `lib:hat_808_closed` | `drum_hat_open` | perc 1 `lib:tambourine` |
| Reggae | `drum_kick` | `kit:snare` | rim `lib:rim` | `lib:hat_808_closed` | `drum_hat_open` | perc 1 `lib:tambourine`, tom 1 `drum_tom_high` |
| Reggaeton | `drum_kick_808` | `lib:snare_808` | clap `lib:clap_707` | `drum_hat_closed` | `lib:hat_707_open` | perc 1 `lib:conga_high`, perc 2 `lib:rim` |
| Strange | `drum_beatbox_kick` | `lib:snap` | — | `lib:beatbox_hat_closed` | `drum_crash` (kurz) | perc 1 `shape_cat`, perc 2 `lib:voice_female_gliss`, fx `lib:voice_male_hey`, tom 1 `shape_atom_bell` (tief) |
| Rock'n'Roll | DRSKit | DRSKit | rim DRSKit (Cross-Stick) | DRSKit | DRSKit | tom 1–3, ride, crash DRSKit |
| Hardrock | DRSKit, volle Stärke | DRSKit, volle Stärke | — | DRSKit | DRSKit | tom 1–3, ride, crash 1–2 DRSKit |
| Heavy Metal | CrocellKit (Doppelpedal: kick 1, kick 2) | CrocellKit | — | CrocellKit | CrocellKit | tom 1–4, ride, crash 1–3, fx China, perc Splash |
| Jazz Brushes | DRSKit leise | `drum_brush_tap`, Akzent `drum_brush_slap` | — | DRSKit (Fuß) | — | perc 1 `drum_brush_sweep`, ride DRSKit leise |

Im Hand-drums-Kit tragen kick und snare den tiefsten und den hellsten
Schlag, damit ein Beat beim Wechsel seinen Puls behält; im Reggae-Kit stehen
Kick und Rim zusammen auf der Drei (One Drop).

### Wie 21a gebaut ist (S123)

- **`loop/DrumKit.hpp`:** `rollenArten`, `leseRolle` und `rollenText` (tom und
  perc immer mit Nummer, die anderen ab der zweiten); `rateRollenArt` sucht in
  den Wörtern von Instrument, Klang, dann Name (von Claude gesetzt: Wortlisten
  wie „hat, hh, hihat", „shaker, conga, djembe, …"; open + hat ist open hat;
  snap ist clap); `rateRollen` nummeriert je Art weiter, Toms von tief nach
  hoch (low, floor vor high). `DrumKit::fuer` folgt K5 a. `wendeKitAn` rät
  erst, setzt dann Klang, Instrument mit Note und Length, Volume und Pan; ein
  Kit ohne Klang neben einem Instrument lässt den alten Klang der Spur stehen
  (für ein älteres Programm). `weichtAb` vergleicht genau diese Felder.
- **Die Datei `.lvkit`:** `format` `lumiviz-kit`, `version` 1, `title`,
  `description`, `roles` (je `role`, `sound`, bei einem Instrument
  `instrument`, `note`, `length`, dazu `volume`, `pan`); Pfade wie in der
  `.lvloop` relativ zum Ordner der Datei; unbekannte und doppelte Rollen fallen
  beim Lesen weg.
- **Die `.lvloop`:** `role` an der Spur und `kit` am Muster, beide nur, wenn
  gesetzt — keine neue Fassung. Geraten wird nicht beim Laden, sondern erst
  beim Kitwechsel: eine alte Datei liest sich gleich wie vorher.
- **Volume der zehn Kits:** gemessen (Effektivwert der ersten 300 ms bei
  Volume 1, `test_LoopKits.cpp` mit `LUMIVIZ_KIT_PEGEL=1`), auf ein Ziel je
  Rolle gebracht (`ZIEL_DB` in `make_kits.py`, von Claude gesetzt); danach
  liegen die Kits je Rolle auf 0,5 dB beieinander, ausgenommen der Snap als
  Snare in Strange (4,9 dB darunter — er ist zu leise für das Ziel). Die
  Reihenfolge im Menü ist die nach Titel.

### Wie 21b gebaut ist (S123)

- **Kopf des Drum-Editors:** rechts der Knopf **Kit** (`drumKit`, ein Knopf
  mit Menü — ein Werkzeugknopf legte seinen Pfeil über den Text); er zeigt
  „Kit", „Kit: <Titel>", „Kit: <Titel> (changed)" oder „Kit: <Nennung> (not
  found)". Das Menü entsteht beim Aufklappen: *Built-in kits* und *My kits*
  (das gewählte mit Haken, die Beschreibung als Tooltip), **Save as kit...**
  (fragt nach dem Namen; ist er vergeben, ob ersetzt wird), **Edit kits...**
  grau bis 21c.
- **Die Zeile darunter** (`drumKitMessage`) sagt nach einem Wechsel, wie viele
  Spuren einen Klang bekamen, welche Rollen das Kit nicht hat und was Add track
  anbietet; nach Save as kit, wie viele Rollen gespeichert sind.
- **Spalte Role** zwischen Track und Sound: gesetzt als Text, geraten kursiv in
  Klammern („(kick)"), keine als „—"; fehlt die Rolle im Kit, durchgestrichen
  mit Tooltip (K2 a). Ein Klick öffnet die Rollen — je Art eine Nummer mehr als
  vergeben (tom und perc immer), darunter **No role**.
- **Add track:** hat das Kit Rollen ohne Spur, ein Menü „Clap — from 808" …,
  darunter **Empty track**; sonst wie bisher.
- Jede Änderung läuft als ein Schritt über Undo („Kit: 808", „Change role",
  „Add track", „Save as kit"). Von Claude gesetzt: die Wortlaute, die Breite
  der Spalte (64), dass ein erneuter Klick auf dasselbe Kit nur die Zeile neu
  setzt, wenn sich nichts ändert.

### Das Fenster Kits (Skizze gebilligt, Patrik S123, K8 a)

Mit dem Zeichenwerkzeug gezeigt; hier als Beschreibung:

- **Kopf:** „Kits", rechts Undo und Redo.
- **Links die Liste:** Abschnitt *Built-in kits* (die vierzehn), Abschnitt *My
  kits*; darunter **Duplicate**, **Rename**, **Delete**. Ein mitgeliefertes
  Kit trägt ein Schloss.
- **Rechts das gewählte Kit:** Titel; bei einem mitgelieferten der Satz
  „Built-in, read-only — Duplicate makes your own copy". Eine Tabelle, je
  Zeile **Role**, **Sound** (dieselbe Auswahl wie im Drum-Editor, mit Samples,
  Instrument..., Sample file...), **Volume**, **Pan**, ▶ zum Anhören.
  Darunter **Add role ▾** (die Rollen aus K5 a, nummeriert selbst weiter),
  **Remove role**, rechts **Play all** (die Rollen nacheinander).
- **Fuß:** „Used by: …" — die Loops, deren Muster dieses Kit tragen (Feld
  `kit`, K4 a), soweit LumiViz sie kennt (die mitgelieferten und die zuletzt
  geöffneten; die Zeile sagt das); **Close**, **Save**.
- **Anmerkung Patrik zur Skizze:** die Texte in der Auswahl Sound waren nicht
  ganz zu sehen. Gebaut wird: die Spalte Sound nimmt den freien Platz, die
  aufgeklappte Liste ist so breit wie ihr längster Eintrag, ein gekürzter
  Name im geschlossenen Feld endet mit „…" und steht ganz im Tooltip.

### Wie 21c gebaut ist (S124)

- **`loop/KitFenster`** nach der Skizze; geöffnet mit **Edit kits...** im
  Menü des Knopfs Kit (`DrumEditor::oeffneKits`), auf dem Kit des Musters,
  sonst auf dem ersten der Liste. Die Liste: *Built-in kits* mit Schloss
  (selbst gezeichnet, in der Farbe des grauen Texts), *My kits*.
- **Mitgeliefert:** alles grau außer ▶, Play all und Duplicate; der Satz
  „Built-in, read-only — Duplicate makes your own copy".
- **Duplicate** schreibt sofort ein eigenes Kit „<Titel> copy" („copy 2", …)
  aus dem Stand, wie er dasteht, und öffnet es. **Rename** ändert nur den
  Titel und schreibt ihn gleich; Datei und Nennung (`user:<name>`) bleiben,
  Loops finden das Kit weiter. **Delete** fragt nach und löscht die Datei —
  ohne Undo; ein Loop, der das Kit nennt, zeigt danach „(not found)".
- **Undo/Redo** (Kopf, Ctrl+Z, Ctrl+Y, auch im Fenster Instruments, das
  Instrument... von hier öffnet) gelten den Änderungen am offenen Kit:
  Sound, Volume, Pan, Add role, Remove role. Ein anderes Kit wählen leert sie;
  nach Save bleiben sie. Wählen oder Schließen mit ungesicherten Änderungen
  fragt Save, Discard, Cancel.
- **Die Auswahl Sound** ist ein Bauteil (`loop/KlangAuswahl`), das der
  Drum-Editor mit benutzt: Kit, Samples, My samples, File, Instrument...,
  Sample file...; im Fenster Kits ohne Record sample... (die Skizze nennt
  es nicht). Ein zu langer Name endet mit „…" und steht ganz im Tooltip; die
  Liste ist so breit wie ihr längster Eintrag — jetzt auch im Drum-Editor.
- **Add role ▾** (Knopf mit Menü wie der Knopf Kit) bietet je Art die Nummer
  hinter der höchsten vergebenen an („kick 2", „tom 3", „ride"); die neue
  Rolle kommt ohne Klang an ihren Platz in der Reihenfolge der Arten.
- **▶ und Play all** bitten das Loop-Panel (`LoopPanel::spieleSchlaege`): je
  Rolle ein Schlag mit ihrem Volume und Pan, einer je Schlag des offenen
  Loops; eine Rolle ohne Klang spielt nicht mit. `spieleSpur` läuft seither
  über dieselbe Funktion.
- **Used by** zählt die mitgelieferten Loops, die zehn zuletzt geöffneten
  (neu: `merkeGeoeffnetenLoop`, das Panel merkt beim Laden und Speichern) und
  den offenen („the open loop"); gerechnet beim Öffnen eines Kits.
- Nach Save, Rename, Duplicate und Delete zeigt der Drum-Editor Knopf Kit und
  Etiketten neu — „(changed)" misst gegen das Kit, wie es jetzt in der Datei
  steht.
- Von Claude gesetzt (nicht vorgelegt): die Wortlaute; der Titel einer Kopie;
  Rename nur als Titel; Delete ohne Undo; Undo nur für das offene Kit; ein
  Schlag je Schlag des Loops bei Play all; zehn zuletzt geöffnete Loops; die
  Beschreibung des Kits nur als Tooltip am Titel; das Fenster Instruments des
  Fensters Kits ist ein eigenes (nicht das des Drum-Editors).
- Gesehen am Bild aus dem Test: in der gewählten Zeile scheinen an den
  Rändern der Zellen schmale Striche der Auswahlfarbe durch — kosmetisch,
  offen.

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
| 11 | **Instrumente aus Samples:** eine Tondatei, je Note in der Tonhöhe umgerechnet (I1 c); als Art von Schicht, mit Root note, Fine tune und Detect (§4h) | Skizze gebilligt (S115), S1 a, S2 a; **11a Schritt 1 gebaut** (der Kern ohne Fenster, `.lvinst` Fassung 4), **Schritt 2 gebaut** (der Creator: Waveform Sample, File..., das Bild der Aufnahme) — im Programm noch nicht gesehen und nicht gehört, **Schritt 3 gebaut** (Detect, Reference tone) — **11a ist gebaut**, Liste 10 Abschnitt Q; **11b Schritt 1 gebaut** (aufnehmen über die Engine, zuschneiden, WAV — ohne Fenster), **Schritt 2 gebaut** (das Fenster Record sample), **Schritt 3 gebaut** (Record... im Creator) — **11b ist gebaut**, im Programm noch nicht gesehen und nicht gehört, Liste 10 Abschnitt R; offen 11b Record sample, 11c Drum-Editor (Entscheid davor); Multisamples vorgemerkt — vorziehen, wenn Tests an Sample-Instrumenten es verlangen |
| 12 | **Effekte am Clip** (E2 b): Reverb, Echo, Distortion je Clip, für Drumkit- und Instrument-Clips; mehrere Clips markieren; der Rechenkern bekommt dafür ein Gedächtnis | S113: Skizze gebilligt (§4g), Format entschieden (F2 a). **12a gebaut:** Modell und Datei in Fassung 3 (§4), ein Fall mit sechs Teilen in `test_LoopKern.cpp`; nichts davon zu hören oder zu sehen. **12b Teil 1 gebaut:** die Rechenbausteine `loop/Effekte` und die Messung ihrer Last (G1 b, §4g), `test_LoopEffekte.cpp`, sieben Fälle ohne Ton; **12b Teil 2 gebaut:** Busse im Rechenkern, die Ketten im Klangerzeuger mit Sprung-Regel und Übergabe des Nachklangs (§4g), fünf Fälle dazu in `test_LoopEffekte.cpp`, das Beispiel `asset/loop/effects_demo.lvloop` mit einem Fall in `test_LoopInstrument.cpp`; **zu hören, aber nicht gehört** — einstellen lässt es sich noch nur in der Datei. **12c gebaut:** mehrere Clips markieren, als Gruppe ziehen, kopieren, löschen, die Marke fx (§4g), zwei Fälle in `test_LoopArrangement.cpp`; **im Programm nicht gesehen**. **12d gebaut:** das Fenster Clip effects und Effects... im Menü (§4g), `test_LoopClipEffekte.cpp`, zwei Fälle; **im Programm nicht gesehen und nicht gehört**. **12e gemacht:** Benutzerhandbuch 1.122.0 (§16: Effekte am Clip, mehrere Clips markieren, das Beispiel), die Liste `Sichttest_Composer_10_Loop.md` (lokal) um Abschnitt F (F0 bis F17) und U25 bis U31; voller Lauf 1061 / 1061, 0 Skips (Testing-MSVC). **Stufe 12 ist gebaut.** Im Programm: Patrik hat F0 bis F4 der Liste 10 geprüft (S113) — „sind alle ok": die Marke fx, der Loop mit und ohne Effekte gehört, das Fenster Clip effects, ein Regler im Lauf. Im Gespräch gemeldet, ohne Testlog; mehr als dieses Wort ist nicht belegt. **F5 bis F17 und die Urteile U25 bis U31 sind offen.** Offen aus der Stufe außerdem: die Last im Ton-Thread mit vielen Clips; der Loop-Clip mit Effekten im Composer ohne eigenen Testfall. |
| 13 | **Mehrfachauswahl in Drum- und Instrument-Editor:** mehrere Schritte oder Noten markieren, kopieren, einfügen, verschieben (§4i) | S113: Skizze gebilligt (§4i), Entscheide V1 a, V2 a, V3 a, dazu das Drehen im Kreis mit Alt+Pfeil. **13a gebaut:** die Regeln als reine Funktionen (`loop/Auswahl`), `test_LoopAuswahl.cpp`, fünf Fälle; **13b gebaut:** im Drum-Editor markieren (Shift+Ziehen, Ctrl+Klick, Ctrl+A), Ctrl+C / X / V / D, Entf, Pfeiltasten, Alt+Pfeil, die Einfügemarke, das Menü, die Hinweiszeile (§4i); ein Fall mit sieben Teilen in `test_LoopDrumEditor.cpp`; **im Programm nicht gesehen**. **13c gebaut:** dasselbe im Instrument-Editor, dazu das Ziehen mehrerer Noten als Gruppe (§4i); ein Fall mit sechs Teilen in `test_LoopInstrumentEditor.cpp`; **im Programm nicht gesehen**. **13d gemacht:** der Haken „Right-click deletes" in beiden Editoren (Wunsch Patrik, §4i); Benutzerhandbuch 1.123.0 (§16: mehrere Schritte und Noten zugleich); die Liste `Sichttest_Composer_10_Loop.md` (lokal) um Abschnitt M (M0 bis M16) und U32 bis U36; voller Lauf 1068 / 1068, 0 Skips (Testing-MSVC) im zweiten Anlauf — der erste brach im 267. Fall ab, ohne fehlgeschlagene Prüfung (nach der Reihenfolge der Liste ein Composer-Fall „S102 Zeitachse", die ein echtes Fenster öffnen; einzeln fünfmal grün; Ursache nicht untersucht). **Stufe 13 ist gebaut — im Programm nicht gesehen.** |
| 14 | **Palette der Instrumente** (P1 a): Fenster Instruments nach Art, mit Suche, Genre, Anspielen (§4h) | Skizze gebilligt (S114), F3 b, P2 a; **14a gebaut** (Modell, Fassung 3, die 49 Dateien, eigene Instrumente), **14b gebaut** (Creator: Type, Genres, Description, Save als eigene Kopie), **14c gebaut** (das Fenster, Browse...); **im Programm noch nicht gesehen und nicht gehört** — Liste 10, Abschnitt P |
| 15 | **Wave-Clips** (Tonspuren wie Gesang) und **Autotune** am Clip (§3.2) | später |
| 16 | **Signal path:** Filter und Effekte als Reihe von Bausteinen, je Schicht im Creator und je Clip (§4j) | S117: Skizzen gezeigt, Entscheide K1 a bis K4 a; S118: Baustufen freigegeben (B1 a), **16a gebaut** (Modell und Dateien, `.lvinst` 7, `.lvloop` 5), **16b gebaut** (der Klangerzeuger rechnet die Reihe; alte Dateien Bit für Bit gleich), **16c gebaut** (der Creator zeigt die Zeile Signal path; Skizze gebilligt, S1 a), **16d gebaut** (Clip effects zeigt die Reihe, Filter als vierter Baustein; K5 a), **16e gebaut** (Kick mit Drive hinter der Envelope, Clap mit zwei Filtern, neu gegen das Kit gemessen); **Stufe 16 fertig** — im Programm ungesehen und ungehört; offen die Hi-Hat-Schwingung, nach Patriks Ohr |
| 17 | **Ein Sample bearbeiten:** Edit... an einer Sample-Schicht öffnet das Sample im Fenster der Aufnahme; Save und Save as... statt Namensfeld; Kit und freie Dateien schreibgeschützt (§4k) | S119: Skizze gebilligt, Entscheide M1 a, M2 b, M2b a, M3 a, M4 a; Baustufen freigegeben (B2 a); **17a gebaut** (das Fenster: `bearbeite`, Save und Save as..., die Rückfragen; ein Fall mit fünf Teilen in `test_LoopAufnahmeFenster.cpp`) ; **17b gebaut** (der Knopf Edit... im Creator, Sample start und Schleife rücken mit, die Grenze von 30 Sekunden; im Fall zur Sample-Schicht in `test_LoopInstrumentEditor.cpp`) ; **17c gemacht** (Benutzerhandbuch 1.148.0, Liste 10 Abschnitt D, voller Lauf 1153 / 1153); **Stufe 17 fertig — im Programm ungesehen** |
| 21 | **Kits als eigene Einheit** (§4l): Rollen, `.lvkit`, Knopf Kit im Drum-Editor, Fenster Kits, vierzehn mitgelieferte Kits | S123: Entscheide K1 a bis K9 a, Skizzen gebilligt (Drum-Editor gezeigt, Fenster Kits K8 a); **21a gebaut** (`loop/DrumKit`: Rollen lesen, schreiben, raten, nummerieren; `.lvkit` Fassung 1; Ablage mitgeliefert und eigen; Kitwechsel nach K2 a und K5 a; `weichtAb`; `kitAusMuster`; in der `.lvloop` die freiwilligen Felder `role` und `kit`; Bereich Kits in Export und Import; zehn Kits aus `asset/loop/kits/make_kits.py`, Volume nach Messung; `test_LoopKits.cpp`, sieben Fälle) ; **21b gebaut** (Drum-Editor: Knopf Kit mit Menü, „(changed)", Zeile mit dem Ergebnis, Etikett Role je Spur, Add track mit den Rollen des Kits, Save as kit...; ein Fall in `test_LoopDrumEditor.cpp`, `LUMIVIZ_DRUM_KIT_BILD`) ; **21c gebaut** (S124: das Fenster Kits, `loop/KitFenster`; die Auswahl Sound als Bauteil `loop/KlangAuswahl`; Edit kits... im Drum-Editor; `LoopPanel::spieleSchlaege`; die zuletzt geöffneten Loops; `test_LoopKitFenster.cpp`, drei Fälle, `LUMIVIZ_KIT_FENSTER_BILD`; im Programm ungesehen) ; offen 21d (akustische Kits), 21e (Icon, Handbuch, Sichttest) |
| 18 | **Pan je Schicht und acht Schichten je Instrument** — die erste Stufe aus [Shape_Konzept.md](Shape_Konzept.md) §6a und §14 (Klang aus einer Form; die Stufen 19 und 20 stehen dort) | S120: Entscheid Z3 a (S119) und „Grenze auf acht" (Patrik S120, davor gemessen); **gebaut:** `Schicht::pan` mit dem Gesetz von Spur und Muster (links `min(1, 1 − pan)`, rechts `min(1, 1 + pan)`), `kSchichtenMax` 8, `.lvinst` Fassung 8; im Creator der Regler **Pan** im Kasten Level, der Kasten nennt ihn neben der Mitte; alte Instrumente rechnen Bit für Bit wie vorher (`test_LoopWerte` unverändert grün); ein neuer Fall in `test_LoopInstrument.cpp`, Ergänzungen in `test_LoopInstrumentEditor.cpp`; Benutzerhandbuch 1.153.0, Liste 10 Abschnitt V; **im Programm ungesehen und ungehört** |

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

- **0.72.0** (S124, 2026-10-14): §4l — Stufe 21c gebaut: das Fenster Kits
  („Wie 21c gebaut ist"), die Auswahl Sound als gemeinsames Bauteil.
- **0.71.0** (S123, 2026-10-14): §4l NEU — mitgelieferte Samples `lib:`
  (Entscheide 1 a, 2 a) gebaut; Kits (3 a) mit der gezeigten Skizze als
  Beschreibung, offen.
- **0.70.0** (S120, 2026-10-13): §5 — Stufe 18 gebaut: Pan je Schicht und
  acht Schichten je Instrument, `.lvinst` Fassung 8. Die Stufe kommt aus
  `Shape_Konzept.md` (Klang aus einer Form), dort stehen auch die Stufen 19
  und 20. Wo dieses Dokument weiter oben von höchstens vier Schichten und von
  Fassung 7 spricht, gilt der Stand vor Stufe 18.
- **0.69.0** (S119, 2026-10-13): §4j — Abweichung im Kasten der Zeile Signal
  path (Clip effects, mehrere Clips): „≠", die bernsteinfarben getönte
  Fläche, der Tooltip mit den Reglern (Befund Patrik zu X5, Entscheid X5 a;
  die Fläche statt des Rands nach seinem Vorschlag)
- **0.68.0** (S119, 2026-10-13): §4k — **Stufe 17c gemacht, Stufe 17
  fertig:** Handbuch, Liste 10, voller Lauf; §5 Stufe 17
- **0.67.0** (S119, 2026-10-13): §4k — **Stufe 17b gebaut:** Edit... im
  Creator, Sample start und Schleife rücken mit, die Grenze von 30 Sekunden;
  §5 Stufe 17
- **0.66.0** (S119, 2026-10-13): §4k — Baustufen freigegeben (B2 a);
  **Stufe 17a gebaut:** das Fenster öffnet ein bestehendes Sample, Save und
  Save as..., geblendet wird nur, wo geschnitten ist; die Folge für den
  Drum-Editor; §5 Stufe 17
- **0.65.0** (S119, 2026-10-13): **§4k neu** — ein Sample bearbeiten (Edit...
  im Creator): Entscheide Patrik M1 a, M2 b, M2b a, M3 a, M4 a, die gebilligte
  Skizze als Beschreibung, Baustufen 17a bis 17c vorgelegt; §5 Stufe 17;
  nichts gebaut
- **0.64.0** (S118, 2026-10-13): §4j — **Stufe 16e gebaut, Stufe 16 fertig:**
  Kick mit Drive hinter der Envelope, Clap mit Hochpass und Bandpass, die
  Tabelle aus 11f mit den neuen Zahlen daneben; §4h Verweis; §5 Stufe 16
- **0.63.0** (S118, 2026-10-13): §4j — Entscheid Patrik K5 a (mehrere Clips
  mit verschiedenen Reihen: einen zeigen, die mit derselben Folge
  mitstellen), die Skizze als Beschreibung; **Stufe 16d gebaut:** das Fenster
  Clip effects mit der Zeile Signal path, Filter als vierter Baustein,
  Showing und Use this path for all; §5 Stufe 16
- **0.62.0** (S118, 2026-10-13): §4j — Skizze des Creators gebilligt
  (Entscheid Patrik S1 a); **Stufe 16c gebaut:** die Zeile Signal path im
  Creator (`SignalWeg`), die Regler folgen dem gewählten Kasten, Add, Move,
  Remove, Ziehen; die Gruppen Envelope und Filter entfallen; §5 Stufe 16
- **0.61.0** (S118, 2026-10-13): §4j — **Stufe 16b gebaut:** der
  Klangerzeuger rechnet die Reihe an Schicht und Clip, der Filter am Clip in
  Hertz; Beleg an 199 festgehaltenen Einträgen, Bit für Bit gleich unter
  beiden Compilern; die Messung der Last; §5 Stufe 16
- **0.60.0** (S118, 2026-10-13): §4j — Baustufen freigegeben (Entscheid
  Patrik B1 a); **Stufe 16a gebaut:** die Reihe von Bausteinen im Modell von
  Schicht und Clip, `.lvinst` Fassung 7, `.lvloop` Fassung 5, der Übergang
  bis 16b bis 16d; §5 Stufe 16
- **0.59.0** (S117, 2026-10-13): neu §4j **Signal path** — Filter und Effekte
  als Reihe von Bausteinen: Befund zum Unterbau der Effect Chain, die zwei
  Skizzen als Beschreibung, Entscheide K1 a bis K4 a, die Baustufen 16a bis
  16e (vorgelegt, nicht freigegeben), offen K5; §5 Stufe 16
- **0.58.0** (S117, 2026-10-12): §4h — **Stufe 11f gebaut:** sechs gerechnete
  Drum-Instrumente unter Drums, für die Note C2, gegen das Kit gemessen (die
  Tabelle), leiser als das Kit; nicht gehört
- **0.57.0** (S117, 2026-10-12): §4h — **Load instrument...** fragt, ob die
  Datei zu den eigenen kopiert wird oder am Ort bleibt (E3 a); die drei
  losen Dateien liegen unter `instruments` (E2 a)
- **0.56.0** (S117, 2026-10-12): §4h GEÄNDERT — **Save as...** im Creator
  fragt nur nach dem Namen und schreibt nach `instruments` (E1 a); ein
  mitgeliefertes Instrument wird nie überschrieben, der Dialog sagt es
  (E1b c); der Schalter „Replace the built-in one" ist nicht mehr vorgemerkt

- **0.55.0** (S116, 2026-10-12): im Creator **Repeat note** mit der Auswahl
  every 1 / 2 / 4 / 8 / 16 beats — der angeklickte Ton spielt immer wieder
  und darf bis zum nächsten klingen, die Taste ist dabei hervorgehoben
  (Wunsch Patrik, §4h).

- **0.54.1** (S116, 2026-10-12): Befund Patrik behoben — der Creator wuchs
  mit jeder aufgeklappten Gruppe über den Bildschirm; die Regler der Schicht
  stehen jetzt in einem Feld, das rollt (§4h).

- **0.54.0** (S116, 2026-10-12): Stufe 11e gebaut — der Filter mit Type
  (Low-pass, High-pass, Band-pass) und Resonance (Skizze gebilligt; Entscheid
  H1 b), `.lvinst` Fassung 6; im Creator lassen sich die Gruppen ein- und
  ausklappen (Wunsch Patrik); Hörproben lokal (§4h).

- **0.53.0** (S116, 2026-10-12): Stufe 11d gebaut — die Schwingung Noise,
  Pitch sweep und Sweep time (Skizze gebilligt; Entscheide N1: beide
  Richtungen, N2 a: auch für Sample), `.lvinst` Fassung 5; Hörproben lokal
  (§4h).

- **0.52.0** (S116, 2026-10-12): 11c Schritt 3 gebaut, 11c damit gebaut —
  Edit sound... im Menü der Spur (aus Kit-Klang, Sample oder Datei wird ein
  Instrument der Art Drums mit einer Sample-Schicht; nichts ändert sich vor
  Save; danach Note auf dem Root note, Length so lang wie die Aufnahme); das
  Fenster Instruments stellt sich aus einer Drum-Spur auf Drums — zu sehen
  erst mit 11f (§4h).

- **0.51.0** (S116, 2026-10-12): 11c Schritt 2 gebaut — die Klangliste im
  Drum-Editor (Kit, My samples, Instrument..., Sample file..., Record
  sample...) und der Knopf „…" mit Note, Length, Play, Browse..., Edit
  instrument...; die Frage aus S2 a steht einmal für Creator und Drum-Editor;
  davor: im Fenster Instruments der Knopf „★" und „★ Favorites" (§4h).

- **0.50.0** (S115, 2026-10-12): 11c Schritt 1 gebaut — die Spur mit drei
  Arten von Klang (Kit, eigenes Sample, Instrument mit Note und Length),
  `.lvloop` Fassung 4, `kit:` in der Sample-Schicht, die Art Drums; GEÄNDERT:
  eine Sample-Schicht spielt die Aufnahme in ihrer Lautstärke; vorgemerkt:
  Filter und Effekte als Kette (§4h).

- **0.49.0** (S115, 2026-10-12): Entscheid K3 a (Art „Drums"), die Skizze
  der Klangliste gebilligt; vorgemerkt: zweiter Filter je Schicht, Drive
  hinter der Hüllkurve, eine Schwingung aus sechs Rechtecken (§4h).

- **0.48.0** (S115, 2026-10-12): Entscheide K1 a (Drum-Spur mit drei
  Quellen und Edit sound..., dazu 11d, 11e, 11f) und K2 a (Note und Length in
  Millisekunden); Rechenversuch, ob das Kit im Creator baubar ist — Tom und
  Snare ja, für Kick, Clap und Hi-Hats fehlt je ein weiterer Baustein (§4h).
  Nichts gebaut.

- **0.47.0** (S115, 2026-10-12): 11b Schritt 3 gebaut — Record... im
  Creator öffnet das Fenster „Record sample", Save and use setzt die Aufnahme
  in die Sample-Schicht; 11b ist gebaut; voller Testlauf 1128 / 1128 (§4h).

- **0.46.0** (S115, 2026-10-12): 11b Schritt 2 gebaut — das Fenster
  „Record sample" (`loop/AufnahmeFenster`): aufnehmen, zuschneiden, Play mit
  Repeat, Save and use; noch öffnet es niemand (§4h).

- **0.45.0** (S115, 2026-10-12): 11b Schritt 1 gebaut — die Aufnahme in
  `IAudioEngine` und `BassEngine`, neu `loop/Aufnahme` (Stille an den
  Rändern, ausschneiden, Normalize, WAV, speichern unter den eigenen
  Samples) (§4h).

- **0.44.0** (S115, 2026-10-12): Beleg vor 11b — BASS nimmt neben der
  laufenden Engine auf, 48000 Hz Stereo float, Blöcke von 100 ms; drei
  Eingänge, darunter der Mitschnitt der Ausgabe (§4h).

- **0.43.0** (S115, 2026-10-12): 11a Schritt 3 gebaut — Detect (neu
  `loop/Grundton`) und Reference tone; 11a ist damit gebaut (§4h).

- **0.42.0** (S115, 2026-10-12): 11a Schritt 2 gebaut — der Creator mit
  Waveform: Sample, File..., der Frage aus S2 a, Root note, Fine tune,
  Sample start, Sustain loop und dem Bild der Aufnahme (§4h).

- **0.41.0** (S115, 2026-10-12): 11a Schritt 1 gebaut — der Kern ohne
  Fenster: `loop/Sample`, Sample als Schicht im Rechnen eines Tons, `.lvinst`
  Fassung 4, Bereich „Samples" im Export; Skizze „Record sample" gebilligt,
  der Teil zum Drum-Editor offen (§4h).

- **0.40.0** (S115, 2026-10-12): Stufe 11 — Skizze des Creators mit einer
  Sample-Schicht gebilligt und beschrieben, Entscheide S1 a (Ordner
  `samples`, `user:<name>`) und S2 a (File... fragt: kopieren oder am Ort
  lassen); Idee „Record sample" mit gezeigter Skizze notiert (§4h). Nichts
  gebaut.

- **0.39.0** (S115, 2026-10-12): Befund aus dem Programm (P5) und Entscheid
  Z1 a — Ctrl+Z und Ctrl+Y gelten im Fenster „Instruments" selbst (§4h).

- **0.38.0** (S114, 2026-10-12): 14c gebaut — das Fenster „Instruments" und
  Browse... im Instrument-Editor (§4h); voller Testlauf 1105 / 1105; Stufe 14
  ist gebaut.

- **0.37.0** (S114, 2026-10-12): 14b gebaut — im Creator die Zeile Type,
  Genres, Description und Save als eigene Kopie in den Benutzerordner (§4h).

- **0.36.0** (S114, 2026-10-12): Entscheid P2 a (eigene Instrumente als
  `user:<name>` im Benutzerordner, nichts wird verdrängt; ein Schalter dafür
  vorgemerkt); 14a gebaut — Fassung 3 der `.lvinst` mit Art, Genres und
  Beschreibung, die 49 Dateien über `instrument_katalog.py` angehoben, eigene
  Instrumente im Modell (§4h).

- **0.35.0** (S114, 2026-10-12): §4h — die Skizze zur Palette der
  Instrumente (Stufe 14) gebilligt und als Beschreibung eingetragen;
  Entscheid F3 b (Art, Genres, Beschreibung in der `.lvinst`, Fassung 3);
  Entscheid D1 b: Stufe 14 wartet auf die Datenablage
  (`core-services/Datenablage_Konzept.md`). Nichts davon gebaut.

- **0.34.0** (S113, 2026-10-11): 13d — der Haken „Right-click deletes" in
  Drum- und Instrument-Editor (Wunsch Patrik; im Instrument-Editor löscht der
  Rechtsklick nur noch mit ihm); Benutzerhandbuch und Sichttest-Liste
  nachgezogen, voller Testlauf; Stufe 13 ist gebaut.

- **0.33.0** (S113, 2026-10-11): 13c gebaut — Mehrfachauswahl, Kopieren,
  Einfügen, Ziehen als Gruppe, Versetzen und Drehen im Instrument-Editor
  (§4i).

- **0.32.0** (S113, 2026-10-11): 13b gebaut — Mehrfachauswahl, Kopieren,
  Einfügen, Versetzen und Drehen im Drum-Editor (§4i).

- **0.31.0** (S113, 2026-10-11): Entscheide V2 a (nur die markierten
  Schritte zählen) und V3 a (Pfeiltasten halten am Rand an), dazu das Drehen
  im Kreis mit Alt+Pfeil (Wunsch Patrik); 13a gebaut — die Regeln der
  Mehrfachauswahl als reine Funktionen (§4i).

- **0.30.0** (S113, 2026-10-11): Patrik hat F0 bis F4 der Liste 10 geprüft
  („alle ok", §5 Stufe 12). Die gebilligte Skizze für Stufe 13 —
  Mehrfachauswahl, Kopieren und Einfügen in Drum- und Instrument-Editor
  (§4i neu); Entscheid V1 a (Einfügen im Drum-Editor ersetzt), b verworfen.

- **0.29.0** (S113, 2026-10-11): 12e — Benutzerhandbuch und Sichttest-Liste
  nachgezogen, voller Testlauf; Stufe 12 ist gebaut.

- **0.28.0** (S113, 2026-10-11): 12d gebaut — das Fenster „Clip effects"
  und Effects... im Menü des Clips (§4g).

- **0.27.0** (S113, 2026-10-11): 12c gebaut — mehrere Clips markieren
  (Ctrl+Klick, Rahmen, Ctrl+A), als Gruppe ziehen, kopieren und löschen, die
  Marke fx (§4g).

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

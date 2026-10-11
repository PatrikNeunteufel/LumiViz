# LumiViz — Loop (Konzept)

> **Version:** 0.10.0
> **Datum:** 2026-10-11 (Session 109)
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

### 3.1 Verworfen, mit Grund

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
| `patterns[]` | die Muster („Basen"): Name, Art, Länge und alles, was ihr Editor einstellt | — |
| `patterns[].kind` | Art des Musters, bestimmt den Editor; heute nur `drumkit` | — |
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
  **Save As…**. New und Load fragen nach, wenn Ungespeichertes verloren ginge.
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
- Beim Schließen der App fragt das Panel **nicht** nach Ungespeichertem.

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
| 7 | **Composer:** `.lvloop` als Clip im Audio-Kanal, Wiederholen bis zum Clip-Ende, Zeichnung des Clips, Ziehen aus dem Audio Browser | offen |
| 8 | **Abschluss:** Doku, Benutzerhandbuch, Vorlage für den Sichttest | offen |
| 9 | **Instrument-Clip:** Piano-Rolle, Klangerzeuger für Töne | später |

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

## 7. Risiken

- **Nachsetzen der Tonführung:** eine Quelle wird nachgesetzt, wenn sie mehr
  als 80 ms von der Uhr abweicht (`Tonfuehrung::kToleranzMs`). Bei einem Song
  ist das ein Ruck, bei Drums ein Stolperer. Kommt in Stufe 7 mit einer
  Messung auf den Tisch.
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

# LumiViz — Loop (Konzept)

> **Version:** 0.1.0
> **Datum:** 2026-10-10 (Session 108)
> **Typ:** Konzept — Entscheide L1 bis L5 gefallen (Patrik S108); gebaut ist
> nur der Beleg aus Stufe 2
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Composer_Konzept.md](../ui/Composer_Konzept.md) §4 (Uhr,
> Tonführung, Mixer) · [Audio_System.md](Audio_System.md)
> **Sprache:** Deutsch

---

## 1. Ziel

Ein **Loop-Panel** nach Art des Channel Rack von FL Studio: Spuren
untereinander, Schritte nebeneinander, es spielt Drums im Kreis. Was dort
entsteht, liegt als Datei `.lvloop` vor und lässt sich als **Clip in einen
Audio-Kanal des Composers** legen.

Vorgaben (Patrik S108):

- **Klangquelle:** ein mitgeliefertes Kit **und** eigene Samples je Spur.
- **Raster:** alles einstellbar — Schritte je Takt, Lautstärke je Schritt,
  Swing, mehrere Muster.
- **Tempo:** einstellbar, in BPM.
- **Als Clip:** wird der Clip länger gezogen, wiederholt sich der Loop bis
  zum Clip-Ende.
- **Zweck:** eine Erweiterung für **eigene Songs**. Drums sind der Anfang;
  später Funktionsgeneratoren, Modulation, möglicherweise MIDI.
- **Allein spielbar:** das Panel hat ein eigenes Play.
- **Ablage:** eigene Datei `.lvloop`, in mehreren Kompositionen verwendbar.

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
| **L3 c** | **Eine Datei enthält Kit, Muster und Abfolge.** Der Clip spielt die Abfolge und wiederholt sie. Der `Clip` und die `.lvcomp` bleiben unverändert. |
| **L4 a** | **Wer spielt, hat den Ton — Rangfolge Composer, Loop-Panel, Player.** Play im Loop-Panel hält den Player an und gibt ihn bei Stop zurück; die Visuals reagieren auf den Loop. Hat der Composer den Ton, schweigt das Loop-Panel. |
| **L5 a** | **Das mitgelieferte Kit wird gerechnet**, nicht bezogen: ein Skript erzeugt die WAV-Dateien, sie liegen unter `asset/`. Jede Spur kann stattdessen eine eigene Sample-Datei laden. |

### 3.1 Verworfen, mit Grund

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

### 3.2 Zurückgestellt (Patrik S108)

- **Tempo der Komposition** (L2 a): später zuschaltbar, an der Komposition.
- **Musterwahl am Clip** (L3 a): als Zugabe zu L3 c möglich; bräuchte ein
  Feld am `Clip`.
- Funktionsgeneratoren, Modulation, MIDI.

### 3.3 Noch offen

- Was der Play-Knopf des Loop-Panels zeigt, solange der Composer den Ton hat
  (das Player-Panel steuert in dieser Lage den Composer mit).
- Das Aussehen des Panels — vor Stufe 6 als Skizze.
- Wie der Composer von außen angehalten und fortgesetzt wird (für L4 noch
  nicht gelesen).

---

## 4. Datei `.lvloop` (Skizze, vor Stufe 3 nicht fest)

- **Kopf:** Version des Formats, BPM, Swing, Schritte je Takt.
- **Spuren:** je Spur ein Klang — ein fester Name aus dem mitgelieferten Kit
  **oder** ein Pfad zu einer Sample-Datei (Regel wie in `.lvcomp`) —, dazu
  Lautstärke und Panorama.
- **Muster:** benannt; je Spur die Schritte, je Schritt eine Lautstärke
  (0 = aus).
- **Abfolge:** eine Liste von Musternamen. Ihre Länge mal die Länge eines
  Musters ist ein Durchlauf; der Clip wiederholt ihn.

Der Klang des mitgelieferten Kits wird über den Namen gefunden, nicht über
einen Pfad: eine Datei klingt so auf jedem Rechner gleich.

---

## 5. Stufen

| Stufe | Inhalt | Stand |
|---|---|---|
| 1 | Dieses Konzept | S108 |
| 2 | **Beleg:** BASS nimmt eine selbst rechnende Quelle im Mixer an, die Schläge kommen auf das Sample genau (`test_LoopBelegeTon.cpp`). Scheitert er, kommt L1 noch einmal auf den Tisch. | S108, siehe §6 |
| 3 | **Kern ohne Ton und ohne Fenster:** Modell, Lesen und Schreiben der `.lvloop`, Rechenkern; Unit-Tests über die gerechneten Samples | offen |
| 4 | **Kit:** Skript und WAV-Dateien unter `asset/` | offen |
| 5 | **Wiedergabe allein:** neue Naht in `IAudioEngine`, Rangfolge aus L4, die Visuals lesen vom Loop | offen |
| 6 | **Loop-Panel:** Raster, Spuren, Muster, Abfolge, Play, Datei-Zeile | offen |
| 7 | **Composer:** `.lvloop` als Clip im Audio-Kanal, Wiederholen bis zum Clip-Ende, Zeichnung des Clips | offen |
| 8 | **Abschluss:** Doku, Benutzerhandbuch, Vorlage für den Sichttest | offen |

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
- **Klang des gerechneten Kits:** erst zu beurteilen, wenn es da ist.

---

## 8. Changelog

- **0.1.0** (S108, 2026-10-10): angelegt — Ziel, Bestand, Entscheide L1 bis
  L5, Skizze der Datei, Stufen, Risiken.

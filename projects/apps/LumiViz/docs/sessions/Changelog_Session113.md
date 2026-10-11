# Changelog Session 113 (2026-10-11)

> **Effekte am Clip und Mehrfachauswahl in den Editoren.** Jeder Clip im
> Panel Loop kann eigene Effekte tragen — Distortion, Echo und Reverb —, und
> im Panel wie in den beiden Editoren lassen sich mehrere Clips, Schritte
> oder Noten zugleich markieren, kopieren, einfügen und bewegen. **Im
> Programm geprüft ist davon nur ein kleiner Teil:** das Laden des Beispiels,
> die Marke fx, das Hören mit und ohne Effekte, das Fenster Clip effects und
> ein Regler im Lauf. Alles andere ist mit Unit-Tests und Bildern aus dem
> Test geprüft; alle Werte der Effekte sind nach Rechnung gesetzt, nicht
> nach Gehör.

## Neu

- **Effekte am Clip:** Rechtsklick auf einen Clip im Panel Loop →
  **Effects...** öffnet das Fenster **Clip effects** mit **Distortion**
  (Drive, Tone, Mix), **Echo** (Time als Notenwert, Feedback, Damping, Mix)
  und **Reverb** (Size, Decay, Damping, Mix), je mit einem Haken. Die Effekte
  gehören zum Clip, nicht zur Basis: zwei Clips derselben Basis können
  verschieden klingen. Echo und Hall klingen über das Ende des Clips hinaus
  aus.
- **Mehrere Clips auf einmal:** haben die markierten Clips an einem Regler
  verschiedene Werte, steht dort „mixed"; der Regler setzt den Wert für
  alle. **Copy**, **Paste** und **Reset** übertragen oder löschen die
  Einstellungen.
- **Die Marke fx** steht an jedem Clip mit einem eingeschalteten Effekt.
- **Mehrere Clips markieren** im Panel Loop: **Ctrl+Klick**, ein Rahmen auf
  freier Fläche, **Ctrl+A**. Ziehen bewegt die markierten zusammen, **Entf**
  löscht sie, **Ctrl+D** kopiert sie als Gruppe.
- **Mehrere Schritte im Drum-Editor:** **Shift+Ziehen** markiert einen
  Bereich, **Ctrl+Klick** einzelne Schritte. **Ctrl+C**, **Ctrl+X**,
  **Ctrl+V** an der **Einfügemarke** (ein Klick in die Zeile der Takte setzt
  sie), **Ctrl+Shift+V** legt nur dazu, **Ctrl+D**, **Entf**, die Pfeiltasten
  verschieben, **Alt+Pfeil links / rechts** dreht im Kreis.
- **Mehrere Noten im Instrument-Editor:** dasselbe, dazu das Ziehen mehrerer
  markierter Noten als Gruppe und **Shift+Pfeil** für eine Oktave.
- **Das Kopierte bleibt gemerkt**, wenn der Editor ein anderes Muster zeigt —
  so kommt ein Teil eines Musters in ein anderes.
- **Der Haken Right-click deletes** in beiden Editoren: an löscht der
  Rechtsklick die Note oder leert den Schritt sofort; aus öffnet er ein Menü
  mit einem Eintrag dafür.
- **Eine Hinweiszeile** unter den Rastern und unter der Achse des Panels
  sagt, wie man markiert, und bei einer Auswahl, was die Tasten tun.
- **Beispiel** `asset\loop\effects_demo.lvloop`: das House-Beispiel mit
  Effekten an fünf Clips.
- **Benutzerhandbuch §16:** „Effekte am Clip", das Markieren mehrerer Clips,
  „Mehrere Schritte zugleich", „Mehrere Noten zugleich".

## Geändert

- **Das Format `.lvloop` trägt Fassung 3** (die Effekte am Clip). Dateien der
  Fassung 2 liest LumiViz weiter; gespeichert wird immer Fassung 3. Ein
  älteres LumiViz spielt eine solche Datei ohne Effekte und meldet „File was
  written by a newer version".
- **Im Instrument-Editor löscht der Rechtsklick auf eine Note nicht mehr von
  selbst.** Er markiert sie und öffnet ein Menü mit **Delete** — außer der
  Haken **Right-click deletes** ist gesetzt. Entf löscht wie bisher.
- **Ein Klick in die Zeile der Takte** setzt in beiden Editoren die
  Einfügemarke (vorher tat er nichts).

## Für Entwickler

- [Loop_Konzept.md](../audio/Loop_Konzept.md) 0.34.0 — Entscheide **E2 b**
  (Effekte je Clip), **F2 a** (Feld `effects`, Fassung 3), **G1 b** (die
  Effekte rechnen im Ton-Thread mit, Regel für den Sprung), **P1 a**
  (Palette der Instrumente, mit Genre), **V1 a**, **V2 a**, **V3 a**
  (Einfügen ersetzt, nur markierte Schritte zählen, Pfeiltasten halten am
  Rand an); §4g (Effekte am Clip), §4h (vorgemerkt: Samples als Schicht,
  Multisamples, Palette), §4i (Mehrfachauswahl in den Editoren); Stufen 12
  und 13 gebaut, 11, 14 und 15 offen.
- `loop/Loop.hpp`: `ClipEffekte` (`ClipVerzerrung`, `ClipEcho`, `ClipHall`),
  `EchoZeit`; `LoopClip` hat einen Konstruktor statt Aggregat.
- `loop/Effekte.hpp` (neu): `Verzerrer`, `EchoEffekt`, `HallEffekt` (Bauart
  Freeverb), `EffektKette`, `nachklangSekunden` — `rechne` und `leere` holen
  keinen Speicher.
- `loop/Rechenkern.hpp`: `Ablauf::busse`, `Schlag::bus`, `rechne` mit
  `busAus`; der Kern bleibt ohne Gedächtnis.
- `loop/Klangerzeuger.hpp`: `EffektZustand` am Programm, `kEffektBlock`,
  `kSprungMs`; ein abgelöstes Programm gibt der Ton-Thread ab, statt es
  selbst loszulassen.
- `loop/Bearbeiten.hpp`: `ClipOrt`, `versetzeClips`, `dupliziereClips`,
  `loescheClips`. `loop/Auswahl.hpp` (neu): die Regeln für Schritte und
  Noten. `loop/ClipEffektFenster.hpp` (neu).
- Gemessen (`test_LoopEffekte.cpp`, eine Kette, außerhalb des Ton-Threads):
  Release-clang nur Hall 0,93 bis 0,97 ms je Sekunde Ton, alle drei Effekte
  1,9 bis 2,4 ms; Testing-MSVC 5,6 und 7,4 bis 7,8 ms.
- Bilder aus den Tests: `LUMIVIZ_LOOP_MARKEN_BILD`,
  `LUMIVIZ_CLIP_EFFEKTE_BILD`, `LUMIVIZ_DRUM_MARKEN_BILD`,
  `LUMIVIZ_INSTRUMENT_MARKEN_BILD` (je `=<datei.png>`);
  `LUMIVIZ_EFFEKT_MESSUNG_S=<Sekunden>` misst an mehr Ton;
  `LUMIVIZ_INSTRUMENT_WAV=<Ordner>` legt auch `effects_demo.wav` ab.

## Offen

- Der Blick ins Programm und das Ohr: von den Effekten F5 bis F17 der
  Sichttest-Liste, von der Mehrfachauswahl in den Editoren alles.
- Die Last der Effekte im Ton-Thread selbst und mit vielen Clips zugleich
  ist nicht gemessen.
- Ein Loop-Clip mit Effekten im Composer hat keinen eigenen Testfall.
- Instrumente aus Samples (mit Root note und Detect), die Palette der
  Instrumente mit Genre, Wave-Clips und Autotune.
- `asset\loop` liegt im Release-Paket weiter nicht neben der Exe.

## Tests

1068 Cases (24 neu), voller Lauf 1068 / 1068, 0 Skips (Testing-MSVC) im
zweiten Anlauf — der erste brach nach 267 Fällen ab, ohne fehlgeschlagene
Prüfung, bei Composer-Fällen, die ein echtes Fenster öffnen; einzeln liefen
sie fünfmal grün, die Ursache ist nicht untersucht. Unter Release-clang 104
Fälle zu Loop, Achse, Effekten, Editoren und Sichttest-Aktionen. Nicht
gemacht: Debug-MSVC, der volle Lauf unter clang.

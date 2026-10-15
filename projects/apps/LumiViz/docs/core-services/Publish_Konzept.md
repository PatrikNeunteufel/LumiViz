# LumiViz — Publish (Konzept)

> **Version:** 0.3.0
> **Datum:** 2026-10-15 (Session 125)
> **Typ:** Konzept — die Entscheide P1 a bis P8 a sind gefallen (Patrik S125,
> §3), die drei Skizzen sind gebilligt (§4); gebaut: der Namensbau von Keep
> both (P6 a, §2b) und die Sammler der Loop-Familie (§5a)
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Datenablage_Konzept.md](Datenablage_Konzept.md) §4a (Export und
> Import der eigenen Daten, `.lvdata` Fassung 1) ·
> [Offene_Punkte.md](../Offene_Punkte.md) §7 „Idee: Publish" (Patrik S120,
> S123) · [Loop_Konzept.md](../audio/Loop_Konzept.md) ·
> [Shape_Konzept.md](../audio/Shape_Konzept.md) §7 (D3 a: ein Instrument nennt
> seine Form)
> **Sprache:** Deutsch

---

## 1. Anlass

Ein Instrument **nennt** seine Samples und seine Form, ein Loop seine
Instrumente, Klänge und sein Kit (Shape_Konzept D3 a). Eine Datei allein ist
deshalb nicht vollständig: Wer sie weitergibt oder verschiebt, verliert, was
sie nennt. Belegt am Versuch „Titanium" (S123): der Loop nennt drei
Instrumente als Datei neben sich.

**Patriks Lösung (S120):** **Publish** archiviert ein Ding mit allen
zugehörigen Dateien und der Ordnerstruktur des Benutzerordners in ein Paket
`<name>.<art>.lvdata` mit Manifest. Angehen nach Kits und Vorlagen S123
(Patrik S123) — das ist mit Stufe 21 erfüllt.

## 2. Befund (S125, nachgesehen)

### 2a. Wer wen nennt

| Art | nennt | Form | Auflösung | Beleg |
|---|---|---|---|---|
| `.lvinst` | Sample | `user:`, `kit:`, `lib:`, Pfad — **unverändert, also absolut** | `sampleDatei` → `klangDatei` | `Instrument.cpp:728`, `Sample.cpp:144`, `Kit.cpp:76` |
| `.lvinst` | Kontur | Name, `user:`, Pfad `.lvcontour` (absolut) | `holeZeichnung` | `Instrument.cpp:739`, `Kontur.cpp:499` |
| `.lvloop` | Instrument | Name, `user:`, Pfad `.lvinst` (relativ, wenn unter dem Loop) | `holeInstrument` | `Loop.cpp:595,623`, `Instrument.cpp:1299` |
| `.lvloop` | Klang | `kit:`, `lib:`, `user:`, Pfad (relativ/absolut) | `klangDatei` | `Loop.cpp:614,747` |
| `.lvloop` | Kit | Name, `user:`, Pfad `.lvkit` | `holeKit` | `Loop.cpp:635,787`, `DrumKit.cpp:351` |
| `.lvkit` | Klang, Instrument | wie `.lvloop` | wie oben | `DrumKit.cpp:232-236` |
| `.lvcontour` | — | nur der Name der Vorlage zur Anzeige | — | `Kontur.cpp:295` |
| `.lvcomp` | Preset, Song, `.lvloop` | relativ, wenn darunter, sonst absolut | `ausDatei` | `Komposition.cpp:27-43,536` |
| `.lvfx`/`.lvfx2` | Preset (Host Group) | `sourceFile`, unverändert | relativ zur ladenden Datei | `ChainSerializer.cpp:278` |
| `.lvfx`/`.lvfx2` | Video, AVI | `filePath`, `resolvedPath` (absolut) | Suche aufwärts, Suchordner | `MultiEffectVisualizer.cpp:3187,3221` |
| `.lvfx`/`.lvfx2` | Kontur | Shape-Nennung | `holeZeichnung` | `MultiEffectVisualizer.cpp:13622` |

Eingebettet und darum ohne Nennung: Bilder von Picture und Texer,
Milkdrop-Texturen, Blenden- und Stückformen der Komposition, der Songtext
eines Loops. Ein Song bringt seinen Text als `.lrc`/`.txt` neben sich mit
(`SongtextQuelle.hpp:11-16`).

### 2b. Was es gibt und was fehlt

- Loop und Kit schreiben einen Pfad relativ, wenn die Datei unter ihrem Ordner
  liegt (`pfadFuerDatei`, `Loop.cpp:58-73`); das Instrument nicht.
- **Loops, Kompositionen und eigene `.lvfx` haben keinen eigenen Ordner** im
  Benutzerordner; die `.lvdata` (Fassung 1) nimmt sie nicht mit
  (`Datensicherung.cpp:154-183`), ihr Import lässt einen Eintrag ohne
  bekannten Bereich aus (`Inhalt::ausgelassen`).
- **Keine Funktion sammelt alle Nennungen eines Dings oder schreibt sie um.**
  Teilstücke: `eigeneInstrumenteMitSample`, `eigeneInstrumenteMitForm`,
  `loopsMitKit`, `benenneBasisUm`, `hostGruppeUmbenennenInSwitchern`;
  `nimmZuEigenen` und `nimmZuEigenenInstrumenten` kopieren in den
  Benutzerordner und bilden bei einem vergebenen Namen `<name>_2`
  (`Instrument.cpp:1271`).
- **Fehler im Import, belegt und behoben (S125):** Keep both
  bildet `bass (2).lvinst` (`nameMitZahl`, `Datensicherung.cpp:410`). Den
  Namen übergeht das Programm in der Liste der eigenen Instrumente
  (`Instrument.cpp:1123`) und beim Auflösen von `user:` (`eigeneDatei`,
  `:1110`) — die Datei ist unsichtbar. Gilt ebenso für Samples, Kits und
  Konturen (gleiche Namensregel). Belegt mit dem Fall „Keep both — der neue
  Name ist ein eigener Name“ in `test_Datensicherung` (vier falsche Namen,
  ein Instrument fehlte in der Liste); behoben mit P6 a: `nameMitZahl` bildet
  `<stamm>_<zahl>`, den Stamm auf 60 Zeichen gekürzt.

## 3. Entscheide (Patrik S125)

**P1 a — welche Arten:** Publish gibt es für jede Art, die eine Datei ist:
Instrument, Kit, Loop, Kontur, Sample, Komposition und Effect chain. Ein Paket
nimmt alles mit, was das Ding nennt, über alle Stufen. Gebaut in zwei
Schritten: zuerst die Loop-Familie, dann Komposition und Effect chain (§5).

**P2 a — wo der Befehl steht:** **Publish...** hinter Save as an allen sieben
Orten, die heute speichern — Panel Loop, Instrument-Creator, Fenster Kits,
Edit shape, Record sample, Panel Composer, im Menü File als **Publish Effect
Chain...**. Dazu im Menü File **Publish a file...** (fragt nach einer Datei
jeder Art) und **Import package...**; Settings → User data → Import... nimmt
ein Paket ebenfalls.

**P3 a — was mitgeht, wohin beim Import:** Alles Eigene und jede Datei an
freiem Ort geht mit. Beim Import kommt jede Zugehörige in den Benutzerordner
ihrer Art, jede Nennung wird `user:<name>` (Name aus dem Dateinamen über
`eigenerName`). Das Hauptding geht bei Loop, Komposition und Effect chain
dorthin, wo der Empfänger es ablegt; Loops und Presets einer Komposition
kommen neben die Komposition. Mitgeliefertes geht nie mit, das Manifest nennt
seine Namen; fehlt beim Empfänger einer, meldet der Import ihn.

**P4 a — Medien:** Haken **Include media files** mit der Summe der Größe,
anfangs aus. Mit dem Haken gehen Songs (mit `.lrc`/`.txt` daneben), Videos und
AVI-Dateien mit und kommen neben das Hauptding. Ohne den Haken nennt das
Manifest jede mit Name und Größe (beim Song Titel und Interpret), die Nennung
wird der bloße Dateiname neben dem Hauptding, der Import meldet: „2 media
files not included — put them next to the composition".

**P5 a — Manifest und Fassung:** Das Paket ist eine `.lvdata` in **Fassung 2**,
`<name>.<art>.lvdata`. Zugehörige unter `data/<Bereich>/…` mit **schon beim
Packen umgeschriebenen** Nennungen; Hauptding, Loops, Presets und Medien ohne
eigenen Bereich unter `package/…`. Im Manifest ein Block `package`: Art,
Hauptdatei, die Zugehörigen (Datei, Art, `user:`-Name, wer sie nennt), die
Namen des Mitgelieferten, die nicht enthaltenen Medien. Der Export aller Daten
schreibt weiter Fassung 1; ein älteres Programm weist ein Paket ab und meldet
die neuere Fassung.

**P6 a — gleiche Namen:** je gleichem Namen eine Zeile mit Vergleich (§4c);
Vorgabe **Keep both**: die aus dem Paket heißt `<name>_2`, jede Nennung im
Paket wird mitgeändert; dazu **Replace mine** und **Use mine**, ein Menü
**All:**. Byte für Byte gleich: keine Zeile, die eigene wird genommen.
`nameMitZahl` bildet künftig `<name>_2` statt `<name> (2)` — auch im Import
aller Daten.

**P7 a — Dialog Publish:** wie §4b.

**P8 a — Dialog Import package:** eigener Dialog wie §4d, von beiden Wegen
aus; eine Datensicherung öffnet weiter den Dialog Import data,
File → Import package... nimmt beides und öffnet den passenden.

## 4. Skizzen (gebilligt S125, als Beschreibung)

### 4a. Die Orte des Befehls

In jeder Datei-Zeile der sieben Orte steht **Publish...** als letzter Knopf,
rechts von Save as / Save As... (Panel Loop: Tags..., New, Load..., Save,
Save As..., **Publish...**). Das Menü File trägt nach Save Effect Chain As...
**Publish Effect Chain...**, nach einem Trenner **Publish a file...** und
**Import package...**, nach einem weiteren Trenner wie heute Open
Composition....

### 4b. Der Dialog Publish

Titel **Publish <Art>** (etwa „Publish loop"). Von oben:
- **Package:** der Dateiname, vorbelegt `<name>.<art>.lvdata`, änderbar.
- Ein Baum des Inhalts: oben das Hauptding („titanium — Loop · main file"),
  darunter je Art ein Zweig mit Zahl („Instruments (3)", „Samples (1)"), je
  Zeile Name und grau die Herkunft mit dem künftigen Namen: „next to the loop
  → user:pump_bass", „yours · user:bass", „D:\Musik\kick.wav → user:kick".
- Grau und eingeklappt: **Bundled, not included (n):** die Namen.
- In der Warnfarbe: **Not found (n):** je fehlende Datei, „the package names
  it". Publish geht trotzdem.
- Haken **Include media files** mit Größe, oder „— none in this <art>".
- **Save to** mit Browse....
- Unten links „5 files, 1.2 MB", rechts **Publish** und **Cancel**.

### 4c. Same names (im Dialog Import package)

Überschrift „Same names (n)", eine Tabelle mit den Spalten Name (mit dem Icon
der Art und darunter grau die Art), Yours, In the package, Use. In Yours und
In the package je ▶ zum Anspielen (Instrument, Sample, Kit) oder ein kleines
Bild (Shape), daneben Datum und Größe (beim Sample die Dauer), darunter grau
„used by …". In Use ein Menü: **Keep both → <name>_2** (Vorgabe), **Replace
mine**, **Use mine**. Unter der Tabelle links die Summe („Keep both renames 2
files; the package's loop will name them"), rechts das Menü **All:**.

### 4d. Der Dialog Import package

Von oben:
- Kopf: Icon, „Import package — <name>", darunter grau Art, Datum der
  Veröffentlichung, LumiViz-Version des Absenders, Dateien und Größe.
- **Contents:** je Datei, wohin sie kommt („→ Instruments, new", „→ Samples,
  same name (see below)"); das Hauptding „→ Save to (below)".
- **Same names (n):** wie §4c; fehlt, wenn es keine gibt.
- **Notes:** in der Warnfarbe, was der Absender nicht hatte („Not in the
  package (the sender didn't have it): …"); die nicht enthaltenen Medien mit
  dem Ort, an den sie gehören; grau das Mitgelieferte („Bundled with LumiViz,
  all present: …" oder in der Warnfarbe, was hier fehlt).
- **Save to** mit Browse... — nur, wenn die Art keinen eigenen Ordner hat.
- Unten links der Haken **Open the <art> after import**, anfangs an; rechts
  **Import** und **Cancel**.

## 5. Stufen (Vorschlag)

- **Stufe P-A — die Loop-Familie:** die Sammler je Art (Instrument, Kit,
  Loop, Kontur, Sample) an einer gemeinsamen Schnittstelle; Packen mit
  Umschreiben; `.lvdata` Fassung 2 schreiben und lesen; Import mit
  Umbenennen und Umschreiben; `nameMitZahl` auf `<name>_2` mit einem Test,
  der den Fehler aus §2b zuerst belegt; die Dialoge Publish und Import
  package; die Knöpfe an den fünf Orten der Loop-Familie und im Menü File.
- **Stufe P-B — Komposition und Effect chain:** ihre Sammler (Presets, Loops,
  Host-Group-Verweise, Videos, AVI, Konturen der Kette), Medien mit dem
  Haken, die Knöpfe im Panel Composer und Publish Effect Chain....
- Danach: Handbuch, Sichttest-Liste, voller Lauf.

### 5a. Wie die Sammler gebaut sind (S125)

`paket/Sammler` (`sammle(datei)`; der Ordner heißt nicht `publish/`, den
schließt die `.gitignore` aus) geht von einer Datei aus alle Nennungen
ab, über alle Stufen, und teilt sie in Zugehörige, Mitgeliefertes und
Fehlendes. Gezählt wird, **was spielt**: an einer Drum-Spur oder einem Klang
des Kits das Instrument, wenn eines gesetzt ist, sonst der Klang; an einer
Schicht das Sample nur bei der Waveform Sample, die Form nur bei Shape. Jede
Datei kommt einmal vor (nach ihrem kanonischen Pfad), mit allen Nennungen,
die auf sie zeigten, und den Titeln dessen, was sie nennt. Tests:
`test_PublishSammler` (vier Fälle).

**Von Claude gesetzt:**
- Eine Datei im Ordner der eigenen ihrer Art mit einem gültigen eigenen
  Namen zählt als eigen, auch wenn sie als Pfad genannt ist (damit ist der
  dritte Punkt aus §6 entschieden).
- Eigene behalten ihren Namen; eine Datei an freiem Ort bekommt ihn aus dem
  Dateinamen (`eigenerName`, „Snare Hit.wav“ → `snare_hit`), bei gleichem
  Namen in derselben Art `<name>_2` — Eigene zuerst, damit sie ihren behalten.
- Eine Zugehörige, die sich nicht lesen lässt, geht trotzdem mit, nur ohne
  ihre Nennungen; fehlt die Hauptdatei, gibt es keinen Fund.
- Die Art im Manifest und im Paketnamen: `lvinst`, `lvkit`, `lvloop`,
  `lvcontour`, für ein Sample `sample`.

## 6. Offen, von Claude zu setzen und vorzulegen, wo es sichtbar wird

- Die Namen im Manifest-Block `package` und der Aufbau unter `package/`.
- Was „used by" in §4c zählt (eigene Instrumente, zuletzt geöffnete Loops —
  wie `loopsMitKit`).
- ~~Ob eine Nennung als Pfad auf eine Datei im **Benutzerordner** (statt
  `user:`) als eigen oder als freier Ort zählt.~~ Eigen (§5a).

## Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.3.0 | 2026-10-15 | Session 125. §5a NEU: die Sammler der Loop-Familie gebaut (`paket/Sammler`), mit den von Claude gesetzten Regeln. |
| 0.2.0 | 2026-10-15 | Session 125. §2b: Fehler bei Keep both mit einem Test belegt und behoben (`nameMitZahl` → `<name>_2`). |
| 0.1.0 | 2026-10-15 | Session 125. Erstfassung: Befund der Nennungen, Entscheide P1 a bis P8 a, drei Skizzen als Beschreibung, Stufen P-A und P-B als Vorschlag. |

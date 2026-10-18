# LumiViz — Saiteninstrumente (Konzept)

> **Version:** 0.13.0
> **Datum:** 2026-10-18 (Session 132)
> **Typ:** Konzept — Ziel, Modell, Klang, Creator. Entschieden sind S-Z1 a,
> S-M1 a, S-M2 a, S-M2b (a und b schaltbar), S-M2c a, S-M3 a, S-K1 (a und
> b schaltbar), S-K2 b, S-K3 a und S-U1 a (Skizze des Creators gebilligt;
> Patrik S132); alles andere ist offen oder Vorschlag und so bezeichnet.
> Die Baustufen (§5) sind freigegeben (S-B1 a); gebaut sind 23a (§5a) und
> 23b (§5b).
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Loop_Konzept.md](Loop_Konzept.md) §4f (Instrumente aus
> Schichten), §4j (Signal path) · `Offene_Punkte.md` §6 („Loop:
> Saiteninstrumente", S128) und §7 (Idee Spielweisen, S127)
> **Sprache:** Deutsch

---

## 1. Ziel und Reihenfolge (Entscheid S-Z1 a, Patrik S132)

Ein Instrument kann ein **Saiteninstrument** sein: es trägt Saiten mit einer
Stimmung, und jede Saite klingt über eine verbesserte Saiten-Schwingung. Gebaut
wird in drei Schichten, jede auf der vorigen:

1. **Das Modell:** Saiten, Stimmung (Saitenzahl, Töne der leeren Saiten,
   Capo), besserer Klang der Saite (etwa Anschlagstelle, Korpus, Helligkeit
   des Anschlags).
2. **Die Spielweisen:** Anschlag *down* und *up* mit Versatz der Saiten eines
   Akkords, Muster wie down-up-down-up, *Tapping* als eigene Artikulation
   (Idee Patrik S127).
3. **Die Tabs:** Akkord-Tabulaturen als Ansicht **neben** der Piano-Roll,
   nicht statt ihr; eigene oder aus Vorlagen, auf dem Griffbrett verschiebbar.

Das Konzept beschreibt alle drei; gebaut wird in dieser Reihenfolge.
Begründung: Spielweisen und Tabs fragen beide „welche Saite spielt diesen
Ton?" — das weiß nur ein Instrument, das seine Saiten und seine Stimmung kennt.

**Als eigene Fälle geführt, nicht in der ersten Stufe:** die Violine (wird
gestrichen, nicht angeschlagen) und die Harfe (kein Griffbrett, eine Saite je
Ton).

---

## 2. Was es schon gibt

Nachgesehen in S132, je mit der Stelle.

| Baustein | Stand |
|---|---|
| Schwingung **Plucked string** | `Schwingung::Saite` (`loop/Instrument.hpp`), gerechnet in `rechneSaite` (`src/loop/Instrument.cpp`): Karplus-Strong — ein Stoß Rauschen aus einer festen Folge je Tonhöhe läuft durch eine Verzögerung von einer Schwingung Länge, je Umlauf gemittelt und gedämpft |
| Regler der Saite | nur **Damping** (`Schicht::saiteDaempfung`, 0,9 … 0,9999, gilt bei 220 Hz; andere Tonhöhen werden so gedämpft, dass sie ähnlich lang klingen) |
| Filter an der Saite | übersprungen (von Claude gesetzt, nicht entschieden — Loop_Konzept, Abschnitt Signal path); Drive und die Stelle der Envelope gelten |
| Ein Ton | wird je Note **vorab und für sich** gerechnet (`rendereTon(instrument, hoehe, dauerMs, rate)`); der Rechenkern spielt ihn wie einen Schlag des Kits und hat kein Gedächtnis zwischen Noten |
| Mitgelieferte Instrumente mit Saite | 12 von 93: Guitar, Clean, Muted, Nylon, Overdrive Guitar, Clavinet, Slap Bass, Upright Bass, Kalimba, Conga, Flute, Broken Radio |
| Violin | eine Säge (`saw`), Art `pads` — keine Saite |
| `.lvinst` | Fassung 9 |

**Fehlt:** Anschlagstelle, Korpus, mehrere Saiten, Stimmung, Capo; eine Note,
die eine noch klingende auf derselben Saite beendet; Anschlagrichtung und
Versatz im Akkord.

---

## 3. Offene Fragen (einzeln vorzulegen)

- ✅ **S-M1 a** (Patrik S132): **Die Saiten gehören dem Instrument.** Ein
  neuer Teil `Instrument::saiten` trägt die Saitenzahl, je Saite den Ton der
  leeren Saite (etwa E2 A2 D3 G3 B3 E4) und das Capo. Ein Instrument ohne
  diesen Teil ist wie heute. Die Schichten bleiben der Klang; eine
  Saiten-Schicht erfährt beim Rechnen, auf welcher Saite die Note liegt, und
  kann ihren Klang danach richten. Verworfen: b (Stimmung je Schicht — zwei
  Saiten-Schichten hätten zwei Stimmungen, Strum und Tabs wüssten nicht,
  welche gilt), c (Stimmung am Clip — eine bei Gitarren fast immer gleiche
  Angabe an jedem Clip, der Creator wüsste beim Vorhören nicht, wie
  gestimmt ist). Eine Abweichung am Clip (Drop D, Capo je Song) bleibt
  später möglich, ohne den Ort zu ändern.
- ✅ **S-M2 a** (Patrik S132): **Eine Note trägt optional eine Saite**; leer =
  automatisch, aus der Stimmung des Instruments nach einer festen Regel
  (S-M2b). Gesetzt gilt sie, solange die Note auf dieser Saite greifbar ist.
  Die Piano-Roll setzt sie nicht von selbst, die Tabs setzen sie immer; Noten
  von Instrumenten ohne Saiten tragen nie eine. Folge: die Saite gehört in den
  Namen des gerechneten Tons (heute `inst:<instrument>:<hoehe>:<dauer>`).
  Verworfen: b (nie an der Note — die Tabs könnten eine Lage nicht
  festhalten), c (immer an der Note — Pflichtangabe in der Piano-Roll, alte
  Noten stünden falsch).
- ✅ **S-M2b a + b, schaltbar** (Patrik S132): zwei Regeln für „automatisch":
  - **Je Klang** (a): Noten, die sich zeitlich überlappen, bekommen
    verschiedene Saiten, mit dem kleinsten Abstand der Bünde zueinander; bei
    Gleichstand die tiefste Lage (leere Saiten zuerst). Ohne Gedächtnis —
    eine Note ändert ihre Saite nur, wenn sich an ihr oder an einer mit ihr
    klingenden etwas ändert.
  - **Lage folgt der Hand** (b): wie a, aber unter mehreren Möglichkeiten die
    nächste zur Lage des vorigen Klangs; das Muster wird von vorn nach hinten
    durchgerechnet.
  - Verworfen: c (je Note die tiefste Lage — Akkorde über die Saiten
    verstreut).
  - Grenze (Vorschlag, mit der Skizze vorzulegen): mehr gleichzeitige Noten
    als Saiten oder eine Note auf keiner Saite greifbar — sie klingt auf der
    nächstbesten Saite, der Editor zeigt sie als nicht greifbar.
- ✅ **S-M2c a** (Patrik S132): **Der Schalter steht am Muster** und wird in
  der `.lvloop` gespeichert; im Instrument-Editor als Auswahl *String choice:
  Per chord / Follow hand*. Ein neues Muster bekommt die Vorgabe des
  Instruments (ein Feld in der `.lvinst`, Fassung 10); Standard *Per chord*.
  Verworfen: b (am Instrument — bindet die Regel an den Klang statt an die
  Musik; Akkorde und Melodie derselben Gitarre bräuchten zwei Instrumente),
  c (Einstellung des Programms — dieselbe `.lvloop` klänge anderswo anders).
- ✅ **S-M3 a** (Patrik S132): **Beim Planen kürzen.** Wo die Saite einer
  Note bestimmt wird (S-M2b), wird jede vorige Note auf derselben Saite bis
  zum Beginn der neuen gekürzt und mit einem kurzen Abdämpfen (wenige
  Millisekunden, wie das Plektrum vor dem Anschlag) beendet statt mit dem
  vollen Ausklang — auch über die Grenze eines Durchlaufs und von einem Clip
  zum nächsten desselben Musters. Gerechnet wird weiter je Note vorab; der
  Rechenkern bleibt ohne Gedächtnis und weiß nichts von Saiten. Folge: der
  Name des Tons trägt die gekürzte Dauer und die Saite. Verworfen: b (Stimmen
  je Saite im Rechenkern — der Kern müsste Instrumente kennen, könnte nur
  ausblenden, Creator, Editor und Gehörtes stimmten nicht überein), c (eine
  Saite schwingt durch das ganze Muster — kein Zwischenspeicher je Ton mehr).
  Für gebundene Noten (Hammer-on, Pull-off, Slide) kann c später als
  Ausnahme dazukommen; die Regel hier bleibt dann richtig.
- ✅ **S-K1 a + b, schaltbar** (Patrik S132): zwei Verfahren für die Saite:
  - **Waveguide** (a): die Saite als zwei gegenläufige Wellen zwischen Sattel
    und Steg, Verlust- und Steifheitsfilter am Ende; Anschlagstelle als Ort
    auf der Saite, Anschlag als Form (Plektrum, Finger, weich/hart), Korpus
    als Resonanzen hinter dem Steg. Trägt später den Bogen (Violine).
  - **Extended Karplus-Strong** (b, nach Jaffe und Smith): eine Schleife mit
    Anschlagstelle (Kammfilter auf dem Stoß), Helligkeit des Anschlags,
    Steifheit (Allpass), frequenzabhängigem Verlust; Korpus als Resonanzen.
  - Verworfen: c (Commuted Synthesis mit aufgenommenen Korpus-Antworten —
    hängt an Aufnahmen Dritter; eine Impulsantwort mit sauberer Lizenz kann
    den Korpus später ergänzen).
  - **Folge aus S-M1 a (nicht eigens gefragt):** der Schalter ist ein Regler
    der Schicht — die Schichten sind der Klang.
  - **Von Claude gesetzt, nicht entschieden:** Vorgabe einer neuen
    Saiten-Schicht ist *Waveguide*.
- ✅ **S-K2 b** (Patrik S132): **Die heutige Saite bleibt als drittes
  Verfahren *Classic*, unberührt im Code** (`rechneSaite`). Die Schwingung
  *Plucked string* trägt den Regler *Model: Classic / Extended KS /
  Waveguide*; eine Datei bis Fassung 9 liest sich als *Classic* und klingt
  damit Bit für Bit wie heute, ohne Beweislast für das erweiterte Verfahren.
  Die neuen Felder bringen **Fassung 10** der `.lvinst` (ein älteres Programm
  spielte einen Waveguide sonst still als die alte Saite). Verworfen: a
  (Classic geht im Extended KS auf — Bitgleichheit erst zu belegen), c (eine
  zweite Schwingung neben *Plucked string* — zwei Saiten, die Strum und Tabs
  beide kennen müssten).
- ✅ **S-K3 a** (Patrik S132): **Regler nach der Physik**, gemeinsam für
  Extended KS und Waveguide — jeder verstellt genau eine Größe des Modells:
  - *Pluck position*: wo angeschlagen wird, Anteil der Saite vom Steg aus
    (nah am Steg hell und dünn, zur Mitte rund);
  - *Pluck*: Form des Anschlags, weich (Finger) bis hart (Plektrum);
  - *Ring time* (in der Vorlage „Decay“, umbenannt mit S-U1 a — die
    Hüllkurve heißt im selben Fenster schon Decay): Klingdauer in Sekunden
    bei 220 Hz, andere Tonhöhen angeglichen;
  - *High damping* (in der Vorlage „High decay“, umbenannt mit S-U1 a): um
    wie viel schneller die Höhen vergehen;
  - *Stiffness*: Steifheit der Saite (Obertöne leicht zu hoch);
  - *Body*: Auswahl None, Acoustic guitar, Classical guitar, Acoustic bass,
    Upright bass, Violin — dazu *Body amount*;
  - *String variation*: wie stark sich die Saiten des Instruments
    unterscheiden (tiefe dicker, dunkler, steifer); 0 = alle gleich;
  - nur Waveguide: *Pickup position*: wo der Ton abgenommen wird
    (Tonabnehmer); ganz am Steg = am Korpus wie akustisch.
  - *Classic* behält nur *Damping*.
  - Nicht hier: Palm mute, Tapping, Anschlagrichtung (Spielweisen, Stufe 2,
    an der Note); Stimmung und Capo (am Instrument, S-M1 a).
  - Verworfen: b (Makros nach Höreindruck — verdeckte Abbildung, nicht
    prüfbar), c (Makros und *Advanced* — doppelte Bedienung, überschreibt
    still).
- ✅ **S-U1 a** (Patrik S132): **Skizze des Creators gebilligt**, wie in §4
  beschrieben, mit den Namen *Tuning*, *Ring time*, *High damping* und
  ziehbaren Marken im Saitenbild. Verworfen: b (Stimmung hinter einem Knopf
  *Tuning...* — gehört sichtbar zum Instrument wie *Type*; erneut vorlegen,
  wenn eine Saite später mehr trägt, etwa Dicke oder Oktav-Partner), c (ohne
  Saitenbild).
- **S-U2** Der Instrument-Editor: *String choice* am Muster, Anzeige einer
  nicht greifbaren Note — **eigene Skizze vor dem Bau**.
- Danach Spielweisen (§1 Punkt 2) und Tabs (§1 Punkt 3) mit eigenen Fragen.

---

## 4. Der Creator (Skizze gebilligt, S-U1 a, Patrik S132)

Neu gegenüber heute; alles Übrige bleibt, wie es ist.

- **Zeile Tuning** — gehört dem Instrument (S-M1 a) und steht unter der
  Zeile *Type / Genres / Description*, über Layers und Signal path. Von
  links:
  - **Tuning**: Vorlage — *None*, *Guitar standard*, *Drop D*, *Bass 4*,
    *Bass 5*, *Ukulele*, *Violin*, *Custom*. *None* = kein
    Saiteninstrument; die Zeile zeigt dann nur diese Auswahl.
  - **Strings**: Zahl der Saiten, 1 … 12.
  - je Saite ein Feld mit dem Ton der leeren Saite (`E2 A2 D3 G3 B3 E4`), von
    der tiefsten an; ein Klick ändert den Ton, die Vorlage springt dann auf
    *Custom*.
  - **Capo**: 0 … 12.
  - **String choice**: *Per chord* / *Follow hand* — die Vorgabe für neue
    Muster (S-M2c a).
  - Passt die Zeile nicht in die Breite, bricht sie um (wie *Signal path*).
- **Kasten Source** nennt bei *Plucked string* das Verfahren („Source ·
  Waveguide“).
- **Gruppe Oscillator**: unter *Waveform* der Regler **Model** — *Classic*,
  *Extended KS*, *Waveguide*.
- **Gruppe String** (einklappbar wie *Oscillator*): zuerst das
  **Saitenbild** — Sattel links, Steg rechts, die Saite dazwischen, eine
  Marke *pluck* und eine Marke *pickup*; beide lassen sich ziehen und stellen
  dieselben Werte wie ihre Regler. Darunter *Pluck position*, *Pluck*, *Ring
  time*, *High damping*, *Stiffness*, *String variation*, *Pickup position*.
- **Gruppe Body** (einklappbar): *Body* (Auswahl), *Body amount*.
- Bei *Classic* stehen statt der Gruppen String und Body nur *Damping* (wie
  heute); bei *Extended KS* fehlen *Pickup position* und die Marke *pickup*.

---

## 5. Baustufen (freigegeben, S-B1 a, Patrik S132)

Fortsetzung der Zählung aus `Loop_Konzept.md` (zuletzt Stufe 22), in dieser
Reihenfolge. Je Stufe die Bereichsfälle unter beiden Compilern, der volle Lauf
am Ende von Stufe 23.

| Stufe | Inhalt | Beleg |
|---|---|---|
| 23a | **Modell und Datei:** `Instrument::saiten` (Töne der leeren Saiten, Capo, Vorgabe *String choice*), `Note::saite` (optional), die Regel am Muster; `.lvinst` **Fassung 10**, `.lvloop` **Fassung 8** (ein älteres Programm verlöre die Saite einer Note still); Namen der Felder von Claude gesetzt und hier nachgetragen | Rundweg der Dateien; jede mitgelieferte Datei liest und schreibt sich gleich; `test_LoopWerte` unverändert |
| 23b | **Saitenwahl und Kürzen beim Planen:** die Regeln *Per chord* und *Follow hand* (S-M2b), die Grenze „nicht greifbar", das Kürzen mit Abdämpfen (S-M3 a), Saite und gekürzte Dauer im Namen des Tons | Fälle ohne Qt-Anwendung je Regel (Akkorde, Melodie, mehr Noten als Saiten, über die Grenze eines Durchlaufs); Instrumente ohne Saiten Bit für Bit wie vorher (`LUMIVIZ_LOOP_WERTE_GENAU`) |
| 23c | **Extended KS:** Pluck position, Pluck, Ring time, High damping, Stiffness, String variation; Body als Resonanzen — die Werte der Korpusse mit Quelle belegt, nicht geschätzt | Messungen am gerechneten Ton (fehlende Teiltöne bei der Anschlagstelle, Abklingzeit, Verstimmung der Obertöne bei Stiffness); Hörproben |
| 23d | **Waveguide** mit Pickup position | dieselben Messungen; bei gleicher Stellung gegen 23c verglichen; Hörproben |
| 23e | **Creator** nach der Skizze (§4) | Bilder aus den Tests; die Marken ziehen mit der Maus |
| 23f | **Instrument-Editor** (S-U2, Skizze vor dem Bau): *String choice* am Muster, Anzeige nicht greifbarer Noten | Bilder aus den Tests |
| 23g | **Mitgelieferte Instrumente:** Vorschlag, welche auf die neuen Verfahren umgestellt werden (vorher/nachher als Hörproben, eigene Vorlage); Handbuch, Sichttest-Liste, voller Lauf | `test_LoopWerte` neu geschrieben nur für die umgestellten |

### 5a. Stufe 23a — Modell und Datei (gebaut S132)

- **`loop/Saiten.hpp`** (neu): `Saitensatz` (`leer` — je Saite der Ton der
  leeren Saite als MIDI-Nummer, in der Reihenfolge am Instrument; `capo`;
  `wahl`), `Saitenwahl` (`JeKlang` = *Per chord*, `Hand` = *Follow hand*),
  `richteSaiten`, die sechs Vorlagen `stimmungsvorlagen()` und `vorlageVon`.
  Die Vorlage steht nicht in der Datei — sie folgt aus den Tönen (*Custom*,
  wenn keine passt).
- **`Instrument::saiten`**, **`Note::saite`** (−1 = keine),
  **`Muster::saitenwahl`**.
- **`.lvinst` Fassung 10:** Block `strings` mit `open` (Liste der Töne),
  `capo`, `stringChoice` (`perChord` / `followHand`); ohne Saiten kein Block.
- **`.lvloop` Fassung 8:** die Saite als fünfter Wert einer Note (nur wenn
  gesetzt), am Instrument-Muster `stringChoice` (nur wenn *Follow hand*).
- **Von Claude gesetzt, nicht vorgelegt:** die Namen der Felder; Grenzen
  12 Saiten, Capo 12; die Reihenfolge der Saiten am Instrument (Ukulele
  `G4 C4 E4 A4`); zwei Noten gleicher Tonhöhe auf demselben Schritt bleiben
  wie bisher eine (keine Unisono-Paare auf zwei Saiten); dass ein neues Muster
  die Regel seines Instruments bekommt, baut 23f (im Editor).
- ✅ **S-A1 a** (Patrik S132): **Die Planung bekommt eine Auskunft herein**
  (`SaitenVon`: die Saiten eines Instruments beim Namen). `plane` und
  `Loop::klangNamen` rechnen über einen gemeinsamen Plan je Vorkommen; ohne
  Auskunft wie bis Stufe 22. Verworfen: b (Kopie der Saiten am Muster —
  zweite Quelle), c (Nachbearbeitung des Ablaufs bei jedem Aufrufer).
- ✅ **S-T1 a** (Patrik S132): **Die Noten reichen von A0 (21) bis C8 (108)**,
  dem Umfang eines Klaviers (`kHoeheMin`, `kHoeheMax` in `Loop.hpp`; bis
  dahin C1 bis C7). Anlass: die tiefe B-Saite eines fünfsaitigen Basses (B0 =
  23) lag darunter, die hohen Lagen einer Violine (bis A7 = 105) darüber.
  Dateien mit Noten im alten Umfang ändern sich nicht; die Piano-Roll hat 88
  statt 73 Zeilen. Verworfen: b (nur unten bis B0 — die Violine stieße oben an
  dieselbe Grenze), c (Umfang bleibt, Bass 5 fällt weg).

### 5b. Stufe 23b — Saitenwahl und Abdämpfen beim Planen (gebaut S132)

- **`loop/Saitenplan.hpp`** (neu): `greifeNoten` (je Note eines Musters der
  `Griff`: Saite, Bund, greifbar) und `toeneImKanal` (je Vorkommen einer Note
  in einem Kanal die `TonAngabe` mit Saite und Ende).
- **Die Regel** (S-M2b): Noten werden nach ihrem Anschlag gruppiert; wer am
  selben Schritt anschlägt, bekommt verschiedene Saiten, Saiten noch
  gehaltener Noten sind belegt. Bewertet wird der Reihe nach: möglichst alle
  greifbar, kleinster Abstand der gegriffenen Bünde (leere Saiten zählen
  nicht), bei *Follow hand* dann die Nähe zur Lage des vorigen Anschlags (der
  tiefste gegriffene Bund; leere Saiten bewegen die Hand nicht), zuletzt die
  tiefste Lage (kleinste Summe der Bünde). Eine gesetzte, greifbare Saite gilt
  (S-M2 a).
- **Das Ende** (S-M3 a): je Kanal, Muster und Saite endet ein Ton dort, wo
  der nächste auf derselben Saite anschlägt — über Wiederholungen, Clips und
  das Ende des Durchlaufs hinweg (der Loop wiederholt sich; ein Ton allein auf
  seiner Saite endet einen Durchlauf später). Die letzten `kAbdaempfenMs`
  (5 ms) blendet er aus; beim Anschlag ist er still.
- **Der Name des Tons**: `inst:<instrument>:<hoehe>:<dauer>:s<saite>:e<ende>`;
  ohne Saite und Ende derselbe Name wie bis Stufe 22.
- **Die Auskunft** `saitenAusInstrumenten()` (Instrument.hpp) — Spieler,
  Tonführung des Composers und Messung je Clip reichen sie an `klangNamen`
  und `baueProgramm`.
- **Beleg:** jeder mitgelieferte Loop plant mit der Auskunft genau wie ohne
  (Namen, Stimmen, Schläge) — keines der mitgelieferten Instrumente trägt
  Saiten.
- **Von Claude gesetzt, nicht vorgelegt:** greifbar bis Bund 24
  (`kBuendeMax`); 5 ms Abdämpfen, endend beim neuen Anschlag; eine nicht
  greifbare Note klingt auf der nächstbesten Saite, dämpft keine andere und
  wird nicht gedämpft; die Reihenfolge der Bewertung oben; ein Ende steht im
  Namen auch, wenn der Ton vorher von selbst verklingt (mehr Namen, gleicher
  Klang); die Suche prüft je Anschlag höchstens 200 000 Möglichkeiten.

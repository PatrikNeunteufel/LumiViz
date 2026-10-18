# LumiViz — Saiteninstrumente (Konzept)

> **Version:** 0.28.0
> **Datum:** 2026-10-18 (Session 134; S-W2 a bis S-W9 a)
> **Typ:** Konzept — Ziel, Modell, Klang, Creator. Entschieden sind S-Z1 a,
> S-M1 a, S-M2 a, S-M2b (a und b schaltbar), S-M2c a, S-M3 a, S-K1 (a und
> b schaltbar), S-K2 b, S-K3 a und S-U1 a (Skizze des Creators gebilligt;
> Patrik S132); alles andere ist offen oder Vorschlag und so bezeichnet.
> Die Baustufen (§5) sind freigegeben (S-B1 a); gebaut sind 23a (§5a), 23b
> (§5b), 23c (§5c), 23d (§5d), der Korpus Parlor (§5e) und der Korpus
> Classical guitar (§5f, S-K10 a, S133); 23e, der Creator: die Zeile Tuning
> (§5g), Model, String, Body (§5h), das Saitenbild (§5i) und die Gruppe
> Pickups (§5j); 23f, der Instrument-Editor (§5k); von 23g die Umstellung der
> mitgelieferten Instrumente (§5l).
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

✅ **S-W1 a** (Patrik S133; Bearbeitung ab der nächsten Session): **Ziel der
Spielweisen** sind Strum (Richtung, Versatz der Saiten, Muster) und die
Artikulationen je Note — Palm mute, Let ring, Tapping, Harmonics, Hammer-on /
Pull-off, Slide, Bend. Gebaut in zwei Stufen: zuerst Strum und was nur den
einzelnen Ton ändert (Palm mute, Let ring, Tapping, Harmonics), danach, was zwei
Töne auf einer Saite verbindet (Hammer-on / Pull-off, Slide, Bend — die Planung
rechnet dafür zwei Noten zu einem Ton). Verworfen: b (nur Strum, Palm mute,
Tapping). Als Nächstes einzeln: wo Strum eingestellt wird, wie eine Artikulation
in der Piano-Roll gesetzt wird (Skizze), dann die Tabs.

✅ **S-W2 a** (Patrik S134): **Strum steht am Muster als Vorgabe, jeder Akkord
kann abweichen.** Das Muster trägt *Strum*: das Richtungsmuster (etwa
Down-Up-Down-Up) und *Spread*, den Versatz der Saiten in ms; ein neues Muster
bekommt beides vom Instrument (wie *String choice*, S-M2c a). Ein Akkord — die
Noten, die am selben Schritt anschlagen, wie im `Saitenplan` — kann in
Richtung (Down, Up, Together) und Versatz abweichen; ohne Abweichung folgt er
dem Muster. Die Tabs setzen die Richtung je Akkord. Folge: `.lvloop` Fassung 9.
Der Klang von Down und Up gehört dem Instrument (eigene Frage). Verworfen: b
(nur am Muster — die Tabs hätten keinen Platz für ihre Angabe, das Format wäre
zweimal zu ändern), c (nur am Akkord — ein gleichmäßiges Schlagmuster Akkord
für Akkord zu setzen), d (am Instrument — bindet die Musik an den Klang, wie
S-M2c b). Als Nächstes einzeln: wonach das Richtungsmuster geht (Lage im Takt
oder Reihe der Akkorde), dann wo die Abweichung je Akkord gespeichert wird.

✅ **S-W3 a** (Patrik S134): **Das Richtungsmuster ist ein Muster der Hand über
die Stellen eines Takts.** Das Muster trägt ein *Hand*-Raster (Schlag, halber
Schlag, Schritt) und je Stelle eines Takts *Down* oder *Up*; Vorgabe
abwechselnd D U D U, jede Stelle umstellbar (etwa D D U U D U). Ein Akkord
nimmt die Richtung der Stelle, auf der er anschlägt; das Muster wiederholt sich
je Takt. Einfügen oder Löschen eines Akkords ändert die Richtung der anderen
nicht. Verworfen: b (Folge über die Reihe der Akkorde — jede Einfügung kippt
alle folgenden), c (nur abwechselnd — nimmt das Einstellen aus der Idee S127).
**Von Claude gesetzt, nicht vorgelegt:** ein Akkord zwischen zwei Stellen nimmt
die vorige; Vorgabe des Rasters der halbe Schlag (Achtel bei 4/4).

✅ **S-W4 a** (Patrik S134): **Die Abweichung eines Akkords steht in einer
Liste am Muster, nach Schritt.** Das Muster trägt *Anschläge*: je Eintrag
Schritt, Richtung (Down, Up, Together) und Versatz in ms (−1 = wie das
Muster), nur für abweichende Akkorde. Wer die Noten eines Schritts verschiebt,
kopiert oder dreht, nimmt den Eintrag mit — `kopiereNoten`, `fuegeNotenEin`,
`loescheNoten`, `versetzeNoten`, `dreheNoten` (`Auswahl.cpp`), `setzeRaster`
und die Kopie beim Verlängern (`Bearbeiten.cpp`), das Ziehen einer Note im
Instrument-Editor; ein Eintrag ohne Note an seinem Schritt fällt in
`Loop::richte` weg. In der `.lvloop` (Fassung 9) ein eigener Block am Muster.
Trennung: was die Hand betrifft (Strum), steht am Zeitpunkt; was eine Saite
betrifft (Palm mute, Let ring, Tapping, Harmonics), an der Note. Verworfen: b
(an jeder Note — derselbe Wert mehrfach, Widersprüche über eine verdeckte
Regel), c (an der tiefsten Note — hängt an einer Note, die man leicht
entfernt).

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
- ✅ **S-K4 a** (Patrik S132): **Die Werte der Korpusse kommen aus
  veröffentlichten Messungen** — je Resonanz Frequenz, Güte, Stärke mit
  Quelle (Seite oder Abbildung, Wortlaut) und Reichweite der Suche; die
  Tabelle kommt vor dem Bau des Korpus zur Vorlage. Ein Korpus ohne Quelle
  bleibt aus der Auswahl. Die Saite ohne Korpus darf vorher gebaut werden.
  Verworfen: b (nach Gehör — ein Name, der nichts belegt), c (Impulsantworten
  als Hauptweg — Aufnahmen Dritter; zum Ablesen mit geklärter Lizenz erlaubt).
- ✅ **S-K5 a** (Patrik S132, auf seine Frage nach E-Gitarre, E-Bass,
  E-Violine): **Body und Tonabnehmer sind unabhängig.** Eine eigene Gruppe
  **Pickups** (nur Waveguide) mit **bis zu zwei** Tonabnehmern — je *Type*
  (None/akustisch, Single coil, Humbucker, Piezo), *Position* und die
  Resonanz der Bauart aus Messwerten (Verfahren wie S-K4 a) — und *Pickup
  select* (Neck, Both, Bridge). Solid Body = *Body None* + Pickups,
  Semi-Hollow = *Body* + Pickups, akustisch = *Body* ohne Pickup, E-Violine =
  *Body None* + Piezo. Verstärker und Box bleiben *Drive* und *Filter* in
  Signal path und am Clip. **Die Skizze der Gruppe kommt vor 23d**; sie
  ersetzt in §4 den Regler *Pickup position* und die eine Marke *pickup*.
  Verworfen: b (ein Tonabnehmer — Datei und Oberfläche zweimal ändern), c
  (nur Filter im Signal path — der folgt der Tonhöhe, die Resonanz eines
  Tonabnehmers liegt fest in Hertz).
- ✅ **S-K6 a** (Patrik S132): **Vor dem Festlegen der Tabelle eine zweite,
  gezielte Recherche** nur für die Lücken der ersten (Güte des Kontrabasses,
  Stärken von Violine und Steelstring, eine Dreadnought, Tonabnehmer belastet
  je Bauart, reale Lagen); danach die Tabelle erneut zur Vorlage. Stand der
  ersten Recherche: §6. Verworfen: b (Lücken als markierte Annahmen — die
  Hintertür zu S-K4 b), c (nur die Classical guitar — vier von fünf fallen
  weg, obwohl es für ihre Lücken wahrscheinlich Quellen gibt).
- ✅ **S-U3 a** (Patrik S132): **Skizze der Gruppe Pickups gebilligt** (§4)
  und **eine Mensur am Instrument**: *Scale* in mm in der Zeile Tuning, jede
  Vorlage bringt die übliche mit (je mit Quelle der Herstellerangabe).
  Pluck position und die Position eines Tonabnehmers zeigen **mm vom Steg**
  und rechnen intern als Anteil; der Humbucker tastet an **zwei Stellen 18 mm
  auseinander** ab. Verworfen: b (Prozent, Humbucker auf feste 648 mm
  bezogen — stimmt nur für eine Gitarre), c (Humbucker als eine Abtaststelle
  — *Type* wäre nur ein Name für eine Resonanz).
- ✅ **S-K7 a** (Patrik S132): **Abgeleitet zählt als belegt**, wenn Rechenweg
  oder Abbildung mit Fundstelle dabeistehen; was ganz fehlt, bleibt draußen.
  In die Auswahl kommen *Classical guitar*, *Acoustic guitar* (als Parlor
  benannt) und alle drei Tonabnehmer; *Violin*, *Upright bass*, *Acoustic
  bass* bleiben draußen, bis ihre Lücke belegt ist. Mensuren der Vorlagen nach
  §6b (Händlerangaben als Herstellerdaten; Ukulele Konzert 15″ = 381 mm aus
  der Größenklasse). Verworfen: b (nur Wörtliches — der Humbucker ohne
  Resonanz, obwohl seine Güte wörtlich belegt ist), c (markierte Annahmen für
  Violine und Kontrabass — die Hintertür aus S-K6).
- ✅ **S-K8 a** (Patrik S132): **Der Korpus ist das gekoppelte Modell der
  Quellen** — Decke und Luft als gekoppelte Schwinger, getrieben von der Kraft
  am Steg, mit genau den Parametern der Quelle: Parlor nach
  Christensen/Vistisen mit Hess Tab. 1 (alle wörtlich), Classical nach Wrights
  Modell (Kap. 4, Anhang A). Der Test misst die gekoppelten Spitzen gegen die
  gemessenen. Verworfen: b (Resonanzen aus den Kurven abgelesen — Güte und
  Höhe mit ±10 %), c (Wrights entkoppelte Moden ohne Luft — die tiefste
  Spitze am falschen Ort). **Befund beim Lesen:** Wrights Modell braucht für
  die Luft das Volumen V und die Schalllochfläche A_h (Gl. A.5–A.7); für
  seine Gitarre BR2 nennt er beides in der Textebene nicht (nur f_h = 122 Hz,
  Q_h = 45, S. 97, und nach Huber 83–89 mm Schalllochdurchmesser) — die
  Klassikgitarre ist vorzulegen.
- ✅ **S-K9 a** (Patrik S132, bearbeitet S133): gezielte Suche nach V und A_h
  — in Wrights Thesis am Seitenbild (Quelle im Repo:
  [quellen/Wright_1996_Acoustics_Psychoacoustics_Guitar.pdf](quellen/Wright_1996_Acoustics_Psychoacoustics_Guitar.pdf),
  von hakwright.co.uk). **Befund:** wörtlich nur für den Satz TOP26, S. 138:
  „A_h = 5.7 × 10⁻³ m² and V = 14 × 10⁻³ m³“ (Schalllochdurchmesser daraus
  85 mm, in Hubers Bereich). Für BR2 auch im Bild nicht (S. 96: „The area of
  the soundhole is easily measured“, ohne Zahl). Christensen 1984 und Huber
  1991 waren danach nicht mehr nötig.
- ✅ **S-K10 a** (Patrik S133): **Classical guitar nach Wrights Satz TOP26** —
  elf Moden aus Tab. 7.1 (S. 140; Decke aus der Finite-Elemente-Rechnung einer
  2,6 mm starken, verstrebten Fichtendecke nach Walker 1991, dazu B(1,1)),
  Luft f_h = 133 Hz, Q_h = 30, A_h und V von S. 138. Der Test misst die
  gekoppelten Spitzen gegen Wrights Abb. 7.1 (S. 139: 112 / 195 / 215 / 435 /
  505 / 674 / 880 Hz). Verworfen: b (BR2 mit A_h und V aus TOP26, als
  übertragen markiert — eine Mischung zweier Sätze, und auch die Luft von BR2
  ist bei Wright nur angenommen), c (beide als Vorlagen — verdoppelt die
  Mischung von b).
- ✅ **S-K11 a** (Patrik S133, nach S-P2 a: zuerst messen): **Ein magnetischer
  Tonabnehmer ist so laut wie die Saite am Steg** (Effektivwert im ersten
  Zehntel); seine Spitze darf über der akustischen liegen, der fertige Ton
  bei Lautstärke 1 höchstens bei 0,9 — nur wo der Abgleich das überschritte,
  wird darauf gedeckelt. Ersetzt die von Claude gesetzte Grenze „höchstens
  so spitz wie am Steg“ (§5d). Verworfen: b (die Grenze behalten — bei E2
  8–12 dB leiser), c (ohne Grenze — Spitzen bis +7 dBFS, ein Begrenzer auf
  jedem Weg des Loops ist nicht geprüft).
- ✅ **S-U2 a** (Patrik S133): **Die Skizze des Instrument-Editors, A bis G**
  — alles nur, wenn das Instrument eine Tuning hat:
  - **A** *String choice* (Per chord / Follow hand) in der Instrument-Zeile
    hinter *Load instrument...*; es stellt `Muster::saitenwahl` dieses
    Musters.
  - **B** ein Punkt auf den Tasten der leeren Saiten (mit Capo: der klingende
    Ton).
  - **C** jede Note trägt die Nummer ihrer Saite im Balken.
  - **D** eine Note mit gesetzter Saite (`Note::saite`) trägt eine Nadel vor
    der Nummer; eine automatische nicht.
  - **E** eine nicht greifbare Note ist rot schraffiert mit „!“ vor der
    Nummer; der Tooltip nennt die Saite, auf der sie trotzdem klingt.
  - **F** im Rechtsklick-Menü (Copy, Cut, …) das Untermenü *String*: „Auto
    (now n)“ und je Saite „n · E4“, die Saiten, auf denen der Ton nicht
    greifbar ist, grau; es gilt für die markierten Noten (mit Undo). Steht
    „Right-click deletes“ an, kommt man über das Menü nicht hin.
  - **G** unten die Hinweiszeile: „number = string · pin = set by you · ! =
    not playable, sounds on the nearest string · right-click a note: String“.
  - Verworfen: b (nur A, C, E — die Saite je Note, die das Modell seit 23a
    trägt, ließe sich nur in der Datei setzen und sähe aus wie eine
    automatische).
- ✅ **S-U2b a** (Patrik S133): **Gezählt wird wie am Instrument üblich** —
  die letzte Saite der Liste (`Saitensatz::leer`) ist 1, die erste trägt die
  höchste Nummer: Gitarre hohe E-Saite 1, tiefe 6; Bass G-Saite 1; Violine
  E-Saite 1; Ukulele A-Saite 1. Jedes Feld der Zeile Tuning nennt seine
  Nummer im Tooltip („String 6“). Verworfen: b (1 = die erste der Liste —
  gegen die Sprache der Spieler und die späteren Tabs).
- ✅ **S-W5 a** (Patrik S134): **Die Skizze der Spielweisen im
  Instrument-Editor, H bis M** — an A bis G anschließend, alles nur mit Tuning
  am Instrument (nach S-W2 a, S-W3 a, S-W4 a):
  - **H** Zeile *Strum* unter der Instrument-Zeile: *Spread* (ms), *Hand*
    (Schlag, 1/2 beat, Schritt) und je Stelle eines Takts ein Feld ↓ oder ↑;
    ein Klick dreht es um, ein umgestelltes ist hinterlegt; *Alternate* setzt
    alle auf ↓ ↑ ↓ ↑ zurück. Nicht „Pattern“ — so heißt im Editor das Muster
    (*Play pattern*).
  - **I** die Noten eines Akkords beginnen in der Piano-Roll um den Versatz
    versetzt (Down: die tiefe Saite zuerst, Up: die hohe); ein feiner Strich am
    Anschlag zeigt die Richtung.
  - **J** Spur *Strum* unter *Note vol.*: je Akkord ein Pfeil — grau aus der
    Hand, blau mit Nadel von dir gesetzt (ein Eintrag in *Anschläge*), „≡“ für
    Together; ein Klick schaltet Follow hand → Down → Up → Together; eine
    einzelne Note zeigt einen Punkt.
  - **K** Artikulationen als Kürzel im Balken hinter der Nummer der Saite:
    PM (Palm mute), LR (Let ring), T (Tapping), ◇ (Harmonic).
  - **L** im Rechtsklick-Menü der Noten neben *String*: Untermenü *Strum*
    (Follow hand, Down, Up, Together, Spread...) für die Akkorde der markierten
    Noten; Haken *Palm mute*, *Let ring*, *Tapping*, *Harmonic* für die
    markierten Noten, mit Undo.
  - **M** die Hinweiszeile nennt Pfeile und Kürzel.
  - Verworfen: b (Artikulationen in einer eigenen Spur nach Schritt — sagt im
    Akkord nicht, welche Note gemeint ist; gegen S-W4 a). Passagen setzt man
    mit Mehrfachauswahl und Haken.
  - Offen, eigene Fragen: der Klang von Down/Up und der Artikulationen am
    Instrument (→ S-W6 a); Stufe 2 (Hammer-on / Pull-off, Slide, Bend).
- ✅ **S-W6 a** (Patrik S134): **Je Spielweise eine Abweichung der vorhandenen
  Größen, einstellbar am Instrument.** Die Saiten-Schicht (*Extended KS*,
  *Waveguide*) bekommt im Creator eine Gruppe *Playing*: je Spielweise wenige
  Regler, jeder verstellt genau eine Größe wie in S-K3 a (etwa *Up level* — das
  Vorzeichen des Anschlags dreht sich immer um —, *Mute* für Palm mute als
  Dämpfung des Handballens am Steg, *Tap level* mit dem Bund als Ort und weichem
  Anschlag, *Harmonic level*); Vorgaben aus Messwerten oder Literatur, je
  Herleitung belegt. Andere Schichten des Instruments (etwa der Thud der Muted
  Guitar) klingen bei jeder Spielweise unverändert. Die Spielweise kommt in den
  Namen des Tons; `.lvinst` Fassung 11. Verworfen: b (feste Beträge im Programm
  — ein Bass klingt gedämpft länger als eine Gitarre; b ist a's Vorgabe), c
  (eigene Schichten je Spielweise — Kopien der Saite, Ring time oder Korpus
  wären in jedem Satz nachzuziehen).
  **Von Claude gesetzt, nicht vorgelegt:** *Let ring* ist Planung, nicht Klang
  — die Note klingt bis zur nächsten auf derselben Saite statt bis zu ihrem
  Ende; *Classic* bekommt keine Gruppe *Playing* (dort wirken nur Strum und Let
  ring); Regler und Vorgaben im Einzelnen mit der Skizze des Creators.
- ✅ **S-W7 a** (Patrik S134): **Harmonic: natürliche und künstliche
  Flageoletts, die natürlichen zuerst.** Die Piano-Roll zeigt den klingenden
  Ton. Für eine Harmonic-Note sucht die Saitenwahl zuerst ein natürliches
  Flageolett (leere Saite, Teilton n = 2 … 6, das kleinste n zuerst), sonst ein
  künstliches (gegriffen auf Bund f, Knoten 12 Bünde darüber: Tonhöhe = leere
  Saite + Capo + f + 12, greifbar bis f + 12 ≤ `kBuendeMax`); danach bewertet
  wie bisher (Abstand der Bünde, bei *Follow hand* die Nähe zur Hand). Was sich
  so nicht bilden lässt, ist nicht greifbar (rot, „!“) und klingt als normaler
  Ton auf der nächstbesten Saite. Gerechnet wird der echte Teilton (n = 5:
  −14 Cent). Die Tabs zeigen später den Griff („<12>“), die Piano-Roll den
  Klang — beide lesen dieselbe Saite und denselben Knoten. Verworfen: b (nur
  natürliche — zwei Drittel der Töne ab E3 nicht greifbar), c (die Note ist der
  Griff — eine Note sähe aus wie E3 und klänge wie E4).
  **Von Claude gesetzt, nicht vorgelegt:** n bis 6; künstlich nur n = 2, keine
  Pinch harmonics; eine gesetzte Saite gilt, wenn die Note auf ihr als
  Flageolett entsteht.
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
- **Nachtrag S-U3 a — Zeile Tuning:** dazu **Scale** (die Mensur in mm),
  vorbelegt von der Vorlage.
- **Nachtrag S-U3 a — Gruppe Pickups** (nur bei *Model: Waveguide*, unter
  *Body*; ersetzt den Regler *Pickup position* und die eine Marke *pickup*):
  - im Saitenbild je Tonabnehmer eine Marke (*neck*, *bridge*), ein Humbucker
    als zwei Striche; die Marken lassen sich ziehen;
  - **Pickup select**: *Neck* / *Both* / *Bridge* (Knopfreihe);
  - je Tonabnehmer (*Neck*, *Bridge*): **Type** — *None (acoustic)*, *Single
    coil*, *Humbucker*, *Piezo* — und **Position** in mm vom Steg; bei *Piezo*
    steht die Position fest am Steg (grau);
  - **Cable** in pF, für beide gemeinsam; daneben die Resonanz, die daraus mit
    der Induktivität der Bauart folgt (≈ kHz);
  - Steht ein Tonabnehmer auf *None*, gilt *Pickup select* für den anderen.

### 4b. Spielweisen im Creator (Skizze gebilligt, S-W8 a, Patrik S134)

Nach S-W2 a und S-W6 a; die Zahlen der Skizze waren Platzhalter, die Vorgaben
werden beim Bau hergeleitet und belegt (wie S-K4 a).

- **N** Zeile **Strum** unter der Zeile Tuning — dasselbe Element wie H im
  Instrument-Editor (*Spread*, *Hand*, ↓/↑ je Stelle, *Alternate*), mit dem
  Hinweis „default for new patterns“. Die Vorgabe gehört dem Instrument, nicht
  der Schicht, wie *String choice*.
- **O** Gruppe **Playing** unter *Pickups*, einklappbar, nur bei *Extended KS*
  und *Waveguide*; je Spielweise ein Regler, daneben in Klartext, was er
  bewirkt:
  - *Up stroke*: **Up level** („starts from the other side“);
  - *Palm mute*: **Mute** — Dämpfung des Handballens am Steg; daneben die
    Klingdauer, die daraus folgt;
  - *Tapping*: **Tap level** — angeschlagen am Bund;
  - *Harmonic*: **Touch** — wie fest der Finger auf dem Knoten liegt (statt
    „Harmonic level“ aus S-W6 a: die Lautstärke des Flageoletts folgt aus der
    Physik, der Finger ist die verstellbare Größe);
  - *Let ring* ohne Regler; *Classic* ohne Gruppe.
- **P** Auswahl **Play as** unter der Klaviatur: Normal, Up stroke, Palm mute,
  Tapping, Harmonic, Strum chord (der Akkord der leeren Saiten mit der
  Strum-Vorgabe); ein Tastenklick spielt den Ton so.
- Verworfen: b (ohne *Play as* — einen Regler drehen, ohne ihn zu hören).
- **Von Claude gesetzt, nicht vorgelegt:** die Namen *Up level*, *Mute*,
  *Tap level*, *Touch*; *Play as* gilt für die ganze Klaviatur und wird nicht
  gespeichert.

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
| 23g | **Mitgelieferte Instrumente:** Vorschlag, welche auf die neuen Verfahren umgestellt werden (vorher/nachher als Hörproben, eigene Vorlage); Handbuch, Sichttest-Liste — **der volle Lauf erst, wenn Tabs mit Akkorden möglich sind** (Patrik S133) | `test_LoopWerte` neu geschrieben nur für die umgestellten |

**Stufe 24 — Spielweisen, erste Stufe** (✅ **S-W9 a**, Patrik S134: in
Schichten wie Stufe 23; die Dateien ändern sich einmal, vollständig). Verworfen:
b (je Spielweise durchgehend — vier Fassungen oder Felder auf Vorrat). Je
Teilschritt eng nach S-P1 a, am Ende der Stufe der Bereichslauf; der volle Lauf
weiter erst mit Tabs und Akkorden. Danach Stufe 25 (Hammer-on / Pull-off,
Slide, Bend) mit eigenen Fragen, dann die Tabs.

| Stufe | Inhalt | Beleg |
|---|---|---|
| 24a | **Modell und Datei:** am Muster die Strum-Vorgabe (*Spread*, *Hand* mit Raster und ↓/↑ je Stelle) und *Anschläge* (S-W4 a); an der Note die Artikulationen; am Instrument die Strum-Vorgabe; an der Schicht *Playing*; `.lvloop` **Fassung 9**, `.lvinst` **Fassung 11**; die sieben Stellen, die Noten bewegen, nehmen den Eintrag mit, `Loop::richte` räumt verwaiste ab | Rundweg der Dateien; jede mitgelieferte Datei liest und schreibt sich gleich; je Stelle ein Fall „Eintrag wandert mit“; `test_LoopWerte` unverändert |
| 24b | **Planung:** Richtung je Akkord (Hand, sonst Eintrag), Versatz je Saite in Anschlagreihenfolge, *Let ring*, die Saitenwahl für *Harmonic* (S-W7 a); Richtung und Spielweise im Namen des Tons | Fälle ohne Qt-Anwendung je Regel; Noten ohne Spielweise und Instrumente ohne Saiten Bit für Bit wie vorher (`LUMIVIZ_LOOP_WERTE_GENAU`) |
| 24c | **Klang** in *Extended KS* und *Waveguide*: Up stroke, Palm mute, Tapping, Harmonic; Vorgaben hergeleitet, mit Quelle | Messungen am gerechneten Ton (Vorzeichen, Klingdauer bei *Mute*, fehlende Teiltöne bei *Harmonic*, Ort des Anschlags beim Tapping); Hörproben |
| 24d | **Creator** nach §4b (N bis P) | Bilder aus den Tests |
| 24e | **Instrument-Editor** nach S-W5 a (H bis M) | Bilder aus den Tests |
| 24f | **Mitgelieferte Instrumente und Loops:** Vorgaben für Strum und *Playing* je Saiteninstrument, Beispiel-Loops mit Strum und Artikulationen, Handbuch, Sichttest-Liste | Bereichslauf unter beiden Compilern; `test_LoopWerte` nur für die geänderten neu geschrieben |

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

### 5c. Stufe 23c — die Saite *Extended KS*, ohne Korpus (gebaut S132)

- **`SaitenModell`** an der Schicht: *Classic* (unberührt, S-K2 b) und
  *Extended KS*; der Waveguide folgt mit 23d. Regler nach S-K3 a: Pluck
  position, Pluck, Ring time, High damping, Stiffness, String variation.
  In der `.lvinst` (Fassung 10) nur an einer Saite *Extended KS*:
  `stringModel` `extendedKS`, `pluckPosition`, `pluck`, `ringTime`,
  `highDamping`, `stiffness`, `stringVariation`.
- **Die Rechnung** (`rechneSaiteKs`): der Stoß aus derselben festen Folge wie
  Classic, durch die Härte zweimal tiefpassgefiltert (Grenze vom Doppelten bis
  zum Achtzigfachen des Grundtons), an der Anschlagstelle gekämmt, ohne
  Gleichanteil, so laut wie Classic. Im Umlauf ein Tiefpass erster Ordnung
  mit Verstärkung — der Grundton fällt in *Ring time* um 60 dB, Töne bei
  2 kHz in *Ring time* · 10^(−2 · High damping) — und acht Allpässe, so
  eingestellt, dass der achte Teilton bei der Inharmonizität B = 0,001 ·
  Stiffness² liegt. Die Verzögerung ist um die Laufzeit der Filter beim
  Grundton kürzer; die Verstärkung bleibt bei jeder Frequenz unter 1.
  *String variation* gibt einer tieferen Saite bis zu +0,4 High damping und
  +0,4 Stiffness.
- **Gemessen** (`test_LoopSaiteKs`, Release-clang): Ring time 1 s bei E2
  1,000 s, 3 s bei E5 2,68 s; Pluck position ¼: Teilton 4 −25 dB gegen 3;
  High damping 0,9 gegen 0,1: Teilton 7 −88 gegen −6 dB; Pluck weich gegen
  hart −29 gegen −5 dB; Stiffness 1: Teilton 8 +52,75 Cent (berechnet +52);
  String variation: tiefe E-Saite 12 dB dunkler als die D-Saite; die Tonhöhe
  auf 3 Cent. Hörproben: `LUMIVIZ_23C_WAV=<Ordner>`. **Patriks Ohr (S133):**
  „passt“.
- **Von Claude gesetzt, nicht vorgelegt:** die Bezugsfrequenz 2 kHz und die
  Spanne 10^(−2 · High damping); B = 0,001 · Stiffness²; acht Allpässe; die
  Grenzen der Härte (2 bis 80 · Grundton); +0,4 je Regler durch String
  variation; die Vorgaben 0,13 / 0,65 / 3,2 s / 0,3 / 0,1 / 0,5 aus der
  Skizze. Eine erste Fassung (Mitteln wie Classic, vier Allpässe) wirkte kaum
  und wurde bei hohen Tönen instabil — verworfen, bevor etwas gespeichert war.

### 5d. Stufe 23d, erster Teil — Waveguide, Mensur, Tonabnehmer als Abtaststellen (gebaut S132)

- **`SaitenModell::Wellenleiter`**, in der Datei `stringModel` `waveguide`;
  Verlust und Steifheit teilt er mit *Extended KS* (`entwerfeUmlauf`, ein
  Umlauf für beide; Extended KS rechnet wie vorher, gemessen dieselben Werte).
- **Die Rechnung** (`rechneSaiteWg`): ein Umlauf = zweimal die Saite. Die
  Auslenkung an der Stelle x (Anteil vom Steg) ist die Summe der Welle, die
  vor x · L/2 vom Steg losging, und der, die vor (2 − x) · L/2 losging und am
  Sattel umgedreht wurde. Anschlag: ein Dreieck mit der Spitze an der
  Anschlagstelle, durch die Härte gerundet.
- **Tonabnehmer** (`Abnehmer`, `AbnehmerArt`, `AbnehmerWahl`): ohne
  Tonabnehmer die Welle am Steg. Single coil: die Geschwindigkeit an einer
  Stelle; Humbucker: an zwei, 18 mm auseinander, je zur Hälfte; Piezo: die
  Kraft am Steg durch einen Hochpass erster Ordnung bei 106 Hz (Zollner 6.2).
  *Pickup select*: Neck, Both (je zur Hälfte), Bridge; steht einer auf None,
  gilt der andere. In der Datei `pickupNeck`, `pickupBridge` (je `type`,
  `position` in mm vom Steg), `pickupSelect`.
- **Mensur** `Saitensatz::mensurMm`, in der Datei `scale` (nur wenn gesetzt);
  ohne sie 648 mm.
- **Gemessen** (`test_LoopSaiteWg`, Release-clang): Ring time 2 s bei E2
  2,00 s, bei E5 1,85 s; Anschlag bei ¼: Teilton 4 −42 dB gegen 3; Single
  coil bei ⅕: Teilton 5 −46 dB gegen 4; Humbucker gegen Single coil: Teilton
  3 −0,3 dB, Teilton 36 (halbe Wellenlänge 18 mm bei 648 mm) −44 dB; Neck
  gegen Bridge: Teilton 9 gegen 1 −18 gegen −3 dB; Piezo beim Grundton E1
  −8,81 dB (Hochpass berechnet −8,82 dB).
- **Zweiter Teil (gebaut S132, nach S-K7 a): Cable.** `Schicht::kabelPf`
  (100 … 2000 pF, Vorgabe 500; in der Datei `cable`); je magnetischem
  Tonabnehmer ein Tiefpass zweiter Ordnung mit der Resonanz aus
  `abnehmerResonanz`: die gemessene Resonanz der Bauart bei ihrer
  Bezugskapazität, umgerechnet mit f ∝ 1/√C, die Güte bleibt — Single coil
  3,5 kHz bei 600 pF, Q ≈ 1,58 (+4 dB, Zollner Abb. 5.5.9 abgelesen);
  Humbucker ≈ 3 kHz bei 450 pF (Abb.), Q 1,83 (Zollner/Zwicker 2019,
  wörtlich). Eine Schaltung aus den Bauteilwerten (Strat 2,2 H, 6 kΩ an
  111 kΩ, 600 pF) ergäbe 4,5 kHz statt der gemessenen 3,5 kHz — Wicklung und
  Wirbelströme fehlen ihr —, deshalb die Messwerte. Gemessen: Teilton um
  5 kHz gegen den Grundton bei 300 pF −37 dB, bei 1500 pF −54 dB.
- **Mensuren der Vorlagen** (`Stimmungsvorlage::mensurMm`): Gitarre und
  Drop D 648, Bass 4 und Bass 5 864, Ukulele 381, Violine 327 mm (§6b).
- **Von Claude gesetzt:** die Bezugslasten der beiden Bauarten sind
  verschieden (111 kΩ und 333 kΩ) — so stehen sie in den Quellen; die
  Wicklungskapazität ist nicht eigens gerechnet (sie steckt in den
  gemessenen Bezugswerten).
- **Von Claude gesetzt, nicht vorgelegt:** ein magnetischer Tonabnehmer ist so
  bemessen, dass er im ersten Zehntel einer Sekunde so laut ist wie die Saite
  am Steg (Effektivwert), aber höchstens so spitz (**ersetzt durch S-K11 a**,
  unten) — der Anfang wird dafür
  einmal vorab gerechnet (eine erste Bemessung nach dem Grundton ergab bei
  tiefen Tönen Spitzen bis 3,1); der Anschlag als reines Dreieck ohne
  Rauschen; die Lagen werden auf 0,2 % … 99,8 % der Saite begrenzt; die
  Vorgaben Neck 160 mm, Bridge 40 mm, beide None, Both.
- **Patriks Ohr (S133):** „etwas leise“. **Gemessen** an den Hörproben
  (`LUMIVIZ_23D_WAV`, neu gerechnet in `hoerproben_23d_s133`, gleich wie die
  aus S132): akustisch RMS −26,4 dBFS, Piezo −28,0, die magnetischen −43,6
  (Humbucker Bridge) bis −46,3 dBFS (Single Neck) — **17–20 dB leiser**.
  **Gemessen** (S-P2 a; Messfall „Loop Saite WG: Messung — die Bemessung der
  Tonabnehmer“, meldet nur Zahlen, Noten 28–76) — zwei Ursachen:
  1. **Ein Fehler (behoben S133):** die Geschwindigkeit begann mit einem
     Sprung um die ganze Auslenkung (die Saite startet ausgelenkt, die
     Stelle „davor“ stand auf 0); nach ihm richtete sich die Bemessung, im
     fertigen Ton dämpfte ihn der Anstieg weg. Jetzt ruht die Saite beim
     Loslassen. Die Tonabnehmer wurden dadurch 10–15 dB lauter (Note 40,
     Single Bridge: Anfang −23,2 → −12,4 dB gegen akustisch).
  2. **Die Regel „höchstens so spitz“:** danach liegt die Spitze jedes
     magnetischen Tonabnehmers genau auf der akustischen (−9,2 dBFS), der
     Effektivwert im ersten Zehntel aber darunter — E2 −8,3 bis −12,4 dB,
     E3 −5,6 bis −8,6 dB, E4 −5,0 bis −8,0 dB, E1 bis −16,3 dB. Ein reiner
     Abgleich des Effektivwerts ergäbe Spitzen bis +3,2 dBFS bei E2 und
     +7,1 dBFS bei E1 (bis 16 dB über der akustischen). Piezo: −0,2 bis
     −7,2 dB, der Hochpass bei 106 Hz (gewollt). Zur Vorlage: S-K11.
- **Die Bemessung nach S-K11 a (gebaut S133):** gleich laut wie die Saite am
  Steg (Effektivwert im ersten Zehntel), die Spitze des fertigen Tons bei
  Lautstärke 1 höchstens `kSpitzeSaiteMax` = 0,9 (an der Abnahme 1,8, weil
  danach `kPegel` 0,5 folgt). **Gemessen:** ab E3 je Tonabnehmer auf 0,3 dB
  gleich laut wie akustisch; wo die Spitze deckelt, leiser — E2 bis −4,1 dB
  (Single Bridge), E1 bis −8,0 dB; *Both* 1,2–1,5 dB unter einem einzelnen
  (die Hälften addieren sich nicht voll). Hörproben neu in
  `hoerproben_23d_s133`: RMS akustisch −26,4, magnetisch −27,2 bis −29,0,
  Piezo −28,0 dBFS (vorher bis −46,3). Belegt in `test_LoopSaiteWg` („so
  laut wie akustisch, die Spitze höchstens 0,9“).

### 5e. Der Korpus Parlor (gebaut S132, S-K8 a)

- **`KorpusArt::Parlor`** (in der Datei `body` `parlor`, dazu `bodyAmount`):
  Decke und Luft als gekoppelte Schwinger nach Christensen/Vistisen, in der
  Schreibweise von Wright (1996) Gl. A.1–A.7, mit allen zehn Werten aus Hess
  (2013) Tab. 1 (c, ρ, V, Schallloch, Deckenfläche, Masse, Steife, Dämpfung
  der Decke, Masse und Dämpfung der Luft). Getrieben von der Kraft am Steg;
  abgestrahlt wird die Änderung des Volumenflusses (Wright Gl. 4.17).
  Trapezregel, vier Zustände.
- **Wo er wirkt:** auf den Ton am Steg — bei *Extended KS* immer, beim
  Waveguide ohne Tonabnehmer; ein Tonabnehmer nimmt die Saite selbst ab.
  Bemessen wie die Tonabnehmer: im ersten Zehntel einer Sekunde so laut wie
  die Saite allein (Effektivwert); *Body amount* mischt.
- **Beleg** (`test_LoopKorpus`): eingegeben sind nur entkoppelte Werte (Decke
  ≈ 211 Hz, Luft 130 Hz — Hess nennt 130 Hz als Helmholtz-Frequenz); das
  Modell hat seine gekoppelten Spitzen bei **116 und 219 Hz** — Hess hat 116
  und 219 Hz gemessen —, die zweite **+7,2 dB** über der ersten (Hess Abb. 1a
  abgelesen: rund +7 dB). Im Ton: A2 Grundton gegen Teilton 5 ohne Korpus
  +5,6 dB, mit Parlor +12,7 dB.
- **Von Claude gesetzt:** der Korpus hört die Saite einen Abtastwert später
  (die Trapezregel kennt die nächste Kraft noch nicht); die Kraft am Steg ist
  die dort ankommende Welle; Body amount 0,6 als Vorgabe; ein Semi-Hollow
  mit Tonabnehmer hört seinen Korpus noch nicht (der Korpus wirkt nur auf die
  Abnahme am Steg).

### 5f. Der Korpus Classical guitar (gebaut S133, S-K10 a)

- **`KorpusArt::Klassik`** (in der Datei `body` `classical`): dasselbe Modell
  wie der Parlor, jetzt für beliebig viele Kolben (Wright Gl. A.20–A.24) —
  zehn Moden der Decke und B(1,1) aus Tab. 7.1 (TOP26, S. 140), die Luft im
  Schallloch mit f_h = 133 Hz, Q_h = 30, A_h = 5,7 × 10⁻³ m², V = 14 × 10⁻³ m³
  (S. 138). Alle Kolben hängen nur über den einen Druck P im Korpus zusammen;
  der Schritt der Trapezregel löst sich deshalb je Kolben, P aus einer
  Gleichung (zwölf Kolben, kein 24×24-System je Abtastwert). Der Parlor läuft
  über denselben Weg (zwei Kolben) und misst unverändert 116 / 219 Hz,
  +7,25 dB.
- **Aus Wrights Werten abgeleitet** (S-K7 a): die Steife je Mode
  k = m ω² − α A² (seine Frequenzen gelten bei geschlossenem Schallloch,
  Gl. A.12, A.28), die Dämpfung R = m ω / Q, die Masse der Luft
  m_h = α A_h² / ω_h² (Gl. A.14; 0,47 g), α = ρ c² / V; der Boden wird nicht
  von der Kraft am Steg getrieben (Gl. A.20), die Decke an jeder Mode
  (wirksame Massen am Steg). Moden mit Fläche 0 (T(2,1), T(4,1), T(2,2))
  strahlen nicht ab und koppeln nicht.
- **Von Claude gesetzt:** ρ = 1,205 kg/m³ und c = 343 m/s wie beim Parlor
  (Wright nennt beide nicht; ±10 % in α verschieben das Triplett um weniger
  als 2 Hz); die Moden der Decke koppeln über P auch untereinander (Wright
  schreibt nur „a summation is performed“, S. 71 — gerechnet als Summe
  einzelner Drei-Kolben-Systeme liegen die Spitzen auf 0,5 Hz gleich).
- **Beleg** (`test_LoopKorpus`): gekoppelte Spitzen bei **115,4 / 191,3 /
  220,0 / 434,0 / 505,8 / 673,6 / 876,5 Hz** gegen Wrights Abb. 7.1 (S. 139,
  wörtlich in der Unterschrift) 112 / 195 / 215 / 435 / 505 / 674 / 880 Hz —
  das Triplett um 2–3 % auseinander, der Rest unter 0,5 %; geprüft auf ±5 %.
  Pegel gegen T(1,1)₂: −7,5 / 0 / −7,6 / −14,4 / −9,8 / −11,2 / −19,5 dB (in
  Abb. 7.1 abgelesen rund −7 / 0 / −8 / −14 / −10 / −12 / −20 dB; geprüft
  ist nur, dass T(1,1)₂ die stärkste ist). Die Ursache der 2–3 % im Triplett
  ist offen — weder ρ und c noch die Art der Kopplung erklären sie.
- Hörproben `ks_classical`, `wg_classical` unter
  `.claude/handover/hoerproben_korpus/` (lokal). **Patriks Ohr (S133):** „passt
  eigentlich soweit“ — gehört mit Parlor und ohne Korpus.

### 5g. Stufe 23e, Teilschritt 1 — die Zeile Tuning (gebaut S133)

- **`loop/SaitenZeile`** (neu): die Zeile nach §4 — *Tuning* (None, die sechs
  Vorlagen, Custom), *Strings* 1 … 12, je Saite ein Feld mit dem Ton der
  leeren Saite, *Capo*, *Scale*, *String choice*. Im Creator unter *Type /
  Genres / Description*, über Layers und Signal path; sie stellt
  `Instrument::saiten`. Bei *None* steht nur die Auswahl. Ein eigenes
  Fließ-Layout bricht die Zeile um, wenn die Breite nicht reicht (bei 1280 px
  und sechs Saiten passt sie in eine Zeile).
- **Belegt** in `test_LoopCreatorTuning` (drei Fälle; Bilder mit
  `LUMIVIZ_CREATOR_TUNING_BILD`): Namen und Zahlen im Tonfeld, jede
  Bedienung der Zeile, der Weg in den Creator mit Reset, der Umbruch (zwölf
  Saiten: 30 px hoch bei 2400 px Breite, 173 px bei 420 px). Der Fall
  „Regler stellen die Kopie“ in `test_LoopInstrumentEditor` gibt dem Fenster
  540 statt 500 px Höhe (die Zeile braucht 33 px) und prüft die Höhe des
  Inhalts statt des Rollwegs allein.
- **Von Claude gesetzt:** *Custom* lässt sich nicht wählen — es steht da,
  wenn die Töne zu keiner Vorlage passen (die Vorlage folgt aus den Tönen,
  wie in der Datei); eine Vorlage setzt Töne und Mensur, Capo und String
  choice bleiben; *Strings* mehr = neue Saite eine Quarte über der höchsten,
  weniger = die obersten fallen weg; das Tonfeld ist ein Drehfeld mit Pfeilen
  und nimmt getippte Namen (`E2`, `C#4`, `Bb3`, auch klein) oder Zahlen;
  *Scale* zeigt ohne gesetzte Mensur die Vorgabe 648 mm.

### 5h. Stufe 23e, Teilschritt 2 — Model, String, Body (gebaut S133)

- **Im Creator** (`InstrumentCreator`), nur bei *Plucked string*: unter
  *Waveform* die Auswahl **Model** (Classic, Extended KS, Waveguide). Bei
  *Classic* steht *Damping* wie bisher; bei den beiden anderen statt seiner
  die Gruppen **String** (Pluck position, Pluck, Ring time, High damping,
  Stiffness, String variation) und **Body** (Body: None, Acoustic guitar
  (Parlor), Classical guitar; Body amount, grau ohne Korpus), beide
  einklappbar (`string`, `body`). Der Kasten Source nennt das Verfahren:
  „Plucked string · Waveguide“.
- **Belegt** in `test_LoopCreatorSaite` (Bilder mit
  `LUMIVIZ_CREATOR_SAITE_BILD`); in `test_LoopInstrumentEditor` erwartet der
  Kasten Source jetzt „Plucked string · Classic“.
- **Von Claude gesetzt:** Ring time logarithmisch über 1000 Schritte (die
  Mitte ≈ 1,73 s); Pluck position in Prozent der Saite vom Steg, 2–50 %; die
  Auswahl Body nennt nur die gebauten Korpusse; die Werte von String und Body
  bleiben in der Schicht, wenn man auf *Classic* zurückstellt.

### 5i. Stufe 23e, Teilschritt 3 — das Saitenbild (gebaut S133)

- **`loop/SaitenBild`** (neu), die erste Zeile der Gruppe String: Sattel
  links, Steg rechts, die Saite dazwischen, maßstäblich zur Mensur. Die Marke
  **pluck** (ein Dreieck über der Saite, „pluck 13 %“) und bei *Waveguide* je
  Tonabnehmer eine Marke **neck** / **bridge** unter der Saite („neck
  160 mm“; ein Humbucker als zwei Striche 18 mm auseinander, greifbar an
  beiden; ein Piezo fest am Steg, nicht zu ziehen). Ziehen stellt dieselben
  Werte wie die Regler (Pluck position; die Lage in mm vom Steg, auf den
  Millimeter). Über einer Marke wechselt der Zeiger, sie greift ±8 px weit;
  unten steht „drag the marks · scale 650 mm“.
- **Belegt** in `test_LoopCreatorSaite` („das Saitenbild“: Griffe, Zeiger,
  Ziehen mit den Grenzen, Humbucker, Piezo, der Weg in die Schicht; ein Bild
  `_bild` mit `LUMIVIZ_CREATOR_SAITE_BILD`).
- **Von Claude gesetzt:** eine Tonabnehmer-Marke steht nur, wenn der Platz
  einen trägt (Type ≠ None) — Type selbst kommt mit der Gruppe Pickups
  (Teilschritt 4); die Lagen werden wie in der Abnahme auf 0,2 … 99,8 % der
  Saite begrenzt.

### 5j. Stufe 23e, Teilschritt 4 — die Gruppe Pickups (gebaut S133; 23e fertig)

- **Im Creator**, nur bei *Waveguide*, unter Body (einklappbar, `pickups`):
  **Pickup select** als Knopfreihe Neck / Both / Bridge; je Platz **Neck** und
  **Bridge** die Auswahl *Type* (None (acoustic), Single coil, Humbucker,
  Piezo) und *Position* in mm vom Steg (bis zur Mensur); **Cable** in pF und
  darunter die Resonanz, die daraus je Bauart folgt („Resonance: neck ≈
  3.8 kHz · bridge ≈ 2.8 kHz“ bei 500 pF). Das Saitenbild zeigt die Marken
  nach dem Type.
- **Belegt** in `test_LoopCreatorSaite` („die Gruppe Pickups“; ein Bild
  `_pickups` mit String und Body eingeklappt). **Bereichslauf am Ende von
  23e:** 322 / 322 unter Testing-MSVC (300 s) und Release-clang (72 s).
- **Von Claude gesetzt:** *Position* ist grau bei None und Piezo (None hat
  keine Lage, ein Piezo sitzt unter dem Steg); *Pickup select* ist grau,
  solange ein Platz auf None steht — dann klingt der andere allein (wie im
  Modell); *Cable* ist grau ohne magnetischen Tonabnehmer, die Zeile sagt
  es. Die Gruppe Body bleibt bei Tonabnehmern sichtbar, wirkt dann aber
  nicht (§5e: ein Tonabnehmer nimmt die Saite selbst ab).

### 5k. Stufe 23f — der Instrument-Editor zeigt die Saiten (gebaut S133)

- **Nach der Skizze S-U2 a, gezählt nach S-U2b a**, alles nur mit Tuning am
  Instrument (`InstrumentEditor`, `NotenRaster`):
  - A *String choice* hinter *Load instrument...* stellt
    `Muster::saitenwahl` (Undo „Change string choice“);
  - B ein Punkt auf den Tasten der leeren Saiten (mit Capo);
  - C die Nummer der Saite im Balken; die Griffe rechnet `greifeNoten` auf
    einer sortierten Kopie des Musters (während eines Zugs ist es nicht immer
    gerichtet);
  - D eine Nadel, wenn die Note ihre gesetzte Saite spielt;
  - E nicht greifbar: rot, durchscheinend schraffiert, „! 6“ (schmal nur
    „!“); der Tooltip jeder Note nennt Saite und Bund oder die Saite, auf der
    sie trotzdem klingt;
  - F das Untermenü *String* im Menü der Noten: „Auto (now n)“, dann „1 ·
    E4“ … — gesperrt, wo der Ton nicht für alle markierten greifbar ist, mit
    „(not playable)“; Undo „String n“ / „String: Auto“;
  - G die Hinweiszeile über der Zeile der Auswahl.
  - Die Felder der Zeile Tuning nennen ihre Nummer im Tooltip („String 6 —
    …“).
- **Belegt** in `test_LoopEditorSaiten` (zwei Fälle; Bild mit
  `LUMIVIZ_EDITOR_SAITEN_BILD`). **Bereichslauf am Ende von 23f:** 324 / 324
  unter Release-clang (71 s) und Testing-MSVC (309 s).
- **Von Claude gesetzt:** die Nadel nur, wenn die Note ihre gesetzte Saite
  auch spielt — sonst sagt es der Tooltip („set to string n, which cannot
  play it here“); im Menü gilt ein Eintrag als greifbar nur, wenn er es für
  jede markierte Note ist.

### 5l. Stufe 23g — die mitgelieferten Saiteninstrumente (gebaut S133)

- ✅ **S-G1 a** (Patrik S133, mit Zusatz): **alle acht umstellen, die alten als
  „old“ behalten** — guitar, nylon_guitar, clean_guitar, muted_guitar,
  overdrive_guitar, upright_bass, slap_bass, clavinet. Verworfen: b (ohne
  Clavinet), c (keines umstellen, die neuen dazu). Die Saite als Klangfarbe
  (conga, flute, kalimba, broken_radio) bleibt Classic.
- **`asset/loop/make_saiten_instrumente.py`** (neu): legt beim ersten Lauf
  `<name>_old.lvinst` an (die Fassung davor, Titel mit „(old)“, ohne Genre)
  und baut `<name>.lvinst` immer aus ihr — Tuning, Verfahren und Regler aus
  der Tabelle im Skript, `level` mit einem Faktor angeglichen. Zwei Läufe
  ergeben dieselben Dateien. `make_genres.py` und `make_examples.py` schreiben
  die acht als Classic und sagen jetzt, dass danach dieses Skript läuft.
  `instrument_katalog.py`: neue Beschreibungen, die acht `_old`, und
  **`ergaenze` trägt `strings` weiter** (vorher hätte ein Lauf die Tuning
  still verworfen).
- **Gemessen** an den Hörproben (`hoerproben_23g`, je Instrument seine Spur
  aus einem mitgelieferten Loop, Effektivwert): guitar, nylon, clean,
  overdrive, upright, clavinet auf 0,1 dB wie vorher; slap_bass −0,6 dB
  (Pegel begrenzt: A4 muss unter 0,75 bleiben); muted_guitar wäre bei
  `level` 1,0, der Grenze, −3,7 dB — Ring time, Pluck, Both und Extended KS
  änderten daran nichts.
- ✅ **S-G2 a** (Patrik S133): beim muted_guitar die Schicht **Thud**
  (Triangle) von 0,4 auf 0,6 — gemessen −0,5 dB gegen vorher. Das Skript
  setzt andere Schichten über `ANDERE` (nach ihrem Namen). Verworfen: b (so
  lassen, −3,7 dB — die Mischung der Loops kippte).
- ✅ **S-G3 a** (Patrik S133): **sechs neue Saiteninstrumente** — Jazz Guitar
  (Humbucker am Hals), Finger Bass (zwei Single coils, dazu die Sinus-Schicht
  des Upright), Ukulele (Vorlage, 381 mm), Drop D Guitar (Humbucker am Steg,
  Drive), Pizzicato Violin (Vorlage Violin, ohne Body), Piezo Guitar (Piezo am
  Steg); Werte in `NEU` von `make_saiten_instrumente.py`. **Pegel wie die
  vergleichbaren** (dasselbe Muster, Effektivwert der Spur): Jazz und Piezo wie
  die Nylon, Drop D wie die Overdrive, Ukulele wie die Guitar — auf 1 dB; an
  der Grenze `level` 1,0 bleiben **Finger Bass −2,6 dB** (gegen den Upright)
  und **Pizzicato Violin −4,6 dB** (gegen die Guitar; Ring time 1,2 s und
  Pluck 70 % machten sie nicht lauter). Verworfen: b (nur vier), c (erst mit
  den Tabs).
- ✅ **S-G4 a** (Patrik S133): **so lassen** — beim Pizzicato zählt der
  Anschlag (Spitze −16,3 gegen −14,5 dBFS der Guitar, 1,8 dB), nicht der
  Effektivwert über lange Notenwerte; nachgestellt wird nach Patriks Ohr an
  den Beispiel-Loops. Verworfen: b (Sinus lauter, Ring time um 2 s — ändert
  den gehörten Klang, und keine bestehende Mischung kippt).
- **Loops** (Wunsch Patrik S133), `asset/loop/make_saiten_loops.py`:
  - `vergleich/*.lvloop` — bossa, funk, disco, rock_demo, instrument_demo je
    zweimal: zuerst mit den `(old)`, dann mit den neuen; die übrigen Spuren
    gleich. In `Asset.cmake` eingetragen; sie zählen nicht für die Genres.
  - vier Beispiele: `jazz_guitar_demo` (Jazz, ii-V-I, Finger Bass läuft),
    `ukulele_demo` (World, mit Piezo Guitar und Finger Bass), `drop_d_demo`
    (Rock), `pizzicato_demo` (Classical, zwei Violinen und Bass) — daraus die
    Genres der sechs; ein Genre mehr (Classical, jetzt 29).
  - Hörproben aller neun unter `.claude/handover/hoerproben_saiten_loops/`.
- **Loop Werte** neu geschrieben: geändert nur die acht, ihre `_old` (Bit für
  Bit die Werte von vorher, 24 / 24) und die 15 Loops, die sie nutzen.
- **Tests:** 101 mitgelieferte Instrumente (keys 9, bass 14, plucked 17);
  „jede mitgelieferte Datei …“ prüft jetzt, dass genau die sieben mit Tuning
  Saiten tragen; „ein mitgelieferter Loop ohne Saiteninstrument plant … wie
  ohne“ gilt für die Loops ohne sie, die anderen tragen Saiten in den Namen.
  Klangfilter 223 / 223 unter beiden Compilern.
- **Von Claude gesetzt:** die Werte je Instrument (Tabelle im Skript); der
  Kontrabass ohne Body mit der Mensur der Vorlage Bass 4; das Clavinet ohne
  Tuning (eine Saite je Taste).
- **Handbuch** 1.189.0 (§16 Loop: „Saiteninstrumente“, Model und Damping, die
  Beispiele und `vergleich\`); **Sichttest-Liste** `.claude\handover\Sichttest_Saiten.md`
  (lokal; Abschnitte T, M, S, P, E, I, N, C und die Urteile U1–U6).
- **Offen:** Patriks Sichttest; **der volle Lauf erst mit Tabs und Akkorden**
  (Patrik S133).

---

## 6. Messwerte für Korpus und Tonabnehmer (erste Recherche, S132)

Recherche durch einen Agenten, die tragenden Stellen von Claude selbst in der
Textebene der Quellen nachgelesen („bestätigt"). Noch nicht festgelegt
(S-K6 a: zweite Recherche für die Lücken). Nicht erreichbar waren: Rossing,
*The Science of String Instruments*; Paiva (Aalto); Paté/Le Carrou (HAL);
Gough (IOA); Zollner 5.15.

| Korpus | Belegt | Quelle | Lücke |
|---|---|---|---|
| Classical guitar (gewählt S-K10 a: TOP26, S. 138–140, §5f) | T(1,1) 165 Hz Q 36 · T(2,1) 204 Q 66 · B(1,2) 293 Q 50 · T(1,2) 391 Q 60 · T(3,1) 432 Q 51 · T(2,2) 607 Q 25 (Schallloch zu, je mit wirksamer Masse); Helmholtz 122 Hz Q 40 (im Modell gesetzt) | H. Wright, *The Acoustics and Psychoacoustics of the Guitar*, Diss. Cardiff 1996, S. 97–98, Tab. 5.4 — bestätigt | Stärke nur über die Masse |
| Acoustic guitar (Steelstring, Parlor) | T(1,1)₁ 116 Hz, T(1,1)₂ 219 Hz, Helmholtz 130 Hz; Modellwerte (Masse, Steife, Dämpfung) | D. P. Hess, *Savart Journal* 1(3), 2013, S. 3, Tab. 1 — bestätigt | Güte nur gerechnet; keine Dreadnought |
| Upright bass | A0 65–67 Hz, T1/A1 115 Hz, C2 145, C4 130, C3 155, A2 150/158 Hz | A. W. Brown, Diss. mdw Wien 2004, S. 80–83 — bestätigt (A0, T1/A1) | keine Güte, Stärke nur als Kurve |
| Violin | A0 270 Hz, Hauptholz 420 Hz, „thick plate" 505 Hz, Bridge Hill 2,7 kHz (Mittel vieler Violinen); Bandbreite je 14 Hz | E. Jansson, *Acoustics for Violin and Guitar Makers*, KTH 2002, S. 7.11 — Frequenzen bestätigt, Bandbreiten nur in der Abbildung | keine Stärken |
| Acoustic bass | — | — | nichts gefunden |

| Tonabnehmer | Belegt | Quelle |
|---|---|---|
| Resonanz mit Kabel | 2–5 kHz bei 2–10 H und 300–600 pF; Q ≈ 3 im Betrieb; Überhöhung ≈ 20·lg Q | M. Zollner, *Physics of the Electric Guitar*, 5.5.4 — bestätigt |
| Humbucker | Polabstand 18–19 mm, Kammfilter-Lücke; tiefere Resonanz, schwächere Überhöhung (qualitativ) | Zollner 5.4.4 — bestätigt; S. Patsch, *Physik Journal* 19 (2020) |
| Lage, Apertur | Beispiel 4 und 16 cm vom Steg bei 64 cm Mensur; Apertur ≈ 1 cm | Zollner 2.8.2, 5.4.4 — bestätigt |
| Piezo | Hochpass f = 1/(2πRC), etwa 106 Hz bei 1,5 nF und 1 MΩ | Zollner 6.2 — bestätigt |

### 6b. Zweite Recherche (S-K6 a, S132) — nur die Lücken

„Bestätigt" = von Claude in der Textebene der Quelle gefunden; „Abb." = aus
einer Kurve abgelesen (±1 dB, ±10 % in der Frequenz); „Händler" = über den
Agenten aus einer Händler- oder Ratgeberseite, nicht selbst gelesen.

| Lücke | Gefunden | Quelle | Stand |
|---|---|---|---|
| Violine, Frequenzen | A0 275 ± 9 Hz, CBR 407 ± 31, B1− 475 ± 16, B1+ 541 ± 22 (17 Violinen) | G. Bissinger, „Structural acoustics of good and bad violins", *JASA* 124(3), 2008, S. 1767 | bestätigt |
| Violine, Dämpfung | ζ ≈ 1 % der kritischen im 400-Hz-Band → Q ≈ 50 | Bissinger 2008, Abb. 4 | Abb., Q abgeleitet |
| Violine, Stärke | A0, B1−, B1+ strahlen stark ab, CBR schwach — keine Zahl je Mode | Bissinger 2008, S. 1766 | nur qualitativ |
| Kontrabass, Güte | — (Askenfelt 1982 nicht erreichbar) | — | **nicht gefunden** |
| Steelstring, größer als Parlor | Gibson Hummingbird: 103 Hz (Atmung), 188, 202, 262, 315 Hz; keine Güte | D. Russell, Penn State (Webseite) | nur über den Agenten |
| Single coil mit Last | Strat R = 6 kΩ, L = 2,2 H, C = 450 pF; Q der Spule 2,1–3,5 | Zollner/Zwicker, „Factoring in the Q", GITEC 2019, S. 2; Zollner 5.5.4 | bestätigt |
| Humbucker mit Last | Q = 1,83 an 333 kΩ ∥ 450 pF (Fender, CuNiFe); Resonanz ≈ 3 kHz | Zollner/Zwicker 2019, S. 6–7 | Q bestätigt, kHz Abb.; **keine Induktivität** |
| Lagen | Strat 152 / 98 / ≈ 41 mm vom Steg, Tele 152 / ≈ 41 mm (aus den Lagen vom Sattel gerechnet); Jazz Bass Steg 54 mm, Hals ≈ 152 mm; Les Paul widersprüchlich | S. Errede, UIUC Physics 406, „Fourier Analysis IV", S. 2; Bartolini (2025) | Errede bestätigt, mm abgeleitet; Bartolini nur über den Agenten |
| Mensur | Gitarre 25½″ = 648 mm (bestätigt bei Errede); klassisch 650 mm; E-Bass 34″ = 864 mm; Ukulele Sopran 13–14″, Konzert 15–16″, Tenor 17–18″; Violine 327 mm (bestätigt bei Jansson S. 7.3); Kontrabass 4/4 106–110 cm | Errede; Yamaha; Händler; Jansson; Thomastik-Infeld | teils bestätigt, teils Händler |

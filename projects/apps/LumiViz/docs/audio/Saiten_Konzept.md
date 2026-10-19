# LumiViz — Saiteninstrumente (Konzept)

> **Version:** 0.35.0
> **Datum:** 2026-10-19 (Session 138: S-L15 b gebaut, Bend bis 2 Ganztöne; S-T8 a bis S-T11 a
> Skizzen des Fensters *Chords* und des Dialogs *Edit chord*; S-T12 b, S-T13 a; 26e-1 und
> 26e-2 gebaut, §5y, §5z; davor Session 137; S-L13 a bis S-L15 b, 25c-4 *Follow string*; Tabs und
> Akkorde S-T1 a bis S-T7 a, gebaut 26a bis 26d, §5u–§5x; davor S136: Stufe 24f
> fertig, S-W16 a bis S-W22 a; Stufe 25 S-L1 a bis S-L12 a, gebaut 25a, 25b, 25c-1 bis
> 25c-3b, §5r bis §5t)
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

✅ **S-T1 a** (Patrik S137, Begriffsskizze A/B im Gespräch): **Die Tabs sind beides.**
**Tab-Linien** sind die Ansicht im Instrument-Editor — zwischen Strum und Piano-Roll,
mit ihr synchron; je Saite eine Linie, an jeder Note die Bundzahl; dort werden Bünde,
Hammer-on, Pull-off, Slide und Bend gesetzt und gezeigt (S-W14 a, S-L3 a, S-L5 a).
**Griffbilder** sind die Eingabe für Akkorde: Vorlagen und eigene, auf dem Griffbrett
verschiebbar; ein Griffbild setzt die Noten mit ihren Saiten ins Muster, sie erscheinen
in den Tab-Linien und in der Piano-Roll — kein zweiter Speicher für Akkorde. Verworfen:
b (nur Tab-Linien — keine Vorlagen, die Idee S127 fehlte), c (nur Griffbilder — kein
Platz für Melodie, „5h7“ und Bend; S-L5 a liefe ins Leere). Als Nächstes einzeln: wo die
Griffbilder liegen, woher die Vorlagen kommen, die Skizze von Tab-Linien und Piano-Roll.

✅ **S-T2 a** (Patrik S137): **Ein eigenes Fenster *Chords*, wie *Instruments* und
*Kits*.** Es zeigt die Griffbilder zur Stimmung des geöffneten Instruments, nach Grundton
und Art; Vorlagen und eigene nebeneinander; *New*, *Edit*, *Duplicate*, *Delete* auf einem
Griffbrett. Geöffnet aus dem Instrument-Editor mit **Chords...** (nur bei einem
Saiteninstrument). Ins Muster durch Ziehen in Tab-Linien oder Piano-Roll oder Doppelklick
an den Einfügepunkt; Verschieben auf dem Griffbrett ändert die Lage. Verworfen: b (feste
Leiste im Editor — kostet dauerhaft Platz, wenige Griffbilder), c (nur ein Menü — schwer zu
finden, kein Platz für das Bild; Merker S110). Später als angedocktes Fenster möglich, ohne
den Ort der Daten zu ändern. **Vorgabe Patrik (S137, nach 26b):** wie im Fenster
*Instruments* (`InstrumentPalette`) ein **Anhören** — ein Klick spielt den Akkord mit dem
Instrument des Editors — und **Favoriten** (★, mit eigenem Zweig *★ Favorites*, dazu
*Recently used*), von der App gemerkt als Nennung (`guitar/c_major_1`, `user:<name>`); die
Einzelheiten (Strum beim Anhören, Lage) mit der Skizze zu 26e. **Dazu Patrik:** gespeichert als eigene Datei (S-T3), mit
eigenem Datei-Icon der Marke — das Motiv ein Griffbild
(`asset/img/logo/icons/build_lvfx2_icons.py`, wie `lumiviz_lvkit.ico`).

✅ **S-T3 a** (Patrik S137): **Je Griffbild eine Datei `.lvchord`** (Format
`lumiviz-chord`, Fassung 1): Titel, Grundton, Art (etwa *major*, *minor*, *7*), die Stimmung,
für die sie gilt (Töne der leeren Saiten), je Saite Bund, leer oder nicht gespielt, dazu die
Finger. Vorlagen unter `asset/loop/chords`, eigene im Benutzerordner unter `chords`
(`user:<name>`), wie Kits und Instrumente; Datensicherung und Publish nehmen den Bereich
mit; kein *Used by* (ein Griffbild setzt nur Noten, S-T1 a). Verworfen: b (eine Sammlung
`.lvchords` je Stimmung — jede Änderung schreibt die ganze Sammlung, ein Griffbild ließe
sich nicht allein weitergeben), c (`.lvcord` — liest sich als „Schnur“; die Endungen folgen
dem englischen Wort).

✅ **S-T4 a** (Patrik S137): **Die Vorlagen aus einer Quelle, wo es sie gibt, sonst
gerechnet und gekennzeichnet.** `asset/loop/make_chords.py` schreibt die Vorlagen für
Gitarre in Standardstimmung und Ukulele aus **chords-db** (D. Rubert,
github.com/tombatossals/chords-db; LICENSE: „MIT License“, „Copyright (c) 2016 David
Rubert“ — gelesen S137), alle Lagen je Akkord; Eintrag in `CREDITS.md`. Gelesen: Stimmungen
`tunings.js` Gitarre `['E2', 'A2', 'D3', 'G3', 'B3', 'E4']`, Ukulele `['G4', 'C4', 'E4',
'A4']`; je Akkord `key`, `suffix`, `positions` mit `frets` (`x` nicht gespielt, `a` = 10),
`fingers`, `barres`, `capo`. Die Liste der Arten nur aus einer Zusammenfassung — beim Bau
am Inhalt prüfen. Für Stimmungen ohne Quelle (Drop D, die Bässe E A D G, die Violine
G D A E) rechnet das Skript nach einer festen Regel: die Akkordtöne über die Saiten,
höchstens vier Bünde Spannweite, leere Saiten bevorzugt, der Grundton im Bass; solche
Griffbilder tragen die Herkunft `computed`, das Fenster zeigt sie mit Hinweis. Verworfen:
b (nur die Quelle — vier mitgelieferte Instrumente ohne Vorlage), c (alles gerechnet — gerade
die meistgespielten Griffe nicht so, wie Spieler sie greifen). Kippt, wenn die gerechneten
Griffe bei der Durchsicht oft unspielbar wirken.

✅ **S-T5 a** (Patrik S137): **Skizze des Instrument-Editors mit Tab-Linien gebilligt**
(gezeichnet im Gespräch, auf dem Bild des Editors aus `test_LoopEditorSpielweise`; hier als
Beschreibung):
- **A Tab-Linien** zwischen Spur Strum und Piano-Roll, je Saite eine Linie, oben die
  höchste; links in der Spalte der Klaviatur Nummer und Ton der leeren Saite; rollen
  waagrecht mit, stehen senkrecht.
- **B** Bundzahl am Anfang jeder Note, ein blasser Strich zeigt die Dauer. Klick auf eine
  Linie und eine Zahl tippen = eine Note auf dieser Saite (Saite gesetzt, S-M2 a); eine Zahl
  auf eine andere Linie ziehen = derselbe Ton auf der anderen Saite (nicht greifbar: rot
  „!“); eine Zahl über eine markierte Note tippen ändert den Bund.
- **C** Hammer-on/Pull-off als Bogen mit h/p, Slide als schräge Linie mit Pfeil, Bend als
  Kurve mit „½“, „full“, „1½“, „2“ (S-L3 a, S-L15 b); gesetzt über das Kontextmenü *Legato*, *Slide*,
  *Bend*, *Edit bend...*.
- **D** Knopf **Chords...** in der Zeile *Instrument* öffnet das Fenster *Chords* (S-T2 a);
  ein Griffbild auf Tab-Linien oder Piano-Roll ziehen setzt am Schritt die Noten mit Saiten.
- **E** Die Piano-Roll zeigt dieselben Zeichen (S-L5 a).
- **F** Haken **Tab** in der Zeile *Strum* blendet die Linien aus — bei einem
  Saiteninstrument von sich aus an, der Editor merkt die Stellung.
- **G** Die Hinweiszeile nennt die Bedienung der Linien und die Zeichen.
- Mit vorgelegt: eine neue Note aus den Linien bekommt die Länge des Rasters; ein Griffbild
  setzt alle gespielten Saiten am selben Schritt, die Richtung kommt aus der Spur Strum.
Verworfen: b (umschaltbar statt der Piano-Roll — „statt“, nicht „neben“, §1), c (unten
unter *Note vol.* — der Griff gehört über die Noten, wie bei Strum S-W14 a).

✅ **S-T6 a** (Patrik S137, auf seine Frage nach Klavier, Bläsern, Schlaginstrumenten):
**Eine `.lvchord` ist entweder ein Griffbild oder ein Voicing — beides in Fassung 1.** Ein
Griffbild trägt Stimmung, Bünde, Finger (S-T3 a); ein Voicing Grundton, Art und die Töne als
Abstände über dem tiefsten Ton (etwa `[0, 4, 7]`, `[4, 7, 12]`). Das Fenster *Chords* öffnet
sich in jedem Instrument-Editor: bei einem Saiteninstrument die Griffbilder zur Stimmung,
sonst die Voicings; ein Voicing auf die Piano-Roll gezogen legt seinen tiefsten Ton auf die
Zeile, auf der es losgelassen wird. Vorlagen-Voicings (Grundstellung, Umkehrungen, weite
Lage je Art) rechnet `make_chords.py` aus den Abständen der Arten (Lehrbuchwissen, keine
Quelle nötig). Der Drum-Editor bekommt nichts. Bläser: LumiViz spielt jedes Instrument
mehrstimmig, ein Akkord auf *Flute* klingt wie ein Satz — keine eigene Regel. Gebaut zuerst
die Saiten mit den Tabs, die Voicings als nächste Stufe, ohne neue Fassung. Verworfen: b (nur
Saiten — Voicings später mit Fassung 2), c (für Tasten nur ein Werkzeug in der Piano-Roll —
keine eigenen Voicings, zwei Bedienungen). Ersetzt in S-T2 a „nur bei einem
Saiteninstrument“.

✅ **S-T8 a** (Patrik S138): **Skizze des Fensters *Chords* gebilligt** (zweite Fassung,
gezeichnet im Gespräch; auf Patriks Wunsch „der Baum ja, aber statt Namen eine Galerie von
Thumbnails“; hier als Beschreibung):
- **A** Kopf *For:* — Instrument und Muster des Editors, dazu Stimmung und Capo; gezeigt wird
  nur, was nach `passtZu` passt.
- **B** *Search* (etwa „Cm7“, „sus“, „C/E“) und eine Auswahl *Type*.
- **C** Links ein Baum der Ordner: ★ *Favorites*, *Recently used*, die zwölf Grundtöne, darunter
  ihre Arten, zuletzt *Your chords*.
- **D** Rechts eine **Galerie mit einem kleinen Griffbild je Lage**, unter Köpfen nach Art
  („C major · 5“); unter jedem Bild Akkordsymbol und Lage (*open*, *fr 3*). Ein Klick spielt an
  und wählt; Doppelklick setzt an die Einfügemarke; ein Bild auf Tab-Linien oder Piano-Roll
  ziehen setzt dort (S-T5 a D).
- **E** Eine Leiste unten zeigt das gewählte Griffbild groß (übliche Form: Saiten senkrecht,
  Sattel oben, × / ○, Finger als Zahl, „fr n“), mit Titel, Lage, Tönen und dem Zeichen
  *computed* bei gerechneten (S-T4 a). Kleines und großes Bild zeichnet derselbe Code.
- **F** *Move −1 / +1 fret* verschiebt auf dem Griffbrett (`toeneDesGriffbilds` mit
  Verschiebung).
- **G** *Play*, ★ (merkt die gezeigte Lage als Nennung, etwa `guitar/c_major_2`), *Use*.
- **H** *New...*, *Edit...*, *Duplicate*, *Delete* — *Edit* und *Delete* an eigenen; *Edit* an
  einer Vorlage legt eine eigene Kopie an.
- Fußzeile mit der Bedienung.
Verworfen: b (ein Bild je Akkord mit *Position ‹ ›* — die Form der übrigen Lagen erst nach dem
Blättern zu sehen), dazu die erste Fassung (Namen im Baum, Lagen rechts zum Blättern). Kippt,
wenn die Galerie mit dem Grundton allein (rund 170 Lagen bei der Gitarre) im Programm zu voll
wirkt — dann die Arten eingeklappt. Offen: der Dialog *New/Edit* (eigene Skizze), die
Voicings in der Galerie (26f); das Anhören regeln S-T9 a und S-T10 a.

✅ **S-T9 a** (Patrik S138, auf seinen Vorschlag „Symbole für die anderen Varianten“): **In
der Leiste unten ersetzen vier Symbole den Knopf *Play*:** ↓ *Down*, ↑ *Up*, ↓↑ *Down-up*,
= *Together*. Jedes spielt die gewählte Lage einmal an; Down und Up mit dem Spread des Musters
(ohne Muster der des Instruments), *Together* mit Spread 0, *Down-up* Down und eine halbe
Zählzeit später Up im Tempo des Loops (von Claude gesetzt: wie die Vorgabe *1/2 beat* der
Hand). **Dazu Patrik:** die gewählte Spielart **bleibt** — ein Klick auf ein anderes kleines
Bild spielt sie, bis eine andere gewählt wird; das Symbol zeigt, welche gilt; zu Beginn Down.
Am Eingefügten ändert sie nichts (die Richtung kommt aus der Spur Strum, S-T5 a). Verworfen: b
(die Symbole unter dem Bild unter dem Zeiger — schwer zu finden, verdeckt den Namen).

✅ **S-T10 a** (Patrik S138): **Das Fenster merkt die Spielart** in den Einstellungen
(`chordWindow/playAs`), wie Favoriten und *Recently used*; in der Test-Exe ist das Merken
abgeschaltet (wie bei den Gruppen des Creators). Verworfen: b (jede Sitzung mit Down — die
Spielart ist eine Vorliebe wie die Favoriten daneben).

✅ **S-T11 a** (Patrik S138): **Skizze des Dialogs *Edit chord* gebilligt** (gezeichnet im
Gespräch; hier als Beschreibung):
- **I** Die Stimmung fest — vom Instrument bei *New*, vom Griffbild bei *Edit*; im Dialog
  nicht änderbar (S-T3 a).
- **J** *First fret* ‹ ›: ab welchem Bund das Griffbrett gezeigt wird; ab 2 links die Zahl.
- **K** *Title*, *Root*, *Type* (die Arten der Vorlagen, dazu freier Text), *Description*.
- **L** Die Töne; daneben ein Vorschlag *sounds like: …* mit **Use name** (setzt Root, Type,
  Title). Erkannt werden die Arten der Vorlagen; was nicht passt, zeigt keinen Vorschlag;
  ohne Klick wird nichts gesetzt.
- **M** Die vier Symbole zum Anhören nach S-T9 a, mit derselben gemerkten Spielart.
- **N** Klick auf einen Bund setzt oder nimmt den Punkt (je Saite höchstens einer); Klick über
  dem Sattel schaltet ○ leer / × nicht gespielt; Rechtsklick auf einen Punkt wählt Finger 1–4;
  einen Punkt über Saiten ziehen setzt ein Barré; ein gestrichelter Rahmen zeigt das Feld
  unter dem Zeiger; Hinweiszeile und Tooltip.
- **O** Ctrl+Z / Ctrl+Y im Dialog (eigenes Undo, wie im Fenster *Kits*).
- **P** *Cancel*, *Save* — bei *New* und an einer Vorlage eine eigene Datei `user:<titel>`
  (vergeben: `_2`, wie *Keep both*), an einer eigenen ersetzt *Save* sie; ohne gespielte Saite
  sagt ein Hinweis, warum nicht gespeichert wird (der Knopf bleibt anklickbar).
Verworfen: b (ohne L — ein falsch benanntes Griffbild landet im falschen Ordner der Galerie).
Kippt, wenn eigene Griffbilder vor allem Griffe ohne gängigen Namen sind.

✅ **S-T12 b** (Patrik S138, nach dem Befund in §5y): **Alle Lagen aus chords-db bleiben;**
das Fenster zeigt an einer Lage, die Töne außerhalb ihrer Art greift, das Zeichen *unusual* mit
diesen Tönen (verglichen mit `akkordArten()`). `alt` und `7sg` bleiben, wie die Quelle sie
liefert (ohne Namensvorschlag). Verworfen: a (abweichende Lagen weglassen, `7sg` als `7` — es
ginge Quellgut verloren, das bewusste Variante sein kann), c (nichts prüfen — ein falscher Name
bliebe unbemerkt).

✅ **S-T13 a** (Patrik S138): **Die Arten stehen in einer Datei `asset/loop/chords/types.json`**
(Name und Stufen je Art, in der Reihenfolge des Vorrangs beim Erkennen). `make_chords.py` liest
sie statt einer eigenen Tabelle; das Programm liest sie als mitgelieferte Datei
(`asset/Asset.cmake`); ein Test prüft, dass jede Art der Vorlagen darin steht. Verworfen: b (die
Tabelle in `Akkord.cpp` maßgeblich, das Skript mit eigener — der Vergleich über die Vorlagen
sähe nur die 13 gerechneten Arten).

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
  - **J** (Lage geändert mit S-W14 a, unten) Spur *Strum* unter *Note vol.*: je Akkord ein Pfeil — grau aus der
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

✅ **S-W10 a** (Patrik S134): **Strum ist von sich aus an.** Die Vorgabe
(Spread, Hand abwechselnd) gilt für jedes Muster eines Saiteninstruments, das
nichts setzt: jeder Akkord wird angeschlagen, jede Note bekommt die Richtung
ihrer Stelle (auch Melodien: Wechselschlag). Zugleich klingt ein Akkord mit
*Spread* 0 oder *Together*. Die mitgelieferten Loops mit Saiten ändern sich
mit Absicht: belegt mit Hörproben vorher/nachher, `test_LoopWerte` für sie neu
geschrieben. Beleg von 24b: Instrumente ohne Saiten Bit für Bit wie vorher.
Verworfen: b (Vorgabe aus — Akkorde, die auf die Millisekunde zugleich
klingen, widersprächen dem Zweck der Stufe).

✅ **S-W11 a** (Patrik S134, auf Patriks Frage „ist das nicht im Loop
gespeichert?“): **Strum steht immer in der Datei** — an jedem Instrument-Muster
der `.lvloop` und unter `strings` jeder `.lvinst` mit Saiten, auch wenn es der
Vorgabe gleicht. Eine gespeicherte Datei klingt gleich, wie immer die Vorgabe
des Programms später lautet; nur Dateien vor Fassung 9 / 11 bekommen beim
Lesen die Vorgabe. Korrigiert die Setzung aus 24a („nur, wenn sie abweicht“).
Verworfen: b (nur bei Abweichung — die Vorgabe wird in 24f noch hergeleitet,
jede Änderung wirkte still auf alle Loops, die sie benutzen).

✅ **S-W12 a** (Patrik S134): ***Spread* ist der Abstand zweier benachbarter
Saiten**, Vorgabe **8 ms**, Grenze 100 ms. Der Versatz einer Note ist Spread ×
die Saiten vom ersten Anschlag des Akkords bis zu ihr; ein Akkord über sechs
Saiten dauert 40 ms, auf jedem Instrument gleich schnell. Beleg: Freire 2018
(SMC, Table 6, S. 547): sechssaitig gestrummt „adagio“ 8–11 ms je Saite,
37–46 ms über den Akkord; „moderate“ 5–12,5 / 26–62 ms. Verworfen: b (Spread
als Zeit über alle Saiten, Vorgabe 40 ms — eine Ukulele bekäme 13 ms je Saite,
eine Zwölfsaitige knapp 4: die Hand wäre je nach Saitenzahl verschieden
schnell). Korrigiert die Setzung aus 24b (§5n).

✅ **S-W13 a** (Patrik S135): **Beim Wechsel des Instruments übernimmt das
Muster *Strum* und *String choice* des neuen nur, wo es noch die Vorgabe des
alten trägt** — je Feld für sich; was am Muster geändert wurde, bleibt. Ein
Instrument ohne Saiten zählt mit den leeren Vorgaben (`Saitensatz{}`): eine
neue Basis (Vorgabe-Instrument, ohne Saiten) wird so mit der ersten Gitarre zu
einer Gitarre mit deren Vorgaben. *Anschläge* bleiben stehen. Verworfen: b
(immer übernehmen — verwirft eine bewusste Einstellung ohne Meldung), c (nie,
dafür ein Knopf *From instrument* — eine neue Basis bliebe bei den Werten des
Vorgabe-Instruments, die Lücke aus 23f offen).

✅ **S-W14 a** (Patrik S135, auf seinen Vorschlag am Bild von 24e-4): **Die
Spur *Strum* steht zwischen der Zeile der Takte und dem Raster der Noten** —
an jedem Schritt, an dem eine Note beginnt, die Anschlagrichtung (grau aus der
Hand, blau mit Nadel gesetzt, „≡“ für Together); ein Klick schaltet weiter. Sie
rollt waagrecht mit, senkrecht steht sie; eine Trennlinie und die Beschriftung
„Strum“ in der Spalte der Klaviatur setzen sie von der Zeile der Takte ab. Eine
spätere Tab-Spur kommt direkt darunter (Pfeile über dem Tab-System wie im
Druck). Ersetzt die Lage aus S-W5 a J (unter *Note vol.*). Verworfen: b (unten
lassen, bei den Spuren je Anschlag wie in einer DAW — Strum ist eine
Spielanweisung, keine Größe wie die Lautstärke, und gehört über den Griff).

✅ **S-W15 a** (Patrik S135): **An jedem Schritt, an dem eine Note beginnt,
steht ein Pfeil — auch bei einer einzelnen Note**, grau aus der Hand, blau mit
Nadel gesetzt. Ein Klick schaltet bei einem Akkord Follow hand → Down → Up →
Together, bei einer einzelnen Note Follow hand → Down → Up (Together ist auf
einer Saite dasselbe wie Down). Ersetzt den Punkt aus S-W5 a J. Verworfen: b
(Punkt, Richtung nur über das Menü — die Richtung einer Melodienote klingt,
S-W10 a). **Von Claude gesetzt, nicht vorgelegt:** schlagen an einem Schritt
nur getippte Noten an, steht dort „T“ statt eines Pfeils (ein getippter Ton hat
keine Richtung, §5o).

✅ **S-W16 a** (Patrik S136, Stufe 24f): ***Spread* je mitgeliefertem
Instrument = Saitenabstand ÷ Geschwindigkeit der Hand.** Die Geschwindigkeit
aus Freire 2018 (Table 6, S. 547: 8 ms je Saite an der Nylongitarre) und dem
Saitenabstand der Nylongitarre; die feste Geschwindigkeit nach Barbancho 2014
(S. 3). Die Saitenabstände am Anschlagort je Bauart (Nylon, Steel, E-Gitarre,
Bass, Ukulele, Violine) werden recherchiert und mit Quelle in §6 eingetragen;
gerundet auf 0,5 ms. Findet sich für eine Bauart kein belegter Abstand, bleibt
dort 8 ms, vermerkt. **Der Wert ist nur die Vorgabe** (Patrik: „kann aber
manuell angepasst werden“) — im Creator (Zeile Strum, N), am Muster (H) und je
Akkord (L) wie bisher verstellbar. Verworfen: b (8 ms für alle — überträgt eine
Messung an einer Bauart still auf andere Abstände), c (Bässe und Violine
zugleich — eine Spielregel, keine Größe des Instruments).

✅ **S-W17 a** (Patrik S136, Stufe 24f): ***Mute* der Bässe 0,37.** Finger
Bass und Slap Bass nach IDMT-SMT-Bass (drei E-Bässe, MU gegen FS, 20 Paare;
gedämpft nach Abeßer 2013, S. 1, „with the inner hand side“): je Paar aus T60 =
6,91 / (6,91/T_FS + 2 f1 ρ sin²(π · 40 mm / L)), L = 864 mm · 2^(−Bund/12),
m = √ρ — Median **0,37** (Quartile 0,32 / 0,37 / 0,45, Spanne 0,19–0,63;
nachgerechnet S136). Der Upright Bass bekommt denselben Wert, **übertragen vom
E-Bass** (nicht gemessen; der E-Bass ist ihm näher als die Gitarre). Die
Gitarren behalten 0,6 (§5o). **Korrektur:** der Recherche-Bericht 24c nannte
für den IDMT-Bass m ≈ 0,47 ohne Rechenweg; mit der Formel des Programms sind es
0,37. Verworfen: b (Upright 0,6 — überträgt das fernere Instrument), c (alle
0,6 — wirft weg, dass die Hand am Bass leichter liegt).

✅ **S-W18 a** (Patrik S136, Stufe 24f): ***Tap level* gilt bei 220 Hz** (wie
*Ring time*); ein getippter Ton bekommt Tap level · √(220 Hz / f1), f1 die
Frequenz der schwingenden Saite (`tappingFaktor`, `kTappingBezugHz`), nach oben
begrenzt bei 1,5 (+3,5 dB; `kTappingFaktorMax`, von Claude gesetzt — ohne Grenze
läge E1 am Bass bei 1,7, und diese Zahl hängt an der unbelegten Saitenlage). Die
Herleitung (Bericht 24c §3.2): Energie des Tippens gegen den Anschlag ≈ 127/f1,
also Amplitude ∝ f1^−½ — die Abhängigkeit folgt aus der Form der Rechnung, nicht
aus den angenommenen Zahlen. Alle Instrumente behalten 0,75. Verworfen: b (ein
fester Wert je Instrument für seine Lage — innerhalb eines Instruments bliebe
der Fehler, E2 gegen E5 9 dB), c (alles fest bis zu eigenen Aufnahmen). Im Creator
heißt die Zeile seither „… · at the fret, at 220 Hz“, der Tooltip nennt den Bezug.

✅ **S-W19 a** (Patrik S136): **Kontrabass 17,5 ms, Violine 7,0 ms**, obwohl die
Abstände nur aus einem Blog (No Treble) und einem Forum (Luthiers Forum) stammen —
die Quelle steht in §6c so vermerkt, ein besserer Beleg ersetzt sie. Beide passen
zur sonst belegten Bauweise (Violinensteg schmaler als ein Gitarrensteg,
Kontrabass breiter als der E-Bass); 8 ms wären von einer ganz anderen Bauart
übertragen. Verworfen: b (8 ms, „kein belegter Abstand“ in strenger Lesart).

✅ **S-W20 a** (Patrik S136): **Clean Guitar und Muted Guitar nach der Strat mit
2-Punkt-Tremolo, Spread 7,0 ms** (2-1/16", der Steg der laufenden Fender-Serien,
Fender-Teileseite und StewMac; §6c). Verworfen: b (Vintage-Tremolo 2-7/32",
7,5 ms — belegt nur an einem Nachbau und einem Ersatzsteg).

**Zuordnung der mitgelieferten Instrumente (S-W16 a bis S-W20 a):** Nylon Guitar
8,0 ms (Klassik); Guitar und Piezo Guitar 7,5 ms (Steelstring); Clean Guitar,
Muted Guitar 7,0 ms (Strat, 2-Punkt); Overdrive Guitar, Drop D Guitar 7,0 ms (Les
Paul); Jazz Guitar 7,0 ms (Archtop); Finger Bass, Slap Bass 12,5 ms (E-Bass);
Upright Bass 17,5 ms (Kontrabass); Ukulele 9,5 ms; Pizzicato Violin 7,0 ms.
*Mute* 0,37 an den drei Bässen (S-W17 a), sonst 0,6; *Up level*, *Tap level*,
*Touch* und das Muster der Hand überall die Vorgabe des Programms. **Von Claude
zugeordnet, nicht vorgelegt:** welche Bauart je Instrument gilt (nach Körper und
Tonabnehmern der Vorlage).

✅ **S-W21 a** (Patrik S136): **Die Skripte der mitgelieferten Loops schreiben in
jedes Muster mit Saiteninstrument dessen Strum**, wie das Programm es bei einem
neuen Muster tut (`uebernimmVorgaben`). Eigene alte Dateien ohne Strum bekommen
beim Lesen weiter die Programmvorgabe (S-W11 a) — eine Datei klingt für sich
allein. `loop_werte.txt` für die betroffenen Loops neu, Diff geprüft. Verworfen:
b (beim Lesen die Vorgabe des Instruments — der Klang einer Datei hinge an der
Instrumentliste beim Lesen; ein Test ohne sie läse anders als das Programm), c
(die mitgelieferten bei 8 ms — kein Loop zeigte die Vorgaben).

✅ **S-W22 a** (Patrik S136): **Vier neue Beispiel-Loops, je einer mit klarem
Schwerpunkt**, aus `asset/loop/make_spielweisen_loops.py`, nach Rechnung gesetzt,
nicht nach Gehör: `strum_demo` (Folk, Guitar: D D U U D U in Achteln — das ergibt
die abwechselnde Hand von selbst —, im vierten Takt *Together* auf der Eins und ein
langsam gebrochener Schlussakkord mit Spread 40 ms), `palm_mute_demo` (Rock, Drop
D Guitar und Finger Bass gedämpft, die Eins offen, Schluss mit *Let ring*),
`harmonics_demo` (Ambient, Clean Guitar mit natürlichen Flageoletts am 12., 7.
und 5. Bund, Nylon Guitar mit Arpeggio und *Let ring*), `tapping_demo` (Rock,
Overdrive Guitar: derselbe Lauf zwei Takte getippt, zwei gezupft). Sie behalten
ihr eigenes Strum: `strum_in_loops` nimmt sie aus (`EIGENES_STRUM`). Reihenfolge
der Skripte: `make_saiten_loops.py`, `make_spielweisen_loops.py`,
`make_saiten_instrumente.py`. Verworfen: b (ein Loop „Playing styles“ — ein
Prüfstück, das die Hörproben schon sind), c (Spielweisen in die Genre-Loops —
änderte gehörte Loops und ihre Vergleiche).

**Stufe 25 — Spielweisen, zweite Stufe: was zwei Töne auf einer Saite verbindet**
(S-W1 a; Fragen einzeln, dann Skizze, Recherche, Baustufen).

✅ **S-L1 a** (Patrik S136): **Die Verbindung steht an der Zielnote, als zwei neue
Artikulationen.** **Legato**: die Note kommt von der vorigen Note auf derselben
Saite, ohne Anschlag — liegt sie höher, heißt es *Hammer-on*, tiefer *Pull-off*
(das Programm benennt es nach der Richtung). **Slide**: sie gleitet von der vorigen
Note auf derselben Saite her. Die Saitenwahl legt verbundene Noten auf dieselbe
Saite; geht das nicht oder gibt es keine vorige Note, ist die Note rot mit „!“ und
schlägt normal an. Die Planung rechnet eine Kette verbundener Noten zu **einem**
Ton mit Tonhöhenverlauf (die Ausnahme, die S-M3 a als c offen ließ). Die Angabe
wandert mit der Note (Verschieben, Kopieren, Drehen); die Tabs schreiben „5h7“
später daraus. Verworfen: b (Liste von Notenpaaren am Muster — an allen Stellen von
S-W4 a mitzuführen, verweist auf Noten, die sich verschieben), c (an der Startnote —
sie wird normal angeschlagen; in 5h7p5 stünde die Angabe an der falschen Hälfte).

✅ **S-L2 a** (Patrik S136): **Ein Bend ist eine Kurve an der Note.** Punkte aus
Zeit (Anteil der Notenlänge, 0 bis 1) und Biegung in Halbtönen (in Vierteln, 0 bis
3); wird die Note länger, dehnt sich der Bend mit. Die Note zeigt den gegriffenen
Ton, von dem aus gebogen wird. Der Editor bietet Vorlagen, die die Punkte schreiben:
*Bend*, *Bend and release*, *Pre-bend*, *Pre-bend and release*. Die Datei speichert
die Punkte. Verworfen: b (feste Form und Höhe — a schließt sie als Vorlagen ein, das
Format bliebe später zu erweitern; Merker S120), c (Spur am Muster — ein Bend gehört
einer Saite, im Akkord wird oft nur eine gebogen).

✅ **S-L3 a** (Patrik S136): **Skizze der Bedienung gebilligt** (gezeichnet im
Gespräch S136; hier als Beschreibung):
- **A Hammer-on:** ein Bogen „h“ von der vorigen Note auf derselben Saite zur
  Zielnote; in der Zielnote das Kürzel **H** neben der Saitennummer.
- **B Pull-off:** derselbe Bogen mit „p“, Kürzel **P** — H oder P nach der Richtung
  (S-L1 a), gesetzt mit demselben Haken *Legato*.
- **C Slide:** eine schräge Linie mit Pfeil vom Ende der vorigen Note zum Anfang der
  Zielnote; Kürzel **SL**.
- **D Bend:** in der Note eine Kurve bis zur Zeile des erreichten Tons, eine
  gestrichelte Linie zeigt das Ziel; beschriftet „½“, „full“ (ein Ganzton), „1½“, „2“;
  Kürzel **↗** mit der Höhe in Ganztönen.
- **E Menü:** unter den vier Haken zwei neue, **Legato (hammer-on / pull-off)** und
  **Slide**, für die markierten Noten, mit Undo; ohne vorige Note auf derselben Saite
  rot mit „!“, die Note schlägt normal an.
- **F Menü Bend:** Vorlagen *None*, *Bend*, *Bend and release*, *Pre-bend*,
  *Pre-bend and release*; Höhe ½, 1, 1½, 2 Ganztöne — beides schreibt die Punkte.
- **G Edit bend...:** ein Dialog mit der Kurve — Zeit über die Länge der Note, Höhe
  in Vierteltönen 0 bis 4 Halbtöne (S-L15 b; zuerst stand hier 3); Punkte ziehen, Doppelklick setzt oder entfernt
  einen; Auswahl Vorlage; OK / Cancel.
- Folge: die Hinweiszeile nennt die Kürzel; *Play as* im Creator bekommt *Hammer-on*,
  *Slide*, *Bend*. Ob *Playing* Regler dafür braucht (etwa die Dauer eines Slide),
  klärt die Recherche zum Klang.
Verworfen: b (Bend nur mit einem Griff in der Piano-Roll — stellt die Höhe, nicht den
Verlauf), c (nur Kürzel und Menü — bei 5h7p5h7 zeigten nur Buchstaben, was
zusammengehört).

✅ **S-L4 a** (Patrik S136): **Stufe 25 in Schichten wie Stufe 24**, je Teilschritt
eng nach S-P1 a, am Ende der Bereichslauf: **25a** Modell und Datei (*Legato*,
*Slide*, Bend-Kurve, `.lvloop` Fassung 10, die Angaben wandern mit; ohne Klang) ·
**25b** Planung (eine Kette verbundener Noten wird ein Ton mit Tonhöhenverlauf; die
Saitenwahl legt die Kette auf eine Saite, sonst „!“; der Name des Tons trägt den
Verlauf) · **25c** Klang (zeitvariable Verzögerung in Extended KS und Waveguide,
Legato-Anregung, Slide, Bend; Vorgaben aus der Recherche
`.claude/handover/recherche_25/`) · **25d** Editor nach S-L3 a · **25e** Creator
(*Play as*; *Playing*-Regler nur, wenn die Recherche sie verlangt) · **25f**
Mitgeliefertes (Beispiel-Loops, Handbuch, Sichttest, Bereichslauf). Danach die
Tabs. Verworfen: b (je Spielweise durchgehend — mehrere Fassungen oder Felder auf
Vorrat; der gemeinsame Kern aus 25b/25c würde nachgebessert).

✅ **S-L5 a** (Patrik S136, ändert S-L4 a hinter 25c): **25a bis 25c wie geplant,
dann die Fragen zu den Tabs, dann eine gemeinsame Stufe** — die Tab-Ansicht und der
Editor für Legato, Slide und Bend (bisher 25d) zusammen, sodass die Zeichen in
Piano-Roll und Tab von Anfang an gleich gesetzt und gezeigt werden; zum Schluss
Creator (25e) und Mitgeliefertes mit Beispielen, Handbuch, Sichttest und dem vollen
Lauf. Verworfen: b (getrennt — die Bedienung würde mit den Tabs ein zweites Mal
angefasst), c (die Tabs zuerst — der Klang der verbundenen Noten wartete, obwohl er
nicht an den Tabs hängt).

✅ **S-L6 a** (Patrik S136): **Der Name eines Tons trägt den Verlauf lesbar**, wie
die übrigen Zusätze: `:v<ms><art><hoehe>,…` für die Wechsel (art `l` Legato, `g`
Slide; etwa `:v240l59,480g62`) und `:b<ms>-<viertel>,…` für den Bend (Halbtöne in
Vierteln als ganze Zahl; etwa `:b0-0,90-8,400-8`), vor `:e`; `zerlegeTonRest` liest
beides zurück. Verworfen: b (eine Prüfsumme — der Name ist der Weg von der Planung
zum Klangerzeuger; ein zweiter Speicher für den Verlauf müsste mit ihm übereinstimmen).

✅ **S-L7 a** (Patrik S136): **Die Verzögerung wird mit Lagrange 5. Ordnung gelesen**
(6 Abgriffe, im Bereich 2 bis 3 Abtastwerte, Koeffizienten je Abtastwert) — für
**alle** Töne mit *Extended KS* und *Waveguide*, nach PPV08 S. 46 (selbst gelesen:
„this fifth-order Lagrange interpolator is operated in the delay range“ 2,0–3,0).
Behebt nebenbei, dass die lineare Interpolation die Höhen je nach Bruchteil der Periode
verschieden dämpfte. Rekursive Allpässe nicht (Välimäki 1995 S. 117: Transienten,
„clicks in audio applications“). `loop_werte.txt` für die Loops mit Saiten neu,
Hörproben vorher/nachher. Verworfen: b (linear wie bisher — ein Slide flatterte in der
Helligkeit), c (Lagrange nur für Töne mit Verlauf — derselbe Ton klänge über Legato
erreicht heller als angeschlagen).

✅ **S-L8 a** (Patrik S136): **Der Gewinn bei einer Änderung des Umlaufs folgt der
Physik der Auslenkungswellen** (im Umlauf laufen Auslenkungen, §5c/§5d): Legato und
Slide **g = 1**, Bend **g = √(1+Δx)** — die Amplitude geht mit √(Verzögerung); dazu je
Slide der gemessene **Mehrverlust 2 dB** (IDMT-SMT-Guitar DI, Median AR/FS, ohne Gang mit
der Weglänge; Recherche 25 §3), gleichmäßig über die Gleitdauer. Herleitung: E/ω ist
bei langsamer Änderung je Mode invariant (Recherche 25 §2, abgeleitet); Bend gemessen
−0,3 dB, hergeleitet +0,5 dB beim Ganzton. Verworfen: b (Pakarinens Energiekorrektur
√(1−Δx) — gilt für Schnellewellen, ließe einen Slide zum Sattel bis 3 dB lauter
werden; PPV08 S. 46 nennt das Verhalten der echten Saite selbst „controversial“), c
(keine Korrektur — beim Bend falsch, den gemessenen Mehrverlust des Slide ließe sie
weg).

✅ **S-L9 a** (Patrik S136): **Ein Slide ist stufig auf Instrumenten mit Bünden, glatt
auf bundlosen.** Je Bund ein Halt, dazwischen ein Übergang, linear in Cent; der Übergang
nimmt **57 %** jedes Bundschritts ein — hergeleitet aus der Messung (IDMT DI: 66 % der
Zeit eines Slide innerhalb ±20 Cent um einen ganzen Halbton; 1 − 0,6 · Anteil = 0,66).
Bundlos: stetig, linear in Cent. Das Instrument bekommt ein Feld **Fretless** an der
Zeile Tuning (Vorgabe aus; die Vorlage *Violin* und der Upright Bass setzen es) —
`.lvinst` **Fassung 12**; die Tabs brauchen es ohnehin (bundlos keine Bundzahl).
Verworfen: b (immer stufig — der Kontrabass glitte wie eine Gitarre), c (immer glatt —
die gemessene Bundstufe fehlte).

✅ **S-L10 a** (Patrik S136): **Ein Slide dauert 20 ms je Halbton und endet am Schritt
der Zielnote.** 20 ms aus der Messung im Zusammenhang (IDMT-Licks: 1 HT 10 ms, 2 HT
40 ms, 3 HT 55 ms); einstellbar in *Playing* als **Slide time**, 5 bis 60 ms je Halbton
(in der `.lvinst` Fassung 12, zusammen mit *Fretless*). Das Gleiten beginnt so viel
früher, frühestens am Anschlag der vorigen Note; ist die vorige kürzer, gleitet es so
viel schneller — die Zielnote kommt auf ihrem Schritt an, wie die Tab „5/7“ sie
schreibt. Verworfen: b (35 ms fest, beginnt am Zielschritt — didaktisch langsam, jede
Zielnote käme zu spät), c (20 ms fest ohne Regler — die Geschwindigkeit liesse sich
dem Stil nicht anpassen).

✅ **S-L11 a** (Patrik S136): **Die Legato-Anregung hat die Form der Physik, der Pegel
ist ein Regler.** Hammer-on: ein Impuls am neuen Bund (der Finger schlägt am Ende der
schwingenden Länge auf — schwach und hell); Pull-off: eine Zupfform an der alten
Bundstelle, β = 1 − 2^(−n/12) vom neuen Ende (abgeleitet, Recherche 25 §1). Der Pegel
ist **Legato level** in *Playing* (Fassung 12); die Vorgabe so, dass im Messfall
(Achtel in Übungsgeschwindigkeit, 1 bis 3 Halbtöne) der Sprung über den abgeklungenen
Pegel im Mittel **+3 dB** beträgt — gemessen an Mitsou 2023 über einen
Verstärker-Simulator, also eine **Untergrenze**; ein Startwert, bis eine saubere
DI-Aufnahme vorliegt (Protokoll in Recherche 25, Lücke 1). Gelesen: Laurson 2001 S. 47
(Slur als kleiner Anschlag, Schleifengewinn unverändert), Özaslan 2010 S. 458 (weniger
Energie, andere Harmonizität). Verworfen: b (ein leiser normaler Anschlag wie Laurson —
Hammer-on und Pull-off klängen gleich und wie ein Anschlag), c (nur der Längensprung —
widerspricht dem gemessenen neuen Einsatz).

✅ **S-L12 a** (Patrik S136, auf seine Frage nach einem Schalter): **Ein Schalter je
Schicht, *In a legato*:** **Own note** (jede Note der Kette spielt die Schicht als
eigenen Ton — Vorgabe) oder **Follow string** (die Schicht folgt dem Verlauf der Saite
als ein Ton, mit Sprüngen, Gleiten und Biegung). Nur die Saiten-Schicht (*Extended KS*,
*Waveguide*) läuft immer als ein Ton über die Kette. Gebaut in zwei Teilen: *Own note*
mit 25c-3b, *Follow string* als **25c-4** (wechselnde Tonhöhe für die übrigen
Schwingungsarten; ohne Verlauf rechnet jede Schicht wie bisher); das Feld kommt in
Fassung 12, die noch nicht ausgeliefert ist. Verworfen: b (ein Schalter je Instrument —
zu grob für ein Instrument mit kurzem Körperklang und gehaltener Fläche zugleich), c
(immer *Own note* — gehaltene Schichten mit Legato wären dauerhaft ausgeschlossen).

✅ **S-L13 a** (Patrik S137, Skizze A–C im Gespräch; hier als Beschreibung): **Der
Schalter steht im Kasten *Source* unter *Start offset*, dazu ein Zeichen in der Liste
*Layers*.**
- **A:** eine Zeile **In a legato** mit der Auswahl *Own note* / *Follow string*, darunter
  ein Satz, was sie bewirkt; Tooltip nennt Hammer-on, Pull-off, Slide und Bend.
- **B:** an einer Saiten-Schicht (*Extended KS*, *Waveguide*) dieselbe Zeile abgeschaltet
  mit „Follow string (always)“ und „The string itself carries the legato“.
- Die Zeile ist nur zu sehen, wenn das Instrument Saiten hat (wie die Hinweise zu
  *Playing*).
- **C:** in der Liste *Layers* steht „~“ hinter dem Namen jeder Schicht, die der Saite
  folgt (die Saiten-Schicht eingeschlossen).
- Folge: mit *Follow string* biegt die Schicht auch bei einer einzelnen Note mit Bend
  mit; mit *Own note* bleibt sie ungebogen.
Verworfen: b (ohne Zeichen in der Liste — einem Instrument sähe man nicht an, welche
Schicht folgt), c (ein Knopf „~“ in der Liste statt der Zeile — schwer zu finden, kein
Platz für die Erklärung; Merker S110).

✅ **S-L14 a** (Patrik S137, nach der Messung 25c-4): **Während eines Bends gilt der Verlust
je Umlauf der gebogenen Tonhöhe** — ein gebogener Ton klingt so lange wie ein gegriffener
derselben Höhe (*Ring time* gilt für jede Tonhöhe gleich). Der Gewinn je Umlauf beim
Grundton wird mit dem Verhältnis der Perioden umgerechnet: g = g₀^(P/P₀). Erwartet rund
−0,5 dB gegen den ungebogenen Ton (das Gesetz √(Verzögerung), S-L8 a). Ohne Bend Bit für
Bit wie vorher. Beleg: die IDMT-Bends zeigen im Halten kein schnelleres Abklingen
(−4 … −9 dB/s, Mehrverlust Median −0,3 dB; Recherche 25 §4). Verworfen: b (der Verlust der
gegriffenen Höhe — T60 11 % kürzer je Ganzton, −1,1 dB; die Messung zeigt das nicht, und ein
gebogenes B3 klänge kürzer als ein gegriffenes).

**Stufe 26 — Tabs und Akkorde** (✅ **S-T7 a**, Patrik S137: in Schichten, je Teilschritt
eng nach S-P1 a unter beiden Compilern, am Ende Bereichslauf und **voller Lauf**; nimmt die
offenen Teile 25d, 25e, 25f auf, S-L5 a; Entscheide S-T1 a bis S-T6 a in §1). Verworfen: b
(zuerst das Fenster Chords, dann die Tab-Linien — Einfügen ohne die Ansicht, in die es
schreibt; die Tests dafür zweimal), c (eine Stufe ohne Teilschritte — gegen S-P1 a). Kippt,
wenn Akkorde auf dem Klavier dringender sind: dann 26f direkt nach 26b.

| Stufe | Inhalt | Beleg |
|---|---|---|
| 26a | **Modell und Datei:** `.lvchord` Fassung 1 (Griffbild oder Voicing, S-T3 a, S-T6 a), Lesen und Schreiben, eigene Ordner, Bereich in Datensicherung und Publish; Griffbild und Voicing in Noten (mit Saiten) umrechnen | Rundweg der Datei; Umrechnung je Art ohne Qt-Anwendung; Wegwerf-Ordner |
| 26b | **Vorlagen:** `make_chords.py` (chords-db für Gitarre und Ukulele, gerechnet für Drop D, Bässe, Violine; die Voicings), `CREDITS.md`, Icon `lumiviz_lvchord.ico`; Bildbogen der gerechneten Griffbilder zur Durchsicht (Kipp-Bedingung S-T4 a) | jede mitgelieferte Datei liest sich; Zahl je Stimmung; jedes Griffbild klingt die Töne seiner Art |
| 26c | **Tab-Linien:** Ansicht und Eingabe nach S-T5 a A, B, F — Bünde tippen, Saite wechseln durch Ziehen, Haken *Tab*, Zoom und Rollen mit der Piano-Roll | Bilder aus den Tests; Bedien-Fälle über den Editor |
| 26d | **Legato, Slide, Bend bedienen** in Tab-Linien und Piano-Roll (S-T5 a C, E, G; S-L3 a A–G mit Menü und *Edit bend...*) | Bilder; Fälle über den Editor; das Gesetzte klingt (Planung 25b) |
| 26e | **Fenster Chords mit Griffbildern** — vorher eine eigene Skizze: Liste, Griffbrett, *New*, *Edit*, *Duplicate*, *Delete*, Einfügen durch Ziehen und Doppelklick; Anhören, ★ Favoriten und *Recently used* wie im Fenster *Instruments* (Vorgabe Patrik S137) | Bilder; Wegwerf-Ordner; Einfügen über den Editor |
| 26f | **Voicings** im selben Fenster für Instrumente ohne Saiten (S-T6 a) | Bilder; Einfügen auf der Zeile des Loslassens |
| 26g | **Creator:** *Play as* mit *Hammer-on*, *Slide*, *Bend* (bisher 25e) | Bilder |
| 26h | **Mitgeliefertes:** Beispiel-Loops mit Tabs, Akkorden, Legato, Slide, Bend; Handbuch; Sichttest-Liste; Bereichslauf; **voller Lauf** | beide Compiler |

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
| 24e | **Instrument-Editor** nach S-W5 a (H bis M); beim Wechsel des Instruments die Vorgaben nach S-W13 a | Bilder aus den Tests |
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

### 5m. Stufe 24a — Modell und Datei der Spielweisen (gebaut S134)

- **`loop/Spielweise.hpp`** (neu): `Richtung` (Down, Up), `HandRaster`
  (*Beat*, *1/2 beat*, *Step*), `Strum` (`spreadMs`, `raster`, `hand` — je
  Stelle eines Takts die Richtung, dahinter abwechselnd), `handStellen`,
  `handStelleVon`, `richtungAnStelle`, `richteStrum`; `Anschlagart` (Follow
  hand, Down, Up, Together), `Anschlag` (`schritt`, `art`, `spreadMs` −1 = wie
  das Muster); die Artikulationen als Bits `kPalmMute`, `kLetRing`,
  `kTapping`, `kHarmonic`.
- **Am Modell:** `Saitensatz::strum` (Vorgabe des Instruments), `Note::
  artikulationen`, `Muster::strum`, `Muster::anschlaege`; `anschlagAn`,
  `setzeAnschlag`, `richteAnschlaege`, `uebernimmVorgaben` (String choice und
  Strum vom Instrument ans Muster); an der Schicht `aufschlagLaut` (*Up level*),
  `handDaempfung` (*Mute*), `tappingLaut` (*Tap level*), `knotenDruck`
  (*Touch*).
- **`.lvloop` Fassung 9:** am Instrument-Muster `strum` (`spread`, `hand`,
  `pattern` als `D`/`U`; seit S-W11 a immer) und `strokes` (`step`, `stroke`,
  `spread`; nur, wenn es welche gibt); die Artikulationen als sechster Wert einer Note (dann die
  Saite als fünfter, −1 = keine). **`.lvinst` Fassung 11:** unter `strings`
  der Block `strum` (seit S-W11 a immer, wenn das Instrument Saiten hat; am
  Muster immer), an einer Saite *Extended KS* oder
  *Waveguide* der Block `playing`.
- **Der Eintrag wandert mit** (S-W4 a): `kopiereNoten` / `fuegeNotenEin`
  (`NotenKopie::anschlaege`), `loescheNoten`, `versetzeNoten`, `dreheNoten`
  (über `nimmAnschlaegeMit`: ein Eintrag bleibt, solange an seinem Schritt
  eine nicht bewegte Note anschlägt, und geht mit an den neuen Schritt),
  `setzeMusterTakte` mit Wiederholen, `setzeRaster`, das Ziehen einer Note im
  Instrument-Editor (gerechnet vom Stand vor dem Zug) und das Löschen einer
  einzelnen Note dort.
- **Belegt** in `test_LoopSpielweise` (vier Fälle, der letzte mit einem
  Teilfall je Stelle); Fassungsnummern in sieben Testdateien nachgezogen.
  Eng unter Release-clang (79 Fälle, 9 s) und Testing-MSVC (81 mit Loop Werte,
  100 s) grün; `test_LoopWerte` unverändert (368 Einträge).
- **Von Claude gesetzt, nicht vorgelegt:** die Namen der Felder und der
  Datei-Schlüssel; *Spread*-Vorgabe 20 ms und Grenze 500 ms, die Vorgaben von
  *Playing* (0,8 / 0,6 / 0,65 / 0,3) — Platzhalter bis 24c/24f; das Muster der
  Hand hat genau eine Schreibweise (hinten weg, was ohnehin abwechselt);
  Artikulationen frei kombinierbar; bei zwei Einträgen am selben Schritt in
  einer Datei bleibt der erste; ein Teil eines Akkords, der verschoben wird,
  nimmt den Eintrag mit, und der Rest behält ihn.
- **Gefunden:** dass ein neues Muster *String choice* von seinem Instrument
  bekommt (§5a, „baut 23f“), war nicht gebaut. `uebernimmVorgaben` deckt es
  jetzt im Modell ab; der Aufruf beim Wechsel des Instruments kommt mit 24e.

### 5n. Stufe 24b — Planung der Spielweisen (gebaut S134)

- **Richtung und Versatz** (`schlageAn`, `Anschlagplan`): je Akkord aus seinem
  Eintrag in *Anschläge*, sonst aus Strum des Musters (Spread, Hand an seiner
  Stelle). *Together*: kein Versatz, klingt wie Down. Der Versatz wächst mit
  dem Weg der Hand: Down beginnt an der ersten Saite des Akkords nach der
  Reihenfolge am Instrument, Up an der letzten; *Spread* ist seit S-W12 a der
  Abstand zweier benachbarter Saiten (Vorgabe 8 ms; bis dahin die Zeit über
  alle Saiten, 20 ms).
- **Flageolett** (S-W7 a): `teiltonHalbtoene`, `flageolettAuf`,
  `kFlageolettTeiltonMax` 6 in `Saiten.hpp`; in `greifeNoten` eine Stelle mehr
  in den Kosten (nach „nicht greifbar“: ein natürliches kostet sein n, ein
  künstliches 7), `Griff::teilton`. Ohne Noten mit *Harmonic* ist die Wahl
  dieselbe wie bis Stufe 23.
- **Let ring:** die Dauer wird bis zum Ende verlängert, an dem die nächste Note
  auf derselben Saite anschlägt.
- **Der Ton:** `TonAngabe` trägt `richtung`, `artikulationen` (nur Palm mute
  und Tapping) und `teilton`; der Name `:h<n>`, `:u`, `:m`, `:t` vor `:e`.
  `GeplanterTon` (= `TonAngabe` + `versatzMs`) in `ClipToene`; `plane` setzt
  jeden Schlag um seinen Versatz später, `toeneImKanal` rechnet das Ende mit
  dem versetzten Anschlag.
- **Gefunden und behoben:** `test_LoopWerte` und die Hörproben
  (`LUMIVIZ_LOOP_WAV`) planten **ohne** die Auskunft über die Saiten — die
  Saitenplanung aus 23b (Saite, Abdämpfen) stand nie in den Werten. Jetzt wie
  im Programm (`saitenAusInstrumenten()`); dazu `LUMIVIZ_LOOP_WAV_OHNE_STRUM=1`
  für Hörproben ohne Strum.
- **Belegt** in `test_LoopSpielweise` (drei Fälle mehr: Richtung und Versatz,
  Harmonic, die Töne je Vorkommen mit `plane`); im 23b-Fall von
  `test_LoopSaiten` ist die zweite E4 jetzt ein Upstroke (`:u`, S-W10 a).
  `test_LoopWerte` neu geschrieben: genau die 24 Loops mit Saiten geändert,
  kein Instrument und kein Loop ohne Saiten. Eng unter beiden Compilern grün
  (54 Fälle; Testing-MSVC 61 s).
- **Hörproben** `.claude/handover/hoerproben_24b/vorher` (ohne Strum) und
  `nachher` (je 24): Unterschied nur, wo Akkorde gespielt werden — am größten
  `drop_d_demo` (effektiv 16 % der Spitze), `jazz_guitar_demo`,
  `ukulele_demo`, `bossa` (3 bis 6 %); null bei den neun Loops ohne Akkorde.
  Die Richtung ändert den Klang erst mit 24c.
- **Von Claude gesetzt, nicht vorgelegt:** *Spread* über alle Saiten des
  Instruments (abgelöst durch S-W12 a); eine nicht greifbare Note bekommt ihren
  Versatz nach der Saite, auf der sie klingt; Palm mute und Tapping gelten
  auch für eine nicht greifbare Note.

### 5o. Stufe 24c — der Klang der Spielweisen (gebaut S134)

Nach S-W6 a, S-W7 a, S-W8 a; nur an Saiten *Extended KS* und *Waveguide*,
jede andere Schicht spielt die klingende Tonhöhe wie immer. Grundlage: der
Recherche-Bericht S134 (Agent; lokal unter `.claude/handover/recherche_24c/bericht.md`;
die tragenden Stellen von Claude an der Quelle nachgelesen, unten mit Fundstelle).

- **Punktdämpfer im Umlauf** (`PunktDaempfer`, `Spielart`, `laufeUm`): nach
  Pakarinen (EUSIPCO 2005, Gl. 2–3, nachgelesen S. 2): ein Widerstand R an
  einem Serienknoten, durch geht 2/(2 + ρ), zurück kommt −ρ/(2 + ρ), ρ = R/Z0.
  Ein Punkt x der Saite liegt im gefalteten Umlauf zweimal (x · L/2 und
  (2 − x) · L/2); dort greift er an, in beiden Verfahren gleich.
- **Palm mute:** der Dämpfer bei `kHandVomStegMm` = 40 mm vor dem Steg
  (abgeleitet im Bericht aus dem Abklingen gedämpfter Töne in IDMT-SMT-Guitar,
  ≈ 40 mm; passt zu Biral et al., ICMC 2014, S. 1485 „the first few
  centimeters“, nachgelesen), ρ = Mute², bezogen auf die schwingende Länge
  (gegriffen kürzer). **Gemessen:** A3 auf der D-Saite T60 0,462 s gegen
  0,456 s nach der Formel des Berichts (T60 = 6,91 / (6,91/Ring time +
  2 f1 ρ sin²(π x/L))); der 4. Teilton verliert schneller als der Grundton;
  der Anschlag bleibt in der gemessenen Spanne (−3,4 dB KS, −1,7 dB WG gegen
  −4 … +6 dB); ein Bass (A1, 864 mm) klingt gedämpft 2,31 s, die Gitarre
  0,46 s.
- **Harmonic:** gerechnet wird der Grundton der Saite (klingend − Teilton),
  der Finger als Dämpfer am Knoten 1 − 1/n vom Steg (über dem Griffbrett),
  ρ = Touch, er bleibt die ganze Note. **Gemessen:** E3 als n = 2: E2 nach
  0,3 s −82 dB gegen E3; n = 5 −13,69 Cent gegen G#4 (Theorie −13,7).
- **Tapping:** die Anregung am Bund (gerechnet an der gleichwertigen Stelle
  `kAnschlagOrtMin`), geglättet bei `kTappingGlaettenHz` 640 Hz (abgeleitet:
  0,5 ms Fingerweg nach Bilbao/Torin DAFx-14, keine Messung), mal *Tap level*;
  ein getippter Ton hat keine Richtung. **Gemessen:** 8. Teilton gegen
  Grundton 12 (KS) bzw. 18 dB (WG) leiser als ein Zupfton an derselben Stelle.
- **Up:** der fertige Ton mal −*Up level*. Weil jeder Ton für sich gerechnet
  wird, hört man das Vorzeichen nur, wo Töne sich überlagern (Bericht §2.2,
  im Code bestätigt).
- **Vorgaben:** *Up level* 1,0 (vorher 0,8 — ein Pegelunterschied je Saite ist
  nicht belegt; dass Up weniger Saiten trifft, macht der Akkord); *Mute* 0,6
  (zwischen den gemessenen Stufen M3 und M4; welche „üblich“ ist, sagt keine
  Quelle); *Tap level* 0,75 (vorher 0,65; abgeleitet, Spanne 0,5–1,2);
  *Touch* 0,3 (gemessen 4–6 dB je Periode, simuliert ρ ≈ 0,25–0,35).
- **Belegt** in `test_LoopSpielweiseKlang` (fünf Fälle, der letzte legt mit
  `LUMIVIZ_24C_WAV=<Ordner>` Hörproben der Spielweisen ab). Ohne Spielweise
  Bit für Bit wie vorher; `test_LoopWerte` geändert nur für 20 Loops mit
  Saiten (das Vorzeichen der Upstrokes), alle Messfälle der Saiten (KS, WG,
  Korpus) unverändert grün.
- **Hörproben:** `.claude/handover/hoerproben_24c/spielweisen` (je Instrument
  normal, up, palm_mute, tapping, harmonic_n2, harmonic_n3);
  `hoerproben_24c/vorher` (= 24b) und `nachher` je 24 Loops.
- **Von Claude gesetzt, nicht vorgelegt:** ρ = Mute² (die gemessenen Stufen
  liegen so gleichmäßig über dem Regler — Bericht §1.4); der Finger bleibt die
  ganze Note liegen; der Knoten über dem Griffbrett (1 − 1/n vom Steg); beim
  Tapping zählt die Richtung nicht.
- **Offen:** ein Punktdämpfer unterschätzt bei starkem Mute die Höhendämpfung
  (die Hand liegt flächig, Bericht §1.4) — zwei oder drei Punkte zwischen 20
  und 60 mm wären eine eigene Frage; Pegel Up gegen Down und Tapping nur durch
  eigene Aufnahmen zu schließen.

### 5p. Stufe 24d — die Spielweisen im Creator (gebaut S134)

Nach der Skizze §4b (S-W8 a), alles nur bei einem Instrument mit Tuning.

- **N** `loop/StrumZeile` (neu, auch für H in 24e): *Strum*, *Spread* (ms je
  Saite, 0 … 100), *Hand* (*Beat*, *1/2 beat*, *Step*), je Stelle ein Feld
  ↓/↑ (ein Klick dreht es, ein abweichendes ist gedrückt hinterlegt),
  *Alternate*, der Hinweis „default for new patterns“. Ohne Loop rechnet die
  Zeile mit 4/4 und 4 Schritten je Schlag (8 Stellen bei *1/2 beat*). Im
  Creator unter Tuning, stellt `Saitensatz::strum`.
- **O** Gruppe *Playing* (`creatorGroup_playing`) hinter *Pickups*, im
  Kasten Source: *Up level* („starts from the other side“), *Mute* („rings
  … s at 220 Hz“ — die Formel aus §5o mit der Mensur als schwingender Länge),
  *Tap level* („at the fret“), *Touch* („finger on the node“), darunter „Let
  ring needs no setting · Classic has no Playing“. Nur bei *Extended KS* und
  *Waveguide*.
- **P** *Play as* (`creatorPlayAs`) unter der Klaviatur: Normal, Up stroke,
  Palm mute, Tapping, Harmonic, Strum chord. `vorhoerNoten` baut die Noten
  eines Klicks (Strum und String choice wie ein neues Muster dieses
  Instruments; *Strum chord* = die leeren Saiten mit Capo, jede auf ihrer).
- **Der Weg zum Ton:** der Creator sendet statt `tonVerlangt`/`tonLangVerlangt`
  ein Signal `vorhoerenVerlangt(nennung, noten, klingtMs)`; Instrument- und
  Drum-Editor reichen es als `instrumentVorhoerenVerlangt` weiter, das
  Loop-Panel spielt es mit `spieleInstrumentNoten` (jede Note an Schritt 0,
  einen Schlag lang, ein Akkord bleibt einer). Die Planung aus 24b macht
  Versatz, Richtung und Artikulation. Eine Taste des Editors nimmt die
  Abweichungen der Akkorde ihres Musters nicht mehr mit (`spieleVor`).
- **Belegt** in `test_LoopCreatorSpielweise` (fünf Fälle, `LUMIVIZ_CREATOR_SPIEL_BILD`),
  in `test_LoopSpieler` der Weg ins Panel; die fünf Verbindungen in
  `test_LoopInstrumentEditor` auf das neue Signal umgestellt (kurze und lange
  Töne nach `klingtMs` getrennt, wie vorher die zwei Signale).
- **Von Claude gesetzt, nicht vorgelegt:** die Objektnamen; *Play as* bei
  einem Instrument ohne Tuning: jede Taste ihr Ton; *Strum chord* hängt nicht
  an der angeklickten Taste.

### 5q. Stufe 24e — die Spielweisen im Instrument-Editor (gebaut S135)

Nach der Skizze S-W5 a (H bis M), in vier Teilschritten: H mit dem Wechsel des
Instruments; I und K (Versatz und Kürzel im Raster); J (Spur *Strum*); L und M
(Menü, Hinweiszeile).

- **Teilschritt 1 (gebaut S135):** **H** die `StrumZeile` (`editorStrum…`)
  unter der Instrument-Zeile, im Raster des Loops, Hinweis „for every chord of
  this pattern“; sie stellt `Muster::strum` (Undo „Change strum“) und steht nur
  bei einem Instrument mit Tuning da. **S-W13 a:** `wechsleVorgaben` im Modell,
  aufgerufen in `InstrumentEditor::setzeInstrument` — derselbe Undo-Schritt wie
  der Wechsel („Change instrument“). Damit ist die Lücke aus 23f geschlossen.
  **Belegt** in `test_LoopEditorSpielweise` (zwei Fälle,
  `LUMIVIZ_EDITOR_STRUM_BILD`); eng (35 Fälle zu Editor, Creator, Palette,
  Spieler, Spielweise) unter Release-clang und Testing-MSVC grün.
- **Teilschritt 2 (gebaut S135):** **I** jede Note steht um ihren Versatz aus
  der Planung (`schlageAn`, über `NotenRaster::anschlaege`) nach rechts
  verschoben, in wahrem Maßstab (ein Schritt = `kSchrittBreite`; bei 120 bpm
  und vier Schritten je Schlag sind 8 ms ein Pixel); je Akkord verbindet ein
  feiner Strich die Anschläge in ihrer Reihenfolge, ein Punkt sitzt an der
  zuerst angeschlagenen Note. **K** die Kürzel `PM LR T ◇` in dieser Folge
  hinter der Nummer der Saite (`artikulationsKuerzel`); was nicht passt,
  schneidet der Balken ab. Das Raster kennt dafür das Tempo (`setTempo`).
  **Belegt** in `test_LoopEditorSpielweise` (ein Fall mehr; das Bild
  `<name>_raster.png` mit Spread 30 ms bei 80 bpm); eng (36 Fälle) unter
  beiden Compilern grün.
  **Von Claude gesetzt, nicht vorgelegt:** der wahre Maßstab (kein
  Mindestabstand — bei schnellem Tempo ist der Versatz kaum zu sehen, der
  Strich zeigt die Reihenfolge trotzdem); kein Pfeil am Strich, sondern ein
  Punkt am Anfang (ein Pfeil nach oben bei einem Down wäre gegen die Sprache
  der Spieler — Down führt in der Piano-Roll von unten nach oben); Klick- und
  Zugflächen der Noten bleiben am Schritt, ohne den Versatz.
- **Teilschritt 3 (gebaut S135):** **J** die Spur *Strum* unter *Note vol.*
  (`kStrumHoehe` 20 px, nur mit Saiten — dann rückt *Note vol.* um so viel
  nach oben): je Schritt mit Akkord ein Pfeil ↓/↑ nach der Planung, grau aus
  der Hand, blau mit Nadel bei einem Eintrag in *Anschläge*, „≡“ für
  *Together*; eine einzelne Note ein Punkt. Ein Klick auf einen Akkord
  (`schalteAnschlag`) schaltet Follow hand → Down → Up → Together → Follow
  hand, Undo „Strum: Down“ usw.; ein eigener *Spread* des Eintrags bleibt.
  **Gefunden und behoben:** `NotenRaster::uebernimm` meldete nur, wenn sich die
  Noten änderten — eine Änderung allein an *Anschläge* wäre still verloren
  gegangen; jetzt vergleicht es das ganze Muster.
  **Belegt** in `test_LoopEditorSpielweise` (ein Fall mehr, mit Mausklick; das
  Bild `<name>_spur.png`); eng (37 Fälle) unter Testing-MSVC, der Bereich
  `*Loop*,*Instrument*,*Drum*` (265 Fälle) unter Release-clang grün.
  **Von Claude gesetzt, nicht vorgelegt:** eine einzelne Note lässt sich in der
  Spur nicht schalten (Punkt, blau, wenn sie einen Eintrag trägt — die Skizze
  nennt nur den Punkt; die Richtung einer Melodienote wird mit L im Menü
  gesetzt); die Höhe der Spur.
- **Teilschritt 4 (gebaut S135; 24e fertig):** **L** im Menü der Noten hinter
  *String* das Untermenü *Strum* (`instrumentStrumMenu`): *Follow hand*,
  *Down*, *Up*, *Together* (`setzeAnschlagart`) und *Spread...* (Dialog
  `strumSpreadQuestion`, ganz unten „Like the pattern“ = −1;
  `setzeAnschlagSpread`) — für jeden Schritt, an dem eine markierte Note
  anschlägt, auch eine einzelne (so bekommt eine Melodienote ihre Richtung);
  ein Haken steht, wo alle diese Schritte gleich sind. Danach die Haken *Palm
  mute*, *Let ring*, *Tapping*, *Harmonic* (`schalteArtikulation`): tragen alle
  markierten Noten sie, geht sie weg, sonst bekommen sie alle. Je Wahl ein
  Undo-Schritt („Strum: Up“, „Spread: 30 ms“, „Palm mute on“). Ohne Saiten
  fehlt beides. **M** die Hinweiszeile nennt die Spur (Pfeile grau/blau, ≡),
  die Kürzel und „right-click a note: String, Strum, playing styles“.
  **Belegt** in `test_LoopEditorSpielweise` (ein Fall mehr, über das Menü und
  den Dialog); eng (38 Fälle) unter Testing-MSVC, `*Loop*,*Instrument*,*Drum*`
  (266 Fälle) unter Release-clang grün.
  **Von Claude gesetzt, nicht vorgelegt:** die Namen im Menü und der Undo-Texte;
  *Strum* im Menü gilt auch für einzelne Noten (die Spur J bis S-W15 a nicht).
- **Nachgezogen nach S-W14 a und S-W15 a (gebaut S135):** die Spur *Strum*
  liegt zwischen der Zeile der Takte und den Noten (`strumOben` = `kKopfHoehe`,
  `rasterOben` = Kopf + Spur; *Note vol.* wieder ganz unten), mit Trennlinien
  oben und unten; ein Klick dort setzt nie die Abspielstelle. An jedem Schritt
  mit einer Note ein Pfeil, auch bei einer einzelnen (Follow hand → Down → Up),
  „T“, wo nur getippt wird. Die Hinweiszeile sagt „Strum lane (top)“ und
  „T = tapped“. **Belegt** in `test_LoopEditorSpielweise` (Fall J neu gefasst:
  Lage, einzelne Note, der Klick versetzt die Abspielstelle nicht); eng
  (42 Fälle mit `*Loop Menue*`) unter Testing-MSVC, `*Loop*,*Instrument*,*Drum*`
  (266 Fälle) unter Release-clang grün.
- **Gefunden im Programm (Patrik S135) und behoben:** der Instrument-Editor
  übernahm aus dem Raster nur die Noten (`ziel->noten = neu.noten`) — ein Klick
  in die Spur und das Menü *Strum* schalteten im Raster, der Editor sah „nichts
  geändert“ und zeichnete den alten Stand zurück; seit 24a ging auch ein Eintrag
  beim Ziehen einer Note nicht mit. Jetzt übernimmt er Noten und *Anschläge*.
  Die Tests hatten das Raster allein geprüft; neu ein Fall über den Editor wie
  im Programm (Klick, Menü, Verschieben). Dazu zeigt die Spur über einem
  Anschlag die Hand als Zeiger und einen Tooltip (`strumBei`). Eng (48 Fälle)
  unter Testing-MSVC, `*Loop*,*Instrument*,*Drum*` (267) unter Release-clang grün.

---

### 5r. Stufe 25a — Modell und Datei für Legato, Slide, Bend (gebaut S136)

Nach S-L1 a, S-L2 a, S-L4 a.

- **Artikulationen** (`Spielweise.hpp`): `kLegato` = 16, `kSlide` = 32, `kVerbunden`
  für beide; `kArtikulationenAlle` = 63.
- **Bend** (`BendPunkt`, `Note::bend`): Punkte aus `zeit` (0 … 1, Anteil der Note) und
  `halbtoene` (0 … `kBendHalbtoeneMax` = 4 seit S-L15 b, vorher 3, Raster `kBendRaster` = ¼); `richteBend`
  (Grenzen, Raster, sortiert), `bendBei` (linear zwischen den Punkten, davor und danach
  gehalten).
- **Datei:** `.lvloop` **Fassung 10** — der Bend als siebter Wert einer Note
  (`[[zeit, halbtoene], …]`), Saite und Artikulationen stehen dann davor. Ein älteres
  Programm verlöre Legato, Slide und Bend still.
- **Wandert mit:** der Bend ist Teil der Note — `kopiereNoten`, `fuegeNotenEin`,
  `versetzeNoten`, `dreheNoten` tragen ihn ohne Zusatz (belegt im Test).
- **Belegt** in `test_LoopSpielweise` „25a“ (Grenzen, Zeitverlauf, Datei hin und zurück,
  ein unbekanntes Bit, Mitwandern); sieben Prüfungen der Fassung auf 10 nachgezogen.
- **Von Claude gesetzt, nicht vorgelegt:** höchstens `kBendPunkteMax` = 16 Punkte;
  zweimal dieselbe Zeit — der spätere Punkt gilt (so steht ein Sprung, etwa am Anfang
  eines Pre-bends); eine Kurve ohne Biegung ist kein Bend (leer); `Note::bend{}` mit
  Vorgabe, damit clang bei `Note{…}` kein fehlendes Feld meldet.
- **Noch nicht:** Planung (25b), Klang (25c), Bedienung (mit den Tabs, S-L5 a).

### 5s. Stufe 25b — Planung der Ketten (gebaut S136)

Nach S-L1 a, S-L2 a, S-L6 a.

- **Saitenwahl** (`greifeNoten`): eine Note mit *Legato* oder *Slide* kommt auf die
  Saite ihrer **vorigen Note** (`Griff::vorige`) — der letzten, die vor ihrem Anschlag
  auf einer Saite anschlug, auf der beide spielbar sind; bei einer gesetzten Saite nur
  auf dieser. Die Saite ist von der vorigen belegt, die Note klingt in ihr weiter.
  Gibt es keine: `Griff::verbindungFehlt`, die Note wird gewählt wie bisher und schlägt
  normal an.
- **Ketten** (`kettenImMuster`): je Note, die von keiner kommt, eine Kette mit ihren
  Gliedern in der Reihe der Zeit.
- **Der Ton einer Kette** (`setzeKette`): `TonAngabe::verlauf` (je Glied ms ab dem
  Anschlag, Tonhöhe, Slide oder Legato), `TonAngabe::biegung` (die Bends aller Noten in
  ms, in Vierteln; zwischen zwei Noten hält der Bend, eine Note ohne Bend löst ihn
  springend), `dauerMs` bis zum Ende des letzten Glieds. Zeiten wie in der Planung
  (Schritte, Swing).
- **Name** nach S-L6 a, mit Rückweg in `zerlegeTonRest`.
- **Noch nicht eingeschaltet:** `toeneImKanal` plant weiter je Note — die Ketten nutzt
  es erst mit dem Klang (25c), damit kein Zwischenstand Töne verschluckt.
  `loop_werte.txt` unverändert.
- **Belegt** in `test_LoopSpielweise` „25b“ (5h7p5 auf der G-Saite statt der leeren
  H-Saite, fehlende Verbindung, Hammer-on nach einem Akkord von der nächsten Tonhöhe,
  Bend in ms, Name hin und zurück).
- **Von Claude gesetzt, nicht vorgelegt:** unter mehreren vorigen Noten die späteste,
  dann die nächste in der Tonhöhe, dann die tiefere Saite; eine Pause dazwischen
  trennt nicht (die Saite klingt nach); trägt eine Note Legato und Slide zugleich, gilt
  Slide; die Biegung hält vor ihrem ersten und nach ihrem letzten Punkt.

### 5t. Stufe 25c — der Klang verbundener Noten (im Bau S136)

Nach S-L7 a bis S-L11 a. Grundlage: Recherche 25 (`.claude/handover/recherche_25/`,
tragende Stellen selbst nachgelesen: PPV08 S. 46, Laurson 2001 S. 47, Välimäki 1995
S. 117, Özaslan 2010 S. 458).

- **25c-1 Lagrange (gebaut):** `leseLagrange` — sechs Abgriffe, Verzögerung im Fenster
  2 … 3, je Abtastwert; vor dem Schreiben eines Takts gilt der Abgriff ab 1. Für alle
  Töne mit *Extended KS* und *Waveguide*; *Classic* bleibt linear. Gemessen:
  `loop_werte.txt` 70 Zeilen (42 Instrument-Töne, 28 Loops mit Saiten), Summe −0,6 …
  +1,4 dB, Spitze bis +2,6 dB (Ukulele A5) — am stärksten hoch, wo die lineare
  Interpolation die Höhen am meisten dämpfte; Tonhöhe und Ring time unverändert (die
  Messfälle KS/WG grün). Hörproben vorher/nachher `.claude/handover/hoerproben_25c_lagrange/`.
- **25c-2 Fassung 12 (gebaut):** `Saitensatz::bundlos` (*Fretless*, in der Zeile Tuning,
  die Vorlage Violin setzt es), `Schicht::gleitMsJeHalbton` (*Slide time*, 5 … 60, Vorgabe
  20) und `Schicht::legatoLaut` (*Legato level*, 0 … 1, Vorgabe vorläufig 0,5 — bemessen
  in 25c-3) in *Playing*; `.lvinst` Fassung 12 (`strings.fretless` immer, `playing.slideTime`,
  `playing.legatoLevel`). Die mitgelieferten Instrumente bleiben bis 25c-3 in Fassung 11
  (gelesen mit den Vorgaben).
- **25c-3a der zeitvariable Umlauf (gebaut):** `Bahn` je Ton mit Verlauf oder Bend —
  Tonhöhe der Hand (Legato springt, Slide gleitet 20 ms je Halbton und endet am
  Zielschritt, mit Bünden stufig 43 % Halt / 57 % Übergang, ohne glatt; linear in
  Cent), Bend obendrauf; je gegriffener Tonhöhe ein eigener Umlauf, im Slide gemischt,
  nach einem Sprung in 1 ms (von Claude gesetzt, gegen Knacken der rekursiven Filter);
  Faktor beim Schreiben nach S-L8 a (Bend √ des Verhältnisses einen Umlauf früher,
  Slide −2 dB über die Gleitdauer). Anregung des Legato (S-L11 a): Waveguide ein Dreieck
  der Auslenkung mit der Spitze am Finger (vom Sattel her), Extended KS ein gekämmter
  Stoß; Hammer-on hart, Pull-off weich wie ein Finger (von Claude gesetzt). `Leser`
  trägt die laufende Länge, damit die Tonabnehmer am Ort bleiben; der Ring wird mit
  `vergroessereRing` auf die längste Länge gebracht. **Ohne Verlauf und Bend Bit für Bit
  wie 25c-1** (Loop Werte unverändert).
  **Gemessen** (`test_LoopSpielweiseKlang` „25c“): Legato A3→B3 ein Ton, Tonhöhe vorher
  und nachher auf 3 Cent; ohne Anregung kein Sprung (|·| < 1,5 dB); *Legato level*
  **0,38** gibt im Messfall (Achtel bei 100 BPM, Hammer-on und Pull-off über 1–3
  Halbtöne) einen Sprung im Median +3,04 dB (KS) / +3,07 dB (WG) — Vorgabe danach
  gesetzt. Slide A3→D4: Tonhöhe vorher/nachher auf 3 Cent, bis zum Beginn des Gleitens
  Bit für Bit wie ohne Slide, kein Pegelgewinn, bundlos ein anderer Verlauf. Bend um
  einen Ganzton: B3 auf 5 Cent; **−1,1 dB** gegen den ungebogenen Ton (gemessen an
  echten Saiten −0,3 dB, hergeleitet +0,5 dB) — vermutlich, weil der Verlust je Umlauf
  der gegriffenen Tonhöhe gilt und der gebogene Ton öfter umläuft; offen.
- **25c-3b die Planung spielt Ketten (gebaut):** `TonTeil` (alle Schichten, nur die
  Saite, ohne die Saite; Name `:p1`/`:p2`). `toeneImKanal`: der Kopf einer Kette spielt
  seine übrigen Schichten als eigenen Ton und bekommt einen Zusatzton
  (`GeplanterTon::zusatz`) — die Saite über die ganze Kette, mit Verlauf und Bend; an ihm
  hängen das Ende (die nächste Note auf der Saite nach der Kette) und *Let ring* (das der
  letzten Note). Die Glieder spielen nur ihre übrigen Schichten (*Own note*, S-L12 a),
  ohne Ende und ohne Strum. Eine Note mit Bend allein trägt den Bend im eigenen Ton.
  Rechenkern und `klangNamen` nehmen die Zusatztöne mit. Die mitgelieferten
  Saiteninstrumente in **Fassung 12** (Slide time 20, Legato level 0,38; Upright Bass
  und Pizzicato Violin *Fretless*). `loop_werte.txt` unverändert (kein mitgelieferter Loop
  hat Ketten). Belegt in `test_LoopSpielweise` „25c“ (Planung) und den Klangfällen;
  Hörproben der Gitarren-Proben mit Ketten unter `.claude/handover/hoerproben_25c_ketten/`
  (ohne: `hoerproben_proben/`).
- **25c-4 *Follow string* (gebaut, S137, S-L12 a, S-L13 a):** `Schicht::folgtSaite`
  (*In a legato*, `.lvinst` Fassung 12 `inALegato` `ownNote`/`followString`, immer an jeder
  Schicht außer der Saite; die mitgelieferten Saiteninstrumente aus
  `make_saiten_instrumente.py`); `istSaitenSchicht`, `folgtDerSaite`. `TonTeil::Folgend`
  (`:p1`, die Saite und was ihr folgt) und `TonTeil::Eigen` (`:p2`, *Own note*). Der Weg der
  Tonhöhe ist aus der `Bahn` als `Handweg` gelöst (Bit für Bit wie vorher); Sinus, Dreieck,
  Säge, Rechteck, Form, Sample und die Saite *Classic* folgen ihm, die Filter der Reihe
  folgen der klingenden Tonhöhe (beim Rauschen nur sie), die Bandgrenze richtet sich nach dem
  höchsten Griff; gleiten mit *Slide time* der ersten Saiten-Schicht. *Own note* spielt die
  Note, wie sie ist, auch beim Bend. Ohne Verlauf rechnet jede Schicht wie bisher (Loop Werte
  unverändert). Creator: Zeile *In a legato* im Kasten Source, „~“ in der Liste.
  **Gemessen** (`test_LoopSpielweiseKlang` „25c-4“): jede Schwingungsart nach dem Legato auf
  B3 (5 Cent), mit Bend auf B3. **Bend:** T60 im Halten gebogen 2,85 s gegen 3,20 s
  gegriffen (Faktor 0,891 = der Ganzton) — der Verlust je Umlauf gilt der gegriffenen Höhe;
  dazu das Gesetz √(Verzögerung) (−0,5 dB). Zusammen −1,1 dB. **Nach S-L14 a gebaut:**
  `Umlauf::grundJeUmlauf`, im Bend der Gewinn mal g₀^(P/P₀−1) — T60 im Halten gebogen
  3,20 s wie gegriffen, der Bend −0,61 dB (Extended KS) / −0,53 dB (Waveguide) gegen den
  ungebogenen Ton; ohne Bend Bit für Bit wie vorher (Loop Werte unverändert).

### 5u. Stufe 26a — Modell und Datei der Akkorde (gebaut S137)

- **`loop/Akkord.hpp`** (neu): `Akkord` — Titel, Grundton (0 … 11), Art, Herkunft
  (`computed`, S-T4 a), Beschreibung; als **Griffbild** `leer` (Stimmung), `buende`
  (−1 nicht gespielt, 0 leer, sonst ab dem Capo), `finger` (0 … 4), `barre`; als
  **Voicing** `abstaende` (aufsteigend ab 0, höchstens `kVoicingToeneMax` = 10 Töne in
  `kVoicingWeiteMax` = 48 Halbtönen — von Claude gesetzt). `richteAkkord`,
  `grundtonNamen` (C C# D Eb E F F# G Ab A Bb B — von Claude gesetzt), `grundtonAusName`
  (liest auch Db, A#, Cb, B#).
- **`.lvchord` Fassung 1** (`lumiviz-chord`): `title`, `root`, `type`, `origin` (nur wenn
  gesetzt), `description`, dann `strings` (`open`, `frets`, `fingers`, `barres`) oder
  `voicing`. Nicht gültig: fremdes Format, Fassung von morgen (`zuNeu`), ohne Grundton,
  ohne gespielte Saite, ohne Ton.
- **Ablage:** `mitgelieferterAkkordOrdner` (`asset/loop/chords`, mit Unterordnern, genannt
  beim Pfad darunter: `guitar/c_major_1`), `eigenerAkkordOrdner` (`chords`, `user:<name>`),
  `holeAkkord`, `setzeEigenenAkkordOrdnerFuerTests`.
- **Umrechnung:** `passtZu` (Griffbild: dieselben leeren Saiten, das Capo zählt nicht;
  Voicing: ein Instrument ohne Saiten), `passendeAkkorde`, `toeneDesGriffbilds` (mit Capo,
  verschoben: jeder gespielte Bund wandert, eine leere Saite wird gegriffen),
  `toeneDesVoicings`, `akkordNoten` (alle am Schritt, Lautstärke 1, das Griffbild setzt die
  Saite).
- **Datensicherung:** Bereich `chords` „Chords“ (13 Bereiche). **Publish:** `Art::Akkord`
  (`lvchord`) — nur als Hauptding, er nennt nichts und wird nicht genannt; Ordner im Paket
  `chords`.
- Belegt in `test_LoopAkkord` (7 Fälle, ohne Qt-Anwendung, Wegwerf-Ordner); die
  mitgelieferten Vorlagen kommen mit 26b (dann auch `asset/Asset.cmake`).

### 5v. Stufe 26b — die mitgelieferten Akkorde (gebaut S137)

- **`asset/loop/chords/make_chords.py <ChordsDb-Ordner>`** schreibt 6373 Dateien:
  `guitar/` 2069 und `ukulele/` 2114 Lagen aus chords-db 0.5.1 (npm `lib/guitar.json`,
  `lib/ukulele.json`; die Originale außerhalb des Repos unter `…\cmake\ChordsDb\0.5.1`).
  Gelesen: die Bünde stehen dort ab `baseFret` (`f > 0` → `baseFret + f − 1`, die Barres
  ebenso); jede Lage trägt `midi` — die Umrechnung wird gegen sie geprüft, alle 4183 Lagen
  stimmen. Gitarre 529 Akkorde in 63 Arten (darunter 22 Slash-Akkorde), Ukulele 552 in 46.
- **Gerechnet** (S-T4 a, `origin` `computed`): `drop_d/` 468, `bass/` 426, `violin/` 420 —
  13 Arten (von Claude gewählt: major, minor, 7, maj7, m7, sus2, sus4, dim, aug, m7b5, 6, m6,
  add9), je Grundton bis zu drei Lagen. Regel: nur Töne der Art, alle außer der Quinte bei
  vier Tönen, der Grundton als tiefster, klingende Saiten ohne Lücke (mindestens drei),
  höchstens vier Bünde, höchstens vier Finger (Barre am tiefsten Bund = einer), Wertung:
  mehr klingende Saiten, mehr leere, tiefere Lage. **Bildbogen zur Durchsicht:**
  `.claude/handover/akkorde_26b/bogen.html` (Kipp-Bedingung S-T4 a).
- **Voicings** `voicings/` 876: 19 Arten × 12 Grundtöne — Grundstellung, Umkehrungen (bis
  zur Oktave), weite Lage (Dreiklang: Grundton, Quinte, Terz + 12) oder Drop 2 (Vierklang).
- Namen `<grundton>_<art>_<lage>` (`c_major_1`, `cs_m_over_b_2`, `g_7_inv1`); Titel als
  Akkordsymbol (`C`, `Cm`, `G7`, `C/E`); Beschreibung „Position n, open“ oder „…, from fret n“.
- `CREDITS.md` mit dem MIT-Lizenztext (wortgleich mit `LICENSE` geprüft),
  `THIRD_PARTY_NOTICES.md` (neuer Abschnitt), `asset/Asset.cmake` (sechs Ordner und
  `CREDITS.md`). **`AssetAbgleich.cmake`:** das Entfernen am Ziel mit `list(REMOVE_ITEM)`
  statt `IN_LIST` je Datei — vorher 35 s je Bau mit den Akkorden, jetzt 1,9 s.
- Icon **`lumiviz_lvchord.ico`** (Marke mit Griffbild, `zeichen_griffbild`), eingetragen in
  `register_file_icons.ps1` (`LumiViz.Chord`) und im README der Icons, PNG fürs Handbuch.
- Belegt in `test_LoopAkkord` „26b“: Zahl je Ordner, jede Datei liest sich, jedes Griffbild
  greift auf seiner Stimmung, die gerechneten nur mit Tönen ihrer Art und dem Grundton im
  Bass, `mitgelieferteAkkorde` findet alle (1,4 s unter Testing-MSVC).

### 5w. Stufe 26c — die Tab-Linien im Instrument-Editor (gebaut S137)

- **A** im `NotenRaster` ein Streifen zwischen Spur Strum und Noten (`tabOben`,
  `tabHoehe` = Saiten × `kTabZeile` 14 px + 2 × `kTabRand` 4 px; `rasterOben` rückt darunter),
  je Saite eine Linie, oben die höchste (`tabLinieY`, `tabSaiteBei`); links „1 E“ … „6 E“
  (Nummer und Ton der leeren Saite, mit Capo der klingende). Rollt und zoomt waagrecht mit.
- **B** an jeder Note der Bund aus der Planung (`greifeNoten`), am Anschlag (mit Strum-Versatz),
  in einem Feld, das die Linie unterbricht; markierte hervorgehoben, nicht greifbar rot „!“;
  dahinter ein blasser Strich über die Dauer. **Bundlos: der Tonname statt der Bundzahl** (von
  Claude gesetzt — S-L9 a sagt nur „keine Bundzahl“). Bedienung: ein Klick auf eine Linie setzt
  die **Tab-Marke** (`setzeTabMarke`, ein Kästchen); eine Ziffer setzt dort den Bund
  (`setzeBund`): liegt am Schritt schon eine Note auf der Saite, bekommt sie ihn, sonst eine neue
  von einem Schritt; die Saite steht danach an der Note (S-M2 a). **Zwei Ziffern binnen
  `kTabZifferMs` = 800 ms werden ein Bund** (1, 2 → 12; von Claude gesetzt), über 24 beginnt die
  zweite neu. Ein Klick auf eine Zahl markiert ihre Note und setzt die Marke dorthin; ziehen auf
  eine andere Linie (`Zug::TabSaite`, gestrichelt die Ziellinie) legt sie beim Loslassen auf diese
  Saite (`legeAufSaite`, derselbe Ton). Ein Klick auf den Namen einer Saite spielt sie leer an;
  Escape nimmt zuerst die Tab-Marke weg. Zeiger: Hand, Pfeil hoch/runter auf einer Zahl,
  Textcursor auf einer leeren Stelle; Tooltips dazu.
- **F** Haken **Tab** am Ende der Zeile *Strum* (`editorTab`, nur mit Saiten; neben, nicht in
  `StrumZeile`, die auch im Creator steht); gemerkt unter `loop/instrumentEditorTab` — wie die
  Lage nicht in der Test-Exe.
- **G** die Hinweiszeile nennt die Tab-Linien.
- Belegt in `test_LoopEditorTab` (2 Fälle über den Editor; `LUMIVIZ_EDITOR_TAB_BILD`); in
  `test_LoopEditorSpielweise` drei Rechnungen der Höhe um `tabHoehe()` ergänzt.
- **Noch nicht** (26d): Hammer-on, Pull-off, Slide, Bend in Linien und Piano-Roll; Zahlen
  waagrecht ziehen (die Zeit bleibt Sache der Piano-Roll).

### 5x. Stufe 26d — Legato, Slide und Bend im Editor (gebaut S137)

- **Modell** (`Spielweise.hpp`): `BendVorlage` (None, Bend, Bend and release, Pre-bend,
  Pre-bend and release), `bendVorlage(art, halbtoene)` — **Zeiten von Claude gesetzt:** Bend
  steigt bis 0,3 der Note; Bend and release 0,3 / hält bis 0,6 / zurück bei 0,9; Pre-bend ein
  Punkt am Anfang; Pre-bend and release hält bis 0,5, zurück bei 0,8 —, `bendVorlageVon`,
  `bendHoehe`, `bendText` (in Ganztönen wie in Tabs: ½, full, 1½, 2; dazwischen Achtel).
- **E Menü:** unter Palm mute, Let ring, Tapping, Harmonic die Haken **Legato (hammer-on /
  pull-off)** und **Slide** (`schalteArtikulation`; sie schließen sich aus). **F** Untermenü
  **Bend**: die fünf Vorlagen, **Height ½ / full / 1½ / 2**, **Edit bend...**; Haken zeigen, was alle
  markierten tragen (`setzeBendVorlage`, `setzeBendHoehe` — eine Vorlage behält ihre Form, eine
  eigene Kurve wird gestreckt, ohne Bend wird es *Bend*). Das Menü öffnet sich auch auf einer
  Zahl der Tab-Linien (sie wird dafür markiert).
- **G Edit bend...** (`bendDialog`, über `fuehreDialogAus`): Auswahl *Template* und *Height*,
  die Kurve `BendKurve` (`loop/BendKurve.hpp`: Zeit über die Länge der Note, Höhe 0 … 4
  Halbtöne in Vierteln, beschriftet ½/full/1½/2; Punkte ziehen, zwischen den Nachbarn;
  Doppelklick setzt oder entfernt einen); eine eigene Form zeigt keine Vorlage.
- **Zeichen, Piano-Roll (S-L5 a, S-T5 a E):** hinter der Saitennummer **H**/**P** (nach der
  Richtung; ohne vorige „H/P“), **SL**, **↗** mit Höhe; Hammer-on und Pull-off als Bogen über
  beide Noten mit „h“/„p“, ein Slide als Linie mit Pfeil vom Ende der vorigen zur Zielnote; ohne
  vorige Note auf der Saite ein rotes „!“ davor (`verbindungFehlt`). Der Bend als Kurve in der
  Note bis zur Zeile des erreichten Tons, gestrichelt das Ziel, mit der Höhe.
- **Zeichen, Tab-Linien (S-T5 a C):** Bogen mit h/p zwischen den Zahlen, „/“ oder „\“ beim
  Slide, ein Pfeil hinauf mit der Höhe hinter der Zahl, rot „!“ ohne vorige.
- Hinweiszeile um H/P, SL, ↗ ergänzt.
- Belegt in `test_LoopEditorVerbunden` (2 Fälle: Vorlagen und Schrift; Menü, Bend, *Edit
  bend...* und Tab-Linien über den Editor, `LUMIVIZ_EDITOR_VERBUNDEN_BILD`); in
  `test_LoopEditorSpielweise` die Liste der Haken um Legato und Slide ergänzt.
- ✅ **S-L15 b** (Patrik S137): **Die Grenze eines Bends wird 4 Halbtöne** (2 Ganztöne, wie
  S-L3 a F): das Menü bietet ½, full, 1½ und 2, *Edit bend...* geht bis 2 Ganztöne, ebenso die
  `.lvloop` (Fassung 10 ist nicht ausgeliefert — keine neue Fassung). Verworfen: a (bei 3
  Halbtönen bleiben — Overbends wären nicht zu setzen). **Gebaut S138:** `kBendHalbtoeneMax` 4, `bendHoehen`
  ½/full/1½/2, der Dialog bis 2 (Raster und Beschriftung folgen der Grenze); die Klangseite
  kannte keine eigene Grenze. Belegt in `test_LoopEditorVerbunden` (Schrift „2“, Vorlagen bis 4,
  *Height 2* über das Menü, vier Höhen im Dialog) und `test_LoopSpielweiseKlang` (Bend um zwei
  Ganztöne: C♯4 im Halten auf 5 Cent, Ring time wie C♯4 gegriffen auf 3 %, beide Verfahren). Gemessen: T60 gebogen 3,200 s gegen
  gegriffen 3,200 s; Pegel −1,09 dB (Extended KS) / −1,00 dB (Waveguide) gegen A3 ohne Bend — das
  Gesetz √(Verzögerung) aus S-L8 a ergibt für vier Halbtöne −1,0 dB.
- **Offen:** bei kurzen Noten in Normalgröße überdeckt die Höhe des Bends in den Tab-Linien die
  nächste Zahl.

### 5y. Stufe 26e-1 — Griffbild zeichnen, Namen erkennen (gebaut S138)

Teilschritte von 26e (von Claude gesetzt, nach S-P1 a je eng unter beiden Compilern): 26e-1
Zeichnen und Namen ohne Fenster, 26e-2 das Fenster *Chords* mit Knopf und Einfügen, 26e-3 der
Dialog *Edit chord*.

- **`Akkord.hpp`:** `verschobenesGriffbild` (S-T8 a F; `toeneDesGriffbilds` rechnet seither
  darüber, Bit für Bit dieselben Töne), `AkkordArt` und `akkordArten()` — 45 Arten nach
  Lehrbuch mit den Namen von chords-db, alle Arten der Vorlagen außer `alt` und `7sg`;
  `AkkordName`, `erkenneAkkord`, `akkordArtText`, `akkordSymbol` (S-T11 a L). **Regel (von
  Claude gesetzt):** die Tonklassen bilden genau eine Art auf dem Bass; sonst Dur oder Moll
  über einem anderen Bass als Slash (C/E, A/G♯); sonst eine Art ab vier Tönen ohne Quinte;
  je Stufe die Reihenfolge der Tabelle. C-E-A heißt damit Am/C, nicht C6 ohne Quinte.
- **`loop/GriffbildBild`** (neu): `griffbildFenster` (mindestens vier Bünde, am Sattel solange
  der höchste gegriffene hineinpasst, sonst ab dem tiefsten), `GriffbildLage` (Saiten, Linien,
  Punkte, × / ○, `trifft` für die Klicks des Dialogs), `zeichneGriffbild` klein und groß
  (Farben aus der Palette; die Bundzahl links neben Punkt und Balken).
- Belegt in `test_LoopGriffbild` (5 Fälle; `LUMIVIZ_GRIFFBILD_BILD` legt einen Bogen mit 20
  Gitarrenlagen klein und groß ab). Gemessen an den Vorlagen: gerechnete Griffbilder 1314
  mit Grundton im Bass alle mit ihrer Art, davon 74 ohne Quinte als Slash gehört (ein C6 ohne
  Quinte ist Am/C — dieselben Töne); Voicings in Grundstellung 372 / 372, Dur- und
  Moll-Umkehrungen als Slash; chords-db vollständig mit Grundton im Bass 1322 / 1322; über
  alle 4095 Lagen von chords-db 1714 mit ihrem Namen (die übrigen ohne Grundton im Bass, ohne
  Grundton, mit Optionen außerhalb der Art).
- **Befund chords-db** (Prüfung S138 gegen die Lehrbuch-Arten): rund 30 Lagen greifen Töne
  außerhalb ihrer Art (etwa `guitar/bb_6_4` mit kleiner Terz und großer Septime,
  `guitar/g_add9_2`, `ukulele/f_11_1`); `alt` greift immer Grundton, Terz, verminderte Quinte,
  `7sg` (drei Lagen) wie ein Septakkord. Vorgelegt als S-T12 (b: bleiben, Zeichen *unusual*
  im Fenster — gebaut mit 26e-2).
- **Nach S-T13 a:** die Arten in `asset/loop/chords/types.json` (Format `lumiviz-chord-types`,
  Fassung 1; in `asset/Asset.cmake`); `ladeAkkordArten`, `akkordArten()` liest sie einmal (fehlt
  sie, erkennt `erkenneAkkord` nichts); Dur und Moll für den Slash beim Namen gesucht.
  `make_chords.py` liest `ABSTAENDE` aus derselben Datei — in einer Kopie gelaufen, alle 6373
  Dateien byte-gleich. Belegt im Fall „die Datei der Arten" (`test_LoopGriffbild`, zehn
  ungültige Dateien).

### 5z. Stufe 26e-2 — das Fenster *Chords* (gebaut S138)

- **`loop/AkkordFenster`** (neu) nach S-T8 a A–G: Kopf *For:* (Instrument, Muster, Stimmung,
  Capo), *Search* (über alle Akkorde der Stimmung, gleich welcher Ordner) und *Type*; der Baum
  (★ *Favorites*, *Recently used*, die Grundtöne mit ihren Arten und Zahlen, *Your chords*);
  die Galerie `AkkordGalerie` (je Lage ein kleines Griffbild, Köpfe „C major · 4", darunter
  Symbol und Lage, Tooltip); die Leiste mit `GriffbildFeld` (groß), Titel und Lage, Tönen,
  *computed* und *unusual: … not in …* (S-T12 b, `fremdeTonklassen`), *Move −1 / +1 fret*, den
  vier Symbolen ↓ ↑ ↓↑ = (S-T9 a; die gewählte bleibt, gemerkt unter `chordWindow/playAs`,
  S-T10 a; in der Test-Exe nicht), ★ und **Use**. Favoriten und Zuletzt (höchstens 12) als
  Nennung der Lage. Ein Klick auf ein anderes Bild wählt es (Verschiebung 0) und spielt an.
- **Anhören:** `akkordZumAnhoeren` (Down, Up, Together; Down-up mit Up eine halbe Zählzeit
  später; zwei Schläge lang — von Claude gesetzt), Weg Editor → Panel über
  `akkordVorhoerenVerlangt` / `spieleAkkordVor` — der Weg behält die Schritte der Noten (der
  Weg des Creators legt bewusst alles auf Schritt 0 und bleibt so).
- **Einfügen:** Knopf **Chords...** in der Zeile *Instrument* (nur mit Saiten);
  `NotenRaster::setzeAkkord` setzt an der Einfügemarke (Use, Doppelklick) oder am Schritt unter
  der Maus (Ziehen, Mime `application/x-lumiviz-chord` mit Nennung und Verschiebung) — je Note
  ihre Saite, ein Schritt lang wie eine getippte Zahl (26c); was am Schritt auf einer seiner
  Saiten liegt oder einen seiner Töne spielt, weicht; ein Schritt für Undo („Chord C").
- Belegt in `test_LoopAkkordFenster` (3 Fälle: Anhören je Spielart, fremde Töne, Lage; das
  Fenster mit Baum, Galerie, Leiste, Klick, ★, Zuletzt, Suche, Type, *unusual*, *computed* bei
  Drop D; über den Editor *Chords...*, Use an die Einfügemarke, Ziehen ins Raster;
  `LUMIVIZ_CHORD_WINDOW_BILD`) und in `test_LoopSpieler` (`spieleAkkordVor`). Bild angesehen.
- Noch nicht: *New/Edit/Duplicate/Delete* (H) — mit 26e-3, dem Dialog *Edit chord*.

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

### 6c. Saitenabstände und Spread (S-W16 a, S136)

Recherche S136 (Agent; lokal unter `.claude/handover/recherche_24f/bericht.md`
mit allen Zitaten und Händlerseiten). Von Claude an der Quelle nachgelesen:
Freire 2018 S. 545 „a Yamaha nylon acoustic guitar (model CG182S)“; Barbancho
et al., „Estimation of the Direction of Strokes and Arpeggios“, ISMIR 2014,
S. 43 „because of human physiology, in most cases the stroke is performed with
fixed speed“; Traube/Smith, WASPAA 2001, S. 1 „The neutral position of the right
hand is just behind the sound hole“; Yamaha Singapur, Produktseite CG182S:
„String Spacing“ 11.8mm, „Nut Width“ 52mm, „Scale Length“ 650mm (bei 52 mm
Sattelbreite ist der Abstand am Steg gemeint).

**Rechnung:** Abstand benachbarter Saiten am Anschlagort, linear zwischen Sattel
und Steg bei einem Fünftel der Mensur vom Steg (angenommen; bei 0,15 statt 0,2
ändert sich der Abstand um 1,4 %). An der CG182S: 11,8 mm (Steg), 8,64 mm
(Sattel, Graph Tech 6200-00, 43,2 mm ÷ 5) → 11,17 mm; mit 8 ms ergibt das eine
Hand von 1,40 m/s. **Spread = 8 ms × Abstand ÷ 11,17 mm**, auf 0,5 ms gerundet.

| Bauart | Abstand am Anschlagort | Spread | Quellen (Steg / Sattel) | Stand |
|---|---|---|---|---|
| Klassik/Nylon (CG182S) | 11,17 mm | 8,0 ms | Yamaha / Graph Tech 6200-00 | Steg gelesen |
| Steelstring (Martin D-28, 00, 000) | 10,31 mm | 7,5 ms | Martin „Bridge String Spacing: 2 5/32"“ / Graph Tech M175-00 | über den Agenten |
| Strat, Vintage-Tremolo (2-7/32") | 10,42 mm | 7,5 ms | StewMac, Fender Road Worn / Graph Tech 5000-00 | über den Agenten |
| Strat, 2-Punkt-Tremolo (2-1/16") | 9,79 mm | 7,0 ms | StewMac, Fender / Graph Tech 5000-00 | über den Agenten |
| Les Paul (ABR-1, 2-1/16") | 9,81 mm | 7,0 ms | StewMac / Graph Tech 6010-00 | über den Agenten |
| Jazz-Archtop (2.125") | 10,07 mm | 7,0 ms | StewMac (Fishman-Steg, kein Werkssteg) / Graph Tech 6010-00 | über den Agenten |
| E-Bass Precision / Jazz | 17,35 / 17,21 mm | 12,5 ms | StewMac „2-1/4" (57.15mm)“ / Graph Tech 1204-00, 1215-00 | über den Agenten |
| Ukulele Konzert | 13,0–13,1 mm | 9,5 ms | Martin 0XK „1 41/64''“ / Graph Tech 1257-00, 1237-00 | über den Agenten |
| Kontrabass | 24,4 mm | 17,5 ms | No Treble (**Blog**): „10 mm at the nut and 28 mm at the bridge“ | S-W19 a |
| Violine | 10,02 mm | 7,0 ms | Luthiers Forum (**Forum**): Steg „G - E 33-34mm“, Sattel „16 - 16.5mm“ | S-W19 a |

**Lücken:** wo Yamaha „String Spacing“ misst, steht dort nicht (Steg aus der
Sattelbreite geschlossen); alle Sattelwerte von Ersatzsätteln (Graph Tech), keiner
vom Hersteller; der Anschlagort nur an der Nylongitarre gestützt; dieselbe
Geschwindigkeit der Hand über alle Bauarten ist unbelegt (Freire misst nur die
Nylongitarre mit Plektrum); Kontrabass und Violine nur aus Blog und Forum
(Strobel, 29 / 11 mm → 18 ms, nicht eingesehen).

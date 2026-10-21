# LumiViz — Spielweisen der Instrumentfamilien (Konzept)

> **Version:** 0.23.0
> **Datum:** 2026-10-21 (Session 139: Zielbild, SF1 c, SF2 a, SF3 a, SF4 a, SF5 d;
> Session 140: SF6 a bis SF10 a, Skizze Stufe 1, SK1 a bis SK11 a; Session 141: SK12 a)
> **Typ:** Konzept — Ziel, Modell, Klang. Entschieden sind SF1 c bis SF10 a und SK1 a bis SK12 a (Patrik S139–S141);
> alles andere ist offen oder Vorschlag und so bezeichnet.
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Saiten_Konzept.md](Saiten_Konzept.md) (die Saite als erste Bauart,
> ihre Spielweisen S-W, S-L, die Tabs S-T) · [Loop_Konzept.md](Loop_Konzept.md)
> §4f (Instrumente aus Schichten) · `Offene_Punkte.md` §7 (Idee Patrik S138)
> **Sprache:** Deutsch

---

## 1. Zielbild (Patrik S139)

**Ziel ist besserer Realismus.** So wie die Saiten Bend, Tapping und Palm mute
haben, bekommen die übrigen Instrumentfamilien ihre eigenen Spielweisen — und
zwar **mit Klang, gerechnet nach echten Instrumenten**, nicht als Näherung.

Die Familien und Spielweisen, die Claude vorgeschlagen hat (Patrik: „alles scheint
gut"); die Liste ist der Umfang, nicht schon die Reihenfolge:

| Familie | Spielweisen |
|---|---|
| Klavier und Tasten | Sustain-Pedal, Sostenuto, Una corda; Orgel: Register, Leslie langsam/schnell |
| Blechbläser | Dämpfer (Straight, Cup, Harmon, Wah-Wah), Fall, Doit, Growl |
| Holzbläser | Zungenstoß einfach/doppelt, Atem- und Luftgeräusch, Flatterzunge |
| Streicher | Pizzicato, *con sordino*, Tremolo, Spiccato, Col legno, Vibrato |
| Für alle | Vibrato, Crescendo/Swell innerhalb einer Note |
| Schlagwerk | Wirbel, Abdämpfen mit der Hand (Becken, Vibraphon) |

**Drei Arten von Spielweisen** (Vorschlag Claude S139, zum Ort im Modell; als Regel
entschieden mit SF8 a):

| Art | Beispiele | Ort |
|---|---|---|
| Bauart — was das Instrument ist | Hammer-Saite, Lippe und Rohr, Bogen-Saite | Schicht und Instrument (SF1 c) |
| Zustand über der Zeit | Pedale, Dämpfer, Register, Leslie | Spur mit Umschaltstellen, wie Strum |
| Art einer Note | Pizzicato, Fall, Doit, Zungenstoß, Tremolo | Bits an der Note, wie Palm mute |

Ein Dämpfer steckt nicht die ganze Zeit (Frage Patrik S139): er wird für eine
Passage eingesetzt (*con sord.*) und in einer Pause herausgenommen (*senza sord.*).
Das Umstecken braucht Zeit — wie viel, ist noch zu belegen; ob das Programm sie
verlangt, warnt oder übergeht, ist offen.

✅ **SF1 c** (Patrik S139): **Zwei Gruppen.** (1) **Steuer-Spielweisen** hat jedes
Instrument, auch ein Synth: Sustain-Pedal (Noten halten), Ausdruck/Swell, Vibrato als
Tonhöhenverlauf — die genaue Liste in einem eigenen Entscheid. (2) **Klang-Spielweisen**
folgen aus der **Bauart** einer Schicht — Hammer-Saite, Lippe und Rohr, Rohrblatt und
Rohr, Bogen-Saite, Platte und Stab, dazu die heutige Saite; was am ganzen Instrument
hängt, steht dort wie der Saitensatz. Trägt ein Instrument ein Modell, **rechnet das
Modell auch die Steuer-Spielweisen echt** (das Sustain-Pedal am Klavier hebt die Dämpfer,
die freien Saiten schwingen mit); am Synth hält es nur die Noten. Schichten ohne Modell
folgen der Modellschicht über einen Schalter je Schicht, wie *Follow string*. Die
Familie steht **nicht** als Feld in der Datei; wo sie gezeigt oder gefiltert wird, liest
das Programm sie aus den Bauarten ab. Verworfen: a (nur aus der Bauart — das
Sustain-Pedal fehlte jedem Synth, obwohl jedes Keyboard es hat), b (Feld `family` als
Quelle — zwei Angaben, die sich widersprechen können, und Klang-Spielweisen ohne
Rechnung). *Die Liste der Bauarten ist mit SF10 a neu gefasst: Paar aus Anregung und
Schwinger.*

✅ **SF2 a** (Patrik S139): **Die Bauart bestimmt, welche Steuer-Spielweisen ein
Instrument mit Modell hat.** Jede Bauart nennt die, die ihr echtes Vorbild hat, und
rechnet nur diese — Hammer-Saite Sustain, Sostenuto, Una corda; Saite Vibrato und Swell;
Lippe und Rohr Vibrato und Swell, kein Pedal. Schichten ohne Bauart (Synths) haben alle
Steuer-Spielweisen. Kein Schalter. Damit gilt SF1 a, wo ein Modell rechnet, und SF1 c am
Synth. Verworfen: b (Schalter *Controls* *As built* / *All* — eine zweite Angabe neben der
Bauart; Unechtes geht mit einer Synth-Schicht, die der Modellschicht folgt), c (alle an
jedem Instrument — am Modell Spielweisen, die nicht echt klingen können). Kippt, wenn
Hörproben zeigen, dass eine Familie ohne Vorbild Pedal oder Swell musikalisch braucht.

✅ **SF3 a** (Patrik S139): **Fünf Steuer-Spielweisen — die Regler eines Keyboards:**
**Sustain** (Noten klingen weiter, bis das Pedal losgelassen wird), **Sostenuto** (nur die
beim Treten klingenden Noten werden gehalten), **Expression** (Lautstärke über der Zeit,
Swell), **Vibrato** (Tiefe über der Zeit, wie das Modulationsrad), **Pitch** (Tonhöhe über
der Zeit, wie das Pitch-Rad). Una corda, Dämpfer und Register sind Klang-Spielweisen ihrer
Bauart. Verworfen: b (nur Sustain, Expression, Vibrato — das Zielbild will alle drei
Klavierpedale, und ohne Pitch gäbe es an Synth und Bläsern keinen Fall oder Doit), c (die
ganze Reglerbelegung — das Soft-Pedal hat am Synth keine feste Bedeutung, Aftertouch und
Breath sind Eingabewege, gerechnet Expression oder Vibrato). Kippt nicht bei Live-MIDI:
dann kommen Aftertouch und Breath als **Eingänge** auf diese Spuren dazu.
*Beleg MIDI-Belegung* (MIDI Association, „MIDI 1.0 Control Change Messages", Table 3,
abgerufen 2026-10-20, Wortlaut): 1 „Modulation Wheel or Lever", 2 „Breath Controller",
11 „Expression Controller", 64 „Damper Pedal on/off (Sustain)", 66 „Sostenuto On/Off",
67 „Soft Pedal On/Off". Pitch Bend steht nicht in der Tabelle (eigene Nachricht — aus
dem Gedächtnis, auf der Seite nicht belegt).

✅ **SF4 a** (Patrik S139): **Das Vibrato der Schicht ist die Vorgabe des Instruments;
wo die Spur Vibrato Werte hat, setzt sie die Tiefe** — auch auf 0. Rate und Einsatz kommen
immer von der Schicht (`vibratoRate`, `vibratoDelay`); ohne Spur klingt alles wie heute.
Mit einer Bauart rechnet das Modell das Vibrato echt (an der Saite über die Spannung, wie
der Bend); Rate, Tiefe und Einsatz der Schicht sind dann die Vorgabe des Spielers.
Verworfen: b (die Spur addiert — gerade spielen und das Vibrato kommen lassen ginge
nicht), c (Vibrato nur noch in der Spur — der Klang des Instruments hinge am Loop, alle
mitgelieferten Dateien umzuschreiben). Kippt, wenn die Spur auch die Rate führen soll.

✅ **SF5 d** (Patrik S139; zuerst a, dann „c als Zusatz?" — daraus d): **Fünf Spuren je
Muster** im Instrument-Editor, wie Strum — unter den Noten, einzeln ein- und
ausklappbar, in der `.lvloop` (*der Ort im Editor ist mit der Skizze §1a ersetzt: oben,
unter Strum; Ein- und Ausblenden über Haken, SK1 a*); **Sustain** und **Sostenuto** als Stufen 0 … 1 (auch das
halbe Pedal, wie am Flügel — noch zu belegen), **Expression**, **Vibrato**, **Pitch** als
Kurven; eine leere Spur = Vorgabe des Instruments. **Dazu am Clip im Loop-Panel ein
Verlauf Expression** über die ganze Länge des Clips mit allen Wiederholungen, mit der
Spur des Musters **malgenommen** (ohne ihn 1). Pedale, Vibrato und Pitch nur im Muster.
Verworfen: e (alle fünf am Clip, überschreibend — zwei Ablagen für denselben Wert),
a (nur im Muster — kein Bogen über Wiederholungen; der `LoopClip` hat heute keine
Lautstärke), b (Kurven je Note — Fall und Doit sind schon Klang-Spielweisen der Note),
c (nur am Clip — dasselbe Muster klänge je nach Ort anders). Offen: ein Pedal, das am
Ende des Musters gedrückt ist, über die Clipgrenze halten (Regel, eigener Entscheid);
wächst ein Vibrato über ein Solo, wäre der nächste Schritt ein malgenommenes Vibrato am
Clip, nicht e.

**An den Saiten** (Antwort Claude S139, Vorschlag): Vibrato über der Zeit (über die
Spannung, wie der Bend) und Swell; die **Bogen-Saite** als neue Bauart auf der Saite
(Saitensatz, Korpus) für `violin`, `strings`, `upright_bass` — Pizzicato wird dann eine
Spielweise der Geige; das Dämpferpedal am Hackbrett (`dulcimer`). Wie das neue Vibrato
sich zur festen Einstellung je Schicht verhält: SF4 a.

✅ **SF6 a** (Patrik S140): **Die Bauart ist die Obergrenze, die Ausstattung je
Instrument entscheidet.** Jede Bauart nennt die Steuer-Spielweisen, die ihr Vorbild haben
kann (Tabelle unten); was ein Instrument davon wirklich hat, steht in seiner
**Ausstattung**, wie `bundlos` am Saitensatz: *Damper* an Platte und Stab (mit
Dämpferleiste dämpft das Loslassen und das Pedal hält; ohne klingt jede Note frei aus,
die Spur Sustain fehlt), *Sostenuto* an der Hammer-Saite, *Tremolo arm* an der Saite.
Expression an der Saite steht nicht als eigenes Feld in der Datei — das Programm liest sie
aus den Tonabnehmern ab (ein Tonabnehmer: ja; `none`: nein). Eine Spur erscheint im
Instrument-Editor nur, wo Bauart und Ausstattung sie tragen. Die Ausstattung ist Teil des
Modells und wird gerechnet — keine zweite Angabe neben der Bauart (SF2 b), sondern der Bau
selbst. Verworfen: b (alles, was die Bauart haben kann, an jedem Instrument — die Marimba
bekäme ein Pedal, das sie nicht hat), c (nur, was alle Vorbilder der Bauart haben — dem
Vibraphon fehlte das Pedal, das sein Klang braucht). Kippt, wenn die Recherche je Familie
zeigt, dass die Ausstattung innerhalb einer Bauart kaum schwankt (dann wäre b gleichwertig);
ein Beleg gegen eine Zeile ändert nur die Zeile.

| Bauart | Sustain | Sostenuto | Expression | Vibrato | Pitch |
|---|---|---|---|---|---|
| Hammer-Saite (Klavier) | ja | Ausstattung (Flügel) | nein — nach dem Anschlag nicht steuerbar | nein | nein |
| Saite, gezupft (heute) | nein | nein | Ausstattung (Tonabnehmer: Knopf, Pedal) | ja (Finger) | Ausstattung (Vibratohebel) |
| Bogen-Saite | nein | nein | ja (Bogen) | ja | ja (Glissando) |
| Lippe und Rohr | nein | nein | ja | ja | ja (Lippe, Posaunenzug) |
| Rohrblatt und Rohr | nein | nein | ja | ja | ja (Bend, Glissando) |
| Platte und Stab | Ausstattung (Dämpferleiste) | nein | nein | nein — der Motor des Vibraphons ist Klang-Spielweise | nein |
| ohne Bauart (Synth) | ja | ja | ja | ja | ja (SF2 a) |

*Herkunft der Tabelle:* aus dem Gedächtnis (Claude S140), nicht belegt — jede Zeile wird
in der Recherche ihrer Familie belegt. *Mit SF10 a gilt sie je Paar aus Anregung und
Schwinger; die Tabelle dort ersetzt diese.*

✅ **SF7 a** (Patrik S140): **Die Spuren aller Schichten zusammen; eine Schicht mit
*Follow* folgt bei den gemeinsamen und nimmt die übrigen selbst.** Der Instrument-Editor
zeigt jede Spur, die mindestens eine Schicht trägt. Die Modellschicht trägt, was Bauart
und Ausstattung ihr geben (SF6 a), und überhört den Rest. Eine Synth-Schicht ohne *Follow*
trägt alle fünf und rechnet sie für sich. Eine Synth-Schicht mit *Follow* richtet sich bei
den Spuren, die auch die Modellschicht trägt, nach deren Ergebnis — sie hält, solange die
Saite des Modells klingt, und gibt mit dessen Dämpfer frei —, die übrigen rechnet sie
selbst. So kommt die Tonhöhe über der Zeit in ein Klavier, wie SF2 a es vorsieht: Pitch
wirkt nur auf die Pad-Schicht. Verworfen: b (nur die Modellschicht bestimmt die Spuren —
schließt den Weg für Unechtes aus SF2 a), c (eine Schicht mit *Follow* nimmt nur die
Spuren des Modells — Unechtes nur mit einer Schicht, die nicht mit dem Pedal hält; *Follow
string* übernimmt schon heute nur die Tonführung, nicht den Klang). Kippt, wenn im Sichttest
eine Spur verwirrt, die nur einen Teil der Schichten erreicht — dann eher eine Anzeige im
Kopf der Spur, welche Schichten sie erreicht, als b.

✅ **SF8 a** (Patrik S140): **Drei Orte der Klang-Spielweisen, gewählt danach, wie oft die
Spielweise wechselt.** Jede hat genau einen Ort. **Nie** (wie das Instrument gebaut ist,
ein fester Registersatz, die Dämpferart, die ein Spieler mitbringt): in der Ausstattung
von Schicht oder Instrument, wie `bundlos`. **Über eine Passage** (Una corda, Dämpfer,
Register, Leslie langsam/schnell, Motor an/aus): eine Spur je Muster mit Umschaltstellen,
wie Strum — sie erscheint nur, wo Bauart und Ausstattung sie tragen (SF6 a), und gilt für
die Schichten nach SF7 a; die Spur setzt den Schalter, **das Modell rechnet den Übergang**
(das Hochlaufen des Leslie, das Umstecken eines Dämpfers). **Je Note** (Pizzicato, Fall,
Doit, Zungenstoß, Tremolo, Spiccato, Col legno, Flatterzunge, Growl, Wirbel, Abdämpfen):
eine Artikulation an der Note, wie Palm mute, die mit der Note wandert. Wechselt eine
Spielweise passagenweise, wird aber je Note gespielt (Pizzicato und Arco), liegt sie an
der Note; eine Passage setzt die Mehrfachauswahl — keine zweite Ablage. Damit gilt als
Regel, was im Saitenstrang gebaut ist (Strum als Spur; Palm mute, Tapping, Harmonic an der
Note; `bundlos` am Saitensatz). Verworfen: b (alles an der Note — kein Ort für den
Übergang zwischen den Noten, also auch nicht für die Zeit zum Umstecken), c (alles als
Spur — Fall und Doit blieben stehen, wenn die Note verschoben oder kopiert wird). Kippt,
wenn die Recherche einer Familie eine Spielweise findet, die Passage und Note braucht und
sich nicht über die Mehrfachauswahl setzen lässt — dann für genau diese ein eigener
Entscheid „Spur als Vorgabe, die Note überschreibt“.

✅ **SF9 a** (Patrik S140): **Zuerst die Spuren, dann die Familien in der Reihenfolge
ihrer Verwandtschaft zur Saite.** **Stufe 1:** die fünf Steuer-Spuren je Muster (SF5 d),
Expression am Clip und die Spuren der Klang-Spielweisen (SF8 a) — gebaut und gehört an
Synth-Schichten und an der heutigen Saite (Vibrato, Expression über die Tonabnehmer, Pitch
über *Tremolo arm*). Danach die Familien: (1) **Hammer-Saite** — Saite und Korpus da, neu
Hammer, Dämpfer, Mitschwingen beim Sustain; (2) **Bogen-Saite** — Saitensatz und Korpus da,
neu die Anregung durch den Bogen; (3) **Platte und Stab** — ein neues Modell aus
Schwingungsformen; (4) **Lippe und Rohr** und (5) **Rohrblatt und Rohr** — sie teilen ein
neues Rohr-Modell. Je Familie Recherche, Modell, Messung, Hörproben wie im Saitenstrang
(S132–S138). Mitgeliefert heute: Hammer-Saite `piano`, `house_piano`; Bogen-Saite
`violin`, `strings`, `pizzicato_violin`, `upright_bass`; Platte und Stab `marimba`,
`vibraphone`, `bell`, `kalimba`, `thumb_piano`, `steel_drum`; Lippe und Rohr `brass`;
Rohrblatt und Rohr `sax`, `alto_sax`. Verworfen: b (nach den Spuren zuerst das Rohr — prüft
die Regeln am fernsten Modell früh, aber die Regeln SF6 a bis SF8 a tragen eigene
Kipp-Bedingungen; bricht eine am Rohr, ändert sich die Regel, nicht das Gebaute), c (die
Spuren verteilt mit den Familien — legte das Modell der Spur an der Hammer-Saite fest, bevor
Pitch und Expression es geprüft haben). Kippt, wenn Patrik eine Familie im Klang vorziehen
will (wie S139 die Lautstärke) — dann rückt sie hinter Stufe 1 vor, die übrige Reihenfolge
bleibt. *Mit SF10 a kommen die Flöten mit dem Rohr-Schritt, Rhodes und Kalimba mit
Platte und Stab, Pfeifenorgel und Tonrad nach den Rohren.*

✅ **SF10 a** (Patrik S140): **Die Bauart ist ein Paar aus Anregung und Schwinger.**
**Anregungen:** Zupfen, Hammer, Schlägel, Bogen, Lippe, Rohrblatt, Luftstrahl.
**Schwinger:** Saite, Platte/Stab, Rohr, dazu das **Tonrad** als elektromechanischer
Schwinger ohne Anregung. Je Anregung und je Schwinger ein Modell: das Rohr dient drei
Anregungen (Lippe, Rohrblatt, Luftstrahl), die Saite vier (Zupfen, Hammer, Bogen, die
Tangente des Clavinets als Hammer). Die Tabelle aus SF6 a gilt je Paar; die Steuer-Spielweisen
folgen aus den beiden Achsen: **Expression, Vibrato, Pitch nur bei dauernder Anregung**
(Bogen, Lippe, Rohrblatt, Luftstrahl) — bei Zupfen, Hammer, Schlägel Expression nur über
einen Tonabnehmer, Pitch nur über einen *Tremolo arm*, Vibrato an der gezupften Saite über
den Finger (SF6 a); **Sustain über den Damper des Schwingers**, Sostenuto als Ausstattung.
Eine neue Kombination hat damit von selbst ihre Spuren.

| Instrument | Anregung | Schwinger | Ausstattung |
|---|---|---|---|
| Gitarre, Bass, Ukulele (heute) | Zupfen | Saite | Tonabnehmer, *Tremolo arm* |
| Klavier (`piano`, `house_piano`) | Hammer | Saite | Damper, Sostenuto (Flügel) |
| Clavinet | Hammer | Saite | Damper, Tonabnehmer |
| Geige, Streicher, Kontrabass | Bogen | Saite | |
| Vibraphon | Schlägel | Stab | Damper |
| Marimba, Glocke, Steel drum | Schlägel | Stab/Platte | |
| Rhodes (`tine_piano`) | Hammer | Stab | Damper, Tonabnehmer |
| Kalimba, Thumb piano | Zupfen | Stab | |
| Blech (`brass`) | Lippe | Rohr | |
| Saxophon | Rohrblatt | Rohr | |
| Flöte, Altflöte, Shakuhachi | Luftstrahl | Rohr | |
| Pfeifenorgel | Luftstrahl oder Rohrblatt je Register | Rohr | Register |
| Hammond | — | Tonrad | Leslie |

Welche Orgel `organ` und `house_organ` sind, klärt die Recherche ihrer Familie.
*Herkunft:* die Einordnung und die Regel aus dem Gedächtnis (Claude S140); dass die
Anregung als nichtlineare Stelle an den Schwinger gekoppelt gerechnet wird (Lippe,
Rohrblatt, Bogen am Wellenleiter), ist im Rohr-Schritt zu belegen. Verworfen: b (die Liste
um ganze Bauarten erweitern — „Schneide und Rohr“, „Stab mit Hammer“, „Tonrad-Orgel“ —
jede Kombination ein eigenes Ganzes, zwei Rohre und zwei Hämmer, und gezupfte und
angeschlagene Stäbe blieben eine Bauart), c (ohne Bauart, bis die Familie dran ist — schiebt
die Frage auf, während Spuren an Instrumenten ohne Einordnung gebaut würden). Kippt, wenn
die Recherche am Rohr zeigt, dass Anregung und Schwinger dort nur gemeinsam rechenbar sind
— dann für das Rohr b, für die übrigen bleibt a.

### 1a. Skizze Stufe 1 (Patrik S140, gebilligt)

Zwei Fassungen gezeichnet; gebilligt ist der Instrument-Editor aus Fassung 2 und der Clip
aus Fassung 1 (Patrik: die Kurve über dem Clip „war schon gut so").

**Instrument-Editor.** Die Spuren liegen **oben, unter Strum** — nicht unter den Noten
(Frage Patrik: „wieso unterhalb der Keys und nicht wie Strum?"). Was über der Zeit gespielt
wird, steht damit beisammen über dem Raster; was zu einer Note gehört (Tab-Linien, Noten,
*Note vol.*), darunter. Von oben: Taktzeile, Strum (bei Saiten), **Sustain, Sostenuto,
Expression, Vibrato, Pitch**, danach die Klang-Spuren (SF8 a, etwa *Una corda*), dann die
Tab-Linien, die Noten, *Note vol.* Damit ist der Ort aus SF5 d („unter den Noten") ersetzt.
- **Zeile *Lanes*** unter der Strum-Zeile: je Spur ein Haken, die vollen Namen; dazu der Haken
  *Tab* (heute am Ende der Strum-Zeile). Es erscheinen nur die Spuren, die das Instrument
  trägt (SF6 a, SF7 a). Ausblenden: SK1 a.
- **Pedale** (Sustain, Sostenuto) als Stufen 0 … 1, als Fläche von unten gezeichnet; das
  halbe Pedal ist eine halbe Fläche.
- **Kurven** (Expression, Vibrato, Pitch) mit Punkten zum Ziehen, wie der Bend; Pitch mit
  einer Mittellinie (die gespielte Höhe).
- **Eine leere Spur** zeigt eine gestrichelte Linie und „Instrument default" (SF4 a, SF5 d).
- **Klang-Spuren** in derselben Leiste, anders gefärbt, als Schalter an/aus mit
  Umschaltstellen (SF8 a); in Stufe 1 nur der Mechanismus.
- Der Name jeder Spur steht in der linken Spalte (heute 44 px, wie *Strum* und *Note vol.*)
  als Kürzel: SK2 a.

**Loop-Panel.** Die **Expression eines Clips liegt als Kurve auf dem Clip**, über seine ganze
Länge mit allen Wiederholungen, mit Punkten zum Ziehen; ohne Kurve zeichnet der Clip
nichts (Wert 1). Der Kanal behält seine Höhe. Die Punkte stehen in Takten ab dem Anfang
des Clips (SK8 a, §3). (Patriks Gegenvorschlag — der Kanal
anderthalbfach hoch, die Kurve im unteren Drittel — war aus einem Missverständnis der
Skizze entstanden und ist zurückgenommen.)

✅ **SK1 a** (Patrik S140): **Ausblenden ist nur Ansicht.** Der Haken in der Zeile *Lanes*
blendet eine Spur ein oder aus; ihre Werte bleiben in der `.lvloop` und klingen weiter.
Trägt eine ausgeblendete Spur Werte, steht ein Punkt hinter ihrem Namen (*Vibrato •*,
Tooltip „hidden, still playing"). Beim Öffnen eines Musters sind die Spuren mit Werten
eingeblendet. Gelöscht wird nur über das Kontextmenü der Spur, *Clear lane*. Verworfen:
b (Ausblenden schaltet stumm — dasselbe Muster klänge je nach Ansicht anders), c
(Ausblenden löscht nach Rückfrage — eine Geste der Ansicht, die Daten löscht; das leistet
*Clear lane*). Kippt, wenn der Vergleich mit und ohne Spur oft gebraucht wird — dann ein
Stummschalter im Kopf jeder Spur dazu, wie *Effects on* am Clip; der Haken bleibt Ansicht.

✅ **SK2 a** (Patrik S140): **Kürzel in der Spalte, volle Namen in der Zeile *Lanes* und im
Tooltip.** In der linken Spalte *Sus*, *Sost*, *Expr*, *Vib*, *Pitch*; eine Klang-Spur
bekommt ihr Kürzel aus ihrer Bauart (etwa *Corda*). Der volle Name steht am Haken der
Zeile *Lanes* — dort in derselben Reihenfolge — und im Tooltip über dem Kürzel. Die Spalte
behält ihre Breite (44 px). Verworfen: b (die Spalte breiter, etwa 72 px — nimmt jedem
Muster Breite, auch ohne Spuren), c (der Name blass in der Spur — Text über den Punkten am
Taktanfang). Kippt, wenn eine Klang-Spur einen Namen hat, der sich nicht klar auf etwa
sechs Zeichen kürzen lässt — dann eine breitere Spalte nur bei sichtbaren Spuren prüfen.

✅ **SK3 a** (Patrik S140): **Pitch in der Datei in Halbtönen, die Grenze vom Instrument.**
Die Spur Pitch speichert Halbtöne mit Bruchteil (Cent-genau), unabhängig von einem
Bereich — wie der Bend je Note (`kBendHalbtoeneMax`, S-L15 b). Die Grenze geben Bauart
und Ausstattung vor (SF10 a; etwa *Tremolo arm* mit eigener Grenze unten und oben); eine
Synth-Schicht hat eine einstellbare *Pitch range* bis ±24 Halbtöne, Vorgabe ±2. Die Spur
zeigt den Bereich ihres Instruments; ein Wert jenseits der Grenze wird beim Rechnen
gekappt und in der Spur als gekappt gezeichnet, aber nicht verändert. Die Bereiche der
Vorbilder (Synth ±2, Vibratohebel, Fall am Blech, Posaunenzug) aus dem Gedächtnis — in der
Recherche der Familien zu belegen. Verworfen: b (fest ±2, normiert wie MIDI — der Wert
wechselte mit dem Bereich seine Bedeutung, ein Fall über eine Oktave wäre unmöglich; ein
späterer MIDI-Eingang rechnet mit dem Bereich des Instruments um), c (fest ±12 — ein
Vibratohebel dürfte eine Oktave hinauf). Kippt, wenn das Kappen beim Wechsel des
Instruments verwirrt — dann ein Hinweis beim Wechsel, wie viele Punkte jenseits der neuen
Grenze liegen; das Datenformat bleibt.

✅ **SK4 a** (Patrik S140): **Expression 0 … 1 als Stärke; 1 heißt „wie die Note gespielt
ist“.** Leer = 1, ohne Kurve am Clip ebenso. Malgenommen werden *Note vol.* der Note, die
Spur des Musters und die Kurve am Clip. Was das Ergebnis bewirkt, entscheidet die Schicht:
eine Synth-Schicht macht daraus einen Pegel (über eine feste Kurve, in Stufe 1 gemessen und
festgelegt), ein Modell mit dauernder Anregung die Stärke der Anregung (Blasdruck,
Bogendruck) samt Klangfarbe; bei Zupfen, Hammer, Schlägel wirkt sie nur über den
Tonabnehmer, als Pegel nach dem Abnehmer (SF10 a). Der Tooltip zeigt am Synth dazu den
Pegel in dB. Dass der MIDI-Regler Expression von sich aus ganz offen steht, aus dem
Gedächtnis — zu belegen. Verworfen: b (dB als Pegel, am Clip hinzugezählt — jedes Modell
drehte nur den Pegel; der Fader je Kanal, LL3 a, arbeitet schon in dB), c (0 … 1 als Faktor
auf das fertige Signal — ein Crescendo am Blech ohne die Helligkeit des lauteren Spielens).
Kippt, wenn die Hörproben in Stufe 1 einen Swell über 1 hinaus verlangen, ohne die Noten
leiser zu setzen — dann ein Bereich bis zu einer Obergrenze über 1; das Datenformat bleibt.

✅ **SK5 a** (Patrik S140): **Die Spur gilt im Clip; an seinem Ende wird das Pedal
losgelassen.** Die Spur wird wie das Muster gelesen, Durchlauf für Durchlauf. An der Naht
zwischen zwei Wiederholungen gilt, was die Spur dort sagt: am Ende gedrückt und am Anfang
gedrückt — die Noten klingen über die Naht weiter; am Anfang oben — dort wird gedämpft.
Am letzten Ende eines Clips gilt das Pedal als losgelassen: gehaltene Noten gehen in ihren
Ausklang wie beim Heben des Fußes, abgeschnitten wird nichts. Ein Schlussakkord, der frei
ausklingen soll, bekommt einen Takt mehr mit gedrücktem Pedal. `plane` rechnet die Dauer
jeder Note aus Note und Pedal, je Clip für sich — der Klang eines Clips hängt nicht am
vorigen (L1 a: `rechne` hat kein Gedächtnis, `include/loop/Rechenkern.hpp`). Verworfen:
b (das Pedal als Zustand des Kanals bis zum nächsten Clip — ein Clip klänge je nach
Vorgänger anders, ein Start in der Mitte müsste rückwärts suchen), c (am Clip-Ende bleibt
das Pedal gedrückt — das Ende klänge anders als jede Naht davor, obwohl die Spur dasselbe
sagt). Kippt, wenn der freie Ausklang am Ende oft gebraucht wird — dann ein Haken je Clip
*Let ring at end*, der für diesen Clip c gilt; a bleibt die Vorgabe.

✅ **SK6 a** (Patrik S140): **Das halbe Pedal: am Synth eine Schwelle, im Modell der
Dämpfer.** Eine Synth-Schicht liest Sustain und Sostenuto als Schalter — ab 0,5 hält sie,
darunter nicht. Ein Modell mit Damper (Hammer-Saite, Stab mit Dämpferleiste; SF6 a, SF10 a)
liest den Wert als Andruck des Dämpfers: 0 voll gedämpft, 1 frei, dazwischen klingt die
Note schneller oder langsamer aus; wie der Andruck die Dämpfung setzt, misst die Recherche
der Hammer-Saite. Eine Synth-Schicht mit *Follow* hält, solange die Saite des Modells
klingt (SF7 a), und folgt so auch dem halben Pedal. Dass der MIDI-Regler 64 bei 0–63 aus
und bei 64–127 an ist und manche Klangerzeuger ihn stufenlos lesen, aus dem Gedächtnis — zu
belegen. Verworfen: b (am Synth stufenlos über den Release — eine Wirkung ohne Vorbild; ein
weiches Nachlassen leisten Expression und die Hüllkurve), c (nur 0 und 1 — nähme dem
Klavier eine echte Spielweise, die SF5 d wollte). Kippt, wenn die Hörproben in Stufe 1 ein
stufenloses Sustain an Synth-Schichten verlangen — dann b als Schalter an der Synth-Schicht,
die Schwelle als Vorgabe.

## 2. Offen

- Je Klang-Spielweise der Ort nach SF8 a — in der Recherche ihrer Familie.
- Die Zeit zum Umstecken eines Dämpfers: belegen, dann entscheiden.

## 3. Stufe 28 — die Spuren (Stufe 1 aus SF9 a)

✅ **SK7 a** (Patrik S140): **Erst Daten und Klang, dann die Oberfläche, wie in Stufe 27.**

| Schritt | Inhalt |
|---|---|
| 28a ✅ | Modell und Datei: Spuren als Stufen, Kurve, Schalter; die fünf Steuer-Spuren am Muster, die Expression-Kurve am `LoopClip`; `.lvloop` Fassung 12, alle Felder immer geschrieben. |
| 28b ✅ | Planung: Sustain und Sostenuto setzen die Dauer der Noten (SK5 a, SK6 a), am Synth mit der Schwelle — an gerechneten Tönen gemessen, ohne Engine. |
| 28c-1 ✅ | Klang an Synth-Schichten: Expression mit der Pegelkurve, gemessen und festgelegt (SK4 a); Vibrato-Tiefe (SF4 a); Pitch mit *Pitch range* (SK3 a, `.lvinst` Fassung 14); Töne je Vorkommen; Rechenzeit gemessen. |
| 28c-2 | Klang an der Saite: Vibrato über die Spannung, Expression über die Tonabnehmer, Pitch über *Tremolo arm* (neues Feld am Saitensatz); Hörproben. *28c-2a ✅, 28c-2b ✅ (SK12 a), Hörproben offen.* |
| 28d | Instrument-Editor: Zeile *Lanes*, Spuren zeichnen und bearbeiten (§1a, SK1 a, SK2 a), ein Fall durch den Editor. |
| 28e | Loop-Panel: die Expression-Kurve auf dem Clip. |
| 28f | Abschluss: Beispiel-Loops, Handbuch, Sichttest-Liste, voller Lauf (wenn Patrik ihn freigibt). |

Der Mechanismus der Klang-Spuren kommt mit 28a und 28d; er klingt erst mit der
Hammer-Saite. **Rechenpunkt:** heute ist ein Ton vorab nach Tonhöhe und Dauer gerechnet;
mit Kurven über der Zeit wird jede Note eigen — Vorbild sind die Töne je Vorkommen der
Saiten (`inst:…:s<n>:e<ms>`), die Rechenzeit wird in 28c-1 gemessen. Verworfen: b (erst die
Oberfläche — eine Oberfläche ohne Wirkung, ein Fehler im Rechenweg fiele spät auf), c (Spur
für Spur durch alle Bereiche — legte das gemeinsame Modell an Sustain fest und änderte es
bei Pitch). Kippt, wenn Patrik die Spuren früh im Programm sehen will — dann rückt 28d
hinter 28a vor, der übrige Plan bleibt.

✅ **SK8 a** (Patrik S140): **Die Punkte der Expression am Clip stehen in Takten ab dem
Anfang des Clips** (mit Bruchteil). Wird der Clip länger, bleibt die Kurve, wo sie ist;
hinter dem letzten Punkt gilt sein Wert. Wird er kürzer, bleiben die Punkte dahinter
erhalten, sind nicht zu hören und werden blass gezeichnet; kommt der Clip auf die alte
Länge, ist die Kurve wieder ganz. Wer den Clip verschiebt, nimmt die Kurve mit.
Verworfen: b (im Anteil der Clip-Länge, wie der Bend an der Note — das Ziel eines
Crescendos rutschte bei jeder Änderung der Länge; ein Bend ist eine Gestalt der Note, die
Kurve am Clip ein Verlauf über einem Stück). Kippt, wenn beim Verlängern meist ein
mitwachsender Bogen gewollt ist — dann ein Eintrag *Stretch expression to clip*, der die
Punkte einmalig umrechnet; das Datenformat bleibt a.

**Modell 28a** (von Claude gesetzt, nach SF5 d, SK1 a bis SK8 a): `loop/Zeitspur.hpp`.
Ein Punkt hat eine Stelle und einen Wert; die Stelle im Muster in **Schritten mit
Bruchteil** (ein Pedal lässt sich kurz nach dem Anschlag treten), am Clip in Takten (SK8 a).
Drei Arten: **Stufen** (Sustain, Sostenuto, 0 … 1; der Wert gilt bis zum nächsten Punkt,
vor dem ersten ist das Pedal oben), **Kurve** (Expression 0 … 1, Vibrato in Cent 0 … 100
wie `vibratoDepth` der Schicht, Pitch in Halbtönen bis ±24 nach SK3 a; linear zwischen den
Punkten, vor dem ersten der erste, nach dem letzten der letzte, wie `bendBei`),
**Schalter** (die Klang-Spuren, beim Namen; ein Name, den das Programm nicht kennt,
bleibt beim Lesen stehen). Eine leere Spur ist die Vorgabe. Je Stelle ein Punkt (der
spätere gilt, wie beim Bend); höchstens 4096 Punkte je Spur (eine Grenze für die Datei).
Im Muster gehen Punkte hinter dem Ende mit, wie Noten (`richte`); *Repeat when
lengthening* wiederholt die Spuren mit den Noten; ein neues Raster (`setzeRaster`) hält
jeden Punkt an derselben Stelle im Schlag, Punkte in einem Schlag, den es nicht mehr gibt,
gehen. Am Clip: wird die linke Kante gezogen, bleibt die Kurve an ihrer Stelle im
Arrangement (die Punkte rücken um den Versatz); Punkte vor dem neuen Anfang bleiben wie die
hinter dem Ende (Auslegung von SK8 a, Claude S140). Die Kurve am Clip gilt für jeden Clip,
auch für ein Drumkit. Ein Drumkit-Muster trägt keine Spuren. In der `.lvloop` (**Fassung 12**, alle Felder
immer geschrieben): am Instrument-Muster `lanes` mit `sustain`, `sostenuto`,
`expression`, `vibrato`, `pitch` (je `[[stelle, wert], …]`) und `switches` (je Name eine
solche Liste); am Clip `expression`.

**Planung 28b** (`loop/Pedalplan.hpp`; nach SK5 a, SK6 a, SF6 a, SF7 a): die Pedale
verlängern eine Note, ihr Ton wird länger gehalten und klingt danach aus — je Vorkommen
im Clip gerechnet. **Sustain** hält eine Note, deren Ende bei gedrücktem Pedal liegt, bis
zum Loslassen. **Sostenuto** hält nur die Noten, die beim Treten angeschlagen und noch nicht
zu Ende sind — eine im selben Moment angeschlagene zählt mit (von Claude gesetzt). An einer
Synth-Schicht ab 0,5 gedrückt (SK6 a). **Schlägt dieselbe Tonhöhe im Clip neu an, endet der
gehaltene Ton dort** (von Claude gesetzt: eine Taste hat eine Saite; im Rechenkern schneidet
derselbe Ton sich ohnehin selbst ab). Ein Muster ohne Punkte auf den Pedalen behält seine
Töne wie bis Stufe 27 (`loop_werte.txt` unverändert). **Mit Saiten** hält die Saite nicht
(SF6 a): ein gehaltener Ton zerfällt in die Saite mit ihren folgenden Schichten wie ohne
Pedal (`:p1`) und die Schichten mit *Own note* als eigener, gehaltener Ton (`:p2`, SF7 a);
in einer Kette hält der Ton der Schichten mit *Own note*.

✅ **SK9 a** (Patrik S140): **Die Pegelkurve von Expression an einer Synth-Schicht ist
quadratisch, wie die MIDI-Empfehlung.** Eine Synth-Schicht nimmt ihren Ton mal e², mit e als
Produkt aus Spur des Musters und Kurve am Clip — in dB 40 · lg e: 0,5 = −12 dB, 0,25 =
−24 dB, 0 = still. Der Tooltip zeigt den Pegel in dB. *Beleg* (jazzdup.nfshost.com,
„Volume", eine Abschrift einer verbreiteten, inoffiziellen Beschreibung der MIDI-Spezifikation,
abgerufen 2026-10-20, Wortlaut): „It is recommended that a device use the volume value in a
logarithmic manner, as specified by the following formula if only the coarse value is used:
40 log (Volume/127)“ und „Note: In the above formula, "Volume" may be equal to Channel Volume
* Expression.“ Nicht in einem Dokument der MIDI Association geprüft; eine Suche nach GM2
ohne Treffer zum Wortlaut. Verworfen: b (linear, 20 · lg e — der hörbare Bereich läge in
der unteren Hälfte), c (−60 … 0 dB gleichmäßig — ein Sprung zur Stille bei 0, ohne Vorbild).
Kippt, wenn die Hörproben zeigen, dass ein gezogenes Crescendo am Anfang zu lange leise
bleibt.

✅ **SK10 a** (Patrik S140): **Ohne eigene Rate gilt für die Spur Vibrato eine Vorgabe.** Hat
eine Schicht keine Rate (0), nimmt die Spur Vibrato 5,5 Hz und Einsatz 0; hat sie eine, gelten
Rate und Einsatz der Schicht (SF4 a). Ohne Spur ändert sich nichts. Im Creator steht bei
Rate 0 der Hinweis „5.5 Hz with the Vibrato lane". *Befund* (gezählt in
`asset/loop/instruments` ohne `_old`, S140): 20 von 99 Instrumenten tragen eine Schicht mit
Rate, 79 nicht; der häufigste Wert ist 5,5 Hz (fünf Schichten), die Spanne 0,4 bis 20 Hz.
5,5 Hz von Claude gesetzt (der häufigste Wert); dass gesungenes und gestrichenes Vibrato um
5 bis 7 Hz liegt, aus dem Gedächtnis — in der Recherche der Bogen-Saite zu belegen.
Verworfen: b (ohne Rate wirkt die Spur nicht — nähme 79 von 99 Instrumenten die Spur, die
im Editor erscheint), c (die Spur führt auch die Rate — eine zweite Angabe neben der
Schicht, kippte SF4 a ohne Anlass aus Hörproben). Kippt, wenn die Hörproben zeigen, dass
Instrumente sehr verschiedene Raten brauchen — dann c prüfen.

**Klang 28c-1a ✅** (Expression; `Ausdruck` im Rechenkern): Expression ist an einer
Synth-Schicht ein reiner Pegel und wird deshalb nicht in den Ton gerechnet, sondern im
Rechenkern je Schlag über ihn gelegt — die Töne bleiben, wie sie sind. Je Clip ein Verlauf:
die Spur des Musters über alle Wiederholungen (vor dem ersten Punkt eines Durchlaufs der
erste Wert, an der Naht ein Sprung) mal die Kurve am Clip; der Pegel e² (SK9 a); ein Ton,
der über das Ende des Clips nachklingt, behält den Wert dort. Ein Drumkit-Clip nimmt nur
die Kurve am Clip. Ein Instrument mit Saiten bekommt Expression erst mit 28c-2 (über die
Tonabnehmer, SF6 a). Ohne Spur und Kurve bleibt jeder Abtastwert, wie er war. *Folge des
Modells 28a:* ein Punkt genau am Ende des Musters fällt beim Richten weg wie eine Note dort —
ein Verlauf, der bis zum Ende steigen soll, endet mit einem Punkt kurz davor.

**Klang 28c-1b ✅** (Vibrato und Pitch; `tonImClip` in `loop/Pedalplan`, `Handweg` im
Instrument): beide ändern die Tonhöhe und gehören deshalb in den Ton. Ein Muster mit Punkten
auf Pitch oder Vibrato (oder den Pedalen) rechnet seine Töne **je Vorkommen**; der Ton
trägt die Spuren in ms ab dem Anschlag (`:q<ms>=<cent>,…`, `:w<ms>=<cent>,…` im Namen),
über die Naht in die nächste Wiederholung, hinter dem Clip mit dem Wert dort. Gelesen wird
bis zum Ende der gehaltenen Note; **im Ausklang gilt der letzte Wert** (von Claude gesetzt).
Gerechnet über den Weg der Tonhöhe, den jede Schwingung schon kennt — damit bekommen auch
Sample, Rauschen und die Saite *Classic* ein Vibrato. **Pitch** wird je Schicht auf
±*Pitch range* gekappt (`pitchRange`, Vorgabe 2, bis 24 Halbtöne; `.lvinst` Fassung 14;
im Creator in der Gruppe Vibrato). **Vibrato**: wo die Spur Punkte trägt, schweigt das
Vibrato der Schicht, die Spur setzt die Tiefe (SF4 a); Rate und Einsatz der Schicht, ohne
Rate 5,5 Hz ohne Einsatz (SK10 a; im Creator steht bei Rate 0 „off · 5.5 Hz with the
Vibrato lane"). Eine Spur Pitch, die überall 0 ist, ändert den Namen nicht. Ein Instrument
mit Saiten bekommt Vibrato und Pitch erst mit 28c-2. *Rechenzeit* (Release-clang, S140):
ein Muster von 16 Noten auf vier Tonhöhen — ohne Spuren 4 Töne in 2,0 ms, mit Pitch und
Vibrato 16 Töne in 14,6 ms (Testing-MSVC 3,4 und 36,1 ms).

✅ **SK11 a** (Patrik S140): **Der Vibratohebel sind zwei Zahlen am Saitensatz.** *Tremolo arm*
steht als zwei Werte in Halbtönen: wie weit nach unten (`tremoloDown`) und nach oben
(`tremoloUp`); 0 und 0 = kein Hebel — dann wirkt die Spur Pitch an der Saite nicht. Wer den
Hebel im Creator einschaltet, bekommt 2 nach unten und 1 nach oben (von Claude gesetzt, in den
Hörproben zu prüfen). Die Spur wird an der Saite auf diese Grenzen gekappt. In der `.lvinst`
beide immer (Fassung 15). *Beleg:* eine Suche nach Zahlen (S140) fand keine; was sie zur
Bauweise fand (Vintage senkt nur, Floyd Rose senkt und hebt), stammt aus der Zusammenfassung
des Suchwerkzeugs, nicht aus den Seiten — kein Beleg. Verworfen: b (ein Schalter mit fester
Spanne — ein Steg, der nur senkt, liesse sich nicht ausdrücken), c (die Bauart des Stegs mit
festen Spannen je Bauart — Spannen ohne Beleg im Format; der Creator kann die Bauarten als
Vorlagen anbieten, die die zwei Zahlen setzen). Kippt, wenn ein Beleg zeigt, dass jede Bauart
eine feste Spanne hat.

**Klang 28c-2a ✅** (Vibrato und Pitch an der Saite; `Bahn` und `Handweg` im Instrument,
`setzeSpuren` in `loop/Pedalplan`, aufgerufen von `toeneImKanal`): die Töne eines
Saiteninstruments tragen die Spuren wie die eines Synths, je Teil (Saite mit ihren Folgern,
*Own note*, Kette) ab ihrer Note und so lange sie gehalten werden. **An der Saite** wirken
beide über die Spannung wie der Bend: Vibrato immer (der Finger; Rate der ersten
Saiten-Schicht, ohne Rate 5,5 Hz nach SK10 a), Pitch nur mit Vibratohebel und auf seine
Grenzen gekappt (SK11 a); ohne Hebel überhört die Saite Pitch, Bit für Bit. Senkt der Hebel
den Ton, wächst der Ring der Saite mit dem tiefsten Stand. **Eine Schicht mit Follow
string** schwingt mit dem Vibrato der Saite und folgt Pitch mit Hebel in seinen Grenzen;
ohne Hebel rechnet sie Pitch selbst, mit ihrer *Pitch range* — sie kann sich so gegen die
Saite verstimmen (wörtlich nach SF7 a, Patrik S140 angezeigt). **Eine Schicht mit Own note**
nimmt beide Spuren immer mit ihren eigenen Werten. Der Creator: in der Zeile *Tuning* der
Haken *Tremolo arm* mit ↓ und ↑ in Halbtönen. `.lvinst` Fassung 15. *Gemessen* (beide
Compiler, `test_LoopSaiteSpuren`): Hebel +1 = 100 ct, −3 mit Grenze 2 = −200 ct, ein Fall um
zwei Oktaven trifft E2 auf 0 ct; Vibrato von 50 ct an der Saite −49 bis +49,5 ct.

✅ **SK12 a** (Patrik S141, im Gespräch X1): **Expression an der Saite sitzt im Ton, direkt
hinter dem Tonabnehmer, vor der Reihe der Schicht** — wie der Lautstärkeknopf der Gitarre:
ein leiserer Ton zerrt weniger. Jede andere Schicht nimmt den Pegel am Ende der Schicht.
Spur und Kurve stehen im Namen des Tons; ein Clip mit Kurve hat eigene Töne. *Befund:* an
den mitgelieferten Instrumenten tragen Saiten-Schichten mit Tonabnehmer einen Drive
(`overdrive_guitar` 0,75, `clavinet` 0,35, `slap_bass` 0,2) — der Ort ist hörbar. Folge:
mit Drive ist e = 0,5 nicht genau −12 dB, der Tooltip in dB stimmt dort nur ohne Drive.
Verworfen: b (im Ton am Ende jeder Schicht, nach dem Drive — wie ein Volume-Pedal hinter dem
Verstärker; ein gültiges Vorbild, aber nicht der Abnehmer aus SK4 a), c (im Rechenkern wie
am Synth, der Ton in einen Teil mit und einen ohne Expression geteilt — ein Ort für die
Pegelkurve und weniger Töne, aber der Pegel säße hinter dem Drive, und eine zweite Teilung
neben `:p1`/`:p2`). Kippt, wenn die Hörproben zeigen, dass ein Swell an der verzerrten
Gitarre seine Zerrung behalten soll (dann b), oder wenn die Rechenzeit der Töne je Clip
spürbar wird (dann c).

**Klang 28c-2b ✅** (Expression an einem Instrument mit Saiten; `setzeAusdruck` in
`loop/Pedalplan`, aufgerufen von `toeneImKanal` nach dem Ende je Ton; `Ausdrucksweg` im
Instrument; der stückweise Leser in `loop/Stueckweise.hpp`, mit dem Rechenkern geteilt): jeder
Ton trägt die Spur des Musters (`:x<ms>=<tausendstel>,…`) und die Kurve am Clip
(`:y…`) getrennt, ab seinem Anschlag (mit Swing und Strum) bis zum Ende des Clips — auch
im Ausklang — oder bis zu seinem eigenen Ende (`:e`); hinter dem Clip der Wert dort. Je
Abtastwert werden beide malgenommen, der Pegel ist e² (SK9 a). **An der Saite** wirkt er nur
mit Tonabnehmer, direkt dahinter, vor der Reihe (SK12 a); akustisch und *Extended KS*
rechnet die Saite Bit für Bit wie ohne. **Jede andere Schicht** nimmt ihn immer, am Ende der
Schicht (SF7 a). Der Rechenkern legt bei Saiten keinen zweiten Pegel darüber. Ein Verlauf,
der überall 1 ist, fällt weg; ohne Spur und Kurve heißen die Töne wie vorher (`Loop Werte`
unverändert). *Gemessen* (beide Compiler, `test_LoopSaiteExpression`): Tonabnehmer, e = 0,5:
−12,04 dB; mit Drive 0,75: −6,70 dB; Saite akustisch plus Sinus: der Sinus genau ein
Viertel (Abweichung 1,5 · 10⁻⁸); im Ausklang bei 1,0 s 0,250. *Rechenzeit:* ein Muster von
16 Noten auf vier Tonhöhen an einer Waveguide-Gitarre — ohne Expression 4 Töne in 5,1 ms,
mit Spur 16 Töne in 20,8 ms (Release-clang; Testing-MSVC 29 und 113 ms).

## Versionen

| Version | Datum | Änderung |
|---|---|---|
| 0.23.0 | 2026-10-21 | Session 141. SK12 a (Expression an der Saite im Ton, hinter dem Tonabnehmer, vor dem Drive); Stufe 28c-2b gebaut. |
| 0.22.0 | 2026-10-20 | Session 140. SK11 a (Vibratohebel als zwei Zahlen); Stufe 28c-2a gebaut (Vibrato und Pitch an der Saite, `.lvinst` 15). |
| 0.21.0 | 2026-10-20 | Session 140. Stufe 28c-1b gebaut (Vibrato und Pitch im Ton, Pitch range, `.lvinst` 14); 28c-1 fertig. |
| 0.20.0 | 2026-10-20 | Session 140. SK9 a (Pegel e², Beleg MIDI-Empfehlung), SK10 a (Vibrato-Rate 5,5 Hz, wo die Schicht keine hat); Stufe 28c-1a gebaut (Expression im Rechenkern). |
| 0.19.0 | 2026-10-20 | Session 140. Stufe 28b gebaut: Planung der Pedale (`loop/Pedalplan`). |
| 0.18.0 | 2026-10-20 | Session 140. SK8 a (Expression am Clip in Takten ab dem Clip-Anfang); Modell 28a. |
| 0.17.0 | 2026-10-20 | Session 140. §3 Stufe 28, SK7 a (28a Modell und Datei bis 28f Abschluss). |
| 0.16.0 | 2026-10-20 | Session 140. SK6 a (halbes Pedal: Schwelle am Synth, Andruck des Dämpfers im Modell). |
| 0.15.0 | 2026-10-20 | Session 140. SK5 a (die Pedal-Spur gilt im Clip, am Clip-Ende losgelassen, an der Naht der Wert der Spur). |
| 0.14.0 | 2026-10-20 | Session 140. SK4 a (Expression 0 … 1 als Stärke, malgenommen mit *Note vol.* und Clip, die Schicht übersetzt). |
| 0.13.0 | 2026-10-20 | Session 140. SK3 a (Pitch in Halbtönen, die Grenze vom Instrument, Synth *Pitch range* bis ±24, Vorgabe ±2). |
| 0.12.0 | 2026-10-20 | Session 140. SK2 a (Kürzel in der linken Spalte, volle Namen in *Lanes* und im Tooltip). |
| 0.11.0 | 2026-10-20 | Session 140. §1a Skizze Stufe 1 (Spuren oben unter Strum, Zeile *Lanes*, Expression als Kurve auf dem Clip), SK1 a (Ausblenden ist nur Ansicht); Ort aus SF5 d ersetzt. |
| 0.10.0 | 2026-10-20 | Session 140. SF10 a (Bauart = Anregung × Schwinger; Einordnung der mitgelieferten Instrumente; Steuer-Spielweisen aus den zwei Achsen). |
| 0.9.0 | 2026-10-20 | Session 140. SF9 a (Stufe 1 die Spuren, dann Hammer-Saite, Bogen-Saite, Platte und Stab, Lippe und Rohr, Rohrblatt und Rohr). |
| 0.8.0 | 2026-10-20 | Session 140. SF8 a (drei Orte der Klang-Spielweisen: Ausstattung, Spur je Muster, Artikulation an der Note). |
| 0.7.0 | 2026-10-20 | Session 140. SF7 a (Spuren aller Schichten zusammen; *Follow* folgt bei den gemeinsamen, die übrigen rechnet die Schicht selbst). |
| 0.6.0 | 2026-10-20 | Session 140. SF6 a (Bauart als Obergrenze, Ausstattung je Instrument; Tabelle der Steuer-Spielweisen je Bauart, noch unbelegt). |
| 0.5.0 | 2026-10-20 | Session 139. SF5 d (fünf Spuren je Muster, dazu Expression am Clip, malgenommen). |
| 0.4.0 | 2026-10-20 | Session 139. SF4 a (Vibrato der Schicht als Vorgabe, die Spur setzt die Tiefe). |
| 0.3.0 | 2026-10-20 | Session 139. SF3 a (fünf Steuer-Spielweisen, Beleg der MIDI-Belegung). |
| 0.2.0 | 2026-10-20 | Session 139. SF2 a (die Bauart bestimmt die Steuer-Spielweisen eines Modells); Antwort zu den Saiten. |
| 0.1.0 | 2026-10-20 | Session 139. Angelegt: Zielbild (alle Familien, Klang nach echten Instrumenten), drei Arten von Spielweisen als Vorschlag, SF1 c. |

# LumiViz — Spielweisen der Instrumentfamilien (Konzept)

> **Version:** 0.78.0
> **Datum:** 2026-10-24 (Session 139: Zielbild, SF1 c, SF2 a, SF3 a, SF4 a, SF5 d;
> Session 140: SF6 a bis SF10 a, Skizze Stufe 1, SK1 a bis SK11 a; Session 141: SK12 a, SK13 a,
> Zielbild Hammer-Saite, H1 a bis H3 a, F1 a bis F6 a; Session 142: TV1 a bis TV7 a, WX1 a;
> Session 143: TS1 a, WX2 a bis WX9 a, HB1 a, FZ1 a bis FZ3 a; Session 144: FV1 a, FV2 a, FZ4 a bis FZ7 a; Session 145: FP1 a bis FP3 a, BL1 a, BL2 a, HS1 a, HS2 a, HA1 a bis HA4 a; Session 146: HK1 a, HK2 a, 31a gebaut, HT1 a bis HT7 a, 31b-1 bis 31b-3 gebaut, KB1 a; Session 147: BM1 a, PL31c a, HT7 b, BM2 a, GB1 a, BM3 a, BM3-1 a, BM3-2 a, BM5 a, BM5-1 a, 31c-1 gebaut, IC1 a, BM4 a, 31c-2 gebaut, EL1 a, 31c-3 gebaut; Session 148: EG1 a, EG2 a, 31c-4 gebaut, BM6 a, BM6-1 a, PL31c-5 a, 31c-5a gebaut, Befund Dämpfung, 31c-5b gebaut, ST1 a, 31c-5c gebaut, 31c-5d gebaut; Session 149: BM7 a, PL31 a, PS1 a, PF1 a, KV1 a, AW1 a, FT1 a, PG1 a, SB1 a, DM1 a, GL1 a, VA1 a, ZA1 a, EM1 a)
> **Typ:** Konzept — Ziel, Modell, Klang. Entschieden sind SF1 c bis SF10 a, SK1 a bis SK13 a, H1 a bis H3 a, F1 a bis F6 a, TV1 a bis TV7 a, WX1 a bis WX9 a, HB1 a, FZ1 a bis FZ3 a, FV1 a, FV2 a, FZ4 a bis FZ7 a, FP1 a bis FP3 a, BL1 a, BL2 a, HS1 a, HS2 a, HA1 a bis HA4 a, HK1 a, HK2 a, HT1 a bis HT6 a, HT7 b (löst das vorläufige HT7 a ab), KB1 a, BM1 a, BM2 a, BM3 a, BM3-1 a, BM3-2 a, BM5 a, BM4 a, BM5-1 a, PL31c a, GB1 a, IC1 a, EL1 a, EG1 a, EG2 a, BM6 a, BM6-1 a, PL31c-5 a, ST1 a, BM7 a, PL31 a, PS1 a, PF1 a, KV1 a, AW1 a, FT1 a, PG1 a, SB1 a, DM1 a, GL1 a, VA1 a, ZA1 a, EM1 a und TS1 a (Patrik S139–S149);
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

✅ **SK13 a** (Patrik S141, im Gespräch E1): **Die Kurve am Clip bearbeitet man am
markierten Clip.** Ein markierter Clip zeigt seine Kurve mit Punkten; ohne Kurve eine
gestrichelte Linie oben bei 1, beschriftet „Expression 1“. Ein Klick auf die Linie oder die
Kurve (bis 4 px daneben) setzt einen Punkt und zieht ihn weiter, auf den Takt mit Bruchteil
in Sechzehnteln, mit Alt frei. Punkte lassen sich ziehen, ein Doppelklick auf einen Punkt
löscht ihn; der Doppelklick auf den Clip öffnet weiter den Editor. Im Kontextmenü des
Clips *Clear expression*. Ein nicht markierter Clip zeichnet seine Kurve ohne Punkte; ein
Klick darauf markiert und verschiebt ihn wie bisher. Verworfen: b (ein Schalter
*Expression* im Kopf des Panels — gegen „kein Schalter Edit“, ein Modus wird vergessen), c
(Alt+Klick — nur auffindbar, wer die Taste kennt; der Fall aus S110). Kippt, wenn das Band um
die Linie beim Verschieben des markierten Clips im Sichttest stört (dann c dazu), oder wenn
die Kurve auch auf nicht markierten Clips greifbar sein soll (dann b).

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
| 28d ✅ | Instrument-Editor: Zeile *Lanes*, Spuren zeichnen und bearbeiten (§1a, SK1 a, SK2 a), ein Fall durch den Editor. |
| 28e ✅ | Loop-Panel: die Expression-Kurve auf dem Clip (SK13 a). |
| 28f | Abschluss: Beispiel-Loops, Handbuch, Sichttest-Liste, voller Lauf (wenn Patrik ihn freigibt). *Gebaut S141 bis auf den vollen Lauf: `lanes_demo`, Handbuch 1.197.0, Sichttest-Liste `Sichttest_Spuren.md` (lokal).* |

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

**Editor 28d ✅** (Instrument-Editor; `NotenRaster` in `InstrumentEditorSpuren.cpp`,
`traegtSpur` und `pitchBereichHt` im Instrument): die Skizze §1a, wie gebilligt. **Welche
Spuren** das Instrument trägt, rechnet es wie die Planung (SF6 a, SF7 a): ohne Saiten alle
fünf; mit Saiten die Pedale nur bei einer Schicht mit *Own note*, Expression an jeder Schicht
außer einer Saite ohne Tonabnehmer, Vibrato immer, Pitch an jeder Schicht außer der Saite,
an ihr nur mit Vibratohebel. Eine Spur mit Werten, die das Instrument nicht trägt, bleibt in
der Zeile *Lanes* (im Raster blass gezeichnet, Tooltip „This instrument does not use it — the values stay in the
pattern") — sonst lägen Daten unsichtbar im Muster. **Zeile *Lanes*** unter der Zeile
*Strum*: je Spur ein Haken mit dem vollen Namen, rechts *Tab* (von der Zeile *Strum*
hierher). **Im Raster** unter der Spur Strum je eingeblendeter Spur 36 px, links das Kürzel
(SK2 a), die Schläge leise, die Takte kräftiger. Pitch zeigt den Bereich des Instruments
(`pitchBereichHt`; ohne 2 Halbtöne); ein Wert dahinter wird am Rand gezeichnet, der Punkt
hohl und rot (SK3 a). *Von Claude gesetzt, wie der Bend und die Noten im selben Fenster:*
ein Klick in eine Spur setzt einen Punkt auf den nächsten Schritt und zieht ihn weiter;
**mit Alt frei** (Hundertstel eines Schritts); ein Punkt lässt sich ziehen, er bleibt
zwischen seinen Nachbarn; ein Doppelklick nimmt ihn weg; das Kontextmenü der Spur hat
*Delete point* und *Clear lane* (SK1 a). Werte auf Hundertstel, Vibrato auf ganze Cent; ein
Schalter ist an oder aus (obere oder untere Hälfte). Der Tooltip zeigt den Wert —
Expression mit dem Pegel in dB (SK9 a). Je Zug ein Schritt für Undo: *Add lane point*,
*Move lane point*, *Delete lane point*, *Clear lane*. Die **Schalter** (Klang-Spuren, SF8 a)
zeigt die Leiste, wenn das Muster welche trägt — anders gefärbt, als Fläche; anlegen lässt
sich einer erst, wenn eine Bauart ihn anbietet (ab der Hammer-Saite). *Geprüft*
(`test_LoopEditorSpuren`, beide Compiler): über den Editor wie im Programm, die Spuren kommen
im Loop an; Bild mit `LUMIVIZ_EDITOR_SPUREN_BILD`.

**Loop-Panel 28e ✅** (`Taktachse`; SK8 a, SK13 a): jeder Clip mit Kurve zeichnet sie über
seine ganze Länge mit allen Wiederholungen, unter dem Namen (der Name liegt dann auf einem
leichten Grund, damit die Kurve ihn nicht durchstreicht); ohne Kurve nichts. Der allein
markierte Clip zeigt die Punkte, ohne Kurve die gestrichelte Linie bei 1 mit „Expression 1“;
Punkte hinter dem Ende des Clips blass. Gesten wie SK13 a; die Kanten des Clips gehen vor.
Zeiger und Statuszeile sagen, was geht, über einem Punkt mit dem Pegel in dB
(`expressionDb` in `Zeitspur.hpp`, von Editor und Panel geteilt). Undo-Schritte *Add
expression point*, *Move expression point*, *Delete expression point*, *Clear expression*.
Esc während des Ziehens nimmt den Punkt zurück. *Clear expression* steht grau im Menü, wenn
der Clip keine Kurve hat. *Geprüft* (`test_LoopClipExpression`, beide Compiler): über das
Panel mit CommandBus und Undo; ein nicht markierter Clip verschiebt sich wie bisher; Bild mit
`LUMIVIZ_LOOP_EXPRESSION_BILD`.

**Abschluss 28f** (bis auf den vollen Lauf): das Beispiel **`lanes_demo`** aus
`asset/loop/make_spuren_loops.py` — „Slow Swell“, je Spur ein Kanal, der sie hörbar macht:
Piano mit Sustain (Pedalwechsel je Takt), Warm Pad mit Expression und Kurve am Clip, Lead mit
Vibrato und Pitch, Clean Guitar mit Volume-Swells über den Tonabnehmer, Round Bass mit einem
Fall, der auf *Pitch range* gekappt wird. Fassung 12 mit allen Feldern der Fassungen 11 und 12
(eine Fassung ist eine Zusage über die ganze Datei); gemischt gegen die Pegel seit S139,
`instrument_pegel.py --loops` lässt die Datei, wie sie ist. Dazu im Katalog das Genre *Ambient* an
Piano, Warm Pad, Lead, Round Bass (`instrument_katalog.py`, zuerst in einer Kopie gelaufen; nur die
Zeile `genres` ändert sich; `make_saiten_instrumente.py` danach ohne Änderung). `loop_werte.txt`:
ein Eintrag dazu, die übrigen unverändert. Test `test_LoopSpurenBeispiel`. Benutzerhandbuch 1.197.0;
Sichttest-Liste `.claude/handover/Sichttest_Spuren.md` (L, B, K, C, H, U).

## 4. Familie Hammer-Saite (Familie 1 aus SF9 a)

### 4.1 Zielbild (Patrik S141)

Antworten auf fünf Fragen (Claude S141, je mit Vorschlag):

1. **Instrumente: alle.** Zuerst Konzertflügel und Klavier (Upright); danach kommen ebenso
   Honky-tonk (verstimmte Saitenchöre), der E-Flügel (wie CP-70: Saiten mit Tonabnehmer), das
   Clavinet mit echter Tangente statt Zupfen, Hackbrett und Cimbalom gerechnet statt aufgenommen.
   Sie sind Vorlagen derselben Bauart (Hammer × Saite, SF10 a); nichts davon fällt weg.
2. **Was man hören muss: alles, wie vorgeschlagen.** Der Filzhammer (laut gespielt heller, nicht
   nur lauter); zwei bis drei Saiten je Ton mit Schwebung und zweistufigem Ausklang; die gestreckte
   Stimmung (Inharmonizität); der Resonanzboden; das Mitschwingen der übrigen Saiten bei
   gedrücktem Pedal; die Geräusche — Dämpfer, Pedal, Mechanik —, diese **abschaltbar**.
3. **Spielweisen: alle.** Sustain (mit halbem Pedal), Sostenuto, Una corda; dazu das Moderator- oder
   Übepedal (Upright), Abdämpfen mit der Hand und das präparierte Klavier.
4. **Messen:** Literatur (etwa Chaigne/Askenfelt, Bank) und Aufnahmen eines echten Flügels.
   Patrik hat keine eigenen — eine freie Sammlung wird gesucht, Lizenz und Inhalt geprüft, bevor
   sie benutzt wird.
5. **Rechenzeit:** Frage Patrik: „die Rechnung wäre ja einmalig beim Laden, oder? Der Composer
   könnte auch alle etwaigen Loops vorladen.“ — Antwort mit Befund aus dem Code in 4.2.

**Freie Aufnahmen (Suche S141):** *Salamander Grand Piano* (Alexander Holm, Yamaha C5) — auf
archive.org (`SalamanderGrandPianoV3`, abgerufen 2026-10-21) als Feld „Usage: Attribution 3.0“ und
„Licence: CC-by“; 48 kHz/24 bit, 16 Anschlagstufen je kleine Terz, dazu Hammergeräusch beim
Loslassen (chromatisch, eine Stufe), Saitenresonanz beim Loslassen (kleine Terzen, drei Stufen),
Pedalgeräusche. Die Behauptung, die Sammlung sei seit 2022 gemeinfrei, steht nur auf einer
Drittseite — nicht geprüft; es gilt CC-BY 3.0 (Nennung in `CREDITS.md`, wenn etwas davon
mitgeliefert würde; zum Messen nicht nötig). *University of Iowa MIS*: „may be downloaded and used
for any projects, without restrictions“ (theremin.music.uiowa.edu/mis.html), der Flügel nicht
genannt, in einem Unterrichtsraum aufgenommen. Ein Upright mit an der Quelle geprüfter Lizenz:
noch keins gefunden.

**Wunsch Patrik (S145): Beispiel-Loops mit der Hammer-Saite** — mindestens einer, nach Patriks Notiz
`docs/modellierung/hammer-saite/welcher komponist ist ideal für ein Flügel tet.txt` (eine Unterhaltung mit einem Sprachmodell;
ihre Angaben zu Lizenzen und Quellen sind **nicht geprüft**). Dort genannt: Chopin als Idealfall, als
Prüfstücke *Nocturne Op. 9 Nr. 2* (Pedal, Resonanz, Ausklang), *Ballade Nr. 1 g-Moll Op. 23* (Dynamik,
Anschlagstufen), *Étude Op. 10 Nr. 4* (Polyphonie, Rechenzeit); dazu Debussy *Clair de lune*, Liszt *La
Campanella*, Rachmaninow *Prélude cis-Moll*. Die Werke sind gemeinfrei (die Komponisten sind seit mehr als
70 Jahren tot); Ausgaben, MIDI-Dateien und Aufnahmen tragen eigene Rechte — die Quelle der Noten ist
deshalb ein eigener Entscheid. Welche Stücke, aus welcher Quelle: offen (BL1 ff.).

✅ **BL1 a** (Patrik S145): **Drei Ausschnitte von Chopin, je einer für eine Prüfung:** `nocturne_demo`
(Nocturne Op. 9 Nr. 2, die erste Phrase mit Pedal-Spur — Pedal, Mitschwingen, Ausklang),
`ballade_demo` (Ballade Nr. 1 Op. 23, eine Stelle von pp nach ff — Anschlagstufen, Filzhammer),
`etude_demo` (Étude Op. 10 Nr. 4, die ersten Takte — Rechenzeit, Töne vorab, Freeze). Grund: jeder
prüft einen anderen Teil des Zielbilds (§4.1 Antwort 2); Chopin ist unstrittig gemeinfrei. Verworfen: b
(nur die Nocturne — die Hälfte des Zielbilds ohne Hörbeispiel), c (dazu Debussy und Rachmaninow — was sie
prüfen, decken Nocturne und Ballade schon; lassen sich nach demselben Verfahren ergänzen). Kippt auf c,
wenn die Beispiele vor allem die Klangvielfalt zeigen sollen; auf b, wenn ein Ausschnitt viel Arbeit
kostet (BL2).

**Befund Quellen (S145, abgerufen 2026-10-22):** Mutopia Project, Liste „Composer=ChopinFF“
(`mutopiaproject.org/cgibin/make-table.cgi`, alle Seiten über ein Skript im Browser nach „Op. 10, No. 4“,
„Op. 23“ und „Ballade“ durchsucht; Felder wörtlich): *Nocturne* Op. 9, No. 2 — Vorlage „G. Schirmer, New
York, 1881“, Lizenz „Creative Commons Attribution-ShareAlike 3.0“; *Première Ballade* Op.23 — „Klindworth;
Bote & Bock,n.d.[1880]“, „Creative Commons Attribution-ShareAlike 4.0“; **Étude Op. 10 Nr. 4: nicht
gefunden** (vorhanden Op. 10 Nr. 1, 2, 5, 9, 12, je „Public Domain“). Die Liste sagt dazu: „No claim is made
as to the accuracy … of any of the material provided here.“ Andere Quellen (IMSLP, Sammlungen von
MIDI-Dateien) nicht abgerufen.

✅ **BL2 a** (Patrik S145), **mit Zusatz: weitere Quellen suchen.** **Claude überträgt jeden Ausschnitt
von Hand aus einer gemeinfreien Druckausgabe vor 1900** (Scan), geprüft Takt für Takt gegen den Scan;
ein Skript `make_fluegel_loops.py` schreibt die drei `.lvloop`; die Ausgabe steht in der Beschreibung des
Loops und in `CREDITS.md`; die Loops stehen unter derselben Lizenz wie die übrigen mitgelieferten. Grund:
eine Quelle und eine Lizenz für alle mitgelieferten Loops — ShareAlike (b) hinge an jeder späteren Fassung
dieser Dateien; die Étude fehlt bei Mutopia ohnehin. Verworfen: b (Mutopia per Skript, CC BY-SA), c
(Sammlungen von MIDI-Dateien — Bedingungen ungeprüft). Kippt auf b, wenn sich für ein Stück keine lesbare
gemeinfreie Ausgabe findet. Erklärungen beim Herunterladen bestätigt Claude nicht selbst.

**Befund weitere Quellen (S145, abgerufen 2026-10-22; Recherche eines Agenten, das Feld auf Polona von
Claude selbst nachgelesen):**
- **Polona** (Polnische Nationalbibliothek): jeder geöffnete Datensatz trägt im Feld „Prawa autorskie“
  wörtlich „Domena Publiczna. Wolno zwielokrotniać, zmieniać i rozpowszechniać oraz wykonywać utwór, nawet
  w celach komercyjnych, bez konieczności pytania o zgodę.“ (frei, auch kommerziell, ohne Erlaubnis; den
  Urheber nennen). Vorhanden: Nocturne Op. 9 Nr. 2 bei Ricordi [ca. 1865] und Gebethner i Wolff [ca. 1870];
  Ballade Op. 23 als Erstausgabe ca. 1836 (BJ Muz. 10789 III; welche der drei Erstausgaben, sagt der
  Datensatz nicht); Étude Op. 10, Liv. 1 bei Kistner ca. 1840 (BJ Muz. 10791 III 1 — ob Nr. 4 darin steht,
  am Scan zu prüfen).
- **IMSLP:** Nocturne Op. 9 als deutsche Erstausgabe Kistner [1833] (#86550), Breitkopf Gesamtausgabe
  [1880], Kullak/Schirmer 1881; Ballade Op. 23 als alle drei Erstausgaben 1836 und Breitkopf [1878] — je
  „Public Domain“, vor 1900 keine mit „Non-PD EU“. Grundsatz (IMSLP:Public_Domain): ein Scan eines
  gemeinfreien Werks sei „not generally considered copyrightable“; ohne Gewähr.
- **BnF Gallica:** nicht abrufbar (Bot-Prüfung, nicht umgangen); nach älteren Fassungen kostet die
  kommerzielle Weiterverwendung eine Lizenz — ungeprüft; der Scan der französischen Erstausgabe von Op. 23
  bei IMSLP stammt von dort.
- **OCVE/CFEO:** Bilder nur mit ausdrücklicher Erlaubnis der Bibliothek. **NIFC** (`chopin.musicsources.pl`)
  beim Abruf in Wartung.
- Nicht durchsucht: Library of Congress, Wikimedia Commons, HathiTrust direkt.

**Befund vor Stufe 31 (Code S145, gelesen):** Das mitgelieferte `piano` ist heute ein Sinus mit fünf
Obertönen und langem Decay (`asset/loop/instruments/piano.lvinst`, Fassung 3, ohne Saiten) — von der
Hammer-Saite übernimmt es nichts. Wiederverwendbar ist die Saite: der Wellenleiter mit Verlust- und
Allpass-Umlauf für die Inharmonizität (`entwerfeUmlauf`, `rechneSaiteWg`), die Abnahme am Steg und der
Korpus als gekoppelte Schwinger (Parlor, Classical) als Vorbild für einen Resonanzboden. Pedale wirken
heute auf die **Dauer** eines Tons (`loop/Pedalplan`: `gehaltenBis`, `gehaltenMs`, Töne je Vorkommen) —
der Ton einer Note wird für sich gerechnet und unter seinem Namen im Tonspeicher gehalten. **Das
Mitschwingen der übrigen Saiten bei gedrücktem Pedal** hängt dagegen von allen Noten ab, die zugleich
klingen; es passt nicht in „ein Ton je Name“. Einen Weg für Töne, die eine Note zusätzlich anstößt, gibt
es schon: die `zusatz`-Töne einer Kette (Stufe 25c, `Rechenkern.cpp` beim Schlagen). Wie das Mitschwingen
gerechnet wird, ist deshalb ein eigener Entscheid nach der Recherche (Literatur zu den Rechenverfahren:
im Gang, S145).

**Befund Literatur (S145, abgerufen 2026-10-22; Recherche eines Agenten — die Sätze aus Bank, Zambon &
Fontana 2010 von Claude im Volltext nachgelesen, die übrigen nur aus dem Bericht):**
- **Modale Synthese** (Bank, Zambon, Fontana, „A Modal-Based Real-Time Piano Synthesizer“, IEEE TASLP 18(4),
  809–821, 2010 — *nachgelesen*): Quer- und Längsschwingung der Saite als Resonatoren zweiter Ordnung, der
  Hammer nichtlinear (F = K·Δy^p); Schwebung und zweistufiger Ausklang über „a set of secondary resonators
  to the string model, driven by the hammer force“; das Mitschwingen **einseitig**: „the outputs of the main
  string models are summed and feed to the secondary resonators of all string models“, mit R = 8 Bereichen
  „an additional 64 multiplications and additions“; volle Polyphonie mit „10000 second-order resonators and
  four 20000 tap“ Faltungen in Echtzeit; Dämpfer durch „simply decreasing the decay times of the
  resonators“. Zum Wellenleiter mit gemeinsamer Abschlussstelle: Mitschwingen „relatively straightforward“;
  zum „commuted“-Verfahren: es „assumes linearity and time-invariance“, Wiederanschlag und nichtlineare
  Saite „cannot be precisely modeled“. Resonanzboden: FIR „1000-2000 tap“, für das Klopfen in Mitte und
  Diskant „ten thousands of taps“ — als FFT-Faltung oder parallele Filter zweiter Ordnung.
- **Wellenleiter mit Hammer** (Smith/Van Duyne 1995, Bank & Sujbert 2004 — *nur Bericht*): am billigsten;
  bloßes Summieren verstimmter Saiten gebe keinen echten zweistufigen Ausklang, dafür Kopplung am Steg; ein
  Hammer an einer Saite mit Dispersion werde fast so teuer wie finite Differenzen.
- **Finite Differenzen** (Bank & Sujbert 2004; Chabassier, Chaigne, Joly 2016 — *nur Bericht*): am
  treuesten; nur die Saite 10–20 Mal so teuer wie ein Wellenleiter; der ganze Flügel mit Platte und Luft
  „24 h of real time on 300 processors“ je Sekunde Klang.
- **Hammer** (Russell; Stulov & Kartofelev 2009 Gl. 9 — *nur Bericht*): F = K·x^p, p etwa 2 im Bass bis 4 im
  Diskant, mit Hysterese; Stulov gibt Masse, p und Steife je Taste n = 1…88 als Formel. **Inharmonizität**
  (Rigaud & David, DAFx 2011 — *nur Bericht*): B(m) als Summe zweier Exponentiale, der Diskant fest.
  Nicht erreicht: Chaigne & Askenfelt 1994, Weinreich 1977, Conklin 1996, Fletcher 1964 (Volltexte hinter
  Schranken).
- **Folge für LumiViz (Claude):** In allen drei Verfahren ist das Mitschwingen eine Stufe **auf der Summe**
  der klingenden Saiten — in der modalen Fassung einseitig und damit nach dem Vorab-Rechnen der einzelnen
  Töne möglich: ein Satz ungedämpfter Resonatoren, gespeist von der Summe, im Rechenkern wie ein Effekt
  des Kanals.

✅ **HS1 a** (Patrik S145): **Die Hammer-Saite wird modal gerechnet, nach Bank, Zambon & Fontana 2010.**
Jede Saite eine Summe gedämpfter Resonatoren mit Frequenzen nach der Inharmonizität B, Quer- und
Längsschwingung; der Filzhammer nichtlinear und gekoppelt (F = K·Δy^p, Werte je Taste nach Stulov);
Schwebung und zweistufiger Ausklang über Nebenresonatoren, getrieben von der Hammerkraft; Resonanzboden als
Faltung; das Mitschwingen einseitig auf der Summe. Ein neues Modell neben dem Wellenleiter — die gezupfte
Saite bleibt, wie sie ist. Grund: jeder Teilton trägt Frequenz, Abklingzeit und Schwebung selbst — was aus
*Salamander* gemessen wird, lässt sich direkt setzen und prüfen; der Hammer bleibt nichtlinear (laut heller,
§4.1); für das Mitschwingen gibt es einen nachgelesenen stabilen Weg, der zum Vorab-Rechnen passt.
Verworfen: b (Wellenleiter erweitern — ein Hammer an einer Saite mit starker Dispersion werde fast so teuer
wie finite Differenzen, nur Bericht; acht Allpässe reichen für den Bass wohl nicht, nicht belegt), c (finite
Differenzen für die Saite — am treuesten, aber nur über physikalische Größen abzugleichen, nicht über das
Gemessene). Kippt auf c für die Saite, wenn Hörproben gegen *Salamander* im Anschlag etwas vermissen, das nur
die Saite in finiten Differenzen trägt.

**Befund (Code S145):** Ein Ton im Tonspeicher trägt heute keine Anschlagstärke — sie wirkt als Pegel auf
den fertigen Ton (`TonAngabe`, `Loop.hpp`). Mit dem nichtlinearen Hammer klingt ein leiser Anschlag anders
als ein lauter; die Stärke muss in den Namen des Tons (Frage in 31a).

✅ **HS2 a** (Patrik S145): **Stufe 31 in acht Teilschritten, je mit Entscheiden davor.** **31a** Modell und
Datei (Bauart Hammer-Saite an der Schicht; Felder der `.lvinst`: Saitenchor, Inharmonizität, Hammer,
Dämpfer, Resonanzboden; die Anschlagstärke im Namen des Tons). **31b** ein Ton (modale Saite mit
Längsschwingung, nichtlinearer Hammer, Chor und Nebenresonatoren, Dämpfer beim Loslassen; gemessen am
gerechneten Ton). **31c** Resonanzboden als Faltung (woher die Impulsantwort kommt: eigene Frage). **31d**
Messung gegen *Salamander* (Teiltöne, B, Abklingzeiten, Helligkeit je Stärke) und Hörproben, die Patrik
gesammelt später hört. **31e** Mitschwingen — erst die Rechenzeit beider Wege messen (Stufe auf der Summe im
Rechenkern oder in jeden Ton eingerechnet, exakt wegen der einseitigen, linearen Kopplung), dann der
Entscheid, dann der Bau. **31f** Spielweisen (Sustain mit halbem Pedal, Sostenuto, Una corda; Geräusche
abschaltbar). **31g** Creator und Editor (zuerst die Skizze). **31h** Abschluss: `grand_piano`, die drei
Beispiel-Loops (BL1 a), Handbuch, Sichttest-Liste, Bereichslauf. Danach das Upright als Vorlage derselben
Rechnung (Stufe 32). Grund: jede Wahl fällt, wenn ihr Grund vorliegt — beim Mitschwingen die gemessene
Rechenzeit, beim Boden die Herkunft der Antwort; die Regler im Creator folgen aus den Größen des Modells.
Verworfen: b (das Mitschwingen schon jetzt als Stufe auf der Summe — verwürfe den exakten Weg „im Ton“ ohne
Messung), c (die Skizze von Creator und Editor zuerst — die Regler stehen erst nach 31a/31b fest). Kippt auf
c, wenn Patrik den Flügel früh im Creator sehen will.

✅ **HA1 a** (Patrik S145): **Der Flügel in der `.lvinst`: Kurven über die Tastatur, mit Abweichung je
Taste.** Jede Größe — Inharmonizität B, Abklingzeit des Grundtons und ihr Abfall zu den hohen Teiltönen,
Zahl der Saiten im Chor und ihre Verstimmung in Cent, Hammermasse, Hammersteife, Exponent p, Anschlagstelle,
Stärke des Dämpfers — ist eine Kurve aus Stützpunkten an Tasten, dazwischen glatt; eine Taste kann eine
Abweichung tragen (leer = die Kurve). Vorgaben aus der Literatur (Hammer nach Stulov — seit HT2 a nach
Chabassier & Duruflé —, B nach Rigaud & David), wo gemessen, Stützpunkte aus *Salamander*. Grund: schließt die Tabelle je Taste ein und hält die
Absicht lesbar; Messung (rund 30 Tasten) und Literatur (Formeln über die Tastatur) liefern beide Stützpunkte.
Verworfen: b (Tabelle je Taste — 88 Zahlen je Änderung, kein Verlauf im Creator), c (physikalische Größen je
Saite — gegen den Grund von HS1 a, Saitendaten eines bestimmten Flügels nicht belegt). Kippt auf b, wenn 31d
zeigt, dass fast jede Taste eine Abweichung braucht.

✅ **HA2 a** (Patrik S145): **An der Schicht bekommt die Schwingung *String* die Achse *Excitation*
(*Pluck* — die Vorgabe, wie bisher — oder *Hammer*) und das Verfahren *Modal*; die Hammer-Saite ist
*Hammer* + *Modal*.** Kurven (HA1 a) und Resonanzboden hängen an dieser Schicht. Am Instrument ein eigener
Block *Keyboard* für die Ausstattung (HA4), neben dem `Saitensatz`, der den gegriffenen Saiten bleibt.
Kombinationen, die es noch nicht gibt (*Hammer* mit *Waveguide*), sind in Datei und Creator nicht wählbar.
Grund: setzt SF10 a um (zwei Achsen, eine neue Kombination ergibt sich von selbst); Clavinet und Bogen-Saite
werden neue Werte auf denselben Achsen. Verworfen: b (eigene Schwingung *Hammer string* — der Weg, den SF10 b
verwarf), c (die Tastatur im `Saitensatz` — Tabs, Saitenwahl und Capo an einer Taste). Kippt auf b, wenn beim
Clavinet Hammer und Zupfen an der Saite nichts teilen außer dem Namen.

✅ **HA3 a** (Patrik S145): **Die Anschlagstärke gestuft im Namen, der Rest als Pegel.** Die Stärke wird
auf Stufen gleichen Abstands in dB der Hammergeschwindigkeit gerundet (etwa 1 dB), die Stufe steht im Namen
(`:v<stufe>` — seit HK2 a `:a<stufe>`, nur bei einem Instrument mit Hammer); den Unterschied zwischen Note und Stufe gleicht ein Pegel
genau aus — die Lautstärke stimmt, die Klangfarbe auf eine Stufe. Wie fein, legt 31d fest (so, dass man die
Klangfarbe nicht unterscheidet). Ein kleines Ziehen an der Stärke spielt sofort, ein großes rechnet neu.
Grund: genau, wo man es hört, und der Editor bleibt bedienbar. Verworfen: b (genau, auf drei Stellen — jede
kleine Änderung rechnet einen ganzen Ton neu), c (acht Stufen mit Überblendung — ein Klang, den der Flügel
nicht hat, mit Schwebungen zwischen den Lagen). Kippt auf b, wenn 31b zeigt, dass ein Ton nur Millisekunden
braucht. Mit Pedal werden die Töne ohnehin je Vorkommen gerechnet; dort spart a wenig.

✅ **HA4 a** (Patrik S145): **Der Block *Keyboard* trägt Umfang, Dämpferbereich und die Pedale als Liste:**
`range` (tiefste und höchste Taste, Vorgabe A0–C8), `undampedFrom` (die erste Taste ohne Dämpfer — der Wert
wird in 31a belegt; aus dem Gedächtnis etwa die obersten anderthalb Oktaven), `pedals` je Platz links, Mitte,
rechts mit einer Art: links *Una corda* (Verschiebung) oder *Soft* (Hämmer näher), Mitte *Sostenuto*,
*Practice* (Moderator) oder keins, rechts *Sustain*. Jede Art bringt ihre Spur in den Instrument-Editor
(SF6 a); die Vorlagen *Grand* und *Upright* setzen die Werte. Grund: schließt die Schalter ein und trägt, was
das Zielbild verlangt (Moderator, das linke Pedal des Upright), ohne neues Feld in Stufe 32. Verworfen: b
(drei Schalter — das Upright bräuchte ein neues Feld), c (keine Ausstattung — schiebt auf, was HS2 a für das
Upright als Vorlage derselben Rechnung festlegt). Kippt auf b, wenn die Recherche zum Upright zeigt, dass sich
seine Pedale nicht auf diese Plätze abbilden lassen.

**Befund Vorgaben zu 31a (S146, abgerufen 2026-10-23; von Claude selbst gelesen, die Formeln am Bild der
Seite, weil die Textebene die Vorzeichen verliert):**
- **Hammer — Stulov & Kartofelev**, „Piano hammer-string interaction: the influence of the elastic parameters
  of bass hammers on the contact time duration“, *Acoustics High Tatras 2009*, S. 3, Gl. 9 (nach Stulov 2005,
  *Acta Acustica* 91): m = 11,074 − 0,074 n + 10⁻⁴ n² [g]; p = 3,7 + 0,015 n; Q0 = 183 exp(0,045 n)
  [N/mm^p]; ε = 0,9894 + 8,8·10⁻⁵ n²; τ = 2,72 − 0,02 n + 9·10⁻⁵ n² [µs] — „for all hammer numbers
  1 ≤ n ≤ 88“. Für langsames Laden gilt F = Q0·u^p (Gl. 4, „Q0 is the static hammer stiffness“), die
  Anschlaggeschwindigkeit „is not greater than 5 m/s“ (S. 3). **Befunde:** p wächst von 3,7 (A0) auf 5,0
  (C8) — der Bericht eines Agenten in S145 („p etwa 2 im Bass bis 4 im Diskant“) war falsch. ε überschreitet
  ab n ≈ 11 den Wert 1 und ist damit nur im Bass brauchbar (die Arbeit rechnet n = 1, 5, 10); für F = Q0·u^p
  braucht es ε nicht. Die Masse fällt von 11,0 g (A0) auf 5,3 g (C8).
- **Inharmonizität — Rigaud, David & Daudet**, „A parametric model of piano tuning“, *DAFx-11*, S. 394,
  Gl. 4 und 5: B(m) = e^(s_B·m + y_B) + e^(s_T·m + y_T), m die MIDI-Nummer 21…108; die Asymptote des
  Diskantstegs ist für alle Flügel gleich, „s_T ≃ 9.26·10⁻², y_T ≃ −13.64“; s_B, y_B je Klavier aus drei
  Tönen A0–D3 geschätzt — **eine Tabelle dieser Werte gibt die Arbeit nicht.** Der Bassanteil kommt deshalb
  aus *Salamander* (31d); bis dahin vorläufig.
- **Hammergeschwindigkeit — KTH**, *Five Lectures on the Acoustics of the Piano* (Askenfelt, Hg.), Seite
  „motions“: „In the forte example, the maximum hammer velocity is about 5 m/s (18 km/h)“ (C4). Ein Wert für
  pp nicht gefunden (0,3–0,5 m/s auf derselben Seite sind Geschwindigkeiten der Taste).
- **Erste Taste ohne Dämpfer — kein Beleg an der Quelle.** Kawai (FAQ, kawaius.com): „the strings for the
  uppermost notes are not fitted with dampers“, ohne Zahl; Yamaha-Händler (markgoodwinpianos.co.uk) ebenso
  ohne Zahl. Zahlen nur aus Foren und Suchzusammenfassungen (E6 bis G6, 18 bis 22 Töne; eine Seite mit „note
  #69“ trug beim Abruf etwas anderes) — nicht zitierfähig. Vorgabe deshalb vorläufig, in 31d an *Salamander*
  zu prüfen (die Aufnahmen des Loslassens sind chromatisch).

✅ **HK1 a** (Patrik S146): **Jede Größe hat eine feste Skala — logarithmisch für B, Q0, Abklingzeit und
Stärke des Dämpfers, linear für p, Masse, Cent, Zahl der Saiten und Anschlagstelle; in dieser Skala wird
zwischen den Stützpunkten monoton kubisch interpoliert (Fritsch–Carlson, wie PCHIP), vor dem ersten und nach
dem letzten Punkt gilt der Randwert.** Die Zahl der Saiten wird danach auf eine ganze Zahl gerundet; eine
Abweichung je Taste ersetzt den Wert der Kurve dort. Grund: in der logarithmischen Skala werden die Formeln
von Rigaud (Diskant) und Stulov (Q0) Geraden, wenige Stützpunkte treffen sie, kein Wert wird negativ; monoton
kubisch schießt nicht über die Punkte und löst das „glatt“ aus HA1 a ein. Verworfen: b (linear in der Skala —
Knicke an den Stützpunkten, gegen „glatt“, in 31d als scheinbare Abweichung), c (linear in den Werten — B und
Q0 brauchten viele Punkte). Kippt nicht, solange HA1 a gilt; mit HA1 b (Tabelle) wäre die Interpolation
gleichgültig.

✅ **HK2 a** (Patrik S146), **mit Zusatz: gilt, solange keine Messung etwas anderes belegt.** **Die
Lautstärke einer Note ist die Hammergeschwindigkeit im Verhältnis zur größten: v = vMax · laut, vMax = 5 m/s
(KTH, forte; Stulov: „not greater than 5 m/s“).** Die Stufe ist round(20·lg laut / Schritt), der Schritt eine
Konstante von 1 dB, bis 31d ihn festlegt; im Namen `:a<stufe>` (ganze Zahl ≤ 0, `:a-6`), bei Stufe 0 ohne
Zusatz, nur an einem Instrument mit Hammer. Untergrenze −40 dB (0,05 m/s), darunter nur der Pegel; den Rest
gleicht der Pegel laut / 10^(stufe·Schritt/20) aus. **HA3 a nennt `:v<stufe>` — `:v` ist seit Stufe 25b der
Verlauf einer Kette; der Buchstabe wird `:a`.** Grund: die Lautstärke kommt aus der Physik des Hammers, `laut`
behält im Verhältnis seine Bedeutung; vMax ist der einzige belegte Wert; −40 dB liegt unter jedem pp (höchstens
41 Töne je Taste); die Namen bleiben eindeutig. Verworfen: b (Kurve *Velocity* im Block *Keyboard* — ohne
Messung kein Grund für eine andere als die Gerade; später ohne Bruch nachzutragen), c (`:v` nach dem Vorzeichen
unterscheiden — zerbrechlich). Kippt auf b, wenn 31d zeigt, dass die 16 Anschlagstufen von *Salamander* nicht
auf der Geraden liegen.

**Gebaut 31a (S146):** `loop/HammerSaite` — `Tastenkurve` mit `wertAn` (HK1 a), `HammerSaite` mit zehn Kurven
(`inharmonicity`, `decay`, `highDecay`, `strings`, `detune`, `hammerMass`, `hammerStiffness`, `hammerExponent`,
`strikePosition`, `damper`) und `soundboard` (nur `amount`, bis 31c), `Tastatur` (`range`, `undampedFrom`,
`pedals`), `anschlagStufe`/`hammerTempo`/`anschlagRest` (HK2 a). An der Schicht `Anregung` und
`SaitenModell::Modal`, `richteSchicht` hält die Paare aus HA2 a; `istSaitenSchicht` zählt die Hammer-Saite
nicht (keine gegriffene Saite), `istHammerSchicht`, `hatHammer`. `.lvinst` **Fassung 16**; `TonAngabe::anschlagStufe`
mit `:a<stufe>` im Namen. **Vorgaben:** Hammer nach Stulov Gl. 9 und B nach Rigaud Gl. 5 an A0…A7, C8 — den
Bassteg aus Abb. 1 der Arbeit pixelgenau abgelesen (s_B = −0,0618, y_B = −6,89; ein Beispiel ohne genannten
Flügel); **vorläufig ohne Beleg** Abklingzeit (25 s an A0 bis 0,5 s an C8), *High decay* 0,2, Chor (eine Saite
bis F#1, zwei bis C3, drei darüber), Verstimmung 1 Cent, Anschlagstelle 1/8, Dämpfer (0,8 s bis 0,2 s),
`undampedFrom` G6 — alle setzt 31d. **Noch nicht:** die Hammer-Saite schweigt bis 31b (sie rechnet nicht als
Classic); die Planung setzt die Stufe noch nicht aus der Lautstärke der Note (mit dem Ton in 31b, weil der Rest
dann als Pegel an die Stelle von `laut` tritt); im Creator gibt es *Modal* noch nicht (31g). Tests:
`test_LoopHammerSaite` (5 Fälle).

**Befund vor 31b (S146, Bank, Zambon & Fontana 2010, Fassung der Autoren `home.mit.bme.hu/~bank/publist/taslp10.pdf`,
von Claude im Volltext gelesen):**
- **Saite:** je Mode ein Resonator zweiter Ordnung, diskretisiert nach der Impulsinvarianz (Gl. 10, 11) — das
  bringt eine Verzögerung um einen Takt und damit keine verzögerungsfreie Schleife mit dem Hammer. Die Mode wird
  getrieben mit b0,k = 2/(Lµ) (Gl. 4c), Anfangsamplitude A_k = 1/(Lµ f_k) (Gl. 5b); Ein- und Abnahme am Hammer
  sin(kπ x_h/L) (Gl. 14, 15), die Kraft am Steg mit w_b,k = kπT0/L (Gl. 16). Verluste 1/τ_k = b1 + b3·(2πf_k)²
  (Gl. 7) — dieselbe Form wie *High decay* aus 31a.
- **Hammer:** Masse und Feder F = K·Δy^p (Gl. 12), die Masse über die Impulsinvarianz mit einem Takt Verzögerung
  (Gl. 13); läuft nur während des Kontakts. „We neglect these secondary phenomena“ — Hysterese und Schaft
  (S. 5).
- **Längsschwingung:** die Spannung T(t) = T0 + ES·π²/(4L²)·Σ n²·y_n² (Gl. 26), dazu K = 2…10 Längsmoden als
  Hochpass-Resonatoren (Gl. 27, 28), angeregt von Paaren der Quermoden (Gl. 22b; eine Kraft für die ungeraden,
  eine für die geraden). Die Längsgrundfrequenz liegt „around 16 to 20 times higher than that of the transverse
  vibration“ (S. 6, nach Conklin 1996). Gegen Aliasing nur Quermoden mit f < fs/4 in die Produkte.
- **Schwebung, zweistufiger Ausklang:** „secondary resonators … driven by the hammer force“, als weitere
  Saitenmodelle mit weniger Moden (S. 9); dieselbe Bank nimmt später das Mitschwingen auf (31e).
- **Dämpfer:** „simply decreasing the decay times of the resonators“ (S. 12).
- **Folge für LumiViz (Claude):** Der Hammer sieht die Saite über ihre Masse Lµ, die Längsschwingung braucht
  dazu L (oder µ) und das Verhältnis der Längs- zur Querfrequenz; Ebene und Pegel der Nebenresonatoren sind
  eigene Größen. **Keine davon steht in den Kurven aus 31a.** Ohne sie lassen sich Masse und Steife nach Stulov
  nicht einsetzen (sie wirken nur gegen eine Saitenmasse). Eine offene Tabelle der Saitendaten je Taste ist
  noch nicht gefunden: Chaigne & Askenfelt 1994 (drei Töne) beim Abruf nicht erreichbar; Banks Dissertation
  2006 nennt die Werte einer G1-Saite nur im Bild.

✅ **HT1 a** (Patrik S146): **Drei weitere Kurven über die Tastatur (HK1 a): *String length* L in m (log),
*String mass* µ in g/m (log), *Longitudinal ratio* f_l0/f0 (linear); T0 und ES folgen aus f0 = (1/2L)√(T0/µ) und
f_l0 = (1/2L)√(ES/µ).** Die Frequenzen bleiben, wie sie sind (f0 aus der Taste, B aus der Kurve). Vorgaben aus
einer veröffentlichten Saitentabelle eines Konzertflügels, vor dem Bau an der Quelle nachgelesen; das
Verhältnis vorläufig 17 (Bank 2010: „around 16 to 20“). Weiter in Fassung 16 (nicht ausgeliefert). Grund: gibt
dem Modell aus HS1 a, was seine Gleichungen verlangen; der Hammer bleibt in den Einheiten der Literatur, die
Längsschwingung in 31b (HS2 a); der Grund gegen HA1 c entfällt mit einer veröffentlichten Tabelle. Verworfen: b
(nur die Masse Lµ, Längsschwingung später — schiebt das Zielbild hinaus, ohne zu sparen), c (Verhältnis der
Massen — verliert die Einheiten). Kippt auf b, wenn sich keine offene Tabelle findet.

**Befund Saitendaten (S146; beide Arbeiten von Patrik heruntergeladen, liegen unter `docs/modellierung/hammer-saite/quellen/` — nicht
versioniert; von Claude gelesen, Tabellen am Bild der Seite geprüft):**
- **Chabassier, Joly & Chaigne**, „Modeling and simulation of a grand piano“, *JASA* 134 (2013), Fassung der
  Autoren hal-00873089, Tab. III (S. 48): Saiten und Hämmer für D♯1, C2, F3, C♯5, G6; die Werte wurden an einem
  Steinway D gewonnen („used for extracting accurate values of input parameters“, S. 41).
- **Chabassier & Duruflé**, „Physical parameters for piano modeling“, INRIA RT-0425 (2012), hal-00688679,
  Anhang A „Wrapped strings values“: je Taste C1 bis B7 (84 Töne; A0–B0 und C8 fehlen) L, d, ρ, T0, Dämpfung,
  Hammer, Anschlagstelle; umwickelte Saiten als homogenes Material höherer Dichte, E = 2,02·10¹¹ Pa (§3.2). Der
  Hammer als Formel über die Taste i = 1…88 (Gl. 2–4): M_H = −6,2348·10⁻⁵ i + 0,0112 [kg]; p = 2,4295·10⁻⁴ i²
  − 0,007703 i + 2,337; K_H = 10^(5,3097·10⁻² i + 7,6425) [N·m⁻ᵖ] — gegen die Tabelle bei C1 (i = 4) geprüft.
- **Daraus gerechnet (Claude):** µ = ρπd²/4 von 99 g/m (C1) über 14,3 (E2) | 7,7 (F2, Sprung am Bassteg) auf
  3,4 g/m (B7); L von 2,007 m über 1,378 (E2) | 1,837 (F2) auf 0,052 m; f_l0/f0 = √(EA/T0) zwischen 12,2 und
  15,7 (unter Banks „around 16 to 20“); x_H/L zwischen 0,113 und 0,127, fast überall 0,120. Monoton kubisch
  (HK1 a) treffen 11 Punkte L, 12 Punkte µ, 6 Punkte f_l0/f0 die Tabelle auf unter 2 %.
- **Widerspruch beim Hammer:** RT-0425 und JASA geben p = 2,3 … 3,5, Stulov 2009 p = 3,7 … 5,0; die Massen
  liegen nah (11,2 → 5,7 g gegen 11,0 → 5,3 g). Die Hämmer von RT-0425 sind mit diesen Saiten zusammen
  gerechnet und gegen den Steinway D verglichen; die von Stulov stammen aus einem Prüfstand für einzelne
  Hämmer. RT-0425 gibt dazu einen Verlustterm R_H (Gl. 5), den Bank 2010 vernachlässigt.

✅ **HT2 a** (Patrik S146): **Die Vorgaben von Masse, p und Steife des Hammers kommen aus RT-0425 Gl. 2–4, mit
derselben Taste i = MIDI − 20 wie die Saiten; die Steife wird in die Einheit der Kurve umgerechnet
(Q0 = K_H·10⁻³ᵖ N/mm^p); ohne R_H (wie Bank 2010).** HA1 a heißt damit „Hammer nach Chabassier & Duruflé“,
Stulov bleibt Beleg im Befund. Grund: Hammer und Saite sind dort ein abgestimmter, am Steinway D verglichener
Satz — die Helligkeit eines Anschlags hängt vor allem an ihrem Verhältnis. Verworfen: b (Stulov mit den Saiten
aus RT-0425 — ein ungemessenes Verhältnis), c (dazu R_H als Kurve — Bank vernachlässigt ihn, ob man ihn hört,
zeigt 31d; ohne Bruch nachzutragen). Kippt auf b, wenn in 31d die Helligkeit je Stärke gegen *Salamander*
(Yamaha C5) mit Stulov besser passt.

**Nachgezogen in 31a (S146, nach HT1 a, HT2 a):** `HammerSaite` trägt dazu `stringLength`, `stringMass`,
`longitudinalRatio` (Fassung 16 bleibt); der Hammer nach RT-0425 Gl. 2–4 mit Stützpunkten alle sechs Tasten (Q0
ist in ln quadratisch in der Taste — an den A-Tasten allein bis 4 % daneben); Länge, Masse und Längsverhältnis
aus Anhang A mit 11, 12 und 6 Stützpunkten; die Anschlagstelle belegt 0,12. Die Funktionen nach Stulov sind
aus dem Code entfernt (Beleg bleibt im Befund). Tests in `test_LoopHammerSaite` gegen die Tabelle an Tasten, die
keine Stützpunkte sind.

**Befund Chor und zweistufiger Ausklang (S146; Bank, Dissertation 2006, Kap. 4.2, S. 54–58, von Claude
gelesen; Weinreich 1977 selbst nicht gelesen):** Die Kopplung der Saiten über den Steg führt zu „beating and
two-stage decay“; beim Klavier klingen „two or three slightly mistuned strings“, „leading to a four or six
vibrating modes for one partial“. Bank bildet das als **Signalmodell** nach: je Teilton ein zweiter Resonator,
dessen Frequenz, Abklingzeit und Amplitude aus der Analyse einer Aufnahme kommen (eine „beating sinusoid“ auf
die Abweichung vom einfachen Abklingen gepasst), „five to ten resonators … enough“; die Anfangsamplituden kommen
dabei aus dem physikalischen Modell, „the beating and two-stage decay are modeled by a signal model“. Bank 2010
nimmt statt weniger Resonatoren eine ganze zweite Bank, vom Hammer getrieben (S. 9). **Größen dafür nennen
beide nicht.** Die Physik dahinter (nach Weinreich, wie Bank sie zusammenfasst): Saiten eines Chors, die über
denselben Steg Energie abgeben, schwingen in einer Gleichlage, die schnell Energie an den Steg verliert, und in
Gegenlagen, die das kaum tun — die zweite Stufe; Verstimmung und ungleicher Anschlag legen Energie in die
Gegenlagen.

✅ **HT3 a** (Patrik S146): **Je Teilton der Chor als N gekoppelte Moden nach dem Bild von Weinreich: N Saiten,
jede um ihre Verstimmung versetzt, alle über einen gemeinsamen Steg gedämpft, vom selben Hammer angestoßen; je
Teilton die Eigenwerte eines N×N-Systems (N ≤ 3) — Frequenz, Abklingzeit und Amplitude jeder Mode, Schwebung und
zweite Stufe ergeben sich.** `decay` ist die Abklingzeit der ersten Stufe; neue Kurve *Late decay* (T60 der
zweiten Stufe in s, log, Fassung 16), vorläufig bis 31d. Grund: bildet ab, was im Flügel geschieht; zwei an
jeder Aufnahme messbare Größen bestimmen mit der Verstimmung den Verlauf (HS1 a); die Verstimmung wirkt auf die
Nachklangphase wie beim Stimmen. Verworfen: b (Nebenbank nach Bank 2010 mit Pegel und Abklingzeit — Werte eines
Signalmodells, die Verstimmung wirkt nicht auf die zweite Stufe), c (nur Schwebung — schiebt das Zielbild bis 31d
hinaus). Kippt auf b, wenn 31d zeigt, dass die gekoppelten Moden die Hüllkurven von *Salamander* auch mit gepassten
Größen nicht treffen; auf c, wenn 31b zeigt, dass drei Saiten je Teilton zu teuer sind.

**Herleitung zu HT3 a:** steht seit S146 in `docs/modellierung/hammer-saite/modell.md` §5 (gekoppelte Moden
A = diag(iω_j − σ_s) − σ_b·J, zwei Stufen σ_früh = σ_s + N·σ_b und σ_spät = σ_s, erst die Verstimmung legt Energie
in die zweite Stufe).

**Gebaut 31b-1 (S146):** `loop/HammerTon` — `chorModen` (Säkulargleichung R(z) = Q(z) + σ_b·Q′(z), Durand–Kerner,
g = (1ᵀv)²/(N·vᵀv)), `rechneHammerTon`, `hammerNachklang`. Je Teilton bis 0,45·fs (höchstens 400) die Moden
des Chors als komplexe Pole (Impulsinvarianz, ein Takt Verzögerung); der Kontakt achtfach feiner, bis der Hammer
1 ms frei ist (höchstens 30 ms); danach je Mode ein abklingender Zeiger, beim Loslassen der Pol des Dämpfers
(nie langsamer als vorher), abgebrochen unter −140 dB der Summe; f0 so, dass der erste Teilton auf der
Tonhöhe liegt; Ausgabe die Kraft am Steg / `kHammerKraftBezug` (200 N, von Claude gesetzt), 5 ms Ausblenden am
Ende. Kurve *Late decay* (`lateDecay`, vorläufig dreimal *Decay*). In `rendereTon` bestimmt die Hammer-Saite
ihre Länge selbst (Dämpfer, ohne Dämpfer die zweite Stufe; ADSR und *Signal path* wirken an ihr nicht).
**Gemessen** (`test_LoopHammerTon`, Release-clang): E1 fällt in 3 s um 8,14 dB, soll 8,14 (*Decay* 22,1 s);
Teiltöne 5, 10, 15 auf 1 Cent nach B; C3 ff gegen pp: Grundton +19,8 dB, Teilton 8 zum Grundton −21,3 gegen
−32,2 dB (heller); C4 mit 2 Cent Verstimmung früh 13,6 dB/s, später 2,5 dB/s, ohne Verstimmung eine Stufe; C4
mit Dämpfer über 200 dB in 0,6 s, C7 ohne Dämpfer 23 dB (seine erste Stufe). Spitzen 0,24 … 0,54. Hörproben:
`LUMIVIZ_31B_WAV=<Ordner>`. **Noch nicht:** Längsschwingung (31b-2), die Stufe aus der Lautstärke der Note und
die Rechenzeit (31b-3).

**Befund 31b-2 (S146, gemessen und nachgelesen):**
- **Längsschwingung nach Bank 2010 eingebaut, mit gleichem Gewicht neben der Querkraft am Steg** (Spannung
  Gl. 26, Längsmoden Gl. 27/28, Anregung Gl. 22b mit den Kräften von Mode 3 und 4; Abklingzeit der Längsmoden
  ein Zehntel von *Decay* nach Bank, Diss. S. 80: „the decay time of the longitudinal modes are an order of a
  magnitude lower than that of the transverse modes“; Hochpass 20 Hz gegen den Gleichanteil der Spannung, von
  Claude). Gemessen: neunfaches ES hebt den Phantomteilton f10 + f11 bei E1 um 18 dB, der Grundton bleibt (auf
  0,001 dB). **Aber die Längskraft beherrscht dann den Klang:** C3 ff Teilton 8 zum Grundton −6 dB (vorher
  −21), Spitzen bis 2,2 (vorher 0,54), bei E1 ff überdecken die Phantomteiltöne die hohen Querteiltöne.
- **Die Auslenkung ist nicht zu groß:** gegen Chabassier, Joly & Chaigne 2013, Tab. IV (dieselben Saiten und
  Hämmer, finite Differenzen) liegt die größte Auslenkung je Durchmesser im Bass bei 0,4, in Mitte und Diskant
  bei 0,6 der Tabelle (z. B. F3 bei 3 m/s 1,39 gegen 2,10; C♯5 bei 4,5 m/s 1,57 gegen 2,64). Kontakt 1,3 … 5,7 ms,
  Kraft 4 … 80 N.
- **Die Längskraft wirkt am Steg nur mit sin α:** Chabassier, Joly & Chaigne 2013, Gl. 21 —
  F_b ≈ cos α·(Querterme) + sin α·(EA·∂ξ/∂x + (EA − T0)·(∂y/∂x)²/2), α der Winkel der Saite gegen die Ebene
  am Steg; „Due do the angle α a vertical force component is transmitted to the soundboard“ (S. 16), „a plausible,
  though not fully proved, way“. **Einen Wert für α nennt die Arbeit nicht**, RT-0425 ebenso wenig; Patente und
  ein Forum der Klaviertechniker nennen 1 bis 5 Grad, je nach Lage verschieden — nicht zitierfähig.

✅ **HT4 a** (Patrik S146): **Die Längskraft geht mit sin α ein, die Querkraft mit cos α (Chabassier, Joly &
Chaigne 2013, Gl. 21); α ist eine weitere Kurve über die Tastatur, *Bridge angle* in Grad (linear, 0 … 10,
Fassung 16), vorläufig 2 Grad, bis 31d sie an den Phantomteiltönen von *Salamander* setzt.** Grund: folgt der
einzigen gelesenen Quelle zur Kopplung am Steg; mit 2 Grad rund 29 dB unter gleichem Gewicht — die
Phantomteiltöne bleiben, wo sie hingehören (laut, im Bass), und beherrschen den Klang nicht; eine Kurve nach HA1 a,
der Winkel ändert sich je Lage. Verworfen: b (eine Zahl für das Instrument — in 31d nicht je Lage anzupassen), c
(gleiches Gewicht — gegen die Physik am Steg). Kippt auf b, wenn 31d zeigt, dass ein Winkel für alle Tasten
trifft.

**Gebaut 31b-2 (S146):** In `loop/HammerTon` die Klasse `Laengs` (Gl. 22b, 26–28 nach Bank 2010; je Saite die
Summe des Chors / N; Querteiltöne in den Produkten bis fs/4; Hochpass 20 Hz; gerechnet, bis die Spannung 100 dB
unter ihrer Spitze liegt, höchstens 10 s — so lange Takt für Takt über alle Moden, danach je Mode); Kurve
*Bridge angle* (`bridgeAngle`, vorläufig 2 Grad), Querkraft mit cos α, Längskraft mit sin α. `HammerMessung`
(Kontakt, Spitzenkraft, größte Auslenkung). **Gemessen** (Release-clang): E1 ff Phantomteilton f10 + f11
44 dB unter dem Grundton, mit neunfachem ES 18 dB höher, der Grundton gleich; C3 ff Teilton 8 zum Grundton
−21,2 dB (pp −32,0); Spitzen 0,26 … 0,58; kein Gleichanteil (unter 2 % des Effektivwerts); Auslenkung gegen
Tab. IV 0,4 … 0,6 der Tabelle. Die Messung der Inharmonizität läuft leise (pp), wo die Längskraft (∝ v²) nicht
stört.

✅ **HT5 a** (Patrik S146): **Die Auskunft der Planung wird allgemein: `InstrumentVon` liefert
`InstrumentAuskunft { Saitensatz saiten; bool hammer; }` und ersetzt `SaitenVon` überall;
`saitenAusInstrumenten()` heißt `instrumentAuskunft()`; ohne Auskunft keine Saiten, kein Hammer.** S-A1 a
(Saiten_Konzept) gilt damit für die ganze Auskunft. Grund: bleibt bei einer Auskunft über das Instrument und bei
der Richtung der Abhängigkeiten (Loop hängt nicht vom Instrument ab); was die Planung später braucht (die Pedale
aus *Keyboard*, 31f), kommt in dieselbe Struktur. Verworfen: b (zweite Auskunft `HammerVon` daneben — zwei
Funktionen, die immer zusammen gebraucht werden, jede neue Größe eine weitere), c (die Planung liest das
Instrument selbst — gegen S-A1 a). Danach ein voller Lauf über Loop und Composer (Composer, Paket und Freeze sind
mitbetroffen).

**Gebaut 31b-3 (S146):** `loop/Auskunft` (`InstrumentAuskunft`, `InstrumentVon`; aus einem `Saitensatz` ohne
Hammer bildbar), umbenannt in 43 Dateien; `instrumentAuskunft()` und `auskunftVon(instrument)` (mit `hatHammer`).
`tonImClip(…, hammer)` und `stufeDerNote` setzen die Anschlagstufe; `klangNamen` und `plane` nennen an einem
Instrument mit Hammer und ohne Saiten dieselben Töne mit `:a<stufe>`, der Schlag trägt den Rest
`anschlagRest(laut, stufe)` statt `laut`; ohne Hammer alles wie bisher (auch die Namen). Ein Instrument mit
Saiten und Hammer zugleich plant über die Saiten, ohne Stufe (gibt es heute nicht). Das Anspielen im Editor und
Creator rechnet weiter mit Stufe 0. **Rechenzeit** (Release-clang, zweimal gemessen, übereinstimmend auf wenige
Prozent): ein Ton 0,5 s gehalten 1,5 ms (C8) … 9 ms (C4) … 30 ms (A0); 4 s gehalten 46 ms (C4) … 110 ms (A0) —
2,3 % der Echtzeit im Bass, 1,1 % in der Mitte, unter 0,3 % im Diskant; je Taste und Stufe einmal gerechnet,
danach aus dem Tonspeicher.

✅ **HT6 a** (Patrik S146): **Die Anschlagstärke bleibt in Stufen (HA3 a, HK2 a); den Schritt legt 31d am Klang
fest — so groß, dass man den Unterschied der Klangfarbe zwischen zwei Stufen nicht hört.** Der Kipppunkt aus
HA3 a („wenn 31b zeigt, dass ein Ton nur Millisekunden braucht“) ist nur halb erreicht: kurze Töne 1,5 … 30 ms,
lange im Bass bis 110 ms; mit Pedal wird je Vorkommen gerechnet, genaue Stärke machte fast jede Note zu einem
eigenen Ton. Grund: mit dem Rest-Pegel stimmt die Lautstärke genau, offen ist nur die Klangfarbe innerhalb einer
Stufe. Verworfen: b (genau auf drei Stellen — Hunderte eigene Töne in den Beispiel-Loops, ohne hörbaren Gewinn).
Kippt auf b, wenn in 31d zwei benachbarte Stufen im Hörvergleich unterscheidbar sind und ein kleinerer Schritt
nichts mehr spart.

**Befund vor 31c (S146, abgerufen 2026-10-23):**
- **Keine frei verfügbare gemessene Antwort eines Resonanzbodens gefunden.** AudioLabs Erlangen (Seite
  „2023-piano-synth-lbd“): „we measured impulse responses of the soundboard of a Yamaha C3 grand piano“ — ohne
  Download, ohne Lizenz. Bank 2010 bemisst seinen Boden „from measured transfer functions“ (Faltung mit
  1000–2000 Abtastwerten für die Tonalität, „ten thousands of taps“ für das Klopfen hoher Töne, S. 9).
- **Synthese nach Boutillon & Ege**, „Vibroacoustics of the piano soundboard: Reduced models, mobility synthesis,
  and acoustical radiation regime“, *J. Sound Vib.* 332 (2013) 4261–4279 (arXiv 1305.3057, nur die Zusammenfassung
  gelesen): die Mobilität aus Holzwerten, Geometrie, Rand und Rippenabstand — unten gekoppelte orthotrope
  Platten, oben Wellenleiter zwischen den Rippen. Dazu Ege, Boutillon & Rébillat, *J. Sound Vib.* 332 (2013)
  (arXiv 1212.2323): Moden bis 3 kHz mit Dämpfung, gemessen an einem Klavier (Upright). Liefert eine Mobilität,
  keinen abgestrahlten Schall.
- ***Salamander*** (Yamaha C5, CC BY 3.0, S141) trägt den Boden in jeder Aufnahme: Aufnahme durch gerechnete
  Kraft am Steg gibt an den Teiltönen aller Tasten die Übertragung Steg → Mikrofon (mit Raum und Mikrofon).

~~HT7 a~~ **abgelöst durch HT7 b (S147, PL31c a, siehe unten nach BM1 a).** ✅ **HT7 a — vorläufig** (Patrik S146): **Die Antwort des Bodens wird aus *Salamander* geschätzt** (Aufnahme durch
gerechnete Kraft am Steg an den Teiltönen aller Tasten und Stärken, geglättet, minimalphasig, als Faltung nach
Bank 2010 VII-A); Raum und Mikrofon gehören dann zum Boden; Reihenfolge Werkzeug der Messung → Boden → übrige
Messung (31d); Download der Sammlung mit Freigabe, die Antwort mit Nennung CC BY 3.0. **Vorläufig, weil Patrik
für b ein sauberes Modell will:** bis zu acht Agenten durchsuchen das Netz nach Daten und Verfahren für einen
physikalischen Boden mit Abstrahlung — möglichst auf einer **allgemeinen Grundlage, auf der auch Körper anderer
Instrumente aufbauen** (Gitarre, Violine, Trommel). Kippt auf b, wenn die Recherche die Werte eines Flügelbodens
und ein Modell der Abstrahlung liefert.

**Wunsch Patrik (S146): eine Klasse „Deluxe“ von Instrumenten** — Instrumente mit aufwendigen Modellen, die mehr
Rechenzeit brauchen, im Fenster *Instruments* eigens markiert (etwa ✨ oder 💫). Offen: was ein Instrument zu
„Deluxe“ macht (eine Eigenschaft der Datei, abgeleitet aus Schichten und Verfahren, oder gemessen), wie die
Marke aussieht, ob Freeze und Töne vorab für sie anders arbeiten. Entscheide dazu nach der Recherche.

**Befund Recherche Boden und Körper (S146; sieben Agenten, Berichte von Claude gelesen; „nachgelesen“ = von
Claude im Volltext geprüft, sonst nur Bericht):**
- **Offene Messdaten:** kein frei lizenzierter Datensatz eines Flügelbodens (Impulsantwort oder Mobilität).
  Für andere Körper frei: Mores, Archiv von 65 Gitarren, Mobilität am Steg mit Hammer und zwei Richtungen,
  Zenodo 4604576, Lizenz `cc-by-4.0` (nur Bericht); CNSM, Violinen mit Stegadmittanz, Zenodo 18696785,
  `cc-by-4.0` (nur Bericht, Inhalt ungeprüft). Plath 2019 (zwei Konzertflügel) hat Rohdaten bei Zenodo 3271370,
  CC BY 4.0 — Kraft und Beschleunigung, keine Mobilität (nur Bericht).
- **Werte für einen physikalischen Flügelboden:** Steinway D Platte 9 mm in der Mitte, 6 mm am Rand (RT-0425
  §2.1 nachgelesen; Boutillon, Ege & Paulello 2012); Rippenabstand rund 12,7 cm (nur Bericht); Holz für Fichte und
  Buche (RT-0425 §2.1 nachgelesen), dazu am Fazioli F278 abgeglichene Werte (Corradi et al. 2017, nur Bericht);
  Masse des fertigen Bodens eines Konzertflügels 13,70 kg (Plath 2019, Tab. 6.2, nachgelesen); Modendichte unter
  1,1 kHz „about 0.06 modes Hz-1“, Verlustfaktor „1% to 3% (mean of 2.3% …)“ (Ege, Boutillon & Rébillat 2013,
  an einem Klavier, nachgelesen); äquivalente Steifigkeit eines Stutzflügels ⟨D_iso⟩ = 68 m⁴s⁻² (Boutillon & Ege
  2013, Anh. B, nachgelesen); die ersten Moden eines eingebauten Flügelbodens ab 43,1 Hz (Corradi et al. 2017,
  Tab. 2, nachgelesen). **Fehlt für jeden Konzertflügel:** Umriss, Zahl und Querschnitt der Rippen, Lage der
  Stege — nur als Netzdatei zu RT-0425 (Dropbox; S147 geprüft: nicht mehr öffentlich zu haben, siehe
  `docs/modellierung/hammer-saite/recherche/2026-10-23_S147_rt0425_netzdatei.md`); die Punkte, an denen jede Saite den
  Steg trifft, stehen in RT-0425 Anhang A (x0, y0).
- **Moden rechnen:** Chabassier, Joly & Chaigne 2013: „2 400 modes are needed to model the soundboard vibrations
  up to 10 kHz“, einmal vorab (nachgelesen); MAESSTRO (Elie, Cotté & Boutillon 2022, CC BY 4.0; Code CeCILL-C):
  Basis bis 5 kHz „about 2 h“ auf vier Kernen (nachgelesen), semi-analytisch nach Trévisan rund 700 s (nur
  Bericht). Über 1,1 kHz statistisch: Modenabstände, Dämpfungen und charakteristische Mobilität aus wenigen Größen
  (Boutillon & Ege 2013, Gl. 23–27, nur Bericht). Pianoteq rechnet den Boden laut Patent US 7 915 515 vorab mit
  finiten Elementen (nur Bericht).
- **Abstrahlung:** je Mode ein Gewicht zum Hörort aus dem Rayleigh-Integral (Platte in Schallwand) — offline
  gerechnet, zur Laufzeit dieselbe Form wie Banks parallele Filter (Pole = Moden des Bodens). Grenze unter rund
  200–300 Hz (die Schallwand stimmt nicht, Ober- und Unterseite schließen kurz); ein Modell für den offenen Deckel
  ist nicht veröffentlicht (nur Bericht). Offline ist die Faltung billiger als Hunderte parallele Filter.
- **Allgemeine Grundlage für Körper:** kein Standard; die Arbeiten rechnen alle mit einer Summe gedämpfter Moden
  mit zwei Ausgängen — die Nachgiebigkeit am Steg (wirkt auf die Saite) und die Abstrahlung, bei Ducceschi, Russo
  & Webb (DAFx26, 65 Gitarren, CC BY 4.0) als zwei Polsätze, bei Wright 1996 je Mode Frequenz, Masse,
  Monopolfläche, Q (nur Bericht).

✅ **KB1 a** (Patrik S146), **mit Zusatz: was sich je Instrument unterscheidet, kommt als Erweiterung auf die
Basis.** **Ein Körper ist eine Liste von Moden — je Mode Frequenz und Dämpfung, je Einkoppelpunkt eine modale
Masse (der Wert ihrer Form dort), je Ausgang ein Abstrahlgewicht; oberhalb einer Grenzfrequenz ein statistischer
Teil (Modendichte, mittlere Mobilität, Verteilung der Dämpfung), mit festem Samen zu Moden ausgewürfelt.** Offline
zu einer Antwort je Einkoppelpunkt und Ausgang gerechnet, per Faltung angewandt; die Rückwirkung auf die Saite aus
denselben Moden, sobald sie gebraucht wird. **Erweiterungen** tragen, was ein Instrument besonders macht (Flügel:
viele Stegpunkte, Deckel; Gitarre: Luftraum, zwei Richtungen am Steg; Trommel: Luftlast der Membran) und woher die
Moden kommen (gerechnet, gemessen, angepasst) — je Instrument eine eigene Frage. Grund: die Form, auf die alle
gelesenen Arbeiten hinauslaufen; jede Quelle lässt sich darin ausdrücken; Rückwirkung und Abstrahlung aus einer
Quelle. Verworfen: b (nur Impulsantworten — keine Rückwirkung, nichts zu verstellen), c (kein gemeinsamer Körper —
gegen den Wunsch). Kippt auf b neben a, wenn für die meisten Instrumente nur Impulsantworten zu haben sind.

**Befund S147 (selbst geprüft):** Die Netzdatei zu RT-0425 (Umriss, Rippen, Stege des Steinway D) ist nicht mehr
öffentlich zu haben — der alte öffentliche Dropbox-Ordner antwortet mit 404, das Webarchiv hat keine Aufnahme mit
Inhalt, Montjoie bietet keine Piano-Netze an; nicht versucht: eine Anfrage an die Autoren. MAESSTRO (Elie, Cotté &
Boutillon 2022, nachgelesen) rechnet „a simplified version of the Steinway D“ mit Zahlen in Tab. 1–2 (Platte 8 mm
Sitka, Faser 135°; Stege Buche 60 × 32 und 20 × 32 mm; Rippen 17–27 × 21–25 mm; 23 006 Elemente, Moden bis 5 kHz
„about 2 h“); Umriss, Stege und Rippen nur als Bild (Abb. 4), keine Geometriedatei im Repo. Zu hören (§6, nachgelesen):
„The impact of the global stiffness is also audible in the resulting sound pressure level“; die Änderung des
Rippenmusters zeigt sich dagegen kaum in ihren Größen (Pegel, Abklingzeit), die von Note zu Note stark streuen.
Bericht: `docs/modellierung/hammer-saite/recherche/2026-10-23_S147_rt0425_netzdatei.md`.

✅ **BM1 a** (Patrik S147), **mit Zusatz: der Boden ist eine Konstruktion in einer Datei — bei Bedarf entstehen
eigene Flügel mit hörbar anderem Klang; kein Editor vorerst, die Möglichkeit bleibt offen.** **Die Moden des
Flügelbodens werden gerechnet:** eigenes Werkzeug, offline, in C++, Reissner-Mindlin-Platte mit finiten Elementen
wie bei Chabassier 2013 und MAESSTRO (orthotrop; Rippen und Stege als Dicke; Rand eingespannt). Geometrie: der
vereinfachte Steinway D aus MAESSTRO (Tab. 1; Umriss, Stege, Rippen aus Abb. 4 abgezeichnet, gegen RT-0425 Abb. 2
geprüft) — **geändert durch BM2 a:** aus RT-0425 Abb. 2, MAESSTRO nur als Prüffall; Stegpunkte aus RT-0425 Anhang A; Holz aus MAESSTRO Tab. 2 und RT-0425 §2.1. Aus den Modenformen die Masse je
Stegpunkt und das Abstrahlgewicht je Mode (Rayleigh); oberhalb einer Grenzfrequenz der statistische Teil nach
Boutillon & Ege 2013 (Gl. 23–27, nachgelesen) — die Grenze legt ein eigener Schritt fest. Prüfstein: die ersten Moden
gemessener Flügel (Corradi 2017 Tab. 2 ab 43,1 Hz, Conklin) und die mittlere Mobilität (Plath 2019, Giordano). **Zum
Zusatz:** Das Werkzeug liest die Konstruktion (Umriss, Platte, Rippen, Stege, Holz, Stegpunkte) aus einer Datei, nicht
aus Code; ein Editor kommt erst auf Wunsch, die Datei ist so gefasst, dass er sie ohne Umbau schreiben kann.
Varianten (etwa dickere Platte, weniger Rippen, kleinerer Boden) erzeugt Claude bei Bedarf als Dateien; ob ein
Unterschied auch für Laien hörbar ist, wird an Hörproben geprüft, nicht vorausgesetzt — nach MAESSTRO §6 tragen
Änderungen der ganzen Steifigkeit (Dicke, Holz, Größe) hörbar, ein anderes Rippenmuster allein kaum. Grund: nur das
Rechnen liefert alles, was KB1 a je Mode verlangt, aus einer Quelle; die Formen tragen den Unterschied zwischen Bass
und Diskant am Steg. Verworfen: b (semi-analytisch nach Trévisan — verliert laut MAESSTRO „fine geometric details“,
die Rechenzeit fällt nur einmal offline an), c (gemessene erste Moden plus Statistik — keine Formen, Masse und
Abstrahlung nur als Mittel; gemessen nur am Fazioli). Kippt auf c, wenn die abgezeichnete Geometrie die ersten Moden
nicht in den Bereich gemessener Flügel bringt (erste Mode rund 43–60 Hz, Abstände um 20 Hz); die Datei nach KB1 a
nimmt beide Herkünfte auf. HT7 a bleibt davon unberührt.

**Befund S147 (selbst angesehen):** RT-0425 Abb. 2 (c) und (d) zeigt den Boden des Steinway D mit Achsen in Metern
(0–1,4 × 0–1,9 m) — im Bezugssystem der Stegpunkte aus Anhang A —, Umriss, Bass- und Diskantsteg, rund 17 Rippen,
Dicke der Platte und Höhe der Rippen als Farbverlauf (Spannen in §2.1). Rasterbild, rund 2–3 mm je Pixel. MAESSTRO
Abb. 4 ist ein Bildschirmfoto ohne Maßstab der vereinfachten Fassung.

✅ **PL31c a** (Patrik S147): **31c in acht Teilschritten.** **31c-1** Konstruktion als Datei (Umriss, Platte mit
Dickenverlauf, Rippen, Stege, Holz, Stegpunkte; ohne Qt-Anwendung) und der Steinway D als erste Datei — Prüfung: die
Stegpunkte aus Anhang A liegen auf der Steglinie, Fläche und Masse plausibel (Plath 13,70 kg). **31c-2** Netz
(Dreiecke im Umriss, Rippen und Stege als Kanten, Elementgröße nach MAESSTRO Gl. 1). **31c-3** Platte
(Reissner-Mindlin, orthotrop, Dicke je Punkt, Elemente höherer Ordnung; Prüfung gegen die frei aufliegende
orthotrope Rechteckplatte und auf Konvergenz). **31c-4** Eigenlöser (Moden in Frequenzfenstern, Rechenzeit gemessen).
**31c-5** Körperdatei nach KB1 a (je Mode Frequenz, Dämpfung nach RT-0425 Gl. 1, Masse je Stegpunkt, Abstrahlgewicht
nach Rayleigh zu einem Hörort; darüber der statistische Teil nach Boutillon & Ege). **31c-6** Prüfstein und
Grenzfrequenz (erste Moden gegen Corradi Tab. 2 und Conklin, mittlere Mobilität gegen Plath und Giordano; Kipppunkt
BM1 a → c). **31c-7** Klang (die Kraft am Steg mit der Antwort des Bodens je Taste gefaltet; `soundboard` am
Instrument verweist auf den Körper, `.lvinst` Fassung 17; Klangfassung, `loop_werte`, Hörproben `LUMIVIZ_31C_WAV`).
**31c-8** Varianten (Zusatz zu BM1 a; zwei bis drei Konstruktionen als Hörproben). Fragen einzeln vor ihrem Schritt:
BM2 Vorlage der Geometrie, BM3 Format und Ort der Dateien, BM4 Netzgenerator, BM5 Ort des Codes und Eigenlöser, BM6
Hörort, BM7 Grenzfrequenz. **Dazu:** HT7 kippt auf b (unten); die **Rückwirkung des Bodens auf die Saite kommt in
31e** mit dem Mitschwingen (dieselben Moden am Steg; die Masse je Stegpunkt liegt ab 31c-5 in der Körperdatei).
Grund: es entsteht nur, was bleibt; ein Boden aus *Salamander* trüge Raum und Mikrofon und würde danach ersetzt.
Verworfen: b (zuerst *Salamander* — Wegwerfarbeit, zweimal neue Klangfassung), c (Rückwirkung schon in 31c —
dieselbe Kopplung zweimal angefasst). Kippt auf c, wenn 31d die Rückwirkung braucht (etwa Abklingzeiten im Bass).

✅ **HT7 b** (Patrik S147, mit PL31c a; löst das vorläufige HT7 a ab): **Der Boden kommt aus dem Modell (BM1 a);
*Salamander* bleibt Vergleich und Messung für 31d.** Grund: der Kipppunkt von HT7 a ist erreicht — Werte eines
Flügelbodens (RT-0425, MAESSTRO) und ein Modell der Abstrahlung (Rayleigh je Mode) liegen vor.

✅ **BM2 a** (Patrik S147; ändert die Geometrie in BM1 a): **Der Steinway D kommt aus RT-0425 Abb. 2 (c, d), per
Farbe ausgewertet** — Umriss, Mittellinien von Stegen und Rippen in Metern, im Bezugssystem von Anhang A („(0, 0) is
the bottom left corner of the soundboard“, §2.3, nachgelesen); Dicke der Platte und Höhe von Rippen und Stegen aus
dem Farbverlauf (Skala „AU“) linear auf die Spannen aus §2.1 gelegt; Breiten (Rippen 21–25 mm, Stege 32 mm) aus
MAESSTRO Tab. 1; Holz aus RT-0425 §2.1. **Dazu der vereinfachte Steinway D aus MAESSTRO als zweite Datei, nur als
Prüffall:** in 31c-6 gegen MAESSTRO Abb. 5 (Modendichte) und Abb. 6 (mittlere Mobilität an den Stegpunkten), in
31c-8 mit den Varianten MP1–MP3 gegen deren Richtungen. Grund: der Boden zu genau dem Flügel, zu dem Saiten und
Hammer gehören (HT1 a, HT2 a), ohne Umrechnung der Stegpunkte; der Prüffall trennt in 31c-6 Fehler des Codes von
Fehlern der Geometrie. Verworfen: b (nur MAESSTRO — vereinfacht, „a generic piano“, anderer Ursprung, Platte gegen
§2.1 überall 8 mm), c (ohne Prüffall). Kippt für die Dicken auf die Tabellenwerte aus b, wenn die Farbauswertung
unzuverlässig ist (JPEG, Überdeckung von Rippen und Steg); Umriss und Lage bleiben aus Abb. 2. Prüfung in 31c-1: die
Stegpunkte aus Anhang A liegen auf der ausgewerteten Steglinie.

**Wunsch Patrik (S147): ein Flügel aus Glas als Variante in 31c-8** („Werte für Glas mit Quelle“), gemeint „so etwas
wie“ der Schimmel Konzert K 213 Glas. **Befund (selbst gelesen):** Schimmels Prospekt „Glasflügel Schimmel Konzert K
213 G“ (schimmel-pianos.de, PDF SCH_2006-03): „Acrylglasflügel“, „bereits 1951 den neuen widerstandsfähigen Werkstoff
Acryl“, „Klang, den der Acrylglaskorpus noch unterstützt“ (Werbung, kein Beleg zum Klang); Händlerseite Boullard
(K213 Glass): „Acrylgehäuse“, „Sound board : Bayerische Fichte - Deutschland“. **Das Gehäuse ist aus Acrylglas, der
Resonanzboden aus Fichte.** In unserem Modell ist der Boden am Rand eingespannt (BM1 a, wie MAESSTRO; Corradi 2017:
Rast „in erster Näherung starr“), ein Deckel kommt nicht vor (Befund S146: kein veröffentlichtes Modell) — ein
Gehäuse aus Acryl änderte dort nichts.

✅ **GB1 a** (Patrik S147), **mit Zusatz: jedes Teil trägt sein eigenes Material, Boden und Gehäuse lassen sich frei
kombinieren.** **In 31c-8 kommt ein Boden aus Glas dazu, als gedachtes Instrument** — Glaswerte (Dichte,
Steifigkeit, Verlustfaktor) mit Quelle, Dicke einmal wie der Holzboden und einmal auf gleiche Masse gerechnet. **Der
Schimmel-Glasflügel (Acrylgehäuse, Fichtenboden) wird als Wunsch notiert** und kommt, wenn Rand und Deckel ins Modell
kommen (Erweiterung „Flügel: Deckel“ nach KB1 a; wann, ist eine eigene Frage). **Zum Zusatz:** Die Konstruktion
(BM3) führt das Material je Teil — Platte, Rippen, Stege, später Rand/Gehäuse und Deckel —, Holz mit Faserrichtung
oder ein Werkstoff ohne (Glas, Acryl). Sobald Rand und Deckel gerechnet werden, ist jede Kombination eine Datei:
Fichtenboden mit Acrylgehäuse (Schimmel), Glasboden mit Holzgehäuse, Glas mit Acryl. Grund: das Modell verspricht nur,
was es rechnet — ein Glasboden ändert Masse, Steifigkeit und Dämpfung des Bodens, ein Gehäuse wirkt erst über Rand und
Deckel. Verworfen: b (Rand und Deckel schon in 31c — für den Deckel keine Vorlage, 31c würde um einen ungeklärten
Teil länger), c (kein Glasboden). Kippt auf b, wenn der Schimmel als Instrument wichtiger wird als das Experiment —
dann zuerst eine Recherche zum Deckel.

✅ **BM3 a, BM3-1 a, BM3-2 a** (Patrik S147), **mit Zusatz: passende Icons gehören dazu.** **Zwei Dateien, beide
JSON, alles ausgeschrieben:** der **Bauplan `.lvinstbodyplan`** (`format: lumiviz-instrument-body-plan`, Fassung 1;
Quelle, von Hand bzw. von Claude geschrieben) — ein Wörterbuch der Materialien (orthotrop mit Faserwinkel oder
isotrop; Dichte, Moduln, Querzahlen, Verlustfaktor), Umriss als Polygon in Metern, Platte mit Dickenverlauf, Rippen
und Stege als Mittellinien mit Höhe und Breite, Stegpunkte je Taste, je Teil ein Material (GB1 a); Rand und Deckel
später als weitere Teile. Der **Körper `.lvinstbody`** (`format: lumiviz-instrument-body`, Fassung 1; vom Werkzeug
geschrieben) — die Moden nach KB1 a und der statistische Teil, dazu **der Abdruck des Bauplans und die Fassung des
Werkzeugs**; ein Test meldet einen Körper, der nicht mehr zum Bauplan passt (Muster des Wächters der Klangfassung).
**Ort:** `asset/loop/instrument_bodies/` neben `instruments/`, `samples/`, `kits/`, mit einer `README.md`
(Resonanzkörper von Instrumenten, je Körper Bauplan und gerechneter Körper); beide Dateien im Repo, der Körper
committet, damit das Programm nie rechnet; Eintrag in `asset/Asset.cmake`. Die `.lvinst` verweist mit
`"soundboard": {"body": "lib:steinway_d", …}`. Der MAESSTRO-Prüffall liegt unter den Testdaten, nicht mitgeliefert.
Eigene Körper (Benutzerordner, Datensicherung, Publish) erst mit einem Editor. **Icons (Zusatz):** je Endung ein
Icon nach dem Muster der Marke mit Zeichen im Kreis (I1 a, S120), aus `lumiviz.ico` erzeugt
(`build_lvfx2_icons.py`); die Zeichen wählt Patrik an einem Vorschaublatt; dazu `register_file_icons.ps1`
(`LumiViz.InstrumentBodyPlan`, `LumiViz.InstrumentBody`), die Tabellen in `asset/img/logo/icons/README.md`, die
Vorschaubilder (`build_previews.py`) und `dateiArtIcon`; in 31c-1. Grund: Quelle und Ergebnis getrennt wie bei Freeze
(FZ2 a); ein Körper ist etwas, das Instrumente benutzen und teilen, wie ein Sample; die Endung ist das Einzige, was
eine Datei überall mitnimmt. Verworfen: BM3 b (eine Datei — Moden veralten still), BM3 c (Körper binär — gegen alle
übrigen Formate; kippt dahin, wenn ein Körper in 31c-5 deutlich über rund 10 MB wächst), BM3-1 b (Unterordner von
`instruments/` — dort liegt nur, was im Fenster *Instruments* erscheint), BM3-2 b (kurze Endungen — außerhalb des
Ordners nicht selbsterklärend), BM3-2 c (`bodies/`, `.lvbody` — mehrdeutig).

✅ **BM5 a** (Patrik S147): **Der Rechenkern des Bodens wird eine eigene Bibliothek `projects/libs/InstrumentBody/`**
— nur Header, reines C++ ohne Qt (Bauplan als Daten, Netz, Platte, Eigenlöser, Abstrahlung, statistischer Teil).
Lesen und Schreiben von `.lvinstbodyplan` und `.lvinstbody` mit Qt-JSON einmal in der App unter `loop/`. **Ein
Werkzeug `projects/exec/InstrumentBodyTool/`** (Konsole) liest einen Bauplan, rechnet mit der Bibliothek und
schreibt den Körper; es kompiliert den Leser aus `loop/` direkt mit (Muster MilkdropStandalone, Executables vor
Apps). Die App liest nur Körper und rechnet nie; ein späterer Editor bindet dieselbe Bibliothek ein. Tests in
`LumiViz.UnitTests`: die Bibliothek ohne Qt-Anwendung (analytische Platte, Konvergenz), der Leser, der Wächter des
Abdrucks. Grund: die Richtung der Abhängigkeiten — der Kern braucht weder Qt noch die App. Verworfen: b (Fall der
Test-Exe hinter einer Umgebungsvariable — ein Test, der Stunden rechnet und eine mitgelieferte Datei schreibt, ist
kein Test), c (alles in der App mit verstecktem Aufruf). Kippt auf b, wenn ein Körper nur Sekunden bis wenige Minuten
braucht (nach 31c-4); dann bleibt das Werkzeug, sein Aufruf darf auch aus einem Test kommen.

✅ **BM5-1 a** (Patrik S147), **mit Zusatz: austauschbar eingebunden, Tests für einen Vergleich alt gegen neu, eigene
Dokumentation.** **Eigen 3.4.1 und Spectra 1.2.0 als Externals** (beide MPL-2.0, laut COPYING.README bzw. GitHub
abgefragt 2026-10-23), unverändert, mit fester Fassung, nur für `InstrumentBody`, `InstrumentBodyTool` und die Tests,
nicht für die App; Lizenzen in `THIRD_PARTY_NOTICES.md`; Eigen 5.0 erst nach einer Probe mit Spectra; ob die Quellen
fest im Repo liegen oder per Tag geholt werden, klärt eine Probe unter beiden Compilern. **Zum Zusatz:** (1) **Eine
Schnittstelle vor dem Löser** — die übrige Bibliothek (Netz, Platte, Abstrahlung) kennt nur sie: dünn besetzte
Matrizen K und M, ein Frequenzfenster, zurück Frequenzen und Formen; Eigen und Spectra stehen nur in der einen
Umsetzung dahinter, kein Typ von Eigen oder Spectra geht über die Schnittstelle. Ein eigener Löser wird eine zweite
Umsetzung derselben Schnittstelle, gewählt im Werkzeug per Schalter. (2) **Tests gegen die Schnittstelle**, nicht
gegen die Umsetzung: dieselben Fälle (geschlossene Lösungen — Rechteckplatte frei aufliegend, orthotrop; kleine
Matrizen mit bekannten Eigenwerten; Vollständigkeit im Fenster, keine Mode doppelt oder ausgelassen; Orthogonalität
bezüglich M) laufen für jede vorhandene Umsetzung; dazu **abgelegte Bezugswerte** (Frequenzen und Formen an
Stützpunkten für die Prüfplatten und den Steinway D) und ein **Vergleichsfall**, der zwei Umsetzungen gegeneinander
rechnet, sobald es zwei gibt (Frequenzen auf eine Toleranz, Formen über den MAC-Wert), mit Rechenzeit beider. (3)
**Eigene Dokumentation** des Lösers: Modul-Doku im CppModuleDoc-Format neben den Headern von `InstrumentBody`
(Projektregel) — Schnittstelle, die Umsetzung mit Eigen und Spectra (Fassungen, Bezug, Lizenz), wie eine eigene
Umsetzung dazukommt, die Tests und Bezugswerte und wie der Vergleich alt gegen neu läuft. Grund: Richtigkeit —
erprobte Numerik für die heikelsten Stellen; die Schnittstelle hält den Weg zu einem eigenen Löser offen, ohne die
übrige Bibliothek anzufassen. Verworfen: b (eigener Löser jetzt — die heikelste Numerik ungeprüft), c (nur Eigen,
Lanczos selbst — gerade das Verfahren ist die Fehlerquelle). Kippt auf c oder b, wenn Eigen oder Spectra unter einem
der beiden Compiler nicht sauber bauen oder die Lizenzprüfung im Bauschritt anderes zeigt.

**Gebaut 31c-1 (S147, ohne Icons):** Bibliothek `projects/libs/InstrumentBody` (`InstrumentBody/Bauplan.hpp`:
Materialien orthotrop/isotrop, Platte mit Dickenraster, Leisten, Stegpunkte, Fläche, Masse, `pruefe`; Modul-Doku
`InstrumentBody.md`), in `Solution.json` als INTERFACE-Bibliothek für Core und Tests; `loop/KoerperBauplan` (Datei
`.lvinstbodyplan` lesen/schreiben, jedes Feld, NaN als null; `mitgelieferterKoerperOrdner`); der Bauplan
`asset/loop/instrument_bodies/steinway_d.lvinstbodyplan` aus `make_steinway_d.py <RT-0425.pdf>`, README im Ordner.
Tests `test_LoopKoerperBauplan` (6 Fälle; der letzte liest den Steinway D vom Arbeitsordner aus). **Befunde beim
Auswerten (selbst gelesen und gemessen):**
- Die **Farbskalen von Abb. 2 tragen Zahlen** (c: 6–10·10⁻³ m; d: 0,01–0,09 m), nicht „AU" wie die
  Bildunterschrift — die Dicken sind direkt abgelesen statt, wie in BM2 a angenommen, linear auf die Spannen von
  §2.1 gelegt; §2.1 dient als Prüfung. Platte 6,0 … 9,98 mm (Median 8,6; §2.1 sagt 6–9, „following the proportions
  seen in the figures"), Rippen gesamt 10–35 mm (§2.1: 9–35), Steg gesamt 30–70 mm (§2.1: 29–69).
- **(c) und (d) schneiden den Boden an den Achsen ab** (B7 liegt bei y = −0,03 m). Der Umriss kommt deshalb aus
  Abb. 2 (a), gelegt auf die Stegpunkte und den Rand von (c): Stegpunkte zum Steg von (a) Median 0,0 mm, Rand (c)
  zu Rand (a) Median 2,0 mm, größter 5,4 mm; die linke untere Ecke liegt 8 mm neben dem Ursprung von §2.3. Fläche
  2,05 m².
- Achsen beider Bilder 290 px/m; **alle 84 Stegpunkte aus Anhang A liegen auf dem gezeichneten Steg** (Median
  4,5 mm, größter 14 mm von der Mittellinie; Steg 32 mm breit) — die Eichung trägt.
- Holz: Chabassier, Joly & Chaigne 2013 Tab. I „Spruce for the table and the ribs" = die zweite Fichtenzeile von
  RT-0425 §2.1 (380 kg/m³, 11,0 GPa …), Buche für den Steg (nachgelesen). Verlustfaktor 0 (keine Angabe) bis 31c-5.
- 16 Rippen unter 35,5°; **Faser quer dazu, 125,5°** (Annahme; MAESSTRO setzt 135°).
- **A0, A♯0, B0 und C8 fehlen in RT-0425** (Anhang A und B: C1 … B7, gleiche Werte): entlang des Stegs
  verlängert, Quelle „extended along the bridge".
- **Masse 11,5 kg** (Platte 6,5, Rippen 1,5, Steg 3,5); Plath 2019 Tab. 6.2 am fertigen Konzertflügel 13,70 kg
  (Rippen 4,17, Steg 3,15). Die Rippen sind in RT-0425 Abb. 2 kürzer gezeichnet (0,3 … 0,8 m) als im Flügel —
  dem Bild gefolgt.
- `asset/Asset.cmake` nimmt die Körper erst auf, wenn es den ersten gibt (31c-5); der Bauplan bleibt im Repo.

✅ **IC1 a** (Patrik S147, am Vorschaublatt): **Bauplan `.lvinstbodyplan` = P1** (der Umriss des Bodens dunkel mit
hellem Rand, Rippen in Cyan, der Steg in Bernstein), **Körper `.lvinstbody` = K1** (derselbe Umriss mit einer Mode,
Bäuche in Cyan und Magenta) — beide als Marke mit Zeichen im Kreis (I1 a), der Umriss aus
`steinway_d.lvinstbodyplan`. Grund: sie zeigen, was in der Datei steht (Rippen und Steg gegen Moden), gleiche Form,
verschiedene Fläche — bei 32 px auseinanderzuhalten. Verworfen: b (P2 mit Maßlinie, K2 mit Schallwellen — dünne
Linien verschwinden klein, Wellen meinen die Abstrahlung), c (P3 Blaupause, K3 Mode mit Steg — K3 zu nah am Bauplan).
Kippt auf K2 neben P1, wenn K1 bei 32 px wie ein Schachbrett wirkt.

✅ **BM4 a** (Patrik S147), **mit Zusatz: ein eigener Netzgenerator vielleicht später, als Herausforderung.**
**CDT 2.0.1** (artem-ogre/CDT, MPL-2.0, als reine Header; Fassung und Lizenz über GitHub abgefragt 2026-10-23;
Ruppert-Verfeinerung seit 2.0.0 vom 20.09.2026) **als External, unverändert, hinter einer eigenen Schnittstelle in
`InstrumentBody`** (wie BM5-1 a): hinein Umriss, Zwangslinien, Elementgröße, heraus Knoten und Dreiecke, kein Typ von
CDT nach außen. Zwangslinien: die Ränder jeder Rippe und des Stegs (Mittellinie ± halbe Breite) — die Dickensprünge
liegen auf Kanten. Verfeinert nach kleinstem Winkel und größter Fläche aus MAESSTRO Gl. 1. Tests gegen die
Schnittstelle: Fläche erhalten, jede Zwangslinie als Kantenfolge, kleinster Winkel und größte Fläche eingehalten,
jedes Dreieck im Umriss, zwei Läufe gleich; Bezugswerte für den Vergleich alt gegen neu; Doku in der Modul-Doku.
**Zum Zusatz:** ein eigener Generator (Delaunay mit Zwangskanten, Ruppert) wird eine zweite Umsetzung derselben
Schnittstelle und mit den Tests und Bezugswerten gegen CDT verglichen — die Schnittstelle hält das offen. Grund:
erprobter Code für die heikelsten Stellen (robuste Prädikate nach Shewchuk), ohne zweites Programm, ohne Qt.
Verworfen: b (Gmsh als externes Programm, GPLv2+ — die Tests hingen an einem installierten Fremdprogramm, ein Editor
in der App hätte es nicht), c (eigener Generator jetzt). Kippt auf b, wenn die Verfeinerung von CDT am Steinway mit
den Zwangslinien scheitert (endet nicht, Winkel nicht eingehalten, Kanten verloren).

**Gebaut 31c-1 Icons (S147):** `zeichen_bauplan` und `zeichen_koerper` in `build_lvfx2_icons.py` (der Umriss aus
`steinway_d.lvinstbodyplan`), `lumiviz_lvinstbodyplan.ico` und `lumiviz_lvinstbody.ico` — die übrigen 17 Icons Byte
für Byte gleich; eingetragen in `register_file_icons.ps1` (`LumiViz.InstrumentBodyPlan`, `LumiViz.InstrumentBody`),
die Tabellen der Icon-README, die Vorschaubilder, Benutzerhandbuch §17 (1.203.0). `dateiArtIcon` findet sie über den
Namen, ohne Code. Test „Loop Koerper 31c-1 Icons". **31c-1 damit fertig;** unter Release-clang 7 / 7, Testing-MSVC
baut.

**Gebaut 31c-2 (S147):** CDT 2.0.1 unter `externals/CDT` (die elf Header und drei Lizenzdateien unverändert aus
`CDT-2.0.1.zip`, 4 350 160 Byte, SHA-256 in der README; Download mit Freigabe Patrik), eigene `Include.cmake`
(INTERFACE-Ziel `CDT`), in `Solution.json` nur für die Unit-Tests; `THIRD_PARTY_NOTICES.md` mit CDT (MPL-2.0) und den
mitgelieferten Teilen (Prädikate von W. C. Lenthe, BSD-3 mit Wortlaut; `portable_nth_element` aus LLVM, Apache-2.0
mit LLVM-Ausnahme). In `InstrumentBody`: `Netz.hpp` (Schnittstelle `Netzgeber`, Prüfgrößen, `netzAuftrag` aus dem
Bauplan: Ränder der Leisten, Kante nach MAESSTRO Gl. 1 an der dünnsten Stelle) und `NetzCdt.hpp` (die einzige Datei mit
CDT); Modul-Doku 0.2.0 §3. Tests `test_LoopKoerperNetz` (4 Fälle gegen die Schnittstelle, für jede Umsetzung aus
`netzgeber()`), Bezugswerte `werte/netz_steinway.txt`. **Gemessen** (Release-clang): Steinway D bis 5 kHz — Kante
13,3 mm, 21 491 Knoten, 42 329 Dreiecke (MAESSTRO: 23 006 Elemente 3. Ordnung für seinen vereinfachten Steinway),
kleinster Winkel 25,0° außer 8 Ecken der Eingabe (Kreuzungen von Rippe und Steg), Fläche genau, jede Zwangslinie als
Kantenfolge, 0,07 s. Der Kipppunkt von BM4 a (CDT scheitert am Steinway) ist nicht eingetreten. Release-clang
`*Loop Koerper*` 11 / 11; ob Testing-MSVC dieselben Zahlen liefert (Bezug auf 2 %), zeigt der Lauf am Stufenende.

✅ **EL1 a** (Patrik S147): **Die Platte bekommt Dreiecke mit Lagrange-Ansätzen frei wählbarer Ordnung k** für w und
die beiden Drehungen, volle Integration, Schubkorrektur 5/6; Vorgabe k = 3 (MAESSTRO); die Ordnung wählt ein
Konvergenztest nach MAESSTRO Gl. 2 an den höchsten Moden (wie Chabassier, Duruflé & Joly 2015 Teil 2: „a Q3
approximation gives … relative error of 1, 3 % … This led us to rp = 4" bis 10 kHz, selbst gelesen) und steht als Zahl
im Körper. Prüfungen: gegen die geschlossene Lösung der frei aufliegenden orthotropen Rechteckplatte nach Mindlin, auch
an einer dünnen Prüfplatte (Locking sichtbar), Konvergenz mit der Ordnung, K und M symmetrisch und positiv. Die Bauart
des Elements liegt hinter einer kleinen Schnittstelle (ein Element gegen Locking kann dazukommen). Grund: der Weg der
beiden Quellen (höhere Ordnung, an der Konvergenz gewählt); unser Verhältnis Dicke zu Kante (rund 0,5–0,7 bei 13,3 mm
Kante) liegt nicht im Bereich starken Lockings (eigene Einschätzung, der Test prüft sie). Verworfen: b (MITC — feste
Ordnung, ohne den Konvergenztest der Quellen), c (2. Ordnung mit reduzierter Integration — unechte Moden landeten still
im Körper). Kippt auf b, wenn k = 3 an der dünnen Prüfplatte mehr als rund 1 % zu hoch liegt.

**Gebaut 31c-3 (S147):** Eigen 3.4.1 unter `externals/Eigen` (der Ordner `Eigen/` und alle `COPYING.*` unverändert aus
`eigen-3.4.1.zip`, 3 721 838 Byte, SHA-256 in der README; Download mit Freigabe Patrik), eigene `Include.cmake` mit
`EIGEN_MPL2_ONLY`, in `Solution.json` nur für die Unit-Tests; `THIRD_PARTY_NOTICES.md` (MPL-2.0; `BFloat16.h` Apache-2.0,
`Half.h` freizügig; die BSD-Teile werden nicht eingebunden). In `InstrumentBody`: `DuenneMatrix.hpp` (oberes Dreieck,
Muster, dann Werte — das Format für den Eigenlöser, BM5-1 a; die Platte kennt Eigen nicht) und `Platte.hpp`
(Reissner-Mindlin, Zonen nach RT-0425 mit Material und Faser je Leiste, `Plattenelement` / `LagrangeDreieck(k)`,
`baueSystem` eingespannt oder gelenkig); Modul-Doku 0.3.0 §4. Tests `test_LoopKoerperPlatte` (7 Fälle; Eigen dicht nur
als Werkzeug im Test). **Gemessen** (Release-clang): gegen Navier (Mindlin, frei aufliegend, orthotrop) mit dem Netz
nach MAESSTRO Gl. 1 trifft k = 3 auf 0,04 % (8 mm), 0,03 % (2 mm, Dicke/Kante 0,084), 0,07 % (1 mm, 0,042); Locking bei
k = 1 (108–871 %) und k = 2 (2,5–10 %), nicht bei k = 3 — **der Kipppunkt von EL1 a ist nicht erreicht**. Steinway D
bis 5 kHz, k = 3: 568 515 freie Größen, 14,7 Mio. Einträge je Matrix, gebaut in 4,7 s, Spitze 407 MB; Masse
11,77 kg (Bauplan 11,48 kg — unter dem Steg rechnet die Zone Buche statt Fichte). Release-clang `*Loop Koerper*` grün
(18 Fälle).

**31c-4 begonnen (S148):** Spectra 1.2.0 unter `externals/Spectra` (der Ordner `include/Spectra` unverändert ohne
`contrib/`, `LICENSE`; `spectra-1.2.0.zip` von GitHub, 381 430 Byte, SHA-256 in der README; Download mit Freigabe
Patrik), eigene `Include.cmake` (hängt an `Eigen`), in `Solution.json` nur für die Unit-Tests; `THIRD_PARTY_NOTICES.md`.

✅ **EG1 a** (Patrik S148): **Der Eigenlöser teilt das Fenster selbst und reicht jede Mode an einen Empfänger
weiter**; zurück kommen nur die Frequenzen. Schnittstelle `Eigenloeser` (`name()`, `loese(EigenAuftrag)`, Liste
`eigenloeser()` wie `netzgeber()`): hinein K und M als `DuenneMatrix`, Fenster `vonHz`/`bisHz`, Toleranz, Empfänger
`jeMode` (aufsteigend; leer = nur Frequenzen) und `weiter` (Fortschritt, Abbruch); heraus Frequenzen, `imFenster`
(Zahl der Eigenwerte im Fenster nach dem Trägheitssatz von Sylvester — Pflicht jeder Umsetzung, damit die
Vollständigkeit auch am Steinway prüfbar ist), Zahl der Faktorisierungen, Sekunden, Fehler. Formen bezüglich M
normiert, nach `PlattenSystem::frei` nummeriert. Grund: Shifts, Ränder der Teilfenster und doppelte Moden dort hängen
am Verfahren; der Löser weiß nicht, was mit einer Form geschieht, der Speicher bleibt bei einer Faktorisierung und
wenigen Formen (alle Formen bis 5 kHz am Steinway wären rund 11 GB). Verworfen: b (alle Moden eines Fensters als
Liste, Teilfenster beim Aufrufer). Kippt, wenn eine Umsetzung die Moden nicht aufsteigend liefern kann.

✅ **EG2 a** (Patrik S148), **mit Zusatz: die Alternativen bleiben im Blick und kommen, sobald sie als nötig erkannt
werden.** **Die erste Umsetzung zerlegt K − σM mit `Eigen::SimplicialLDLT`** (AMD-Ordnung) in einem eigenen Operator
für `SymGEigsShiftSolver` (Spectra lässt eigene Operatoren zu, `SymGEigsShiftSolver.h:66–67`): eine Zerlegung je
Shift; aus `vectorD()` zählt er die negativen Einträge (Eigenwerte unter σ, Sylvester) — zwei Zählungen an den
Grenzen ergeben `imFenster`; jede Mode wird an ihrem Residuum ‖Kφ − λMφ‖ geprüft, darüber meldet der Löser einen
Fehler statt still zu liefern. Grund: K − σM ist symmetrisch; die symmetrische Zerlegung hält etwa halb so viel wie
`SparseLU` (der Operator `SymShiftInvert` von Spectra, `SymShiftInvert.h:166–170`) und liefert die Zählung ohne
zweite Zerlegung. **Die Alternativen** (Zusatz): b — `SparseLU` mit Pivotsuche für die Moden, `SimplicialLDLT` nur zum
Zählen; c — ein Löser mit Superknoten (CHOLMOD, Pardiso) als weiteres External und zweite Umsetzung. **Als nötig
erkannt** gilt b, wenn an Prüfplatten oder Steinway Residuen über der Toleranz bleiben, die ein verschobener Shift
nicht behebt (`SimplicialLDLT` sucht keinen Pivot, `SimplicialCholesky_impl.h:150–152`); c, wenn Speicher oder
Rechenzeit der Zerlegung am Steinway ein Fenster unbrauchbar machen. Dann wird die Alternative mit der Messung
vorgelegt.

**Gebaut 31c-4 (S148):** in `InstrumentBody` die Schnittstelle `Eigenloeser.hpp` (EG1 a; dazu im Ergebnis der größte
Rest und die Zeit der Zerlegungen, für den Vergleich alt gegen neu), die Umsetzung `EigenloeserSpectra.hpp` (EG2 a:
`SimplicialLDLT` in eigenem Operator, Zählung nach Sylvester, Teilfenster [P, σ) zwischen zwei gezählten Shifts,
geliefert erst bei stimmender Zahl; **Deflation** — gefundene Formen werden im Operator bezüglich M herausprojiziert,
sonst sähe Lanczos von einem doppelten Eigenwert nur eine Form; Rest je Mode), `Umsetzungen.hpp` (`netzgeber()` und
`eigenloeser()` an einer Stelle — Tests und Werkzeug lesen dieselbe Liste); das Werkzeug
`projects/exec/InstrumentBodyTool` (Konsole: Bauplan → Netz → Platte → Moden, `--mesher`/`--solver`/`--list`,
`--frequencies`; den Körper schreibt es ab 31c-5); Modul-Doku 0.4.0 §5. Tests `test_LoopKoerperEigen` (6 Fälle:
Kette mit geschlossener Lösung, doppelte Eigenwerte, Prüfplatte gegen den dichten Löser, Bezugswerte
`werte/eigen_pruefplatte.txt`, Fehler und Abbruch, Messung `LUMIVIZ_EIGEN_MESSUNG=<Hz>`). **Gemessen** (Release-clang):
Steinway D (Netz bis 5 kHz, k = 3, 568 515 Größen) bis 5 kHz **820 Moden, gezählt 820**, 13 Zerlegungen (248 s),
größter Rest 4,0·10⁻⁷, **21 min, Spitze 4,8 GB**; bis 1 kHz 80 Moden in 164 s; erste Mode 38,2 Hz (Prüfstein 31c-6).
**Kipppunkte:** EG2 a b und c nicht erreicht (Reste klein, Zeit und Speicher brauchbar); BM5 a nicht erreicht — ein
Körper braucht nicht „Sekunden bis wenige Minuten", das Werkzeug bleibt der Ort der Rechnung. Release-clang
`*Loop Koerper*` 24 / 24; Testing-MSVC baut, getestet wird dort am Stufenende (MT1 a).

✅ **BM6 a** (Patrik S148): **Die Hörorte stehen im Bauplan als Liste, jeder mit Name und Lage (x, y, z in m im
Bezugssystem des Bodens, z über der Bodenebene).** Der Körper trägt je Hörort ein Abstrahlgewicht je Mode (aus dem
Rayleigh-Integral über ihre Form) — das sind die Ausgänge nach KB1 a. Zwei Hörorte geben Stereo aus derselben
Rechnung, dieselben Moden mit anderen Gewichten (Bank 2007, nur Bericht). Vorgabe für den Steinway D ein Stereopaar;
seine Lage klärt **BM6-1**, nachdem die Angaben von *Salamander* zur Lage seiner Mikrofone geprüft sind (Vergleich in
31d). Ein anderer Hörort heißt: das Werkzeug neu laufen lassen (rund 21 min). `.lvinstbodyplan` Fassung 2. Grund: KB1 a
sieht mehrere Ausgänge vor, der Bauplan ist der Ort der Eingaben der Rechnung; Stereo aus den Lagen über dem Boden
statt aus einem Panorama; 31d vergleicht Gleiches mit Gleichem. Verworfen: b (ein fester Ort im Werkzeug, Mono — ein
Wert im Code statt in der Datei), c (Leistung je Mode ohne Ort — trennt am reinsten, aber ohne Ort fehlt, was man
hört, und der Vergleich mit einer Aufnahme geht nicht; die Leistung kann in a zusätzlich mit). Grenzen wie im Befund:
Schallwand unter rund 200–300 Hz unzutreffend, kein Modell für den offenen Deckel. Kippt auf c, wenn 31d zeigt, dass
die Gewichte je Ort an *Salamander* nicht besser treffen als die Leistung je Mode.

**Befund S148 (selbst abgerufen):** die Readme von `sfzinstruments/SalamanderGrandPiano` (Weitergabe der Samples von
Alexander Holm): „Two AKG c414 disposed in an AB position ~12cm above the strings." Wo über dem Flügel, Abstand und
Deckel: nicht genannt; die Readme von Holm selbst nicht gesehen
(`docs/modellierung/hammer-saite/recherche/2026-10-23_S148_salamander_mikrofone.md`).

✅ **BM6-1 a** (Patrik S148): **Zwei Hörorte `left` und `right`, je 12 cm über den Saiten, über den Stegpunkten der
Tasten am ersten und dritten Viertel der Tastatur** (Steinway D: MIDI 43 bei (0,27; 1,41) m und MIDI 86 bei
(1,09; 0,09) m). „Über den Saiten“ = über der Oberseite des Stegs an diesem Punkt (Dicke der Platte plus Höhe des
Stegs). Die Höhe aus *Salamander*, die Waagrechte eine eigene Regel (die Quelle nennt sie nicht);
`make_steinway_d.py` rechnet die Lagen und schreibt sie als Zahlen mit Quelle in den Bauplan. Grund: die belegte
Höhe, die Waagrechte an Daten des Bauplans (wo die Saiten den Boden antreiben) statt an einer freien Zahl; die Regel
taugt für jeden Bauplan. Verworfen: b (AB über dem Schwerpunkt, 0,5 m — ein Abstand ohne Quelle, Bass und Diskant
kaum getrennt), c (Ohren des Spielers — ohne Bezug zu *Salamander*; als weiterer Hörort später möglich). Kippt, wenn
die Readme von Holm oder Bilder der Aufnahme eine andere Lage zeigen (dann deren Lage), oder wenn 31d das Verhältnis
Bass zu Diskant gegen *Salamander* deutlich daneben findet (dann dort anpassen, neu vorlegen).

✅ **PL31c-5 a** (Patrik S148): **31c-5 in vier Teilschritten, der statistische Teil dabei.** **31c-5a** Hörorte im
Bauplan (`.lvinstbodyplan` Fassung 2, Liste `listeningPoints`; Fassung 1 bleibt lesbar), die Lagen nach BM6-1 a aus
`make_steinway_d.py`. **31c-5b** Auswertung je Mode in `InstrumentBody`, während die Moden kommen (`jeMode`): der Wert
der Form an jedem Stegpunkt (über die Ansatzfunktionen des Elements, in dem er liegt), daraus die Masse je Stegpunkt;
die Dämpfung nach RT-0425 Gl. 1; je Hörort das Abstrahlgewicht aus dem Rayleigh-Integral über die Gauss-Punkte des
Netzes, komplex bei der Frequenz der Mode (Betrag und Laufzeit — über die schmale Bandbreite einer Mode ändert sich das
Integral kaum); geprüft gegen eine geschlossene Lösung (Rechteckplatte frei aufliegend). **31c-5c** statistischer Teil
ab der höchsten gerechneten Mode (vorläufig 5 kHz; BM7 kann die Grenze in 31c-6 senken), mit einer Frage davor (woher
seine Größen kommen). **31c-5d** die Datei `.lvinstbody` (Fassung 1) lesen und schreiben, mit Abdruck des Bauplans und
der Fassung des Werkzeugs; das Werkzeug schreibt sie; `steinway_d.lvinstbody` committet (Größe gegen den Kipppunkt von
BM3 c, rund 10 MB); Wächter-Test; Eintrag in `asset/Asset.cmake`. Grund: über 5 kHz gibt es keine gerechneten Moden —
der statistische Teil wird gebraucht, wo BM7 die Grenze auch hinlegt; BM3 a nennt ihn als Teil des Körpers, die Datei
hat so von Anfang an ihre Gestalt; die Grenze ist eine Zahl in der Datei, eine Änderung durch BM7 heißt neu rechnen,
keine neue Fassung. Verworfen: b (ohne statistischen Teil, er käme mit BM7 als Fassung 2). Kippt auf b, wenn die Frage
vor 31c-5c zeigt, dass seine Größen erst aus den Messungen des Prüfsteins in 31c-6 kommen können.

**Gebaut 31c-5a (S148):** `Hoerort` in `Bauplan.hpp` (Name, Lage, Höhe z, Quelle; `pruefe`: Name eindeutig, über der
Platte), `.lvinstbodyplan` **Fassung 2** mit `listeningPoints` (`loop/KoerperBauplan`; Fassung 1 ohne die Liste
bleibt lesbar), `make_steinway_d.py` schreibt das Stereopaar nach BM6-1 a — erst in eine Kopie geschrieben und
verglichen: nur Fassung und Hörorte neu. **Präzisiert:** z zählt über der **Mittelfläche** der Platte (der Ebene, in
der die Rechnung sie als dünn nimmt); die Oberseite des Stegs liegt dort eine halbe Plattendicke plus die Steghöhe
darüber (BM6-1 a sagt „Dicke der Platte plus Höhe des Stegs“ — gemessen von der Unterseite, derselbe Ort). Steinway D:
`left` (0,27; 1,41) m, z = 173,7 mm; `right` (1,09; 0,09) m, z = 154,8 mm. Tests in `test_LoopKoerperBauplan`
(`pruefe`, Datei mit Fassung 1, das Paar des Steinway gegen Stegpunkt und Steghöhe auf 1 mm). Release-clang
`*Loop Koerper*` 24 / 24; Testing-MSVC baut.

**Befund S148 zur Dämpfung (selbst gelesen, an den Seitenbildern):** RT-0425 §2.1 (gedruckt S. 4) nennt nur die
Konstanten „a = 2.00 × 10⁻⁵ (1a), b = 0.07 (1b), c = 0.00 (1c)“ für „a modal approach which consists in considering
that the damping occurs mode per mode“, das Gesetz selbst nicht. Chabassier, Joly & Chaigne 2013 Gl. 17 schreibt die
Dämpfung als Funktion der Frequenz: Ẍn + α(fn) Ẋn + (2πfn)² Xn = Fn, „α is a positive damping function matching
experimental data“. Chabassier, Duruflé & Joly 2015 Teil 2 (S. 6) gibt die Form fd(λ) = αλ + β√λ + γ mit λ = ω².
**Gelesen als α(f) = a f² + b f + c, f in Hz** (dieselbe Form wie Teil 2, in f statt in ω): dann ist der
Verlustfaktor η = α/(2πf) 1,1 % bei 100 Hz, 1,4 % bei 1 kHz, 2,7 % bei 5 kHz — im gemessenen Bereich 1–3 % (Ege,
Boutillon & Rébillat 2013). In ω gelesen gäben dieselben Zahlen 8 % bei 100 Hz bis 70 % bei 5 kHz, physikalisch nicht
haltbar. Die Lesart ist eine Deutung, kein Wortlaut; der Prüfstein (31c-6) und *Salamander* (31d) messen sie nach.

**Gebaut 31c-5b (S148):** `InstrumentBody/Auswertung.hpp` — `Auswertung` (einmal vorbereitet: Abtastung der
Stegpunkte, Gauss-Punkte mit ihrem Abstand zu jedem Hörort) und `werte(mode)` → `ModeImKoerper`: Frequenz,
Verlustfaktor η (nach dem Befund oben), φ je Stegpunkt (über die Ansatzfunktionen des Elements; die modale Masse ist
1/φ², das Vorzeichen bleibt), je Hörort das Abstrahlgewicht G = ρa/(2π) ∫ φ e^{−iωR/c}/R dS bei der Frequenz der Mode
(komplex, √kg/m²; p = q̈ · G). Dazu in der Platte `PlattenSystem::elemente`, `Plattenelement::basis` und
`Auflager::Frei` (nur für Prüfungen). Das Werkzeug wertet jede Mode beim Liefern aus. Tests `test_LoopKoerperAuswertung`
(5 Fälle): Dämpfung, Abtastung (Polynome genau), Rayleigh am Kolben auf der Achse gegen die geschlossene Lösung auf
0,2 % (auch im Nahfeld), die erste Mode der Prüfplatte gegen Navier (φ auf 0,08 %, Fernfeld auf 0,09 %), Steinway.
**Gemessen** (Release-clang, `InstrumentBodyTool`, Steinway D bis 5 kHz): alle 88 Stegpunkte im Netz, Vorbereitung
0,06 s, **Auswertung aller 820 Moden 17 s** (innerhalb der 1125 s des Lösers), Spitze 4,9 GB; erste Mode 38,2 Hz,
η = 1,13 %, |G| links 0,149, rechts 0,108 √kg/m². Release-clang `*Loop Koerper*` 29 / 29; Testing-MSVC baut.

**Befund S148 vor 31c-5c (selbst gerechnet, aus den 820 Moden des Steinway bis 5 kHz):** Modendichte 0,064 /Hz unter
500 Hz (Ege, Boutillon & Rébillat 2013: „about 0.06 modes Hz⁻¹“), dann gleichmäßig steigend bis 0,230 /Hz bei
4,5–5 kHz. Die Abstände, auf die örtliche Dichte normiert: Mittel 0,998, Varianz 0,285, Anteil unter 0,25 bei 4,4 % —
die Rayleigh-Verteilung von Boutillon & Ege 2013 Gl. 23/24 hat 1, 0,273 und 4,8 % (Poisson: 1, 1, 22 %). Das gerechnete
Modell zeigt sie von sich aus. Boutillon & Ege (S. 19–21, an den Seitenbildern gelesen): Gl. 23/24 Abstände nach
Rayleigh, Gl. 25 Dämpfung η = 2,3 % · r (Chi-Quadrat), Gl. 26/27 mittlere Mobilität n/(4M); ihre Synthese begrenzen sie
selbst („We have limited the frequency range of the synthesis to this limit“, S. 20), zur Abstrahlung nichts.

✅ **ST1 a** (Patrik S148): **Die Größen des statistischen Teils kommen aus den gerechneten Moden, die Form der Ziehung
aus Boutillon & Ege.** Dichte n(f) aus dem oberen Band der gerechneten Moden (2,5–5 kHz) fortgesetzt; Abstände nach
Rayleigh (Gl. 23/24); Dämpfung nach demselben Gesetz wie die gerechneten Moden (RT-0425 Gl. 1, keine Naht bei 5 kHz);
je Stegpunkt ein Formwert, normal verteilt mit der Streuung der gerechneten Moden im oberen Band dort, Vorzeichen
zufällig, die Stegpunkte unabhängig; je Hörort ein Gewicht mit dem mittleren Betrag des oberen Bands und gleichverteilter
Phase. Bis 20 kHz. Die Datei trägt nur diese Größen und den Samen, gezogen wird beim Bilden der Antworten (KB1 a) —
ausgeschrieben wären es rund 5000 Moden zu je 92 Zahlen, nahe am Kipppunkt von BM3 c. Grund: ein Instrument, eine
Quelle — bei 5 kHz passen Dichte, Dämpfung und Mobilität ohne Sprung; die Form der Ziehung ist an den gerechneten Moden
belegt. Verworfen: b (Formeln von Boutillon & Ege aus dem Bauplan — die Autoren begrenzen ihre Synthese selbst darunter,
ihre Dämpfung widerspricht RT-0425, für die Abstrahlung keine Vorlage), c (Messungen des Prüfsteins — gemessen ist eine
mittlere Mobilität anderer Flügel, keine Dichte und keine Abstrahlung; taugt als Prüfung). Kippt auf c, wenn der
Prüfstein (31c-6) die gerechnete Mobilität oberhalb rund 3 kHz deutlich neben Plath oder Giordano findet; kippt bei der
Ziehung, wenn benachbarte Stegpunkte über 5 kHz merklich zusammen schwingen.

**Gebaut 31c-5c (S148):** `InstrumentBody/Statistik.hpp` — `StatistischerTeil` (was die Datei trägt: Grenzen, Dichte
und Anstieg, Dämpfung, Streuung je Stegpunkt, Betrag je Hörort, Samen, das Band der Schätzung), `schaetze` (Dichte als
Gerade durch die Zählung in zehn Stücken des Bands, fallend wird waagrecht; Streuung und Betrag als quadratisches
Mittel — „mittlerer Betrag“ aus ST1 a so gelesen, dieselbe Leistung wie die gerechneten Moden), `ziehe` (Rayleigh nach
Gl. 23/24, normal je Stegpunkt, Phase gleichverteilt), `Zufall` (SplitMix64, eigene Umrechnungen, auf jedem Compiler
dieselbe Folge; zwei Züge nie in einem Ausdruck). Das Werkzeug hängt den Teil an (`--statistics-to`, Vorgabe 20 kHz;
`--seed`, Vorgabe 1). Tests `test_LoopKoerperStatistik` (3 Fälle: Zufall gegen Bezugsfolge und Verteilungen, Ziehung,
Schätzung). **Gemessen** (Steinway D, Band 2,5–5 kHz, 528 Moden): Dichte 0,238 /Hz bei 5 kHz, Anstieg 2,1·10⁻⁵ /Hz²,
**5933 Moden bis 20 kHz gezogen**, Streuung an den Stegpunkten im Mittel 0,087 /√kg, |G| links 0,0067, rechts 0,0069
√kg/m². Release-clang `*Loop Koerper*` 32 / 32; Testing-MSVC baut.

**Gebaut 31c-5d (S148) — damit ist 31c-5 fertig:** `InstrumentBody/Koerper.hpp` (`Koerper`, `Rechnung`,
`kKoerperFassung` = 1, `kRechenFassung` = 1 — hochzählen, wenn bei gleichem Bauplan ein anderer Körper herauskäme),
`loop/Koerper` in der App (die Datei `.lvinstbody` mit Qt-JSON, jedes Feld geschrieben, Zahlen genau zurück, Samen als
Text; `bauplanAbdruck` = SHA-256 über das kompakte JSON des Bauplans); das Werkzeug schreibt den Körper neben den
Bauplan (`--out`); **`asset/loop/instrument_bodies/steinway_d.lvinstbody`** gerechnet und abgelegt (820 Moden bis 5 kHz,
statistischer Teil bis 20 kHz, **2,9 MB** — weit unter dem Kipppunkt von BM3 c, 22 min); in `asset/Asset.cmake` gehen
nur die Körper mit, nicht die Baupläne. Tests `test_LoopKoerperDatei` (3 Fälle: Datei, Abdruck, **Wächter** über jeden
mitgelieferten Körper — Abdruck des Bauplans, `kRechenFassung`, Stegpunkte und Hörorte wie im Bauplan, Moden
aufsteigend im Fenster, Anschluss des statistischen Teils, unter 10 MB). Release-clang `*Loop Koerper*` 35 / 35;
App unter Release-clang und Testing-MSVC bauen.

**Befund S149 vor BM7 (selbst gerechnet, aus den Zahlen oben):** Die Modenüberlappung M = f · η · n(f) der gerechneten
Moden des Steinway liegt bei rund 0,4 bei 500 Hz, 1,1 bei 1 kHz und 31 bei 5 kHz — M = 1 also bei rund 1 kHz (Ege:
1,1 kHz, nur Bericht; Boutillon, Ege & Paulello Tab. 1 für den Steinway D fg ≈ 1355 Hz zwischen tiefem und hohem
Bereich, Zuordnung der Spalten vom Agenten erschlossen). Bei 5 kHz liegen die Moden rund 4 Hz auseinander; ein Fehler
der Frequenz von 0,1 % ist dort schon 5 Hz — welche Mode genau wo liegt, steht nicht mehr fest, Dichte und mittlere
Mobilität können trotzdem stimmen.

✅ **BM7 a** (Patrik S149): **Die Grenzfrequenz ist die höchste gerechnete Mode (Steinway D: 5 kHz); gerechnete Moden
werden nicht durch gezogene ersetzt, solange sie im Mittel stimmen.** Geprüft bandweise in Terzbändern, nur Dichte und
mittlere Mobilität an den Stegpunkten, nicht einzelne Frequenzen. 31c-6 prüft (1) die **Konvergenz** — eine zweite
Rechnung mit feinerem Netz oder Ordnung k + 1 bis 5 kHz, die Mittel je Band auf wenige Prozent gleich; Rechenzeit und
Speicher vorher an einem kleinen Fenster gemessen, wird es zu teuer, eigene Rückfrage; (2) den **Prüfstein** — mittlere
Mobilität gegen Plath und Giordano. Versagt ein Band, sinkt die Grenze auf seinen unteren Rand; der Körper wird neu
gerechnet, das Band der Schätzung nach ST1 a rückt mit. Keine neue Fassung — die Grenze ist eine Zahl in der Datei.
Grund: Die Überlappung ist in der Summe der gerechneten Moden schon enthalten; der statistische Teil ersetzt nur, was
nicht gerechnet ist, und gibt Mittel ohne Zusammenhang zwischen den Stegpunkten (ST1 a), die gerechneten Formen
verbinden Stegpunkte und Hörorte. Verworfen: b (Grenze bei M = 1, rund 1 kHz — die Schule Boutillon & Ege setzt dort
an, weil sie darüber nichts rechnet; die Statistik würde aus 0,5–1 kHz über mehr als vier Oktaven fortgesetzt),
c (Grenze, wo der Frequenzfehler den halben Modenabstand übersteigt — verlangt einzeln stimmende Moden, die der Klang
nicht braucht; würfe Moden weg, die im Mittel stimmen). Kippt auf b, wenn die Konvergenzprüfung schon unter rund
1,5 kHz versagt; liegt nur der Prüfstein oberhalb rund 3 kHz daneben, greift zusätzlich der Kipppunkt von ST1 a —
beides dann zusammen neu vorlegen.

**Hinweis Patrik (S149):** Stufen zerfallen immer feiner (31 → 31c → 31c-3 → 31c-5a), ohne abzuschließen, statt von
Anfang an vollständig geplant zu sein. Ursache: Fragen, die den Umfang eines Schritts bestimmen, standen als „eigene
Frage“ im Schritt (HS2 a: 31c „woher die Impulsantwort kommt: eigene Frage“ — mit BM1 a wurde daraus ein FEM-Werkzeug
in acht Teilschritten), geplant war genau nur der nächste Schritt.

✅ **PL31 a** (Patrik S149): **Erst 31c abschließen, dann den Rest von Stufe 31 planen; der Rest von 31c wird vorher
vollständig geklärt.** (1) Die zwölf Fragen, die den Umfang von 31c-6 bis 31c-8 und den Abschluss bestimmen, werden
geklärt — zuerst zwei kurze Recherchen (Vergleichswerte der Mobilität oberhalb 1 kHz; Werte für Glas mit Quelle), dann
jede Frage einzeln: *31c-6* (1) Vergleich oberhalb 1 kHz (Plath nur 50–1000 Hz, Giordano 1998 am Klavier, Conklin 1996
bis 3,2 kHz ohne bekannte Bezugsgröße), (2) Bauzustand des Vergleichs (gemessen eingebaut und besaitet, das Modell
eingespannt ohne Saitenlast und Wölbung), (3) Bauplan des MAESSTRO-Prüffalls aus Abb. 4 (Bildschirmfoto ohne Maßstab),
(4) Weg der Konvergenzrechnung (`--order 4` oder feineres Netz, vorher an einem kleinen Fenster gemessen),
(5) Toleranzen als Zahlen; *31c-7* (6) wo und wann die Antworten je Taste entstehen (tiefste Mode T60 rund 5 s, 88 × 2
Antworten roh rund 170 MB), (7) Faltung (Weg, Rechenzeit je Ton, Mono-Kraft zu Stereo), (8) Pegel (Stegkraft heute
„als 1“, mit dem Boden Schalldruck; `instrument_pegel` neu), (9) `soundboard.amount` und `.lvinst` Fassung 17;
*31c-8* (10) welche Rechnungen (MAESSTRO MP1–MP3, Glas in zwei Dicken), (11) Werte für Glas; *Abschluss 31c* (12)
Testumfang, Hörproben, Doku, voller Lauf jetzt oder am Ende von Stufe 31. (2) Daraus der feste Plan 31c-6 bis 31c-8
plus ein Schritt *Abschluss 31c*; Nummern fest, keine neue Ebene. (3) Bauen bis zum Abschluss von 31c. (4) Danach für
31d bis 31h dasselbe: alle Umfangsfragen klären, die Stufe bis zu ihrem Abschluss planen. Wächst ein Plan trotzdem,
als Entscheid „Plan erweitern“ vorlegen. Grund: der Umfang steht fest, bevor gebaut wird, das Ende der Stufe ist ein
geplanter Schritt. Verworfen: b (Rest von 31c wie geplant, Fragen je vor ihrem Schritt — die Fragen 1, 3 und 6 können
je einen Schritt verdoppeln). Kippt für eine Frage, die sich nur durch Bauen klären lässt (etwa die Rechenzeit der
Faltung): sie wird ein benannter Mess-Schritt mit festem Platz im Plan.

**Befund S149 zum Prüfstein (Recherchen `docs/modellierung/hammer-saite/recherche/2026-10-24_S149_mobilitaet_ueber_1khz.md`
und `…_glas_werkstoffwerte.md`; selbst nachgelesen wie dort im Kopf vermerkt):** Gemessene Mobilität am Steg eines
Flügels oberhalb 1 kHz gibt es nur bei Plath 2019 Fig. 6.10 (bis 5 kHz, Treiberpunkt, sechs Bauzustände; die Achse
„Driving point mobility [dB]“ ohne Bezugsgröße — am Bild gesehen; der Agent erschließt 10⁻⁴ s/kg aus Fig. 6.9) und bei
Conklin 1996 (bis 3,2 kHz, Bezug unbekannt, nur in Wiedergabe). Absolut skaliert nur Giordano 1998 an einem Klavier
(C4, in Wiedergabe bei Boutillon & Ege 2013 Fig. 7; abgelesen rund −66 dB re 1 s/kg bis 3 kHz). **Zum Bauzustand
(Frage 2):** Plath S. 89 — Steg „decreases overall mobility by 10 dB and 10 dB to 20 dB above 1 kHz“, nach dem
Einleimen „From 500 Hz to 5000 Hz the mean mobility stays constant“, Saiten und Rahmen wirken „Up to 350 Hz“; das
Modell (eingespannt, ohne Saitenlast und Wölbung, mit Steg und Rippen) ist oberhalb rund 500 Hz mit Plath PROD5/PROD6
vergleichbar, darunter nicht. MAESSTRO Abb. 6 ist ein Wert je Stegpunkt — das geometrische Mittel von |Y| über
25–5000 Hz (Gl. 12, S. 8), dB re 1 s/kg („about 1500 kg/s, which corresponds to a mean mobility of −63.5 dB“) —, und
MAESSTRO setzt η = 2 % je Werkstoff (Tab. 2).

✅ **PS1 a** (Patrik S149, Frage 1 und 2 aus PL31 a): **Drei Prüfungen, je mit der Toleranz ihrer Quelle; nur die
Messungen können eine Grenze kippen.** Alle rechnen die Mobilität am Stegpunkt nach MAESSTRO Gl. 10 aus den
gerechneten Moden (Treiberpunkt, mit dem η der Moden). (1) **MAESSTRO Abb. 6 am Prüffall** (gleiche Vereinfachungen,
η = 2 %): je Stegpunkt das geometrische Mittel von |Y| über 25–5000 Hz in 1-Hz-Schritten (Gl. 12); bestanden, wenn
jeder Wert auf ±2 dB an der abgelesenen Kurve liegt (abgelesen auf ±1 dB) — prüft den Code. (2) **Dasselbe Mittel am
Steinway D aus RT-0425:** bestanden, wenn die Werte im Bereich von MAESSTRO liegen (−75 bis −57 dB) und der Bass 5 bis
15 dB unter dem Diskant — prüft die Geometrie grob (Platte 6–9 statt 8 mm). (3) **Gegen die Messungen in Terzbändern
500 Hz bis 5 kHz**, Mittel über die Stegpunkte im Diskant, gegen Plath PROD5/PROD6 (Bezug 10⁻⁴ s/kg als Annahme
vermerkt) und Giordano C4: bestanden, wenn jedes Band zwischen −72 und −58 dB re 1 s/kg liegt (beide ±5 dB) und kein
Anstieg oder Abfall über 6 dB von 500 Hz bis 5 kHz auftritt; unter 500 Hz nur Plath Fig. 6.9 (Mittel 50–1000 Hz
0,0007 s/kg, bis Faktor 2). Versagt (3) in einem Band, sinkt die Grenze nach BM7 a, oberhalb 3 kHz greift zusätzlich
der Kipppunkt von ST1 a; versagt (1), wird der Fehler im Code gesucht, die Grenze bleibt; versagt nur (2), wird die
Geometrie geprüft. Grund: MAESSTRO ist am selben Flügel mit beschriebenen Vereinfachungen der schärfste Test des
Codes, nur Messungen zeigen die Wirklichkeit, sind aber unscharf — deshalb verschieben nur sie eine Grenze, mit ihrer
Toleranz. Verworfen: b (nur MAESSTRO — Rechnung gegen Rechnung, BM7 a und ST1 a beziehen sich auf Messungen), c (nur
Messungen — Code- und Geometriefehler nicht zu trennen, gegen BM2 a). Kippt, wenn Abb. 6 nicht auf ±1 dB lesbar ist
(dann Prüfung 1 auf ±3 dB, vermerkt), und wenn Plaths Bezugsgröße belegt auftaucht (dann sie statt der Annahme).

**Befund S149 zu MAESSTRO Abb. 4 (selbst angesehen, 300 dpi):** ein Bildschirmfoto der Oberfläche ohne Maßstab,
der Boden rund 500 Pixel hoch (rund 4 mm je Pixel); die Liste nennt „Ribs - 17“ (16 im Bild zu zählen), eine Rippe
mit Enden in mm ((147; 1694) und (336; 1850), Breite 25, Höhe 27); als Werkstoff steht dort „Norway Spruce 1“, in
Tab. 2 Sitka. MP2 (Abb. 4 b) hat jede zweite Rippe.

✅ **PF1 a** (Patrik S149, Frage 3 aus PL31 a): **Der MAESSTRO-Prüffall entsteht aus unserem Bauplan mit den
Vereinfachungen von MAESSTRO.** `make_steinway_d.py --maesstro` schreibt eine zweite Datei: Umriss, Steglinien,
Rippenlinien und Stegpunkte wie im Steinway D aus RT-0425; darauf, was MAESSTRO in Text und Tab. 1–2 nennt — Platte
8 mm, Stege 60 bzw. 20 × 32 mm ohne Verlauf, Rippen ohne Verjüngung (Höhe 27 → 17 mm, Breite 25 → 21 mm von Bass zu
Diskant), Faser 135°, Sitka und Buche aus Tab. 2, η = 2 % je Werkstoff. Unter den Testdaten, nicht mitgeliefert
(BM3 a). Geprüft am Überlagerungsbild gegen Abb. 4 a (Zahl, Richtung, Abstand der Rippen, Lage der Stege; die Rippe
mit Koordinaten in mm als Stichprobe). MP1–MP3 für 31c-8 aus derselben Datei (16 mm; jede zweite Rippe; ohne Rippen).
Grund: der Prüffall trennt Code- von Geometriefehlern (BM2 a) nur, wenn er sich vom Steinway D allein in Zahlen mit
Quelle unterscheidet. Verworfen: b (aus Abb. 4 abgezeichnet — rund 4 mm je Pixel, Maßstab aus einer Rippe; eine
Abweichung wäre nicht mehr dem Code zuzuordnen). Kippt, wenn das Überlagerungsbild Rippen deutlich anders zeigt
(Winkel über 5°, Abstand über 2 cm daneben): dann die Rippen aus Abb. 4 a, Umriss und Stege bleiben.

**Befund S149 zum Konvergenztest (MAESSTRO S. 4, selbst gelesen):** Gl. 2 vergleicht L = 5 Moden „in the high
frequency domain, where the numerical dispersion is the highest“ zwischen Ordnung k und k + 1 und nimmt das kleinste
k mit mittlerer relativer Abweichung unter ε; „In the paper ε = 10⁻⁶, which leads to k = 3“. Dazu: „the user can only
provide a constant modal loss factor using a material dictionary“. Für k = 4 auf dem Netz des Steinway überschlagen
(aus Knoten, Kanten und Dreiecken): rund 1,0 Mio. freie Größen statt 568 515; Speicher und Zeit nicht gemessen.

✅ **KV1 a** (Patrik S149, Frage 4 und 5 aus PL31 a): **Die Konvergenz prüft eine zweite Rechnung mit k = 4 auf
demselben Netz bis 5 kHz.** Vorher ein Messlauf k = 4 im Fenster 0–500 Hz (die Zerlegung bestimmt den Speicher, die
Zeit wird hochgerechnet); über 20 GB Spitze oder über 3 h hochgerechnet: eigene Rückfrage. Verglichen werden (1) je
Terzband 100 Hz–5 kHz die Modendichte auf ±3 % und die mittlere Mobilität an den Stegpunkten (geometrisches Mittel
nach PS1 a) auf ±0,5 dB — das Kriterium von BM7 a, ein versagendes Band senkt die Grenze; (2) die ersten 20 Moden
einzeln auf ±0,1 %; (3) MAESSTRO Gl. 2 an den fünf höchsten Moden (nach Rang zugeordnet) als Zahl im Körper,
berichtet, kein Kriterium. Fällt (1) überall durch, wird k = 4 Vorgabe und der Körper neu gerechnet (`kRechenFassung`
hochzählen); bestehen alle, bleibt k = 3. Grund: der Weg von EL1 a und beiden Quellen — an der Ordnung konvergieren,
auf demselben Netz misst der Vergleich nur den numerischen Fehler; die Toleranzen weit unter denen von PS1 a, damit
die Numerik dort nicht mitzählt. Verworfen: b (feineres Netz mit k = 3 — vermischt Geometrie und Numerik), c (beides —
eine zweite große Rechnung, die keine Quelle verlangt). Kippt, wenn der Messlauf über der Rückfragegrenze liegt (dann
vorlegen: Fenster teilen oder b), und wenn (1) besteht, (2) aber nicht (dann kommt b dazu).

✅ **AW1 a** (Patrik S149, Frage 6 aus PL31 a): **Die App bildet die Antwort des Bodens je Taste und Hörort aus der
Körperdatei, wenn die Taste zum ersten Mal klingt** — im Hintergrund wie die Töne, gemerkt im Tonspeicher unter
Abdruck des Körpers, Taste, Hörort, Rate. Gebildet als Summe der Moden direkt im Spektrum (je Mode ihr Beitrag an den
Frequenzpunkten der FFT für die Faltung), die gezogenen Moden aus Samen und Größen der Datei (ST1 a). Länge bis die
langsamste Mode 60 dB gefallen ist, höchstens 8 s (tiefste Mode 38 Hz, η 1,1 %: rund 5 s). **31c-7 beginnt mit einem
benannten Messteil:** Zeit zum Bilden einer Antwort und Speicher je Taste (Release-clang); über 1 s je Antwort:
Rückfrage. Die Faltung bleibt nach 31e richtig (der Boden ist linear und nur über den Steg angetrieben; 31e ändert die
Kraft, nicht die Antwort). Grund: die Körperdatei bleibt die eine Quelle, die Antwort ist eine jederzeit neu
bildbare Form derselben Moden; es entsteht nur, was gespielt wird. Verworfen: b (Werkzeug rechnet vorab, rund
170 MB je Körper und Rate — eine zweite Quelle mit eigenem Wächter), c (Moden als Resonatoren im Ton — rund 6700
Resonatoren je Abtastwert und Hörort, vielfach teurer als die Faltung; 31e braucht es nicht). Kippt, wenn eine Antwort
auch im Spektrum deutlich über 1 s braucht: zuerst vorlegen, Moden außerhalb ihres Bands zu kappen; erst dann b, als
Zwischenspeicher im Benutzerordner.

✅ **FT1 a** (Patrik S149, Frage 7 aus PL31 a): **Gefaltet wird je Ton und Hörort mit einer FFT über den ganzen
Ton** (auf die nächste Zweierpotenz über Ton plus Antwort aufgefüllt); die Antwort liegt im Tonspeicher als Zeitreihe
(AW1 a) und wird je Ton ins Spektrum gebracht. Die FFT kommt aus **pocketfft** (C++, nur Header, BSD-3), unverändert
unter `externals/pocketfft` mit README (SHA-256) und eigener `Include.cmake`, hinter einer eigenen Schnittstelle `Fft`
in `loop/`, mit Tests gegen die Schnittstelle (gegen die direkte Faltung an kurzen Signalen, Bezugswerte); der
Download mit Freigabe Patrik (Datei, Quelle, Größe), wenn es so weit ist. Der Ton wird echtes Stereo (`left` links,
`right` rechts; ein Hörort: Mono in beide). Heute schreibt `Instrument.cpp` die Stegkraft in beide Kanäle. Die
Rechenzeit je Ton misst der Messteil von AW1 a mit. Grund: fremder Code für die Kernrechnung hinter eigener
Schnittstelle, austauschbar (wie Eigen, Spectra); der Ton entsteht ohnehin am Stück vorab. Verworfen: b (eigene FFT —
nachgebauter Code ohne Gewinn), c (Blöcke, overlap-add — Echtzeit ist kein Ziel, später hinter derselben
Schnittstelle nachrüstbar). Kippt auf b, wenn pocketfft bei Lizenz oder Einbindung Probleme macht; auf c, wenn der
Messteil den Speicher langer Töne im Bass zu groß findet.

✅ **PG1 a** (Patrik S149, Frage 8 aus PL31 a): **Ein fester Bezug für den Schalldruck, gleich für alle Körper** —
`kSchalldruckBezug` (Pa = 1) neben `kHammerKraftBezug`, mit Herkunft im Kommentar. Den Wert legt der Messteil von
31c-7 einmal fest: der lauteste Ton des Steinway D (höchste Anschlagstufe, lauteste Taste) bei −6 dBFS Spitze. Die
Antworten bleiben unnormiert (Taste, Hörort und Anschlag behalten ihr Verhältnis); der Pegel des Instruments kommt wie
bei allen über `instrument_pegel.py` (Messung in 31h für `grand_piano`). Der Messteil berichtet den Schalldruck in
dB SPL am Hörort; verglichen mit *Salamander* in 31d. Grund: ein Bezug verschiebt nur, wo „laut“ auf der Skala liegt;
was der Boden physikalisch bringt (Bass gegen Diskant, links gegen rechts, Anschlagstufen), bleibt. Verworfen: b
(Antworten normiert — nimmt je Taste weg, wofür der Boden gerechnet wird), c (alter Bezug, skaliert — bindet den
Schalldruck an eine gesetzte Kraft). Kippt, wenn die Varianten aus 31c-8 bei diesem Bezug über 0 dBFS gehen: dann
liegt der Bezug am lautesten aller mitgelieferten Körper, weiter einer für alle.

✅ **SB1 a** (Patrik S149, Frage 9 aus PL31 a): **`soundboard` trägt in `.lvinst` Fassung 17 nur `body`** — den
Verweis auf einen Körper (`lib:<name>`; `user:` erst mit einem Editor, BM3 a), immer geschrieben, leer = ohne Boden;
**`amount` fällt weg** (Fassung 16 bleibt lesbar, `amount` wird überlesen — es wirkte nie). Mit Körper klingen seine
Hörorte `left` und `right` (BM6-1 a) als Stereo; andere Namen: der erste links, der zweite rechts; einer allein in
beide Kanäle. Ohne Körper klingt die Stegkraft wie heute (für Tests und als Vergleich). Fehlt die Körperdatei, klingt
der Ton ohne Boden, der Bericht meldet „Body not found“ (wie bei einem fehlenden Sample). `kInstrumentFassung` = 17;
vorher die Fassungs-Literale in `tests/` suchen, danach `*Klangfassung*`. Grund: ein Feld trägt, was es physikalisch
gibt — beim Flügel kommt der ganze Klang durch den Boden. Verworfen: b (`amount` als Mischverhältnis — gäbe die
Stegkraft als Klang aus), c (`amount` als Stärke — wiederholt Lautstärke und Pegel, PG1 a). Kippt, wenn 31d gegen
*Salamander* einen Anteil Direktschall braucht (Hammer, Mechanik) — dann als eigenes Feld mit eigener Quelle.

✅ **DM1 a** (Patrik S149, Frage 13 aus PL31 a, neu beim Sammeln): **η je Mode nach dem Anteil der
Verformungsenergie je Werkstoff** (modale Verformungsenergie; Johnson & Kienholz 1982 — aus Claudes Wissen, nicht
nachgelesen): η_Mode = Σ_Werkstoff η_Werkstoff(f) · U_Werkstoff / U_gesamt, U je Element aus φ_eᵀ K_e φ_e, beim
Auswerten jeder Mode mitgerechnet. Ein Werkstoff mit `lossFactor` > 0 trägt diesen festen Wert, einer mit 0 das Gesetz
des Körpers (RT-0425 Gl. 1) — der Wortlaut des Feldes im Bauplan. Der Steinway D (überall 0) behält seine
Verlustfaktoren; weil der Anteil je Werkstoff als Zahl je Mode in den Körper kommt, ändert sich die Datei
(`kKoerperFassung`, neu rechnen). Der MAESSTRO-Prüffall bekommt 2 % (PF1 a), ein Glasboden das Glas mit seinem Wert,
Rippen und Stege aus Holz mit dem Gesetz. **Grenze, in der Doku benannt:** für Glas fehlen die Verluste an Rand und
durch Abstrahlung (für Holz stecken sie im Gesetz von RT-0425). Grund: die Dämpfung einer Mode hängt davon ab, wo sie
ihre Energie hat; nur Größen, die schon da sind. Verworfen: b (ein η je Körper — ein Glasboden mit Holzrippen bekäme
einen Wert, den keiner der Werkstoffe hat), c (Gesetz für alle — gegen GB1 a). Kippt, wenn die Hörproben in 31c-8 den
Glasboden unnatürlich lange klingen lassen: dann die Dämpfung durch Abstrahlung (Rayleigh über die ganze Fläche) als
eigene Frage.

✅ **GL1 a** (Patrik S149, Frage 11 aus PL31 a): **Der Glasboden ist Kalk-Natron-Floatglas, monolithisch, isotrop** —
ρ = 2500 kg/m³, E = 70 GPa, ν = 0,2 (europäische Datenblätter nach EN 572; NGA FM05-12 als Gegenwert 71,7 GPa, 0,22 —
beide als Quelle im Bauplan), **η = 1·10⁻³** fest (Cremer/Heckl über Irvine 2010 Tab. 7, Mitte der Spanne 0,6–2·10⁻³;
Ashby nennt weniger — vermerkt). Rippen und Stege bleiben Fichte und Buche (GB1 a). Zwei Baupläne: **Dicke wie Holz**
(Verlauf 9 → 6 mm) und **gleiche Masse** (Verlauf mal 380/2500: 1,4 → 0,9 mm; Biegewellen rund 20 % kürzer, Netz rund
1,6-mal so viele Größen — selbst gerechnet). Die Doku vermerkt: Bruch unter der Saitenlast nicht gerechnet (gedachtes
Instrument), Rand und Abstrahlung fehlen in der Dämpfung (DM1 a). Grund: das bestbelegte Glas; für η gibt es bei
keinem Glas mehr als eine Spanne. Verworfen: b (Borosilikat — dieselbe Unsicherheit bei η), c (Verbundglas — eine
Platte aus Schichten, Werte nur aus Patenten). Kippt, wenn die Rechnung des dünnen Glasbodens über der
Rückfragegrenze von KV1 a liegt (20 GB oder 3 h): dann nur „Dicke wie Holz“, vorgelegt.

✅ **VA1 a** (Patrik S149, Frage 10 aus PL31 a): **31c-8 rechnet fünf Körper in zwei Familien, je gegen ihre eigene
Grundlage.** **Familie MAESSTRO** (aus dem Prüffall nach PF1 a): MP1 (Platte 16 mm), MP2 (jede zweite Rippe), MP3
(ohne Rippen) — geprüft gegen die Richtungen von MAESSTRO: Modendichte MP1 < RP < MP2 < MP3 (Abb. 5), mittlere
Mobilität MP3 höchste, MP1 niedrigste, Abstand Bass/Diskant bei MP1 rund 5 statt 10 dB (S. 8); unter den Testdaten,
nicht mitgeliefert. **Familie Steinway** (aus dem Steinway D nach RT-0425): Glas mit Dicke wie Holz und Glas auf
gleiche Masse (GL1 a), beide als `lib:`-Körper unter `asset/loop/instrument_bodies/` mitgeliefert (je rund 3 MB) für
31h. **Hörproben** je Familie gegen ihre Grundlage (RP gegen MP1–MP3, Steinway gegen beide Glasböden), dieselben Töne
für alle (Satz in Frage 12). Weitere Varianten auf dem echten Steinway (dickere Platte, kleinerer Boden) **bei
Bedarf** nach Patriks Urteil über die Hörproben (BM1 a). Grund: jede Rechnung hat eine Aufgabe aus einem Entscheid
(BM2 a, GB1 a), beide Familien liefern zugleich die Hörproben von BM1 a; eine Variante wird nur gegen ihre Grundlage
gehört, sonst hört man die Vereinfachungen mit. Verworfen: b (dazu drei Varianten auf dem echten Steinway — vorab
gerechnet, was vielleicht niemand will), c (nur Glas — streicht die Prüfung der Richtungen aus BM2 a). Kippt, wenn
MP1 klar hörbar anders klingt: dann Varianten auf dem echten Steinway als „Plan erweitern“ (PL31 a).

✅ **ZA1 a** (Patrik S149, Frage 12 aus PL31 a): **31c-9 Abschluss 31c als eigener Schritt nach 31c-8.** (1) Tests:
zuerst der Bereich `*Loop*,*Instrument*,*Shape*,*Palette*,*Drum*,*Wav*,*Publish*,*Import package*`,
`--source-file=*test_Composer*` und `*Klangfassung*` unter Release-clang und Testing-MSVC (damit auch der
ausstehende MSVC-Lauf von `*Loop Koerper*`), dann **der volle Lauf unter beiden Compilern** (offen seit Stufe 26),
Dauer gemessen und notiert. (2) Hörproben über `LUMIVIZ_31C_WAV=<Ordner>`, gesammelt für Patrik: je Körper A0, C2,
C4, C6, C8 mit mf, dazu C4 mit pp und ff, Stereo; Steinway ohne gegen mit Boden, die Familien aus VA1 a; mit einer
Liste, was jede Datei vergleicht. (3) Doku: `InstrumentBody.md`, Gebaut-Absätze, `THIRD_PARTY_NOTICES.md`
(pocketfft), README der Körper, `Offene_Punkte`, Steckbrief. Keine Sichttests (in 31c entsteht nichts auf dem
Bildschirm; die Bedienung kommt in 31g). Erst damit ist 31c fertig; danach die Planung von 31d–31h nach PL31 a. Grund:
ein Abschluss, der den vollen Lauf weiter schiebt, ist keiner; 31c ändert mitbenutzten Code (`.lvinst` 17,
Tonspeicher, Stereo der Hammer-Saite). Verworfen: b (voller Lauf erst am Ende von 31). Kippt, wenn der volle Lauf
unter Testing-MSVC über rund 2 h dauert: dann vorlegen, ihn unter MSVC am Ende von 31 zu fahren, unter clang jetzt.

**Befund S149 zu den ersten Moden (Corradi 2017 selbst gelesen, Tab. 2 S. 25, S. 6–11):** gemessen ist ein Fazioli
F278; unserem Modell (eingespannt, ohne Saiten) entspricht Stufe 3 („soundboard is assembled in the piano case by
gluing its edge on the internal rim“) — erste Moden 43,1, 46,0, 60,8, 65,7, 75,7 Hz; „34 modes up to 348 Hz … at stage
3“ (rund 0,11 /Hz); Corradis FE-Modell trifft Stufe 3 erst ab 59,3 Hz. Unser Steinway: erste Mode 38,2 Hz, Dichte
unter 500 Hz 0,064 /Hz (MAESSTRO Abb. 5 rund 0,05–0,07, Ege am Klavier rund 0,06). Der Bereich in BM1 a („43–60 Hz,
Abstände um 20 Hz“) war festgelegt, bevor Bauzustand und Flügel des Vergleichs geklärt waren (60 Hz ist bei Conklin
der Zustand mit Saiten; die Abstände haben keine Quelle).

✅ **EM1 a** (Patrik S149, Frage 14 aus PL31 a, beim Planen gefunden; fasst den Kipppunkt von BM1 a neu): **Die
ersten Moden werden gegen Corradi Stufe 3 (gleicher Bauzustand) und MAESSTRO Abb. 5 (gleicher Flügel, gerechnet)
geprüft. BM1 a kippt auf c, wenn (1) die erste Mode außerhalb 35–65 Hz liegt** (Corradi Stufe 3 43,1 Hz ±20 % für den
Unterschied der Flügel, nach oben bis zum Zustand mit Saiten) **oder (2) die Modendichte unter 500 Hz außerhalb
0,04–0,14 /Hz** (unter MAESSTRO bis über Corradi). Der Steinway (38,2 Hz, 0,064 /Hz) besteht beides; geprüft und
berichtet wird an den nachgerechneten Moden in 31c-6. Conklin nur als Bericht (Volltext nicht gesehen). Grund: ein
Kipppunkt soll anzeigen, dass das Modell daneben liegt, nicht dass ein Steinway nicht wie ein Fazioli schwingt.
Verworfen: b (BM1 a wörtlich — der Bereich steht auf falscher Grundlage), c (nur berichten — gibt einen Prüfstein
auf, den PL31c a will). Kippt, wenn die Moden aus 31c-6 (k = 4 nach KV1 a) außerhalb dieser Grenzen liegen (dann BM1 c
vorlegen), oder wenn Patrik eine engere Toleranz nennt.

### Fester Plan: IP1 und 31c-6 bis 31c-9 (PL31 a, S149)

Nummern fest; wächst etwas, als Entscheid „Plan erweitern“. Je Teilschritt beide Compiler bauen, eng testen unter
Release-clang (MT1 a); Testing-MSVC und die großen Läufe in 31c-9.

**IP1 — Fenster *Instruments* misst im Hintergrund** (Loop_Konzept §4n, IP1 a). Hintergrund-Thread für
`abstandZumBezug`, gemerkt unter `instrumentAbdruck` und `kKlangfassung`, kein doppeltes Lesen beim ersten Öffnen.
Prüfung: ein Fall belegt, dass das Öffnen nicht misst, die Spalte sich füllt und ein zweites Öffnen den gemerkten Wert
nimmt. Eng: `*Palette*,*Pegel*`. Eigener Commit.

**31c-6 — Prüfstein und Konvergenz.**
1. *Dämpfung je Werkstoff* (DM1 a): Anteil der Verformungsenergie je Werkstoff in `Auswertung`, `lossFactor` wirkt;
   der Anteil je Mode in den Körper (`kKoerperFassung` 2, Fassung 1 bleibt lesbar). Prüfung: zwei Werkstoffe an der
   Prüfplatte (Anteile summieren zu 1, η gegen Handrechnung), der Steinway mit allen 0 behält seine η.
2. *Mobilität* (PS1 a): Treiberpunkt nach MAESSTRO Gl. 10 aus den Moden, das geometrische Mittel 25–5000 Hz
   (Gl. 12) je Stegpunkt, Mittel je Terzband; im Werkzeug als Bericht. Prüfung gegen einen Einmassenschwinger und die
   Prüfplatte (geschlossene Lösung).
3. *Prüffall* (PF1 a): `make_steinway_d.py --maesstro` unter den Testdaten; Überlagerungsbild gegen Abb. 4 a.
4. *Rechnungen*: Prüffall (k = 3); Steinway neu (Fassung 2); Messlauf k = 4 im Fenster 0–500 Hz (über 20 GB oder 3 h
   hochgerechnet: Rückfrage); k = 4 bis 5 kHz (KV1 a).
5. *Prüfungen und Bericht*: PS1 a (1)–(3), KV1 a (1)–(3), EM1 a, Modendichte gegen Abb. 5; Ergebnisse als Befund
   hier, Kipppunkte von BM7 a, ST1 a, BM1 a ausgewertet. Eng: `*Loop Koerper*`.

**31c-7 — Klang.**
1. *Messteil* (AW1 a, FT1 a, PG1 a): Zeit und Speicher je Antwort, Rechenzeit der Faltung je Ton, Schalldruck am
   Hörort in dB SPL → Wert von `kSchalldruckBezug`; über 1 s je Antwort: Rückfrage.
2. *FFT* (FT1 a): pocketfft unter `externals/pocketfft` (Download mit Freigabe), Schnittstelle `Fft`, Tests gegen die
   direkte Faltung, `THIRD_PARTY_NOTICES.md`.
3. *Antworten und Faltung*: Antwort je Taste und Hörort im Tonspeicher (AW1 a), Faltung je Ton, echtes Stereo
   (FT1 a), `kSchalldruckBezug` (PG1 a).
4. *Datei* (SB1 a): `.lvinst` Fassung 17 mit `soundboard.body`, „Body not found“, Fassungs-Literale in `tests/`,
   `*Klangfassung*`.
5. *Hörproben*: `LUMIVIZ_31C_WAV`. Eng: `*Loop Hammer*,*Loop Koerper*,*Klangfassung*,*Instrument*`.

**31c-8 — Varianten** (VA1 a, GL1 a): Baupläne MP1–MP3 aus dem Prüffall (Testdaten) und Glas mit Dicke wie Holz
und auf gleiche Masse (mitgeliefert, `asset/Asset.cmake`, Wächter); fünf Rechnungen; Prüfung der Richtungen gegen
MAESSTRO Abb. 5 und S. 8. Eng: `*Loop Koerper*`.

**31c-9 — Abschluss 31c** (ZA1 a): Bereich und voller Lauf unter beiden Compilern, Hörproben gesammelt mit Liste,
Doku. Danach die Planung von 31d–31h nach PL31 a.

**Gebaut IP1 (S149):** siehe Loop_Konzept §4n.

**Gebaut 31c-6 Teil 1 — Dämpfung je Werkstoff (S149, DM1 a):** `InstrumentBody/Verformungsenergie.hpp` (U_m je
Werkstoff an denselben Integrationspunkten wie K), `Steife::werkstoff`, `Plattenelement::integrationsPunkte`,
`verlustfaktorAusAnteilen`, `ModeImKoerper::werkstoffAnteil`, im statistischen Teil das Mittel der Anteile;
`.lvinstbody` **Fassung 2** (`materials`, je Mode `materialShares`; Fassung 1 lesbar), `kRechenFassung` 2; das
Werkzeug gibt die Anteile der ersten Moden aus. **Steinway D neu gerechnet** (1233 s, Spitze 4,99 GB, Auswertung 39 s
statt 17 s) und gegen den alten Körper verglichen: Frequenzen, η, Stegwerte, Abstrahlung und Statistik **Bit für Bit
gleich**, neu nur die Anteile — Fichte im Mittel 94 %, Buche 6 % (bei 100,4 Hz 38 %); 3,0 MB. Tests in
`test_LoopKoerperAuswertung` (Mischung; Prüfplatte mit zwei Werkstoffen: 2 Σ U_m = ω² auf 10⁻⁶, erste Mode
0,4999 / 0,5001; statistischer Teil; Werkstoffe des Steinway) und `test_LoopKoerperDatei` (Fassung 2, Fassung 1
lesbar, falsche Zahl an Anteilen, Wächter mit Werkstoffen und η aus den Anteilen). Release-clang `*Loop Koerper*`
38 / 38; Testing-MSVC baut.

**Frage Patrik (S147), ohne Umsetzung: ließe sich daraus ein VST-Plugin machen?** Antwort (Claude, nicht gebaut):
grundsätzlich ja — der Körper ist nach BM3 a/BM5 a eine Datei plus Faltung, das passt in ein Plugin. Heute im Weg:
die Rechnung der Hammer-Saite (`loop/HammerTon`) hängt über `loop/HammerSaite` an Qt (`QJsonObject`, `QString`);
ein Plugin bräuchte den Kern wie `InstrumentBody` ohne Qt. Ein Plugin rechnet in Echtzeit auf MIDI, LumiViz rechnet
Töne vorab in den Tonspeicher; die gemessene Rechenzeit (31b-3: 2,3 % der Echtzeit je Ton im Bass) spräche nicht
dagegen, Pedal und viele Stimmen sind ungemessen. Lizenz des VST3-SDK und Alternativen (CLAP) ungeprüft.

### 4.2 Befund: wann ein Ton gerechnet wird (Code S141, gelesen)

Nicht einmal beim Laden. **Im Loop-Panel** rechnet `Spieler::ladeProgramm` erst beim Play
(`Spieler.cpp:49`, `setzeLoop` rechnet nur, wenn er spielt); während er spielt, prüft jede
Bearbeitung die Liste der Tonnamen und rechnet **die ganze Liste** neu, sobald sich ein Name ändert
(`Spieler.cpp:62`) — einen Speicher je Tonname gibt es nicht, nur die dekodierten Samples sind
gemerkt. **Im Composer** bekommt jeder Loop-Clip seine eigene `LoopQuelle` und rechnet rund zwei
Sekunden vor seinem Beginn (`Tonfuehrung.cpp:362-366`, `kVorlaufMs = 2000`); zwei Clips desselben
Loops teilen nichts, eine wieder gebrauchte Quelle rechnet neu. **Alles auf dem Thread der
Oberfläche**, synchron (kein `QtConcurrent`/`std::thread` unter `loop/`). Bisher genügte das, weil
die Töne in Millisekunden entstehen (16 Töne einer Gitarre 20,8 ms, Release-clang, S141). Ein
Flügel nach Zielbild — 88 Töne, Saitenchöre, Resonanzboden, Mitschwingen, mit Pedal Töne je
Vorkommen — wird um Größenordnungen teurer; dann stockt die Oberfläche beim Play und vor jedem
Clip.

✅ **H1 a** (Patrik S141): **Vor dem Klavier eine eigene Stufe „Töne vorab“.** Ein Speicher je
Tonname für das ganze Programm, mit einer Obergrenze, die das Älteste verwirft; Loop-Panel,
Composer und alle Clips desselben Loops teilen ihn, eine Bearbeitung rechnet nur die neuen Namen.
Gerechnet wird im Hintergrund auf mehreren Kernen, nicht auf dem Thread der Oberfläche. Angestoßen
schon beim Laden eines Loops und beim Laden einer Komposition (für alle Loop-Clips) — Patriks
Vorschlag. Was geschieht, wenn ein Ton beim Abspielen noch nicht fertig ist: Folgefrage. Verworfen:
b (nur der Speicher je Tonname — das erste Play und der Composer stockten weiter, Hintergrund und
Vorladen kämen bei der nächsten Familie ohnehin), c (nichts ändern, den Flügel sparsam bauen —
gegen das Zielbild). Kippt, wenn die erste Messung eines Flügel-Modells zeigt, dass ein Ton nur
Millisekunden braucht — dann reicht b.

✅ **H2 a** (Patrik S141): **Play beginnt erst, wenn alle Töne fertig sind** — im Loop-Panel wie im
Composer; bis dahin „Preparing sounds… 12 / 40“, Stop bricht ab. Eine Bearbeitung während des
Spielens lässt die bisherigen Töne klingen und schaltet um, sobald die neuen fertig sind — es fehlt
nie ein Ton, die Änderung wird nur einen Moment später hörbar. Tritt der Fall doch ein (ein Sprung
im Composer zu einem Clip, dessen Töne der Speicher verworfen hat), bleiben nur die fehlenden Noten
still, und die Zeile sagt es. Verworfen: b (sofort spielen, fehlende Noten still — Lücken, die
nach Fehler klingen; eine Hörprobe wäre nicht verlässlich), c (sofort spielen mit schnellem Ersatz —
täuscht über den Klang). Kippt, wenn das Warten nach dem Öffnen eines Loops im Alltag spürbar dauert
(über zwei Sekunden) — dann b als Schalter daneben prüfen.

✅ **H3 a** (Patrik S141): **Der Speicher liegt im Arbeitsspeicher**, die Obergrenze in den Settings
einstellbar, Vorgabe 1 GB; ist sie erreicht, geht der am längsten nicht gebrauchte Ton. Nach einem
Neustart wird neu gerechnet. *Größen:* eine Sekunde Stereo bei 48 kHz als Gleitkomma rund 384 KB,
ein Klavierton mit 10 s Ausklang fast 4 MB, die Höchstlänge 30 s rund 11,5 MB. Verworfen: b (dazu
ein Speicher auf der Platte mit Schlüssel aus Name, Instrument-Datei und Fassung des Rechenkerns —
kann veralten: ändert sich der Rechenkern, ohne dass die Fassung mitwächst, kämen alte Töne still
zurück und verfälschten Hörproben), c (feste Grenze — passt nicht für jeden Rechner). Kippt, wenn
das Öffnen eines Loops nach einem Neustart am Flügel spürbar dauert — dann b als eigene Stufe, mit
der Fassung des Rechenkerns im Schlüssel und einem Test, der sichert, dass sie mitwächst.

**Vorgemerkt (Patrik S141): Compute-Shader für den Fall der Fälle.** Frage Patrik, ob zur Rechnung
CUDA dienen kann. Antwort Claude: Die heutige Saite rechnet Abtastwert für Abtastwert und lässt sich
kaum verteilen; parallel liegen Resonanzboden (viele Schwingungsformen), Mitschwingen (bis rund 230
gekoppelte Saiten) und eine Saite als Gitter (Finite Differenzen). CUDA liefe nur auf NVIDIA (die
4090, nicht die Radeon 610M), bräuchte das Toolkit im Bau und rechnet auf Spielkarten in doppelter
Genauigkeit langsam. Falls die CPU nach der Messung am Flügel nicht reicht: **Compute-Shader über
OpenGL** (schon im Programm, NVIDIA und AMD), dann als Entscheid mit Messwerten.

### 4.3 Idee Patrik (S141): rechenintensive Clips vorrechnen und als Wellenform abspielen

Wortlaut Patrik: „was, wenn rechenintensive Loop-Clips vorgerechnet werden und als Waveform
abgespeichert werden, um diese alternativ abzuspielen? Dass Waveforms auch im Loop eingefügt werden
können, ist sowieso geplant. Das Waveform wird mittels Hash für den ganzen Clip inklusive aller
Modifikationen gespeichert, so ist klar, wann das Waveform neu gerechnet werden muss. (Kann auch für
andere Instrumente außer Flügel sinnvoll sein, aber evtl. gibt es einen Indikatorwert, um zu zeigen,
wo es sinnvoll wäre?) Die Option kann über das Kontextmenü eines Clips aktiviert werden.“

*Einordnung Claude:* in Audioprogrammen als *Freeze* bekannt. Es ergänzt H1 a, ersetzt es nicht —
wer bearbeitet, braucht die Töne einzeln (H1 a); wer nur abspielt, nimmt die fertige Wellenform.
Anschluss an den vorgemerkten **Audio-Clip im Loop** (`Loop_Konzept.md` §3.2): das Abspielen einer
Wellenform im Loop wäre derselbe Weg. Der Hash löst die Frage „wann neu rechnen“ für alle
**Eingaben** (Muster, Instrument-Dateien, Samples, Kit, Clip mit Effekten und Kurve, Tempo, Swing);
was er von selbst nicht sieht, ist eine **Änderung am Rechenkern** — dieselbe Gefahr wie bei H3 b.
Abhilfe: eine *Klangfassung* des Programms im Hash und ein Test, der rot wird, wenn
`loop_werte.txt` sich ändert, ohne dass die Klangfassung mitwächst. Offene Fragen, einzeln: F1 die
Form, danach was in die Wellenform gehört (Effekte, Kurve, Kanal-Fader?), der Indikator, der Ort der
Dateien und die Reihenfolge der Stufen.

✅ **F1 a** (Patrik S141): **Freeze als Eigenschaft des Clips.** Im Kontextmenü „Freeze clip“ /
„Unfreeze clip“; ein eingefrorener Clip spielt seine Wellenform, als Datei mit dem Hash als Namen
(Hash aus allen Eingaben und der Klangfassung); gleiche Clips teilen eine Datei; eine Marke am Clip
zeigt eingefroren oder „wird gerechnet“. Er bleibt bearbeitbar. *Mit dem Einwand Patrik:* „es sollte
doch nicht gleich das Wave neu gerechnet resp. überschrieben werden, wenn man den Clip überarbeitet —
das wäre too much, solange man das noch rückgängig machen kann.“ Überschrieben wird nie (der Name
ist der Hash: ein neuer Stand ist eine neue Datei; Undo findet die alte wieder). Wann nach einer
Bearbeitung neu gerechnet wird und was bis dahin spielt: F2. Verworfen: b („Bounce to audio“ —
getrennt vom Muster, ein vergessener veralteter Audio-Clip wäre ein stiller Fehler), c (beides —
*Bounce* hängt am Audio-Clip im Loop, `Loop_Konzept.md` §3.2, und wird dort entschieden).

✅ **F2 a** (Patrik S141): **Neu eingefroren wird beim Speichern des Loops.** Wird ein eingefrorener
Clip bearbeitet, spielt er bis zum Speichern live (die Töne aus H1 a, nur neue werden gerechnet);
die Marke zeigt „frozen · changed“. Beim Speichern wird der neue Stand im Hintergrund eingefroren,
bis dahin spielt er live weiter. Macht ein Undo den Stand wieder gleich dem eingefrorenen, gilt
dessen Datei sofort. Grund: der Composer spielt Loops aus der gespeicherten Datei — die Wellenform
passt dann genau zu dem, was er liest; Speichern ist der Moment, in dem ein Stand gilt. Verworfen: b
(nach einer Pause ohne Bearbeitung — rechnete mitten im Bearbeiten, die Datei passte zu einem
ungespeicherten Stand), c (nur von Hand „Refreeze clip“ — ein vergessenes Refreeze ließe den
Composer still live rechnen). Kippt, wenn lange ohne Speichern bearbeitet wird und live zu langsam
spielt — dann b als Schalter dazu.

✅ **F3 a** (Patrik S141): **In die Wellenform gehört alles bis zum Ausgang des Clips:** Noten,
Lautstärke und Panorama des Musters, Effekte am Clip, Kurve am Clip, über alle Wiederholungen, mit
dem **Nachklang** hinter dem Ende (ausklingende Töne, Hall, Echo). Live bleiben Fader und Schalter
des Kanals, Lautstärke des Loops und Limiter — Mischen ändert den Hash nicht. Die Grenze liegt, wo
heute der Bus eines Clips endet. Verworfen: b (nur die Noten, Effekte und Kurve live — Hall und Echo
kosten beim Spielen, gerade die soll Freeze sparen; Effekte verstellen ist Bearbeiten, dafür gilt
F2 a), c (auch Kanal-Fader und Lautstärke des Loops — ein Fader-Zug ist Mischen, keine Bearbeitung).
Kippt, wenn Clip-Effekte oft im Zusammenspiel abgestimmt werden, während der Clip eingefroren
bleibt — dann b.

✅ **F4 a** (Patrik S141): **Gemessen, als Zeichen am Clip.** Beim Rechnen der Töne (H1 a) misst das
Programm je Clip, wie lange seine Töne brauchen, auf diesem Rechner. Ab einer Schwelle (Vorgabe 1 s,
in den Settings) trägt der Clip im Loop-Panel ein kleines Zeichen; der Tooltip etwa „Computing this
clip takes 3.2 s — Freeze clip to start it at once“; im Kontextmenü „Freeze clip (3.2 s to
compute)“. Vor dem ersten Rechnen keine Zahl. Die Schwelle liegt unter den 2 s, die H2 a spürbar
nennt. Verworfen: b (geschätzt aus Kostenwerten je Schicht — irrt bei jedem neuen Modell und bei
Tönen je Vorkommen, gerade am Flügel), c (nur die Zahl im Menü — man müsste suchen). Kippt nicht auf
b, wenn die Messung schwankt (Drosseln unter Hitze, S107): dann der kleinste von mehreren
Messwerten.

✅ **F5 a** (Patrik S141), **mit Zusatz: bei Publish mit exportiert.** Die Wellenformen liegen in
einem Ordner im Benutzerbereich des Programms (unter AppData, neben eigenen Instrumenten und
Samples, etwa `frozen`), je Datei eine WAV mit 32-bit Gleitkomma. Sie sind ein Speicher, kein
Werk — jederzeit neu zu rechnen —, deshalb nicht im Export der eigenen Daten (Datensicherung).
Grenze in den Settings (Vorgabe 2 GB), danach gehen die am längsten nicht gespielten; dazu „Clear
frozen clips“. Die Marke „frozen“ steht in der `.lvloop` am Clip; fehlt die Datei, wird sie neu
gerechnet. *Zusatz Patrik:* **Publish nimmt die Wellenformen eines Pakets mit**, damit der Empfänger
nicht wartet; er nutzt sie nur, wo der Hash (mit der Klangfassung) bei ihm derselbe ist — sonst
rechnet er neu. Verworfen: b (neben der `.lvloop` in `<name>.frozen` — Dateien in Patriks Ordnern,
die keiner bestellt hat; bei mitgelieferten Loops nicht möglich), c (in der `.lvloop` eingebettet —
die Datei würde viele MB groß, jedes Speichern schriebe alles).

### 4.4 Bauplan (F6 a)

✅ **F6 a** (Patrik S141): **Erst das Fundament, dann der Flügel.**

| Stufe | Inhalt |
|---|---|
| 29 | **Töne vorab** (H1 a bis H3 a): Speicher je Tonname im Arbeitsspeicher (Grenze in den Settings, 1 GB), Rechnen im Hintergrund auf mehreren Kernen, Vorladen beim Öffnen eines Loops und einer Komposition, „Preparing sounds… n / m“, eine Bearbeitung schaltet um, sobald die neuen Töne fertig sind. |
| 30 | **Freeze** (F1 a bis F5 a): „Freeze clip“ / „Unfreeze clip“ im Kontextmenü, Hash aus allen Eingaben und der Klangfassung mit dem Test an `loop_werte.txt`, Wellenform bis zum Ausgang des Clips mit Nachklang, neu eingefroren beim Speichern, Messung je Clip mit Zeichen ab 1 s, Ordner `frozen` mit Grenze 2 GB und „Clear frozen clips“, Publish nimmt die Wellenformen mit. Die eingefrorene Wellenform spielt im Rechenkern wie ein langer Klang ab dem Beginn des Clips — der Audio-Clip im Loop (`Loop_Konzept.md` §3.2) ist dafür nicht nötig. |
| 31 ff. | **Hammer-Saite** (Zielbild §4.1): Recherche, Entscheide, Skizze, Modell, Messung gegen *Salamander Grand Piano*; zuerst Flügel und Upright, danach die übrigen Instrumente der Familie. Die Recherche kann schon während 30 beginnen (Dokumentarbeit, hängt an nichts davon). In S142 hinter den Audio-Clip gerückt (32 ff.), **seit S144 wieder Stufe 31** (Patrik: „tausche 31 und 32“). |
| danach | **Audio-Clip im Loop** (Patrik S142, vorgemerkt; `Loop_Konzept.md` §3.2): die dritte Clip-Art neben Drumkit und Instrument, ein Clip mit einer Wellenform (Samples, Gesang …) und einer Autotune-Spur. Beginnt mit eigenen Entscheiden und einer Skizze — Ort der Spur, Datei, Bearbeitung der Wellenform, Autotune-Spur und Spur Pitch, *Bounce* (F1 c). Das Abspielen einer Wellenform im Rechenkern bringt Stufe 30 mit. Seit S144 hinter der Hammer-Saite (Stufe 31 ff.); die Nummer bekommt er, wenn er an der Reihe ist. |

Verworfen: b (Töne vorab, dann die Hammer-Saite, Freeze erst danach — die Werte, die sich am Flügel
prüfen ließen, stehen ohnehin in den Settings und kippen das Gebaute nicht), c (zuerst die
Hammer-Saite — gegen H1 a „vor dem Klavier“). Kippt, wenn der Flügel früher zu hören sein soll, als
29 und 30 dauern — dann rückt die Hammer-Saite hinter 29 vor.

**Idee Patrik (S142):** Im Kontextmenü von Clips — Instrument- und Drumkit-Clips im Loop-Panel, der
Loop-Clip im Composer — einen Clip als Wellenform exportieren. Im Code gab es dafür nichts (gesucht in
`loop/` und `composer/` nach Export, Bounce, Render).

✅ **WX1 a** (Patrik S142): **Stufe 30 beginnt mit der Rechnung „ein Clip als Wellenform“; darauf
zuerst der Export, dann Freeze.** 30a die Rechnung (nach F3 a), 30b „Export audio…“ im Kontextmenü
der drei Clip-Arten mit einem Dialog zum Speichern, 30c ff. Freeze wie entschieden. Einzeln danach:
WX2 das Format, WX3 ob der Loop-Clip im Composer den Loop mit seinem Mix (Fader, Limiter) exportiert
oder nur bis zum Ausgang des Clips wie Freeze. Der Export bringt **keine dritte Clip-Art** in den
Loop: der Audio-Clip (`Loop_Konzept.md` §3.2) bleibt ein eigenes Thema. Verworfen: b (eigene kleine
Stufe vor Freeze mit eigener Rechnung — Freeze müsste sie übernehmen oder doppeln), c (nach Freeze —
nichts spricht dafür außer der Reihenfolge der Ideen). Kippt, wenn der Export vor 29c bis 29f gebraucht
wird — dann rücken 30a und 30b hinter 29b vor, die Rechnung bleibt eine.

✅ **WX2 a** (Patrik S143): **WAV mit Wahl der Tiefe.** *Export audio…* schreibt WAV, Stereo, mit der
Rate, mit der der Loop spielt. Der Dialog zum Speichern bietet *Format*: **32-bit float** (Vorgabe,
wie Freeze, Spitzen über 0 dB bleiben erhalten), **24-bit**, **16-bit** (mit Dither); gemerkt wird die
zuletzt gewählte Art. Schneiden 24 oder 16 Bit Spitzen über 0 dB ab, sagt es die Rückmeldung nach dem
Export. Grund: ein Export ist ein Zwischenstand, der verlustfrei weitergeht — 32-bit float ist
dieselbe Datei wie bei Freeze (eine Rechnung, WX1 a); 24 und 16 Bit sind die üblichen Lieferformate.
Heute schreibt `schreibeWav` (`loop/Aufnahme`) nur 16 Bit — 30a erweitert den Schreiber ohnehin.
Verworfen: b (nur 32-bit float — manches ältere Programm liest es nicht), c (dazu FLAC, MP3, OGG über
BASSenc — vier weitere proprietäre Bibliotheken als Pflicht für Bau und Weitergabe; liegen unter
`externals/bass/`, sind nicht eingebunden). Kippt, wenn exportierte Clips direkt verschickt oder
veröffentlicht werden sollen — dann c oder FLAC über das freie libFLAC als eigener Entscheid.

✅ **WX3 a** (Patrik S143), **mit Zusatz: ohne die Pegeländerungen des Composers.** *Export audio…*
am Loop-Clip im Composer schreibt den Clip über **seine Länge und seinen Versatz**, mit allen
Wiederholungen, mit den **Fadern und Schaltern der Kanäle des Loops** — ohne die anderen Kanäle der
Komposition und ohne ihren Limiter. Die Datei endet, wo der Clip im Composer endet. *Zusatz Patrik:*
„ohne die fade in fade out und sonstigen pegeländerungen im composer“ — also **ohne Gain, Blenden und
Pegelkurve** des Composer-Clips. Grund: der Export gibt den Ausschnitt des Loops, den der Clip
spielt; was der Composer am Pegel daraus macht, ist Mischen der Komposition (wie Fader und Limiter
bei Freeze, F3 a). Verworfen: b (der Loop, wie das Loop-Panel ihn spielt, ein Durchlauf mit dem
Limiter des Loops — den Loop als Ganzes exportiert man sauberer im Panel Loop). Kippt, wenn der Loop
ohne Composer als Datei gebraucht wird — dann ein eigener Eintrag im Panel Loop (etwa *Export loop
audio…*), WX3 bleibt.

✅ **WX4 a** (Patrik S143): **Stufe 30 von innen nach außen.**

| Teilschritt | Inhalt |
|---|---|
| 30a | **Die Rechnung**, ohne Oberfläche und ohne Engine. *Clip im Loop-Panel:* nur dieser Clip über alle Wiederholungen, mit seinen Effekten und der Kurve am Clip, ohne Kanal-Fader (F3 a), mit dem Nachklang hinter dem Ende, ohne dass der Durchlauf umspringt — er endet, wenn die Summe 0,5 s lang unter −96 dBFS bleibt, höchstens 30 s nach dem Ende (Werte von Claude gesetzt). *Loop-Clip im Composer:* der Ausschnitt über Versatz und Länge, mit Fadern und Schaltern des Loops (WX3 a), ohne Nachklang. Gerechnet mit dem `Klangerzeuger` selbst — derselbe Weg wie live. Tests: Bit für Bit wie live an derselben Stelle, der Nachklang endet in Stille, Rechenzeit. |
| 30b | **Der WAV-Schreiber:** 32-bit float, 24 und 16 Bit mit Dither, Meldung bei Kappung. Tests: zurücklesen mit dem eigenen Leser und mit der BassEngine. |
| 30c | **Export audio…:** zuerst die Skizze des Dialogs mit *Format*; dann der Eintrag im Kontextmenü am Drumkit- und Instrument-Clip im Loop-Panel und am Loop-Clip im Composer, Rechnen im Hintergrund mit Fortschritt, Rückmeldung, Handbuch, Sichttest-Liste. |
| 30d ff. | **Freeze** (F1 a bis F5 a); der Plan der Teilschritte als eigene Vorlage nach 30c. |

Verworfen: b (30a und 30b in einem — ein roter Test hätte zwei mögliche Ursachen, wie bei TV1 a).
Kippt, wenn Freeze vor dem Export kommen soll — dann 30a, 30b, danach Freeze als 30c.

✅ **FZ1 a** (Patrik S143): **Freeze von innen nach außen, sechs Teilschritte.**

| Teilschritt | Inhalt |
|---|---|
| 30d | **Hash und Klangfassung**, ohne Oberfläche: `clipAbdruck` über alles, was in den Clip eingeht (Muster, Instrumente mit ihren Dateien, Samples, Kit, Effekte und Kurve am Clip, Tempo, Swing, Raster, Rate, Klangfassung) — nicht Fader, Kanal-Schalter, Lautstärke des Loops, Limiter (F3 a, WX5 a). Ein Test wird rot, wenn sich `loop_werte.txt` ändert, ohne dass die Klangfassung mitwächst. |
| 30e | **Die Dateien:** Ordner `frozen` im Benutzerbereich, je Hash eine WAV 32-bit float (`audio/WavDatei`), Grenze 2 GB, die am längsten nicht gespielten gehen; in den Settings eine Zeile wie *Loop Sound Memory* mit **Clear frozen clips** (vorher Skizze). |
| 30f | **Abspielen:** `.lvloop` Feld `frozen` am Clip (immer geschrieben, Fassung 13); liegt die Datei zum Hash vor, spielt der Rechenkern sie als einen langen Klang ab dem Clip, Fader und Lautstärke des Loops live; sonst live. Der Composer ebenso. |
| 30g | **Bedienung:** Skizze, dann *Freeze clip* / *Unfreeze clip*, die Marke („frozen“, „computing…“, „frozen · changed“), neu eingefroren beim Speichern im Hintergrund (F2 a), Undo findet die alte Datei. |
| 30h | **Messung (F4 a):** Rechenzeit je Clip aus dem Tonspeicher, Zeichen ab der Schwelle (Settings, Vorgabe 1 s), Tooltip und Menütext. |
| 30i | **Publish und Abschluss:** ein Paket nimmt die Wellenformen mit, der Empfänger nutzt sie nur bei gleichem Hash; Handbuch, Sichttest-Liste, Bereichslauf. |

Grund: jede Stufe steht auf etwas Geprüftem — die Bedienung braucht Dateien, die sich abspielen lassen;
der Hash mit der Klangfassung ist die heikelste Stelle (F1 a, H3 b) und kommt zuerst und allein.
Verworfen: b (die Bedienung gleich nach dem Hash — ein Menü ohne Wirkung oder vorläufige Wege). Kippt,
wenn das Menü früh im Sichttest stehen soll — dann b mit grauem *Freeze clip* bis 30f.

**Gebaut 30d (S143):** `loop/Freeze` — `kKlangfassung` (1), `abdruckFassung` (der Clip allein auf
Takt 0 über `clipFassung`, ohne Tags, Namen von Kanal und Muster, Kit-Nennung; Limiter fest) und
`clipAbdruck` (SHA-1 über Klangfassung, Rate, die Fassung als JSON und je Klang seinen Schlüssel im
Tonspeicher — `instrumentAbdruck` bzw. `klangSchluessel`). Gleiche Clips an anderer Stelle oder unter
anderem Namen teilen einen Abdruck; Fader, Schalter, Lautstärke des Loops, Limiter, Tags und andere
Kanäle ändern ihn nicht. Hinweis: wie im Tonspeicher stehen in den Schlüsseln Ort, Größe und
Änderungszeit der Dateien — ein Auschecken oder Kopieren friert neu ein, schadet aber nicht. *(Durch
FZ2 a ersetzt, siehe unten.)*
**Der Wächter der Klangfassung** (`test_LoopFreeze`, `werte/klangfassung.txt`): je Eintrag von
`loop_werte.txt` ein Abdruck seiner Eingaben **aus dem Inhalt** der Dateien (Instrument mit Samples
und Zeichnungen, Loop-Datei mit Instrumenten und Klangdateien) und seine Werte; rot nur, wenn gleiche
Eingaben andere Werte haben — eine geänderte oder neue mitgelieferte Datei verlangt keine neue Fassung.
Probeweise ein Wert verfälscht: rot mit dem Namen des Eintrags. Grenze: `loop_werte.txt` misst auf ein
Tausendstel — eine Änderung darunter (wie WX5 a an `effects_demo`) sieht der Wächter nicht.
`LUMIVIZ_KLANGFASSUNG_SCHREIBEN=1` schreibt die Datei neu. Tests in `test_LoopFreeze` (drei Fälle: der
Wächter, was den Abdruck ändert und was nicht, geänderte Instrument- und Klangdatei).

**Befund (S143):** Gebaut wie oben, gingen Dateien mit Ort, Größe und Änderungszeit in den Abdruck —
ein Paket aus Publish passte beim Empfänger nie (andere Pfade; gegen den Zusatz zu F5 a), schon ein
Auschecken fror alles neu ein.

✅ **FZ2 a** (Patrik S143): **Dateien gehen über ihren Inhalt in den Abdruck** (SHA-1 der Bytes), nicht
über Ort und Zeit; das Programm merkt sich je Datei den Inhalts-Hash zu Ort, Größe und Änderungszeit
und liest nur neu, wenn sich davon etwas geändert hat. *Umsetzung (Claude):* jeder **Verweis** auf eine
Datei wird im Abdruck durch den Hash ihres Inhalts ersetzt — in der Fassung des Clips (Klang einer
Spur, Instrument eines Musters oder einer Spur), in den Tonnamen (das Instrument darin) und in den
Instrumenten selbst (Sample einer Schicht, Zeichnung einer Shape-Schicht). Grund: der Abdruck soll
sagen, ob der Klang derselbe ist — der hängt am Inhalt; nur so geht der Zusatz zu F5 a auf, und
„gleiche Clips teilen eine Datei“ (F1 a) gilt über zwei Rechner. Verworfen: b (wie im Tonspeicher mit
Ort und Zeit — Publish nützte nichts, ein Auschecken fröre neu ein). Kippt, wenn Publish die
Wellenformen doch nicht mitnehmen soll.

✅ **FZ3 a** (Patrik S143, nach Skizze): **Die Zeile *Frozen Clips* wie *Loop Sound Memory*, gleich
darunter** (Settings → Panels): „**Frozen Clips:** [0.25 bis 256 GB, Schritt 0.25, Vorgabe 2.00 GB] ·
*on disk: 0 MB · 0 clips* · [**Clear frozen clips**]“; ein getippter Wert gilt mit Enter, eine kleinere
Grenze sofort; der Tooltip nennt den Ort (Benutzerordner, `frozen`), dass sie jederzeit neu gerechnet
werden können und nicht in *Export...* der eigenen Daten gehen; *Clear frozen clips* fragt nach.
Grund: dieselbe Art Speicher wie die Zeile darüber — Grenze und Belegung auf einen Blick, dieselben
Gesten. Verworfen: b (dazu *Open folder* — die Dateien heißen nach ihrem Hash, von Hand ist darin
nichts zu finden; *User Data: Open Folder* gibt es schon). Kippt, wenn die Wellenformen außerhalb von
LumiViz gebraucht werden — dafür ist *Export audio...* da.

**Gebaut 30e (S143):** in `loop/Freeze` der Ordner `frozen` (`frozenOrdner`, für Tests
`setzeFrozenOrdnerFuerTests`), `frozenDatei` (nur 40 Hex-Zeichen und `.wav` gehören dazu — fremde
Dateien im Ordner bleiben unberührt), `legeFrozen` (32-bit float über `audio/WavDatei`, danach die
Grenze), `holeFrozen` (nur in der passenden Rate; setzt die Änderungszeit — sie ist der letzte
Gebrauch), `frozenStand`, `leereFrozen`, `halteFrozenGrenze` (die am längsten nicht gespielten zuerst,
auch die zuletzt gelegte, wenn sie allein darüber liegt), `gemerkteFrozenGrenzeMb` /
`merkeFrozenGrenzeMb` (`loop/frozenMB`, 256 MB bis 256 GB). Die Datensicherung nimmt feste Bereiche
(`Datensicherung.cpp`), `frozen` ist keiner. In den Settings die Zeile *Frozen Clips* nach FZ3 a
(`loopFrozenLimit`, `loopFrozenOnDisk`, `loopFrozenClear`; die Rückfrage `loopFrozenClearQuestion`
über `lumi::ui::fuehreDialogAus`). Handbuch 1.201.0 (§8). Tests in `test_LoopFreeze` (zwei Fälle
mehr: die Dateien mit Grenze nach letztem Gebrauch, die Zeile mit Rückfrage).

**Plan 30f (S144, Claude):** drei Teilschritte. **30f-1** Modell und Datei: Feld `frozen` am Clip
(der Abdruck des eingefrorenen Stands, FV2 a), `.lvloop` Fassung 13, die Literale in `tests/`, `lanes_demo` über
`make_spuren_loops.py`. **30f-2** Rechenkern: `klangNamen`, `plane` und `baueProgramm` bekommen wie
`SaitenVon` eine Angabe, welche Clips eingefroren sind und unter welchem Abdruck; ein eingefrorener
Clip ist **eine** Stimme `frozen:<abdruck>` mit einem Schlag je Durchlauf am Anfang des Clips, ohne
Bus und ohne Expression (beides steckt in der Wellenform), Pegel Fader mal Lautstärke des Loops, ein
ausgeschalteter Kanal schweigt; Test gegen live (wegen der Reihenfolge der Faktoren auf etwa 1e-6,
nicht Bit für Bit). **30f-3** Laden: `eingefrorenVon(loop, rate, saiten)` (Clips mit Marke, deren
Datei vorliegt), der `Tonauftrag` lädt `frozen:`-Namen über `holeFrozen` in den Tonspeicher; Spieler,
Tonführung, Sitzung und ClipLautheit nutzen es; fehlt die Datei beim Laden doch, eine Zeile im Bericht
und das nächste Programm spielt den Clip live. *Export audio…* rechnet weiter live.

✅ **FV1 a** (Patrik S144): **Ein eingefrorener Clip schneidet sich nicht selbst ab:** sein Nachklang
klingt unter dem nächsten Durchlauf zu Ende, wie Hall und Echo live über die Naht klingen. Der
Rechenkern schaut dafür so viele Durchläufe zurück, wie die Wellenform lang ist. Grund: F3 a nimmt den
Nachklang in die Wellenform, damit Hall und Echo beim Spielen nichts kosten — mit dem Schnitt bräche
gerade er bei jedem Wiedereinsatz hart ab. Verworfen: b (eine Stimme wie jede andere, der alte
Nachklang endet am Wiedereinsatz mit der Blende von 64 Abtastwerten — näher an live für Töne, die
sich selbst abschneiden, aber eine deutliche Kante im Hall). Ohne eingefrorene Clips bleibt `rechne`
Bit für Bit, wie es ist. Kippt, wenn Freeze vor allem trockene Drums ohne Effekte trifft — dann ist b
gleichwertig und näher an live.

✅ **FV2 a** (Patrik S144): **`frozen` trägt den Abdruck des Stands, der beim letzten Speichern
eingefroren wurde** (40 Hex-Zeichen); leer = nicht eingefroren. Gespeichert wird immer der jetzige
Abdruck, im Hintergrund entsteht seine Datei. Gespielt wird die Datei zum **jetzigen** Abdruck, sonst
live. Weicht der jetzige vom gespeicherten ab, ist der Clip seit dem Einfrieren verändert („frozen ·
changed“, 30g); fehlt die Datei zum gespeicherten, wird sie neu gerechnet. Grund: der gespeicherte
Abdruck hält fest, welcher Stand eingefroren wurde — das lässt sich später nicht ausrechnen; nur damit
sind „bearbeitet“, „von der Grenze gelöscht“ und „noch nicht gerechnet“ zu unterscheiden, und Publish
findet seine Wellenformen ohne zu rechnen (30i). Er ist ein Protokoll des letzten Einfrierens, kein
Doppel des jetzigen. Ein geändertes Instrument oder eine neue Klangfassung gilt ebenfalls als
„verändert“ — zu Recht, der Klang ist ein anderer. Verworfen: b (ein Schalter `true`/`false` — ob der
Clip verändert wurde oder seine Datei nur fehlt, sagte die Datei nicht). Kippt, wenn die Marke nur
„frozen“ und „computing…“ zeigen soll und Publish den Abdruck ohnehin rechnet — dann b.

**Gebaut 30f-1 (S144):** `LoopClip::frozen` (der Abdruck als Text, leer = nicht eingefroren), `.lvloop`
Fassung 13 mit `frozen` an jedem Clip, auch leer; eine ältere Datei liest sich als nicht eingefroren;
`Loop::richte` leert alles, was nicht 40 Zeichen `0-9a-f` ist, mit „Frozen mark not valid“ im Bericht;
`abdruckFassung` leert die Marke — das Einfrieren ändert den Abdruck nicht. Abweichung vom Plan:
`lanes_demo` bleibt eine Datei der Fassung 12 (sie trägt alle Felder ihrer Fassung; eine Fassungsnummer
ist eine Zusage über die ganze Datei), `make_spuren_loops.py` unverändert. Literale in neun Testdateien
auf 13; Test in `test_LoopFreeze` (Datei, Fassung 12 ohne Feld, Richten, Abdruck). Eng unter Release-clang
und Testing-MSVC 73 / 73, die App unter clang gebaut.

**Gebaut 30f-2 (S144):** `Eingefroren` (je Clip — Kanal, Starttakt — der Abdruck, dessen Datei
vorliegt; `loop/Loop.hpp`), `frozenKlang`/`abdruckVonKlang` (`frozen:<abdruck>`). `klangNamen`, `plane`
und `baueProgramm` nehmen die Angabe wie `SaitenVon`: ein eingefrorener Clip nennt seine Wellenform
(hinten, in der Reihenfolge der Clips) statt seiner Töne; ein Muster, das nur in eingefrorenen Clips
liegt, nennt keine Klänge mehr (eines ohne Clip wie bisher). Im Ablauf eine Stimme je eingefrorenem Clip:
ein Schlag je Durchlauf am Anfang des Clips, Pegel Fader mal Lautstärke des Loops, kein Bus, keine
Expression, `Stimme::schneidet` aus (FV1 a) — `rechne` beginnt dann so viele Durchläufe zurück, wie
der Klang reicht. Ohne Angabe Bit für Bit wie vorher (`Loop Werte` unverändert). Tests in
`test_LoopFreezeSpielen` (Namen; Rechenkern mit einer 5-s-Wellenform über 2-s-Durchläufe, drei
zugleich; gegen live am Echo-Clip von `effects_demo`: größte Abweichung 6·10⁻⁶ bei Spitze 0,56). Eng
(Klang mit `Loop Werte`) unter Release-clang und Testing-MSVC 73 / 73.

**Gebaut 30f-3 (S144):** in `loop/Freeze` `eingefrorenVon` (je Clip mit Marke der **jetzige**
Abdruck, wenn seine Datei vorliegt — sie zählt dann als gebraucht), `vergissFrozen` (eine unlesbare
Datei geht; bis sie neu abgelegt ist, spielt ihr Clip live) und `frozenFehlt`. Der `Tonauftrag` lädt
`frozen:<abdruck>` über `holeFrozen` in den Tonspeicher (Schlüssel Name und Rate — eine Datei mit
diesem Namen ändert sich nie); unlesbar: „Frozen clip not readable, playing live“ im Bericht. Es planen
mit `eingefrorenVon`: der Spieler (der Satz trägt die eingefrorenen Clips; fehlt eine Wellenform, plant
er neu), die Tonführung des Composers (neu im nächsten Takt), das Vorladen der Sitzung und die Messung
der Lautheit eines Loop-Clips. *Export audio…* rechnet weiter live. Tests in `test_LoopSpieler` (Datei
da: die Wellenform, nichts dekodiert, beim zweiten Play aus dem Speicher; keine Datei: live; verändert
seit dem Einfrieren: live; unlesbar: die Datei geht, live, neu abgelegt zählt sie wieder; im Composer
die Wellenform). Spieler, Freeze, Ton und Composer unter Release-clang und Testing-MSVC 194 / 194.

**Verdacht (S144, beim Lesen für 30f-3; noch nicht mit einem Test belegt):** `Tonfuehrung::LoopQuelle::baue` baut nach einer Bearbeitung,
deren Töne noch rechnen, das Programm schon mit dem **neuen** Loop und den **alten** Klängen
(`Tonfuehrung.cpp`, `baue`, Fall C) — `plane` sucht die Klänge aber über die neuen Namen. Verschieben
sich die Namen (ein neuer Ton in einem früheren Muster), spielt eine Stimme bis zum Ende des Auftrags
einen falschen Klang; kommen sie nur hinten dazu, schweigt sie. Der Kommentar sagt „bis er da ist,
klingt das bisherige Programm“; der Spieler des Loop-Panels hält es so. Nicht behoben — zuerst mit einem
Test belegen, der ohne Fix rot ist.
*Belegt und behoben (S144, Patrik a):* `test_LoopSpieler` „Loop im Composer S144“ — eine neue Spur vorn
verschiebt die Namen; ohne Fix schwieg der Loop-Clip, bis die neuen Töne da waren (0 statt 0,5). Jetzt
baut `baue` ein Programm nur mit Klängen, die zu den Namen gehören (`passt`; im Fall D die lückenhaften
dieser Namen); sonst klingt das bisherige bis `pruefe`. Spieler, Freeze, Ton und Composer unter
Release-clang und Testing-MSVC 195 / 195.

✅ **FZ4 a** (Patrik S144, nach Skizze): **Bedienung nach Skizze A–D.**
- **A — Marke am Clip:** rechts auf dem Clip, links vom „fx“: „❄ frozen“ (durchgezogen), „computing… n %“
  (gepunktet), „❄ frozen · changed“ (gestrichelt, matter); auf einem schmalen Clip nur das Zeichen,
  der Text im Tooltip.
- **B — Kontextmenü:** nach *Export audio...* „Freeze clip“, an einem eingefrorenen Clip „Unfreeze
  clip“; bei mehreren markierten „Freeze 3 clips“ / „Unfreeze 3 clips“.
- **C — Ablauf:** *Freeze clip* setzt die Marke auf den jetzigen Abdruck (mit Undo) und rechnet sofort im
  Hintergrund (computing… → frozen); nach einer Bearbeitung „frozen · changed“, der Clip spielt live;
  beim Speichern im Hintergrund neu eingefroren (F2 a); ein Undo auf den eingefrorenen Stand gilt
  sofort (die Datei liegt noch); *Unfreeze clip* nimmt die Marke (mit Undo), die Datei bleibt im Speicher,
  bis die Grenze sie räumt.
- **D — Hinweiszeile** des Loop-Panels (wie „Preparing sounds…“): „Freezing clip 2 of 3 … 40 %“.

Eingefroren wird nur, wo jemand es wählt — welche Clips sich lohnen, zeigt 30h (F4 a). Grund: wer *Freeze
clip* wählt, will den Clip ab jetzt fertig spielen; F2 a verhindert das Rechnen mitten im Bearbeiten, ein
einmaliges Rechnen auf ausdrücklichen Wunsch berührt das nicht. Verworfen: b (auch *Freeze clip* rechnet
erst beim Speichern — nach dem Klick stünde „changed“ da, ohne dass etwas verändert wurde). Kippt, wenn
oft eingefroren und gleich weiterbearbeitet wird — dann rechnete a umsonst, b sparte es.

**Plan 30g (S144, Claude):** 30g-1 Ablauf ohne Oberfläche (`FreezeAuftrag`, Marken, Zustand), 30g-2
Bedienung (Marke und Menü in der `Taktachse`, Befehle mit Undo, Speichern, Hinweiszeile, Spieler und
Composer schalten um, sobald die Datei liegt). Handbuch und Sichttest-Liste in 30i.

**Gebaut 30g-1 (S144):** in `loop/Freeze` `FrozenZustand` (Aus, Frozen, Veraendert, Fehlt) mit
`frozenZustand`, `setzeFrozenMarken` (*Freeze clip*; zurück die Clips ohne Datei), `frischeFrozenMarken`
(beim Speichern, F2 a), `loeseFrozenMarken` (*Unfreeze clip*), `fehlendeFrozen` (unverändert, aber die
Datei fehlt — F5 a). Neu `loop/FreezeAuftrag`: auf einem eigenen Thread je Clip die Töne
(`Tonauftrag`), `rechneClip`, `legeFrozen` unter dem jetzigen Abdruck; liegt die Datei schon vor,
wird nichts gerechnet; Stand (Clip n von m, Anteil: Töne bis 0,5, Rechnen bis 1), Abbruch ohne Datei.
Tests in `test_LoopFreezeSpielen` (zwei Fälle: Marken und Auftrag über Einfrieren, Bearbeiten, Speichern,
Undo, Lösen, Räumen; Abbruch mit haltendem Dekodierer). Freeze, Spieler, Ton unter Release-clang und
Testing-MSVC 46 / 46.

**Gebaut 30g-2 (S144):** in der `Taktachse` die Marke (`FreezeMarke`, `setzeFreezeMarken`, `freezeText`,
`freezeTooltip`): rechts, links vom „fx“; „❄ frozen“ durchgezogen, „computing… n %“ gepunktet, „❄ frozen ·
changed“ gestrichelt und matter (Fehlt: „❄ frozen · missing“); ist kein Platz für den Text (dem Namen
bleiben bis 60 px), die Kurzform — eingefroren die Flocke im Quadrat, **verändert oder fehlend die Flocke
in einem Warndreieck** (Zusatz Patrik S144: im Quadrat nicht zu unterscheiden), rechnend die Prozent;
Tooltip mit Erklärung. Im Menü *Freeze clip* / *Unfreeze clip* nach *Export audio...*, bei mehreren
*Freeze 2 clips* / *Unfreeze 2 clips* (Unfreeze, wenn alle eingefroren sind). Im `LoopPanel`
`friereEin` (ein Undo-Schritt, rechnet sofort), Speichern frischt die Marken auf und rechnet den neuen
Stand (F2 a), nach jeder Änderung `bestimmeFreeze` (fehlt eine Datei unverändert, wird sie gerechnet —
F5 a; was misslang, erst nach der nächsten Änderung wieder), die Hinweiszeile „Freezing clip n of m … p %“
(D), Meldungen im Bericht. `frozenFassung()` wächst mit jeder abgelegten, vergessenen oder geräumten
Datei: Spieler und Tonführung planen dann neu — auch ohne Änderung am Loop. Tests: `test_LoopFreezeBedienung`
(Menü, Marke, Undo/Redo, Speichern, Räumen, Lösen; Kurzformen; `LUMIVIZ_FREEZE_BILD` legt vier Bilder ab),
`test_LoopMenues` kennt *Freeze clip*. Bereich `*Loop*,*Instrument*,*Shape*,*Palette*,*Drum*,*Wav*` unter
Release-clang 545 / 545 (vor dem Fall der Kurzformen), unter Testing-MSVC 546 / 546; Composer je 151 / 151.

**Gebaut 30h (S144):** `Tonauftrag::rechenzeitImSpeicherMs` (die Summe der gemerkten Zeiten seiner
Klänge, ohne zu laden; leer, wenn einer fehlt) und `Tonauftrag::rechenThreads`; in `loop/Freeze`
`clipRechenzeitMs` (FZ6 a) und die Settings `loop/freezeHintS` (0,1 bis 60 s, Vorgabe 1 s) und
`loop/freezeHintWaiting` (FZ7 a). In der `Taktachse` die Sanduhr an der Stelle der Marke, nur ohne Marke
(`FreezeMarke::zeigeZeit`, `zeitText`, Kurzform nur „⏳“), der Tooltip mit Arbeit und Wartezeit, im Menü
„Freeze clip (3.2 s to compute)“. Das `LoopPanel` misst nach jeder Änderung und sobald der Tonspeicher
Neues aufnimmt (`stand().gelegt`), liest die Settings etwa jede Sekunde. In den Settings unter *Frozen
Clips* die Zeile *Freeze hint* (`loopFreezeHint`, `loopFreezeHintKind`). Tests in `test_LoopFreezeBedienung`
(Sanduhr mit Text, Tooltip, Menü, Bild `_hourglass`; Rechenzeit aus dem Tonspeicher — die Klaviertöne
rechnen ~1 ms, der Fall legt je Ton 600 ms in den Speicher —, Settings, Schwelle, Wartezeit, eingefroren
ohne Sanduhr), in `test_LoopFreeze` die Zeile *Freeze hint* über das Settings-Panel. Freeze, Menüs, Ton,
Spieler unter Release-clang und Testing-MSVC je 57 / 57, `test_LoopFreeze` danach je 6 / 6.

✅ **FZ5 a** (Patrik S144, nach Skizze): **Messung nach Skizze A–D.**
- **A — Zeichen:** rechts auf dem Clip, links vom „fx“, eine Sanduhr mit der Zeit („⏳ 3.2 s“); auf einem
  schmalen Clip nur die Sanduhr; keines an einem eingefrorenen Clip, unter der Schwelle und vor dem
  ersten Rechnen.
- **B — Tooltip:** „Computing this clip takes 3.2 s — Freeze clip to start it at once“.
- **C — Menü:** „Freeze clip (3.2 s to compute)“.
- **D — Settings** → Panels, unter *Frozen Clips*: „Freeze hint: from [1.0 s] of computing time“,
  0,1 bis 60 s, Schritt 0,1.

Grund: F4 a soll zeigen, wo Freeze sich lohnt — mit der Zahl auf dem Clip sieht man beim Überblicken, welcher
am meisten kostet. Verworfen: b (die Sanduhr ohne Zahl, die Zeit nur in Tooltip und Menü — ruhiger, aber man
müsste über jeden Clip fahren). Zeichen und Marke Freeze schließen sich aus, höchstens zwei Marken stehen
zugleich. Kippt, wenn viele schmale Clips über der Schwelle liegen und Zahlen die Namen verdrängen — dann b.

✅ **FZ6 a, mit Zusatz: auch b, umschaltbar** (Patrik S144). **Die Rechenzeit eines Clips ist die Summe
der gemessenen Zeiten seiner Töne aus dem Tonspeicher** — die Arbeit auf einem Kern, die Freeze erspart; ein
Ton, den mehrere Clips nutzen, zählt bei jedem voll; fehlt die Zeit eines seiner Töne im Speicher, zeigt der
Clip keine Zahl, bis er neu gerechnet ist. *Zusatz:* umschaltbar auf **b** — die Summe geteilt durch die
Threads, die rechnen (Kerne − 1), als Schätzung der Wartezeit beim Öffnen. Grund für a als Grundlage: die
Summe ist eine Eigenschaft des Clips, unabhängig davon, was gleichzeitig rechnet; Clips lassen sich
vergleichen. Wo der Schalter sitzt: FZ7.

✅ **FZ7 a** (Patrik S144): **Der Schalter sitzt in der Settings-Zeile von FZ5 a D:** „Freeze hint: from
[1.0 s] of [computing work ▾]“ mit *computing work* (die Summe, Vorgabe) und *waiting time* (geteilt durch
die Threads); gemerkt; die Schwelle gilt für die gewählte Zahl. Der Tooltip an der Sanduhr nennt beide,
etwa „Computing this clip takes 3.2 s of work (about 0.5 s waiting on 7 threads) — Freeze clip to start it
at once“. Grund: Schalter und Schwelle hängen zusammen (1 s Arbeit und 1 s Warten sind verschiedene
Grenzen), alle Grenzen von Freeze stehen in den Settings. Verworfen: b (im Loop-Panel per Rechtsklick auf
die Sanduhr — ein Menüeintrag für eine Anzeige wäre in der Achse fremd). Kippt, wenn oft umgeschaltet wird —
dann b zusätzlich.

**Befund vor 30i (Code S145, gelesen):** Publish (`paket/Packen`) lädt den gespeicherten Loop und schreibt
jede Nennung einer Zugehörigen als `user:<name>`; `frozen` am Clip bleibt dabei stehen. Der Import
(`paket/Import`) liest nur, was das Manifest unter `files` nennt — ein weiterer Block wird von einem
heutigen Programm übergangen. Der Abdruck (`clipAbdruck`) setzt für jeden Verweis den Inhalt ein, für ein
Instrument das gelesene Instrument mit dem Inhalt seiner Samples und Zeichnungen: Umbenennen beim Import
(Keep both) ändert ihn nicht, *Use mine* mit anderem Inhalt schon — dann rechnet der Empfänger zu Recht
neu. Die Rate des Ausgabegeräts geht in den Abdruck; ein Empfänger mit anderer Rate rechnet ebenfalls neu.

✅ **FP1 a** (Patrik S145): **Publish nimmt je Clip mit Marke die Wellenform zu dem Abdruck, den der
gespeicherte Loop jetzt ergibt.** Fehlt sie (noch im Rechnen, von der Grenze geräumt), rechnet Publish sie
vor dem Packen — mit einer Fortschrittszeile im Dialog („Freezing clip n of m … p %“) und Cancel. Die
Datei zu einer veralteten Marke geht nicht mit. Grund: so gilt der Zusatz zu F5 a ohne Ausnahme (der
Empfänger wartet nicht), unabhängig davon, ob beim Absender gerade die Grenze geräumt hat; das Rechnen gibt
es schon (`FreezeAuftrag`), im Normalfall liegt alles vor. Verworfen: b (nur, was vorliegt — ein
unvollständiges Paket gerade in den Ausnahmen, ohne dass der Absender etwas tun kann), c (die Datei zur
gespeicherten Marke — nach einer Änderung am Instrument Bytes, die beim Empfänger nie passen). Kippt, wenn
Publish nie warten darf (etwa Pakete ohne Dialog) — dann b.

✅ **FP2 a** (Patrik S145): **Im Publish-Dialog eine Gruppe „Frozen clips (n)“ im Baum unter dem Loop,
je Clip eine Zeile** („Bass · bar 5 — ready · 10.1 MB“, „Lead · bar 9 — to compute · 3.2 s“); in der Zeile
unter dem Baum ein zweiter Haken „☑ Include frozen clips — the receiver plays them at once“ (Vorgabe an,
gemerkt); die Summe im Fuß zählt die Wellenformen mit. Fehlende rechnen ab dem Öffnen des Dialogs im
Hintergrund; ist man vorher bei Publish, steht im Fuß statt der Summe „Freezing clip 2 of 3 … 40 %“ und
Publish ist bis dahin gesperrt; Cancel bricht das Rechnen ab und schließt, Gerechnetes bleibt in `frozen`.
Grund: der Baum ist die volle Liste dessen, was ins Paket geht; der Haken entscheidet über ein vielfach
größeres Paket (32-bit float Stereo ≈ 21 MB je Minute, Zip packt es kaum) — die Vorgabe „an“ erfüllt den
Zusatz zu F5 a; ab dem Öffnen zu rechnen heißt, beim Klick ist meist alles da. Verworfen: b (ohne Haken —
ein Paket, das man nicht verschicken kann, wiegt schwerer als ein Schalter weniger), c (nur der Haken mit
Zahl und Größe — verschweigt, welcher Clip rechnet). Kippt, wenn Pakete nie über Wege mit Größengrenze
gehen — dann b.

✅ **FP3 a** (Patrik S145): **Der Import legt nur Wellenformen ab, die beim Empfänger passen.** Erst Loop
und Dateien schreiben, dann je Clip mit Marke den Abdruck beim Empfänger rechnen (seine Ausgaberate);
nach `frozen` kommt nur, was einem davon gleicht; eine vorhandene Datei gleichen Namens bleibt; danach
die Grenze (die eben gelegten zählen als gerade gebraucht). In *Contents* „Frozen clips (3) → frozen
clips, 30.4 MB“, in *Notes* „Frozen clips: the receiver checks them on import“, nach dem Import
„Frozen clips: 2 of 3 ready to play — 1 will be computed here“. Grund: nur was zu einem Clip des
importierten Loops gehört und hier passt — keine toten Dateien, die eigene über die Grenze drängen, und
kein Paket legt eine Datei für einen fremden Loop ab; eine beim Empfänger gerechnete Datei wiegt
schwerer als eine fremde. Verworfen: b (alles ungeprüft ablegen), c (dazu ein Haken *Use the frozen
clips* — eine passende Datei ist genau, was der Empfänger selbst rechnen würde; die Größe entscheidet
der Absender, FP2 a). Kippt, wenn Empfänger fremden Wellenformen grundsätzlich misstrauen sollen — dann c
(prüfen ließe sich der Inhalt nur durch Nachrechnen).

**Plan 30i (S145, gebilligt):** drei Teilschritte. **30i-1** Paket und Import ohne Oberfläche; **30i-2**
die Dialoge (Rate und Engine durchreichen, Publish nach FP2 a, Import nach FP3 a); **30i-3** Handbuch,
Sichttest-Liste Freeze, Bereichslauf mit `*Publish*`.

**Gebaut 30i-1 (S145):** in `loop/Freeze` `legeFrozenDaten` (Bytes einer fertigen WAV: erst als
`<abdruck>.wav.part` gelesen — Rate und Inhalt —, dann umbenannt; eine lesbare vorhandene bleibt, danach
die Grenze; `FrozenAblage`). In `paket/Packen` `FrozenTeil` und `frozenTeile` (je Clip mit Marke Etikett
„Bass · bar 5“, jetziger Abdruck, Datei und Größe, wenn sie vorliegt), `frozenPfad`, `packe` mit den
Abdrücken: je einmal unter `frozen/<abdruck>.wav`, im Manifest `package.frozen` (`path`, `fingerprint`), nur
bei einem Loop, eine geräumte bleibt draußen; die Fassung des Pakets bleibt 2 (ein Programm vor 30i
übergeht den Eintrag). In `paket/Import` liest `lies` nur Einträge mit 40 Hex-Zeichen unter passendem Pfad
(sonst „damaged“), `PaketInhalt::frozen`/`frozenGroesse`; `PaketWahl::rate` (0 = nichts ablegen);
`importierePaket` legt nach dem Loop nach FP3 a ab und zählt `frozenClips`, `frozenBereit`,
`frozenGelegt`. Tests `test_PublishFreeze` (drei Fälle: Weg vom Absender zum Empfänger mit gleichem Abdruck
trotz anderer Nennungen; Keep both, Use mine, andere Rate, vorhandene Datei, fremde und kaputte
Wellenform, falscher Pfad, Paket ohne Eintrag; fehlende beim Packen, Instrument-Paket). `*Publish*,*Freeze*`
unter Release-clang und Testing-MSVC je 31 / 31.

**Gebaut 30i-2 (S145):** in `UI/dialogs/PaketDialoge` `FreezeTon` (Rate und Dekodierer) und
`freezeTonVon(engine)` (wie das Loop-Panel: ohne Engine 44100); durchgereicht vom Loop-Panel
(`veroeffentlicheAusFenster`), aus dem Menü (*Publish a file...*, *Import package...*) und aus den Settings
(*Import...*, `zeigeDatenImport`). `PublishDialog` nach FP2 a: die Gruppe *Frozen clips (n)* unter dem
Loop („ready · 8 KB“, „computing… 40 %“, „to compute · 3.2 s“, nach misslungenem Rechnen „could not
compute — the receiver computes it“ in der Hinweisfarbe), der Haken *Include frozen clips* (gemerkt unter
`data/publishFrozen`, Vorgabe an), die Summe mit den Wellenformen; ein `FreezeAuftrag` ab dem Öffnen,
eine Uhr alle 100 ms; **Publish** wartet mit „Freezing clip n of m … p %“ im Fuß und läuft danach von
selbst weiter. Seine Meldungen laufen jetzt über `fuehreDialogAus` (`publishReplaceQuestion`,
`publishFailed`, `publishDone`). `ImportPaketDialog` nach FP3 a: die Zeile *Frozen clips (n) → frozen
clips, 16 KB*, der Hinweis, die Zahl im Kopf, `frozenMeldung` in der Meldung nach dem Import.
`PaketBestand.hpp` trägt jetzt `FrozenBestand` und die Hilfen dazu. Tests in `test_PublishDialoge` (zwei
Fälle; `LUMIVIZ_PUBLISH_FROZEN_BILD` legt zwei Bilder ab, das zweite mit `_import`).
`--source-file=*test_Publish*,*test_Datensicherung*,*test_LoopFreeze*` unter Release-clang und
Testing-MSVC je 53 / 53; die App unter clang gebaut.

**Gebaut 30i-3 (S145), Stufe 30 fertig bis auf den vollen Lauf:** Handbuch 1.202.0 (§16 „Clips
einfrieren: Freeze“, „Weitergeben: Publish“ mit eingefrorenen Clips, §8 *Freeze hint*), Sichttest-Liste
`.claude/handover/Sichttest_Freeze.md` (S, F, M, P, U). Bereich
`*Loop*,*Instrument*,*Shape*,*Palette*,*Drum*,*Wav*,*Publish*,*Import package*` unter Release-clang
561 / 561, Composer 151 / 151 (zusammen gut 4½ Minuten). Unter Testing-MSVC nicht gefahren (Patrik:
verworfen, dauert zu lange) — dort liefen die engen Fälle von 30i-1 und 30i-2 grün.

**Befund vor 30a (S143):** Die Lautstärke des Loops (`Loop::laut`) wurde in jeden Schlag gerechnet
(`Rechenkern.cpp`, `plane`), also **vor** den Effekten der Clips; der Fader des Kanals wirkt nach
ihnen (27e-1). Weil Drive nicht linear ist, verstellte die Lautstärke des Loops die Zerrung eines
Clips — und eine Wellenform ohne sie, der man sie live wieder dazugibt, klänge bei Drive anders als
live. F3 a ging so nicht auf.

✅ **WX5 a** (Patrik S143): **Die Lautstärke des Loops wirkt nach den Effekten der Clips**, wie ein
Summenregler: Schlag → Effekte des Clips → Fader des Kanals → Lautstärke des Loops → Limiter. F3 a gilt
damit wörtlich. Grund: sie ist die Lautstärke des ganzen Loops — an einem Mischpult hinter den
Effekten, wie der Fader (LL12 a); dass sie die Zerrung eines Clips verstellte, war ein Fehler im
Signalweg. Verworfen: b (sie bleibt vor den Effekten, F3 a wird berichtigt, ihre Änderung friert neu
ein — kein Klang ändert sich, aber der Fehler bliebe). *Gebaut (S143):* wie der Fader — ohne Effekte
am Schlag (dieselbe Rechnung, Bit für Bit wie vorher), mit Effekten im Faktor des Busses
(`Bus::faktor` = Fader mal Lautstärke). Betroffen sind nur Clips mit Drive in Loops mit einer
Lautstärke unter 1; unter den mitgelieferten **nur `effects_demo`** (*Acid* im Kanal *Bass*, Drive 0,5,
Mix 0,7, Lautstärke des Loops 0,3): er zerrt jetzt stärker und ist danach auf 0,3 gesenkt. `Loop
Werte` sieht das **nicht** (größter Betrag 0,516817 → 0,516810, die Summe innerhalb eines Tausendstels
— der Clip ist vier von 32 Takten); die Behauptung in der Vorlage, der Test zeige jede Änderung, war zu
stark. Hörprobe offen (`LUMIVIZ_LOOP_WAV`).

✅ **HB1 a** (Patrik S143): **Die Lautstärke des Loops steht im Handbuch** (§16, „Die Lautstärke des
Loops“): Datei `volume`, Vorgabe 0,5 = −6 dB, Luft nach oben, nach Effekten und Fadern, vor dem
Limiter, im Panel nicht einstellbar. Bei 30c dazu ein Satz, dass *Export audio…* eines Clips ohne
Fader und ohne diese Lautstärke schreibt. Verworfen: b (dazu ein Summen-Fader im Panel — seit WX5 a
tut sie dasselbe wie alle Fader zusammen, ein Regler wäre eine zweite Stelle für dieselbe Wirkung),
c (nicht beschreiben — sie formt den Pegel, den man hört und exportiert). Kippt, wenn Loops
verschiedener Lautstärke im Composer ohne Fader und Gain angeglichen werden sollen — dann b.

**Gebaut 30a (S143):** `loop/Wellenform` — `clipFassung` (nur dieser Clip in seinem Kanal, an seinem
Takt; Fader 0 dB, Lautstärke 1, nur sein Muster; dahinter ein ausgeschalteter Kanal mit einem leeren
Muster, der den Durchlauf über den längsten Nachklang verlängert — sonst dämpfte die Saitenplanung
die letzten Töne mit den ersten der nächsten Runde), `rechneClip` (durch den `Klangerzeuger` in
Blöcken wie im Mixer, ab der Stelle des Clips; der Nachklang endet, wenn die Summe 0,5 s unter −96 dBFS
bleibt, sonst nach 30 s mit `nachklangGekappt`), `rechneAusschnitt` (WX3 a: der Loop ab dem Versatz,
im Durchlauf wie die Tonführung, mit Fadern und Lautstärke, ohne Nachklang); Fortschritt und
Abbruch. Test `test_LoopWellenform`: Bit für Bit wie live (Clip allein), WX5 a am Drive-Clip (live mit
Fader und Lautstärke = Welle mal beide, Bit für Bit), der Nachklang endet in Stille und gleicht dem
weitergerechneten Live-Ton, Saiten mit *Let ring* klingen 3,4 s nach (die Töne tragen ein späteres
Ende als im gewöhnlichen Durchlauf), der Composer-Ausschnitt gleicht dem Klangerzeuger und wiederholt
sich, Fortschritt und Abbruch, eine Messung. Dazu in `test_LoopSpieler` „S127 Loop 3b im Composer“: er
wartet jetzt auf die Töne (seit 29e hing er an der Reihenfolge der Fälle). Im Programm nicht
angesehen.

**Gebaut 30b (S143):** `audio/WavDatei` — `schreibeWav` (Stereo; 32-bit float als Format 3 mit
`fact`, 24 und 16 Bit als PCM; Kappung bei ±1 mit Zählung, der größte Betrag; Dither bei 16 Bit als
TPDF ±1 LSB aus einem festen Startwert mit eigener Rechnung — dieselben Werte ergeben dieselbe Datei
unter jedem Compiler; über `QSaveFile`, über 4 GB nichts) und `liesWav` (was hier geschrieben wird,
dazu Mono und `WAVE_FORMAT_EXTENSIBLE`). **Ein Schreiber für alles:** `loop/Aufnahme::schreibeWav`
gibt an ihn weiter (16 Bit ohne Dither, Bit für Bit wie seit 11b). Skala wie seit 11b mit 32767
(8388607); BASS liest PCM mit 1/32768 — dazu ein halbes LSB Rundung, zusammen höchstens anderthalb
LSB. Test `test_WavDatei`: der Kopf jeder Art Byte für Byte, Zurücklesen (float genau, PCM auf ein
halbes LSB), Kappung, Dither (fest, ein Ton von 0,4 LSB bleibt: Ton mal gelesen / Energie 0,9997),
Grenzen, Mono, die BassEngine liest alle drei (float ohne Abweichung, 24 Bit 1,2·10⁻⁷, 16 Bit
4,0·10⁻⁵).

✅ **WX6 a** (Patrik S143, nach Skizze): **Ein eigenes Fenster *Export audio*.** Die Skizze:
- **Menü:** im Loop-Panel am Clip *Export audio...* nach *Effects...* (grau bei mehreren markierten);
  im Composer am Loop-Clip nach *Open in Loop panel*, auch ohne *Edit*.
- **Vor dem Export:** *Clip* (welcher: „Keys · Piano stabs, bars 5–8“), *Contains* (was drin ist — im
  Loop-Panel „the clip with its effects, expression and tail — without channel fader and loop
  volume“, im Composer „the loop from 0:12 for 0:30, with its faders — without the composition's
  gain, fades and limiter“), *File* mit Vorschlag und **Browse...** (der Windows-Dialog, nur für den
  Ort), *Format* mit drei Knöpfen: **32-bit float** (vorgewählt), **24-bit**, **16-bit (dither)**;
  gemerkt werden Format und Ordner. **Export** / **Cancel**.
- **Beim Export:** dasselbe Fenster zeigt „Preparing sounds… n / m“, dann „Rendering… n %“, mit
  **Cancel**.
- **Am Ende:** „Exported 12.4 s (tail 3.4 s) · 32-bit float · 4.6 MB“; bei Kappung eine Zeile in
  Warnfarbe („312 values above 0 dB were clipped — 32-bit float keeps them“); **Show in folder**,
  **Close**.

Grund: der Export muss vor dem Klick sagen, was er enthält (ohne Fader, ohne Lautstärke des Loops,
mit Nachklang); das Format ist eine sichtbare Wahl; Fortschritt und Ergebnis brauchen ohnehin ein
Fenster (der Composer hat keine Meldezeile). Verworfen: b (Windows-Dialog mit dem Format als
*Dateityp*, danach ein kleines Fenster — die Wahl versteckt sich in einer Liste, das Enthaltene stünde
erst danach da). Kippt, wenn ohne stehendes Fenster exportiert werden soll — dann b mit einem
Hinweis, der von selbst verschwindet.

✅ **WX7 a** (Patrik S143, Frage Patrik „dann kann man im Loop-Panel den ganzen Loop exportieren?“):
**Das Loop-Panel exportiert auch den ganzen Loop**, so wie das Panel ihn spielt — mit Fadern,
Kanal-Schaltern, der Lautstärke des Loops und seinem Limiter; daneben bleibt *Export audio...* am
Clip. Dieselbe Rechnung (`rechneAusschnitt`, dazu der Begrenzer `audio/Begrenzer` ohne Engine),
dasselbe Fenster mit einer dritten Fassung von *Contains*. Grund: der Export gehört dorthin, wo der
Loop entsteht; im Composer ist er ein Ausschnitt ohne seinen Limiter. Verworfen: b (nur Clips, den
ganzen Loop über den Composer — ein Umweg als Regel, der Klang des Panels fehlte). Kippt, wenn Loops
nur als Teil einer Komposition weitergegeben werden. Einzeln danach: WX8 der Ort des Eintrags, WX9
wie der Export endet (ein Durchlauf nahtlos oder mit Nachklang).

✅ **WX8 a** (Patrik S143, nach Skizze): **Ein Knopf *Export audio...* in der Datei-Zeile des
Loop-Panels**, zwischen **Save As...** und **Publish...**; Tooltip „Write the whole loop as you hear it
here — with faders, loop volume and limiter — into a WAV file“. Grund: es geht um die ganze Datei, und
die Datei-Zeile sammelt speichern, speichern unter, weitergeben (wie im Composer: Neues im Kopf in die
Datei-Zeile). Verworfen: b (im Kontextmenü einer freien Stelle der Achse — dort steht, was an der
Stelle geschieht, nicht der ganze Loop). Kippt, wenn die Datei-Zeile bei schmalem Panel zu voll wird
— dann *Export audio...* und *Publish...* hinter einem Knopf mit Menü.

✅ **WX9 a** (Patrik S143): **Beim ganzen Loop die Wahl im Fenster.** Zwei Zeilen mehr: *Ending* mit
**Seamless loop** (Vorgabe; „One pass as it sounds from the second on — repeats without a gap“) und
**With tail** („Starts from silence and rings out after the end“), dazu *Passes* 1 bis 64 (Vorgabe 1);
gemerkt wird die letzte Wahl. Beim Clip bleibt es, wie es ist (immer mit Nachklang, F3 a). Grund:
beide Enden haben echte Verwendungen (weiter loopen, als Schluss), keines schließt das andere ein;
„nahtlos“ ist, was das Panel ab dem zweiten Durchlauf spielt. Verworfen: b (nur nahtlos — für einen
Schluss bliebe der Umweg über den Composer), c (nur mit Nachklang). Kippt, wenn exportierte Loops nur
in anderen Programmen geloopt werden — dann b ohne *Passes*. *Rechnung (Claude):* nahtlos = ein
Durchlauf vorweg für Effekte und Limiter, verworfen, dann *Passes* Durchläufe; mit Nachklang = die
Clips *Passes*-mal hintereinander gelegt (die Saitenplanung läuft über die Durchläufe), dahinter Platz
für den Nachklang wie bei `clipFassung`.

**Gebaut 30c (S143):** `loop/Wellenform` um den ganzen Loop erweitert (`loopFassung`,
`loopKlangNamen`, `rechneLoop` — der Limiter `audio/Begrenzer` offline, seine Verzögerung
ausgeglichen; mit Nachklang haben die Effekte je Durchlauf ihr eigenes Gedächtnis, ein Drive über den
Nachklang des vorigen hinweg kann sich darum leicht von live unterscheiden). `loop/ExportAuftrag`
(ein Thread: Töne über einen `Tonauftrag`, Wellenform, `audio::schreibeWav`; Stand, Abbruch,
Ergebnis; der Destruktor bricht ab und wartet — §6.7), `exportName`, `tiefeText`.
`loop/ExportFenster` (WX6 a, die drei Zustände; gemerkt unter `loop/exportFormat`, `exportFolder`,
`exportEnding`, `exportPasses`; **Show in folder** mit Testhaken). Im Loop-Panel der Knopf
`loopExportAudio` (WX8 a) und am Clip `loopClipExportAudio` (`Taktachse::exportVerlangt`,
`LoopPanel::oeffneExport`); im Composer am Loop-Clip `composerExportLoopAudio` (`Zeitachse::loopExportieren`,
`ComposerPanel::oeffneLoopExport`, die offene Fassung des Loop-Panels zuerst, L7 a). Handbuch
1.200.0 (§16 „Als WAV exportieren“, §15), Sichttest-Liste `.claude/handover/Sichttest_Export.md`.
Test `test_LoopExport` (sieben Fälle: Clip Bit für Bit, ganzer Loop nahtlos — zwei Durchläufe weichen
um 3,9·10⁻⁵ voneinander ab — und mit Nachklang 2,7 s, Composer-Ausschnitt 24 Bit, Abbruch und
Fehler, Dateinamen, das Fenster mit Gemerktem und Cancel, der Weg über Loop-Panel und Composer);
in `test_LoopMenues` der Eintrag. Release-clang: Bereich 530 / 530, Composer 151 / 151. Im Programm
nicht angesehen.

### 4.5 Stufe 29 „Töne vorab“ — Teilschritte

**Befund (Code S142, gelesen): darf `rendereTon` auf mehreren Threads laufen?** Der Rechenkern ja:
er rechnet auf einem `const Instrument&` mit lokalen Daten; die zwei geteilten Speicher, die er liest,
sind gesperrt (`sampleDaten`, `Sample.cpp:215`; `shape::tonformFuer`, `Tonform.cpp:470`; die
mitgelieferten Zeichnungen darunter einmal und threadsicher gelesen). **Nicht threadfest ist der
Rand:** `instrumente()` füllt seine Liste beim ersten Aufruf ohne Sperre (`Instrument.cpp:2408`,
mit Absicht wiederholbar, wenn der Ordner fehlte); `fassung()` für Instrumente und für Zeichnungen
ist ein einfacher `uint64_t`; die gemerkte Map aus `saitenAusInstrumenten()` hat keine Sperre (je
Thread ein eigenes Objekt); `ladeKlaenge` dekodiert Samples über BASS (`decodeStereo`) — ob BASS
threadsicher ist, vor dem Bau in der Doku nachlesen. **Gerechnet wird heute an drei Orten**, jeder
für sich und auf dem Thread der Oberfläche: `Spieler::ladeProgramm`, `Tonfuehrung::LoopQuelle::baue`
und die Lautheitsmessung je Clip im Composer (`ClipLautheit.cpp:63`).

✅ **TV1 a** (Patrik S142): **Sechs Teilschritte, von innen nach außen** — jede Schicht ist belegt,
bevor die nächste auf ihr aufbaut.

| Teilschritt | Inhalt |
|---|---|
| 29a | **Threadfest machen:** `instrumente()` mit Sperre (wiederholt weiter, wenn der Ordner fehlte), beide `fassung()` als `std::atomic`. Test: jedes mitgelieferte Instrument auf mehreren Threads gleichzeitig, Bit für Bit gegen einen Lauf allein (auch Sample- und Shape-Schichten). Kein Verhalten ändert sich, `Loop Werte` bleibt. |
| 29b | **Speicher je Tonname** (`loop/Tonspeicher`; Schlüssel: TV2): Grenze 1 GB, der am längsten nicht gebrauchte Ton geht, je Ton die gemessene Rechenzeit (für F4 a). `ladeKlaenge` holt aus dem Speicher, noch synchron; Spieler, Tonführung und ClipLautheit teilen ihn, eine Bearbeitung rechnet nur neue Namen. Tests: ein zweiter Aufruf rechnet nichts, die Grenze verwirft das Älteste, Werte Bit für Bit gleich. |
| 29c | **Im Hintergrund:** ein Auftrag (Liste von Tonnamen) auf einem Pool von Threads (Kerne − 1), meldet n / m, lässt sich abbrechen; wo Samples dekodiert werden: TV3. Test ohne Fenster. |
| 29d | **Loop-Panel:** Play wartet mit „Preparing sounds… n / m“, Stop bricht ab (H2 a); Vorladen beim Öffnen eines Loops; eine Bearbeitung während des Spielens lässt die alten Töne klingen und schaltet um, wenn die neuen fertig sind; das Anspielen im Editor wird mitgeprüft. Tests über das Panel. |
| 29e | **Composer:** Vorladen beim Öffnen einer Komposition für alle Loop-Clips, Play wartet; `LoopQuelle` und ClipLautheit holen aus dem Speicher; der Vorlauf von 2 s bleibt nur für den Fall „verworfen“ (fehlende Noten still, die Zeile sagt es). |
| 29f | **Abschluss:** Grenze in den Settings, Handbuch, Sichttest-Liste, Bereichslauf. |

Verworfen: b (drei Teilschritte nach Orten: Speicher mit Absicherung und Hintergrund in einem — ein
roter Test hätte drei mögliche Ursachen, und Wettläufe sind selten und schwer nachzustellen). Kippt,
wenn 29a sehr klein ausfällt — dann gehört es an den Anfang von 29b, die Reihenfolge bleibt.

✅ **TV2 a** (Patrik S142): **Der Schlüssel ist ein Fingerabdruck der Eingaben.** Tonname + Rate +
Hash über das Instrument, wie es in die Datei geschrieben würde (`alsJson`), dazu je Sample-Schicht
Größe und Änderungszeit der Datei (der Schlüssel von `sampleDaten`) und je Shape-Schicht der Inhalt
der Zeichnung. Ein geändertes Instrument bekommt neue Schlüssel, seine alten Töne fallen mit der Zeit
über die Grenze heraus; alle anderen bleiben. `instrumentFassung()` heißt für den Speicher nur noch
„nachsehen“, nicht „wegwerfen“. Auch Titel, Genres und Beschreibung gehen in den Hash — bewusst: wer
nur klangrelevante Felder hasht, muss jedes neue Feld dort eintragen. Verworfen: b (die globale Zahl
wie heute — jedes *Save* im Creator leerte den ganzen Speicher, eine von Hand geänderte Datei bliebe
unbemerkt), c (eine Zahl je Instrument — richtig nur, solange jede Änderung alle Benutzer findet,
heute sieben Stellen; eine vergessene brächte still falsche Töne, wie bei H3 b verworfen). Kippt, wenn
das Hashen beim Vorladen einer großen Komposition spürbar dauert — dann den Hash je Datei und
Änderungszeit merken, der Schlüssel bleibt.

**Befund BASS (S142, lokale `bass.chm` 2.4, alle Seiten nach *thread-safe*, *simultaneously*,
*current thread* durchsucht; die Hilfe von BASSmix nicht):** Gerät und Fehlercode gelten je Thread
(`BASS_SetDevice`: „The device setting is local to the current thread“); ist in einem Thread kein
Gerät gewählt, nimmt BASS das niedrigste initialisierte. Dekodier-Kanäle spielen nicht ab und haben
keine Wiedergabepuffer. Ein Satz, dass mehrere Threads gleichzeitig dekodieren dürfen, steht nicht
da — erschlossen, nicht belegt.

✅ **TV3 a** (Patrik S142): **Dekodieren gehört zum Auftrag im Hintergrund.** Der Auftrag bekommt
keine ganze Engine, nur eine Funktion „Datei und Rate → Stereo-Daten“. Auch die Klänge der
Drum-Spuren (`kit:`, `lib:`, eigene Samples) kommen in den Tonspeicher, Schlüssel nach TV2 (Nennung,
Rate, Größe und Änderungszeit der Datei), und zählen in „n / m“ mit. Die Engine fährt erst herunter,
wenn alle Aufträge abgebrochen und beendet sind — als Regel in den Lebenszyklus-Vertrag
(`Bootstrap_Integration.md` §6, mit 29c). Ein Test in 29c dekodiert dieselben Dateien auf mehreren
Threads gleichzeitig und vergleicht Bit für Bit mit einem Lauf nacheinander. Verworfen: b (vorher auf
der Oberfläche dekodieren — gegen H1 a, Play und Anzeige warteten auf Verschiedenes, die
Drum-Klänge blieben ohne gemeinsamen Speicher), c (eigener WAV-Leser — eigene Samples dürfen FLAC und
MP3 sein). Kippt halb, wenn der Test Abweichungen oder Abstürze zeigt: dann im Hintergrund hinter
einer Sperre, eins nach dem anderen; zurück zu b erst, wenn BASS auch so auf einem fremden Thread
versagt.

**Gebaut 29a (S142):** `instrumente()` hält eine Sperre und setzt seine Liste genau einmal (leer
zurück kommt eine eigene Liste, die niemand füllt — fehlte der Ordner, versucht der nächste Aufruf es
wieder); `fassung()` der Instrumente ist `std::atomic` (die der Zeichnungen war es schon). Test
`test_LoopTonThreads`: 2174 Töne — alle Tonnamen der mitgelieferten Loops, geplant wie im Programm,
und je Instrument eine Probe — erst allein, dann auf 8 Threads aus einer gemeinsamen Liste, nach
`vergissTonformen`; jede Prüfsumme gleich (Release-clang rund 10 s). Nicht abgedeckt: der Wettlauf
beim allerersten Aufruf von `instrumente()` — die gemerkte Liste lässt sich in der Test-Exe nicht
zurücksetzen.

✅ **TV4 a** (Patrik S142): **Die Programme halten geteilte Zeiger auf die Töne im Speicher.** Neuer
Typ `Klaenge = std::vector<std::shared_ptr<const Klang>>` (`nullptr` = kein Klang, die Stimme
schweigt); `ladeKlaenge`, `Programm::klaenge`, `baueProgramm` und `rechne` nehmen ihn. Ein Ton
existiert einmal; verwirft der Speicher einen, den ein spielendes Programm noch hält, lebt er, bis
das Programm ihn loslässt. Verworfen: b (Kopien wie heute — der Speicher spart Rechenzeit, aber jeder
Spieler und jeder Clip im Composer hielte eine Kopie, die Grenze aus H3 a begrenzte nichts), c
(`Klang::daten` teilt seine Daten — jede Stelle, die einen Klang baut oder schreibt, änderte sich, und
der Typ, mit dem gerechnet wird, wäre kein einfacher Vektor mehr). Kippt, wenn Klänge außerhalb der
Programme häufig als Wert weitergereicht werden — dann c.

**Gebaut 29b (S142):** `loop/Tonspeicher` (Grenze, der am längsten nicht gebrauchte Ton geht zuerst,
je Ton die gemessene Rechen- oder Dekodierzeit, Zähler; von jedem Thread aus benutzbar),
`instrumentAbdruck`, `tonSchluessel`, `klangSchluessel`; `ladeKlaenge` holt jeden Ton und jeden
dekodierten Klang aus dem Speicher und rechnet nur, was fehlt; der Typ `Klaenge` in Rechenkern,
Klangerzeuger, Spieler, Tonführung, ClipLautheit und Pegel. Zwei Regeln, die sich erst beim Bau
zeigten: **eine Klangdatei, die es nicht gibt, kommt nicht in den Speicher** (an ihr ließe sich keine
Änderung erkennen — die Engine aus Pappe der Tests „dekodierte“ solche Pfade, ihre Klänge blieben
über Testfälle hinweg liegen); **nach einem Import leert das Loop-Panel den Speicher** wie schon die
Samples (ein Archiv kann Dateien mit altem Stand ersetzen). Die Grenze stand hier noch fest auf 1 GB
(seit 29f in den Settings, TS1 a). Test `test_LoopTonspeicher` (der Speicher allein; `ladeKlaenge` mit Instrumenten
und einer Klangdatei in einem Wegwerf-Ordner: ein zweites Laden rechnet nichts, ein geändertes
Instrument nur seine Töne, eine geänderte Datei wird neu dekodiert, jeder Ton Bit für Bit wie
`rendereTon`).

**Gebaut 29c (S142):** `loop/Tonauftrag` — Klangnamen und Rate hinein, je Name ein Klang heraus;
jeder Name einmal geladen, auch wenn er mehrfach vorkommt; je Instrument einmal geholt (mit
Samples und Fingerabdruck), verschiedene Instrumente nebeneinander. `rechneHier` lädt auf dem
rufenden Thread, `starte` auf einem Pool (Kerne − 1); `erledigt`/`gesamt` zeigen den Fortschritt,
`brichAb` hält an (ein begonnener Name wird zu Ende geladen), der Destruktor bricht ab und wartet.
Dekodiert wird über einen `Dekodierer` (TV3 a). Der Bericht steht in der Reihenfolge der Namen,
gleich welcher Thread was zuerst fand. **`ladeKlaenge` ist jetzt ein Auftrag auf dem rufenden
Thread** — eine Ladearbeit für vorn und hinten. Lebenszyklus: `Bootstrap_Integration.md` §6.7 (der
Auftrag stirbt vor der Engine). Test `test_LoopTonauftrag`: dieselben Namen vorn und auf vier
Threads, Bit für Bit gleich, derselbe Bericht; Anhalten, während der Thread im Dekodierer wartet;
**BASS dekodiert die 78 mitgelieferten Samples auf 8 Threads Bit für Bit wie nacheinander** — der
Beleg zu TV3 a, den die BASS-Hilfe nicht ausdrücklich gibt (läuft nur mit Ausgabegerät).

✅ **TV5 a** (Patrik S142, nach Skizze): **Das Warten steht an der Stelle von „Bar · Step“, der Knopf
wird sofort zu Stop.** Die Skizze in vier Zuständen, Spielzeile [Play] [BPM] [Info] [Bar · Step]
[Limiter] [Scroll with playhead] [Fit]:
- **A** — Play gedrückt, Töne fehlen: der Knopf zeigt „Stop“, an der Stelle von „Bar · Step“ steht
  in Akzentfarbe „Preparing sounds… 12 / 40“; Stop bricht ab.
- **B** — alles da: der Loop beginnt von vorn (oder an der Startmarke), dort steht wieder
  „Bar · Step“.
- **C** — eine Bearbeitung während des Spielens: die alten Töne klingen weiter, hinter „Bar · Step“
  steht „· updating sounds 3 / 5“, bis umgeschaltet ist.
- **D** — die Editoren (*Play pattern*) ebenso, der Text in ihrer Hinweiszeile.

Das Vorladen beim Öffnen eines Loops zeigt nichts, solange niemand Play drückt; drückt man Play
mittendrin, zeigt A den Rest. Verworfen: b (Fortschritt im Knopf — Zustand und Bedienung vermischt,
der Knopf würde breiter oder der Text zu knapp), c (Balken unter der Spielzeile — schiebt das Panel
bei jeder Bearbeitung um eine Zeile). Kippt, wenn das Warten am Flügel so lange dauert, dass man
abschätzen will, wie lange noch — dann c als Ergänzung zu a.

✅ **TV6 a** (Patrik S142): **Auch das Anspielen geht über den Hintergrund — mit einer Abkürzung für
Töne aus dem Speicher.** `Spieler::starte` schaut zuerst, ob alle Töne im Speicher liegen; dann
beginnt er sofort. Fehlt etwas, rechnet der Auftrag im Hintergrund, der Ton klingt, sobald er da ist,
die Hinweiszeile des Editors zeigt „Preparing sound…“; ein neuer Anschlag ersetzt einen, der noch
rechnet. Betrifft `LoopPanel::spieleAllein` (Taste und Note im Editor, eine Drum-Spur, ▶ im Dialog
*Import package*). Verworfen: b (ein fehlender Ton sofort vorn wie heute — die Oberfläche stünde,
solange er rechnet, und es blieben zwei Ladewege). Kippt, wenn die bis zu 33 ms, die der Takt des
Spielers bis zum Umschalten zufügt, beim Spielen auf der Klaviatur stören — dann prüft der Spieler
häufiger, solange ein Auftrag läuft.

**Gebaut 29d (S142):** Der `Spieler` lädt über einen `Tonauftrag`. `ausSpeicher` prüft zuerst, ob
alles im Tonspeicher liegt (TV6 a: dann sofort); sonst rechnet der Auftrag im Hintergrund. Beim Start
(`starte`) übernimmt der Spieler den Ton und spielt still (`wartet`), `takt` schaltet um, sobald alles
da ist, und beginnt von vorn — oder an der Stelle, zu der inzwischen gesprungen wurde (die
Startmarke B4). Eine Bearbeitung während des Spielens lässt das bisherige Programm klingen
(`bereitet`). Stop bricht ab; ein abgebrochener Auftrag läuft aus, ohne die Oberfläche zu
blockieren, und endet spätestens mit dem Spieler. `ladeVor` rechnet beim Öffnen eines Loops vorab
(H1 a); Play mitten im Vorladen **übernimmt diesen Auftrag**, statt dieselben Töne ein zweites Mal
zu rechnen. Das Loop-Panel zeigt nach TV5 a „Preparing sounds… n / m“ und „· updating sounds n / m“;
der Instrument-Editor in seiner Hinweiszeile „Preparing sound…“ (Anspielen), „Preparing sounds… n /
m“ (*Play pattern*) und „Updating sounds n / m“. **Der Drum-Editor bekam dafür eine graue
Hinweiszeile am Ende seiner Kopfzeile** (Patrik S142, „wie der Instrument-Editor — noch in 29d“),
mit denselben Texten und „The Composer plays the audio — Play pattern is silent“. Ein angespielter
Ton zählt seine Dauer erst, wenn sein Klang da ist. Tests in `test_LoopSpieler` (Play wartet still,
Springen beim Warten, Stop beim Warten; Bearbeitung während des Spielens; Vorladen und Übernahme; die
Anzeige im Panel und im Drum-Editor), dazu eine Engine aus Pappe, die das
Dekodieren anhält. **Testfalle:** der Tonspeicher gilt für das ganze Programm — Fälle mit der echten
BassEngine leeren ihn zuerst, sonst könnten Klänge einer Engine aus Pappe zu denselben Dateien
klingen (`test_LoopWiedergabe`). Im Programm nicht angesehen.

✅ **TV7 a** (Patrik S142, nach Skizze): **Der Composer zeigt das Warten wie das Loop-Panel — im Feld
Zeit der Kopfzeile** ([On] [Play] [Stop] [Loop] [Resume] [Zeit] [Edit] [Sync] …; eine Meldezeile hat
das Panel nicht). Die Skizze in vier Zuständen:
- **A** — Play gedrückt oder die Komposition geöffnet, Töne fehlen: die Uhr steht, im Feld Zeit steht
  in Akzentfarbe „Preparing sounds… 12 / 40“ — gezählt die Töne **aller** Loop-Clips der Komposition
  (H2 a). Play zeigt schon Pause, Stop bricht ab.
- **B** — alles da: die Uhr läuft ab der Stelle, an der Play gedrückt wurde.
- **C** — ein Loop im Loop-Panel geändert, während der Composer ihn spielt: der Clip klingt wie bisher,
  hinter der Zeit „· updating sounds 3 / 5“.
- **D** — ein Sprung zu einem Clip, dessen Töne der Speicher verworfen hat: die Uhr läuft weiter, nur
  die fehlenden Noten schweigen, hinter der Zeit in Warnfarbe „· some notes silent, preparing 7 / 20“.

Verworfen: b (eigene Meldezeile — schöbe das Panel bei jedem Warten um eine Zeile), c (in D die Uhr
kurz anhalten — gegen H2 a, und am Composer hängen Bild und Karaoke). Kippt, wenn das Warten auf alle
Loop-Clips bei großen Kompositionen spürbar dauert — dann nur auf die Clips der ersten Sekunden
warten und den Rest im Hintergrund rechnen; das ändert H2 a und braucht einen eigenen Entscheid.

**Gebaut 29e (S142):** Die `Sitzung` rechnet beim Laden einer Komposition (und nach jeder
Bearbeitung) die Töne **aller Loop-Clips** der eingeschalteten Audio-Kanäle vorab — je Loop-Datei
einmal gelesen, die offene Fassung des Loop-Panels zuerst; neu nur, wenn sich Namen, Rate oder ein
Instrument geändert haben. `spiele` wirft die Uhr erst an, wenn alles da ist (`wartetAufToene`), der
Takt startet sie danach; `pausiere`/`stoppe` brechen das Warten ab; auch der Weg „lief vor dem
Ausschalten, läuft beim Einschalten weiter“ wartet. In der `Tonfuehrung` holt jede `LoopQuelle`
ihre Klänge über einen Tonauftrag: aus dem Speicher sofort, sonst im Hintergrund — ohne Programm
(neuer Clip, D) spielt sie mit Lücken, nach einer Änderung (C) das bisherige Programm; je Takt
schaltet sie um, sobald der Auftrag fertig ist (ein anderer Durchlauf gilt als Sprung). Abgebrochene
Aufträge laufen aus, ohne die Oberfläche zu blockieren. Das Feld Zeit zeigt TV7 a A, C (Akzentfarbe)
und D (Warnfarbe `#d8b400` wie im Dialog *Match loudness*); Play zeigt beim Warten Pause, ein Klick
bricht ab. **ClipLautheit** blieb, wie sie ist: sie lädt über `ladeKlaenge` und damit seit 29b aus
dem Speicher. Tests: in `test_LoopSpieler` D und C an der Tonführung, in `test_ComposerPanel` „S142
Panel 29e“ (Laden mit Vorladen, Play wartet, Pause bricht ab, die Uhr läuft an). **Nur unter
Release-clang geprüft** (Composer 151 / 151, Bereich 498 / 498) — Testing-MSVC weggelassen (Patrik,
zum Abschluss). Im Programm nicht angesehen.

✅ **TS1 a** (Patrik S143, nach Skizze): **Die Grenze in den Settings mit Stand und Clear.**
Settings → Panels, unter *Chosen Sample Files*: „**Loop Sound Memory:** [0.25 bis 64 GB, Schritt
0.25, Vorgabe 1.00 GB] · *in use: 312 MB · 85 sounds* · [**Clear**]“; der Stand schreibt jede Sekunde
nach, *Clear* leert den Speicher (wer einen Ton hält, behält ihn). Eine kleinere Grenze gilt sofort.
Grund: eine Grenze lässt sich nur mit Blick auf den Stand sinnvoll einstellen, und ohne *Clear* ließe
sich das Warten nach dem Öffnen (TV5 a, TV7 a) im Sichttest nur über einen Neustart ein zweites Mal
prüfen. Verworfen: b (nur das Feld — die Grenze bliebe eine geratene Zahl). Kippt, wenn *Clear* mit
*Clear frozen clips* (F5 a) gebündelt werden soll — dann wandert der Knopf zu Freeze.

**Gebaut 29f (S143):** `gemerkteTonspeicherGrenzeMb` / `merkeTonspeicherGrenzeMb` in
`loop/Tonspeicher` (Einstellung `loop/soundMemoryMB` in MB, auf 256 bis 65 536 geklemmt; die
Vorgabe wird nicht eingetragen); `MainWindow` gibt die Grenze beim Start an den Speicher, vor allem,
was Töne vorlädt. Das Feld übernimmt einen getippten Wert erst mit Enter oder beim Verlassen
(`keyboardTracking` aus) — sonst verwürfe eine halb getippte Zahl Töne. Handbuch 1.198.0 (§8, §15
Feld Zeit, §16 „Die Töne werden vorbereitet“), Sichttest-Liste `.claude/handover/Sichttest_ToeneVorab.md`
mit der neuen Vorlage `asset/sichttest/composer_M_toene.lvcomp` (`lanes_demo` und `tabs_demo`
nacheinander; Zusage in `test_ComposerVorlagen`). Test in `test_LoopTonspeicher` „29f“ (die
Einstellung, die Zeile im Settings-Panel: Grenze gilt sofort und wird gemerkt, Stand, Clear, ein
neues Panel zeigt die gemerkte Grenze). **Nur unter Release-clang geprüft** (Bereich
`*Loop*,*Instrument*,*Shape*,*Palette*,*Drum*` 499 / 499, Composer 151 / 151; Testing-MSVC
zurückgestellt, Patrik S143). Im Programm nicht angesehen.

## Versionen

| Version | Datum | Änderung |
|---|---|---|
| 0.78.0 | 2026-10-24 | Session 149. Befund Modenüberlappung (M = 1 bei rund 1 kHz); BM7 a (Grenzfrequenz = höchste gerechnete Mode, 31c-6 prüft Konvergenz und Prüfstein bandweise und senkt die Grenze nur, wo ein Band versagt); Hinweis Patrik zur immer feineren Teilung; PL31 a (erst 31c abschließen, vorher die zwölf Umfangsfragen von 31c-6 bis Abschluss klären, fester Plan; danach 31d–31h ebenso); Recherchen Mobilität über 1 kHz und Glas; Befund Prüfstein und Bauzustand; PS1 a (drei Prüfungen der Mobilität mit Toleranzen); Befund MAESSTRO Abb. 4; PF1 a (Prüffall aus unserem Bauplan mit den Vereinfachungen von MAESSTRO); Befund Konvergenztest MAESSTRO Gl. 2; KV1 a (zweite Rechnung k = 4, Toleranzen je Terzband); AW1 a (Antworten je Taste in der App aus der Körperdatei, im Tonspeicher, Messteil zu Beginn von 31c-7); FT1 a (Faltung je Ton mit pocketfft hinter `Fft`, echtes Stereo); PG1 a (fester Bezug für den Schalldruck, Antworten unnormiert); SB1 a (`soundboard` trägt nur `body`, `amount` fällt weg, `.lvinst` Fassung 17); DM1 a (η je Mode nach Verformungsenergie je Werkstoff); GL1 a (Floatglas, η 1·10⁻³, zwei Dicken); VA1 a (31c-8: MP1–MP3 und zwei Glasböden, Hörproben je Familie); ZA1 a (31c-9 Abschluss: Bereich und voller Lauf unter beiden Compilern, Hörproben, Doku); Befund erste Moden (Corradi Stufe 3); EM1 a (Kipppunkt von BM1 a neu gefasst); fester Plan IP1 und 31c-6 bis 31c-9; gebaut 31c-6 Teil 1 (DM1 a). |
| 0.77.0 | 2026-10-23 | Session 148. BM6 a (Hörorte als Liste im Bauplan, je Hörort ein Abstrahlgewicht je Mode; Lage der Vorgabe in BM6-1); Befund Mikrofone von *Salamander*; BM6-1 a (Stereopaar 12 cm über den Saiten an den Stegpunkten von MIDI 43 und 86); PL31c-5 a (31c-5a bis 31c-5d, statistischer Teil dabei); 31c-5a gebaut (Hörorte, `.lvinstbodyplan` Fassung 2); Befund zur Dämpfung (RT-0425 Gl. 1 als Polynom in f); 31c-5b gebaut (Auswertung je Mode); Befund Modendichte und Abstände; ST1 a (statistischer Teil aus den gerechneten Moden, Ziehung nach Boutillon & Ege); 31c-5c gebaut (5933 Moden bis 20 kHz); 31c-5d gebaut (Körperdatei, `steinway_d.lvinstbody` 2,9 MB, Wächter) — 31c-5 fertig. |
| 0.76.0 | 2026-10-23 | Session 148. Spectra 1.2.0 als External; EG1 a (Schnittstelle des Eigenlösers: Teilfenster im Löser, Moden an einen Empfänger, Zählung nach Sylvester); EG2 a (`SimplicialLDLT` in eigenem Operator, Residuum je Mode; SparseLU und Superknoten als Alternativen, wenn nötig). |
| 0.75.0 | 2026-10-23 | Session 147. EL1 a (Lagrange-Dreiecke der Ordnung k, Vorgabe 3); 31c-3 gebaut (Eigen als External, Platte nach Reissner-Mindlin gegen Navier gemessen, Steinway bis 5 kHz gebaut). |
| 0.74.0 | 2026-10-23 | Session 147. BM4 a (CDT 2.0.1 hinter einer eigenen Schnittstelle, eigener Generator vielleicht später); 31c-2 gebaut (Netz, CDT als External, Steinway bis 5 kHz gemessen). |
| 0.73.0 | 2026-10-23 | Session 147. IC1 a (Icons P1 und K1); 31c-1 fertig mit den Icons. |
| 0.72.0 | 2026-10-23 | Session 147. 31c-1 gebaut (ohne Icons): Bibliothek `InstrumentBody` mit dem Bauplan, `loop/KoerperBauplan`, der Steinway D aus RT-0425 Abb. 2 und Anhang A (`make_steinway_d.py`); Befunde: Farbskalen mit Zahlen, Umriss aus (a), Stegpunkte auf dem Steg, Holz nach Chabassier 2013 Tab. I, A0–B0 und C8 verlängert, Masse 11,5 kg. |
| 0.71.0 | 2026-10-23 | Session 147. BM5-1 a (Eigen 3.4.1 und Spectra 1.2.0 als Externals hinter einer Schnittstelle; Tests gegen die Schnittstelle mit Bezugswerten und Vergleich alt gegen neu; eigene Modul-Doku). |
| 0.70.0 | 2026-10-23 | Session 147. BM5 a (Rechenkern als Bibliothek `InstrumentBody` ohne Qt, Werkzeug `InstrumentBodyTool`, die App liest nur Körper); Frage VST mit Antwort, ohne Umsetzung. |
| 0.69.0 | 2026-10-23 | Session 147. BM3 a, BM3-1 a, BM3-2 a (Bauplan `.lvinstbodyplan` und Körper `.lvinstbody` als JSON unter `asset/loop/instrument_bodies/`, Abdruck des Bauplans im Körper, Verweis `lib:` aus der `.lvinst`; Icons dazu). |
| 0.68.0 | 2026-10-23 | Session 147. Wunsch Glas mit Befund Schimmel K 213 Glas (Acrylgehäuse, Fichtenboden); GB1 a (Glasboden in 31c-8, Schimmel als Wunsch bis Rand und Deckel; Material je Teil, frei kombinierbar). |
| 0.67.0 | 2026-10-23 | Session 147. BM2 a (Geometrie aus RT-0425 Abb. 2 per Farbe, MAESSTRO-Fassung als Prüffall für 31c-6 und 31c-8). |
| 0.66.0 | 2026-10-23 | Session 147. Befund RT-0425 Abb. 2 (Geometrie in Metern); PL31c a (31c-1 bis 31c-8, Fragen BM2–BM7 je Schritt, Rückwirkung auf die Saite in 31e); HT7 b (Boden aus dem Modell, Salamander als Vergleich für 31d). |
| 0.65.0 | 2026-10-23 | Session 147. Befund: die Netzdatei zu RT-0425 ist nicht mehr zu haben, MAESSTRO rechnet einen vereinfachten Steinway D (Tab. 1–2, Abb. 4); BM1 a (die Moden des Flügelbodens gerechnet, eigenes FEM-Werkzeug; Konstruktion als Datei, Varianten bei Bedarf, Editor offen). |
| 0.64.0 | 2026-10-23 | Session 146. HT6 a (Stufen bleiben); Befund vor 31c; HT7 a vorläufig (Boden aus Salamander) mit Auftrag zur Recherche für b; Wunsch „Deluxe“; Befund Recherche Boden und Körper (sieben Agenten, Berichte unter `docs/modellierung/`); KB1 a (gemeinsame Grundlage für Körper, Erweiterungen je Instrument); die Herleitung zu HT3 a nach `docs/modellierung/hammer-saite/modell.md`. |
| 0.63.0 | 2026-10-23 | Session 146. HT5 a (die Auskunft `InstrumentVon` mit Saiten und Hammer); 31b-3 gebaut (Stufe in der Planung, Rest als Pegel) und Rechenzeit gemessen. |
| 0.62.0 | 2026-10-23 | Session 146. Befund 31b-2 (die Längskraft mit gleichem Gewicht beherrscht den Klang; Auslenkung gegen Chabassier Tab. IV; Kopplung am Steg mit sin α nach Gl. 21); HT4 a (Kurve Bridge angle); 31b-2 gebaut. |
| 0.61.0 | 2026-10-23 | Session 146. Befund Chor und zweistufiger Ausklang (Bank, Diss. 2006, Kap. 4.2); HT3 a (gekoppelte Moden nach Weinreich, Kurve Late decay) mit Herleitung; 31b-1 gebaut und gemessen (`loop/HammerTon`). |
| 0.60.0 | 2026-10-23 | Session 146. Befund vor 31b (Bank 2010 im Volltext: Resonatoren, Hammer, Längsschwingung, Nebenresonatoren, Dämpfer; die Kurven aus 31a tragen keine Saitendaten); HT1 a (Länge, Masse, Längsverhältnis als Kurven); Befund Saitendaten (Chabassier, Joly & Chaigne 2013 Tab. III; Chabassier & Duruflé RT-0425 Anhang A, 84 Tasten); HT2 a (Hammer nach RT-0425 statt Stulov); 31a nachgezogen. |
| 0.59.0 | 2026-10-23 | Session 146. Befund Vorgaben zu 31a (Stulov Gl. 9 und Rigaud Gl. 4/5 am Bild gelesen; p 3,7…5,0 statt des Agenten-Berichts 2…4; kein Beleg für die erste Taste ohne Dämpfer); HK1 a (Skala je Größe, monoton kubisch), HK2 a (laut ∝ Hammergeschwindigkeit, 5 m/s, 1 dB, −40 dB, `:a<stufe>` statt `:v`); 31a gebaut (`loop/HammerSaite`, Fassung 16). |
| 0.58.0 | 2026-10-22 | Session 145. Stufe 31 begonnen: Wunsch Beispiel-Loops, BL1 a (drei Chopin-Ausschnitte), BL2 a (von Hand aus gemeinfreier Ausgabe vor 1900), Befunde Quellen (Mutopia, Polona, IMSLP), Befund Code und Literatur, HS1 a (modal nach Bank 2010), HS2 a (Teilschritte 31a–31h); zu 31a HA1 a (Kurven über die Tastatur), HA2 a (Excitation und Modal an der Schicht, Block Keyboard am Instrument), HA3 a (Anschlagstärke gestuft im Namen), HA4 a (Ausstattung Keyboard). |
| 0.57.0 | 2026-10-22 | Session 145. Stufe 30i-3 (Handbuch, Sichttest-Liste, Bereichslauf unter clang); Stufe 30 fertig bis auf den vollen Lauf. |
| 0.56.0 | 2026-10-22 | Session 145. Stufe 30i-2 gebaut (Publish- und Import-Dialog mit eingefrorenen Clips). |
| 0.55.0 | 2026-10-22 | Session 145. Plan 30i; Stufe 30i-1 gebaut (Wellenformen im Paket, Import prüft den Abdruck). |
| 0.54.0 | 2026-10-22 | Session 145. Befund vor 30i (Publish und Abdruck); FP1 a (welche Wellenformen ins Paket gehen); FP2 a (der Publish-Dialog); FP3 a (der Import prüft). |
| 0.53.0 | 2026-10-22 | Session 144. Stufe 30h gebaut (Rechenzeit je Clip, Sanduhr, Settings *Freeze hint*). |
| 0.52.0 | 2026-10-22 | Session 144. FZ5 a (Messung von Freeze nach Skizze A–D); FZ6 a mit Zusatz (Summe der Töne, umschaltbar auf geteilt durch die Threads); FZ7 a (Schalter in der Settings-Zeile). |
| 0.51.0 | 2026-10-22 | Session 144. Stufe 30g-2 gebaut (Marke, Menü, Undo, Speichern, Hinweiszeile; Kurzform „verändert“ im Warndreieck, Patrik). |
| 0.50.0 | 2026-10-22 | Session 144. Plan 30g; Stufe 30g-1 gebaut (`FreezeAuftrag`, Marken, Zustand). |
| 0.49.0 | 2026-10-22 | Session 144. FZ4 a (Bedienung von Freeze nach Skizze A–D). |
| 0.48.0 | 2026-10-22 | Session 144. Verdacht Tonführung Fall C mit Test belegt und behoben. |
| 0.47.0 | 2026-10-22 | Session 144. Stufe 30f-3 gebaut (Laden: Spieler, Composer, Vorladen, Lautheit); Verdacht Tonführung Fall C (noch ohne Test). |
| 0.46.0 | 2026-10-22 | Session 144. §4.4 Bauplan: Hammer-Saite wieder Stufe 31 ff., der Audio-Clip im Loop danach (Patrik). |
| 0.45.0 | 2026-10-22 | Session 144. Stufe 30f-2 gebaut (Rechenkern: eingefrorene Clips als eine Stimme, ohne Schnitt über die Naht). |
| 0.44.0 | 2026-10-22 | Session 144. Stufe 30f-1 gebaut (`frozen` am Clip, `.lvloop` 13). |
| 0.43.0 | 2026-10-22 | Session 144. Plan 30f (drei Teilschritte); FV1 a (der Nachklang eines eingefrorenen Clips klingt über die Naht); FV2 a (`frozen` trägt den Abdruck des eingefrorenen Stands). |
| 0.42.0 | 2026-10-22 | Session 143. FZ3 a (Zeile *Frozen Clips*, Skizze); Stufe 30e gebaut (Ordner `frozen`, Grenze, Settings). |
| 0.41.0 | 2026-10-22 | Session 143. FZ1 a (Freeze in sechs Teilschritten 30d–30i); Stufe 30d gebaut (`loop/Freeze`, Wächter der Klangfassung); Befund Pfade im Abdruck, FZ2 a (Dateien über ihren Inhalt, gebaut). |
| 0.40.0 | 2026-10-22 | Session 143. WX6 a (Fenster *Export audio*, Skizze), WX7 a (auch der ganze Loop, Frage Patrik), WX8 a (Knopf in der Datei-Zeile), WX9 a (Ending, Passes); Stufe 30c gebaut. |
| 0.39.0 | 2026-10-22 | Session 143. Stufe 30b gebaut (`audio/WavDatei`, ein Schreiber auch für die Aufnahme, `test_WavDatei`). |
| 0.38.0 | 2026-10-22 | Session 143. Befund Lautstärke des Loops vor den Effekten; WX5 a (nach den Effekten, gebaut); HB1 a (ins Handbuch); Stufe 30a gebaut (`loop/Wellenform`, `test_LoopWellenform`). |
| 0.37.0 | 2026-10-22 | Session 143. WX2 a (WAV, 32-bit float als Vorgabe, 24 und 16 Bit wählbar); WX3 a mit Zusatz (Loop-Clip im Composer: Länge, Versatz, Fader des Loops — ohne Gain, Blenden, Pegelkurve und Limiter); WX4 a (Teilschritte 30a–30d ff.). |
| 0.36.0 | 2026-10-22 | Session 143. TS1 a (Loop Sound Memory mit Stand und Clear); Stufe 29f gebaut (Grenze in den Settings, Handbuch, Sichttest-Liste). |
| 0.35.0 | 2026-10-21 | Session 142. TV7 a (das Warten im Composer im Feld Zeit, Skizze A–D); Stufe 29e gebaut (Vorladen in der Sitzung, Play wartet, Loop-Quellen der Tonführung über den Tonauftrag). |
| 0.34.0 | 2026-10-21 | Session 142. Stufe 29d gebaut (Spieler über den Tonauftrag, Warten, Umschalten, Vorladen, Anzeige im Loop-Panel und im Instrument-Editor). |
| 0.33.0 | 2026-10-21 | Session 142. TV5 a (Anzeige des Wartens an der Stelle von „Bar · Step“, Skizze A–D); TV6 a (Anspielen über den Hintergrund, sofort aus dem Speicher). |
| 0.32.0 | 2026-10-21 | Session 142. Stufe 29c gebaut (`loop/Tonauftrag`, `ladeKlaenge` als Auftrag auf dem rufenden Thread, `test_LoopTonauftrag` mit dem BASS-Beleg). |
| 0.31.0 | 2026-10-21 | Session 142. Idee Patrik Export eines Clips als Wellenform; WX1 a (Stufe 30: erst die Rechnung, dann Export, dann Freeze); Audio-Clip im Loop als Stufe 31 vorgemerkt (Patrik), Hammer-Saite 32 ff. |
| 0.30.0 | 2026-10-21 | Session 142. TV4 a (Programme halten geteilte Zeiger auf die Töne); Stufe 29b gebaut (`loop/Tonspeicher`, `ladeKlaenge` über den Speicher, Typ `Klaenge`, `test_LoopTonspeicher`). |
| 0.29.0 | 2026-10-21 | Session 142. Stufe 29a gebaut (`instrumente()` mit Sperre, Fassung atomar, `test_LoopTonThreads`). |
| 0.28.0 | 2026-10-21 | Session 142. §4.5 Stufe 29: Befund Threadsicherheit von `rendereTon` (Kern ja, Rand nein), TV1 a (sechs Teilschritte 29a–29f), TV2 a (Schlüssel als Fingerabdruck der Eingaben), Befund BASS, TV3 a (Dekodieren im Hintergrund, Drum-Klänge im Tonspeicher). |
| 0.27.0 | 2026-10-21 | Session 141. §4 Familie Hammer-Saite: Zielbild (alle Instrumente, alle Teile, alle Spielweisen; Geräusche abschaltbar), freie Aufnahmen (Salamander, CC-BY 3.0), Befund „wann ein Ton gerechnet wird“; H1 a bis H3 a (Töne vorab: Speicher je Tonname, Hintergrund, Vorladen, Play wartet, 1 GB im Arbeitsspeicher); vorgemerkt Compute-Shader; Idee Patrik Freeze mit F1 a bis F5 a; Bauplan F6 a (29 Töne vorab, 30 Freeze, 31 ff. Hammer-Saite). |
| 0.26.0 | 2026-10-21 | Session 141. Stufe 28f bis auf den vollen Lauf (Beispiel `lanes_demo`, Handbuch, Sichttest-Liste). |
| 0.25.0 | 2026-10-21 | Session 141. SK13 a (Kurve am markierten Clip); Stufe 28e gebaut. |
| 0.24.0 | 2026-10-21 | Session 141. Stufe 28d gebaut (Zeile *Lanes*, Spuren im Raster, bearbeiten). |
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

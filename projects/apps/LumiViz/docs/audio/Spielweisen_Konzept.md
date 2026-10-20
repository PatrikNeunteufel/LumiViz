# LumiViz — Spielweisen der Instrumentfamilien (Konzept)

> **Version:** 0.8.0
> **Datum:** 2026-10-20 (Session 139: Zielbild, SF1 c, SF2 a, SF3 a, SF4 a, SF5 d;
> Session 140: SF6 a, SF7 a, SF8 a)
> **Typ:** Konzept — Ziel, Modell, Klang. Entschieden sind SF1 c bis SF8 a (Patrik S139, S140);
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
Rechnung).

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
ausklappbar, in der `.lvloop`; **Sustain** und **Sostenuto** als Stufen 0 … 1 (auch das
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
in der Recherche ihrer Familie belegt.

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

## 2. Offen

- Die Reihenfolge der Familien; je Familie Recherche, Modell, Messung, Hörproben wie
  im Saitenstrang (S132–S138).
- Je Klang-Spielweise der Ort nach SF8 a — in der Recherche ihrer Familie.
- Die Zeit zum Umstecken eines Dämpfers: belegen, dann entscheiden.

## Versionen

| Version | Datum | Änderung |
|---|---|---|
| 0.8.0 | 2026-10-20 | Session 140. SF8 a (drei Orte der Klang-Spielweisen: Ausstattung, Spur je Muster, Artikulation an der Note). |
| 0.7.0 | 2026-10-20 | Session 140. SF7 a (Spuren aller Schichten zusammen; *Follow* folgt bei den gemeinsamen, die übrigen rechnet die Schicht selbst). |
| 0.6.0 | 2026-10-20 | Session 140. SF6 a (Bauart als Obergrenze, Ausstattung je Instrument; Tabelle der Steuer-Spielweisen je Bauart, noch unbelegt). |
| 0.5.0 | 2026-10-20 | Session 139. SF5 d (fünf Spuren je Muster, dazu Expression am Clip, malgenommen). |
| 0.4.0 | 2026-10-20 | Session 139. SF4 a (Vibrato der Schicht als Vorgabe, die Spur setzt die Tiefe). |
| 0.3.0 | 2026-10-20 | Session 139. SF3 a (fünf Steuer-Spielweisen, Beleg der MIDI-Belegung). |
| 0.2.0 | 2026-10-20 | Session 139. SF2 a (die Bauart bestimmt die Steuer-Spielweisen eines Modells); Antwort zu den Saiten. |
| 0.1.0 | 2026-10-20 | Session 139. Angelegt: Zielbild (alle Familien, Klang nach echten Instrumenten), drei Arten von Spielweisen als Vorschlag, SF1 c. |

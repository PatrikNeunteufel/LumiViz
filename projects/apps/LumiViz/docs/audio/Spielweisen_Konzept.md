# LumiViz — Spielweisen der Instrumentfamilien (Konzept)

> **Version:** 0.2.0
> **Datum:** 2026-10-20 (Session 139: Zielbild, SF1 c, SF2 a)
> **Typ:** Konzept — Ziel, Modell, Klang. Entschieden sind SF1 c und SF2 a (Patrik S139);
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

**Drei Arten von Spielweisen** (Vorschlag Claude S139, zum Ort im Modell; noch
nicht entschieden):

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

**An den Saiten** (Antwort Claude S139, Vorschlag): Vibrato über der Zeit (über die
Spannung, wie der Bend) und Swell; die **Bogen-Saite** als neue Bauart auf der Saite
(Saitensatz, Korpus) für `violin`, `strings`, `upright_bass` — Pizzicato wird dann eine
Spielweise der Geige; das Dämpferpedal am Hackbrett (`dulcimer`). Wie das neue Vibrato
sich zur festen Einstellung je Schicht verhält, ist ein eigener Entscheid.

## 2. Offen

- Die Liste der Steuer-Spielweisen (SF1 c, Gruppe 1) und je Bauart (SF2 a); das
  Verhältnis zum Vibrato je Schicht.
- Die Reihenfolge der Familien; je Familie Recherche, Modell, Messung, Hörproben wie
  im Saitenstrang (S132–S138).
- Der Ort je Spielweise (Bauart, Spur, Note) — einzeln.
- Die Zeit zum Umstecken eines Dämpfers: belegen, dann entscheiden.

## Versionen

| Version | Datum | Änderung |
|---|---|---|
| 0.2.0 | 2026-10-20 | Session 139. SF2 a (die Bauart bestimmt die Steuer-Spielweisen eines Modells); Antwort zu den Saiten. |
| 0.1.0 | 2026-10-20 | Session 139. Angelegt: Zielbild (alle Familien, Klang nach echten Instrumenten), drei Arten von Spielweisen als Vorschlag, SF1 c. |

# Offene Fragen — Membran (Klasse 5)

> **Gebündelt 2026-10-29 (IC Session 3):** die offenen Fragen dieser Datei sind in [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md)
> nach Themen zusammengefasst — dort beantworten; die Antwort wird hierher übertragen.
>
> **Stand:** 2026-10-26, IC Session 1. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz (nachgelesen 2026-10-26) unter `…\LumiViz\projects\apps\LumiViz\docs\audio\`: `Loop_Konzept.md`
> (§4a, §4d, §4l), `Spielweisen_Konzept.md`. Fragen, die Becken und Gong teilen, stehen hier und werden in
> [klassen/Klasse 6 - Becken und Gong/offene-fragen.md](../../becken-gong/ic/offene-fragen.md) nur verwiesen (F-2, F-3, F-4).

---

## F-1 — Umfang und Reihenfolge

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T8): a.**

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 5. **Heutiger Wortlaut:** „Pauke, Toms, Bassdrum, Snare (mit Teppich),
Conga, Bongo, Tabla“.

**Problem:** Am besten gemessen sind die Toms (144 Schläge mit Pendel in 16 Stimmungen, CC BY 4.0) und die Pauke
(Moden und T60 gemessen bei Packer, CC0-Aufnahmen); die Snare hat die meisten Anschläge, aber den Teppich als teuersten
und unsichersten Teil; Conga und Bongo haben keine Modenmessung, die Tabla keine Luftlast im Modell.

**a (Empfehlung):** „Zuerst der Tom (Fell modal mit Luft, zwei Felle, Schlag — gesetzt am Tom-Tom-Datensatz), dann die
Pauke (Luftlast als Prüfstein gegen Packer), dann Bassdrum, dann die Snare mit Teppich, zuletzt Conga, Bongo und Tabla.“

**b:** „Zuerst die Snare — sie ist im Kit am häufigsten und hat die meisten Aufnahmen.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass der Kern (Moden mit Luft, Kopplung, Kontakt) an dem Instrument mit
kontrolliertem Anschlag und bekannter Stimmung gesetzt wird; an der Snare mischt sich der Teppich hinein, ein Fehler wäre
nicht zuzuordnen. Für b spricht die Häufigkeit im Kit — das wiegt weniger, weil die Snare nach Tom und Bassdrum nur den
Teppich dazu braucht. **Kippt auf b**, wenn Patrik die Snare zuerst hören will (Hörprobe vor dem Ausbau).

---

## F-2 — Anschlagstärke: Lautstärke je Schritt als Auftreffgeschwindigkeit, gestuft im Namen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a.**

**Ort:** `Loop_Konzept.md` §4d („Step volume“); `Spielweisen_Konzept.md` **HA3 a**. **Heutiger Wortlaut:** HA3 a —
„Die Anschlagstärke gestuft im Namen, der Rest als Pegel“ (nur bei einem Instrument mit Hammer); im Drum-Editor „die
Lautstärke des Schritts“.

**Problem:** Mit der Stärke ändert sich am Fell nicht nur der Pegel: die Kontaktzeit wird kürzer, die Kraft steigt
„from 3 N (piano) up to slightly over 100 N (forte)“ (Wagner, nachgelesen), die Tonhöhe gleitet stärker. Heute ist die
Lautstärke je Schritt ein Pegel auf einem festen Klang.

**a (Empfehlung):** „Bei einer gerechneten Trommel ist die Lautstärke des Schritts die Auftreffgeschwindigkeit. Die Stärke
wird wie HA3 a auf Stufen gerundet und steht im Namen des vorab gerechneten Tons; den Rest gleicht ein Pegel aus. Die
Abbildung Lautstärke → Geschwindigkeit ist eine Kurve des Instruments, gesetzt an den gestaffelten DrumGizmo-Anschlägen.
Gilt ebenso für Becken und Gong (Klasse 6), dort mit feineren Stufen.“

**b:** „Ein gerechneter Schlag je Rolle; die Lautstärke des Schritts bleibt ein Pegel.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass ein leiser Schlag anders klingt als ein leise gedrehter lauter — a
übernimmt dafür den Weg, den LumiViz beim Flügel schon entschieden hat (HA3 a), ohne neues Feld am Schritt. Für b spricht
der kleinere Tonspeicher — das wiegt weniger, weil die Stufen gemessen werden und ein Schlag nur Sekundenbruchteile
Rechnung kostet. **Kippt auf b**, wenn Hörproben zwischen Stufen keinen Unterschied außer dem Pegel zeigen (an der Snare
unwahrscheinlich).

---

## F-3 — Spielweisen am Schritt: Schlagstelle, Rimshot, Cross-Stick, Abdämpfen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a.**

**Ort:** `Loop_Konzept.md` §4d (Schritt = an/aus + Lautstärke), **K5 a** (feste Rollen); `Spielweisen_Konzept.md`
**SF8 a** (je Note: „Wirbel, Abdämpfen“). **Heutiger Wortlaut:** K5 a — „Feste Rollen, nummeriert mehrfach: kick,
snare, clap, rim, hat, open hat, ride, crash, tom …“; SF8 a — „Je Note … eine Artikulation an der Note, wie Palm mute“.

**Problem:** Schlagstelle (Mitte/Rand), Rimshot, Cross-Stick, gedämpfter Schlag sind Spielweisen derselben Trommel und
wechseln von Schlag zu Schlag; im Drum-Editor hat ein Schritt dafür keinen Ort. Cross-Stick deckt die Rolle `rim` schon
ab, Rimshot und Schlagstelle nicht.

**a (Empfehlung):** „Ein Schritt erhält eine Artikulation wie die Note bei SF8 a (Vorgabe ‚normal‘): Snare und Toms
*rimshot*, *edge* (Rand), *center*, *muted*; Cross-Stick bleibt die Rolle `rim`. Ein Kit nennt je Rolle den Klang; eine
gerechnete Trommel rechnet die Artikulation, ein Sample-Klang ohne sie spielt ‚normal‘. Becken (Klasse 6) nutzen
dieselbe Stelle für Kuppe/Rand und *choke*.“

**b:** „Jede Spielweise wird eine eigene Rolle bzw. Spur (etwa `snare 2` für den Rimshot, wie heute der Akzent der
Besen in Jazz Brushes).“

**Warum die Empfehlung:** Den Ausschlag gibt SF8 a: was je Schlag wechselt, gehört an den Schlag, nicht in eine zweite
Spur — sonst gehen Rimshot und Schlag beim Kit-Wechsel getrennte Wege, und eine Folge Mitte–Rand–Mitte braucht zwei
Spuren für ein Instrument. Für b spricht, dass es ohne Änderung am Drum-Editor geht — das wiegt weniger, weil b das
Rollen-Schema mit Spielweisen statt Instrumenten füllt. **Kippt auf b**, wenn LumiViz am Drum-Editor kein Feld je Schritt
haben will (Entscheid dort).

---

## F-4 — Wirbel als gerechneter Ton über die Notendauer

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a.**

**Ort:** `Spielweisen_Konzept.md` Zielbild (Schlagwerk „Wirbel“), **SF8 a**; `Loop_Konzept.md` §4d (Raster).
**Heutiger Wortlaut:** Wirbel je Note (SF8 a).

**Problem:** Ein Buzz-Wirbel hat Aufschläge im Abstand von grob 30–80 ms in Gruppen (Miura 2023, Gruppenabstand
nachgelesen; Rechnung im Bericht); das Schrittraster bildet das nicht ab, und einzelne Schritte würden das Fell jedes Mal
neu starten statt auf das noch schwingende Fell zu schlagen.

**a (Empfehlung):** „Wirbel ist eine Artikulation mit Dauer am Schritt (Art: Einzelschlag oder Buzz; die Rate aus dem
Instrument, die Dichte mit der Stärke). Der Wirbel wird als ein Ton über die ganze Dauer gerechnet, mit allen Aufschlägen
auf demselben schwingenden Fell und gestreuter Zeit und Stärke.“

**b:** „Ein Wirbel wird mit vielen Schritten geschrieben; jeder Schritt ist ein eigener Schlag.“

**Warum die Empfehlung:** Den Ausschlag gibt die Physik: im Wirbel schwingt das Fell weiter, jeder Aufschlag trifft es
in Bewegung — nur ein durchgehender Ton gibt das wieder. Für b spricht, dass es heute schon geht — das wiegt weniger,
weil b einen Buzz nicht schreiben kann und Einzelschläge abschneidet. **Kippt auf b**, wenn LumiViz keine Dauer am
Drum-Schritt kennt und nicht bekommen soll.

---

## F-5 — Pauke: Instrument mit Tonhöhe und Pedal-Glissando

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a.**

**Ort:** `Loop_Konzept.md` K5 a (Rollen), 11f („Alle sechs sind für die Note C2 gebaut“); `Spielweisen_Konzept.md`
SF5 d (Pitch-Spur). **Heutiger Wortlaut:** gerechnete Drums spielen eine feste Note in einer Rolle.

**Problem:** Die Pauke spielt Tonhöhen (je Kessel etwa eine Quinte, nur Bericht) und gleitet mit dem Pedal; eine Rolle mit
fester Note kennt weder Tonhöhe noch Verlauf. Gemessen ist das Glissando nirgends.

**a (Empfehlung):** „Die Pauke ist ein Instrument im Muster (wie Klavier oder Bläser), nicht eine Drum-Rolle: die Note
gibt die Tonhöhe, die Pitch-Spur das Pedal-Glissando (das Modell ändert die Spannung über die Zeit), der Umfang je Kessel
steht in der Ausstattung. Wirbel und Abdämpfen wie F-4 und F-3.“

**b:** „Die Pauke bleibt eine Drum-Rolle (`perc`), je Kessel eine feste Note, ohne Glissando.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Pauke im Orchester gestimmt und melodisch geführt wird — eine
Rolle mit fester Note bildet das nicht ab, die Pitch-Spur gibt es schon. Für b spricht, dass Kits einfach bleiben — das
wiegt weniger, weil die Pauke selten im Kit steht. **Kippt auf b**, wenn LumiViz Pauken nur als Kit-Klang braucht.

---

## F-6 — Snare-Teppich: Mischform oder volle Rechnung

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T8): a.**

**Ort:** [modell-entwurf.md](modell-entwurf.md) §2.6. **Heutiger Wortlaut:** neu.

**Problem:** Der Teppich ist ein verteilter, nichtlinearer Kontakt vieler Drähte am Resonanzfell; volle Finite
Differenzen kosten nach den Pauken-Zahlen Minuten je s Ton (82 s auf der GPU schon ohne Teppich, nachgelesen).

**a (Empfehlung):** „Im Standard: modale Felle, wenige Drahtmoden mit Straf-Kontakt am Resonanzfell, dazu ein
Rauschanteil, dessen Hüllkurve die Auslenkung des Resonanzfells steuert — gesetzt an Snare mit und ohne Teppich (VCSL).
Volle Rechnung (FDTD mit Drähten) als Deluxe. Teppich an/ab ist eine Spielweise über eine Passage.“

**b:** „Nur die volle Rechnung (FDTD), vorab je Stärke.“

**c:** „Der Teppich als reines Rauschen ohne Kontakt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass a den Mechanismus (Kontakt am Resonanzfell) behält und nur die vielen
Drähte statistisch fasst — das Rauschen folgt der Fellbewegung statt einer festen Hüllkurve. Für b spricht die volle
Physik — das wiegt weniger, weil sie für jede Stärkestufe Minuten kostet und an keiner Messung mit Drahtbewegung geprüft
werden kann. c verliert den Zusammenhang zwischen Fell und Teppich. **Kippt auf b**, wenn die Mischform das gemessene
Teppichrauschen (M6) nicht trifft.

---

## F-7 — Hand-Trommeln: Schlagarten und Tabla

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a, abgewandelt** — „nichts zurückstellen“: Conga und Bongo werden gebaut und als ‚ungemessen‘ gekennzeichnet (T8 a), gesetzt, sobald Modendaten des offenen Kessels vorliegen.

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 5 („Conga, Bongo, Tabla“); `Loop_Konzept.md` Kit *Hand drums*.
**Heutiger Wortlaut:** Hand drums aus `lib:djembe_low`, `lib:conga_high` u. a.

**Problem:** Hand-Trommeln leben von Schlagarten (Conga: open, slap, mute, bass; Tabla: Bols wie Na, Tin, Ge, Ke) —
gemessen ist davon wenig (Conga-Slap fehlt ganz); die Tabla ist eine belastete Membran mit harmonischen Moden (Raman,
Sathej & Adhikari), das Bayan gleitet mit dem Handballen (ungemessen).

**a (Empfehlung):** „Schlagarten sind Artikulationen am Schritt (F-3). Die Tabla wird als Paar Dayan/Bayan modal
gerechnet (belastete Membran), ihre Bols als Artikulationen; das Bayan-Gleiten folgt F-5 (Instrument mit Pitch-Spur),
sobald es gemessen ist. Conga und Bongo erst mit Modendaten des offenen Kessels.“

**b:** „Hand-Trommeln bleiben Samples (VCSL, Tabla strokes); nur Pauke und Kit werden gerechnet.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Tabla physikalisch gut beschrieben ist und Bols als Artikulationen
in denselben Ort fallen wie Rimshot und Choke. Für b spricht die dünne Datenlage bei Conga/Bongo — das wiegt bei der
Tabla weniger, deshalb stellt a nur Conga und Bongo zurück. *Nach der Regel „Pausieren nur mit Freigabe“:* a stellt
Conga und Bongo bis zu Modendaten zurück — hiermit vorgelegt. **Kippt auf b**, wenn LumiViz Hand-Trommeln nur im Kit
braucht.

---

## F-8 — Bezugsaufnahmen und Downloads

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): a.**

📁 **Ablage (Anweisung Patrik 2026-10-26, IC Session 2, A-1 a, für alle Klassen gleich):** Aufsätze kommen ins Repo nach `quellen/`; **Aufnahmen und Messdaten lokal** nach `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für LumiViz erreichbar), im Repo nur `aufnahmen/<Datensatz>/` bzw. `daten/<Datensatz>/` mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`; jeder Datensatz in [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). „Neben LumiViz“ im Text unten ist damit überholt; welche Dateien geladen werden, entscheidet diese Frage.

**Ort:** [aufnahmen.md](aufnahmen.md). **Heutiger Wortlaut:** neu.

**Problem:** DrumGizmo (CC BY 4.0, an der Quelle nachgelesen; Namensnennung auch in Kompositions-Credits) hat viele
Stärken mit Nahmikrofonen, aber 2,8 und 5,6 GB; LumiViz hält davon 37 Anschläge. Der Tom-Tom-Datensatz ist klein (90 MB).
`snare-position.zip` hätte Schlagstelle × Stärke, trägt aber keine Lizenz.

**a (Empfehlung):** „Bezug: Tom-Tom-Datensatz, DrumGizmo DRSKit und CrocellKit (nur die gebrauchten Instrumente über
Teil-Abrufe wie D1 a, alle Stärken einer Trommel), VCSL/VSCO für Pauke und Hand-Trommeln, Tabla strokes. Vor jedem
Laden legt Claude Dateien, Größen und Zielordner neben LumiViz zur Freigabe vor. `snare-position.zip` erst nach einer
Anfrage beim DrumGizmo-Team. NC/ND-Datensätze und synthetische Sätze werden nicht genutzt.“

**b:** „Nur die 37 Anschläge, die LumiViz schon hat.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass Stärke und Schlag nur an fein gestaffelten Anschlägen gesetzt werden
können (F-2) — drei Stufen je Instrument reichen dafür nicht. Für b spricht, dass nichts geladen werden muss — das wiegt
weniger, weil a nur die gebrauchten Teile holt. **Kippt auf b**, wenn F-2 b gewählt wird.

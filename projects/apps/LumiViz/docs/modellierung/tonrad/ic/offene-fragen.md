# Offene Fragen — Tonrad (Klasse 11)

> **Gebündelt 2026-10-29 (IC Session 3):** die offenen Fragen dieser Datei sind in [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md)
> nach Themen zusammengefasst — dort beantworten; die Antwort wird hierher übertragen.
>
> **Stand:** 2026-10-26, IC Session 1. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz (nachgelesen 2026-10-26) unter `…\LumiViz\projects\apps\LumiViz\docs\audio\`:
> `Spielweisen_Konzept.md`, `Loop_Konzept.md`; Instrumente unter `…\LumiViz\asset\loop\instruments\`.

---

## F-1 — Rechenweg: je Note vorab oder die Orgel als Ganzes über die Passage

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T2): a.**

**Ort:** `Spielweisen_Konzept.md` §4.2–§4.5 (Töne vorab: Tonspeicher, Freeze, Klangfassung). **Heutiger Wortlaut:**
Töne werden je Note vorab gerechnet; ein Modell, das je Note mit ihrer Spur rechnet, muss dort hineinpassen
(`AUFGABE.md`).

**Problem:** Die Tonradorgel ist über die Noten hinweg gekoppelt. Alle Tasten greifen auf dieselben 91 Räder zu — zwei
Noten auf einem Rad addieren sich phasengleich. Die Perkussion hat eine einzige Hüllkurve für das ganze Obermanual und
setzt erst wieder ein, wenn alle Tasten losgelassen sind; Scanner, Vorverstärker (leichte Kompression bei Akkorden) und
Leslie wirken auf die Summe und haben einen Zustand über die Zeit (beide Berichte; Werte nur Bericht). Je Note
vorab gerechnete Töne verlieren die Phasengleichheit und können Perkussion und Scanner nicht richtig abbilden.

**a (Empfehlung):** „Die Tonradorgel rechnet je Clip als Ganzes: die 91 Räder als periodische Wellen mit einer Phase
aus der Zeit, die Tasten über die Matrix aus Zugriegeln und Rädern, danach Perkussion, Scanner, Vorverstärker,
Schweller und Leslie auf der Summe. Vorab gespeichert wird der Klang des Clips (wie Freeze), abhängig von Noten,
Registrierung und Spuren. Billig genug, um bei jeder Änderung neu zu rechnen.“

**b:** „Je Note vorab wie heute (Generator, Klick, Übersprechen je Note); Perkussion als Variante je Note (mit/ohne),
Scanner und Leslie als Bausteine der Effektkette am Clip (E2 b).“

**Warum die Empfehlung:** Den Ausschlag gibt die Physik der Orgel — sie ist ein einziger Generator mit Summenstufen, und a
rechnet sie so; die Phasengleichheit gemeinsamer Räder und die Einzeltrigger-Perkussion entstehen nur dort richtig. Für
b spricht, dass es sich ohne neuen Rechenweg in den Tonspeicher fügt — das wiegt weniger, weil b genau die Merkmale
verliert, die eine Tonradorgel von einer additiven Sinus-Orgel unterscheiden (das ist `organ` heute schon). **Kippt auf
b**, wenn LumiViz für ein Instrument keinen Rechenweg je Clip zulassen will; dann wenigstens die Phase aus der Zeit (ein
Tonspeicher je Rad statt je Note).

---

## F-2 — Leslie: Ausstattung des Instruments, Ort und Zustand

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T2): a.**

**Ort:** `Spielweisen_Konzept.md` **SF10 a** („Hammond | — | Tonrad | Leslie“ in der Spalte Ausstattung), **SF8 a**
(„Leslie langsam/schnell“ über eine Passage, „das Modell rechnet den Übergang“); `Loop_Konzept.md` **E2 b** (Effekte am
Clip; „Nicht drin: … Effekte je Kanal“).

**Problem:** Der Leslie verstärkt die Summe aller Noten nichtlinear (Röhren) und dreht weiter, während Noten kommen und
gehen; sein Zustand (Winkel und Drehzahl von Horn und Trommel) läuft über die ganze Passage (Bericht Leslie). Ein Effekt
am Clip beginnt in jedem Clip neu — ein Leslie, der mitten im Stück steht, klingt anders als einer, der aus dem
vorigen Clip schnell weiterdreht. Gleichzeitig wird ein Drehlautsprecher auch hinter anderen Instrumenten gespielt
(Gitarre, E-Piano).

**a (Empfehlung):** „Der Leslie bleibt Ausstattung der Tonradorgel (SF10 a) und rechnet auf ihrer Summe (F-1 a); die
Spur ‚Leslie langsam/schnell‘ setzt den Sollwert, das Modell rechnet Hochlauf und Auslauf. Der Zustand der Rotoren läuft
über aufeinanderfolgende Clips derselben Orgel in einem Kanal weiter (ein Leslie je Kanal); ein Clip am Anfang des Stücks
beginnt in einem festen Zustand (langsam, Winkel 0). Derselbe Leslie steht zusätzlich als Baustein der Effektkette am
Clip für andere Instrumente zur Verfügung.“

**b:** „Jeder Clip beginnt mit einem festen Zustand des Leslie (wie die Effekte nach E2 b).“

**Warum die Empfehlung:** Den Ausschlag gibt, dass ein Leslie zwischen zwei Clips nicht anhält — wer schnell spielt und
in den nächsten Clip geht, erwartet die Rotoren in voller Fahrt; a hält das, und der Anfangszustand macht das Ergebnis
trotzdem eindeutig. Für b spricht, dass jeder Clip für sich gerechnet und eingefroren werden kann — das wiegt weniger,
weil der Zustand am Clipende eine kleine, berechenbare Größe ist, die der nächste Clip übernimmt. **Kippt auf b**, wenn
Clips in LumiViz unabhängig von ihren Nachbarn eingefroren werden müssen (Freeze je Clip ohne Vorgeschichte).

---

## F-3 — Expression als Schweller, Vibrato als Scanner

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T3a): a.**

**Ort:** `Spielweisen_Konzept.md` **SF10 a**. **Heutiger Wortlaut:** „Expression, Vibrato, Pitch nur bei dauernder
Anregung (Bogen, Lippe, Rohrblatt, Luftstrahl)“; das Tonrad ist ein Schwinger „ohne Anregung“.

**Problem:** Wörtlich hätte die Tonradorgel weder Expression noch Vibrato. Sie hat aber einen Schweller (Fußpedal) für
beide Manuale und das Pedal (Handbuch, nur Bericht) — eine dauernde Steuerung während des Spiels —, und ihr Vibrato ist
der Scanner mit fester Rate (6,87 Hz) und gewählter Tiefe V1–V3/C1–C3, keine frei geführte Tonhöhe.

**a (Empfehlung):** „Bei der Tonradorgel wirkt die Spur Expression als Schweller (Pegel und Frequenzgang auf der Summe,
vor dem Leslie). Vibrato und Pitch hat sie nicht; der Scanner ist eine Spielweise über eine Passage (Aus, V1, V2, V3,
C1, C2, C3). SF10 a erhält dazu den Satz: ‚Schwinger ohne Anregung haben Expression, wenn sie einen Schweller haben
(Tonrad, Pfeifenorgel).‘“

**b:** „Die Spur Vibrato steuert die Tiefe des Scanners stufenlos; Expression wie a.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Spuren das abbilden sollen, was der Spieler bewegt: den Schweller
stufenlos, den Scanner in Stufen per Schalter. Dieselbe Lösung schlägt Klasse 9 F-3 für die Pfeifenorgel vor
(Schwellkasten, Tremulant). Für b spricht die feinere Steuerung — das wiegt weniger, weil der Scanner nur sechs Stellungen
kennt und eine stufenlose Tiefe ihn zu einem gewöhnlichen Vibrato machen würde. **Kippt auf b**, wenn Hörproben zeigen,
dass Nutzer die Tiefe zwischen den Stellungen brauchen.

---

## F-4 — Registrierung und Perkussion: Ausstattung oder Spur

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T3b): a.**

**Ort:** `Spielweisen_Konzept.md` Zielbild („Orgel: Register, Leslie langsam/schnell“), **SF8 a** (Register über eine
Passage). **Heutiger Wortlaut:** Register als Zustand über die Zeit.

**Problem:** Die Klangfarbe der Tonradorgel ist die Zugriegel-Registrierung (neun Zugriegel, je 0–8); Organisten ziehen
Zugriegel auch während gehaltener Töne. Dazu kommen Perkussion (an/aus, 2./3. Harmonische, fast/slow, soft/normal) und
die Presets.

**a (Empfehlung):** „Die Registrierung (neun Zahlen 0–8) und die Perkussion sind Ausstattung des Instruments (Vorgabe);
eine Spur ‚Register‘ über die Passage schaltet zu einer anderen Registrierung um, und das Modell rechnet den Übergang als
Bewegung der Zugriegel über eine kurze Zeit. Perkussion und Scanner sind Schalter derselben Spur.“

**b:** „Jede Registrierung ist ein eigenes Instrument; gewechselt wird durch einen anderen Clip.“

**Warum die Empfehlung:** Den Ausschlag gibt SF8 a: was über eine Passage wechselt, gehört auf eine Spur mit
Umschaltstellen; ein Registerwechsel mitten in einer Phrase ist Spielpraxis. Für b spricht, dass nichts Neues gebaut
werden muss — das wiegt weniger, weil ein Clipwechsel den Leslie-Zustand und die Perkussion abschneiden würde (F-2).
**Kippt auf b**, wenn F-1 b gewählt wird (dann gibt es keinen Rechenweg, der den Übergang rechnen kann).

---

## F-5 — Was `organ` und `house_organ` werden

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T8): a.**

**Ort:** `Spielweisen_Konzept.md` SF10 a: „Welche Orgel `organ` und `house_organ` sind, klärt die Recherche ihrer
Familie.“ **Heutiger Wortlaut:** `organ` „A sine with five overtones that holds at full level while the key is down.“;
`house_organ` „A sine with four overtones and a quick attack, for stabbed chords.“

**Problem:** `organ` ist eine Zugriegel-Orgel ohne Generator, ohne Klick, Perkussion, Scanner und Leslie — das Vorbild
ist erkennbar die Tonradorgel. `house_organ` ist ein kurzer, abklingender Orgelklang für Akkordstöße; ein Vorbild nennt die
Datei nicht (nach Einschätzung ein Orgelklang aus der House-Musik der späten 1980er, ungeprüft).

**a (Empfehlung):** „`organ` wird die gerechnete Tonradorgel (eine Standard-Registrierung, Scanner aus, Leslie langsam).
`house_organ` wird dieselbe Tonradorgel mit kurzer Registrierung und Perkussion, als Näherung des abklingenden Stoßes; ob
das dem Vorbild nahekommt, entscheidet eine Hörprobe. Die heutigen Sinus-Fassungen bleiben als Synth-Schichten unter
anderem Namen erhalten.“

**b:** „`organ` wird die Tonradorgel, `house_organ` bleibt der heutige Synth-Klang.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass beide Namen nach SF10 a eine Orgel meinen und die Tonradorgel mit
Perkussion den abklingenden Stoß physikalisch hervorbringt. Für b spricht, dass `house_organ` vielleicht gar keine
Tonradorgel nachbildet — das wiegt weniger, weil a das ausdrücklich an eine Hörprobe bindet. **Kippt auf b**, wenn die
Hörprobe den Stoß mit Perkussion nicht trifft.

---

## F-6 — Bezugsaufnahmen und Downloads

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): a.**

📁 **Ablage (Anweisung Patrik 2026-10-26, IC Session 2, A-1 a, für alle Klassen gleich):** Aufsätze kommen ins Repo nach `quellen/`; **Aufnahmen und Messdaten lokal** nach `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für LumiViz erreichbar), im Repo nur `aufnahmen/<Datensatz>/` bzw. `daten/<Datensatz>/` mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`; jeder Datensatz in [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). „Neben LumiViz“ im Text unten ist damit überholt; welche Dateien geladen werden, entscheidet diese Frage.

**Ort:** [aufnahmen.md](aufnahmen.md). **Heutiger Wortlaut:** neu.

**Problem:** Frei sind nur Leslie-Hochläufe (CC0, gemeinfrei, nachgelesen), ein gepaarter Leslie-145-Datensatz (CC BY 4.0,
968 MB, nachgelesen) und M100-Dauerklänge (CC BY 4.0, nachgelesen); für den Generator selbst (einzelne Zugriegel direkt)
gibt es nichts. Die beste Quelle (HamOrg) hat keine Lizenz.

**a (Empfehlung):** „Der Generator wird an Konstruktionswerten (Übersetzungen, Handbuch als Fakten) und an den
M100-Dauerklängen gesetzt; die Klangfarbe je Zugriegel bleibt offen, bis eine eigene Aufnahme vorliegt (Forschungsauftrag)
oder der Autor von HamOrg eine Lizenz gibt. Der Leslie wird an den CC0-Hochläufen und am Leslie-145-Datensatz gesetzt.
Vor dem Laden legt Claude Dateien, Größen und Zielordner neben LumiViz zur Freigabe vor; die freien Aufsätze (Pekonen et
al. 2011, Werner et al. 2016, Moro et al. 2017) nach Freigabe unter `quellen/`.“

**b:** „Den Generator vorerst nur nach den Konstruktionswerten bauen, ohne Abgleich an Aufnahmen.“

**Warum die Empfehlung:** Den Ausschlag gibt die Lehre aus dem Flügel: jede Größe braucht eine Messgröße — a nimmt, was
frei ist, und benennt die Lücke, statt sie mit Annahmen zu füllen. Für b spricht, dass die Konstruktion gut dokumentiert
ist — das wiegt weniger, weil Wellenform, Filter und Pegel je Rad im Handbuch nur als Sollwerte mit „30 percent“
Toleranz stehen (nur Bericht). **Kippt auf b**, wenn keine eigene Aufnahme möglich ist.

---

## F-7 — Markennamen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): a.**

**Ort:** Namen und Beschreibungen mitgelieferter Instrumente in LumiViz; [quellen.md](quellen.md). **Heutiger
Wortlaut:** `organ` „Organ“; SF10 a nennt „Hammond“ und „Leslie“.

**Problem:** „Hammond“ und „Leslie“ sind Markennamen (laut Händlerangabe „trademarked by Hammond Suzuki“, nur Bericht).
In der Recherche sind sie als Gattungsbezeichnung der Quellen nötig; in einem ausgelieferten Programm wären sie ein
Rechtsrisiko.

**a (Empfehlung):** „In LumiViz heißen das Instrument ‚Tonewheel Organ‘ und die Ausstattung ‚Rotary speaker‘; die
Markennamen stehen nur in der Doku als Herkunft der Werte.“

**b:** „Die Markennamen werden verwendet, wie es Synthesizer-Hersteller oft tun.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass neutrale Namen nichts vom Klang nehmen und kein Risiko tragen. Für b
spricht die Bekanntheit der Namen — das wiegt weniger, weil die Rechtslage nicht geprüft ist. **Kippt auf b**, wenn eine
rechtliche Prüfung die Verwendung freigibt.

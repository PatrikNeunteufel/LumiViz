# Offene Fragen — Rohrblatt und Rohr (Klasse 8)

> **Gebündelt 2026-10-29 (IC Session 3):** die offenen Fragen dieser Datei sind in [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md)
> nach Themen zusammengefasst — dort beantworten; die Antwort wird hierher übertragen.
>
> **Stand:** 2026-10-26, IC Session 1. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz: `…\LumiViz\projects\apps\LumiViz\docs\audio\Spielweisen_Konzept.md`, nachgelesen 2026-10-26.
> Fragen für alle Bläser in [gemeinsam/offene-fragen.md](../../gemeinsam/ic/offene-fragen.md) (F-R1 bis F-R5).

---

## F-1 — Umfang und Reihenfolge

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T8): a („nach Empfehlung“)** — Kern und Reihenfolge wie a; **nichts wird zurückgestellt** (Z-1 a, Patrik 2026-10-29, [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md) §4): was die Klassenfrage zurückstellen wollte, wird aus Literatur und Modell gebaut und als ‚ungemessen‘ gekennzeichnet (T8 a).

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 8. **Heutiger Wortlaut:** „Klarinette, Saxophon, Oboe, Fagott“.

**Problem:** Die Klarinette hat Rohrblattwerte (Chatziioannou 2019), eine Tonlochtabelle (Debut et al., zu prüfen),
Einschwing-Messungen (Li 2016) und Druckdaten (MIPCAT); das Saxophon hat Impedanzen je Griff (Maestre 2018), aber keine
freie Geometrie des Alt; Oboe und Fagott haben weder Geometrie noch dynamische Rohrblattwerte frei.

**a (Empfehlung):** „Zuerst die Klarinette (Kern: Einzelblatt, zylindrisches Rohr, Tonlöcher, Legato), dann das
Altsaxophon (Kegel, Mundstück; Geometrie aus Impedanzen zurückgerechnet oder gemessen), dann Oboe und Fagott, sobald
eine Geometrie vorliegt.“

**b:** „Zuerst das Saxophon — es ist in LumiViz heute schon als Instrument da (`sax`, `alto_sax`).“

**Warum die Empfehlung:** Den Ausschlag gibt, dass der Kern an dem Instrument mit vollständigen Daten geprüft wird —
an der Klarinette sind Rohrblatt, Rohr und Einschwingen belegt, am Saxophon fehlt die Bohrung. Für b spricht, dass das
Saxophon in LumiViz vorhanden ist und ersetzt würde — das wiegt weniger, weil das Saxophon nach dem geprüften Kern mit
weniger Raten folgt. **Kippt auf b**, wenn eine freie Altsax-Geometrie auftaucht. *Nach der Regel „Pausieren nur mit
Freigabe“:* a stellt Oboe und Fagott bis zu einer Geometrie zurück — hiermit vorgelegt.

---

## F-2 — Bend: Pitch-Spur und Vokaltrakt

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T3a): a.**

**Ort:** `Spielweisen_Konzept.md` SF6 a. **Heutiger Wortlaut:** Rohrblatt und Rohr — „Pitch … ja (Bend, Glissando)“.

**Problem:** Ein Bend an Klarinette und Saxophon entsteht über den Vokaltrakt und die Lippe, nicht über das Rohr;
abwärts bis etwa eine große Terz („400 cents or a major third“ bei C6), aufwärts kaum, und erst ab etwa D5 groß (Chen et
al. 2009, nur Bericht). Eine Pitch-Spur, die beliebig in beide Richtungen biegt, kann das Modell nicht treu umsetzen.

**a (Empfehlung):** „Die Pitch-Spur bleibt das Ziel; das Modell setzt sie über Vokaltrakt und Lippe (F-R5) um, so weit
die Physik reicht, und zeigt im Editor die Grenzen des Instruments (wie *Pitch range* an Synth-Schichten).“

**b:** „Die Pitch-Spur verstimmt das Rohr direkt (wie ein Posaunenzug), ohne Grenzen.“

**Warum die Empfehlung:** Den Ausschlag gibt die Treue — ein Bend klingt nach Klarinette, weil er an den Grenzen des
Instruments entsteht; a nutzt den Vokaltrakt, der ohnehin für das Saxophon-Altissimo gebraucht wird. Für b spricht die
volle Freiheit der Spur — das wiegt weniger, weil ein verstimmtes Rohr einen anderen, unnatürlichen Klang gäbe. **Kippt
auf b**, wenn F-R5 (Vokaltrakt) verworfen wird.

---

## F-3 — Atemgeräusch, Flatterzunge, Growl ohne physikalisches Modell

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T3c): a.**

**Ort:** Zielbild §1 (Holzbläser); SF8 a. **Heutiger Wortlaut:** „Zungenstoß einfach/doppelt, Atem- und Luftgeräusch,
Flatterzunge“.

**Problem:** Für Atem- und Luftgeräusch am Rohrblatt gibt es kein Messmodell („Possibly the most important neglected
phenomenon … turbulence of the flow“, Smith/PASP, nur Bericht); Flatterzunge und Growl sind nicht gemessen. Zungenstoß
ist belegt (Li 2016, Chatziioannou 2019).

**a (Empfehlung):** „Zungenstoß physikalisch (Zunge am Rohrblatt). Atemgeräusch als Rauschen, moduliert vom
Rohrblattspalt, gesetzt am Restsignal der Aufnahmen (Messkette B4) und als nicht physikalischer Teil gekennzeichnet.
Flatterzunge als periodische Unterbrechung durch die Zunge (dieselbe Zunge, Rate als Wert der Art); Growl wie am Blech
als zweites Ventil (Klasse 7, F-5).“

**b:** „Nur der Zungenstoß; Atemgeräusch, Flatterzunge und Growl entfallen, bis es Modelle gibt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass alle vier im Zielbild stehen und a für jedes den Mechanismus nimmt,
der schon da ist (Zunge, zweites Ventil) oder an Messung gesetzt werden kann (Rauschen) — mit klarer Kennzeichnung, was
nicht physikalisch ist. Für b spricht, dass nichts Ungemessenes entsteht — das wiegt weniger, weil das Atemgeräusch am
Restsignal messbar ist und Flatterzunge/Growl physikalisch naheliegen. **Kippt auf b** für die Flatterzunge, wenn sie
mit der Zunge allein nicht nach Flatterzunge klingt.

---

## F-4 — Bezugsaufnahmen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): a.**

📁 **Ablage (Anweisung Patrik 2026-10-26, IC Session 2, A-1 a, für alle Klassen gleich):** Aufsätze kommen ins Repo nach `quellen/`; **Aufnahmen und Messdaten lokal** nach `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für LumiViz erreichbar), im Repo nur `aufnahmen/<Datensatz>/` bzw. `daten/<Datensatz>/` mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`; jeder Datensatz in [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). „Neben LumiViz“ im Text unten ist damit überholt; welche Dateien geladen werden, entscheidet diese Frage.

**Ort:** [aufnahmen.md](aufnahmen.md). **Heutiger Wortlaut:** neu.

**Problem:** Iowa MIS hat Klarinette, Altsax, Oboe, Fagott (pp/mf/ff, reflexionsarm, nachgelesen); kein Tenorsax. Für
das Tenorsax gibt es Good-Sounds mit zwei Lizenzangaben (Zenodo CC BY 4.0, Freesound CC BY 3.0, nur Bericht) und VCSL
(CC0, ohne Stufen).

**a (Empfehlung):** „Iowa MIS ist der Bezug; TinySOL und Good-Sounds zum Gegenprüfen nach Prüfung der Lizenz am Ort
(bei Good-Sounds die Zenodo-Fassung, CC BY 4.0, wenn bestätigt); vor dem Laden legt Claude Dateien, Größen und
Zielordner zur Freigabe vor.“

**b:** „Good-Sounds ist der Bezug (Einzeltöne, mehrere Spieler, Fehltöne).“

**Warum die Empfehlung:** Den Ausschlag gibt der dokumentierte reflexionsarme Raum bei Iowa. Für b sprechen Einzeltöne
und mehrere Spieler — das wiegt weniger, weil die Lizenz widersprüchlich angegeben ist und Raum und Stufen weniger
dokumentiert sind (nur Bericht). **Kippt auf b** für das Tenorsax, weil Iowa es nicht hat.

# Auftrag: Vorab-Recherche je Instrumentklasse

> **Für:** eine Claude-Sitzung in diesem Ordner (`…\Visuals_Project\cmake\InstrumentsCalculation`), unabhängig von
> LumiViz. **Erteilt:** Patrik, 2026-10-26 (formuliert in LumiViz S154). **Ziel:** je Klasse alles vorbereiten, damit
> sie später in wenigen Sitzungen nach dem Muster des Flügels ausgearbeitet werden kann. **Kein Code, keine
> Schnittstelle, keine Änderung an LumiViz.**

## Zuerst lesen

1. `README.md`, `KLASSEN.md`, `SCHNITTSTELLE.md`, `VORBILD_FLUEGEL.md` in diesem Ordner.
2. In LumiViz (nur lesen): `…\cmake\LumiViz\projects\apps\LumiViz\docs\modellierung\README.md` und
   `…\modellierung\hammer-saite\README.md`, `modell.md`, `messplan.md`, dazu zwei Berichte unter `recherche/` als
   Vorbild für Form und Tiefe. Die Entscheide stehen in `…\docs\audio\Spielweisen_Konzept.md` §4 (lang — gezielt
   suchen, etwa nach „SF10 a“, „KB1 a“, „IK1“, „UF16 a“).
3. Für die gemeinsame Grundlage: `…\modellierung\koerper\README.md` und die Berichte dort.
4. Vor jeder Klasse den **Bestand in LumiViz** (nächster Abschnitt) für diese Klasse, dazu die **Zusatzfragen aus
   LumiViz** (Abschnitt danach) — auch für schon abgeschlossene Klassen.

## Bestand in LumiViz (Rahmen mit Prüfauftrag)

> Entscheid Patrik IC-B1 a (2026-10-26, LumiViz S154): LumiViz hat für mehrere Klassen schon Verfahren, Spielweisen
> und Steuerungen entschieden, gebaut und teils an Aufnahmen gemessen. **Diesen Bestand lesen, nicht neu
> recherchieren.** Wo die Recherche ihm widerspricht oder ihn mit einer Quelle verbessern kann, wird das eine Frage in
> `offene-fragen.md` — mit Ort und Wortlaut der LumiViz-Stelle (Datei, Abschnitt, Entscheid) und dem Beleg. Nichts
> still übernehmen, nichts still ersetzen. Alle Pfade unter `…\cmake\LumiViz\projects\apps\LumiViz\docs\`.

**Für alle Klassen:**
- `audio/Spielweisen_Konzept.md` §1 und §1a (Zielbild S139, Skizze S140): die **Bauart** als Paar aus Anregung und
  Schwinger (SF10 a), Ausstattung als Obergrenze (SF6 a), die **Orte der Klang-Spielweisen** nach Häufigkeit des Wechsels
  (SF8 a: nie → Ausstattung, über eine Passage → Spur mit Umschaltstellen, je Note → am Anschlag), die Reihenfolge der
  Familien (SF9 a).
- `audio/Spielweisen_Konzept.md` §3 (Stufe 28): **fünf Spuren je Muster** — Sustain, Sostenuto (Stufen 0 … 1),
  Expression, Vibrato, Pitch (Kurven) — und Expression am Clip (SF5 d); Expression wirkt als Pegel e².
- `audio/Spielweisen_Konzept.md` §4.2–§4.5: Töne werden **vorab** gerechnet (Tonspeicher, Freeze, Klangfassung) —
  ein Modell, das je Note mit ihrer Spur rechnet, muss dort hineinpassen.
- Pegel: jedes mitgelieferte Instrument liegt auf dem Bezug −13,4 LUFS ±1 LU (Stufe 27).

**1 Hammer-Saite (Rest):** der Flügel als Vorbild (`Spielweisen_Konzept.md` §4, `modellierung/hammer-saite/`);
Tastatur mit Pedalen und `undampedFrom` (HA4 a), Ausstattung mit Moderator/Übepedal für das Upright (siehe SF6 a und
die Vorlagen *Grand*/*Upright*); geplant in LumiViz: Mitschwingen (31e), halbes Pedal, Sostenuto, Una corda (31f),
das Upright als Stufe 32 (Name `upright_piano`).

**2 Gezupfte Saite:** `audio/Saiten_Konzept.md` und `modellierung/gezupfte-saite/` (Recherchen S132–S138) —
- Verfahren der Saite *Classic*, *Extended KS*, *Waveguide*; Saitensatz, Tuning, Saitenwahl beim Planen; Tonabnehmer
  als Abtaststellen; Korpus *Parlor* und *Classical guitar* (§5a–§5l);
- **Spielweisen** (§5m–§5q, Stufe 24): Strum mit **Richtung** (Down/Up) und Anschlägen, von sich aus an, Artikulationen
  (Palm mute, Tapping, Harmonic …) mit an IDMT-SMT-Guitar/-Bass gemessenen Vorgaben (Recherche 24c);
- **verbundene Noten** (§5r–§5t, Stufe 25): Legato (Hammer-on/Pull-off), Slide, Bend mit Kurve, Ketten;
- **Akkorde und Tabs** (§5u–§5zd, Stufe 26): `.lvchord` (Griffbild oder Voicing), die Arten in
  `asset/loop/chords/types.json`, 6373 mitgelieferte Akkorde, Tab-Linien im Editor, Voicings, *Play as*;
- Vibrato und Pitch an der Saite, Vibratohebel (Stufe 28); Saitenabstand und Spread (§6c).
- Offen in LumiViz: der Gitarrenkorpus auf der gemeinsamen Grundlage (KB1 a, `modellierung/koerper/`).

**3 Bogen-Saite:** Saitensatz und Korpus aus Klasse 2 sind da (SF9 a: „neu die Anregung durch den Bogen“); die Spuren
Expression und Vibrato (Stufe 28) sind die heutige Steuerung — Bogendruck und -geschwindigkeit wären neu und gehören
nach SF8 a an einen Ort (Frage, nicht still festlegen).

**4 Platte und Stab, 7–9 Rohr, 10 Zunge, 11 Tonrad:** kein eigener Bestand außer den allgemeinen Regeln oben; für 11
nennt SF8 a „Leslie langsam/schnell“ ausdrücklich als Spielweise über eine Passage.

**5 Membran, 6 Becken:** Drums in LumiViz sind Spuren im Drum-Editor mit **Kits und Rollen** (`.lvkit`,
`audio/Loop_Konzept.md` §4a, §4d, §4l); ein gerechnetes Schlagzeug muss in diese Rollen passen (Frage, wenn nicht).

## Zusatzfragen aus LumiViz

> Fragen, die in LumiViz aufgekommen sind und hier recherchiert werden sollen. **Gilt auch für schon abgeschlossene
> Klassen:** dort als eigener Bericht unter `recherche/` nachtragen und `modell-entwurf.md` / `offene-fragen.md`
> ergänzen; in `STAND.md` vermerken.

**Z1 — Kombination von Moden und Waveguides** (Frage Patrik, LumiViz S154, 2026-10-26). Für die Klassen mit Saite (1, 2,
3) und mit Rohr (7, 8, 9), wo sinnvoll auch für die übrigen: Gibt es in der Literatur Verfahren, die Moden und Waveguides
(digitale Wellenleiter) verbinden, und was bringen sie? Zu klären je Klasse:
- welche Verfahren es gibt (etwa Moden für die tiefen Teiltöne und ein Waveguide darüber; ein Waveguide für die Saite und
  Moden für Körper bzw. Steg; Kopplung mehrerer Saiten über eine Steg-Verbindung) — mit Quelle, Belegkraft gekennzeichnet;
- was je Weg geht und was nicht: Steuerung je Teilton (Verstimmung, zweistufiges Abklingen, zwei Schwingungsebenen),
  Teiltöne ohne Obergrenze, frequenzabhängiger Verlust, Nichtlineares (Hammerkontakt, Bogen, Lippe, Rohrblatt),
  Zeitveränderliches (Bend, Vibrato), Rechenzeit, Rechnung vorab (Tonspeicher) gegen Echtzeit;
- **und ob ein Verfahren an Aufnahmen gesetzt werden kann:** LumiViz setzt die Größen der Hammer-Saite durch das Modell
  hindurch und braucht dafür eine schnelle Vorhersage der Messgrößen je Teilton aus den Moden (Entscheid SZ3 a,
  `Spielweisen_Konzept.md` §4.1) — ein Waveguide hat diese Vorhersage so nicht.

Stand in LumiViz (nur lesen): die Saite der Gitarre gibt es als *Waveguide* (`Saiten_Konzept.md` §5d), die Hammer-Saite
nur modal (HA2 a); die Antwort von Claude in S154 steht im Konzept unter „Frage Patrik (S154): eine Kombination von
Moden und Waveguides?“ — dort ungeprüft, ohne Quellen. Ergebnis je Klasse als Bericht
`recherche/JJJJ-MM-TT_moden_und_waveguides.md`, Folgen als Frage im Format oben.

> Z2 und Z3: Vorschläge aus LumiViz S157 (`LEHREN_SETZEN_FLUEGEL.md` §3), aufgenommen nach Entscheid Patrik
> **F-L1 a, F-L2 a** (2026-10-28, IC Session 3; Z3 um die Länge der Töne erweitert). Lehren dazu:
> [VORBILD_FLUEGEL.md](VORBILD_FLUEGEL.md) §Lehren aus dem Setzen.

**Z2 — Kurven als Suchraum** (Vorschlag LumiViz S157, 2026-10-27). Für jede Klasse, auch die abgeschlossenen: Welche
Größen sind Kurven über den Tonumfang, welche fest, welche hängen an der Stärke; wie viele Stützpunkte verlangt die
Physik je Kurve (Wechsel von Steg, Saite, Register, Bohrung, Fell) und wo liegen sie; reicht die Zahl der Noten im
Bezugsdatensatz, um diese Stützpunkte zu bestimmen (Messgrößen gegen Parameter)? Wo eine Größe je Note gesucht werden
soll, steht der physikalische Grund dabei. Ergebnis als Tabelle in `modell-entwurf.md` (Größe | Art | Stützpunkte:
Zahl, Lage, Grund | Noten im Bezug | Messgrößen je Parameter), das Setzen in `messplan.md` als Ausgleich über alle Noten
zugleich; Lücken als Frage.

**Z3 — Toleranz als Auflösung des Bezugs** (Vorschlag LumiViz S157, 2026-10-27, erweitert). Für jeden Datensatz in
`aufnahmen.md`: Hat er Nachbarstufen, mehrere Kanäle oder Wiederholungen derselben Note, aus denen sich je Messgröße
eine Streuung messen lässt? Wenn nein: welche zweite Quelle gäbe sie? Und wie lang sind seine Töne (kürzeste und
typische Dauer, an den Dateien gemessen)? Ergebnis als Spalten „Toleranz messbar aus“ und „Länge der Töne“ in
`aufnahmen.md`, in `messplan.md` je Messgröße die Toleranz als Rezept (welche Streuung, an welchen Noten) und der
gültige Bereich als Grenze aus der Länge (Abklingzeiten bis etwa zum Dreifachen der Länge). Grundlage:
[gemeinsam/messkette.md](../../messung/ic/messkette.md) §4a.

## Reihenfolge

Nach `KLASSEN.md`, Abschnitt „Empfohlene Reihenfolge“: **0 Gemeinsam → 4 Platte und Stab → 1 Hammer-Saite (Rest) →
7/8/9 Rohr → 3 Bogen-Saite → 5/6 Membran, Becken → 11 Tonrad → 10 Zunge → 2 Gezupfte Saite**. Eine Klasse ganz
abschließen (alle Lieferungen unten), bevor die nächste beginnt. Will Patrik eine andere Reihenfolge, gilt seine.

## Lieferungen je Klasse (unter `klassen/<klasse>/`)

1. **`recherche/JJJJ-MM-TT_<thema>.md`** — je Thema ein Bericht. Themen mindestens: Physik und Rechenverfahren
   (welche Modelle es gibt, ihre Kosten, was davon modal vorab rechenbar ist); Bauteile und Maße (Daten, die ein Modell
   braucht, mit Herkunft); Spielweisen und Steuerung; Körper/Abstrahlung, falls die Klasse einen braucht.
   **Kopf jedes Berichts:** Auftrag (eine Zeile), Datum, Reichweite der Suche (welche Datenbanken, Suchbegriffe,
   Sprachen; was nicht erreichbar war), Kennzeichnung je Angabe „nachgelesen“ (im Volltext selbst gelesen, mit Seite)
   oder „nur Bericht“ (aus Zusammenfassung, Abstract, Drittseite), „eingeflossen in“ (welche Datei hier).
2. **`quellen.md`** — Liste der Literatur: Autoren, Jahr, Titel, Ort **so, wie sie in der Quelle selbst stehen**;
   Fundort (DOI, HAL, arXiv, Verlag); frei oder nicht; Nicht frei Erreichbares ausdrücklich nennen (wie Weinreich 1977
   beim Flügel). **Rechtslage je Eintrag, auch bei nicht freien:** Lizenz oder Rechtsvermerk im Wortlaut (etwa
   „CC BY 4.0“ oder „© … All rights reserved“), wo er steht (Seite der Quelle, Feld der Ablage), Datum des Abrufs; ob
   die Datei unter `quellen/` liegt (Dateiname); **Übernahme nach LumiViz:** was mitgehen darf (Datei, Auszüge mit
   Lizenzhinweis, nur Verweis) — im Zweifel „offen, vor Übernahme klären“.
3. **`aufnahmen.md`** — freie Aufnahmen und Messdaten der Instrumente der Klasse: Quelle, **Lizenz an der Quelle
   geprüft** (Wortlaut des Feldes zitieren, Datum des Abrufs), Inhalt (Töne, Stufen, Mikrofone, Format), Größe,
   Eignung für die Messung (Einzeltöne? mehrere Stärken? Raum?). Behauptungen von Drittseiten als solche markieren.
   Je Datensatz „Toleranz messbar aus“ und „Länge der Töne“ (Z3).
4. **`modell-entwurf.md`** — was gerechnet werden könnte: Anregung, Schwinger, Kopplung, Körper; Gleichungen mit
   Quelle; **jede Größe mit Herkunft** (Quelle, Messung an einer Aufnahme, oder offen); Abschätzung der Rechenzeit je
   Ton; ob vorab rechenbar oder je Note mit ihrer Spur; was davon „Deluxe“ wäre. Am Rahmen aus `SCHNITTSTELLE.md`
   ausrichten, Abweichungen begründen. **Ergänzt (F-L3 a, 2026-10-28):** die Tabelle aus Z2 (Art und Stützpunkte je
   Größe); wo das Modell seine Struktur an Schwellen wechselt (Register, Anblasen, Anstreichen gegen Rutschen,
   Kopplung, zweite Stufe), als Liste mit Entscheid „gesucht“ oder „gesetzt“ (aus Quelle oder nach Gehör) — dort wird
   das Modell an der Struktur des Bezugs gemessen, nicht mit eigener Erkennung (`gemeinsam/messkette.md` §3a);
   Rechenzeit **je Auswertung beim Setzen** neben der je Ton beim Spielen.
5. **`messplan.md`** — welche Größen aus den Aufnahmen messbar sind, wie (an welche Teile der Messkette aus
   `SCHNITTSTELLE.md` anschließend, was neu gebraucht wird), gültiger Bereich je Messgröße, vorgeschlagene Toleranz;
   **je Modellgröße die Messgröße, die sie verursacht** (Lehre aus dem Flügel). **Ergänzt (F-L3 a, 2026-10-28):**
   Toleranz als Rezept aus der Streuung des Bezugs, gültiger Bereich als Grenze dessen, was die Aufnahme bestimmt (Z3);
   das Setzen als Ausgleich über die Stützpunkte aller Noten zugleich (Z2) und als Feinabstimmung **nach** dem
   spielbaren, fertigen Instrument (RF1 a); Hörproben als eigene Zeile (welche Noten, Stärken, gegen welchen Bezug,
   Frage je Datei); Zahl der Auswertungen je Suche und Dauer eines Setz-Laufs, mit Ergebnisdatei je Note bzw.
   Iteration.
6. **`offene-fragen.md`** — die Entscheidungsfragen an Patrik, die vor dem Bau zu klären sind (Zielbild der Klasse wie
   die fünf Fragen beim Flügel; Umfang; Aufnahmen). **Format je Frage, verbindlich:** Ort, heutiger Wortlaut (oder
   „neu“), Problem in Klartext, jede Option als fertiger Text, die empfohlene vorn als **a (Empfehlung)**, danach ein
   eigener Absatz *Warum die Empfehlung* (was den Ausschlag gibt, was für die andere spricht und warum es hier weniger
   wiegt, woran die Empfehlung kippen würde). Empfehlungen nach „sauber/richtig“, nicht nach Aufwand.
7. **`README.md`** der Klasse nachführen: Stand, Inhaltsverzeichnis, und eine Zeile in `KLASSEN.md` (Spalte Aufwand
   und Qualität begründet nachschärfen, wenn die Recherche es hergibt — die alte Schätzung daneben stehen lassen).

Für **0 Gemeinsam** statt 4–6: `gemeinsam/koerper.md` (welche Körper welche Klassen brauchen, offene Daten,
Übertragbarkeit der Platte des Flügels auf Gitarre, Violine, Membran), `gemeinsam/rohr.md` (ein Rohrmodell für 7, 8, 9:
Verfahren, Kosten), `gemeinsam/messkette.md` (was die Teiltonmessung des Flügels für andere Klassen nicht kann:
Rauschanteile, Einschwingen, Vibrato, gehaltene Töne).

## Regeln

- **Sprache:** Deutsch, auch in Zwischensätzen; Fachbegriffe beim ersten Mal mit dem englischen in Klammern.
- **Belege:** Eine Zusammenfassung eines Abrufwerkzeugs ist **kein Beleg** — zitierfähig ist nur, was als Wortlaut
  oder Feldinhalt zitiert ist. Je Angabe notieren, woraus sie stammt; die Reichweite der Suche nennen. Kein Treffer ist
  kein Beweis; ein glatter Treffer ist noch keine Quelle.
- **PDFs durchsuchen** nicht mit `grep`/`rg` (liefert still nichts), sondern `~/.claude/tools/quelle_suchen.py <pdf>
  <begriff>` (PDF-Seite und vermutete gedruckte Seite); in einer Cloud-Sitzung `gemeinsam/cloud-global/tools/` (G-1 a).
- **Downloads nur mit Freigabe:** vor jedem Download Patrik fragen (Dateiname, Quelle, Größe). Ohne Freigabe den
  Kandidaten in `quellen.md` bzw. `aufnahmen.md` aufnehmen und weiterarbeiten. Freigegebene PDFs nach
  `klassen/<klasse>/quellen/` (Dateiname `Autoren_Jahr_Titel_Ort.pdf`); sie werden **versioniert** (Repo privat, siehe
  `README.md` „Ablage und Lizenzen“), die Rechtslage steht in `quellen.md`. **Aufnahmen und Messdaten** kommen nach
  Freigabe **nicht** ins Repo, sondern lokal nach `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für LumiViz
  erreichbar). Im Repo bleiben je Datensatz unter `klassen/<Klasse n - Name>/aufnahmen/<Datensatz>/` bzw. `…/daten/…` nur
  `HERKUNFT.md` (Quelle, Lizenz, Datum, Ablage), `SHA256SUMS` und `LADELISTE.tsv` (Prüfsumme, Pfad, Quelle je Datei);
  jeder Datensatz steht in [UEBERNAHME_LUMIVIZ.md](UEBERNAHME_LUMIVIZ.md). Aufsätze (PDF, klein) bleiben unter
  `quellen/` im Repo. (Anweisung Patrik 2026-10-26, IC Session 2, A-1 a: Git ist für Text; jede Cloud-Sitzung klonte
  sonst Gigabytes, die sie kaum braucht. Ersetzt die Zwischenregel „Aufnahmen ins Repo“ vom selben Tag.)
- **Git:** keine Befehle, die etwas verändern (`init`, `add`, `commit` …) — das macht Patrik. Kein `Co-Authored-By`.
- **Nichts in LumiViz ändern**, keine Dateien dort anlegen; LumiViz nur lesen.
- **Kein Code.** Kleine Rechnungen zur Abschätzung (etwa Rechenzeit, Moden einer Platte) als Python-Skript im
  Scratchpad der Sitzung, Ergebnis mit Skript-Text im Bericht.
- **Agenten:** Für breite Suchen dürfen Unteragenten laufen; ihre Berichte werden **unverändert** unter `recherche/`
  abgelegt (mit Kopf, Kennzeichnung „Bericht eines Agenten“), und was davon zählt, liest die Sitzung selbst nach.
- **Zwischenstand:** Am Ende jeder Sitzung `STAND.md` im Wurzelordner nachführen (welche Klasse, was fertig, was offen,
  welche Freigaben ausstehen, welche Fragen an Patrik warten).

## Fertig ist eine Klasse, wenn

- alle sieben Lieferungen da sind, jede Angabe gekennzeichnet ist,
- mindestens eine freie Aufnahme mit geprüfter Lizenz gefunden **oder** die Suche mit Reichweite als erfolglos belegt ist,
- `modell-entwurf.md` je Größe eine Herkunft oder „offen“ trägt und `messplan.md` je Modellgröße eine Messgröße nennt,
- `offene-fragen.md` die Fragen im verbindlichen Format enthält,
- die Punkte aus Z2 und Z3 und die Ergänzungen der Lieferungen 3–5 (F-L3 a) da sind.

## Nicht Teil des Auftrags

- Schnitt zwischen LumiViz und diesem Projekt, Repo, Build, Schnittstellen festlegen (Entscheid nach IK1, später).
- Editor/Creator, Oberfläche, mitgelieferte Instrumente.
- Der Flügel selbst (wird in LumiViz fertig).

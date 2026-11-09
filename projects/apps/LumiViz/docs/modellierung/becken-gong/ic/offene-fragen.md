# Offene Fragen — Becken und Gong (Klasse 6)

> **Gebündelt 2026-10-29 (IC Session 3):** die offenen Fragen dieser Datei sind in [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md)
> nach Themen zusammengefasst — dort beantworten; die Antwort wird hierher übertragen.
>
> **Stand:** 2026-10-26, IC Session 1. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz wie Klasse 5. **Mit Klasse 5 geteilt** (dort ausgeführt, gilt hier ebenso):
> [membran F-2](../../membran/ic/offene-fragen.md) Anschlagstärke gestuft im Namen (für Becken feinere Stufen),
> [F-3](../../membran/ic/offene-fragen.md) Spielweisen am Schritt (Kuppe/Fläche/Rand), [F-4](../../membran/ic/offene-fragen.md)
> Wirbel als Ton über die Dauer (Beckenwirbel, Tamtam-Swell).

---

## F-1 — Umfang, Reihenfolge und „Deluxe“

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T8): a („nach Empfehlung“)** — Kern und Reihenfolge wie a; **nichts wird zurückgestellt** (Z-1 a, Patrik 2026-10-29, [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md) §4): was die Klassenfrage zurückstellen wollte, wird aus Literatur und Modell gebaut und als ‚ungemessen‘ gekennzeichnet (T8 a).

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 6. **Heutiger Wortlaut:** „Crash, Ride, Hi-Hat, Gong, Tamtam“; Risiko
„hoch (Rechenzeit, Chaos; eher Deluxe)“.

**Problem:** Die Rechenzeit ist kein Hindernis mehr: eine von-Kármán-Platte rechnet in C++ unter Echtzeit (DAFx23,
nachgelesen), vorab je Stärke reichen Sekunden. Was fehlt, ist die Form: Gong und Tamtam sind (fast) flach und gut
aufgenommen (Iowa reflexionsarm), Crash und Ride brauchen eine Schale mit Kuppe und Verjüngung, deren Maße und Material
nicht gemessen sind; das Hi-Hat hat nur ein Modell, das am Fußdruck scheitert.

**a (Empfehlung):** „Becken und Gong werden physikalisch gerechnet (von Kármán, energieerhaltend), vorab je Stärke und
Schlagstelle — kein Deluxe-Pfad allein. Reihenfolge: Tamtam und Gong (Platte, Gleiten) zuerst, dann Crash, China, Splash
und Ride (Schale; Form an den Iowa-Aufnahmen gesetzt), zuletzt das Hi-Hat. Die Spalte Risiko in KLASSEN.md nennt künftig
‚Form und Daten‘ statt ‚Rechenzeit‘.“

**b:** „Becken werden mit der billigen Stufe gerechnet (gemessene Moden plus Bandtransport, §2.6); von Kármán nur für
Gong und Tamtam und als Deluxe.“

**c:** „Becken bleiben Samples (Iowa, DrumGizmo); gerechnet werden nur Gong und Tamtam.“

**Warum die Empfehlung:** Den Ausschlag gibt die Treue: das Aufblühen, das Crash-Rauschen und das Gleiten entstehen aus
derselben Nichtlinearität — a rechnet sie, statt sie nachzubilden, und die Rechenzeit erlaubt es. Für b spricht, dass die
billige Stufe sich an Aufnahmen setzen ließe, ohne die Form zu kennen — das wiegt weniger, weil sie als Verfahren nicht
belegt ist und den Zusammenhang Stärke–Klangfarbe nur abbildet. c gäbe die Klasse für Becken auf. **Kippt auf b** für
Crash und Ride, wenn die Schale ohne gemessene Form die Iowa-Aufnahmen nicht trifft. *Nach der Regel „Pausieren nur mit
Freigabe“:* a stellt das Hi-Hat ans Ende, bis der Fußdruck numerisch gelöst ist — hiermit vorgelegt.

---

## F-2 — Vorab gerechnete Töne aus chaotischen Rechnungen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT E1): a.**

**Ort:** `Spielweisen_Konzept.md` §4.2–§4.5 (Töne vorab, Tonspeicher, Freeze); Antwort Claude S154 zu Z1 („den Bezug Bit
für Bit“). **Heutiger Wortlaut:** Töne werden vorab gerechnet und gespeichert; der Bezug wird Bit für Bit geprüft.

**Problem:** Becken und Gong rechnen chaotisch: kleinste Unterschiede (Rechenreihenfolge, FMA, SIMD-Breite) ändern den
Verlauf nach kurzer Zeit, statistisch bleibt er gleich (Bericht, Chaigne et al. 2005: positive Lyapunov-Exponenten). Ein
Test „Bit für Bit“ bricht dann bei jeder Änderung am Compiler.

**a (Empfehlung):** „Das Modell rechnet mit festen Regeln (feste Reihenfolge, kein fast-math, keine automatische FMA), so
dass derselbe Build denselben Ton liefert; der Tonspeicher trägt die Modellversion. Tests prüfen bei chaotischen Modellen
zusätzlich Statistiken (Band über der Zeit, Schwerpunkt) statt nur Abtastwerte; ein Bit-Unterschied nach einem
Compiler-Wechsel ist dort kein Fehler, eine abweichende Statistik schon.“

**b:** „Nur Statistik-Tests; Bitgleichheit wird für diese Modelle nicht verlangt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass ein gespeicherter Ton reproduzierbar bleiben muss (Freeze, Austausch
von Loops) — a hält das für denselben Build und macht die Tests trotzdem robust gegen das, was Chaos unvermeidlich ändert.
Für b spricht die Einfachheit — das wiegt weniger, weil ohne feste Regeln derselbe Loop auf zwei Rechnern verschieden
klingen könnte. **Kippt auf b**, wenn feste Regeln die Rechenzeit über die Grenze heben.

---

## F-3 — Hi-Hat: Öffnung als Größe an der Note statt zweier Rollen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a.**

**Ort:** `Loop_Konzept.md` **K5 a** (Rollen `hat`, `open hat`), K2 a. **Heutiger Wortlaut:** „Feste Rollen … hat, open
hat …“.

**Problem:** Offen, halboffen, geschlossen ist physikalisch eine Größe (Abstand bzw. Andruck der Becken; Sekiguchi &
Samejima 2023); DrumGizmo nimmt drei Stufen auf (closed, semi-open, open) plus Fuß. Dazu gehört eine Abhängigkeit über die
Rollen: ein geschlossener Schlag oder ein Fuß-Chick beendet den klingenden offenen Hi-Hat (ungemessen).

**a (Empfehlung):** „Die Rollen `hat` und `open hat` bleiben (Kits, alte Loops). Ein gerechnetes Hi-Hat liest die Öffnung
als Artikulation am Schritt (geschlossen, halb, offen; Vorgabe nach der Rolle), dazu *chick* (Fuß) und *splash*. Ein
Kit erhält eine Abschneide-Gruppe: ein Schritt der Gruppe beendet den klingenden Ton einer anderen Rolle derselben Gruppe
(für alle Kits, auch mit Samples).“

**b:** „Zwei Rollen wie heute; nur die Abschneide-Gruppe kommt dazu.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass halboffen und Chick ohne einen Ort für die Öffnung nicht spielbar sind,
und a fügt sich in F-3 der Klasse 5 (Spielweisen am Schritt) und K5 a, ohne Rollen zu brechen. Für b spricht der kleinste
Eingriff — das wiegt weniger, weil b drei gemessene Stufen auf zwei Rollen verteilt und das halboffene Hi-Hat verliert.
**Kippt auf b**, wenn Klasse 5 F-3 b gewählt wird.

---

## F-4 — Choke: Abdämpfen mit der Hand am Ende der Note

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T6): a.**

**Ort:** `Spielweisen_Konzept.md` Zielbild (Schlagwerk „Abdämpfen mit der Hand (Becken, Vibraphon)“), SF8 a.
**Heutiger Wortlaut:** Abdämpfen je Note.

**Problem:** Ein Drum-Schritt hat heute keine Länge; der Klang läuft aus. Gemessene Abklingzeiten nach dem Greifen gibt es
nicht; Aufnahmen mit Choke schon (Iowa `chokecrash`, CrocellKit `CrashLStopped`).

**a (Empfehlung):** „*Choke* ist eine Artikulation mit Länge am Schritt: am Ende der Länge greift die Hand, das Modell
dämpft (Dämpfung an der Kante über eine kurze Zeit), gesetzt an den Choke-Aufnahmen. Ohne *choke* klingt das Becken
aus.“

**b:** „Choke als eigener Klang (abgeschnittene Aufnahme bzw. kurzer Schlag), ohne Länge.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass der Zeitpunkt des Greifens musikalisch gesetzt wird (auf einen
Schlag) — nur eine Länge trägt ihn, und das Modell dämpft dann aus dem schwingenden Zustand. Für b spricht, dass kein
Feld nötig ist — das wiegt weniger, weil b den Zeitpunkt festlegt. **Kippt auf b**, wenn Drum-Schritte keine Länge
bekommen sollen (gleiche Frage wie Klasse 5 F-4).

---

## F-5 — Material und Form: gemessen oder gesetzt

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T9): a.**

**Ort:** [modell-entwurf.md](modell-entwurf.md) §4. **Heutiger Wortlaut:** neu.

**Problem:** Für B20-Bronze gibt es nur Rechenwerte ohne Herkunft (8860 kg/m³, 110 GPa), für die Form echter Becken
keine Messung; die Moden hängen stark von der Wölbung ab.

**a (Empfehlung):** „Material und Form werden an den Iowa-Aufnahmen gesetzt (Modalfit der leisen Schläge, M1): die
Rechnung sucht Dicke, Kuppe und Wölbung, die die gemessenen tiefen Moden treffen; Materialwerte aus einer Quelle mit
Herkunft, sobald gefunden (Forschungsauftrag), sonst als gesetzt gekennzeichnet.“

**b:** „Form und Material werden nach Herstellerangaben geschätzt und nicht gesetzt.“

**Warum die Empfehlung:** Den Ausschlag gibt die Lehre aus dem Flügel: jede Größe hat eine Messgröße, die sie verursacht —
hier die tiefen Moden. Für b spricht, dass kein Fit über die Form nötig ist — das wiegt weniger, weil eine geschätzte
Wölbung die Moden um mehr verschiebt, als man hört. **Kippt auf b**, wenn der Fit über die Form nicht eindeutig ist.

---

## F-6 — Bezugsaufnahmen und Downloads

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): a.**

📁 **Ablage (Anweisung Patrik 2026-10-26, IC Session 2, A-1 a, für alle Klassen gleich):** Aufsätze kommen ins Repo nach `quellen/`; **Aufnahmen und Messdaten lokal** nach `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für LumiViz erreichbar), im Repo nur `aufnahmen/<Datensatz>/` bzw. `daten/<Datensatz>/` mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`; jeder Datensatz in [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). „Neben LumiViz“ im Text unten ist damit überholt; welche Dateien geladen werden, entscheidet diese Frage.

**Ort:** [aufnahmen.md](aufnahmen.md). **Heutiger Wortlaut:** neu.

**Problem:** Iowa (Lizenz nachgelesen) hat Becken, Hi-Hat, Tamtam, Gong systematisch und reflexionsarm; DrumGizmo die
Hi-Hat-Stufen und Choke; VSCO 17 Gong-Stärken; Beijing Opera die Operngongs mit Gleiten (CC BY 4.0).

**a (Empfehlung):** „Bezug: Iowa (Becken, Tamtam, Gong), DrumGizmo (Hi-Hat, Ride, Choke — nur die Becken-Instrumente per
Teil-Abruf), VSCO (Gong-Stärken), Beijing Opera (Gleiten), Oslo (Schlagstelle). Vor jedem Laden legt Claude Dateien,
Größen und Zielordner neben LumiViz zur Freigabe vor; die freien Aufsätze DAFx23 (CC BY 4.0) und SMAC 2013 (CC BY 3.0)
nach Freigabe unter `quellen/`.“

**b:** „Nur Iowa.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass Iowa kein Hi-Hat-Halboffen, keine feinen Stärken und keine Operngongs
hat — jede Lücke deckt ein freier Datensatz. Für b spricht die einheitliche, trockene Aufnahme — das wiegt weniger, weil
die anderen nur für ihre Größe genutzt werden. **Kippt auf b**, wenn F-3 b und Klasse 5 F-2 b gewählt werden.

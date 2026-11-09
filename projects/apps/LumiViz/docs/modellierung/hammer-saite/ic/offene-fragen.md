# Offene Fragen — Hammer-Saite (Rest), Klasse 1

> **Stand:** 2026-10-26, IC Session 1. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz (IC-B1 a) mit Datei und Entscheid, alle in `…\LumiViz\projects\apps\LumiViz\docs\audio\
> Spielweisen_Konzept.md`, nachgelesen 2026-10-26. Das Zielbild der Klasse steht schon in LumiViz (§4.1, S141 —
> „Instrumente: alle“); F-1 fragt nur nach der Reihenfolge. F-2 bis F-4 gleichen den Bestand ab.
> **Entscheide Patrik (2026-10-26, IC Session 2):** F-1, F-2, F-3, F-7, F-8 a; F-5 a abgewandelt (Downloads in dieses
> Repo); F-6 **b**; F-4 über F-4-1 **a** (Schieber stufenlos, Tastendruck als Pitch nach oben, Grenze 0 bis zur Messung). **Hinweis Patrik:** Die Hammer-Saite wird in LumiViz am
> Flügel gerade bearbeitet (Stufe 31, S155) — vor jeder Arbeit an dieser Klasse den Stand in LumiViz neu prüfen.

---

## F-1 — Reihenfolge innerhalb der Klasse

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §4.1 Zielbild, Antwort 1; [modell-entwurf.md](modell-entwurf.md) §2. **Heutiger
Wortlaut:** „Zuerst Konzertflügel und Klavier (Upright); danach kommen ebenso Honky-tonk …, der E-Flügel …, das
Clavinet mit echter Tangente statt Zupfen, Hackbrett und Cimbalom gerechnet statt aufgenommen.“

**Problem:** Das Zielbild legt den Anfang fest (Upright, Stufe 32), die übrigen fünf nur als Liste. Die Recherche zeigt
große Unterschiede: Honky-tonk ist nur eine Kurve; der CP-70 ist das Flügelmodell ohne Boden, aber ohne freie
Aufnahme; das Clavinet braucht eine neue Anregung (F-3) und hat keine belastbare Aufnahme; Hackbrett und Cimbalom
brauchen eine über den Steg geteilte Saite, Mitschwingen aller Saiten und fast fehlende Daten.

**a (Empfehlung):** „Nach dem Upright: Honky-tonk (eine Vorlage mit weiter Verstimmung, gesetzt an FreePats), dann
CP-70 (Flügel ohne Boden, Stegkraft als Abnahme), dann Clavinet (neue Anregung Tangente), dann Hackbrett; das Cimbalom
zuletzt und erst mit einer Aufnahme (Forschungsauftrag). Hackbrett und Cimbalom tragen die Marke Deluxe.“

**b:** „Nach dem Upright die Instrumente in der Reihenfolge des Zielbilds, ohne Bedingung für das Cimbalom.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass jeder Schritt auf dem vorigen aufbaut und an einer Messung geprüft
werden kann, solange es eine gibt — Honky-tonk und CP-70 nutzen das Flügelmodell fast unverändert, das Clavinet bringt
eine neue Anregung, Hackbrett und Cimbalom das Mitschwingen (das in LumiViz mit 31e erst entsteht). Für b spricht,
dass es keinen neuen Entscheid braucht — das wiegt weniger, weil die Liste im Zielbild keine Reihenfolge der Physik ist
und ein Cimbalom ohne Aufnahme nicht gesetzt werden könnte. **Kippt**, wenn Patrik ein Instrument im Klang vorziehen
will (wie SF9 a es vorsieht).

---

## F-2 — Upright-Pedale: Bass-Sustain

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §4.1, HA4 a. **Heutiger Wortlaut:** „`pedals` je Platz links, Mitte, rechts mit
einer Art: links *Una corda* (Verschiebung) oder *Soft* (Hämmer näher), Mitte *Sostenuto*, *Practice* (Moderator) oder
keins, rechts *Sustain* … Kippt auf b, wenn die Recherche zum Upright zeigt, dass sich seine Pedale nicht auf diese
Plätze abbilden lassen.“

**Problem:** Links *Soft* (Half-blow), Mitte *Practice* (Moderator) und *Sostenuto* (selten, teure Uprights) passen
(nur Bericht: Wikipedia „Piano pedals“). Nicht abbildbar sind: Mitte **Bass-Sustain** („the middle pedal may only
operate the bass dampers“) und Mitte **Silent** (eine Leiste fängt die Hämmer ab — kein Saitenklang). HA4 a kippt also
für einen Teil der Uprights — nicht auf b (drei Schalter), sondern um eine Art.

**a (Empfehlung):** „HA4 a bleibt; der mittlere Platz bekommt eine vierte Art *Bass sustain* mit einer Grenztaste (die
erste Taste, deren Dämpfer nicht gehoben wird). Die Vorlage *Upright* setzt Mitte *Practice*. *Silent* wird nicht
abgebildet (kein Klang des Modells).“

**b:** „HA4 a bleibt unverändert; Bass-Sustain gibt es nicht.“

**Warum die Empfehlung:** Den Ausschlag gibt die fachliche Treue bei kleinstem Eingriff — HA4 a ist als Liste von Arten
gebaut, eine Art mehr ist genau sein Weg, und der Dämpfer je Taste ist im Modell schon da (`undampedFrom` zeigt, dass
eine Grenztaste ein gewohntes Feld ist). Für b spricht, dass die Vorlage *Upright* ohnehin den Moderator nimmt — das
wiegt weniger, weil ein Nutzer, der ein Upright mit Bass-Sustain nachbauen will, sonst keinen Weg hat und der
Kipppunkt von HA4 a genau diesen Fall meint. **Kippt auf b**, wenn Patrik nur die Vorlage *Upright* (mit Moderator) will
und keine Varianten im Editor.

---

## F-3 — Clavinet: Tangente als Hammer oder eigene Anregung

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §1 SF10 a und §4.1 HA2 a. **Heutiger Wortlaut:** SF10 a — „die Saite vier (Zupfen,
Hammer, Bogen, die Tangente des Clavinets als Hammer)“; HA2 a — „Clavinet und Bogen-Saite werden neue Werte auf
denselben Achsen. … Kippt auf b, wenn beim Clavinet Hammer und Zupfen an der Saite nichts teilen außer dem Namen.“

**Problem:** Die Tangente „strikes the string and traps it against a metal stud, or anvil, for the duration of the
note“ und teilt die Saite in einen klingenden und einen stummen Teil; beim Loslassen werden beide wieder eine Saite,
die Tonhöhe fällt um drei Halbtöne (nachgelesen, Gabrielli et al. 2013, S. 4, 6). Mit dem Hammer teilt sie den
Aufschlag, mit dem Zupfen nichts; dazu ist sie eine **Randbedingung**, die sich beim Loslassen ändert. „Tangente als
Hammer“ (SF10 a) beschreibt nur den Aufschlag. Der Kipppunkt von HA2 a (Hammer und Zupfen teilen nichts) greift nicht —
es geht um einen dritten Wert.

**a (Empfehlung):** „Die Achse *Excitation* bekommt einen dritten Wert *Tangent*: Aufschlag mit dem Kontakt des
Hammers (Q0·Δy^p), danach Klemme am Amboss (Moden der klingenden Länge), beim Loslassen Wechsel auf die Moden der
ganzen Saite mit Garndämpfung. SF10 a nennt die Tangente als eigene Anregung des Clavinets.“

**b:** „Die Tangente bleibt *Hammer*; Klemme und Loslassen sind Ausstattung des Clavinets (wie der Dämpfer).“

**Warum die Empfehlung:** Den Ausschlag gibt die Regel von SF10 a selbst — Bauart ist Anregung × Schwinger, und die
Tangente regt anders an als ein Hammer (sie bleibt, klemmt, bestimmt die Länge und erzeugt beim Loslassen einen eigenen
Klang); HA2 a sieht für das Clavinet ausdrücklich „neue Werte auf denselben Achsen“ vor, also genau diesen Weg. Für b
spricht, dass der Aufschlag derselbe Kontakt ist und keine neue Achsenstufe entsteht — das wiegt weniger, weil die
Klemme keine Ausstattung (fest am Instrument) ist, sondern der Vorgang der Anregung, und ein Hammer mit Klemme im Editor
falsch hieße. **Kippt auf b**, wenn sich beim Bau zeigt, dass Klemme und Loslassen ohne eigenen Code aus dem Hammer
und dem Dämpfer folgen.

---

## F-4 — Clavinet: Druck auf die Taste und Dämpferschieber

✅ **Antwort Patrik (2026-10-26): „beides?“ → entschieden über F-4-1 a** (ersetzt F-4 a) — siehe [F-4-1](#f-4-1--clavinet-druck-auf-die-taste-und-dämpferschieber-beides) unten.

**Ort:** `Spielweisen_Konzept.md` SF6 a (Tabelle, Zeile „Hammer-Saite (Klavier)“), SF8 a. **Heutiger Wortlaut:**
SF6 a — Expression „nein — nach dem Anschlag nicht steuerbar“, Vibrato „nein“, Pitch „nein“.

**Problem:** Die Bedienungsanleitung des D6 sagt „intonation depends on keyboard pressure“, Lexika nennen
„aftertouch as the string is stretched“ (nur Bericht); gemessen ist nichts, Spieler berichten, dass es am eigenen Gerät
nicht geht, und Gabrielli misst f0 „very stable over time“ (1–2 Cent, nur Bericht). Der Dämpferschieber (mute slider)
ist verstellbar, empfohlen sind „the end positions“ (Anleitung, nur Bericht).

**a (Empfehlung):** „Druck auf die Taste wird nicht gebaut (SF6 a bleibt, auch für das Clavinet). Der Dämpferschieber
ist eine Klang-Spielweise über eine Passage (SF8 a) mit zwei Stellungen, an und aus.“

**b:** „Druck wirkt als Pitch an der Note (wie ein Bend), Dämpferschieber stufenlos als Kurve.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass nur gebaut wird, was belegt ist — der Druckeffekt ist behauptet,
nicht gemessen, und die einzige Messung zeigt eine stabile Tonhöhe; der Schieber wird nach der Anleitung nur in den
Endlagen benutzt. Für b spricht, dass Spieler den Effekt als Kennzeichen des Clavinets nennen — das wiegt weniger, weil
ein ungemessenes Modell ein erfundener Klang wäre. **Kippt auf b**, wenn eine Aufnahme oder Messung den Druckeffekt
zeigt.

---

## F-5 — Bezugsaufnahme für das Upright

🔁 **Ablage geändert durch A-1 a (Patrik 2026-10-26):** Aufnahmen und Messdaten liegen nicht mehr im Repo, sondern unter `…\cmake\samples\Klasse 1 - Hammer-Saite\`; im Repo nur `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`.

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a, abgewandelt** — „downloads gleich wie schon bei klasse 2“: geladen wird **in dieses Repo** (`klassen/Klasse 1 - Hammer-Saite/aufnahmen/`), nicht neben LumiViz; Eintrag in [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). Stand: [aufnahmen.md](aufnahmen.md).

**Ort:** [aufnahmen.md](aufnahmen.md); LumiViz §4.1 („Ein Upright mit an der Quelle geprüfter Lizenz: noch keins
gefunden.“). **Heutiger Wortlaut:** neu.

**Problem:** **K18 Upright Piano** (Rudi Fiasco) ist CC BY 4.0 am Ort (Metadaten nachgelesen), kleine Terzen ab A0,
6 Stufen, Loslass- und Pedalgeräusche, AB-Paar ~30 cm, 48 kHz/24 bit, 424 MB — Hersteller und Raum unbekannt. Die
beiden VCSL-Uprights sind CC0, aber dünner (2 bzw. 3 Stufen, Ganz- bzw. C/G-Raster).

**a (Empfehlung):** „K18 ist die Bezugsaufnahme des Upright (CC BY 4.0, Nennung in `CREDITS.md`, wenn etwas davon
mitgeliefert würde); die beiden VCSL-Uprights dienen zum Gegenprüfen. Zum Laden legt Claude Datei, Größe (424 MB) und
Zielordner neben LumiViz (mit README: Quelle, Lizenz, SHA-256) zur Freigabe vor.“

**b:** „Ein VCSL-Upright (CC0) ist der Bezug; K18 nur zum Vergleich.“

**Warum die Empfehlung:** Den Ausschlag gibt die Eignung — K18 ist wie Salamander aufgebaut (Raster, Stufen,
Loslassen), und genau dieser Aufbau hat beim Flügel das Setzen durch das Modell getragen; CC BY 4.0 erlaubt die Nutzung
mit Nennung, wie bei Salamander. Für b spricht die freiere Lizenz (CC0, keine Nennung) — das wiegt weniger, weil 2–3
Stufen die Helligkeit über die Stärke kaum setzen. **Kippt auf b**, wenn K18 beim Prüfen der Dateien Raumanteil oder
Schnittfehler zeigt.

---

## F-6 — Instrumente ohne freie Aufnahme (CP-70, Clavinet, Cimbalom)

✅ **Entscheid Patrik (2026-10-26, IC Session 2): b** — „bis recherchen noch mehr gefunden haben“: alle drei werden aus Literatur und Modell gebaut, auch das Cimbalom; was spätere Recherchen finden (Aufnahmen, Saitendaten), setzt sie nach. Damit ist das Zurückstellen des Cimbaloms nicht freigegeben.

**Ort:** [aufnahmen.md](aufnahmen.md), [messplan.md](messplan.md) B. **Heutiger Wortlaut:** neu.

**Problem:** Für CP-70/80, Clavinet und Cimbalom gibt es keine Aufnahme mit am Ort geprüfter freier Lizenz (Reichweite
im Bericht). Das Clavinet hat aber eine frei lizenzierte Messung in der Literatur (Gabrielli et al. 2013, CC BY 2.0:
B je Taste, T60, Schwebung, Loslassen).

**a (Empfehlung):** „CP-70 und Clavinet werden aus Literatur und Modell gebaut und als ‚ungemessen‘ gekennzeichnet; das
Clavinet wird gegen die Werte von Gabrielli et al. 2013 geprüft (nicht gesetzt). Das Cimbalom wartet auf eine Aufnahme
(Forschungsauftrag, [messplan.md](messplan.md) C) — es wird nicht ohne gebaut.“

**b:** „Alle drei werden aus Literatur und Modell gebaut, auch das Cimbalom.“

**c:** „Keines der drei wird gebaut, bevor es eine freie Aufnahme gibt.“

**Warum die Empfehlung:** Den Ausschlag gibt, wie viel das Modell ohne Aufnahme trägt — CP-70 ist das gemessene
Flügelmodell ohne Boden, das Clavinet hat eine veröffentlichte Messung; das Cimbalom hätte weder Daten noch Prüfstein.
Für b spricht Vollständigkeit — das wiegt weniger, weil ein Cimbalom ohne Daten erfunden wäre. Für c spricht, dass
nichts Ungemessenes entsteht — das wiegt weniger, weil CP-70 und Clavinet auf belegten Teilen stehen. *Hinweis nach
der globalen Regel „Pausieren nur mit Freigabe“:* a stellt das Cimbalom zurück; das ist hiermit zur Freigabe vorgelegt.
**Kippt** für das Cimbalom auf b, wenn Pap 2000 oder Fleischer 2004 Saitendaten liefern.

---

## F-7 — Saitendaten des Upright

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** [modell-entwurf.md](modell-entwurf.md) §3 (`stringLength`, `stringMass`). **Heutiger Wortlaut:** neu (beim
Flügel: „RT-0425 Anhang A, µ = ρπd²/4 (HT1 a)“).

**Problem:** Eine Mensur eines modernen Upright (Länge, Durchmesser, Wicklung je Taste) ist frei nicht gefunden;
Fletcher 1964 (Hamilton-Upright) ist nicht erreicht. Ohne Saitendaten rechnet das Modell mit geschätzten Saiten, und
B, Hammer und Helligkeit werden gegen K18 durch ein Modell mit falschen Saiten gesetzt.

**a (Empfehlung):** „Zuerst Fletcher 1964 über eine Bibliothek beschaffen (Patrik oder Download mit Freigabe); fehlt
es, wird die Mensur eines Upright aus Bass-Länge, typischen Durchmessern und dem gemessenen B (K18) zurückgerechnet und
als ‚abgeleitet‘ gekennzeichnet.“

**b:** „Die Mensur des Steinway D wird auf die Länge eines Upright skaliert.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass jede Größe an der Messgröße gesetzt wird, die sie verursacht — B
hängt an Länge und Durchmesser, also liefert das gemessene B eine Prüfung der Mensur; eine Quelle ist besser als eine
Ableitung, eine Ableitung aus Messung besser als eine Skalierung. Für b spricht, dass es sofort geht — das wiegt
weniger, weil ein Upright anders mensuriert ist (kürzere Bässe, andere Spannung) und die Lehre aus dem Flügel ist, dass
ein Modell, das die Messung nicht erreichen kann, falsche Werte setzt. **Kippt auf b**, wenn K18 zeigt, dass B im Bass
sich mit skalierter Mensur auf ±5 % trifft.

---

## F-8 — Zusatzfrage Z1: Moden und Waveguides bei der Hammer-Saite

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §4, **HA2 a** (Hammer-Saite modal) und **SZ3 a** (Setzen am Modell der Moden je
Teilton); dort die Antwort Claude S154 „Für 31d-7 also kein Gewinn; als Idee für später vorgemerkt (etwa für hohe
Teiltöne über der Grenze der Moden)“. **Heutiger Wortlaut:** wie zitiert.

**Problem:** Die Literatur bestätigt den modalen Weg für das Setzen: modal „all the parameters of the partials can be set
independently“, und Längswellen „precise modeling is not possible with digital waveguides in their efficient form“
(Bank, Zambon & Fontana 2010, nachgelesen PDF-S. 3); die Vorhersage je Teilton eines Waveguides kennt nur eine
Abklingstufe und keine Schwebung (Bank & Välimäki 2003, nur Bericht). Zwei Mischformen sind belegt: (A) Waveguide plus
wenige Resonatoren für die schwebenden Teiltöne — „Five to ten resonators have been found to be enough“ (Bank 2006,
nachgelesen PDF-S. 68, gedruckt 54); (B) Moden für die tiefen Teiltöne, Waveguide für die hohen (Lee, Smith, Abel &
Berners 2009, nur Bericht). Gegen modal spricht nur das Mitschwingen vieler Saiten: im modalen Flügel ist die Kopplung
„uni-directional“ (Bank et al. 2010, nachgelesen PDF-S. 9) — wichtig für Hackbrett und Cimbalom.

**a (Empfehlung):** „HA2 a bleibt: die Hammer-Saite ist modal, gesetzt nach SZ3 a. Die Mischform B (Waveguide über der
Grenze der Moden) wird erst geprüft, wenn eine Hörprobe zeigt, dass die Obergrenze der Moden im Diskant hörbar ist. Für
Hackbrett und Cimbalom (viele mitschwingende Saiten auf einem Steg) wird vor dem Bau geprüft, ob die einseitige modale
Kopplung reicht; sonst kommt dort die zweiseitige Kopplung (gemeinsam gelöst oder als Waveguide am Steg).“

**b:** „Die Mischform B wird gleich eingebaut (Moden bis zu einer Grenze, darüber Waveguide), für alle Hammer-Saiten.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Literatur genau das bestätigt, was SZ3 a braucht — zwei Stufen,
Schwebung, Längswellen je Teilton —, und dass kein Befund einen hörbaren Mangel der Modengrenze zeigt. Für b spricht,
dass hohe Teiltöne ohne Grenze kämen — das wiegt weniger, weil der Waveguide dort Steuerung und genaue Inharmonizität
verliert („±8.39 cents“ als Fehlergrenze eines abgestimmten Dispersionsfilters, Rauhala & Välimäki 2006, nur Bericht)
und die Töne vorab gerechnet werden, der Rechenvorteil also wenig zählt. **Kippt auf b**, wenn die Hörprobe die Grenze
hörbar macht. Bericht: [gemeinsam/recherche/2026-10-26_moden_und_waveguides_saite.md](recherche/2026-10-26_moden_und_waveguides_saite.md).

---

## F-4-1 — Clavinet: Druck auf die Taste und Dämpferschieber — beides?

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** F-4 oben; `Spielweisen_Konzept.md` SF6 a (Hammer-Saite: Pitch „nein“), SF8 a. **Heutiger Wortlaut:** F-4 a —
„Druck auf die Taste wird nicht gebaut … Der Dämpferschieber ist eine Klang-Spielweise über eine Passage (SF8 a) mit
zwei Stellungen, an und aus.“ Antwort Patrik: „beides?“

**Problem:** „Beides“ lässt sich auf zwei Arten lesen — (1) Druck **und** Schieber bauen, (2) beim Schieber beides, die
Endlagen **und** Zwischenstellungen. Zum Druck gibt es weiterhin nur Behauptungen („intonation depends on keyboard
pressure“, Anleitung D6; „aftertouch as the string is stretched“, Lexika — nur Bericht) und eine Messung, die eine stabile
Tonhöhe zeigt (Gabrielli et al. 2013: 1–2 Cent, nur Bericht); wie weit der Ton steigt, weiß niemand. Physikalisch ist
der Effekt plausibel: die Tangente drückt die Saite gegen den Amboss, mehr Druck dehnt sie und hebt die Spannung.

**a (Empfehlung):** „Beides wird gebaut, mit unterschiedlicher Belegkraft. Der Dämpferschieber ist eine Spur über die
Passage mit stufenloser Stellung 0 … 1; die Vorgaben sind die Endlagen, wie die Anleitung empfiehlt. Der Druck auf die
Taste ist ein Pitch an der Note **nur nach oben** (wie ein Bend), Datenformat und Ort gleich jetzt; seine Grenze steht auf
**0 Cent**, bis eine Messung den Effekt beziffert, und das Instrument zeigt ihn als ‚ungemessen‘. Damit gilt für das
Clavinet in SF6 a: Pitch ‚ja (Tastendruck, Grenze aus Messung)‘.“

**b:** „Beides sofort hörbar: Druck als Pitch an der Note bis zu einer geschätzten Grenze (Vorschlag Claude: +20 Cent,
Einschätzung ohne Beleg), Schieber stufenlos.“

**c:** „Nur der Schieber, stufenlos; Druck auf die Taste gar nicht (SF6 a bleibt).“

**Warum die Empfehlung:** Den Ausschlag gibt die Regel, nur Gemessenes hörbar zu machen, ohne das Datenformat später
ändern zu müssen — a legt Ort und Format für den Tastendruck jetzt fest (Spieler nennen ihn ein Kennzeichen des
Clavinets), lässt die Größe aber an einer Messung hängen; der Schieber ist ein stufenloses Bauteil, die Endlagen sind nur
die übliche Wahl. Für b spricht, dass der Effekt sofort hörbar wäre — das wiegt weniger, weil +20 Cent eine erfundene
Zahl ist und die einzige Messung eher dagegen spricht. Für c spricht, dass nichts Ungemessenes entsteht — das wiegt
weniger, weil der Ort für den Tastendruck dann später nachträglich ins Format müsste. **Kippt auf b**, wenn Patrik einen
ausdrücklich als ungemessen gekennzeichneten Klang will, bevor eine Messung da ist; **auf c**, wenn er keine Spur zeigen
will, die nichts tut.

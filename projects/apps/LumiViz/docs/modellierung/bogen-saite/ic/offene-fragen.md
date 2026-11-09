# Offene Fragen — Bogen-Saite (Klasse 3)

> **Stand:** 2026-10-26, IC Session 1. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz (nachgelesen 2026-10-26) unter `…\LumiViz\projects\apps\LumiViz\docs\`: `audio/Spielweisen_Konzept.md`,
> `audio/Saiten_Konzept.md`. Belege in [modell-entwurf.md](modell-entwurf.md) (Kennzeichnung je Angabe).
> **Entscheide Patrik (2026-10-26, IC Session 2):** F-1 a, **F-2 b**, F-3 bis F-8 a, F-9 a (Downloads freigegeben).

---

## F-1 — Umfang und Reihenfolge

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 3. **Heutiger Wortlaut:** „Violine, Viola, Cello, Kontrabass,
Streicher-Ensemble“.

**Problem:** Neu ist die Reibstelle Bogen–Saite; sie ist nur am **Cello** mit gemessenem Bogen belegt (Roboterbogen der
mdw Wien: Kraft, Geschwindigkeit, Abstand zum Steg, Stegkraft; dazu die Wolfton-Mode mit Güte und Stärke bei Zhang &
Woodhouse). Die **Violine** hat die besten Aufnahmen und die Stegmobilität dreier Instrumente (CNSM), aber keine
Bogenkraft-Messung mit Klang. Viola und Kontrabass fehlen Saiten- und Korpusdaten.

**a (Empfehlung):** „Zuerst der Kern an der Cellosaite ohne Korpus (Reibung gegen die Roboterbogen-Messungen), dann
das Cello mit Korpus (Wolfton als Prüfstein), dann die Violine (Korpus aus CNSM, F-7), dann Viola und Kontrabass, sobald
Saiten- und Korpusdaten vorliegen, zuletzt das Ensemble aus den fertigen Stimmen.“

**b:** „Zuerst die Violine — `violin` ist das bekannteste Instrument und heute nur eine Säge.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass der neue Teil, die Reibung, dort gesetzt wird, wo sie gemessen ist —
an der Cellosaite lässt sich das Schelleng-Fenster des Modells gegen eine Messung legen, ohne dass ein ungemessener
Korpus mitspielt. Für b spricht, dass die Violine das sichtbarste Instrument ist — das wiegt weniger, weil sie nach dem
geprüften Kern nur noch Saitenwerte und Korpus braucht. **Kippt auf b**, wenn sich die Messungen der mdw als ungeeignet
erweisen (etwa Format oder Saitentyp). *Nach der Regel „Pausieren nur mit Freigabe“:* a stellt Viola und Kontrabass bis
zu Saiten- und Korpusdaten zurück — hiermit vorgelegt.

---

## F-2 — Saitenverfahren für den Bogen: modal oder Waveguide

✅ **Entscheid Patrik (2026-10-26, IC Session 2): b** — die Empfehlung aus dem Nachtrag Z1: Waveguide-Saite aus S-K1 a mit Reibstelle, Korpus als Moden am Steg.

**Ort:** `Saiten_Konzept.md` §3, **S-K1 a**. **Heutiger Wortlaut:** „**Waveguide** (a): die Saite als zwei gegenläufige
Wellen zwischen Sattel und Steg, Verlust- und Steifheitsfilter am Ende; … Korpus als Resonanzen hinter dem Steg. Trägt
später den Bogen (Violine).“

**Problem:** Beide Verfahren tragen eine Reibstelle. Modal: Dämpfung je Mode frei setzbar („without any additional
computational effort“, nur Bericht), gemessen 0,19–0,91 der Echtzeit in Matlab (Russo et al. 2022, nachgelesen), der
Korpus koppelt als weitere Moden am Steg — wie der Flügel, dessen Saite in LumiViz modal ist (Bank 2010). Waveguide: in
LumiViz gebaut und für Gitarren gemessen; die Reibstelle braucht nach den Quellen hohe Abtastraten (Woodhouse-Typ
200 kHz, nur Bericht), Dämpfung je Mode nur über Filter, der Korpus sitzt heute „hinter dem Steg“.

**a (Empfehlung):** „Die Bogen-Saite rechnet die Saite **modal** mit Reibstelle am Bogenpunkt und dem Korpus als Moden am
Steg, die auf die Saite zurückwirken. Saitensatz, Stimmung, Mensur und Spread bleiben die der Saite (§5, Stufe 23);
Pizzicato an einer Bogen-Saite nutzt dieselbe modale Saite. Der Satz ‚Trägt später den Bogen‘ in S-K1 a wird
gestrichen.“

**b:** „Die Bogen-Saite nutzt die Waveguide-Saite aus S-K1 a mit einer Reibstelle; der Korpus wird so umgebaut, dass er
am Steg zurückwirkt.“

**Warum die Empfehlung:** Den Ausschlag gibt die Physik der gestrichenen Saite: Abklingen je Mode, Korpus in der
Rückkopplung (sonst kein Wolfton) und eine Reibstelle bei mäßiger Überabtastung lassen sich modal direkt ausdrücken —
dieselbe Form wie die Saite des Flügels, also eine Rechenart für die gedämpfte, angeregte Saite in LumiViz. Für b
spricht, dass die Waveguide-Saite gebaut, gemessen und für die Gitarre bewährt ist und S-K1 a sie vorsieht — das wiegt
weniger, weil der Korpus dafür ohnehin umgebaut werden müsste und die Reibstelle dort teurer abtastet. **Kippt auf b**,
wenn eine Waveguide-Reibstelle bei zwei- bis vierfacher Überabtastung das Schelleng-Fenster der Messung (M2) genauso trifft.

**Nachtrag Z1 (2026-10-26) — Empfehlung neu: b.** Die Zusatzfrage Z1 hat zwei Punkte gefunden, die gegen a sprechen:
(1) Bank, Zambon & Fontana 2010 koppeln im modalen Flügel die Saiten nur in eine Richtung, damit die Struktur stabil
bleibt — „the coupling of the strings is uni-directional“ (nachgelesen PDF-S. 9); der Wolfton braucht aber gerade die
Rückwirkung des Korpus auf die Saite, und mit Waveguides ist die zweiseitige Kopplung „relatively straightforward“
(ebd., nur Bericht). (2) Für die Reibung „makes a time-domain approach more natural and intuitive“ (Mansour, Woodhouse
& Scavone 2016, nachgelesen PDF-S. 12); dieselbe Arbeit rechnet die Saite als Welle mit gemessener Dämpfung und
Dispersion und den Korpus als 2×2-Admittanz aus Moden (nur Bericht). Der Vorteil von a beim Setzen trägt beim Bogen
wenig: hier entscheidet die Helmholtzbewegung, nicht das Abklingen je Teilton (Einschätzung im Bericht).
**Neue Empfehlung b** in dieser Form: „Waveguide-Saite aus S-K1 a mit Reibstelle; der Korpus als Moden am Steg, als
Admittanz, die auf die Saite zurückwirkt (wie Mansour et al.); Pizzicato auf derselben Saite.“ **Kippt zurück auf a**,
wenn eine zweiseitig gekoppelte modale Rechnung (Saite und Korpus gemeinsam gelöst) stabil gelingt und am Monochord
(M2) besser trifft. Bericht: [gemeinsam/recherche/2026-10-26_moden_und_waveguides_saite.md](../../hammer-saite/ic/recherche/2026-10-26_moden_und_waveguides_saite.md).

---

## F-3 — Expression am Bogen: Bogendruck allein oder Weg durch Geschwindigkeit, Kontaktstelle und Kraft

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §1, **SK4 a**; Auftrag (`AUFGABE.md`, Bestand Klasse 3: „Bogendruck und
-geschwindigkeit wären neu und gehören nach SF8 a an einen Ort“). **Heutiger Wortlaut:** „ein Modell mit dauernder
Anregung die Stärke der Anregung (Blasdruck, Bogendruck) samt Klangfarbe“.

**Problem:** Gemessen hängt der Pegel nicht am Bogendruck: „the amplitude of the string vibration is proportional to
vB/β“ (Schoonderwaldt et al. 2007, nachgelesen); die Kraft trägt die Obertöne und muss im Schelleng-Fenster bleiben,
sonst kratzt oder pfeift der Ton. Eine Expression, die nur die Kraft hebt, würde lauter **und** bald kratzig, statt lauter
und heller.

**a (Empfehlung):** „Expression bleibt eine Zahl 0 … 1 (SK4 a). Die Bogen-Saite bildet sie auf einen Weg durch
Bogengeschwindigkeit, Abstand zum Steg und Bogenkraft ab — leiser: langsamer, weiter vom Steg, weniger Kraft; lauter:
schneller, näher am Steg, mehr Kraft —, so dass die Kraft im Spielbereich bleibt. Der Weg ist eine Größe des Instruments,
gesetzt an pp/mf/ff. Keine eigene Spur für Bogendruck, -geschwindigkeit oder Kontaktstelle; in SK4 a wird ‚Bogendruck‘
zu ‚Bogen (Geschwindigkeit, Kontaktstelle, Kraft)‘.“

**b:** „Drei neue Spuren an der Bogen-Saite: Bogengeschwindigkeit, Kontaktstelle, Bogenkraft; Expression wirkt nur als
Pegel.“

**c:** „Expression ist der Bogendruck allein (Wortlaut SK4 a).“

**Warum die Empfehlung:** Den Ausschlag gibt die gemessene Spielweise — Streicher bewegen alle drei zugleich (im Crescendo
schneller, näher am Steg, mehr Kraft — Schoonderwaldt et al. 2007, nur Bericht) —, und a fasst das wie F-R2 a bei den
Bläsern: eine Spur, deren Bedeutung das Modell physikalisch füllt. Für b spricht die volle Kontrolle (sul ponticello in
einer Note, Bogenkunst) — das wiegt weniger, weil drei gekoppelte Spuren, die nur gemeinsam im Spielbereich bleiben, im
Editor kaum zu führen sind; die Kontaktstelle als Passage deckt F-6 ab. c widerspricht der Messung. **Kippt auf b**, wenn
Hörproben zeigen, dass Spieler gestrichene Farben brauchen, die der Weg nicht erreicht.

---

## F-4 — Strichrichtung und Bogenwechsel

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** neu; Vorbild `Saiten_Konzept.md` §5m–§5q (Strum mit **Richtung** Down/Up an der Note), SF8 a.
**Heutiger Wortlaut:** neu.

**Problem:** Ein Bogen hat endliche Länge — Haar „normal length (650 mm)“ (Guettler, nur Bericht); bei 16,62 cm/s
(gemessen am Cello im Legato, nachgelesen) reicht ein Strich rund 4 s (eigene Rechnung). Jede neue Note beginnt meist
mit neuem Strich, Auf- und Abstrich klingen verschieden (am Frosch liegt das Gewicht der Hand, an der Spitze nicht — Einschätzung), und eine lange Note braucht
hörbare Bogenwechsel.

**a (Empfehlung):** „Jede Note beginnt mit einem neuen Strich; die Richtung wechselt von selbst (Ab, Auf, Ab …) und ist
an der Note umstellbar, wie die Richtung beim Strum. Reicht der Bogen für eine Note nicht, setzt das Modell Bogenwechsel
nach Bogenlänge und Geschwindigkeit und rechnet sie (Abbremsen, Haften, neuer Ansatz). Legato-Ketten (F-6) bleiben in
einem Strich, solange er reicht.“

**b:** „Ein unendlicher Bogen ohne Richtung — jede Note ein gleicher Strich, kein Bogenwechsel.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass Strichrichtung und Bogenwechsel zum Klang einer gestrichenen Linie
gehören und a sie an die Orte legt, die LumiViz dafür schon hat (Richtung an der Note wie beim Strum, SF8 a). Für b
spricht die Einfachheit — das wiegt weniger, weil ein Ton über 4 s ohne Wechsel physikalisch nicht gespielt wird und
die Unterschiede zwischen Auf und Ab sonst fehlen. **Kippt auf b**, wenn die Bogenwechsel in Hörproben stören, ohne nach
Streicher zu klingen.

---

## F-5 — Vibrato: Vorgabe 5,5 Hz und Tiefe

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §3, **SK10 a**. **Heutiger Wortlaut:** „hat eine Schicht keine Rate (0), nimmt die Spur
Vibrato 5,5 Hz … dass gesungenes und gestrichenes Vibrato um 5 bis 7 Hz liegt, aus dem Gedächtnis — in der Recherche
der Bogen-Saite zu belegen.“

**Problem:** Belegt (Crossref-Abstracts, nachgelesen): „mean rate of vibrato was approximately 5.5 Hz“ bei Schülern und
Studenten an Violine und Cello (Geringer & Allen 2004); 5,9 Hz und „mean excursion of ±15.2 cents“ an der Violine
(Mellody & Wakefield 2000); bei Solisten 6,63 Hz und 63 Cent Breite (MacLeod 2010); in hohen Lagen „0.32 Hz faster and
approximately 26 cents wider“ (MacLeod 2008). 5,5 Hz ist also belegt, aber als unterer Rand; Profis liegen um 6–6,6 Hz.

**a (Empfehlung):** „SK10 a bleibt: 5,5 Hz ist belegt (Geringer & Allen 2004). Die Streicher-Instrumente tragen eine
eigene Rate von 6 Hz (Mellody & Wakefield 5,9 Hz; MacLeod 6,63 Hz bei Solisten) und erhöhen sie in hohen Lagen leicht.
Der Satz ‚aus dem Gedächtnis‘ in SK10 a wird durch die Belege ersetzt. Die Amplitudenmodulation entsteht am Korpus von
selbst (wie F-R3 a), sie ist keine eigene Größe.“

**b:** „Die Vorgabe in SK10 a wird für alle Instrumente auf 6 Hz gesetzt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass SK10 a eine Vorgabe für alle Instrumente ist und die Rate einer
Schicht sie überschreibt — so bekommt jedes Instrument die gemessene Rate seines Spielers, ohne die belegte allgemeine
Vorgabe zu verschieben. Für b spricht, dass Profis schneller vibrieren — das wiegt weniger, weil 79 von 99 Instrumenten
ohne eigene Rate (Befund SK10 a) dann eine Streicher-Zahl erbten, die für sie nicht gemessen ist. **Kippt auf b**, wenn
Hörproben die 5,5 Hz an Synth-Schichten als zu langsam zeigen.

---

## F-6 — Orte der Streicher-Spielweisen und zwei Ergänzungen

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` §1 Zielbild und **SF8 a**. **Heutiger Wortlaut:** Streicher: „Pizzicato, *con sordino*,
Tremolo, Spiccato, Col legno, Vibrato“; SF8 a: Pizzicato, Tremolo, Spiccato, Col legno je Note, Dämpfer über eine
Passage.

**Problem:** Die Recherche bestätigt die Orte und findet zwei Spielweisen, die das Zielbild nicht nennt, die aber aus
dem Modell von selbst kommen: *sul ponticello* / *sul tasto* (Kontaktstelle näher am Steg oder am Griffbrett — mehr bzw.
weniger Obertöne, nur Bericht; normales Spiel β ≈ 1/30 … 1/6, nur Bericht) und **Legato/Portamento** (wie Stufe 25 an der
Gitarre). Zu Tremolo, Col legno und Sordino fehlen Zahlen; die Zeit zum Aufsetzen des Dämpfers ist nirgends belegt.

**a (Empfehlung):** „Die Orte aus SF8 a gelten. Dazu kommen *sul ponticello* und *sul tasto* als Spielweise über eine
Passage (verschiebt die Kontaktstelle des Expression-Wegs) und Legato/Portamento als verbundene Noten wie in Stufe 25
(eine Kette in einem Strich, Portamento über die Pitch-Spur). Tremolo und Spiccato werden als Bogenbewegung gerechnet
(Sprungrate „between 6 and 35 Hz“, Guettler 2002), Col legno als Kontakt mit Holz — beide als ungemessen gekennzeichnet,
bis eine freie Aufnahme vorliegt. Das Umstecken des Dämpfers braucht eine Pause; ihre Länge bleibt offen.“

**b:** „Nur das Zielbild, ohne ponticello/tasto und Legato.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass beide Ergänzungen keine neuen Mechanismen sind — die Kontaktstelle
ist Teil des Bogenwegs (F-3), die Kette ist in LumiViz gebaut — und ohne Legato eine Streicherlinie in Einzelstriche
zerfällt. Für b spricht der kleinere Umfang — das wiegt weniger, weil a nichts Neues baut, sondern Vorhandenes zuordnet.
**Kippt auf b** für ponticello/tasto, wenn F-3 b (eigene Spur Kontaktstelle) gewählt wird — dann ist die Passage
überflüssig.

---

## F-7 — Korpus der Violine: Stärken selbst bestimmen

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Saiten_Konzept.md` §3, **S-K7 a**. **Heutiger Wortlaut:** „**Abgeleitet zählt als belegt**, wenn Rechenweg oder
Abbildung mit Fundstelle dabeistehen; … *Violin*, *Upright bass*, *Acoustic bass* bleiben draußen, bis ihre Lücke belegt
ist.“

**Problem:** Auch die zweite Recherche fand die Stärken der Violin-Moden (A0, CBR, B1−, B1+) nirgends als Zahl, nur als
Kurven (Woodhouse 2014, Bissinger 2006, Malvermi 2021 — nur Bericht). Frei sind aber die **Stegmobilitäten dreier
Violinen** (CNSM, `cc-by-4.0`, nachgelesen) und ein **Verfahren**, daraus Frequenz, Güte und Stärke je Mode zu gewinnen
(Maestre, Scavone & Smith 2013, CC BY 3.0, nachgelesen). Für das Cello stehen Güte und Stärke der Wolfton-Mode als Zahl
in einer Tabelle (Zhang & Woodhouse, „all-rights-reserved“ am Repositorium — Zahlen sind Fakten).

**a (Empfehlung):** „Die Stärken der Violine werden aus den CNSM-Mobilitäten mit dem Verfahren von Maestre et al. 2013
selbst bestimmt (Modalfit B6); Rechenweg, Daten und Fundstelle stehen dabei, nach S-K7 a zählt das als belegt. Das Cello
nimmt die Werte von Zhang & Woodhouse als Fakten mit Fundstelle. Viola und Kontrabass bleiben draußen. Vor dem Laden von
CNSM (654 MB) legt Claude Datei, Größe und Zielordner neben LumiViz zur Freigabe vor.“

**b:** „Der Violin-Korpus bleibt draußen, bis eine Quelle die Stärken als Zahl nennt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass a genau den Weg geht, den S-K7 a zulässt (abgeleitet mit Rechenweg)
und die Werte aus Messungen an echten Violinen kommen, nicht aus Annahmen. Für b spricht die strengste Lesart von
S-K4 a — das wiegt weniger, weil zwei Recherchen gezeigt haben, dass die Zahl in der Literatur nur als Kurve vorkommt.
**Kippt auf b**, wenn die CNSM-Daten sich als unvollständig erweisen (etwa ohne Bezug in m/(N·s)).

---

## F-8 — Streicher-Ensemble: einzelne Stimmen oder Chorus

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 3 („Streicher-Ensemble“); LumiViz `strings` (heute Sägezahn, SF9 a).
**Heutiger Wortlaut:** neu.

**Problem:** Ein Satz klingt durch gestreute Stimmung, Einsätze, Vibrato und **verschiedene Korpusse** (Weinreich:
Vibrato mit stark richtungsabhängiger Abstrahlung ergibt ein „scintillating“ Feld, nur Bericht). Gemessen sind nur
Einsatzstreuungen von 30–50 ms im Trio (Rasch 1979, nur Bericht); die Stimmungsstreuung kennt man nur vom Chor.

**a (Empfehlung):** „Das Ensemble ist eine Zahl gerechneter Stimmen (Vorgabe 8, wählbar), je Stimme eine eigene Saite
und ein eigener Korpus (gemessene Instrumente oder oberhalb von etwa 1 kHz gestreute Moden), gestreute Stimmung,
Einsatzzeit und Vibrato (Rate, Phase). Die Streuwerte sind bis zu einer Messung als gesetzt gekennzeichnet.“

**b:** „Eine gerechnete Stimme mit Chorus-Effekt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass der Satzklang aus unabhängigen Instrumenten entsteht und ein Chorus
die je Stimme verschiedenen Korpusantworten nicht erzeugt. Für b spricht die Rechenzeit — das wiegt weniger, weil die
Töne vorab gerechnet werden und 8–16 modale Stimmen billig sind (Einschätzung im Bericht). **Kippt auf b**, wenn Hörproben
zwischen 8 Stimmen und einer Stimme mit Chorus keinen Unterschied zeigen.

---

## F-9 — Bezugsaufnahmen und Downloads

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a, Downloads freigegeben** — die zwei Aufsätze liegen unter `quellen/`; die Aufnahmen und Bogendaten (Iowa, TinySOL, Lampis 3,0 GB …) warten auf die Ablagefrage A-1 in [STAND.md](../../anbindung-ic/ic/STAND.md) (Patrik 2026-10-26: „3GB auf github sind schon etwas viel“).
**Geladen 2026-10-27 (IC Session 3):** Iowa MIS 2012, TinySOL (Streicher), TU-Note, VSCO 2 CE (Streicher), CNSM,
Lampis 13374477 — unter `…\cmake\samples\Klasse 3 - Bogen-Saite\`, Begleitdateien in `aufnahmen/`, `daten/`
([aufnahmen.md](aufnahmen.md) „Geladen“). Offen: ein Teil der Saitenantworten (17749111, ≥ 14 GB) — vorher fragen.

📁 **Ablage (Anweisung Patrik 2026-10-26, IC Session 2, A-1 a, für alle Klassen gleich):** Aufsätze kommen ins Repo nach `quellen/`; **Aufnahmen und Messdaten lokal** nach `…\cmake\samples\<Klasse n - Name>\<Datensatz>\` (auch für LumiViz erreichbar), im Repo nur `aufnahmen/<Datensatz>/` bzw. `daten/<Datensatz>/` mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`; jeder Datensatz in [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). „Neben LumiViz“ im Text unten ist damit überholt; welche Dateien geladen werden, entscheidet diese Frage.

**Ort:** [aufnahmen.md](aufnahmen.md), [quellen.md](quellen.md). **Heutiger Wortlaut:** neu.

**Problem:** Frei und gut sind Iowa MIS (Lizenz nachgelesen) und TinySOL (CC BY 4.0, nachgelesen); die Übergänge stehen
nur in TU-Note (CC BY-ND 4.0 — messen ja, bearbeitet weitergeben nein); die Bogendaten der mdw sind groß (Teil 1 der
Saitenantworten 44 GB, nachgelesen). Nicht frei sind OrchideaSOL, Good-Sounds und die MTG-Vibrato-Daten.

**a (Empfehlung):** „Bezug sind Iowa MIS 2012 und TinySOL; TU-Note, VSCO 2 CE und die mdw-Daten dienen nur der Messung,
mitgeliefert wird keine Aufnahme. Nicht freie Sammlungen werden nicht genutzt. Vor jedem Laden legt Claude Datei, Größe
und Zielordner neben LumiViz zur Freigabe vor; zuerst Lampis 13374477 (3,0 GB) und ein Teil der Saitenantworten, nicht
alle acht. Die zwei freien Aufsätze Russo et al. 2022 (CC BY 4.0) und Maestre et al. 2013 (CC BY 3.0) kommen nach
Freigabe unter `quellen/`.“

**b:** „Nur Iowa und TinySOL; keine Bogendaten — das Reibmodell wird nur an Aufnahmen gesetzt.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Reibung nur an Messungen mit bekanntem Bogen sauber gesetzt werden
kann (M2) — an einer Aufnahme ohne Bogenwerte ließe sich ein falsches Reibgesetz durch einen falschen Bogenweg
ausgleichen. Für b sprechen Speicher und Ladezeit — das wiegt weniger, weil a mit 3 GB und einem Teil beginnt. **Kippt
auf b**, wenn die mdw-Daten sich nicht ohne deren Werkzeuge lesen lassen.

---

## F-9-1 — Saitenantworten (Zenodo 17749111) laden oder zurückstellen

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): a.** — zurückgestellt.

**Ort:** [aufnahmen.md](aufnahmen.md) „Noch nicht geladen“. **Heutiger Wortlaut:** „die Saitenantworten (Zenodo
17749111, kleinster Teil 14 083 546 513 B) — vor dem Laden fragen (Platz auf C)“. Vorgelegt nur im Chat (IC Session 3,
2026-10-27), hier nachgetragen.

**a (Empfehlung):** „Die Saitenantworten bleiben zurückgestellt, bis Platz und Bedarf geklärt sind (Setzen der
Bogen-Saite lokal, nach den Grundlagen).“ **b:** „Den kleinsten Teil (14,1 GB) jetzt laden.“

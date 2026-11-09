# Offene Fragen — Gezupfte Saite (Deluxe) (Klasse 2)

> **Stand:** 2026-10-26, IC Session 2. Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md); Empfehlungen nach „sauber/richtig“.
> Stellen in LumiViz (nachgelesen 2026-10-26) unter `…\LumiViz\projects\apps\LumiViz\docs\audio\`:
> `Saiten_Konzept.md`, `Spielweisen_Konzept.md`. **Entscheide:** F-1 bis F-9 a, F-10 a abgewandelt (Patrik 2026-10-26). Belege: [modell-entwurf.md](modell-entwurf.md),
> [quellen.md](quellen.md), [aufnahmen.md](aufnahmen.md).

---

## F-1 — Umfang und Reihenfolge der Klasse

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md) Zeile 2 („Akustikgitarre (Stahl, Nylon) mit gerechnetem Korpus, Harfe, Cembalo
(Kiel), Koto, Laute/Oud“). **Heutiger Wortlaut:** alle gleichrangig, „Deluxe“.

**Problem:** Die Datenlage ist sehr ungleich. Gut: Korpus und Klang der Nylongitarre (Mores CC BY 4.0, Iowa), Cembalo
(VCSL CC0, Dulcken-Boden CC BY 4.0, Kiel CC BY 3.0), Harfe (VCSL CC0, Fingermessungen). Schwach: Stahlsaitengitarre
(kein Korpus frei), Laute/Oud (keine Aufnahme), Koto (keine Aufnahme, Spielweisen ungemessen). Nach der Regel „Pausieren
nur mit Freigabe“ wird ein Zurückstellen vorgelegt.

**a (Empfehlung):** „Reihenfolge: (1) zwei Polarisationen und 2×2-Admittanz am Steg für die vorhandene Waveguide-Saite;
(2) Korpus der Nylongitarre aus den Mores-Messungen; (3) Cembalo; (4) Harfe; (5) Laute/Oud auf der Gitarre aufbauend,
zunächst ohne eigene Bezugsaufnahme. **Zurückgestellt** bis zu einer freien Aufnahme: der gemessene Korpus der
Stahlsaitengitarre (bis dahin bleibt *Parlor*) und das Koto.“

**b:** „Alle Instrumente, mit Literaturwerten, wo Messungen fehlen.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass jede Größe eine Messgröße braucht (Lehre aus dem Flügel): a baut
zuerst, was sich an freien Messungen setzen lässt, und die Stufen (1) und (2) heben alle Gitarren, die es heute schon
gibt. Für b spricht, dass das Koto heute nur ein Sample ist und viel gewänne — das wiegt weniger, weil weder Saite noch
Körper noch Oshi-de-Verlauf belegt sind und ein geratenes Modell unechter wäre als das Sample. **Kippt auf b** für ein
Instrument, sobald eine eigene Aufnahme vorliegt (Forschungsauftrag in [messplan.md](messplan.md)).

---

## F-2 — Woher der Gitarrenkorpus „Deluxe“ seine Moden nimmt

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Saiten_Konzept.md` **S-K4 a** („Die Werte der Korpusse kommen aus veröffentlichten Messungen“), **S-K8 a**
(„Der Korpus ist das gekoppelte Modell der Quellen“); `Spielweisen_Konzept.md` **KB1 a** (gemeinsame Form: Moden mit
Masse am Einkoppelpunkt und Abstrahlgewicht, dazu Erweiterungen; Gitarre: „Luftraum, zwei Richtungen am Steg“);
gemeinsam **F-K1** (Luftraum als gemeinsame Erweiterung).

**Problem:** KB1 a legt die Form fest, nicht die Herkunft der Moden. Zwei Wege: aus einem **Bauplan** gerechnet
(`InstrumentBody` mit Luftraum nach F-K1) oder aus **Messungen** gefittet. Für die Nylongitarre gibt es freie Messungen
von über 60 Instrumenten (Mores, CC BY 4.0, nachgelesen); für ein Bauplanmodell bleibt nach Brauchler et al. 2023 selbst
nach Inversion von 35 Holzparametern ein Frequenzrest von im Mittel 1,55 % (nur Bericht).

**a (Empfehlung):** „Der Gitarrenkorpus Deluxe ist die Form nach KB1 a mit Moden **aus Messung**: Modalfit der
Stegmobilität (zwei Richtungen) und des Nahfelds aus dem Mores-Archiv, je gemessener Gitarre ein Korpus. Der Bauplan-Weg
(F-K1) dient als Prüfstein und für Gitarren ohne freie Messung; *Classical* und *Parlor* bleiben als Fälle derselben Form
erhalten (gleiche Spitzen wie heute).“

**b:** „Der Gitarrenkorpus wird aus einem Bauplan gerechnet (`InstrumentBody` mit Luftraum, F-K1); die Messungen dienen
nur zum Prüfen.“

**Warum die Empfehlung:** Den Ausschlag gibt S-K4 a: Korpuswerte kommen aus veröffentlichten Messungen — a nimmt sie
unmittelbar und bekommt dazu die echte Streuung zwischen Gitarren. Für b spricht, dass ein Bauplan Bauweisen erlaubt, für
die es keine Messung gibt, und dass die gemeinsame Grundlage einen Weg für alle Klassen bietet — das wiegt hier weniger,
weil b eine plausible, nicht eine gemessene Gitarre ergibt und a die Form von KB1 a ja einhält. **Kippt auf b**, wenn
der Modalfit an den Mores-Daten nicht passiv oder nicht eindeutig gelingt, oder für die Stahlsaitengitarre, solange keine
Messung frei ist.

---

## F-3 — Zwei Polarisationen und Mitschwingen: je Note oder je Clip

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Saiten_Konzept.md` §2 („Ein Ton wird je Note **vorab und für sich** gerechnet … kein Gedächtnis zwischen
Noten“), **S-K1 a** (Waveguide, „Korpus als Resonanzen hinter dem Steg“); `Spielweisen_Konzept.md` §4.2–§4.5 (Töne vorab),
**SZ3 a** (Setzen über eine Vorhersage je Teilton aus den Moden).

**Problem:** Schwebung und zweistufiges Abklingen entstehen schon an **einer** Saite mit zwei Polarisationen an einem
Steg mit 2×2-Admittanz — „beating and two-stage decay appears for those partials which are in that frequency range where
the elements of the admittance matrix are large“ (Bank & Karjalainen 2010, nachgelesen PDF-S. 5). Das passt in den Ton
je Note. Das **Mitschwingen** anderer Saiten (Gitarre, Harfe ohne Dämpfer, Cembalo-Register) koppelt dagegen Noten über
den Steg und passt nicht. Zudem gibt ein Waveguide mit Admittanzmatrix keine geschlossene Vorhersage je Teilton (SZ3 a).

**a (Empfehlung):** „Zwei Polarisationen und die 2×2-Admittanz gehören in den Ton je Note (Stufe 1). Das Mitschwingen
ist Deluxe und rechnet **je Clip**: alle Saiten des Instruments an einem gemeinsamen Steg, die nicht gespielten offen
(Harfe), über dem Dämpfer (Gitarre: leere Saiten; Cembalo: nach Register). Gesetzt wird der Korpus an der Stegmobilität
(M1), die Saite wie heute; für SZ3 a liefert die Admittanz die Pole, die Saite ihre Teiltöne.“

**b:** „Zwei Polarisationen je Note; kein Mitschwingen.“

**Warum die Empfehlung:** Den Ausschlag gibt die Physik: bei der Harfe ohne Dämpfer ist das Mitschwingen der Klang selbst
(Le Carrou et al. 2009: „Sympathetic string modes“, nur Bericht), und ein Ton je Note kann es nicht abbilden. Dieselbe
Kopplung über die Noten hat das Tonrad (Klasse 11 F-1) und der Flügel mit Pedal (HA4 a); ein Rechenweg je Clip wäre für
mehrere Klassen derselbe. Für b spricht, dass es ohne neuen Rechenweg geht — das wiegt weniger, weil b gerade die Harfe
zu einer Reihe einzelner Töne macht. **Kippt auf b**, wenn LumiViz keinen Rechenweg je Clip zulassen will; dann für die
Harfe wenigstens die Nachbarsaiten derselben Tonklasse je Note mitrechnen.

---

## F-4 — Cembalo: was die Notenstärke bewirkt

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` **SF10 a** (Zupfen: „Expression nur über einen Tonabnehmer“); Notenstärke an der Note
(*Note vol.*, SK4 a). **Heutiger Wortlaut:** neu für das Cembalo.

**Problem:** Die Quellen widersprechen sich teilweise. Paté et al. 2017: „The jack velocity does not have an influence on
any of the measured quantities.“ (Abstract, nachgelesen). Chadefaux et al. 2013 messen bei schnellem Tempo eine kürzere
Wechselwirkung (7,9 gegen 12 ms) und andere Anfangsbedingungen (nachgelesen S. 157–158). Penttinen 2006 misst zwischen pp
und ff an C4 Teiltonunterschiede „5 dB, 3 dB, and 0.5 dB“ und in den tiefen Korpusmoden „at least a general 10 dB“, aus
dem Stoß der Taste in den Boden; hörbar „about the size of 1 dB“ je Stufe (nachgelesen PDF-S. 2–4).

**a (Empfehlung):** „Die Notenstärke wirkt am Cembalo auf zwei Wegen: auf den **Stoß der Taste** in den Resonanzboden (der
Großteil, Penttinen) und schwach auf die Anfangsbedingung der Saite (Chadefaux). Die Saite selbst hängt vor allem an der
Form des Kiels (Ausstattung). Die Vorgaben werden an VCSL und Penttinen gesetzt; ohne Notenstärke klingt es wie mf.“

**b:** „Das Cembalo hat keine Anschlagdynamik; die Notenstärke wirkt nur als Pegel nach dem Instrument.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die gemessenen Unterschiede da sind, nur nicht dort, wo man sie sucht —
Paté misst an der Saite und findet nichts, Penttinen findet sie im Boden. a ordnet beides dem richtigen Teil zu. Für b
spricht die verbreitete Lehre, das Cembalo sei ohne Dynamik — das wiegt weniger, weil die Unterschiede gemessen und
hörbar sind (bis 2,41 dB, nachgelesen). **Kippt auf b**, wenn der Volltext von Paté et al. zeigt, dass auch der
Korpusanteil sich nicht ändert.

---

## F-5 — Cembalo: Register, Lautenzug und Manual

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` **SF8 a** (Register als Beispiel „über eine Passage“); `Saiten_Konzept.md` **S-M2 a**
(„Eine Note trägt optional eine Saite“).

**Problem:** Das Cembalo hat Chöre (8′ vorn, 8′ hinten, 4′, selten 16′) mit verschiedenem Zupfpunkt — „The 8′ registers
differ from each other in the plucking point“ (Välimäki et al., nachgelesen S. 935) —, einen Lautenzug und bei zwei
Manualen eine Koppel. Welche Register klingen, wechselt über Passagen; zwischen den Manualen wechselt der Spieler aber oft
**Note für Note** (pièces croisées, nur Bericht).

**a (Empfehlung):** „Die Chöre (Fußlage, Zupfpunkt, Material) und der Lautenzug sind Ausstattung. Welche Register an
sind und ob gekoppelt ist, steht je Manual auf einer Spur über die Passage. Das **Manual** ist eine optionale Eigenschaft
der Note wie die Saite (S-M2 a): leer = nach einer festen Regel (oberes Manual für die rechte Hand); gesetzt gilt es für
diese Note.“

**b:** „Jede Registrierung ist ein eigenes Instrument; Manual und Register wechseln mit dem Clip.“

**Warum die Empfehlung:** Den Ausschlag gibt SF8 a mit den Häufigkeiten: der Bau (nie) in der Ausstattung, die Register
(Passage) auf der Spur, das Manual (je Note) an der Note — und LumiViz hat für „welche Saite spielt diese Note“ schon den
Ort. Für b spricht, dass mitgelieferte Instrumente ohne Spur gleich klingen — das wiegt weniger, weil eine Vorgabe der
Spur dasselbe leistet und ein Clipwechsel pièces croisées unmöglich machte. **Kippt auf b**, wenn Hörproben zeigen, dass
Registerwechsel praktisch nur zwischen Stücken vorkommen.

---

## F-6 — Harfe: Pedale

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` **SF8 a**. **Heutiger Wortlaut:** neu.

**Problem:** Die sieben Pedale stimmen je eine Tonklasse über alle Oktaven in drei Stellungen (♭, ♮, ♯; Wikipedia, nur
Bericht). Die Tonhöhe einer Note steht in LumiViz schon fest; die Pedalstellung bestimmt aber, **welche Saiten** dabei
klingen und mitschwingen, ob ein Glissando welche Töne trifft, ob Bisbigliando (zwei enharmonische Saiten) möglich ist —
und ob eine Notenfolge überhaupt spielbar ist. Eine Wechselzeit wurde nicht gefunden.

**a (Empfehlung):** „Das Pedaldiagramm ist eine Spur über die Passage (sieben Stellungen je Umschaltstelle). Das Programm
schlägt es aus den Noten vor und schreibt es in die Spur; der Nutzer kann es ändern (enharmonische Wahl). Noten, die mit
dem Diagramm nicht spielbar sind, werden markiert wie nicht greifbare Noten an der Gitarre (S-U2 a, E). Glissando und
Bisbigliando lesen die Spur.“

**b:** „Kein Pedaldiagramm; jede Note klingt auf ihrer Saite, Glissandi spielen die Töne der Tonart.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Pedale den Zustand des Instruments über eine Passage bestimmen —
genau der Ort aus SF8 a — und das Mitschwingen (F-3) davon abhängt, welche Saiten wie gestimmt sind. Für b spricht, dass
Nutzer keine Pedale bedienen wollen — das wiegt weniger, weil a das Diagramm selbst vorschlägt. **Kippt auf b**, wenn
F-3 b gewählt wird (dann klingen nur die gespielten Saiten, und das Diagramm wirkt nur noch aufs Glissando).

---

## F-7 — Harfe und Koto: Sustain ohne Dämpfer

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Spielweisen_Konzept.md` **SF10 a** („Sustain über den Damper des Schwingers“). **Heutiger Wortlaut:** Zupfen ×
Saite ohne Damper hat kein Sustain (Tabelle SF6 a: „Saite, gezupft — Sustain nein“).

**Problem:** Harfe und Koto haben keine Dämpfer; die Saite klingt, bis die Hand sie abdämpft (Bericht Spielweisen). In
LumiViz endet ein Ton mit seiner Note. Harfenisten notieren „laissez vibrer“ und „étouffez“.

**a (Empfehlung):** „Bei Harfe und Koto bedeutet das Notenende ‚mit der Hand abgedämpft‘. Die Spur Sustain gibt es bei
ihnen als ‚laissez vibrer‘: bei 1 dämpft die Hand nicht, die Saite klingt aus. Sons étouffés sind eine Artikulation an der
Note (kurz abgedämpft, wie Palm mute).“

**b:** „Kein Sustain; die Note klingt immer ihre volle Länge und endet.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Hand hier die Rolle des Dämpfers spielt — a hält die Regel aus
SF10 a, nur mit der Hand als Dämpfer. Für b spricht, dass es nichts Neues braucht — das wiegt weniger, weil ohne
„laissez vibrer“ jede Harfennote bis zum Ausklingen notiert werden müsste. **Kippt auf b**, wenn LumiViz die Bedeutung
des Notenendes nicht je Bauart unterscheiden will.

---

## F-8 — Koto: Oshi-de und Hiki-iro als Bend

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Saiten_Konzept.md` **S-L2 a** („Ein Bend ist eine Kurve an der Note“), **S-L15 b** (bis zwei Ganztöne).

**Problem:** Beim Oshi-de wird die Saite links vom Steg gedrückt, der Zug steigt und der Ton geht um einen Halbton
(„弱押し“) oder Ganzton („強押し“) hinauf (Nishiguchi et al. 2021, nachgelesen). Das geschieht **vor** dem Anschlag
(Oshi-de), **nach** dem Anschlag (Ato-oshi, Tsuki-iro als kurzer Puls) oder nach unten (Hiki-iro, nur Bericht). Der Bend
der Gitarre beginnt am Anschlag und geht nach oben.

**a (Empfehlung):** „Der Bend an der Note wird erweitert: er darf vor dem Anschlag schon stehen (Startwert ungleich 0)
und nach unten gehen. Oshi-de, Ato-oshi, Tsuki-iro und Hiki-iro sind Vorlagen dieser Kurve. Erst nach F-1 (Koto
zurückgestellt) zu bauen.“

**b:** „Eigene Artikulationen ‚Oshi-de‘ usw. mit festen Verläufen, getrennt vom Bend.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass es physikalisch derselbe Vorgang ist wie der Bend: der Zug der Saite
ändert sich über die Zeit — eine Kurve an der Note bildet ihn ab, und der Pre-Bend nützt auch der Gitarre. Für b spricht
die Erkennbarkeit im Editor — das wiegt weniger, weil Vorlagen der Kurve sie ebenso benennen. **Kippt auf b**, wenn eine
Messung zeigt, dass das Drücken links vom Steg mehr ändert als die Tonhöhe (etwa ein Klopfen des Stegs).

---

## F-9 — Laute/Oud: Doppelchöre und Intonation ohne Bünde

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a.**

**Ort:** `Saiten_Konzept.md` **S-M1 a** (Saitensatz am Instrument), **S-L9 a** („glatt“ ohne Bünde). **Heutiger
Wortlaut:** neu.

**Problem:** Laute und Oud haben Doppelchöre (Oud „usually with 11 strings grouped in six courses“, nur Bericht); ihre
leichte Verstimmung gibt Schwebung und Doppelabklingen (Woodhouse 2025, nur Bericht). Die Oud ist bundlos und wird im
Maqam mit Tonhöhen zwischen den gleichstufigen gespielt — „at least 12 notes between my lowest e-flat and my highest
e-natural“ (Abu Shumays, nur Bericht).

**a (Empfehlung):** „Der Saitensatz erhält Chöre: je Chor eine oder zwei Saiten mit Verstimmung in Cent (Ausstattung).
Die Intonation der Oud ist ein Versatz in Cent an der Note (wie die Saite optional an der Note); eine Spur ‚Maqam‘ als
Vorgabe dafür wird erst nach einer Recherche zur Intonation vorgelegt.“

**b:** „Doppelchöre als zwei Saiten-Schichten; Intonation nur über die Pitch-Spur.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass ein Chor physikalisch zwei Saiten an einem Steg ist (F-3) — er gehört
in den Saitensatz, nicht in zwei Schichten, die nichts voneinander wissen. Die Intonation ist eine Eigenschaft der Note,
nicht eine Bewegung über die Zeit; die Pitch-Spur ist bei Zupfen dem Vibratohebel vorbehalten (SF10 a). Für b spricht,
dass es ohne Änderung am Datenformat geht — das wiegt weniger, weil b die Schwebung zwischen den Chorsaiten nicht über
den Steg koppeln kann. **Kippt auf b**, wenn LumiViz keinen Cent-Versatz an der Note zulassen will.

---

## F-10 — Bezugsaufnahmen, Messdaten und Downloads

🔁 **Ablage geändert durch A-1 a (Patrik 2026-10-26):** Aufnahmen und Messdaten liegen nicht mehr im Repo, sondern unter `…\cmake\samples\Klasse 2 - Gezupfte Saite\`; im Repo nur `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`.

✅ **Entscheid Patrik (2026-10-26, IC Session 2): a, abgewandelt** — geladen wird **in dieses Repo**, nicht neben LumiViz: „dieses repo ist nicht öffentlich und wird es nie sein, es ist nur temporär“; es wird danach in LumiViz integriert, dafür sind Notizen nötig, welche Dateien dort ignoriert werden müssen und wo Lizenzen zu nennen sind → [UEBERNAHME_LUMIVIZ.md](../../anbindung-ic/ic/UEBERNAHME_LUMIVIZ.md). Stand der Downloads: [aufnahmen.md](aufnahmen.md).

**Ort:** [aufnahmen.md](aufnahmen.md), [quellen.md](quellen.md). **Heutiger Wortlaut:** neu.

**Problem:** Frei sind: Mores-Archiv (CC BY 4.0; Rohdaten 32,7 MB, gesamt 1,10 GB mit Fotos), Iowa Nylongitarre
(„without restrictions“, keine Standardlizenz; Größe nicht gelesen), VCSL Harfe und Cembali (CC0; Harfen 77 + 83 MB, zwei der fünf
Cembali 94 + 126 MB nach dem Bericht, die übrigen ohne Größe), AG-PT-set Stahl (CC BY 4.0, 6,75 GB), Rabāb-Stegadmittanz (CC BY 4.0, klein). Aufsätze frei unter CC BY:
Chadefaux et al. SMAC 2013, Bader et al. 2025, Brauchler et al. 2023, Abdallah et al. 2026.

**a (Empfehlung):** „Laden nach Freigabe, außerhalb des Repos neben LumiViz mit README (Quelle, Lizenz, SHA-256): aus dem
Mores-Archiv nur `qualified_selected_impulses.mat` und die PDFs; Iowa Nylongitarre (Lizenz vorher schriftlich bestätigen
lassen); VCSL Harfe und Cembali. AG-PT-set erst, wenn der Stahl-Korpus ansteht (F-1). Die vier CC-BY-Aufsätze nach Freigabe
unter `quellen/`. Vor jedem Laden legt Claude Datei, Größe und Zielordner vor.“

**b:** „Nur Mores und VCSL; die Gitarre ohne Iowa, an IDMT-SMT gemessen wie bisher.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass Korpus und Klang derselben Gattung (Nylon) frei gemessen vorliegen und
erst beide zusammen den Prüfstein aus [messplan.md](messplan.md) erlauben. Für b spricht die unklare Form der
Iowa-Freigabe — das wiegt weniger, weil a die Bestätigung vor der Übernahme verlangt und IDMT-SMT (CC BY-NC-ND) für
LumiViz selbst gar nicht übernehmbar ist. **Kippt auf b**, wenn Iowa die Freigabe nicht schriftlich bestätigt.

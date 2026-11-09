# Offene Fragen — Klasse 0 (Gemeinsam)

> **Gebündelt 2026-10-29 (IC Session 3):** die offenen Fragen dieser Datei sind in [FRAGEN_KONSOLIDIERT.md](../../anbindung-ic/ic/FRAGEN_KONSOLIDIERT.md)
> nach Themen zusammengefasst — dort beantworten; die Antwort wird hierher übertragen.
>
> **Stand:** 2026-10-26, IC Session 1. Fragen an Patrik im Format aus [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md). Keine davon muss vor
> der nächsten Klasse entschieden sein; F-D1 betrifft Aufnahmen für mehrere Klassen und lohnt sich früh.
> Empfehlungen nach „sauber/richtig“, nicht nach Aufwand.

---

## F-R1 — Kern des Rohrmodells für die Klassen 7, 8, 9

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T5): a.**

**Ort:** [rohr.md](../../rohr/ic/rohr.md) §3. **Heutiger Wortlaut:** neu.

**Problem:** Ein Rohr lässt sich auf zwei Arten rechnen. Modal: die Bohrung wird vorab in Moden am Eingang übersetzt,
im Ton läuft nur der Anreger gegen diese Moden — billig (weit unter 1 % der Echtzeit, Einschätzung), passt zum Muster
des Flügels (Moden vorab, Nichtlineares daneben im Zeitbereich), kann aber kein Rohr, das sich während des Tons ändert
(Halbventil, Übergang zwischen Griffen), und keine nichtlineare Ausbreitung (Schmettern). Im Zeitbereich (FDTD/FEM):
kann beides, kostet bei einer Trompete mit Wandverlusten rund 86 % der Echtzeit (Harrison et al. 2015, nachgelesen) —
etwa das Zwanzigfache der Grenze des Flügels.

**a (Empfehlung):** „Der Kern ist modal: die Bohrung je Griff wird vorab in Moden der Eingangsimpedanz übersetzt
(Kurven über den Tonumfang), der Anreger (Lippe, Rohrblatt, Strahl) läuft je Note im Zeitbereich gegen sie, gekoppelt
über Druck und Fluss am Eingang. Halbventil, Übergänge mit schaltenden Tonlöchern und Schmettern kommen als Deluxe im
Zeitbereich (FDTD/FEM).“

**b:** „Der Kern rechnet das Rohr im Zeitbereich (FDTD/FEM mit Wandverlusten); alle Blechbläser und die Holzbläser mit
Übergängen sind Deluxe.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass a die Physik genau so trennt, wie sie getrennt ist — das Rohr ist
linear und lässt sich vorab vollständig beschreiben (Wandverluste, Konus, Schallstück, Tonlöcher stecken in Dämpfung und
Residuen), nur die Anregung ist nichtlinear; das ist dieselbe Trennung wie beim Flügel (Saite modal, Hammer im
Zeitbereich) und gibt Größen, die sich direkt an gemessenen Impedanzen prüfen lassen. Für b spricht, dass es mehr
Physik in einem Verfahren enthält (zeitveränderliches Rohr, Stoßwellen) — das wiegt hier weniger, weil es nur einen
Teil der Spielweisen betrifft und als Deluxe-Weg in a erhalten bleibt. **Kippt auf b**, wenn das Zielbild der Klasse 7
das Schmettern oder Halbventil-Effekte als Kern verlangt, oder wenn modales Umschalten zwischen Griffen hörbar falsch
klingt (Legato).

**Befund nach der Recherche der Klassen 7–9 (2026-10-26), zu den Kipppunkten:**
- **Legato/Griffwechsel:** Maestre, Scavone & Smith (DAFx-18) rechnen genau so — Moden je Griff, beim Wechsel „based on
  linear interpolation of input impedance and sound pressure radiation models“ (nachgelesen S. 1), „above 30 times
  faster than real-time“ mit im Mittel 32 Resonatoren je Griff (nachgelesen S. 7). Kippt nicht; Grenze: Registersprünge
  hängen von der Phase ab (Szwarcberg et al. 2025, nur Bericht).
- **Posaunenzug:** FDTD mit dynamischem Gitter (Willemsen et al., DAFx20in21) begrenzt die Änderung auf einen
  Gitterpunkt „every 20 samples“, der ganze Zug „in ca. 0.06 s“ (nachgelesen S. 157) — langsamer als die Schwingung;
  modal geht ein Raster von Zugstellungen mit interpolierten Moden (Einschätzung). Kippt nicht.
- **Schmettern:** bei Posaune und Trompete mit dem Crescendo stark — Spektralschwerpunkt vor der Stürze „from 1.5 to
  4.0“, im Mundstück nur „from 1.2 to 1.6“ (Myers et al. 2012, nachgelesen S. 681); entsteht also auf dem Weg durch das
  Rohr. Als Stufe am Rohrausgang rechenbar (Einschätzung) — kippt den Kern nicht, gehört aber in den Kern statt in
  Deluxe (Klasse 7, F-2).
- **Halbventil:** bleibt im Deluxe-Pfad (Bilbao DAFx-11, FDTD; kein modaler Weg gefunden, nur Bericht).

**Befund Z1 (Moden und Waveguides, 2026-10-26):** Maestre et al. 2018, auf das sich a stützt, ist selbst schon eine
Mischform — die Moden werden als Reflektanz gerechnet und über Wanderwellen an das Blatt gekoppelt, weil die rein modale
Kopplung mit impliziten Verfahren „may cause numerical dispersion and require high computational cost“ (nachgelesen
PDF-S. 1). Welche Teile zusätzlich als Waveguide laufen sollten: F-R6. Bericht:
`recherche/2026-10-26_moden_und_waveguides_rohr.md`.

---

## F-R2 — Expression bei dauernder Anregung: Blasdruck allein oder Ansatz dazu

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T3a): a.**

**Ort:** LumiViz `Spielweisen_Konzept.md` §1, SK4 a. **Heutiger Wortlaut:** „Expression 0 … 1 als Stärke; 1 heißt ‚wie
die Note gespielt ist‘ … ein Modell mit dauernder Anregung die Stärke der Anregung (Blasdruck, Bogendruck) samt
Klangfarbe“.

**Problem:** Bei allen drei Anregern hängen Pegel und Klangfarbe nicht am Blasdruck allein. Klarinette: „High P and
low F produce the highest sound levels and stronger higher harmonics“ (Lippenkraft F; Almeida et al. 2013, nur
Bericht); Flöte: mehr Druck hebt die Tonhöhe und führt ins nächste Register, Spieler gleichen mit Abstand und Abdecken
aus (Fletcher & Douglas 1980, nachgelesen; Terrien et al., nur Bericht); Blech: die Lippe wird je Register nachgeführt.
Ein Modell, das Expression nur als Druck liest, driftet in Tonhöhe und Register.

**a (Empfehlung):** „Expression bleibt eine Zahl 0 … 1 (SK4 a). Jedes Bläser-Modell bildet sie auf einen Weg durch
seine Spielparameter ab — Blasdruck **und** Ansatz (Lippenkraft beim Rohrblatt, Lippenspannung beim Blech, Abstand und
Abdecken bei der Flöte), so gewählt, dass die Tonhöhe der Note bleibt. Der Weg ist eine Größe des Instruments
(Kurve über den Tonumfang), gesetzt an Aufnahmen pp/mf/ff.“

**b:** „Expression ist der Blasdruck allein (wie SK4 a wörtlich); der Ansatz bleibt je Note fest.“

**Warum die Empfehlung:** Den Ausschlag gibt die Physik der Spieler — sie ändern Druck und Ansatz zugleich, sonst
stimmt der Ton nicht; a hält SK4 a („Stärke der Anregung … samt Klangfarbe“) und füllt „Stärke“ so, wie sie gespielt
wird, ohne eine neue Spur. Für b spricht die einfache, eindeutige Bedeutung — das wiegt weniger, weil b bei Flöte und
Blech hörbar falsche Tonhöhen und ungewollte Registersprünge erzeugt. **Kippt auf b**, wenn sich zeigt, dass ein fester
Ansatz je Note mit Druck allein die gemessenen pp/mf/ff-Spektren trifft (am Modell prüfbar).

---

## F-R3 — Die Vibrato-Spur bei Bläsern

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T3a): a.**

**Ort:** LumiViz `Spielweisen_Konzept.md` §3, Modell 28a; SF4 a. **Heutiger Wortlaut:** „**Kurve** (Expression 0 … 1,
Vibrato in Cent 0 … 100 wie `vibratoDepth` der Schicht …)“.

**Problem:** Vibrato ist in LumiViz eine Tiefe in Cent, also Tonhöhe. Bei der Flöte ist es überwiegend „an amplitude
modulation of the upper partials of the tone with little change in the fundamental“, „consistently about 5 Hz“, „a 10%
variation in blowing pressure“ (Fletcher, nur Bericht); bei der Oboe eine Druckmodulation um 6,5 kPa (Almeida 2016, nur
Bericht); beim Saxophon Kiefer- oder Luftvibrato (Gilbert et al. 2005, nur Bericht); beim Blech Lippe, bei der Posaune
auch der Zug. Die Tonhöhe schwankt dabei als **Folge**, nicht als Steuerung.

**a (Empfehlung):** „Die Vibrato-Spur bleibt in Cent (Tiefe der Tonhöhenschwankung, wie heute). Bei Bläsern setzt das
Modell sie um, indem es den Anreger moduliert (Flöte: Druck; Rohrblatt: Druck oder Kiefer; Blech: Lippe) und die
Modulationstiefe so wählt, dass die Tonhöhe um den angegebenen Wert schwankt; das Verhältnis von Amplitude zu Tonhöhe
ergibt sich aus dem Modell.“

**b:** „Bei Bläsern bedeutet die Vibrato-Spur die Tiefe der Druckmodulation in Prozent statt Cent.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass eine Spur in allen Instrumenten dasselbe heißen soll (SF4 a, SF5 d)
und das Modell die Physik liefert — a hält die Einheit und erzeugt die Amplitudenmodulation von selbst, wo sie
physikalisch entsteht. Für b spricht, dass Druckprozent der Ursache näher ist — das wiegt weniger, weil eine Spur mit
zwei Einheiten je nach Instrument im Editor nicht zu lesen wäre. **Kippt auf b**, wenn sich die Tonhöhenschwankung der
Flöte als zu klein erweist, um die Tiefe des Vibratos zu führen (bei „little change in the fundamental“ möglich).

---

## F-R4 — Übergänge zwischen Noten bei Bläsern (Legato)

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T2): a.**

**Ort:** LumiViz `Saiten_Konzept.md` §5r–§5t (Stufe 25: verbundene Noten an der Saite — Legato, Slide, Bend, Ketten);
`Spielweisen_Konzept.md` SK12 a. **Heutiger Wortlaut:** für Bläser neu.

**Problem:** Ein Bläser verbindet Noten ohne neuen Einsatz (Legato, Lippenbindung, Griffwechsel); der Übergang ist
klangwesentlich (Hruška et al.: Lippen „not fully closing“ für 0,2–0,25 s beim gebundenen Trompetenwechsel, nur
Bericht; Logie et al.: aufwärts anders als abwärts, nur Bericht). Ein Ton je Note, vorab gerechnet, kennt keinen
Übergang.

**a (Empfehlung):** „Bläser übernehmen die verbundenen Noten der Saite (Stufe 25) als Mechanismus: eine Kette
verbundener Noten ist **ein** vorab gerechneter Ton mit seiner Spur (SK12 a); im Modell wechseln die Moden von Griff zu
Griff durch Überblenden (Maestre et al.) und der Anreger läuft ohne neuen Einsatz durch.“

**b:** „Jede Note bleibt ein eigener Ton; ein Legato wird als kurzer Übergang zwischen zwei fertigen Tönen
überblendet.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass der Übergang Physik ist (das Rohr wechselt, die Lippe oder das
Rohrblatt schwingt weiter) und LumiViz den Mechanismus für Ketten schon hat — a setzt ihn ein, statt einen zweiten zu
bauen. Für b spricht, dass Töne je Note öfter wiederverwendet werden (Tonspeicher) — das wiegt weniger, weil ein
Überblenden fertiger Töne den Übergang gerade nicht enthält. **Kippt auf b**, wenn die Rechenzeit ganzer Ketten den
Tonspeicher spürbar belastet.

---

## F-R5 — Vokaltrakt als Baustein des Rohr-Kerns

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T5): a.**

**Ort:** [rohr.md](../../rohr/ic/rohr.md) §3; Klassen 7 (Didgeridoo) und 8 (Saxophon). **Heutiger Wortlaut:** neu.

**Problem:** Vor der Lippe bzw. dem Rohrblatt liegt der Vokaltrakt des Spielers; seine Impedanz liegt in Reihe zum
Rohr (Fletcher et al. 2006, F-II: Fluss U = (2/ρ)^½[p_L − (Z_V + Z_D)U]^½ S(t), nur Bericht; „two series-resonant LRC
circuits in parallel“, nachgelesen S. 1207). Beim Didgeridoo macht er die Formanten (Tarnopolsky et al. 2005/2006), beim
Saxophon Altissimo und Bend (Chen et al. 2011: „in normal playing … no consistent tuning“, nachgelesen), bei der
Klarinette den Bend (Chen et al. 2009, nur Bericht); bei der Trompete ist er klein (Chen et al. 2012, nur Bericht).

**a (Empfehlung):** „Der Rohr-Kern erhält einen Eingang ‚Vokaltrakt‘ als Impedanz in Reihe vor dem Anreger, im
Normalfall null (keine Wirkung). Gerechnet als zwei gedämpfte Resonanzen plus Glottis-Zweig (Fletcher et al. 2006).
Benutzt zuerst vom Didgeridoo, später für Saxophon-Altissimo und Bend.“

**b:** „Kein Vokaltrakt im Kern; das Didgeridoo bekommt ihn als eigene Sonderlösung, Saxophon und Klarinette nicht.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass es physikalisch dieselbe Stelle für alle Bläser ist (Impedanz vor
dem Ventil) und eine Aussage an einer Stelle stehen soll; als Null im Normalfall kostet er nichts und verändert keinen
Ton, bis ein Instrument ihn braucht. Für b spricht, dass er bei Trompete und Posaune kaum wirkt — das wiegt weniger,
weil ihn die beiden Instrumente mit der größten Wirkung (Didgeridoo, Saxophon) aus zwei Klassen brauchen. **Kippt auf
b**, wenn das Didgeridoo nicht gebaut wird (Klasse 7, F-6) und Saxophon-Bend nicht ins Zielbild kommt. *Folgefrage,
nicht hier:* wie der Vokaltrakt gesteuert wird (eine Kurve „Vokal“ fehlt in den Spuren von SF5 d) — Klasse 7, F-6.

---

## F-R6 — Waveguide-Stücke im modalen Rohr-Kern (Zusatzfrage Z1)

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T5): a.**

**Ort:** [rohr.md](../../rohr/ic/rohr.md) §3; F-R1 a; Klasse 7 F-2 a („Stufe am Rohrausgang … ihre Rückwirkung auf die Lippe wird
vernachlässigt“). **Heutiger Wortlaut:** „Der Kern ist modal: … Halbventil, Übergänge mit schaltenden Tonlöchern und
Schmettern kommen als Deluxe im Zeitbereich“ (F-R1 a), Schmettern seither als Ausgangsstufe vorgeschlagen.

**Problem:** Moden sind am stärksten beim Setzen — Pole und Residuen kommen direkt aus der gemessenen Impedanz, und aus
den Modenparametern lässt sich die Spielfrequenz je Register vorhersagen (Petiot et al. 2023, nur Bericht), wie es LumiViz
beim Flügel braucht (SZ3 a). Schwach sind sie bei allem, was sich im Ton ändert: das Rohr der Posaune wird während des
Tons länger, und das Schmettern wirkt in der Schleife — nach Msallam (zitiert bei Cooper & Abel 2010) verändert der
rückwärts laufende Anteil die Grundfrequenz „and increase the brassiness“ (nachgelesen PDF-S. 4, Textebene verschoben).
Eine Ausgangsstufe ohne Rückwirkung verfehlt das. Ein Waveguide ändert seine Länge „simply by varying the length of the
corresponding delay line“, das aber nur „for simplified bore profiles“ (Willemsen et al., nachgelesen PDF-S. 2).

**a (Empfehlung):** „Der Kern bleibt modal (Moden der Impedanz je Griff, gesetzt an Messungen); die Kopplung an den
Anreger läuft über Wanderwellen wie bei Maestre 2018 und ist Teil der Schnittstelle. Wo sich etwas im Ton ändert oder in
der Schleife wirkt, kommt ein Waveguide-Stück dazu: der zylindrische Zug der Posaune als veränderliche Verzögerung, das
Schmettern als druckabhängige Verzögerung im Zylinder innerhalb der Schleife (Cooper & Abel 2010), der Vokaltrakt als
Reflektanz vor dem Anreger (F-R5), der Strahl der Flöte als Verzögerung. Halbventil und halb gedeckte Tonlöcher bleiben
Deluxe (FDTD).“

**b:** „Rein modal wie F-R1 a: Zug über ein Raster von Zugstellungen, Schmettern als Stufe am Ausgang.“

**c:** „Das Rohr als Waveguide (Tonlöcher als Streuglieder, Scavone & Smith 1997), Moden nur für das Schallstück.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass a jeden Teil mit dem Verfahren rechnet, das seiner Physik entspricht:
das feste, verlustreiche Rohr mit Schallstück modal (kompakt, an Messungen setzbar), die zeitveränderliche Länge und
die Wirkung in der Schleife als Wanderwelle — und Maestre 2018 zeigt, dass beides an derselben Schnittstelle sitzt. Für
b spricht ein einziges Verfahren — das wiegt weniger, weil die Ausgangsstufe die gemessene Rückwirkung weglässt und das
Raster den Zug nur stufig abbildet. Für c spricht die natürliche Länge — das wiegt weniger, weil ein Waveguide keine
Vorhersage je Resonanz hat und bei Kegeln „difficult to tune“ ist (Maestre 2018, nur Bericht). **Kippt auf b**, wenn
die Ausgangsstufe die Iowa-ff-Spektren der Posaune ebenso trifft wie das Stück in der Schleife (am Modell prüfbar);
**kippt** für den Zug auf b, wenn das Raster hörbar gleich klingt.

---

## F-K1 — „Luftraum“ als gemeinsame Erweiterung des Körpers

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T4): a.**

**Ort:** [koerper.md](../../koerper/ic/koerper.md) §1–§3; LumiViz `Spielweisen_Konzept.md` §4.1 KB1 a und `Saiten_Konzept.md` S-K8 a,
S-K10 a, §5e–§5f. **Heutiger Wortlaut in LumiViz:** KB1 a — „Erweiterungen tragen, was ein Instrument besonders macht
(Flügel: viele Stegpunkte, Deckel; Gitarre: Luftraum, zwei Richtungen am Steg; Trommel: Luftlast der Membran)“;
S-K8 a — „Der Korpus ist das gekoppelte Modell der Quellen — Decke und Luft als gekoppelte Schwinger, getrieben von der
Kraft am Steg“ (Parlor nach Christensen/Vistisen, Classical nach Wright Gl. A.20–A.24 mit Moden aus Wrights Tab. 7.1).

**Problem:** Vier Klassen brauchen eingeschlossene Luft am schwingenden Teil: Gitarre, Harfe (2), Violinfamilie (3),
Kalimba (4), Trommeln (5). LumiViz hat die Kopplung Decke–Luft für die Gitarre schon gebaut und an Messwerten geprüft
(S-K8 a, S-K10 a), aber außerhalb der gemeinsamen Grundlage und mit Moden aus einer Tabelle statt aus einem Körper nach
KB1 a. Wird der Luftraum je Instrument neu gebaut, entsteht dieselbe Physik mehrfach (Hohlraum, Löcher, Kopplung an
die Struktur) mit verschiedenen Näherungen.

**a (Empfehlung):** „Klasse 0 erhält eine gemeinsame Erweiterung ‚Luftraum‘ nach dem Muster des Gitarrenkorpus in
LumiViz (Wright Gl. A.20–A.24, S-K10 a): die Moden eines Körpers nach KB1 a als Kolben, gekoppelt an die Luft des
Hohlraums (Helmholtz-Mode, bei Bedarf weitere Hohlraummoden), Löcher als zusätzliche Ausgänge der Abstrahlung. Der
Korpus *Classical* wird damit ein Fall der Erweiterung (gleiche Spitzen wie heute als Prüfstein). Gitarre, Harfe,
Violinfamilie, Kalimba und Trommeln nutzen sie; die äußere Luftlast einer Membran bleibt eine Erweiterung der Klasse 5.“

**b:** „Jede Klasse baut ihren Luftraum selbst, wenn sie an der Reihe ist; der Gitarrenkorpus in LumiViz bleibt, wie
er ist.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass es physikalisch dieselbe Erscheinung ist (Hohlraum mit Öffnung,
gekoppelt an eine schwingende Wand) und eine Aussage an einer Stelle stehen soll — und dass LumiViz die Form dafür
schon gebaut und an Messwerten geprüft hat (S-K8 a: gekoppelte Spitzen 116 und 219 Hz wie bei Hess); a verallgemeinert
diesen Bestand, statt ihn zu ersetzen. Die Gitarre trägt so die Prüfung für alle (zusätzlich die Messdaten von Mores). Für b spricht,
dass jede Klasse ihre Besonderheit sofort richtig treffen kann, ohne auf einen gemeinsamen Baustein zu warten — das
wiegt weniger, weil die Besonderheiten (Lage der Löcher, Form des Raums) Daten des Bauplans sind, nicht eigene
Rechnung. **Kippt auf b**, wenn sich in der Recherche einer Klasse zeigt, dass ihr Luftraum anders gerechnet werden muss
(etwa die Pauke nur mit verteilter Luftlast über alle Moden, nicht über Hohlraummoden).

---

## F-D1 — Lizenz der TU-Berlin-Datenbank (Richtcharakteristiken und reflexionsarme Aufnahmen)

✅ **Entscheid Patrik (2026-10-29, IC Session 3, über FRAGEN_KONSOLIDIERT T7): b, mit Vermerk** — „mit Bemerkung laden, repo bleibt privat“: die Aufnahmen werden nach Freigabe (Datei, Größe, Zielordner) unter `samples\` geladen und **nur zum Messen** genutzt; die Lizenzlage steht im Vermerk in `HERKUNFT.md` und `UEBERNAHME_LUMIVIZ.md`; nach LumiViz geht nichts davon, bis die Lizenz geklärt ist.

**Ort:** [koerper.md](../../koerper/ic/koerper.md) §4, [aufnahmen.md](aufnahmen.md). **Heutiger Wortlaut:** neu.

**Problem:** Die Datenbank „A Database of Anechoic Microphone Array Measurements of Musical Instruments“ (DOI
10.14279/depositonce-19858) enthält 3305 reflexionsarme Einzeltöne von 41 Instrumenten (Streicher, Gitarre, Harfe,
Pauke, Blech, Holz; 44,1 kHz, je Ton Note und Stärke) und ihre Richtcharakteristiken — für die Klassen 2, 3, 5, 7, 8, 9
womöglich die einzigen freien Einzeltöne ohne Raum. Zur Lizenz gibt es **drei Angaben derselben Autoren**: der
Datensatz trägt „In Copyright“ (`dc.rights.uri`, nachgelesen); die Dokumentation der Fassung 3 sagt „provided under a
Creative Commons BY-NC-SA licence … but not for commercial purposes“ mit Verweis auf CC BY-NC-SA 4.0 (nachgelesen,
`quellen/`, S. 2); das Paper arXiv 2307.02110 nennt CC BY-SA 4.0 (nur Bericht). Gilt die Dokumentation, dürfen die
Aufnahmen nur **nicht kommerziell** genutzt werden, und Abgeleitetes muss unter derselben Lizenz stehen. Ob LumiViz
kommerziell ist oder wird, und ob Messwerte und daraus gesetzte Kurven „Abgeleitetes“ sind, ist hier nicht zu
entscheiden.

**a (Empfehlung):** „Patrik fragt bei den Autoren (Stefan Weinzierl, TU Berlin, Fachgebiet Audiokommunikation; Adresse
in der Dokumentation S. 1) nach, welche Lizenz gilt, und schildert den Zweck (Messgrößen aus Einzeltönen, um ein
gerechnetes Instrument in LumiViz zu setzen). Bis zur Antwort gilt die strengste Angabe („In Copyright“): keine
Aufnahmen laden, nichts davon nach LumiViz; für die Planung reicht die Dokumentation.“

**b:** „Die Dokumentation gilt (CC BY-NC-SA 4.0): die Aufnahmen werden hier zum Messen genutzt; nach LumiViz gehen sie
nur, wenn LumiViz nicht kommerziell ist und Abgeleitetes unter CC BY-NC-SA 4.0 stehen darf.“

**c:** „Die Datenbank gilt als nicht frei; die Klassen suchen andere Aufnahmen.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass die Rechtslage am Ort das Maß ist und hier drei Angaben derselben
Autoren widersprechen — sauber ist, den Widerspruch an der Quelle aufzulösen, statt eine der Angaben auszuwählen; bis
dahin schützt die strengste Angabe. Für b spricht, dass die Dokumentation die ausführlichste und jüngste Angabe ist
(2. Februar 2024) und ausdrücklich an Nutzer gerichtet — das wiegt weniger, weil das Feld des Repositoriums dieselbe
Fassung anders auszeichnet und eine Übernahme nach LumiViz später nicht zurückzunehmen ist. Für c spricht, dass es ohne
jedes Risiko ist — das wiegt weniger, weil a bis zur Antwort dasselbe für LumiViz tut und die Recherche weiterlaufen
kann. **Kippt auf c**, wenn die Autoren nicht antworten oder bei „nicht kommerziell“ bleiben und LumiViz kommerziell
werden soll; **auf b**, wenn LumiViz sicher nicht kommerziell bleibt.

---

## Lehren aus dem Setzen des Flügels (IC Session 3, 2026-10-27)

> Grundlage: [LEHREN_SETZEN_FLUEGEL.md](../../anbindung-ic/ic/LEHREN_SETZEN_FLUEGEL.md) (Input LumiViz S157), eingearbeitet in
> [VORBILD_FLUEGEL.md](../../anbindung-ic/ic/VORBILD_FLUEGEL.md), [SCHNITTSTELLE.md](../../anbindung-ic/ic/SCHNITTSTELLE.md) §Messkette und
> [messkette.md](../../messung/ic/messkette.md) §3a/§4a; Prüfbefund aller Klassen: [lehren-pruefung.md](../../anbindung-ic/ic/lehren-pruefung.md).
> F-L1 und F-L2 sind die Vorschläge Z2 und Z3 aus LumiViz für `AUFGABE.md`.
> **Antwort Patrik (2026-10-28):** „prinzipiell alle a, es gibt aber news aus abklärungen in lumiViz“. Gedeutet:
> alle a, angepasst an den neuen Stand in LumiViz — **RF1 a** (S157, nachgelesen): Grundlagen bis und mit 31h zuerst,
> die Feinabstimmung (das Setzen an der Aufnahme) danach als eigene, letzte Stufe 31i; 31d-8 „Spielbar“ gebaut, das
> Hörurteil 8.1 offen. `LEHREN_SETZEN_FLUEGEL.md` selbst ist unverändert (Stand 2026-10-27 16:15).

## F-L1 — Zusatzfrage Z2 „Kurven als Suchraum“ in den Auftrag

✅ **Entscheid Patrik (2026-10-28, IC Session 3): a** — Z2 steht in [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md). Zum Kipppunkt: die Probe
von SP2 a (7.5.10) liegt nach RF1 a erst in 31i, also nach 31h; bis dahin bleibt die Kurvensuche in LumiViz ungeprüft —
die Tabelle aus Z2 gilt unabhängig davon.

**Ort:** [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md), Abschnitt „Zusatzfragen aus LumiViz“, nach Z1. **Heutiger Wortlaut:** neu
(Vorschlag aus LumiViz in `LEHREN_SETZEN_FLUEGEL.md` §3).

**Problem:** Beim Flügel suchte das Setzen je Taste 6–8 Parameter, fand Mehrdeutiges und interpolierte die Glätte
danach wieder hinein; seit SP2 a setzt LumiViz die **Stützpunkte der Kurven** über alle Tasten zugleich. Die Zahl der
Stützpunkte ist die Zahl der Parameter. In den Entwürfen hier steht die Art je Größe (Kurve/fest/Stärke) in 2–11, die
Zahl der Stützpunkte in **keiner** Klasse, und **kein** Messplan setzt über alle Noten; Klasse 7 sucht die
Lippenresonanz ausdrücklich je Note ([lehren-pruefung.md](../../anbindung-ic/ic/lehren-pruefung.md) §1, §3).

**a (Empfehlung):** „**Z2 — Kurven als Suchraum** (Vorschlag LumiViz S157, 2026-10-27). Für jede Klasse, auch die
abgeschlossenen: Welche Größen sind Kurven über den Tonumfang, welche fest, welche hängen an der Stärke; wie viele
Stützpunkte verlangt die Physik je Kurve (Wechsel von Steg, Saite, Register, Bohrung, Fell) und wo liegen sie; reicht
die Zahl der Noten im Bezugsdatensatz, um diese Stützpunkte zu bestimmen (Messgrößen gegen Parameter)? Wo eine Größe je
Note gesucht werden soll, steht der physikalische Grund dabei. Ergebnis als Tabelle in `modell-entwurf.md` (Größe |
Art | Stützpunkte: Zahl, Lage, Grund | Noten im Bezug | Messgrößen je Parameter), das Setzen in `messplan.md` als
Ausgleich über alle Noten zugleich; Lücken als Frage.“

**b:** „Z2 wird nicht in den Auftrag aufgenommen; Art und Stützpunkte je Größe werden erst bei der Ausarbeitung der
Klasse in LumiViz festgelegt.“

**Warum die Empfehlung:** Den Ausschlag gibt die Richtung der Abhängigkeit: Das Modell interpoliert Kurven, also
bestimmt die Physik der Klasse, wie viele Parameter gesetzt werden — das gehört in den Entwurf, nicht in den Bau. Ob
ein Datensatz genug Noten dafür hat, ist eine Frage der Aufnahmen, und die liegen hier. Für b spricht, dass SP2 a in
LumiViz noch nicht an einer Probe bestanden hat (7.5.10 steht aus). Das wiegt weniger, weil die Tabelle für jedes
Verfahren gilt: auch eine Suche je Note braucht die Zahl der Parameter gegen die der Messgrößen. **Kippt auf b**, wenn
die Probe in LumiViz zeigt, dass Kurven aus Stützpunkten die Aufnahme grundsätzlich nicht treffen (dann wäre das Modell
der Kurven selbst offen, nicht nur ihre Zahl).

## F-L2 — Zusatzfrage Z3 „Toleranz als Auflösung des Bezugs“ in den Auftrag

✅ **Entscheid Patrik (2026-10-28, IC Session 3): a** — Z3 mit der Länge der Töne steht in [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md).

**Ort:** [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md), Abschnitt „Zusatzfragen aus LumiViz“, nach Z2. **Heutiger Wortlaut:** neu
(Vorschlag aus LumiViz in `LEHREN_SETZEN_FLUEGEL.md` §3).

**Problem:** Alle Toleranzen in den Messplänen sind geschätzte Zahlen; LumiViz setzt sie seit ZF1 a/ZF4 a als
gemessene Streuung des Bezugs (Nachbarstufen, Kanäle). Keine `aufnahmen.md` sagt, ob ein Datensatz diese Streuung
hergibt. Dazu fehlt überall die **Länge** der Bezugstöne — ohne sie lässt sich nach L3 nicht sagen, bis zu welcher
Abklingzeit eine Aufnahme eine Größe bestimmt (beim Flügel: 355 gegen 637 s über 23 s Aufnahme, 1,7 dB).

**a (Empfehlung):** „**Z3 — Toleranz als Auflösung des Bezugs** (Vorschlag LumiViz S157, 2026-10-27, erweitert).
Für jeden Datensatz in `aufnahmen.md`: Hat er Nachbarstufen, mehrere Kanäle oder Wiederholungen derselben Note, aus
denen sich je Messgröße eine Streuung messen lässt? Wenn nein: welche zweite Quelle gäbe sie? Und wie lang sind seine
Töne (kürzeste und typische Dauer, an den Dateien gemessen)? Ergebnis als Spalten „Toleranz messbar aus“ und „Länge der
Töne“ in `aufnahmen.md`, in `messplan.md` je Messgröße die Toleranz als Rezept (welche Streuung, an welchen Noten) und
der gültige Bereich als Grenze aus der Länge (Abklingzeiten bis etwa zum Dreifachen der Länge).“

**b:** „**Z3 — Toleranz als Auflösung des Bezugs** wie von LumiViz vorgeschlagen: Für jeden Datensatz in
`aufnahmen.md`: Hat er Nachbarstufen, mehrere Kanäle oder Wiederholungen derselben Note, aus denen sich je Messgröße
eine Streuung messen lässt? Wenn nein: welche zweite Quelle gäbe sie? Ergebnis als Spalte „Toleranz messbar aus“ in
`aufnahmen.md` und als Rezept in `messplan.md`.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass L1 und L3 an denselben Dateien hängen: die Streuung sagt, wie genau
eine Messgröße getroffen sein muss, die Länge, ob die Aufnahme sie überhaupt bestimmt — eine Toleranz für eine Größe
außerhalb ihres gültigen Bereichs ist leer. Die Prüfung fand die Länge in keiner Klasse für die Hauptdatensätze. Für b
spricht, dass es der Wortlaut aus LumiViz ist und enger bleibt. Das wiegt weniger, weil die Länge im selben Durchgang
an denselben Dateien gemessen wird. **Kippt auf b**, wenn die Länge besser je Messgröße im Messplan steht als je
Datensatz (etwa weil nur ein Ausschnitt des Tons gemessen wird).

## F-L3 — Lieferungen 3–5 in `AUFGABE.md` um L3, L4, L5, L7 ergänzen und die Klassen nachziehen

✅ **Entscheid Patrik (2026-10-28, IC Session 3): a** — Lieferungen 3, 4, 5 und „Fertig ist eine Klasse“ in
[AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md) ergänzt; nach RF1 a steht in Punkt 5 dazu, dass das Setzen Feinabstimmung **nach** dem
spielbaren, fertigen Instrument ist. **Der Durchgang über die Klassen 0–11 steht aus** — er ist der nächste Schritt,
vor neuen Antworten.

**Ort:** [AUFGABE.md](../../anbindung-ic/ic/AUFGABE.md), „Lieferungen je Klasse“ Punkte 4 und 5, „Fertig ist eine Klasse, wenn“.
**Heutiger Wortlaut:** Punkt 4 „… Abschätzung der Rechenzeit je Ton; ob vorab rechenbar oder je Note mit ihrer Spur
…“; Punkt 5 „… gültiger Bereich je Messgröße, vorgeschlagene Toleranz; je Modellgröße die Messgröße, die sie
verursacht (Lehre aus dem Flügel)“.

**Problem:** Was Z2 und Z3 nicht abdecken, fehlt in den Entwürfen ebenfalls überall: eine Liste der Schwellen, an
denen das Modell seine Struktur wechselt, mit Entscheid „gesucht oder gesetzt“ (L4; Klassen 9 und 10 erkennen am
Modell selbst); Hörproben als eigene Zeile des Messplans (L5); Rechenzeit je Auswertung beim Setzen und Dauer eines
Setz-Laufs (L7). Der Auftrag kennt diese Punkte nicht; ohne ihn bleiben sie bei jeder neuen Klasse wieder aus.

**a (Empfehlung):** „Punkt 4 ergänzt: *Wo das Modell seine Struktur an Schwellen wechselt (Register, Anblasen,
Anstreichen gegen Rutschen, Kopplung, zweite Stufe), als Liste mit Entscheid ‚gesucht‘ oder ‚gesetzt‘ (aus Quelle oder
nach Gehör); dort wird das Modell an der Struktur des Bezugs gemessen, nicht mit eigener Erkennung (`gemeinsam/
messkette.md` §3a). Rechenzeit je Auswertung beim Setzen neben der je Ton beim Spielen.* Punkt 5 ergänzt: *Hörproben
als eigene Zeile (welche Noten, Stärken, gegen welchen Bezug, Frage je Datei); Zahl der Auswertungen je Suche und
Dauer eines Setz-Laufs, mit Ergebnisdatei je Note bzw. Iteration.* ‚Fertig ist eine Klasse‘ ergänzt um: *die Punkte aus
Z2, Z3 und diese Ergänzungen sind da.* Alle Klassen 0–11 werden in **einem** Durchgang nachgezogen, zusammen mit Z2
und Z3, bevor neue Antworten eingearbeitet werden.“

**b:** „Punkte 4 und 5 werden wie in a ergänzt, gelten aber nur für künftige Arbeit; die vorhandenen Klassen werden
erst nachgezogen, wenn sie in LumiViz ausgearbeitet werden.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass eine Vorab-Recherche den Bau ohne Überraschung vorbereiten soll — und
genau diese Punkte haben beim Flügel Sitzungen gekostet. Ein Durchgang über alle Klassen zugleich hält sie
gleichförmig; der Prüfbefund ([lehren-pruefung.md](../../anbindung-ic/ic/lehren-pruefung.md)) zeigt je Klasse, wo anzusetzen ist. Für b
spricht, dass die Entwürfe ohnehin bei der Ausarbeitung neu gelesen werden. Das wiegt weniger, weil dann jede Klasse
die Lehren einzeln neu entdeckt und die Reihenfolge der Klassen (nach Aufwand und Risiko) auf unvollständigen Zahlen
beruht. **Kippt auf b**, wenn die erste Klasse nach dem Flügel schon feststeht und bald in LumiViz beginnt — dann zuerst
nur diese nachziehen.

## F-L4 — Aufwand in `KLASSEN.md` in „spielbar“ und „gesetzt“ teilen (L6)

✅ **Entscheid Patrik (2026-10-28, IC Session 3): a, nach RF1 a angepasst** — die zwei Zahlen heißen *Grundlagen bis
spielbar und fertig* (Bau, Quellenwerte, Spielweisen, Editor, Abschluss) und *Feinabstimmung an der Aufnahme* (statt
„spielbar mit Quellenwerten“ / „an Aufnahme gesetzt“), weil LumiViz die Feinabstimmung hinter den Abschluss gelegt hat.
Vermerk in [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md); die Zahlen kommen im Durchgang nach F-L3 a.

**Ort:** [KLASSEN.md](../../anbindung-ic/ic/KLASSEN.md), Spalte „Aufwand (Sitzungen)“. **Heutiger Wortlaut:** je Klasse eine Spanne,
etwa „**nach IC1: 9–15**“ für Klasse 1.

**Problem:** Beim Flügel war das Setzen an der Aufnahme der Teil, der um Sitzungen zu klein geschätzt war, und das
Instrument war nach fünf Sitzungen Messen nicht anspielbar (L6; LumiViz zog 31d-8 „Spielbar“ vor, SP1 a). Die
Schätzungen hier trennen beides nicht; damit lässt sich nicht sagen, wann eine Klasse brauchbar klingt.

**a (Empfehlung):** „Die Spalte ‚Aufwand‘ bekommt je Klasse zwei Zahlen: *spielbar mit Quellenwerten* (Bau, Vorgaben aus
Quellen, Werte ohne Quelle nach Gehör innerhalb belegter Grenzen, mitgeliefert) und *an Aufnahme gesetzt* (Messkette,
Toleranzen, Setz-Läufe, Hörproben); die alte Schätzung bleibt daneben stehen. Geschätzt im Durchgang von F-L3.“

**b:** „`KLASSEN.md` bleibt wie es ist; die Teilung kommt mit der Ausarbeitung der jeweiligen Klasse.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass „spielbar“ der Stand ist, an dem Klangfragen entschieden werden, und
die Reihenfolge der Klassen davon abhängt, wie früh eine Klasse dorthin kommt; die zweite Zahl macht sichtbar, wo der
Flügel sich verschätzt hat. Für b spricht, dass beide Zahlen Schätzungen bleiben. Das wiegt weniger, weil die Teilung
die Unsicherheit dorthin legt, wo sie liegt (das Setzen), statt sie in einer Spanne zu verstecken. **Kippt auf b**, wenn
die Reihenfolge der Klassen nicht nach Aufwand entschieden wird.

## F-L5 — Iowa-Streicher: geben die Stereofassungen eine Kanalstreuung her?

✅ **Entscheid Patrik (2026-10-28, IC Session 3): a — geprüft.** Vier Stereo-Einzeldateien in den Scratchpad geladen
(zusammen 33,8 MB, nicht unter `samples\`): `Violin.arco.mf.sulG.G3B3`, `Viola.arco.sulA.pp.A4B4`,
`Cello.arco.ff.sulG.G3D4`, `Bass.arco.sulC.mf.C1Eb1` (`.stereo.aif`). Vergleich der Kanäle im klingenden Teil (Skript
`kanaele.py` im Scratchpad):

| Datei | Format | L−R gesamt | Korrelation | Versatz | Terzbänder 100 Hz–16 kHz: Median / größter Unterschied |
|---|---|---|---|---|---|
| Violine | 16 bit, 44,1 kHz | +1,29 dB | 0,870 | 1 Probe | 1,7 / 8,9 dB |
| Viola | 24 bit, 44,1 kHz | +1,54 dB | 0,407 | 34 Proben (0,8 ms) | 1,9 / 9,8 dB |
| Cello | 16 bit, 44,1 kHz | +0,80 dB | 0,911 | 3 Proben | 1,0 / 3,5 dB |
| Kontrabass | 16 bit, 44,1 kHz | +0,25 dB | 0,985 | 4 Proben | 0,9 / 2,9 dB |

**Befund:** die Kanäle sind nirgends gleich und unterscheiden sich je Band um Dezibel, mit Laufzeitversatz — zwei
Mikrofonorte, keine Mischung (Einschätzung aus der Messung; die Seiten nennen nur das Modell „Earthworks QTC40“). Die
Stereofassungen geben also eine Kanalstreuung her, in derselben Größenordnung wie bei *Salamander* (1,5–7,0 dB). Nach a
legt Claude die Stereo-Zips vor → F-L5-1.

## F-L5-1 — Iowa-Streicher stereo 2496 laden, und was mit mono geschieht

✅ **Entscheid Patrik (2026-10-28, IC Session 3): a** („a: laden, mono prüfen“) — **erledigt.** Die acht Zips geladen
(5 474 224 821 B, SHA-256 in der `LADELISTE.tsv`), ausgepackt nach `Iowa_MIS_Strings2012_stereo2496\` (264 AIFF,
8 086 826 202 B, alle 2 Kanäle, 24 bit; 96 kHz, 11 Violin-Dateien 48 kHz wie in mono), Archive gelöscht. **Vergleich
an allen 264 Paaren** (Skript `mono_vs_stereo.py` im Scratchpad; gepaart über den Namen ohne `.mono`/`.stereo`, Versatz
per Kreuzkorrelation ±64 Proben, Pegel angeglichen): mono ist überall **das Mittel (L+R)/2** — Viola, Cello,
Kontrabass in 24 bit mit Rest −98 bis −115 dB, Versatz 0, Pegel 1,000; Violine dasselbe Mittel **auf 16 bit
abgeschnitten** (Abweichung höchstens 1 LSB, gegen das Mittel in 24 bit an allen 72 Dateien nachgerechnet). Damit ist
mono ableitbar; der mono-Ordner unter `samples` ist gelöscht, die Begleitdateien bleiben mit Vermerk.
**Nebenbefund:** 17 Pizzicato-Dateien der Violine sind auch in stereo **doppelt mono** (L = R bitgleich, Inhalt 16 bit)
— dort gibt es keine Kanalstreuung (Liste in `aufnahmen/Iowa_MIS_Strings2012_stereo2496/HERKUNFT.md`).

**Ort:** `klassen/Klasse 3 - Bogen-Saite/aufnahmen.md` „Geladen“; Ablage `…\cmake\samples\Klasse 3 - Bogen-Saite\`.
**Heutiger Wortlaut:** geladen ist `Iowa_MIS_Strings2012_mono2496` (264 AIFF, 3 764 056 512 B).

**Problem:** Die Stereo-Zips der Streicher (Quelle wie die mono-Zips, `theremin.music.uiowa.edu/sound%20files/MIS/
Strings/<instrument>2012/`) haben zusammen **5 474 224 821 B**: Violin.arco 652 349 839, Violin.pizz 190 482 282,
Viola.arco 1 044 006 782, Viola.pizz 680 215 962, Cello.arco 952 811 077, Cello.pizz 404 985 856, Bass.arco 802 513 172,
Bass.pizz 746 859 851 (Content-Length, abgefragt 2026-10-28). Ausgepackt grob 7–8 GB (Einschätzung nach dem Verhältnis
bei mono, 2,47 → 3,76 GB). Auf C sind 26 GB frei. Ob die mono-Fassung genau einer der Kanäle oder die Summe ist, ist
ungeprüft; ist sie die Summe, lässt sie sich aus stereo nachbilden, ist sie ein Kanal, steckt sie ohnehin in stereo.

**a (Empfehlung):** „Die acht Stereo-Zips werden nach `…\samples\Klasse 3 - Bogen-Saite\Iowa_MIS_Strings2012_stereo2496\`
geladen (Archive einzeln auspacken und löschen, wie bei mono), mit `HERKUNFT.md`, `SHA256SUMS`, `LADELISTE.tsv`. Danach
prüft Claude an allen Dateien, ob mono gleich einem Kanal oder der Summe ist; trifft eins davon zu, wird der mono-Ordner
gelöscht (Begleitdateien im Repo bleiben als Verweis, mit Vermerk), sonst bleiben beide.“

**b:** „Die acht Stereo-Zips werden geladen wie in a; mono bleibt auf jeden Fall liegen.“

**c:** „Nicht laden; die Toleranz der Streicher kommt aus den Stufen von Iowa und den Kanälen von TU-Note und VSCO.“

**Warum die Empfehlung:** Den Ausschlag gibt, dass stereo für die vier Instrumente der einzige Bezug mit Kanälen ist
(F-L5) und mono dann eine Teilmenge oder ableitbar wäre — zwei Fassungen desselben Spiels nebeneinander sind Ballast
und führen zu verschiedenen Messungen desselben Tons. Für b spricht, dass nichts gelöscht wird, was einmal geprüft war;
das wiegt weniger, weil a nur löscht, was nachweislich in stereo steckt, und die Ladeliste es wiederbeschafft. Für c
spricht der Platz; das wiegt weniger, weil nach a rund 18 GB frei bleiben (nach dem Löschen von mono rund 22 GB).
**Kippt auf b**, wenn mono weder ein Kanal noch die Summe ist (eigene Mischung oder eigenes Mikrofon); **auf c**, wenn
vorher die Saitenantworten (≥ 14 GB) geladen werden sollen.

**Ort:** `klassen/Klasse 3 - Bogen-Saite/aufnahmen.md` „Geladen“ und `aufnahmen/Iowa_MIS_Strings2012_mono2496/
HERKUNFT.md`; betrifft auch Iowa in den Klassen 2, 4, 6–9. **Heutiger Wortlaut:** geladen sind die Fassungen „mono
2496“ (Freigabe F-9 a).

**Problem:** Nach L1 streuen bei *Salamander* die **Kanäle** stärker als die Stufen; eine Toleranz nur aus den Stufen
wäre zu eng. Die geladenen Iowa-Fassungen sind mono, geben also keine Kanalstreuung. Iowa bietet jede Datei auch
stereo an; die Seiten nennen je Instrument **ein** Mikrofonmodell („Earthworks QTC40“), ob die zwei Kanäle zwei Orte
sind, ist ungeprüft.

**a (Empfehlung):** „Zuerst je Instrument eine Stereo-Einzeldatei laden (Violine, Viola, Cello, Kontrabass; vier
Dateien, je einige MB, Größen werden vor dem Laden genannt) und die Kanäle vergleichen. Tragen sie verschiedene Orte,
legt Claude die Stereo-2496-Zips der Streicher mit Größe zur Freigabe vor; sind sie gleich, bleibt es bei mono.“

**b:** „Bei mono bleiben; die Kanalstreuung der Streicher kommt aus TU-Note (Violine, DPA/B&K) und VSCO (stereo).“

**Warum die Empfehlung:** Den Ausschlag gibt, dass Iowa der Bezug für vier Instrumente ist, TU-Note nur die Violine
und VSCO keine Einzeltöne von Viola und Cello hat — eine Toleranz für diese käme sonst nur aus den Stufen, also zu eng.
Die Prüfung kostet vier kleine Dateien. Für b spricht der Platz auf C (rund 26 GB frei) und dass zwei andere Sätze
Kanäle haben. Das wiegt weniger, weil a erst nach dem Vergleich groß lädt. **Kippt auf b**, wenn die Stereokanäle
gleich oder nur eine Mischung sind.

## Abgleich mit dem Bestand in LumiViz (IC-B1 a)

- **Ton je Note mit Spur:** kein Widerspruch. Ein Blaston (7–10) und ein Bogenton (3) hängen vom Verlauf der Steuerung
  ab; LumiViz hat dafür schon den Weg — SF10 a (Expression, Vibrato, Pitch bei dauernder Anregung) und SK12 a („Spur
  und Kurve stehen im Namen des Tons; ein Clip mit Kurve hat eigene Töne“), nachgelesen in `Spielweisen_Konzept.md`.
  Eine frühere Fassung dieser Datei sagte, der Tonspeicher schlüssele nur nach Taste, Dauer und Stufe — das stimmte
  nur für den Flügel (`modellierung/hammer-saite/modell.md` §1) und ist zurückgenommen.
- **Rohr für drei Anregungen:** F-R1 setzt SF10 a („das Rohr dient drei Anregungen“) um; kein Widerspruch.
- **Luftraum:** F-K1 baut auf dem gekoppelten Korpus von S-K8 a/S-K10 a auf (siehe dort).

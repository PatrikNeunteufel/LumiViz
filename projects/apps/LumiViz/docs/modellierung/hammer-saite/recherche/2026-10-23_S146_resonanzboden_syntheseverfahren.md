# Recherche S146: Resonanzboden — Verfahren der Klangsynthese und ihre Kosten

> **Datum:** 2026-10-23 (Session 146) · **Auftrag:** Mit welchen Verfahren wird ein physikalischer Körper/Boden in
> der Klangsynthese umgesetzt (modale Synthese, parallele Filter, Faltung, FDN, commuted synthesis), wie machen es
> Pianoteq, Roland V-Piano und Forschungssynthesizer, was kostet es.
> **Bearbeitet von:** einem Agenten (Claude); der Bericht steht unten **unverändert**. Kennzeichnung im Bericht:
> [wörtlich] = selbst im Volltext gelesen, [wörtlich lt. Werkzeug] = Zitat aus dem Abrufwerkzeug, [nur
> Zusammenfassung] = kein Wortlaut.
> **Von Claude selbst nachgelesen:** Bank 2010, Abschnitt VII (Faltung, „ten thousands of taps“, 633 Operationen) —
> beim Lesen von Bank 2010 in S146. Ege 2013 „0.06 modes Hz-1“. Alles andere: nur Bericht.
> **Quellen-PDFs:** `../quellen/`, `../../koerper/quellen/` (nicht versioniert); die im Bericht genannten Pfade ins
> Scratchpad gibt es nicht mehr.
> **Eingeflossen in:** `docs/audio/Spielweisen_Konzept.md` §4.1, „Befund Recherche Boden und Körper“.

---

**Bericht: Wie man einen Resonanzboden in der Klangsynthese rechnet, und was es kostet**

Zur Kennzeichnung: **[wörtlich]** heißt, ich habe die Stelle selbst mit `pdftotext` aus dem Volltext geholt und dort gelesen. **[wörtlich lt. Werkzeug]** heißt, das Zitat stammt aus dem Abrufwerkzeug, den Volltext habe ich nicht selbst gelesen. **[nur Zusammenfassung]** heißt, es gibt keinen Wortlaut als Beleg. Die Zitate sind gekürzt.

**1. Faltung, FIR-Filter, Commuted Synthesis, FDN – die Übersicht in Bank/Zambon/Fontana 2010**
Quelle: Bank, Zambon, Fontana, „A Modal-Based Real-Time Piano Synthesizer“, IEEE TASLP 18(4), 2010, PDF https://home.mit.bme.hu/~bank/publist/taslp10.pdf, Abschnitt VII.
- Commuted Synthesis **[wörtlich]**: „the soundboard response is implemented as a wavetable … However, the method assumes linearity and time-invariance, therefore, some important effects, such as the restrike of the same string or the nonlinear vibration of strings, cannot be precisely modeled.“
- FDN und Hall-Verfahren **[wörtlich]**: „feedback delay networks in [6], [7]. A difficulty … is that only the statistical distribution and the overall damping of the body modes can be set“.
- FIR-Länge und der „Knock“ **[wörtlich]**: „for a good tonality, 1000-2000 tap FIR filters are needed at fs = 44.1 kHz, and for reproducing the characteristic knock sound of the middle and high notes, ten thousands of taps are needed.“
- Kosten der FFT-Faltung **[wörtlich]**: pro Abtastwert „O(log N + M/N)“. Beispiel: „soundboard length of 20000 taps and block size N =128, the multiplication in the frequency domain requires 633 floating point operations at each time sample.“
- Parallelfilter **[wörtlich]**: Das Beispiel nutzt „100 logarithmically spaced poles with R = 0.98, giving a filter order of 200“. Im Vergleich: „the computational complexity seems to be larger compared to the partitioned convolution for the same sound quality“, dafür „zero latency“ und weniger Speicher. Polfrequenzen lassen sich verschieben, um „a bigger or smaller instrument body“ nachzubilden; die Abklingzeiten ändert man über die Polradien.
- Abhängigkeit vom Anschlagort am Steg **[wörtlich]**: Man rechnet „one high-order soundboard filter … and additional low-order shaping filters for the different regions of the bridge“.
- Einordnung **[wörtlich]**: „all the parts are real physical models except the soundboard, which is implemented as a digital filter and parameterized from measured transfer functions.“
- Kosten der Saite (Tabelle I) **[wörtlich]**: 3 Multiplikationen pro Abtastwert je Resonator mit Eingang, 2 ohne Eingang. Für den Boden nennt die Tabelle keine Zahl.

**2. Parallelfilter mit festen Polen statt Kautz-Filtern (Bank, DSc-Arbeit)**
Quelle: https://real-d.mtak.hu/1387/7/dc_1787_20_doktori_mu_.pdf
- Kosten je Sektion, Abschnitt 6.4.2 **[wörtlich]**: „For the parallel filter every second-order section requires 4 MAC operations … reduces the number of arithmetic operations to the half (from 8 to 4)“ im Vergleich zu Kautz. Auf Prozessoren ohne MAC-Befehl: „4 additions and 4 multiplications“ gegen „8 additions and 6 multiplications“.
- Beispiel Kap. 9 **[wörtlich]**: ein Resonanzboden mit „50 second-order sections“ und einem FIR-Teil 200. Ordnung, weil die Antwort „highly nonminimum-phase“ ist (Spitze bei n ≈ 200). In Festkomma ergab das 130 dB Dynamikproblem; die Arbeit löst es mit dem „delayed parallel filter“.
- Industrie **[wörtlich]**: „soundboard modeling in the Physis piano of the Viscount corporation“.

**3. Mehrere Stegregionen mit gemeinsamen Polen (Gabrielli/Zambon/Fontana, EUSIPCO 2015)**
Quelle: https://new.eurasip.org/Proceedings/Eusipco/Eusipco2015/papers/1570104271.pdf
- **[wörtlich]**: Vier Regionen teilen sich die Pole („common-pole description between the 4 regions“). Bei Puffer 64 ist das Verfahren „in the order of 50x“ schneller als „straightforward FFT-based convolution“.
- **[wörtlich]**: Das ganze Klavier hat „around 15,000 resonators … plus the soundboard modelling, requiring approximately 3 GFLOPS“. Der Boden läuft auf einem eigenen DSP.
- Weiterführend, nicht gelesen: Zambon, „Distributed piano soundboard modeling with common-pole parallel filters“, SMAC 2013.

**4. Sustain-Pedal mit Parallelfiltern (Zambon/Lehtonen/Bank, DAFx-08)**
Quelle: https://dafx.de/paper-archive/2008/papers/dafx08_37.pdf
- **[wörtlich]**: „A parallel bank of K = 1500 second-order sections“, entworfen mit FZ-ARMA (rund 2500 Pole vor der Auslese). Jeder Resonator braucht „4 multiply-and-add operations“. Bei „K is typically in the order of 1000“ ist das „remarkably higher“ als bei den Hall-Modellen; als Abhilfe wird Multirate vorgeschlagen.

**5. Modendichte des echten Resonanzbodens (Ege/Boutillon/Rébillat, JSV 2013)**
Quelle: https://arxiv.org/pdf/1212.2323
- **[wörtlich]**: Unterhalb von 1,1 kHz strebt die Dichte „towards a constant value of about 0.06 modes Hz-1“, der Boden verhält sich ungefähr wie eine homogene Platte. Darüber nimmt die an einem Punkt gemessene Dichte „decreases significantly“ ab, weil sich die Schwingung lokalisiert: Die Rippen wirken als Wellenleiter.
- **[wörtlich]**: Verlustfaktor „1% to 3%“, Modenüberlappung „30% at 150 Hz to 100% at 1 kHz“.

**6. Pianoteq (Modartt)**
Quelle: Patent US 7,915,515 B2, https://patents.google.com/patent/US7915515
- **[wörtlich lt. Werkzeug]**: Das Patent nennt „a method of finite elements … to model the sounding-board and the strings“, liefert komplexe Resonanzfrequenzen und „fundamental modes … of the coupled system“ sowie je Teilton eine Bodenimpedanz Z_np. Die Rechnung läuft vorab: „These calculations are effected beforehand and not in real time“ und „as a background task“. Die Laufzeitrechnung sei danach „very simple“.
- Wie es zusammenhängt, steht dort sinngemäß **[nur Zusammenfassung]**: Der Boden wird nicht als Filter gerechnet. Er geht über FEM-Moden in Frequenz und Dämpfung der Teiltöne ein, die dann additiv synthetisiert werden.
- Ein Artikel von 2018 (https://inria.hal.science/hal-01894219v1/document) soll das Verfahren beschreiben. Der Abruf scheiterte am Bot-Schutz; nicht geprüft.

**7. Roland V-Piano**
Quelle: https://articles.roland.com/?p=62190
- **[wörtlich lt. Werkzeug]**: einziger Satz zum Boden: „we made a supplemental virtual soundboard and combined it with the above component objects“. Einzelheiten fehlen; ein Patent habe ich nicht gefunden.

**8. FDN als Bodenmodell (Bank, Dissertation)**
Quelle: https://home.mit.bme.hu/~bank/thesis/thesabs.html
- **[wörtlich lt. Werkzeug]**: „The soundboard model consists of a feedback delay network with shaping filters.“ Kosten werden im Abstract nicht genannt.

**Nicht gefunden oder nicht geprüft:**
- Penttinen „soundboard as a filter“: Ich fand nur den Titel „New techniques to model reverberant instrument body responses“ (ICMC 2001), keinen Volltext.
- Rauhala/Välimäki: nichts zum Boden.
- Die FDN-Arbeit von Borin: nur als Verweis.
- Statistische oder zufällige Moden über einer Grenzfrequenz: keine Synthesequelle mit Zahlen. Physikalisch stützt sie nur der Befund von Ege bei 1,1 kHz.
- Rechenzeiten der vollen FEM-Zeitbereichsmodelle von Chabassier: nicht gefunden.
- Die Übersicht in EURASIP 2003: der Abruf landete hinter dem Login.
- Echtzeitanteile in Prozent für den Boden: gibt keine Quelle an.

**Empfehlung für eine offline gerechnete, physikalisch begründete Variante (eigene Einschätzung, nicht belegt)**
Weil offline gerechnet wird, fallen die Gründe weg, die oben zu Kompromissen zwangen: Latenz, Speicher und die Echtzeitlast für einen Ton.
1. **Moden unten physikalisch rechnen.** Den Boden einmalig als orthotrope Platte mit Rippen modal rechnen (FEM oder Rayleigh-Ritz, wie Pianoteq), etwa bis 1–1,5 kHz. Bei rund 0,06 Moden/Hz sind das nur ungefähr 60–100 Moden. Je Mode aus der Modenform die Stegbeweglichkeit am Anregungspunkt jeder Note nehmen, als Dämpfung η ≈ 2 % ansetzen.
2. **Moden oben statistisch ersetzen.** Darüber zufällig verteilte Moden mit Dichte und Dämpfung nach Plattenformel und Ege-Messung, abhängig von der Region (Rippen-Wellenleiter). Mit einem Zufallskeim je Instrument bleibt das deterministisch. Mehrere Tausend Moden sind offline billig.
3. **Pro Stegregion eine Impulsantwort.** Daraus Kraft→Druck-Antworten von 0,5–1 s Länge bauen und mit der Stegkraft per FFT falten, mit großen Blöcken. Offline kostet das nur Dutzende Flops pro Abtastwert, also wenig neben der Saite.
4. **Rückwirkung des Bodens in die Saite.** Die Impedanzwirkung wie bei Bank und Pianoteq über die Saitenrandbedingung führen, etwa mit Z je Teilton aus denselben Moden. Dann teilen sich Saite und Boden eine physikalische Quelle.

Zum Vergleich der Kosten: Eine Parallelfilterbank mit 1500 Sektionen braucht etwa 6000 MAC pro Abtastwert (4 MAC je Sektion, Bank). Die Faltung mit 20000 Taps braucht 633 Flops pro Abtastwert bei N = 128 und deutlich weniger mit großen Blöcken. Faltung ist also offline der günstigere Weg. Parallelfilter lohnen nur, wenn Parameter während des Tons wandern sollen (Größe, Abklingen).

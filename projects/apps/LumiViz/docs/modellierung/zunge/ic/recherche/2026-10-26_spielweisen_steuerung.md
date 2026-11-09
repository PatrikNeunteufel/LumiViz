# Recherche IC1: Durchschlagende Zunge — Spielweisen und Steuerung

> **Auftrag:** Klasse 10 (Akkordeon, Mundharmonika, Harmonium, Melodica): Spielweisen und Steuerung mit gemessenen
> Werten — Balg, Balgwechsel, Bellows shake, Register und Musette, Bass-Seite, Bending, Overblow, Harmonium-Register,
> Melodica, Ansprache; Ort je Spielweise im Schema von LumiViz.
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich, selbst gelesen, S. x] = vom Agenten per `pdftotext` gelesen; [wörtlich, Werkzeug]; [nur
> Zusammenfassung].
> **Reichweite:** im Bericht selbst (17 Suchen, rund 42 Abrufe; cs.bu.edu, ioa.org.uk nicht erreicht; patmissin.com
> wegen eines „No-AI statement“ nicht ausgelesen).
> **Anmerkung der Sitzung:** `pdftotext` hat Textauszüge neben die zwischengespeicherten PDFs geschrieben (im Cache der
> Sitzung, nicht im Projekt). Die Lieferungen der Klasse 10 schreibt die nächste Sitzung. **Von der Sitzung selbst
> nachgelesen (IC Session 2, 2026-10-26):** Hergert FA 2023 (Lizenz, VIOLIN/MUSETTE, „0.5 Hz and 15 Hz“, „gradual
> rise“ S. 4455; „0.5…7 Hz at A4“ S. 4457; 2,00 Hz = 7,9 ct S. 4460); Cottingham 2013 Folien („takes over“ PDF-S. 12,
> „decreases (slightly)“ PDF-S. 13, „Max Bend“ 2,5 Hz und 4,12 Hz PDF-S. 14); Adachi und Elejalde-García wie im
> Physik-Bericht. Grove, Wikipedia, Liberty Bellows, euphonics **nicht** selbst gelesen. Eingeflossen in
> `modell-entwurf.md`, `offene-fragen.md`.

---

# Bericht: Klasse 10 „Durchschlagende Zunge" (free reed) – Spielweisen und Steuerung mit Messwerten

**Auftrag:** Spielweisen und Steuerung von Akkordeon, Mundharmonika, Harmonium und Melodica mit gemessenen Werten, die ein physikalisches Modell als Vorgabe braucht.
**Datum:** 2026-10-26

**Reichweite**
- **Abrufe:** 17 Websuchen und etwa 42 WebFetch-Abrufe. Das liegt über dem Rahmen von 30–45, weil 5 Abrufe gescheitert sind. Dazu kamen lokale `pdftotext`-Auszüge aus den PDFs, die das Werkzeug zwischengespeichert hat. Eigene Dateien habe ich nicht angelegt; die `.txt`-Auszüge schrieb `pdftotext` neben die gespeicherten PDFs.
- **Orte:** Crossref-API (JASA, POMA, ISMA), EAA-Archiv (Forum Acusticum 2023 und 2026, Acoustics in Practice), DAFx 2023, ISMA 2017, Vienna Talk 2015, CCRMA (Kursfolien Cottingham), euphonics.org, Wikisource (Grove 1, EB 1911), Wikipedia (EN), Google Patents, TU Graz SPSC, Liberty Bellows (nur als Hinweis).
- **Sprachen:** Englisch, dazu Treffer auf Spanisch und Französisch.
- **Nicht erreichbar:**
  - cs.bu.edu (Zertifikat abgelehnt): Damit fehlen Bahnson/Antaki/Beery, JASA 1998, im Volltext sowie Cottinghams Berlin-Papiere.
  - ioa.org.uk: Fehler 404 für Millot/Cuesta/Valette, ISMA 1997.
  - patmissin.com: führt ein „No-AI statement" (Link auf no-ai-icon.com). Ich habe die Seite deshalb nicht weiter ausgelesen.
  - Bei ISMA 2017 und Vienna Talk 2015 enthalten die PDFs nur das Abstract.
  - Zu HAL und arXiv gab es keine einschlägigen Treffer.
- **Zwischengespeicherte PDFs** (alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):
  - `webfetch-1793045888421-p1m2mi.pdf` – Adachi u. a., FA 2026 (Auszug `b39obxah5.txt`)
  - `webfetch-1793045888494-lkbhih.pdf` – Elejalde-García u. a., AiP 2021 (Auszug `aip2021.txt`)
  - `webfetch-1793045962090-zax9ap.pdf` – Hergert, FA 2023 (Auszug `fa2023_celeste.txt`)
  - `webfetch-1793045974571-1s7moi.pdf` – Puranik/Scavone, FA 2023
  - `webfetch-1793046010460-m7kvvi.pdf` – Puranik/Scavone, DAFx23
  - `webfetch-1793046292399-a1kgp2.pdf` – Cottingham, CCRMA-Folien 2013
  - `webfetch-1793045914386-c77vu1.pdf` – Cottingham, ISMA 2017
  - `webfetch-1793045994826-cm70k7.pdf` – Cottingham/Wolff, Vienna Talk 2015
  - `webfetch-1793046003409-qrul41.pdf` – Granzow/Rossing/Egbert, Vienna Talk 2015
  - `webfetch-1793046062534-52ykip.pdf` – Partitur von Caine, „[squeezeBox]2"; unergiebig
- **Seitenangaben:** „S. x" ist die PDF-Seite nach dem Seitenzähler von `pdftotext`. Bei zweispaltigem Satz kann sie um eine Seite abweichen.
- **Kennzeichnung der Belege:**
  - [wörtlich, selbst gelesen, S. x] = von mir im pdftotext-Auszug gelesen.
  - [wörtlich, Werkzeug] = Zitat, das WebFetch als Wortlaut ausgegeben hat.
  - [nur Zusammenfassung] = kein Beleg.

---

## Quellen

**Q1 – Adachi, Okada, Samejima, Takeda, Nishimiya (2026)**
- Titel: „Aerodynamic mechanism of accordion pitch bending: simulation and theoretical analysis". 12th Convention of the EAA (Forum Acusticum), Graz, 8.–12. Sept. 2026.
- Fundort: dael.euracoustics.org/confs/fa2026/data/articles/000418.pdf. Frei zugänglich.
- Lizenz, S. 1 [wörtlich, selbst gelesen]: „Copyright: ©2026 Seiji Adachi This is an open-access article distributed under the terms of the Creative Commons Attribution 3.0 Unported License". Abgerufen am 2026-10-26.
- Vermerk der Tagung, S. 1: „The submitted manuscript was not reviewed".

**Q2 – Elejalde-García, Macho-Stadler, Llanos-Vázquez (2021)**
- Titel: „Accordion acoustics: A study on pitch bending". Acoustics in Practice, AiP-2021-02 (Februar 2021), EAA.
- Fundort: euracoustics.org/documents/14/AiP_2021_02_Accordion_acoustics.pdf. Frei lesbar, aber nicht frei lizenziert.
- Rechtsvermerk, S. 1 [wörtlich, selbst gelesen]: „All rights reserved. Use by third parties of the contents of this journal without the prior written consent of the copyright holder may constitute a criminal offence…". Abgerufen am 2026-10-26.

**Q3 – Hergert, F. (2023)**
- Titel: „Celeste ranks in pipe organs and accordions: tonal timbre and consonance of detuned unison intervals". 10th Convention of the EAA, Turin. DOI 10.61782/fa.2023.0326.
- Frei zugänglich. Lizenz, S. 1 [wörtlich, selbst gelesen]: „Creative Commons Attribution 3.0 Unported License". Abgerufen am 2026-10-26.

**Q4 – Puranik, N.; Scavone, G. (2023)**
- Titel: „Clamped bar model for free reeds". FA 2023, DOI 10.61782/fa.2023.0853.
- Frei zugänglich, CC BY 3.0 (S. 1, selbst gelesen). Abgerufen am 2026-10-26.

**Q5 – Puranik, N.; Scavone, G. (2023)**
- Titel: „Physically inspired signal model for harmonium sound synthesis". Proc. DAFx23, Kopenhagen.
- Frei zugänglich. Lizenz, S. 1 [wörtlich, selbst gelesen]: „Creative Commons Attribution 4.0 International License … for non-commercial purposes". Der Vermerk ist in sich widersprüchlich (CC BY 4.0 und zugleich „nicht kommerziell"); vor einer Übernahme klären.

**Q6 – Cottingham, J. P. (2013)**
- Titel: „Reed Vibration and Pitch Bending in Western Free Reed Instruments". Kursfolien, CCRMA Stanford, Music 318, 15. Feb. 2013.
- Frei zugänglich. Einen Rechtsvermerk habe ich nicht gefunden.
- Die Folien zeigen Daten von W. Coyle (2009).

**Q7 – Cottingham, J. (2017)**
- Titel: „Reed Chamber Resonances and Reed Tongue Vibrational Modes in Free Reed Excitation". Proc. ISMA 2017, Montreal. Frei zugänglich.
- Gelesen habe ich nur das Abstract. Einen Lizenzvermerk habe ich nicht gefunden.

**Q8 – Cottingham, J.; Wolff, D. (2015)**
- Titel: „Experimental study of free reed initial transients". Proc. 3rd Vienna Talk.
- Nur das Abstract ist frei. Einen Vermerk habe ich nicht gefunden.

**Q9 – Granzow, J.; Rossing, T.; Egbert, P. (2015)**
- Titel: „Bionic pitch benders for free reed instruments". Vienna Talk 2015.
- Nur das Abstract. Einen Vermerk habe ich nicht gefunden.

**Q10 – euphonics.org (J. Woodhouse), „11.6 Free reeds"**
- Lizenz [wörtlich, Werkzeug]: „Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License". Abgerufen am 2026-10-26.

**Q11 – Crossref-Metadaten** (nur Titel und Abstracts; ein Lizenzfeld fehlt durchweg)
- Cottingham 2012, „Pitch bending in the diatonic harmonica", JASA 132(3 Suppl.) 1990, doi 10.1121/1.4755348
- Bahnson/Antaki/Beery 1998, JASA 103(4) 2134–2144, doi 10.1121/1.421359
- Adachi u. a. 2026, „Time-domain simulation of harmonica pitch bending and overblowing", JASA 159(5) 4191–4203, doi 10.1121/10.0043784
- Biernat/Cottingham 2014, „Attack transients in free reed instruments", POMA 035002, doi 10.1121/1.4891627
- Coyle/Behrens/Cottingham 2009, JASA 126, 2216(A)
- Tonon 2009, JASA 126, 2217(A)
- Ricot/Caussé/Misdariis 2005, doi 10.1121/1.1852546
- Levy 2011, doi 10.1121/1.3654374
- Volltexte habe ich davon nicht gelesen.

**Q12 – Hipkins, A. J., „Harmonium"**
- In: Grove, A Dictionary of Music and Musicians, Bd. 1; über Wikisource.
- Gemeinfrei; Wikisource-Fußzeile CC BY-SA [Werkzeug]. Abgerufen am 2026-10-26.

**Q13 – Hipkins/Schlesinger, „Harmonium"**
- In: Encyclopædia Britannica 1911, Bd. 12; über Wikisource. Gemeinfrei.

**Q14 – Wikipedia (EN), Lizenz jeweils CC BY-SA 4.0** [Fußzeile, Werkzeug]
- „Accordion": zuletzt bearbeitet am 16.10.2026
- „Accordion reed ranks and switches": 08.08.2025
- „Stradella bass system": 11.10.2025
- „Chromatic harmonica": 16.07.2026
- „Melodica": 24.09.2026
- „Harmonica techniques": 15.10.2026
- Belegkraft gering, nur Einordnung.

**Q15 – Bahnson, H. T., US-Patent 5,739,446**
- Anmeldung 21.05.1992, Erteilung 14.04.1998; über Google Patents. Patenttext, keine Messung.

**Q16 – Liberty Bellows, „General accordion questions"**
- „Copyright © 2007-2026 - Liberty Bellows". Händlerseite, nur als Hinweis.

**Q17 – TU Graz SPSC, Projekt Zugin2 (Graber/Truppe, 2013)**
- Projektseite ohne Lizenzvermerk.

---

## 1. Akkordeon

### 1a. Balgdruck und Dynamik
- **Messwerte:** Einen Druckbereich in Pa von pp bis ff, an Spielern gemessen, habe ich **nicht gefunden**.
- **Nächstliegende Zahlen:**
  - Q1, Laborapparat mit C4-Zunge: Die Achse des Blasdrucks in Fig. 5 reicht von 0,0 bis 2,5 kPa (S. 4) [selbst gelesen, Achsbeschriftung]. Die Simulation rechnet mit „p0 = 1 kPa" (S. 4) [wörtlich, selbst gelesen].
  - Q6, Folie 26 (S. 13), Daten Coyle: G#-Zunge bei 0,05–0,85 kPa [selbst gelesen, Achse]; Mess- und Strömungsplots „@ 0.6 kPa" (S. 4–5) [wörtlich, selbst gelesen].
  - Q4, Harmonium: „in a real harmonium are in the range of 0.2 - 1.5 kPa" (S. 2) [wörtlich, selbst gelesen]. Das Harmonium ist dem Akkordeon am nächsten.
  - Q10, euphonics: Schwelle einer schließenden Zunge „predicted to be around 2 kPa" [wörtlich, Werkzeug]. Gemeint ist die Mundharmonika-Rechnung.
- **Wie Dynamik entsteht:**
  - Q14 Accordion: „The keyboard touch is not expressive" und „applied pressure increases the volume" [wörtlich, Werkzeug].
  - Q5: Der Balgdruck moduliert vor allem die Amplitude, „typically less than 1-2 Hz" Tonhöhenänderung (S. 3) [wörtlich, selbst gelesen]. Das gilt für das Harmonium.
- **Akzente:** Gemessene Akzent-Druckprofile habe ich nicht gefunden.
- **Vorschlag (Einschätzung):** **Balg = Spur Expression.** Das passt zur Definition in LumiViz („Stärke der Anregung samt Klangfarbe"), denn die Tastatur ist dynamisch neutral. Arbeitsbereich etwa 0,2–2 kPa, hergeleitet aus Q1, Q4 und Q6; das ist **keine Spielermessung**. Akzente als Hüllkurve auf Expression an der Note.

### 1b. Balgwechsel (Richtungswechsel)
- **Mechanismus:** Jede Note hat ein Zungenpaar auf einer Stimmplatte, eine Zunge für Zug, eine für Druck. Ventile verhindern, dass die jeweils andere Luft verliert. Q14: „Valves on opposing reeds of each note are used" [wörtlich, Werkzeug].
- **Bisonorisch und unisonorisch:**
  - Q14: „Piano accordions are unisonoric."
  - Diatonische Knopfakkordeons „tend to be bisonoric"; das Bandoneon ist bisonorisch [wörtlich, Werkzeug].
- **Klang der Richtung:** Q1, S. 1: Biegetiefe, Lautstärke und Klangfarbe hängen u. a. von der Balgrichtung ab („push or pull") [wörtlich, selbst gelesen]. Q2 hat nur beim Aufziehen gemessen: „always opening the bellows" (S. 2) [wörtlich, selbst gelesen].
- **Nicht gefunden:** Dauer der Pause beim Wechsel, Ventilklick, Zahl des Verstimmens zwischen Zug- und Druckzunge.
- **Vorschlag (Einschätzung):**
  - **Unisonorisches Akkordeon:** Balgwechsel als **Artikulation an der Note**, also als Umschaltstelle innerhalb einer Kette. Standard ist automatisch an Phrasengrenzen; der Spieler setzt ihn bewusst. Bei Wechsel: kurzer Druckeinbruch (Expression → 0 und zurück) und ein Wechsel auf die zweite Zunge mit leicht anderer Stimmung und Farbe.
  - **Bandoneon, diatonisches Akkordeon und Mundharmonika:** Die Richtung bestimmt die Tonhöhe. Hier ist sie **Eigenschaft der Note** und kein frei wählbares Merkmal.

### 1c. Bellows shake (Balgtremolo)
- Q14 nennt „bellows shake" als schnellen wiederholten Richtungswechsel [nur Zusammenfassung].
- Eine **Rate in Hz habe ich nicht gefunden**. Die Partitur von Caine notiert „b.s bellows shake" und „rhythmic bellows shake" ohne Tempo (S. 3) [wörtlich, selbst gelesen].
- **Vorschlag (Einschätzung):** **Artikulation an der Note** mit rhythmischer Rate, gekoppelt ans Tempo. Er ist keine Vibrato-Spur, weil er Amplitude und Zungenwechsel moduliert, nicht die Tonhöhe.

### 1d. Register, Chöre, Musette
- **Chöre:**
  - Q14 Reed ranks: 16′ = „lowest and deepest-sounding reed rank", 8′ = „basic middle reed rank", 4′ = „the highest reed rank" [wörtlich, Werkzeug].
  - Diskant mit „between two and four reed ranks", Bass mit „between three and five reed ranks" [wörtlich, Werkzeug].
  - Q16 (Hinweis): „L = Low = 16' organ pipe", „M = Middle = 8'…", „H = High = 4'…" [wörtlich, Werkzeug].
- **Musette und Violine:** Q3, S. 1 [wörtlich, selbst gelesen]: „The VIOLIN stop of the Accordion combines two reed ranks (true, sharp)" und „the MUSETTE stop consists of three ranks (tuned flat, true and sharp)".
- **Schwebungsraten:**
  - Q3, S. 1: „The beat frequency (BF) is usually set between 0.5 Hz and 15 Hz"; „a gradual rise upon pitch is considered desirable" [wörtlich, selbst gelesen].
  - Q3, S. 3: Traditionen „from <dry> to <wet> (referring to f = 0.5...7 Hz at A4)" [wörtlich, selbst gelesen].
  - Q3, Tabellen S. 6 [selbst gelesen]: Bei 2,00 Hz an A4 sind es 7,9 ct. Bei Typ d mit 3,5 Hz an A4 liegt C4 bei 2,47 Hz und 16,4 ct.
- **Händler-Hinweise (Q16, nur Hinweis):**
  - „0 hz = 0 cents = Unison (Dry, Secco)"; „1hz = 4 cents = Swing" [wörtlich, Werkzeug].
  - Deutsches Tremolo M = A440, M+ = A444.
  - Französische Musette M- = A435, M = 440, M+ = 445, also etwa ±20 ct an A4 (eigene Rechnung).
- **Obergrenze:** Q14: Gepaarte 8′-Chöre selten mehr als „25 cents" auseinander [wörtlich, Werkzeug].
- **Messgenauigkeit:** Q17: Stimmgerät mit „0.1 cents" Genauigkeit für 2–3 Zungen eines Tons [wörtlich, Werkzeug].
- **Registerwechsel im Spiel:** Q14 Reed ranks: Kinn- und Handballenschalter erlauben „faster changes to the register while the right hand is playing the melody" [wörtlich, Werkzeug].
- **Vorschlag (Einschätzung):**
  - **Musette-Verstimmung (Kurve in ct je Ton) = Ausstattung.** Sie ist am Instrument gestimmt und im Spiel nicht änderbar.
  - **Welche Chöre klingen (Register) = Passage-Spur mit Umschaltstellen.** Der Wechsel geschieht mitten im Spiel und betrifft, wie das Register auf die Tasten wirkt.
  - Damit ist „Musette" zweierlei: die Stimmung als Ausstattung, das Einschalten des Musette-Registers als Passage-Spur.

### 1e. Bass-Seite
- Q14 Stradella [wörtlich, Werkzeug]:
  - „each chord button sounds three individual pitches"
  - früher „four pitches for the seventh and diminished chords"
  - Akkordtypen M, m, 7, D
  - Töne „usually sounded simultaneously in multiple octaves"
  - bis zu „nine register switches" auf der Bass-Seite
- Free bass: nur Erwähnung („Various free-bass systems…") [wörtlich, Werkzeug].
- **Vorschlag (Einschätzung):** Bass-System = **Ausstattung**. Bass-Register = **Passage-Spur**. Ein Akkordknopf entspricht 3 vorab gerechneten Tönen über mehrere Oktaven, Oktavlage nach Register.

### 1f. Pitch-Bend am Akkordeon
- **Q1:**
  - „lowers the pitch by up to approximately a semitone" (S. 1) [wörtlich, selbst gelesen].
  - Gradient −2,5 Hz/kPa bei großer Ventilöffnung, −5,1 Hz/kPa bei Lp = 1 mm (S. 2) [wörtlich, selbst gelesen]. Bei etwa 260 Hz entspricht das etwa −17 bzw. −34 ct/kPa (eigene Rechnung).
- **Q2:**
  - „reduced by approximately 25 cents (bellows-and-finger glissando) and 15 cents (only-finger glissando)" (S. 1) [wörtlich, selbst gelesen].
  - „All notes have a frequency decrease of 10 to 35 cents" (S. 3) [wörtlich, selbst gelesen]. Gemessen an A#2–A3, 8′ ohne Cassotto.
- **Q6, Coyle-Daten:**
  - „Max Bend = 2.5 Hz (1/2 of a semitone)" bei teilweise geöffnetem Ventil (S. 14).
  - Mit kleinerem Kammervolumen „Max Bend = 4.12 Hz, 4.5%" (S. 14) [wörtlich, selbst gelesen].
- **Vorschlag (Einschätzung):** Spur **Pitch**, nur nach unten, 15–100 ct je Instrument, gekoppelt an Expression: Die Biegung braucht Druck bei halb gedrückter Taste.

### 1g. Tremolo durch den Balg
Nicht gefunden über den Bellows shake hinaus (1c).

---

## 2. Mundharmonika

- **Atemdruck:** Gemessene Spielerdrücke **nicht gefunden**. Q10 rechnet mit „a mouth pressure of 2 kPa" [wörtlich, Werkzeug].
- **Blas- und Ziehtöne:** In jeder Kanzelle sitzen eine Blas- und eine Ziehzunge. Die Richtung bestimmt den Ton (Q15, Q11 Bahnson-Abstract „Three different modes of speaking from each hole and its pair of reeds" [wörtlich, Werkzeug]).
- **Bending:**
  - Mechanismus nach Q6, S. 12: Ein Ziehbend von G nach etwa F# entsteht, wenn „the blow reed takes over as an opening reed" [wörtlich, selbst gelesen]; Abbildung nach Bahnson u. a. 1998.
  - Grenze nach Q10: „a limit that is about a semitone above the pitch of the lower reed" [wörtlich, Werkzeug].
  - Umfang je Kanzelle: Der Biegeumfang ist der Abstand der beiden Zungen minus 1 Halbton (eigene Folgerung aus Q10 und Richter-Stimmung, **kein Beleg**). Bei C-Richter: Ziehen 1: 1 HT, 2: 2 HT, 3: 3 HT, 4: 1 HT, 6: 1 HT; Blasen 8: 1 HT, 9: 1 HT, 10: 2 HT. Q14 Harmonica techniques belegt nur „bend 3 draw from a B down to a G♯" [wörtlich, Werkzeug].
  - Die Dauer eines Bends habe ich nicht gefunden.
- **Overblow und Overdraw:**
  - Q15: Overblows aus „the draw reed of the first, fourth, fifth, and sixth holes"; Ziehzunge „vibrate as an opening reed" [wörtlich, Werkzeug].
  - Q15: Overblow ist ein Sprung, „a sudden jump in pitch" (nach Websuche zitiert, [nur Zusammenfassung]).
  - Q11: Adachi u. a. 2026 simulieren „overblowing at hole 6" [wörtlich, Werkzeug].
- **Chromatische Mundharmonika:** Q14: „pressing the button accesses the same scale a semitone higher in each hole" [wörtlich, Werkzeug]. Wegen der Windklappen (windsavers) nur Einzelzungen-Bends von höchstens etwa einem Halbton [nur Zusammenfassung].
- **Zungenblockade:** Q14: „cover all the holes but one with your tongue" [wörtlich, Werkzeug]. Messwerte nicht gefunden.
- **Hand-Vibrato/-Wah, Kehlkopf- und Zwerchfell-Vibrato:** Raten und Tiefen **nicht gefunden**. Ein Forum nennt 3–8 Hz; das ist kein Beleg.
- **Vorschlag (Einschätzung):**
  - Blasen/Ziehen = Eigenschaft der Note.
  - Bend = Spur **Pitch**, begrenzt je Kanzelle.
  - Overblow = eigene Note, nicht Pitch.
  - Schieber = Eigenschaft der Note.
  - Zungenblockade (Oktaven, Slaps) = Artikulation an der Note.
  - Hand-Wah = eigene Filter-Modulation. Sie würde in Expression „Klangfarbe" passen, aber zeitlich unabhängig. Vibrato aus dem Hals = Spur **Vibrato** mit Amplitudenanteil.

---

## 3. Harmonium

- **Windversorgung:**
  - Q12: Tretschemel „moved by the feet of the player, feed the bellows" [wörtlich, Werkzeug].
  - Q13: Das amerikanische Harmonium (Saugwind) „acts by wind exhaustion" [wörtlich, Werkzeug].
- **Expression-Register:**
  - Q12: „the air reservoir is cut off and the pressure made to depend entirely upon the management of the bellows" [wörtlich, Werkzeug].
  - Q12 Double expression (Mustel): „pneumatic balance" [wörtlich, Werkzeug].
- **Prolongement:** Q12: „enables the player to prolong certain notes after the fingers have quitted the keys" [wörtlich, Werkzeug].
- **Percussion:** Q12: „a little hammer strikes the tongue" [wörtlich, Werkzeug]. Der Hammer verbessert die träge Ansprache.
- **Register:** Q12 Diapason 8 ft, Bourdon 16 ft, Clarion 4 ft, Bassoon/Oboe 8 ft [wörtlich, Werkzeug].
- **Druck:** 0,2–1,5 kPa am Handharmonium (Q4, S. 2). Druckabhängigkeit „typically less than 1-2 Hz" (Q5, S. 3) [beide wörtlich, selbst gelesen].
- **Schweller-Klappe als Mechanik:** gemessene Werte nicht gefunden.
- **Vorschlag (Einschätzung):**
  - Expression-Register an = Pedaldruck geht direkt auf die Spur **Expression**. Ohne Expression ist der Druck konstant, die Dynamik kommt dann vom Schweller (ebenfalls Expression, ohne Tonhöhenwirkung).
  - Prolongement = Spur **Sostenuto**. Das passt genau.
  - Percussion = Ausstattung bzw. Register; sie verkürzt die Ansprache.
  - Register = Passage-Spur.

---

## 4. Melodica

- Q14 Melodica [wörtlich, Werkzeug]:
  - Mundstück starr oder „a long flexible plastic tube"
  - „The keyboard usually covers two or three octaves"
  - Taste „opens a valve blocking its corresponding reed"
  - „Players can control the instrument's volume with air pressure"
- Gemessene Werte zu Atemdruck und Bending **nicht gefunden**. Ein Forum (melodicaworld) berichtet, dass Biegen durch hartes Blasen Zungen schädige; das ist kein Beleg.
- **Vorschlag (Einschätzung):**
  - Atemdruck = Spur Expression, nur Blasrichtung.
  - Halb gedrückte Taste = Pitch nach unten, analog zum Ventil-Bend beim Akkordeon (Q1). Nur Analogie.

---

## 5. Allgemein: Ansprache und Druckabhängigkeit der Tonhöhe

- **Ansprache:**
  - Q8: Beginn mit Auslenkung „often accompanied by a few cycles of irregular oscillation", dann eine kurze Übergangsphase [wörtlich, selbst gelesen, S. 1].
  - Q7: „the problem of slow speech that typically affects larger, lower-pitched reeds" [wörtlich, selbst gelesen, S. 1].
  - **Einschwingdauer in ms oder Perioden nicht gefunden**. Die Volltexte Biernat/Cottingham POMA 2014 und Cottingham POMA 2013 habe ich nicht gelesen.
- **Tonhöhe gegen Druck:**
  - Normal sinkt die Tonhöhe mit steigendem Druck: Q1 −2,5 Hz/kPa; Q6, S. 13: „Frequency decreases (slightly) with increasing pressure" [wörtlich, selbst gelesen].
  - Beim Harmonium steigt sie leicht, wenn der Druck nachlässt (Q5, S. 3: „the slight increase in playing frequency as the air leaks out") [wörtlich, selbst gelesen].
  - Laut Websuche zu Cottinghams Berlin-Papier gibt es Abweichungen bei sehr kleinem und sehr großem Druck [nur Zusammenfassung; Quelle nicht erreichbar].
- **Vorschlag (Einschätzung):** Kopplung Expression → Tonhöhe mit etwa −10 bis −20 ct/kPa bei voll geöffnetem Ventil (aus Q1 und Q6). Sie ist am Instrument fest eingebaut (Ausstattung) und keine eigene Spur.

---

## Nicht gefunden
- Balgdruck in Pa für pp/mf/ff, an Spielern gemessen; Akzentprofile.
- Pausendauer, Klick und Verstimmung beim Balgwechsel.
- Rate des Bellows shake in Hz.
- Mundharmonika: Spieler-Atemdruck, Dauer eines Bends, Raten und Tiefen von Hand-, Kehl- und Zwerchfell-Vibrato. Zu Mundharmonika und Bending fand ich bei der UNSW nichts.
- Melodica: alle Messwerte.
- Einschwingdauer einer Zunge in ms oder Perioden.
- Harmonium: Saugdruck des amerikanischen Harmoniums, Kennwerte der Schweller-Klappe.
- Die Volltexte Bahnson 1998 (cs.bu.edu gesperrt), Millot ISMA 1997 (404), Ricot 2005, Johnston 1987 (Acoustics Australia).

Kein Treffer ist kein Beweis. Die Suche war auf englische Quellen und frei Zugängliches begrenzt.

## Eigene Einschätzung (kein Beleg)
- **Balg als Expression:** ja. Er ist beim Akkordeon die einzige Quelle der Dynamik. Wirkt er gemeinsam mit halb gedrückter Taste, treibt er auch Pitch, deshalb Expression → Pitch koppeln.
- **Balgwechsel:** Beim unisonorischen Akkordeon eine Artikulation an der Note. Ein Automatismus darf ihn an Phrasengrenzen vorschlagen, wenn der Balgweg zu Ende geht. Beim bisonorischen Instrument ist die Richtung eine Eigenschaft der Note.
- **Musette:** Stimmkurve = Ausstattung (ct je Ton, ansteigend; Größenordnung 0,5–7 Hz an A4, Obergrenze etwa 25 ct). Registerwahl = Passage-Spur.
- **Vorgabe für Spur Vibrato:** Die Vorgabe 5,5 Hz ist für die Mundharmonika plausibel, aber nicht belegt. Beim Akkordeon gibt es kein Tonhöhen-Vibrato vom Spieler. Die „Schwebung" kommt aus der Ausstattung, der Shake ist Artikulation.
- **Lizenzlage:**
  - Frei übernehmbar sind die CC-BY-Quellen Q1, Q3 und Q4.
  - Bei Q5 ist der Vermerk widersprüchlich.
  - Q2 ist „All rights reserved".
  - Q10 ist NC-SA.

> **Typ:** Agentenbericht (Recherche-Agent, S157, 2026-10-28), Teilschritt 31d-9, Schritt 9.2, Entscheid N6 a.
> **Prüfstand:** „nachgelesen“ heißt: im Volltext der Datei unter `quellen/` selbst gelesen (Textebene **und**
> bei Notenbeispielen das gerenderte Seitenbild), mit PDF-Seite und gedruckter Seite. „nur Bericht“ heißt:
> Katalogeintrag, Zusammenfassung oder Drittseite. Bei urheberrechtlich geschützten Quellen (Ekier/Kamiński,
> Palmer) stehen nur kurze Wortlaut-Anker; der volle Wortlaut steht an der genannten Stelle im PDF.

# Recherche: Ausführung der Verzierungen bei Chopin (für die Übertragung von Nocturne Op. 9 Nr. 2)

## Kopf

- **Auftrag:** Belastbare Quellen zur Ausführung von Triller, Vorschlag, Pralltriller/Mordent, Doppelschlag
  und Fiorituren bei Chopin finden, damit Op. 9 Nr. 2 (Es-Dur) Note für Note übertragen werden kann.
- **Datum:** 2026-10-28 (Session 157)
- **Reichweite der Suche:**
  - Werkzeuge: WebSearch, WebFetch, curl (ohne eigene Browser-Kennung); PDFs durchsucht mit
    `quelle_suchen.py`, Notenbeispiele über `pdftoppm` als Bild gerendert und angesehen.
  - Datenbanken/Seiten: chopin-nationaledition.com (Kommentare der Wydanie Narodowe, frei als PDF),
    archive.org (Volltextsuche „Mikuli AND Chopin“), IMSLP (Seite *Nocturnes (Chopin)*), Colenda (UPenn),
    enc.piano.or.jp (PTNA), Wikipedia (nur Bibliographie), content.alfred.com (Leseprobe).
  - Suchbegriffe: Mikuli Vorwort Triller Hilfsnote; Ekier National Edition performance commentary trill
    upper note appoggiatura on the beat; Nocturnes commentaries; Introduction to the National Edition
    Problems of Performance; Op. 9 No. 2 variants pupils’ copies; nifc Nokturn op. 9 nr 2 warianty
    ozdobniki; Henle Vorwort Nocturnes; dissertation Chopin ornamentation nocturnes.
  - Sprachen: Deutsch, Englisch, Polnisch (eine Suche).
  - **Nicht erreichbar / nicht frei:**
    - IMSLP: `https://imslp.org/wiki/Special:ImagefromIndex/972729` (Paderewski-Ausgabe, Nocturnes, PWM 1951)
      und ebenso `…/799177` (Mikuli/Kistner, Nocturnes) antworten mit 302 auf
      `/friendlyredirect.html#/wiki/Special:ImagefromIndex/…` — eine Zwischenseite, die nur mit JavaScript
      weiterführt. **Nicht umgangen**, Dateien nicht geladen. Damit fehlt der Kommentar der
      Paderewski-Ausgabe (Bronarski/Turczyński) ganz.
    - Jan Ekier, *Introduction to the National Edition* (Teil *Performance Issues*, dort die allgemeinen
      Regeln zu Verzierungen): nur als kostenpflichtiges PDF bei PWM
      (`https://www.chopin-nationaledition.com/en/jan-ekier-introduction-to-the-national-edition-performance-issues/`).
    - J.-J. Eigeldinger, *Chopin: Pianist and Teacher as Seen by His Pupils* (Cambridge UP, engl. 1986/1989):
      nicht frei; laut Wikipedia-Bibliographie (nur Bericht) behandelt S. 77–79 die Nocturne Op. 9/2.
    - Henle HN 185 (Nocturnes, Zimmermann): kein freies Vorwort gefunden.
    - UVic-Arbeit `https://dspace.library.uvic.ca/bitstreams/e0a338a8-7c4e-4e4e-905f-9909c622560f/download`:
      HTTP 502. Aufsatz „Chopin’s Singing Melody: Coloratura and Beyond“ (J. of Music Research, NTNU 2022):
      Host `www.musicollege.ntnu.edu.tw` nicht auflösbar; Spiegel bei Mahidol lieferte nur „DSpace“.
- **Geladene Dateien** (alle unter `quellen/`):
  - `Ekier_Kaminski_2017_ChopinWN_Nocturnes_Commentaries.pdf` — Wydanie Narodowe, Nocturnes, *Performance
    Commentary* und *Source Commentary /abridged/*, 17 S. (3. Auflage, Upload 2017).
  - `Ekier_Kaminski_2019_ChopinWN_VariousWorks_Commentaries.pdf`, `…_2022_…_Impromptus_…`,
    `…_2013_…_Ballades_…` — zum Abgleich der Regeln über einzelne Werke hinaus.
  - `Mikuli_1879_ChopinPianoforteWerke_Bd3_Etuden_Vorwort_IA.pdf` — Kistner-Ausgabe Bd. 3 (Etuden), Scan der
    Royal Academy of Music bei archive.org (`gb-lam-0156896`), enthält Mikulis Vorwort, datiert „Lemberg,
    September 1879“, S. III–IV (PDF-S. 7–8).
  - `Palmer_oJ_AlfredChopinNocturnes_Ornamentation_Leseprobe.pdf` — Leseprobe der Alfred-Ausgabe (W. A. Palmer),
    S. 2 „Ornamentation“; Jahr in der Probe nicht angegeben.

**Wichtiger Vorbehalt zur Lage der Quellen:** Die Wydanie Narodowe sagt selbst, die allgemeine Erörterung
der Aufführung stehe im getrennten Band *Introduction … Problems of Performance* (Nocturnes-Kommentar,
PDF-S. 1, gedr. S. 3). Dieser Band ist nicht frei. Die Regeln unten sind deshalb aus **Einzelentscheiden**
der Kommentare zusammengesetzt, nicht aus Ekiers allgemeiner Regel.

---

## 1. Triller (tr mit Wellenlinie)

**1a — Mikuli 1879, Vorwort S. IV (PDF-S. 8), nachgelesen (Bild gezoomt, Fraktur-Antiqua gut lesbar):**
„Triller, die er meist mit der oberen Hilfsnote anfangen liess, mussten weniger schnell, als mit grosser
Gleichheit geschlagen werden; die Trillerendigung ruhig und ohne Ueberstürzung.“
→ Beginn **meist** oben; Tempo: Gleichmaß vor Geschwindigkeit; Schluss ruhig. Keine Zahl für die Wechsel.

**1b — Palmer (Alfred), S. 2, nachgelesen (Seitenbild):** beruft sich auf Chopins Schüler: Chopin habe
„usually began his trills on the *upper* note“; Ausnahme: geht die obere Nebennote legato unmittelbar voraus,
darf auf der Hauptnote begonnen werden (nach Clementi). Steht vor dem Triller ein kurzer Vorschlag **auf der
Tonhöhe der Hauptnote**, beginnt der Triller auf der Hauptnote, auf dem Schlag, ohne Wiederholung. Beim
Triller mit Vorschlagsgruppe („prefixed trill“) wird die zweite Note der Gruppe nicht wiederholt, der
eigentliche Triller beginnt oben. Nachschlag („suffix“) nur, wo er notiert ist (Notenbeispiel „or, with suffix“).

**1c — Wydanie Narodowe (Ekier/Kamiński), nachgelesen:**
- Grundsatz für Vorschlag in Höhe der Trillernote: ein solcher Vorschlag zeigt an, dass der Triller auf der
  Hauptnote ohne deren Wiederholung beginnt, „as in other of Chopin's compositions“ (Nocturnes, Op. 62/1 T. 67,
  PDF-S. 5, gedr. S. 7). Gleich im Bolero Op. 19 und Allegro de Concert Op. 46 T. 130/206
  (VariousWorks, PDF-S. 1–2, gedr. S. 2–3).
- Der Beginn des Trillers fällt **auf den Schlag, zusammen mit der Note der linken Hand**: durchgehend in
  den Nocturnes-Kommentaren, z. B. Op. 32/1 T. 40, 61; Op. 32/2 T. 8; Op. 37/1 T. 8, 24, 74; Op. 48/2 T. 117,
  131–135; Op. 55/1 T. 14, 30, 46; Op. 55/2 T. 1 (PDF-S. 3–5, gedr. S. 5–7; Wortlaut je „The start of the
  trill: … together with … in the L.H.“).
- **Gegenstimme zu Mikuli:** Ekier entscheidet je Stelle und nicht immer „oben“: Op. 55/2 T. 52–53
  „should begin on the principal notes“ (damit die Linie nicht verwischt; PDF-S. 5, gedr. S. 7); im Bolero
  sollen die übrigen Triller ohne Vorschlag „from the main note“ beginnen, einer „better … from the upper
  note“, einer wahlweise (VariousWorks, PDF-S. 1, gedr. S. 2).
- Beginn von unten (untere Sekunde) zeigt Chopin laut Ekier in der Regel mit **zwei** Vorschlagsnoten an
  (Impromptus, Source Commentary zu Op. 29 T. 78, PDF-S. 4, gedr. S. 6).
- Tempo: der Lauf, der einen Triller fortsetzt, ungefähr in der Geschwindigkeit des Trillers
  (Op. 48/2 T. 136, PDF-S. 4, gedr. S. 6). Eine absolute Rate nennt keine Quelle.

**1d — Speziell Op. 9 Nr. 2, T. 7, 15, 23 — nachgelesen (Text und gerendertes Notenbeispiel, Nocturnes PDF-S. 1,
gedr. S. 3, und Source Commentary PDF-S. 7–8, gedr. S. 9–10):**
- Performance Commentary: „The start of the trill with the grace-note (variants 5, 10, 18)“, Notenbeispiel:
  **e♮² – f² – g² – f² – g² – f²** in Zweiunddreißigsteln, Text: „e² together with the L.H. B♭“. Der Triller
  steht also auf f²; mit der Variante beginnt er mit dem Vorschlag e♮² **auf dem Schlag** mit dem B der linken
  Hand, dann Hauptnote, dann obere Nebennote.
- Source Commentary T. 7, 15, 23: In den Erstausgaben (außer FE4) fehlt das Auflösungszeichen e♭²→e♮² **am Ende
  des Trillers** — „definitely Chopin's omission“. → Der Triller hat einen **notierten Nachschlag mit e♮²**.
- Source Commentary T. 15 und 23 (Versionen mit Varianten): als Chopin die Vorschlagsnoten erstmals in T. 7
  einfügte, könnte er sie auch für die Wiederholungen als verbindlich gemeint haben.
- Quellen der Variante 5 (Vorschlag vor dem Triller T. 7): FED, FES, FEJ, GET, FEFr (Liste PDF-S. 8, gedr. S. 10)
  — also in fast allen Schülerexemplaren.

## 2. Vorschläge (kleine Noten, auch durchgestrichen)

**2a — Wydanie Narodowe, nachgelesen:** Regel ist **auf dem Schlag, die erste kleine Note zusammen mit der
Bassnote**:
- Op. 9 Nr. 3 T. 14, 16: Vorschläge „should begin together with the corresponding L.H. note“
  (PDF-S. 2, gedr. S. 4).
- Op. 37/1 T. 1, 5, 6: aus häufigen Zeichen Chopins in einem Schülerexemplar folge, dass „in a l l analogous
  places“ die erste Note der Verzierung mit der Note der linken Hand angeschlagen wird (PDF-S. 3, gedr. S. 5).
- Op. 32/1 T. 9, 13; Op. 55/2 T. 55; Op. 62/2 T. 38, 76 ebenso (PDF-S. 3, 5, 6).
- Allegro de Concert Op. 46: Vorschläge und Arpeggien „as a rule should be begun on the beat“ mit der linken
  Hand (VariousWorks PDF-S. 1–2, gedr. S. 2–3).
**Ausnahmen in derselben Quelle:** Op. 32/1 T. 28, 49 — ein kurzer Vorschlag leicht und schnell, „it is less
important whether the grace-note sounds together with the corresponding bass note or a little before it“
(PDF-S. 3, gedr. S. 5); Op. 46 T. 94–95, 213: „more comfortably played before the beat“ (VariousWorks PDF-S. 2);
liegt die Verzierung über einer gleichbleibenden, gehaltenen Harmonie, sei der Einsatzmoment „not so important“
(ebd.).

**2b — Palmer, S. 2, nachgelesen:** Appoggiaturen einfach und doppelt „generally played on the beat“;
Ausnahmen: Vorwegnahmen der folgenden Note, Oktavsprünge, vor dem Taktstrich notierte.

**2c — Mikuli, Vorwort S. IV, nachgelesen:** zu Doppelschlag und Appoggiatur nur: „empfahl er die grossen
italienischen Sänger als Muster“ — keine Regel zur Lage auf dem Schlag.

Zu durchgestrichenen gegenüber nicht durchgestrichenen Vorschlägen fand sich **keine** eigene Aussage; Ekier
behandelt beide Formen in den gelesenen Stellen gleich (Einsatz mit dem Bass). Kein Treffer ist kein Beweis —
die allgemeine Regel stünde im nicht freien Einführungsband.

## 3. Pralltriller / Mordent („w“ bzw. kurze Wellenlinie)

- **Palmer, S. 2, nachgelesen:** Chopin habe „w“ meist für den dreitönigen „transient trill“ (Pralltriller,
  „inverted mordant“) gebraucht; Rhythmusbild: Triole aus drei kurzen Noten oder zwei kurze und eine längere.
  Die Tonhöhen zeigt das Beispiel nicht ausdrücklich; die Notenköpfe stehen tief–hoch–tief (Hauptnote –
  obere Nebennote – Hauptnote). Wo im Takt, sagt Palmer hier nicht.
- **Wydanie Narodowe, nachgelesen:** „mordent“ meint dort das Zeichen „w“ bzw. zwei kleine Noten:
  - Op. 32/2 ab T. 27: „the first note of the ornament is struck together with the corresponding remaining
    notes of both hands“ (PDF-S. 3, gedr. S. 5).
  - Impromptu Op. 29 T. 1: Beginn mit der ersten Note der linken Hand „more correct“ als vorweggenommen
    (dritte Note mit dem Bass); betont wird die Hauptnote (Impromptus PDF-S. 1, gedr. S. 3).
  - Op. 37/2 T. 7: auf dem Schlag „more correct“, vorweggenommen „also admissible“ (PDF-S. 4, gedr. S. 6).
- **Speziell Op. 9 Nr. 2, T. 27 — nachgelesen (Source Commentary, PDF-S. 8, gedr. S. 10):** ein undeutliches
  Zeichen Chopins in FED „probably indicates that the mordent be erased“. → In T. 27 steht ein Mordent, den
  Chopin in einem Schülerexemplar wohl gestrichen hat; Übertragung mit oder ohne ist belegbar, die Quelle
  neigt zu „ohne“ im Fall der Fassung nach FED.

## 4. Doppelschlag

- **Vorhanden in Op. 9 Nr. 2 — nachgelesen:** Source Commentary T. 2: „In the first editions the turn has no
  chromatic accidentals. Chopin corrected this omission in FED.“ (PDF-S. 7, gedr. S. 9). Welche Vorzeichen,
  zeigt der Kommentar nicht (sie stehen im Notentext der WN, nicht frei).
- **Mikuli, S. IV, nachgelesen:** für den Doppelschlag (gruppetto) die großen italienischen Sänger als Muster.
- **Ausführungsregel aus Chopin-Quellen: nicht gefunden.** Weder die gelesenen WN-Kommentare (Nocturnes,
  Ballades, Impromptus, Various Works) noch die Palmer-Leseprobe geben eine Ausführung des Zeichens. Die
  übliche Konvention (Zeichen über der Note: obere Nebennote – Hauptnote – untere – Hauptnote; Zeichen
  zwischen zwei Noten: erst Hauptnote, dann die Gruppe vor der nächsten Note) ist hier **nicht belegt**.

## 5. Fiorituren / Gruppen kleiner Noten gegen die linke Hand

- **Mikuli, Vorwort S. III (PDF-S. 7), nachgelesen:** „Selbst bei seinem so viel verleumdeten Tempo rubato
  spielte immer eine, die begleitende Hand streng gemessen fort, während die andere, singende, entweder
  unentschlossen zögernd, oder aber wie in leidenschaftlicher Rede mit einer gewissen ungeduldigen Heftigkeit
  früher einfallend und bewegter, die Wahrheit des musikalischen Ausdrucks von allen rhythmischen Fesseln frei
  machte.“ Ebd.: zu Fields Nocturnes habe Chopin „die reizendsten Verzierungen improvisirt“.
- **Wydanie Narodowe, nachgelesen:**
  - Op. 9 Nr. 1 T. 3: Chopins Markierung in einem Schülerexemplar zeige, dass er die freie Gruppierung der
    Figur gegen die linke Hand erlaubte; die rhythmische Teilung darf keinesfalls hörbar werden, die Figur soll
    frei und unregelmäßig wirken (PDF-S. 1, gedr. S. 3).
  - **Op. 9 Nr. 2 T. 31:** Chopins skizzenhafte Notierung der Variante 22b nennt den Einsatz nicht genau;
    „should be played freely“, etwas früher oder später als die Werte nahelegen (PDF-S. 1, gedr. S. 3).
  - **Op. 9 Nr. 2 T. 16 (Source Commentary, PDF-S. 8, gedr. S. 10):** GE2 „quetscht“ den Lauf in den zweiten
    punktierten Viertelschlag; Chopins scheinbarer Fehler (der Lauf passt nicht in den Takt) sei womöglich
    Absicht und deute **poco ritenuto** an.
  - Anker an Schlägen, wo Chopin sie setzt: Op. 15/2 T. 28 (a der linken Hand gleichzeitig mit a¹ auf der
    vierten Sechzehntel, PDF-S. 2), Op. 27/2 T. 51 (Beginn des Laufs mit der vorletzten Begleitnote markiert,
    PDF-S. 3), Op. 46 T. 138 (Lauf nach der fünften Sechzehntel der linken Hand, VariousWorks PDF-S. 1).
- Ergebnis: Die Quellen sprechen für **freie Verteilung innerhalb des Schlags/Takts bei streng gehaltener
  linker Hand**, mit Ankerpunkten nur dort, wo Chopin sie markiert hat; keine Quelle verlangt gleichmäßige
  Teilung auf die Begleitnoten.

## 6. Aussagen speziell zu Op. 9 Nr. 2

- **WN, nachgelesen (PDF-S. 1, gedr. S. 3):** Chopin brachte beim Spielen und Unterrichten immer neue
  Varianten ein; die WN druckt drei Sätze aus Schülerexemplaren (I: 1a, 2c, 20, 21a; II: 2a oder 2b, 4, 6, 12,
  14b, 19b, 21a; III: 1a, 4, 5, 12, 17c oder 17d, 19a, 23, 24b, 25c; vielleicht 10 & 18). Fassung 2a (WN S. 22–25)
  = die reichste authentische Sammlung (aus GET), 2b (S. 26–29) = alle Varianten, Hauptzeilen „nicht
  übertrieben“. Grundsatz der Herausgeber: analoge Stellen zuerst einfach, erst in der Wiederholung verziert;
  die natürliche Einfachheit soll nicht in Verzierungen untergehen.
- T. 5, 13, 21: nach dem Bericht eines Schülers legte Chopin großen Wert auf die Durchgangsnote **fis²**
  (Varianten 3, 8, 16; Quelle laut Source Commentary: Lenz-Aufsatz 1872) (PDF-S. 1, gedr. S. 3).
- T. 7, 15, 23 Triller mit Vorschlag e♮² und Nachschlag e♮² (siehe 1d); T. 2 Doppelschlag mit chromatischen
  Vorzeichen (siehe 4); T. 27 Mordent wohl gestrichen (siehe 3); T. 16 Lauf poco ritenuto (siehe 5);
  T. 31 Variante frei (siehe 5); T. 1 Metronom: Erstausgaben mit falschem Notenwert (zu schnell für Andante)
  (Source Commentary, PDF-S. 7).
- Quellen der Varianten laut WN (PDF-S. 8): FED (Dubois), FER (Rosengardt), FES (Stirling), FEJ (Jędrzejewicz),
  FEX, GET (Toruń), AvL (Lenz-Blatt, UPenn), FEFr (Franchomme), Autograph der ersten 2 Takte für M. Wodzińska
  (1835), **Mikulis Ausgabe „mit den authentischen Verzierungen“ (Kistner 6640)** mit den Varianten 1b, 2a, 6, 7,
  9a, 11, 13, 14, 17a, 20a, 21, 22a, 25b, Lenz 1872, Kleczyński 1886 (nach Tellefsen).
- **Lenz-Blatt, nur Bericht** (Colenda, UPenn, Katalogtext): ein Blatt, Bleistift, vier Ersatzverzierungen
  für Lenz, drei für Op. 9/2, eine für Op. 9/1 T. 45, signiert „Paris 45“.
- **PTNA (Hayashikawa), nur Bericht, maschinell übersetzt:** Chopin habe die Verzierungen jedes Mal anders
  gespielt.
- **Paderewski-Ausgabe:** nicht gelesen (IMSLP-Zwischenseite, s. Kopf). **Eigeldinger S. 77–79:** nicht frei.

---

## Vorschlag: Regel für die Übertragung

1. **Triller ohne Vorschlag:** Beginn auf der **oberen Nebennote, auf dem Schlag** zusammen mit der Note der
   linken Hand; gleichmäßige, nicht schnelle Wechsel; ruhiger Schluss in den notierten Nachschlag.
   *Träger:* Mikuli 1879 S. IV („meist mit der oberen Hilfsnote“), Palmer S. 2; Einsatz mit der linken Hand:
   WN Nocturnes passim. *Widerspruch:* Ekier entscheidet je Stelle und lässt mehrfach auf der **Hauptnote**
   beginnen (Op. 55/2 T. 52–53, Bolero); Palmer erlaubt die Hauptnote, wenn die obere Nebennote legato
   vorausgeht. → Ausnahme in die Regel aufnehmen: geht die obere Nebennote unmittelbar gebunden voraus,
   Beginn auf der Hauptnote.
2. **Triller mit Vorschlag in Höhe der Hauptnote:** Beginn auf der Hauptnote, auf dem Schlag, ohne
   Wiederholung. *Träger:* WN Op. 62/1 T. 67, Op. 46, Bolero; Palmer S. 2 (einig).
3. **Triller mit Vorschlag von unten (Op. 9/2 T. 7, 15, 23):** e♮² auf dem Schlag mit dem B der linken Hand,
   dann f² – g² – f² – g² …, Nachschlag e♮² – f². *Träger:* WN Op. 9/2 Performance Commentary (Notenbeispiel)
   und Source Commentary (e♮² am Trillerende). Ob die Variante schon in T. 7 genommen wird, ist Fassungswahl
   (WN: in den Wiederholungen eher verbindlich).
4. **Vorschläge (einzeln, doppelt, Gruppen):** **auf dem Schlag**, erste kleine Note mit der Bassnote, zulasten
   der Hauptnote. *Träger:* WN Op. 9/3, Op. 37/1 („in all analogous places“), Op. 46; Palmer S. 2.
   *Ausnahmen in denselben Quellen:* sehr kurze Vorschläge über gehaltener Harmonie dürfen auch knapp vorher
   kommen (WN Op. 32/1 T. 28, Op. 46); Palmer: Vorwegnahme, Oktavsprung, vor dem Taktstrich notiert.
5. **„w“ (Pralltriller/Mordent):** drei Töne Hauptnote – obere Nebennote – Hauptnote, **erste Note auf dem
   Schlag** mit den übrigen Noten beider Hände, Betonung auf der Hauptnote. *Träger:* WN Op. 32/2, Op. 29
   („more correct“), Palmer (drei Töne). *Gegenseite:* vorweggenommene Ausführung nennt die WN „also
   admissible“ (Op. 37/2). In Op. 9/2 T. 27 Mordent nach FED wohl gestrichen (WN Source Commentary).
6. **Doppelschlag (Op. 9/2 T. 2):** mit den chromatischen Vorzeichen aus FED; Ausführung **nicht aus
   Chopin-Quellen belegt** — vorläufig nach allgemeiner Konvention (über der Note: oben – Haupt – unten – Haupt,
   auf dem Schlag; zwischen zwei Noten: nach der Hauptnote), mit Mikulis Hinweis „Sänger als Muster“ (also
   gebunden, nicht hastig). Vor dem Bau gegen den Notentext der WN oder Eigeldinger prüfen.
7. **Fiorituren gegen die linke Hand:** linke Hand **streng im Takt**; die Gruppe frei, aber gleichmäßig
   fließend über den Schlag verteilt, ohne hörbare Unterteilung; Ankerpunkte nur, wo Chopin sie markiert;
   in Op. 9/2 T. 16 leichtes ritenuto zulassen, T. 31 frei. *Träger:* Mikuli S. III (begleitende Hand streng),
   WN Op. 9/1 T. 3, Op. 9/2 T. 16 und 31. Für eine Note-für-Note-Übertragung heißt „frei“ praktisch: die
   kleinen Noten gleichmäßig auf die verfügbare Zeit der Melodienote verteilen und keinen Ton einer
   Begleitnote aufzwingen.

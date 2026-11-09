# LumiViz — Ganze Stücke für die Instrumente mit Kern (Kandidaten)

> **Version:** 0.1.0
> **Datum:** 2026-11-09 (Session 182)
> **Typ:** Liste — gemeinfreie Werke, die sich später als ganze Stücke unter
> `asset/loop/modelled_instruments/full/` übertragen lassen (Vorbild `full/nocturne_demo.lvloop`, S158)
> **Status:** Entwurf — nichts davon ist entschieden oder begonnen
> **Zielgruppe:** App-Entwickler, Patrik (Auswahl)
> **Bezug:** [Instrumente_IC_Konzept.md](Instrumente_IC_Konzept.md) · `asset/loop/make_modelled_loops.py` ·
> `asset/loop/make_fluegel_loops.py`, `asset/loop/nocturne_op9_2.py` (Muster einer Übertragung) ·
> `asset/loop/CREDITS.md` (Nennung der Ausgabe)
> **Sprache:** Deutsch

## 1 Herkunft dieser Liste und ihre Reichweite

Die Liste stammt **aus dem Wissen des Modells, ohne Abruf** (S182). Kein Eintrag ist an einer Quelle geprüft:
Lebensdaten, Jahr des Erstdrucks und die Eignung für ein Instrument sind Angaben, die **vor der Aufnahme eines Stücks
je Stück zu belegen sind**. Zu belegen ist dann wie bei der Nocturne:

- das Werk und die Lebensdaten des Komponisten (Quelle nennen),
- die **Ausgabe**, aus der übertragen wird: Verlag, Ort, Jahr, Platte, Scan (etwa IMSLP mit Nummer) — sie muss selbst
  gemeinfrei sein,
- die Nennung in `asset/loop/CREDITS.md`.

## 2 Wann ein Stück frei ist (Stand 2026)

1. **Die Komposition:** Der Komponist ist **vor 1956** gestorben (EU: 70 Jahre nach dem Tod, gezählt ab Jahresende). Für die
   USA gilt zusätzlich die Veröffentlichung **vor 1931** (95 Jahre; Werke von 1930 sind seit dem 1.1.2026 frei).
   Frankreich hat Kriegsverlängerungen — bei französischen Komponisten, die um 1920–1950 gestorben sind, eigens prüfen.
2. **Die Ausgabe:** Aus einer **alten Ausgabe** übertragen, die selbst frei ist. Eine moderne Urtextausgabe kann eigenen
   Schutz haben (Deutschland: wissenschaftliche Ausgaben, § 70 UrhG, 25 Jahre).
3. **Bearbeitungen:** Eine Orchestrierung oder Bearbeitung durch einen anderen hat ihr eigenes Recht (etwa Ravels
   Orchestrierung der *Bilder einer Ausstellung*, Grofés Fassung der *Rhapsody in Blue*). Übertragen wird das Original
   oder eine Bearbeitung, deren Bearbeiter die Regel 1 erfüllt.
4. **Volksweisen und Rhythmen** (Clave, Tumbao, Theka) sind kein Werk; eine bestimmte Aufnahme oder Transkription schon.
   Solche Stücke werden selbst gesetzt.

## 3 Kandidaten

Die Spalte *Instrumente* nennt die Instrumente mit Kern, die das Stück tragen würden (Namen der `.lvinst`). Die Länge
ist ungefähr.

### 3.1 Klaviere mit Hammer (`grand_piano`, `upright_piano`, `honky_tonk_piano`)

| Stück | Komponist (Lebensdaten) | Erstdruck | Instrumente | Warum | Länge | Vorbehalte |
|---|---|---|---|---|---|---|
| Prélude e-Moll Op. 28 Nr. 4 | Frédéric Chopin (1810–1849) | 1839 | `grand_piano` | ruhige Akkorde, Pedal, leise Stufen — zeigt Dämpfer und Stärke; kurz, ein guter zweiter Chopin nach der Nocturne | ca. 2 min | — |
| Prélude Des-Dur Op. 28 Nr. 15 | Chopin | 1839 | `grand_piano` | wiederholter Ton über lange Zeit, Pedal | ca. 5 min | — |
| Nocturne cis-Moll (Lento con gran espressione), op. posth. | Chopin | 1875 | `grand_piano` | Gegenstück zur Nocturne Es-Dur, freie Läufe | ca. 4 min | Ausgabe von 1875 suchen |
| Gymnopédie Nr. 1 | Erik Satie (1866–1925) | 1888 | `upright_piano`, `vibraphone` | langsam, wenige Töne, viel Ausklang — für das Upright und das Vibraphon mit Motor | ca. 3 min | — |
| Gnossienne Nr. 1 | Satie | 1893 | `upright_piano`, `tine_electric_piano` | ohne Taktstriche, freie Zeit | ca. 4 min | — |
| Clair de lune (Suite bergamasque) | Claude Debussy (1862–1918) | 1905 | `grand_piano` | weite Lagen, pp bis f, Pedal | ca. 5 min | — |
| Golliwogg's Cakewalk (Children's Corner) | Debussy | 1908 | `honky_tonk_piano`, `upright_piano` | Cakewalk, kurze Noten, Akzente | ca. 3 min | — |
| Träumerei (Kinderszenen Op. 15 Nr. 7) | Robert Schumann (1810–1856) | 1839 | `grand_piano`, `upright_piano` | kurz, gesangliche Mittelstimme | ca. 3 min | — |
| Für Elise WoO 59 | Ludwig van Beethoven (1770–1827) | 1867 | `upright_piano` | bekannt, Lagenwechsel | ca. 3 min | — |
| Sonate C-Dur KV 545, 1. Satz | Wolfgang Amadeus Mozart (1756–1791) | 1805 | `grand_piano`, `upright_piano` | Läufe und Alberti-Bässe über den Umfang | ca. 4 min | — |
| Maple Leaf Rag | Scott Joplin (1868–1917) | 1899 | `honky_tonk_piano` | der Ragtime schlechthin; das verstimmte Klavier ist sein Klang | ca. 3 min | — |
| The Entertainer | Joplin | 1902 | `honky_tonk_piano` | dasselbe, ruhiger | ca. 4 min | — |
| Solace | Joplin | 1909 | `upright_piano`, `tine_electric_piano` | langsamer Rag, Habanera-Bass | ca. 5 min | — |
| Twelfth Street Rag | Euday L. Bowman (1887–1949) | 1914 | `honky_tonk_piano` | schneller Rag | ca. 3 min | EU frei seit 2020 |

### 3.2 Elektrische Tasten (`electric_grand_piano`, `tine_electric_piano`, `reed_electric_piano`, `clavinet_modal`)

| Stück | Komponist (Lebensdaten) | Erstdruck | Instrumente | Warum | Länge | Vorbehalte |
|---|---|---|---|---|---|---|
| Inventionen BWV 772 (C-Dur), 779 (F-Dur), 784 (a-Moll) | Johann Sebastian Bach (1685–1750) | 18. Jh. (Abschriften), Drucke 19. Jh. | `clavinet_modal` | das Clavinet ist ein Clavichord mit Tangenten — zweistimmige Inventionen sind sein Ursprung; Tonabnehmer je Stimme | je ca. 1–2 min | Ausgabe des 19. Jh. wählen |
| Präludium C-Dur BWV 846 (Wohltemperiertes Klavier I) | Bach | 1801 | `clavinet_modal`, `celesta`, `tine_electric_piano` | gebrochene Akkorde, gleichmäßig — zeigt Dämpfer und Pedal | ca. 2 min | — |
| Preludes for Piano (Nr. 1–3) | George Gershwin (1898–1937) | 1926 (uraufgeführt) | `electric_grand_piano`, `tine_electric_piano` | jazzige Harmonik, Synkopen — der CP-70 und das Rhodes | je ca. 1–2 min | gedruckt 1927: US frei seit 2023, EU seit 2008 |
| Rhapsody in Blue (Fassung für Klavier solo) | Gershwin | 1924 | `electric_grand_piano`, `grand_piano` | groß, Glissandi, Wechsel der Stärke | ca. 16 min | nur Gershwins eigene Klavierfassung, **nicht** Grofés Orchestrierung (Grofé † 1972) |
| Rêverie | Debussy | 1890 | `tine_electric_piano`, `reed_electric_piano` | weiche Akkorde, Pedal — der Klang eines Rhodes | ca. 4 min | — |

### 3.3 Stabspiele (`marimba`, `vibraphone`, `xylophone`, `glockenspiel`, `tubular_bells`, `celesta`, `kalimba`)

| Stück | Komponist (Lebensdaten) | Erstdruck | Instrumente | Warum | Länge | Vorbehalte |
|---|---|---|---|---|---|---|
| Tanz der Zuckerfee (Der Nussknacker Op. 71) | Pjotr Tschaikowsky (1840–1893) | 1892 | `celesta` (Melodie), `marimba` / `upright_piano` (Begleitung) | **das** Stück der Celesta, für sie geschrieben | ca. 2 min | Klavierauszug oder Partitur des 19. Jh. |
| Danse macabre Op. 40 | Camille Saint-Saëns (1835–1921) | 1875 | `xylophone`, `timpani`, `grand_piano` | das Xylophon als Knochenklappern, berühmt | ca. 7 min | — |
| Fossiles und Aquarium (Karneval der Tiere) | Saint-Saëns | 1922 | `xylophone` (Fossiles), `celesta` + `grand_piano` (Aquarium) | Aquarium: Glasharmonika und zwei Klaviere — die Celesta trägt die Glasstimme | je ca. 1–2 min | Erstdruck 1922 (US frei seit 1998) |
| Hummelflug (Das Märchen vom Zaren Saltan) | Nikolai Rimski-Korsakow (1844–1908) | 1900 | `xylophone`, `marimba` | chromatische Sechzehntel durch den Umfang | ca. 1,5 min | — |
| Suite für Violoncello Nr. 1, Prélude BWV 1007 | Bach | 19. Jh. | `marimba`, `vibraphone` | ein Standard der Marimba; Lagen und Akkordbrechung | ca. 2,5 min | Ausgabe des 19. Jh. |
| „Das klinget so herrlich“ (Die Zauberflöte) | Mozart | 1791 (Partitur 1814) | `glockenspiel`, `celesta` | Papagenos Glockenspiel | ca. 1 min | Ausschnitt aus der Oper |
| Der Zauberlehrling | Paul Dukas (1865–1935) | 1897 | `glockenspiel`, `xylophone`, `timpani` | Glockenspiel und Pauke im Orchester | ca. 11 min | EU frei seit 2006 |
| Greensleeves, Frère Jacques (Volksweisen) | — | — | `kalimba`, `vibraphone` | einfache Melodien für die Kalimba; selbst gesetzt | je ca. 1 min | selbst setzen, keine fremde Bearbeitung |
| Symphonie fantastique, 5. Satz (Glocken, Dies irae) | Hector Berlioz (1803–1869) | 1845 | `tubular_bells`, `timpani` | die Glocken auf C und G | Ausschnitt ca. 2 min | Ausschnitt |

### 3.4 Trommeln (`timpani`, `snare_drum`, `bass_drum`, `low_tom`, `conga`, `bongo`, `tabla`)

| Stück | Komponist (Lebensdaten) | Erstdruck | Instrumente | Warum | Länge | Vorbehalte |
|---|---|---|---|---|---|---|
| Also sprach Zarathustra, Einleitung | Richard Strauss (1864–1949) | 1896 | `timpani`, `tubular_bells`, `grand_piano` (Orgelpunkt) | das Paukensolo, C und G | ca. 1,5 min | EU frei seit 2020 |
| Symphonie Nr. 94 „Mit dem Paukenschlag“, 2. Satz | Joseph Haydn (1732–1809) | 1790er | `timpani`, `grand_piano` | leise Melodie, dann der Schlag — Stärke in Stufen | ca. 6 min | — |
| Symphonie Nr. 9, 2. Satz (Scherzo), Anfang | Beethoven | 1826 | `timpani` | die Pauke in Oktaven F | Ausschnitt ca. 1 min | — |
| Boléro (Ostinato der kleinen Trommel) | Maurice Ravel (1875–1937) | 1929 | `snare_drum`, `low_tom` | 169 Mal derselbe Rhythmus, crescendo — Stärke über 15 min | ca. 15 min | **Frankreich: Kriegsverlängerung, eigens prüfen**; US frei seit 2025 |
| Mars (Die Planeten) | Gustav Holst (1874–1934) | 1921 | `timpani`, `snare_drum`, `bass_drum` | 5/4, Pauke und Trommeln | ca. 7 min | EU frei seit 2005, US seit 2017 |
| Clave, Tumbao, Teentaal (Rhythmen) | — | — | `conga`, `bongo`, `tabla` | kein Werk — selbst gesetzte Muster mit offenen, gedämpften und Randschlägen | — | selbst setzen |

### 3.5 Für viele Instrumente zugleich (Ensemble)

| Stück | Komponist (Lebensdaten) | Erstdruck | Instrumente | Warum | Länge | Vorbehalte |
|---|---|---|---|---|---|---|
| In der Halle des Bergkönigs (Peer Gynt) | Edvard Grieg (1843–1907) | 1888 | `marimba`, `xylophone`, `glockenspiel`, `timpani`, `bass_drum`, `grand_piano` | ein Motiv durch alle Lagen, accelerando, crescendo | ca. 2,5 min | — |
| Chinesischer Tanz, Trepak (Der Nussknacker) | Tschaikowsky | 1892 | `glockenspiel`, `celesta`, `xylophone`, `snare_drum` | kurz, farbig | je ca. 1 min | — |
| Das große Tor von Kiew (Bilder einer Ausstellung) | Modest Mussorgski (1839–1881) | 1886 | `tubular_bells`, `grand_piano`, `timpani` | Glockengeläut im Klaviersatz | ca. 5 min | **nur das Klavieroriginal**, nicht Ravels Orchestrierung |

## 4 Vorschlag zur Reihenfolge (nicht entschieden)

Nach dem, was jedes Stück an den Instrumenten zeigt, nicht nach Aufwand:

1. **Tanz der Zuckerfee** — die Celesta ist dafür gebaut; Begleitung aus Klavier und Marimba.
2. **Inventionen** von Bach am **Clavinet** — das Instrument ist ein Clavichord; zugleich der Hörtest
   `klasse1_34i` (welches Clavinet den Namen behält).
3. **Maple Leaf Rag** am **Honky-tonk** — sein Klang ist der des Ragtime.
4. **Also sprach Zarathustra**, Einleitung — die Pauke mit Wirbel und Pedal (Spur `pitch`), die Röhrenglocken.
5. **Gymnopédie Nr. 1** am Upright, zweite Fassung am Vibraphon mit Motor — sobald 31g Spuren am Muster kann.

Ein ganzes Stück mit Spielweisen, die heute nur feste Werte je Drum-Spur kennen (Motor, Wirbel, Pedal der Pauke), spielt
erst mit 31g so, wie es notiert ist.

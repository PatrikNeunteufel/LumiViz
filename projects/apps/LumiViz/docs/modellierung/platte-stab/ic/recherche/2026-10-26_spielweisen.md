# Recherche IC1: Spielweisen und Steuerung — Platte und Stab

> **Auftrag:** Schlägel, Anschlagstelle, Wirbel, Abdämpfen, Vibraphon-Motor, weitere Spielweisen (Bogen,
> Pitch-Bending, Kalimba, Steeldrum, Rhodes/Wurlitzer-Tremolo) und Widersprüche zum Bestand in LumiViz (SF6 a, SF8 a,
> SF10 a, SK6 a).
> **Datum:** 2026-10-26 (IC Session 1).
> **Bearbeitet von:** einem Agenten (Claude) — **Bericht eines Agenten**, unten **unverändert**. Kennzeichnung im
> Bericht: [wörtlich] = Wortlaut, den das Werkzeug zitierte oder den der Agent selbst im PDF-Text las (mit Seite);
> [nur Zusammenfassung] = sonst.
> **Reichweite:** im Bericht selbst (27 Suchen, 37 Abrufe, nur Englisch; Yamaha-Anatomie, Varner-PDF, Chaigne &
> Doutaut 1997, Fletcher & Rossing nicht erreicht).
> **Von der Sitzung selbst nachgelesen:** siehe `../modell-entwurf.md` und `../offene-fragen.md` (je Angabe vermerkt).
> **Eingeflossen in:** `../modell-entwurf.md`, `../offene-fragen.md`.
> **Anmerkung der Sitzung:** Die genannten PDFs liegen im Cache der Claude-Sitzung (`tool-results/`), nicht im Repo.

---

# Spielweisen und Steuerung der Klasse „Platte und Stab“ – Recherchebericht

## Reichweite

- **Umfang:** 27 Websuchen (WebSearch) und 37 Abrufe (WebFetch). Das sind deutlich mehr als die vorgesehenen 25–40. Grund: Viele Abrufe scheiterten, und WebFetch konnte PDFs nicht lesen.
- **Sprache und Suchbegriffe:** Gesucht habe ich nur auf Englisch, unter anderem mit vibraphone motor rpm/Hz, stop angle, mallet contact time, hardness, strike position, roll speed, dead stroke, mallet dampening, half pedal, bowed vibraphone/crotales, pitch bend, kalimba wa-wa, mbira buzzers, steelpan roll/nonlinear, Rhodes hammer tips/damper/Suitcase vibrato, Wurlitzer tremolo, celesta damper, chimes damper.
- **Fundorte:** US-Patente (Google Patents), Wikipedia, Yamaha-Lehrseiten, Lehrseiten von Spielern (malletjazz.com, adamholmesmusic.com, PAS), fenderrhodes.com, Hersteller (Adams, Collins Rhythmcraft) sowie Tagungs- und Hochschulschriften als PDF.
- **Nicht erreichbar:**
  - hub.yamaha.com/anatomy-of-a-vibraphone (404)
  - Varner-PDF bei Yamaha AU (403)
  - steelpan-steeldrums-information.com (DNS-Fehler)
  - soundbrenner.com (404 bzw. 429)
  - micklabriola.com und der chise.org-Wikipedia-Spiegel (404)
  - Chaigne & Doutaut 1997 (JASA) und Fletcher & Rossing, *The Physics of Musical Instruments*, habe ich nicht im Volltext erreicht.
- **Wie ich PDFs gelesen habe:** WebFetch lieferte bei PDFs nur Binärdaten. Das Werkzeug legt die Dateien selbst in seinen Cache. Diese Cache-Dateien habe ich lokal nur gelesen, mit `pdftotext` und `~/.claude/tools/quelle_suchen.py`; gespeichert habe ich nichts. PDF-Zitate unten sind deshalb **[wörtlich]** mit PDF-Seite.

**Cache-Dateien aus meinen Abrufen** (alle unter `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-InstrumentsCalculation\82480a87-77d1-4e60-8192-4b6b647cc1bd\tool-results\`):

| Datei | Inhalt |
|---|---|
| `webfetch-1793025665109-xeyn0t.pdf` | Candocia |
| `webfetch-1793025689894-a11ox3.pdf` | Bilbao/Torin/Chatziioannou |
| `webfetch-1793025780891-492rlg.pdf` | Fletcher 2010 |
| `webfetch-1793025914789-tliswh.pdf` | Yang et al. 2020 |
| `webfetch-1793025978632-3bsaxa.pdf` | Hansen & Rossing 2005 |

**Hinweis zu fremden Cache-Dateien:** Im selben Cache-Ordner lagen PDFs, die ich **nicht** selbst abgerufen habe. Vermutlich stammen sie von parallelen Agenten derselben Sitzung. Drei davon habe ich mitgelesen, weil sie zum Thema gehören:

- `webfetch-1793025712476-mhf890.pdf` (Rucz & Ulveczki)
- `webfetch-1793025751806-5sloba.pdf` (Pfeifle 2017)
- `webfetch-1793025782841-e4j470.pdf` (Sønderbo 2024)

Woher genau diese Dateien stammen, habe ich nicht geprüft.

**Kennzeichnung im Bericht:**
- **[wörtlich]** heißt: Das Werkzeug gab den Wortlaut als Zitat zurück, oder ich habe ihn selbst aus dem PDF-Text gelesen.
- **[nur Zusammenfassung]** heißt: Die Angabe beruht auf einer Zusammenfassung des Werkzeugs und ist kein Beleg.

---

## 1. Schlägel (mallets)

**Arten und Klang**

- **Lehrseite Yamaha „deconstructing mallets“** [nur Zusammenfassung]: Harte Schlägel klingen heller, weiche dunkler, weil weiche Schlägel die hohen Frequenzen dämpfen.
- **Wikipedia „Vibraphone“:** Schlägel haben meist einen „rubber ball core wrapped in yarn or cord“ [wörtlich].
- **NewMusicBox, „Writing for Percussion: Mallets and Related Technical Issues“, BFC Admin, 1.4.2004**, newmusicusa.org, frei zugänglich:
  - Garn: „very little“ Anschlaggeräusch [wörtlich, Bruchstück].
  - Kordel: „Cord mallets are generally harder than yarn mallets and have a firmer impact sound.“ [wörtlich]
  - Messing vor allem für Glockenspiel [nur Zusammenfassung].
- **Adams Musical Instruments, Produktseite Robert van Sice** (Herstellerangabe). Ein wichtiger Fund, weil die Klangfarbe hier selbst von der Anschlagstärke abhängt:
  - monotonal: „designed to produce a consistent, singular tone across the dynamic range“ [wörtlich]
  - multitonal: „designed to produce different tones depending on how hard they strike the bars“ [wörtlich]
- **Candocia, *Harmonic Analysis of Mallet Percussion*** (Folien eines REU-Projekts, Univ. of Illinois, 2010 laut PDF-Metadaten), frei zugänglich, Folie „Conclusion“:
  - „Soft mallets on marimba and vibes induce sounds with less harmonics“ [wörtlich]
  - „Harder mallets on marimba produce trends in harmonics similar to xylophone“ [wörtlich]

**Kontaktzeit und Masse: keine Messwerte gefunden**

- **Liljencrants, Mechanical Music Digest, 6.1.2002** (Forenbeitrag, also kein Fachaufsatz):
  - optimale Kontaktzeit „about a half period of the oscillation you want to evoke“ [wörtlich]
  - Spektrum der Anregung „extends roughly up to the frequency 1/T“ [wörtlich]
  - Masse des Schlägels „in proportion to the weight of the beaten resonator“ [wörtlich]
- **NewMusicBox (s. o.):** Härteres Kopfmaterial und steifere Stiele erhöhen die Kontaktzeit [nur Zusammenfassung; Wortlaut: „the harder the material of the mallet heads and the more stiff the shafts“]. Das widerspricht der üblichen Physik (hart = kürzer). Gemeint ist dort vermutlich das Liegenbleiben durch geringen Rückprall. Die Tabelle 2.3 „approximate contact time of mallet heads on bars“ liegt nur als Bild vor; ihre Werte konnte ich nicht lesen.
- **Kontaktmodell:** Rucz & Ulveczki, „A Coupled 3D Finite Element Model of the Sound Production by Mallet Percussion Instruments“, ICSV27 (IIAV), Juli 2021, frei als PDF. Hertz-Kontakt: „F = K_M δ_M^{3/2}“ [wörtlich, Formel (6), S. 3 sinngemäß], „Following [1]“ (gemeint ist Chaigne & Doutaut). Dazu „very high sampling frequency is necessary for the short duration of the contact“ [wörtlich, S. 4–5].
- **Rhodes-Hammer:** Sønderbo, *Real-time physical model of the Rhodes Electric Piano*, Masterarbeit, Aalborg University, 23.5.2024, frei:
  - Tabelle 3.2 „Hammer mass … Mh (Kg) 0.011“ [wörtlich, PDF-S. 18 / gedr. S. 14]. Die übrigen Werte der Tabelle sind im Textauszug durcheinandergeraten und deshalb nicht verwertbar.
  - „the stiffness of the hammer tip increases for higher tines“ [wörtlich, ebd.]
  - Hammermodell nach Hunt & Crossley.
- **fenderrhodes.com „hammer-tips“:**
  - Wechsel um 1970: „Rhodes changed to a neoprene hammer tip“ [wörtlich]
  - „5 hardness gradations across the 73 keys“ [wörtlich]
  - Shore 30/50/70/90, im Diskant Ahorn mit Schrumpfschlauch [nur Zusammenfassung]
  - zu hart: „clank“; zu weich: „thunk“ [wörtlich, Einzelwörter]
- **Wikipedia „Rhodes piano“:** „the hammer tips were changed to neoprene rubber instead of felt“ [wörtlich].
- **Pfeifle**, „Real-Time Physical Model of a Wurlitzer and Rhodes Electric Piano“, DAFx-17, Edinburgh 2017, frei, S. 2:
  - Tine „excited by hammer having a neoprene tip“ [wörtlich]
  - Wurlitzer: Hammer aus Ahornsperrholz „that has a felt tip“ [wörtlich]
  - höhere Anschlaggeschwindigkeit „results in a richer harmonic sound“ [wörtlich]

**Wechselt der Spieler die Schlägel je Stück?**

- **NewMusicBox:** Ein Wechsel ist möglich, aber oft fehlt die Zeit („little or no time to pick up or put down mallets“ [wörtlich]). Komponisten geben die Schlägel als Text in der Partitur an [nur Zusammenfassung].
- **Vic Firth** (über Suche): Komponisten können dieselben Modelle „for longer passages“ in der Hand lassen [nur Zusammenfassung].
- **Adams** empfiehlt den weichsten Multitone-Schlägel „in the bass position“ [nur Zusammenfassung]. Bei vier Schlägeln kann also **jede Hand andere Schlägel** halten.

## 2. Anschlagstelle

- **Jeff La Favre, „Position of Mallet Blow on Bar“**, lafavre.us, Stand 16.3.2007 (Messungen eines Hobbyisten, frei zugänglich):
  - Mitte: „the first transverse mode (fundamental) is strongly excited because that is the antinode“ [wörtlich]. Die 2. und 4. Mode werden unterdrückt, weil sie dort einen Knoten haben [nur Zusammenfassung].
  - Außermittige Position 2: „the second and fourth transverse modes are excited to a much greater degree“ [wörtlich].
  - Position 3 liegt „relatively near the nodes of the first and fourth transverse modes“ [wörtlich].
  - Stabende: „should excite all four modes … since there are no nodes located there“ [wörtlich].
- **Rucz & Ulveczki 2021**, S. 7–8: Neben den Knotenlinien der ersten Mode „torsional modes can quite efficiently be excited, especially by stronger hits“ [wörtlich]. Auf der Längsachse „torsional modes are not excited at all“ [wörtlich, S. 8].
- **Candocia:** Tabellen mit Teiltönen für Mitte, Rand und Knoten. Der Grundton bleibt bei etwa 887 Hz (Xylophon) [wörtlich als Tabellenwert]. Laut Fazit hat das Xylophon „different sounds depending on striking location“ [wörtlich].
- **Rhodes:** Die Anschlagstelle (strike line) wird beim Regulieren eingestellt. Ein Treffer neben der Linie gibt einen „thunk“ [nur Zusammenfassung, fenderrhodes.com Service]. Sønderbo (S. 14): Tiefe Töne werden näher zur Tine-Mitte getroffen, hohe näher an der Basis [nur Zusammenfassung meiner Lektüre, kein Zitat].
- **Röhrenglocken:** Angeschlagen „along their top edge“ [wörtlich, Yamaha „anatomy of chimes“, Troy C. Wollwage, 5.9.2019].
- **Steeldrum:** Hansen & Rossing, „The Caribbean Steelpan, and some Offsprings“, Forum Acusticum 2005 Budapest, frei als PDF. Abb. 2 zeigt Spektren bei „mallet blow near the front of the section“ [wörtlich, S. 2]. Zur Stelle selbst nur eine Aussage aus der Tuner-Praxis [nur Zusammenfassung].

## 3. Wirbel (roll)

- **Schlagrate:** Yang, Savery, Sankaranarayanan, Zahray, Weinberg, „Mechatronics-Driven Musical Expressivity for Robotic Percussionists“, NIME 2020 / arXiv:2007.14850, frei. Auf S. 5: „the average fastest speed recorded from 4 different human marimba players was 7.1 Hz“ [wörtlich]. Ob das je Schlägel oder für beide Hände zusammen gilt, ist offen.
- **Register:** Laut Yamaha/Varner ist der Wirbel in der Tiefe langsamer als in der Höhe [nur Zusammenfassung; PDF 403].
- **Rollarten:** Christopher Wilson, PAS-Blog, 8.5.2023:
  - Ripple Roll: die innere Hand „flop“ hinter die äußere [wörtlich, Bruchstück]
  - Rollgeschwindigkeit als „an extra element of musicality and tone color“ [wörtlich]
  - Zahlen gibt der Beitrag nicht.
  - Stevens, „Rolls and Notation“, Percussive Notes 19/1 (1980), nur zitiert, nicht erreicht.
- **Steeldrum:** Der Single-Stroke-Wirbel ist das Mittel, einen Ton zu halten [nur Zusammenfassung]. Laut Sonokinetic-Handbuch ist ein schneller Tremolo zwischen zwei Tönen üblich [nur Zusammenfassung].
- **Vibraphon:** Keine Quelle zur Häufigkeit von Wirbeln gefunden.

## 4. Abdämpfen

**Pedal am Vibraphon**

- Wikipedia: „A foot pedal lowers the bar and allows notes to ring freely“ [wörtlich]. Beim halben Pedal drückt man das Pedal so weit, dass es „just enough to remove the spring pressure from the bars“ [wörtlich].
- Gifford Howarth, Yamaha „vibraphone pedagogy“, 12.7.2021: „Pedaling is not an “on” or “off” option.“ [wörtlich] und „“half pedal” or “flutter pedal”“ [wörtlich].

**Abdämpfen mit Schlägel oder Hand**

- James Walker, malletjazz.com „Mallet Dampening“, © 2001 (Lehrseite):
  - mit „the head of a mallet to stop the vibrations of a particular bar“ [wörtlich]
  - Handdämpfen nur „when moving from a note on the lower manual to the upper manual“ [wörtlich]
- Wikipedia: „pressing another mallet into the ringing bar“ [wörtlich].

**Dead stroke**

- Walker: „where the mallet remains on the bar after striking it“ [wörtlich].
- Notation oft mit „x“ [nur Zusammenfassung].

**Celesta**

- UBC Wiki „Course:Phys341 2020/Celeste“ (Kursprojekt, 9.4.2020):
  - „each bar has a damper placed over it“ [wörtlich]
  - Das eine Pedal „lifts all of the dampers off at once“ [wörtlich]
- Hämmer mit Filz („felt-tipped mallets“ [wörtlich]).

**Röhrenglocken**

- Yamaha (Wollwage 2019): Das Pedal löst die Dämpferleiste, die Rohre klingen „freely“ [wörtlich, Einzelwort].
- Ein „damper stopper“ hält die Dämpfung dauerhaft offen [nur Zusammenfassung].
- Zum halben Pedal steht dort nichts.

**Rhodes**

- Pfeifle 2017, S. 2: „individual felt damper that is in contact with the tines from below“ [wörtlich].
- Sønderbo, S. 14: Der Dämpfer greift „when a key is released, unless the sostenuto pedal is held“ [wörtlich]. Gemeint ist vermutlich das Haltepedal (Sustain); Wikipedia nennt „a sustain pedal derived from a Rogers hi-hat stand“ [wörtlich].

**Wurlitzer**

- Wikipedia „Wurlitzer electronic piano“: „The instrument is fitted with a mechanical sustain pedal.“ [wörtlich]

## 5. Vibraphon-Motor

**Drehzahl**

- US 9 922 628 B1 (Stephen J. Cole, Marimba One Inc., angemeldet 10.5.2017, erteilt 20.3.2018): „As each disc rotates (e.g., at 60 to 720 rpm), it alternately occludes or opens its respective tube“ [wörtlich], also 1–12 Umdrehungen je Sekunde.
- US 10 847 126 B2 (Deal, Galler, Rizkalla, Salama; Indiana University Research and Technology Corp.; 24.11.2020): Bereich „from about zero to 12 rotations per second“ [wörtlich].
- Wikipedia: „typically support rotation rates in the range of 1–12 Hz“ [wörtlich].
- **Offen:** Ob eine Umdrehung einer oder zwei Modulationsperioden ergibt (Scheibe zweimal zu je Umdrehung?), sagt keine der Quellen.
- **Spieler:** Milt Jackson etwa 3,3 Umdrehungen je Sekunde, Lionel Hampton etwa 10 [nur Zusammenfassung, Wikipedia-Abschriften, Quelle nicht erreicht].

**Stellung der Klappen bei „aus“**

- US 4 619 178 (Motoya Kondoh, Nippon Gakki, 28.10.1986): Ohne Steuerung bleiben die Rohre „open, half closed or fully closed“ [wörtlich]. Dann sei „no constant tone volume is expected for non-vibrato performance“ [wörtlich]. Die Erfindung hält die Klappen so an, dass sie „leave the top end of the resonance tube … open“ [wörtlich].
- US 9 922 628: „Discs 720 are stopped in a vertical position“ [wörtlich], also offen.

**Hochlaufen**

- US 9 922 628: Der Motor „accelerates to a predetermined speed at a rate determined by a manually adjustable acceleration control“ [wörtlich].

**Wirkung auf den Klang**

- Wikipedia: Der Motor „creates a tremolo effect and a slight vibrato“ [wörtlich].
- Yamaha: kleine Tonhöhenschwankung, dazu Lautstärkeänderung, stärker bei höherem Tempo [nur Zusammenfassung; Seite 404].
- Wikipedia: Resonatoren „amplify the fundamental frequency, but not the upper partials“ [wörtlich]. Die Modulation trifft also vor allem den Grundton. Das ist meine Ableitung, kein Beleg.
- Candocia: „Vibraphone very simple sound when wet, more complicated when dry“ [wörtlich]. Was „wet“ hier heißt, ist unklar.
- **Tiefe der Modulation:** keine Messung gefunden.

**Tempo während des Spiels ändern**

- US 10 847 126: Bisher werde das Tempo nur geändert, „when the player is not playing … with one of the player's hands“ [wörtlich, Bruchstück]. Ein Fußpedal „may be used to modulate the motor speed“ [wörtlich], stört aber „operation of the dampening pedal“ [wörtlich]. Ziel ist Ändern „without having to stop playing“ [wörtlich].

## 6. Weitere Spielweisen

**Gestrichenes Vibraphon und Crotales (Bogen)**

- Walker, malletjazz.com „Extended Techniques“ (© 2001):
  - meist Kontrabassbogen, „draw the bow vertically across the extended edge of the bar“ [wörtlich]
  - Lautstärke über Bogendruck und -geschwindigkeit [nur Zusammenfassung]
- Adam Holmes, „on bowed vibraphone“, 7.12.2020 (Spieler-Blog):
  - „a bowed vibraphone won't make sound unless the pedal is down!“ [wörtlich]
  - Dynamik: „the ceiling is rather low“ [wörtlich]
- Wikipedia: Der Bogen „emphasizes the higher harmonics“ [wörtlich].
- Crotales (Wikipedia): „commonly played by being struck with hard mallets“ und „by bowing“ [wörtlich]. Crotales ins Wasser tauchen ergibt ein Glissando [nur Zusammenfassung].

**Flageolett (harmonics)**

- Walker: Mitte berühren, am Knoten anschlagen, klingt zwei Oktaven höher [nur Zusammenfassung].

**Pitch-Bending am Vibraphon**

- Walker: harten Schlägel an den Knoten setzen, „drag it away from the nodal point“ [wörtlich]. Hinter der Mitte steigt die Tonhöhe wieder [nur Zusammenfassung].
- Wikipedia: „lower the pitch by about a half step“ [wörtlich].
- NewMusicBox nennt Becks *Jazz Variants* als Beispiel in der Literatur [nur Zusammenfassung].

**Glissando**

- Keine Quelle gefunden.

**Kalimba**

- Collins Rhythmcraft, „Kalimbas“ (Herstellerseite, ohne Datum):
  - „Cover the holes with your pointer fingers and pluck a tine.“ [wörtlich]
  - zwei Löcher „to produce a wonderful vibrato sound“ [wörtlich]
  - zu hartes Zupfen „will produce a distorted sound“ [wörtlich]
  - mit den Daumennägeln zupfen („sliding my thumb nails gently down the tines“ [wörtlich])

**Mbira, Schnarren (buzz)**

- mbira.org: Blechdeckel oder Muscheln auf einer Platte, „placed on the lower portion of the mbira soundboard“ [wörtlich]. Der Klang reicht „from a soft shimmering hiss to a tambourine-like sound“ [wörtlich]. Auch auf der Kalebasse (deze) [wörtlich: „mounted on the deze“].

**Steeldrum**

- Hansen & Rossing 2005, S. 2: Stärkere Anschläge „do not merely cause a proportionate spectral amplitude increase, but rather excite additional spectral components“ [wörtlich, gekürzt über 15 Wörter: nur „excite additional spectral components“ als Kern]. Außerdem „The tone color of the instrument is noticeably dependent on the intensity of performance.“ [wörtlich]
- Schlägel: Gummi am härtesten, Wolle mittel [nur Zusammenfassung, Sonokinetic].

**Rhodes (Suitcase-Tremolo)**

- Wikipedia: „a tremolo feature that bounces the output signal … across two speakers“ [wörtlich]. Auf manchen Modellen ist es „inaccurately labeled "vibrato"“ [wörtlich].
- fenderrhodes.com „effects“:
  - „The idea behind the Vibrato was to simulate a rotating speaker“ [wörtlich]
  - Amplitude verändert „in a square-wave pattern“ [wörtlich]
  - ab Stereo „translated into a panning effect“ [wörtlich]
  - „Front-panel controls were provided for Speed and Intensity.“ [wörtlich]
  - Raten in Hz: nicht gefunden.

**Wurlitzer**

- Wikipedia: „It had a fixed rate but adjustable depth.“ [wörtlich] Auf dem Bedienfeld „incorrectly labelled "vibrato"“ [wörtlich].
- Harter Anschlag macht den Klang „more aggressive“ („bark“) [wörtlich, Bruchstück].

**Röhrenglocken**

- Rohhaut- oder Kunststoffhammer („rawhide- or plastic-headed hammer“ [wörtlich]), Dämpferpedal (s. 4).

---

## 7. Widersprüche zum Bestand

| # | Bestand | Quelle | Beleg |
|---|---|---|---|
| W1 | Expression: nein | Der Bogen am Vibraphon und an den Crotales regt dauernd an. Die Lautstärke folgt Bogendruck und -geschwindigkeit; der Ton baut sich langsam auf. | Walker „Extended Techniques“ [wörtlich: Bogenführung; Druck/Tempo nur Zusammenfassung]; Holmes 2020 [wörtlich] |
| W2 | Pitch: nein | Pitch-Bending verschiebt die Tonhöhe stetig innerhalb einer Note, etwa einen Halbton nach unten; die Tonhöhe kann auch wieder steigen. | Wikipedia [wörtlich]; Walker [wörtlich] |
| W3 | Vibrato: nein | Der Motor erzeugt neben dem Tremolo „a slight vibrato“. Das ist kein Widerspruch, solange das Vibrato als Folge des Motors gerechnet wird und keine Spielersteuerung ist. | Wikipedia [wörtlich] |
| W4 | Motor als Passage-Spur mit Umschaltstellen (an/aus) | Das Tempo des Motors ist stufenlos (0 bis etwa 12 U/s). Es wird im Spiel geändert, auch per Fußpedal, also stetig. Ein Hochlaufen ist einstellbar. Der Haltewinkel beim Ausschalten bestimmt den Klang ohne Motor (offen, halb, zu). | US 10 847 126 [wörtlich]; US 9 922 628 [wörtlich]; US 4 619 178 [wörtlich] |
| W5 | Ausstattung Dämpferleiste: Loslassen dämpft, Pedal hält | Beim Vibraphon ist der Dämpfer **eine** Leiste unter dem Pedal; Loslassen der Taste gibt es dort nicht. Abgedämpft wird über das Pedal oder je Note mit Schlägel/Hand. Celesta, Rhodes und Wurlitzer haben Einzeldämpfer je Taste und ein Pedal. Röhrenglocken haben eine Dämpferleiste mit Pedal und eine Feststellung („damper stopper“). Sønderbo nennt das Rhodes-Pedal „sostenuto“, vermutlich falsch. | Wikipedia [wörtlich]; UBC [wörtlich]; Pfeifle [wörtlich]; Wollwage [Zusammenfassung]; Sønderbo S. 14 [wörtlich] |
| W6 | Halbes Pedal = Andruck des Dämpfers | Das stimmt mit „remove the spring pressure“ überein. Dazu kommt das „flutter pedal“, also schnelles Pedalspiel als stetige Kurve. | Wikipedia [wörtlich]; Howarth [wörtlich] |
| W7 | Kalimba = Zupfen × Stab, Expression nein | Das „Wa-Wa“ durch Öffnen und Schließen der Löcher verändert den Klang laufend während des Tons; der Hersteller nennt es „vibrato“. | Collins [wörtlich] |
| W8 | Spielweisen an genau einem der drei Orte | Bei vier Schlägeln können die Hände verschiedene Schlägel halten (weich im Bass). Der Schlägel hängt dann an der Hand bzw. der Note, nicht an der Passage. | Adams [nur Zusammenfassung]; NewMusicBox [Zusammenfassung] |
| W9 | Anschlagstärke wie beim Klavier | Dazu passt die Quellenlage: Multitone-Schlägel, Steeldrum und Rhodes/Wurlitzer ändern mit der Stärke die Klangfarbe, nicht nur den Pegel. Das ist kein Widerspruch; das Modell braucht dafür einen nichtlinearen Kontakt. | Adams [wörtlich]; Hansen & Rossing S. 2 [wörtlich]; Pfeifle S. 2 [wörtlich] |
| W10 | Glissando nicht genannt | Crotales im Wasser ergeben ein Tonhöhenglissando. | [nur Zusammenfassung] |
| W11 | Rhodes/Wurlitzer: „Vibrato“ | Es ist ein Amplituden-Tremolo bzw. Panning, keine Tonhöhe. Das passt zu „Vibrato: nein“, braucht aber einen eigenen Ort für das Effektgerät (Tempo, Tiefe). | Wikipedia [wörtlich]; fenderrhodes.com [wörtlich] |

---

## Nicht gefunden

- Gemessene Kontaktzeiten in ms je Schlägelart. Chaigne & Doutaut 1997 und Fletcher & Rossing habe ich nicht im Volltext erreicht; die Tabelle bei NewMusicBox liegt nur als Bild vor.
- Tiefe der Motor-Modulation (dB, Cent), ob eine Umdrehung eine oder zwei Modulationsperioden ergibt, Tremolo-Raten von Rhodes (Peterson/Janus) und Wurlitzer in Hz.
- Wirbelraten je Register in Zahlen; eine Quelle zu Wirbeln am Vibraphon und an Glocken.
- Glissando an Marimba und Vibraphon; Tonhöhenänderung durch Druck auf die Kalimba-Zunge.
- Halbes Pedal an Celesta, Röhrenglocken und Rhodes.
- Primärquelle für die Motortempi von Jackson und Hampton.
- Abdeckbare Schnarrer an der Kalimba (nur Mbira gefunden).

Kein Treffer ist kein Beweis.

## Eigene Einschätzung (kein Beleg)

| Spielweise | Ort | Modellgröße |
|---|---|---|
| Schlägelart (Härte, Masse, Kopfgröße) | Passage (Wechsel braucht Zeit); bei 4 Schlägeln je Hand bzw. Note | Kontaktsteifigkeit K, Exponent α (Hertz ≈ 3/2), Masse, Kontaktbreite |
| Rhodes-Hammerspitze, strike line, Abstand Tine–Tonabnehmer | nie | Shore-Härte je Register, Anschlagstelle, Geometrie des Tonabnehmers |
| Anschlagstärke | Note | Anfangsgeschwindigkeit des Schlägels (nichtlinearer Kontakt → Klangfarbe) |
| Anschlagstelle (Mitte/Rand/Knoten) | Note (ggf. Grundwert je Passage) | Ort der Kraft auf der Modenform (gewichtet die Moden); außerhalb der Achse → Torsionsmoden |
| Wirbel | Note | Schlagrate (Ausgangswert 5–7 Hz, je Register geringer), Abwechseln der Hände, Zufall in Zeit und Stärke |
| Dead stroke | Note | Schlägel bleibt nach dem Anschlag liegen (Dämpferkontakt am Anschlagort) |
| Abdämpfen mit Schlägel/Hand | Note (zum Zeitpunkt) | Dämpferkontakt auf einem einzelnen Stab |
| Pedal / halbes Pedal / flutter | Passage als **stetige Spur** (nicht nur Umschaltstellen) | Andruck des Dämpfers 0…1 |
| Feststellung der Dämpfung (Röhrenglocken) | nie oder Passage | Dämpfer dauerhaft offen |
| Motor an/aus, Tempo | Passage (Tempo als stetige Spur, Übergang gerechnet) | Drehzahl, Hochlauf, Haltewinkel (Ausstattung: offen) |
| Bogen | Note mit **Expression** (Widerspruch W1) | Bogendruck und -geschwindigkeit über die Zeit (dauernde Anregung, Reibmodell) |
| Pitch-Bending | Note mit stetigem Verlauf (Widerspruch W2) | Ort und Kraft des aufgesetzten Schlägels (verschiebt Randbedingung bzw. Masse) |
| Flageolett | Note | Berührpunkt (Mitte) und Anschlag am Knoten |
| Kalimba Wa-Wa | Note mit stetigem Verlauf (Widerspruch W7) | Öffnungsgrad der Löcher (Helmholtz-Resonator) |
| Kalimba Nagel/Fingerkuppe | Passage | Härte und Breite der Anregung beim Zupfen |
| Mbira-Schnarrer | nie (Ausstattung) | Schnarrkontakt an der Platte, an/aus |
| Steeldrum-Wirbel | Note | wie Wirbel, mit nichtlinearer Kopplung zwischen den Tonfeldern |
| Rhodes/Wurlitzer-Tremolo | Ausstattung (vorhanden) plus Passage (Tempo, Tiefe); Wurlitzer: Tempo fest | Amplitudenmodulation bzw. Stereo-Panning nach dem Tonabnehmer, Kurvenform (Rhodes laut Quelle Rechteck) |
| Haltepedal Rhodes/Wurlitzer/Celesta | Passage (Spur) | Einzeldämpfer je Taste abgehoben |

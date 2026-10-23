# Recherche 24f — Saitenabstand je Bauart und Vorgabe „Spread“

Stand 2026-10-19. Ziel: Spread (ms zwischen zwei benachbarten Saiten beim Strummen) je Bauart als
Saitenabstand am Anschlagort ÷ Geschwindigkeit der Hand; die Geschwindigkeit aus Freire 2018
(Nylongitarre, 8 ms) und dem Abstand an genau dieser Gitarre.

Kennzeichnung: **gelesen** = Wortlaut an der Quelle selbst gesehen (PDF mit `quelle_suchen.py`
bzw. `pdftotext`, Webseite im Browser/HTML); **abgeleitet** = gerechnet, Rechnung steht daneben.
Zollangaben umgerechnet mit 25,4 mm/Zoll (abgeleitet). „E–E“ = Mitte äußerste zu Mitte
äußerste Saite; der Abstand benachbarter Saiten ist E–E ÷ (Saitenzahl − 1).

## Quellenliste

| Kürzel | Quelle | URL |
|---|---|---|
| Frei18 | S. Freire, A. Armondes, J. Viana, R. Silva, „Strumming on an Acoustic Nylon Guitar: Microtiming, Beat Control and Rhythmic Expression in Three Different Accompaniment Patterns“, Proc. 15th Sound and Music Computing Conference (SMC 2018), Limassol, S. 544–549 | https://zenodo.org/records/1422685 (PDF `smc_2018_077.pdf`) |
| Barb14 | I. Barbancho, G. Tzanetakis, L. J. Tardón, P. F. Driessen, A. M. Barbancho, „Estimation of the Direction of Strokes and Arpeggios“, 15th International Society for Music Information Retrieval Conference (ISMIR 2014), S. 41–46 | https://archives.ismir.net/ismir2014/paper/000130.pdf |
| Trau01 | C. Traube, J. O. Smith III, „Extracting the Fingering and the Plucking Points on a Guitar String from a Recording“, IEEE WASPAA, New Paltz, 21.–24.10.2001 | https://www.ee.columbia.edu/~dpwe/papers/TraubS01-guitar.pdf |
| YamCG | Yamaha USA, Produktseite CG182S (Specs) | https://shop.usa.yamaha.com/en/p/instruments/guitars-basses-amps/acoustic-guitars/cg182s-classical-acoustic-guitar |
| YamSG | Yamaha Singapur, Produktseite CG182S (Spec-Tabelle CG182C/S, CG192) | https://shop.sg.yamaha.com/cg182s.html |
| YamFG | Yamaha USA, Produktseite FG800 (Specs) | https://shop.usa.yamaha.com/en/p/instruments/guitars-basses-amps/acoustic-guitars/fg800-acoustic-guitar |
| MarD28 | Martin Guitar, D-28 (Standard Series), Specs | https://www.martinguitar.com/guitars/standard-series/D-28.html |
| Mar00 | Martin Guitar, 00-28 und 000-28 (Standard Series), Specs | https://www.martinguitar.com/guitars/standard-series/00-28.html · …/000-28.html |
| Mar0XK | Martin Guitar, 0XK Concert Uke, Specs | https://www.martinguitar.com/ukuleles/concert-body/0XK-Concert-Uke.html |
| GT | Graph Tech (Hersteller), vorgeschlitzte TUSQ-Sättel, Maßtabelle „Str.Sp“ je Produkt | https://graphtech.com/collections/nuts (Einzelseiten unten) |
| SMabr | StewMac, ABR-1 Tune-o-matic Bridge (mit Definition „String spread“) | https://www.stewmac.com/parts-and-hardware/all-hardware-and-parts-by-instrument/electric-guitar-parts/electric-guitar-bridges-and-tailpieces/tune-o-matic-bridges/abr-1-tune-o-matic-bridge/ |
| SMgot | StewMac, Gotoh Traditional Tremolo for Strat | https://www.stewmac.com/parts-and-hardware/all-hardware-and-parts-by-instrument/electric-guitar-parts/electric-guitar-bridges-and-tailpieces/electric-guitar-tremolos/gotoh-traditional-tremolo-for-strat/ |
| SMrw | StewMac, Fender Road Worn Stratocaster Bridge Section Kit | https://www.stewmac.com/parts-and-hardware/all-hardware-and-parts-by-instrument/electric-guitar-parts/electric-guitar-saddles/fender-road-worn-stratocaster-bridge-section-kit/ |
| SMam | StewMac, Fender American Standard Strat Tremolo Bridge Assembly | https://www.stewmac.com/parts-and-hardware/all-hardware-and-parts-by-instrument/electric-guitar-parts/electric-guitar-bridges-and-tailpieces/electric-guitar-tremolos/fender-american-standard-strat-tremolo-bridge-assembly/ |
| F2pt | Fender, Deluxe Series 2-Point Tremolo Assembly (deutsche Länderseite) | https://www.fender.com/en-US/parts/bridges/deluxe-series-2-point-tremolo-assembly/0992079000.html (leitet um auf ch.fender.com/products/deluxe-series-2-point-tremolo-assembly) |
| SCat | Stratcat (Händler), Fender American Vintage Strat Tremolo 009-4247-049 | https://www.stratcat.biz/009-4247-049.shtml |
| SMfish | StewMac, Fishman Archtop Guitar Pickup (Ebenholzsteg mit Piezo) | https://www.stewmac.com/electronics/pickups/fishman-archtop-guitar-pickup |
| SMbass | StewMac, GuitarSlinger Cryocated Aged 4-String Bass Bridge with Grooved Barrel Saddles | https://www.stewmac.com/parts-and-hardware/all-hardware-and-parts-by-instrument/electric-bass-parts/electric-bass-bridges/guitarslinger-cryocated-aged-4-string-bass-bridge-with-grooved-barrel-saddles/ |
| NoTr | D. Stokes, „My Double Bass Setup: Part 3 – String Spacing and String Height“, No Treble, 03.10.2016 — **Kolumne/Blog** | https://www.notreble.com/buzz/2016/10/03/my-double-bass-setup-part-3-string-spacing-and-string-height/ |
| LFvio | Official Luthiers Forum, Thread „Violin Setup Specs“ — **Forum** | https://www.luthiersforum.com/forum/viewtopic.php?p=743972 |

## 1. Klassische Gitarre / Nylon (Freire-Instrument)

Frei18 nennt das Instrument — **gelesen**, PDF-Seite 2 (gedruckt S. 545):
„The hardware consisted of a Yamaha nylon acoustic guitar (model CG182S), equipped with RMC hexaphonic pickups.“
Gespielt wurde mit Plektrum — **gelesen**, PDF-Seite 1: „The three experiments presented in this paper were recorded using a guitar pick“.
Einen Anschlagort nennt Frei18 nicht (gesucht: pick, sound hole, bridge, hole — kein Treffer zum Ort).

| Größe | Wert | Quelle | Art | Zitat |
|---|---|---|---|---|
| Mensur | 650 mm | YamCG | gelesen | „Scale Length 650 mm (25 9/16")“ |
| Sattelbreite | 52 mm | YamCG, YamSG | gelesen | „Nut Width 52 mm (2 1/16")“ |
| **Steg**, benachbart | **11,8 mm** | YamCG, YamSG | gelesen (Ort abgeleitet) | „String Spacing 11.8 mm“ |
| Sattel E–E (Ersatzteil 52,85 mm, klassisch) | 43,2 mm | GT 6200-00 | gelesen | Tabelle „Str.Sp … 1.701” … 43.2“ |
| **Sattel**, benachbart | **8,64 mm** | GT 6200-00 | abgeleitet: 43,2 ÷ 5 | — |
| Anschlagort, benachbart | 11,17 mm | — | abgeleitet (s. u.) | — |

Anmerkungen: Yamaha sagt nicht, *wo* „String Spacing“ gemessen ist. Dass es der Steg ist,
ist abgeleitet: Bei der FG800 steht dieselbe Zeile mit 11 mm neben 43 mm Sattelbreite (YamFG,
**gelesen**: „Nut Width 43 mm (1 11/16") String Spacing 11 mm“) — am Sattel wären 11 mm je
Saite bei 43 mm Breite unmöglich (5 × 11 = 55 mm). Zum Vergleich: 11,8 × 5 = 59 mm E–E am Steg.
Der Sattelwert ist **nicht** von Yamaha, sondern der Graph-Tech-Ersatzsattel für 52-mm-Hälse
(Länge 52,85 mm passt zur CG182S); Graph Tech nennt die Spalte nur „Str.Sp“ ohne Definition
(Mitte–Mitte nicht ausdrücklich gesagt). Weitere klassische GT-Sättel: 6220-00 (50,55 mm) 41,5;
6250-00 (48,1 mm) 40,9; 6260-00 (48,8 mm) 40,6 mm — alle **gelesen**.

## 2. Akustische Steelstring (Dreadnought, Parlor/00/000)

| Modell | Sattel E–E | Sattel benachb. | Steg E–E | Steg benachb. | Quelle | Art |
|---|---|---|---|---|---|---|
| Martin D-28 (25,4" = 645 mm, Sattel 1 3/4") | 38,6 mm (GT M175-00, Martin-Stil 1,75") | 7,72 mm | 2 5/32" = 54,77 mm | 10,95 mm | MarD28, GT | Steg gelesen, Sattel gelesen (GT), benachb. abgeleitet |
| Martin 00-28 / 000-28 (24,9" = 632 mm, 1 3/4") | wie oben | 7,72 mm | 2 5/32" | 10,95 mm | Mar00 | gelesen |
| Yamaha FG800 (Dreadnought, Sattel 43 mm) | — | — | — | 11 mm | YamFG | gelesen (Ort abgeleitet wie oben) |

Zitate: MarD28 „Bridge String Spacing: 2 5/32"“ und „1 3/4'' fingerboard width at nut“;
Mar00 bei beiden Modellen „Bridge String Spacing“ = „2 5/32"“, Mensur „24.9"“; GT M175-00
(„TUSQ Nut … Martin Style Slotted 1.75"“) „Str.Sp 1.52” / 38.6“. Andere Martin-Stil-Sättel:
M644-00 (1 3/4", 45,25 mm lang) 36,0 mm; M600-00 (1 11/16") 34,9 mm — die Schlitzung streut
also um gut 2 mm E–E bei gleicher Breite (**gelesen**).

## 3. E-Gitarre

| Modell | Sattel E–E | Sattel benachb. | Steg E–E | Steg benachb. | Quelle | Art |
|---|---|---|---|---|---|---|
| Strat, Vintage 6-Schrauben-Tremolo (25,5" = 648 mm) | 35,3 mm (GT 5000-00 Strat Style) | 7,06 mm | 2-7/32" = 56,33–56,36 mm | 11,27 mm | SMgot, SMrw, SCat, GT | gelesen / benachb. abgeleitet |
| Strat, modernes 2-Punkt-Tremolo (American Standard, Player, Deluxe) | 35,3 mm | 7,06 mm | 2-1/16" = 52,37 mm | 10,47 mm | SMam, F2pt | gelesen / abgeleitet |
| Gibson Les Paul, Tune-o-matic ABR-1 (24,75" = 629 mm) | 35,8 mm (GT 6010-00 Gibson Style) | 7,16 mm | 2-1/16" = 52,37 mm | 10,47 mm | SMabr, GT | gelesen / abgeleitet |
| Jazz-Archtop, Ebenholzsteg (Fishman-Archtop-Steg) | 35,8 mm (Gibson-Sattel angenommen) | 7,16 mm | 2,125" = 53,98 mm | 10,80 mm | SMfish | gelesen / abgeleitet |

Zitate (**gelesen**): SMabr „String spread 2-1/16"“ und Definition „String spread is the
distance between the centers of the outer strings on a bridge or tailpiece.“ — SMgot „String
spread and mounting screw spread is 2-7/32" (56.36mm).“ — SMrw „String spread and mounting
screw spread is 2-7/32" (56.33mm)“ — SMam „String spread: 2-1/16" (52.37mm) Post spacing:
2-7/32" (56.33mm)“ — F2pt (deutsche Seite) „2-1/16″ String Spacing und 2-7/32″ Bolzenabstand“ —
SCat „String spacing: 2-7/32"“ (Händler, Fender-Teil 009-4247-049) — SMfish „String Spacing:
2.125"“ — GT 5000-00 „Str.Sp 1.39” / 35.3“, GT 6010-00 „Str.Sp 1.409” / 35.8“, GT 6011-00
(Gibson Style XL) 36,0 mm. Die Mensuren 25,5" und 24,75" sind hier **nicht belegt** (Allgemeinwissen),
ebenso die Mensur der Archtop.

Hinweis: StewMac warnt selbst, „Not all vintage-style Strat tremolos have the same string
spread“ (SMrw, **gelesen**).

## 4. E-Bass 4-saitig (Precision / Jazz, 34" = 864 mm)

| Modell | Sattel E–G | Sattel benachb. | Steg E–G | Steg benachb. | Quelle | Art |
|---|---|---|---|---|---|---|
| Precision | 31,65 mm (GT 1204-00 „Bass Fender Precision Style“, 41,88 mm lang) | 10,55 mm | 2-1/4" = 57,15 mm | 19,05 mm | GT, SMbass | gelesen / abgeleitet |
| Jazz | 29,6 mm (GT 1215-00 „XL Fender Jazz Bass“, 38,28 mm lang) | 9,87 mm | 57,15 mm | 19,05 mm | GT, SMbass | gelesen / abgeleitet |

Zitate (**gelesen**): SMbass „Sized for Jazz or Precision bass“ und „String spread 2-1/4"
(57.15mm)“. GT 1204-00 „Str.Sp 1.246” / 31.65“; GT 1215-00 „Str.Sp 1.165” / 29.6“; GT 1214-00
(„Bass Fender Style Jazz“, 39,48 mm lang) 31,32 mm — für Jazz habe ich 1215-00 genommen, weil seine
Länge zum 1,5"-Sattel (38,1 mm) passt (abgeleitet). Die Mensur 864 mm ist nicht belegt.

## 5. Kontrabass

| Größe | Sattel benachb. | Steg benachb. | Quelle | Art |
|---|---|---|---|---|
| „traditionelle“ Einrichtung | 10 mm | 28 mm | NoTr (**Blog/Kolumne**) | gelesen |
| Stokes' eigene, enge Einrichtung (5-Saiter) | 8 mm | 22 mm | NoTr | gelesen |
| Strobel, *Useful Measurements for Violin Makers*, 4/4-Bass: E–G 33 mm Sattel, 87 mm Steg | (11 mm) | (29 mm) | nur über Suchwerkzeug-Zusammenfassung eines MIMF-Forenbeitrags | **nicht gelesen** — Seite hinter Cloudflare-Prüfung, nicht umgangen |

Zitat NoTr: „By contrast a more traditional setup might be 10 mm at the nut and 28 mm at the
bridge.“ Mensur des Kontrabasses nicht belegt (für die Rechnung unerheblich, nur der Anteil zählt).

## 6. Ukulele (Konzert, 381 mm)

| Größe | Wert | Quelle | Art | Zitat |
|---|---|---|---|---|
| Steg E–A (A–G) | 1 41/64" = 41,67 mm | Mar0XK | gelesen | „Bridge String Spacing: 1 41/64''“ |
| Steg benachb. | 13,89 mm | — | abgeleitet: 41,67 ÷ 3 | — |
| Sattel E–E, GT 1257-00 „Ukulele Slotted“ (35,7 mm lang) | 28,85 mm → 9,62 mm benachb. | GT | gelesen / abgeleitet | „Str.Sp 1.136” / 28.85“ |
| Sattel E–E, GT 1237-00 „Slotted Uke“ (37,45 mm lang) | 30,2 mm → 10,07 mm benachb. | GT | gelesen / abgeleitet | „Str.Sp 1.189” / 30.2“ |
| weitere GT-Uke-Sättel | 1253-00: 26,5; 1234-00: 27,2; 1243-00 (Kamaka): 27,9 mm | GT | gelesen | — |

Welche Sattelbreite die Martin 0XK hat, steht auf der Seite nicht (gesucht: „Nut“, „Width“ — nur
„Nut Material“). Die Konzert-Mensur 381 mm ist hier nicht belegt.

## 7. Violine (Pizzicato-Akkord)

| Größe | Wert | Quelle | Art | Zitat |
|---|---|---|---|---|
| Steg G–E | 33–34 mm → benachb. 11,0–11,33 (Mitte 11,17) mm | LFvio (**Forum**) | gelesen / abgeleitet ÷ 3 | „Bridge string spacing G - E 33-34mm“ |
| Sattel G–E | 16–16,5 mm → benachb. 5,33–5,5 (Mitte 5,42) mm | LFvio (**Forum**) | gelesen / abgeleitet ÷ 3 | „Nut spacing 16 - 16.5mm“ |

Eine Herstellerangabe oder Literaturstelle mit Wortlaut habe ich nicht gefunden (Strobel nur als
Buchtitel; violinist.com-Archiv war nicht abrufbar — Verbindungsabbruch).

## Anschlagort

- Trau01, **gelesen**, PDF-Seite 1: „The neutral position of the right hand is just behind the sound hole.“
  und „the low strings are usually plucked further away from the bridge than the higher ones.“
- Trau01, **gelesen**, PDF-Seite 3–4: die Messbeispiele an der klassischen Gitarre liegen bei
  „Plucking point distance from the bridge D = 12, 13, 14 cm“ — das sind gewählte Prüfabstände,
  **keine** Messung, wo Spieler typisch anschlagen.
- Für E-Gitarre, Bass, Ukulele, Kontrabass und Violine habe ich **keine** Quelle mit Abstand vom
  Steg beim Strummen/Pizzicato gefunden.

Annahme für die Rechnung (abgeleitet): Anschlag bei einem Fünftel der Mensur vom Steg
(13 cm ÷ 650 mm = 0,20 — Trau01-Mitte an der Nylongitarre), für alle Bauarten gleich. Abstand
am Anschlagort linear zwischen Steg (Anteil 0) und Sattel (Anteil 1):

  s(a) = s_Steg − (s_Steg − s_Sattel) · a, mit a = 0,2.

Empfindlichkeit: Der Unterschied Steg–Sattel ist klein; bei der Nylongitarre ändert a = 0,15
statt 0,2 den Abstand um 0,16 mm (1,4 %). Der Anschlagort ist also nicht die große Unsicherheit.

## Handgeschwindigkeit

- Frei18, Table 6, **gelesen**, PDF-Seite 5 (gedruckt S. 547): Zeile „6-note / strummed adagio
  (Excerpt 1)“ mit „Intertone spread int. (ms)“ 8–11 und „Global spread int. (ms)“ 37–46. Die
  Tabelle fasst Mittelwerte zusammen („Table 6. Mean values for intertone and global spread
  intervals, in different chord types.“). Weitere Zeilen derselben Tabelle: 6-note strummed
  moderate (Excerpt 3) 5–12,5 / 26–62; 6-note arpeggio moderate (Excerpt 2) 16–40 / 64–202;
  Blockakkorde aus [2] 0–6 bzw. 0–5,5 ms.
- Barb14, **gelesen**, PDF-Seite 3 (gedruckt S. 43), Abschnitt 2.2: „The key observation is that
  the time difference between the note onsets that belong to the same stroke or arpeggio will be
  approximately constant. The reason is that, because of human physiology, in most cases the
  stroke is performed with fixed speed.“ Die Venue ist ISMIR 2014 (Kopfzeile „15th International
  Society for Music Information Retrieval Conference (ISMIR 2014)“). Barb14 belegt die
  Annahme nur als Begründung, nicht mit eigener Messung; ihre künstlich zusammengesetzten
  Akkorde haben „between 1000 samples (11 ms) and 5000 samples (55 ms)“ (S. 44).

Rechnung (abgeleitet):
- Abstand an der Freire-Gitarre am Anschlagort: 11,8 − (11,8 − 8,64) · 0,2 = **11,17 mm**.
- v = 11,17 mm ÷ 8 ms = **1,40 m/s** (untere Zeitgrenze, schnellste Hand);
  bei 11 ms: 11,17 ÷ 11 = 1,02 m/s.
- Damit Spread_X = s_X ÷ v = 8 ms · s_X ÷ 11,17 mm (bei 11 ms: Faktor 11/8 darauf).

## Abgeleitete Spread-Werte je Bauart (gerundet auf 0,5 ms)

| Bauart | s_Steg | s_Sattel | s(0,2) | Spread (8 ms-Basis) | gerundet | bei 11 ms-Basis | Hauptquelle |
|---|---|---|---|---|---|---|---|
| Klassik/Nylon (Yamaha CG182S) | 11,80 | 8,64 | 11,17 | 8,00 | **8,0** | 11,0 | Frei18, YamCG, GT 6200 |
| Steelstring Dreadnought / 00 / 000 (Martin) | 10,95 | 7,72 | 10,31 | 7,38 | **7,5** | 10,2 | MarD28, Mar00, GT M175 |
| Strat Vintage (2-7/32") | 11,27 | 7,06 | 10,42 | 7,47 | **7,5** | 10,3 | SMgot/SMrw, GT 5000 |
| Strat modern (2-1/16") | 10,47 | 7,06 | 9,79 | 7,01 | **7,0** | 9,6 | SMam/F2pt, GT 5000 |
| Les Paul (ABR-1) | 10,47 | 7,16 | 9,81 | 7,03 | **7,0** | 9,7 | SMabr, GT 6010 |
| Jazz-Archtop | 10,80 | 7,16 | 10,07 | 7,21 | **7,0** | 9,9 | SMfish, GT 6010 |
| E-Bass Precision | 19,05 | 10,55 | 17,35 | 12,43 | **12,5** | 17,1 | SMbass, GT 1204 |
| E-Bass Jazz | 19,05 | 9,87 | 17,21 | 12,33 | **12,5** | 17,0 | SMbass, GT 1215 |
| Kontrabass (traditionell, NoTr) | 28 | 10 | 24,40 | 17,48 | **17,5** | 24,0 | NoTr (Blog) |
| Kontrabass (Strobel, nicht gelesen) | 29 | 11 | 25,40 | 18,19 | (18,0) | 25,0 | — |
| Ukulele Konzert (Martin 0XK, GT 1257) | 13,89 | 9,62 | 13,04 | 9,34 | **9,5** | 12,8 | Mar0XK, GT 1257 |
| Ukulele Konzert (GT 1237) | 13,89 | 10,07 | 13,13 | 9,40 | (9,5) | 12,9 | Mar0XK, GT 1237 |
| Violine | 11,17 | 5,42 | 10,02 | 7,18 | **7,0** | 9,9 | LFvio (Forum) |

Alle Werte in mm bzw. ms; alle Spalten außer s_Steg/s_Sattel sind abgeleitet.

## Offene Lücken

1. **Ort von Yamahas „String Spacing“** ist nicht ausgesprochen; Steg nur abgeleitet (FG800-Vergleich).
2. **Sattelwerte** stammen durchweg von Graph-Tech-Ersatzsätteln, nicht vom Originalinstrument;
   Graph Tech definiert „Str.Sp“ nicht (Mitte–Mitte vermutet). Für Martin, Fender, Gibson, Yamaha,
   Kala habe ich keine Herstellerangabe zum Sattel-E–E gefunden.
3. **Anschlagort** ist nur für die klassische Gitarre grob gestützt (Trau01: „just behind the sound
   hole“, Prüfabstände 12–14 cm); für alle anderen Bauarten ist a = 0,2 eine Annahme. Wirkung klein.
4. **Gleiche Handgeschwindigkeit über Bauarten** ist eine Annahme: Barb14 begründet nur „feste
   Geschwindigkeit innerhalb eines Schlags“, nicht gleiche Geschwindigkeit an Bass, Ukulele oder
   beim Pizzicato der Violine (dort meist Daumen oder Finger statt Plektrum). Frei18 misst nur Nylon
   mit Plektrum und nur 6-saitig; „adagio“ ist Excerpt 1, die schnellere Zeile „moderate“ reicht bis 5 ms.
5. **Kontrabass und Violine** nur mit Blog- bzw. Forumsquelle; Strobels Buch (Literatur) nicht
   eingesehen, der MIMF-Beitrag dazu stand hinter einer Bot-Prüfung.
6. **Mensuren** von Strat, Les Paul, Archtop, E-Bass, Kontrabass, Ukulele sind hier nicht belegt
   (für die Rechnung unerheblich, weil nur der Anteil a eingeht).
7. **Archtop**: der Wert ist der eines Fishman-Pickup-Stegs, kein Gibson-/Eastman-Werkssteg; ein
   Archtop mit Tune-o-matic läge bei 10,47 mm wie die Les Paul (Spread ebenfalls 7,0).

Reichweite der Suche: Freire-PDF ganz (Begriffe Table 6, CG182S, adagio, pick, sound hole,
bridge, 18 ms); Barbancho-PDF (physiology, constant, speed, ms); Traube-PDF (sound hole, neutral,
cm, from the bridge); Yamaha CG182S/FG800/LL16; Martin D-28/00-28/000-28/0XK; Fender-Teileseiten
(meist 403/404, eine deutsche Länderseite lesbar); StewMac-Steg- und Tremoloseiten; Graph-Tech-
Sattelkatalog (rund 25 Produkte ausgelesen); Websuchen zu klassischem Sattel/Steg (nur Delcamp-Forum,
nicht verwendet), Ukulele-Steg (Kala ohne Angabe gefunden), Kontrabass und Violine (nur Foren/Blog).
Nicht gefunden: Sattel-Spezifikationen der Hersteller, Anschlagort außerhalb der Nylongitarre,
eine Literaturstelle (statt Forum) für Violine und Kontrabass. Kein Treffer ist kein Beweis.

> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_05_oud_laute_saiten_bau` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 21:06–21:15, claude-opus-5-5, 75 Runden, success, 3.25 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S202:**
> - **Das PDF von Inanli & Altinsoy 2018 liegt nirgends:** der Lauf hat es mit einem eigenen Skript (`pdf_lesen.py`,
>   `urllib`) in eine temporäre Datei geladen und danach gelöscht; es blieben nur die zwei Seitenbilder im Archiv der
>   Werkbank (`seiten/inanli2018-1.png`, `-2.png`). Seitdem verbietet die `CLAUDE.md` der Werkbank das (Entscheid 2 a).
>   Ebenso sind **alle Händler- und Herstellerseiten** (Saitensätze, Dusepo, Van Edwards, Schreiner, luth.org) mit
>   `seite_lesen.py` roh gelesen und **nicht abgelegt** — der Wortlaut lässt sich nicht mehr nachprüfen.
> - **Inanli & Altinsoy 2018 an den zwei Seitenbildern:** S. 923 — Mensuren 610–620 / 585 mm, Dicke 1,5–2,0 mm, diese
>   Decke 2,0 mm, 490 × 360 mm, Antwortpunkt (−40, +180) mm, 317 Punkte, 1,22 Hz — stimmt; das Holz heißt **„Borçka
>   spruce“** (Ort Borçka), nicht „Boróka“. S. 924 — Tab. 1 (alle sieben Zeilen, auch h1 = 5 mm bei Balken 3), Abb. 2
>   (85 / 42 / 160 / 80 / 490 / 360 mm), die acht Frequenzen 31 … 347 Hz, „three soft foam elements“ — stimmt. Die
>   Zuordnung der Moden (m, n) steht auf S. 925 und ist **nicht** geprüft (kein Seitenbild).
> - **Zitate in den lokalen PDFs gefunden:** Woodhouse & Lynch-Aird 2019 (Umspinnung, trockene Reibung), Woodhouse
>   u. a. 2012 („multifilament cores with metal windings“), Abdallah u. a. 2026 (Değirmenli, „brace tuning“).
> - **Nicht gegengelesen:** Değirmenli 2017, alle Web- und Händlerangaben (siehe oben), die Laute-Abschnitte, die
>   Freesound- und Commons-Angaben.
> - **Nacharbeit:** Auftrag `2026-11-17_25_oud_laute_nacharbeit` der Werkbank (Quellen neu lesen und ablegen).
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_05_oud_laute_saiten_bau.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_05_oud_laute_saiten_bau/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_oud_laute_saiten_bau_werkbank/`: `pdf_lesen.py`, `seite_lesen.py`.

# Bericht: Baumaße, Besaitung und Messgrößen von Oud und Renaissancelaute

- **Auftrag:** `auftraege/2026-11-16_05_oud_laute_saiten_bau.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5
- **Reichweite:** 23 Netzsuchen (`WebSearch`, 2 davon „extended“), rund 25 direkte Abrufe von Seiten und PDFs
(einschließlich der fehlgeschlagenen). In diesem Lauf
  gab es kein `WebFetch`, deshalb habe ich Seiten mit `python ergebnisse/…/seite_lesen.py` (HTML → Text, nichts
  gespeichert) und PDFs mit `python ergebnisse/…/pdf_lesen.py` gelesen. Das zweite Skript legt das PDF nur in eine
  temporäre Datei, liest es mit `pdftotext` und löscht es danach. Unter `abrufe/` liegt deshalb **nichts** aus diesem
  Lauf. Seitenbilder liegen unter `seiten/`. Dazu kamen 4 lokale PDFs aus `…\cmake\quellen\gezupfte-saite\`.
- **Nicht erreicht:**
  - YÖK Açık Bilim (Dissertation Değirmenli 2018): Verbindung vom Server getrennt;
  - IOA-PDF Richardson „Acoustics of historical guitars“: HTTP 404;
  - mikeouds.com: Name nicht auflösbar;
  - metmuseum.org: HTTP 429;
  - emirdegirmenli.com: Zertifikat abgelaufen, nicht umgangen;
  - Lundberg, „Historical Lute Construction“ und die Reihe in American Lutherie: nur der Abstract frei, der Rest hinter
    der Bezahlschranke;
  - Hellwig 1968 (Galpin Society Journal) und Nurse 1986: nicht frei.

**Worauf aufgebaut wird (nicht wiederholt):** Woodhouse & Lynch-Aird 2019 (Tab. I/II), D'Addario EJ95A, Abdallah u. a.
2026 (Meccanica 61:89; Abb. 3, 6, Tab. 1–3), Woodhouse 2021 (JASA 150, Abb. 1/5) und Santa Maria Bouquet 2020 (Rauwolf
CL.93). Nachgeprüft habe ich nur eines: Abdallah 2026 nennt **keine Deckendicke**. `pdf_suche.py` findet „thickness“
nur in Tab. 2 (Balken), auf S. 3 allgemein und auf S. 21 („the specific scale, plate thickness, and material of the oud
modeled here“, ohne Zahl). Das bestätigt S195 und S196.

---

## 1 Oud: gemessene Baumaße

### 1.1 Inanli & Altinsoy 2018: die einzige gefundene vollständige Decke mit Maßen

S. Inanli, E. Altinsoy, „Vibroacoustics of Oud“, DAGA 2018 München, S. 923–925, TU Dresden.
URL: https://pub.dega-akustik.de/DAGA_2018/data/articles/000581.pdf (678 476 Bytes). Text mit `pdftotext` gelesen,
S. 924 (Abb. 2, Tab. 1, Abb. 4–6) als Bild geprüft (`seiten/inanli2018-2.png`).

**Instrument:** eine **türkische** Oud, und zwar **nur die Decke mit Balken, frei gelagert** („supported by three soft
foam elements“). Sie ist nicht in einen Korpus eingebaut. Ein Erbauer ist nicht genannt.

| Größe | Wert | Marke / Ort |
|---|---|---|
| Mensur arabische Oud | 610–620 mm | [wörtlich] S. 923, Einleitung |
| Mensur türkische Oud | 585 mm | [wörtlich] S. 923 |
| Deckendicke allgemein | „varying between 1.5-2.0 mm“ | [wörtlich] S. 923 |
| Deckendicke dieser Decke | „uniform thickness of 2.0 mm“; „can decrease around to 1.5 mm“; „Decreasing thickness profile may also be applicable from bridge location to the edge“ | [wörtlich] S. 923 |
| Holz | Decke und Balken aus „Boróka spruce (from North-eastern of Turkey)“ | [wörtlich] S. 923; Kennwerte nicht genannt |
| Decke Länge × Breite | 490 × 360 mm | [wörtlich] S. 923 und Maßzahlen in Abb. 2 |
| Schalllöcher | eines Ø 85 mm, zwei Ø 42 mm; die kleinen 160 mm auseinander (Mitte zu Mitte), 80 mm unter der Mitte des großen | Maßzahlen in Abb. 2 [wörtlich am Bild]; dass sich „80 mm“ auf den senkrechten Abstand der Mitten bezieht, ist [eigene Lesart] |
| Lage der Löcher über dem unteren Ende | großes ≈ 29–30 cm; kleine ≈ 21–22 cm, ±8 cm | [eigene Ablesung] aus dem Raster in Abb. 4 (Teilung 2, beschriftet „mm“, gemeint sind nach der Länge 0–50 cm), ±1 cm |
| Steg | als Rechteck in Abb. 4 bei ≈ 8–10,5 cm über dem unteren Ende | [eigene Ablesung], grob; Länge aus dem Bild nicht sicher. Masse und Maße nicht angegeben |
| Balken | sieben, parallel, quer zu den Saiten | [wörtlich] S. 923 |
| Gemessene Moden der freien Decke | 31 (0,2) / 51 (1,1) / 69 (0,3) / 115 (1,2) / 123 (0,4) / 198 (1,3) / 292 (0,5) / 347 (2,3) Hz | [wörtlich] S. 924 und Abb. 7 (m = Halbwellen quer, n = längs) |
| Messung | Hammer an 317 Punkten im 20-mm-Raster, Antwortpunkt (−40 mm, +180 mm), 0–350 Hz, Auflösung 1,22 Hz | [wörtlich] S. 923–924 |

**Tab. 1 „Fan-Brace Dimensions“** [wörtlich, S. 924 am Bild geprüft]. Laut Abb. 3 ist L1 die ganze Länge, L2 die Länge
mit voller Höhe, L3 die Länge der Verjüngung an jedem Ende, h1 die Höhe in der Mitte, h2 die Höhe am Ende und w die
Breite:

| Balken | L1 (mm) | L2 (mm) | L3 (mm) | h1 (mm) | h2 (mm) | w (mm) |
|---|---|---|---|---|---|---|
| 1 | 300 | 200 | 50 | 12 | 6 | 5 |
| 2 | 355 | 255 | 50 | 12 | 6 | 6 |
| 3 | 355 | 255 | 50 | 5 | 6 | 5 |
| 4 | 344 | 244 | 50 | 12 | 6 | 5 |
| 5 | 118 | 48 | 35 | 12 | 6 | 5 |
| 6 | 278 | 178 | 50 | 12 | 6 | 6 |
| 7 | 222 | 122 | 50 | 12 | 6 | 6 |

**Die Lage der Balken steht nicht als Zahl in der Arbeit.** Abb. 2 zeigt sie nur als schräge 3D-Ansicht mit den Nummern
1–7, ohne Maßstab. Balken 1 liegt am unteren Ende, Balken 5 ist der kurze am großen Loch, 6 und 7 liegen darüber. Mehr
lässt sich nicht ablesen; Abb. 4 zeigt keine Balken. Bei Balken 3 ist h1 = 5 mm kleiner als h2 = 6 mm. So steht es in
der Tabelle, und es ist nicht erklärt.

### 1.2 Weitere Fundstellen zur Oud

| Quelle | Inhalt | Marke |
|---|---|---|
| Değirmenli 2017, GU J Sci Part C 5(2):23–35, https://dergipark.org.tr/en/download/article-file/840569 | Sieben **türkische** Ouds mit Hammer am Steg angeregt (Bass, Mitte, Diskant), dazu Schallabstrahlung und LTAS; Ergebnis nur als Kurven in Terzbändern (Şekil 6, 7, 9, 10). Keine Baumaße, keine Frequenztabelle; Tab. 1 enthält nur Korrelationskoeffizienten | [wörtlich] S. 4–5, 11 |
| Değirmenli 2014 (Masterarbeit) und 2018 (Dissertation), Gazi Üniversitesi | Freie Decke mit sechs Balkenhöhen; Stegmobilität und Modalanalyse an Ouds, FE-Modell | [nur Zusammenfassung], YÖK nicht erreicht |
| Abdallah u. a. 2026 | zitiert Değirmenli, „Soundboard & brace tuning“, emirdegirmenli.com | [wörtlich] Lit. 25; die Seite war nicht erreichbar |
| Dusepo, „Oud (‚Daramad‘ model)“, https://dusepo.co.uk/Instruments/Show/Oud%20('Daramad'%20model) | Nach den Ouds der Familie Nahat; Mensur **58,5 cm**, wahlweise 60 cm | [wörtlich] Herstellerseite |
| Forum „Mike's Oud Forums“ (tid 1453, 6219 u. a.) | Nahat-Ouds: Mensur 60,9 cm, Korpuslänge 49 cm, Breite 34,5–36 cm; Decken 1,5–2,2 mm; Balken 8–18 mm hoch, 4–9 mm breit | [nur Zusammenfassung], Seite nicht erreichbar, nicht zitierfähig |
| „Tuning the structural eigenfrequencies of an oud guitar…“, ResearchGate 351335073 | FE-Rechnung mit Decken von 1,27 / 2,29 / 3,30 mm | [nur Zusammenfassung]; ein Modell, keine Messung |
| World Scientific, IJSSD 2026, doi 10.1142/S0219455426500392 | Optimierung von Loch, Dicke und Balken einer Oud | [nur Zusammenfassung], hinter der Bezahlschranke |

**Iranische Oud (Barbat):** nichts mit Maßen gefunden.

## 2 Oud: Besaitung und Spielweise

### 2.1 Saitensätze (Händlerseiten direkt gelesen)

| Satz | Stimmung (tief → hoch, wie angegeben) | Durchmesser (mm, in der Reihenfolge der Quelle) | Werkstoff | Zug | Marke / URL |
|---|---|---|---|---|---|
| Aquila Red Series Arabic (Nr. 13O) | cc gg dd AA FF C | 0.56, 0.74, 0.63w, 0.73w, 0.84w, 1.25w | Diskant „Sugar“ (Bioplastik aus Zuckerrohr); Bässe mit rotem Kupfer umsponnen; dritter Chor mit „grey metal blend“ umsponnen | „standard tension (3.4 Kg average for each string)“ | [wörtlich] thomann.es/aquila_new_nylgut_arabic_oud_strings.htm; stringsandbeyond.com/aquila13oud.html |
| Aquila Red Series Turkish | dd aa ee BB AA E | 0.52, 0.70, 0.60w, 0.71w, 0.80w, 1.09w | wie oben | „3.4 kg average for each string“ | [wörtlich] thomann.co.uk/aquila_new_nylgut_turkish_oud_strings.htm |
| Pyramid Super-Aoud, 11 Saiten (Thomann 225300) | c' g d A G C | 0.70 - 0.90 - 0.57 - 0.73 - 0.76 - 1.10 | „high-strength nylon silk“ als Kern; d umsponnen („d-3 roundwound“) | nicht angegeben; „Scale: 60 - 62 cm“ | [wörtlich] thomannmusic.com/pyramid_super_aoud_strings_11strings.htm |
| Kürschner Nr. 401104 | c'c' gg dd AA FF C | nicht angegeben | c'c', gg Fluorcarbon; dd–C „nylon fibre wound with silver-plated copper“ | „Medium tension“ | [wörtlich] thomannmusic.com/kuerschner_oud_strings_set_no._401104.htm |
| Kürschner Carbon, light | c' g d A G C | c' 0.54, g 0.66; d/A/G/C nur als Codes „2VNG 5100 / 5136 / 5150“, „1VNG 5230“ | c', g „PVF“ (Fluorcarbon) | „light“ | [wörtlich] orientalinstruments.com/…/kuerchner-carbon-set-light-tension. Die Codes sind nicht erklärt |
| La Bella OU80A | F A d g c' f' (schwebender Steg) | nicht angegeben | g blank Nylon | „LaBella does not provide tension information … approx. 4 kg on a standard Arabic oud“ | [wörtlich] orientalinstruments.com/…/labella-ou-80a; Schätzung des Händlers |
| Kürschner, allgemein | – | – | – | Sätze sind „für 60 cm berechnet“; Zugtabelle nur auf Anfrage beim Händler; gemessen 3,7 kg an 61,5 cm | [nur Zusammenfassung] Forum tid 13799, nicht erreicht |

- [eigene Rechnung] 3,4 kgf = 33,4 N je Saite (Mittelwert über den Satz, Aquila). Die Mensur, auf die sich dieser Wert
  bezieht, nennt Aquila nicht.
- Bei Pyramid steht nicht, welcher Durchmesser zu welchem Chor gehört. Dass die Reihenfolge der Stimmung „c' g d A G C“
  folgt, ist [eigene Lesart]. Sie ist plausibel, weil der letzte Wert (1.10) der dickste ist.

### 2.2 Ist der Basschor einzeln?

Die Sätze selbst geben die Antwort, nicht nur das Forum:

- Aquila arabisch: „cc; gg; dd; AA; FF; C“ [wörtlich];
- Kürschner 401104: „c'c' - gg - dd - AA - FF - C“ [wörtlich];
- Kürschner 416104: „F'F' - C'C' - GG - DD - AA - FF - C“ [wörtlich, Titel der Thomann-Liste];
- Pyramid: „Tuning c'g d A G C (double string = 5 choirs) … Low C-string is separate“ [wörtlich]. Elf Saiten sind also
  fünf Doppelchöre und ein einzelnes C [eigene Rechnung].

In allen gelesenen Elf-Saiten-Sätzen ist der tiefste Chor **eine einzelne Saite**. Laut Forum wird er gelegentlich
doppelt besaitet, das ist aber selten [nur Zusammenfassung].

### 2.3 Zupfen mit der Risha

Nicht gefunden: gemessener Zupfort, Auslenkung, Kraft oder Winkel. Gefunden habe ich nur „Rishas are typically
100–180 mm long and 6–10 mm wide“ (oudforguitarists.com) [nur Zusammenfassung] und die qualitative Aussage, dass der
Abstand zum Steg und der Winkel den Klang prägen (wiki.ubc.ca, Phys341 2020/Oud) [nur Zusammenfassung].

## 3 Laute: Bau historischer Instrumente

| Quelle | Inhalt | Marke |
|---|---|---|
| Schreiner, „The Soundboard Barring for the Jauch Baroque Lute“, 11.08.2012, https://schreinerlutesandguitars.blogspot.com/2012/08/the-soundboard-barring-for-jauch.html | **Historische Laute von Jauch** (Barock), durch Risse von I. Magherini teilweise eingesehen: „The first bar is 32mm in front of the bridge (measurements are taken from the middle of each bar). The second bar is 76mm from the first. The third bar is 59mm from the second. The fourth bar is 50mm from the third. This bar is the center of the rose.“ „the front of the bridge is 101mm from the rear of the belly.“ Ein Balken läuft vom Rand unter die Diskantspitze des Stegs, kein J-Balken. Oberhalb der Rose nicht einsehbar | [wörtlich] |
| derselbe, Nachbau (moderne Laute des Autors, **nicht** das Original) | Decke als Beispiel 0,074 in ≈ 1,9 mm; Feld über der Rose 2,0–2,1 mm; erster „finger bar“ 7,5 mm hoch, 3,9 mm breit; erster „harmonic bar“ 15,8 mm hoch, 4,2 mm breit, am Span 6 mm | [wörtlich] |
| Van Edwards, „Lute structure“, https://vanedwards.co.uk/structure.htm | „Most surviving lute soundboards are quite thin, often about 1.5mm“; die frühesten (um 1540) vielleicht dicker, später dünner mit mehr Balken (nach Nurse 1986); Späne „typically about 1.5mm“; Steg aus Pflaume, Birne oder Nussbaum | [wörtlich] |
| FEMTO-ST 2019, Folien „Model-based decision support … heritage musical instruments“, https://publiweb.femto-st.fr/tntnet/entries/18458/documents/author/data | FE-Modell der **Laux-Maler-Laute** (Musée de la musique, auf den Folien „E.2006.3.1“, anderswo E.2005.3.1): gerechnete Deckenmoden 212 / 239 / 244 / 266 / 324 Hz (S. 21; S. 19 nennt 212 / 243 / 244 / 266 / 328). Holz je Teil (Fichte, Esche, Birne, Ebenholz, Linde) mit Kennwerten auf S. 15. Dicken nur als Eingangsgrößen ohne Wert. **Gerechnet, nicht gemessen** | [wörtlich] Folien 15, 19, 21; die Tabelle auf S. 15 nicht gegengeprüft, `pdftotext` verwürfelt die Spalten |
| Lundberg, „The Erlangen Lectures Day Three: The Belly“, AL #14 S. 22 / HLC S. 30, https://search.luth.org/the-erlangen-lectures-day-three-the-belly | Nur der Abstract: „Many drawings illustrate the shape and thickness of historical lute tops.“ Die Zahlen stehen im Heft oder Buch | [wörtlich] Abstract; Inhalt hinter der Bezahlschranke |
| Barber & Harris, lutesandguitars.co.uk/htm/cat.htm | „complete survey of thicknesses and bar placement“ einer Maler-Laute (1529) | [nur Zusammenfassung], Seite nicht gelesen |
| NMM Vermillion, Technische Zeichnung Diskantlaute um 1500 (Santa Maria Bouquet 2009), shop.nmmusd.org | käuflich; ob Balken und Dicken eingezeichnet sind, ist offen | [nur Zusammenfassung] |

**Vergleich mit dem Ersatz-Bauplan der Laute** [eigene Rechnung]. Abstand der Balkenmitten zur Vorderkante des Stegs,
ungestreckt:

| | Balken 1 | Balken 2 | Balken 3 | Rose (Mitte) |
|---|---|---|---|---|
| Bauplan S197 (128 / 190 / 228 / 287,4 mm über dem Ende, Steg 89 mm) | 39 mm | 101 mm | 139 mm | 198 mm |
| Jauch (Schreiner) | 32 mm | 108 mm | 167 mm | 217 mm |

Ob Schreiner bei „in front of the bridge“ von der Vorderkante oder der Mitte des Stegs misst, steht nicht da. Die
Jauch-Laute ist eine größere Barocklaute: ihr Steg liegt 101 mm über dem Ende, bei Rauwolf sind es 89 mm. Die Reihe ist
also nur eine **Ordnung** (Balken 1 nahe am Steg, Balken 2 ungleich geteilt, ein Balken durch die Mitte der Rose), keine
übertragbare Maßreihe.

**Moden oder Stegadmittanz einer Laute außer Woodhouse 2021: nicht gefunden.** Die Laux-Maler-Folien sind gerechnet.
Richardson („Acoustics of historical guitars“, IOA) war nicht erreichbar (404).

## 4 Gemessene Einzeltöne und Aufnahmen

- **T60 oder Abklingrate je Teilton für Oud oder Laute: nicht gefunden.**
- Richardson, IOA: eine moderne Oud aus Bahrain gegen eine Vihuela; beim Zupfen mit der bloßen Fingerkuppe ohne Bund
  klingen die tiefen Teiltöne „two to three times faster“ ab [nur Zusammenfassung, PDF nicht erreicht].
- Değirmenli 2017 enthält nur LTAS-Kurven in Terzbändern (§1.2).

**Aufnahmen mit Lizenz** (nur genannt, nichts geladen):

| Aufnahme | Umfang / Format | Lizenz | Marke |
|---|---|---|---|
| Wikimedia Commons, „File:Oud open Strings.ogg“ (User:Bottomline, 2006), https://commons.wikimedia.org/wiki/File:Oud_open_Strings.ogg | 5,75 s, Ogg, 61 250 Bytes; „Open strings of the Oud (common Arab tuning)“ | „CC BY-SA 3.0“ | [wörtlich] Commons-API (extmetadata) |
| Freesound, Benutzer „hammondman“, 30.12.2012: `a2.wav`, `g2.wav`, `c3.wav` („real oud sample“ / „oud sample“) | einzelne Töne A2, G2, C3; WAV | Treffer unter dem Filter `license:"Creative Commons 0"` | [wörtlich] Trefferliste; die einzelnen Seiten nicht geöffnet, Größe und Lizenztext dort prüfen |
| Freesound, „MG_NYC“, `Oud_A2a.wav` (17.11.2015) | aus dem hammondman-Ton, leiser gemacht | wie oben, CC0-Filter | [wörtlich] Trefferliste |
| Freesound, „tarane468“, Pack „Arabic Oud“ (10 Töne) | „found this sound somewhere in the net as a free sound for Yamaha Motif“ | CC0-Filter, **Herkunft unklar** | [wörtlich]; nicht empfohlen |
| Laute: Freesound CC0 | nur „Medieval Lute Chords“ (gebrochene Akkorde) | CC0 | [wörtlich]; keine Einzeltöne |

Datensätze mit Einzeltönen (TinySOL, Good-sounds, OrchideaSOL) enthalten laut Zusammenfassung **keine** Oud oder Laute
[nur Zusammenfassung].

## 5 Verluste umsponnener Saiten

- **Für Seiden- oder Nylonkern bei Oud und Laute keine Messung gefunden.**
- Die heute übertragene Gitarrenquelle ist Woodhouse u. a. 2012 (Acta Acustica 98), Tab. I, S. 480. Sie liegt lokal und
  ist in LumiViz schon benutzt (S195, S199). Ich habe sie am Bild gegengelesen; die Werte stehen hier nur, damit die
  Lücke benannt ist. Es sind D'Addario Pro Arté „Composites, hard tension“, die drei tiefen Saiten „multifilament cores
  with metal windings“ [wörtlich]:

| Saite | f (Hz) | T (N) | m (g/m) | EI (10⁻⁶ N m²) | ηF (10⁻⁵) | ηB (10⁻²) | ηA (1/s) |
|---|---|---|---|---|---|---|---|
| 4 D3 | 146,8 | 71,2 | 1,95 | 51 | 5 | 2,0 | 1,2 |
| 5 A2 | 110,0 | 73,9 | 3,61 | 40 | 7 | 2,5 | 0,9 |
| 6 E2 | 82,4 | 71,6 | 6,24 | 57 | 2 | 2,0 | 1,2 |

- Woodhouse & Lynch-Aird 2019, S. 528: „tight windings introduce a new damping mechanism associated with dry friction,
  whereas an open winding eliminates this effect“ (nach Valette) [wörtlich]. Das zeigt, dass Verluste umsponnener Saiten
  vom Aufbau abhängen. Eine Zahl gibt es dort nicht.

## Reicht das für …?

| Annahme heute | Stand nach den Funden | Was Annahme bliebe |
|---|---|---|
| **Oud: Mensur 0,60 m** | **teilweise.** Belegte Spanne: arabisch 610–620 mm, türkisch 585 mm (Inanli 2018); Hersteller 58,5 / 60 cm (Dusepo, nach Nahat); Saitensätze für 60–62 cm (Pyramid) | die Mensur des Abdallah-Instruments selbst (nicht angegeben) |
| **Oud: Deckendicke 1,5 mm** | **teilweise.** 1,5–2,0 mm als übliche Spanne und 2,0 mm gleichmäßig an einer dokumentierten Decke (Inanli 2018). Die Spanne deckt die Annahme ab | die Dicke des Abdallah-Instruments; ein Verlauf über die Decke (nur „decreasing … from bridge location to the edge“ als Möglichkeit) |
| **Oud: Querschnitt der Schale** | **nicht** | ganz |
| **Oud: kein Steg im Bauplan** | **nicht.** Inanli zeigt nur die Lage (≈ 8–10,5 cm über dem Ende, grob) | Masse, Länge, Breite, Höhe |
| **Oud: Abstand der Chöre** | **nicht** | ganz |
| **Oud: Verluste der umsponnenen Bässe (von der Gitarre)** | **nicht.** Bauart belegt (Nylonfaser oder „nylon silk“, versilbertes oder rotes Kupfer) | alle Verlustwerte und die Biegesteife |
| **Oud: tiefster Chor einzeln** | **ersetzt.** Alle gelesenen 11-Saiten-Sätze (Aquila, Kürschner, Pyramid) haben ein einzelnes C (bzw. E) | – |
| **Oud: Zug je Saite** (für die Besaitung) | **teilweise.** Aquila 3,4 kgf Mittel, La Bella ≈ 4 kgf (Schätzung des Händlers); Durchmesser für Aquila und Pyramid | Zug je Chor (keine Tabelle gefunden; Kürschner nur auf Anfrage) |
| **Oud: Balken des Bauplans** (aus Abdallah) | zweite, unabhängige Quelle für Querschnitte (Inanli Tab. 1: Höhe 12 mm, Enden 6 mm, Breite 5–6 mm; Abdallah 16,5 mm, Breite 4,2–6,6 mm). Ersetzt nichts, macht die Spanne sichtbar | Lage der Balken bei Inanli |
| **Laute: Deckendicke 1,8 mm** | **teilweise.** „often about 1.5mm“ für erhaltene Lauten (Van Edwards); 1,9–2,1 mm an einem modernen Nachbau (Schreiner) | ein gemessener Verlauf an einer historischen Renaissancelaute |
| **Laute: Lage und Maße der sieben Querbalken** | **teilweise, nur die Lage.** Reihe an der Jauch-Laute (32 / +76 / +59 / +50 mm, Balken 4 durch die Mitte der Rose); passt der Ordnung nach zum Bauplan | Querschnitte historischer Balken; Balken über der Rose; eine Renaissance-Laute statt einer Barocklaute |
| **Laute: Rosette** (85 % offen) | **nicht** | offener Anteil |

## Nicht gefunden: Suchwege

- **Oud, Baumaße:**
  - englisch: „oud soundboard thickness mm bracing“, „oud vibroacoustic finite element thickness“, „Nahat oud plan
    dimensions“, „Tuning the structural eigenfrequencies of an oud“;
  - türkisch: „ud göğüs kalınlığı mm kafes ölçüleri tez akustik“;
  - zu Değirmenli (YÖK, DergiPark).
  - Gelesen: DAGA 2018, GU J Sci C 2017, Dusepo.
  - Nicht erreicht: YÖK, mikeouds.com, metmuseum.org, emirdegirmenli.com.
  - Nicht gesucht: arabische Hochschulschriften (keine Suche auf Arabisch), Barbat.
- **Besaitung:** „oud strings gauges tension Pyramid Aquila La Bella Kürschner“, „Kürschner oud tension chart“, „Pyramid
  Super-Aoud gauges tension catalogue“, „music-strings.de Kürschner Saitenzug“. Gelesen: Thomann (vier Artikel),
  orientalinstruments.com (zwei), stringsandbeyond.com. pyramid-strings.de und music-strings.de habe ich nicht direkt
  abgefragt.
- **Risha:** „oud risha plucking point distance from bridge measurement acoustics“. Nur Spielanleitungen und Forum.
- **Laute:** „Lundberg Historical Lute Construction thickness bars“, „Renaissance lute soundboard modal analysis /
  bridge admittance“ (zweimal, einmal „extended“), „lute belly barring museum drawing“, „Laux Maler lute … thickness“.
  Gelesen: luth.org (Abstract), Schreiner-Blog, Van Edwards, FEMTO-Folien.
- **Einzeltöne:** „oud OR lute decay time T60 partials“, „oud single notes dataset Creative Commons“; Commons-API;
  Freesound mit CC0-Filter („oud“, „lute note“).
- **Umsponnene Saiten:** „wound string damping loss factor silk core nylon core“ und „silk string … pipa guqin oud“;
  lokale Woodhouse-PDFs mit `pdf_suche.py` („wound“, „loss factor“, „bending stiffness“, „silk“).

## Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | wozu |
|---|---|---|---|---|
| `Inanli_Altinsoy_2018_VibroacousticsOud_DAGA.pdf` | https://pub.dega-akustik.de/DAGA_2018/data/articles/000581.pdf | 678 476 B | DAGA-Tagungsband, frei abrufbar; Lizenz nicht geprüft | Decke, Balken, Löcher, freie Moden (§1.1) |
| `Degirmenli_2017_UdTanburKanun_GUJSciC.pdf` | https://dergipark.org.tr/en/download/article-file/840569 | 1 172 112 B | DergiPark, frei; Lizenz nicht geprüft | Kurven von 7 Ouds (Abb. 7, 10) |
| `FEMTO_2019_LauxMalerLute_Folien.pdf` | https://publiweb.femto-st.fr/tntnet/entries/18458/documents/author/data | 3 201 294 B | Folien, Lizenz nicht angegeben | Holzkennwerte und gerechnete Moden einer Renaissancelaute |
| `Oud_open_Strings.ogg` | https://upload.wikimedia.org/wikipedia/commons/7/77/Oud_open_Strings.ogg | 61 250 B | CC BY-SA 3.0, User:Bottomline | Hörprobe leerer Saiten |
| hammondman `a2.wav`, `g2.wav`, `c3.wav` | über https://freesound.org/search/?q=oud&f=license:%22Creative+Commons+0%22 | ? | CC0 laut Filter; Download nur mit Anmeldung | Einzeltöne |
| (kaufen oder anfragen) Lundberg, *Historical Lute Construction*; AL #14 | luth.org | – | käuflich | Dicken und Balken historischer Lauten |
| (kaufen oder anfragen) Hellwig 1968, GSJ XXI; Nurse 1986 | JSTOR / Lute Society | – | hinter der Bezahlschranke | Teilung der Lautendecke |
| (anfragen) Kürschner-Zugtabelle | music-strings.de | – | nur auf Anfrage | Zug je Chor |
| (anfragen) Değirmenli 2018, Dissertation | YÖK Açık Bilim, handle 20.500.12812/582063 | – | nicht erreicht | Stegmobilität und Moden türkischer Ouds |

## Was nicht geliefert ist, und was die LumiViz-Sitzung selbst prüfen muss

- **Nicht geliefert:**
  - ein **vollständig vermessenes Oud-Instrument** (Decke, Balkenlage, Steg und Schale an einem Stück): Inanli liefert
    Decke und Balkenquerschnitte, aber weder die Balkenlage noch den Steg noch die Schale;
  - Stegmasse und Stegmaße für Oud und Laute;
  - Schalenvolumen der Oud;
  - Abstand der Chöre;
  - Zug je Chor;
  - Risha-Parameter;
  - T60 je Teilton;
  - Verluste von Saiten mit Seiden- oder Nylonkern;
  - Lautenmoden außer Woodhouse 2021;
  - Querschnitte historischer Lautenbalken.
- **Selbst prüfen:**
  1. ob die Lesart von Abb. 2 bei Inanli („80 mm“ = senkrechter Abstand der Lochmitten) und die Ablesung der Lagen aus
     Abb. 4 stimmen (`seiten/inanli2018-2.png`);
  2. ob „32 mm in front of the bridge“ bei Schreiner von der Stegvorderkante gemessen ist;
  3. die Lizenz auf den einzelnen Freesound-Seiten;
  4. die Zuordnung der Pyramid-Durchmesser zu den Chören.

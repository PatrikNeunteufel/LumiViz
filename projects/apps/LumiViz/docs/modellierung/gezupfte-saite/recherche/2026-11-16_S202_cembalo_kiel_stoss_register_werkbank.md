> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-16_04_cembalo_kiel_stoss_register` (gestellt in LumiViz
> S201), 1 Segment(e): 2026-11-16 20:48–21:01, claude-opus-5-5, 69 Runden, success, 2.73 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S202** — nur dieser Kopf ist dazugekommen. **Nicht gegengelesen:** keine Formel,
> Tabelle oder Zahl dieses Berichts ist bisher an der Quelle nachgelesen; die Marken der Belegkraft sind die des Laufs.
> Der Auftrag liegt seit S202 im Archiv der Werkbank: `archiv/auftraege/2026-11-16_04_cembalo_kiel_stoss_register.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-16_04_cembalo_kiel_stoss_register/` (darin `seiten/` — Seitenbilder der Quellen — und Textauszüge aus PDFs, die nicht ins
> Repo gehören), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Abgerufene PDFs** liegen als Kopie unter `…\cmake\quellen\` (der Bericht nennt die Namen der Werkbank):
> - `ICSV30_2024_HarpsichordMechanicalNoise_Bologna.pdf` → `quellen\gezupfte-saite\Thwaites_Fletcher_1981_NotesOnClavichord.pdf` — Name der Werkbank war falsch (ICSV30_2024_…); Inhalt Thwaites & Fletcher, Clavichord
> - `webfetch-1794858809204-t8epq1.pdf` → `quellen\gezupfte-saite\Law_2008_HarpsichordRegulation_HFP13-1.pdf` — CC BY-NC 4.0 (PDF-S. 1)
> - `webfetch-1794859017570-veug06.pdf` → `quellen\gezupfte-saite\Fletcher_Beebe_2010_HarpsichordClavichord_Kap8.pdf` — Buchkapitel 8 (Buch laut Bericht nicht geprueft)
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-16_S202_cembalo_kiel_stoss_register_werkbank/`: `edinburgh_zupf.py`.

# Bericht: Zupfkraft des Kiels je Taste, mechanische Geräusche, Lautenzug, Saitendämpfung, Zupfpunkte der Register (Cembalo)

- **Auftrag:** `auftraege/2026-11-16_04_cembalo_kiel_stoss_register.md`
- **Datum:** 2026-11-16
- **Modell:** claude-opus-5-5
- **Reichweite:** 17 Websuchen (WebSearch, davon 2 „extended“), 23 Abrufe (WebFetch), 1 `curl -I` (nur Kopfzeilen, kein Download).
  Lokal gelesen: Perng 2012 (Diss., S. 126–128 als Bild; Verzeichnisse durchsucht), Välimäki u. a. 2004 (S. 935–941 als
  Bild), Chadefaux u. a. 2012 (Harfe, Textsuche), Bader/Plath/Kontopidis 2025 und Penttinen 2006 (nur Textsuche),
  sieben Datenblätter Edinburgh St Cecilia's (lokale HTML, Auszug mit `edinburgh_zupf.py` in diesem Ordner).
  **Nicht erreicht:** Simmons 1974 (PDXScholar, Cloudflare-Abfrage, HTTP 403 — nicht umgangen); ICSV30-Beitrag 2024 und
  Málaga-/Bologna-Beitrag zum „modal harpsichord“ (cris.unibo.it, HTTP 403); Forum-Acusticum-2025-Beitrag 000046
  (über 10 MB, Abbruch des Werkzeugs); *Harpsichord & Fortepiano* 16(2) Art. 08 (Zeitüberschreitung); Fletcher 1977
  (*Acustica* 37, hinter Bezahlschranke, im ANU-Repositorium bis 2037 gesperrt); Giordano & Winans 1999 (*JSV*,
  Bezahlschranke); O'Brien 1990 und Kottick (Bücher, kein Volltext); Brünner Abschlussarbeit David 2023 (kein Volltext
  auf der Seite, siehe Ladeliste).

Gelesene PDFs aus dem Sitzungsordner (liegen nach dem Lauf unter `abrufe/`):

| Datei im Sitzungsordner | URL | Inhalt |
|---|---|---|
| `webfetch-1794859017570-veug06.pdf` | https://newt.phys.unsw.edu.au/music/people/publications/Acoustics%20of%20the%20Harpsichord%20and%20Clavichord.pdf | Fletcher & Beebe, Kap. 8 „Harpsichord and Clavichord“ (S. 123 ff.) |
| `webfetch-1794858809204-t8epq1.pdf` | https://mircat.org/moa/journals/hfp/articles/2008_13-1_17.pdf | D. J. Law, „Harpsichord Regulation“, *Harpsichord & Fortepiano* (Heft 13/1, 2008), S. 17 ff. |
| `webfetch-1794858746071-qiakg3.pdf` | https://www.phys.unsw.edu.au/music/people/publications/Thwaitesetal1981.pdf | Thwaites & Fletcher 1981, *Some notes on the clavichord* — **Clavichord, für diesen Auftrag ohne Belang** |

**Worauf aufgebaut wird (nicht wiederholt):** Penttinen 2006 (Dynamik je Stufe, Hörtest), Perng 2011 Tab. 2
(Auslenkung), Roy 2015 und Chadefaux 2013 (keine Kraft je Taste; 0,8 N an G4 laut GZ13 a).

---

## 1. Zupfkraft / Ablösekraft des Kiels je Taste

**Ergebnis vorweg:** Eine **gemessene Kraft je Taste über den Umfang** (Bass bis Diskant), getrennt nach Delrin/Federkiel
und 8′/4′, wurde **nicht gefunden**. Gefunden sind Einzelwerte, eine Tastenkraft in Gramm mit Hebelverhältnis und
qualitative Angaben von Cembalobauern zum Gang über den Umfang.

### 1a. Kraftwerte

| Größe | Wert | Instrument / Taste | Quelle | Marke |
|---|---|---|---|---|
| Kraft des Plektrums, Größenordnung | „1.5 N“ | Plastik-Springer und -Plektrum, kleines Modell der Zupfmechanik (Giordano & Winans 1999, ref. 15) | Chadefaux u. a. 2012, *JASA* 131, PDF-S. 6: „the measured order of magnitude for a plectrum (1.5 N) is much lower than the force magnitude measured for the finger in the present study (up to 15 N)“; lokal `Chadefaux_etal_2012_HarpPluckingDescription_JASA131.pdf` | [wörtlich] (Zitat aus zweiter Hand; Giordano & Winans selbst nicht gelesen) |
| Kraft an der Taste („plucking force“), 2 × 8′ gekoppelt | „160 g“ vorher, „130 g“ nach Umbau | Ross-Cembalo des Verfassers (deutsche Größe), Taste nicht genannt | J. Sankey, https://www.johnsankey.ca/action.html | [Werkzeug] |
| Obergrenze historisch | „unusual for a pre-19th century harpsichord to require a key force greater than 150 g“ | allgemein | ebd. | [Werkzeug] |
| Übersetzung Finger → Springer | Flämisch 16. Jh. 1,35; italienisch ≥ 1,2; 18. Jh. „1:1 … common“; Ross nach Umbau „about 1.15 for the main 8'“ | – | ebd. | [Werkzeug] |
| Gewicht des Springers | historisch „less than 5 g“; Kunststoff-Springer (Boston) „6 g“ | – | ebd. | [Werkzeug] |
| Tastengewicht („touch weight“), Faustwert | „75-125 grams“ (Fred Sturm, zitiert); eigener Bau ~100 g je Taste | Virginal (Petzold) | C. Petzold, https://charlespetzold.com/blog/2023/05/Building-a-Virginal-Feeling-Like-Im-Just-Getting-Started.html | [nur Zusammenfassung] |
| Messverfahren Zupfstärke | Messinggewicht 115–120 g wird auf den Tastenhebel gesenkt; abgelesen wird die **Stelle** (mm), an der es gerade zupft; „+x mm“ = härter | Italienische Nachbauten (Wraight) | D. Wraight, https://denzilwraight.com/quillperf.htm | [Werkzeug] |
| Verhärtung eines neuen Federkiels | +25 bis +35 mm (Gewichtsstelle) nach 50–250 Zupfern | Federkiel, 8′-Diskant, Kielspitze ≈ 0,22 mm dick | ebd. | [Werkzeug] |

**Umrechnung auf die Kraft am Kiel** [eigene Rechnung, Annahmen offen]: 130 g an der Taste für 2 × 8′ = 1,28 N.
Bei Übersetzung 1,15 (Springerweg / Fingerweg, Sankey) folgt nach virtueller Arbeit eine Kraft an beiden Springern von
1,28 / 1,15 = 1,11 N; abzüglich zweier Springer à 6 g (0,12 N) bleiben ≈ 1,0 N, **je Kiel ≈ 0,5 N**. Nicht berücksichtigt:
Reibung in Führung und Waagebalken, Ausgleich der Taste (Blei), welche Taste gemessen wurde. Das ist eine Obergrenze für
die quasistatische Ablösekraft *eines* Kiels an einer unbekannten Taste dieses einen Instruments. Mit 150 g als
historischer Obergrenze und Übersetzung 1,0 bis 1,35 liegt sie zwischen ≈ 0,5 und 0,7 N je Kiel bei 2 × 8′.
Zum Vergleich: Chadefaux 2013 0,8 N an G4; Giordano & Winans (aus zweiter Hand) ≈ 1,5 N; LumiViz heute 2 N an C4.

**Wichtige Einschränkung** [eigene Einschätzung]: Gemessen wird beim langsamen Niederdrücken. Ob die Ablösekraft beim
schnellen Anschlag (ff) höher ist, sagt keine dieser Quellen; Giordano & Winans finden laut der Purdue-Seite die
Auslenkung „approximately independent of the velocity“ ([nur Zusammenfassung], https://www.physics.purdue.edu/piano/articles.shtml).

### 1b. Gang über den Umfang (nur qualitativ)

| Aussage | Quelle | Marke |
|---|---|---|
| „The bass needs stronger quills because the string feels like it stretches somewhat before it gets plucked“; „In the treble, the short strings and close plucking point makes things feel stiff, so a weaker quill is needed.“ | trasuntino.blogspot.com/2009/08/voicing.html (Nachbau eines italienischen Cembalos) | [Werkzeug] |
| Sechs Kieldicken, „changing approximately every tritone to a thinner quill“; Kiellänge „around 5 mm“; Überstand über die Saite „about one string diameter“ | ebd. | [Werkzeug] |
| Voicing beginnt in der Mitte („starting from middle C, I voice the naturals for one octave upwards“); zu hartes Zupfen hört man daran, dass die Tonhöhe am Anfang höher ist als beim Abklingen | Law 2008, *HFP* 13/1, PDF-S. 4 (`webfetch-1794858809204-t8epq1.pdf`) | [wörtlich] (Textebene, Spaltensatz verschränkt) |
| Tastentiefgang „Short keys on a single-manual instrument are best given a key dip of 6mm.“; die Springer werden durch die gepolsterte Springerleiste angehalten | Law 2008, PDF-S. 2 | [wörtlich] (Textebene, verschränkt) |

Die Form „Bass stärker, Diskant schwächer“ stützt die **Richtung** von GZ13 a (fallend zum Diskant), gibt aber keine dB je
Oktave.

### 1c. Delrin gegen Federkiel, 8′ gegen 4′

Keine Kraftwerte. Wraight: Federkiel „work-hardens“ nicht, Delrin schon [Werkzeug]. Trasuntino: Delrin „can get close if
worked on by a knowledgeable voicer“ [Werkzeug]. Für 8′ gegen 4′ siehe Abschnitt 5 (Schallpegel je Register, nicht Kraft).

---

## 2. Mechanische Geräusche

| Größe | Wert | Instrument | Quelle | Marke |
|---|---|---|---|---|
| Geräusch der Mechanik allein (Springer bewegt, Kiel trifft die Saite nicht: „recorded in isolation by moving the 8-foot lower register“), Taste F3 | lauter Anschlag: Spitze „-42.5 dB at the peak frequency of 172 Hz“; leiser Anschlag „-42.0 dB“ bei derselben Frequenz; laut zusätzlich mehr Anteile über 9 kHz | Taskin 1788 (Revision), Mikrofone ≈ 1 m | MacRitchie & Nuti 2015, *Front. Psychol.* 6:183, Exp. 2, Methods; https://www.frontiersin.org/journals/psychology/articles/10.3389/fpsyg.2015.00183/full | [Werkzeug] (über WebFetch als Zitat geliefert) |
| Abklingen des Geräuschs | „After 250 ms in each isolated knocking sound there is a 10 dB decrease“ | dto. | dto. | [Werkzeug] |
| Ton zum Vergleich (Grundton, Mittel über das ganze Signal), F3 unteres 8′ | laut −33,60 dB, leise −35,44 dB (Tab. 1) | dto. | dto., Tab. 1 | [Werkzeug] |

**Relativ zum Ton** [eigene Rechnung, schwach]: Die Spitze des Mechanikgeräuschs (−42,5 dB) liegt rund **9 dB unter**
dem Grundton des lauten F3 im unteren 8′ (−33,6 dB). Der dB-Bezug ist im Aufsatz nicht genannt, und die beiden Zahlen
sind verschieden gewonnen (Spektralspitze gegen Mittel über das ganze Signal). Auffällig: 172 Hz liegt nahe am Grundton
von F3 bei A = 415 Hz (≈ 165 Hz) — beim Bewegen des Springers hebt sich der Dämpfer, die Saite kann also mitklingen.
Ob das „Klopfen“ Mechanik oder angeregte Saite ist, lässt der Aufsatz offen. Das muss die LumiViz-Sitzung am Volltext
prüfen.

Weitere Angaben:

| Aussage | Quelle | Marke |
|---|---|---|
| Välimäki u. a. spielen beim Loslassen ein aufgenommenes Geräusch ab („key-release thump“); gewonnen, indem der Ton erst ganz ausklingt und dann die Taste losgelassen wird. Pegel und Spektrum sind **nicht angegeben**. | Välimäki u. a. 2004, *EURASIP JASP*, S. 934 (Abstract) und S. 939 §3.3; lokal | [wörtlich] |
| Das Aufsetzen des Springers an der Leiste ist „a noisy occurrence even with a pad“ | Sankey, action.html | [Werkzeug] |
| Flügel als Vergleich: Geräusch von Finger und Taste („touch precursor“) etwa 20–30 ms vor dem Ton; Bodengeräusch der Taste getrennt davon; dB gegen den Ton nicht gefunden | Goebl/Bresin/Fujinaga 2014 (*JASA*), zitiert nach der Suchmaschine; KTH-Seite Askenfelt: Taste unten und Hammeranschlag bei mf etwa gleichzeitig, Versatz bis „about 20 ms“ | [nur Zusammenfassung] bzw. [Werkzeug] |
| Weg in den Boden: keine Quelle gefunden. Bader u. a. 2025 sprechen nur vom „Klopfen“ als Impuls am Steg im Modell (lokal, PDF-S. 2 und 5). | – | [wörtlich] (nur Textsuche) |

Kraftstoß in N·s, Dauer, Spektrum über den Umfang: **nicht gefunden**.

---

## 3. Lautenzug (buff stop)

| Größe | Wert | Quelle | Marke |
|---|---|---|---|
| Lage | Die Polster gleiten **am 8′-Sattel** gegen die Saiten: „a batten with a set of small felt or leather pads glued to its upper surface, which is able to slide against the 8′ nut“ | Fletcher & Beebe, Kap. 8, S. 134 §8.2.5 (`webfetch-1794859017570-veug06.pdf`) | [wörtlich] (Seite als Bild gelesen) |
| Wirkung | „The effect of the pads is to damp dramatically the upper harmonics of the string vibration“ | ebd. | [wörtlich] |
| Lage, Bauanleitung | Für einen „singing buff“ muss die Leiste bündig am Sattel liegen: „If yours is even a few millimetres away from the nut, there is no way you can fit a singing buff.“ | Bray Baroque, https://braybaroque.ie/harps/buff.htm | [Werkzeug] |
| Maße | Oberkante der Leiste „about 1-2mm below the string“; Leiste ≥ 5 mm breit; Polsterhöhe ≈ 3 mm; Polsterdicke 6 mm (nur ein 8′) bzw. ≈ 4 mm (je 8′ ein Polster); Saite liegt an der **Mitte** des Polsters | ebd. | [Werkzeug] |
| Material | Leder, „at least 3-4mm“ dick, Büffel oder ähnlich weiches Leder im Bass; dünner im Tenor; Diskant „1mm-thin pads“ aus weichem Rindleder; Tuch genügt für den „singing buff“ nicht | ebd. | [Werkzeug] |
| Andruck | nicht angegeben; Polster beim Kleben „gently touching the string“ | ebd. | [Werkzeug] |
| Wirkung, qualitativ | „singing buff“: „the high partials will be damped, but the whole string hardly so“, Ton „will last many seconds“; „damping buff“: „will last for a few seconds only“ | ebd. | [Werkzeug] |
| Modell | Leder als Feder `k_spring` und Dämpfer `R_damp` parallel, punktförmig, ohne Masse; Reflexion `R = −Z_eff/(2 Z0 + Z_eff)`, `Z_eff = k/s + R` (Gl. 5.5–5.10). **Keine Zahlenwerte** für k und R. | Perng 2012, Diss., S. 126–128 (lokal) | [wörtlich] (Seiten als Bild gelesen) |

**Abstand zum Sattel** [eigene Rechnung]: Liegt das Polster bündig am Sattel und berührt die Saite mit seiner Mitte, so
liegt die Wirkstelle bei der halben Polsterdicke, also bei **≈ 2–3 mm** (Polster 4–6 mm). LumiViz setzt heute 0,5 mm. Ob
das Polster in Saitenrichtung 4–6 mm misst oder quer dazu, sagt die Seite nicht ausdrücklich („thick“). Das muss an
einem Bild oder einer Zeichnung nachgeprüft werden.

Gemessene Abklingzeit, Spektrum oder Lautheit mit und ohne Lautenzug: **nicht gefunden**. Perng zeigt nur eine Simulation
(Abb. 5.11/5.12). Für das *peau de buffle*-Register (Lederkiel, kein Dämpfer) gibt MacRitchie & Nuti 2015 Tab. 1 Pegel
des Grundtons; das ist kein Lautenzug.

---

## 4. Dämpfung und Inharmonizität der Saiten

| Größe | Wert | Instrument | Quelle | Marke |
|---|---|---|---|---|
| Inharmonizität B, alle 56 Tasten | Minimum ≈ 1,5·10⁻⁵ bei ≈ 70–100 Hz; ≈ 3·10⁻⁵ bei 200 Hz; ≈ 1·10⁻⁴ bei 400 Hz; ≈ 2·10⁻⁴ bei 700 Hz; ≈ 4·10⁻⁴ bei 1050 Hz (8′, 46–1100 Hz). Text: „in the case of the 19 lowest keys … the inharmonicity coefficients are about 10⁻⁵“ | Knif/Pelto 2000, nach italienisch-süddeutscher Art, 2 × 8′ + 4′, **alle Saiten Messing**, A4 = 415 Hz | Välimäki u. a. 2004, Abb. 5 (S. 939) und S. 938 | Zahlen [aus Abbildung abgelesen], log. Achse, ± 30 %; Satz [wörtlich] |
| Nachhallzeit je Teilton, ein Ton | f0 = 197 Hz: Teilton 1 ≈ 3,8 s; 2–5 ≈ 7–9 s; Teilton 6 (≈ 1180 Hz) ≈ 11,7 s; um 2–4 kHz ≈ 3–6 s; Teiltöne 10, 11, 16, 18 verworfen | dto. | Välimäki u. a. 2004, Abb. 8 b (S. 941) | [aus Abbildung abgelesen], ± 0,5 s |
| Verlustfilter dazu | „g = 0.9960 and a = −0.0296“ (Einpol), Welligkeit „r = −0.0015“, „r_rate = 0.0833“ | dto. | ebd., Bildunterschrift Abb. 8 | [wörtlich] |
| Abklingen bis Unhörbarkeit, über den Umfang | 8′: ≈ 10 s (C2) bis ≈ 4 s (C6), Steigung „(1/frequency)^0.25“; 4′: ≈ 7 s bis ≈ 2 s, „(1/frequency)^0.45“ | moderner Nachbau eines Ruckers, ein Manual, 8′ + 4′ | Fletcher & Beebe, Abb. 8.8 und Text S. 133 | Steigungen [wörtlich]; Zeiten [aus Abbildung abgelesen], log. Achse, ± 15 % |
| Abklingrate, c′ | Ton 2,6 /s; davon Luft 0,86 /s, „1.7/s for the soundboard“ (Rechnung des Verfassers) | Sankeys eigenes Cembalo | https://johnsankey.ca/energy.html | [Werkzeug] |
| Aufteilung der Verluste | „Most of it is dissipated in viscous losses from the motion of the string through the air and in losses to the material of the soundboard.“ | allgemein | Fletcher & Beebe, S. 134 | [wörtlich] |
| Drähte im Vergleich (Eisen, Stahl, Messing, Bronze), mit Vakuummessung | Abstract: Durchmesser „170 … 600“ (Einheit im Abruf „mm“, gemeint wohl µm), 20 Hz – 12 kHz; Stahl klingt länger als Eisen; Luft und Abstrahlung „greatly exceed all other modes of energy loss“. **Zahlen der inneren Reibung nicht gelesen.** | Laborsaiten | Simmons 1974, MS-Arbeit Portland State, https://pdxscholar.library.pdx.edu/open_access_etds/1993 | [Werkzeug] (nur Abstract; Volltext gesperrt) |

**Was das für den Verlustfaktor heißt** [eigene Rechnung]: Aus T60 folgt ein Gesamtverlustfaktor
η = ln(1000)/(π f T60) = 6,91/(π f T60). Välimäki Abb. 8: Teilton 1 (197 Hz, 3,8 s) → η ≈ 2,9·10⁻³; Teilton 6
(1180 Hz, 11,7 s) → η ≈ 1,6·10⁻⁴; ≈ 3,9 kHz, 3,5 s → η ≈ 1,6·10⁻⁴. Das sind **Gesamtverluste am Instrument** (Steg und
Boden, Luft, innere Reibung). Die innere Dämpfung des Drahts allein kann nicht größer sein als der kleinste Wert, also
η_innen ≲ 2·10⁻⁴ für Messing bei 1–4 kHz an diesem Instrument. Ein frequenzunabhängiger Wert von 0,001 lässt die
Teiltöne um 1–4 kHz rund fünfmal zu schnell abklingen gegenüber dieser Messung — sofern der Wert in LumiViz dasselbe
η bezeichnet; das prüft die LumiViz-Sitzung. Für Eisen fehlt ein entsprechender Wert; Fletcher & Beebe geben nur
Abklingzeiten bis zur Unhörbarkeit (hängt vom Anfangspegel ab, ist also kein T60).

Durchmesser und Spannung der bei Välimäki gemessenen Saiten: **nicht angegeben** (nur „all made of brass“, Länge des
Instruments 240 cm).

---

## 5. Zupfpunkte der Register (4′, zweites 8′)

### 5a. 8′ und 4′ getrennt — Nachbau eines Ruckers (Fletcher & Beebe, Abb. 8.4)

Abgelesen an `seiten/webfetch_1794859017570_veug06_S0006_130dpi.png` (Abb. 8.4, S. 128). „The pluck fraction gives the
position of the plucking point from the nut relative to the total sounding length of the string involved“ [wörtlich].
Rechte Achse 0–0,9, Raster 0,2 (≈ 36 Pixel je 0,1 bei 130 dpi); linke Achse 0–1800 mm. Genauigkeit der Anteile ± 0,01,
der Längen ± 20 mm. Alle Zahlen [aus Abbildung abgelesen].

| Taste | 8′ Länge (mm) | 8′ Anteil | 8′ Abstand vom Sattel (mm) | 4′ Länge (mm) | 4′ Anteil | 4′ Abstand vom Sattel (mm) |
|---|---|---|---|---|---|---|
| C2 | ≈ 1500 | ≈ 0,10 | ≈ 150 | ≈ 790 | ≈ 0,09 | ≈ 70 |
| C3 | ≈ 1100 | ≈ 0,12 | ≈ 130 | ≈ 540 | ≈ 0,11 | ≈ 60 |
| C4 | ≈ 665 | ≈ 0,15 | ≈ 100 | ≈ 320 | ≈ 0,15 | ≈ 50 |
| C5 | ≈ 340 | ≈ 0,25 | ≈ 85 | ≈ 165 | ≈ 0,28 | ≈ 45 |
| C6 | ≈ 175 | ≈ 0,35 | ≈ 60 | ≈ 90 | ≈ 0,48 | ≈ 45 |

Abstand = Länge × Anteil [eigene Rechnung]. Ein Instrument mit **einem** 8′; ein zweites 8′ gibt es hier nicht.

### 5b. Ein Zupfpunkt je Taste — Datenblätter Edinburgh (lokal)

Alle Datenblätter geben „String lengths (plucking points)“ ohne Einheit (vermutlich mm) und **ohne Angabe des
Registers**, außer bei Kirkman 1755 („plucking point of dogleg“). Für die LumiViz-Mensur (Taskin 1769) liegt das schon
vor; neu ist hier nur der Vergleich.

| Taste | Taskin 1769 (96075) | Taskin 1783/84 (96081) | Hass 1764 (96074) | Kirkman 1755, Dogleg-8′ (96082) | Shudi 1766, Dogleg (96088) | Falkener 1773 (96076) |
|---|---|---|---|---|---|---|
| F6 | 148 (91) | 142 (80) | 141 (77) | 132 (48) | 133 (54) | 143 (63) |
| C5 | 357 (115) | 366 (105) | 362 (100) | 343 (75) | 344 (82) | 350 (83) |
| C4 | 711 (137) | 713 (124) | 694 (135) | 690 (102) | 701 (104) | 693 (107) |
| C3 | 1178 (162) | 1207 (148) | 1183 (161) | 1193 (131) | 1232 (130) | 1184 (137) |
| F1 | 1765 (208) | 1835 (193) | 1817 (203) | 1791 (186) | 1862 (175) | 1825 (189) |

[wörtlich] (lokale HTML). Bei Taskin 1769 steht für F3 „982 (51)“ — offenbar ein Schreibfehler (die Nachbarn liegen bei
137 und 162).

Die englischen Instrumente (Dogleg-8′, das von beiden Manualen gespielt wird) zupfen 25–35 mm näher am Sattel als die
französischen [eigene Rechnung aus der Tabelle]. Das ist **kein** Beleg für das zweite 8′ eines Taskin.

### 5c. Saitenlängen des 4′ bei Taskin (Di Veroli)

| Instrument | c | c′ | c″ |
|---|---|---|---|
| Taskin 1769, Edinburgh | 605 | 337 | 163 |
| Taskin 1774, Brüssel | 606 | 340 | 167 |
| Taskin 1781, Boston | 558 | 315 | 160 |
| Taskin 1782, Colares | 594 | 336 | 170 |
| Taskin 1784, Edinburgh | 637 | 351 | 169 |
| Taskin 1788, Mailand | 554 | 314 | 161 |

Klingende Längen in mm, C. Di Veroli, „Taskin Harpsichord Scalings and Stringings Revisited“, Teil 1 (2011), Tab. 1,
https://www.braybaroque.ie/harps/Taskin_stringing.htm — [Werkzeug]. Zupfpunkte des 4′ enthält die Seite nicht.

### 5d. Weitere Angaben

| Aussage | Quelle | Marke |
|---|---|---|
| Zupfpunkt „changes from 12% to about 50% of the string length in the bass and in the treble range, respectively“ (Register nicht genannt; 4′-Saiten im letzten Oktavbereich ohne Dämpfer) | Välimäki u. a. 2004, S. 936 §2.3 | [wörtlich] |
| „the jacks of the upper keyboard are closer to the bridge, while the jacks of the lower keyboard are further away“ — Taskin 1788 | MacRitchie & Nuti 2015 | [Werkzeug]; widerspricht der üblichen Beschreibung (oberes 8′ nasal = nahe am Sattel). Am Volltext prüfen. |
| Schallpegel in 2 m für eine rasch wiederholte kurze Tonleiter: 8′ „70 ± 5 dB(A)“, 4′ „68 ± 3 dB(A)“, zusammen „72 ± 5 dB(A)“ über den ganzen Umfang | Fletcher & Beebe, S. 132–133 §8.2.4 (Ruckers-Nachbau) | [wörtlich] (Seite als Bild gelesen) |
| Pegel des Grundtons, Taskin 1788, laut: unteres 8′ F2/F3/F4/F5 = −42,13/−33,60/−34,20/−43,45 dB; oberes 8′ = −48,89/−39,15/−37,32/−46,18 dB | MacRitchie & Nuti 2015, Tab. 1 | [Werkzeug] |

Die beiden letzten Zeilen betreffen nicht den Zupfpunkt, sondern **Zielgrößen für die Ablösekraft des 4′ und des
zweiten 8′** (GZ12 a: „Ablösekraft und Zupfpunkt des 4′ an den Aufnahmen bestimmen“): Am Ruckers-Nachbau ist das 4′ im
Mittel ≈ 2 dB(A) leiser als das 8′; am Taskin 1788 liegt der Grundton des oberen 8′ ≈ 3–7 dB unter dem des unteren 8′
(F5 bis F2) [eigene Rechnung]. Ein Grundton ist kein Lautheitsmaß; die Zahlen geben nur eine Richtung.

---

## Reicht das für …?

| Annahme heute | Urteil | Was die Quellen sagen | Was fehlt |
|---|---|---|---|
| **Ablösekraft 8′:** 2 N an C4, −2,8 dB je Oktave | **teilweise belegt — der Maßstab nicht** | Richtung (Bass stärker, Diskant schwächer) von Cembalobauern bestätigt [Werkzeug]. Größe: aus der Tastenkraft (130–150 g für 2 × 8′, Übersetzung 1,0–1,35) folgen **≈ 0,5–0,7 N je Kiel**; Chadefaux 0,8 N (G4); Giordano & Winans ≈ 1,5 N (Modell, aus zweiter Hand). Alle Werte liegen **unter** 2 N. | Eine Messung je Taste über den Umfang; Delrin gegen Federkiel; die Taste, an der Sankey misst; die Kraft bei schnellem Anschlag. |
| **Tastenstoß:** 7 mN·s am Stegpunkt, Gauß 0,4 ms | **weiter Annahme** | Einziger Messwert: Mechanikgeräusch am Taskin 1788, F3, Spitze ≈ 9 dB unter dem Grundton des Tons, fällt in 250 ms um 10 dB, bei 172 Hz (möglicherweise mitklingende Saite). Lauter und leiser Anschlag fast gleich (−42,5 / −42,0 dB). | Kraftstoß, Dauer, Ort und Weg in den Boden; ein Wert, der Taste/Springer von angeregter Saite trennt. |
| **Lautenzug:** 40 N·s/m bei 0,5 mm | **teilweise belegt (Lage und Material), Stärke weiter angepasst** | Lage am 8′-Sattel belegt (Fletcher & Beebe [wörtlich]); Bauanleitung: bündig am Sattel, Leder 1–4 mm, Polster 4–6 mm, Saite an der Polstermitte → Wirkstelle ≈ 2–3 mm vom Sattel [eigene Rechnung]. Modellform Feder + Dämpfer (Perng) ohne Zahlen. | Andruck, Kennwerte des Leders (k, R), gemessene Abklingzeit mit und ohne Lautenzug. |
| **4′ und zweites 8′:** keine Werte | **4′: teilweise belegt; zweites 8′: weiter offen** | 4′-Zupfanteil über den Umfang am Ruckers-Nachbau (Abb. 8.4, abgelesen); 4′-Längen von sechs Taskins; 4′ rund 2 dB(A) leiser als 8′ (Ruckers-Nachbau). Zweites 8′: nur Grundtonpegel Taskin 1788 und Edinburgh-Zupfpunkte ohne Register. | Zupfpunkte des 4′ und des zweiten 8′ an einem Taskin (Edinburgh-Zeichnung von Turner, kostenpflichtig); Kraft des 4′-Kiels. |
| **Verlustfaktor Draht:** 0,001 | **teilweise belegt — als Obergrenze; spricht gegen 0,001 bei hohen Teiltönen** | Gesamtverlust Messing (Välimäki) ≈ 2,9·10⁻³ am Grundton 197 Hz, ≈ 1,6·10⁻⁴ bei 1–4 kHz → η_innen ≲ 2·10⁻⁴. B je Taste für Messing vorhanden (Abb. 5). | Innere Dämpfung getrennt von Steg und Luft (Simmons 1974 mit Vakuummessung, gesperrt); Werte für Eisen; Durchmesser und Spannung der gemessenen Saiten. |

---

## Nicht gefunden

| Lücke | Wo gesucht |
|---|---|
| Kraft je Taste über den Umfang, Delrin/Federkiel, 8′/4′ | Websuche: „harpsichord plectrum pluck force measured newtons keys bass treble voicing“, „touch weight grams“, „quill delrin plectrum force measurement stiffness“, „jack plectrum … load cell … Galpin Society Journal“ (extended), „Kottick Harpsichord Owner's Guide … grams“, „Giordano Winans 1999“, „Griffel 1994“, „Fletcher 1977 Acustica“. Gelesen: Wraight (2 Seiten), Sankey (action, energy), trasuntino, Law 2008 (HFP), Petzold (nur Suchtreffer). Lokal: Perng 2012 (Tabellenverzeichnis, Suche „Newton“, „gram“, „jack weight“), Chadefaux 2012. |
| Pegel/Kraftstoß der Mechanikgeräusche | Websuche „harpsichord action noise key thump jack noise measurement“, „Askenfelt … key bottom … dB“; gelesen MacRitchie & Nuti 2015, KTH-Seite Askenfelt; lokal Välimäki 2004, Penttinen 2006, Bader 2025 (Suche „noise“, „knock“, „jack“). ICSV30-Beitrag (Bologna 2024) gesperrt. |
| Messung des Lautenzugs | Websuche „buff stop leather measurement decay time spectrum“, „buff stop OR lute stop … acoustical measurement“ (extended); gelesen Bray Baroque, Perng 2012 §5.2, Fletcher & Beebe §8.2.5; Brünner Arbeit David 2023 nur als Datensatz (ob sie den Lautenzug misst, unklar). |
| Innere Dämpfung historischer Drähte (Eisen, Messing, Nachbau nach Rose/Goble) | Websuche „historical harpsichord wire … loss factor“, „string decay time T60 … B per key“; Simmons 1974 gesperrt; lokal Välimäki 2004, Bader 2025 (nur Bodendämpfung). |
| Zupfpunkte des 4′ / zweiten 8′ an Taskin, Dulcken, Ruckers, Blanchet | Websuche „Taskin … plucking points 4' 8' registers“, „Grant O'Brien Ruckers plucking points 4-foot“, „Taskin 1769 … drawing plucking points“; gelesen Bray Baroque (Taskin-Mensuren), Chinnery (Taskin-Seite, keine Zahlen), lokale Edinburgh-Blätter. |

---

## Ladeliste (nichts geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | Wozu |
|---|---|---|---|---|
| `Simmons_1974_DampingHarpsichordStrings_MS_PSU.pdf` | https://pdxscholar.library.pdx.edu/cgi/viewcontent.cgi?article=2993&context=open_access_etds | 3,5 MB (Angabe der Seite) | Open Access (PDXScholar); Abruf scheitert an Cloudflare-Abfrage, nur im Browser von Hand | Lücke 4: Abklingzeit je Frequenz für Eisen, Stahl, Messing, Bronze, in Luft und Vakuum |
| `Fletcher_Beebe_2010_HarpsichordClavichord_Kap8.pdf` | https://newt.phys.unsw.edu.au/music/people/publications/Acoustics%20of%20the%20Harpsichord%20and%20Clavichord.pdf | 778,5 KB | Verlagsrecht (Buchkapitel, Buchtitel nicht geprüft — vermutlich Rossing (Hg.), *The Science of String Instruments*, Springer 2010 [eigene Einschätzung]); liegt schon als `webfetch-1794859017570-veug06.pdf` in `abrufe/` | Abb. 8.4 (Zupfanteil 8′/4′), Abb. 8.8 (Abklingen), Pegel je Register |
| `MacRitchie_Nuti_2015_HarpsichordTouchTaskin1788_FrontPsychol.pdf` | https://www.frontiersin.org/journals/psychology/articles/10.3389/fpsyg.2015.00183/pdf | unbekannt | CC BY (Frontiers, Open Access) [eigene Einschätzung, Lizenzvermerk nicht gelesen] | Mechanikgeräusch F3, Tab. 1/2 je Register; die Werte oben sind nur [Werkzeug] |
| `Law_2008_HarpsichordRegulation_HFP13-1.pdf` | https://mircat.org/moa/journals/hfp/articles/2008_13-1_17.pdf | 3,3 MB | „Licensed under CC BY-NC 4.0“ (PDF-S. 1, [wörtlich]); liegt als `webfetch-1794858809204-t8epq1.pdf` in `abrufe/` | Tastentiefgang, Voicing-Ablauf |
| `Fletcher_1977_DesignPerformanceHarpsichords_Acustica37.pdf` | (Hirzel/Ingenta; ANU-Datensatz https://openresearch-repository.anu.edu.au/items/c22ee166-22ba-4134-8986-073b1fd2efa3) | – | Bezahlschranke, Repositorium gesperrt bis 2037 | Lücke 1: Kraft des Plektrums, Auslenkung je Taste (Inhalt nicht geprüft) |
| `Giordano_Winans_1999_PluckedStringsHarpsichord_JSV224.pdf` | (*J. Sound Vib.* 224, 455–473) | – | Bezahlschranke | Lücke 1: gemessene Kraft am Plektrum (≈ 1,5 N laut Chadefaux 2012) |
| `ICSV30_2024_HarpsichordMechanicalNoise_Bologna.pdf` | https://cris.unibo.it/retrieve/369030a5-c29f-48c4-8cd0-6e9b441da66d/full_paper_636_20240430213941625.pdf | – | HTTP 403 | Lücke 2: erste 50 ms als Mechanikgeräusch, verschiedene Kielmaterialien |
| `FA2025_000046_HarpsichordForceSensors.pdf` | https://dael.euracoustics.org/confs/fa2025/data/articles/000046.pdf | > 10 MB | Forum Acusticum 2025 | Kraftsensoren unter den Springern (Werte in N) — Lücke 1, Bezug zu echtem Kiel unklar |
| `David_2023_SpektraADSRCembala_BP_VUT.pdf` | http://hdl.handle.net/11012/212594 | – | Abschlussarbeit VUT Brno | Inharmonizität je Register, Hüllkurven; vielleicht Lautenzug |
| Zeichnung Taskin 1769 (Turner) | über die Friends of St Cecilia's Hall, http://www.stceciliasfriends.org.uk/ | – | kostenpflichtig | Lücke 5: Lage aller drei Springerreihen am Taskin |

---

## Am Ende

**Verlangt und nicht geliefert:**
- Kraft je Taste über den Umfang, getrennt nach Delrin/Federkiel und 8′/4′ — es gibt nur Einzelwerte und eine Umrechnung
  aus der Tastenkraft.
- Mechanikgeräusche als Kraftstoß (N·s), Dauer und Spektrum je Teil (Taste, Springer, Dämpfer); Weg in den Boden.
- Gemessene Wirkung des Lautenzugs (Abklingen, Spektrum, Lautheit); Andruck.
- Innere Dämpfung je Teilton für Eisen und für Nachbaudrähte; Durchmesser und Spannung der gemessenen Saiten.
- Zupfpunkte des 4′ und des zweiten 8′ an einem Taskin, Dulcken, Ruckers-Original oder Blanchet.

**Muss die LumiViz-Sitzung selbst nachprüfen:**
1. MacRitchie & Nuti 2015 im Volltext: dB-Bezug, ob die −42,5 dB und die Werte der Tab. 1 vergleichbar sind, ob das
   „Klopfen“ bei 172 Hz eine mitklingende Saite ist, und die Aussage zur Lage der Springer der Manuale.
2. Fletcher & Beebe Abb. 8.4 am vergrößerten Bild nachlesen (meine Ablesung ± 0,01 im Anteil).
3. Ob der Verlustfaktor 0,001 in LumiViz dasselbe η meint wie η = 1/Q = ln(1000)/(π f T60) hier, und ob er nur die
   innere Reibung oder auch die Luft trägt.
4. Ob die Polsterdicke bei Bray Baroque in Saitenrichtung gemeint ist (davon hängt 2–3 mm gegen 0,5 mm ab).
5. Die Umrechnung der Tastenkraft auf den Kiel: die Annahmen (Übersetzung 1,15, Springer 6 g, keine Reibung, keine
   Ausgleichsgewichte) sind offen gelegt, aber nicht an einem Instrument gemessen.

> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-17_24_cembalo_nacharbeit` (gestellt in LumiViz S202, Nacharbeit zu den Berichten 04 und 14), 1 Segment: 2026-11-17 13:56–14:09, claude-opus-5-5, 110 Runden, success, 5.07 USD; Status `fertig` (regulär beendet).
> **Unverändert abgelegt in S203** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S203** (Seitenbilder des Laufs; das PostScript von Giordano & Winans selbst mit Ghostscript gerendert und Abb. 9 aus den Pixeln digitalisiert; Rechnungen des Gegenlesens nicht im Repo):
> - **Di Veroli 2012 — stimmt:** PDF-S. 3 (gedruckt S. 9: „one gram apart … accuracy of .10 gram“, „both stops off“, „action
>   weight is subtracted“), PDF-S. 4 (S. 10: 106,6 / 84,6 / 80,2 g; „much less weight and key dip (2.6mm) than the 8′ (4.2mm)“,
>   am Bild), PDF-S. 6 (S. 12: „made on one instrument“, am Bild). Abb. 2 von Auge (±3 g): 8′ netto ≈ 67 g bei 4,1 mm,
>   4′ ≈ 38 g, Mechanik 15–18 g — in sich stimmig mit 106,6 g. Nachgerechnet: 0,657 N; gegen GZ13 a (2 N) +9,7 dB
>   (Übersetzung 1,0) bzw. +10,9 dB (1,15); 4′ / 8′ −4,9 dB.
> - **MacRitchie & Nuti 2015 — Wortlaut stimmt:** S. 1 (Lage der Springer; steht in der Einleitung, nicht in „Abschnitt 2“),
>   S. 5 (Abstand ≈ 1 m, Fenster 1024 / Vorschub 512), S. 6 Tab. 1 am Bild (alle 32 Mittelwerte), S. 7–8 (−42,5 / −42,0 dB
>   bei 172 Hz, „above 9 kHz“, „After 250 ms … 10 dB“).
> - **Stimmt nicht — 172 Hz:** gedruckt sind F2 = 86, F3 = 172, F4 = 334.5, F5 = 689.1 Hz — bei 44,1 kHz und 1024 Punkten die
>   Mitten der FFT-Kanäle 2, 4, (8), 16 (86,13 / 172,27 / 344,53 / 689,06 Hz, Kanal 43,07 Hz; „334.5“ wohl Druckfehler für
>   344,5) — in S203 selbst nachgerechnet. Die „peak frequency of 172 Hz“ des Klopfens heißt nur „irgendwo in 151–194 Hz“;
>   eine mitklingende Saite F3 ist damit nicht belegt. Dazu falsch gerechnet: 172 Hz liegt **4,4 % (75 Cent)** über F3 bei
>   a′ = 415 Hz (164,7 Hz), nicht 7,6 %.
> - **Stimmt nicht — Giordano & Winans Abb. 9 (Vorabdruck S. 28):** Spitze **≈ 1,6 N bei ≈ 52 ms** (am gerenderten Bild in S203
>   selbst gesehen), danach Abfall auf ≈ 0,3 N bis 55 ms; Anstieg ≈ 50 ms. Das Skript des Laufs (`gw_abb9.py`) liest nur den
>   ersten Linienzug (Ende 46 ms) und findet 1,365 N. Wortlaut S. 6 („of order 1 N … typical key-force“) und Unterschrift
>   Abb. 14 stimmen in der Textebene. „1,5 N bei Chadefaux 2012 = gerundete Spitze dieser Kurve“ passt mit 1,6 N besser.
> - **Bray Baroque `buff.htm` — stimmt:** Dicke „left-to-right“, Breite „between the front and the back … length of string …
>   touched“; Damping / Singing 6/4 … 2/1 mm; Kante „flush with the end of the buff batten, i.e. where the latter meets the
>   nut“. Die Berichtigung zu Bericht 04 trägt. Verstimmung 4,66 / 9,33 / 18,7 Cent für a = 0,5 / 1 / 2 mm bei L = 186 mm.
> - **Logik Lautenzug:** „bündig am Sattel“ heißt an der Stirnseite des Sattels; der Abstand vom Grat bis zur Leiste kommt
>   dazu, also a ≥ w/2 (Mitte) bzw. a ≥ w (Ende). „0,5 mm an C6“ (GZ11 a) ist die Untergrenze, nicht widerlegt.
> - **Chadefaux 2013:** mit K = T / (L β (1 − β)) und β = 0,259 geben y_s 0,71 / 0,90 mm **0,67 / 0,85 N**, nicht 0,71 / 0,89 N
>   (die Zahl aus Bericht 14 setzt β ≈ 0,24 voraus; −0,5 dB).
> - **Weitere Wortlaute — stimmen:** Sankey (160 g → 130 g, c″ 375 mm, −12 %), Edinburgh 96075 (Zupfpunkte, Lautenzug auf
>   beiden 8′), Di Veroli „Taskin Revisited“ (Längen), Simmons-Abstract, Chadefaux 2013 S. 155 / 158.
> - **Nicht gegengelesen:** Petzold-Tabelle; hubbimp (Zupfrichtung); Goodway & Savage (nur Auszug); NUŠL / VUT; FA2025 000046;
>   JAMIS 2012 (Donahue, vom Lauf als ohne Belang eingestuft); JSV 224 gegen den Vorabdruck; Di Veroli Abb. 1 / 2 nicht
>   digitalisiert.
> **Ablage:** die Dokumente aus `abrufe/` liegen nur noch unter `…\cmake\quellen\` (Herkunft in `quellen\UMZUG_S203.tsv`; Name im Bericht → Ablage):
> - `MacRitchie_Nuti_2015_HarpsichordTouchTaskin_FrontPsychol6-183.pdf` → `gezupfte-saite\MacRitchie_Nuti_2015_HarpsichordTouchTaskin_FrontPsychol6.pdf`
> - `DiVeroli_2012_OptimisingHarpsichordStaggering_HFP16-2.pdf` → `gezupfte-saite\DiVeroli_2012_OptimisingHarpsichordStaggering_HFP16.pdf`
> - `JAMIS_2012_078.pdf` → `gezupfte-saite\Donahue_2012_PitchLevelsStringLengthsIronStrung_JAMIS.pdf`
> - `Giordano_Winans_1999_PluckedStringsHarpsichord_preprint.ps` → `gezupfte-saite\Giordano_Winans_1999_PluckedStringsHarpsichord_JSVpreprint.ps`
> HTML-, JSON- und Textabrufe, Fehlerseiten und Seitenbilder bleiben beim Lauf.
> Der Auftrag liegt seit S203 im Archiv der Werkbank: `archiv/auftraege/2026-11-17_24_cembalo_nacharbeit.md`, Ergebnisordner `archiv/ergebnisse/2026-11-17_24_cembalo_nacharbeit/` (darin `abrufe/` und, falls vorhanden, `seiten/`), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…` nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen und Textauszüge des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-17_S203_cembalo_nacharbeit_werkbank/`.

# Bericht: Nacharbeit zu den Berichten 04 und 14 — Ablösekraft des Kiels, Mechanikgeräusch, Lautenzug, Register im Wortlaut

- **Auftrag:** `auftraege/2026-11-17_24_cembalo_nacharbeit.md`
- **Datum:** 2026-11-17
- **Modell:** claude-opus-5-5
- **Reichweite:** 15 Websuchen (WebSearch, davon 1 „extended“); 27 rohe Abrufe mit `holen.py` (Python, legt jede Datei unter
  `abrufe/` ab und trägt sie mit SHA-256 in `abrufe/INDEX.md` ein), dazu 1 `curl -I` (nur Kopfzeilen). `WebFetch` stand
  in diesem Lauf nicht zur Verfügung; alles Zitierte ist **Rohtext** oder **Seitenbild**, nicht Zusammenfassung.
  Lokal: Chadefaux u. a. 2013 (Textsuche, PDF-S. 1 als Bild), Edinburgh-Datenblatt 96075 (lokale HTML).
  **Nicht erreicht:** Forum Acusticum 2025, Beitrag 000046 (32 396 919 Bytes laut `Content-Length` — **über der
  20-MB-Grenze**, nicht geladen); dessen komprimierte Fassung und Landeseite bei cris.unibo.it (HTTP 403); ICSV-Beitrag
  `full_paper_636` bei cris.unibo.it (HTTP 403); Simmons 1974, PDF bei PDXScholar (HTTP 403; die Landeseite mit Abstract
  war erreichbar); HAL `hal-01565218` (Paté u. a. 2017, JASA — Anubis-Abfrage „Making sure you're not a bot“, nicht
  umgangen); VUT Brno (David 2023 — Abfrageseite, nicht umgangen); Gutenberg (Hoover, HTTP 503); MIMO (SSL-Zertifikat
  nicht prüfbar); claviantica.com („This site doesn't exist here“).

**Gelesene Quellen (Autor, Jahr, Titel wie auf der ersten Seite; Ablage):**

| Quelle | URL | Datei in `abrufe/` |
|---|---|---|
| Jennifer MacRitchie, Giulia Nuti (2015): „Using historical accounts of harpsichord touch to empirically investigate the production and perception of dynamics on the 1788 Taskin“, *Frontiers in Psychology* 6:183, doi 10.3389/fpsyg.2015.00183 (CC BY, PDF-S. 13) | https://www.frontiersin.org/journals/psychology/articles/10.3389/fpsyg.2015.00183/pdf | `MacRitchie_Nuti_2015_HarpsichordTouchTaskin_FrontPsychol6-183.pdf` |
| N. Giordano, J. P. Winans II: „Plucked Strings and the Harpsichord“ — **Vorabdruck** (PostScript) zu *J. Sound Vib.* 224, 455–473 (1999); Text kann von der gedruckten Fassung abweichen | https://www.physics.purdue.edu/piano/articles/pluck.ps | `Giordano_Winans_1999_PluckedStringsHarpsichord_preprint.ps` (Textauszug `../giordano_winans_text.txt`) |
| Claudio Di Veroli (2012): „Optimising Harpsichord Staggering“, *Harpsichord & Fortepiano* 16(2), Spring 2012, S. 8–12 („Licensed under CC BY-NC 4.0“, PDF-S. 1) | https://mircat.org/moa/journals/hfp/articles/2012_16-2_08.pdf | `DiVeroli_2012_OptimisingHarpsichordStaggering_HFP16-2.pdf` |
| Thomas Donahue (2012): „Correlating Pitch Levels and String Lengths in Iron-Strung Harpsichords“, *JAMIS* (S. 78 ff.) — **ohne Belang** (keine Dämpfung) | https://amis.mircat.org/jamis/2012_078.pdf | `JAMIS_2012_078.pdf` |
| J. Sankey (2000): „The Keyboard of a Harpsichord“ (HTML) | https://www.johnsankey.ca/action.html | `sankey_action.html` |
| C. Di Veroli (2011): „Harpsichord – Fitting a singing buff“ (HTML) | https://braybaroque.ie/harps/buff.htm | `braybaroque_buff.html` |
| C. Di Veroli (2011): „Taskin Harpsichord Scalings and Stringings Revisited, Part 1“ (HTML) | https://www.braybaroque.ie/harps/Taskin_stringing.htm | `braybaroque_Taskin_stringing.html` |
| C. Di Veroli: „Improving an old Hubbard harpsichord kit“ (HTML) | https://braybaroque.ie/harps/hubbimp.html | `braybaroque_hubbimp.html` |
| C. Petzold (17.05.2023): „Building a Virginal: Feeling Like I'm Just Getting Started…“ (HTML) | https://charlespetzold.com/blog/2023/05/Building-a-Virginal-Feeling-Like-Im-Just-Getting-Started.html | `petzold_virginal_2023-05.html` |
| Landeseiten (nur Abstract/Metadaten): PDXScholar Simmons 1974; DAEL Paté u. a. 2017 (*Acta Acustica* 103(4)); Cambridge Goodway & Savage 1992 (*MRS Bulletin*); Purdue-Artikelliste; NUŠL David 2023; Edinburgh 96075 online | siehe `abrufe/INDEX.md` | `pdxscholar_simmons_1993.html`, `dael_aaua_52387_Pate2017.html`, `cambridge_Goodway_Savage_1992_MRSBulletin.html`, `purdue_piano_articles.html`, `nusl_530824.html`, `edinburgh_96075_online.html` |

Zu jeder HTML-Datei liegt ein Textauszug `<datei>.txt` daneben (nur zum Lesen erzeugt). Seitenbilder unter `seiten/`.
Hilfsskripte: `holen.py` (Abruf mit Ablage), `ps_text.py` (Text aus PostScript), `ps_seite_roh.py`, `gw_abb9.py`
(Abb. 9 von Giordano & Winans aus den Vektorkoordinaten).

---

## Ergebnis vorweg

1. **Ablösekraft:** Neu ist **eine gemessene Kraft an einer benannten Taste**: Di Veroli 2012 misst an **c′** (Unterman.
   eines französischen Doppelmanuals) mit Gewichten in 1-g-Schritten die **Nettokraft des 8′-Kiels an der Taste**:
   Spitze ≈ **67 g ≈ 0,66 N**, des 4′-Kiels ≈ **38 g ≈ 0,37 N** [aus Abbildung abgelesen]. Mit Übersetzung 1,0–1,15
   folgt am Springer ≈ **0,57–0,66 N** [eigene Rechnung] — gegen GZ13 a (2 N an C4) **≈ 9,6–10,9 dB** weniger, im
   Einklang mit Chadefaux (G4) und Modell K6 aus Bericht 14 (0,57 N an C4). Giordano & Winans messen gegen eine
   **starre Metallklinge** eine Spitze von ≈ **1,37 N** (Text: „of order 1 N“) — das ist der Grenzfall „Saite unendlich
   steif“, kein Wert an einer Saite. Der Zupfanteil β an G4 bei Chadefaux 2013 bleibt **ungenannt**.
2. **Mechanikgeräusch (MacRitchie & Nuti 2015):** Alle Zahlen aus Bericht 04 stehen so im Volltext [wörtlich, am
   Seitenbild]. **172 Hz ist nach der Quelle selbst der Grundton von F3** („F3 = 172 Hz“, S. 5) — die Spitze des
   „Klopfens“ liegt also genau auf dem Grundton der Saite. Der dB-Bezug ist nicht genannt; ob Dämpfer gehoben waren,
   steht nicht da.
3. **Lage der Springer am Taskin 1788:** Der Satz „the jacks of the upper keyboard are closer to the bridge“ ist eine
   **allgemeine Aussage** der Einleitung über französische Cembali der zweiten Hälfte des 18. Jh., ohne Quelle und
   ohne Messung am Taskin 1788.
4. **Lautenzug (Bray Baroque):** Bericht 04 hat die Maße verwechselt. **4–6 mm ist die Dicke quer zur Saite**
   („left-to-right distance“); die Länge **längs der Saite** („PAD WIDTH … the length of string that will be touched“)
   steht in einer eigenen Tabelle: **1–4 mm („singing“) bzw. 2–6 mm („damping“)**, vom Bass zum Diskant fallend. Die
   Polsterkante liegt bündig am Sattel. Damit liegt die Mitte der Berührung bei **0,5–2 mm** („singing“) bzw. **1–3 mm**
   („damping“), an c‴ bei **0,5 mm bzw. 1 mm** [eigene Rechnung] — nicht 2–3 mm. Gemessene Abklingzeiten mit/ohne
   Lautenzug: **nicht gefunden**.
5. **Zupfpunkte 4′ / zweites 8′ an französischen Cembali:** **nicht gefunden**. Di Veroli „Taskin Scalings“ gibt nur
   Längen (4′-Werte von Bericht 04 jetzt [wörtlich] bestätigt).
6. **Innere Dämpfung Draht:** Simmons 1974 weiter gesperrt (PDF 403); Abstract jetzt [wörtlich]. Keine andere Messung
   mit Vakuum gefunden.

---

## 1. Ablösekraft und Zupfanteil an gemessenen Tasten

### 1a. Tabelle

| Instrument | Taste | β / Zupfstelle | Saite (Zug, Länge) | Kraft oder Auslenkung | Quelle, Seite | Marke |
|---|---|---|---|---|---|---|
| Cembalo, nicht benannt (Paris, Musée de la musique beteiligt); Kiel Delrin | G4 („fundamental frequency is 392 Hz“) | **nicht genannt** (PDF-S. 1 als Bild, Textsuche nach „pluck“, „position“, „z0“, „β“, „nut“, „bridge“: β nur als Symbol in K = T/(L_S β(1 − β))) | 85,7 N, 47,5 cm | y_s 0,71 / 0,90 mm (langsam / schnell) → F ≈ 0,71 / 0,89 N **nur mit β = 0,259 angenommen** | Chadefaux, Le Carrou, Le Conte, Castellengo 2013, S. 155 (Wahl G4), S. 158 Tab. 1/2 | Wahl G4 [wörtlich, am Seitenbild]; Tab. [wörtlich] (Bericht 14, am Bild geprüft); Kraft [eigene Rechnung, Bericht 14] |
| französisches Doppelmanual, nicht benannt (Messinstrument des Verfassers) | **c′**, Untermanual | nicht genannt | nicht genannt | **8′-Kiel netto ≈ 67 g** bei 4,1 mm Tastentiefgang; **4′-Kiel netto ≈ 38 g** bei 5,0 mm; Spielgewicht der Mechanik ≈ 15–18 g; Summe Spitze „106.6g“ (8′ zuerst) bzw. „84.6 g“ (4′ zuerst) | Di Veroli 2012, *HFP* 16(2), S. 9 (Methode), S. 10 Abb. 1/2 und Text | Methode und 106,6 / 84,6 / 80,2 g [wörtlich, am Seitenbild]; 67 / 38 / 15–18 g [aus Abbildung abgelesen, 250 dpi, Raster 20 g, ± 3 g] |
| dto. | c′ | – | – | Tastentiefgang bis zum Zupfen: „the 4′ plucking action requires much less weight and key dip (2.6mm) than the 8′ (4.2mm)“ | Di Veroli 2012, S. 10 | [wörtlich, am Seitenbild] |
| Laboraufbau (Klaviertaste, Kunststoff-Springer, Delrin 0,5 mm) gegen **Metallklinge 0,5 mm auf Kraftsensor** | – | – | (keine Saite) | Text: „The maximum force was found to be of order 1 N, which is also a typical key-force [11]“; Abb. 9 (Achse „force (N)“ 0–2,0): Spitze **1,37 N**, Anstieg über ≈ 30 ms | Giordano & Winans, Vorabdruck, S. 6 (Text) und Abb. 9 | Text [wörtlich] (Textebene der PS-Datei, Ligaturen fehlen); 1,37 N [aus Abbildung, Vektorkoordinaten mit `gw_abb9.py`, Achse an Teilstrichen 0/1,0/2,0 N geeicht] |
| dto., mit Saite | – | „approximately 10% of the string length from the [agraffe]“; Vergleich 5 % und 15 % | Stahl 0,2 mm, 53 cm (300 Hz); Messing 0,5 mm, 59 cm (150 Hz); Messing 0,5 mm, 2,00 m (35 Hz) | Saitenauslenkung beim Ablösen (Modell, F0 an Abb. 9 geeicht): „≈ 0.4 mm, which agreed with direct observation in the experiments“ (Messing 0,5 mm, 59 cm) | Giordano & Winans, S. 5–6, Bildunterschriften Abb. 10–14 | [wörtlich] (Textebene) |
| Ross-Cembalo (Nachbau, deutsche Größe), 2 × 8′ gekoppelt | **nicht genannt** | – | „c'' length of 375 mm“; Zug nach Umbau „reduced by 12%“ | „a 2x8' plucking force of 160 g“ → „the plucking force is now 130 g“ | Sankey 2000, action.html, Abschnitt „Action weight“ | [wörtlich] (Rohtext) |
| Virginal (Zuckermann Troubadour), Celcon-Kiele, frisch eingesetzt, **nicht intoniert** | B♭3 … F5 (20 Tasten) | – | – | „Touchweight“ (Gesamtkraft, „First Peak“ eines Kraftmessers): 94 g (C4) … 240 g (C♯5); C4 94, G4 210, C5 165 g; „The keys that are about 100 grams seem about right“ | Petzold 2023 (Tabelle im Blog) | [wörtlich] (Rohtext); nur als Rahmen, Verfasser nennt die Werte „not good“ |
| Modell mit drei Tasten, Kraftsensoren (FSR) unter den Springern | – | – | – | **nicht gelesen** (PDF 32,4 MB) | Hamilton, Russo, Webb, Ducceschi 2025, Forum Acusticum, Beitrag 000046 | Inhalt [nur Zusammenfassung] der Suchmaschine; Autoren ebenfalls nur aus der Zusammenfassung |

**Zitate im Wortlaut:**

- Di Veroli 2012, S. 9: „The measurements reported below were obtained using calibrated weights, at first one gram apart and
  checked electronically to an accuracy of .10 gram.“ — „With both stops off, yielding only the ‚action weight‘ produced by
  the key and the jacks, the key is lowered by carefully adding successive weights, measuring key dip in tenths of a
  millimetre with a calliper“ — „the action weight is subtracted from the other measurements, yielding the net weights
  produced by the solo 8′ and the solo 4′.“ — „The curves in this study were derived from a French double harpsichord, and
  although absolute values vary for different instruments, their shape and proportions are the same“. [wörtlich, am
  Seitenbild]
- Di Veroli 2012, S. 10: „it rises very steeply reaching a maximum of 106.6g.“ [wörtlich, am Seitenbild]
- Di Veroli 2012, S. 12: „The measurements above were made on one instrument.“ [wörtlich, am Seitenbild]
- Sankey: „I am certain that it was unusual for a pre-19th century harpsichord to require a key force greater than 150 g
  even on plein jeu“; Übersetzung: „Several surviving 16th-century Flemish instruments make their 2x8' jacks move 1.35
  times as much as the finger. 1.2 or greater was common for Italian instruments. … A 1:1 ratio is common in 18th-century
  keyboards, although as high as 1.15 is found in some upper manuals of this time. … it's now about 1.15 for the main 8'.“
  Springer: „Early jacks typically weigh less than 5 g.“ — „The Boston plastic jacks used by Ross weigh 6 g“. [wörtlich]
  — Damit sind die `[Werkzeug]`-Zeilen von Bericht 04 §1a zu Sankey **bestätigt**; eine Taste nennt Sankey nicht.

### 1b. Was das für den Maßstab heißt [eigene Rechnung]

- Di Veroli c′, 8′: 67 g · 9,81 = **0,66 N** an der Taste, netto (ohne Mechanikgewicht). Am Springer nach virtueller
  Arbeit F_Springer = F_Taste / Übersetzung: bei 1,0 („common in 18th-century keyboards“, Sankey) **0,66 N**, bei 1,15
  **0,57 N**. Reibung in Führung und Waagebalken steckt mit darin (Obergrenze für die Ablösekraft). Wo auf der Taste die
  Gewichte lagen, sagt die Quelle nicht.
- Gegen GZ13 a (2 N an C4): 20 lg(2 / 0,66) = **+9,6 dB**, 20 lg(2 / 0,57) = **+10,9 dB**. Gegen K6 (Bericht 14, Option
  A, C4 = 0,57 N): 0 bis +1,2 dB. Gegen Chadefaux G4 (0,71 / 0,89 N mit β = 0,259): gleiche Größe.
- 4′ gegen 8′ an derselben Taste: 20 lg(38 / 67) = **−4,9 dB** Kraft. Bericht 14 §1.5 rechnete mit gleichem Kiel −0,2 …
  +1,7 dB; Di Veroli schreibt, das 4′ „has to be voiced more softly than the 8′“ (S. 9, [wörtlich]) — das Voicing, nicht
  die Saite, macht hier den Unterschied.
- Giordano & Winans: 1,37 N gegen eine **starre** Klinge ist der Grenzfall K → ∞ von K6a (Bericht 14); an einer echten
  Saite weicht diese aus, und die Kraft fällt. Die Zahl „1.5 N“ bei Chadefaux 2012 ist die gerundete Spitze dieser Kurve
  (Achse bis 2,0 N), kein Wert an einer Saite.
- **Zupfanteil β an G4 (Chadefaux 2013):** In keiner gelesenen Quelle der Gruppe genannt. Damit bleibt die Spanne
  0,71/0,89 N (β = 0,259) bis ≈ 1,0/1,3 N (β = 0,15) aus dem Kopf zu Bericht 14 bestehen.

**Antwort in einem Satz:** Die einzige gefundene Kraftmessung an einer benannten Taste eines echten Cembalos (Di Veroli,
c′, französisches Doppelmanual) gibt netto ≈ 0,66 N für das 8′ und ≈ 0,37 N für das 4′ an der Taste und stützt damit den
Maßstab von K6 (≈ 0,6 N) gegen 2 N; β an Chadefaux' G4 ist nirgends genannt.

---

## 2. Mechanikgeräusch und Springer am Taskin 1788 (MacRitchie & Nuti 2015)

Alle Stellen am Seitenbild gelesen (`seiten/MacRitchie_Nuti_2015_HarpsichordTouchTas_S000{1,5,6,7,8}_110dpi.png`).

| Größe | Wortlaut / Wert | Seite | Marke |
|---|---|---|---|
| Methode des Geräuschs | „The key mechanism sound for this particular harpsichord was recorded in isolation by moving the 8-foot lower register so the plectrum did not touch the strings as the jack traveled upwards. Key F3 was used to produce the ‚knock‘.“ | S. 7 | [wörtlich, am Seitenbild] |
| Anlass | „A pilot study … suggested that the finger-key contact noise (or possibly the mechanical noise of the jack hitting the jack rail) may be used to identify the loud or soft touch.“ | S. 7 | [wörtlich, am Seitenbild] |
| Pegel | „a loud touch had a peak magnitude of −42.5 dB at the peak frequency of 172 Hz and a soft touch had a peak magnitude of −42.0 dB at the same peak frequency. The loud touch mechanical sound showed higher amplitudes at frequencies above 9 kHz compared to the soft touch“ | S. 7–8 | [wörtlich, am Seitenbild] |
| Abklingen | „After 250 ms in each isolated knocking sound there is a 10 dB decrease in both recordings (a 10 dB decrease meaning the sound is now a 10th of its original magnitude).“ | S. 8 | [wörtlich, am Seitenbild] |
| Grundtöne | „F2 = 86 Hz, F3 = 172 Hz, F4 = 334.5 Hz, and F5 = 689.1 Hz.“ | S. 5 | [wörtlich, am Seitenbild] |
| Stimmung | „tuned by the restorer Augusto Bonza to A4 = 415 Hz using ‚Kirnberger II‘ temperament“ | S. 5 | [wörtlich, am Seitenbild] |
| Aufnahme | „a stereo-pair of microphones placed close to the harpsichord (approximately 1 m)“ | S. 5 | [wörtlich, am Seitenbild] |
| Auswertung | Kurzzeit-Fourier-Analyse, „window size of 1024 and increment of 512 samples“, Sonic Visualiser / libxtract; Tab. 1 = „Mean and SD of the magnitude of the fundamental frequency taken across the whole signal for each recorded tone“; „All values are reported in decibels (dB).“ Abtastrate nicht genannt. | S. 5, 6 | [wörtlich, am Seitenbild] |
| Tab. 1, Mittelwerte (dB), laut / leise | Lower 8-foot: F2 −42,13 / −45,33; F3 −33,60 / −35,44; F4 −34,20 / −36,75; F5 −43,45 / −47,72. Upper 8-foot: −48,89 / −52,56; −39,15 / −42,58; −37,32 / −41,06; −46,18 / −48,96. Coupled: −44,58 / −45,50; −34,24 / −38,00; −38,14 / −40,70; −44,51 / −45,23. PDB: −39,98 / −48,68; −32,11 / −36,90; −34,32 / −37,35; −41,26 / −45,98 | S. 6 | [wörtlich, am Seitenbild] |
| Zustand des Instruments | „the jacks on the Taskin are not original but are faithful copies … reconstructed in a previous restoration undertaken by Bonza in 2006“; „the registers are original by Taskin“; „the instrument was last restrung in 1980“; „the jack rail is not original and was rebuilt by Bonza in 2006“ | S. 5 | [wörtlich, am Seitenbild] |
| **Satz zur Lage der Springer** | „Harpsichords built in France in the second half of the 18th century, such as the instrument considered in this paper, typically have two 8-foot registers … The plucking point of the quill on the string in relation to the bridge differs between registers; the jacks of the upper keyboard are closer to the bridge, while the jacks of the lower keyboard are further away. Depending on the cut of the quill (the voicing of the instrument) the sound of the upper keyboard tends to be softer than the lower keyboard“ | S. 1 (Einleitung) | [wörtlich, am Seitenbild] |

Bericht 04 hat damit alle Zahlen richtig wiedergegeben; die Marke `[Werkzeug]` lässt sich durch `[wörtlich]` ersetzen.

**Lesart [eigene Einschätzung]:**

- **Dämpfer:** Ob die Dämpfer gehoben waren, sagt die Quelle nicht. Ausgerückt wurde das **Register** (die Springerleiste
  wird seitlich verschoben, sodass der Kiel die Saite verfehlt). Der Dämpfer sitzt am selben Springer; bei ausgerücktem
  Register liegt er üblicherweise ebenfalls neben der Saite. Die F3-Saite des unteren 8′ wäre dann **ungedämpft** und
  frei, auf jeden Stoß zu antworten. Ob das 4′ (ebenfalls Untermanual) dabei an- oder abgestellt war, steht nicht da.
- **172 Hz:** Die Quelle nennt selbst F3 = 172 Hz. Die Spitze des „Klopfens“ liegt also auf dem Grundton dieser Taste —
  das spricht stark dafür, dass das gemessene Maximum die **mitklingende Saite** ist und nicht die Mechanik allein. Ein
  Mechanikgeräusch an sich hätte keinen Grund, genau dort sein Maximum zu haben. (172 Hz liegt 7,6 % über F3 bei A4 =
  415 Hz gleichstufig (164,8 Hz); woher die Abweichung kommt, sagt die Quelle nicht.)
- **dB-Bezug:** Keiner genannt; Tab. 1 mittelt über das ganze Signal, das Geräusch ist eine Spektralspitze. Beide stammen
  aus derselben Kette (Mikrofone, FFT), sind also wohl auf denselben Vollausschlag bezogen — die Differenz ≈ 9 dB gegen
  F3 laut im unteren 8′ bleibt aber ein Vergleich von Spitze gegen Mittelwert.
- **„10 dB … a 10th of its original magnitude“:** 10 dB sind Faktor 10 in der Leistung, Faktor 3,2 in der Amplitude. Was
  „magnitude“ dort meint, ist offen.
- **Lage der Springer:** Der Satz ist allgemein (französische Cembali), nennt keine Quelle und keinen Messwert am Taskin
  1788. „Bridge“ ist mehrdeutig (Steg auf dem Resonanzboden oder der Sattel auf dem Stimmstock, engl. auch „nut
  bridge“). Ob die Aussage für den Taskin 1788 gilt, lässt sich aus dem Aufsatz nicht ableiten. Zur Richtung des
  Zupfens dagegen Di Veroli: „a lower 8' jack plucks the string on its left but an upper 8' jack plucks the string on
  its right“ (hubbimp.html, Abschnitt zum „piano stop“, [wörtlich]); die Lage längs der Saite nennt er dort nicht.

**Antwort in einem Satz:** Die Zahlen stehen so im Volltext, der Geräuschgipfel liegt nach der Quelle selbst genau auf dem
F3-Grundton (172 Hz) und ist damit sehr wahrscheinlich die mitklingende Saite; der Satz zur Lage der Springer ist eine
allgemeine Aussage ohne Messung.

---

## 3. Lautenzug

### 3a. Bray Baroque (Di Veroli 2011, „Fitting a singing buff“) im Wortlaut

| Größe | Wortlaut | Marke |
|---|---|---|
| Lage | „In most harpsichords the batten is located against the nut. If yours is even a few millimetres away from the nut, there is no way you can fit a singing buff. The upper surface of the batten is normally about 1-2mm below the string. The batten should be at least 5mm wide.“ | [wörtlich] |
| Höhe | „The height of each pad—from its top to its bottom glued to the batten—is uniform, about 3mm, so that the string is touched by the middle of the pad.“ | [wörtlich] |
| **Dicke (quer zur Saite)** | „THE PAD THICKNESS. By this here we mean the left-to-right distance once the pad is glued. … If you are fitting a buff against one 8' string choir only, you can make the pads 6mm thick … each pad will be about 4mm thick.“ | [wörtlich] |
| **Breite (längs der Saite)** | „THE PAD WIDTH. By this we mean the distance between the front and the back of each pad: this is the length of string that will be touched by the pad. It is the single most important element in defining whether you are going to have a ‚singing buff‘ or a ‚damping buff‘“ | [wörtlich] |
| Breite je Lage (mm), „Damping“ / „Singing“ | CC–C# 6 / 4; D–G# 5 / 3,5; A–f# 4 / 3; g–e 3 / 2; f′–c#″ 2,5 / 1,5; d″–g‴ 2 / 1 („derived empirically by experimentation, and relate to the frequencies of harmonics to be damped“) | [wörtlich] (Tabelle im Rohtext; Notenbereiche in der Schreibweise der Quelle) |
| Kante am Sattel | „place the pad gently touching the string, the nearest border flush with the end of the buff batten, i.e. where the latter meets the nut.“ | [wörtlich] |
| Material | Leder „at least 3-4mm (millimetres) thick“ im Bass, „1mm-thin pads“ im Diskant; „For a singing buff, cloth will not do: you need leather.“ | [wörtlich] |
| Wirkung | „singing buff“: „the high partials will be damped, but the whole string hardly so“; „notes with a slow decay whereby the sound will last many seconds“; „damping buff“: „the sound will last for a few seconds only“ | [wörtlich] (keine Messung) |
| Gleichmaß | Hubbard habe „recommended to have the pads decreasing in width and to ensure that all the notes are evenly buffed“ (Kit 1970, S. 63) | [wörtlich] |
| Taskin 1769 | „buff batten works on both 8-ft registers“ | Edinburgh 96075 (lokal und online), [wörtlich] |

**Berichtigung zu Bericht 04 §3** [eigene Einschätzung, am Wortlaut]: Bericht 04 las „Polsterdicke 6 mm bzw. ≈ 4 mm“ als
mögliche Länge längs der Saite und folgerte eine Wirkstelle bei 2–3 mm. Die Quelle definiert die Dicke ausdrücklich als
„left-to-right“ (quer zur Saite; die Saiten laufen beim Cembalo von vorn nach hinten), die Länge längs der Saite ist die
„PAD WIDTH“ der Tabelle.

### 3b. Folgen für den Wirkabstand [eigene Rechnung]

Die Polsterkante liegt am Sattel, die Berührung reicht vom Sattel bis zur Breite w. Zwei Lesarten:

| Lage (Quelle) | w singing / damping mm | Mitte w/2 singing / damping | Ende w singing / damping |
|---|---|---|---|
| CC–C# | 4 / 6 | 2,0 / 3,0 | 4 / 6 |
| D–G# | 3,5 / 5 | 1,75 / 2,5 | 3,5 / 5 |
| A–f# | 3 / 4 | 1,5 / 2,0 | 3 / 4 |
| g–e | 2 / 3 | 1,0 / 1,5 | 2 / 3 |
| f′–c#″ | 1,5 / 2,5 | 0,75 / 1,25 | 1,5 / 2,5 |
| d″–g‴ (enthält c‴ = C6) | 1 / 2 | 0,5 / 1,0 | 1 / 2 |

Gegen die Verstimmung der VCSL-Aufnahmen (Bericht 14 §3.3: an C6 +5,2 Cent; L5: Δ ≈ 1731 a/L, L(C6) = 186 mm): a = 0,5 mm
→ 4,7 Cent; a = 1 mm → 9,3 Cent; a = 2 mm → 18,6 Cent. Die Lesart „Mitte, singing“ trifft an C6 genau die 0,5 mm von
GZ11 a; „Ende, singing“ oder „Mitte, damping“ gäben das Doppelte. Welches Polster (singing/damping) am VCSL-Instrument
sitzt und ob ein ganz anliegendes Polster wie ein Klemmpunkt an seiner Mitte oder an seinem Ende wirkt, sagt keine
Quelle — das ist für die LumiViz-Sitzung zu prüfen. Wichtig ist: Die Bauanleitung **widerspricht** a ≈ 0,5 mm im
Diskant **nicht**; der Widerspruch in Bericht 14 („2–3 mm der Bauanleitung“) entstand aus der falschen Lesart.

### 3c. Messungen mit und ohne Lautenzug

**Nicht gefunden.** Gesucht: Websuche „harpsichord buff stop OR lute stop measured decay spectrum with and without“;
gelesen Bray Baroque (nur Erfahrung), Sankey energy.html (Bericht 04; kein Lautenzug). David 2023 (VUT Brno, Abstract
[wörtlich] auf NUŠL: Spektren, Hüllkurven, Inharmonizität „u jednotlivých rejstříků“, kein Lautenzug erwähnt) — Volltext
hinter Abfrageseite.

**Antwort in einem Satz:** Längs der Saite misst ein Polster nach Bray Baroque 1–4 mm („singing“) bzw. 2–6 mm
(„damping“) ab der Sattelkante, der Wirkabstand liegt damit bei 0,5–3 mm und im Diskant bei 0,5–1 mm; gemessene
Abklingzeiten mit/ohne Lautenzug gibt es in keiner gefundenen Quelle.

---

## 4. Zupfpunkte des 4′ und des zweiten 8′ an französischen Cembali

| Quelle | Inhalt | Marke |
|---|---|---|
| Edinburgh 96075 (Taskin 1769), lokal und online gleich | „Three sets of strings, 2 × 8-ft, 1 × 4-ft, buff batten works on both 8-ft registers.“ — „String lengths (plucking points): F₆ 148 (91), … C₄ 711 (137), F₃ 982 (51), C₃ 1178 (162), F₂ 1443 (179), C₂ 1681 (191), F₁ 1765 (208).“ — **ein** Zupfpunkt je Taste, Register nicht genannt; F₃ „(51)“ unverändert auch online | [wörtlich] |
| Di Veroli 2011, „Taskin … Revisited“, Tab. 1 | Taskin 1769 Edinburgh: Gehäuse 2319; „Scaling of long/back/lower 8'“ F 1764, C 1617, c 1178, c′ 711, c″ 357; „Scaling of 4'“ c 605, c′ 337, c″ 163 (mm). Taskin 1774 Brüssel 4′: 606 / 340 / … Keine Zupfpunkte auf der Seite (Suche „pluck“, „jack“, „nut“). | [wörtlich] (Rohtext); bestätigt Bericht 04 §5c für 1769/1774 |
| Di Veroli 2012 | Stellt keine Zupfpunkte, nur Kraft über Tastentiefgang | – |
| MacRitchie & Nuti 2015 | Allgemeiner Satz (Abschnitt 2), keine Zahl | [wörtlich] |

**Nicht gefunden:** Zupfpunkt je Register an Taskin, Blanchet oder Hemsch. Gesucht: Websuche „Taskin harpsichord plucking
points 4' 8' jack rows distance from nut measured drawing“, „French double harpsichord ‚plucking points‘ upper 8' lower
8' 4' mm Blanchet OR Taskin OR Hemsch table“ (extended); gelesen Edinburgh 96075 (online), Di Veroli „Taskin Revisited“
und „hubbimp“. MFA Boston und Musée de la musique: keine Datenblätter mit Zupfpunkten in den Treffern. MIMO-Datensatz
(UEDIN 4329) nicht lesbar (SSL-Fehler). claviantica.com (Taskin-1769-Nachbau) abgeschaltet. Eine Suchmaschinen-
Zusammenfassung schrieb einer Quelle die Registerfolge „Rear lower-manual 8', … 4', … upper-manual 8' … permanently
engaged“ zu — im Wortlaut der beiden Edinburgh-Seiten steht das **nicht** [nur Zusammenfassung, nicht verwendet].

**Antwort in einem Satz:** Für die Zupfpunkte des 4′ und des zweiten 8′ an einem französischen Cembalo gibt es in den
erreichbaren Quellen keine Zahl; der einzige Beleg bleibt der Ruckers-Nachbau bei Fletcher & Beebe (Bericht 04 §5a).

---

## 5. Innere Dämpfung von Cembalodraht

| Quelle | Inhalt | Marke |
|---|---|---|
| Simmons, Jack Lee (1974): „Vibration Damping Characteristics of Typical Harpsichord Strings“, M.S. Physics, Portland State University, 71 S., doi 10.15760/etd.1992 | Abstract: Draht elektromagnetisch in Resonanz, Abklingzeit über der Frequenz für „iron, steel, brass, bronze, etc.“, Durchmesser „from 170 mm to 600 mm“ (so in der Quelle; gemeint offenbar µm), 20 Hz – 12 kHz; Halterungen variiert; „The energy losses due to sound radiation and viscous damping were examined by placing the vibrating wire in a vacuum.“ Schlüsse: Stahl klingt länger als Eisen; „Energy losses to sound radiation and viscous damping greatly exceed all other modes of energy loss from the wire.“ Rechte: „In Copyright“ | Abstract [wörtlich] (Landeseite); Volltext **HTTP 403** |
| Goodway & Savage (1992): „Coulomb's Data on Harpsichord Wire“, *MRS Bulletin* | Auszug: „During an investigation of the internal friction of several antique music wires by means of a torsion pendulum …“; Proben aus dem Vater-Cembalo 1732 (Messing rot/gelb, Eisen) | Auszug [wörtlich] (Cambridge-Landeseite); Zahlen nicht im Auszug; Volltext hinter Bezahlschranke |

Keine andere Messung mit Vakuum oder getrennten Verlusten gefunden (Websuche „harpsichord wire brass iron internal damping
loss factor measurement vacuum decay strings“). Der Pendelversuch von Goodway & Savage misst Torsion, nicht Biegung —
für η_E der Querschwingung nur bedingt übertragbar [eigene Einschätzung].

**Antwort in einem Satz:** Eine Zahl für die innere Dämpfung von Messing- oder Eisendraht war nicht erreichbar; Simmons
1974 bleibt gesperrt, und sein Abstract sagt selbst, dass Luft und Abstrahlung alle anderen Verluste weit übersteigen.

---

## Nicht gefunden (Suchwege und Fehlercodes)

| Lücke | Wo gesucht | Ergebnis |
|---|---|---|
| β oder Abstand vom Sattel an G4 (Chadefaux 2013) | lokal PDF-S. 1 (Bild), Textsuche S. 1–6; Websuchen „Chadefaux … plucking point G4 key“, „… harpsichord used string length G key Musée de la musique“, „clavecin … sol4 point de pincement“, „Arthur Givois thèse clavecin“, „Paté Givois Le Carrou Roy harpsichord“; Roy 2015 lokal (Textsuche) | nicht genannt; Paté u. a. 2017 JASA: HAL Anubis-Abfrage, Acta Acustica 103(4): nur Abstract (dort „minimum triggering force on the key“ gemessen, Werte nicht im Abstract) |
| Giordano & Winans 1999, frei | Purdue-Artikelliste → `articles/pluck.ps` | **gefunden** (Vorabdruck); gedruckte Fassung nicht verglichen |
| FA2025 000046 | `holen.py` (Abbruch, 32 396 919 Bytes > 20 MB); `curl -I` (200, `application/pdf`); unibo komprimiert und Landeseite: HTTP 403 | nicht gelesen |
| Sankey: Taste | action.html Rohtext | keine Taste genannt |
| Lautenzug gemessen | s. §3c | nicht gefunden |
| Zupfpunkte 4′ / zweites 8′ | s. §4 | nicht gefunden |
| Innere Dämpfung | s. §5 | gesperrt / nicht gefunden |

---

## Ladeliste (nur Gesperrtes; nichts davon geladen)

| Vorschlag Dateiname | URL | Größe | Lizenz / Vermerk | Wozu |
|---|---|---|---|---|
| `Hamilton_etal_2025_ModalHarpsichordSensors_FA2025-000046.pdf` | https://dael.euracoustics.org/confs/fa2025/data/articles/000046.pdf | 32 396 919 Bytes | Forum Acusticum 2025; **über der 20-MB-Grenze von CLAUDE.md** — braucht ausdrückliche Freigabe | Kraft (FSR, N) unter Springern eines Modells mit drei Tasten; Bezug zu echtem Kiel offen |
| `Simmons_1974_DampingHarpsichordStrings_MS_PSU.pdf` | https://pdxscholar.library.pdx.edu/cgi/viewcontent.cgi?article=2993&context=open_access_etds | 3,5 MB (Angabe Bericht 04) | „In Copyright“ (Landeseite); HTTP 403 (Abfrage) | Abklingzeiten je Frequenz Eisen/Stahl/Messing/Bronze, Luft und Vakuum |
| `Pate_Givois_LeCarrou_Roy_2017_PlectrumShapeJackVelocity_JASA141.pdf` | https://hal.sorbonne-universite.fr/hal-01565218 | – | HAL: Anubis-Abfrage; laut Suchmaschine dort keine Datei hinterlegt [nur Zusammenfassung] | Zupfstelle und Kraft am Instrument der Pariser Gruppe |
| `Pate_etal_2017_PerceptionPlectraVoicing_ActaAcustica103.pdf` | https://doi.org/10.3813/AAA.919097 (HAL hal-01565228) | – | Verlag/HAL (Abfrage) | „minimum triggering force on the key“ für zwei Kielsätze über den Umfang — **wahrscheinlich die gesuchte Kraft je Taste** |
| `Goodway_Savage_1992_CoulombHarpsichordWire_MRSBulletin.pdf` | https://www.cambridge.org/core/journals/mrs-bulletin/article/coulombs-data-on-harpsichord-wire/62C0058D52D2AB887B9B9ABA1D05EDAF | – | Bezahlschranke; eine Kopie in einem privaten Zotero-Spiegel (isidore.co) wurde **nicht** geladen (Rechte unklar) | innere Reibung antiker Drähte (Torsion) |
| `ICSV_fullpaper636_HarpsichordQuill_Bologna.pdf` | https://cris.unibo.it/retrieve/369030a5-c29f-48c4-8cd0-6e9b441da66d/full_paper_636_20240430213941625.pdf | – | HTTP 403 | Kiele aus Raben- und Schwanenfeder (laut Suchmaschine) |
| `David_2023_SpektraADSRCembala_BP_VUT.pdf` | http://hdl.handle.net/11012/212594 | – | VUT: Abfrageseite | Inharmonizität je Register, Hüllkurven |

---

## Am Ende

**Verlangt und nicht geliefert:**
- β bzw. Abstand vom Sattel an G4 bei Chadefaux 2013 (in keiner erreichbaren Quelle der Gruppe).
- Inhalt FA2025 000046 (über 20 MB).
- Gemessene Abklingzeiten oder Spektren mit/ohne Lautenzug.
- Zupfpunkte je Register (4′, zweites 8′) an Taskin, Blanchet, Hemsch.
- Innere Dämpfung von Messing/Eisen (Simmons gesperrt).

**Muss die LumiViz-Sitzung selbst nachprüfen:**
1. Di Veroli 2012 Abb. 2 am Seitenbild (`seiten/DiVeroli_2012_S0004_Abb1-2_250dpi-4.png`): 8′ ≈ 67 g, 4′ ≈ 38 g netto an
   c′ (Ablesung ± 3 g); Übersetzung und Lage der Gewichte auf der Taste sind nicht angegeben (Umrechnung 0,57–0,66 N
   setzt 1,0–1,15 voraus).
2. Ob die Lesart „Wirkstelle = Mitte des Polsters“ oder „= Ende des Polsters“ zur Verstimmung der VCSL-Aufnahmen passt
   (§3b; an C6 0,5 gegen 1 mm), und welche Art Polster (singing/damping) das Vorbild hat.
3. MacRitchie & Nuti: Die Deutung „172 Hz = mitklingende F3-Saite“ ist meine Einschätzung; die Quelle sagt nicht, ob der
   Dämpfer gehoben war. Für den Tastenstoß bleibt deshalb kein Pegel der Mechanik allein belegt.
4. Giordano & Winans: Die Zahlen stammen aus dem Vorabdruck (Textebene ohne Ligaturen; Abb. 9 aus Vektorkoordinaten, nicht
   als Bild gesehen — PostScript ließ sich mit den erlaubten Werkzeugen nicht rendern). Die gedruckte Fassung (JSV 224)
   kann abweichen.
5. Freigabe für FA2025 000046 (32 MB) und für die Paté-Arbeiten 2017 (die „minimum triggering force“ je Taste ist der
   aussichtsreichste offene Beleg für den Gang über den Umfang).

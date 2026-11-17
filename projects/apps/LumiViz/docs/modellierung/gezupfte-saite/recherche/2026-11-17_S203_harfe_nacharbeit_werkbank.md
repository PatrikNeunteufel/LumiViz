> **Herkunft:** Bericht der Research-Werkbank `…\cmake\LumiViz_Research`, Auftrag `2026-11-17_22_harfe_nacharbeit` (gestellt in LumiViz S202,
> Nacharbeit zu den Berichten 02 und 12), 1 Segment: 2026-11-17 13:05–13:32, claude-opus-5-5, 127 Runden, success, 6.58 USD; Status `fertig`
> (regulär beendet). **Unverändert abgelegt in S203** — nur dieser Kopf ist dazugekommen.
>
> **Gegengelesen in S203** (Seitenbilder des Laufs; Rechnung `proben/2026-11-17_S203_gegenprobe_bericht22.py` aus den
> Tabellenwerten des Berichts, Ausgabe `.txt` daneben):
> - **Am Seitenbild — stimmen:** Bell S. 149 (Stufen 1–4, „completed“ harp, Bezug „116dB = 1 Ω⁻¹ (or 1 s/kg)“); S. 159
>   (Anstieg vom Bass zum Diskant, zwei Spitzen B2 und S(1,3), 15 dB an den Saiten 31 / 32, im Mittel 3 dB); Abb. 4.9,
>   Titel „The Helmholtzian Mode on the Soundbox with all Apertures Open (At 190 Hz, Q=6)“ — **die Abbildung zeigt
>   Schalldruckprofile über die Länge des Kastens, keine Resonanzkurve; die Güte steht nur im Titel**; Abb. 7.12
>   (S(1,3)): ohne Saiten 316 Hz Q 45, mit Saiten 334 Hz Q 27, Extremwerte wie Tabelle 1.3.
> - **Nachgerechnet — stimmt:** Bezug 20 lg(1 / 1,58·10⁻⁶) = 116,03 dB (S. 133 gegen S. 149 stimmig); Reihenfeder: Linie
>   5,2·10⁴ N/m und kleinste zulässige Feder an allen zehn Moden wie Tabelle 1.4, weichste 1,19·10⁵ N/m, ihr Beitrag an
>   den Saiten aus Abb. 7.22 70,7 … 90,3 dB, also 7–14 dB unter Bell; Steigungen der Extremwerte über die Moden 5,5 / 6,6
>   (ohne Saiten) und 7,0 / 8,9 dB/Oktave (mit); Güten aus Le Carrou Tab. 4.1 19,1 / 19,5 und aus ζ der JASA-Abb. 4 21,7 /
>   35,7. Abb. 7.6 an den Stützstellen der Tabelle 1.2, 200–790 Hz: +3,9 (mit) / −2,2 dB/Oktave (ohne; der Bericht −2,5
>   aus der vollen Spalte), Energiemittel 400–790 Hz +9,2 dB (Bericht +9,5).
> - **Schluss geprüft:** Eine Feder in Reihe setzt ω/k nur ohne Auslöschung als Untergrenze; der Bericht nimmt das Minimum
>   je Profil als Wert ohne Auslöschung. An zehn Profilen und an Ort 17 zugleich trägt das; „widerlegt“ gilt für eine
>   Reihenfeder als Ursache des Pegels, nicht für jeden Fehler im Messweg (so auch 1.6).
> - **Zu KQ29 a:** Bells Q 6 gilt am starren, in Sand eingegrabenen Kasten mit sechs offenen Öffnungen — Abstrahlung und
>   Verluste an Wand und Sand eingeschlossen, nicht die Güte einer gekoppelten Mode; Q 9–11 (Bericht 12) gelten je
>   einzelner Öffnung. Die Güte der Moden um 190–210 Hz an der besaiteten Salvi ist bei Bell „indeterminate“.
> - **Nicht gegengelesen:** die Digitalisierung von Abb. 7.6 und 7.21 (`d1`, `d2`, `d4`) Pixel für Pixel; die Profile 7.8–7.11
>   und 7.13–7.17 einzeln; Le Carrou Tab. 4.1 und JASA 2007 Abb. 4 nicht neu am Bild (aus S202 bekannt); Bell & Firth (IOA
>   S. 65) und SMAC 83 S. 169 nicht am Bild.
> **Ablage:** unter `…\cmake\quellen\gezupfte-saite\` (Herkunft in `quellen\UMZUG_S203.tsv`):
> `Fabre_LeCarrou_Chomette_LeBomin_2023_CentralAfricaHarps_FA2023.pdf` (im Bericht `FA2023_000712.pdf`),
> `Bell_Firth_oJ_HarpSoundboardSoundbox_ProcIOA.pdf` (im Bericht `Bell_Firth_IOA_HarpSoundboardSoundbox.pdf`),
> `SMAC83_1985_Volume2_FirthBell_HarpSoundboardSoundbox.pdf` (ganzer Band; im Bericht `SMAC83_Volume2.pdf`). `Bell_Firth_IOA.pdf` ist
> die HTML-Fehlerseite (404) und bleibt als Beleg beim Lauf.
> Der Auftrag liegt seit S203 im Archiv der Werkbank: `archiv/auftraege/2026-11-17_22_harfe_nacharbeit.md`, Ergebnisordner
> `archiv/ergebnisse/2026-11-17_22_harfe_nacharbeit/` (darin `seiten/`), Protokolle unter `archiv/laeufe/`. Wo der Bericht `ergebnisse/…`
> oder Seitenbilder nennt, ist `archiv/ergebnisse/…` gemeint.
> **Rechnungen des Laufs** (unverändert, Pfade zeigen auf die Werkbank) im Ordner daneben `2026-11-17_S203_harfe_nacharbeit_werkbank/`.

# Bericht: Nacharbeit Harfe — Messweg bei Bell, Güten der besaiteten Harfe, offene Quellen

- **Auftrag:** `auftraege/2026-11-17_22_harfe_nacharbeit.md`
- **Datum:** 2026-11-17 · **Modell:** claude-opus-5-5 · **Budget:** 10 USD, verbraucht rund 6 USD
- **Voraussetzung:** Bericht 12 (`archiv/ergebnisse/2026-11-16_12_harfe_grundlagen/bericht.md`) und Bericht 02 liegen
  im Archiv vor; beide gelesen, dazu die Köpfe aus S202 im LumiViz-Repo.
- **Reichweite:**
  - **Bell 1988** (`quellen\gezupfte-saite\Bell_1988_AcousticalInvestigationConcertHarp_PhD.pdf`), am Seitenbild
    gelesen: PDF 161–162 (Abb. 4.8 iv–vi, 4.9), 237–239, 242–245 (§6.3, §6.4), 256–258 (Abb. 6.1–6.3), 275–279
    (§7.4, §7.5), 285 (§7.8), 293–303 (Abb. 7.6, 7.8–7.17), 308 (Abb. 7.21, Bild aus Bericht 02 neu gerendert).
    Suchen mit `pdf_suche.py` in PDF 125–160, 228–292.
  - **Le Carrou 2006** (Thèse): Tab. 3.2 (PDF 66) und Tab. 4.1 (PDF 80) an den Bildern aus dem Archiv.
  - **Le Carrou, Gautier & Foltête 2007:** S. 561–562 (PDF 3–4) am Bild; Tab. I/II in der Textebene (`lc2007.txt`
    aus dem Archiv).
  - **Le Carrou, Gautier & Badeau 2009** (ACTA): nur Suche. **Chadefaux 2012/2013, Le Carrou/Gautier/Badeau ICA
    2007:** nur Suche (für Teil 4).
  - **Netz:** 5 WebSearch, 1 WebFetch, 6 Aufrufe `curl` (davon 1 HTTP 404, 1 Abfrage beim Webarchiv, 1 nur Kopfzeilen).
    Roh gelesen und unter `abrufe/` abgelegt: 3 PDFs (siehe `abrufe/INDEX.md`).
  - **Nicht erreicht:** Bell 1997 (J. Catgut Acoust. Soc. 3(3), 2–8): kein freier Volltext. Daltrop 2009 (UBC,
    Honours Thesis): kein Volltext gefunden. Bells Admittanzkarten Abb. 7.3/7.4 (PDF 290–291) **nicht** an den 24
    Saitenorten abgelesen (zu dicht für ein Ablesen von Auge mit vertretbarer Genauigkeit; siehe „Nicht geliefert“).
- **Skripte** (in diesem Ordner):
  - `d1_abb76_geometrie.py`, `d2_abb76_kurven.py`: Abb. 7.6 spaltenweise digitalisiert;
  - `d3_profile_reihenfeder.py`: Profile 7.8–7.17 und die Prüfung „Reihenfeder“;
  - `d4_abb721_steigung.py`: Abb. 7.21 zeilenweise digitalisiert;
  - `d5_index.py`: Prüfsummen der Abrufe.
  Seitenbilder unter `seiten/` (`.png`, dazu die Graustufen `bell293_120-293.pgm` und `bell308_110-308.pgm`).

Marken: `[am Bild geprüft]` (Wortlaut oder Wert am Seitenbild gelesen), `[wörtlich]` (Wortlaut aus der Quelle, so in
Bericht 02 oder 12 schon belegt), `[Textebene]` (nur `pdftotext`), `[digitalisiert]` (Kurve per Skript aus dem
Seitenbild abgegriffen; Genauigkeit steht dabei), `[Werkzeug]`, `[nur Zusammenfassung]`, `[eigene Rechnung]`,
`[eigene Einschätzung]`.

---

## Kurzfassung

1. **Der Bezug der besaiteten Messungen ist belegt.** Bell S. 149 nennt für alle Admittanzmessungen in Kap. 7
   „116dB = 1 Ω⁻¹ (or 1 s/kg)“ `[am Bild geprüft]`. Abb. 7.22 trägt den Bezug nicht an der Achse, gehört aber zu
   diesem Kapitel. Die Folge aus dem S202-Kopf („Bezug ist dort nicht angegeben“) ist damit erledigt.
2. **Eine Reihenfeder im Messweg als Ursache der Pegel in Abb. 7.22 ist widerlegt.** `[eigene Rechnung aus Werten
   am Bild]`
   - Eine Feder k in Reihe setzt eine Untergrenze von etwa ω/k unter jede gemessene Admittanz.
   - In Bells **besaiteten** Modenprofilen (Abb. 7.8–7.17) liegen die kleinsten Werte 7–15 dB unter der Linie von
     5,2·10⁴ N/m.
   - Eine Reihenfeder kann also höchstens 1,2·10⁵ N/m weich sein. Dann trägt sie zu Abb. 7.22 nur Werte bei, die
     7–14 dB unter Bells Werten liegen.
3. **Die Steigung um 6 dB/Oktave ist keine Eigenheit der besaiteten Messung.**
   - Die Extremwerte der **unbesaiteten** Profile steigen über die Moden um 5,5 dB/Oktave (Minima) und 6,6 dB/Oktave
     (Maxima) `[eigene Rechnung]`.
   - Bell sagt es für den losen Kasten F selbst: „At all the Testing Positions, the input admittance level generally
     increases with frequency“ (S. 134) `[am Bild geprüft]`.
   - Was das Besaiten am selben Ort ändert, zeigt Abb. 7.6 (Messort 17): Unter 350 Hz bleibt der Pegel im Mittel
     gleich. Über 380 Hz heben sich die Täler um 10–17 dB, die Spitzen bleiben oder sinken. Das Bandmittel 400–790 Hz
     steigt um 9,5 dB (energetisch) `[digitalisiert]`.
4. **Antwort auf Frage 1:**
   - **Belegt** ist, dass Bells besaiteter Pegel denselben Bezug und dieselbe Messkette hat wie seine unbesaiteten
     Messungen.
   - **Widerlegt** ist die Reihenfeder als Erklärung.
   - **Nicht entscheidbar** ist, ob etwas anderes im Messweg den Pegel verfälscht. Bell nennt weder eine Kompensation
     der Masse unter dem Kraftaufnehmer noch die Steife oder Befestigung des Stifts.
   - Nach Bells eigener Vermutung (S. 153) leiten „strings and damping plastic“ Schwingung an Knoten weiter. Eine
     unabhängige Messung derselben Harfe gibt es nicht.
5. **Güte der Helmholtz-Mode mit allen Öffnungen offen:** **Q = 6** bei 190 Hz `[am Bild geprüft]`.
   - Belegt durch Bell Abb. 4.8 vi (PDF 161) und Abb. 4.9 (PDF 162, drei Lautsprecherorte).
   - Gemessen am starren, in Sand eingegrabenen Kasten.
   - Mit fünf offenen Öffnungen: Q 6 bei rund 164 Hz.
   - Die Q 9–11 aus Bericht 12 gelten **je einzelner** Öffnung.
6. **Güten der tiefen Moden, besaitet:**
   - **Salvi (Bell):** S(1,1)− und S(1,1)+ „indeterminate“. S(1,2) bis S(1,8): Q 32 / 30 / 27 / 28 / 19 / 12 / 17 / 18
     `[am Bild geprüft]`.
   - **Camac (Le Carrou), modal ausgewertet:** T1 Q 22, A0 Q 36 (Abb. 3.4 = JASA 2007 Abb. 4).
   - **Camac, aus den komplexen Eigenwerten der Tab. 4.1:** T1 Q 19,1, A0 Q 19,5 `[am Bild; eigene Rechnung]`.
7. **Offene Quellen:**
   - **Forum Acusticum 2023/000712** handelt von zentralafrikanischen Harfen (8 Saiten), nicht von der Konzertharfe;
     für die Eichung nicht übertragbar.
   - **Bell & Firth (IOA)** liegt im Webarchiv. „12 mm at the bass to 2 mm at the treble“ steht dort im Wortlaut,
     ebenso in Firth & Bell, SMAC 83, S. 169 `[am Bild geprüft]`. Besaitet gegen unbesaitet steht in keinem der beiden
     Texte.
   - **Daltrop 2009** ist nicht frei zugänglich.
8. **Abstrahlung je Kraft 220 Hz – 2 kHz mit absolutem Bezug:** nicht gefunden.

---

## 1 Messweg bei Bell

### 1.1 Was Bell über die Messkette schreibt

| Seite (PDF) | Aussage | Marke |
|---|---|---|
| S. 127 (238) | „The soundboxes were excited by a small pin connected to an impedance head which was in turn connected to a small vibrator.“ Gemessen hängend im reflexionsarmen Raum. | [am Bild geprüft] |
| S. 128 (239) | In Kap. 6 „the excitation was always at right angles to the plane of the soundbox“; Impedanzkopf B+K 8001 mit zwei Sätzen Piezoscheiben (Beschleunigung, Kraft) | [am Bild geprüft] |
| S. 128 (239) | Erreger an einem Laborstativ, dazu 2,3 kg Blei; „which with the impedance head and pin weighs 1.1kg“; weitere 4 kg ändern die Kurve „by no more than 0.6dB“; Wiederholbarkeit „less than 1dB“ | [am Bild geprüft] |
| S. 128 (239) | „The input admittance of the system is loaded by 3.4 kg, but comparison with impulse measurements showed resonances at the same frequencies.“ | [am Bild geprüft] |
| S. 128–129 | Kraft über den Kompressor des Analysators konstant geregelt; Beschleunigung zur Schnelle integriert; „As the force remains constant, the velocity response is proportional to the response of the input admittance.“ | [Textebene] (S. 129 nicht am Bild) |
| S. 130–131 (241–242) | Eichung: „a final additional frequency sweep was made with the vibrator removed from the soundbox and a 200g mass fastened to the impedance head. Using the formula given in Fig.6.1, the absolute value of the admittance of this mass could be calculated and this was used as a standard“ | S. 131 [am Bild geprüft], S. 130 [Textebene] |
| Abb. 6.1 (256) | Modell mit einem Freiheitsgrad; Feder: „A = jω/k“, Steigung „6 dB/octave“ unterhalb der Resonanz | [am Bild geprüft] |
| Abb. 6.2 (257) | Blockbild: Stift zwischen Kasten und „Vibrator with impedance head“; keine Maße, keine Befestigung | [am Bild geprüft] |
| S. 133 (244) | „0dB = 1.58 × 10⁻⁶ s/kg … 116 dB = 1 s/kg“. Am losen Kasten F unterhalb der ersten Resonanz (183 Hz) „only the steady rise in input admittance. This steady rise is expected, because the action of the soundbox below the first soundbox resonance can be modelled by the simple model shown in Fig. 6.1“ | [am Bild geprüft] |
| S. 134 (245) | „At all the Testing Positions, the input admittance level generally increases with frequency. This indicates that although there are resonances, the action of this soundbox is dominated by stiffness elements of the soundbox, which cause the admittance to increase contiuously.“ Höchster Wert 108 dB bei 660 Hz an Messort 3. | [am Bild geprüft] |
| S. 147 (273) | Besaitet: Saiten mit Kunststoffbändern gedämpft; Erreger „<10°“ schräg; Stift „as close to the centre of the cover bar as the strings would allow“ | [wörtlich] (Bericht 02, S202 gegengelesen) |
| S. 149 (275) | Stufen an Kasten G: lose, Harfe ohne Saiten, mit Saiten, Schalldruck. „completed“ = Holzbau „without tuning mechanism (including the pedals) nor with varnishing“; der Hals hatte Stimmwirbel und Stegstifte. An der fertigen Harfe keine Messwerte für die Orte 1–3 und 37–40. **„The reference level for the input admittance measurements is 116dB = 1 Ω⁻¹ (or 1 s/kg).“** | [am Bild geprüft] |
| S. 152 (278) | S(1,1)− besaitet: „raise the resonant frequency, by a factor of 10%, and … reduce the input admittance level, by as much as 7dB. The Q-value … was indeterminate because there was insuffuicient trough …“ | [am Bild geprüft] |
| S. 153 (279) | zu S(1,3), Schalldruckprofil gegen Admittanzprofil: „there could be a resonance of the strings and damping plastic that is transmitting the vibration from this nodal point.“ | [am Bild geprüft] |
| S. 159 (285) | zu Abb. 7.22: „In general, there is a rise in input admittance from the bass strings to the treble. There are only two peaks on this plot and these are due to the strings being on major active areas of two particular modes of the harp; the modes are B2 and S(1,3).“ Größter Sprung zwischen Nachbarsaiten 15 dB (Saiten 31/32), im Mittel 3 dB. | [am Bild geprüft] |

**Nicht gefunden (Kap. 6.3):** Kompensation der Masse unter dem Kraftaufnehmer, Steife, Länge oder Werkstoff des
Stifts, Befestigung am Kasten (geklebt, Wachs, geschraubt).
- Gesucht mit `pdf_suche.py` nach „pin“, „wax“, „glue“, „mass cancel“, „compensat“, „stiff“ in PDF 228–250. Treffer
  gab es nur für „pin“ und „stiff“ an den Stellen oben.
- Dazu die Seiten 237–245 ganz am Bild gelesen.
- Drei Seiten im Suchbereich haben keine Textebene: Abb. 6.1–6.3, ebenfalls am Bild gelesen.

**Was die Eichung leistet** `[eigene Einschätzung]`: Die 200-g-Masse eicht den Faktor zwischen Kraft- und
Beschleunigungssignal. Eine Masse unter dem Kraftaufnehmer, die Bell nicht nennt, liegt bei der Messung parallel zum
Kasten. Sie **senkt** die Admittanz mit steigender Frequenz, kann sie also nicht heben. Eine Nachgiebigkeit im Stift
würde sie heben (Abschnitt 1.4).

### 1.2 Abb. 7.6 — Messort 17 an Kasten G, lose / ohne Saiten / mit Saiten (PDF 293)

Messort 17 liegt bei 80 cm auf der Mittelleiste, zwischen den Saitenösen 20 und 21 (Abb. 6.3, PDF 258)
`[am Bild geprüft]`.
- Die drei Admittanzfelder haben dieselbe dB-Teilung: 10 dB = 89 px bei 120 dpi, Gitterlinien 70/80/90 dB
  `[digitalisiert, d1]`.
- Die Frequenzachse ist nur am ersten Feld beschriftet (100–800 Hz, linear). Die Ticks der beiden anderen Felder
  stehen an denselben Stellen; dieselbe Achse ist also angenommen.
- **Rasterweite:** eine Pixelspalte = 1,13 Hz; jeder Wert ist über ±2 Spalten gemittelt (±2,3 Hz).
- **Genauigkeit** ±2 dB, an steilen Flanken bis ±5 dB. Lose Kasten unter 120 Hz durch die Achsbeschriftung gestört,
  nicht verwenden.
- Bezug 116 dB = 1 s/kg (S. 149).

| f [Hz] | lose | ohne Saiten | mit Saiten | mit − ohne |
|---|---|---|---|---|
| 140 | 75,2 | 78,6 | 76,3 | −2,3 |
| 170 | 69,2 | 69,8 | 79,1 | +9,3 |
| 200 | 80,6 | 78,6 | 81,6 | +3,1 |
| 230 | 79,6 | 84,7 | 84,9 | +0,2 |
| 260 | 94,0 | 88,5 | 86,1 | −2,4 |
| 290 | 69,7 | 67,6 | 77,6 | +9,9 |
| 320 | 74,2 | 83,5 | 86,9 | +3,3 |
| 350 | 76,6 | 78,3 | 78,0 | −0,3 |
| 380 | 67,7 | 64,9 | 81,2 | +16,3 |
| 410 | 74,5 | 69,2 | 83,3 | +14,1 |
| 440 | 82,9 | 80,2 | 81,0 | +0,8 |
| 470 | 64,7 | 71,4 | 84,0 | +12,6 |
| 500 | 74,6 | 73,6 | 85,2 | +11,5 |
| 530 | 75,8 | 78,3 | 88,4 | +10,1 |
| 560 | 82,4 | 85,1 | 82,8 | −2,3 |
| 590 | 68,6 | 77,7 | 84,5 | +6,8 |
| 620 | 69,2 | 68,9 | 86,2 | +17,2 |
| 650 | 73,9 | 73,3 | 87,0 | +13,7 |
| 680 | 75,7 | 74,6 | 89,1 | +14,4 |
| 710 | 78,8 | 77,6 | 88,1 | +10,5 |
| 740 | 76,6 | 77,4 | 89,1 | +11,7 |
| 770 | 79,5 | 76,9 | 89,7 | +12,8 |
| 790 | 75,6 | 77,4 | 90,3 | +12,8 |

Alle Werte in dB re Bell, `[digitalisiert, d2]`.

| | lose | ohne Saiten | mit Saiten |
|---|---|---|---|
| Regression über log₂ f, 110–790 Hz | −1,2 dB/Okt. | −0,6 dB/Okt. | **+4,3 dB/Okt.** |
| Regression 200–790 Hz | −2,2 | −2,5 | **+3,9** |
| Bandmittel 110–200 Hz (arithm. / energ.) | 76,1 / 78,4 | 74,1 / 77,7 | 77,9 / 78,9 |
| Bandmittel 200–400 Hz | 76,6 / 83,2 | 77,5 / 83,7 | 82,4 / 83,6 |
| Bandmittel 400–790 Hz | 75,3 / 78,3 | 75,4 / 77,5 | 86,3 / 87,0 |
| Spanne 400–790 Hz | 64,3–95,1 | 66,1–85,6 | 80,9–90,7 |

Die Spannen sind `[digitalisiert, d2]`.

**Befund** `[eigene Rechnung]`:
- An Ort 17 steigt die besaitete Kurve, die unbesaitete nicht. Die besaitete Steigung ist aber 3,9–4,3 dB/Oktave,
  nicht 6.
- Der Unterschied entsteht über 380 Hz: Die Täler steigen dort um 10–17 dB, die Spitzen bleiben gleich hoch oder
  fallen (560 Hz −2 dB).
- Unter 350 Hz ist das Energiemittel besaitet und unbesaitet gleich (+1,2 / −0,1 dB).
- Das ist Bells „more even“ (S. 150) in Zahlen.

### 1.3 Abb. 7.8–7.17 — Modenprofile ohne und mit Saiten (PDF 294–303)

Jede Abbildung zeigt die Admittanz einer Mode über die Messorte 36…1 (Bass → Diskant), bei der Frequenz der Mode:
lose / fertige Harfe ohne Saiten / mit Saiten / Schalldruck.
- Die Nummern 7.8–7.13 und die Moden sind am Bild und im Text (S. 151–153) geprüft.
- Für PDF 300–303 ist die Nummer am 80-dpi-Bild unscharf. Gemeint sind wohl 7.14–7.17, sicher sind die Moden in den
  Titeln.
- Abgelesen sind der kleinste und der größte Wert je Feld, ±1,5 dB (Teilung je Feld aus den beschrifteten Linien,
  5,8–9,2 px/dB). Bezug 116 dB = 1 s/kg (S. 149).

| PDF | Mode | ohne Saiten: f [Hz], Q | min … max [dB] | mit Saiten: f [Hz], Q | min … max [dB] | Δmin / Δmax |
|---|---|---|---|---|---|---|
| 294 (7.8) | S(1,1)− | 170, 11 | 68,0 … 86,3 | 187, unbestimmt | 68,2 … 79,3 | +0,2 / −7,0 |
| 295 (7.9) | S(1,1)+ | 216, 16 | 66,4 … 81,6 | 207, unbestimmt | 76,8 … 86,5 | +10,4 / +4,9 |
| 296 (7.10) | S(1,2) | 249, 39 | 65,7 … 96,0 | 257, 32 | 72,5 … 92,3 | +6,8 / −3,7 |
| 297 (7.11) | S(1,2)′ | 249, 39 (dasselbe Feld wie 7.10) | 65,7 … 96,0 | 284, 30 | 75,1 … 91,5 | +9,4 / −4,5 |
| 298 (7.12) | S(1,3) | 316, 45 | 75,7 … 104,5 | 334, 27 | 80,7 … 101,0 | +5,0 / −3,5 |
| 299 (7.13) | S(1,4) | 432, 43 | 73,5 … 105,3 | 430, 28 | 80,7 … 99,7 | +7,2 / −5,6 |
| 300 | S(1,5) | 545, 31 | 73,2 … 101,5 | 555, 19 | 81,7 … 102,7 | +8,5 / +1,2 |
| 301 | S(1,6) | 709, 10 | 75,3 … 100,7 | 697, 12 | 85,3 … 103,0 | +10,0 / +2,3 |
| 302 | S(1,7) | 757, 33 | 79,3 … 99,7 | 753, 17 | 85,8 … 100,8 | +6,5 / +1,1 |
| 303 | S(1,8) | 905, 18 | 81,0 … 102,2 | 895, 18 | 86,9 … 103,1 | +5,9 / +0,9 |

- **Werte** (f, Q, min, max): `[am Bild geprüft]`.
- **Spalte Δmin / Δmax:** `[eigene Rechnung]` (mit minus ohne).

**Steigung über die Moden** (Regression der Extremwerte über log₂ f, `d3`) `[eigene Rechnung]`:

| | Minima | Maxima |
|---|---|---|
| ohne Saiten (9 Profile) | +5,5 dB/Okt. | +6,6 dB/Okt. |
| mit Saiten (10 Profile) | +7,0 dB/Okt. | +8,9 dB/Okt. |

**Antwort auf „steigen die besaiteten Werte mit 6 dB/Oktave, die unbesaiteten nicht?“:** **widerlegt**. Die
unbesaiteten Extremwerte steigen über die Moden ebenso stark. Besaitet heben sich vor allem die Minima (+5 bis
+10 dB); die Maxima ändern sich um −7 bis +5 dB.

### 1.4 Prüfung „Reihenfeder im Messweg“ (Hypothese aus Bericht 12, 3.4 d)

Eine Nachgiebigkeit k zwischen Impedanzkopf und Kasten gibt |Y_gemessen| = |Y_Kasten + jω/k|. Ohne Auslöschung durch
massenartiges Y_Kasten liegt jeder Wert mindestens bei etwa ω/k `[hergeleitet, Bericht 12 (M1)]`.

| Mode (besaitet) | f [Hz] | kleinster Wert [dB] | Feder 5,2·10⁴ N/m [dB] | Abstand | k ≥ [N/m] |
|---|---|---|---|---|---|
| S(1,1)− | 187 | 68,2 | 83,1 | −14,9 | 2,9·10⁵ |
| S(1,1)+ | 207 | 76,8 | 84,0 | −7,2 | 1,2·10⁵ |
| S(1,2) | 257 | 72,5 | 85,8 | −13,3 | 2,4·10⁵ |
| S(1,2)′ | 284 | 75,1 | 86,7 | −11,6 | 2,0·10⁵ |
| S(1,3) | 334 | 80,7 | 88,1 | −7,4 | 1,2·10⁵ |
| S(1,4) | 430 | 80,7 | 90,3 | −9,6 | 1,6·10⁵ |
| S(1,5) | 555 | 81,7 | 92,5 | −10,8 | 1,8·10⁵ |
| S(1,6) | 697 | 85,3 | 94,5 | −9,2 | 1,5·10⁵ |
| S(1,7) | 753 | 85,8 | 95,2 | −9,4 | 1,5·10⁵ |
| S(1,8) | 895 | 86,9 | 96,7 | −9,8 | 1,6·10⁵ |

`[eigene Rechnung, d3]`.
- Auch an Ort 17 (Abb. 7.6) liegt die besaitete Kurve über 400 Hz 5–8 dB unter der Linie von 5,2·10⁴ N/m (470 Hz:
  84,0 gegen 91,1 dB; 790 Hz: 90,3 gegen 95,6 dB).
- Mit der weichsten zulässigen Feder (k = 1,2·10⁵ N/m) beträgt ihr Beitrag an den Saiten aus Abb. 7.22:
  104 Hz 70,8 dB (Bell 80), 207 Hz 76,8 (86), 330 Hz 80,9 (95), 494 Hz 84,4 (91), 740 Hz 87,9 (99), 988 Hz 90,4 (99).
  Er liegt also 7–14 dB unter Bells Werten.
- **Ergebnis:** Eine Reihenfeder kann den Pegel von Abb. 7.22 nicht erklären. **Widerlegt**, mit dem Vorbehalt, dass
  an einzelnen Orten Auslöschung möglich ist; an allen 10 Profilen und an Ort 17 zugleich ist das
  `[eigene Einschätzung]` nicht plausibel.

### 1.5 Abb. 7.21 — besaitet, 50 Hz – 10 kHz, Messorte 4 und 7 (PDF 308)

Die Achse ist relativ (0 bis −40 dB ohne Bezug); Ort 7 ist im Bild um 20 dB nach unten verschoben, hier
zurückgerechnet `[am Bild geprüft]`.
- **Frequenzachse:** logarithmisch, die Ticks liegen bei 50/100/200/500/1k/2k/5k/10k Hz auf den Zeilen
  1022/925,5/828/699/602/504/374,5/278 (110 dpi), also 97,3 px je Oktave.
- **Rasterweite:** eine Pixelzeile = 1/97 Oktave; je Wert ±3 Zeilen gemittelt.
- **Genauigkeit** ±1,5 dB `[digitalisiert, d4]`.
- Bei 50 Hz liegen beide Kurven auf einer Gitterlinie (−20 dB) und sind dort nicht vom Gitter zu trennen. Der
  Wert −20 dB ist am Bild abgelesen.

| f [Hz] | 63 | 100 | 160 | 200 | 250 | 315 | 400 | 500 | 800 | 1000 | 1250 | 1600 | 2000 | 5000 | 10000 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Ort 4 [dB rel.] | −19,7 | −16,7 | −13,7 | −11,6 | −9,3 | −7,0 | −5,8 | −7,1 | −5,1 | −1,0 | +2,7 | −1,9 | −4,1 | −7,6 | −10,5 |
| Ort 7 [dB rel.] | −19,2 | −16,4 | −14,0 | −11,8 | −9,4 | −5,2 | −7,7 | −11,8 | −4,7 | −0,9 | −1,0 | −0,3 | −3,4 | −11,2 | −15,2 |

**Steigungen** `[eigene Rechnung, d4]`:

| Ort | 50–200 Hz | 50–300 Hz | 50 Hz bis 1 kHz | Hochfläche (höchstes Terzmittel) | 63 Hz → Hochfläche | 2–10 kHz |
|---|---|---|---|---|---|---|
| 4 | +4,6 | +5,0 | +4,5 | +2,9 dB bei 1240 Hz | +5,3 | −2,7 |
| 7 | +4,1 | +4,7 | +3,7 | +1,0 dB bei 1480 Hz | +4,4 | −5,4 |

Alle Steigungen in dB/Oktave.

**Befund:** Der Anstieg unter der ersten Resonanz ist 4–5 dB/Oktave, nicht 6. Bell nennt den Grund am losen Kasten
selbst: Steife des Kastens unterhalb der ersten Resonanz (S. 133).

### 1.6 Antwort Frage 1: Trägt der Pegel der besaiteten Messungen Bells?

| Teilaussage | Ergebnis | Grundlage |
|---|---|---|
| Bezug der Kap.-7-Messungen | **belegt**: 116 dB = 1 s/kg | S. 149 [am Bild geprüft] |
| Gleiche Messkette besaitet / unbesaitet | **belegt** bis auf Winkel < 10° und Ort „as close to the centre of the cover bar as the strings would allow“; Winkel 30° gegen senkrecht ändert unbesaitet „rarely“ mehr als ±3 dB | S. 147, S. 157 [wörtlich, Bericht 02] |
| Reihenfeder im Messweg erklärt 7.22 | **widerlegt** (k ≥ 1,2·10⁵ N/m; Beitrag 7–14 dB unter 7.22) | 1.4 |
| 6 dB/Oktave nur besaitet | **widerlegt** (unbesaitete Profile 5,5–6,6 dB/Okt.; Bell S. 134) | 1.3 |
| Masse unter dem Kraftaufnehmer kompensiert | **nicht entscheidbar**: nicht genannt; sie würde ohnehin senken, nicht heben | 1.1 |
| Pegel frei von jedem Fehler im Messweg | **nicht entscheidbar**, weil Bell keine Gegenmessung derselben besaiteten Harfe mit anderem Verfahren hat und selbst einen Weg über „strings and damping plastic“ für möglich hält (S. 153) | 1.1 |

**In einem Satz:** Der besaitete Pegel Bells trägt so weit wie sein unbesaiteter — gleicher Bezug, gleiche Kette, und
die einzige benannte Verfälschung (Reihenfeder) ist widerlegt. Was bleibt, ist eine echte Änderung durch das
Besaiten (Täler über 380 Hz +10–17 dB). Ihr Mechanismus ist nicht belegt: Bänder, Saiten, Stelle neben der Mitte.

**Folge für den Widerspruch zu Le Carrou (25–33 dB im Bass)** `[eigene Einschätzung]`: Er ist mit dem Messweg bei
Bell nicht erklärt. Bells **unbesaitete** Maxima liegen bei denselben Moden ebenso hoch (96–105 dB). Der Unterschied
Salvi gegen Camac bleibt eine Frage des Instruments oder des Messwegs bei Le Carrou, nicht der besaiteten Messung bei
Bell.

---

## 2 Güten der tiefen Moden

| Instrument | Zustand | Mode | f [Hz] | Q | Quelle (Seite) | Marke |
|---|---|---|---|---|---|---|
| Salvi „Orchestra“, Kasten G | lose | S(1,1)− | 153 | 23 | Bell Abb. 7.8 (PDF 294) | [am Bild geprüft] |
| | lose | S(1,1)+ | ≈ 200 | nicht messbar („only … three testing positions“) | Abb. 7.9; S. 152 | [am Bild geprüft] |
| | lose | S(1,2) / S(1,2)′ | 221 / 251 | 13 / 36 | Abb. 7.10 / 7.11 | [am Bild geprüft] |
| | lose | S(1,3) | „3?0“ (Ziffer unscharf, 300 oder 310) | 36 | Abb. 7.12 | [am Bild geprüft], f unsicher |
| | lose | S(1,4) … S(1,8) | 420 / 557 / 702 / 768 / 912 | 38 / 32 / 28 / 44 / 26 | PDF 299–303 | [am Bild geprüft] |
| | Harfe, **ohne** Saiten | S(1,1)− / S(1,1)+ | 170 / 216 | 11 / 16 | Abb. 7.8 / 7.9 | [am Bild geprüft] |
| | ohne Saiten | S(1,2) (eine Mode) | 249 | 39 | Abb. 7.10; S. 153 | [am Bild geprüft] |
| | ohne Saiten | S(1,3) … S(1,8) | 316 / 432 / 545 / 709 / 757 / 905 | 45 / 43 / 31 / 10 / 33 / 18 | PDF 298–303 | [am Bild geprüft] |
| | Harfe, **mit** Saiten (Kunststoffbänder, S. 147) | S(1,1)− | 187 | „indeterminate … insuffuicient trough“ | Abb. 7.8; S. 152 | [am Bild geprüft] |
| | mit Saiten | S(1,1)+ | 207 | „indeterminate“ | Abb. 7.9 | [am Bild geprüft] |
| | mit Saiten | S(1,2) / S(1,2)′ | 257 / 284 | 32 / 30 | Abb. 7.10 / 7.11 | [am Bild geprüft] |
| | mit Saiten | S(1,3) … S(1,8) | 334 / 430 / 555 / 697 / 753 / 895 | 27 / 28 / 19 / 12 / 17 / 18 | PDF 298–303 | [am Bild geprüft] |
| | ohne / mit Saiten | B1 (ganzer Körper) | 85 / 92 | – (nicht genannt) | S. 155 | [Textebene] |
| Salvi, Kasten starr (in 120 kg Sand, Öffnungen mit Bleiplatten verschließbar) | eine Öffnung offen | Helmholtz | 75 / 87 / 94 / 88 / 86 | 10 / 9 / 9 / 11 / 10 | S. 76 (PDF 134) | [am Bild geprüft] (Bericht 12, S202) |
| | Öffnungen 3–6 offen | Helmholtz | 123 | 8 (Handschrift, 8 oder 9) | Abb. 4.8 iv (PDF 161) | [am Bild geprüft] |
| | Öffnungen 2–6 offen | Helmholtz | 164 (Handschrift über gedruckter Zahl) | 6 | Abb. 4.8 v (PDF 161) | [am Bild geprüft] |
| | **alle sechs Öffnungen offen** | Helmholtz | **190** | **6** | Abb. 4.8 vi (PDF 161); Abb. 4.9 (PDF 162): „The Helmholtzian Mode on the Soundbox with all Apertures Open (At 190 Hz, Q=6)“, Lautsprecher an Öffnung 2, 5 und 6 | [am Bild geprüft] |
| Camac Atlantide Prestige | besaitet, Saiten mit Papier gedämpft | 1 / 2 / 3 / 4 (T1) / 5 / 6 (A0) | 54,8 / 80,9 / 123,4 / 152,2 / 161,9 / 168,5 | ζ 5,5 / 4,8 / 2,5 / 2,3 / 0,9 / 1,4 % → Q 9,1 / 10,4 / 20 / 21,7 / 55,6 / 35,7 | Le Carrou, Gautier & Foltête 2007, Abb. 4, S. 562 (PDF 4) = Thèse Abb. 3.4 | [am Bild geprüft]; Q [eigene Rechnung] Q = 1/(2ζ) |
| | besaitet | T1 / A0 | 154,1 / 170,2 | 19,1 / 19,5 | Thèse Tab. 4.1, S. 69 (PDF 80): s₄ = −25,3 + j 968,4, s₆ = −27,4 + j 1069,2 („lissage des fonctions de transfert“) | [am Bild geprüft]; f, Q [eigene Rechnung] Q = \|s\|/(2\|Re s\|) |
| | besaitet | T1 / A0 | 155,2 / 170,0 | 15,3 / 16,4 | Thèse Gl. 4.19–4.21 (2 FHG) | aus Bericht 12 (R10), nicht neu gerechnet |
| | besaitet | Moden 1–6 | 61,5 / 84,5 / 124,5 / 153,5 / 172 | **keine Dämpfung** | Thèse Tab. 3.2 (PDF 66); JASA 2007 Tab. II | [am Bild geprüft] / [Textebene] |

**Ohne Güten der Körpermoden:**
- Le Carrou 2007 Tab. I gibt nur die Maße der Schalllöcher `[Textebene]`.
- Le Carrou, Gautier & Badeau 2009 Tab. II (S. 18) gibt Dämpfungsfaktoren der **Komponenten im ersten Teilton** der
  Saiten 30/31 (mitschwingende Saiten), keine Güten der Körpermoden. Dazu „a maximum of 30% of variation for the
  damping factors“ zwischen den Konfigurationen (S. 19) `[Textebene]`.

**Antwort Frage 2:**
- **Belegt:** Güte der Helmholtz-Mode mit allen Öffnungen offen Q 6 (Bell, starrer Kasten); besaitete Güten der
  Plattenmoden S(1,2)–S(1,8) an der Salvi; T1/A0 der besaiteten Camac (Q 22/36 modal, 19/19,5 aus dem
  2-FHG-Abgleich).
- **Nicht gefunden:** eine Güte der Moden um 190–210 Hz an der **besaiteten Salvi**; Bell nennt sie
  „indeterminate“.

**Einordnung** `[eigene Einschätzung]`:
- LumiViz setzt Q_Luft = 5,5 (KQ29 a). Das liegt bei Bells Q 6 für alle Öffnungen offen.
- Bericht 12 hat 5,5 an Q 9–11 gemessen; das sind die Werte **je einzelner Öffnung**, also nicht die passende
  Vergleichsgröße.
- Bells Q 6 gilt im starren Kasten. Die Güte schließt Abstrahlung durch sechs Öffnungen und Verluste an Wand und Sand
  ein. Sie ist keine Güte der gekoppelten Mode in der Harfe.
- Die besaitete Camac zeigt für die gekoppelten Moden Q 19–36, je nach Auswertung. Diese Streuung zwischen zwei
  Auswertungen derselben Messung (Faktor 1,9 bei A0) ist selbst ein Befund.

---

## 3 Offene Quellen der Ladelisten 02 / 12

### 3.1 Forum Acusticum 2023, Beitrag 000712

- **Erste Seite / PDF-Metadaten:** F. Fabre, J.-L. Le Carrou, B. Chomette, S. Le Bomin, „Vibroacoustic Descriptors of
  Central Africa Harps“, Forum Acusticum 2023 (Turin), S. 3429–3432.
  - Autoren und Titel stehen in den PDF-Metadaten (`pdfinfo`) und im Seitenbild (Kopf, „Copyright: ©2023 François
    Fabre et al. This is an open-access article distributed under the terms of the Creative Commons …“).
  - Die Seitenzahl 3430 ist am Bild von S. 2 gelesen.
- **URL:** https://dael.euracoustics.org/confs/fa2023/data/articles/000712.pdf
- **Datei:** `abrufe/FA2023_000712.pdf`, 670 121 Bytes.
- **Inhalt:** Sechs Harfen aus Zentralafrika, je 8 Saiten aus Angelschnur 0,8–1,1 mm.
  - „The static string/body coupling is kept but a strip of felt is intertwined with the strings to dampen their
    transverse vibrations“ (S. 3430) `[am Bild geprüft]`.
  - Gemessen mit automatischem Impulshammer (PCB 086E80) und Beschleunigungsaufnehmern; „collocated conductances (real
    part of the mobility) along string attachment points“ `[am Bild geprüft]`.
- **Für die Konzertharfe:** **nicht übertragbar.** Werte an den Saitenorten sind deshalb nicht abgegriffen.
- **Lesart der Ladeliste:** Die Lesart „Konduktanz längs der Saitenösen bei statischer Saitenlast“ stimmt. Das
  Instrument stand in der Ladeliste nicht, weil sie aus einer Suchmaschinen-Zusammenfassung stammt.
- **Methodisch brauchbar:** Die Gruppe um Le Carrou hält die statische Last und dämpft die Saiten mit Filz, wie bei der
  Camac mit Papier.

### 3.2 Bell & Firth, IOA-Proceedings

- **Erste Seite:** „Proceedings of The Institute of Acoustics — VIBRATIONS OF THE CONCERT HARP SOUNDBOARD AND
  SOUNDBOX — ALEXANDER J BELL and IAN FIRTH — UNIVERSITY OF ST.ANDREWS“, S. 65 `[am Bild geprüft]`. Ein Jahr steht
  nicht auf S. 1.
- **URL:** Die IOA-Adresse gibt weiter HTTP 404 (Fehlerseite abgelegt als `abrufe/Bell_Firth_IOA.pdf`, ist HTML).
  Gelesen ist die Kopie im Webarchiv:
  https://web.archive.org/web/20250429214825id_/https://www.ioa.org.uk/system/files/publications/AJ%20BELL%20I%20FIRTH%20VIBRATIONS%20OF%20THE%20CONCERT%20HARP%20SOUNDBOARD%20AND%20SOUNDBOX.pdf
  → `abrufe/Bell_Firth_IOA_HarpSoundboardSoundbox.pdf` (997 072 Bytes).
- **Wortlaut S. 65** `[am Bild geprüft]`: „the Salvi "Orchestra" – is a plate of length 136 cm with a width of 44 cm at
  the bass end, narrowing to 12 cm at the treble end, and a tapering thickness which varies from 12 mm at the bass to
  2 mm at the treble. The soundboard is of European Spruce, quarter sawn with the grain running across the board. On
  the front of the board there is a thin veneer of spruce with the grain running along the board.“
  - Dieselbe Seite: Die Harmonieleisten „bear little of the tension from the strings“.
  - Die Textebene liest „II cm“ statt „12 cm“ — am Bild steht 12.
- **Besaitet gegen unbesaitet:** **nicht gefunden.** Suche nach „strung“: 0 Treffer, „string“ nur 3 Treffer
  (Aufbau, Knotenlinie, Ausblick „Work is continuing on … harp strings“). Der Beitrag behandelt freien Boden,
  Verjüngung, Breite und Kasten.

**Zusätzlich gefunden: Firth & Bell, SMAC 83.**
- Google Scholar führt „On the acoustics of the concert harp's soundboard and soundbox“, 1985, S. 167–183
  `[nur Zusammenfassung]`.
- Der Band liegt frei bei der KTH: https://www.speech.kth.se/music/smac83/SMAC83_Volume2.pdf →
  `abrufe/SMAC83_Volume2.pdf` (16,7 MB, ganzer Band).
- **Erste Seite des Beitrags** (S. 167, PDF 168): „ON THE ACOUSTICS OF THE CONCERT HARP'S SOUNDBOARD AND SOUNDBOX —
  I.M. Firth and A.J. Bell — University of St. Andrews“ `[Textebene]`.
- **S. 169 (PDF 170), Abb. 2** `[am Bild geprüft]`: „Typical dimensions are length 136 cm, width (at bass) 44 cm,
  thickness (at bass) 12 mm, width (at treble) 12 cm, thickness (at treble) 2 mm“.
- **S. 177:** Verjüngungsversuch „stage 4 = 12 mm tapering to 2 mm“ `[Textebene]`.
- **Besaitet gegen unbesaitet:** „unstrung“ und „with strings“ 0 Treffer im ganzen Band; „strung“ nur in einem
  fremden Beitrag `[Textebene; drei Seiten des Bands ohne Textebene]`.

### 3.3 Daltrop 2009 (UBC)

**Nicht gefunden.** Zitiert als „S. Daltrop, Vibro-acoustic Behaviour of Harp Soundboxes, Honours Thesis, University
of British Columbia, (2009)“ auf https://acoustics.org/pressroom/httpdocs/157th/waltham.html `[nur Zusammenfassung]`.
- **Suchweg:** WebSearch „Daltrop 2009 harp soundbox honours thesis UBC physics pdf“.
- **Treffer:** nur Zitate und die UBC-Publikationsliste. In cIRcle (UBC) nicht gesucht.

### 3.4 Bell 1997, J. Catgut Acoust. Soc.

Inhaltsverzeichnis Mai 1997 (https://catgutacoustical.org/journal/may97.htm): „The Helmholtz Resonance and Higher Air
Modes of the Harp Soundbox“, A. J. Bell, Bd. 3 Nr. 3, ab S. 2 `[Werkzeug]`.
- Kurzfassung nach dem Werkzeug: „The air modes of a rigid soundbox of the Salvi "Orchestra" Concert Harp are
  analysed …“. Keine Güten, kein Volltext.
- Da derselbe starre Kasten wie Bell 1988 Kap. 4 gemeint ist, liefert Abb. 4.8 vi/4.9 die gesuchte Güte schon.

---

## 4 Abstrahlung je Kraft 220 Hz – 2 kHz

**Nicht gefunden.**
- **Lokale Suche:** `pdf_suche.py` nach „Pa/N“, „Pa N“, „radiat“, „anechoic“ in Chadefaux 2012, Chadefaux 2013 und
  Le Carrou/Gautier/Badeau ICA 2007. Ergebnis: keine Abstrahlung je Kraft.
- **Netz:** WebSearch „concert harp radiated sound pressure per unit force Pa/N measurement soundboard 2 kHz“. Die
  Treffer reichen bis 1 kHz (Le Carrou 2010, schon in Bericht 02) oder betreffen andere Instrumente: Salvi Aurora,
  Hammer, Druck in der Öffnung (Waltham, JCAA) `[nur Zusammenfassung]`.
- **Bell Kap. 8:** Ein Bezug je Kraft wurde dort schon in Bericht 02 nicht gefunden; hier nicht erneut gesucht.

---

## Nicht gefunden – Suchwege

| Lücke | Wo gesucht |
|---|---|
| Kompensation der Masse, Steife und Befestigung des Stifts | Bell PDF 228–250 (`pdf_suche`, s. 1.1), PDF 237–245 und 256–258 am Bild |
| Güte von S(1,1)± an der besaiteten Salvi | Bell S. 151–152 am Bild („indeterminate“), Abb. 7.8/7.9 |
| Güte der Ganzkörpermode B1/B2 | Bell S. 155 (nur f) |
| Besaitet gegen unbesaitet bei Bell & Firth | IOA-Beitrag (Suche „strung“, „string“), SMAC 83 (Suche „strung“, „unstrung“, „with strings“) |
| Daltrop 2009 | WebSearch (1 Lauf) |
| Bell 1997 Volltext | WebSearch (1 Lauf), Inhaltsverzeichnis catgutacoustical.org |
| Abstrahlung je Kraft > 1 kHz | s. Abschnitt 4 |

## Liste der Abweichungen (Regel 9)

- **Abb. 7.6, 7.21:** Die Kurven sind mit eigenen Skripten digitalisiert; Bell gibt keine Zahlen. Die Frequenzachse
  von Abb. 7.6 Feld 2 und 3 ist als gleich mit Feld 1 angenommen (nur dort beschriftet).
- **Profile 7.8–7.17:** Nur Minimum und Maximum je Feld sind abgelesen, nicht der ganze Verlauf. Die Prüfung in 1.4
  nimmt das Minimum als Wert ohne Auslöschung.
- **Abb. 7.22 gegen Karten:** Ein „unbesaitetes 7.22“ (Karte Abb. 7.3 an den 24 Saitenorten bei den Saitenfrequenzen)
  ist **nicht** gebildet. Das wäre die direkteste Prüfung, braucht aber das Ablesen der dichten Höhenlinienkarten.

## Ladeliste (nur Gesperrtes oder nicht Gefundenes)

| Vorschlag Dateiname | URL | Größe | Vermerk | wozu |
|---|---|---|---|---|
| `Bell_1997_HelmholtzHigherAirModesHarpSoundbox_JCAS3-3.pdf` | catgutacoustical.org (nur Inhaltsverzeichnis) | ? | nicht frei; CAS-Archiv bzw. Bibliothek | Güten der Luftmoden, falls über Kap. 4 hinaus |
| `Daltrop_2009_VibroacousticHarpSoundboxes_UBC.pdf` | – (cIRcle, UBC, nicht durchsucht) | ? | nicht gefunden | Kasten ohne/mit Saiten? (unbekannt) |

## Nicht geliefert / von der LumiViz-Sitzung nachzuprüfen

- **Nicht geliefert:**
  - Abstrahlung je Kraft 220 Hz – 2 kHz (Teil 4);
  - ein „unbesaitetes 7.22“ aus der Karte Abb. 7.3;
  - die Güte der gekoppelten Moden um 190–210 Hz an der besaiteten Salvi (Bell: unbestimmbar).
- **Nachzuprüfen:**
  - **Fig.-Nummern von PDF 300–303:** am 80-dpi-Bild unscharf (7.14–7.17 angenommen); die Moden sind sicher.
  - **Abb. 4.8 iv / v:** Q 8 oder 9 bei 123 Hz und die Frequenz 164 Hz stehen handschriftlich und unscharf da.
  - **Abb. 7.6:** dieselbe Frequenzachse in allen drei Feldern (nur Feld 1 beschriftet).
  - **KQ29 a:** Q_Luft 5,5 gegen Bells Q 6 (alle Öffnungen, starrer Kasten) statt gegen Q 9–11 (je Öffnung) — die
    Vergleichsgröße in Bericht 12 war die falsche. Ob eine Güte am starren Kasten als Luftgüte im gekoppelten Modell
    stehen darf, entscheidet die Sitzung.
  - **Bezug von Abb. 7.22:** 116 dB = 1 s/kg nach S. 149 — damit gilt die S201-Prüfung („Saite 26 bei 207 Hz 87,3
    gegen 86,1 dB“) wieder mit belegtem Bezug.
  - **Kopf S202 zu Bericht 12:** „Ziel Z1 ohne Bell 7.22“ beruhte auf der Messweg-Hypothese, die hier widerlegt ist.

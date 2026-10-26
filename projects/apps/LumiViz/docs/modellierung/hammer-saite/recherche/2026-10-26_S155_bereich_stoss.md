> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: ungleicher Stoss je Saite (`stossLeise`, `stossLaut`, u). Suche: im Netz.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Recherche: ungleicher Stoß je Saite (`stossLeise`, `stossLaut`, Parameter u)

Stand 2026-10-26 · Recherche-Agent · Projekt LumiViz, Hammer-Saite

## Auftrag

Belegte Wertebereiche aus Quellen zum Konzertflügel dafür finden, **wie ungleich** der Hammer die
2–3 Saiten eines Chors anregt (Kraft, Zeitversatz, Amplitude, Rillen, Schiefstand,
Anschlagstärke, Intonation, Una corda). Modell: drei Saiten 1+u : 1 : 1−u, zwei Saiten
1+u : 1−u, u ∈ [0, 1], getrennt für leise und laut.

## Reichweite

- Rund 40 Abrufe (WebSearch und WebFetch, ein Abruf über den eingebauten Browser, HAL-API per curl).
- **Gelesen im Volltext:** Weinreich, *Five lectures* (KTH-Webseiten `strings.html`, `mistuned.html`);
  Askenfelt & Jansson, *Five lectures* (`voicing.html`, `stringvib.html`); Conklin, *Five lectures*
  (`thehammers.html`); Weinreich, Scan „The Coupled Motions of Piano Strings“ (OCR-Textebene);
  Capleton 2004 (JASA); **Thomas, Rousseau, Caussé, Marandas 1998 (ISMA, HAL)**; Aramaki et al.
  DAFx 1999; Woodhouse 2022 (Cambridge-Repositorium); Woodhouse, euphonics.org §7.3 und §12.2;
  Sato & Samejima 2023 (AST); Miranda Valiente et al., Forum Acusticum 2026 (Aufprallgeräusch);
  Edinburgh-Tutorial; Patent US 7122728 (Google Patents).
- **Nicht erreicht:** Weinreich 1977 JASA (Bezahlschranke), Askenfelt & Jansson I–III JASA
  (Bezahlschranke, kein freies STL-QPSR-Exemplar gefunden), Conklin 1996 I–III JASA, Fletcher &
  Rossing Kap. 12, Giordano (Buch; ein Sammelband-PDF war über 10 MB, nicht gelesen), Caussé &
  Félix 2000 (nur Abstract), Caussé, Marandas, Thomas FA 1999 (HAL-Eintrag ohne Datei),
  Marandas, Caussé, Gibiat ISMA 1995 „Tuning of trichords in the piano“ (nicht gefunden),
  Southampton-Eprint 504922 (403).
- Die lokalen PDFs unter `quellen/` (Bank, Chabassier, MirandaValiente_2024 u. a.) habe ich
  auftragsgemäß **nicht** gelesen.
- **Kein Treffer ist kein Beweis.** Gesucht wurde gezielt nach Messungen von Zeitversatz,
  Kraft- oder Amplitudenunterschied je Saite eines Chors; **eine solche Messung habe ich nicht
  gefunden.**

## Quellen

### Q1 Thomas, Rousseau, Caussé, Marandas 1998 — die einzige Quelle mit Zahlen für die Ungleichheit

- **Angabe:** Olivier Thomas, David Rousseau, René Caussé, Eric Marandas, „Comparison of the
  effect of hammer striking irregularities and mistuning on the double decay of piano tones“,
  ISMA, Jun 1998, Leavenworth, pp. 253–258 (aus dem Deckblatt von HAL, wörtlich).
- **Zugang:** HAL hal-01105499, Datei `https://hal.science/hal-01105499/file/OTDR.pdf`
  (489 132 Byte), Lizenz „HAL Authorization“ (hal-authorisation-v1). HAL steht hinter einer
  Bot-Prüfung (Anubis); curl bekam nur die Prüfseite. Gelesen im eingebauten Browser
  (Textebene mit pdf.js). **Volltext gesehen.** Nicht in `quellen/` abgelegt — siehe
  „Zwischenkopien“.
- **Was es ist:** eine **Simulation**, keine Messung der Ungleichheit. Zwei Saiten (Doublet),
  nur Grundmode, Es4 (ω₀ = 1954 rad/s), Steg-Impedanz nach Wogram 1981.
- **Wie die Ungleichheit eingeht** (PDF-S. 4, gedr. S. 255 [Vermutung aus der Zählung 253–258]):
  „The easiest way to introduce the hammer striking irregularities in the model is to give
  different initial conditions to the two strings.“ — „it is enough to fix y01 and adjust the
  value of y02 ∈ [0, y01]“ (Anfangsauslenkungen, Anfangsgeschwindigkeit null).
- **Zahlen:**
  - PDF-S. 5: „with a small striking irregularity (y01 =1mm, y02 =0,8mm)“.
  - PDF-S. 7 (gedr. 258): „large striking irregularities (y01 = 1 mm, y02 = 0,4 mm)“.
  - PDF-S. 6, Abb. 5: Kurven für mehrere y02, darunter „y02 = 0“ mit der Beschriftung „una corda“
    (OCR der Abbildung, Werte dazwischen nicht sicher lesbar).
  - PDF-S. 7: „the una-corda pedal leads to a 10 dB aftersound gain. We could compare its effect
    to an extreme striking irregularity where is excited the antisymmetric mode.“
  - PDF-S. 7: „The aftersound level is mainly linked with the striking irregularities.“
  - PDF-S. 5: Marandas „mesured 0,5 cent as an average value for the mistuning“ (Verstimmung,
    nicht Stoß).
- **Grenze:** „small“ und „large“ sind **gesetzte** Werte der Autoren, nicht gemessen. Die Arbeit
  nennt selbst als offen: „the existence of a third string, with hammer striking irregulatities
  modelled with realistic hammer deformation laws“ (PDF-S. 7).

### Q2 Weinreich, „The Coupled Motions of Piano Strings“ (Scan)

- **Angabe:** Titel und Autor aus dem Scan; Zeitschrift auf der Scan-Textebene nicht lesbar —
  **Vermutung:** Scientific American 240 (1), Januar 1979; gedruckte Seiten 94–99 (aus der Scan-Zählung).
- **Zugang:** `http://www.nat.vu.nl/~henkb/Betamusica/Weinreich.pdf` (Kursseite der VU Amsterdam,
  9 S., 4,9 MB, gescannt mit OCR-Ebene). Urheberrecht beim Verlag; **nicht** als Beleg abgelegt,
  weil es weder Verlags- noch Autorenseite ist. **Volltext (OCR) gesehen.**
- **Zitat** (PDF-S. 6, gedr. S. 99, OCR bereinigt): „the motions of the strings in one unison group
  will initially be almost perfectly symmetric, since the hammer apparently strikes the strings
  with the same strength at the same time. Minor imperfections in the hammer, however, will
  result in string amplitudes that are not absolutely equal.“
- Ebenda: Nachklang entsteht aus „structural irregularities“ und „probably varies erratically from
  note to note“; der Stimmer gleiche über die Verstimmung aus.
- **Zahlen zur Ungleichheit: keine.**

### Q3 Weinreich, *Five lectures on the Acoustics of the Piano* (KTH, 1990)

- **Zugang:** `https://www.speech.kth.se/music/5_lectures/weinreic/strings.html`, `mistuned.html`;
  frei lesbar, © 1990 Royal Swedish Academy of Music. **Volltext gesehen** (über WebFetch,
  das nur Bruchstücke bis 125 Zeichen zitiert).
- **Zitate:** „namely the randomness of hammer imperfections“; „than if it were due entirely to
  imperfections in the hammer or the string mountings“ (`mistuned.html`). In `strings.html` führt
  er die Asymmetrie auf Hammergeometrie und unterschiedliche Auflage am Steg zurück, hält aber die
  Verstimmung für den eigentlichen Grund (nur sinngemäß, das Werkzeug gab den Satz nicht wörtlich).
- **Zahlen zur Ungleichheit: keine.**

### Q4 Capleton 2004

- **Angabe:** Brian Capleton, „False beats in coupled piano string unisons“, J. Acoust. Soc. Am.
  (DOI 10.1121/1.1636461), Pages 885–892 (aus dem PDF).
- **Zugang:** Kopie auf `persianney.com` (keine Verlags- oder Autorenseite; Original hinter
  Bezahlschranke) — gelesen, **nicht** abgelegt. **Volltext gesehen.**
- **Zitate** (PDF-S. 4, gedr. S. 888): „an antisymmetric mode will not be present at zero mistuning
  unless there is a difference in the initial conditions of the two component oscillators“;
  „imperfection in the hammer strike, including strike misalignment, especially when there are
  grooves worn in the hammer face, could also contribute to antisymmetric mode excitation.“
- **Zahlen: keine.** Das Modell selbst nimmt gleichen Stoß an.

### Q5 Conklin, *Five lectures* — Hammer

- **Zugang:** `https://www.speech.kth.se/music/5_lectures/conklin/thehammers.html`, frei lesbar.
  **Volltext gesehen.**
- **Zitat:** „to have the hammer strike all of the strings of a note equally“ (als Bauziel).
- **Zahlen zur Ungleichheit: keine.**

### Q6 Askenfelt & Jansson, *Five lectures* — „From touch to string vibration“

- **Zugang:** `…/5_lectures/askenflt/voicing.html`, `stringvib.html`, frei lesbar. **Volltext gesehen.**
- **Zitat** (`voicing.html`): „the strings introduce grooves in the hammer surface.“
- Zu Ungleichheit, Zeitversatz oder mehreren Saiten eines Chors: **nichts** auf diesen beiden Seiten.

### Q7 Woodhouse, euphonics.org §7.3 „Multiple strings and double decays“

- **Zugang:** `https://euphonics.org/7-3-multiple-strings-and-double-decays/`, frei lesbar.
  **Volltext gesehen** (WebFetch).
- **Zitate:** die zwei Grenzfälle „the two strings are initially excited identically, or just one
  string is driven“; Una corda: „the hammer is shifted sideways so that it only strikes one or two
  strings.“
- Bedeutung für das Modell: die Grenzfälle entsprechen bei zwei Saiten u = 0 und u = 1.

### Q8 Caussé & Félix 2000 (nur Abstract)

- **Angabe:** René Caussé, Simon Félix, „Study on double decay of individual partials of piano
  sound—Preliminary results on the ‘unison quality’“, J. Acoust. Soc. Am. 108, 2592 (2000),
  Tagungsabstract. **Nur Abstract gesehen.**
- **Zitat:** „the adjustment of the regularity of the action of the hammer in the case of important
  irregularities in hammer striking, are parameters used by the tuner“.
- **Zahlen: keine.**

### Q9 Russo, Patent US 7122728 B1 (2006), „String leveling device and method of use thereof“

- **Zugang:** `https://patents.google.com/patent/US7122728`, frei. **Volltext gesehen** (WebFetch).
- **Zitate:** „a hammer strike on an uneven string set produces tones with differing decays“;
  für Una corda: „an uneven tonal quality that is readily heard because of the pianissimo mode.“
- **Zahlen zur Ungleichheit oder Toleranz der Saitenhöhe: keine.**

### Q10 Edinburgh-Tutorial (Lehrmaterial)

- **Angabe:** „MUSICAL ACOUSTICS Tutorial 4: Solution Guide“, Universität Edinburgh (Autor nicht
  angegeben; Ordner `~dmc` — Vermutung: D. M. Campbell).
- **Zugang:** `https://www2.ph.ed.ac.uk/~dmc/PHY-1-MusAc/Tutorials/MusAc-tut4nsol.pdf`, 17 KB.
  **Volltext gesehen.**
- **Zitat:** „in practice the strings are not hit at exactly the same time, or with exactly the
  same force.“ — keine Zahlen; Lehrtext, kein Messbeleg.

### Ohne Ertrag (geprüft)

- Aramaki, Bensa, Daudet, Guillemain, Kronland-Martinet, DAFx 1999 (dafx.de, frei): regen **nur
  eine Saite** an, die zweite schwingt mit — keine Aussage zur Ungleichheit.
- Woodhouse 2022 „A necessary condition for double-decay envelopes…“ (Cambridge-Repositorium,
  frei): keine Aussage zur Anregung je Saite.
- Sato & Samejima 2023, Acoust. Sci. & Tech. 44(3) (J-STAGE, frei): ein Saitenmodell mit
  Hammerstiel; „unison“, „simultan“: 0 Treffer.
- Miranda Valiente et al., Forum Acusticum 2026, Beitrag 000032 (Aufprallgeräusch): nicht zum Thema.
- Technikerforen und Herstellerseiten: nur Ziel „alle Saiten zugleich treffen“, keine Zahlen
  (nicht als Beleg gewertet).

## Umrechnung auf u (Vorschlag, kein Beleg)

Zwei Saiten, Modell 1+u : 1−u. Bei einem Amplitudenverhältnis r = A₂/A₁ ist
**u = (1 − r) / (1 + r)**.
Annahmen: (a) die Anfangsauslenkung bei Thomas et al. entspricht dem Anteil am Stoß; (b) linear,
nur Grundmode; (c) Es4, ein Doublet. Für den Chor aus drei Saiten gibt es keine Quelle.
Una corda auf drei Saiten (zwei getroffen, 1 : 1 : 0) lässt sich mit 1+u : 1 : 1−u **nicht** darstellen.

## Tabelle „Bereich“

| Register / Stärke | gemessene Größe | Wert | u (Vorschlag) | Quelle | Zitat (kurz) | Prüfstand |
|---|---|---|---|---|---|---|
| Es4 (MIDI 63), Doublet, Stärke nicht genannt | Anfangsauslenkung y02/y01 (gesetzt) | 0,8 | ≈ 0,11 | Q1, PDF-S. 5 | „a small striking irregularity (y01 =1mm, y02 =0,8mm)“ | Simulation, 2 Saiten, Grundmode; **nicht gemessen** |
| Es4, Doublet | y02/y01 (gesetzt) | 0,4 | ≈ 0,43 | Q1, PDF-S. 7 | „large striking irregularities (… y02 = 0,4 mm)“ | Simulation wie oben |
| Es4, Doublet, Una corda | y02/y01 | 0 | 1 | Q1, PDF-S. 6 (Abb. 5), S. 7 | „una corda“ / „an extreme striking irregularity“ | Simulation; Nachklang +10 dB |
| Doublet, Grenzfälle | Anregung je Saite | gleich / nur eine | 0 / 1 | Q7 | „excited identically, or just one string is driven“ | Lehrbuchmodell |
| alle Register, Bauziel | Kraft je Saite | gleich | 0 (Ziel) | Q5 | „strike all of the strings of a note equally“ | Konstrukteur, ohne Messung |
| allgemein | Amplitude je Saite | „not absolutely equal“, „almost perfectly symmetric“ | klein, nicht beziffert | Q2 S. 99 | „Minor imperfections in the hammer“ | Aufsatz, ohne Messung |
| allgemein, abgenutzter Hammer | Rillen, Schiefstand | qualitativ | — | Q4 S. 888 | „grooves worn in the hammer face“ | theoretisch |
| leise vs. laut | — | **keine Quelle gefunden** | — | — | — | — |
| Zeitversatz zwischen Saiten (µs/ms) | — | **keine Quelle gefunden** | — | — | — | — |

## Offene Punkte

1. **Keine Messung** von Zeitversatz, Kraft oder Amplitude je Saite eines Chors gefunden.
   Am ehesten noch: Askenfelt & Jansson I–III (JASA 1990–1993, Bezahlschranke), Marandas, Caussé
   & Gibiat ISMA 1995 „Tuning of trichords in the piano“, die DEA-Arbeit Rousseau & Thomas 1997
   (IRCAM), Caussé, Marandas, Thomas FA 1999 (HAL hal-01105528, ohne Datei).
2. **Abhängigkeit von der Anschlagstärke** (stossLeise vs. stossLaut): in keiner gelesenen Quelle.
3. **Register:** der einzige Wert gilt für Es4; nichts zu Bass oder Diskant.
4. **Drei Saiten:** Thomas et al. nur Doublet; das Modell 1+u : 1 : 1−u bildet Una corda auf dem
   Trichord (1 : 1 : 0) nicht ab.
5. Die lokalen PDFs (Bank 2000 S. 29, MirandaValiente_2024 zu Una corda) liest ein anderer Agent —
   deren Zahlen gehören in diese Tabelle.

## Zwischenkopien und Ablage

**In `quellen/` liegt nichts Neues.** Thomas et al. 1998 wäre nach der Freigabe ablagefähig
(HAL, Archiv), aber HAL lässt curl nicht durch die Bot-Prüfung; im Browser lag die Datei nur im
Seitenspeicher. Beim Aufruf von `…/file/OTDR.pdf` im eingebauten Browser **erschien Patrik ein
Speichern-Dialog** — falls gewünscht, dort speichern als
`Thomas_Rousseau_Causse_Marandas_1998_HammerIrregularitiesMistuningDoubleDecay_ISMA_hal-01105499.pdf`.
Ein erster curl-Versuch hatte unter diesem Namen in `quellen/` eine **HTML-Prüfseite (12,5 KB)**
abgelegt; ich habe sie sofort wieder entfernt.

Zwischenkopien (WebFetch, automatisch):
- `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\tool-results\webfetch-1793045302671-3ertzb.pdf` (Aramaki DAFx99)
- `…\tool-results\webfetch-1793045376707-3giofa.pdf` (Capleton 2004)
- `…\tool-results\webfetch-1793045393958-gxbizf.pdf` (Weinreich-Scan)
- `…\tool-results\webfetch-1793045495642-7pt7tz.pdf` (Patent-Scan USPTO)
- `…\tool-results\webfetch-1793045533380-rtlyaq.pdf` (Sato & Samejima 2023)
- `…\tool-results\webfetch-1793045567265-o1sywr.pdf` (Edinburgh-Tutorial)

Im Scratchpad (curl):
- `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\recherche\dl\woodhouse.pdf` (1,1 MB)
- `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\recherche\dl\fa2026_032.pdf` (11 MB)
- `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\recherche\dl\OTDR.pdf` und `hal.html` (je HTML-Prüfseite von HAL, kein PDF)

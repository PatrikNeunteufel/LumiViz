> **Typ:** Agentenbericht, unverändert (Recherche-Agent, S155, 2026-10-26) — nur dieser Kopf ist dazugesetzt.
> **Anlass:** SQ1 a (`docs/audio/Spielweisen_Konzept.md` §4.1, Teilplan 7.5): Grenzen beim Setzen aus Quellen zum
> Flügel statt „Faktor 3 um *Salamander*“. Größe: Verstimmung im Chor (`verstimmungCent`). Suche: im Netz.
> **Prüfstand:** Nichts davon ist von Claude im Volltext nachgelesen. „Volltext gesehen“ heißt: der Agent hat die
> abgerufene Datei gelesen. Zitierfähig ist erst, was aus der Quelle selbst unter `quellen/` nachgelesen ist.
> **Eingeflossen:** in die Zusammenstellung der Bereiche je Größe (S155, Konzept §4.1 nach SQ1 a).

# Recherche: Verstimmung im Chor (`verstimmungCent`, „Detune“)

Stand 2026-10-26, Recherche-Agent für LumiViz (Hammer-Saite). Kein Eintrag ins Repo.

## Auftrag

Belegte Wertebereiche für die Unisono-Verstimmung (Abstand der äußeren Saiten eines Chors, in Cent,
Modellbereich 0 … 20) je Register (MIDI 21–108) an gestimmten Flügeln: gemessen, angestrebt,
Haltbarkeit, Wahrnehmungsschwellen, Weinreichs Argument zum Nachklang.

## Reichweite

- Rund 38 Abrufe (WebSearch und WebFetch zusammen).
- Volltext gesehen: Weinreich, KTH-Vorlesung (HTML); Plath, ISMA 2019 (PDF); Cerisano, PTJ März 2016
  (PDF, Textebene); Klouche, DAGA 2014 (PDF); Campbell, Physics Today 2014 (HTML); Bucknell-Aufgabenblatt
  (PDF, nur als Sekundärzitat von Rossing).
- Nur Abstract: Kirk 1959 (JASA 31, 1644), Weinreich 1977 (JASA 62, 1474).
- **Nicht erreicht:** Volltexte Kirk 1959 und Weinreich 1977 (Bezahlschranke AIP); Fletcher & Rossing Kap. 12
  (Buch); Capleton „Piano Unison Tuning“ / „The Physics of Piano Unisons“ (Buch, nur Händlerlisting gesehen);
  Lattard 1993 (nicht abgerufen); HAL-Papier „Comparison of the effect of hammer striking irregularities and
  mistuning on the double decay of piano tones“ (hal-01105499, Bot-Sperre „Anubis“, Access Denied);
  Baxandall/Brown/Gough, IOA (404); Acta Acustica 2021 aacus200079 (403, Thema unbekannt);
  Aramaki/Bensa et al. (DAFx 1999, JNMR 2001; nicht abgerufen).
- **Nicht gefunden:** eine Messung der Unisono-Verstimmung **je Register** an gestimmten Konzertflügeln.
  Kein Treffer ist kein Beweis — Kirks Volltext (Tabellen) und Capletons Buch sind die wahrscheinlichsten Orte.
- Lokale PDFs unter `docs/modellierung/hammer-saite/quellen/` wurden auftragsgemäß nicht gelesen
  (Plath 2019 liegt dort evtl. schon — ob als dieser ISMA-Beitrag oder als Dissertation, nicht geprüft).

## Quellen

### 1. Kirk, R. E. (1959): „Tuning Preferences for Piano Unison Groups“, JASA 31(12), 1644–1648
- Zugang: AIP/JASA, Bezahlschranke; **nur Abstract gesehen**
  (https://pubs.aip.org/jasa/article/31/12/1644/600354/Tuning-Preferences-for-Piano-Unison-Groups,
  DOI 10.1121/1.1907673). Autor laut Seite: Roger E. Kirk, The Baldwin Piano Company, Cincinnati.
- Aufbau (laut Abstract): Konzertflügel, fünf Zustände — „zero-beat“ und Dreierchöre, obere Saite hoch,
  untere tief gegen die mittlere; Aufnahmen, Paarvergleich mit geschulten und ungeschulten Hörern.
- Zitate (Abstract, Wortlaut über das Abrufwerkzeug als Fragment geliefert):
  - „tuned flat by 12, 1, 2, and 3 cents relative to the center string“ — „12“ ist im HTML vermutlich ein
    verlorenes „½“ (Vermutung).
  - „are 1 and 2 cents maximum deviation among strings“ (bevorzugt).
  - „the way artist tuners actually tune piano unison strings“ (die Vorlieben entsprechen dem).
- Laut Abstract (Werkzeug-Zusammenfassung, nicht Wortlaut): Geschulte Hörer bevorzugen weniger Abweichung als
  ungeschulte.
- **Deutung offen:** „maximum deviation among strings“ = Spanne außen–außen (dann bevorzugt 1–2 Cent) oder
  Abweichung gegen die Mitte (dann außen–außen 2–4 Cent bei symmetrischem ±x). Ohne Volltext nicht entscheidbar.
  Die geprüften Stufen ±½ … ±3 Cent ergeben außen–außen 1 … 6 Cent. Welche Taste(n) geprüft wurden: unbekannt.

### 2. Weinreich, G. (1977): „Coupled piano strings“, JASA 62(6), 1474–1484
- Zugang: AIP, Bezahlschranke; **nur Abstract gesehen** (https://pubs.aip.org/asa/jasa/article/62/6/1474/770474/Coupled-piano-strings, DOI 10.1121/1.381677).
- Zitate (Abstract):
  - „The way in which an excellent tuner can use fine tuning of the unisons to make the aftersound more uniform is discussed.“
  - „… depends on bridge admittance, hammr irregularities, and the exact state in which the piano is tuned.“ (sic)
- Keine Zahlen im Abstract.

### 3. Weinreich, G.: „The coupled motion of piano strings“, in: Five lectures on the Acoustics of the piano (KTH, © 1990 Royal Swedish Academy of Music)
- Zugang: frei lesbar, HTML; **Volltext gesehen**
  (https://www.speech.kth.se/music/5_lectures/weinreic/mistuned.html, Abschnitt „'Mistuned' strings“, Fig. 8).
- Fig. 8 ist **Modellrechnung**, keine Messung: „Fig. 8 shows some theoretical curves“, Bildunterschrift
  „Calculated vertical force on the soundboard“; Parameter „the middle range of a piano keyboard“, Stegimpedanz so
  gewählt, dass die Frequenzen einrasten („which is not always the case in practice“). Keine Taste genannt.
- Zitate:
  - „there are no beats unless the "mistuning" is more than about 0.3 Hz“
  - „for smaller "mistunings" there is just a single "beat null,"“ / „followed by a beatless aftersound whose level depends on the "mistuning."“
  - „Above about 0.3 Hz beats do appear, as exemplified by the curve drawn for a "mistuning" of 0.64 Hz“
  - Kurven außerdem für Δf = 0,22 Hz und 0,06 Hz („cf. the curves for f = 0.22 Hz and 0.06 Hz“, Δ im Text verloren).
  - Zu Kirk: „a carefully and competently tuned piano had the strings of the trichords tuned slightly differently“, „by an amount that appeared to vary randomly from note to note.“ — **keine Zahl** auf der Seite.
  - „an excellent tuner can, under some circumstances, use very fine tuning control“
- Umrechnung (Δf zwischen zwei Saiten, c = 1200·log2(1 + Δf/f)):

  | Δf | bei A3 (220 Hz) | bei C4 (261,63 Hz) | bei A4 (440 Hz) |
  |---|---|---|---|
  | 0,06 Hz | 0,47 ct | 0,40 ct | 0,24 ct |
  | 0,22 Hz | 1,73 ct | 1,46 ct | 0,87 ct |
  | 0,30 Hz (Schwelle) | 2,36 ct | 1,98 ct | 1,18 ct |
  | 0,64 Hz | 5,03 ct | 4,23 ct | 2,52 ct |

  Die Schwelle 0,3 Hz gilt nur für die gewählte Kopplung; sie ist eine Grenze der Mode-Kopplung, keine Hörschwelle.

### 4. Campbell, D. M. (2014): „Evaluating musical instruments“, Physics Today 67(4)
- Zugang: frei lesbar (physicstoday.aip.org/features/evaluating-musical-instruments); **Volltext gesehen** (über Abrufwerkzeug, als Fragmente).
- Zitate: „precise measurements of grand-piano strings after expert tuning“ / „reveal that typically there are pitch differences of about 1.5 cents“ / „(1.5% of a musical semitone) between the strings.“
- **Keine Quelle** für die 1,5 Cent im Text angegeben; Register, Zahl der Flügel, Messart unbekannt. Für Mode-Locking zitiert er Weinreich 1977.

### 5. Plath, N. (2019): „Influence of playing on the tonal characteristics of a concert piano – an observational study“, ISMA 2019 (Detmold), S. 450–461
- Zugang: frei (https://pub.dega-akustik.de/ISMA2019/data/articles/000005.pdf); **Volltext gesehen** (pdftotext).
- Zitat (Abschn. 2.5, PDF S. 4, gedruckt S. 453): „Technicians usually detune the unison strings by 1-2 cents to give the decay of the tone a complex varying structure [14].“ — [14] = Kirk 1959. Also **Sekundärzitat**, keine eigene Messung der Unisono-Spreizung.
- Nebenbefund (Abschn. 5.2, gedruckt S. 457 vermutet): Grundfrequenz-Unterschiede zwischen zwei Stimmungen desselben Flügels im Abstand eines Jahres (gleicher Stimmer): Bass „without measurable differences“, „In the mid range deviations of approx 2.5 cent are observable. In the highest octave deviations of up to 10 cent are observable. These notes are also the hardest to tune properly.“ — betrifft die **Stimmung zwischen Terminen**, nicht die Chorspreizung; als Anhalt dafür, dass im Diskant die Stimmgenauigkeit grob 4× schlechter ist als in der Mitte.

### 6. Cerisano, M. (2016): „Unison Drift“, Piano Technicians Journal 59(3), März 2016, S. 10–13
- Zugang: frei als PDF auf der Seite des Autors (https://howtotunepianos.com/wp-content/uploads/2020/02/PTJ-Unison-Drift-For-Public.pdf); Textebene vorhanden; **Volltext gesehen** (pdftotext). **Praxisangabe, nicht wissenschaftlich** (Fachzeitschrift der Stimmer, Messung mit Stroboskop-App, wenige Klaviere).
- Zitate: „the final trichord measured a full 1.5 cents lower“; „The trichord's measured pitch varied by 4.4 cents and averaged 2.2 cents lower than the initial string pitches.“; „the trichord varied 3.0 cents!“; „The final trichord's pitch may vary up to 3.0 cents“; aus Weinreich 1977 referiert: „two cents sharp or flat“.
- Betrifft die **Tonhöhe des gekoppelten Chors** gegen die Einzelsaite (Weinreich-Drift), nicht direkt die Spreizung. Für das Modell: Kopplung kann den Chor um 1–3 Cent verschieben.

### 7. Klouche, T. (2014): „How Microtonal is a Well-Tuned Concert Grand Piano?“, DAGA 2014 Oldenburg
- Zugang: frei (https://pub.dega-akustik.de/DAGA_2014/data/articles/000164.pdf); **Volltext gesehen**.
- Steinway D, 16 Stimmungen „directly after the tuning process preparing public concerts“, F0 per Praat-Autokorrelation.
- „Intra-note ranges“ (Fig. 4, „fluctuations around 5 cents“) = Schwankung der gemessenen F0 **innerhalb eines Tons** über 6 s — vermischt Schwebung, Messrauschen, Teiltonverwechslung; **nicht** als Chorspreizung verwendbar. Zitat: „Intra-note microtonality is statistically present with the amount of roughly 28 cents over 5 octaves“.
- Nur als Hinweis: Auch eine Konzertstimmung zeigt in der Messung mehrere Cent Unruhe je Ton.

### 8. Rossing/Moore/Wheeler, The Science of Sound, 3. Aufl., Abschn. 14.3 — nur Sekundärzitat
- Gesehen nur im Aufgabenblatt Bucknell UNIV 213 (https://www.eg.bucknell.edu/~dkelley/univ213/assignments/univ213hw7_sp26.pdf): „As explained in Sec. 14.3 of the textbook (Rossing, Moore, and Wheeler, 3rd ed.), the strings on a piano that are assigned to a single note … are purposely mistuned by a few cents.“ Lehrbuchtext selbst nicht gesehen.

### 9. Praxis/Foren (nicht wissenschaftlich)
- Modartt-Forum (Pianoteq), Thread 916, Nutzer „jgarnao“, 19.11.2009, kein Stimmer („This is what I understand from my readings“): „about 2 cents detuning which causes the note to 'evolve'“ und „This 2 cents difference is not acceptable to most concert tuners.“
- Suchmaschinen-Zusammenfassungen nannten außerdem: RPT-Prüfung „1-cent limit for full marks“, Hörgrenze „half a cent“ (Forum), lose Stimmwirbel „more than 10-15 cents“ (Forum), Drift „1.5 cents within hours to a day“ (Forum). **Keine davon im Wortlaut gesehen — kein Beleg.**

## Umrechnung Spreizung → Schwebung (Δf = f·(2^(c/1200) − 1))

| Taste | 0,5 ct | 1 ct | 2 ct | 4 ct | 6 ct |
|---|---|---|---|---|---|
| A0 (27,5 Hz) | 0,008 Hz | 0,016 | 0,032 | 0,064 | 0,095 |
| A2 (110 Hz) | 0,032 | 0,064 | 0,127 | 0,254 | 0,382 |
| C4 (261,63 Hz) | 0,076 | 0,151 | 0,302 | 0,605 | 0,908 |
| A4 (440 Hz) | 0,127 | 0,254 | 0,509 | 1,018 | 1,528 |
| A6 (1760 Hz) | 0,508 | 1,017 | 2,034 | 4,071 | 6,110 |
| C8 (4186 Hz) | 1,209 | 2,419 | 4,839 | 9,683 | 14,533 |

Folge: Gleiche Cent heißen im Bass kaum hörbare, im Diskant schnelle Schwebungen. Stimmer hören auf Schwebung
(Hz), daher ist im Bass eine größere Cent-Spreizung, im Diskant eine kleinere zu erwarten — **Vermutung,
nicht belegt**. Gegenläufig: Plath (Quelle 5) misst im Diskant die schlechteste Stimmgenauigkeit.

## Tabelle „Bereich je Register“

| Register / MIDI | min | max | typisch | Quelle | Zitat-Kurzform | Prüfstand |
|---|---|---|---|---|---|---|
| unbestimmt (Konzertflügel, Tasten unbekannt) | 0 („zero-beat“) | 6 (±3 geprüft, außen–außen) | 1–2 bevorzugt (oder 2–4, Deutung offen) | Kirk 1959 | „1 and 2 cents maximum deviation among strings“ | Abstract |
| unbestimmt (Flügel nach Expertenstimmung) | – | – | ≈ 1,5 | Campbell 2014 | „about 1.5 cents … between the strings“ | Volltext, ohne Primärquelle |
| unbestimmt | 1 | 2 | – | Plath 2019 (nach Kirk) | „detune the unison strings by 1-2 cents“ | Volltext, Sekundärzitat |
| Mitte (≈ C4–A4, Modell) | 0 | Einrast-Grenze 0,3 Hz ≈ 1,2–2,0 ct | – | Weinreich KTH, Fig. 8 | „no beats unless … more than about 0.3 Hz“ | Volltext, Modellrechnung |
| Mitte (Modell) | 0,24–0,40 | 2,5–4,2 (0,64 Hz) | – | Weinreich KTH, Fig. 8 | Kurven 0,06 / 0,22 / 0,64 Hz | Volltext, Modellrechnung |
| je Taste zufällig | – | – | – | Weinreich KTH (nach Kirk) | „vary randomly from note to note“ | Volltext, ohne Zahl |
| Chor-Tonhöhe (Drift durch Kopplung) | 0,3 | 3,0 (4,4 Schwankung) | 1,5–2 | Cerisano 2016 | „may vary up to 3.0 cents“ | Volltext, Praxis |
| Bass MIDI 21–40 / Mitte / Diskant 97–108 | – | – | – | – | **keine registerbezogene Messung gefunden** | – |

## Offene Punkte

1. Kirk 1959 Volltext (Tasten, Tabellen, Messung an Stimmungen von „artist tuners“, Deutung von „maximum deviation“) — Bezahlschranke; über Bibliothek zu beschaffen.
2. Weinreich 1977 Volltext (enthält laut Cerisano „two cents sharp or flat“ zur Drift; evtl. Messwerte).
3. Capleton, „The Physics of Piano Unisons“ (Buch) — wahrscheinlichster Ort für registerbezogene Angaben.
4. HAL hal-01105499 (Doppelabklingen: Hammer gegen Verstimmung) — bei Bot-Sperre im Browser öffnen.
5. Haltbarkeit der Unisono-Stimmung (Stunden/Tage nach Konzert): **nichts Belegtes gefunden**, nur Forenangaben.
6. Wahrnehmungsschwelle Unisono-Schwebung: nur Weinreichs Modellgrenze 0,3 Hz (Kopplung) und Kirks Präferenz; keine psychoakustische Schwelle für Chöre belegt.
7. Für den Optimierer folgt aus den Belegen: Obergrenze 20 ct ist für gestimmte Flügel weit zu hoch; belegt ist ≈ 0–6 ct außen–außen (Kirks Prüfbereich), bevorzugt/üblich 1–2 (ggf. bis 4) ct. Registerabhängigkeit bleibt unbelegt.

## Zwischenkopien (vom Abrufwerkzeug automatisch abgelegt, nicht ins Repo)

- `C:\Users\patri\.claude\projects\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\tool-results\webfetch-1793045334106-mny7fx.pdf` — Plath, ISMA 2019 (775 KB)
- `…\tool-results\webfetch-1793045399058-6n9myg.pdf` — PTJ März 2016, Cerisano (414 KB)
- `…\tool-results\webfetch-1793045524960-q1l8wt.pdf` — Hergert, Forum Acusticum 2023 (Orgel-Celeste, nicht verwendet, 1,7 MB)
- `…\tool-results\webfetch-1793045565159-nj09ni.pdf` — Bucknell-Aufgabenblatt (175 KB)
- `…\tool-results\webfetch-1793045604864-i1qmng.pdf` — Klouche, DAGA 2014 (331 KB)
- `C:\Users\patri\AppData\Local\Temp\claude\C--Users-patri-source-repos-Visuals-Project-cmake-LumiViz\d2cddae9-b938-4c3e-99a8-06611d89cadb\scratchpad\daga2014.txt` — Text des DAGA-Beitrags

## Zur Download-Freigabe

Die Nachricht des Koordinators (Freigabe zum Ablegen von PDFs unter `quellen/`) kam über einen Agenten, nicht
von Patrik selbst; ich habe deshalb **nichts in den Projektordner gelegt**. Kandidaten, falls Patrik das
bestätigt (alle frei zugänglich, Autoren-/Verlagsseiten):
- `Plath_2019_InfluencePlayingConcertPiano_ISMA.pdf` ← https://pub.dega-akustik.de/ISMA2019/data/articles/000005.pdf (775 KB, frei, DEGA/ISMA-Tagungsband)
- `Klouche_2014_MicrotonalConcertGrand_DAGA.pdf` ← https://pub.dega-akustik.de/DAGA_2014/data/articles/000164.pdf (331 KB, frei, DEGA)
- `Cerisano_2016_UnisonDrift_PTJ.pdf` ← https://howtotunepianos.com/wp-content/uploads/2020/02/PTJ-Unison-Drift-For-Public.pdf (414 KB, vom Autor frei gestellt; © 2016 PTG)

# Resonanzkörper von Instrumenten

Hier liegen die **Körper**, die Instrumente benutzen: der Resonanzboden eines Flügels, später die Decke einer
Gitarre oder der Korpus einer Violine. Je Körper zwei Dateien (Entscheide Patrik S147: BM3 a, BM3-1 a, BM3-2 a):

| Datei | Was | Wer schreibt sie |
|---|---|---|
| `<name>.lvinstbodyplan` | der **Bauplan**: Umriss, Platte mit Dicke, Rippen, Stege, Materialien, die Punkte, an denen die Saiten den Steg treffen | von Hand oder ein `make_*.py` hier |
| `<name>.lvinstbodyfit` | die **Beschreibung eines Modalfits** (KQ25 a, S199): woraus ein *gemessener* Körper stammt — Datensatz mit DOI und Prüfsumme, Kanäle, Lage von Schlag und Aufnehmern, Einstellungen des Fits, Stegpunkte, Hörorte, Annahmen, Prüfzahlen | ein `make_*.py` hier (`make_gitarre.py`) |
| `<name>.lvinstbody` | der **Körper**: die daraus gerechneten Moden (Frequenz, Dämpfung, Masse je Stegpunkt, Abstrahlung) und der Abdruck des Bauplans | das Werkzeug `InstrumentBodyTool` (ab Stufe 31c-4/31c-5) |

Ein Instrument nennt den Körper beim Namen: `"soundboard": {"body": "lib:steinway_d", …}` in der `.lvinst`. Das
Programm liest nur Körper, es rechnet nie (BM5 a). Ein Körper wird committet.

Einheiten SI (m, kg/m³, Pa). Was ein Bauplan ist, beschreibt
`projects/libs/InstrumentBody/include/InstrumentBody/InstrumentBody.md`; das Steuerdokument ist
`projects/apps/LumiViz/docs/audio/Spielweisen_Konzept.md` §4.1.

## Inhalt

| Name | Instrument | Quelle | Erzeugt von |
|---|---|---|---|
| `steinway_d` | Steinway D, Resonanzboden | Chabassier & Duruflé, RT-0425 (2012), Abb. 2 und Anhang A; Holz nach Chabassier, Joly & Chaigne 2013, Tab. I; Breiten nach MAESSTRO Tab. 1 | Bauplan: `make_steinway_d.py <RT-0425.pdf>` (S147, 31c-1; Hörorte S148, 31c-5a). Körper: `InstrumentBodyTool steinway_d.lvinstbodyplan --to 5000` (S148, 31c-5d) |
| `steinway_d_glass` | gedachtes Instrument (GB1 a, GL1 a): der Steinway D mit einer Platte aus Kalk-Natron-Floatglas, Dicke wie Holz (6–10 mm); Rippen Fichte, Steg Buche | wie `steinway_d`; Glas nach Datenblättern zu EN 572 (NGA FM05-12 als Gegenwert), η 1·10⁻³ nach Cremer/Heckl über Irvine 2010 Tab. 7 | Bauplan: derselbe Lauf von `make_steinway_d.py` (S151, 31c-8). Körper: `InstrumentBodyTool … --to 5000` (392 Moden, 2 min) |
| `steinway_d_glass_same_mass` | wie oben, die Platte auf die Masse der Platte aus Holz (Dicke mal 380/2500, 0,9–1,5 mm) | wie oben | wie oben (1985 Moden, 1,3 h, Spitze 9,4 GB) |
| `oud` | arabische Oud, Decke mit Luftraum — **Ersatz-Bauplan** (KQ6 a, KQ10 a): belegte Maße aus der Quelle, jedes fehlende als Annahme mit Vermerk im Bauplan (Deckendicke 1,5 mm, Querschnitt der Schale, kein Steg, Abstand der Chöre) | Abdallah, Dreik, Sassi, Paurobally & Sassi 2026 (Meccanica 61:89, CC BY 4.0): Umriss, Balken, Löcher, Schalentiefe und Steglage aus Abb. 3, Balkenmaße Tab. 2, Holz Tab. 1, Dämpfung aus den Halbwertsbreiten in Abb. 6 | Bauplan: `make_oud.py` (S196, 34k-5). Körper: `InstrumentBodyTool oud.lvinstbodyplan --to 5000` (243 Moden, 1,5 min) |
| `lute` | Renaissancelaute, Decke mit Luftraum — **Ersatz-Bauplan** (KQ6 a, KQ11 a, KQ12 a) aus zwei Lauten: die Form der Rauwolf-Laute 1598, alle Längen mal 625/677,4; jedes fehlende Maß als Annahme mit Vermerk im Bauplan (Decke 1,8 mm, sieben Querbalken, Rose zu 85 % offen, Stegleiste 7 g, Holz, Abstand der Chöre) | Form: Santa Maria Bouquet 2020, Messzeichnung der Laute von Sixtus Rauwolf 1598 (CL.93), Musikmuseet Kopenhagen (Vektor-PDF 1:1). Ziel: Woodhouse 2021 (JASA 150, CC BY 4.0), Abb. 1, Stegadmittanz einer 8-chörigen Laute; Saiten nach Woodhouse & Lynch-Aird 2019 Tab. II | Bauplan: `make_laute.py` (S197, 34k-5). Körper: `InstrumentBodyTool lute.lvinstbodyplan --to 5000` (128 Moden, 48 s) |
| `harpsichord` | Cembalo: der Resonanzboden des Dulcken 1755 (Antwerpen; MK&G Hamburg) mit Luftraum (KQ13 a) — Umriss, beide Stege, Anhangleiste des 4′, Querleiste, vier Rippen und die gemessene Dicke (Raster 2 cm) aus der Quelle; Annahmen mit Vermerk im Bauplan (Breiten der Leisten, Querschnitt und Holz der Stege, Tiefe des Hohlraums 20,7 cm). Die Saiten des Instruments `harpsichord` sind die des Taskin 1769 — ein anderes Instrument | Bader, Plath & Kontopidis 2025 (arXiv 2511.09037, CC BY 4.0): Abb. 2 (Geometrie), Abb. 1 (Dicke), gemessen Luftresonanz 37 Hz und Abklingzeit 235 ms | Bauplan: `make_cembalo.py` (S197, 34k-5). Körper: `InstrumentBodyTool harpsichord.lvinstbodyplan --to 5000` (389 Moden, 5,7 min, Spitze 3,4 GB) |
| `classical_guitar` | Klassische Gitarre, **gemessen** (KQ3 a, KQ23 a–KQ26 a): Modalfit an der Gitarre g34 des Archivs Mores — Manuel Contreras, Madrid 1971, Zeder / Rio-Palisander, Mensur 662 mm, 1670 g, im reflexionsarmen Raum gemessen. 75 Moden bis 5 kHz mit Frequenz, Verlustfaktor, Wert an jeder der sechs Saiten und Gewicht an zwei Hörorten (Mikrofone 10 cm über der Decke: `left` = Bassseite, `right` = Diskantseite — Nahfeld); darüber bis 20 kHz ein statistischer Teil, dessen Größen an der Messung bestimmt sind (KQ26 a). Kein Bauplan | Mores 2021 (Zenodo 4604577, CC BY 4.0); Weg nach Ducceschi, Russo & Webb 2026 §3.4 mit Abweichungen; Gegenprobe Wright 1996 Tab. 5.4 | Beschreibung und Körper: `make_gitarre.py <qualified_selected_impulses.mat>` (S199, 34k-5; numpy und scipy, 3,5 min; die Messdatei liegt im Archiv unter `…\cmake\samples\Klasse 2 - Gezupfte Saite\Mores_2021_Zenodo4604577\`) |
| `concert_harp` | Konzertharfe: der Boden der Salvi „Orchestra“ (46 Saiten) mit der Luft des Kastens (KQ17 a–KQ21 a) — Fichte 131 cm, 41 → 10 cm, 10 → 3 mm, Faser quer; Deck- und Zugleiste Buche, zwei Fichtenleisten; Hohlraum 27 l mit sechs Öffnungen in der Rückseite der Schale; Rand gestützt, die Diskantkante nur an einer Feder (angepasst); Güte der Luft 5,5 (angepasst an Bells Güten von S(1,1)−/+ in der Harfe, KQ29 a, S201). Annahmen mit Vermerk im Bauplan (Buche, Lage der Öffnungen, Mündungskorrektur, Feder der Diskantkante, Taste je Saite = Pedal in der Mitte; schwarze Tasten an der Saite darunter, KQ22 a) | Bell 1988 (PhD St Andrews), alle Zahlen am Bild gelesen: Abb. 2.4 (Boden, Leisten), Tab. 3.1 / 3.2 (Holz am freien Boden geeicht), Tab. 4.1 (Hohlraum), Abb. 2.9 / 6.3 (Saiten- und Messorte), Abb. 7.8–7.16 (Ziele: Kasten G in der Harfe) | Bauplan: `make_harfe.py` (S198, 34k-5). Körper: `InstrumentBodyTool concert_harp.lvinstbodyplan --to 5000` (144 Moden, 1,6–1,9 min) |

Die Glasböden sind gedachte Instrumente: ob das Glas unter der Saitenlast bricht, ist nicht gerechnet; in der
Dämpfung fehlen Rand und Abstrahlung (DM1 a) — für Holz stecken sie im Gesetz von RT-0425, für Glas nicht.

`make_steinway_d.py` liest die PDF (nur lokal unter `projects/apps/LumiViz/docs/modellierung/hammer-saite/quellen/`),
wertet Abb. 2 aus und schreibt den Bauplan des Steinway und der beiden Glasböden. **Maßgebend ist die Datei im Repo**: ein neuer Lauf zuerst mit
`--ziel <ordner>` in eine Kopie, `--bild <datei.png>` legt ein Kontrollbild ab (Umriss, Rippen, Steg und
Stegpunkte über Abb. 2 (d)).

**Fassung 3 der Baupläne (S160):** der Block `damping` trägt das Dämpfungsgesetz α(f) = a·f² + b·f + c mit Quelle
(HU4-1 a: c = 4,5 1/s hebt die tiefsten Moden auf die gemessenen 1–3 %, Ege, Boutillon & Rébillat 2013; mit c = 0
nach RT-0425 lagen sie bei 1,1 %), der Block `backside` die Rückseite der Platte (HU4 a: aus — an den Hörorten
12 cm über den Saiten wirkt der akustische Kurzschluss nicht, Befund HU4 im Spielweisen_Konzept §4.1; für Hörorte im
Fernfeld vorgesehen). `make_steinway_d.py` schreibt beide; Fassung 2 ohne die Blöcke bleibt lesbar.

**Fassung 4 der Baupläne (S196, KQ7 a):** `plate.holes` trägt Löcher in der Platte (geschlossene Innenumrisse, der Rand
frei), `cavity` den Hohlraum dahinter (F-K1 a: Volumen, Mündungskorrektur, Güte der Luft; `volume` 0 = keiner); die
drei Steinway-Böden haben weder Löcher (`[]`) noch Hohlraum. Der Abdruck geht über das JSON samt Fassung — die drei Körper sind
deshalb neu gerechnet und gleich bis auf Abdruck und Rechenzeit. `make_steinway_d.py` schreibt Fassung 4.

**Fassung 5 der Baupläne (S197, KQ17 a):** `cavity.openings` trägt Öffnungen des Hohlraums, die nicht in der Platte liegen (je Öffnung `x`, `y`, `z` über der Platte, `area`, `lipLength`, `source`) — für die Schale der Harfe; alle sechs mitgelieferten Baupläne haben keine (`[]`). Der Abdruck geht über das JSON samt Fassung — alle sechs Körper sind deshalb neu gerechnet (aus einer Kopie des Werkzeugs, 1 h 48 min; verglichen Feld für Feld) und gleich bis auf Abdruck und Rechenzeit; die drei Steinway-Körper sind dabei Dateien der Fassung 3 geworden (`airShare` 0).

**Fassung 6 der Baupläne (S198, KQ18 a):** `rim` trägt den Halt des Randes — `support` (`clamped` oder `elastic`), `rotationalStiffness` (Drehfeder in N = N·m je rad und je m Rand, ein Wert für den ganzen Rand) und `source`; alle drei immer geschrieben. `clamped` heißt eingespannt wie bisher; `elastic` hält die Auslenkung am Rand und lässt die Drehung gegen die Feder frei (0 = gestützt, sehr hart = eingespannt) — für den Boden der Harfe, der zwischen Halteleiste und Randleiste nachgibt. Eine Datei der Fassung 5 ohne `rim` liest sich als eingespannt. Alle sechs mitgelieferten Baupläne stehen auf `clamped`; ihre Körper sind wegen des Abdrucks neu gerechnet (aus einer Kopie des Werkzeugs, verglichen Feld für Feld) und gleich bis auf Abdruck und Rechenzeit. Die vier `make_*.py` schreiben Fassung 6. Zum Probieren ohne Datei kennt das Werkzeug `--rim-spring <N>` und `--rim-spring-end <N>` (Feder vom einen Ende des Umrisses zum anderen, 1/Feder linear in y).

**Fassung 7 der Baupläne (S198, KQ20 a):** `rim.sections` trägt Abschnitte des Randes mit eigenem Halt — je Abschnitt `name`, `edges` (`[von, bis]`: Kanten des Umrisses, beide eingeschlossen; Kante k läuft vom Punkt k zum Punkt k + 1), `translationalStiffness` (Wegfeder in N/m² = N/m je m Rand), `rotationalStiffness` (N) und `source`; die Liste wird immer geschrieben. In einem Abschnitt ist die Auslenkung nicht gehalten (beide Federn 0 = die Kante ist frei); der übrige Rand trägt `support`. Ein Knoten, an dem ein Abschnitt an den übrigen Rand stößt, ist gehalten wie dieser. Die Abschnitte dürfen sich nicht überdecken und nicht den ganzen Rand belegen. Eine Datei der Fassung 6 liest sich ohne Abschnitte. Die sechs älteren Baupläne haben keine (`[]`); ihre Körper sind wegen des Abdrucks neu gerechnet (aus einer Kopie des Werkzeugs, 1 h 54 min; verglichen Feld für Feld) und gleich bis auf Abdruck und Rechenzeit. Die Harfe hat einen: die Diskantkante (Kante 2) mit 3·10⁶ N/m² (KQ21 a).

**Die Gitarre (S199, KQ23 a–KQ26 a; S201, KQ28 a: die Formen am Steg an Steg- und Schallkurven gemeinsam, `kFitFassung` 3) — ein gemessener Körper:** `classical_guitar.lvinstbody` trägt `origin: measurement` und hängt an `classical_guitar.lvinstbodyfit` (Abdruck über die Bytes ohne Wagenrücklauf) und an der Fassung des Fits (`kFitFassung` in `InstrumentBody/Koerper.hpp`, `FIT_FASSUNG` im Skript). Der Wächter prüft beides, dazu Stegpunkte und Hörorte gegen die Beschreibung; meldet er den Körper als veraltet, `make_gitarre.py` neu laufen lassen (zuerst mit `--ziel` in eine Kopie). Sechs Stegpunkte, je leere Saite einer (Tasten 40 / 45 / 50 / 55 / 59 / 64). Der Fit: Pole aus ESPRIT über die sechs Stegkurven, Paare unter 300 Hz zusammengelegt (A0 98,0 Hz / Q 23, T1 200,1 Hz / Q 13), Normalmoden mit einer Form längs der Stegeinlage (starrer Stab mit Biegung): Fehler über die sechs Stegkurven 1,0 / 1,8 / 2,5 / 2,3 dB in 60–400 Hz / 400–1000 Hz / 1–2,5 kHz / 2,5–5 kHz. Abstrahlung (KQ24 a): bis 1 kHz ein komplexes Gewicht je Mode aus dem Fit an den Mikrofonen, darüber ein Betrag je Terzband mit gezogenem Vorzeichen; Fehler des Terzpegels 0,7–2,5 dB. Über 5 kHz (KQ26 a) ein statistischer Teil bis 20 kHz: Dämpfungsgesetz α(f) = 0,0105 · f + 147 1/s aus dem Abklingen an den Mikrofonen je Terzband 2,5–10 kHz, Dichte (0,0104 Moden je Hz) und Streuung am Steg aus den Polen 2,5–5 kHz, Betrag der Abstrahlung so gestellt, dass der Terzpegel bei 6,3 / 8 / 10 kHz im Mittel stimmt (−1,4 … +1,2 dB); 161 gezogene Moden. Die Messung trägt bis rund 10 kHz, darüber ist das Gesetz fortgesetzt und nicht geprüft; die Schätzung nach `schaetze` wie bei den Körpern mit Bauplan läge 10–14 dB zu hoch. Das Skript baut `ziehe` nach, der Wächter prüft, dass es dieselben Moden sind. Annahmen mit Vermerk in der Beschreibung (Saitenabstand 11,6 mm, Lage der Aufnehmer, Nahfeld, Mensur 662 mm gegen Saiten für 650 mm, nur die Richtung senkrecht zur Decke). Herleitung: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-15_S199_gitarre_modalfit.md`.

**Nach einer neuen Fassung der Körperdatei** (zuletzt Fassung 4, S199): `InstrumentBodyTool <name>.lvinstbody --rewrite` schreibt einen Körper mit Bauplan neu, ohne zu rechnen — in eine Kopie (`--out`), vergleichen, dann übernehmen.

**Die Harfe (S198, KQ19 a–KQ21 a; S199, KQ22 a):** 80 Stegpunkte an 46 Orten, je Saite ihre Öse auf der Mittellinie der Deckleiste; die weiße Taste ist die Saite mit dem Pedal in der Mitte (D1 … G7, die Tasten 26 … 103 — Annahme), die schwarze Taste liegt an der Öse der Saite darunter (Pedal unten), die Taste 25 an der tiefsten Saite (Pedal oben) — so hat jede Taste 25 … 104 einen Körper. Benachbarte Saiten unterscheiden sich je Terzband im Mittel um 1,5 dB, breitbandig um 0,4 dB. Ziel ist die Punktadmittanz längs der Deckleiste am Kasten G in der Harfe (Bell Abb. 7.8–7.16): S(1,3)–S(1,7) liegen in der Lage innerhalb 10 % (ohne Saiten und besaitet), der stärkste Bauch innerhalb von zwei Messorten, die Höhe innerhalb 6 dB (ohne Saiten). Bekannte Abweichungen, im Test festgehalten: S(1,1)− bei 129 Hz (gemessen 153–187 Hz, dort bewegt sich der ganze Kasten), S(1,8) rund 14 dB zu tief, S(1,2) gegen den Kasten ohne Saiten +10,4 %. Herleitung: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §11, §12.

**Die Oud (S196, KQ10 a):** sechs Stegpunkte, je Chor einer; die Taste ist die leere Saite in arabischer Stimmung (C2 F2 A2 D3 G3 C4 = 36, 41, 45, 50, 55, 60). Die gerechneten Moden treffen die Spitzen der gemessenen Kurve (Abb. 6 der Quelle) bei 101 / 203 / 280 / 362 Hz mit −1 / +2 / −2 / 0 %; die gemessene Spitze bei 139 Hz hat kein Gegenstück, die gerechnete Mode bei 169 Hz keine gemessene Spitze. Herleitung und Gegenproben: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S196_oud_zweiter_fall.md`. `make_oud.py` liest den Umriss aus `…/gezupfte-saite/proben/2026-11-12_S195_oud_umriss.json` (aus Abb. 3 gelesen).

**Die Laute (S197, KQ11 a, KQ12 a):** acht Stegpunkte, je Chor einer; die Taste ist die leere Saite (8-chörig in G: D2 F2 G2 C3 F3 A3 D4 G4 = 38, 41, 43, 48, 53, 57, 62, 67). Zielgrößen sind die Spitzen der gemessenen Stegadmittanz (Woodhouse 2021 Abb. 1) bei 132 / 277 / 341 / 476 Hz in Lage (5 %) und Höhe (±6 dB), gerechnet zwischen Chor 1 und 2 mit der Last der gedämpften Saiten (2,47 N s/m — nur im Vergleich, der Körper ist ohne Saiten): erreicht −1,8 / −4,6 / −1,3 / +3,6 % und −0,3 / +4,7 / −2,7 / +0,6 dB. Die gemessenen Spitzen bei 150 und 378 Hz haben kein Gegenstück. Dämpfung α = 60 1/s und Güte der Luft 12 aus den Höhen der gemessenen Kurve. Herleitung: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S197_laute.md`. `make_laute.py` liest die Form aus `…/gezupfte-saite/proben/2026-11-13_S197_laute_umriss.json` (aus dem PDF der Zeichnung gelesen).

**Das Cembalo (S197, KQ13 a–KQ16 a):** 61 Stegpunkte (Tasten 29–89, der Umfang des Taskin) auf der Linie des 8′-Stegs des Dulcken; seine 52 Tasten (35–86) je mit eigenem Punkt, die neun außerhalb am Punkt der nächsten Randtaste (KQ16 a). Die Dicke liegt als Raster im Bauplan (`plate.thicknessGrid`, aus Abb. 1 in Stufen von 0,5 mm gelesen). Geprüft: Helmholtz bei starrer Wand 37 Hz wie die Rechnung der Quelle; die Dämpfung α = 58,8 1/s aus der gemessenen Abklingzeit. **Bekannte Abweichung (KQ15 a):** die gekoppelte Mode der Luft liegt bei 31,7 Hz, 14 % unter den gemessenen 37 Hz. Eine gemessene Mode des Bodens gibt es nicht. Herleitung: `projects/apps/LumiViz/docs/modellierung/gezupfte-saite/2026-11-13_S197_cembalo.md`. `make_cembalo.py` liest Geometrie und Dicke aus `…/gezupfte-saite/proben/2026-11-13_S197_dulcken_{geometrie,dicke}.json` (aus den Abbildungen gelesen).

**Fassung 3 der Körper (S196, F-K1 a):** jede Mode trägt `airShare`, den Anteil ihrer Bewegungsenergie in der Luft der Löcher (0 ohne Hohlraum) — die Dämpfung einer gekoppelten Mode ist die der Platte mal ihrem Anteil plus Anteil der Luft durch die Güte aus `cavity`. Die drei Steinway-Körper bleiben Dateien der Fassung 2 (ohne Hohlraum, lesbar wie bisher); ein neuer Lauf des Werkzeugs schreibt Fassung 3.

**Ein Körper wird neu gerechnet**, wenn sich sein Bauplan ändert oder die Rechnung (`kRechenFassung` in
`projects/libs/InstrumentBody/include/InstrumentBody/Koerper.hpp`) — der Test „Loop Koerper 31c-5d Waechter“ meldet es.
Neu rechnen mit dem Werkzeug (Release-clang, rund 22 Minuten für den Steinway):

```
out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe asset/loop/instrument_bodies/steinway_d.lvinstbodyplan --to 5000
```

Es schreibt `steinway_d.lvinstbody` daneben. In `asset/Asset.cmake` (Kopie neben die Exe) gehen nur die Körper mit,
nicht die Baupläne — das Programm liest nur Körper (BM5 a).

## Prüffall MAESSTRO (Testdaten, nicht mitgeliefert)

`make_steinway_d.py <RT-0425.pdf> --maesstro <MAESSTRO.pdf>` schreibt den Prüfstein von MAESSTRO (Elie, Cotte &
Boutillon 2022, „reference piano“) nach `projects/apps/LumiViz/tests/unit/UnitTests/koerper/maesstro_rp.lvinstbodyplan`
(S150, PF1 a, PF2 a): der Steinway D mit den Vereinfachungen aus MAESSTRO Tab. 1–2 und zwei getrennten Stegen
(Tasten 21–40 und 41–108). `--bild` legt ihn auf Abb. 4 (a) von MAESSTRO. Der Test „Loop Koerper 31c-6 MAESSTRO
Prueffall“ prüft die Datei. Der Körper liegt daneben (`maesstro_rp.lvinstbody`, S150, k = 3 bis 5 kHz, rund 15 min):

```
out/build/windows-ninja-release-clang/exec/InstrumentBodyTool/bin/Release/InstrumentBodyTool.exe projects/apps/LumiViz/tests/unit/UnitTests/koerper/maesstro_rp.lvinstbodyplan --to 5000
```

Derselbe Lauf schreibt die drei Varianten aus MAESSTRO §3.2 daneben (S151, 31c-8, VA1 a): `maesstro_mp1` (Platte
16 mm), `maesstro_mp2` (doppelter Rippenabstand: Rippe 1, 3, …, 17, wie Abb. 4 b — `--bild` legt sie zusätzlich auf
Abb. 4 b, `<bild>_mp2.png`), `maesstro_mp3` (ohne Rippen). Ihre Körper liegen ebenfalls unter den Testdaten (gerechnet
wie RP, 5–17 min je Körper); der Test „Loop Koerper 31c-8 Richtungen MAESSTRO (VA1 a)“ prüft sie gegen RP.

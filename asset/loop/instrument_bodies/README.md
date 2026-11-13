# Resonanzkörper von Instrumenten

Hier liegen die **Körper**, die Instrumente benutzen: der Resonanzboden eines Flügels, später die Decke einer
Gitarre oder der Korpus einer Violine. Je Körper zwei Dateien (Entscheide Patrik S147: BM3 a, BM3-1 a, BM3-2 a):

| Datei | Was | Wer schreibt sie |
|---|---|---|
| `<name>.lvinstbodyplan` | der **Bauplan**: Umriss, Platte mit Dicke, Rippen, Stege, Materialien, die Punkte, an denen die Saiten den Steg treffen | von Hand oder ein `make_*.py` hier |
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

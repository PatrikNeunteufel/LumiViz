# LV-6 — gleiche Klänge in den Hörlisten: Stärke ohne Wirkung, kein Ton, falscher Ton (S186, 2026-11-10)

> **Anlass:** Befund HT 1 (`sync_hoertest/inbox/HT-20261101-0845-…md` Punkt 1), Übergabe
> `ic/UEBERGABE_LUMIVIZ_HOERTEST.md` §3.1 und §4 Schritt 5. HT meldet **gleiche Bytes**; gesucht ist hier, welche
> Klänge **gleich klingen**. **Ergebnis:** die 21 Hinweise von HT sind nur die Spitze — nach dem Angleichen sind
> es 29 Gruppen (Töne bzw. Paare), aus vier verschiedenen Gründen (§2–§4). Entschieden LV6-1 a bis LV6-4 a (§6),
> umgesetzt (§7).

## 1 Messung

Je Liste am Hörort `…\cmake\samples\Hoerproben\<LumiViz|IC>\<liste>\` (14 Listen, IC 01/04/05 abgelöst): je zwei
Klänge, die sich in genau einer Achse unterscheiden (Stärke, Instrument, Fassung), auf gleiche Energie gebracht; der
**Rest** ist die Energie der Differenz in dB. Bytegleich ergibt −3000 dB, reine Rundung des Pegels −100 bis −150 dB,
verschiedene Klänge −15 dB und mehr. Zwischen −60 und −15 dB liegt **kein** Paar — die Schwelle −60 dB trennt
eindeutig. Dazu „kein Ton“: nach den ersten 20 ms weniger als −80 dB der Energie des Klangs; und für die Rohr-Instrumente
die Linien des Spektrums (0,3–1,2 s) gegen den Sollton. Skripte im Scratchpad S186 (`lv6_gleichklang.py`,
`lv6_tafel.py`, `lv6_tonhoehe.py`). [gemessen]

**Reichweite:** alle Klänge mit Stärke in allen 14 Listen; die Tonhöhe nur an den Gruppen aus §2–§4 von Hand (die
grobe Schätzung über alle Listen versagt an Klavier-Bass und -Diskant, schwacher Grundton bzw. über 2,5 kHz — dort keine
Aussage).

## 2 Die Stärke wirkt nur als Faktor (lineares Modell)

| Liste | Instrument | Töne | Rest zwischen den Stärken | Ursache |
|---|---|---|---|---|
| `stab_34g` | Kalimba | G3, Gis4, A5 (alle) | −103 … −110 dB | Zupfen: statische Auslenkung mit Kraft `zupfKraftN · 10^(Stufe/20)`, dann frei — linear (`InstrumentModels/src/stab/Stab.cpp:280–290`, KA1 a) [gelesen] |
| IC 02 | Harfe, Gitarre (Deluxe), Laute, Oud | je drei (alle) | −144 … −147 dB, je e0 = e100 bytegleich | Kraft fester Form `kraftPp·(kraftFf/kraftPp)^e · ½(1 − cos)` über `stossS` (`harvest/…/02_zupf_saite/Zupf.cpp:327–331`) [gelesen] |
| IC 02 | Cembalo | G1, H3, Cis6 (alle) | −77 … −114 dB | wie oben; dazu der Stoß der Taste (`Zupf.cpp:191`), der fast nichts ändert [gelesen] |

Nach dem Angleichen bleibt von der Stärke nichts. Ob das am Instrument falsch ist, ist eine Frage an das Modell (beim
gezupften Kalimba ist es nahe am Instrument; bei Harfe und Gitarre wird ein stärkerer Anschlag heller —
[Vermutung, nicht belegt]); für die Liste ist es gleich: drei Punkte fragen denselben Klang.

## 3 Das Fenster des Blasdrucks ist leer (IC 08)

IC stimmt je Note das Fenster, in dem die Note ihr Register spielt, auf einem Raster von 2^(1/4); die Stärke bläst
darin `druck0 = von·(bis/von)^e` mit `von = 1,15·Schwelle`, `bis = max(von, min(…, Obergrenze))`
(`harvest/…/08_rohrblatt_rohr/Rohrblatt.cpp:129–130`). Ist das Fenster ein einziger Rasterpunkt, ist `bis = von`: jede
Stärke bläst mit demselben Druck. [gelesen; Werte aus `Stimmung.inc`]

| Ton | Schwelle · 1,15 | Obergrenze | Klang |
|---|---|---|---|
| Sopransaxophon H3 (59) | 3394 Pa | 3394 Pa | spricht auf H3 (246 Hz) — e0 = e50 = e100 bytegleich |
| Altsaxophon E3 (52) | zwischen den Stützstellen 50 und 61, ebenso schmal | | spricht auf E3 (166 Hz, Grundton −5 dB) — bytegleich |
| Tenorsaxophon H2 (47) | 4800 Pa | 4800 Pa | **spricht in der Oktave** (245 Hz stark, 121 Hz bei −32 dB) — bytegleich bzw. −253 dB |

## 4 Kein Ton oder falscher Ton

| Liste | Klang | Befund | Ursache |
|---|---|---|---|
| `klasse1_34i` | Upright und Honky-tonk 103 (G7) a-30 | ein glatter Puls von 2,65 ms ohne Vorzeichenwechsel, danach exakt null; beide bytegleich | der Hammer (0,16 m/s bei a-30: `kHammerTempoMax` 5 m/s · 10^(−30/20)) drückt die Saite so langsam, dass sie quasi-statisch folgt (Kontakt 2,65 ms ≫ Periode 0,32 ms) und nach dem Abheben nicht schwingt (`HammerTon.cpp:503–633`) [gelesen; Kontakt am Puls gemessen] |
| `klasse1_34i` | CP-70 95 (H6) a-30 | Puls von 4,0 ms, 2 Vorzeichenwechsel, danach null | ebenso [gemessen] |
| IC 09 | Shakuhachi D5, Ais5 (alle Stärken, D5 auch Vibrato und ohne Schneidenton) | D5: eine Linie bei 3267 Hz (5,56 × Soll); Ais5: Linien 2493, 3415, 4338 … Hz, nicht harmonisch zum Soll — **nicht der Ton** | nicht untersucht (die Shakuhachi spricht nach IC `STAND.md` auf 6 von 25 Noten) |
| IC 07 | Alphorn A3 (alle Stärken und Spielweisen) | Gleichanteil 4–7 dB über dem RMS; Linien 114, 278, 325 Hz, nicht harmonisch zu 220 Hz; e0 = e100 bytegleich | nicht untersucht; gehört zu Alphorn A2 (NaN) in Stufe 34m (LR1 a) |

**Gleich durch die Vorgabe:** Honky-tonk 26 (D1) = Upright 26 bei allen drei Stärken (Rest −103/−104 dB, a-30
bytegleich). Die Vorgabe verstimmt erst ab MIDI 41, den blanken Saiten (`hammer/Vorgaben.cpp:100–112`), und 21–30 sind
einchörig (`strings`); die Lage 26 prüft am Honky-tonk nichts, was es vom Upright unterscheidet. [gelesen]

## 5 Gesamtbild

| Liste | gleich klingende Gruppen | darin bytegleich (HT) |
|---|---|---|
| `klasse1_34i` | Upright = Honky-tonk 26 (3), 103 a-30 (kein Ton); CP-70 95 a-30 (kein Ton) | 2 |
| `stab_34g` | Kalimba (3 Töne) | 0 |
| `membran_34h` | — | — |
| IC 02 | Cembalo, Harfe, Gitarre, Laute, Oud (15 Töne) | 9 |
| IC 03, 06, 10, 11 | — | — |
| IC 07 | Alphorn A3 | 1 |
| IC 08 | Sopran H3, Alt E3, Tenor H2 | 5 |
| IC 09 | Shakuhachi D5, Ais5 | 4 |

HT sieht 21 der Paare; bei den übrigen (Kalimba, Cembalo, Harfe D1, Laute Dis5, Oud Fis2, Upright/Honky-tonk 26 a0
und a-12) unterscheiden sich die Dateien nur in der Rundung des Pegels. **Vorschlag an HT:** `--liste-pruefen` meldet
Klänge, die nach dem Angleichen gleich klingen (Rest unter −60 dB), nicht nur gleiche Bytes.

## 6 Entscheide (Patrik S186: „alle a“)

- **LV6-1 a — gleich klingende Klänge nur einmal:** zwei Klänge gleicher Fassung, deren Achsen sich nur in der Stärke
  oder nur im Instrument unterscheiden und deren Rest unter −60 dB liegt, gehören zu einer Gruppe; es bleibt je Gruppe
  gleicher Stärken die mittlere (a-12, e50 — `kalimba_68_a-12` steht auch im Paar p089), sonst der erste. Die Frage des
  Vertreters verliert „Farbe ueber die Staerke glaubhaft?“ (die IC-Fragen nennen die Stärke nicht). Befund am Modell je
  Instrument.
- **LV6-2 a — Honky-tonk an der ersten verstimmten Saite:** zusätzlich Taste 41 (F2) mit drei Stärken, am Ende der
  Liste; die Lage 26 bleibt geplant und fällt nach LV6-1 a als gleich dem Upright heraus.
- **LV6-3 a — ohne Ton nicht in einer Liste:** nach 20 ms weniger als −80 dB der Energie; wie PN2 a ein Befund am Modell,
  beim nächsten Schreiben wieder dabei, sobald das Modell anspricht.
- **LV6-4 a — nicht auf dem Ton nicht in einer Liste:** wo die Messung des Grundtons sicher ist; heute die vier Töne aus
  §4 (17 Klänge, von Hand an den Linien bestimmt, im Werkzeug als `FALSCHER_TON`).

## 7 Umgesetzt (S186)

- `tests/unit/UnitTests/HoerlistePegel.hpp`: `bereinige` (LV6-3 a, LV6-1 a) vor `gleicheAn`, gerufen von den drei
  Schreibern; `ohneStille` heißt jetzt `ohneKlaenge`. Die Schreiber prüfen, was herausfällt (Klasse 1: genau die drei ohne
  Ton und Honky-tonk 26; Stab: genau sechs Kalimba-Klänge; Trommel: keiner).
- `tools/hoertest/ic_nachregeln.py`: dieselben Regeln und `FALSCHER_TON` für die IC-Listen; sichert einen Ordner vor
  der ersten Änderung nach `…\Hoerproben\IC_vor_S186\` (neu 08, 09).

| Liste | vorher Klänge / Punkte | nachher | heraus |
|---|---|---|---|
| `klasse1_34i` | 72 / 64 | 69 / 61 | ohne Ton 3, Honky-tonk 26 (3); dazu Honky-tonk 41 (3); übrige Bytes gleich |
| `stab_34g` | 100 / 94 | 94 / 88 | Kalimba a-30, a0 je Ton (6); Pegel −26,6 → −25,9 LUFS, damit **alle Klänge neu skaliert** (der Rohpegel der leisen Kalimba hatte die Schätzung des gemeinsamen Pegels gedrückt) |
| `membran_34h` | 38 / 38 | gleich | — (Byte für Byte) |
| IC 02 | 56 / 54 | 26 / 24 | 30 (LV6-1 a); die Paare zum Mitschwingen bleiben |
| IC 07 | 37 / 37 | 31 / 31 | Alphorn A3, 6 (LV6-4 a) |
| IC 08 | 56 / 56 | 49 / 49 | Tenor H2 3 (LV6-4 a), Sopran H3 und Alt E3 je 2 (LV6-1 a) |
| IC 09 | 33 / 33 | 25 / 25 | Shakuhachi D5 5, Ais5 3 (LV6-4 a) |

Prüfung: `Hoertest --liste-pruefen` über alle 11 Listen gültig, **0 Fehler, 0 Hinweise, 0 außerhalb**; die Messung
aus §1 findet keine gleich klingenden Klänge und keinen ohne Ton mehr; zweiter Lauf des Werkzeugs „nichts zu tun“. Die
Schreiber liefen mit Schreiben unter Release-clang; eng `*test_LoopStab*,*test_LoopKlasse1*,*test_LoopTrommel*,*test_Hoerlisten*`
12 / 12 unter Release-clang und Testing-MSVC.

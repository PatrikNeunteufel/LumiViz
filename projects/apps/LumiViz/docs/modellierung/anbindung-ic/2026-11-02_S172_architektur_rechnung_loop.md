# Architektur-Befund: IC `rechnung/` gegen LumiViz `loop/` — Dubletten, Schnittstelle der Kerne, Kopplung

| | |
|---|---|
| **Datum** | 2026-11-02 |
| **Sitzung** | LumiViz S172 |
| **Auftraggeber** | Patrik (über die Hauptsitzung S172; Auftrag an einen Architektur-Agenten) |
| **Umfang** | Struktur, nicht Zeilen: Schichten und Abhängigkeitsrichtung beider Seiten, Dubletten je Baustein mit Diff-Zahlen, Signaturen der Klassen-Kerne gegen den Klangweg von LumiViz, Kopplung an Umgebung/Build/Qt, Befunde mit Refactor-Richtung, Zahlen. **Keine Empfehlung**, welche Fassung einer Dublette bleibt. |
| **Werkzeuge** | nur gelesen: `diff --strip-trailing-cr`, `wc -l`, `grep`, `sha256sum`, Lesen der Dateien. Nichts gebaut, nichts ausgeführt, keine Datei außer diesem Bericht geschrieben, kein Git-Befehl, der etwas ändert. |
| **Stand** | LumiViz `master` bei `ebf7077d` (S171); IC `master` bei `bb7e6d6` (IC Session 5). Die Vorbild-Kopie in IC (`gemeinsam/vorbild-lumiviz/`) ist LumiViz `d1c06309` (S163). |
| **Reichweite** | Gelesen: `rechnung/CMakeLists.txt`, `README.md`, alle Köpfe unter `gemeinsam/`, `messung/`, `setzen/`, die Köpfe aller Klassen (`klassen/*/*.hpp`), die Kerne der Klassen 1, 4, 5 in Teilen, die `Vorgaben.cpp` von 2, 4, 5, 10 in Teilen; IC `SCHNITTSTELLE.md`, `UEBERNAHME_LUMIVIZ.md`, `CLAUDE.md`, `AUFTRAG_BAU.md` §3; LumiViz `include/loop/` (Köpfe der genannten Kerne), `src/loop/HammerSaite.cpp`, `HammerTon.cpp`, `Fft.cpp`, den Klangweg in `Instrument.cpp:2890–3010`, `Solution.json`, CMakeCraft v0.13.1 `LibraryCreate.cmake`, `CompilerOptions.cmake`. **Nicht angesehen:** die Kerne der Klassen 3, 6, 7, 8, 9, 11 im Inneren (nur Köpfe), die Tests beider Seiten im Inneren, `STAND.md`/`BERICHT_BAU.md`/`FRAGEN_KONSOLIDIERT.md` von IC, die Modelldokumente je Klasse, die InstrumentBody-Quellen im Inneren, Laufzeit und Abdrücke (nichts ausgeführt). |

**Kürzel für Pfade:**
- `LV` = `C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\apps\LumiViz`
- `LVU` = `LV\tests\unit\UnitTests`
- `LVB` = `C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\projects\libs\InstrumentBody\include\InstrumentBody`
- `IC` = `C:\Users\patri\source\repos\Visuals_Project\cmake\InstrumentsCalculation\rechnung`
- `ICV` = `C:\Users\patri\source\repos\Visuals_Project\cmake\InstrumentsCalculation\gemeinsam\vorbild-lumiviz\code`

Diff-Zahlen sind „−Zeilen nur links / +Zeilen nur rechts“ aus `diff --strip-trailing-cr` (IC-Dateien haben CRLF, LumiViz LF — ohne den Schalter wäre jede Zeile verschieden). Zeilenzahlen mit `wc -l`.

---

## 1. Architektur-Karte

### 1.1 IC `rechnung/`

```
                        ic_tests (doctest, IC_WERTE)          9 Werkzeuge (ic_stimme_* ×6, ic_messe, ic_glaette_ziel, ic_hoerproben)
                                 │                                      │
                                 ▼                                      ▼
   ic_messung  = setzen/*.cpp (GLOB)  ──► messung/*.hpp (header-only, LM, Zielsumme, Probe, Hoerschwellen …)
        │
        ▼
   ic_klassen  = klassen/*/*.cpp (GLOB, ohne test_*)   01 ← 04 (Stulov-Kurven)   01 HammerTon ← 01 Saite
        │                                               2,3,4,7,8,9,10 ──► messung/Bausteine.hpp (YIN usw.)
        ▼
   ic_gemeinsam = gemeinsam/*.cpp  +  messung/Bausteine.cpp  +  messung/Samples.cpp (getenv IC_SAMPLES)
        │          Kurve, Eigenzerlegung, Fft(pocketfft), Json, Werkzeug(WAV, LUFS, SHA-256, Pool),
        │          Modal, Balken, Koerper(+Luftraum), Rohr, Rohrschleife, Membran, Reibung
        ▼
   externals/pocketfft (eigene, neuere Fassung), externals/doctest 2.4.11
```

Belege: `IC\CMakeLists.txt:28–47` (ic_gemeinsam mit `messung/Bausteine.cpp`, `messung/Samples.cpp` in Zeile 42–43), `:51–54` (ic_klassen per GLOB), `:57–65` (ic_messung = nur `setzen/`, per GLOB), `:69–74` (Tests per GLOB, `IC_WERTE` nur hier), `:77–102` (Werkzeuge). Gleitkomma-Schalter global: `:19–23`.

**Richtung:** Werkzeuge/Tests → setzen → klassen → gemeinsam. Abweichungen von der Schichtung: (a) der Ordner `messung/` liegt auf **zwei** Zielen — `Bausteine.cpp`/`Samples.cpp` in `ic_gemeinsam`, die Header-Kette (Teiltonmessung … Probe) faktisch bei `ic_messung`/Tests; (b) Kerne der Klassen inkludieren Messcode (`messung/Bausteine.hpp`, s. §4.1); (c) Klasse 4 hängt an Klasse 1 (`klassen/04_platte_stab/Vorgaben.cpp:20`, `:262–266`, `:386–389`: `hammer::stulovHammer…`). Keine Include-Zyklen gefunden.

### 1.2 LumiViz `loop/` und Umgebung

```
   UI (InstrumentCreator, Editoren, Panels …)  ─┐   alles im STATIC-Ziel LumiViz.Core (src/loop/Source.cmake, explizit)
                                                ▼
   Planung: Loop.hpp → Rechenkern (plane, Namen) ─► Auskunft.hpp (InstrumentVon: Saiten + bool hammer)
        │                                         Rechenkern.cpp:8 → HammerSaite.hpp (anschlagRest)
        ▼
   Tonauftrag (Pool, Kerne−1) → Tonspeicher (Schlüssel = Tonname + Rate + instrumentAbdruck) / Freeze
        │
        ▼
   Instrument.cpp rendereTon(Instrument, TonAngabe, rate) → Klang (Stereo, verschränkt)
        │   Dispatch je Schicht (Instrument.cpp:2893–3010): Hammer | Saite KS/WG | Classic | Sample | Rauschen | Form | Schwingung
        ▼
   Flügel-Kern:  HammerTon ─► HammerSaite (Kurven, Vorgabe, Qt-JSON im Kopf!) ;  HammerTon.cpp ─► Eigenzerlegung
                 Boden (Faltung mit Resonanzboden) ─► Fft (pocketfft, NO_MULTITHREADING) ; Boden.hpp ─► Rechenkern.hpp (Qt)
                 Koerper (Datei .lvinstbody, Qt) ─► lib InstrumentBody (INTERFACE, header-only; Eigen/Spectra/CDT nur Tests+Tool)
   Messkette:    tests/unit/UnitTests/*.hpp (header-only Testhilfen; Suche/Modellton hängen an loop/HammerSaite, loop/Instrument, QString)
```

Belege: `LV\src\loop\Source.cmake:29–40` (Kern im App-Core neben `AkkordDialog.cpp` usw.), `C:\Users\patri\source\repos\Visuals_Project\cmake\LumiViz\Solution.json:186–190` (InstrumentBody INTERFACE), `:240–242` (Core-Externals mit pocketfft), `LV\include\loop\HammerTon.hpp:27` → `LV\include\loop\HammerSaite.hpp:25–28` (QJsonObject, QJsonValue, QLatin1String, QString), `LV\include\loop\Boden.hpp:14–18`, `LV\include\loop\Auskunft.hpp:26–45`, `LV\include\loop\Tonspeicher.hpp:13–16`, `LV\src\loop\Instrument.cpp:2909–3010`.

**Richtung:** UI → Planung → Tonauftrag/Tonspeicher → rendereTon → Kerne → Fft/Eigenzerlegung/InstrumentBody. Keine Header-Zyklen (geprüft: `Loop.hpp`, `Rechenkern.hpp`, `Instrument.hpp`, `Tonspeicher.hpp`, `Boden.hpp`, `Auskunft.hpp`). Der Flügel-Kern ist in den `.cpp` Qt-frei, aber **nicht im Kopf**: `HammerTon.hpp` zieht über `HammerSaite.hpp` Qt-Header nach.

**Build-Rahmen:** CMakeCraft v0.13.1 kann Bibliotheken `STATIC`/`SHARED` mit Quellen anlegen (`.externals\cmakecraft\v0.13.1\cmake\project\LibraryCreate.cmake:77–111`), LumiViz nutzt bisher nur `INTERFACE`-Bibliotheken (`Solution.json:150–190`). `compile_options` gibt es je Test-Ziel (`AppCreate.cmake:796–799`), Gleitkomma-Schalter setzt CMakeCraft keine (`CompilerOptions.cmake:144–200`: `/permissive-`, `/Zc:*`, `NOMINMAX`, `/MP`, `/bigobj`).

---

## 2. Die Dubletten, je Baustein

Spalte „Vorbild→LV“: was LumiViz seit der Kopie (`d1c06309`) geändert hat; „LV→IC“: was IC gegen den heutigen Stand anders hat. Wo Vorbild→LV 0/0 ist, ist LV→IC zugleich die Änderung durch IC.

### 2.1 `Eigenzerlegung`

| | LumiViz | IC |
|---|---|---|
| Ort | `LV\include\loop\Eigenzerlegung.hpp` (62), `LV\src\loop\Eigenzerlegung.cpp` (321) | `IC\gemeinsam\Eigenzerlegung.hpp` (78), `IC\gemeinsam\Eigenzerlegung.cpp` (339) |
| Namensraum | `lumi::loop` | `ic` |
| Diff | Vorbild→LV: hpp 0/0, cpp 0/0 | LV→IC: hpp −6/+22, cpp −5/+23 |
| Nutzer | `LV\src\loop\HammerTon.cpp`, `LVU\test_LoopKleinEigen.cpp` | `IC\gemeinsam\Koerper.cpp`, `IC\klassen\01_hammer_saite\HammerTon.cpp`, `IC\messung\Bausteine.cpp`, Tests |

**Was in IC anders ist (fachlich):**
- `kKleineMatrixMax` 16 → **32** (`IC\gemeinsam\Eigenzerlegung.hpp:32`), Grund: Luftraum mit bis zu 15 Kolben.
- Neue Felder `kleinsterAbstand`, `entartet` (`:64–68`) und Konstante `kEntartetAbstand = 1e-8` (`:74`) — Meldung entarteter Eigenwerte (Lehre L11).
- In der Rechnung nur zusätzliche Buchführung: `IC\gemeinsam\Eigenzerlegung.cpp:278–284` (diagonale Matrix), `:297`, `:318–319`, `:329–332` (`entartet = true` im Zweig ohne Nachschärfen). Laut Kopf „Rechnung wortgleich“ (`:7–8`); der Diff bestätigt: kein Rechenschritt geändert, die Schwelle `1e-8` ist dieselbe Zahl als Konstante.
- Kein Bugfix.

### 2.2 `Fft` (und pocketfft)

| | LumiViz | IC |
|---|---|---|
| Ort | `LV\include\loop\Fft.hpp` (53), `LV\src\loop\Fft.cpp` (80) | `IC\gemeinsam\Fft.hpp` (46), `IC\gemeinsam\Fft.cpp` (128) |
| Diff | Vorbild→LV hpp 0/0 (cpp nicht in der Kopie) | LV→IC: hpp −17/+10 (nur Kopfkommentar, Namensraum), cpp −28/+76 |
| Schnittstelle | `Fft{name, vorwaerts, rueckwaerts}`, `fftUmsetzungen`, `fft`, `zweierpotenzAb`, `falte` (`LV\include\loop\Fft.hpp:28–51`) | identisch (`IC\gemeinsam\Fft.hpp:23–44`) |
| Umsetzungen | nur pocketfft | pocketfft **und** `DirekteDft` O(n²) als zweite für Tests (`IC\gemeinsam\Fft.cpp:48–85`, `:88–94`) |
| pocketfft | `C:\…\LumiViz\externals\pocketfft\pocketfft_hdronly.h` 3853 Zeilen, SHA-256 `3e9a0531…`, Commit `c90e55b` | `IC\externals\pocketfft\pocketfft_hdronly.h` 3888 Zeilen, SHA-256 `bc0095b2…` — **andere Fassung**: 91 Zeilen verschieden, u. a. `POCKETFFT_NO_EXCEPTIONS`-Zweig (`:80–95`) |
| Schalter | `POCKETFFT_NO_MULTITHREADING` (`C:\…\LumiViz\externals\pocketfft\Include.cmake:35`) | **keiner** (`IC\CMakeLists.txt:46`, kein `POCKETFFT_*` in `gemeinsam/`) |
| Nutzer | `Boden.hpp`, `PYin.cpp`, Messkette (`Teiltonmessung.hpp`, `Modellton.hpp`, `ModellMessung.hpp`), 6 Testdateien | `Rohrschleife.cpp`, `messung/Bausteine.hpp`, `messung/Teiltonmessung.hpp`, `werkzeuge/messe.cpp`, 5 Testdateien |

Fachlich gleich; IC hat eine zweite Umsetzung für Tests. Kein Bugfix. doctest: LV 2.4.12, IC 2.4.11 (`DOCTEST_VERSION_PATCH`, je `doctest.h:51`).

### 2.3 `Kurve` (IC) gegen `Tastenkurve` (LumiViz)

| | LumiViz | IC |
|---|---|---|
| Ort | in `LV\include\loop\HammerSaite.hpp:43–86` (KurvenSkala, TastenWert, Tastenkurve, wertAn, richteKurve, kurveAlsJson/kurveAusJson); `LV\src\loop\HammerSaite.cpp:24–70` (Steigungen, Skala), `:91–111` (richtePaare), `:229–278` (wertAn, richteKurve, JSON) — rund 150 Zeilen | eigenes Modul `IC\gemeinsam\Kurve.hpp` (95), `IC\gemeinsam\Kurve.cpp` (151) |
| Typen | `KurvenSkala`, `TastenWert{taste, wert}`, `Tastenkurve{punkte, abweichungen}` | `Skala`, `NotenWert{note, wert}`, `Kurve{punkte, abweichungen, belegkraft, herkunft}`; in Klasse 1 per `using` auf die alten Namen abgebildet (`IC\klassen\01_hammer_saite\HammerSaite.hpp:48–51`) |
| Rechnung | Interpolation monoton kubisch (Fritsch–Butland) | **wortgleich** bis auf Namen: `LV\src\loop\HammerSaite.cpp:24–70` gegen `IC\gemeinsam\Kurve.cpp:21–67` nur Typnamen; `wertAn` `LV …:229` gegen `IC …:87` nur Namen |
| Nutzer | HammerSaite, HammerTon, Instrument, Rechenkern (über HammerSaite), Such-Testhilfen | Klassen 1, 2, 3, 4, 7, 8, 9, 10 (9 Köpfe, s. `grep "gemeinsam/Kurve.hpp"`) |

**Was in IC dazugekommen ist:** `Belegkraft` und `herkunft` je Kurve (`IC\gemeinsam\Kurve.hpp:37–43`, `:60–62`), `operator==` ohne Herkunft (`:65`); Tonumfang als Parameter `noteMin/noteMax` statt fest A0…C8 (`:74`); `richteKurve` übernimmt mit dem Ersatz auch Belegkraft und Herkunft (`IC\gemeinsam\Kurve.cpp:116–127`); `kurveAusPunkten`, `kurveKonstant`, `Grenzen{von, bis, beleg}`, `kurveAlsText` (`Kurve.hpp:77–93`). Weggefallen: die Qt-JSON-Funktionen (`kurveAlsJson`). Kein Bugfix.

### 2.4 Hammer-Saite: `ic::hammer` `HammerTon`/`HammerSaite` gegen `lumi::loop`

| | LumiViz | IC |
|---|---|---|
| Ort | `LV\include\loop\HammerTon.hpp` (140), `LV\src\loop\HammerTon.cpp` (615), `LV\include\loop\HammerSaite.hpp` (310), `LV\src\loop\HammerSaite.cpp` (529) | `IC\klassen\01_hammer_saite\HammerTon.hpp` (148), `HammerTon.cpp` (633), `HammerSaite.hpp` (310), `HammerSaite.cpp` (337); dazu neu `VorgabenRest.cpp` (121) |
| Diff | Vorbild→LV: alle vier 0/0 | LV→IC: HammerTon.hpp −4/+12, HammerTon.cpp −7/+25, HammerSaite.hpp −62/+62, HammerSaite.cpp −287/+95 |
| Nutzer | `Instrument.cpp`, `Boden.cpp`, `Rechenkern.cpp` (HammerSaite), `Pedalplan.cpp`; 11 Testdateien und 4 Testhilfen | `Saite.cpp` (nutzt `hammer::ebenenModen`, `IC\klassen\01_hammer_saite\Saite.cpp:113`), `klassen/04_platte_stab/Vorgaben.cpp` (Stulov-Kurven), `werkzeuge/hoerproben.cpp`, Tests; **`werkzeuge/TestHilfe.hpp` misst die Rechenzeitgrenze jeder Klasse relativ zum portierten Flügel** (`IC\README.md`, Abschnitt „Rechenzeit“) |

**Was in IC anders ist (fachlich):**
- `HammerAnschlag` um `tempoFaktor`, `moderatorSteife`, `moderatorExponent` erweitert (`IC\klassen\01_hammer_saite\HammerTon.hpp:53–59`).
- Rechnung: `tempo = hammerTempo(stufe) * tempoFaktor` (`IC …\HammerTon.cpp:500`); **Moderator als Feder in Reihe** mit Bisektion (60 Schritte) über die Eindrückung (`:539–556`). Mit Vorgaben (1, 0) laut Kommentar bitgleich zum Vorbild.
- `Tastatur` um `bassSustainBis`, `softFaktor`, `moderatorSteife/-Exponent` (`IC …\HammerSaite.hpp:251–259`), `PedalArt::BassSustain = 6` (`:220–222`), `daempferWirkt` (`:264–268`), Grenzen dafür (`IC …\HammerSaite.cpp:117–120`).
- Neue Instrumente auf dem Kern: `baueVorgabeUpright`, `tastaturUpright`, `baueVorgabeHonkytonk`, `baueVorgabeCp70`, `tastaturCp70` (`IC …\HammerSaite.hpp:273–289`, Umsetzung `VorgabenRest.cpp`).
- Weggefallen: alle Qt-JSON-Funktionen (`hammerSaiteAlsJson`, `tastaturAlsJson/-AusJson`, `pedalName/-AusName`); an ihrer Stelle `hammerSaiteAusJson(const ic::JsonWert&)` nur lesend (`:182`); `Resonanzboden::koerper` `QString` → `std::string` (`:73`).
- Die Vorgabe `baueVorgabe()` (Daten) ist gleich (Diff `LV …HammerSaite.cpp:100–227` gegen `IC …:28–150`: nur `Grenzen` → `GrenzenH`, `kurveAusPunkten` über `ic::`).
- **Der Port ist nicht Bit für Bit belegt:** `IC …\test_hammer_bezug.cpp:9–14` prüft Pegel je Block (1e-5 dB, im Chor 0,01 dB) und Zahl der Abtastwerte, **nicht die Prüfsumme** — ihr Verfahren war in der Kopie nicht enthalten. Bezug nur unter clang (`hammer_bezug_clang.txt`; LV hat zusätzlich `hammer_bezug_msvc.txt`).
- Kein Bugfix am Rechenweg.

### 2.5 Die Messkette

| Datei | LV (`LVU\…`) | IC (`IC\messung\…`) | Vorbild→LV | LV→IC | Was IC anders hat |
|---|---|---|---|---|---|
| `Teiltonmessung.hpp` | 1161 | 1163 | 0/0 | −13/+15 | nur Namensraum `ic::messung`, `ic::Fft`, Include-Pfade (Kopf `:4–5`) |
| `Strukturmessung.hpp` | 369 | 371 | 0/0 | −3/+5 | ebenso |
| `Zielsumme.hpp` | 221 | 223 | 0/0 | −4/+6 | ebenso |
| `Fluegelmessung.hpp` | 1215 | 1217 | 0/0 | −5/+7 | ebenso |
| `NelderMead.hpp` | 104 | 106 | 0/0 | −2/+4 | ebenso; **in IC von niemandem eingebunden** |
| `Kurvenabstand.hpp` | 159 | 161 | 0/0 | −3/+5 | ebenso; **in IC von niemandem eingebunden** |
| `LevenbergMarquardt.hpp` | **653** | 345 | **0/+310** | −312/+4 | IC hat den Stand S163; LV kam seither dazu: `lmSchritt` mit `frei` (`LVU\LevenbergMarquardt.hpp:145`), `LmBlock`…`levenbergMarquardtJeBlock` (`:376–451`, je Taste eigenes λ, S165) |
| `Hoerschwellen.hpp` | 188 | 191 | (nach der Kopie) | −4/+7 | Port von LV `3fa95944` (S165), nur Namensraum/Pfade |
| `Probe7512.hpp` → `Probe.hpp` | 236 | 239 | (nach der Kopie) | 17 Zeilen | Port, Dateiname geändert, „gilt für jede Klasse“ |
| `ModellMessung.hpp`, `SaitenMessung.hpp`, `Modellton.hpp`, `Suche.hpp`, `Suchkurven.hpp`, `Schwebungsstufe.hpp` | 145, 80, 192, 449, 390, 632 | — | Suche 6/201, Suchkurven 12/161 | — | nicht portiert (hängen an `loop/HammerSaite`, `loop/Instrument`, QString: `LVU\Modellton.hpp:26–32`, `LVU\Suche.hpp:27`) |
| neu in IC | — | `Bausteine.hpp/.cpp` (660), `Samples.hpp/.cpp` (121) | | | Modalfit, Teiltonverfolgung, YIN, Vibrato, Rest, Einschwingen, Bänder (`IC\messung\Bausteine.hpp:37–144`); Zugang zu Aufnahmen über `IC_SAMPLES` |

Wertedateien: `hammer_bezug_clang.txt`, `hammer_bezug_saite.json`, `salamander_ziel.txt`, `salamander_struktur.txt` beidseitig gleich; `hoerschwellen.txt` gleich bis auf eine Kopfzeile in IC.

Nutzer LV: `test_Teiltonmessung`, `test_Kurvenabstand`, `test_LoopHammerTon`, 8 Dateien `test_LoopSalamander*`. Nutzer IC: `messung/Bausteine.cpp` (Teiltonmessung), `messung/test_messung.cpp`, `setzen/Setzen.cpp` (LM), `werkzeuge/messe.cpp`.

### 2.6 `Json`

| | LumiViz | IC |
|---|---|---|
| Ort | Qt (`QJsonObject` usw.) in jedem Dateimodul (`HammerSaite.cpp`, `Instrument.cpp`, `Koerper.cpp` …) | eigener Leser/Schreiber `IC\gemeinsam\Json.hpp` (54) / `.cpp` (333): `JsonWert`, `leseJson`, `leseJsonDatei`, `alsJson` (`:19–52`) |
| Nutzer | überall | nur `klassen/01_hammer_saite/HammerSaite.hpp` (Bezugsdatei) und `werkzeuge/Sammlung.cpp` (Hörproben-Listen) |

Keine Code-Dublette, aber zwei Wege für dasselbe Format (`hammerString`-Block). **Die Klassen 2–11 haben keine Datei-Fassung** (kein `…AusJson/…AlsJson` außerhalb Klasse 1).

### 2.7 Setz-Werkzeug: `setzen/` gegen die Suche in den LV-Testhilfen

| | LumiViz | IC |
|---|---|---|
| Ort | `LVU\Suche.hpp` (449), `LVU\Suchkurven.hpp` (390), `LVU\Modellton.hpp` (192, mit Pool `parallelJe` `:55`), Teile von `LVU\LevenbergMarquardt.hpp` (`levenbergMarquardtJeBlock`), `LVU\Probe7512.hpp` | `IC\setzen\Setzen.hpp` (181), `IC\setzen\Setzen.cpp` (649), dazu `IC\messung\Probe.hpp` (239) |
| Bindung | an den Flügel: `Suchraum` über Kurven der `HammerSaite`, Ton über `rendereTon` (`LVU\Suche.hpp:27`, `Modellton.hpp:26–30`) | **modell-agnostisch**: `using Modell = std::function<std::vector<double>(std::size_t gruppe, const std::vector<double>& werte)>` (`IC\setzen\Setzen.hpp:75`), `Parameter`, `Messgroesse`, `Gruppe` (`:46–74`) |
| Verfahren | LM gemeinsam oder je Block (S165), Dateien je Iteration, Wiederaufnahme | LM mit Grenzen je Gruppe (L8), Wirkung vorab (L9), verrauschte Spalten (L11), 1D zuerst (L12/L16), `welligkeit`, `glaette`, `setzeMehrfach`, `eindeutig`, `probe` (`Setzen.hpp:3–24`, `:119–178`) |
| Nutzer | `test_LoopSalamanderSuche`, `test_LoopSalamanderHoerproben` | `werkzeuge/glaette_ziel.cpp`, `setzen/test_setzen.cpp` |

Keine Kopie, sondern ein zweiter Bau mit überlappender Aufgabe (LM je Gruppe ≈ `levenbergMarquardtJeBlock`). IC baut auf der älteren `LevenbergMarquardt.hpp` (ohne JeBlock) auf (`IC\setzen\Setzen.cpp` inkludiert `messung/LevenbergMarquardt.hpp`).

### 2.8 Weitere Überschneidungen (keine Kopie, gleiche Aufgabe)

| Baustein IC | Gegenstück LV | Befund |
|---|---|---|
| `IC\gemeinsam\Modal.hpp/.cpp` (387): Moden als Pole, Impulsinvarianz, `Stossanreger` nach Stulov/Hertz/Hunt–Crossley | Modalkern und Hammer **innerhalb** `LV\src\loop\HammerTon.cpp` | Verallgemeinerung „nach dem Vorbild HammerTon.cpp“ (`Modal.hpp:6–24`); der Flügel-Port in IC nutzt `Modal` **nicht** (er bleibt wortgleich), Klassen 1-Saite, 4, 5 nutzen es |
| `IC\gemeinsam\Koerper.hpp/.cpp` (309): `ModenKoerper`, `mobilitaet`, `koerperModen`, `koerperAntwort`, **Luftraum** | `LVB\Koerper.hpp` (Daten des gerechneten Körpers), `LV\include\loop\Koerper.hpp` (nur Datei-IO, Qt), Gitarrenkorpus `korpusImpulsantwort` (`LV\include\loop\Instrument.hpp:146`) | Gleicher Dateiname, anderer Inhalt. Der Luftraum ist ein Neubau des Gitarrenkorpus nach Wright „ohne Trapezregel wie in LumiViz“ (`IC\gemeinsam\Koerper.hpp:13–22`). **In IC von keiner Klasse eingebunden** (nur Koerper.cpp und Test) |
| `IC\gemeinsam\Werkzeug.hpp/.cpp` (537): WAV, `lautheit` (BS.1770-4), SHA-256-`abdruck`, `parallelJe` | `LV\include\audio\WavFormat.hpp`, `WavDatei.hpp`, `LV\include\audio\Lautheit.hpp/.cpp` (272), Pool in `LVU\Modellton.hpp:55`, Abdruck `klangfassung.txt` / QCryptographicHash | Dritte Fassung von Pool und Lautheit; K-Filter laut Kopf „aus dem Gedächtnis, offen“ (`Werkzeug.hpp:11–14`) |
| `stufeAus` (`IC\klassen\04_platte_stab\Stab.cpp:116–120`) | `anschlagStufe` (`LV\src\loop\HammerSaite.cpp:511–517`) | gleiche Formel, −40 … 0 fest statt `kAnschlagStufeMin`/`kAnschlagSchrittDb` |

---

## 3. Die Schnittstelle der Kerne

### 3.1 IC: alle Ton-Funktionen

Alle haben die Form `void rechne…(const <X>Spiel&, const <X>Anschlag&, std::vector<float>& mono, std::int64_t frames, <X>Messung* = nullptr)`; Ausgabe **immer Mono float**; die Rate steht im Anschlag (`int rate = 48000`).

| Klasse | Funktion (Datei:Zeile) | Spiel | Stärke | Tonhöhe | Halten | `bereite`/`stimme` | Messung* | Sonst |
|---|---|---|---|---|---|---|---|---|
| 1 Flügel | `rechneHammerTon` (`IC\klassen\01_hammer_saite\HammerTon.hpp:135`) | `HammerSaite` + Tastatur außerhalb | `stufe` int ≤ 0 (+ `tempoFaktor`) | `taste` + `hz` | `halten` Frames, `gedaempft` | — | `HammerMessung` | `moderator*` |
| 1 Rest | `rechneSaitenTon` (`Saite.hpp:144`), `rechneSaitenClip` (`:155`) | `SaitenSpiel` (`ic::saite`) | `stufe` | `note` + `hz` | `halten` Frames | — | `SaitenMessung` / Clip ohne | Clip: `SaitenClipNote{abS, anschlag}`; `mitschwingen`, `schieber`, `abnehmerWahl` |
| 2 | `rechneZupfTon` (`Zupf.hpp:111`), `rechneZupfClip` (`:105`, Rate als eigener Parameter) | `ZupfSpiel` | `staerke` 0…1 | `note` | `haltenS` s | `stimme` in der Vorgabe | `ZupfMessung` / Clip ohne | `ClipNote{abS, anschlag}`, `mitschwingen` |
| 3 | `rechneStreichTon` (`Bogen.hpp:125`), `rechneEnsembleTon` (`:136`, mit `stimmen`) | `StreichSpiel` | `expression` 0…1 | **Kette** `KettenNote{abS, note, hz}` | `haltenS` | `stimme` | `StreichMessung` / Ensemble ohne | Vibrato, Pitch, Art, Lage |
| 4 | `rechneStabTon` (`Stab.hpp:236`) | `StabSpiel` | `stufe` (gestrichen: `expression`) | `note` + `hz` | `daempferAb` Frames, `andruck`; gestrichen `haltenS` | `stimmeBogen` | `StabMessung` | Motor-Spur, `stelleOffset` |
| 5 | `rechneTrommelTon` (`Trommel.hpp:95`) | `TrommelSpiel` (mit `shared_ptr` auf gerechnete Moden, `:72`) | `staerke` 0…1 | `note` (nur Pauke) | — (`wirbelS` s) | **`bereite(TrommelSpiel&)`** (`:94`) | `TrommelMessung` | `Schlag`, `glissandoCent`, `ohneTeppich` |
| 6 | `rechneBeckenTon` (`Becken.hpp:111`, dazu `ohneDaempfung`) | `BeckenSpiel` | `staerke` | — | `chokeAbS` s | **`bereite`** (`:110`) | `BeckenMessung` | `Stelle`, `oeffnung` |
| 7 | `rechneBlechTon` (`Blech.hpp:158`) | `BlechSpiel` | `expression` | Kette | `haltenS` | **`bereite`** (`:172`), `stimme(s,1)` | `BlechMessung` | Ende, Growl, Vokal, Dämpfer, Wah |
| 8 | `rechneRohrblattTon` (`Rohrblatt.hpp:134`) | `RohrblattSpiel` | `expression` | Kette | `haltenS` | `stimme` | `RohrblattMessung` | Ansatz, Flatter |
| 9 | `rechneStrahlTon` (`Luftstrahl.hpp:115`) | `StrahlSpiel` | `expression` | Kette | `haltenS` | `stimme` | `StrahlMessung` | Schneidenton |
| 10 | `rechneZungenTon` (`Zunge.hpp:91`), `rechneEineZunge` (`:94`, Ausgabe `double`) | `ZungenSpiel` | `expression` (oder `druckPa`) | `note` | `haltenS` | `stimme` | `ZungenMessung` | `pitchCent`, Vibrato, `balgwechselS` |
| 11 | `rechneOrgel` (`Orgel.hpp:89`) | `OrgelSpiel` | Schweller in `OrgelSpur` | `OrgelNote{abS, bisS, midi, tempo}` | `bisS` | — | `OrgelMessung` | **Clip**: Noten + Spur (Rotary, Register), Rate als Parameter |

16 Funktionen in 11 Klassen, 12 `Spiel`-Typen, 63 Vorgaben `baueVorgabe…()` in den Köpfen (zusätzlich der Flügel über `hammerSaiteVorgabe()`).

### 3.2 LumiViz: das Gegenstück

| Stelle | Inhalt |
|---|---|
| `LV\include\loop\Loop.hpp:265–308` `TonAngabe` | `QString instrument`, `hoehe`, `dauerMs` (int ms), `saite`, `endeMs`, `richtung`, `artikulationen`, `teilton`, **`verlauf` (TonWechsel{ms, hoehe, gleitet})**, `biegung`, `teil`, **`tonhoehe`/`vibrato` (TonPunkt{ms, cent})**, **`ausdruckMuster`/`ausdruckClip` (PegelPunkt{ms, tausendstel}, Kurven über die Zeit)**, `anschlagStufe` int ≤ 0 |
| `LV\include\loop\Loop.hpp:309–320` `tonKlang`/`zerlegeTon` | die Angabe als Name (`:a<stufe>`, `:x…`, `:v…` …) — der Name ist der Schlüssel des Tonspeichers |
| `LV\include\loop\Instrument.hpp:772–779` `rendereTon` | `Klang rendereTon(const Instrument&, const TonAngabe&, int rate)` → `Klang{std::vector<float> daten}` **Stereo verschränkt** (`LV\include\loop\Rechenkern.hpp:52–56`) |
| `LV\src\loop\Instrument.cpp:2909–2927`, `:2993–3005` | Bau von `HammerAnschlag` aus Schicht und Angabe: `taste = ton`, `hz = tonHz(ton)`, `stufe = angabe->anschlagStufe`, `halten = huelle.ende` (Frames), `gedaempft` aus `Tastatur::ungedaempftAb`; Ausgang Mono auf beide Seiten oder gefaltet mit dem Boden (`falteMitBoden`) |
| `LV\include\loop\HammerTon.hpp:44–52`, `:128–130` | `HammerAnschlag{taste, hz, stufe, halten, gedaempft, rate}`; `rechneHammerTon(saite, anschlag, mono, frames, HammerMessung*)` |
| `LV\include\loop\Auskunft.hpp:26–45` | `InstrumentAuskunft{Saitensatz saiten; bool hammer}`, `InstrumentVon = std::function<InstrumentAuskunft(const QString&)>` — die Planung weiß nur „Saiten“ und „Hammer ja/nein“ |
| `LV\src\loop\Rechenkern.cpp:261` | Lautstärke der Note: mit Hammer `anschlagRest(note.laut, stufe)`, sonst `note.laut` — Stufe in den Namen, Rest als Pegel |
| `LV\include\loop\Tonspeicher.hpp:13–16`, `:104` | Schlüssel = Tonname + Rate + `instrumentAbdruck` |

### 3.3 Gleich und verschieden

| Achse | Gleich | Verschieden |
|---|---|---|
| Form des Kerns | `rechne…(Spiel, Anschlag, mono, frames, Messung*)` überall wie `rechneHammerTon` | Clip-Kerne (`rechneSaitenClip`, `rechneZupfClip`, `rechneOrgel`) nehmen viele Noten — `rendereTon` kennt nur einen Ton je Name |
| Ausgabe | float, vorab gerechnet | IC immer Mono; LV liefert Stereo (Mono dupliziert oder über Boden gefaltet) |
| Stärke | `stufe` int ≤ 0 in 1, 1-Rest, 4 ↔ `TonAngabe::anschlagStufe` | `staerke` 0…1 (2, 5, 6) und `expression` 0…1 **als Skalar** (3, 7, 8, 9, 10, Stab gestrichen) ↔ in LV Expression als **Kurve über die Zeit** (`ausdruckMuster`, `ausdruckClip`) und Notenlaut `Note::laut` |
| Tonhöhe | `note`/`taste` MIDI + `hz` | Kette `KettenNote{abS s, note, hz}` (4 gleiche Definitionen: `Bogen.hpp:94`, `Blech.hpp:111`, `Rohrblatt.hpp:95`, `Luftstrahl.hpp:89`) ↔ `TonWechsel{ms int, hoehe, gleitet}` |
| Zeit | Frames bei 1, 1-Rest, 4 (`halten`, `daempferAb`) | Sekunden `double` bei 2, 3, 7–10 (`haltenS`), `abS` in Clips; LV: `dauerMs`/`endeMs` int ms, Kurvenpunkte in ms |
| Spuren | Vibrato/Pitch in Cent | IC je Ton ein Wert (`vibratoCent`, `vibratoHz`, `pitchCent`); LV Kurven (`tonhoehe`, `vibrato` als Punktlisten) |
| Vorbereitung | — | `bereite(Spiel&)` mutiert das Spiel (5, 6, 7), `stimme(Spiel&)` sucht am Modell (2, 3, 4, 7–10) — LV hat kein Gegenstück; dort kommt ein Instrument fertig aus der `.lvinst` |
| Rate | `int rate` im Anschlag | `rechneZupfClip`, `rechneOrgel` tragen die Rate zusätzlich als Parameter |
| Planung | — | `InstrumentAuskunft` kennt nur `hammer`; welche Klasse Stufe, Kette, Clip braucht, steht nirgends als Auskunft |
| Pegel | — | IC: `ausgangBezug` so, dass ein Ton der Vorgabe Spitze 0,5 hat (`normiere` in 10 `Vorgaben.cpp`, z. B. `IC\klassen\05_membran\Vorgaben.cpp:25–40`); LV: `kHammerKraftBezug = 200 N`, `kSchalldruckBezug = 97,3 Pa` (`LV\include\loop\HammerTon.hpp:134–139`) und Pegel je Instrument nach LUFS (`asset/loop/instrument_pegel.py`) |

---

## 4. Kopplung und Richtung

### 4.1 IC: was nach LumiViz nicht passt

| Kopplung | Ort | Wirkung bei Anbindung |
|---|---|---|
| GLOB statt expliziter Liste | `IC\CMakeLists.txt:51`, `:57`, `:69` | LV verlangt `sources.mode: explicit`; ein neuer Kern in IC erscheint in LV nicht von selbst |
| Gleitkomma-Schalter global | `IC\CMakeLists.txt:19–23` (`/fp:precise /fp:contract-`, `-ffp-contract=off -fno-fast-math`) | CMakeCraft setzt keine (`CompilerOptions.cmake:144–200`); die Abdrücke `IC\werte\abdruck_*.txt` gelten nur unter IC-Schaltern |
| `IC_WERTE` | `IC\CMakeLists.txt:73` — nur am Testziel | unkritisch für Kerne |
| `getenv("IC_SAMPLES")` in der Kern-Bibliothek | `IC\messung\Samples.cpp:36`, gebaut in `ic_gemeinsam` (`CMakeLists.txt:43`) | wer `ic_gemeinsam` linkt, linkt Dateizugang zu Aufnahmen mit |
| `IC_ABDRUCK_SCHREIBEN`, `IC_BERICHT` | `IC\werkzeuge\TestHilfe.hpp:53`, `test_hammer_bezug.cpp:111` | nur Tests |
| Messcode in den Kernen | `messung/Bausteine.hpp` in `Zupf.cpp:8`, `Bogen.cpp:9`, `Stab.cpp:10`, `Blech.cpp:10`, `Rohrblatt.cpp:8`, `Luftstrahl.cpp:8`, `Zunge.cpp:8`; z. B. YIN für `StabMessung` (`Stab.cpp:489`), `ZungenMessung` (`Zunge.cpp:109`, `:121`) | `Bausteine.cpp` zieht `Teiltonmessung.hpp` (1163 Zeilen) und `Eigenzerlegung` mit; der Kern hängt an der Messkette |
| Klasse 4 → Klasse 1 | `klassen/04_platte_stab/Vorgaben.cpp:20`, `:262–266`, `:386–389` | wer Klasse 4 bindet, bindet den Flügel-Port (HammerSaite, Kurve, Json) mit |
| `__has_include` mit stillem Ersatz | `klassen/04_platte_stab/Vorgaben.cpp:29–31`, `:211–214`; gleiches Muster `IC_*_STIMMUNG` in 3, 7, 8, 9, 10 | fehlt `Stimmung.inc`, rechnet die Vorgabe beim Bau `stimmeBogen` (Suche am Modell auf dem Pool) — ohne Meldung |
| Vorgaben rechnen Töne | `normiere` je Klasse (z. B. `klassen/02_zupf_saite/Vorgaben.cpp:54–66` mit `stimme(s)`, `05_membran/Vorgaben.cpp:25–40`) | eine Vorgabe zu bauen kostet Rechenzeit (statische Initialisierung, `static const … vorgabe = baueVorgabe()` in `HammerSaite.cpp:167`) |
| Pool im Ton | `rechneEnsembleTon` → `parallelJe` (`klassen/03_bogen_saite/Bogen.cpp:401`) | verschachtelt im Pool des `Tonauftrag` (Kerne − 1) |
| pocketfft ohne `NO_MULTITHREADING`, andere Fassung | `IC\externals\pocketfft\pocketfft_hdronly.h` (3888 Zeilen) gegen LV (3853), Schalter nur in LV (`externals\pocketfft\Include.cmake:35`) | beide Header in einem Binärprogramm = zwei Fassungen derselben `inline`-Vorlagen im Namensraum `pocketfft` (ODR) |
| Zeilenenden | IC CRLF (`file …\HammerTon.cpp`), LV LF | Diffs und Übernahmen nur mit `--strip-trailing-cr` lesbar |
| Globaler Zustand | nur Magic Statics (`HammerSaite.cpp:167`, `Stab.cpp:39`), kein `mutable`/`thread_local`/Mutex in Kernen | von mehreren Threads rufbar |
| Qt / `windows.h` | keins (`IC\CMakeLists.txt:1`, geprüft per grep) | passt |

### 4.2 LumiViz: wo der Kern an Qt hängt

| Datei | Includes | Befund |
|---|---|---|
| `HammerTon.hpp` | `loop/HammerSaite.hpp` (`:27`) | Qt **über den Kopf** |
| `HammerTon.cpp` | `loop/Eigenzerlegung.hpp`, Standard | Qt-frei |
| `HammerSaite.hpp` | `QJsonObject`, `QJsonValue`, `QLatin1String`, `QString` (`:25–28`) | Daten + Datei in einem Kopf |
| `HammerSaite.cpp` | `QJsonArray` (`:8`) | |
| `Eigenzerlegung.hpp/.cpp` | nur Standard | Qt-frei |
| `Fft.hpp/.cpp` | Standard, `pocketfft_hdronly.h` | Qt-frei |
| `Koerper.hpp/.cpp` | `InstrumentBody/*`, `QJsonObject`, `QString`, `QCryptographicHash`, `QDir`, `QFile`, `QSaveFile` … | reine Datei-Schicht, Qt |
| `Boden.hpp` | `InstrumentBody/Koerper.hpp`, `loop/Fft.hpp`, **`loop/Rechenkern.hpp`**, `QString` (`:14–18`) | Faltung mit dem Boden hängt über `Rechenkern.hpp` → `Loop.hpp` an Planung und Qt |
| `Tastenkurve` | in `HammerSaite.hpp` | kein eigenes Modul |
| `windows.h` | in keiner dieser Dateien | — |

Weitere Kopplung im Klangweg: `Instrument.hpp:431` (`HammerSaite hammerSaite` als festes Feld jeder `Schicht`), `Instrument.hpp:544` (`std::optional<Tastatur>`), `Rechenkern.cpp:8` (`HammerSaite.hpp` für `anschlagRest`), Dispatch als `if`-Kette über `istHammerSchicht`/`schwingung` (`Instrument.cpp:2914–3010`).

**Zyklen:** keine Header-Zyklen auf beiden Seiten gefunden.

---

## 5. Befunde

1. **[Hoch] Zwei Fassungen desselben Flügel-Kerns ohne Bitgleich-Beleg** — `LV\src\loop\HammerTon.cpp` gegen `IC\klassen\01_hammer_saite\HammerTon.cpp` (−7/+25), `HammerSaite.cpp` (−287/+95); `IC …\test_hammer_bezug.cpp:9–14` prüft die Prüfsumme nicht — Zwei Rechenkerne für dasselbe Instrument laufen auseinander, sobald eine Seite weiterbaut; IC misst zudem die Rechenzeitgrenze aller Klassen relativ zu seinem Port (`werkzeuge/TestHilfe.hpp`), der Port ist damit Infrastruktur, nicht nur Prüfstein. Der Unterschied ist fachlich klein und additiv (Soft, Moderator, Bass sustain, Upright/Honky-tonk/CP-70) — **Refactor-Richtung:** ein Kern mit einer Quelle; die IC-Erweiterungen als Felder mit neutralen Vorgaben wieder an *eine* Fassung; der Bitgleich-Wächter gegen `hammer_bezug_<compiler>.txt` mit Prüfsumme auf der Seite, die nicht SSOT ist. Trade-off: solange IC in der Cloud ohne LumiViz baut, braucht es dort *eine lesbare* Fassung — die Frage ist nur, welche Seite schreibt.

2. **[Hoch] Der Kopf des Flügel-Kerns zieht Qt nach** — `LV\include\loop\HammerTon.hpp:27` → `LV\include\loop\HammerSaite.hpp:25–28`; `Boden.hpp:16` → `Rechenkern.hpp` → `Loop.hpp` — Der Kern ist in den `.cpp` Qt-frei, aber kein Qt-freier Verbraucher (IC, ein späteres Plugin, `InstrumentBody`) kann `HammerTon.hpp` einbinden. Genau diese Trennung hat IC beim Port nachgezogen (Datei raus, `ic::Kurve`, `std::string`). — **Refactor-Richtung:** Daten (Kurven, Saite, Tastatur, Anschlag) und Datei (JSON, Namen der Pedale) in getrennte Köpfe; die Faltung mit dem Boden ohne `Rechenkern.hpp` (sie braucht nur `Klang`/Spans). Trade-off: ein Kopf mehr je Modul; dafür fällt der Grund für den IC-Port der Datenstruktur weg.

3. **[Hoch] Der Rechenkern hat in LumiViz keine eigene Bibliothek** — `LV\src\loop\Source.cmake:29–40` (Kern im App-Core neben UI), `Solution.json:150–190` (nur INTERFACE-Bibliotheken) gegen `IC\CMakeLists.txt:28–65` (drei Bibliotheken) — Eine Anbindung von IC-Klassen hat in LumiViz keinen Ort außer dem App-Core; ein Plugin-Ziel (CLAP/VST3, beide Richtungen) bräuchte den Kern ohne Qt und ohne App. CMakeCraft kann STATIC-Bibliotheken mit Quellen (`LibraryCreate.cmake:77–111`). — **Refactor-Richtung:** eine STATIC-Bibliothek „Rechenkerne“ unter `projects/libs/` mit expliziter `Source.cmake`, Externals pocketfft (und später Eigen nur für Werkzeuge); App-Core und Tests hängen daran. Trade-off: CMakeCraft-Bibliothek mit Quellen ist in LumiViz neu (Erstfall, mögliche Build-Lücke gehört nach CMakeCraft); dafür ist die Abhängigkeitsrichtung App → Kern erzwungen.

4. **[Hoch] Die Schnittstelle der Kerne ist pro Klasse eigen, `rendereTon` kennt sie nicht** — §3.3; `LV\include\loop\Auskunft.hpp:31` (`bool hammer`), `LV\src\loop\Instrument.cpp:2914–3010` (if-Kette), `LV\include\loop\Instrument.hpp:431` (Feld `hammerSaite` an jeder Schicht) — Jede angebundene Klasse vergrößert `Schicht`, die Dispatch-Kette, `InstrumentAuskunft` und das Namensschema `tonKlang`. Die IC-Kerne sind untereinander formgleich (`rechne…(Spiel, Anschlag, mono, frames, Messung*)`), unterscheiden sich aber in Stärke (Stufe / Stärke / Expression-Skalar), Zeit (Frames / Sekunden), Tonhöhe (Note / Kette) und Vorbereitung (`bereite`, `stimme`); LV führt Expression, Pitch, Vibrato als Kurven in ms. — **Refactor-Richtung:** eine schmale, Qt-freie Ton-Schnittstelle zwischen Klangweg und Kern (ein Anschlag mit Note/Kette, Stärke, Haltezeit, Spuren als Kurven, Rate; Ausgabe Mono oder Kanäle; dazu eine Auskunft je Kern: braucht Stufe im Namen, ist Clip-Kern, Nachklang), daneben die Daten je Klasse als variantes „Spiel“ statt fester Felder je Schicht. Trade-off: die Kurven von LV in IC-Skalare zu falten verliert Information (Expression über die Zeit) — die Schnittstelle muss die reichere Seite tragen, damit Klassen 3/7–10 später keine zweite brauchen. Plugins: ein Kern, der vorab rechnet (Mono-Puffer), passt zu Freeze/Tonspeicher, aber nicht zu Echtzeit-Steuerung; Ketten- und Clip-Kerne (2, 3, 11) zeigen, dass die Schnittstelle Notenfolgen tragen muss.

5. **[Mittel] Messcode in den Kernen und in der „gemeinsam“-Bibliothek** — `IC\CMakeLists.txt:41–43` (`messung/Bausteine.cpp`, `messung/Samples.cpp` in `ic_gemeinsam`), Includes in 7 Kernen (§4.1) — Die Schicht „Kern“ hängt an der Schicht „Messung“ (YIN, Einschwingen für `…Messung*`, Stimmen am Modell); `Samples.cpp` bringt `getenv("IC_SAMPLES")` in jede Bindung. Für Klasse 4 heißt das: Teiltonmessung (1163 Zeilen) und Eigenzerlegung kommen mit. — **Refactor-Richtung:** die Messung im Ton (`Messung*`) und das Stimmen (`stimme`) als eigene Schicht über dem Kern, oder die wenigen Bausteine (YIN) unter `gemeinsam/`; `Samples` nur in Werkzeuge. Trade-off: `Messung*` im Kern spart einen zweiten Rechenweg für Tests; die Trennung kostet eine Rückgabe der Rohsignale.

6. **[Mittel] Abhängigkeit Klasse 4 → Klasse 1** — `IC\klassen\04_platte_stab\Vorgaben.cpp:20`, `:262–266`, `:386–389` (fünf Stulov-Kurvenfunktionen) — Die zuerst geplante Anbindung (Klasse 4) zieht den Flügel-Port und damit Befund 1 mit. — **Refactor-Richtung:** die Hammer-Kennlinien (Stulov-Tabellen) als gemeinsamer Datenbaustein unterhalb beider Klassen. Trade-off: gering.

7. **[Mittel] Zwei pocketfft-Fassungen mit verschiedenen Schaltern** — `C:\…\LumiViz\externals\pocketfft\pocketfft_hdronly.h` (SHA-256 `3e9a0531…`, `POCKETFFT_NO_MULTITHREADING` in `Include.cmake:35`) gegen `IC\externals\pocketfft\pocketfft_hdronly.h` (`bc0095b2…`, ohne Schalter) — In einem Binärprogramm verletzen zwei Fassungen derselben header-only Vorlagen die ODR; ohne `NO_MULTITHREADING` kompiliert IC den eigenen Pool von pocketfft mit. — **Refactor-Richtung:** ein External, ein Schalter, die Fassung im README festgehalten (wie in LV). Trade-off: keiner.

8. **[Mittel] Gleitkomma-Regeln nur in IC** — `IC\CMakeLists.txt:19–23` gegen CMakeCraft `CompilerOptions.cmake` — Die Bitgleich-Wächter beider Seiten (IC `abdruck_*.txt`, LV `loop_werte.txt`/`klangfassung.txt`/`hammer_bezug_*`) gelten je unter anderen Schaltern; ein in LV gebauter IC-Kern kann andere Bits liefern als in IC. — **Refactor-Richtung:** die Schalter als Eigenschaft der Kern-Bibliothek (CMakeCraft: `compile_options` je Bibliothek, falls nicht vorhanden als CMakeCraft-Lücke melden), damit beide Builds dieselbe Regel haben. Trade-off: `-ffp-contract=off` kann den Flügel in LV messbar langsamer machen — vor dem Wechsel `*31b-3 Rechenzeit*` fahren.

9. **[Mittel] Messkette und Suche dreifach (LV, Vorbild-Kopie, IC) mit auseinanderlaufendem Stand** — §2.5, `LVU\LevenbergMarquardt.hpp:376–451` (nur LV), `IC\setzen\Setzen.hpp:46–178` (nur IC) — Die Messkette ist mechanisch portiert (nur Namensraum), aber LV und IC haben danach unabhängig weitergebaut: LV `levenbergMarquardtJeBlock`, IC ein modell-agnostisches `setzen` mit L8–L18. Die LV-Kette liegt als Testhilfe in `tests/unit/UnitTests/`, ist an `loop/HammerSaite`/`loop/Instrument` gebunden (`LVU\Suche.hpp:27`, `Modellton.hpp:26–30`) und nicht als Bibliothek nutzbar. — **Refactor-Richtung:** eine Messbibliothek (Qt-frei, gegen `Fft`) mit einem Setz-Werkzeug gegen eine Modell-Funktion (die Form von `IC\setzen\Setzen.hpp:75` ist dafür geeignet); der Flügel liefert seine Modell-Funktion. Trade-off: die Suche des Flügels ist in LV gerade aktiv (Plan A–D) — ein Umzug mitten im Lauf gefährdet laufende Proben.

10. **[Niedrig] Muster-Dubletten innerhalb IC** — `KettenNote` viermal (`Bogen.hpp:94`, `Blech.hpp:111`, `Rohrblatt.hpp:95`, `Luftstrahl.hpp:89`), `normiere` in zehn `Vorgaben.cpp` (Spitze 0,5), `stufeAus` (`Stab.cpp:116`) = `anschlagStufe` — Bei einer gemeinsamen Schnittstelle (Befund 4) werden diese zu Abweichungen je Klasse. — **Refactor-Richtung:** Kette, Stufe und Pegelbezug einmal unter `gemeinsam/`, und *ein* Pegelbezug für beide Seiten (heute Spitze 0,5 in IC gegen Kraft/Schalldruck-Bezug plus LUFS in LV). Trade-off: gering.

11. **[Niedrig] Stiller Ersatz bei fehlender `Stimmung.inc`** — `klassen/04_platte_stab/Vorgaben.cpp:29–31`, `:211–214` (gleiches Muster in 3, 7, 8, 9, 10) — Fehlt die Datei in einer Kopie, wird die Vorgabe beim ersten Gebrauch teuer gesucht, ohne Meldung; mit expliziten Quelllisten in LV fällt das nicht auf. — **Refactor-Richtung:** fehlende Stimmung als Bau- oder Testfehler; Stimmung als Daten (Datei der Vorgabe) statt Code. Trade-off: keiner.

12. **[Niedrig] Bausteine ohne Nutzer in IC** — `IC\gemeinsam\Koerper.hpp/.cpp` (309; Luftraum), `IC\messung\NelderMead.hpp`, `IC\messung\Kurvenabstand.hpp` — von keiner Klasse/keinem Werkzeug eingebunden; `Koerper` trägt denselben Dateinamen wie zwei andere Dinge in LV (`LV\include\loop\Koerper.hpp` = Datei-IO, `LVB\Koerper.hpp` = gerechneter Körper). — **Refactor-Richtung:** bei der Anbindung entscheiden, ob Luftraum und Gitarrenkorpus (`korpusImpulsantwort`) ein Baustein werden; Namen eindeutig.

---

## 6. Zahlen für die Vorlage

### 6.1 IC `rechnung/` (Zeilen `wc -l`, `.cpp` + `.hpp` + `.inc`)

| Teil | gesamt | davon Tests | ohne Tests |
|---|---:|---:|---:|
| `gemeinsam/` | 4 550 | 374 | 4 176 |
| `messung/` | 5 259 | 462 | 4 797 |
| `setzen/` | 1 118 | 288 | 830 |
| `werkzeuge/` | 2 178 | 145 | 2 033 |
| `klassen/` | 12 926 | 2 843 | 10 083 |
| **Summe** | **26 031** | **4 112** | **21 919** |

Je Klasse (gesamt / Tests / ohne Tests; `.inc` in „gesamt“):

| Klasse | gesamt | Tests | ohne Tests | `.inc` |
|---|---:|---:|---:|---:|
| 01 Hammer-Saite | 2 977 | 550 | 2 427 | 0 |
| 02 Zupf-Saite | 1 203 | 252 | 951 | 0 |
| 03 Bogen-Saite | 1 082 | 291 | 791 | 33 |
| 04 Platte/Stab | 1 655 | 327 | 1 328 | 10 |
| 05 Membran | 788 | 240 | 548 | 0 |
| 06 Becken/Gong | 766 | 143 | 623 | 0 |
| 07 Lippe/Rohr | 1 325 | 304 | 1 021 | 26 |
| 08 Rohrblatt/Rohr | 947 | 197 | 750 | 25 |
| 09 Luftstrahl/Rohr | 836 | 174 | 662 | 21 |
| 10 Zunge | 693 | 184 | 509 | 25 |
| 11 Tonrad | 654 | 181 | 473 | 0 |

- **Klassen-Kerne:** 11 Klassen, 12 `Spiel`-Typen, 16 `rechne…`-Funktionen (12 je Ton, 3 je Clip, 1 Hilfsfunktion `rechneEineZunge`), 63 Vorgaben `baueVorgabe…()`.
- **Werkzeuge:** 10 Ziele `add_executable` (`IC\CMakeLists.txt:70–102`): `ic_tests` + 9 Werkzeuge (`ic_stimme_blech`, `_rohrblatt`, `_luftstrahl`, `_bogen`, `_zunge`, `_stab`, `ic_messe`, `ic_glaette_ziel`, `ic_hoerproben`).
- **Tests:** 125 `TEST_CASE` (gemeinsam 13, messung 13, setzen 10, werkzeuge 2, klassen 87).
- **Abhängigkeitshülle** (ohne Tests, ohne Externals): Klasse 4 = Stab 1 328 + Balken 352 + Reibung 121 + Werkzeug 537 + Bausteine 660 + Teiltonmessung 1 163 + Fft 174 + Eigenzerlegung 417 + HammerSaite 647 + Kurve 246 + Json 387 + Modal 387 = **6 419**; Klasse 5 = Trommel 548 + Membran 493 + Balken 352 + Modal 387 + Werkzeug 537 = **2 317**.

### 6.2 Dubletten je Seite (Zeilen)

| Baustein | IC | LumiViz |
|---|---:|---:|
| Eigenzerlegung | 417 | 383 |
| Fft | 174 | 133 |
| Kurve / Tastenkurve | 246 | ≈ 150 (in HammerSaite, unten mitgezählt) |
| HammerTon | 781 | 755 |
| HammerSaite | 647 | 839 |
| Messkette (Teiltonmessung, Strukturmessung, Zielsumme, LM, NelderMead, Kurvenabstand, Fluegelmessung, Hoerschwellen, Probe) | 4 016 | 4 306 |
| **Summe Code-Dubletten** | **6 281** | **6 416** |
| Externals doppelt | pocketfft 3 888, doctest | pocketfft 3 853, doctest |
| funktionale Überschneidung (kein Kopie): Json 387, Werkzeug 537, Modal 387, Koerper 309, Setzen 830 | 2 450 | (Qt-JSON, audio/Lautheit 272, Pool in Modellton, Korpus in Instrument.cpp, Suche/Suchkurven 839) |

### 6.3 Was in `rechnung/` ohne Dubletten übrig bleibt

- Ohne Tests: 21 919 − 6 281 = **15 638 Zeilen** eigenständiger Code (Kerne 2–11, Klasse-1-Rest `Saite`/`SaitenVorgaben`/`VorgabenRest`, gemeinsame Bausteine Modal/Balken/Membran/Rohr/Rohrschleife/Reibung/Koerper/Json/Werkzeug, Messbausteine, Setzen, Werkzeuge).
- Davon funktional überschneidend mit LV (§6.2 letzte Zeile): 2 450 Zeilen.
- Kerne der Klassen 2–11 allein (ohne Tests): 7 656 Zeilen; Klasse 1 ohne den Port (Saite 695 + SaitenVorgaben 183 + VorgabenRest 121): 999 Zeilen.

### 6.4 LumiViz zum Vergleich

- `include/loop` + `src/loop`: 57 317 Zeilen; davon Flügel-Kern und Körper-Anbindung (HammerTon, HammerSaite, Eigenzerlegung, Fft, Boden, Koerper, KoerperBauplan): 3 035.
- `projects/libs/InstrumentBody` (Köpfe): 3 066 Zeilen (ohne `.md`).
- Messkette und Suche als Testhilfen (`LVU`, 15 Köpfe): 6 194 Zeilen; Tests des Flügels und der Messung (Salamander, Teiltonmessung, Kurvenabstand, Hammer, Koerper, KleinEigen, Fft): 13 656 Zeilen.

# Übernahme-Journal — Bausteine aus IC und LumiViz in die Bibliotheken

> **Version:** 0.10.0
> **Datum:** 2026-11-09 (Sessions 175–181)
> **Typ:** Journal (Instrumente_IC_Konzept.md IK2-2 a: „jede Übernahme steht in `Uebernahme_Journal.md`“)
> **Status:** wird je Teilschritt fortgeschrieben
> **Bezug:** [../../audio/Instrumente_IC_Konzept.md](../../audio/Instrumente_IC_Konzept.md) §5 (IK2-2 a, IK2-2-1 a), §6 (34c,
> 34d), KN1 a, KN2 a, FU1 a–FU3 a, BK1 a
> **Sprache:** Deutsch

---

## 0. Wozu

Patrik (S173): „etwas kann ja eine reichere Fassung haben, aber falsch sein“ — und „auch Fehlschläge sind Erkenntnisse“.
Je Baustein steht hier: **Quelle** (Repo, Commit, Datei), **Ziel**, was übernommen und was verworfen wurde, **warum**,
und das Ergebnis der Wächter vorher/nachher. Eine falsche Übernahme lässt sich so finden und an der Quelle in Git
nachlesen. Was wegfällt, bleibt hier mit dem, was es gezeigt hat.

Stände der Quellen: **IC** `…\cmake\InstrumentsCalculation`, `master` `bb7e6d6` (Arbeitsbaum mit IC Session 5,
uncommittet; geprüft S175: `Eigenzerlegung`, `Fft`, `Kurve`, `Modal` und pocketfft sind dort unverändert gegen
`bb7e6d6`; `Werkzeug.*` ist im Arbeitsbaum nur um `lautheit` ergänzt, `CMakeLists.txt` nur um Werkzeug-Ziele — die
übernommenen Teile und die Schalter sind gleich); **LumiViz**
`master` `a7e2bbf0` (Stand vor 34c).

## 1. Stufe 34c — Bibliothek `InstrumentCore` (S175)

Ziel: `projects/libs/InstrumentCore` (STATIC, `lumi::kern`, KN1 a), Schalter nach E1 a PRIVATE in `Solution.json`:
MSVC `/fp:precise`, clang/gcc `-ffp-contract=off -fno-fast-math`. Zuschnitt KN2 a: was die App schon hatte, zieht um
(App-Kopien entfallen).

**Befund Schalter (J0):** IC `rechnung/CMakeLists.txt` setzt unter MSVC `/fp:precise /fp:contract-`. Zuerst so
übernommen; cl 19.51 meldet `D9002: Unbekannte Option "/fp:contract-" wird ignoriert` — die Option gibt es nicht
(MSVC kennt nur `/fp:contract` zum Einschalten). Seit VS 2022 17.0 zieht `/fp:precise` ohne `/fp:contract` keine
Ausdrücke mehr zu FMA zusammen; E1 a („keine automatische FMA“) gilt also schon mit `/fp:precise`. In LumiViz entfernt;
**für IC** (Auftrag in 34l): dieselbe wirkungslose Option streichen, die Bits ändert das nicht.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J1 | pocketfft | GitHub `mreineck/pocketfft` Zweig `cpp`, Commit `4d865cba24359b05b37da4e75aee24498c3b78e7` (2026-10-18) — dieselbe Datei wie IC `rechnung/externals/pocketfft/pocketfft_hdronly.h` (dort CRLF, LF-Prüfsumme `518a95c5…` stimmt) | `externals/pocketfft/` (ersetzt `c90e55b`) | die Datei unverändert, LF; Pin im README | Unterschied zu `c90e55b` (91 Zeilen): nur Fehlerbehandlung — 28 × `throw` → `POCKETFFT_THROW`, Schalter `POCKETFFT_NO_EXCEPTIONS` (nicht gesetzt). Die Rechnung ist unverändert → erwartet bitgleich. IC baut **ohne** `POCKETFFT_NO_MULTITHREADING`, LumiViz mit (bleibt so; `nthreads` = 1 an jedem Aufruf in beiden) |
| J2 | Fft | LumiViz `include/loop/Fft.hpp`, `src/loop/Fft.cpp` (`a7e2bbf0`); IC `rechnung/gemeinsam/Fft.cpp` (`DirekteDft`) | `InstrumentCore/Fft.hpp`, `src/Fft.cpp` | Schnittstelle und `FftPocket` aus LumiViz (mit der Prüfung der Spektrumlänge, die IC nicht hat); `nthreads` = 1 ausdrücklich wie IC; `DirekteDft` aus IC | `DirekteDft` steht **nicht** in `fftUmsetzungen()` (in IC schon), sondern einzeln als `direkteDft()`: `test_LoopFft` rechnet jede Umsetzung bis n = 65536, O(n²) wären dort rund 4·10⁹ Schritte. Name der Vorgabe `pocketfft 4d865cb` (vorher `pocketfft c90e55b`) |
| J3 | Eigenzerlegung | IC `rechnung/gemeinsam/Eigenzerlegung.hpp`/`.cpp` (`bb7e6d6`), dort aus LumiViz `src/loop/Eigenzerlegung.cpp` (S153) portiert | `InstrumentCore/Eigenzerlegung.hpp`, `src/Eigenzerlegung.cpp` | wortgleich, nur Namensraum, Include, Köpfe | gegen LumiViz neu: `kKleineMatrixMax` 32 statt 16 (Luftraum mit 15 Kolben in IC), `kleinsterAbstand`, `entartet` (IC L11). Die Rechnung ist wortgleich (Diff der `.cpp` in S175 gelesen: nur die Meldung kommt dazu). Der Flügel nutzt höchstens 6 × 6 |
| J4 | Anschlagstufe | LumiViz `loop/HammerSaite.hpp`/`.cpp`; IC `klassen/01_hammer_saite/HammerSaite.cpp` `anschlagStufe`, `klassen/04_platte_stab/Stab.cpp` `stufeAus` (alle drei dieselbe Formel) | `InstrumentCore/Anschlag.hpp`, `src/Anschlag.cpp` | `anschlagStufe`, `anschlagRest`, `kAnschlagSchrittDb`, `kAnschlagStufeMin` wortgleich; neu `anschlagFaktor` (der Ausdruck, der in `hammerTempo` und `anschlagRest` stand) | `hammerTempo` bleibt beim Hammer (`loop/HammerSaite`, Größe des Hammers), rechnet mit `kern::anschlagFaktor` — derselbe Ausdruck, erwartet bitgleich. IC `stufeAus` fällt beim Umzug der Klasse 4 weg (34g) |
| J5 | Kurve | IC `rechnung/gemeinsam/Kurve.hpp`/`.cpp` (`bb7e6d6`) | `InstrumentCore/Kurve.hpp`, `src/Kurve.cpp` | wortgleich, nur Namensraum, Kopf, `<initializer_list>` (fehlte) | die `Tastenkurve` des Flügels (Qt-JSON, Tasten 21–108) bleibt bis 34d; ein Test prüft `wertAn` beider **Bit für Bit** an 40 Kurven, 101 Tasten, beiden Skalen |
| J6 | Modal | IC `rechnung/gemeinsam/Modal.hpp`/`.cpp` (`bb7e6d6`) | `InstrumentCore/Modal.hpp`, `src/Modal.cpp` | wortgleich, nur Namensraum und Kopf | in LumiViz bis 34g ohne Nutzer; `HammerTon.cpp` rechnet seine Moden weiter selbst (IK2-2 a: HammerTon bleibt wortgleich) |
| J7 | Pool `parallelJe` | IC `rechnung/gemeinsam/Werkzeug.hpp`/`.cpp` (`bb7e6d6`); LumiViz `tests/unit/UnitTests/Modellton.hpp` (`modelltest::parallelJe`, `poolThreads`, `kPoolHoechstens`) | `InstrumentCore/Pool.hpp`, `src/Pool.cpp` | Form aus IC `(anzahl, aufgabe, threads = 0)`, Grenze 11 aus beiden | **neu:** eine Ausnahme in einer Aufgabe wird nach dem Join weitergeworfen (IC und LumiViz-Test: `std::terminate`). Die LumiViz-Testfassung entfällt; ihre fünf Aufrufe (`Modellton.hpp`, `Schwebungsstufe.hpp`, `test_LoopSalamanderProbe.cpp`) rufen die Bibliothek. Unterschied: IC begrenzt die Threads auf die Zahl der Aufgaben und rechnet mit einem Thread auf dem rufenden — am Ergebnis ändert das nichts (jede Aufgabe für sich) |
| J8 | SHA-256 | IC `rechnung/gemeinsam/Werkzeug.hpp`/`.cpp` (`bb7e6d6`) | `InstrumentCore/Pruefsumme.hpp`, `src/Pruefsumme.cpp` | `Sha256`, `sha256`, `abdruck` wortgleich | dazu `sha256(string_view)` und `Sha256::dazu(string_view)`; geprüft gegen FIPS 180-2 B.1/B.2. Die Qt-Schicht der App behält `QCryptographicHash` |
| J9 | KettenNote | IC viermal (`klassen/03_bogen_saite/Bogen.hpp:94`, `07_lippe_rohr/Blech.hpp:111`, `08_rohrblatt_rohr/Rohrblatt.hpp:95`, `09_luftstrahl_rohr/Luftstrahl.hpp:89`: `{abS, note, hz}` in Sekunden, je eigene Vorgaben) | `InstrumentCore/Tonauftrag.hpp` | der Gedanke | **geändert** nach IK2-3 a: Frames statt Sekunden, `{beginn, ende, note, hz, gleitet}`; die vier IC-Fassungen bleiben bis zu ihren Klassen (Ketten, später) |
| J10 | normiere | IC elfmal je Klasse (`klassen/*/Vorgaben.cpp`, `11_tonrad/Orgel.cpp`) | `InstrumentCore/Tonkern.hpp` `normiere` | das Verfahren: Bezug 1, Ton an der Mitte bei Stufe 0, Bezug = rohe Spitze / 0,5; Runden wählbar (Membran: zwei, „die Rauschanteile sind relativ zum Bezug gesetzt“, `05_membran/Vorgaben.cpp:25–40`) | **geändert:** über Fabrik und Tafel statt je `Spiel`; der Bezug ist ein Wert der Tafel (IK2-10 a). Werkzeug und Wächter folgen in 34f |
| J11 | Tonkern, Fabrik, Auskunft, Tonauftrag, Parametertafel | neu nach IK2-3 a, IK2-4 a, §6a (CK1 a) | `InstrumentCore/Tonkern.hpp`, `Tonauftrag.hpp`, `Parametertafel.hpp` | — | Entscheidungen im Bau (nicht vorgelegt, aus den Entscheiden abgeleitet): die Auskunft hängt am `KernTyp`, nicht am erzeugten Kern (die Planung braucht sie, ohne vorzubereiten); `ungemessen` folgt aus der Belegkraft (IK2-9, Folgerung S173) mit `klangbestimmend` je Angabe; ein gestimmter Wert merkt den Abdruck seiner Abhängigkeiten (`abhaengigVon`, leer = alle nicht gestimmten) plus `rechenFassung`; ein fehlender Wert ist ein Fehler, ein Wert mit Fehler bleibt stehen, ein unbekannter fällt beim Richten weg (gemeldet) |

**Weggefallen in der App** (KN2 a): `include/loop/Fft.hpp`, `src/loop/Fft.cpp`, `include/loop/Eigenzerlegung.hpp`,
`src/loop/Eigenzerlegung.cpp`, `include/loop/Eigenzerlegung.md` (Inhalt in `InstrumentCore/InstrumentCore.md`),
`anschlagStufe`/`anschlagRest`/Konstanten in `loop/HammerSaite`; in den Tests `modelltest::parallelJe`,
`poolThreads`, `kPoolHoechstens`. Nachzulesen in LumiViz `a7e2bbf0`.

**Wächter 34c** (KN2 a), alles nach J1–J11 zugleich:

- **Release-clang, voller Lauf** 1765 / 1765, 0 Skips (10:10 min; vorher 1753, dazu die 12 Fälle `*InstrumentCore*`).
  Darin `*Loop Werte*`, `*Klangfassung*` und der Flügel-Bezug (`hammer_bezug_*.txt` in `test_LoopHammerTon`)
  **unverändert** — pocketfft `4d865cb`, die Eigenzerlegung aus IC und `anschlagFaktor` rechnen Bit für Bit wie vorher.
- **Testing-MSVC, voller Lauf** 1765 / 1765, 0 Skips (11:06 min), nach J0 neu gebaut ohne Warnung an `InstrumentCore`
  (`FloatingPointModel` Precise im `.vcxproj` nur dort).
- **Release-clang, alle Ziele** gebaut (App, Werkzeuge, Standalones).
- **Quellpaket** `package_instrumentcore` → `out/package/instrumentcore-v0.1.0.zip` (SHA-256 `4b9a93f6…`): 10 Quellen,
  11 Köpfe (mit `InstrumentCore.md`), `externals/pocketfft/` mit `POCKETFFT_NO_MULTITHREADING`, die Schalter wörtlich im
  `CMakeLists.txt`; `sha256sum -c SHA256SUMS` ohne Nacharbeit grün, jede Datei LF (aus den Bytes geprüft).
- **Rechenzeit** `*31b-3 Rechenzeit*` (RZ1-1 a), Release-clang, Taste 40 (der schwerste Ton), je Lauf gehalten 500 /
  4000 ms. Drei Läufe nach einer ruhigen Taktprobe gaben 4,69 / 4,74, 5,21 / 5,13, 4,63 / 4,50 % — über der Grenze 4,6 %,
  aber ohne Bezug seit dem Hammer nach Stulov (S159; letzte Messung S156: 3,73–3,82 %). Deshalb der Stand **vor** 34c
  (`a7e2bbf0`, per `git archive` im Scratchpad gebaut) **abwechselnd** mit dem neuen gemessen, je nach Taktprobe:

  | Durchgang | Taktprobe | vorher (`a7e2bbf0`) | neu (34c) |
  |---|---|---|---|
  | 1 | 1,16 (knapp) | 3,96 / 4,01 % | 4,02 / 4,05 % |
  | 2, 3 | gedrosselt | 31,93 / 23,69 %, 7,81 / 6,49 % | 9,00 / 9,73 %, 4,70 / 4,19 % |
  | 4 | gedrosselt | 5,78 / 6,00 % | 4,03 / 3,93 % |
  | 5 | ruhig | 4,05 / 4,01 % | 3,95 / 4,15 % |

  **Ergebnis:** in Ruhe gleich (3,9–4,2 %), unter 4,6 %; die Werte darüber kamen vom Drosseln des Prozessors, nicht von
  34c. Taste 21 2,7–2,9 %, Taste 60 2,2–2,3 % in Ruhe.

**Nicht geprüft:** das Paket unter Linux (kommt mit dem ersten Bau in IC; `parallelJe` braucht dort Threads —
glibc ≥ 2.34 hat sie in der libc, das erzeugte `CMakeLists.txt` linkt `Threads::Threads` nicht).

## 2. Stufe 34d — der Flügel als Kern `hammerString` (S175, S176)

Ziel nach BK1 a: `projects/libs/InstrumentModels` (STATIC, je Klasse Unterordner und Namensraum, `hammer/` mit
`lumi::kern::hammer`; Paket `instrumentmodels`), Schalter nach E1 a PRIVATE wie `InstrumentCore`. Dazu FU1 a (Körper an
der Schicht), FU2 a (Dämpfer und Pedale in der Tafel), FU3 a (der Creator zeigt die Kern-Schicht). Quellen: LumiViz
`master` `ed6a8d99` (Stand nach 34c), IC `bb7e6d6`.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J12 | HammerTon | LumiViz `src/loop/HammerTon.cpp`/`include/loop/HammerTon.hpp` (`ed6a8d99`); IC `rechnung/klassen/01_hammer_saite/HammerTon.cpp` (`bb7e6d6`, aus LumiViz portiert) | `InstrumentModels/hammer/HammerTon.hpp`, `src/hammer/HammerTon.cpp` | die **IC-Fassung**: gegen LumiViz dazu `HammerAnschlag::tempoFaktor` (Soft: so viel langsamer trifft der Hammer) und der Moderator in Reihe (Practice: die Eindrückung teilt sich, bis beide Filze dieselbe Kraft tragen, 60 Halbierungen). Diff gegen IC 24 Zeilen, nur Include, Namensraum, `Tastenkurve`/`KurvenSkala` → `Kurve`/`Skala` | mit den Vorgaben (`tempoFaktor` 1, Moderator 0) rechnet der Ton dieselben Ausdrücke in derselben Reihenfolge → erwartet bitgleich, belegt durch J17. Die Moden rechnet `HammerTon` weiter selbst (J6: `Modal` bleibt bis 34g ohne Nutzer) |
| J13 | HammerSaite, Kurven, Vorgaben | LumiViz `src/loop/HammerSaite.cpp`/`include/loop/HammerSaite.hpp` (`ed6a8d99`) | `InstrumentModels/hammer/HammerSaite.hpp`, `src/hammer/HammerSaite.cpp` | die Zahlen der Vorgaben wortgleich (Stulov, RT-0425, Rigaud, die Messungen an *Salamander*); `kurvenAngaben()` als die eine Liste der 22 Kurven mit Name, Titel, **Gruppe**, Einheit, Skala, Grenzen | **geändert:** `Tastenkurve` → `kern::Kurve` (J5; der Typ der App entfällt, der Vergleich in `test_InstrumentCore` mit ihm); **neu** Belegkraft und Herkunft je Kurve (IK2-4 a) — aus gemessenen und belegten Teilen gilt die schwächere (B: gemessen bis E2, darüber Rigaud → *Quelle*); **weg** die Qt-JSON-Funktionen (die Datei liest und schreibt die App allgemein nach der Tafel, `loop/KernDatei`), `Resonanzboden` (FU1 a: `Schicht::koerper`), `Tastatur` (FU2 a: `loop/Tastatur` für die Planung, `undampedFrom` in die Tafel) |
| J14 | Kern und Tafel des Flügels | neu nach IK2-3 a, IK2-4 a, FU2 a; die Pedalwerte aus IC `klassen/01_hammer_saite/HammerSaite.hpp` `Tastatur` (Klasse 1 Rest) | `InstrumentModels/hammer/HammerKern.hpp`, `src/hammer/HammerKern.cpp` | Tafel: die 22 Kurven, Gruppe *Pedals* mit `undampedFrom` (Ganzzahl 89, *Quelle*: SFZ von *Salamander*), `softFactor` (1), `moderatorStiffness` (2), `moderatorExponent` (2) — die drei *Offen* wie in IC; Auskunft Rechenweg *Note*, Stufe im Namen, Spuren `soft`/`practice`, innere Größe `contact`; `HammerKern` hält Spiel und Pedale, rechnet die erste Note | Entscheidungen im Bau: getreten heißt Spurwert ≥ 0,5 am Beginn der Note; ohne Note bleibt der Ton still; `rechenFassung` 1 |
| J15 | `kerne()` | in 34c in `InstrumentCore/Tonkern.hpp` (ohne Kern) | `InstrumentModels/Kerne.hpp`, `src/Kerne.cpp` | die Fabrik der eingebauten Kerne, heute `hammerString` | **verschoben** nach BK1 a: `InstrumentCore` kennt keinen Kern |
| J16 | Anbindung in der App | `ed6a8d99` | `loop/Instrument` (Fassung 21: `core`, `coreParams`, `body`), `loop/KernDatei`, `loop/Tastatur`, `tonkernFuer` (gemerkt unter dem SHA-256-Abdruck von Kern und Werten, höchstens 256), `rendereTon` über `Tonkern`, `stufeImNamen` statt `InstrumentAuskunft::hammer` | ein fehlender Wert oder ein unbekannter Kern macht die Datei unlesbar (IK2-11 a, kein stiller Ersatz); `grand_piano.lvinst` in Fassung 21, die Werte der Datei unverändert (verglichen mit der Fassung 20: Kurven, Körper, `undampedFrom`, alle übrigen Felder) | **weg:** `SaitenModell::Modal`, `Anregung`; `stringModel` `modal` (Fassung 16–20) wird nicht mehr gelesen — eine solche Schicht wird eine Saite Classic (unveröffentlicht, keine Rückwärtspflege) |
| J17 | Flügel-Bezug | `werte/hammer_bezug_clang.txt`, `hammer_bezug_msvc.txt` (untereinander gleich); der Stand vor 34c (`a7e2bbf0`, in S175 unter clang geschrieben) | `werte/hammer_bezug_windows.txt` (IK2-5 a: je Plattform) | der Stand vor 34c; **jeder** der neun Töne Bit für Bit | die alten Dateien waren im Chor älter als 34c: **7 von 9** Tönen trugen andere Prüfsummen (S175 notierte 6; C2 mit zwei Saiten mitgezählt), der Test verglich dort nur den Pegel mit 0,01 dB. Die Toleranz im Chor entfällt. Die Saite des Bezugs bleibt `hammer_bezug_saite.json` im Format bis Fassung 20, gelesen über `kurvenAngaben()` (wird nicht mehr geschrieben) |
| J18 | Creator | neu nach FU3 a, §6a (CK1 a, CK2 a) | `loop/KernAnsicht`, `loop/InstrumentCreator` | Zeile *Core*, *Body*, *Level*; die Tafel zum Ansehen in ihren Gruppen (Kurve als kleine Form mit kleinstem und größtem Wert, Marke der Belegkraft, Herkunft im Tooltip), *Unmeasured* neben *Type* | Entscheidungen im Bau: *+ Add* grau (der Kern liest die Reihe nicht); kein Code je Kern — alles aus der Tafel |

**Wächter 34d** (S176), alles nach J12–J18 zugleich:

- `*Loop Werte*` **unverändert** (`loop_werte.txt` ohne Änderung) — die mitgelieferten Loops, auch `grand_piano_demo` und
  `nocturne_demo`, rechnen Bit für Bit wie vorher.
- `klangfassung.txt` neu (nur Abdrücke): 379 von 380 Einträgen mit neuem Abdruck der Eingaben (Fassung 21), **kein Wert
  anders**; danach 380 Einträge verglichen, 0 anders.
- Flügel-Bezug `hammer_bezug_windows.txt`: alle neun Töne Bit für Bit unter Release-clang **und** Testing-MSVC — E1 a
  macht beide gleich (IK2-5 a belegt).
- **Voller Lauf** Release-clang 1769 / 1770 (11:42 min; rot nur das Literal `kInstrumentFassung == 20` in
  `test_LoopInstrument`, auf 21 gestellt, danach die Datei 18 / 18); **Testing-MSVC 1770 / 1770, 0 Skips** (11:13 min).
- **Quellpaket** `package_instrumentmodels` → `out/package/instrumentmodels-v0.1.0.zip`: Quellen, Köpfe, `VERSION`, die
  Schalter wörtlich im `CMakeLists.txt`, das Ziel `InstrumentCore` verlangt; `sha256sum -c SHA256SUMS` grün, jede Datei LF.
- **Rechenzeit** `*31b-3 Rechenzeit*` (RZ1-1 a), Release-clang, Taste 40 (gehalten 500 / 4000 ms). Drei Läufe allein nach
  einer Taktprobe mit einem Drosselschub: 4,16 / 4,36, 4,96 / 5,41, 5,51 / 5,69 % — Taste 60 stieg dabei mit (2,0 → 3,2 %),
  also die Wärme. Deshalb gegen den Stand **vor** 34d (`ed6a8d99`, per `git archive` im Scratchpad gebaut)
  **abwechselnd**:

  | Runde | Bezug 34c | 34d | zusammen 34c / 34d |
  |---|---|---|---|
  | 1 | 4,08 / 7,49 % | 5,42 / 5,04 % | 750 / 723 ms |
  | 2 | 4,48 / 4,68 % | 4,31 / 4,23 % | 627 / 612 ms |
  | 3 | 4,31 / 4,40 % | 4,18 / 3,99 % | 567 / 537 ms |

  **Ergebnis:** in jeder Runde gleich oder schneller als vorher; in der ruhigsten unter 4,6 %.

## 3. Stufe 34e — Bibliothek `InstrumentMeasure` (S177)

Ziel nach KN1 a, IK2-2 a: `projects/libs/InstrumentMeasure` (STATIC, `lumi::messung`, Paket `instrumentmeasure`), in 34e
nur über `InstrumentCore` (und ab 34e-2 `AudioCore`, ME1 a), Schalter nach E1 a PRIVATE wie die Schwestern. Teilplan
34e-1 bis 34e-5 im Konzept §6. Quellen: LumiViz `master` `b54b2cd9` (Stand nach 34d), IC `bb7e6d6` (Arbeitsbaum mit IC
Session 5).

**Befund vor dem Umzug (S177):** IC `rechnung/messung/` führt die LumiViz-Messkette als Port (aus `vorbild-lumiviz`,
LumiViz `d1c06309`, IC Session 4; `Hoerschwellen`, `Probe` aus `3fa95944`, IC Session 5). Diff gegen LumiViz je Datei:
nur Namensraum (`ic::messung`), `ic::Fft`, Pfade, Herkunftsvermerk — außer **`LevenbergMarquardt`** (IC 345 Zeilen,
LumiViz 653: `levenbergMarquardtJeBlock`, `lmSchritt` mit Maske, 7.5.12 A2/A3, gibt es nur in LumiViz). Die
LumiViz-Fassung ist damit überall die reichere oder gleiche; übernommen wird sie, der IC-Port entfällt mit 34l.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J19 | Messkette | LumiViz `tests/unit/UnitTests/` `Teiltonmessung.hpp`, `Fluegelmessung.hpp`, `Strukturmessung.hpp`, `Kurvenabstand.hpp`, `Zielsumme.hpp`, `Hoerschwellen.hpp`, `Probe7512.hpp`, `LevenbergMarquardt.hpp`, `NelderMead.hpp` (`b54b2cd9`) | `InstrumentMeasure/<Name>.hpp`, `src/<Name>.cpp` | jede Funktion wortgleich; der Namensraum `lumi::messung` war schon gesetzt | **geteilt:** 98 freie Funktionen und die Methoden von `detail::Spektrum` und `detail::Geraden` in die `.cpp` (E1 a wirkt nur dort; Werkzeug im Scratchpad, Vorgabewerte bleiben im Kopf), kleine Zugriffe (`teilton(k)`, `fuer`, `Matrix`) bleiben im Kopf; **umbenannt** `Probe7512` → `Probe` (Name aus IC: die Probe gilt für jede Klasse); die Kopien in den Tests entfallen |

**Wächter 34e-1** (S177):

- `LUMIVIZ_SALAMANDER` (Aufnahme von `E:\samples\Salamander\…`) misst neu in 31 s und schreibt `salamander_c5.txt`,
  `salamander_kurven.txt`, `salamander_struktur.txt`, `salamander_ziel.txt` — **alle vier ohne Diff** gegen Git (Release-clang).
- `*Teiltonmessung*,*Salamander*,*Kurvenabstand*` 73 / 73 unter Release-clang (113 s); Testing-MSVC gebaut.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J20 | Lautheit, WAV-Format | LumiViz `include/audio/Lautheit.hpp`, `src/audio/Lautheit.cpp`, `include/audio/WavFormat.hpp`, `src/audio/WavFormat.cpp` (`b54b2cd9`); IC `gemeinsam/Werkzeug` (`schreibeWav`, `leseWav`, `lufsNaeherung`, `lautheit`) | `projects/libs/AudioCore` (ME1 a), `AudioCore/Lautheit.hpp`, `AudioCore/WavFormat.hpp`, Namensraum `lumi::audio` wie bisher | die LumiViz-Fassung wortgleich (Lautheit gegen EBU Tech 3341 belegt, `test_ComposerLautheit`); in der App und den Tests nur die Include-Pfade | **verworfen:** die IC-Fassung — `lufsNaeherung` ohne Gating (nach IC selbst nur für Hörproben), `lautheit` nach BS.1770-4 nur Mono und ohne Momentary/Short-term, die Koeffizienten „aus dem Gedächtnis, offen“ (IC `Werkzeug.hpp:11`); das Lesen von WAV (IC `leseWav`: 16/24 Bit PCM, 32-bit float) deckt `liesWavBytes`/`liesWavDatei` ab. **Offen:** IC `schreibeWav` (32-bit float, ohne Qt) hat in `AudioCore` kein Gegenstück — LumiViz schreibt über `audio/WavDatei` mit Qt (32/24/16 Bit mit Dither); bis 34l bleibt das Schreiben für IC dort. IC bezieht `audiocore` mit 34l |

**Wächter 34e-2** (S177): `*Lautheit*,*Pegel*,*Wav*,*Loop Werte*,*Begrenzer*,*Abgleich*` 105 / 105 und
`test_ComposerLautheit`, `test_ComposerClipLautheit`, `test_WavDatei`, `test_LoopExport`, `test_LoopFreeze` 30 / 30 unter
Release-clang; `loop_werte.txt` und `werte/` unverändert; App und Tests gebaut unter Release-clang und Testing-MSVC.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J21 | Bausteine B1–B8, `amplitudenBei` | IC `rechnung/messung/Bausteine.hpp`, `.cpp` (`bb7e6d6`; nur in IC) | `InstrumentMeasure/Bausteine.hpp`, `src/Bausteine.cpp` | die Rechnung wortgleich: B1 Verfolgung (McAulay–Quatieri), B2 YIN, B3 Vibrato, B4 Rest (SMS), B5 Einschwingen, B6 Modalfit (ESPRIT), B7 Bänder, B8 Gleiten; die Fälle aus IC `messung/test_messung.cpp` mit denselben Signalen und Grenzen in `test_InstrumentMeasure` | **geändert:** Namensraum `lumi::messung`, Fft-Typ `lumi::kern::Fft` im Kopf, in der `.cpp` `using`-Erklärungen für die Namen aus `lumi::kern` (der Rumpf bleibt wortgleich), Include von `gemeinsam/Balken.hpp` entfällt (J23). **Nicht übernommen:** der Fall „B6 Modalfit am Modell“ (rechnet einen Xylophonton mit dem Kern der Klasse 4 — kommt mit 34g). Die IC-Kerne rufen `tonhoehe`/`einschwingen` heute selbst auf (fünf Klassen) — nach IK2-2 a geben sie die Messgrößen ab, Thema ab 34g |
| J22 | Samples | IC `rechnung/messung/Samples.hpp`, `.cpp` (`bb7e6d6`) | `InstrumentMeasure/Samples.hpp`, `src/Samples.cpp` | `liesSummen`, `sha256Datei`, `pruefeDatei` wortgleich (SHA-256 aus `InstrumentCore/Pruefsumme`, J8) | **geändert:** der Name der Umgebungsvariable kommt vom Aufrufer (`samplesWurzelAus(variable, wert, meldung)`, `samplesWurzel(variable, meldung)`) statt fest `IC_SAMPLES`; Meldungen nennen die Variable. Im Testfall statt einer WAV aus `schreibeWav` eine Datei mit festen Bytes (der Inhalt ist für die Summe gleichgültig; `schreibeWav` hat kein Gegenstück, J20) |
| J23 | `zerlegeSymmetrisch` | IC `rechnung/gemeinsam/Balken.hpp:28–34`, `Balken.cpp:63–127` (`bb7e6d6`) | `InstrumentCore/Eigenzerlegung.hpp`, `src/Eigenzerlegung.cpp` (ME2 a) | wortgleich, mit den Hilfen `bei`/`beiC` (anonymer Namensraum) | Kipppunkt von ME2 a geprüft: der Löser ist eigenständig, Cholesky steht in `biegeModen`, nicht im Löser. Nutzer in IC: Balken (4, 10), Membran (5), ESPRIT (B6). Neu ein Fall in `test_InstrumentCore` gegen die geschlossene Lösung der Tridiagonalen (n = 1, 2, 8, 40: Werte auf 10⁻¹², A·v = λv, orthonormal) und eine diagonale Matrix; ein Bit-Vergleich gegen einen IC-Lauf fehlt (IC hat keinen Fall für den Löser allein) |

**Wächter 34e-3** (S177): `*InstrumentMeasure*,*InstrumentCore*` 21 / 21 unter Release-clang (8 neu aus IC, 1 neu für
J23); Testing-MSVC gebaut.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J24 | `setzen` | IC `rechnung/setzen/Setzen.hpp`, `.cpp`, `test_setzen.cpp` (`bb7e6d6`; nur in IC) | `InstrumentMeasure/Setzen.hpp`, `src/Setzen.cpp` (`lumi::messung::setzen`), `test_InstrumentMeasureSetzen.cpp` | der Rahmen wortgleich: normierter Raum u, L9 (wirkungslos vorab fest), L11 (verrauscht), L12 (wellig zuerst eindimensional), L16 (Träger), Protokoll je Iteration, `schnitt`, `welligkeit`, `glaette`, `setzeMehrfach`, `eindeutig`, `probe`, `alsTasten`, Hörschwellen; die zehn IC-Fälle mit ihren Grenzen | **ersetzt:** das eigene LM je Gruppe (IC `Setzen.cpp:325–424`) durch `levenbergMarquardtJeBlock` (eine Gruppe = ein Block, die k-te freie Spalte jeder Gruppe = eine Gruppe der finiten Differenz) — damit **A3** in `setzen` (`wirkungMindestens` 0,1). **Neu (ME3 a):** `Parameter::schritt`/`ende` (im Raum u, 0 = Einstellung), `Einstellung::beobachter` (`Iteration` mit `GruppenSchritt` je Gruppe), `Einstellung::start` (Wiederaufnahme durch den Aufrufer, geht vor dem Protokoll), im Bericht `ohneWirkung` und `lambda`. **Geändert:** die Enden heißen wie im LM je Block („Schritte klein“, „kein besserer Schritt“, „Iterationen“); im Protokoll eine Spalte `iterationen`, Enden ohne Leerzeichen (`_`), „Iterationen“ gilt beim Lesen nicht als Ende; Iterationen je Gruppe zählen die aktiven (IC: die Nummer der letzten Annahme) |
| J25 | Endregel des LM (ME4 a) | IC `setzen/Setzen.cpp:382–404` (ein kleiner Schritt wird ausgewertet) gegen LumiViz `LevenbergMarquardt.cpp` (7.5.10: endet ohne Auswertung) | `levenbergMarquardt`, `levenbergMarquardtJeBlock` | die IC-Regel in beiden LM: der kleine Schritt wird ausgewertet und genommen, wenn er Σ r² senkt; danach „Schritte klein“ | **Befund:** mit der LumiViz-Regel endete der IC-Fall „findet die Werte“ bei Rest 0,502 statt 0,107 (fast ganz in der verrauschten Spalte, r = −0,708; mit und ohne A3 gleich), „ohne L12“ bei 0,905 statt 0,307 — die Prüfung „mehr als das Doppelte“ fiel. Mit der IC-Regel genau die IC-Zahlen (im IC-Bau `out-clang` nachgefahren) |
| J26 | Flügel-Suche über `setzen` (ME3 a) | LumiViz `test_LoopSalamanderSuche.cpp` `sucheJeTaste` (`2076a29f`) | dieselbe Funktion, innen `setzen::setze` | je Taste eine Gruppe, ihre Stützpunkte die Parameter (Grenzen, Skala aus `SuchKurve`), Schritt und Ende = 1 % der Größe in u umgerechnet, die Ziele mit der wirksamen Toleranz wie `vergleiche` (UG1 a) — die Residuen sind dieselben Zahlen; nach außen dieselben Dateien, `LmStartBloecke`, `LmErgebnisBloecke` | das `Modell` rechnet ohne eigenen Pool (`werteAus(…, threads)` neu; `parallelJe` verschachtelt sich nicht); die Summen je Taste merkt es unter einer Sperre; neu der Schalter `LUMIVIZ_75_SUCHE2_ITERATIONEN` für die Probe |

**Wächter 34e-4** (S177):

- **Probe vorher/nachher** (Release-clang, `LUMIVIZ_75_SUCHE2` mit Tasten 33 und 60, Startpunkt 1, zwei Iterationen; vorher
  auf `2076a29f` + Schalter, nachher mit J24, J26, vor J25 — die Probe endet über die Iterationen, J25 wirkt dort nicht):

  | | Rest Iteration 1 | Rest Iteration 2 | getroffen | Auswertungen | Dauer |
  |---|---|---|---|---|---|
  | vorher (LM je Block direkt) | 3365,7 → 2032,1 | 1111,8 | 6, 7 / 32 | 42 | 181 s |
  | nachher (über `setzen`) | 3365,7 → 2032,1 | 1111,9 | 6, 7 / 32 | 96 | 255 s |

  **Derselbe Weg, nicht Bit für Bit.** Ursache: `setzen` rechnet im normierten Raum u statt in ln x — das LM mit
  Marquardt-Skalierung ist gegen die Umskalierung unveränderlich, die Rundung nicht: nach Iteration 1 ≈ 1e-10 relativ,
  in Iteration 2 (λ = 1e-5) bis 1,7e-4 relativ (`stossLeise` an 33), Rest der Taste 33 752,420 → 752,425. Die
  Auswertungen mehr sind L9/L11 vor dem Lauf (zwei Jacobi-Matrizen aller Gruppen) und die Jacobi-Matrix für den Bericht
  am Ende; über 20 Iterationen an 30 Tasten grob 10 % (geschätzt, nicht gemessen).
- `*InstrumentMeasure*,*InstrumentCore*,*7.5.10 Suche*,*7.5.11*,*7.5.12*,*Salamander*` 88 / 88 unter Release-clang (mit J25).

**Wächter 34e-5** (S177, Abschluss von 34e):

- **Voller Lauf** 1789 / 1789, 0 Skips unter Release-clang (640 s) und Testing-MSVC (613 s); `werte/` unverändert
  (`loop_werte.txt`, `klangfassung.txt`, Flügel-Bezug, `salamander_*.txt`).
- **Quellpakete** `audiocore-v0.1.0`, `instrumentcore-v0.2.0` (neu wegen `zerlegeSymmetrisch`), `instrumentmodels-v0.1.0`,
  `instrumentmeasure-v0.1.0` unter `out/package/`: `sha256sum -c SHA256SUMS` grün, jede Datei LF, Modul-Doku darin, die
  Schalter nach E1 a im erzeugten `CMakeLists.txt`; `instrumentmeasure` verlangt die Ziele `InstrumentCore` und
  `AudioCore`.
- Rechenzeit `*31b-3*` nicht gemessen: 34e berührt keinen Kern (RZ1-1 a gilt, wo ein Kern berührt wird).

## 4. Stufe 34f — Werkzeuge und Wächter (S177)

Teilplan 34f-1 bis 34f-5 im Konzept §6 (WZ1 a, WZ2 a). Quellen: LumiViz `master` `c27af4b0` (nach 34e), IC `bb7e6d6`.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J27 | Quellenregister | IC `UEBERNAHME_LUMIVIZ.md` §3 (24 Zeilen der Tabelle, dazu die zwei „nicht geladen“: Brauchler 2023, Abdallah 2026); die Herkunftstexte der Flügel-Vorgaben (`InstrumentModels/src/hammer/HammerSaite.cpp`) | `asset/loop/instrument_quellen.json` (WZ2 a, 31 Einträge) | je Eintrag Nennung, Lizenz und Pflicht wie in IC; „nichts ausliefern“ → `shippable: false` (Weinzierl NC, TU-Note ND, beide Iowa bis zur schriftlichen Bestätigung) | **neu:** `salamander` (CC BY 3.0, belegt im README des Archivs „Licence: CC-by … /by/3.0/“, „Author: Alexander Holm“), `rt0425` (Titel belegt im Repo: „Physical parameters for piano modeling“, hal-00688679), `stulov2009`, `rigaud2011` (nur Autoren, Jahr, Gleichung — ein Titel steht im Repo nicht; **Lizenz der drei Veröffentlichungen nicht geprüft**, übernommen werden Zahlen und Formeln), `lumiviz` (eigene Vorgabe). `credit`/`license` englisch (sie stehen in der englischen `CREDITS.md`), `note` deutsch |
| J28 | Quellen am Wert (WZ1 a) | IK2-12 a | `Kurve::quellen`, `ParameterAngabe::quellen` (InstrumentCore), `sources` in `loop/KernDatei`, `.lvinst` **Fassung 22** | die Vorgaben des Flügels nennen ihre Schlüssel (B: `salamander`, `rigaud2011`; Winkel am Steg: `salamander`, `lumiviz`; Hammer: `stulov2009`; Saite und Anschlagstelle: `rt0425`; T60, hohe Dämpfung, Verstimmung: `salamander`; die vorläufigen und der Chor: `lumiviz`; Pedale: `undampedFrom` `salamander`, die drei offenen `lumiviz`) | die Quellen zählen wie die Herkunft nicht zum Klang (`Kurve::operator==`); `grand_piano.lvinst` neu (Programm, dann Kopf aus dem Katalog) — Diff nur Fassung und `sources` |
| J29 | `CREDITS.md` der Instrumente, Wächter | IK2-12 a (Muster `make_samples.py` bis zur Marke) | `asset/loop/make_instrument_credits.py`, `asset/loop/instruments/CREDITS.md` (in `Asset.cmake`, in `THIRD_PARTY_NOTICES.md`), `test_InstrumentQuellen.cpp` | das Skript liest die `sources` der Kurven; der Wächter dazu die Quellen der Vorgaben aus der Tafel | das Skript sieht die Quellen der Vorgaben, die keine Kurve sind, nicht (sie stehen in der Tafel, nicht in der Datei) — der Wächter in C++ sieht sie und meldet eine fehlende Pflichtquelle; heute deckt `salamander` aus den Kurven sie ab |
| J30 | Stimmen und Bezug als Werkzeug | IC `werkzeuge/stimme_*.cpp` (schreiben `Stimmung.inc`), IC `normiere` je Klasse (J10) | `Parametertafel::bezugWert`/`bezugRunden`, `kern::bezugsProbe`, `kern::normiereNachTafel` (InstrumentCore 0.2.0); `messung::Stimmer`, `stimmer()` (leer), `stimmeUndNormiere`, `veralteteWerte`, `bezugVeraltet` (`InstrumentMeasure/Stimmen`); `projects/exec/InstrumentTuneTool` | die Probe des Bezugs wie IC: mittlere Note, Stufe 0 (1 s bei 48 kHz); die gestimmten Werte als Daten mit Abdruck (IK2-11 a) | **geändert:** der Bezug zählt nicht zum Abdruck gestimmter Werte (er wird nach dem Stimmen gesetzt und hängt an ihnen — sonst wäre jede Stimmung nach dem Normieren veraltet); das Werkzeug schreibt mit Qt, die Form setzt danach `instrument_katalog.py`. Die Stimm-Funktionen der IC-Klassen kommen mit ihren Klassen (34j, 34k) — bis dahin prüft ein Prüfkern (`testTuned`), der Schreibweg des Werkzeugs ist erst mit einem stimmenden Kern zu prüfen |

**Wächter 34f** (S177):

- `grand_piano.lvinst` Fassung 22: Diff nur `version` und je Kurve `sources` (45 + / 23 −). `klangfassung.txt` neu: 379 von
  383 Zeilen mit neuem Abdruck der Eingaben, **kein Wert anders** (dieselbe Spalte wie in 34d).
- `*Klangfassung*,*Loop Werte*,*Hammer*,*InstrumentCore*,*InstrumentModels*,*Instrument*` 142 / 142 unter Release-clang;
  `loop_werte.txt` und Flügel-Bezug unverändert.
- `*Instrument Quellen*` (3 Fälle): grün; rot an ND-Quelle, unbekanntem Schlüssel, fehlender Quelle, unbekanntem Kern.
  `*Instrument Stimmen*` (4 Fälle): grün am Prüfkern; rot bei geänderter Eingabe der Stimmung und bei verstelltem
  Bezug; über die mitgelieferten: 1 Kern-Schicht, 0 mit gestimmten Werten, 0 mit Bezug.
- `InstrumentTuneTool` auf einer Kopie von `grand_piano.lvinst`: `--check` „up to date“, `--reference` „nothing to do“.
- `.gitignore`: `git check-ignore -v` — Begleitdateien unter `daten/`/`aufnahmen/` nicht ignoriert (Ausnahmen), eine
  `.wav` dort ignoriert, `aufnahmen.md` nicht betroffen.
- **Voller Lauf** (neue Dateifassung, UF17-1 a) 1796 / 1796, 0 Skips unter Release-clang (650 s) und Testing-MSVC (587 s).
- Pakete `instrumentcore-v0.2.0` (mit `quellen`, Bezug) und `instrumentmeasure-v0.2.0` (mit `Stimmen`) neu: `sha256sum -c`
  grün, jede Datei LF.

## 5. Stufe 34g — Klasse 4, der Kern `bar` (S178)

Teilplan 34g-1 bis 34g-7 im Konzept §6 (KS1 a–KS4 a). Quellen: LumiViz `master` `88e3951c` (nach 34f), IC `bb7e6d6`
(die Dateien der Klasse 4, `gemeinsam/Balken`, `gemeinsam/Reibung` und `werte/abdruck_msvc.txt` sind dort ohne
uncommittete Änderung).

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J31 | Balken | IC `rechnung/gemeinsam/Balken.hpp`, `.cpp` (`bb7e6d6`) | `InstrumentModels/gemeinsam/Balken` (`lumi::kern`) | `biegeModen`, `formAn`, `freiFreiVerhaeltnisse`, `kragBetaL` wortgleich; der IC-Fall „Balken: frei–frei …“ mit den IC-Grenzen in `test_InstrumentModelsStab` | **ohne** `zerlegeSymmetrisch` (seit 34e im Core, J23 — der Balken ruft ihn von dort) |
| J32 | Reibung | IC `rechnung/gemeinsam/Reibung.hpp`, `.cpp` (`bb7e6d6`) | `InstrumentModels/gemeinsam/Reibung` (KS3 a) | `reibwertGalluzzo`, `reibung` wortgleich | IC hat keinen Fall für die Reibung allein (geprüft über den Bogen); **neu** ein Fall: Galluzzo an den Rändern, Haften mit Hysterese, Gleitpunkt als Lösung von x − d + (F_B/z)·μ(x) = 0 (Halbierung rund 3e-11 genau, Newton 1e-13) |
| J33 | Tastenumfang je Instrument (KS1 a) | — (neu in LumiViz; IC trägt den Umfang im Spiel, `StabSpiel::noteMin`) | `Parametertafel::umfangVon`/`umfangBis`, `tastenumfang`, `richteWerte`, `bezugsProbe(tafel, werte)`, `normiereNachTafel`, `loop/KernAnsicht` | — | **Befund und Korrektur:** `richteKurve` setzte bei einer leeren Kurve den Ersatz erst nach dem Klemmen ein — mit einem Umfang je Tafel unauffällig, mit KS1 a nicht idempotent (Kalimba: die Partnerkurven der Marimba bei 36 und 96, beim zweiten Richten auf 50 und 86). Jetzt wird der Ersatz ebenso gerichtet und trägt seine **Quellen** mit (fehlten seit 34f, WZ1 a). Abweichung von IC (dort deckt der Umfang einer Klasse ihre Vorgaben); der Flügel bleibt Bit für Bit (Vorgaben im Umfang der Tafel) |
| J34 | Gestimmte Werte in Reihenfolge (KS2 a) | IK2-11 a | `pruefeTafel`, `eingabenAbdruck` (die gestimmten vor dem Ziel zählen mit), `messung::stimmeUndNormiere` (ein späterer wird mit dem früheren neu gestimmt), `Stimmer::stimme(fabrik, werte, namen)` | — | **geändert:** der Stimmer bekommt die Namen der zu stimmenden Werte — ein früherer gestimmter Wert, der nicht dabei ist, gilt, wie er in den Werten steht (ein von Hand geänderter Pegel trägt die Fenster) |
| J35 | Der Ton eines Stabspiels | IC `rechnung/klassen/04_platte_stab/Stab.hpp`, `Stab.cpp` (`bb7e6d6`) | `InstrumentModels/stab/Stab` (`lumi::kern::stab`), übernommen mit einem Skript (Anker je genau einmal) | `stabModen`, `stabNachklang`, `rechneStabTon` (Stab, Torsion, Partner, Motor, Dämpfer, Tonabnehmer, quadratische Kopplung, Bogen) wortgleich | **geändert:** kein Messcode im Kern (IK2-2 a) — IC maß beim gestrichenen Ton die Tonhöhe mit YIN (`Stab.cpp:489`), hier gibt der Kern den gehaltenen Teil ab (`StabMessung::gehalten`), gemessen wird im Stimmer (`stabSpielHz`, dieselbe Rechnung); `stufeAus` entfällt (`kern::anschlagStufe`); `stimmeBogen` zieht in die Messung (J38); `kennzeichen` entfällt (IK2-9). **Neu am Spiel** (der Ton liest sie nicht): `partnerZielDb`/`partnerZielNote` (KS2 a), `bogenAb` (KS3 a) |
| J36 | Die zehn Vorgaben | IC `rechnung/klassen/04_platte_stab/Vorgaben.cpp` (`bb7e6d6`) | `InstrumentModels/stab/Vorgaben.cpp`, mit einem Skript | jede Zahl, Kurve und Herkunft wortgleich, die Grenzen (`richteStabSpiel`) | **geändert:** `setzePartnerAbstrahl(s, note, 15)` wird das Ziel des Stimmers (`partnerZielNote` 65, Celesta 84); `normiere` → `normiereNachTafel`; `Stimmung.inc` und `__has_include` entfallen (IK2-11 a); die Quellen je Kurve (WZ1 a) aus einer Tafel von Namen in der IC-Herkunft (Rucz, Chapman, Falaize, Soenderbo, Pfeifle, Thomas/Monteil, CCRMA), sonst `lumiviz` — **nicht** La Favre (Torsion „ohne Zahl“), das Patent (Dämpfer „unklar“) und Iowa (nur Ziel einer Messung): die Werte dort sind Einschätzungen. Die Herkunftstexte bleiben deutsch wie in IC (im Creator im Tooltip). Der Wurlitzer erbt in IC die konstanten Kurven des Rhodes mit Punkten bei 28 und 100 — gerichtet auf 33 … 96, Werte und Ton gleich |
| J37 | Adapter und Tafel `bar` | IK2-3 a, IK2-4 a | `InstrumentModels/stab/StabKern` | Gruppen nach den Abschnitten von `StabSpiel` (Keys, Bar, Partner, Mallet, Damper, Motor, Pickup, Bow, Output), die Grenzen aus `richteStabSpiel`, 57 Werte; Spuren `expression`, `motor`, `bowed`, `malletOffset`, `damperPressure`; innere Größe `contact` | **gewählt:** der Pegel des Partners unten 1e-6 statt 0 (logarithmisch; der Rhodes trug 0 — er wirkt dort nicht, der Tonabnehmer ist magnetisch, der Stimmer schreibt 1 ohne Wirkung); der Dämpfer greift am Ende der Note (wie der Flügel; Sustain in der Planung); ohne Spur `motor` kein Motor, die Phase der Klappen beginnt bei 0 (gleiche Noten bleiben gleiche Töne); `nachklang` nach dem Ende der Note, unter einem Dämpfer höchstens seine Abklingzeit. Die L11-Regel (κ/κ_c nicht in 0,95 … 1,05) bleibt in `spielAus` |
| J38 | Der Stimmer `bar` | IC `Vorgaben.cpp` `setzePartnerAbstrahl`, `Stab.cpp` `stimmeBogen` (`bb7e6d6`) | `InstrumentMeasure/StabStimmen` (`stabStimmer`, `stabPartnerPegel`, `stabBogenFenster`, `stabSpielHz`); `InstrumentMeasure` hängt seither an `InstrumentModels` (BK1 a) | beide Rechnungen wortgleich; der Pegel mit dem Bezug 1, die Fenster mit dem Bezug nach `normiere` — wie in IC | Instrumente ohne akustischen Partner bzw. ohne Bogen bekommen 1 mit Herkunft „without effect“ (ein gestimmter Wert fehlt nie, IK2-11 a). **Ergebnis:** die Fenster des Vibraphons Bit für Bit gleich `Stimmung.inc` (unter Windows gerechnet; in IC von `ic_stimme_stab` erzeugt), 137 s auf 11 Threads |
| J39 | Die mitgelieferten Dateien | KS4 a, Muster `grand_piano` | `asset/loop/instruments/{marimba, xylophone, vibraphone, glockenspiel, tubular_bells, celesta, kalimba, tine_electric_piano, reed_electric_piano}.lvinst`, `test_LoopStab.cpp` (`LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1`), `InstrumentTuneTool --all --reference`, `instrument_katalog.py` | — | **Abweichung vom Teilplan:** statt `InstrumentTuneTool --create` schreibt ein Test die Dateien mit dem Schreiber des Programms (`alsJson`, gerichtet) wie bei `grand_piano` — das Werkzeug kennt den Schreiber der App nicht; es stimmt danach (sein Schreibweg damit erstmals an einem stimmenden Kern geprüft, J30). Die Nachbildungen heißen `synth_marimba`, `synth_vibraphone`, `synth_kalimba` (Skript, zuerst in einer Kopie: 3 Dateien, 7 Verweise in 6 Loops, Generatoren, Katalog, Pegel, README). **Pegel:** `--messung` hätte 26 bestehende Instrumente in der vierten Stelle neu gerundet — eingetragen sind nur die neun (alt = neu), die Nachbildungen messen unverändert −13,40 LUFS. Register: `rucz2021`, `chapman2006`, `falaize2015`, `soenderbo2024`, `pfeifle2017`, `thomas2014`, `ccrma` aus IC `quellen.md` (nur belegte Angaben, Lizenzen nicht geprüft) |
| J40 | Creator: Body nur am Flügel (CK1 a) | Skizze §6a 1 | `KernAuskunft::koerperAnDerSchicht` (Core), `InstrumentCreator` | — | **Befund:** seit 34d zeigte der Creator bei jeder Kern-Schicht *Body* (grau „none“); die gebilligte Skizze verlangt ihn nur, wo der Körper außerhalb des Kerns liegt — der Stab rechnet seinen Partner selbst (IK2-10 a). Jetzt Core, Level beim Stab; Core, Body, Level beim Flügel |
| J41 | B6 Modalfit am Modell | IC `rechnung/messung/test_messung.cpp:119–149` (`bb7e6d6`) | `test_InstrumentMeasure` | der Fall mit dem Xylophon ohne Rohr, denselben Grenzen | in 34e zurückgestellt (J21), weil er den Stab braucht |

**Wächter 34g** (S178):

- **Bit für Bit wie IC:** die 11 Abdrücke aus IC `werte/abdruck_msvc.txt` (je Instrument die mittlere Note bei Stufe −12,
  das Vibraphon mit Motor, und das Vibraphon gestrichen an 72) aus den gestimmten und normierten Werten — im Kern-Fall und
  aus den mitgelieferten Dateien (`test_LoopStab`); die Fenster des Bogens am Modell gestimmt gleich `Stimmung.inc`
  (`LUMIVIZ_34G_STIMMEN=1`, 156 s, und vom Werkzeug beim Schreiben). Die IC-Fälle aus `test_stab.cpp` grün, das Vibraphon
  gestrichen 18 von 18 auf ±10 Cent (IC verlangt 90 %).
- `loop_werte.txt`: 380 Einträge gleich (9 unter neuem Namen), 28 neu (die neun Stabspiele, `bars_demo`) — nur geänderte
  Zeilen (Steckbrief). Ein neuer Schreiblauf hätte 8 Loops ohne Stabspiel in der letzten Stelle geändert (`effects_demo`
  8,7e-4 relativ, im Tausendstel des Wächters): die Datei ist von S161, die Abweichung älter als 34g — `klangfassung.txt`
  (zuletzt in S177 geschrieben) zeigt für dieselben Loops dieselben Zahlen. `klangfassung.txt`: 381 Werte gleich, neue
  Abdrücke nur bei den drei umbenannten und den sechs Loops mit ihnen, 28 neu.
- Flügel-Bezug, `*Loop Werte*`, `*Klangfassung*` nach dem Core-Umbau (34g-2) gleich.
- **Rechenzeit** (Release-clang, nach der Taktprobe — die feste Last 0,25 s, Spanne 1,02 —, je dreimal allein; der erste
  Lauf nach einer gedrosselten Probe verworfen): schwerster Ton das Vibraphon mit Motor an C3, **2,07–2,17 %** der Echtzeit,
  **0,67–0,70 × Flügel-Kern** (A0, 4 s gehalten, 3,0–3,2 %); gestrichen an E♭4 0,60–0,62 %; alle übrigen unter 0,28 %.
  `*31b-3 Rechenzeit*` im Bass 2,76–2,93 %. Grenze 4,6 % (IK2-6 a) eingehalten. IC (S172, Windows-clang): Vibraphon 2,21 %.
- Pegel: die neun auf −13,4 LUFS gesetzt (gemessen −19,1 … −7,9 LUFS bei Volume 1); `*Loop Pegel 27c*` grün.
- Eng 200 / 200 unter Release-clang und Testing-MSVC (auch die Abdrücke unter MSVC Bit für Bit — die Bits hängen an der
  Plattform). Pakete `instrumentcore-v0.3.0`, `instrumentmodels-v0.2.0`, `instrumentmeasure-v0.3.0`: Summen gut, jede Datei
  LF, Modul-Doku darin; `instrumentmeasure` verlangt `InstrumentModels` vor sich.
- **Voller Lauf** (Core geändert, UF17-1 a) 1815 / 1815, 0 Skips unter Release-clang (9:15 min) und Testing-MSVC (9:01 min).
- Hörproben `hoertest/listen/stab_34g` im Format von HörTest_Helper: `Hoertest.exe --liste-pruefen` „Liste gültig“, 96
  Klänge auf −27,2 LUFS; Hinweise: 5 Töne still (wie IC: 10 von 108), Kalimba gleich über die Stärken (linear).

## 6. Stufe 34h — Klasse 5, der Kern `membrane` (S179)

Teilplan 34h-1 bis 34h-7 im Konzept §6 (TM1 a–TM4 a). Quellen: LumiViz `master` `7a6492dc` (nach 34g), IC `bb7e6d6`
(`gemeinsam/Membran`, `klassen/05_membran/` und `werte/abdruck_msvc.txt` sind dort ohne uncommittete Änderung).

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J42 | Membran | IC `rechnung/gemeinsam/Membran.hpp`, `.cpp` (`bb7e6d6`) | `InstrumentModels/gemeinsam/Membran` (`lumi::kern`), mit einem Skript (Anker je genau einmal) | `fellModen` (Galerkin, Luftlast nach Rayleigh, Kessel als Luftfeder, zwei Felle gekoppelt), `formAn`, `besselNull`, die Hilfen `cholesky`, `verallgemeinert`, `luft` wortgleich; der IC-Fall „ohne Luft die Bessel-Verhältnisse“ mit den IC-Grenzen in `test_InstrumentModelsMembran` | **geändert:** nur Kopf, Includes und Namensraum — `zerlegeSymmetrisch` aus dem Core (J23) statt aus dem Balken. **Neu** ein Fall: die Nullstellen j_01, j_02, j_11, j_21 gegen die Tafel (10⁻¹²), jede Mode am Rand still, m > 0 in der Mitte still, die Form auf den größten Betrag 1. Die Fälle Pauke gegen Packer, Paar zweier Felle und Tabla harmonisch brauchen die Vorgaben — sie kommen mit 34h-3. `Fell::belag` bleibt eine Funktion (die Tafel trägt ihn als Werte, 34h-3) |
| J43 | Bezug aus IC | IC `werte/abdruck_msvc.txt` (zwei Abdrücke der Klasse 5), die IC-Quellen der Klasse 5 (`bb7e6d6`) | `docs/modellierung/anbindung-ic/abdruck_trommel.cpp` mit `.bat`, `tests/unit/UnitTests/werte/trommel_ic.txt` | die zwei IC-Abdrücke (Tom mf, Pauke an 52 mf) — vom Programm gleich `abdruck_msvc.txt` gerechnet | **neu:** IC führt die übrigen nicht; das Programm rechnet sie mit den IC-Quellen unter Windows-clang, ohne das IC-Repo zu ändern: je Instrument ff an der Mitte seines Umfangs (der Weg des Adapters bei Stufe 0, TM1 a — tempoPp·(tempoFf/tempoPp)¹ ist bei allen sieben genau tempoFf), die Spielweisen der Snare bei ff (Mitte, Rand, Randschlag, Reifen, gedämpft, ohne Teppich, Wirbel 0,8 s), die sieben Bezüge als Bits. **Befund:** die Probe von LumiViz (1 s) gibt bei allen sieben dieselben Bits wie IC `normiere` (0,5 s). **Befund 2 (Plattform):** zuerst mit `clang++ -O2` und statischer Laufzeit gerechnet — dort wich die Bongo ff in einem Wert (7525) um ein float-ULP ab; MSVC `/MD` und der LumiViz-Bau unter clang (`-O3 -ffp-contract=off`, dynamische ucrt) geben dieselben Bits, clang `-O2` und `-O3` mit statischer Laufzeit dieselben anderen — die statische ucrt rechnet eine Funktion anders. Der Beleg baut deshalb mit MSVC `/MD`, der Plattform von `abdruck_msvc.txt` |
| J44 | Der Ton einer Trommel | IC `rechnung/klassen/05_membran/Trommel.hpp`, `Trommel.cpp` (`bb7e6d6`) | `InstrumentModels/trommel/Trommel` (`lumi::kern::trommel`), übernommen mit einem Skript (Anker je genau einmal) | `bereite`, `rechneTrommelTon` (Schlag mit Hertz-Kontakt, Gleiten nach Marogna et al., Teppich, Rauschen über `bisHz`, Klick des Reifens, Wirbel, Prellen, Pauke auf der Note) wortgleich | **geändert:** `kennzeichen` entfällt (IK2-9); `TrommelAnschlag::tempo` (> 0 setzt die Geschwindigkeit von außen, TM1 a); `TrommelAnschlag::pitch` — mit Punkten folgt das Glissando der Spur (ohne die IC-Rampe `glissandoCent`, die bleibt für die IC-Fälle); der Belag als Werte `Belag`/`belagFunktion` (IC: eine Funktion am Fell, nicht in Daten zu fassen — `belagFunktion` rechnet dieselbe Formel mit denselben Operationen, die Tabla Bit für Bit wie IC). **Neu:** `trommelNachklang` (IC kennt keinen): die langsamste Mode bis −60 dB nach den Schlägen, dazu Kontakt und Hüllen des Rauschens |
| J45 | Die sieben Vorgaben | IC `rechnung/klassen/05_membran/Vorgaben.cpp` (`bb7e6d6`) | `InstrumentModels/trommel/Vorgaben.cpp`, mit einem Skript | jede Zahl und jeder Kommentar wortgleich, die Hilfen `stock`, `paukenschlaegel`, `fell` | **geändert:** `normiere` → `kern::normiereNachTafel` (zwei Runden); die Vorgaben rufen `bereite` nicht mehr (sie bleiben Daten — sonst rechnete jeder erste Zugriff auf `kerne()` sieben Mal die Moden; der Kern rechnet sie beim Bau). Die Kennzeichen (IK2-9 entfällt das Feld) trugen Hinweise, die hier stehen, damit nichts verloren geht: Tom „Tom-Tom-Datensatz lokal“, Pauke „VCSL/VSCO lokal“, Bassdrum „Werte offen; DrumGizmo lokal“, Snare „Teppich vereinfacht; DrumGizmo lokal“, Tabla „Bols als Artikulation offen“, Conga und Bongo „offener Kessel ohne Moden-Daten“ |
| J46 | Adapter und Tafel `membrane` | IK2-3 a, IK2-4 a, TM1 a, TM2 a, TM4 a | `InstrumentModels/trommel/TrommelKern` | Gruppen nach den Abschnitten von `TrommelSpiel` (Keys, Heads, Shell, Stroke, Glide, Snares, Noise, Roll, Output); die Felle als Liste (ein oder zwei, je mit Belag); Bezug in zwei Runden; Spuren `stroke`, `roll`, `snaresOff`, `pitch`; innere Größen `contact`, `glide`; `vorbereitungTeuer` (die Moden 0,05–0,3 s je Instrument) | **gewählt:** die Grenzen der Tafel (IC hat für die Klasse 5 kein `richte…`) weit um die Vorgaben, Richten ändert keine Vorgabe; `tempoPp` nicht in der Tafel (TM1 a); die Stufe 0 gibt genau tempoFf wie IC mit `staerke` 1 (bei allen sieben ist tempoPp·(tempoFf/tempoPp) genau tempoFf) — so sind die ff-Abdrücke über den Kern Bit für Bit; eine feste Trommel spielt auf jeder Note denselben Ton (die Kit-Rolle gibt Note 36); ohne Wirbel ist die Länge der Note ohne Wirkung (kein Dämpfer) |
| J47 | Die mitgelieferten Dateien | TM4 a, Muster 34g (J39) | `asset/loop/instruments/{low_tom, timpani, bass_drum, snare_drum, tabla, conga, bongo}.lvinst`, `test_LoopTrommel.cpp` (`LUMIVIZ_TROMMEL_INSTRUMENTE_SCHREIBEN=1`), `InstrumentTuneTool --reference`, `instrument_katalog.py` | — | Geschrieben vom Programm, der Bezug vom Werkzeug (zwei Runden; die Zahlen gleich IC, J43); die Tastatur ohne Pedale im Umfang des Instruments. Katalog: Tom, Bassdrum, Snare unter *drums* (Pegel über die Kits), Pauke, Tabla, Conga, Bongo unter *percussion*. Die Nachbildung heißt `synth_conga` (Skript, zuerst in einer Kopie: Datei, `genres/afrobeat`, `make_genres.py`, Katalog, Pegel, README, Test; `flute_tropics` spielt das Sample `lib:conga_high` — die Vorlage S179 nannte es irrtümlich). **Pegel:** nur die vier neuen Schlagwerke eingetragen (Bongo 4,5714, Conga 5,3549, Tabla 4,0425, Pauke 2,0289; gemessen −26,6 / −28,0 / −25,5 / −19,5 LUFS bei Volume 1, alt = neu), `synth_conga` unverändert −13,40 LUFS. Register +9 aus IC `quellen.md` (`avanzini2010`, `marogna2010`, `bilbao2014`, `torin2014`, `packer1993`, `wagner2006`, `dahl2011`, `sathej2008`, `miura2023`; nur belegte Angaben, Lizenzen wie in IC, „report only“ nicht geprüft). **`loop_werte.txt`:** 21 Zeilen neu bzw. ersetzt (`conga` ist jetzt der Kern), 3 unter neuem Namen mit gleichen Zahlen; die 8 alten Loops, die ein Schreiblauf im Tausendstel ändert (seit S161, J39), bleiben. **`klangfassung.txt`:** dieselben Zeilen; **Befund:** die Abdrücke der Eingaben von `kalimba`, `reed_electric_piano` und `bars_demo` waren seit S178 veraltet (die Dateien seither unverändert, der Wächter übersprang sie, weil die Eingaben nicht passten) — neu geschrieben, die Zahlen gleich; seither 429 Einträge verglichen, 0 anders. **Offen:** `CREDITS.md` — die Quellen der Membran sind keine Kurven, das Skript sieht sie nicht (J29); Entscheid QM1 |
| J48 | Spielweise an Drum-Spur und Kit-Rolle (TM2 a) | — (neu in LumiViz) | `loop/Loop` (`KernSpurWert`, `richteKernSpuren`, `kernSpurenAlsJson`/`…AusJson`, `TonAngabe::kernSpuren`, `Spur::kernSpuren`, Tonname `:k<name>=<wert>,…`), `.lvloop` **Fassung 18** (`coreTracks` neben `instrument`, immer geschrieben), `loop/DrumKit` (`KitKlang::kernSpuren`, `.lvkit` **Fassung 2**), Kitwechsel, `spurAusKitKlang`, `weichtAb`, `kitAusMuster`, `KitFenster::alsSpur`, `loop/Instrument` (die Werte als Spuren mit einem Punkt bei 0 in den Tonauftrag der Kern-Schicht, vor dem Nachklang) | — | **gewählt:** Namen nur aus Buchstaben (sie stehen im Tonnamen), geordnet, der letzte gilt; ohne Werte derselbe Tonname wie bisher (die Tonspeicher und Abdrücke bleiben). Wechselt eine Spur oder Rolle Klang oder Instrument, fällt die Spielweise weg (sie gehört zum Instrument); bearbeitet der Creator das Instrument, bleibt sie. Ein Kern liest nur die Spuren, die er kennt — ein fremder Name ändert am Ton nichts (Fall). Werte je Schritt und die Bedienung kommen mit 31g. Fassungs-Literale: 16 Stellen `== 17` in den Tests |
| J49 | Kit `modelled_drums`, Demo, Pegel der Kit-Trommeln | TM3 a, 27d (Kits nach R128) | `asset/loop/kits/make_kits.py` (Fassung 2: `coreTracks` an jeder Instrument-Rolle; Schreibweise `snare_drum:kstroke=4`; vorläufig 0,8 bis zur Messung), `asset/loop/make_membran_loops.py`, `membrane_demo.lvloop`, `instrument_pegel.py` | — | **gewählt:** auch die drei Kit-Trommeln (Tom, Bassdrum, Snare) stehen wie jedes Kern-Instrument auf dem Bezug (`level` 4,51 / 4,02 / 4,96); mit `level` 1 hätte die Kick des Kits 4,6 dB über Volume 1 gebraucht. Die Grenze der Spitze für Drums (0,75 / 0,9, aus 27c für die alten `drum_*`) gilt ihnen nicht. `PEGEL` beider Skripte: nur die neuen Zeilen (die übrigen Byte für Byte). Die 14 bestehenden Kits ändern sich nur um Fassung 2 und `coreTracks` (zuerst in einen Kopie-Ordner geschrieben und verglichen). `instrument_pegel.py --loops` lehnte jede Drum-Spur mit einem Instrument aus `PEGEL` ab — jetzt nur bei einem Faktor ungleich 1. **Befund:** Cross-Stick 15 dB unter der Snare (IC-Modell, offen), im Kit gekappt (−4,9 dB), Randschlag −1,6 dB |

**Wächter 34h** (S179):

- **Bit für Bit wie IC:** die 23 Abdrücke und 7 Bezüge aus `werte/trommel_ic.txt` im Kern (`test_InstrumentModelsMembran`)
  und aus den mitgelieferten Dateien (`test_LoopTrommel`), unter Release-clang und Testing-MSVC. Die IC-Fälle aus
  `test_trommel.cpp` mit den IC-Grenzen grün.
- `loop_werte.txt` / `klangfassung.txt`: 22 Zeilen neu bzw. ersetzt (die Trommeln, `membrane_demo`), 3 unter neuem Namen
  (`synth_conga`, gleiche Zahlen); die Abdrücke von `kalimba`, `reed_electric_piano`, `bars_demo` nachgeholt (seit S178
  veraltet); 430 Einträge verglichen, 0 anders.
- **Rechenzeit** (Release-clang, nach der Taktprobe — Spanne 1,02 —, dreimal allein): schwerster Ton die Snare ff,
  **3,09–3,10 %**, **1,01–1,02 × Flügel-Kern** (A0 3,04–3,07 %); Bassdrum 2,76–2,83 %. `*31b-3*` im Bass 2,61–2,67 %.
  Grenze 4,6 % eingehalten.
- Hörliste `hoertest/listen/membran_34h`: 38 Klänge auf −30,9 LUFS, `Hoertest.exe --liste-pruefen` „Liste gültig“
  (0 Fehler, 0 Hinweise); der gemeinsame Pegel in Runden (das Gate macht die Lautheit leiser Klänge maßstabsabhängig).
- Creator mit der Snare: Core · Membrane und Level, *Unmeasured*, die Felle „2 entries“, keine Kurven (Bild gesehen).
- **Voller Lauf** (Dateifassungen, neue Instrumente; UF17-1 a): **1834 / 1835, 0 Skips unter Release-clang (12:50 min)
  und Testing-MSVC (14:32 min)**; rot nur `*Instrument Quellen*` — `CREDITS.md` ohne die Quellen der Werte ohne Kurve
  (J47, QM1). Dabei gefunden: auch Celesta und Wurlitzer rechnen den Filzhammer nach Stulov, `CREDITS.md` nennt ihn nur bei
  `grand_piano` (dieselbe Lücke seit 34g).

## 6a. QM1 a — Quellen am Wert (S180)

| Nr. | Was | Entscheid | Wo | IC | Abweichung / Befund |
|---|---|---|---|---|---|
| J50 | Quellen je Wert ohne Kurve | QM1 a, WZ1 a | `InstrumentCore/Parametertafel` (`QuellenJeWert`, `Werte::quellen`, `quellenVon`, `setzeQuellen`, `Werte::operator==` ohne Quellen; `vorgaben` mit den Quellen der Angaben; `richteWerte` ordnet nach der Tafel und füllt fehlende aus der Angabe), `loop/KernDatei` (`quellenAlsJson`/`quellenAusJson`), `loop/Instrument` (**`.lvinst` Fassung 23**, `coreSources` neben `coreParams`, immer geschrieben, auch leer) | — (neu in LumiViz) | **gewählt:** nur an den Werten eines Instruments, nicht an den Einträgen einer Liste — eine Liste (die Felle, die festen Verhältnisse) trägt ihre Quellen als Ganzes; Quellen zu Kurven und unbekannten Namen fallen beim Richten weg. Nicht im Vergleich und nicht im Abdruck gestimmter Werte (wie die Herkunft einer Kurve): kein gestimmter Wert veraltet. Eine Datei bis Fassung 22 bekommt die Quellen der Tafel. Fassungs-Literale: 2 Stellen (`test_LoopHammerSaite`, `test_LoopInstrument`); `*Klangfassung*` neu geschrieben (nur die Abdrücke, die Werte aller 433 Zeilen gleich) |
| J51 | Quellen je Vorlage | QM1 a | `InstrumentModels/trommel/TrommelKern` (`quellenDerTrommel`), `stab/StabKern` (`quellenDesStabs`), je `Vorlage::quellen` und `werteVon`; die Tafel trägt die der ersten Vorlage (Tom, Marimba) | `Vorgaben.cpp` beider Klassen (gelesen) | Nur, was die Werte des Instruments trägt, gegen `Vorgaben.cpp`, `Trommel.hpp`, `Membran.hpp` geprüft. **Trommel:** Felle d1, d3 (und σ ohne eigene Angabe) Avanzini & Marogna bei allen, dazu Pauke Packer, Snare Bilbao et al., Tabla Sathej & Adhikari; Gleiten Marogna et al. und Avanzini & Marogna bei allen; der Stock (Tom, Snare) Dahl und Wagner, v ff 8 m/s Dahl; Pauke Schlägel Bilbao et al., v ff Torin et al., Kessel Packer; Bassdrum, Conga, Bongo nur Felle und Gleiten. `miura2023` ist nicht mehr benutzt (Prellen 1, nicht aus Miura). **Befund:** die tafelweite Herkunft der Schlägel sagte, auch Filz, Finger und Hand seien an der Kontaktzeit gesetzt — IC `Vorgaben.cpp` setzt nur den Stock so, die anderen „offen“; der Text ist berichtigt. **Stab:** Schlägel Rucz an den sechs Stäben mit `ruczSchlaegel`, Stulov an Celesta und Reed electric piano (fünf Werte), Sønderbo am Tine electric piano (Hunt–Crossley, v bis 4 m/s), Tonabnehmer 1 mm Falaize & Hélie; Ziel des Partners Rucz an den Rohren und der Celesta (übertragen); feste Verhältnisse der Steelpan Thomas et al. Der Flügel aus der Tafel (`undampedFrom` Salamander). |
| J52 | Die mitgelieferten Dateien, ohne neu zu stimmen | QM1 a | `test_InstrumentQuellen.cpp` (`LUMIVIZ_KERN_QUELLEN_SCHREIBEN=1`: liest die JSON, setzt nur `coreSources` und die Fassung), `instrument_katalog.py`, `make_instrument_credits.py` (liest `coreSources`) | — | Der Diff der 17 Kern-Dateien: je `"version": 22 → 23` und der Block `coreSources`, sonst nichts (17 von 17 gelöschten Zeilen sind die Fassung). `CREDITS.md`: 19 Quellen benutzt, neu Avanzini & Marogna, Bilbao et al., Dahl, Marogna et al., Packer, Sathej & Adhikari, Torin et al., Wagner; Stulov jetzt auch bei Celesta und Reed electric piano (die Lücke seit 34g). **Merker:** `instrument_katalog.py` nicht mit `python -I` starten — der isolierte Modus nimmt den Ordner des Skripts aus dem Suchpfad (`instrument_pegel` fehlt dann). |

**Wächter QM1 a** (S180): `*Instrument Quellen*` — jeder Wert der Tafel steht in `coreSources` mit mindestens einer
Quelle aus dem Register, keine aus einer Quelle, die nichts ausliefern lässt; die Quellen jeder Kern-Datei sind die
ihrer Vorlage (`werteVon`, der Flügel `vorgaben`); Richten, Vergleich und Datei an der Pauke; die Fälle der Probe mit
ND-Quelle, unbekanntem Schlüssel, leerer und fehlender Angabe.

## 6b. Stufe 34i — Klasse 1 ohne Flügel (S181)

Teilplan 34i-1 bis 34i-7 im Konzept §6 (KL1 a–KL4 a, KL4-1 a, KL3-1 a, TT1 a). Quellen: LumiViz `master` `63b94e5f` (nach
S180), IC `bb7e6d6` (`klassen/01_hammer_saite/` und `werte/abdruck_msvc.txt` ohne uncommittete Änderung). Bezug der
Abdrücke: `tests/unit/UnitTests/werte/klasse1_ic.txt` (zwölf aus `abdruck_msvc.txt`: fünf `hammer_vorgabe_m*`, sechs
`klasse1_*_m60_a-12`, `saite_Hackbrett_clip_C4_G4`) — kein Beleg-Programm nötig, IC führt sie alle.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J53 | Beleg der Flügel-Vorgabe | IC `test_hammer_ton.cpp` (Anschlag Stufe −12, 0,5 s, Nachklang bis 4 s) | `test_InstrumentModelsKlasse1.cpp` | — | **Befund:** unsere `hammerSaiteVorgabe` gibt die fünf IC-Abdrücke `hammer_vorgabe_m{21,40,60,84,100}_a-12_d500` Bit für Bit — die Vorgabe des Flügels ist in beiden Repos dieselbe (S172 sah nur Unterschiede in Kommentaren). Damit sind die IC-Vorgaben der Klasse 1 übertragbar |
| J54 | Upright, Honky-tonk, CP-70 | IC `klassen/01_hammer_saite/VorgabenRest.cpp` (`bb7e6d6`) | `InstrumentModels/hammer/Vorgaben.cpp`, `HammerSaite.hpp` (`baueVorgabeUpright/Honkytonk/Cp70`), `HammerKern` (`Vorlage`, `vorlagen()`, `werteVon`) | die Rechnungen wortgleich (Länge gekappt auf 1,189 m, µ und B mal (L_F/L_U)², ±1 Hz ab MIDI 41, eine Saite bis 42) | **geändert:** `tastaturUpright`/`tastaturCp70` stehen nicht im Kern — Pedalplätze und `bassSustainBelow` in der Datei, Pedalwerte und Umfang in der Tafel (FU2 a, KL4 a); **neu** die Quellen je Kurve: vom Flügel Übernommenes behält seine (die Zahlen stammen daher), Gerechnetes nennt dazu `yamaha-u1`, die Verstimmung `ptg-honkytonk`, der Chor des CP-70 `smem-cp70`, `modartt-cp80`; die Anschlagstelle 1/8 `lumiviz` (das Patent ist im IC-Bericht nicht genannt). Die drei IC-Abdrücke `klasse1_{Upright,Honkytonk,CP70}` Bit für Bit, direkt und über den Kern; die IC-Fälle zu Mensur, Practice/Soft, ±1 Hz und Chor grün |
| J55 | Umfang und Richten der Kurven | KL4 a, KL4-1 a | `InstrumentCore/Parametertafel.cpp` (`richteWerte` auf den Umfang der Tafel), `stab/Vorgaben.cpp` (`richteStabSpiel`, `kStabUmfangMin/Max`), `saite/Saite.cpp` (`richteSaitenSpiel`), `hammer/HammerKern` (Gruppe *Keys*, `noteMin`/`noteMax`, `werteAus(…, umfang)`), `loop/Tastatur` ohne `range`, **`.lvinst` Fassung 24** | — | **Befund:** `richtePaare` schob jeden Stützpunkt außerhalb des Umfangs auf den Rand — am CP-70 (28–100) wichen danach B an 22 von 73 Tasten bis 95 %, die Länge an 55, der Dämpfer an 62 Tasten ab, der Ton an Taste 60 um −36 dB (Messfall `LUMIVIZ_34I_KLEMMEN=1`). Gerichtet nur auf die Tafel: alle Abdrücke von `bar` (11), `membrane` (23) und Flügel Bit für Bit; `kalimba` und `reed_electric_piano` neu geschrieben und gestimmt — anders nur die Lage konstanter Kurven (28/100 statt 33/96) und die Eingangs-Abdrücke, die gestimmten Werte und der Bezug gleich. Die 17 Kern-Dateien auf Fassung 24 (`LUMIVIZ_KERN_QUELLEN_SCHREIBEN=1` ergänzt fehlende Werte aus der Vorlage und nimmt `keyboard.range` heraus); `grand_piano` mit `noteMin`/`noteMax` 21/108. Fassungs-Literale: 2 Stellen. `klangfassung.txt` zweimal neu (nur Abdrücke) |
| J56 | Der Kern `struckString` | IC `klassen/01_hammer_saite/Saite.hpp`, `Saite.cpp`, `SaitenVorgaben.cpp` (`bb7e6d6`) | `InstrumentModels/saite/` (`lumi::kern::saite`): `Saite`, `Vorgaben.cpp`, `SaiteKern`; Skript `port_saite.py` (Anker je genau einmal) | Tangente, Schlägel, Mittelsteg, Mitschwingen, `rechneSaitenClip`, die drei Vorgaben wortgleich | **geändert:** Includes, Namensraum, `richteSaitenSpiel` auf den Umfang der Tafel (KL4-1 a); `normiere` → `kern::normiereNachTafel` (eine Runde) — **Befund:** die Probe von LumiViz (1 s) gibt dieselben Bits wie IC `normiere` (0,5 s) bei allen drei. **Neu:** `mitQuellen` je Instrument ausdrücklich (nicht nach Namen im Text: das B des Clavinets nennt Gabrielli, der Wert ist offen); Tafel mit zehn Gruppen, die Tonabnehmer als Liste (höchstens zwei), Spuren `damperSlider`, `keyPressure`, `pickupSelection` am Beginn der Note; Mitschwingen nur je Clip (Rechenweg Clip mit 31e/34k). Ausgeliefert nur `clavinet_modal` (KL1 a, KL2 a); die Hackbrett-Vorlage heißt `hammered_dulcimer` (`dulcimer` ist ein LumiViz-Instrument). Die vier IC-Abdrücke (drei Töne, der Clip) Bit für Bit; die IC-Fälle zu Clavinet und Hackbrett grün |
| J57 | Die mitgelieferten Dateien | KL2 a, Muster 34g/34h | `asset/loop/instruments/{upright_piano, honky_tonk_piano, electric_grand_piano, clavinet_modal}.lvinst`, `test_LoopKlasse1.cpp` (`LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1`), `InstrumentTuneTool --reference` (nur das Clavinet; der Flügel-Kern hat keinen Bezug), `instrument_katalog.py` | — | Tastatur nach IC (Upright/Honky-tonk Soft, Practice, Sustain; CP-70 nur Sustain; Clavinet ohne). Der Ton aus der Datei Bit für Bit wie IC, der Bezug des Clavinets gleich IC `normiere`. Register +5 (nur belegte Angaben aus den IC-Berichten). **Pegel** nur die vier neuen Zeilen: 1,8787 / 2,3097 / 2,5073 / 0,5258 (gemessen −18,88 / −20,67 / −21,38 / −7,82 LUFS bei Level 1, alt = neu). `loop_werte.txt` / `klangfassung.txt`: 13 Zeilen neu (zwölf Töne, `keys_demo`), die 8 Loops im Tausendstel bleiben; 443 Einträge verglichen, 0 anders. `CREDITS.md` 25 Quellen |
| J58 | Demo, Hörliste, Creator | 34i-6 | `asset/loop/make_klasse1_loops.py` → `keys_demo.lvloop` (Jazz, zuerst in einer Kopie), `test_LoopKlasse1.cpp` (`LUMIVIZ_34I_WAV=hoertest/listen/klasse1_34i`), Fall „Instrument-Creator 34i-6“ | — | Hörliste 50 Klänge, 44 Punkte (8 Paare: Clavinet modal gegen Waveguide, Upright gegen Flügel, Practice, Tonabnehmer), gemeinsamer Pegel −35,3 LUFS, `--liste-pruefen` gültig. **Befund:** an Taste 103 bei a-30 ist das Honky-tonk vor dem Angleichen genau 1,79 dB (der Pegelunterschied) vom Upright entfernt — die Verstimmung wirkt dort nicht, ab a-12 schon (vermutlich trifft der Hammer leise über `stossLeise` eine Saite; ungeprüft). TT1 a: die Tafel des Flügel-Kerns heißt „Hammer string“ |
| J59 | Körper des Upright | KL3 a → KL3-1 a | — | — | **Befund:** Chaigne, Cotté & Viggiano 2013 geben den P131 nur teilweise mit Zahlen; Steglinie, Bassteg, Stegpunkte je Taste nur als Bild oder gar nicht (ebenso Boutillon & Ege 2013, Ege et al. 2013). Ausgeliefert ohne Boden; Recherche nach einem Bauplan mit Maßen (Bericht unter `modellierung/hammer-saite/`) |

**Wächter 34i** (S181):

- **Bit für Bit wie IC:** die zwölf Abdrücke aus `werte/klasse1_ic.txt` im Kern (`test_InstrumentModelsKlasse1`) und aus
  den mitgelieferten Dateien (`test_LoopKlasse1`), unter Release-clang und Testing-MSVC; alle Abdrücke von `bar`,
  `membrane` und Flügel nach KL4-1 a unverändert.
- **Voller Lauf** (neue Dateifassung, neuer Kern; UF17-1 a): **Release-clang 1850 / 1850 (556 s)** mit einem Skip — der
  Messfall `LUMIVIZ_34I_KLEMMEN` trug `doctest::skip`, danach auf „meldet nur Zahlen“ umgestellt und `*34i*` 14 / 14 grün;
  **Testing-MSVC 1851 / 1851, 0 Skips (681 s)**.
- Bereich `*Instrument*,*Loop Stab*,*Loop Trommel*,*Hammer*,*Klangfassung*,*Loop Werte*,*Kit*,*Fluegel*,*34i*,*Pegel*,
  *Palette*,*Klasse1*` 330 / 330; Creator, Quellen, 34i 95 / 95 nach TT1 a.
- **Rechenzeit ausgesetzt** bis zum Tausch der Kühlung (Patrik S181).

## 6c. LP2 b — Vorgaben je Taste: Wurlitzer, Kalimba, Clavinet (S183)

Anlass: Befund S182 (`2026-11-09_S182_pegel_ueber_den_umfang.md`) — Gefälle der Lautheit über den Umfang aus Bauwerten, die über
die Tasten fest waren. Herleitung mit Quellen und Proben: `2026-11-09_S183_vorgaben_je_taste.md`. Entscheide Patrik 2026-11-09:
**WU1 a, WU2 a** (Wurlitzer), **KA1 a** (Kalimba), **CL1 a, CL2 a** (Clavinet). Neue Quellen unter `…\cmake\quellen\platte-stab\`
(Pfeifle 2017, Chapman 2006, Service Manual 200/200A) und `…\hammer-saite\` (Clavinet-FAQ); Register +7 (Händler, Forum, Manual,
cmuse — alle *nur Bericht*). Erster Fall der Regel aus S182: **die IC-Abdrücke sind Beleg der Übernahme, kein Maßstab** — drei
Abdrücke mit Vermerk neu, die übrigen (Hackbrett, Cimbalom, alle anderen Stabspiele) Bit für Bit.

| Nr. | Baustein | Quelle | Ziel | Übernommen | Verworfen / geändert, warum |
|---|---|---|---|---|---|
| J60 | Wurlitzer: Zunge je Taste, Hammer, Tonabnehmer | Länge je Nummer (DocWurly, Paleophone), Breite/Dicke je Gruppe (Vintage Vibe), Stahl und Hammer (Service Manual S. 2), Grundmode und Ausschnitt der Platte (Pfeifle 2017 S. DAFX-18/21) | `stab/Vorgaben.cpp` (`wurlitzerMasseKg`, `wurlitzerMasse`, `baueVorgabeWurlitzer`), `Stab.hpp`/`Stab.cpp` (`Abnahme::Schlitz`), `StabKern.cpp` (`slot`) | — | **neu statt IC:** Masse der Zunge je Taste aus Länge, Breite, Dicke und dem Lot aus f₀ (3,2 g A1 … 0,2 g C7; IC 2 g fest); Hammer klein und hart (Hunt–Crossley 3 g, p 1,5, 300 N/mm^1,5 — Einschätzung, offen bis M8; IC Stulov an C4, WU1 a zunächst n = 100: der Diskant blieb 14–21 dB unter der Mitte); Tonabnehmer als **Schlitz** C(y) ∝ 1/(1 + ((y − y₀)/T)²), T 1,5 mm, y₀ 0,75 mm (IC Plattenkondensator d = 3 mm klemmte den Bass bei Spitze 1,0). Abdruck `stab_Wurlitzer_mitte_a-12` neu. Spanne LP3 a 34,3 → **10,8 LU**; Helligkeit über die Stärke bleibt (Taste 45 ×3,2, 64 ×1,9, 84 ×1,1) |
| J61 | Kalimba: Zunge je Taste, Zupfen mit Kraft, Kraft am Steg | Chapman 2006 Tab. 1 (gelesen; a·√f₀ = 89–94 cm·√Hz), Federstahl 1,2 × 5 mm (cmuse, nur Bericht) | `Stab.hpp`/`Stab.cpp` (`zupfKraftN`, `Abnahme::Steg`), `StabKern.cpp` (`pluckForce`, `bridge`), `stab/Vorgaben.cpp` (`kalimbaMasse`), `InstrumentMeasure/StabStimmen.cpp` | — | **neu statt IC:** Zupfen gibt eine Kraft (3 N bei Stufe 0, offen), der Weg folgt aus der Steife (IC δ = 1 mm fest → Ausgang ∝ ω²); Ausgang die Kraft an der Einspannung (m·Σ q̈, das Brett strahlt; IC Beschleunigung der Zunge); Masse je Taste 3,6 g (D3) … 1,3 g (D6) (IC 3 g fest). **Befund:** die Stützen von `ratio2` lagen eine Oktave zu tief — Chapmans „D3“ ist 293,7 Hz = MIDI 62 (IC 50/62, jetzt 62/74). Abdruck `stab_Kalimba_mitte_a-12` neu. Spanne 37,4 → **1,4 LU** |
| J62 | Clavinet: Durchmesser je Taste, Tonabnehmer ∝ d² | Stärken je Taste (Vintage Vibe, Electric Keys — nur Bericht), „the first 23 strings wound“ (Gabrielli 2013 S. 3), Dipolmoment ∝ Volumen (Horton & Moore 2009) | `Saite.hpp`/`Saite.cpp` (`durchmesserMm`, μ = ρπd²/4, `aus·d²`), `SaiteKern.cpp` (`stringDiameter` statt `stringMass`), `saite/Vorgaben.cpp` | Hackbrett und Cimbalom: 2r aus Peterson 2005 — **Bit für Bit wie IC** | **neu statt IC:** die Saite trägt ihren Durchmesser, der magnetische Tonabnehmer wiegt mit d² (IC Masse 1 g/m fest „nur Maßstab“); Umspinnung als Stahl gerechnet (Material ohne Quelle, offen). Abdruck `klasse1_Clavinet_m60_a-12` neu. Spanne 22,0 → **18,8 LU**: Bass eben, Senke an den .011″-Saiten, Diskant ∝ f aus festem Tangentenweg — **Ausnahme bleibt (CL2 a)** mit diesem Grund; offen Saitenlängen und Abstände der Tonabnehmer je Taste |
| J63 | Tafeln, Dateien, Register, Pegel | — | `rechenFassung` 2 (`bar`, `struckString`); `LUMIVIZ_STAB_INSTRUMENTE_SCHREIBEN=1`, `LUMIVIZ_KLASSE1_INSTRUMENTE_SCHREIBEN=1`, `InstrumentTuneTool --all --reference` (9) bzw. `--reference` (1), `instrument_katalog.py`, `make_instrument_credits.py` | — | 10 Dateien neu; die sieben unveränderten Stabspiele tragen nur `pluckForce` 3 dazu und neue Abdrücke der gestimmten Werte (die Fassung steht im Eingaben-Abdruck), ihre gestimmten Werte und der Ton Bit für Bit. Pegel der drei: kalimba 0,7627 → 2,5457, reed 1,9335 → 0,6153, clavinet 0,5258 → 0,0951 (`alt` bleibt — die Loops sind mit dem alten Level gemischt, LV1). **Befund:** `instrument_pegel.py --messung --wurzel <Kopie>` schreibt Skript und Dateien trotzdem im Repo (alle Werte neu gerundet, Kommentare weg) — aus HEAD wiederhergestellt, drei Zeilen von Hand, dann `instrument_katalog.py` |
| J64 | Wächter | — | `test_LoopPegel.cpp` (LP3 a: Wurlitzer und Kalimba laufen mit, Clavinet Ausnahme mit Grund CL2 a), `werte/stab_ic.txt`, `werte/klasse1_ic.txt` (Vermerk S183), `loop_werte.txt`, `klangfassung.txt` neu | — | siehe unten |

**Wächter LP2 b** (S183): Release-clang `*Loop Stab*,*Loop Klasse1*,*Instrument Quellen*,*Instrument Stimmen*,*InstrumentModels 34g*,
*InstrumentModels 34i*,*Modelled loops*` 39 / 39; `Loop Pegel LP3 a` grün (15 Instrumente über den Umfang); Testing-MSVC siehe
Report S183. Kein voller Lauf (UF17-1 a: Rechenkern geändert — der volle Lauf folgt mit dem Abschluss der Stufe oder auf Anforderung;
Rechenzeit ausgesetzt bis zum Tausch der Kühlung).

## 6d. HS-3 a — der Hammer nach Kartofelev & Stulov 2014 (S187)

| Nr. | Was | Quelle | Wo | Bit für Bit | Abweichung |
|---|---|---|---|---|---|
| J65 | Hysterese und Steife des Filzhammers, Kontakt 32-fach | Kartofelev & Stulov 2014, Gl. 5 und 7 (Dissertation Kartofelev S. 73, am Bild) | `hammer/HammerSaite.cpp` (`stulovHammerHysterese`, `stulovHammerSteife`), `HammerTon.hpp` (`kHammerUeberabtastung` 32), `HammerKern.cpp` (Rechenfassung 2), `stab/Vorgaben.cpp` (Celesta 32-fach), Register `kartofelev2014` | Masse, p, τ0 wie IC | **neu statt IC:** ε = 0,9894 + 0,000088 N statt 0,9894 − 8,8·10⁻⁵ n² (IC und LumiViz seit HU3-1 c ohne Beleg), Q0 = F0·(1 − ε); Befund `hammer-saite/2026-11-10_S186_befund_leiser_anschlag_diskant.md` §5–§8. Abdrücke `hammer_vorgabe_*` (5), `klasse1_Upright`, `_Honkytonk`, `_CP70`, `_Cimbalom`, `stab_Celesta_mitte_a-12` neu mit Vermerk |

**Wächter HS-3 a** (S187): Release-clang und Testing-MSVC `*Hammer*,*Leiser*,*Stab*,*Klasse1*,*34i*,*34g*,*Instrument*,*Klangfassung*,
*Loop Werte*,*Loop Pegel*,*Modelled loops*,*Loop Spitze*` grün, nachdem die Spielweisen der Celesta an Taste 72 liegen
(HS3-1 a; an 84 −42,6 dB); Abdrücke und Hammer-Bezug unter beiden Compilern gleich. **Voller Lauf 1860 / 1860, 0 Skips unter
Release-clang (789 s) und Testing-MSVC (695 s).** Rechenzeit `*31b-3*` ausgesetzt bis zum Tausch der Kühlung.

## 7. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.10.0 | 2026-11-11 | Session 187. §6d neu: HS-3 a (J65, Wächter). |
| 0.9.0 | 2026-11-09 | Session 183. §6c neu: LP2 b (J60–J64, Wächter). |
| 0.8.0 | 2026-11-09 | Session 181. §6b neu: 34i (J53–J59, Wächter). |
| 0.7.0 | 2026-11-08 | Session 180. §6a neu: QM1 a (J50–J52, Wächter). |
| 0.6.0 | 2026-11-08 | Session 179. §6 neu: 34h (J42 ff.). |
| 0.5.0 | 2026-11-08 | Session 178. §5 neu: 34g (J31–J41, Wächter). |
| 0.4.0 | 2026-11-08 | Session 177. §4 neu: 34f (J27–J30, Wächter). |
| 0.3.0 | 2026-11-08 | Session 177. §3 neu: 34e (J19–J26, Wächter je Teilschritt, Abschluss mit vollem Lauf unter beiden Compilern). |
| 0.2.0 | 2026-11-07 | Session 176. §2 neu: 34d (J12–J18, Wächter). |
| 0.1.0 | 2026-11-07 | Session 175. Angelegt mit 34c (J1–J11). |

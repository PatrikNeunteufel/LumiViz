# Übernahme-Journal — Bausteine aus IC und LumiViz in die Bibliotheken

> **Version:** 0.3.0
> **Datum:** 2026-11-08 (Sessions 175–177)
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

## 4. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.3.0 | 2026-11-08 | Session 177. §3 neu: 34e (J19–J26, Wächter je Teilschritt, Abschluss mit vollem Lauf unter beiden Compilern). |
| 0.2.0 | 2026-11-07 | Session 176. §2 neu: 34d (J12–J18, Wächter). |
| 0.1.0 | 2026-11-07 | Session 175. Angelegt mit 34c (J1–J11). |

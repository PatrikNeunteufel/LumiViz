# Übernahme-Journal — Bausteine aus IC und LumiViz in die Bibliotheken

> **Version:** 0.1.0
> **Datum:** 2026-11-07 (Session 175)
> **Typ:** Journal (Instrumente_IC_Konzept.md IK2-2 a: „jede Übernahme steht in `Uebernahme_Journal.md`“)
> **Status:** wird je Teilschritt fortgeschrieben
> **Bezug:** [../../audio/Instrumente_IC_Konzept.md](../../audio/Instrumente_IC_Konzept.md) §5 (IK2-2 a, IK2-2-1 a), §6 (34c),
> KN1 a, KN2 a
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

## 2. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.1.0 | 2026-11-07 | Session 175. Angelegt mit 34c (J1–J11). |

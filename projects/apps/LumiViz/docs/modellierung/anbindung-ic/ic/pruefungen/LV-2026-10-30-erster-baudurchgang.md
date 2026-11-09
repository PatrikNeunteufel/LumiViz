# Aufgaben an InstrumentsCalculation (IC) — Prüfung des ersten Baudurchgangs aus Sicht von LumiViz

> **Von:** LumiViz Session 166 (Prüfung im Auftrag Patriks, nur gelesen; tragende Stellen IC-A1, IC-A2, IC-A8,
> IC-A11, IC-A16 von LV selbst nachgeprüft). **Datum:** 2026-10-30. **Status:** Aufgaben an IC; wo „Entscheid ja“
> steht, legt IC Patrik eine Vorlage im Format vor. In IC selbst ist nichts geändert.
>
> **Grundlage (gelesen):** IC `master` bei `bb7e6d6` (IC Session 5, lokal Windows) mit `2ef095c` (Merge PR #1) und den
> Cloud-Commits bis `10815fa` — `CLAUDE.md`, `README.md`, `BERICHT_BAU.md`, `AUFTRAG_BAU.md` (ganz),
> `STAND.md` (Einträge Session 5 und 4), `UEBERNAHME_LUMIVIZ.md`, `LEHREN_SETZEN_FLUEGEL.md` (ganz),
> `gemeinsam/hoerschwellen.md`, `gemeinsam/messkette.md` §3b–§4, `rechnung/README.md`, `rechnung/CMakeLists.txt`,
> `rechnung/setzen/Setzen.hpp` und `Setzen.cpp` (Z. 140–310, 379–412), `rechnung/setzen/test_setzen.cpp`,
> `rechnung/werkzeuge/Hoerproben.hpp`, `hoerproben.cpp` (Kopf, `lege`), `TestHilfe.hpp` (Abdrücke, Rechenzeit),
> `rechnung/werte/abdruck_{gcc,clang,msvc}.txt` (verglichen), `klassen/01_hammer_saite/test_hammer_bezug.cpp` (Kopf);
> Stichproben `klassen/Klasse 8 - Rohrblatt und Rohr/offen.md` und `messplan.md`, `Klasse 4/offen.md`,
> `Klasse 6/offen.md`/`modell.md`, Abschnitt „Feinabstimmung und Ausstieg“ in den Messplänen 4, 5, 10; Liste der
> `spielbar = false` im Code; `.claude/journal/2026-10-30.md` (Überschriften).
> LumiViz (heutiger Stand, `bb7859f0`): `Spielweisen_Konzept.md` §4.1 (V2 a bis PV1 a, Teilplan 31d-8 mit „8.5
> Hörproben (S166)“), `Changelog_Session164.md`–`166.md`, `proben/S165_*` (Liste, `kurzfassung.txt` und `probe.txt` von
> `S165_suche2_probe_ohne_a4`), `tests/unit/UnitTests/werte/hoerschwellen.txt`, `test_LoopHammerTon.cpp`
> (Z. 80–87, 640–702), `tools/hoertest/hoertest.py` (Kopf), `hoerproben_pv1/hoertest.json` (Anfang); Vergleich der
> Vorbild-Kopie `gemeinsam/vorbild-lumiviz/code/` gegen LumiViz heute (`diff`).
>
> **Reichweite:** Code nur gelesen, **nicht gebaut und nicht getestet** — Testzahlen (114 Fälle, 1363 Prüfungen) und
> Rechenzeiten sind „nur Bericht“; die 114 `TEST_CASE` sind gezählt. Nicht gelesen: die Kerne der Klassen 2–11 im
> Einzelnen, `FRAGEN_KONSOLIDIERT.md`, `KLASSEN.md` über die Zeilen hinaus, die Journale ganz, die `quellen.md`.
> **Belegkraft:** „gelesen“ = in der genannten Datei selbst gesehen; „nur Bericht“ = aus einem Bericht von IC oder
> LumiViz übernommen; „Einschätzung“ = Schluss des Prüfers, kein Befund.

---

## Übersicht

| Nr. | Thema | Vorrang | Entscheid Patrik |
|---|---|---|---|
| IC-A1 | Setz-Werkzeug: L16 friert die Träger der Schwebung ein | hoch | ja |
| IC-A2 | Hörschwellen nach HW1 a (Format, Zahlen, Belegkraft) | hoch | nein |
| IC-A3 | HW2 a–HW4 a fehlen im Setz-Werkzeug und in den Messplänen | hoch | nein |
| IC-A4 | Glätte des Ziels jetzt am Modell prüfen (ohne Aufnahmen) | hoch | nein |
| IC-A5 | Plan A–D der Messpläne nach der Wende PV1 a / ZZ1 a | hoch | ja |
| IC-A6 | Lehren aus S166 (L19–L21) nachtragen | mittel | ja (wie F-B6 a) |
| IC-A7 | Vorbild-Kopie auf Stand S163, Suche seither geändert | mittel | ja (Auftrag zum Erneuern) |
| IC-A8 | Abdruck fehlt für rund 21 spielbare Instrumente | mittel | nein |
| IC-A9 | MSVC: 29 von 42 Abdrücken abweichend, Ursache offen; kein lokaler clang-Bau | mittel | nein |
| IC-A10 | Rechenzeit: Zahlen aus der Cloud, Grenze relativ, LumiViz-Maß fehlt | mittel | ja (Grenze) |
| IC-A11 | `rechnung/CMakeLists.txt`: GLOB, Namen, Gleitkomma-Schalter | mittel | teils |
| IC-A12 | Übernahme-Architektur: Dubletten zu LumiViz, Namen, Schnittstelle | mittel | ja (nach IK1) |
| IC-A13 | Prüfsumme des Hammer-Bezugs: Verfahren aus LumiViz | niedrig | nein |
| IC-A14 | Hörproben: lokal erzeugen, Format für blinde Tests | hoch | teils |
| IC-A15 | Listen und Merkmale je Klasse für HörTest_Helper | mittel | ja |
| IC-A16 | `dld/` auf master | mittel | ja (Git) |
| IC-A17 | Regeltexte und Berichte nachführen | niedrig | teils |
| IC-A18 | Zwei Zahlen „aus dem Gedächtnis“ | niedrig | nein |

---

## IC-A1 — Setz-Werkzeug: L16 friert die Träger der Schwebung ein (hoch, Entscheid ja)

**Befund (gelesen).** `rechnung/setzen/Setzen.cpp:146` legt jeden Parameter mit `wellig = true` als **nicht frei**
für Levenberg–Marquardt an (`frei = !fest && !wellig`). Er wird nur in der Staffelung bewegt (Z. 225–282: Raster,
goldener Schnitt, danach bleibt er fest). Teilt ein zweiter freier Parameter dieselbe Wahrnehmung (`traegt`, L16),
überspringt Z. 236–240 die Staffelung — der Parameter bleibt dann **am Startwert stehen**, weder gestaffelt noch frei.
Der Test `test_setzen.cpp:139–164` prüft nur, dass „nicht gestaffelt“ im Bericht steht, nicht wo δ endet.

**Warum das ein Problem ist (gelesen, LumiViz `Spielweisen_Konzept.md` §4.1 „A4-Wege (S165)“).** An den sechs
Probetasten war der Weg **ohne Staffelung, beide Träger frei** der beste: Σ r² 2257, hörbar getroffen 60 / 104. Mit
`detune` gestaffelt und fest: 2565 / 50. Staffelung nur als Start, dann frei: 2471 / 55. Beide gemeinsam gestaffelt und
fest: 2320 / 45. δ startet unten: 4325 / 41. IC bildet heute nur „gestaffelt und fest“ ab (der schwächere Weg) und
„eingefroren“ (gar nicht geprüft). L16 sagt „nie einzeln staffeln“, nicht „festhalten“.

**Zu tun.**
1. Parameter mit geteiltem `traegt` in der Suche **frei** lassen.
2. Für die Staffelung (L12) zwischen „fest danach“ und „nur als Start“ wählen lassen — als Option der `Einstellung`,
   nicht still.
3. Einen synthetischen Test mit **zwei echten Trägern** einer Schwebung ergänzen (wie `detune` und δ): prüft, wo beide
   enden, und vergleicht die drei Wege.
4. Bericht `nichtGestaffelt` umbenennen oder ergänzen, damit „frei gesucht“ von „eingefroren“ zu unterscheiden ist.

**Entscheid Patrik:** ja — welche Vorgabe die Staffelung bekommt (aus, nur Start, fest). IC legt das im Format vor; die
Zahlen oben sind die einzigen gemessenen (Flügel, sechs Tasten, ein Startpunkt).

## IC-A2 — Hörschwellen nach HW1 a übernehmen (hoch, Entscheid nein)

**Befund (gelesen).**
- `setzen/Setzen.hpp` (Struktur `Messgroesse`): **eine** symmetrische, absolute Schwelle `hoerschwelle`. Geprüft wird
  `|Messung − Ziel| > hs` (`Setzen.cpp:394`).
- LumiViz `werte/hoerschwellen.txt` (HW1 a) braucht **drei Skalen**:
  - `faktor`, unsymmetrisch, d = ln(Modell/Ziel): kürzer 0,232815 (Faktor 0,7923), länger 0,358304 (Faktor 1,4309);
  - `db`: Knicktiefe 0,5 dB;
  - `ewert`: Schwebungs- und Einbruchtiefe nach Ewert 2002, eine stückweise Regel;
  - dazu die Schwebungsfrequenz mit 6 %.

  Mit der heutigen Struktur lässt sich keine davon richtig darstellen.
- `gemeinsam/hoerschwellen.md:11–14` trägt nur Spannen (0,70–0,79; 1,43–1,50) statt der gesetzten Werte, keine
  Ewert-Regel und keine Schwebungsfrequenz.
- Z. 14 ist falsch: Dort steht, für die Knicktiefe gebe es „keine Zahl“ und LumiViz setze den strengsten Satz. HW1 a
  setzt für den Knick aber 0,5 dB (als Übertragung ausgewiesen). Der strengste Satz betrifft nur die Zeiten.
- Belegkraft dort „nur Bericht“, weil die PDFs in der Cloud nicht erreichbar waren. **Lokal liegen sie** unter
  LumiViz `docs/modellierung/hammer-saite/quellen/`: Tolonen & Järveläinen 2000, Järveläinen 2003, Ewert 2002 (gelesen:
  Dateiliste).

**Zu tun.**
1. Das Format von `hoerschwellen.txt` übernehmen: Größenmuster, Skala, unten, oben, Quelle.
2. `Messgroesse` um Skala und unsymmetrische Grenzen erweitern; die Ewert-Regel als eigene Funktion, mit Test gegen
   die Zahlen von HW1 a.
3. In einer lokalen Sitzung die drei PDFs mit `quelle_suchen.py` nachlesen und die Belegkraft in `hoerschwellen.md` auf
   „nachgelesen“ heben.
4. Je Klasse ausweisen, welche Schwelle übertragen ist (T9 a). Beispiel: das Abklingen einer gezupften Saite gilt nicht
   ohne weiteres für einen gehaltenen Bläserton.

## IC-A3 — HW2 a bis HW4 a fehlen im Setz-Werkzeug und in den Messplänen (hoch, Entscheid nein)

**Befund (gelesen).**
- `GruppenBericht::hoerbar` (`Setzen.hpp:92`) zählt „hörbar **verschieden**“ (|d| > S).
- HW2 a zählt dagegen „hörbar **getroffen**“ bei |d| ≤ max(τ, S). „Eindeutig“ heißt dort: die Startpunkte enden
  höchstens max(τ, S) auseinander.
- `setze` kennt nur einen Startpunkt. Mehrere Startpunkte, „bester je Note“ (A5), die Probe nach HW3 a (Glätte an 5
  von 6, eindeutig an 5/6, besser als die Vorgabe) und der Vorzeichentest mit Holm samt Ausreißern (HW4 a) fehlen.
- Die Messpläne bleiben bei der älteren Fassung:
  - `gemeinsam/messkette.md` §4a, letzter Punkt: „eindeutig … innerhalb dieser Toleranz“ (SP3 a, von HW2 a abgelöst);
  - jeder Messplan, Punkt B: „auf eine Toleranz“ (V2 a, vor HW2 a; Stichprobe Klassen 4, 5, 8, 10);
  - Punkt C: „systematisch“ ohne Test.

**Zu tun.**
1. Die Bausteine aus LumiViz `tests/unit/UnitTests/Hoerschwellen.hpp` und `Probe7512.hpp` lesen und für IC portieren:
   mehrere Startpunkte, „bester je Note“, Probe, Vorzeichentest mit Holm, Ausreißer.
2. Je Baustein ein synthetischer Test.
3. `messkette.md` §4a und den Punkt B/C aller elf Messpläne auf HW2 a bis HW4 a bringen.

## IC-A4 — Glätte des Ziels jetzt am Modell prüfen (hoch, Entscheid nein)

**Befund (gelesen).** Die Probe am Flügel scheiterte an der Glätte: 0 von 6 Tasten auf allen fünf Wegen. PV1 a hält
dazu fest: Das Glätte-Kriterium „hätte vor V2 a geprüft werden müssen“. `messkette.md` §3b verlangt die Prüfung vor dem
Plan, und das Werkzeug `welligkeit` gibt es. In den Messplänen steht sie aber nur als Regel; ein Ergebnis je Klasse
fehlt (Stichprobe 4, 5, 8, 10). Jede dieser Klassen nennt selbst zwei Träger einer Schwebung oder Rate:
- 4: Motor und Resonator;
- 5: Fell und Kessel;
- 10: Musette und Balg (dort ausdrücklich „zwei Träger derselben Schwebung“);
- 11: Leslie.

**Warum jetzt:** Die Prüfung braucht keine Aufnahme. Ein Ziel, das vom Modell selbst erzeugt wird, zeigt schon, ob die
geplante Messgröße (z. B. Anteil bei fester Frequenz) in einem Parameter wellig ist. Das geht auch in der Cloud.

**Zu tun.**
1. Je Klasse die geplanten Messgrößen der heiklen Mechanismen mit synthetischem Ziel vom Modell bauen.
2. `welligkeit` über jeden Parameter fahren, der eine Frequenz oder Rate verschiebt.
3. Ergebnis als Tabelle in `messplan.md` eintragen.
4. Wo es wellig ist, die Messgröße nach §3b (2) ändern, bevor ein Plan mit Glätte-Schwelle beschlossen wird.

## IC-A5 — Plan A–D der Messpläne nach der Wende PV1 a / ZZ1 a (hoch, Entscheid ja)

**Befund (gelesen).**
- Alle elf Messpläne enthalten „Feinabstimmung und Ausstieg (L15–L18)“: einen Anlauf, genau einen zweiten nach A–D,
  „Schwellen stehen vorher fest“.
- Eingetragen ist **keine** Schwelle (Stichprobe 8: Abschnitt endet mit dem Ausstieg).
- Punkt D lautet „Hörproben Bezug | Vorgabe | Suche“, also die Dreierfolge, die sich in S166 als schwer vergleichbar
  erwiesen hat (IC-A6).
- `AUFTRAG_BAU.md` §10 sagt noch „der Plan A–D läuft dort“. Er ist mit PV1 a beendet: kein automatisches Setzen am
  Flügel, Patrik setzt u, γ, δ nach Gehör an 33, 60, 84.

**Was das für IC bedeutet (Einschätzung, keine Entscheidung).** Am Flügel scheiterten beide automatischen Anläufe am
**Ziel**, nicht an der Suche (L13, L16). Für die Klassen von IC fehlen dazu die Bezüge noch ganz.

**Zu tun.**
1. IC legt Patrik je Klasse oder gemeinsam eine Vorlage im Format vor. Zu entscheiden ist, ob der Plan A–D bleibt
   (dann je Klasse Schwellen nach dem Muster HW3 a vorher eintragen). Oder ob die Reihenfolge wechselt: zuerst Hörtest
   der Vorgabe (L5, L6), die Suche nur als Erzeuger von Vorschlägen und nur nach bestandener Glätte-Prüfung (IC-A4).
2. Punkt D in allen Messplänen auf blinde Paare umstellen (IC-A14).
3. §10 in `AUFTRAG_BAU.md` nachführen.

## IC-A6 — Lehren aus S166 nachtragen (mittel, Entscheid ja wie F-B6 a)

**Befund (gelesen).** `LEHREN_SETZEN_FLUEGEL.md` endet mit §6 (L16–L18, S164/S165). Zu S166 steht in IC nichts: die
Suche nach „S166“, „Hörtest“, „ABX“ und „blind“ findet nur L17 (ABX als Empfehlung). Die Wende PV1 a selbst ist in
L16–L18 abgedeckt. Aus S166 (LumiViz Konzept §4.1 „8.5 Hörproben (S166)“, Changelog 166) fehlen drei Lehren —
Vorschlag zum Wortlaut:

- **L19 — Vergleichen statt Folgen:**
  - Drei Fassungen hintereinander (Bezug | Vorgabe | Suche) sind schwer zu beurteilen (Patrik nach `hoerproben_pv1`
    01/02).
  - Besser: blinde Paare, A und B zufällig, jedes Paar läuft, bis weitergegangen wird; je Klang gezielte Fragen auf 1–5;
    Kontrollpaare mit zweimal demselben Klang.
  - Die Fragen entstanden aus Patriks freien Worten („hölele“ → „hohl, einzelner Sinus“). Das spricht für ein erstes
    freies Hören, dann gezielte Merkmale.
- **L20 — Suchgrenze ist nicht Setzgrenze:**
  - Der Vorschlag der Suche lag an A1 mit γ = 41,0° außerhalb der belegten Grenze SQ2 a (0–25°), innerhalb der für die
    Suche erweiterten SG2 a (0–45°).
  - → Setz-Werkzeug: zwei Grenzen je Parameter; Vorschläge außerhalb der belegten Grenze im Bericht kennzeichnen.
  - Heute hat `Parameter` nur `unten`/`oben` (gelesen, `Setzen.hpp`).
- **L21 — Vor dem Hören belegen, dass die Probe das Suchergebnis ist:**
  - Das Werkzeug `LUMIVIZ_PV1_WAV` prüft, dass Kurven und Σ r² je Taste die Ergebnisdatei wiedergeben (576,1 / 114,6 /
    9,8).
  - Der Raum kommt aus der Zeile `optionen` der Ergebnisdatei.
  - → Hörproben aus einer Suche tragen den Punkt und prüfen ihn.

**Zu tun:**
1. L19–L21 in §7 von `LEHREN_SETZEN_FLUEGEL.md` aufnehmen, nach Bestätigung durch Patrik wie bei F-B6 a.
2. Kurzfassung in `VORBILD_FLUEGEL.md`; Kopf und `README.md` (Zeile „L16–L18 aus S164/S165“) nachführen.

## IC-A7 — Vorbild-Kopie auf Stand S163, Suche seither geändert (mittel, Auftrag Patrik zum Erneuern)

**Befund (gelesen, `diff` gegen LumiViz heute).**
- Rechenkern gleich: `HammerTon.cpp`/`.hpp` und `HammerSaite.cpp`.
- Messkette gleich: `Teiltonmessung`, `Strukturmessung`, `Zielsumme`, `Fluegelmessung`, `Modellton`, `NelderMead`,
  `Kurvenabstand`.
- **Geändert:**
  - `LevenbergMarquardt.hpp` (310 Diff-Zeilen, `levenbergMarquardtJeBlock`);
  - `Suche.hpp` (207);
  - `Suchkurven.hpp` (173).
- **Fehlen ganz:** `Hoerschwellen.hpp`, `Probe7512.hpp`, `Schwebungsstufe.hpp`, `werte/hoerschwellen.txt`,
  `werte/hammer_bezug_msvc.txt`; dazu das Konzept §4.1 ab „7.5.13 B1“.

**Zu tun.**
1. Lokale Sitzungen lesen LumiViz direkt.
2. Vor einer weiteren Cloud-Sitzung die Kopie von einer lokalen Sitzung auf `bb7859f0` erneuern lassen (wie in IC
   Session 3) und den Stand im Kopf der `vorbild-lumiviz/README.md` nennen.

## IC-A8 — Abdruck fehlt für rund 21 spielbare Instrumente (mittel, Entscheid nein)

**Befund (gelesen, `werte/abdruck_*.txt`, je 42 Einträge).** `AUFTRAG_BAU.md` §4 Lieferung 3 (f) verlangt „den
Abdruck je Instrument“. Ohne Eintrag sind:

| Klasse | ohne Abdruck |
|---|---|
| 2 | Laute, Oud |
| 3 | Ensemble |
| 5 | Bassdrum, Snare, Tabla, Conga, Bongo |
| 6 | Tamtam, Gong, China, Splash, Ride, Hi-Hat |
| 8 | Sopransaxophon, Tenorsaxophon |
| 9 | Shakuhachi |
| 10 | Harmonium, Melodica, Mundharmonika |
| 11 | `house_organ` |

Gemeint ist: nach den Namen in den Abdruckdateien, gegen die spielbaren Instrumente im `BERICHT_BAU.md` §2. Die Zeile
„Abdruck gcc = clang“ in `STAND.md` und `BERICHT_BAU.md` stimmt für die vorhandenen 42 Einträge; die Abdrücke von gcc
und clang sind gleich (gelesen).

**Zu tun:** Je spielbarem Instrument mindestens einen Ton (Mitte, mf) als Abdruck ergänzen und für gcc, clang und
MSVC schreiben. Bei Klasse 6 dazu die Statistik wie beim Crash. Dann ist jede spätere Klangänderung nachweisbar — der
Zweck des Wächters, wie `klangfassung.txt` in LumiViz.

## IC-A9 — MSVC: 29 von 42 abweichend, Ursache offen; kein lokaler clang-Bau (mittel, Entscheid nein)

**Befund (gelesen, Abdrücke verglichen).** **Bitgleich** mit gcc/clang sind 13:
- Blech: drei;
- Bogen: vier;
- Labialpfeife, Kalimba, Orgel, Akkordeon, Cembalo, Harfen-Clip.

**Abweichend** sind 29: alle der Klasse 1 einschließlich der fünf `hammer_vorgabe`, neun der zehn Stabinstrumente,
Crash, Pauke, Tom, Klarinette, Altsaxophon, Blockflöte, Gitarre, Hackbrett-Clip.

`STAND.md` Session 5 vermutet die Mathematikbibliothek von MSVC; die Ursache ist nicht gesucht (nur Bericht). Gebaut
wurde lokal **nur mit MSVC**. LumiViz misst und prüft Klang unter **Release-clang** in der Umgebung von Visual Studio 18
(`tools\bau_clang.bat`); dafür ist in IC kein Abdruck vorhanden (der von clang stammt aus Linux, clang 18.1).

**Zu tun.**
1. Lokal unter clang in der VS-18-Umgebung bauen und gegen `abdruck_clang.txt` vergleichen (Linux gegen Windows).
2. An einem Ton der Klasse 1 die erste abweichende Stelle suchen und benennen: erster Abtastwert, beteiligte Funktion
   (`exp`, `sin`, `pow` …). Die Klasse 1 eignet sich, weil ihr Kern dem von LumiViz entspricht und LumiViz einen
   MSVC-Bezug hat (`werte/hammer_bezug_msvc.txt`): den Port unter MSVC auch gegen diesen Bezug prüfen (Pegel je Block).
3. Den Befund in `rechnung/README.md` festhalten. Für LumiViz zählt, ob die Abweichung im Rundungsrauschen bleibt; der
   Pegel je Block zeigt das.

## IC-A10 — Rechenzeit: Zahlen aus der Cloud, Grenze relativ, LumiViz-Maß fehlt (mittel, Entscheid ja zur Grenze)

**Befund (gelesen).**
- `werkzeuge/TestHilfe.hpp:99–102`: Grenze = Anteil des portierten Flügels auf derselben Maschine × 0,05 / 0,046.
  Eine Klasse darf also rund 9 % langsamer sein als der Flügel dort.
- Die Tests drucken den gemessenen Anteil und „Grenze hier“.
- `STAND.md` Session 4 nennt „Grenze hier 5,5–7 %“. Daraus folgt rechnerisch: Der Flügel lief auf der Cloud bei rund
  5,1–6,4 %, also über dem LumiViz-Maß von 4,6 % (Einschätzung aus der Formel).
- `BERICHT_BAU.md` §2 sagt nicht, ob die Prozente der Tabelle (z. B. Klasse 8 „≈ 5 %“) absolut auf der Cloud oder
  umgerechnet sind. `rechnung/README.md` nennt die Grenze „rund 5 % … (Flügel in LumiViz 4,6 %)“.
- `test_hammer_ton.cpp:76` prüft den Flügel nur mit `WARN`.
- LumiViz verlangt für die Hammer-Saite: im Bass höchstens 4,6 % unter Release-clang, dreimal allein nach der
  Taktprobe (RZ1-1 a).

**Zu tun.**
1. Lokal unter Release-clang je Klasse den tiefsten Ton dreimal allein nach der Taktprobe messen.
2. Absolut und im Verhältnis zum Flügel in `BERICHT_BAU.md` und `KLASSEN.md` eintragen.
3. Die Einheit der heutigen Tabelle klarstellen.

**Entscheid Patrik:** Gilt für die Instrumente aus IC in LumiViz die Grenze 5 % (`AUFTRAG_BAU` §4.3e) oder 4,6 % wie
für den Flügel (RZ1-1 a)? Und wird sie absolut unter Release-clang geprüft, wie in LumiViz?

## IC-A11 — `rechnung/CMakeLists.txt` für die Übernahme (mittel, Entscheid teils)

**Befund (gelesen).**
- **GLOB:** Z. 51 `file(GLOB … klassen/*/*.cpp)`, Z. 57 `setzen/*.cpp`, Z. 69 die Tests. LumiViz verlangt explizite
  Quelllisten (`sources.mode: explicit`, LumiViz `CLAUDE.md` „Regeln“).
- **Irreführender Name:** `ic_messung` (Z. 56–65) enthält nur `setzen/`. Die Messkette (`messung/Bausteine.cpp`,
  `Samples.cpp`) liegt in `ic_gemeinsam` (Z. 41–44).
- **Tote Bedingung:** `if(EXISTS …/hoerproben.cpp)` (Z. 98) — die Datei ist versioniert.
- **Gleitkomma-Schalter:** Z. 19–23 setzt `/fp:precise /fp:contract-` bzw. `-ffp-contract=off -fno-fast-math` für
  ganz `rechnung/`. In CMakeCraft v0.13.1 (`.cmake`/`.json`/`.txt`) und in LumiViz `Solution.json`/`projects/**`
  kommt keiner dieser Schalter vor (gelesen, Suche ohne Treffer). Die Abdrücke von IC hängen an ihnen.
- **Warnungen:** drei C4996 (`getenv`, `sscanf` in Testhilfen) unter MSVC (nur Bericht, `STAND.md` Session 5).

**Zu tun.**
1. Explizite Listen statt GLOB (ohne Entscheid, sofort möglich). Bibliothek `setzen` und `messung` klar trennen; die
   Bedingung Z. 98 entfernen; die Warnungen beheben.
2. **Entscheid Patrik bei der Übernahme:** Die Gleitkomma-Schalter kommen nach CMakeCraft (je Ziel), oder die Abdrücke
   werden unter den LumiViz-Schaltern neu geschrieben. Das ist eine Frage an CMakeCraft (Sync CC), nicht an IC allein.

## IC-A12 — Übernahme-Architektur (mittel, Entscheid ja, nach IK1)

**Befund (gelesen).**
- **Dubletten:** `rechnung/` enthält Gegenstücke zu LumiViz-Bausteinen:
  - `gemeinsam/Eigenzerlegung` (aus `loop/Eigenzerlegung`), `Fft` (aus `loop/Fft`), `Kurve` (wie `Tastenkurve`);
  - den Port `klassen/01_hammer_saite/HammerTon` (`ic::hammer`; LumiViz `lumi::loop`, gut 1250 Diff-Zeilen zum
    Vorbild);
  - die Messkette `messung/Teiltonmessung`, `Strukturmessung`, `Zielsumme`, `LevenbergMarquardt`, `NelderMead`,
    `Kurvenabstand`.
- **Namen:** die Umgebungsvariablen `IC_SAMPLES`, `IC_ABDRUCK_SCHREIBEN`, `IC_WERTE` gegen die LumiViz-Art
  `LUMIVIZ_*`; Klassenordner „Klasse n - Name“ gegen `modellierung/hammer-saite/` (`UEBERNAHME_LUMIVIZ.md` §2 nennt
  das schon).
- **Schnittstelle:** Die Unterschiede (Stärke als `stufe` dB / `staerke` / `expression`; Note, Kette, Clip;
  `bereite(s)`) stehen in `BERICHT_BAU.md` §4.2.

**Problem (Einschätzung).** Kommen die Dubletten mit, gibt es zwei Rechenwege für dasselbe. LumiViz ist für den Flügel
und die geteilten Bausteine die SSOT; der Port in IC darf nicht als zweiter Flügel-Kern einziehen.

**Zu tun:** Eine Vorlage im Format für den Entscheid nach IK1, je Baustein:
- welche Fassung bleibt;
- in welche Richtung die Abhängigkeit zeigt;
- wie die Adapter für Stärke, Kette und Clip aussehen.

Am besten mit einer Prüfung durch den Agenten `sw-architekt`.

## IC-A13 — Prüfsumme des Hammer-Bezugs: Verfahren aus LumiViz (niedrig, Entscheid nein)

**Befund (gelesen).** `test_hammer_bezug.cpp` (Kopf) prüft die Prüfsumme nicht: Das Verfahren fehle in der Kopie;
FNV-1a, `std::hash` und Polynom 31 trafen nicht. In LumiViz `test_LoopHammerTon.cpp`:
- **Z. 657–671:** FNV-1a 64 Bit, Startwert 1469598103934665603, Faktor 1099511628211, über die Bytes **jedes float in
  `Klang::daten`**.
- **Z. 80–87:** Der Klang kommt aus `rendereTon(instrument, angabe, kSaitenRate)`, nicht direkt aus dem Rechenkern.
- **Z. 680–702:** Das Instrument entsteht aus `bezugInstrument`. Dort sind Ebenen, Stoß und Hysterese auf 0 gesetzt;
  `hammerRelaxationUs` steht auf 1.

Ob `rendereTon` dieselben Abtastwerte liefert wie der Port, ist nicht geprüft.

**Zu tun:** Diese Stellen lesen, den Weg über `rendereTon` nachbilden (oder den Unterschied benennen) und die
Prüfsumme dann prüfen oder begründet weglassen. `BERICHT_BAU.md` §6.2 Punkt 4 nachführen.

## IC-A14 — Hörproben: lokal erzeugen, Format für blinde Tests (hoch, Entscheid teils)

**Befund (gelesen).**
- **Ablage:** `ic_hoerproben [Ordner] [Klasse …]` schreibt nach `<Ordner>/<nn_klasse>/<Instrument>/<Name>.wav`, mono
  (`hoerproben.cpp`, `Sammlung::lege`). Der Ordner ist in `.gitignore` (Z. 379).
- **Namen:**
  - `<Note>_a<Stufe>` für Hammer und Stab (Stufe in dB, z. B. `_a-12`);
  - `<Note>_e<Stärke×100>` für Zupf, Bogen und Bläser;
  - dazu Spielweisen-Suffixe wie `_moderator`, `_abgedaempft`, `_motor<Drehzahl>`, `_gestrichen_e…`,
    `_clip_mitschwingen` / `_clip_ohne`.
- **Liste:** je Instrumentordner eine `liste.txt` mit drei Spalten, durch Tabulator getrennt: Datei; Pegel vorher,
  ggf. „−x dB gegen Übersteuern“; eine freie Frage („glaubhaft?“, „wie ein Satz?“). Rund 46 Aufrufstellen von `lege`.
- **Pegel:**
  - Jede Datei für sich auf −23 LUFS. Die Messung ist eine Näherung **ohne Gating**; LumiViz misst nach R128
    (`audio/Lautheit`).
  - Bei einer Spitze über 0,99 wird **diese Datei** leiser (Z. 64–70) und liegt dann unter −23 LUFS.
  - LumiViz: „jede Fassung auf −23 LUFS, gemeinsam unter −1 dBFS“.
- **Kein Bezug** in den Proben (Aufnahmen liegen lokal), **keine Paare**, keine maschinenlesbaren Felder.
- **Bestand:** Im IC-Ordner liegt keine erzeugte Hörprobe (Suche bis Tiefe 4: kein Ordner `hoerproben*`). Die „rund
  450 WAVs“ (nur Bericht) entstanden in der Cloud und sind nicht da.

**Zu tun.**
1. **Lokal erzeugen** (Release-Bau) und den Ablageort festlegen — z. B. außerhalb des Repos wie LumiViz
   `.claude\handover\…`.
2. Das Werkzeug so umstellen:
   - jeder Klang einzeln;
   - dazu ein **Manifest** im Format von LumiViz `hoertest.json` oder dem künftigen HörTest_Helper-Format (IC-A15);
   - Lautheit nach R128 mit Gating, gemeinsame Spitzengrenze für alle Klänge eines Vergleichs, damit gleich laut gleich
     laut bleibt;
   - je Klang die Kennung des Modells oder Abdrucks, damit ein Urteil einem Rechenstand zugeordnet ist.
3. Die Stärken sind heute auf gleiche Lautheit gebracht. Das steht in der Liste; im Manifest als Feld führen, damit ein
   Test über die Dynamik den Pegel je Stärke wiederherstellen kann.

**Entscheid Patrik:** Wo die Proben liegen; ob IC bis HörTest_Helper fertig ist das LumiViz-Format nimmt (Einschätzung:
vermeidet ein drittes Format).

## IC-A15 — Listen und Merkmale je Klasse für HörTest_Helper (mittel, Entscheid ja)

**Befund.** Was HörTest_Helper braucht (Einzelklang 1–5, Paar A/B, ABX, Reihe/MUSHRA, Merkmale mit Erklärung und
Hörbeispielen), liefert IC heute nicht: freie Fragen je Datei, keine Merkmale, keine Beispiele (gelesen, IC-A14).
Ohne Bezug (für die Klassen 4–11 ist nichts geladen; `BERICHT_BAU.md` §5.1, nur Bericht) gehen heute nur Einzelklang,
Reihe und ABX Modell gegen Modell.

**Was IC liefern müsste (Vorschlag, Einschätzung):**

1. **Je Klasse eine Liste** (ein Manifest je Klasse; je Instrument ein Abschnitt):
   - Klänge mit Feldern Klasse, Instrument, Note, Stärke (einheitliche Skala neben der eigenen), Spielweise, Fassung
     (`vorgabe`, später `bezug`, `suche`), Dauer, Pegel vor dem Angleichen, Rechenstand;
   - die Vergleiche: Paare, ABX-Tripel, Reihen;
   - Kontrollpaare (zweimal derselbe Klang).
2. **Merkmale je Klasse mit Erklärung** (Text, linker und rechter Pol wie `fragen` in `hoertest.json`):
   - **Für alle:** wie das echte Instrument, Klangfarbe dumpf–hell, hohl / einzelner Sinus, Länge.
   - **Je Klasse aus dem heiklen Mechanismus des Messplans** (nur Bericht / Einschätzung, `BERICHT_BAU.md` §3.1
     Punkt 4, Messpläne 4, 5, 8, 10):

     | Klasse | Merkmale |
     |---|---|
     | 1 | Schwebung, Nachklang |
     | 2 | zweistufiges Abklingen, Zupfstelle |
     | 3 | Einschwingen (Kratzen), Vibrato, Wolfton |
     | 4 | Tremolo, Resonator-Schwebung, Härte des Schlägels |
     | 5 | Tonhöhe, Fell gegen Kessel, Knall des Schlags, Teppich |
     | 6 | Aufblühen, Rauschhaftigkeit, Höhen |
     | 7 | Schmettern, Ansprache, Dämpfer |
     | 8, 9 | Ansprache, Register, Atem- und Anblasgeräusch |
     | 10 | Musette-Schwebung, Ansprache im Bass |
     | 11 | Leslie, Klick, Perkussion |

   - Nach L19 erst nach einem ersten freien Hören festlegen.
3. **Hörbeispiele je Merkmal:** Paare, in denen das Modell genau den einen Parameter verstellt. Vorbild ist 8.5 in
   S157 (u, γ, δ in Stufen, Abstand der Fassungen −6 … −41 dB). IC kann sie ohne Aufnahmen rechnen.
4. **ABX für eigene Hörschwellen (L17):** je Klasse für die Größen ohne Literaturzahl (siehe `hoerschwellen.md`) eine
   Reihe von Δ um die übertragene Schwelle. Das ergibt den einzigen Beleg am eigenen Reiz.
5. **Mit Bezug (später, lokal):** Paare Bezug–Vorgabe, Vorgabe–Suche, Bezug–Suche wie `hoertest.json` von PV1. Im
   Manifest die Lizenz des Bezugs (ND/NC nur zum lokalen Hören, nichts weitergeben; `UEBERNAHME_LUMIVIZ.md` §1).

**Entscheid Patrik:** welche Testarten zuerst und welche Merkmale je Klasse; das Format gemeinsam mit HörTest_Helper.

## IC-A16 — `dld/` auf master (mittel, Entscheid ja — Git)

**Befund (gelesen).** `dld/` ist auf master versioniert, mit 13 PDFs. Darunter sind solche ohne CC-Lizenz laut
`BERICHT_BAU.md` §7: Hendrie, `oboes_paper_revised_submitted.pdf` (Mamou-Mani). Der Ordner liegt außerhalb der
Ablage-Regel `klassen/<Klasse>/quellen/`. Die in LumiViz schon vorhandene Ignorier-Regel trifft nur
`docs/modellierung/**/quellen/` (`UEBERNAHME_LUMIVIZ.md` §2) — ein mitkopiertes `dld/` würde in LumiViz **nicht**
ignoriert. `BERICHT_BAU.md` §7 und `STAND.md` sagen „kann weg“.

**Zu tun (Patrik, Git):** `dld/` aus dem Baum nehmen. Wo eine Datei für die Belege bleiben soll, nach
`klassen/<Klasse>/quellen/` mit `SHA256SUMS` und Zeile in `quellen.md` — auch für Dateien ohne CC-Lizenz, im privaten
Repo zulässig nach `README.md` „Ablage und Lizenzen“. Die Historie (rund 2 GB, `60bdc39`, `d965732`) bleibt eine eigene
Entscheidung.

## IC-A17 — Regeltexte und Berichte nachführen (niedrig, Entscheid teils)

**Befund (gelesen).**
1. **Git-Regel widersprüchlich.** IC `CLAUDE.md` Z. 22–24 verlangt nach jedem Teilschritt „committen und pushen“.
   `README.md` Z. 108 und die globale Regel sagen „keine Git-Befehle, die etwas ändern“. Die Ausnahme gilt nur in der
   Cloud (`AUFTRAG_BAU.md` §8). Ob der Commit `bb7e6d6` (Session 5, Autor Patriks lokale Kennung) von Patrik oder von
   einer Sitzung stammt, lässt sich aus dem Log nicht sagen. → Wortlaut in `CLAUDE.md` auf „nur Cloud (§8); lokal liefert
   die Sitzung die Commit-Message“ eingrenzen. **Entscheid Patrik**, weil Regeltext.
2. **Journal fehlt.** Für IC Session 5 steht kein Eintrag im Journal (`2026-10-30.md` endet mit „Sitzungsabschluss IC
   Session 4“) — gegen J-1 a.
3. **`AUFTRAG_BAU.md` veraltet.** Er nennt die Lehren noch als L1–L15 (Z. 6, 44, 84, 186, 205) und in §10 „der Plan
   A–D läuft dort“.
4. **`BERICHT_BAU.md` auf Stand Ende Session 4.** §1 und §3.1 Punkt 6 sagen „gcc und clang bitgleich“ ohne den
   MSVC-Befund; §7 „lokal auch unter Windows testen“ ist mit Session 5 erledigt. → einen Abschnitt „Nachtrag Session 5“
   anfügen.

## IC-A18 — Zwei Zahlen „aus dem Gedächtnis“ (niedrig, Entscheid nein)

**Befund (gelesen).**
- `klassen/Klasse 8 - Rohrblatt und Rohr/offen.md:14`: Tonlochgitter f_c = 1500 Hz, Übergang x⁸/(1+x⁸), „offen
  (Gedächtnis)“, Benade.
- `klassen/Klasse 9 - Luftstrahl und Rohr/offen.md:14`: δ_d = (4/π)√(2hW), „offen (Gedächtnis)“, Verge 1997.

Beide sind als offen gekennzeichnet. `AUFTRAG_BAU.md` §7 sagt aber: „Eine Zahl aus dem Gedächtnis ist keine.“

**Zu tun:** In der Quelle nachlesen oder durch eine belegte Vorgabe ersetzen. Bis dahin im Code als „offen ohne Beleg“
kommentieren.

---

## Geprüft, in Ordnung

- **Lieferungen je Klasse** (`AUFTRAG_BAU.md` §4, gelesen):
  - alle elf Klassen haben `modell.md`, `offen.md` und im `messplan.md` den Abschnitt „Feinabstimmung und Ausstieg“;
  - jede Testdatei einer Klasse enthält Fälle zu Abdruck, Rechenzeit, Grenzen und Threads (gezählt nach Stichwörtern,
    nicht inhaltlich geprüft).
  - Ausnahme: der Abdruck je Instrument (IC-A8).
- **Testzahl:** 114 `TEST_CASE` gezählt — stimmt mit `BERICHT_BAU.md` §1 und beiden `STAND`-Einträgen. Die 1363
  Prüfungen und „grün“ sind nur Bericht.
- **Abdrücke:** gcc = clang in allen 42 Einträgen. MSVC 13 gleich, 29 abweichend — stimmt mit `STAND.md` Session 5.
- **Nicht spielbar:** Gerüste mit `spielbar = false` im Code für Koto, die Blech- und Rohrblatt-Gerüste, Didgeridoo,
  Oboe und die Luftstrahl-Gerüste. Passt zur Einteilung D in `BERICHT_BAU.md` §4.1 (Stichprobe der Fundstellen, nicht
  je Instrument abgeglichen).
- **Befunde stimmen überein** zwischen Bericht und `offen.md`: Hi-Hat (flache Platte, Schalenkontakt nicht gebaut) und
  gestrichenes Vibraphon (+80 … +110 Cent, im `STAND` gerundet +100).
- **Lehren L16–L18** sind in `LEHREN_SETZEN_FLUEGEL.md` §6, `VORBILD_FLUEGEL.md` und `messkette.md` §3b wörtlich
  passend zu PV1 a und den A4-Wegen von S165 übernommen. Es fehlt nur S166 (IC-A6) und die Umsetzung im Code (IC-A1,
  IC-A3).
- **Rechenkern der Vorbild-Kopie** (`HammerTon`, `HammerSaite`) ist bitgleich mit LumiViz heute; LumiViz hat in
  S164–S166 keinen Rechenkern berührt (Changelogs 164–166).
- **Aufnahmen und Pfade:** Kein Pfad zu Aufnahmen im Code (Regel `IC_SAMPLES`, `rechnung/README.md`). Laut
  `STAND.md`/`UEBERNAHME_LUMIVIZ.md` wurden in der Cloud nur doctest und pocketfft (mit SHA-256 und Lizenz) und,
  „nach Freigabe der Hosts“, Aufsätze geladen; keine Aufnahmen (nur Bericht).
- **Ignoriert:** `rechnung/out*/`, `.vs/` und `hoerproben/` stehen in `.gitignore`. `sync/` ist leer.

**Nebenbefund LumiViz (nicht an IC):** `Spielweisen_Konzept.md` PV1 a verweist auf „L16
(`LEHREN_SETZEN_FLUEGEL.md` §5)“ — L16 steht dort in §6.

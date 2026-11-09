> **Aufgelöst (S182, 2026-11-09):** IC ist aufgelöst, dieser Auftrag gilt nicht mehr; erhalten als Geschichte
> (Plan `../Aufloesung_IC_Plan.md`).

# Auftrag: Bau der Rechenkerne aller Instrumentklassen (Cloud-Sitzung)

> **Für:** eine Claude-Sitzung auf GitHub (Cloud, ohne Patriks Rechner) in diesem Repo, gestartet von Patrik; danach
> jede weitere Sitzung, die daran weiterbaut. **Erteilt:** Patrik, 2026-10-29 (formuliert in LumiViz S163). **Ziel:**
> die Berechnungen aller Instrumentklassen aus `KLASSEN.md` als reinen C++20-Code **so genau, wie Quellen und Lehren es
> hergeben**, nach dem Muster des Flügels und den Lehren L1–L15 — während Patrik schläft; die offenen Fragen vorher
> **gesammelt, nach Themen gebündelt und so zusammengefasst, dass Patrik nichts doppelt beantworten muss**, und mit
> der Empfehlung als Vorgabe weitergebaut, bis seine Antworten kommen.
> **Dieser Auftrag löst für den Bau die Regel „Kein Code“ aus [AUFGABE.md](AUFGABE.md) ab.** Alles andere dort
> (Belege, Lizenzen, Ablage, Sprache, Format der Fragen) gilt weiter.

## 0 Was diese Sitzung hat — und was nicht

- **Hat:** dieses Repo vollständig — die Vorab-Recherche aller Klassen (`klassen/`, `gemeinsam/`), die Lehren
  (`LEHREN_SETZEN_FLUEGEL.md`, `VORBILD_FLUEGEL.md`), die Aufsätze unter `quellen/`, und seit 2026-10-29 eine **Kopie
  des Flügel-Vorbilds aus LumiViz** unter gemeinsam/vorbild-lumiviz/ (*Kopie fuer die Cloud-Sitzung, entfaellt mit der Aufloesung (S182)*): Rechenkern
  der Hammer-Saite, Eigenlöser, Messkette, Suche, Steuerdokumente (Spielweisen-, Saiten-, Loop-Konzept). Eine
  Linux-Umgebung mit Compiler und CMake (zu prüfen, §8).
- **Hat nicht:** die Aufnahmen und Messdaten (liegen lokal unter `…\cmake\samples\`, A-1 a) — also **kein Setzen an
  Aufnahmen** in der Cloud, keine Toleranzmessung am Bezug; LumiViz selbst (`github.com/PatrikNeunteufel/LumiViz`,
  privat) — zu Beginn prüfen, ob ein `git clone` nur zum Lesen gelingt; wenn nicht, gilt die Kopie; Patriks Ohr — Hörproben
  werden als Werkzeug gebaut, gehört wird lokal.
- **Die Cloud rechnet, sie lädt keine Aufnahmen** (Patrik 2026-10-29: „für die offline arbeiten geht es um die
  berechnungen, downloads von samples passieren nur lokal“): kein Download von Aufnahmen, Messdaten oder Sample-Sätzen,
  auch nicht freigegebener (G F-D1, K4 F-7 und alle Bezüge aus T7 lädt eine lokale Sitzung). Erlaubt sind nur die
  Externals für den Code (doctest, pocketfft — Freigabe im Startauftrag) und, nach Freigabe, Aufsätze unter `quellen/`.
- **Der Ort der Aufnahmen ist nicht fest:** heute `…\cmake\samples\<Klasse n - Name>\`, Patrik erwägt, sie auf eine
  externe Festplatte zu verschieben und bei Bedarf einzuhängen. Deshalb steht in `rechnung/` **kein Pfad zu Aufnahmen
  im Code**: Werkzeuge, die lokal an Aufnahmen messen oder setzen, lesen die Wurzel aus der Umgebungsvariablen
  `IC_SAMPLES` (oder einem Kommandozeilenargument) und brechen mit einer klaren Meldung ab, wenn sie fehlt oder der
  Ordner nicht da ist; Pfade unterhalb der Wurzel kommen aus der `LADELISTE.tsv` des Datensatzes, die Prüfung aus
  `SHA256SUMS`.
- **Daraus folgt der Umfang:** die **Grundlagen** jeder Klasse (Lehre L6: Grundlagen vor Feinabstimmung) — Rechenkern,
  Instrumente mit Vorgaben aus Quellen, Tests gegen Analytik und Literatur, Rechenzeit, Hörproben-Werkzeug, die
  Messkette an synthetischen Signalen, das Setz-Werkzeug gegen synthetische Ziele. Die Feinabstimmung an Aufnahmen
  (Toleranzen, Setz-Läufe, Hörurteil) bleibt lokal und kommt später.

## 1 Zuerst lesen (in dieser Reihenfolge, mit Zweck)

0. **Zuerst** `gemeinsam/cloud-global/CLAUDE.md` mit `README.md` dort — die globalen Regeln Patriks (Format der
   Fragen, „sauber/richtig“, Git, Pausieren, Belege), die in der Cloud sonst fehlen; sie gelten wie lokal.
1. `README.md`, `CLAUDE.md`, `STAND.md` (Stand und Bilanz), `AUFGABE.md` (Regeln, Lieferungen, Bestand in LumiViz),
   `FRAGEN_KONSOLIDIERT.md` (die Vorgaben für alles Offene, §2).
2. `VORBILD_FLUEGEL.md` ganz — der Ablauf und **alle Lehren L1–L15**; dann `LEHREN_SETZEN_FLUEGEL.md` §5 (Zahlen).
3. `SCHNITTSTELLE.md`, `KLASSEN.md` (Reihenfolge, Aufwand, Risiko), `gemeinsam/README.md`, `gemeinsam/koerper.md`,
   `gemeinsam/rohr.md`, `gemeinsam/messkette.md`, `gemeinsam/offene-fragen.md`, `gemeinsam/lehren-pruefung.md`.
4. Das Vorbild: `gemeinsam/vorbild-lumiviz/README.md`, dann `code/loop/HammerTon.hpp`/`.cpp` und `HammerSaite.hpp`
   (so rechnet LumiViz einen Ton), `code/loop/Eigenzerlegung.cpp`, `code/messung/Teiltonmessung.hpp` (Kopf),
   `Strukturmessung.hpp` (Kopf), `Zielsumme.hpp`, `Suche.hpp` (Kopf), `LevenbergMarquardt.hpp`; `docs/hammer-saite/modell.md`
   und `messplan.md`. Im `docs/Spielweisen_Konzept.md` gezielt: §1 (Zielbild), §4.1 ab „Suche und Vergleich gelaufen
   (S162)“ und die Entscheide, die `AUFGABE.md` „Bestand in LumiViz“ nennt.
5. Je Klasse vor ihrem Bau: ihr `README.md`, `modell-entwurf.md`, `messplan.md`, `offene-fragen.md`, `quellen.md`,
   `aufnahmen.md`; die Berichte unter `recherche/` nur, wo der Entwurf auf sie verweist.

## 2 Schritt 1 — Die Fragen konsolidieren: `FRAGEN_KONSOLIDIERT.md`

> **Erledigt lokal vor der Cloud-Sitzung (IC Session 3, 2026-10-29, Auftrag Patrik):** [FRAGEN_KONSOLIDIERT.md](FRAGEN_KONSOLIDIERT.md)
> liegt vor — 62 offene Fragen in 13 Bündeln (T1, T2, T3a–c, T4–T9, E1, E2), mit den beantworteten Fragen der Klassen
> 1–3 als Vorbild je Bündel (§1a dort). **Die Cloud-Sitzung baut nicht neu, sondern:** liest die Datei ganz (vor dem
> Grundgerüst), übernimmt eingetragene Entscheide Patriks („✅ Entscheid Patrik“), baut sonst nach **a** und führt §3 der
> Datei (Stellen im Code). Findet sie eine offene Frage, die fehlt, trägt sie sie am Ende nach (Punkt 4 unten). Die
> Regeln unten bleiben für das Nachführen gültig.

**Warum zuerst:** Die Klassen 0 und 4–11 haben zusammen über fünfzig unbeantwortete Fragen (`offene-fragen.md` je
Klasse, `gemeinsam/offene-fragen.md`); die Klassen 1, 2, 3 sind beantwortet. Viele Fragen betreffen dasselbe
(Steuerung über Spuren, Rechenweg je Note gegen je Clip, Körper und Luftraum, Bezugsaufnahmen). Patrik beantwortet sie
**nach** dieser Sitzung im Chat des Projekts — die Sitzung darf nicht an ihnen hängen bleiben.

**Lieferung:** eine Datei `FRAGEN_KONSOLIDIERT.md` im Wurzelordner, nach dieser Regel:

1. **Alle offenen Fragen einsammeln** (jede Klasse, gemeinsam) in eine Tabelle: Kennung (Klasse, F-n), Thema,
   Kurzfassung, Empfehlung aus der Frage, ob sie den Bau blockiert.
2. **Nach Themen bündeln**, so dass ein Thema nur **einmal** gefragt wird — je Bündel **eine** Frage im verbindlichen
   Format aus `AUFGABE.md` (Ort, heutiger Wortlaut, Problem, Optionen mit **a (Empfehlung)** vorn, *Warum die
   Empfehlung* mit Kipppunkten), darunter die Tabelle, welche Klassen-Fragen darin aufgehen und was je Klasse
   abweicht. Vorgeschlagene Bündel (anpassen, wenn die Fragen es anders hergeben): **T1** Bauart-Paare und
   Erweiterungen von SF10 a (Zunge als Wind × Stab, Membran, Becken, Wurlitzer, Celesta); **T2** Rechenweg Ton je Note
   vorab gegen je Clip (Tonrad 11 F-1, Mitschwingen 2 F-3, Leslie-Zustand, Pedale); **T3** Steuerung über die fünf
   Spuren (Expression-Weg F-R2, Vibrato F-R3, Legato F-R4, Vokaltrakt F-R5; Balg, Motor, Leslie, Register als
   Passage-Spur; Bogen in Klasse 3); **T4** Körper, Luftraum, Resonator (F-K1, Resonator 4 F-2, Kessel 5, Gehäuse 10);
   **T5** Rohr-Kern und Waveguide-Stücke (F-R1, F-R6) und Register; **T6** Schlagwerk im Drum-Editor (Rollen, Kits,
   Wirbel, Länge am Schritt; 5 F-3/F-4, 6 F-3); **T7** Bezugsaufnahmen, Lizenzen, Downloads (F-D1, je Klasse die
   Bezugsfrage; NC-Quellen) — **blockiert den Bau nicht**, nur das Setzen; **T8** Umfang, Reihenfolge und
   Zurückstellungen je Klasse (Deluxe, „bis zu einer Aufnahme“); **T9** Feinabstimmung nach L1–L15 (Toleranz,
   Hörschwelle, Anläufe) — gilt für alle Klassen gleich.
3. **Die Empfehlung ist die Vorgabe:** gebaut wird nach **a**, bis Patrik anders entscheidet. Jede Stelle im Code, die
   an einer offenen Frage hängt, trägt den Kommentar `// Vorgabe nach FRAGEN_KONSOLIDIERT T3 a (offen)`; die Liste
   dieser Stellen steht am Ende der Datei, damit eine Antwort gezielt nachgezogen werden kann.
4. **Keine Frage doppelt, keine still beantwortet:** eine Klassenfrage, die ein Bündel nicht trifft, bleibt als eigene
   Frage (am Ende der Datei, mit Kennung). Der Bestand in LumiViz hat Vorrang (IC-B1 a): widerspricht eine Empfehlung
   ihm, steht die LumiViz-Stelle mit Wortlaut dabei.
5. **Reihenfolge der Bündel** nach Wirkung auf den Bau: zuerst, was den Kern bestimmt (T1, T2, T4, T5), dann Steuerung
   (T3, T6), dann T8, T9, zuletzt T7.

Erst **nach** dieser Datei beginnt der Bau (Schritt 2). Kommen Antworten während der Arbeit (im Chat oder als Vermerk
„✅ Entscheid Patrik“ in der Datei), werden sie eingearbeitet und die betroffenen Stellen nachgezogen.

## 3 Schritt 2 — Das Grundgerüst `rechnung/`

Ein eigenständiges CMake-Projekt im Wurzelordner `rechnung/`, **reines C++20, ohne Qt, ohne `windows.h`**, baubar
unter Linux (Cloud) und Windows (MSVC, clang); Tests mit doctest, ohne Anwendung. Vorbild für Stil, Namen und
Schnittstellen: `gemeinsam/vorbild-lumiviz/code/` (deutsche Bezeichner ohne Umlaute wie `abklingS`,
`polarisationWinkelGrad`; Kommentare deutsch; je Datei ein Kopf mit Zweck und Entscheiden, wie dort).

```
rechnung/
  CMakeLists.txt                 Projekt, Optionen, Tests (ctest)
  externals/doctest/             doctest.h (MIT; LICENSE beilegen)
  externals/pocketfft/           nur bei Bedarf (BSD-3; LICENSE beilegen), hinter der Fft-Schnittstelle
  gemeinsam/                     Bausteine aller Klassen (unten)
  klassen/<nn_name>/             je Klasse Kern, Instrumente (Vorgaben), Tests, Werkzeuge
  messung/                       die Messkette (Schritt 4)
  setzen/                        das Setz-Werkzeug (Schritt 5)
  werkzeuge/                     Hörproben als WAV, Rechenzeit, Abdrücke
  werte/                         Bezugs- und Abdruckdateien je Compiler
```

**Bausteine in `gemeinsam/` (zuerst, mit Tests):**
- **Kurven über den Tonumfang:** Stützpunkte (Note, Wert), monoton kubisch interpoliert wie `Tastenkurve` im Vorbild
  (linear und logarithmisch), Abweichungen je Note, Grenzen durch Klemmen, Herkunft je Kurve als Text (L2; Z2-Tabellen
  aus `modell-entwurf.md`).
- **Modaler Kern:** Moden als komplexe Pole mit Anteil vom Anreger zum Abnehmer, Impulsinvarianz (Bank 2010 wie im
  Vorbild), Summe der Moden als Ton, Nachklang-Länge aus der schwächsten Dämpfung; gekoppelte Schwinger über den
  **kleinen komplexen Eigenlöser** nach `Eigenzerlegung.cpp` — mit **Erkennung entarteter Eigenwerte** (L11: melden,
  und der Aufrufer vermeidet sie durch Grenzen).
- **Anreger-Schnittstelle** im Zeitbereich (Hammer, Schlägel, Kiel, Bogen, Lippe, Rohrblatt, Strahl, Wind): eine
  Kraft oder ein Fluss je Abtastwert gegen die Auslenkung oder den Druck des Kerns, überabgetastet wo nötig (Vorbild:
  Hammer nach Stulov, achtfach feiner).
- **Körper-Schnittstelle** nach `Koerper.hpp`: Moden mit Einkoppelpunkt und Abstrahlgewicht, Mobilität am Punkt,
  Antwort auf einem Raster; dazu der Baustein **Luftraum** (F-K1, Vorgabe a: Kolben + Helmholtz-Mode, Löcher als
  Ausgänge). Körper aus Bauplänen (Platte mit Rippen, Spectra) sind **offen, lokal** — hier nur Körper aus
  Modenlisten mit Quellenwerten.
- **Rohr-Kern** (für 7, 8, 9 und den Resonator in 4): Moden der Eingangsimpedanz je Griff aus einer Bohrung (F-R1 a),
  Kopplung über Wanderwellen an den Anreger (F-R6 a), Überblenden zwischen Griffen (Maestre 2018).
- **Fft-Schnittstelle**, **WAV-Schreiber** (32-bit float), **Pegel** (RMS, LUFS-Näherung reicht für das Werkzeug; der
  Bezug −13,4 LUFS gilt erst in LumiViz), **Rechenzeit-Messung** (je Ton und je Auswertung, L7), **Abdruck** (SHA-256
  eines Tons als Bitgleich-Wächter je Compiler, wie `klangfassung.txt` im Vorbild).

**Der Flügel-Kern als Prüfstein:** `HammerTon` aus dem Vorbild wird in `klassen/01_hammer_saite/` portiert (Qt-frei
ist er schon; nur die JSON-Datei aus `HammerSaite.cpp` bleibt weg) und gegen `werte/hammer_bezug_saite.json` /
`hammer_bezug_clang.txt` geprüft — bitgleich, soweit der Compiler es zulässt; Abweichungen im Bericht. Das belegt,
dass Kurven, modaler Kern, Eigenlöser und Anreger das Vorbild tragen, **bevor** eine neue Klasse darauf baut. Das
Upright (Klasse 1 Rest, F-1 a bis F-8 a beantwortet) folgt direkt daraus (andere Daten, Boden nach Chaigne 2013 als
Modenliste oder offen).

## 4 Schritt 3 — Die Klassen

**Reihenfolge** nach `KLASSEN.md` (Nähe zum Flügel, Zahl der Instrumente, Risiko): **4 Platte und Stab → 1 Hammer-Saite
(Rest) → 7/8/9 Rohr → 3 Bogen-Saite → 5/6 Membran, Becken → 11 Tonrad → 10 Zunge → 2 Gezupfte Saite (Deluxe)**. Eine
Klasse wird **ganz** gebaut (alle Lieferungen unten) und committet, bevor die nächste beginnt; reicht die Sitzung nicht,
endet sie nach einer vollständigen Klasse mit `STAND.md`. Klasse 0 (Gemeinsam) ist Schritt 2.

**Lieferung je Klasse** (unter `rechnung/klassen/<nn_name>/` und in `klassen/<Klasse n - Name>/`):
1. **Der Kern** nach `modell-entwurf.md` — Anregung, Schwinger, Kopplung, Körper; jede Abweichung vom Entwurf
   begründet im Dateikopf. **Alle Mechanismen eingeschaltet** (L10): was der Entwurf als Klangursache nennt (zweite
   Stufe, Schwebung, Register, Vokaltrakt im Normalfall null), hat einen Parameter mit Grenzen, keine stille Null.
   Schwellen, an denen das Modell seine Struktur wechselt, als Liste mit „gesucht/gesetzt“ (L4, F-L3 a).
2. **Die Instrumente der Klasse** als Vorgaben (`baueVorgabe<Instrument>()`): je Größe eine Kurve oder ein Wert **mit
   Herkunft** — Quelle mit Seite, Rechnung mit Skript, oder **„offen“ mit Vorgabe und Grund** (nie eine erfundene
   Zahl); Grenzen je Größe mit Beleg (wie SQ2 a beim Flügel). Zurückgestellte Instrumente (T8) als Vorgabe mit Vermerk.
3. **Tests (doctest):** (a) analytische Fälle (frei-freier Stab 1 : 2,76 : 5,40; Rohr geschlossen/offen; Membran
   nach Bessel; Saite mit Inharmonizität) auf die Genauigkeit der Rechnung; (b) Literaturwerte aus den Entwürfen
   (Tabellen mit Seite — etwa Rucz & Ulveczki Tab. 1, Russo 2022 Tab. 2, Willemsen Tab. 1); (c) Grenzen klemmen; (d)
   Determinismus: derselbe Ton bitgleich bei Wiederholung und auf mehreren Threads; (e) Rechenzeit je Ton (Grenze
   wie der Flügel: höchstens rund 5 % der Echtzeit im tiefsten Ton, sonst Marke **Deluxe**) und je Auswertung beim
   Setzen (L7); (f) der Abdruck je Instrument in `werte/` als Wächter.
4. **Hörproben-Werkzeug:** schreibt je Instrument WAVs (drei Noten über den Umfang, drei Stärken, mit Spielweisen, die
   die Klasse hat), gleich laut normiert, mit `liste.txt` (Frage je Datei) — Patrik hört lokal (L5); die WAVs kommen
   nicht ins Repo.
5. **`klassen/<Klasse n - Name>/modell.md`** (gebaut, nach dem Muster von `vorbild-lumiviz/docs/hammer-saite/modell.md`:
   je Größe Wert, Herkunft, Grenze, Messgröße, die sie verursacht) und **`offen.md`**: die Tabelle des Offenen (Größe |
   Stand | Quelle | Vorgabe | Frage), nachgeführt bei jedem Schritt (Patrik: „es muss einfach ganz klar notiert sein,
   was noch offen ist“). `README.md` der Klasse und die Zeile in `KLASSEN.md` (Aufwand in zwei Zahlen, F-L4 a)
   nachführen.

## 5 Schritt 4 — Die Messkette (nach Klasse 4 und 1, vor den Bläsern)

`rechnung/messung/`: die Teiltonmessung ohne Qt nach `vorbild-lumiviz/code/messung/Teiltonmessung.hpp` und die
Messung **an der Struktur des Bezugs** nach `Strukturmessung.hpp` (ZF3 a, L4), dazu die Bausteine, die
`gemeinsam/messkette.md` §3 für andere Klassen nennt (Modalfit ohne Reihe für Stäbe und Platten, Einschwingen,
gehaltene Töne, Rauschanteile) — geprüft **an synthetischen Signalen** mit bekannten Werten (Genauigkeit wie 31d-1:
Zeiten 3 %, Pegel 0,5 dB, Schwebung 2 %, dokumentiert je Baustein). Das Zielformat ist das von `salamander_ziel.txt`
und `salamander_struktur.txt` (ein Lese- und Schreibmodul), damit lokale Messungen an Aufnahmen später direkt
anschließen.

## 6 Schritt 5 — Das Setz-Werkzeug nach L8–L15 (gegen synthetische Ziele)

`rechnung/setzen/`: Levenberg–Marquardt mit Grenzen nach dem Vorbild, aber **mit den Lehren**: je Note (unabhängige
Gruppe) eigenes λ, eigene Annahme, eigenes Ende, die Auswertung aller Noten in einem Pool-Aufruf (L8); Wirkung je
Parameter vor dem Lauf, wirkungslose ohne Schritt und als solche berichtet (L9); verrauschte Spalten als Warnung
(L11); gestaffeltes Setzen welliger Größen über einen 1D-Löser (L12); Bericht mit Treffern **und** Rest, Diagonale,
Grenzen, je Iteration eine Datei mit Wiederaufnahme (L7, L14); der Schnitt je Parameter und LM je Note allein als
Werkzeuge (L14); die hörbare Schwelle neben der Toleranz als Feld im Ziel (L13 — die Zahlen kommen aus der Recherche
T9, bis dahin „offen“). Geprüft an synthetischen Aufgaben (ein Modell erzeugt ein Ziel mit bekannten Parametern; die
Suche muss sie wiederfinden — mit und ohne wirkungslose Parameter, mit einer welligen Größe). Der Lauf an Aufnahmen
bleibt lokal.

## 7 Was „so genau wie möglich“ heißt

- **Keine Zahl ohne Herkunft.** Quelle mit Seite (aus den `quellen.md`, selbst nachgelesen, wo die PDF unter `quellen/`
  liegt — mit `quelle_suchen.py` aus `gemeinsam/cloud-global/tools/`, nicht `grep`), Rechnung mit Skript im Repo (`rechnung/werkzeuge/` oder im Bericht),
  oder „offen“ mit Vorgabe und Grund. Eine Zahl aus dem Gedächtnis ist keine.
- **Physik vor Bequemlichkeit** (L10): kein Mechanismus aus Vorsicht abgeschaltet; teuer heißt Deluxe, nicht weg.
- **Nichts still vereinfacht:** jede Vereinfachung gegenüber dem Entwurf ist eine Zeile in `offen.md`.
- **Lehren als Pflicht:** L1–L15 (`VORBILD_FLUEGEL.md`) gelten für jede Klasse; wo eine nicht anwendbar ist, steht
  warum.
- **Empfehlungen nach „sauber/richtig“, nicht nach Aufwand** (globale Regel Patrik) — auch bei den Vorgaben nach §2.

## 8 Regeln

- **Sprache:** Deutsch (Dokumente, Kommentare, Fragen); Bezeichner deutsch ohne Umlaute wie im Vorbild; englische
  Fachbegriffe beim ersten Mal in Klammern.
- **Umgebung prüfen, bevor gebaut wird:** `cmake --version`, `g++ --version` / `clang++ --version`, `ctest`; fehlt
  etwas, in `STAND.md` vermerken und mit dem Vorhandenen bauen (Header-only-Bausteine mit einem kleinen Testtreiber).
  Netz: nur, was die Umgebung freigibt; **kein Download fremder Dateien ohne Freigabe** (Kandidaten in `quellen.md`); **Aufnahmen und Messdaten nie**
  (§0, nur lokal).
- **Git in der Cloud:** die Sitzung arbeitet auf **ihrem eigenen Zweig** und **committet dort nach jedem
  abgeschlossenen Teil** (Grundgerüst, je Klasse, Messkette, Setz-Werkzeug) — sonst ginge die Arbeit verloren. **Kein
  Push nach `master`, kein Merge, kein `--force`;** am Ende ein Pull Request mit der Bilanz aus `STAND.md`. Dies ist eine
  bewusste Abweichung von der globalen Regel „Commits macht nur Patrik“, nur für Cloud-Sitzungen, **von Patrik beim
  Start zu bestätigen**; kein `Co-Authored-By`.
- **LumiViz:** nur lesen (Klon oder Kopie); nichts dorthin schreiben; die Kopie unter `gemeinsam/vorbild-lumiviz/`
  nicht ändern.
- **Determinismus:** keine Zufallszahlen ohne festen Startwert, keine Threads, deren Reihenfolge das Ergebnis ändert;
  Pool nur über Aufgaben, die für sich rechnen (Vorbild `parallelJe`).
- **Zeilenenden:** `.gitattributes` normalisiert (`text=auto`); neue Dateien mit LF; bestehende Dateien beim Ändern in
  ihrer Form lassen.
- **Belege, PDFs, Agenten:** wie `AUFGABE.md` „Regeln“.

## 9 Abschluss jeder Sitzung

- **Journal nach jedem Teilschritt** (J-1 a, Patrik 2026-10-29), nicht erst am Ende: `.claude/journal/JJJJ-MM-TT.md`
  (eine Datei je Tag), je Eintrag Uhrzeit/Teilschritt, was gemacht, Befunde mit Zahlen, Irrwege und warum, Lehren;
  sofort committen und pushen. STAND.md bleibt der knappe Stand, das Journal hält den Verlauf und die Gründe.
- `STAND.md` im Wurzelordner nachführen (oberster Block: Datum, was gebaut und committet ist, welche Tests grün sind,
  Rechenzeiten, was offen ist, welche Fragen in `FRAGEN_KONSOLIDIERT.md` warten, was die Umgebung nicht konnte).
- Je gebauter Klasse `offen.md` und `README.md` aktuell; `KLASSEN.md`-Zeile; `UEBERNAHME_LUMIVIZ.md` je neuem
  fremdem External (doctest, pocketfft: Lizenzdatei beilegen, Zeile in §3).
- Der Pull Request nennt die Klassen, die Tests (Zahl, grün), die Rechenzeiten und die offenen Fragen.

## 10 Nicht Teil des Auftrags

- Der Flügel selbst (wird in LumiViz fertig; der Plan A–D läuft dort) — hier nur als Prüfstein portiert.
- Setzen an Aufnahmen, Toleranzmessung am Bezug, Hörurteile (lokal, nach den Grundlagen; L6).
- Oberfläche, Editor/Creator, Dateiformat `.lvinst`, Integration in LumiViz (Entscheid nach IK1).
- Körper aus Bauplänen (Platte mit Rippen, Netz, Spectra) — Schnittstelle ja, Rechnung lokal.

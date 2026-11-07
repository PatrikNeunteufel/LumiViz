# LumiViz — Instrumente aus InstrumentsCalculation (Konzept)

> **Version:** 0.5.0
> **Datum:** 2026-11-07 (Session 174)
> **Typ:** Konzept — Planung der Anbindung der Rechenkerne aus `InstrumentsCalculation` (IC) an LumiViz;
> Zielbild Z1 bis Z4 und M1 entschieden (Patrik S172), **IK2-0 bis IK2-13 entschieden, IK2-2-1 a, CC1 a, TP1 a**
> (S173); **Teilplan Stufe 34 (34a–34l) in §6**; 34b erledigt (S174, CMakeCraft v0.14.0, §5a), 34a gebilligt (§6a, CK1 a, CK2 a); nächster Schritt 34c
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Spielweisen_Konzept.md](Spielweisen_Konzept.md) (IK1, UF12 a, UF13 a, SP1 a, RF1 a) ·
> [VST3_Konzept.md](VST3_Konzept.md) (Entwurf S123, nichts entschieden) · IC `BERICHT_BAU.md`, `STAND.md`,
> `SCHNITTSTELLE.md`, `UEBERNAHME_LUMIVIZ.md`, Prüfung `pruefungen/LV-2026-10-30-erster-baudurchgang.md`
> (IC-A1 bis A18) · Plan der Sitzung `.claude/handover/plan_S172_anbindung_ic.md` (lokal) ·
> Berichte unter [../modellierung/anbindung-ic/](../modellierung/anbindung-ic/)
> **Sprache:** Deutsch

---

## 1. Ziel

Die Rechenkerne der elf Instrumentklassen aus `InstrumentsCalculation` (`rechnung/`, reines C++20 ohne Qt) werden
über **eine** Schnittstelle an LumiViz angebunden, so dass ihre Instrumente im Loop wie die heutigen spielbar sind
(Creator, Palette, Tonspeicher, Freeze, Pegel, Körper). Dieses Dokument hält das Zielbild, die Entscheide und den
Teilplan; die Herleitungen liegen unter `docs/modellierung/anbindung-ic/`.

## 2. Voraussetzungen (Patrik, Ende S171)

- **Keine Legacy-Rücksicht:** LumiViz ist unveröffentlicht. Eine Option wird nicht schlechter, weil sie
  Dateifassungen, die Ton-Funktion, den Tonspeicher je Ton, `.lvinst`-Felder oder mitgelieferte Instrumente bricht.
  Alte Fassungen weiter lesen oder `_old`-Kopien nur auf ausdrücklichen Wunsch.
- **Bestand weiterverwenden:** was es in LumiViz oder IC gibt (Rechenkerne, Eigenlöser, FFT, Messkette, Tonspeicher,
  Freeze, Klangerzeuger, Bausteine), bleibt, solange ein Neubau keine **deutliche** Verbesserung bringt; je Option
  steht je Baustein, was übernommen, angepasst oder neu gebaut wird, und für jedes „neu“ die Verbesserung als Zahl
  oder Grund.

## 3. Zielbild (Patrik S172, „alles nach Vorgabe“ — je Frage Option a)

✅ **Z1 — Reihenfolge:** Zuerst **Klasse 4** (Platte und Stab) und **Klasse 5** (Membran) als erste echte IC-Kerne
über die neue Schnittstelle; als zweites **Klasse 1** (Upright, Honky-tonk, CP-70, Clavinet) mit den IC-Vorgaben am
LumiViz-Kern der Hammer-Saite (der Port in IC zieht nicht als zweiter Kern ein, IC-A12); danach **Klasse 10**
(Zunge) und **Klasse 2** (gezupfte Saite, je Note). Ketten (Klassen 3, 7, 8, 9) und Clip-Rechnung (11, Mitschwingen
in 1 und 2) später als eigene Teilschritte. Grund: die Schnittstelle wird sofort an fremdem Code geprüft; die
Klassen 4 und 5 haben Prüfsteine und Rechenzeiten von 0,2 bis 3,6 % (IC `BERICHT_BAU.md` §2); der Upright hängt am
Flügel-Kern, dessen Klang noch nicht abgenommen ist (Hörtest PV1 4 / 30). Kippt: will Patrik zuerst ein besseres
Klavier hören, Klasse 1 vor; sollen die Drum-Kits unberührt bleiben, Klasse 4 vor 5 und die Kits später.

✅ **Z2 — Ungemessen ausliefern:** **Ja, nach SP1 a** — Vorgaben aus Quellen; vorher die Hörproben der betroffenen
Klassen bei Patrik im HörTest (Stufe S2 ist gebaut, Listen unter IC `hoertest/listen/`); Kennzeichnung
**„ungemessen“** im Creator und in der Palette; die Messung (IC-A5, Plan A bis D) als spätere Stufe. Grund: dieselbe
Reihenfolge wie beim Flügel (`grand_piano` seit S146, Feinabstimmung läuft); die Feinabstimmung aller Klassen läge
sonst vor jedem Instrument. Kippt: fällt eine Klasse in den Hörproben klar durch, bleibt sie bis zur Messung draußen.

✅ **Z3 — IC bleibt Werkstatt:** Beide Repos bleiben bestehen und bauen für sich; IC bleibt die Werkstatt für
Recherche, Messpläne, Setzen, Hörproben und Cloud-Sitzungen. Wo die Rechenkerne liegen und in welche Richtung die
Abhängigkeit zeigt, entscheidet **IK2-1** mit Zahlen. Kippt: keine weiteren Cloud-Sitzungen gewollt — dann alles
lokal in LumiViz, das IC-Repo nur noch Beleg.

✅ **Z4 — Plugins in beide Richtungen als spätere Ziele:** eigene Instrumente als Plugin nach außen (Hülle um die
eigenen Bibliotheken) **und** fremde Plugins in LumiViz (Host) sind Ziele, je als eigene spätere Stufe. Jetzt wird
die Schnittstelle der Rechenkerne so geschnitten, dass eine Plugin-Hülle sie ohne Umbau nutzt; die Recherche
(`modellierung/anbindung-ic/2026-11-02_S172_recherche_plugins_clap_vst3.md`) belegt, was das heißt. Kein Host und
keine Hülle in der ersten Stufe der Anbindung. Kippt: will Patrik fremde Plugins nie, nur die Hülle.

## 4. Ausgangslage (gelesen 2026-11-02)

- **IC** (`…\cmake\InstrumentsCalculation`, `master` `bb7e6d6`; der Stand von IC Session 5 vom 31.10. ist im
  Arbeitsbaum, nicht committet): `rechnung/` mit `ic_gemeinsam` → `ic_klassen` → `ic_messung`, elf Klassenordner,
  Werkzeuge, 114 Testfälle grün unter gcc, clang (Linux) und MSVC (lokal); nichts gemessen, nichts gehört.
  **IC-A8 bis A12 offen** (Abdrücke für 21 Instrumente, MSVC-Abweichung 29 von 42, Rechenzeit nur relativ aus der
  Cloud, GLOB und Gleitkomma-Schalter in `CMakeLists.txt`, Dubletten zu LumiViz); erledigt IC-A2 bis A4.
- **LumiViz:** IK1 (S151) — der Flügel wird hier fertig, neuer Rechencode ohne Qt und `windows.h`, nur gegen `Fft`
  und `Eigenloeser`; Stufe 31 ruht (31d-8 ff., 31e bis 31h offen, 31g Creator/Editor nach UF12 a); Stufe 32 ist das
  Upright. Heutige Instrumente, die die Einteilung A ersetzen oder ergänzen würde: `piano`, `marimba`, `vibraphone`,
  `bell`, `clavinet`, die Drum-Kits. `VST3_Konzept.md` (S123): Lizenz des VST3-SDK (MIT seit 3.8, Pressemitteilung
  29.10.2025 gelesen), Ablauf, Wege B1/B2, Stufen P1 bis P6 — nichts entschieden.
- **Hörtest** (Sync `sync_hoertest`, Leitung HT): HT hat Stufe S2 gebaut und die elf IC-Listen geprüft (gültig; zwei
  Befunde an IC); LV im Leerlauf, LV-2 und LV-3 offen.
- **IC T2 a** (Patrik, IC Session 3, 2026-10-29) legt den Rechenweg fest: „das Modell rechnet über die Spanne, in der
  der physikalische Zustand lebt, und speichert das Ergebnis vorab mit der Spur im Namen“ — **Note**, **Kette** (ein
  Ton), **Passage-Spur** (Motor, Balg), **Clip** (Tonradorgel, Mitschwingen; Rotary-Zustand über Clips im Kanal).
  LumiViz hat Note und Spur im Namen (SK12 a, Ketten `:p1`/`:p2`); der Clip-Weg ist neu.
- **Architektur** (`modellierung/anbindung-ic/2026-11-02_S172_architektur_rechnung_loop.md`, tragende Befunde in S172
  nachgelesen): zwei Flügel-Kerne ohne Bitgleich-Beleg; `HammerSaite.hpp` zieht Qt in den Kopf des Kerns; der
  Rechenkern hat in LumiViz keine eigene Bibliothek (App-Core, alle Bibliotheken in `Solution.json` INTERFACE); 16
  Ton-Funktionen in 12 `Spiel`-Typen mit je eigener Stärke, Zeit, Tonhöhe und Vorbereitung; zwei pocketfft-Fassungen,
  Gleitkomma-Schalter nur in IC. Dubletten 6.281 Zeilen in IC gegen 6.416 in LumiViz; ohne Dubletten bleiben in
  `rechnung/` 15.638 Zeilen ohne Tests.
- **Plugins** (`…_recherche_plugins_clap_vst3.md`): VST3-SDK 3.8.1 und CLAP 1.2.10 unter MIT; Marke „VST“ mit Pflichten
  (Logo, Zuschreibung), ein neutrales „Plug-in“ nicht; kein Format verspricht gleiche Ausgabe bei gleicher Eingabe
  (REAPER warnt ausdrücklich); kein Vertrag für vorab gerechnete Töne je Note; Host-Bausteine ohne JUCE vorhanden
  (VST3-Hosting-Klassen, `clap-helpers`, `clap-wrapper`).
- **Messung** (`…_messung_clang_windows.md`, M1 a): `rechnung/` unter Windows-clang 20.1.7 gebaut; 125 Fälle, 115 grün,
  alle 29 roten Prüfungen Abdrücke, **bitgleich mit MSVC** (die 13 übrigen bitgleich mit Linux) — die Abweichung hängt
  an der Plattform, nicht am Compiler. Rechenzeit (drei Läufe allein, Taktprobe ruhig): Klasse 4 höchstens 2,21 %
  (Vibraphon), 5 2,40 % (Tom), 1 ohne Deluxe 2,09 % (CP-70), 10 1,40 %, 2 0,20 %; Flügel-Port 3,38 %; Deluxe Cimbalom
  5,34 %, Crash 5,7 × vorab.
- **Zahlen der Loops** (56 mitgelieferte): 229 Instrument-Muster, 249 Instrument-Clips, 6836 Noten, 2605 verschiedene
  Töne (ein Ton 2,6-mal wiederverwendet); nur in sieben Loops liegt ein Muster in zwei Clips.

## 5. Entscheide (IK2-0 bis IK2-13)

*Reihenfolge: IK2-0 (Grundsatz: Form der Schnittstelle), dann IK2-1 bis IK2-13 einzeln. Die Fragen stehen im Plan der
Sitzung (`.claude/handover/plan_S172_anbindung_ic.md` §3); hier kommen die Entscheide mit Wortlaut, Grund, Verworfenem
und Kipppunkt hinein.*

✅ **M1 a** (Patrik S172): Die Messungen aus Plan §2.4 laufen in der LumiViz-Sitzung, nur lesend und bauend (Bauordner
`rechnung/out-clang`, ignoriert), nichts im IC-Repo geändert außer einer Prüfdatei unter `pruefungen/` nach dem Vorbild
S166. Ergebnis in §4.

✅ **IK2-0 a** (Patrik S173, „a nach Empfehlung“): **drei Schichten** — (1) Qt-freie Kern-Bibliothek (STATIC unter
`projects/libs/`, nur gegen `Fft` und `Eigenloeser`) mit **einer** Ton-Schnittstelle (Auftrag mit Notenfolge Note/Kette/
Clip, Stärke, Haltezeit, Spuren als Kurven, Rate → Mono-Puffer) und einer Auskunft je Kern, je Klasse ein Adapter auf
`rechne<X>Ton`; (2) LumiViz nutzt sie direkt im Prozess, vorab (Tonspeicher mit Tonnamen, Clip-Kerne über den
Clip-Abdruck aus Freeze; Stereo, Körper, Pegel aus LumiViz); (3) Plugin-Hüllen und Host als spätere eigene Stufen, nicht
in Stufe 34. Der Flügel-Kern bekommt getrennte Köpfe (Daten ohne Qt, Datei mit Qt). Verworfen: b (je Klasse ein
Sonderweg, `Schicht` und if-Kette wachsen, Hülle müsste 16 Funktionen spiegeln), c (fremde Plugins nicht bestimmt, ohne
Vorab-Rechnung und Tonspeicher, Becken live unmöglich). Nachgeprüft S173: CMakeCraft v0.13.1 baut STATIC mit Quellen
(`LibraryCreate.cmake` Z. 138–170, STATIC ist Vorgabe, `Solution.cmake:240`) — in LumiViz der erste Einsatz, alle
Bibliotheken bisher INTERFACE. Kippt auf b, wenn dieser erste Einsatz scheitert und die Lücke in CMakeCraft nicht
geschlossen wird; Richtung c, wenn fremde Plugins vor den IC-Instrumenten gewünscht sind (Host als Stufe vor 34).

✅ **IK2-1 c** (Patrik S173, gegen die Empfehlung a): **Die Kern-Bibliothek liegt in LumiViz unter `projects/libs/`**,
angelegt mit der Struktur, die CMakeCraft dafür hat. Patrik: „sie können separat erstellt werden und bei Bedarf auch
separat zur Verfügung gestellt werden. gleiche thirdware braucht es dann nur in einem Repo … das sollte Standardvorgehen
sein.“ Folgen: jedes External der Kerne (pocketfft, doctest, Eigen für Werkzeuge) liegt nur in LumiViz; IC bezieht die
Bibliothek von dort — separat gebaut, bei Bedarf als Paket (CMakeCraft `package_<name>`, Archiv mit SHA-256,
`cmake/project/PackageBuild.cmake`; Gegenstück der Archiv-Handler der Externals). Wie die Cloud-Sitzung sie bekommt
(Paket im IC-Repo, Abruf, oder Kern-Arbeit lokal in LumiViz), klärt der Teilplan. Verworfen: a (eigenes Repo; Patrik:
die Struktur in `projects/libs/` trägt das schon), b (Kern im Werkstatt-Repo), d (Kopie, zwei Fassungen). Gilt als
Standard für künftige Bibliotheken (Memory „Bibliotheken unter projects/libs“).

✅ **IK2-2 a** (Patrik S173): **je Baustein die reichere Fassung als die eine Quelle, angelegt in LumiViz, an IC als
Paket.** Zwei Bibliotheken unter `projects/libs/`: **Kern** (rechnet, hängt nur an Fft und Eigenzerlegung) und
**Messung** (Messkette, Setzen; hängt am Kern) — Richtung App → Messung → Kern. Je Baustein: Flügel-Kern = LumiViz mit
den IC-Feldern (Moderator, `tempoFaktor`, Bass sustain, Soft, Upright/Honky-tonk/CP-70) bei neutralen Vorgaben, der
IC-Port fällt weg; Eigenzerlegung = IC (Grenze 32 — LumiViz schneidet heute bei n > 16 still ab, `Eigenzerlegung.cpp:266`;
`entartet`); Fft = LumiViz-Schnittstelle mit `DirekteDft` aus IC als Prüfumsetzung — **pocketfft korrigiert durch IK2-2-1 a**
(die IC-Fassung, neuer und belegt; hier stand irrtümlich die LumiViz-Fassung `c90e55b`); `Kurve` = IC-Modul (Belegkraft, Herkunft), Qt-JSON in die Dateischicht;
Messkette = LumiViz-Stand (LM je Block) plus IC-`Bausteine` in „Messung“; Setzen = IC-`setzen` mit LM je Block, die
Flügel-Suche liefert ihre Modell-Funktion; Lautheit und WAV = LumiViz (`audio/Lautheit` gegen EBU Tech 3341),
`parallelJe` und SHA-256 aus IC in den Kern; `Modal` aus IC in den Kern, HammerTon bleibt wortgleich; `KettenNote`,
`normiere`, `stufeAus`/`anschlagStufe` je einmal; IC-`Json` und IC-`Koerper` (Luftraum) bleiben in IC. Die IC-Kerne geben
ihre Messgrößen an die Messschicht ab statt Messcode einzubinden (Form in IK2-3). **Bitgleich:** alles, was LumiViz heute
rechnet — Wächter `Loop Werte`, `Klangfassung`, `hammer_bezug_<compiler>.txt` (neu mit Prüfsumme).
**Übernahme-Journal (Patrik S173):** „etwas kann ja eine reichere Fassung haben, aber falsch sein“ — jede Übernahme steht
in `docs/modellierung/anbindung-ic/Uebernahme_Journal.md`: je Baustein Quelle (Repo, Commit, Datei), Ziel, was
übernommen und was verworfen wurde, warum, Ergebnis der Wächter vorher/nachher. So lässt sich eine falsche Übernahme
finden und an der Quelle in Git nachlesen; die Reproduzierbarkeit bleibt über die Commits beider Repos. **Nichts geht
verloren (Patrik S173: „auch Fehlschläge sind Erkenntnisse — sogar wertvolle“):** was wegfällt (etwa der IC-Port des
Flügels als Code), steht im Journal mit dem, was er gezeigt hat, und mit Commit und Datei, an denen es nachzulesen ist;
verworfene Übernahmen, gescheiterte Wächter und ihre Ursache ebenso. Die Kenntnis je Klasse — Vorgaben mit Herkunft und
Belegkraft (IK2-4 a), Rechenzeiten, Prüfsteine, Befunde — bleibt beim Kern und unter `docs/modellierung/<klasse>/`.
Kippt: liefern die IC-Flügelfelder mit neutralen Vorgaben nicht bitgleich, kommen sie als eigener Teilschritt mit neuer
Klangfassung; läuft die Flügel-Suche (Plan A–D) wieder, zieht die Messkette erst nach dem Lauf um (Befund 9).
Verworfen: b (IC-Bestand verloren), c (Flügel-Port ohne Bitgleich-Beleg, älterer LM, ungeprüfte Lautheit), d (Kern trüge
die Messkette).

✅ **IK2-3 a** (Patrik S173): **abstrakte Klasse `Tonkern`** in der Kern-Bibliothek; die Schicht hält
`std::shared_ptr<const Tonkern>`, je Klasse ein Adapter (hält ihr `Spiel`, ruft `rechne<X>Ton`), Fabrik je Name.
**Auftrag:** Rate einmal; **Zeit überall in Frames** (`std::int64_t`, auch Kurvenpunkte); **Notenfolge**
`{beginn, ende, note, hz, gleitet}` (eine Note, Kette oder Clip-Noten; `hz` von außen, Stimmung bleibt in der App,
`tonHz`); **Stärke** als Anschlagstufe in dB (`int` ≤ 0, wie `:a<stufe>`) plus Expression als Kurve 0…1 — die
Übersetzung in `staerke` 0…1 steht im Adapter und im Übernahme-Journal; **Spuren** als benannte Kurven
`{name, punkte(frame, wert)}`, fest `expression`, `pitch` (Cent), `vibrato` (Cent), weitere nennt der Kern (`motor`,
`balg`, `register`, `rotary`). **Ausgabe** Mono `float`; innere Größen (Kontaktkraft, Moden) auf Wunsch als Daten, alles
Messen am Ausgang in „Messung“. **`vorbereite(Spiel)`** einmal je Instrument, gemerkt unter `instrumentAbdruck` (deckt
`bereite` und `stimme`; wann `stimme` vorab rechnet: IK2-11). **Auskunft** je Kern: Rechenweg Note/Kette/Clip, Stufe im
Namen, unterstützte Spuren, Nachklang in Frames (heute `hammerNachklang`, `Instrument.cpp:2927`), Vorbereitung teuer;
ersetzt `InstrumentAuskunft::hammer`. Einzelnote zuerst in Stufe 34, Kette und Clip als eigene Teilschritte. Flügel
bitgleich: `halten = huelle.ende` ist schon Frames (`Instrument.cpp:2919`), `hz`, `stufe` unverändert. Verworfen: b
(`std::variant` — jede Klasse fasst `variant` und jedes `visit` an, die Hülle muss jedes Spiel kennen), c (Sekunden —
Flügel bräuchte Rundung zurück, Kurvenpunkte zwischen Abtastwerten). Kippt auf b nur bei Aufruf je Block in Echtzeit (in
IK2-0 verworfen); auf Sekunden nur bei einem Kern mit eigener innerer Rate.

✅ **IK2-4 a** (Patrik S173): **`.lvinst` Fassung 21** — eine Schicht mit Kern trägt `"core": "<name>"` und
`"coreParams": { … }`. Jeder Kern meldet über die Fabrik eine **Parametertafel** (Name, Einheit, Art — Zahl, Ganzzahl,
Wahl aus Namen, Kurve je Note —, Grenzen, Vorgabe, Herkunft; reine Daten, ohne Qt); die App liest und schreibt die Werte
**allgemein nach der Tafel** in der Qt-Dateischicht, kein Code je Klasse; Kurven je Note in einer Form (`Kurve` mit
Punkten, Abweichungen, Belegkraft, Herkunft); alle Werte immer geschrieben. Der Flügel wird der erste Kern auf diesem Weg:
`"core": "hammerString"` mit seinen 23 Werten, `excitation`/`stringModel` fallen für diese Schicht weg; Fassung 20 wird
nicht weiter gelesen, `grand_piano.lvinst` (die einzige mit `hammerString`, von keinem Skript geschrieben) neu.
Wächter: `Loop Werte` bleibt, `klangfassung.txt` und Freeze-Abdrücke neu (nur Abdrücke). Creator und Setzen aus der
Tafel: IK2-13 und Teilplan. Verworfen: b (je Klasse Objekt, Lese-/Schreibcode, Erkennung — elfmal, dazu Abbildung für
Setzen und Plugins), c (eigene Datei je Kern — Publish, Datensicherung, Austausch, Abdruck müssten mitziehen). Kippt nur
für Werte, die keine Tafel trägt (gerechnete Tabellen wie der Körper): dieser Teil als eigene Datei, von der Tafel mit
Namen genannt.

✅ **IK2-5 a** (Patrik S173): **E1 a (kein fast-math, keine automatische FMA, feste Reihenfolge) wird Eigenschaft der
Bibliotheken Kern und Messung** — `compile_options` je Bibliothek in `Solution.json`, eine neue Fähigkeit von CMakeCraft
(heute nur für Tests und Executables; `LibraryCollect.cmake`/`LibraryCreate.cmake` ohne Treffer), als Auftrag über den
Sync CC, und sie muss ins Paket (`package_<name>`), damit IC und die Linux-Cloud dieselben Schalter bauen. Kein Behelf in
einer `Source.cmake`; die Bibliotheken entstehen nach dem CC-Release. **Bezüge je Plattform statt je Compiler:**
`hammer_bezug_windows.txt` ersetzt die beiden Dateien (gemessen S173: `hammer_bezug_clang.txt` und `_msvc.txt` gleich bis
auf die Kopfzeile, je 11 Zeilen — wie IC in S172: die Bits hängen an der Plattform); IC benennt seine Abdrücke ebenso um.
Beim Umzug: `*31b-3 Rechenzeit*` dreimal, `Loop Werte`, `Klangfassung`, Flügel-Bezug unter beiden Compilern, Ergebnis
ins Übernahme-Journal (erwartet gleich: MSVC `/fp:precise` ohne Kontraktion, clang ohne `-mfma` ohne FMA — ungemessen).
Verworfen: b (Regel für die ganze Solution — engt Visuals/UI ein, reist nicht mit dem Paket), c (Vorgaben der Compiler —
Linux-gcc kontrahiert ab Werk, ein `-march` änderte die Bits still). Kippt auf b, wenn CMakeCraft `compile_options` je
Bibliothek ablehnt; eigene Vorlage, wenn E1 a am Flügel die Grenze von 4,6 % (RZ1-1 a) reißt.

✅ **IK2-6 a** (Patrik S173, nach Rückfrage neu gefasst): **eine Grenze für die Bibliothek Kern: 4,6 % der Echtzeit**
am schwersten Ton jeder Klasse, unter Release-clang hier, nach der Taktprobe, dreimal allein, von Hand (Verfahren
RZ1-1 a). Tests melden **zwei Zahlen ohne Urteil**: den absoluten Anteil und das **Verhältnis zum einen Flügel-Kern** auf
derselben Maschine als Kennzahl (macht Cloud-Läufe vergleichbar); je Klasse mit Datum und Maschine im Bericht der Klasse.
Die relative Grenze in IC (Port × 5 / 4,6, `TestHilfe.hpp:99–102`) fällt weg — sie war ein Messtrick ohne festen Bezug,
keine Kenntnis über ein Instrument; die gemessenen Zahlen je Klasse (S172) bleiben. Deluxe (Cimbalom je Clip) und Becken
(10 × vorab) bekommen ihre Grenze mit ihren Klassen, nicht in Stufe 34. Heute höchstens 2,40 % (Tom) in Z1. Verworfen: b
(5 % neben 4,6 % — zwei Zahlen in einer Bibliothek), c (relativ zum Flügel — der Bezug wandert mit dessen Ausbau,
3,73–4,60 %). Kippt: braucht ein Kern der Reihenfolge Z1 mehr als 4,6 %, eigene Vorlage mit seinen Zahlen.

✅ **IK2-7 a** (Patrik S173): **Stufe 34 in dieser Reihenfolge:** (1) **Fundament** — Bibliotheken Kern und Messung (nach
dem CC-Release, CC1 a), `Tonkern` mit Fabrik und Auskunft, `.lvinst` Fassung 21 mit Parametertafel, der Flügel als
erster Kern umgezogen und bitgleich, die Messkette umgezogen, Übernahme-Journal; (2) Klassen 4 und 5 je Note;
(3) Klasse 1 ohne Flügel mit den IC-Vorgaben am einen Kern — **die bisherige Stufe 32 (Upright) geht hier auf**, die
Nummer 32 bleibt frei; (4) Klasse 10; (5) Klasse 2 je Note. **Der Rest von Stufe 31 danach auf der Bibliothek:** 31f als
Erweiterung des einen Flügel-Kerns, 31g zusammen mit IK2-13 (Creator/Editor nach der Parametertafel), 31e (Mitschwingen)
mit dem Clip-Weg, der auch Klasse 2 trägt, 31h als Abschluss; 31d-8 ff. wann Patrik es ansetzt. **Einteilung C** (IC
`BERICHT_BAU.md` §4.1) wird nicht mitgeliefert — Klasse 4 kommt ohne die gestrichene tiefe Lage des Vibraphons —, die
Befunde bleiben in den IC-Berichten, die Stufe der Klasse entscheidet neu; **D** bleibt als Gerüst in IC bis zu seiner
Klasse. Verworfen: b (Stufe 31 zuerst — 31e/31f im App-Core gebaut und umgezogen, 31g als Flügel-Creator vor der Tafel,
Mitschwingen zweimal), c (Fundament, dann 31e–31h — 31e vor dem Clip-Weg). Kippt auf c (mit 31e nach dem Clip-Weg), wenn
Patrik den Flügel vor den neuen Klassen hören will; zieht der Flügel nicht bitgleich um, kommen 4 und 5 trotzdem, der
Flügel-Teil als eigener Teilschritt mit neuer Klangfassung.

✅ **IK2-8 a** (Patrik S173, nach Rückfrage neu gefasst): **Die IC-Instrumente tragen den allgemeinen Namen des echten
Instruments** (Muster `grand_piano`; `upright_piano`, `marimba`, `vibraphone`, `glockenspiel`, `tubular_bells` …, keine
Markennamen; Namen je Instrument im Teilplan). **Die alten bleiben alle bestehen; umbenannt wird nur bei Kollision** — die
heutige Nachbildung, deren Name ein IC-Instrument braucht, heißt `synth_<name>` (Titel „Synth …“): sicher `marimba` →
`synth_marimba` (1 Loop: bells_demo) und `vibraphone` → `synth_vibraphone` (4: dnb, hiphop, lofi, waltz); `piano` und
`bell` bleiben (IC heißt `upright_piano`, `tubular_bells`/`glockenspiel`/`celesta`); `tine_piano`, `thumb_piano` nur, wenn
der Teilplan einen IC-Namen so wählt (vorgesehen: eigene allgemeine Namen). **`clavinet`** ist schon ein Modell
(Waveguide-Saite mit zwei Tonabnehmern — IC `BERICHT_BAU.md` §4.1 nennt es irrtümlich Sample): nach den Hörproben behält
das bessere den Namen, das andere bekommt sein Verfahren als Zusatz (kein `synth_`). Ein Skript zieht die Verweise in den
mitgelieferten Loops nach — sie klingen gleich, `Loop Werte` dieselben Zahlen unter neuem Namen; auf Patriks eigenen
Loop-Ordner nur auf Wunsch. Die 14 Kits bleiben, aus Klasse 5 kommt ein neues Kit; kein mitgelieferter Loop wird auf IC
umgestellt, je Klasse ein Beispiel `<name>_demo`; Umstellen bestehender Loops entscheidet Patrik nach dem Hören.
Verworfen: b (gleichnamig ersetzen — bis zu 21 Loops still auf ungemessene Klänge, Synth-Klangfarben verloren), c (IC mit
Zusatz daneben — „Marimba“ bliebe ein Sinus). Kippt: fällt ein IC-Instrument in den Hörproben durch, bleibt es draußen
(Z2 a), die Nachbildung behält ihren Namen.

✅ **IK2-9 — durch Z2 a beantwortet** (S173): ungemessen ausliefern nach SP1 a, Hörproben vorher, Kennzeichnung
„ungemessen“ in Creator und Palette, Messung als spätere Stufe. Folgerung (Claude S173, Einspruch offen): die
Kennzeichnung wird nicht von Hand gesetzt, sondern folgt aus der Herkunft der Werte in der Parametertafel (IK2-4 a) —
ein Instrument ist „ungemessen“, solange ein klangbestimmender Wert keine Herkunft „gemessen“ trägt.

✅ **IK2-10 a** (Patrik S173): **`ausgangBezug` wird ein Wert der Parametertafel** — die Zahl liefert `normiere` für die
Vorgabe, einmal gerechnet, mit Herkunft im Übernahme-Journal; `normiere` bleibt als Werkzeug, das sie neu rechnet, ein
Wächter prüft Zahl gegen Werkzeug. Grund: in Klasse 5 hängen die Rauschanteile am Bezug (`IC rechnung/klassen/
05_membran/Vorgaben.cpp:25–40`, „relativ zum Bezug gesetzt“) — er gehört zum Klang; ohne Rechnen beim ersten Gebrauch.
**Pegel** des Instruments über `instrument_pegel.py` (−13,4 LUFS) wie bei allen. **Körper:** Klassen 4 und 5 im Kern
(Partner Resonatorrohr/Tonebar/Kasten, `Stab.hpp:61`; Membran und Kessel), der Flügel behält den LumiViz-Boden; Klasse 2
nimmt `korpusImpulsantwort` (LumiViz, Stufe 23), der IC-Luftraum (`gemeinsam/Koerper`, heute ohne Nutzer) wird im
Teilschritt der Klasse 2 dagegen verglichen (Prüfstein, Hörproben) und ersetzt nur bei deutlich besserem Ergebnis.
**Stereo:** Mono durch Pan/Stereo XY der Schicht, mit Boden durch die Faltung. Verworfen: b (`normiere` zur Laufzeit —
63 Vorgaben rechnen je einen Ton, die Zahl steht nirgends), c (ein physikalischer Bezug — ohne hörbare Folge neben LUFS,
Hörorte und Rauschwerte je Klasse neu). Kippt auf c nur als Frage für alle Instrumente (Lautstärke physikalisch statt
LUFS).

✅ **IK2-11 a** (Patrik S173): **gestimmte Werte sind Daten** — Kurven in `coreParams` mit Belegkraft „Rechnung“, Herkunft
(Verfahren, Kriterium) und einem **Abdruck der Eingaben** (Tafel ohne die gestimmten Werte, Fassung der Rechnung des
Kerns). Das Stimmen (`stimme`, `stimmeBogen`, die sechs `ic_stimme_*`) zieht als Werkzeug in die Bibliothek Messung; ein
Programm `InstrumentTuneTool` (Muster `InstrumentBodyTool`) schreibt die Werte in die `.lvinst` — von Hand für die
mitgelieferten (Befehl im README des Ordners), für eigene über einen Knopf im Creator (IK2-13). **Wächter:** ein Test
vergleicht den gespeicherten Abdruck mit dem der heutigen Eingaben und meldet veraltete gestimmte Werte, ohne zu rechnen.
Fehlt ein gestimmter Wert, meldet das Laden einen Fehler — kein stiller Ersatz (Befund 11; heute
`#ifdef IC_STAB_STIMMUNG … #else stimmeBogen(s)`, `IC rechnung/klassen/04_platte_stab/Vorgaben.cpp:205–216`).
`Stimmung.inc` und `__has_include` fallen weg, die Werte ziehen mit Herkunft ins Journal; Klasse 2 stimmt nicht mehr bei
jedem Bau der Vorgabe (heute `Zupf.cpp:391–403`). In Stufe 34 betrifft es Klasse 10 und Klasse 2 (die Stimmung der
Klasse 4 gilt nur dem gestrichenen Vibraphon, IK2-7 a). Verworfen: b (`Stimmung.inc` als Code — zweiter Ort, kein
Wächter, eigene Instrumente nicht stimmbar), c (stimmen in `vorbereite` — Suche am Modell bei jedem ersten Gebrauch,
Dauer ungemessen; Setzen und Creator sähen die Werte nicht). Kippt für eine Klasse auf c, wenn ihr Stimmen gemessen
unter rund 100 ms je Instrument bleibt (Messung im Teilschritt der Klasse 2).

✅ **IK2-2-1 a** (Patrik S173, Korrektur zu IK2-2 a): **pocketfft wird die neuere Fassung aus IC** — in IK2-2 a stand
irrtümlich, sie habe keine Herkunft; tatsächlich belegt in IC `UEBERNAHME_LUMIVIZ.md` §3 (Zweig `cpp`, geladen
2026-10-29, SHA-256 `518a95c5…` mit LF; die Datei liegt in IC mit CRLF, daher `bc0095b2…` im Architektur-Bericht —
nachgerechnet S173), neuer als LumiViz `c90e55b` (2026-06-30). Im Fundament: Commit feststellen (Blob gegen die
LF-Prüfsumme), im README pinnen wie heute, `POCKETFFT_NO_MULTITHREADING`; den Diff der 91 Zeilen lesen und ins Journal;
dann `Loop Werte`, `Klangfassung`, Flügel-Bezug. Bleiben die Bits gleich, erledigt; ändern sie sich, eigener Teilschritt
mit neuer Klangfassung und Grund. Verworfen: b (`c90e55b` bleibt — Bitgleichheit ist kein Grund für die ältere Fassung,
wenn die Wächter jede Änderung zeigen). Kippt: lässt sich der Commit nicht eindeutig feststellen, der neueste Commit des
Zweigs `cpp` mit Prüfsumme.

✅ **IK2-12 a** (Patrik S173): **Quellenregister als Daten** in `asset/loop/` (Name im Teilplan) — je Quelle Schlüssel,
Nennung, Lizenz, Pflicht zur Nennung, **ob Werte daraus ausgeliefert werden dürfen**; Anfang ist die Tabelle aus IC
`UEBERNAHME_LUMIVIZ.md` §3 (26 Einträge, darunter CC BY-NC-SA Weinzierl und CC BY-ND TU-Note: „nichts ausliefern“). Die
Herkunft eines Werts in der Parametertafel nennt den Schlüssel. **Wächter:** kein Wert einer mitgelieferten `.lvinst`
aus einer Quelle „nicht ausliefern“; jede Pflichtquelle steht in der `CREDITS.md` des Instrument-Ordners, die ein Skript
aus dem Register schreibt (Muster `make_samples.py` bis zur Marke). **Ignorieren:** die Regeln für `aufnahmen/` und
`daten/` aus IC §2 als Sicherung in die LumiViz-`.gitignore` (nur Begleitdateien; `git check-ignore -v`). **Recherche
bleibt in IC** (Z3 a); LumiViz verweist und behält `docs/modellierung/<kebab>/` für eigene Berichte — nichts umbenannt
oder kopiert. Externals: pocketfft (BSD-3) in den Hinweisen der Anwendung, doctest nur bei Tests. Verworfen: b/c
(`CREDITS.md` von Hand, Recherche nach LumiViz mit kebab- bzw. IC-Namen — ND/NC nur durch Sorgfalt geschützt, zwei Orte
gegen Z3 a). Kippt auf b, wenn IC als Werkstatt aufgegeben wird (Kipppunkt Z3 a).

✅ **IK2-13 a** (Patrik S173): **eine Skizze für alle Kerne zuerst, Bau nach den Klassen.** Der erste Teilschritt des
Fundaments ist eine Skizze von Creator und Editor (Zeichenwerkzeug) für den Flügel und die Kerne der Reihenfolge Z1
zugleich, gebilligt und als Beschreibung ins Konzept (wie UF12 a): Gruppen aus der Tafel, Kurven über die Tastatur mit
Herkunft und Belegkraft (Kipppunkt UF12 a: Fenster *Keyboard curves*), gestimmte Werte mit „veraltet“ und „Neu stimmen“
(IK2-11 a), Marke „ungemessen“ (IK2-9), Ausstattung als Haken, Anspielen je Stufe. Die Parametertafel bekommt danach die
Angaben, die die Skizze braucht (Gruppe, Reihenfolge, Darstellung). Bau der Bedienung als 31g nach den Klassen (IK2-7 a),
ein Creator für alle Kerne; bis dahin zeigt der Creator eine Kern-Schicht mit Namen, Herkunft und „ungemessen“, die
Kern-Werte nur zum Ansehen — Anspielen, Palette, Pegel, „Save as“ wie bei jedem Instrument. Körper-Editor (UF13 b) bleibt
vorgemerkt als eigene spätere Stufe. Verworfen: b (Bedienung je Klasse — Code je Klasse gegen IK2-4 a, Tafel wüchse mit
jeder Klasse), c (eigene Stufe getrennt von 31g — zwei Creator-Formen). Kippt: will Patrik ein IC-Instrument vor 31g
verändern, vorab eine Liste der Tafelwerte (Zahlen editierbar, Kurven ansehen), nach der Skizze gebaut, von 31g ersetzt.

✅ **CC1 a** (Patrik S173): **Der Auftrag an CMakeCraft geht nach IK2-13 und dem Teilplan, in einem Stück** — als
`auftrag/CMakeCraft_Auftrag_v0.14.0.md` im Sync `sync_LumiViz_CMakeCraft` (Spezifikation, Abnahme, Prüffälle), Sync neu
eröffnet mit `session-sync` (LumiViz leitet), die CC-Sitzung startet Patrik. Bis dahin sammelt §5a die Punkte. Verworfen:
b (jetzt mit Teilpunkten — ein zweites Release und ein zweiter Pin-Wechsel, wenn IK2-11/IK2-12 Punkte bringen; Punkt 3
hängt am Teilplan). Kippt auf b, wenn Patrik die CC-Sitzung parallel offen haben will; Nachträge dann in denselben
Auftrag vor dem Release.

*Wortlaut der Vorlage IK2-0 (S172, in S173 erneut gestellt):*

**Ort:** Plan §3 IK2-0; IC `SCHNITTSTELLE.md` (unverbindlich); IC T2 a; `VST3_Konzept.md` §4 (nichts entschieden).

**Heutiger Wortlaut:** Patrik (Ende S171): „macht es Sinn, den ganzen Aufbau zu hinterfragen — Bibliotheken, oder gar
Plugins (VST), die von der Oberfläche her den LumiViz-Stil behalten? Die Schnittstellen sollten so oder so vereinheitlicht
werden.“ Dazu T2 a (§4).

**Problem:** 16 Rechenfunktionen in 12 `Spiel`-Typen, formgleich im Aufruf, verschieden in Stärke (Stufe in dB, Stärke
0…1, Expression als Skalar), Zeit (Frames oder Sekunden), Tonhöhe (Note, Kette, Clip-Noten) und Vorbereitung
(`bereite`, `stimme`). `rendereTon` kennt nur den Flügel; jede Klasse vergrößerte `Schicht`, die if-Kette in
`Instrument.cpp` und `InstrumentAuskunft`. `HammerSaite.hpp` zieht Qt in den Kern, der Kern hat keine Bibliothek.

| Zielgröße | a Schichten | b nur Bibliotheken, je Klasse | c Plugins als Schnittstelle |
|---|---|---|---|
| Bestimmtheit (Abdruck, Freeze, `Loop Werte`) | ja, vorab im Prozess | ja | eigene Hüllen ja; fremde Plugins nein |
| Rechenwege aus T2 a (Note, Kette, Passage, Clip) | eine Schnittstelle trägt alle vier | je Klasse ein Sonderweg (4 Ketten-, 3 Clip-Kerne, 2 Passage-Spuren) | Ereignisstrom trägt alle, aber live |
| Rechenzeit live | 0 (vorab) | 0 | je Stimme 0,2–4,6 % eines Kerns; Becken 1–7 × Echtzeit, live unmöglich |
| Tonspeicher | Schlüssel bleibt Tonname; Clip-Kerne über den Clip-Abdruck aus Freeze | gleich | entfällt für Plugin-Töne |
| Wiederverwendung (2,6 je Ton) | bleibt | bleibt | entfällt |
| Neu zu bauen | Qt-freie Kern-Bibliothek (STATIC, Erstfall in CMakeCraft), eine Ton-Schnittstelle mit Auskunft je Kern, dünne Adapter je Klasse, Kopf von `HammerSaite` teilen | Adapter je Klasse in `Instrument.cpp`, 12 Felder mehr an `Schicht` | Host (Laden, Parameter, Zustand, Threads, Fenster, Absturzschutz), 11 Hüllen, Ereignisweg im Klangerzeuger |
| Übernommen | alle Kerne, Tonspeicher, Freeze, Tonauftrag, Klangerzeuger | gleich | Kerne; Tonspeicher und Freeze nicht für Plugin-Töne |
| Plugin-Fähigkeit später | Hülle um dieselbe Schnittstelle; Vorab-Rechnung im Plugin belegt möglich | Hülle müsste 16 Funktionen spiegeln | sofort |

- **a (Empfehlung):** Drei Schichten. (1) Eine Qt-freie **Kern-Bibliothek** (reines C++20, STATIC unter
  `projects/libs/`, nur gegen `Fft` und `Eigenloeser`) mit **einer** Ton-Schnittstelle: ein Auftrag trägt die Notenfolge
  (eine Note, eine Kette, oder die Noten eines Clips mit Beginn), die Stärke, die Haltezeit, die Spuren als Kurven über
  die Zeit (Expression, Pitch, Vibrato, benannte Passage-Spuren wie Motor, Balg, Register, Rotary), die Rate; heraus
  kommt ein Mono-Puffer; dazu je Kern eine **Auskunft** (Rechenweg Note/Kette/Clip, Stufe im Namen, Nachklang,
  Vorbereitung). Je Klasse ein Adapter auf das heutige `rechne<X>Ton`. (2) **LumiViz nutzt sie direkt** im Prozess,
  vorab: Tonspeicher mit dem Tonnamen als Schlüssel, Clip-Kerne über den Clip-Abdruck aus Freeze, Stereo, Körper und
  Pegel aus LumiViz. (3) **Plugin-Hüllen** um dieselbe Bibliothek und ein **Host** für fremde Plugins als spätere
  eigene Stufen, nicht in Stufe 34. Der Flügel-Kern bekommt getrennte Köpfe (Daten ohne Qt, Datei mit Qt).
- **b:** Nur Bibliotheken, Schnittstelle je Klasse wie heute in IC; je Klasse ein Adapter in `Instrument.cpp` und Felder
  an `Schicht`; Ketten- und Clip-Kerne je ein eigener Weg; keine Vorbereitung für Plugins.
- **c:** Plugins sind die Schnittstelle: jeder IC-Kern wird eine CLAP, LumiViz baut jetzt den Host und spielt eigene wie
  fremde Instrumente als Ereignisstrom live, blockweise im Klangerzeuger.

*Warum die Empfehlung:* Den Ausschlag gibt T2 a — die vier Rechenwege sind entschieden, und a ist die eine Form, die sie
trägt, ohne je Klasse etwas zu erfinden; Abdruck, Freeze und `Loop Werte` verlangen die Vorab-Rechnung im Prozess, die a
behält. Für b spricht der kleinste Umbau; das wiegt weniger, weil jede Klasse die if-Kette und `Schicht` weiter
aufbläht, die Ketten- und Clip-Kerne drei Sonderwege brauchen und eine Plugin-Hülle später 16 Funktionen einzeln spiegeln
müsste. Für c spricht, dass Ereignisstrom und Oberfläche je Plugin sofort da wären; das wiegt weniger, weil c die
Bestimmtheit für fremde Plugins nachweislich nicht hat, Vorab-Rechnung und Tonspeicher verliert, Becken live unmöglich
macht und den größten Bau vor das erste Instrument stellt. Kippt auf b, wenn CMakeCraft eine STATIC-Bibliothek mit
Quellen nicht trägt und die Lücke dort nicht geschlossen wird; kippt in Richtung c, wenn Patrik fremde Plugins vor den
IC-Instrumenten hören will (dann der Host als eigene Stufe vor 34, die Bibliothek bleibt).

## 5a. Aufträge an CMakeCraft (gesammelt nach CC1 a)

1. `compile_options` je Bibliothek in `Solution.json` (IK2-5 a; heute nur Tests und Executables).
2. Die Schalter reisen mit dem Paket `package_<name>` (IK2-5 a), damit IC und die Linux-Cloud dieselben bauen.
3. Das Paket einer Bibliothek lässt sich in einem Projekt **ohne** CMakeCraft einbinden (IC baut mit einfachem CMake;
   etwa `find_package` oder Archiv) — ob CMakeCraft das heute kann, ungeprüft; nach TP1 a liegt das Paket als Fassung
   unter `rechnung/externals/<name>/<version>/` im IC-Repo und wird dort von einfachem CMake eingebunden.
4. Erster Einsatz STATIC mit Quellen in LumiViz (`LibraryCreate.cmake:138–170`) — kein neuer Bau in CMakeCraft, aber
   als Prüffall in die Abnahme.

✅ **Erledigt S174: CMakeCraft v0.14.0** (`8b68d25`), Pin gewechselt, voller Lauf 1753 / 1753 unter beiden Compilern.
Gebaut nach Auftrag und Nachtrag CC (Entscheide P1 a–P3 a in der CC-Sitzung): `compile_options` PRIVATE an
`libraries[]` (E105 bei INTERFACE); Paket `{ "name", "library" }` mit `src/`, `include/`, `externals/<ext>/`, erzeugtem
`CMakeLists.txt` (Sprachregeln `/permissive- /Zc:… NOMINMAX`, Ausnahmen/RTTI, `CXX_STANDARD`/`CXX_EXTENSIONS` wie in
LumiViz — hier OFF über das Preset), übertragenen Defines lokaler Externals (`POCKETFFT_NO_MULTITHREADING`) mit
Prüfsumme, `VERSION`, `SHA256SUMS` mit LF (E106 bei nicht Abbildbarem). Gegenprüfung mit einer Probe-Bibliothek: Schalter
nur an der Probe, Bezieher mit einfachem CMake unter clang und cl gleicher Abdruck; der Befund CRLF in den erzeugten
Dateien wurde vor dem Release behoben. **Für 34c:** privater Kopf muss in `Source.cmake` unter `_HEADERS` stehen, sonst
fehlt er im Paket; `.gitattributes` `eol=lf` unter `projects/libs/` (LV-4); `std::sin`/`exp` aus UCRT/glibc können je
Plattform im letzten Bit abweichen (Bezug je Plattform, IK2-5 a).

**Abgelegt S174** als `auftrag/CMakeCraft_Auftrag_v0.14.0.md` im Sync (gelesen gegen CMakeCraft `2d0656f`). Befund zu
Punkt 3: heute kann es CMakeCraft nicht — ein Paket kopiert nur Dateien (`Packages.cmake`: `headers_of`, `binary_of`,
`output_dir_of`, `from`), ohne Quellenliste, CMake-Beschreibung oder Schalter; der Bezieher eines Archive External ist
selbst CMakeCraft und linkt nichts. Der Auftrag verlangt deshalb ein **Quellpaket** einer Bibliothek: Quellen, Köpfe,
erzeugtes einfaches `CMakeLists.txt` mit den `compile_options` wörtlich (Generator Expressions je Compiler), lokale
Header-Externals (pocketfft) im Paket, `VERSION` der Bibliothek. Aus TP1 a abgeleitet und **neu** dazu: `SHA256SUMS` über
jede Datei des Pakets, weil IC den entpackten Ordner prüft, nicht das Zip; in IC dafür `.gitattributes` `* -text` (sonst
CRLF beim Checkout, wie bei pocketfft). Für 34c gemerkt: die Schalter gelten PRIVATE — gerechnet wird in `.cpp`-Dateien,
nicht in Köpfen, sonst übersetzt der Bezieher die Rechnung mit seinen Schaltern.

## 6. Teilplan

**Stufe 34 — Anbindung der Rechenkerne** (Entwurf S173, nach IK2-0 bis IK2-13; Nummern fest, Wachstum nur als
Entscheid). Prüfung je Teilschritt nach MT1 a (eng unter Release-clang, beide gebaut); **voller Lauf** nach UF17-1 a bei
34d, 34e und 34l (Rechenkern, Dateifassung, geteilte Dienste). Rechenzeit von Hand nach RZ1-1 a, wo ein Kern berührt
wird. Hörproben sammelt Patrik später (Memory „Proben später“); kein Teilschritt wartet darauf. TP1 a entschieden (unten).

| Nr. | Teilschritt | Inhalt | Prüfung, fertig wenn |
|---|---|---|---|
| 34a | Skizze | Creator und Editor für Flügel und Kerne Z1 (IK2-13 a), Zeichenwerkzeug | gebilligt, als Beschreibung in diesem Konzept |
| 34b | CMakeCraft | Auftrag v0.14.0 nach §5a (CC1 a), Sync, Release, Pin | CC-Abnahme; LumiViz konfiguriert mit neuem Pin unter beiden Compilern |
| 34c | Bibliothek Kern | `projects/libs/<Kern>` STATIC (Name im Schritt): Eigenzerlegung (IC), Fft + pocketfft (IK2-2-1 a, Commit gepinnt), `Kurve`, `Modal`, `parallelJe`, SHA-256, `KettenNote`/`normiere`/Stufe je einmal; `Tonkern`, Fabrik, Auskunft, Parametertafel nach 34a (IK2-3 a, IK2-4 a); E1 a an der Bibliothek (IK2-5 a); Übernahme-Journal angelegt | Tests ohne Qt gegen die Schnittstellen; Journal je Baustein |
| 34d | Flügel umziehen | HammerTon/HammerSaite in den Kern, Köpfe getrennt (Daten ohne Qt, Datei mit Qt), IC-Felder mit neutralen Vorgaben, Adapter; `.lvinst` Fassung 21 (`core`, `coreParams`), `grand_piano.lvinst` neu, Verteilung über `Tonkern`, `InstrumentAuskunft::hammer` ersetzt | **bitgleich:** `Loop Werte` gleich, `klangfassung.txt` neu (nur Abdrücke), `hammer_bezug_windows.txt` mit Prüfsumme; `*31b-3 Rechenzeit*` dreimal; voller Lauf beide Compiler |
| 34e | Bibliothek Messung | Messkette (LumiViz-Stand) + IC-`Bausteine`, `setzen` mit LM je Block, `Lautheit`, `WavFormat`; Flügel-Suche liefert ihre Modell-Funktion (IK2-2 a) | `salamander_*.txt` unverändert, Salamander-Fälle gleich; voller Lauf |
| 34f | Werkzeuge und Wächter | `normiere` als Werkzeug + Wächter (IK2-10 a), `InstrumentTuneTool` + Abdruck-Wächter (IK2-11 a), Quellenregister + Wächter + Skript für `CREDITS.md` (IK2-12 a), `.gitignore`-Regeln | Wächter rot bei absichtlich veralteter Zahl/Stimmung/ND-Quelle, sonst grün |
| 34g | Klasse 4 | Adapter, Vorgaben als `.lvinst` (Namen nach IK2-8 a), `synth_marimba`/`synth_vibraphone` mit Skript für die Loops, Pegel, Kennzeichnung „ungemessen“, `<name>_demo`, Hörproben-Liste | `Loop Werte` nur Namen geändert; Rechenzeit ≤ 4,6 %; Kennzahl Verhältnis zum Flügel im Bericht |
| 34h | Klasse 5 | Trommeln, neues Kit, Demo, Pegel, Liste | wie 34g |
| 34i | Klasse 1 ohne Flügel | Upright, Honky-tonk, elektrischer Flügel (CP-70), Clavinet (Name nach den Hörproben, IK2-8 a); ehemals Stufe 32 | wie 34g |
| 34j | Klasse 10 | Akkordeon, Harmonium, Melodica, Mundharmonika; Stimmung über `InstrumentTuneTool` | wie 34g; Wächter der Stimmung |
| 34k | Klasse 2 je Note | Cembalo, Harfe, Gitarre, Laute, Oud; Korpus `korpusImpulsantwort`, Vergleich IC-Luftraum (IK2-10 a); Messung von `stimme` (Kipppunkt IK2-11 a) | wie 34g; Bericht Luftraum gegen Korpus |
| 34l | Abschluss | Handbuch, Sichttest-Liste, Hörprobenliste für HörTest, Doku (Modul-Doku der Bibliotheken), Auftrag an IC als Datei (Paket beziehen nach TP1, Abdrücke je Plattform, Port entfernen) | Bereich und voller Lauf unter beiden Compilern |

Danach **Rest von Stufe 31 auf der Bibliothek** (IK2-7 a): 31f, 31g (Bedienung nach 34a), 31e mit dem Clip-Weg, 31h.
Ketten (3, 7, 8, 9) und Clips (11, Mitschwingen) als eigene spätere Stufen (Z1 a).

✅ **TP1 a** (Patrik S173): **Paket als Fassung im IC-Repo.** LumiViz baut je Fassung der Bibliotheken
`package_<name>` (Quellen, CMake-Beschreibung, Schalter nach E1 a, SHA-256; §5a Punkte 2 und 3); Patrik legt es unter
`rechnung/externals/<name>/<version>/` ins IC-Repo und committet es, wie IC heute pocketfft und doctest führt; das
IC-CMake bindet es ein, die Cloud sieht es im eigenen Repo. Geändert wird es in IC nie, ein Test dort prüft die
Prüfsumme. Neue Kerne (Klassen 3, 6, 7, 8, 9, 11) entstehen in IC als Werkstatt-Code gegen `Tonkern` und kommen über eine
Übernahme mit Journal nach LumiViz (Muster IK2-2 a). Verworfen: b (Abruf aus einem LumiViz-Release — Zugang einer
Cloud-Sitzung auf ein zweites privates Repo ungeprüft), c (Kern-Arbeit nur lokal — die Cloud baute keine Kerne mehr,
gegen Z3 a). Kippt auf b, wenn eine Cloud-Probe zeigt, dass eine Sitzung das LumiViz-Repo lesen kann; auf c, wenn keine
Cloud-Sitzungen für Kerne mehr gewollt sind.

## 6a. Skizze Creator und Editor (34a, gebilligt S174)

✅ **CK1 a** (Patrik S174, „Alle nach Empfehlung“): **die Skizze gilt** — ein Creator für alle Kerne, alles aus der
Parametertafel, kein Code je Klasse (IK2-13 a, IK2-4 a). Gezeichnet mit dem Zeichenwerkzeug, umschaltbar zwischen
Flügel, Vibraphon (Klasse 4), Snare (5), Akkordeon (10) und Gitarre (2); Werte und Gruppen aus den IC-Spielen
(`StabSpiel`, `TrommelSpiel`, `ZungenSpiel`, `ZupfSpiel`, `HammerSaite`/`Tastatur`) und `modell.md` §2.
✅ **CK2 a** (Patrik S174): **die Kurven bleiben im Creator** — der Kipppunkt von UF12 a (Fenster *Keyboard curves*)
greift nicht: die 22 Kurven des Flügels sind schmale Zeilen in ihren Gruppen, bearbeitet wird eine nach der anderen in
einem Editor; ein Vergleich zweier Kurven geht im Creator (zweite Kurve blass mitgezeichnet). Kippt, wenn Patrik beim
Stimmen regelmäßig mehrere Kurven nebeneinander braucht.

**Creator, Kern-Schicht** (Kopf, Schichten links, Anspielen wie heute):

1. **Signalweg:** *Core* (Name des Instruments) statt *Source*, kein *Envelope* (der Kern rechnet den ganzen Ton);
   *Body* nur, wo der Körper außerhalb des Kerns liegt (Flügel: LumiViz-Boden, Wahl aus den mitgelieferten oder „ohne“,
   UF13 a; Klasse 2: `korpusImpulsantwort`, IK2-10 a); danach *+ Add* und *Level* wie bei jeder Schicht.
2. **Marke *Unmeasured*** neben *Type*, solange der Kern das Kennzeichen „ungemessen“ trägt (Z2 a, IK2-9).
3. **Kasten *Core* gewählt:** Kopf „Core · <Instrument>“ mit Name des Kerns und Tastenumfang; darunter die **Gruppen
   der Tafel**, einklappbar, in der Reihenfolge der Tafel. Darstellung je Art: Zahl → Regler mit Einheit, Wahl → Liste,
   Schalter (Ausstattung) → Haken, Liste von Objekten (Chöre, Felle, Stegmoden) → Zeile mit Kurzform und *Edit...*
   (kleiner Dialog), **Kurve je Note → Zeile mit kleiner Form und Marke der Belegkraft**.
4. **Belegkraft** in vier Marken nach IC `Kurve.hpp`: *Measured* (Gemessen), *Source* (Quelle), *Computed* (Rechnung),
   *Open* (Offen); die Herkunft als Text unter dem Kurveneditor.
5. **Kurveneditor** rechts (eine Kurve, die gewählte): über den Tastenumfang des Kerns, Stützpunkte ziehen,
   Doppelklick fügt einen hinzu, Rechtsklick auf eine Taste gibt ihr einen eigenen Wert (HA1 a, als Marke sichtbar),
   *Reset curve*. Ein Kern ohne Kurven (Klasse 5) zeigt dort „one sound for the whole instrument“.
6. **Gestimmte Werte** (IK2-11 a): passen sie nicht mehr zu geänderten Werten, steht über den Gruppen ein Hinweis mit
   dem Namen der Größe und *Retune*; nach dem Stimmen „Tuned values fit“.
7. **Anspielen:** Tastatur „Click a key to hear it“ und darunter die Stufe *Play at* pp −40 … ff 0 dB (Anschlagstufe
   `:a<stufe>`).

**Instrument-Editor:** unter den Noten die **Spuren, die der Kern in seiner Auskunft nennt** (IK2-3 a) — Kurven
(Expression, Pitch, Motor, Balg, Bend, Vibrato), Stufen (Sustain, Sostenuto, Una corda, Dämpferpedal, Wirbel) und
Marken je Note (Schlag der Trommel, gestrichen, *Let ring*); beim Flügel nach der Wahl der Pedale (UF12 a, SF6 a).
Bedienung auffindbar (Zeiger, Griffe, Hinweise; Bilder aus den Tests).

**Was die Parametertafel dafür trägt** (für 34c, IK2-4 a ergänzt): je Wert **Gruppe** (Name), **Reihenfolge**
(Gruppe und Wert), **Darstellung** (Regler, Liste, Haken, Kurve, Liste mit Dialog), dazu Einheit, Grenzen, Vorgabe,
Skala der Kurve (lin/log) und Belegkraft/Herkunft; je Kern Tastenumfang, Kennzeichen und die Liste der gestimmten Werte
mit ihren Abhängigkeiten. Die Gruppen folgen den Abschnitten in den IC-Köpfen (etwa `Stab.hpp` *Partner*, *Schlaegel*,
*Daempfer*, *Motor*, *Abnahme*, *Bogen*).

## 7. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.5.0 | 2026-11-07 | Session 174. §6a neu: Skizze Creator und Editor (34a) gebilligt — CK1 a (ein Creator für alle Kerne aus der Tafel), CK2 a (Kurven im Creator, Kipppunkt UF12 a greift nicht); was die Tafel dafür trägt. §6: TP1 als entschieden vermerkt. |
| 0.4.0 | 2026-11-07 | Session 174. §5a: Auftrag v0.14.0, Befund zu Punkt 3 (Pakete ohne CMake-Beschreibung), Quellpaket mit `SHA256SUMS` (neu, aus TP1 a abgeleitet), Merker PRIVATE für 34c; 34b erledigt (v0.14.0 veröffentlicht, Pin gewechselt, voller Lauf), Merker für 34c aus der Gegenprüfung. |
| 0.3.0 | 2026-11-03 | Session 173. §5 IK2-0 a (drei Schichten) mit Nachprüfung CMakeCraft STATIC; IK2-1 c (Bibliotheken unter `projects/libs/`, IC bezieht sie als Paket); IK2-2 a (reichere Fassung je Baustein, Kern und Messung, Übernahme-Journal, nichts geht verloren); IK2-3 a (`Tonkern`, Frames); IK2-4 a (Fassung 21, Parametertafel); IK2-5 a (E1 a an den Bibliotheken, Bezüge je Plattform); IK2-6 a (4,6 %, Verhältnis zum Flügel als Kennzahl); IK2-7 a (Fundament zuerst, Stufe 32 geht in 34 auf, C/D); IK2-8 a (echte Namen, `synth_` nur bei Kollision); IK2-9 durch Z2 a; CC1 a und §5a (Aufträge an CMakeCraft); IK2-10 a (Bezug als Zahl der Tafel, LUFS, Körper, Stereo); IK2-11 a (gestimmte Werte als Daten mit Abdruck-Wächter); IK2-2-1 a (Korrektur: pocketfft aus IC, neuer und belegt); IK2-12 a (Quellenregister mit Wächter, Recherche bleibt in IC); IK2-13 a (eine Skizze für alle Kerne zuerst); §6 Teilplan Stufe 34 (34a–34l) und TP1 a (Paket als Fassung im IC-Repo). Wortlaut der Vorlage IK2-0 bleibt darunter. |
| 0.2.0 | 2026-11-02 | Session 172. §4 um T2 a, Architektur, Plugins, Messung (M1 a) und Loop-Zahlen ergänzt; §5 M1 a und die Vorlage IK2-0 im Wortlaut (Antwort in S173). |
| 0.1.0 | 2026-11-02 | Session 172. Angelegt: Ziel, Voraussetzungen (Patrik S171), Zielbild Z1 bis Z4 (Patrik S172, je a), Ausgangslage; Entscheide und Teilplan offen. |

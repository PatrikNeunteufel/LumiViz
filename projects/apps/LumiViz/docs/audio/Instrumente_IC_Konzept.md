# LumiViz — Instrumente aus InstrumentsCalculation (Konzept)

> **Version:** 0.25.0
> **Datum:** 2026-11-12 (Session 193)
> **IC aufgelöst (S182, LP2 b):** LumiViz ist die einzige Quelle für Rechenkerne und Vorgaben; IC wird nicht mehr
> gebaut oder geändert. Die Doku von IC liegt unter `../modellierung/<klasse>/ic/` und `../modellierung/anbindung-ic/ic/`,
> der Code als Vorlage unter `harvest/instruments-calculation/rechnung/` (Plan
> [../modellierung/anbindung-ic/Aufloesung_IC_Plan.md](../modellierung/anbindung-ic/Aufloesung_IC_Plan.md)). Wo dieses
> Konzept von IC als Werkstatt, von Paketen für IC oder von einer Cloud-Sitzung spricht, ist das Geschichte.
> **LP2 b erledigt (S183):** die Vorgaben von Wurlitzer, Kalimba und Clavinet je Taste hergeleitet (WU1 a, WU2 a, KA1 a,
> CL1 a, CL2 a; Herleitung [2026-11-09_S183_vorgaben_je_taste.md](../modellierung/anbindung-ic/2026-11-09_S183_vorgaben_je_taste.md),
> Journal J60–J64) — erster Fall der Regel, dass die IC-Abdrücke Beleg der Übernahme sind, kein Maßstab: Kern `bar` mit
> Zupfkraft und den Abnahmen `slot`/`bridge`, Kern `struckString` mit Durchmesser je Taste, beide Rechenfassung 2.
> **Typ:** Konzept — Planung der Anbindung der Rechenkerne aus `InstrumentsCalculation` (IC) an LumiViz;
> Zielbild Z1 bis Z4 und M1 entschieden (Patrik S172), **IK2-0 bis IK2-13 entschieden, IK2-2-1 a, CC1 a, TP1 a**
> (S173); **Teilplan Stufe 34 (34a–34m, 34m vor 34l) in §6**; 34b erledigt (S174, CMakeCraft v0.14.0, §5a), 34a gebilligt (§6a, CK1 a, CK2 a); 34c erledigt (S175, `InstrumentCore`, KN1 a, KN2 a); 34d erledigt (S176, `InstrumentModels`, FU1 a–FU3 a, BK1 a); 34e erledigt (S177, `InstrumentMeasure`, `AudioCore`, ME1 a–ME4 a); 34f erledigt (S177, Register, Wächter, `InstrumentTuneTool`, WZ1 a, WZ2 a); 34g erledigt (S178, Kern `bar`, neun Stabspiele, KS1 a–KS4 a); 34h erledigt (S179, TM1 a–TM4 a; S180 QM1 a, `.lvinst` Fassung 23); 34i erledigt (S181, KL1 a–KL4 a, KL4-1 a, KL3-1 a, TT1 a; `.lvinst` Fassung 24, Kern `struckString`); 34j erledigt (S189–S192, ZV1 a–ZV14 a; Kern `freeReed`, Akkordeon, Harmonium, Melodica, Mundharmonika); **Teilplan 34k (34k-1 bis 34k-11) in §6** (S193, GZ1 a–GZ4 a, GZ1-1 a)
> **Status:** Entwurf
> **Zielgruppe:** App-Entwickler
> **Bezug:** [Spielweisen_Konzept.md](Spielweisen_Konzept.md) (IK1, UF12 a, UF13 a, SP1 a, RF1 a) ·
> [VST3_Konzept.md](VST3_Konzept.md) (Entwurf S123, nichts entschieden) · IC `BERICHT_BAU.md`, `STAND.md`,
> `SCHNITTSTELLE.md`, `UEBERNAHME_LUMIVIZ.md`, Prüfung `pruefungen/LV-2026-10-30-erster-baudurchgang.md` (seit S182
> unter [../modellierung/anbindung-ic/ic/](../modellierung/anbindung-ic/ic/))
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
rechnet — Wächter `Loop Werte`, `Klangfassung`, `hammer_bezug_<compiler>.txt` (neu mit Prüfsumme; seit 34d
`hammer_bezug_windows.txt` nach IK2-5 a).
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
Teilschritt der Klasse 2 dagegen verglichen (Prüfstein, Hörproben) und ersetzt nur bei deutlich besserem Ergebnis
*(für Klasse 2 berichtigt S193, GZ4 a: Steg und Abstrahlung aus einem Körper nach KB1 a, §6 Teilplan 34k)*.
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
| 34c | Bibliothek Kern | `projects/libs/InstrumentCore` STATIC (KN1 a; ohne einen Kern, BK1 a): Eigenzerlegung (IC), Fft + pocketfft (IK2-2-1 a, Commit gepinnt), `Kurve`, `Modal`, `parallelJe`, SHA-256, `KettenNote`/`normiere`/Stufe je einmal; `Tonkern`, Fabrik, Auskunft, Parametertafel nach 34a (IK2-3 a, IK2-4 a); E1 a an der Bibliothek (IK2-5 a); Übernahme-Journal angelegt | Tests ohne Qt gegen die Schnittstellen; Journal je Baustein |
| 34d | Flügel umziehen | HammerTon/HammerSaite als Kern `hammerString` in `projects/libs/InstrumentModels` (BK1 a, `lumi::kern::hammer`, `kerne()`), Köpfe getrennt (Daten ohne Qt, Datei mit Qt), IC-Felder mit neutralen Vorgaben, Adapter; Körper an der Schicht (FU1 a), Pedale in der Tafel (FU2 a), der Creator zeigt die Kern-Schicht (FU3 a); `.lvinst` Fassung 21 (`core`, `coreParams`), `grand_piano.lvinst` neu, Verteilung über `Tonkern`, `InstrumentAuskunft::hammer` ersetzt | **bitgleich:** `Loop Werte` gleich, `klangfassung.txt` neu (nur Abdrücke), `hammer_bezug_windows.txt` mit Prüfsumme; `*31b-3 Rechenzeit*` dreimal; voller Lauf beide Compiler |
| 34e | Bibliothek Messung | Messkette (LumiViz-Stand) + IC-`Bausteine`, `setzen` mit LM je Block, `Lautheit`, `WavFormat`; Flügel-Suche liefert ihre Modell-Funktion (IK2-2 a) | `salamander_*.txt` unverändert, Salamander-Fälle gleich; voller Lauf |
| 34f | Werkzeuge und Wächter | `normiere` als Werkzeug + Wächter (IK2-10 a), `InstrumentTuneTool` + Abdruck-Wächter (IK2-11 a), Quellenregister + Wächter + Skript für `CREDITS.md` (IK2-12 a), `.gitignore`-Regeln | Wächter rot bei absichtlich veralteter Zahl/Stimmung/ND-Quelle, sonst grün |
| 34g | Klasse 4 | Adapter, Vorgaben als `.lvinst` (Namen nach IK2-8 a), `synth_marimba`/`synth_vibraphone` mit Skript für die Loops, Pegel, Kennzeichnung „ungemessen“, `<name>_demo`, Hörproben-Liste | `Loop Werte` nur Namen geändert; Rechenzeit ≤ 4,6 %; Kennzahl Verhältnis zum Flügel im Bericht |
| 34h | Klasse 5 | Trommeln, neues Kit, Demo, Pegel, Liste | wie 34g |
| 34i | Klasse 1 ohne Flügel | Upright, Honky-tonk, elektrischer Flügel (CP-70), Clavinet (Name nach den Hörproben, IK2-8 a); ehemals Stufe 32 | wie 34g |
| 34j | Klasse 10 | Akkordeon, Harmonium, Melodica, Mundharmonika; Stimmung über `InstrumentTuneTool`; Vorlage `harvest/instruments-calculation/rechnung/klassen/10_zunge/` (S182). **Seit S188 nach ZV1 a–ZV3 a:** erst der Umzug Bit für Bit als Beleg, dann die Vorgaben je Taste mit Quellen hergeleitet, ein Winddruck für alle Tasten, laufender Wind — Teilplan 34j unten | Zielgrößen je Instrument (jede Taste spricht im Windbereich an, Lautheit über den Umfang ≤ 15 LU, ±10 Cent); Wächter der Stimmung; voller Lauf |
| 34k | Klasse 2 je Note | Cembalo, Harfe, Gitarre, Laute, Oud; Vorlage `harvest/instruments-calculation/rechnung/klassen/02_zupf_saite/` (S182). **Seit S193 nach GZ1 a–GZ4 a, GZ1-1 a:** erst der Umzug Bit für Bit als Beleg, dann Rechenfassung 2 mit Quellen (Zupf als Auslenkung, Saiten je Taste bzw. je Saite), Steg und Abstrahlung aus einem Körper nach KB1 a (statt `korpusImpulsantwort`), Mitschwingen je Note, Spielweisen am Kern mit Saitensatz — Teilplan 34k unten | Zielgrößen je Instrument (Lautheit über den Umfang ≤ 15 LU, kein Sprung über 6 LU, ±10 Cent unabhängig gemessen); Wächter der Stimmung und der Stegmoden; Bericht Luftraum und Steg gegen `korpusImpulsantwort`; voller Lauf |
| 34m | Klasse 7 (LR1 a; nach 34j und 34k, **vor 34l**) | Kern `Blech` mit laufender Anregung (Blasdruck als Spur) für alle spielbaren Instrumente der Klasse: Posaune, Trompete, Alphorn; Vorlage `harvest/instruments-calculation/rechnung/klassen/07_lippe_rohr/`, Doku `../modellierung/lippe-rohr/ic/`; Ursache des NaN am Alphorn A2 (ab rund 53 ms), Geometrie des Alphorns aus einer Quelle (R1, Mundstück, Becher, Länge); Rechenzeit (nach dem Tausch der Kühlung); Loops als Demos, das Alphorn zuerst; Hörliste. Tuba, Horn, Didgeridoo erst mit belegter Bohrung bzw. Lippe. Teilplan bis zum Abschluss vor dem Bau | wie 34g; kein NaN über den Umfang; Rechenzeit im Bericht |
| 34l | Abschluss | Handbuch, Sichttest-Liste, Hörprobenliste für HörTest, Doku (Modul-Doku der Bibliotheken); ~~Auftrag an IC als Datei~~ entfällt (IC aufgelöst S182, LP2 b); den Teil der übernommenen Klassen in `harvest/instruments-calculation/` löschen | Bereich und voller Lauf unter beiden Compilern |

✅ **LR1 a** (Patrik S186): **Klasse 7 kommt als eigene Stufe 34m in den Plan**, über den ganzen Kern `Blech` mit
Posaune, Trompete und Alphorn, nach 34j und 34k und vor dem Abschluss 34l (der die Vorlagen unter `harvest/` löscht).
Grund: alle Blechbläser teilen einen Kern; wer ihn für das Alphorn baut, baut ihn für alle, und die Befunde der Klasse
gehen in eine Fassung des Kerns statt in eine spätere zweite mit neuen Abdrücken und Hörproben. Das Alphorn ist das am
schwächsten belegte Instrument der Klasse (Geometrie unvollständig, A2 NaN). Verworfen: b (nur das Alphorn vorziehen —
die Demo stünde auf den schwächsten Werten), c (erst nach 34l entscheiden — gewinnt nichts). Kippt, wenn sich die Geometrie
des Alphorns belegen lässt und die Ursache des NaN klein ist (dann ginge das Alphorn zuerst ohne Nachteil). Ob 34m vor 34j
gezogen wird, ist eine eigene Frage (Reihenfolge nach dem, was Patrik zuerst hören will).

✅ **KN1 a** (Patrik S175, „Alle nach Empfehlung“): **Die Bibliothek Kern heißt `InstrumentCore`** (Ziel und Ordner
`projects/libs/InstrumentCore`, Namensraum `lumi::kern`, Paket `instrumentcore`); die Bibliothek Messung (34e) heißt
`InstrumentMeasure` mit `lumi::messung`. Grund: Muster der Schwester `InstrumentBody` (`lumi::koerper`). Verworfen: b
(`LumiKern` — nur Beispiel im Auftrag, mischt Deutsch und Englisch im Zielnamen), c (`ToneCore` — Tafel und Auskunft gelten
je Instrument). Kippt, wenn die Bibliothek Dinge außerhalb der Instrumente tragen soll (dann `AudioCore`).

✅ **KN2 a** (Patrik S175): **34c verschiebt** — Eigenzerlegung, Fft mit pocketfft und `anschlagStufe` ziehen aus der App
in `InstrumentCore`, App und Tests binden die Bibliothek, ihre Kopien entfallen; `parallelJe` der Tests wird das der
Bibliothek; pocketfft wird die IC-Fassung. Danach Wächter `Loop Werte`, `Klangfassung`, Flügel-Bezug, `*31b-3 Rechenzeit*`
dreimal und ein **voller Lauf unter beiden Compilern** (External geändert, UF17-1 a), alles im Übernahme-Journal. Neu dazu
`Kurve`, `Modal`, SHA-256, `KettenNote`, `normiere`, `Tonkern`, Fabrik, Auskunft, Parametertafel; `Tastenkurve` weicht der
`Kurve` erst in 34d (hängt an der Flügel-Datei). Grund: ändern sich Bits, hängt die Ursache an genau einer Übernahme
(pocketfft und Grenze 32 in 34c, Flügel in 34d). Verworfen: b (daneben anlegen — zwei Fassungen je Baustein für einen
Schritt, drei Ursachen in 34d zusammen). Kippt, wenn die Flügel-Suche (Plan A–D) wieder laufen soll, bevor 34c fertig ist.

✅ **FU1 a** (Patrik S175, „Alle nach Empfehlung“): **Der Resonanzboden ist ein Feld der Schicht, nicht der Tafel** —
`"body": "lib:steinway_d"` (leer = ohne) an jeder Kern-Schicht, deren Körper die App rechnet (Flügel jetzt, Klasse 2 mit
`korpusImpulsantwort` in 34k); `coreParams` des Flügels trägt die 22 Kurven (IK2-4 a „23 Werte“ berichtigt: 22 und der
Körper an der Schicht). Grund: CK1 a zeichnet *Body* als eigenen Kasten im Signalweg; den Körper faltet die App, der Kern
liest ihn nie. Verworfen: b (Wert der Tafel mit einer Art *Verweis* — die Datenschicht ohne Qt hinge an App-Dateien, IC sähe
den Körper nie). Kippt, wenn ein Kern seinen Körper selbst rechnet (IC-Luftraum, 34k): der gehört in seine Tafel.

✅ **FU2 a** (Patrik S175): **Was die Rechnung bestimmt, kommt in die Tafel; der Block `keyboard` bleibt für die
Planung.** Gruppe *Pedals* der Tafel: `undampedFrom` (Ganzzahl, 89), `softFactor` (1), `moderatorStiffness` (2),
`moderatorExponent` (2). Getreten wird über Spuren, die der Kern in seiner Auskunft nennt: `soft`, `practice` (Stufe 0/1 am
Beginn der Note); Sustain und Sostenuto bleiben in der Planung (`halten`). `keyboard` am Instrument trägt Umfang,
Pedalplätze und `bassSustainBis` (dazu `PedalArt::BassSustain` aus IC). Spuren Soft/Practice in Loops und Editor kommen mit
31g; in 34d prüfen Tests sie am Kern. Grund: IK2-4 a — ein Kern bekommt alles aus seiner Tafel (Fabrik, Abdruck, Hülle ohne
Wissen über Klaviere); vom Block ging nur `undampedFrom` in die Rechnung (`Instrument.cpp:2922`). Verworfen: b (alles im
Block, der Adapter bekäme die Tastatur dazu — ein Klavier-Sonderweg je Fabrik, der Abdruck sähe die Dämpfer nicht). Kippt,
wenn die Planung `undampedFrom` selbst braucht (Sostenuto je Taste, 31f): sie liest es aus der Tafel.

✅ **FU3 a** (Patrik S175): **Der Creator zeigt die Kern-Schicht schon in 34d** (IK2-13 a „bis dahin“): Kasten *Core* (Titel,
Tastenumfang), Marke *Unmeasured* aus `ungemessen`, *Body* nach FU1 a, die Werte der Tafel nur zum Ansehen in ihren Gruppen
(Kurve als Zeile mit Belegkraft); ein Fall über den Creator mit Bild `LUMIVIZ_CREATOR_CORE_BILD` prüft „Save as“ Bit für
Bit. Die Bearbeitung bleibt 31g. Verworfen: b (keine Ansicht bis 31g — der mitgelieferte Flügel bliebe im Creator
unlesbar, der Hörtest der IC-Instrumente braucht dieselbe Ansicht). Kippt, wenn der Creator vor 31g nicht für
Kern-Instrumente geöffnet werden soll.

✅ **BK1 a** (Patrik S175, Rückfrage während 34d): **Zwei Bibliotheken für das Rechnen — `InstrumentCore` ohne einen
Kern, darüber `InstrumentModels` mit allen Kernen.** `InstrumentCore` bleibt der allgemeine Teil (Tonkern, Fabrik, Auskunft,
Tafel, Tonauftrag, Fft, Eigenzerlegung, Modal, Kurve, Pool, SHA-256, Anschlagstufe); `InstrumentModels`
(`projects/libs/InstrumentModels`, Unterordner und Namensraum je Klasse, etwa `hammer/` und `lumi::kern::hammer`; Paket
`instrumentmodels`) trägt die Kerne und `kerne()`. Richtung: App → (Messung →) Models → Core. Ergänzt IK2-2 a („Kern“ =
Core + Models) und KN1 a. Grund: die Schnittstelle kennt keinen Kern; IC baut neue Kerne gegen das Core-Paket allein
(TP1 a); geschichtet wie IC (`ic_gemeinsam` → `ic_klassen`); die Kerne teilen Bausteine in einer Bibliothek. Verworfen: b (je
Klasse eine Bibliothek — bis zu zwölf gleiche Schalterblöcke und Pakete, geteilte Bausteine über Grenzen), c (alles in
`InstrumentCore` — die Schnittstelle hinge am Flügel). Kippt auf c, wenn IC nie gegen die Schnittstelle allein bauen soll;
auf b, wenn ein Instrument einzeln ausgeliefert werden soll und keine Bausteine mehr teilt.

**Teilplan 34e** (S177, Zuschnitt nach IK2-2 a, KN1 a, ME1 a, ME2 a; Nummern fest). `InstrumentMeasure` (`lumi::messung`,
STATIC, E1 a, Paket `instrumentmeasure`) hängt in 34e nur an `InstrumentCore` und `AudioCore`. Gefunden: die Messkette
liegt als Köpfe ohne Qt unter `tests/unit/UnitTests/` (schon `lumi::messung`); IC führt sie als Port mit geänderten
Namen und Pfaden, nur das LM ist in LumiViz neuer (`levenbergMarquardtJeBlock`, 7.5.12); `Bausteine`, `setzen` und
`Samples` gibt es nur in IC; `setzen` trägt ein eigenes LM je Gruppe (L8, L9) ohne A3.

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34e-1 | Bibliothek anlegen; die LumiViz-Messkette (`Teiltonmessung`, `Fluegelmessung`, `Strukturmessung`, `Kurvenabstand`, `Zielsumme`, `Hoerschwellen`, `Probe7512` → `Probe` wie IC, `LevenbergMarquardt`, `NelderMead`) zieht um, geteilt in `.hpp`/`.cpp` (Schalter PRIVATE); die Kopien in `UnitTests/` entfallen | `LUMIVIZ_SALAMANDER` misst neu, `salamander_*.txt` ohne Diff; `*Teiltonmessung*`, `*Salamander*`, `*Kurvenabstand*` gleich |
| 34e-2 | `AudioCore` nach ME1 a mit `Lautheit`, `WavFormat` (`lumi::audio`) | `*Lautheit*`, `*Pegel*`, `*Wav*`, Composer-Lautheit grün; `Loop Werte` gleich |
| 34e-3 | IC-`Bausteine` B1–B8, `amplitudenBei`, von `Samples` der allgemeine Teil (Name der Umgebungsvariable vom Aufrufer); `zerlegeSymmetrisch` nach ME2 a; Fälle aus IC `test_messung.cpp` (der Fall „Modalfit am Modell“ braucht den Stab: 34g) | die übernommenen Fälle grün mit den Grenzen aus IC |
| 34e-4 | `setzen` = IC-Rahmen (L9, L11, L12, L16, `setzeMehrfach`, `probe`, `glaette`), innen das LM je Block aus LumiViz (A2, A3, Differenz je Parameter); das IC-LM entfällt; die Flügel-Suche (`Suche.hpp`) liefert ihr `setzen::Modell` | IC `test_setzen` und die LM-Fälle aus `test_LoopSalamanderSuche` grün; Probe vorher/nachher an zwei Tasten, zwei Iterationen — gleich oder Ursache im Journal |
| 34e-5 | Pakete `audiocore`, `instrumentmeasure` (`sha256sum -c`, LF), Modul-Doku, Journal J19 ff., Konzept, Steckbrief | voller Lauf unter Release-clang und Testing-MSVC |

In den Tests bleiben, was `loop/Instrument` oder Qt braucht: `FluegelSchicht`, `ModellMessung`, `Modellton`,
`Suchkurven`, `Suche`, `Schwebungsstufe`, dazu `SaitenMessung`. Nicht in 34e: das Stimmen (IK2-11 a, 34f/34k); pYIN
bleibt in der App (HMM für Gesang 55–880 Hz, B2 misst Instrumente 25–4 000 Hz mit wählbarem Bereich — zwei Aufgaben).
Die IC-Kerne rufen heute `messung::tonhoehe`/`einschwingen` selbst (fünf Klassen); nach IK2-2 a geben sie ihre
Messgrößen ab — Thema ab 34g.

✅ **ME1 a** (Patrik S177): **`Lautheit` und `WavFormat` kommen in eine eigene Bibliothek `AudioCore`**
(`projects/libs/AudioCore`, STATIC, E1 a, Paket `audiocore`, Namensraum `lumi::audio` bleibt); `InstrumentMeasure` und
die App hängen daran, die App nicht an `InstrumentMeasure`. Grund: der Composer misst Songs (`Abgleich`,
`ClipLautheit`), `WavFormat` trägt Export, Freeze und Aufnahme — allgemeines Audio, der Kipppunkt von KN1 a. Verworfen:
b (in `InstrumentMeasure`, `lumi::messung` — der Composer hinge an einer Instrument-Bibliothek; spart nur ein Paket). Alle
Zielgrößen gleich (wortgleicher Umzug). Kippt, wenn beide nie außerhalb von Instrumenten gebraucht würden.

✅ **ME2 a** (Patrik S177): **`zerlegeSymmetrisch` zieht wortgleich als allgemeiner Löser in
`InstrumentCore/Eigenzerlegung`** (aus IC `gemeinsam/Balken.hpp:28–34`), mit eigenem Test und Journal; der Balken nutzt
ihn in 34g von dort. Grund: Schichtung nach BK1 a — ein allgemeiner Löser in den allgemeinen Teil, ein Balken-Modell zu
den Kernen. Verworfen: b (ganzer `Balken.hpp` in den Core — FEM einer Klasse in der Schnittstelle), c (Kopie in der
Messung — zwei Fassungen, gegen „je einmal“). Kippt, wenn der Löser mit dem Cholesky-Schritt des Balkens verflochten ist
(geprüft in 34e-3: nicht verflochten).

✅ **ME3 a** (Patrik S177): **`setzen` bekommt einen Beobachter je Iteration und einen Startzustand für die
Wiederaufnahme** (wie das LM je Block); das IC-Protokoll bleibt als eigene Möglichkeit. Die Flügel-Suche ruft `setzen`
mit ihrem `Modell` und schreibt ihre Dateien weiter im Beobachter — Formate, Wiederaufnahme (`liesLetzteIteration`) und
`Probe` bleiben. Dazu nimmt `setzen` Schritt und Ende je Parameter (die Wege sind unter der Marquardt-Skalierung gleich,
nicht Bit für Bit), und das Flügel-`Modell` rechnet ohne eigenen Pool (`parallelJe` verschachtelt sich nicht: bis
11 × 3 Threads). Grund: ein Setzen mit einem LM (IK2-2 a), ohne Verlust der gemessenen Läufe S162–S165. Verworfen: b
(Protokoll und Bericht von `setzen` auch für den Flügel — die Läufe S162–S165 nicht mehr aufnehm- und prüfbar), c (die
Flügel-Suche beim LM je Block direkt — bitgleich, aber der Flügel bliebe ohne L9, L12, L16, Mehrfachstart und Probe aus
IC). Kippt auf b, wenn alte Läufe nicht mehr aufgenommen werden sollen; auf c, wenn die Flügel-Suche bald wieder läuft
und Bitgleichheit mit S165 vorgeht.

✅ **ME4 a** (Patrik S177): **Beide LM werten auch den letzten kleinen Schritt aus** und nehmen ihn, wenn er Σ r² senkt;
danach Ende „Schritte klein“ (Regel aus IC `setzen`; „Ende, wenn jeder Schritt unter `LmRaum::ende` liegt“ bleibt
wahr). Grund: nie schlechter, höchstens eine Auswertung je Block mehr; die übernommenen IC-Fälle gelten mit ihren
Zahlen (Rest 0,107 / 0,307 wie in IC) — mit der alten LumiViz-Regel 0,502 / 0,905, der Fall „ohne L12“ rot. Verworfen: b
(alte Regel, den IC-Fall auf den Rest der `schwebung` umschreiben — eine geflickte Prüfung für die übernommene Regel).
Kippt, wenn sich am Flügel zeigt, dass diese Schritte systematisch Rauschen der Messung fangen (`eindeutig` über die
Startpunkte schlechter; erst in einem langen Lauf der Suche zu sehen).

**Teilplan 34f** (S177, Zuschnitt nach IK2-10 a bis IK2-12 a, WZ1 a, WZ2 a; Nummern fest). Gefunden: `normiere`,
`eingabenAbdruck`, `veralteteGestimmte`, `merkeAbdruck` stehen im Core (34c, J10); der einzige Kern (Flügel) hat weder
Bezug noch gestimmte Werte — die Wächter prüfen bis 34g/34j an einem Prüfkern; die Herkunft der Flügel-Werte ist heute
freier Text ohne Schlüssel; `loop/KernDatei` hängt nur an der Tafel und Qt-JSON.

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34f-1 | Feld `quellen` an `Kurve` und `ParameterAngabe` (WZ1 a), `.lvinst` **Fassung 22** (`sources`, immer geschrieben); Register `asset/loop/instrument_quellen.json` (WZ2 a) aus IC §3 und den Quellen des Flügels; die Vorgaben des Flügels nennen ihre Schlüssel; `grand_piano.lvinst` neu, `klangfassung.txt` neu (nur Abdrücke) | `Loop Werte` gleich, Flügel-Bezug gleich, jeder Wert des Flügels mit Schlüssel |
| 34f-2 | Wächter Register (kein Wert aus „nicht ausliefern“, jeder Schlüssel bekannt); Skript schreibt `asset/loop/instruments/CREDITS.md` bis zur Marke | grün; rot bei eingesetzter ND-Quelle und unbekanntem Schlüssel |
| 34f-3 | `messung::stimmer()` (je Kern die Funktion der gestimmten Werte; heute leer), `InstrumentTuneTool <datei.lvinst> [--bezug]` unter `projects/exec/` (kompiliert `loop/KernDatei` wie `InstrumentBodyTool` die Körper-Dateien) | am Prüfkern geschrieben, danach nichts veraltet |
| 34f-4 | Wächter Stimmung (Abdruck, ohne Rechnen) und Bezug (gegen `normiere`) über die mitgelieferten `.lvinst` | rot am Prüfkern mit veralteter Stimmung bzw. Bezug, sonst grün (heute ohne Fall: meldet das) |
| 34f-5 | `.gitignore` nach IC §2 (`aufnahmen/`, `daten/`, nur Begleitdateien; `git check-ignore -v`), README-Befehl, Journal J27 ff., Modul-Doku | Bereich unter beiden Compilern; **voller Lauf** (neue Dateifassung, UF17-1 a) |

✅ **WZ1 a** (Patrik S177): **Ein Wert trägt seine Quellen als eigenes Feld** — `Kurve::quellen` und
`ParameterAngabe::quellen` (Schlüssel des Registers), in der Datei `"sources": [ … ]` neben dem Text `source`, immer
geschrieben; `.lvinst` Fassung 22. Grund: ein Schlüssel, auf den ein Wächter die Lizenzpflicht stützt, gehört in ein
Feld, nicht in Prosa (Tippfehler oder eine zweite Quelle im Satz ließen ND/NC still durch); B nennt zwei Quellen.
Verworfen: b (Schlüssel vorn im Text, `[rt0425] …` — Fassung bliebe, Abdrücke änderten sich trotzdem). Kippt, wenn IC
das Feld nicht führen kann (es bezieht `Kurve` aus dem Core-Paket).

✅ **WZ2 a** (Patrik S177): **Register `asset/loop/instrument_quellen.json`** — Liste von
`{ "key", "credit", "license", "creditRequired", "shippable", "note" }`; Qt liest es im Wächter, Python im Skript für
`CREDITS.md`; nicht in `asset/Asset.cmake` (wird nicht ausgeliefert). Verworfen: b (`.py` wie Katalog und Pegel — der
C++-Wächter bräuchte eine erzeugte Kopie), c (`.tsv` — keine Listen, keine Tabulatoren in Nennungen). Kippt, wenn die
App das Register zeigen soll (dann in `Asset.cmake`, Format bleibt).

**Teilplan 34g** (S178, Zuschnitt nach IK2-7 a, IK2-8 a, IK2-11 a, KS1 a–KS4 a; Nummern fest). Gefunden: IC Einteilung A
nennt neun Instrumente der Klasse 4 (die Steeldrum ist Deluxe); IC führt je Instrument einen Abdruck des Tons unter Windows
(`rechnung/werte/abdruck_msvc.txt`, `stab_<Name>_mitte_a-12` und `stab_Vibraphon_gestrichen_m72_mf`; Windows-clang ist
bitgleich mit MSVC, S172) — der Beleg des Umzugs; `Modal` und `zerlegeSymmetrisch` liegen im Core, `Balken` und `Reibung`
nicht; Celesta und Wurlitzer nehmen den Filzhammer aus `hammer::stulovHammer…` (in `InstrumentModels/hammer` vorhanden);
`rechneStabTon` ruft für den Bogen `messung::tonhoehe` (IC `Stab.cpp:489`) — das Messen zieht nach IK2-2 a in den Stimmer;
`partnerAbstrahl` wird beim Bau der Vorgabe durch das Modell gesucht (IC `Vorgaben.cpp:124–148`); `kalimba` kollidiert
zusätzlich, und `make_examples.py`/`make_genres.py` schreiben die heutigen `marimba`, `vibraphone`, `kalimba` selbst.

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34g-1 | `InstrumentModels/gemeinsam/Balken` (IC, nutzt `zerlegeSymmetrisch` des Core) und `gemeinsam/Reibung` (KS3 a); Fälle aus IC `test_gemeinsam` | die übernommenen Fälle grün mit den IC-Grenzen |
| 34g-2 | Core nach KS1 a (Umfang aus den Werten) und KS2 a (gestimmte Werte in Reihenfolge der Tafel), `InstrumentCore` 0.3.0 | Flügel unberührt: `Loop Werte`, `Klangfassung`, Flügel-Bezug gleich; Fälle zu Tafel und Abdruck |
| 34g-3 | Kern `bar` unter `InstrumentModels/stab/` (`lumi::kern::stab`): Tafel mit den Gruppen aus IC `Stab.hpp`, Werte ↔ Spiel, Adapter Rechenweg Note (Dämpfer ab `halten` wie beim Flügel; Spuren `motor`, `bowed`, `malletOffset`, `damperPressure` — im Loop erst mit 31g; ohne Spur `motor` kein Motor), `stufeAus` → `kern::anschlagStufe`; die zehn Vorgaben in `stab/Vorgaben.cpp` mit Quellen-Schlüsseln (Steeldrum eingeschlossen) | je Instrument der Ton Bit für Bit gleich IC `abdruck_msvc.txt` (11 Abdrücke); die Fälle aus IC `test_stab` grün |
| 34g-4 | Stimmer `bar` in `InstrumentMeasure/Stimmen`: `partnerAbstrahl` (KS2 a), Bogenfenster (KS3 a, Tonhöhe dort gemessen); `InstrumentTuneTool --create <kern> <vorgabe> <datei>` | Fenster Bit für Bit gleich IC `Stimmung.inc`, `partnerAbstrahl` gleich der IC-Vorgabe; Wächter Stimmung und Bezug grün, rot bei absichtlich veralteten Werten (Schreibweg erstmals an einem stimmenden Kern, J30) |
| 34g-5 | die neun `.lvinst` (`--create`, `--reference`, Kopf aus `instrument_katalog.py`), Register und `CREDITS.md`, `synth_…` in den Generatoren und Skript für die Loops (erst in einer Kopie, vergleichen), Pegel `--messung` und `--loops`, Zahlen je Art in den Tests | `Loop Werte` nur Namen geändert und neue Zeilen; `Klangfassung` nachgeführt; Quellen-Wächter grün |
| 34g-6 | `bars_demo.lvloop`; Hörproben `LUMIVIZ_34G_WAV=<Ordner>` (je Instrument tief/mitte/hoch bei pp/mf/ff, Synth gegen IC, `hoertest.json`); Fall `*34g Rechenzeit*` (Anteil und Verhältnis zum Flügel); IC-Fall B6 „Modalfit am Modell“ in `test_InstrumentMeasure`; Creator-Fall mit dem Vibraphon (Auswahl, Haken, Umfang aus den Werten; Bild) | Rechenzeit ≤ 4,6 % am schwersten Ton (RZ1-1 a, dreimal allein); Kennzahl im Bericht |
| 34g-7 | Modul-Doku, Journal J31 ff., Konzept, Bericht der Klasse `docs/modellierung/platte-stab/` (Rechenzeit, Kennzahl, Datum, Maschine), Pakete `instrumentcore` 0.3.0, `instrumentmodels` 0.2.0 | **voller Lauf** unter beiden Compilern (Core geändert, neue Instrumente; UF17-1 a) |

Abgeleitet (Muster 34d/34f, `vorgabeHammerSaite`): die Vorgaben bleiben als Herleitung im Code und als Vorgabe der Tafel;
die Datei schreibt einmal das Werkzeug, ein Wächter vergleicht mitgelieferte Datei und Vorgabe.

✅ **KS1 a** (Patrik S178, „alles a“): **Der Tastenumfang ist ein Wert des Instruments.** Die Tafel nennt zwei
Ganzzahl-Werte `noteMin`/`noteMax` (Gruppe *Keys*; Felder `umfangVon`/`umfangBis` wie `bezugWert`); `richteWerte` richtet
die Kurven auf diesen Umfang, `bezugsProbe` nimmt seine Mitte, die Kern-Ansicht zeigt ihn; die Tafel trägt den möglichen
Umfang (21–108); ein Kern ohne die Namen (Flügel) bleibt wie heute. Grund: nur so bleibt der Ton Bit für Bit gleich IC
(IC normiert an der Mitte des Instruments) und ein Kern ein Verfahren. Verworfen: b (Kern-Typ je Instrument — neun gleiche
Tafeln, Umfang eigener Instrumente fest), c (weitester Umfang in der Tafel — der Bezug wiche für 9 von 9 ab). Kippt, wenn
kein Kern außer `bar` je Instrument einen Umfang braucht (Klasse 10 braucht ihn).

✅ **KS2 a** (Patrik S178): **`partnerAbstrahl` ist ein gestimmter Wert**, das Ziel steht in der Tafel (`partnerGainDb` 15,
Quelle Rucz & Ulveczki 2021 S. 7; `partnerGainNote` 65 bzw. 84 bei der Celesta). Ein gestimmter Wert darf von gestimmten
Werten abhängen, die vor ihm in der Tafel stehen; gestimmt wird in dieser Reihenfolge, der Abdruck enthält die vorgelagerten
(die Bogenfenster veralten mit dem Partner). Grund: die belegte Größe ist die dB-Zahl, nicht das Gewicht; IK2-11 a „kein
stiller Ersatz“. Verworfen: b (Gewicht einmal gerechnet — nach einer Änderung still falsch), c (gestimmt, aber die Fenster
ohne die Abhängigkeit). Kippt, wenn das Stimmen gemessen deutlich über einer Sekunde je Instrument liegt (dann wann, nicht
wie).

✅ **KS3 a** (Patrik S178): **Der Bogen am Vibraphon zieht ganz um** — `Reibung` nach `InstrumentModels/gemeinsam`, die
Fenster als gestimmte Werte (Stimmer in `InstrumentMeasure`, Tonhöhe dort gemessen), gespielt über die Spur `bowed` (0/1 am
Beginn der Note), gestrichen ab `bowedFrom` (Ganzzahl, 63, Herkunft: IC-Test der oberen Lage), darunter der Anschlag.
Prüfung: Fenster gleich `Stimmung.inc`, Ton gleich `stab_Vibraphon_gestrichen_m72_mf`, ±10 Cent wie in IC, Rechenzeit
gestrichen ≤ 4,6 %. Grund: ein vollständiger, in IC belegter Satz; IK2-7 a schließt nur die tiefe Lage aus. Verworfen: b
(Bogen mit Klasse 3 — 10 von 11 Abdrücken, später alles zweimal anfassen). Kippt, wenn die Rechenzeit gestrichen über
4,6 % liegt.

✅ **KS4 a** (Patrik S178): **Namen** — Kern `bar`; `marimba`, `xylophone`, `vibraphone`, `glockenspiel`, `tubular_bells`,
`celesta`, `kalimba`, `tine_electric_piano` (Rhodes), `reed_electric_piano` (Wurlitzer); umbenannt `marimba` →
`synth_marimba`, `vibraphone` → `synth_vibraphone`, `kalimba` → `synth_kalimba` (Loops `bells_demo`, `dnb`, `hiphop`,
`lofi`, `trap`, `waltz`, die Generatoren, Katalog, Pegel); die Steeldrum umgezogen, nicht ausgeliefert (Deluxe, nicht in
A). Verworfen: b (`electric_piano`/`reed_piano` — zu allgemein neben CP-70 und Clavinet in 34i), c (Steeldrum als
`steelpan` — Tonfelder sind Platzhalter, IC `Vorgaben.cpp:412`). Kippt, wenn Patrik die Steeldrum trotzdem hören will.

**Teilplan 34h** (S179, Zuschnitt nach IK2-8 a, IK2-10 a, TM1 a–TM4 a; Nummern fest). Gefunden: IC `gemeinsam/Membran` und
`klassen/05_membran/` mit sieben Vorgaben (Tom, Pauke, Bassdrum, Snare, Tabla, Conga, Bongo); IC führt nur zwei Abdrücke
(`trommel_Tom_mf`, `trommel_Pauke_m52_mf`) — ein Programm gegen die IC-Quellen unter Windows-clang (S179) rechnet die
übrigen ohne Änderung am IC-Repo; der Belag der Tabla ist in IC eine Funktion (`std::function`) und wird drei Zahlen
(σ 2,57, k 0,492, ξ 0,091); gemessen (clang -O2, S179): die sieben Vorgaben mit `normiere` 1,4 s, Rechenzeit ff höchstens
3,73 % (Bassdrum); `conga` kollidiert (`genres/afrobeat`; `flute_tropics` spielt das Sample `lib:conga_high`); eine Kit-Rolle kennt keine Spielweise;
IC normiert mit 0,5 s, LumiViz mit 1 s (die Spitze liegt am Anfang — 34h-3 prüft den Bezug).

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34h-1 | `InstrumentModels/gemeinsam/Membran` per Skript aus IC (Anker je genau einmal), nutzt `zerlegeSymmetrisch` des Core; Fälle aus IC `test_trommel` (Bessel-Verhältnisse, Pauke gegen Packer, Paar zweier Felle, Tabla harmonisch) | die übernommenen Fälle grün mit den IC-Grenzen |
| 34h-2 | Bezug aus IC: das Programm als Beleg unter `docs/modellierung/anbindung-ic/` mit Baubefehl; `werte/trommel_ic.txt` (sieben Abdrücke, die Spielweisen der Snare, die sieben Bezüge) | Tom und Pauke gleich IC `abdruck_msvc.txt` |
| 34h-3 | Kern `membrane` unter `InstrumentModels/trommel/` (`lumi::kern::trommel`): `Trommel.cpp`, `Vorgaben.cpp` per Skript; Tafel (Gruppen *Keys* mit Umfang nach KS1 a und *Tunable*, *Heads* als Liste mit Belag als Werte, *Shell*, *Stroke*, *Glide*, *Snares*, *Noise*, *Roll*, *Output*), Bezug mit zwei Runden (IK2-10 a); Adapter nach TM1 a und TM2 a; neu `nachklang` aus den T60 der Moden (IC kennt keinen); abgeleitet: das Glissando der Pauke folgt der Spur `pitch` (die IC-Rampe ist der Fall mit zwei Punkten, kein Abdruck enthält ihn) | alle Abdrücke aus 34h-2 Bit für Bit unter Release-clang und Testing-MSVC; die IC-Fälle aus `test_trommel` grün |
| 34h-4 | die sieben `.lvinst` (Schreiber des Programms wie 34g, Bezug über `InstrumentTuneTool --reference`; keine gestimmten Werte), Katalog, Register und `CREDITS.md`, `conga` → `synth_conga` (Skript zuerst in einer Kopie), Pegel nur neue Zeilen, `loop_werte.txt` nur geänderte Zeilen | `Loop Werte` nur Namen geändert und neue Zeilen; `Klangfassung` nachgeführt; Quellen-Wächter grün |
| 34h-5 | feste Spurwerte je Kit-Rolle und Drum-Spur (TM2 a): `.lvkit` Fassung 2, `.lvloop` Fassung 18, der Tonname trägt sie, der Kitwechsel nimmt sie mit | Fälle zu Datei, Kitwechsel und Ton; Datensicherung und Publish grün |
| 34h-6 | Kit `modelled_drums` (TM3 a) über `make_kits.py` (Pegel nur neue Zeilen), `membrane_demo.lvloop`, Hörliste `hoertest/listen/membran_34h` (Stufe −18/−9/0, Spielweisen, `synth_conga` gegen `conga`), Fall `*34h Rechenzeit*` (Bassdrum), Creator-Fall mit der Snare | Rechenzeit ≤ 4,6 % dreimal allein (RZ1-1 a); `--liste-pruefen` gültig |
| 34h-7 | Modul-Doku, Journal J42 ff., Konzept, Bericht `docs/modellierung/membran/README.md`, Pakete | **voller Lauf** unter beiden Compilern (Dateifassungen, neue Instrumente; UF17-1 a) |

✅ **TM1 a** (Patrik S179, „alle a“): **Die Geschwindigkeit folgt der Stufe in dB wie bei Hammer und Stab** —
`tempo = tempoFf · anschlagFaktor(stufe)`, `tempoPp` fällt aus der Tafel; die IC-Abdrücke prüft der Test über den
direkten Aufruf mit `staerke`, der Adapter gegen denselben Aufruf mit der Geschwindigkeit. Gemessen (RMS 1 s, Stufe −20
gegen 0): a −20,0 bis −23,9 dB, Umfang −40 … 0 30,9–49,8 dB; die IC-Abbildung −9,1 bis −11,3 dB, Umfang 18,2–23,9 dB.
Grund: die Stufe ist die Lautstärke der Note in dB — eine Note bei −20 dB wird rund 20 dB leiser wie jedes Instrument.
Befund dabei: die Snare wird unter etwa −20 kaum leiser (−40 gegen −30: 3 dB, Schwerpunkt 5,4–5,9 kHz), weil IC das
Rauschen des Teppichs durch die Geschwindigkeit teilt (`Trommel.cpp:194`, offen) — er bleibt wie in IC und steht im
Bericht. Verworfen: b (IC-Abbildung `staerke = 1 + stufe/40` — Trommeln zusammengedrückt gegen alle anderen), c (wie a,
nicht leiser als pp — 22 gleiche Töne unter verschiedenen Namen). Kippt auf c, wenn Patrik die Snare unter −20 hören will,
bevor der Teppich gemessen ist.

✅ **TM2 a** (Patrik S179): **Spielweisen als Spuren des Kerns, feste Werte je Kit-Rolle und Drum-Spur** — der Kern liest
`stroke` (0 normal, 1 Mitte, 2 Rand, 3 Randschlag, 4 Reifen, 5 gedämpft; am Beginn der Note), `roll` (0/1, Wirbel über
die Länge der Note), `snaresOff` (0/1) und `pitch` (Pauke); Kit-Rolle und Drum-Spur tragen feste Werte dafür (`.lvkit`
Fassung 2, `.lvloop` Fassung 18), der Tonname trägt sie, der Kitwechsel nimmt sie mit; die Rolle *rim* ist `snare_drum`
mit `stroke` 4 (Cross-Stick, wie `lib:rim`, `lib:drs_rim`). Werte je Schritt bleiben bei 31g. Grund: das Rollenmodell
der Kits führt *rim* schon als „Snare, so gespielt“. Verworfen: b (Spuren nur im Kern wie 34g, Kit ohne Spielweisen — das
Lieferstück unvollständig), c (Spielweise als Wert des Instruments — drei Dateien mit denselben Werten). Kippt auf b,
wenn 31g Spurwerte je Rolle anders lösen soll.

✅ **TM3 a** (Patrik S179): **Das Kit führt nur Klasse 5** — kick Bassdrum, snare Snare, rim Snare Cross-Stick, snare 2
Snare Randschlag, tom 1 Tom, perc 1 Conga, perc 2 Bongo, perc 3 Tabla; ein Kitwechsel lässt Hats, Ride und Crash einer Spur
stehen (K2 a), mit Klasse 6 kommen die Becken in dieses Kit. Verworfen: b (Becken aus DRSKit — ändern sich mit Klasse 6
in Klang und Datei). Kippt, wenn Patrik das Kit vor Klasse 6 allein spielen will.

✅ **TM4 a** (Patrik S179): **Namen** — Kern `membrane`; `low_tom`, `timpani`, `bass_drum`, `snare_drum`, `tabla`, `conga`,
`bongo`; `conga` → `synth_conga` (Titel „Synth Conga“); Kit `modelled_drums` („Modelled drums“). Grund: `low_tom` hält, wenn
weitere Größen kommen (GM). Verworfen: b (`tom` — mehrdeutig mit dem zweiten Tom). Kippt, wenn es bei einem Tom bleibt.

✅ **QM1 a** (Patrik S179): **Quellen am Wert für alle Werte, in der Datei** — `Werte` trägt auch zu Zahlen, Ganzzahlen,
Schaltern, Wahlen und Listen die Quellen (eine Kurve weiter ihre eigenen); die Vorgaben setzen sie je Instrument (Trommel,
Stab mit Celesta/Wurlitzer nach Stulov und Rhodes nach Sønderbo, Flügel); `.lvinst` **Fassung 23** mit `coreSources`
(Name → Schlüssel, immer geschrieben); `make_instrument_credits.py` liest sie, der Wächter prüft je Instrument. Grund: WZ1 a
— die Datei trägt ihre Herkunft; tafelweit stünde in `CREDITS.md` an 40 von 63 Stellen „benutzt in“, wo es nicht stimmt
(je Trommel sind 2–5 der 9 Quellen benutzt), und seit 34g fehlt Stulov bei Celesta und Wurlitzer. Verworfen: b (Quellen
ohne Kurve aus dem Code, `InstrumentTuneTool --credits` — die Herkunft halb in der Datei, halb im Code; ein abgeleitetes
eigenes Instrument wüsste nicht, woher seine Zahlen kommen), c (tafelweit — 40 falsche Angaben in einer ausgelieferten
Datei). Kippt auf b, wenn eigene Instrumente ihre Herkunft nicht tragen sollen.
**Gebaut S180** (Journal J50–J52): `Werte::quellen` (nicht im Vergleich, nicht im Abdruck; nur an den Werten eines
Instruments, eine Liste trägt sie als Ganzes), `coreSources` in Fassung 23, die Quellen je Vorlage in `InstrumentModels`
(`quellenDerTrommel`, `quellenDesStabs`, `werteVon`; die Tafel trägt die der ersten Vorlage), die 17 Kern-Dateien ohne
neu zu stimmen (`LUMIVIZ_KERN_QUELLEN_SCHREIBEN=1`), `CREDITS.md` mit 19 Quellen; InstrumentCore 0.4.0, InstrumentModels
0.4.0; voller Lauf 1837 / 1837 unter Release-clang und Testing-MSVC.

**Teilplan 34i** (S181, Zuschnitt nach IK2-8 a, IK2-10 a, KS1 a, FU1 a, KL1 a–KL4 a; Nummern fest). Gefunden: Upright,
Honky-tonk und CP-70 sind in IC Vorgaben auf dem Flügel-Kern (`klassen/01_hammer_saite/VorgabenRest.cpp`, aufgebaut auf
IC `hammerSaiteVorgabe()`); das Clavinet rechnet auf einem zweiten Kern (`Saite.hpp/.cpp`, `SaitenVorgaben.cpp`: Tangente
nach Gabrielli et al. 2013, Tonabnehmer, Verstärker-Shelves, Dämpferschieber, Tastendruck mit Grenze 0), der auch
Hackbrett und Cimbalom trägt (Schlägel, Mittelsteg 3 : 2, Mitschwingen je Clip; Deluxe); die Bezüge stehen alle in IC
`abdruck_msvc.txt` (`klasse1_*_m60_a-12` sechs, `saite_Hackbrett_clip_C4_G4`, `hammer_vorgabe_m*_a-12_d500` fünf) — kein
Beleg-Programm wie in 34h; ob die LumiViz-Vorgabe des Flügels gleich der IC-Vorgabe ist, belegen erst die fünf
`hammer_vorgabe`-Abdrücke (34i-1); IC rechnet Upright, Honky-tonk, Hackbrett, Cimbalom ohne Boden (Stegkraft), der CP-70
fachlich richtig ohne (Piezo); Rechenzeit in IC (S172): CP-70 2,09 %, Upright 1,88 %, Clavinet 0,64 %. **Die Messung der
Rechenzeit ist ausgesetzt, bis die Kühlung des Rechners erneuert ist (Patrik S181)** — die Fälle werden gebaut, die
Zahlen nachgeholt.

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34i-1 | Beleg: die LumiViz-Vorgabe `hammerSaiteVorgabe` gegen die fünf IC-Abdrücke `hammer_vorgabe_m{21,40,60,84,100}_a-12_d500`; dann `baueVorgabeUpright/Honkytonk/Cp70` per Skript nach `InstrumentModels/hammer/` (Anker je genau einmal), Quellen je Vorlage (QM1 a), Pedalwerte aus IC `Tastatur` | 5 + 3 Abdrücke Bit für Bit unter Release-clang und Testing-MSVC; die IC-Fälle Upright, Honky-tonk, CP-70 aus `test_hammer_rest` grün. **Haltepunkt:** weicht die Flügel-Vorgabe ab, Ursache als Vorlage |
| 34i-2 | Umfang in der Tafel des Flügel-Kerns nach KL4 a | Flügel-Bezug, `Loop Werte`, Klang von `grand_piano` und den Glasflügeln Bit für Bit; `klangfassung.txt` nur in der Spalte des Abdrucks; CP-70 auf 28–100 gleich dem IC-Abdruck |
| 34i-3 | Kern `struckString` nach KL1 a unter `InstrumentModels/saite/` (`lumi::kern::saite`), `Saite` und `SaitenVorgaben` per Skript; Tafel (*Keys*, *String*, *Decay*, *Course*, *Excitation*, *Tangent*, *Damper*, *Pickups*, *Amp*, *Output*), Bezug über `normiere` (IK2-10 a), Adapter Rechenweg Note mit Spuren `damperSlider` (0 … 1) und `keyPressure` (Grenze 0), Wahl der Tonabnehmer als Ganzzahl | Clavinet, Hackbrett, Cimbalom und der Clip Bit für Bit unter beiden Compilern; die IC-Fälle zum Clavinet und Hackbrett grün |
| 34i-4 | die vier `.lvinst` nach KL2 a (Schreiber des Programms, Bezug über `InstrumentTuneTool --reference`), Katalog, Register und `CREDITS.md`, Pegel und `loop_werte.txt` nur neue Zeilen, `klangfassung.txt` nachgeführt; keine Umbenennung (kein Name kollidiert) | `Loop Werte` nur neue Zeilen; Quellen-Wächter grün |
| 34i-5 | ~~Körper `pleyel_p131` nach KL3 a~~ — entfällt nach KL3-1 a bis zu einem belegten Bauplan (Recherche) | — |
| 34i-6 | `keys_demo.lvloop`; Hörliste `hoertest/listen/klasse1_34i` (je Instrument tief/mitte/hoch bei −18/−9/0, Soft und Practice am Upright, `clavinet_modal` gegen `clavinet`, Upright gegen `grand_piano`); Creator-Fall mit dem Upright (Pedale, Umfang; Bild); Fall `*34i Rechenzeit*` | `--liste-pruefen` gültig; Rechenzeit ≤ 4,6 % (RZ1-1 a) nach dem Tausch der Kühlung nachgeholt |
| 34i-7 | Modul-Doku, Journal J53 ff., Konzept, Bericht der Klasse unter `docs/modellierung/hammer-saite/`, Paket `instrumentmodels` 0.5.0 | **voller Lauf** unter beiden Compilern (neuer Kern, neue Instrumente; UF17-1 a) |

✅ **KL1 a** (Patrik S181, „a“ auf KL1–KL4): **Der ganze IC-Kern `Saite` zieht als Kern `struckString` um**
(`InstrumentModels/saite/`, `lumi::kern::saite`) — Tangente, Schlägel, Mittelsteg und `rechneSaitenClip`; ausgeliefert nur
das Clavinet, Hackbrett und Cimbalom als Vorgaben ohne Datei (wie die Steeldrum, KS4 a); der Adapter für Mitschwingen je
Clip mit dem Rechenweg Clip (31e/34k). Grund: alle drei Abdrücke und der Clip sofort Bit für Bit belegt, der Kern in einer
Fassung, IC kann den Port ganz streichen (34l); der Name nennt das Verfahren (angeschlagene Saite), nicht ein Instrument.
Verworfen: b (nur die Tangente als `tangentString` — der Kern später ein zweites Mal geöffnet, ohne Beleg aus der ersten
Übernahme). Kippt auf b, wenn Hackbrett und Cimbalom nie ausgeliefert werden sollen.

✅ **KL2 a** (Patrik S181): **Namen** — `upright_piano`, `honky_tonk_piano`, `electric_grand_piano` (CP-70, ohne Marke),
`clavinet_modal` (IC); das heutige `clavinet` bleibt bis nach den Hörproben (IK2-8 a): ist das Modell besser, wird es
`clavinet` und das heutige `clavinet_waveguide` (Skript für die Loops); Demo `keys_demo`. Verworfen: b (sofort
umbenennen — nimmt das Hörurteil vorweg). Kippt bei einem anderen Wunschnamen.

✅ **KL3 a** (Patrik S181): **Upright und Honky-tonk bekommen einen eigenen Körper `pleyel_p131`** — Bauplan nach Chaigne,
Cotté & Viggiano 2013 (Maße, Rippen, Holz, Steglinie aus dem Paper abgelesen, nicht geschätzt), gerechnet mit
`InstrumentBodyTool` wie der Steinway; Voraussetzung: Patrik beschafft das Paper nach `quellen/`; bis dahin `body` leer.
Der CP-70 bleibt ohne Körper (Piezo misst die Stegkraft). Die Abdrücke hängen nicht daran (FU1 a: die App faltet den
Körper an der Schicht). Grund: nur so trägt der Upright den Boden seines Instruments. Verworfen: b (leer wie IC — ein
Klang ohne Abstrahlung neben `grand_piano` mit Boden), c (`lib:steinway_d` — ein Flügelboden unter einer Mensur von
1,19 m). Kippt auf b, wenn das Paper nicht zu bekommen ist oder die Steglinie auch dort nur als Bild ohne Maße vorliegt.

✅ **KL3-1 a** (Patrik S181, Kipppunkt von KL3 a erreicht): **Upright und Honky-tonk gehen ohne Boden mit** (Ausgang
Stegkraft wie IC, `body` leer); der Körper bleibt offen mit benannter Lücke, dazu eine Recherche nach einem Upright-Bauplan
mit Maßen; gebaut wird er, sobald ein belegter Satz da ist. Befund (34i-5): Chaigne, Cotté & Viggiano 2013 geben mit
Zahlen Fläche 1,365 × 0,96 m (S. 2461), Platte 9 mm und zehn Rippen 2,1 × 2,5 cm (S. 2457), Rippenabstände (Tab. III),
Faser 131°, Balken 2 × 3,5 cm, Diskantsteg Buche 39 × 28 mm (S. 2458), Holz Tab. I, η 1,5 % (S. 2459); die Stege nur als
Bild ohne Achsen (Abb. 5 die vereinfachte Platte, Abb. 9 Foto, Abb. 10 Netz), die Stegpunkte je Taste und der Bassteg
fehlen; ebenso nur Fotos bei Boutillon & Ege 2013 (Atlas) und Ege, Boutillon & Rébillat 2013. Grund: der Körper färbt
jede Taste über ihren Stegpunkt — geraten wäre gerade dieser Teil, und eine falsche Färbung je Taste ist im Hörtest
nicht von einer richtigen zu unterscheiden. Verworfen: b (aus Abb. 5 abgelesen, Stegpunkte als Annahme — ein geflickter
Satz). Kippt auf b (als „gedachtes Instrument“ gekennzeichnet), wenn die Recherche ergebnislos bleibt und der Hörtest das
Upright ohne Boden als unbrauchbar zeigt. Die Teilschritt-Zeile 34i-5 entfällt damit bis zum Fund.

✅ **KL4 a** (Patrik S181): **Der Flügel-Kern bekommt den Umfang in der Tafel** — `noteMin`/`noteMax` in der Gruppe *Keys*
wie `bar` (KS1 a); die Flügel-Dateien tragen 21/108; für Kern-Instrumente liest die Planung den Umfang aus der Tafel,
`keyboard.range` fällt dort weg (der Block behält Pedale und `bassSustainBelow`). Grund: ein Ort für den Umfang aller Kerne
(der Kipppunkt von KS1 a ist mit dem CP-70 eingetreten). Verworfen: b (nur `keyboard.range` wie IC `Tastatur` — zwei Orte
für denselben Wert). Kippt auf b, wenn das Richten der Kurven auf 28–100 den CP-70-Abdruck aus IC verändert.

✅ **KL4-1 a** (Patrik S181, nach dem Befund in 34i-2): **Kurven werden nur auf den Umfang der Tafel gerichtet; der Umfang
des Instruments verschiebt keine Stützpunkte**, er begrenzt Spiel, Bezugsprobe und Ansicht (KS1 a berichtigt,
InstrumentCore 0.5.0). Befund: `richtePaare` schob jeden Stützpunkt außerhalb auf den Rand — am CP-70 (28–100) wichen
danach B an 22 von 73 Tasten bis 95 %, die Länge an 55, der Dämpfer an 62 Tasten ab, der Ton an Taste 60 um −36 dB
(Differenz gegen den Ton); gerichtet nur auf die Tafel blieben alle Abdrücke von `bar`, `membrane` und Flügel Bit für Bit
(S181 nachgerechnet), anders nur die Lage konstanter Kurven in `kalimba` und `reed_electric_piano` (Werte gleich; neu
geschrieben). Verworfen: b (Umfang nur in `keyboard.range` — der Mangel bliebe für jedes Instrument mit eigenem Umfang,
zwei Orte), c (die Kurven des CP-70 auf 28–100 abtasten — `exp(log(x))` nicht sicher bitgleich, 73 Punkte je Kurve).
Kippt, wenn das Neuschreiben andere gestimmte Werte ergibt.

✅ **TT1 a** (Patrik S181, nach dem Bild des Creators in 34i-6): **Die Tafel des Kerns `hammerString` heißt „Hammer
string“** (bis S181 „Grand piano“) — der Titel nennt das Verfahren wie bei den anderen Kernen („Bar and plate“,
„Membrane“, „Struck string“); Upright, Honky-tonk und CP-70 zeigen im Creator nicht mehr „Core · Grand piano“. Der Titel
steht nicht im Abdruck der Werte (`Klangfassung` unverändert). Verworfen: b (lassen). Kippt, wenn der Kern im Creator als
Flügel benannt sein soll.

**Teilplan 34j** (S188, Zuschnitt nach IK2-2 a, IK2-8 a, IK2-10 a, IK2-11 a, SK4 a, ZV1 a–ZV3 a; Nummern fest). Gefunden
(Befund [2026-11-11_S188_befund_ic_vorlage.md](../modellierung/zunge/2026-11-11_S188_befund_ic_vorlage.md), am Ton der
Vorlage gemessen, Bau gleich IC `zunge_Akkordeon_m69_mf`): die Vorlage stuft über den Umfang nur die Länge der Zunge
(`Zunge.cpp:38–42`) und gibt jeder Taste bei gleicher Expression einen anderen Druck (Fenster je Note, `Zunge.cpp:45–51`)
— Lautheit über den Umfang 15,4–28,9 LU (Akkordeon Diskant, Melodica, Mundharmonika) und 48,9–52,8 LU (Harmonium), die
Bass-Seite des Akkordeons stumm, am Harmonium die Tasten 48–57 fast stumm und die Dynamik −0,6–5,6 LU, Bend −100 Cent
ergibt +8,0 Cent; mit einem Druck für alle Tasten ist die Lautheit eben (3–5 LU), aber die Ansprechschwelle steigt mit der
Tonhöhe (F3 unter 150 Pa, A6 ab rund 1 200 Pa). IC führt einen Abdruck (`zunge_Akkordeon_m69_mf`) — die übrigen rechnet
ein Programm gegen die Vorlage wie in 34h; der Kern ruft `messung::tonhoehe` und `einschwingen` selbst
(`Zunge.cpp:109–121`), `stimme` läuft auf `parallelJe` (IC: alle vier 1:41 min auf vier Threads); IC normiert mit 0,6 s
bei Expression 1 an der Mitte oberhalb der Teilung, LumiViz mit 1 s bei Stufe 0; die App gibt einem Kern eine Note, die
Stufe und feste Spurwerte am Beginn (`Instrument.cpp:2964–2969`) und nimmt die Expression danach als Pegel
(`Instrument.cpp:3067 ff.`), der Tonname trägt die Spuren schon (`Loop.cpp:644–647`); der Kern `bar` nennt `expression`
in seiner Auskunft (Bogen) und bleibt beim Pegel; die Quellen der Klasse liegen nicht unter `…\cmake\quellen`, Aufnahmen
keine; kein Name kollidiert. Die Rechenzeit ist ausgesetzt (Kühlung): die Fälle werden gebaut, die Zahlen nachgeholt.

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34j-1 | Beleg aus der Vorlage: Programm mit Baubefehl unter `docs/modellierung/anbindung-ic/` (Muster `abdruck_trommel.bat`), `werte/zunge_ic.txt` — je Instrument tief/mitte/hoch bei Expression 0 / 0,5 / 1, Vibrato, Bend, Balgwechsel, Bass-Seite, die vier Bezüge, die Kurven aus `Stimmung.inc` | Akkordeon m69 mf gleich IC `abdruck_msvc.txt` |
| 34j-2 | Kern `freeReed` unter `InstrumentModels/zunge/` (`lumi::kern::zunge`), Tafel „Free reed“: `Zunge.cpp`, `Vorgaben.cpp` per Skript (Anker je genau einmal); das Messen zieht aus dem Kern (IK2-2 a): innere Größe `reedTip`, gemessen wird im Stimmer; Tafel (*Keys* mit Umfang nach KS1 a und `bassBelow`, *Reed*, *Chamber*, *Wind*, *Ranks* und *Bass ranks* als Listen, *Case*, *Tunable*, *Output*); Bezug über `normiere` (IK2-10 a — Probe gegen den IC-Bezug prüfen, IC normiert anders); Adapter Rechenweg Note, in diesem Schritt wie IC (feste Werte am Beginn) | alle Abdrücke aus 34j-1 Bit für Bit unter Release-clang und Testing-MSVC; die IC-Fälle aus `test_zunge` grün |
| 34j-3 | Stimmer `freeReed` in `InstrumentMeasure/ZungeStimmen` (f₀ und Fenster des Drucks je Note, IK2-11 a), Dauer gemessen | Kurven Bit für Bit gleich `Stimmung.inc` für alle vier; Wächter Stimmung rot bei absichtlich veralteten Werten |
| 34j-4 | Recherche und Quellen (ZV1 a): die freien Arbeiten aus `../modellierung/zunge/ic/quellen.md` nach `…\cmake\quellen\zunge\` (Ladeliste vorher zur Freigabe), Register, `archiv_index.py`; gesucht: Maße der Stimmplatten je Ton (Länge, Breite, Dicke, Aufbiegung), Basszungen (Gewichte), Kanzellen je Ton, Harmonium- und Mundharmonika-Zungen, gemessene Balg- und Atemdrücke, Einschwingzeiten, Bend nach Adachi et al.; Agenten ohne Browser; Bericht unter `../modellierung/zunge/recherche/` | Bericht mit Fundstelle je Größe (Seite, an der Abbildung gegengeprüft). **Haltepunkt:** je Instrument, ob die Quellen für die Herleitung reichen — sonst Vorlage (Kipppunkt ZV1 a) |
| 34j-5 | Herleitung am Kern aus 34j-2, ohne App: Vorgaben je Taste als Kurven, Windbereich je Instrument (ZV2 a), Basszungen, Harmonium, Bend; je Instrument jede Option gegen **alle** Zielgrößen gerechnet (spricht im Windbereich an, Lautheit über den Umfang, Stimmung über den Windbereich, Dynamik, Einschwingzeit, Amplitude über dem Druck gegen Cottingham, Lilly, Reed 1999 Abb. 2 (ZH3 a), Tonhöhe über den Druck gegen Adachi und Cottingham 1999; dazu das belegte Verhalten nach ZH1 a); Dokument `../modellierung/zunge/<Datum>_vorgaben_je_taste.md` | Entscheide je Instrument (ZV4 ff.). **Haltepunkt:** was kein Instrument trägt, wird nicht ausgeliefert, mit benannter Lücke |
| 34j-6 | Kern Rechenfassung 2 nach 34j-5: die Größen je Taste als Kurven der Tafel mit Quelle und Belegkraft, Wind nach ZV2 a (`windMin`, `windMax`; f₀ am mittleren Druck gestimmt; die Fenster als Prüfgröße), Stimmer neu; Wächter „jede Taste deckt den Windbereich“; `zunge_ic.txt` mit Vermerk neu (Journal); Messfall `LUMIVIZ_34J_MESSUNG=<datei.md>` mit der Messkette des Befunds S188 (Lautheit aus `AudioCore`) | die Zielgrößen aus 34j-5 am gebauten Kern wie am Prüfstand; die IC-Fälle angepasst mit Grund im Journal |
| 34j-7 | Laufender Wind (ZV3 a): der Kern liest `expression`, `vibrato` und `bellows` (Richtung 0/1, am Wechsel der Druckeinbruch) über die Zeit, `pitch` über die Kurve Ventilöffnung → Cent je Taste aus dem Stimmer, gekappt an der Grenze der Taste (ZV7 a); Stärke = Stufe mal Expression (SK4 a), die Abbildung Stufe → Stärke an der gemessenen Kurve Pegel über Druck festgelegt (Grundsatz TM1 a: eine Note bei −20 dB klingt rund 20 dB leiser, begrenzt durch die Dynamik des Instruments); `nachklang` aus dem Ausklang der Zunge (gemessen 31–777 ms), gegen die Messung geprüft; `KernAuskunft` bekommt ein Feld für dauernde Anregung (nicht die Liste der Spuren — `bar` nennt `expression` für den Bogen), `InstrumentCore` nächste Fassung; die App gibt solchen Kernen die Spuren von Muster und Clip in den Tonauftrag und nimmt die Expression nicht mehr als Pegel. Die Spuren `bellows` im Loop erst mit 31g (wie `motor`) | konstante Spur Bit für Bit wie 34j-6; ein Crescendo am gehaltenen Ton ändert Pegel und Teiltöne; `Loop Werte`, `Klangfassung` und Flügel-Bezug unverändert (kein anderer Kern betroffen) |
| 34j-8 | die `.lvinst` der Instrumente, die 34j-5 trägt — `accordion`, `harmonium`, `melodica`, `harmonica` (IK2-8 a): Schreiber `LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN=1`, `InstrumentTuneTool --all --reference`, Katalog, Register und `CREDITS.md`, Pegel und `loop_werte.txt` nur neue Zeilen, `klangfassung.txt` nachgeführt, Zahlen je Art in den Tests | `Loop Pegel LP3 a` ohne Ausnahme; `Loop Werte` nur neue Zeilen; Quellen-Wächter grün |
| 34j-9 | `free_reeds_demo.lvloop` (Crescendo über die Expression am Clip, Musette), Loops je Instrument über `make_modelled_loops.py`, Lautstärke nach LV1 a (Skript zuerst in einer Kopie); Hörliste `hoertest/listen/zunge_34j` am Hörort (je Instrument tief/mitte/hoch bei −18/−9/0, Crescendo, Vibrato, Balgwechsel, Musette gegen einen Chor, Bass-Seite), sie löst `ic/10_zunge` ab (wie LG1 a); Creator-Fall mit dem Akkordeon (Listen der Chöre, gestimmte Werte; Bild); Fall `*34j Rechenzeit*` (Akkordeon, Bass-Seite und Musette) | `--liste-pruefen` 0 Fehler, 0 Hinweise; Rechenzeit ≤ 4,6 % (RZ1-1 a) nach dem Tausch der Kühlung nachgeholt |
| 34j-10 | Modul-Doku, Journal J66 ff., Konzept, Bericht der Klasse `../modellierung/zunge/README.md`, Pakete `instrumentcore` und `instrumentmodels` (nächste Fassung), Steckbrief; `klassen/10_zunge` und `werkzeuge/stimme_zunge.cpp` unter `harvest/` gelöscht | **voller Lauf** unter beiden Compilern (neuer Kern, Core und Rechenweg der App geändert; UF17-1 a) |

Prüfung je Teilschritt nach MT1 a (eng unter Release-clang, beide gebaut). Modell je Sitzung: 34j-1 bis 34j-3 und 34j-6
bis 34j-10 Opus; 34j-4 und 34j-5 Fable (Herleitung aus mehreren Quellen).

Abgeleitet: Namen nach IK2-8 a (Kern nach dem Verfahren, wie `bar`, `membrane`, `struckString`); Stärke nach SK4 a; die
Hörliste von LumiViz löst die von IC ab wie bei den Klassen 1, 4, 5 (LG1 a).

**Nicht in 34j** (gebilligt Patrik S188; in IC nicht gebaut, `../modellierung/zunge/ic/offen.md`; jede Zeile braucht Bedienung aus 31g oder eine
Quelle, die fehlt): Register als Spur (F-4 a — die Chöre stehen fest in der Tafel), Bisonorik und Bellows shake (F-3 a),
Prolongement, Expression-Register und Percussion des Harmoniums (F-7 a), Bending und Overblow der Mundharmonika über den
Vokaltrakt (F-6 a), Cassotto und Verdeck.

✅ **ZV1 a** (Patrik S188, „Alle nach Empfehlung“): **34j zieht den Kern zuerst Bit für Bit um und leitet danach die
Vorgaben je Taste mit Quellen her wie LP2 b** — Zunge (Länge, Breite, Dicke, Aufbiegung), Kanzelle und Kanal je Taste,
Basszungen, Harmonium, Bend nach Adachi. Zielgrößen je Instrument: jede Taste spricht im Windbereich an, Lautheit über den
Umfang ≤ 15 LU, Stimmung ±10 Cent, Dynamik und Einschwingzeit im Bericht; ausgeliefert wird ein Instrument erst, wenn es
sie hält; Haltepunkt nach der Recherche. Grund: der wortgleiche Umzug fällt in allen gemessenen Zielgrößen durch
(15–53 LU, halbes Akkordeon, Harmonium ohne Dynamik), die Ursache ist von der Art, die LP2 b an Wurlitzer und Kalimba
behoben hat (41,9 → 10,8 LU, 37,5 → 1,4 LU). Verworfen: b (ausliefern, was spielt, vier Ausnahmen in LP3 a — Hörlisten,
Pegel, Loops und Abdrücke zweimal, der Hörtest an bekannten Fehlern), c (nur Beleg, Herleitung als spätere Stufe — der
Kern ohne spielbares Instrument, dieselben Dateien zweimal geöffnet). Kippt auf c, wenn die Recherche für kein Instrument
Maße je Taste hergibt; auf b, wenn Patrik die Klasse vor der Herleitung hören will.

✅ **ZV2 a** (Patrik S188): **Die Stärke gibt einen Winddruck des Instruments, gleich für alle Tasten** —
p = p_min · (p_max / p_min)^Stärke, der Windbereich als belegter Wert der Tafel (Harmonium 0,2–1,5 kPa nach Puranik &
Scavone; Akkordeon und Atem aus der Recherche); f₀ wird am mittleren Druck gestimmt; die Fenster je Note bleiben
gestimmte Werte als Prüfgröße, ein Wächter meldet jede Taste, deren Fenster den Windbereich nicht deckt. Berichtigt IC
F-2 a („Weg über den Tonumfang“). Grund: ein Balg hat einen Druck (IC F-5 a); die Gegenprobe zeigt bei gleichem Druck
3–5 LU über den Umfang, wo die Zungen ansprechen, gegen 22–26 LU mit dem Fenster je Note (F3 283 Pa, A6 2 934 Pa bei
derselben Expression). Verworfen: b (Fenster je Note wie IC — jede Diskanttaste spricht an, aber mit Drücken bis 4,5 kPa,
die kein Balg liefert). Kippt auf b, wenn sich keine Zungenmaße finden, mit denen alle Tasten eines Instruments über
seinen Windbereich ansprechen.

✅ **ZV3 a** (Patrik S188): **34j baut den laufenden Wind** — der Kern liest Expression, Vibrato und die Balgrichtung als
Kurven über die Zeit; die App gibt einem Kern mit dauernder Anregung die Spuren von Muster und Clip in den Tonauftrag
und nimmt die Expression dann nicht mehr als Pegel; mit konstanter Spur bleibt der Ton Bit für Bit gleich; 34m nutzt
denselben Weg. Grund: SK4 a — bei dauernder Anregung ist die Expression die Stärke der Anregung, und der Balg ist am
Akkordeon die einzige Quelle der Dynamik; die Zunge ist der einfachere erste Fall (kein Rohr, keine Kette). Verworfen: b
(feste Werte am Beginn der Note, laufend erst mit 34m — Hörliste, Demo und Töne der Zunge entstünden in 34m neu). Kippt
auf b, wenn das Blech einen anderen Weg braucht als Spuren im Tonauftrag (Rechenweg Kette), sodass die App-Seite zweimal
gebaut würde.

**34j-4 erledigt (S190).** Quellenlage:
[2026-11-11_S190_quellenlage_34j-4.md](../modellierung/zunge/2026-11-11_S190_quellenlage_34j-4.md) — 28 Arbeiten unter
`…\cmake\quellen\zunge\` (`LADELISTE.tsv` dort), drei Agentenberichte unter `../modellierung/zunge/recherche/`; eine
Maßtabelle je Ton gibt es öffentlich für kein Instrument der Klasse. (In der Sitzung hießen die fünf Entscheide LL1, LL2,
HP1–HP3; diese Kürzel waren vergeben, sie sind hier als ZQ1, ZQ2, ZH1–ZH3 geführt.)

✅ **ZQ1 a** (Patrik S190): **Die elf Arbeiten, an denen IC am 26.10.2026 gelesen hat, kommen aus dem Cache jener
Sitzung in die Ablage**, mit sha256 und dem Fundort aus `ic/quellen.md`; neu geladen wird nur, was dort fehlt. Grund: die
Seitenangaben der IC-Berichte gelten für genau diese Bytes. Verworfen: b (alles neu laden — eine getauschte Fassung
verschöbe die Seiten, cs.bu.edu war im Oktober gesperrt). Kippt auf b je Datei, wenn eine Kopie unvollständig ist.

✅ **ZQ2 a** (Patrik S190): **Alle 16 von den Agenten genannten Arbeiten werden abgelegt**, auch die nachrangigen.
Grund: jeder Beleg der Berichte soll an einer abgelegten Datei nachprüfbar sein, die Fundorte sind unsicher. Verworfen:
b (nur was eine Vorgabe trägt). Milne 1930 wird nicht abgelegt (nicht gemeinfrei, archive.org nur Ausleihe).

✅ **ZH1 a** (Patrik S190, Haltepunkt 34j-4): **34j-5 leitet die Vorgaben je Taste für Akkordeon, Harmonium und
Mundharmonika aus den Ankern und der Schwellentheorie her.** Jeder Wert der Tafel trägt seine Belegkraft: *gemessen*
(Anker mit Quelle), *abgeleitet* (Kurve durch die Anker, Skalierung nach Fletcher 1993) oder *Annahme* (Windbereich des
Akkordeons, Atemdruck — nirgends gemessen gefunden). Geprüft wird am Kern gegen das belegte Verhalten: Schwelle rund
0,2 kPa an tiefen Harmoniumzungen, Tonhöhe −16 bis −24 Cent/kPa im Spielbereich, Zungen ab C6 ersticken nie,
Einschwingzeit A4 60–90 ms mit dem Finger; dazu die Zielgrößen aus ZV1 a. Was sie nicht hält, wird nicht ausgeliefert
(Haltepunkt 34j-5). Richter 1985 steht in `../Bibliothek_offen.md`; kommt das Buch, werden abgeleitete Werte daran
geprüft. Grund: die Recherche gibt keine Tabelle, aber prüfbare Zielgrößen — eine Herleitung, die Schwelle, Tonhöhe über
dem Druck, Ersticken und Einschwingzeit zugleich trifft, ist an mehr Messungen gebunden als die Vorlage. Berichtigt den
Kipppunkt von ZV1 a („kippt auf c, wenn die Recherche für kein Instrument Maße je Taste hergibt“): Anker und geprüftes
Verhalten genügen für den Eintritt in 34j-5. Verworfen: b (anhalten bis Richter oder eine Vermessung — der Kern ohne
spielbares Instrument, Zugang zur Bibliothek frühestens Frühling 2027), c (Patrik vermisst einen Stimmstock — bleibt
möglich, gemessene Werte ersetzen abgeleitete). Kippt auf c, wenn vor 34j-5 ein Stimmstock vermessen wird; auf b, wenn
sich in 34j-5 für kein Instrument eine Skalierung findet, die alle Zielgrößen hält.

✅ **ZH2 a** (Patrik S190): **Die Melodica wird mit derselben Skalierung hergeleitet wie das Akkordeon** (Umfang F3–C6,
Atemdruck als Annahme); alle ihre Werte tragen die Belegkraft *Annahme*, der Katalog nennt sie „ungemessen“ (wie IC
Z-1 a, T8 a); ausgeliefert wird sie nur, wenn sie die Zielgrößen hält. Grund: zur Melodica fand sich keine Fachquelle
und keine Messung; die Zunge hat dieselbe Bauart wie am Akkordeon. Verworfen: b (nicht bauen, Lücke benennen), c
(Patrik vermisst eine Melodica — bleibt möglich). Kippt auf c, wenn eine zur Hand ist.

✅ **ZH3 a** (Patrik S190): **Zielgröße Amplitude in 34j-5: die Spitze über dem Druck gegen Cottingham, Lilly, Reed
1999, Abb. 2** — rascher Anstieg an der Schwelle, danach fast konstant, Größenordnung 3–4 % der Länge an tiefen
Harmoniumzungen; die 15 % (Cottingham 2011, Braasch & Cottingham 2023, ohne Messkurve) bleiben als Obergrenze der
Plausibilität im Bericht. Grund: eine Zielgröße hängt an einer Messung; die 5–11 % der Vorlage sind damit kein Mangel.
Verworfen: b (15 % bleiben), c (keine Zielgröße). Kippt auf b, wenn eine Quelle eine Messkurve mit rund 15 % für
Akkordeonzungen liefert.

**34j-5 gerechnet (S190).** Herleitung und Zahlen:
[2026-11-11_S190_vorgaben_je_taste.md](../modellierung/zunge/2026-11-11_S190_vorgaben_je_taste.md), Prüfstand unter
`../modellierung/zunge/proben/` (rechnet Bit für Bit wie der Kern). Schließdruck = Masse je Fläche · ω₀² · Aufbiegung,
Schwelle bestenfalls 2ζ mal Schließdruck; mit Gesetzen je Taste sprechen alle vier Instrumente auf jeder Taste über
ihren Windbereich an (Akkordeon Diskant 41 / 41 / 41, Lautheit über den Umfang 2,4 / 1,7 / 4,1 LU; Harmonium
42 / 42 / 42; Mundharmonika 37 / 37 / 37; Melodica 32 / 32 / 32; Bass 8′ 23 / 24 / 24).

✅ **ZV4 a** (Patrik S190): **Der Kern behält seine Gleichungen; Rechenfassung 2 ändert die Vorgaben** — die Größen je
Taste als Kurven der Tafel (Länge, Breite, Masse je Fläche, Aufbiegung, Seitenspalt, Dämpfung, Kanal, Ventil,
Kanzelle) mit Belegkraft je Wert. Das Modell nach Adachi u. a. 2026 (Gl. 2–7) bleibt Prüfgröße an C4 im Prüfstand.
Grund: nachgebaut trifft es seine veröffentlichten Zahlen (265,17 gegen 265,1 Hz, 261,65 gegen 261,7 Hz, 1,68 gegen
rund 1,6 mm), über den Umfang aber spricht es mit der veröffentlichten Trägheit bei 0 / 6 / 6 von 15 Tasten an
(Endwert erst nach 2–3 s; gemessen an A4 60–90 ms), und mit vierfacher Trägheit wächst die Amplitude bis 160 % der
Zungenlänge — es hat keinen Begrenzer. Die Kern-Form: 15 / 15 / 15, 70–95 ms, 1–5 % der Länge. Verworfen: b (Adachis
Gleichungen im Kern). Kippt auf b, wenn sich für die Adachi-Form eine belegte Begrenzung der Amplitude findet (etwa
die Plattendicke).

✅ **ZV5 a** (Patrik S190): **Die Tonhöhe folgt dem Druck wie gemessen** (Satz D3: Ventil 6 mm² an C4, über den Umfang
gestuft; an C4 −11 Cent/kPa gegen −16 gemessen, Harmonium C3 −22 gegen rund −24). Die Zielgröße Stimmung aus ZV1 a
lautet damit: **±10 Cent am mittleren Druck über alle Tasten**; die Abweichung über den Windbereich ist eine
Berichtsgröße mit der Messung als Bezug (Akkordeon +4,8 … −16,7 Cent, Harmonium +4,9 … −24,0 Cent). Grund: das
Sinken mit dem Balgdruck ist an zwei Instrumenten gemessen und trägt den Bend über die halb gedrückte Taste; alle
Tasten sinken gemeinsam (ein Balg, ZV2 a). Verworfen: b (halbe Steigung, ohne Beleg), c (±10 Cent über den ganzen
Windbereich, Ventil offen — die Tonhöhe über dem Druck als Lücke, der Bend ohne Mechanismus). Kippt auf c, wenn die
Verstimmung bei ff im Zusammenspiel stört.

✅ **ZV6 a** (Patrik S190): **Die Bass-Seite des Akkordeons klingt in 8′ und 4′** (`bassChoere` {0; 1,0} und {+12; 0,7}
statt 16′ und 8′), die tiefste Zunge ist F1; Tasten und Teilung bleiben. Grund: der 16′ von F1 läge bei 21,8 Hz, die
tiefste echte Zunge um E1; im Prüfstand sprechen die Zungen F0–E1 bei 200 Pa nicht an (0 von 5), die der Lage 8′
23 / 24 / 24, die 4′-Lage liegt im Diskantgesetz. Verworfen: b (16′ bleibt, Wind ab 300 Pa — der Windbereich des
Diskants ohne Beleg enger), c (Tasten eine Oktave hinauf — die Teilung rückt in den Diskant). Kippt auf b, wenn eine
Quelle Zungen unter E1 belegt.

✅ **ZV7 a** (Patrik S190): **34j-7 baut den Bend über das Ventil für Akkordeon und Melodica.** Der Stimmer (34j-6)
misst je Taste am mittleren Druck die Kurve Ventilöffnung → Cent und ihre Grenze (letzte Öffnung, bei der die Zunge
anspricht) und legt beide als gestimmte Werte ab; der Kern bildet die Spur `pitch` über diese Kurve ab und kappt an
der Grenze der Taste (SK3 a: die Grenze trägt das Instrument); `bendGrenzeCent` als fester Wert entfällt, ebenso die
Abbildung der Vorlage (−100 Cent → 1 % der Öffnung). Dass der Bend bei höherem Druck tiefer wird, bleibt. Grund: mit
dem Ventilgesetz aus ZV5 a verhält sich der Bend wie gemessen — bei 566 Pa bis −33 Cent an F3, −25 an C♯4, −9 an A4,
ab C♯6 keiner; bei 1 600 Pa bis rund −50 Cent (Spieler 10–35 Cent, mit Balg tiefer, ab C6 kein Bend). Verworfen: b
(Bend in 34j ungebaut), c (Eigenfrequenz unmittelbar verschieben — ein Bend ohne Pegelabfall und Klangänderung).
Kippt auf b, wenn die Kurve je Taste am gebauten Kern nicht eindeutig umkehrbar ist (am Prüfstand an sechs Tasten und
sieben Öffnungen monoton).

✅ **ZV8 a** (Patrik S191): **Der Stimmer legt den Bend je Taste auf dem Raster 2^(−1/4) mit der Grenze ab** — gestimmte
Kurven `bendStep1` … `bendStep7` (Cent gegen offen am mittleren Druck bei den Öffnungen 0,84 … 0,30; unter der Grenze
der Taste ihr Wert) und `bendLimitOpening`, `bendLimitCent` (die kleinste Öffnung, die noch anspricht, auf 1 %
eingegabelt, und die Tonhöhe dort); dazwischen linear in ln(Öffnung). Grund: gegen ein Raster 2^(−1/8) liegt der größte
Fehler bei 2,0 Cent (Raster 0,7 / 0,5 / 0,35 mit Grenze: 4,3 Cent an F3–F♯3; nur die Grenze: 14,9 Cent); die Grenze
liegt an F3 bei Öffnung 0,26 und −60 Cent, an A4 bei 0,375 und −15 Cent, ab C♯6 gibt es keinen Bend. Verworfen: b
(fünf Kurven, doppelter Fehler im Bass), c (nur die Grenze). Kippt auf b, wenn die Hörprobe in 34j-9 4 Cent im Bend
nicht hören lässt und neun Kurven im Creator unhandlich sind.

✅ **ZV9 a** (Patrik S191): **`pitch` wirkt schon in 34j-6** — am Beginn der Note über die gestimmte Kurve der gespielten
Taste (`ventilFuerCent`: nur abwärts, gekappt an der Grenze), dieselbe Öffnung für alle Zungen der Taste; 34j-7 macht
daraus einen Verlauf. Grund: kein Zwischenstand, in dem die Auskunft eine Spur nennt, die nichts tut; Umkehrung und
Grenze werden geprüft, wo die Kurve entsteht. Verworfen: b (`pitch` erst mit 34j-7). Kippt auf b, wenn 34j-7 für den
laufenden Bend eine andere Größe braucht als Cent am Beginn.

**34j-7 erledigt (S191, S192):** der laufende Wind (S191) — der Kern liest `expression`, `pitch`, `vibrato` und `bellows`
über die Zeit (eine konstante Spur rechnet Bit für Bit wie der feste Wert), `KernAuskunft::anregungDauernd`, die App gibt
einem solchen Kern Expression (Muster mal Clip), Pitch und Vibrato als Spuren und legt die Expression nicht als Pegel
hinter den Ton. Gemessen (Messfall `LUMIVIZ_34J_MESSUNG` in `test_InstrumentModelsZunge`): Dynamik Stärke 0 → 1 15–18 LU,
Ausklang bis −60 dB 55–4 570 ms, Bend über die Spur −19,9 Cent in der Mitte des Winds. Dazu (S192, Journal J72): ZV10 a
— der Stimmer misst je Taste `strengthAtLevel2` … `strengthAtLevel20` am ganzen Ton (Raster 0,05, eingegabelt auf
0,05 LU; `messeDynamik`), der Adapter setzt Stärke = `staerkeBei(Stufe)` · Expression; am Ton nachgemessen trifft eine
Note bei −x dB ihr Ziel auf 1,79 LU (Akkordeon), 0,27 (Harmonium), 1,84 (Melodica), 0,28 (Mundharmonika) — der Fehler
liegt zwischen den Stufen, die Stützpunkte treffen auf 0,05 LU. ZV11 a nach ZV13 a — `releaseTime` je Taste
(`messeNachklang`), `nachklang` liest ihn: Akkordeon 190–4 575 ms, Harmonium 845–1 110, Melodica 310–1 525,
Mundharmonika 60–340. Das Stimmen aller vier dauert 41 s (vorher 13 s). Messung
[`proben/2026-11-12_S192_messung_34j7.md`](../modellierung/zunge/proben/2026-11-12_S192_messung_34j7.md). ZV12 a danach
untersucht, ZV14 a (unten).

✅ **ZV10 a** (Patrik S191, „alle nach Empfehlung“): **Die Stufe setzt die Stärke je Taste an ihrer gemessenen Kurve** —
gestimmte Kurven `strengthAtLevel2` … `strengthAtLevel20` (die Stärke, bei der die Taste 2, 4 … 20 dB leiser klingt als
bei voller Stärke; unter ihrer Dynamik 0), dazwischen linear; Stärke = diese Stärke mal Expression (SK4 a). Grund: am Ton
nachgemessen größter Fehler gegen TM1 a 1,8 LU (Akkordeon, Melodica), 0,3 LU (Harmonium, Mundharmonika); an der
mittleren Kurve 2,9 / 0,6 LU, an der Geraden 1 + Stufe/Dynamik 4,4 / 0,9 LU. Verworfen: b (mittlere Kurve), c (Gerade).
Kippt auf b, wenn ZV12 zeigt, dass der Anstieg des Pegels unter voller Stärke ein Fehler des Modells ist und die Kurven
danach über alle Tasten gleich laufen.

✅ **ZV11 a** (Patrik S191): **Der Nachklang ist eine gestimmte Kurve je Taste** (`releaseTime`): am Kern bei `windMin`
bis −60 dB unter dem gehaltenen Ton gemessen (dort klingt die Zunge am längsten, 1,03–1,50 mal die Mitte). Grund: nichts
wird abgeschnitten (fest 1 s schneidet 30 von 65 Akkordeon- und 5 von 32 Melodicatasten ab, Akkordeon tief bis 4,6 s), und
kein Ausklang wird umsonst gerechnet — die Formel 6,9/(ζω₀) überschätzt um das 1,2- bis 2,9-Fache (die Dämpfung durch die
Strömung fehlt ihr; im Bass 8,3 s). Verworfen: b (Formel wie `bar`, `membrane`), c (fest 1 s). Kippt auf b, wenn die
Messung bei `windMin` an langsam ansprechenden Tasten (F1) unstet ist.

✅ **ZV12 a** (Patrik S191): **Der Befund „lauter wird dunkler“ wird nach ZV10 und ZV11, vor 34j-8 untersucht** — Spektren
über dem Blasdruck in den Quellen (Cottingham 2011, Braasch & Cottingham 2023, Tarnopolsky u. a. 2000) und am Kern die
Ursache; Vorlage mit Optionen. Befund: der Schwerpunkt der Teiltöne 1–20 wird von Stärke 0 auf 1 nur an einem Teil der
Tasten heller (Akkordeon 10 von 17, Verhältnis 0,49–2,05; Harmonium 4 von 11; Melodica 1 von 8; Mundharmonika 4 von 10;
Melodica A4 im Crescendo 1 394 → 895 Hz). Grund: 34j-8 liefert Instrumente, Pegel, Loops und Abdrücke aus — ist die
Klangfarbe über dem Druck falsch, entstünde alles zweimal (wie S187). Verworfen: b (in der Hörprobe 34j-9). Kippt auf b,
wenn die Quellen keine Spektren über dem Druck hergeben.

✅ **ZV13 a** (Patrik S192): **`releaseTime` ist je Taste der längste Ausklang über die Haltedauern 0,1 / 0,25 / 0,5 / 1 /
2 / 4 s und die Stärken 0 / 0,5 / 1, gemessen bis jeder 5-ms-Block 60 dB unter dem lautesten Block derselben Note
bleibt** (präzisiert ZV11 a, das die Haltedauer offen ließ). Befund beim Bau: mit 1 s Halten bei Stärke 0 und dem
gehaltenen Ton als Bezug klingen andere Haltedauern länger — Akkordeon bis 1,27-fach / +110 ms (0,25 s, Stärke 0; die
tiefen Zungen wachsen bei `windMin` über 1 s hinaus), die anderen bis +30 ms; gegen den eigenen gehaltenen Ton bis
1,79-fach, gegen den eingeschwungenen gleicher Stärke bis 1,32-fach. Grund: nur so wird bei keiner gemessenen Haltedauer und
Stärke etwas abgeschnitten, und die Lesart ist auch für eine Note eindeutig, die nur aus dem Einschwingen besteht; über
alle Tasten wird etwa gleich viel Nachklang gerechnet (Akkordeon 46,7 gegen 48,3 s, die anderen 0,99–1,03-fach); der
Stimmer misst 18 Ausklänge je Taste. Verworfen: b (ZV11 a wie gebaut, bis 110 ms abgeschnitten), c (Formel 6,9/(ζω₀),
1,3–2,1-fach Nachklang und an einer Akkordeontaste zu kurz). Kippt auf b, wenn Haltedauern über 4 s beim Akkordeon
deutlich länger ausklingen (von 2 auf 4 s 1,18 → 1,16).

**ZV12 a erledigt (S192):** der Befund „lauter wird dunkler“ war größtenteils ein Fehler der Messung — S191 las die
Teiltöne an der Sollfrequenz der Taste, die Tonhöhe folgt aber dem Druck (ZV5 a, bei ff bis −24 Cent), bei ff fielen die
hohen Teiltöne aus dem Fenster. An der gemessenen Tonhöhe wird der Ton mit dem Wind heller: Harmonium 11 von 11,
Mundharmonika 10 von 10, Melodica 6 von 8, Akkordeon (ganzer Ton) 12 von 17. Rest: Akkordeon E3–A3 und Melodica F3–A3
0,77–0,98-fach dunkler — dort drosselt das Ventil (ZV5 a), die Amplitude bleibt oder fällt (Ventil × 4: 1,15–1,23-fach
heller). Quellen: kein gemessenes Spektrum des Schalls über dem Druck in den 28 Arbeiten; die Zungenbewegung wird mit dem
Druck obertonreicher (Puranik & Scavone 2023, gedr. S. 1810–1811), die Amplitude von Harmoniumzungen bleibt fast gleich
und fällt oben um rund 15 % (Cottingham u. a. 1999). Befund
[`../modellierung/zunge/2026-11-12_S192_zv12_klangfarbe_ueber_druck.md`](../modellierung/zunge/2026-11-12_S192_zv12_klangfarbe_ueber_druck.md),
Messung `proben/2026-11-12_S192_messung_zv12.md`, Agentenbericht `recherche/2026-11-12_S192_klangfarbe_ueber_druck.md`.

✅ **ZV14 a** (Patrik S192): **Der Kern bleibt; die Messung liest die Teiltöne an der gemessenen Tonhöhe** (Messfall
`LUMIVIZ_34J_MESSUNG`, `LUMIVIZ_ZV12_MESSUNG`, der Crescendo-Fall in `test_LoopZunge` prüft „heller“ an der Melodica A4:
1,08-fach). Der Rest am unteren Diskant und die Amplitude des Harmoniums über dem Druck (im Kern 1,5- bis 2-fach,
gemessen fast gleich) stehen als Befund in den Offenen Punkten, zu prüfen in der Hörprobe 34j-9; weiter mit 34j-8. Grund:
a liegt an allen gemessenen Zielgrößen am nächsten (Tonhöhe über dem Druck, Amplitude im unteren Diskant, Richtung
„heller“ an 39 von 46 Tasten); a legt nichts fest, was ein späterer Befund nicht neu stimmen kann. Verworfen: b (Ventil im
unteren Diskant × 4 — heller, aber die Tonhöhe fiele über den Wind nur noch 3–5 statt 17 Cent, die Amplitude wüchse um
70–130 %), c (vor 34j-8 Ricot u. a. 2005, Koopman & Cottingham 1997, Llanos-Vázquez u. a. 2014 laden). Kippt auf c, wenn die
Klangfarbe des unteren Diskants vor dem Ausliefern belegt sein soll; auf b, wenn eine dieser Quellen am Schall „heller über
den ganzen Umfang“ misst.

**34j-8 erledigt (S192, Journal J73):** `accordion`, `harmonium`, `melodica`, `harmonica` unter `asset/loop/instruments/`,
geschrieben vom Programm (`test_LoopZunge`, `LUMIVIZ_ZUNGE_INSTRUMENTE_SCHREIBEN=1`), gestimmt und normiert mit
`InstrumentTuneTool --all --reference` (Stimmung und Bezug Bit für Bit wie `werte/zunge_ic.txt`; der neue Wächter
„Loop Zunge 34j-8“ bindet beides an die Datei); ohne Pedale; Art nach dem Aufbau (mit Balg und Tastatur *keys*, geblasen
*wind*); Pegel −12,4 / −10,7 / −8,7 / −13,8 LUFS bei 1,0 → 0,8904 / 0,7348 / 0,5814 / 1,0457 (alt = neu); `LP3 a` ohne
Ausnahme (über den Umfang 4,3 / 3,7 / 4,2 / 0,9 LU); `loop_werte.txt` und `klangfassung.txt` je zwölf neue Zeilen, keine
alte geändert; zehn Zeilen mehr in `CREDITS.md`. Die Fälle der App in `test_LoopZunge` spielen jetzt die Dateien. Der
Quellen-Wächter vergleicht jede Kern-Datei mit ihrer eigenen Vorlage (bis S192 nur Trommel, Stab, Hammer — die übrigen
mit der ersten Vorlage der Tafel; jetzt auch `struckString` und `freeReed`). Genres bekommen die vier mit ihren Loops in
34j-9.

**34j-9 erledigt (S192, Journal J74):** `free_reeds_demo.lvloop` (`asset/loop/make_zungen_loops.py`; Folk, 104 bpm,
Am – Dm – E – Am zweimal; das Akkordeon mit Melodie und Bass-Seite, das Harmonium mit dem Crescendo über die Expression am
Clip 0,3 → 1 über sechs Takte, danach Melodica und Mundharmonika) und je Instrument ein Loop der Nuancen über
`make_modelled_loops.py` (Spielweisen als feste Spuren: *Vibrato* an allen vier, *Pallet half closed* an Akkordeon und
Melodica; Balgwechsel und Bend als Verlauf erst mit 31g); alle bestehenden Loops Bit für Bit gleich (in einer Kopie
geprüft). Lautstärke nach LV1 a: alle fünf 1,0 (Spitze −5,4 … −8,6 dBFS); `loop_werte.txt` fünf neue Zeilen, in
`klangfassung.txt` nur die Abdrücke der vier Instrumente neu (das Genre Folk steht in der Datei, die Werte gleich). Das
Ensemble bleibt bei den Instrumenten bis 34i. Hörliste `zunge_34j` am Hörort und unter `hoertest/listen/` (53 Klänge, 51
Punkte: je Instrument tief / mitte / hoch bei −18 / −9 / 0, Crescendo und Vibrato, die Befunde ZV14 a als Crescendo an G3,
Balgwechsel über den Kern, Bass-Seite, Musette gegen einen Chor; `--liste-pruefen` 0 Fehler, 0 Hinweise); sie löst IC
`10_zunge` ab (aus dem Repo entfernt, am Hörort `abgeloest.txt`). Creator-Fall „Instrument-Creator 34j-9“ (Akkordeon: Core
und Level, Chöre und Bass-Seite als Listen, 21 gestimmte Kurven, Save as Bit für Bit; Bild `_accordion`). Fall
`*34j-9 Rechenzeit*` meldet nur Zahlen: unter Drosselung gemessen 1,25–1,27 % am Akkordeon (Musette, Bass-Seite, F1),
0,63 % an den drei übrigen, 0,43 × Flügel — die Prüfung gegen 4,6 % (RZ1-1 a) wird nach dem Tausch der Kühlung nachgeholt.

**34j-10 erledigt (S192, Journal J75) — Stufe 34j fertig:** Modul-Doku `InstrumentModels.md` 0.7.0 (die Zunge; dazu
der Hammer seit S187 und der Kopf, der seit S183 auf 0.5.0 stand), `InstrumentCore.md` 0.5.0 (`anregungDauernd`),
`InstrumentMeasure.md` 0.4.0 (`ZungeStimmen`); Bibliotheken in `Solution.json` `InstrumentCore` 0.6.0, `InstrumentModels`
0.7.0 (stand seit S183 auf 0.5.0), `InstrumentMeasure` 0.4.0; Quellpakete `instrumentcore-v0.6.0`,
`instrumentmodels-v0.7.0`, `instrumentmeasure-v0.4.0` (Summen gut, jede Datei LF, Zunge und Modul-Doku darin — die ersten
seit 34g); Bericht der Klasse `../modellierung/zunge/README.md`; `klassen/10_zunge` und `werkzeuge/stimme_zunge.cpp` unter
`harvest/` gelöscht (zuletzt im Commit `6e9f4756`, die Belege `abdruck_zunge.bat` und S188 nennen ihn). **Voller Lauf**
1888 / 1888, 0 Skips unter Release-clang (802 s) und Testing-MSVC (752 s).

**Teilplan 34k** (S193, Zuschnitt nach IK2-2 a, IK2-3 a, IK2-8 a, IK2-11 a, KB1 a, F-K1 a, UF9 a, GZ1 a–GZ4 a, GZ1-1 a;
Nummern fest). Gefunden — Befund [2026-11-12_S192_befund_ic_vorlage.md](../modellierung/gezupfte-saite/2026-11-12_S192_befund_ic_vorlage.md)
und Gegenprobe [2026-11-12_S193_gegenprobe_ursachen.md](../modellierung/gezupfte-saite/2026-11-12_S193_gegenprobe_ursachen.md),
beide am Ton der Vorlage gemessen (Bau gleich IC `abdruck_msvc.txt`):

- **Lautheit:** keines der fünf hält `LP3 a` (Cembalo 36,1, Harfe 26,3, Laute 20,0, Gitarre 19,4, Oud 18,6 LU über den
  Umfang, Sprünge bis 21,8 LU). Zwei Ursachen im Rechengesetz, nicht in den Werten je Taste: **U1** der Zupf ist ein
  Kraftstoß fester Dauer (`Zupf.cpp:147`, `stossS` 2–4 ms, erste Nullstelle bei 500–1000 Hz); **U2** der Steg trägt 5–10
  scharfe Moden, die Annahmen sind (`Vorgaben.cpp:92–93, 117, 147, 165, 178`). Mit Stoß = ¼ Periode und einem glatten
  Prüfsteg fallen die Sprünge auf 1,9–4,2 LU; übrig bleibt ein glatter Anstieg von rund 1 LU je kleine Terz (gleiche Kraft
  je Taste, Ausgang = abgeleitete Stegschnelle, `Zupf.cpp:187`).
- **Saiten:** die Vorlage rechnet jede Note als eigene leere Saite (Gitarre 650 mm und 70 N für alle Noten);
  `ZupfAnschlag` kennt Note, Stärke, Haltedauer, *laissez vibrer*, Lautenzug, Polarisation — keine Saite, keinen Bund,
  keine Anschlagrichtung. Der Tastenstoß des Cembalos ist stumm (`Zupf.cpp:194`, Faktor 1e-4).
- **Mitschwingen:** Saiten und Steg sind linear — Clip mit zwei Noten minus Summe der Einzelnoten −184,4 dB (frei), mit
  Dämpfen 53 dB unter dem Ton. Je Note: Gitarre mit sechs leeren Saiten 1,1–1,3 % der Echtzeit, Harfe mit allen 47 Saiten
  9,4–9,9 %, mit Oktaven, Quinten, Quarten (18–20 Saiten) 3,7–4,9 % bei höchstens 2,2 dB im Nachklang und 2,1 LU an der
  Note neben „alle“. Die Saiten am Steg ändern die Lautheit der Note selbst (bis −8,1 LU). Alles unter Drosselung und an
  den Stegmoden der Vorlage gemessen.
- **App:** alle Kerne melden Rechenweg „Note“, die App baut `kern::einzelnote` (`Instrument.cpp:3027`); einen Clip-Weg
  gibt es nicht. `Tonauftrag` trägt schon `pitch`, `vibrato` und `gleitet`. Die **Planung** der Spielweisen (Saitensatz,
  Akkorde, Griffbilder, Tabs, Saitenwahl, Strum) hängt am Instrument (`instrument.saiten`: `Saitenplan.cpp:131`,
  `Akkord.cpp:390`, `InstrumentEditor.cpp`), ihr **Klang** (Saite, Bund, Up/Down, Palm mute, Harmonic, Tapping, Bend,
  Slide, Legato) nur an der Saiten-Schicht (`Instrument.cpp:3046` ff.). Kein Kern-Instrument trägt heute einen Saitensatz
  — ob die Planung damit ohne Änderung läuft, ist nicht getestet.
- **Körper:** `korpusImpulsantwort` (`Instrument.cpp:3232`) gibt es für Parlor und Classical, kein Kern-Pfad ruft sie; der
  Kern-Pfad faltet nur `.lvinstbody` (`:3037`). `InstrumentBody` rechnet Moden mit Wert am Stegpunkt, Verlustfaktor und
  Abstrahlgewicht (`Auswertung.hpp:156–165`) und den statistischen Teil, keinen Luftraum; die IC-Fassung des Luftraums
  liegt unter `harvest/instruments-calculation/rechnung/gemeinsam/Koerper.hpp:66–104`.
- **Namen und Quellen:** `guitar`, `nylon_guitar` (Extended KS, 7 und 5 Loops) und `harp` (Sample) sind vergeben;
  `harpsichord`, `concert_harp`, `lute`, `oud`, `classical_guitar` frei. In der Ablage `…\cmake\quellen\gezupfte-saite\`
  liegen Bader 2025 (Dulcken), Chadefaux 2013, Mores, Wright 1996; Aufnahmen im Archiv (Iowa MIS Gitarre, Mores 2021,
  VCSL). Dünn: Mensuren (Taskin nach Di Veroli nur an den C-Tönen), Harfen-Saitentabellen nicht gefunden, Le Carrou
  (Dissertation) ungelesen, Abdallah 2026 (Oud) ungeladen. YIN springt bei schwachem Grundton auf Teiltonverhältnisse
  (Befund S192). Die Rechenzeit ist ausgesetzt (Kühlung): die Fälle werden gebaut, die Zahlen nachgeholt.

| Nr. | Inhalt | fertig, wenn |
|---|---|---|
| 34k-1 | Beleg aus der Vorlage: Programm mit Baubefehl `docs/modellierung/anbindung-ic/abdruck_zupf.bat` (Muster `abdruck_zunge.bat`; MSVC `/MD`), `werte/zupf_ic.txt` — je Instrument tief/mitte/hoch bei Stärke 0,2 / 0,5 / 1, *laissez vibrer*, Lautenzug, Doppelchor, Polarisation, Tastenstoß, je ein Clip mit Mitschwingen (Gitarre, Harfe; eine Pedalstellung ungleich C-Dur), die Bezüge, die gestimmten Laufzeitfaktoren aus `stimme` | die drei IC-Abdrücke gleich `abdruck_msvc.txt` |
| 34k-2 | Kern `pluckedString` unter `InstrumentModels/zupf/` (`lumi::kern::zupf`), Tafel „Plucked string“: `Zupf.cpp`, `Vorgaben.cpp`, `Pedale.cpp` per Skript (Anker je genau einmal); die fünf Vorlagen in `vorlagen()` ohne Datei (wie KL1 a); das Messen zieht aus dem Kern (IK2-2 a); `rechneZupfClip` zieht als Funktion des Kerns mit (Bezug für die Tests, GZ3 a); Tafel (*Keys* mit Umfang nach KS1 a, *String*, *Pluck*, *Bridge* als Liste der Stegmoden, *Courses*, *Sympathetic strings*, *Pedals*, *Tunable*, *Output*); Bezug über `normiere` (Probe gegen den IC-Bezug prüfen); Adapter Rechenweg Note wie IC, ohne Mitschwingen; `clang-cl` mit `/clang:-ffp-contract=off` | alle Abdrücke aus 34k-1 Bit für Bit unter Release-clang und Testing-MSVC; die IC-Fälle aus `test_zupf` grün; `Loop Werte`, `Klangfassung` unverändert |
| 34k-3 | Stimmer `pluckedString` in `InstrumentMeasure/ZupfStimmen` (Laufzeitfaktor je Note, IK2-11 a), Dauer gemessen; dazu eine **zweite, unabhängige Tonhöhenmessung** für die Prüfung ±10 Cent (Grundton aus den Teiltönen an der gemessenen Tonhöhe, nicht YIN allein — Befund S192), an synthetischen Tönen mit schwachem Grundton geprüft | Kurven Bit für Bit gleich der Vorlage für alle fünf; Wächter Stimmung rot bei absichtlich veralteten Werten; die zweite Messung an einem Ton ohne Grundton auf ±1 Cent |
| 34k-4 | Recherche und Quellen (GZ2 a, GZ4 a): Ladeliste vorher zur Freigabe, Ablage `…\cmake\quellen\gezupfte-saite\`, Register, `archiv_index.py`; Patrik um Ablage und Archiv bitten. Gesucht je Instrument: **Saiten** (Länge, Durchmesser, Werkstoff, Zug je Taste bzw. je Saite; Cembalo Zupfpunkt je Taste und Register; Harfe Saitentabelle eines Konzertmodells; Laute und Oud Chöre, Stimmung, Mensur; Gitarre Saitensatz mit Zug je Saite); **Zupf** (Auslenkung, Ablösegeschwindigkeit, Zupfstelle, Breite von Finger, Plektrum, Risha, Kiel — Chadefaux 2012 / 2013 und was sie zitieren; Tastenstoß des Cembalos nach Penttinen); **Körper** (Dicke, Fläche, Holz, Beleistung der Decke oder des Bodens, tiefe Moden, Luftraum und Schalllöcher; gemessene Stegadmittanz in zwei Richtungen; Le Carrou für die Harfe, Bader 2025 und Mores für Cembalo und Gitarre, Abdallah 2026 für die Oud); **Ziele** (Ausklang je Lage, Dynamik, Lautheit über den Umfang aus VCSL, Iowa MIS, Mores 2021 — mit derselben Messkette wie das Modell). Agenten ohne Browser; Formeln und Tabellen an der Seite als Bild gegengeprüft; Bericht unter `../modellierung/gezupfte-saite/recherche/` | Bericht mit Fundstelle je Größe. **Haltepunkt:** je Instrument, ob die Quellen für Saiten und Zupf reichen (sonst Kipppunkt GZ2 a), und welchen Weg der Körper nimmt — Bauplan, Plattenwerte oder „gedachtes Instrument“ (Kipppunkt GZ4 a) |
| 34k-5 | Körper und Steg (GZ4 a): **zuerst die Probe**, ob der statistische Teil von `InstrumentBody` aus Plattenwerten allein (ohne Bauplan) Moden mit Wert am Stegpunkt liefert; Luftraum aus `harvest/…/gemeinsam/Koerper` nach `InstrumentBody` (F-K1 a) hinter der bestehenden Schnittstelle, Tests gegen die geschlossene Lösung (Helmholtz, Kasten); je Instrument ein Körper mit Quellen (`.lvinstbodyplan` oder Plattenwerte mit belegten tiefen Moden) unter `asset/loop/instrument_bodies/`; Werkzeug (`InstrumentBodyTool` oder `InstrumentTuneTool`, nach dem, wo der Körper gelesen wird) schreibt die Stegmoden (Frequenz, Güte, Spitze, Richtung) mit Abdruck des Körpers in die Tafel; die Richtung der Moden als Annahme mit Vermerk, wo keine Quelle sie gibt; `InstrumentBody` nächste Fassung, Modul-Doku | Stegmoden je Instrument aus dem Werkzeug, Wächter rot bei verändertem Körper; `*Loop Koerper*` und die Körper des Flügels unverändert. **Haltepunkt:** liefert die Probe keine Moden ohne Bauplan, Vorlage an Patrik (Bauplan je Instrument, ein gemeinsames Plattenmodell, oder Kipppunkt GZ4 a) |
| 34k-6 | Herleitung am Kern aus 34k-2, ohne App (Prüfstand wie `proben/2026-11-12_S193_gegenprobe_ursachen.cpp`), Dokument `../modellierung/gezupfte-saite/<Datum>_herleitung.md`: **Zupf** als Auslenkung und Loslassen (Stärke → Auslenkung und Ablösegeschwindigkeit; Zupfstelle und Breite je Instrument); **Saiten** aus 34k-4 — bei Cembalo und Harfe je Taste als Kurven, bei Laute, Oud und Gitarre **je Saite des Saitensatzes**, die Note als gegriffene Saite (Länge um den Bund verkürzt; GZ1-1 a); **Steg** aus 34k-5; **Ausgang** (Abstrahlung statt abgeleiteter Stegschnelle, KB1 a); **Mitschwingen je Note** (GZ3 a): Gitarre, Laute, Oud die übrigen Saiten des Satzes leer, Harfe die Regel der Auswahl gegen „alle 47“ (Nachklang ≤ 3 dB, Lautheit ≤ 2 LU, Rechenzeit ≤ 4,6 %), Cembalo keines; **Tastenstoß**; **Probe Bend**: eine Saite mit laufender Tonhöhe (`pitch`, Bend um 2 Halbtöne, Slide über 5 Bünde) im Wellenleiter mit zwei Polarisationen. Je Instrument jede Option gegen **alle** Zielgrößen gerechnet: Lautheit über den Umfang ≤ 15 LU ohne Ausnahme (Messung wie `LP3 a`: jede dritte Taste, `laut` 0,7, 500 ms) und zusätzlich an **jeder** Taste, kein Sprung über 6 LU zwischen Nachbarn im Terzabstand, Tonhöhe ±10 Cent an der zweiten Messung aus 34k-3, Ausklang und Dynamik gegen Quellen und Aufnahmen, Rechenzeit | Entscheide je Instrument (GZ5 ff.). **Haltepunkte:** was kein Instrument trägt, wird nicht ausgeliefert, mit benannter Lücke; hält der Bend ±10 Cent nicht, Kipppunkt GZ1-1 a (Saite, Bund, Up/Down, Palm mute bleiben; Bend, Slide, Legato eigene Stufe) |
| 34k-7 | Kern Rechenfassung 2 nach 34k-6: Zupf, Saiten (Kurven je Taste oder Werte je Saite, mit Quelle und Belegkraft), Steg als gestimmte Daten aus 34k-5, Ausgang, Tastenstoß, Mitschwingen je Note im Rechenweg Note (Liste der freien Saiten in der Tafel bzw. aus dem Saitensatz; `rechneZupfClip` als Bezug: je Note gleich dem Clip mit einer Note); Pedalstellung der Harfe als fester Wert am Beginn (TM2 a); Stimmer neu; `zupf_ic.txt` mit Vermerk neu (Journal); Messfall `LUMIVIZ_34K_MESSUNG=<datei.md>` mit der Messkette aus 34k-6 (Lautheit aus `AudioCore`) | die Zielgrößen aus 34k-6 am gebauten Kern wie am Prüfstand; Linearität (Clip minus Summe der Noten, frei) unter −120 dB; die IC-Fälle angepasst mit Grund im Journal |
| 34k-8 | Spielweisen am Kern (GZ1-1 a): `KettenNote` bekommt Saite, Bund, Anschlagrichtung und Artikulation (`InstrumentCore` nächste Fassung; ohne Angabe wie bisher, andere Kerne Bit für Bit gleich); `KernAuskunft` nennt, dass der Kern einen Saitensatz spielt; die App gibt solchen Kernen aus `TonAngabe` Saite, Bund, Richtung, Artikulation und die Verläufe (`pitch`, `vibrato`, Kette mit `gleitet`) in den Tonauftrag, der Tonname trägt sie wie bei der Saiten-Schicht; der Kern rechnet Up und Down, Palm mute, Harmonic, Tapping, Bend, Slide, Legato, Vibrato nach 34k-6; Kern-Instrument mit Saitensatz in Planung und Fenstern: Instrument-Editor (Tab-Linien, Spur Strum, Menü), Fenster Chords (Griffbilder), Creator (Zeilen *Tuning*, *Strum*, *Play as*) — je ein Fall **über das Fenster** (Memory „Test-Eingaben wie im Programm“), Bilder | ohne Angaben Bit für Bit wie 34k-7; jede Spielweise ändert den Ton messbar in der belegten Richtung (Muster `test_LoopSpielweiseKlang`); Bend und Slide ±10 Cent am Ziel; `Loop Werte`, `Klangfassung`, Flügel-Bezug unverändert |
| 34k-9 | die `.lvinst` der Instrumente, die 34k-6 trägt — `harpsichord`, `concert_harp` (*keys* bzw. nach dem Katalog), `lute`, `oud`, `classical_guitar` mit Saitensatz (Oud bundlos; Laute und Oud mit Chören): Schreiber `LUMIVIZ_ZUPF_INSTRUMENTE_SCHREIBEN=1`, `InstrumentTuneTool --all --reference`, Katalog, Register und `CREDITS.md`, Pegel und `loop_werte.txt` nur neue Zeilen, `klangfassung.txt` nachgeführt, Zahlen je Art in den Tests; braucht die Datei ein neues Feld, neue `.lvinst`-Fassung (Feld immer geschrieben, Literale in `tests/` breit suchen) | `Loop Pegel LP3 a` ohne Ausnahme; `Loop Werte` nur neue Zeilen; Quellen- und Stimm-Wächter grün |
| 34k-10 | `plucked_strings_demo.lvloop` (alle fünf; Laute, Oud und Gitarre mit Tabs, Strum und einem Bend), Loops je Instrument über `make_modelled_loops.py`, Lautstärke nach LV1 a (Skripte zuerst in einer Kopie); Hörliste `zupf_34k` am Hörort (je Instrument tief/mitte/hoch bei −18/−9/0, Lautenzug, Tastenstoß, *laissez vibrer*, Mitschwingen an und aus, Harfe Auswahl gegen alle Saiten, Doppelchor, Akkord als Strum Up und Down, Palm mute, Harmonic, Bend, `classical_guitar` gegen `nylon_guitar`), sie löst IC `02_zupf_saite` ab (wie LG1 a); Creator-Fall mit der Laute (Listen der Chöre und Stegmoden, gestimmte Werte, Saitensatz; Bild); Fall `*34k Rechenzeit*` (Harfe mit Mitschwingen, Gitarren-Akkord mit sechs Noten) | `--liste-pruefen` 0 Fehler, 0 Hinweise; Rechenzeit ≤ 4,6 % (RZ1-1 a) nach dem Tausch der Kühlung nachgeholt |
| 34k-11 | Modul-Doku (`InstrumentModels`, `InstrumentCore`, `InstrumentMeasure`, `InstrumentBody`; Kopf, Changelog und `Solution.json` gegeneinander geprüft), Journal J76 ff., Konzept (IK2-10 a berichtigt), Bericht der Klasse `../modellierung/gezupfte-saite/README.md` mit dem Abschnitt „Luftraum und Steg gegen `korpusImpulsantwort`“, Quellpakete der nächsten Fassungen, Steckbrief; `klassen/02_zupf_saite` und die zugehörigen Werkzeuge unter `harvest/` gelöscht (der Beleg nennt den Commit) | **voller Lauf** unter beiden Compilern (neuer Kern, Core, `InstrumentBody` und Rechenweg der App geändert; UF17-1 a) |

Prüfung je Teilschritt nach MT1 a (eng unter Release-clang, beide gebaut). Modell je Sitzung: 34k-1 bis 34k-3 und 34k-7
bis 34k-11 Opus; 34k-4 bis 34k-6 Fable (Herleitung aus mehreren Quellen, Schwellen in Frage). Scheitert in 34k-8 der Bend
am gebauten Kern ohne erkennbare Ursache, ist das ein Fable-Kandidat.

Abgeleitet: Kern nach dem Verfahren benannt (IK2-8 a); die Hörliste von LumiViz löst die von IC ab (LG1 a); Stärke über
die Anschlagstufe (TM1 a); gestimmte Werte und Stegmoden sind Daten mit Abdruck-Wächter (IK2-11 a).

**Nicht in 34k** (jede Zeile braucht den Rechenweg Clip aus 31e, Bedienung aus 31g oder eine eigene Stufe):
Pedalwechsel der Harfe innerhalb einer Passage und das Zusammenspiel der Dämpfer (31e); im Akkord gegriffene Saiten als
mitschwingende Saiten — je Note schwingen die übrigen Saiten **leer** mit, das ist eine benannte Annahme (31e);
Register des Cembalos als Spur (31g; die Register stehen fest in der Tafel); `guitar` (Stahlsaiten), E-Gitarren und
Bässe auf dem Kern — eigene spätere Stufe nach der Hörprobe `classical_guitar` gegen `nylon_guitar`; `harp` (Sample) und
`koto` bleiben; was IC in `../modellierung/gezupfte-saite/ic/offen.md` als nicht gebaut führt.

✅ **GZ1 a** (Patrik S193, „alle a“; in einem Punkt ersetzt durch GZ1-1 a): **34k liefert `harpsichord`, `concert_harp`,
`lute` und `oud` auf dem Kern `pluckedString`; `harp` (Sample), `guitar` und `nylon_guitar` bleiben** (IK2-8 a). Der
Satz „die IC-Gitarre wird nicht ausgeliefert, Polarisationen und Steg kommen später an die Waveguide-Saite der
bestehenden Gitarren“ gilt nicht mehr (GZ1-1 a). Verworfen: c (die IC-Gitarre ersetzt `nylon_guitar` — fünf Loops
verlören ihre Spielweisen).

✅ **GZ1-1 a** (Patrik S193): **34k baut die Spielweisen in den Kern.** Laute, Oud und Gitarre tragen einen Saitensatz
(Oud bundlos, Laute und Oud mit Chören je Saite); Akkorde, Griffbilder, Tabs, Saitenwahl und Strum gelten für sie wie für
jede Gitarre. Der Tonauftrag bekommt an der Note Saite, Bund, Anschlagrichtung und Artikulation; der Kern rechnet die
gegriffene Saite des Satzes, Up und Down, Palm mute, Harmonic, Tapping sowie Bend, Slide, Legato und Vibrato über `pitch`,
`vibrato` und `gleitet`. Das Mitschwingen nimmt die übrigen Saiten des Satzes. Die Gitarre wird als `classical_guitar`
neben `nylon_guitar` ausgeliefert; welches den Platz behält, entscheidet Patrik nach der Hörprobe. Harfe und Cembalo ohne
Saitensatz. Grund: eine gegriffene Saite ist das Modell des Instruments, und die Vorgaben aus GZ2 a sind bei
Griffbrett-Instrumenten Werte je Saite, nicht je Taste — wer erst je Taste herleitet, leitet zweimal her; ohne die
Erweiterung hätten Laute und Oud keine der Spielweisen, die LumiViz an Saiteninstrumenten hat. Verworfen: b (nur der
Saitensatz, der Klang je Saite später — der Tab zeigt eine Saite, zu hören ist eine andere), c (wie GZ1 a zuerst
entschieden — gibt Laute und Oud nichts, die zwei Polarisationen müssten im App-Zweig nachgebaut werden). Kippt auf b,
wenn die Probe in 34k-6 für Bend und Slide die ±10 Cent nicht hält (dann bleiben Saite, Bund, Up/Down und Palm mute, der
Rest wird eigene Stufe); auf c, wenn Patrik 34k bei vier Instrumenten ohne Griffbrett-Spielweisen halten will.

✅ **GZ2 a** (Patrik S193): **34k zieht den Kern zuerst Bit für Bit um und leitet danach Rechenfassung 2 mit Quellen
her** — der Zupf als Auslenkung und Loslassen, Saiten aus Mensuren und Saitentabellen, Steg nach GZ4 a, Tastenstoß nach
Penttinen. Zielgrößen je Instrument: Lautheit über den Umfang ≤ 15 LU ohne Ausnahme, kein Sprung über 6 LU zwischen
Nachbarn im Terzabstand (Schwelle gesetzt in S193, ohne Quelle), Tonhöhe ±10 Cent an einer unabhängigen Messung, Dynamik
und Ausklang gegen Quellen und Aufnahmen; ausgeliefert wird ein Instrument erst, wenn es sie hält; Haltepunkt nach der
Recherche. Grund: die Gegenprobe S193 — die Sprünge fallen von bis zu 21,8 LU auf 1,9–4,2 LU, sobald Zupf und Steg
stimmen, und nur dann; Werte je Taste berühren keine der zwei Ursachen. Verworfen: b (wortgleich mit einer Pegelkurve je
Taste — die Stegspitzen sind schmaler als der Tastenabstand des Wächters, der Diskant bliebe ohne Grundton, der
Tastenstoß stumm), c (ausliefern mit vier bis fünf Ausnahmen in `LP3 a` — Hörliste, Pegel, Loops und Abdrücke zweimal, wie
bei ZV1 a verworfen). Kippt auf b je Instrument, wenn die Recherche keine Mensur hergibt und das neue Gesetz die Grenze
trotzdem verfehlt; auf c, wenn Patrik vor der Herleitung hören will.

✅ **GZ3 a** (Patrik S193): **34k rechnet das Mitschwingen je Note** — die gespielte Saite und die freien Saiten des
Instruments an einem Steg, Rechenweg „Note“. Harfe: eine Auswahl der Saiten, deren Regel die Herleitung gegen die
Rechnung mit allen Saiten festlegt (Nachklang höchstens 3 dB, Lautheit höchstens 2 LU daneben, Rechenzeit ≤ 4,6 %;
Grenzen gesetzt in S193); Gitarre, Laute, Oud: die übrigen Saiten des Satzes; Cembalo: keines (Dämpfer).
`rechneZupfClip` zieht als Bezug für die Tests mit um. Pedalwechsel der Harfe in der Passage und das Zusammenspiel der
Dämpfer bleiben bei 31e. Grund: die belegte Linearität (−184 dB) — je Note gerechnet ist für frei schwingende Saiten
dasselbe wie der Clip und der Weg, den UF9 a den exakten nennt; die Saiten am Steg verschieben die Lautheit der Note
(bis −8,1 LU), Vorgaben ohne sie müssten mit 31e neu gesetzt werden. Ergänzt IK2-7 a („31e mit dem Clip-Weg, der auch
Klasse 2 trägt“): der Clip-Weg bleibt für das, was je Note nicht geht. Verworfen: b (Adapter ohne Mitschwingen bis 31e,
wie KL1 a), c (Rechenweg Clip in 34k — App-weite Fragen zu Tonspeicher, Vorhören und Flügel-Pedal, für die 31e einen
eigenen Messschritt vorsieht; der gemessene Gewinn ist klein). Kippt auf c, wenn die Auswahl am neuen Steg die Grenzen
nicht innerhalb von 4,6 % hält; auf b, wenn Patrik die Harfe zuerst ohne Mitschwingen hören will.

✅ **GZ4 a** (Patrik S193; **berichtigt IK2-10 a für Klasse 2**): **Steg und Abstrahlung eines Instruments kommen aus
einem Körper in der Form von KB1 a** — Moden mit Frequenz, Dämpfung, Wert am Stegpunkt und Abstrahlgewicht, darüber der
statistische Teil. Woher die Moden kommen, entscheidet ein Haltepunkt je Instrument nach der Recherche: aus einem Bauplan
mit `InstrumentBody`, wo Maße belegt sind; sonst aus Plattenwerten über den statistischen Teil und den belegten tiefen
Moden (Cembalo: Helmholtz 37 Hz nach Bader 2025). Der Luftraum zieht aus `harvest/…/gemeinsam/Koerper` nach
`InstrumentBody` (F-K1 a). Ein Werkzeug schreibt die Stegmoden mit Abdruck des Körpers als Daten in die Tafel des Kerns
(IK2-11 a); der Kern liest `InstrumentBody` nicht. Die Richtung der Moden am Steg (2×2) bleibt Annahme mit Vermerk, bis
eine Quelle sie gibt. Grund: Rückwirkung und Abstrahlung sollen aus einer Quelle kommen (KB1 a), und der Steg ist die
gemessene Ursache der Sprünge; er sitzt in der Saitenschleife, eine Faltung nach dem Kern kann ihn nicht ersetzen.
Verworfen: b (die Vorlage um Moden ergänzen, bis der Steg glatt ist — jede Mode ohne Quelle), c (IK2-10 a wörtlich:
`korpusImpulsantwort` als Faltung — lässt die Sprünge unverändert und legt einen Gitarrenkorpus hinter ein Cembalo).
Nicht nachgerechnet: ob der statistische Teil ohne Bauplan Moden liefert — erste Probe in 34k-5. Kippt auf b, als
„gedachtes Instrument“ gekennzeichnet, für ein Instrument, zu dem weder Bauplan noch Plattenwerte zu finden sind.

Danach **Rest von Stufe 31 auf der Bibliothek** (IK2-7 a): 31f, 31g (Bedienung nach 34a), 31e mit dem Clip-Weg, 31h.
Ketten (3, 7, 8, 9) und Clips (11, Mitschwingen) als eigene spätere Stufen (Z1 a).

*(Überholt S182, LP2 b: IC ist aufgelöst, es gibt kein Paket für IC.)* ✅ **TP1 a** (Patrik S173): **Paket als Fassung im IC-Repo.** LumiViz baut je Fassung der Bibliotheken
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
| 0.25.0 | 2026-11-12 | Session 193. §6: Teilplan 34k (34k-1 bis 34k-11) nach dem Befund S192 und der Gegenprobe S193 an der IC-Vorlage der Klasse 2; GZ1 a (Namen, Bestand bleibt), GZ1-1 a (Spielweisen am Kern, Saitensatz an Laute, Oud und `classical_guitar`), GZ2 a (Umzug als Beleg, dann Rechenfassung 2 mit Quellen), GZ3 a (Mitschwingen je Note), GZ4 a (Steg und Abstrahlung aus einem Körper nach KB1 a; berichtigt IK2-10 a für Klasse 2); Zeile 34k der Übersicht. |
| 0.24.0 | 2026-11-12 | Session 192. §6: 34j-10 erledigt — Stufe 34j fertig (Modul-Doku, Bibliotheken und Quellpakete, Bericht der Klasse, harvest gelöscht, voller Lauf 1888 / 1888 unter beiden Compilern); Journal J75. |
| 0.23.0 | 2026-11-12 | Session 192. §6: 34j-9 erledigt — `free_reeds_demo`, Loops der Nuancen, Hörliste `zunge_34j` (löst IC 10 ab), Creator-Fall, Rechenzeit (Zahlen); Journal J74. |
| 0.22.0 | 2026-11-12 | Session 192. §6: 34j-8 erledigt — die vier Zungeninstrumente als `.lvinst`, gestimmt, Pegel, Katalog, Credits; Journal J73. |
| 0.21.0 | 2026-11-12 | Session 192. §6: ZV12 a erledigt — der Befund „lauter wird dunkler“ war größtenteils ein Messfehler (Teiltöne an der Sollfrequenz), Rest am unteren Diskant vom Ventil; ZV14 a (Kern bleibt, Messung an der gemessenen Tonhöhe, Rest als Befund bis 34j-9). |
| 0.20.0 | 2026-11-12 | Session 192. §6: 34j-7 erledigt — ZV10 a gebaut (Stufe → Stärke je Taste, gestimmt), ZV11 a gebaut nach ZV13 a (neu: `releaseTime` als längster Ausklang über sechs Haltedauern und drei Stärken, −60 dB unter der lautesten Stelle der Note); Journal J72. |
| 0.19.0 | 2026-11-11 | Session 191. §6: 34j-6 erledigt (Kern `freeReed` Rechenfassung 2, Journal J70); ZV8 a (Bend je Taste auf dem Raster 2^(−1/4) mit Grenze), ZV9 a (`pitch` wirkt schon in 34j-6, am Beginn der Note). 34j-7 begonnen (laufender Wind, Journal J71); ZV10 a (Stufe → Stärke je Taste an der gemessenen Kurve), ZV11 a (Nachklang als gestimmte Kurve je Taste), ZV12 a (Befund Helligkeit über dem Druck vor 34j-8 untersuchen). |
| 0.18.0 | 2026-11-11 | Session 190. §6: 34j-4 erledigt (Quellenlage, 28 Arbeiten in der Ablage); ZQ1 a, ZQ2 a (Ablage), ZH1 a (Herleitung aus Ankern und Schwellentheorie, Belegkraft je Wert), ZH2 a (Melodica als Annahme, „ungemessen“), ZH3 a (Amplitude gegen die Messung von 1999); Zielgrößen in Zeile 34j-5 nachgeführt. 34j-5 gerechnet: ZV4 a (Kern-Form bleibt, Rechenfassung 2 ändert die Vorgaben), ZV5 a (Tonhöhe folgt dem Druck; Stimmung ±10 Cent am mittleren Druck), ZV6 a (Bass-Seite 8′ und 4′), ZV7 a (Bend über das Ventil, Kurve und Grenze je Taste aus dem Stimmer). |
| 0.17.0 | 2026-11-11 | Session 188. §6: Teilplan 34j (34j-1 bis 34j-10) nach dem Befund an der IC-Vorlage der Zunge; ZV1 a (Umzug als Beleg, dann Vorgaben je Taste mit Quellen), ZV2 a (ein Winddruck für alle Tasten), ZV3 a (laufender Wind in 34j); Zeile 34j der Übersicht. |
| 0.16.0 | 2026-11-10 | Session 186. §6: LR1 a — Stufe 34m Klasse 7 (Kern `Blech`; Posaune, Trompete, Alphorn) nach 34j und 34k, vor 34l; Kopf auf 0.16.0 (stand noch auf 0.14.0). |
| 0.15.0 | 2026-11-09 | Session 183. LP2 b erledigt: Vermerk im Kopf (WU1 a, WU2 a, KA1 a, CL1 a, CL2 a; Kern `bar` mit `pluckForce`, Abnahmen `slot` und `bridge`; Kern `struckString` mit `stringDiameter`; Rechenfassung 2; Journal J60–J64). |
| 0.14.0 | 2026-11-09 | Session 182. IC aufgelöst (LP2 b): Vermerk im Kopf, Bezug auf die Ablage der IC-Doku, 34j/34k mit Vorlage unter `harvest/`, 34l ohne Auftrag an IC, TP1 a als überholt markiert; Plan des Umzugs `../modellierung/anbindung-ic/Aufloesung_IC_Plan.md` (U0–U7). |
| 0.13.0 | 2026-11-09 | Session 181. §6: Teilplan 34i (34i-1 bis 34i-7), KL1 a (Kern `struckString` ganz, nur das Clavinet ausgeliefert), KL2 a (Namen, `clavinet_modal` bis zu den Hörproben), KL3 a (Körper `pleyel_p131` für Upright und Honky-tonk), KL4 a (Umfang in der Tafel des Flügel-Kerns); Rechenzeit ausgesetzt bis zum Tausch der Kühlung. Dazu im Bau: KL4-1 a (Kurven nur auf den Umfang der Tafel gerichtet), TT1 a (Tafel „Hammer string“), KL3-1 a (Upright ohne Boden, Recherche; 34i-5 entfällt bis zum Fund); 34i-1 bis 34i-4 und 34i-6 gebaut (Journal J53–J59, Bericht `modellierung/hammer-saite/klasse1_ohne_fluegel.md`). |
| 0.12.0 | 2026-11-08 | Session 180. §6: QM1 a gebaut, 34h erledigt. |
| 0.11.0 | 2026-11-08 | Session 179. §6: Teilplan 34h (34h-1 bis 34h-7), TM1 a (Geschwindigkeit nach der Stufe in dB), TM2 a (Spielweisen als Spuren, feste Werte je Kit-Rolle und Drum-Spur), TM3 a (Kit nur Klasse 5), TM4 a (Namen, `synth_conga`). |
| 0.10.0 | 2026-11-08 | Session 178. §6: Teilplan 34g (34g-1 bis 34g-7), KS1 a (Umfang als Wert des Instruments), KS2 a (`partnerAbstrahl` gestimmt, gestimmte Werte in Reihenfolge), KS3 a (Bogen am Vibraphon ganz), KS4 a (Namen, `synth_kalimba`, Steeldrum nicht ausgeliefert); 34g erledigt — Journal J31–J41, elf Abdrücke Bit für Bit wie IC, Rechenzeit höchstens 2,17 % (0,70 × Flügel). Abweichung: die Dateien schreibt ein Test mit dem Schreiber des Programms (Muster `grand_piano`), nicht `InstrumentTuneTool --create`. |
| 0.9.0 | 2026-11-08 | Session 177. §6: Teilplan 34f, WZ1 a (Quellen am Wert, `.lvinst` Fassung 22), WZ2 a (Register `instrument_quellen.json`); 34f erledigt — Journal J27–J30. |
| 0.8.0 | 2026-11-08 | Session 177. §6: Teilplan 34e (34e-1 bis 34e-5), ME1 a (`AudioCore`), ME2 a (`zerlegeSymmetrisch` in den Core), ME3 a (`setzen` mit Beobachter und Startzustand, die Flügel-Suche darüber), ME4 a (Endregel aus IC in beiden LM); 34e erledigt — Journal J19–J26, `salamander_*.txt` unverändert, voller Lauf 1789 / 1789 unter beiden Compilern. |
| 0.7.0 | 2026-11-07 | Session 176. 34d erledigt — der Flügel als Kern `hammerString` in `InstrumentModels`, `.lvinst` Fassung 21, Creator nach FU3 a; Journal J12–J18; Flügel-Bezug `hammer_bezug_windows.txt` Bit für Bit unter beiden Compilern, `Loop Werte` unverändert, Rechenzeit wie 34c, voller Lauf 1770 / 1770. §6: Zeilen 34c/34d nach BK1 a; §5 IK2-2 a Verweis auf den Bezug je Plattform. |
| 0.6.0 | 2026-11-07 | Session 175. §6: KN1 a (Name `InstrumentCore`, `lumi::kern`; Messung `InstrumentMeasure`), KN2 a (34c verschiebt); 34c erledigt — Übernahme-Journal `modellierung/anbindung-ic/Uebernahme_Journal.md` J0–J11, voller Lauf 1765 / 1765 unter beiden Compilern, Wächter unverändert. |
| 0.5.0 | 2026-11-07 | Session 174. §6a neu: Skizze Creator und Editor (34a) gebilligt — CK1 a (ein Creator für alle Kerne aus der Tafel), CK2 a (Kurven im Creator, Kipppunkt UF12 a greift nicht); was die Tafel dafür trägt. §6: TP1 als entschieden vermerkt. |
| 0.4.0 | 2026-11-07 | Session 174. §5a: Auftrag v0.14.0, Befund zu Punkt 3 (Pakete ohne CMake-Beschreibung), Quellpaket mit `SHA256SUMS` (neu, aus TP1 a abgeleitet), Merker PRIVATE für 34c; 34b erledigt (v0.14.0 veröffentlicht, Pin gewechselt, voller Lauf), Merker für 34c aus der Gegenprüfung. |
| 0.3.0 | 2026-11-03 | Session 173. §5 IK2-0 a (drei Schichten) mit Nachprüfung CMakeCraft STATIC; IK2-1 c (Bibliotheken unter `projects/libs/`, IC bezieht sie als Paket); IK2-2 a (reichere Fassung je Baustein, Kern und Messung, Übernahme-Journal, nichts geht verloren); IK2-3 a (`Tonkern`, Frames); IK2-4 a (Fassung 21, Parametertafel); IK2-5 a (E1 a an den Bibliotheken, Bezüge je Plattform); IK2-6 a (4,6 %, Verhältnis zum Flügel als Kennzahl); IK2-7 a (Fundament zuerst, Stufe 32 geht in 34 auf, C/D); IK2-8 a (echte Namen, `synth_` nur bei Kollision); IK2-9 durch Z2 a; CC1 a und §5a (Aufträge an CMakeCraft); IK2-10 a (Bezug als Zahl der Tafel, LUFS, Körper, Stereo); IK2-11 a (gestimmte Werte als Daten mit Abdruck-Wächter); IK2-2-1 a (Korrektur: pocketfft aus IC, neuer und belegt); IK2-12 a (Quellenregister mit Wächter, Recherche bleibt in IC); IK2-13 a (eine Skizze für alle Kerne zuerst); §6 Teilplan Stufe 34 (34a–34l) und TP1 a (Paket als Fassung im IC-Repo). Wortlaut der Vorlage IK2-0 bleibt darunter. |
| 0.2.0 | 2026-11-02 | Session 172. §4 um T2 a, Architektur, Plugins, Messung (M1 a) und Loop-Zahlen ergänzt; §5 M1 a und die Vorlage IK2-0 im Wortlaut (Antwort in S173). |
| 0.1.0 | 2026-11-02 | Session 172. Angelegt: Ziel, Voraussetzungen (Patrik S171), Zielbild Z1 bis Z4 (Patrik S172, je a), Ausgangslage; Entscheide und Teilplan offen. |

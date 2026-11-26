# LumiViz — Instrumente aus InstrumentsCalculation (Konzept)

> **Version:** 0.91.0
> **Datum:** 2026-11-26 (Session 223)
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
| 34k-2 | Kern `pluckedString` unter `InstrumentModels/zupf/` (`lumi::kern::zupf`), Tafel „Plucked string“: `Zupf.cpp`, `Vorgaben.cpp`, `Pedale.cpp` per Skript (Anker je genau einmal); die fünf Vorlagen in `vorlagen()` ohne Datei (wie KL1 a); das Messen zieht aus dem Kern (IK2-2 a); `rechneZupfClip` zieht als Funktion des Kerns mit (Bezug für die Tests, GZ3 a); Tafel (*Keys* mit Umfang nach KS1 a, *String*, *Pluck*, *Bridge* als Liste der Stegmoden, *Courses*, *Sympathetic strings*, *Tunable*, *Output*); Bezug über `normiere` (Probe gegen den IC-Bezug prüfen); Adapter Rechenweg Note wie IC, ohne Mitschwingen; `clang-cl` mit `/clang:-ffp-contract=off` | alle Abdrücke aus 34k-1 Bit für Bit unter Release-clang und Testing-MSVC; die IC-Fälle aus `test_zupf` grün; `Loop Werte`, `Klangfassung` unverändert |
| 34k-3 | Stimmer `pluckedString` in `InstrumentMeasure/ZupfStimmen` (Laufzeitfaktor je Note, IK2-11 a), Dauer gemessen; dazu eine **zweite, unabhängige Tonhöhenmessung** für die Prüfung ±10 Cent (Grundton aus den Teiltönen an der gemessenen Tonhöhe, nicht YIN allein — Befund S192), an synthetischen Tönen mit schwachem Grundton geprüft | Kurven Bit für Bit gleich der Vorlage für alle fünf; Wächter Stimmung rot bei absichtlich veralteten Werten; die zweite Messung an einem Ton ohne Grundton auf ±1 Cent |
| 34k-4 | Recherche und Quellen (GZ2 a, GZ4 a): Ladeliste vorher zur Freigabe, Ablage `…\cmake\quellen\gezupfte-saite\`, Register, `archiv_index.py`; Patrik um Ablage und Archiv bitten. Gesucht je Instrument: **Saiten** (Länge, Durchmesser, Werkstoff, Zug je Taste bzw. je Saite; Cembalo Zupfpunkt je Taste und Register; Harfe Saitentabelle eines Konzertmodells; Laute und Oud Chöre, Stimmung, Mensur; Gitarre Saitensatz mit Zug je Saite); **Zupf** (Auslenkung, Ablösegeschwindigkeit, Zupfstelle, Breite von Finger, Plektrum, Risha, Kiel — Chadefaux 2012 / 2013 und was sie zitieren; Tastenstoß des Cembalos nach Penttinen); **Körper** (Dicke, Fläche, Holz, Beleistung der Decke oder des Bodens, tiefe Moden, Luftraum und Schalllöcher; gemessene Stegadmittanz in zwei Richtungen; Le Carrou für die Harfe, Bader 2025 und Mores für Cembalo und Gitarre, Abdallah 2026 für die Oud); **Ziele** (Ausklang je Lage, Dynamik, Lautheit über den Umfang aus VCSL, Iowa MIS, Mores 2021 — mit derselben Messkette wie das Modell). Agenten ohne Browser; Formeln und Tabellen an der Seite als Bild gegengeprüft; Bericht unter `../modellierung/gezupfte-saite/recherche/` | Bericht mit Fundstelle je Größe. **Haltepunkt:** je Instrument, ob die Quellen für Saiten und Zupf reichen (sonst Kipppunkt GZ2 a), und welchen Weg der Körper nimmt — Bauplan, Plattenwerte oder „gedachtes Instrument“ (Kipppunkt GZ4 a) |
| 34k-5 | Körper und Steg (GZ4 a): **zuerst die Probe**, ob der statistische Teil von `InstrumentBody` aus Plattenwerten allein (ohne Bauplan) Moden mit Wert am Stegpunkt liefert; Luftraum aus `harvest/…/gemeinsam/Koerper` nach `InstrumentBody` (F-K1 a) hinter der bestehenden Schnittstelle, Tests gegen die geschlossene Lösung (Helmholtz, Kasten); je Instrument ein Körper mit Quellen (`.lvinstbodyplan` oder Plattenwerte mit belegten tiefen Moden) unter `asset/loop/instrument_bodies/`; ~~Werkzeug schreibt die Stegmoden mit Abdruck des Körpers in die Tafel~~ (KQ27 a: die Form leitet 34k-6 her, das Werkzeug baut 34k-7); die Richtung der Moden als Annahme mit Vermerk, wo keine Quelle sie gibt; `InstrumentBody` nächste Fassung, Modul-Doku | je Instrument ein Körper mit Wächter (KQ27 a: ohne das Werkzeug); `*Loop Koerper*` und die Körper des Flügels unverändert. **Haltepunkt:** liefert die Probe keine Moden ohne Bauplan, Vorlage an Patrik (Bauplan je Instrument, ein gemeinsames Plattenmodell, oder Kipppunkt GZ4 a) |
| 34k-6 | Herleitung am Kern aus 34k-2, ohne App (Prüfstand wie `proben/2026-11-12_S193_gegenprobe_ursachen.cpp`), Dokument `../modellierung/gezupfte-saite/<Datum>_herleitung.md`: **Zupf** als Auslenkung und Loslassen (Stärke → Auslenkung und Ablösegeschwindigkeit; Zupfstelle und Breite je Instrument); **Saiten** aus 34k-4 — bei Cembalo und Harfe je Taste als Kurven, bei Laute, Oud und Gitarre **je Saite des Saitensatzes**, die Note als gegriffene Saite (Länge um den Bund verkürzt; GZ1-1 a); **Steg** aus 34k-5; **Ausgang** (Abstrahlung statt abgeleiteter Stegschnelle, KB1 a); **Mitschwingen je Note** (GZ3 a): Gitarre, Laute, Oud die übrigen Saiten des Satzes leer, Harfe die Regel der Auswahl gegen „alle 47“ (Nachklang ≤ 3 dB, Lautheit ≤ 2 LU, Rechenzeit ≤ 4,6 %), Cembalo keines; **Tastenstoß**; **Probe Bend**: eine Saite mit laufender Tonhöhe (`pitch`, Bend um 2 Halbtöne, Slide über 5 Bünde) im Wellenleiter mit zwei Polarisationen. Je Instrument jede Option gegen **alle** Zielgrößen gerechnet: Lautheit über den Umfang ≤ 15 LU ohne Ausnahme (Messung wie `LP3 a`: jede dritte Taste, `laut` 0,7, 500 ms) und zusätzlich an **jeder** Taste, kein Sprung über 6 LU zwischen Nachbarn im Terzabstand, Tonhöhe ±10 Cent an der zweiten Messung aus 34k-3, Ausklang und Dynamik gegen Quellen und Aufnahmen, Rechenzeit | Entscheide je Instrument (GZ5 ff.). **Haltepunkte:** was kein Instrument trägt, wird nicht ausgeliefert, mit benannter Lücke; hält der Bend ±10 Cent nicht, Kipppunkt GZ1-1 a (Saite, Bund, Up/Down, Palm mute bleiben; Bend, Slide, Legato eigene Stufe) |
| 34k-7 | Kern Rechenfassung 2 nach 34k-6: Zupf, Saiten (Kurven je Taste oder Werte je Saite, mit Quelle und Belegkraft), Steg aus dem Körper nach GZ5 a und GZ7 a: die Moden der Körperdatei in der Saitenschleife, vom Tonauftrag gereicht (`InstrumentCore` nächste Fassung, andere Kerne Bit für Bit gleich), der Kern gibt die Stegkraft senkrecht zur Decke aus, die App faltet mit der Antwort des Körpers wie beim Flügel, der Tonname trägt den Abdruck des Körpers; **kein Stegmoden-Werkzeug** (GZ7 a ersetzt den Teil aus KQ27 a), Ausgang, Tastenstoß, Mitschwingen je Note im Rechenweg Note (Liste der freien Saiten in der Tafel bzw. aus dem Saitensatz; `rechneZupfClip` als Bezug: je Note gleich dem Clip mit einer Note); Pedalstellung der Harfe als fester Wert am Beginn (TM2 a), dazu die Gruppe *Pedals* der Tafel (sieben Werte C–H, −1/0/+1; PD1 a); Stimmer neu; `zupf_ic.txt` mit Vermerk neu (Journal); Messfall `LUMIVIZ_34K_MESSUNG=<datei.md>` mit der Messkette aus 34k-6 (Lautheit aus `AudioCore`) | die Zielgrößen aus 34k-6 am gebauten Kern wie am Prüfstand; Linearität (Clip minus Summe der Noten, frei) unter −120 dB; die IC-Fälle angepasst mit Grund im Journal |
| 34k-8 | Spielweisen am Kern (GZ1-1 a): `KettenNote` bekommt Anschlagrichtung und Artikulation (Saite und Bund seit 34k-7, ZK2 a; `InstrumentCore` nächste Fassung; ohne Angabe wie bisher, andere Kerne Bit für Bit gleich); `KernAuskunft` nennt, dass der Kern einen Saitensatz spielt; die App gibt solchen Kernen aus `TonAngabe` Saite, Bund, Richtung, Artikulation und die Verläufe (`pitch`, `vibrato`, Kette mit `gleitet`) in den Tonauftrag, der Tonname trägt sie wie bei der Saiten-Schicht; der Kern rechnet Up und Down, Palm mute, Harmonic, Tapping, Bend, Slide, Legato, Vibrato nach 34k-6; Kern-Instrument mit Saitensatz in Planung und Fenstern: Instrument-Editor (Tab-Linien, Spur Strum, Menü), Fenster Chords (Griffbilder), Creator (Zeilen *Tuning*, *Strum*, *Play as*) — je ein Fall **über das Fenster** (Memory „Test-Eingaben wie im Programm“), Bilder | ohne Angaben Bit für Bit wie 34k-7; jede Spielweise ändert den Ton messbar in der belegten Richtung (Muster `test_LoopSpielweiseKlang`); Bend und Slide ±10 Cent am Ziel; `Loop Werte`, `Klangfassung`, Flügel-Bezug unverändert |
| 34k-9 | die `.lvinst` der Instrumente, die 34k-6 trägt — `harpsichord`, `concert_harp` (*keys* bzw. nach dem Katalog), `lute`, `oud`, `classical_guitar` mit Saitensatz (Oud bundlos; Laute und Oud mit Chören): Schreiber `LUMIVIZ_ZUPF_INSTRUMENTE_SCHREIBEN=1`, `InstrumentTuneTool --all --reference`, Katalog, Register und `CREDITS.md`, Pegel und `loop_werte.txt` nur neue Zeilen, `klangfassung.txt` nachgeführt, Zahlen je Art in den Tests; braucht die Datei ein neues Feld, neue `.lvinst`-Fassung (Feld immer geschrieben, Literale in `tests/` breit suchen) | `Loop Pegel LP3 a` ohne Ausnahme; `Loop Werte` nur neue Zeilen; Quellen- und Stimm-Wächter grün |
| 34k-10 | `plucked_strings_demo.lvloop` (alle fünf; Laute, Oud und Gitarre mit Tabs, Strum und einem Bend), Loops je Instrument über `make_modelled_loops.py`, Lautstärke nach LV1 a (Skripte zuerst in einer Kopie); Hörliste `zupf_34k` am Hörort (je Instrument tief/mitte/hoch bei −18/−9/0, Lautenzug, Tastenstoß, *laissez vibrer*, Mitschwingen an und aus, Harfe Auswahl gegen alle Saiten, Doppelchor, Akkord als Strum Up und Down, Palm mute, Harmonic, Bend, `classical_guitar` gegen `nylon_guitar`), sie löst IC `02_zupf_saite` ab (wie LG1 a); Creator-Fall mit der Laute (Listen der Chöre und Stegmoden, gestimmte Werte, Saitensatz; Bild); Fall `*34k Rechenzeit*` (Harfe mit Mitschwingen, Gitarren-Akkord mit sechs Noten) | `--liste-pruefen` 0 Fehler, 0 Hinweise; Rechenzeit ≤ 4,6 % (RZ1-1 a) nach dem Tausch der Kühlung nachgeholt |
| 34k-11 | Modul-Doku (`InstrumentModels`, `InstrumentCore`, `InstrumentMeasure`, `InstrumentBody`; Kopf, Changelog und `Solution.json` gegeneinander geprüft), Journal J76 ff., Konzept (IK2-10 a berichtigt), Bericht der Klasse `../modellierung/gezupfte-saite/README.md` mit dem Abschnitt „Luftraum und Steg gegen `korpusImpulsantwort`“, Quellpakete der nächsten Fassungen, Steckbrief; `klassen/02_zupf_saite` und die zugehörigen Werkzeuge unter `harvest/` gelöscht (der Beleg nennt den Commit) | **voller Lauf** unter beiden Compilern (neuer Kern, Core, `InstrumentBody` und Rechenweg der App geändert; UF17-1 a) |

Prüfung je Teilschritt nach MT1 a (eng unter Release-clang, beide gebaut). Modell je Sitzung: 34k-1 bis 34k-3 und 34k-7
bis 34k-11 Opus; 34k-4 bis 34k-6 Fable (Herleitung aus mehreren Quellen, Schwellen in Frage). Scheitert in 34k-8 der Bend
am gebauten Kern ohne erkennbare Ursache, ist das ein Fable-Kandidat.

**Teilplan 34k-7** (S220, ZK1 a, ZK2 a; Nummern fest, Bauvorlage `../modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md`):

| Schritt | Inhalt | Prüfung |
|---|---|---|
| 34k-7a Bezug | Prüfstand (`proben/2026-11-15_S199_pruefstand_zupf.py`) an den heutigen fünf Körpern (Harfe Fassung 8 mit Federn und Rohr, Oud nach KQ42 a–KQ45 a, Gitarre mit Richtung nach KQ35); Zielgrößen nach KQ36 a neu (Spanne, Sprünge, Tonhöhe, Abklingen), die Auswahl der mitschwingenden Harfensaiten gegen alle 46 neu; Herleitung §4, §6, §11 nachgeführt. Kein Programmcode | hält ein Instrument nicht mehr: Vorlage vor dem Bau |
| 34k-7b Core | `InstrumentCore` nächste Fassung: `KettenNote` mit Saite und Bund (ZK2 a; ohne Angabe wie bisher); `Tonauftrag` trägt den Steg — je Stegpunkt der gespielten und der freien Saiten Frequenz, η, Formwert und Richtung je Mode, dazu n(f) und φ_rms für den Dämpfer; `Tonkern` gibt die Stegkraft je Stegpunkt aus (Vorgabe ruft `rechne`). Vorher prüfen, ob die Saitenwahl der App Chöre (Laute, Oud) trägt (Kipppunkt ZK2 a) | andere Kerne Bit für Bit gleich; neue Fälle in `test_InstrumentCore` |
| 34k-7c Umlauf-Filter | Glieder in `InstrumentModels/gemeinsam` (Thiran 5, Kette aus Biquad-Allpässen, Verlust 8. Ordnung); Entwurf in `InstrumentMeasure/ZupfStimmen` (ZK1 a: Bank & Välimäki, Abel & Smith, Levenberg–Marquardt über logit; Tafel je Taste für Harfe und Cembalo, je Saite gebunden über Legendre für Gitarre, Laute, Oud); Rechenzeit des Entwurfs je Instrument gemessen (Kipppunkt ZK1 a) | an Stichtasten dieselben Zahlen wie die Proben S204 / S205 (Cent, Abklingen nach UL2 a), Ausnahme Laute Chor 8.1 |
| 34k-7d Kern | Rechenfassung 2, Rechenweg Note: Vorgaben der Saiten je Instrument mit Quelle und Belegkraft, Zupf als Kraftstufe, Steg im Zeitschritt („genau für stückweise lineare Kraft“, Durchgriff), Dämpfer über 5 kHz als sieben breite Moden, Mitschwingen je Note, Tastenstoß (GZ10 a, KQ75 a), Lautenzug (GZ11 a, Kipppunkt Stabilität hier entschieden), Gruppe *Pedals* (PD1 a, TM2 a); Tafel neu | am Ton gegen eine Bezugsdatei des Prüfstands (Gitarre ≤ 0,01 LU, Cembalo ≤ 0,14 LU, Cent nach UL2 a) |
| 34k-7e App | der Körper je Note in den Auftrag (aus `BodenImKlang`, Stegpunkte der gespielten und der freien Saiten), eine Faltung je Stegpunkt, der Tonname trägt den Abdruck des Körpers; Saite und Bund aus `saitenSpielFuer` | Linearität (Clip minus Summe der Noten) unter −120 dB, `rechneZupfClip` als Bezug |
| 34k-7f Abschluss | Stimmer neu (Filtertafel und Feinzug am Grundton), `zupf_ic.txt` mit Vermerk, Journal; Messfall `LUMIVIZ_34K_MESSUNG=<datei.md>` mit der Messkette nach KQ36 a (Lautheit aus `AudioCore`) | die Zielgrößen aus 34k-7a am gebauten Kern; Rechenzeit gemessen und berichtet; am Stufenende die engen Fälle unter beiden Compilern und der Bereich (MT1 a) |

**Schnittfolge 34k-7d** (S222, mit ZB1 a, ZU1 a; Nummern fest, innerhalb von 34k-7d). Der Stimmer zieht mit dem Entwurf der Tafel aus 34k-7f nach 7d-3 vor (die Tests brauchen eine abgelegte Tafel), die Abdrücke von Rechenfassung 2 nach 7d-4 (ZB1 a); in 34k-7f bleiben Feinzug am Grundton, Wächter und Messfall.

| Schritt | Inhalt | Prüfung |
|---|---|---|
| 7d-1 Bezug | Zupfstelle der Gitarre (16 cm gegen rund 6,5 cm, Regel an hohen Bünden) als Vorlage mit Zahlen des Prüfstands gegen alle Zielgrößen; danach schreibt ein Skript `werte/zupf_pruefstand.txt`: Stichtasten je Instrument mit Stegkraft im Zeitverlauf, Teiltönen (Lage, Abfall) und Lautheit am Hörort | der Bezug trifft den Kandidaten S200 |
| 7d-2 Core | `InstrumentCore` 0.8.0: eine Liste darf gestimmt sein (ZK1 a), mit Abdruck und Wächter wie bei Kurven | andere Kerne Bit für Bit gleich; neue Fälle in `test_InstrumentCore` |
| 7d-3 Tafel | Stimmer entwirft die Tafel aus den Werten der Saiten; ein Schreiber legt sie für die Tests ab (rund 2 h, angesagt) | die Tafel hält UL2 a wie die Proben S220 / S221 |
| 7d-4 Kern | neue Tafel, Rechenweg Note für eine Saite an einem Stegpunkt: Saitenwerte mit Quelle, Zupf als Kraftstufe (ZU1 a), zwei Polarisationen, Umlauf-Filter, Steg „genau für stückweise lineare Kraft“ mit Durchgriff, Dämpfer als sieben breite Moden, Ausgabe über `rechneAmSteg`; Rechenfassung 1 geht (ZB1 a) | gegen den Bezug: Gitarre ≤ 0,01 LU, Cembalo ≤ 0,14 LU, Lage nach UL2 a |
| 7d-5 Mitschwingen | mehrere Stegpunkte über den Körper gekoppelt (Harfe 20 Saiten nach GZ8 a; Gitarre, Laute, Oud die leeren Saiten); Probe Durchgriff je Stegpunkt (Herleitung §12) | Harfe gegen den Prüfstand mit Mitschwingen |
| 7d-6 Cembalo, Harfe | Tastenstoß (GZ10 a, KQ75 a), Lautenzug mit Probe der Stabilität (Kipppunkt GZ11 a), Gruppe *Pedals* (PD1 a, TM2 a) | alle fünf gegen den Bezug; Rechenzeit als Anhalt |

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

✅ **PD1 a** (Patrik S194): **Die Gruppe *Pedals* kommt mit 34k-7 in die Tafel**, zusammen mit dem Mitschwingen je Note —
sieben Werte C–H (−1/0/+1), die die mitschwingenden Saiten der Harfe stimmen. Bis dahin trägt die Harfe ihre 47 Saiten
in C-Dur unter *Sympathetic strings*, der Clip stimmt sie wie IC mit `harfeMitPedalen`. Grund: im Rechenweg Note wirkt
eine Pedalstellung erst mit dem Mitschwingen; ein Regler ohne Wirkung wäre im Creator ein falsches Versprechen, und die
Abdrücke gestimmter Werte hingen an einer Größe, die nichts rechnet. Die Tafel wird erst mit den Dateien (34k-9) fest,
also ändert sich bis dahin keine Datei. Verworfen: b (sieben Ganzzahlen schon in 34k-2, ohne Wirkung bis 34k-7). Kippt,
wenn vor 34k-7 etwas die Pedale aus der Tafel lesen muss.

**Haltepunkt 34k-4 (S195).** Grundlage: `../modellierung/gezupfte-saite/recherche/2026-11-12_S195_gegenlesen.md`
(am Original gelesen), `…_ladeliste.md` (QZ1 a: Stufe 1 und 2 geladen), `../modellierung/gezupfte-saite/2026-11-12_S195_ziele_aufnahmen.md`.
Für keine Option waren die Zielgrößen schon gerechnet — das leistet die Probe in 34k-5; die Entscheide legen fest, welchen
Weg 34k-5 zuerst baut und misst.

✅ **KQ1 a** (Patrik S195, „alle a“): **Vorbild der `concert_harp` ist die Salvi „Orchestra“ nach Bell 1988** — 46 Saiten
(00–44, D1 bis G7; Tab. 2.1, 2.2 mit Kern- und Wickeldraht der elf Metallsaiten), der Körper aus dem Bauplan (Abb. 2.4–2.9:
Fichtenplatte 131 cm, 41→10 cm, 10→3 mm, Furnier, Deckleiste, zwei Leisten, Kasten, Öffnungen, Saitenorte) mit
`InstrumentBody`. Der Zupf kommt als Geste aus Chadefaux 2012/2013 (Ort als Bruchteil der Länge, Auslenkung,
Geschwindigkeit), Vermerk „an der Camac Atlantide gemessen“; die tiefen Moden der Atlantide (Le Carrou 2007) sind
Gegenprobe der Größenordnung, nicht Vorgabe. **Folge: die Harfe trägt 46 Saiten, nicht 47** (PD1 a, Tafel, GZ3 a „alle
47“ entsprechend). Grund: nur hier kommen Saiten und Körper als gemessene Maße aus einem Instrument, und der Körper ist
ein Bauplan. Verworfen: b (Camac Atlantide nach Le Carrou 2006: 47 Saiten mit Länge, Durchmesser, Winkel, Lage — aber
Werkstoffwerte der Saiten von Bell geliehen, Boden nur als Ersatzbalken auf eine Mode gestellt). Kippt auf b, wenn die
Probe in 34k-5 zeigt, dass `InstrumentBody` den halbkonischen Kasten mit der verjüngten Platte nicht abbildet, oder wenn
Patrik die 47. Saite (C1) behalten will.

✅ **KQ2 a** (Patrik S195): **Vorbild des `harpsichord` ist der Taskin 1769** — Mensur und Zupfpunkt aus dem Datenblatt
Edinburgh (elf Tasten), Besaitung nach Di Veroli (nach dem Gegenlesen). Der Körper kommt aus Plattenwerten über den
statistischen Teil und den belegten tiefen Moden, mit Vermerk, dass diese an anderen Instrumenten gemessen sind (Dulcken
1755, Välimäki 2004). Zupfpunkte der übrigen Register aus dem gemessenen und dem Springerabstand, Belegkraft
„abgeleitet“; Kielwerte nach Perng als Startwerte, Vermerk „Modellwert“. Verworfen: b (Körper als „gedachtes
Instrument“), c (erst Savage 1992 über die Bibliothek beschaffen). Kippt auf c, wenn die Probe in 34k-5 die Sprünge unter
600 Hz ohne gemessene Moden nicht unter die Grenze bringt; der Körper steht als Daten mit Abdruck in der Tafel und ist
ersetzbar, sobald Savage vorliegt.

✅ **KQ3 a** (Patrik S195; **ergänzt GZ4 a um einen dritten Weg**): **Die Stegmoden der `classical_guitar` kommen aus
einem Modalfit an einer Gitarre des Archivs Mores 2021** (Frequenz, Dämpfung, Wert am Stegpunkt; Weg nach Ducceschi
2026); welche Gitarre, legt 34k-5 mit Begründung fest. Abstrahlgewichte aus dem statistischen Teil über Plattenwerte; die
zweite Richtung am Steg bleibt Annahme mit Vermerk. Saiten nach Woodhouse u. a. 2012 Tab. I. Grund: der Steg ist die
gemessene Ursache der Sprünge, eine gemessene Stegmobilität die stärkste Quelle. Verworfen: b (Plattenwerte und tiefe
Moden, das Archiv nur als Gegenprobe). Kippt auf b, wenn der Fit oberhalb von rund 1 kHz keine stabilen Moden liefert.

✅ **KQ4 a** (Patrik S195): **Die `lute` bekommt die Saiten nach Woodhouse & Lynch-Aird 2019 Tab. II** (8-chörig in G,
0,625 m); der Körper aus Plattenwerten über den statistischen Teil — Fläche und Luftraum aus der Messzeichnung Rauwolf
1598, die tiefen Moden aus der Kurve bei Woodhouse 2021 abgelesen (Vermerk „aus der Kurve gelesen, andere Laute“). Der
Zupf wird von der Gitarre übertragen (Woodhouse 2012), Belegkraft „übertragen“; Dynamik und Ausklang gegen die Gitarre
plausibilisiert, weil Aufnahmen fehlen — das steht im Bericht der Stufe. Verworfen: b („gedachtes Instrument“). Kippt
auf b, wenn sich aus der Kurve keine drei bis vier Moden sicher ablesen lassen.

✅ **KQ5 a** (Patrik S195): **Die `oud` wird in 34k gebaut** — Saiten nach Herstellerangaben (Vermerk „Hersteller“), der
Körper aus Plattenwerten mit den vier gemessenen Moden nach Abdallah 2026 als Anker (142 / 226 / 242 / 280 Hz), die Risha
als Plektrum mit gesetzten Werten (Vermerk „Annahme“). Geprüft gegen Lautheit über den Umfang, Sprünge und Tonhöhe;
Dynamik und Ausklang nur gegen Laute und Gitarre plausibilisiert, vermerkt in Tafel und Bericht. Verworfen: b (aus 34k
herausnehmen, bis Quelle zur Risha und freie Einzeltöne vorliegen). Kippt auf b, wenn Patrik nur Instrumente mit
gemessenen Zielen ausliefern will.

**Haltepunkt 34k-5 (S195).** Probe `../modellierung/gezupfte-saite/2026-11-12_S195_probe_statistik_ohne_bauplan.md`:
der statistische Teil nimmt seine Größen aus gerechneten Moden und liefert ohne Bauplan nichts; Plattenformeln treffen an
den drei Flügelkörpern die Modendichte oberhalb 2,5 kHz auf −16 … +32 %, unter 1 kHz liegen sie um den Faktor 1,2–3,3 zu
hoch, der Formwert am Stegpunkt liegt um 1,3–4,5 unter 1/√M und streut zwischen den Stegpunkten um 2–8; für das
Abstrahlgewicht gibt es keine Formel.

✅ **KQ6 a** (Patrik S195; **ersetzt in KQ2 a, KQ4 a, KQ5 a den Weg „Plattenwerte über den statistischen Teil“**):
**Cembalo, Laute und Oud bekommen je einen Ersatz-Bauplan (`.lvinstbodyplan`), `InstrumentBody` rechnet ihre Moden wie
beim Flügel.** Belegte Maße gehen ein (Laute: Umriss, Rose, Steglage nach Rauwolf 1598; Oud: Balken und Holzwerte nach
Abdallah 2026; Cembalo: Mensur und Saitenorte nach Edinburgh, Bodendicke 2–4 mm nach Välimäki 2004); jedes fehlende Maß
steht als Annahme mit Vermerk im Bauplan. Die Annahmen werden so gesetzt, dass die gerechneten tiefen Moden die belegten
treffen (Oud 142 / 226 / 242 / 280 Hz; Laute aus der Kurve bei Woodhouse 2021; Cembalo Luftresonanz 37 Hz); der Abstand
steht im Bericht. **Erster Fall ist die Oud**; verfehlt sie die vier Moden um mehr als 10 %, kommt der Weg erneut zur
Vorlage. Grund: nur so entstehen einzelne Moden mit Wert am Ort jeder Saite und ein Abstrahlgewicht aus derselben
Rechnung (KB1 a, GZ4 a). Verworfen: b (gemeinsames Plattenmodell mit Abschlägen aus der Probe — am Flügelboden gemessen,
nicht übertragbar; ein Steg für alle Tasten), c (GZ4 a kippt auf „gedachtes Instrument“). Kippt auf b, wenn der erste
Fall die Moden nicht innerhalb von 10 % trifft und sich das nicht auf eine einzelne Annahme zurückführen lässt; für die
Oud auf c, wenn sich die Deckendicke weder belegen noch aus den vier Moden eingrenzen lässt.

**Erster Fall (S195)** `../modellierung/gezupfte-saite/2026-11-12_S195_oud_erster_fall.md`: der Ersatz-Bauplan der Oud
(Umriss und Balkenlagen aus Abb. 3 gelesen, ohne Löcher, ohne Steg) trifft die erste Mode bei jeder Dicke von 1,5–2,5 mm
(138–154 gegen 142 Hz), aber mit keiner Dicke alle vier innerhalb von 10 % (1,75 mm: +1 / −21 / +20 / +9 %); die
Reihenfolge der Formen ist anders als in der Quelle. Schwelle verfehlt.

✅ **KQ7 a** (Patrik S195): **`InstrumentBody` bekommt Löcher in der Platte** — im Bauplan eine Liste geschlossener
Innenumrisse, im Netz ausgespart, ihr Rand frei, in der Abstrahlung ohne Fläche. Bestehende Baupläne ohne Löcher rechnen
Bit für Bit wie vorher; geprüft an einer Platte mit Loch gegen eine geschlossene Lösung oder eine feinere Rechnung. Danach
die Oud mit ihren drei Löchern (Ø 10,5 cm in 30,0 cm Höhe; zwei Ø 3,9 cm in 20,4 cm Höhe, ±10,3 cm) neu: trifft sie die
vier Moden innerhalb von 10 % bei einer Dicke zwischen 1,5 und 3 mm, gilt KQ6 a weiter; sonst werden die Bildformen der
Quelle Mode für Mode zugeordnet, der Abstand je Form berichtet, und der Weg kommt mit diesen Zahlen noch einmal zur
Vorlage. Grund: ein Loch von 10,5 cm in einer Decke von 37 cm fehlt an der Stelle, an der die Quelle die Bewegung der
ersten Mode zeigt, und Löcher gehören bei Oud, Gitarre, Laute und Cembalo zur Geometrie. Verworfen: b (Bauplan ohne
Löcher, Dicke auf die erste Mode, Abweichung bis 21 % als bekannt geführt), c (KQ6 a kippt auf das gemeinsame
Plattenmodell). Kippt auf b, wenn die Oud auch mit Löchern nicht trifft und die Zuordnung zeigt, dass Messung und Rechnung
nicht dieselben Moden meinen — dann wird die Schwelle an Laute oder Cembalo neu geprüft.

**KQ7 a gebaut, zweiter Fall (S196)** `../modellierung/gezupfte-saite/2026-11-13_S196_oud_zweiter_fall.md`:
`InstrumentBody` rechnet Löcher (`.lvinstbodyplan` Fassung 4, `plate.holes`; der Kreisring gegen Kirchhoff auf 0,09 %;
die drei Körper des Flügels neu gerechnet, gleich bis auf Abdruck und Rechenzeit). Die Oud mit ihren drei Löchern:
143 / 169 / 260 / 276 Hz bei 1,5 mm (+1 / −25 / +7 / −1 %), 148 / 186 / 291 / 304 Hz bei 1,75 mm — die Löcher heben
die ersten Moden um 3–5 %, **die Schwelle ist wieder verfehlt**. Zuordnung über `InstrumentBodyTool --shapes`: die
Arten der Formen und ihre Reihenfolge stimmen mit der Rechnung der Quelle überein (die Lesart von S195 „zweite Mode
mit zwei Bäuchen nebeneinander“ war falsch); die Abstände je Form (bis −20 % und +33 %) schließt keine Dicke.
Unbelegte Annahmen wirken stärker als die Schwelle (Balkenenden am Rand: erste Mode 148 → 228 Hz), und eine falsche
Annahme erfüllt sie (Faser quer, 2,5 mm: 156 / 208 / 240 / 285 Hz) — vier Frequenzen ohne gemessene Form prüfen den
Bauplan nicht. Die Messung stammt von einer Oud mit Luftraum, die Rechnung hat keinen.

✅ **KQ8 a** (Patrik S196): **KQ6 a kippt noch nicht — zuerst zieht der Luftraum nach `InstrumentBody`** (F-K1 a, in
34k-5 ohnehin vorgesehen), geprüft gegen Helmholtz und Kasten. Danach wird die Oud mit Luftraum gegen die vier
gemessenen Spitzen gerechnet; Deckendicke, Anschluss der Balkenenden und Schalentiefe stehen als benannte Annahmen im
Bauplan. Die Schwelle heißt dann: jede gemessene Spitze hat eine gerechnete Mode innerhalb von 10 %, und die
Zuordnung der Formen wird mit dem Bild belegt. Verfehlt die Oud auch das, kommt KQ6 a zur Vorlage. Grund: Messung
und Rechnung beschreiben heute nicht dasselbe Ding (ein Überschlag mit angenommener Schalentiefe von 19 cm legt die
Helmholtz-Frequenz auf rund 125 Hz, dicht an der ersten gemessenen Spitze), und KQ6 a trägt auch Laute und Cembalo.
Verworfen: b (KQ6 a kippt nach seinem Wortlaut auf das gemeinsame Plattenmodell — dessen Abweichungen aus der Probe
S195 sind größer als alles, was die Oud verfehlt), c (Bauplan mit Löchern bei 1,5 mm, −25 % an der zweiten Mode als
bekannt geführt, Schwelle an der Laute neu — dort wartet dieselbe Lücke), d (Faser quer, weil sie trifft —
widerspricht Instrument und Quelle). Kippt auf b, wenn die Oud mit Luftraum und belegbarer Schalentiefe weiter mehr
als 10 % verfehlt und sich das nicht auf eine Annahme zurückführen lässt; auf c, wenn sich die Schalentiefe nicht
belegen lässt und der Luftraum damit nur eine weitere freie Annahme wäre.

**KQ8 a gebaut (S196)** — `InstrumentBody/Luftraum.hpp`: die Moden der Platte (im Vakuum) und die Luft in den Löchern,
gekoppelt über den Druck des Hohlraums, exakt als Diagonale plus Rang 1; im Bauplan der Block `cavity` (Fassung 4);
geprüft gegen Helmholtz, den Kasten ohne Loch, zwei gekoppelte Schwinger und den dichten Löser von Eigen. Die Oud
(Bericht §7): aus Abb. 3 am Bild gelesen die Schale 21 cm tief, Löcher R 5,3 und R 2,0 cm; mit „Querschnitt halbe
Ellipse“ 20,6 l. **142 Hz ist die Mode der Luft, 226 Hz die von ihr verschobene erste Deckenmode.** Mit Balken am
Rand und 1,5 mm: 132 / 243 / 305 / 324 Hz — drei gemessene Spitzen mit eigener Mode (−7 % / +7,5 % / +9 %), die Spitze
bei 242 Hz ohne eigene Mode; mit frei endenden Balken liegt die Luftmode bei 100–117 Hz. Netzunabhängig (drei Netze).

✅ **KQ9 a** (Patrik S196): **KQ6 a gilt weiter, die Oud bekommt diesen Bauplan** — drei Löcher, Luftraum (Schale
21 cm, 20,6 l), Balken am Rand, 1,5 mm Decke; Dicke, Balkenanschluss und Querschnitt der Schale stehen als Annahmen
mit Vermerk im Bauplan, die Spitze bei 242 Hz als „ohne Gegenstück“ im Bericht. Cembalo und Laute folgen auf demselben
Weg, jeweils mit Luftraum. Grund: die Rechnung erklärt die beiden kräftigsten Spitzen (Luft und verschobene
Deckenmode) aus einer belegten Schalentiefe — das sind die Moden, die die Saite am Steg sieht. Verworfen: b (KQ6 a
kippt auf das Plattenmodell — es hat keinen Luftraum und damit die 142 Hz nicht), c (Oud zurückstellen, erst die
Laute — verbessert die Oud nicht, die Laute kommt ohnehin als Nächstes). Kippt auf c, wenn „Balkenenden am Rand“
als unbelegte Annahme nicht tragen soll; auf b, wenn die Laute mit Luftraum ihre gemessene Kurve um mehr als 10 %
verfehlt und sich das nicht auf eine Annahme zurückführen lässt. **Ersetzt durch KQ10 a.**

**Befund an der gemessenen Kurve (S196, Bericht §8).** Abb. 6 der Quelle, erstmals als Bild gelesen und
digitalisiert: Spitzen bei **101 / 139 / 203 (die stärkste, Gruppe 197–210) / 280 / 362 / 385 Hz**, bei 242 Hz ein
Tal. Tab. 3 (142 / 226 / 242 / 280) ordnet die Messung den vier Moden der FE-Rechnung der Quelle zu und ist keine
Liste der Spitzen. Die Übertragungskurve am Messpunkt gerechnet: mit den Balken, wie Abb. 3 sie zeichnet
(1,3–1,5 cm vor dem Rand endend), und 1,5 mm Decke liegen 101 / 203 / 280 / 362 Hz bei 0 / +3 / −1 / 0 %; die
Fassung nach KQ9 a bei +31 / +20 / −13 / +5 %. 101 Hz ist die Mode der Luft, 203 Hz die verschobene erste
Deckenmode. Die Spitze bei 139 Hz hat kein Gegenstück (gerechnet eine gegengleiche Mode bei 169 Hz); weder ein
Steg noch die Lage der kleinen Löcher nach Abb. 5 b schiebt sie dorthin.

✅ **KQ10 a** (Patrik S196; **ersetzt KQ9 a**): **Die Oud bekommt die Balken, wie Abb. 3 sie zeichnet** (aus dem
Bild gelesen, 1,3–1,5 cm vor dem Rand endend), drei Löcher, Luftraum 20,6 l, Decke 1,5 mm, kein Steg. Zielgrößen sind
die Spitzen der gemessenen Kurve bei 101 / 203 / 280 / 362 Hz, innerhalb von 3 %. Im Bericht: 139 Hz „ohne
Gegenstück, möglich eine Mode des ganzen Instruments“ (es war frei gelagert, die Rechnung hält Schale und Rand
fest), die gerechnete Mode bei 169 Hz „in der Messung nicht als Spitze“. KQ6 a gilt weiter; Cembalo und Laute
folgen mit Luftraum. Grund: vier der sechs Spitzen innerhalb von 3 %, darunter die Mode der Luft und die stärkste
Spitze, und die unbelegte Annahme „Balken am Rand“ entfällt. Verworfen: b (1,6 mm — trifft auch 385 Hz, die vier
großen etwas schlechter; der Unterschied liegt in der Lesegenauigkeit), c (KQ9 a bleibt — verfehlt die Mode der
Luft um 31 % und die stärkste Spitze um 20 %), d (Oud zurückstellen, bis 139 Hz geklärt ist — die Decke allein
gibt die Spitze in keiner Gegenprobe her). Kippt auf d, wenn die Oud nur mit einem Bauplan ausgeliefert werden
soll, der jede gemessene Spitze erklärt. **Lehre:** eine Tabelle „gemessen gegen gerechnet“ ist die Zuordnung der
Quelle — die gemessene Kurve selbst lesen, bevor eine Schwelle an ihr hängt.

**KQ10 a gebaut (S196)** — `asset/loop/instrument_bodies/oud.lvinstbodyplan` (Schreiber `make_oud.py`) und
`oud.lvinstbody` (243 Moden bis 5 kHz): sechs Stegpunkte, je Chor einer (Tasten 36 / 41 / 45 / 50 / 55 / 60, die
leeren Saiten); Dämpfung α = 0,06 f + 23 1/s aus den Halbwertsbreiten der gemessenen Kurve. Gekoppelte Moden
99,9 / 168,6 / 207,8 / 269–280 / 351 / 362 Hz, gegen 101 / 203 / 280 / 362 Hz −1 / +2 / −2 / 0 %. Der Körper
trägt je Mode den Anteil der Luft (`.lvinstbody` Fassung 3, `airShare`), der Wächter rechnet die Dämpfung damit
nach. Bericht §9.

**Befund an der Laute (S197, Bericht `../modellierung/gezupfte-saite/2026-11-13_S197_laute.md`).** Woodhouse 2021
Abb. 1 (Stegadmittanz der Laute, zwischen Chor 1 und 2, dB re 1 m/s/N) als Bild gelesen: Spitzen bei **132 / 150 /
277 / 341 / 378 / 476 Hz** mit −28 / −34 / −17 / −20 / −32 / −12 dB. Die Rauwolf-Zeichnung ist ein Vektor-PDF 1:1 und
gibt Umriss, Rose, Steg und über sieben Querschnitte den Hohlraum (14,0 l), aber weder Balken noch Deckendicke; ihre
Mensur ist 677,4 mm, die der gemessenen Laute und des Saitensatzes 625 mm. Die Decke allein ist am Steg 12–18 dB
beweglicher als gemessen — es fehlen die Stegleiste und die Last der gedämpften Saiten (Summe der Wellenwiderstände
aus Woodhouse & Lynch-Aird 2019 Tab. II: 2,47 N s/m; dass die Saiten gedämpft waren, ist Annahme). Eine Schwelle nur
auf die Lage lässt einen Bauplan durch, der 3 % hält und die stärkste Spitze einer am Messpunkt unsichtbaren Mode
zuordnet (−17 dB).

✅ **KQ11 a** (Patrik S197): **Zielgrößen der Laute sind die vier Spitzen 132 / 277 / 341 / 476 Hz in Lage und Höhe**
— Lage innerhalb von 5 %, Höhe innerhalb von ±6 dB, gerechnet am Messpunkt der Quelle mit Stegleiste und Saitenlast.
Die Saitenlast gehört nur zum Vergleich; der Körper unter `asset/` ist ohne Saiten. 150 und 378 Hz stehen im Bericht
als „ohne Gegenstück“. Grund: erst die Höhe am Messpunkt zeigt, ob eine gerechnete Mode die gemessene ist, und sie
ist, was die Saite am Steg sieht. Verworfen: b (nur die Lage, 3 % wie bei der Oud — lässt die falsche Zuordnung
durch), c (3 % und ±6 dB zugleich — kein Fall hält beides, der Bauplan stammt von einer anderen Laute als die
Messung). Kippt auf c, wenn ein Fall beides hält; auf b, wenn die Annahme „Saiten gedämpft“ nicht Teil einer
Zielgröße sein soll.

✅ **KQ12 a** (Patrik S197; **ergänzt KQ4 a und KQ6 a**): **Der Bauplan nimmt die Form der Rauwolf-Laute, alle Längen
mal 625/677,4** (Mensur der gemessenen Laute und des Saitensatzes; Hohlraum 11,0 l), mit Vermerk im Bauplan. Grund:
Saiten, Stimmung und gemessene Admittanz gehören zu einer Laute mit 0,625 m; mit gleichen Annahmen liegt der Bauplan
1:1 bei −19 / −15 / −8 %, verkleinert bei −1 / −5 / +2 / +3 %. Verworfen: b (Rauwolf 1:1, die Annahmen gleichen den
Größenfehler aus), c (Rauwolf 1:1 als eigenes Instrument ohne gemessenes Ziel). Kippt auf b, wenn unveränderte Maße
wichtiger sind als die Übereinstimmung mit Saiten und Messung.

**KQ11 a und KQ12 a gebaut (S197)** — `asset/loop/instrument_bodies/lute.lvinstbodyplan` (Schreiber `make_laute.py`)
und `lute.lvinstbody` (128 Moden bis 5 kHz): Decke 1,8 mm, sieben Querbalken bis an die Muschel (14 mm, der zweite
9,8 mm), Rose zu 85 % offen, Stegleiste 7 g, Dämpfung α = 60 1/s und Güte der Luft 12 aus den Höhen der gemessenen
Kurve — alles Annahmen mit Vermerk; acht Stegpunkte, je Chor einer (Tasten 38 / 41 / 43 / 48 / 53 / 57 / 62 / 67).
Gekoppelte Moden 132,1 / 264,1 / 334,6 / 471,8 / 488,6 Hz; die Stegadmittanz mit Saitenlast gegen die vier Ziele:
Lage −1,8 / −4,6 / −1,3 / +3,6 %, Höhe −0,3 / +4,7 / −2,7 / +0,6 dB. Test „Loop Koerper 34k-5 Luftraum die
mitgelieferte Laute“ prüft beides am Körper. Offen (Bericht §7): 150 und 378 Hz, die Steife am Steg unter 120 Hz, die
Breite der Luftmode.

**Befund am Cembalo (S197, Bericht `../modellierung/gezupfte-saite/2026-11-13_S197_cembalo.md`).** Eine gemessene
Kurve des Bodens gibt es in den geladenen Quellen nicht. Bader, Plath & Kontopidis 2025 zeigt dafür den Boden des
Dulcken 1755 fast als Bauplan: Abb. 2 (Eingabe ihres Rechenmodells: Umriss, beide Stege mit Stiften, Anhangleiste des
4′, Querleiste, vier Rippen), Abb. 1 (gemessene Dicke, 497 Punkte, 2,4–7,5 mm), dazu gemessen die Luftresonanz 37 Hz
und die Abklingzeit des Bodens 235 ms. Maßstab aus den Stufen des Umrisses: 14 px je cm, Boden 166,4 × 79,7 cm
(Gegenprobe über die Breite des Instruments, 847 mm). Der Dulcken hat 52 Tasten (H₁–d‴), der Taskin 61.

✅ **KQ13 a** (Patrik S197; **ersetzt in KQ2 a und KQ6 a den Ersatz-Bauplan nach dem Taskin**): **Der Körper des
`harpsichord` ist der Boden des Dulcken 1755 nach Bader u. a. 2025** — Umriss, Stege, Stiftorte, Rippen, Querleiste,
Anhangleiste aus Abb. 2, Dicke aus Abb. 1. Die Saiten bleiben die des Taskin 1769 (KQ2 a); Vermerk in Bauplan und
Tafel „Boden Dulcken 1755, Saiten Taskin 1769“. Grund: Geometrie und Zielgrößen stammen aus demselben Instrument, fast
jedes Maß des Bodens ist gemessen statt gesetzt. Verworfen: b (Ersatz-Bauplan Taskin aus elf Mensuren — außer der
Steglinie nichts belegt), c (das Instrument ganz als Dulcken — seine Mensuren und Saiten sind nicht abgedruckt). Kippt
auf c, wenn sich die Mensuren des Dulcken beschaffen lassen.

✅ **KQ14 a** (Patrik S197): **Die gemessene Dicke geht als Feld in den Bauplan.** Befund danach: `InstrumentBody` hat
das Feld seit Fassung 1 (`plate.thicknessGrid`, der Steinway nutzt es) — die Vorlage nannte irrtümlich einen Eingriff
in die Bibliothek; es gibt keine neue Fassung und keine neu zu rechnenden Körper. Gilt auch für die Harfe (Bell 1988:
10 → 3 mm).

✅ **KQ15 a** (Patrik S197): **Die Tiefe des Hohlraums bleibt bei 20,7 cm** — dem Wert, bei dem die Formel für starre
Wände die 37 Hz gibt, die die Quelle aus Lochradius und Volumen selbst rechnet, und der zu 256 mm Höhe des Instruments
passt. Geprüft wird Helmholtz bei starrer Wand 37 Hz innerhalb von 3 % und die Dämpfung aus der gemessenen Abklingzeit
(α = 6 ln 10 / 0,235 s). Die gekoppelte Luftmode liegt bei 31,7 Hz, **14 % unter der Messung — bekannte Abweichung**,
in Bauplan und Bericht mit den möglichen Ursachen. Verworfen: b (Tiefe 13–15 cm, bis die gekoppelte Mode trifft — mit
der Höhe des Instruments nicht vereinbar), c (erst die Ursache suchen — es gibt keine zweite Messgröße, die die
Kandidaten trennt). Kippt auf c, wenn eine gemessene Kurve des Dulcken oder Savage 1992 vorliegt; auf b, wenn die
Luftmode in 34k-6 hörbar falsch liegt.

✅ **KQ16 a** (Patrik S197; **berichtigt KQ13 a**: die verlängerte Steglinie verlässt den Boden): **Die 52 Tasten des
Dulcken (35–86) behalten ihren Punkt; die Tasten 29–34 bekommen den Punkt der Taste 35, die Tasten 87–89 den der
Taste 86**, mit Vermerk je Punkt. Grund: 52 von 61 Tasten liegen dort, wo am Dulcken die Saite gleicher Tonhöhe den
Steg trifft. Verworfen: b (61 Tasten gleichmäßig über die Steglinie — jede Taste eigener Punkt, aber keine am Ort
ihrer Tonhöhe), c (`harpsichord` auf 52 Tasten — widerspricht KQ2 a). Kippt auf b, wenn in 34k-6 an den gleichen
Basspunkten ein Sprung über 6 LU entsteht.

**KQ13 a bis KQ16 a gebaut (S197)** — `asset/loop/instrument_bodies/harpsichord.lvinstbodyplan` (Schreiber
`make_cembalo.py`) und `harpsichord.lvinstbody`: 61 Stegpunkte auf der Linie des 8′-Stegs, Dicke als Raster von 2 cm,
Buche für die Stege, Breiten der Leisten und Querschnitte der Stege als Annahme mit Vermerk. 389 gekoppelte Moden bis 5 kHz (5,7 min): Luft 31,7 Hz, Boden 95,9 / 117,8 / 166,1 / 183,0 / 225,0 Hz; Helmholtz bei
starrer Wand 37,0 Hz. Test „Loop
Koerper 34k-5 Luftraum das mitgelieferte Cembalo“. Offen (Bericht §8): keine gemessene Mode des Bodens, Dämpfung für
alle Frequenzen gleich, Dicke im Diskant eher zu dick gelesen. **Als Nächstes die Harfe** (Bell 1988), dann die
Gitarre.

**Befund an der Harfe (S197, `../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md`).** Bell 1988 am Bild
gelesen: Boden 131 cm, 41 → 10 cm breit, 10 → 3 mm dick, Faser quer, Deck- und Zugleiste aus Buche, zwei
Fichtenleisten; **sechs Öffnungen in der Rückseite der Schale** (Tab. 4.1: 58–93 cm², Lippen 1,2–2,1 cm), Luftvolumen
27 ± 2 l, Helmholtz bei starrer Wand 190 Hz; Tab. 6.1: 17 gemessene Moden des Kastens F mit Güte (186 / 204 / 250 /
289 / 332 / 421 / 468 / 535 Hz …), darunter das gekoppelte Paar aus Luft und erster Bodenmode (186 / 204 Hz) und
Kopplungen mit höheren Luftmoden (289, 637, 720 Hz). `InstrumentBody` kannte Öffnungen nur als Löcher der Platte
(`Luftraum.hpp`, `luftraumGroessen`) und rechnet nur die Helmholtz-Mode.

✅ **KQ17 a** (Patrik S197): **Der Hohlraum bekommt Öffnungen außerhalb der Platte** — `cavity.openings` (je Öffnung
Ort, Höhe über der Platte, Fläche, Lippenlänge, Quelle), immer geschrieben, **Bauplan Fassung 5**; `luftraumGroessen`
nimmt Löcher und Öffnungen parallel, eine Öffnung strahlt als freie Punktquelle von ihrem Ort. Bestehende Baupläne
rechnen gleich; alle sechs Körper neu gerechnet und verglichen, voller Lauf unter beiden Compilern. Grund: nur so ist
abgebildet, was gebaut ist. Verworfen: b (Öffnungen als Löcher im Boden — 450 cm² aus einem Boden, der an der
schmalen Stelle 10 cm breit ist), c (Harfe ohne Luftraum — gibt das Paar 186 / 204 Hz auf). Kippt auf c, wenn der
Eingriff nicht gewünscht ist und die Harfe nur oberhalb von 250 Hz stimmen soll. **Offen:** die höheren Luftmoden
im Kasten (Vorlage, sobald die Harfe einmal gerechnet ist und Tab. 6.1 mit Zahlen zeigt, was ohne sie fehlt); der
Überschlag mit einzelnen Öffnungen gibt 225 Hz statt der gemessenen 190 Hz — die Mündungskorrektur wird an Bells
Messung gestellt.

**Befund am Rand des Harfenbodens (S198).** Kap. 7 von Bell gelesen (Kasten G lose, in der Harfe, besaitet): ab
S(1,3) ändern Einbau und Saiten die Lage um höchstens 2–4 %, zwei Kästen desselben Modells liegen bis 6 %
auseinander, die Saiten sitzen auf der Knotenlinie aller S(2,n). Der Boden steckt zwischen Halte- und Randleiste
(Abb. 2.6). Gerechnet (Boden mit Leisten im Vakuum, geeichtes Holz, S(1,2)–S(1,7) gegen Tab. 6.1): Rand eingespannt
+47 … +16 %, gestützt −20 … −16 %, Drehfeder 3000 N bei gehaltenem w −5 … +1 %, Holz stellen (E quer × 1,5) −14 …
−10 % und die Eichung am freien Boden verloren. Zahlen in
`../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §8–§10.

✅ **KQ18 a** (Patrik S198): **Der Rand der Platte steht im Bauplan** — Block `rim`: eingespannt (Vorgabe, wie bisher)
oder elastisch (w gehalten, Drehfeder in N), immer geschrieben, **Bauplan Fassung 6**; `baueSystem` rechnet die Feder
als Randterm. Keine Wegfeder: sie bessert nur S(2,1), die die Saiten nicht anregen, und verschlechtert S(1,5)–S(1,7).
Verworfen: b (Holz oder Leisten stellen). Kippt, wenn S(2,1) Ziel wird (Stegpunkte neben der Mittellinie).
✅ **Folgeentscheid zu KQ18 a** (Patrik S198): **ein Federwert für den ganzen Rand**, kein Verlauf. Am selben Ziel ist
die größte Abweichung 5 gegen 3 % (Kasten F) und 7 gegen 5 % (Kasten G besaitet) — innerhalb der Streuung des Baus;
für den einen Wert spricht die Bauweise. Kippt, wenn die Höhen der Admittanz je Saitenort (Abb. 7.22) mit gleicher
Feder die Bäuche am Diskant sichtbar falsch setzen. **Offen:** S(2,1) 17–23 % zu hoch (kein Befund über den Rand);
Ziele und Schwelle der Harfe (Kasten G besaitet, Lage und Höhe mit Saitenlast) als nächste Vorlage.

🔴 **Berichtigung (S198, nach KQ18 a):** der Kipppunkt des Folgeentscheids ist eingetreten, und mehr als das. An der Lage der Bäuche längs der Deckleiste (Bell Abb. 7.8–7.16) geprüft, war die Zuordnung der gerechneten Formen zu Bells S(1,n) um eins versetzt (die Form zu S(1,n) hat n + 1 Bäuche, der am Bassende ist schwach). Richtig zugeordnet trifft der **gestützte Rand** (`elastic`, Feder 0) Ort und Frequenz zugleich: Lage +10 … −8 %, stärkster Bauch ±1 Messort; mit 4500 N liegen die Bäuche 6–9 Messorte zu weit am Bass. Die Zahlen „gestützt −20 … −16 %“ und die Federwerte 3000 / 4500 N im Befund oben sind hinfällig. `rim` und Fassung 6 bleiben (die Vorgabe ist eingespannt); die Harfe bekommt Feder 0. Zahlen im Bericht §11. Lehre: Moden nie nur über Frequenz und Zahl der Bäuche zuordnen, immer auch über deren Lage gegen ein gemessenes Profil.

✅ **KQ19 a** (Patrik S198): **Ziele der Harfe** — der Test prüft S(1,2)–S(1,7) am Kasten G in der Harfe (Bell Abb. 7.8–7.16): Lage ≤ 10 % gegen „ohne Saiten“ und gegen „besaitet“, Ort des stärksten Bauchs ≤ 2 Messorte, Höhe ≤ 6 dB gegen „ohne Saiten“ mit der Dämpfung des Bauplans aus Bells Güten ohne Saiten; S(1,6) fällt bei der Höhe heraus; S(1,1)− und S(1,8) stehen als bekannte Abweichung im Test. Der erste Probekörper verfehlte zwei Ziele knapp (S(1,2) Lage +10,4 %, S(1,7) Höhe −7,3 dB); nach dem Kipppunkt wurde die Ursache gesucht statt die Schwelle geweitet (Bericht §11): Leistenmasse, Wegfeder am ganzen Rand und schmalerer freier Boden scheiden aus; an Bells Messort 2 (Abb. 6.6) liegt die Messung 10–20 dB über der Rechnung, auch zwischen den Resonanzen — das Diskantende ist zu fest gehalten.

✅ **KQ20 a** (Patrik S198): **Halt je Randstück.** `rim.sections` — Abschnitte des Rands, je Abschnitt Kanten des Umrisses (`edges: [von, bis]`), eine Wegfeder (N/m²) und eine Drehfeder (N); im Abschnitt ist die Auslenkung nicht gehalten, der übrige Rand trägt `support`. **`.lvinstbodyplan` Fassung 7**, `sections` immer geschrieben, Fassung 6 liest sich ohne Abschnitte. Das hebt den Ausschluss der Wegfeder aus KQ18 a auf (seine Begründung beruhte auf der falschen Zuordnung). Geprüft gegen die Lévy-Lösung einer Platte mit einer freien Kante auf 1 %.

✅ **KQ21 a** (Patrik S198): **Fassung 7 bleibt, die Harfe bekommt an der Diskantkante 3·10⁶ N/m²** — angepasst, nicht gemessen. Am vollen Kasten gerechnet (Bericht §12) bringt keine Feder S(1,5)–S(1,8) zugleich in die Schwellen (der Kipppunkt von KQ20 a, wörtlich eingetreten); mit 3·10⁶ N/m² liegen S(1,3)–S(1,7) darin (S(1,7) −5,4 statt −7,4 dB), der Pegel an Messort 2 steigt um 5–10 dB von rund 20 dB Lücke. Bekannte Abweichungen im Test: S(1,1)−, S(1,8) (−14 dB) und die Lage von S(1,2) gegen den Kasten ohne Saiten (+10,4 %; besaitet +7 %). Offen: die übrige Ursache am Diskantende (Boden dort dünner, Kopfblock schwingt mit, Leisten enden vor der Kante — nicht untersucht), die Taste je Saite (Annahme Pedal Mitte).

✅ **KQ22 a** (Patrik S199): **Je chromatischer Taste ein Stegpunkt im Bauplan der Harfe** — 80 Stegpunkte für die Tasten 25–104 an den 46 Ösen: die weiße Taste an der Öse ihrer Saite (Pedal in der Mitte), die schwarze an der Öse der Saite darunter (Pedal unten), die Taste 25 an der tiefsten Saite (Pedal oben); je Punkt steht in `source`, welche Saite mit welcher Pedalstellung gemeint ist. Kein neues Format, keine Änderung am Programm. Befund: `stegpunktVon` (`src/loop/Boden.cpp`) sucht die Taste genau — mit 46 Punkten hätten 34 Tasten im Umfang keinen Körper, der Ton liefe dort ohne Faltung (gegen das Ziel „jede Taste“ aus 34k-6). Nachgerechnet am Körper (`proben/2026-11-15_S199_harfe_nachbarpunkte.py`): benachbarte Saiten unterscheiden sich je Terzband im Mittel um 1,5 dB (90 % unter 3,3 dB, an Knoten einzelner Moden bis 10,4 dB), breitbandig um 0,4 dB (höchstens 1,7 dB) — die Wahl „Saite darunter“ gegen „Saite darüber“ liegt unter der Schwelle von 6 dB aus KQ19 a. Grund: die Zuordnung Taste → Saite steht mit Quelle im Bauplan, wie bei den Randtasten des Cembalos (KQ16 a); das Programm muss nichts über Harfen wissen. Verworfen: b (Regel „nächster Punkt darunter“ in `stegpunktVon` — gälte für jeden Körper, gäbe bei Laute und Oud jeder gegriffenen Note den falschen Chor und ließe eine versehentliche Lücke still klingen), c (nur die weißen Tasten). Kippt, wenn die Pedalstellung in 34k-6 / 34k-7 als Spielweise bestimmen soll, welche Saite eine Taste spielt (dann trägt der Stegpunkt die Saite statt der Taste: neue Bauplan-Fassung, alle Körper neu), oder wenn in 34k-6 an einer schwarzen Taste ein Sprung über 6 LU gegen ihre weißen Nachbarn entsteht. Der Körper ist neu gerechnet (97 s): gleich bis auf Abdruck, Rechenzeit und die 34 neuen Spalten, die den Wert ihrer Saite tragen. **Nebenbefund:** Laute und Oud haben nur an den Tasten ihrer leeren Saiten einen Körper (8 und 6 Stegpunkte) — das löst nach Plan der Saitensatz in 34k-8.

**Gitarre, Modalfit am Archiv Mores (S199)** — Bericht [2026-11-15_S199_gitarre_modalfit.md](../modellierung/gezupfte-saite/2026-11-15_S199_gitarre_modalfit.md). Weg nach Ducceschi 2026 §3.4 mit zwei Abweichungen (volles Signal mit Exponentialfenster statt 62,5 ms; Pole aus ESPRIT über alle sechs Stegkurven statt Spitzensuche). Befund: der freie Fit (Residuen je Kurve) trifft die Kurven, ist aber kein Satz von Moden (24–51 % der Moden ohne gemeinsames positives Vorzeichen an den Antriebspaaren); eingeschränkt auf Normalmoden mit einer Form längs der Stegeinlage trägt der starre Stab bis rund 1,2 kHz, **mit Biegung (φ(x) = a + b x + c x²) bis 5 kHz** (g36: 1,3 / 1,7 / 2,7 / 3,6 dB gegen frei 1,1 / 1,5 / 2,4 / 2,8 dB) — je Mode Frequenz, Dämpfung und der Wert an jeder der sechs Saiten; der Kipppunkt von KQ3 a greift nicht. Dieselben Moden erklären die Nahfeld-Mikrofone bis 400 Hz auf 1,2–2,7 dB (reflexionsarm), 400–1000 Hz auf 3,6–5,9 dB, darüber nicht (5–13 dB). Gegenprobe gegen Wright 1996 Tab. 5.4: wirksame Masse an Saite 6 über 85–240 Hz 0,053–0,062 kg an vier von fünf Kandidaten gegen 0,062 kg.

✅ **KQ23 a** (Patrik S199): **Vorbild der `classical_guitar` ist g34 des Archivs Mores — Manuel Contreras 1971** (Zeder / Rio-Palisander, 662 mm, 1670 g, im reflexionsarmen Raum gemessen), nachdem die dichten Polpaare an A0 und T1 geklärt sind. Grund: bester Stegfit über 1 kHz (2,5 / 2,3 dB gegen 3,3 / 3,6 dB bei g35), der Schall bis 400 Hz am besten erklärt (1,2–2,0 dB), Messung reflexionsarm. Geklärt: die Paare sind geteilte Pole — ein Pol je Paar trifft 85–240 Hz besser (0,82 statt 1,06 dB): A0 98,1 Hz / Q 24, T1 199,7 Hz / Q 12, wirksame Masse 0,054 kg. Vermerk im Körper: „Mensur 662 mm; Saiten nach Woodhouse 2012 für 650 mm“. Verworfen: b (g35, M. López Bellido 1978 — 652 mm und sauber getrennte tiefe Moden, aber Fit über 1 kHz schlechter, Hammer fällt bei 8 kHz), c (g36, Fichte — Stufe bei 2,8 kHz ungeklärt), d (g08, 2018 — halbhallig). Kippt auf c, wenn die `classical_guitar` ausdrücklich eine Fichtendecke haben soll; auf b, wenn die Mensur des Vorbilds in 34k-6 in die Rechnung eingeht. **Offen:** Abstrahlung oberhalb von rund 500 Hz und der Hörort (Nahfeld 10 cm); Ablage eines gemessenen Körpers ohne Bauplan; die zweite Richtung am Steg; Pole nach ESPRIT allgemein nachstellen.

✅ **KQ24 a** (Patrik S199; **ersetzt in KQ3 a „Abstrahlgewichte aus dem statistischen Teil über Plattenwerte“**): **Die Abstrahlung der Gitarre ist gemessen und geteilt** — bis zur Grenze trägt jede Mode ein komplexes Gewicht aus dem Fit an den Mikrofonen (ein Satz je Hörort für alle drei Schlagorte), darüber einen Betrag je Terzband aus dem gemessenen Pegel mit gezogenem Vorzeichen (Samen 1), wie `hoerBetrag` im statistischen Teil der anderen Körper. Hörorte: `left` = Mikrofon bass, `right` = Mikrofon treble, 10 cm über der Decke (Nahfeld, Vermerk). Die Grenze nach dem kleineren Fehler über beide Hörorte: **1 kHz** (1,62 dB gegen 1,84 dB bei 500 Hz). Grund: der Weg aus KQ3 a liefert ohne Bauplan nichts (Probe S195); ein Gewicht je Mode bis 5 kHz verfehlt den Terzpegel über 1 kHz um 2,7–5,9 dB (bis 10 dB), geteilt bleiben 1,4–2,6 dB. Verworfen: b (je Mode bis 5 kHz), c (Modell der Decke mit Ersatz-Bauplan). Nicht gerechnet: der Unterschied Nahfeld 10 cm gegen 1 m (das Archiv enthält die Fernfeld-Mikrofone nicht). Kippt auf eine Korrektur je Band oder auf c, wenn 34k-6 gegen die Aufnahmen eine Schieflage des Spektrums zeigt; auf c, wenn der Hörort wie bei den anderen Körpern in 1 m liegen soll.

✅ **KQ25 a** (Patrik S199): **Ein gemessener Körper ist eine `.lvinstbody` mit Herkunft „Messung“** — **`.lvinstbody` Fassung 4**: `origin` (`plan` / `measurement`) und `measurement` (`file`, `fingerprint`, `fitVersion`), beides immer geschrieben, Fassung 3 liest sich als `plan`. An der Stelle des Bauplans steht die Beschreibung `<name>.lvinstbodyfit` unter `asset/` (Datensatz mit DOI und SHA-256, Gitarre, Kanäle, Lage von Schlag und Aufnehmern, Einstellungen des Fits, Stegpunkte, Hörorte, Annahmen, Prüfzahlen); der Körper trägt ihren Abdruck (Bytes ohne Wagenrücklauf) und die Fassung des Fits (`kFitFassung`). Der Wächter prüft bei `measurement` Abdruck, Fit-Fassung, Stegpunkte und Hörorte gegen die Beschreibung. Grund: der gemessene Körper ist dieselbe Sache wie ein gerechneter; alles, was Körper liest, liest ihn. Verworfen: b (Stegmoden nur in der Tafel des Kerns), c (eigene Dateiart). Kippt auf b, wenn die Gitarre nicht in der Körper-Auswahl erscheinen soll.

**KQ23 a bis KQ25 a gebaut (S199)** — `asset/loop/instrument_bodies/classical_guitar.lvinstbodyfit` und `classical_guitar.lvinstbody` aus `make_gitarre.py <qualified_selected_impulses.mat>` (numpy, scipy; 3,5 min): 75 Moden bis 5 kHz (52 über 1 kHz), sechs Stegpunkte (Tasten 40 / 45 / 50 / 55 / 59 / 64), Hörorte `left` / `right`. Fehler über die sechs Stegkurven mit Normalmoden 1,01 / 1,77 / 2,45 / 2,31 dB (frei 0,91 / 1,39 / 1,98 / 1,55 dB); drei Paare unter 300 Hz zusammengelegt (A0 98,0 Hz / Q 23; T1 200,1 Hz / Q 13; 260,1 Hz / Q 29, dort 0,75 statt 0,67 dB); wirksame Masse an Saite 6 über 85–240 Hz 0,049 kg (Wright 0,062 kg); Terzpegel an den Hörorten auf 0,65–2,52 dB. Die sieben Körper mit Bauplan mit `InstrumentBodyTool --rewrite` auf Fassung 4 geschrieben und verglichen: gleich bis auf `version`, `origin`, `measurement`. **Offen:** die zweite Richtung am Steg, dichte Paare über 300 Hz, Nahfeld gegen 1 m, die Gitarre in der Körper-Auswahl der App ungeprüft.

**Befund über 5 kHz (S199)** — die Messung trägt bis rund 10 kHz (Hammerkraft −6,5 dB bei 8 kHz, −10,5 dB bei 10 kHz, −24 dB bei 16 kHz; Störabstand an den Mikrofonen 15–22 dB bei 8 kHz, 11–16 dB bei 10 kHz, darüber unter 10 dB); der gemessene Pegel von Schalldruck je Kraft bleibt von 2 bis 10 kHz flach. Vier Fassungen gegen die Messung (Terzpegel 6,3 / 8 / 10 kHz `left`; Abklingen α in 1/s gegen gemessen 211 / 231 / 247; Schwankung des Pegels 6–10 kHz gegen gemessen 5,5 dB): **A** nur die Ausläufer der 75 Moden −3,6 / −4,4 / −7,7 dB, kein Abklingen (ein glatter Sockel), 0,5 dB; **B** statistischer Teil nach `schaetze` wie bei den Körpern mit Bauplan +10,4 / +12,7 / +11,8 dB (Formwert und Abstrahlgewicht sind an gefitteten Polen gegenläufig, 2,6 dB; das angepasste Gesetz liegt unter der Dämpfung der Pole; die Gegenprobe im eigenen Band 2,5–5 kHz liegt 2–9,5 dB zu hoch); **C** statistischer Teil mit Größen an der Messung −0,4 / +1,2 / −0,8 dB, 211 / 218 / 240 1/s, 5,1 dB; **D** Modalfit bis 10 kHz: über 5 kHz 33 Pole, ihr Abstand wächst (133 → 167 Hz, Überlappung 1,2–1,4 — Ersatzpole, keine Moden), Fehler über die Stegkurven mit Normalmoden 5,7 / 6,0 dB, das Band 2,5–5 kHz wird schlechter (3,1 statt 2,3 dB). Nebenbefunde: die Aufteilung zwischen Formwert und Abstrahlgewicht ist über rund 2,5 kHz aus der Messung nicht bestimmt (Schlagorte und Aufnehmer an verschiedenen Stellen), nur das Produkt — für den Klang zählt nur das Produkt; im Band 3–4 kHz klingt der Körper schneller ab als gemessen (α 310 / 276 gegen 164 / 203 1/s).

✅ **KQ26 a** (Patrik S199): **Die Gitarre trägt über 5 kHz einen statistischen Teil bis 20 kHz, dessen Größen an der Messung bestimmt sind (Fassung C)** — Dämpfungsgesetz aus dem Abklingen der Mikrofone je Terzband 2,5–10 kHz, Betrag der Abstrahlung je Hörort aus dem Terzpegel 6,3–10 kHz, Dichte und Streuung am Steg aus den Polen 2,5–5 kHz, Samen 1; über 10 kHz fortgesetzt, nicht geprüft (Vermerk in der Beschreibung). Das Format der Körperdatei bleibt; `kFitFassung` 2. Verworfen: b (Modalfit bis 10 kHz), c (bei 5 kHz enden), d (`schaetze` wie bei den Körpern mit Bauplan). Kipppunkte: fallen in 34k-6 die Töne über 5 kHz gegen die Aufnahmen zu hell oder zu rau aus, zuerst die Dichte prüfen (mit dreifacher Dichte bleiben Pegel und Abklingen, die Schwankung 5,2 / 6,0 dB).

✅ **KQ27 a** (Patrik S199): **Die Form, in der der Körper in den Kern kommt, leitet 34k-6 her; das Werkzeug dazu baut 34k-7.** Zeile 34k-5 verliert den Satz zum Stegmoden-Werkzeug und gilt mit den fünf Körpern als fertig. 34k-6 rechnet am Prüfstand die Formen gegeneinander — alle Moden in der Saitenschleife; Moden bis zu einer Grenze in der Schleife und der Rest gefaltet; nur Stegkraft aus dem Kern und Faltung wie beim Flügel — gegen Lautheit, Sprünge, Ausklang und Rechenzeit und legt sie als GZ5 vor. Grund: das Werkzeug legt ein Datenformat fest, bevor feststeht, was der Kern braucht; die Körper tragen weit mehr Moden als die Liste der Vorlage (Gitarre 236, Cembalo allein im Band 2,5–5 kHz 243, die Vorlage 5–10 je Instrument). Verworfen: b (das Werkzeug jetzt nach Wortlaut, alle Moden als `StegMode` in die Tafel). Kippt nachträglich auf denselben Bau, wenn die Herleitung die Liste in der Tafel mit wenigen Moden als richtige Form ergibt.

✅ **GZ6 a** (Patrik S199): **An der `classical_guitar` setzt die Stufe die Auslenkung und kippt das Spektrum des Zupfs** — Auslenkung 4 mm · 10^(Stufe/20) bei 16 cm vom Steg (Pavlidou 1997 nach Scherrer 2013 Tab. 2.1: 0,8 mm „soft“, 4 mm „strong“); dazu eine Neigung von 0,16 dB je Oktave je dB Stufe um 1 kHz, Belegkraft „angepasst an Iowa MIS, eine Gitarre, von Hand gespielt“. Der Kern rechnet weiter je Stufe einen eigenen Ton. Grundlage (`../modellierung/gezupfte-saite/proben/2026-11-15_S199_dynamik_je_teilton.txt`, 66 Töne in pp / mf / ff, 748 Teiltöne): ff − pp im Mittel 18,7 dB, Neigung 3,0 dB je Oktave (ff − mf 0,8); Rest gegen die Messung 7,80 dB mit Neigung, 8,63 dB nur mit Auslenkung, 8,33 dB mit Fingerbreite je Stärke (6,3 mm), bei 13 dB Streuung. Verworfen: b (nur die Auslenkung — jede Stärke klingt gleich), c (Fingerbreite je Stärke — die Messung trägt sie nicht). Kippt auf eine vierte Fassung, wenn Scherrer 2013 Kap. 3 ein Gesetz für die Ablösung je Stärke hergibt (vor dem Bau lesen); auf b, wenn die Neigung an einer zweiten Aufnahme nicht wiederkehrt.

✅ **GZ5 a** (Patrik S199): **In der Saitenschleife stehen alle Moden der Körperdatei (bis 5 kHz, ohne den statistischen Teil); der Kern gibt die Kraft am Steg senkrecht zur Decke aus, die App faltet sie mit der Antwort des ganzen Körpers an beiden Hörorten wie beim Flügel.** Grundlage (`../modellierung/gezupfte-saite/proben/2026-11-15_S199_steg_formen.txt`, Gitarre, 22 Tasten, gegen alle 236 Moden in der Schleife; größter Betrag in Lautheit / Pegel bei 1–2 s): Moden der Datei (75) 0,00 LU / 0,34 dB; bis 2 kHz (41) 1,0 LU / 1,0 dB; bis 1 kHz (23) 1,1 LU / 2,4 dB; bis 500 Hz (8) 1,5 LU / 10,5 dB; starr 7,2 LU / 22,4 dB; der Rest der weggelassenen Moden als Feder bringt wenig; Abstrahlung nur aus der Kraft senkrecht zur Decke höchstens 0,6 LU daneben (Median 0,07). Moden bis 5 / 2 / 1 kHz je Körper: Gitarre 75 / 41 / 23, Laute 128 / 34 / 13, Oud 243 / 73 / 28, Harfe 145 / 36 / 15, Cembalo 389 / 106 / 38. **Berichtigt GZ4 a** in einem Satz: die Faltung nach dem Kern ersetzt den Steg nicht, sie trägt aber die Abstrahlung. Verworfen: b (nur bis 2 kHz), c (nur bis 1 kHz), d (alle Moden samt statistischem Teil in der Schleife, Abstrahlung im Kern). Nicht gemessen: Rechenzeit; die anderen vier Instrumente. Kippt je Instrument auf b, wenn der Zeitbereichs-Kandidat mit allen Moden die 4,6 % nicht hält.

✅ **GZ5-1 a** (Patrik S199; **ergänzt GZ5 a**): **Über 5 kHz steht in der Saitenschleife ein Dämpfer aus den Größen des statistischen Teils** — der mittlere Wirkanteil der dichten Moden am Stegpunkt, Re Y = n(f) · φ_rms² / 4 (n Moden je Hz, φ_rms die Streuung am Stegpunkt, beides aus der Körperdatei; die App reicht sie mit dem Tonauftrag, GZ7 a). Gilt für alle fünf Instrumente. Grundlage (`../modellierung/gezupfte-saite/proben/2026-11-15_S199_steg_formen_cembalo.txt`, `…_daempfer.txt`; Cembalo, jede fünfte Taste, größter Betrag in Lautheit / Pegel bei 1–2 s gegen alle 3460 Moden): Moden der Datei (389) 2,2 LU / 2,8 dB (an jeder Taste bis 3,4 LU); dazu die gezogenen bis 7,5 kHz (696) 2,1 / 1,7; bis 10 kHz (1097) 1,5 / 0,7; bis 15 kHz (2141) 0,9 / 0,9; Moden der Datei und der Dämpfer 0,45 LU / 1,1 dB. Die Abweichung sitzt ab etwa C5 in Teiltönen über 5 kHz, die ohne den Körper am Steg fast ungedämpft klingen. Verworfen: b (am Cembalo die gezogenen Moden bis 10 kHz), c (GZ5 a unverändert, Abweichung als bekannt), d (alle Moden). Kippt auf b, wenn sich der Dämpfer im Zeitbereich nicht passiv und einfach bauen lässt oder an einem der anderen Instrumente die Rechnung verschlechtert; auf c, wenn belegte Drahtverluste die Abweichung unter 1 LU drücken.

✅ **GZ7 a** (Patrik S199; **ersetzt in GZ4 a den Satz zum Werkzeug und in KQ27 a den Bau des Werkzeugs**): **Die App reicht dem Kern die Moden des Körpers mit dem Tonauftrag** — die Schicht nennt ihren Körper wie beim Flügel (`koerperAnDerSchicht`), die App lädt ihn einmal und gibt je Note Frequenz, Verlustfaktor und Wert am Stegpunkt der gespielten und der mitschwingenden Saiten; kein Werkzeug, keine Kopie in der Tafel, der Kern bindet `InstrumentBody` weiter nicht ein; der Tonname trägt den Abdruck des Körpers. Grund: Rückwirkung und Abstrahlung kommen aus derselben geladenen Datei, es gibt keine zweite Fassung der Zahlen, die veralten kann (nach GZ5 a wären es in der Tafel rund 11 600 Zahlen an der Harfe, 23 700 am Cembalo). Verworfen: b (Werkzeug schreibt die Stegmoden in die Tafel), c (der Kern liest die Körperdatei selbst — ein Rechenkern lädt Dateien). Kippt auf b, wenn ein Kern-Instrument ohne Körperdatei spielbar bleiben soll oder der Tonauftrag die Daten nicht tragen kann, ohne die anderen Kerne zu berühren. Stimmer und Messfälle bekommen die Moden über denselben Auftrag.

✅ **GZ8 a** (Patrik S200; **ersetzt in GZ3 a die Rechenzeit-Grenze für das Mitschwingen**): **Die Harfe schwingt mit Oktaven, Quinten und Quarten der gespielten Saite mit (20 Saiten). Die Grenze 4,6 % gilt für eine Saite ohne Mitschwingen; mit Mitschwingen wird die Rechenzeit je Instrument am gebauten Kern gemessen und im Bericht geführt, ohne Grenze.** Laute, Oud und Gitarre nehmen die übrigen Saiten des Satzes, das Cembalo seine Register; der Kern rechnet in doppelter Genauigkeit. Grund: die Auswahl hält die beiden Klanggrenzen aus GZ3 a (größter Betrag gegen alle 46 über zehn Tasten: 0,43 LU, Nachklang 2,8 dB), und keine Auswahl, die den Nachklang trägt, hält 4,6 % (Probe S200, doppelte Genauigkeit, Durchgriff je Stegpunkt: 20 Saiten 20 %, 33 Saiten 33 %, 46 Saiten 47 % der Echtzeit; Laute 7,1 %, Oud 8,7 %, Cembalo mit drei Registern 6,0 %, Gitarre 3,6 % — ein Anhalt bei ungetauschter Kühlung). Eine Saite allein mit allen Moden der Körperdatei und dem Dämpfer hält die Grenze (Gitarre 0,8 %, Laute 1,3 %, Harfe 1,4 %, Oud 2,2 %, Cembalo 3,5 %) — der Kipppunkt in GZ5 a greift nicht. Verworfen: b (dazu Terzen und Sexten, 33 Saiten: 0,36 LU / 0,9 dB für das 1,6-Fache), c (alle 46 ohne Regel: mehr als das Doppelte für höchstens 0,43 LU und 2,8 dB, an C7 rund 40 dB unter dem Anschlag), d (die Grenze bleibt, Harfe, Laute und Oud ohne Mitschwingen: die Harfe verfehlt dann 3,9 LU und 15,4 dB). Gerechnet und verworfen: Oktaven allein (1,8 LU / 6,9 dB), die Auswahl nur im Umkreis von zwei Oktaven (1,2 LU / 7,1 dB), Kopplung nur bis 2 kHz (0,43 LU / 4,2 dB), Rückwirkung nur der gespielten Saite (14,3 LU / 18,4 dB). Kippt auf b, wenn die 2,8 dB an C7 am gebauten Kern über 3 dB liegen; steht unter Vorbehalt, bis die Rechenzeit am gebauten Kern gemessen ist. Nicht am Ton geprüft: der Durchgriff je Stegpunkt (die Kopplung zwischen den Punkten einen Takt später).

✅ **GZ9 a** (Patrik S200; **Haltepunkt aus GZ1-1 a**): **Der Haltepunkt ist bestanden; Bend, Slide und Legato bleiben in 34k.** Die ±10 Cent gelten für gehaltene Töne nach dem Stimmer und für das Ziel eines Bend oder Slide, nicht für den Durchgang durch eine Körpermode; die Tonhöhe wird aus den Teiltönen gemessen, nicht am Grundton allein. Dass das Modell den Grundton nahe A0 und T1 rund 1,7-mal so stark verstimmt wie die Iowa-Gitarre, wird in 34k-7 an der gemessenen Stegadmittanz von g34 geprüft. Grund: die Gegenprobe am starren Steg (gehalten ±0,1 Cent, am Ziel 0,1 Cent; Saite 1 mit Körper im Lauf höchstens 2,5 Cent) — was abweicht, ist die Kopplung von Saite und Körper (G2 auf Saite 6 gehalten −10 Cent, Slide auf Saite 4 durch 200 Hz im Durchgang 12–16 Cent, am Ziel 3,2 Cent), und die Iowa-Gitarre zeigt sie auch (Grundton gegen die Teiltöne 2–4: bis 12,7 Cent um 100 Hz, 10–17 Cent um 200 Hz; Modell 22,6 und 34,3 Cent). Verworfen: b (der Kipppunkt greift, Bend, Slide und Legato eigene Stufe — die Verstimmung betrifft gehaltene Töne genauso). Kippt, wenn die Prüfung in 34k-7 die Stegadmittanz von g34 bei 98 und 200 Hz bestätigt und die Verstimmung des Grundtons so nicht ausgeliefert werden soll — dann ist es eine Frage an den Körper der Gitarre.

GZ8 a und GZ9 a kamen auf ein einzelnes „a“ zu zwei Fragen; so gelesen.

✅ **GZ10 a** (Patrik S200): **Das Cembalo bekommt einen Tastenstoß: ein Kraftstoß senkrecht auf den Boden am Stegpunkt der Taste, Gauß mit 0,4 ms, 7 mN s bei ff, 10 dB weniger je Stufe (ff, mf, pp = Stufe 0, −9, −18 dB). Die Ablösekraft des Kiels beträgt 2 N bei ff und fällt bis pp um 1 dB.** Belegkraft: Stoßstärke angepasst an Penttinens Hörtest und die VCSL-Aufnahmen, Gesetz je Stufe nach Penttinen 2006, Ort und Form Annahme. Grund: Penttinens Spektren (Abb. 4 / 5) eichen den Stoß nicht (das Fenster fehlt; je nach Fenster 0–39 mN s an C3, 23–187 mN s an C6); mit der Saite bei 1 dB gibt 7 mN s pp → ff 2,3 / 1,8 / 1,7 / 1,3 / 1,1 LU an C2 … C6 gegen rund 1,3 / 2,4 / 2,2 / 1,4 / 1,3 dB im Hörtest (Abb. 6 a, grob gelesen — vor dem Bau am vergrößerten Bild nachlesen) und liegt in der Spanne der Aufnahmen (2,5–10 mN s; Band 30–300 Hz in den ersten 0,2 s). Verworfen: b (10 mN s: C2 um 2 dB überzeichnet), c (5 mN s: C3 / C4 0,9 dB zu wenig), d (kein Stoß: C3 / C4 1,3 dB unter dem Hörtest, das Merkmal lauter Töne fehlt). Kippt auf d, wenn der Stoß in der Hörprobe als Fremdkörper klingt; neu zu stellen, wenn eine Quelle den Weg von der Taste in den Boden belegt.

✅ **GZ11 a** (Patrik S200): **Der Lautenzug ist ein Dämpfer am Sattel: 40 N s/m im Abstand 0,5 mm, für alle Tasten gleich.** Belegkraft: angepasst an das englische Cembalo der VCSL-Sammlung (*Lute* gegen *Normal*, 27 Tasten). Grund: er trifft alle drei gemessenen Größen zugleich und ihren Gang über die Tasten — Lautheit im Median −1,8 LU (C2 −0,9 … C6 −3,4; Aufnahmen −1,8, Bass −0,4 … C6 −4,9), früher Abfall 2,2-fach (1,2–3,8; Aufnahmen 2,3, je Oktave 1,9–3,2), Grundton +0,0 … +4,2 Cent (Aufnahmen Median +1,6, C6 +5,2). Ein schwacher Dämpfer (R ≤ 3 Z0) nimmt die Höhen, nicht den Grundton, und trifft Lautheit und Abfall nie zugleich. Verworfen: b (80 N s/m bei 1 mm: −0,9 LU, bis +6,9 Cent), c (Güte durch acht wie die Vorlage: an der Vorlage 4,4 LU leiser, keine Verstimmung). Kippt, wenn der Dämpfer so dicht am Sattel im Zeitschritt nicht stabil zu bauen ist — dann ein gleichwertiger Abschluss mit denselben drei Zielgrößen.

✅ **GZ12 a** (Patrik S200; **klärt den Satz „die Register stehen fest in der Tafel“**): **34k liefert das `harpsichord` mit einem 8′ je Taste. Zweites 8′ und 4′ kommen mit den Registern als Spur in 31g**; dafür festgehalten: Stegpunkte auf dem 4′-Steg im Körper (das Format um den Steg je Punkt erweitern), Ablösekraft und Zupfpunkt des 4′ an den Aufnahmen bestimmen. Grund: ein Register ist, was die Zielgrößen und alle Aufnahmen messen, und Register sind ein Spielmittel — fest eingebaut wäre das einzelne 8′ nicht mehr spielbar. Gerechnet (S200, Probe-Körper mit beiden Stegen): zweites 8′ ±0,1 LU gegen das erste, beide 8′ +3,9 LU, das 4′ mit 2 N rund 5–8 LU zu laut gegen die Aufnahmen, alle drei +6,9 LU und 9,3 % der Echtzeit. Verworfen: b (beide 8′ fest), c (alle drei fest mit Nachtrag zu 34k-5). Kippt auf c als schaltbares Feld in der Tafel, wenn 31g in absehbarer Zeit nicht gebaut wird.

GZ10 a, GZ11 a und GZ12 a auf „alle drei Fragen a“.

✅ **GZ13 a** (Patrik S200; **ergänzt GZ10 a**): **Die Ablösekraft des Kiels fällt über den Umfang um 2,8 dB je Oktave: 2 N an C4 bei ff, 4,9 N an F1, 1,0 N an f‴.** Belegkraft: angepasst an die Form der Lautheit von fünf Cembali der VCSL-Sammlung; der Maßstab (2 N an C4) ist Annahme. Grund: die gemessenen Cembali sind über den Umfang eben (je Oktave −1,0 … +0,9 LU), das Modell mit fester Kraft steigt um 15 LU; mit dem Gesetz Spanne 6,8 statt 18,7 LU, 10.–90. Perzentil 4,3 statt 13,0, Abstand zur Form der Aufnahmen 1,7 statt 4,5 LU (Aufnahmen: Spanne 6,1–13,6). Der Bauer schneidet die Kiele nach Gehör auf gleichmäßige Lautstärke: die Kraft je Taste ist am Instrument die Stellgröße. Eine Quelle für die Kraft je Taste gibt es nicht (Roy 2015, Chadefaux 2013 durchsucht; Chadefaux misst 0,8 N an G4). GZ10 a bleibt damit stimmig (Abstand zum Hörtest 1,4 statt 1,8). Verworfen: b (Parabel: 0,1 LU besser, ein Wert mehr), c (feste 2 N: 15 LU Gang bleiben). Nicht getrennt geprüft: ob der Gang am Hörort (ein Punkt 1 m über dem Boden) oder am Körper (Dämpfung für alle Frequenzen gleich) liegt — kippt, wenn sich das zeigt; 4,9 N im Bass liegen am oberen Rand dessen, was Perngs Modell nahelegt (1,6–2,8 N).

✅ **GZ14 a** (Patrik S200; **ersetzt in GZ2 a die Schwellen für Lautheit und Sprünge**): **Gemessen an jeder Taste, Sprünge als Paare bis 4 Halbtöne Abstand: Spanne ≤ 15 LU, an der Harfe ab D2 (darunter folgt sie dem Instrument); mittlerer Sprung ≤ 4 LU; größter Sprung ≤ 12 LU.** Die Schwellen stammen von den Aufnahmen aus S195 (mittlerer Sprung dort höchstens 3,9 LU, größter an den Cembali höchstens 11,3 LU; die Konzertharfe 17,7 LU über den vollen Umfang, 10,8 LU ab D2). Stand der Modelle (S200; Spanne / größter / mittlerer Sprung): Laute 13,9 / 11,8 / 2,4 und Cembalo mit GZ13 a 6,8 / 6,0 / 1,5 halten; Gitarre 13,8 / 13,8 / 2,0 verfehlt den größten Sprung (Tasten 78–83, Ursache offen); Oud 15,8 / 11,2 / 2,6 die Spanne (der einzelne Basschor); Harfe 23,3 / 13,5 / 3,0 Spanne und größten Sprung (Mode 208 Hz unter G3–A3; ohne die drei Tasten 17,0 / 10,4 / 2,8). Verworfen: b (ohne Schwelle für den größten Sprung — der Fehler der Harfe ginge durch), c (15 LU / 6 LU wie gesetzt — kein echtes Instrument hält die 6 LU). Kippt auf b, wenn die 13,8 LU der Gitarre eine Eigenschaft von g34 sind und kein Fehler.

GZ13 a und GZ14 a auf „alle a“.

✅ **KQ28 a** (Patrik S201; **ergänzt KQ23 a–KQ25 a im Weg des Fits**): **Die Form je Mode längs des Stegs der Gitarre wird an den sechs Stegkurven und den neun Schallkurven gemeinsam gestellt** (drei Schlagorte, drei Mikrofone; beide Kurvenarten mit gleichem Gewicht, abwechselnd Gewichte je Mikrofon und Form, acht Runden; Start ist der Fit an den Stegkurven allein). `make_gitarre.py` Schritt 4, `FIT_FASSUNG` = `kFitFassung` = 3; Schritt 5 (Grenze 1 kHz, KQ24 a) und Schritt 6 bleiben. Anlass: die Körperdatei lag am Diskantende in 780–880 Hz 9–11 dB unter der Messung, weil die zwei Aufnehmer an den Enden der Stegeinlage die Form zwischen 500 und 1000 Hz nicht festlegen — fünf Moden (575,6 / 606,4 / 756,6 / 809,2 / 916,0 Hz) widersprachen den Mikrofonen; die Pole reichen. Zahlen vorher → nachher: größter Sprung der Lautheit über Paare bis 4 Halbtöne 14,3 → 11,9 LU (Schwelle 12 LU, GZ14 a; jetzt an den Tasten 68 / 71), Spanne 14,4 → 11,9 LU, mittlerer Sprung 2,0 → 1,6 LU; größter Fehler im Halbtonband −8,6 / −10,7 → −5,8 / −8,7 dB (`left` / `right`); Terzpegel 500–1000 Hz 2,14 / 2,10 → 1,67 / 1,69 dB, 1250–2500 Hz 1,45 / 1,87 → 1,87 / 2,26 dB; Stegkurven 1,01 / 1,77 / 2,45 / 2,31 → 1,34 / 2,20 / 2,77 / 2,71 dB; wirksame Masse an Saite 6 0,049 → 0,052 kg (Wright 0,062 kg). Verworfen: **b** (Schritt 4 wie bisher, der Sprung als bekannte Abweichung), **c** (Schallkurven mit dreifachem Gewicht: Stegkurven über 1 kHz 0,8–1,2 dB schlechter, der Sprung fällt nicht weiter), **d** (Gewicht 0,3: der Sprung bleibt bei 13,1 LU). Bericht [../modellierung/gezupfte-saite/2026-11-16_S201_gitarre_senke_780_880.md](../modellierung/gezupfte-saite/2026-11-16_S201_gitarre_senke_780_880.md). Offen daraus: der Sprung an den Tasten 68 / 71; über 1 kHz stimmen Form und Mikrofone bei einem Drittel bis der Hälfte der Moden nicht überein (dort trägt die Grenze nach KQ24 a).

✅ **KQ29 a** (Patrik S201; **ändert in KQ17 a–KQ21 a die Güte der Luft**): **Die Luft im Kasten der Harfe bekommt die Güte 5,5 statt 16** (`cavity.q` in `make_harfe.py`), so gestellt, dass S(1,1)− / S(1,1)+ die Güte 13 bekommen — Bell 1988 misst am Kasten G in der Harfe ohne Saiten 11 und 16 (Abb. 7.8, 7.9); der Wert 16 stammte vom losen Kasten F (Tab. 6.1: 18, 14) und gab den Moden 192 / 208 Hz die Güte 27. Der Wert ist angepasst und steht auch für Verluste im Rahmen der Harfe. Körper neu gerechnet (96 s): nur die Verlustfaktoren ändern sich (129 / 192 / 208 Hz Q 25 / 27 / 27 → 13 / 13 / 13, S(1,2) 41 → 35, gemessen 39). Zahlen vorher → nachher: größter Sprung über Paare bis 4 Halbtöne 13,5 → 11,0 LU (Schwelle 12 LU), Spanne ab D2 23,3 → 19,6 LU (Schwelle 15 LU — **bleibt verfehlt**, sie hängt jetzt am Diskantende, Tasten 92–94), mittlerer Sprung 2,8 → 2,9 LU, Taste 55 gegen das Mittel von 52 und 59 +9,4 → +6,5 LU (Aufnahme KSHarp +4,5 LU). Die laute Stelle um 200 Hz bleibt und ist belegt (Le Carrou, Gautier & Foltête 2007 Abb. 7: Druck je Kraft im Fernfeld bei 204 Hz +16,6 dB gegen 240–250 Hz). Verworfen: **b** (Güte 16 lassen, die Tasten 55–57 als bekannte Abweichung). Bericht [../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md](../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md) §14. Offen daraus: das Diskantende; der Widerspruch zwischen Bells besaiteter Harfe (Abb. 7.22) und Le Carrous (Auftrag `2026-11-16_harfe_besaitet_admittanz` der Research-Werkbank).

✅ **KQ30 a** (Patrik S203; **ändert in GZ13 a den Maßstab und in GZ10 a die Stoßstärke**): **Die Ablösekraft des Kiels beträgt 1,07 N an C4 bei ff (2,6 N an F1, 0,54 N an f‴; die Form −2,8 dB je Oktave aus GZ13 a bleibt), der Tastenstoß 3,8 mN s bei ff (statt 7; die Spanne der Aufnahmen wird 1,3–5,4 statt 2,5–10 mN s).** Belegkraft: Quelle für den Maßstab — Chadefaux, Le Carrou, Le Conte & Castellengo 2013 (SMAC) Tab. 1/2 S. 158 am Seitenbild: G4, schneller Anschlag, Auslenkung beim Ablösen 0,90 ± 0,01 mm senkrecht und 0,28 ± 0,02 mm waagerecht, T 85,7 N, L 47,5 cm, K = T / (L β (1 − β)) mit β = 0,259 aus Taskin 1769 (in der Quelle nicht genannt) → 0,89 N, mit der Form von GZ13 a an C4 1,07 N; die Form bleibt angepasst. Grund: die 2 N stammten aus Perngs Modell (1,6–2,8 N) und lagen über der starren Klinge bei Giordano & Winans 1999 (Abb. 9, ≈ 1,6 N); gemessen sind 0,89 / 0,71 N an G4 (schnell / langsam) und an C4 0,57–0,66 N Gewicht an der Taste (Di Veroli 2012). Der Prüfstand ist in der Kraft linear (`2026-11-16_S200_cembalo_register.py` Z. 78, 99, 157–159, 179): mit dem Stoß im selben Verhältnis bleiben Lautheit über den Umfang, Anteil des Stoßes pp → ff und Band 30–300 Hz gegen die Aufnahmen Bit für Bit gleich — die Änderung macht die Zahlen belegt, nicht den Klang anders. Verworfen: b (0,66 N an C4 nach Di Veroli, Stoß 2,3 mN s: die Taste statisch über einen Hebel unbekannter Übersetzung gewogen, nicht die Kraft am Kiel), c (2 N lassen: eine Modellzahl über der Grenze der starren Klinge). Kippt, wenn β an Chadefaux' Instrument deutlich kleiner ist (β = 0,15 gäbe 1,6 N an C4). Nebenbefund: Chadefaux misst langsam gegen schnell +2,1 dB, GZ10 a setzt pp gegen ff 1 dB an — „langsam“ ist nicht sicher pp, GZ10 a bleibt. Für 34k-7: der Kern trägt heute 1,0 N bei ff an jeder Taste (`src/zupf/Vorgaben.cpp:76–77`); Belege der Nacharbeit in [../modellierung/gezupfte-saite/recherche/2026-11-17_S203_cembalo_nacharbeit_werkbank.md](../modellierung/gezupfte-saite/recherche/2026-11-17_S203_cembalo_nacharbeit_werkbank.md).

✅ **KQ31 a** (Patrik S203): **Die Admittanz der Harfe ohne Saiten über 200 Hz wird bis zur Ursache untersucht — ein eigener Schritt in 34k-6 vor 34k-7, mit Fable in eigener Sitzung (Merkmal 3); Ziel höchstens 3 dB Abstand je Terzband über 200–800 Hz gegen Bell 1988 (Ort 17 aus Abb. 7.6, Täler der Profile 7.8–7.17, Karte 7.3), danach der Körper neu und Spanne und Sprünge der Harfe neu gemessen.** Befund (S203, `../modellierung/gezupfte-saite/proben/2026-11-17_S203_harfe_ort17_ohne_saiten.py` und `…_harfe_nachgiebigkeit_oesen.py`): an Ort 17 ohne Saiten (Oesen der Saiten 20 / 21) Modell gegen Bell −1,5 dB (140–200 Hz), −6,3 dB (200–400 Hz), −10,0 dB (400–790 Hz), Steigung 200–790 Hz −6,3 gegen −2,2 dB/Oktave; die Spitzen der Moden trifft das Modell (S198). Nicht die Ursache: die abgeschnittene Modensumme (die Nachgiebigkeit Σ φ²/ω² konvergiert, Moden über 8 kHz 1–2 %). Gegen Abb. 7.22 (besaitet, Bezug 116 dB = 1 s/kg nach Bell S. 149 belegt) liegt das Modell an den Ösen im Median 23 dB darunter; das Besaiten hebt bei Bell die Täler über 380 Hz um rund 9,5 dB (Bericht 22), das Modell bildet das nicht ab — Frage nach diesem Schritt. Verworfen: b (Abweichung festhalten, nur an der Lautheit je Taste messen: glicht eine bekannte Ursache an der falschen Stelle aus). Kippt, wenn Bells Ort 17 nicht auf der Mittellinie liegt (Abb. 6.3 erneut lesen). Dazu ohne Entscheid: KQ29 a bleibt (Bells Helmholtz-Güte mit allen Öffnungen offen Q 6 bei 190 Hz am starren Kasten stützt 5,5); in Proben mit Saitenlast gilt der Querwiderstand cos² 30° = 0,75 (Le Carrou 2006 Gl. 6.12–6.15) statt 0,25 — an den Ösen ≤ 1,5 dB.

**Befund zu KQ31 a (S205, Bericht [../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md](../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md) §15, Proben `../modellierung/gezupfte-saite/proben/2026-11-18_S205_harfe_*`):** zwei Ursachen. **(1) Im Bauplan fehlt die Zugleiste in der Rechnung:** Deck- und Zugleiste liegen auf derselben Linie, `steifeBei` nimmt an einem Punkt nur eine Leiste (der Steg geht vor) — gerechnet ist Platte + 5 mm statt Platte + 5 + 20–25 mm Buche (Masse 1,764 statt 2,179 kg). Mit beiden Leisten: Lage S(1,2)–S(1,6) −6,4 / +4,0 / +0,3 / +1,6 / −1,5 % (mitgeliefert +10,4 / +9,7 / −0,3 / −3,3 / −9,6 %), Zahl und Lage aller Bäuche wie bei Bell (mitgeliefert im Bassdrittel je Mode ein Bauch zu viel); die Höhen am stärksten Bauch −4,2 / −8,5 / −9,4 / −6,8 / −8,1 dB (mitgeliefert +1,6 / −4,6 / −5,5 / −2,6 dB, mit einer um 0,4 kg zu leichten Mitte) — Ursache offen, Bells Pegel liegen rund 6 dB über einem Überschlag aus seinen eigenen Maßen. **(2) Bells Messung trägt einen federartigen Untergrund:** an den Orten 37–40 (Bodenblock) des losen Kastens steigt der Pegel mit 6 dB je Oktave, aus sechs Profilen folgt dieselbe Feder 2,6 · 10⁵ N/m (±0,3); mit ihr in Reihe liegt das Modell an Ort 17 über 200–790 Hz im Median +1,2 dB (ohne −17,2 dB), je Terzband −0,4 / −6,1 / −7,2 / +1,3 / +0,1 / +1,4 / +3,5 dB — es bleiben S(1,2) und die Lage eines Knotens von S(1,3) an Ort 17. Herkunft der Feder nicht belegt (Stift auf Holz: eigene Abschätzung 2–4 · 10⁵ N/m); für den Klang zählt sie nicht (Blindanteil). Prüfstand je Taste mit der Zugleiste: Spanne ab D2 21,7 LU, größter Sprung 14,6 LU (78 / 80; Schwelle 12), mittlerer 3,0 LU; Güte der Luft (KQ29 a) und Diskantkante (KQ21 a) sind am neuen Bauplan nicht nachgestellt. Der Körper im Repo ist unverändert. Entschieden mit KQ31-1 a und KQ31-2 a.

✅ **KQ31-1 a** (Patrik S205; **ändert das Format des Bauplans**): **Der Bauplan sagt, wie Leisten übereinander zählen — ein Feld mit den Werten „nur die erste“ (wie bisher: der Steg geht vor) und „Höhen addieren“, `.lvinstbodyplan` Fassung 8, immer geschrieben; die Harfe bekommt „addieren“, alle übrigen Körper bleiben bei „nur die erste“ und werden nur für den Abdruck neu gerechnet; `InstrumentBodyTool` meldet, wenn eine Leiste eine andere verdeckt.** Grund: der Bauplan bleibt die Zeichnung, der Kern behandelt die Überlagerung ausdrücklich. Für die Harfe liefern alle drei Wege dasselbe (zwei Probe-Baupläne auf 1 Hz und 0,3 dB gleich). Verworfen: b (der Kern addiert immer: am Steinway D lägen 157 von 988 cm² Stegfläche über Rippen, 278 cm³ kämen dazu; RT-0425 §2.1 nennt vier Zonen, gibt die Dicke nur für drei an, für den Steg 29–69 mm — die Wirkung auf den Flügel ist nicht gerechnet), c (nur der Bauplan der Harfe mit einer Deckleiste von 25 mm, die es nicht gibt; die Falle bliebe). Kippt zu b, wenn Chabassiers Netz an den Kreuzungen die Summe trägt. Bau: mit dem neuen Körper der Harfe (Güte der Luft und Diskantkante nachstellen, KQ34 a neu messen).

✅ **KQ31-2 a** (Patrik S205; **ersetzt das Ziel von KQ31 a und in KQ19 a die Schwelle der Höhe**): **Die Harfe wird ohne Saiten an den Moden S(1,2)–S(1,6) gemessen: Lage der Frequenz innerhalb 7 %, Zahl der Bäuche gleich, Lage jedes Bauchs innerhalb zwei Messorten, Höhe der übrigen Bäuche gegen den stärksten innerhalb 6 dB; die Höhe des stärksten Bauchs ist bekannte Abweichung mit Vermerk (heute −4 … −9 dB), bis eine Ursache gefunden ist; die Kurve an Ort 17 wird mit der Feder 2,6 · 10⁵ N/m in Reihe berichtet, ohne Schwelle. Das Terzband-Ziel entfällt.** Grund: an der Resonanz trägt die Feder nichts bei (bei 432 Hz 30 dB unter der Spitze) und Bells Bilder stimmen dort auf 1 dB überein; abseits weichen sie bis 7 dB voneinander ab, und an Ort 17 machen 7 cm in der Lage eines Knotens 20 dB. Verworfen: b (Terzband-Ziel mit der Feder: 250 Hz −6,1 dB, 315 Hz −7,2 dB; hängt an einer Feder ohne belegte Herkunft), c (Höhe des stärksten Bauchs bleibt Schwelle: Bells Pegel liegen rund 6 dB über einem Überschlag aus seinen eigenen Maßen). Kippt zu c, wenn eine zweite Messung einer Konzertharfe ohne Saiten Bells Höhen bestätigt; zu b, wenn die Feder aus Bells Messaufbau belegt wird.

**Gebaut (S206):** KQ31-1 a — `.lvinstbodyplan` Fassung 8 mit `barOverlap` (`first` / `add`), `InstrumentBody` 0.23.0, die Harfe mit `add`, alle sieben Körper mit Bauplan neu (die sechs übrigen gleich bis auf Abdruck und Rechenzeit); bei `add` gelten Werkstoff und Faserrichtung der ersten Leiste. Diskantkante (KQ21 a) nachgestellt: 3 · 10⁶ N/m² bleibt. Messung nach KQ31-2 a und Zahlen in [../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md](../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md) §16.

✅ **KQ29-1 a** (Patrik S206; **ersetzt den Wert von KQ29 a**): **Die Güte der Luft der Harfe ist 8,5 — der Wert mit der kleinsten gemeinsamen Abweichung von Bells zwei Güten im Logarithmus.** Mit der Zugleiste teilen sich die zwei Moden die Luft ungleich (134 Hz: 0,28; 198 Hz: 0,67), ein Wert trifft nicht beide (Bell 170 Hz Q 11, 216 Hz Q 16; Modell 18,2 / 11,5). Am Prüfstand je Taste ab D2: Spanne 21,9 LU, größter Sprung 14,5 LU (78 / 80), mittlerer 3,3 LU — der größte und der mittlere Sprung liegen über KQ34 a und sind offen. Verworfen: b (5,5 bleibt: hält den mittleren Sprung bei 3,0 LU, die Mode mit dem größten Luftanteil wäre doppelt so stark gedämpft wie gemessen — eine Dämpfung, die auf die Kennzahl hin gestellt ist, gliche einen offenen Fehler mit einem anderen aus), c (12,4: trifft 198 Hz genau, 134 Hz klingt doppelt so lang wie gemessen, alle drei Kennzahlen über der Schwelle). Kippt zu c, wenn die 134-Hz-Mode nicht Bells 170-Hz-Mode ist und nicht mitzählen soll.

✅ **KQ31-3 a** (Patrik S206): **Die Schwellen von KQ31-2 a bleiben; was am Körper darüber liegt, steht als bekannte Abweichung mit ihrem Wert im Test (rot, wenn sie wächst oder verschwindet), die Ursache als offener Punkt zusammen mit der Höhe der Spitzen.** Das sind zwei Lagen (S(1,2) Bauch 1 +2,1 Messorte, S(1,3) Bauch 2 +2,5) und sechs Höhen (S(1,5) Bäuche 4, 5: −9,1 / −11,2 dB; S(1,6) Bäuche 3–6: −6,5 / −8,2 / −15,4 / −16,5 dB). Bells schwache Bäuche liegen nur 6–9 dB über der Linie der Feder 2,6 · 10⁵ N/m; die Feder allein erklärt sie nicht (mit ihr verschwinden sie im Modell als Maxima). Verworfen: b (Höhe nur vergleichen, wo Bells Pegel 10 dB über der Federlinie liegt: nähme eine offene Abweichung aus dem Test, an einer Feder ohne belegte Herkunft), c (erst die Ursache suchen: der neue Körper liegt in jeder gemessenen Größe besser als der alte, die Suche hat keinen Weg). Kippt zu b, wenn die Feder aus Bells Messaufbau belegt wird.

**Gebaut (S206):** KQ35-1 a und KQ35-2 a — **`.lvinstbody` Fassung 5**: je Mode `bridgeAngle` (Winkel der Stegbewegung gegen die Senkrechte der Decke in Grad; `bridge` ist der Formwert längs dieser Richtung: senkrecht `bridge` · cos, parallel `bridge` · sin, 90 = nur parallel zur Decke), im statistischen Teil `bridgeAngle` (Betrag für die gezogenen Moden, Vorzeichen gezogen) und `parallelFactor` (je gezogener Mode eine zweite nur parallel zur Decke); alles immer geschrieben, ein Körper aus einem Bauplan schreibt 0, Fassung 4 liest sich mit 0. `InstrumentBody` 0.24.0 (`ModeImKoerper::stegWinkelGrad`, `senkrechtAm`, `parallelAm`; Antwort und Mobilität rechnen mit dem senkrechten Formwert, bei Winkel 0 Bit für Bit wie vorher; `ziehe` zieht das Vorzeichen nur mit einem Winkel). Die sieben Körper mit Bauplan mit `--rewrite` neu geschrieben: gleich bis auf `version`, `units` und die neuen Felder (alle 0). **Die Gitarre neu (`kFitFassung` 4):** 233 Moden bis 5 kHz — 75 gefittet, 42 gezogen bis zur Dichte nach Elie 2011, 116 nur parallel zur Decke; statistischer Teil 0,0215 / Hz, Winkel 12,76°, Anteil 0,397, 653 gezogene Moden. Gegen Elie 2012 an der Saite 1: Kreuzterm auf 0–5 dB, Yyy auf 1–5 dB (1–4 kHz −14,6 / −13,3 gegen −12,9 / −15,6 dB). Am Prüfstand je Taste (zwei Polarisationen, 30°): Spanne 12,7 LU, größter Sprung 12,7 LU (68 / 71), mittlerer 1,7 LU, Lautheit −2,04 LUFS, Abfall 14,4 / 13,1 dB/s — wie die Probe aus S205. **Abweichung von der Probe aus S205:** der Betrag der Abstrahlung der gezogenen Moden wird an überlappenden Terzbändern gestellt (4 kHz zählt für beide Auffüllbänder), ohne Überrelaxation und höchstens bis zum Betrag der kopierten Mode — das Stellen der Probe lag je Samen bis 8,6 dB über der Messung bei 4 kHz, so höchstens 1,3 dB (acht Samen); im Körper −5,3 / −13,2 / −3,6 dB, Terzpegel gegen g34 bei 2,5 / 3,15 / 4 kHz −0,1 / +0,6 / +0,6 dB (`left`). Der Prüfstand liest die Richtung aus der Körperdatei (`richtung_von`); die Annahme nach Scherrer gilt nur noch für Körper ohne Richtung. **Der größte Sprung 12,7 LU liegt über KQ34 a (12 LU) — offen.**

✅ **KQ32 a** (Patrik S203): **Die Teiltonmessung bestimmt zwei Stufen als Summe zweier abklingender Komponenten mit Boden, an die Energie angepasst; die zwei Geraden mit Knick sind nur der Startwert.** Ziel ±5 % auf beide Abklingzeiten auch bei flachem Knick (Fälle „flacher Knick“ in `test_Teiltonmessung.cpp`); danach Salamander neu messen (`salamander_*.txt`, Archiv `E:\samples`), Diff prüfen, den Flügel gegen die neuen Ziele vergleichen. Befund (S203, Bericht 16 der Research-Werkbank Fall D an unserem Code): bei einem Knick 20 dB unter der Spitze T60 früh 1,795 statt 1,5 s (+20 %) und 0,438 statt 0,4 s (+10 %, Harfe im Diskant), die späte Stufe auf 1 %; bei 30–35 dB unter 5 %. Verworfen: b (zwei Geraden mit Grenze: die frühe Rate wäre bei flachem Knick keine physikalische Größe), c (beide Verfahren: zwei Werte für dieselbe Größe). Kippt, wenn die Anpassung an Teiltönen mit Schwebung instabil wird — der Fall „zwei Stufen … Auslöschungen am Übergang“ muss grün bleiben.

✅ **KQ33 a** (Patrik S203; **ändert die Herkunft der Schwellen in GZ14 a**): **Die Schwellen für Spanne und Sprünge werden an reflexionsarmen Aufnahmen neu bestimmt — TU-Berlin-Datenbank (Weinzierl u. a., CC BY-NC-SA 4.0), Gitarre E2–B5 bei ff und Harfe C1–F4, je Ton die Leistungssumme der 32 Mikrofone; über F4 bleibt für die Harfe die heutige Schwelle mit Vermerk.** Vor dem Laden Größe und Dateien zur Freigabe. Grund: ein Mikrofon im Raum erzeugt allein über 72 Tasten einen mittleren Sprung von 2,4 LU und einen größten von 10,2 LU (95 %: 12,6; Bericht 16 § 3 C, Gl. 13) — die Schwellen 4 / 12 LU liegen fast ganz im Rauschen des Raums. Verworfen: b (Schwellen lassen), c (rechnerisch um den Raum verschärfen: angenommenes DRR, nicht unabhängige Größen abgezogen). Kippt, wenn die Datenbank Einzeltöne nicht trennbar liefert oder die Lizenz die Ablage verbietet.

✅ **KQ34 a** (Patrik S203; **ersetzt die Schwellen von GZ14 a**, nach KQ33 a): **Spanne ≤ 15 LU, an der Harfe ab D2 ≤ 22 LU; mittlerer Sprung ≤ 3 LU; größter Sprung ≤ 12 LU** (Sprünge weiter als Paare bis 4 Halbtöne an jeder Taste). Belegkraft: Messung an reflexionsarmen Aufnahmen — TU Berlin (Weinzierl u. a., DepositOnce 19858, CC BY-NC-SA 4.0; Ablage `E:\samples\Klasse 2 - Gezupfte Saite\TU_Berlin_Weinzierl_DepositOnce19858\`), je Ton K-bewertete Leistung gemittelt über 32 Mikrofone, Momentary-Maximum wie im Prüfstand: Gitarre (Kohno 1985) ff 14,4 / 10,3 / 2,5 LU (Spanne / größter / mittlerer Sprung), Harfe ff ab D2 22,0 / 11,1 / 2,5 LU (MIDI 23–102; bei pp eine einzelne zu laut gespielte Note, 17,6 LU) — `../modellierung/gezupfte-saite/proben/2026-11-17_S203_kq33_tu_*`. Befund: ohne Raum liegt der größte Sprung am Instrument selbst bei 10–11 LU; Bericht 16 überschätzt den Anteil des Raums. Modelle (S201) gegen die neuen Grenzen: Gitarre 11,9 / 11,9 / 1,6, Laute 13,9 / 11,8 / 2,4, Cembalo 6,8 / 6,0 / 1,5, Harfe ab D2 19,6 / 11,0 / 2,9 halten; die Oud verfehlt die Spanne (15,8 LU, der einzelne Basschor). Verworfen: b (nur die Spanne der Harfe, mittlerer Sprung ≤ 4 LU), c (GZ14 a lassen). Kippt, wenn ein drittes Instrument ohne Raum einen mittleren Sprung über 3 LU zeigt (Kandidat Cembalo).

✅ **KQ36 a** (Patrik S207; **legt fest, woran KQ34 a am Modell gemessen wird**): **KQ34 a gilt für das Leistungsmittel über viele Hörorte in rund 2 m. Ein Körper aus einem Bauplan wird für diese Prüfung mit 16 Hörorten auf der vorderen Halbkugel in 2,06 m gerechnet (Prüfkörper, nicht der mitgelieferte). Ein gemessener Körper, der das nicht kann (Gitarre: zwei Mikrofone im Nahfeld), wird gegen die Verteilung der TU-Aufnahmen für dieselbe Zahl Mikrofone geprüft: größter Sprung ≤ 13,3 LU, mittlerer ≤ 3,0 LU (95. Perzentil der 496 Paare), mit Vermerk Nahfeld.** Befund (S207, Bericht [../modellierung/gezupfte-saite/2026-11-19_S207_spruenge_je_taste.md](../modellierung/gezupfte-saite/2026-11-19_S207_spruenge_je_taste.md), Proben `../modellierung/gezupfte-saite/proben/2026-11-19_S207_*`): die Sprünge über KQ34 a sitzen in |p/F| des Körpers am Grundton an ein bis zwei Hörorten, nicht in Saite, Steg oder Fit — Gitarre 68 / 71: −3,9 / −0,2 dB bei 415 Hz gegen +11,0 / +10,0 dB bei 494 Hz, an g34 an denselben zwei Mikrofonen gemessen (+10,8 dB, Modell +11,3 dB; am dritten Mikrofon `east` −9,2 dB); Harfe 78 / 80: −39 gegen −16 dB am einen Ort `front`. An einem einzelnen Mikrofon liegt die echte Harfe (TU, ff ab D2) bei 13,9 LU größtem Sprung (5.–95. Perzentil 11,6–16,3) und 3,1 LU mittlerem, die Gitarre bei 11,1 (10,1–14,9) und 2,8; Paare 11,9 (10,5–14,0) / 2,8 und 10,5 (9,7–13,3) / 2,7. Gemessen nach KQ36 a: **Harfe 20,0 / 12,8 / 3,0 LU** (Spanne / größter / mittlerer Sprung) — **der größte Sprung 0,8 LU über der Schwelle, am Paar 43 / 47 (Übergang Metall- zu Darmsaiten; die echte Harfe 11,1 LU an 45 / 48), offen**; das Paar 78 / 80 liegt bei 6,9 LU. Gitarre 12,7 / 12,7 / 1,7 LU — hält. Das Harfenmodell liegt bei jeder Zahl von Orten rund 2 LU über der echten Harfe; die Tasten 58, 64, 80 treffen eine Körpermode auf wenige Hertz und werden mit doppeltem Verlustfaktor 2,8 / 4,7 / 5,1 LU leiser (der Körper ist ohne Saitenlast gerechnet). Verworfen: b (am Modell je Zahl der Hörorte das 95. Perzentil der TU-Aufnahmen, gemessen am mitgelieferten Hörort — Harfe 14,5 / 3,3 hielte; der Ort `front` liegt aber günstig: 14,5 gegen 16,2 LU im Median der 16 Orte, die 2 LU Überschuss gälten als bestanden), c (KQ34 a lassen, die Sprünge als bekannte Abweichung). Kippt zu b, wenn die Schwelle als Aussage über den Klang am mitgelieferten Hörort verstanden wird; kippt auch, wenn die vordere Halbkugel die TU-Anordnung zu schlecht trifft (Abstrahlung nach hinten — nicht geprüft). **Laute, Oud, Cembalo nach KQ36 a (S207, Bericht §7):** Laute 12,9 / 8,4 / 2,4 LU — hält; Cembalo mit GZ13 a 5,8 / 3,0 / 0,6 LU — hält; **Oud 18,8 / 14,7 / 1,8 LU — Spanne und größter Sprung (40 / 44) verfehlt, offen**: in 2 m fällt der einzelne Basschor stärker ab als am bisherigen Ort in 0,5 m (dort 15,8 / 11,2 / 2,7); Laute und Oud strahlen in 2 m fast ungerichtet (Streuung über die Orte 0,4 dB). Die Moden der Prüfkörper sind die der mitgelieferten Körper; die gezogenen Moden über 5 kHz sind mit 17 Hörorten eine andere Ziehung (Laute Chor 8 bis 2,1 LU).

✅ **KQ37 a** (Patrik S207): **Die Oud bleibt nach KQ36 a bekannte Abweichung mit Ursache (18,8 / 14,7 / 1,8 LU; ohne den einzelnen Basschor, ab Taste 41, 11,4 / 10,3 / 1,6 LU), bis die Messgrundlage da ist: Auftrag `2026-11-19_35_oud_basschor_einzeltoene` an die Research-Werkbank (Pegel einzelner Töne je Chor, Abstrahlung oder Stegadmittanz einer arabischen Oud mit Zuordnung der Moden, Zupf des Basschors, Zug je Saite); mit dem Bericht wird entschieden, ob Modell, Zupf oder Schwelle geändert wird.** Befund (Bericht S207 §8): der Basschor liegt 12,9 LU unter dem Median der übrigen — eine Saite statt zwei (4–5 dB weniger Kraft bei gleicher Auslenkung), und sein zweiter und dritter Teilton liegen im Tal der Abstrahlung zwischen der Luftmode 99,9 Hz und den Moden 208 / 269 Hz (|p/F| in 2 m −19 … −10 dB gegen +3 … +8 dB). **Das Tal ist gemessen** (Değirmenli 2017 Abb. 7, sieben türkische Ouds, Schalldruck je Kraft in 1 m, in S207 am Seitenbild abgelesen: Tal 26 dB unter der Hauptspitze, Modell 33 dB; Luftmode −13 gegen −16 dB). **Verdacht am Bauplan:** die Hauptspitze der Abstrahlung liegt im Modell beim 2,7-Fachen der Luftmode (257 Hz), gemessen beim 1,7-Fachen (205 Hz); Abdallah 2026 Abb. 6 hat die stärkste Spitze bei 204 Hz und Tab. 3 die erste Mode der Decke bei 142 Hz, im Modell ist die Mode 207,8 Hz am Steg die schwächste unter 420 Hz und die Mode 168,6 Hz bewegt den Steg stark und strahlt kaum — der Bauplan wurde in S196 nur an der Lage der Spitzen ausgerichtet, Höhen und Formen sind nicht verglichen; offen. Verworfen: b (Zupf mit gleicher Kraft je Chor: 14,1 / 13,1 / 1,9 LU, der größte Sprung wandert nach 52 / 56 und verfehlt; träfe auch die Laute), c (Kennzahl ab Chor 2 wie „Harfe ab D2“: hielte, ohne Beleg an einer echten Oud). Kippt zu c, wenn der Bericht zeigt, dass der tiefe Chor echter Ouds um 10 LU oder mehr abfällt und der Bauplan in Höhe und Form hält.

✅ **KQ48 a** (Patrik S209; **nach dem Wortlaut von KQ40 a**): **Die Abstrahlung an den Saiten 33–22 gegen Bell 1988 Abb. 7.23 bleibt bekannte Abweichung mit Wert — die gebaute Fassung trägt ihre zwölf Werte im Bericht und die Zahl der gehaltenen Saiten im Test (rot, wenn sie fällt).** Befund: in rund 60 Fassungen halten höchstens 6 von 12 (heute 4), der Überschuss bei 155 / 165 Hz und um 233 Hz bleibt 10–15 dB; Bells Wert verlangt die beiden Moden mit Luft dicht beieinander (besaitet 187 / 207 Hz), im Modell liegen sie 50–60 Hz auseinander, und Bells Paare (ohne Saiten 170 / 216, besaitet 187 / 207) passen untereinander nicht zu einer festen Kopplung von Platte und Luft. Verworfen: b (das Ziel in drei Bändern fassen — formt das Ziel um, ohne dass der Körper besser wird). Kippt, wenn die Feinsuche nach KQ47 a eine Fassung findet, die in Bändern klar besser ist als eine andere mit gleichen Lagen.

✅ **KQ47 a** (Patrik S209): **Der Bauplan der Harfe wird der Kandidat aus S209, nach neuer Eichung fein gesucht — Boden 12 → 2 mm (Bell & Firth; Abb. 2.4 a nennt 10 → 3 mm), Bassende eingespannt (die Schraube durch Deck- und Zugleiste in den Bodenblock), Mündungskorrektur um 2,0. Zuerst die Eichung des Holzes am freien Boden mit der neuen Dicke nachrechnen (Tab. 3.1 / 3.2, elf Moden auf 3 %); dann fein suchen über Dicke am Bass, Mündung, Feder der Diskantkante und Güte der Luft gegen die sieben Lagen, am Netz des Körpers gegengeprüft; danach Körper, KQ31-2 a und die Lautheit mit Mitschwingen nach KQ39 a.** Kandidat (bis 1,2 kHz, Hohlraum 27 l): S(1,1)− / S(1,1)+ 164 / 222 Hz (−4 / +3 % gegen 170 / 216; heute 134 / 198), S(1,2)–S(1,6) +1,7 / +5,9 / +0,3 / +3,2 / +0,7 %; KQ31-2 a: vier bekannte Abweichungen statt acht (die zwei Lagen der Bäuche fallen weg); Lautheit je Taste (16 Hörorte, ohne Mitschwingen) 16,8 / 13,0 / 2,7 LU gegen 20,0 / 12,8 / 3,0 heute. Befund: die Platte allein liegt ohne Luft bei 146 Hz — sie ist im Bassteil zu weich, am Diskant stimmt sie; Luftraum, Drehfeder am ganzen Seitenrand und das eingespannte Bassende heben je für sich S(1,1)− nicht weit genug oder alle Moden gleich. Verworfen: b (Drehfeder 10⁴ N an den unteren 60 cm der Seiten: Lagen ähnlich, kein Beleg, Abstrahlung 2 / 12), c (Bauplan bleibt). Kippt, wenn die Eichung mit 12 → 2 mm die elf Moden des freien Bodens nicht auf 3 % hält und sich mit anderem Holz nicht herstellen lässt.

✅ **KQ50 a** (Patrik S210; **hebt KQ47 a auf**): **Der Boden der Harfe bleibt 10 → 3 mm nach Bell 1988 Abb. 2.4 a mit dem Holz aus S197. Die Steife im Bass kommt aus einer Drehfeder an den Bassseiten als Randabschnitt (angepasst, ohne Quelle, wie die Diskantkante nach KQ21 a). Die Feinsuche läuft über Länge und Wert der Feder, Mündung, Bassende und den Faktor der Saiten.** Befund (Harfenbericht §19): die Eichung am freien Boden mit 12 → 2 mm verlangt E quer 4,77 statt 7,3 GPa (längs 0,768, G 0,387 GPa, 456,7 kg/m³ aus der Masse 1,375 kg; elf Moden innerhalb 1,8 %) — der freie Boden unterscheidet die Dicken nicht (10 → 3 mm neu angepasst 0,94 %, 12 → 2 mm 0,83 % im quadratischen Mittel). Der Kandidat aus S209 rechnete 12 → 2 mm mit dem Holz für 10 → 3 mm; geeicht liegt er bei 145 / 214 Hz gegen 170 / 216 Hz (S209: 164 / 222). Im Raster nach der Eichung (32 Fassungen) kommt ohne Drehfeder an den Bassseiten keine über 154 Hz. Die Lage der Bäuche (KQ31-2 a) trennt die Dicken: mit 12 → 2 mm sitzen die Bäuche von S(1,2)–S(1,4) 1–3,5 Messorte zu weit am Diskant, mit 10 → 3 mm alle innerhalb von 2 Orten. Verworfen: b (12 → 2 mm geeicht mit Feder, Lage der Bäuche als bekannte Abweichung), c (KQ47 a wörtlich, S(1,1)− −15 % als bekannte Abweichung). Kippt, wenn Bell & Firth die 12 → 2 mm als an diesen Böden gemessen ausweisen oder die Feinsuche S(1,3) nicht unter 7 % bringt. Annahme der Eichung: die (2,n)-Moden aus Tab. 3.2 der Reihe nach zugeordnet (§19.1).

✅ **KQ51 a** (Patrik S211): **Die Harfe trägt eine Drehfeder von 7 000 N an beiden Seiten vom Bassende bis y = 0,6 m, die Mündung ist 2,3 (Fassung F4 der Feinsuche). Der größte Sprung der Lautheit je Taste (mit dem Faktor der Saiten nach KQ52 a 12,8 LU gegen 12) wird im Test als bekannte Abweichung mit Wert festgehalten.** Grund: nur die Fassungen mit 7 000 N treffen die Lage aller Bäuche (F2 / F3 verfehlen S(1,3)); der größte Sprung schwankt zwischen Nachbarn des Rasters um 3 LU und trennt nicht; gegen F1 liegt F4 an zweiter Mode mit Luft (+1,3 gegen +2,8 %), Güte, Sprung und Abstrahlung ohne Saiten näher. Verworfen: b F3, c F2, d F1 (Harfenbericht §20.2, §20.4, §20.5). Kippt auf b, wenn die Grenze 12 LU ohne bekannte Abweichung gelten soll, oder neu vorlegen, wenn der Sprung von F4 mit dem Faktor der Saiten nach KQ52 deutlich über 12 LU liegt. Der Kipppunkt ist geprüft (S211): mit 0,3 / 0,4 liegt der Sprung bei 12,8 LU, 0,8 LU über der Grenze und unter der Schwankung zwischen Nachbarn — F4 bleibt. **Nachtrag S211: die Werte stammen aus der Rechnung im Modenraum (KQ54 a, Befund); am gebauten Körper liegt der Sprung bei 15,7 LU (Spanne 18,1, Mittel 2,3) — der Kipppunkt greift, KQ51 ist neu vorzulegen (Harfenbericht §20.7).**

✅ **KQ52 a** (Patrik S211): **Die Saiten der Harfe wirken als Federn mit k = 0,3 · E·A/L an den Ösen (`pointSprings`), je mit ihrem Verlustfaktor nach KQ49 a. Der größte Sprung der Lautheit je Taste (12,8 LU) wird bekannte Abweichung mit Wert.** Grund: Bell 1988 misst beim Besaiten fünf Verschiebungen (S(1,1)− +10, S(1,2) +3, S(1,3) +5,7, S(1,4) −0,5, S(1,5) +1,8 %); an F4 gibt 0,3 +5,3 / +6,5 / +3,6 / +2,1 / +1,2 %, die kleinste Summe der Quadrate über fünf wie über drei Moden (46 / 39 gegen 56 / 51 bei 0,2 und 57 / 46 bei 0,4). Lautheit mit Mitschwingen 19,2 / 12,8 / 2,5 LU (0,2: 18,4 / 12,2 / 2,5; 0,4: 19,6 / 12,8 / 2,5) — der Unterschied im Sprung liegt unter den 3 LU, um die er zwischen Nachbarn schwankt. Verworfen: b 0,2 (kleinster Sprung, S(1,1)− am weitesten von Bell), c 0,4 (tiefste Mode am besaiteten Wert 187 Hz am nächsten, S(1,2) fast dreimal so weit wie bei Bell). Kippt, wenn die E-Module der Saiten gemessen vorliegen (dann c aus ihnen), oder wenn die Lage von S(1,1)− besaitet innerhalb 7 % von 187 Hz harte Zielgröße wird (dann 0,4). Harfenbericht §20.6.

✅ **KQ53 a** (Patrik S211): **Die Körperdatei trägt je Mode `springShare` (immer geschrieben, Fassung 5 liest sich als 0). `.lvinstbody` Fassung 6, alle sieben Körper mit `--rewrite` neu geschrieben, die Harfe neu gerechnet. Der Wächter rechnet die Dämpfung mit dem Anteil der Federn nach, genau wie für alle anderen Körper.** Befund: mit der Harfe als erstem Körper mit Federn meldete der Wächter alle 129 Moden als falsch — η = (1 − Anteil der Federn) · η der Platte, und der Anteil (an den tiefen Moden 0,19 / 0,19 / 0,13) stand nicht in der Datei. Verworfen: b (der Wächter prüft mit Federn nur, dass der zurückgerechnete Anteil zwischen 0 und 1 liegt). **Gebaut S211:** `kKoerperFassung` 6, `InstrumentBody` 0.26.0; der Wächter rechnet genau, wenn alle Federn denselben Verlustfaktor tragen, sonst zwischen dem kleinsten und dem größten; die Gitarre schreibt `make_gitarre.py` (Fassung 6 im Schreiber, die Datei um die zwei Felder ergänzt, nicht neu gefittet).

✅ **KQ54 a** (Patrik S211): **Der Fall der Harfe rechnet die Abstrahlung an den Saiten 33–22 an der besaiteten Platte mit den 16 Hörorten von KQ36 a selbst (Hörorte nur im Test) und hält 6 von 12 fest, rot, wenn es weniger werden. Der größte Sprung der Lautheit steht wie bei der Oud im Bericht, mit Wert und Verweis im Test.** Grund: die Hörorte im Bauplan änderten den Klang (das Programm nimmt für rechts `right`, ohne ihn den zweiten Hörort, `Boden.cpp`); die Lautheit je Taste ist eine Messung von rund 65 min und passt in keinen Unit-Test. Verworfen: b (beide nur im Bericht). **Gebaut S211:** der Fall rechnet die Platte ohne und mit Federn bis 1,5 kHz am Netz für 5 kHz (rund 28 s; bis 1,5 und bis 5 kHz auf 0,1 dB gleich). Befund beim Bau: die volle Rechnung mit Federn in der Platte gibt an den Basssaiten bis 5 dB andere Abstrahlung als die Suche, die die Saiten im Modenraum ansetzte — Luftanteil und Güte der beiden Moden mit Luft 0,65 / 0,34 und 11,9 / 19,0 gegen 0,50 / 0,45 und 15,1 / 16,7 (der Modenraum führt Luftanteil und Dämpfung nur auf der Diagonale; mit 409 statt 129 Moden gleich). Die Lautheit je Taste aus §20.4 / §20.6 des Harfenberichts ist am Modenraum gemessen; am gebauten Körper steht sie aus (Harfenbericht §20.7).

✅ **KQ55 a** (Patrik S212): **F1–F3 werden an der vollen Rechnung gemessen (Saiten 0,3 in der Platte, 16 Hörorte), danach F3 mit je einem Hebel an den Tasten 50–62: Faktor der Saiten 0,2 / 0,4, Güte der Luft 5,5 / 12, Mündung 1,8 / 2,2. Bringt kein Hebel das Paar 54–58 unter 12 LU, ist es ein Fable-Fall.** Befund (Harfenbericht §20.8): an der vollen Rechnung 13,2–15,7 LU in allen vier Fassungen, immer 54 → 58; S(1,3) überall 9,8–10,3 % zu hoch (im Modenraum ≤ 7 %). Kein Hebel unter 12 LU (am nächsten Faktor 0,2 mit 12,02 LU), jeder verschiebt den Sprung um höchstens 1,2 LU.

✅ **KQ56 a** (Patrik S212): **Der Bauplan der Harfe wird in einer eigenen Sitzung mit Fable neu gedacht, die Suche läuft an der vollen Rechnung (Federn in der Platte), nicht im Modenraum. Ziele zusätzlich zu KQ31-2 a, KQ40 a und GZ14 a: die zweite Mode mit Luft der besaiteten Harfe bei 216 Hz, S(1,3) auf 7 %, das Paar 54–58 unter 12 LU. Der gebaute Körper F4 (KQ51 a, KQ52 a) bleibt bis dahin.** Grund: die Hebel an F1–F4 kratzen nur an der Schwelle, die zweite Mode mit Luft (+4 bis +9 % gegen Bell) und S(1,3) (+9 bis +11 %) bleiben in jeder Variante — der Fehler liegt in der Struktur des Bauplans. Verworfen: Faktor 0,2 an F3 voll rechnen (12,02 LU auf der Grenze, gegen Bells Verschiebung beim Besaiten, KQ52 a; S(1,3) bliebe +8,7 %). Kippt, wenn S(1,3) und die Verschiebung beim Besaiten als bekannte Abweichungen genommen werden. **S213 (KQ57 a): die Ziele lauten gegen Bell besaitet — zweite Mode mit Luft 207 Hz, S(1,3) gegen 334 Hz; die Ursache liegt nicht im Bauplan, sondern im Modell von Luft und Kasten (KQ58 a).**

✅ **KQ57 a** (Patrik S213): **Die Lagen des gebauten Körpers der Harfe gelten gegen Bells besaitete Harfe (Kasten G in der Harfe, besaitet: 187 / 207 / 257 / 334 / 430 / 555 / 697 Hz für S(1,1)− bis S(1,6), je 7 %). Die Werte ohne Saiten (170 / 216 / 249 / 316 / 432 / 545 / 709 Hz) bleiben im Test als Bericht an der Platte ohne Federn, ohne Schwelle am Paar mit Luft. KQ56 a lautet damit: zweite Mode mit Luft bei 207 Hz, S(1,3) gegen 334 Hz.** Grund: der Körper ist nach KQ46 a die besaitete Harfe; `harfe_suche.py` und KQ56 a verglichen besaitete Körper mit den Werten ohne Saiten. Heute (F4): −7,4 / +10,5 / 0,0 / +4,4 / +4,9 / +3,4 / +2,6 % — nur das Paar mit Luft verfehlt. Verworfen: b (beide Zustände als Ziel) — eine Kopplung an die Luft trägt nicht beide: mit Faktor 1 hält der Zustand ohne Saiten (−3,3 / +1,3 %), mit 0,35 der besaitete (+1,3 / +1,7 %). Kippt, wenn sich Bells 187 / 207 Hz als andere Moden herausstellen (für 187 Hz nennt er keine Güte); dagegen steht Le Carrou 2007 mit 153,5 / 172 Hz. Harfenbericht §21.1, §21.2.

✅ **KQ58 a** (Patrik S213): **Erst die Ursache an den Quellen, dann das Modell. Das Modell koppelt den Boden drei- bis viermal zu stark an die Luft (Versteifung durch die eingeschlossene Luft 49 Hz gegen 12–15 Hz an zwei besaiteten Harfen), und die obere Mode des Paars strahlt 10–15 dB zu stark. Gelesen werden am Seitenbild Bell Kap. 5 und 7 (Bewegung der Schale, Profile und Höhen von S(1,1)± ohne und mit Saiten) und Le Carrou 2006 Kap. 3–4 (Formen von T1 / A0 samt Schale, Schnelle in den Öffnungen); drei Kandidaten je mit einer Probe gegen alle Zielgrößen: mitbewegte Schale, die Luft des 1,3 m langen Kastens als Rohr mit Öffnungen längs, Dämpfung durch die Saiten. Danach eine Vorlage, welches Bauteil in `InstrumentBody` kommt. F4 bleibt gebaut.** Grund: ein Kopplungsfaktor ohne Ursache (Probe, Faktor 0,35) bringt alle sieben Lagen auf 5 %, verschlechtert aber die Abstrahlung (3 / 12 statt 6 / 12) und den größten Sprung (19,0 statt 15,7 LU, 52 → 56). Verworfen: b (Kopplungsfaktor am Hohlraum, Bauplan-Fassung 10), c (F4 mit 15,7 LU als bekannter Abweichung, weiter mit 34k-7 — der Sprung liegt im Modell an der falschen Stelle, 54 → 58 statt 52 → 54 wie in Bell Abb. 7.23). Verworfen in der Sitzung: der Boden als flache Schale unter dem Saitenzug (das Volumen der ersten Mode bleibt 0,148 → 0,147 m³/√kg). Kippt zu c, wenn die Quellen keine der drei Ursachen unterscheidbar machen. Harfenbericht §21.2–§21.5.

✅ **KQ59 a** (Patrik S214): **Die Luft der Harfe wird als Rohr gerechnet: `InstrumentBody` bekommt eine zweite Art des Hohlraums mit dem Druck längs des Kastens (Längsmoden des beidseitig geschlossenen Rohrs mit dem Querschnitt längs, je Öffnung ein Luftpfropfen an seinem Ort, getrieben vom Volumen der Plattenmoden je Länge; Bauplan-Fassung 10). Ein Wächter prüft das Rohr allein gegen Bell Kap. 4 (nur Öffnung 6 offen: A2–A5 283 / 414 / 545 / 680 Hz; alle offen: Helmholtz 190 ± 1,5 Hz). Die Harfe wird damit neu gerechnet; die Form der ersten Mode geht danach als eigene Suche an den Bauplan.** Grund: die erste Längsmode der eingeschlossenen Luft liegt bei 150 Hz unter dem Paar mit Luft, ein Druck für 1,3 m Kasten überschätzt die Kopplung (Versteifung der ersten Bodenmode 55 Hz statt 36,5 Hz; gemessen 12–25 Hz an drei Kästen und einer zweiten Harfe). Probe `proben/2026-11-23_S214_harfe_rohr.py` an F4, Mündungskorrektur 1,85: Rohr allein 283 / 406 / 532 / 659 Hz und 189,4 Hz; Paar 172,5 / 215,4 Hz (−7,7 / +4,0 % gegen 187 / 207), S(1,2)–S(1,6) −4,1 / −2,2 / 0,0 / +9,2 / +5,3 %, eine Mode bei 312 Hz (Bell S(1,2)′ 284 Hz), Abstrahlung 33–22 6 / 12 mit größter Abweichung 11,2 dB (gebaut 20,3), größter Sprung an den Tasten 50–62 mit Mitschwingen 10,9 LU (gebaut 15,7), Öffnungen zu: untere Mode weg, obere 215 → 207 Hz (Bell S. 106). Verworfen: b (erst die Form der ersten Mode am Bauplan, Rohr nur als Probe — die Suche liefe wieder im Modenraum und mit dem falschen Luftmodell), c (F4 mit bekannter Abweichung). In der Sitzung verworfen: die Saiten als Dämpfer als Ursache der Lage (Zuwachs des Verlustfaktors 0,007–0,015, wie Bells Güten beim Besaiten; gehört zum Messzustand), die Schale als nachgiebige Wand (Bells Helmholtz-Mode am freien Kasten 187 Hz, im Sand 190 Hz). Kippt zu b, wenn das Rohr im Werkzeug die 10,9 LU der Probe nicht bestätigt — dann zuerst die Probe prüfen. Harfenbericht §22.

✅ **KQ60 a** (Patrik S214; ändert KQ29-1 a, KQ40 a, KQ51 a in den Zielen der Luft): **(1) Die Luft der Harfe wird auf Bells Helmholtz-Mode 190 ± 1,5 Hz gestellt (starre Wände, alle Öffnungen offen); dazu gilt ohne Saiten: die Bodenmode bei verschlossenen Öffnungen und die Helmholtz-Mode liegen auf 3 % beieinander (Bell S. 115: 191 / 187–190 Hz an Kasten A, 196 / 194 Hz an Kasten F). Das Paar 170 / 216 Hz von Kasten G in der Harfe ohne Saiten bleibt Bericht. (2) Die Abstrahlung an den Saiten 33–22 (KQ40 a) wird auf den Ausgleich über alle zwölf Saiten bezogen (Median der Abweichungen) statt auf Saite 26 und mit den Saiten als Dämpfer gerechnet (Wellenwiderstand √(T m′) je Saite quer zur Saite, Anteil cos² 35°; Bell S. 147, 163: Saiten mit Kunststoffband stark gedämpft). (3) Das Profil von S(1,1)− wird Zielgröße: Bauch bei y = 0,45 m auf zwei Messorte, Knoten bei Ort 14–16 (Bell Abb. 7.8).** Grund: ein Ziel trägt den Zustand, an dem die Quelle gemessen hat — 216 Hz ist ein schwacher Gipfel (16 dB unter S(1,2), Einbruch im Profil, Abb. 7.9), vier andere Bestimmungen geben 12–25 Hz Versteifung; Saite 26 sitzt auf der oberen Mode des Paars, mit ihr als Bezug fällt die Zielgröße umso schlechter aus, je richtiger das Paar liegt; das Modell hat für S(1,1)− einen Bauch von 0,74 m um y = 0,55 m ohne Knoten, Bell 0,40 m um 0,45 m. Mit dem neuen Bezug (ohne Dämpfer): gebaut 4 / 12, größte 17,5 dB; Rohr 6 / 12, größte 11 dB. Verworfen: b (nur 1 und 2, die Form bleibt Bericht), c (Zielgrößen unverändert). Kippt in (3), wenn die Rechnung am Bauplan zeigt, dass Bauch und Knoten nicht zusammen zu halten sind. Harfenbericht §22.2, §22.3, §22.6.

✅ **KQ61 a** (Patrik S215; zu KQ59 a): **Der Querschnitt des Rohrs steht im Bauplan als Tabelle längs: `cavity.kind` (`singlePressure` / `tube`) und `cavity.tube.sections` (y, Fläche) mit Quelle, beides immer geschrieben (Bauplan-Fassung 10); `pruefe` verlangt, dass die Querschnitte das Volumen auf 1 % tragen. Die Harfe: Halbkreis mit Tiefe gleich halbe Breite (Bell Abb. 2.7), also S ∝ Breite², auf Bells 27 l gestellt.** Grund: die Form des Kastens gehört zu den Daten des Instruments, die Bibliothek nimmt nichts über Harfen an. Gegen Bells zwölf Luftmoden (Kap. 4): S ∝ w² rms 6,6 %, S ∝ w 12,1 %, S konstant 13,6 %. Verworfen: b (eine Regel `widthSquared` in der Bibliothek). Kippt, wenn auf absehbare Zeit kein zweiter Körper mit Rohr kommt. Harfenbericht §23.1, §23.2.

✅ **KQ62 a** (Patrik S215): **Gerechnet werden alle Längsmoden des Rohrs bis zur oberen Grenze des Rechenfensters (Harfe 5 kHz: rund 38); die Zahl folgt aus Querschnitt und Fenster und steht nicht im Bauplan. Der Kopf von `Luftraum.hpp` nennt die Grenze: ein Rohr kennt keine Quermoden.** Grund: die Platte wird bis 5 kHz gerechnet, der Druck soll dort aufgelöst sein. Am Rohr allein gleich ab 5 Längsmoden; an der Platte 10 gegen 38: Paar 173,2 / 215,1 gegen 172,9 / 215,6 Hz, S(1,2) −3,7 gegen −4,0 %. Verworfen: b (feste Zahl im Bauplan), c (10). Kippt, wenn die oberen Längsmoden die oberen Plattenmoden nur durch den Fehler des Rohrs verschieben. Harfenbericht §23.2, §23.4.

✅ **KQ63 a** (Patrik S215; ändert KQ29-1 a): **Zwei Güten der Luft: `cavity.q` für die Luft in den Löchern und Öffnungen, neu `cavity.tube.q` für die Luft längs; η der Luft = Anteil in den Pfropfen / Q_Öffnung + Anteil längs / Q_Rohr. Die Harfe: 4,4 und 28 aus Bells zwölf Güten der Luftmoden (Abb. 4.8, 4.11–4.15). Die Körperdatei trägt je Mode den Anteil längs (`tubeShare`, `.lvinstbody` Fassung 7), der Wächter `*31c-5d*` rechnet damit.** Grund: der Verlust sitzt in den Öffnungen (Bell: Q fällt, je mehr offen sind); eine Güte für alles trifft Bells Güten nur auf den Faktor 1,9 (Helmholtz 13 statt 6), getrennt auf 1,3. An der Platte: S(1,2) Q 18,0 gegen Bell besaitet 32 (mit einer Güte 13,4: 31,6). Verworfen: b (eine Güte 13,4), c (8,5 wie gebaut). Kippt, wenn S(1,2) auch nach der Suche nach der Form der ersten Mode bei Q um 18 bleibt. Harfenbericht §23.3, §23.4.

✅ **KQ64 a** (Patrik S215): **Jetzt bauen. Der Wächter gegen Bell 1988 Kap. 4 (`test_LoopKoerperRohr`): mit Öffnung 6 allein A2–A5 auf 3 %, mit allen Öffnungen die Helmholtz-Mode bei 190 ± 1,5 Hz und ihr Druckabfall zum Diskant (Bell rund 18 dB) auf 3 dB, die zwölf Güten auf den Faktor 1,5; mit allen Öffnungen A1–A5 und die Helmholtz-Mode mit Öffnung 6 allein, soweit über 3 %, als bekannte Abweichungen mit Wert (rot, wenn sie wachsen oder verschwinden). Die Ursache der zu tiefen Längsmoden mit allen Öffnungen (−9 bis −13 %) ist ein offener Punkt.** Grund: der Bau hängt nicht an der Ursache (sie ändert Daten je Öffnung, nicht die Rechnung); ausgeschlossen sind das Volumen und die Länge der Öffnungen. Verworfen: b (erst die Ursache, Fable-Kandidat). Kippt, wenn die Ursache eine andere Art der Rechnung verlangt (Quermoden im Bassende). Harfenbericht §23.2.

✅ **KQ65 a** (Patrik S215): **`InstrumentBody` hängt an `InstrumentCore` und zerlegt das gekoppelte Rohr dicht mit `kern::zerlegeSymmetrisch`; der eine Druck bleibt Bit für Bit beim Weg mit Rang 1. Ein Test prüft, dass das Rohr ohne Längsmoden den einen Druck auf 1e-9 trifft.** Grund: Abhängigkeit zur Grundnumerik wie bei `InstrumentModels` und `InstrumentMeasure`, kein zweiter Löser; die App bindet beide. Verworfen: b (`Eigenloeser` mit Spectra, für dünne Fenster gebaut, nur in Werkzeug und Tests), c (eigener Jacobi). Kippt, wenn `InstrumentBody` als Quellpaket ohne `InstrumentCore` weitergegeben werden soll.

✅ **KQ66 a** (Patrik S215): **Der statistische Teil zählt nur Moden mit weniger als halber Energie in der Luft (`kStatistikLuftGrenze` 0,5) — er setzt den Boden fort, nicht die Längsmoden des Rohrs.** Grund: über 5 kHz gilt das eindimensionale Rohr nicht; mit den 19 Längsmoden im Band stieg die Dichte der Harfe von 0,043 auf 0,051 je Hz, die Streuung am Steg fiel von 0,315 auf 0,285. Verworfen: b (alle Moden zählen). Kippt, wenn ein gemessener Körper zeigt, dass seine Luft über 5 kHz messbar beiträgt — dann als eigener Teil der Statistik.

✅ **ZK1 a** (Patrik S220; **Umfang 34k-7**): **Die Tafel des Umlauf-Filters ist ein gestimmter Wert (IK2-11 a): `InstrumentMeasure/ZupfStimmen` entwirft sie aus den Werten der Saiten, `InstrumentTuneTool` schreibt sie in die `.lvinst`, der Kern liest sie; der Stimmer-Wächter meldet eine Tafel, die nicht zu den Saiten passt.** Grund: der Entwurf dauerte in Python (S205) am Cembalo 1–40 s je Taste, rund 15 min für 61 Tasten, an Gitarre, Laute und Oud mit mehreren Starts je Saite; beim Laden (b) träfe das jeden Creator, der an einer Saite dreht; was ein Werkzeug aus anderen Werten rechnet, steht bei allen Kernen gestimmt in der Datei, der Wächter fängt veraltete Werte. Verworfen: b (Entwurf beim Erzeugen, `vorbereitungTeuer`, nicht in der Datei). Kippt auf b, wenn der Entwurf in C++ unter rund einer Sekunde je Instrument liegt (in 34k-7c gemessen, dann neu vorgelegt).

✅ **ZK2 a** (Patrik S220; **Umfang 34k-7, zieht einen Teil aus 34k-8 vor**): **34k-7 nimmt Saite und Bund in `KettenNote` auf (ohne Angabe wie bisher); die App gibt sie aus ihrer Saitenwahl (`saitenSpielFuer`) wie bei der Saiten-Schicht. 34k-8 ergänzt Anschlagrichtung, Artikulation und Verläufe.** Grund: ab Rechenfassung 2 spielen Gitarre, Laute und Oud eine gegriffene Saite des Saitensatzes (GZ1-1 a), der Kern muss sie schon im Rechenweg Note kennen; so bleibt die Wahl der Saite in der App, `InstrumentCore` ändert sich in einer Fassung statt zweien, und die Zielgrößen in 34k-7 messen dieselben Saiten, die das Programm spielt. Verworfen: b (bis 34k-8 eine feste Regel im Kern — eine Regel, die 34k-8 wieder entfernt, und Messung mit einer anderen Saitenwahl als im Programm). Kippt, wenn die Saitenwahl der App die Chöre von Laute oder Oud nicht trägt (in 34k-7b vor der Schnittstelle geprüft).

ZK1 a und ZK2 a auf „alle nach Empfehlung“.

✅ **ZB1 a** (Patrik S222; **34k-7d**): **Rechenfassung 2 ersetzt Rechenfassung 1 im Schritt 34k-7d-4: `Zupf.cpp` in der Form aus IC, der gestimmte Faktor `delayFactor`, der IC-Teil von `ZupfStimmen` und die Fälle, die Rechenfassung 1 prüfen, gehen mit dem Umstieg; `werte/zupf_ic.txt` trägt danach die Abdrücke von Rechenfassung 2, vom Kern geschrieben, mit Vermerk (wie `zunge_ic.txt`); das Journal nennt den Commit der letzten Rechenfassung 1.** Grund: nach dem Umstieg ruft kein Kern, kein Werkzeug und keine Stelle der App Rechenfassung 1; IC-Fälle bis 34k-7f prüften drei Schritte lang Code, der nichts mehr erzeugt; der Beleg der Übernahme (J77) ist abgeschlossen und bleibt in Git und unter `harvest/`; gemessen wird gegen den Prüfstand, nicht gegen IC (S182). Verworfen: b (Rechenfassung 1 bis 34k-7f aufrufbar neben dem Kern). Kippt, wenn eine Probe oder Messung bis 34k-7f Rechenfassung 1 als Gegenstück braucht.

✅ **ZU1 a** (Patrik S222; **34k-7d**, Herleitung §12 „Quelle des Zupfs im Zeitschritt“): **Der Kern rechnet die Quelle g am Steg je Note genau aus der Frequenzantwort (Kraftstufe, Tiefpass der Fingerbreite, die Wege e^(−ikx_p) und e^(−ik(2L−x_p)), Neigung je Stärke) mit einer FFT über rund zwei Umläufe — wie Prüfstand und Kandidat S200.** Grund: Zielgröße von 34k-7d ist die Übereinstimmung mit dem Prüfstand; mit genauem g trug die Quelle im Kandidaten keinen eigenen Fehler (Gitarre ≤ 0,005 LU), eine Nachbildung im Takt (b) könnte nur Fehler dazugeben; die FFT je Note kostet gegen Sekunden Ton mit Hunderten Moden wenig, `kern::fft()` liegt in `InstrumentCore`. Für beide gilt: der Puls trägt die genauen Wege, die Schleife das Umlauf-Filter (≤ 3 Cent, UL2 a). Verworfen: b (Verzögerungen mit Thiran-Glied, Fingerbreite als FIR, Neigung als Filter im Takt; nicht gerechnet). Kippt, wenn 34k-8 eine Quelle braucht, die sich während der Note stetig ändert (nicht nur neu einsetzt wie Legato oder Hammer-on).

ZB1 a und ZU1 a auf „alle nach Empfehlung“, mit der Schnittfolge 34k-7d-1 bis -6 (unter dem Teilplan 34k-7).

✅ **ZW1 a** (Patrik S222; **34k-7d-1**, Herleitung §12 „Zupfort der Gitarre“): **Gitarre, Laute und Oud zupfen 16 cm vom Steg, für alle Bünde gleich (die Hand bleibt stehen); die Kappe auf die halbe Saite entfällt.** Grundlage (`../modellierung/gezupfte-saite/proben/2026-11-26_S222_zupfort_gitarre.py` mit den Ausgaben `…_iowa_mf.txt`, `…_iowa_hoch_*.txt`, `…_laut_*.txt`): das Teiltonprofil je Ton gegen die Iowa-Gitarre (mf, Teiltöne 1–12, ohne den Pegel je Ton; Median des Betrags) bis Bund 12 bei 16 / 10 / 6,5 cm 6,72 / 7,94 / 7,88 dB, in jeder Gruppe der Teiltöne 16 cm vorn, der Grundton im Mittel −2,8 / −6,1 / −8,9 dB; Bünde 13–19 bei 16 cm mit Kappe 22,72 dB (gerade Teiltöne −29 … −35 dB, in der Mitte gezupft), fest 8,06 dB. Lautheit nach KQ34 a: 16 cm mit Kappe 12,7 / 12,7 / 1,7 LU, fest 13,0 / 12,7 / 1,7, 10 cm 12,3 / 12,3 / 1,6, 6,5 cm 10,5 / 10,5 / 1,5 — alle im Band der TU-Gitarre (KQ36 a). Pavlidous Auslenkung (GZ6 a) ist am selben Ort gemessen. Laute und Oud spielen bis Bund 12, dort wirkt die Kappe nie. Quellen der Stellen: Traube 2004 Abschn. 3.1, 3.3, PDF-S. 156; Scherrer 2013 Abb. 3.7, Tab. 2.1. Verworfen: b (16 cm, über Bund 12 höchstens die halbe Saite), c (6,5 cm fest nach Sor und Biberian). Kippt, wenn eine zweite Aufnahme mit bekannter Spielweise (TU Berlin, Mores) ein Profil zeigt, das näher bei 6,5 oder 10 cm liegt — dann neu, oder die Stelle als Wert je Spielweise (34k-8).

✅ **SR1 a** (Patrik S223; **34k-7d-4**, Herleitung §2 „Zwei Polarisationen“): **Für einen Körper ohne Richtung am Steg (alle `bridgeAngle` 0 — jeder Körper aus einem Bauplan: Harfe, Cembalo, Laute, Oud) setzt der Bau des `Steg` aus der Körperdatei (`InstrumentBody`) die Annahme nach Scherrer ein, wie `richtung_von` des Prüfstands (tan θ_n = 0,03 · (f / 200 Hz)^1,27 auf 0,03 … 0,4, Vorzeichen je Mode mit SplitMix64, Samen 7); die Körperdatei bleibt, wie sie gerechnet ist. Kern, App, Stimmer und Tests nehmen den `Steg` aus derselben Funktion.** Grundlage (`../modellierung/gezupfte-saite/proben/2026-11-26_S223_bezug_ohne_richtung.py`, Vergleich `…_vergleiche_bezug.py`, Ergebnis `…_bezug_ohne_richtung.txt`): der Bezug `werte/zupf_pruefstand.txt` rechnet mit der Annahme; mit Winkel 0 Gitarre unverändert, Cembalo 84 Pegel spät bis 16,6 dB und 1–2 s +2,6 dB, Harfe 91 +1,35 dB, Laute und Oud bis 6,9 Cent und spät bis 5,6 dB, Lautheit höchstens 0,10 LU — ohne Annahme schwingt die zweite Polarisation fast ungedämpft. Verworfen: b (`InstrumentBodyTool` schreibt die Annahme als `bridgeAngle` mit Kennzeichnung in die Körper — dieselben Zahlen, aber eine Annahme in den Daten der FE-Rechnung und eine neue Fassung der Körperdatei), c (die Annahme entfällt, Bezug mit Winkel 0 — ein Steg bewegt sich in beiden Richtungen). Kippt auf b, wenn andere Leser der Körperdatei (Abstrahlung parallel zur Decke, Werkzeuge) die Richtung brauchen; entfällt für einen Körper, sobald eine gemessene Richtung in seiner Datei steht (wie die Gitarre nach Elie).

✅ **TK1 a** (Patrik S220; **34k-7c**): **Cembalo T73 verfehlt im Messfall UL2 a (3,27 Cent, die Probe S205 0,01 Cent) — die Ursache wird gesucht, bevor die Taste als Ausnahme gilt.** Grund: an derselben Taste mit demselben Start trifft die Vorlage die Schwelle; eine Ausnahme ohne Ursache verdeckte einen Fehler der Übertragung, der auch andere Tasten treffen kann. Verworfen: b (T73 als bekannte Abweichung festhalten).

✅ **TK2 a** (Patrik S220; **34k-7c**, Ursache aus TK1 a): **Beide Behebungen am Entwurf — bei Rangverlust im Vertrauensbereich der Schritt kleinster Norm mit α = 0, und die Logit-Parameter der Kette nach jedem angenommenen Schritt auf ±40 gehalten; danach der Messfall über alle Tasten und Saiten neu.** Grund: läuft ein Winkel der Kette nach π, ist die Spalte von J exakt null; scipy rechnet dort α < 0 und einen Schritt, der das Modell vergrößert, und der Abbruch nach xtol hängt am gesättigten Parameter — Python kommt nur über Rundungen von LAPACK durch; beide Stellen greifen nur, wenn ein Parameter an seiner Grenze steht. Verworfen: b (nur eine der beiden Behebungen). Ergebnis S220: Harfe 80 / 80, Cembalo 61 / 61 (T73 0,01 Cent / 0,21 %), 17 Tasten mit anderer Kette, alle in der Schwelle; Gitarre, Laute, Oud mit TK2 a stehen aus (S221). Kippt, wenn eine Taste oder Saite mit TK2 a die Schwelle verliert, die sie vorher hielt.

✅ **KQ75 a** (Patrik S220; **bestätigt GZ10 a nach dem Nachlesen**): **GZ10 a bleibt. Penttinen 2006 Abb. 6 ist am Seitenbild nachgelesen (Mediane, C2 / C3 / C4 / C5 / C6: pp → ff 1,25 / 2,5 / 2,0 / 1,25 / 1,25, pp → mf 1,0 / 1,25 / 2,0 / 1,0 / 0,5, mf → ff 0,5 / 1,5 / 1,5 / 1,0 / 0,25 dB). Das Modell (Kraft nach GZ13 a / KQ30 a, Stoß 3,8 mN s, Saite bis pp 1 dB) trifft den Hörtest mit 0,59 dB rms gegen die Mediane und 0,33 dB gegen Tab. 1 (pp → ff 1,68, pp → mf 1,08, mf → ff 0,98), innerhalb seiner Streuung. Der Hörtest trennt Stoß und Saite nicht; die 1 dB der Saite bis pp bleiben Annahme, die Stoßstärke bleibt an den VCSL-Aufnahmen geeicht.** Befund (`../modellierung/gezupfte-saite/proben/2026-11-24_S220_cembalo_stoss_hoertest.py`): über Saite je Stufe 0,5 / 0,75 / 1,0 dB und Stoß 0–5 mN s liegt jede sinnvolle Fassung gegen die Mediane bei 0,53–0,59 dB; der Stoß klingt in unter 0,2 s ab, im Ausklang (auf den die Hörer achteten) misst das Modell nur die Saite; Penttinen misst an der Saite bei 0,4 s pp → ff Teilton 1 / 2 / 3 an C4 5 / 3 / 0,5 dB, an C6 0 / 2 / 1 dB — ein Gesetz je Taste ist daraus nicht mit Belegkraft zu rechnen (Absolutwerte nur an zwei Tasten, Abb. 3 relativ mit unklarem Bezug). Verworfen: b (Saite bis pp 2 dB nach Chadefaux 2013, G4 langsam gegen schnell: 0,70 / 0,49 dB rms, und „langsam“ ist nicht als pp belegt). Kippt auf b, wenn eine Quelle die Kraft am Kiel bei pp und ff misst; dann den Stoß an den VCSL-Aufnahmen neu prüfen.

✅ **KQ74 a** (Patrik S219; ändert KQ57 a, KQ70 a ruht): **Besaitet gelten als Schwelle die Lagen der Harfe ab S(1,2) (257 / 334 / 430 / 555 / 697 Hz, 7 %). Das Paar 187 / 207 Hz ist Bericht mit Wert und dem Vermerk „bei Bell ohne trennbare Gipfel“; der Gipfel des Modells bei 217 Hz am Messort 17 ist bekannte Abweichung im Fall „Luftraum die mitgelieferte Harfe“. Die Maße, Massen und Verbindungen von Säule, Hals und Sockel gehen als Auftrag `2026-11-24_36_harfe_rahmen_masse` an die Werkbank; KQ70 a ruht bis zu deren Bericht und wird dann mit der Rahmenprobe aus S219 wieder aufgenommen.** Befund (Harfenbericht §27): Bell liest 187 / 207 Hz an flachen Profilen mit Güte „indeterminate“ (Abb. 7.8 / 7.9), am Messort 17 steht besaitet kein Gipfel (Abb. 7.6); das Modell, gemessen wie bei Bell, zeigt 169–171 / 214–218 Hz; verworfen sind Ablesefehler, Vorspannung des Rahmens durch den Saitenzug (unter 3 Hz) und die Wölbung des Bodens (KQ73 a); die Rahmenprobe entscheidet mit angenommenen Querschnitten nichts. Verworfen: b (KQ70 a schließen), c (Schwelle lassen).

✅ **KQ73 a** (Patrik S219; erledigt, gekippt): **Die Wölbung des Bodens unter dem Saitenzug wird als Probe gerechnet (Platte mit großer Auslenkung, Ränder in der Ebene frei und gehalten als Grenzfälle).** Befund (Harfenbericht §27.6, `proben/2026-11-24_S219_harfe_woelbung.py`): Wölbung 3,4 mm; das verdrängte Volumen der ersten Mode bleibt in jedem Fall auf 1 % gleich, mit frei oder nur längs gehaltenen Rändern ändert sich keine Mode um mehr als 1 Hz, mit ganz gehaltenen steigt die erste um 16 % und der Abstand des Paars wächst — die Wölbung ist als Ursache verworfen. Verworfen: b (Schwelle sofort ändern), c (Maße des Rahmens über die Werkbank).

✅ **KQ72 a** (Patrik S218; nach KQ71 a, löst KQ63 a und KQ64 a ab): **Die Öffnungen der Harfe werden aus ihrer Geometrie gerechnet, als Kolben in einer Wand: Massenmatrix (die Öffnungen belasten sich gegenseitig, κ = 1), Strahlungswiderstand als Matrix R_ij = ρ c k² / 2π · sin(k d) / (k d), Schall an den Hörorten √2 · ρ / 4π (die Leistung passt zum Widerstand), der innere Verlust aus Bells Güten mit nur Öffnung 6 offen abzüglich deren Abstrahlung; Mündungskorrektur und die Güten 4,4 / 28 entfallen. Bauplan-Fassung 11 (`cavity.openingModel`, `cavity.surroundings`, `cavity.innerLoss`, je Öffnung `length`), Körperdatei Fassung 8 (`airLoss` je Mode), `InstrumentBody` 0.28.0; der Wächter prüft alle 36 Lagen aus Bell Kap. 4; alle Körper mit Bauplan neu.** Befund (Probe S218): Wand gegen frei gegen gebaut S215 — 36 Lagen rms 6,1 / 7,4 / 7,2 %, Helmholtz 192 / 209 / 190 Hz (angepasst), Paar besaitet −6,8 / +5,1 gegen −1,8 / +8,0 gegen −7,5 / +4,1 %, Abstrahlung 33–22 größte 14,6 / 12,9 / 10,9 dB, Lautheit 50–62 11,6 / 11,6 / 2,6 gegen 13,1 / 12,3 / 3,4 gegen 13,8 / 13,8 / 2,9 LU; nur die Wand hält GZ14 a; Bell: Helmholtz am freien Kasten 187, im Sand 190 Hz. Verworfen: b (freier Raum — trifft das Paar besser, verfehlt aber die Helmholtz-Mode um 10 %), c (nicht bauen — ließe drei angepasste Werte stehen). Kippt, wenn die Abstrahlung an den Saiten 33–22 Vorrang vor der Lautheit bekommt. Harfenbericht §26.

✅ **KQ71 a** (Patrik S217; löst KQ69 a ab): **Für die Luft der Öffnungen als Matrizen aus der Geometrie — die gegenseitige Belastung der Pfropfen (Massenmatrix, KQ69 a) und der Strahlungswiderstand der Öffnungen (R_ij = ρ c k² / 2π · sin(k d) / (k d), dazu der innere Verlust aus Bells Güten mit nur Öffnung 6 offen) — werden zuerst die Zielgrößen gerechnet: ein Körper aus der Probe, daran Lautheit der Tasten 50–62 mit Mitschwingen, Abstrahlung 33–22, Güten ohne und mit Saiten, Wächter gegen alle 36 Werte aus Bell Kap. 4. Danach wird der Bau vorgelegt; bis dahin gelten KQ63 a und KQ64 a.** Befund S217: die Strahlungsmatrix gibt die Güten der sechs Luftmoden mit allen Öffnungen ohne angepassten Wert (Wand 4,5 / 5,9 / 7,7 / 13 / 21 / 27, freier Raum 6,3 / 8,4 / 11 / 18 / 25 / 31 gegen Bell 6 / 6 / 7 / 10 / 15 / 30; gebaut mit zwei angepassten Güten 5 / 6 / 8 / 13 / 17 / 21); die Güten der Plattenmoden hebt sie wenig (besaitet 25 / 25 / 16–21 / 23–30 gegen Bell 32 / 27 / 28 / 19). Verworfen: b (jetzt bauen — S215 zeigte, dass eine an den Luftmoden bessere Probe den Sprung am gebauten Körper verschlechtern kann, 10,9 → 13,8 LU), c (auf Dauer Probe — stellt belegte Physik gegen Anpassungen zurück). Kippt zu b, wenn die Matrizen unabhängig vom Ergebnis an Lautheit und Abstrahlung ins Modell sollen. Harfenbericht §24.2, §25.6.

✅ **KQ70 a** (Patrik S217; hebt die Prämisse von KQ58 a, KQ67 a und KQ68 a auf): **Die Prämisse „das Modell koppelt den Boden zu stark an die Luft“ entfällt; die Kopplung Platte–Luft bleibt wie gerechnet. Die Harfe wird in zwei Zuständen gemessen: ohne Saiten gegen Bell Kasten G in der Harfe ohne Saiten (170 / 216 / 249 / 316 Hz), besaitet weiter nach KQ57 a. Die Abweichung des Paars im besaiteten Zustand (187 / 207 Hz) heißt „Wirkung des Besaitens“ und wird als eigene Frage geprüft — der Rahmen der Harfe aus Sockel, Säule und Hals mit mehr als einem Freiheitsgrad (Le Carrou 2007), die Saiten zwischen Boden und Hals —, als Probe, mit Lautheit, Abstrahlung und Profilen vor jeder Vorlage. Die Harfe bleibt gebaut.** Befund S217: (1) die Schale als elastischer Ring ändert das eingeschlossene Volumen nicht (Versteifung 48 / 51 / 54 Hz wie starr, Helmholtz × 0,995–0,999) — KQ68 a ist in der gefragten Form gekippt; (2) die Balken-Probe aus S216 hatte eine rund 10⁴-mal zu kleine Steife, ihr Befund ist hinfällig; (3) mit der Biegesteife aus der Geometrie biegt sich der lose Kasten bei 220–260 Hz und drückt die obere Mode des Paars nach unten (ohne Saiten 167 / 197–199 Hz, „Versteifung“ nach Bells Rechnung 30–32 statt 46 Hz; Bell Kasten A 176 / 201 Hz, 26 Hz), dazu eine Mode bei 257–289 Hz (Bell S(1,2)′ 284 Hz); (4) der gehaltene Kasten ohne Saiten trifft Bell ohne angepassten Wert an der Luft: 165,2 / 210,8 / 232,9 / 319,7 Hz (−2,8 / −2,4 / −6,5 / +1,2 %), Abstand des Paars 46 gegen 46 Hz. Offen: beim Besaiten steigt bei Bell die untere Mode um 17 Hz und die obere fällt um 9 Hz — Federn gegen einen festen Punkt können das nicht; ein Hals als ein Schwinger auch nicht; ohne Saiten tragen S(1,2)–S(1,5) im Modell 15–50 % Luft, bei Bell haben sie die Güte der Platte allein. Verworfen: b (Kopplung × 0,6 im Bauplan — verstellt den Zustand ohne Saiten, der stimmt, und hob in S213 den größten Sprung auf 19,0 LU), c (Kasten als Balken in `InstrumentBody` — betrifft den losen Kasten; wie er in der Harfe gehalten ist, ist nicht gemessen). Kippt, wenn kein plausibler Rahmen das Paar auf 187 / 207 Hz bringt — dann b als bekannte Abweichung neu vorlegen. Harfenbericht §25.

✅ **KQ69 a** (Patrik S216): **Die gegenseitige Belastung der Luftpfropfen der Öffnungen (Massenmatrix aus der Geometrie statt der angepassten Mündung) bleibt Probe und wird zusammen mit dem Ergebnis von KQ68 vorgelegt; bis dahin gilt KQ64 a.** Befund (Rohr allein gegen Bell Kap. 4): Helmholtz mit allen Öffnungen ohne Anpassung 192 Hz (Bell 190), Öffnung 6 allein 72 statt 69 Hz (Bell 75), A2 von −12 auf −4 %, die 36 Werte der Abb. 4.8, 4.11–4.15 rms 7,2 → 6,0 %; A3–A5 bleiben 6–9 % zu tief, der Luftanteil der Plattenmoden ändert sich nicht. Verworfen: b (jetzt bauen, Bauplan-Fassung 11) — das Luftmodell wäre nur halb erklärt und die Harfe an ihren Zielen nicht zu beurteilen, solange die Kopplung falsch ist. Kippt, wenn die Harfe ruhen soll (dann vor der Pause bauen). Harfenbericht §24.2.

✅ **KQ68 a** (Patrik S216; folgt auf KQ67 a): **Die Schale der Harfe wird als elastisches Bauteil geprüft, zuerst als Probe: der Querschnitt als Ring aus Boden und Schale, gegen Bells Hologramme (Phase von Boden und Rückseite je Mode, Abb. 5.3–5.19), die Versteifung der ersten Bodenmode 25 Hz, das Paar lose / in der Harfe (153 / 200 gegen 170 / 216 Hz) und die Güten. Erst danach wird vorgelegt, ob und wie die Schale in `InstrumentBody` kommt. Die Harfe bleibt gebaut, 13,8 LU bleibt bekannte Abweichung.** Befund S216: der Luftanteil der Plattenmoden (21–69 %) hängt an der Stärke der Kopplung Platte–Luft, nicht an der Lage der Luftmoden (alle Rohrmoden × 1,05 … 1,15: Luftanteil 0,30–0,53, Q 14–22; Kopplung × 0,6: Q von S(1,2)–S(1,5) 30 / 28 / 22 / 23 gegen Bell besaitet 32 / 27 / 28 / 19, Paar 183 / 212 Hz); Bells Paar am losen Kasten A gibt 25 Hz Versteifung (22–27), das Modell 54 Hz (ein Druck) und 42 Hz (Rohr). Nach Bell Kap. 5 bewegt sich die Rückseite in jeder Mode, bei S(1,1)+ Boden und Rückseite beide nach innen; `InstrumentBody` rechnet die Wände starr. Verworfen als Ursache: Lage der Luftmoden, Kasten als Balken mit starren Querschnitten (Rückstoß hebt die obere Mode auf 235–270 Hz, Versteifung 46–51 Hz). Verworfen: b (Faktor für die Kopplung im Bauplan — stellt einen Wert ohne Ursache ein, Lautheit, Abstrahlung und Profile dafür nicht gerechnet, erklärt die Verschiebung beim Einbauen und Besaiten nicht), c (Harfe ruhen lassen). Kippt zu b, wenn die Probe zeigt, dass eine elastische Schale die Kopplung nicht schwächt. Harfenbericht §24.

✅ **KQ67 a** (Patrik S215; bestätigt KQ63 a): **Die Güten der Luft bleiben 4,4 / 28 (KQ63 a); der größte Sprung der Harfe 13,8 LU an den Tasten 50–62 und die zu niedrigen Güten ihrer Plattenmoden sind bekannte Abweichung. Als Nächstes klärt eine eigene Sitzung mit Fable, warum die Plattenmoden S(1,1)–S(1,5) 21–69 % Luft tragen (Längsmoden mit allen Öffnungen 6–13 % zu tief, Versteifung der ersten Bodenmode 36,5 statt 19–25 Hz, Form der ersten Mode), mit den Güten der Harfenmoden ohne und mit Saiten als Zielgröße.** Grund: eine Güte 13,4 (b) träfe Lautheit (10,5 LU) und Güten der Harfe besser, weil sie die Luft zu schwach dämpft — sie gliche zu viel Luft in den Plattenmoden aus und läge an der Luft allein bei Faktor 1,9 (Helmholtz Q 13 statt 6). Verworfen: b. Kippt, wenn die Sitzung zeigt, dass die Luft in den Plattenmoden richtig ist. Harfenbericht §23.5, §23.6.

✅ **KQ49 a** (Patrik S210; ergänzt KQ46 a): **Die Energie der Federn zählt in der Dämpfung mit, jede Feder trägt einen eigenen Verlustfaktor `lossFactor` (immer geschrieben, 0 = ohne Verlust): η = (1 − Anteil der Federn) · η der Platte + Σ Anteil je Feder · η der Feder. Die Harfe schreibt 0 mit der Quelle „Annahme, wie die Probe S209“.** Befund: die Anteile der Verformungsenergie waren auf die Werkstoffe normiert — ohne die Regel trügen die Federn stillschweigend den Verlust des Holzes, die Probe aus S209 rechnete sie ohne Verlust. Am Kandidaten mit Saiten (Faktor 0,2) gegen alle Zielgrößen: Lagen gleich; η der Moden 170,8 / 225,3 / 261,6 Hz a 0,0521 / 0,0801 / 0,0251, b 0,0559 / 0,0826 / 0,0268, größter Unterschied über 129 Moden 7,2 % (0,6 dB an der Spitze); Lautheit 38–104 a 17,8 / 10,5 / 2,8 LU, b 17,6 / 10,7 / 2,7 LU, an einer Taste höchstens 0,5 LU (48). Verworfen: b (die Federn zählen in der Dämpfung nicht — die Annahme „die Saite verliert wie das Holz“ stünde nirgends). Kippt auf einen Wert größer 0, wenn eine Messung der Güte besaiteter Moden vorliegt (Bell nennt für S(1,1)− besaitet keine). **Gebaut S210 mit KQ46 a:** `.lvinstbodyplan` Fassung 9, `Punktfeder`, `setzePunktfedern`, `Auswertung::setzePunktfedern`, `ModeImKoerper::federAnteil`; `InstrumentBody` 0.25.0; Test „KQ46 Punktfedern“ (gegen die Reihe nach Navier 0,1 %, gegen den Modenraum mit 60 Moden 0,03 %); sieben Baupläne und fünf Schreiber auf Fassung 9, die Körper neu gerechnet. Der Bauplan der Harfe hat noch keine Federn (KQ47 a steht aus). Bericht `../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §18, Probe `../modellierung/gezupfte-saite/proben/2026-11-20_S210_kq49_federverlust.py`.

✅ **KQ46 a** (Patrik S209): **Der Körper der Harfe ist die besaitete Harfe. Der Bauplan bekommt Federn an Punkten — neue Fassung 9, Block `pointSprings` (je Feder Ort, Steife in N/m, Quelle), immer geschrieben, bei allen anderen Körpern leer. Die Harfe trägt je Saite eine Feder an ihrer Öse: Längssteife E·A/L mal einem Faktor für Winkel und Nachgiebigkeit des Rahmens, der Faktor an Bells Verschiebung von S(1,1)− beim Besaiten (170 → 187 Hz) gestellt und an S(1,2) gegengeprüft. Geprüft wird ohne Federn gegen Bells Harfe ohne Saiten, mit Federn gegen die besaitete. Alle acht Körper werden neu gerechnet.** Befund (Federn im Modenraum auf den gerechneten Körper gesetzt; Nylon 4, Darm 5,5, Stahlkern 200 GPa, Durchmesser und Längen Bell Tab. 2.1 / 2.2; Summe E·A/L 1,8 · 10⁶ N/m): mit dem Faktor 0,2 steigt am heutigen Körper S(1,1)− um 9 % und S(1,2) um 4 % — Bell misst +10 % und +3 %. Lautheit je Taste mit Saiten: heutiger Bauplan 19,2 / 14,1 / 2,9 LU, Kandidat nach KQ47 a **17,8 / 10,5 / 2,8 LU — die erste Fassung der Harfe, die alle drei Schwellen hält** (ohne Mitschwingen gemessen). Verworfen: b (Saiten bleiben draußen: größter Sprung 13,0 LU), c (Steife nachträglich im Modenraum: die Körperdatei stimmt dann nicht mehr mit ihrem Bauplan überein). Kippt auf einen Block nur für die Harfe, wenn die Federn bei Laute oder Oud nicht dasselbe bedeuten. Bericht `../modellierung/gezupfte-saite/2026-11-13_S197_harfe_gelesen.md` §17, Suchgerüst `../modellierung/gezupfte-saite/proben/2026-11-19_S209_harfe_suche/`.

✅ **KQ45 a** (Patrik S209; **schliesst die Suche nach KQ38 a ab**): **Gebaut wird die Fassung mit den besten Lagen — Decke 2,23 mm, Steg 17 cm × 25,8 × 8,6 mm (650 kg/m³, E 10 GPa, 24,5 g), Balkenenden 2 mm vor dem Rand, Hohlraum 20,61 l, Mündung 1,52, Rosetten 11,4 g + 2 × 1 g. Der Test hält die Lagen des Instruments ohne Aufnehmer und Ziel 3; was verfehlt ist, steht als bekannte Abweichung mit Wert.** Gegen alle Zielgrößen (Bauplan vor S209 → neu): Lagen wie gemessen gegen 139 / 203 / 280 / 362 / 385 Hz +0,3 / +0,2 / −1,9 / −3,0 / +1,1 % (am Netz für 5 kHz; die Suche lief am Netz für 2,5 kHz und zeigte dort −0,9 % an der gegengleichen Mode); Ziel 3 2,72 → 1,51; Basschor T2 / T3 unter dem Maximum unter 300 Hz −32,8 / −11,6 → −9,4 / −0,3 dB (1 m, Terzmittel); **Ziel 2 verfehlt**: am Antwortpunkt 3 gewinnt die gegengleiche Mode um 4,4 dB (Rastermittel, mit Aufnehmer; vor S209 −16,9 dB), gemessen ist sie 0,37 der Spitze um 203 Hz, im Modell 1,65; **KQ36 a verfehlt**: Spanne / größter / mittlerer Sprung 18,8 / 14,7 / 1,8 → 19,8 / 14,3 / 2,9 LU (ab Taste 41: 16,6 / 13,5 / 2,7) — KQ37 a bleibt. Verworfen: b (erst die Höhen von Abb. 6 als Ziele — die Mittelung der Kurve ist nicht bekannt, ihr Tal zwischen 139 und 203 Hz ist mit ihrer eigenen Dämpfung keine Modensumme), c (leichter Steg 14 g: KQ36 a 18,7 / 12,8 LU, Lage −3,7 % — an die Schwelle statt an die Messung angepasst). Ziel 2 und KQ36 a trägt die Körperdatei nicht (kein Antwortpunkt, keine Saite): sie stehen im Bericht `../modellierung/gezupfte-saite/2026-11-19_S207_spruenge_je_taste.md` §12, nicht im Test.

✅ **KQ44 a** (Patrik S209): **Die Rosetten in den drei Löchern (Abdallah 2026 Abb. 4, 5 a) gehen als Masse am Lochrand in den Bauplan, als Annahme mit Vermerk.** Umsetzung ohne neue Dateifassung: je Loch ein weicher Ring als Leiste (5 mm breit, Mitte 3 mm außerhalb des Lochrands, 10 mm hoch, E 10 MPa, Dichte aus der Masse) — gegen die genaue Massenänderung im Modenraum auf rund 1 % (275,0 gegen 271–276 Hz), über die Netze 450 / 2500 / 5000 Hz stabil. Wirkung: nur die Mode am Lochfeld (302 → 275 Hz), Luft- und Deckenmode ≤ 0,2 %. Nach KQ36 a kostet die Masse rund 2 LU Spanne.

✅ **KQ43 a** (Patrik S209): **Die Lagen werden mit dem Aufnehmer der Messung verglichen (5,8 g fest am Antwortpunkt 3, der Hammer wandert); der Bauplan beschreibt das Instrument ohne ihn.** Gerechnet als Massenänderung im Modenraum (K = diag(ω²), M = I + m φφᵀ, dicht); eine 1-cm-Leiste im Bauplan taugt dafür nicht (die Leisten werden an Quadraturpunkten abgetastet: −13 % statt −5,5 %). Wirkung am Körper: 141,1 → 139,4, 214,1 → 203,4, 274,7 → 274,7, 372,5 → 351,2, 389,2 → 389,1 Hz.

✅ **KQ42 a** (Patrik S209; **fasst KQ10 a neu**): **Lesart der gemessenen Kurve (Abdallah 2026 Abb. 6): 139 Hz ist die Mode der Luft, 203 Hz die erste Deckenmode; die Spitze bei 101 Hz hat im Modell kein Gegenstück (vermutlich Hals oder Schale, die hier starr sind). Gebaut wird Familie R: die Balken reichen bis 2 mm vor den Rand, der Steg trägt Masse und Steife.** Befund der Suche: einzelne Stellgrößen (Dicke, Halt am Rand, Balkenhöhen, Hohlraum, Mündung, Lage der Stegpunkte) erfüllen Ziel 2 und 3 nicht; mit den Balken wie in Abb. 3 gezeichnet (10–16,5 mm vor dem Rand) liegt der Bauch der strahlenden Mode im Lochfeld (Steg 0,10 des Bauchs), mit den Enden am Rand am Steg (0,73 / 0,79); der Abstand muss bei 2 mm bleiben (3 mm: 128 / 197 Hz). Die alte Lesart (101 Hz Luft, 203 Hz Decke) traf die Lagen, hatte aber zwischen 101 und 203 Hz eine gegengleiche Mode ohne Gegenstück in der Messung und die gemessene Spitze bei 139 Hz ohne Mode.

✅ **KQ41 a** (Patrik S208; **ändert Ziel 2 von KQ38 a**): **Die Hauptspitze der Abstrahlung der Oud liegt beim 1,5- bis 2,0-Fachen der Luftmode (Değirmenli 2017 Abb. 7: 1,7; Değirmenli 2023 Abb. 3: 1,5–1,6; Abdallah 2026 am Antwortpunkt 3: 2,0). Die Formen und Güten aus Değirmenli 2023 Abb. 1 prüft die Suche nur als Bericht, nicht als Schwelle.** Befund (Bericht `../modellierung/gezupfte-saite/2026-11-19_S207_spruenge_je_taste.md` §11, Bericht der Werkbank `../modellierung/gezupfte-saite/recherche/2026-11-19_S208_oud_basschor_werkbank.md`, in S208 am Seitenbild gegengelesen): an zwei lauten türkischen Ouds erste Spitze 125–140 Hz, Maximum 170–225 Hz; der zweite und dritte Teilton des Basschors liegen 0 … −8 dB unter dem Maximum unter 300 Hz, im Modell −27 / −18 dB; Modell heute beim 2,7-Fachen. Gemessene Moden der besaiteten Oud 117,7 / 177,8 / 206,0 Hz mit Q 25 / 64 / 15, im Modell 99,9 / 168,6 / 207,8 Hz mit Q 26 / 32 / 35 — Folge der Formen gleich, Lage der Bäuche gegen den Steg am Bild nicht bezifferbar. Grund: ein Ziel schließt die gemessenen Instrumente ein; mit 1,7 als Untergrenze fiele eine von zwei Messungen der Abstrahlung heraus. Pegel je Chor an einer echten Oud ist nicht gefunden (nur in der Dissertation Değirmenli 2018, nicht erreicht); KQ37 a bleibt, der Zug nach D'Addario EJ95A (C2 35,6 N gegen F2-Chor 56,0 N, −3,9 dB) bestätigt die erste Ursache. Verworfen: b (Band 1,7–2,0, Değirmenli 2023 nur im Bericht). Kippt, wenn eine arabische Messung oder die Dissertation 2018 ein Verhältnis über 1,7 belegt, oder wenn eine Fassung nur bei 1,5–1,6 liegt und dort Ziel 1 verfehlt — dann gilt Abdallahs Oud vor den türkischen.

✅ **KQ39 a** (Patrik S208; **ergänzt KQ36 a**): **KQ34 a misst die Harfe so, wie der Kern sie rechnet — mit Mitschwingen nach GZ8 a (Oktaven, Quinten, Quarten der gespielten Saite, die Pedale der übrigen in der Mitte, die gespielte Saite mit ihrem Pedal), am Prüfkörper mit 16 Hörorten.** Probe `../modellierung/gezupfte-saite/proben/2026-11-19_S208_harfe_mitschwingen_16.py` (rund 50 s je Taste, 67 Tasten rund 1 h). Heute: Spanne / größter / mittlerer Sprung 20,9 / 13,3 (44 / 47) / 3,1 LU (eine Saite allein 20,0 / 12,8 (43 / 47) / 3,0). Grund: die Schwelle gilt für den Ton, der erklingt, und die Aufnahmen tragen alle Saiten frei; das Mitschwingen ändert einzelne Tasten bis 2 LU (Bericht S207 §10). Verworfen: b (eine Saite allein, das Mitschwingen erst am gebauten Kern in 34k-7). Kippt, wenn eine Suche viele Läufe hintereinander braucht — dann sucht sie mit einer Saite und prüft jede gewählte Fassung mit Mitschwingen.

✅ **KQ40 a** (Patrik S208; **erweitert KQ31-2 a**): **Der Harfenkörper bekommt zwei weitere Ziele — die Lage von S(1,1)− und S(1,1)+ gegen Bell in der Harfe ohne Saiten (170 / 216 Hz) auf 7 %, und die Abstrahlung an den Saiten 33–22 gegen Bell 1988 Abb. 7.23 (Schalldruck an der Öse bei der Frequenz der Saite, besaitete Harfe, Bezug Saite 26) auf 6 dB wie KQ19 a; Le Carrou, Gautier & Foltête 2007 Abb. 7 als Bericht. Danach wird die Ursache im Bauplan gesucht (Luftraum, Halt am Rand, Last aller 46 Saiten an den Ösen), mit Fable (Merkmal 3); dann Körper neu, KQ31-2 a und KQ34 a nach KQ39 a neu messen.** Heute: S(1,1)− 134 Hz (−21 %), S(1,1)+ 198 Hz (−8 %); an den Saiten 33–22 halten 4 von 12 (Modell − Bell −9,6 / −6,6 / +5,3 / +16,3 / +14,8 / +10,7 / −0,9 / 0 / +19,7 / +4,6 / −8,9 / −7,1 dB; Probe `…/proben/2026-11-19_S208_harfe_gegen_bell_723.py`, Bell und Le Carrou in S208 am Seitenbild gelesen). Grund: die Abweichung ist an zwei Harfen gemessen und betrifft ein Band (125–260 Hz, Tasten 45–62), nicht eine Taste; die Kraft der Metallsaiten ist es nicht (gegen die Darmsaiten 52–56 liegt das Metallregister im Modell 4,5 LU leiser, in der Aufnahme 1,6 LU lauter). Verworfen: b (bekannte Abweichung 13,3 / 3,1 LU im Test, Körper bleibt), die Güte der Mode S(1,2) senken (besaitet gemessen Q 32, Modell 34 — eine Anpassung gegen die Messung). Kippt auf b, wenn die Suche keinen Bauplan findet, der die zwei Ziele zusammen mit KQ31-2 a hält; dann bleibt die beste Fassung mit ihren Werten als bekannte Abweichung.

✅ **KQ38 a** (Patrik S207; **erweitert das Ziel von KQ10 a**): **Der Bauplan der Oud bekommt zwei weitere Ziele — am Antwortpunkt 3 von Abdallah ist die Spitze um 203 Hz die stärkste unter 400 Hz (gemessen +7 dB über 280 Hz), und die Hauptspitze der Abstrahlung liegt beim 1,7- bis 2,0-Fachen der Luftmode (Değirmenli 2017 Abb. 7: 1,7) — und wird über die benannten Annahmen neu gesucht (Deckendicke auch als Verlauf, Balkenenden, Stegleiste, Hohlraum, Rand). Die Suche beginnt, wenn der Bericht zu Auftrag `2026-11-19_35_oud_basschor_einzeltoene` gegengelesen ist; bringt er Formen oder eine Stegadmittanz einer arabischen Oud, kommen sie als Ziele dazu.** Befund (Bericht S207 §8, Bild `../modellierung/gezupfte-saite/proben/2026-11-19_S207_oud_formen.png`): die gleichsinnige Deckenmode 208,8 Hz hat ihren Bauch im mittleren Feld (y 19,5 cm, zwischen Balken 2 und 3) und am Steg 28 % ihres größten Werts; den Steg trägt die Mode 270 Hz (91 %). Am Antwortpunkt 3 rechnete S196 209 Hz 13 dB unter 276 Hz, gemessen liegt 203 Hz 7 dB über 280 Hz — rund 20 dB Abstand an dieser Größe; keine der in S196 gerechneten Fassungen (Decke 1,3–1,6 mm, Balken wie gezeichnet oder am Rand) macht die Spitze um 200 Hz zur stärksten. Der Bauplan trifft die Lagen (KQ10 a) und verfehlt, welche Mode der Steg treibt; der Basschor verliert dadurch Abstrahlung bei 130–250 Hz. Gemessene Formen gibt es in keiner der drei Quellen. Verworfen: b (sofort neu suchen: ein zweimal neu gefasster Bauplan zieht zweimal Körper, Test, Lautheit je Taste und Loops nach), c (Höhen als bekannte Abweichung: an zwei Messungen belegt und am Ton messbar). Kippt zu b, wenn der Bericht nichts zu Formen und Admittanz bringt.

✅ **KQ35 a** (Patrik S203): **Der Gitarrenkörper über 2,2 kHz und die zweite Stegrichtung werden an Elie gesetzt — ein eigener Schritt in 34k-6 vor 34k-7, mit Fable (Merkmal 2): die Dichte über 2,2 kHz nach Elie 2011 (Gitarre vom Gitarrenbauer, 0,022–0,029 / Hz; die gefitteten Moden bleiben, gezogene füllen die Lücke, gestellt am Terzpegel von g34), die Richtung je Mode nach Elies Messung (Kreuzterm je Band −9 … −19 dB, an T1 −15 dB) statt nach Scherrer; danach Körper neu, Lautheit je Taste, Sprünge (KQ34), Abklingen je Teilton (KQ32) und Stegkurven gegen Mores neu gemessen; die Wirkung auf alle Zielgrößen ist der erste Teil des Schritts.** Befund (S203): die Körperdatei hat in 2,5–5 kHz 26 Moden (0,0104 / Hz aus ESPRIT an g34) und setzt diese Dichte statistisch bis 20 kHz fort; Elie misst 0,019–0,029 / Hz und nennt fehlende Moden an Knotenlinien. Der Prüfstand nimmt tan θ = 0,03 (f / 200 Hz)^1,27 (Scherrer, Annahme): an T1 Yzy − Yzz −29 dB, Yyy − Yzz −58 dB gegen Elie −15,4 / −22,5 dB. Verworfen: b (beides als bekannte Abweichung). Kippt, wenn das Auffüllen die Stegkurven gegen Mores über 2,2 kHz verschlechtert (heute 2,2–2,8 dB) — dann nur die Richtung neu. Ohne Entscheid: Hörort 10 cm gegen 1 m bleibt offen — die TU-Gitarre (Kohno, 1,05 m, ohne Raum) ist um T1 5,9 LU lauter als ihr Mittel, das Modell flach; Mores g20 sagt das Gegenteil (T1 gegen A0 im Modell 10 dB zu stark); zwei fremde Gitarren widersprechen sich (`../modellierung/gezupfte-saite/proben/2026-11-17_S203_gitarre_form_gegen_tu.py`).

**Befund zu KQ35 a, erster Teil (S205, Bericht [../modellierung/gezupfte-saite/2026-11-18_S205_gitarre_kq35.md](../modellierung/gezupfte-saite/2026-11-18_S205_gitarre_kq35.md), Probe `../modellierung/gezupfte-saite/proben/2026-11-18_S205_gitarre_kq35.py`):** **Dichte:** aufgefüllt auf 0,027 / 0,026 / 0,0235 / Hz (2,2–3 / 3–4 / 4–5 kHz, 34–44 gezogene Moden zu den 31 gefitteten) und 0,0215 / Hz ab 5 kHz; die gezogenen Moden tragen am Steg einen um 9–13 dB kleineren Formwert² (die Moden, die ESPRIT verfehlt, sind die an den Messpunkten schwachen), ihre Abstrahlung ist je Band am Terzpegel gestellt (−8,7 / −29,7 / +0,4 dB gegen die gefitteten — in 3–4 kHz lässt die Messung keinen Platz). Stegkurven gegen Mores 2,5–5 kHz 2,60 statt 2,72 dB: der Kipppunkt greift nicht. Am Ton ändert das Auffüllen nichts Messbares (Lautheit ≤ 0,4 LU, Abfall ≤ 0,9 dB/s). **Richtung:** die Annahme nach Scherrer liegt über 1 kHz schon bei Elie (1–4 kHz −10,3 / −17,3 gegen −12,9 / −15,6 dB), unter 500 Hz 12–14 dB (Kreuzterm) und 15–31 dB (Yyy) zu tief. Mit |tan θ| je Band aus dem Kreuzterm stimmt der Kreuzterm auf 0–5 dB, Yyy liegt 5–17 dB zu tief (ein Winkel je Mode); mit zusätzlichen Moden nur parallel zur Decke stimmen beide auf 1–5 dB (Zahl, Lage, Dämpfung dieser Moden angenommen). Wirkung: Lautheit ≤ 1,1 LU, Spanne 12,7–12,9 LU, mittlerer Sprung 1,6–1,8 LU; das Abklingen ändert sich an 8–14 von 44 Tasten um mehr als 2 dB/s (bis +6,6 früh an Taste 47, +9,5 spät an Taste 48). Der größte Sprung liegt mit zwei Polarisationen bei 12,3 LU (heute) und 12,7 LU (nach Elie), über der Schwelle 12 LU von KQ34 a (Tasten 68 / 71). Der Körper im Repo ist unverändert. Entschieden mit KQ35-1 a und KQ35-2 a.

✅ **KQ35-1 a** (Patrik S205): **Der Körper der Gitarre bekommt je Mode |tan θ| aus dem Kreuzterm des Bands nach Elie 2012 Abb. 3.17 (Vorzeichen gezogen; unter 150 Hz die Werte von 150–300 Hz, über 4 kHz die von 1–4 kHz — Annahme) und dazu je Mode eine zweite, die den Steg nur parallel zur Decke bewegt: auf halbem Weg zur nächsten Mode, Dämpfung wie die Mode, ohne Abstrahlung, Formwert so, dass Yyy im Band stimmt.** Belegkraft: Kreuzterm und Yyy je Band gemessen (eine Gitarre, Yamaha CG131S, eine Saite, 183 Hz–4 kHz); Zahl, Lage und Dämpfung der Moden parallel zur Decke angenommen, im Körper vermerkt. Damit an der Saite 1 Kreuzterm auf 0–5 dB, Yyy auf 1–5 dB. Verworfen: b (nur |tan θ| je Band: Yyy 5–17 dB zu tief), c (Scherrer lassen: unter 500 Hz 12–31 dB daneben). Kippt zu b, wenn die doppelte Zahl der Stegmoden im Zeitbereichs-Kandidaten die Rechenzeit sprengt (nicht gemessen) oder eine Messung des Abklingens je Polarisation zeigt, dass Yyy für den Ton nicht zählt.

✅ **KQ35-2 a** (Patrik S205; **ändert das Format der Körperdatei**): **Die Richtung am Steg steht in der Körperdatei — je Mode ein Feld (Winkel gegen die Senkrechte), immer geschrieben; der statistische Teil trägt den Wert für die gezogenen Moden; neue Fassung der `.lvinstbody`. Körper aus einem Bauplan schreiben 0; `make_gitarre.py` schreibt die Werte mit Quelle in die Fit-Beschreibung.** Grund: die Richtung ist eine Eigenschaft des Körpers und steht neben Formwert und Dämpfung derselben Mode; Kern und Prüfstand lesen dieselbe Stelle. Verworfen: b (Tafel je Instrument in den Vorgaben des Kerns). Bau: mit dem neuen Körper der Gitarre in einem Zug — `make_gitarre.py` (Auffüllen nach dem Befund oben, Richtung, `kFitFassung` 4), neue Fassung der Körperdatei, alle Körper neu geschrieben, Wächter.

✅ **UL1 a** (Patrik S204): **Für Teiltonlage und Abklingen des Umlaufs zählt jeder Teilton bis 20 kHz, dessen Abklingrate am starren Steg höchstens 100 1/s beträgt (T60 mindestens 69 ms).** Die Grenze ist eine Setzung (aus Bericht 11 der Research-Werkbank), bis eine Quelle zur Hörbarkeit kurz klingender Teiltöne gelesen ist. Gerechnet über 295 Tasten (`../modellierung/gezupfte-saite/proben/2026-11-17_S204_dispersion_biquads.txt` und `…_jeder_teilton.txt`): mit dieser Zählung alle zählenden Teiltöne an 279 Tasten in 3 Cent, an 290 in 10 Cent, das Abklingen an 294 in der Schwelle; zählt jeder Teilton bis 20 kHz, sind es 240 / 284 Tasten, bis 160 Biquads, und an 14 Tasten fällt das Abklingen aus der Schwelle (schmale Pole). Verworfen: b (jeder Teilton bis 20 kHz), c (Teiltöne 1–12: am Cembalo blieben über 200 klingende Teiltöne ungeprüft). Kippt, wenn eine Quelle zeigt, dass die Lage so kurz klingender Teiltöne hörbar ist, oder wenn eine bessere Anpassung b ohne die Ausfälle erreicht.

✅ **UL2 a** (Patrik S204): **Der Umlauf des Kerns trifft den Umlauf des Prüfstands je zählendem Teilton (UL1 a): Lage ±3 Cent, Grundton ±0,1 Cent; Abklingzeit im Faktor 0,7923 … 1,4309 (HW1 a, auf den einzelnen Teilton übertragen — Abklingrate −30,1 … +26,2 %). Tasten, die das nicht erreichen, stehen mit Wert und Grund in einer Liste der Ausnahmen.** Belegkraft: das Abklingen aus Tolonen & Järveläinen 2000 Tab. 6 (gemessen am ganzen Ton, auf den Teilton übertragen); für die Lage einzelner Teiltöne ist **keine Hörschwelle an einer Quelle belegt** (Järveläinen, Välimäki & Karjalainen 2001, ARLO 2(3), nicht erreicht) — 3 Cent sind die sichere Seite und an 279 Tasten erreicht. Verworfen: b (±10 Cent: 290 statt 279 Tasten ohne Ausnahme, benennt 11 Tasten nur um). Kippt, wenn die Quelle deutlich weitere Schwellen belegt und die Ketten am Cembalo-Bass (bis 119 Biquads) die Rechenzeit sprengen.

✅ **UL3 a** (Patrik S204; **ersetzt den Pol 1. Ordnung des heutigen Kerns und die Optionen aus den Berichten 11 §8 und 21 §4**): **Der Umlauf-Filter ist eine Tafel je Taste, vorab gerechnet. Verlust: Vorgabe nach Bank & Välimäki 2003 ohne Glättung und ohne Hochfrequenz-Vorgabe, unter f₁ der Wert bei f₁; Gewicht des relativen Fehlers 1 / (g ln g)²; Nenner aus dem Gleichungsfehler, Zähler mit Nebenbedingung |H| ≤ 1; Ordnung 8, an Tasten mit zu wenig Teiltönen 4 oder 2. Steifheit: Kette von Biquad-Allpässen, Start nach Abel & Smith an der Zielphase, danach Anpassung an die zählenden Teiltöne mit begrenztem Polradius; die kleinste Zahl, die UL2 a hält. Stimmung: Thiran 1. Ordnung, am Grundton nachgestimmt (seit UL3-1 a: Thiran 5. Ordnung). Vor dem Bau wird der Verlauf der Beiwerte längs der Bünde für Bend und Slide geprüft.** Herleitung des Gewichts: τ = −1 / (f₀ ln g), also d ln τ = −dg / (g ln g) — die Quelle minimiert den Fehler in Sekunden, die Hörschwelle ist ein Faktor. Abweichungen von der Quelle: die Glättung ist für streuende Messwerte gedacht (S. 20, VII), unser Ziel ist glatt; die Überblendung der letzten fünf Sollpunkte in einen Ein-Pol ändert dort das Ziel und war die Ursache der Fehler an den obersten Teiltönen (mit ihr 50–70 %, ohne sie 1–6 % bei Ordnung 8). Zahlen (`…_S204_umlauf_umfang.txt`, starrer Steg): Verlust Ordnung 8 an allen 287 damit rechenbaren Tasten jeder Teilton bis 21,6 kHz in der Schwelle, größter Fehler 3,0 % Gitarre, 15 % Harfe, 5,7 % Cembalo, 12 % Laute, 2,9 % Oud; Steifheit: Biquads je Taste im Median 4 (Gitarre, Harfe, Laute), 7,5 (Oud), 12 (Cembalo), höchstens 119 (Cembalo 29); 32 gleiche Allpässe halten 10 Cent nur an 103 Tasten. Über 10 Cent bleiben Harfe 101–103 (21–28 Cent) und Cembalo 88 / 89 (10–11 Cent) — offen. Verworfen: b (32 gleiche Allpässe, ein Wert je Taste). Kippt für die gegriffenen Instrumente, wenn die Beiwerte der Kette längs der Bünde springen und sich nicht glätten lassen — dann eine gebundene Anpassung über alle Bünde einer Saite.

✅ **UL3-1 a** (Patrik S205; **ersetzt in UL3 a „Thiran 1. Ordnung“ und legt die gebundene Anpassung fest**): **Der gebrochene Verzug läuft über einen Thiran-Allpass 5. Ordnung, Verzug in [4,5 ; 5,5), am Grundton nachgestimmt. An den gegriffenen Instrumenten (Gitarre, Laute, Oud) wird die Kette je Saite gebunden angepasst: logit(Radius) und logit(Winkel) jedes Biquads als Polynom 3. Grades in der Lage längs der Saite, Stützstellen an den Bünden und auf halbem Weg, Beiwerte ab Grad 1 gedämpft; der Verlustfilter wird zwischen Nachbarbünden gemittelt.** Befund: die Reste aus S204 (Saite 1: 9–19 Cent, Saiten 4–2: 3,6–4,5 Cent) kommen vom Glied, nicht von der Kette — zwischen zwei Bünden ändert sich der Umlauf um mehrere Takte, der Bruchteil durchläuft seinen Bereich mehrfach, und Thiran 1 verschiebt die obersten zählenden Teiltöne je nach Bruchteil (halbe Streuung an Saite 1, Bund 19: 7,0 Cent; Thiran 3: 2,1; Thiran 4: 1,2 — `../modellierung/gezupfte-saite/proben/2026-11-18_S205_verzug_glied_boden.*`); eine längs der Saite glatte Kette gleicht das nicht aus. Die Ausreißer zwischen den Bünden (S204: 110 / 23 Cent; an der Laute 10 Cent, ein überzähliges Biquad kippt zwischen zwei Stützstellen um) beseitigen die Stützstellen auf halbem Weg und die Dämpfung (0,05 Cent je Einheit). Zahlen (`…/2026-11-18_S205_gebunden_stuetzstellen*`; geprüft an Bünden, auf halbem Weg und an sechs Stellen je Bund, die die Anpassung nicht kennt; größter Fehler der zählenden Teiltöne in Cent, Thiran 1 / 3 / 4 / 5): Gitarre Saite 6 1,7 / 0,3 / 0,3 / 0,3; 5 4,0 / 0,5 / 0,4 / 0,4; 4 7,0 / 1,9 / 1,3 / 0,6; 3 3,7 / 0,3 / 0,2 / 0,2; 2 7,0 / 1,4 / 0,5 / 0,3; 1 24,8 / 4,9 / 3,0 / 2,7; Oud über alle Chöre 4,4 / 0,8 / 0,5 / 0,5; Laute Chöre 1.2 bis 6 ≤ 2,1 / ≤ 0,7 / ≤ 0,7 / ≤ 0,7, Chor 7 12,7 / 4,1 / 3,4 / 2,0. Abklingen mit Thiran 3 bis 5 an diesen Saiten überall in der Schwelle; Lagrange 5 hält die Lage (Saite 1: 0,9 Cent), sein Betrag wirft das Abklingen aber aus der Schwelle (Saite 1: 113 von 153 Stellen, bis 123 %). Annahme: Laute und Oud über die Lagen 0–12. Verworfen: b (Thiran 4: Saite 1 an der Schwelle, Laute Chor 7 darüber), c (Thiran 3 mit zwei Ausnahmen), d (Thiran 1 mit einer Kette, die auch vom Bruchteil abhängt — nicht gerechnet). Laute Chor 1.1 (73 Hz: 8,5–11,6 Cent bei jedem Glied, Abklingen an 4 von 97 Stellen außerhalb) und Chor 8.1 (zählend bis 19,7 kHz: auch mit Thiran 5 10,5 Cent) siehe „UL3-1 a nachgeschärft“; nicht gerechnet ist die Rechenzeit je Glied. **Kipppunkt geprüft (S205)** — das Einschwingen des Glieds, wenn der ganzzahlige Verzug im Slide umspringt, im Zeitbereich (`…/2026-11-18_S205_slide_einschwingen.*`; Gitarre Saiten 1, 4, 6, Bund 5 → 7 → 5 mit 50 und 10 ms je Halbton, 22 / 50 / 84 Sprünge; Kette und Verlust jeden Takt aus der Lage, Direktform 1, das Glied ohne Korrektur beim Sprung): die Störung je Sprung (Ausgang des Glieds gegen ein eingeschwungenes Glied am selben Eingang, ganzes Band, größter 1-ms-Wert gegen das Signal) liegt mit Thiran 5 bei −95 / −83 / −83 dB (50 ms je Halbton) und −80 / −67 / −67 dB (10 ms), mit Thiran 1 bei −75 / −72 / −74 und −75 / −64 / −67 dB — **die höhere Ordnung schwingt nicht schlechter ein, bei 50 ms je Halbton um 9–20 dB besser** (die Beiwerte springen an den Rändern ihres Bereichs weniger weit). Zwei überblendete Glieder (jedes mit Gewicht 0, wenn sein Verzug springt) senken die Störung im Band über den zählenden Teiltönen um weitere 2–16 dB und sind nicht nötig. Der Pegel nach dem Slide liegt bei jedem Glied 0,05–0,24 dB unter dem Ton ohne Slide. Der Kipppunkt greift nicht; d wird nicht nachgerechnet. Ohne Quelle: eine Hörschwelle für diese Störung — −64 dB und tiefer unter dem gleichzeitig klingenden Ton ist eine Einschätzung, keine Messung. Nicht gerechnet: Laute und Oud, andere Lagen, der Ton mit Körper.

**UL3-1 a nachgeschärft (S205)** — die zwei offenen Lautensaiten (`../modellierung/gezupfte-saite/proben/2026-11-18_S205_laute_je_teilton.py`, `…_verlust_entwurfsstellen.*`, `…_gebunden_stuetzstellen_*_rand_verlust_halb.txt`). **Chor 1.1** hatte zwei Ursachen: (1) nur der oberste zählende Teilton lag daneben (8,5 Cent bei einer Abklingrate knapp unter 100 1/s) — die Anpassung endete genau dort und lief darüber frei weg; (2) das Abklingen fiel nur zwischen Bund 10 und 11 heraus (bis +40 % am Grundton), weil dort zwei sehr verschiedene Verlustfilter gemittelt wurden (größter Polradius 0,960 gegen 0,9955). **Chor 8.1** verfehlt nur an Teiltönen über 16 kHz (bis 16 kHz ausgewertet höchstens 2,5 Cent, bis 17 kHz 3,6, bis 19 kHz 5,5, alle 10,3); Thiran 7 und 9 helfen kaum (8,4 / 7,3 Cent). Dazu ein Befund zur Anpassung selbst: sie endet je nach Startwert und Dämpfung in verschiedenen Nebenminima — Chor 1.1 lag in drei Läufen bei 2,3 / 0,4 / 8,8 Cent, Chor 2.1 bei 6,1 / 0,2 Cent; eine Zugabe bis zur doppelten Grenze brach an Gitarre Saite 5 ab. **Für die gebundene Anpassung gilt deshalb (Ergänzung zu UL3-1 a, keine neue Option):** (a) angepasst wird an den Teiltönen bis zur 1,5-fachen Grenze von UL1 a (die Zugabe nur bis 19,5 kHz), gezählt wird weiter bis 100 1/s; (b) der Verlustfilter wird an den Bünden und auf halbem Weg entworfen und dazwischen gemittelt; (c) je Saite vier Anpassungen (Startwert nach Abel & Smith bis zur 1,02- oder 1,3-fachen Grenze, Dämpfung 0,05 oder 0,1), es zählt die mit dem kleinsten größten Fehler an Bünden, halben und Achtel-Stellen; (d) geprüft wird an Stellen, die weder Anpassung noch Auswahl kennen (ungerade Sechzehntel). Damit, Thiran 5, Grad 3, größter Fehler an den fremden Stellen: Gitarre Saiten 6–1 0,06 / 0,09 / 0,45 / 0,12 / 0,23 / 1,59 Cent; Laute Chor 1.1 0,37, Chöre 1.2–6 ≤ 0,27, Chor 7 2,22 Cent; Oud ≤ 0,17 Cent; Abklingen überall in der Schwelle (größter Fehler 12,1 % an Laute Chor 1.1). **Liste der Ausnahmen nach UL2 a (angelegt S205, Patrik):** Laute Chor 8.1 (G4, Lagen 0–12) — Lage bis 12,2 Cent an Teiltönen über 16 kHz (Abklingen in der Schwelle, 21,9 %); Grund: zählende Teiltöne bis 19,7 kHz, das Glied für den gebrochenen Verzug trägt nahe der halben Abtastrate nicht. Sonst keine: **die Tafel je Taste für Harfe und Cembalo ist über den ganzen Umfang neu gerechnet (S205, starrer Steg; `../modellierung/gezupfte-saite/proben/2026-11-18_S205_tafel_je_taste*`)** — das Glied steht mit in der Anpassung (wie in der Probe zu kurzen Umläufen aus S204), Thiran 5, angepasst bis zur 1,5-fachen Grenze, Starts nach Abel & Smith und bei bis zu vier Biquads zehn zufällige: **Harfe 80 von 80 Tasten** in 3 Cent und mit dem Abklingen in der Schwelle (größter Fehler 2,99 Cent, 15,0 %; Biquads im Median 3, höchstens 21), **Cembalo 61 von 61** (2,99 Cent, 5,7 %; Median 7, höchstens 99; die Tasten 72, 73 und 89 brauchen 8 Biquads statt 4). Die Ausnahmen aus UL3 a (Harfe 101–103 mit 21–28 Cent, Cembalo 88 / 89) entfallen; ein Glied niedrigerer Ordnung an den höchsten Tasten ist nicht nötig (Harfe 104: Umlauf 14,4 Takte, ein Biquad, 2,0 Cent). **Mit Körper (S205, `…/2026-11-18_S205_tafel_mit_koerper*`, Rahmen und Tasten wie S204, gegen den genauen Umlauf):** Cembalo (11 Tasten, dazu 76) Lautheit höchstens 0,14 LU, Pegel 1–2 s 0,16 dB, Teiltöne 1–12 0,4 Cent, Abfall früh → spät 0,9 dB (S204: 0,36 LU / 0,13 dB / 1,4 Cent / 3,9 dB) — **Cembalo 71 und 76 sind damit erledigt** (71: Abfall 0,9 statt 3,9 dB, der Rest am 10. Teilton bei −72 dB; 76: 0,6 dB, am starren Steg Abklingen 1,9 %). Harfe (14 Tasten) Lautheit 0,01 LU, Pegel 0,60 dB (Taste 48, wie S204), Teiltöne 1–12 höchstens 2,8 Cent (Taste 84; S204: 1,7), Abfall 3,0 dB an Taste 26 — dort nur der 12. Teilton (450 Hz), der auf einer Körpermode sitzt, in 0,4 s um 49 dB fällt und spät bei −80 statt −85 dB liegt; die Teiltöne 1–11 weichen höchstens 0,06 dB ab. Kein Befund gegen die Tafel.

**UL3 a geprüft (S204)** — drei Proben unter `../modellierung/gezupfte-saite/proben/2026-11-17_S204_*`. (1) **Längs der Bünde** (`…_verlauf_buende`, sechs Gitarrensaiten, Bünde 0–19, Probe auf halbem Weg zwischen den Bünden): je Bund einzeln angepasste Ketten springen (Beiwerte gemittelt: bis 93 Cent); **der Kipppunkt von UL3 a greift — die Kette wird je Saite gebunden angepasst**: logit(Radius) und logit(Winkel) jedes Biquads als Polynom in x = Bund / 19, alle Bünde gemeinsam. Damit an den Saiten 6–2 an den Bünden und dazwischen höchstens 1,3 / 2,3 / 4,5 / 3,6 / 3,6 Cent (Grad 2 oder 3, 8–15 Biquads, 48–120 Zahlen je Saite); einzelne Ausreißer zwischen den Bünden (Saite 3 Grad 3: 110 Cent nach Bund 0, Saite 2 Grad 2: 23 Cent) — die Anpassung braucht Stützstellen auch zwischen den Bünden. Saite 1 bleibt bei 9–19 Cent (kurzer Umlauf, siehe (2)). Der Verlustfilter lässt sich zwischen Nachbarbünden mitteln (b und a direkt): Abklingen an allen Stellen in der Schwelle, Betrag ≤ 1. (2) **Kurzer Umlauf** (`…_kurzer_umlauf`, Harfe 96–104, Cembalo 83–89, Gitarre Saite 1 Bund 12 / 17): wird das Thiran-Glied mit angepasst, der ganzzahlige Verzug mit gesucht (er entscheidet: 0 bis 60 Cent je nach Wahl) und von 40 Startwerten aus gerechnet, liegen alle zwölf Tasten mit 3–4 Biquads unter 1 Cent, das Abklingen in der Schwelle — die Ausnahmen aus UL3 a entfallen an diesen Tasten; über den ganzen Umfang ist diese Fassung noch nicht gerechnet. (3) **Mit Körper** (`…_umlauf_mit_koerper`, Rahmen und Tasten wie S200, gegen den genauen Umlauf): Lautheit höchstens 0,01 LU (Gitarre, Harfe) und 0,36 LU (Cembalo 89; 71: 0,34), Teiltöne 1–12 höchstens 1,3 / 1,4 / 1,7 Cent, Pegel 1–2 s höchstens 0,16 / 0,13 / 0,60 dB (Harfe 48), Abfall früh → spät höchstens 0,4 / 3,9 (Cembalo 71) / 0,6 dB; der Pol 1. Ordnung liegt bei 1,66 LU, 50 Cent, 31 dB. Offen: Cembalo 71 und 76 (Abfall 3,9 dB; Abklingen an 76 außerhalb der Schwelle — vermutlich schmale Pole), die gebundene Anpassung mit Stützstellen zwischen den Bünden und für Saite 1, Laute und Oud, der Lauf der Fassung (2) über alle Tasten, Slide schneller als ein Halbton in 50 ms.

**Dokument der Herleitung (S205)** — [../modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md](../modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md): der Rechenweg des Kerns vom Zupf bis zum Hörort mit Gleichung, Wert, Belegkraft und Probe je Schritt, der Stand der Zielgrößen und die offenen Stellen vor 34k-7 (§12 dort); es ordnet die Entscheide dieses Abschnitts und entscheidet selbst nichts.

**Prüfstand 34k-6 (S199)** — `../modellierung/gezupfte-saite/proben/2026-11-15_S199_pruefstand_zupf.py`: der Ton einer Note geschlossen je Frequenz (steife Saite mit Dämpfung nach Valette, Zupf als Auslenkung und Loslassen mit Fingerbreite, Punktadmittanz und Abstrahlung aus der Körperdatei, Momentary-Maximum nach BS.1770); der Bezug, an dem der Kern in 34k-7 gemessen wird.

**KQ26 a gebaut (S199)** — `make_gitarre.py` Schritt 6 (Statistik; `ziehe` mit SplitMix64, Rayleigh, Box-Muller nachgebaut), `FIT_FASSUNG` 2: α(f) = 0,0105 · f + 147 1/s (gemessen 177 / 164 / 203 / 211 / 211 / 231 / 247 1/s bei 2,5 … 10 kHz, das Gesetz 173 … 253), Dichte 0,0104 Moden je Hz, 161 gezogene Moden; Terzpegel 6,3 / 8 / 10 kHz `left` −0,36 / +1,20 / −0,84 dB, `right` −1,36 / +1,09 / +0,27 dB. Die 75 gefitteten Moden sind unverändert (Feld für Feld verglichen). Der Wächter prüft, dass die im Skript gezogenen Moden die von `ziehe` sind (Zahl, die ersten drei, die letzte).

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
| 0.26.0 | 2026-11-12 | Session 194. §6: PD1 a (Gruppe *Pedals* mit 34k-7 statt 34k-2; Zeilen 34k-2 und 34k-7). |
| 0.27.0 | 2026-11-12 | Session 195. §6: Haltepunkt 34k-4 mit KQ1 a bis KQ5 a (Vorbild und Weg des Körpers je Instrument; Harfe 46 Saiten nach Bell 1988; Gitarre mit Modalfit als drittem Weg zu GZ4 a); Haltepunkt 34k-5 mit KQ6 a (Ersatz-Bauplan für Cembalo, Laute, Oud; erster Fall Oud); erster Fall verfehlt, KQ7 a (Löcher in `InstrumentBody`, dann Oud neu). |
| 0.85.0 | 2026-11-23 | Session 216. §6: KQ68 a (Schale der Harfe als elastisches Bauteil, zuerst als Probe), KQ69 a (gegenseitige Belastung der Öffnungen bleibt Probe bis KQ68). |
| 0.90.0 | 2026-11-26 | Session 222. §6: ZB1 a (Rechenfassung 1 geht mit dem Umstieg in 34k-7d-4), ZU1 a (Quelle des Zupfs je Note genau aus der Frequenzantwort), ZW1 a (Zupfstelle 16 cm fest vom Steg für Gitarre, Laute, Oud); Schnittfolge 34k-7d-1 bis -6. |
| 0.91.0 | 2026-11-26 | Session 223. §6: SR1 a (Richtung am Steg für Körper ohne Messung: die Annahme nach Scherrer beim Bau des `Steg` aus der Körperdatei). |
| 0.89.0 | 2026-11-24 | Session 220. §6: KQ75 a (Tastenstoß des Cembalos: Penttinen Abb. 6 am Seitenbild nachgelesen, GZ10 a bleibt; der Hörtest trennt Stoß und Saite nicht); Teilplan 34k-7 (34k-7a–f), ZK1 a (Tafel des Umlauf-Filters gestimmt), ZK2 a (Saite und Bund in `KettenNote` schon in 34k-7); 34k-7a, 34k-7b, 34k-7c gebaut; TK1 a, TK2 a (Entwurf bei Rangverlust, Cembalo T73). |
| 0.88.0 | 2026-11-24 | Session 219. §6: KQ73 a (Wölbung des Bodens unter dem Saitenzug als Probe — verworfen), KQ74 a (das Paar der Harfe besaitet nur Bericht, Schwelle ab S(1,2); Rahmen an die Werkbank, KQ70 a ruht). |
| 0.87.0 | 2026-11-23 | Session 218. §6: KQ72 a (die Öffnungen der Harfe aus ihrer Geometrie als Kolben in der Wand; löst KQ63 a und KQ64 a ab). |
| 0.86.0 | 2026-11-23 | Session 217. §6: KQ70 a (Prämisse „Kopplung zu stark“ entfällt, zwei Messzustände, „Wirkung des Besaitens“ als eigene Frage), KQ71 a (Massen- und Strahlungsmatrix der Öffnungen: erst Zielgrößen, dann Bau; löst KQ69 a ab). |
| 0.84.0 | 2026-11-23 | Session 215. §6: KQ61 a–KQ65 a (Bau des Rohrs: Querschnitt als Tabelle, alle Längsmoden bis zur Grenze des Fensters, zwei Güten der Luft, Wächter gegen Bell Kap. 4 mit bekannten Abweichungen, Eigenzerlegung aus `InstrumentCore`), KQ66 a (der statistische Teil ohne Moden mit mehr als halber Luft), KQ67 a (Güten 4,4 / 28 bleiben, 13,8 LU bekannte Abweichung, Ursache der Kopplung mit Fable). |
| 0.83.0 | 2026-11-23 | Session 214. §6: KQ59 a (Luft der Harfe als Rohr mit Öffnungen längs, Bauplan-Fassung 10), KQ60 a (Zielgrößen der Luft: Helmholtz 190 Hz, Bezug der Abstrahlung, Profil von S(1,1)−). |
| 0.82.0 | 2026-11-22 | Session 213. §6: KQ57 a (Lagen der Harfe gegen Bell besaitet), KQ58 a (Kopplung an die Luft zu stark: erst die Ursache an den Quellen, dann das Modell). |
| 0.81.0 | 2026-11-22 | Session 212. §6: KQ55 a (F1–F3 an der vollen Rechnung, Probe mit sechs Hebeln an F3), KQ56 a (Bauplan der Harfe neu mit Fable, Suche an der vollen Rechnung). |
| 0.80.0 | 2026-11-21 | Session 211. §6: KQ53 a (`.lvinstbody` Fassung 6 mit `springShare`), KQ54 a (Abstrahlung der Harfe im Test, Lautheit im Bericht); die Harfe gebaut (F4, Saiten 0,3), Befund Modenraum gegen volle Rechnung. |
| 0.79.0 | 2026-11-21 | Session 211. §6: KQ52 a (Faktor der Saiten der Harfe 0,3; größter Sprung 12,8 LU als bekannte Abweichung), KQ51 a mit dem Wert nachgeführt. |
| 0.78.0 | 2026-11-21 | Session 211. §6: KQ51 a (Harfe in der Fassung F4: Drehfeder 7 000 N bis 0,6 m, Mündung 2,3; größter Sprung 12,2 LU als bekannte Abweichung). |
| 0.77.0 | 2026-11-20 | Session 210. §6: KQ51 offen eingetragen (Fassung der Harfe, vier Fassungen der Feinsuche gegen alle Zielgrößen gerechnet, Empfehlung F4), KQ52 steht aus. |
| 0.76.0 | 2026-11-20 | Session 210. §6: KQ50 a (hebt KQ47 a auf: Boden der Harfe bleibt 10 → 3 mm, Steife im Bass aus einer Drehfeder an den Bassseiten; Eichung mit 12 → 2 mm gerechnet, der Kandidat aus S209 hält nicht). |
| 0.75.0 | 2026-11-20 | Session 210. §6: KQ49 a (Verlust der Punktfedern: eigener Verlustfaktor, die Energie der Federn zählt in der Dämpfung); KQ46 a gebaut (Bauplan-Fassung 9, `pointSprings`). |
| 0.74.0 | 2026-11-19 | Session 209. §6: KQ46 a (der Körper der Harfe ist die besaitete Harfe: Federn an Punkten, Bauplan-Fassung 9), KQ47 a (Bauplan der Harfe: Boden 12 → 2 mm, Bassende eingespannt, Mündung um 2,0, nach neuer Eichung fein suchen), KQ48 a (Abstrahlung an den Saiten 33–22 bekannte Abweichung). |
| 0.73.0 | 2026-11-19 | Session 209. §6: KQ42 a–KQ45 a (Bauplan der Oud neu: Luftmode 139 Hz, Deckenmode 203 Hz, Balkenenden am Rand, Steg, Rosetten; Lagen mit Aufnehmer auf 3 %, Ziel 3 hält, Ziel 2 und KQ36 a bekannte Abweichung). |
| 0.72.0 | 2026-11-19 | Session 208. §6: KQ41 a (Bericht zu Auftrag 35 gegengelesen; Ziel 2 von KQ38 a im Band 1,5–2,0). |
| 0.71.0 | 2026-11-19 | Session 208. §6: KQ39 a (KQ34 a an der Harfe mit Mitschwingen nach GZ8 a), KQ40 a (Harfenkörper: Lage von S(1,1)± und Abstrahlung an den Saiten 33–22 gegen Bell Abb. 7.23 als Ziele, Suche im Bauplan mit Fable). |
| 0.70.0 | 2026-11-19 | Session 207. §6: KQ38 a (Bauplan der Oud: Höhe der Spitze um 203 Hz und Lage der Hauptspitze der Abstrahlung als Ziele, neue Suche nach dem Bericht zu Auftrag 35). |
| 0.69.0 | 2026-11-19 | Session 207. §6: KQ37 a (Oud: bekannte Abweichung bis zur Messgrundlage, Auftrag 35 der Werkbank; Verdacht am Bauplan — Höhen und Formen gegen Abdallah nicht verglichen). |
| 0.68.0 | 2026-11-19 | Session 207. §6: KQ36 a — Laute, Oud, Cembalo gemessen (Laute und Cembalo halten, Oud verfehlt Spanne und größten Sprung). |
| 0.67.0 | 2026-11-19 | Session 207. §6: KQ36 a (KQ34 a am Modell als Leistungsmittel über 16 Hörorte in 2 m; gemessene Körper gegen die TU-Verteilung bei gleicher Zahl Mikrofone). |
| 0.66.0 | 2026-11-18 | Session 206. §6: KQ35-1 a und KQ35-2 a gebaut (`.lvinstbody` Fassung 5, die Gitarre mit `kFitFassung` 4, alle Körper neu geschrieben). |
| 0.65.0 | 2026-11-18 | Session 206. §6: KQ31-1 a gebaut (Bauplan-Fassung 8, die Harfe mit beiden Leisten, alle Körper mit Bauplan neu), KQ29-1 a (Güte der Luft der Harfe 8,5), KQ31-3 a (Überschreitungen von KQ31-2 a als bekannte Abweichungen im Test). |
| 0.64.0 | 2026-11-18 | Session 205. §6: KQ35-1 a (Richtung je Band und Moden parallel zur Decke), KQ35-2 a (die Richtung je Mode in der Körperdatei). |
| 0.63.0 | 2026-11-18 | Session 205. §6: Befund zu KQ35 a, erster Teil (Dichte und Richtung der Gitarre nach Elie, Wirkung auf die Zielgrößen), offen KQ35-1 und KQ35-2. |
| 0.62.0 | 2026-11-18 | Session 205. §6: KQ31-1 a (Bauplan-Fassung 8: Leisten übereinander), KQ31-2 a (die Harfe an den Moden gemessen, das Terzband-Ziel entfällt). |
| 0.61.0 | 2026-11-18 | Session 205. §6: Befund zu KQ31 a (Zugleiste fehlt in der Rechnung; federartiger Untergrund in Bells Messung), offen KQ31-1 und KQ31-2. |
| 0.60.0 | 2026-11-18 | Session 205. §6: Verweis auf das Dokument der Herleitung 34k-6. |
| 0.59.0 | 2026-11-18 | Session 205. §6: die Tafel je Taste mit Körper geprüft (Cembalo 71 / 76 erledigt). |
| 0.58.0 | 2026-11-18 | Session 205. §6: Tafel je Taste für Harfe und Cembalo über den ganzen Umfang mit Thiran 5 (141 von 141 Tasten in 3 Cent); die Ausnahmen aus UL3 a entfallen. |
| 0.57.0 | 2026-11-18 | Session 205. §6: UL3-1 a nachgeschärft (Anpassung bis zur 1,5-fachen Grenze, Verlustfilter auch auf halbem Weg, vier Starts, Prüfung an fremden Stellen); Liste der Ausnahmen nach UL2 a angelegt (Laute Chor 8.1). |
| 0.56.0 | 2026-11-18 | Session 205. §6: Kipppunkt von UL3-1 a geprüft (Einschwingen des Glieds im Slide: Thiran 5 nicht schlechter als Thiran 1). |
| 0.55.0 | 2026-11-18 | Session 205. §6: UL3-1 a (gebrochener Verzug über Thiran 5. Ordnung; Kette je Saite gebunden mit Stützstellen auf halbem Weg und gedämpften Beiwerten). |
| 0.54.0 | 2026-11-17 | Session 204. §6: UL1 a (zählende Teiltöne: Abklingrate ≤ 100 1/s), UL2 a (je Teilton ±3 Cent, Abklingzeit im Faktor nach HW1 a), UL3 a (Umlauf-Filter als Tafel je Taste: Verlust 8. Ordnung mit \|H\| ≤ 1, Biquad-Allpässe für die Steifheit). |
| 0.53.0 | 2026-11-17 | Session 203. §6: KQ35 a (Gitarre: Dichte über 2,2 kHz und zweite Richtung nach Elie, eigener Schritt vor 34k-7 mit Fable). |
| 0.52.0 | 2026-11-17 | Session 203. §6: KQ34 a (Schwellen aus reflexionsarmen Aufnahmen: Harfe ab D2 ≤ 22 LU, mittlerer Sprung ≤ 3 LU). |
| 0.51.0 | 2026-11-17 | Session 203. §6: KQ32 a (Teiltonmessung: zwei Stufen als zwei Komponenten mit Boden), KQ33 a (Schwellen aus GZ14 a an reflexionsarmen Aufnahmen der TU Berlin). |
| 0.50.0 | 2026-11-17 | Session 203. §6: KQ31 a (Harfe: Admittanz ohne Saiten über 200 Hz bis 10 dB unter Bell, Untersuchung als eigener Schritt vor 34k-7 mit Fable). |
| 0.49.0 | 2026-11-17 | Session 203. §6: KQ30 a (Cembalo: Ablösekraft 1,07 N an C4 nach Chadefaux 2013 statt 2 N, Tastenstoß 3,8 statt 7 mN s; Klang unverändert). |
| 0.48.0 | 2026-11-16 | Session 201. §6: KQ29 a (Harfe: Güte der Luft 5,5 statt 16 nach Bells Güten in der Harfe; größter Sprung 13,5 → 11,0 LU, die Spanne bleibt am Diskantende verfehlt). |
| 0.47.0 | 2026-11-16 | Session 201. §6: KQ28 a (Gitarre: Form am Steg an Steg- und Schallkurven gemeinsam, `kFitFassung` 3; der Sprung an den Tasten 81 / 83 war ein Fehler des Fits). |
| 0.46.0 | 2026-11-16 | Session 200. §6: GZ13 a (Ablösekraft des Cembalos −2,8 dB je Oktave), GZ14 a (Spanne ≤ 15 LU — Harfe ab D2 —, mittlerer Sprung ≤ 4 LU, größter ≤ 12 LU; ersetzt die Schwellen aus GZ2 a). |
| 0.45.0 | 2026-11-16 | Session 200. §6: GZ10 a (Tastenstoß 7 mN s bei ff, 10 dB je Stufe; Ablösekraft 2 N), GZ11 a (Lautenzug als Dämpfer am Sattel, 40 N s/m bei 0,5 mm), GZ12 a (ein 8′ je Taste, weitere Register mit 31g). |
| 0.44.0 | 2026-11-16 | Session 200. §6: GZ8 a (Harfe mit Oktaven, Quinten, Quarten; die Rechenzeit-Grenze gilt für eine Saite ohne Mitschwingen), GZ9 a (Haltepunkt Bend bestanden). |
| 0.43.0 | 2026-11-15 | Session 199. §6: GZ5-1 a (über 5 kHz ein Dämpfer aus den Größen des statistischen Teils in der Saitenschleife; am Cembalo gerechnet). |
| 0.42.0 | 2026-11-15 | Session 199. §6: GZ5 a (alle Moden der Körperdatei in der Saitenschleife, Stegkraft aus dem Kern, Faltung in der App), GZ7 a (die App reicht die Moden mit dem Tonauftrag, kein Stegmoden-Werkzeug); Zeile 34k-7. |
| 0.41.0 | 2026-11-15 | Session 199. §6: GZ6 a (Gitarre: die Stufe setzt die Auslenkung und kippt das Spektrum des Zupfs, an Iowa MIS angepasst). |
| 0.40.0 | 2026-11-15 | Session 199. §6: KQ27 a (die Form des Körpers im Kern leitet 34k-6 her, das Stegmoden-Werkzeug aus 34k-5 baut 34k-7; Zeilen 34k-5 und 34k-7); Prüfstand 34k-6 im Frequenzbereich. |
| 0.39.0 | 2026-11-15 | Session 199. §6: Befund der Gitarre über 5 kHz (die Messung trägt bis rund 10 kHz; vier Fassungen gegen die Messung), KQ26 a (statistischer Teil 5–20 kHz mit Größen an der Messung, `kFitFassung` 2); gebaut. |
| 0.38.0 | 2026-11-15 | Session 199. §6: KQ24 a (Abstrahlung der Gitarre gemessen und geteilt bei 1 kHz, ersetzt den Weg aus KQ3 a), KQ25 a (gemessener Körper als `.lvinstbody` Fassung 4 mit Herkunft und Beschreibung `.lvinstbodyfit`); Körper `classical_guitar` gebaut. |
| 0.37.0 | 2026-11-15 | Session 199. §6: Modalfit der Gitarre am Archiv Mores (Steg mit Biegung, Gegenproben am Schall und gegen Wright 1996); KQ23 a (Vorbild g34, Manuel Contreras 1971; Paare an A0 und T1 sind geteilte Pole); offen Abstrahlung, Ablage, zweite Richtung. |
| 0.36.0 | 2026-11-15 | Session 199. §6: KQ22 a (je chromatischer Taste ein Stegpunkt im Bauplan der Harfe, 80 Punkte an 46 Orten, Tasten 25–104); Nebenbefund Laute und Oud (Körper nur an den leeren Saiten, bis 34k-8). |
| 0.35.0 | 2026-11-14 | Session 198. §6: KQ19 a (Ziele der Harfe), KQ20 a (Halt je Randstück, Bauplan Fassung 7), KQ21 a (Diskantkante der Harfe 3·10⁶ N/m², angepasst); Körper `concert_harp`; offen die übrige Ursache am Diskantende und die Taste je Saite. |
| 0.34.0 | 2026-11-14 | Session 198. §6: Berichtigung zu KQ18 a — der Rand der Harfe ist gestützt (Feder 0), die Zuordnung der Formen war um eins versetzt; offen Ziele und Schwelle der Harfe (KQ19). |
| 0.33.0 | 2026-11-13 | Session 198. §6: Kap. 7 von Bell gelesen, Rand des Harfenbodens gerechnet; KQ18 a (Rand im Bauplan: eingespannt oder elastisch mit Drehfeder, Fassung 6, alle Körper neu) und Folgeentscheid (ein Federwert für den ganzen Rand); offen S(2,1), Ziele und Schwelle der Harfe. |
| 0.32.0 | 2026-11-13 | Session 197. §6: Befund an der Harfe (Bell 1988 gelesen: Maße, Öffnungen in der Schale, Tab. 6.1 mit 17 Moden); KQ17 a (Öffnungen des Hohlraums außerhalb der Platte, Bauplan Fassung 5, alle Körper neu); höhere Luftmoden offen. |
| 0.31.0 | 2026-11-13 | Session 197. §6: Befund am Cembalo (keine gemessene Kurve; Boden des Dulcken 1755 in Bader 2025 als Bauplan lesbar); KQ13 a (Körper = Boden des Dulcken, Saiten Taskin), KQ14 a (Dicke als Raster — im Format vorhanden), KQ15 a (Tiefe 20,7 cm, Luftmode als bekannte Abweichung), KQ16 a (Randtasten); Bauplan und Körper des Cembalos unter `asset/`. |
| 0.30.0 | 2026-11-13 | Session 197. §6: Befund an der gemessenen Stegadmittanz der Laute (Woodhouse 2021 Abb. 1) und an der Rauwolf-Zeichnung; KQ11 a (Ziele in Lage und Höhe, mit Stegleiste und Saitenlast), KQ12 a (Form mal 625/677,4); Bauplan und Körper der Laute unter `asset/`. |
| 0.29.0 | 2026-11-13 | Session 196. §6: Befund an der gemessenen Kurve der Oud (Abb. 6), KQ10 a ersetzt KQ9 a (Balken wie gezeichnet, 1,5 mm; Ziele 101 / 203 / 280 / 362 Hz); Bauplan und Körper der Oud unter `asset/`, `.lvinstbody` Fassung 3. |
| 0.28.0 | 2026-11-13 | Session 196. §6: KQ7 a gebaut (Löcher in `InstrumentBody`, Bauplan Fassung 4), zweiter Fall der Oud verfehlt die Schwelle; KQ8 a (erst der Luftraum nach `InstrumentBody`, dann die Oud mit Luftraum gegen die Messung); Luftraum gebaut (F-K1 a), KQ9 a (KQ6 a gilt weiter; Oud mit Luftraum, Balken am Rand, 1,5 mm). |
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

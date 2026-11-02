# LumiViz — Instrumente aus InstrumentsCalculation (Konzept)

> **Version:** 0.2.0
> **Datum:** 2026-11-02 (Session 172)
> **Typ:** Konzept — Planung der Anbindung der Rechenkerne aus `InstrumentsCalculation` (IC) an LumiViz;
> Zielbild Z1 bis Z4 und M1 entschieden (Patrik S172), **IK2-0 vorgelegt (Antwort in S173)**, IK2-1 bis IK2-13 offen,
> Teilplan folgt
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

🟡 **IK2-0 — vorgelegt S172, Antwort in S173** (Patrik: in der nächsten Sitzung erneut stellen). Wortlaut der Vorlage:

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

## 6. Teilplan

*Folgt nach den Entscheiden. Stufennummer: Vorschlag **Stufe 34** (32 bleibt das Upright, 33 ist vergeben).*

## 7. Changelog

| Version | Datum | Änderung |
|---|---|---|
| 0.2.0 | 2026-11-02 | Session 172. §4 um T2 a, Architektur, Plugins, Messung (M1 a) und Loop-Zahlen ergänzt; §5 M1 a und die Vorlage IK2-0 im Wortlaut (Antwort in S173). |
| 0.1.0 | 2026-11-02 | Session 172. Angelegt: Ziel, Voraussetzungen (Patrik S171), Zielbild Z1 bis Z4 (Patrik S172, je a), Ausgangslage; Entscheide und Teilplan offen. |
